#include "EffectV2_Runtime.h"
#include "ActorCatalog.h"
#include "EffectV2_Document.h"
#include "EffectV2_Object.h"
#include "GameInstance.h"
#include "Model.h"
#include "Npc.h"
#include "NpcPresentationAssetService.h"

#include <map>
#include <set>
#include <unordered_map>
#include <vector>

namespace
{
	constexpr const wchar_t* EFFECT_LAYER_TAG = L"Layer_EffectV2";
	constexpr const wchar_t* EFFECT_PROTOTYPE_TAG = L"Prototype_GameObject_EffectV2";
	constexpr const char* VALTAN_ARCHETYPE_ID = "BOSS_VALTAN";

	struct BINDING_SET final
	{
		bool_t bLoaded = false;
		bool_t bFailed = false;
		std::vector<Client::EFFECT_V2_BINDING> Bindings;
	};

	struct DOCUMENT_ENTRY final
	{
		bool_t bLoaded = false;
		bool_t bFailed = false;
		Client::EFFECT_V2_DOCUMENT Document;
	};

	struct PENDING_SPAWN final
	{
		Client::EFFECT_V2_BINDING Binding;
		bool_t bSpawned = false;
	};

	struct SPAWNED_EFFECT final
	{
		std::weak_ptr<Client::CEffectV2Object> pObject;
		bool_t bStopWithClip = false;
		bool_t bStageBound = false;
	};

	struct TARGET_STATE final
	{
		Client::EFFECT_V2_TARGET Target;
		std::string strArchetypeId;
		std::string strClip;
		f32_t fLastSeconds = -1.f;
		std::vector<PENDING_SPAWN> Pending;
		std::string strStage;
		f32_t fStageLastSeconds = -1.f;
		std::vector<PENDING_SPAWN> StagePending;
		std::vector<SPAWNED_EFFECT> Spawned;
	};

	std::unordered_map<std::string, BINDING_SET> g_BindingSets;
	std::unordered_map<std::string, DOCUMENT_ENTRY> g_Documents;
	std::map<std::wstring, std::string> g_ModelTagToArchetype;
	bool_t g_bModelTagMapBuilt = false;
	std::map<const Engine::CGameObject*, TARGET_STATE> g_TargetStates;
	std::set<const Engine::CGameObject*> g_IgnoredTargets;
	bool_t g_bPrototypeRegistered = false;
	std::string g_strLastError;

	void Report(const std::string& strMessage)
	{
		g_strLastError = strMessage;
		OutputDebugStringA(("[EffectV2Runtime] " + strMessage + "\n").c_str());
	}

	const std::string* Resolve_Archetype(const Client::EFFECT_V2_TARGET& Target)
	{
		static const std::string ValtanArchetype = VALTAN_ARCHETYPE_ID;
		if (Client::EFFECT_V2_TARGET_KIND::VALTAN == Target.eKind)
			return &ValtanArchetype;
		if (Client::EFFECT_V2_TARGET_KIND::NPC != Target.eKind)
			return nullptr;
		const std::shared_ptr<Engine::CGameObject> pOwner = Target.pOwner.lock();
		if (nullptr == pOwner)
			return nullptr;
		if (!g_bModelTagMapBuilt)
		{
			g_bModelTagMapBuilt = true;
			for (const Client::NPC_ACTOR_ENTRY& Entry : Client::CActorCatalog::Get_Npcs())
			{
				const wstring_t strTag =
					Client::CNpcPresentationAssetService::Get_ModelPrototypeTag(Entry.archetypeId);
				if (!strTag.empty() && !g_ModelTagToArchetype.contains(strTag))
					g_ModelTagToArchetype.emplace(strTag, Entry.archetypeId);
			}
		}
		const auto Found = g_ModelTagToArchetype.find(
			std::static_pointer_cast<Client::CNpc>(pOwner)->Get_ModelTag());
		return Found != g_ModelTagToArchetype.end() ? &Found->second : nullptr;
	}

	const BINDING_SET& Ensure_Bindings(const std::string& strArchetypeId)
	{
		BINDING_SET& Set = g_BindingSets[strArchetypeId];
		if (Set.bLoaded)
			return Set;
		Set.bLoaded = true;
		std::error_code Error;
		const std::filesystem::path Path =
			Client::CEffectV2Document::Binding_Path(strArchetypeId);
		if (Path.empty() || !std::filesystem::is_regular_file(Path, Error))
			return Set;
		std::string strError;
		if (!Client::CEffectV2Document::Load_BindingsFile(strArchetypeId, Set.Bindings, strError))
		{
			Set.bFailed = true;
			Set.Bindings.clear();
			Report("bindings rejected for " + strArchetypeId + ": " + strError);
		}
		return Set;
	}

	const DOCUMENT_ENTRY& Ensure_Document(const std::string& strEffectId)
	{
		DOCUMENT_ENTRY& Entry = g_Documents[strEffectId];
		if (Entry.bLoaded)
			return Entry;
		Entry.bLoaded = true;
		std::string strError;
		if (!Client::CEffectV2Document::Load_DocumentFile(strEffectId, Entry.Document, strError))
		{
			Entry.bFailed = true;
			Report("document rejected " + strEffectId + ": " + strError);
		}
		return Entry;
	}

	bool_t Ensure_Prototype(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext)
	{
		if (g_bPrototypeRegistered)
			return true;
		unique_ptr<Client::CEffectV2Object> pPrototype =
			Client::CEffectV2Object::Create(pDevice, pContext);
		if (nullptr == pPrototype)
		{
			Report("effect prototype creation failed.");
			return false;
		}
		(void)CGameInstance::Get().Add_Prototype(
			ETOUI(LEVEL::STATIC), EFFECT_PROTOTYPE_TAG, std::move(pPrototype));
		g_bPrototypeRegistered = true;
		return true;
	}

	void Spawn(
		TARGET_STATE& State,
		PENDING_SPAWN& Pending,
		const Client::EFFECT_V2_TARGET_VIEW& View,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext,
		const bool_t bStageBound)
	{
		Pending.bSpawned = true;
		const DOCUMENT_ENTRY& Entry = Ensure_Document(Pending.Binding.strEffectId);
		if (Entry.bFailed || !Ensure_Prototype(pDevice, pContext))
			return;
		CGameInstance& GameInstance = CGameInstance::Get();
		Client::CEffectV2Object::DESC Desc = Entry.Document.Desc;
		if (!Client::CEffectV2Object::Resolve_TargetPivot(
			View,
			Pending.Binding.strBone,
			Pending.Binding.eRotation,
			Desc.PivotWorld))
		{
			Report("pivot bone not found: " + Pending.Binding.strBone +
				" for " + Pending.Binding.strEffectId);
			return;
		}
		std::shared_ptr<CGameObject> pGameObject;
		if (FAILED(GameInstance.Add_GameObject_to_Layer(
			ETOUI(LEVEL::STATIC), EFFECT_PROTOTYPE_TAG,
			GameInstance.Get_CurrentLevelID(), EFFECT_LAYER_TAG,
			&Desc, &pGameObject)))
		{
			Report("spawn failed for " + Pending.Binding.strEffectId + ": " +
				Client::CEffectV2Object::Last_Error());
			return;
		}
		const std::shared_ptr<Client::CEffectV2Object> pObject =
			std::dynamic_pointer_cast<Client::CEffectV2Object>(pGameObject);
		if (nullptr == pObject)
			return;
		for (uint32_t iPart = 0u;
			iPart < Entry.Document.Parts.size() && iPart < pObject->Part_Count(); ++iPart)
		{
			pObject->Part_Visible(iPart) = Entry.Document.Parts[iPart].bVisible;
			if (!Entry.Document.Parts[iPart].strBaseAssetId.empty())
				(void)pObject->Set_PartBase(iPart, Entry.Document.Parts[iPart].strBaseAssetId);
		}
		if (!Entry.Document.strAnimationClip.empty())
		{
			for (uint32_t iClip = 0u; iClip < pObject->Animation_Count(); ++iClip)
			{
				const char_t* pName = pObject->Animation_Name(iClip);
				if (nullptr != pName && Entry.Document.strAnimationClip == pName)
				{
					pObject->Params().iAnimationIndex = iClip;
					break;
				}
			}
		}
		if (Pending.Binding.bFollowBone)
		{
			pObject->Set_FollowTarget(
				State.Target, Pending.Binding.strBone, Pending.Binding.eRotation);
		}
		State.Spawned.push_back({ pObject, Pending.Binding.bStopWithClip, bStageBound });
	}

	/* bStopClipBound finishes stopWithClip effects of the clip lane,
	   bStopStageBound those of the stage lane; both lanes always drop what has
	   finished on its own. */
	void Prune_Spawned(
		TARGET_STATE& State,
		const bool_t bStopClipBound,
		const bool_t bStopStageBound = false)
	{
		CGameInstance& GameInstance = CGameInstance::Get();
		for (auto Iterator = State.Spawned.begin(); Iterator != State.Spawned.end();)
		{
			const std::shared_ptr<Client::CEffectV2Object> pObject = Iterator->pObject.lock();
			const bool_t bStopLane = Iterator->bStageBound ? bStopStageBound : bStopClipBound;
			if (nullptr != pObject && bStopLane && Iterator->bStopWithClip)
				pObject->Finish();
			if (nullptr == pObject || pObject->Is_Finished())
			{
				if (nullptr != pObject)
				{
					GameInstance.Remove_GameObject_from_Layer(
						GameInstance.Get_CurrentLevelID(), EFFECT_LAYER_TAG, pObject);
				}
				Iterator = State.Spawned.erase(Iterator);
				continue;
			}
			++Iterator;
		}
	}

	void Remove_AllSpawned(TARGET_STATE& State)
	{
		CGameInstance& GameInstance = CGameInstance::Get();
		for (SPAWNED_EFFECT& Spawned : State.Spawned)
		{
			if (const std::shared_ptr<Client::CEffectV2Object> pObject =
					Spawned.pObject.lock())
			{
				GameInstance.Remove_GameObject_from_Layer(
					GameInstance.Get_CurrentLevelID(), EFFECT_LAYER_TAG, pObject);
			}
		}
		State.Spawned.clear();
	}
}

void Client::CEffectV2Runtime::Prewarm_Archetype(
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext,
	const std::string& strArchetypeId)
{
	const BINDING_SET& Set = Ensure_Bindings(strArchetypeId);
	if (Set.bFailed)
		return;
	for (const EFFECT_V2_BINDING& Binding : Set.Bindings)
	{
		const DOCUMENT_ENTRY& Entry = Ensure_Document(Binding.strEffectId);
		if (Entry.bFailed)
			continue;
		std::string strError;
		if (FAILED(CEffectV2Object::Prewarm(pDevice, pContext, Entry.Document.Desc, strError)))
			Report("prewarm failed " + Binding.strEffectId + ": " + strError);
	}
	Ensure_Prototype(pDevice, pContext);
}

void Client::CEffectV2Runtime::Set_Ignored(const EFFECT_V2_TARGET& Target, const bool_t bIgnored)
{
	if (nullptr == Target.pKey)
		return;
	if (bIgnored)
	{
		g_IgnoredTargets.insert(Target.pKey);
		const auto Found = g_TargetStates.find(Target.pKey);
		if (Found != g_TargetStates.end())
		{
			Prune_Spawned(Found->second, true, true);
			g_TargetStates.erase(Found);
		}
	}
	else
		g_IgnoredTargets.erase(Target.pKey);
}

void Client::CEffectV2Runtime::Invalidate_Caches()
{
	g_BindingSets.clear();
	g_Documents.clear();
	g_ModelTagToArchetype.clear();
	g_bModelTagMapBuilt = false;
}

const std::string& Client::CEffectV2Runtime::Last_Error()
{
	return g_strLastError;
}

void Client::CEffectV2Runtime::Notify_Clip(
	const EFFECT_V2_TARGET& Target,
	const char_t* pClipName)
{
	if (!Target.Is_Valid() || nullptr == pClipName || g_IgnoredTargets.contains(Target.pKey))
		return;
	const std::string* pArchetypeId = Resolve_Archetype(Target);
	if (nullptr == pArchetypeId)
		return;
	const BINDING_SET& Set = Ensure_Bindings(*pArchetypeId);
	TARGET_STATE& State = g_TargetStates[Target.pKey];
	State.Target = Target;
	State.strArchetypeId = *pArchetypeId;
	State.strClip = pClipName;
	State.fLastSeconds = -1.f;
	Prune_Spawned(State, true);
	State.Pending.clear();
	if (Set.bFailed)
		return;
	for (const EFFECT_V2_BINDING& Binding : Set.Bindings)
	{
		if (Binding.strStage.empty() && Binding.strClip == State.strClip)
			State.Pending.push_back({ Binding, false });
	}
}

void Client::CEffectV2Runtime::Sync_Stage(
	const EFFECT_V2_TARGET& Target,
	const char_t* pActionId,
	const f32_t fAgeSeconds,
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext)
{
	if (!Target.Is_Valid() || g_IgnoredTargets.contains(Target.pKey))
		return;
	const std::string strActionId = nullptr != pActionId ? pActionId : "";
	const auto Found = g_TargetStates.find(Target.pKey);
	if (Found == g_TargetStates.end() && strActionId.empty())
		return;
	const std::string* pArchetypeId = Resolve_Archetype(Target);
	if (nullptr == pArchetypeId)
		return;
	TARGET_STATE& State = Found != g_TargetStates.end() ?
		Found->second : g_TargetStates[Target.pKey];
	State.Target = Target;
	State.strArchetypeId = *pArchetypeId;
	if (State.strStage != strActionId)
	{
		State.strStage = strActionId;
		State.fStageLastSeconds = -1.f;
		Prune_Spawned(State, false, true);
		State.StagePending.clear();
		const BINDING_SET& Set = Ensure_Bindings(*pArchetypeId);
		if (!Set.bFailed && !strActionId.empty())
		{
			for (const EFFECT_V2_BINDING& Binding : Set.Bindings)
			{
				if (!Binding.strStage.empty() && Binding.strStage == strActionId)
					State.StagePending.push_back({ Binding, false });
			}
		}
	}
	if (State.StagePending.empty() || !std::isfinite(fAgeSeconds) || fAgeSeconds < 0.f)
	{
		Prune_Spawned(State, false, false);
		return;
	}
	if (fAgeSeconds < State.fStageLastSeconds)
	{
		for (PENDING_SPAWN& Pending : State.StagePending)
			Pending.bSpawned = false;
	}
	State.fStageLastSeconds = fAgeSeconds;
	EFFECT_V2_TARGET_VIEW View;
	if (!CEffectV2Object::Resolve_TargetView(State.Target, View))
		return;
	for (PENDING_SPAWN& Pending : State.StagePending)
	{
		if (Pending.bSpawned)
			continue;
		if (fAgeSeconds >= static_cast<f32_t>(Pending.Binding.iStartMs) / 1000.f)
			Spawn(State, Pending, View, pDevice, pContext, true);
	}
	Prune_Spawned(State, false, false);
}

void Client::CEffectV2Runtime::Reset_LocalPreviewTarget(
	const EFFECT_V2_TARGET& Target)
{
	if (nullptr == Target.pKey)
		return;
	const auto Found = g_TargetStates.find(Target.pKey);
	if (Found == g_TargetStates.end())
		return;
	Remove_AllSpawned(Found->second);
	g_TargetStates.erase(Found);
}

void Client::CEffectV2Runtime::Tick(
	const EFFECT_V2_TARGET& Target,
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext)
{
	for (auto Iterator = g_TargetStates.begin(); Iterator != g_TargetStates.end();)
	{
		if (!Iterator->second.Target.Is_Valid())
		{
			Prune_Spawned(Iterator->second, true, true);
			Iterator = g_TargetStates.erase(Iterator);
			continue;
		}
		++Iterator;
	}
	if (nullptr == Target.pKey)
		return;
	const auto Found = g_TargetStates.find(Target.pKey);
	if (Found == g_TargetStates.end())
		return;
	TARGET_STATE& State = Found->second;
	EFFECT_V2_TARGET_VIEW View;
	if (!CEffectV2Object::Resolve_TargetView(State.Target, View))
		return;
	const shared_ptr<Engine::CModel>& pModel = View.pModel;
	const uint32_t iClip = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentClip = pModel->Get_AnimationName(iClip);
	if (nullptr == pCurrentClip || State.strClip != pCurrentClip)
	{
		Prune_Spawned(State, true);
		State.Pending.clear();
		return;
	}
	if (State.Pending.empty())
	{
		Prune_Spawned(State, false);
		return;
	}
	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const f32_t fTickPerSecond = pModel->Get_AnimationTickPerSecond(iClip);
	if (fTickPerSecond <= 0.f || !pModel->Get_AnimationProgress(iClip, fPosition, fDuration))
		return;
	const f32_t fSeconds = fPosition / fTickPerSecond;
	if (fSeconds < State.fLastSeconds)
	{
		for (PENDING_SPAWN& Pending : State.Pending)
			Pending.bSpawned = false;
	}
	State.fLastSeconds = fSeconds;
	for (PENDING_SPAWN& Pending : State.Pending)
	{
		if (Pending.bSpawned)
			continue;
		if (fSeconds >= static_cast<f32_t>(Pending.Binding.iStartMs) / 1000.f)
			Spawn(State, Pending, View, pDevice, pContext, false);
	}
	Prune_Spawned(State, false);
}

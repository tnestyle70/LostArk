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
	};

	struct NPC_STATE final
	{
		std::weak_ptr<Client::CNpc> pNpc;
		std::string strArchetypeId;
		std::string strClip;
		f32_t fLastSeconds = -1.f;
		std::vector<PENDING_SPAWN> Pending;
		std::vector<SPAWNED_EFFECT> Spawned;
	};

	std::unordered_map<std::string, BINDING_SET> g_BindingSets;
	std::unordered_map<std::string, DOCUMENT_ENTRY> g_Documents;
	std::map<std::wstring, std::string> g_ModelTagToArchetype;
	bool_t g_bModelTagMapBuilt = false;
	std::map<const Client::CNpc*, NPC_STATE> g_NpcStates;
	std::set<const Client::CNpc*> g_IgnoredNpcs;
	bool_t g_bPrototypeRegistered = false;
	std::string g_strLastError;

	void Report(const std::string& strMessage)
	{
		g_strLastError = strMessage;
		OutputDebugStringA(("[EffectV2Runtime] " + strMessage + "\n").c_str());
	}

	const std::string* Resolve_Archetype(const Client::CNpc& Npc)
	{
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
		const auto Found = g_ModelTagToArchetype.find(Npc.Get_ModelTag());
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
		NPC_STATE& State,
		PENDING_SPAWN& Pending,
		const std::shared_ptr<Client::CNpc>& pNpc,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext)
	{
		Pending.bSpawned = true;
		const DOCUMENT_ENTRY& Entry = Ensure_Document(Pending.Binding.strEffectId);
		if (Entry.bFailed || !Ensure_Prototype(pDevice, pContext))
			return;
		CGameInstance& GameInstance = CGameInstance::Get();
		Client::CEffectV2Object::DESC Desc = Entry.Document.Desc;
		if (!Client::CEffectV2Object::Resolve_TargetPivot(
			*pNpc,
			Pending.Binding.bFollowBone ? Pending.Binding.strBone : std::string(),
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
				pNpc, Pending.Binding.strBone, Pending.Binding.eRotation);
		}
		State.Spawned.push_back({ pObject, Pending.Binding.bStopWithClip });
	}

	void Prune_Spawned(NPC_STATE& State, const bool_t bStopClipBound)
	{
		CGameInstance& GameInstance = CGameInstance::Get();
		for (auto Iterator = State.Spawned.begin(); Iterator != State.Spawned.end();)
		{
			const std::shared_ptr<Client::CEffectV2Object> pObject = Iterator->pObject.lock();
			if (nullptr != pObject && bStopClipBound && Iterator->bStopWithClip)
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

void Client::CEffectV2Runtime::Set_Ignored(const std::shared_ptr<CNpc>& pNpc, const bool_t bIgnored)
{
	if (nullptr == pNpc)
		return;
	if (bIgnored)
	{
		g_IgnoredNpcs.insert(pNpc.get());
		const auto Found = g_NpcStates.find(pNpc.get());
		if (Found != g_NpcStates.end())
		{
			Prune_Spawned(Found->second, true);
			g_NpcStates.erase(Found);
		}
	}
	else
		g_IgnoredNpcs.erase(pNpc.get());
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

void Client::CEffectV2Runtime::Notify_NpcClip(
	const std::shared_ptr<CNpc>& pNpc,
	const char_t* pClipName)
{
	if (nullptr == pNpc || nullptr == pClipName || g_IgnoredNpcs.contains(pNpc.get()))
		return;
	const std::string* pArchetypeId = Resolve_Archetype(*pNpc);
	if (nullptr == pArchetypeId)
		return;
	const BINDING_SET& Set = Ensure_Bindings(*pArchetypeId);
	NPC_STATE& State = g_NpcStates[pNpc.get()];
	State.pNpc = pNpc;
	State.strArchetypeId = *pArchetypeId;
	State.strClip = pClipName;
	State.fLastSeconds = -1.f;
	Prune_Spawned(State, true);
	State.Pending.clear();
	if (Set.bFailed)
		return;
	for (const EFFECT_V2_BINDING& Binding : Set.Bindings)
	{
		if (Binding.strClip == State.strClip)
			State.Pending.push_back({ Binding, false });
	}
}

void Client::CEffectV2Runtime::Tick_Npc(
	const std::shared_ptr<CNpc>& pNpc,
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext)
{
	for (auto Iterator = g_NpcStates.begin(); Iterator != g_NpcStates.end();)
	{
		if (Iterator->second.pNpc.expired())
		{
			Prune_Spawned(Iterator->second, true);
			Iterator = g_NpcStates.erase(Iterator);
			continue;
		}
		++Iterator;
	}
	if (nullptr == pNpc)
		return;
	const auto Found = g_NpcStates.find(pNpc.get());
	if (Found == g_NpcStates.end())
		return;
	NPC_STATE& State = Found->second;
	const shared_ptr<Engine::CModel> pModel = pNpc->Get_Model();
	if (nullptr == pModel)
		return;
	const uint32_t iClip = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentClip = pModel->Get_AnimationName(iClip);
	if (nullptr == pCurrentClip || State.strClip != pCurrentClip)
	{
		Prune_Spawned(State, true);
		State.Pending.clear();
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
			Spawn(State, Pending, pNpc, pDevice, pContext);
	}
	Prune_Spawned(State, false);
}

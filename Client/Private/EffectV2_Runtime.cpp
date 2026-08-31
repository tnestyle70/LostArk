#include "EffectV2_Runtime.h"
#include "ActorCatalog.h"
#include "EffectV2_Document.h"
#include "EffectV2_Object.h"
#include "GameInstance.h"
#include "Model.h"
#include "Npc.h"
#include "NpcPresentationAssetService.h"

#include <algorithm>
#include <cmath>
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

	struct GROUP_ENTRY final
	{
		bool_t bLoaded = false;
		bool_t bFailed = false;
		Client::EFFECT_V2_GROUP Group;
	};

	/* One leaf document waiting on a lane clock. Group children arrive here
	   already expanded: Binding.strEffectId is the child document, Local the
	   child offset/yaw, fStopSeconds the lane time of its stop (< 0 = none). */
	constexpr size_t NO_CHILD = static_cast<size_t>(-1);

	struct PENDING_SPAWN final
	{
		Client::EFFECT_V2_BINDING Binding;
		bool_t bSpawned = false;
		float4x4_t Local;
		f32_t fStopSeconds = -1.f;
		Client::EFFECT_V2_CHILD_STOP eStop = Client::EFFECT_V2_CHILD_STOP::KILL;
		size_t iChildIndex = NO_CHILD;
		float3_t vScale = { 1.f, 1.f, 1.f };
	};

	struct SPAWNED_EFFECT final
	{
		std::weak_ptr<Client::CEffectV2Object> pObject;
		bool_t bStopWithClip = false;
		bool_t bStageBound = false;
		f32_t fStopSeconds = -1.f;
		Client::EFFECT_V2_CHILD_STOP eStop = Client::EFFECT_V2_CHILD_STOP::KILL;
		bool_t bStopApplied = false;
		size_t iChildIndex = NO_CHILD;
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

	struct FREE_GROUP final
	{
		float4x4_t Pivot;
		f32_t fSeconds = 0.f;
		std::vector<PENDING_SPAWN> Pending;
		std::vector<SPAWNED_EFFECT> Spawned;
	};

	std::unordered_map<std::string, BINDING_SET> g_BindingSets;
	std::unordered_map<std::string, DOCUMENT_ENTRY> g_Documents;
	std::unordered_map<std::string, GROUP_ENTRY> g_Groups;
	std::map<std::wstring, std::string> g_ModelTagToArchetype;
	bool_t g_bModelTagMapBuilt = false;
	std::map<const Engine::CGameObject*, TARGET_STATE> g_TargetStates;
	std::set<const Engine::CGameObject*> g_IgnoredTargets;
	std::map<uint32_t, FREE_GROUP> g_FreeGroups;
	uint32_t g_iNextFreeGroupHandle = 1u;
	bool_t g_bPrototypeRegistered = false;
	std::string g_strLastError;

	void Report(const std::string& strMessage)
	{
		g_strLastError = strMessage;
		OutputDebugStringA(("[EffectV2Runtime] " + strMessage + "\n").c_str());
	}

	f32_t Ms_ToSeconds(const uint32_t iMs)
	{
		return static_cast<f32_t>(iMs) / 1000.f;
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

	const GROUP_ENTRY& Ensure_Group(const std::string& strGroupId)
	{
		GROUP_ENTRY& Entry = g_Groups[strGroupId];
		if (Entry.bLoaded)
			return Entry;
		Entry.bLoaded = true;
		std::string strError;
		if (!Client::CEffectV2Document::Load_GroupFile(strGroupId, Entry.Group, strError))
		{
			Entry.bFailed = true;
			Report("group rejected " + strGroupId + ": " + strError);
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

	float4x4_t Child_Local(const Client::EFFECT_V2_GROUP_CHILD& Child)
	{
		float4x4_t Local;
		XMStoreFloat4x4(&Local,
			XMMatrixScaling(Child.vScale.x, Child.vScale.y, Child.vScale.z) *
			XMMatrixRotationRollPitchYaw(
				XMConvertToRadians(Child.fPitchDegrees),
				XMConvertToRadians(Child.fYawDegrees),
				XMConvertToRadians(Child.fRollDegrees)) *
			XMMatrixTranslation(Child.vOffset.x, Child.vOffset.y, Child.vOffset.z));
		return Local;
	}

	matrix_t Binding_Local(const Client::EFFECT_V2_BINDING& Binding)
	{
		return XMMatrixRotationY(XMConvertToRadians(Binding.fYawDegrees)) *
			XMMatrixTranslation(Binding.vOffset.x, Binding.vOffset.y, Binding.vOffset.z);
	}

	/* Lane time at which a child stops: its own duration, capped by the
	   group duration; < 0 when neither is set. */
	f32_t Child_StopSeconds(
		const Client::EFFECT_V2_GROUP& Group,
		const Client::EFFECT_V2_GROUP_CHILD& Child,
		const uint32_t iBindingStartMs)
	{
		const f32_t fGroupStop = Group.iDurationMs > 0u ?
			Ms_ToSeconds(iBindingStartMs + Group.iDurationMs) : -1.f;
		f32_t fStop = Child.iDurationMs > 0u ?
			Ms_ToSeconds(iBindingStartMs + Child.iStartMs + Child.iDurationMs) : -1.f;
		if (fGroupStop >= 0.f && (fStop < 0.f || fGroupStop < fStop))
			fStop = fGroupStop;
		return fStop;
	}

	void Retarget_Pending(
		PENDING_SPAWN& Pending,
		const Client::EFFECT_V2_GROUP& Group,
		const size_t iChildIndex,
		const uint32_t iBindingStartMs)
	{
		const Client::EFFECT_V2_GROUP_CHILD& Child = Group.Children[iChildIndex];
		Pending.Binding.strGroupId.clear();
		Pending.Binding.strEffectId = Child.strEffectId;
		Pending.Binding.iStartMs = iBindingStartMs + Child.iStartMs;
		const float4x4_t ChildLocal = Child_Local(Child);
		XMStoreFloat4x4(&Pending.Local,
			XMLoadFloat4x4(&ChildLocal) * Binding_Local(Pending.Binding));
		Pending.eStop = Child.eStop;
		Pending.fStopSeconds = Child_StopSeconds(Group, Child, iBindingStartMs);
		Pending.iChildIndex = iChildIndex;
		Pending.vScale = Child.vScale;
	}

	/* One pending row per child, in child order, so a row's iChildIndex
	   stays valid while the tool edits the same group in place. */
	void Expand_Group(
		const Client::EFFECT_V2_GROUP& Group,
		const Client::EFFECT_V2_BINDING& Binding,
		std::vector<PENDING_SPAWN>& Out)
	{
		for (size_t iChild = 0u; iChild < Group.Children.size(); ++iChild)
		{
			PENDING_SPAWN Pending;
			Pending.Binding = Binding;
			Retarget_Pending(Pending, Group, iChild, Binding.iStartMs);
			Out.push_back(std::move(Pending));
		}
	}

	/* A document binding becomes one pending row; a group binding becomes
	   one row per child with the binding's own start added and the group
	   duration capping every child stop. A rejected group adds nothing. */
	void Expand_Binding(
		const Client::EFFECT_V2_BINDING& Binding,
		std::vector<PENDING_SPAWN>& Out)
	{
		if (Binding.strGroupId.empty())
		{
			PENDING_SPAWN Pending;
			Pending.Binding = Binding;
			XMStoreFloat4x4(&Pending.Local, Binding_Local(Binding));
			Out.push_back(std::move(Pending));
			return;
		}
		const GROUP_ENTRY& Entry = Ensure_Group(Binding.strGroupId);
		if (Entry.bFailed)
			return;
		Expand_Group(Entry.Group, Binding, Out);
	}

	/* Child scale rides on top of the document: the scale track is
	   multiplied per axis, particle sprite sizes by the X component. */
	void Apply_ChildScale(
		Client::CEffectV2Object& Object,
		const Client::EFFECT_V2_DOCUMENT& Document,
		const float3_t& vScale)
	{
		const Client::CEffectV2Object::PARAMS& Source = Document.Desc.Params;
		Client::CEffectV2Object::PARAMS& Params = Object.Params();
		Params.Scale.vStart = {
			Source.Scale.vStart.x * vScale.x,
			Source.Scale.vStart.y * vScale.y,
			Source.Scale.vStart.z * vScale.z };
		Params.Scale.vEnd = {
			Source.Scale.vEnd.x * vScale.x,
			Source.Scale.vEnd.y * vScale.y,
			Source.Scale.vEnd.z * vScale.z };
		Params.Particle.vSizeStart = {
			Source.Particle.vSizeStart.x * vScale.x,
			Source.Particle.vSizeStart.y * vScale.x };
		Params.Particle.vSizeEnd = {
			Source.Particle.vSizeEnd.x * vScale.x,
			Source.Particle.vSizeEnd.y * vScale.x };
	}

	void Spawn(
		PENDING_SPAWN& Pending,
		const float4x4_t& Pivot,
		const Client::EFFECT_V2_TARGET& FollowTarget,
		const bool_t bStageBound,
		std::vector<SPAWNED_EFFECT>& Spawned,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext)
	{
		Pending.bSpawned = true;
		const DOCUMENT_ENTRY& Entry = Ensure_Document(Pending.Binding.strEffectId);
		if (Entry.bFailed || !Ensure_Prototype(pDevice, pContext))
			return;
		CGameInstance& GameInstance = CGameInstance::Get();
		Client::CEffectV2Object::DESC Desc = Entry.Document.Desc;
		XMStoreFloat4x4(&Desc.PivotWorld,
			XMLoadFloat4x4(&Pending.Local) * XMLoadFloat4x4(&Pivot));
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
		if (Pending.Binding.bFollowBone && FollowTarget.Is_Valid())
		{
			pObject->Set_FollowTarget(
				FollowTarget, Pending.Binding.strBone, Pending.Binding.eRotation);
			pObject->Set_FollowLocal(Pending.Local);
		}
		if (NO_CHILD != Pending.iChildIndex)
			Apply_ChildScale(*pObject, Entry.Document, Pending.vScale);
		SPAWNED_EFFECT Effect;
		Effect.pObject = pObject;
		Effect.bStopWithClip = Pending.Binding.bStopWithClip;
		Effect.bStageBound = bStageBound;
		Effect.fStopSeconds = Pending.fStopSeconds;
		Effect.eStop = Pending.eStop;
		Effect.iChildIndex = Pending.iChildIndex;
		Spawned.push_back(std::move(Effect));
	}

	void Spawn_OnTarget(
		TARGET_STATE& State,
		PENDING_SPAWN& Pending,
		const Client::EFFECT_V2_TARGET_VIEW& View,
		const bool_t bStageBound,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext)
	{
		float4x4_t Pivot;
		if (!Client::CEffectV2Object::Resolve_TargetPivot(
			View, Pending.Binding.strBone, Pending.Binding.eRotation, Pivot))
		{
			Pending.bSpawned = true;
			Report("pivot bone not found: " + Pending.Binding.strBone +
				" for " + Pending.Binding.strEffectId);
			return;
		}
		Spawn(Pending, Pivot, State.Target, bStageBound, State.Spawned, pDevice, pContext);
	}

	/* Applies each child's stop once when the lane clock passes it. With
	   bFilterLane the clip lane only stops clip-bound effects and the stage
	   lane only stage-bound ones; free groups pass false. */
	void Apply_ChildStops(
		std::vector<SPAWNED_EFFECT>& Spawned,
		const f32_t fLaneSeconds,
		const bool_t bStageLane,
		const bool_t bFilterLane)
	{
		for (SPAWNED_EFFECT& Effect : Spawned)
		{
			if (Effect.bStopApplied || Effect.fStopSeconds < 0.f ||
				fLaneSeconds < Effect.fStopSeconds)
				continue;
			if (bFilterLane && Effect.bStageBound != bStageLane)
				continue;
			Effect.bStopApplied = true;
			const std::shared_ptr<Client::CEffectV2Object> pObject = Effect.pObject.lock();
			if (nullptr == pObject)
				continue;
			if (Client::EFFECT_V2_CHILD_STOP::DEACTIVATE == Effect.eStop)
				pObject->Stop_Emission();
			else
				pObject->Finish();
		}
	}

	/* bStopClipBound finishes stopWithClip effects of the clip lane,
	   bStopStageBound those of the stage lane; both lanes always drop what has
	   finished on its own. */
	void Prune_List(
		std::vector<SPAWNED_EFFECT>& Spawned,
		const bool_t bStopClipBound,
		const bool_t bStopStageBound)
	{
		CGameInstance& GameInstance = CGameInstance::Get();
		for (auto Iterator = Spawned.begin(); Iterator != Spawned.end();)
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
				Iterator = Spawned.erase(Iterator);
				continue;
			}
			++Iterator;
		}
	}

	void Prune_Spawned(
		TARGET_STATE& State,
		const bool_t bStopClipBound,
		const bool_t bStopStageBound = false)
	{
		Prune_List(State.Spawned, bStopClipBound, bStopStageBound);
	}

	void Kill_List(std::vector<SPAWNED_EFFECT>& Spawned)
	{
		for (SPAWNED_EFFECT& Effect : Spawned)
		{
			if (const std::shared_ptr<Client::CEffectV2Object> pObject = Effect.pObject.lock())
				pObject->Finish();
		}
		Prune_List(Spawned, false, false);
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
	std::vector<PENDING_SPAWN> Expanded;
	for (const EFFECT_V2_BINDING& Binding : Set.Bindings)
		Expand_Binding(Binding, Expanded);
	for (const PENDING_SPAWN& Pending : Expanded)
	{
		const DOCUMENT_ENTRY& Entry = Ensure_Document(Pending.Binding.strEffectId);
		if (Entry.bFailed)
			continue;
		std::string strError;
		if (FAILED(CEffectV2Object::Prewarm(pDevice, pContext, Entry.Document.Desc, strError)))
			Report("prewarm failed " + Pending.Binding.strEffectId + ": " + strError);
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
	g_Groups.clear();
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
			Expand_Binding(Binding, State.Pending);
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
					Expand_Binding(Binding, State.StagePending);
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
		if (fAgeSeconds >= Ms_ToSeconds(Pending.Binding.iStartMs))
			Spawn_OnTarget(State, Pending, View, true, pDevice, pContext);
	}
	Apply_ChildStops(State.Spawned, fAgeSeconds, true, true);
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
		if (fSeconds >= Ms_ToSeconds(Pending.Binding.iStartMs))
			Spawn_OnTarget(State, Pending, View, false, pDevice, pContext);
	}
	Apply_ChildStops(State.Spawned, fSeconds, false, true);
	Prune_Spawned(State, false);
}

uint32_t Client::CEffectV2Runtime::Play_Group(
	const EFFECT_V2_GROUP& Group,
	const float4x4_t& PivotWorld,
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext)
{
	if (!Ensure_Prototype(pDevice, pContext))
		return 0u;
	EFFECT_V2_BINDING Binding;
	Binding.bFollowBone = false;
	FREE_GROUP Lane;
	Lane.Pivot = PivotWorld;
	Expand_Group(Group, Binding, Lane.Pending);
	if (Lane.Pending.empty())
	{
		Report("group has no children: " + Group.strGroupId);
		return 0u;
	}
	const uint32_t iHandle = g_iNextFreeGroupHandle++;
	if (0u == g_iNextFreeGroupHandle)
		g_iNextFreeGroupHandle = 1u;
	g_FreeGroups.emplace(iHandle, std::move(Lane));
	return iHandle;
}

void Client::CEffectV2Runtime::Update_Group(const uint32_t iHandle, const EFFECT_V2_GROUP& Group)
{
	const auto Found = g_FreeGroups.find(iHandle);
	if (Found == g_FreeGroups.end())
		return;
	FREE_GROUP& Lane = Found->second;
	const matrix_t Pivot = XMLoadFloat4x4(&Lane.Pivot);
	for (PENDING_SPAWN& Pending : Lane.Pending)
	{
		if (Pending.iChildIndex >= Group.Children.size())
			continue;
		if (Pending.bSpawned)
		{
			Pending.Local = Child_Local(Group.Children[Pending.iChildIndex]);
			Pending.vScale = Group.Children[Pending.iChildIndex].vScale;
			continue;
		}
		Retarget_Pending(Pending, Group, Pending.iChildIndex, 0u);
	}
	for (size_t iChild = Lane.Pending.size(); iChild < Group.Children.size(); ++iChild)
	{
		PENDING_SPAWN Pending;
		Pending.Binding.bFollowBone = false;
		Retarget_Pending(Pending, Group, iChild, 0u);
		Lane.Pending.push_back(std::move(Pending));
	}
	for (SPAWNED_EFFECT& Effect : Lane.Spawned)
	{
		if (Effect.iChildIndex >= Group.Children.size())
			continue;
		const std::shared_ptr<CEffectV2Object> pObject = Effect.pObject.lock();
		if (nullptr == pObject)
			continue;
		const EFFECT_V2_GROUP_CHILD& Child = Group.Children[Effect.iChildIndex];
		const float4x4_t Local = Child_Local(Child);
		XMStoreFloat4x4(&pObject->PivotWorld(), XMLoadFloat4x4(&Local) * Pivot);
		const DOCUMENT_ENTRY& Entry = Ensure_Document(Child.strEffectId);
		if (!Entry.bFailed)
			Apply_ChildScale(*pObject, Entry.Document, Child.vScale);
		if (!Effect.bStopApplied)
		{
			Effect.fStopSeconds = Child_StopSeconds(Group, Child, 0u);
			Effect.eStop = Child.eStop;
		}
	}
}

void Client::CEffectV2Runtime::Set_GroupPivot(
	const uint32_t iHandle, const float4x4_t& PivotWorld)
{
	const auto Found = g_FreeGroups.find(iHandle);
	if (Found == g_FreeGroups.end())
		return;
	Found->second.Pivot = PivotWorld;
}

void Client::CEffectV2Runtime::Stop_Group(const uint32_t iHandle)
{
	const auto Found = g_FreeGroups.find(iHandle);
	if (Found == g_FreeGroups.end())
		return;
	Kill_List(Found->second.Spawned);
	g_FreeGroups.erase(Found);
}

f32_t Client::CEffectV2Runtime::Group_Seconds(const uint32_t iHandle)
{
	const auto Found = g_FreeGroups.find(iHandle);
	return Found != g_FreeGroups.end() ? Found->second.fSeconds : -1.f;
}

void Client::CEffectV2Runtime::Advance_FreeGroups(
	const f32_t fTimeDelta,
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext)
{
	if (!std::isfinite(fTimeDelta) || fTimeDelta < 0.f)
		return;
	for (auto Iterator = g_FreeGroups.begin(); Iterator != g_FreeGroups.end();)
	{
		FREE_GROUP& Group = Iterator->second;
		Group.fSeconds += fTimeDelta;
		for (PENDING_SPAWN& Pending : Group.Pending)
		{
			if (Pending.bSpawned)
				continue;
			if (Group.fSeconds >= Ms_ToSeconds(Pending.Binding.iStartMs))
			{
				Spawn(Pending, Group.Pivot, EFFECT_V2_TARGET{}, false, Group.Spawned,
					pDevice, pContext);
			}
		}
		Apply_ChildStops(Group.Spawned, Group.fSeconds, false, false);
		Prune_List(Group.Spawned, false, false);
		const bool_t bAllSpawned = std::all_of(Group.Pending.begin(), Group.Pending.end(),
			[](const PENDING_SPAWN& Pending) { return Pending.bSpawned; });
		if (bAllSpawned && Group.Spawned.empty())
		{
			Iterator = g_FreeGroups.erase(Iterator);
			continue;
		}
		++Iterator;
	}
}

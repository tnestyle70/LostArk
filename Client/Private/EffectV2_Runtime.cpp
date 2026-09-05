#include "EffectV2_Runtime.h"
#include "ActorCatalog.h"
#include "EffectV2_Catalog.h"
#include "EffectV2_Document.h"
#include "EffectV2_Object.h"
#include "GameInstance.h"
#include "Model.h"
#include "Npc.h"
#include "NpcPresentationAssetService.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <set>
#include <span>
#include <string_view>
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
		uint64_t iCatalogRevision = 0u;
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
		/* Epoch 0 is the first occurrence. ONCE/free/legacy lanes advance this
		   to one after their single attempt; EACH_LOOP advances once per source
		   loop. Spawn failures are attempts too, so they cannot flood every tick. */
		uint64_t iNextLoopEpoch = 0u;
		uint32_t iClockStartMs = 0u;
		uint32_t iChildStartMs = 0u;
		float4x4_t Local;
		f32_t fStopSeconds = -1.f;
		/* Group child stop in the group's own playback clock, relative to the
		   binding occurrence. It is converted to Stage wall time per epoch. */
		f32_t fRelativeStopSeconds = -1.f;
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
		std::vector<Client::EFFECT_V2_CLIP_OCCURRENCE_CLOCK> StageClocks;
		std::vector<SPAWNED_EFFECT> Spawned;
		std::shared_ptr<const Client::EFFECT_V2_CATALOG_SNAPSHOT>
			pAuthoringSnapshot;
	};

	struct FREE_GROUP final
	{
		float4x4_t Pivot;
		f32_t fSeconds = 0.f;
		f32_t fPlaybackRate = 1.f;
		bool_t bProductOwned = false;
		std::vector<PENDING_SPAWN> Pending;
		std::vector<SPAWNED_EFFECT> Spawned;
		std::shared_ptr<const Client::EFFECT_V2_CATALOG_SNAPSHOT> pSnapshot;
		std::string strFailure;
	};

	std::unordered_map<std::string, BINDING_SET> g_BindingSets;
	std::unordered_map<std::string, DOCUMENT_ENTRY> g_Documents;
	std::unordered_map<std::string, GROUP_ENTRY> g_Groups;
	std::map<std::wstring, std::string> g_ModelTagToArchetype;
	bool_t g_bModelTagMapBuilt = false;
	std::map<const Engine::CGameObject*, TARGET_STATE> g_TargetStates;
	std::set<const Engine::CGameObject*> g_IgnoredTargets;
	std::map<uint32_t, FREE_GROUP> g_FreeGroups;
	std::map<uint32_t, std::string> g_FreeGroupTerminalFailures;
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
		if (Client::EFFECT_V2_TARGET_KIND::PREVIEW_BODY == Target.eKind)
			return Target.strArchetypeId.empty() ? nullptr : &Target.strArchetypeId;
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
		if (strArchetypeId == VALTAN_ARCHETYPE_ID)
		{
			BINDING_SET& Set = g_BindingSets[strArchetypeId];
			Client::CEffectV2Catalog& Catalog = Client::CEffectV2Catalog::Get();
			std::shared_ptr<const Client::EFFECT_V2_CATALOG_SNAPSHOT> pSnapshot =
				Catalog.Get_Snapshot();
			if (nullptr == pSnapshot || !pSnapshot->Is_Ready())
			{
				/* A failed first read is cached until an explicit Load/Save invalidates
				   caches. Never rescan the authoring tree from every render tick. */
				if (Set.bLoaded)
					return Set;
				std::string strCatalogStatus;
				if (!Catalog.Reload_BossValtan(strCatalogStatus))
				{
					Set.bLoaded = true;
					Set.bFailed = true;
					Set.Bindings.clear();
					Report("bindings rejected for " + strArchetypeId +
						": " + strCatalogStatus);
					return Set;
				}
				pSnapshot = Catalog.Get_Snapshot();
			}

			if (nullptr != pSnapshot && pSnapshot->Is_Ready())
			{
				if (!Set.bLoaded ||
					Set.iCatalogRevision != pSnapshot->Get_Revision())
				{
					Set.bLoaded = true;
					Set.bFailed = false;
					Set.iCatalogRevision = pSnapshot->Get_Revision();
					Set.Bindings = pSnapshot->Get_BossValtanBindings();
				}
				return Set;
			}
		}

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

	f32_t Child_RelativeStopSeconds(
		const Client::EFFECT_V2_GROUP& Group,
		const Client::EFFECT_V2_GROUP_CHILD& Child)
	{
		const f32_t fGroupStop = Group.iDurationMs > 0u ?
			Ms_ToSeconds(Group.iDurationMs) : -1.f;
		f32_t fStop = Child.iDurationMs > 0u ?
			Ms_ToSeconds(Child.iStartMs + Child.iDurationMs) : -1.f;
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
		Pending.iClockStartMs = iBindingStartMs;
		Pending.iChildStartMs = Child.iStartMs;
		Pending.Binding.strGroupId.clear();
		Pending.Binding.strEffectId = Child.strEffectId;
		Pending.Binding.iStartMs = iBindingStartMs + Child.iStartMs;
		const float4x4_t ChildLocal = Child_Local(Child);
		XMStoreFloat4x4(&Pending.Local,
			XMLoadFloat4x4(&ChildLocal) * Binding_Local(Pending.Binding));
		Pending.eStop = Child.eStop;
		Pending.fStopSeconds = Child_StopSeconds(Group, Child, iBindingStartMs);
		Pending.fRelativeStopSeconds = Child_RelativeStopSeconds(Group, Child);
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
		std::vector<PENDING_SPAWN>& Out,
		const Client::EFFECT_V2_CATALOG_SNAPSHOT* const pSnapshot = nullptr)
	{
		if (Binding.strGroupId.empty())
		{
			PENDING_SPAWN Pending;
			Pending.Binding = Binding;
			Pending.iClockStartMs = Binding.iStartMs;
			XMStoreFloat4x4(&Pending.Local, Binding_Local(Binding));
			Out.push_back(std::move(Pending));
			return;
		}
		if (nullptr != pSnapshot)
		{
			const Client::EFFECT_V2_GROUP* const pGroup =
				pSnapshot->Find_Group(Binding.strGroupId);
			if (nullptr == pGroup)
			{
				Report("authoring group missing: " + Binding.strGroupId);
				return;
			}
			Expand_Group(*pGroup, Binding, Out);
			return;
		}
		const GROUP_ENTRY& Entry = Ensure_Group(Binding.strGroupId);
		if (!Entry.bFailed)
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
		const ComPtr<ID3D11DeviceContext>& pContext,
		const Client::EFFECT_V2_CATALOG_SNAPSHOT* const pSnapshot = nullptr,
		std::string* const pOutFailure = nullptr,
		const f32_t fPlaybackRate = 1.f,
		const f32_t fInitialElapsedSeconds = 0.f)
	{
		const auto Reject = [pOutFailure](const std::string& strFailure)
		{
			Report(strFailure);
			if (nullptr != pOutFailure && pOutFailure->empty())
				*pOutFailure = strFailure;
		};
		const Client::EFFECT_V2_DOCUMENT* pDocument = nullptr;
		if (nullptr != pSnapshot)
		{
			pDocument = pSnapshot->Find_Document(Pending.Binding.strEffectId);
			if (nullptr == pDocument)
			{
				Reject("authoring document missing: " + Pending.Binding.strEffectId);
				return;
			}
		}
		else
		{
			const DOCUMENT_ENTRY& Entry =
				Ensure_Document(Pending.Binding.strEffectId);
			if (!Entry.bFailed)
				pDocument = &Entry.Document;
			else if (nullptr != pOutFailure)
				*pOutFailure = "Product document unavailable: " +
					Pending.Binding.strEffectId;
		}
		if (nullptr == pDocument)
			return;
		if (!Ensure_Prototype(pDevice, pContext))
		{
			if (nullptr != pOutFailure)
			{
				*pOutFailure = g_strLastError.empty() ?
					"Effect V2 prototype creation failed." : g_strLastError;
			}
			return;
		}
		CGameInstance& GameInstance = CGameInstance::Get();
		Client::CEffectV2Object::DESC Desc = pDocument->Desc;
		XMStoreFloat4x4(&Desc.PivotWorld,
			XMLoadFloat4x4(&Pending.Local) * XMLoadFloat4x4(&Pivot));
		std::shared_ptr<CGameObject> pGameObject;
		if (FAILED(GameInstance.Add_GameObject_to_Layer(
			ETOUI(LEVEL::STATIC), EFFECT_PROTOTYPE_TAG,
			GameInstance.Get_CurrentLevelID(), EFFECT_LAYER_TAG,
			&Desc, &pGameObject)))
		{
			Reject("spawn failed for " + Pending.Binding.strEffectId + ": " +
				Client::CEffectV2Object::Last_Error());
			return;
		}
		const std::shared_ptr<Client::CEffectV2Object> pObject =
			std::dynamic_pointer_cast<Client::CEffectV2Object>(pGameObject);
		if (nullptr == pObject)
		{
			if (nullptr != pGameObject)
			{
				GameInstance.Remove_GameObject_from_Layer(
					GameInstance.Get_CurrentLevelID(), EFFECT_LAYER_TAG, pGameObject);
			}
			Reject("spawned object has the wrong Effect V2 type: " +
				Pending.Binding.strEffectId);
			return;
		}
		for (uint32_t iPart = 0u;
			iPart < pDocument->Parts.size() && iPart < pObject->Part_Count(); ++iPart)
		{
			pObject->Part_Visible(iPart) = pDocument->Parts[iPart].bVisible;
			if (!pDocument->Parts[iPart].strBaseAssetId.empty())
				(void)pObject->Set_PartBase(iPart, pDocument->Parts[iPart].strBaseAssetId);
		}
		if (!pDocument->strAnimationClip.empty())
		{
			for (uint32_t iClip = 0u; iClip < pObject->Animation_Count(); ++iClip)
			{
				const char_t* pName = pObject->Animation_Name(iClip);
				if (nullptr != pName && pDocument->strAnimationClip == pName)
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
			Apply_ChildScale(*pObject, *pDocument, Pending.vScale);
		pObject->Params().fPlayRate *= fPlaybackRate;
		if (fInitialElapsedSeconds > 0.f)
			pObject->Seek_ElapsedSeconds(fInitialElapsedSeconds);
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
		const ComPtr<ID3D11DeviceContext>& pContext,
		const f32_t fPlaybackRate = 1.f,
		const f32_t fInitialElapsedSeconds = 0.f)
	{
		float4x4_t Pivot;
		if (!Client::CEffectV2Object::Resolve_TargetPivot(
			View, Pending.Binding.strBone, Pending.Binding.eRotation, Pivot))
		{
			Report("pivot bone not found: " + Pending.Binding.strBone +
				" for " + Pending.Binding.strEffectId);
			return;
		}
		Spawn(
			Pending, Pivot, State.Target, bStageBound, State.Spawned,
			pDevice, pContext, State.pAuthoringSnapshot.get(), nullptr,
			fPlaybackRate, fInitialElapsedSeconds);
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

	void Reset_StageLane(TARGET_STATE& State)
	{
		for (SPAWNED_EFFECT& Effect : State.Spawned)
		{
			if (!Effect.bStageBound)
				continue;
			if (const std::shared_ptr<Client::CEffectV2Object> pObject =
					Effect.pObject.lock())
			{
				pObject->Finish();
			}
		}
		Prune_List(State.Spawned, false, false);
	}

	struct STAGE_SPAWN_CLOCK final
	{
		f32_t fStartSeconds = 0.f;
		f32_t fStopSeconds = -1.f;
		f32_t fPlaybackRate = 1.f;
	};

	const Client::EFFECT_V2_CLIP_OCCURRENCE_CLOCK* Find_OccurrenceClock(
		const std::span<const Client::EFFECT_V2_CLIP_OCCURRENCE_CLOCK> Clocks,
		const std::string_view strClipOccurrenceId)
	{
		const Client::EFFECT_V2_CLIP_OCCURRENCE_CLOCK* pFound = nullptr;
		for (const Client::EFFECT_V2_CLIP_OCCURRENCE_CLOCK& Clock : Clocks)
		{
			if (Clock.strClipOccurrenceId != strClipOccurrenceId)
				continue;
			if (nullptr != pFound)
				return nullptr;
			pFound = &Clock;
		}
		return pFound;
	}

	bool_t Resolve_StageSpawnClock(
		const PENDING_SPAWN& Pending,
		const std::span<const Client::EFFECT_V2_CLIP_OCCURRENCE_CLOCK> Clocks,
		const uint64_t iLoopEpoch,
		STAGE_SPAWN_CLOCK& Out,
		std::string& strOutError)
	{
		Out = {};
		strOutError.clear();
		if (Client::EFFECT_V2_CLOCK_BASIS::STAGE ==
			Pending.Binding.eClockBasis)
		{
			if (0u != iLoopEpoch)
			{
				strOutError = "a Stage-clock binding cannot repeat by clip epoch";
				return false;
			}
			const double fBindingStart =
				static_cast<double>(Pending.iClockStartMs) * 0.001;
			Out.fStartSeconds = static_cast<f32_t>(fBindingStart +
				static_cast<double>(Pending.iChildStartMs) * 0.001);
			if (Pending.fRelativeStopSeconds >= 0.f)
			{
				Out.fStopSeconds = static_cast<f32_t>(fBindingStart +
					static_cast<double>(Pending.fRelativeStopSeconds));
			}
			return std::isfinite(Out.fStartSeconds) &&
				(Out.fStopSeconds < 0.f || std::isfinite(Out.fStopSeconds));
		}

		const Client::EFFECT_V2_CLIP_OCCURRENCE_CLOCK* const pClock =
			Find_OccurrenceClock(Clocks, Pending.Binding.strClipOccurrenceId);
		if (nullptr == pClock)
		{
			strOutError = "clipOccurrenceId does not resolve exactly once: " +
				Pending.Binding.strClipOccurrenceId;
			return false;
		}
		if (!std::isfinite(pClock->fStageWallStartSeconds) ||
			pClock->fStageWallStartSeconds < 0.f ||
			!std::isfinite(pClock->fSourceStartSeconds) ||
			pClock->fSourceStartSeconds < 0.f ||
			!std::isfinite(pClock->fSourceDurationSeconds) ||
			pClock->fSourceDurationSeconds <= 0.f ||
			!std::isfinite(pClock->fLoopWallDurationSeconds) ||
			pClock->fLoopWallDurationSeconds <= 0.f ||
			!std::isfinite(pClock->fPlaybackRate) ||
			pClock->fPlaybackRate <= 0.f)
		{
			strOutError = "clip occurrence clock is invalid: " +
				Pending.Binding.strClipOccurrenceId;
			return false;
		}
		if (Client::EFFECT_V2_REPEAT_POLICY::EACH_LOOP ==
				Pending.Binding.eRepeatPolicy && !pClock->bLoop)
		{
			strOutError = "EACH_LOOP requires a looping clip occurrence: " +
				Pending.Binding.strClipOccurrenceId;
			return false;
		}
		if (!pClock->bLoop && 0u != iLoopEpoch)
		{
			strOutError = "a non-looping clip occurrence has only epoch zero: " +
				Pending.Binding.strClipOccurrenceId;
			return false;
		}

		constexpr double EpsilonSeconds = 0.000001;
		const double fCueSourceSeconds =
			static_cast<double>(Pending.iClockStartMs) * 0.001;
		const double fSourceBegin = pClock->fSourceStartSeconds;
		const double fSourceEnd = fSourceBegin + pClock->fSourceDurationSeconds;
		if (fCueSourceSeconds + EpsilonSeconds < fSourceBegin ||
			fCueSourceSeconds + EpsilonSeconds >= fSourceEnd)
		{
			strOutError = "binding startMs is outside its clip source window: " +
				Pending.Binding.strBindingId;
			return false;
		}

		const double fPlaybackRate = pClock->fPlaybackRate;
		const double fEpochWallStart =
			static_cast<double>(pClock->fStageWallStartSeconds) +
			static_cast<double>(iLoopEpoch) *
				static_cast<double>(pClock->fLoopWallDurationSeconds);
		const double fBindingWallStart = fEpochWallStart +
			(fCueSourceSeconds - fSourceBegin) / fPlaybackRate;
		const double fChildWallStart = fBindingWallStart +
			static_cast<double>(Pending.iChildStartMs) * 0.001 /
				fPlaybackRate;
		if (!std::isfinite(fChildWallStart) ||
			fChildWallStart > static_cast<double>(
				(std::numeric_limits<f32_t>::max)()))
		{
			strOutError = "binding occurrence wall clock overflowed: " +
				Pending.Binding.strBindingId;
			return false;
		}
		Out.fStartSeconds = static_cast<f32_t>(fChildWallStart);
		Out.fPlaybackRate = pClock->fPlaybackRate;

		if (Pending.fRelativeStopSeconds >= 0.f)
		{
			Out.fStopSeconds = static_cast<f32_t>(fBindingWallStart +
				static_cast<double>(Pending.fRelativeStopSeconds) /
					fPlaybackRate);
		}
		if (Client::EFFECT_V2_STOP_POLICY::CLIP_OCCURRENCE_END ==
			Pending.Binding.eStopPolicy)
		{
			const f32_t fOccurrenceEnd = static_cast<f32_t>(
				fEpochWallStart + pClock->fLoopWallDurationSeconds);
			if (Out.fStopSeconds < 0.f || fOccurrenceEnd < Out.fStopSeconds)
				Out.fStopSeconds = fOccurrenceEnd;
		}
		return Out.fStopSeconds < 0.f || std::isfinite(Out.fStopSeconds);
	}

	bool_t Resolve_LastDueLoopEpoch(
		const PENDING_SPAWN& Pending,
		const std::span<const Client::EFFECT_V2_CLIP_OCCURRENCE_CLOCK> Clocks,
		const f32_t fAgeSeconds,
		uint64_t& iOutEpoch,
		std::string& strOutError)
	{
		iOutEpoch = 0u;
		STAGE_SPAWN_CLOCK First;
		if (!Resolve_StageSpawnClock(Pending, Clocks, 0u, First, strOutError))
			return false;
		if (fAgeSeconds + 0.00001f < First.fStartSeconds)
			return true;
		if (Client::EFFECT_V2_REPEAT_POLICY::EACH_LOOP !=
			Pending.Binding.eRepeatPolicy)
		{
			return true;
		}
		const Client::EFFECT_V2_CLIP_OCCURRENCE_CLOCK* const pClock =
			Find_OccurrenceClock(Clocks, Pending.Binding.strClipOccurrenceId);
		if (nullptr == pClock)
		{
			strOutError = "clipOccurrenceId does not resolve exactly once: " +
				Pending.Binding.strClipOccurrenceId;
			return false;
		}
		const double fEpoch = std::floor(
			(static_cast<double>(fAgeSeconds) + 0.00001 -
				static_cast<double>(First.fStartSeconds)) /
			static_cast<double>(pClock->fLoopWallDurationSeconds));
		if (!std::isfinite(fEpoch) || fEpoch < 0.0 ||
			fEpoch >= static_cast<double>(
				(std::numeric_limits<uint64_t>::max)()))
		{
			strOutError = "binding loop epoch overflowed: " +
				Pending.Binding.strBindingId;
			return false;
		}
		iOutEpoch = static_cast<uint64_t>(fEpoch);
		return true;
	}

	void Sync_Stage_Impl(
		const Client::EFFECT_V2_TARGET& Target,
		const char_t* const pActionId,
		const f32_t fAgeSeconds,
		const std::span<const Client::EFFECT_V2_CLIP_OCCURRENCE_CLOCK>
			ClipOccurrences,
		std::shared_ptr<const Client::EFFECT_V2_CATALOG_SNAPSHOT> pSnapshot,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext)
	{
		if (!Target.Is_Valid() || g_IgnoredTargets.contains(Target.pKey))
			return;
		const std::string strActionId = nullptr != pActionId ? pActionId : "";
		const auto Found = g_TargetStates.find(Target.pKey);
		if (Found == g_TargetStates.end() && strActionId.empty())
			return;
		const std::string* const pArchetypeId = Resolve_Archetype(Target);
		if (nullptr == pArchetypeId)
			return;
		TARGET_STATE& State = Found != g_TargetStates.end() ?
			Found->second : g_TargetStates[Target.pKey];
		State.Target = Target;
		State.strArchetypeId = *pArchetypeId;
		const bool_t bSnapshotChanged = State.pAuthoringSnapshot != pSnapshot;
		const bool_t bStageChanged = State.strStage != strActionId;
		const bool_t bClocksChanged =
			State.StageClocks.size() != ClipOccurrences.size() ||
			!std::equal(
				State.StageClocks.begin(), State.StageClocks.end(),
				ClipOccurrences.begin(), ClipOccurrences.end());
		if (bStageChanged || bSnapshotChanged || bClocksChanged)
		{
			if (!bStageChanged && (bSnapshotChanged || bClocksChanged))
				Reset_StageLane(State);
			State.strStage = strActionId;
			State.fStageLastSeconds = -1.f;
			Prune_Spawned(State, false, true);
			State.StagePending.clear();
			State.StageClocks.assign(
				ClipOccurrences.begin(), ClipOccurrences.end());
			State.pAuthoringSnapshot = std::move(pSnapshot);
			if (!strActionId.empty())
			{
				if (nullptr != State.pAuthoringSnapshot &&
					State.pAuthoringSnapshot->Is_Ready())
				{
					for (const Client::EFFECT_V2_BINDING& Binding :
						State.pAuthoringSnapshot->Get_BossValtanBindings())
					{
						if (Binding.strStage == strActionId)
						{
							Expand_Binding(
								Binding, State.StagePending,
								State.pAuthoringSnapshot.get());
						}
					}
				}
				else if (nullptr == State.pAuthoringSnapshot)
				{
					const BINDING_SET& Set = Ensure_Bindings(*pArchetypeId);
					if (!Set.bFailed)
					{
						for (const Client::EFFECT_V2_BINDING& Binding : Set.Bindings)
						{
							if (Binding.strStage == strActionId)
								Expand_Binding(Binding, State.StagePending);
						}
					}
				}
			}
		}
		if (State.StagePending.empty() || !std::isfinite(fAgeSeconds) ||
			fAgeSeconds < 0.f)
		{
			Prune_Spawned(State, false, false);
			return;
		}
		if (fAgeSeconds + 0.00001f < State.fStageLastSeconds)
		{
			Reset_StageLane(State);
			for (PENDING_SPAWN& Pending : State.StagePending)
				Pending.iNextLoopEpoch = 0u;
		}
		State.fStageLastSeconds = fAgeSeconds;
		Client::EFFECT_V2_TARGET_VIEW View;
		if (!Client::CEffectV2Object::Resolve_TargetView(State.Target, View))
			return;
		for (PENDING_SPAWN& Pending : State.StagePending)
		{
			if ((std::numeric_limits<uint64_t>::max)() ==
				Pending.iNextLoopEpoch)
			{
				continue;
			}
			uint64_t iLastDueEpoch = 0u;
			std::string strClockError;
			if (!Resolve_LastDueLoopEpoch(
					Pending, ClipOccurrences, fAgeSeconds,
					iLastDueEpoch, strClockError))
			{
				Pending.iNextLoopEpoch =
					(std::numeric_limits<uint64_t>::max)();
				Report("binding clock rejected " +
					Pending.Binding.strBindingId + ": " + strClockError);
				continue;
			}
			if (iLastDueEpoch < Pending.iNextLoopEpoch)
				continue;

			constexpr uint64_t MAX_EPOCHS_PER_SYNC = 256u;
			if (iLastDueEpoch - Pending.iNextLoopEpoch >=
				MAX_EPOCHS_PER_SYNC)
			{
				Pending.iNextLoopEpoch =
					iLastDueEpoch - MAX_EPOCHS_PER_SYNC + 1u;
			}
			for (uint64_t iEpoch = Pending.iNextLoopEpoch;
				iEpoch <= iLastDueEpoch; ++iEpoch)
			{
				STAGE_SPAWN_CLOCK Clock;
				if (!Resolve_StageSpawnClock(
						Pending, ClipOccurrences, iEpoch,
						Clock, strClockError))
				{
					Pending.iNextLoopEpoch =
						(std::numeric_limits<uint64_t>::max)();
					Report("binding clock rejected " +
						Pending.Binding.strBindingId + ": " + strClockError);
					break;
				}
				if (fAgeSeconds + 0.00001f < Clock.fStartSeconds)
					break;
				Pending.iNextLoopEpoch =
					iEpoch == (std::numeric_limits<uint64_t>::max)() ?
						(std::numeric_limits<uint64_t>::max)() : iEpoch + 1u;
				/* A late snapshot after a finite clip/group stop must not flash a
				   just-created object for one frame. The epoch remains attempted. */
				if (Clock.fStopSeconds >= 0.f &&
					fAgeSeconds + 0.00001f >= Clock.fStopSeconds)
				{
					continue;
				}
				Pending.fStopSeconds = Clock.fStopSeconds;
				Spawn_OnTarget(
					State, Pending, View, true, pDevice, pContext,
					Clock.fPlaybackRate,
					(std::max)(0.f, fAgeSeconds - Clock.fStartSeconds));
				if (iEpoch == (std::numeric_limits<uint64_t>::max)())
					break;
			}
		}
		Apply_ChildStops(State.Spawned, fAgeSeconds, true, true);
		Prune_Spawned(State, false, false);
	}

	void Advance_FreeGroupLanes(
		const bool_t bProductOwned,
		const f32_t fTimeDelta,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext)
	{
		if (!std::isfinite(fTimeDelta) || fTimeDelta < 0.f)
			return;
		for (auto Iterator = g_FreeGroups.begin(); Iterator != g_FreeGroups.end();)
		{
			FREE_GROUP& Group = Iterator->second;
			if (Group.bProductOwned != bProductOwned)
			{
				++Iterator;
				continue;
			}
			Group.fSeconds += fTimeDelta * Group.fPlaybackRate;
			for (PENDING_SPAWN& Pending : Group.Pending)
			{
				if (0u != Pending.iNextLoopEpoch)
					continue;
				if (Group.fSeconds >= Ms_ToSeconds(Pending.Binding.iStartMs))
				{
					Pending.iNextLoopEpoch = 1u;
					std::string SpawnFailure;
					Spawn(Pending, Group.Pivot, Client::EFFECT_V2_TARGET{}, false,
						Group.Spawned, pDevice, pContext, Group.pSnapshot.get(),
						&SpawnFailure, Group.fPlaybackRate, 0.f);
					if (Group.strFailure.empty() && !SpawnFailure.empty())
						Group.strFailure = std::move(SpawnFailure);
				}
			}
			Apply_ChildStops(Group.Spawned, Group.fSeconds, false, false);
			Prune_List(Group.Spawned, false, false);
			const bool_t bAllSpawned = std::all_of(
				Group.Pending.begin(), Group.Pending.end(),
				[](const PENDING_SPAWN& Pending)
				{ return 0u != Pending.iNextLoopEpoch; });
			if (bAllSpawned && Group.Spawned.empty())
			{
				if (!Group.strFailure.empty())
				{
					g_FreeGroupTerminalFailures[Iterator->first] =
						std::move(Group.strFailure);
				}
				Iterator = g_FreeGroups.erase(Iterator);
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

bool_t Client::CEffectV2Runtime::Prewarm_Group(
	const EFFECT_V2_GROUP& Group,
	std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pSnapshot,
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext)
{
	if (nullptr == pSnapshot || !pSnapshot->Is_Ready())
		return false;
	for (const EFFECT_V2_GROUP_CHILD& Child : Group.Children)
	{
		const EFFECT_V2_DOCUMENT* pDocument =
			pSnapshot->Find_Document(Child.strEffectId);
		std::string strError;
		if (nullptr == pDocument || FAILED(CEffectV2Object::Prewarm(
				pDevice, pContext, pDocument->Desc, strError)))
		{
			Report(nullptr == pDocument ?
				"group prewarm document missing: " + Child.strEffectId :
				"group prewarm failed " + Child.strEffectId + ": " + strError);
			return false;
		}
	}
	return Ensure_Prototype(pDevice, pContext);
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
	for (auto& [pTarget, State] : g_TargetStates)
	{
		(void)pTarget;
		State.strClip.clear();
		State.fLastSeconds = -1.f;
		State.Pending.clear();
		State.strStage.clear();
		State.fStageLastSeconds = -1.f;
		State.StagePending.clear();
		State.StageClocks.clear();
		State.pAuthoringSnapshot.reset();
	}
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
	/* Valtan's format-v2 bindings are keyed by stable clip occurrence ID and
	   action ID, never by the reusable model clip name supplied here. Its typed
	   Stage path is Sync_Stage; retain this lane only for NPC/legacy bindings. */
	if (EFFECT_V2_TARGET_KIND::VALTAN == Target.eKind)
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
	const std::span<const EFFECT_V2_CLIP_OCCURRENCE_CLOCK> ClipOccurrences,
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext)
{
	Sync_Stage_Impl(
		Target, pActionId, fAgeSeconds, ClipOccurrences,
		nullptr, pDevice, pContext);
}

void Client::CEffectV2Runtime::Sync_Stage(
	const EFFECT_V2_TARGET& Target,
	const char_t* pActionId,
	const f32_t fAgeSeconds,
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext)
{
	Sync_Stage(Target, pActionId, fAgeSeconds,
		std::span<const EFFECT_V2_CLIP_OCCURRENCE_CLOCK>{},
		pDevice, pContext);
}

void Client::CEffectV2Runtime::Sync_StageAuthoring(
	const EFFECT_V2_TARGET& Target,
	const char_t* pActionId,
	const f32_t fAgeSeconds,
	const std::span<const EFFECT_V2_CLIP_OCCURRENCE_CLOCK> ClipOccurrences,
	std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pSnapshot,
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext)
{
	Sync_Stage_Impl(
		Target, pActionId, fAgeSeconds, ClipOccurrences, std::move(pSnapshot),
		pDevice, pContext);
}

void Client::CEffectV2Runtime::Sync_StageAuthoring(
	const EFFECT_V2_TARGET& Target,
	const char_t* pActionId,
	const f32_t fAgeSeconds,
	std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pSnapshot,
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext)
{
	Sync_StageAuthoring(Target, pActionId, fAgeSeconds,
		std::span<const EFFECT_V2_CLIP_OCCURRENCE_CLOCK>{},
		std::move(pSnapshot), pDevice, pContext);
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
			Pending.iNextLoopEpoch = 0u;
	}
	State.fLastSeconds = fSeconds;
	for (PENDING_SPAWN& Pending : State.Pending)
	{
		if (0u != Pending.iNextLoopEpoch)
			continue;
		if (fSeconds >= Ms_ToSeconds(Pending.Binding.iStartMs))
		{
			Pending.iNextLoopEpoch = 1u;
			Spawn_OnTarget(State, Pending, View, false, pDevice, pContext);
		}
	}
	Apply_ChildStops(State.Spawned, fSeconds, false, true);
	Prune_Spawned(State, false);
}

uint32_t Client::CEffectV2Runtime::Play_Group(
	const EFFECT_V2_GROUP& Group,
	std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pSnapshot,
	const float4x4_t& PivotWorld,
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext)
{
	EFFECT_V2_GROUP_PLAYBACK_DESC Playback;
	Playback.PivotWorld = PivotWorld;
	return Play_Group(Group, std::move(pSnapshot), Playback, pDevice, pContext);
}

uint32_t Client::CEffectV2Runtime::Play_Group(
	const EFFECT_V2_GROUP& Group,
	std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pSnapshot,
	const EFFECT_V2_GROUP_PLAYBACK_DESC& Playback,
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext)
{
	if (!std::isfinite(Playback.fInitialAgeSeconds) ||
		Playback.fInitialAgeSeconds < 0.f ||
		!std::isfinite(Playback.fPlaybackRate) ||
		Playback.fPlaybackRate <= 0.f || Playback.fPlaybackRate > 16.f)
	{
		Report("group playback has an invalid age/rate: " + Group.strGroupId);
		return 0u;
	}
	if (nullptr == pSnapshot || !pSnapshot->Is_Ready())
	{
		Report("group preview requires one ready typed authoring snapshot: " +
			Group.strGroupId);
		return 0u;
	}
	for (const EFFECT_V2_GROUP_CHILD& Child : Group.Children)
	{
		if (nullptr == pSnapshot->Find_Document(Child.strEffectId))
		{
			Report("group preview child is absent from the staged authoring snapshot: " +
				Group.strGroupId + " -> " + Child.strEffectId);
			return 0u;
		}
	}
	if (!Ensure_Prototype(pDevice, pContext))
		return 0u;
	EFFECT_V2_BINDING Binding;
	Binding.bFollowBone = false;
	FREE_GROUP Lane;
	Lane.Pivot = Playback.PivotWorld;
	Lane.fPlaybackRate = Playback.fPlaybackRate;
	Lane.fSeconds = Playback.fInitialAgeSeconds * Playback.fPlaybackRate;
	Lane.bProductOwned = Playback.bProductOwned;
	Lane.pSnapshot = std::move(pSnapshot);
	Expand_Group(Group, Binding, Lane.Pending);
	if (Lane.Pending.empty())
	{
		Report("group has no children: " + Group.strGroupId);
		return 0u;
	}
	const uint32_t iHandle = g_iNextFreeGroupHandle++;
	if (0u == g_iNextFreeGroupHandle)
		g_iNextFreeGroupHandle = 1u;
	g_FreeGroupTerminalFailures.erase(iHandle);
	g_FreeGroups.emplace(iHandle, std::move(Lane));
	/* Prime zero-time and late-snapshot children immediately. This advances no
	   other lane clock and preserves one global frame advance in MainApp. */
	Advance_FreeGroupLanes(Playback.bProductOwned, 0.f, pDevice, pContext);
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
		if (0u != Pending.iNextLoopEpoch)
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
		const EFFECT_V2_DOCUMENT* const pDocument = nullptr == Lane.pSnapshot ?
			nullptr : Lane.pSnapshot->Find_Document(Child.strEffectId);
		if (nullptr != pDocument)
		{
			Apply_ChildScale(*pObject, *pDocument, Child.vScale);
		}
		else if (Lane.strFailure.empty())
		{
			Lane.strFailure =
				"live Group edit references a child outside its staged authoring snapshot: " +
				Child.strEffectId;
			Report(Lane.strFailure);
		}
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
	g_FreeGroupTerminalFailures.erase(iHandle);
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

bool_t Client::CEffectV2Runtime::Consume_GroupFailure(
	const uint32_t iHandle, std::string& strOutFailure)
{
	strOutFailure.clear();
	const auto Active = g_FreeGroups.find(iHandle);
	if (Active != g_FreeGroups.end() && !Active->second.strFailure.empty())
	{
		strOutFailure = std::move(Active->second.strFailure);
		Active->second.strFailure.clear();
		return true;
	}
	const auto Terminal = g_FreeGroupTerminalFailures.find(iHandle);
	if (Terminal == g_FreeGroupTerminalFailures.end())
		return false;
	strOutFailure = std::move(Terminal->second);
	g_FreeGroupTerminalFailures.erase(Terminal);
	return true;
}

void Client::CEffectV2Runtime::Advance_FreeGroups(
	const f32_t fTimeDelta,
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext)
{
	Advance_FreeGroupLanes(false, fTimeDelta, pDevice, pContext);
}

void Client::CEffectV2Runtime::Advance_ProductGroups(
	const f32_t fTimeDelta,
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext)
{
	Advance_FreeGroupLanes(true, fTimeDelta, pDevice, pContext);
}

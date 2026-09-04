#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "ClientReplicationEvent.h"
#include "CombatObjectProjectionRuntime.h"
#include "EstherActionSoundCueDocument.h"
#include "NetObjectRegistry.h"
#include "NpcPlacementPresentationService.h"
#include "MonsterPresentationContract.h"
#include "WorldDestructionProjectionDocument.h"
#include "WorldDestructionProjectionRuntime.h"
#include "ReplicatedPlayerHealth.h"
#include "CombatDebugVisibility.h"

#include <chrono>
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

//Network Event瑜??ㅼ젣 Engine GameObject ?앹꽦 ?쒓굅濡?踰덉뿭?섎뒗 ???섎굹??main-thread 寃쎄퀎

namespace Client
{
	class CCharacter;
	class CNpc;
	class CValtan;
	class CDeployPropRuntime;
	struct VALTAN_PATTERN_SOUND_SOURCE_RECEIPT;

	/* Pure fail-closed admission state used by the native authoring harness and
	   the replicated primary Valtan consumer. Source saves may succeed while a
	   live presentation reload fails; that failure must remain visible and keep
	   Complete Play closed until the same authoritative consumer admits a later
	   reload (or the world is reset). */
	class CPrimaryValtanPresentationFreshnessGate final
	{
	public:
		void Admit(
			const LostArk::Shared::GameplayDataRevision& Revision,
			const std::string_view strStatus = {})
		{
			if (!Revision.Is_Valid())
			{
				Reject(
					"Authoritative primary Valtan presentation admission supplied no immutable revision.");
				return;
			}
			m_isFresh = true;
			m_Revision = Revision;
			m_strStatus.assign(strStatus);
		}

		void Reject(const std::string_view strDiagnostic)
		{
			m_isFresh = false;
			m_Revision = {};
			m_strStatus = strDiagnostic.empty() ?
				"Authoritative primary Valtan presentation reload failed." :
				std::string(strDiagnostic);
		}

		bool_t Can_Play(
			const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
			std::string& strOutStatus) const
		{
			if (ExpectedRevision.Is_Valid() && m_isFresh &&
				m_Revision == ExpectedRevision)
			{
				strOutStatus.clear();
				return true;
			}
			strOutStatus = !ExpectedRevision.Is_Valid() ?
				"Complete Play is blocked because no valid Server-active presentation revision was supplied." :
				(m_isFresh ?
					"Complete Play is blocked because the authoritative primary Valtan presentation cache belongs to a different immutable revision." :
					"Complete Play is blocked because the authoritative primary Valtan presentation cache is stale. " +
						m_strStatus);
			return false;
		}

	private:
		bool_t m_isFresh = false;
		LostArk::Shared::GameplayDataRevision m_Revision{};
		std::string m_strStatus;
	};

	// Level presentation이 읽는 복제 player snapshot이다. NetEntityId가 identity이며
	// nickname과 class는 Server spawn record에서만 오고 Character 수명은 소유하지 않는다.
	struct REPLICATED_PLAYER_VIEW
	{
		LostArk::Shared::PLAYER_ID iPlayerId =
			LostArk::Shared::INVALID_PLAYER_ID;
		LostArk::Shared::NET_ENTITY_ID iNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		std::string strNickname;
		bool_t isLocal = false;
		std::weak_ptr<CCharacter> pCharacter;
	};

	struct DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_VIEW final
	{
		std::uint64_t iGeneration = 0u;
		LostArk::Shared::NET_ENTITY_ID iNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		std::uint32_t iServerTick = 0u;
	};

	enum class DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT : uint8_t
	{
		NO_PENDING,
		COMMITTED,
		RECOVERED_FAILURE,
		FATAL_FAILURE
	};

	struct VALTAN_PRESENTATION_STATE final
	{
		bool_t isValid = false;
		LostArk::Shared::NET_ENTITY_ID iNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint32_t iServerTick = 0u;
		LostArk::Shared::WORLD_ENTITY_ACTION eAction =
			LostArk::Shared::WORLD_ENTITY_ACTION::END;
		std::string strArchetypeId;
		std::string strPatternId;
		std::string strActionId;
		std::uint32_t iPatternSequence = 0u;
		std::uint32_t iPatternStageIndex = 0u;
		LostArk::Shared::NET_ENTITY_ID iPatternTargetNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		LostArk::Shared::PORTAL_RUSH_ROUTE_SNAPSHOT PortalRushRoute;
		std::uint32_t iActionStartTick = 0u;
		LostArk::Shared::BOSS_COMBAT_SNAPSHOT BossCombat;
		float3_t vPosition = {};
		f32_t fYawDegrees = 0.f;
	};

	struct NPC_ACTION_EDGE_STATE final
	{
		std::string strLastActionId;
		std::uint32_t iLastActionStartTick = 0u;
		bool_t hasAppliedActionEdge = false;
	};

	struct NPC_ACTION_PLAYBACK_REQUEST final
	{
		std::string strClipName;
		bool_t isLoop = true;
		f32_t fPlaybackRate = 1.f;
		f32_t fBlendSeconds = 0.12f;
	};

	struct NPC_ACTION_PLAYBACK_RESULT final
	{
		bool_t isPrimaryPlayed = false;
		bool_t isIdleFallbackAttempted = false;
		bool_t isIdleFallbackPlayed = false;
	};

	/* Pure action-edge projection shared by product replication and its focused
	contract harness. Model playback stays injected so one bad optional binding
	falls back only that NPC to idle. Esther chains remain in the existing
	replication branch and are never flattened through this helper. */
	class CNpcActionPresentationRuntime final
	{
	public:
		static bool_t Is_NewActionEdge(
			const NPC_ACTION_EDGE_STATE& state,
			const std::string_view actionId,
			const std::uint32_t iActionStartTick)
		{
			return !state.hasAppliedActionEdge ||
				state.strLastActionId != actionId ||
				state.iLastActionStartTick != iActionStartTick;
		}

		static NPC_ACTION_PLAYBACK_REQUEST Resolve_Playback(
			const std::string_view actionId,
			const std::string& resolvedIdleClip,
			const NPC_PLACEMENT_PRESENTATION_ENTRY& placement,
			const std::map<
				std::string,
				std::vector<std::string>,
				std::less<>>* pLegacyActionClips)
		{
			NPC_ACTION_PLAYBACK_REQUEST request{};
			if (actionId.empty() || actionId == "npc.idle")
			{
				request.strClipName = resolvedIdleClip;
				return request;
			}
			if (actionId == "npc.move.walk")
			{
				request.strClipName = !placement.strWalkClip.empty() ?
					placement.strWalkClip : resolvedIdleClip;
				return request;
			}

			const auto authored = placement.ActionBindings.find(actionId);
			if (authored != placement.ActionBindings.end())
			{
				request.strClipName = authored->second.strClipName;
				request.isLoop = authored->second.isLoop;
				request.fPlaybackRate = authored->second.fPlaybackRate;
				request.fBlendSeconds = authored->second.fBlendSeconds;
				return request;
			}
			if (nullptr != pLegacyActionClips)
			{
				const auto legacy = pLegacyActionClips->find(actionId);
				if (legacy != pLegacyActionClips->end() &&
					!legacy->second.empty())
				{
					request.strClipName = legacy->second.front();
					request.isLoop = false;
				}
			}
			return request;
		}

		template <typename TRY_PLAY_ACTION, typename TRY_PLAY_IDLE>
		static NPC_ACTION_PLAYBACK_RESULT Apply_Playback(
			const NPC_ACTION_PLAYBACK_REQUEST& request,
			const std::string& resolvedIdleClip,
			TRY_PLAY_ACTION&& tryPlayAction,
			TRY_PLAY_IDLE&& tryPlayIdle,
			std::string& inOutCurrentClip)
		{
			NPC_ACTION_PLAYBACK_RESULT result{};
			result.isPrimaryPlayed = !request.strClipName.empty() &&
				std::forward<TRY_PLAY_ACTION>(tryPlayAction)(request);
			if (result.isPrimaryPlayed)
			{
				inOutCurrentClip = request.strClipName;
				return result;
			}

			result.isIdleFallbackAttempted = true;
			result.isIdleFallbackPlayed =
				std::forward<TRY_PLAY_IDLE>(tryPlayIdle)(0.12f);
			inOutCurrentClip = resolvedIdleClip;
			return result;
		}

		static void Commit_ActionEdge(
			NPC_ACTION_EDGE_STATE& state,
			const std::string_view actionId,
			const std::uint32_t iActionStartTick)
		{
			state.strLastActionId.assign(actionId);
			state.iLastActionStartTick = iActionStartTick;
			state.hasAppliedActionEdge = true;
		}
	};

	class CClientReplication final
	{
	public:
		//replication??character瑜??대뒓 layer? prototype???앹꽦?댁빞 ?섎뒗吏瑜??꾨떖?쒕떎.
		//rpelication??level baren???섎뱶肄붾뵫???꾩슂媛 ?녿떎.
		struct DESC
		{
			ComPtr<ID3D11Device> pDevice;
			ComPtr<ID3D11DeviceContext> pContext;
			std::uint32_t iPrototypeLevelIndex = 0;
			std::uint32_t iLayerLevelIndex = 0;
			/* Stable LevelRegistry area ID. Replication resolves the navigation
			   prototype that the Loader registered for this exact area and gives it
			   only to the locally controlled Character. Ground-target preview may
			   reject an invalid cell, while the Server still owns final admission. */
			std::string strMapAreaId;
			std::wstring strPlayerLayerTag;
			std::wstring strWorldEntityLayerTag;
			CDeployPropRuntime* pDeployPropRuntime = nullptr;
			const CWorldDestructionProjectionDocument*
				pWorldDestructionProjection = nullptr;
			bool_t bDeferLocalCharacterClassReplacement = false;
			/* Optional main-thread presentation edge. The reliable Server
			despawn remains authoritative; Levels may attach non-gameplay
			presentation such as a BGM transition without parsing packets. */
			std::function<void(
				std::string_view placementId,
				std::string_view archetypeId)> onWorldEntityDespawned;
		};

	public:
		bool Initialize(const DESC& desc);
		bool Update();
		bool Has_PendingConnectionLoss() const;
		bool Has_FatalWorldDestructionFailure() const
		{
			return m_hasFatalWorldDestructionFailure;
		}
		void Acknowledge_ConnectionLoss();
		void Reset();
		bool Has_WorldEntity(std::string_view archetypeId) const;
		bool Try_Consume_PresentationFailure(std::string& outStatus);
		/* Authoring reloads target the primary replicated Server-authoritative
		   Valtan, never only the Development preview returned by
		   CAnimationTargetService. A rejected active reload latches the freshness
		   gate consumed by Boss Tool Complete Play. */
		bool_t Reload_PrimaryValtanPresentationAuthoring(
			const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
			std::string& strOutStatus);
		bool_t Reload_PrimaryValtanCombatObjectSoundCues(
			const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
			std::string& strOutStatus);
		bool_t Can_Play_PrimaryValtanPresentation(
			const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
			std::string& strOutStatus) const;
		bool_t Get_PrimaryValtanPatternSoundSourceReceipt(
			VALTAN_PATTERN_SOUND_SOURCE_RECEIPT& OutReceipt,
			std::string& strOutStatus) const;
		bool Try_Consume_WorldDestructionLiveEvent(
			LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE& outEvent);
#ifdef _DEBUG
		/* F1 Developer Tools writes one process-global snapshot. Every Level-owned
		   replication consumes it by revision, including while disconnected. */
		static COMBAT_DEBUG_VISIBILITY_SNAPSHOT
			Get_GlobalCombatDebugVisibility();
		static void Set_GlobalCombatDebugVisibility(
			const COMBAT_DEBUG_VISIBILITY_SNAPSHOT& Visibility);
#endif

		std::shared_ptr<CCharacter> Get_LocalCharacter() const;
		bool_t Try_Get_DeferredLocalCharacterClassReplacement(
			DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_VIEW& OutView) const;
		DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT
			Commit_DeferredLocalCharacterClassReplacement();
		void Collect_PlayerViews(
			std::vector<REPLICATED_PLAYER_VIEW>& outPlayers) const;
		const VALTAN_PRESENTATION_STATE& Get_ValtanPresentationState() const
		{
			return m_ValtanPresentationState;
		}
		uint64_t Get_WorldDestructionPresentationGeneration() const
		{
			return m_iWorldDestructionPresentationGeneration;
		}
		bool_t Is_WorldDestructionSynchronized() const
		{
			return m_WorldDestructionProjectionRuntime.Is_Synchronized();
		}
		uint32_t Get_WorldDestructionEncounterEpoch() const
		{
			return m_WorldDestructionProjectionRuntime.Get_EncounterEpoch();
		}
		uint32_t Get_WorldDestructionServerTick() const
		{
			return m_WorldDestructionProjectionRuntime.Get_ServerTick();
		}
		const std::vector<LostArk::Shared::WORLD_DESTRUCTION_STATE_WIRE>&
		Get_WorldDestructionGroupStates() const
		{
			return m_WorldDestructionProjectionRuntime.Get_GroupStates();
		}
		/* Last Server-owned collision/navigation counters. Read-only; the Debug
		   panel reports them instead of deriving passage from wall states. */
		const LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS&
		Get_WorldDestructionDiagnostics() const
		{
			return m_WorldDestructionDiagnostics;
		}
		/* Live pillar slot states as the Server last published them. The four
		   slots repeat, so this is a replace-in-full state, never a log. */
		const LostArk::Shared::S2C_ENCOUNTER_PROP_SYNC&
		Get_EncounterPropState() const
		{
			return m_EncounterPropState;
		}
		/* Debug-only inventory slice. Replace-in-full, same as
		   Get_EncounterPropState: the Server always answers with the whole
		   current inventory, never a delta. */
		const LostArk::Shared::S2C_INVENTORY_SNAPSHOT&
		Get_InventoryState() const
		{
			return m_InventoryState;
		}
		/* One-shot: true exactly once, the frame a party invite arrives (or a
		   newer one silently replaced an unconsumed older one -- see
		   Apply_PartyInviteReceived). */
		bool Try_Consume_PartyInviteReceived(
			LostArk::Shared::S2C_PARTY_INVITE_RECEIVED& outInvite);
		/* Replace-in-full, same shape as Get_EncounterPropState/
		   Get_InventoryState. Empty Members means "not in a party". */
		const LostArk::Shared::S2C_PARTY_ROSTER&
		Get_PartyRoster() const
		{
			return m_PartyRoster;
		}
		const CReplicatedPlayerHealth& Get_PlayerHealth() const { return m_PlayerHealth; }
		/* Server-decided world sequence starts, in arrival order. The caller
		   takes them so one start is never played twice. */
		std::vector<std::string> Consume_WorldSequencePlays()
		{
			return std::move(m_PendingWorldSequencePlays);
		}
		bool Try_Consume_PartyTransferResult(
			LostArk::Shared::S2C_PARTY_TRANSFER_RESULT& outResult);
		/* Head-bubble text for whoever last chatted, while their line is still
		   within CHAT_BUBBLE_DURATION of arriving -- false (text left
		   untouched) once it has aged out, so the renderer only ever draws a
		   bubble that is still "live". */
		bool Try_Get_ActiveChatBubble(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			std::string& outText) const;

	private:
#ifdef _DEBUG
		struct COMBAT_OBJECT_HIT_AREA_DEBUG final
		{
			std::string strHitShape;
			bool_t bContact = false;
			f32_t fOuterRadiusM = 0.f;
			f32_t fInnerRadiusM = 0.f;
			f32_t fAngleDegrees = 0.f;
			f32_t fLengthM = 0.f;
			f32_t fHalfWidthM = 0.f;
			std::uint32_t iAtMs = 0u;
			std::uint32_t iRepeatCount = 0u;
			std::uint32_t iRepeatIntervalMs = 0u;
		};
		void Sync_GlobalCombatDebugVisibility();
		void Apply_CombatDebugVisibility(
			const COMBAT_DEBUG_VISIBILITY_SNAPSHOT& Visibility);
		bool_t Load_CombatObjectHitAreaDebug(std::string& strOutStatus);
		void Draw_CombatObjectHitAreaDebug();
#endif
		bool Create_Character(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass,
			std::string_view nickName,
			const float3_t& position,
			f32_t yawDegrees,
			bool_t isLocallyControlled,
			std::shared_ptr<CCharacter>& outCharacter);
		bool Apply_Spawn(
			const LostArk::Shared::S2C_PLAYER_SPAWNED& spawned);

		bool Apply_Despawn(
			const LostArk::Shared::S2C_PLAYER_DESPAWNED& despawned);
		bool Apply_WorldEntitySpawn(
			const LostArk::Shared::S2C_WORLD_ENTITY_SPAWNED& spawned);
		bool Apply_WorldEntityDespawn(
			const LostArk::Shared::S2C_WORLD_ENTITY_DESPAWNED& despawned);
		bool_t Resolve_PrimaryValtan(
			std::string_view strArchetypeId,
			LostArk::Shared::NET_ENTITY_ID iOwnerBossNetEntityId,
			std::shared_ptr<CValtan>& pOutValtan,
			std::string& strOutStatus) const;
		void Remove_DependentBossPresentations(
			LostArk::Shared::NET_ENTITY_ID ownerBossNetEntityId);
		bool Apply_CombatObjectSpawn(
			const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawned);
		bool Apply_CombatObjectPresentationEvent(
			const LostArk::Shared::S2C_COMBAT_OBJECT_PRESENTATION_EVENT& event);
		bool Apply_CombatObjectDespawn(
			const LostArk::Shared::S2C_COMBAT_OBJECT_DESPAWNED& despawned);
		//snapshot??netentityid瑜??ㅼ젣 client character濡??댁꽍?섎뒗 ?⑥닔
		bool Apply_WorldSnapshot(
			const LostArk::Shared::S2C_WORLD_SNAPSHOT& snapshot);
		bool Apply_WorldDestructionFullSync(
			const LostArk::Shared::S2C_WORLD_DESTRUCTION_FULL_SYNC& fullSync);
		bool Apply_WorldDestructionDelta(
			const LostArk::Shared::S2C_WORLD_DESTRUCTION_DELTA& delta);
		bool Apply_EncounterPropSync(
			const LostArk::Shared::S2C_ENCOUNTER_PROP_SYNC& sync);
		bool Apply_InventorySnapshot(
			const LostArk::Shared::S2C_INVENTORY_SNAPSHOT& snapshot);
		void Apply_PartyInviteReceived(
			const LostArk::Shared::S2C_PARTY_INVITE_RECEIVED& received);
		void Apply_PartyRoster(
			const LostArk::Shared::S2C_PARTY_ROSTER& roster);
		void Apply_ChatReceived(
			const LostArk::Shared::S2C_CHAT& received);
		enum class CHARACTER_REPLACE_RESULT
		{
			REPLACED,
			RECOVERED_FAILURE,
			FATAL_FAILURE
		};
		CHARACTER_REPLACE_RESULT Replace_CharacterClass(
			const LostArk::Shared::PLAYER_SNAPSHOT& snapshot);
		void Stage_LocalCharacterClassReplacement(
			const LostArk::Shared::PLAYER_SNAPSHOT& Snapshot,
			std::uint32_t iServerTick);
		void Clear_DeferredLocalCharacterClassReplacement();

		void Update_DeathPresentations();
		void Reset_World();
		bool Spawn_CombatObjectPresentation(
			const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawned,
			COMBAT_OBJECT_PRESENTATION_HANDLE& outHandle,
			std::string& outStatus);
		bool Update_CombatObjectPresentation(
			COMBAT_OBJECT_PRESENTATION_HANDLE handle,
			const LostArk::Shared::COMBAT_OBJECT_SNAPSHOT& snapshot);
		void Stop_CombatObjectPresentation(
			COMBAT_OBJECT_PRESENTATION_HANDLE handle);
		void Release_CombatObjectPresentation(
			COMBAT_OBJECT_PRESENTATION_HANDLE handle);
		struct COMBAT_OBJECT_PRESENTATION_SINK final
		{
			CClientReplication& Owner;
			bool Spawn(
				const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& message,
				COMBAT_OBJECT_PRESENTATION_HANDLE& outHandle,
				std::string& outStatus)
			{
				return Owner.Spawn_CombatObjectPresentation(
					message, outHandle, outStatus);
			}
			bool Update(
				COMBAT_OBJECT_PRESENTATION_HANDLE handle,
				const LostArk::Shared::COMBAT_OBJECT_SNAPSHOT& snapshot)
			{
				return Owner.Update_CombatObjectPresentation(handle, snapshot);
			}
			void Stop(COMBAT_OBJECT_PRESENTATION_HANDLE handle)
			{
				Owner.Stop_CombatObjectPresentation(handle);
			}
			void Release(COMBAT_OBJECT_PRESENTATION_HANDLE handle)
			{
				Owner.Release_CombatObjectPresentation(handle);
			}
		};

	private:
		//?대뼡 layer怨?prototype???앹꽦?섏뼱???섎뒗吏
		DESC m_Desc;
		std::wstring m_strLocalPlayerNavigationPrototypeTag;
		// Stable net objects: slot table, free-slot index and the
		// handle-by-entity-id lookup, kept across frames.
		CNetObjectRegistry m_Registry;
		//index slot, slotindex, generation
		OBJECT_HANDLE m_LocalCharacterHandle;
		bool m_isInitialized = false;
		bool m_wasConnected = false;
		bool m_hasPendingConnectionLoss = false;
		bool m_hasFatalWorldDestructionFailure = false;
		//留덉?留됱쑝濡??곸슜??snapshot tick
		std::uint32_t m_iLastServerTick = 0;
		struct DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT final
		{
			bool_t isPending = false;
			std::uint64_t iGeneration = 0u;
			std::uint32_t iServerTick = 0u;
			LostArk::Shared::PLAYER_SNAPSHOT Snapshot{};
		} m_DeferredLocalCharacterClassReplacement;
		std::uint64_t m_iNextDeferredLocalCharacterClassReplacementGeneration = 1u;
		std::string m_strPendingPresentationFailure;
		VALTAN_PRESENTATION_STATE m_ValtanPresentationState;
		CCombatObjectProjectionRuntime m_CombatObjectProjectionRuntime;
		CWorldDestructionProjectionRuntime m_WorldDestructionProjectionRuntime;
		std::deque<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>
			m_WorldDestructionLiveEvents;
		uint64_t m_iWorldDestructionPresentationGeneration = 0u;
		LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS
			m_WorldDestructionDiagnostics{};
		LostArk::Shared::S2C_ENCOUNTER_PROP_SYNC m_EncounterPropState{};
		LostArk::Shared::S2C_INVENTORY_SNAPSHOT m_InventoryState{};
		bool m_hasPendingPartyInvite = false;
		LostArk::Shared::S2C_PARTY_INVITE_RECEIVED m_PendingPartyInvite{};
		LostArk::Shared::S2C_PARTY_ROSTER m_PartyRoster{};
		CReplicatedPlayerHealth m_PlayerHealth;
		bool m_hasPendingPartyTransferResult = false;
		LostArk::Shared::S2C_PARTY_TRANSFER_RESULT m_PendingPartyTransferResult{};
		std::vector<std::string> m_PendingWorldSequencePlays;

		struct CHAT_BUBBLE_ENTRY
		{
			std::string strText;
			std::chrono::steady_clock::time_point ExpireAt;
		};
		static constexpr std::chrono::seconds CHAT_BUBBLE_DURATION{ 5 };
		std::unordered_map<LostArk::Shared::NET_ENTITY_ID, CHAT_BUBBLE_ENTRY>
			m_ChatBubblesByNetEntityId;
#ifdef _DEBUG
		COMBAT_DEBUG_VISIBILITY_SNAPSHOT m_CombatDebugVisibility{};
		bool_t m_isCombatObjectHitAreaDebugLoadAttempted = false;
		std::unordered_map<std::string,
			std::vector<COMBAT_OBJECT_HIT_AREA_DEBUG>>
			m_CombatObjectHitAreasByArchetype;
#endif

		struct WORLD_ENTITY_PRESENTATION
		{
			LostArk::Shared::NET_ENTITY_ID iOwnerBossNetEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			LostArk::Shared::WORLD_ENTITY_KIND eKind =
				LostArk::Shared::WORLD_ENTITY_KIND::END;
			std::string strPlacementId;
			std::string strArchetypeId;
			std::string strEncounterId;
			std::string strCurrentClip;
			/* Copied at spawn so a live entity keeps one validated placement
			binding for its entire presentation lifetime. */
			std::string strResolvedIdleClip;
			NPC_PLACEMENT_PRESENTATION_ENTRY NpcPresentation;
			NPC_ACTION_EDGE_STATE NpcActionEdge;
			MONSTER_PRESENTATION_ACTION_STATE MonsterActionState;
			std::string strActiveActionId;
			std::size_t iActionClipIndex = 0u;
			ESTHER_ACTION_SOUND_PLAYBACK_STATE EstherActionSoundState;
			f32_t fCollisionRadius = 0.f;
			/* The Server occurrence pin and the R -> M generation admitted by the
			   concrete CValtan are tracked independently. A structural exact-reload
			   failure latches RejectedPresentationRevision; transient world-entry
			   lock contention leaves it empty so the bounded baseline recovery may
			   admit the same occurrence on a later snapshot. */
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			LostArk::Shared::GameplayDataRevision AdmittedPresentationRevision{};
			LostArk::Shared::GameplayDataRevision RejectedPresentationRevision{};
			bool_t bPresentationIsolated = false;
			std::weak_ptr<CNpc> pNpc;
			std::weak_ptr<CValtan> pValtan;
		};
		WORLD_ENTITY_PRESENTATION* Find_ValtanPresentation(
			const std::shared_ptr<CValtan>& pValtan);
		bool_t Ensure_ValtanPresentationRevision(
			WORLD_ENTITY_PRESENTATION& Presentation,
			const std::shared_ptr<CValtan>& pValtan,
			const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
			bool_t bIsPrimary,
			std::string& strOutStatus);
		std::unordered_map<
			LostArk::Shared::NET_ENTITY_ID,
			WORLD_ENTITY_PRESENTATION> m_WorldEntities;
		CPrimaryValtanPresentationFreshnessGate
			m_PrimaryValtanJoinedPresentationFreshness;
		CPrimaryValtanPresentationFreshnessGate
			m_PrimaryValtanCombatObjectSoundFreshness;
		/* The existing Layer owns death tails; they are no longer network entities. */
		std::vector<std::weak_ptr<CValtan>> m_DeathPresentations;
	};
}

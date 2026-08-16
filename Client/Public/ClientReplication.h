#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "ClientReplicationEvent.h"
#include "NetObjectRegistry.h"
#include "WorldDestructionProjectionDocument.h"
#include "WorldDestructionProjectionRuntime.h"

#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

//Network Event瑜??ㅼ젣 Engine GameObject ?앹꽦 ?쒓굅濡?踰덉뿭?섎뒗 ???섎굹??main-thread 寃쎄퀎

namespace Client
{
	class CCharacter;
	class CNpc;
	class CValtan;
	class CDeployPropRuntime;

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
		std::uint32_t iActionStartTick = 0u;
		float3_t vPosition = {};
		f32_t fYawDegrees = 0.f;
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
			std::wstring strPlayerLayerTag;
			std::wstring strWorldEntityLayerTag;
			CDeployPropRuntime* pDeployPropRuntime = nullptr;
			const CWorldDestructionProjectionDocument*
				pWorldDestructionProjection = nullptr;
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
		bool Try_Consume_WorldDestructionLiveEvent(
			LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE& outEvent);
#ifdef _DEBUG
		void Set_CombatColliderDebugVisible(bool_t isVisible);
		void Set_SkillHitAreaDebugVisible(bool_t isVisible);
#endif

		std::shared_ptr<CCharacter> Get_LocalCharacter() const;
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

	private:
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
		//snapshot??netentityid瑜??ㅼ젣 client character濡??댁꽍?섎뒗 ?⑥닔
		bool Apply_WorldSnapshot(
			const LostArk::Shared::S2C_WORLD_SNAPSHOT& snapshot);
		bool Apply_WorldDestructionFullSync(
			const LostArk::Shared::S2C_WORLD_DESTRUCTION_FULL_SYNC& fullSync);
		bool Apply_WorldDestructionDelta(
			const LostArk::Shared::S2C_WORLD_DESTRUCTION_DELTA& delta);
		bool Apply_EncounterPropSync(
			const LostArk::Shared::S2C_ENCOUNTER_PROP_SYNC& sync);
		enum class CHARACTER_REPLACE_RESULT
		{
			REPLACED,
			RECOVERED_FAILURE,
			FATAL_FAILURE
		};
		CHARACTER_REPLACE_RESULT Replace_CharacterClass(
			const LostArk::Shared::PLAYER_SNAPSHOT& snapshot);

		void Reset_World();

	private:
		//?대뼡 layer怨?prototype???앹꽦?섏뼱???섎뒗吏
		DESC m_Desc;
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
		std::string m_strPendingPresentationFailure;
		VALTAN_PRESENTATION_STATE m_ValtanPresentationState;
		CWorldDestructionProjectionRuntime m_WorldDestructionProjectionRuntime;
		std::deque<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>
			m_WorldDestructionLiveEvents;
		uint64_t m_iWorldDestructionPresentationGeneration = 0u;
		LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS
			m_WorldDestructionDiagnostics{};
		LostArk::Shared::S2C_ENCOUNTER_PROP_SYNC m_EncounterPropState{};
#ifdef _DEBUG
		bool_t m_isCombatColliderDebugVisible = false;
		bool_t m_isSkillHitAreaDebugVisible = true;
#endif

		struct WORLD_ENTITY_PRESENTATION
		{
			LostArk::Shared::WORLD_ENTITY_KIND eKind =
				LostArk::Shared::WORLD_ENTITY_KIND::END;
			std::string strArchetypeId;
			std::string strEncounterId;
			std::string strCurrentClip;
			f32_t fCollisionRadius = 0.f;
			std::weak_ptr<CNpc> pNpc;
			std::weak_ptr<CValtan> pValtan;
		};
		std::unordered_map<
			LostArk::Shared::NET_ENTITY_ID,
			WORLD_ENTITY_PRESENTATION> m_WorldEntities;
	};
}

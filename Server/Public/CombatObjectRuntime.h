#pragma once

#include "GameplayCatalog.h"
#include "ServerCombatGeometry.h"
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"

#include "Network/PacketMessages.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace LostArk::Server
{
	/* The room owns one list for every gameplay object that can outlive the
	spawning action. Player projectiles and boss pattern objects differ only in
	the adapter that builds this immutable runtime description. */
	enum class SERVER_COMBAT_OBJECT_SOURCE_KIND : std::uint8_t
	{
		PLAYER,
		WORLD_ENTITY
	};

	enum class SERVER_COMBAT_OBJECT_HIT_TRIGGER : std::uint8_t
	{
		CONTACT,
		TIMED
	};

	enum class SERVER_COMBAT_OBJECT_CONTACT_SAMPLING : std::uint8_t
	{
		POSE,
		SWEPT
	};

	struct SERVER_COMBAT_OBJECT_HIT_RUNTIME final
	{
		SERVER_COMBAT_OBJECT_HIT_TRIGGER eTrigger =
			SERVER_COMBAT_OBJECT_HIT_TRIGGER::TIMED;
		SERVER_COMBAT_OBJECT_CONTACT_SAMPLING eContactSampling =
			SERVER_COMBAT_OBJECT_CONTACT_SAMPLING::POSE;
		std::uint32_t iAtMs = 0u;
		std::uint32_t iRepeatIntervalMs = 0u;
		SERVER_COMBAT_SHAPE_XZ Shape;
		/* One resolved raw amount per repeat. Keeping the split here means a
		projectile never has to look its skill definition up again after spawn. */
		std::vector<std::uint32_t> RepeatRawDamage;
		std::uint32_t iStaggerDamage = 0u;
		std::uint32_t iPartDamage = 0u;
		std::uint32_t iCounterPower = 0u;
		float fPushRangeM = 0.f;
		std::uint32_t iPushMs = 0u;
		bool bKnockdown = false;
		std::uint32_t iDownMs = 0u;
		std::uint32_t iAppliedTimedCount = 0u;
	};

	struct SERVER_COMBAT_OBJECT_CONTACT_MARK final
	{
		LostArk::Shared::NET_ENTITY_ID iTargetNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint16_t iHitIndex = 0u;
		std::uint32_t iAppliedCount = 0u;
		float fNextMilliseconds = 0.f;
	};

	struct SERVER_COMBAT_OBJECT_POSE final
	{
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fDirectionX = 0.f;
		float fDirectionZ = 1.f;
		float fYawDegrees = 0.f;
	};

	/* Runtime-only occurrence identity and motion history. These values are
	kept with the live object so later ticks never have to infer its owner from
	the boss's current stage, which may already have advanced. */
	struct SERVER_COMBAT_OBJECT_LIVE_STATE final
	{
		std::uint32_t iOwnerPatternSequence = 0u;
		std::string strOwnerPatternId;
		std::string strOwnerStageActionId;
		SERVER_COMBAT_OBJECT_POSE PreviousPose;
		SERVER_COMBAT_OBJECT_POSE CurrentPose;
	};

	/* The room resolves either the still-live locked player or the boss's last
	valid locked-target position before preparing a boss object. */
	struct SERVER_COMBAT_OBJECT_LOCKED_TARGET final
	{
		LostArk::Shared::NET_ENTITY_ID iNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
	};

	struct SERVER_COMBAT_OBJECT final
	{
		LostArk::Shared::COMBAT_OBJECT_ID iCombatObjectId =
			LostArk::Shared::INVALID_COMBAT_OBJECT_ID;
		SERVER_COMBAT_OBJECT_SOURCE_KIND eSourceKind =
			SERVER_COMBAT_OBJECT_SOURCE_KIND::PLAYER;
		LostArk::Shared::PLAYER_ID iSourcePlayerId =
			LostArk::Shared::INVALID_PLAYER_ID;
		LostArk::Shared::SKILL_ID iSourceSkillId =
			LostArk::Shared::INVALID_SKILL_ID;
		LostArk::Shared::NET_ENTITY_ID iSourceNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		LostArk::Shared::NET_ENTITY_ID iLockedTargetNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint32_t iSpawnTick = 0u;
		std::string strCombatObjectArchetypeId;
		std::string strClientVisualId;
		bool bReplicated = false;
		bool bTrackLockedTargetUntilFirstPulse = false;
		SERVER_COMBAT_OBJECT_LIVE_STATE LiveState;
		float fSpeedMps = 0.f;
		float fRemainingDistanceM = -1.f;
		float fRemainingMilliseconds = 0.f;
		float fElapsedMilliseconds = 0.f;
		std::vector<SERVER_COMBAT_OBJECT_HIT_RUNTIME> Hits;
		std::vector<SERVER_COMBAT_OBJECT_CONTACT_MARK> ContactMarks;
	};

	/* A transaction is prepared without touching the live set. Stage EXIT and
	ENTER actions share one transaction, so an invalid second action cannot
	leave the first action's objects committed. */
	struct SERVER_COMBAT_OBJECT_TRANSACTION final
	{
		std::uint64_t iExpectedRevision = 0u;
		LostArk::Shared::COMBAT_OBJECT_ID iNextCombatObjectId =
			LostArk::Shared::INVALID_COMBAT_OBJECT_ID;
		std::vector<SERVER_COMBAT_OBJECT> Objects;
	};

	class CCombatObjectRuntime final
	{
	public:
		[[nodiscard]] SERVER_COMBAT_OBJECT_TRANSACTION Begin_Transaction() const;

		bool Stage_PlayerProjectile(
			SERVER_COMBAT_OBJECT_TRANSACTION& transaction,
			const SERVER_PLAYER& source,
			const PLAYER_SKILL_DEFINITION& skill,
			const PLAYER_SKILL_PROJECTILE& definition,
			std::uint32_t stageIndex,
			std::uint32_t projectileIndex,
			std::uint64_t totalDamage,
			std::uint32_t subHitTotal,
			std::uint32_t subHitBase,
			std::uint32_t serverTick,
			std::string& status) const;

		bool Stage_BossCombatObject(
			SERVER_COMBAT_OBJECT_TRANSACTION& transaction,
			const SERVER_WORLD_ENTITY& boss,
			const SERVER_COMBAT_OBJECT_LOCKED_TARGET* lockedTarget,
			const BOSS_COMBAT_OBJECT_DEFINITION& definition,
			const CGameplayCatalog& catalog,
			std::uint32_t count,
			std::uint32_t serverTick,
			std::string& status) const;

		/* Commit has no validation side effects: every fallible check belongs to
		Stage_*. A revision mismatch is the only possible rejection. */
		bool Commit(SERVER_COMBAT_OBJECT_TRANSACTION&& transaction);

		void Update(
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			std::vector<SERVER_WORLD_ENTITY>& worldEntities,
			const CGameplayCatalog& catalog,
			float fixedDeltaSeconds,
			std::uint32_t serverTick,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents);

		void Cancel_Source(LostArk::Shared::NET_ENTITY_ID sourceNetEntityId);
		void Reset();
		/* Empty-room reset has no observer. Discarding its lifecycle prevents a
		later party from receiving spawn/despawn edges from the prior epoch. */
		void Discard_PendingLifecycle();

		void Drain_Lifecycle(
			std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED>& outSpawned,
			std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_DESPAWNED>& outDespawned);
		void Build_LiveSpawnMessages(
			std::uint32_t serverTick,
			std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED>& outSpawned) const;
		bool Build_Snapshots(
			std::vector<LostArk::Shared::COMBAT_OBJECT_SNAPSHOT>& outSnapshots) const;

		[[nodiscard]] const std::vector<SERVER_COMBAT_OBJECT>& Get_LiveObjects() const
		{
			return m_Objects;
		}

	private:
		bool Allocate_Id(
			SERVER_COMBAT_OBJECT_TRANSACTION& transaction,
			LostArk::Shared::COMBAT_OBJECT_ID& outId) const;
		void Despawn_At(std::size_t index);
		static LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED To_SpawnedMessage(
			const SERVER_COMBAT_OBJECT& object);

		std::vector<SERVER_COMBAT_OBJECT> m_Objects;
		std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED> m_PendingSpawned;
		std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_DESPAWNED> m_PendingDespawned;
		LostArk::Shared::COMBAT_OBJECT_ID m_iNextCombatObjectId = 1u;
		std::uint64_t m_iRevision = 1u;
	};
}

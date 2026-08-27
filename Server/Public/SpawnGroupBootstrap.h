#pragma once

#include "Network/PacketType.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace LostArk::Server
{
	struct MONSTER_RUNTIME_PROFILE final
	{
		std::string strArchetypeId;
		std::uint32_t iMaxHp = 0;
		std::uint32_t iAttackPower = 0;
		std::uint32_t iDefense = 0;
		float fCollisionRadius = 0.f;
		float fEngageRange = 0.f;
		float fTargetReleaseRange = 0.f;
		float fMoveSpeed = 0.f;
		float fTurnSpeedDegreesPerSecond = 0.f;
		float fAcceleration = 0.f;
		float fDeceleration = 0.f;
		float fArrivalSlowRadius = 0.f;
		float fAttackRange = 0.f;
		std::uint32_t iAttackWindupMs = 0;
		std::uint32_t iAttackActiveMs = 0;
		std::uint32_t iAttackRecoveryMs = 0;
		std::uint32_t iDeadDespawnMs = 0;
		/* Multiplier on the authored push range of each player hit; 0 means the
		monster never moves (super armour). */
		float fHitKnockbackScale = 0.f;
		/* Player push of this monster's landed attack: metres over iAttackPushMs,
		a negative range pulls the player toward the monster. */
		float fAttackPushRangeM = 0.f;
		std::uint32_t iAttackPushMs = 0;
		bool bAttackKnockdown = false;
		std::uint32_t iAttackDownMs = 0;
	};

	struct SPAWN_GROUP_ANCHOR final
	{
		std::string strAnchorId;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;
	};

	struct SPAWN_GROUP_ENTRY final
	{
		std::string strArchetypeId;
		std::uint32_t iCount = 0;
		std::string strAnchorId;
		std::uint32_t iInitialDelayMs = 0;
		std::uint32_t iSpawnIntervalMs = 0;
	};

	enum class SPAWN_NEXT_WAVE_POLICY : std::uint8_t
	{
		/* The wave ends only once everything it scheduled is dead. */
		ALL_DEAD,
		/* The wave ends on its own clock, so the next one can start while the
		previous is still alive. maxAlive still caps the group. */
		TIMER
	};

	enum class SPAWN_GROUP_REPEAT_POLICY : std::uint8_t
	{
		ONCE,
		/* The group restarts at its first wave after iRepeatDelayMs. */
		REPEAT
	};

	struct SPAWN_GROUP_WAVE final
	{
		std::string strWaveId;
		std::uint32_t iStartDelayMs = 0;
		SPAWN_NEXT_WAVE_POLICY eNextWavePolicy =
			SPAWN_NEXT_WAVE_POLICY::ALL_DEAD;
		/* Measured from the wave's own start, and only read by TIMER. ALL_DEAD
		waves are required to publish zero so the field can never quietly mean
		two things. */
		std::uint32_t iNextWaveDelayMs = 0;
		std::vector<SPAWN_GROUP_ENTRY> Entries;
	};

	struct SPAWN_GROUP_DEFINITION final
	{
		std::string strSpawnGroupId;
		std::string strRequiredCompletedGroupId;
		std::uint32_t iMaxAlive = 0;
		SPAWN_GROUP_REPEAT_POLICY eRepeatPolicy =
			SPAWN_GROUP_REPEAT_POLICY::ONCE;
		/* Quiet time between the group completing and restarting. Only read by
		REPEAT; ONCE groups publish zero. */
		std::uint32_t iRepeatDelayMs = 0;
		std::vector<SPAWN_GROUP_WAVE> Waves;
	};

	class CSpawnGroupBootstrap final
	{
	public:
		bool Load(LostArk::Shared::WORLD_ID worldId);

		const std::vector<SPAWN_GROUP_DEFINITION>& Get_Groups() const { return m_Groups; }
		const SPAWN_GROUP_ANCHOR* Find_Anchor(const std::string& anchorId) const;
		const MONSTER_RUNTIME_PROFILE* Find_Profile(const std::string& archetypeId) const;
		const std::string& Get_Status() const { return m_strStatus; }
		std::uint32_t Get_Revision() const { return m_iRevision; }

	private:
		std::vector<SPAWN_GROUP_DEFINITION> m_Groups;
		std::unordered_map<std::string, SPAWN_GROUP_ANCHOR> m_Anchors;
		std::unordered_map<std::string, MONSTER_RUNTIME_PROFILE> m_Profiles;
		std::string m_strStatus;
		std::uint32_t m_iRevision = 0;
	};
}

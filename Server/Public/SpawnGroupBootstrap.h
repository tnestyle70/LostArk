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
		float fMoveSpeed = 0.f;
		float fAttackRange = 0.f;
		std::uint32_t iAttackWindupMs = 0;
		std::uint32_t iAttackActiveMs = 0;
		std::uint32_t iAttackRecoveryMs = 0;
		std::uint32_t iDeadDespawnMs = 0;
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

	struct SPAWN_GROUP_WAVE final
	{
		std::string strWaveId;
		std::uint32_t iStartDelayMs = 0;
		std::vector<SPAWN_GROUP_ENTRY> Entries;
	};

	struct SPAWN_GROUP_DEFINITION final
	{
		std::string strSpawnGroupId;
		std::string strRequiredCompletedGroupId;
		std::uint32_t iMaxAlive = 0;
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

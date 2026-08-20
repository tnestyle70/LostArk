#pragma once

#include "Client_Defines.h"
#include "Network/PacketMessages.h"

#include <cstdint>
#include <string>

namespace Client
{

// Gameplay input emits intent through this boundary. It does not know whether
// the command is sent to a server, recorded by a test, or rejected offline.
class IPlayerCommandSink
{
public:
	virtual ~IPlayerCommandSink() = default;

	virtual bool Request_MoveGoal(
		std::uint32_t clientSequence,
		float goalX,
		float goalZ) = 0;

	virtual bool Request_UseSkill(
		std::uint32_t clientSequence,
		LostArk::Shared::SKILL_ID skillId,
		float aimX,
		float aimZ) = 0;

	virtual bool Request_ReleaseSkill(
		std::uint32_t clientSequence,
		LostArk::Shared::SKILL_ID skillId) = 0;

	virtual bool Request_SkillAim(
		std::uint32_t clientSequence,
		LostArk::Shared::SKILL_ID skillId,
		float aimX,
		float aimZ) = 0;
	virtual bool Request_RevivePlayer(
		std::uint32_t clientSequence) = 0;
	// Raid Esther roster slot (1..3), aimed at a world-space point. The slot
	// is positional; the server owns which archetype it summons.
	virtual bool Request_EstherSkill(
		std::uint32_t clientSequence,
		std::uint8_t slotIndex,
		float aimX,
		float aimZ) = 0;
	virtual bool Request_ChangeCharacterClass(
		std::uint32_t clientSequence,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass) = 0;
	// Bern's Valtan-entry confirm window's confirm button. npcPlacementId names
	// which guide NPC the player right-clicked (npc.bern.beda.guide /
	// npc.bern.aylara).
	virtual bool Request_ConfirmNpcEntry(
		std::uint32_t clientSequence,
		const std::string& npcPlacementId) = 0;
};

}

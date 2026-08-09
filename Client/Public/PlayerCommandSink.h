#pragma once

#include "Client_Defines.h"
#include "Network/PacketMessages.h"

#include <cstdint>

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
	virtual bool Request_RevivePlayer(
		std::uint32_t clientSequence) = 0;
	virtual bool Request_ChangeCharacterClass(
		std::uint32_t clientSequence,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass) = 0;
};

}

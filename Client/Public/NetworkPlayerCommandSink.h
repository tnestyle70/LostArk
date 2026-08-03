#pragma once

#include "PlayerCommandSink.h"

namespace Client
{

class CNetworkPlayerCommandSink final : public IPlayerCommandSink
{
public:
	bool Request_MoveGoal(
		std::uint32_t clientSequence,
		float goalX,
		float goalZ) override;
	bool Request_UseSkill(
		std::uint32_t clientSequence,
		LostArk::Shared::SKILL_ID skillId,
		float aimX,
		float aimZ) override;
};

}

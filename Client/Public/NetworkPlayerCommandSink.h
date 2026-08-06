#pragma once

#include "PlayerCommandSink.h"

#include <atomic>

namespace Client
{

class CNetworkPlayerCommandSink final : public IPlayerCommandSink
{
public:
	CNetworkPlayerCommandSink();
	~CNetworkPlayerCommandSink() override;

	CNetworkPlayerCommandSink(const CNetworkPlayerCommandSink&) = delete;
	CNetworkPlayerCommandSink& operator=(
		const CNetworkPlayerCommandSink&) = delete;

	static std::uint32_t Get_LiveInstanceCount();

	bool Request_MoveGoal(
		std::uint32_t clientSequence,
		float goalX,
		float goalZ) override;
	bool Request_UseSkill(
		std::uint32_t clientSequence,
		LostArk::Shared::SKILL_ID skillId,
		float aimX,
		float aimZ) override;
	bool Request_ReleaseSkill(
		std::uint32_t clientSequence,
		LostArk::Shared::SKILL_ID skillId) override;

private:
	static std::atomic_uint32_t s_iLiveInstanceCount;
};

}

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
	bool Request_UseGroundTargetSkill(
		std::uint32_t clientSequence,
		LostArk::Shared::SKILL_ID skillId,
		float targetX,
		float targetZ) override;
	bool Request_ReleaseSkill(
		std::uint32_t clientSequence,
		LostArk::Shared::SKILL_ID skillId) override;
	bool Request_SkillAim(
		std::uint32_t clientSequence,
		LostArk::Shared::SKILL_ID skillId,
		float aimX,
		float aimZ) override;
	bool Request_RevivePlayer(
		std::uint32_t clientSequence) override;
#ifdef _DEBUG
	bool Request_DebugKillSelf(
		std::uint32_t clientSequence) override;
#endif
	bool Request_EstherSkill(
		std::uint32_t clientSequence,
		std::uint8_t slotIndex,
		float aimX,
		float aimZ) override;
	bool Request_ChangeCharacterClass(
		std::uint32_t clientSequence,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass) override;
	bool Request_ConfirmNpcEntry(
		std::uint32_t clientSequence,
		const std::string& npcPlacementId) override;

private:
	static std::atomic_uint32_t s_iLiveInstanceCount;
};

}

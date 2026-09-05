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

	bool Request_DebugTeleportToPosition(
		std::uint32_t requestSequence, float pickedX, float pickedY, float pickedZ) override;
	bool Consume_DebugTeleportResult(
		LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT& result) override;
	bool Request_DebugMadnessForm(
		std::uint32_t requestSequence,
		LostArk::Shared::PLAYER_MADNESS_FORM form) override;
	bool Consume_DebugMadnessFormResult(
		LostArk::Shared::S2C_DEBUG_SET_MADNESS_FORM_RESULT& result) override;

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
	bool Request_RaidEntryPropose(
		std::uint32_t clientSequence,
		const std::string& npcPlacementId,
		LostArk::Shared::RAID_ENTRY_TARGET target) override;
	bool Request_RaidEntryRespond(
		std::uint32_t clientSequence,
		std::uint32_t proposalId,
		bool accepted) override;
	bool Request_ReturnToBern(
		std::uint32_t clientSequence) override;
	bool Request_PartyInvite(
		std::uint32_t clientSequence,
		LostArk::Shared::NET_ENTITY_ID targetNetEntityId) override;
	bool Request_PartyInviteRespond(
		std::uint32_t clientSequence,
		LostArk::Shared::NET_ENTITY_ID fromNetEntityId,
		bool accepted) override;
	bool Request_SendChat(const std::string& text) override;

private:
	static std::atomic_uint32_t s_iLiveInstanceCount;
};

}

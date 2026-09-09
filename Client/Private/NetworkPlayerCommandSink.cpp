#include "NetworkPlayerCommandSink.h"

#include "NetworkManager.h"

bool Client::CNetworkPlayerCommandSink::Request_DebugWorldPlayback(
	const LostArk::Shared::C2S_DEBUG_WORLD_PLAYBACK& request)
{
	return CNetworkManager::Get().Send_DebugWorldPlayback(request);
}

bool Client::CNetworkPlayerCommandSink::Consume_DebugWorldPlaybackResult(
	LostArk::Shared::S2C_DEBUG_WORLD_PLAYBACK_RESULT& result)
{
	return CNetworkManager::Get().Try_Consume_DebugWorldPlaybackResult(result);
}

std::atomic_uint32_t
	Client::CNetworkPlayerCommandSink::s_iLiveInstanceCount = 0u;

Client::CNetworkPlayerCommandSink::CNetworkPlayerCommandSink()
{
	++s_iLiveInstanceCount;
}

Client::CNetworkPlayerCommandSink::~CNetworkPlayerCommandSink()
{
	--s_iLiveInstanceCount;
}

std::uint32_t
Client::CNetworkPlayerCommandSink::Get_LiveInstanceCount()
{
	return s_iLiveInstanceCount.load();
}

bool Client::CNetworkPlayerCommandSink::Request_DebugTeleportToPosition(
	const std::uint32_t requestSequence,
	const float pickedX, const float pickedY, const float pickedZ)
{
	return CNetworkManager::Get().Send_DebugTeleportToPosition(
		requestSequence, pickedX, pickedY, pickedZ);
}

bool Client::CNetworkPlayerCommandSink::Consume_DebugTeleportResult(
	LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT& result)
{
	return CNetworkManager::Get().Try_Consume_DebugTeleportResult(result);
}

bool Client::CNetworkPlayerCommandSink::Request_MarioMove(
	const std::uint32_t clientSequence, const LostArk::Shared::MARIO_DIRECTION direction)
{
	return CNetworkManager::Get().Send_MarioMove(clientSequence, direction);
}

bool Client::CNetworkPlayerCommandSink::Request_DebugMarioJump(
	const std::uint32_t clientSequence, const LostArk::Shared::MARIO_DIRECTION direction)
{
	return CNetworkManager::Get().Send_DebugMarioJump(clientSequence, direction);
}

bool Client::CNetworkPlayerCommandSink::Consume_DebugMarioJumpResult(
	LostArk::Shared::S2C_DEBUG_MARIO_JUMP_RESULT& result)
{
	return CNetworkManager::Get().Try_Consume_DebugMarioJumpResult(result);
}

bool Client::CNetworkPlayerCommandSink::Request_DebugMadnessForm(
	const std::uint32_t requestSequence,
	const LostArk::Shared::PLAYER_MADNESS_FORM form)
{
	return CNetworkManager::Get().Send_DebugSetMadnessForm(requestSequence, form);
}

bool Client::CNetworkPlayerCommandSink::Consume_DebugMadnessFormResult(
	LostArk::Shared::S2C_DEBUG_SET_MADNESS_FORM_RESULT& result)
{
	return CNetworkManager::Get().Try_Consume_DebugMadnessFormResult(result);
}

bool Client::CNetworkPlayerCommandSink::Request_InteractionSlot(
 std::uint32_t sequence, LostArk::Shared::INTERACTION_SLOT slot)
{
 return CNetworkManager::Get().Send_InteractionSlot(sequence, slot);
}

bool Client::CNetworkPlayerCommandSink::Request_DebugBingoFill(
 std::uint32_t sequence, std::uint32_t cellMask, bool reset)
{
 return CNetworkManager::Get().Send_DebugBingoFill(sequence, cellMask, reset);
}

bool Client::CNetworkPlayerCommandSink::Request_DebugBingoBomb(
 std::uint32_t sequence)
{
 return CNetworkManager::Get().Send_DebugBingoBomb(sequence);
}

bool Client::CNetworkPlayerCommandSink::Request_DebugBingoHammer(
 std::uint32_t sequence)
{
 return CNetworkManager::Get().Send_DebugBingoHammer(sequence);
}

bool Client::CNetworkPlayerCommandSink::Request_DebugKoukuHudMode(
 std::uint32_t sequence, LostArk::Shared::KOUKU_HUD_MODE mode)
{
 return CNetworkManager::Get().Send_DebugKoukuHudMode(sequence, mode);
}

bool Client::CNetworkPlayerCommandSink::Consume_DebugKoukuHudModeResult(
 LostArk::Shared::S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT& result)
{
 return CNetworkManager::Get().Try_Consume_DebugKoukuHudModeResult(result);
}

bool Client::CNetworkPlayerCommandSink::Request_MoveGoal(
	std::uint32_t clientSequence,
	float goalX,
	float goalZ)
{
	return CNetworkManager::Get().Send_MoveGoal(
		clientSequence,
		goalX,
		goalZ);
}

bool Client::CNetworkPlayerCommandSink::Request_UseSkill(
	std::uint32_t clientSequence,
	LostArk::Shared::SKILL_ID skillId,
	float aimX,
	float aimZ)
{
	return CNetworkManager::Get().Send_UseSkill(
		clientSequence,
		skillId,
		aimX,
		aimZ);
}

bool Client::CNetworkPlayerCommandSink::Request_UseGroundTargetSkill(
	std::uint32_t clientSequence,
	LostArk::Shared::SKILL_ID skillId,
	float targetX,
	float targetZ)
{
	return CNetworkManager::Get().Send_UseGroundTargetSkill(
		clientSequence, skillId, targetX, targetZ);
}

bool Client::CNetworkPlayerCommandSink::Request_ReleaseSkill(
	std::uint32_t clientSequence,
	LostArk::Shared::SKILL_ID skillId)
{
	return CNetworkManager::Get().Send_ReleaseSkill(clientSequence, skillId);
}

bool Client::CNetworkPlayerCommandSink::Request_SkillAim(
	std::uint32_t clientSequence,
	LostArk::Shared::SKILL_ID skillId,
	float aimX,
	float aimZ)
{
	return CNetworkManager::Get().Send_SkillAim(
		clientSequence,
		skillId,
		aimX,
		aimZ);
}

bool Client::CNetworkPlayerCommandSink::Request_InteractTrigger(
	const std::uint32_t clientSequence,
	const std::string& triggerPlacementId)
{
	return CNetworkManager::Get().Send_InteractTrigger(
		clientSequence, triggerPlacementId);
}

bool Client::CNetworkPlayerCommandSink::Request_RevivePlayer(
	const std::uint32_t clientSequence)
{
	return CNetworkManager::Get().Send_RevivePlayer(clientSequence);
}

#ifdef _DEBUG
bool Client::CNetworkPlayerCommandSink::Request_DebugKillSelf(
	const std::uint32_t clientSequence)
{
	return CNetworkManager::Get().Send_DebugKillSelf(clientSequence);
}
#endif

bool Client::CNetworkPlayerCommandSink::Request_EstherSkill(
	const std::uint32_t clientSequence,
	const std::uint8_t slotIndex,
	const float aimX,
	const float aimZ)
{
	return CNetworkManager::Get().Send_EstherSkill(
		clientSequence, slotIndex, aimX, aimZ);
}

bool Client::CNetworkPlayerCommandSink::Request_ChangeCharacterClass(
	const std::uint32_t clientSequence,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	return CNetworkManager::Get().Send_ChangeCharacterClass(
		clientSequence, characterClass);
}

bool Client::CNetworkPlayerCommandSink::Request_ConfirmNpcEntry(
	const std::uint32_t clientSequence,
	const std::string& npcPlacementId)
{
	return CNetworkManager::Get().Send_ConfirmNpcEntry(
		clientSequence, npcPlacementId);
}

bool Client::CNetworkPlayerCommandSink::Request_RaidEntryPropose(
	const std::uint32_t clientSequence,
	const std::string& npcPlacementId,
	const LostArk::Shared::RAID_ENTRY_TARGET target)
{
	return CNetworkManager::Get().Send_RaidEntryPropose(
		clientSequence, npcPlacementId, target);
}

bool Client::CNetworkPlayerCommandSink::Request_RaidEntryRespond(
	const std::uint32_t clientSequence,
	const std::uint32_t proposalId,
	const bool accepted)
{
	return CNetworkManager::Get().Send_RaidEntryRespond(
		clientSequence, proposalId, accepted);
}

bool Client::CNetworkPlayerCommandSink::Request_ReturnToBern(
	const std::uint32_t clientSequence)
{
	return CNetworkManager::Get().Send_ReturnToBern(clientSequence);
}

bool Client::CNetworkPlayerCommandSink::Request_PartyInvite(
	const std::uint32_t clientSequence,
	const LostArk::Shared::NET_ENTITY_ID targetNetEntityId)
{
	return CNetworkManager::Get().Send_PartyInvite(
		clientSequence, targetNetEntityId);
}

bool Client::CNetworkPlayerCommandSink::Request_PartyInviteRespond(
	const std::uint32_t clientSequence,
	const LostArk::Shared::NET_ENTITY_ID fromNetEntityId,
	const bool accepted)
{
	return CNetworkManager::Get().Send_PartyInviteRespond(
		clientSequence, fromNetEntityId, accepted);
}

bool Client::CNetworkPlayerCommandSink::Request_SendChat(
	const std::string& text)
{
	return CNetworkManager::Get().Send_Chat(text);
}

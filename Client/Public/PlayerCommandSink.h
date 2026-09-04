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

	/* A confirmed class-neutral ground-point selection. This is deliberately a
	 different typed call from directional aim, even though both serialize onto
	 C2S_USE_SKILL, so input code cannot accidentally cast before confirmation. */
	virtual bool Request_UseGroundTargetSkill(
		std::uint32_t clientSequence,
		LostArk::Shared::SKILL_ID skillId,
		float targetX,
		float targetZ) = 0;

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
	// Same-room-only party invite -- targetNetEntityId names another player
	// currently replicated in this room (right-clicked locally).
	virtual bool Request_PartyInvite(
		std::uint32_t clientSequence,
		LostArk::Shared::NET_ENTITY_ID targetNetEntityId) = 0;
	virtual bool Request_PartyInviteRespond(
		std::uint32_t clientSequence,
		LostArk::Shared::NET_ENTITY_ID fromNetEntityId,
		bool accepted) = 0;
	// Same-room chat line. The Server relays it to every current room member
	// (sender included) as S2C_CHAT. Fire-and-forget -- no ack, so no
	// clientSequence unlike the other Request_* calls above.
	virtual bool Request_SendChat(const std::string& text) = 0;
#ifdef _DEBUG
	// Debug/Development-build test aid only -- see PACKET_TYPE::C2S_DEBUG_KILL_SELF.
	virtual bool Request_DebugKillSelf(
		std::uint32_t clientSequence) = 0;
#endif
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
	// 파티 레이드 입장 투표 발의(리더/솔로). eTarget으로 발탄/쿠크를 고른다 -- NPC가
	// 아니라 UI 탭이 소유하며 Server가 이 값으로 target world를 결정한다.
	virtual bool Request_RaidEntryPropose(
		std::uint32_t clientSequence,
		const std::string& npcPlacementId,
		LostArk::Shared::RAID_ENTRY_TARGET target) = 0;
	// 투표 프롬프트에 대한 개별 수락/거절 응답. proposalId는 프롬프트가 준 값 그대로.
	virtual bool Request_RaidEntryRespond(
		std::uint32_t clientSequence,
		std::uint32_t proposalId,
		bool accepted) = 0;
	// Raid Clear screen's own "돌아가기" (return) button, Valtan Arena only --
	// the reverse trip of Request_ConfirmNpcEntry. No target NPC to name (the
	// button has no proximity requirement); the Server lands the player back
	// near Bern's own Valtan-entry guide NPC.
	virtual bool Request_ReturnToBern(
		std::uint32_t clientSequence) = 0;
};

}

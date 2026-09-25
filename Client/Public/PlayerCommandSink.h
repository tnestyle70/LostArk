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
	virtual bool Request_DebugKillGateBosses(const LostArk::Shared::C2S_DEBUG_KILL_GATE_BOSSES&) { return false; }
	virtual bool Request_SetCooldownMode(const LostArk::Shared::C2S_SET_COOLDOWN_MODE&) { return false; }
	virtual bool Consume_DebugKillGateBossesResult(LostArk::Shared::S2C_DEBUG_KILL_GATE_BOSSES_RESULT&) { return false; }
	virtual bool Consume_SetCooldownModeResult(LostArk::Shared::S2C_SET_COOLDOWN_MODE_RESULT&) { return false; }
	virtual bool Request_KoukuRaid(const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST&) { return false; }
	virtual bool Request_DebugWorldPlayback(const LostArk::Shared::C2S_DEBUG_WORLD_PLAYBACK&) { return false; }
	virtual bool Consume_DebugWorldPlaybackResult(LostArk::Shared::S2C_DEBUG_WORLD_PLAYBACK_RESULT&) { return false; }

	virtual bool Request_DebugTeleportToPosition(
		std::uint32_t requestSequence, float pickedX, float pickedY, float pickedZ) = 0;
	// Exact authored arena start; unsupported sinks reject this Debug reset.
	virtual bool Request_DebugReturnToKoukuStart(std::uint32_t) { return false; }
	virtual bool Consume_DebugTeleportResult(
		LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT& result) = 0;
	// Legacy wire naming is retained for the Server-authoritative Mario jump. Unsupported sinks reject it.
	virtual bool Request_DebugMarioJump(std::uint32_t, LostArk::Shared::MARIO_DIRECTION) { return false; }
	virtual bool Consume_DebugMarioJumpResult(
		LostArk::Shared::S2C_DEBUG_MARIO_JUMP_RESULT&) { return false; }
	virtual bool Request_MarioReturn(std::uint32_t) { return false; }
	virtual bool Consume_MarioReturnResult(
		LostArk::Shared::S2C_MARIO_RETURN_RESULT&) { return false; }
	// Mario left/right intent; STOP ends movement on the Server-owned lane.
	virtual bool Request_MarioMove(std::uint32_t, LostArk::Shared::MARIO_DIRECTION) { return false; }
	/* Debug F1 clown/player avatar toggle: intent only, the Server owns the
	form and the snapshot presents it. */
	virtual bool Request_DebugMadnessForm(
		std::uint32_t requestSequence,
		LostArk::Shared::PLAYER_MADNESS_FORM form) = 0;
	virtual bool Consume_DebugMadnessFormResult(
		LostArk::Shared::S2C_DEBUG_SET_MADNESS_FORM_RESULT& result) = 0;
	/* H key riding toggle: a vehicle id mounts, INVALID_VEHICLE_ID dismounts.
	Sinks without a Server reject it. */
	virtual bool Request_SetVehicleRiding(std::uint32_t, LostArk::Shared::VEHICLE_ID) { return false; }
	virtual bool Consume_VehicleRidingResult(
		LostArk::Shared::S2C_SET_VEHICLE_RIDING_RESULT&) { return false; }
	/* Title window: a title id wears it, INVALID_HONOR_TITLE_ID takes it off.
	Sinks without a Server reject it. */
	virtual bool Request_SetHonorTitle(std::uint32_t, LostArk::Shared::HONOR_TITLE_ID) { return false; }
	virtual bool Consume_HonorTitleResult(
		LostArk::Shared::S2C_SET_HONOR_TITLE_RESULT&) { return false; }

	virtual bool Request_InteractionSlot(std::uint32_t sequence,
		LostArk::Shared::INTERACTION_SLOT slot) = 0;
	virtual bool Request_DebugKoukuHudMode(std::uint32_t sequence,
		LostArk::Shared::KOUKU_HUD_MODE mode) = 0;
	/* Debug bingo board fill. The board comes back on the world snapshot, so
	there is no result to consume. */
	virtual bool Request_DebugBingoFill(std::uint32_t sequence,
		std::uint32_t cellMask, bool reset) = 0;
	/* Debug bingo bomb. The Server picks the carrier and owns the clock, so
	the request names nothing and there is no result to consume. */
	virtual bool Request_DebugBingoBomb(std::uint32_t sequence) = 0;
	/* Debug bingo hammer. The Server rolls the anchor and owns the phase
	clock, so the request names nothing. */
	virtual bool Request_DebugBingoHammer(std::uint32_t sequence) = 0;
	/* Debug wave-monster re-summon (F1 "Normal Monster 1/2"). The Server maps the
	button to its fixed spawn group and the monsters come back on the world
	snapshot, so the request names no group and there is no result to consume. */
	virtual bool Request_DebugResummonWaveMonsters(std::uint32_t sequence,
		LostArk::Shared::WAVE_MONSTER_BUTTON button) = 0;
	virtual bool Consume_DebugKoukuHudModeResult(
		LostArk::Shared::S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT& result) = 0;

	virtual bool Request_VehicleFlightInput(std::uint32_t, float, float, float) { return false; }
	virtual bool Request_RoomPing(std::uint32_t, float, float, float) { return false; }
	virtual bool Consume_RoomPing(LostArk::Shared::S2C_ROOM_PING&) { return false; }
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
	/* Answers an interact-gated trigger box the Server is currently offering.
	   Names only the box; the Server re-tests that this player is still
	   standing in it before anything runs. */
	virtual bool Request_InteractTrigger(
		std::uint32_t clientSequence,
		const std::string& triggerPlacementId) = 0;
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
	/* Debug F1 Esther summon by name, outside the roster slots and the gauge.
	Sinks without a Server reject it. */
	virtual bool Request_DebugUseEsther(
		std::uint32_t, LostArk::Shared::ESTHER_ID, float, float) { return false; }
	// World map square hole click. holeId is the 1-based row of the zone's square-hole
	// document; the Server owns the song lock and (later) the teleport.
	virtual bool Request_UseSquareHole(
		std::uint32_t clientSequence,
		std::uint16_t holeId) = 0;
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
	/* Commander raid gate progress after a gate clear: the leader / solo player proposes
	to move on, members answer the vote, and the Server's state comes back through
	Consume_GateProgressState. Sinks without a Server reject it. */
	virtual bool Request_GateProgressPropose(std::uint32_t, LostArk::Shared::GATE_PROGRESS_KIND) { return false; }
	virtual bool Request_GateProgressRespond(std::uint32_t, std::uint32_t, bool) { return false; }
	virtual bool Consume_GateProgressState(LostArk::Shared::S2C_GATE_PROGRESS_STATE&) { return false; }
	/* Raid-clear award input: each player's contribution to the cleared gate, as the Server
	recorded it. Sinks without a Server have none. */
	virtual bool Consume_RaidMvpResult(LostArk::Shared::S2C_RAID_MVP_RESULT&) { return false; }
	// Raid Clear screen's own "돌아가기" (return) button, Valtan Arena only --
	// the reverse trip of Request_ConfirmNpcEntry. No target NPC to name (the
	// button has no proximity requirement); the Server lands the player back
	// near Bern's own Valtan-entry guide NPC.
	virtual bool Request_ReturnToBern(
		std::uint32_t clientSequence) = 0;
};

}

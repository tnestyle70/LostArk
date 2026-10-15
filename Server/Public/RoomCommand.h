#pragma once

// 큐 명령의 발신 연결을 SESSION_ID와 INVALID_SESSION_ID로 식별한다.
#include "ServerIds.h"
// 월드 전환 때 플레이어 재화 상태를 값으로 담는 SERVER_PURSE 정의가 필요하다.
#include "ServerPlayer.h"
// 검증된 C2S 메시지와 인벤토리 snapshot을 명령 안에 값으로 보관한다.
#include "Network/PacketMessages.h"

// 등록 명령이 처리될 때까지 CClientSession을 shared_ptr로 유지한다.
#include <memory>
// 월드 전환 인벤토리 snapshot 목록을 명령 자체가 소유한다.
#include <vector>

namespace LostArk::Server
{
	//server는 여러 개의 존재하는 client를 가져야 한다.
	// 여기서는 shared_ptr 멤버 선언에 필요한 타입만 전방 선언한다. 실제 세션 등록은 GameRoom이 수행한다.
	class CClientSession;
	// Room은 같은 World 상태와 Tick을 공유하고 서로의 Event를 Broadcast 받는 세션 집합이다.
	// 현재는 베른성 하나이므로 Server에 CGameRoom 하나만 존재한다.
	// 현재 구현은 여러 공유 월드, 세션별 Character Select arena, 독립 Colosseum 경기 룸을 사용한다.
	// CServerApp이 바인딩으로 목적 룸을 고르며 이 enum은 wire packet 번호와 별개인 내부 분기 값이다.
	enum class ROOM_COMMAND_TYPE
	{
		REGISTER_SESSION,
		ENTER_WORLD,
		MOVE,
		USE_SKILL,
		RELEASE_SKILL,
		UPDATE_SKILL_AIM,
		USE_ESTHER_SKILL,
		USE_SQUAREHOLE,
		REVIVE_PLAYER,
		DEBUG_KILL_SELF,
		DEBUG_KILL_GATE_BOSSES,
		SET_COOLDOWN_MODE,
		DEBUG_ENTER_KOUKUSAYDON_ARENA,
		DEBUG_TELEPORT_TO_PLACEMENT,
		DEBUG_TELEPORT_TO_POSITION,
		DEBUG_MARIO_JUMP,
		MARIO_MOVE,
		MARIO_RETURN,
		DEBUG_SET_MADNESS_FORM,
		SET_VEHICLE_RIDING,
		SET_HONOR_TITLE,
		DEBUG_BINGO_FILL,
		DEBUG_BINGO_BOMB,
		DEBUG_BINGO_HAMMER,
		DEBUG_RESUMMON_WAVE_MONSTERS,
		DEBUG_USE_ESTHER,
		INTERACTION_SLOT,
		DEBUG_SET_KOUKU_HUD_MODE,
		CHANGE_CHARACTER_CLASS,
		SPAWN_WORLD_ENTITY,
		VALTAN_AUDITION,
		VALTAN_PATTERN_FLOW_START,
		VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT,
		KOUKUSAYDON_PATTERN_AUDITION,
		KOUKUSAYDON_DRAFT_CHUNK,
		KOUKUSAYDON_RAID,
		DEBUG_GIVE_ITEM,
		USE_ITEM,
		SET_EQUIPMENT,
		BUY_ITEMS,
		RESTORE_CHARACTER,
		UPGRADE_EQUIPMENT,
		CAPTURE_CHARACTER,
		REPAIR_EQUIPMENT,
		DESPAWN_ALL_WORLD_ENTITIES,
		CONFIRM_NPC_ENTRY,
		INTERACT_TRIGGER,
		DEBUG_WORLD_PLAYBACK,
        MAHARAKA_AI_TUNING,
		RETURN_TO_BERN,
		GUIDE_CONTROL,
		PARTY_INVITE,
		PARTY_INVITE_RESPOND,
		RAID_ENTRY_PROPOSE,
		RAID_ENTRY_RESPOND,
		COLOSSEUM_QUEUE_JOIN,
		COLOSSEUM_QUEUE_LEAVE,
		COLOSSEUM_RECRUIT,
		COLOSSEUM_LOAD_READY,
		COLOSSEUM_RETURN,
		GATE_PROGRESS_PROPOSE,
		GATE_PROGRESS_RESPOND,
		CHAT,
		ROOM_PING,
		LEAVE
	};

	// 수신 콜백이나 서버 내부 전환이 준비한 명령을 룸 worker까지 옮기는 값 객체다.
	// eType에 맞는 payload만 채우고 CGameRoom::Tick의 분기가 그 필드를 소비한다.
	// 자체 동기화는 없으며 Enqueue_Detailed의 명령 큐 잠금과 tick의 큐 인계가 접근을 직렬화한다.
	struct ROOM_COMMAND
	{
		// 기본 enum만으로 유효한 등록 명령이 되지는 않는다. REGISTER_SESSION은 아래 두 identity가 맞아야 한다.
		ROOM_COMMAND_TYPE eType =
			ROOM_COMMAND_TYPE::REGISTER_SESSION;

		// 패킷 안의 임의 ID가 아닌 현재 연결의 ID다. 큐 admission과 룸 handler가 발신자를 찾는 기준이다.
		SESSION_ID iSessionId = INVALID_SESSION_ID;

		// REGISTER_SESSION에서 사용한다. 큐가 보유한 동안 세션을 살려 두고 Handle_Register에 넘긴다.
		std::shared_ptr<CClientSession> pSession;

		// 입장 메시지의 소유 복사본이다. 아래 전환 보조 필드들은 실제 클라이언트 입장 payload에 포함되지 않는다.
		LostArk::Shared::C2S_ENTER_WORLD EnterWorld;
		// SERVER_WORLD_TRANSFER_REQUEST::strSpawnPlacementOverrideId를 옮기는 서버 내부 값이다.
		// CServerApp::Transfer_SessionWorld가 직접 만든 ENTER_WORLD만 설정하며 wire 메시지에 넣지 않는다.
		std::string strSpawnPlacementOverrideId;
		// 서버가 확인한 레이드 복귀 NPC placement ID를 Join까지 운반한다. 문자열을 명령 자체가 소유한다.
		std::string strRaidReturnNpcPlacementId;
		// 권위 있는 서버 월드 전환만 출발 월드를 설정한다. 클라이언트 입장 패킷에서는 채우지 않는다.
		LostArk::Shared::WORLD_ID eEntrySourceWorldId = LostArk::Shared::WORLD_ID::BERN;
		// SERVER_WORLD_TRANSFER_REQUEST의 서버 내부 운반 상태다. wire 메시지 필드가 아니다.
		// 명시적인 carry flag가 있어야 비어 있는 가방·재화도 전환한 실제 상태로 복원할 수 있다.
		bool bHasCarriedCharacterState = false;
		std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT> CarriedInventory;
		// 인벤토리와 함께 출발 월드의 재화 상태를 값으로 운반한다.
		SERVER_PURSE CarriedPurse;
		// ENTER_WORLD가 출발 월드의 장착 칭호를 복원할 때 사용한다. 전환 요청의 iHonorTitleId에서 오며
		// Lobby의 새 입장처럼 운반 상태가 없으면 INVALID_HONOR_TITLE_ID다.
		LostArk::Shared::HONOR_TITLE_ID iCarriedHonorTitleId = LostArk::Shared::INVALID_HONOR_TITLE_ID;

		// 이동·스킬·조준 입력은 packet reader가 검증한 값이다. 현재 플레이어 상태·권한 판정은 룸 handler가 맡는다.
		LostArk::Shared::C2S_MOVE Move;

		LostArk::Shared::C2S_USE_SKILL UseSkill;

		LostArk::Shared::C2S_RELEASE_SKILL ReleaseSkill;
		LostArk::Shared::C2S_UPDATE_SKILL_AIM UpdateSkillAim;
		LostArk::Shared::C2S_USE_ESTHER_SKILL UseEstherSkill;
		LostArk::Shared::C2S_USE_SQUAREHOLE UseSquareHole;
		LostArk::Shared::C2S_REVIVE_PLAYER RevivePlayer;
		// 전투·위치·쿠크 기믹 디버그 입력도 직접 엔티티 포인터를 넘기지 않고 typed payload로 전달한다.
		LostArk::Shared::C2S_DEBUG_KILL_SELF DebugKillSelf;
		LostArk::Shared::C2S_DEBUG_KILL_GATE_BOSSES DebugKillGateBosses;
		LostArk::Shared::C2S_SET_COOLDOWN_MODE SetCooldownMode;
		LostArk::Shared::C2S_DEBUG_ENTER_KOUKUSAYDON_ARENA
			DebugEnterKoukuSaydonArena;
		LostArk::Shared::C2S_DEBUG_TELEPORT_TO_PLACEMENT
			DebugTeleportToPlacement;
		LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION DebugTeleportToPosition;
		LostArk::Shared::C2S_DEBUG_MARIO_JUMP DebugMarioJump;
		LostArk::Shared::C2S_MARIO_RETURN MarioReturn;
		LostArk::Shared::C2S_MARIO_MOVE MarioMove;
		LostArk::Shared::C2S_DEBUG_SET_MADNESS_FORM DebugSetMadnessForm;
		LostArk::Shared::C2S_SET_VEHICLE_RIDING SetVehicleRiding;
		LostArk::Shared::C2S_SET_HONOR_TITLE SetHonorTitle;
		LostArk::Shared::C2S_DEBUG_BINGO_FILL DebugBingoFill;
		LostArk::Shared::C2S_DEBUG_BINGO_BOMB DebugBingoBomb;
		LostArk::Shared::C2S_DEBUG_BINGO_HAMMER DebugBingoHammer;
		LostArk::Shared::C2S_DEBUG_RESUMMON_WAVE_MONSTERS DebugResummonWaveMonsters;
		LostArk::Shared::C2S_DEBUG_USE_ESTHER DebugUseEsther;
		LostArk::Shared::C2S_INTERACTION_SLOT InteractionSlot;
		LostArk::Shared::C2S_DEBUG_SET_KOUKU_HUD_MODE DebugSetKoukuHudMode;
		LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS ChangeCharacterClass;

		// spawn과 패턴 재생/초안 조정은 각 명령의 stable ID와 데이터로 룸의 기존 검증 경로를 사용한다.
		LostArk::Shared::C2S_SPAWN_WORLD_ENTITY SpawnWorldEntity;

		LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST ValtanAudition;
		LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START
			ValtanPatternFlowStart;
		LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT
			ValtanPatternFlowStopAfterCurrent;
		LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST
			KoukuSaydonPatternAudition;
		LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST KoukuSaydonRaid;
		LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_DRAFT_CHUNK KoukuSaydonDraftChunk;

		// 아이템·장비·캐릭터 캡처/복원 요청이다. 원본 인벤토리 포인터가 아니라 메시지 값을 소유한다.
		LostArk::Shared::C2S_DEBUG_GIVE_ITEM DebugGiveItem;
		LostArk::Shared::C2S_USE_ITEM UseItem;
		LostArk::Shared::C2S_SET_EQUIPMENT SetEquipment;
		LostArk::Shared::C2S_BUY_ITEMS BuyItems;
		LostArk::Shared::C2S_RESTORE_CHARACTER RestoreCharacter;
		LostArk::Shared::C2S_UPGRADE_EQUIPMENT UpgradeEquipment;
		LostArk::Shared::C2S_CAPTURE_CHARACTER CaptureCharacter;
		LostArk::Shared::C2S_REPAIR_EQUIPMENT RepairEquipment;
		LostArk::Shared::C2S_DESPAWN_ALL_WORLD_ENTITIES DespawnAllWorldEntities;
		// NPC·trigger·월드 도구 요청은 룸 상태에서 적법한 전환과 저작 동작인지 확인한 뒤 처리한다.
		LostArk::Shared::C2S_CONFIRM_NPC_ENTRY ConfirmNpcEntry;
		LostArk::Shared::C2S_INTERACT_TRIGGER InteractTrigger;
		LostArk::Shared::C2S_DEBUG_WORLD_PLAYBACK DebugWorldPlayback;
        LostArk::Shared::C2S_MAHARAKA_AI_TUNING MaharakaAITuning;
		LostArk::Shared::C2S_RETURN_TO_BERN ReturnToBern;
		LostArk::Shared::C2S_GUIDE_CONTROL GuideControl;
		// 파티·레이드 동의와 매칭 입력을 발신 sessionId와 묶어 같은 룸 명령 순서로 전달한다.
		LostArk::Shared::C2S_PARTY_INVITE PartyInvite;
		LostArk::Shared::C2S_PARTY_INVITE_RESPOND PartyInviteRespond;
		LostArk::Shared::C2S_RAID_ENTRY_PROPOSE RaidEntryPropose;
		LostArk::Shared::C2S_RAID_ENTRY_RESPOND RaidEntryRespond;
		LostArk::Shared::C2S_COLOSSEUM_QUEUE_JOIN ColosseumQueueJoin;
		LostArk::Shared::C2S_COLOSSEUM_QUEUE_LEAVE ColosseumQueueLeave;
		LostArk::Shared::C2S_COLOSSEUM_RECRUIT ColosseumRecruit;
		LostArk::Shared::C2S_COLOSSEUM_LOAD_READY ColosseumLoadReady;
		LostArk::Shared::C2S_COLOSSEUM_RETURN ColosseumReturn;
		LostArk::Shared::C2S_GATE_PROGRESS_PROPOSE GateProgressPropose;
		LostArk::Shared::C2S_GATE_PROGRESS_RESPOND GateProgressRespond;
		// 채팅과 룸 ping도 연결 바인딩의 룸이 대상과 복제 범위를 결정한다.
		LostArk::Shared::C2S_CHAT Chat;
		LostArk::Shared::C2S_ROOM_PING RoomPing;

		// LEAVE는 일반 gameplay 큐 용량과 분리된 cleanup 큐를 사용한다.
		// Enqueue_Detailed가 세션별 중복을 합치고 Tick이 일반 명령보다 먼저 이 이유로 Leave를 처리한다.
		LostArk::Shared::PLAYER_DESPAWN_REASON eLeaveReason =
			LostArk::Shared::PLAYER_DESPAWN_REASON::DISCONNECTED;
	};
}

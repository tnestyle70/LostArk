#pragma once
//64 bit 표현을 위한 cstddef, cstdint? 정확히 어떤 부분들을 지원해주는 걸까?
#include <cstddef>
#include <cstdint>

//LostArk::Shared 안에서 관리가 되고, 해당 packet이 사용된다.

namespace LostArk::Shared
{
	// 해당 네트워크 프로토콜 버전이 맞지 않아서 계속해서 에러가 났었다.
	// 모든 팀원들과 같이 protocol 버전을 맞춰서 서버와의 계약을 동일하게 맞춰야 한다.
	// 이 버전 변수가 영향을 받는 곳은? 반대로 server의 어떤 부분이 수정되면 이 protocol version이 바뀌도록 설계가 되어있는 거지? 
	//uint8_t로 바꿔도 아무런 문제가 없는 거 맞지? 지금은 133인데 256까지 8bit가 2^8으로 표현이 가능하니까 그런 거잖아.
	inline constexpr std::uint8_t NETWORK_PROTOCOL_VERSION = 133;

	//이거 uint8_t로 바꿔도 문제 없지? 
	enum class WORLD_ID : std::uint8_t
	{
		BERN = 1,
		VALTAN_ARENA = 2,
		TRAINING_GROUND = 3,
		CHARACTER_SELECT_ARENA = 4,
		KOUKUSAYDON_ARENA = 5,
		MAHARAKA = 6,
		COLOSSEUM = 7,
		END
	};

	//World 진입에 대한 요청을 확인할 때 사용하는 API? 월드 진입에 대한 검증을 할 때 호출한다.
	[[nodiscard]]
	constexpr bool Is_Known_World_Id(const WORLD_ID worldId)
	{
		return WORLD_ID::BERN == worldId ||
			WORLD_ID::VALTAN_ARENA == worldId ||
			WORLD_ID::TRAINING_GROUND == worldId ||
			WORLD_ID::CHARACTER_SELECT_ARENA == worldId ||
			WORLD_ID::KOUKUSAYDON_ARENA == worldId ||
			WORLD_ID::MAHARAKA == worldId ||
			WORLD_ID::COLOSSEUM == worldId;
	}

	//캐릭터 선택에 대한 요청을 보낼 때, 해당 캐릭터에 대한 정보를 담아서 어떤 캐릭터에 대한 요청인지를
	//받을 수 있도록 한다.
	enum class CHARACTER_CLASS_ID : std::uint8_t
	{
		LANCE_MASTER = 0,
		GUNSLINGER = 1,
		SLAYER = 2,
		ARTIST = 3,
		DESTROYER = 4,
		DIMENSIONMASTER = 5,
		WARLORD = 6,
		GUARDIANKNIGHT = 7,
		END
	};

	//유효한 캐릭터 클래스인지를 검증하는 API
	//constexpr은 컴파일 타임에서 결정되는 함수? 왜 이 함수는 constexpr로 선언된 걸까?
	[[nodiscard]]
	constexpr bool Is_Known_Character_Class(const CHARACTER_CLASS_ID characterClass)
	{
		return static_cast<std::uint8_t>(characterClass) <
			static_cast<std::uint8_t>(CHARACTER_CLASS_ID::END);
	}

	enum class PLAYER_SKILL_KIND : std::uint8_t
	{
		ACTIVE = 0,
		COMBO = 1,
		HOLD = 2,
		// 첫 단계가 방어하고 피격으로 다음 counter 단계에 진입한다. 추가 키 입력으로 진행하는 combo와 구분한다.
		//카운터 - Q W E R이 카운터에 해당하는 스킬이다. 
		//실제 스킬에 대한 판정은 어디에서 수행?
		COUNTER = 3,
		/* KNOCKDOWN 상태에서 일어나는 단일 clip이며 해당 상태에서만 시작한다. */
		//KNOCKDOWN 기준으로 일어날 때, 사용되는 skill  kind
		STANDUP = 4,
		END
	};

	// protocol에 값이 있어도 runtime bundle이 준비되지 않을 수 있어 실제 입장 지원 class를 별도로 제한한다.
	[[nodiscard]]
	constexpr bool Is_Supported_Playable_Character_Class(
		const CHARACTER_CLASS_ID characterClass)
	{
		return CHARACTER_CLASS_ID::LANCE_MASTER == characterClass ||
			CHARACTER_CLASS_ID::GUNSLINGER == characterClass ||
			CHARACTER_CLASS_ID::SLAYER == characterClass ||
			CHARACTER_CLASS_ID::ARTIST == characterClass ||
			CHARACTER_CLASS_ID::DIMENSIONMASTER == characterClass ||
			CHARACTER_CLASS_ID::WARLORD == characterClass ||
			CHARACTER_CLASS_ID::GUARDIANKNIGHT == characterClass;
	}

	// 인간과 Server가 조종하는 guide·Waterpang·Colosseum AI를 구분하는 복제 값이다.
	//PLAYER를 컨트롤하는 AI의 Kind
	enum class PLAYER_CONTROL_KIND : std::uint8_t 
	{ 
		HUMAN = 0,
		GUIDE_AI = 1, 
		WATERPANG_AI = 2, 
		COLOSSEUM_MERCENARY_AI = 3 
	};

	//캐릭터에 대한 조작을 어떤 방식으로 하는 지에 대한 선택을 한다.
	constexpr bool Is_Known_Player_Control_Kind(PLAYER_CONTROL_KIND kind)
	{ 
		return kind == PLAYER_CONTROL_KIND::HUMAN ||
		kind == PLAYER_CONTROL_KIND::GUIDE_AI ||
		kind == PLAYER_CONTROL_KIND::WATERPANG_AI ||
		kind == PLAYER_CONTROL_KIND::COLOSSEUM_MERCENARY_AI;
	}

	//받는 타입에 대한 정보를 정리한 부분이다.
	enum class PACKET_TYPE : std::uint16_t
	{
		INVALID,

		C2S_ENTER_WORLD,

		S2C_ENTER_ACCEPTED,
		S2C_ENTER_REJECTED,
		S2C_PLAYER_SPAWNED,
		S2C_WORLD_ENTITY_SPAWNED,

		C2S_SPAWN_WORLD_ENTITY,

		S2C_WORLD_ENTITY_SPAWN_RESULT,

		C2S_MOVE,
		C2S_USE_SKILL,
		C2S_RELEASE_SKILL,
		C2S_UPDATE_SKILL_AIM,
		C2S_USE_ESTHER_SKILL,
		C2S_REVIVE_PLAYER,

		C2S_DEBUG_KILL_SELF,
		C2S_CHANGE_CHARACTER_CLASS,

		S2C_CHARACTER_CLASS_CHANGE_RESULT,
		S2C_WORLD_SNAPSHOT,

		C2S_CHAT,
		S2C_CHAT,

		S2C_PLAYER_DESPAWNED,
		S2C_WORLD_ENTITY_DESPAWNED,

		S2C_WORLD_DESTRUCTION_FULL_SYNC,
		S2C_WORLD_DESTRUCTION_DELTA,

		S2C_ENCOUNTER_PROP_SYNC,

		C2S_VALTAN_AUDITION_REQUEST,
		S2C_VALTAN_AUDITION_RESULT,

		C2S_DEBUG_GIVE_ITEM,
		S2C_INVENTORY_SNAPSHOT,

		C2S_USE_ITEM,
		C2S_DESPAWN_ALL_WORLD_ENTITIES,
		C2S_CONFIRM_NPC_ENTRY,

		// Append-only room-owned combat-object lifecycle.
		S2C_COMBAT_OBJECT_SPAWNED,
		S2C_COMBAT_OBJECT_DESPAWNED,

		// 즉시 요청 결과와 별도로 version이 붙은 완료·중단 lifecycle을 전달해 저작 도구가 실행을 추적한다.
		// AUDITION의 의미와 역할이 정확하게 뭘까?
		S2C_VALTAN_AUDITION_LIFECYCLE,

		// 불변 data revision의 준비 요청·응답·결과를 나누어 commit 여부를 전달한다. packet 식별 자체는 Release에서도 유지한다.
		// 이거는 정확히 어떤 요구를 보내고, 어떤 결과를 원하고, 어떤 처리를 진행을 하는 거지? 
		//DATA_REVISION이 정확히 뭘 요구하고 어떤 거에 대한 처리를 요청하는 구조인 걸까?
		C2S_DATA_REVISION_PREPARE_REQUEST,
		S2C_DATA_REVISION_PREPARE,
		C2S_DATA_REVISION_PREPARE_RESPONSE,
		S2C_DATA_REVISION_RESULT,

		// Debug 의사결정 추적 조회다. Release도 packet을 인식하고 REJECTED_RELEASE_BUILD로 답한다.
		C2S_VALTAN_DECISION_TRACE_QUERY,
		S2C_VALTAN_DECISION_TRACE_RESPONSE,

		// Boss Tool의 순서 있는 패턴 재생·정지·lifecycle 계약이다. packet 식별과 실행 허용 정책은 별도로 처리한다.
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START,
		S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT,
		C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT,
		S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE,

		// 같은 CGameRoom의 NetEntityId로 초대·수락·거절한다. nickname을 방 밖 플레이어 조회 key로 쓰지 않는다.
		C2S_PARTY_INVITE,
		S2C_PARTY_INVITE_RECEIVED,
		C2S_PARTY_INVITE_RESPOND,
		S2C_PARTY_ROSTER,
		S2C_PARTY_TRANSFER_RESULT,

		// Raid Clear screen's own "돌아가기" (return) button, Valtan Arena only.
		// Bern 복귀 요청 sequence를 보내고 기존 S2C_ENTER_ACCEPTED·S2C_ENTER_REJECTED로 이동 결과를 받는다.
		C2S_RETURN_TO_BERN,

		// 룸 소유 combat object의 의미 있는 타격을 reliable 단발 event로 보낸다. 합칠 수 있는 위치 snapshot과 달리 순서 장벽이며 같은 tick의 despawn 뒤에도 전달될 수 있다.
		S2C_COMBAT_OBJECT_PRESENTATION_EVENT,

		// 쿠크 저작 진입도 Server 방 이동을 거친다. stage 이동은 PLAYER_SPAWN placement를 지정하며 Client world Transform을 전송하지 않는다.
		C2S_DEBUG_ENTER_KOUKUSAYDON_ARENA,
		C2S_DEBUG_TELEPORT_TO_PLACEMENT,

		// Server가 시작을 확정한 world sequence instance를 방 전체에 reliable event로 전달한다. Client는 표현을 담당한다.
		S2C_WORLD_SEQUENCE_PLAY,

		// 파티 레이드 입장 전원 수락 투표. 입장을 propose -> prompt -> respond -> vote로
		// 통일한다(솔로는 1명 투표). Append-only이므로 이전 payload를 이 identity로 재해석하지 않는다.
		C2S_RAID_ENTRY_PROPOSE,
		S2C_RAID_ENTRY_PROMPT,
		C2S_RAID_ENTRY_RESPOND,
		S2C_RAID_ENTRY_VOTE,

		// 쿠크 패턴 재생 요청·결과·lifecycle이다. Release는 게시 패턴을 허용하고 draft·Mario test override는 typed 결과로 거부한다.
		C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST,
		S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT,
		S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE,

		C2S_DEBUG_TELEPORT_TO_POSITION,
		S2C_DEBUG_TELEPORT_TO_POSITION_RESULT,

		// F1의 광기 외형 전환 요청·결과다. Release에서도 packet은 인식하지만 실행은 typed 결과로 거부한다.
		C2S_DEBUG_SET_MADNESS_FORM,
		S2C_DEBUG_SET_MADNESS_FORM_RESULT,

		// Server가 trigger 안의 session에 제안을 보내고 이탈 시 거둔다. Client는 입력만 전달하며 위치·조건 재검증과 실행은 Server가 맡는다.
		S2C_INTERACT_PROMPT,
		C2S_INTERACT_TRIGGER,

		// Server 상호작용 mode의 slot 입력, HUD mode 변경 결과, Server 시계 기준 scene profile cue를 전달한다.
		C2S_INTERACTION_SLOT,
		C2S_DEBUG_SET_KOUKU_HUD_MODE,
		S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT,
		S2C_SCENE_PROFILE_APPLY,

		// Mario의 Debug 점프 요청·결과와 좌우 이동·정지를 typed 명령으로 보낸다.
		C2S_DEBUG_MARIO_JUMP,
		S2C_DEBUG_MARIO_JUMP_RESULT,
		C2S_MARIO_MOVE,
		C2S_DEBUG_WORLD_PLAYBACK,
		S2C_DEBUG_WORLD_PLAYBACK_RESULT,
		S2C_KOUKUSAYDON_BUNDLE_STATE,

		// Debug 빙고 cell 채우기는 별도 결과 없이 world snapshot의 board 상태로 반영한다.
		//디버그용 빙고 - 빙고 채우기, 빙고 폭탄, 빙고 해머
		C2S_DEBUG_BINGO_FILL,
		// Debug 빙고 폭탄은 요청자 자신의 player에 표시하고 기존 snapshot으로 반영한다.
		C2S_DEBUG_BINGO_BOMB,
		// Bingo hammer: Debug-only start on a random row or column.
		C2S_DEBUG_BINGO_HAMMER,
		// H키 탑승 전환의 요청·판정이다. 탑승 상태는 snapshot에 담고 결과는 요청이 처리되지 않은 이유를 전달한다.
		C2S_SET_VEHICLE_RIDING,
		S2C_SET_VEHICLE_RIDING_RESULT,
		C2S_MARIO_RETURN,
		S2C_MARIO_RETURN_RESULT,
		// 칭호 변경 요청·판정을 전달하며 장착된 칭호는 player snapshot으로 복제한다.
		C2S_SET_HONOR_TITLE,
		S2C_SET_HONOR_TITLE_RESULT,
		// 일반 스퀘어홀은 Server가 목적지를 검증하고 노래 action을 시작한다. 만료 시 재검증해 이동하며 중단되면 예약 이동을 취소한다. Colosseum에서는 노래 없이 Bern 전환을 예약한다.
		C2S_USE_SQUAREHOLE,
		// 관문 완료 뒤 리더·솔로가 다음 관문을 제안하고 멤버가 응답한다. Server가 완료 mask·투표·전환을 소유해 하나의 상태로 복제한다.
		C2S_GATE_PROGRESS_PROPOSE,
		C2S_GATE_PROGRESS_RESPOND,
		S2C_GATE_PROGRESS_STATE,
		C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST,
		S2C_KOUKUSAYDON_RAID_STATE,
		// 완료 관문 보스에 대한 플레이어별 기록 기여도를 레이드 보상 화면에 전달한다.
		S2C_RAID_MVP_RESULT,
		// 가방과 장비 slot 사이의 장착·해제 요청이다.
		C2S_SET_EQUIPMENT,
		// Debug wave 그룹의 살아 있는 몬스터를 제거하고 처음부터 재소환한다. Release에서는 무시하며 별도 결과 packet은 없다.
		C2S_DEBUG_RESUMMON_WAVE_MONSTERS,
		// Debug는 gauge·roster 순서를 건너뛰고 이름으로 Esther를 소환한다. Release에서는 무시하며 소환 상태는 world snapshot에 담는다.
		C2S_DEBUG_USE_ESTHER,
		C2S_DEBUG_KOUKUSAYDON_DRAFT_CHUNK,
		C2S_DEBUG_KILL_GATE_BOSSES,
		S2C_DEBUG_KILL_GATE_BOSSES_RESULT,
		C2S_SET_COOLDOWN_MODE,
		S2C_SET_COOLDOWN_MODE_RESULT,
		C2S_ROOM_PING,
		S2C_ROOM_PING,
		S2C_GUIDE_PROMPT,
		S2C_GUIDE_STATE,
		// Server가 상점·재고·가격·가방을 확인해 장바구니 전체를 적용하거나 보존하고 inventory snapshot으로 답한다.
		C2S_BUY_ITEMS,
		C2S_BALANCE_QUERY,
		S2C_BALANCE_SNAPSHOT,
		C2S_BALANCE_PATCH,
		S2C_BALANCE_RESULT,
		// One-shot restore of a saved character's inventory, purse and honor title
		// right after entering Bern. Every admitted request receives a typed result;
		// success follows the authoritative inventory snapshot in the reliable queue.
		C2S_RESTORE_CHARACTER,
		S2C_RESTORE_CHARACTER_RESULT,
		// Preserve Colosseum's published range; append repair and Waterpang tuning.
		C2S_COLOSSEUM_QUEUE_JOIN = 113,
		C2S_COLOSSEUM_QUEUE_LEAVE = 114,
		S2C_COLOSSEUM_QUEUE_STATE = 115,
		S2C_COLOSSEUM_MATCH_FOUND = 116,
		C2S_REPAIR_EQUIPMENT = 117,
		C2S_MAHARAKA_AI_TUNING = 118,
		S2C_MAHARAKA_AI_TUNING = 119,
		C2S_COLOSSEUM_RECRUIT = 120,
		S2C_COLOSSEUM_MATCH_STATE = 121,
		C2S_GUIDE_CONTROL = 122,
		C2S_COLOSSEUM_LOAD_READY = 123,
		C2S_COLOSSEUM_RETURN = 124,
		C2S_UPGRADE_EQUIPMENT = 125,
		S2C_UPGRADE_EQUIPMENT_RESULT = 126,
		C2S_CAPTURE_CHARACTER = 127,
		S2C_CAPTURE_CHARACTER_RESULT = 128
	};

	//TCP는 메시지 경계를 보존하지 않기 때문에, payload앞에 header를 둔다.
	//패킷별로 경계를 구분해야, packet을 하나의 의미 단위로 읽을 수 있다.
	// Header는 uint32 전체 Frame 크기 4바이트 + uint16 PacketType 2바이트 = 6바이트다.
	inline constexpr std::size_t PACKET_HEADER_BYTES = 6;
	// Header를 포함한 Frame 하나의 최대 크기는 64 KiB다.
	inline constexpr std::uint32_t MAX_PACKET_BYTES =
		64u * 1024u;
	// Parser 누적 버퍼는 최대 Frame 4개 분량만 허용해 비정상 입력의 무제한 증가를 막는다.
	inline constexpr std::size_t MAX_BUFFERED_PACKET_BYTES =
		static_cast<std::size_t>(MAX_PACKET_BYTES) * 4u;
	//알려진 패킷인지를 검사하는 함수, client에서 정해진 규칙대로 보낸 packet인지를 검사 
	[[nodiscard]]
	constexpr bool Is_Known_Packet_Type(
		PACKET_TYPE packetType)
	{
		switch (packetType)
		{
		case PACKET_TYPE::C2S_ENTER_WORLD:
		case PACKET_TYPE::S2C_ENTER_ACCEPTED:
		case PACKET_TYPE::S2C_ENTER_REJECTED:
		case PACKET_TYPE::S2C_PLAYER_SPAWNED:
		case PACKET_TYPE::S2C_WORLD_ENTITY_SPAWNED:
		case PACKET_TYPE::S2C_COMBAT_OBJECT_SPAWNED:
		case PACKET_TYPE::C2S_SPAWN_WORLD_ENTITY:
		case PACKET_TYPE::S2C_WORLD_ENTITY_SPAWN_RESULT:
		case PACKET_TYPE::C2S_MOVE:
		case PACKET_TYPE::C2S_USE_SKILL:
		case PACKET_TYPE::C2S_RELEASE_SKILL:
		case PACKET_TYPE::C2S_UPDATE_SKILL_AIM:
		case PACKET_TYPE::C2S_USE_ESTHER_SKILL:
		case PACKET_TYPE::C2S_REVIVE_PLAYER:
		case PACKET_TYPE::C2S_DEBUG_KILL_GATE_BOSSES:
		case PACKET_TYPE::S2C_DEBUG_KILL_GATE_BOSSES_RESULT:
		case PACKET_TYPE::C2S_SET_COOLDOWN_MODE:
		case PACKET_TYPE::S2C_SET_COOLDOWN_MODE_RESULT:
		case PACKET_TYPE::C2S_DEBUG_KILL_SELF:
		case PACKET_TYPE::C2S_CHANGE_CHARACTER_CLASS:
		case PACKET_TYPE::S2C_CHARACTER_CLASS_CHANGE_RESULT:
		case PACKET_TYPE::S2C_WORLD_SNAPSHOT:
		case PACKET_TYPE::C2S_ROOM_PING:
		case PACKET_TYPE::S2C_ROOM_PING:
		case PACKET_TYPE::C2S_CHAT:
		case PACKET_TYPE::S2C_GUIDE_PROMPT:
		case PACKET_TYPE::S2C_GUIDE_STATE:
		case PACKET_TYPE::S2C_CHAT:
		case PACKET_TYPE::S2C_PLAYER_DESPAWNED:
		case PACKET_TYPE::S2C_WORLD_ENTITY_DESPAWNED:
		case PACKET_TYPE::S2C_COMBAT_OBJECT_DESPAWNED:
		case PACKET_TYPE::S2C_VALTAN_AUDITION_LIFECYCLE:
		case PACKET_TYPE::C2S_DATA_REVISION_PREPARE_REQUEST:
		case PACKET_TYPE::S2C_DATA_REVISION_PREPARE:
		case PACKET_TYPE::C2S_DATA_REVISION_PREPARE_RESPONSE:
		case PACKET_TYPE::S2C_DATA_REVISION_RESULT:
		case PACKET_TYPE::C2S_VALTAN_DECISION_TRACE_QUERY:
		case PACKET_TYPE::S2C_VALTAN_DECISION_TRACE_RESPONSE:
		case PACKET_TYPE::C2S_DEBUG_VALTAN_PATTERN_FLOW_START:
		case PACKET_TYPE::S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT:
		case PACKET_TYPE::C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT:
		case PACKET_TYPE::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE:
		case PACKET_TYPE::S2C_WORLD_DESTRUCTION_FULL_SYNC:
		case PACKET_TYPE::S2C_WORLD_DESTRUCTION_DELTA:
		case PACKET_TYPE::S2C_ENCOUNTER_PROP_SYNC:
		case PACKET_TYPE::C2S_VALTAN_AUDITION_REQUEST:
		case PACKET_TYPE::S2C_VALTAN_AUDITION_RESULT:
		case PACKET_TYPE::C2S_DEBUG_GIVE_ITEM:
		case PACKET_TYPE::S2C_INVENTORY_SNAPSHOT:
		case PACKET_TYPE::C2S_USE_ITEM:
		case PACKET_TYPE::C2S_DESPAWN_ALL_WORLD_ENTITIES:
		case PACKET_TYPE::C2S_CONFIRM_NPC_ENTRY:
		case PACKET_TYPE::C2S_RETURN_TO_BERN:
		case PACKET_TYPE::S2C_COMBAT_OBJECT_PRESENTATION_EVENT:
		case PACKET_TYPE::C2S_DEBUG_ENTER_KOUKUSAYDON_ARENA:
		case PACKET_TYPE::C2S_DEBUG_TELEPORT_TO_PLACEMENT:
		case PACKET_TYPE::C2S_DEBUG_TELEPORT_TO_POSITION:
		case PACKET_TYPE::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT:
		case PACKET_TYPE::C2S_PARTY_INVITE:
		case PACKET_TYPE::S2C_PARTY_INVITE_RECEIVED:
		case PACKET_TYPE::C2S_PARTY_INVITE_RESPOND:
		case PACKET_TYPE::S2C_PARTY_ROSTER:
		case PACKET_TYPE::S2C_PARTY_TRANSFER_RESULT:
		case PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY:
		case PACKET_TYPE::C2S_RAID_ENTRY_PROPOSE:
		case PACKET_TYPE::S2C_RAID_ENTRY_PROMPT:
		case PACKET_TYPE::C2S_RAID_ENTRY_RESPOND:
		case PACKET_TYPE::S2C_RAID_ENTRY_VOTE:
		case PACKET_TYPE::C2S_DEBUG_KOUKUSAYDON_DRAFT_CHUNK:
		case PACKET_TYPE::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST:
		case PACKET_TYPE::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT:
		case PACKET_TYPE::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE:
		case PACKET_TYPE::S2C_KOUKUSAYDON_BUNDLE_STATE:
		case PACKET_TYPE::C2S_DEBUG_SET_MADNESS_FORM:
		case PACKET_TYPE::S2C_DEBUG_SET_MADNESS_FORM_RESULT:
		case PACKET_TYPE::S2C_INTERACT_PROMPT:
		case PACKET_TYPE::C2S_INTERACT_TRIGGER:
		case PACKET_TYPE::C2S_DEBUG_WORLD_PLAYBACK:
		case PACKET_TYPE::S2C_DEBUG_WORLD_PLAYBACK_RESULT:
		case PACKET_TYPE::C2S_DEBUG_BINGO_FILL:
		case PACKET_TYPE::C2S_DEBUG_BINGO_BOMB:
		case PACKET_TYPE::C2S_DEBUG_BINGO_HAMMER:
		case PACKET_TYPE::C2S_INTERACTION_SLOT:
		case PACKET_TYPE::C2S_DEBUG_SET_KOUKU_HUD_MODE:
		case PACKET_TYPE::S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT:
		case PACKET_TYPE::S2C_SCENE_PROFILE_APPLY:
		case PACKET_TYPE::C2S_DEBUG_MARIO_JUMP:
		case PACKET_TYPE::S2C_DEBUG_MARIO_JUMP_RESULT:
		case PACKET_TYPE::C2S_MARIO_MOVE:
		case PACKET_TYPE::C2S_SET_VEHICLE_RIDING:
		case PACKET_TYPE::S2C_SET_VEHICLE_RIDING_RESULT:
		case PACKET_TYPE::C2S_MARIO_RETURN:
		case PACKET_TYPE::S2C_MARIO_RETURN_RESULT:
		case PACKET_TYPE::C2S_SET_HONOR_TITLE:
		case PACKET_TYPE::S2C_SET_HONOR_TITLE_RESULT:
		case PACKET_TYPE::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST:
		case PACKET_TYPE::S2C_KOUKUSAYDON_RAID_STATE:
		case PACKET_TYPE::C2S_USE_SQUAREHOLE:
		case PACKET_TYPE::C2S_GATE_PROGRESS_PROPOSE:
		case PACKET_TYPE::C2S_GATE_PROGRESS_RESPOND:
		case PACKET_TYPE::S2C_GATE_PROGRESS_STATE:
		case PACKET_TYPE::S2C_RAID_MVP_RESULT:
		case PACKET_TYPE::C2S_SET_EQUIPMENT:
		case PACKET_TYPE::C2S_DEBUG_RESUMMON_WAVE_MONSTERS:
		case PACKET_TYPE::C2S_DEBUG_USE_ESTHER:
		case PACKET_TYPE::C2S_BUY_ITEMS:
		case PACKET_TYPE::C2S_RESTORE_CHARACTER:
		case PACKET_TYPE::S2C_RESTORE_CHARACTER_RESULT:
		case PACKET_TYPE::C2S_UPGRADE_EQUIPMENT:
		case PACKET_TYPE::S2C_UPGRADE_EQUIPMENT_RESULT:
		case PACKET_TYPE::C2S_CAPTURE_CHARACTER:
		case PACKET_TYPE::S2C_CAPTURE_CHARACTER_RESULT:
		case PACKET_TYPE::C2S_MAHARAKA_AI_TUNING:
		case PACKET_TYPE::S2C_MAHARAKA_AI_TUNING:
		case PACKET_TYPE::C2S_REPAIR_EQUIPMENT:
		case PACKET_TYPE::C2S_BALANCE_QUERY:
		case PACKET_TYPE::S2C_BALANCE_SNAPSHOT:
		case PACKET_TYPE::C2S_BALANCE_PATCH:
		case PACKET_TYPE::S2C_BALANCE_RESULT:
		case PACKET_TYPE::C2S_COLOSSEUM_QUEUE_JOIN:
		case PACKET_TYPE::C2S_COLOSSEUM_QUEUE_LEAVE:
		case PACKET_TYPE::S2C_COLOSSEUM_QUEUE_STATE:
		case PACKET_TYPE::S2C_COLOSSEUM_MATCH_FOUND:
		case PACKET_TYPE::C2S_GUIDE_CONTROL:
		case PACKET_TYPE::C2S_COLOSSEUM_RECRUIT:
		case PACKET_TYPE::C2S_COLOSSEUM_LOAD_READY:
		case PACKET_TYPE::C2S_COLOSSEUM_RETURN:
		case PACKET_TYPE::S2C_COLOSSEUM_MATCH_STATE:
			return true;
		default:
			return  false;
		}
	}

	//inline constexpr인 이유
	//inline : 이 헤더를 여러 .cpp가 include해도 동일한 변수 정의로 취급한다.
	//constexpr : 컴파일 타임 시간 상수
	inline constexpr std::size_t MAX_NICKNAME_BYTES = 32;

	// 생성 voice type은 class별 1~8이다. wire에는 0을 허용하지 않는다.
	inline constexpr std::uint8_t MIN_VOICE_TYPE = 1;
	inline constexpr std::uint8_t MAX_VOICE_TYPE = 8;

	// 같은 방 인간 파티의 최대 인원이다. Server의 초대·수락과 Shared roster codec이 이 상한을 사용한다.
	// powershell로 실행하는 harness가 있나? 4/4? ROOM_RULL? 이거 늘렸는데 레거시?
	//아니면 4명 검증 테스트하려고 추가한 건가?
	inline constexpr std::size_t MAX_PARTY_MEMBERS = 4;

	inline constexpr std::size_t MAX_VALTAN_RAID_PLAYERS = 8;

	// 채팅 wire 본문의 최대 byte 수다. Client의 256-byte 입력 buffer에는 문자열 종료 공간도 필요하다.
	inline constexpr std::size_t MAX_CHAT_TEXT_BYTES = 256;
	// 저작 world sequence의 stable instance ID와 같은 byte 상한이다.
	inline constexpr std::size_t MAX_SEQUENCE_INSTANCE_ID_BYTES = 128;
}

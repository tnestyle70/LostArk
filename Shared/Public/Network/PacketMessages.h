#pragma once

#include "GameplayDataRevision.h"
#include "Gameplay/BalanceNumericContract.h"
#include "Network/PacketType.h"
#include "NetworkIds.h"

#include <array>
#include <string>
#include <string_view>
#include <limits>
#include <vector>
#include <optional>

//character의 class와 nickname용 packet
namespace LostArk::Shared
{
	inline constexpr std::size_t MAX_REQUIRED_PINNED_GAMEPLAY_REVISIONS = 16u;

	//harness가 직접 write u8, write string을 호출하기 때문에, client와 server가 같은 함수를 쓰도록,
	//shared로 옮긴다.

	class CPacketReader;
	class CPacketWriter;

	[[nodiscard]] bool Is_Valid_PlayerNickname(
		std::string_view nickname) noexcept;

	[[nodiscard]] bool Is_Valid_VoiceType(
		std::uint8_t voiceType) noexcept;
	inline constexpr std::size_t MAX_PLAYER_APPEARANCE_BYTES = 16384u;
	// 용량을 제한한 수치형 커스터마이징 preset을 검증한다. 경로나 Client가 고른 runtime asset을 받는 필드가 아니다.
	// 빈 문자열은 직업 기본 외형을 뜻하며, 구체적인 preset 문법과 직업 일치는 CPP에서 검사한다.
	[[nodiscard]] bool Is_Valid_PlayerAppearance(std::string_view text, CHARACTER_CLASS_ID characterClass);

	// world sequence 문서와 맞춘 stable ID 문자 집합과 길이만 검사한다.
	// 이 검사를 통과해도 해당 ID의 실제 문서·리소스 존재나 Client 로드 성공까지 보장하지 않는다.
	[[nodiscard]] bool Is_Valid_SequenceInstanceId(
		std::string_view instanceId) noexcept;

	// Client가 월드 입장 의사를 보낸다. 아래 기본값만으로 입장 승인이 성립하지 않으며 Server가 protocol·직업·입력 계약을 검증한다.
	struct C2S_ENTER_WORLD
	{
		std::uint16_t iProtocolVersion = NETWORK_PROTOCOL_VERSION;
		WORLD_ID eWorldId = WORLD_ID::BERN;

		CHARACTER_CLASS_ID eCharacterClass =
			CHARACTER_CLASS_ID::END;

		std::string strNickName;
		std::uint8_t iVoiceType = MIN_VOICE_TYPE;
		std::string strAppearanceJson;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_ENTER_WORLD& message);

	bool Read_Message(
		CPacketReader& reader,
		C2S_ENTER_WORLD& message);

	// Server가 승인한 월드와 player/entity ID, 활성·필수 pinned gameplay revision을 돌려준다.
	struct S2C_ENTER_ACCEPTED
	{
		std::uint16_t iProtocolVersion = NETWORK_PROTOCOL_VERSION;
		WORLD_ID eWorldId = WORLD_ID::BERN;

		PLAYER_ID iPlayerId =
			INVALID_PLAYER_ID;

		NET_ENTITY_ID iNetEntityId =
			INVALID_NET_ENTITY_ID;

		GameplayDataRevision ActiveGameplayRevision{};
		std::vector<GameplayDataRevision> RequiredPinnedGameplayRevisions;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_ENTER_ACCEPTED& message);

	bool Read_Message(
		CPacketReader& reader,
		S2C_ENTER_ACCEPTED& message);

	enum class ENTER_WORLD_REJECTION_REASON : std::uint8_t
	{
		ROOM_FULL,
		END
	};

	struct S2C_ENTER_REJECTED
	{
		std::uint16_t iProtocolVersion = NETWORK_PROTOCOL_VERSION;
		WORLD_ID eWorldId = WORLD_ID::END;
		ENTER_WORLD_REJECTION_REASON eReason =
			ENTER_WORLD_REJECTION_REASON::END;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_ENTER_REJECTED& message);

	bool Read_Message(
		CPacketReader& reader,
		S2C_ENTER_REJECTED& message);

	// Server가 생성한 플레이어의 초기 표현 정보를 전달한다. 이후 위치·행동·전투 상태는 PLAYER_SNAPSHOT이 갱신한다.
	struct S2C_PLAYER_SPAWNED
	{
        // Server 소유 Waterpang 참가자를 그릴 NPC archetype이다. Client는 이 고정 ID로 외형을 찾는다.
        std::string strWaterpangNpcArchetypeId;
		PLAYER_ID iPlayerId = INVALID_PLAYER_ID;
		NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
		CHARACTER_CLASS_ID eCharacterClass = CHARACTER_CLASS_ID::END;
		std::string strNickName;
		//서버 기준 최초 생성 위치 
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		//서버 기준 Y축 회전 각도
		float fYawDegrees = 0.f;
		PLAYER_CONTROL_KIND eControlKind = PLAYER_CONTROL_KIND::HUMAN;
		std::uint8_t iVoiceType = MIN_VOICE_TYPE;
		std::string strAppearanceJson;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_PLAYER_SPAWNED& spawned);

	bool Read_Message(
		CPacketReader& reader,
		S2C_PLAYER_SPAWNED& spawned);

	inline constexpr std::size_t MAX_STABLE_NETWORK_ID_BYTES = 128;
	inline constexpr std::size_t MAX_WORLD_SNAPSHOT_ENTITIES = 256;

	struct C2S_SPAWN_WORLD_ENTITY
	{
		std::string strPlacementId;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_SPAWN_WORLD_ENTITY& message);
	bool Read_Message(
		CPacketReader& reader,
		C2S_SPAWN_WORLD_ENTITY& message);

	enum class WORLD_ENTITY_SPAWN_RESULT : std::uint8_t
	{
		SPAWNED,
		ALREADY_EXISTS,
		ACTIVATED,
		REJECTED,
		END
	};

	struct S2C_WORLD_ENTITY_SPAWN_RESULT
	{
		std::string strPlacementId;
		WORLD_ENTITY_SPAWN_RESULT eResult =
			WORLD_ENTITY_SPAWN_RESULT::REJECTED;
		NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_WORLD_ENTITY_SPAWN_RESULT& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_WORLD_ENTITY_SPAWN_RESULT& message);

	enum class WORLD_ENTITY_KIND : std::uint8_t
	{
		NPC,
		BOSS,
		MONSTER,
		END
	};

	enum class WORLD_ENTITY_ACTION : std::uint8_t
	{
		IDLE,
		CHASE,
		PATTERN_WINDUP,
		PATTERN_ACTIVE,
		PATTERN_RECOVERY,
		DEAD,
		END
	};

	// 2관문 Large Saydon은 기믹 공격의 주체지만 플레이어가 피해를 줄 대상에서는 제외한다.
	// Client targeting과 피격 표현도 같은 archetype 규칙을 공유한다. 다른 ID의 존재·생존 여부까지 검사하는 함수는 아니다.
	[[nodiscard]] inline bool Is_PlayerDamageableWorldArchetype(
		const std::string_view archetypeId) noexcept
	{
		return archetypeId != "BOSS_KOUKUSAYDON_G2_BIG_SAYDON";
	}

	struct S2C_WORLD_ENTITY_SPAWNED
	{
		NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
		// 고정된 종속 보스 관계다. 0은 소유 보스가 없는 독립 entity를 뜻한다.
		NET_ENTITY_ID iOwnerBossNetEntityId = INVALID_NET_ENTITY_ID;
		WORLD_ENTITY_KIND eKind = WORLD_ENTITY_KIND::END;
		std::string strArchetypeId;
		std::string strEncounterId;
		std::string strPlacementId;
		// 생성 순간의 action ID이며 snapshot의 strActionId와 같은 ID 영역을 사용한다.
		// 행동 중 생성되는 에스더 소환수도 다음 snapshot 전부터 해당 clip을 고를 수 있다. 빈 값은 idle 생성이다.
		// 이 메시지에는 action 시작 tick이 없으므로 이후 snapshot이 행동 시계를 보완한다.
		std::string strActionId;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;
		// Server가 정한 XZ 전투 몸체 반지름(m)이다. 전투 몸체가 없는 NPC 표현은 0을 보낸다.
		float fCollisionRadius = 0.f;
		// 생성 시 이 entity의 정의를 소유하는 고정 gameplay generation이다.
		// 동일한 정의 바이트를 유지한 live activation 뒤에도 기존 occurrence가 남을 수 있어 room 활성 revision과 다를 수 있다.
		GameplayDataRevision PinnedDefinitionRevision{};
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_WORLD_ENTITY_SPAWNED& spawned);
	bool Read_Message(
		CPacketReader& reader,
		S2C_WORLD_ENTITY_SPAWNED& spawned);

	// Client의 종속 보스 입장 검사는 먼저 도착한 owner를 참조한다. 중첩 소유는 허용하지 않는다.
	// owner가 있으면 같은 encounter·pinned revision의 독립 BOSS여야 한다.
	// 함수는 전달된 두 구조체만 검사하며 owner를 직접 검색하거나 생성하지 않는다. 독립 entity는 pOwner=nullptr여야 한다.
	bool Is_Valid_WorldEntitySpawnOwner(
		const S2C_WORLD_ENTITY_SPAWNED& spawned,
		const S2C_WORLD_ENTITY_SPAWNED* pOwner);

	enum class WORLD_ENTITY_DESPAWN_REASON : std::uint8_t
	{
		REMOVED = 0,
		DEAD,
		END
	};

	struct S2C_WORLD_ENTITY_DESPAWNED
	{
		NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
		// 최종 HP=0 snapshot을 보지 못해도 reliable DEAD 제거 사유로 사망을 표현할 수 있다.
		WORLD_ENTITY_DESPAWN_REASON eReason = WORLD_ENTITY_DESPAWN_REASON::REMOVED;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_WORLD_ENTITY_DESPAWNED& despawned);
	bool Read_Message(
		CPacketReader& reader,
		S2C_WORLD_ENTITY_DESPAWNED& despawned);

	struct S2C_COMBAT_OBJECT_SPAWNED
	{
		COMBAT_OBJECT_ID iCombatObjectId = INVALID_COMBAT_OBJECT_ID;
		NET_ENTITY_ID iSourceNetEntityId = INVALID_NET_ENTITY_ID;
		std::uint32_t iSpawnTick = 0;
		std::uint32_t iServerTick = 0;
		std::string strCombatObjectArchetypeId;
		std::string strClientVisualId;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;
		// occurrence 생성 시 정한 균일 배율이다. 표현과 공격 도형이 같은 배율을 사용한다.
		float fUniformScale = 1.f;
		GameplayDataRevision PinnedDefinitionRevision{};
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_COMBAT_OBJECT_SPAWNED& spawned);
	bool Read_Message(
		CPacketReader& reader,
		S2C_COMBAT_OBJECT_SPAWNED& spawned);

	struct S2C_COMBAT_OBJECT_DESPAWNED
	{
		COMBAT_OBJECT_ID iCombatObjectId = INVALID_COMBAT_OBJECT_ID;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_COMBAT_OBJECT_DESPAWNED& despawned);
	bool Read_Message(
		CPacketReader& reader,
		S2C_COMBAT_OBJECT_DESPAWNED& despawned);

	enum class COMBAT_OBJECT_PRESENTATION_EVENT_KIND : std::uint8_t
	{
		HIT_PULSE,
		END
	};

	// 전투 오브젝트가 발생시킨 reliable 표현 이벤트다. Server는 Sound/Effect asset 이름 대신 stable hit identity와 당시 pose를 보낸다.
	// CombatObjectRuntime이 큐에 쌓고 GameRoom은 spawn → presentation event → despawn 순서로 송신한다.
	// Client는 revision·owner를 확인한 뒤 로컬 표현 데이터로 asset을 찾는다. 이 메시지 자체가 피해를 다시 적용하지 않는다.
	// 현재 발탄 V1 hit/armed Effect는 이벤트의 월드 위치와 yaw를 사용한다. Sound는 로컬 cue로 재생하며 3D 위치 음향까지 보장하지 않는다.
	// 이벤트에 pose가 있어 전투 오브젝트 제거와 같은 tick의 종결 표현도 가능하지만, 모든 소비자가 live owner/record 없이 처리되는 것은 아니다.
	struct S2C_COMBAT_OBJECT_PRESENTATION_EVENT
	{
		std::uint64_t iEventSequence = 0u;
		std::uint32_t iServerTick = 0u;
		COMBAT_OBJECT_ID iCombatObjectId = INVALID_COMBAT_OBJECT_ID;
		NET_ENTITY_ID iSourceNetEntityId = INVALID_NET_ENTITY_ID;
		COMBAT_OBJECT_PRESENTATION_EVENT_KIND eKind =
			COMBAT_OBJECT_PRESENTATION_EVENT_KIND::END;
		std::string strCombatObjectArchetypeId;
		std::string strOwnerPatternId;
		std::string strOwnerStageActionId;
		// 타격에 연결된 pulse는 hitId를 담는다. 시각 표현 전용 typed object는 같은 wire 필드에 stable presentationEventId를 담는다.
		// 필드 이름이 strHitId여도 모든 이벤트가 HP 피해 성공을 뜻하지 않는다.
		std::string strHitId;
		std::uint32_t iRepeatIndex = 0u;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;
		GameplayDataRevision PinnedDefinitionRevision{};
	};

	// 입력을 검증한 뒤 기록한다. 명시적인 false 반환은 기존 writer를 보존한다.
	// 기록 중 할당 예외까지 rollback하는 함수는 아니다.
	bool Write_Message(
		CPacketWriter& writer,
		const S2C_COMBAT_OBJECT_PRESENTATION_EVENT& event);
	// 지역 decoded 값으로 읽은 뒤 성공 때만 event를 교체한다.
	// false는 기존 event를 보존하지만 reader의 소비 위치까지 되돌리지는 않는다.
	bool Read_Message(
		CPacketReader& reader,
		S2C_COMBAT_OBJECT_PRESENTATION_EVENT& event);

	//server와 client가 player제거으 byte 순서를 똑같이 사용하기 위해서이다.
	//H 계약 : NetEntityId와 제거 이유 enun을 값으로 선언하고, writer/reaeder/
	//overload를 공개한다.
	//cpp 흐름 : 모든 입력 검증 -> ID U32 reasonU8 기록. 
	//읽을 때는 지역 변수로 전부 복원하고 검증이 끝난 뒤 출력 구조체에 한 번만 commit한다.
	enum class PLAYER_DESPAWN_REASON : std::uint8_t
	{
		DISCONNECTED,
		LEFT_ROOM,
		KICKED,
		LEVEL_CHANGED,
		END
	};
	//player entity id와 reason에 대한 정보를 enum으로 들고있다.
	struct S2C_PLAYER_DESPAWNED
	{
		NET_ENTITY_ID iNetEntityId =
			INVALID_NET_ENTITY_ID;

		PLAYER_DESPAWN_REASON eReason =
			PLAYER_DESPAWN_REASON::END;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_PLAYER_DESPAWNED& message);

	bool Read_Message(
		CPacketReader& reader,
		S2C_PLAYER_DESPAWNED& message);

	//플레이어 이동과 스냅샷을 server에게 전달하고 동기화 시키는 것까지 구현
	inline constexpr std::size_t
		MAX_WORLD_SNAPSHOT_PLAYERS = 32;
	// 동시에 유지할 수 있는 skill cooldown 목록의 상한이다. 여러 ACTIVE 스킬의 HUD 쿨다운을 빠뜨리지 않도록 여유를 둔다.
	inline constexpr std::size_t MAX_PLAYER_COOLDOWNS = 32;
	// 한 WORLD_SNAPSHOT에 담을 피해·회복·기믹 표시 이벤트 목록의 상한이다. 전투 전체 누적 개수가 아니다.
	// 이 값이 actor당 한 tick에 타격을 한 번만 허용한다는 뜻은 아니다.
	inline constexpr std::size_t MAX_DAMAGE_EVENTS = 64;
	// WORLD_ENTITY_SNAPSHOT::iBrokenArmorMask가 표현하는 파괴 가능 방어구 판의 최대 수다.
	// 판 하나당 bit 하나를 사용하며 Server 정의와 Client의 해당 part 표현이 이 상한을 공유한다.
	inline constexpr std::uint8_t MAX_WORLD_ENTITY_ARMOR_PLATES = 4;
	// 보스 전투 변화는 해당 tick의 표현 이벤트이며, 지속 상태는 WORLD_ENTITY_SNAPSHOT에 따로 담는다.
	// 표현 이벤트를 놓쳤다는 이유로 Client가 지속 상태를 재계산하지 않는다.
	inline constexpr std::size_t MAX_BOSS_COMBAT_EVENTS = 64;
	inline constexpr std::size_t MAX_COMBAT_OBJECTS_PER_SNAPSHOT = 128;
	// publisher의 comboStages 2..8개 계약에 맞춘 최댓값이다.
	inline constexpr std::uint8_t MAX_COMBO_STAGES = 8;
	using SKILL_ID = std::uint32_t;
	inline constexpr SKILL_ID INVALID_SKILL_ID = 0;
	//이게 플레이어의 스킬을 의미하는 건가?
	// 아래 enum은 스킬 종류가 아니라 정지/이동 상태다. 실제 스킬은 SKILL_ID와 action 상태로 구분한다.
	enum class PLAYER_LOCOMOTION_STATE : std::uint8_t
	{
		IDLE,
		MOVING,
		END
	};
	// Client의 이동 의도와 Server가 복제하는 비행 단계다. 일반 목표 이동과 비행 방향 입력을 구분한다.
	enum class PLAYER_MOVE_INTENT : std::uint8_t
	{
		GROUND_GOAL,
		VEHICLE_FLIGHT,
		END
	};
	enum class VEHICLE_FLIGHT_PHASE : std::uint8_t
	{
		GROUNDED,
		TAKEOFF,
		FLYING,
		LANDING,
		END
	};
	inline constexpr VEHICLE_ID ANCIENT_SEA_VEHICLE_ID = 9523u;

	struct C2S_MOVE
	{
		std::uint32_t iClientSequence = 0;

		float fGoalX = 0.f;
		float fGoalZ = 0.f;
		// 비행 의도일 때 fGoalX/Z는 월드 XZ 단위 방향이고 fVerticalInput은 수직 입력이다. 이동 확정 위치를 보내는 계약이 아니다.
		PLAYER_MOVE_INTENT eIntent = PLAYER_MOVE_INTENT::GROUND_GOAL;
		float fVerticalInput = 0.f;
	};
	//근데 read가 const가 붙어야 하는 거 아닌가?
	// Write는 입력 message를 읽기만 하므로 const이고, Read는 출력 message와 reader의 위치를 갱신하므로 const가 아니다.
	bool Write_Message(
		CPacketWriter& writer,
		const C2S_MOVE& message);

	bool Read_Message(
		CPacketReader& reader,
		C2S_MOVE& message);

	// AIM_POINT는 월드의 조준점을, GROUND_POINT는 정확한 지면 목표 XZ를 요청한다.
	// Server는 skill별 허용 의도와 사거리·navigation을 확인한 뒤 action 목표로 승인한다.
	// 좌표만 바꿔 일반 방향 skill을 지면 지정 skill로 사용할 수 없도록 종류를 별도로 전달한다.
	enum class SKILL_TARGET_INTENT_KIND : std::uint8_t
	{
		AIM_POINT,
		GROUND_POINT,
		END
	};

	// Client 입력에는 player/entity ID가 없다. Server가 이 명령을 보낸 session에서 조작할 actor를 찾는다.
	struct C2S_USE_SKILL
	{
		std::uint32_t iClientSequence = 0;
		SKILL_ID iSkillId = INVALID_SKILL_ID;
		SKILL_TARGET_INTENT_KIND eTargetIntent =
			SKILL_TARGET_INTENT_KIND::AIM_POINT;
		float fAimX = 0.f;
		float fAimZ = 0.f;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_USE_SKILL& message);

	bool Read_Message(
		CPacketReader& reader,
		C2S_USE_SKILL& message);

	// HOLD skill의 키 해제를 전달해 loop 종료를 요청한다.
	// COMBO 단계 입력은 USE_SKILL 명령을 반복해 전달하므로 이 해제 메시지와 구분한다.
	struct C2S_RELEASE_SKILL
	{
		std::uint32_t iClientSequence = 0;
		SKILL_ID iSkillId = INVALID_SKILL_ID;
	};

	// 실행 중 선택하는 cooldown 정책 이름이다. EXE의 Debug/Release 빌드 구성과 동일한 개념이 아니다.
	enum class COOLDOWN_MODE : std::uint8_t
	{
		DEBUG_THREE_SECONDS,
		RELEASE_AUTHORED,
		END
	};
	struct C2S_SET_COOLDOWN_MODE
	{
		std::uint32_t iRequestSequence = 0;
		WORLD_ID eWorldId = WORLD_ID::END;
		COOLDOWN_MODE eMode = COOLDOWN_MODE::DEBUG_THREE_SECONDS;
	};
	enum class SET_COOLDOWN_MODE_RESULT : std::uint8_t
	{
		ACCEPTED,
		WRONG_WORLD,
		INVALID_PLAYER,
		STALE_REQUEST,
		END
	};
	struct S2C_SET_COOLDOWN_MODE_RESULT
	{
		std::uint32_t iRequestSequence = 0;
		WORLD_ID eWorldId = WORLD_ID::END;
		COOLDOWN_MODE eMode = COOLDOWN_MODE::DEBUG_THREE_SECONDS;
		SET_COOLDOWN_MODE_RESULT eResult = SET_COOLDOWN_MODE_RESULT::INVALID_PLAYER;
	};
	bool Write_Message(CPacketWriter&, const C2S_SET_COOLDOWN_MODE&);
	bool Read_Message(CPacketReader&, C2S_SET_COOLDOWN_MODE&);
	bool Write_Message(CPacketWriter&, const S2C_SET_COOLDOWN_MODE_RESULT&);
	bool Read_Message(CPacketReader&, S2C_SET_COOLDOWN_MODE_RESULT&);

	// 테스트용 처치 의도다. 관찰한 archetype은 오래된 화면을 구분하는 조건이며 실제 관문·대상은 Server 방 상태에서 찾는다.
	struct C2S_DEBUG_KILL_GATE_BOSSES
	{
		std::uint32_t iRequestSequence = 0;
		WORLD_ID eWorldId = WORLD_ID::END;
		std::string strExpectedBossArchetypeId;
	};
	enum class DEBUG_KILL_GATE_BOSSES_RESULT : std::uint8_t
	{
		ACCEPTED,
		DISABLED,
		WRONG_WORLD,
		INVALID_PLAYER,
		STALE_REQUEST,
		NO_CURRENT_BOSS,
		STALE_BOSS,
		BUSY,
		END
	};
	struct S2C_DEBUG_KILL_GATE_BOSSES_RESULT
	{
		std::uint32_t iRequestSequence = 0;
		WORLD_ID eWorldId = WORLD_ID::END;
		DEBUG_KILL_GATE_BOSSES_RESULT eResult = DEBUG_KILL_GATE_BOSSES_RESULT::DISABLED;
		std::uint8_t iKilledCount = 0;
	};
	bool Write_Message(CPacketWriter&, const C2S_DEBUG_KILL_GATE_BOSSES&);
	bool Read_Message(CPacketReader&, C2S_DEBUG_KILL_GATE_BOSSES&);
	bool Write_Message(CPacketWriter&, const S2C_DEBUG_KILL_GATE_BOSSES_RESULT&);
	bool Read_Message(CPacketReader&, S2C_DEBUG_KILL_GATE_BOSSES_RESULT&);

	// Balance Tool의 부활 요청이다. Server가 session에서 플레이어를 찾으며 Client의 위치나 HP를 입력받지 않는다.
	struct C2S_REVIVE_PLAYER
	{
		std::uint32_t iClientSequence = 0;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_RELEASE_SKILL& message);

	bool Read_Message(
		CPacketReader& reader,
		C2S_RELEASE_SKILL& message);

	// HOLD skill의 충전 단계에서 조준점을 갱신한다.
	// Server가 충전 중일 때만 반영하며 발사 단계는 마지막으로 확정한 방향을 유지한다.
	struct C2S_UPDATE_SKILL_AIM
	{
		std::uint32_t iClientSequence = 0;
		SKILL_ID iSkillId = INVALID_SKILL_ID;
		float fAimX = 0.f;
		float fAimZ = 0.f;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_UPDATE_SKILL_AIM& message);

	bool Read_Message(
		CPacketReader& reader,
		C2S_UPDATE_SKILL_AIM& message);

	// 에스더 슬롯 1..3은 월드 roster의 소환 순서에 대응한다. 발탄은 Sillian, Wei, Bahuntur 순서다.
	// Server가 사용 가능한 슬롯과 소환 정의를 결정하며 Client는 archetype/skill ID를 직접 보내지 않는다.
	inline constexpr std::uint8_t MIN_ESTHER_SLOT_INDEX = 1;
	inline constexpr std::uint8_t MAX_ESTHER_SLOT_INDEX = 3;

	struct C2S_USE_ESTHER_SKILL
	{
		std::uint32_t iClientSequence = 0;
		std::uint8_t iSlotIndex = 0;
		float fAimX = 0.f;
		float fAimZ = 0.f;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_USE_ESTHER_SKILL& message);

	bool Read_Message(
		CPacketReader& reader,
		C2S_USE_ESTHER_SKILL& message);

	// Server가 식별하는 에스더 종류다. 한 월드의 세 슬롯에 무엇이 배치되는지와는 별개다.
	// 각 종류에 대응하는 소환 archetype은 Server가 결정한다.
	enum class ESTHER_ID : std::uint8_t
	{
		NONE,
		SILLIAN,
		WEI,
		BAHUNTUR,
		NINAV,
		INANNA,
		END
	};

	constexpr bool Is_Valid_EstherId(const ESTHER_ID id) noexcept
	{
		return id > ESTHER_ID::NONE && id < ESTHER_ID::END;
	}

	// Debug F1에서 에스더를 직접 지정하는 요청이다. gauge와 roster 슬롯 검사를 건너뛰되 월드의 에스더 계약·시전자 검사는 남는다.
	// Release handler는 이 요청을 실행하지 않는다. 시전자 동작과 소환수는 기존 world snapshot 경로로 표현한다.
	struct C2S_DEBUG_USE_ESTHER
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		ESTHER_ID eEsther = ESTHER_ID::NONE;
		float fAimX = 0.f;
		float fAimZ = 0.f;
	};

	bool Write_Message(CPacketWriter& writer,
		const C2S_DEBUG_USE_ESTHER& message);
	bool Read_Message(CPacketReader& reader,
		C2S_DEBUG_USE_ESTHER& message);

	// 월드맵 이동의 공용 시간 계약이다. 일반 목적지 ID는 square-hole 문서의 1부터 시작하는 행 번호다.
	// WORLD_MAP_SHIP_TRAVEL_DESTINATION_ID는 Set Sail의 예약 ID이며 disabled ship triggerBox로 연결한다.
	// 착지 좌표는 Gameplay.world.json의 movePlayer 데이터에서 읽고 Server가 노래 시작 전에 목적지의 유효성을 검사한다.
	// 노래 길이는 SQUAREHOLE_SONG_DURATION_MS, 검정 전환은 SQUAREHOLE_BLACKOUT_FADE_MS, 검정 유지 구간은 SQUAREHOLE_BLACKOUT_HOLD_MS다.
	// Server가 이동/action 시계를 소유하고 Client는 복제된 행동의 경과 시간으로 화면 전환을 표현한다. 상수의 단위는 ms다.
	inline constexpr std::uint32_t SQUAREHOLE_SONG_DURATION_MS = 3000u;
	inline constexpr std::uint32_t SQUAREHOLE_BLACKOUT_FADE_MS = 600u;
	inline constexpr std::uint32_t SQUAREHOLE_BLACKOUT_HOLD_MS = 400u;
	// 일반 square-hole 행 ID와 구분하는 UINT16_MAX 예약값이다. 기존 U16 필드로 선박 목적지를 명시한다.
	inline constexpr std::uint16_t WORLD_MAP_SHIP_TRAVEL_DESTINATION_ID = 0xffffu;
	// 베른 성/도서관의 movePlayer 이동 시간(ms)이다. Server는 TRIGGER_MOVE 시작 뒤 BERN_TRAVEL_HOLD_MS를 기다려 이동한다.
	// Client는 해당 action을 받은 뒤 BERN_TRAVEL_FADE_OUT_MS 동안 검정으로 전환하고 action 종료 뒤 화면을 복원한다.
	// 네트워크 도착 시각과 실제 화면 확인은 이 상수 선언만으로 보장되지 않는다.
	inline constexpr std::uint32_t BERN_TRAVEL_FADE_OUT_MS = 300u;
	inline constexpr std::uint32_t BERN_TRAVEL_HOLD_MS = 500u;
	struct C2S_USE_SQUAREHOLE
	{
		std::uint32_t iClientSequence = 0;
		std::uint16_t iSquareHoleId = 0;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_USE_SQUAREHOLE& message);

	bool Read_Message(
		CPacketReader& reader,
		C2S_USE_SQUAREHOLE& message);

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_REVIVE_PLAYER& message);

	bool Read_Message(
		CPacketReader& reader,
		C2S_REVIVE_PLAYER& message);

	// Debug/Development 빌드의 테스트 요청이다. 처리 가능 범위는 PACKET_TYPE의 해당 선언과 Server handler를 함께 따른다.
	struct C2S_DEBUG_KILL_SELF
	{
		std::uint32_t iClientSequence = 0;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_DEBUG_KILL_SELF& message);

	bool Read_Message(
		CPacketReader& reader,
		C2S_DEBUG_KILL_SELF& message);

	// Character Select에서 KoukuSaydon 아레나로 이동을 요청하는 Debug 메시지다.
	// 실제 Level 전환 승인은 일반 S2C_ENTER_ACCEPTED 경로를 사용한다.
	struct C2S_DEBUG_ENTER_KOUKUSAYDON_ARENA
	{
		std::uint32_t iRequestSequence = 0;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_DEBUG_ENTER_KOUKUSAYDON_ARENA& message);

	bool Read_Message(
		CPacketReader& reader,
		C2S_DEBUG_ENTER_KOUKUSAYDON_ARENA& message);

	// Debug 단계 이동 요청이다. strPlacementId는 현재 KoukuSaydon world bootstrap의 PLAYER_SPAWN 행을 가리킨다.
	struct C2S_DEBUG_TELEPORT_TO_PLACEMENT
	{
		std::uint32_t iRequestSequence = 0;
		std::string strPlacementId;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_DEBUG_TELEPORT_TO_PLACEMENT& message);

	bool Read_Message(
		CPacketReader& reader,
		C2S_DEBUG_TELEPORT_TO_PLACEMENT& message);

	// 피킹 위치는 이동 의도다. 현재 방이 navigation·높이·충돌을 검사하고 최종 플레이어 Transform을 snapshot으로 확정한다.
	struct C2S_DEBUG_TELEPORT_TO_POSITION
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
	};

	enum class DEBUG_TELEPORT_RESULT : std::uint8_t
	{
		ACCEPTED,
		REJECTED_DISABLED,
		REJECTED_SESSION,
		REJECTED_WRONG_WORLD,
		REJECTED_STALE_SEQUENCE,
		REJECTED_PLAYER_STATE,
		REJECTED_INVALID_POSITION,
		REJECTED_NAVIGATION,
		REJECTED_HEIGHT,
		REJECTED_COLLISION,
		END
	};

	struct S2C_DEBUG_TELEPORT_TO_POSITION_RESULT
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		DEBUG_TELEPORT_RESULT eResult = DEBUG_TELEPORT_RESULT::REJECTED_SESSION;
		// ACCEPTED일 때 Server가 승인한 위치를 담는다. 거부 응답은 이동을 적용하지 않으며 지속 위치의 정본은 일반 snapshot이다.
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
	};

	bool Write_Message(CPacketWriter& writer,
		const C2S_DEBUG_TELEPORT_TO_POSITION& message);
	bool Read_Message(CPacketReader& reader,
		C2S_DEBUG_TELEPORT_TO_POSITION& message);
	bool Write_Message(CPacketWriter& writer,
		const S2C_DEBUG_TELEPORT_TO_POSITION_RESULT& message);
	bool Read_Message(CPacketReader& reader,
		S2C_DEBUG_TELEPORT_TO_POSITION_RESULT& message);

	enum class MARIO_DIRECTION : std::uint8_t
	{
		STOP = 0,
		LEFT = 1,
		RIGHT = 2,
		END
	};

	// 횡스크롤의 논리 방향만 전달한다. 현재 구간의 축은 Server가 해석하며 STOP은 Server 위치에서 이동 입력을 해제한다.
	struct C2S_MARIO_MOVE
	{
		std::uint32_t iClientSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		MARIO_DIRECTION eDirection = MARIO_DIRECTION::STOP;
	};

	bool Write_Message(CPacketWriter& writer, const C2S_MARIO_MOVE& message);
	bool Read_Message(CPacketReader& reader, C2S_MARIO_MOVE& message);

	// Debug 점프 요청의 LEFT/RIGHT는 방향 선택이며 월드 조준점·착지점을 직접 지정하지 않는다.
	// Server가 허용 범위의 점프를 정하고 위치는 snapshot으로 복제한다.
	struct C2S_DEBUG_MARIO_JUMP
	{
		std::uint32_t iClientSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		MARIO_DIRECTION eDirection = MARIO_DIRECTION::STOP;
	};

	enum class DEBUG_MARIO_JUMP_RESULT : std::uint8_t
	{
		ACCEPTED,
		REJECTED_DISABLED,
		REJECTED_WRONG_WORLD,
		REJECTED_PLAYER_STATE,
		REJECTED_STALE_SEQUENCE,
		REJECTED_OUTSIDE_MARIO,
		REJECTED_INVALID_TARGET,
		REJECTED_NO_LANDING,
		END
	};

	struct S2C_DEBUG_MARIO_JUMP_RESULT
	{
		std::uint32_t iClientSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		DEBUG_MARIO_JUMP_RESULT eResult = DEBUG_MARIO_JUMP_RESULT::REJECTED_DISABLED;
	};

	bool Write_Message(CPacketWriter& writer, const C2S_DEBUG_MARIO_JUMP& message);
	bool Read_Message(CPacketReader& reader, C2S_DEBUG_MARIO_JUMP& message);
	bool Write_Message(CPacketWriter& writer, const S2C_DEBUG_MARIO_JUMP_RESULT& message);
	bool Read_Message(CPacketReader& reader, S2C_DEBUG_MARIO_JUMP_RESULT& message);

	// 복귀 요청에는 목적지가 없다. Server가 현재 기믹의 출구를 결정한다.
	struct C2S_MARIO_RETURN
	{
		std::uint32_t iClientSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
	};

	enum class MARIO_RETURN_RESULT : std::uint8_t
	{
		ACCEPTED,
		REJECTED_WRONG_WORLD,
		REJECTED_PLAYER_STATE,
		REJECTED_STALE_SEQUENCE,
		REJECTED_OUTSIDE_MARIO,
		REJECTED_DESTINATION,
		END
	};

	struct S2C_MARIO_RETURN_RESULT
	{
		std::uint32_t iClientSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		MARIO_RETURN_RESULT eResult = MARIO_RETURN_RESULT::REJECTED_PLAYER_STATE;
	};

	bool Write_Message(CPacketWriter& writer, const C2S_MARIO_RETURN& message);
	bool Read_Message(CPacketReader& reader, C2S_MARIO_RETURN& message);
	bool Write_Message(CPacketWriter& writer, const S2C_MARIO_RETURN_RESULT& message);
	bool Read_Message(CPacketReader& reader, S2C_MARIO_RETURN_RESULT& message);

	// 아래 PLAYER_MADNESS_FORM은 플레이어의 표현 몸체를 구분한다. NORMAL은 직업 몸체, CLOWN은 쿠크의 광대 몸체다.
	// HUD가 아이콘과 남은 시간을 그릴 수 있도록 buff/debuff ID와 종료 tick을 복제한다.
	inline constexpr std::size_t MAX_ACTIVE_BUFFS = 4u;
	struct ACTIVE_BUFF final
	{
		std::uint32_t iBuffId = 0u;
		std::uint32_t iEndTick = 0u;
	};

	enum class PLAYER_MADNESS_FORM : std::uint8_t
	{
		NORMAL,
		CLOWN,
		END
	};

	constexpr bool Is_Valid_PlayerMadnessForm(const PLAYER_MADNESS_FORM form) noexcept
	{
		return form < PLAYER_MADNESS_FORM::END;
	}

	// 아래 C2S_DEBUG_SET_MADNESS_FORM은 Debug F1의 광대/플레이어 변신 요청이다. 실제 form은 Server가 정해 PLAYER_SNAPSHOT으로 복제한다.
	// Debug F1의 빙고 칠하기 요청이다. Server가 white 칸과 완성된 line을 판정한다.
	// 별도 결과 메시지 없이 다음 world snapshot에서 board 상태를 받는다.
	struct C2S_DEBUG_BINGO_FILL
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		// bit 0..24가 칸에 대응한다. 이미 칠한 칸은 유지한다.
		std::uint32_t iCellMask = 0u;
		// 칠하기 전에 board를 초기화하는 옵션이다.
		bool bReset = false;
	};

	bool Write_Message(CPacketWriter& writer,
		const C2S_DEBUG_BINGO_FILL& message);
	bool Read_Message(CPacketReader& reader,
		C2S_DEBUG_BINGO_FILL& message);

	// Debug 빙고 폭탄을 요청한 session의 자기 플레이어에게 부여한다. 요청에는 대상 entity ID를 넣지 않는다.
	struct C2S_DEBUG_BINGO_BOMB
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
	};

	bool Write_Message(CPacketWriter& writer,
		const C2S_DEBUG_BINGO_BOMB& message);
	bool Read_Message(CPacketReader& reader,
		C2S_DEBUG_BINGO_BOMB& message);

	// Debug 빙고 망치 요청이다. anchor는 Server가 고르므로 요청에는 sequence와 world만 담는다.
	struct C2S_DEBUG_BINGO_HAMMER
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
	};

	bool Write_Message(CPacketWriter& writer,
		const C2S_DEBUG_BINGO_HAMMER& message);
	bool Read_Message(CPacketReader& reader,
		C2S_DEBUG_BINGO_HAMMER& message);

	// F1의 두 일반 몬스터 버튼 중 누른 항목이다. Server가 (world, button)을 정해진 spawn group에 대응시킨다.
	// 요청은 group ID를 보내지 않는다. 쿠크 1/2는 Book1_Monsters/Book2_Monsters, 발탄 1/2는 Stage_1/Stage_2다.
	enum class WAVE_MONSTER_BUTTON : std::uint8_t
	{
		NORMAL_MONSTER_1,
		NORMAL_MONSTER_2,
		END
	};

	// Debug F1의 일반 몬스터 1/2 재소환 요청이다. 대상 wave의 현재 몬스터를 정리하고 저작 anchor에서 다시 소환한다.
	// 별도 결과 메시지 대신 기존 world entity 생성·snapshot 경로로 표현한다.
	struct C2S_DEBUG_RESUMMON_WAVE_MONSTERS
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		WAVE_MONSTER_BUTTON eButton = WAVE_MONSTER_BUTTON::END;
	};

	bool Write_Message(CPacketWriter& writer,
		const C2S_DEBUG_RESUMMON_WAVE_MONSTERS& message);
	bool Read_Message(CPacketReader& reader,
		C2S_DEBUG_RESUMMON_WAVE_MONSTERS& message);

	struct C2S_DEBUG_SET_MADNESS_FORM
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		PLAYER_MADNESS_FORM eForm = PLAYER_MADNESS_FORM::NORMAL;
	};

	enum class DEBUG_MADNESS_FORM_RESULT : std::uint8_t
	{
		ACCEPTED,
		REJECTED_DISABLED,
		REJECTED_SESSION,
		REJECTED_WRONG_WORLD,
		REJECTED_STALE_SEQUENCE,
		REJECTED_PLAYER_STATE,
		REJECTED_SAME_FORM,
		END
	};

	struct S2C_DEBUG_SET_MADNESS_FORM_RESULT
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		DEBUG_MADNESS_FORM_RESULT eResult = DEBUG_MADNESS_FORM_RESULT::REJECTED_SESSION;
		// 요청이 승인됐는지와 별도로, 응답을 만들 때 플레이어가 가진 form이다.
		PLAYER_MADNESS_FORM eActiveForm = PLAYER_MADNESS_FORM::NORMAL;
	};

	bool Write_Message(CPacketWriter& writer,
		const C2S_DEBUG_SET_MADNESS_FORM& message);
	bool Read_Message(CPacketReader& reader,
		C2S_DEBUG_SET_MADNESS_FORM& message);
	bool Write_Message(CPacketWriter& writer,
		const S2C_DEBUG_SET_MADNESS_FORM_RESULT& message);
	bool Read_Message(CPacketReader& reader,
		S2C_DEBUG_SET_MADNESS_FORM_RESULT& message);

	// 탈것 탑승/해제 요청이다. vehicle ID로 탑승을, INVALID_VEHICLE_ID로 해제를 요청한다.
	// Server가 가능 여부를 판정하고 지속 탑승 상태는 snapshot에 담는다.
	struct C2S_SET_VEHICLE_RIDING
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		VEHICLE_ID iVehicleId = INVALID_VEHICLE_ID;
	};

	enum class VEHICLE_RIDING_RESULT : std::uint8_t
	{
		ACCEPTED,
		REJECTED_SESSION,
		REJECTED_WRONG_WORLD,
		REJECTED_STALE_SEQUENCE,
		REJECTED_WORLD_NOT_ALLOWED,
		REJECTED_UNKNOWN_VEHICLE,
		REJECTED_PLAYER_STATE,
		REJECTED_SAME_STATE,
		END
	};

	struct S2C_SET_VEHICLE_RIDING_RESULT
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		VEHICLE_RIDING_RESULT eResult = VEHICLE_RIDING_RESULT::REJECTED_SESSION;
		// 요청 승인 여부와 별도로, 응답을 만들 때 플레이어가 타고 있는 vehicle ID다.
		VEHICLE_ID iActiveVehicleId = INVALID_VEHICLE_ID;
	};

	bool Write_Message(CPacketWriter& writer,
		const C2S_SET_VEHICLE_RIDING& message);
	bool Read_Message(CPacketReader& reader,
		C2S_SET_VEHICLE_RIDING& message);
	bool Write_Message(CPacketWriter& writer,
		const S2C_SET_VEHICLE_RIDING_RESULT& message);
	bool Read_Message(CPacketReader& reader,
		S2C_SET_VEHICLE_RIDING_RESULT& message);

	// 칭호 창의 변경 요청이다. 응답은 처리 결과와 현재 칭호를, snapshot은 지속 착용 상태를 전달한다.
	struct C2S_SET_HONOR_TITLE
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		HONOR_TITLE_ID iHonorTitleId = INVALID_HONOR_TITLE_ID;
	};

	enum class HONOR_TITLE_RESULT : std::uint8_t
	{
		ACCEPTED,
		REJECTED_SESSION,
		REJECTED_WRONG_WORLD,
		REJECTED_STALE_SEQUENCE,
		REJECTED_UNKNOWN_TITLE,
		REJECTED_SAME_STATE,
		END
	};

	struct S2C_SET_HONOR_TITLE_RESULT
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		HONOR_TITLE_RESULT eResult = HONOR_TITLE_RESULT::REJECTED_SESSION;
		// 요청 승인 여부와 별도로, 응답을 만들 때 착용한 칭호 ID다.
		HONOR_TITLE_ID iActiveHonorTitleId = INVALID_HONOR_TITLE_ID;
	};

	bool Write_Message(CPacketWriter& writer,
		const C2S_SET_HONOR_TITLE& message);
	bool Read_Message(CPacketReader& reader,
		C2S_SET_HONOR_TITLE& message);
	bool Write_Message(CPacketWriter& writer,
		const S2C_SET_HONOR_TITLE_RESULT& message);
	bool Read_Message(CPacketReader& reader,
		S2C_SET_HONOR_TITLE_RESULT& message);

	// 플레이어 머리 위에 표시할 쿠크 기믹 카드의 무늬다. 부여·유지·해제 시점은 Server 기믹이 결정한다.
	enum class MECHANIC_CARD_SYMBOL : std::uint8_t
	{
		NONE,
		HEART,
		SPADE,
		CLUB,
		DIAMOND,
		END
	};

	constexpr bool Is_Valid_MechanicCardSymbol(const MECHANIC_CARD_SYMBOL symbol) noexcept
	{
		return symbol < MECHANIC_CARD_SYMBOL::END;
	}

	// 쿠크 카드 미로 역할이다. 망원경을 먼저 확보한 플레이어와 나머지 사냥꾼을 Server가 구분한다.
	// 사냥 대상 무늬도 Server가 배정하고 Client는 역할·무늬를 표현한다.
	enum class CARD_MAZE_ROLE : std::uint8_t
	{
		NONE,
		TELESCOPE,
		HUNTER,
		END
	};

	// Server 소유 미로 표현 상태다. flags는 관찰=1, 탈출=2, 출구=4, 미로 이탈=8, 입장 연출 숨김=16이다.
	// 이펙트 asset과 카메라 pose는 Client가 소유한다. exitX/Y/Z는 월드 좌표, marchCycleMs는 ms다.
	inline constexpr std::uint8_t CARD_MAZE_ENTRY_HIDDEN = 16u;
	inline constexpr std::uint8_t CARD_MAZE_VALID_FLAGS = 31u;
	struct CARD_MAZE_PRESENTATION final
	{
		std::uint8_t flags = 0u;
		float exitX = 0.f, exitY = 0.f, exitZ = 0.f;
		std::uint32_t marchStartTick = 0u, marchCycleMs = 0u;
		std::uint32_t transferStartTick = 0u;
	};

	constexpr bool Is_Valid_CardMazeRole(const CARD_MAZE_ROLE role) noexcept
	{
		return role < CARD_MAZE_ROLE::END;
	}

	enum class MECHANIC_CARD_COLOR : std::uint8_t
	{
		NONE,
		RED,
		BLACK,
		END
	};

	constexpr bool Is_Valid_MechanicCardColor(const MECHANIC_CARD_COLOR color) noexcept
	{
		return color < MECHANIC_CARD_COLOR::END;
	}

	// Server가 플레이어별로 선택한 상호작용 HUD다. 광대 변신·Mario·춤·카드 미로를 구분한다.
	// NONE은 직업 HUD를 사용하며 각 quick slot의 icon index 배치도 snapshot으로 전달한다.
	enum class KOUKU_HUD_MODE : std::uint8_t
	{
		NONE,
		POLYMORPH,
		MARIO,
		DANCE,
		MAZE,
		END
	};

	constexpr bool Is_Valid_KoukuHudMode(const KOUKU_HUD_MODE mode) noexcept
	{
		return mode < KOUKU_HUD_MODE::END;
	}

	// HUD quick slot 순서 Q W E R A S D F에 대응한다.
	enum class INTERACTION_SLOT : std::uint8_t
	{
		Q,
		W,
		E,
		R,
		A,
		S,
		D,
		F,
		END
	};

	constexpr std::size_t KOUKU_HUD_SLOT_COUNT = 8u;
	constexpr std::int8_t KOUKU_HUD_SLOT_EMPTY = -1;
	// 상호작용 입력은 Server의 INTERACTION action 동안 해당 mode/slot clip을 표현한다.
	// 이 값은 별도 길이를 선택하지 않은 slot에 사용하는 기본 길이(ms)다.
	constexpr std::uint32_t KOUKU_INTERACTION_ACTION_MS = 3000u;
	constexpr std::uint32_t KOUKU_INTERACTION_COOLDOWN_MS = 3000u;
	// 미로 망치의 action 길이와 cooldown은 별개다. clip은 action 경과 시간으로 표본화한다.
	// 여기서는 cooldown을 400ms로 두어 이후 입력의 사용 가능 시각을 결정한다.
	constexpr std::uint32_t KOUKU_MAZE_HAMMER_COOLDOWN_MS = 400u;
	// Server와 Client가 mode/slot index에 같은 상호작용 길이(ms)를 사용하도록 공유하는 함수다.
	// POLYMORPH/MARIO/MAZE의 지정 index는 아래 고정 길이를 쓰고 그 외는 3000ms를 반환한다.
	// Client는 action 경과 시간으로 clip을 표본화하고 Server는 이 길이를 30Hz tick 수로 바꾸어 action을 끝낸다.
	// 카드 미로 탈출 이동은 별도 transferStartTick 경로에서 기본 길이를 선택한다. 이 함수가 JSON/clip을 직접 읽지는 않는다.
	constexpr std::uint32_t Kouku_InteractionActionMs(
		const KOUKU_HUD_MODE mode, const std::uint32_t slotSkillIndex) noexcept
	{
		switch (mode)
		{
		case KOUKU_HUD_MODE::POLYMORPH:
			return 2u == slotSkillIndex ? 1000u :
				slotSkillIndex < 2u ? 1500u : KOUKU_INTERACTION_ACTION_MS;
		case KOUKU_HUD_MODE::MARIO:
			return 0u == slotSkillIndex ? 2500u :
				1u == slotSkillIndex ? 1500u : KOUKU_INTERACTION_ACTION_MS;
		case KOUKU_HUD_MODE::MAZE:
			return 0u == slotSkillIndex ? 2500u :
				1u == slotSkillIndex ? 1000u : KOUKU_INTERACTION_ACTION_MS;
		default:
			// 그 외 mode/index는 기본 길이를 사용한다.
			return KOUKU_INTERACTION_ACTION_MS;
		}
	}

	// 쿠크 빙고는 5×5칸이며 간격은 3.04m다. 아래 상수는 월드 XZ에 축 정렬된 board를 정의한다.
	// cell=row*5+column이며 row는 +Z, column은 +X다. cell 0은 -X/-Z, cell 24는 +X/+Z 모서리다.
	// Server 판정과 Client 표현이 같은 계산을 쓰도록 Shared에 둔다.
	constexpr std::int32_t KOUKU_BINGO_SIDE = 5;
	constexpr std::int32_t KOUKU_BINGO_CELL_COUNT = KOUKU_BINGO_SIDE * KOUKU_BINGO_SIDE;
	constexpr float KOUKU_BINGO_CELL_SIZE_M = 3.04f;
	// cell 0의 중심 월드 X/Z(m)다. board 바깥 경계가 아니라 칸 중심이다.
	constexpr float KOUKU_BINGO_ORIGIN_X = -6.08f;
	constexpr float KOUKU_BINGO_ORIGIN_Z = 1140.8f;
	constexpr std::int32_t KOUKU_BINGO_OFF_BOARD = -1;
	// bit 0..24가 각각 한 칸이다. bit 25 이상은 유효한 칸이 아니다.
	constexpr std::uint32_t KOUKU_BINGO_ALL_CELLS_MASK = 0x1FFFFFFu;
	// 행 5개, 열 5개, 대각선 2개를 합해 12개의 완성 line을 판정한다.
	constexpr std::int32_t KOUKU_BINGO_LINE_COUNT = 12;

	constexpr bool Is_KoukuBingoCell(const std::int32_t cell) noexcept
	{
		return cell >= 0 && cell < KOUKU_BINGO_CELL_COUNT;
	}

	constexpr float Kouku_BingoCellCenterX(const std::int32_t cell) noexcept
	{
		return KOUKU_BINGO_ORIGIN_X +
			KOUKU_BINGO_CELL_SIZE_M * static_cast<float>(cell % KOUKU_BINGO_SIDE);
	}

	constexpr float Kouku_BingoCellCenterZ(const std::int32_t cell) noexcept
	{
		return KOUKU_BINGO_ORIGIN_Z +
			KOUKU_BINGO_CELL_SIZE_M * static_cast<float>(cell / KOUKU_BINGO_SIDE);
	}

	// 월드 X/Z가 속한 칸을 반환하며 board 바깥과 NaN은 KOUKU_BINGO_OFF_BOARD다.
	// 원점이 cell 0 중심이므로 반 칸을 더한 뒤 0 이상 범위에서 정수로 변환한다. 상한 경계는 제외한다.
	// 음수/NaN을 먼저 정수로 변환하지 않도록 비교를 앞에 둔다.
	inline std::int32_t Kouku_BingoCellAt(const float x, const float z) noexcept
	{
		const float column =
			(x - KOUKU_BINGO_ORIGIN_X) / KOUKU_BINGO_CELL_SIZE_M + 0.5f;
		const float row =
			(z - KOUKU_BINGO_ORIGIN_Z) / KOUKU_BINGO_CELL_SIZE_M + 0.5f;
		const float side = static_cast<float>(KOUKU_BINGO_SIDE);
		if (!(column >= 0.f) || !(row >= 0.f) || !(column < side) || !(row < side))
			return KOUKU_BINGO_OFF_BOARD;
		return static_cast<std::int32_t>(row) * KOUKU_BINGO_SIDE +
			static_cast<std::int32_t>(column);
	}

	// 아래 Kouku_BingoNeighbour는 행/열 단위로 이동한 이웃 칸을 구한다.
	// 단순 index 덧셈과 달리 열 경계를 넘어 다음 행으로 이어지는 것을 막는다.
	// line의 칸을 bit mask로 돌려준다. 0..4는 행, 5..9는 열, 10..11은 대각선이다.
	// 범위 밖 line은 0이다. 이 함수 자체가 현재 board의 완성 여부를 판정하지는 않는다.
	constexpr std::uint32_t Kouku_BingoLineMask(const std::int32_t line) noexcept
	{
		if (line < 0 || line >= KOUKU_BINGO_LINE_COUNT)
			return 0u;
		if (line < KOUKU_BINGO_SIDE)
			return 0x1Fu << (line * KOUKU_BINGO_SIDE);
		if (line < KOUKU_BINGO_SIDE * 2)
			return 0x108421u << (line - KOUKU_BINGO_SIDE);
		// 두 대각선의 cell index는 0,6,12,18,24와 4,8,12,16,20이다.
		return (KOUKU_BINGO_SIDE * 2 == line) ? 0x1041041u : 0x111110u;
	}

	constexpr std::int32_t Kouku_BingoNeighbour(const std::int32_t cell,
		const std::int32_t rowStep, const std::int32_t columnStep) noexcept
	{
		if (!Is_KoukuBingoCell(cell))
			return KOUKU_BINGO_OFF_BOARD;
		const std::int32_t row = cell / KOUKU_BINGO_SIDE + rowStep;
		const std::int32_t column = cell % KOUKU_BINGO_SIDE + columnStep;
		if (row < 0 || row >= KOUKU_BINGO_SIDE ||
			column < 0 || column >= KOUKU_BINGO_SIDE)
		{
			return KOUKU_BINGO_OFF_BOARD;
		}
		return row * KOUKU_BINGO_SIDE + column;
	}

	// 폭탄 중심과 상하좌우 이웃의 mask다. 가장자리에서는 board 밖 이웃을 제외한다.
	// 중심이 유효한 칸이 아니면 0을 반환한다.
	constexpr std::uint32_t Kouku_BingoCrossMask(const std::int32_t cell) noexcept
	{
		if (!Is_KoukuBingoCell(cell))
			return 0u;
		std::uint32_t mask = 1u << cell;
		const std::int32_t steps[4][2] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
		for (const auto& step : steps)
		{
			const std::int32_t neighbour =
				Kouku_BingoNeighbour(cell, step[0], step[1]);
			if (Is_KoukuBingoCell(neighbour))
				mask |= 1u << neighbour;
		}
		return mask;
	}

	// 빙고 폭탄 시계는 Server가 소유한다. MARKED는 플레이어를 따라가고 만료 시 그 위치에서 PLANTED로 전환한다.
	// Client는 snapshot의 단계와 위치를 표현한다. 아래 시간 상수는 ms다.
	constexpr std::uint32_t KOUKU_BINGO_BOMB_MARK_MS = 6000u;
	constexpr std::uint32_t KOUKU_BINGO_BOMB_INTERVAL_MS = 20000u;
	constexpr std::uint32_t KOUKU_BINGO_HAMMER_INTERVAL_MS = 10000u;
	constexpr std::uint32_t KOUKU_BINGO_HAMMER_WARNING_MS = 3000u;
	// 설치된 폭탄이 십자 mask를 적용할 때까지의 대기 시간(ms)이다.
	constexpr std::uint32_t KOUKU_BINGO_BOMB_FUSE_MS = 4000u;
	constexpr std::uint32_t KOUKU_BINGO_BOMB_DROP_DELAY_MS = 2000u;
	constexpr std::uint32_t KOUKU_BINGO_BOMB_INITIAL_DELAY_MS = 10000u;
	// 동시에 복제할 폭탄 slot의 상한은 4개다.
	constexpr std::int32_t KOUKU_BINGO_MAX_BOMBS = 4;

	enum class BINGO_BOMB_PHASE : std::uint8_t
	{
		NONE = 0,
		// 플레이어를 따라가는 표식 단계다.
		MARKED,
		// 표식 종료 시 플레이어가 있던 위치에 설치된 단계다.
		PLANTED,
		END
	};

	struct BINGO_BOMB_SNAPSHOT
	{
		// MARKED에서 사용한다. Client는 carrier의 복제 Transform을 참조한다.
		NET_ENTITY_ID iCarrierNetEntityId = INVALID_NET_ENTITY_ID;
		// PLANTED에서 사용하는 월드 X/Z(m)다.
		float fPositionX = 0.f;
		float fPositionZ = 0.f;
		BINGO_BOMB_PHASE ePhase = BINGO_BOMB_PHASE::NONE;
	};

	// 빙고 망치는 행/열에 해당하는 앞의 10개 line만 사용하고 대각선은 사용하지 않는다.
	// 각 line 양쪽에서 시작할 수 있어 anchor는 20개다. 시작/끝 좌표는 board 상수로 계산한다.
	constexpr std::int32_t KOUKU_BINGO_HAMMER_LINE_COUNT = KOUKU_BINGO_SIDE * 2;
	constexpr std::int32_t KOUKU_BINGO_HAMMER_ANCHOR_COUNT =
		KOUKU_BINGO_HAMMER_LINE_COUNT * 2;
	// 공중 대기 → 하강 → line 이동의 각 단계 시간(ms)이다.
	constexpr std::uint32_t KOUKU_BINGO_HAMMER_RAISE_MS = 2000u;
	constexpr std::uint32_t KOUKU_BINGO_HAMMER_DESCEND_MS = 1400u;
	constexpr std::uint32_t KOUKU_BINGO_HAMMER_SWEEP_MS = 1600u;
	// 망치의 공중 대기 높이(m)다.
	constexpr float KOUKU_BINGO_HAMMER_SKY_Y = 9.f;
	// 훑는 동안 진행 방향으로 기울이는 각도(degree)다.
	// 머리가 원점에 있고 사슬이 위로 향하는 모델 배치를 기준으로 적용한다.
	constexpr float KOUKU_BINGO_HAMMER_LEAN_DEGREES = 40.f;
	// 전체 sweep 진행 중 기울기가 최댓값에 도달하는 비율이다.
	constexpr float KOUKU_BINGO_HAMMER_LEAN_RATIO = 0.3f;

	enum class BINGO_HAMMER_PHASE : std::uint8_t
	{
		NONE = 0,
		// 진입 지점 위 공중에서 대기한다.
		RAISED,
		// board로 내려온다.
		DESCENDING,
		// 진입점에서 이탈점까지 line을 훑는다.
		SWEEPING,
		END
	};

	constexpr bool Is_KoukuBingoHammerAnchor(const std::int32_t anchor) noexcept
	{
		return anchor >= 0 && anchor < KOUKU_BINGO_HAMMER_ANCHOR_COUNT;
	}

	// anchor의 시작/끝 월드 XZ(m)다. 끝 칸 중심에서 한 칸 간격 밖까지 경로를 잡는다.
	// 짝수 anchor는 축의 양의 방향, 홀수는 반대 방향이다.
	struct BINGO_HAMMER_PATH
	{
		float fStartX = 0.f;
		float fStartZ = 0.f;
		float fEndX = 0.f;
		float fEndZ = 0.f;
	};

	constexpr BINGO_HAMMER_PATH Kouku_BingoHammerPath(
		const std::int32_t anchor) noexcept
	{
		BINGO_HAMMER_PATH path{};
		if (!Is_KoukuBingoHammerAnchor(anchor))
			return path;
		const std::int32_t line = anchor / 2;
		const bool reversed = 0 != (anchor % 2);
		const float outside = KOUKU_BINGO_CELL_SIZE_M;
		const float low = -outside;
		const float high =
			KOUKU_BINGO_CELL_SIZE_M * static_cast<float>(KOUKU_BINGO_SIDE);
		if (line < KOUKU_BINGO_SIDE)
		{
			// 행은 Z를 고정하고 X 방향으로 이동한다.
			const float z = Kouku_BingoCellCenterZ(line * KOUKU_BINGO_SIDE);
			path.fStartX = KOUKU_BINGO_ORIGIN_X + (reversed ? high : low);
			path.fEndX = KOUKU_BINGO_ORIGIN_X + (reversed ? low : high);
			path.fStartZ = z;
			path.fEndZ = z;
			return path;
		}
		// 열은 X를 고정하고 Z 방향으로 이동한다.
		const float x = Kouku_BingoCellCenterX(line - KOUKU_BINGO_SIDE);
		path.fStartX = x;
		path.fEndX = x;
		path.fStartZ = KOUKU_BINGO_ORIGIN_Z + (reversed ? high : low);
		path.fEndZ = KOUKU_BINGO_ORIGIN_Z + (reversed ? low : high);
		return path;
	}

	// anchor가 지나가는 행/열 전체 mask다. 유효하지 않은 anchor는 0이다.
	constexpr std::uint32_t Kouku_BingoHammerLineMask(
		const std::int32_t anchor) noexcept
	{
		return Is_KoukuBingoHammerAnchor(anchor) ?
			Kouku_BingoLineMask(anchor / 2) : 0u;
	}

	struct BINGO_HAMMER_SNAPSHOT
	{
		// NONE 단계에서는 -1이다.
		std::int32_t iAnchor = -1;
		// Client가 시작/종료 Server tick 사이의 진행률로 위치를 계산한다.
		// 매 tick의 망치 좌표를 이 구조체에 따로 싣지 않는다.
		std::uint32_t iPhaseStartTick = 0u;
		std::uint32_t iPhaseEndTick = 0u;
		BINGO_HAMMER_PHASE ePhase = BINGO_HAMMER_PHASE::NONE;
	};

	constexpr bool Is_Valid_BingoHammerSnapshot(
		const BINGO_HAMMER_SNAPSHOT& hammer) noexcept
	{
		if (BINGO_HAMMER_PHASE::NONE == hammer.ePhase)
			return -1 == hammer.iAnchor;
		if (hammer.ePhase >= BINGO_HAMMER_PHASE::END ||
			!Is_KoukuBingoHammerAnchor(hammer.iAnchor))
		{
			return false;
		}
		// 활성 단계의 종료 tick은 시작 tick보다 커야 한다. 동일/역순 구간을 거부한다.
		// 여기서는 wrap을 가로지르는 구간을 따로 해석하지 않는다.
		return hammer.iPhaseEndTick > hammer.iPhaseStartTick;
	}

	// 상호작용 cooldown의 예약 ID다. mode별 animation index 및 직업 skill ID와 구분한다.
	// 이 함수는 bit를 조합할 뿐 전달된 mode/index의 범위를 검증하지 않는다.
	constexpr SKILL_ID Kouku_InteractionCooldownSkillId(
		const KOUKU_HUD_MODE mode, const std::uint32_t skillIndex) noexcept
	{
		return 0xFFFF0000u | (static_cast<std::uint32_t>(mode) << 8u) | (skillIndex + 1u);
	}

	constexpr bool Is_Valid_InteractionSlot(const INTERACTION_SLOT slot) noexcept
	{
		return slot < INTERACTION_SLOT::END;
	}

	// 상호작용 HUD의 quick slot 입력이다. Server가 플레이어의 복제 slot 배치에서 mode skill index를 찾는다.
	// Client는 여기에 pose/skill ID를 직접 넣지 않는다.
	struct C2S_INTERACTION_SLOT
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		INTERACTION_SLOT eSlot = INTERACTION_SLOT::Q;
	};

	bool Write_Message(CPacketWriter& writer, const C2S_INTERACTION_SLOT& message);
	bool Read_Message(CPacketReader& reader, C2S_INTERACTION_SLOT& message);

	// F1의 상호작용 HUD override 요청이다. Server가 mode/world를 확인하고 typed 결과를 돌려준다.
	// 현재 Release도 KOUKUSAYDON_ARENA의 NONE/MARIO/MAZE 요청을 처리한다.
	struct C2S_DEBUG_SET_KOUKU_HUD_MODE
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		KOUKU_HUD_MODE eMode = KOUKU_HUD_MODE::NONE;
	};

	enum class DEBUG_KOUKU_HUD_MODE_RESULT : std::uint8_t
	{
		ACCEPTED,
		REJECTED_DISABLED,
		REJECTED_SESSION,
		REJECTED_WRONG_WORLD,
		REJECTED_STALE_SEQUENCE,
		REJECTED_UNSUPPORTED_MODE,
		END
	};

	struct S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		DEBUG_KOUKU_HUD_MODE_RESULT eResult = DEBUG_KOUKU_HUD_MODE_RESULT::REJECTED_SESSION;
		// 요청 승인 여부와 별도로, 응답을 만들 때 적용된 override다.
		KOUKU_HUD_MODE eActiveOverride = KOUKU_HUD_MODE::NONE;
	};

	bool Write_Message(CPacketWriter& writer,
		const C2S_DEBUG_SET_KOUKU_HUD_MODE& message);
	bool Read_Message(CPacketReader& reader,
		C2S_DEBUG_SET_KOUKU_HUD_MODE& message);
	bool Write_Message(CPacketWriter& writer,
		const S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT& message);
	bool Read_Message(CPacketReader& reader,
		S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT& message);

	// 보스 패턴의 scene-profile cue다. stable profile ID는 Client의 rendering profile catalog에서 해석한다.
	// iBlendMs/iDurationMs는 ms이며 duration=0은 이후 cue 또는 Level profile 복원까지 유지하는 계약이다.
	struct S2C_SCENE_PROFILE_APPLY
	{
		std::string strProfileId;
		std::uint32_t iBlendMs = 0u;
		std::uint32_t iDurationMs = 0u;
	};

	bool Write_Message(CPacketWriter& writer, const S2C_SCENE_PROFILE_APPLY& message);
	bool Read_Message(CPacketReader& reader, S2C_SCENE_PROFILE_APPLY& message);

	struct C2S_CHANGE_CHARACTER_CLASS
	{
		std::uint32_t iClientSequence = 0;
		CHARACTER_CLASS_ID eCharacterClass = CHARACTER_CLASS_ID::END;
	};

	enum class CHARACTER_CLASS_CHANGE_RESULT : std::uint8_t
	{
		ACCEPTED,
		REJECTED_WRONG_WORLD,
		REJECTED_STALE_SEQUENCE,
		REJECTED_UNSUPPORTED_CLASS,
		REJECTED_SAME_CLASS,
		REJECTED_STATE,
		END
	};

	struct S2C_CHARACTER_CLASS_CHANGE_RESULT
	{
		std::uint32_t iClientSequence = 0;
		CHARACTER_CLASS_CHANGE_RESULT eResult =
			CHARACTER_CLASS_CHANGE_RESULT::REJECTED_STATE;
		CHARACTER_CLASS_ID eRequestedClass = CHARACTER_CLASS_ID::END;
		CHARACTER_CLASS_ID eActiveClass = CHARACTER_CLASS_ID::END;
	};

	bool Write_Message(CPacketWriter& writer,
		const C2S_CHANGE_CHARACTER_CLASS& message);
	bool Read_Message(CPacketReader& reader,
		C2S_CHANGE_CHARACTER_CLASS& message);
	bool Write_Message(CPacketWriter& writer,
		const S2C_CHARACTER_CLASS_CHANGE_RESULT& message);
	bool Read_Message(CPacketReader& reader,
		S2C_CHARACTER_CLASS_CHANGE_RESULT& message);

	enum class PLAYER_ACTION_STATE : std::uint8_t
	{
		NONE,
		SKILL,
		TRIGGER_MOVE,
		KNOCKDOWN,
		DEAD,
		// 지면 붕괴에 따른 낙하 상태다. Server가 위치와 사망 시각을 결정하고 snapshot은 action/위치를 복제한다.
		// 기존 wire enum 값을 유지하려면 항목 순서를 바꾸지 않는다.
		FALLING,
		// 에스더 호출 중인 시전자 상태다. 소환은 roster 정의를 사용하므로 일반 balance skill ID를 싣지 않는다.
		// Room이 호출 동작의 종료 시간을 결정한다.
		ESTHER_CAST,
		// 포획/부착 상태다. Server가 attachment 계약에 따라 gameplay 위치·방향과 해제를 처리한다.
		// Client의 손 socket 표현은 BOSS_LEFT_HAND에 해당하며 WORLD_HOOK_TIP은 월드 오브젝트 표현을 사용한다.
		GRABBED,
		// 쿠크 상호작용 action이다. iSkillId는 balance skill이 아니라 slot의 mode skill index(0..KOUKU_HUD_SLOT_COUNT-1)다.
		// 종료 시간은 Kouku_InteractionActionMs 또는 탈출 이동의 별도 기본 길이를 사용한다.
		INTERACTION,
		FEAR,
		// 탈것 skill이다. iSkillId는 Server가 해당 vehicle에 허용한 EFTable_Vehicle skill, iActionStartTick은 시작 시각이다.
		// Room이 지속 시간과 저작 root motion을 처리한다.
		VEHICLE_SKILL,
		// 월드맵 이동 노래 상태다. Server가 SQUAREHOLE_SONG_DURATION_MS와 검정 유지 구간에 맞춰 이동/action 종료를 처리한다.
		SQUAREHOLE_SONG,
		// Server의 TrackMove가 벽 등반 위치를 갱신한다. 일반 TRIGGER_MOVE의 지형 점프 표현과 구분한다.
		// 현재 Client는 설치된 등반 loop/end clip을 action 시계로 표본화하고 clip이 없으면 idle로 대체한다.
		WALL_CLIMB,
		END
	};

	// 복제하는 부착 슬롯 ID다. 모델 bone 이름을 네트워크 계약에 넣지 않는다.
	// Client가 자신의 표현 socket/월드 오브젝트 경로로 해석한다.
	enum class PLAYER_ATTACHMENT_SLOT : std::uint8_t
	{
		NONE,
		BOSS_LEFT_HAND,
		// 3관문 hook처럼 boss bone 대신 월드 오브젝트 끝이 플레이어를 운반하는 슬롯이다.
		// owner ID는 해제/despawn을 추적하기 위해 패턴 소유 보스를 가리킨다. 손 socket으로 해석하지 않는다.
		WORLD_HOOK_TIP,
		END
	};

	enum class PLAYER_STANCE_ID : std::uint8_t
	{
		NONE,
		LANCE_MASTER_LONG_SPEAR,
		LANCE_MASTER_SHORT_SPEAR,
		WARLORD_NORMAL,
		WARLORD_DEFENSE,
		GUARDIANKNIGHT_HUMAN,
		GUARDIANKNIGHT_DRAGON,
		END
	};

	struct SKILL_COOLDOWN_SNAPSHOT
	{
		SKILL_ID iSkillId = INVALID_SKILL_ID;
		std::uint32_t iCooldownEndTick = 0;
		std::uint32_t iCooldownDurationTicks = 0;
	};

	// Server가 확정한 플레이어의 지속 상태다. ClientReplication이 이동·행동·HUD를 갱신한다.
	// 위치는 월드 m, Y축 회전은 degree다. Tick/DurationTicks는 Server 시계, Seconds/Ms는 이름에 적힌 단위다.
	struct PLAYER_SNAPSHOT
	{
		// MOVE 목표가 gameplay lock/경로 검사로 거부돼도 해당 입력 sequence를 소비할 수 있다.
		// 이 ack는 입력 처리만 뜻하며 Client 위치를 정본으로 승인하는 값이 아니다.
		std::uint32_t iLastProcessedMoveSequence = 0u;
		// 현재 stance 배율을 포함한 실제 이동 속도(m/s)다.
		float fMoveSpeed = 0.f;
		// Server가 일반 클릭 이동을 현재 허용하는지 나타낸다.
		bool canPredictMove = false;
		// 일반 이동 경로의 다음 waypoint만 노출한다. 강제 이동과 Mario는 별도 Server 상태를 사용한다.
		// 일반 경로가 없으면 좌표는 0이어야 한다. Client가 만든 위치를 보내는 필드가 아니다.
		bool hasMoveGoal = false;
		float fMoveWaypointX = 0.f;
		float fMoveWaypointY = 0.f;
		float fMoveWaypointZ = 0.f;

		NET_ENTITY_ID iNetEntityId =
			INVALID_NET_ENTITY_ID;
		CHARACTER_CLASS_ID eCharacterClass = CHARACTER_CLASS_ID::END;

		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;

		PLAYER_LOCOMOTION_STATE eLocomotionState =
			PLAYER_LOCOMOTION_STATE::IDLE;

		PLAYER_ACTION_STATE eAction = PLAYER_ACTION_STATE::NONE;
		PLAYER_STANCE_ID eStance = PLAYER_STANCE_ID::NONE;
		SKILL_ID iSkillId = INVALID_SKILL_ID;
		std::uint32_t iActionStartTick = 0;
		// GRABBED일 때만 유효한 owner/slot이다. owner는 복제된 world entity를 식별한다.
		// 다른 action은 INVALID/NONE을 보내 해제 뒤 이전 attachment가 남지 않게 한다.
		NET_ENTITY_ID iAttachmentOwnerNetEntityId = INVALID_NET_ENTITY_ID;
		PLAYER_ATTACHMENT_SLOT eAttachmentSlot = PLAYER_ATTACHMENT_SLOT::NONE;
		// owner gameplay root 기준 local offset(m)과 상대 yaw(degree)다.
		// BOSS_LEFT_HAND는 Server 위치/방향 계산에 사용하고 Client는 손 socket과 복제 위치 fallback을 구분한다.
		// WORLD_HOOK_TIP은 별도 월드 경로를 사용한다. GRABBED 이외에서는 네 값 모두 0이어야 한다.
		float fAttachmentLocalOffsetX = 0.f;
		float fAttachmentLocalOffsetY = 0.f;
		float fAttachmentLocalOffsetZ = 0.f;
		float fAttachmentYawOffsetDegrees = 0.f;
		// Server가 승인한 GROUND_POINT skill 목표다. Y는 Server navigation 높이로 Client action Effect도 같은 표면을 기준으로 삼는다.
		// 목표를 쓰지 않는 action은 false와 0 좌표를 보낸다.
		bool hasSkillTarget = false;
		float fSkillTargetX = 0.f;
		float fSkillTargetY = 0.f;
		float fSkillTargetZ = 0.f;
		std::uint32_t iCurrentHp = 1;
		std::uint32_t iMaximumHp = 1;
		// HP보다 먼저 피해를 흡수하는 현재 shield다. HUD의 보호막 수치/막대로 표현한다.
		std::uint32_t iShield = 0;
		std::uint8_t iActiveBuffCount = 0;
		ACTIVE_BUFF ActiveBuffs[MAX_ACTIVE_BUFFS]{};
		std::uint32_t iCurrentResource = 0;
		std::uint32_t iMaximumResource = 1;
		// 직업 identity gauge다. maximum=0이면 이 gauge를 사용하지 않는 상태다.
		std::uint32_t iCurrentIdentity = 0;
		std::uint32_t iMaximumIdentity = 0;
		// Guardian Knight ember 상태다. maximum=0이면 ember 저장소를 사용하지 않는다.
		// 보유 orb와 잠긴 socket 수의 합은 maximum을 넘지 않아야 한다.
		std::uint8_t iEmberOrbs = 0;
		std::uint8_t iEmberLockedSockets = 0;
		std::uint8_t iEmberMaximumSockets = 0;
		// 쿠크 광기 gauge와 form은 Server가 결정한다. maximum=0이면 gauge가 비활성이다.
		// form은 Client가 표시할 플레이어 몸체를 정한다.
		std::uint32_t iCurrentMadness = 0;
		std::uint32_t iMaximumMadness = 0;
		PLAYER_MADNESS_FORM eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
		// Server가 승인한 vehicle ID이며 미탑승은 INVALID_VEHICLE_ID다.
		// 탑승 중 VEHICLE_SKILL도 가능하므로 이 필드만으로 action이 항상 NONE이라고 해석하지 않는다.
		VEHICLE_ID iVehicleId = INVALID_VEHICLE_ID;
		VEHICLE_FLIGHT_PHASE eVehicleFlightPhase = VEHICLE_FLIGHT_PHASE::GROUNDED;
		std::uint32_t iVehicleFlightPhaseStartTick = 0u;
		float fVehicleFlightPhaseDurationSeconds = 0.f;
		// 머리 위에 표시할 착용 칭호다. INVALID_HONOR_TITLE_ID는 미착용이다.
		HONOR_TITLE_ID iHonorTitleId = INVALID_HONOR_TITLE_ID;
		// 쿠크 카드 무늬/색과 Server가 고른 HUD mode 및 slot 배치를 담는다.
		// Q..F 각 slot은 mode별 icon/skill index이며 -1은 빈 칸이다. NONE mode는 전부 -1이어야 한다.
		MECHANIC_CARD_SYMBOL eMechanicCardSymbol = MECHANIC_CARD_SYMBOL::NONE;
		MECHANIC_CARD_COLOR eMechanicCardColor = MECHANIC_CARD_COLOR::NONE;
		KOUKU_HUD_MODE eKoukuHudMode = KOUKU_HUD_MODE::NONE;
		std::int8_t ModeSkillIndexBySlot[KOUKU_HUD_SLOT_COUNT] =
			{ -1, -1, -1, -1, -1, -1, -1, -1 };
		// Mario 밖은 0, 안에서는 Server 소유 stage 1..4다.
		std::uint8_t iMarioStage = 0u;
		// Server가 정한 Mario/카드 미로 종료 tick이다. 0은 비활성이다.
		std::uint32_t iKoukuMinigameEndTick = 0u;
		// Server가 고른 layout이다. 0은 미선택, 1..3은 원본 세 case다.
		std::uint8_t iMarioLayoutVariant = 0u;
		// Server가 터뜨린 공의 mask다. bit는 layout binding slot이며 stage 1..3은 9개, stage 4는 12개다.
		// Mario 밖에서는 0이다.
		std::uint16_t iMarioPoppedBallMask = 0u;
		// bit 0=빨강, 1=파랑, 2=노랑이다. 해당 색의 공 세 개를 터뜨려 저주를 해제했음을 나타낸다.
		std::uint8_t iMarioCurseReleasedMask = 0u;
		// 머리 위 marker다. 0=없음, 1=빨강, 2=파랑, 3=노랑이며 Mario 밖에도 표시할 수 있다.
		std::uint8_t iMarioMarkerColor = 0u;
		// 입장자의 목표 색/진행 수이며 marker를 표시하는 다른 플레이어와는 별개다.
		// Mario 밖은 둘 다 0이다. 색은 1=빨강, 2=파랑, 3=노랑이고 진행 수 상한은 3이다.
		std::uint8_t iMarioRequiredColor = 0u;
		std::uint8_t iMarioMatchingBallCount = 0u;
		// Server의 카드 미로 역할/무늬/처치 수다. NONE은 무늬 NONE과 0 개수를 보낸다.
		// HUNTER는 배정된 무늬를 가지며 처치 수는 목표 수를 넘지 않는다.
		CARD_MAZE_ROLE eCardMazeRole = CARD_MAZE_ROLE::NONE;
		MECHANIC_CARD_SYMBOL eCardMazeSuit = MECHANIC_CARD_SYMBOL::NONE;
		std::uint8_t iCardMazeKills = 0u;
		std::uint8_t iCardMazeKillTarget = 0u;
		CARD_MAZE_PRESENTATION CardMaze;
		bool isCombatReady = true;
		// Server가 소유하는 패턴 구속이다. Client는 종료 tick으로 남은 시간을 표시하되 해제 시점을 직접 정하지 않는다.
		bool isPatternBound = false;
		std::uint32_t iPatternBindEndTick = 0;
		// 침묵 종료 tick과 지속 tick 수다. 0이면 침묵이 없고 nonzero이면 직업 skill 입력을 제한한다.
		// 침묵 자체는 이동을 잠그지 않지만 다른 action/구속은 별도로 적용된다.
		std::uint32_t iSilenceEndTick = 0;
		std::uint32_t iSilenceDurationTicks = 0;
		// FEAR 시작은 iActionStartTick, 종료는 아래 Server tick으로 나타낸다.
		std::uint32_t iFearEndTick = 0u;
		std::string strFearPresentationId;
		// 활성 무적 zone의 최근 진입/2초 pulse tick이다. 해당 zone 밖은 0이다.
		std::uint32_t iInvulnerabilityZonePulseTick = 0u;
		// Server가 부여한 Ronaun 전멸 방어다. 소비해도 부여 occurrence의 tick은 남는다.
		bool bRonaunGuard = false;
		std::uint32_t iRonaunGrantTick = 0u;
		// 단계 action 밖은 0, 실행 중은 1부터 시작하는 번호다. COMBO와 HOLD의 start/loop/end가 사용한다.
		// 단계 진행은 Server가 결정하며 Client가 자체 증가시키지 않는다.
		std::uint8_t iComboStage = 0;
		COOLDOWN_MODE eCooldownMode = COOLDOWN_MODE::DEBUG_THREE_SECONDS;
		std::vector<SKILL_COOLDOWN_SNAPSHOT> Cooldowns;
		// Server 포물선 피격 반응이 공중 진행 중인지 나타낸다. KNOCKDOWN 밖과 착지 뒤에는 false다.
		bool isKnockbackAirborne = false;
		// Maharaka Waterpang 경기 중 Server가 무장시킨 참가자인지 나타낸다.
		bool isWaterpangArmed = false;
		// 물총의 최근 skill ID와 시전 시작 Server tick이다. 아직 시전하지 않았으면 둘 다 0이다.
		std::uint32_t iWaterGunSkillId = 0u;
		std::uint32_t iWaterGunCastTick = 0u;
		PLAYER_CONTROL_KIND eControlKind = PLAYER_CONTROL_KIND::HUMAN;
		// Server inventory의 AVATAR_HEAD/AVATAR_OUTFIT에 장착한 item ID다. 미착용은 빈 문자열이다.
		// Client가 직업 visual set으로 연결하며 메시지에는 model/visual set ID를 직접 넣지 않는다.
		std::string strAvatarHeadItemId;
		std::string strAvatarOutfitItemId;
	};

	enum class BOSS_COMBAT_STATE_FLAG : std::uint16_t
	{
		NONE = 0,
		INVULNERABLE = 1u << 0,
		SHIELDED = 1u << 1,
		COUNTERABLE = 1u << 2,
		GROGGY = 1u << 3,
		// 3페이즈 재배치의 숨김 경계다. 주 보스 identity와 HP를 유지하면서 Client 표현을 숨긴다.
		GHOST_HIDDEN = 1u << 4
	};

	inline constexpr std::uint16_t BOSS_COMBAT_STATE_KNOWN_FLAG_MASK =
		static_cast<std::uint16_t>(BOSS_COMBAT_STATE_FLAG::INVULNERABLE) |
		static_cast<std::uint16_t>(BOSS_COMBAT_STATE_FLAG::SHIELDED) |
		static_cast<std::uint16_t>(BOSS_COMBAT_STATE_FLAG::COUNTERABLE) |
		static_cast<std::uint16_t>(BOSS_COMBAT_STATE_FLAG::GROGGY) |
		static_cast<std::uint16_t>(BOSS_COMBAT_STATE_FLAG::GHOST_HIDDEN);

	[[nodiscard]]
	constexpr bool Has_BossCombatFlag(
		const std::uint16_t flags,
		const BOSS_COMBAT_STATE_FLAG flag) noexcept
	{
		return 0u != (flags & static_cast<std::uint16_t>(flag));
	}

	enum class BOSS_MECHANIC_GAUGE_KIND : std::uint8_t
	{
		NONE,
		STAGGER,
		BOSS_HP
	};

	struct BOSS_COMBAT_SNAPSHOT
	{
		std::uint32_t iStateRevision = 0;
		std::uint32_t iAlivePartMask = 0;
		std::uint16_t iFlags = 0;
		std::uint32_t iCurrentStagger = 0;
		std::uint32_t iMaximumStagger = 0;
		std::uint32_t iCurrentShield = 0;
		std::uint32_t iMaximumShield = 0;
		std::uint32_t iResponseProgress = 0;
		std::uint32_t iResponseThreshold = 0;
		// Server가 정한 기믹 gauge 종류와 잔여/최대 수치다. NONE이면 두 수치는 0이어야 한다.
		BOSS_MECHANIC_GAUGE_KIND eMechanicGaugeKind = BOSS_MECHANIC_GAUGE_KIND::NONE;
		std::uint32_t iCurrentMechanicGauge = 0;
		std::uint32_t iMaximumMechanicGauge = 0;
		std::uint8_t iGameplayPhase = 1;
	};

	// 지연된 Portal 돌진 stage의 시작/끝 경로다. Server가 확정한 값을 전달한다.
	// snapshot 병합으로 이동 중 처음 관찰해도 Client가 현재 boss pose에서 시작 경로를 추측하지 않게 한다.
	struct PORTAL_RUSH_ROUTE_SNAPSHOT
	{
		bool isValid = false;
		float fStartX = 0.f;
		float fStartY = 0.f;
		float fStartZ = 0.f;
		float fEndX = 0.f;
		float fEndY = 0.f;
		float fEndZ = 0.f;

		bool operator==(const PORTAL_RUSH_ROUTE_SNAPSHOT&) const = default;
	};

	// Server가 점프 패턴에 확정한 착지 anchor다. 지연/병합된 snapshot에도 담아 움직이는 대상이나 공중 boss 위치에서 다시 추측하지 않게 한다.
	struct PATTERN_LANDING_SNAPSHOT
	{
		bool isValid = false;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;

		bool operator==(const PATTERN_LANDING_SNAPSHOT&) const = default;
	};

	struct WORLD_ENTITY_SNAPSHOT
	{
		NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
		WORLD_ENTITY_ACTION eAction = WORLD_ENTITY_ACTION::END;
		std::string strPatternId;
		std::string strActionId;
		std::uint32_t iPatternSequence = 0;
		std::uint32_t iPatternStartTick = 0;
		std::uint32_t iPatternStageIndex = 0;
		// 같은 보스 위에서 재생할 Server 선택 animation child다. 다른 실행 경로는 부모 identity와 시계를 유지한다.
		std::string strPresentationPatternId;
		std::string strPresentationActionId;
		std::uint32_t iPresentationPatternStartTick = 0;
		std::uint32_t iPresentationActionStartTick = 0;
		std::uint32_t iPresentationPatternStageIndex = 0;
		// 보스 패턴이 고른 플레이어다. 비보스는 INVALID를 보내며 Client가 표현 대상을 다시 선택하지 않는다.
		NET_ENTITY_ID iPatternTargetNetEntityId = INVALID_NET_ENTITY_ID;
		PORTAL_RUSH_ROUTE_SNAPSHOT PortalRushRoute;
		PATTERN_LANDING_SNAPSHOT PatternLanding;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;
		std::uint32_t iActionStartTick = 0;
		std::uint32_t iCurrentHp = 1;
		std::uint32_t iMaximumHp = 1;
		std::uint8_t iActiveBuffCount = 0;
		ACTIVE_BUFF ActiveBuffs[MAX_ACTIVE_BUFFS]{};
		std::uint8_t iPhase = 1;
		// 방어구 판 i가 파괴되면 bit i가 켜진다. 내구도/파괴는 Server가, part 숨김은 Client가 처리한다.
		// 0은 파괴된 판이 없다는 뜻이며 방어구 없는 entity도 0을 보낸다.
		std::uint8_t iBrokenArmorMask = 0;
		// 보스만 true로 보내는 payload 존재 표시다. NPC/monster에 보스 상태를 매번 싣지 않도록 구분한다.
		bool hasBossCombatState = false;
		BOSS_COMBAT_SNAPSHOT BossCombat;
		// 이 occurrence의 정의를 소유한 고정 gameplay generation이다.
		GameplayDataRevision PinnedDefinitionRevision{};
	};
	// Server가 확정한 피해/회복/흡수 등의 숫자 표현 종류다. Client가 해당 DamageText 표시로 대응시킨다.
	// 일반 피해/회복은 양수 amount를 사용한다. INVINCIBLE은 막힌 outgoing 0-damage 결과이며 무력화/기믹 기여를 싣지 않는다.
	enum class DAMAGE_HIT_FLAG : std::uint8_t
	{
		NORMAL = 0,
		CRITICAL,
		MISS,
		INVINCIBLE,
		HEAL,
		ABSORB, // 실제 shield 흡수량이며 HP 피해와 구분한다.
		DAMAGE_REDUCED,
		CRITICAL_DAMAGE_REDUCED,
		END
	};
	inline bool Is_Valid_DamageHitFlag(const DAMAGE_HIT_FLAG eFlag)
	{
		return static_cast<std::uint8_t>(eFlag) <
			static_cast<std::uint8_t>(DAMAGE_HIT_FLAG::END);
	}
	// DAMAGE_EVENT는 한 번의 확정된 결과다. HP snapshot만으로는 500 피해 한 번과 250 피해 두 번을 구별할 수 없다.
	// Server가 숫자와 발생 위치를 보내며 Client는 HP 차이로 피해를 재계산하지 않는다.
	// 표현 이벤트와 지속 HP 상태는 서로 다른 계약이다.
	// Mario 접촉 판정에서 Server가 제공하는 표현 원인이다.
	enum class MARIO_HIT_SOURCE : std::uint8_t
	{
		NONE,
		FLYING_BALL,
		END
	};

	struct DAMAGE_EVENT
	{
		// 대상은 플레이어 또는 world entity이며 둘 다 NET_ENTITY_ID로 식별한다.
		NET_ENTITY_ID iTargetNetEntityId = INVALID_NET_ENTITY_ID;
		std::uint32_t iAmount = 0;
		// 발생 순간의 월드 위치(m)다. 숫자가 이후 대상 이동을 따라가지 않도록 고정 위치를 보낸다.
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		// 플레이어가 가한 결과는 outgoing이다. 받는 피해/주는 피해의 표시 구분에 사용한다.
		// HEAL도 대상이 얻는 회복 결과로 outgoing에 포함된다.
		bool isOutgoing = false;
		// 일반 hit는 NONE이다. 카드 미로 조각 이벤트는 사냥꾼 무늬를 담고 iAmount를 누적 처치 수로 사용한다.
		// 모든 DAMAGE_EVENT의 iAmount를 HP 피해로 합산해서는 안 된다.
		MECHANIC_CARD_SYMBOL eCardMazeSuit = MECHANIC_CARD_SYMBOL::NONE;
		// 전투 분석 입력이다. iSourcePlayerId는 기여한 플레이어이며 world→player/기믹은 INVALID일 수 있다.
		// iStaggerAmount는 무력화 기여, isCounterSuccess는 counter 성공이다. HP 피해 0에도 이 기여를 담을 수 있다.
		PLAYER_ID iSourcePlayerId = INVALID_PLAYER_ID;
		std::uint32_t iStaggerAmount = 0;
		bool isCounterSuccess = false;
		// Server가 확인한 무력화 임계 도달 이벤트다. 이후 groggy animation과 별개다.
		bool isStaggerSuccess = false;
		MARIO_HIT_SOURCE eMarioHitSource = MARIO_HIT_SOURCE::NONE;
		// Server가 정한 DamageText 표시 종류를 Client가 해당 표현으로 그린다.
		DAMAGE_HIT_FLAG eHitFlag = DAMAGE_HIT_FLAG::NORMAL;
	};

	enum class BOSS_COMBAT_EVENT_KIND : std::uint8_t
	{
		PART_BROKEN,
		END
	};

	struct BOSS_COMBAT_EVENT
	{
		std::uint64_t iEventSequence = 0;
		std::uint32_t iEventTick = 0;
		NET_ENTITY_ID iBossNetEntityId = INVALID_NET_ENTITY_ID;
		BOSS_COMBAT_EVENT_KIND eKind = BOSS_COMBAT_EVENT_KIND::END;
		std::uint32_t iPartMask = 0;
	};

	// 방 전체가 공유하는 빙고 상태다. 앞의 두 U32 mask가 8바이트이며 아래 폭탄/망치를 포함한 전체 구조체 크기가 아니다.
	// white는 칠한 칸, red는 완성 line의 칸이다. red는 white의 부분집합이어야 한다.
	struct BINGO_BOARD_SNAPSHOT
	{
		std::uint32_t iWhiteMask = 0u;
		std::uint32_t iRedMask = 0u;
		// 활성 폭탄을 앞의 iBombCount개 slot에 연속으로 담는다.
		std::uint8_t iBombCount = 0u;
		BINGO_BOMB_SNAPSHOT Bombs[KOUKU_BINGO_MAX_BOMBS]{};
		// 동시에 실행하는 망치는 최대 하나다.
		BINGO_HAMMER_SNAPSHOT Hammer{};
	};

	constexpr bool Is_Valid_BingoBoardSnapshot(
		const BINGO_BOARD_SNAPSHOT& board) noexcept
	{
		if (0u != (board.iWhiteMask & ~KOUKU_BINGO_ALL_CELLS_MASK) ||
			board.iRedMask != (board.iRedMask & board.iWhiteMask) ||
			static_cast<std::int32_t>(board.iBombCount) > KOUKU_BINGO_MAX_BOMBS)
		{
			return false;
		}
		// 사용 중인 slot은 활성 단계를 가져야 한다. MARKED에는 따라갈 carrier가 필요하다.
		for (std::uint8_t index = 0u; index < board.iBombCount; ++index)
		{
			const BINGO_BOMB_SNAPSHOT& bomb = board.Bombs[index];
			if (BINGO_BOMB_PHASE::NONE == bomb.ePhase ||
				bomb.ePhase >= BINGO_BOMB_PHASE::END)
			{
				return false;
			}
			if (BINGO_BOMB_PHASE::MARKED == bomb.ePhase &&
				INVALID_NET_ENTITY_ID == bomb.iCarrierNetEntityId)
			{
				return false;
			}
		}
		return Is_Valid_BingoHammerSnapshot(board.Hammer);
	}

	struct COMBAT_OBJECT_SNAPSHOT
	{
		COMBAT_OBJECT_ID iCombatObjectId = INVALID_COMBAT_OBJECT_ID;
		NET_ENTITY_ID iSourceNetEntityId = INVALID_NET_ENTITY_ID;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;
		GameplayDataRevision PinnedDefinitionRevision{};
	};

	inline constexpr std::size_t MAX_WORLD_PICKUPS = 16u;
	enum class WORLD_PICKUP_STATE : std::uint8_t
	{
		WALL,
		FALLING,
		GROUNDED,
		COLLECTED,
		REMOVED,
		END
	};
	struct WORLD_PICKUP_SNAPSHOT
	{
		std::string strPlacementId;
		WORLD_PICKUP_STATE eState = WORLD_PICKUP_STATE::WALL;
		float fPositionX = 0.f, fPositionY = 0.f, fPositionZ = 0.f;
		std::uint32_t iStateStartTick = 0u;
		bool operator==(const WORLD_PICKUP_SNAPSHOT&) const = default;
	};

	//player snapshot을 vector 구조체로 들고, servertick을 들고있다?
	// PLAYER_SNAPSHOT의 목록과 그 목록을 만든 Server tick을 함께 보낸다. 아래 Entities·판정 사건·보스 보조 상태도 같은 메시지에 포함된다.
	struct S2C_WORLD_SNAPSHOT
	{
		std::uint32_t iServerTick = 0;
		WORLD_ID eWorldId = WORLD_ID::BERN;
		std::vector<PLAYER_SNAPSHOT> Players;
		std::vector<WORLD_ENTITY_SNAPSHOT> Entities;
		std::vector<DAMAGE_EVENT> DamageEvents;
		BINGO_BOARD_SNAPSHOT Bingo;
		std::vector<BOSS_COMBAT_EVENT> BossCombatEvents;
		// 현재 살아 있는 combat object 중 bReplicated인 객체 전체를 iCombatObjectId 순으로 보낸다.
		// 생성·제거의 reliable frame 순서가 이 30Hz 위치 갱신의 앞뒤 경계를 지킨다.
		std::vector<COMBAT_OBJECT_SNAPSHOT> CombatObjects;
		// stable ID 순서의 전체 상태다. 수집·제거된 항목도 삭제 사실을 나타내는 상태로 남는다.
		std::vector<WORLD_PICKUP_SNAPSHOT> WorldPickups;
		// 방 전체가 공유하는 레이드 에스더 게이지다. maximum이 0이면 이 world는 에스더 명단을 제공하지 않으므로 HUD도 게이지를 그리지 않는다.
		std::uint32_t iEstherGauge = 0;
		std::uint32_t iEstherGaugeMaximum = 0;
		GameplayDataRevision ActiveGameplayRevision{};
		std::vector<GameplayDataRevision> RequiredPinnedGameplayRevisions;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_WORLD_SNAPSHOT& message);

	bool Read_Message(
		CPacketReader& reader,
		S2C_WORLD_SNAPSHOT& message);

	// 지속되는 파괴 상태는 30Hz world snapshot과 별도로 복제한다.
	// Full sync에는 과거 일회성 사건을 싣지 않고, delta에는 이번에 달라진 상태와 현재 방에서 발생한 사건만 싣는다.
	inline constexpr std::size_t MAX_COMBAT_RUNTIME_REVISION_BYTES = 64;
	inline constexpr std::size_t MAX_WORLD_DESTRUCTION_GROUPS = 128;
	inline constexpr std::size_t MAX_WORLD_DESTRUCTION_CHANGED_STATES = 128;
	// 상태 변경 128개가 모두 들어가는 최대 길이 delta도 64KiB frame 안에 들어가야 한다.
	// 사건 목록의 상한 106은 같은 frame의 상태·ID·사건 크기를 함께 제한하기 위한 값이다.
	inline constexpr std::size_t MAX_WORLD_DESTRUCTION_EVENTS = 106;

	enum class WORLD_DESTRUCTION_RUNTIME_STATE : std::uint8_t
	{
		INTACT,
		BREAKING,
		FRACTURED,
		DESPAWNED,
		END
	};

	struct WORLD_DESTRUCTION_STATE_WIRE
	{
		std::string strGroupId;
		WORLD_DESTRUCTION_RUNTIME_STATE eState =
			WORLD_DESTRUCTION_RUNTIME_STATE::INTACT;
		std::uint32_t iStateVersion = 0;
		std::uint32_t iStateStartTick = 0;
		std::uint32_t iCommitTick = 0;
	};

	struct WORLD_DESTRUCTION_EVENT_WIRE
	{
		std::uint64_t iEventSequence = 0;
		std::string strGroupId;
		std::string strMutationId;
		std::string strBindingId;
		std::uint32_t iPatternSequence = 0;
		std::uint64_t iSourceNetEntityId = 0;
		std::uint32_t iServerTick = 0;
		float fImpactOriginX = 0.f;
		float fImpactOriginY = 0.f;
		float fImpactOriginZ = 0.f;
		float fImpactDirectionX = 0.f;
		float fImpactDirectionY = 0.f;
		float fImpactDirectionZ = 0.f;
		std::uint32_t iRandomSeed = 0;
	};

	// Server가 실제로 보유한 충돌체·navigation blocker의 개수와 revision이다.
	// Debug 재생 패널은 복제된 벽 모양으로 통행 가능 여부를 추측하지 않고 이 값을 표시한다.
	// 벽 표현은 사라졌지만 blocker가 남은 경우 두 상태의 불일치를 드러내기 위한 진단 입력이다.
	struct WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS
	{
		std::uint32_t iActiveWallCollisionCount = 0;
		std::uint32_t iActiveNavBlockerRegionCount = 0;
		std::uint64_t iNavigationRevision = 0;
		std::uint64_t iLastEventSequence = 0;
	};

	struct S2C_WORLD_DESTRUCTION_FULL_SYNC
	{
		std::string strCombatRuntimeRevision;
		std::uint32_t iServerTick = 0;
		std::uint32_t iEncounterEpoch = 0;
		std::vector<WORLD_DESTRUCTION_STATE_WIRE> GroupStates;
		WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS Diagnostics;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_WORLD_DESTRUCTION_FULL_SYNC& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_WORLD_DESTRUCTION_FULL_SYNC& message);

	struct S2C_WORLD_DESTRUCTION_DELTA
	{
		std::string strCombatRuntimeRevision;
		std::uint32_t iServerTick = 0;
		std::uint32_t iEncounterEpoch = 0;
		std::vector<WORLD_DESTRUCTION_STATE_WIRE> ChangedStates;
		std::vector<WORLD_DESTRUCTION_EVENT_WIRE> LiveEvents;
		WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS Diagnostics;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_WORLD_DESTRUCTION_DELTA& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_WORLD_DESTRUCTION_DELTA& message);

	// 반복해서 등장하는 기둥 등의 소품은 한 번 파괴되는 벽과 별도로 복제한다.
	// 슬롯은 HIDDEN → SPAWNING → INTACT → BREAKING → HIDDEN으로 재사용한다.
	// 뒤늦게 들어온 Client에는 현재 슬롯 상태를 보내며 과거 생성·파괴 연출을 다시 실행하지 않는다.
	inline constexpr std::size_t MAX_ENCOUNTER_PROP_SLOTS = 16;

	enum class ENCOUNTER_PROP_STATE : std::uint8_t
	{
		HIDDEN,
		SPAWNING,
		INTACT,
		BREAKING,
		END
	};

	struct ENCOUNTER_PROP_SLOT_WIRE
	{
		std::string strSlotId;
		ENCOUNTER_PROP_STATE eState = ENCOUNTER_PROP_STATE::HIDDEN;
		std::uint32_t iStateVersion = 0;
		std::uint32_t iStateStartTick = 0;
		std::uint32_t iOccurrenceSequence = 0;
	};

	struct S2C_ENCOUNTER_PROP_SYNC
	{
		std::string strPropSetId;
		std::uint32_t iServerTick = 0;
		std::uint32_t iEncounterEpoch = 0;
		std::vector<ENCOUNTER_PROP_SLOT_WIRE> Slots;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_ENCOUNTER_PROP_SYNC& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_ENCOUNTER_PROP_SYNC& message);

	// 발탄 Debug 재생 명령은 체력줄 교차, 타임라인 행, stable pattern ID 등 operation별 대상을 다룬다.
	// 체력줄 재생은 목표 바로 앞 줄을 준비한 뒤 목표 줄로 옮겨 CValtanBrain이 그 교차를 판단하게 한다.
	// 그래야 더 높은 모든 체력줄 기믹이 함께 예약되는 것을 피하고 패턴·카메라·벽 연결을 기존 실행 경로로 확인할 수 있다.
	// ARM/CROSS는 준비와 교차를 나누어 확인하며, PLAY_ENTRANCE는 입장 연출의 소비 상태를 되돌리는 전용 명령이다.
	// 구체적인 인자 의미와 허용 조건은 아래 operation과 GameRoom의 해당 명령 소비자가 결정한다.
	enum class VALTAN_ARENA_PRESET : std::uint32_t
	{
		FRESH = 1u,
		CIRCLE_WALLS_GONE,
		THREE_OCLOCK_BROKEN,
		NINE_OCLOCK_BROKEN,
		BOTH_SIDES_BROKEN,
		END
	};

	enum class VALTAN_AUDITION_OPERATION : std::uint8_t
	{
		ARM_HEALTH_BAR,
		CROSS_HEALTH_BAR,
		PLAY_HEALTH_BAR,
		PLAY_ENTRANCE,
		// 기둥 한 주기를 Debug에서 재생한다. 기존 패턴이 네 슬롯을 올리고 이 명령의 제어가 INTACT → BREAKING → HIDDEN을 진행한다.
		PLAY_PILLAR_CYCLE,
		// PLAY_WALL_ATTACK은 실제 VALTAN_DOWN_SMASH를 일반 벽에 예약한다.
		// SHOW_FINAL_ARENA는 Server 파괴 런타임으로 벽·지형 상태를 준비해 열린 아레나를 확인하는 명령이다.
		PLAY_WALL_ATTACK,
		SHOW_FINAL_ARENA,
		// SHOW_FINAL_ARENA의 파괴 경로를 사용하되 두 바닥 단계는 남긴다.
		// 벽을 먼저 제거하고 이후 바닥 붕괴를 별도로 재생할 때 사용하는 명령이다.
		BREAK_EVERY_WALL,
		// 선택한 타임라인 행을 시작하거나 현재 Debug 재생을 중지한다.
		// PLAY의 iTargetHealthBar는 0이 아닌 행 command ID이며 STOP은 정확히 0을 보낸다.
		PLAY_TIMELINE_ROW,
		STOP_TIMELINE_ROW,
		// 체력줄로 고르기 어려운 일반 패턴을 encounter 문서상의 순서로 재생하는 기존 명령이다.
		// iTargetHealthBar는 체력줄 대신 1부터 시작하는 패턴 index를 담는다.
		// Client encounter와 Server 게시 PATTERN 행이 같은 저작 순서를 사용해야 하는 경로다.
		PLAY_PATTERN,
		// 도구 간 Server 미리보기에서 사용하는 stable ID 재생이다.
		// 배열 위치를 공유한다고 가정하지 않고 이미 생성된 boss placement와 그 encounter의 pattern ID를 함께 지정한다.
		PLAY_PATTERN_ID,
		// 검증된 페이지 경계에서 Server 권위의 정상 encounter 재생을 시작한다.
		// iTargetHealthBar는 페이지 경계 타임라인 행의 stable command ID이며 Server가 허용한 페이지 행만 받는다.
		START_FIGHT_PAGE,
		QUEUE_NEXT_PATTERN_ID,
		CLEAR_NEXT_PATTERN_ID,
		// 아레나를 초기화하지 않고 관찰한 Product 또는 소유 Flow의 현재 발생을 이어받는다.
		// Server는 Next 생명주기에서 새 재생 epoch를 부여한다.
		QUEUE_NEXT_LIVE_PATTERN_ID,
		// Workbench가 환경을 준비할 때 하나의 VALTAN_ARENA_PRESET을 보낸다. 그 결과 생기는 상태 변경은 Server가 소유한다.
		SET_ARENA_PRESET,
		// 이전 room epoch·pattern sequence·고정 definition revision이 가리키는 정확한 root 발생을 교체한다.
		// 현재 Server는 해당 root의 ACTIVE 또는 COMPLETED_HOLD 상태를 확인하며 pattern ID만 같은 다른 발생은 재시작하지 않는다.
		RESTART_PATTERN_ID,
		END
	};

	enum class VALTAN_AUDITION_RESULT : std::uint8_t
	{
		ARMED,
		QUEUED,
		// 같은 요청 sequence가 다시 오면 결과를 회신하되 패턴은 다시 실행하지 않는다.
		DUPLICATE_IGNORED,
		REJECTED_RELEASE_BUILD,
		REJECTED_WRONG_WORLD,
		REJECTED_NO_BOSS,
		REJECTED_BOSS_DEAD,
		// 해당 줄에 저작된 HEALTH_BAR 패턴이 없어 재생을 거절한 결과다.
		REJECTED_UNKNOWN_HEALTH_BAR,
		// 이미 소비한 기믹이거나 실행 중인 패턴 등으로 요청한 패턴을 사용할 수 없다.
		REJECTED_PATTERN_UNAVAILABLE,
		// 같은 체력줄을 ARM으로 준비하지 않은 CROSS는 알 수 없는 범위의 기믹을 넘을 수 있으므로 거절한다.
		REJECTED_NOT_ARMED,
		// 교전 거리 안에 전투 가능한 대상이 없어 예약한 패턴을 시작할 수 없는 경우다.
		REJECTED_PLAYER_NOT_ENGAGED,
		CLEARED,
		REJECTED_STALE_REQUEST,
		REJECTED_STALE_AUDITION,
		REJECTED_NOT_OWNER,
		REJECTED_NEXT_CHANGED,
		// 정확한 이전 발생은 확인했지만 교체 사전 검증에서 거절되어 보스·플레이어·아레나 상태를 보존한 결과다.
		// Client가 이전 발생의 표현을 계속 유효한 Server 상태로 취급할 수 있는 명시적인 거절 종류다.
		REJECTED_OCCURRENCE_PRESERVED,
		END
	};

	struct C2S_VALTAN_AUDITION_REQUEST
	{
		std::uint32_t iRequestSequence = 0;
		VALTAN_AUDITION_OPERATION eOperation =
			VALTAN_AUDITION_OPERATION::ARM_HEALTH_BAR;
		// 보통은 체력줄이다. PLAY_PATTERN에서는 1-based 패턴 index, PLAY_TIMELINE_ROW와 START_FIGHT_PAGE에서는 행 command ID다.
		// STOP_TIMELINE_ROW는 0, SET_ARENA_PRESET은 VALTAN_ARENA_PRESET 값을 담는다.
		std::uint32_t iTargetHealthBar = 0;
		// stable ID operation에서만 이 문자열들을 직렬화한다. 기존 operation은 원래 wire 형식을 유지하고 이 필드를 비워 둔다.
		std::string strBossPlacementId;
		std::string strPatternId;
		// Next 제어는 현재 발생의 식별자와 다음 한 칸 예약의 sequence를 함께 비교한다.
		// LIVE는 epoch·예약 token이 0이고 관찰한 pattern sequence를 사용한다. 아직 실행한 패턴이 없을 때만 관찰 sequence도 0이다.
		std::uint32_t iPredecessorRoomAuditionEpoch = 0u;
		std::uint32_t iPredecessorPatternSequence = 0u;
		std::uint32_t iExpectedNextRequestSequence = 0u;
		// PLAY_PATTERN_ID는 Client가 관찰한 active 정의 세대와 비교한다.
		// RESTART_PATTERN_ID와 Next 제어는 이전 발생이 고정한 정의 세대와 비교한다. 기존 operation만 0을 유지한다.
		GameplayDataRevision ExpectedDefinitionRevision{};
		// RESTART_PATTERN_ID만 사용하는 두 번째 revision 비교값이다.
		// 이전 발생은 ExpectedDefinitionRevision에 남고 교체 발생은 이 값과 일치하는 Server active 세대를 사용해야 한다.
		// 동시 데이터 활성화 때문에 Client가 확인하지 않은 세대로 재시작하는 것을 막으며 다른 operation은 0을 유지한다.
		GameplayDataRevision ReplacementDefinitionRevision{};
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_VALTAN_AUDITION_REQUEST& message);
	bool Read_Message(
		CPacketReader& reader,
		C2S_VALTAN_AUDITION_REQUEST& message);

	struct S2C_VALTAN_AUDITION_RESULT
	{
		std::uint32_t iRequestSequence = 0;
		VALTAN_AUDITION_OPERATION eOperation =
			VALTAN_AUDITION_OPERATION::ARM_HEALTH_BAR;
		std::uint32_t iTargetHealthBar = 0;
		VALTAN_AUDITION_RESULT eResult =
			VALTAN_AUDITION_RESULT::REJECTED_WRONG_WORLD;
		// 요청을 처리한 뒤 보스가 실제로 가진 체력줄이다. 요청이 거절되어도 현재 상태를 전달한다.
		std::uint32_t iCurrentHealthBar = 0;
		// stable ID 응답 소비자가 원래 요청과 대조할 식별자다. 해당 operation에서만 직렬화하며 기존 frame 형식을 바꾸지 않는다.
		std::string strBossPlacementId;
		std::string strPatternId;
		std::uint32_t iPredecessorRoomAuditionEpoch = 0u;
		std::uint32_t iPredecessorPatternSequence = 0u;
		std::uint32_t iExpectedNextRequestSequence = 0u;
		GameplayDataRevision ExpectedDefinitionRevision{};
		GameplayDataRevision ReplacementDefinitionRevision{};
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_VALTAN_AUDITION_RESULT& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_VALTAN_AUDITION_RESULT& message);

	inline constexpr std::size_t MAX_VALTAN_AUDITION_LIFECYCLE_REASON_BYTES =
		192u;

	enum class VALTAN_AUDITION_LIFECYCLE_STATE : std::uint8_t
	{
		PENDING,
		ACTIVE,
		COMPLETED,
		ABORTED,
		NEXT_RESERVED,
		WAITING_FOR_PLAYER,
		END
	};

	struct S2C_VALTAN_AUDITION_LIFECYCLE
	{
		std::uint32_t iRequestSequence = 0;
		// Server 방의 재생 식별자다. 새 재생을 시작할 때 증가하며 Client 수신 generation과는 별개의 값이다.
		std::uint32_t iRoomAuditionEpoch = 0;
		std::uint32_t iPatternSequence = 0;
		std::string strPatternId;
		VALTAN_AUDITION_LIFECYCLE_STATE eState =
			VALTAN_AUDITION_LIFECYCLE_STATE::PENDING;
		GameplayDataRevision PinnedDefinitionRevision{};
		// ABORTED일 때만 길이 제한이 있는 진단 사유를 담고 그 외에는 비어 있다.
		std::string strReason;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_VALTAN_AUDITION_LIFECYCLE& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_VALTAN_AUDITION_LIFECYCLE& message);

	// Boss Tool의 순차 Flow는 stable slot ID와 vector 순서로 재생 순서를 표현한다.
	// 서로 다른 슬롯이 같은 pattern을 재생할 수 있으므로 pattern ID 중복은 허용한다.
	// wire 개수는 1byte이므로 최대 255개까지 표현하며 저작·런타임도 이 상한을 공유한다.
	inline constexpr std::size_t MAX_VALTAN_PATTERN_FLOW_SLOTS = 255u;
	static_assert(
		MAX_VALTAN_PATTERN_FLOW_SLOTS ==
			(static_cast<std::size_t>((std::numeric_limits<std::uint8_t>::max)())));
	inline constexpr std::uint32_t
		MIN_VALTAN_PATTERN_FLOW_INTER_STEP_PURSUIT_MS = 100u;
	inline constexpr std::uint32_t
		MAX_VALTAN_PATTERN_FLOW_INTER_STEP_PURSUIT_MS = 10000u;
	inline constexpr std::size_t VALTAN_PATTERN_FLOW_REVISION_HEX_BYTES = 64u;
	inline constexpr std::size_t MAX_VALTAN_PATTERN_FLOW_REASON_BYTES = 192u;

	struct VALTAN_PATTERN_FLOW_SLOT_WIRE
	{
		std::string strSlotId;
		std::string strPatternId;
	};

	struct C2S_DEBUG_VALTAN_PATTERN_FLOW_START
	{
		std::uint32_t iRequestSequence = 0;
		// Client가 저장한 Flow를 제출할 때 확인한 gameplay 정의 세대다.
		// Server는 플레이어·보스·아레나·Flow 상태를 준비하기 전에 active catalog와 이 값을 비교한다.
		GameplayDataRevision ExpectedDefinitionRevision{};
		std::string strBossPlacementId;
		std::string strFlowId;
		// 승인한 저장 저작 바이트의 SHA-256을 소문자 64자리 16진수로 표현한다.
		std::string strFlowRevision;
		std::string strStartSlotId;
		std::uint32_t iInterStepPursuitMs = 0;
		std::vector<VALTAN_PATTERN_FLOW_SLOT_WIRE> Slots;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_DEBUG_VALTAN_PATTERN_FLOW_START& message);
	bool Read_Message(
		CPacketReader& reader,
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START& message);

	struct C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT
	{
		std::uint32_t iControlSequence = 0;
		std::string strFlowId;
		std::uint32_t iRoomFlowEpoch = 0;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& message);
	bool Read_Message(
		CPacketReader& reader,
		C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& message);

	enum class VALTAN_PATTERN_FLOW_COMMAND : std::uint8_t
	{
		START,
		STOP_AFTER_CURRENT,
		END
	};

	enum class VALTAN_PATTERN_FLOW_RESULT : std::uint8_t
	{
		QUEUED,
		DUPLICATE_IGNORED,
		REJECTED_RELEASE_BUILD,
		REJECTED_WRONG_WORLD,
		REJECTED_NO_BOSS,
		REJECTED_BOSS_DEAD,
		REJECTED_PLAYER_NOT_ENGAGED,
		REJECTED_CONFLICT,
		REJECTED_INVALID_FLOW,
		REJECTED_STALE_FLOW,
		REJECTED_STALE_DEFINITION,
		END
	};

	struct S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT
	{
		std::uint32_t iCommandSequence = 0;
		VALTAN_PATTERN_FLOW_COMMAND eCommand =
			VALTAN_PATTERN_FLOW_COMMAND::START;
		VALTAN_PATTERN_FLOW_RESULT eResult =
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_INVALID_FLOW;
		std::string strFlowId;
		std::string strFlowRevision;
		std::uint32_t iRoomFlowEpoch = 0;
		GameplayDataRevision PinnedDefinitionRevision{};
		std::string strReason;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT& message);

	enum class VALTAN_PATTERN_FLOW_LIFECYCLE_STATE : std::uint8_t
	{
		PENDING,
		ACTIVE,
		PAUSED_FOR_REVIVE,
		COMPLETED_HOLD,
		STOPPED_HOLD,
		REJECTED,
		ABORTED,
		END
	};

	struct S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE
	{
		std::uint32_t iRequestSequence = 0;
		std::uint32_t iRoomFlowEpoch = 0;
		std::uint32_t iPatternSequence = 0;
		std::string strBossPlacementId;
		std::string strFlowId;
		std::string strFlowRevision;
		std::string strStartSlotId;
		std::string strCurrentSlotId;
		std::string strCurrentPatternId;
		// 제출된 슬롯 vector에서 1부터 세는 표시용 위치다. stable slot ID를 대신하는 저장 식별자는 아니다.
		std::uint16_t iCurrentSlotOrdinal = 0;
		std::uint16_t iSlotCount = 0;
		VALTAN_PATTERN_FLOW_LIFECYCLE_STATE eState =
			VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PENDING;
		GameplayDataRevision PinnedDefinitionRevision{};
		std::string strReason;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE& message);

	// 응답은 Server가 보관한 결정 하나와 최대 64개 후보를 싣는다.
	// query의 after-sequence보다 새 결정이 없으면 후보 전체를 재전송하지 않고 UNCHANGED를 응답한다.
	inline constexpr std::size_t MAX_VALTAN_DECISION_TRACE_CANDIDATES = 64u;

	enum class VALTAN_DECISION_TRACE_SOURCE : std::uint8_t
	{
		NONE,
		INTRO,
		FORCED_HEALTH_BAR,
		FORCED_AUDITION,
		ORDERED,
		WEIGHTED,
		GLOBAL,
		END
	};

	enum class VALTAN_DECISION_TRACE_RESULT : std::uint8_t
	{
		SELECTED,
		WAITING_FOR_INTRO_RANGE,
		NO_ELIGIBLE_PATTERN,
		NO_VALID_TARGET,
		CATALOG_UNAVAILABLE,
		MECHANIC_RESET_REQUIRED,
		END
	};

	enum VALTAN_DECISION_TRACE_EXCLUSION : std::uint32_t
	{
		VALTAN_DECISION_TRACE_EXCLUDE_NONE = 0u,
		VALTAN_DECISION_TRACE_EXCLUDE_WRONG_SELECTION_KIND = 1u << 0u,
		VALTAN_DECISION_TRACE_EXCLUDE_INTRO_ROW = 1u << 1u,
		VALTAN_DECISION_TRACE_EXCLUDE_NOT_IN_SELECTION_SET = 1u << 2u,
		VALTAN_DECISION_TRACE_EXCLUDE_ARMOR_MISMATCH = 1u << 3u,
		VALTAN_DECISION_TRACE_EXCLUDE_PHASE_REQUIREMENT = 1u << 4u,
		VALTAN_DECISION_TRACE_EXCLUDE_PHASE_RANGE = 1u << 5u,
		VALTAN_DECISION_TRACE_EXCLUDE_HEALTH_BAR_RANGE = 1u << 6u,
		VALTAN_DECISION_TRACE_EXCLUDE_NO_TARGET = 1u << 7u,
		VALTAN_DECISION_TRACE_EXCLUDE_BELOW_MINIMUM_RANGE = 1u << 8u,
		VALTAN_DECISION_TRACE_EXCLUDE_ABOVE_MAXIMUM_RANGE = 1u << 9u,
		VALTAN_DECISION_TRACE_EXCLUDE_COOLDOWN = 1u << 10u,
		VALTAN_DECISION_TRACE_EXCLUDE_SOFT_REPEAT_BLOCKED = 1u << 11u,
		VALTAN_DECISION_TRACE_EXCLUDE_SOFT_REPEAT_RELAXED = 1u << 12u,
		VALTAN_DECISION_TRACE_EXCLUDE_DISABLED = 1u << 13u,
		VALTAN_DECISION_TRACE_EXCLUDE_UNRESOLVED_DEFINITION = 1u << 14u
	};

	inline constexpr std::uint32_t
		VALTAN_DECISION_TRACE_KNOWN_EXCLUSION_MASK =
		VALTAN_DECISION_TRACE_EXCLUDE_WRONG_SELECTION_KIND |
		VALTAN_DECISION_TRACE_EXCLUDE_INTRO_ROW |
		VALTAN_DECISION_TRACE_EXCLUDE_NOT_IN_SELECTION_SET |
		VALTAN_DECISION_TRACE_EXCLUDE_ARMOR_MISMATCH |
		VALTAN_DECISION_TRACE_EXCLUDE_PHASE_REQUIREMENT |
		VALTAN_DECISION_TRACE_EXCLUDE_PHASE_RANGE |
		VALTAN_DECISION_TRACE_EXCLUDE_HEALTH_BAR_RANGE |
		VALTAN_DECISION_TRACE_EXCLUDE_NO_TARGET |
		VALTAN_DECISION_TRACE_EXCLUDE_BELOW_MINIMUM_RANGE |
		VALTAN_DECISION_TRACE_EXCLUDE_ABOVE_MAXIMUM_RANGE |
		VALTAN_DECISION_TRACE_EXCLUDE_COOLDOWN |
		VALTAN_DECISION_TRACE_EXCLUDE_SOFT_REPEAT_BLOCKED |
		VALTAN_DECISION_TRACE_EXCLUDE_SOFT_REPEAT_RELAXED |
		VALTAN_DECISION_TRACE_EXCLUDE_DISABLED |
		VALTAN_DECISION_TRACE_EXCLUDE_UNRESOLVED_DEFINITION;

	struct VALTAN_DECISION_TRACE_CANDIDATE_WIRE
	{
		std::string strPatternId;
		std::uint32_t iExclusionMask =
			VALTAN_DECISION_TRACE_EXCLUDE_NONE;
		std::uint32_t iAuthoredWeight = 0u;
		std::uint32_t iEffectiveWeight = 0u;
		std::uint32_t iCooldownRemainingTicks = 0u;
		std::uint32_t iConsecutiveUses = 0u;
		std::uint32_t iMaximumConsecutiveUses = 0u;
		std::uint64_t iWeightBeginInclusive = 0u;
		std::uint64_t iWeightEndExclusive = 0u;
		bool isSelected = false;
	};

	struct VALTAN_DECISION_TRACE_WIRE
	{
		std::uint64_t iTraceSequence = 0u;
		std::uint32_t iServerTick = 0u;
		std::uint32_t iPatternSequenceBeforeDecision = 0u;
		std::uint32_t iExpectedPatternSequence = 0u;
		std::uint32_t iCurrentHp = 0u;
		std::uint32_t iMaximumHp = 0u;
		std::uint32_t iHealthBar = 0u;
		std::uint8_t iGameplayPhase = 1u;
		NET_ENTITY_ID iTargetNetEntityId = INVALID_NET_ENTITY_ID;
		float fTargetDistance = 0.f;
		bool isIntroPatternConsumed = false;
		std::uint32_t iRotationStepIndex = 0u;
		VALTAN_DECISION_TRACE_SOURCE eSource =
			VALTAN_DECISION_TRACE_SOURCE::NONE;
		VALTAN_DECISION_TRACE_RESULT eResult =
			VALTAN_DECISION_TRACE_RESULT::NO_ELIGIBLE_PATTERN;
		std::string strRotationId;
		std::string strPendingPatternId;
		VALTAN_DECISION_TRACE_SOURCE ePendingSource =
			VALTAN_DECISION_TRACE_SOURCE::NONE;
		std::string strSelectedPatternId;
		std::uint64_t iRawRandomInput = 0u;
		std::uint64_t iMixedRandomValue = 0u;
		std::uint64_t iTotalWeight = 0u;
		std::uint64_t iRandomTicket = 0u;
		bool isMaximumConsecutiveRelaxed = false;
		bool areCandidatesTruncated = false;
		std::vector<VALTAN_DECISION_TRACE_CANDIDATE_WIRE> Candidates;
	};

	struct C2S_VALTAN_DECISION_TRACE_QUERY
	{
		std::uint32_t iRequestSequence = 0u;
		std::string strBossPlacementId;
		std::uint64_t iAfterTraceSequence = 0u;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_VALTAN_DECISION_TRACE_QUERY& message);
	bool Read_Message(
		CPacketReader& reader,
		C2S_VALTAN_DECISION_TRACE_QUERY& message);

	enum class VALTAN_DECISION_TRACE_QUERY_RESULT : std::uint8_t
	{
		TRACE,
		UNCHANGED,
		REJECTED_RELEASE_BUILD,
		REJECTED_WRONG_WORLD,
		REJECTED_NO_BOSS,
		NO_TRACE,
		END
	};

	struct S2C_VALTAN_DECISION_TRACE_RESPONSE
	{
		std::uint32_t iRequestSequence = 0u;
		std::string strBossPlacementId;
		VALTAN_DECISION_TRACE_QUERY_RESULT eResult =
			VALTAN_DECISION_TRACE_QUERY_RESULT::NO_TRACE;
		// TRACE 응답에만 포함하며 해당 선택 판단이 사용한 불변 정의 세대를 가리킨다.
		GameplayDataRevision DefinitionRevision{};
		VALTAN_DECISION_TRACE_WIRE Trace{};
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_VALTAN_DECISION_TRACE_RESPONSE& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_VALTAN_DECISION_TRACE_RESPONSE& message);

	// presentation lane은 여러 비트를 결합하는 집합이다.
	// 알 수 없는 bit는 protocol 오류로 거절하여 구버전이 모르는 필수 lane을 준비 완료로 처리하지 못하게 한다.
	enum class GAMEPLAY_PRESENTATION_LANE : std::uint32_t
	{
		ANIMATION = 1u << 0u,
		EFFECT = 1u << 1u,
		COMBAT_VISUAL = 1u << 2u,
		CAMERA = 1u << 3u,
		WORLD_EVENT_SET = 1u << 4u
	};

	inline constexpr std::uint32_t GAMEPLAY_PRESENTATION_KNOWN_LANE_MASK =
		static_cast<std::uint32_t>(GAMEPLAY_PRESENTATION_LANE::ANIMATION) |
		static_cast<std::uint32_t>(GAMEPLAY_PRESENTATION_LANE::EFFECT) |
		static_cast<std::uint32_t>(GAMEPLAY_PRESENTATION_LANE::COMBAT_VISUAL) |
		static_cast<std::uint32_t>(GAMEPLAY_PRESENTATION_LANE::CAMERA) |
		static_cast<std::uint32_t>(GAMEPLAY_PRESENTATION_LANE::WORLD_EVENT_SET);

	inline constexpr std::size_t MAX_DATA_REVISION_REASON_BYTES = 256u;

	struct C2S_DATA_REVISION_PREPARE_REQUEST
	{
		std::uint32_t iTransactionSequence = 0;
		GameplayDataRevision BaseRevision{};
		GameplayDataRevision CandidateRevision{};
		std::uint32_t iRequiredPresentationLaneMask = 0;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_DATA_REVISION_PREPARE_REQUEST& message);
	bool Read_Message(
		CPacketReader& reader,
		C2S_DATA_REVISION_PREPARE_REQUEST& message);

	struct S2C_DATA_REVISION_PREPARE
	{
		std::uint32_t iTransactionSequence = 0;
		GameplayDataRevision BaseRevision{};
		GameplayDataRevision CandidateRevision{};
		std::uint32_t iRequiredPresentationLaneMask = 0;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_DATA_REVISION_PREPARE& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_DATA_REVISION_PREPARE& message);

	enum class DATA_REVISION_PREPARE_STATUS : std::uint8_t
	{
		READY,
		READY_DEGRADED,
		NACK,
		END
	};

	struct C2S_DATA_REVISION_PREPARE_RESPONSE
	{
		std::uint32_t iTransactionSequence = 0;
		GameplayDataRevision CandidateRevision{};
		DATA_REVISION_PREPARE_STATUS eStatus =
			DATA_REVISION_PREPARE_STATUS::NACK;
		std::uint32_t iRequiredPresentationLaneMask = 0;
		std::uint32_t iPreparedPresentationLaneMask = 0;
		std::uint32_t iFailedPresentationLaneMask = 0;
		std::string strReason;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_DATA_REVISION_PREPARE_RESPONSE& message);
	bool Read_Message(
		CPacketReader& reader,
		C2S_DATA_REVISION_PREPARE_RESPONSE& message);

	enum class DATA_REVISION_RESULT : std::uint8_t
	{
		COMMITTED,
		ABORTED,
		END
	};

	struct S2C_DATA_REVISION_RESULT
	{
		std::uint32_t iTransactionSequence = 0;
		GameplayDataRevision CandidateRevision{};
		GameplayDataRevision ActiveRevision{};
		DATA_REVISION_RESULT eResult = DATA_REVISION_RESULT::ABORTED;
		std::string strReason;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_DATA_REVISION_RESULT& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_DATA_REVISION_RESULT& message);

	// itemId는 표시 문자열이 아니라 item catalog의 stable ID다. UI 이름과 구분해 전송 길이를 제한한다.
	inline constexpr std::size_t MAX_ITEM_ID_BYTES = 64;
	// 현재 inventory snapshot·상점·캐릭터 복원에서도 공유하는 항목 수 상한이다.
	// Debug 아이템 지급만의 제한이나 화면의 칸 수로 해석하지 않는다.
	inline constexpr std::size_t MAX_INVENTORY_ITEMS = 64;

	struct C2S_DEBUG_GIVE_ITEM
	{
		std::uint32_t iRequestSequence = 0;
		std::string strItemId;
		std::uint32_t iQuantity = 1;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_DEBUG_GIVE_ITEM& message);
	bool Read_Message(
		CPacketReader& reader,
		C2S_DEBUG_GIVE_ITEM& message);

	// 캐릭터 장착 슬롯이다. 기본 장비·장신구 13종 외에 아바타 2종을 포함하며 NONE은 가방 안의 미장착 항목이다.
	enum class EQUIPMENT_SLOT : std::uint8_t
	{
		NONE = 0,
		HELMET,
		SHOULDER,
		TOP,
		PANTS,
		GLOVES,
		WEAPON,
		NECKLACE,
		EARRING1,
		EARRING2,
		RING1,
		RING2,
		STONE,
		BRACELET,
		AVATAR_HEAD,
		AVATAR_OUTFIT,
		END
	};

	// 슬롯을 Data/Items/ItemCatalog.json의 equipSlot 문자열로 바꾼다.
	// 귀걸이 두 칸은 earring, 반지 두 칸은 ring을 공유한다. NONE·END·미정의 값은 nullptr다.
	// Server의 장착·복원 검증과 Client 장착 표시가 같은 변환을 사용한다.
	[[nodiscard]]
	constexpr const char* Equipment_SlotKind(const EQUIPMENT_SLOT slot)
	{
		switch (slot)
		{
		case EQUIPMENT_SLOT::HELMET:
			return "helmet";
		case EQUIPMENT_SLOT::SHOULDER:
			return "shoulder";
		case EQUIPMENT_SLOT::TOP:
			return "top";
		case EQUIPMENT_SLOT::PANTS:
			return "pants";
		case EQUIPMENT_SLOT::GLOVES:
			return "gloves";
		case EQUIPMENT_SLOT::WEAPON:
			return "weapon";
		case EQUIPMENT_SLOT::NECKLACE:
			return "necklace";
		case EQUIPMENT_SLOT::EARRING1:
		case EQUIPMENT_SLOT::EARRING2:
			return "earring";
		case EQUIPMENT_SLOT::RING1:
		case EQUIPMENT_SLOT::RING2:
			return "ring";
		case EQUIPMENT_SLOT::STONE:
			return "stone";
		case EQUIPMENT_SLOT::BRACELET:
			return "bracelet";
		case EQUIPMENT_SLOT::AVATAR_HEAD:
			return "avatarHead";
		case EQUIPMENT_SLOT::AVATAR_OUTFIT:
			return "avatarOutfit";
		default:
			return nullptr;
		}
	}

	inline constexpr std::uint16_t INITIAL_EQUIPMENT_UPGRADE_LEVEL = 10u;
	inline constexpr std::uint16_t MAX_EQUIPMENT_UPGRADE_LEVEL = 65535u;
	// Server 지급·강화·복원과 Client 표시가 공유하는 강화 대상 분류다.
	// ID 접두·접미와 equipSlot의 조합만 검사하며 실제 소유·거리·재화 조건은 Server 호출자가 검증한다.
	[[nodiscard]] inline bool Is_Upgradeable_Equipment(
		const std::string_view itemId, const std::string_view equipSlot)
	{
		if (!itemId.starts_with("EQUIP_"))
			return false;
		return (equipSlot == "weapon" && itemId.ends_with("_HONORWHISPER_WEAPON")) ||
			(equipSlot == "helmet" && itemId.ends_with("_HONORWHISPER_HELMET")) ||
			(equipSlot == "shoulder" && itemId.ends_with("_HONORWHISPER_SHOULDER")) ||
			(equipSlot == "top" && itemId.ends_with("_HONORWHISPER_TOP")) ||
			(equipSlot == "pants" && itemId.ends_with("_HONORWHISPER_PANTS")) ||
			(equipSlot == "gloves" && itemId.ends_with("_HONORWHISPER_GLOVES"));
	}

	// 인벤토리 한 항목이다. 장착해도 같은 목록에 남고 eEquippedSlot만 장착 위치를 나타낸다.
	// 따라서 snapshot·world transfer·캐릭터 복원이 장비도 함께 운반한다.
	// (itemId, slot) 쌍은 중복할 수 없고 실제 장착 슬롯 하나에는 한 항목만 들어간다.
	struct INVENTORY_ITEM_SNAPSHOT
	{
		std::string strItemId;
		std::uint32_t iQuantity = 0;
		EQUIPMENT_SLOT eEquippedSlot = EQUIPMENT_SLOT::NONE;
		// 내구도는 0(파손)~100(정상)의 정수 백분율이다. 무기와 방어구만 닳으며 나머지는 100을 유지한다.
		// 인벤토리 항목과 함께 전송되어 장착·방 이동·복원에서도 같은 값으로 유지된다.
		std::uint8_t iDurabilityPercent = 100;
		// 강화 불가능한 항목은 0이며 Honor Whisper 장비의 초기 강화 단계는 10이다.
		std::uint16_t iUpgradeLevel = 0u;
	};

	// 내구도가 적용되는 무기와 방어구 다섯 부위의 공통 조건이다.
	// Server 내구도 감소와 Client 내구도 HUD·수리 창이 같은 목록을 사용한다.
	[[nodiscard]]
	constexpr bool Is_Durable_Slot(const EQUIPMENT_SLOT slot) noexcept
	{
		return EQUIPMENT_SLOT::HELMET == slot || EQUIPMENT_SLOT::SHOULDER == slot ||
			EQUIPMENT_SLOT::TOP == slot || EQUIPMENT_SLOT::PANTS == slot ||
			EQUIPMENT_SLOT::GLOVES == slot || EQUIPMENT_SLOT::WEAPON == slot;
	}

	// 장비 한 개의 수리 비용이다. 내구도 0이면 1000실링, 정상인 100 이상이면 0이다.
	// 손상 비율에 비례한 정수 나눗셈에 +99를 더해 올림하며 Server 청구와 수리 창 표시가 같은 함수를 사용한다.
	[[nodiscard]]
	constexpr std::uint32_t Repair_Silver_Cost(const std::uint8_t durabilityPercent) noexcept
	{
		return durabilityPercent >= 100u ? 0u : (1000u * (100u - durabilityPercent) + 99u) / 100u;
	}

	// 현재 인벤토리 전체를 교체하는 snapshot이다.
	// 이전 지급 사건을 모두 재생하지 않아도 재진입·갱신 때 현재 항목과 장착 상태를 맞출 수 있다.
	struct S2C_INVENTORY_SNAPSHOT
	{
		std::uint32_t iRequestSequence = 0;
		std::vector<INVENTORY_ITEM_SNAPSHOT> Items;
		// 실링과 골드는 가방 아이템이 아니라 별도 재화이므로 항목 목록과 나란히 보낸다.
		std::uint32_t iSilver = 0;
		std::uint32_t iGold = 0;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_INVENTORY_SNAPSHOT& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_INVENTORY_SNAPSHOT& message);

	// Item_1~4 중 어느 quick slot에서 사용했는지는 Client 표시·바인딩 책임이다.
	// Server는 item ID로 인벤토리를 확인하므로 이 요청에는 quick slot index가 없다.
	struct C2S_USE_ITEM
	{
		std::uint32_t iRequestSequence = 0;
		std::string strItemId;
		// 목표 지점에 대한 요청일 뿐이다. 실제 사거리와 목표 표면은 Server가 검증한다.
		bool hasGroundTarget = false;
		float fTargetX = 0.f, fTargetZ = 0.f;
		NET_ENTITY_ID iTargetPlayerNetEntityId = INVALID_NET_ENTITY_ID;
	};

	// 장착이면 가방의 strItemId 한 개를 eSlot으로 옮기고 기존 장비를 가방으로 돌려보낸다.
	// 해제이면 해당 슬롯의 항목을 가방으로 옮기며 item ID는 비워 둔다.
	// Server가 장착 종류·직업을 검사하며 처리가 가능한 session에는 현재 인벤토리 snapshot을 회신한다.
	struct C2S_SET_EQUIPMENT
	{
		std::uint32_t iRequestSequence = 0;
		EQUIPMENT_SLOT eSlot = EQUIPMENT_SLOT::NONE;
		bool bEquip = false;
		std::string strItemId;
	};
	bool Write_Message(CPacketWriter& writer, const C2S_SET_EQUIPMENT& message);
	bool Read_Message(CPacketReader& reader, C2S_SET_EQUIPMENT& message);

	// bAllSlots=true는 가방을 포함해 내구도가 100 미만인 모든 항목, false는 장착한 무기·방어구만 수리한다.
	// 전체 수리는 catalog 종류를 다시 검사하지 않고 항목의 내구도로 대상을 고른다.
	// Server는 전체 대상의 비용을 먼저 합산하고 잔액이 충분할 때만 차감·수리하므로 일부만 결제하지 않는다.
	struct C2S_REPAIR_EQUIPMENT
	{
		std::uint32_t iRequestSequence = 0;
		bool bAllSlots = false;
	};
	bool Write_Message(CPacketWriter& writer, const C2S_REPAIR_EQUIPMENT& message);
	bool Read_Message(CPacketReader& reader, C2S_REPAIR_EQUIPMENT& message);

	// NPC 상점에서 한 번에 구매할 장바구니다. 최대 10행이며 각 행은 catalog item ID와 수량을 담는다.
	inline constexpr std::size_t MAX_SHOP_BASKET_ENTRIES = 10;
	inline constexpr std::uint32_t MAX_SHOP_BASKET_QUANTITY = 999;
	struct SHOP_BASKET_ENTRY
	{
		std::string strItemId;
		std::uint32_t iQuantity = 0;
	};
	struct C2S_BUY_ITEMS
	{
		std::uint32_t iRequestSequence = 0;
		std::string strNpcPlacementId;
		std::vector<SHOP_BASKET_ENTRY> Entries;
	};
	bool Write_Message(CPacketWriter& writer, const C2S_BUY_ITEMS& message);
	bool Read_Message(CPacketReader& reader, C2S_BUY_ITEMS& message);

	// Client의 process-session 로스터에 보관한 인벤토리·재화·칭호를 Bern 진입 초기에 한 번 복원한다.
	// 장착 항목은 원래 슬롯을 유지한다. Server는 허용 기회를 먼저 소비하므로 검증 실패 후 변형 재요청도 허용하지 않는다.
	// 승인하면 인벤토리 snapshot과 복원 결과를 이 순서로 전송할 준비를 마친 뒤 상태를 적용한다. 거절은 명시적 결과로 알린다.
	inline constexpr std::uint32_t MAX_RESTORE_PURSE_AMOUNT = (std::numeric_limits<std::uint32_t>::max)();
	struct C2S_RESTORE_CHARACTER
	{
		std::uint32_t iRequestSequence = 0;
		std::vector<INVENTORY_ITEM_SNAPSHOT> Items;
		std::uint32_t iSilver = 0;
		std::uint32_t iGold = 0;
		HONOR_TITLE_ID iHonorTitleId = INVALID_HONOR_TITLE_ID;
	};
	bool Write_Message(CPacketWriter& writer, const C2S_RESTORE_CHARACTER& message);
	bool Read_Message(CPacketReader& reader, C2S_RESTORE_CHARACTER& message);

	enum class CHARACTER_RESTORE_RESULT : std::uint8_t
	{
		APPLIED,
		REJECTED_SESSION,
		REJECTED_UNAVAILABLE,
		REJECTED_CATALOG,
		END
	};
	struct S2C_RESTORE_CHARACTER_RESULT
	{
		std::uint32_t iRequestSequence = 0u;
		CHARACTER_RESTORE_RESULT eResult = CHARACTER_RESTORE_RESULT::REJECTED_SESSION;
		HONOR_TITLE_ID iHonorTitleId = INVALID_HONOR_TITLE_ID;
	};
	bool Write_Message(CPacketWriter& writer, const S2C_RESTORE_CHARACTER_RESULT& message);
	bool Read_Message(CPacketReader& reader, S2C_RESTORE_CHARACTER_RESULT& message);

	struct C2S_UPGRADE_EQUIPMENT
	{
		std::uint32_t iRequestSequence = 0u;
		std::string strItemId;
		EQUIPMENT_SLOT eSlot = EQUIPMENT_SLOT::NONE;
		std::uint16_t iExpectedUpgradeLevel = 0u;
	};
	enum class EQUIPMENT_UPGRADE_RESULT : std::uint8_t
	{
		SUCCEEDED,
		FAILED,
		REJECTED,
		END
	};
	struct S2C_UPGRADE_EQUIPMENT_RESULT
	{
		std::uint32_t iRequestSequence = 0u;
		EQUIPMENT_UPGRADE_RESULT eResult = EQUIPMENT_UPGRADE_RESULT::REJECTED;
		std::uint16_t iUpgradeLevel = 0u;
		WORLD_ID eWorldId = WORLD_ID::BERN;
	};
	bool Write_Message(CPacketWriter& writer, const C2S_UPGRADE_EQUIPMENT& message);
	bool Read_Message(CPacketReader& reader, C2S_UPGRADE_EQUIPMENT& message);
	bool Write_Message(CPacketWriter& writer, const S2C_UPGRADE_EQUIPMENT_RESULT& message);
	bool Read_Message(CPacketReader& reader, S2C_UPGRADE_EQUIPMENT_RESULT& message);

	// 퇴장 전에 앞서 접수한 방 명령이 반영된 상태를 회수하는 reliable 명령이다.
	// 응답의 world·player·entity 식별자를 Client가 확인한 뒤 자신의 로스터 슬롯에 보관한다.
	struct C2S_CAPTURE_CHARACTER
	{
		std::uint32_t iRequestSequence = 0u;
	};
	enum class CHARACTER_CAPTURE_RESULT : std::uint8_t
	{
		CAPTURED,
		REJECTED,
		END
	};
	struct S2C_CAPTURE_CHARACTER_RESULT
	{
		std::uint32_t iRequestSequence = 0u;
		CHARACTER_CAPTURE_RESULT eResult = CHARACTER_CAPTURE_RESULT::REJECTED;
		std::vector<INVENTORY_ITEM_SNAPSHOT> Items;
		std::uint32_t iSilver = 0u, iGold = 0u;
		HONOR_TITLE_ID iHonorTitleId = INVALID_HONOR_TITLE_ID;
		WORLD_ID eWorldId = WORLD_ID::BERN;
		PLAYER_ID iPlayerId = INVALID_PLAYER_ID;
		NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
		CHARACTER_CLASS_ID eCharacterClass = CHARACTER_CLASS_ID::END;
	};
	bool Write_Message(CPacketWriter& writer, const C2S_CAPTURE_CHARACTER& message);
	bool Read_Message(CPacketReader& reader, C2S_CAPTURE_CHARACTER& message);
	bool Write_Message(CPacketWriter& writer, const S2C_CAPTURE_CHARACTER_RESULT& message);
	bool Read_Message(CPacketReader& reader, S2C_CAPTURE_CHARACTER_RESULT& message);

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_USE_ITEM& message);
	bool Read_Message(
		CPacketReader& reader,
		C2S_USE_ITEM& message);

	// "되돌리기"는 제거할 entity 목록 대신 요청 sequence만 보낸다.
	// 이 명령은 요청 sequence만 보내고 제거할 객체의 선택은 Server가 한다.
	// 현재 소비자는 Character Select뿐 아니라 Valtan·Kouku 아레나도 각각의 허용 조건과 정리 경로로 처리한다.
	struct C2S_DESPAWN_ALL_WORLD_ENTITIES
	{
		std::uint32_t iRequestSequence = 0;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_DESPAWN_ALL_WORLD_ENTITIES& message);
	bool Read_Message(
		CPacketReader& reader,
		C2S_DESPAWN_ALL_WORLD_ENTITIES& message);

	// Gameplay.world.json NPC placement의 stable ID 길이 상한이다. 표시 이름이나 itemId와 혼용하지 않는다.
	inline constexpr std::size_t MAX_NPC_PLACEMENT_ID_BYTES = 64;

	// Bern 입장 확인 UI가 대화한 NPC placement ID를 보낸다.
	// Server는 플레이어가 아직 해당 NPC 근처인지 다시 확인하고 기존 world transfer 경로로 입장을 처리한다.
	struct C2S_CONFIRM_NPC_ENTRY
	{
		std::uint32_t iRequestSequence = 0;
		std::string strNpcPlacementId;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_CONFIRM_NPC_ENTRY& message);
	bool Read_Message(
		CPacketReader& reader,
		C2S_CONFIRM_NPC_ENTRY& message);

	// Bern 콜로세움 참가·이탈 요청이다. JOIN은 NPC 제안 수락 후 보내며 거절 UI는 요청을 보내지 않는다.
	// Server는 NPC 근접 조건·최대 인간 4명·첫 대기자 기준 10초 기한과 팀 배정을 소유한다.
	// LEAVE는 요청한 플레이어만 대기열에서 뺀다. Client 제안 창의 대기 시간과 Server 모집 기한은 서로 다르다.
	struct C2S_COLOSSEUM_QUEUE_JOIN
	{
		std::uint32_t iRequestSequence = 0;
		std::string strNpcPlacementId;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_COLOSSEUM_QUEUE_JOIN& message);
	bool Read_Message(
		CPacketReader& reader,
		C2S_COLOSSEUM_QUEUE_JOIN& message);

	struct C2S_COLOSSEUM_QUEUE_LEAVE
	{
		std::uint32_t iRequestSequence = 0;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_COLOSSEUM_QUEUE_LEAVE& message);
	bool Read_Message(
		CPacketReader& reader,
		C2S_COLOSSEUM_QUEUE_LEAVE& message);

	// WAITING은 참가 승인 또는 인원 변경, LEFT는 이탈 적용, REJECTED는 참가 거절이다.
	// Client는 거절 응답에서 대기 창을 닫는다.
	enum class COLOSSEUM_QUEUE_STATE : std::uint8_t
	{
		WAITING = 0,
		LEFT,
		REJECTED,
		END
	};

	struct S2C_COLOSSEUM_QUEUE_STATE
	{
		COLOSSEUM_QUEUE_STATE eState = COLOSSEUM_QUEUE_STATE::END;
		std::uint8_t iQueuedCount = 0;
		std::uint8_t iRequiredCount = 0;
		std::uint32_t iServerTick = 0u, iDeadlineTick = 0u;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_COLOSSEUM_QUEUE_STATE& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_COLOSSEUM_QUEUE_STATE& message);

	inline constexpr std::size_t MAX_COLOSSEUM_MATCH_PLAYERS = 4;

	struct COLOSSEUM_MATCH_PARTICIPANT
	{
		std::string strNickname;
		CHARACTER_CLASS_ID eCharacterClass = CHARACTER_CLASS_ID::END;
		// Server가 정한 팀 번호로 0은 왼쪽 팀, 1은 오른쪽 팀이다.
		std::uint8_t iTeam = 0;
	};

	// Server가 확정해 world transfer 직전에 각 대기 session으로 보내는 로딩용 명단이다.
	// iLocalIndex가 수신자 본인의 행을 가리키며 nickname은 조회 키로 쓰지 않는다.
	struct S2C_COLOSSEUM_MATCH_FOUND
	{
		std::uint64_t iMatchId = 0u;
		std::uint8_t iLocalIndex = 0;
		std::vector<COLOSSEUM_MATCH_PARTICIPANT> Participants;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_COLOSSEUM_MATCH_FOUND& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_COLOSSEUM_MATCH_FOUND& message);

	// 방 시간은 30Hz Server tick이다. 모집 후보도 계속 복제하지만 선택된 참가자만 각 팀의 전투 자리 4개를 차지한다.
	enum class COLOSSEUM_MATCH_PHASE : std::uint8_t
	{
		LOADING,
		RECRUITING,
		ENTRY_COUNTDOWN,
		INTRO,
		COUNTDOWN,
		ACTIVE,
		PLAYING = ACTIVE,
		FINISHED,
		END
	};
	inline constexpr std::size_t MAX_COLOSSEUM_COMBAT_PLAYERS = 8;
	inline constexpr std::size_t MAX_COLOSSEUM_STATE_PLAYERS = 14;
	inline constexpr std::uint8_t COLOSSEUM_NO_TEAM = 255u;
	inline constexpr std::uint8_t COLOSSEUM_DRAW_TEAM = COLOSSEUM_NO_TEAM;
	struct COLOSSEUM_MATCH_PLAYER_STATE
	{
		NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
		std::uint8_t iTeam = COLOSSEUM_NO_TEAM;
		bool bParticipant = false;
		PLAYER_ID iPlayerId = INVALID_PLAYER_ID;
		// team + teamSlot * 2로 인간·용병의 선택 좌석을 같은 0~7 index에 배치한다.
		std::uint8_t iArrivalIndex = 0u;
		bool bReady = false;
		std::uint32_t iKills = 0u;
	};
	struct C2S_COLOSSEUM_RECRUIT
	{
		std::uint32_t iRequestSequence = 0;
		std::uint64_t iMatchId = 0;
		NET_ENTITY_ID iMercenaryNetEntityId = INVALID_NET_ENTITY_ID;
	};
	struct C2S_COLOSSEUM_LOAD_READY
	{
		std::uint64_t iMatchId = 0u;
	};
	struct C2S_COLOSSEUM_RETURN
	{
		std::uint64_t iMatchId = 0u;
	};
	inline constexpr std::size_t MAX_COLOSSEUM_RECENT_KILLS = 8u;
	struct COLOSSEUM_KILL_EVENT
	{
		std::uint32_t iSequence = 0u, iServerTick = 0u;
		PLAYER_ID iKillerId = INVALID_PLAYER_ID, iVictimId = INVALID_PLAYER_ID;
		std::uint8_t iKillerTeam = COLOSSEUM_DRAW_TEAM, iVictimTeam = COLOSSEUM_DRAW_TEAM;
		std::string strKillerNickname, strVictimNickname;
	};
	struct S2C_COLOSSEUM_MATCH_STATE
	{
		std::uint64_t iMatchId = 0;
		COLOSSEUM_MATCH_PHASE ePhase = COLOSSEUM_MATCH_PHASE::END;
		std::uint8_t iWinnerTeam = COLOSSEUM_NO_TEAM;
		std::uint32_t iRevision = 0;
		std::vector<COLOSSEUM_MATCH_PLAYER_STATE> Players;
		std::uint32_t iServerTick = 0u, iPhaseStartTick = 0u, iPhaseEndTick = 0u;
		std::uint32_t iLeftScore = 0u, iRightScore = 0u;
		std::uint8_t iWinningTeam = COLOSSEUM_DRAW_TEAM;
		std::uint8_t iExpectedPlayers = 0u;
		std::vector<COLOSSEUM_MATCH_PLAYER_STATE> Participants;
		std::vector<COLOSSEUM_KILL_EVENT> RecentKills;
	};
	bool Write_Message(CPacketWriter&, const C2S_COLOSSEUM_RECRUIT&);
	bool Read_Message(CPacketReader&, C2S_COLOSSEUM_RECRUIT&);
	bool Write_Message(CPacketWriter&, const C2S_COLOSSEUM_LOAD_READY&);
	bool Read_Message(CPacketReader&, C2S_COLOSSEUM_LOAD_READY&);
	bool Write_Message(CPacketWriter&, const C2S_COLOSSEUM_RETURN&);
	bool Read_Message(CPacketReader&, C2S_COLOSSEUM_RETURN&);
	bool Write_Message(CPacketWriter&, const S2C_COLOSSEUM_MATCH_STATE&);
	bool Read_Message(CPacketReader&, S2C_COLOSSEUM_MATCH_STATE&);

	// 상호작용 조건을 가진 trigger box 안의 session에만 표시하며 나가면 회수한다.
	// bAvailable=false는 Client 표시를 해제한다. 요청도 placement ID를 되돌려 다른 box의 오래된 안내와 섞이지 않게 한다.
	struct S2C_INTERACT_PROMPT
	{
		std::string strTriggerPlacementId;
		bool bAvailable = false;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_INTERACT_PROMPT& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_INTERACT_PROMPT& message);

	// 특정 box 안내가 없을 때 G 입력으로 현재 서 있는 G 전용 trigger들을 조회하는 예약 ID다.
	// 실제 placement ID는 [A-Za-z0-9_.-]만 사용하므로 @here와 충돌하지 않는다.
	// wire는 일반 ID 요청과 같지만 실제 처리 의미는 이를 지원하는 Server의 소비자가 결정한다.
	inline constexpr const char* INTERACT_TRIGGER_HERE_ID = "@here";

	// 플레이어가 상호작용 키를 눌렀다는 요청이다.
	// Server가 session·요청 순서·현재 trigger 위치와 실행 조건을 다시 확인하므로 Client가 동작 결과를 직접 확정하지 않는다.
	struct C2S_INTERACT_TRIGGER
	{
		std::uint32_t iRequestSequence = 0;
		std::string strTriggerPlacementId;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_INTERACT_TRIGGER& message);
	bool Read_Message(
		CPacketReader& reader,
		C2S_INTERACT_TRIGGER& message);

	// "돌아가기"는 대상 좌표 없이 Bern 복귀 요청 sequence만 보낸다.
	// 이 요청은 대상 좌표 없이 sequence만 보낸다. 복귀 허용 여부와 실제 Bern 이동은 Server의 현재 world·레이드 상태 검증에 따른다.
	struct C2S_RETURN_TO_BERN
	{
		std::uint32_t iRequestSequence = 0;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_RETURN_TO_BERN& message);
	bool Read_Message(
		CPacketReader& reader,
		C2S_RETURN_TO_BERN& message);

	// 같은 방에 있는 다른 플레이어를 NetEntityId로 초대한다.
	// Server는 자기 m_Players에서 현재 대상을 찾지 못하면 거절하며 방 밖 nickname 검색으로 대체하지 않는다.
	struct C2S_PARTY_INVITE
	{
		std::uint32_t iRequestSequence = 0;
		NET_ENTITY_ID iTargetNetEntityId = INVALID_NET_ENTITY_ID;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_PARTY_INVITE& message);
	bool Read_Message(
		CPacketReader& reader,
		C2S_PARTY_INVITE& message);

	// 초대받은 플레이어에게만 보낸다. strFromNickname은 표시용이며 응답의 상대 식별은 NetEntityId를 사용한다.
	struct S2C_PARTY_INVITE_RECEIVED
	{
		NET_ENTITY_ID iFromNetEntityId = INVALID_NET_ENTITY_ID;
		std::string strFromNickname;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_PARTY_INVITE_RECEIVED& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_PARTY_INVITE_RECEIVED& message);

	struct C2S_PARTY_INVITE_RESPOND
	{
		std::uint32_t iRequestSequence = 0;
		NET_ENTITY_ID iFromNetEntityId = INVALID_NET_ENTITY_ID;
		bool bAccepted = false;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_PARTY_INVITE_RESPOND& message);
	bool Read_Message(
		CPacketReader& reader,
		C2S_PARTY_INVITE_RESPOND& message);

	struct PARTY_ROSTER_MEMBER
	{
		NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
		std::string strNickname;
		CHARACTER_CLASS_ID eCharacterClass = CHARACTER_CLASS_ID::END;
		PLAYER_CONTROL_KIND eControlKind = PLAYER_CONTROL_KIND::HUMAN;
	};

	// 파티 구성 변경 때 현재 멤버 전원에게 전체 명단을 보낸다.
	// 이전 가입·탈퇴 사건을 재생하지 않고 이 snapshot으로 현재 구성을 교체한다.
	struct S2C_PARTY_ROSTER
	{
		std::vector<PARTY_ROSTER_MEMBER> Members;
		// 인간 자리는 최대 4명이다. optional 동행 필드는 wire에 남아 있지만 현재 Bern Guide는 인간 파티 멤버로 생성하지 않는다.
		std::optional<PARTY_ROSTER_MEMBER> GuideCompanion;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_PARTY_ROSTER& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_PARTY_ROSTER& message);

	// 같은 방에 위치 핑을 요청한다. Server가 발신 session·world·새 sequence·빈도와 navigation 표면을 검증한다.
	// 발신자를 포함한 방 참가자는 승인된 S2C_ROOM_PING의 위치로 표시한다.
	struct C2S_ROOM_PING
	{
		std::uint32_t iClientSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		float fPositionX = 0.f, fPositionY = 0.f, fPositionZ = 0.f;
	};
	struct S2C_ROOM_PING
	{
		WORLD_ID eWorldId = WORLD_ID::END;
		NET_ENTITY_ID iFromNetEntityId = INVALID_NET_ENTITY_ID;
		std::uint32_t iClientSequence = 0u;
		float fPositionX = 0.f, fPositionY = 0.f, fPositionZ = 0.f;
	};
	bool Write_Message(CPacketWriter& writer, const C2S_ROOM_PING& message);
	bool Read_Message(CPacketReader& reader, C2S_ROOM_PING& message);
	bool Write_Message(CPacketWriter& writer, const S2C_ROOM_PING& message);
	bool Read_Message(CPacketReader& reader, S2C_ROOM_PING& message);

	// 방 채팅의 본문 요청이다. Server는 session에서 발신 entity·nickname을 정해 S2C_CHAT으로 방에 배포한다.
	// 핑의 좌표·빈도 검증과 채팅 본문 검증은 서로 다른 메시지 경로다.
	struct C2S_CHAT
	{
		std::string strText;
	};
	bool Write_Message(
		CPacketWriter& writer,
		const C2S_CHAT& message);
	bool Read_Message(
		CPacketReader& reader,
		C2S_CHAT& message);

	struct S2C_CHAT
	{
		NET_ENTITY_ID iFromNetEntityId = INVALID_NET_ENTITY_ID;
		std::string strFromNickname;
		std::string strText;
	};
	bool Write_Message(
		CPacketWriter& writer,
		const S2C_CHAT& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_CHAT& message);

    enum class GUIDE_CONTROL_ACTION : std::uint8_t
    {
        START,
        STOP,
        END
    };
    struct C2S_GUIDE_CONTROL
    {
        std::uint32_t iRequestSequence = 0u;
        NET_ENTITY_ID iGuideNetEntityId = INVALID_NET_ENTITY_ID;
        GUIDE_CONTROL_ACTION eAction = GUIDE_CONTROL_ACTION::START;
    };
    bool Write_Message(CPacketWriter& writer, const C2S_GUIDE_CONTROL& message);
    bool Read_Message(CPacketReader& reader, C2S_GUIDE_CONTROL& message);

	// 저작된 안내 대사는 Server 사건이다. 플레이어 채팅과 별도의 텍스트 byte 상한을 사용한다.
	inline constexpr std::size_t MAX_GUIDE_PROMPT_TEXT_BYTES = 512u;
	struct S2C_GUIDE_PROMPT
	{
		NET_ENTITY_ID iGuideNetEntityId = INVALID_NET_ENTITY_ID;
		std::uint32_t iEventSequence = 0u;
		std::uint64_t iRevision = 0u;
		std::string strPromptId;
		std::string strText;
		std::uint32_t iDurationMs = 5000u;
	};
	// Server의 최근 판단을 설명하는 읽기 전용 상태다. 이 메시지를 받았다고 Client가 AI 명령을 실행하지 않는다.
	struct S2C_GUIDE_STATE
	{
		NET_ENTITY_ID iGuideNetEntityId = INVALID_NET_ENTITY_ID;
		NET_ENTITY_ID iOwnerNetEntityId = INVALID_NET_ENTITY_ID;
		std::uint64_t iRevision = 0u;
		std::uint32_t iServerTick = 0u;
		std::uint8_t iContext = 0u, iAction = 0u;
		float fFollowScore = 0.f, fEvadeScore = 0.f, fCombatScore = 0.f;
		float fThreat = 0.f, fAnchorDistance = 0.f, fHpRatio = 1.f;
		bool bSurvivalOverride = false;
		std::string strReason;
		std::string strComboId;
		std::uint32_t iComboStep = 0u;
	};
	bool Write_Message(CPacketWriter& writer, const S2C_GUIDE_PROMPT& message);
	bool Read_Message(CPacketReader& reader, S2C_GUIDE_PROMPT& message);
	bool Write_Message(CPacketWriter& writer, const S2C_GUIDE_STATE& message);
	bool Read_Message(CPacketReader& reader, S2C_GUIDE_STATE& message);

	enum class PARTY_TRANSFER_RESULT : std::uint8_t
	{
		REJECTED_NOT_LEADER = 1,
		REJECTED_ROOM_FULL,
		REJECTED_MEMBER_UNAVAILABLE,
		REJECTED_ADMISSION_FAILED,
		REJECTED_OUTBOUND_BUSY
	};

	// 파티 이동 실패만 전달한다. 실패하면 모든 멤버가 원래 world에 남는다.
	// 성공한 입장은 기존 S2C_ENTER_ACCEPTED가 확정하므로 여기에는 별도의 성공 결과를 만들지 않는다.
	struct S2C_PARTY_TRANSFER_RESULT
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eTargetWorldId = WORLD_ID::END;
		PARTY_TRANSFER_RESULT eResult = PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED;
	};
	bool Write_Message(CPacketWriter& writer, const S2C_PARTY_TRANSFER_RESULT& message);
	bool Read_Message(CPacketReader& reader, S2C_PARTY_TRANSFER_RESULT& message);

	// ---- 파티 레이드 입장 전원 수락 투표 ----
	// 입장은 전부 이 투표로 통일된다: 리더/솔로가 발의 -> Server가 파티 전원(솔로는 본인 1명)에게
	// 프롬프트 -> 전원 수락 -> Server가 기존 SERVER_WORLD_TRANSFER_REQUEST batch 경로로 전송.
	// 한 명이라도 거절/타임아웃이면 전원 원래 world 유지. 성공 authority는 S2C_ENTER_ACCEPTED뿐이다.

	// 발의 시 고를 목표 레이드. NPC placement가 아니라 UI 탭이 소유하고, Server가 이 값으로
	// target WORLD_ID를 결정한다(Client가 world를 직접 지정하지 않는다).
	enum class RAID_ENTRY_TARGET : std::uint8_t
	{
		VALTAN = 0,
		KOUKUSAYDON,
		MAHARAKA,
		MAHARAKA_RETURN,
		END
	};

	// 투표 종료 사유. Server가 확정해 S2C_RAID_ENTRY_VOTE(bClosed=true)로 통지한다.
	enum class RAID_ENTRY_VOTE_RESULT : std::uint8_t
	{
		ALL_ACCEPTED = 0,
		DECLINED,
		TIMEOUT,
		CANCELLED,
		END
	};

	// 파티장(또는 솔로)이 입장 UI에서 입장하기를 눌러 투표를 발의한다. 즉시 전송하지 않고
	// 전원 투표를 연다. strNpcPlacementId로 near-NPC를 Server가 재검증하고, eTarget으로 목표를 고른다.
	struct C2S_RAID_ENTRY_PROPOSE
	{
		std::uint32_t iRequestSequence = 0u;
		std::string strNpcPlacementId;
		RAID_ENTRY_TARGET eTarget = RAID_ENTRY_TARGET::VALTAN;
	};
	bool Write_Message(CPacketWriter& writer, const C2S_RAID_ENTRY_PROPOSE& message);
	bool Read_Message(CPacketReader& reader, C2S_RAID_ENTRY_PROPOSE& message);

	// Server가 투표 대상 전원(발의자 포함)에게 보낸다. iProposalId는 방 로컬 단조 증가 식별자로,
	// 응답이 이 값을 되돌려 stale 응답을 걸러낸다. strProposerNickname은 표시 전용이다.
	struct S2C_RAID_ENTRY_PROMPT
	{
		std::uint32_t iProposalId = 0u;
		NET_ENTITY_ID iProposerNetEntityId = INVALID_NET_ENTITY_ID;
		RAID_ENTRY_TARGET eTarget = RAID_ENTRY_TARGET::VALTAN;
		std::string strProposerNickname;
	};
	bool Write_Message(CPacketWriter& writer, const S2C_RAID_ENTRY_PROMPT& message);
	bool Read_Message(CPacketReader& reader, S2C_RAID_ENTRY_PROMPT& message);

	// 각 대상이 수락/거절로 응답한다. iProposalId는 프롬프트가 준 값 그대로 되돌린다.
	struct C2S_RAID_ENTRY_RESPOND
	{
		std::uint32_t iRequestSequence = 0u;
		std::uint32_t iProposalId = 0u;
		bool bAccepted = false;
	};
	bool Write_Message(CPacketWriter& writer, const C2S_RAID_ENTRY_RESPOND& message);
	bool Read_Message(CPacketReader& reader, C2S_RAID_ENTRY_RESPOND& message);

	// 진행/종료 통지. 대상 전원에게 보낸다. bClosed=false면 진행 중(eResult=END),
	// bClosed=true면 eResult가 확정 결과이며 ALL_ACCEPTED면 이어서 S2C_ENTER_ACCEPTED가 온다.
	struct S2C_RAID_ENTRY_VOTE
	{
		std::uint32_t iProposalId = 0u;
		std::uint8_t iAccepted = 0u;
		std::uint8_t iTotal = 0u;
		bool bClosed = false;
		RAID_ENTRY_VOTE_RESULT eResult = RAID_ENTRY_VOTE_RESULT::END;
	};
	bool Write_Message(CPacketWriter& writer, const S2C_RAID_ENTRY_VOTE& message);
	bool Read_Message(CPacketReader& reader, S2C_RAID_ENTRY_VOTE& message);

	// 쿠크 1~3관문 진행 투표다. Server가 마지막 primary boss의 사망으로 클리어를 기록한다.
	// 리더 또는 솔로가 이동을 제안하고 전원이 수락하면 현재 관문 정리·다음 관문 생성·플레이어 이동을 Server가 처리한다.
	// NONE은 열린 투표가 없다는 뜻이며 레이드 입장 투표와 별개의 현재 관문 상태를 운반한다.
	enum class GATE_PROGRESS_VOTE_RESULT : std::uint8_t
	{
		NONE = 0,
		ALL_ACCEPTED,
		DECLINED,
		TIMEOUT,
		CANCELLED,
		END
	};

	// ADVANCE는 클리어 뒤 다음 관문, RESTART는 현재 관문을 다시 준비한다.
	// ENTER_GATE3는 현재 관문과 별도로 3관문 도착 구역 진입을 요청하며 EXIT는 투표 통과 후 전원을 Bern으로 복귀시킨다.
	enum class GATE_PROGRESS_KIND : std::uint8_t
	{
		ADVANCE = 0,
		RESTART,
		ENTER_GATE3,
		EXIT,
		END
	};

	struct C2S_GATE_PROGRESS_PROPOSE
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		GATE_PROGRESS_KIND eKind = GATE_PROGRESS_KIND::ADVANCE;
	};
	bool Write_Message(CPacketWriter& writer, const C2S_GATE_PROGRESS_PROPOSE& message);
	bool Read_Message(CPacketReader& reader, C2S_GATE_PROGRESS_PROPOSE& message);

	struct C2S_GATE_PROGRESS_RESPOND
	{
		std::uint32_t iRequestSequence = 0u;
		std::uint32_t iProposalId = 0u;
		bool bAccepted = false;
	};
	bool Write_Message(CPacketWriter& writer, const C2S_GATE_PROGRESS_RESPOND& message);
	bool Read_Message(CPacketReader& reader, C2S_GATE_PROGRESS_RESPOND& message);

	// 상태가 바뀌면 방 참가자 전원에게 보낸다. iCurrentGate는 1-based이며 0은 아직 열린 관문이 없다는 뜻이다.
	// iClearedMask의 (gate-1) bit가 해당 관문의 클리어 여부다.
	// iProposalId=0은 투표 없음, bClosed와 eResult는 종료 결과를 나타낸다.
	struct S2C_GATE_PROGRESS_STATE
	{
		WORLD_ID eWorldId = WORLD_ID::END;
		std::uint8_t iGateCount = 0u;
		std::uint8_t iCurrentGate = 0u;
		std::uint8_t iClearedMask = 0u;
		std::uint32_t iProposalId = 0u;
		GATE_PROGRESS_KIND eKind = GATE_PROGRESS_KIND::ADVANCE;
		NET_ENTITY_ID iProposerNetEntityId = INVALID_NET_ENTITY_ID;
		std::uint8_t iAccepted = 0u;
		std::uint8_t iTotal = 0u;
		bool bClosed = false;
		GATE_PROGRESS_VOTE_RESULT eResult = GATE_PROGRESS_VOTE_RESULT::NONE;
	};
	bool Write_Message(CPacketWriter& writer, const S2C_GATE_PROGRESS_STATE& message);
	bool Read_Message(CPacketReader& reader, S2C_GATE_PROGRESS_STATE& message);

	// 레이드 클리어 시 Server가 기록한 참가자별 전투 기여를 Client에 전달한다.
	// 피해·무력화·카운터와 생존 tick, 부위 파괴·마지막 타격·피격·최저 체력·클리어 생존·카운터 간격을 포함한다.
	// Client는 이 수치로 MVP·표시 칭호·메달을 선정하며 전투 실적 자체를 만들어 내지 않는다.
	// 배열 상한 8은 이 메시지의 참가자 수 제한이며 인간 파티 자리 수와 같은 개념은 아니다.
	inline constexpr std::size_t MAX_RAID_MVP_PARTICIPANTS = 8u;
	struct RAID_MVP_PARTICIPANT
	{
		PLAYER_ID iPlayerId = INVALID_PLAYER_ID;
		NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
		CHARACTER_CLASS_ID eCharacterClass = CHARACTER_CLASS_ID::END;
		std::string strNickname;
		std::uint64_t iDamage = 0u;
		std::uint64_t iStagger = 0u;
		std::uint32_t iCounterCount = 0u;
		std::uint32_t iAliveTicks = 0u;
		std::uint32_t iFightTicks = 0u;
		std::uint64_t iPartDamage = 0u;
		std::uint32_t iFinishingBlows = 0u;
		std::uint32_t iDamagingHitsTaken = 0u;
		// 전투 중 최저 체력을 최대 체력 대비 0~1000의 천분율로 나타낸다.
		std::uint16_t iLowestHpPermille = 1000u;
		bool bAliveAtClear = false;
		// 연속한 두 카운터 사이의 최소 간격(ms)이다. 카운터가 두 번 미만이면 0이다.
		std::uint32_t iMinCounterGapMs = 0u;
		// 전투 중 넘어진 횟수와 전투 시간(ms)이다.
		std::uint32_t iKnockdowns = 0u;
		std::uint32_t iFightMs = 0u;
	};
	struct S2C_RAID_MVP_RESULT
	{
		WORLD_ID eWorldId = WORLD_ID::END;
		// 클리어한 관문의 1-based 번호다. 발탄은 단일 관문 1을 사용한다.
		std::uint8_t iGate = 0u;
		std::vector<RAID_MVP_PARTICIPANT> Participants;
	};
	bool Write_Message(CPacketWriter& writer, const S2C_RAID_MVP_RESULT& message);
	bool Read_Message(CPacketReader& reader, S2C_RAID_MVP_RESULT& message);

	// 저작 world sequence의 실행·중지 제어다. Server가 trigger 진입과 소유 발생을 판정한다.
	// Client는 stable instance ID를 로드한 Area 문서에서 찾아 표현을 재생하며 전투 권위를 갖지 않는다.
	enum class WORLD_SEQUENCE_OPERATION : std::uint8_t
	{
		PLAY = 0,
		REPLAY = 1,
		STOP = 2,
		STOP_OWNER = 3,
		FINISH_OWNER = 4,
		STOP_CUE = 5,
		END
	};
	enum class DEBUG_WORLD_PLAYBACK_OPERATION : std::uint8_t
	{
		PLAY_TRIGGER,
		REPLAY_TRIGGER,
		PLAY_SEQUENCE,
		REPLAY_SEQUENCE,
		STOP_SEQUENCE,
		PLACE_ROOM_PLAYER,
		END
	};
	enum class DEBUG_WORLD_PLAYBACK_RESULT : std::uint8_t
	{
		ACCEPTED,
		DISABLED,
		WRONG_WORLD,
		INVALID_TARGET,
		INVALID_PLAYER,
		ALREADY_USED,
		ACTION_REJECTED,
		STALE_REQUEST,
		SKIPPED_PLAYER,
		END
	};
	struct C2S_DEBUG_WORLD_PLAYBACK
	{
		std::uint32_t iRequestSequence = 0;
		WORLD_ID eWorldId = WORLD_ID::END;
		DEBUG_WORLD_PLAYBACK_OPERATION eOperation = DEBUG_WORLD_PLAYBACK_OPERATION::END;
		std::string strTargetId;
		// PLACE_ROOM_PLAYER에서만 사용하는 고정 Sequence 실행과 발생 식별자다.
		std::uint32_t iRunEpoch = 0u;
		std::string strOccurrenceId;
		std::uint8_t iRoomPlayerSlot = 0u;
		float fPositionX = 0.f, fPositionY = 0.f, fPositionZ = 0.f;
	};
	struct S2C_DEBUG_WORLD_PLAYBACK_RESULT
	{
		std::uint32_t iRequestSequence = 0;
		WORLD_ID eWorldId = WORLD_ID::END;
		DEBUG_WORLD_PLAYBACK_OPERATION eOperation = DEBUG_WORLD_PLAYBACK_OPERATION::END;
		DEBUG_WORLD_PLAYBACK_RESULT eResult = DEBUG_WORLD_PLAYBACK_RESULT::DISABLED;
		std::string strTargetId;
	};
	bool Write_Message(CPacketWriter&, const C2S_DEBUG_WORLD_PLAYBACK&);
	bool Read_Message(CPacketReader&, C2S_DEBUG_WORLD_PLAYBACK&);
	bool Write_Message(CPacketWriter&, const S2C_DEBUG_WORLD_PLAYBACK_RESULT&);
	bool Read_Message(CPacketReader&, S2C_DEBUG_WORLD_PLAYBACK_RESULT&);
	struct S2C_WORLD_SEQUENCE_PLAY
	{
		WORLD_SEQUENCE_OPERATION eOperation = WORLD_SEQUENCE_OPERATION::PLAY;
		std::string strSequenceInstanceId;
		// 저작된 instance 재생 속도에 곱하는 배율이다. 1이면 원래 속도이며 패턴에서 회전·낙하 연출의 속도를 조절할 수 있다.
		float fPlaybackSpeed = 1.f;
		float fPositionOffsetX = 0.f;
		float fPositionOffsetY = 0.f;
		float fPositionOffsetZ = 0.f;
		std::uint32_t iDurationMs = 0u; // 0이면 저작 sequence의 수명을 사용한다.
		// bUntilDestroyed가 true인 Server 소유 combat object는 고정 종료 시간 대신 파괴·명시 취소로 종료한다.
		bool bUntilDestroyed = false;
		// 해당 cue가 소유하며 Server가 승인한 살아 있는 combat body일 때만 유효한 entity ID를 담는다.
		NET_ENTITY_ID iCombatBodyNetEntityId = INVALID_NET_ENTITY_ID;
		// 비어 있으면 새 sequence를 시작한다. 값이 있으면 그 기존 instance에 motion을 적용한다.
		std::string strTargetSequenceInstanceId;
		std::uint32_t iRunEpoch = 0u; // 0은 Debug 재생에 소속되지 않은 일반 map sequence다.
		std::string strMemberId;
		std::string strCueId;
		std::string strOccurrenceId;
		std::string strTargetCueId;
		std::uint32_t iStartTick = 0u;
		std::uint32_t iServerTick = 0u;
		NET_ENTITY_ID iBossNetEntityId = INVALID_NET_ENTITY_ID;
		std::uint32_t iPatternSequence = 0u;
		// 소유 WORLD box는 저장 instance의 배치를 덮어쓸 수 있다.
		// motion은 대상 cue의 배치를 유지하므로 별도의 placement 변경을 요청하지 않는다.
		bool bHasPlacement = false;
		float fWorldPositionX = 0.f, fWorldPositionY = 0.f, fWorldPositionZ = 0.f;
		float fWorldRotationXDegrees = 0.f, fWorldRotationYDegrees = 0.f, fWorldRotationZDegrees = 0.f;
		float fWorldScaleX = 1.f, fWorldScaleY = 1.f, fWorldScaleZ = 1.f;
	};
	bool Write_Message(CPacketWriter& writer, const S2C_WORLD_SEQUENCE_PLAY& message);
	bool Read_Message(CPacketReader& reader, S2C_WORLD_SEQUENCE_PLAY& message);

	// 쿠크 Boss Tool은 발탄과 별도 명령 종류를 사용한다.
	// 모든 결과에 scope를 돌려주므로 world·placement·archetype·정의 세대가 다른 응답을 현재 실행으로 채택하지 않는다.
	enum class KOUKUSAYDON_PATTERN_AUDITION_OPERATION : std::uint8_t
	{
		PLAY_SELECTED,
		PLAY_ALL,
		PLAY_BUNDLE,
		STOP,
		RESTART_BUNDLE,
		END
	};

	enum class KOUKUSAYDON_PATTERN_AUDITION_RESULT : std::uint8_t
	{
		QUEUED,
		DUPLICATE_IGNORED,
		REJECTED_RELEASE_BUILD,
		REJECTED_SCOPE_MISMATCH,
		REJECTED_NO_BOSS,
		REJECTED_BOSS_DEAD,
		REJECTED_BUSY,
		REJECTED_UNKNOWN_PATTERN,
		REJECTED_NO_PRODUCT_SEQUENCE,
		REJECTED_UNSUPPORTED_PATTERN,
		REJECTED_REVISION_MISMATCH,
		REJECTED_SOURCE_REVISION_MISMATCH,
		REJECTED_STALE_REQUEST,
		STOPPED,
		END
	};

	enum class KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE : std::uint8_t
	{
		PENDING,
		ACTIVE,
		PATTERN_COMPLETED,
		COMPLETED,
		ABORTED,
		END
	};

	inline constexpr std::size_t
		MAX_KOUKUSAYDON_PATTERN_AUDITION_REASON_BYTES = 192u;

	inline constexpr std::size_t MAX_KOUKUSAYDON_DRAFT_BYTES = 16u * 1024u * 1024u;
	inline constexpr std::size_t MAX_KOUKUSAYDON_DRAFT_CHUNK_BYTES = 48u * 1024u;
	// 재생 요청 전에 크기 제한이 있는 draft를 연속 chunk로 올린다.
	// request sequence와 SHA-256 revision으로 실행을 정확한 메모리 draft에 연결한다.
	// Server는 offset이 누적 길이와 다르거나 요청·전체 크기·revision이 섞이면 해당 업로드를 폐기한다.
	struct C2S_DEBUG_KOUKUSAYDON_DRAFT_CHUNK final
	{
		std::uint32_t iRequestSequence = 0u;
		std::uint32_t iOffsetBytes = 0u;
		std::uint32_t iTotalBytes = 0u;
		GameplayDataRevision RowsRevision{};
		std::string strBytes;
	};
	bool Write_Message(CPacketWriter& writer, const C2S_DEBUG_KOUKUSAYDON_DRAFT_CHUNK& message);
	bool Read_Message(CPacketReader& reader, C2S_DEBUG_KOUKUSAYDON_DRAFT_CHUNK& message);

	struct KOUKUSAYDON_PATTERN_AUDITION_SCOPE final
	{
		WORLD_ID eWorldId = WORLD_ID::END;
		std::string strEncounterId;
		std::string strGateId;
		std::string strBossPlacementId;
		std::string strBossArchetypeId;
		GameplayDataRevision ExpectedGameplayRevision{};
		std::uint32_t iExpectedSourceRevision = 0u;
		// 게시 데이터 재생에서는 비어 있고 draft 결과·생명주기에는 해당 draft revision을 되돌려준다.
		GameplayDataRevision DraftRowsRevision{};
	};

	struct C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST final
	{
		std::uint32_t iRequestSequence = 0u;
		KOUKUSAYDON_PATTERN_AUDITION_OPERATION eOperation =
			KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
		KOUKUSAYDON_PATTERN_AUDITION_SCOPE Scope;
		// PLAY_SELECTED에서만 pattern ID가 필수다. PLAY_ALL 순서는 Server가 승인한 Product sequence에서 결정한다.
		std::string strPatternId;
		std::string strBundleId;
		std::uint32_t iExpectedRunEpoch = 0u;
		// 0은 현재 방 진행을 사용한다. 1~4는 선택 패턴 시험에서 명시한 Mario 시작 단계다.
		std::uint8_t iMarioTestStartStage = 0u;
		std::uint32_t iMarioTestSeed = 0u;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& message);
	bool Read_Message(
		CPacketReader& reader,
		C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& message);

	inline constexpr std::size_t MAX_KOUKUSAYDON_BUNDLE_MEMBERS = 4u;
	struct KOUKUSAYDON_BUNDLE_MEMBER_STATE final
	{
        // 유지되는 root 표현은 Server가 정한 Mario 입장 연출의 시작·유지 시간과 단계를 따른다.
        std::string strMarioEntryPatternId;
        std::uint32_t iMarioEntryStartTick = 0u;
        std::uint32_t iMarioEntryHoldMs = 0u;
        std::uint8_t iMarioEntryStage = 0u;
        float fMarioEntryX = 0.f, fMarioEntryY = 0.f, fMarioEntryZ = 0.f, fMarioEntryYawDegrees = 0.f;
		std::string strMemberId;
		NET_ENTITY_ID iBossNetEntityId = INVALID_NET_ENTITY_ID;
		std::string strPatternId;
		std::uint32_t iPatternSequence = 0u;
		std::uint32_t iStartTick = 0u;
		KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE eState = KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::PENDING;
	};
	// 방이 소유하는 지속 실행 상태다. 늦게 입장한 Client를 포함해 참가자 모두에게 보낸다.
	struct S2C_KOUKUSAYDON_BUNDLE_STATE final
	{
		WORLD_ID eWorldId = WORLD_ID::END;
		std::string strEncounterId;
		std::string strBundleId; // 기존 단일 actor 재생일 때는 비어 있다.
		std::uint32_t iRunEpoch = 0u;
		std::uint32_t iCommonStartTick = 0u;
		std::uint32_t iServerTick = 0u;
		KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE eState = KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::PENDING;
		GameplayDataRevision PinnedGameplayRevision{};
		std::uint32_t iPinnedSourceRevision = 0u;
		GameplayDataRevision DraftRowsRevision{};
		std::vector<KOUKUSAYDON_BUNDLE_MEMBER_STATE> Members;
	};
	bool Write_Message(CPacketWriter& writer, const S2C_KOUKUSAYDON_BUNDLE_STATE& message);
	bool Read_Message(CPacketReader& reader, S2C_KOUKUSAYDON_BUNDLE_STATE& message);

	struct S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT final
	{
		std::uint32_t iRequestSequence = 0u;
		KOUKUSAYDON_PATTERN_AUDITION_OPERATION eOperation =
			KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
		KOUKUSAYDON_PATTERN_AUDITION_SCOPE Scope;
		std::string strRequestedPatternId;
		std::uint32_t iExpectedRunEpoch = 0u;
		KOUKUSAYDON_PATTERN_AUDITION_RESULT eResult =
			KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_SCOPE_MISMATCH;
		std::uint32_t iRoomAuditionEpoch = 0u;
		std::string strBundleId;
		std::uint32_t iCommonStartTick = 0u;
		NET_ENTITY_ID iBossNetEntityId = INVALID_NET_ENTITY_ID;
		std::string strResolvedPatternId;
		std::uint32_t iPatternSequence = 0u;
		std::uint32_t iStageIndex = 0u;
		GameplayDataRevision PinnedGameplayRevision{};
		std::uint32_t iPinnedSourceRevision = 0u;
		std::string strReason;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT& message);

	struct S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE final
	{
		std::uint32_t iRequestSequence = 0u;
		KOUKUSAYDON_PATTERN_AUDITION_OPERATION eOperation =
			KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
		KOUKUSAYDON_PATTERN_AUDITION_SCOPE Scope;
		std::uint32_t iRoomAuditionEpoch = 0u;
		std::string strBundleId;
		std::uint32_t iCommonStartTick = 0u;
		NET_ENTITY_ID iBossNetEntityId = INVALID_NET_ENTITY_ID;
		std::string strPatternId;
		std::string strMemberId;
		std::uint32_t iPatternSequence = 0u;
		std::uint32_t iStageIndex = 0u;
		KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE eState =
			KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::PENDING;
		GameplayDataRevision PinnedGameplayRevision{};
		std::uint32_t iPinnedSourceRevision = 0u;
		std::string strReason;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE& message);
	// Complete Play의 진행 시간과 상태는 Server 방 하나가 소유한다. Client는 복제된 상태를 표현한다.
	enum class KOUKUSAYDON_RAID_OPERATION : std::uint8_t
	{
		START,
		STOP,
		READY,
		FAILED,
		END
	};
	enum class KOUKUSAYDON_RAID_PHASE : std::uint8_t
	{
		INACTIVE,
		PREPARING,
		CINEMATIC,
		COMBAT,
		WAIT_GATE,
		WAIT_MINIGAME,
		COMPLETE,
		ABORTED,
		WAIT_ENTRY,
		END
	};
	struct C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST final
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		KOUKUSAYDON_RAID_OPERATION eOperation = KOUKUSAYDON_RAID_OPERATION::START;
		GameplayDataRevision ExpectedGameplayRevision{};
		std::uint32_t iActionSourceRevision = 0u;
		std::uint32_t iSequenceSourceRevision = 0u;
		std::string strStartGateId;
		std::uint32_t iExpectedRunEpoch = 0u;
		std::string strReason; // FAILED 요청에서만 길이 제한이 있는 준비 실패 사유를 담는다.
	};
	struct S2C_KOUKUSAYDON_RAID_STATE final
	{
		PLAYER_ID iOwnerPlayerId = INVALID_PLAYER_ID;
		WORLD_ID eWorldId = WORLD_ID::END;
		std::uint32_t iRequestSequence = 0u;
		std::uint32_t iRunEpoch = 0u;
		KOUKUSAYDON_RAID_PHASE ePhase = KOUKUSAYDON_RAID_PHASE::INACTIVE;
		std::string strGateId;
		std::string strSequenceCompositionId;
		std::string strSequencePatternId;
		std::uint32_t iSequenceSourceRevision = 0u;
		std::uint32_t iStartTick = 0u, iEndTick = 0u, iServerTick = 0u;
		std::string strFlowEntryId;
		std::uint32_t iFlowEntryIndex = 0u;
		GameplayDataRevision PinnedGameplayRevision{};
		std::uint32_t iActionSourceRevision = 0u;
		std::string strReason;
		std::vector<PLAYER_ID> ParticipantPlayerIds; // START 때 확정한 참가자 명단이다. 이후 입장자는 관찰만 한다.
		std::uint8_t iReadyMask = 0u;
	};
	bool Write_Message(CPacketWriter&, const C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST&);
	bool Read_Message(CPacketReader&, C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST&);
	bool Write_Message(CPacketWriter&, const S2C_KOUKUSAYDON_RAID_STATE&);
	bool Read_Message(CPacketReader&, S2C_KOUKUSAYDON_RAID_STATE&);

    // Server가 소유하는 워터팡 AI 설정이다. Ticks는 30Hz tick 수, M은 미터, Ms는 밀리초다.
    // 이동 확률·공격성은 0~1의 무차원 값이며 전체 범위 검사는 Is_Valid_MaharakaAITuning이 맡는다.
    struct MAHARAKA_AI_TUNING final
    {
        std::uint32_t iRevision = 1u, iBotCount = 20u, iDecisionTicks = 9u;
        std::uint32_t iMoveRetargetTicks = 45u, iSkillIntervalTicks = 60u;
        float fTargetRangeM = 9.f, fMoveProbability = .8f, fAggression = .8f, fKnockbackRangeM = 6.f;
        std::uint32_t iKnockbackMs = 242u;
    };
    enum class MAHARAKA_AI_OPERATION : std::uint8_t
    {
        GET,
        APPLY,
        SAVE
    };
    enum class MAHARAKA_AI_RESULT : std::uint8_t
    {
        ACCEPTED,
        REVISION_CONFLICT,
        INVALID_VALUE,
        WRONG_WORLD,
        SAVE_FAILED
    };
    // GET은 현재 설정 조회, APPLY/SAVE는 iExpectedRevision과 현재 revision을 비교하는 변경 요청이다.
    // Client가 저장 파일을 직접 수정하는 메시지가 아니며 적용·저장의 실제 성공은 Server 응답으로 확인한다.
    struct C2S_MAHARAKA_AI_TUNING final
    {
        std::uint32_t iRequestSequence = 0u, iExpectedRevision = 0u;
        MAHARAKA_AI_OPERATION eOperation = MAHARAKA_AI_OPERATION::GET;
        MAHARAKA_AI_TUNING Tuning;
    };
    struct S2C_MAHARAKA_AI_TUNING final
    {
        std::uint32_t iRequestSequence = 0u;
        MAHARAKA_AI_RESULT eResult = MAHARAKA_AI_RESULT::ACCEPTED;
        MAHARAKA_AI_TUNING Tuning;
        std::string strStatus;
    };
    bool Is_Valid_MaharakaAITuning(const MAHARAKA_AI_TUNING&);
    bool Write_Message(CPacketWriter&, const C2S_MAHARAKA_AI_TUNING&);
    bool Read_Message(CPacketReader&, C2S_MAHARAKA_AI_TUNING&);
    bool Write_Message(CPacketWriter&, const S2C_MAHARAKA_AI_TUNING&);
    bool Read_Message(CPacketReader&, S2C_MAHARAKA_AI_TUNING&);

}

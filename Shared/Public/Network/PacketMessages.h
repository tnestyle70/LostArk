#pragma once

#include "Network/PacketType.h"
#include "NetworkIds.h"

#include <string>
#include <vector>
//character의 class와 nickname용 packet
namespace LostArk::Shared
{
	//harness가 직접 write u8, write string을 호출하기 때문에, client와 server가 같은 함수를 쓰도록,
	//shared로 옮긴다.

	class CPacketReader;
	class CPacketWriter;

	//Enter World
	struct C2S_ENTER_WORLD
	{
		std::uint16_t iProtocolVersion = NETWORK_PROTOCOL_VERSION;
		WORLD_ID eWorldId = WORLD_ID::BERN;

		CHARACTER_CLASS_ID eCharacterClass =
			CHARACTER_CLASS_ID::END;

		std::string strNickName;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_ENTER_WORLD& message);

	bool Read_Message(
		CPacketReader& reader,
		C2S_ENTER_WORLD& message);

	//Enter Accepted
	struct S2C_ENTER_ACCEPTED
	{
		std::uint16_t iProtocolVersion = NETWORK_PROTOCOL_VERSION;
		WORLD_ID eWorldId = WORLD_ID::BERN;

		PLAYER_ID iPlayerId =
			INVALID_PLAYER_ID;

		NET_ENTITY_ID iNetEntityId =
			INVALID_NET_ENTITY_ID;
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

	//Player Spawn
	struct S2C_PLAYER_SPAWNED
	{
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

	struct S2C_WORLD_ENTITY_SPAWNED
	{
		NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
		WORLD_ENTITY_KIND eKind = WORLD_ENTITY_KIND::END;
		std::string strArchetypeId;
		std::string strEncounterId;
		std::string strPlacementId;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;
		// Server-authoritative XZ combat body radius. NPC presentations have no
		// combat body and therefore publish zero.
		float fCollisionRadius = 0.f;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_WORLD_ENTITY_SPAWNED& spawned);
	bool Read_Message(
		CPacketReader& reader,
		S2C_WORLD_ENTITY_SPAWNED& spawned);

	struct S2C_WORLD_ENTITY_DESPAWNED
	{
		NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_WORLD_ENTITY_DESPAWNED& despawned);
	bool Read_Message(
		CPacketReader& reader,
		S2C_WORLD_ENTITY_DESPAWNED& despawned);

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
	// Lance Master alone authors nine ACTIVE skills and can hold all nine on
	// cooldown at once, so eight silently dropped one tile from the HUD.
	inline constexpr std::size_t MAX_PLAYER_COOLDOWNS = 16;
	// One tick applies at most one player hit and one boss hit per actor, so this
	// bounds a 30 Hz frame rather than a fight.
	inline constexpr std::size_t MAX_DAMAGE_EVENTS = 64;
	// Matches the publisher's 2..8 comboStages bound.
	inline constexpr std::uint8_t MAX_COMBO_STAGES = 8;
	using SKILL_ID = std::uint32_t;
	inline constexpr SKILL_ID INVALID_SKILL_ID = 0;
	//이게 플레이어의 스킬을 의미하는 건가?
	enum class PLAYER_LOCOMOTION_STATE : std::uint8_t
	{
		IDLE,
		MOVING,
		END
	};
	//client->server move
	struct C2S_MOVE
	{
		std::uint32_t iClientSequence = 0;

		float fGoalX = 0.f;
		float fGoalZ = 0.f;
	};
	//근데 read가 const가 붙어야 하는 거 아닌가?
	bool Write_Message(
		CPacketWriter& writer,
		const C2S_MOVE& message);

	bool Read_Message(
		CPacketReader& reader,
		C2S_MOVE& message);

	// Client intent contains no player or entity ID. The server resolves the
	// actor from the authenticated session that owns this command.
	struct C2S_USE_SKILL
	{
		std::uint32_t iClientSequence = 0;
		SKILL_ID iSkillId = INVALID_SKILL_ID;
		float fAimX = 0.f;
		float fAimZ = 0.f;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_USE_SKILL& message);

	bool Read_Message(
		CPacketReader& reader,
		C2S_USE_SKILL& message);

	// A HOLD skill leaves its loop when the player lets the key go. The server
	// still owns when the action ends; this only reports the input edge.
	struct C2S_RELEASE_SKILL
	{
		std::uint32_t iClientSequence = 0;
		SKILL_ID iSkillId = INVALID_SKILL_ID;
	};

	// Development Balance Tool intent. The authenticated session identifies the
	// player; no position or HP is trusted from the client.
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

	// A HOLD skill may keep turning toward the cursor while it charges. The
	// server only honors this during the charge stages; the firing stage keeps
	// the last direction it was given.
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

	// The raid Esther roster is positional: slot 1..3 maps to the world's
	// authored summon order (Valtan: Sillian, Wei, Bahuntur). The server owns
	// which slots are usable; the client never sends an archetype or skill id.
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

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_REVIVE_PLAYER& message);

	bool Read_Message(
		CPacketReader& reader,
		C2S_REVIVE_PLAYER& message);

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
		DEAD,
		END
	};

	enum class PLAYER_STANCE_ID : std::uint8_t
	{
		NONE,
		LANCE_MASTER_LONG_SPEAR,
		LANCE_MASTER_SHORT_SPEAR,
		WARLORD_NORMAL,
		WARLORD_DEFENSE,
		END
	};

	struct SKILL_COOLDOWN_SNAPSHOT
	{
		SKILL_ID iSkillId = INVALID_SKILL_ID;
		std::uint32_t iCooldownEndTick = 0;
	};

	//player
	struct PLAYER_SNAPSHOT
	{
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
		std::uint32_t iCurrentHp = 1;
		std::uint32_t iMaximumHp = 1;
		std::uint32_t iCurrentResource = 0;
		std::uint32_t iMaximumResource = 1;
		// The class identity gauge. A maximum of 0 says the class has none, and
		// the HUD then has nothing to draw.
		std::uint32_t iCurrentIdentity = 0;
		std::uint32_t iMaximumIdentity = 0;
		bool isCombatReady = true;
		// 0 outside a staged action, 1-based stage index while one runs: combo
		// stages, and start/loop/end for a HOLD skill. The server owns it; the
		// client must not count stages itself.
		std::uint8_t iComboStage = 0;
		std::vector<SKILL_COOLDOWN_SNAPSHOT> Cooldowns;
	};

	struct WORLD_ENTITY_SNAPSHOT
	{
		NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
		WORLD_ENTITY_ACTION eAction = WORLD_ENTITY_ACTION::END;
		std::string strPatternId;
		std::string strActionId;
		std::uint32_t iPatternSequence = 0;
		std::uint32_t iPatternStageIndex = 0;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;
		std::uint32_t iActionStartTick = 0;
		std::uint32_t iCurrentHp = 1;
		std::uint32_t iMaximumHp = 1;
		std::uint8_t iPhase = 1;
	};
	// One resolved hit. HP in the snapshots above is a level, so a client that
	// only sees levels cannot tell 500 damage from two 250s inside one tick, and
	// cannot place a number where the hit landed. The server already computes this
	// value to subtract it; this carries the same number rather than letting the
	// client re-derive one it has no authority for. This is the snapshot's only
	// edge-triggered payload: a missed event never desyncs HP.
	struct DAMAGE_EVENT
	{
		// Whoever took the damage: a player or a world entity, both of which live
		// in the same NET_ENTITY_ID space.
		NET_ENTITY_ID iTargetNetEntityId = INVALID_NET_ENTITY_ID;
		std::uint32_t iAmount = 0;
		// Where to anchor the number, in world units. Taken from the target at the
		// moment of the hit so a number does not follow the target afterwards.
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		// True when a player dealt it. Presentation styles incoming and outgoing
		// damage differently, and only the server knows which is which.
		bool isOutgoing = false;
	};

	//player snapshot을 vector 구조체로 들고, servertick을 들고있다?
	struct S2C_WORLD_SNAPSHOT
	{
		std::uint32_t iServerTick = 0;
		WORLD_ID eWorldId = WORLD_ID::BERN;
		std::vector<PLAYER_SNAPSHOT> Players;
		std::vector<WORLD_ENTITY_SNAPSHOT> Entities;
		std::vector<DAMAGE_EVENT> DamageEvents;
		// Room-shared raid Esther gauge. A maximum of 0 says this world has no
		// Esther roster and the HUD then has nothing to draw.
		std::uint32_t iEstherGauge = 0;
		std::uint32_t iEstherGaugeMaximum = 0;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_WORLD_SNAPSHOT& message);

	bool Read_Message(
		CPacketReader& reader,
		S2C_WORLD_SNAPSHOT& message);

	// Persistent destroyable state is replicated separately from the 30 Hz
	// world snapshot. A full sync never contains historical one-shot events;
	// a delta contains only changed states and events emitted by the live room.
	inline constexpr std::size_t MAX_COMBAT_RUNTIME_REVISION_BYTES = 64;
	inline constexpr std::size_t MAX_WORLD_DESTRUCTION_GROUPS = 128;
	inline constexpr std::size_t MAX_WORLD_DESTRUCTION_CHANGED_STATES = 128;
	inline constexpr std::size_t MAX_WORLD_DESTRUCTION_EVENTS = 64;

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

	struct S2C_WORLD_DESTRUCTION_FULL_SYNC
	{
		std::string strCombatRuntimeRevision;
		std::uint32_t iServerTick = 0;
		std::uint32_t iEncounterEpoch = 0;
		std::vector<WORLD_DESTRUCTION_STATE_WIRE> GroupStates;
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
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_WORLD_DESTRUCTION_DELTA& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_WORLD_DESTRUCTION_DELTA& message);

	// Debug audition of an authored Valtan health-bar pattern.
	//
	// The request never names a pattern, a camera cue or a wall group. It names
	// a health bar the encounter already authors, and the Server reproduces the
	// real crossing at that bar so the pattern, its cinematic cue and its wall
	// binding all run through the product path. Dropping straight to the target
	// bar would cross every threshold above it and queue those patterns too.
	// PLAY atomically primes the previous bar and moves onto the target in one
	// Server command, then CValtanBrain judges the single crossing on its own
	// fixed tick. ARM/CROSS remain available for step-by-step diagnostics.
	enum class VALTAN_AUDITION_OPERATION : std::uint8_t
	{
		ARM_HEALTH_BAR,
		CROSS_HEALTH_BAR,
		PLAY_HEALTH_BAR,
		END
	};

	enum class VALTAN_AUDITION_RESULT : std::uint8_t
	{
		ARMED,
		QUEUED,
		// The same request sequence arriving twice is answered, not replayed.
		DUPLICATE_IGNORED,
		REJECTED_RELEASE_BUILD,
		REJECTED_WRONG_WORLD,
		REJECTED_NO_BOSS,
		REJECTED_BOSS_DEAD,
		// The bar carries no authored HEALTH_BAR pattern, so there is nothing
		// to audition and no reason to move the boss.
		REJECTED_UNKNOWN_HEALTH_BAR,
		// That pattern already fired this encounter, or a pattern is running.
		REJECTED_PATTERN_UNAVAILABLE,
		// CROSS without a preceding ARM at the same bar would cross an unknown
		// span of thresholds.
		REJECTED_NOT_ARMED,
		// CValtanBrain drops its target when nobody is combat-ready inside the
		// engage distance, so a pattern queued now would never start.
		REJECTED_PLAYER_NOT_ENGAGED,
		END
	};

	struct C2S_VALTAN_AUDITION_REQUEST
	{
		std::uint32_t iRequestSequence = 0;
		VALTAN_AUDITION_OPERATION eOperation =
			VALTAN_AUDITION_OPERATION::ARM_HEALTH_BAR;
		std::uint32_t iTargetHealthBar = 0;
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
		// The bar the boss actually sits on after the Server handled the
		// request, so a rejected audition still reports the live state.
		std::uint32_t iCurrentHealthBar = 0;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_VALTAN_AUDITION_RESULT& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_VALTAN_AUDITION_RESULT& message);
}

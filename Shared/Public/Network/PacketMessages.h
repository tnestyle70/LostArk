#pragma once

#include "GameplayDataRevision.h"
#include "Network/PacketType.h"
#include "NetworkIds.h"

#include <string>
#include <string_view>
#include <limits>
#include <vector>
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

	// Same stable-ID alphabet the authored world sequence document enforces, so
	// a wire value can never name something the Client could not have loaded.
	[[nodiscard]] bool Is_Valid_SequenceInstanceId(
		std::string_view instanceId) noexcept;

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
		// Immutable dependent-boss relation. Zero identifies an independent entity.
		NET_ENTITY_ID iOwnerBossNetEntityId = INVALID_NET_ENTITY_ID;
		WORLD_ENTITY_KIND eKind = WORLD_ENTITY_KIND::END;
		std::string strArchetypeId;
		std::string strEncounterId;
		std::string strPlacementId;
		/* The entity's action at the spawn moment, same id space as the
		snapshot's strActionId. An entity that spawns mid-action (a raid Esther
		summon) presents its clip from its very first frame instead of standing
		one snapshot interval in the idle clip. Empty = spawns idle. */
		std::string strActionId;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;
		// Server-authoritative XZ combat body radius. NPC presentations have no
		// combat body and therefore publish zero.
		float fCollisionRadius = 0.f;
		/* Exact immutable gameplay generation that owns this entity at spawn.
		   It may differ from the room-active revision when a late join observes
		   an occurrence retained across a byte-identical live activation. */
		GameplayDataRevision PinnedDefinitionRevision{};
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_WORLD_ENTITY_SPAWNED& spawned);
	bool Read_Message(
		CPacketReader& reader,
		S2C_WORLD_ENTITY_SPAWNED& spawned);

	// Reliable admission is owner-first. A dependent boss may only refer to an
	// independent boss in the same encounter; nested ownership is not supported.
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
		// DEAD is reliable even when the final HP-zero snapshot was not observed.
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

	/* Reliable, self-contained occurrence. The gameplay Server never sends a
	Sound or Effect asset name; presentation data joins the stable hit identity
	to an asset locally. Keeping the pose in the event also permits a future 3D
	audio/effect projection after the object despawns in the same room tick. */
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
		/* Damage-backed pulses carry their hitId. A visual-only typed combat
		object carries its stable presentationEventId in the same wire field. */
		std::string strHitId;
		std::uint32_t iRepeatIndex = 0u;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;
		GameplayDataRevision PinnedDefinitionRevision{};
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_COMBAT_OBJECT_PRESENTATION_EVENT& event);
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
	// Lance Master alone authors nine ACTIVE skills and can hold all nine on
	// cooldown at once, so eight silently dropped one tile from the HUD.
	inline constexpr std::size_t MAX_PLAYER_COOLDOWNS = 16;
	// One tick applies at most one player hit and one boss hit per actor, so this
	// bounds a 30 Hz frame rather than a fight.
	inline constexpr std::size_t MAX_DAMAGE_EVENTS = 64;
	/* A world entity wears at most this many destructible armour plates, one
	bit each in WORLD_ENTITY_SNAPSHOT::iBrokenArmorMask. The Server catalog and
	the Client parts that wear them read this same bound, so a plate can never
	be authored that the wire cannot name. */
	inline constexpr std::uint8_t MAX_WORLD_ENTITY_ARMOR_PLATES = 4;
	// Boss combat edges are one-tick presentation events. Persistent boss state
	// lives on WORLD_ENTITY_SNAPSHOT so missing an edge cannot desynchronize it.
	inline constexpr std::size_t MAX_BOSS_COMBAT_EVENTS = 64;
	inline constexpr std::size_t MAX_COMBAT_OBJECTS_PER_SNAPSHOT = 128;
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

	/* AIM_POINT preserves the existing world-point aim contract. GROUND_POINT
	 carries an exact world XZ which the server must range/nav validate before
	 it becomes an action target. Keeping the kind typed prevents an old or
	 compromised client from making an ordinary directional skill acquire
	 ground-target semantics just by changing its coordinates. */
	enum class SKILL_TARGET_INTENT_KIND : std::uint8_t
	{
		AIM_POINT,
		GROUND_POINT,
		END
	};

	// Client intent contains no player or entity ID. The server resolves the
	// actor from the authenticated session that owns this command.
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

	// A HOLD skill leaves its loop when the player lets the key go. COMBO input
	// uses discrete/repeated USE_SKILL commands and does not consume mouse-up.
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

	// Debug/Development-build test aid only -- see the PACKET_TYPE declaration comment.
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

	/* Debug-only Character Select -> KakulSaydon Server transfer. The ordinary
	S2C_ENTER_ACCEPTED result remains the single level-transition authority. */
	struct C2S_DEBUG_ENTER_KAKULSAYDON_ARENA
	{
		std::uint32_t iRequestSequence = 0;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_DEBUG_ENTER_KAKULSAYDON_ARENA& message);

	bool Read_Message(
		CPacketReader& reader,
		C2S_DEBUG_ENTER_KAKULSAYDON_ARENA& message);

	/* Debug-only stage navigation. strPlacementId must name an authored
	PLAYER_SPAWN row in the current KakulSaydon world bootstrap. */
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

	/* The picked point is intent. The current room owns navigation, height,
	collision and the eventual replicated player transform. */
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
		/* Nonzero positions are meaningful only for ACCEPTED. Rejection has no
		position side effect; the ordinary snapshot remains the state authority. */
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

	/* Logical side-scroll intent only. The Server resolves the current segment
	axis; STOP releases movement at its authoritative player position. */
	struct C2S_MARIO_MOVE
	{
		std::uint32_t iClientSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		MARIO_DIRECTION eDirection = MARIO_DIRECTION::STOP;
	};

	bool Write_Message(CPacketWriter& writer, const C2S_MARIO_MOVE& message);
	bool Read_Message(CPacketReader& reader, C2S_MARIO_MOVE& message);

	/* Debug test aid: LEFT/RIGHT selects a side, never a world-space aim or
	landing. The Server owns the bounded jump and the snapshot owns position. */
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

	/* Which body a player presents. NORMAL is the class body; CLOWN is the
	colourless KoukuSaydon body a full madness gauge turns the player into. */
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

	/* Debug F1 "Change to Clown" / "Return to Player". The Server owns the
	form and replicates it in PLAYER_SNAPSHOT; this only asks for it. */
	/* Debug F1 bingo check: paints cells white and lets the Server promote any
	line that completes. Release ignores it. The board comes back on the next
	world snapshot, so there is no separate result message. */
	struct C2S_DEBUG_BINGO_FILL
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		// Bits 0..24. Cells already painted stay painted.
		std::uint32_t iCellMask = 0u;
		// Clears the board before painting, so one button can restart the check.
		bool bReset = false;
	};

	bool Write_Message(CPacketWriter& writer,
		const C2S_DEBUG_BINGO_FILL& message);
	bool Read_Message(CPacketReader& reader,
		C2S_DEBUG_BINGO_FILL& message);

	/* Debug bingo bomb: marks the requesting session's own player. The
	Server picks the carrier, so the request names no entity. */
	struct C2S_DEBUG_BINGO_BOMB
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
	};

	bool Write_Message(CPacketWriter& writer,
		const C2S_DEBUG_BINGO_BOMB& message);
	bool Read_Message(CPacketReader& reader,
		C2S_DEBUG_BINGO_BOMB& message);

	/* Debug bingo hammer. The Server rolls the anchor, so the request
	carries nothing but its own identity. */
	struct C2S_DEBUG_BINGO_HAMMER
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
	};

	bool Write_Message(CPacketWriter& writer,
		const C2S_DEBUG_BINGO_HAMMER& message);
	bool Read_Message(CPacketReader& reader,
		C2S_DEBUG_BINGO_HAMMER& message);

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
		// The form the player has after this request, accepted or not.
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

	/* KoukuSaydon mechanic card shown over a player's head. The Server assigns it
	when a roulette window opens and clears it when the window is judged. */
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

	/* KoukuSaydon card maze. The first player to claim the telescope owns it
	and hunts nothing; every other living player is dealt one distinct suit to
	hunt. The Server owns both; the Client only draws them. */
	enum class CARD_MAZE_ROLE : std::uint8_t
	{
		NONE,
		TELESCOPE,
		HUNTER,
		END
	};

	/* Server-owned maze state. Flags: observing=1, escaped=2, exit=4,
	   leaving maze=8, entry cutscene hidden=16. Effect assets and camera poses stay Client-side. */
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

	/* Interaction HUD the Server decides for one player: the clown polymorph,
	the Mario side-scroll, the dance time and the card maze. NONE keeps the class
	HUD. Which icon sits in which quick slot travels with it. */
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

	/* Q W E R A S D F, the order the HUD walks its quick-slot row. */
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
	/* One interaction HUD press locks the player into INTERACTION while the
	Client plays the clip the mode assigns to the pressed slot. This is the
	ceiling, used by a slot with no authored length of its own. */
	constexpr std::uint32_t KOUKU_INTERACTION_ACTION_MS = 3000u;
	constexpr std::uint32_t KOUKU_INTERACTION_COOLDOWN_MS = 3000u;
	/* The card maze hammer keeps the authored swing above, because the Client
	scrubs that clip by the action's age and returns to idle the moment the
	action ends. Only the cooldown is shortened, so a hunter who cancels the
	recovery with their own next input can swing again at once. */
	constexpr std::uint32_t KOUKU_MAZE_HAMMER_COOLDOWN_MS = 400u;
	/* The Client scrubs an interaction clip by the action's age and clamps at
	the clip's last frame, so a lock longer than the clip leaves the doll
	frozen on that frame for the remainder. Each slot therefore ends on the
	length its clip is authored at in
	Data/Animation/Authored/KoukuSaydon/Clown.interactionbindings.json,
	measured on MN_RPCZ_00-1 at the runtime's 30 ticks per second. A slot
	outside a mode's authored set keeps the ceiling, and so does the card maze
	escape teleport, which borrows this same action with no slot of its own. */
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
			return 0u == slotSkillIndex ? 2500u : KOUKU_INTERACTION_ACTION_MS;
		default:
			/* The four dance poses are authored at exactly the ceiling. */
			return KOUKU_INTERACTION_ACTION_MS;
		}
	}

	/* The KoukuSaydon bingo board. The authored FLOOR03 floor tiles already
	form it: 25 tiles at yaw 0, 3.04 m apart, x -6.08..6.08 and z
	1140.80..1152.96. Nothing is rotated, so a world position resolves to a
	cell with two divisions. The diamond look in game is the camera, not the
	board. Cell index is row * 5 + column, row along +Z and column along +X,
	so cell 0 is the -X/-Z corner and cell 24 the +X/+Z corner. Server and
	Client both call these; neither keeps a second copy of the arithmetic. */
	constexpr std::int32_t KOUKU_BINGO_SIDE = 5;
	constexpr std::int32_t KOUKU_BINGO_CELL_COUNT = KOUKU_BINGO_SIDE * KOUKU_BINGO_SIDE;
	constexpr float KOUKU_BINGO_CELL_SIZE_M = 3.04f;
	/* Centre of cell 0. */
	constexpr float KOUKU_BINGO_ORIGIN_X = -6.08f;
	constexpr float KOUKU_BINGO_ORIGIN_Z = 1140.8f;
	constexpr std::int32_t KOUKU_BINGO_OFF_BOARD = -1;
	/* Bits 0..24, one per cell. Anything above bit 24 is not a cell. */
	constexpr std::uint32_t KOUKU_BINGO_ALL_CELLS_MASK = 0x1FFFFFFu;
	/* Five rows, five columns and both diagonals all count as a bingo. */
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

	/* The cell containing this position, or KOUKU_BINGO_OFF_BOARD. Shifting by
	half a cell before truncating makes the nearest centre and containment the
	same answer on a uniform grid. The comparisons are written so a NaN falls
	out as off board rather than truncating to an arbitrary index. */
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

	/* A cell stepped by whole rows and columns. Stepping a column must not
	carry into the neighbouring row, which is why this is not index math. */
	/* The cells of one line. Row r fills five adjacent bits; column c takes
	every fifth bit; the last two are the diagonals. Zero for a line that
	does not exist, so a caller cannot complete a line by accident. */
	constexpr std::uint32_t Kouku_BingoLineMask(const std::int32_t line) noexcept
	{
		if (line < 0 || line >= KOUKU_BINGO_LINE_COUNT)
			return 0u;
		if (line < KOUKU_BINGO_SIDE)
			return 0x1Fu << (line * KOUKU_BINGO_SIDE);
		if (line < KOUKU_BINGO_SIDE * 2)
			return 0x108421u << (line - KOUKU_BINGO_SIDE);
		/* 0, 6, 12, 18, 24 and 4, 8, 12, 16, 20. */
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

	/* The five cells one bomb paints: its own and the four orthogonal
	neighbours. Neighbours that fall off the board are dropped, and a bomb
	that did not land on the board paints nothing at all. */
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

	/* Bingo bomb. The Server owns the whole clock: the mark rides one
	player, and when it expires the bomb is planted at wherever that player
	was standing. The Client draws only the phase the snapshot names. */
	constexpr std::uint32_t KOUKU_BINGO_BOMB_MARK_MS = 3000u;
	/* How long the planted bomb burns before it paints its cross. */
	constexpr std::uint32_t KOUKU_BINGO_BOMB_FUSE_MS = 2000u;
	/* One mark per player in a full room. */
	constexpr std::int32_t KOUKU_BINGO_MAX_BOMBS = 4;

	enum class BINGO_BOMB_PHASE : std::uint8_t
	{
		NONE = 0,
		/* Riding the carrier. */
		MARKED,
		/* Dropped where the carrier stood. */
		PLANTED,
		END
	};

	struct BINGO_BOMB_SNAPSHOT
	{
		/* MARKED only; the Client reads the carrier's own transform. */
		NET_ENTITY_ID iCarrierNetEntityId = INVALID_NET_ENTITY_ID;
		/* PLANTED only. */
		float fPositionX = 0.f;
		float fPositionZ = 0.f;
		BINGO_BOMB_PHASE ePhase = BINGO_BOMB_PHASE::NONE;
	};

	/* Bingo hammer. It only ever runs a row or a column, so it picks one of
	the first ten lines that Kouku_BingoLineMask already numbers that way;
	the two diagonals stay a bingo rule and are never swept. Each line has
	two ends, which is where the twenty anchors come from. Nothing here is
	authored: every anchor falls out of the board constants. */
	constexpr std::int32_t KOUKU_BINGO_HAMMER_LINE_COUNT = KOUKU_BINGO_SIDE * 2;
	constexpr std::int32_t KOUKU_BINGO_HAMMER_ANCHOR_COUNT =
		KOUKU_BINGO_HAMMER_LINE_COUNT * 2;
	/* Hangs in the sky, comes down slowly, then whips down the line. */
	constexpr std::uint32_t KOUKU_BINGO_HAMMER_RAISE_MS = 2000u;
	constexpr std::uint32_t KOUKU_BINGO_HAMMER_DESCEND_MS = 1400u;
	constexpr std::uint32_t KOUKU_BINGO_HAMMER_SWEEP_MS = 1600u;
	/* How high above the board the hammer waits. */
	constexpr float KOUKU_BINGO_HAMMER_SKY_Y = 9.f;
	/* The head never comes back up during the sweep. It tips this far past
	hanging, into the direction of travel, as if flung on its chain. The
	authored model already hangs with its head at the origin and its chain
	running up, so nothing else has to be rotated to make it hang. */
	constexpr float KOUKU_BINGO_HAMMER_LEAN_DEGREES = 40.f;
	/* How much of the sweep the lean takes to reach full. */
	constexpr float KOUKU_BINGO_HAMMER_LEAN_RATIO = 0.3f;

	enum class BINGO_HAMMER_PHASE : std::uint8_t
	{
		NONE = 0,
		/* Waiting in the sky over its entry point. */
		RAISED,
		/* Coming down onto the board. */
		DESCENDING,
		/* Running the line from entry to exit. */
		SWEEPING,
		END
	};

	constexpr bool Is_KoukuBingoHammerAnchor(const std::int32_t anchor) noexcept
	{
		return anchor >= 0 && anchor < KOUKU_BINGO_HAMMER_ANCHOR_COUNT;
	}

	/* Where one anchor's sweep begins and ends, both one cell outside the
	board so the whole line is covered. Even anchors run along the axis,
	odd anchors run back the other way. */
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
			/* A row: the hammer travels along X at that row's Z. */
			const float z = Kouku_BingoCellCenterZ(line * KOUKU_BINGO_SIDE);
			path.fStartX = KOUKU_BINGO_ORIGIN_X + (reversed ? high : low);
			path.fEndX = KOUKU_BINGO_ORIGIN_X + (reversed ? low : high);
			path.fStartZ = z;
			path.fEndZ = z;
			return path;
		}
		/* A column: the hammer travels along Z at that column's X. */
		const float x = Kouku_BingoCellCenterX(line - KOUKU_BINGO_SIDE);
		path.fStartX = x;
		path.fEndX = x;
		path.fStartZ = KOUKU_BINGO_ORIGIN_Z + (reversed ? high : low);
		path.fEndZ = KOUKU_BINGO_ORIGIN_Z + (reversed ? low : high);
		return path;
	}

	/* The cells one anchor's sweep passes over: its whole line. */
	constexpr std::uint32_t Kouku_BingoHammerLineMask(
		const std::int32_t anchor) noexcept
	{
		return Is_KoukuBingoHammerAnchor(anchor) ?
			Kouku_BingoLineMask(anchor / 2) : 0u;
	}

	struct BINGO_HAMMER_SNAPSHOT
	{
		/* -1 while idle. */
		std::int32_t iAnchor = -1;
		/* The Client interpolates the current phase between these two, so
		no per-tick position needs to ride the wire. */
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
		/* An empty or backwards window would make the Client divide by
		zero or run the sweep in reverse. */
		return hammer.iPhaseEndTick > hammer.iPhaseStartTick;
	}

	/* Reserved interaction cooldown identity. It is separate from the mode's
	zero-based animation index and from product character skill IDs. */
	constexpr SKILL_ID Kouku_InteractionCooldownSkillId(
		const KOUKU_HUD_MODE mode, const std::uint32_t skillIndex) noexcept
	{
		return 0xFFFF0000u | (static_cast<std::uint32_t>(mode) << 8u) | (skillIndex + 1u);
	}

	constexpr bool Is_Valid_InteractionSlot(const INTERACTION_SLOT slot) noexcept
	{
		return slot < INTERACTION_SLOT::END;
	}

	/* One quick-slot press while an interaction HUD mode is active. The Server
	resolves the slot through the player's replicated slot layout; the Client
	never names a pose or skill here. */
	struct C2S_INTERACTION_SLOT
	{
		std::uint32_t iRequestSequence = 0u;
		WORLD_ID eWorldId = WORLD_ID::END;
		INTERACTION_SLOT eSlot = INTERACTION_SLOT::Q;
	};

	bool Write_Message(CPacketWriter& writer, const C2S_INTERACTION_SLOT& message);
	bool Read_Message(CPacketReader& reader, C2S_INTERACTION_SLOT& message);

	/* Debug F1 "HUD Mode: MARIO / MAZE / Clear". Only the two modes whose
	gimmick has no Server trigger yet may be forced; the Server still owns the
	replicated mode and Release answers with a typed rejection. */
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
		// The override the player has after this request, accepted or not.
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

	/* A boss pattern's authored scene-profile cue reached its Server tick. The
	profile ID is an opaque stable presentation ID the Client resolves against
	its own rendering profile catalog; durationMs 0 keeps the profile until the
	next cue or the level's own profile restores it. */
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
		/* The authored ground under the player was removed by a collapse. The
		server owns the descent and the death tick; the snapshot carries only
		this state and the position it already sends, so no field is added.
		Appended last so it takes the newest wire value instead of renumbering
		an existing one. */
		FALLING,
		/* The caster raises a hand to call an Esther. The room owns the fixed
		duration; the state carries no skill id because the summon is a roster
		slot, not a balance skill. Appended last, same wire rule as FALLING. */
		ESTHER_CAST,
		/* A boss owns the player's gameplay transform until it releases the
		attachment. The Server recomputes the world position and yaw from its
		boss-local attachment snapshot and judges, releases and ejects from it;
		the Client draws the body on the owner presentation's hand socket while
		this state lasts. */
		GRABBED,
		/* One KoukuSaydon interaction HUD slot press. The room owns the fixed
		length (KOUKU_INTERACTION_ACTION_MS) and iSkillId carries the mode skill
		index the slot resolved to (0..KOUKU_HUD_SLOT_COUNT-1), never a balance
		skill id. Appended last, same wire rule as FALLING. */
		INTERACTION,
		FEAR,
		END
	};

	/* Typed replicated attachment identity. This is deliberately not a model
	bone name: Shared and Server never consume Client assets. The Client alone
	maps the slot to its own presentation socket. */
	enum class PLAYER_ATTACHMENT_SLOT : std::uint8_t
	{
		NONE,
		BOSS_LEFT_HAND,
		/* A world object carries the player instead of a boss bone: the Gate 3
		hook hangs whoever it sweeps from its point. The owner id still names the
		boss that owns the running pattern, because that is what release and
		despawn key on, but the Client must not look for a hand socket. */
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
		/* The server consumes MOVE sequences even when a gameplay lock or path
		validation refuses the requested goal. This acknowledgement retires only
		that input; it never authorizes a Client position as gameplay truth. */
		std::uint32_t iLastProcessedMoveSequence = 0u;
		// Effective metres/second, including the active stance speed scale.
		float fMoveSpeed = 0.f;
		// Ordinary click locomotion is currently permitted by the Server.
		bool canPredictMove = false;
		/* Only an active ordinary movement path is exposed. Forced movement and
		Mario use their own Server state. The next waypoint is canonical zero
		when there is no ordinary path; it is never a Client-authored position. */
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
		/* Canonical only while eAction is GRABBED. The owner is a replicated
		world entity and the typed slot identifies the Server attachment contract;
		every other action must carry INVALID/NONE so a stale attachment cannot
		survive release. */
		NET_ENTITY_ID iAttachmentOwnerNetEntityId = INVALID_NET_ENTITY_ID;
		PLAYER_ATTACHMENT_SLOT eAttachmentSlot = PLAYER_ATTACHMENT_SLOT::NONE;
		/* Captured once in the owner's gameplay-root frame. The Server uses these
		offsets each tick to publish the authoritative player world transform that
		judgement and release consume; Client presentation follows the owner
		socket instead and keeps this position as its fallback.
		Canonical zero outside GRABBED. */
		float fAttachmentLocalOffsetX = 0.f;
		float fAttachmentLocalOffsetY = 0.f;
		float fAttachmentLocalOffsetZ = 0.f;
		float fAttachmentYawOffsetDegrees = 0.f;
		/* Present only for a server-approved GROUND_POINT skill. The Y component
		 is sampled from the authoritative navigation grid so every client roots
		 the action effect at the same surface. A non-target action must carry
		 false and canonical zero coordinates. */
		bool hasSkillTarget = false;
		float fSkillTargetX = 0.f;
		float fSkillTargetY = 0.f;
		float fSkillTargetZ = 0.f;
		std::uint32_t iCurrentHp = 1;
		std::uint32_t iMaximumHp = 1;
		std::uint32_t iCurrentResource = 0;
		std::uint32_t iMaximumResource = 1;
		// The class identity gauge. A maximum of 0 says the class has none, and
		// the HUD then has nothing to draw.
		std::uint32_t iCurrentIdentity = 0;
		std::uint32_t iMaximumIdentity = 0;
		/* KoukuSaydon madness gauge and the avatar it drives. Both are Server
		truth: a maximum of 0 means no Saydon encounter owns the gauge, and the
		form says which body the Client presents for this player. */
		std::uint32_t iCurrentMadness = 0;
		std::uint32_t iMaximumMadness = 0;
		PLAYER_MADNESS_FORM eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
		/* KoukuSaydon interaction state. The card is the symbol a roulette
		window dealt this player; the mode and its slot layout are the HUD the
		Server wants drawn (index into the mode's authored icon list per Q..F
		slot, -1 empty). NONE carries an all-empty layout. */
		MECHANIC_CARD_SYMBOL eMechanicCardSymbol = MECHANIC_CARD_SYMBOL::NONE;
		MECHANIC_CARD_COLOR eMechanicCardColor = MECHANIC_CARD_COLOR::NONE;
		KOUKU_HUD_MODE eKoukuHudMode = KOUKU_HUD_MODE::NONE;
		std::int8_t ModeSkillIndexBySlot[KOUKU_HUD_SLOT_COUNT] =
			{ -1, -1, -1, -1, -1, -1, -1, -1 };
		// 0 outside Mario; 1..4 identify the Server-owned side-scroll stage.
		std::uint8_t iMarioStage = 0u;
		// Server-selected source layout: 0 unselected, 1..3 original cases.
		std::uint8_t iMarioLayoutVariant = 0u;
		/* Source balls of that layout the Server has popped, bit = the layout's
		binding slot (9 on stages 1-3, 12 on stage 4). Zero outside Mario. */
		std::uint16_t iMarioPoppedBallMask = 0u;
		// Bit 0 red, 1 blue, 2 yellow: every ball of that colour is popped.
		std::uint8_t iMarioCurseReleasedMask = 0u;
		/* Card maze truth. NONE carries suit NONE and zero counts; a HUNTER
		carries the suit it was dealt and kills <= the kill target. */
		CARD_MAZE_ROLE eCardMazeRole = CARD_MAZE_ROLE::NONE;
		MECHANIC_CARD_SYMBOL eCardMazeSuit = MECHANIC_CARD_SYMBOL::NONE;
		std::uint8_t iCardMazeKills = 0u;
		std::uint8_t iCardMazeKillTarget = 0u;
		CARD_MAZE_PRESENTATION CardMaze;
		bool isCombatReady = true;
		/* Pattern bind is a Server-authoritative control lock. The deadline lets a
		late Client present the remaining window without deciding its lifetime. */
		bool isPatternBound = false;
		std::uint32_t iPatternBindEndTick = 0;
		/* Zero means skills are enabled. A non-zero Server deadline masks every
		product skill slot while movement remains available. */
		std::uint32_t iSilenceEndTick = 0;
		std::uint32_t iSilenceDurationTicks = 0;
		// FEAR uses iActionStartTick and a Server-owned deadline.
		std::uint32_t iFearEndTick = 0u;
		std::string strFearPresentationId;
		// 0 outside a staged action, 1-based stage index while one runs: combo
		// stages, and start/loop/end for a HOLD skill. The server owns it; the
		// client must not count stages itself.
		std::uint8_t iComboStage = 0;
		std::vector<SKILL_COOLDOWN_SNAPSHOT> Cooldowns;
	};

	enum class BOSS_COMBAT_STATE_FLAG : std::uint16_t
	{
		NONE = 0,
		INVULNERABLE = 1u << 0,
		SHIELDED = 1u << 1,
		COUNTERABLE = 1u << 2,
		GROGGY = 1u << 3,
		/* Phase-three keeps the primary boss identity and HP alive while its
		presentation is absent for the one-tick relocation boundary. */
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
		std::uint8_t iGameplayPhase = 1;
	};

	/* Immutable Server route for one delayed Portal target-rush Stage.  The
	Client cannot reconstruct this from the current boss pose because snapshot
	coalescing may first expose the Stage after travel has already started. */
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

	struct WORLD_ENTITY_SNAPSHOT
	{
		NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
		WORLD_ENTITY_ACTION eAction = WORLD_ENTITY_ACTION::END;
		std::string strPatternId;
		std::string strActionId;
		std::uint32_t iPatternSequence = 0;
		std::uint32_t iPatternStartTick = 0;
		std::uint32_t iPatternStageIndex = 0;
		// Server-selected player locked by the running boss pattern. Non-boss
		// entities must leave this invalid; Client presentation never reselects it.
		NET_ENTITY_ID iPatternTargetNetEntityId = INVALID_NET_ENTITY_ID;
		PORTAL_RUSH_ROUTE_SNAPSHOT PortalRushRoute;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;
		std::uint32_t iActionStartTick = 0;
		std::uint32_t iCurrentHp = 1;
		std::uint32_t iMaximumHp = 1;
		std::uint8_t iPhase = 1;
		/* Bit i is set once authored armour plate i has been destroyed. The
		server owns the durability that breaks it; presentation only hides the
		part wearing that index. Zero means every plate is still on, which is
		also what an entity that wears no armour sends. */
		std::uint8_t iBrokenArmorMask = 0;
		// Only boss entities set this. Keeping it explicit avoids spending the
		// boss payload on every NPC/monster while preserving one typed contract.
		bool hasBossCombatState = false;
		BOSS_COMBAT_SNAPSHOT BossCombat;
		// The immutable definition generation pinned when this occurrence began.
		GameplayDataRevision PinnedDefinitionRevision{};
	};
	// One resolved hit. HP in the snapshots above is a level, so a client that
	// only sees levels cannot tell 500 damage from two 250s inside one tick, and
	// cannot place a number where the hit landed. The server already computes this
	// value to subtract it; this carries the same number rather than letting the
	// client re-derive one it has no authority for. Like boss combat events below,
	// a missed presentation edge never desynchronizes its persistent level state.
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
		/* NONE on an ordinary hit. A card maze shard instead names the suit the
		hunter was dealt and carries their running count in iAmount, because the
		entity snapshot has no archetype for the Client to read the suit from. */
		MECHANIC_CARD_SYMBOL eCardMazeSuit = MECHANIC_CARD_SYMBOL::NONE;
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

	/* The whole bingo board in eight bytes. A set white bit is a skull; a set
	red bit is a skull on a completed line, which is why red is always a
	subset of white. Both are room state, not per player. */
	struct BINGO_BOARD_SNAPSHOT
	{
		std::uint32_t iWhiteMask = 0u;
		std::uint32_t iRedMask = 0u;
		/* Live bombs, packed into the first iBombCount slots. */
		std::uint8_t iBombCount = 0u;
		BINGO_BOMB_SNAPSHOT Bombs[KOUKU_BINGO_MAX_BOMBS]{};
		/* At most one hammer runs at a time. */
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
		/* A packed slot always names a live phase, and a mark without a
		carrier would leave the Client with nothing to follow. */
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

	//player snapshot을 vector 구조체로 들고, servertick을 들고있다?
	struct S2C_WORLD_SNAPSHOT
	{
		std::uint32_t iServerTick = 0;
		WORLD_ID eWorldId = WORLD_ID::BERN;
		std::vector<PLAYER_SNAPSHOT> Players;
		std::vector<WORLD_ENTITY_SNAPSHOT> Entities;
		std::vector<DAMAGE_EVENT> DamageEvents;
		BINGO_BOARD_SNAPSHOT Bingo;
		std::vector<BOSS_COMBAT_EVENT> BossCombatEvents;
		// Full live set, sorted by iCombatObjectId. Reliable spawn/despawn
		// frames remain ordering barriers around this 30 Hz transform level.
		std::vector<COMBAT_OBJECT_SNAPSHOT> CombatObjects;
		// Room-shared raid Esther gauge. A maximum of 0 says this world has no
		// Esther roster and the HUD then has nothing to draw.
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

	// Persistent destroyable state is replicated separately from the 30 Hz
	// world snapshot. A full sync never contains historical one-shot events;
	// a delta contains only changed states and events emitted by the live room.
	inline constexpr std::size_t MAX_COMBAT_RUNTIME_REVISION_BYTES = 64;
	inline constexpr std::size_t MAX_WORLD_DESTRUCTION_GROUPS = 128;
	inline constexpr std::size_t MAX_WORLD_DESTRUCTION_CHANGED_STATES = 128;
	// A maximum-length delta must still fit one 64 KiB frame when all 128
	// changed-state slots are populated. 106 is the exact remaining budget;
	// the current 109 collapse emits 97 changed states and 97 live events.
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

	// Server-owned collision and navigation counters. The Debug audition panel
	// reports what the Server actually holds instead of inferring passage from
	// the replicated wall states, so a wall that is gone while its blocker is
	// still active stays visible as a defect rather than as a matching number.
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

	// Repeatable encounter props, replicated separately from one-way wall
	// destruction. The four pillars come back four times in one fight, so their
	// slots cycle HIDDEN -> SPAWNING -> INTACT -> BREAKING -> HIDDEN instead of
	// being consumed once. A late joiner receives the current slot states only;
	// it never replays a past spawn or shatter.
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
	// PLAY_ENTRANCE is the one operation that puts the encounter intro ledger
	// back, so the first-appearance sweep runs again on a reset boss. Every other
	// operation stages the intro as consumed and auditions its own pattern first.
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
		// One whole pillar cycle: the encounter's own pillar pattern raises the
		// four slots through the product path, and because no product trigger
		// for the shatter is identified yet, this Debug operation is what closes
		// INTACT -> BREAKING -> HIDDEN so the reusable runtime is verifiable.
		PLAY_PILLAR_CYCLE,
		// Debug-only product-path checks. PLAY_WALL_ATTACK queues the real
		// VALTAN_DOWN_SMASH action against one ordinary wall. SHOW_FINAL_ARENA
		// stages every independent wall through the Server destruction runtime
		// so the fully opened arena can be inspected without replaying 99 hits.
		PLAY_WALL_ATTACK,
		SHOW_FINAL_ARENA,
		// The same Server destruction transaction as SHOW_FINAL_ARENA without
		// the two floor stages. It reproduces the arena the recording shows at
		// the 109 crossing: every wall already smashed while the floor is still
		// whole, so the 84 and 30 collapses can then be auditioned with nothing
		// standing above them.
		BREAK_EVERY_WALL,
		// Starts one selected chronological timeline row, or stops the row that
		// currently owns Debug playback. PLAY uses iTargetHealthBar as the
		// row's stable non-zero command ID; STOP carries exactly zero.
		PLAY_TIMELINE_ROW,
		STOP_TIMELINE_ROW,
		// Debug pattern browser. A NORMAL pattern that no health bar owns can
		// otherwise only be seen by fighting until it is rolled, so this plays
		// one authored pattern chosen by its position in the encounter
		// document. iTargetHealthBar carries that 1-based index rather than a
		// bar: both ends read the same authored order, the Client from
		// ValtanEncounter.json and the Server from the PATTERN rows the
		// publisher writes in that same document order.
		PLAY_PATTERN,
		// Stable-ID pattern audition used by cross-tool Server previews. Unlike
		// PLAY_PATTERN, this never relies on the Client and Server sharing an
		// authored vector position: the request names both the already-spawned
		// boss placement and the pattern owned by its encounter.
		PLAY_PATTERN_ID,
		// Starts normal Server-authoritative encounter playback at one reviewed
		// page boundary. iTargetHealthBar carries that boundary timeline row's
		// stable command ID; only the four product page rows are admitted.
		START_FIGHT_PAGE,
		QUEUE_NEXT_PATTERN_ID,
		CLEAR_NEXT_PATTERN_ID,
		// Adopt the observed Product/owned Flow occurrence without an arena reset.
		// The Server assigns a new audition epoch in the Next lifecycle.
		QUEUE_NEXT_LIVE_PATTERN_ID,
		// Workbench environment staging. The payload is one stable
		// VALTAN_ARENA_PRESET and the Server owns every resulting mutation.
		SET_ARENA_PRESET,
		// Replaces only the exact ACTIVE stable-ID occurrence named by the
		// predecessor room epoch, pattern sequence and pinned definition
		// revision. Unlike PLAY_PATTERN_ID, an ID-only match is never enough.
		RESTART_PATTERN_ID,
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
		CLEARED,
		REJECTED_STALE_REQUEST,
		REJECTED_STALE_AUDITION,
		REJECTED_NOT_OWNER,
		REJECTED_NEXT_CHANGED,
		// RESTART_PATTERN_ID matched the exact predecessor occurrence, but a
		// later preflight rejected the replacement before any boss, player, or
		// arena state changed. Only this verdict authorizes the Client to keep
		// presenting the predecessor occurrence as authoritative.
		REJECTED_OCCURRENCE_PRESERVED,
		END
	};

	struct C2S_VALTAN_AUDITION_REQUEST
	{
		std::uint32_t iRequestSequence = 0;
		VALTAN_AUDITION_OPERATION eOperation =
			VALTAN_AUDITION_OPERATION::ARM_HEALTH_BAR;
		// Usually an authored health bar. PLAY_PATTERN reuses it as a one-based
		// encounter-pattern index, while PLAY_TIMELINE_ROW and START_FIGHT_PAGE
		// reuse it as a timeline row's stable command ID. STOP_TIMELINE_ROW
		// carries 0. SET_ARENA_PRESET carries VALTAN_ARENA_PRESET.
		std::uint32_t iTargetHealthBar = 0;
		// Stable-ID operations encode these strings. Older operations keep their
		// original wire shape and require hidden identity fields to stay empty.
		std::string strBossPlacementId;
		std::string strPatternId;
		// Next controls compare the exact current occurrence and one-slot token.
		// Encoded by all Next controls. LIVE requires epoch/token zero and an
		// observed sequence (zero only when the Server has not run a pattern).
		std::uint32_t iPredecessorRoomAuditionEpoch = 0u;
		std::uint32_t iPredecessorPatternSequence = 0u;
		std::uint32_t iExpectedNextRequestSequence = 0u;
		// PLAY_PATTERN_ID compares the exact active definition that the Client
		// observed before requesting a new occurrence. RESTART_PATTERN_ID compares
		// the immutable definition used by its predecessor occurrence. Every Next
		// queue/replace/clear compares the same predecessor pin; only legacy
		// operations keep this reserved value zero.
		GameplayDataRevision ExpectedDefinitionRevision{};
		// RESTART_PATTERN_ID alone carries a second revision CAS. The predecessor
		// remains pinned to ExpectedDefinitionRevision while this value names the
		// Server-active generation which must own the replacement occurrence. This
		// prevents a concurrent data activation from silently restarting into a
		// generation the Client did not admit. Every other operation keeps zero.
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
		// The bar the boss actually sits on after the Server handled the
		// request, so a rejected audition still reports the live state.
		std::uint32_t iCurrentHealthBar = 0;
		// Exact request echo for the stable-ID result consumer. These strings are
		// encoded only for stable-ID operations; legacy frames are unchanged.
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
		// Server room identity. This is deliberately unrelated to a Client-local
		// inbound generation and advances whenever the room starts a new audition.
		std::uint32_t iRoomAuditionEpoch = 0;
		std::uint32_t iPatternSequence = 0;
		std::string strPatternId;
		VALTAN_AUDITION_LIFECYCLE_STATE eState =
			VALTAN_AUDITION_LIFECYCLE_STATE::PENDING;
		GameplayDataRevision PinnedDefinitionRevision{};
		// Empty except for ABORTED, where it is a bounded diagnostic reason.
		std::string strReason;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_VALTAN_AUDITION_LIFECYCLE& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_VALTAN_AUDITION_LIFECYCLE& message);

	// Debug Boss Tool ordered-flow payload. Slots use stable authoring IDs;
	// their vector order is the playback order. Repeated pattern IDs are valid
	// because two distinct slots may intentionally audition the same pattern.
	// The existing wire count is one byte. Keep the authoring/runtime bound at
	// the full encodable range so pattern inventory growth is not coupled to an
	// arbitrary smaller slot cap.
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
		// Exact gameplay definition generation admitted by the Client when this
		// saved Flow was submitted. The Server compare-and-swaps this value against
		// its active catalog before staging any player, boss, arena, or Flow state.
		GameplayDataRevision ExpectedDefinitionRevision{};
		std::string strBossPlacementId;
		std::string strFlowId;
		// Exact lowercase SHA-256 of the admitted saved authoring bytes.
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
		// One-based display position in the submitted slot vector.
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

	// One response carries at most one retained Server decision and at most the
	// same 64 candidates CValtanBrain admits into its bounded trace. The query's
	// after-sequence makes unchanged polling a small typed response instead of
	// repeatedly shipping the full candidate table.
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
		// Present only for TRACE. It is the immutable definition generation used
		// for the selector lookup that produced Trace.
		GameplayDataRevision DefinitionRevision{};
		VALTAN_DECISION_TRACE_WIRE Trace{};
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_VALTAN_DECISION_TRACE_RESPONSE& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_VALTAN_DECISION_TRACE_RESPONSE& message);

	// Presentation lanes are a bitset, not an enum value. Unknown bits are a
	// protocol error so old peers cannot silently call a new required lane ready.
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

	// Debug-only inventory slice. itemId is a stable catalog ID, not a display
	// string, so it shares the same bound a UI label never needs.
	inline constexpr std::size_t MAX_ITEM_ID_BYTES = 64;
	// One F1 debug session giving itself a handful of catalog items at a time
	// is the whole current use case; a real inventory window is a separate
	// vertical slice.
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

	struct INVENTORY_ITEM_SNAPSHOT
	{
		std::string strItemId;
		std::uint32_t iQuantity = 0;
	};

	// Replace-in-full, the same shape S2C_ENCOUNTER_PROP_SYNC uses: one message
	// carries the whole current inventory, so a late joiner or a re-entering
	// session is correct without replaying every past give.
	struct S2C_INVENTORY_SNAPSHOT
	{
		std::uint32_t iRequestSequence = 0;
		std::vector<INVENTORY_ITEM_SNAPSHOT> Items;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_INVENTORY_SNAPSHOT& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_INVENTORY_SNAPSHOT& message);

	// Which quick slot (Item_1..4) the use came from is purely a Client-local
	// display/binding concern -- the Server only owns the inventory by item
	// ID, so this carries no slot index.
	struct C2S_USE_ITEM
	{
		std::uint32_t iRequestSequence = 0;
		std::string strItemId;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_USE_ITEM& message);
	bool Read_Message(
		CPacketReader& reader,
		C2S_USE_ITEM& message);

	// Debug Character Select Arena "되돌리기" -- despawns every world entity the
	// debug spawn buttons created in this session's room. No payload beyond the
	// sequence number; the Server owns which entities exist.
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

	// A stable Gameplay.world.json npc placement ID, not a display string, so it
	// shares the same bound C2S_USE_ITEM's itemId never needs.
	inline constexpr std::size_t MAX_NPC_PLACEMENT_ID_BYTES = 64;

	// Bern's confirm-to-enter window fires this when the player presses the
	// window's confirm button. strNpcPlacementId names which guide NPC the
	// player talked to (npc.bern.beda.guide / npc.bern.aylara); the Server
	// re-validates the requesting player is still near that NPC before
	// building the same world transfer the old automatic trigger used.
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

	// Offered to the one session standing in an interact-gated trigger box, and
	// withdrawn when it leaves. bAvailable false clears whatever the Client is
	// showing; the placement id is carried both ways so a stale offer can never
	// answer for a different box.
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

	// The player pressed the offered key. Same shape as C2S_CONFIRM_NPC_ENTRY:
	// the Server re-tests that this player is still inside that exact box before
	// running its action, so a replayed or forged request changes nothing.
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

	// Raid Clear screen's "돌아가기" button, Valtan Arena only. No target to
	// name -- the button has no proximity requirement -- so this is just a
	// request sequence, same shape as C2S_DESPAWN_ALL_WORLD_ENTITIES.
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

	// Same-room-only party invite: iTargetNetEntityId names another player
	// currently replicated in this same CGameRoom (right-clicked locally).
	// There is no cross-room player identity yet, so the Server rejects a
	// target it cannot find in its own m_Players by NetEntityId.
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

	// Sent only to the invited player. strFromNickname is display text only
	// (matches every other nameplate-facing nickname use), never an identity
	// lookup key -- the Respond message below answers by NetEntityId.
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
	};

	// Replace-in-full, the same shape S2C_ENCOUNTER_PROP_SYNC/
	// S2C_INVENTORY_SNAPSHOT use: broadcast to every current member whenever
	// membership changes, so a late reader never needs to replay past joins.
	struct S2C_PARTY_ROSTER
	{
		std::vector<PARTY_ROSTER_MEMBER> Members;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_PARTY_ROSTER& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_PARTY_ROSTER& message);

	// Same-room chat: the sender's own client already appends its typed line
	// to its local scrollback immediately (no round trip needed for that), so
	// this only exists to (a) let the Server relay it to everyone else in the
	// room and (b) drive the sender's own head bubble from the same broadcast
	// every other player's bubble uses, rather than a second local-only path.
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

	enum class PARTY_TRANSFER_RESULT : std::uint8_t
	{
		REJECTED_NOT_LEADER = 1,
		REJECTED_ROOM_FULL,
		REJECTED_MEMBER_UNAVAILABLE,
		REJECTED_ADMISSION_FAILED,
		REJECTED_OUTBOUND_BUSY
	};

	// Failure only: every member remains in the source world. Successful
	// admission uses S2C_ENTER_ACCEPTED, never a second success authority.
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
		KAKULSAYDON,
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

	// One authored world sequence instance started. The Server owns the trigger
	// entry that decided when; the Client resolves the stable instance ID
	// against the Area document it already loaded and plays only presentation.
	enum class WORLD_SEQUENCE_OPERATION : std::uint8_t { PLAY = 0, REPLAY = 1, STOP = 2, STOP_OWNER = 3, FINISH_OWNER = 4, END };
	enum class DEBUG_WORLD_PLAYBACK_OPERATION : std::uint8_t
	{
		PLAY_TRIGGER, REPLAY_TRIGGER, PLAY_SEQUENCE, REPLAY_SEQUENCE, STOP_SEQUENCE, END
	};
	enum class DEBUG_WORLD_PLAYBACK_RESULT : std::uint8_t
	{
		ACCEPTED, DISABLED, WRONG_WORLD, INVALID_TARGET, INVALID_PLAYER, ALREADY_USED,
		ACTION_REJECTED, STALE_REQUEST, END
	};
	struct C2S_DEBUG_WORLD_PLAYBACK
	{
		std::uint32_t iRequestSequence = 0;
		WORLD_ID eWorldId = WORLD_ID::END;
		DEBUG_WORLD_PLAYBACK_OPERATION eOperation = DEBUG_WORLD_PLAYBACK_OPERATION::END;
		std::string strTargetId;
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
		/* Multiplies the authored instance speed. 1 plays the sequence as
		authored; a pattern box may slow a roulette spin or a curtain drop. */
		float fPlaybackSpeed = 1.f;
		float fPositionOffsetX = 0.f;
		float fPositionOffsetY = 0.f;
		float fPositionOffsetZ = 0.f;
		std::uint32_t iDurationMs = 0u; // Zero uses the authored sequence lifetime.
		// Empty starts a sequence; otherwise apply its motion to this existing instance.
		std::string strTargetSequenceInstanceId;
		std::uint32_t iRunEpoch = 0u; // Zero is a non-audition map sequence.
		std::string strMemberId;
		std::string strCueId;
		std::string strOccurrenceId;
		std::string strTargetCueId;
		std::uint32_t iStartTick = 0u;
		std::uint32_t iServerTick = 0u;
		NET_ENTITY_ID iBossNetEntityId = INVALID_NET_ENTITY_ID;
		std::uint32_t iPatternSequence = 0u;
		// An owned WORLD box may replace its saved instance placement. Motions
		// keep their target cue's placement and therefore leave these at identity.
		bool bHasPlacement = false;
		float fWorldPositionX = 0.f, fWorldPositionY = 0.f, fWorldPositionZ = 0.f;
		float fWorldRotationXDegrees = 0.f, fWorldRotationYDegrees = 0.f, fWorldRotationZDegrees = 0.f;
		float fWorldScaleX = 1.f, fWorldScaleY = 1.f, fWorldScaleZ = 1.f;
	};
	bool Write_Message(CPacketWriter& writer, const S2C_WORLD_SEQUENCE_PLAY& message);
	bool Read_Message(CPacketReader& reader, S2C_WORLD_SEQUENCE_PLAY& message);

	// KoukuSaydon Boss Tool playback deliberately owns a separate command family
	// from Valtan. The scope is echoed on every response so a tool can never
	// mistake another world, placement, archetype, or data generation for Live.
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

	struct KOUKUSAYDON_PATTERN_AUDITION_SCOPE final
	{
		WORLD_ID eWorldId = WORLD_ID::END;
		std::string strEncounterId;
		std::string strGateId;
		std::string strBossPlacementId;
		std::string strBossArchetypeId;
		GameplayDataRevision ExpectedGameplayRevision{};
		std::uint32_t iExpectedSourceRevision = 0u;
	};

	struct C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST final
	{
		std::uint32_t iRequestSequence = 0u;
		KOUKUSAYDON_PATTERN_AUDITION_OPERATION eOperation =
			KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
		KOUKUSAYDON_PATTERN_AUDITION_SCOPE Scope;
		// Required only by PLAY_SELECTED. PLAY_ALL order is resolved exclusively
		// from the admitted Server Product sequence.
		std::string strPatternId;
		std::string strBundleId;
		std::uint32_t iExpectedRunEpoch = 0u;
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
		std::string strMemberId;
		NET_ENTITY_ID iBossNetEntityId = INVALID_NET_ENTITY_ID;
		std::string strPatternId;
		std::uint32_t iPatternSequence = 0u;
		std::uint32_t iStartTick = 0u;
		KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE eState = KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::PENDING;
	};
	// Persistent room-owned run state, broadcast to every client, including late joiners.
	struct S2C_KOUKUSAYDON_BUNDLE_STATE final
	{
		WORLD_ID eWorldId = WORLD_ID::END;
		std::string strEncounterId;
		std::string strBundleId; // Empty for the existing single-actor audition.
		std::uint32_t iRunEpoch = 0u;
		std::uint32_t iCommonStartTick = 0u;
		std::uint32_t iServerTick = 0u;
		KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE eState = KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::PENDING;
		GameplayDataRevision PinnedGameplayRevision{};
		std::uint32_t iPinnedSourceRevision = 0u;
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
}

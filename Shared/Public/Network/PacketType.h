#pragma once

#include <cstddef>
#include <cstdint>

namespace LostArk::Shared
{
	/* 53 adds generic boss response progress and threshold levels to the
	world-entity boss combat snapshot.
	52 adds the Server-active replacement definition revision to exact
	Valtan Restart requests and verdict echoes.
	49 requires stable-ID Valtan Complete Play to carry the exact active
	definition revision that the Client observed before requesting mutation.
	48 adds exact-occurrence Valtan Restart CAS identity and its pinned
	definition revision to the stable-ID audition wire.
	47 admits the KakulSaydon Arena as a Server-owned shared world.
	46 adds a reliable semantic combat-object presentation event. The Server
	transmits hit identity and world occurrence only; Client data resolves the
	actual Sound/Effect asset.
	45 adds the Raid Clear screen's C2S_RETURN_TO_BERN command.
	44 adds an immutable owner boss identity to world-entity spawn and a
	reliable typed death reason to world-entity despawn.
	43 adds live Product/Flow Next adoption with an observed predecessor.
	42 adds resetless Valtan next-pattern reservation and correlated control.
	41 combines same-room party invite/accept and roster sync with the
	expanded world destruction live-event bound. Each feature independently
	used 40 before integration, so neither v40 peer is wire-compatible.
	39 adds bounded Debug Valtan pattern-flow authoring playback.
	51 adds Server-owned Pattern bind and silence deadlines to player snapshots. */
	inline constexpr std::uint16_t NETWORK_PROTOCOL_VERSION = 54;

	enum class WORLD_ID : std::uint16_t
	{
		BERN = 1,
		VALTAN_ARENA = 2,
		TRAINING_GROUND = 3,
		CHARACTER_SELECT_ARENA = 4,
		KAKULSAYDON_ARENA = 5,
		END
	};

	[[nodiscard]]
	constexpr bool Is_Known_World_Id(const WORLD_ID worldId)
	{
		return WORLD_ID::BERN == worldId ||
			WORLD_ID::VALTAN_ARENA == worldId ||
			WORLD_ID::TRAINING_GROUND == worldId ||
			WORLD_ID::CHARACTER_SELECT_ARENA == worldId ||
			WORLD_ID::KAKULSAYDON_ARENA == worldId;
	}

	enum class CHARACTER_CLASS_ID : std::uint8_t
	{
		LANCE_MASTER = 0,
		GUNSLINGER = 1,
		SLAYER = 2,
		ARTIST = 3,
		DESTROYER = 4,
		DIMENSIONMASTER = 5,
		WARLORD = 6,
		END
	};

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
		// Two stages whose advance is a hit taken, not a press: the first stage
		// guards and the second is the counter it buys.
		COUNTER = 3,
		/* One clip that stands the player up out of KNOCKDOWN; startable in that
		action state only and never anywhere else. */
		STANDUP = 4,
		END
	};

	// A protocol value may be reserved before its runtime bundle exists. Only
	// classes accepted here may enter a world on the current build.
	[[nodiscard]]
	constexpr bool Is_Supported_Playable_Character_Class(
		const CHARACTER_CLASS_ID characterClass)
	{
		return CHARACTER_CLASS_ID::LANCE_MASTER == characterClass ||
			CHARACTER_CLASS_ID::GUNSLINGER == characterClass ||
			CHARACTER_CLASS_ID::SLAYER == characterClass ||
			CHARACTER_CLASS_ID::ARTIST == characterClass ||
			CHARACTER_CLASS_ID::DIMENSIONMASTER == characterClass ||
			CHARACTER_CLASS_ID::WARLORD == characterClass;
	}

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
		// Debug/Development-build test aid only: instantly zeroes the caster's own HP and
		// sets PLAYER_ACTION_STATE::DEAD, so a death-screen tester does not have to survive
		// a real hit. The wire type exists in every build; the Server body that would touch
		// gameplay state is compiled out in Release (see GameRoom.cpp's
		// Handle_DebugKillSelf), matching Evaluate_ValtanAudition's convention.
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

		// Repeatable encounter props such as the four pillars. One message
		// carries the whole current slot set, so a late joiner is correct
		// without replaying any past spawn or shatter.
		S2C_ENCOUNTER_PROP_SYNC,

		// Debug Valtan pattern audition. Both configurations know these types so
		// a Release Server answers an explicit rejection instead of closing the
		// session on an unknown frame; only a Debug Server ever accepts one.
		C2S_VALTAN_AUDITION_REQUEST,
		S2C_VALTAN_AUDITION_RESULT,

		// Debug-only inventory slice. The Server owns the truth; F1 Give Item
		// requests one catalog item and the Server answers with the full
		// current inventory, the same replace-in-full shape a late joiner or a
		// re-entering session receives on world entry.
		C2S_DEBUG_GIVE_ITEM,
		S2C_INVENTORY_SNAPSHOT,

		// A consumable used from a quick slot (Item_1..4). The Server validates
		// ownership + heal amount and answers with the player's next
		// S2C_WORLD_SNAPSHOT (HP) and an updated S2C_INVENTORY_SNAPSHOT
		// (decremented count) -- no separate result message.
		C2S_USE_ITEM,

		// Debug Character Select Arena "되돌리기" (revert) button -- despawns every
		// world entity the debug spawn buttons (Monster/Mid Boss/Valtan) created in
		// this room. The Server broadcasts one S2C_WORLD_ENTITY_DESPAWNED per removed
		// entity (already-wired machinery, previously only defined and never called)
		// -- no separate result message.
		C2S_DESPAWN_ALL_WORLD_ENTITIES,

		// Bern's two Valtan-entry guide NPCs (npc.bern.beda.guide, npc.bern.aylara)
		// right-click open a client-local confirm window; this fires only when the
		// player presses that window's confirm button. The Server re-validates
		// proximity to the named NPC and answers with the same
		// S2C_ENTER_ACCEPTED/S2C_ENTER_REJECTED world-transfer flow the old
		// automatic changeLevel triggerBox used -- no separate result message.
		C2S_CONFIRM_NPC_ENTRY,

		// Append-only room-owned combat-object lifecycle.
		S2C_COMBAT_OBJECT_SPAWNED,
		S2C_COMBAT_OBJECT_DESPAWNED,

		// Versioned lifecycle edges supplement the existing immediate audition
		// request/result pair; both tools can now correlate completion/abort.
		S2C_VALTAN_AUDITION_LIFECYCLE,

		// Debug-only immutable data revision two-phase commit. These packet types
		// remain known in Release so an unsupported request can be rejected by
		// typed policy instead of being treated as an unknown frame.
		C2S_DATA_REVISION_PREPARE_REQUEST,
		S2C_DATA_REVISION_PREPARE,
		C2S_DATA_REVISION_PREPARE_RESPONSE,
		S2C_DATA_REVISION_RESULT,

		// Debug decision observatory. Release keeps both packet identities known
		// and answers the query with REJECTED_RELEASE_BUILD rather than treating
		// the frame as an incompatible protocol command.
		C2S_VALTAN_DECISION_TRACE_QUERY,
		S2C_VALTAN_DECISION_TRACE_RESPONSE,

		// Debug Boss Tool ordered-flow audition. Release keeps the identities
		// known so policy rejection remains typed instead of closing a session.
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START,
		S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT,
		C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT,
		S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE,

		// Same-room party invite: right-click another player's NetEntityId in the
		// same CGameRoom, invite, target accepts/declines. No cross-room identity
		// exists yet (nickname is not a stable lookup key -- see CLAUDE.md), so
		// this stays scoped to players currently sharing one room.
		C2S_PARTY_INVITE,
		S2C_PARTY_INVITE_RECEIVED,
		C2S_PARTY_INVITE_RESPOND,
		S2C_PARTY_ROSTER,
		S2C_PARTY_TRANSFER_RESULT,

		// Raid Clear screen's own "돌아가기" (return) button, Valtan Arena only.
		// This append-only v45 identity is the reverse of C2S_CONFIRM_NPC_ENTRY.
		// It carries only a request sequence and receives the ordinary typed
		// S2C_ENTER_ACCEPTED/S2C_ENTER_REJECTED transfer result.
		C2S_RETURN_TO_BERN,

		// Reliable one-shot edge for a room-owned combat object's semantic hit.
		// Append-only: snapshot transform updates remain coalescible, this event
		// remains an ordering barrier and can outlive same-tick object despawn.
		S2C_COMBAT_OBJECT_PRESENTATION_EVENT,

		// Debug authoring navigation for the KakulSaydon vertical slice. Entry is
		// still a Server room transfer and stage movement still lands on an exact
		// authored PLAYER_SPAWN placement; the Client never sends a world transform.
		C2S_DEBUG_ENTER_KAKULSAYDON_ARENA,
		C2S_DEBUG_TELEPORT_TO_PLACEMENT,

		// Reliable one-shot edge telling every session in the room that an
		// authored world sequence instance started. The Server owns the trigger
		// entry that decides when; the Client owns only how it looks. Append-only
		// so an older payload is never reinterpreted as this identity.
		S2C_WORLD_SEQUENCE_PLAY
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
		case PACKET_TYPE::C2S_DEBUG_KILL_SELF:
		case PACKET_TYPE::C2S_CHANGE_CHARACTER_CLASS:
		case PACKET_TYPE::S2C_CHARACTER_CLASS_CHANGE_RESULT:
		case PACKET_TYPE::S2C_WORLD_SNAPSHOT:
		case PACKET_TYPE::C2S_CHAT:
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
		case PACKET_TYPE::C2S_DEBUG_ENTER_KAKULSAYDON_ARENA:
		case PACKET_TYPE::C2S_DEBUG_TELEPORT_TO_PLACEMENT:
		case PACKET_TYPE::C2S_PARTY_INVITE:
		case PACKET_TYPE::S2C_PARTY_INVITE_RECEIVED:
		case PACKET_TYPE::C2S_PARTY_INVITE_RESPOND:
		case PACKET_TYPE::S2C_PARTY_ROSTER:
		case PACKET_TYPE::S2C_PARTY_TRANSFER_RESULT:
		case PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY:
			return true;
		default:
			return  false;
		}
	}

	//inline constexpr인 이유
	//inline : 이 헤더를 여러 .cpp가 include해도 동일한 변수 정의로 취급한다.
	//constexpr : 컴파일 타임 시간 상수
	inline constexpr std::size_t MAX_NICKNAME_BYTES = 32;

	// Same as Character Select Arena's own room cap (see
	// Run-CharacterSelectIsolationHarness.ps1's 4/4 ROOM_FULL contract).
	inline constexpr std::size_t MAX_PARTY_MEMBERS = 4;

	// Network players admitted to one Valtan raid room. Party ownership remains
	// a four-member contract; the remaining raid seats may be occupied by a
	// second party or, later, Server-owned raid AI without widening party wire
	// messages or nickname identity semantics.
	inline constexpr std::size_t MAX_VALTAN_RAID_PLAYERS = 8;

	// Matches CChatWindowView::INPUT_BUFFER_SIZE (including the terminator),
	// so a locally-typeable line always round-trips.
	inline constexpr std::size_t MAX_CHAT_TEXT_BYTES = 256;
	// Matches the stable ID budget the authored world sequence document uses.
	inline constexpr std::size_t MAX_SEQUENCE_INSTANCE_ID_BYTES = 128;
}

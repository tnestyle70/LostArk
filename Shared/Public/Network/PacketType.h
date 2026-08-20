#pragma once

#include <cstddef>
#include <cstdint>

namespace LostArk::Shared
{
	inline constexpr std::uint16_t NETWORK_PROTOCOL_VERSION = 29;

	enum class WORLD_ID : std::uint16_t
	{
		BERN = 1,
		VALTAN_ARENA = 2,
		TRAINING_GROUND = 3,
		CHARACTER_SELECT_ARENA = 4,
		END
	};

	[[nodiscard]]
	constexpr bool Is_Known_World_Id(const WORLD_ID worldId)
	{
		return WORLD_ID::BERN == worldId ||
			WORLD_ID::VALTAN_ARENA == worldId ||
			WORLD_ID::TRAINING_GROUND == worldId ||
			WORLD_ID::CHARACTER_SELECT_ARENA == worldId;
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
		C2S_CONFIRM_NPC_ENTRY
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
		case PACKET_TYPE::C2S_SPAWN_WORLD_ENTITY:
		case PACKET_TYPE::S2C_WORLD_ENTITY_SPAWN_RESULT:
		case PACKET_TYPE::C2S_MOVE:
		case PACKET_TYPE::C2S_USE_SKILL:
		case PACKET_TYPE::C2S_RELEASE_SKILL:
		case PACKET_TYPE::C2S_UPDATE_SKILL_AIM:
		case PACKET_TYPE::C2S_USE_ESTHER_SKILL:
		case PACKET_TYPE::C2S_REVIVE_PLAYER:
		case PACKET_TYPE::C2S_CHANGE_CHARACTER_CLASS:
		case PACKET_TYPE::S2C_CHARACTER_CLASS_CHANGE_RESULT:
		case PACKET_TYPE::S2C_WORLD_SNAPSHOT:
		case PACKET_TYPE::C2S_CHAT:
		case PACKET_TYPE::S2C_CHAT:
		case PACKET_TYPE::S2C_PLAYER_DESPAWNED:
		case PACKET_TYPE::S2C_WORLD_ENTITY_DESPAWNED:
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
			return true;
		default:
			return  false;
		}
	}

	//inline constexpr인 이유
	//inline : 이 헤더를 여러 .cpp가 include해도 동일한 변수 정의로 취급한다.
	//constexpr : 컴파일 타임 시간 상수
	inline constexpr std::size_t MAX_NICKNAME_BYTES = 32;
}

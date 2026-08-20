#include "Network/PacketMessages.h"

#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <utility>

//writer가 일부만 기록한 뒤 실패하지 않도록 snapshot의 모든 state를 먼저 검증한다.
namespace
{
	bool Is_Utf8Continuation(const std::uint8_t value) noexcept
	{
		return 0x80u == (value & 0xC0u);
	}

	bool Is_AsciiWhitespace(const char value) noexcept
	{
		return ' ' == value || '\t' == value || '\n' == value ||
			'\r' == value || '\f' == value || '\v' == value;
	}

    //유효한 애니메이션인지 검증
    bool Is_Valid_Locomotion(
        LostArk::Shared::PLAYER_LOCOMOTION_STATE state)
    {
        return static_cast<std::uint8_t>(state) <
            static_cast<std::uint8_t>(
                LostArk::Shared::PLAYER_LOCOMOTION_STATE::END);
    }

	bool Is_Valid_PlayerAction(
		const LostArk::Shared::PLAYER_ACTION_STATE state)
	{
		return static_cast<std::uint8_t>(state) <
			static_cast<std::uint8_t>(
				LostArk::Shared::PLAYER_ACTION_STATE::END);
	}

	bool Is_Valid_Stance(
		const LostArk::Shared::PLAYER_STANCE_ID stance)
	{
		return static_cast<std::uint8_t>(stance) <
			static_cast<std::uint8_t>(
				LostArk::Shared::PLAYER_STANCE_ID::END);
	}

	bool Is_Valid_Cooldowns(
		const std::vector<LostArk::Shared::SKILL_COOLDOWN_SNAPSHOT>& cooldowns)
	{
		if (cooldowns.size() > LostArk::Shared::MAX_PLAYER_COOLDOWNS)
			return false;
		for (std::size_t i = 0; i < cooldowns.size(); ++i)
		{
			if (cooldowns[i].iSkillId == LostArk::Shared::INVALID_SKILL_ID ||
				0 == cooldowns[i].iCooldownEndTick)
			{
				return false;
			}
			for (std::size_t j = i + 1; j < cooldowns.size(); ++j)
			{
				if (cooldowns[i].iSkillId == cooldowns[j].iSkillId)
					return false;
			}
		}
		return true;
	}
    //유효한 플레이어 스냅샷인지 검증 - netentityid, position x y z, locomotion state
    bool Is_Valid_PlayerSnapshot(
        const LostArk::Shared::PLAYER_SNAPSHOT& snapshot)
    {
        return
            snapshot.iNetEntityId != LostArk::Shared::INVALID_NET_ENTITY_ID &&
			LostArk::Shared::Is_Supported_Playable_Character_Class(
				snapshot.eCharacterClass) &&
            std::isfinite(snapshot.fPositionX) &&
            std::isfinite(snapshot.fPositionY) &&
            std::isfinite(snapshot.fPositionZ) &&
            std::isfinite(snapshot.fYawDegrees) &&
            Is_Valid_Locomotion(snapshot.eLocomotionState) &&
			Is_Valid_PlayerAction(snapshot.eAction) &&
			Is_Valid_Stance(snapshot.eStance) &&
			0 != snapshot.iMaximumHp &&
			snapshot.iCurrentHp <= snapshot.iMaximumHp &&
			0 != snapshot.iMaximumResource &&
			snapshot.iCurrentResource <= snapshot.iMaximumResource &&
			snapshot.iCurrentIdentity <= snapshot.iMaximumIdentity &&
			Is_Valid_Cooldowns(snapshot.Cooldowns) &&
			snapshot.iComboStage <= LostArk::Shared::MAX_COMBO_STAGES &&
			(0 == snapshot.iComboStage ||
				LostArk::Shared::PLAYER_ACTION_STATE::SKILL == snapshot.eAction) &&
			((LostArk::Shared::PLAYER_ACTION_STATE::SKILL == snapshot.eAction &&
				snapshot.iSkillId != LostArk::Shared::INVALID_SKILL_ID &&
				0 != snapshot.iActionStartTick) ||
			 (LostArk::Shared::PLAYER_ACTION_STATE::TRIGGER_MOVE == snapshot.eAction &&
				snapshot.iSkillId == LostArk::Shared::INVALID_SKILL_ID &&
				0 != snapshot.iActionStartTick) ||
			 /* A fall is timed: the client seeks the descent from this tick when
			 it joins late, so a FALLING snapshot without one is malformed. */
			 (LostArk::Shared::PLAYER_ACTION_STATE::FALLING == snapshot.eAction &&
				snapshot.iSkillId == LostArk::Shared::INVALID_SKILL_ID &&
				0 != snapshot.iActionStartTick) ||
			 (LostArk::Shared::PLAYER_ACTION_STATE::KNOCKDOWN == snapshot.eAction &&
				snapshot.iSkillId == LostArk::Shared::INVALID_SKILL_ID &&
				0 != snapshot.iActionStartTick) ||
			 ((LostArk::Shared::PLAYER_ACTION_STATE::SKILL != snapshot.eAction &&
				LostArk::Shared::PLAYER_ACTION_STATE::TRIGGER_MOVE != snapshot.eAction &&
				LostArk::Shared::PLAYER_ACTION_STATE::FALLING != snapshot.eAction &&
				LostArk::Shared::PLAYER_ACTION_STATE::KNOCKDOWN != snapshot.eAction) &&
				snapshot.iSkillId == LostArk::Shared::INVALID_SKILL_ID));
    }

	bool Is_Valid_StableId(const std::string& value, const bool allowEmpty)
	{
		return (allowEmpty && value.empty()) ||
			(!value.empty() && value.size() <=
				LostArk::Shared::MAX_STABLE_NETWORK_ID_BYTES &&
				std::all_of(value.begin(), value.end(), [](const unsigned char character)
				{
					return 0 != std::isalnum(character) || character == '_' ||
						character == '-' || character == '.';
				}));
	}

	// Same character class as Is_Valid_StableId, but capped at the item ID's
	// own, tighter wire bound instead of the general stable-ID bound.
	bool Is_Valid_ItemId(const std::string& value)
	{
		return !value.empty() &&
			value.size() <= LostArk::Shared::MAX_ITEM_ID_BYTES &&
			std::all_of(value.begin(), value.end(), [](const unsigned char character)
			{
				return 0 != std::isalnum(character) || character == '_' ||
					character == '-' || character == '.';
			});
	}

	bool Is_Valid_InventoryItems(
		const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>& items)
	{
		for (std::size_t index = 0; index < items.size(); ++index)
		{
			const LostArk::Shared::INVENTORY_ITEM_SNAPSHOT& item = items[index];
			if (!Is_Valid_ItemId(item.strItemId) || 0u == item.iQuantity)
				return false;
			for (std::size_t other = index + 1; other < items.size(); ++other)
			{
				if (items[other].strItemId == item.strItemId)
					return false;
			}
		}
		return true;
	}

	bool Is_Valid_WorldEntityKind(
		const LostArk::Shared::WORLD_ENTITY_KIND kind)
	{
		return static_cast<std::uint8_t>(kind) <
			static_cast<std::uint8_t>(LostArk::Shared::WORLD_ENTITY_KIND::END);
	}

	bool Is_Valid_WorldEntityAction(
		const LostArk::Shared::WORLD_ENTITY_ACTION action)
	{
		return static_cast<std::uint8_t>(action) <
			static_cast<std::uint8_t>(LostArk::Shared::WORLD_ENTITY_ACTION::END);
	}

	bool Is_Valid_WorldEntitySnapshot(
		const LostArk::Shared::WORLD_ENTITY_SNAPSHOT& snapshot)
	{
		return snapshot.iNetEntityId != LostArk::Shared::INVALID_NET_ENTITY_ID &&
			Is_Valid_WorldEntityAction(snapshot.eAction) &&
			Is_Valid_StableId(snapshot.strPatternId, true) &&
			Is_Valid_StableId(snapshot.strActionId, true) &&
			std::isfinite(snapshot.fPositionX) &&
			std::isfinite(snapshot.fPositionY) &&
			std::isfinite(snapshot.fPositionZ) &&
			std::isfinite(snapshot.fYawDegrees) &&
			0 != snapshot.iMaximumHp &&
			snapshot.iCurrentHp <= snapshot.iMaximumHp &&
			0 != snapshot.iPhase;
	}

	// A zero amount is not a hit, so it must not reach presentation as one; the
	// server clamps every resolved hit to at least 1.
	bool Is_Valid_DamageEvent(
		const LostArk::Shared::DAMAGE_EVENT& damage)
	{
		return
			damage.iTargetNetEntityId !=
				LostArk::Shared::INVALID_NET_ENTITY_ID &&
			0 != damage.iAmount &&
			std::isfinite(damage.fPositionX) &&
			std::isfinite(damage.fPositionY) &&
			std::isfinite(damage.fPositionZ);
	}

	bool Is_Valid_CombatRuntimeRevision(const std::string& revision)
	{
		if (LostArk::Shared::MAX_COMBAT_RUNTIME_REVISION_BYTES !=
			revision.size())
		{
			return false;
		}

		bool hasNonZeroDigit = false;
		for (const unsigned char character : revision)
		{
			if (!((character >= '0' && character <= '9') ||
				(character >= 'a' && character <= 'f')))
			{
				return false;
			}
			hasNonZeroDigit = hasNonZeroDigit || character != '0';
		}
		return hasNonZeroDigit;
	}

	bool Is_Valid_DestructionState(
		const LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_STATE state)
	{
		return static_cast<std::uint8_t>(state) <
			static_cast<std::uint8_t>(
				LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_STATE::END);
	}

	bool Is_Valid_DestructionStateWire(
		const LostArk::Shared::WORLD_DESTRUCTION_STATE_WIRE& state)
	{
		return Is_Valid_StableId(state.strGroupId, false) &&
			Is_Valid_DestructionState(state.eState) &&
			0 != state.iStateVersion &&
			0 != state.iStateStartTick &&
			((LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_STATE::BREAKING ==
				state.eState && 0 != state.iCommitTick) ||
			 (LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_STATE::BREAKING !=
				state.eState && 0 == state.iCommitTick));
	}

	bool Are_DestructionStatesCanonical(
		const std::vector<LostArk::Shared::WORLD_DESTRUCTION_STATE_WIRE>& states)
	{
		for (std::size_t i = 0; i < states.size(); ++i)
		{
			if (!Is_Valid_DestructionStateWire(states[i]) ||
				(0 != i &&
					!(states[i - 1].strGroupId < states[i].strGroupId)))
			{
				return false;
			}
		}
		return true;
	}

	bool Is_Valid_DestructionEventWire(
		const LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE& event)
	{
		if (0 == event.iEventSequence ||
			!Is_Valid_StableId(event.strGroupId, false) ||
			!Is_Valid_StableId(event.strMutationId, false) ||
			!Is_Valid_StableId(event.strBindingId, false) ||
			0 == event.iPatternSequence ||
			0 == event.iSourceNetEntityId ||
			0 == event.iServerTick ||
			!std::isfinite(event.fImpactOriginX) ||
			!std::isfinite(event.fImpactOriginY) ||
			!std::isfinite(event.fImpactOriginZ) ||
			!std::isfinite(event.fImpactDirectionX) ||
			!std::isfinite(event.fImpactDirectionY) ||
			!std::isfinite(event.fImpactDirectionZ))
		{
			return false;
		}

		const float directionLengthSquared =
			event.fImpactDirectionX * event.fImpactDirectionX +
			event.fImpactDirectionY * event.fImpactDirectionY +
			event.fImpactDirectionZ * event.fImpactDirectionZ;
		return directionLengthSquared > 0.f &&
			std::fabs(directionLengthSquared - 1.f) <= 0.001f;
	}

	bool Are_DestructionEventsCanonical(
		const std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>& events)
	{
		for (std::size_t i = 0; i < events.size(); ++i)
		{
			// Event sequences never wrap within an encounter epoch. The room must
			// reset the epoch before UINT64_MAX can be reused, so raw ascending
			// order is the canonical zero-reuse order on this wire.
			if (!Is_Valid_DestructionEventWire(events[i]) ||
				(0 != i &&
					!(events[i - 1].iEventSequence < events[i].iEventSequence)))
			{
				return false;
			}
		}
		return true;
	}

	void Write_U64(
		LostArk::Shared::CPacketWriter& writer,
		const std::uint64_t value)
	{
		writer.Write_U32(static_cast<std::uint32_t>(value));
		writer.Write_U32(static_cast<std::uint32_t>(value >> 32));
	}

	bool Read_U64(
		LostArk::Shared::CPacketReader& reader,
		std::uint64_t& value)
	{
		std::uint32_t low = 0;
		std::uint32_t high = 0;
		if (!reader.Read_U32(low) || !reader.Read_U32(high))
			return false;
		value = static_cast<std::uint64_t>(low) |
			(static_cast<std::uint64_t>(high) << 32);
		return true;
	}

	bool Write_DestructionStateWire(
		LostArk::Shared::CPacketWriter& writer,
		const LostArk::Shared::WORLD_DESTRUCTION_STATE_WIRE& state)
	{
		if (!writer.Write_String(
			state.strGroupId,
			LostArk::Shared::MAX_STABLE_NETWORK_ID_BYTES))
		{
			return false;
		}
		writer.Write_U8(static_cast<std::uint8_t>(state.eState));
		writer.Write_U32(state.iStateVersion);
		writer.Write_U32(state.iStateStartTick);
		writer.Write_U32(state.iCommitTick);
		return true;
	}

	bool Read_DestructionStateWire(
		LostArk::Shared::CPacketReader& reader,
		LostArk::Shared::WORLD_DESTRUCTION_STATE_WIRE& state)
	{
		LostArk::Shared::WORLD_DESTRUCTION_STATE_WIRE decoded{};
		std::uint8_t rawState = 0;
		if (!reader.Read_String(
				decoded.strGroupId,
				LostArk::Shared::MAX_STABLE_NETWORK_ID_BYTES) ||
			!reader.Read_U8(rawState) ||
			!reader.Read_U32(decoded.iStateVersion) ||
			!reader.Read_U32(decoded.iStateStartTick) ||
			!reader.Read_U32(decoded.iCommitTick))
		{
			return false;
		}
		decoded.eState = static_cast<
			LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_STATE>(rawState);
		if (!Is_Valid_DestructionStateWire(decoded))
			return false;
		state = std::move(decoded);
		return true;
	}

	bool Write_DestructionEventWire(
		LostArk::Shared::CPacketWriter& writer,
		const LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE& event)
	{
		Write_U64(writer, event.iEventSequence);
		if (!writer.Write_String(
				event.strGroupId,
				LostArk::Shared::MAX_STABLE_NETWORK_ID_BYTES) ||
			!writer.Write_String(
				event.strMutationId,
				LostArk::Shared::MAX_STABLE_NETWORK_ID_BYTES) ||
			!writer.Write_String(
				event.strBindingId,
				LostArk::Shared::MAX_STABLE_NETWORK_ID_BYTES))
		{
			return false;
		}
		writer.Write_U32(event.iPatternSequence);
		Write_U64(writer, event.iSourceNetEntityId);
		writer.Write_U32(event.iServerTick);
		writer.Write_F32(event.fImpactOriginX);
		writer.Write_F32(event.fImpactOriginY);
		writer.Write_F32(event.fImpactOriginZ);
		writer.Write_F32(event.fImpactDirectionX);
		writer.Write_F32(event.fImpactDirectionY);
		writer.Write_F32(event.fImpactDirectionZ);
		writer.Write_U32(event.iRandomSeed);
		return true;
	}

	bool Read_DestructionEventWire(
		LostArk::Shared::CPacketReader& reader,
		LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE& event)
	{
		LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE decoded{};
		if (!Read_U64(reader, decoded.iEventSequence) ||
			!reader.Read_String(
				decoded.strGroupId,
				LostArk::Shared::MAX_STABLE_NETWORK_ID_BYTES) ||
			!reader.Read_String(
				decoded.strMutationId,
				LostArk::Shared::MAX_STABLE_NETWORK_ID_BYTES) ||
			!reader.Read_String(
				decoded.strBindingId,
				LostArk::Shared::MAX_STABLE_NETWORK_ID_BYTES) ||
			!reader.Read_U32(decoded.iPatternSequence) ||
			!Read_U64(reader, decoded.iSourceNetEntityId) ||
			!reader.Read_U32(decoded.iServerTick) ||
			!reader.Read_F32(decoded.fImpactOriginX) ||
			!reader.Read_F32(decoded.fImpactOriginY) ||
			!reader.Read_F32(decoded.fImpactOriginZ) ||
			!reader.Read_F32(decoded.fImpactDirectionX) ||
			!reader.Read_F32(decoded.fImpactDirectionY) ||
			!reader.Read_F32(decoded.fImpactDirectionZ) ||
			!reader.Read_U32(decoded.iRandomSeed) ||
			!Is_Valid_DestructionEventWire(decoded))
		{
			return false;
		}
		event = std::move(decoded);
		return true;
	}

	bool Is_Valid_EncounterPropSlots(
		const std::vector<LostArk::Shared::ENCOUNTER_PROP_SLOT_WIRE>& slots)
	{
		// Slot IDs arrive in one canonical ascending order so two servers cannot
		// disagree about the same set, and duplicates are a hard reject.
		for (std::size_t index = 0; index < slots.size(); ++index)
		{
			const LostArk::Shared::ENCOUNTER_PROP_SLOT_WIRE& slot = slots[index];
			if (slot.strSlotId.empty() ||
				slot.strSlotId.size() >
					LostArk::Shared::MAX_STABLE_NETWORK_ID_BYTES ||
				slot.eState >= LostArk::Shared::ENCOUNTER_PROP_STATE::END ||
				0u == slot.iStateVersion)
			{
				return false;
			}
			if (index > 0 && slots[index - 1].strSlotId >= slot.strSlotId)
				return false;
		}
		return true;
	}

	void Write_DestructionDiagnostics(
		LostArk::Shared::CPacketWriter& writer,
		const LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS& diagnostics)
	{
		writer.Write_U32(diagnostics.iActiveWallCollisionCount);
		writer.Write_U32(diagnostics.iActiveNavBlockerRegionCount);
		Write_U64(writer, diagnostics.iNavigationRevision);
		Write_U64(writer, diagnostics.iLastEventSequence);
	}

	bool Read_DestructionDiagnostics(
		LostArk::Shared::CPacketReader& reader,
		LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS& diagnostics)
	{
		LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS decoded{};
		if (!reader.Read_U32(decoded.iActiveWallCollisionCount) ||
			!reader.Read_U32(decoded.iActiveNavBlockerRegionCount) ||
			!Read_U64(reader, decoded.iNavigationRevision) ||
			!Read_U64(reader, decoded.iLastEventSequence))
		{
			return false;
		}
		diagnostics = decoded;
		return true;
	}
}

bool LostArk::Shared::Is_Valid_PlayerNickname(
	const std::string_view nickname) noexcept
{
	if (nickname.empty() || nickname.size() > MAX_NICKNAME_BYTES ||
		Is_AsciiWhitespace(nickname.front()) ||
		Is_AsciiWhitespace(nickname.back()))
	{
		return false;
	}

	const auto* bytes = reinterpret_cast<const std::uint8_t*>(
		nickname.data());
	std::size_t offset = 0u;
	while (offset < nickname.size())
	{
		const std::uint8_t first = bytes[offset];
		std::uint32_t codePoint = 0u;
		std::size_t length = 0u;

		if (first <= 0x7Fu)
		{
			codePoint = first;
			length = 1u;
		}
		else if (first >= 0xC2u && first <= 0xDFu)
		{
			if (offset + 1u >= nickname.size() ||
				!Is_Utf8Continuation(bytes[offset + 1u]))
			{
				return false;
			}
			codePoint = ((first & 0x1Fu) << 6u) |
				(bytes[offset + 1u] & 0x3Fu);
			length = 2u;
		}
		else if (first >= 0xE0u && first <= 0xEFu)
		{
			if (offset + 2u >= nickname.size() ||
				!Is_Utf8Continuation(bytes[offset + 1u]) ||
				!Is_Utf8Continuation(bytes[offset + 2u]) ||
				(0xE0u == first && bytes[offset + 1u] < 0xA0u) ||
				(0xEDu == first && bytes[offset + 1u] > 0x9Fu))
			{
				return false;
			}
			codePoint = ((first & 0x0Fu) << 12u) |
				((bytes[offset + 1u] & 0x3Fu) << 6u) |
				(bytes[offset + 2u] & 0x3Fu);
			length = 3u;
		}
		else if (first >= 0xF0u && first <= 0xF4u)
		{
			if (offset + 3u >= nickname.size() ||
				!Is_Utf8Continuation(bytes[offset + 1u]) ||
				!Is_Utf8Continuation(bytes[offset + 2u]) ||
				!Is_Utf8Continuation(bytes[offset + 3u]) ||
				(0xF0u == first && bytes[offset + 1u] < 0x90u) ||
				(0xF4u == first && bytes[offset + 1u] > 0x8Fu))
			{
				return false;
			}
			codePoint = ((first & 0x07u) << 18u) |
				((bytes[offset + 1u] & 0x3Fu) << 12u) |
				((bytes[offset + 2u] & 0x3Fu) << 6u) |
				(bytes[offset + 3u] & 0x3Fu);
			length = 4u;
		}
		else
		{
			return false;
		}

		if (0u == codePoint || codePoint <= 0x1Fu ||
			(codePoint >= 0x7Fu && codePoint <= 0x9Fu) ||
			(codePoint >= 0xD800u && codePoint <= 0xDFFFu) ||
			codePoint > 0x10FFFFu)
		{
			return false;
		}
		offset += length;
	}
	return true;
}

bool LostArk::Shared::Write_Message(CPacketWriter& writer, const C2S_ENTER_WORLD& message)
{
	if (NETWORK_PROTOCOL_VERSION != message.iProtocolVersion ||
		!Is_Known_World_Id(message.eWorldId))
	{
		return false;
	}

    //character class write
    const std::uint8_t rawCharacterClass =
        static_cast<std::uint8_t>(
            message.eCharacterClass);

    if (rawCharacterClass >= static_cast<std::uint8_t>(
        CHARACTER_CLASS_ID::END))
        return false;

	if (!Is_Valid_PlayerNickname(message.strNickName))
        return false;

	writer.Write_U16(message.iProtocolVersion);
	writer.Write_U16(static_cast<std::uint16_t>(message.eWorldId));

	//character class write
	writer.Write_U8(rawCharacterClass);

    //nickname write
    return writer.Write_String(
        message.strNickName,
        MAX_NICKNAME_BYTES);
}

bool LostArk::Shared::Read_Message(CPacketReader& reader, C2S_ENTER_WORLD& message)
{
	std::uint16_t protocolVersion = {};
	std::uint16_t rawWorldId = {};
	std::uint8_t rawCharacterClass = {};
	std::string nickName;

	if (!reader.Read_U16(protocolVersion) ||
		!reader.Read_U16(rawWorldId) ||
		NETWORK_PROTOCOL_VERSION != protocolVersion ||
		!Is_Known_World_Id(static_cast<WORLD_ID>(rawWorldId)))
	{
		return false;
	}

    //character class 예외처리
    if (!reader.Read_U8(rawCharacterClass))
        return false;

    if (rawCharacterClass >= static_cast<std::uint8_t>(
        CHARACTER_CLASS_ID::END))
        return false;

    //nickname read
    if (!reader.Read_String(
        nickName,
        MAX_NICKNAME_BYTES))
        return false;

	if (!Is_Valid_PlayerNickname(nickName))
        return false;

	C2S_ENTER_WORLD decoded{};
	decoded.iProtocolVersion = protocolVersion;
	decoded.eWorldId = static_cast<WORLD_ID>(rawWorldId);

    decoded.eCharacterClass =
        static_cast<CHARACTER_CLASS_ID>(
            rawCharacterClass);

    decoded.strNickName = std::move(nickName);

    //const라서 대입이 안 되는 상황?
    message = std::move(decoded);

    return true;
}

bool LostArk::Shared::Write_Message(CPacketWriter& writer, 
    const S2C_ENTER_ACCEPTED& message)
{
	if (NETWORK_PROTOCOL_VERSION != message.iProtocolVersion ||
		!Is_Known_World_Id(message.eWorldId))
	{
		return false;
	}

    //playerid가 유효한지 검사
    if (message.iPlayerId == INVALID_PLAYER_ID)
        return false;

    //netid가 유효한지 검사
    if (message.iNetEntityId == INVALID_NET_ENTITY_ID)
        return false;

	writer.Write_U16(message.iProtocolVersion);
	writer.Write_U16(static_cast<std::uint16_t>(message.eWorldId));

	//playerid, netentityid를 u32로 기록
    writer.Write_U32(message.iPlayerId);
    writer.Write_U32(message.iNetEntityId);

    return true;
}

bool LostArk::Shared::Read_Message(CPacketReader& reader,
   S2C_ENTER_ACCEPTED& message)
{
	std::uint16_t protocolVersion = {};
	std::uint16_t rawWorldId = {};
	PLAYER_ID playerId =
        INVALID_PLAYER_ID;

    NET_ENTITY_ID netEntityId =
        INVALID_NET_ENTITY_ID;

	if (!reader.Read_U16(protocolVersion) ||
		!reader.Read_U16(rawWorldId) ||
		NETWORK_PROTOCOL_VERSION != protocolVersion ||
		!Is_Known_World_Id(static_cast<WORLD_ID>(rawWorldId)))
	{
		return false;
	}

	if (!reader.Read_U32(playerId))
        return false;
    if (!reader.Read_U32(netEntityId))
        return false;

    if (playerId == 0 || netEntityId == 0)
        return false;

	S2C_ENTER_ACCEPTED decoded {};

	decoded.iProtocolVersion = protocolVersion;
	decoded.eWorldId = static_cast<WORLD_ID>(rawWorldId);
	decoded.iPlayerId = playerId;
    decoded.iNetEntityId = netEntityId;
   
    message = decoded;

    return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_ENTER_REJECTED& message)
{
	if (NETWORK_PROTOCOL_VERSION != message.iProtocolVersion ||
		!Is_Known_World_Id(message.eWorldId) ||
		ENTER_WORLD_REJECTION_REASON::ROOM_FULL != message.eReason)
	{
		return false;
	}

	writer.Write_U16(message.iProtocolVersion);
	writer.Write_U16(static_cast<std::uint16_t>(message.eWorldId));
	writer.Write_U8(static_cast<std::uint8_t>(message.eReason));
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_ENTER_REJECTED& message)
{
	std::uint16_t protocolVersion = 0;
	std::uint16_t rawWorldId = 0;
	std::uint8_t rawReason = 0;
	if (!reader.Read_U16(protocolVersion) ||
		!reader.Read_U16(rawWorldId) ||
		!reader.Read_U8(rawReason) ||
		NETWORK_PROTOCOL_VERSION != protocolVersion ||
		!Is_Known_World_Id(static_cast<WORLD_ID>(rawWorldId)) ||
		static_cast<std::uint8_t>(ENTER_WORLD_REJECTION_REASON::ROOM_FULL) !=
			rawReason)
	{
		return false;
	}

	S2C_ENTER_REJECTED decoded{};
	decoded.iProtocolVersion = protocolVersion;
	decoded.eWorldId = static_cast<WORLD_ID>(rawWorldId);
	decoded.eReason = static_cast<ENTER_WORLD_REJECTION_REASON>(rawReason);
	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(CPacketWriter& writer, 
    const S2C_PLAYER_SPAWNED& spawned)
{
    //playerid가 유효한지 검사
    if (spawned.iPlayerId == INVALID_PLAYER_ID)
        return false;

    //netid가 유효한지 검사
    if (spawned.iNetEntityId == INVALID_NET_ENTITY_ID)
        return false;

    //character class가 end인지 검사
    const std::uint8_t rawCharacterClass =
        static_cast<std::uint8_t>(spawned.eCharacterClass);

    if (rawCharacterClass >=
        static_cast<std::uint8_t>(CHARACTER_CLASS_ID::END))
        return false;

	if (!Is_Valid_PlayerNickname(spawned.strNickName))
        return false;

    //position X/Y/Z가 finite인지 검사
    if (!std::isfinite(spawned.fPositionX) ||
        !std::isfinite(spawned.fPositionY) ||
        !std::isfinite(spawned.fPositionZ))
        return false;

    //yawDegrees가 finite인지 검사
    if (!std::isfinite(spawned.fYawDegrees))
        return false;

    //playerid, net entity, character class, nickname
    //position x y z, yawdegrees를 u32로 기록
    writer.Write_U32(spawned.iPlayerId);
    writer.Write_U32(spawned.iNetEntityId);

    writer.Write_U8(rawCharacterClass);

    if (!writer.Write_String(
        spawned.strNickName,
        MAX_NICKNAME_BYTES))
    {
        return false;
    }
    
    writer.Write_F32(spawned.fPositionX);
    writer.Write_F32(spawned.fPositionY);
    writer.Write_F32(spawned.fPositionZ);
    writer.Write_F32(spawned.fYawDegrees);

    return true;
}

bool LostArk::Shared::Read_Message(CPacketReader& reader, 
    S2C_PLAYER_SPAWNED& spawned)
{
    PLAYER_ID iPlayerId =
        INVALID_PLAYER_ID;

    NET_ENTITY_ID iNetEntityId =
        INVALID_NET_ENTITY_ID;

    std::uint8_t rawCharacterClass = {};
    std::string nickName;

    float positionX = 0.f;
    float positionY = 0.f;
    float positionZ = 0.f;
    float yawDegrees = 0.f;

    if (!reader.Read_U32(iPlayerId))
        return false;

    if (!reader.Read_U32(iNetEntityId))
        return false;

    if (!reader.Read_U8(rawCharacterClass))
        return false;

    if (!reader.Read_String(
        nickName,
        MAX_NICKNAME_BYTES))
    {
        return false;
    }

    if (!reader.Read_F32(positionX))
        return false;

    if (!reader.Read_F32(positionY))
        return false;

    if (!reader.Read_F32(positionZ))
        return false;

    if (!reader.Read_F32(yawDegrees))
        return false;

    if (iPlayerId == INVALID_PLAYER_ID)
        return false;

    if (iNetEntityId == INVALID_NET_ENTITY_ID)
        return false;

    if (rawCharacterClass >=
        static_cast<std::uint8_t>(
            CHARACTER_CLASS_ID::END))
    {
        return false;
    }

	if (!Is_Valid_PlayerNickname(nickName))
        return false;

    if (!std::isfinite(positionX) ||
        !std::isfinite(positionY) ||
        !std::isfinite(positionZ) ||
        !std::isfinite(yawDegrees))
    {
        return false;
    }



    S2C_PLAYER_SPAWNED decoded{};

    decoded.iPlayerId = iPlayerId;
    decoded.iNetEntityId = iNetEntityId;

    decoded.eCharacterClass = static_cast<CHARACTER_CLASS_ID>(
        rawCharacterClass);

    decoded.strNickName =
        std::move(nickName);

    decoded.fPositionX = positionX;
    decoded.fPositionY = positionY;
    decoded.fPositionZ = positionZ;
    decoded.fYawDegrees = yawDegrees;

    spawned = std::move(decoded);

    return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_WORLD_ENTITY_SPAWNED& spawned)
{
	if (spawned.iNetEntityId == INVALID_NET_ENTITY_ID ||
		!Is_Valid_WorldEntityKind(spawned.eKind) ||
		!Is_Valid_StableId(spawned.strArchetypeId, false) ||
		!Is_Valid_StableId(spawned.strEncounterId, true) ||
		!Is_Valid_StableId(spawned.strPlacementId, true) ||
		!Is_Valid_StableId(spawned.strActionId, true) ||
		!std::isfinite(spawned.fPositionX) ||
		!std::isfinite(spawned.fPositionY) ||
		!std::isfinite(spawned.fPositionZ) ||
		!std::isfinite(spawned.fYawDegrees) ||
		!std::isfinite(spawned.fCollisionRadius) ||
		(WORLD_ENTITY_KIND::NPC == spawned.eKind &&
			0.f != spawned.fCollisionRadius) ||
		(WORLD_ENTITY_KIND::NPC != spawned.eKind &&
			spawned.fCollisionRadius <= 0.f))
	{
		return false;
	}
	writer.Write_U32(spawned.iNetEntityId);
	writer.Write_U8(static_cast<std::uint8_t>(spawned.eKind));
	if (!writer.Write_String(
		spawned.strArchetypeId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!writer.Write_String(
		spawned.strEncounterId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!writer.Write_String(
		spawned.strPlacementId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!writer.Write_String(
		spawned.strActionId, MAX_STABLE_NETWORK_ID_BYTES))
	{
		return false;
	}
	writer.Write_F32(spawned.fPositionX);
	writer.Write_F32(spawned.fPositionY);
	writer.Write_F32(spawned.fPositionZ);
	writer.Write_F32(spawned.fYawDegrees);
	writer.Write_F32(spawned.fCollisionRadius);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_WORLD_ENTITY_SPAWNED& spawned)
{
	S2C_WORLD_ENTITY_SPAWNED decoded{};
	std::uint8_t rawKind = 0;
	if (!reader.Read_U32(decoded.iNetEntityId) ||
		!reader.Read_U8(rawKind) ||
		!reader.Read_String(
			decoded.strArchetypeId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_String(
			decoded.strEncounterId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_String(
			decoded.strPlacementId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_String(
			decoded.strActionId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_F32(decoded.fPositionX) ||
		!reader.Read_F32(decoded.fPositionY) ||
		!reader.Read_F32(decoded.fPositionZ) ||
		!reader.Read_F32(decoded.fYawDegrees) ||
		!reader.Read_F32(decoded.fCollisionRadius))
	{
		return false;
	}
	decoded.eKind = static_cast<WORLD_ENTITY_KIND>(rawKind);
	if (decoded.iNetEntityId == INVALID_NET_ENTITY_ID ||
		!Is_Valid_WorldEntityKind(decoded.eKind) ||
		!Is_Valid_StableId(decoded.strArchetypeId, false) ||
		!Is_Valid_StableId(decoded.strEncounterId, true) ||
		!Is_Valid_StableId(decoded.strPlacementId, true) ||
		!Is_Valid_StableId(decoded.strActionId, true) ||
		!std::isfinite(decoded.fPositionX) ||
		!std::isfinite(decoded.fPositionY) ||
		!std::isfinite(decoded.fPositionZ) ||
		!std::isfinite(decoded.fYawDegrees) ||
		!std::isfinite(decoded.fCollisionRadius) ||
		(WORLD_ENTITY_KIND::NPC == decoded.eKind &&
			0.f != decoded.fCollisionRadius) ||
		(WORLD_ENTITY_KIND::NPC != decoded.eKind &&
			decoded.fCollisionRadius <= 0.f))
	{
		return false;
	}
	spawned = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_WORLD_ENTITY_DESPAWNED& despawned)
{
	if (INVALID_NET_ENTITY_ID == despawned.iNetEntityId)
		return false;

	writer.Write_U32(despawned.iNetEntityId);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_WORLD_ENTITY_DESPAWNED& despawned)
{
	S2C_WORLD_ENTITY_DESPAWNED decoded{};
	if (!reader.Read_U32(decoded.iNetEntityId) ||
		INVALID_NET_ENTITY_ID == decoded.iNetEntityId)
	{
		return false;
	}

	despawned = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_SPAWN_WORLD_ENTITY& message)
{
	return Is_Valid_StableId(message.strPlacementId, false) &&
		writer.Write_String(
			message.strPlacementId,
			MAX_STABLE_NETWORK_ID_BYTES);
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_SPAWN_WORLD_ENTITY& message)
{
	std::string placementId;
	if (!reader.Read_String(placementId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!Is_Valid_StableId(placementId, false))
	{
		return false;
	}

	C2S_SPAWN_WORLD_ENTITY decoded{};
	decoded.strPlacementId = std::move(placementId);
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_WORLD_ENTITY_SPAWN_RESULT& message)
{
	const std::uint8_t rawResult =
		static_cast<std::uint8_t>(message.eResult);
	const bool hasEntity =
		WORLD_ENTITY_SPAWN_RESULT::SPAWNED == message.eResult ||
		WORLD_ENTITY_SPAWN_RESULT::ALREADY_EXISTS == message.eResult;
	if (!Is_Valid_StableId(message.strPlacementId, false) ||
		rawResult >= static_cast<std::uint8_t>(
			WORLD_ENTITY_SPAWN_RESULT::END) ||
		(hasEntity && INVALID_NET_ENTITY_ID == message.iNetEntityId) ||
		(!hasEntity && INVALID_NET_ENTITY_ID != message.iNetEntityId))
	{
		return false;
	}
	if (!writer.Write_String(
		message.strPlacementId,
		MAX_STABLE_NETWORK_ID_BYTES))
	{
		return false;
	}
	writer.Write_U8(rawResult);
	writer.Write_U32(message.iNetEntityId);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_WORLD_ENTITY_SPAWN_RESULT& message)
{
	S2C_WORLD_ENTITY_SPAWN_RESULT decoded{};
	std::uint8_t rawResult = 0;
	if (!reader.Read_String(
		decoded.strPlacementId,
		MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_U8(rawResult) ||
		!reader.Read_U32(decoded.iNetEntityId))
	{
		return false;
	}
	decoded.eResult = static_cast<WORLD_ENTITY_SPAWN_RESULT>(rawResult);
	const bool hasEntity =
		WORLD_ENTITY_SPAWN_RESULT::SPAWNED == decoded.eResult ||
		WORLD_ENTITY_SPAWN_RESULT::ALREADY_EXISTS == decoded.eResult;
	if (!Is_Valid_StableId(decoded.strPlacementId, false) ||
		rawResult >= static_cast<std::uint8_t>(
			WORLD_ENTITY_SPAWN_RESULT::END) ||
		(hasEntity && INVALID_NET_ENTITY_ID == decoded.iNetEntityId) ||
		(!hasEntity && INVALID_NET_ENTITY_ID != decoded.iNetEntityId))
	{
		return false;
	}
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(CPacketWriter& writer, const S2C_PLAYER_DESPAWNED& message)
{
    //검증
    if (message.iNetEntityId == INVALID_NET_ENTITY_ID)
        return false;

    const std::uint8_t rawReason =
        static_cast<std::uint8_t>(message.eReason);

    if (rawReason >= static_cast<std::uint8_t>(
        PLAYER_DESPAWN_REASON::END))
    {
        return false;
    }

    //정보 쓰기
    writer.Write_U32(message.iNetEntityId);
    writer.Write_U8(rawReason);

    return true;
}

bool LostArk::Shared::Read_Message(CPacketReader& reader, S2C_PLAYER_DESPAWNED& message)
{
    NET_ENTITY_ID netEntityId = INVALID_NET_ENTITY_ID;
    std::uint8_t rawReason = {};

    if (!reader.Read_U32(netEntityId))
        return false;

    if (!reader.Read_U8(rawReason))
        return false;

    if (netEntityId == INVALID_NET_ENTITY_ID)
        return false;

    if (rawReason >= static_cast<std::uint8_t>(
        PLAYER_DESPAWN_REASON::END))
        return false;

    S2C_PLAYER_DESPAWNED decoded{};

    decoded.iNetEntityId = netEntityId;
    decoded.eReason = static_cast<PLAYER_DESPAWN_REASON>(
        rawReason);

    message = decoded;

    return true;
}

bool LostArk::Shared::Write_Message(CPacketWriter& writer, const C2S_MOVE& message)
{
    if (0 == message.iClientSequence ||
        !std::isfinite(message.fGoalX) ||
        !std::isfinite(message.fGoalZ))
    {
        return false;
    }

    writer.Write_U32(message.iClientSequence);
    writer.Write_F32(message.fGoalX);
    writer.Write_F32(message.fGoalZ);

    return true;
}

bool LostArk::Shared::Read_Message(CPacketReader& reader, C2S_MOVE& message)
{
    std::uint32_t clientSequence = 0;
    float fGoalX = 0.f;
    float fGoalZ = 0.f;

    if (!reader.Read_U32(clientSequence) ||
        !reader.Read_F32(fGoalX) ||
        !reader.Read_F32(fGoalZ))
    {
        return false;
    }

    if (0 == clientSequence ||
        !std::isfinite(fGoalX) ||
        !std::isfinite(fGoalZ))
    {
        return false;
    }

    C2S_MOVE decoded{};
    decoded.iClientSequence = clientSequence;
    decoded.fGoalX = fGoalX;
    decoded.fGoalZ = fGoalZ;

    message = decoded;
    return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_USE_SKILL& message)
{
	if (0 == message.iClientSequence ||
		INVALID_SKILL_ID == message.iSkillId ||
		!std::isfinite(message.fAimX) ||
		!std::isfinite(message.fAimZ))
	{
		return false;
	}

	writer.Write_U32(message.iClientSequence);
	writer.Write_U32(message.iSkillId);
	writer.Write_F32(message.fAimX);
	writer.Write_F32(message.fAimZ);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_USE_SKILL& message)
{
	C2S_USE_SKILL decoded{};
	if (!reader.Read_U32(decoded.iClientSequence) ||
		!reader.Read_U32(decoded.iSkillId) ||
		!reader.Read_F32(decoded.fAimX) ||
		!reader.Read_F32(decoded.fAimZ) ||
		0 == decoded.iClientSequence ||
		INVALID_SKILL_ID == decoded.iSkillId ||
		!std::isfinite(decoded.fAimX) ||
		!std::isfinite(decoded.fAimZ))
	{
		return false;
	}

	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_RELEASE_SKILL& message)
{
	if (0 == message.iClientSequence ||
		INVALID_SKILL_ID == message.iSkillId)
	{
		return false;
	}

	writer.Write_U32(message.iClientSequence);
	writer.Write_U32(message.iSkillId);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_REVIVE_PLAYER& message)
{
	if (0u == message.iClientSequence)
		return false;
	writer.Write_U32(message.iClientSequence);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_RELEASE_SKILL& message)
{
	C2S_RELEASE_SKILL decoded{};
	if (!reader.Read_U32(decoded.iClientSequence) ||
		!reader.Read_U32(decoded.iSkillId) ||
		0 == decoded.iClientSequence ||
		INVALID_SKILL_ID == decoded.iSkillId)
	{
		return false;
	}

	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_UPDATE_SKILL_AIM& message)
{
	if (0 == message.iClientSequence ||
		INVALID_SKILL_ID == message.iSkillId ||
		!std::isfinite(message.fAimX) ||
		!std::isfinite(message.fAimZ))
	{
		return false;
	}

	writer.Write_U32(message.iClientSequence);
	writer.Write_U32(message.iSkillId);
	writer.Write_F32(message.fAimX);
	writer.Write_F32(message.fAimZ);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_UPDATE_SKILL_AIM& message)
{
	C2S_UPDATE_SKILL_AIM decoded{};
	if (!reader.Read_U32(decoded.iClientSequence) ||
		!reader.Read_U32(decoded.iSkillId) ||
		!reader.Read_F32(decoded.fAimX) ||
		!reader.Read_F32(decoded.fAimZ) ||
		0 == decoded.iClientSequence ||
		INVALID_SKILL_ID == decoded.iSkillId ||
		!std::isfinite(decoded.fAimX) ||
		!std::isfinite(decoded.fAimZ))
	{
		return false;
	}

	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_USE_ESTHER_SKILL& message)
{
	if (0 == message.iClientSequence ||
		message.iSlotIndex < MIN_ESTHER_SLOT_INDEX ||
		message.iSlotIndex > MAX_ESTHER_SLOT_INDEX ||
		!std::isfinite(message.fAimX) ||
		!std::isfinite(message.fAimZ))
	{
		return false;
	}

	writer.Write_U32(message.iClientSequence);
	writer.Write_U8(message.iSlotIndex);
	writer.Write_F32(message.fAimX);
	writer.Write_F32(message.fAimZ);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_USE_ESTHER_SKILL& message)
{
	C2S_USE_ESTHER_SKILL decoded{};
	if (!reader.Read_U32(decoded.iClientSequence) ||
		!reader.Read_U8(decoded.iSlotIndex) ||
		!reader.Read_F32(decoded.fAimX) ||
		!reader.Read_F32(decoded.fAimZ) ||
		0 == decoded.iClientSequence ||
		decoded.iSlotIndex < MIN_ESTHER_SLOT_INDEX ||
		decoded.iSlotIndex > MAX_ESTHER_SLOT_INDEX ||
		!std::isfinite(decoded.fAimX) ||
		!std::isfinite(decoded.fAimZ))
	{
		return false;
	}

	message = decoded;
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_REVIVE_PLAYER& message)
{
	C2S_REVIVE_PLAYER decoded{};
	if (!reader.Read_U32(decoded.iClientSequence) ||
		0u == decoded.iClientSequence)
	{
		return false;
	}
	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_CHANGE_CHARACTER_CLASS& message)
{
	if (0u == message.iClientSequence ||
		!Is_Known_Character_Class(message.eCharacterClass))
	{
		return false;
	}
	writer.Write_U32(message.iClientSequence);
	writer.Write_U8(static_cast<std::uint8_t>(message.eCharacterClass));
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_CHANGE_CHARACTER_CLASS& message)
{
	C2S_CHANGE_CHARACTER_CLASS decoded{};
	std::uint8_t rawClass = 0;
	if (!reader.Read_U32(decoded.iClientSequence) ||
		!reader.Read_U8(rawClass))
	{
		return false;
	}
	decoded.eCharacterClass = static_cast<CHARACTER_CLASS_ID>(rawClass);
	if (0u == decoded.iClientSequence ||
		!Is_Known_Character_Class(decoded.eCharacterClass))
	{
		return false;
	}
	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_CHARACTER_CLASS_CHANGE_RESULT& message)
{
	if (0u == message.iClientSequence ||
		static_cast<std::uint8_t>(message.eResult) >=
			static_cast<std::uint8_t>(CHARACTER_CLASS_CHANGE_RESULT::END) ||
		!Is_Known_Character_Class(message.eRequestedClass) ||
		!Is_Supported_Playable_Character_Class(message.eActiveClass) ||
		((CHARACTER_CLASS_CHANGE_RESULT::ACCEPTED == message.eResult ||
			CHARACTER_CLASS_CHANGE_RESULT::REJECTED_SAME_CLASS == message.eResult) &&
			message.eRequestedClass != message.eActiveClass))
	{
		return false;
	}
	writer.Write_U32(message.iClientSequence);
	writer.Write_U8(static_cast<std::uint8_t>(message.eResult));
	writer.Write_U8(static_cast<std::uint8_t>(message.eRequestedClass));
	writer.Write_U8(static_cast<std::uint8_t>(message.eActiveClass));
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_CHARACTER_CLASS_CHANGE_RESULT& message)
{
	S2C_CHARACTER_CLASS_CHANGE_RESULT decoded{};
	std::uint8_t rawResult = 0;
	std::uint8_t rawRequested = 0;
	std::uint8_t rawActive = 0;
	if (!reader.Read_U32(decoded.iClientSequence) ||
		!reader.Read_U8(rawResult) ||
		!reader.Read_U8(rawRequested) ||
		!reader.Read_U8(rawActive))
	{
		return false;
	}
	decoded.eResult = static_cast<CHARACTER_CLASS_CHANGE_RESULT>(rawResult);
	decoded.eRequestedClass = static_cast<CHARACTER_CLASS_ID>(rawRequested);
	decoded.eActiveClass = static_cast<CHARACTER_CLASS_ID>(rawActive);
	if (0u == decoded.iClientSequence ||
		rawResult >= static_cast<std::uint8_t>(CHARACTER_CLASS_CHANGE_RESULT::END) ||
		!Is_Known_Character_Class(decoded.eRequestedClass) ||
		!Is_Supported_Playable_Character_Class(decoded.eActiveClass) ||
		((CHARACTER_CLASS_CHANGE_RESULT::ACCEPTED == decoded.eResult ||
			CHARACTER_CLASS_CHANGE_RESULT::REJECTED_SAME_CLASS == decoded.eResult) &&
			decoded.eRequestedClass != decoded.eActiveClass))
	{
		return false;
	}
	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(CPacketWriter& writer, const S2C_WORLD_SNAPSHOT& message)
{
    //world의 snapshot write, servertick과 player 정보
	if (0 == message.iServerTick ||
		!Is_Known_World_Id(message.eWorldId) ||
        message.Players.empty() ||
        message.Players.size() >
        MAX_WORLD_SNAPSHOT_PLAYERS ||
		message.Entities.size() > MAX_WORLD_SNAPSHOT_ENTITIES ||
		message.DamageEvents.size() > MAX_DAMAGE_EVENTS ||
		// maximum 0 means "no Esther in this world" and then the level must be
		// 0 too; a live gauge can never exceed its maximum.
		(0 == message.iEstherGaugeMaximum && 0 != message.iEstherGauge) ||
		message.iEstherGauge > message.iEstherGaugeMaximum)
    {
        return false;
    }
    //유효하지 않은 플레이어 스냅샷 검사
    for (const PLAYER_SNAPSHOT& player : message.Players)
    {
        if (!Is_Valid_PlayerSnapshot(player))
            return false;
    }
	for (const WORLD_ENTITY_SNAPSHOT& entity : message.Entities)
	{
		if (!Is_Valid_WorldEntitySnapshot(entity))
			return false;
	}
	for (const DAMAGE_EVENT& damage : message.DamageEvents)
	{
		if (!Is_Valid_DamageEvent(damage))
			return false;
	}
    //server tick과 player size 넣기
	writer.Write_U32(message.iServerTick);
	writer.Write_U16(
		static_cast<std::uint16_t>(message.eWorldId));
	writer.Write_U16(
        static_cast<std::uint16_t>(
            message.Players.size()));
	writer.Write_U16(
		static_cast<std::uint16_t>(message.Entities.size()));
	// U8 is enough: MAX_DAMAGE_EVENTS bounds one tick, far under 255.
	writer.Write_U8(
		static_cast<std::uint8_t>(message.DamageEvents.size()));
	writer.Write_U32(message.iEstherGauge);
	writer.Write_U32(message.iEstherGaugeMaximum);

    for (const PLAYER_SNAPSHOT& player : message.Players)
    {
        writer.Write_U32(player.iNetEntityId);
		writer.Write_U8(static_cast<std::uint8_t>(player.eCharacterClass));
        writer.Write_F32(player.fPositionX);
        writer.Write_F32(player.fPositionY);
        writer.Write_F32(player.fPositionZ);
        writer.Write_F32(player.fYawDegrees);
        writer.Write_U8(
            static_cast<std::uint8_t>(
                player.eLocomotionState));
		writer.Write_U8(static_cast<std::uint8_t>(player.eAction));
		writer.Write_U8(static_cast<std::uint8_t>(player.eStance));
		writer.Write_U32(player.iSkillId);
		writer.Write_U32(player.iActionStartTick);
		writer.Write_U32(player.iCurrentHp);
		writer.Write_U32(player.iMaximumHp);
		writer.Write_U32(player.iCurrentResource);
		writer.Write_U32(player.iMaximumResource);
		writer.Write_U32(player.iCurrentIdentity);
		writer.Write_U32(player.iMaximumIdentity);
		writer.Write_U8(player.isCombatReady ? 1u : 0u);
		writer.Write_U8(player.iComboStage);
		writer.Write_U8(static_cast<std::uint8_t>(player.Cooldowns.size()));
		for (const SKILL_COOLDOWN_SNAPSHOT& cooldown : player.Cooldowns)
		{
			writer.Write_U32(cooldown.iSkillId);
			writer.Write_U32(cooldown.iCooldownEndTick);
		}
    }
	for (const WORLD_ENTITY_SNAPSHOT& entity : message.Entities)
	{
		writer.Write_U32(entity.iNetEntityId);
		writer.Write_U8(static_cast<std::uint8_t>(entity.eAction));
		if (!writer.Write_String(
			entity.strPatternId, MAX_STABLE_NETWORK_ID_BYTES))
		{
			return false;
		}
		if (!writer.Write_String(
			entity.strActionId, MAX_STABLE_NETWORK_ID_BYTES))
		{
			return false;
		}
		writer.Write_F32(entity.fPositionX);
		writer.Write_F32(entity.fPositionY);
		writer.Write_F32(entity.fPositionZ);
		writer.Write_F32(entity.fYawDegrees);
		writer.Write_U32(entity.iActionStartTick);
		writer.Write_U32(entity.iPatternSequence);
		writer.Write_U32(entity.iPatternStageIndex);
		writer.Write_U32(entity.iCurrentHp);
		writer.Write_U32(entity.iMaximumHp);
		writer.Write_U8(entity.iPhase);
	}
	for (const DAMAGE_EVENT& damage : message.DamageEvents)
	{
		writer.Write_U32(damage.iTargetNetEntityId);
		writer.Write_U32(damage.iAmount);
		writer.Write_F32(damage.fPositionX);
		writer.Write_F32(damage.fPositionY);
		writer.Write_F32(damage.fPositionZ);
		writer.Write_U8(damage.isOutgoing ? 1u : 0u);
	}

    return true;
}

bool LostArk::Shared::Read_Message(CPacketReader& reader, S2C_WORLD_SNAPSHOT& message)
{
	std::uint32_t serverTick = 0;
	std::uint16_t rawWorldId = 0;
	std::uint16_t playerCount = 0;
	std::uint16_t entityCount = 0;
	std::uint8_t damageEventCount = 0;
	std::uint32_t estherGauge = 0;
	std::uint32_t estherGaugeMaximum = 0;

	if (!reader.Read_U32(serverTick) ||
		!reader.Read_U16(rawWorldId) ||
		!reader.Read_U16(playerCount) ||
		!reader.Read_U16(entityCount) ||
		!reader.Read_U8(damageEventCount) ||
		!reader.Read_U32(estherGauge) ||
		!reader.Read_U32(estherGaugeMaximum))
    {
        return false;
    }

	if (0 == serverTick ||
		!Is_Known_World_Id(static_cast<WORLD_ID>(rawWorldId)) ||
        0 == playerCount ||
        playerCount > MAX_WORLD_SNAPSHOT_PLAYERS ||
		entityCount > MAX_WORLD_SNAPSHOT_ENTITIES ||
		damageEventCount > MAX_DAMAGE_EVENTS ||
		(0 == estherGaugeMaximum && 0 != estherGauge) ||
		estherGauge > estherGaugeMaximum)
    {
        return false;
    }

	S2C_WORLD_SNAPSHOT decoded{};
	decoded.iServerTick = serverTick;
	decoded.eWorldId = static_cast<WORLD_ID>(rawWorldId);
	decoded.iEstherGauge = estherGauge;
	decoded.iEstherGaugeMaximum = estherGaugeMaximum;
    decoded.Players.reserve(playerCount);
	decoded.Entities.reserve(entityCount);
	decoded.DamageEvents.reserve(damageEventCount);

    for (std::uint16_t i = 0; i < playerCount; ++i)
    {
        PLAYER_SNAPSHOT player{};
		std::uint8_t rawCharacterClass = 0;
        std::uint8_t rawLocomotion = 0;
		std::uint8_t rawAction = 0;
		std::uint8_t rawStance = 0;
		std::uint8_t rawCombatReady = 0;
		std::uint8_t cooldownCount = 0;

        if (!reader.Read_U32(player.iNetEntityId) ||
			!reader.Read_U8(rawCharacterClass) ||
            !reader.Read_F32(player.fPositionX) ||
            !reader.Read_F32(player.fPositionY) ||
            !reader.Read_F32(player.fPositionZ) ||
            !reader.Read_F32(player.fYawDegrees) ||
            !reader.Read_U8(rawLocomotion) ||
			!reader.Read_U8(rawAction) ||
			!reader.Read_U8(rawStance) ||
			!reader.Read_U32(player.iSkillId) ||
			!reader.Read_U32(player.iActionStartTick) ||
			!reader.Read_U32(player.iCurrentHp) ||
			!reader.Read_U32(player.iMaximumHp) ||
			!reader.Read_U32(player.iCurrentResource) ||
			!reader.Read_U32(player.iMaximumResource) ||
			!reader.Read_U32(player.iCurrentIdentity) ||
			!reader.Read_U32(player.iMaximumIdentity) ||
			!reader.Read_U8(rawCombatReady) ||
			rawCombatReady > 1u ||
			!reader.Read_U8(player.iComboStage) ||
			player.iComboStage > MAX_COMBO_STAGES ||
			!reader.Read_U8(cooldownCount) ||
			cooldownCount > MAX_PLAYER_COOLDOWNS)
        {
            return false;
        }

        player.eLocomotionState =
            static_cast<PLAYER_LOCOMOTION_STATE>(
                rawLocomotion);
		player.eCharacterClass = static_cast<CHARACTER_CLASS_ID>(rawCharacterClass);
		player.eAction = static_cast<PLAYER_ACTION_STATE>(rawAction);
		player.eStance = static_cast<PLAYER_STANCE_ID>(rawStance);
		player.isCombatReady = 0u != rawCombatReady;
		player.Cooldowns.reserve(cooldownCount);
		for (std::uint8_t cooldownIndex = 0;
			cooldownIndex < cooldownCount;
			++cooldownIndex)
		{
			SKILL_COOLDOWN_SNAPSHOT cooldown{};
			if (!reader.Read_U32(cooldown.iSkillId) ||
				!reader.Read_U32(cooldown.iCooldownEndTick))
			{
				return false;
			}
			player.Cooldowns.push_back(cooldown);
		}

        if (!Is_Valid_PlayerSnapshot(player))
            return false;

        decoded.Players.push_back(player);
    }
	for (std::uint16_t i = 0; i < entityCount; ++i)
	{
		WORLD_ENTITY_SNAPSHOT entity{};
		std::uint8_t rawAction = 0;
		if (!reader.Read_U32(entity.iNetEntityId) ||
			!reader.Read_U8(rawAction) ||
			!reader.Read_String(
				entity.strPatternId, MAX_STABLE_NETWORK_ID_BYTES) ||
			!reader.Read_String(
				entity.strActionId, MAX_STABLE_NETWORK_ID_BYTES) ||
			!reader.Read_F32(entity.fPositionX) ||
			!reader.Read_F32(entity.fPositionY) ||
			!reader.Read_F32(entity.fPositionZ) ||
			!reader.Read_F32(entity.fYawDegrees) ||
			!reader.Read_U32(entity.iActionStartTick) ||
			!reader.Read_U32(entity.iPatternSequence) ||
			!reader.Read_U32(entity.iPatternStageIndex) ||
			!reader.Read_U32(entity.iCurrentHp) ||
			!reader.Read_U32(entity.iMaximumHp) ||
			!reader.Read_U8(entity.iPhase))
		{
			return false;
		}
		entity.eAction = static_cast<WORLD_ENTITY_ACTION>(rawAction);
		if (!Is_Valid_WorldEntitySnapshot(entity))
			return false;
		decoded.Entities.push_back(std::move(entity));
	}
	for (std::uint8_t i = 0; i < damageEventCount; ++i)
	{
		DAMAGE_EVENT damage{};
		std::uint8_t rawOutgoing = 0;
		if (!reader.Read_U32(damage.iTargetNetEntityId) ||
			!reader.Read_U32(damage.iAmount) ||
			!reader.Read_F32(damage.fPositionX) ||
			!reader.Read_F32(damage.fPositionY) ||
			!reader.Read_F32(damage.fPositionZ) ||
			!reader.Read_U8(rawOutgoing) ||
			rawOutgoing > 1u)
		{
			return false;
		}
		damage.isOutgoing = 0u != rawOutgoing;
		if (!Is_Valid_DamageEvent(damage))
			return false;
		decoded.DamageEvents.push_back(damage);
	}

    message = std::move(decoded);
    return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_WORLD_DESTRUCTION_FULL_SYNC& message)
{
	if (!Is_Valid_CombatRuntimeRevision(
			message.strCombatRuntimeRevision) ||
		0 == message.iServerTick ||
		0 == message.iEncounterEpoch ||
		message.GroupStates.empty() ||
		message.GroupStates.size() > MAX_WORLD_DESTRUCTION_GROUPS ||
		!Are_DestructionStatesCanonical(message.GroupStates))
	{
		return false;
	}

	if (!writer.Write_String(
		message.strCombatRuntimeRevision,
		MAX_COMBAT_RUNTIME_REVISION_BYTES))
	{
		return false;
	}
	writer.Write_U32(message.iServerTick);
	writer.Write_U32(message.iEncounterEpoch);
	writer.Write_U16(static_cast<std::uint16_t>(
		message.GroupStates.size()));
	for (const WORLD_DESTRUCTION_STATE_WIRE& state :
		message.GroupStates)
	{
		if (!Write_DestructionStateWire(writer, state))
			return false;
	}
	Write_DestructionDiagnostics(writer, message.Diagnostics);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_WORLD_DESTRUCTION_FULL_SYNC& message)
{
	S2C_WORLD_DESTRUCTION_FULL_SYNC decoded{};
	std::uint16_t groupCount = 0;
	if (!reader.Read_String(
			decoded.strCombatRuntimeRevision,
			MAX_COMBAT_RUNTIME_REVISION_BYTES) ||
		!reader.Read_U32(decoded.iServerTick) ||
		!reader.Read_U32(decoded.iEncounterEpoch) ||
		!reader.Read_U16(groupCount) ||
		!Is_Valid_CombatRuntimeRevision(
			decoded.strCombatRuntimeRevision) ||
		0 == decoded.iServerTick ||
		0 == decoded.iEncounterEpoch ||
		0 == groupCount ||
		groupCount > MAX_WORLD_DESTRUCTION_GROUPS)
	{
		return false;
	}

	decoded.GroupStates.reserve(groupCount);
	for (std::uint16_t i = 0; i < groupCount; ++i)
	{
		WORLD_DESTRUCTION_STATE_WIRE state{};
		if (!Read_DestructionStateWire(reader, state))
			return false;
		decoded.GroupStates.push_back(std::move(state));
	}
	if (!Are_DestructionStatesCanonical(decoded.GroupStates) ||
		!Read_DestructionDiagnostics(reader, decoded.Diagnostics))
	{
		return false;
	}

	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_WORLD_DESTRUCTION_DELTA& message)
{
	if (!Is_Valid_CombatRuntimeRevision(
			message.strCombatRuntimeRevision) ||
		0 == message.iServerTick ||
		0 == message.iEncounterEpoch ||
		(message.ChangedStates.empty() && message.LiveEvents.empty()) ||
		message.ChangedStates.size() >
			MAX_WORLD_DESTRUCTION_CHANGED_STATES ||
		message.LiveEvents.size() > MAX_WORLD_DESTRUCTION_EVENTS ||
		!Are_DestructionStatesCanonical(message.ChangedStates) ||
		!Are_DestructionEventsCanonical(message.LiveEvents))
	{
		return false;
	}

	if (!writer.Write_String(
		message.strCombatRuntimeRevision,
		MAX_COMBAT_RUNTIME_REVISION_BYTES))
	{
		return false;
	}
	writer.Write_U32(message.iServerTick);
	writer.Write_U32(message.iEncounterEpoch);
	writer.Write_U16(static_cast<std::uint16_t>(
		message.ChangedStates.size()));
	writer.Write_U16(static_cast<std::uint16_t>(
		message.LiveEvents.size()));
	for (const WORLD_DESTRUCTION_STATE_WIRE& state :
		message.ChangedStates)
	{
		if (!Write_DestructionStateWire(writer, state))
			return false;
	}
	for (const WORLD_DESTRUCTION_EVENT_WIRE& event :
		message.LiveEvents)
	{
		if (!Write_DestructionEventWire(writer, event))
			return false;
	}
	Write_DestructionDiagnostics(writer, message.Diagnostics);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_WORLD_DESTRUCTION_DELTA& message)
{
	S2C_WORLD_DESTRUCTION_DELTA decoded{};
	std::uint16_t changedStateCount = 0;
	std::uint16_t eventCount = 0;
	if (!reader.Read_String(
			decoded.strCombatRuntimeRevision,
			MAX_COMBAT_RUNTIME_REVISION_BYTES) ||
		!reader.Read_U32(decoded.iServerTick) ||
		!reader.Read_U32(decoded.iEncounterEpoch) ||
		!reader.Read_U16(changedStateCount) ||
		!reader.Read_U16(eventCount) ||
		!Is_Valid_CombatRuntimeRevision(
			decoded.strCombatRuntimeRevision) ||
		0 == decoded.iServerTick ||
		0 == decoded.iEncounterEpoch ||
		(0 == changedStateCount && 0 == eventCount) ||
		changedStateCount > MAX_WORLD_DESTRUCTION_CHANGED_STATES ||
		eventCount > MAX_WORLD_DESTRUCTION_EVENTS)
	{
		return false;
	}

	decoded.ChangedStates.reserve(changedStateCount);
	decoded.LiveEvents.reserve(eventCount);
	for (std::uint16_t i = 0; i < changedStateCount; ++i)
	{
		WORLD_DESTRUCTION_STATE_WIRE state{};
		if (!Read_DestructionStateWire(reader, state))
			return false;
		decoded.ChangedStates.push_back(std::move(state));
	}
	for (std::uint16_t i = 0; i < eventCount; ++i)
	{
		WORLD_DESTRUCTION_EVENT_WIRE event{};
		if (!Read_DestructionEventWire(reader, event))
			return false;
		decoded.LiveEvents.push_back(std::move(event));
	}
	if (!Are_DestructionStatesCanonical(decoded.ChangedStates) ||
		!Are_DestructionEventsCanonical(decoded.LiveEvents) ||
		!Read_DestructionDiagnostics(reader, decoded.Diagnostics))
	{
		return false;
	}

	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_ENCOUNTER_PROP_SYNC& message)
{
	if (message.strPropSetId.empty() ||
		message.strPropSetId.size() > MAX_STABLE_NETWORK_ID_BYTES ||
		0 == message.iServerTick || 0 == message.iEncounterEpoch ||
		message.Slots.empty() ||
		message.Slots.size() > MAX_ENCOUNTER_PROP_SLOTS ||
		!Is_Valid_EncounterPropSlots(message.Slots))
	{
		return false;
	}
	if (!writer.Write_String(message.strPropSetId, MAX_STABLE_NETWORK_ID_BYTES))
		return false;
	writer.Write_U32(message.iServerTick);
	writer.Write_U32(message.iEncounterEpoch);
	writer.Write_U16(static_cast<std::uint16_t>(message.Slots.size()));
	for (const ENCOUNTER_PROP_SLOT_WIRE& slot : message.Slots)
	{
		if (!writer.Write_String(slot.strSlotId, MAX_STABLE_NETWORK_ID_BYTES))
			return false;
		writer.Write_U8(static_cast<std::uint8_t>(slot.eState));
		writer.Write_U32(slot.iStateVersion);
		writer.Write_U32(slot.iStateStartTick);
		writer.Write_U32(slot.iOccurrenceSequence);
	}
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_ENCOUNTER_PROP_SYNC& message)
{
	S2C_ENCOUNTER_PROP_SYNC decoded{};
	std::uint16_t slotCount = 0;
	if (!reader.Read_String(decoded.strPropSetId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_U32(decoded.iServerTick) ||
		!reader.Read_U32(decoded.iEncounterEpoch) ||
		!reader.Read_U16(slotCount) ||
		decoded.strPropSetId.empty() ||
		0 == decoded.iServerTick || 0 == decoded.iEncounterEpoch ||
		0 == slotCount || slotCount > MAX_ENCOUNTER_PROP_SLOTS)
	{
		return false;
	}
	decoded.Slots.reserve(slotCount);
	for (std::uint16_t index = 0; index < slotCount; ++index)
	{
		ENCOUNTER_PROP_SLOT_WIRE slot{};
		std::uint8_t rawState = 0;
		if (!reader.Read_String(slot.strSlotId, MAX_STABLE_NETWORK_ID_BYTES) ||
			!reader.Read_U8(rawState) ||
			!reader.Read_U32(slot.iStateVersion) ||
			!reader.Read_U32(slot.iStateStartTick) ||
			!reader.Read_U32(slot.iOccurrenceSequence) ||
			rawState >= static_cast<std::uint8_t>(ENCOUNTER_PROP_STATE::END))
		{
			return false;
		}
		slot.eState = static_cast<ENCOUNTER_PROP_STATE>(rawState);
		decoded.Slots.push_back(std::move(slot));
	}
	if (!Is_Valid_EncounterPropSlots(decoded.Slots))
		return false;
	message = std::move(decoded);
	return true;
}

namespace
{
	using namespace LostArk::Shared;

	// A request sequence of zero can never be told apart from a default-built
	// struct, so it is not a usable duplicate key. Health bar zero is the dead
	// boss and carries no authored pattern.
	bool Is_Valid_AuditionRequest(
		const std::uint32_t requestSequence,
		const std::uint8_t rawOperation,
		const std::uint32_t targetHealthBar)
	{
		if (0u == requestSequence ||
			rawOperation >= static_cast<std::uint8_t>(
				VALTAN_AUDITION_OPERATION::END))
		{
			return false;
		}
		// These operations name an authored mechanic or a Debug view directly,
		// rather than a health-bar crossing, so they carry exactly zero.
		if (static_cast<std::uint8_t>(VALTAN_AUDITION_OPERATION::PLAY_ENTRANCE) ==
			rawOperation ||
			static_cast<std::uint8_t>(
				VALTAN_AUDITION_OPERATION::PLAY_PILLAR_CYCLE) == rawOperation ||
			static_cast<std::uint8_t>(
				VALTAN_AUDITION_OPERATION::PLAY_WALL_ATTACK) == rawOperation ||
			static_cast<std::uint8_t>(
				VALTAN_AUDITION_OPERATION::SHOW_FINAL_ARENA) == rawOperation ||
			static_cast<std::uint8_t>(
				VALTAN_AUDITION_OPERATION::BREAK_EVERY_WALL) == rawOperation ||
			static_cast<std::uint8_t>(
				VALTAN_AUDITION_OPERATION::PLAY_ORDERED_1_67) == rawOperation ||
			static_cast<std::uint8_t>(
				VALTAN_AUDITION_OPERATION::STOP_ORDERED_1_67) == rawOperation)
		{
			return 0u == targetHealthBar;
		}
		return 0u != targetHealthBar;
	}
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_VALTAN_AUDITION_REQUEST& message)
{
	const std::uint8_t rawOperation =
		static_cast<std::uint8_t>(message.eOperation);
	if (!Is_Valid_AuditionRequest(
		message.iRequestSequence, rawOperation, message.iTargetHealthBar))
	{
		return false;
	}
	writer.Write_U32(message.iRequestSequence);
	writer.Write_U8(rawOperation);
	writer.Write_U32(message.iTargetHealthBar);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_VALTAN_AUDITION_REQUEST& message)
{
	C2S_VALTAN_AUDITION_REQUEST decoded{};
	std::uint8_t rawOperation = 0;
	if (!reader.Read_U32(decoded.iRequestSequence) ||
		!reader.Read_U8(rawOperation) ||
		!reader.Read_U32(decoded.iTargetHealthBar))
	{
		return false;
	}
	if (!Is_Valid_AuditionRequest(
		decoded.iRequestSequence, rawOperation, decoded.iTargetHealthBar))
	{
		return false;
	}
	decoded.eOperation =
		static_cast<VALTAN_AUDITION_OPERATION>(rawOperation);
	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_VALTAN_AUDITION_RESULT& message)
{
	const std::uint8_t rawOperation =
		static_cast<std::uint8_t>(message.eOperation);
	const std::uint8_t rawResult =
		static_cast<std::uint8_t>(message.eResult);
	if (!Is_Valid_AuditionRequest(
		message.iRequestSequence, rawOperation, message.iTargetHealthBar) ||
		rawResult >= static_cast<std::uint8_t>(VALTAN_AUDITION_RESULT::END))
	{
		return false;
	}
	writer.Write_U32(message.iRequestSequence);
	writer.Write_U8(rawOperation);
	writer.Write_U32(message.iTargetHealthBar);
	writer.Write_U8(rawResult);
	writer.Write_U32(message.iCurrentHealthBar);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_VALTAN_AUDITION_RESULT& message)
{
	S2C_VALTAN_AUDITION_RESULT decoded{};
	std::uint8_t rawOperation = 0;
	std::uint8_t rawResult = 0;
	if (!reader.Read_U32(decoded.iRequestSequence) ||
		!reader.Read_U8(rawOperation) ||
		!reader.Read_U32(decoded.iTargetHealthBar) ||
		!reader.Read_U8(rawResult) ||
		!reader.Read_U32(decoded.iCurrentHealthBar))
	{
		return false;
	}
	if (!Is_Valid_AuditionRequest(
		decoded.iRequestSequence, rawOperation, decoded.iTargetHealthBar) ||
		rawResult >= static_cast<std::uint8_t>(VALTAN_AUDITION_RESULT::END))
	{
		return false;
	}
	decoded.eOperation =
		static_cast<VALTAN_AUDITION_OPERATION>(rawOperation);
	decoded.eResult = static_cast<VALTAN_AUDITION_RESULT>(rawResult);
	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_DEBUG_GIVE_ITEM& message)
{
	if (0u == message.iRequestSequence || !Is_Valid_ItemId(message.strItemId) ||
		0u == message.iQuantity)
	{
		return false;
	}
	writer.Write_U32(message.iRequestSequence);
	if (!writer.Write_String(message.strItemId, MAX_ITEM_ID_BYTES))
		return false;
	writer.Write_U32(message.iQuantity);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_DEBUG_GIVE_ITEM& message)
{
	C2S_DEBUG_GIVE_ITEM decoded{};
	if (!reader.Read_U32(decoded.iRequestSequence) ||
		!reader.Read_String(decoded.strItemId, MAX_ITEM_ID_BYTES) ||
		!reader.Read_U32(decoded.iQuantity) ||
		0u == decoded.iRequestSequence || !Is_Valid_ItemId(decoded.strItemId) ||
		0u == decoded.iQuantity)
	{
		return false;
	}
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_INVENTORY_SNAPSHOT& message)
{
	if (message.Items.size() > MAX_INVENTORY_ITEMS ||
		!Is_Valid_InventoryItems(message.Items))
	{
		return false;
	}
	writer.Write_U32(message.iRequestSequence);
	writer.Write_U16(static_cast<std::uint16_t>(message.Items.size()));
	for (const INVENTORY_ITEM_SNAPSHOT& item : message.Items)
	{
		if (!writer.Write_String(item.strItemId, MAX_ITEM_ID_BYTES))
			return false;
		writer.Write_U32(item.iQuantity);
	}
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_INVENTORY_SNAPSHOT& message)
{
	S2C_INVENTORY_SNAPSHOT decoded{};
	std::uint16_t itemCount = 0;
	if (!reader.Read_U32(decoded.iRequestSequence) ||
		!reader.Read_U16(itemCount) || itemCount > MAX_INVENTORY_ITEMS)
	{
		return false;
	}
	decoded.Items.reserve(itemCount);
	for (std::uint16_t index = 0; index < itemCount; ++index)
	{
		INVENTORY_ITEM_SNAPSHOT item{};
		if (!reader.Read_String(item.strItemId, MAX_ITEM_ID_BYTES) ||
			!reader.Read_U32(item.iQuantity))
		{
			return false;
		}
		decoded.Items.push_back(std::move(item));
	}
	if (!Is_Valid_InventoryItems(decoded.Items))
		return false;
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_USE_ITEM& message)
{
	if (0u == message.iRequestSequence || !Is_Valid_ItemId(message.strItemId))
		return false;
	writer.Write_U32(message.iRequestSequence);
	if (!writer.Write_String(message.strItemId, MAX_ITEM_ID_BYTES))
		return false;
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_USE_ITEM& message)
{
	C2S_USE_ITEM decoded{};
	if (!reader.Read_U32(decoded.iRequestSequence) ||
		!reader.Read_String(decoded.strItemId, MAX_ITEM_ID_BYTES) ||
		0u == decoded.iRequestSequence || !Is_Valid_ItemId(decoded.strItemId))
	{
		return false;
	}
	message = std::move(decoded);
	return true;
}

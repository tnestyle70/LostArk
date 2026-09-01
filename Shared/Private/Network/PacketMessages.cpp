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

	bool Is_Valid_PlayerAttachmentSlot(
		const LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot)
	{
		return static_cast<std::uint8_t>(slot) <
			static_cast<std::uint8_t>(
				LostArk::Shared::PLAYER_ATTACHMENT_SLOT::END);
	}

	bool Is_Valid_SkillTargetIntent(
		const LostArk::Shared::SKILL_TARGET_INTENT_KIND intent)
	{
		return static_cast<std::uint8_t>(intent) <
			static_cast<std::uint8_t>(
				LostArk::Shared::SKILL_TARGET_INTENT_KIND::END);
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
			Is_Valid_PlayerAttachmentSlot(snapshot.eAttachmentSlot) &&
			std::isfinite(snapshot.fAttachmentLocalOffsetX) &&
			std::isfinite(snapshot.fAttachmentLocalOffsetY) &&
			std::isfinite(snapshot.fAttachmentLocalOffsetZ) &&
			std::isfinite(snapshot.fAttachmentYawOffsetDegrees) &&
			std::isfinite(snapshot.fSkillTargetX) &&
			std::isfinite(snapshot.fSkillTargetY) &&
			std::isfinite(snapshot.fSkillTargetZ) &&
			(snapshot.hasSkillTarget ?
				(LostArk::Shared::PLAYER_ACTION_STATE::SKILL == snapshot.eAction) :
				(0.f == snapshot.fSkillTargetX &&
				 0.f == snapshot.fSkillTargetY &&
				 0.f == snapshot.fSkillTargetZ)) &&
			0 != snapshot.iMaximumHp &&
			snapshot.iCurrentHp <= snapshot.iMaximumHp &&
			0 != snapshot.iMaximumResource &&
			snapshot.iCurrentResource <= snapshot.iMaximumResource &&
			snapshot.iCurrentIdentity <= snapshot.iMaximumIdentity &&
			((0u == snapshot.iSilenceEndTick) ==
			 (0u == snapshot.iSilenceDurationTicks)) &&
			snapshot.iSilenceDurationTicks <= 3600u &&
			(snapshot.isPatternBound ?
				(snapshot.iPatternBindEndTick != 0u &&
				 snapshot.iCurrentHp != 0u && !snapshot.isCombatReady &&
				 LostArk::Shared::PLAYER_LOCOMOTION_STATE::IDLE ==
					snapshot.eLocomotionState &&
				 LostArk::Shared::PLAYER_ACTION_STATE::NONE == snapshot.eAction &&
				 LostArk::Shared::INVALID_SKILL_ID == snapshot.iSkillId &&
				 0u == snapshot.iComboStage) :
				(0u == snapshot.iPatternBindEndTick)) &&
			Is_Valid_Cooldowns(snapshot.Cooldowns) &&
			snapshot.iComboStage <= LostArk::Shared::MAX_COMBO_STAGES &&
			(0 == snapshot.iComboStage ||
				LostArk::Shared::PLAYER_ACTION_STATE::SKILL == snapshot.eAction) &&
			(LostArk::Shared::PLAYER_ACTION_STATE::GRABBED == snapshot.eAction ?
				(snapshot.iAttachmentOwnerNetEntityId !=
					LostArk::Shared::INVALID_NET_ENTITY_ID &&
				 snapshot.iAttachmentOwnerNetEntityId != snapshot.iNetEntityId &&
				 LostArk::Shared::PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND ==
					snapshot.eAttachmentSlot &&
				 std::abs(snapshot.fAttachmentLocalOffsetX) <= 1000.f &&
				 std::abs(snapshot.fAttachmentLocalOffsetY) <= 1000.f &&
				 std::abs(snapshot.fAttachmentLocalOffsetZ) <= 1000.f &&
				 std::abs(snapshot.fAttachmentYawOffsetDegrees) <= 180.f &&
				 !snapshot.isCombatReady &&
				 LostArk::Shared::PLAYER_LOCOMOTION_STATE::IDLE ==
					snapshot.eLocomotionState) :
				(snapshot.iAttachmentOwnerNetEntityId ==
					LostArk::Shared::INVALID_NET_ENTITY_ID &&
				 LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE ==
					snapshot.eAttachmentSlot &&
				 0.f == snapshot.fAttachmentLocalOffsetX &&
				 0.f == snapshot.fAttachmentLocalOffsetY &&
				 0.f == snapshot.fAttachmentLocalOffsetZ &&
				 0.f == snapshot.fAttachmentYawOffsetDegrees)) &&
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
			 (LostArk::Shared::PLAYER_ACTION_STATE::ESTHER_CAST == snapshot.eAction &&
				snapshot.iSkillId == LostArk::Shared::INVALID_SKILL_ID &&
				0 != snapshot.iActionStartTick) ||
			 (LostArk::Shared::PLAYER_ACTION_STATE::GRABBED == snapshot.eAction &&
				snapshot.iSkillId == LostArk::Shared::INVALID_SKILL_ID &&
				0 != snapshot.iActionStartTick) ||
			 ((LostArk::Shared::PLAYER_ACTION_STATE::SKILL != snapshot.eAction &&
				LostArk::Shared::PLAYER_ACTION_STATE::TRIGGER_MOVE != snapshot.eAction &&
				LostArk::Shared::PLAYER_ACTION_STATE::FALLING != snapshot.eAction &&
				LostArk::Shared::PLAYER_ACTION_STATE::KNOCKDOWN != snapshot.eAction &&
				LostArk::Shared::PLAYER_ACTION_STATE::ESTHER_CAST != snapshot.eAction &&
				LostArk::Shared::PLAYER_ACTION_STATE::GRABBED != snapshot.eAction) &&
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

	bool Is_Valid_BoundedReason(
		const std::string& value,
		const std::size_t maximumBytes,
		const bool allowEmpty)
	{
		return value.size() <= maximumBytes &&
			(allowEmpty || !value.empty()) &&
			std::string::npos == value.find('\0');
	}

	bool Is_Valid_PresentationLaneMask(const std::uint32_t mask) noexcept
	{
		return 0u == (mask &
			~LostArk::Shared::GAMEPLAY_PRESENTATION_KNOWN_LANE_MASK);
	}

	bool Are_Valid_RequiredPinnedRevisions(
		const LostArk::Shared::GameplayDataRevision& activeRevision,
		const std::vector<LostArk::Shared::GameplayDataRevision>& revisions)
	{
		if (!activeRevision.Is_Valid() ||
			revisions.size() >
				LostArk::Shared::MAX_REQUIRED_PINNED_GAMEPLAY_REVISIONS)
		{
			return false;
		}
		for (std::size_t index = 0; index < revisions.size(); ++index)
		{
			if (!revisions[index].Is_Valid() ||
				revisions[index] == activeRevision)
			{
				return false;
			}
			for (std::size_t other = index + 1u;
				other < revisions.size(); ++other)
			{
				if (revisions[index] == revisions[other])
					return false;
			}
		}
		return true;
	}

	bool Write_RequiredPinnedRevisions(
		LostArk::Shared::CPacketWriter& writer,
		const LostArk::Shared::GameplayDataRevision& activeRevision,
		const std::vector<LostArk::Shared::GameplayDataRevision>& revisions)
	{
		if (!Are_Valid_RequiredPinnedRevisions(activeRevision, revisions) ||
			!LostArk::Shared::Write_GameplayDataRevision(writer, activeRevision))
		{
			return false;
		}
		writer.Write_U8(static_cast<std::uint8_t>(revisions.size()));
		for (const LostArk::Shared::GameplayDataRevision& revision : revisions)
		{
			if (!LostArk::Shared::Write_GameplayDataRevision(writer, revision))
				return false;
		}
		return true;
	}

	bool Read_RequiredPinnedRevisions(
		LostArk::Shared::CPacketReader& reader,
		LostArk::Shared::GameplayDataRevision& activeRevision,
		std::vector<LostArk::Shared::GameplayDataRevision>& revisions)
	{
		LostArk::Shared::GameplayDataRevision decodedActive{};
		std::uint8_t count = 0;
		if (!LostArk::Shared::Read_GameplayDataRevision(reader, decodedActive) ||
			!reader.Read_U8(count) ||
			count > LostArk::Shared::MAX_REQUIRED_PINNED_GAMEPLAY_REVISIONS)
		{
			return false;
		}
		std::vector<LostArk::Shared::GameplayDataRevision> decoded;
		decoded.reserve(count);
		for (std::uint8_t index = 0; index < count; ++index)
		{
			LostArk::Shared::GameplayDataRevision revision{};
			if (!LostArk::Shared::Read_GameplayDataRevision(reader, revision))
				return false;
			decoded.push_back(revision);
		}
		if (!Are_Valid_RequiredPinnedRevisions(decodedActive, decoded))
			return false;
		activeRevision = decodedActive;
		revisions = std::move(decoded);
		return true;
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

	bool Is_SameOrForwardTick(
		const std::uint32_t current,
		const std::uint32_t start) noexcept
	{
		return current == start ||
			static_cast<std::int32_t>(current - start) > 0;
	}

	bool Is_Default_BossCombatSnapshot(
		const LostArk::Shared::BOSS_COMBAT_SNAPSHOT& snapshot)
	{
		return 0u == snapshot.iStateRevision &&
			0u == snapshot.iAlivePartMask &&
			0u == snapshot.iFlags &&
			0u == snapshot.iCurrentStagger &&
			0u == snapshot.iMaximumStagger &&
			0u == snapshot.iCurrentShield &&
			0u == snapshot.iMaximumShield &&
			1u == snapshot.iGameplayPhase;
	}

	bool Is_Valid_BossCombatSnapshot(
		const LostArk::Shared::BOSS_COMBAT_SNAPSHOT& snapshot)
	{
		const bool hasShield = LostArk::Shared::Has_BossCombatFlag(
			snapshot.iFlags,
			LostArk::Shared::BOSS_COMBAT_STATE_FLAG::SHIELDED);
		const bool isInvulnerable = LostArk::Shared::Has_BossCombatFlag(
			snapshot.iFlags,
			LostArk::Shared::BOSS_COMBAT_STATE_FLAG::INVULNERABLE);
		const bool isGhostHidden = LostArk::Shared::Has_BossCombatFlag(
			snapshot.iFlags,
			LostArk::Shared::BOSS_COMBAT_STATE_FLAG::GHOST_HIDDEN);
		return 0u != snapshot.iStateRevision &&
			0u == (snapshot.iFlags &
				~LostArk::Shared::BOSS_COMBAT_STATE_KNOWN_FLAG_MASK) &&
			snapshot.iCurrentStagger <= snapshot.iMaximumStagger &&
			snapshot.iCurrentShield <= snapshot.iMaximumShield &&
			(hasShield == (0u != snapshot.iCurrentShield)) &&
			(!isGhostHidden ||
				(isInvulnerable && snapshot.iGameplayPhase >= 3u)) &&
			0u != snapshot.iGameplayPhase;
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
			0 != snapshot.iPhase &&
			snapshot.iBrokenArmorMask <
				(1u << LostArk::Shared::MAX_WORLD_ENTITY_ARMOR_PLATES) &&
			(snapshot.hasBossCombatState ||
			 snapshot.iPatternTargetNetEntityId ==
				LostArk::Shared::INVALID_NET_ENTITY_ID) &&
			(snapshot.hasBossCombatState ?
				(Is_Valid_BossCombatSnapshot(snapshot.BossCombat) &&
				 snapshot.iPhase == snapshot.BossCombat.iGameplayPhase) :
				Is_Default_BossCombatSnapshot(snapshot.BossCombat)) &&
			snapshot.PinnedDefinitionRevision.Is_Valid();
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

	bool Is_Valid_BossCombatEvent(
		const LostArk::Shared::BOSS_COMBAT_EVENT& event)
	{
		return 0u != event.iEventSequence &&
			0u != event.iEventTick &&
			event.iBossNetEntityId !=
				LostArk::Shared::INVALID_NET_ENTITY_ID &&
			LostArk::Shared::BOSS_COMBAT_EVENT_KIND::PART_BROKEN ==
				event.eKind &&
			0u != event.iPartMask;
	}

	bool Are_BossCombatEventsCanonical(
		const std::vector<LostArk::Shared::BOSS_COMBAT_EVENT>& events)
	{
		for (std::size_t index = 0; index < events.size(); ++index)
		{
			if (!Is_Valid_BossCombatEvent(events[index]) ||
				(0u != index &&
				 !(events[index - 1u].iEventSequence <
					events[index].iEventSequence)))
			{
				return false;
			}
		}
		return true;
	}

	bool Are_BossCombatEventsConsistent(
		const std::vector<LostArk::Shared::WORLD_ENTITY_SNAPSHOT>& entities,
		const std::vector<LostArk::Shared::BOSS_COMBAT_EVENT>& events)
	{
		for (const LostArk::Shared::BOSS_COMBAT_EVENT& event : events)
		{
			const auto boss = std::find_if(
				entities.begin(), entities.end(),
				[&event](const LostArk::Shared::WORLD_ENTITY_SNAPSHOT& entity)
				{
					return entity.iNetEntityId == event.iBossNetEntityId;
				});
			if (boss == entities.end() || !boss->hasBossCombatState ||
				0u != (boss->BossCombat.iAlivePartMask & event.iPartMask))
			{
				return false;
			}
		}
		return true;
	}

	bool Is_Valid_CombatObjectSnapshot(
		const LostArk::Shared::COMBAT_OBJECT_SNAPSHOT& snapshot)
	{
		return snapshot.iCombatObjectId !=
				LostArk::Shared::INVALID_COMBAT_OBJECT_ID &&
			snapshot.iSourceNetEntityId !=
				LostArk::Shared::INVALID_NET_ENTITY_ID &&
			std::isfinite(snapshot.fPositionX) &&
			std::isfinite(snapshot.fPositionY) &&
			std::isfinite(snapshot.fPositionZ) &&
			std::isfinite(snapshot.fYawDegrees) &&
			snapshot.PinnedDefinitionRevision.Is_Valid();
	}

	bool Are_CombatObjectSnapshotsCanonical(
		const std::vector<LostArk::Shared::COMBAT_OBJECT_SNAPSHOT>& objects)
	{
		for (std::size_t index = 0; index < objects.size(); ++index)
		{
			if (!Is_Valid_CombatObjectSnapshot(objects[index]) ||
				(0u != index &&
				 !(objects[index - 1u].iCombatObjectId <
					objects[index].iCombatObjectId)))
			{
				return false;
			}
		}
		return true;
	}

	bool Are_CombatObjectSourcesPresent(
		const std::vector<LostArk::Shared::PLAYER_SNAPSHOT>& players,
		const std::vector<LostArk::Shared::WORLD_ENTITY_SNAPSHOT>& entities,
		const std::vector<LostArk::Shared::COMBAT_OBJECT_SNAPSHOT>& objects)
	{
		for (const LostArk::Shared::COMBAT_OBJECT_SNAPSHOT& object : objects)
		{
			const bool hasPlayerSource = std::any_of(
				players.begin(), players.end(), [&object](const auto& player)
				{
					return player.iNetEntityId == object.iSourceNetEntityId;
				});
			const bool hasEntitySource = std::any_of(
				entities.begin(), entities.end(), [&object](const auto& entity)
				{
					return entity.iNetEntityId == object.iSourceNetEntityId;
				});
			if (!hasPlayerSource && !hasEntitySource)
				return false;
		}
		return true;
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

bool LostArk::Shared::Is_Valid_SequenceInstanceId(
	const std::string_view instanceId) noexcept
{
	if (instanceId.empty() ||
		instanceId.size() > MAX_SEQUENCE_INSTANCE_ID_BYTES)
	{
		return false;
	}
	for (const char character : instanceId)
	{
		const auto value = static_cast<unsigned char>(character);
		const bool allowed =
			(value >= 'a' && value <= 'z') ||
			(value >= 'A' && value <= 'Z') ||
			(value >= '0' && value <= '9') ||
			'_' == value || '-' == value || '.' == value;
		if (!allowed)
			return false;
	}
	return true;
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
		!Is_Known_World_Id(message.eWorldId) ||
		!Are_Valid_RequiredPinnedRevisions(
			message.ActiveGameplayRevision,
			message.RequiredPinnedGameplayRevisions))
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

	return Write_RequiredPinnedRevisions(
		writer,
		message.ActiveGameplayRevision,
		message.RequiredPinnedGameplayRevisions);
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

	GameplayDataRevision activeRevision{};
	std::vector<GameplayDataRevision> requiredPinnedRevisions;
	if (playerId == 0 || netEntityId == 0 ||
		!Read_RequiredPinnedRevisions(
			reader, activeRevision, requiredPinnedRevisions))
        return false;

	S2C_ENTER_ACCEPTED decoded {};

	decoded.iProtocolVersion = protocolVersion;
	decoded.eWorldId = static_cast<WORLD_ID>(rawWorldId);
	decoded.iPlayerId = playerId;
    decoded.iNetEntityId = netEntityId;
	decoded.ActiveGameplayRevision = activeRevision;
	decoded.RequiredPinnedGameplayRevisions =
		std::move(requiredPinnedRevisions);
   
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
		(INVALID_NET_ENTITY_ID != spawned.iOwnerBossNetEntityId &&
			(WORLD_ENTITY_KIND::BOSS != spawned.eKind ||
			 spawned.iOwnerBossNetEntityId == spawned.iNetEntityId)) ||
		!Is_Valid_StableId(spawned.strArchetypeId, false) ||
		!Is_Valid_StableId(spawned.strEncounterId, true) ||
		!Is_Valid_StableId(spawned.strPlacementId, true) ||
		!Is_Valid_StableId(spawned.strActionId, true) ||
		!std::isfinite(spawned.fPositionX) ||
		!std::isfinite(spawned.fPositionY) ||
		!std::isfinite(spawned.fPositionZ) ||
		!std::isfinite(spawned.fYawDegrees) ||
		!std::isfinite(spawned.fCollisionRadius) ||
		!spawned.PinnedDefinitionRevision.Is_Valid() ||
		(WORLD_ENTITY_KIND::NPC == spawned.eKind &&
			0.f != spawned.fCollisionRadius) ||
		(WORLD_ENTITY_KIND::NPC != spawned.eKind &&
			spawned.fCollisionRadius <= 0.f))
	{
		return false;
	}
	writer.Write_U32(spawned.iNetEntityId);
	writer.Write_U32(spawned.iOwnerBossNetEntityId);
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
	return Write_GameplayDataRevision(
		writer, spawned.PinnedDefinitionRevision);
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_WORLD_ENTITY_SPAWNED& spawned)
{
	S2C_WORLD_ENTITY_SPAWNED decoded{};
	std::uint8_t rawKind = 0;
	if (!reader.Read_U32(decoded.iNetEntityId) ||
		!reader.Read_U32(decoded.iOwnerBossNetEntityId) ||
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
		!reader.Read_F32(decoded.fCollisionRadius) ||
		!Read_GameplayDataRevision(
			reader, decoded.PinnedDefinitionRevision))
	{
		return false;
	}
	decoded.eKind = static_cast<WORLD_ENTITY_KIND>(rawKind);
	if (decoded.iNetEntityId == INVALID_NET_ENTITY_ID ||
		!Is_Valid_WorldEntityKind(decoded.eKind) ||
		(INVALID_NET_ENTITY_ID != decoded.iOwnerBossNetEntityId &&
			(WORLD_ENTITY_KIND::BOSS != decoded.eKind ||
			 decoded.iOwnerBossNetEntityId == decoded.iNetEntityId)) ||
		!Is_Valid_StableId(decoded.strArchetypeId, false) ||
		!Is_Valid_StableId(decoded.strEncounterId, true) ||
		!Is_Valid_StableId(decoded.strPlacementId, true) ||
		!Is_Valid_StableId(decoded.strActionId, true) ||
		!std::isfinite(decoded.fPositionX) ||
		!std::isfinite(decoded.fPositionY) ||
		!std::isfinite(decoded.fPositionZ) ||
		!std::isfinite(decoded.fYawDegrees) ||
		!std::isfinite(decoded.fCollisionRadius) ||
		!decoded.PinnedDefinitionRevision.Is_Valid() ||
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

bool LostArk::Shared::Is_Valid_WorldEntitySpawnOwner(
	const S2C_WORLD_ENTITY_SPAWNED& spawned,
	const S2C_WORLD_ENTITY_SPAWNED* pOwner)
{
	if (INVALID_NET_ENTITY_ID == spawned.iNetEntityId ||
		!Is_Valid_WorldEntityKind(spawned.eKind) ||
		!spawned.PinnedDefinitionRevision.Is_Valid())
	{
		return false;
	}
	if (INVALID_NET_ENTITY_ID == spawned.iOwnerBossNetEntityId)
		return nullptr == pOwner;
	return WORLD_ENTITY_KIND::BOSS == spawned.eKind &&
		spawned.iOwnerBossNetEntityId != spawned.iNetEntityId &&
		nullptr != pOwner &&
		spawned.iOwnerBossNetEntityId == pOwner->iNetEntityId &&
		WORLD_ENTITY_KIND::BOSS == pOwner->eKind &&
		INVALID_NET_ENTITY_ID == pOwner->iOwnerBossNetEntityId &&
		pOwner->PinnedDefinitionRevision.Is_Valid() &&
		spawned.PinnedDefinitionRevision ==
			pOwner->PinnedDefinitionRevision &&
		!spawned.strEncounterId.empty() &&
		spawned.strEncounterId == pOwner->strEncounterId;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_WORLD_ENTITY_DESPAWNED& despawned)
{
	if (INVALID_NET_ENTITY_ID == despawned.iNetEntityId ||
		(despawned.eReason != WORLD_ENTITY_DESPAWN_REASON::REMOVED &&
		 despawned.eReason != WORLD_ENTITY_DESPAWN_REASON::DEAD))
	{
		return false;
	}
	writer.Write_U32(despawned.iNetEntityId);
	writer.Write_U8(static_cast<std::uint8_t>(despawned.eReason));
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_WORLD_ENTITY_DESPAWNED& despawned)
{
	S2C_WORLD_ENTITY_DESPAWNED decoded{};
	std::uint8_t reason = 0u;
	if (!reader.Read_U32(decoded.iNetEntityId) || !reader.Read_U8(reason) ||
		INVALID_NET_ENTITY_ID == decoded.iNetEntityId ||
		reason >= static_cast<std::uint8_t>(WORLD_ENTITY_DESPAWN_REASON::END))
	{
		return false;
	}
	decoded.eReason = static_cast<WORLD_ENTITY_DESPAWN_REASON>(reason);
	despawned = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_COMBAT_OBJECT_SPAWNED& spawned)
{
	if (INVALID_COMBAT_OBJECT_ID == spawned.iCombatObjectId ||
		INVALID_NET_ENTITY_ID == spawned.iSourceNetEntityId ||
		0u == spawned.iSpawnTick ||
		0u == spawned.iServerTick ||
		!Is_SameOrForwardTick(spawned.iServerTick, spawned.iSpawnTick) ||
		!Is_Valid_StableId(spawned.strCombatObjectArchetypeId, false) ||
		!Is_Valid_StableId(spawned.strClientVisualId, false) ||
		!std::isfinite(spawned.fPositionX) ||
		!std::isfinite(spawned.fPositionY) ||
		!std::isfinite(spawned.fPositionZ) ||
		!std::isfinite(spawned.fYawDegrees) ||
		!spawned.PinnedDefinitionRevision.Is_Valid())
	{
		return false;
	}

	Write_U64(writer, spawned.iCombatObjectId);
	writer.Write_U32(spawned.iSourceNetEntityId);
	writer.Write_U32(spawned.iSpawnTick);
	writer.Write_U32(spawned.iServerTick);
	if (!writer.Write_String(
			spawned.strCombatObjectArchetypeId,
			MAX_STABLE_NETWORK_ID_BYTES) ||
		!writer.Write_String(
			spawned.strClientVisualId,
			MAX_STABLE_NETWORK_ID_BYTES))
	{
		return false;
	}
	writer.Write_F32(spawned.fPositionX);
	writer.Write_F32(spawned.fPositionY);
	writer.Write_F32(spawned.fPositionZ);
	writer.Write_F32(spawned.fYawDegrees);
	return Write_GameplayDataRevision(
		writer, spawned.PinnedDefinitionRevision);
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_COMBAT_OBJECT_SPAWNED& spawned)
{
	S2C_COMBAT_OBJECT_SPAWNED decoded{};
	if (!Read_U64(reader, decoded.iCombatObjectId) ||
		!reader.Read_U32(decoded.iSourceNetEntityId) ||
		!reader.Read_U32(decoded.iSpawnTick) ||
		!reader.Read_U32(decoded.iServerTick) ||
		!reader.Read_String(
			decoded.strCombatObjectArchetypeId,
			MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_String(
			decoded.strClientVisualId,
			MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_F32(decoded.fPositionX) ||
		!reader.Read_F32(decoded.fPositionY) ||
		!reader.Read_F32(decoded.fPositionZ) ||
		!reader.Read_F32(decoded.fYawDegrees) ||
		!Read_GameplayDataRevision(
			reader, decoded.PinnedDefinitionRevision) ||
		INVALID_COMBAT_OBJECT_ID == decoded.iCombatObjectId ||
		INVALID_NET_ENTITY_ID == decoded.iSourceNetEntityId ||
		0u == decoded.iSpawnTick ||
		0u == decoded.iServerTick ||
		!Is_SameOrForwardTick(decoded.iServerTick, decoded.iSpawnTick) ||
		!Is_Valid_StableId(decoded.strCombatObjectArchetypeId, false) ||
		!Is_Valid_StableId(decoded.strClientVisualId, false) ||
		!std::isfinite(decoded.fPositionX) ||
		!std::isfinite(decoded.fPositionY) ||
		!std::isfinite(decoded.fPositionZ) ||
		!std::isfinite(decoded.fYawDegrees) ||
		!decoded.PinnedDefinitionRevision.Is_Valid())
	{
		return false;
	}

	spawned = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_COMBAT_OBJECT_DESPAWNED& despawned)
{
	if (INVALID_COMBAT_OBJECT_ID == despawned.iCombatObjectId)
		return false;
	Write_U64(writer, despawned.iCombatObjectId);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_COMBAT_OBJECT_DESPAWNED& despawned)
{
	S2C_COMBAT_OBJECT_DESPAWNED decoded{};
	if (!Read_U64(reader, decoded.iCombatObjectId) ||
		INVALID_COMBAT_OBJECT_ID == decoded.iCombatObjectId)
	{
		return false;
	}
	despawned = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_COMBAT_OBJECT_PRESENTATION_EVENT& event)
{
	if (0u == event.iEventSequence || 0u == event.iServerTick ||
		INVALID_COMBAT_OBJECT_ID == event.iCombatObjectId ||
		INVALID_NET_ENTITY_ID == event.iSourceNetEntityId ||
		COMBAT_OBJECT_PRESENTATION_EVENT_KIND::HIT_PULSE != event.eKind ||
		!Is_Valid_StableId(event.strCombatObjectArchetypeId, false) ||
		!Is_Valid_StableId(event.strOwnerPatternId, false) ||
		!Is_Valid_StableId(event.strOwnerStageActionId, false) ||
		!Is_Valid_StableId(event.strHitId, false) ||
		event.iRepeatIndex >= 64u ||
		!std::isfinite(event.fPositionX) ||
		!std::isfinite(event.fPositionY) ||
		!std::isfinite(event.fPositionZ) ||
		!std::isfinite(event.fYawDegrees) ||
		!event.PinnedDefinitionRevision.Is_Valid())
	{
		return false;
	}

	Write_U64(writer, event.iEventSequence);
	writer.Write_U32(event.iServerTick);
	Write_U64(writer, event.iCombatObjectId);
	writer.Write_U32(event.iSourceNetEntityId);
	writer.Write_U8(static_cast<std::uint8_t>(event.eKind));
	if (!writer.Write_String(event.strCombatObjectArchetypeId,
			MAX_STABLE_NETWORK_ID_BYTES) ||
		!writer.Write_String(event.strOwnerPatternId,
			MAX_STABLE_NETWORK_ID_BYTES) ||
		!writer.Write_String(event.strOwnerStageActionId,
			MAX_STABLE_NETWORK_ID_BYTES) ||
		!writer.Write_String(event.strHitId,
			MAX_STABLE_NETWORK_ID_BYTES))
	{
		return false;
	}
	writer.Write_U32(event.iRepeatIndex);
	writer.Write_F32(event.fPositionX);
	writer.Write_F32(event.fPositionY);
	writer.Write_F32(event.fPositionZ);
	writer.Write_F32(event.fYawDegrees);
	return Write_GameplayDataRevision(writer, event.PinnedDefinitionRevision);
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_COMBAT_OBJECT_PRESENTATION_EVENT& event)
{
	S2C_COMBAT_OBJECT_PRESENTATION_EVENT decoded{};
	std::uint8_t rawKind = 0u;
	if (!Read_U64(reader, decoded.iEventSequence) ||
		!reader.Read_U32(decoded.iServerTick) ||
		!Read_U64(reader, decoded.iCombatObjectId) ||
		!reader.Read_U32(decoded.iSourceNetEntityId) ||
		!reader.Read_U8(rawKind) ||
		!reader.Read_String(decoded.strCombatObjectArchetypeId,
			MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_String(decoded.strOwnerPatternId,
			MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_String(decoded.strOwnerStageActionId,
			MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_String(decoded.strHitId,
			MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_U32(decoded.iRepeatIndex) ||
		!reader.Read_F32(decoded.fPositionX) ||
		!reader.Read_F32(decoded.fPositionY) ||
		!reader.Read_F32(decoded.fPositionZ) ||
		!reader.Read_F32(decoded.fYawDegrees) ||
		!Read_GameplayDataRevision(reader, decoded.PinnedDefinitionRevision))
	{
		return false;
	}
	decoded.eKind =
		static_cast<COMBAT_OBJECT_PRESENTATION_EVENT_KIND>(rawKind);
	if (0u == decoded.iEventSequence || 0u == decoded.iServerTick ||
		INVALID_COMBAT_OBJECT_ID == decoded.iCombatObjectId ||
		INVALID_NET_ENTITY_ID == decoded.iSourceNetEntityId ||
		COMBAT_OBJECT_PRESENTATION_EVENT_KIND::HIT_PULSE != decoded.eKind ||
		!Is_Valid_StableId(decoded.strCombatObjectArchetypeId, false) ||
		!Is_Valid_StableId(decoded.strOwnerPatternId, false) ||
		!Is_Valid_StableId(decoded.strOwnerStageActionId, false) ||
		!Is_Valid_StableId(decoded.strHitId, false) ||
		decoded.iRepeatIndex >= 64u ||
		!std::isfinite(decoded.fPositionX) ||
		!std::isfinite(decoded.fPositionY) ||
		!std::isfinite(decoded.fPositionZ) ||
		!std::isfinite(decoded.fYawDegrees) ||
		!decoded.PinnedDefinitionRevision.Is_Valid())
	{
		return false;
	}
	event = std::move(decoded);
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
		!Is_Valid_SkillTargetIntent(message.eTargetIntent) ||
		!std::isfinite(message.fAimX) ||
		!std::isfinite(message.fAimZ))
	{
		return false;
	}

	writer.Write_U32(message.iClientSequence);
	writer.Write_U32(message.iSkillId);
	writer.Write_U8(static_cast<std::uint8_t>(message.eTargetIntent));
	writer.Write_F32(message.fAimX);
	writer.Write_F32(message.fAimZ);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_USE_SKILL& message)
{
	C2S_USE_SKILL decoded{};
	std::uint8_t rawTargetIntent = 0;
	if (!reader.Read_U32(decoded.iClientSequence) ||
		!reader.Read_U32(decoded.iSkillId) ||
		!reader.Read_U8(rawTargetIntent) ||
		!reader.Read_F32(decoded.fAimX) ||
		!reader.Read_F32(decoded.fAimZ) ||
		0 == decoded.iClientSequence ||
		INVALID_SKILL_ID == decoded.iSkillId ||
		!Is_Valid_SkillTargetIntent(
			static_cast<SKILL_TARGET_INTENT_KIND>(rawTargetIntent)) ||
		!std::isfinite(decoded.fAimX) ||
		!std::isfinite(decoded.fAimZ))
	{
		return false;
	}
	decoded.eTargetIntent =
		static_cast<SKILL_TARGET_INTENT_KIND>(rawTargetIntent);

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
	const C2S_DEBUG_KILL_SELF& message)
{
	if (0u == message.iClientSequence)
		return false;
	writer.Write_U32(message.iClientSequence);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_DEBUG_KILL_SELF& message)
{
	C2S_DEBUG_KILL_SELF decoded{};
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
	const C2S_DEBUG_ENTER_KAKULSAYDON_ARENA& message)
{
	if (0u == message.iRequestSequence)
		return false;
	writer.Write_U32(message.iRequestSequence);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_DEBUG_ENTER_KAKULSAYDON_ARENA& message)
{
	C2S_DEBUG_ENTER_KAKULSAYDON_ARENA decoded{};
	if (!reader.Read_U32(decoded.iRequestSequence) ||
		0u == decoded.iRequestSequence)
	{
		return false;
	}
	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_DEBUG_TELEPORT_TO_PLACEMENT& message)
{
	if (0u == message.iRequestSequence ||
		!Is_Valid_StableId(message.strPlacementId, false))
	{
		return false;
	}
	writer.Write_U32(message.iRequestSequence);
	return writer.Write_String(
		message.strPlacementId, MAX_STABLE_NETWORK_ID_BYTES);
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_DEBUG_TELEPORT_TO_PLACEMENT& message)
{
	C2S_DEBUG_TELEPORT_TO_PLACEMENT decoded{};
	if (!reader.Read_U32(decoded.iRequestSequence) ||
		0u == decoded.iRequestSequence ||
		!reader.Read_String(
			decoded.strPlacementId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!Is_Valid_StableId(decoded.strPlacementId, false))
	{
		return false;
	}
	message = std::move(decoded);
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
		message.BossCombatEvents.size() > MAX_BOSS_COMBAT_EVENTS ||
		message.CombatObjects.size() > MAX_COMBAT_OBJECTS_PER_SNAPSHOT ||
		!Are_Valid_RequiredPinnedRevisions(
			message.ActiveGameplayRevision,
			message.RequiredPinnedGameplayRevisions) ||
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
	if (!Are_BossCombatEventsCanonical(message.BossCombatEvents) ||
		!Are_BossCombatEventsConsistent(
			message.Entities, message.BossCombatEvents))
		return false;
	if (!Are_CombatObjectSnapshotsCanonical(message.CombatObjects) ||
		!Are_CombatObjectSourcesPresent(
			message.Players, message.Entities, message.CombatObjects))
		return false;
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
	writer.Write_U8(
		static_cast<std::uint8_t>(message.BossCombatEvents.size()));
	writer.Write_U8(
		static_cast<std::uint8_t>(message.CombatObjects.size()));
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
		writer.Write_U32(player.iAttachmentOwnerNetEntityId);
		writer.Write_U8(static_cast<std::uint8_t>(player.eAttachmentSlot));
		writer.Write_F32(player.fAttachmentLocalOffsetX);
		writer.Write_F32(player.fAttachmentLocalOffsetY);
		writer.Write_F32(player.fAttachmentLocalOffsetZ);
		writer.Write_F32(player.fAttachmentYawOffsetDegrees);
		writer.Write_U8(player.hasSkillTarget ? 1u : 0u);
		writer.Write_F32(player.fSkillTargetX);
		writer.Write_F32(player.fSkillTargetY);
		writer.Write_F32(player.fSkillTargetZ);
		writer.Write_U32(player.iCurrentHp);
		writer.Write_U32(player.iMaximumHp);
		writer.Write_U32(player.iCurrentResource);
		writer.Write_U32(player.iMaximumResource);
		writer.Write_U32(player.iCurrentIdentity);
		writer.Write_U32(player.iMaximumIdentity);
		writer.Write_U8(player.isCombatReady ? 1u : 0u);
		writer.Write_U8(player.isPatternBound ? 1u : 0u);
		writer.Write_U32(player.iPatternBindEndTick);
		writer.Write_U32(player.iSilenceEndTick);
		writer.Write_U32(player.iSilenceDurationTicks);
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
		writer.Write_U32(entity.iPatternTargetNetEntityId);
		writer.Write_U32(entity.iCurrentHp);
		writer.Write_U32(entity.iMaximumHp);
		writer.Write_U8(entity.iPhase);
		writer.Write_U8(entity.iBrokenArmorMask);
		writer.Write_U8(entity.hasBossCombatState ? 1u : 0u);
		if (entity.hasBossCombatState)
		{
			writer.Write_U32(entity.BossCombat.iStateRevision);
			writer.Write_U32(entity.BossCombat.iAlivePartMask);
			writer.Write_U16(entity.BossCombat.iFlags);
			writer.Write_U32(entity.BossCombat.iCurrentStagger);
			writer.Write_U32(entity.BossCombat.iMaximumStagger);
			writer.Write_U32(entity.BossCombat.iCurrentShield);
			writer.Write_U32(entity.BossCombat.iMaximumShield);
			writer.Write_U8(entity.BossCombat.iGameplayPhase);
		}
		if (!Write_GameplayDataRevision(
			writer, entity.PinnedDefinitionRevision))
		{
			return false;
		}
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
	for (const BOSS_COMBAT_EVENT& event : message.BossCombatEvents)
	{
		Write_U64(writer, event.iEventSequence);
		writer.Write_U32(event.iEventTick);
		writer.Write_U32(event.iBossNetEntityId);
		writer.Write_U8(static_cast<std::uint8_t>(event.eKind));
		writer.Write_U32(event.iPartMask);
	}
	for (const COMBAT_OBJECT_SNAPSHOT& object : message.CombatObjects)
	{
		Write_U64(writer, object.iCombatObjectId);
		writer.Write_U32(object.iSourceNetEntityId);
		writer.Write_F32(object.fPositionX);
		writer.Write_F32(object.fPositionY);
		writer.Write_F32(object.fPositionZ);
		writer.Write_F32(object.fYawDegrees);
		if (!Write_GameplayDataRevision(
			writer, object.PinnedDefinitionRevision))
		{
			return false;
		}
	}

	return Write_RequiredPinnedRevisions(
		writer,
		message.ActiveGameplayRevision,
		message.RequiredPinnedGameplayRevisions);
}

bool LostArk::Shared::Read_Message(CPacketReader& reader, S2C_WORLD_SNAPSHOT& message)
{
	std::uint32_t serverTick = 0;
	std::uint16_t rawWorldId = 0;
	std::uint16_t playerCount = 0;
	std::uint16_t entityCount = 0;
	std::uint8_t damageEventCount = 0;
	std::uint8_t bossCombatEventCount = 0;
	std::uint8_t combatObjectCount = 0;
	std::uint32_t estherGauge = 0;
	std::uint32_t estherGaugeMaximum = 0;

	if (!reader.Read_U32(serverTick) ||
		!reader.Read_U16(rawWorldId) ||
		!reader.Read_U16(playerCount) ||
		!reader.Read_U16(entityCount) ||
		!reader.Read_U8(damageEventCount) ||
		!reader.Read_U8(bossCombatEventCount) ||
		!reader.Read_U8(combatObjectCount) ||
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
		bossCombatEventCount > MAX_BOSS_COMBAT_EVENTS ||
		combatObjectCount > MAX_COMBAT_OBJECTS_PER_SNAPSHOT ||
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
	decoded.BossCombatEvents.reserve(bossCombatEventCount);
	decoded.CombatObjects.reserve(combatObjectCount);

    for (std::uint16_t i = 0; i < playerCount; ++i)
    {
        PLAYER_SNAPSHOT player{};
		std::uint8_t rawCharacterClass = 0;
        std::uint8_t rawLocomotion = 0;
		std::uint8_t rawAction = 0;
		std::uint8_t rawStance = 0;
		std::uint8_t rawAttachmentSlot = 0;
		std::uint8_t rawHasSkillTarget = 0;
		std::uint8_t rawCombatReady = 0;
		std::uint8_t rawPatternBound = 0;
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
			!reader.Read_U32(player.iAttachmentOwnerNetEntityId) ||
			!reader.Read_U8(rawAttachmentSlot) ||
			!reader.Read_F32(player.fAttachmentLocalOffsetX) ||
			!reader.Read_F32(player.fAttachmentLocalOffsetY) ||
			!reader.Read_F32(player.fAttachmentLocalOffsetZ) ||
			!reader.Read_F32(player.fAttachmentYawOffsetDegrees) ||
			!reader.Read_U8(rawHasSkillTarget) ||
			rawHasSkillTarget > 1u ||
			!reader.Read_F32(player.fSkillTargetX) ||
			!reader.Read_F32(player.fSkillTargetY) ||
			!reader.Read_F32(player.fSkillTargetZ) ||
			!reader.Read_U32(player.iCurrentHp) ||
			!reader.Read_U32(player.iMaximumHp) ||
			!reader.Read_U32(player.iCurrentResource) ||
			!reader.Read_U32(player.iMaximumResource) ||
			!reader.Read_U32(player.iCurrentIdentity) ||
			!reader.Read_U32(player.iMaximumIdentity) ||
			!reader.Read_U8(rawCombatReady) ||
			rawCombatReady > 1u ||
			!reader.Read_U8(rawPatternBound) ||
			rawPatternBound > 1u ||
			!reader.Read_U32(player.iPatternBindEndTick) ||
			!reader.Read_U32(player.iSilenceEndTick) ||
			!reader.Read_U32(player.iSilenceDurationTicks) ||
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
		player.eAttachmentSlot =
			static_cast<PLAYER_ATTACHMENT_SLOT>(rawAttachmentSlot);
		player.hasSkillTarget = 0u != rawHasSkillTarget;
		player.isCombatReady = 0u != rawCombatReady;
		player.isPatternBound = 0u != rawPatternBound;
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
		std::uint8_t rawHasBossCombatState = 0;
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
			!reader.Read_U32(entity.iPatternTargetNetEntityId) ||
			!reader.Read_U32(entity.iCurrentHp) ||
			!reader.Read_U32(entity.iMaximumHp) ||
			!reader.Read_U8(entity.iPhase) ||
			!reader.Read_U8(entity.iBrokenArmorMask) ||
			!reader.Read_U8(rawHasBossCombatState) ||
			rawHasBossCombatState > 1u)
		{
			return false;
		}
		entity.hasBossCombatState = 0u != rawHasBossCombatState;
		if (entity.hasBossCombatState &&
			(!reader.Read_U32(entity.BossCombat.iStateRevision) ||
			 !reader.Read_U32(entity.BossCombat.iAlivePartMask) ||
			 !reader.Read_U16(entity.BossCombat.iFlags) ||
			 !reader.Read_U32(entity.BossCombat.iCurrentStagger) ||
			 !reader.Read_U32(entity.BossCombat.iMaximumStagger) ||
			 !reader.Read_U32(entity.BossCombat.iCurrentShield) ||
			 !reader.Read_U32(entity.BossCombat.iMaximumShield) ||
			 !reader.Read_U8(entity.BossCombat.iGameplayPhase)))
		{
			return false;
		}
		if (!Read_GameplayDataRevision(
			reader, entity.PinnedDefinitionRevision))
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
	for (std::uint8_t index = 0; index < bossCombatEventCount; ++index)
	{
		BOSS_COMBAT_EVENT event{};
		std::uint8_t rawKind = 0;
		if (!Read_U64(reader, event.iEventSequence) ||
			!reader.Read_U32(event.iEventTick) ||
			!reader.Read_U32(event.iBossNetEntityId) ||
			!reader.Read_U8(rawKind) ||
			!reader.Read_U32(event.iPartMask))
		{
			return false;
		}
		event.eKind = static_cast<BOSS_COMBAT_EVENT_KIND>(rawKind);
		decoded.BossCombatEvents.push_back(event);
	}
	if (!Are_BossCombatEventsCanonical(decoded.BossCombatEvents) ||
		!Are_BossCombatEventsConsistent(
			decoded.Entities, decoded.BossCombatEvents))
		return false;

	for (std::uint8_t index = 0; index < combatObjectCount; ++index)
	{
		COMBAT_OBJECT_SNAPSHOT object{};
		if (!Read_U64(reader, object.iCombatObjectId) ||
			!reader.Read_U32(object.iSourceNetEntityId) ||
			!reader.Read_F32(object.fPositionX) ||
			!reader.Read_F32(object.fPositionY) ||
			!reader.Read_F32(object.fPositionZ) ||
			!reader.Read_F32(object.fYawDegrees) ||
			!Read_GameplayDataRevision(
				reader, object.PinnedDefinitionRevision))
		{
			return false;
		}
		decoded.CombatObjects.push_back(object);
	}
	if (!Are_CombatObjectSnapshotsCanonical(decoded.CombatObjects) ||
		!Are_CombatObjectSourcesPresent(
			decoded.Players, decoded.Entities, decoded.CombatObjects))
	{
		return false;
	}
	if (!Read_RequiredPinnedRevisions(
		reader,
		decoded.ActiveGameplayRevision,
		decoded.RequiredPinnedGameplayRevisions))
	{
		return false;
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

	bool Is_NextAuditionOperation(const std::uint8_t rawOperation)
	{
		return static_cast<std::uint8_t>(
			VALTAN_AUDITION_OPERATION::QUEUE_NEXT_PATTERN_ID) == rawOperation ||
			static_cast<std::uint8_t>(
				VALTAN_AUDITION_OPERATION::CLEAR_NEXT_PATTERN_ID) == rawOperation ||
			static_cast<std::uint8_t>(
				VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID) == rawOperation;
	}

	bool Is_RestartAuditionOperation(const std::uint8_t rawOperation)
	{
		return static_cast<std::uint8_t>(
			VALTAN_AUDITION_OPERATION::RESTART_PATTERN_ID) == rawOperation;
	}

	bool Is_PlayPatternIdAuditionOperation(const std::uint8_t rawOperation)
	{
		return static_cast<std::uint8_t>(
			VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID) == rawOperation;
	}

	bool Is_StableAuditionOperation(const std::uint8_t rawOperation)
	{
		return Is_NextAuditionOperation(rawOperation) ||
			Is_RestartAuditionOperation(rawOperation) ||
			Is_PlayPatternIdAuditionOperation(rawOperation);
	}

	// A request sequence of zero can never be told apart from a default-built
	// struct, so it is not a usable duplicate key. Health bar zero is the dead
	// boss and carries no authored pattern.
	bool Is_Valid_AuditionRequest(
		const std::uint32_t requestSequence,
		const std::uint8_t rawOperation,
		const std::uint32_t targetHealthBar,
		const std::string& bossPlacementId,
		const std::string& patternId,
		const std::uint32_t predecessorEpoch,
		const std::uint32_t predecessorPatternSequence,
		const std::uint32_t expectedNextRequestSequence,
		const GameplayDataRevision& expectedDefinitionRevision,
		const GameplayDataRevision& replacementDefinitionRevision)
	{
		if (0u == requestSequence ||
			rawOperation >= static_cast<std::uint8_t>(
				VALTAN_AUDITION_OPERATION::END))
		{
			return false;
		}
		if (Is_RestartAuditionOperation(rawOperation))
		{
			return 0u == targetHealthBar && 0u != predecessorEpoch &&
				0u != predecessorPatternSequence &&
				0u == expectedNextRequestSequence &&
				expectedDefinitionRevision.Is_Valid() &&
				replacementDefinitionRevision.Is_Valid() &&
				Is_Valid_StableId(bossPlacementId, false) &&
				Is_Valid_StableId(patternId, false);
		}
		if (replacementDefinitionRevision.Is_Valid())
			return false;
		if (Is_PlayPatternIdAuditionOperation(rawOperation))
		{
			return 0u == targetHealthBar && 0u == predecessorEpoch &&
				0u == predecessorPatternSequence &&
				0u == expectedNextRequestSequence &&
				expectedDefinitionRevision.Is_Valid() &&
				Is_Valid_StableId(bossPlacementId, false) &&
				Is_Valid_StableId(patternId, false);
		}
		if (static_cast<std::uint8_t>(
				VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID) == rawOperation)
		{
			return 0u == targetHealthBar && 0u == predecessorEpoch &&
				0u == expectedNextRequestSequence &&
				expectedDefinitionRevision.Is_Valid() &&
				Is_Valid_StableId(bossPlacementId, false) &&
				Is_Valid_StableId(patternId, false);
		}
		if (Is_NextAuditionOperation(rawOperation))
		{
			return 0u == targetHealthBar && 0u != predecessorEpoch &&
				0u != predecessorPatternSequence &&
				expectedDefinitionRevision.Is_Valid() &&
				(static_cast<std::uint8_t>(
					VALTAN_AUDITION_OPERATION::CLEAR_NEXT_PATTERN_ID) != rawOperation ||
				 0u != expectedNextRequestSequence) &&
				Is_Valid_StableId(bossPlacementId, false) &&
				Is_Valid_StableId(patternId, false);
		}
		if (0u != predecessorEpoch || 0u != predecessorPatternSequence ||
			0u != expectedNextRequestSequence ||
			expectedDefinitionRevision.Is_Valid())
		{
			return false;
		}
		/* Legacy operations retain their exact wire shape. Rejecting hidden
		   stable-ID data here prevents a caller from believing those IDs were
		   transported when their operation intentionally does not encode them. */
		if (!bossPlacementId.empty() || !patternId.empty())
			return false;
		// A selected timeline row is addressed by its stable non-zero command ID;
		// STOP names no row and therefore carries exactly zero.
		if (static_cast<std::uint8_t>(
				VALTAN_AUDITION_OPERATION::PLAY_TIMELINE_ROW) == rawOperation)
		{
			return 0u != targetHealthBar;
		}
		if (static_cast<std::uint8_t>(
				VALTAN_AUDITION_OPERATION::START_FIGHT_PAGE) == rawOperation)
		{
			return 0u != targetHealthBar;
		}
		if (static_cast<std::uint8_t>(
				VALTAN_AUDITION_OPERATION::STOP_TIMELINE_ROW) == rawOperation)
		{
			return 0u == targetHealthBar;
		}
		if (static_cast<std::uint8_t>(
				VALTAN_AUDITION_OPERATION::SET_ARENA_PRESET) == rawOperation)
		{
			return targetHealthBar >= static_cast<std::uint32_t>(
					VALTAN_ARENA_PRESET::FRESH) &&
				targetHealthBar < static_cast<std::uint32_t>(
					VALTAN_ARENA_PRESET::END);
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
				VALTAN_AUDITION_OPERATION::BREAK_EVERY_WALL) == rawOperation)
		{
			return 0u == targetHealthBar;
		}
		return 0u != targetHealthBar;
	}

	bool Is_Valid_AuditionResult(
		const std::uint8_t rawOperation, const std::uint8_t rawResult)
	{
		if (rawResult >= static_cast<std::uint8_t>(VALTAN_AUDITION_RESULT::END))
			return false;
		if (Is_RestartAuditionOperation(rawOperation))
		{
			const auto result = static_cast<VALTAN_AUDITION_RESULT>(rawResult);
			return VALTAN_AUDITION_RESULT::QUEUED == result ||
				VALTAN_AUDITION_RESULT::REJECTED_OCCURRENCE_PRESERVED == result ||
				VALTAN_AUDITION_RESULT::REJECTED_RELEASE_BUILD == result ||
				VALTAN_AUDITION_RESULT::REJECTED_WRONG_WORLD == result ||
				VALTAN_AUDITION_RESULT::REJECTED_NO_BOSS == result ||
				VALTAN_AUDITION_RESULT::REJECTED_BOSS_DEAD == result ||
				VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE == result ||
				VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED == result ||
				VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST == result ||
				VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION == result ||
				VALTAN_AUDITION_RESULT::REJECTED_NOT_OWNER == result;
		}
		if (static_cast<std::uint8_t>(
				VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID) == rawOperation &&
			VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST ==
				static_cast<VALTAN_AUDITION_RESULT>(rawResult))
		{
			return true;
		}
		if (!Is_NextAuditionOperation(rawOperation))
			return rawResult < static_cast<std::uint8_t>(VALTAN_AUDITION_RESULT::CLEARED);
		const auto result = static_cast<VALTAN_AUDITION_RESULT>(rawResult);
		if (VALTAN_AUDITION_RESULT::ARMED == result ||
			VALTAN_AUDITION_RESULT::DUPLICATE_IGNORED == result ||
			VALTAN_AUDITION_RESULT::REJECTED_UNKNOWN_HEALTH_BAR == result ||
			VALTAN_AUDITION_RESULT::REJECTED_NOT_ARMED == result ||
			VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED == result)
		{
			return false;
		}
		const bool clear = static_cast<std::uint8_t>(
			VALTAN_AUDITION_OPERATION::CLEAR_NEXT_PATTERN_ID) == rawOperation;
		return !(clear && VALTAN_AUDITION_RESULT::QUEUED == result) &&
			!(!clear && VALTAN_AUDITION_RESULT::CLEARED == result);
	}

	template <typename TMessage>
	bool Write_AuditionIdentity(
		CPacketWriter& writer, const TMessage& message, const std::uint8_t operation)
	{
		if (!Is_StableAuditionOperation(operation))
			return true;
		if (!writer.Write_String(message.strBossPlacementId, MAX_STABLE_NETWORK_ID_BYTES) ||
			!writer.Write_String(message.strPatternId, MAX_STABLE_NETWORK_ID_BYTES))
		{
			return false;
		}
		if (Is_NextAuditionOperation(operation))
		{
			writer.Write_U32(message.iPredecessorRoomAuditionEpoch);
			writer.Write_U32(message.iPredecessorPatternSequence);
			writer.Write_U32(message.iExpectedNextRequestSequence);
			return Write_GameplayDataRevision(
				writer, message.ExpectedDefinitionRevision);
		}
		else if (Is_RestartAuditionOperation(operation))
		{
			writer.Write_U32(message.iPredecessorRoomAuditionEpoch);
			writer.Write_U32(message.iPredecessorPatternSequence);
			if (!Write_GameplayDataRevision(
					writer, message.ExpectedDefinitionRevision))
			{
				return false;
			}
			return Write_GameplayDataRevision(
				writer, message.ReplacementDefinitionRevision);
		}
		else if (Is_PlayPatternIdAuditionOperation(operation) &&
			!Write_GameplayDataRevision(
				writer, message.ExpectedDefinitionRevision))
		{
			return false;
		}
		return true;
	}

	template <typename TMessage>
	bool Read_AuditionIdentity(
		CPacketReader& reader, TMessage& message, const std::uint8_t operation)
	{
		if (!Is_StableAuditionOperation(operation))
			return true;
		if (!reader.Read_String(message.strBossPlacementId, MAX_STABLE_NETWORK_ID_BYTES) ||
			!reader.Read_String(message.strPatternId, MAX_STABLE_NETWORK_ID_BYTES))
		{
			return false;
		}
		if (Is_NextAuditionOperation(operation))
		{
			return reader.Read_U32(message.iPredecessorRoomAuditionEpoch) &&
				reader.Read_U32(message.iPredecessorPatternSequence) &&
				reader.Read_U32(message.iExpectedNextRequestSequence) &&
				Read_GameplayDataRevision(
					reader, message.ExpectedDefinitionRevision);
		}
		if (Is_RestartAuditionOperation(operation))
		{
			return reader.Read_U32(message.iPredecessorRoomAuditionEpoch) &&
				reader.Read_U32(message.iPredecessorPatternSequence) &&
				Read_GameplayDataRevision(
					reader, message.ExpectedDefinitionRevision) &&
				Read_GameplayDataRevision(
					reader, message.ReplacementDefinitionRevision);
		}
		if (Is_PlayPatternIdAuditionOperation(operation))
		{
			return Read_GameplayDataRevision(
				reader, message.ExpectedDefinitionRevision);
		}
		return true;
	}
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_VALTAN_AUDITION_REQUEST& message)
{
	const std::uint8_t rawOperation =
		static_cast<std::uint8_t>(message.eOperation);
	if (!Is_Valid_AuditionRequest(
		message.iRequestSequence, rawOperation, message.iTargetHealthBar,
		message.strBossPlacementId, message.strPatternId,
		message.iPredecessorRoomAuditionEpoch, message.iPredecessorPatternSequence,
		message.iExpectedNextRequestSequence,
		message.ExpectedDefinitionRevision,
		message.ReplacementDefinitionRevision))
	{
		return false;
	}
	writer.Write_U32(message.iRequestSequence);
	writer.Write_U8(rawOperation);
	writer.Write_U32(message.iTargetHealthBar);
	return Write_AuditionIdentity(writer, message, rawOperation);
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
	if (!Read_AuditionIdentity(reader, decoded, rawOperation))
	{
		return false;
	}
	if (!Is_Valid_AuditionRequest(
		decoded.iRequestSequence, rawOperation, decoded.iTargetHealthBar,
		decoded.strBossPlacementId, decoded.strPatternId,
		decoded.iPredecessorRoomAuditionEpoch, decoded.iPredecessorPatternSequence,
		decoded.iExpectedNextRequestSequence,
		decoded.ExpectedDefinitionRevision,
		decoded.ReplacementDefinitionRevision))
	{
		return false;
	}
	decoded.eOperation =
		static_cast<VALTAN_AUDITION_OPERATION>(rawOperation);
	message = std::move(decoded);
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
		message.iRequestSequence, rawOperation, message.iTargetHealthBar,
		message.strBossPlacementId, message.strPatternId,
		message.iPredecessorRoomAuditionEpoch, message.iPredecessorPatternSequence,
		message.iExpectedNextRequestSequence,
		message.ExpectedDefinitionRevision,
		message.ReplacementDefinitionRevision) ||
		!Is_Valid_AuditionResult(rawOperation, rawResult))
	{
		return false;
	}
	writer.Write_U32(message.iRequestSequence);
	writer.Write_U8(rawOperation);
	writer.Write_U32(message.iTargetHealthBar);
	writer.Write_U8(rawResult);
	writer.Write_U32(message.iCurrentHealthBar);
	return Write_AuditionIdentity(writer, message, rawOperation);
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
	if (!Read_AuditionIdentity(reader, decoded, rawOperation))
	{
		return false;
	}
	if (!Is_Valid_AuditionRequest(
		decoded.iRequestSequence, rawOperation, decoded.iTargetHealthBar,
		decoded.strBossPlacementId, decoded.strPatternId,
		decoded.iPredecessorRoomAuditionEpoch, decoded.iPredecessorPatternSequence,
		decoded.iExpectedNextRequestSequence,
		decoded.ExpectedDefinitionRevision,
		decoded.ReplacementDefinitionRevision) ||
		!Is_Valid_AuditionResult(rawOperation, rawResult))
	{
		return false;
	}
	decoded.eOperation =
		static_cast<VALTAN_AUDITION_OPERATION>(rawOperation);
	decoded.eResult = static_cast<VALTAN_AUDITION_RESULT>(rawResult);
	message = std::move(decoded);
	return true;
}

namespace
{
	using namespace LostArk::Shared;

	bool Is_Valid_AuditionLifecycle(
		const S2C_VALTAN_AUDITION_LIFECYCLE& message)
	{
		const std::uint8_t rawState =
			static_cast<std::uint8_t>(message.eState);
		if (0u == message.iRequestSequence ||
			0u == message.iRoomAuditionEpoch ||
			!Is_Valid_StableId(message.strPatternId, false) ||
			rawState >= static_cast<std::uint8_t>(
				VALTAN_AUDITION_LIFECYCLE_STATE::END) ||
			!message.PinnedDefinitionRevision.Is_Valid())
		{
			return false;
		}
		if (0u == message.iPatternSequence &&
			(VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE == message.eState ||
			 VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED == message.eState ||
			 VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED == message.eState ||
			 VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER == message.eState))
		{
			return false;
		}

		const bool aborted =
			VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED == message.eState;
		return Is_Valid_BoundedReason(
			message.strReason,
			MAX_VALTAN_AUDITION_LIFECYCLE_REASON_BYTES,
			!aborted) &&
			(aborted || message.strReason.empty());
	}

	bool Is_Valid_ValtanPatternFlowRevision(
		const std::string& revision,
		const bool allowEmpty)
	{
		if (revision.empty())
			return allowEmpty;
		return VALTAN_PATTERN_FLOW_REVISION_HEX_BYTES == revision.size() &&
			std::all_of(
				revision.begin(), revision.end(), [](const unsigned char value)
				{
					return ('0' <= value && value <= '9') ||
						('a' <= value && value <= 'f');
				});
	}

	bool Is_Valid_ValtanPatternFlowSlot(
		const VALTAN_PATTERN_FLOW_SLOT_WIRE& slot)
	{
		return Is_Valid_StableId(slot.strSlotId, false) &&
			Is_Valid_StableId(slot.strPatternId, false);
	}

	bool Is_Valid_ValtanPatternFlowStart(
		const C2S_DEBUG_VALTAN_PATTERN_FLOW_START& message)
	{
		if (0u == message.iRequestSequence ||
			!message.ExpectedDefinitionRevision.Is_Valid() ||
			!Is_Valid_StableId(message.strBossPlacementId, false) ||
			!Is_Valid_StableId(message.strFlowId, false) ||
			!Is_Valid_ValtanPatternFlowRevision(
				message.strFlowRevision, false) ||
			!Is_Valid_StableId(message.strStartSlotId, false) ||
			message.iInterStepPursuitMs <
				MIN_VALTAN_PATTERN_FLOW_INTER_STEP_PURSUIT_MS ||
			message.iInterStepPursuitMs >
				MAX_VALTAN_PATTERN_FLOW_INTER_STEP_PURSUIT_MS ||
			message.Slots.empty() ||
			message.Slots.size() > MAX_VALTAN_PATTERN_FLOW_SLOTS)
		{
			return false;
		}

		/* Slot uniqueness, start membership, and definition resolution are
		   Server semantics so every well-shaped invalid request receives a typed
		   result instead of disappearing at the frame decoder. */
		if (!std::all_of(
			message.Slots.begin(), message.Slots.end(),
			Is_Valid_ValtanPatternFlowSlot))
		{
			return false;
		}

		/* Keep typed encoding inside the shared 64 KiB frame contract even for
		   callers that construct wire rows without the Boss Tool document's
		   shorter canonical slot IDs. */
		std::size_t encodedBytes = sizeof(std::uint32_t) * 2u +
			GAMEPLAY_DATA_REVISION_BYTES +
			sizeof(std::uint8_t);
		for (const std::string_view value : {
			std::string_view(message.strBossPlacementId),
			std::string_view(message.strFlowId),
			std::string_view(message.strFlowRevision),
			std::string_view(message.strStartSlotId) })
		{
			encodedBytes += sizeof(std::uint16_t) + value.size();
		}
		for (const VALTAN_PATTERN_FLOW_SLOT_WIRE& slot : message.Slots)
		{
			encodedBytes += sizeof(std::uint16_t) + slot.strSlotId.size();
			encodedBytes += sizeof(std::uint16_t) + slot.strPatternId.size();
		}
		return encodedBytes <= MAX_PACKET_BYTES - PACKET_HEADER_BYTES;
	}

	bool Is_Valid_ValtanPatternFlowStop(
		const C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& message)
	{
		return 0u != message.iControlSequence &&
			Is_Valid_StableId(message.strFlowId, false) &&
			0u != message.iRoomFlowEpoch;
	}

	bool Is_Accepted_ValtanPatternFlowResult(
		const VALTAN_PATTERN_FLOW_RESULT result)
	{
		return VALTAN_PATTERN_FLOW_RESULT::QUEUED == result ||
			VALTAN_PATTERN_FLOW_RESULT::DUPLICATE_IGNORED == result;
	}

	bool Is_Valid_ValtanPatternFlowResult(
		const S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT& message)
	{
		const std::uint8_t rawCommand =
			static_cast<std::uint8_t>(message.eCommand);
		const std::uint8_t rawResult =
			static_cast<std::uint8_t>(message.eResult);
		if (0u == message.iCommandSequence ||
			rawCommand >= static_cast<std::uint8_t>(
				VALTAN_PATTERN_FLOW_COMMAND::END) ||
			rawResult >= static_cast<std::uint8_t>(
				VALTAN_PATTERN_FLOW_RESULT::END) ||
			!Is_Valid_StableId(message.strFlowId, false))
		{
			return false;
		}

		const bool accepted =
			Is_Accepted_ValtanPatternFlowResult(message.eResult);
		const bool stopRejection =
			VALTAN_PATTERN_FLOW_COMMAND::STOP_AFTER_CURRENT ==
				message.eCommand && !accepted;
		if (!Is_Valid_ValtanPatternFlowRevision(
			message.strFlowRevision, stopRejection))
		{
			return false;
		}

		if (accepted)
		{
			return 0u != message.iRoomFlowEpoch &&
				message.PinnedDefinitionRevision.Is_Valid() &&
				message.strReason.empty();
		}
		return !message.PinnedDefinitionRevision.Is_Valid() &&
			Is_Valid_BoundedReason(
				message.strReason,
				MAX_VALTAN_PATTERN_FLOW_REASON_BYTES,
				false);
	}

	bool Is_Valid_ValtanPatternFlowLifecycle(
		const S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE& message)
	{
		const std::uint8_t rawState =
			static_cast<std::uint8_t>(message.eState);
		if (0u == message.iRequestSequence ||
			!Is_Valid_StableId(message.strBossPlacementId, false) ||
			!Is_Valid_StableId(message.strFlowId, false) ||
			!Is_Valid_ValtanPatternFlowRevision(
				message.strFlowRevision, false) ||
			!Is_Valid_StableId(message.strStartSlotId, false) ||
			rawState >= static_cast<std::uint8_t>(
				VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::END) ||
			0u == message.iSlotCount ||
			message.iSlotCount > MAX_VALTAN_PATTERN_FLOW_SLOTS)
		{
			return false;
		}

		const bool rejected =
			VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::REJECTED == message.eState;
		const bool aborted =
			VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ABORTED == message.eState;
		if (rejected)
		{
			return 0u == message.iRoomFlowEpoch &&
				0u == message.iPatternSequence &&
				message.strCurrentSlotId.empty() &&
				message.strCurrentPatternId.empty() &&
				0u == message.iCurrentSlotOrdinal &&
				!message.PinnedDefinitionRevision.Is_Valid() &&
				Is_Valid_BoundedReason(
					message.strReason,
					MAX_VALTAN_PATTERN_FLOW_REASON_BYTES,
					false);
		}

		if (0u == message.iRoomFlowEpoch ||
			!message.PinnedDefinitionRevision.Is_Valid() ||
			!Is_Valid_StableId(message.strCurrentSlotId, false) ||
			!Is_Valid_StableId(message.strCurrentPatternId, false) ||
			0u == message.iCurrentSlotOrdinal ||
			message.iCurrentSlotOrdinal > message.iSlotCount ||
			(VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PENDING != message.eState &&
				0u == message.iPatternSequence))
		{
			return false;
		}

		return Is_Valid_BoundedReason(
			message.strReason,
			MAX_VALTAN_PATTERN_FLOW_REASON_BYTES,
			!aborted) &&
			(aborted || message.strReason.empty());
	}

	bool Is_Valid_ValtanDecisionTraceCandidate(
		const VALTAN_DECISION_TRACE_CANDIDATE_WIRE& candidate)
	{
		if (!Is_Valid_StableId(candidate.strPatternId, false) ||
			0u != (candidate.iExclusionMask &
				~VALTAN_DECISION_TRACE_KNOWN_EXCLUSION_MASK) ||
			candidate.iWeightEndExclusive <
				candidate.iWeightBeginInclusive)
		{
			return false;
		}

		const std::uint64_t interval = candidate.iWeightEndExclusive -
			candidate.iWeightBeginInclusive;
		if (0u == candidate.iEffectiveWeight)
		{
			return 0u == candidate.iWeightBeginInclusive &&
				0u == candidate.iWeightEndExclusive;
		}
		return interval == candidate.iEffectiveWeight;
	}

	bool Is_Valid_ValtanDecisionTrace(
		const VALTAN_DECISION_TRACE_WIRE& trace)
	{
		const std::uint8_t rawSource =
			static_cast<std::uint8_t>(trace.eSource);
		const std::uint8_t rawPendingSource =
			static_cast<std::uint8_t>(trace.ePendingSource);
		const std::uint8_t rawResult =
			static_cast<std::uint8_t>(trace.eResult);
		if (0u == trace.iTraceSequence ||
			0u == trace.iExpectedPatternSequence ||
			0u == trace.iMaximumHp ||
			trace.iCurrentHp > trace.iMaximumHp ||
			0u == trace.iGameplayPhase ||
			!std::isfinite(trace.fTargetDistance) ||
			trace.fTargetDistance < 0.f ||
			rawSource >= static_cast<std::uint8_t>(
				VALTAN_DECISION_TRACE_SOURCE::END) ||
			rawPendingSource >= static_cast<std::uint8_t>(
				VALTAN_DECISION_TRACE_SOURCE::END) ||
			rawResult >= static_cast<std::uint8_t>(
				VALTAN_DECISION_TRACE_RESULT::END) ||
			!Is_Valid_StableId(trace.strRotationId, true) ||
			!Is_Valid_StableId(trace.strPendingPatternId, true) ||
			!Is_Valid_StableId(trace.strSelectedPatternId, true) ||
			trace.Candidates.size() >
				MAX_VALTAN_DECISION_TRACE_CANDIDATES ||
			(0u != trace.iTotalWeight &&
				trace.iRandomTicket >= trace.iTotalWeight))
		{
			return false;
		}

		std::size_t selectedCount = 0u;
		for (std::size_t index = 0u; index < trace.Candidates.size(); ++index)
		{
			const VALTAN_DECISION_TRACE_CANDIDATE_WIRE& candidate =
				trace.Candidates[index];
			if (!Is_Valid_ValtanDecisionTraceCandidate(candidate))
				return false;
			for (std::size_t other = index + 1u;
				other < trace.Candidates.size(); ++other)
			{
				if (candidate.strPatternId ==
					trace.Candidates[other].strPatternId)
				{
					return false;
				}
			}
			if (candidate.isSelected)
			{
				++selectedCount;
				if (candidate.strPatternId != trace.strSelectedPatternId)
					return false;
			}
		}

		if (VALTAN_DECISION_TRACE_RESULT::SELECTED == trace.eResult)
		{
			return !trace.strSelectedPatternId.empty() && 1u == selectedCount;
		}
		return trace.strSelectedPatternId.empty() && 0u == selectedCount;
	}

	bool Is_Valid_ValtanDecisionTraceQuery(
		const C2S_VALTAN_DECISION_TRACE_QUERY& message)
	{
		return 0u != message.iRequestSequence &&
			Is_Valid_StableId(message.strBossPlacementId, false);
	}

	bool Has_Empty_ValtanDecisionTracePayload(
		const S2C_VALTAN_DECISION_TRACE_RESPONSE& message)
	{
		return !message.DefinitionRevision.Is_Valid() &&
			0u == message.Trace.iTraceSequence &&
			message.Trace.strRotationId.empty() &&
			message.Trace.strPendingPatternId.empty() &&
			message.Trace.strSelectedPatternId.empty() &&
			message.Trace.Candidates.empty();
	}

	bool Is_Valid_ValtanDecisionTraceResponse(
		const S2C_VALTAN_DECISION_TRACE_RESPONSE& message)
	{
		const std::uint8_t rawResult =
			static_cast<std::uint8_t>(message.eResult);
		if (0u == message.iRequestSequence ||
			!Is_Valid_StableId(message.strBossPlacementId, false) ||
			rawResult >= static_cast<std::uint8_t>(
				VALTAN_DECISION_TRACE_QUERY_RESULT::END))
		{
			return false;
		}
		if (VALTAN_DECISION_TRACE_QUERY_RESULT::TRACE == message.eResult)
		{
			return message.DefinitionRevision.Is_Valid() &&
				Is_Valid_ValtanDecisionTrace(message.Trace);
		}
		return Has_Empty_ValtanDecisionTracePayload(message);
	}

	bool Is_Valid_DataRevisionPrepareIdentity(
		const std::uint32_t sequence,
		const GameplayDataRevision& baseRevision,
		const GameplayDataRevision& candidateRevision,
		const std::uint32_t requiredLaneMask)
	{
		return 0u != sequence &&
			baseRevision.Is_Valid() &&
			candidateRevision.Is_Valid() &&
			baseRevision != candidateRevision &&
			Is_Valid_PresentationLaneMask(requiredLaneMask);
	}

	bool Is_Valid_DataRevisionPrepareResponse(
		const C2S_DATA_REVISION_PREPARE_RESPONSE& message)
	{
		const std::uint8_t rawStatus =
			static_cast<std::uint8_t>(message.eStatus);
		if (0u == message.iTransactionSequence ||
			!message.CandidateRevision.Is_Valid() ||
			rawStatus >= static_cast<std::uint8_t>(
				DATA_REVISION_PREPARE_STATUS::END) ||
			!Is_Valid_PresentationLaneMask(
				message.iRequiredPresentationLaneMask) ||
			!Is_Valid_PresentationLaneMask(
				message.iPreparedPresentationLaneMask) ||
			!Is_Valid_PresentationLaneMask(
				message.iFailedPresentationLaneMask) ||
			0u != (message.iPreparedPresentationLaneMask &
				message.iFailedPresentationLaneMask))
		{
			return false;
		}

		switch (message.eStatus)
		{
		case DATA_REVISION_PREPARE_STATUS::READY:
			return 0u == message.iFailedPresentationLaneMask &&
				(message.iPreparedPresentationLaneMask &
					message.iRequiredPresentationLaneMask) ==
					message.iRequiredPresentationLaneMask &&
				message.strReason.empty();
		case DATA_REVISION_PREPARE_STATUS::READY_DEGRADED:
			return 0u != message.iFailedPresentationLaneMask &&
				0u == (message.iFailedPresentationLaneMask &
					message.iRequiredPresentationLaneMask) &&
				(message.iPreparedPresentationLaneMask &
					message.iRequiredPresentationLaneMask) ==
					message.iRequiredPresentationLaneMask &&
				Is_Valid_BoundedReason(
					message.strReason,
					MAX_DATA_REVISION_REASON_BYTES,
					false);
		case DATA_REVISION_PREPARE_STATUS::NACK:
			return Is_Valid_BoundedReason(
				message.strReason,
				MAX_DATA_REVISION_REASON_BYTES,
				false);
		default:
			return false;
		}
	}

	bool Is_Valid_DataRevisionResult(
		const S2C_DATA_REVISION_RESULT& message)
	{
		const std::uint8_t rawResult =
			static_cast<std::uint8_t>(message.eResult);
		if (0u == message.iTransactionSequence ||
			!message.CandidateRevision.Is_Valid() ||
			!message.ActiveRevision.Is_Valid() ||
			rawResult >= static_cast<std::uint8_t>(
				DATA_REVISION_RESULT::END))
		{
			return false;
		}

		if (DATA_REVISION_RESULT::COMMITTED == message.eResult)
		{
			return message.ActiveRevision == message.CandidateRevision &&
				message.strReason.empty();
		}
		return message.ActiveRevision != message.CandidateRevision &&
			Is_Valid_BoundedReason(
				message.strReason,
				MAX_DATA_REVISION_REASON_BYTES,
				false);
	}

	bool Write_ValtanDecisionTrace(
		CPacketWriter& writer,
		const VALTAN_DECISION_TRACE_WIRE& trace)
	{
		if (!Is_Valid_ValtanDecisionTrace(trace))
			return false;
		Write_U64(writer, trace.iTraceSequence);
		writer.Write_U32(trace.iServerTick);
		writer.Write_U32(trace.iPatternSequenceBeforeDecision);
		writer.Write_U32(trace.iExpectedPatternSequence);
		writer.Write_U32(trace.iCurrentHp);
		writer.Write_U32(trace.iMaximumHp);
		writer.Write_U32(trace.iHealthBar);
		writer.Write_U8(trace.iGameplayPhase);
		writer.Write_U32(trace.iTargetNetEntityId);
		writer.Write_F32(trace.fTargetDistance);
		writer.Write_U8(trace.isIntroPatternConsumed ? 1u : 0u);
		writer.Write_U32(trace.iRotationStepIndex);
		writer.Write_U8(static_cast<std::uint8_t>(trace.eSource));
		writer.Write_U8(static_cast<std::uint8_t>(trace.eResult));
		if (!writer.Write_String(
				trace.strRotationId, MAX_STABLE_NETWORK_ID_BYTES) ||
			!writer.Write_String(
				trace.strPendingPatternId, MAX_STABLE_NETWORK_ID_BYTES))
		{
			return false;
		}
		writer.Write_U8(static_cast<std::uint8_t>(trace.ePendingSource));
		if (!writer.Write_String(
				trace.strSelectedPatternId, MAX_STABLE_NETWORK_ID_BYTES))
		{
			return false;
		}
		Write_U64(writer, trace.iRawRandomInput);
		Write_U64(writer, trace.iMixedRandomValue);
		Write_U64(writer, trace.iTotalWeight);
		Write_U64(writer, trace.iRandomTicket);
		writer.Write_U8(trace.isMaximumConsecutiveRelaxed ? 1u : 0u);
		writer.Write_U8(trace.areCandidatesTruncated ? 1u : 0u);
		writer.Write_U16(static_cast<std::uint16_t>(trace.Candidates.size()));
		for (const VALTAN_DECISION_TRACE_CANDIDATE_WIRE& candidate :
			trace.Candidates)
		{
			if (!writer.Write_String(
					candidate.strPatternId, MAX_STABLE_NETWORK_ID_BYTES))
			{
				return false;
			}
			writer.Write_U32(candidate.iExclusionMask);
			writer.Write_U32(candidate.iAuthoredWeight);
			writer.Write_U32(candidate.iEffectiveWeight);
			writer.Write_U32(candidate.iCooldownRemainingTicks);
			writer.Write_U32(candidate.iConsecutiveUses);
			writer.Write_U32(candidate.iMaximumConsecutiveUses);
			Write_U64(writer, candidate.iWeightBeginInclusive);
			Write_U64(writer, candidate.iWeightEndExclusive);
			writer.Write_U8(candidate.isSelected ? 1u : 0u);
		}
		return true;
	}

	bool Read_ValtanDecisionTrace(
		CPacketReader& reader,
		VALTAN_DECISION_TRACE_WIRE& trace)
	{
		VALTAN_DECISION_TRACE_WIRE decoded{};
		std::uint8_t rawIntroConsumed = 0u;
		std::uint8_t rawSource = 0u;
		std::uint8_t rawResult = 0u;
		std::uint8_t rawPendingSource = 0u;
		std::uint8_t rawRelaxed = 0u;
		std::uint8_t rawTruncated = 0u;
		std::uint16_t candidateCount = 0u;
		if (!Read_U64(reader, decoded.iTraceSequence) ||
			!reader.Read_U32(decoded.iServerTick) ||
			!reader.Read_U32(decoded.iPatternSequenceBeforeDecision) ||
			!reader.Read_U32(decoded.iExpectedPatternSequence) ||
			!reader.Read_U32(decoded.iCurrentHp) ||
			!reader.Read_U32(decoded.iMaximumHp) ||
			!reader.Read_U32(decoded.iHealthBar) ||
			!reader.Read_U8(decoded.iGameplayPhase) ||
			!reader.Read_U32(decoded.iTargetNetEntityId) ||
			!reader.Read_F32(decoded.fTargetDistance) ||
			!reader.Read_U8(rawIntroConsumed) || rawIntroConsumed > 1u ||
			!reader.Read_U32(decoded.iRotationStepIndex) ||
			!reader.Read_U8(rawSource) ||
			rawSource >= static_cast<std::uint8_t>(
				VALTAN_DECISION_TRACE_SOURCE::END) ||
			!reader.Read_U8(rawResult) ||
			rawResult >= static_cast<std::uint8_t>(
				VALTAN_DECISION_TRACE_RESULT::END) ||
			!reader.Read_String(
				decoded.strRotationId, MAX_STABLE_NETWORK_ID_BYTES) ||
			!reader.Read_String(
				decoded.strPendingPatternId, MAX_STABLE_NETWORK_ID_BYTES) ||
			!reader.Read_U8(rawPendingSource) ||
			rawPendingSource >= static_cast<std::uint8_t>(
				VALTAN_DECISION_TRACE_SOURCE::END) ||
			!reader.Read_String(
				decoded.strSelectedPatternId, MAX_STABLE_NETWORK_ID_BYTES) ||
			!Read_U64(reader, decoded.iRawRandomInput) ||
			!Read_U64(reader, decoded.iMixedRandomValue) ||
			!Read_U64(reader, decoded.iTotalWeight) ||
			!Read_U64(reader, decoded.iRandomTicket) ||
			!reader.Read_U8(rawRelaxed) || rawRelaxed > 1u ||
			!reader.Read_U8(rawTruncated) || rawTruncated > 1u ||
			!reader.Read_U16(candidateCount) ||
			candidateCount > MAX_VALTAN_DECISION_TRACE_CANDIDATES)
		{
			return false;
		}
		decoded.isIntroPatternConsumed = 0u != rawIntroConsumed;
		decoded.eSource = static_cast<VALTAN_DECISION_TRACE_SOURCE>(rawSource);
		decoded.eResult = static_cast<VALTAN_DECISION_TRACE_RESULT>(rawResult);
		decoded.ePendingSource =
			static_cast<VALTAN_DECISION_TRACE_SOURCE>(rawPendingSource);
		decoded.isMaximumConsecutiveRelaxed = 0u != rawRelaxed;
		decoded.areCandidatesTruncated = 0u != rawTruncated;
		decoded.Candidates.reserve(candidateCount);
		for (std::uint16_t index = 0u; index < candidateCount; ++index)
		{
			VALTAN_DECISION_TRACE_CANDIDATE_WIRE candidate{};
			std::uint8_t rawSelected = 0u;
			if (!reader.Read_String(
					candidate.strPatternId, MAX_STABLE_NETWORK_ID_BYTES) ||
				!reader.Read_U32(candidate.iExclusionMask) ||
				!reader.Read_U32(candidate.iAuthoredWeight) ||
				!reader.Read_U32(candidate.iEffectiveWeight) ||
				!reader.Read_U32(candidate.iCooldownRemainingTicks) ||
				!reader.Read_U32(candidate.iConsecutiveUses) ||
				!reader.Read_U32(candidate.iMaximumConsecutiveUses) ||
				!Read_U64(reader, candidate.iWeightBeginInclusive) ||
				!Read_U64(reader, candidate.iWeightEndExclusive) ||
				!reader.Read_U8(rawSelected) || rawSelected > 1u)
			{
				return false;
			}
			candidate.isSelected = 0u != rawSelected;
			decoded.Candidates.push_back(std::move(candidate));
		}
		if (!Is_Valid_ValtanDecisionTrace(decoded))
			return false;
		trace = std::move(decoded);
		return true;
	}
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_VALTAN_AUDITION_LIFECYCLE& message)
{
	if (!Is_Valid_AuditionLifecycle(message))
		return false;

	writer.Write_U32(message.iRequestSequence);
	writer.Write_U32(message.iRoomAuditionEpoch);
	writer.Write_U32(message.iPatternSequence);
	if (!writer.Write_String(message.strPatternId, MAX_STABLE_NETWORK_ID_BYTES))
		return false;
	writer.Write_U8(static_cast<std::uint8_t>(message.eState));
	if (!Write_GameplayDataRevision(
		writer, message.PinnedDefinitionRevision))
	{
		return false;
	}
	return writer.Write_String(
		message.strReason, MAX_VALTAN_AUDITION_LIFECYCLE_REASON_BYTES);
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_VALTAN_AUDITION_LIFECYCLE& message)
{
	S2C_VALTAN_AUDITION_LIFECYCLE decoded{};
	std::uint8_t rawState = 0;
	if (!reader.Read_U32(decoded.iRequestSequence) ||
		!reader.Read_U32(decoded.iRoomAuditionEpoch) ||
		!reader.Read_U32(decoded.iPatternSequence) ||
		!reader.Read_String(
			decoded.strPatternId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_U8(rawState) ||
		rawState >= static_cast<std::uint8_t>(
			VALTAN_AUDITION_LIFECYCLE_STATE::END) ||
		!Read_GameplayDataRevision(
			reader, decoded.PinnedDefinitionRevision) ||
		!reader.Read_String(
			decoded.strReason,
			MAX_VALTAN_AUDITION_LIFECYCLE_REASON_BYTES))
	{
		return false;
	}
	decoded.eState = static_cast<VALTAN_AUDITION_LIFECYCLE_STATE>(rawState);
	if (!Is_Valid_AuditionLifecycle(decoded))
		return false;
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_DEBUG_VALTAN_PATTERN_FLOW_START& message)
{
	if (!Is_Valid_ValtanPatternFlowStart(message))
		return false;

	writer.Write_U32(message.iRequestSequence);
	if (!Write_GameplayDataRevision(
			writer, message.ExpectedDefinitionRevision) ||
		!writer.Write_String(
			message.strBossPlacementId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!writer.Write_String(message.strFlowId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!writer.Write_String(
			message.strFlowRevision, VALTAN_PATTERN_FLOW_REVISION_HEX_BYTES) ||
		!writer.Write_String(
			message.strStartSlotId, MAX_STABLE_NETWORK_ID_BYTES))
	{
		return false;
	}
	writer.Write_U32(message.iInterStepPursuitMs);
	writer.Write_U8(static_cast<std::uint8_t>(message.Slots.size()));
	for (const VALTAN_PATTERN_FLOW_SLOT_WIRE& slot : message.Slots)
	{
		if (!writer.Write_String(slot.strSlotId, MAX_STABLE_NETWORK_ID_BYTES) ||
			!writer.Write_String(
				slot.strPatternId, MAX_STABLE_NETWORK_ID_BYTES))
		{
			return false;
		}
	}
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_DEBUG_VALTAN_PATTERN_FLOW_START& message)
{
	C2S_DEBUG_VALTAN_PATTERN_FLOW_START decoded{};
	std::uint8_t slotCount = 0u;
	if (!reader.Read_U32(decoded.iRequestSequence) ||
		!Read_GameplayDataRevision(
			reader, decoded.ExpectedDefinitionRevision) ||
		!reader.Read_String(
			decoded.strBossPlacementId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_String(decoded.strFlowId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_String(
			decoded.strFlowRevision, VALTAN_PATTERN_FLOW_REVISION_HEX_BYTES) ||
		!reader.Read_String(
			decoded.strStartSlotId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_U32(decoded.iInterStepPursuitMs) ||
		!reader.Read_U8(slotCount) ||
		0u == slotCount)
	{
		return false;
	}

	decoded.Slots.reserve(slotCount);
	for (std::size_t index = 0u; index < slotCount; ++index)
	{
		VALTAN_PATTERN_FLOW_SLOT_WIRE slot{};
		if (!reader.Read_String(
				slot.strSlotId, MAX_STABLE_NETWORK_ID_BYTES) ||
			!reader.Read_String(
				slot.strPatternId, MAX_STABLE_NETWORK_ID_BYTES))
		{
			return false;
		}
		decoded.Slots.push_back(std::move(slot));
	}
	if (!Is_Valid_ValtanPatternFlowStart(decoded))
		return false;
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& message)
{
	if (!Is_Valid_ValtanPatternFlowStop(message))
		return false;
	writer.Write_U32(message.iControlSequence);
	if (!writer.Write_String(message.strFlowId, MAX_STABLE_NETWORK_ID_BYTES))
		return false;
	writer.Write_U32(message.iRoomFlowEpoch);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& message)
{
	C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT decoded{};
	if (!reader.Read_U32(decoded.iControlSequence) ||
		!reader.Read_String(decoded.strFlowId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_U32(decoded.iRoomFlowEpoch) ||
		!Is_Valid_ValtanPatternFlowStop(decoded))
	{
		return false;
	}
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT& message)
{
	if (!Is_Valid_ValtanPatternFlowResult(message))
		return false;
	writer.Write_U32(message.iCommandSequence);
	writer.Write_U8(static_cast<std::uint8_t>(message.eCommand));
	writer.Write_U8(static_cast<std::uint8_t>(message.eResult));
	if (!writer.Write_String(message.strFlowId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!writer.Write_String(
			message.strFlowRevision, VALTAN_PATTERN_FLOW_REVISION_HEX_BYTES))
	{
		return false;
	}
	writer.Write_U32(message.iRoomFlowEpoch);
	if (Is_Accepted_ValtanPatternFlowResult(message.eResult) &&
		!Write_GameplayDataRevision(
			writer, message.PinnedDefinitionRevision))
	{
		return false;
	}
	return writer.Write_String(
		message.strReason, MAX_VALTAN_PATTERN_FLOW_REASON_BYTES);
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT& message)
{
	S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT decoded{};
	std::uint8_t rawCommand = 0u;
	std::uint8_t rawResult = 0u;
	if (!reader.Read_U32(decoded.iCommandSequence) ||
		!reader.Read_U8(rawCommand) ||
		rawCommand >= static_cast<std::uint8_t>(
			VALTAN_PATTERN_FLOW_COMMAND::END) ||
		!reader.Read_U8(rawResult) ||
		rawResult >= static_cast<std::uint8_t>(
			VALTAN_PATTERN_FLOW_RESULT::END) ||
		!reader.Read_String(decoded.strFlowId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_String(
			decoded.strFlowRevision, VALTAN_PATTERN_FLOW_REVISION_HEX_BYTES) ||
		!reader.Read_U32(decoded.iRoomFlowEpoch))
	{
		return false;
	}
	decoded.eCommand = static_cast<VALTAN_PATTERN_FLOW_COMMAND>(rawCommand);
	decoded.eResult = static_cast<VALTAN_PATTERN_FLOW_RESULT>(rawResult);
	if (Is_Accepted_ValtanPatternFlowResult(decoded.eResult) &&
		!Read_GameplayDataRevision(
			reader, decoded.PinnedDefinitionRevision))
	{
		return false;
	}
	if (!reader.Read_String(
			decoded.strReason, MAX_VALTAN_PATTERN_FLOW_REASON_BYTES) ||
		!Is_Valid_ValtanPatternFlowResult(decoded))
	{
		return false;
	}
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE& message)
{
	if (!Is_Valid_ValtanPatternFlowLifecycle(message))
		return false;
	writer.Write_U32(message.iRequestSequence);
	writer.Write_U32(message.iRoomFlowEpoch);
	writer.Write_U32(message.iPatternSequence);
	if (!writer.Write_String(
			message.strBossPlacementId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!writer.Write_String(message.strFlowId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!writer.Write_String(
			message.strFlowRevision, VALTAN_PATTERN_FLOW_REVISION_HEX_BYTES) ||
		!writer.Write_String(
			message.strStartSlotId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!writer.Write_String(
			message.strCurrentSlotId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!writer.Write_String(
			message.strCurrentPatternId, MAX_STABLE_NETWORK_ID_BYTES))
	{
		return false;
	}
	writer.Write_U16(message.iCurrentSlotOrdinal);
	writer.Write_U16(message.iSlotCount);
	writer.Write_U8(static_cast<std::uint8_t>(message.eState));
	if (VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::REJECTED != message.eState &&
		!Write_GameplayDataRevision(
			writer, message.PinnedDefinitionRevision))
	{
		return false;
	}
	return writer.Write_String(
		message.strReason, MAX_VALTAN_PATTERN_FLOW_REASON_BYTES);
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE& message)
{
	S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE decoded{};
	std::uint8_t rawState = 0u;
	if (!reader.Read_U32(decoded.iRequestSequence) ||
		!reader.Read_U32(decoded.iRoomFlowEpoch) ||
		!reader.Read_U32(decoded.iPatternSequence) ||
		!reader.Read_String(
			decoded.strBossPlacementId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_String(decoded.strFlowId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_String(
			decoded.strFlowRevision, VALTAN_PATTERN_FLOW_REVISION_HEX_BYTES) ||
		!reader.Read_String(
			decoded.strStartSlotId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_String(
			decoded.strCurrentSlotId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_String(
			decoded.strCurrentPatternId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_U16(decoded.iCurrentSlotOrdinal) ||
		!reader.Read_U16(decoded.iSlotCount) ||
		!reader.Read_U8(rawState) ||
		rawState >= static_cast<std::uint8_t>(
			VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::END))
	{
		return false;
	}
	decoded.eState =
		static_cast<VALTAN_PATTERN_FLOW_LIFECYCLE_STATE>(rawState);
	if (VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::REJECTED != decoded.eState &&
		!Read_GameplayDataRevision(
			reader, decoded.PinnedDefinitionRevision))
	{
		return false;
	}
	if (!reader.Read_String(
			decoded.strReason, MAX_VALTAN_PATTERN_FLOW_REASON_BYTES) ||
		!Is_Valid_ValtanPatternFlowLifecycle(decoded))
	{
		return false;
	}
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_VALTAN_DECISION_TRACE_QUERY& message)
{
	if (!Is_Valid_ValtanDecisionTraceQuery(message))
		return false;
	writer.Write_U32(message.iRequestSequence);
	if (!writer.Write_String(
			message.strBossPlacementId, MAX_STABLE_NETWORK_ID_BYTES))
	{
		return false;
	}
	Write_U64(writer, message.iAfterTraceSequence);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_VALTAN_DECISION_TRACE_QUERY& message)
{
	C2S_VALTAN_DECISION_TRACE_QUERY decoded{};
	if (!reader.Read_U32(decoded.iRequestSequence) ||
		!reader.Read_String(
			decoded.strBossPlacementId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!Read_U64(reader, decoded.iAfterTraceSequence) ||
		!Is_Valid_ValtanDecisionTraceQuery(decoded))
	{
		return false;
	}
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_VALTAN_DECISION_TRACE_RESPONSE& message)
{
	if (!Is_Valid_ValtanDecisionTraceResponse(message))
		return false;
	writer.Write_U32(message.iRequestSequence);
	if (!writer.Write_String(
			message.strBossPlacementId, MAX_STABLE_NETWORK_ID_BYTES))
	{
		return false;
	}
	writer.Write_U8(static_cast<std::uint8_t>(message.eResult));
	if (VALTAN_DECISION_TRACE_QUERY_RESULT::TRACE != message.eResult)
		return true;
	return Write_GameplayDataRevision(
		writer, message.DefinitionRevision) &&
		Write_ValtanDecisionTrace(writer, message.Trace);
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_VALTAN_DECISION_TRACE_RESPONSE& message)
{
	S2C_VALTAN_DECISION_TRACE_RESPONSE decoded{};
	std::uint8_t rawResult = 0u;
	if (!reader.Read_U32(decoded.iRequestSequence) ||
		!reader.Read_String(
			decoded.strBossPlacementId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_U8(rawResult) ||
		rawResult >= static_cast<std::uint8_t>(
			VALTAN_DECISION_TRACE_QUERY_RESULT::END))
	{
		return false;
	}
	decoded.eResult =
		static_cast<VALTAN_DECISION_TRACE_QUERY_RESULT>(rawResult);
	if (VALTAN_DECISION_TRACE_QUERY_RESULT::TRACE == decoded.eResult &&
		(!Read_GameplayDataRevision(
			reader, decoded.DefinitionRevision) ||
		 !Read_ValtanDecisionTrace(reader, decoded.Trace)))
	{
		return false;
	}
	if (!Is_Valid_ValtanDecisionTraceResponse(decoded))
		return false;
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_DATA_REVISION_PREPARE_REQUEST& message)
{
	if (!Is_Valid_DataRevisionPrepareIdentity(
		message.iTransactionSequence,
		message.BaseRevision,
		message.CandidateRevision,
		message.iRequiredPresentationLaneMask))
	{
		return false;
	}
	writer.Write_U32(message.iTransactionSequence);
	if (!Write_GameplayDataRevision(writer, message.BaseRevision) ||
		!Write_GameplayDataRevision(writer, message.CandidateRevision))
	{
		return false;
	}
	writer.Write_U32(message.iRequiredPresentationLaneMask);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_DATA_REVISION_PREPARE_REQUEST& message)
{
	C2S_DATA_REVISION_PREPARE_REQUEST decoded{};
	if (!reader.Read_U32(decoded.iTransactionSequence) ||
		!Read_GameplayDataRevision(reader, decoded.BaseRevision) ||
		!Read_GameplayDataRevision(reader, decoded.CandidateRevision) ||
		!reader.Read_U32(decoded.iRequiredPresentationLaneMask) ||
		!Is_Valid_DataRevisionPrepareIdentity(
			decoded.iTransactionSequence,
			decoded.BaseRevision,
			decoded.CandidateRevision,
			decoded.iRequiredPresentationLaneMask))
	{
		return false;
	}
	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_DATA_REVISION_PREPARE& message)
{
	if (!Is_Valid_DataRevisionPrepareIdentity(
		message.iTransactionSequence,
		message.BaseRevision,
		message.CandidateRevision,
		message.iRequiredPresentationLaneMask))
	{
		return false;
	}
	writer.Write_U32(message.iTransactionSequence);
	if (!Write_GameplayDataRevision(writer, message.BaseRevision) ||
		!Write_GameplayDataRevision(writer, message.CandidateRevision))
	{
		return false;
	}
	writer.Write_U32(message.iRequiredPresentationLaneMask);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_DATA_REVISION_PREPARE& message)
{
	S2C_DATA_REVISION_PREPARE decoded{};
	if (!reader.Read_U32(decoded.iTransactionSequence) ||
		!Read_GameplayDataRevision(reader, decoded.BaseRevision) ||
		!Read_GameplayDataRevision(reader, decoded.CandidateRevision) ||
		!reader.Read_U32(decoded.iRequiredPresentationLaneMask) ||
		!Is_Valid_DataRevisionPrepareIdentity(
			decoded.iTransactionSequence,
			decoded.BaseRevision,
			decoded.CandidateRevision,
			decoded.iRequiredPresentationLaneMask))
	{
		return false;
	}
	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_DATA_REVISION_PREPARE_RESPONSE& message)
{
	if (!Is_Valid_DataRevisionPrepareResponse(message))
		return false;
	writer.Write_U32(message.iTransactionSequence);
	if (!Write_GameplayDataRevision(writer, message.CandidateRevision))
		return false;
	writer.Write_U8(static_cast<std::uint8_t>(message.eStatus));
	writer.Write_U32(message.iRequiredPresentationLaneMask);
	writer.Write_U32(message.iPreparedPresentationLaneMask);
	writer.Write_U32(message.iFailedPresentationLaneMask);
	return writer.Write_String(
		message.strReason, MAX_DATA_REVISION_REASON_BYTES);
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_DATA_REVISION_PREPARE_RESPONSE& message)
{
	C2S_DATA_REVISION_PREPARE_RESPONSE decoded{};
	std::uint8_t rawStatus = 0;
	if (!reader.Read_U32(decoded.iTransactionSequence) ||
		!Read_GameplayDataRevision(reader, decoded.CandidateRevision) ||
		!reader.Read_U8(rawStatus) ||
		rawStatus >= static_cast<std::uint8_t>(
			DATA_REVISION_PREPARE_STATUS::END) ||
		!reader.Read_U32(decoded.iRequiredPresentationLaneMask) ||
		!reader.Read_U32(decoded.iPreparedPresentationLaneMask) ||
		!reader.Read_U32(decoded.iFailedPresentationLaneMask) ||
		!reader.Read_String(
			decoded.strReason, MAX_DATA_REVISION_REASON_BYTES))
	{
		return false;
	}
	decoded.eStatus = static_cast<DATA_REVISION_PREPARE_STATUS>(rawStatus);
	if (!Is_Valid_DataRevisionPrepareResponse(decoded))
		return false;
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_DATA_REVISION_RESULT& message)
{
	if (!Is_Valid_DataRevisionResult(message))
		return false;
	writer.Write_U32(message.iTransactionSequence);
	if (!Write_GameplayDataRevision(writer, message.CandidateRevision) ||
		!Write_GameplayDataRevision(writer, message.ActiveRevision))
	{
		return false;
	}
	writer.Write_U8(static_cast<std::uint8_t>(message.eResult));
	return writer.Write_String(
		message.strReason, MAX_DATA_REVISION_REASON_BYTES);
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_DATA_REVISION_RESULT& message)
{
	S2C_DATA_REVISION_RESULT decoded{};
	std::uint8_t rawResult = 0;
	if (!reader.Read_U32(decoded.iTransactionSequence) ||
		!Read_GameplayDataRevision(reader, decoded.CandidateRevision) ||
		!Read_GameplayDataRevision(reader, decoded.ActiveRevision) ||
		!reader.Read_U8(rawResult) ||
		rawResult >= static_cast<std::uint8_t>(DATA_REVISION_RESULT::END) ||
		!reader.Read_String(
			decoded.strReason, MAX_DATA_REVISION_REASON_BYTES))
	{
		return false;
	}
	decoded.eResult = static_cast<DATA_REVISION_RESULT>(rawResult);
	if (!Is_Valid_DataRevisionResult(decoded))
		return false;
	message = std::move(decoded);
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

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_DESPAWN_ALL_WORLD_ENTITIES& message)
{
	if (0u == message.iRequestSequence)
		return false;
	writer.Write_U32(message.iRequestSequence);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_DESPAWN_ALL_WORLD_ENTITIES& message)
{
	C2S_DESPAWN_ALL_WORLD_ENTITIES decoded{};
	if (!reader.Read_U32(decoded.iRequestSequence) ||
		0u == decoded.iRequestSequence)
	{
		return false;
	}
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_CONFIRM_NPC_ENTRY& message)
{
	if (0u == message.iRequestSequence || message.strNpcPlacementId.empty() ||
		message.strNpcPlacementId.size() > MAX_NPC_PLACEMENT_ID_BYTES)
	{
		return false;
	}
	writer.Write_U32(message.iRequestSequence);
	if (!writer.Write_String(
		message.strNpcPlacementId, MAX_NPC_PLACEMENT_ID_BYTES))
	{
		return false;
	}
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_CONFIRM_NPC_ENTRY& message)
{
	C2S_CONFIRM_NPC_ENTRY decoded{};
	if (!reader.Read_U32(decoded.iRequestSequence) ||
		!reader.Read_String(
			decoded.strNpcPlacementId, MAX_NPC_PLACEMENT_ID_BYTES) ||
		0u == decoded.iRequestSequence || decoded.strNpcPlacementId.empty())
	{
		return false;
	}
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_RETURN_TO_BERN& message)
{
	if (0u == message.iRequestSequence)
		return false;
	writer.Write_U32(message.iRequestSequence);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_RETURN_TO_BERN& message)
{
	C2S_RETURN_TO_BERN decoded{};
	if (!reader.Read_U32(decoded.iRequestSequence) ||
		0u == decoded.iRequestSequence)
	{
		return false;
	}
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_PARTY_INVITE& message)
{
	if (0u == message.iRequestSequence ||
		INVALID_NET_ENTITY_ID == message.iTargetNetEntityId)
	{
		return false;
	}
	writer.Write_U32(message.iRequestSequence);
	writer.Write_U32(message.iTargetNetEntityId);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_PARTY_INVITE& message)
{
	C2S_PARTY_INVITE decoded{};
	if (!reader.Read_U32(decoded.iRequestSequence) ||
		!reader.Read_U32(decoded.iTargetNetEntityId) ||
		0u == decoded.iRequestSequence ||
		INVALID_NET_ENTITY_ID == decoded.iTargetNetEntityId)
	{
		return false;
	}
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_PARTY_INVITE_RECEIVED& message)
{
	if (INVALID_NET_ENTITY_ID == message.iFromNetEntityId ||
		!Is_Valid_PlayerNickname(message.strFromNickname))
	{
		return false;
	}
	writer.Write_U32(message.iFromNetEntityId);
	if (!writer.Write_String(message.strFromNickname, MAX_NICKNAME_BYTES))
		return false;
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_PARTY_INVITE_RECEIVED& message)
{
	S2C_PARTY_INVITE_RECEIVED decoded{};
	if (!reader.Read_U32(decoded.iFromNetEntityId) ||
		!reader.Read_String(decoded.strFromNickname, MAX_NICKNAME_BYTES) ||
		INVALID_NET_ENTITY_ID == decoded.iFromNetEntityId ||
		!Is_Valid_PlayerNickname(decoded.strFromNickname))
	{
		return false;
	}
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_PARTY_INVITE_RESPOND& message)
{
	if (0u == message.iRequestSequence ||
		INVALID_NET_ENTITY_ID == message.iFromNetEntityId)
	{
		return false;
	}
	writer.Write_U32(message.iRequestSequence);
	writer.Write_U32(message.iFromNetEntityId);
	writer.Write_U8(message.bAccepted ? 1u : 0u);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_PARTY_INVITE_RESPOND& message)
{
	C2S_PARTY_INVITE_RESPOND decoded{};
	std::uint8_t rawAccepted = 0u;
	if (!reader.Read_U32(decoded.iRequestSequence) ||
		!reader.Read_U32(decoded.iFromNetEntityId) ||
		!reader.Read_U8(rawAccepted) ||
		0u == decoded.iRequestSequence ||
		INVALID_NET_ENTITY_ID == decoded.iFromNetEntityId ||
		rawAccepted > 1u)
	{
		return false;
	}
	decoded.bAccepted = (1u == rawAccepted);
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_PARTY_ROSTER& message)
{
	if (message.Members.size() > MAX_PARTY_MEMBERS)
		return false;
	writer.Write_U8(static_cast<std::uint8_t>(message.Members.size()));
	for (const PARTY_ROSTER_MEMBER& member : message.Members)
	{
		if (INVALID_NET_ENTITY_ID == member.iNetEntityId ||
			!Is_Valid_PlayerNickname(member.strNickname) ||
			!Is_Known_Character_Class(member.eCharacterClass))
		{
			return false;
		}
		writer.Write_U32(member.iNetEntityId);
		if (!writer.Write_String(member.strNickname, MAX_NICKNAME_BYTES))
			return false;
		writer.Write_U8(static_cast<std::uint8_t>(member.eCharacterClass));
	}
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_PARTY_ROSTER& message)
{
	S2C_PARTY_ROSTER decoded{};
	std::uint8_t memberCount = 0u;
	if (!reader.Read_U8(memberCount) || memberCount > MAX_PARTY_MEMBERS)
		return false;
	decoded.Members.reserve(memberCount);
	for (std::uint8_t index = 0; index < memberCount; ++index)
	{
		PARTY_ROSTER_MEMBER member{};
		std::uint8_t rawCharacterClass = 0u;
		if (!reader.Read_U32(member.iNetEntityId) ||
			!reader.Read_String(member.strNickname, MAX_NICKNAME_BYTES) ||
			!reader.Read_U8(rawCharacterClass) ||
			INVALID_NET_ENTITY_ID == member.iNetEntityId ||
			!Is_Valid_PlayerNickname(member.strNickname))
		{
			return false;
		}
		member.eCharacterClass =
			static_cast<CHARACTER_CLASS_ID>(rawCharacterClass);
		if (!Is_Known_Character_Class(member.eCharacterClass))
			return false;
		decoded.Members.push_back(std::move(member));
	}
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_CHAT& message)
{
	if (message.strText.empty() ||
		message.strText.size() > MAX_CHAT_TEXT_BYTES)
	{
		return false;
	}
	return writer.Write_String(message.strText, MAX_CHAT_TEXT_BYTES);
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_CHAT& message)
{
	C2S_CHAT decoded{};
	if (!reader.Read_String(decoded.strText, MAX_CHAT_TEXT_BYTES) ||
		decoded.strText.empty())
	{
		return false;
	}
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_CHAT& message)
{
	if (INVALID_NET_ENTITY_ID == message.iFromNetEntityId ||
		!Is_Valid_PlayerNickname(message.strFromNickname) ||
		message.strText.empty() ||
		message.strText.size() > MAX_CHAT_TEXT_BYTES)
	{
		return false;
	}
	writer.Write_U32(message.iFromNetEntityId);
	if (!writer.Write_String(message.strFromNickname, MAX_NICKNAME_BYTES))
		return false;
	return writer.Write_String(message.strText, MAX_CHAT_TEXT_BYTES);
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_CHAT& message)
{
	S2C_CHAT decoded{};
	if (!reader.Read_U32(decoded.iFromNetEntityId) ||
		!reader.Read_String(decoded.strFromNickname, MAX_NICKNAME_BYTES) ||
		!reader.Read_String(decoded.strText, MAX_CHAT_TEXT_BYTES) ||
		INVALID_NET_ENTITY_ID == decoded.iFromNetEntityId ||
		!Is_Valid_PlayerNickname(decoded.strFromNickname) ||
		decoded.strText.empty())
	{
		return false;
	}
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer, const S2C_PARTY_TRANSFER_RESULT& message)
{
	const auto result = static_cast<std::uint8_t>(message.eResult);
	if (0u == message.iRequestSequence ||
		WORLD_ID::VALTAN_ARENA != message.eTargetWorldId || result < 1u || result > 5u)
		return false;
	writer.Write_U32(message.iRequestSequence);
	writer.Write_U16(static_cast<std::uint16_t>(message.eTargetWorldId));
	writer.Write_U8(result);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader, S2C_PARTY_TRANSFER_RESULT& message)
{
	S2C_PARTY_TRANSFER_RESULT decoded{};
	std::uint16_t world = 0u;
	std::uint8_t result = 0u;
	if (!reader.Read_U32(decoded.iRequestSequence) || !reader.Read_U16(world) ||
		!reader.Read_U8(result) || 0u == decoded.iRequestSequence ||
		static_cast<std::uint16_t>(WORLD_ID::VALTAN_ARENA) != world ||
		result < 1u || result > 5u)
		return false;
	decoded.eTargetWorldId = static_cast<WORLD_ID>(world);
	decoded.eResult = static_cast<PARTY_TRANSFER_RESULT>(result);
	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer, const S2C_WORLD_SEQUENCE_PLAY& message)
{
	if (!Is_Valid_SequenceInstanceId(message.strSequenceInstanceId))
		return false;
	return writer.Write_String(
		message.strSequenceInstanceId, MAX_SEQUENCE_INSTANCE_ID_BYTES);
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader, S2C_WORLD_SEQUENCE_PLAY& message)
{
	S2C_WORLD_SEQUENCE_PLAY decoded{};
	if (!reader.Read_String(
			decoded.strSequenceInstanceId, MAX_SEQUENCE_INSTANCE_ID_BYTES) ||
		!Is_Valid_SequenceInstanceId(decoded.strSequenceInstanceId))
	{
		return false;
	}
	message = std::move(decoded);
	return true;
}

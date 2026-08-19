#pragma once

#include "ServerIds.h"

#include "Network/NetworkIds.h"
#include "Network/PacketType.h"
#include "Network/PacketMessages.h"
#include "ServerNavigation.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace LostArk::Server
{
	struct SERVER_TRIGGER_MOVE
	{
		float fStartX = 0.f;
		float fStartY = 0.f;
		float fStartZ = 0.f;
		float fTargetX = 0.f;
		float fTargetY = 0.f;
		float fTargetZ = 0.f;
		float fDurationSeconds = 0.f;
		float fElapsedSeconds = 0.f;
		float fArcHeight = 0.f;
		bool isActive = false;
	};

	/* One projectile-hit target the object has already touched: a contact hit
	fires iRepeatCount times per target, iRepeatMs apart, and never again. */
	struct SERVER_PROJECTILE_CONTACT_MARK
	{
		LostArk::Shared::NET_ENTITY_ID iNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint8_t iHitIndex = 0;
		std::uint8_t iAppliedCount = 0;
		float fNextSeconds = 0.f;
	};

	/* A live object a skill spawned (missile, fixed area...). It outlives the
	action that spawned it and is advanced by CPlayerSkillSystem::Update every
	tick; the definition is looked up by skill/stage/index in the catalog so
	nothing here points into it. Damage is a share of the skill's rate: the
	object's hits continue the caster's sub-hit numbering. */
	struct SERVER_SKILL_PROJECTILE
	{
		LostArk::Shared::SKILL_ID iSkillId = LostArk::Shared::INVALID_SKILL_ID;
		std::uint8_t iStageIndex = 0;
		std::uint8_t iProjectileIndex = 0;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fDirectionX = 0.f;
		float fDirectionZ = 1.f;
		float fSpeed = 0.f;
		// Metres still to travel; negative means unlimited (life-bound only).
		float fRemainingDistance = -1.f;
		float fRemainingSeconds = 0.f;
		float fElapsedSeconds = 0.f;
		std::uint64_t iTotalDamage = 0;
		std::uint32_t iSubHitTotal = 1;
		std::uint32_t iSubHitBase = 0;
		std::uint64_t iAppliedTimedMask = 0;
		std::vector<SERVER_PROJECTILE_CONTACT_MARK> ContactMarks;
	};

	/* Get-up grace window after a knockdown ends: 2000 ms at the fixed 30 Hz
	tick. Long enough to cover the stand-up roll and one step of breathing
	room, short enough that staying in a boss pattern still punishes. */
	inline constexpr std::uint32_t PLAYER_HIT_REACTION_GRACE_TICKS = 60;

	struct SERVER_PLAYER
	{
		SESSION_ID iSessionId = INVALID_SESSION_ID;
		LostArk::Shared::PLAYER_ID iPlayerId =
			LostArk::Shared::INVALID_PLAYER_ID;
		LostArk::Shared::NET_ENTITY_ID iNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;

		std::string strNickName;
		std::string strSpawnPlacementId;

		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;

		std::uint32_t iLastMoveSequence = 0;
		std::uint32_t iLastReviveSequence = 0;
		std::uint32_t iLastClassChangeSequence = 0;
		float fMoveGoalX = 0.f;
		float fMoveGoalZ = 0.f;
		float fMoveSpeed = 6.f;
		bool hasMoveGoal = false;
		// Valtan cannot acquire or damage this player until the server accepts the
		// first valid move/skill intent after entry or revive.
		bool isCombatReady = true;
		std::vector<SERVER_NAV_POINT> MovePath;
		std::size_t iMovePathIndex = 0;
		/* The knockback in flight from a boss pattern or monster attack: unit XZ
		direction (a pull flips it toward the attacker when armed), metres per
		second, and the remaining window. While a window or a knockdown is
		active a new hit does not re-arm, so overlapping hits cannot stack. */
		float fKnockbackDirectionX = 0.f;
		float fKnockbackDirectionZ = 0.f;
		float fKnockbackSpeed = 0.f;
		float fKnockbackRemainingSeconds = 0.f;
		/* KNOCKDOWN holds until this tick; move and skill commands are rejected
		while it runs and the action returns to NONE when it expires. */
		std::uint32_t iKnockdownEndTick = 0;
		/* Get-up grace: until this tick no new hit reaction arms (damage still
		lands), so a boss cannot chain the player from one knockdown straight
		into the next. Set when a knockdown ends by expiry or by the STANDUP
		skill. */
		std::uint32_t iHitReactionGraceEndTick = 0;

		std::uint32_t iCurrentHp = 1000;
		std::uint32_t iMaximumHp = 1000;
		std::uint32_t iCurrentResource = 100;
		std::uint32_t iMaximumResource = 100;
		// Fixed-point regen carry in ticks: gains profile regen per tick and pays
		// out one resource per SERVER_TICK_HZ accumulated, so a second restores
		// exactly resourceRegenPerSecond with integers only.
		std::uint32_t iResourceAccumulator = 0;
		// The class identity gauge, and the same fixed-point carry the resource
		// pool uses. Both stay 0 for a class whose profile has no gauge.
		std::uint32_t iCurrentIdentity = 0;
		std::uint32_t iMaximumIdentity = 0;
		std::uint32_t iIdentityAccumulator = 0;
		LostArk::Shared::PLAYER_ACTION_STATE eAction =
			LostArk::Shared::PLAYER_ACTION_STATE::NONE;
		LostArk::Shared::PLAYER_STANCE_ID eStance =
			LostArk::Shared::PLAYER_STANCE_ID::NONE;
		// A LanceMaster-style pair of opposite-direction stance-swap skills (e.g.
		// 34000/34500) are tracked as separate CooldownEndTickBySkillId entries, so
		// nothing stops the reverse skill firing the instant the first one lands --
		// this shared gate is what actually prevents an immediate swap-back.
		std::uint32_t iStanceSwitchCooldownEndTick = 0;
		LostArk::Shared::SKILL_ID iCurrentSkillId =
			LostArk::Shared::INVALID_SKILL_ID;
		std::uint32_t iActionStartTick = 0;
		/* Live only while eAction is FALLING. The velocity integrates downward
		from zero at the tick the ground disappeared, and the death tick is the
		deadline that same tick scheduled. Neither is replicated: the client
		reads the descent from the position the snapshot already carries. */
		float fFallVelocityY = 0.f;
		std::uint32_t iFallDeathTick = 0u;
		SERVER_TRIGGER_MOVE TriggerMove;
		std::uint32_t iLastSkillSequence = 0;
		float fActionElapsedSeconds = 0.f;
		float fSkillAimDirectionX = 0.f;
		float fSkillAimDirectionZ = 1.f;
		// Distance from the caster to the aim point the press carried, so an
		// object that lands where the cursor points can be placed.
		float fSkillAimDistance = 0.f;
		bool hasAppliedSkillDamage = false;
		std::uint64_t iAppliedHitMask = 0;
		// Bit per projectile definition of the running stage already spawned.
		std::uint16_t iSpawnedProjectileMask = 0;
		std::vector<SERVER_SKILL_PROJECTILE> Projectiles;
		// 1-based while a combo action runs, 0 otherwise.
		std::uint8_t iComboStage = 0;
		// Set by a press inside the open window, consumed when the stage ends.
		bool hasBufferedComboInput = false;
		// The aim that press carried. The next stage turns to it, so a combo
		// follows the cursor instead of repeating the first stage's facing.
		float fBufferedComboAimX = 0.f;
		float fBufferedComboAimZ = 1.f;
		float fBufferedComboAimDistance = 0.f;
		// Set when a HOLD skill's key is let go, consumed when its loop ends.
		bool hasReleasedHold = false;
		std::unordered_map<LostArk::Shared::SKILL_ID, std::uint32_t>
			CooldownEndTickBySkillId;
		// Debug-only inventory slice. Small owned list, stacked per itemId and
		// capped at the catalog's maxStack; the Shared snapshot struct is
		// reused directly since the wire shape and the server truth are the
		// same {itemId, quantity} pair.
		std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT> Inventory;
	};
}

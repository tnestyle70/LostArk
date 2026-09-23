#pragma once

#include "ServerIds.h"

#include "Network/NetworkIds.h"
#include "Network/PacketType.h"
#include "Network/PacketMessages.h"
#include "ServerNavigation.h"

#include <array>
#include <optional>
#include <cstdint>
#include <bitset>
#include <string>
#include <unordered_map>
#include <vector>

namespace LostArk::Server
{
	struct SERVER_TRIGGER_MOVE_SAMPLE
	{
		std::uint32_t iTimeMs = 0u;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
	};

	struct SERVER_TRIGGER_MOVE
	{
		/* Empty for a validated direct move such as the Debug Mario jump.
		Authored triggers retain their stable source for destination-lane changes. */
		std::string strSourcePlacementId;
		float fStartX = 0.f;
		float fStartY = 0.f;
		float fStartZ = 0.f;
		float fTargetX = 0.f;
		float fTargetY = 0.f;
		float fTargetZ = 0.f;
		float fDurationSeconds = 0.f;
		float fElapsedSeconds = 0.f;
		float fArcHeight = 0.f;
		/* Empty is the established direct/arc move.  A populated list is an
		authored absolute TrackMove sampled by the Server, never by the Client. */
		std::vector<SERVER_TRIGGER_MOVE_SAMPLE> TrackSamples;
		LostArk::Shared::KOUKU_HUD_MODE eKoukuHudModeOnArrival = LostArk::Shared::KOUKU_HUD_MODE::END;
		bool isActive = false;
		/* The player stands still for fHoldSeconds after the action starts and only then travels
		(Bern castle/library: the Client darkens the screen during the wait). 0 travels at once. */
		float fHoldSeconds = 0.f;
		float fHeldSeconds = 0.f;
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
		std::bitset<192u> iAppliedTimedMask;
		std::vector<SERVER_PROJECTILE_CONTACT_MARK> ContactMarks;
	};

	/* Get-up grace window after a knockdown ends: 2000 ms at the fixed 30 Hz
	tick. Long enough to cover the stand-up roll and one step of breathing
	room, short enough that staying in a boss pattern still punishes. */
	inline constexpr std::uint32_t PLAYER_HIT_REACTION_GRACE_TICKS = 60;

	enum class PLAYER_PENDING_COMMAND_KIND : std::uint8_t
	{
		NONE,
		MOVE,
		SKILL
	};

	/* One explicit intent accepted while a COMBO stage owns the action.  Packet
	payloads are copied by value; arrival order, not unrelated MOVE/SKILL sequence
	spaces, makes the latest explicit intent replace the previous one. */
	struct SERVER_PENDING_PLAYER_COMMAND
	{
		PLAYER_PENDING_COMMAND_KIND eKind = PLAYER_PENDING_COMMAND_KIND::NONE;
		std::uint32_t iClientSequence = 0u;
		LostArk::Shared::SKILL_ID iSkillId =
			LostArk::Shared::INVALID_SKILL_ID;
		LostArk::Shared::SKILL_TARGET_INTENT_KIND eTargetIntent =
			LostArk::Shared::SKILL_TARGET_INTENT_KIND::AIM_POINT;
		float fX = 0.f;
		float fZ = 0.f;

		void Clear()
		{
			*this = {};
		}

		void Set_Move(const LostArk::Shared::C2S_MOVE& move)
		{
			Clear();
			eKind = PLAYER_PENDING_COMMAND_KIND::MOVE;
			iClientSequence = move.iClientSequence;
			fX = move.fGoalX;
			fZ = move.fGoalZ;
		}

		void Set_Skill(const LostArk::Shared::C2S_USE_SKILL& skill)
		{
			Clear();
			eKind = PLAYER_PENDING_COMMAND_KIND::SKILL;
			iClientSequence = skill.iClientSequence;
			iSkillId = skill.iSkillId;
			eTargetIntent = skill.eTargetIntent;
			fX = skill.fAimX;
			fZ = skill.fAimZ;
		}
	};

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
		// Server-validated Bern entry guide; retained until this raid visit ends.
		std::string strRaidReturnNpcPlacementId;

		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;

		std::uint32_t iLastMoveSequence = 0;
		std::uint32_t iLastReviveSequence = 0;
		std::uint32_t iLastClassChangeSequence = 0;
		/* One idempotent Debug verdict belongs to this player in this room. */
		LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT LastDebugTeleportResult;
		LostArk::Shared::S2C_DEBUG_MARIO_JUMP_RESULT LastDebugMarioJumpResult;
		LostArk::Shared::S2C_MARIO_RETURN_RESULT LastMarioReturnResult;
		LostArk::Shared::S2C_DEBUG_SET_MADNESS_FORM_RESULT LastDebugMadnessFormResult;
		LostArk::Shared::S2C_SET_VEHICLE_RIDING_RESULT LastVehicleRidingResult;
		LostArk::Shared::S2C_SET_HONOR_TITLE_RESULT LastHonorTitleResult;
		/* The ridden vehicle or INVALID_VEHICLE_ID on foot. Only riding worlds
		admit it, and Enforce_VehicleRidingState clears it before the snapshot
		whenever the player can no longer ride. */
		LostArk::Shared::VEHICLE_ID iVehicleId = LostArk::Shared::INVALID_VEHICLE_ID;
		LostArk::Shared::VEHICLE_FLIGHT_PHASE eVehicleFlightPhase = LostArk::Shared::VEHICLE_FLIGHT_PHASE::GROUNDED;
		std::uint32_t iVehicleFlightPhaseStartTick = 0u;
		float fVehicleFlightPhaseSeconds = 0.f;
		float fVehicleFlightGroundY = 0.f;
		float fVehicleFlightStartHeight = 0.f;
		float fVehicleFlightInputX = 0.f, fVehicleFlightInputZ = 0.f, fVehicleFlightInputY = 0.f;
		float fVehicleFlightInputAge = 0.f;
		float fVehicleFlightVelocityX = 0.f, fVehicleFlightVelocityZ = 0.f, fVehicleFlightVelocityY = 0.f;
		/* Worn honor title (cosmetic); admitted from the honor title bootstrap only and
		carried through world transfers. */
		LostArk::Shared::HONOR_TITLE_ID iHonorTitleId = LostArk::Shared::INVALID_HONOR_TITLE_ID;
		std::uint8_t iMarioStage = 0u;
		std::uint8_t iMarioLayoutVariant = 0u;
		// Pinned by the entry pattern; survives the arena phase and terminal move start.
		std::optional<std::array<float, 3u>> MarioReturnPosition;
		// Safe arena revive point retained through the fall's below-floor death pose.
		std::optional<std::array<float, 3u>> KoukuFallRevivePosition;
		bool bKoukuFallDeath = false;
		LostArk::Shared::PLAYER_MADNESS_FORM ePreMarioForm =
			LostArk::Shared::PLAYER_MADNESS_FORM::NORMAL;
		std::uint32_t iLastMarioMoveSequence = 0u;
		std::uint32_t iMarioMoveExpiryTick = 0u;
		float fMarioDirectionX = 0.f;
		float fMarioDirectionZ = 0.f;
		bool bMarioRailReady = false;
		std::string strMarioRailArrivalId;
		float fMarioRailOriginX = 0.f;
		float fMarioRailOriginZ = 0.f;
		float fMarioRailRightX = 0.f;
		float fMarioRailRightZ = 0.f;
		void Clear_MarioControl(const bool preserveReturnPosition = false)
		{
			if (0u != iMarioStage)
			{
				eMadnessForm = ePreMarioForm;
				/* The stage owns the arena HUD mode the same way it owns the
				form, so leaving hands both back. Clear_KoukuInteractionState
				is not used here: it would also drop a Debug override. */
				eKoukuAreaHudMode = LostArk::Shared::KOUKU_HUD_MODE::NONE;
				hasMoveGoal = false;
				MovePath.clear();
				iMovePathIndex = 0u;
			}
			if (!preserveReturnPosition) MarioReturnPosition.reset();
			iMarioStage = 0u;
			iMarioLayoutVariant = 0u;
			iMarioMoveExpiryTick = 0u;
			fMarioDirectionX = fMarioDirectionZ = 0.f;
			bMarioRailReady = false;
			strMarioRailArrivalId.clear();
			fMarioRailOriginX = fMarioRailOriginZ = 0.f;
			fMarioRailRightX = fMarioRailRightZ = 0.f;
		}
		float fMoveGoalX = 0.f;
		float fMoveGoalZ = 0.f;
		/* The point the client last asked for, before navigation projected it
		onto walkable ground. A goal picked on top of an obstacle projects
		metres away, so only the request itself can tell a held re-send of the
		same goal apart from a genuinely new one. */
		float fMoveRequestX = 0.f;
		float fMoveRequestZ = 0.f;
		float fMoveSpeed = 6.f;
		bool hasMoveGoal = false;
		// Valtan cannot acquire or damage this player until the server accepts the
		// first valid move/skill intent after entry or revive.
		bool isCombatReady = true;
		/* Empty means the player is steering straight at fMoveGoal, which is what
		a goal with a clear line to it -- and so a held right mouse -- uses.
		Routing only fills this in when that straight line is closed. */
		std::vector<SERVER_NAV_POINT> MovePath;
		std::size_t iMovePathIndex = 0;
		/* When the body sweep last met something the grid does not carry and the
		move was routed around it. Rate limits that rebuild: brushing an obstacle
		reports blocked on many ticks in a row. */
		std::uint32_t iMoveRerouteTick = 0u;
		/* The knockback in flight from a boss pattern or monster attack: unit XZ
		direction (a pull flips it toward the attacker when armed), metres per
		second, and the remaining window. While a window or a knockdown is
		active a new hit does not re-arm, so overlapping hits cannot stack. */
		float fKnockbackDirectionX = 0.f;
		float fKnockbackDirectionZ = 0.f;
		float fKnockbackSpeed = 0.f;
		float fKnockbackRemainingSeconds = 0.f;
		// Explicit authored push policy; ordinary knockback still stops at navigation edges.
		bool bKnockbackCanLeaveArena = false;
		// Explicit mechanic flight; normal knockback remains constrained to walking ground.
		bool bKnockbackBallistic = false;
		float fKnockbackVelocityY = 0.f;
		float fKnockbackLaunchY = 0.f;
		float fKnockbackSupportY = 0.f;
		static constexpr float KNOCKBACK_GRAVITY_MPS2 = 9.8f;
		float fKnockbackGravityMps2 = KNOCKBACK_GRAVITY_MPS2;
		/* Typed release policy for the existing knockback integrator. Only arena
		ejection ignores nav/collision and ends in the ordinary FALLING state. */
		bool bArenaEjectionActive = false;
		LostArk::Shared::NET_ENTITY_ID iEjectionOwnerNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		/* KNOCKDOWN holds until this tick; move and skill commands are rejected
		while it runs and the action returns to NONE when it expires. */
		std::uint32_t iKnockdownEndTick = 0;
		std::uint32_t iFearEndTick = 0u;
		std::string strFearPresentationId;
		// Current-tick zone contact expires presentation on exit; protection stays in the pattern runtime.
		std::uint32_t iInvulnerabilityZoneContactTick = 0u;
		std::uint32_t iInvulnerabilityZonePulseTick = 0u;
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
		/* Guardian Knight ember. Orbs held now, sockets a human-form expression
		skill has locked (each one lowers the pool cap), and the orbs the running
		action spent, which its damage reads. All 0 for every other class. */
		std::uint32_t iEmberOrbs = 0;
		std::uint32_t iEmberLockedSockets = 0;
		std::uint32_t iEmberSpentOnAction = 0;
		/* KoukuSaydon madness gauge and the avatar it drives. The maximum is a
		fixed first value until the encounter owns it; nothing raises the
		current value yet. The form is Server truth the Client presents; the
		Debug F1 toggle and authored Mario entry change it. */
		static constexpr std::uint32_t MADNESS_GAUGE_MAXIMUM = 10000u;
		std::uint32_t iCurrentMadness = 0;
		std::uint32_t iMaximumMadness = MADNESS_GAUGE_MAXIMUM;
		LostArk::Shared::PLAYER_MADNESS_FORM eMadnessForm =
			LostArk::Shared::PLAYER_MADNESS_FORM::NORMAL;
		/* Tick the gauge-driven clown hold expires at; 0 means no expiry, which
		is what the Debug F1 toggle leaves so only "Return to Player" ends it. */
		std::uint32_t iMadnessFormEndTick = 0u;
		/* KoukuSaydon interaction HUD truth, recomputed every tick by
		CKoukuSaydonLogicRuntime::Update_PlayerModes: which mode the Client
		draws and which authored icon each Q..F slot carries (-1 empty). Dance
		poses always use Q/W/E/R = Superman/open arms/one leg/folded arms. */
		LostArk::Shared::KOUKU_HUD_MODE eKoukuHudMode =
			LostArk::Shared::KOUKU_HUD_MODE::NONE;
		LostArk::Shared::KOUKU_HUD_MODE eDebugKoukuHudModeOverride =
			LostArk::Shared::KOUKU_HUD_MODE::NONE;
		std::int8_t ModeSkillIndexBySlot[LostArk::Shared::KOUKU_HUD_SLOT_COUNT] =
			{ -1, -1, -1, -1, -1, -1, -1, -1 };
		std::uint32_t iLastKoukuInteractionSequence = 0u;
		bool bKoukuPatternOwnsClown = false;
		std::uint32_t iKoukuSuppressedPatternSequence = 0u;
		LostArk::Shared::KOUKU_HUD_MODE eKoukuAreaHudMode = LostArk::Shared::KOUKU_HUD_MODE::NONE;
		/* One suit/color dealt by the active Gate 1 Saydon encounter. */
		LostArk::Shared::MECHANIC_CARD_SYMBOL eMechanicCardSymbol =
			LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE;
		LostArk::Shared::MECHANIC_CARD_COLOR eMechanicCardColor = LostArk::Shared::MECHANIC_CARD_COLOR::NONE;
		/* Card maze truth dealt by CKoukuCardMazeRuntime when the telescope is
		claimed. Cleared with the run, never by the roulette that shares the
		suit enum. */
		LostArk::Shared::CARD_MAZE_ROLE eCardMazeRole = LostArk::Shared::CARD_MAZE_ROLE::NONE;
		LostArk::Shared::MECHANIC_CARD_SYMBOL eCardMazeSuit = LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE;
		std::uint8_t iCardMazeKills = 0u;
		std::uint8_t iCardMazeKillTarget = 0u;
		LostArk::Shared::CARD_MAZE_PRESENTATION CardMaze;
		float fCardMazeTransferX = 0.f, fCardMazeTransferY = 0.f, fCardMazeTransferZ = 0.f;
		bool bCardMazeTransferCommitted = false;
		void Clear_CardMazeState() noexcept
		{
			eCardMazeRole = LostArk::Shared::CARD_MAZE_ROLE::NONE;
			eCardMazeSuit = LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE;
			iCardMazeKills = 0u;
			iCardMazeKillTarget = 0u;
			CardMaze = {};
		}
		LostArk::Shared::S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT LastDebugKoukuHudModeResult;

		void Clear_KoukuInteractionState()
		{
			iMadnessFormEndTick = 0u;
			bKoukuPatternOwnsClown = false;
			iKoukuSuppressedPatternSequence = 0u;
			eKoukuAreaHudMode = LostArk::Shared::KOUKU_HUD_MODE::NONE;
			eKoukuHudMode = LostArk::Shared::KOUKU_HUD_MODE::NONE;
			eDebugKoukuHudModeOverride = LostArk::Shared::KOUKU_HUD_MODE::NONE;
			for (std::int8_t& index : ModeSkillIndexBySlot)
				index = -1;
		}
		void Clear_KoukuAssignedCard()
		{
			eMechanicCardSymbol = LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE;
			eMechanicCardColor = LostArk::Shared::MECHANIC_CARD_COLOR::NONE;
		}
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
		/* The square-hole row (1-based) whose song this player is in; 0 when none. The
		Server moves the player to that row's destination when the song lock ends, and
		Update_Players drops it on any tick the action is no longer SQUAREHOLE_SONG. */
		std::uint16_t iSquareHoleId = 0u;
		/* One boss-pattern bind occurrence owns this status. The Server keeps the
		pre-bind pose so every exit path can restore the exact admitted pose before
		returning movement/action control. */
		bool bPatternBound = false;
		LostArk::Shared::NET_ENTITY_ID iPatternBindOwnerNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint32_t iPatternBindSequence = 0u;
		std::uint32_t iPatternBindEndTick = 0u;
		float fPatternBindRestoreX = 0.f;
		float fPatternBindRestoreY = 0.f;
		float fPatternBindRestoreZ = 0.f;
		float fPatternBindRestoreYawDegrees = 0.f;
		bool bPatternBindRestoreCombatReady = false;
		/* Silence never blocks movement. Its deadline may outlive the applying boss
		occurrence; only deadline, death, or an explicit room/reset boundary clears it. */
		LostArk::Shared::NET_ENTITY_ID iSilenceOwnerNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint32_t iSilencePatternSequence = 0u;
		std::uint32_t iSilenceEndTick = 0u;
		std::uint32_t iSilenceDurationTicks = 0u;

		void Clear_PatternBindStatus()
		{
			bPatternBound = false;
			iPatternBindOwnerNetEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			iPatternBindSequence = 0u;
			iPatternBindEndTick = 0u;
			fPatternBindRestoreX = 0.f;
			fPatternBindRestoreY = 0.f;
			fPatternBindRestoreZ = 0.f;
			fPatternBindRestoreYawDegrees = 0.f;
			bPatternBindRestoreCombatReady = false;
		}

		void Clear_SilenceStatus()
		{
			iSilenceOwnerNetEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			iSilencePatternSequence = 0u;
			iSilenceEndTick = 0u;
			iSilenceDurationTicks = 0u;
		}
		/* GRABBED is Server authority. The slot is a Shared typed identity and
		these boss-local offsets are the canonical attachment snapshot used to
		recompute player world position and yaw. The Client does not compose a
		presentation hand bone. */
		LostArk::Shared::NET_ENTITY_ID iAttachmentOwnerNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT eAttachmentSlot =
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
		std::uint32_t iAttachmentPatternSequence = 0u;
        // Zero keeps the existing stage-controlled Valtan attachment lifetime.
        std::uint32_t iAttachmentEndTick = 0u;
		float fAttachmentLocalOffsetX = 0.f;
		float fAttachmentLocalOffsetY = 0.f;
		float fAttachmentLocalOffsetZ = 0.f;
		float fAttachmentYawOffsetDegrees = 0.f;
		/* WORLD_HOOK_TIP rides an authored judgement region instead of a boss
		bone, so the player names the region that caught it and the tick that
		region's world track lets go. The owner id still names the boss whose
		pattern owns the region, which is what every existing release path keys
		on. Canonical zero for every other slot. */
		std::uint32_t iAttachmentWindowIndex = 0u;
		std::uint32_t iAttachmentRegionIndex = 0u;
		std::uint32_t iAttachmentReleaseTick = 0u;

		void Clear_Attachment()
		{
			bArenaEjectionActive = false;
			bKnockbackCanLeaveArena = false;
			bKnockbackBallistic = false;
			fKnockbackVelocityY = 0.f;
			iEjectionOwnerNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			iAttachmentOwnerNetEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			eAttachmentSlot = LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
			iAttachmentPatternSequence = 0u;
            iAttachmentEndTick = 0u;
			fAttachmentLocalOffsetX = 0.f;
			fAttachmentLocalOffsetY = 0.f;
			fAttachmentLocalOffsetZ = 0.f;
			fAttachmentYawOffsetDegrees = 0.f;
			iAttachmentWindowIndex = 0u;
			iAttachmentRegionIndex = 0u;
			iAttachmentReleaseTick = 0u;
		}
		/* Live only while eAction is FALLING. The velocity integrates downward
		from zero at the tick the ground disappeared, and the death tick is the
		deadline that same tick scheduled. Neither is replicated: the client
		reads the descent from the position the snapshot already carries. */
		float fFallVelocityY = 0.f;
		// Kouku uses a plane five metres below the support/flight launch height.
		float fFallDeathPlaneY = 0.f;
		std::uint32_t iFallDeathTick = 0u;
		SERVER_TRIGGER_MOVE TriggerMove;
		std::uint32_t iLastSkillSequence = 0;
		float fActionElapsedSeconds = 0.f;
		float fSkillAimDirectionX = 0.f;
		float fSkillAimDirectionZ = 1.f;
		// Distance from the caster to the aim point the press carried, so an
		// object that lands where the cursor points can be placed.
		float fSkillAimDistance = 0.f;
		/* Committed only after the ground-point request passes finite/range/nav
		 validation. It remains stable for the action and is copied to every
		 snapshot so damage and presentation share one authoritative root. */
		bool hasSkillTarget = false;
		float fSkillTargetX = 0.f;
		float fSkillTargetY = 0.f;
		float fSkillTargetZ = 0.f;

		void Clear_SkillTarget()
		{
			hasSkillTarget = false;
			fSkillTargetX = 0.f;
			fSkillTargetY = 0.f;
			fSkillTargetZ = 0.f;
		}
		bool hasAppliedSkillDamage = false;
		std::bitset<192u> iAppliedHitMask;
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
		SERVER_PENDING_PLAYER_COMMAND PendingCommand;
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

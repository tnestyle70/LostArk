#pragma once

#include "KoukuSaydonAnimationActionDocument.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace Client
{
	struct KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE final
	{
		std::string strOccurrenceId;
		std::string strProfileId;
		// Action 0 is a valid reference; physical clips use action 0 plus stage RAW.
		std::uint32_t iSourceActionId = 0u;
		std::string strSourceStageId;
		std::string strSourceSlotId;
		std::string strReferenceRevision;
		std::string strRuntimeClip;
		std::uint32_t iStartOffsetMs = 0u;
		std::uint32_t iSourceStartMs = 0u;
		std::uint32_t iPlayMs = 0u;
		std::uint32_t iBlendInMs = 0u;
		f32_t fPlayRate = 1.f;
		std::string strEndPolicy;

		bool operator==(
			const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE&) const = default;
	};

	struct KOUKU_SAYDON_COMPOSITION_STAGE final
	{
		std::string strStageId;
		std::string strActionId;
		std::string strStageKind;
		std::uint32_t iDurationMs = 0u;
		std::vector<KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE>
			AnimationOccurrences;
		bool bRetargetOnEnter = false;

		bool operator==(const KOUKU_SAYDON_COMPOSITION_STAGE&) const = default;
	};

	/* The judgement a DURATION Logic runs and the outcome a RESULT Logic
	   applies. Both are the Server's typed vocabulary; a definition that is
	   only a name keeps the kind empty and stays DRAFT-only. */
	inline constexpr std::array<const char_t*, 9u> KOUKU_SAYDON_JUDGEMENT_KINDS = {
		"ROULETTE_CARD_MATCH", "GAZE_REAL_BOSS", "POSE_INPUT", "STAGGER_WINDOW", "COUNTER_WINDOW", "AREA_OVERLAP", "OBJECT_OVERLAP", "EXTERNAL_SIGNAL", "ATTACHMENT_HOLD" };
	inline constexpr std::array<const char_t*, 11u> KOUKU_SAYDON_OUTCOME_KINDS = {
		"INSTANT_DEATH", "MAX_HP_PERCENT_DAMAGE", "MADNESS_GAUGE_ADD_PERCENT",
		"CLOWN_TRANSFORM", "FEAR", "FOLLOWUP_PATTERN", "PLAY_WORLD_OBJECT_MOTION",
		"PLAY_CONTACT_WORLD_OBJECT_MOTION", "COMPLETE_LOGIC_WINDOW", "CAPTURE_PLAYER", "GRAB_TO_WORLD_OBJECT" };
	inline constexpr std::array<const char_t*, 4u> KOUKU_SAYDON_CARD_SYMBOLS = {
		"HEART", "SPADE", "CLUB", "DIAMOND" };
	inline constexpr std::size_t KOUKU_SAYDON_MAX_OUTCOMES_PER_SLOT = 4u;
	inline constexpr std::uint32_t KOUKU_SAYDON_DANCE_POSE_COUNT = 4u;

	enum class KOUKU_SAYDON_OUTCOME_SLOT : std::uint8_t
	{
		SUCCESS,
		FAIL,
		TIMEOUT,
	};

	/* An end-tick judgement (roulette card, real-boss gaze) is decided once
	   when its window closes. Roulette outside every mapped region is Timeout;
	   gaze has only Success and Fail. The
	   stagger window has no wrong answer, so it never ends in Fail. */
	inline bool_t Kouku_IsEndTickJudgement(const std::string_view judgementKind)
	{
		return judgementKind == "ROULETTE_CARD_MATCH" ||
			judgementKind == "GAZE_REAL_BOSS" || judgementKind == "AREA_OVERLAP";
	}
	inline bool_t Kouku_IsOutcomeSlotAllowed(
		const std::string_view judgementKind,
		const KOUKU_SAYDON_OUTCOME_SLOT slot)
	{
		if (judgementKind == "ATTACHMENT_HOLD") return false;
		if (KOUKU_SAYDON_OUTCOME_SLOT::TIMEOUT == slot)
			return judgementKind != "GAZE_REAL_BOSS" && judgementKind != "OBJECT_CONTACT";
		if (KOUKU_SAYDON_OUTCOME_SLOT::FAIL == slot)
			return judgementKind != "STAGGER_WINDOW" && judgementKind != "COUNTER_WINDOW" && judgementKind != "ENTER_AREA" &&
				judgementKind != "OBJECT_CONTACT" && judgementKind != "EXTERNAL_SIGNAL";
		return true;
	}

	/* One reusable Logic definition owned by the composition document. The
	   identity, name and type are always present; the typed values below are
	   read as optional keys so a definition can be named first and typed later. */
	struct KOUKU_SAYDON_CONTACT_MOTION final
	{
		std::string strTargetWorldOccurrenceId;
		std::string strMotionInstanceId;
		bool operator==(const KOUKU_SAYDON_CONTACT_MOTION&) const = default;
	};

	struct KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION final
	{
		std::string strLogicId;
		std::string strDisplayName;
		std::string strLogicType;
		/* DURATION values. Only the keys of strJudgementKind are meaningful. */
		std::string strJudgementKind;
		std::uint32_t iSectorCount = 0u;
		std::vector<std::string> SectorSymbols;
		std::vector<std::string> RegionIds;
		double fCenterX = 0.0;
		double fCenterZ = 0.0;
		double fOuterRadiusM = 0.0;
		std::string strWorldSequenceInstanceId;
		double fHalfAngleDegrees = 0.0;
		double fMaxDistanceM = 0.0;
		std::uint32_t iPoseIndex = 0u;
		std::uint32_t iThreshold = 0u;
		double fShieldArcDegrees = 0.0;
		bool_t bEndsPatternOnSuccess = false;
		double fNormalYawOffsetDegrees = 0.0;
		std::string strInsideOutcome = "SUCCESS";
		/* RESULT values. */
		std::string strOutcomeKind;
		std::uint32_t iPercent = 0u;
		std::uint32_t iDurationMs = 0u;
		std::string strFollowupPatternId;
		std::string strSceneProfileId;
		std::string strEffectResourceId;
		std::string strLightResourceId;
		std::uint32_t iEffectDelayMs = 0u;
		std::string strAttachmentSlot;
		// Metres in the owner's facing basis: forward, up, right.
		std::array<double, 3u> GripLocalOffset{};
		std::string strTargetWorldInstanceId;
		std::string strMotionInstanceId;
		double fTargetRadiusM = 0.0;
		/* TRIGGER values are projected to Server mechanic cues. */
		std::string strTriggerKind;
		double fBossChargeDistanceM = 0.0;
		std::string strHudMode;
		std::array<double, 3> TeleportPosition{};
		std::string strClonePatternId;
		std::vector<std::uint32_t> ClockHours;
		double fFaceCenterYawOffsetDegrees = 0.0;
		std::vector<std::string> TargetWorldOccurrenceIds;
		std::string strContactGroupId;
		std::uint32_t iContactPriority = 0u;
		std::vector<KOUKU_SAYDON_CONTACT_MOTION> ContactMotions;
		std::string strTargetLogicOccurrenceId;
		std::string strContactTargetWorldOccurrenceId;

		bool operator==(
			const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION&) const = default;
	};

	inline bool_t Kouku_LogicOwnsOutcomes(const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic)
	{
		return (logic.strLogicType == "DURATION" && logic.strJudgementKind != "ATTACHMENT_HOLD") ||
			(logic.strLogicType == "TRIGGER" && (logic.strTriggerKind == "ENTER_AREA" || logic.strTriggerKind == "OBJECT_CONTACT"));
	}
	inline bool_t Kouku_LogicAcceptsColliders(const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic)
	{
		return (logic.strLogicType == "DURATION" && logic.strJudgementKind != "EXTERNAL_SIGNAL" && logic.strJudgementKind != "COUNTER_WINDOW" && logic.strJudgementKind != "ATTACHMENT_HOLD") ||
			(logic.strLogicType == "TRIGGER" && (logic.strTriggerKind == "ENTER_AREA" || logic.strTriggerKind == "OBJECT_CONTACT"));
	}
	inline const std::string& Kouku_LogicOutcomeKind(const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic)
	{
		return logic.strLogicType == "TRIGGER" ? logic.strTriggerKind : logic.strJudgementKind;
	}

	/* One placed Logic box. Its window is pattern-relative, not Stage-relative,
	   so a box can span the repeated clips it judges. */
	struct KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE final
	{
		std::string strOccurrenceId;
		std::string strLogicId;
		bool_t bEnabled = true;
		std::uint32_t iStartMs = 0u;
		std::uint32_t iDurationMs = 0u;
		/* Outcome wiring of a DURATION box: each slot lists up to four RESULT
		   Logics applied in order. The box, not the definition, owns where
		   success, failure and timeout go. */
		std::vector<std::string> OnSuccessLogicIds;
		std::vector<std::string> OnFailLogicIds;
		std::vector<std::string> OnTimeoutLogicIds;
		// ENTER_AREA capture uses this same-pattern ATTACHMENT_HOLD deadline.
		std::string strHoldLogicOccurrenceId;

		std::vector<std::string>& Outcomes(const KOUKU_SAYDON_OUTCOME_SLOT slot)
		{
			return KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS == slot ? OnSuccessLogicIds :
				(KOUKU_SAYDON_OUTCOME_SLOT::FAIL == slot ? OnFailLogicIds : OnTimeoutLogicIds);
		}
		const std::vector<std::string>& Outcomes(const KOUKU_SAYDON_OUTCOME_SLOT slot) const
		{
			return KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS == slot ? OnSuccessLogicIds :
				(KOUKU_SAYDON_OUTCOME_SLOT::FAIL == slot ? OnFailLogicIds : OnTimeoutLogicIds);
		}

		bool operator==(
			const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE&) const = default;
	};

	/* One reusable Summon definition. Only its name is authored today; the
	   archetype it spawns and the Pattern the spawn plays are later fields. */
	struct KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION final
	{
		std::string strSummonId;
		std::string strDisplayName;

		bool operator==(
			const KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION&) const = default;
	};

	/* One placed Summon box: startMs is the spawn time and durationMs the
	   lifetime after which the spawn despawns. Pattern-relative like Logic. */
	struct KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE final
	{
		std::string strOccurrenceId;
		std::string strSummonId;
		std::uint32_t iStartMs = 0u;
		std::uint32_t iDurationMs = 0u;

		bool operator==(
			const KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE&) const = default;
	};

	/* One authored world sequence instance of the Area, named for the lane. A
	   box plays it on the pattern clock at the box playback speed. */
	struct KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION final
	{
		std::string strWorldId;
		std::string strDisplayName;
		std::string strSequenceInstanceId;
		// Optional parent Object; sequenceInstanceId pins the initial state chosen at Append.
		std::string strObjectResourceId;
		std::array<double, 3u> PositionOffset{};
		std::string strAnchorKind = "NONE";
		std::array<double, 3u> AnchorPosition{};
		std::string strCompanionEffectResourceId;

		bool operator==(
			const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION&) const = default;
	};

	// Absolute WORLD frame of one cloned Object occurrence; saved motion stays local.
	struct KOUKU_SAYDON_WORLD_PLACEMENT final
	{
		std::array<double, 3u> Position{};
		std::array<double, 3u> RotationDegrees{};
		std::array<double, 3u> Scale{ 1.0, 1.0, 1.0 };
		bool operator==(const KOUKU_SAYDON_WORLD_PLACEMENT&) const = default;
	};

	struct KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE final
	{
		std::string strOccurrenceId;
		std::string strWorldId;
		std::uint32_t iStartMs = 0u;
		std::uint32_t iDurationMs = 0u;
		f32_t fPlaybackSpeed = 1.f;

		std::optional<KOUKU_SAYDON_WORLD_PLACEMENT> Placement;

		bool operator==(
			const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE&) const = default;
	};

	/* One rendering profile named for the lane. A box applies it on the
	   pattern clock for the box lifetime and blends in over blendMs. */
	struct KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION final
	{
		std::string strSceneProfileId;
		std::string strDisplayName;
		std::string strRenderingProfileId;

		bool operator==(
			const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION&) const = default;
	};

	struct KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE final
	{
		std::string strOccurrenceId;
		std::string strSceneProfileId;
		std::uint32_t iStartMs = 0u;
		std::uint32_t iDurationMs = 0u;
		std::uint32_t iBlendMs = 0u;

		bool operator==(
			const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE&) const = default;
	};

	enum class KOUKU_SAYDON_PRESENTATION_KIND : std::uint8_t
	{
		EFFECT, SOUND, CAMERA, COLLIDER, LIGHT, WORLD, SCENE_PROFILE
	};

	/* EFFECT/SOUND/CAMERA/COLLIDER/LIGHT definitions share stable resource identity.
	   WORLD and SCENE_PROFILE keep their existing document families and use
	   this vocabulary only in a one-shot resource preview request. */
	struct KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE final
	{
		std::string strResourceId;
		std::string strDisplayName;
		KOUKU_SAYDON_PRESENTATION_KIND eKind = KOUKU_SAYDON_PRESENTATION_KIND::EFFECT;
		std::string strAssetId;
		std::string strResourceKind = "GROUP";
		std::string strElementId;
		std::string strDefaultAnchorKind = "BOSS";
		std::uint32_t iDurationMs = 1000u;
		std::string strShape = "BOX";
		std::string strColliderKind = "GEOMETRY";
		std::array<double, 3u> HalfExtents{ 1.0, 1.0, 1.0 };
		double fRadiusM = 3.0;
		double fHalfAngleDegrees = 45.0;
		bool operator==(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE&) const = default;
	};

	/* Values are per occurrence; tuning a box never rewrites an Effect V2
	   group's children or the source leaf, camera shot, or audio asset. */
	struct KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE final
	{
		std::string strOccurrenceId;
		std::string strResourceId;
		std::uint32_t iStartMs = 0u;
		std::uint32_t iDurationMs = 1000u;
		std::array<double, 3u> PositionOffset{};
		std::array<double, 3u> RotationDegrees{};
		std::array<double, 3u> Scale{ 1.0, 1.0, 1.0 };
		std::uint32_t iFadeInMs = 0u;
		std::uint32_t iFadeOutMs = 0u;
		double fDissolveStart = 0.85;
		double fDissolveEnd = 1.0;
		double fVolume = 1.0;
		double fBrightnessMultiplier = 1.0;
		bool_t bFollowBoss = true;
		bool_t bDebugRender = true;
		std::string strBone;
		std::string strBoneTarget = "BODY";
		// Each placed region owns its card, independently of the reusable shape.
		std::string strRegionId;
		std::string strCardSymbol = "NONE";
		std::string strCardColor = "NONE";
		std::string strAnchorKind = "BOSS";
		std::string strWorldId;
		std::string strLogicOccurrenceId;
		std::string strWorldOccurrenceId;
		// Row of that World box's authored emission list this box follows; 0 for single emitters.
		std::uint32_t iWorldEmissionIndex = 0u;
		bool operator==(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE&) const = default;
	};

	/* The madness gauge maximum and the clown hold the encounter applies to
	   every player; the projector copies it into the Product. */
	struct KOUKU_SAYDON_COMPOSITION_MADNESS_POLICY final
	{
		std::uint32_t iMaximum = 100u;
		std::uint32_t iClownHoldMs = 15000u;

		bool operator==(
			const KOUKU_SAYDON_COMPOSITION_MADNESS_POLICY&) const = default;
	};

	// DURATION judges a window, TRIGGER starts a Pattern, RESULT applies an outcome.
	inline constexpr std::array<const char_t*, 3u> KOUKU_SAYDON_LOGIC_TYPES = {
		"DURATION", "TRIGGER", "RESULT" };

	// Absolute world base transform; animation keeps its original vertical pose.
	struct KOUKU_SAYDON_BOSS_MOTION final
	{
		std::uint32_t iStartMs = 0u;
		std::uint32_t iEndMs = 1u;
		std::array<double, 3u> StartPosition{};
		std::array<double, 3u> EndPosition{};
		double fYawDegrees = 0.0;
		bool operator==(const KOUKU_SAYDON_BOSS_MOTION&) const = default;
	};

	struct KOUKU_SAYDON_COMPOSITION_PATTERN final
	{
		std::string strPatternId;
		std::string strActorProfileId;
		std::string strGateId = "GATE1";
		std::string strTargetBossPlacementId;
		// Independent Pattern placement; bundle membership remains in Members.
		std::string strFolderId;
		std::string strDisplayName;
		std::string strAuthoringStatus;
		std::string strCategory;
		std::uint32_t iNextStageOrdinal = 1u;
		std::uint32_t iNextAnimationOrdinal = 1u;
		std::uint32_t iNextLogicOccurrenceOrdinal = 1u;
		std::uint32_t iNextSummonOccurrenceOrdinal = 1u;
		std::uint32_t iNextWorldOccurrenceOrdinal = 1u;
		std::uint32_t iNextSceneProfileOccurrenceOrdinal = 1u;
		std::uint32_t iNextPresentationOccurrenceOrdinal = 1u;
		bool_t bResetBossToSpawn = false;
		std::optional<double> ResetBossYawDegrees;
		std::optional<KOUKU_SAYDON_BOSS_MOTION> BossMotion;
		double fAnimationRootVerticalScale = 1.0;
		std::vector<KOUKU_SAYDON_COMPOSITION_STAGE> Stages;
		std::vector<KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE> LogicOccurrences;
		std::vector<KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE> SummonOccurrences;
		std::vector<KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE> WorldOccurrences;
		std::vector<KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE> SceneProfileOccurrences;
		std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE> PresentationOccurrences;
		// A malformed Pattern remains visible and survives unrelated saves.
		// These fields are editor state; Serialize writes the preserved JSON.
		std::string strLoadError;
		std::string strPreservedJson;

		bool operator==(const KOUKU_SAYDON_COMPOSITION_PATTERN&) const = default;
	};

	// Holds start/end outside the interval. False leaves outputs unchanged.
	[[nodiscard]] bool Sample_KoukuSaydonBossMotion(
		const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, double patternTimeMs,
		std::array<double, 3u>& outPosition, double& outYawDegrees) noexcept;

	struct KOUKU_SAYDON_COMPOSITION_FOLDER final
	{
		std::string strFolderId;
		std::string strGateId;
		std::string strDisplayName;
		std::string strLoadError;
		std::string strPreservedJson;
		bool operator==(const KOUKU_SAYDON_COMPOSITION_FOLDER&) const = default;
	};

	struct KOUKU_SAYDON_COMPOSITION_BUNDLE_MEMBER final
	{
		std::string strMemberId;
		std::string strPatternId;
		std::uint32_t iStartOffsetMs = 0u;
		bool operator==(const KOUKU_SAYDON_COMPOSITION_BUNDLE_MEMBER&) const = default;
	};

	struct KOUKU_SAYDON_COMPOSITION_BUNDLE final
	{
		std::string strBundleId;
		std::string strGateId;
		std::string strFolderId;
		std::string strDisplayName;
		std::string strAuthoringStatus = "DRAFT";
		std::uint32_t iNextMemberOrdinal = 1u;
		std::uint32_t iNextSceneProfileOccurrenceOrdinal = 1u;
		std::uint32_t iNextPresentationOccurrenceOrdinal = 1u;
		std::vector<KOUKU_SAYDON_COMPOSITION_BUNDLE_MEMBER> Members;
		std::vector<KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE> SceneProfileOccurrences;
		std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE> PresentationOccurrences;
		std::string strLoadError;
		std::string strPreservedJson;
		bool operator==(const KOUKU_SAYDON_COMPOSITION_BUNDLE&) const = default;
	};

	struct KOUKU_SAYDON_COMPOSITION_DOCUMENT final
	{
		std::uint32_t iFormatVersion = 3u;
		std::uint32_t iRevision = 1u;
		std::string strCompositionId;
		std::string strEncounterId;
		std::string strBossArchetypeId;
		std::string strBossPlacementId;
		std::string strAreaId;
		std::uint32_t iFixedTickHz = 30u;
		std::uint32_t iNextPatternOrdinal = 1u;
		std::uint32_t iNextFolderOrdinal = 1u;
		std::uint32_t iNextBundleOrdinal = 1u;
		std::uint32_t iNextLogicOrdinal = 1u;
		std::uint32_t iNextSummonOrdinal = 1u;
		std::uint32_t iNextWorldOrdinal = 1u;
		std::uint32_t iNextSceneProfileOrdinal = 1u;
		std::uint32_t iNextPresentationResourceOrdinal = 1u;
		KOUKU_SAYDON_COMPOSITION_MADNESS_POLICY MadnessPolicy;
		std::vector<std::string> PlayAllPatternIds;
		std::vector<KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION> Logics;
		std::vector<KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION> Summons;
		std::vector<KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION> Worlds;
		std::vector<KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION> SceneProfiles;
		std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE> PresentationResources;
		std::vector<KOUKU_SAYDON_COMPOSITION_PATTERN> Patterns;
		std::vector<KOUKU_SAYDON_COMPOSITION_FOLDER> Folders;
		std::vector<KOUKU_SAYDON_COMPOSITION_BUNDLE> Bundles;

		bool operator==(const KOUKU_SAYDON_COMPOSITION_DOCUMENT&) const = default;
	};

	struct KOUKU_SAYDON_ACTION_REFERENCE_SET final
	{
		std::array<KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT, 4u>
			Documents;
		std::array<std::string, 4u> SourceBytes;

		bool operator==(const KOUKU_SAYDON_ACTION_REFERENCE_SET&) const = default;
	};

	/* Composition edits own their file. Optional animation references assist
	   Resource browsing but never gate the Pattern list or composition Save. */
	class CKoukuSaydonCompositionDocument final
	{
	public:
		CKoukuSaydonCompositionDocument() = default;
		explicit CKoukuSaydonCompositionDocument(std::filesystem::path path);

		static std::filesystem::path Resolve_Path();
		// Separate authoring owner; never a Product publisher input.
		static std::filesystem::path Resolve_SequencePath();
		static bool_t Is_KnownProfile(std::string_view profileId);
		static bool_t Is_KnownGate(std::string_view gateId);
		static std::string_view Resolve_DefaultPlacementId(std::string_view gateId, std::string_view actorProfileId);
		static std::string_view Resolve_BossArchetypeId(std::string_view placementId);
		static std::string_view Resolve_ActorProfileForPlacement(std::string_view placementId);
		// Source profile 07 shares actor 05; unknown profiles return an empty view.
		static std::string_view Resolve_ActorProfileId(std::string_view sourceProfileId);
		static bool_t Parse_Text(
			std::string_view text,
			KOUKU_SAYDON_COMPOSITION_DOCUMENT& outDocument,
			std::string& outStatus);
		static bool_t Load_ImmutableActionReferences(
			KOUKU_SAYDON_ACTION_REFERENCE_SET& outReferences,
			std::string& outStatus);
		static bool_t Validate(
			const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
			const KOUKU_SAYDON_ACTION_REFERENCE_SET& references,
			std::string& outStatus);
		static std::string Serialize(
			const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document);

		bool_t Reload(std::string& outStatus);
		bool_t Reload_FromPath(
			const std::filesystem::path& path,
			std::string& outStatus);
		/* Candidate keeps the currently admitted revision. Save increments it,
		   performs source compare-and-swap, atomically replaces the
		   file, and only then commits the exact reopened document as LastGood. */
		bool_t Save_Atomic(
			const KOUKU_SAYDON_COMPOSITION_DOCUMENT& candidate,
			std::string& outStatus);

		[[nodiscard]] bool_t Has_LastGood() const noexcept {
			return m_bHasLastGood;
		}
		[[nodiscard]] bool_t Is_Fresh() const noexcept { return m_bFresh; }
		[[nodiscard]] std::uint64_t Get_Generation() const noexcept {
			return m_iGeneration;
		}
		[[nodiscard]] const std::filesystem::path& Get_Path() const noexcept {
			return m_Path;
		}
		[[nodiscard]] const KOUKU_SAYDON_COMPOSITION_DOCUMENT& Get_LastGood() const {
			return m_LastGood;
		}
		[[nodiscard]] const KOUKU_SAYDON_ACTION_REFERENCE_SET& Get_References() const {
			return m_References;
		}
		[[nodiscard]] const std::string& Get_Status() const noexcept {
			return m_strStatus;
		}

	private:
		std::filesystem::path m_Path;
		KOUKU_SAYDON_COMPOSITION_DOCUMENT m_LastGood;
		KOUKU_SAYDON_ACTION_REFERENCE_SET m_References;
		std::string m_strBaselineSourceBytes;
		std::string m_strStatus;
		std::uint64_t m_iGeneration = 0u;
		bool_t m_bHasLastGood = false;
		bool_t m_bFresh = false;
	};
}

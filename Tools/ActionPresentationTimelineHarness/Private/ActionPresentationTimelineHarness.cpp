#include "ActionPresentationTimeline.h"
#include "EncounterPatternReference.h"
#include "MonsterPresentationContract.h"
#include "ValtanCinematicCameraController.h"
#include "ValtanCinematicCameraDocument.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <span>
#include <string>

using Client::ACTION_PRESENTATION_CLIP_TIMING;
using Client::ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE;
using Client::ACTION_PRESENTATION_CUE_PREVIEW_TIMING;
using Client::ACTION_PRESENTATION_SAMPLE;
using Client::CActionPresentationTimeline;

namespace
{
	constexpr float EPSILON = 0.00001f;

	bool NearlyEqual(const float Left, const float Right)
	{
		return std::fabs(Left - Right) <= EPSILON;
	}
	bool NearlyEqualPosition(const float3_t& Left, const float3_t& Right);

	bool Require(const bool Condition, const char* pMessage)
	{
		if (Condition)
			return true;
		std::cerr << "ActionPresentationTimelineHarness: "
			<< pMessage << '\n';
		return false;
	}

	bool VerifyAdjacentExplicitSourceWindows()
	{
		const std::array<ACTION_PRESENTATION_CLIP_TIMING, 2u> Clips{
			ACTION_PRESENTATION_CLIP_TIMING{
				0.6f, 300u, 1.f, false, 0.f },
			ACTION_PRESENTATION_CLIP_TIMING{
				0.6f, 300u, 1.f, false, 0.3f }
		};

		float fSourceDuration = 0.f;
		float fWallDuration = 0.f;
		if (!Require(CActionPresentationTimeline::Resolve_ClipDuration(
				Clips[0], fSourceDuration, fWallDuration),
			"first explicit source window did not resolve") ||
			!Require(NearlyEqual(fSourceDuration, 0.3f) &&
				NearlyEqual(fWallDuration, 0.3f),
			"first explicit source window duration changed") ||
			!Require(CActionPresentationTimeline::Resolve_ClipDuration(
				Clips[1], fSourceDuration, fWallDuration),
			"second explicit source window did not resolve") ||
			!Require(NearlyEqual(fSourceDuration, 0.3f) &&
				NearlyEqual(fWallDuration, 0.3f),
			"second explicit source window duration changed"))
		{
			return false;
		}

		ACTION_PRESENTATION_SAMPLE Sample;
		if (!Require(CActionPresentationTimeline::Resolve_Sample(
				std::span<const ACTION_PRESENTATION_CLIP_TIMING>(Clips),
				0.3f, Sample),
			"shared wall-time boundary did not resolve") ||
			!Require(1u == Sample.iClipIndex &&
				NearlyEqual(Sample.fClipSourceTimeSeconds, 0.3f),
			"shared wall-time boundary did not select the second slice"))
		{
			return false;
		}

		float fCueWallOffset = 0.f;
		if (!Require(CActionPresentationTimeline::Resolve_CueWallOffset(
				std::span<const ACTION_PRESENTATION_CLIP_TIMING>(Clips),
				0u, 0.299f, 0u, fCueWallOffset),
			"cue inside the first explicit slice was rejected") ||
			!Require(NearlyEqual(fCueWallOffset, 0.299f),
			"first-slice cue wall offset changed") ||
			!Require(!CActionPresentationTimeline::Resolve_CueWallOffset(
				std::span<const ACTION_PRESENTATION_CLIP_TIMING>(Clips),
				0u, 0.3f, 0u, fCueWallOffset),
			"first explicit slice claimed its half-open end") ||
			!Require(CActionPresentationTimeline::Resolve_CueEndWallOffset(
				std::span<const ACTION_PRESENTATION_CLIP_TIMING>(Clips),
				0u, 0.3f, 0u, fCueWallOffset),
			"finite cue end rejected its explicit slice boundary") ||
			!Require(NearlyEqual(fCueWallOffset, 0.3f),
			"finite cue-end wall offset changed") ||
			!Require(!CActionPresentationTimeline::Resolve_CueEndWallOffset(
				std::span<const ACTION_PRESENTATION_CLIP_TIMING>(Clips),
				0u, 0.302f, 0u, fCueWallOffset),
			"finite cue end escaped its explicit source slice") ||
			!Require(CActionPresentationTimeline::Resolve_CueWallOffset(
				std::span<const ACTION_PRESENTATION_CLIP_TIMING>(Clips),
				1u, 0.3f, 0u, fCueWallOffset),
			"second explicit slice rejected its inclusive start") ||
			!Require(NearlyEqual(fCueWallOffset, 0.3f),
			"second-slice boundary cue wall offset changed") ||
			!Require(!CActionPresentationTimeline::Resolve_CueWallOffset(
				std::span<const ACTION_PRESENTATION_CLIP_TIMING>(Clips),
				1u, 0.6f, 0u, fCueWallOffset),
			"second explicit slice claimed its half-open end"))
		{
			return false;
		}

		return true;
	}

	bool VerifyLegacyNaturalEndCompatibility()
	{
		const std::array<ACTION_PRESENTATION_CLIP_TIMING, 1u> Clips{
			ACTION_PRESENTATION_CLIP_TIMING{
				0.6f, 0u, 1.f, false, 0.f }
		};
		float fCueWallOffset = 0.f;
		return Require(CActionPresentationTimeline::Resolve_CueWallOffset(
				std::span<const ACTION_PRESENTATION_CLIP_TIMING>(Clips),
				0u, 0.6f, 0u, fCueWallOffset),
			"legacy natural clip no longer accepts a cue at model end") &&
			Require(NearlyEqual(fCueWallOffset, 0.6f),
				"legacy natural-end cue wall offset changed");
	}

	bool VerifyClipOccurrenceTransitions()
	{
		constexpr uint32_t Animation = 42u;
		constexpr uint32_t OtherAnimation = 43u;
		const std::size_t InvalidOccurrence =
			(std::numeric_limits<std::size_t>::max)();
		return Require(
			!CActionPresentationTimeline::Requires_ClipOccurrenceTransition(
				0u, 0u, Animation, Animation),
			"unchanged occurrence and animation requested a transition") &&
			Require(
				CActionPresentationTimeline::Requires_ClipOccurrenceTransition(
					0u, 1u, Animation, Animation),
				"repeated model clip did not transition at its next occurrence") &&
			Require(
				CActionPresentationTimeline::Requires_ClipOccurrenceTransition(
					0u, 0u, Animation, OtherAnimation),
				"changed model animation did not request a transition") &&
			Require(
				CActionPresentationTimeline::Requires_ClipOccurrenceTransition(
					InvalidOccurrence, 0u, Animation, Animation),
				"uninitialized occurrence did not request its first transition");
	}

	bool VerifyCompletedAnimationClockRelease()
	{
		return Require(
			CActionPresentationTimeline::Should_ReleaseCompletedAnimationClock(
				true, false, true, false, true, 0.3333f, 0.3333f),
			"final explicit non-loop clip did not release its completed clock") &&
			Require(
				CActionPresentationTimeline::Should_ReleaseCompletedAnimationClock(
					true, false, false, true, true, 0.3333f, 0.3333f),
				"non-final authored end-pose hold kept ownership of a stopped animation clock") &&
			Require(
				!CActionPresentationTimeline::Should_ReleaseCompletedAnimationClock(
					true, false, false, false, true, 0.3333f, 0.3333f),
				"ordinary non-final clip released without an authored hold") &&
			Require(
				!CActionPresentationTimeline::Should_ReleaseCompletedAnimationClock(
					true, true, false, true, true, 0.3333f, 0.3333f),
				"looping clip released its animation clock") &&
			Require(
				!CActionPresentationTimeline::Should_ReleaseCompletedAnimationClock(
					true, false, false, true, true, 0.2f, 0.3333f),
				"incomplete authored hold released its animation clock") &&
			Require(
				!CActionPresentationTimeline::Should_ReleaseCompletedAnimationClock(
					true, false, false, true, false, 0.3333f, 0.3333f),
				"actively playing authored hold released its animation clock") &&
			Require(
				!CActionPresentationTimeline::Should_ReleaseCompletedAnimationClock(
					false, false, true, true, true, 0.3333f, 0.3333f),
				"implicit clip policy released a completed animation clock");
	}

	bool VerifyProductPreviewClock()
	{
		ACTION_PRESENTATION_CUE_PREVIEW_TIMING Timing;
		Timing.fClipSourceStartSeconds = 0.3f;
		Timing.fPlayRate = 2.f;
		Timing.fCueSourceStartSeconds = 0.4f;
		Timing.fCueSourceEndSeconds = 0.5f;
		Timing.bHasCueSourceEnd = true;

		ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE Sample;
		if (!Require(CActionPresentationTimeline::Resolve_CuePreviewSample(
				Timing, 0.075f, Sample),
			"trimmed Product preview clock did not resolve") ||
			!Require(NearlyEqual(Sample.fCueWallStartSeconds, 0.05f),
				"Product cue wall start ignored sourceStart/playRate") ||
			!Require(NearlyEqual(Sample.fCueWallEndSeconds, 0.1f),
				"Product cue wall end ignored sourceStart/playRate") ||
			!Require(NearlyEqual(Sample.fEffectSampleSeconds, 0.05f),
				"Product effect sample clock ignored playRate") ||
			!Require(Sample.bVisible,
				"Product cue was hidden inside its wall-time window"))
		{
			return false;
		}

		if (!Require(CActionPresentationTimeline::Resolve_CuePreviewSample(
				Timing, 0.1f, Sample),
			"Product preview end boundary did not resolve") ||
			!Require(!Sample.bVisible,
				"Product cue claimed its half-open wall-time end"))
		{
			return false;
		}

		return true;
	}

	bool VerifyNaturalProductPreviewDurationFloor()
	{
		const ACTION_PRESENTATION_CLIP_TIMING Willowrend{
			2.767f, 0u, 1.f, false, 0.f };
		float fSourceDuration = 0.f;
		float fClipWallDuration = 0.f;
		if (!Require(CActionPresentationTimeline::Resolve_ClipDuration(
				Willowrend, fSourceDuration, fClipWallDuration),
			"natural Product clip duration floor did not resolve") ||
			!Require(NearlyEqual(fSourceDuration, 2.767f) &&
				NearlyEqual(fClipWallDuration, 2.767f),
			"natural Product clip duration floor lost the full source clip"))
		{
			return false;
		}

		const float fShortOccurrencePreview = 1.3f;
		const float fPreviewDuration = (std::max)(
			fShortOccurrencePreview, fClipWallDuration);
		return Require(NearlyEqual(fPreviewDuration, 2.767f),
			"short Product occurrence truncated the full animation preview");
	}

	bool VerifyMonsterActionOccurrenceProjection()
	{
		using Client::CMonsterPresentationContract;
		using Client::MONSTER_PRESENTATION_ACTION_KIND;
		using Client::MONSTER_PRESENTATION_ACTION_STATE;
		using LostArk::Shared::WORLD_ENTITY_ACTION;

		MONSTER_PRESENTATION_ACTION_STATE state{};
		const auto idle = CMonsterPresentationContract::Project(
			WORLD_ENTITY_ACTION::IDLE, 10u, state);
		const auto idleAgain = CMonsterPresentationContract::Project(
			WORLD_ENTITY_ACTION::IDLE, 10u, state);
		const auto chase = CMonsterPresentationContract::Project(
			WORLD_ENTITY_ACTION::CHASE, 20u, state);
		const auto windup = CMonsterPresentationContract::Project(
			WORLD_ENTITY_ACTION::PATTERN_WINDUP, 30u, state);
		const auto active = CMonsterPresentationContract::Project(
			WORLD_ENTITY_ACTION::PATTERN_ACTIVE, 31u, state);
		const auto recovery = CMonsterPresentationContract::Project(
			WORLD_ENTITY_ACTION::PATTERN_RECOVERY, 32u, state);
		const auto nextWindup = CMonsterPresentationContract::Project(
			WORLD_ENTITY_ACTION::PATTERN_WINDUP, 40u, state);
		const auto dead = CMonsterPresentationContract::Project(
			WORLD_ENTITY_ACTION::DEAD, 50u, state);

		MONSTER_PRESENTATION_ACTION_STATE lateJoinState{};
		const auto lateActive = CMonsterPresentationContract::Project(
			WORLD_ENTITY_ACTION::PATTERN_ACTIVE, 70u, lateJoinState);
		const auto lateRecovery = CMonsterPresentationContract::Project(
			WORLD_ENTITY_ACTION::PATTERN_RECOVERY, 71u, lateJoinState);
		const std::size_t firstAttack =
			CMonsterPresentationContract::Select_AttackPresentation(
				77u, 30u, 3u);
		const std::size_t repeatedAttack =
			CMonsterPresentationContract::Select_AttackPresentation(
				77u, 30u, 3u);
		const std::size_t nextAttack =
			CMonsterPresentationContract::Select_AttackPresentation(
				77u, 40u, 3u);

		return Require(idle.shouldRestartClip && idle.isLoop &&
			MONSTER_PRESENTATION_ACTION_KIND::IDLE == idle.eKind,
			"monster idle edge did not start its loop") &&
			Require(!idleAgain.shouldRestartClip,
				"unchanged monster idle snapshot restarted its clip") &&
			Require(chase.shouldRestartClip && chase.isLoop &&
				MONSTER_PRESENTATION_ACTION_KIND::CHASE == chase.eKind,
				"monster chase edge did not start its loop") &&
			Require(windup.shouldRestartClip && !windup.isLoop &&
				30u == windup.iOccurrenceStartTick,
				"monster windup did not start an attack occurrence") &&
			Require(!active.shouldRestartClip &&
				30u == active.iOccurrenceStartTick,
				"monster ACTIVE restarted or replaced the windup occurrence") &&
			Require(!recovery.shouldRestartClip &&
				30u == recovery.iOccurrenceStartTick,
				"monster RECOVERY restarted or replaced the attack occurrence") &&
			Require(nextWindup.shouldRestartClip &&
				40u == nextWindup.iOccurrenceStartTick,
				"next monster windup did not restart the same attack clip") &&
			Require(dead.shouldRestartClip && !dead.isLoop &&
				MONSTER_PRESENTATION_ACTION_KIND::DEAD == dead.eKind,
				"monster dead edge did not start its one-shot clip") &&
			Require(lateActive.shouldRestartClip &&
				70u == lateActive.iOccurrenceStartTick &&
				!lateRecovery.shouldRestartClip &&
				70u == lateRecovery.iOccurrenceStartTick,
				"late-join ACTIVE did not start exactly one attack occurrence") &&
			Require(firstAttack < 3u && firstAttack == repeatedAttack &&
				firstAttack != nextAttack &&
				0u == CMonsterPresentationContract::Select_AttackPresentation(
					77u, 30u, 0u),
				"monster attack presentation selection was not bounded and deterministic");
	}

	bool NearlyEqualPosition(
		const float3_t& Left,
		const float3_t& Right)
	{
		return NearlyEqual(Left.x, Right.x) &&
			NearlyEqual(Left.y, Right.y) &&
			NearlyEqual(Left.z, Right.z);
	}

	const Client::VALTAN_CINEMATIC_CAMERA_CUE* FindCueById(
		const Client::CValtanCinematicCameraDocument& Document,
		const std::string_view CueId)
	{
		const auto& Cues = Document.Get_Cues();
		const auto Found = std::find_if(
			Cues.begin(), Cues.end(),
			[CueId](const Client::VALTAN_CINEMATIC_CAMERA_CUE& Cue)
			{ return Cue.strCueId == CueId; });
		return Cues.end() == Found ? nullptr : &*Found;
	}

	bool VerifyBoundedCameraTransitionSampler()
	{
		const Client::VALTAN_CINEMATIC_CAMERA_POSE From{
			float3_t(0.f, 2.f, -8.f), float3_t(0.f, 1.f, 0.f), 40.f };
		const Client::VALTAN_CINEMATIC_CAMERA_POSE To{
			float3_t(10.f, 6.f, 4.f), float3_t(4.f, 2.f, 1.f), 60.f };
		Client::VALTAN_CINEMATIC_CAMERA_POSE Sample{};
		if (!Require(
			Client::CValtanCinematicCameraController::Sample_BoundedTransition(
				From, To, 400u, 0.f, Sample) &&
			NearlyEqualPosition(Sample.vEye, From.vEye) &&
			NearlyEqualPosition(Sample.vLookAt, From.vLookAt) &&
			NearlyEqual(Sample.fFovYDegrees, From.fFovYDegrees),
			"bounded camera transition did not preserve its outgoing pose") ||
			!Require(
			Client::CValtanCinematicCameraController::Sample_BoundedTransition(
				From, To, 400u, 0.2f, Sample) &&
			NearlyEqualPosition(Sample.vEye, float3_t(5.f, 4.f, -2.f)) &&
			NearlyEqualPosition(Sample.vLookAt, float3_t(2.f, 1.5f, 0.5f)) &&
			NearlyEqual(Sample.fFovYDegrees, 50.f),
			"bounded camera transition midpoint changed") ||
			!Require(
			Client::CValtanCinematicCameraController::Sample_BoundedTransition(
				From, To, 400u, 0.4f, Sample) &&
			NearlyEqualPosition(Sample.vEye, To.vEye) &&
			NearlyEqualPosition(Sample.vLookAt, To.vLookAt) &&
			NearlyEqual(Sample.fFovYDegrees, To.fFovYDegrees),
			"bounded camera transition did not converge to its target") ||
			!Require(
			!Client::CValtanCinematicCameraController::Sample_BoundedTransition(
				From, To,
				Client::VALTAN_CINEMATIC_CAMERA_CUE::MAX_TRANSITION_IN_MS + 1u,
				0.f, Sample),
			"over-bound camera transition duration was admitted"))
		{
			return false;
		}
		return true;
	}

	bool EqualCameraCue(
		const Client::VALTAN_CINEMATIC_CAMERA_CUE& Left,
		const Client::VALTAN_CINEMATIC_CAMERA_CUE& Right)
	{
		if (Left.strCueId != Right.strCueId ||
			Left.strPatternId != Right.strPatternId ||
			Left.strStageId != Right.strStageId ||
			Left.strStageActionId != Right.strStageActionId ||
			Left.iStageIndex != Right.iStageIndex ||
			Left.iDurationMs != Right.iDurationMs ||
			Left.iTransitionInMs != Right.iTransitionInMs ||
			Left.iTransitionOutMs != Right.iTransitionOutMs ||
			Left.eInterpolation != Right.eInterpolation ||
			Left.eEasing != Right.eEasing ||
			Left.eTrackingMode != Right.eTrackingMode ||
			!NearlyEqualPosition(Left.vTrackingOrigin, Right.vTrackingOrigin) ||
			!NearlyEqual(Left.fShakeAmplitude, Right.fShakeAmplitude) ||
			Left.iShakeDurationMs != Right.iShakeDurationMs ||
			Left.Keyframes.size() != Right.Keyframes.size())
		{
			return false;
		}
		for (size_t Index = 0u; Index < Left.Keyframes.size(); ++Index)
		{
			const auto& A = Left.Keyframes[Index];
			const auto& B = Right.Keyframes[Index];
			if (A.strSceneId != B.strSceneId ||
				A.iTimeMs != B.iTimeMs ||
				!NearlyEqualPosition(A.vEye, B.vEye) ||
				!NearlyEqualPosition(A.vLookAt, B.vLookAt) ||
				!NearlyEqual(A.fFovYDegrees, B.fFovYDegrees))
			{
				return false;
			}
		}
		return true;
	}

	bool VerifyValtanCinematicTracking(
		const std::filesystem::path& RepositoryRoot)
	{
		const std::filesystem::path EncounterPath = RepositoryRoot / L"Data" /
			L"Encounters" / L"Valtan" / L"ValtanEncounter.json";
		const std::filesystem::path CameraPath = RepositoryRoot / L"Data" /
			L"Encounters" / L"Valtan" / L"ValtanCinematicCamera.json";
		Client::CEncounterPatternReference Encounter;
		Client::CValtanCinematicCameraDocument Document;
		std::string Status;
		if (!Encounter.Load(EncounterPath, Status))
		{
			std::cerr << "ActionPresentationTimelineHarness: Valtan encounter "
				"reference did not load: " << Status << '\n';
			return false;
		}
		if (!Document.Load(CameraPath, Encounter, Status))
		{
			std::cerr << "ActionPresentationTimelineHarness: Valtan cinematic "
				"camera document did not load: " << Status << '\n';
			return false;
		}

		std::string Serialized;
		Client::CValtanCinematicCameraDocument RoundTripped;
		if (!Require(Document.Serialize_Text(Serialized, Status),
				"camera document did not serialize") ||
			!Require(Client::CValtanCinematicCameraDocument::Parse_Text(
				Serialized, Encounter, RoundTripped, Status),
				"serialized camera document did not strict-reparse") ||
			!Require(Document.Get_EncounterId() == RoundTripped.Get_EncounterId() &&
				Document.Get_Cues().size() == RoundTripped.Get_Cues().size() &&
				Document.Has_DeathCue() == RoundTripped.Has_DeathCue(),
				"camera v6 root/cue/death shape changed on roundtrip"))
		{
			return false;
		}
		for (size_t Index = 0u; Index < Document.Get_Cues().size(); ++Index)
		{
			if (!Require(EqualCameraCue(
				Document.Get_Cues()[Index], RoundTripped.Get_Cues()[Index]),
				"camera cue semantics changed on roundtrip"))
			{
				return false;
			}
		}
		if (Document.Has_DeathCue() &&
			!Require(EqualCameraCue(
				*Document.Find_DeathCue(), *RoundTripped.Find_DeathCue()),
				"camera death cue semantics changed on roundtrip"))
		{
			return false;
		}
		std::string ReSerialized;
		if (!Require(RoundTripped.Serialize_Text(ReSerialized, Status) &&
			Serialized == ReSerialized,
			"camera canonical serialization is not idempotent"))
		{
			return false;
		}

		/* v6 adds stable scene IDs and product path interpolation. Any older
		   header and the retired skyCues payload must both fail before replacing
		   a ready target. */
		std::string LegacyV4 = Serialized;
		const size_t VersionOffset = LegacyV4.find("\"formatVersion\": 6");
		if (!Require(std::string::npos != VersionOffset,
				"camera v6 serialization omitted its version") )
		{
			return false;
		}
		LegacyV4.replace(VersionOffset,
			std::string("\"formatVersion\": 6").size(),
			"\"formatVersion\": 5");
		const size_t ReadyCueCount = RoundTripped.Get_Cues().size();
		if (!Require(!Client::CValtanCinematicCameraDocument::Parse_Text(
				LegacyV4, Encounter, RoundTripped, Status),
				"legacy camera formatVersion 5 was admitted") ||
			!Require(RoundTripped.Is_Ready() &&
				RoundTripped.Get_Cues().size() == ReadyCueCount,
				"rejected legacy camera document replaced the ready target"))
		{
			return false;
		}

		std::string FloatingVersion = Serialized;
		const size_t FloatingVersionOffset =
			FloatingVersion.find("\"formatVersion\": 6");
		FloatingVersion.replace(
			FloatingVersionOffset, std::string("\"formatVersion\": 6").size(),
			"\"formatVersion\": 6.0");
		if (!Require(!Client::CValtanCinematicCameraDocument::Parse_Text(
				FloatingVersion, Encounter, RoundTripped, Status),
				"floating-point token was admitted for an integer camera field") ||
			!Require(RoundTripped.Is_Ready() &&
				RoundTripped.Get_Cues().size() == ReadyCueCount,
				"rejected floating camera integer replaced the ready target"))
		{
			return false;
		}

		std::string NullDeathCue = Serialized;
		const size_t NullDeathOffset = NullDeathCue.find("  \"deathCue\":");
		const size_t NullDeathColon = NullDeathCue.find(':', NullDeathOffset);
		const size_t NullDeathRootClose = NullDeathCue.rfind("\n}");
		if (!Require(std::string::npos != NullDeathOffset &&
				std::string::npos != NullDeathColon &&
				std::string::npos != NullDeathRootClose &&
				NullDeathColon < NullDeathRootClose,
				"camera v6 serialization cannot stage a null deathCue test"))
		{
			return false;
		}
		NullDeathCue.replace(
			NullDeathColon + 1u,
			NullDeathRootClose - (NullDeathColon + 1u), " null");
		if (!Require(!Client::CValtanCinematicCameraDocument::Parse_Text(
				NullDeathCue, Encounter, RoundTripped, Status),
				"null deathCue was admitted instead of an object") ||
			!Require(RoundTripped.Is_Ready() &&
				RoundTripped.Get_Cues().size() == ReadyCueCount,
				"rejected null deathCue replaced the ready camera target"))
		{
			return false;
		}

		std::string RetiredSkyPayload = Serialized;
		const size_t DeathOffset = RetiredSkyPayload.find("  \"deathCue\":");
		if (!Require(std::string::npos != DeathOffset,
				"camera v6 serialization omitted deathCue"))
		{
			return false;
		}
		RetiredSkyPayload.insert(DeathOffset, "  \"skyCues\": [],\n");
		if (!Require(!Client::CValtanCinematicCameraDocument::Parse_Text(
				RetiredSkyPayload, Encounter, RoundTripped, Status),
				"retired skyCues payload was admitted by camera v6") ||
			!Require(RoundTripped.Is_Ready() &&
				RoundTripped.Get_Cues().size() == ReadyCueCount,
				"rejected sky payload replaced the ready camera target"))
		{
			return false;
		}

		std::vector<Client::VALTAN_CINEMATIC_CAMERA_CUE> InvalidIdDraft =
			Document.Get_Cues();
		InvalidIdDraft.front().strCueId = "camera cue with spaces";
		Client::CValtanCinematicCameraDocument RejectedDraft;
		std::string RejectedText;
		if (!Require(!Document.Stage_CameraDraft(
			InvalidIdDraft,
			Document.Has_DeathCue(),
			Document.Has_DeathCue() ? *Document.Find_DeathCue() :
				Client::VALTAN_CINEMATIC_CAMERA_CUE{},
			Encounter, RejectedDraft, RejectedText, Status),
			"unstable camera cue ID was admitted by authoring validation") ||
			!Require(Document.Is_Ready() && !Document.Get_Cues().empty(),
				"rejected camera draft replaced the loaded document"))
		{
			return false;
		}
		std::vector<Client::VALTAN_CINEMATIC_CAMERA_CUE>
			DuplicateSceneDraft = Document.Get_Cues();
		DuplicateSceneDraft[1u].Keyframes.front().strSceneId =
			DuplicateSceneDraft.front().Keyframes.front().strSceneId;
		if (!Require(!Document.Stage_CameraDraft(
			DuplicateSceneDraft,
			Document.Has_DeathCue(),
			Document.Has_DeathCue() ? *Document.Find_DeathCue() :
				Client::VALTAN_CINEMATIC_CAMERA_CUE{},
			Encounter, RejectedDraft, RejectedText, Status),
			"duplicate stable camera scene ID was admitted") ||
			!Require(Document.Is_Ready() && !Document.Get_Cues().empty(),
				"rejected duplicate scene draft replaced the loaded document"))
		{
			return false;
		}
		Client::VALTAN_CINEMATIC_CAMERA_CUE DuplicateDeathCue =
			*Document.Find_DeathCue();
		DuplicateDeathCue.strCueId = Document.Get_Cues().front().strCueId;
		if (!Require(!Document.Stage_CameraDraft(
			Document.Get_Cues(), true, DuplicateDeathCue,
			Encounter, RejectedDraft, RejectedText, Status),
			"death cue ID duplicated a normal camera cue ID") ||
			!Require(Document.Is_Ready() && !Document.Get_Cues().empty(),
				"rejected duplicate death cue replaced the loaded document"))
		{
			return false;
		}
		std::vector<Client::VALTAN_CINEMATIC_CAMERA_CUE>
			InvalidTransitionDraft = Document.Get_Cues();
		const auto InvalidTransition = std::find_if(
			InvalidTransitionDraft.begin(), InvalidTransitionDraft.end(),
			[](const Client::VALTAN_CINEMATIC_CAMERA_CUE& Cue)
			{
				return Cue.strCueId ==
					"camera.valtan.arena-break-109.recovery";
			});
		if (!Require(InvalidTransitionDraft.end() != InvalidTransition,
				"109 recovery cue is missing from transition validation") )
		{
			return false;
		}
		InvalidTransition->iTransitionInMs = InvalidTransition->iDurationMs + 1u;
		if (!Require(!Document.Stage_CameraDraft(
			InvalidTransitionDraft,
			Document.Has_DeathCue(),
			Document.Has_DeathCue() ? *Document.Find_DeathCue() :
				Client::VALTAN_CINEMATIC_CAMERA_CUE{},
			Encounter, RejectedDraft, RejectedText, Status),
			"camera entry transition longer than its cue was admitted") ||
			!Require(Document.Is_Ready() && !Document.Get_Cues().empty(),
				"rejected transition draft replaced the loaded document"))
		{
			return false;
		}
		std::vector<Client::VALTAN_CINEMATIC_CAMERA_CUE>
			InvalidExitTransitionDraft = Document.Get_Cues();
		const auto InvalidExitTransition = std::find_if(
			InvalidExitTransitionDraft.begin(), InvalidExitTransitionDraft.end(),
			[](const Client::VALTAN_CINEMATIC_CAMERA_CUE& Cue)
			{
				return Cue.strCueId ==
					"camera.valtan.arena-break-109.recovery";
			});
		if (!Require(InvalidExitTransitionDraft.end() != InvalidExitTransition,
				"109 recovery cue is missing from exit transition validation"))
		{
			return false;
		}
		InvalidExitTransition->iTransitionOutMs =
			Client::VALTAN_CINEMATIC_CAMERA_CUE::MAX_TRANSITION_IN_MS + 1u;
		if (!Require(!Document.Stage_CameraDraft(
			InvalidExitTransitionDraft,
			Document.Has_DeathCue(),
			Document.Has_DeathCue() ? *Document.Find_DeathCue() :
				Client::VALTAN_CINEMATIC_CAMERA_CUE{},
			Encounter, RejectedDraft, RejectedText, Status),
			"over-bound camera exit transition was admitted") ||
			!Require(Document.Is_Ready() && !Document.Get_Cues().empty(),
				"rejected exit transition draft replaced the loaded document"))
		{
			return false;
		}

		const std::set<std::string> ExpectedCameraStages = {
			"TAKEOFF", "YELLOW_ZONE"
		};
		std::set<std::string> CameraStages;
		bool AllCameraCuesTrackBossXZ = true;
		for (const Client::VALTAN_CINEMATIC_CAMERA_CUE& Cue :
			Document.Get_Cues())
		{
			if ("VALTAN_FOUR_PILLARS_105" != Cue.strPatternId)
				continue;
			CameraStages.insert(Cue.strStageId);
			AllCameraCuesTrackBossXZ = AllCameraCuesTrackBossXZ &&
				Client::VALTAN_CINEMATIC_TRACKING_MODE::BOSS_XZ ==
				Cue.eTrackingMode;
		}
		if (!Require(ExpectedCameraStages == CameraStages,
				"100-bar camera stages are not exactly TAKEOFF,YELLOW_ZONE") ||
			!Require(AllCameraCuesTrackBossXZ,
				"a 100-bar camera stage is not boss-XZ tracked"))
		{
			return false;
		}

		const Client::VALTAN_CINEMATIC_CAMERA_CUE* TrackingCue = FindCueById(
			Document, "camera.valtan.four-pillars-105.takeoff");
		const Client::VALTAN_CINEMATIC_CAMERA_CUE* WorldCue = FindCueById(
			Document, "camera.valtan.arena-break-109.takeoff");
		const Client::VALTAN_CINEMATIC_CAMERA_CUE* SplineCue = FindCueById(
			Document, "camera.valtan.arena-break-109.drop");
		const Client::VALTAN_CINEMATIC_CAMERA_CUE* BossFacingCue = FindCueById(
			Document, "camera.valtan.arena-break-109.wide-reveal");
		const Client::VALTAN_CINEMATIC_CAMERA_CUE* PlayerBossCue = FindCueById(
			Document, "camera.valtan.arena-break-109.recovery");
		const Client::VALTAN_CINEMATIC_CAMERA_CUE* PizzaLandingCue = FindCueById(
			Document, "camera.valtan.six-pizza-106.landing");
		const Client::VALTAN_CINEMATIC_CAMERA_CUE* EntranceEstablishCue =
			FindCueById(Document, "camera.valtan.entrance.establish");
		const Client::VALTAN_CINEMATIC_CAMERA_CUE* ClearCue =
			Document.Find_DeathCue();
		if (!Require(nullptr != TrackingCue && nullptr != WorldCue &&
				nullptr != SplineCue &&
				nullptr != BossFacingCue && nullptr != PlayerBossCue &&
				nullptr != PizzaLandingCue && nullptr != EntranceEstablishCue &&
				nullptr != ClearCue,
				"required Valtan camera cues are missing") ||
			!Require(
				Client::VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM ==
					EntranceEstablishCue->eInterpolation &&
				Client::VALTAN_CINEMATIC_CAMERA_EASING::LINEAR ==
					EntranceEstablishCue->eEasing,
				"entrance camera still stops its velocity at every keyframe") ||
			!Require(Client::VALTAN_CINEMATIC_TRACKING_MODE::BOSS_XZ ==
					TrackingCue->eTrackingMode,
				"100-bar camera cue is not boss-XZ tracked") ||
			!Require(Client::VALTAN_CINEMATIC_TRACKING_MODE::WORLD ==
					WorldCue->eTrackingMode,
				"legacy 109 camera cue stopped being world-space") ||
			!Require(Client::VALTAN_CINEMATIC_TRACKING_MODE::BOSS_FACING ==
					BossFacingCue->eTrackingMode &&
				BossFacingCue->strPatternId == "VALTAN_ARENA_BREAK_109" &&
				BossFacingCue->strStageId == "WIDE_REVEAL",
				"109 wide reveal is not an exact BOSS_FACING cue") ||
			!Require(Client::VALTAN_CINEMATIC_TRACKING_MODE::PLAYER_BOSS_FRAME ==
					PlayerBossCue->eTrackingMode &&
				PlayerBossCue->strPatternId == "VALTAN_ARENA_BREAK_109" &&
				PlayerBossCue->strStageId == "RECOVERY" &&
				400u == PlayerBossCue->iTransitionInMs &&
				400u == PlayerBossCue->iTransitionOutMs,
				"109 recovery is not an exact PLAYER_BOSS_FRAME cue") ||
			!Require(Client::VALTAN_CINEMATIC_TRACKING_MODE::PLAYER_BOSS_FRAME ==
					PizzaLandingCue->eTrackingMode &&
				PizzaLandingCue->strPatternId == "VALTAN_SIX_PIZZA_106" &&
				PizzaLandingCue->strStageId == "STEP_03" &&
				1200u == PizzaLandingCue->iDurationMs &&
				0u == PizzaLandingCue->iTransitionInMs &&
				0u == PizzaLandingCue->iTransitionOutMs,
				"pizza landing is not an exact PLAYER_BOSS_FRAME cue"))
		{
			return false;
		}

		const std::vector<uint32_t> ExpectedClearTimes = {
			0u, 1200u, 2400u, 3266u, 3267u, 4700u, 6100u,
			7266u, 7267u, 8800u, 10300u, 11800u, 13967u
		};
		std::vector<uint32_t> ClearTimes;
		ClearTimes.reserve(ClearCue->Keyframes.size());
		for (const Client::VALTAN_CINEMATIC_CAMERA_KEYFRAME& Keyframe :
			ClearCue->Keyframes)
		{
			ClearTimes.push_back(Keyframe.iTimeMs);
		}
		Client::VALTAN_CINEMATIC_CAMERA_POSE FirstCutBefore{};
		Client::VALTAN_CINEMATIC_CAMERA_POSE FirstCutAfter{};
		Client::VALTAN_CINEMATIC_CAMERA_POSE SecondCutBefore{};
		Client::VALTAN_CINEMATIC_CAMERA_POSE SecondCutAfter{};
		if (!Require(ClearCue->strCueId == "camera.valtan.clear.wide" &&
				13967u == ClearCue->iDurationMs &&
				Client::VALTAN_CINEMATIC_CAMERA_INTERPOLATION::LINEAR ==
					ClearCue->eInterpolation &&
				Client::VALTAN_CINEMATIC_CAMERA_EASING::LINEAR ==
					ClearCue->eEasing &&
				ExpectedClearTimes == ClearTimes,
				"clear camera is not the exact authored 13.967-second sequence") ||
			!Require(Client::CValtanCinematicCameraController::Sample_Cue(
					*ClearCue, 3.266f, FirstCutBefore) &&
				Client::CValtanCinematicCameraController::Sample_Cue(
					*ClearCue, 3.267f, FirstCutAfter) &&
				Client::CValtanCinematicCameraController::Sample_Cue(
					*ClearCue, 7.266f, SecondCutBefore) &&
				Client::CValtanCinematicCameraController::Sample_Cue(
					*ClearCue, 7.267f, SecondCutAfter),
				"clear camera hard-cut samples were rejected"))
		{
			return false;
		}
		const auto EyeDistanceSquared = [](
			const float3_t& Left, const float3_t& Right)
		{
			const f32_t X = Right.x - Left.x;
			const f32_t Y = Right.y - Left.y;
			const f32_t Z = Right.z - Left.z;
			return X * X + Y * Y + Z * Z;
		};
		if (!Require(EyeDistanceSquared(
				FirstCutBefore.vEye, FirstCutAfter.vEye) > 100.f &&
			EyeDistanceSquared(
				SecondCutBefore.vEye, SecondCutAfter.vEye) > 100.f,
			"clear camera hard-cut boundary was smoothed or collapsed"))
		{
			return false;
		}

		Client::VALTAN_CINEMATIC_CAMERA_CUE CatmullCue = *SplineCue;
		CatmullCue.eInterpolation =
			Client::VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM;
		Client::VALTAN_CINEMATIC_CAMERA_POSE CatmullStart{};
		Client::VALTAN_CINEMATIC_CAMERA_POSE CatmullMiddle{};
		Client::VALTAN_CINEMATIC_CAMERA_POSE CatmullEnd{};
		if (!Require(CatmullCue.Keyframes.size() >= 3u &&
			Client::CValtanCinematicCameraController::Sample_Cue(
				CatmullCue, 0.f, CatmullStart) &&
			Client::CValtanCinematicCameraController::Sample_Cue(
				CatmullCue, 0.215f, CatmullMiddle) &&
			Client::CValtanCinematicCameraController::Sample_Cue(
				CatmullCue,
				static_cast<f32_t>(CatmullCue.iDurationMs) * 0.001f,
				CatmullEnd),
			"Catmull-Rom product sampler rejected a valid camera curve") ||
			!Require(NearlyEqualPosition(
				CatmullStart.vEye, CatmullCue.Keyframes.front().vEye) &&
				NearlyEqualPosition(
					CatmullStart.vLookAt,
					CatmullCue.Keyframes.front().vLookAt) &&
				NearlyEqualPosition(
					CatmullEnd.vEye, CatmullCue.Keyframes.back().vEye) &&
				NearlyEqualPosition(
					CatmullEnd.vLookAt,
					CatmullCue.Keyframes.back().vLookAt) &&
				std::isfinite(CatmullMiddle.vEye.x) &&
				std::isfinite(CatmullMiddle.vLookAt.z),
				"Catmull-Rom camera endpoints or middle sample drifted"))
		{
			return false;
		}

		Client::VALTAN_CINEMATIC_CAMERA_CUE CrossingSpline{};
		CrossingSpline.iDurationMs = 3000u;
		CrossingSpline.eInterpolation =
			Client::VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM;
		CrossingSpline.eEasing =
			Client::VALTAN_CINEMATIC_CAMERA_EASING::LINEAR;
		CrossingSpline.Keyframes = {
			{ "camera.test.scene.0", 0u,
				float3_t(0.f, 0.f, 0.f), float3_t(0.f, 0.f, 1.f), 60.f },
			{ "camera.test.scene.1", 1000u,
				float3_t(0.f, 0.f, 0.f), float3_t(0.f, 0.f, 1.f), 60.f },
			{ "camera.test.scene.2", 2000u,
				float3_t(0.f, 0.f, 0.f), float3_t(0.f, 0.f, 1.f), 60.f },
			{ "camera.test.scene.3", 3000u,
				float3_t(0.f, 0.f, 0.f), float3_t(0.f, 0.f, 17.f), 60.f }
		};
		Client::VALTAN_CINEMATIC_CAMERA_POSE CrossingPose{};
		if (!Require(
			Client::CValtanCinematicCameraController::Sample_Cue(
				CrossingSpline, 1.5f, CrossingPose) &&
			NearlyEqual(CrossingPose.vLookAt.z, 1.f),
			"invalid Catmull Eye/LookAt crossing did not fall back to its linear segment"))
		{
			return false;
		}

		const auto BuildInput = [](
			const Client::VALTAN_CINEMATIC_CAMERA_CUE& Cue,
			const float3_t& BossPosition,
			const uint32_t ServerTick)
		{
			Client::VALTAN_CINEMATIC_CAMERA_INPUT Input{};
			Input.isValid = true;
			Input.iNetEntityId = 91u;
			Input.iServerTick = ServerTick;
			Input.strPatternId = Cue.strPatternId;
			Input.strStageId = Cue.strStageId;
			Input.strStageActionId = Cue.strStageActionId;
			Input.iPatternSequence = 7u;
			Input.iStageIndex = Cue.iStageIndex;
			Input.iActionStartTick = ServerTick;
			Input.vBossPosition = BossPosition;
			Input.fBossYawDegrees = 0.f;
			Input.hasLocalPlayerPosition = true;
			Input.vLocalPlayerPosition = float3_t(
				BossPosition.x, BossPosition.y, BossPosition.z - 8.f);
			return Input;
		};
		const float3_t Origin = TrackingCue->vTrackingOrigin;
		const float3_t ShiftedBoss(
			Origin.x + 10.f, Origin.y + 46.f, Origin.z - 7.f);
		Client::CValtanCinematicCameraController BaseController;
		Client::CValtanCinematicCameraController ShiftedController;
		Client::VALTAN_CINEMATIC_CAMERA_POSE BasePose{};
		Client::VALTAN_CINEMATIC_CAMERA_POSE ShiftedPose{};
		const Client::VALTAN_CINEMATIC_CAMERA_INPUT BaseInput =
			BuildInput(*TrackingCue, Origin, 300u);
		const Client::VALTAN_CINEMATIC_CAMERA_INPUT ShiftedInput =
			BuildInput(*TrackingCue, ShiftedBoss, 300u);
		/* Product playback advances on every render frame.  A delayed 0.5-second
		   Server snapshot may correct the clock rate, but must never snap the
		   camera backwards to that quantized authoritative sample. */
		Client::CValtanCinematicCameraController ContinuousClockController;
		Client::VALTAN_CINEMATIC_CAMERA_INPUT ContinuousClockInput =
			BuildInput(*EntranceEstablishCue,
				EntranceEstablishCue->vTrackingOrigin, 1000u);
		Client::VALTAN_CINEMATIC_CAMERA_POSE ContinuousClockPose{};
		if (!Require(ContinuousClockController.Initialize(
				&Document, Encounter.Get_FixedTickHz()) &&
			ContinuousClockController.Update(
				ContinuousClockInput, 0.f, ContinuousClockPose),
				"entrance continuous render clock did not initialize"))
		{
			return false;
		}
		for (uint32_t Frame = 0u; Frame < 40u; ++Frame)
		{
			if (!ContinuousClockController.Update(
				ContinuousClockInput, 0.016f, ContinuousClockPose))
			{
				return false;
			}
		}
		const f32_t BeforeDelayedSnapshot =
			ContinuousClockController.Get_ElapsedSeconds();
		ContinuousClockInput.iServerTick += Encounter.Get_FixedTickHz() / 2u;
		if (!Require(ContinuousClockController.Update(
				ContinuousClockInput, 0.016f, ContinuousClockPose) &&
			ContinuousClockController.Get_ElapsedSeconds() >
				BeforeDelayedSnapshot,
				"a delayed Server snapshot quantized the entrance camera backwards"))
		{
			return false;
		}
		if (!Require(BaseController.Initialize(
				&Document, Encounter.Get_FixedTickHz()) &&
			ShiftedController.Initialize(&Document, Encounter.Get_FixedTickHz()),
				"camera tracking controllers did not initialize") ||
			!Require(BaseController.Update(BaseInput, 0.f, BasePose) &&
				ShiftedController.Update(ShiftedInput, 0.f, ShiftedPose),
				"100-bar tracked camera pose did not resolve") ||
			!Require(NearlyEqual(ShiftedPose.vEye.x - BasePose.vEye.x, 10.f) &&
				NearlyEqual(ShiftedPose.vEye.y, BasePose.vEye.y) &&
				NearlyEqual(ShiftedPose.vEye.z - BasePose.vEye.z, -7.f) &&
				NearlyEqual(ShiftedPose.vLookAt.x - BasePose.vLookAt.x, 10.f) &&
				NearlyEqual(ShiftedPose.vLookAt.y, BasePose.vLookAt.y) &&
				NearlyEqual(ShiftedPose.vLookAt.z - BasePose.vLookAt.z, -7.f),
				"boss-XZ camera tracking changed Y or lost horizontal displacement"))
		{
			return false;
		}
		Client::VALTAN_CINEMATIC_CAMERA_POSE ToolSample{};
		if (!Require(Client::CValtanCinematicCameraController::Sample_Cue(
				*TrackingCue, 0.f, ToolSample),
				"public product camera sampler rejected the tool clock") ||
			!Require(NearlyEqualPosition(ToolSample.vEye, BasePose.vEye) &&
				NearlyEqualPosition(ToolSample.vLookAt, BasePose.vLookAt) &&
				NearlyEqual(ToolSample.fFovYDegrees, BasePose.fFovYDegrees),
				"Camera Tool sampler diverged from product base pose") ||
			!Require(Client::CValtanCinematicCameraController::Apply_CueTracking(
				*TrackingCue, ShiftedInput, ToolSample),
				"Camera Tool actor-tracking seam rejected the replicated frame") ||
			!Require(NearlyEqualPosition(ToolSample.vEye, ShiftedPose.vEye) &&
				NearlyEqualPosition(ToolSample.vLookAt, ShiftedPose.vLookAt) &&
				NearlyEqual(ToolSample.fFovYDegrees, ShiftedPose.fFovYDegrees),
				"Camera Tool actor-tracking sample diverged from product playback"))
		{
			return false;
		}
		Client::VALTAN_CINEMATIC_CAMERA_POSE BossXzRoundTrip{};
		if (!Require(Client::CValtanCinematicCameraController::Sample_Cue(
				*TrackingCue, 0.f, BossXzRoundTrip) &&
			Client::CValtanCinematicCameraController::Apply_CueTracking(
				*TrackingCue, ShiftedInput, BossXzRoundTrip) &&
			Client::CValtanCinematicCameraController::Remove_CueTracking(
				*TrackingCue, ShiftedInput, BossXzRoundTrip),
			"BOSS_XZ tracking roundtrip failed") ||
			!Require(NearlyEqualPosition(
					BossXzRoundTrip.vEye,
					TrackingCue->Keyframes.front().vEye) &&
				NearlyEqualPosition(
					BossXzRoundTrip.vLookAt,
					TrackingCue->Keyframes.front().vLookAt),
				"BOSS_XZ capture inverse did not restore authored coordinates"))
		{
			return false;
		}

		Client::CValtanCinematicCameraController WorldBaseController;
		Client::CValtanCinematicCameraController WorldShiftedController;
		Client::VALTAN_CINEMATIC_CAMERA_POSE WorldBasePose{};
		Client::VALTAN_CINEMATIC_CAMERA_POSE WorldShiftedPose{};
		Client::VALTAN_CINEMATIC_CAMERA_INPUT WorldBaseInput =
			BuildInput(*WorldCue, Origin, 400u);
		Client::VALTAN_CINEMATIC_CAMERA_INPUT WorldShiftedInput =
			BuildInput(*WorldCue, ShiftedBoss, 400u);
		if (!Require(WorldBaseController.Initialize(
				&Document, Encounter.Get_FixedTickHz()) &&
			WorldShiftedController.Initialize(&Document, Encounter.Get_FixedTickHz()) &&
			WorldBaseController.Update(WorldBaseInput, 0.f, WorldBasePose) &&
			WorldShiftedController.Update(WorldShiftedInput, 0.f, WorldShiftedPose),
				"legacy world-space camera poses did not resolve") ||
			!Require(NearlyEqualPosition(WorldBasePose.vEye, WorldShiftedPose.vEye) &&
				NearlyEqualPosition(WorldBasePose.vLookAt, WorldShiftedPose.vLookAt),
				"legacy world-space camera moved with the boss"))
		{
			return false;
		}

		Client::CValtanCinematicCameraController FacingBaseController;
		Client::CValtanCinematicCameraController FacingRotatedController;
		Client::VALTAN_CINEMATIC_CAMERA_POSE FacingBasePose{};
		Client::VALTAN_CINEMATIC_CAMERA_POSE FacingRotatedPose{};
		Client::VALTAN_CINEMATIC_CAMERA_INPUT FacingBaseInput =
			BuildInput(*BossFacingCue, BossFacingCue->vTrackingOrigin, 500u);
		Client::VALTAN_CINEMATIC_CAMERA_INPUT FacingRotatedInput =
			FacingBaseInput;
		FacingRotatedInput.fBossYawDegrees = 90.f;
		if (!Require(FacingBaseController.Initialize(
				&Document, Encounter.Get_FixedTickHz()) &&
			FacingRotatedController.Initialize(
				&Document, Encounter.Get_FixedTickHz()) &&
			FacingBaseController.Update(
				FacingBaseInput, 0.f, FacingBasePose) &&
			FacingRotatedController.Update(
				FacingRotatedInput, 0.f, FacingRotatedPose),
				"boss-facing camera poses did not resolve") ||
			!Require(NearlyEqualPosition(
				FacingBasePose.vEye, BossFacingCue->Keyframes.front().vEye) &&
				NearlyEqual(
					FacingRotatedPose.vEye.x - FacingRotatedInput.vBossPosition.x,
					FacingBasePose.vEye.z - BossFacingCue->vTrackingOrigin.z) &&
				NearlyEqual(
					FacingRotatedPose.vEye.z - FacingRotatedInput.vBossPosition.z,
					-(FacingBasePose.vEye.x - BossFacingCue->vTrackingOrigin.x)),
				"boss-facing camera did not rotate authored offsets by boss yaw"))
		{
			return false;
		}
		Client::VALTAN_CINEMATIC_CAMERA_POSE BossFacingRoundTrip{};
		if (!Require(Client::CValtanCinematicCameraController::Sample_Cue(
				*BossFacingCue, 0.f, BossFacingRoundTrip) &&
			Client::CValtanCinematicCameraController::Apply_CueTracking(
				*BossFacingCue, FacingRotatedInput, BossFacingRoundTrip) &&
			Client::CValtanCinematicCameraController::Remove_CueTracking(
				*BossFacingCue, FacingRotatedInput, BossFacingRoundTrip),
			"BOSS_FACING tracking roundtrip failed") ||
			!Require(NearlyEqualPosition(
					BossFacingRoundTrip.vEye,
					BossFacingCue->Keyframes.front().vEye) &&
				NearlyEqualPosition(
					BossFacingRoundTrip.vLookAt,
					BossFacingCue->Keyframes.front().vLookAt),
				"BOSS_FACING capture inverse drifted authored coordinates"))
		{
			return false;
		}

		Client::CValtanCinematicCameraController PlayerBossController;
		Client::VALTAN_CINEMATIC_CAMERA_POSE PlayerBossPose{};
		Client::VALTAN_CINEMATIC_CAMERA_INPUT PlayerBossInput =
			BuildInput(*PlayerBossCue, float3_t(10.f, 2.f, 0.f), 600u);
		PlayerBossInput.vLocalPlayerPosition = float3_t(-10.f, 1.f, 0.f);
		if (!Require(PlayerBossController.Initialize(
				&Document, Encounter.Get_FixedTickHz()) &&
			PlayerBossController.Update(
				PlayerBossInput, 0.f, PlayerBossPose),
				"player-boss framed camera pose did not resolve") ||
			!Require(NearlyEqual(PlayerBossPose.vLookAt.x, 0.f) &&
				NearlyEqual(PlayerBossPose.vLookAt.z, 0.f) &&
				PlayerBossPose.vEye.x < PlayerBossInput.vLocalPlayerPosition.x &&
				PlayerBossPose.vEye.y > PlayerBossInput.vBossPosition.y,
				"player-boss frame did not use midpoint, separation, and height"))
		{
			return false;
		}
		Client::VALTAN_CINEMATIC_CAMERA_POSE PlayerBossRoundTrip{};
		if (!Require(Client::CValtanCinematicCameraController::Sample_Cue(
				*PlayerBossCue, 0.f, PlayerBossRoundTrip) &&
			Client::CValtanCinematicCameraController::Apply_CueTracking(
				*PlayerBossCue, PlayerBossInput, PlayerBossRoundTrip) &&
			Client::CValtanCinematicCameraController::Remove_CueTracking(
				*PlayerBossCue, PlayerBossInput, PlayerBossRoundTrip),
			"PLAYER_BOSS_FRAME tracking roundtrip failed") ||
			!Require(NearlyEqualPosition(
					PlayerBossRoundTrip.vEye,
					PlayerBossCue->Keyframes.front().vEye) &&
				NearlyEqualPosition(
					PlayerBossRoundTrip.vLookAt,
					PlayerBossCue->Keyframes.front().vLookAt),
				"PLAYER_BOSS_FRAME capture inverse drifted authored coordinates"))
		{
			return false;
		}
		Client::VALTAN_CINEMATIC_CAMERA_POSE ToolPlayerBossPose{};
		if (!Require(Client::CValtanCinematicCameraController::Sample_Cue(
				*PlayerBossCue, 0.f, ToolPlayerBossPose) &&
			Client::CValtanCinematicCameraController::Apply_CueTracking(
				*PlayerBossCue, PlayerBossInput, ToolPlayerBossPose),
				"Camera Tool player/boss tracking sample did not resolve") ||
			!Require(NearlyEqualPosition(
					ToolPlayerBossPose.vEye, PlayerBossPose.vEye) &&
				NearlyEqualPosition(
					ToolPlayerBossPose.vLookAt, PlayerBossPose.vLookAt) &&
				NearlyEqual(ToolPlayerBossPose.fFovYDegrees,
					PlayerBossPose.fFovYDegrees),
				"Camera Tool PLAYER_BOSS_FRAME diverged from product playback"))
		{
			return false;
		}

		/* The 109 recovery changes from a boss-facing world pose into the dynamic
		   player/boss frame. The first recovery frame must be the exact pose that
		   was actually output by the prior stage, then converge to the normally
		   sampled incoming pose within the authored bound. */
		const uint32_t FixedTickHz = Encounter.Get_FixedTickHz();
		const uint32_t OutgoingServerTick = 900u;
		Client::VALTAN_CINEMATIC_CAMERA_INPUT OutgoingInput = BuildInput(
			*BossFacingCue, BossFacingCue->vTrackingOrigin, OutgoingServerTick);
		OutgoingInput.iActionStartTick = OutgoingServerTick - FixedTickHz;
		OutgoingInput.fBossYawDegrees = 35.f;
		OutgoingInput.vLocalPlayerPosition = float3_t(
			BossFacingCue->vTrackingOrigin.x - 12.f,
			BossFacingCue->vTrackingOrigin.y,
			BossFacingCue->vTrackingOrigin.z - 8.f);
		Client::VALTAN_CINEMATIC_CAMERA_INPUT RecoveryStartInput = BuildInput(
			*PlayerBossCue, BossFacingCue->vTrackingOrigin,
			OutgoingServerTick + 1u);
		RecoveryStartInput.iPatternSequence = OutgoingInput.iPatternSequence;
		RecoveryStartInput.fBossYawDegrees = OutgoingInput.fBossYawDegrees;
		RecoveryStartInput.vLocalPlayerPosition =
			OutgoingInput.vLocalPlayerPosition;

		Client::CValtanCinematicCameraController TransitionController;
		Client::CValtanCinematicCameraController DirectStartController;
		Client::VALTAN_CINEMATIC_CAMERA_POSE OutgoingPose{};
		Client::VALTAN_CINEMATIC_CAMERA_POSE RecoveryStartPose{};
		Client::VALTAN_CINEMATIC_CAMERA_POSE DirectRecoveryStartPose{};
		if (!Require(TransitionController.Initialize(&Document, FixedTickHz) &&
			DirectStartController.Initialize(&Document, FixedTickHz),
				"109 transition controllers did not initialize") ||
			!Require(TransitionController.Update(
				OutgoingInput, 0.f, OutgoingPose),
				"109 outgoing boss-facing pose did not resolve") ||
			!Require(DirectStartController.Update(
				RecoveryStartInput, 0.f, DirectRecoveryStartPose),
				"109 direct recovery pose did not resolve") ||
			!Require(!NearlyEqualPosition(
				OutgoingPose.vEye, DirectRecoveryStartPose.vEye),
				"109 transition fixture does not contain a coordinate-frame jump") ||
			!Require(TransitionController.Update(
				RecoveryStartInput, 0.f, RecoveryStartPose),
				"109 recovery transition did not start") ||
			!Require(NearlyEqualPosition(RecoveryStartPose.vEye, OutgoingPose.vEye) &&
				NearlyEqualPosition(
					RecoveryStartPose.vLookAt, OutgoingPose.vLookAt) &&
				NearlyEqual(
					RecoveryStartPose.fFovYDegrees, OutgoingPose.fFovYDegrees),
				"109 recovery first frame jumped away from the outgoing pose"))
		{
			return false;
		}

		const uint32_t HalfTransitionTicks = (std::max)(1u,
			(PlayerBossCue->iTransitionInMs * FixedTickHz) / 2000u);
		Client::VALTAN_CINEMATIC_CAMERA_INPUT RecoveryMidInput =
			RecoveryStartInput;
		RecoveryMidInput.iServerTick = RecoveryMidInput.iActionStartTick +
			HalfTransitionTicks;
		Client::CValtanCinematicCameraController DirectMidController;
		Client::VALTAN_CINEMATIC_CAMERA_POSE RecoveryMidPose{};
		Client::VALTAN_CINEMATIC_CAMERA_POSE DirectRecoveryMidPose{};
		if (!Require(DirectMidController.Initialize(&Document, FixedTickHz) &&
			TransitionController.Update(RecoveryMidInput, 0.f, RecoveryMidPose) &&
			DirectMidController.Update(
				RecoveryMidInput, 0.f, DirectRecoveryMidPose),
				"109 recovery midpoint did not resolve"))
		{
			return false;
		}
		const f32_t MidAgeSeconds =
			static_cast<f32_t>(HalfTransitionTicks) /
			static_cast<f32_t>(FixedTickHz);
		const f32_t RawMidAlpha = (std::clamp)(MidAgeSeconds /
			(static_cast<f32_t>(PlayerBossCue->iTransitionInMs) * 0.001f),
			0.f, 1.f);
		const f32_t MidAlpha = RawMidAlpha * RawMidAlpha *
			(3.f - 2.f * RawMidAlpha);
		const auto BlendPosition = [](const float3_t& From, const float3_t& To,
			const f32_t Alpha)
		{
			return float3_t(
				From.x + (To.x - From.x) * Alpha,
				From.y + (To.y - From.y) * Alpha,
				From.z + (To.z - From.z) * Alpha);
		};
		const float3_t ExpectedMidEye = BlendPosition(
			OutgoingPose.vEye, DirectRecoveryMidPose.vEye, MidAlpha);
		const float3_t ExpectedMidLookAt = BlendPosition(
			OutgoingPose.vLookAt, DirectRecoveryMidPose.vLookAt, MidAlpha);
		const f32_t ExpectedMidFov = OutgoingPose.fFovYDegrees +
			(DirectRecoveryMidPose.fFovYDegrees - OutgoingPose.fFovYDegrees) *
			MidAlpha;
		if (!Require(RawMidAlpha > 0.f && RawMidAlpha < 1.f &&
			NearlyEqualPosition(RecoveryMidPose.vEye, ExpectedMidEye) &&
			NearlyEqualPosition(RecoveryMidPose.vLookAt, ExpectedMidLookAt) &&
			NearlyEqual(RecoveryMidPose.fFovYDegrees, ExpectedMidFov),
				"109 recovery midpoint did not use the bounded outgoing-pose lerp"))
		{
			return false;
		}

		const uint32_t CompleteTransitionTicks =
			(PlayerBossCue->iTransitionInMs * FixedTickHz + 999u) / 1000u;
		Client::VALTAN_CINEMATIC_CAMERA_INPUT RecoveryCompleteInput =
			RecoveryStartInput;
		RecoveryCompleteInput.iServerTick =
			RecoveryCompleteInput.iActionStartTick + CompleteTransitionTicks;
		Client::CValtanCinematicCameraController DirectCompleteController;
		Client::VALTAN_CINEMATIC_CAMERA_POSE RecoveryCompletePose{};
		Client::VALTAN_CINEMATIC_CAMERA_POSE DirectRecoveryCompletePose{};
		if (!Require(DirectCompleteController.Initialize(&Document, FixedTickHz) &&
			TransitionController.Update(
				RecoveryCompleteInput, 0.f, RecoveryCompletePose) &&
			DirectCompleteController.Update(
				RecoveryCompleteInput, 0.f, DirectRecoveryCompletePose),
				"109 recovery transition completion did not resolve") ||
			!Require(NearlyEqualPosition(
				RecoveryCompletePose.vEye, DirectRecoveryCompletePose.vEye) &&
				NearlyEqualPosition(RecoveryCompletePose.vLookAt,
					DirectRecoveryCompletePose.vLookAt) &&
				NearlyEqual(RecoveryCompletePose.fFovYDegrees,
					DirectRecoveryCompletePose.fFovYDegrees),
				"109 recovery transition did not converge within its authored bound"))
		{
			return false;
		}

		Client::VALTAN_CINEMATIC_CAMERA_INPUT RecoveryLastFrameInput =
			RecoveryCompleteInput;
		const uint32_t LastRecoveryTickOffset =
			(PlayerBossCue->iDurationMs * FixedTickHz - 1u) / 1000u;
		RecoveryLastFrameInput.iServerTick =
			RecoveryLastFrameInput.iActionStartTick + LastRecoveryTickOffset;
		Client::VALTAN_CINEMATIC_CAMERA_POSE RecoveryLastPose{};
		if (!Require(TransitionController.Update(
			RecoveryLastFrameInput, 0.f, RecoveryLastPose),
			"109 recovery last in-bound pose did not resolve"))
		{
			return false;
		}
		Client::VALTAN_CINEMATIC_CAMERA_INPUT RecoveryEndedInput =
			RecoveryLastFrameInput;
		const uint32_t RecoveryEndTickOffset =
			(PlayerBossCue->iDurationMs * FixedTickHz + 999u) / 1000u;
		RecoveryEndedInput.iServerTick =
			RecoveryEndedInput.iActionStartTick + RecoveryEndTickOffset;
		Client::VALTAN_CINEMATIC_CAMERA_POSE IgnoredEndedPose{};
		if (!Require(!TransitionController.Update(
			RecoveryEndedInput, 0.f, IgnoredEndedPose) &&
			TransitionController.Is_ExitTransitionActive(),
			"109 recovery completion did not retain an exit handoff"))
		{
			return false;
		}

		const Client::VALTAN_CINEMATIC_CAMERA_POSE FollowPose{
			float3_t(18.f, 9.f, -6.f), float3_t(14.f, 3.f, 1.f), 60.f };
		Client::VALTAN_CINEMATIC_CAMERA_POSE ExitStartPose{};
		if (!Require(TransitionController.Update_ExitTransition(
			FollowPose, 0.f, ExitStartPose) &&
			NearlyEqualPosition(ExitStartPose.vEye, RecoveryLastPose.vEye) &&
			NearlyEqualPosition(
				ExitStartPose.vLookAt, RecoveryLastPose.vLookAt) &&
			NearlyEqual(
				ExitStartPose.fFovYDegrees, RecoveryLastPose.fFovYDegrees),
			"109 exit handoff did not begin at the actual outgoing pose"))
		{
			return false;
		}
		Client::VALTAN_CINEMATIC_CAMERA_POSE ExitQuarterPose{};
		Client::VALTAN_CINEMATIC_CAMERA_POSE ExitMidPose{};
		if (!Require(TransitionController.Update_ExitTransition(
				FollowPose, 0.1f, ExitQuarterPose) &&
			TransitionController.Update_ExitTransition(
				FollowPose, 0.1f, ExitMidPose),
			"109 exit handoff midpoint did not resolve"))
		{
			return false;
		}
		Client::VALTAN_CINEMATIC_CAMERA_POSE ExpectedExitMidPose{};
		if (!Require(
			Client::CValtanCinematicCameraController::Sample_BoundedTransition(
				RecoveryLastPose, FollowPose, PlayerBossCue->iTransitionOutMs,
				0.2f, ExpectedExitMidPose) &&
			NearlyEqualPosition(ExitMidPose.vEye, ExpectedExitMidPose.vEye) &&
			NearlyEqualPosition(
				ExitMidPose.vLookAt, ExpectedExitMidPose.vLookAt) &&
			NearlyEqual(ExitMidPose.fFovYDegrees,
				ExpectedExitMidPose.fFovYDegrees),
			"109 exit handoff midpoint left the bounded sampler"))
		{
			return false;
		}

		const Client::VALTAN_CINEMATIC_CAMERA_POSE MovedFollowPose{
			float3_t(20.f, 9.f, -4.f), float3_t(16.f, 3.f, 3.f), 60.f };
		Client::VALTAN_CINEMATIC_CAMERA_POSE ExitThreeQuarterPose{};
		Client::VALTAN_CINEMATIC_CAMERA_POSE ExitCompletePose{};
		if (!Require(TransitionController.Update_ExitTransition(
				MovedFollowPose, 0.1f, ExitThreeQuarterPose) &&
			TransitionController.Update_ExitTransition(
				MovedFollowPose, 0.1f, ExitCompletePose) &&
			!TransitionController.Is_ExitTransitionActive(),
			"109 exit handoff did not complete inside 400 ms") ||
			!Require(NearlyEqualPosition(
					ExitCompletePose.vEye, MovedFollowPose.vEye) &&
				NearlyEqualPosition(
					ExitCompletePose.vLookAt, MovedFollowPose.vLookAt) &&
				NearlyEqual(ExitCompletePose.fFovYDegrees,
					MovedFollowPose.fFovYDegrees),
				"109 exit handoff did not converge to the live follow pose"))
		{
			return false;
		}

		Client::CValtanCinematicCameraController MissingPlayerController;
		Client::VALTAN_CINEMATIC_CAMERA_POSE MissingPlayerPose{};
		PlayerBossInput.hasLocalPlayerPosition = false;
		if (!Require(MissingPlayerController.Initialize(
				&Document, Encounter.Get_FixedTickHz()) &&
			!MissingPlayerController.Update(
				PlayerBossInput, 0.f, MissingPlayerPose),
				"PLAYER_BOSS_FRAME accepted a missing local player position"))
		{
			return false;
		}

		std::ifstream Input(CameraPath, std::ios::binary);
		std::ostringstream Buffer;
		Buffer << Input.rdbuf();
		std::string InvalidText = Buffer.str();
		const std::string ValidMode = "\"trackingMode\":  \"BOSS_XZ\"";
		const size_t ModeAt = InvalidText.find(ValidMode);
		if (!Require(Input.good() || Input.eof(),
				"camera document text could not be read for rollback test") ||
			!Require(std::string::npos != ModeAt,
				"camera tracking field was not found for rollback test"))
		{
			return false;
		}
		InvalidText.replace(ModeAt, ValidMode.size(),
			"\"trackingMode\":  \"BOSS_Y\"");
		const size_t CueCountBeforeFailure = Document.Get_Cues().size();
		if (!Require(!Client::CValtanCinematicCameraDocument::Parse_Text(
				InvalidText, Encounter, Document, Status),
				"unknown cinematic tracking mode was accepted") ||
			!Require(Document.Is_Ready() &&
				CueCountBeforeFailure == Document.Get_Cues().size() &&
				nullptr != FindCueById(
					Document, "camera.valtan.four-pillars-105.takeoff"),
				"failed tracking parse replaced the committed camera document"))
		{
			return false;
		}

		return true;
	}
}

int main()
{
	if (!VerifyAdjacentExplicitSourceWindows() ||
		!VerifyLegacyNaturalEndCompatibility() ||
		!VerifyClipOccurrenceTransitions() ||
		!VerifyCompletedAnimationClockRelease() ||
		!VerifyProductPreviewClock() ||
		!VerifyNaturalProductPreviewDurationFloor() ||
		!VerifyBoundedCameraTransitionSampler() ||
		!VerifyMonsterActionOccurrenceProjection() ||
		!VerifyValtanCinematicTracking(std::filesystem::current_path()))
	{
		return 1;
	}

	std::cout << "ActionPresentationTimelineHarness: PASS\n";
	return 0;
}

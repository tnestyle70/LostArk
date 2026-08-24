#include "ActionPresentationTimeline.h"
#include "EncounterPatternReference.h"
#include "ValtanCinematicCameraController.h"
#include "ValtanCinematicCameraDocument.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
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
		if (!Require(Encounter.Load(EncounterPath, Status),
				"Valtan encounter reference did not load") ||
			!Require(Document.Load(CameraPath, Encounter, Status),
				"Valtan cinematic camera document did not load"))
		{
			return false;
		}

		const std::set<std::string> ExpectedCameraStages = {
			"TAKEOFF", "YELLOW_ZONE"
		};
		const std::set<std::string> ExpectedSkyStages = {
			"TAKEOFF", "YELLOW_ZONE", "TARGET_CONE", "RECOVERY"
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
		std::set<std::string> SkyStages;
		bool AllSkyCuesTrackBossXZ = true;
		for (const Client::VALTAN_CINEMATIC_SKY_CUE& Cue :
			Document.Get_SkyCues())
		{
			if ("VALTAN_FOUR_PILLARS_105" != Cue.strPatternId)
				continue;
			SkyStages.insert(Cue.strStageId);
			AllSkyCuesTrackBossXZ = AllSkyCuesTrackBossXZ &&
				Client::VALTAN_CINEMATIC_TRACKING_MODE::BOSS_XZ ==
				Cue.eTrackingMode;
		}
		if (!Require(ExpectedCameraStages == CameraStages,
				"100-bar camera stages are not exactly TAKEOFF,YELLOW_ZONE") ||
			!Require(AllCameraCuesTrackBossXZ,
				"a 100-bar camera stage is not boss-XZ tracked") ||
			!Require(ExpectedSkyStages == SkyStages,
				"100-bar sky stages are not exactly TAKEOFF,YELLOW_ZONE,TARGET_CONE,RECOVERY") ||
			!Require(AllSkyCuesTrackBossXZ,
				"a 100-bar sky stage is not boss-XZ tracked"))
		{
			return false;
		}

		const Client::VALTAN_CINEMATIC_CAMERA_CUE* TrackingCue = FindCueById(
			Document, "camera.valtan.four-pillars-105.takeoff");
		const Client::VALTAN_CINEMATIC_CAMERA_CUE* WorldCue = FindCueById(
			Document, "camera.valtan.arena-break-109.takeoff");
		if (!Require(nullptr != TrackingCue && nullptr != WorldCue,
				"required Valtan camera cues are missing") ||
			!Require(Client::VALTAN_CINEMATIC_TRACKING_MODE::BOSS_XZ ==
					TrackingCue->eTrackingMode,
				"100-bar camera cue is not boss-XZ tracked") ||
			!Require(Client::VALTAN_CINEMATIC_TRACKING_MODE::WORLD ==
					WorldCue->eTrackingMode,
				"legacy 109 camera cue stopped being world-space"))
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

		const Client::VALTAN_CINEMATIC_SKY_STATE BaseSky =
			BaseController.Resolve_SkyState(BaseInput, 0.f);
		const Client::VALTAN_CINEMATIC_SKY_STATE ShiftedSky =
			ShiftedController.Resolve_SkyState(ShiftedInput, 0.f);
		if (!Require(BaseSky.isActive && ShiftedSky.isActive,
				"100-bar tracked sky state did not resolve") ||
			!Require(NearlyEqual(ShiftedSky.vAnchor.x - BaseSky.vAnchor.x, 10.f) &&
				NearlyEqual(ShiftedSky.vAnchor.y, BaseSky.vAnchor.y) &&
				NearlyEqual(ShiftedSky.vAnchor.z - BaseSky.vAnchor.z, -7.f),
				"boss-XZ sky anchor changed Y or lost horizontal displacement"))
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
		!VerifyProductPreviewClock() ||
		!VerifyNaturalProductPreviewDurationFloor() ||
		!VerifyValtanCinematicTracking(std::filesystem::current_path()))
	{
		return 1;
	}

	std::cout << "ActionPresentationTimelineHarness: PASS\n";
	return 0;
}

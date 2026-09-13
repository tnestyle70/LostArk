#include "imgui.h"
#include "Animation_Tool_Internal.h"
#include "ActionPresentationTimeline.h"
#include "BalanceTool.h"
#include "Character.h"
#include "Model.h"
#include "SoundCueCatalog.h"
#include <charconv>
#include <algorithm>
#include <array>
#include <cerrno>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <io.h>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <span>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>




bool_t Client::CAnimation_Tool::Render_ValtanAnimationBindingInspector(
	const shared_ptr<Engine::CModel>& pModel,
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage)
{
	if (!m_bValtanPatternAnimationBindingLoadAttempted)
	{
		m_bValtanPatternAnimationBindingLoadAttempted = true;
		(void)Load_ValtanAnimationBindingDraft(pModel);
	}

	ImGui::SeparatorText("Animation / Presentation Projection");
	ImGui::TextWrapped(
		"AUTHORING OWNER: Data/Valtan/Valtan.presentation.json");
	ImGui::TextDisabled(
		"READ-ONLY GENERATED PRODUCT: Data/Animation/Authored/Valtan/Valtan.patternbindings.json");
	ImGui::TextDisabled(
		"Pattern %s | semantic stage %s | gameplay action %s",
		Pattern.strPatternId.c_str(), Stage.strStageId.c_str(),
		Stage.strActionId.c_str());
	if (!m_strValtanPatternAnimationBindingStatus.empty())
	{
		ImGui::TextWrapped(
			"%s", m_strValtanPatternAnimationBindingStatus.c_str());
	}
	if (ImGui::SmallButton("Reload Read-only Animation Product"))
		(void)Load_ValtanAnimationBindingDraft(pModel);

	ImGui::SeparatorText("Authoring Occurrences / Projected Binding");
	if (Stage.ClipOccurrences.empty())
	{
		ImGui::TextDisabled(
			"No animation occurrence is owned by this presentation stage.");
	}
	else
	{
		for (const VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence :
			Stage.ClipOccurrences)
		{
			ImGui::PushID(Occurrence.strClipOccurrenceId.c_str());
			ImGui::TextWrapped(
				"%s | %s | source %u ms | play %u ms @ %.3fx | loop %s",
				Occurrence.strClipOccurrenceId.c_str(),
				Occurrence.strClipName.c_str(), Occurrence.iSourceStartMs,
				Occurrence.iPlayMs, Occurrence.fPlayRate,
				Occurrence.bLoop ? "true" : "false");
			ImGui::PopID();
		}
	}
	const auto ProductBinding = std::find_if(
		m_ValtanPatternAnimationBindingDraft.Bindings.begin(),
		m_ValtanPatternAnimationBindingDraft.Bindings.end(),
		[&Stage](const BOSS_PATTERN_ANIMATION_BINDING& Binding)
		{
			return Binding.strActionId == Stage.strActionId;
		});
	if (!m_bValtanPatternAnimationBindingReady ||
		m_ValtanPatternAnimationBindingDraft.Bindings.end() == ProductBinding)
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.35f, 0.25f, 1.f),
			"Projected Product binding is missing or rejected for this action.");
	}
	else
	{
		ImGui::TextDisabled(
			"Projected Product parity: %zu clip(s), playback %s",
			ProductBinding->Clips.size(),
			ProductBinding->bSuppressAnimation ? "NONE" : "CLIP_SEQUENCE");
	}
	ImGui::TextWrapped(
		"Sequence rows are read-only until a typed presentation-source adapter stages them. Save & Apply validates the data and reloads the admitted animation result.");
	return false;

}

void Client::CAnimation_Tool::Render_ValtanCounterWindowInspector(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& SavedStage)
{
	if ("WINDUP" != SavedStage.strStageKind)
		return;

	ImGui::SeparatorText("Counter / Groggy Server Edge");
	if (nullptr == m_pBalanceTool)
	{
		ImGui::TextDisabled(
			"Typed Counter window draft is unavailable: Balance Tool owner is missing.");
		return;
	}

	CBalanceTool::VALTAN_COUNTER_WINDOW_EDIT Counter{};
	std::string CounterStatus;
	if (!m_pBalanceTool->Get_ValtanCounterWindowDraft(
		Pattern.strPatternId, SavedStage.strStageId,
		Counter, CounterStatus))
	{
		ImGui::TextDisabled("Counter draft unavailable: %s", CounterStatus.c_str());
		return;
	}

	std::vector<const VALTAN_STAGE_VIEW*> GroggyTargets;
	for (const VALTAN_STAGE_VIEW& Candidate : Pattern.Stages)
	{
		if ("GROGGY" == Candidate.strStageKind)
			GroggyTargets.push_back(&Candidate);
	}
	const auto SubmitCounter = [&](const CBalanceTool::VALTAN_COUNTER_WINDOW_EDIT& Edit)
	{
		if (m_pBalanceTool->Set_ValtanCounterWindowDraft(
			Pattern.strPatternId, SavedStage.strStageId,
			Edit, CounterStatus))
		{
			(void)m_pBalanceTool->Get_ValtanCounterWindowDraft(
				Pattern.strPatternId, SavedStage.strStageId,
				Counter, CounterStatus);
		}
		m_strValtanPatternMasterStatus = CounterStatus;
	};

	bool_t bEnabled = Counter.enabled;
	ImGui::BeginDisabled(!bEnabled && GroggyTargets.empty());
	if (ImGui::Checkbox("Counter Enabled", &bEnabled))
	{
		CBalanceTool::VALTAN_COUNTER_WINDOW_EDIT Changed = Counter;
		Changed.enabled = bEnabled;
		if (bEnabled && Changed.successStageId.empty() &&
			!GroggyTargets.empty())
		{
			Changed.successStageId = GroggyTargets.front()->strStageId;
			Changed.successActionId = GroggyTargets.front()->strActionId;
		}
		SubmitCounter(Changed);
	}
	ImGui::EndDisabled();
	if (GroggyTargets.empty())
	{
		ImGui::TextDisabled(
			"No same-pattern GROGGY stage/action is available; Counter cannot be enabled.");
	}

	const std::string TargetLabel = Counter.enabled ?
		Counter.successStageId + " / " + Counter.successActionId : "NONE";
	ImGui::TextDisabled("Current Counter target: %s", TargetLabel.c_str());
	ImGui::BeginDisabled(!Counter.enabled);
	if (ImGui::BeginCombo(
		"Counter success GROGGY stage/action", TargetLabel.c_str()))
	{
		for (const VALTAN_STAGE_VIEW* const pTarget : GroggyTargets)
		{
			const bool_t bSelected =
				pTarget->strStageId == Counter.successStageId &&
				pTarget->strActionId == Counter.successActionId;
			const std::string Label = pTarget->strStageId + " / " +
				pTarget->strActionId;
			if (ImGui::Selectable(Label.c_str(), bSelected) && !bSelected)
			{
				CBalanceTool::VALTAN_COUNTER_WINDOW_EDIT Changed = Counter;
				Changed.successStageId = pTarget->strStageId;
				Changed.successActionId = pTarget->strActionId;
				SubmitCounter(Changed);
			}
			if (bSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	ImGui::EndDisabled();

	const auto FlagPairState = [](
		const std::vector<VALTAN_STAGE_ACTION_VIEW>& Actions,
		const std::string_view strFlagId)
	{
		std::size_t iRows = 0u;
		std::size_t iEnterTrue = 0u;
		std::size_t iExitFalse = 0u;
		for (const VALTAN_STAGE_ACTION_VIEW& Action : Actions)
		{
			if ("SET_BOSS_FLAG" != Action.strKind ||
				strFlagId != Action.strTargetId)
			{
				continue;
			}
			++iRows;
			if ("ENTER" == Action.strTrigger && Action.fValue > 0.5f)
				++iEnterTrue;
			if ("EXIT" == Action.strTrigger && Action.fValue < 0.5f)
				++iExitFalse;
		}
		if (0u == iRows)
			return 0;
		return 2u == iRows && 1u == iEnterTrue && 1u == iExitFalse ? 1 : -1;
	};

	CBalanceTool::PATTERN_STAGE_EDIT SourceDraft{};
	std::string DraftStatus;
	const bool_t bSourceDraftReady = m_pBalanceTool->Get_ValtanStageDraft(
		Pattern.strPatternId, SavedStage.strStageId,
		SourceDraft, DraftStatus);
	const int32_t iCounterablePair = bSourceDraftReady ?
		FlagPairState(SourceDraft.actions, "boss.flag.counterable") : -1;
	if (Counter.enabled && 1 == iCounterablePair)
	{
		ImGui::TextColored(
			ImVec4(0.35f, 0.82f, 0.45f, 1.f),
			"Counterable ENTER=true / EXIT=false");
		ImGui::TextDisabled("Counterable true | COUNTER_HIT -> %s",
			Counter.successActionId.c_str());
	}
	else if (!Counter.enabled && 0 == iCounterablePair)
	{
		ImGui::TextDisabled("Counterable false | paired rows absent");
	}
	else
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.35f, 0.25f, 1.f),
			"Counterable pair is inconsistent in the current Balance Tool draft.");
	}

	if (Counter.enabled)
	{
		CBalanceTool::PATTERN_STAGE_EDIT GroggyDraft{};
		const bool_t bGroggyDraftReady =
			m_pBalanceTool->Get_ValtanStageDraft(
				Pattern.strPatternId, Counter.successStageId,
				GroggyDraft, DraftStatus);
		const int32_t iGroggyPair = bGroggyDraftReady ?
			FlagPairState(GroggyDraft.actions, "boss.flag.groggy") : -1;
		if (1 == iGroggyPair)
		{
			ImGui::TextColored(
				ImVec4(0.35f, 0.82f, 0.45f, 1.f),
				"Groggy ENTER=true / EXIT=false");
		}
		else
		{
			ImGui::TextColored(
				ImVec4(1.f, 0.35f, 0.25f, 1.f),
				"Groggy flag pair is inconsistent in the current Balance Tool draft.");
		}
	}
	for (const VALTAN_STAGE_BRANCH_VIEW& Branch : SavedStage.Branches)
	{
		if ("COUNTER_HIT" == Branch.strOutcome)
			continue;
		ImGui::TextDisabled(
			"TIMEOUT/default branch %s -> %s (saved non-Counter edge)",
			Branch.strOutcome.c_str(),
			Branch.strNextActionId.has_value() ?
				Branch.strNextActionId->c_str() : "TERMINAL");
	}
}

void Client::CAnimation_Tool::Render_ValtanStageDraftInspector(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& SavedStage)
{
	const std::string& strPatternId = Pattern.strPatternId;
	if (nullptr == m_pBalanceTool)
	{
		ImGui::TextDisabled(
			"Typed Server gameplay draft is unavailable: Balance Tool owner is missing.");
		return;
	}

	CBalanceTool::PATTERN_STAGE_EDIT Draft{};
	std::string DraftStatus;
	if (!m_pBalanceTool->Get_ValtanStageDraft(
		strPatternId, SavedStage.strStageId, Draft, DraftStatus))
	{
		ImGui::TextDisabled("Draft unavailable: %s", DraftStatus.c_str());
		return;
	}
	const auto SubmitDraft = [&]()
	{
		VALTAN_STAGE_VIEW CandidateStage = SavedStage;
		CandidateStage.strActionId = Draft.actionId;
		CandidateStage.iDurationMs = Draft.durationMs;
		CandidateStage.iAuthoringRepeatCount = Draft.animationRepeatCount;
		CandidateStage.strAnimationEndPolicy = Draft.animationEndPolicy;
		CandidateStage.bSuppressAnimation = Draft.animationSlots.empty();
		CandidateStage.ClipOccurrences.clear();
		CandidateStage.ClipOccurrences.reserve(Draft.animationSlots.size());
		for (const CBalanceTool::ANIMATION_SLOT_EDIT& Slot :
			Draft.animationSlots)
		{
			VALTAN_CLIP_OCCURRENCE_VIEW Occurrence;
			Occurrence.strClipOccurrenceId = Slot.clipOccurrenceId;
			Occurrence.strClipName = Slot.clip;
			Occurrence.strMappingBasis = Slot.mappingBasis;
			Occurrence.iSourceStartMs = Slot.sourceStartMs;
			Occurrence.iPlayMs = Slot.playMs;
			Occurrence.fPlayRate = static_cast<f32_t>(Slot.playRate);
			Occurrence.bLoop = Slot.repeatUntilStageEnd;
			CandidateStage.ClipOccurrences.push_back(std::move(Occurrence));
		}
		if (!Validate_ValtanCompositionAnimationStageMutation(
				SavedStage, CandidateStage, DraftStatus))
		{
			m_strValtanPatternMasterStatus =
				"Stage draft rejected before mutation by native Animation admission: " +
				DraftStatus;
			return;
		}
		if (!Validate_ValtanCompositionPatternSoundStageDependencies(
				Pattern, SavedStage, CandidateStage, DraftStatus))
		{
			m_strValtanPatternMasterStatus =
				"Stage draft rejected before mutation: " + DraftStatus;
			return;
		}
		(void)m_pBalanceTool->Set_ValtanStageDraft(
			strPatternId, SavedStage.strStageId, Draft, DraftStatus);
		m_strValtanPatternMasterStatus = DraftStatus;
	};

	ImGui::SeparatorText("Server Stage Clock");
	const uint32_t iOne = 1u;
	const uint32_t iStepMs = 100u;
	const uint32_t iFastStepMs = 1000u;
	ImGui::BeginDisabled(
		!Draft.durationEditable || Draft.portalRushMotionEditable);
	ImGui::SetNextItemWidth(210.f);
	if (ImGui::InputScalar(
		"Server wall / blank timeline ms",
		ImGuiDataType_U32, &Draft.durationMs,
		&iStepMs, &iFastStepMs, "%u"))
	{
		Draft.durationMs = std::clamp(Draft.durationMs, 1u, 600000u);
		SubmitDraft();
	}
	ImGui::EndDisabled();
	if (!Draft.durationEditable)
	{
		ImGui::TextDisabled(
			"Duration is read-only under this Stage's typed gameplay policy.");
	}
	else if (Draft.portalRushMotionEditable)
	{
		ImGui::TextDisabled(
			"This Stage clock is derived for all eight WARP legs by the motion controls below.");
	}
	if (Draft.durationMs != SavedStage.iDurationMs)
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.70f, 0.20f, 1.f),
			"Draft %u ms | saved source %u ms",
			Draft.durationMs, SavedStage.iDurationMs);
	}
	if ("VALTAN_HIGH_JUMP" == strPatternId &&
		"AIRBORNE" == SavedStage.strStageId)
	{
		ImGui::TextWrapped(
			"AIRBORNE duration is the boss stage/blank wall-clock. It extends the looping axe-flight animation and derives each spawned axe lifetime; the axe hit remains object-local at +1200 ms.");
		std::uint32_t iDraftAxes = 0u;
		std::uint32_t iSavedAxes = 0u;
		std::uint32_t iArenaRandomAxes = 0u;
		std::uint32_t iMaximumAxeObjects = 0u;
		std::string AxeStatus;
		if (m_pBalanceTool->Get_ValtanHighJumpAxeCountDraft(
			iDraftAxes, iSavedAxes, iArenaRandomAxes,
			iMaximumAxeObjects, AxeStatus))
		{
			const std::uint32_t iOneAxe = 1u;
			ImGui::SetNextItemWidth(210.f);
			if (ImGui::InputScalar(
				"Axes per alive player", ImGuiDataType_U32, &iDraftAxes,
				&iOneAxe, nullptr, "%u"))
			{
				iDraftAxes = std::clamp(iDraftAxes, 1u, 8u);
				(void)m_pBalanceTool->Set_ValtanHighJumpAxeCountDraft(
					iDraftAxes, AxeStatus);
				m_strValtanPatternMasterStatus = AxeStatus;
			}
			ImGui::TextDisabled(
				"PER_ALIVE_PLAYER draft %u | saved %u | arena-random %u | maximum total %u",
				iDraftAxes, iSavedAxes, iArenaRandomAxes, iMaximumAxeObjects);
		}
		else
		{
			ImGui::TextDisabled("Axe-volley draft unavailable: %s", AxeStatus.c_str());
		}
	}

	ImGui::SeparatorText("Stage Identity / Motion");
	ImGui::TextDisabled("Pattern %s | stage %s | action %s",
		strPatternId.c_str(), Draft.stageId.c_str(), Draft.actionId.c_str());
	ImGui::TextDisabled("Kind %s | sequence role %s | animation end %s",
		Draft.stageKind.c_str(), SavedStage.strSequenceRole.c_str(),
		SavedStage.strAnimationEndPolicy.c_str());
	if (SavedStage.Motion.has_value())
	{
		const VALTAN_STAGE_MOTION_VIEW& Motion = *SavedStage.Motion;
		ImGui::TextDisabled(
			"Motion %s | delay %u ms | speed %.3f m/s | distance %.3f m | corner %u | half extents [%.3f, %.3f] m",
			Motion.strKind.c_str(), Motion.iRetargetDelayMs, Motion.fSpeedMps,
			Motion.fDistance, Motion.iCornerIndex,
			Motion.HalfExtentsM[0], Motion.HalfExtentsM[1]);
	}
	else
	{
		ImGui::TextDisabled("Motion: NONE");
	}
	if (Draft.portalRushMotionEditable)
	{
		CBalanceTool::VALTAN_WARP_RUSH_EDIT RushDraft{};
		std::string RushStatus;
		if (!m_pBalanceTool->Get_ValtanWarpRushDraft(RushDraft, RushStatus))
		{
			ImGui::TextDisabled("All-leg WARP draft unavailable: %s",
				RushStatus.c_str());
		}
		const auto SubmitPortalRush = [&]()
		{
			const bool_t bFirstRushLeg = "STEP_02" == Draft.stageId;
			Draft.portalRetargetDelayMs = RushDraft.retargetDelayMs +
				(bFirstRushLeg ? 0u : RushDraft.trailingGapMs);
			Draft.portalSpeedMps = RushDraft.speedMps;
			Draft.portalDistanceM = RushDraft.distanceM;
			if (!CBalanceTool::Normalize_ValtanPortalRushDraft(
					Draft, 0u, RushStatus))
			{
				m_strValtanPatternMasterStatus = RushStatus;
				return;
			}
			RushDraft.speedMps = Draft.portalSpeedMps;
			RushDraft.distanceM = Draft.portalDistanceM;
			if (m_pBalanceTool->Set_ValtanWarpRushDraft(
					RushDraft, RushStatus))
			{
				const std::string AppliedStatus = RushStatus;
				CBalanceTool::VALTAN_WARP_RUSH_EDIT Refreshed{};
				std::string RefreshStatus;
				if (m_pBalanceTool->Get_ValtanWarpRushDraft(
						Refreshed, RefreshStatus))
				{
					RushDraft = Refreshed;
					Draft.durationMs = bFirstRushLeg ?
						Refreshed.legDurationMs - Refreshed.trailingGapMs :
						Refreshed.legDurationMs;
					Draft.portalRetargetDelayMs = Refreshed.retargetDelayMs +
						(bFirstRushLeg ? 0u : Refreshed.trailingGapMs);
					Draft.portalSpeedMps = Refreshed.speedMps;
					Draft.portalDistanceM = Refreshed.distanceM;
					Draft.hitCount = Refreshed.hitCount;
				}
				RushStatus = AppliedStatus;
			}
			m_strValtanPatternMasterStatus = RushStatus;
		};
		ImGui::TextWrapped(
			"Typed WARP rush - All 8 Legs: STEP_02 owns the first portal lead; later legs add the next-portal offset before the same lead. The Server aligns every Stage boundary to arrival and regenerates 50 ms swept-hit samples.");
		ImGui::BeginDisabled(!RushStatus.empty());
		const std::uint32_t iDelayStepMs = 50u;
		const std::uint32_t iDelayFastStepMs = 100u;
		ImGui::SetNextItemWidth(210.f);
		if (ImGui::InputScalar("Portal lead before rush ms", ImGuiDataType_U32,
			&RushDraft.retargetDelayMs, &iDelayStepMs,
			&iDelayFastStepMs, "%u"))
		{
			SubmitPortalRush();
		}
		const double fSpeedStep = 0.5;
		const double fSpeedFastStep = 5.0;
		ImGui::SetNextItemWidth(210.f);
		if (ImGui::InputDouble("Rush speed m/s", &RushDraft.speedMps,
			fSpeedStep, fSpeedFastStep, "%.3f"))
		{
			SubmitPortalRush();
		}
		const double fDistanceStep = 0.25;
		const double fDistanceFastStep = 1.0;
		ImGui::SetNextItemWidth(210.f);
		if (ImGui::InputDouble("Rush distance m", &RushDraft.distanceM,
			fDistanceStep, fDistanceFastStep, "%.3f"))
		{
			SubmitPortalRush();
		}
		ImGui::SetNextItemWidth(210.f);
		if (ImGui::InputScalar(
			"Next portal offset after arrival ms", ImGuiDataType_U32,
			&RushDraft.trailingGapMs, &iDelayStepMs,
			&iDelayFastStepMs, "%u"))
		{
			RushDraft.trailingGapMs = (std::min)(
				RushDraft.trailingGapMs, 120000u);
			SubmitPortalRush();
		}
		const std::uint32_t iFirstLegDurationMs =
			RushDraft.legDurationMs >= RushDraft.trailingGapMs ?
			RushDraft.legDurationMs - RushDraft.trailingGapMs : 0u;
		ImGui::TextDisabled(
			"First %u ms | repeat %u ms | travel %.3f ms | next portal offset %u ms | swept hits %u",
			iFirstLegDurationMs, RushDraft.legDurationMs,
			RushDraft.travelMs, RushDraft.trailingGapMs,
			RushDraft.hitCount);
		ImGui::TextColored(
			ImVec4(1.f, 0.70f, 0.20f, 1.f),
			"Distance endpoint currently bypasses navigation clamp; keep it inside the arena.");
		ImGui::EndDisabled();
	}

	ImGui::SeparatorText("Server Stage Actions");
	if (Draft.actions.empty())
		ImGui::TextDisabled("No typed stage actions.");
	for (std::size_t iAction = 0u; iAction < Draft.actions.size(); ++iAction)
	{
		VALTAN_STAGE_ACTION_VIEW& Action = Draft.actions[iAction];
		const VALTAN_STAGE_ACTION_VIEW* const pSavedAction =
			iAction < SavedStage.Actions.size() ?
				&SavedStage.Actions[iAction] : nullptr;
		ImGui::PushID(static_cast<int>(iAction));
		ImGui::TextWrapped(
			"Typed row %zu | trigger=%s | kind=%s",
			iAction + 1u, Action.strTrigger.c_str(), Action.strKind.c_str());
		ImGui::TextDisabled(
			"targetId=%s | value=%.3f | durationMs=%u",
			Action.strTargetId.c_str(), Action.fValue, Action.iDurationMs);
		if ("SET_BOSS_FLAG" == Action.strKind &&
			"boss.flag.counterable" == Action.strTargetId)
		{
			const char_t* const pCounterableState = Action.fValue > 0.5f ?
				"Counterable true" : "Counterable false";
			ImGui::TextColored(
				ImVec4(0.35f, 0.75f, 1.f, 1.f),
				"%s | exact typed %s row",
				pCounterableState, Action.strTrigger.c_str());
		}
		if ("RELEASE_GRABBED_PLAYERS" == Action.strKind)
		{
			ImGui::TextDisabled(
				"releaseMode=%s | speedMps=%.3f | durationMs=%u | yawOffsetDegrees=%.3f",
				Action.strReleaseMode.c_str(), Action.fSpeedMps,
				Action.iDurationMs, Action.fYawOffsetDegrees);
			static constexpr const char_t* RELEASE_MODES[] = {
				"HOLD", "OPPOSITE_KNOCKBACK", "ARENA_EJECTION" };
			if (ImGui::BeginCombo("Release mode", Action.strReleaseMode.c_str()))
			{
				for (const char_t* const pMode : RELEASE_MODES)
				{
					const bool_t bSelected = Action.strReleaseMode == pMode;
					if (ImGui::Selectable(pMode, bSelected) && !bSelected)
					{
						Action.strReleaseMode = pMode;
						if ("HOLD" == Action.strReleaseMode)
						{
							Action.fSpeedMps = 0.f;
							Action.iDurationMs = 0u;
							Action.fYawOffsetDegrees = 0.f;
						}
						else
						{
							Action.fSpeedMps = (std::max)(Action.fSpeedMps, 0.1f);
							Action.iDurationMs = (std::max)(Action.iDurationMs, 1u);
							if ("OPPOSITE_KNOCKBACK" == Action.strReleaseMode)
								Action.fYawOffsetDegrees = 0.f;
						}
						SubmitDraft();
					}
					if (bSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			const bool_t bLaunch = "HOLD" != Action.strReleaseMode;
			const bool_t bYawEditable =
				"ARENA_EJECTION" == Action.strReleaseMode;
			ImGui::BeginDisabled(!bLaunch);
			const float fSpeedStep = 0.1f;
			const float fSpeedFastStep = 1.f;
			ImGui::SetNextItemWidth(190.f);
			if (ImGui::InputFloat("Release speed m/s", &Action.fSpeedMps,
				fSpeedStep, fSpeedFastStep, "%.3f"))
			{
				Action.fSpeedMps = std::clamp(Action.fSpeedMps, 0.1f, 50.f);
				SubmitDraft();
			}
			ImGui::SetNextItemWidth(190.f);
			if (ImGui::InputScalar("Release duration ms", ImGuiDataType_U32,
				&Action.iDurationMs, &iOne, &iStepMs, "%u"))
			{
				Action.iDurationMs = std::clamp(Action.iDurationMs, 1u, 5000u);
				SubmitDraft();
			}
			ImGui::EndDisabled();
			const float fYawStep = 1.f;
			const float fYawFastStep = 15.f;
			ImGui::BeginDisabled(!bYawEditable);
			ImGui::SetNextItemWidth(190.f);
			if (ImGui::InputFloat("Release yaw offset deg",
				&Action.fYawOffsetDegrees, fYawStep, fYawFastStep, "%.3f"))
			{
				Action.fYawOffsetDegrees = std::clamp(
					Action.fYawOffsetDegrees, -180.f, 180.f);
				SubmitDraft();
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("Set 180 deg Draft"))
			{
				Action.fYawOffsetDegrees = 180.f;
				SubmitDraft();
			}
			ImGui::EndDisabled();
			const f32_t fSavedYaw = nullptr == pSavedAction ?
				Action.fYawOffsetDegrees : pSavedAction->fYawOffsetDegrees;
			ImGui::TextDisabled(
				"Saved yawOffsetDegrees %.3f | draft %.3f | delta %.3f",
				fSavedYaw, Action.fYawOffsetDegrees,
				Action.fYawOffsetDegrees - fSavedYaw);
			ImGui::TextDisabled(
				"Server release baseline: yawOffsetDegrees 0 ejects backward from boss facing; +180 flips that result forward. Save above, then compare Pattern Offline or Complete Play.");
		}
		else
		{
			ImGui::TextDisabled("Read-only typed action family.");
		}
		ImGui::PopID();
	}

	ImGui::SeparatorText("Branches / Counter Proxy");
	if (SavedStage.Branches.empty())
		ImGui::TextDisabled("Branches: sequential/default edge.");
	for (const VALTAN_STAGE_BRANCH_VIEW& Branch : SavedStage.Branches)
	{
		ImGui::BulletText("%s -> %s", Branch.strOutcome.c_str(),
			Branch.strNextActionId.has_value() ?
				Branch.strNextActionId->c_str() : "TERMINAL");
	}
	if (SavedStage.CounterProxy.has_value())
	{
		const VALTAN_COUNTER_PROXY_VIEW& Counter = *SavedStage.CounterProxy;
		ImGui::TextDisabled(
			"Counter proxy %s | forward %.3f | right %.3f | radius %.3f m",
			Counter.strSpace.c_str(), Counter.fForwardOffsetM,
			Counter.fRightOffsetM, Counter.fRadiusM);
	}
	else
	{
		ImGui::TextDisabled("Counter proxy: NONE");
	}

	ImGui::SeparatorText("Effect Cue Details");
	if (Draft.productCues.empty())
	{
		if ("VALTAN_WARP" == strPatternId &&
			("STEP_01" == SavedStage.strStageId ||
			 "STEP_10" == SavedStage.strStageId))
		{
			ImGui::TextDisabled(
				"No authored portal cue owner exists on this selected start/return stage; no synthetic Effect row is created.");
		}
		else
		{
			ImGui::TextDisabled("No Product Effect cues on this stage.");
		}
	}
	for (std::size_t iCue = 0u; iCue < Draft.productCues.size(); ++iCue)
	{
		VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue = Draft.productCues[iCue];
		ImGui::PushID(Cue.strOccurrenceId.c_str());
		ImGui::TextWrapped("%s", Cue.strEffectAssetId.c_str());
		ImGui::TextDisabled("Binding %s | occurrence %s",
			Cue.strBindingId.c_str(), Cue.strOccurrenceId.c_str());
		ImGui::TextDisabled("V1 alias %s | anchor %s",
			Cue.strV1EffectAssetId.empty() ? "NONE" :
				Cue.strV1EffectAssetId.c_str(), Cue.strAnchorSlotId.c_str());
		ImGui::TextDisabled("Follow %s | stop %s | repeat %s | scale %s",
			Cue.strFollowPolicy.c_str(), Cue.strStopPolicy.c_str(),
			Cue.strRepeatPolicy.c_str(), Cue.strScalePolicy.c_str());
		ImGui::TextDisabled(
			"Position [%.3f %.3f %.3f] | rotation X/Z [%.3f %.3f] | scale [%.3f %.3f %.3f]",
			Cue.LocalTransform.vPosition.x, Cue.LocalTransform.vPosition.y,
			Cue.LocalTransform.vPosition.z,
			Cue.LocalTransform.vRotationDegrees.x,
			Cue.LocalTransform.vRotationDegrees.z,
			Cue.LocalTransform.vScale.x, Cue.LocalTransform.vScale.y,
			Cue.LocalTransform.vScale.z);
		const float fYawStep = 1.f;
		const float fYawFastStep = 15.f;
		ImGui::SetNextItemWidth(190.f);
		if (ImGui::InputFloat("Local Y rotation deg",
			&Cue.LocalTransform.vRotationDegrees.y,
			fYawStep, fYawFastStep, "%.3f"))
		{
			Cue.LocalTransform.vRotationDegrees.y = std::clamp(
				Cue.LocalTransform.vRotationDegrees.y, -180.f, 180.f);
			SubmitDraft();
		}
		if ("arena.center.facing" == Cue.strAnchorSlotId)
		{
			ImGui::TextDisabled(
				"Sector final yaw = Server-selected target facing + this local Y rotation.");
		}
		else if ("arena.center.target-follow" == Cue.strAnchorSlotId)
		{
			ImGui::TextDisabled(
				"Sector yaw = current locked-target Server tick facing + this local Y rotation.");
		}
		ImGui::TextDisabled("Source %u..%s | stage clock %s +%u ms",
			Cue.iSourceStartMs,
			Cue.bHasSourceEnd ? std::to_string(Cue.iSourceEndMs).c_str() : "END",
			Cue.bUsesStageClock ? "YES" : "NO", Cue.iStageOffsetMs);
		ImGui::PopID();
	}

	ImGui::SeparatorText("Server Hit / Collider");
	ImGui::TextDisabled("Action %s | stage kind %s",
		Draft.actionId.c_str(), Draft.stageKind.c_str());
	ImGui::TextDisabled("DamageProfile (read-only): %s",
		Draft.damageProfileId.empty() ? "NONE" : Draft.damageProfileId.c_str());
	ImGui::TextDisabled("Player response: %s | attachment: %s",
		Draft.playerResponse.c_str(), Draft.attachmentSlot.c_str());
	if (!Draft.hitEditable)
	{
		ImGui::TextDisabled(
			"Collider NONE. Adding a new Server hit also requires a typed DamageProfile choice, so it remains in Balance Tool.");
	}
	else
	{

	static constexpr const char_t* HIT_SHAPES[] = {
		"CIRCLE", "RING", "CONE", "BOX", "CROSS", "SIX_DIRECTIONS" };
	if (ImGui::BeginCombo("Collider shape", Draft.hitShape.c_str()))
	{
		for (const char_t* const pShape : HIT_SHAPES)
		{
			const bool_t bSelected = Draft.hitShape == pShape;
			if (ImGui::Selectable(pShape, bSelected) && !bSelected)
			{
				const double fExtent = (std::max)({
					1.0, Draft.hitOuterRadius, Draft.hitInnerRadius,
					Draft.hitLength, Draft.hitHalfWidth });
				Draft.hitShape = pShape;
				Draft.hitOuterRadius = 0.0;
				Draft.hitInnerRadius = 0.0;
				Draft.hitAngleDegrees = 0.0;
				Draft.hitLength = 0.0;
				Draft.hitHalfWidth = 0.0;
				if ("CIRCLE" == Draft.hitShape)
					Draft.hitOuterRadius = fExtent;
				else if ("RING" == Draft.hitShape)
				{
					Draft.hitOuterRadius = fExtent;
					Draft.hitInnerRadius = (std::max)(0.1, fExtent * 0.5);
				}
				else if ("CONE" == Draft.hitShape)
				{
					Draft.hitAngleDegrees = 90.0;
					Draft.hitLength = fExtent;
				}
				else
				{
					Draft.hitLength = fExtent;
					Draft.hitHalfWidth = (std::max)(0.1, fExtent * 0.5);
				}
				SubmitDraft();
			}
			if (bSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	const auto EditGeometry = [&](const char_t* const pLabel,
		double& fValue, const double fMinimum, const double fMaximum)
	{
		const double fStep = 0.1;
		const double fFastStep = 1.0;
		ImGui::SetNextItemWidth(210.f);
		if (!ImGui::InputDouble(
			pLabel, &fValue, fStep, fFastStep, "%.3f"))
		{
			return;
		}
		fValue = std::clamp(fValue, fMinimum, fMaximum);
		SubmitDraft();
	};
	if ("CIRCLE" == Draft.hitShape || "RING" == Draft.hitShape)
	{
		const double fMinimumOuter = "RING" == Draft.hitShape ?
			Draft.hitInnerRadius + 0.001 : 0.001;
		EditGeometry("Outer radius m", Draft.hitOuterRadius,
			fMinimumOuter, 1000.0);
	}
	if ("RING" == Draft.hitShape)
	{
		EditGeometry("Inner radius m", Draft.hitInnerRadius,
			0.001, (std::max)(0.001, Draft.hitOuterRadius - 0.001));
	}
	if ("CONE" == Draft.hitShape)
	{
		EditGeometry("Angle degrees", Draft.hitAngleDegrees, 0.001, 180.0);
		EditGeometry("Length m", Draft.hitLength, 0.001, 1000.0);
	}
	if ("BOX" == Draft.hitShape || "CROSS" == Draft.hitShape ||
		"SIX_DIRECTIONS" == Draft.hitShape)
	{
		EditGeometry("Length m", Draft.hitLength, 0.001, 1000.0);
		EditGeometry("Half width m", Draft.hitHalfWidth, 0.001, 1000.0);
	}

	ImGui::SeparatorText("Server Hit Schedule");
	if (!Draft.hitOffsetsMs.empty())
	{
		ImGui::TextDisabled(
			"EXPLICIT_OFFSETS is read-only here; preserve the source contact ordering in Balance Tool.");
		for (std::size_t iOffset = 0u;
			iOffset < Draft.hitOffsetsMs.size(); ++iOffset)
		{
			ImGui::BulletText("hit %zu at stage +%u ms",
				iOffset + 1u, Draft.hitOffsetsMs[iOffset]);
		}
	}
	else
	{
		ImGui::SetNextItemWidth(180.f);
		if (ImGui::InputScalar(
			"Hit count", ImGuiDataType_U32, &Draft.hitCount,
			&iOne, nullptr, "%u"))
		{
			Draft.hitCount = std::clamp(Draft.hitCount, 1u, 64u);
			if (1u == Draft.hitCount)
				Draft.hitIntervalMs = 0u;
			else if (0u == Draft.hitIntervalMs)
				Draft.hitIntervalMs = 1u;
			SubmitDraft();
		}
		const uint32_t iMaximumDelay = Draft.durationMs - 1u;
		ImGui::SetNextItemWidth(180.f);
		if (ImGui::InputScalar(
			"First hit offset ms", ImGuiDataType_U32, &Draft.hitDelayMs,
			&iOne, &iStepMs, "%u"))
		{
			Draft.hitDelayMs = (std::min)(Draft.hitDelayMs, iMaximumDelay);
			SubmitDraft();
		}
		ImGui::BeginDisabled(1u == Draft.hitCount);
		ImGui::SetNextItemWidth(180.f);
		if (ImGui::InputScalar(
			"Hit interval ms", ImGuiDataType_U32, &Draft.hitIntervalMs,
			&iOne, &iStepMs, "%u"))
		{
			const uint32_t iRemaining = Draft.durationMs - 1u -
				(std::min)(Draft.hitDelayMs, Draft.durationMs - 1u);
			const uint32_t iMaximumInterval = Draft.hitCount > 1u ?
				iRemaining / (Draft.hitCount - 1u) : 0u;
			Draft.hitIntervalMs = std::clamp(
				Draft.hitIntervalMs, 1u, (std::max)(1u, iMaximumInterval));
			SubmitDraft();
		}
		ImGui::EndDisabled();
	}

	ImGui::SeparatorText("Server Player Reaction");
	const bool_t bCapture = "CAPTURE" == Draft.playerResponse;
	ImGui::BeginDisabled(bCapture);
	double fPushStep = 0.1;
	double fPushFastStep = 1.0;
	ImGui::SetNextItemWidth(190.f);
	if (ImGui::InputDouble(
		"Push range m", &Draft.pushRangeM,
		fPushStep, fPushFastStep, "%.3f"))
	{
		Draft.pushRangeM = std::clamp(Draft.pushRangeM, -20.0, 20.0);
		if (std::abs(Draft.pushRangeM) < 0.000001)
		{
			Draft.pushRangeM = 0.0;
			Draft.pushMs = 0u;
		}
		else if (0u == Draft.pushMs)
		{
			Draft.pushMs = 1u;
		}
		SubmitDraft();
	}
	ImGui::BeginDisabled(0.0 == Draft.pushRangeM);
	ImGui::SetNextItemWidth(190.f);
	if (ImGui::InputScalar(
		"Push duration ms", ImGuiDataType_U32, &Draft.pushMs,
		&iOne, &iStepMs, "%u"))
	{
		Draft.pushMs = std::clamp(Draft.pushMs, 1u, 600000u);
		SubmitDraft();
	}
	ImGui::EndDisabled();
	if (ImGui::Checkbox("Knockdown", &Draft.knockdown))
	{
		Draft.downMs = Draft.knockdown ?
			(std::max)(Draft.downMs, 1u) : 0u;
		SubmitDraft();
	}
	ImGui::BeginDisabled(!Draft.knockdown);
	ImGui::SetNextItemWidth(190.f);
	if (ImGui::InputScalar(
		"Down duration ms", ImGuiDataType_U32, &Draft.downMs,
		&iOne, &iStepMs, "%u"))
	{
		Draft.downMs = std::clamp(Draft.downMs, 1u, 600000u);
		SubmitDraft();
	}
	ImGui::EndDisabled();
	ImGui::EndDisabled();
	const char_t* const pDirectionPolicy = 0.0 == Draft.pushRangeM ? "NONE" :
		(Draft.pushRangeM > 0.0 ? "AWAY_FROM_HIT_SOURCE" :
			"TOWARD_HIT_SOURCE");
	const double fPushSpeedMps = 0u == Draft.pushMs ? 0.0 :
		std::abs(Draft.pushRangeM) * 1000.0 /
		static_cast<double>(Draft.pushMs);
	ImGui::TextDisabled(
		"Direction policy (Server-derived): %s | derived speed %.3f m/s",
		pDirectionPolicy, fPushSpeedMps);
	if (bCapture)
	{
		ImGui::TextDisabled(
			"CAPTURE owns attachment instead of push/knockdown; reaction values are read-only zero.");
	}
	}
}

bool_t Client::CAnimation_Tool::Render_ValtanPatternSoundInspector(
	const shared_ptr<Engine::CModel>& pModel,
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage)
{
	bool_t bReloadJoinedWorkbenchAfterSave = false;
	ImGui::SeparatorText("Pattern Sound Typed Owner");
	ImGui::TextWrapped(
		"Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json");
	ImGui::TextDisabled(
		"Selection %s / %s | action %s",
		Pattern.strPatternId.c_str(), Stage.strStageId.c_str(),
		Stage.strActionId.c_str());
	const std::unordered_map<std::string, f32_t>
		ClipSourceDurationSecondsByName =
			CollectModelClipSourceDurationSeconds(pModel);

	std::string SoundLifecycleStatus;
	const bool_t bSoundSourceCommitAdmitted =
		Can_CommitValtanCompositionPatternSoundGeneration(
			SoundLifecycleStatus);
	ImGui::BeginDisabled(!bSoundSourceCommitAdmitted);
	if (ImGui::SmallButton(
		m_bValtanPatternSoundCuesDirty ?
			"Discard / Reload Pattern Sound" : "Reload Pattern Sound"))
	{
		(void)Reload_ValtanPatternSoundCues();
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(
		!bSoundSourceCommitAdmitted ||
		!m_bValtanPatternSoundCuesReady ||
		!m_bValtanPatternSoundCuesDirty);
	if (ImGui::SmallButton("Save Pattern Sound"))
	{
		std::string SaveStatus;
		bReloadJoinedWorkbenchAfterSave =
			Save_ValtanCompositionPatternSounds(SaveStatus);
		m_strValtanPatternSoundCueStatus = std::move(SaveStatus);
	}
	ImGui::EndDisabled();
	if (!bSoundSourceCommitAdmitted && !SoundLifecycleStatus.empty())
		ImGui::TextWrapped("%s", SoundLifecycleStatus.c_str());
	if (!m_strValtanPatternSoundCueStatus.empty())
		ImGui::TextWrapped("%s", m_strValtanPatternSoundCueStatus.c_str());

	if (!m_bValtanPatternSoundCuesReady)
	{
		ImGui::TextDisabled(
			"Strict Pattern Sound authoring source is not admitted.");
		return bReloadJoinedWorkbenchAfterSave;
	}

	const std::vector<std::string> AllSoundEvents =
		CSoundCueCatalog::Collect_EventNames("Valtan");
	std::vector<const std::string*> AuthoringEvents;
	AuthoringEvents.reserve(AllSoundEvents.size());
	for (const std::string& EventName : AllSoundEvents)
	{
		if (IsValtanSoundAuthoringCandidate(EventName))
			AuthoringEvents.push_back(&EventName);
	}
	std::vector<ACTION_PRESENTATION_CLIP_TIMING> RuntimeClipTimings;
	RuntimeClipTimings.reserve(Stage.ClipOccurrences.size());
	bool_t bRuntimeClipTimingsReady = !Stage.ClipOccurrences.empty();
	for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip : Stage.ClipOccurrences)
	{
		const auto Duration =
			ClipSourceDurationSecondsByName.find(Clip.strClipName);
		if (ClipSourceDurationSecondsByName.end() == Duration)
		{
			bRuntimeClipTimingsReady = false;
			break;
		}
		ACTION_PRESENTATION_CLIP_TIMING Timing{
			Duration->second,
			Clip.iPlayMs,
			Clip.fPlayRate,
			Clip.bLoop,
			static_cast<f32_t>(Clip.iSourceStartMs) * 0.001f };
		f32_t fSourceDurationSeconds = 0.f;
		f32_t fWallDurationSeconds = 0.f;
		if (!CActionPresentationTimeline::Resolve_ClipDuration(
				Timing, fSourceDurationSeconds, fWallDurationSeconds))
		{
			bRuntimeClipTimingsReady = false;
			break;
		}
		RuntimeClipTimings.push_back(Timing);
	}

	ImGui::SeparatorText("Add Pattern Sound Row");
	const auto FindAddClip = [this, &Stage]()
	{
		return std::find_if(
			Stage.ClipOccurrences.begin(), Stage.ClipOccurrences.end(),
			[this](const VALTAN_CLIP_OCCURRENCE_VIEW& Candidate)
			{
				return Candidate.strClipOccurrenceId ==
					m_strValtanPatternSoundAddClipOccurrenceId;
			});
	};
	auto AddClip = FindAddClip();
	if (Stage.ClipOccurrences.end() == AddClip &&
		!Stage.ClipOccurrences.empty())
	{
		m_strValtanPatternSoundAddClipOccurrenceId =
			Stage.ClipOccurrences.front().strClipOccurrenceId;
		m_iValtanPatternSoundAddStartMs =
			Stage.ClipOccurrences.front().iSourceStartMs;
		m_eValtanPatternSoundAddRepeatPolicy =
			VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
		AddClip = FindAddClip();
	}
	const auto FindAddEvent = [this, &AuthoringEvents]()
	{
		return std::find_if(
			AuthoringEvents.begin(), AuthoringEvents.end(),
			[this](const std::string* const pCandidate)
			{
				return nullptr != pCandidate &&
					*pCandidate == m_strValtanPatternSoundAddEvent;
			});
	};
	auto AddEvent = FindAddEvent();
	if (AuthoringEvents.end() == AddEvent && !AuthoringEvents.empty())
	{
		m_strValtanPatternSoundAddEvent = *AuthoringEvents.front();
		AddEvent = FindAddEvent();
	}

	const char_t* const pAddClipPreview =
		Stage.ClipOccurrences.end() == AddClip ?
			"Select an admitted clip occurrence" :
			AddClip->strClipOccurrenceId.c_str();
	ImGui::SetNextItemWidth(-1.f);
	if (ImGui::BeginCombo(
		"Clip Occurrence##AddPatternSound", pAddClipPreview))
	{
		for (const VALTAN_CLIP_OCCURRENCE_VIEW& Candidate :
			Stage.ClipOccurrences)
		{
			const bool_t bSelected =
				Candidate.strClipOccurrenceId ==
					m_strValtanPatternSoundAddClipOccurrenceId;
			const std::string Label = Candidate.strClipOccurrenceId +
				" | " + Candidate.strClipName;
			if (ImGui::Selectable(Label.c_str(), bSelected) && !bSelected)
			{
				m_strValtanPatternSoundAddClipOccurrenceId =
					Candidate.strClipOccurrenceId;
				m_iValtanPatternSoundAddStartMs = Candidate.iSourceStartMs;
				m_eValtanPatternSoundAddRepeatPolicy =
					VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
			}
			if (bSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	AddClip = FindAddClip();
	const char_t* const pAddEventPreview =
		AuthoringEvents.end() == AddEvent ?
			"Select a validated Valtan event" :
			m_strValtanPatternSoundAddEvent.c_str();
	ImGui::SetNextItemWidth(-1.f);
	if (ImGui::BeginCombo(
		"Sound Event##AddPatternSound", pAddEventPreview))
	{
		for (const std::string* const pEventName : AuthoringEvents)
		{
			const bool_t bSelected =
				*pEventName == m_strValtanPatternSoundAddEvent;
			if (ImGui::Selectable(pEventName->c_str(), bSelected) && !bSelected)
				m_strValtanPatternSoundAddEvent = *pEventName;
			if (bSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	AddEvent = FindAddEvent();
	ImGui::TextDisabled(
		"Sound Bank (derived): %s",
		std::string(ValtanSoundBankForEvent(
			m_strValtanPatternSoundAddEvent)).c_str());

	std::uint32_t iAddMinimumStartMs = 0u;
	std::uint32_t iAddMaximumStartMs = 0u;
	f64_t fAddModelDurationMilliseconds = 0.0;
	f64_t fAddRemainingStageWallMilliseconds = 0.0;
	bool_t bAddRuntimeWindowReady = false;
	if (Stage.ClipOccurrences.end() != AddClip)
	{
		iAddMinimumStartMs = AddClip->iSourceStartMs;
		const std::size_t iClipIndex = static_cast<std::size_t>(
			AddClip - Stage.ClipOccurrences.begin());
		f32_t fResolvedSourceDurationSeconds = 0.f;
		f32_t fResolvedWallDurationSeconds = 0.f;
		f32_t fClipStageWallStartSeconds = 0.f;
		bAddRuntimeWindowReady = bRuntimeClipTimingsReady &&
			iClipIndex < RuntimeClipTimings.size() &&
			CActionPresentationTimeline::Resolve_ClipDuration(
				RuntimeClipTimings[iClipIndex],
				fResolvedSourceDurationSeconds,
				fResolvedWallDurationSeconds) &&
			CActionPresentationTimeline::Resolve_CueWallOffset(
				RuntimeClipTimings, iClipIndex,
				RuntimeClipTimings[iClipIndex].fSourceStartSeconds,
				0u, fClipStageWallStartSeconds);
		const f64_t fRemainingStageWallSeconds =
			bAddRuntimeWindowReady ? (std::max)(0.0,
				static_cast<f64_t>(Stage.iDurationMs) * 0.001 -
				static_cast<f64_t>(fClipStageWallStartSeconds)) : 0.0;
		const f64_t fResolvedSourceEndSeconds =
			static_cast<f64_t>(AddClip->iSourceStartMs) * 0.001 +
			static_cast<f64_t>(fResolvedSourceDurationSeconds);
		const f64_t fStageSourceEndSeconds =
			static_cast<f64_t>(AddClip->iSourceStartMs) * 0.001 +
			fRemainingStageWallSeconds *
				static_cast<f64_t>(AddClip->fPlayRate);
		const f64_t fEffectiveSourceEndMilliseconds = 1000.0 *
			(std::min)(fResolvedSourceEndSeconds, fStageSourceEndSeconds);
		const std::uint64_t iEffectiveSourceEndExclusiveMs =
			bAddRuntimeWindowReady &&
			std::isfinite(fEffectiveSourceEndMilliseconds) &&
			fEffectiveSourceEndMilliseconds > 0.0 ?
				static_cast<std::uint64_t>(
					std::ceil(fEffectiveSourceEndMilliseconds)) : 0u;
		bAddRuntimeWindowReady =
			iEffectiveSourceEndExclusiveMs > iAddMinimumStartMs;
		iAddMaximumStartMs = bAddRuntimeWindowReady ?
			static_cast<std::uint32_t>((std::min)(
				iEffectiveSourceEndExclusiveMs - 1u,
				static_cast<std::uint64_t>(
					(std::numeric_limits<std::uint32_t>::max)()))) :
			iAddMinimumStartMs;
		const auto ModelDuration =
			ClipSourceDurationSecondsByName.find(AddClip->strClipName);
		if (ClipSourceDurationSecondsByName.end() != ModelDuration)
			fAddModelDurationMilliseconds =
				static_cast<f64_t>(ModelDuration->second) * 1000.0;
		fAddRemainingStageWallMilliseconds =
			fRemainingStageWallSeconds * 1000.0;
		m_iValtanPatternSoundAddStartMs = std::clamp(
			m_iValtanPatternSoundAddStartMs,
			iAddMinimumStartMs, iAddMaximumStartMs);
		if (!AddClip->bLoop)
		{
			m_eValtanPatternSoundAddRepeatPolicy =
				VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
		}
	}
	const std::uint32_t iStepMs = 1u;
	const std::uint32_t iFastStepMs = 100u;
	ImGui::BeginDisabled(!bAddRuntimeWindowReady);
	ImGui::SetNextItemWidth(210.f);
	if (ImGui::InputScalar(
		"startMs##AddPatternSound", ImGuiDataType_U32,
		&m_iValtanPatternSoundAddStartMs,
		&iStepMs, &iFastStepMs, "%u"))
	{
		m_iValtanPatternSoundAddStartMs = std::clamp(
			m_iValtanPatternSoundAddStartMs,
			iAddMinimumStartMs, iAddMaximumStartMs);
	}
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"Runtime-equivalent source window: %u..%u ms | model end %.3f ms | stage wall remaining %.3f ms",
		iAddMinimumStartMs, iAddMaximumStartMs,
		fAddModelDurationMilliseconds,
		fAddRemainingStageWallMilliseconds);
	const char_t* const pAddRepeatLabel =
		VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
			m_eValtanPatternSoundAddRepeatPolicy ? "each_loop" : "once";
	ImGui::SetNextItemWidth(210.f);
	if (ImGui::BeginCombo(
		"Repeat Policy##AddPatternSound", pAddRepeatLabel))
	{
		const bool_t bOnce =
			VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE ==
				m_eValtanPatternSoundAddRepeatPolicy;
		if (ImGui::Selectable("once", bOnce) && !bOnce)
		{
			m_eValtanPatternSoundAddRepeatPolicy =
				VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
		}
		ImGui::BeginDisabled(
			Stage.ClipOccurrences.end() == AddClip || !AddClip->bLoop);
		const bool_t bEachLoop =
			VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
				m_eValtanPatternSoundAddRepeatPolicy;
		if (ImGui::Selectable("each_loop", bEachLoop) && !bEachLoop)
		{
			m_eValtanPatternSoundAddRepeatPolicy =
				VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP;
		}
		ImGui::EndDisabled();
		ImGui::EndCombo();
	}
	const bool_t bCanAddPatternSoundRow =
		Stage.ClipOccurrences.end() != AddClip &&
		AuthoringEvents.end() != AddEvent && bAddRuntimeWindowReady;
	ImGui::BeginDisabled(!bCanAddPatternSoundRow);
	if (ImGui::SmallButton("Add Exact Pattern Sound Row"))
	{
		VALTAN_PATTERN_SOUND_CUE_ADD_ROW Row;
		Row.strPatternId = Pattern.strPatternId;
		Row.strStageId = Stage.strStageId;
		Row.strActionId = Stage.strActionId;
		Row.strClipOccurrenceId = AddClip->strClipOccurrenceId;
		Row.strSoundEvent = m_strValtanPatternSoundAddEvent;
		Row.strSoundBank = std::string(
			ValtanSoundBankForEvent(Row.strSoundEvent));
		Row.eRepeatPolicy = m_eValtanPatternSoundAddRepeatPolicy;
		Row.iStartMs = m_iValtanPatternSoundAddStartMs;
		VALTAN_PATTERN_SOUND_CUE_ROW_ID CreatedRowId;
		std::string AddStatus;
		if (CValtanPatternSoundCueDocument::Add_AuthoringRow(
			m_ValtanPatternSoundCues, Row,
			ClipSourceDurationSecondsByName, CreatedRowId, AddStatus))
		{
			m_bValtanPatternSoundCuesDirty = true;
			++m_iValtanPatternSoundDraftGeneration;
			m_strValtanPatternSoundCueStatus =
				"UNSAVED Pattern Sound row added: " +
				CreatedRowId.strBindingId + " / " +
				CreatedRowId.strOccurrenceId + ". " + AddStatus;
		}
		else
		{
			m_strValtanPatternSoundCueStatus =
				"Pattern Sound Add Row rejected; admitted draft preserved: " +
				AddStatus;
		}
	}
	ImGui::EndDisabled();
	if (!bAddRuntimeWindowReady)
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.35f, 0.25f, 1.f),
			"Add is blocked until the selected clip has a model-backed source remainder inside the Server stage wall.");
	}

	std::size_t iSelectedRows = 0u;
	VALTAN_PATTERN_SOUND_CUE_ROW_ID PendingRemoveRowId;
	bool_t bHasPendingRemoveRow = false;
	for (VALTAN_PATTERN_SOUND_CUE& Cue : m_ValtanPatternSoundCues.Cues)
	{
		if (Cue.strPatternId != Pattern.strPatternId ||
			Cue.strStageId != Stage.strStageId)
		{
			continue;
		}
		++iSelectedRows;
		ImGui::PushID(Cue.strBindingId.c_str());
		ImGui::Separator();
		ImGui::TextDisabled("bindingId: %s", Cue.strBindingId.c_str());
		ImGui::TextDisabled("occurrenceId: %s", Cue.strOccurrenceId.c_str());
		ImGui::TextDisabled("patternId: %s", Cue.strPatternId.c_str());
		ImGui::TextDisabled("stageId: %s", Cue.strStageId.c_str());
		ImGui::TextDisabled("actionId: %s", Cue.strActionId.c_str());
		ImGui::TextDisabled(
			"clipOccurrenceId: %s", Cue.strClipOccurrenceId.c_str());

		const auto Clip = std::find_if(
			Stage.ClipOccurrences.begin(), Stage.ClipOccurrences.end(),
			[&Cue](const VALTAN_CLIP_OCCURRENCE_VIEW& Candidate)
			{
				return Candidate.strClipOccurrenceId ==
					Cue.strClipOccurrenceId;
			});
		const bool_t bClipReady = Stage.ClipOccurrences.end() != Clip;
		const bool_t bLoopClip = bClipReady && Clip->bLoop;

		ImGui::SetNextItemWidth(-1.f);
		if (ImGui::BeginCombo("Sound Event", Cue.strSoundEvent.c_str()))
		{
			for (const std::string* const pEventName : AuthoringEvents)
			{
				const bool_t bSelected = *pEventName == Cue.strSoundEvent;
				if (ImGui::Selectable(pEventName->c_str(), bSelected) &&
					!bSelected)
				{
					Cue.strSoundEvent = *pEventName;
					Cue.strSoundBank = std::string(
						ValtanSoundBankForEvent(*pEventName));
					m_bValtanPatternSoundCuesDirty = true;
					++m_iValtanPatternSoundDraftGeneration;
					m_strValtanPatternSoundCueStatus =
						"UNSAVED Pattern Sound event: " + Cue.strBindingId +
						" -> " + Cue.strSoundEvent;
				}
				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::TextDisabled(
			"Sound Bank (derived from event prefix): %s",
			Cue.strSoundBank.c_str());

		if (bClipReady)
		{
			const std::uint32_t iMinimumStartMs = Clip->iSourceStartMs;
			const std::size_t iClipIndex = static_cast<std::size_t>(
				Clip - Stage.ClipOccurrences.begin());
			f32_t fResolvedSourceDurationSeconds = 0.f;
			f32_t fResolvedWallDurationSeconds = 0.f;
			f32_t fClipStageWallStartSeconds = 0.f;
			const bool_t bRuntimeWindowReady =
				bRuntimeClipTimingsReady &&
				iClipIndex < RuntimeClipTimings.size() &&
				CActionPresentationTimeline::Resolve_ClipDuration(
					RuntimeClipTimings[iClipIndex],
					fResolvedSourceDurationSeconds,
					fResolvedWallDurationSeconds) &&
				CActionPresentationTimeline::Resolve_CueWallOffset(
					RuntimeClipTimings, iClipIndex,
					RuntimeClipTimings[iClipIndex].fSourceStartSeconds,
					0u, fClipStageWallStartSeconds);
			const f64_t fSavedStageWallSeconds =
				static_cast<f64_t>(Cue.iStageDurationMs) * 0.001;
			const f64_t fRemainingStageWallSeconds = bRuntimeWindowReady ?
				(std::max)(0.0, fSavedStageWallSeconds -
					static_cast<f64_t>(fClipStageWallStartSeconds)) : 0.0;
			const f64_t fResolvedSourceEndSeconds =
				static_cast<f64_t>(Clip->iSourceStartMs) * 0.001 +
				static_cast<f64_t>(fResolvedSourceDurationSeconds);
			const f64_t fStageSourceEndSeconds =
				static_cast<f64_t>(Clip->iSourceStartMs) * 0.001 +
				fRemainingStageWallSeconds *
					static_cast<f64_t>(Clip->fPlayRate);
			const f64_t fEffectiveSourceEndMilliseconds = 1000.0 *
				(std::min)(fResolvedSourceEndSeconds, fStageSourceEndSeconds);
			const std::uint64_t iEffectiveSourceEndExclusiveMs =
				bRuntimeWindowReady &&
				std::isfinite(fEffectiveSourceEndMilliseconds) &&
				fEffectiveSourceEndMilliseconds > 0.0 ?
				static_cast<std::uint64_t>(
					std::ceil(fEffectiveSourceEndMilliseconds)) : 0u;
			const bool_t bEditableRuntimeWindow =
				iEffectiveSourceEndExclusiveMs > iMinimumStartMs;
			const std::uint32_t iMaximumStartMs =
				bEditableRuntimeWindow ? static_cast<std::uint32_t>((std::min)(
					iEffectiveSourceEndExclusiveMs - 1u,
					static_cast<std::uint64_t>(
						(std::numeric_limits<std::uint32_t>::max)()))) :
				iMinimumStartMs;
			const std::uint32_t iStepMs = 1u;
			const std::uint32_t iFastStepMs = 100u;
			ImGui::BeginDisabled(!bEditableRuntimeWindow);
			ImGui::SetNextItemWidth(210.f);
			if (ImGui::InputScalar(
				"startMs", ImGuiDataType_U32, &Cue.iStartMs,
				&iStepMs, &iFastStepMs, "%u"))
			{
				Cue.iStartMs = std::clamp(
					Cue.iStartMs, iMinimumStartMs, iMaximumStartMs);
				m_bValtanPatternSoundCuesDirty = true;
				++m_iValtanPatternSoundDraftGeneration;
				m_strValtanPatternSoundCueStatus =
					"UNSAVED Pattern Sound startMs: " + Cue.strBindingId;
			}
			ImGui::EndDisabled();
			const auto ModelDuration =
				ClipSourceDurationSecondsByName.find(Clip->strClipName);
			const f64_t fModelDurationMilliseconds =
				ClipSourceDurationSecondsByName.end() != ModelDuration ?
					static_cast<f64_t>(ModelDuration->second) * 1000.0 : 0.0;
			ImGui::TextDisabled(
				"Runtime-equivalent source window: %u..%u ms | clip loop=%s",
				iMinimumStartMs, iMaximumStartMs,
				bLoopClip ? "true" : "false");
			ImGui::TextDisabled(
				"Model source end %.3f ms | stage wall remaining %.3f / %u ms",
				fModelDurationMilliseconds,
				fRemainingStageWallSeconds * 1000.0,
				Cue.iStageDurationMs);
			if (!bEditableRuntimeWindow)
			{
				ImGui::TextColored(
					ImVec4(1.f, 0.35f, 0.25f, 1.f),
					"No editable startMs survives the current model source window and saved Server stage wall.");
			}
		}
		else
		{
			ImGui::TextColored(
				ImVec4(1.f, 0.35f, 0.25f, 1.f),
				"The stable clip occurrence is absent from this admitted stage.");
		}

		const char_t* const pRepeatLabel =
			VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
				Cue.eRepeatPolicy ? "each_loop" : "once";
		ImGui::SetNextItemWidth(210.f);
		if (ImGui::BeginCombo("Repeat Policy", pRepeatLabel))
		{
			const bool_t bOnce = VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE ==
				Cue.eRepeatPolicy;
			if (ImGui::Selectable("once", bOnce) && !bOnce)
			{
				Cue.eRepeatPolicy = VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
				m_bValtanPatternSoundCuesDirty = true;
				++m_iValtanPatternSoundDraftGeneration;
				m_strValtanPatternSoundCueStatus =
					"UNSAVED Pattern Sound repeatPolicy: " + Cue.strBindingId;
			}
			ImGui::BeginDisabled(!bLoopClip);
			const bool_t bEachLoop =
				VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
					Cue.eRepeatPolicy;
			if (ImGui::Selectable("each_loop", bEachLoop) && !bEachLoop)
			{
				Cue.eRepeatPolicy =
					VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP;
				m_bValtanPatternSoundCuesDirty = true;
				++m_iValtanPatternSoundDraftGeneration;
				m_strValtanPatternSoundCueStatus =
					"UNSAVED Pattern Sound repeatPolicy: " + Cue.strBindingId;
			}
			ImGui::EndDisabled();
			ImGui::EndCombo();
		}

		const std::vector<std::string>& Variants =
			CSoundCueCatalog::Find_Variants("Valtan", Cue.strSoundEvent);
		for (std::size_t iVariant = 0u; iVariant < Variants.size(); ++iVariant)
		{
			ImGui::PushID(static_cast<int32_t>(iVariant));
			if (ImGui::SmallButton("Preview Selected WAV"))
				(void)Preview_ValtanSoundAsset(Variants[iVariant]);
			ImGui::SameLine();
			ImGui::TextDisabled("%s", Variants[iVariant].c_str());
			ImGui::PopID();
		}
		if (ImGui::SmallButton("Remove Exact Pattern Sound Row"))
		{
			PendingRemoveRowId.strBindingId = Cue.strBindingId;
			PendingRemoveRowId.strOccurrenceId = Cue.strOccurrenceId;
			bHasPendingRemoveRow = true;
		}
		ImGui::PopID();
	}
	if (bHasPendingRemoveRow)
	{
		std::string RemoveStatus;
		if (CValtanPatternSoundCueDocument::Remove_AuthoringRow(
			m_ValtanPatternSoundCues, PendingRemoveRowId, RemoveStatus))
		{
			m_bValtanPatternSoundCuesDirty = true;
			++m_iValtanPatternSoundDraftGeneration;
			m_strValtanPatternSoundCueStatus =
				"UNSAVED Pattern Sound row removed: " +
				PendingRemoveRowId.strBindingId + " / " +
				PendingRemoveRowId.strOccurrenceId + ". " + RemoveStatus;
		}
		else
		{
			m_strValtanPatternSoundCueStatus =
				"Pattern Sound Remove Row rejected; admitted draft preserved: " +
				RemoveStatus;
		}
	}
	if (0u == iSelectedRows)
	{
		ImGui::TextDisabled(
			"No exact pattern/stage-qualified Pattern Sound row is admitted for this selection.");
	}
	ImGui::TextDisabled(
		"Inventory is typed: Add allocates deterministic stable IDs; Remove requires the exact bindingId + occurrenceId pair. Save remains a separate CAS boundary.");
	return bReloadJoinedWorkbenchAfterSave;
}

void Client::CAnimation_Tool::Render_ValtanPresentationLanes(
	const VALTAN_PATTERN_VIEW& Pattern,
	const std::string_view strStageFilter)
{
	struct JOINED_COMBAT_OBJECT_SOUND final
	{
		VALTAN_COMBAT_OBJECT_SOUND_CUE* pCue = nullptr;
		std::string strServerEventId;
		uint32_t iEventOffsetMs = 0u;
		bool_t bPresentationEvent = false;
	};
	const std::vector<std::string> ValtanSoundEventNames =
		CSoundCueCatalog::Collect_EventNames("Valtan");

	ImGui::SeparatorText("Joined Presentation Lanes");
	ImGui::TextDisabled(
		"Server Stage -> Animation -> Effect -> Sound Asset -> Camera/Shake -> World Event -> Combat Object");
	if (!m_strValtanPatternSoundCueStatus.empty())
		ImGui::TextWrapped("Sound: %s", m_strValtanPatternSoundCueStatus.c_str());
	if (!m_strValtanPatternShakeCueStatus.empty())
		ImGui::TextWrapped("Camera/Shake: %s",
			m_strValtanPatternShakeCueStatus.c_str());
	if (!m_strValtanCombatObjectSoundCueStatus.empty())
		ImGui::TextWrapped("Server-hit Sound: %s",
			m_strValtanCombatObjectSoundCueStatus.c_str());

	for (std::size_t iStage = 0u; iStage < Pattern.Stages.size(); ++iStage)
	{
		const VALTAN_STAGE_VIEW& Stage = Pattern.Stages[iStage];
		if (!strStageFilter.empty() &&
			Stage.strStageId != strStageFilter)
		{
			continue;
		}
		std::vector<const VALTAN_PATTERN_SOUND_CUE*> SoundCues;
		if (m_bValtanPatternSoundCuesReady)
		{
			for (const VALTAN_PATTERN_SOUND_CUE& Cue :
				m_ValtanPatternSoundCues.Cues)
			{
				if (Cue.strPatternId == Pattern.strPatternId &&
					Cue.strStageId == Stage.strStageId)
				{
					SoundCues.push_back(&Cue);
				}
			}
		}
		std::vector<const VALTAN_PATTERN_SHAKE_CUE*> ShakeCues;
		if (m_bValtanPatternShakeCuesReady)
		{
			for (const VALTAN_PATTERN_SHAKE_CUE& Cue :
				m_ValtanPatternShakeCues.Cues)
			{
				if (Cue.strPatternId == Pattern.strPatternId &&
					Cue.strStageId == Stage.strStageId)
				{
					ShakeCues.push_back(&Cue);
				}
			}
		}
		std::vector<const VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW*> WorldEvents;
		for (const VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW& Event :
			Pattern.WorldEventTriggerRefs)
		{
			if (Event.strPatternId == Pattern.strPatternId &&
				Event.strStageId == Stage.strStageId)
			{
				WorldEvents.push_back(&Event);
			}
		}
		std::vector<JOINED_COMBAT_OBJECT_SOUND> CombatObjectSoundCues;
		std::size_t iMissingCombatObjectSounds = 0u;
		for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& CombatObject :
			Stage.CombatObjectEffects)
		{
			if (CombatObject.HitIds.size() != CombatObject.HitOffsetsMs.size())
			{
				iMissingCombatObjectSounds += (std::max)(
					CombatObject.HitIds.size(), CombatObject.HitOffsetsMs.size());
				continue;
			}
			for (std::size_t iHit = 0u; iHit < CombatObject.HitIds.size(); ++iHit)
			{
				VALTAN_COMBAT_OBJECT_SOUND_CUE* pJoinedCue = nullptr;
				if (m_bValtanCombatObjectSoundCuesReady)
				{
					const auto Found = std::find_if(
						m_ValtanCombatObjectSoundCues.Cues.begin(),
						m_ValtanCombatObjectSoundCues.Cues.end(),
						[&CombatObject, &iHit](
							const VALTAN_COMBAT_OBJECT_SOUND_CUE& Cue)
						{
							return Cue.strCombatObjectArchetypeId ==
									CombatObject.strCombatObjectArchetypeId &&
								Cue.strHitId == CombatObject.HitIds[iHit];
						});
					if (Found != m_ValtanCombatObjectSoundCues.Cues.end())
						pJoinedCue = &*Found;
				}
				if (nullptr == pJoinedCue)
				{
					++iMissingCombatObjectSounds;
					continue;
				}
				CombatObjectSoundCues.push_back(
					{ pJoinedCue, CombatObject.HitIds[iHit],
						CombatObject.HitOffsetsMs[iHit], false });
			}
			for (const VALTAN_COMBAT_OBJECT_PRESENTATION_EVENT_VIEW& Event :
				CombatObject.PresentationEvents)
			{
				VALTAN_COMBAT_OBJECT_SOUND_CUE* pJoinedCue = nullptr;
				if (m_bValtanCombatObjectSoundCuesReady)
				{
					const auto Found = std::find_if(
						m_ValtanCombatObjectSoundCues.Cues.begin(),
						m_ValtanCombatObjectSoundCues.Cues.end(),
						[&CombatObject, &Event](
							const VALTAN_COMBAT_OBJECT_SOUND_CUE& Cue)
						{
							return Cue.strCombatObjectArchetypeId ==
									CombatObject.strCombatObjectArchetypeId &&
								Cue.strPresentationEventId ==
									Event.strPresentationEventId;
						});
					if (Found != m_ValtanCombatObjectSoundCues.Cues.end())
						pJoinedCue = &*Found;
				}
				if (nullptr == pJoinedCue)
				{
					++iMissingCombatObjectSounds;
					continue;
				}
				CombatObjectSoundCues.push_back(
					{ pJoinedCue, Event.strPresentationEventId,
						Event.iAtMs, true });
			}
		}

		ImGui::PushID(static_cast<int32_t>(iStage));
		const std::string StageLabel = Stage.strStageId + " | " +
			std::to_string(Stage.iDurationMs) + " ms | animation " +
			std::to_string(Stage.ClipOccurrences.size()) + " | effect " +
			std::to_string(Stage.ProductCues.size() +
				Stage.CombatObjectEffects.size()) + " | sound " +
			std::to_string(SoundCues.size() + CombatObjectSoundCues.size()) +
			" | camera/shake " +
			std::to_string(Stage.CameraInvocations.size() + ShakeCues.size()) +
			" | world " + std::to_string(WorldEvents.size());

		const auto ResolveClipSourceMsToStageMs = [&Stage](
			const std::string_view strClipOccurrenceId,
			const std::uint32_t iSourceMs)
		{
			std::uint64_t iWallCursorMs = 0u;
			for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
				Stage.ClipOccurrences)
			{
				if (Clip.strClipOccurrenceId == strClipOccurrenceId)
				{
					const std::uint32_t iLocalSourceMs =
						iSourceMs > Clip.iSourceStartMs ?
							iSourceMs - Clip.iSourceStartMs : 0u;
					const f32_t fRate = Clip.fPlayRate > 0.f ?
						Clip.fPlayRate : 1.f;
					const std::uint64_t iLocalWallMs =
						static_cast<std::uint64_t>(std::llround(
							static_cast<double>(iLocalSourceMs) /
							static_cast<double>(fRate)));
					return static_cast<std::uint32_t>((std::min)(
						iWallCursorMs + iLocalWallMs,
						static_cast<std::uint64_t>(Stage.iDurationMs)));
				}
				iWallCursorMs += Clip.iAuthoringWallMs;
			}
			return 0u;
		};

		if (ImGui::BeginTable(
			"##JoinedTrackSegments", 3,
			ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
			ImGuiTableFlags_Resizable |
			ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableSetupColumn(
				"Track", ImGuiTableColumnFlags_WidthFixed, 90.f);
			ImGui::TableSetupColumn(
				"Time Axis (stage ms)", ImGuiTableColumnFlags_WidthStretch, 0.46f);
			ImGui::TableSetupColumn(
				"Stable owner row", ImGuiTableColumnFlags_WidthStretch, 0.54f);
			ImGui::TableHeadersRow();
			std::size_t iTrackRow = 0u;
			const auto TrackSegment = [&](
				const char_t* const pTrack,
				const std::uint32_t iStartMs,
				const std::uint32_t iEndMs,
				const std::string& strOwner,
				const ImU32 iColor)
			{
				ImGui::PushID(static_cast<int32_t>(iTrackRow++));
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TextUnformatted(pTrack);
				ImGui::TableSetColumnIndex(1);
				const f32_t fWidth = (std::max)(
					80.f, ImGui::GetContentRegionAvail().x);
				const f32_t fHeight = ImGui::GetTextLineHeightWithSpacing();
				const ImVec2 Position = ImGui::GetCursorScreenPos();
				ImGui::InvisibleButton(
					"##track-segment", ImVec2(fWidth, fHeight));
				ImDrawList* const pDrawList = ImGui::GetWindowDrawList();
				pDrawList->AddRectFilled(
					Position,
					ImVec2(Position.x + fWidth, Position.y + fHeight),
					IM_COL32(38, 42, 48, 255), 3.f);
				const f32_t fDuration = static_cast<f32_t>((std::max)(
					Stage.iDurationMs, 1u));
				const std::uint32_t iClampedStart = (std::min)(
					iStartMs, Stage.iDurationMs);
				const std::uint32_t iClampedEnd = (std::min)(
					(std::max)(iEndMs, iClampedStart), Stage.iDurationMs);
				const f32_t fStartX = Position.x + fWidth *
					static_cast<f32_t>(iClampedStart) / fDuration;
				const f32_t fEndX = (std::max)(
					fStartX + 3.f,
					Position.x + fWidth *
						static_cast<f32_t>(iClampedEnd) / fDuration);
				pDrawList->AddRectFilled(
					ImVec2(fStartX, Position.y + 2.f),
					ImVec2((std::min)(fEndX, Position.x + fWidth),
						Position.y + fHeight - 2.f),
					iColor, 3.f);
				const std::string Range = std::to_string(iStartMs) +
					".." + std::to_string(iEndMs) + " ms";
				pDrawList->AddText(
					ImVec2(Position.x + 4.f, Position.y + 1.f),
					IM_COL32(245, 245, 245, 255), Range.c_str());
				ImGui::TableSetColumnIndex(2);
				ImGui::TextWrapped("%s", strOwner.c_str());
				ImGui::PopID();
			};

			std::uint64_t iAnimationCursorMs = 0u;
			if (Stage.bSuppressAnimation)
			{
				TrackSegment(
					"Animation", 0u, Stage.iDurationMs,
					"explicit NONE / pose hold",
					IM_COL32(84, 132, 220, 255));
			}
			for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
				Stage.ClipOccurrences)
			{
				const std::uint32_t iStartMs =
					static_cast<std::uint32_t>((std::min)(
						iAnimationCursorMs,
						static_cast<std::uint64_t>(UINT32_MAX)));
				iAnimationCursorMs += Clip.iAuthoringWallMs;
				const std::uint32_t iEndMs =
					static_cast<std::uint32_t>((std::min)(
						iAnimationCursorMs,
						static_cast<std::uint64_t>(UINT32_MAX)));
				TrackSegment(
					"Animation", iStartMs, iEndMs,
					Clip.strClipOccurrenceId + " | " + Clip.strClipName,
					IM_COL32(84, 132, 220, 255));
			}
			for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
				Stage.ProductCues)
			{
				const std::uint32_t iStartMs = Cue.bUsesStageClock ?
					Cue.iStageOffsetMs : ResolveClipSourceMsToStageMs(
						Cue.strClipOccurrenceId, Cue.iSourceStartMs);
				std::uint32_t iEndMs = iStartMs;
				if (!Cue.bUsesStageClock && Cue.bHasSourceEnd)
				{
					iEndMs = ResolveClipSourceMsToStageMs(
						Cue.strClipOccurrenceId, Cue.iSourceEndMs);
				}
				if ("each_loop" == Cue.strRepeatPolicy)
					iEndMs = Stage.iDurationMs;
				TrackSegment(
					"Effect", iStartMs, iEndMs,
					Cue.strOccurrenceId + " | " + Cue.strEffectAssetId,
					IM_COL32(164, 101, 220, 255));
			}
			for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& CombatObject :
				Stage.CombatObjectEffects)
			{
				const std::uint32_t iObjectStartMs = (std::min)(
					CombatObject.iFirstSpawnOffsetMs, Stage.iDurationMs);
				const std::uint64_t iObjectEndMs = (std::min)(
					static_cast<std::uint64_t>(iObjectStartMs) +
						static_cast<std::uint64_t>(CombatObject.iLifetimeMs),
					static_cast<std::uint64_t>(Stage.iDurationMs));
				TrackSegment(
					"Effect", iObjectStartMs,
					static_cast<std::uint32_t>(iObjectEndMs),
					"Server combat object " +
						CombatObject.strCombatObjectArchetypeId + " | " +
						CombatObject.strEffectAssetId + " (local lifetime)",
					IM_COL32(150, 88, 205, 255));
			}
			for (const VALTAN_PATTERN_SOUND_CUE* const pCue : SoundCues)
			{
				const std::uint32_t iStartMs = ResolveClipSourceMsToStageMs(
					pCue->strClipOccurrenceId, pCue->iStartMs);
				const std::uint32_t iEndMs =
					VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
						pCue->eRepeatPolicy ? Stage.iDurationMs : iStartMs;
				TrackSegment(
					"Sound", iStartMs, iEndMs,
					pCue->strOccurrenceId + " | " + pCue->strSoundEvent,
					IM_COL32(70, 175, 118, 255));
			}
			for (const JOINED_COMBAT_OBJECT_SOUND& Joined :
				CombatObjectSoundCues)
			{
				TrackSegment(
					"Sound", Joined.iEventOffsetMs, Joined.iEventOffsetMs,
					Joined.pCue->strBindingId + " | " +
						Joined.pCue->strSoundEvent +
						(Joined.bPresentationEvent ?
							" (presentation-event local)" :
							" (server-hit local)"),
					IM_COL32(55, 150, 103, 255));
			}
			for (const VALTAN_CAMERA_INVOCATION_VIEW& Invocation :
				Stage.CameraInvocations)
			{
				TrackSegment(
					"Camera", Invocation.iStartOffsetMs,
					Invocation.iStartOffsetMs + Invocation.iDurationMs,
					Invocation.strCameraInvocationId + " | " +
						Invocation.strCameraCueId,
					IM_COL32(218, 154, 63, 255));
			}
			for (const VALTAN_PATTERN_SHAKE_CUE* const pCue : ShakeCues)
			{
				const std::uint32_t iStartMs = ResolveClipSourceMsToStageMs(
					pCue->strClipOccurrenceId, pCue->iStartMs);
				const std::uint64_t iShakeEndMs =
					static_cast<std::uint64_t>(iStartMs) +
					static_cast<std::uint64_t>(std::llround(
						static_cast<double>(pCue->Spec.fDurationSeconds) *
						1000.0));
				TrackSegment(
					"Camera", iStartMs,
					VALTAN_PATTERN_SHAKE_REPEAT_POLICY::EACH_LOOP ==
						pCue->eRepeatPolicy ? Stage.iDurationMs :
							static_cast<std::uint32_t>((std::min)(
								iShakeEndMs,
								static_cast<std::uint64_t>(UINT32_MAX))),
					pCue->strOccurrenceId + " | camera shake",
					IM_COL32(202, 130, 52, 255));
			}
			for (const VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW* const pEvent :
				WorldEvents)
			{
				const bool_t bExitEdge =
					std::string_view::npos != pEvent->strTriggerKind.find("EXIT") ||
					std::string_view::npos != pEvent->strTriggerKind.find("END");
				const std::uint32_t iEventMs = bExitEdge ?
					Stage.iDurationMs : 0u;
				TrackSegment(
					"World", iEventMs, iEventMs,
					pEvent->strPatternId + "/" + pEvent->strStageId +
						" | " + pEvent->strTriggerKind,
					IM_COL32(198, 84, 91, 255));
			}
			ImGui::EndTable();
		}
		if (ImGui::TreeNodeEx(
			StageLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::TextDisabled("Server action: %s", Stage.strActionId.c_str());

			ImGui::SeparatorText("Animation");
			if (Stage.bSuppressAnimation)
				ImGui::TextDisabled("NONE (hold current boss pose)");
			for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
				Stage.ClipOccurrences)
			{
				ImGui::BulletText(
					"%s | %s | source +%u ms | play %u ms | wall %u ms",
					Clip.strClipOccurrenceId.c_str(), Clip.strClipName.c_str(),
					Clip.iSourceStartMs, Clip.iPlayMs, Clip.iAuthoringWallMs);
			}

			ImGui::SeparatorText("Effect");
			if (Stage.ProductCues.empty() && Stage.CombatObjectEffects.empty())
				ImGui::TextDisabled("No Product Effect cue on this stage.");
			for (std::size_t iCue = 0u; iCue < Stage.ProductCues.size(); ++iCue)
			{
				const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue =
					Stage.ProductCues[iCue];
				ImGui::PushID(static_cast<int32_t>(iCue));
				ImGui::BulletText(
					"%s | %s | anchor %s | %s",
					Cue.strOccurrenceId.c_str(), Cue.strEffectAssetId.c_str(),
					Cue.strAnchorSlotId.c_str(),
					Cue.bUsesStageClock ? "stage clock" :
						Cue.strClipOccurrenceId.c_str());
				ImGui::SameLine();
				if (ImGui::SmallButton("Open Effect Tool"))
				{
					m_strEffectToolOpenPatternId = Pattern.strPatternId;
					m_strEffectToolOpenStageId = Stage.strStageId;
					m_strEffectToolOpenCueOccurrenceId = Cue.strOccurrenceId;
					m_strEffectToolOpenEffectAssetId = Cue.strEffectAssetId;
					m_hasEffectToolOpenRequest = true;
				}
				ImGui::PopID();
			}
			for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& CombatObject :
				Stage.CombatObjectEffects)
			{
				ImGui::BulletText(
					"Server combat object %s x%u -> %s",
					CombatObject.strCombatObjectArchetypeId.c_str(),
					CombatObject.iSpawnValue,
					CombatObject.strEffectAssetId.c_str());
				ImGui::TextDisabled(
					"  Product clock (read-only): life %u ms | %s | origin %s | direction %s | speed %.3f m/s | max %.3f m",
					CombatObject.iLifetimeMs,
					CombatObject.strKind.c_str(),
					CombatObject.strOriginPolicy.c_str(),
					CombatObject.strDirectionPolicy.c_str(),
					CombatObject.fSpeedMps,
					CombatObject.fMaximumDistanceM);
				for (std::size_t iHit = 0u;
					iHit < CombatObject.HitOffsetsMs.size(); ++iHit)
				{
					const char_t* pHitId = iHit < CombatObject.HitIds.size() ?
						CombatObject.HitIds[iHit].c_str() : "MISSING_HIT_ID";
					ImGui::TextDisabled(
						"  Server hit %s at combat-object +%u ms (local clock; stage duration %u ms)",
						pHitId, CombatObject.HitOffsetsMs[iHit],
						Stage.iDurationMs);
				}
				if ("VALTAN_HIGH_JUMP" == Pattern.strPatternId &&
					"AIRBORNE" == Stage.strStageId)
				{
					ImGui::TextColored(
						ImVec4(0.35f, 0.75f, 1.f, 1.f),
						"  Separate clocks: AIRBORNE stage %u ms | axe lifetime %u ms | first axe-local hit atMs %u",
						Stage.iDurationMs, CombatObject.iLifetimeMs,
						CombatObject.HitOffsetsMs.empty() ? 0u :
							CombatObject.HitOffsetsMs.front());
				}
			}

			ImGui::SeparatorText("Sound");
			if (SoundCues.empty() && CombatObjectSoundCues.empty())
				ImGui::TextDisabled("No Sound cue on this stage.");
			for (std::size_t iCue = 0u; iCue < SoundCues.size(); ++iCue)
			{
				const VALTAN_PATTERN_SOUND_CUE& Cue = *SoundCues[iCue];
				ImGui::PushID(static_cast<int32_t>(iCue));
				ImGui::BulletText(
					"%s | event %s | clip +%u ms | %s",
					Cue.strOccurrenceId.c_str(), Cue.strSoundEvent.c_str(),
					Cue.iStartMs,
					VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
						Cue.eRepeatPolicy ? "each loop" : "once");
				const std::vector<std::string>& Variants =
					CSoundCueCatalog::Find_Variants("Valtan", Cue.strSoundEvent);
				if (Variants.empty())
				{
					ImGui::TextColored(
						ImVec4(1.f, 0.35f, 0.25f, 1.f),
						"  MISSING: event has no WAV asset variant.");
				}
				for (std::size_t iVariant = 0u;
					iVariant < Variants.size(); ++iVariant)
				{
					ImGui::PushID(static_cast<int32_t>(iVariant));
					if (ImGui::SmallButton("Preview WAV"))
						(void)Preview_ValtanSoundAsset(Variants[iVariant]);
					ImGui::SameLine();
					ImGui::TextDisabled("%s", Variants[iVariant].c_str());
					ImGui::PopID();
				}
				ImGui::PopID();
			}
			if (!CombatObjectSoundCues.empty())
				ImGui::SeparatorText("Server Semantic Event Sound");
			ImGui::PushID("server-semantic-event-sounds");
			for (std::size_t iCue = 0u;
				iCue < CombatObjectSoundCues.size(); ++iCue)
			{
				const JOINED_COMBAT_OBJECT_SOUND& Joined =
					CombatObjectSoundCues[iCue];
				VALTAN_COMBAT_OBJECT_SOUND_CUE& Cue = *Joined.pCue;
				ImGui::PushID(static_cast<int32_t>(iCue));
				ImGui::BulletText(
					"%s | %s + %s (%s) | event %s | object +%u ms",
					Cue.strBindingId.c_str(),
					Cue.strCombatObjectArchetypeId.c_str(),
					Joined.strServerEventId.c_str(),
					Joined.bPresentationEvent ? "presentation" : "hit",
					Cue.strSoundEvent.c_str(), Joined.iEventOffsetMs);
				ImGui::SetNextItemWidth(360.f);
				if (ImGui::BeginCombo("Impact Sound Event", Cue.strSoundEvent.c_str()))
				{
					for (const std::string& EventName : ValtanSoundEventNames)
					{
						const bool_t bSelected = EventName == Cue.strSoundEvent;
						if (ImGui::Selectable(EventName.c_str(), bSelected) && !bSelected)
						{
							Cue.strSoundEvent = EventName;
							m_bValtanCombatObjectSoundCuesDirty = true;
							m_strValtanCombatObjectSoundCueStatus =
								"UNSAVED: " + Cue.strBindingId + " -> " + EventName;
						}
						if (bSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				const std::vector<std::string>& Variants =
					CSoundCueCatalog::Find_Variants("Valtan", Cue.strSoundEvent);
				for (std::size_t iVariant = 0u;
					iVariant < Variants.size(); ++iVariant)
				{
					ImGui::PushID(static_cast<int32_t>(iVariant));
					if (ImGui::SmallButton("Preview Impact WAV"))
						(void)Preview_ValtanSoundAsset(Variants[iVariant]);
					ImGui::SameLine();
					ImGui::TextDisabled("%s", Variants[iVariant].c_str());
					ImGui::PopID();
				}
				ImGui::PopID();
			}
			ImGui::PopID();
			if (0u != iMissingCombatObjectSounds)
			{
				ImGui::TextColored(
					ImVec4(1.f, 0.25f, 0.20f, 1.f),
					"COVERAGE GAP: %zu Server combat-object semantic event(s) have no exact Sound binding.",
					iMissingCombatObjectSounds);
			}

			ImGui::SeparatorText("Camera / Shake");
			if (Stage.CameraInvocations.empty() && ShakeCues.empty())
				ImGui::TextDisabled("No Camera or Shake cue on this stage.");
			for (std::size_t iCue = 0u;
				iCue < Stage.CameraInvocations.size(); ++iCue)
			{
				const VALTAN_CAMERA_INVOCATION_VIEW& Invocation =
					Stage.CameraInvocations[iCue];
				ImGui::PushID(static_cast<int32_t>(iCue));
				ImGui::BulletText(
					"%s -> %s | %s +%u ms | %s %u ms",
					Invocation.strCameraInvocationId.c_str(),
					Invocation.strCameraCueId.c_str(),
					Invocation.strTrigger.c_str(), Invocation.iStartOffsetMs,
					Invocation.strDurationPolicy.c_str(), Invocation.iDurationMs);
				ImGui::SameLine();
				if (ImGui::SmallButton("Open Camera Tool"))
				{
					m_strCameraToolOpenCueId = Invocation.strCameraCueId;
					m_hasCameraToolOpenRequest = true;
				}
				ImGui::PopID();
			}
			ImGui::PushID("camera-shakes");
			for (std::size_t iCue = 0u; iCue < ShakeCues.size(); ++iCue)
			{
				const VALTAN_PATTERN_SHAKE_CUE& Cue = *ShakeCues[iCue];
				ImGui::PushID(static_cast<int32_t>(iCue));
				ImGui::BulletText(
					"%s | %s +%u ms | %.3f s | F/R/U/FOV amp %.3g/%.3g/%.3g/%.3g | %s",
					Cue.strOccurrenceId.c_str(), Cue.strClipOccurrenceId.c_str(),
					Cue.iStartMs, Cue.Spec.fDurationSeconds,
					Cue.Spec.Forward.fAmplitude, Cue.Spec.Right.fAmplitude,
					Cue.Spec.Up.fAmplitude, Cue.Spec.Fov.fAmplitude,
					VALTAN_PATTERN_SHAKE_REPEAT_POLICY::EACH_LOOP ==
						Cue.eRepeatPolicy ? "each loop" : "once");
				ImGui::PopID();
			}
			ImGui::PopID();

			ImGui::SeparatorText("World Event / Runtime UI");
			if (WorldEvents.empty())
				ImGui::TextDisabled("No world-event trigger on this stage.");
			for (const VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW* const pEvent :
				WorldEvents)
			{
				ImGui::BulletText(
					"%s/%s -> %s",
					pEvent->strPatternId.c_str(), pEvent->strStageId.c_str(),
					pEvent->strTriggerKind.c_str());
			}
			ImGui::TextDisabled(
				"HUD/UI observes replicated Server state; this stage does not author a second UI command path.");

			ImGui::TreePop();
		}
		ImGui::PopID();
	}
}

void Client::CAnimation_Tool::Render_ValtanSelectedResourceUsage(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW* pStage)
{
	ImGui::SeparatorText("Used by Selected Pattern");
	ImGui::TextDisabled(
		"Semantic runtime relationships only. Choose a slot to focus its typed Persistent Detail owner.");
	const auto SemanticSlot = [this](
		const char_t* pSlot,
		const VALTAN_WORKBENCH_DETAIL_OWNER eOwner,
		const std::string& Meaning,
		const std::string_view ExactOwner)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushID(pSlot);
		if (ImGui::SmallButton(pSlot))
		{
			m_eValtanWorkbenchDetailOwner = eOwner;
			m_bValtanWorkbenchFocusDetailRequested = true;
		}
		ImGui::PopID();
		ImGui::TableSetColumnIndex(1);
		ImGui::TextWrapped("%s", Meaning.c_str());
		ImGui::TableSetColumnIndex(2);
		ImGui::TextWrapped(
			"%.*s", static_cast<int32_t>(ExactOwner.size()), ExactOwner.data());
	};
	if (ImGui::BeginTable(
		"##ValtanSelectedSemanticResources", 3,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
			ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableSetupColumn(
			"Meaning slot", ImGuiTableColumnFlags_WidthFixed, 190.f);
		ImGui::TableSetupColumn(
			"Selected relationship", ImGuiTableColumnFlags_WidthStretch, 0.55f);
		ImGui::TableSetupColumn(
			"Exact typed owner", ImGuiTableColumnFlags_WidthStretch, 0.45f);
		ImGui::TableHeadersRow();

		const std::string StageIdentity = nullptr == pStage ?
			"Pattern " + Pattern.strPatternId +
				" selected; choose a semantic stage for exact action ownership." :
			"Pattern " + Pattern.strPatternId + " / stage " +
				pStage->strStageId + " / action " + pStage->strActionId +
				" / wall " + std::to_string(pStage->iDurationMs) + " ms";
		SemanticSlot(
			"Gameplay Source / Product",
			VALTAN_WORKBENCH_DETAIL_OWNER::GAMEPLAY,
			StageIdentity,
			"Data/Valtan/Valtan.gameplay.json -> Data/Encounters/Valtan/ValtanEncounter.json");

		std::string HitMeaning =
			"No semantic stage selected; collider/hit ownership is not guessed.";
		if (nullptr != pStage)
		{
			HitMeaning = pStage->Has_HitShape() ?
				("shape " + pStage->strHitShape + " | damage " +
				 pStage->strServerDamageProfileId + " | hits " +
				 std::to_string(pStage->iHitCount)) :
				"No Server hit shape on the selected stage; this is an explicit empty gameplay slot.";
		}
		SemanticSlot(
			"Collider / Hit", VALTAN_WORKBENCH_DETAIL_OWNER::GAMEPLAY,
			HitMeaning,
			"Data/Valtan/Valtan.gameplay.json + Data/Balance/DamageProfiles.json");

		std::string MotionMeaning =
			"No semantic stage selected; motion/action/branch flow is not guessed.";
		if (nullptr != pStage)
		{
			MotionMeaning = pStage->Motion.has_value() ?
				("stage motion " + pStage->Motion->strKind + " | actions " +
				 std::to_string(pStage->Actions.size()) + " | branches " +
				 std::to_string(pStage->Branches.size())) :
				("No stage motion; actions " +
				 std::to_string(pStage->Actions.size()) + " | branches " +
				 std::to_string(pStage->Branches.size()));
		}
		else if (Pattern.ServerMotion.has_value())
		{
			MotionMeaning = "pattern motion " + Pattern.ServerMotion->strKind;
		}
		SemanticSlot(
			"Motion / Flow", VALTAN_WORKBENCH_DETAIL_OWNER::GAMEPLAY,
			MotionMeaning,
			"Data/Valtan/Valtan.gameplay.json");

		const std::string ChainMeaning =
			Pattern.strSourceAnimationChainId.empty() ?
				"No promoted Animation Intake chain is recorded for this Product pattern." :
				("source chain " + Pattern.strSourceAnimationChainId);
		SemanticSlot(
			"Animation Chain", VALTAN_WORKBENCH_DETAIL_OWNER::ANIMATION,
			ChainMeaning,
			"Data/Valtan/Valtan.presentation.json (joined read-only chain identity)");

		std::string ClipMeaning =
			"No semantic stage selected; clip occurrences are not guessed.";
		if (nullptr != pStage)
		{
			if (pStage->bSuppressAnimation)
			{
				ClipMeaning = "Explicit NONE / hold current boss pose.";
			}
			else if (pStage->ClipOccurrences.empty())
			{
				ClipMeaning = "No admitted clip occurrence on the selected stage.";
			}
			else
			{
				ClipMeaning.clear();
				for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
					pStage->ClipOccurrences)
				{
					if (!ClipMeaning.empty())
						ClipMeaning += " | ";
					ClipMeaning += Clip.strClipOccurrenceId + " -> " +
						Clip.strClipName;
				}
			}
		}
		SemanticSlot(
			"Animation Clips", VALTAN_WORKBENCH_DETAIL_OWNER::ANIMATION,
			ClipMeaning,
			"OWNER Data/Valtan/Valtan.presentation.json -> READ-ONLY PRODUCT Data/Animation/Authored/Valtan/Valtan.patternbindings.json");

		std::string EffectMeaning =
			"No semantic stage selected; Effect ownership is not guessed.";
		if (nullptr != pStage)
		{
			if (pStage->ProductCues.empty())
			{
				EffectMeaning =
					"No Product Effect cue on the selected stage; no synthetic Effect JSON is offered.";
			}
			else
			{
				EffectMeaning.clear();
				for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
					pStage->ProductCues)
				{
					if (!EffectMeaning.empty())
						EffectMeaning += " | ";
					EffectMeaning += Cue.strOccurrenceId + " -> " +
						Cue.strEffectAssetId + " (Data/Effects/Authored/" +
						Cue.strEffectAssetId + ".effect.json)";
				}
			}
		}
		SemanticSlot(
			"Effect Cues / Assets", VALTAN_WORKBENCH_DETAIL_OWNER::EFFECT,
			EffectMeaning,
			"OWNER Data/Valtan/Valtan.presentation.json -> READ-ONLY PRODUCT Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json + exact authored Effect assets");

		std::string SoundMeaning =
			"No Pattern Sound cue on the selected stage; no WAV asset is inferred.";
		if (nullptr != pStage)
		{
			for (const VALTAN_PATTERN_SOUND_CUE& Cue :
				m_ValtanPatternSoundCues.Cues)
			{
				if (Cue.strPatternId != Pattern.strPatternId ||
					Cue.strStageId != pStage->strStageId)
				{
					continue;
				}
				if (0u == SoundMeaning.find("No Pattern Sound cue"))
					SoundMeaning.clear();
				if (!SoundMeaning.empty())
					SoundMeaning += " | ";
				SoundMeaning += Cue.strOccurrenceId + " -> " + Cue.strSoundEvent;
				const std::vector<std::string>& Variants =
					CSoundCueCatalog::Find_Variants("Valtan", Cue.strSoundEvent);
				if (Variants.empty())
					SoundMeaning += " (MISSING asset variant)";
				for (const std::string& Variant : Variants)
					SoundMeaning += " -> " + Variant;
			}
		}
		SemanticSlot(
			"Sound Cues / Assets", VALTAN_WORKBENCH_DETAIL_OWNER::SOUND,
			SoundMeaning,
			"Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json + Data/Sound catalog assets");

		std::string CameraWorldMeaning =
			"No semantic stage selected; Camera/World relationships are not guessed.";
		if (nullptr != pStage)
		{
			CameraWorldMeaning = "camera invocations " +
				std::to_string(pStage->CameraInvocations.size());
			std::size_t iWorldCount = 0u;
			for (const VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW& Event :
				Pattern.WorldEventTriggerRefs)
			{
				if (Event.strStageId == pStage->strStageId)
					++iWorldCount;
			}
			CameraWorldMeaning += " | world triggers " +
				std::to_string(iWorldCount);
		}
		SemanticSlot(
			"Camera / World", VALTAN_WORKBENCH_DETAIL_OWNER::CAMERA,
			CameraWorldMeaning,
			"Data/Animation/Authored/Valtan/Valtan.patternshakecues.json + Data/Encounters/Valtan typed owners");
		ImGui::EndTable();
	}

	if (ImGui::CollapsingHeader("Raw owner index / diagnostics"))
	{
		ImGui::TextDisabled(
			"Complete owner inventory for diagnosis only. It is not a second JSON editor and does not replace the semantic slots above.");
		ImGui::TextDisabled(
			"Animation/Effect invocation Products are projector-owned and never dirty in this Workbench.");
		if (m_bValtanPatternSoundCuesDirty)
			ImGui::TextColored(
				ImVec4(1.f, 0.75f, 0.2f, 1.f),
				"Pattern Sound owner: UNSAVED typed draft");
		if (ImGui::BeginTable(
			"##ValtanWorkbenchOwnerFiles", 3,
			ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
				ImGuiTableFlags_Resizable |
				ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableSetupColumn(
				"Domain", ImGuiTableColumnFlags_WidthFixed, 110.f);
			ImGui::TableSetupColumn(
				"Owner / state", ImGuiTableColumnFlags_WidthFixed, 235.f);
			ImGui::TableSetupColumn(
				"Exact source", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();
			const auto DataFileRow = [](
				const char_t* pDomain,
				const char_t* pOwnerState,
				const std::string_view strPath)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TextUnformatted(pDomain);
				ImGui::TableSetColumnIndex(1);
				ImGui::TextWrapped("%s", pOwnerState);
				ImGui::TableSetColumnIndex(2);
				ImGui::TextWrapped(
					"%.*s", static_cast<int32_t>(strPath.size()),
					strPath.data());
			};
			DataFileRow(
				"Gameplay", "EDITABLE HERE / typed atomic Save",
				"Data/Valtan/Valtan.gameplay.json");
			DataFileRow(
				"Presentation", "AUTHORING OWNER / immutable revision Save",
				"Data/Valtan/Valtan.presentation.json");
			DataFileRow(
				"Animation Product", "READ-ONLY / generated by projector",
				"Data/Animation/Authored/Valtan/Valtan.patternbindings.json");
			DataFileRow(
				"Effect Invocation Product", "READ-ONLY / generated by projector",
				"Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json");
			DataFileRow(
				"Sound", "EDITABLE IN PERSISTENT DETAIL / typed owner Save",
				"Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json");
			DataFileRow(
				"Camera / World", "READ-ONLY HERE / typed owner tools",
				"Data/Animation/Authored/Valtan/Valtan.patternshakecues.json + Data/Encounters/Valtan");
			ImGui::EndTable();
		}
	}
}

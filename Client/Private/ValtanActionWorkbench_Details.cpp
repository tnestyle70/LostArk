// 선택된 행의 Detail 입력과 적용·preview 요청을 담당한다. 세션 상태는 CValtanActionWorkbench가 계속 소유한다.
// Details는 입력 widget과 기존 typed setter 사이의 경계다. 읽기 전용 frame view에서 값 후보를 만들고
// 검증을 통과한 변경만 Balance·Animation·Effect 소유 도구에 전달한다. widget의 임시 값과 저장된 원본은 다르다.
#include "imgui.h"
#include "ValtanActionWorkbench.h"
#include "BalanceTool.h"
#include "Animation_Tool.h"
#include "AnimationTargetService.h"
#include "Valtan.h"
#include "PlayerHandGripTransform.h"
#include "EffectV2_Catalog.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ValtanActionWorkbench_EditSupport.h"

// EditSupport에 선언된 동일한 ID·시간·종속 검증 함수를 사용해 본체와 Details의 적용 규칙이 갈라지지 않게 한다.
using namespace Client::ValtanWorkbenchDetail;

// 이 이름 공간의 비교 함수는 Details에서만 쓰므로 파일 내부 링크로 둔다.
// 이 비교는 widget 임시값의 변경 여부를 판정하며 문서 저장이나 runtime 적용은 수행하지 않는다.
namespace
{

	// 메모리 전체를 비교하면 padding이나 표시용 필드까지 섞일 수 있으므로 저장 의미가 있는 필드를 명시적으로 대조한다.
	// 선택 ID가 같아도 source 구간·부착·종료 정책이 달라졌다면 새 편집 기준으로 인식한다.
	bool_t SameSavedValtanEffectCue(
		const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& left,
		const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& right)
	{
		return left.strBindingId == right.strBindingId &&
			left.strOccurrenceId == right.strOccurrenceId &&
			left.strPatternId == right.strPatternId &&
			left.strStageId == right.strStageId &&
			left.strActionId == right.strActionId &&
			left.strClipOccurrenceId == right.strClipOccurrenceId &&
			left.strEffectAssetId == right.strEffectAssetId &&
			left.strV1EffectAssetId == right.strV1EffectAssetId &&
			left.strAnchorSlotId == right.strAnchorSlotId &&
			left.eFollowPolicy == right.eFollowPolicy &&
			left.eStopPolicy == right.eStopPolicy &&
			left.strFollowPolicy == right.strFollowPolicy &&
			left.strStopPolicy == right.strStopPolicy &&
			left.strRepeatPolicy == right.strRepeatPolicy &&
			left.eScalePolicy == right.eScalePolicy &&
			left.strScalePolicy == right.strScalePolicy &&
			left.vWorldScale.x == right.vWorldScale.x &&
			left.vWorldScale.y == right.vWorldScale.y &&
			left.vWorldScale.z == right.vWorldScale.z &&
			left.bHasExplicitScalePolicy == right.bHasExplicitScalePolicy &&
			left.bUsesStageClock == right.bUsesStageClock &&
			left.iStageOffsetMs == right.iStageOffsetMs &&
			left.iSourceStartMs == right.iSourceStartMs &&
			left.bHasPlaybackOffset == right.bHasPlaybackOffset &&
			(!left.bHasPlaybackOffset || left.iPlaybackOffsetMs == right.iPlaybackOffsetMs) &&
			left.iSourceEndMs == right.iSourceEndMs &&
			left.iStageDurationMs == right.iStageDurationMs &&
			left.bHasSourceEnd == right.bHasSourceEnd &&
			left.LocalTransform.vPosition.x ==
				right.LocalTransform.vPosition.x &&
			left.LocalTransform.vPosition.y ==
				right.LocalTransform.vPosition.y &&
			left.LocalTransform.vPosition.z ==
				right.LocalTransform.vPosition.z &&
			left.LocalTransform.vRotationDegrees.x ==
				right.LocalTransform.vRotationDegrees.x &&
			left.LocalTransform.vRotationDegrees.y ==
				right.LocalTransform.vRotationDegrees.y &&
			left.LocalTransform.vRotationDegrees.z ==
				right.LocalTransform.vRotationDegrees.z &&
			left.LocalTransform.vScale.x == right.LocalTransform.vScale.x &&
			left.LocalTransform.vScale.y == right.LocalTransform.vScale.y &&
			left.LocalTransform.vScale.z == right.LocalTransform.vScale.z;
	}
}



// 선택된 Animation의 저작 시각을 입력받는 Detail UI다. 명령을 호출한 뒤 원래 preview 갱신·상태 반환 순서를 유지한다.
void Client::CValtanActionWorkbench::Render_SelectedAnimationTiming(
	const VALTAN_PATTERN_VIEW& Pattern,
	const bool_t bMutationAdmitted)
{
	if (DETAIL_OWNER::ANIMATION != m_eDetailOwner ||
		m_strSelectedStableId.empty() || nullptr == m_pBalanceTool)
	{
		return;
	}
	const VALTAN_STAGE_VIEW* const pStage = Find_SelectedStage(&Pattern);
	if (nullptr == pStage)
		return;
	if (VALTAN_VIEW_ADMISSION::ADMITTED != m_eAdmission)
	{
		const auto Pinned = std::find_if(
			pStage->ClipOccurrences.begin(), pStage->ClipOccurrences.end(),
			[this](const VALTAN_CLIP_OCCURRENCE_VIEW& Slot)
			{
				return Slot.strClipOccurrenceId == m_strSelectedStableId;
			});
		if (pStage->ClipOccurrences.end() != Pinned)
		{
			ImGui::SeparatorText("Selected Animation Occurrence Timing");
			ImGui::Text("%s", Pinned->strClipName.c_str());
			ImGui::TextDisabled(
				"Pinned source %u ms | play %u ms | rate %.3f | read only",
				Pinned->iSourceStartMs, Pinned->iPlayMs, Pinned->fPlayRate);
		}
		return;
	}

	CBalanceTool::PATTERN_STAGE_EDIT Draft;
	std::string Status;
	if (!m_pBalanceTool->Get_ValtanStageDraft(
			Pattern.strPatternId, pStage->strStageId, Draft, Status))
	{
		ImGui::TextDisabled("Selected Animation timing unavailable: %s",
			Status.c_str());
		return;
	}
	const auto Found = std::find_if(
		Draft.animationSlots.begin(), Draft.animationSlots.end(),
		[this](const CBalanceTool::ANIMATION_SLOT_EDIT& Slot)
		{
			return Slot.clipOccurrenceId == m_strSelectedStableId;
		});
	if (Draft.animationSlots.end() == Found)
		return;

	ImGui::SeparatorText("Selected Animation Occurrence Timing");
	ImGui::Text("%s", Found->clip.c_str());
	ImGui::SameLine();
	ImGui::TextDisabled("| %s", Found->clipOccurrenceId.c_str());
	ImGui::TextDisabled(
		"Source Start and Source Duration are native clip milliseconds. Wall duration = Source Duration / Play Rate. Edge trims ripple later clips inside the unchanged Server Stage; a shortened EXACT playlist holds its last pose.");
	int32_t iSourceStartMs = static_cast<int32_t>(Found->sourceStartMs);
	int32_t iPlayMs = static_cast<int32_t>(Found->playMs);
	ImGui::PushID("##SelectedAnimationOccurrenceTiming");
	ImGui::BeginDisabled(!bMutationAdmitted || !Draft.animationEditable);
	(void)ImGui::InputInt(
		"Source Start (ms)", &iSourceStartMs, 5, 100);
	const bool_t bSourceChanged = ImGui::IsItemDeactivatedAfterEdit();
	ImGui::BeginDisabled(Found->repeatUntilStageEnd);
	(void)ImGui::InputInt(
		"Source Duration (ms)", &iPlayMs, 5, 100);
	const bool_t bPlayChanged = ImGui::IsItemDeactivatedAfterEdit();
	ImGui::EndDisabled();
	if (bSourceChanged || bPlayChanged)
	{
		iSourceStartMs = (std::clamp)(iSourceStartMs, 0, 600000);
		iPlayMs = (std::clamp)(iPlayMs, 0, 600000);
		if (Apply_AnimationOccurrenceTiming(
				Pattern, *pStage, Found->clipOccurrenceId,
				static_cast<uint32_t>(iSourceStartMs),
				static_cast<uint32_t>(iPlayMs), Status))
		{
			Invalidate_TimelineCache();
		}
		m_strStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	ImGui::PopID();
	if (Found->repeatUntilStageEnd)
	{
		ImGui::TextDisabled(
			"Loop occurrence: playMs remains 0 and the Server Stage clock owns its visible end.");
	}
}

// 현재 선택 ID와 입력 초안이 같은 Effect를 가리키는지 확인한다. 보관 중인 입력 초안과 저장 가능한 cue 값의 차이만 읽고 문서를 변경하지 않는다.
bool_t Client::CValtanActionWorkbench::Has_PendingSelectedEffectDetails(
    const VALTAN_PATTERN_VIEW* const Pattern) const
{
    if (!Pattern || m_eDetailOwner != DETAIL_OWNER::EFFECT || m_strEffectEditIdentity.empty() ||
        Pattern->strPatternId != m_strSelectedPatternId) return false;
    const auto stage = std::find_if(Pattern->Stages.begin(), Pattern->Stages.end(),
        [&](const auto& row) { return row.strStageId == m_strSelectedStageId; });
    if (stage == Pattern->Stages.end()) return false;
    const auto cue = std::find_if(stage->ProductCues.begin(), stage->ProductCues.end(),
        [&](const auto& row) { return row.strOccurrenceId == m_strSelectedStableId; });
    return cue != stage->ProductCues.end() &&
        m_strEffectEditIdentity == cue->strBindingId + "/" + cue->strOccurrenceId &&
        m_EffectCueEditDraft.strBindingId == cue->strBindingId &&
        m_EffectCueEditDraft.strOccurrenceId == cue->strOccurrenceId &&
        !SameSavedValtanEffectCue(*cue, m_EffectCueEditDraft);
}

// Effect 숫자 입력 완료 시 시계·종속 구간 검증 후 BalanceTool에 반영한다. 실패하면 입력 후보만 거부하고 기존 draft를 유지하며 성공 뒤 preview 갱신을 요청한다.
// 보충: const 참조의 Pattern/Stage는 이번 호출의 읽기 입력이다. 수정은 값 draft와 owner API로 전달하며 참조로 원본을 직접 바꾸지 않는다.
// owner API가 저장소를 갱신한 뒤에는 이전 cue 포인터로 후속 편집을 계속하지 않고 표시 cache를 다시 조회해야 한다.
bool_t Client::CValtanActionWorkbench::Apply_EffectOccurrenceDetails(
    const VALTAN_PATTERN_VIEW& Pattern, const VALTAN_STAGE_VIEW& Stage,
    const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Current,
    const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Candidate, std::string& status)
{
    if (!Can_MutateValtanView(m_eAdmission) || nullptr == m_pBalanceTool)
    {
        status = "Effect details require an admitted Pattern authoring draft.";
        return false;
    }
    VALTAN_PRODUCT_EFFECT_CUE_VIEW Applied = Candidate;
    const auto effectIn = [](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& cue)
    {
        if (cue.bHasPlaybackOffset) return cue.iPlaybackOffsetMs;
        return !cue.bUsesStageClock && !cue.strBindingId.starts_with("cue.valtan.composition.") &&
            cue.strEffectAssetId.starts_with("effect.valtan.action.") &&
            cue.strEffectAssetId.ends_with(".full.restore") ? cue.iSourceStartMs : 0u;
    };
    // 숫자로 Effect In을 바꿀 때 명시적인 Effect Out은 고정한다.
    // 자연 수명 cue는 줄어든 잔여 수명을 사용하며 box 드래그가 실제 시간 구간 전체의 이동을 담당한다.
    if (Current.bHasSourceEnd && Applied.bHasSourceEnd &&
        Current.iSourceStartMs == Applied.iSourceStartMs && Current.iSourceEndMs == Applied.iSourceEndMs &&
        effectIn(Current) != effectIn(Applied))
    {
        const int64_t end = static_cast<int64_t>(Current.iSourceEndMs) + effectIn(Current) - effectIn(Applied);
        if (end <= Applied.iSourceStartMs || end > 600000)
        {
            status = "Effect In must leave a positive playback duration before the existing Effect Out.";
            return false;
        }
        Applied.iSourceEndMs = static_cast<uint32_t>(end);
    }
    if (SameSavedValtanEffectCue(Current, Applied))
    {
        status = "Effect details are unchanged.";
        return true;
    }
    if (!ValidateValtanEffectInvocationClock(m_pAnimationTool, Stage, Applied, status) ||
        !Validate_TimelineDependencyWindows(Pattern, Stage, &Applied, nullptr, status) ||
        !m_pBalanceTool->Update_ValtanStageEffectCue(Pattern.strPatternId, Stage.strStageId,
            Stage.strActionId, Current.strBindingId, Current.strOccurrenceId, Applied, status))
        return false;
    // widget 편집 완료 시 Source Save가 직렬화하는 같은 소유 도구에 stage한다.
    // 현재 preview 시각·일시정지를 보존하며 편집 뒤 한 번 다시 구성한다.
    m_bAuthoringDraftDirty = m_pBalanceTool->Is_ValtanDraftDirty();
    Refresh_PatternLocalPreviewAfterMutation(&Pattern, status);
    return true;
}

// Stage의 gameplay 입력 UI를 그린다. 필드는 기존 BalanceTool draft를 통해 적용하며, 이 파일 분리는 gameplay나 저장 소유자를 새로 만들지 않는다.
void Client::CValtanActionWorkbench::Render_GameplayStageDetails(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const bool_t bMutationAdmitted)
{
	if (VALTAN_VIEW_ADMISSION::ADMITTED != m_eAdmission)
	{
		/* STALE_PRESERVED는 고정된 불변 generation이다.
		   widget 값을 채우려고 그 canonical Pattern에 더 새로운 Balance draft를 섞지 않는다. */
		ImGui::Text("%s / %s", Pattern.strPatternId.c_str(),
			Stage.strStageId.c_str());
		ImGui::TextDisabled("Action %s | %s | %u ms | sequence role %s",
			Stage.strActionId.c_str(), Stage.strStageKind.c_str(),
			Stage.iDurationMs, Stage.strSequenceRole.c_str());
		ImGui::TextColored(ImVec4(1.f, 0.70f, 0.25f, 1.f),
			"Pinned canonical generation (read only). Reload Canonical before viewing or editing a newer authoring draft.");
		ImGui::SeparatorText("Stage Hit (Boss -> Player)");
		ImGui::TextDisabled(
			"Server-authoritative pulse schedule. Effect geometry is presentation only and never becomes hit authority.");
		ImGui::Text("Shape %s | count %u | delay %u ms | interval %u ms",
			Stage.strHitShape.c_str(), Stage.iHitCount,
			Stage.iHitDelayMs, Stage.iHitIntervalMs);
		ImGui::Text("Damage %s | response %s | attachment %s",
			Stage.strServerDamageProfileId.c_str(),
			Stage.strPlayerResponse.c_str(), Stage.strAttachmentSlot.c_str());
		ImGui::Text("Push %.2f m / %u ms | knockdown %s / %u ms",
			Stage.fPushRangeM, Stage.iPushMs,
			Stage.bKnockdown ? "true" : "false", Stage.iDownMs);
		for (const VALTAN_STAGE_ACTION_VIEW& Action : Stage.Actions)
		{
			if ("RELEASE_GRABBED_PLAYERS" != Action.strKind)
				continue;
			ImGui::SeparatorText("Grab Release");
			ImGui::Text("%s | %.2f m/s | %u ms | yaw %.1f deg",
				Action.strReleaseMode.c_str(), Action.fSpeedMps,
				Action.iDurationMs, Action.fYawOffsetDegrees);
		}
		const auto Counter = std::find_if(
			Stage.Branches.begin(), Stage.Branches.end(),
			[](const VALTAN_STAGE_BRANCH_VIEW& Branch)
			{ return "COUNTER_HIT" == Branch.strOutcome; });
		if (Counter != Stage.Branches.end())
		{
			ImGui::SeparatorText("Counter Hurt Proxy (Player -> Boss)");
			ImGui::TextDisabled(
				"The boss-local hurt proxy is active for the whole WINDUP Stage; it is not a boss-to-player hit collider.");
			ImGui::Text("COUNTER_HIT -> %s",
				BuildBranchTargetLabel(*Counter).c_str());
		}
		return;
	}
	if (nullptr == m_pBalanceTool)
	{
		ImGui::TextDisabled("Gameplay owner is unavailable.");
		return;
	}
	CBalanceTool::PATTERN_STAGE_EDIT Draft;
	std::string DraftStatus;
	if (!m_pBalanceTool->Get_ValtanStageDraft(
			Pattern.strPatternId, Stage.strStageId, Draft, DraftStatus))
	{
		ImGui::TextWrapped("Typed gameplay draft unavailable: %s", DraftStatus.c_str());
		return;
	}
	ImGui::Text("%s / %s", Pattern.strPatternId.c_str(), Stage.strStageId.c_str());
	ImGui::TextDisabled("Action %s | %s", Draft.actionId.c_str(), Draft.stageKind.c_str());
	const std::string strStageDetailPrefix = Stage.strStageId + "/";
	const bool_t bFocusStageClock = m_strSelectedStableId == Stage.strStageId;
	const bool_t bFocusTopology = m_strSelectedStableId ==
		Stage.strStageId + "/topology";
	const bool_t bFocusCollider = 0u == m_strSelectedStableId.rfind(
		strStageDetailPrefix + "collider", 0u);
	const auto SelectedBranch = std::find_if(
		Stage.Branches.begin(), Stage.Branches.end(),
		[this, &Stage](const VALTAN_STAGE_BRANCH_VIEW& Branch)
		{
			return m_strSelectedStableId ==
				BuildBranchStableId(Stage.strStageId, Branch.strOutcome);
		});
	const bool_t bCounterAuthoringRequested = m_strSelectedStableId ==
		BuildBranchStableId(Stage.strStageId, "COUNTER_HIT");
	const bool_t bFocusCounterBranch = bCounterAuthoringRequested ||
		(SelectedBranch != Stage.Branches.end() &&
			"COUNTER_HIT" == SelectedBranch->strOutcome);
	const bool_t bFocusNonCounterBranch = SelectedBranch != Stage.Branches.end() &&
		!bFocusCounterBranch;
	const bool_t bFocusAction = 0u == m_strSelectedStableId.rfind(
		strStageDetailPrefix + "action/", 0u);
	const auto SelectedStatusAction = std::find_if(
		Draft.actions.begin(), Draft.actions.end(),
		[this, &strStageDetailPrefix](const VALTAN_STAGE_ACTION_VIEW& Action)
		{
			return "ENTER" == Action.strTrigger &&
				m_strSelectedStableId == strStageDetailPrefix + "action/" +
					Action.strKind + "/" + Action.strTargetId &&
				("SET_STAGGER_GAUGE" == Action.strKind ||
				 "SET_PLAYER_BIND" == Action.strKind ||
				 "SET_PLAYER_SILENCE" == Action.strKind);
		});
	const bool_t bFocusMotion = 0u == m_strSelectedStableId.rfind(
		strStageDetailPrefix + "motion/", 0u);
	const auto FocusDetailSection = [this](const bool_t bMatches)
	{
		if (!m_bDetailFocusRequested || !bMatches)
			return;
		ImGui::SetScrollHereY(0.12f);
		m_bDetailFocusRequested = false;
	};
	if (Pattern.bManualServerAudition)
	{
		ImGui::SeparatorText("Stage Structure");
		FocusDetailSection(bFocusTopology);
		ImGui::TextDisabled(
			"Add, move, or delete Stage nodes in Composition Boss Pattern. Details owns only the selected Stage's typed clocks, motion, collider, reactions, and Logic values.");
		if (ImGui::SmallButton("Open Boss Pattern Structure"))
		{
			m_bBossPatternWindowVisible = true;
			m_bBossPatternFocusRequested = true;
		}
	}
	else
	{
		ImGui::TextDisabled(
			"Stage add/remove/reorder is disabled for canonical rotation/gimmick Patterns.");
	}
	bool_t bWarpRushAppliedThisFrame = false;
	if ("VALTAN_WARP" == Pattern.strPatternId)
	{
		ImGui::SeparatorText("Warp Rush - All 8 Legs");
		FocusDetailSection(bFocusMotion);
		CBalanceTool::VALTAN_WARP_RUSH_EDIT RushDraft{};
		std::string RushStatus;
		const bool_t bRushReady =
			m_pBalanceTool->Get_ValtanWarpRushDraft(RushDraft, RushStatus);
		if (!bRushReady)
		{
			ImGui::TextWrapped("All-leg WARP draft unavailable: %s",
				RushStatus.c_str());
		}
		else
		{
			ImGui::TextDisabled(
				"STEP_02 owns the first portal lead; STEP_03..STEP_09 add the next-portal offset before the same lead. Shared speed/distance changes commit only after all eight arrival-aligned legs validate.");
			bool_t bRushChanged = false;
			ImGui::BeginDisabled(!bMutationAdmitted);
			int32_t iRetargetDelay = static_cast<int32_t>(
				RushDraft.retargetDelayMs);
			if (ImGui::DragInt(
					"Portal Lead Before Rush (ms)##WarpAllLegs", &iRetargetDelay,
					5.f, 0, 120000,
					"%d ms", ImGuiSliderFlags_AlwaysClamp))
			{
				RushDraft.retargetDelayMs = static_cast<uint32_t>(iRetargetDelay);
				bRushChanged = true;
			}
			const double fMinimumSpeedMps = 0.1;
			const double fMaximumSpeedMps = 1000.0;
			const double fMinimumDistanceM = 0.000001;
			const double fMaximumDistanceM = 1000.0;
			bRushChanged |= ImGui::DragScalar(
				"Rush Speed (m/s)##WarpAllLegs", ImGuiDataType_Double,
				&RushDraft.speedMps, 0.05f, &fMinimumSpeedMps,
				&fMaximumSpeedMps, "%.3f", ImGuiSliderFlags_AlwaysClamp);
			bRushChanged |= ImGui::DragScalar(
				"Rush Distance (m)##WarpAllLegs", ImGuiDataType_Double,
				&RushDraft.distanceM, 0.05f, &fMinimumDistanceM,
				&fMaximumDistanceM, "%.3f", ImGuiSliderFlags_AlwaysClamp);
			int32_t iTrailingGap = static_cast<int32_t>(
				RushDraft.trailingGapMs);
			if (ImGui::DragInt(
					"Next Portal Offset After Arrival (ms)##WarpAllLegs", &iTrailingGap,
					5.f, 0, 120000, "%d ms",
					ImGuiSliderFlags_AlwaysClamp))
			{
				RushDraft.trailingGapMs = static_cast<uint32_t>(iTrailingGap);
				bRushChanged = true;
			}
			ImGui::EndDisabled();

			if (bRushChanged)
			{
				CBalanceTool::PATTERN_STAGE_EDIT NormalizedStage{};
				if (m_pBalanceTool->Get_ValtanStageDraft(
						"VALTAN_WARP", "STEP_02", NormalizedStage, RushStatus))
				{
					NormalizedStage.portalRetargetDelayMs =
						RushDraft.retargetDelayMs;
					NormalizedStage.portalSpeedMps = RushDraft.speedMps;
					NormalizedStage.portalDistanceM = RushDraft.distanceM;
					if (CBalanceTool::Normalize_ValtanPortalRushDraft(
							NormalizedStage, 0u,
							RushStatus))
					{
						RushDraft.retargetDelayMs =
							NormalizedStage.portalRetargetDelayMs;
						RushDraft.speedMps = NormalizedStage.portalSpeedMps;
						RushDraft.distanceM = NormalizedStage.portalDistanceM;
						RushDraft.travelMs = RushDraft.distanceM /
							RushDraft.speedMps * 1000.0;
						RushDraft.hitCount = NormalizedStage.hitCount;
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
							}
							RushStatus = AppliedStatus;
							bWarpRushAppliedThisFrame = true;
							Invalidate_TimelineCache();
						}
					}
				}
				m_strStatus = RushStatus;
			}
			const uint32_t iFirstLegDurationMs =
				RushDraft.legDurationMs >= RushDraft.trailingGapMs ?
				RushDraft.legDurationMs - RushDraft.trailingGapMs : 0u;
			ImGui::Text(
				"First leg %u ms; repeated cadence %u ms = next-portal offset %u ms + portal lead %u ms + travel %.3f ms",
				iFirstLegDurationMs, RushDraft.legDurationMs,
				RushDraft.trailingGapMs, RushDraft.retargetDelayMs,
				RushDraft.travelMs);
			ImGui::TextDisabled(
				"Swept hit count %u at 50 ms. Portal visuals use boss-root snapshots at Stage boundaries; they do not predict or expose a Server-authoritative endpoint.",
				RushDraft.hitCount);
			ImGui::TextColored(
				ImVec4(1.f, 0.70f, 0.20f, 1.f),
				"Distance endpoint currently bypasses navigation clamp; keep it inside the arena.");
		}
	}
	FocusDetailSection(bFocusStageClock);
	const CBalanceTool::PATTERN_STAGE_EDIT ColliderBaseline = Draft;
	bool_t bChanged = false;
	if ("WAIT" == Stage.strSequenceRole)
	{
		ImGui::SeparatorText("WAIT Stage Promotion");
		ImGui::TextDisabled(
			"WAIT is a blank Server clock. Promotion preserves this Stage/Action identity and clock, then unlocks exact Animation, Collider, and Counter authoring.");
		std::string RequestedRole;
		ImGui::BeginDisabled(!bMutationAdmitted ||
			!Pattern.bManualServerAudition);
		if (ImGui::Button("Promote WAIT -> ACTIVE"))
			RequestedRole = "ACTIVE";
		ImGui::SameLine();
		if (ImGui::Button("Promote WAIT -> WINDUP"))
			RequestedRole = "WINDUP";
		ImGui::SameLine();
		if (ImGui::Button("Promote WAIT -> GROGGY"))
			RequestedRole = "GROGGY";
		ImGui::EndDisabled();
		if (!RequestedRole.empty())
		{
			std::string PromotionStatus;
			if (m_pBalanceTool->Promote_ValtanManualWaitStage(
					Pattern.strPatternId, Stage.strStageId,
					RequestedRole, PromotionStatus))
			{
				m_strSelectedStageId = Stage.strStageId;
				m_strSelectedStableId = Stage.strStageId;
				m_eDetailOwner = DETAIL_OWNER::GAMEPLAY_STAGE;
				Invalidate_TimelineCache();
				m_strStatus = std::move(PromotionStatus);
				/* Pattern/Stage 참조는 아직 이 node를 WAIT로 표시하는 불변 frame view에 속한다.
				   draft generation이 바뀌면 그리기를 중단하고 다음 프레임에서 같은 stable ID를 다시 찾는다. */
				return;
			}
			m_strStatus = std::move(PromotionStatus);
		}
	}
	ImGui::BeginDisabled(!bMutationAdmitted);
	if (Draft.stageKindEditable)
	{
		const char_t* const StageKinds[] = { "ACTIVE", "WINDUP", "GROGGY" };
		int32_t iStageKind = -1;
		for (std::size_t iKind = 0u; iKind < std::size(StageKinds); ++iKind)
		{
			if (Draft.stageKind == StageKinds[iKind])
				iStageKind = static_cast<int32_t>(iKind);
		}
		if (iStageKind < 0)
		{
			ImGui::TextDisabled(
				"Stage Role: %s (read only; this typed gameplay kind is not a manual role)",
				Draft.stageKind.c_str());
		}
		else if (ImGui::Combo(
				"Stage Role", &iStageKind, StageKinds,
				static_cast<int32_t>(std::size(StageKinds))))
		{
			Draft.stageKind = StageKinds[iStageKind];
			bChanged = true;
		}
	}
	else
	{
		ImGui::TextDisabled(
			"Stage Role is topology-owned for canonical gameplay Patterns.");
	}
	uint32_t iAnimationWallMs = 0u;
	const bool_t bExactWallKnown =
		ComputeExactAnimationWallMs(Draft, iAnimationWallMs);
	const bool_t bWarpLegClockOwned = Draft.portalRushMotionEditable;
	if (!bWarpLegClockOwned && Draft.durationEditable &&
		Draft.animationEditable &&
		"LOOP_TO_STAGE_END" != Draft.animationEndPolicy && bExactWallKnown)
	{
		ImGui::TextDisabled("Animation Sequence wall: %u ms", iAnimationWallMs);
		int32_t iGapMs = static_cast<int32_t>(Draft.durationMs > iAnimationWallMs ?
			Draft.durationMs - iAnimationWallMs : 0u);
		if (ImGui::DragInt(
				"Gap After Sequence / Before Next Stage (ms)", &iGapMs,
				10.f, 0, 120000, "%d ms", ImGuiSliderFlags_AlwaysClamp))
		{
			const uint64_t iDuration = static_cast<uint64_t>(iAnimationWallMs) +
				static_cast<uint32_t>(iGapMs);
			if (iDuration <= 600000u)
			{
				Draft.durationMs = static_cast<uint32_t>(iDuration);
				Draft.animationEndPolicy = 0 == iGapMs ?
					"EXACT" : "HOLD_LAST_POSE";
				bChanged = true;
			}
		}
	}
	else if (!bWarpLegClockOwned)
	{
		int32_t iDurationMs = static_cast<int32_t>((std::min)(
			Draft.durationMs, static_cast<uint32_t>(INT32_MAX)));
		const char_t* const pDurationLabel =
			"LOOP_TO_STAGE_END" == Draft.animationEndPolicy ?
				"Loop Stage Length (ms)" : "Stage Duration (ms)";
		if (Draft.durationEditable && ImGui::DragInt(
				pDurationLabel, &iDurationMs, 10.f, 1, 120000,
				"%d ms", ImGuiSliderFlags_AlwaysClamp))
		{
			std::string ClockStatus;
			if (ApplyStageClockPolicy(
					Draft, static_cast<uint32_t>(iDurationMs), ClockStatus))
			{
				bChanged = true;
			}
			else
			{
				m_strStatus = std::move(ClockStatus);
			}
		}
	}
	if (bWarpLegClockOwned)
	{
		ImGui::TextDisabled(
			"Stage duration is derived for all eight legs from the portal lead + distance/speed travel, with the repeated cadence adding the Next Portal Offset After Arrival above.");
	}
	if (!Draft.durationEditable)
		ImGui::TextDisabled("Stage duration is locked by its typed gameplay policy.");

	ImGui::SeparatorText("Status & Gauge (Non-spatial)");
	if (SelectedStatusAction != Draft.actions.end())
	{
		VALTAN_STAGE_ACTION_VIEW& StatusAction = *SelectedStatusAction;
		ImGui::SeparatorText("Selected Status / Gauge Box");
		FocusDetailSection(true);
		ImGui::Text("%s | %s", StatusAction.strKind.c_str(),
			StatusAction.strTargetId.c_str());
		if ("SET_STAGGER_GAUGE" == StatusAction.strKind)
		{
			ImGui::Text("Server gauge capacity: %.0f", StatusAction.fValue);
			ImGui::TextDisabled(
				"Admitted stagger damage depletes this displayed gauge; break branches to GROGGY and timeout follows the Stage recovery edge.");
		}
		else if ("SET_PLAYER_BIND" == StatusAction.strKind)
		{
			ImGui::Text("Locked target lift: +%.2f m",
				StatusAction.fValue / 1000.f);
			ImGui::TextDisabled(
				"The Server restores the captured valid pose and control on exit, timeout, death, or occurrence abort.");
		}
		else
		{
			ImGui::TextDisabled(
				"Movement remains enabled. The Server rejects skill commands and the HUD reuses the existing cooldown mask.");
		}
		ImGui::Text("Stage window: %u ms", Draft.durationMs);
		ImGui::TextDisabled(
			"Status value and ENTER/EXIT timing remain read-only until their paired typed source adapter is available.");
	}

	ImGui::SeparatorText("Stage Hit (Boss -> Player)");
	ImGui::TextDisabled(
		"Timing is either instantaneous Server pulses or one per-target Active Window. Effect geometry is presentation only and never becomes hit authority.");
	FocusDetailSection(bFocusCollider);
	const std::vector<std::string> DamageProfiles =
		m_pBalanceTool->Get_ValtanDamageProfileIds();
	if (!DamageProfiles.empty())
	{
		const auto CurrentProfile = std::find(
			DamageProfiles.begin(), DamageProfiles.end(), Draft.damageProfileId);
		if (CurrentProfile != DamageProfiles.end())
			m_iDamageProfileSelection = static_cast<int32_t>(
				CurrentProfile - DamageProfiles.begin());
		m_iDamageProfileSelection = (std::clamp)(
			m_iDamageProfileSelection, 0,
			static_cast<int32_t>(DamageProfiles.size() - 1u));
	}
	const bool_t bHasExistingServerHit = "NONE" != Draft.hitShape;
	const bool_t bColliderAddAdmitted = Draft.colliderAddAdmitted;
	if (!bHasExistingServerHit)
	{
		ImGui::TextDisabled(
			"No Server collider. A visible Effect never becomes hit authority by itself.");
		ImGui::BeginDisabled(!bColliderAddAdmitted);
		if (!DamageProfiles.empty())
		{
			ImGui::SetNextItemWidth(-1.f);
			if (ImGui::BeginCombo(
					"Damage Profile", DamageProfiles[
						static_cast<std::size_t>(m_iDamageProfileSelection)].c_str()))
			{
				for (std::size_t iProfile = 0u;
					iProfile < DamageProfiles.size(); ++iProfile)
				{
					const bool_t bSelected = iProfile ==
						static_cast<std::size_t>(m_iDamageProfileSelection);
					if (ImGui::Selectable(
							DamageProfiles[iProfile].c_str(), bSelected))
						m_iDamageProfileSelection = static_cast<int32_t>(iProfile);
				}
				ImGui::EndCombo();
			}
			if (ImGui::Button("Add Server Collider (BOX)"))
			{
				std::string ColliderStatus;
				if (InitializeDefaultValtanColliderBoxDraft(
						Draft, DamageProfiles, m_iDamageProfileSelection,
						ColliderStatus))
				{
					bChanged = true;
				}
				else
				{
					m_strStatus = std::move(ColliderStatus);
				}
			}
		}
		ImGui::EndDisabled();
		if (DamageProfiles.empty())
		{
			ImGui::TextDisabled(
				"New Collider Add is unavailable until one admitted damage.valtan. profile is present.");
		}
		else if (bColliderAddAdmitted)
		{
			ImGui::TextDisabled(
				"New Collider Add is admitted only because this is a non-WAIT manual audition Stage.");
		}
		else if ("WAIT" == Stage.strSequenceRole)
		{
			ImGui::TextDisabled(
				"New Collider Add is unavailable: WAIT remains a clock-only gap.");
		}
		else
		{
			ImGui::TextDisabled(
				"New Collider Add is unavailable: %s is %s and this Stage has no existing hit contract. Create or select a manual audition Stage instead.",
				Pattern.strPatternId.c_str(), Pattern.strAdmissionState.c_str());
		}
	}
	if (bHasExistingServerHit && Draft.colliderTuneAdmitted)
	{
		ImGui::TextDisabled(
			"Existing Server hit contract: tune its typed geometry/timing. Remove is a separate manual-audition admission.");
		const bool_t bDamageResponse = "DAMAGE" == Draft.playerResponse;
		const bool_t bCaptureBeforeEdit = "CAPTURE" == Draft.playerResponse;
		const bool_t bOwnsGrabReleasePath = std::any_of(
			Pattern.Stages.begin(), Pattern.Stages.end(),
			[](const VALTAN_STAGE_VIEW& CandidateStage)
			{
				return std::any_of(
					CandidateStage.Actions.begin(), CandidateStage.Actions.end(),
					[](const VALTAN_STAGE_ACTION_VIEW& Action)
					{ return "RELEASE_GRABBED_PLAYERS" == Action.strKind; });
			});
		if (bDamageResponse || bCaptureBeforeEdit)
		{
			const char_t* const ColliderTypes[] = { "Damage", "Grab (Capture)" };
			int32_t iColliderType = bCaptureBeforeEdit ? 1 : 0;
			ImGui::BeginDisabled(
				bCaptureBeforeEdit || (bDamageResponse && !bOwnsGrabReleasePath));
			if (ImGui::Combo(
					"Collider Type", &iColliderType, ColliderTypes,
					static_cast<int32_t>(std::size(ColliderTypes))))
			{
				if (1 == iColliderType)
				{
					Draft.playerResponse = "CAPTURE";
					Draft.attachmentSlot = "BOSS_LEFT_HAND";
					Draft.hasGripLocalOffset = true;
					Draft.gripForwardM = 0.0;
					Draft.gripUpM = -0.9;
					Draft.gripRightM = 0.0;
					Draft.pushRangeM = 0.0;
					Draft.pushMs = 0u;
					Draft.knockdown = false;
					Draft.downMs = 0u;
				}
				else
				{
					Draft.playerResponse = "DAMAGE";
					Draft.attachmentSlot = "NONE";
					Draft.hasGripLocalOffset = false;
					Draft.gripForwardM = 0.0;
					Draft.gripUpM = 0.0;
					Draft.gripRightM = 0.0;
				}
				bChanged = true;
			}
			ImGui::EndDisabled();
			if (bCaptureBeforeEdit)
			{
				ImGui::TextDisabled(
					"Grab is fixed here. Removing Capture requires the whole typed grab topology transaction.");
			}
			else if (!bOwnsGrabReleasePath)
			{
				ImGui::TextDisabled(
					"Grab (Capture) is unavailable: this Pattern has no RELEASE_GRABBED_PLAYERS action. Add the typed release topology first.");
			}
			else
			{
				ImGui::TextDisabled(
					"Grab is admitted because this Pattern owns a typed left-hand release path; a rejected Save keeps its exact reason.");
			}
		}
		else
		{
			ImGui::TextDisabled(
				"Unsupported typed Collider Type: %s / %s",
				Draft.playerResponse.c_str(), Draft.attachmentSlot.c_str());
		}
		if ("CAPTURE" == Draft.playerResponse)
		{
			ImGui::SeparatorText("Left-hand Grip");
			ImGui::TextDisabled(
				"Authored CAPTURE grip in metres. The Client composes it onto the owner's bip001-l-hand socket in the owner's yaw frame (forward/right) and world up for presentation only; the Server keeps its boss-local anchor for judgement, release and ejection.");
			constexpr double fMinimumGripOffset =
				-Client::CPlayerHandGripTransform::MAX_GRIP_OFFSET_COMPONENT_M;
			constexpr double fMaximumGripOffset =
				Client::CPlayerHandGripTransform::MAX_GRIP_OFFSET_COMPONENT_M;
			Draft.hasGripLocalOffset = true;
			bChanged |= ImGui::DragScalar(
				"Grip Forward (m)", ImGuiDataType_Double,
				&Draft.gripForwardM, 0.01f, &fMinimumGripOffset,
				&fMaximumGripOffset, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			bChanged |= ImGui::DragScalar(
				"Grip Up (m)", ImGuiDataType_Double,
				&Draft.gripUpM, 0.01f, &fMinimumGripOffset,
				&fMaximumGripOffset, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			bChanged |= ImGui::DragScalar(
				"Grip Right (m)", ImGuiDataType_Double,
				&Draft.gripRightM, 0.01f, &fMinimumGripOffset,
				&fMaximumGripOffset, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		}

		ImGui::SeparatorText("Shape / Size");
		const char_t* const HitShapes[] = {
			"CIRCLE", "RING", "CONE", "BOX", "CROSS", "SIX_DIRECTIONS" };
		int32_t iShape = 0;
		for (std::size_t iKind = 0u; iKind < std::size(HitShapes); ++iKind)
			if (Draft.hitShape == HitShapes[iKind])
				iShape = static_cast<int32_t>(iKind);
		if (ImGui::Combo("Collider Shape", &iShape, HitShapes,
				static_cast<int32_t>(std::size(HitShapes))))
		{
			Draft.hitShape = HitShapes[iShape];
			Draft.hitOuterRadius = 0.0;
			Draft.hitInnerRadius = 0.0;
			Draft.hitAngleDegrees = 0.0;
			Draft.hitLength = 0.0;
			Draft.hitHalfWidth = 0.0;
			if ("CIRCLE" == Draft.hitShape)
				Draft.hitOuterRadius = 5.0;
			else if ("RING" == Draft.hitShape)
			{
				Draft.hitInnerRadius = 2.0;
				Draft.hitOuterRadius = 5.0;
			}
			else if ("CONE" == Draft.hitShape)
			{
				Draft.hitAngleDegrees = 90.0;
				Draft.hitLength = 8.0;
			}
			else
			{
				Draft.hitLength = 8.0;
				Draft.hitHalfWidth = 2.0;
			}
			bChanged = true;
		}
		const bool_t bCaptureCollider = "CAPTURE" == Draft.playerResponse;
		constexpr double fMinimumColliderSize = 0.01;
		constexpr double fMaximumColliderSize = 1000.0;
		constexpr double fMaximumColliderAngle = 180.0;
		if ("CIRCLE" == Draft.hitShape || "RING" == Draft.hitShape)
		{
			const double fMinimumOuterRadius = "RING" == Draft.hitShape ?
				(std::min)(fMaximumColliderSize,
					Draft.hitInnerRadius + fMinimumColliderSize) :
				fMinimumColliderSize;
			bChanged |= ImGui::DragScalar(
				"Outer Radius (m)", ImGuiDataType_Double,
				&Draft.hitOuterRadius, 0.05f, &fMinimumOuterRadius,
				&fMaximumColliderSize, "%.2f",
				ImGuiSliderFlags_AlwaysClamp);
		}
		if ("RING" == Draft.hitShape)
		{
			const double fMaximumInnerRadius = (std::max)(
				fMinimumColliderSize,
				Draft.hitOuterRadius - fMinimumColliderSize);
			bChanged |= ImGui::DragScalar(
				"Inner Radius (m)", ImGuiDataType_Double,
				&Draft.hitInnerRadius, 0.05f, &fMinimumColliderSize,
				&fMaximumInnerRadius, "%.2f",
				ImGuiSliderFlags_AlwaysClamp);
		}
		if ("CONE" == Draft.hitShape)
		{
			bChanged |= ImGui::DragScalar(
				"Angle (deg)", ImGuiDataType_Double,
				&Draft.hitAngleDegrees, 0.5f, &fMinimumColliderSize,
				&fMaximumColliderAngle, "%.1f",
				ImGuiSliderFlags_AlwaysClamp);
		}
		if ("CONE" == Draft.hitShape || "BOX" == Draft.hitShape ||
			"CROSS" == Draft.hitShape || "SIX_DIRECTIONS" == Draft.hitShape)
		{
			bChanged |= ImGui::DragScalar(
				"Length (m)", ImGuiDataType_Double,
				&Draft.hitLength, 0.05f, &fMinimumColliderSize,
				&fMaximumColliderSize, "%.2f",
				ImGuiSliderFlags_AlwaysClamp);
		}
		if ("BOX" == Draft.hitShape || "CROSS" == Draft.hitShape ||
			"SIX_DIRECTIONS" == Draft.hitShape)
		{
			bChanged |= ImGui::DragScalar(
				"Half Width (m)", ImGuiDataType_Double,
				&Draft.hitHalfWidth, 0.05f, &fMinimumColliderSize,
				&fMaximumColliderSize, "%.2f",
				ImGuiSliderFlags_AlwaysClamp);
		}
		ImGui::TextDisabled(
			"Collider size is boss-local meters. The next Play/Restart restages its non-authoritative Arena Clone debug wire; Save writes these typed fields to the physical Pattern JSON.");

		ImGui::SeparatorText("Timing");
		ImGui::TextUnformatted("Timing Mode");
		int32_t iTimingMode = Draft.hasHitActivation ? 1 : 0;
		bool_t bTimingModeChanged = false;
		bTimingModeChanged |= ImGui::RadioButton(
			"Pulse Schedule", &iTimingMode, 0);
		ImGui::SameLine();
		bTimingModeChanged |= ImGui::RadioButton(
			"Active Window (lifetime)", &iTimingMode, 1);
		if (bTimingModeChanged)
		{
			const uint32_t iStageDurationMs = (std::max)(Draft.durationMs, 1u);
			if (1 == iTimingMode)
			{
				const uint32_t iRequestedStartMs = Draft.hitOffsetsMs.empty() ?
					Draft.hitDelayMs : Draft.hitOffsetsMs.front();
				Draft.hasHitActivation = true;
				Draft.hitActivationStartMs = (std::min)(
					iRequestedStartMs, iStageDurationMs - 1u);
				Draft.hitActivationLifetimeMs =
					iStageDurationMs - Draft.hitActivationStartMs;
				Draft.hitCount = 0u;
				Draft.hitIntervalMs = 0u;
				Draft.hitDelayMs = 0u;
				Draft.hitOffsetsMs.clear();
			}
			else
			{
				const uint32_t iPulseOffsetMs = (std::min)(
					Draft.hitActivationStartMs, iStageDurationMs - 1u);
				Draft.hasHitActivation = false;
				Draft.hitActivationStartMs = 0u;
				Draft.hitActivationLifetimeMs = 0u;
				Draft.hitCount = 1u;
				Draft.hitIntervalMs = 0u;
				Draft.hitDelayMs = iPulseOffsetMs;
				Draft.hitOffsetsMs.clear();
			}
			bChanged = true;
		}
		if (Draft.hasHitActivation)
		{
			const uint32_t iStageDurationMs = (std::max)(Draft.durationMs, 1u);
			const int32_t iMaximumStartMs = static_cast<int32_t>((std::min)(
				iStageDurationMs - 1u, static_cast<uint32_t>(INT32_MAX)));
			int32_t iActivationStartMs = static_cast<int32_t>((std::min)(
				Draft.hitActivationStartMs,
				static_cast<uint32_t>(INT32_MAX)));
			if (ImGui::DragInt(
					"Start (ms)", &iActivationStartMs, 5.f, 0,
					iMaximumStartMs, "%d ms", ImGuiSliderFlags_AlwaysClamp))
			{
				Draft.hitActivationStartMs =
					static_cast<uint32_t>(iActivationStartMs);
				Draft.hitActivationLifetimeMs = (std::clamp)(
					Draft.hitActivationLifetimeMs, 1u,
					iStageDurationMs - Draft.hitActivationStartMs);
				bChanged = true;
			}
			const uint32_t iMaximumLifetimeMs =
				iStageDurationMs - Draft.hitActivationStartMs;
			int32_t iActivationLifetimeMs = static_cast<int32_t>((std::min)(
				Draft.hitActivationLifetimeMs,
				static_cast<uint32_t>(INT32_MAX)));
			if (ImGui::DragInt(
					"Lifetime (ms)", &iActivationLifetimeMs, 5.f, 1,
					static_cast<int32_t>((std::min)(
						iMaximumLifetimeMs, static_cast<uint32_t>(INT32_MAX))),
					"%d ms", ImGuiSliderFlags_AlwaysClamp))
			{
				Draft.hitActivationLifetimeMs =
					static_cast<uint32_t>(iActivationLifetimeMs);
				bChanged = true;
			}
			ImGui::TextDisabled(
				"Each target may be admitted once while it overlaps this continuous Server window.");
		}
		else if (Draft.hitOffsetsMs.empty())
		{
			ImGui::TextDisabled(
				"Schedule: INTERVAL. Each scheduled time is an instantaneous Server pulse. There is no persistent overlap lifetime.");
			int32_t iHitDelay = static_cast<int32_t>(Draft.hitDelayMs);
			int32_t iHitInterval = static_cast<int32_t>(Draft.hitIntervalMs);
			int32_t iHitCount = static_cast<int32_t>(Draft.hitCount);
			if (ImGui::DragInt("Hit Delay (ms)", &iHitDelay, 5.f, 0, 120000))
			{
				Draft.hitDelayMs = static_cast<uint32_t>(iHitDelay);
				bChanged = true;
			}
			if (ImGui::DragInt("Hit Interval (ms)", &iHitInterval, 5.f, 0, 120000))
			{
				Draft.hitIntervalMs = static_cast<uint32_t>(iHitInterval);
				bChanged = true;
			}
			if (ImGui::DragInt("Hit Count", &iHitCount, 0.1f, 1, 64))
			{
				Draft.hitCount = static_cast<uint32_t>(iHitCount);
				if (1u == Draft.hitCount)
					Draft.hitIntervalMs = 0u;
				bChanged = true;
			}
		}
		else
		{
			ImGui::TextDisabled(
				"Schedule: EXPLICIT_OFFSETS. Times are stored strictly increasing and below the Stage duration; changing a time reorders it chronologically.");
			const uint32_t iMaximumOffsetMs = (std::min)(
				Draft.durationMs - 1u, static_cast<uint32_t>(INT32_MAX));
			bool_t bOffsetListMutated = false;
			for (std::size_t iOffset = 0u;
				iOffset < Draft.hitOffsetsMs.size(); ++iOffset)
			{
				ImGui::PushID(static_cast<int32_t>(iOffset));
				ImGui::Text("Pulse %zu", iOffset + 1u);
				ImGui::SameLine();
				ImGui::SetNextItemWidth(150.f);
				int32_t iEditedOffset = static_cast<int32_t>(
					Draft.hitOffsetsMs[iOffset]);
				if (ImGui::DragInt(
						"##ExplicitHitOffsetMs", &iEditedOffset, 5.f, 0,
						static_cast<int32_t>(iMaximumOffsetMs), "%d ms",
						ImGuiSliderFlags_AlwaysClamp))
				{
					const uint32_t iCandidate = static_cast<uint32_t>(iEditedOffset);
					if (iCandidate != Draft.hitOffsetsMs[iOffset])
					{
						std::vector<uint32_t> Ordered = Draft.hitOffsetsMs;
						Ordered.erase(Ordered.begin() + iOffset);
						const auto InsertAt = std::lower_bound(
							Ordered.begin(), Ordered.end(), iCandidate);
						if (InsertAt != Ordered.end() && *InsertAt == iCandidate)
						{
							m_strStatus =
								"Explicit hit offsets must be unique and strictly increasing.";
						}
						else
						{
							Ordered.insert(InsertAt, iCandidate);
							Draft.hitOffsetsMs = std::move(Ordered);
							Draft.hitCount = static_cast<uint32_t>(
								Draft.hitOffsetsMs.size());
							Draft.hitDelayMs = 0u;
							Draft.hitIntervalMs = 0u;
							bChanged = true;
							bOffsetListMutated = true;
						}
					}
				}
				if (bOffsetListMutated)
				{
					ImGui::PopID();
					break;
				}
				ImGui::SameLine();
				ImGui::BeginDisabled(Draft.hitOffsetsMs.size() <= 1u);
				if (ImGui::SmallButton("Delete"))
				{
					Draft.hitOffsetsMs.erase(
						Draft.hitOffsetsMs.begin() + iOffset);
					Draft.hitCount = static_cast<uint32_t>(
						Draft.hitOffsetsMs.size());
					Draft.hitDelayMs = 0u;
					Draft.hitIntervalMs = 0u;
					bChanged = true;
					bOffsetListMutated = true;
				}
				ImGui::EndDisabled();
				ImGui::PopID();
				if (bOffsetListMutated)
					break;
			}
			const bool_t bCanAddOffset =
				Draft.hitOffsetsMs.size() < 64u &&
				Draft.hitOffsetsMs.back() < iMaximumOffsetMs;
			ImGui::BeginDisabled(!bCanAddOffset);
			if (ImGui::SmallButton("Add Hit Offset"))
			{
				const uint32_t iPreviousOffset = Draft.hitOffsetsMs.back();
				const uint32_t iSuggestedStep = Draft.hitOffsetsMs.size() > 1u ?
					(std::max)(1u, iPreviousOffset -
						Draft.hitOffsetsMs[Draft.hitOffsetsMs.size() - 2u]) : 100u;
				Draft.hitOffsetsMs.push_back((std::min)(
					iMaximumOffsetMs, iPreviousOffset + iSuggestedStep));
				Draft.hitCount = static_cast<uint32_t>(Draft.hitOffsetsMs.size());
				Draft.hitDelayMs = 0u;
				Draft.hitIntervalMs = 0u;
				bChanged = true;
			}
			ImGui::EndDisabled();
			if (!bCanAddOffset)
				ImGui::TextDisabled(
					"No later unique offset fits this Stage, or the 64-pulse limit is reached.");
		}

		if ("DAMAGE" == Draft.playerResponse)
		{
			ImGui::SeparatorText("Damage");
			if (!DamageProfiles.empty())
			{
				ImGui::SetNextItemWidth(-1.f);
				if (ImGui::BeginCombo(
						"Damage Profile", Draft.damageProfileId.c_str()))
				{
					for (std::size_t iProfile = 0u;
						iProfile < DamageProfiles.size(); ++iProfile)
					{
						const bool_t bSelected =
							Draft.damageProfileId == DamageProfiles[iProfile];
						if (ImGui::Selectable(
								DamageProfiles[iProfile].c_str(), bSelected))
						{
							Draft.damageProfileId = DamageProfiles[iProfile];
							m_iDamageProfileSelection =
								static_cast<int32_t>(iProfile);
							bChanged = true;
						}
						if (bSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}
			else
			{
				ImGui::TextDisabled(
					"No admitted damage.valtan. profile is available.");
			}

			uint32_t iDamageRatePercent = 0u;
			std::string DamageRateStatus;
			const bool_t bDamageRateReady =
				!Draft.damageProfileId.empty() &&
				m_pBalanceTool->Get_ValtanDamageRateDraft(
					Draft.damageProfileId, iDamageRatePercent,
					DamageRateStatus);
			if (bDamageRateReady)
			{
				int32_t iDamageRate = static_cast<int32_t>((std::clamp)(
					iDamageRatePercent, 1u, 100000u));
				if (ImGui::DragInt(
						"Damage Rate (%)", &iDamageRate, 1.f, 1, 100000,
						"%d %%", ImGuiSliderFlags_AlwaysClamp))
				{
					std::string SetDamageRateStatus;
					if (m_pBalanceTool->Set_ValtanDamageRateDraft(
							Draft.damageProfileId,
							static_cast<uint32_t>(iDamageRate),
							SetDamageRateStatus))
					{
						iDamageRatePercent = static_cast<uint32_t>(iDamageRate);
						m_strStatus = std::move(SetDamageRateStatus);
					}
					else
					{
						m_strStatus = std::move(SetDamageRateStatus);
					}
				}

				std::size_t iSharedStageUsers = 0u;
				std::string StageUserCountStatus;
				if (m_pBalanceTool->Get_ValtanDamageProfileStageUserCountDraft(
						Draft.damageProfileId, iSharedStageUsers,
						StageUserCountStatus))
				{
					ImGui::TextDisabled(
						"Shared by %zu Valtan Stage(s). Editing the rate changes every user in this Save transaction.",
						iSharedStageUsers);
				}
				else
				{
					ImGui::TextDisabled(
						"Shared Stage count is unavailable: %s",
						StageUserCountStatus.c_str());
				}

				uint32_t iBossAttackPower = 0u;
				std::string AttackPowerStatus;
				if (m_pBalanceTool->Get_ValtanBossAttackPowerDraft(
						iBossAttackPower, AttackPowerStatus))
				{
					const uint64_t iRawDamageWide =
						static_cast<uint64_t>(iBossAttackPower) *
						iDamageRatePercent / 100u;
					const uint32_t iRawDamage =
						0u == iBossAttackPower || 0u == iDamageRatePercent ? 0u :
						static_cast<uint32_t>((std::min)(
							(std::max)(iRawDamageWide, uint64_t{ 1u }),
							static_cast<uint64_t>(
								(std::numeric_limits<uint32_t>::max)())));
					ImGui::Text(
						"Raw damage before defense: %u x %u%% / 100 = %u",
						iBossAttackPower, iDamageRatePercent, iRawDamage);
				}
				else
				{
					ImGui::TextDisabled(
						"Raw damage before defense is unavailable: %s",
						AttackPowerStatus.c_str());
				}
			}
			else if (!DamageRateStatus.empty())
			{
				ImGui::TextDisabled(
					"Damage rate is unavailable: %s",
					DamageRateStatus.c_str());
			}
		}

		ImGui::BeginDisabled(!Draft.colliderRemoveAdmitted);
		if (ImGui::Button("Remove Server Collider"))
		{
			Draft.hitShape = "NONE";
			Draft.hitOuterRadius = Draft.hitInnerRadius = 0.0;
			Draft.hitAngleDegrees = Draft.hitLength = Draft.hitHalfWidth = 0.0;
			Draft.hitCount = Draft.hitIntervalMs = Draft.hitDelayMs = 0u;
			Draft.hitOffsetsMs.clear();
			Draft.hasHitAnchor = false;
			Draft.hitAnchorKind = "BOSS_CURRENT";
			Draft.hitAnchorForwardOffsetM = 0.0;
			Draft.hitAnchorRightOffsetM = 0.0;
			Draft.hitAnchorYawOffsetDegrees = 0.0;
			Draft.hasHitActivation = false;
			Draft.hitActivationStartMs = 0u;
			Draft.hitActivationLifetimeMs = 0u;
			Draft.damageProfileId.clear();
			Draft.pushRangeM = 0.0;
			Draft.pushMs = 0u;
			Draft.knockdown = false;
			Draft.downMs = 0u;
			Draft.playerResponse = "DAMAGE";
			Draft.attachmentSlot = "NONE";
			bChanged = true;
		}
		ImGui::EndDisabled();
		if (bCaptureCollider)
		{
			ImGui::TextDisabled(
				"Capture collider removal is blocked: grab branches, left-hand attachment, and release actions form one typed transaction.");
		}
		else if (!Draft.colliderRemoveAdmitted)
		{
			ImGui::TextDisabled(
				"Remove is unavailable: %s is %s. Tune this existing collider in place, or create/select a manual audition Stage for add/remove topology authoring.",
				Pattern.strPatternId.c_str(), Pattern.strAdmissionState.c_str());
		}
	}
	else if (bHasExistingServerHit)
	{
		ImGui::TextDisabled(
			"This existing Server hit contract is not editable: %s is %s. Reload Canonical or select an admitted Pattern draft.",
			Pattern.strPatternId.c_str(), Pattern.strAdmissionState.c_str());
	}

	ImGui::SeparatorText("Player Reaction / Grab Release");
	FocusDetailSection(bFocusAction);
	ImGui::BeginDisabled("NONE" == Draft.hitShape ||
		"CAPTURE" == Draft.playerResponse);
	if (ImGui::DragScalar(
			"Push Range (m)", ImGuiDataType_Double,
			&Draft.pushRangeM, 0.05f, nullptr, nullptr, "%.2f"))
	{
		if (0.0 == Draft.pushRangeM)
			Draft.pushMs = 0u;
		else if (0u == Draft.pushMs)
			Draft.pushMs = 500u;
		bChanged = true;
	}
	int32_t iPushMs = static_cast<int32_t>(Draft.pushMs);
	if (ImGui::DragInt("Push Duration (ms)", &iPushMs, 5.f, 0, 10000))
	{
		Draft.pushMs = static_cast<uint32_t>(iPushMs);
		if (0u == Draft.pushMs)
			Draft.pushRangeM = 0.0;
		else if (0.0 == Draft.pushRangeM)
			Draft.pushRangeM = 1.0;
		bChanged = true;
	}
	if (ImGui::Checkbox("Knockdown", &Draft.knockdown))
	{
		Draft.downMs = Draft.knockdown ?
			(std::max)(Draft.downMs, 1000u) : 0u;
		bChanged = true;
	}
	int32_t iDownMs = static_cast<int32_t>(Draft.downMs);
	if (ImGui::DragInt("Down Duration (ms)", &iDownMs, 5.f, 0, 30000))
	{
		Draft.downMs = static_cast<uint32_t>(iDownMs);
		Draft.knockdown = 0u != Draft.downMs;
		bChanged = true;
	}
	ImGui::EndDisabled();
	for (VALTAN_STAGE_ACTION_VIEW& Action : Draft.actions)
	{
		if ("RELEASE_GRABBED_PLAYERS" != Action.strKind)
			continue;
		ImGui::PushID(Action.strTargetId.c_str());
		ImGui::TextDisabled("Release trigger %s | target %s",
			Action.strTrigger.c_str(), Action.strTargetId.c_str());
		const char_t* const ReleaseModes[] = {
			"HOLD", "OPPOSITE_KNOCKBACK", "ARENA_EJECTION" };
		int32_t iReleaseMode = 0;
		for (std::size_t iMode = 0u; iMode < std::size(ReleaseModes); ++iMode)
		{
			if (Action.strReleaseMode == ReleaseModes[iMode])
				iReleaseMode = static_cast<int32_t>(iMode);
		}
		if (ImGui::Combo(
				"Release Mode", &iReleaseMode, ReleaseModes,
				static_cast<int32_t>(std::size(ReleaseModes))))
		{
			Action.strReleaseMode = ReleaseModes[iReleaseMode];
			if ("HOLD" == Action.strReleaseMode)
			{
				Action.fSpeedMps = 0.f;
				Action.iDurationMs = 0u;
				Action.fYawOffsetDegrees = 0.f;
			}
			else
			{
				if (Action.fSpeedMps <= 0.f)
					Action.fSpeedMps = 10.f;
				if (0u == Action.iDurationMs)
					Action.iDurationMs = 500u;
				if ("OPPOSITE_KNOCKBACK" == Action.strReleaseMode)
					Action.fYawOffsetDegrees = 0.f;
			}
			bChanged = true;
		}
		const bool_t bHoldRelease = "HOLD" == Action.strReleaseMode;
		const bool_t bArenaEjection =
			"ARENA_EJECTION" == Action.strReleaseMode;
		ImGui::BeginDisabled(bHoldRelease);
		bChanged |= ImGui::DragFloat(
			"Release Velocity (m/s)", &Action.fSpeedMps, 0.05f, 0.f, 50.f,
			"%.2f", ImGuiSliderFlags_AlwaysClamp);
		int32_t iReleaseDuration = static_cast<int32_t>(Action.iDurationMs);
		if (ImGui::DragInt(
				"Release Duration (ms)", &iReleaseDuration, 5.f, 0, 5000))
		{
			Action.iDurationMs = static_cast<uint32_t>(iReleaseDuration);
			bChanged = true;
		}
		ImGui::EndDisabled();
		ImGui::BeginDisabled(!bArenaEjection);
		bChanged |= ImGui::DragFloat(
			"Release Rotation / Yaw Offset (deg)",
			&Action.fYawOffsetDegrees, 0.5f, -180.f, 180.f, "%.1f",
			ImGuiSliderFlags_AlwaysClamp);
		ImGui::EndDisabled();
		if (bHoldRelease)
			ImGui::TextDisabled(
				"HOLD owns no launch velocity, duration, or rotation.");
		else if (!bArenaEjection)
			ImGui::TextDisabled(
				"OPPOSITE_KNOCKBACK derives direction from the boss/player relation; yaw is fixed at 0. Use ARENA_EJECTION for an authored rotation offset.");
		ImGui::PopID();
	}

	if (Draft.portalRushMotionEditable)
		ImGui::TextDisabled(
			"This selected leg's portal motion is owned by Warp Rush - All 8 Legs above.");

	if (SelectedBranch != Stage.Branches.end() && bFocusNonCounterBranch)
	{
		ImGui::SeparatorText("Selected Logic Branch");
		FocusDetailSection(true);
		ImGui::Text("Outcome: %s", SelectedBranch->strOutcome.c_str());
		ImGui::Text("Target: %s",
			BuildBranchTargetLabel(*SelectedBranch).c_str());
		std::vector<CBalanceTool::VALTAN_STAGE_BRANCH_EDIT> BranchDrafts;
		std::vector<std::string> TargetActionIds;
		std::string BranchStatus;
		const bool_t bBranchDraftReady = bMutationAdmitted &&
			nullptr != m_pBalanceTool &&
			m_pBalanceTool->Get_ValtanStageBranchDrafts(
				Pattern.strPatternId, Stage.strStageId, BranchDrafts,
				TargetActionIds, BranchStatus);
		const auto BranchDraft = std::find_if(
			BranchDrafts.begin(), BranchDrafts.end(),
			[&SelectedBranch](
				const CBalanceTool::VALTAN_STAGE_BRANCH_EDIT& Candidate)
			{ return Candidate.outcome == SelectedBranch->strOutcome; });
		if (!bBranchDraftReady || BranchDrafts.end() == BranchDraft)
		{
			ImGui::TextDisabled("%s", BranchStatus.empty() ?
				"Branch retarget requires an admitted, mutable Pattern draft." :
				BranchStatus.c_str());
		}
		else if (BranchDraft->crossPattern)
		{
			ImGui::TextDisabled(
				"Cross-pattern follow-up branches are read-only here.");
		}
		else if ("TIMEOUT" == BranchDraft->outcome)
		{
			ImGui::TextDisabled(
				"TIMEOUT is owned by the typed Counter or Stage topology editor.");
		}
		else
		{
			/* 목적지만 바꾼다. 가능한 outcome 집합은 Server가 소유한다.
			   순환이나 없는 목적지 참조는 Save 검증에서 거절한다. */
			const std::string strCurrentTarget = BranchDraft->nextActionId.empty() ?
				"(pattern end)" : BranchDraft->nextActionId;
			ImGui::SetNextItemWidth(-1.f);
			if (ImGui::BeginCombo("##LogicBranchTarget", strCurrentTarget.c_str()))
			{
				auto Retarget = [&](const std::string& strTarget)
				{
					CBalanceTool::VALTAN_STAGE_BRANCH_EDIT Edit = *BranchDraft;
					Edit.nextActionId = strTarget;
					std::string RetargetStatus;
					m_strStatus = m_pBalanceTool->Set_ValtanStageBranchDraft(
						Pattern.strPatternId, Stage.strStageId, Edit,
						RetargetStatus) ?
						RetargetStatus : "Branch retarget rejected: " + RetargetStatus;
					Invalidate_TimelineCache();
				};
				if (ImGui::Selectable("(pattern end)",
						BranchDraft->nextActionId.empty()))
				{
					Retarget(std::string());
				}
				for (const std::string& strTargetActionId : TargetActionIds)
				{
					if (ImGui::Selectable(strTargetActionId.c_str(),
							strTargetActionId == BranchDraft->nextActionId))
					{
						Retarget(strTargetActionId);
					}
				}
				ImGui::EndCombo();
			}
			ImGui::TextDisabled(
				"Counter authoring uses the typed COUNTER_HIT editor below.");
		}
	}

	ImGui::SeparatorText("Counter Hurt Proxy (Player -> Boss)");
	ImGui::TextDisabled(
		"BOSS_LOCAL circle checked against player attack sources for the whole WINDUP Stage. It never damages or captures a player.");
	FocusDetailSection(bFocusCounterBranch);
	const bool_t bWaitStage = "WAIT" == Stage.strSequenceRole;
	const bool_t bCounterSourceEditable =
		!bWaitStage && "WINDUP" == Stage.strStageKind;
	ImGui::BeginDisabled(!bCounterSourceEditable);
	CBalanceTool::VALTAN_COUNTER_WINDOW_EDIT Counter;
	std::string CounterStatus;
	if (m_pBalanceTool->Get_ValtanCounterWindowDraft(
			Pattern.strPatternId, Stage.strStageId, Counter, CounterStatus))
	{
		std::vector<const VALTAN_STAGE_VIEW*> SuccessTargets;
		std::vector<const VALTAN_STAGE_VIEW*> TimeoutTargets;
		const auto CurrentStage = std::find_if(
			Pattern.Stages.begin(), Pattern.Stages.end(),
			[&Stage](const VALTAN_STAGE_VIEW& Candidate)
			{ return Candidate.strStageId == Stage.strStageId; });
		const std::size_t iCurrentStageIndex =
			CurrentStage == Pattern.Stages.end() ? Pattern.Stages.size() :
			static_cast<std::size_t>(
				std::distance(Pattern.Stages.begin(), CurrentStage));
		for (std::size_t iCandidate = iCurrentStageIndex + 1u;
			iCandidate < Pattern.Stages.size(); ++iCandidate)
		{
			const VALTAN_STAGE_VIEW& Candidate = Pattern.Stages[iCandidate];
			TimeoutTargets.push_back(&Candidate);
			if ("WINDUP" == Candidate.strStageKind ||
				"GROGGY" == Candidate.strStageKind ||
				"RECOVERY" == Candidate.strStageKind)
				SuccessTargets.push_back(&Candidate);
		}
		ImGui::BeginDisabled(!Counter.enabled &&
			(SuccessTargets.empty() || TimeoutTargets.empty()));
		bool_t bCounterChanged = ImGui::Checkbox("Counter Enabled", &Counter.enabled);
		ImGui::EndDisabled();
		if (Counter.enabled && (Counter.successStageId.empty() ||
			Counter.successActionId.empty()) && !SuccessTargets.empty())
		{
			Counter.successStageId = SuccessTargets.front()->strStageId;
			Counter.successActionId = SuccessTargets.front()->strActionId;
		}
		if (Counter.enabled && (Counter.timeoutStageId.empty() ||
			Counter.timeoutActionId.empty()) && !TimeoutTargets.empty())
		{
			Counter.timeoutStageId = TimeoutTargets.front()->strStageId;
			Counter.timeoutActionId = TimeoutTargets.front()->strActionId;
		}
		ImGui::BeginDisabled(!Counter.enabled || SuccessTargets.empty());
		const std::string SuccessPreview = Counter.successStageId.empty() ?
			std::string("Select Success Stage") :
			Counter.successStageId + " / " + Counter.successActionId;
		if (ImGui::BeginCombo("Counter Success Stage", SuccessPreview.c_str()))
		{
			for (const VALTAN_STAGE_VIEW* const Candidate : SuccessTargets)
			{
				const bool_t bSelected =
					Candidate->strStageId == Counter.successStageId;
				const std::string Label = Candidate->strStageKind + " | " +
					Candidate->strStageId + " / " + Candidate->strActionId;
				if (ImGui::Selectable(Label.c_str(), bSelected))
				{
					Counter.successStageId = Candidate->strStageId;
					Counter.successActionId = Candidate->strActionId;
					bCounterChanged = true;
				}
				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::EndDisabled();
		ImGui::BeginDisabled(!Counter.enabled || TimeoutTargets.empty());
		const std::string TimeoutPreview = Counter.timeoutStageId.empty() ?
			std::string("Select Timeout Stage") :
			Counter.timeoutStageId + " / " + Counter.timeoutActionId;
		if (ImGui::BeginCombo("Counter Timeout Stage", TimeoutPreview.c_str()))
		{
			for (const VALTAN_STAGE_VIEW* const Candidate : TimeoutTargets)
			{
				const bool_t bSelected =
					Candidate->strStageId == Counter.timeoutStageId;
				const std::string Label = Candidate->strStageKind + " | " +
					Candidate->strStageId + " / " + Candidate->strActionId;
				if (ImGui::Selectable(Label.c_str(), bSelected))
				{
					Counter.timeoutStageId = Candidate->strStageId;
					Counter.timeoutActionId = Candidate->strActionId;
					bCounterChanged = true;
				}
				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::EndDisabled();
		if (Counter.enabled &&
			(SuccessTargets.empty() || TimeoutTargets.empty()))
		{
			Counter.enabled = false;
			bCounterChanged = false;
			ImGui::TextColored(ImVec4(1.f, 0.55f, 0.25f, 1.f),
				"Add a later same-Pattern Stage before enabling Counter.");
		}
		if (bCounterChanged)
		{
			std::string Status;
			if (!m_pBalanceTool->Set_ValtanCounterWindowDraft(
					Pattern.strPatternId, Stage.strStageId, Counter, Status))
				m_strStatus = std::move(Status);
			else
			{
				m_strStatus = std::move(Status);
				Invalidate_TimelineCache();
			}
		}
		if (Counter.enabled)
		{
			ImGui::SeparatorText("Counter Hurt Proxy Box");
			ImGui::TextDisabled(
				"Lifetime: whole WINDUP Stage. The Logic box width is that Stage clock; it is not a persistent Stage Hit collider.");
			int32_t iCounterDurationMs = static_cast<int32_t>((std::min)(
				Draft.durationMs, static_cast<uint32_t>(INT32_MAX)));
			if (ImGui::DragInt(
					"Counter Window (ms)", &iCounterDurationMs, 10.f,
					1, 120000, "%d ms", ImGuiSliderFlags_AlwaysClamp))
			{
				std::string ClockStatus;
				if (ApplyStageClockPolicy(
						Draft, static_cast<uint32_t>(iCounterDurationMs),
						ClockStatus))
				{
					bChanged = true;
				}
				else
				{
					m_strStatus = std::move(ClockStatus);
				}
			}

			CBalanceTool::VALTAN_COUNTER_PROXY_EDIT Proxy;
			std::string ProxyStatus;
			if (m_pBalanceTool->Get_ValtanCounterProxyDraft(
					Pattern.strPatternId, Stage.strStageId, Proxy, ProxyStatus))
			{
				bool_t bProxyChanged = false;
				bProxyChanged |= ImGui::DragFloat(
					"Area Radius (m)", &Proxy.radiusM, 0.05f,
					0.1f, 20.f, "%.2f m", ImGuiSliderFlags_AlwaysClamp);
				bProxyChanged |= ImGui::DragFloat(
					"Area Forward Offset (m)", &Proxy.forwardOffsetM, 0.05f,
					-20.f, 20.f, "%.2f m", ImGuiSliderFlags_AlwaysClamp);
				bProxyChanged |= ImGui::DragFloat(
					"Area Right Offset (m)", &Proxy.rightOffsetM, 0.05f,
					-20.f, 20.f, "%.2f m", ImGuiSliderFlags_AlwaysClamp);
				if (bProxyChanged)
				{
					Proxy.exists = true;
					if (m_pBalanceTool->Set_ValtanCounterProxyDraft(
							Pattern.strPatternId, Stage.strStageId,
							Proxy, ProxyStatus))
					{
						Invalidate_TimelineCache();
					}
					m_strStatus = std::move(ProxyStatus);
				}
				if (!Proxy.exists)
					ImGui::TextDisabled(
						"Editing an area value creates the default boss-local Counter area.");
			}
			else
			{
				ImGui::TextDisabled("Counter area unavailable: %s",
					ProxyStatus.c_str());
			}
		}
	}
	else
	{
		ImGui::TextDisabled("No typed Counter/Groggy edge on this Stage: %s",
			CounterStatus.c_str());
	}
	ImGui::EndDisabled();
	if (!bCounterSourceEditable)
	{
		ImGui::TextDisabled(
			bWaitStage ?
				"WAIT is a clock-only gap. Select or insert a WINDUP Stage to author Counter -> GROGGY." :
				"Counter is authored only on a WINDUP Stage and must target a later GROGGY Stage in the same Pattern.");
	}

	if (bChanged && !bWarpRushAppliedThisFrame)
	{
		std::string Status;
		const bool_t bColliderChanged =
			!SameValtanColliderDraft(ColliderBaseline, Draft);
		if (SetValtanStageDraftWithSoundDependencyAdmission(
				m_pAnimationTool, m_pBalanceTool,
				m_bPatternShakesReady ? &m_PatternShakes : nullptr,
				Pattern, Stage, Draft, Status))
		{
			if (bColliderChanged)
			{
				Status +=
					" Collider geometry/timing is staged in the Pattern draft; Play/Restart restages the boss-local debug wire.";
			}
			m_strStatus = std::move(Status);
			Invalidate_TimelineCache();
		}
		else
		{
			m_strStatus = std::move(Status);
		}
	}
	ImGui::EndDisabled();
}

// Stage Animation 목록과 편집 입력을 표시한다. Pattern/Stage는 현재 pane 호출 동안 차용하고 기존 typed draft 명령으로 변경을 제출한다.
void Client::CValtanActionWorkbench::Render_AnimationStageDetails(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const bool_t bMutationAdmitted)
{
	if (VALTAN_VIEW_ADMISSION::ADMITTED != m_eAdmission)
	{
		ImGui::TextWrapped(
			"Pinned canonical Animation generation (read only). Reload Canonical before viewing or editing a newer authoring draft.");
		ImGui::Text("End Policy: %s | Stage %u ms",
			Stage.strAnimationEndPolicy.c_str(), Stage.iDurationMs);
		if (Stage.ClipOccurrences.empty())
		{
			ImGui::TextDisabled("Animation Mode: NONE");
			return;
		}
		for (std::size_t iSlot = 0u;
			iSlot < Stage.ClipOccurrences.size(); ++iSlot)
		{
			const VALTAN_CLIP_OCCURRENCE_VIEW& Slot =
				Stage.ClipOccurrences[iSlot];
			ImGui::PushID(static_cast<int32_t>(iSlot));
			const bool_t bSelected =
				Slot.strClipOccurrenceId == m_strSelectedStableId;
			const std::string Label = "Slot " + std::to_string(iSlot + 1u) +
				" | " + Slot.strClipName;
			if (ImGui::Selectable(Label.c_str(), bSelected))
				m_strSelectedStableId = Slot.strClipOccurrenceId;
			ImGui::TextDisabled("%s", Slot.strClipOccurrenceId.c_str());
			ImGui::Text("Source %u ms | play %u ms | rate %.3f | loop %s",
				Slot.iSourceStartMs, Slot.iPlayMs, Slot.fPlayRate,
				Slot.bLoop ? "true" : "false");
			ImGui::PopID();
		}
		return;
	}
	if (nullptr == m_pBalanceTool)
	{
		ImGui::TextDisabled("Animation authoring owner is unavailable.");
		return;
	}
	CBalanceTool::PATTERN_STAGE_EDIT Draft;
	std::string Status;
	if (!m_pBalanceTool->Get_ValtanStageDraft(
			Pattern.strPatternId, Stage.strStageId, Draft, Status))
	{
		ImGui::TextWrapped("Animation slot draft unavailable: %s", Status.c_str());
		return;
	}
	ImGui::TextWrapped(
		"Slots are saved in Data/Valtan/Valtan.presentation.json through the same immutable authoring revision as the Stage clock. Generated patternbindings remains read-only.");
	ImGui::BeginDisabled(!bMutationAdmitted || !Draft.animationEditable);
	bool_t bChanged = false;
	if (Draft.animationSlots.empty())
	{
		ImGui::TextDisabled("Animation Mode: NONE | Stage clock and gameplay remain active.");
	}
	else
	{
		const char_t* const EndPolicies[] = {
			"EXACT", "HOLD_LAST_POSE", "LOOP_TO_STAGE_END" };
		int32_t iEndPolicy = "HOLD_LAST_POSE" == Draft.animationEndPolicy ? 1 :
			("LOOP_TO_STAGE_END" == Draft.animationEndPolicy ? 2 : 0);
		if (ImGui::Combo("End Policy", &iEndPolicy, EndPolicies,
				static_cast<int32_t>(std::size(EndPolicies))))
		{
			Draft.animationEndPolicy = EndPolicies[iEndPolicy];
			bChanged = true;
		}
	}

	std::size_t iRemove = Draft.animationSlots.size();
	std::size_t iMoveUp = Draft.animationSlots.size();
	std::size_t iMoveDown = Draft.animationSlots.size();
	std::size_t iDuplicate = Draft.animationSlots.size();
	for (std::size_t iSlot = 0u; iSlot < Draft.animationSlots.size(); ++iSlot)
	{
		CBalanceTool::ANIMATION_SLOT_EDIT& Slot = Draft.animationSlots[iSlot];
		ImGui::PushID(static_cast<int32_t>(iSlot));
		const bool_t bSelected = Slot.clipOccurrenceId == m_strSelectedStableId;
		const std::string Header = "Slot " + std::to_string(iSlot + 1u) +
			" | " + Slot.clip;
		if (ImGui::Selectable(Header.c_str(), bSelected))
		{
			m_strSelectedStableId = Slot.clipOccurrenceId;
			m_eDetailOwner = DETAIL_OWNER::ANIMATION;
		}
		ImGui::TextDisabled("%s", Slot.clipOccurrenceId.c_str());
		int32_t iSourceStartMs = static_cast<int32_t>(Slot.sourceStartMs);
		int32_t iPlayMs = static_cast<int32_t>(Slot.playMs);
		float fPlayRate = static_cast<float>(Slot.playRate);
		(void)ImGui::InputInt("Source Start (ms)", &iSourceStartMs, 5, 100);
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			Slot.sourceStartMs = static_cast<uint32_t>((std::clamp)(
				iSourceStartMs, 0, 600000));
			bChanged = true;
		}
		(void)ImGui::InputInt("Source Duration (ms)", &iPlayMs, 5, 100);
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			Slot.playMs = static_cast<uint32_t>((std::clamp)(
				iPlayMs, 0, 600000));
			bChanged = true;
		}
		(void)ImGui::InputFloat(
			"Play Rate", &fPlayRate, 0.01f, 0.1f, "%.3f");
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			Slot.playRate = (std::clamp)(fPlayRate, 0.01f, 16.f);
			bChanged = true;
		}
		if (ImGui::Checkbox("Loop to Stage End", &Slot.repeatUntilStageEnd))
		{
			if (Slot.repeatUntilStageEnd)
			{
				Slot.playMs = 0u;
				Draft.animationEndPolicy = "LOOP_TO_STAGE_END";
			}
			bChanged = true;
		}
		if (ImGui::SmallButton("Move Up")) iMoveUp = iSlot;
		ImGui::SameLine();
		if (ImGui::SmallButton("Move Down")) iMoveDown = iSlot;
		ImGui::SameLine();
		if (ImGui::SmallButton("Duplicate Clip")) iDuplicate = iSlot;
		ImGui::SameLine();
		if (ImGui::SmallButton("Remove Slot")) iRemove = iSlot;
		ImGui::Separator();
		ImGui::PopID();
	}
	if (iDuplicate < Draft.animationSlots.size())
	{
		CBalanceTool::ANIMATION_SLOT_EDIT Duplicate =
			Draft.animationSlots[iDuplicate];
		Duplicate.clipOccurrenceId = BuildNextCompositionSlotId(
			Pattern.strPatternId, Stage.strStageId, Draft.animationSlots);
		if (Duplicate.clipOccurrenceId.empty())
		{
			m_strStatus =
				"Duplicate Clip rejected: no free stable occurrence ID is available.";
		}
		else
		{
			Draft.animationSlots.insert(
				Draft.animationSlots.begin() +
					static_cast<std::ptrdiff_t>(iDuplicate + 1u),
				Duplicate);
			m_strSelectedStableId = Duplicate.clipOccurrenceId;
			bChanged = true;
		}
	}
	else if (iRemove < Draft.animationSlots.size())
	{
		const std::string RemovedOccurrenceId =
			Draft.animationSlots[iRemove].clipOccurrenceId;
		Draft.animationSlots.erase(Draft.animationSlots.begin() +
			static_cast<std::ptrdiff_t>(iRemove));
		if (m_strSelectedStableId == RemovedOccurrenceId)
		{
			m_strSelectedStableId = Draft.animationSlots.empty() ?
				Stage.strStageId : Draft.animationSlots.front().clipOccurrenceId;
		}
		bChanged = true;
	}
	else if (iMoveUp > 0u && iMoveUp < Draft.animationSlots.size())
	{
		std::swap(Draft.animationSlots[iMoveUp],
			Draft.animationSlots[iMoveUp - 1u]);
		bChanged = true;
	}
	else if (iMoveDown + 1u < Draft.animationSlots.size())
	{
		std::swap(Draft.animationSlots[iMoveDown],
			Draft.animationSlots[iMoveDown + 1u]);
		bChanged = true;
	}
	if (bChanged)
	{
		const bool_t bOneRepeatedClip = Draft.animationSlots.size() > 1u &&
			std::all_of(Draft.animationSlots.begin() + 1u,
				Draft.animationSlots.end(),
				[&Draft](const CBalanceTool::ANIMATION_SLOT_EDIT& Slot)
				{
					return Slot.clip == Draft.animationSlots.front().clip;
				});
		Draft.animationRepeatCount = Draft.animationSlots.empty() ? 0u :
			(bOneRepeatedClip ?
				static_cast<uint32_t>(Draft.animationSlots.size()) : 1u);
		if (Draft.animationSlots.empty())
			Draft.animationEndPolicy = "NONE";
		if ("LOOP_TO_STAGE_END" != Draft.animationEndPolicy)
		{
			uint64_t iWallMs = 0u;
			for (const CBalanceTool::ANIMATION_SLOT_EDIT& Slot :
				Draft.animationSlots)
			{
				if (0u == Slot.playMs || Slot.repeatUntilStageEnd ||
					!std::isfinite(Slot.playRate) || Slot.playRate <= 0.0)
				{
					iWallMs = 0u;
					break;
				}
				iWallMs += static_cast<uint64_t>(std::llround(
					static_cast<double>(Slot.playMs) / Slot.playRate));
			}
			if (0u != iWallMs && iWallMs <= 600000u)
			{
				if ("EXACT" == Draft.animationEndPolicy ||
					Draft.durationMs < iWallMs)
				{
					Draft.durationMs = static_cast<uint32_t>(iWallMs);
				}
			}
		}
		if (SetValtanStageDraftWithSoundDependencyAdmission(
				m_pAnimationTool, m_pBalanceTool,
				m_bPatternShakesReady ? &m_PatternShakes : nullptr,
				Pattern, Stage, Draft, Status))
		{
			m_strStatus = std::move(Status);
			Invalidate_TimelineCache();
		}
		else
		{
			m_strStatus = std::move(Status);
		}
	}
	ImGui::EndDisabled();
	if (!Draft.animationEditable)
	{
		ImGui::TextDisabled(
			"This canonical Stage has no editable animation occurrence. Manual audition animation NONE Stages admit their first Sequence through Replace Stage Slots.");
	}
	else if (Draft.animationSlots.empty())
	{
		ImGui::TextDisabled(
			"Animation NONE: select an extracted Sequence on the left and use Replace Stage Slots to assign the first stable occurrences.");
	}
	ImGui::SeparatorText("Sequence Resource");
	ImGui::TextDisabled(
		"Use the Animation Sequences browser on the left to preview and Replace/Append exact source sequences.");
}

// Effect 전용 도구에 넘길 stable ID와 요청 값을 보관한다. 외부 도구를 여기서 생성하거나 cue 데이터의 소유권을 가져오지 않는다.
void Client::CValtanActionWorkbench::Request_EffectOwner(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue)
{
	m_strEffectPatternId = Pattern.strPatternId;
	m_strEffectStageId = Stage.strStageId;
	m_strEffectOccurrenceId = Cue.strOccurrenceId;
	m_strEffectAssetId = Cue.strEffectAssetId;
	m_bEffectToolOpenRequested = true;
}

// Stage 초안을 읽고 시계 정책과 Sound 종속 구간을 검증한 뒤 기존 BalanceTool 변경 경계에 전달한다. dirty/cache 갱신은 호출자가 처리하도록 적용 성공 여부만 반환한다.
bool_t Client::CValtanActionWorkbench::Apply_StageDurationDraft(
	const VALTAN_PATTERN_VIEW& pattern, const std::string& stageId,
	uint32_t requestedDurationMs, std::string& status)
{
	const auto stage = std::find_if(pattern.Stages.begin(), pattern.Stages.end(),
		[&](const auto& row) { return row.strStageId == stageId; });
	if (!m_pBalanceTool || stage == pattern.Stages.end())
	{ status = "The selected Stage is no longer available."; return false; }
	CBalanceTool::PATTERN_STAGE_EDIT draft;
	return m_pBalanceTool->Get_ValtanStageDraft(pattern.strPatternId, stageId, draft, status) &&
		ApplyStageClockPolicy(draft, requestedDurationMs, status) &&
		SetValtanStageDraftWithSoundDependencyAdmission(m_pAnimationTool, m_pBalanceTool,
			m_bPatternShakesReady ? &m_PatternShakes : nullptr, pattern, *stage, draft, status);
}

// 선택된 combat object의 Sound 초안 입력을 표시한다. 객체 수명과 재생은 기존 소유자가 처리하며 이 함수는 저작 값을 다룬다.
void Client::CValtanActionWorkbench::Render_CombatObjectSoundDetails(
	const std::string& archetypeId, const std::string& hitId, const bool_t editable)
{
	if (!m_bCombatObjectSoundsReady)
	{ ImGui::TextWrapped("Object Sound unavailable: %s", m_strCombatObjectSoundStatus.c_str()); return; }
	const auto found = std::find_if(m_CombatObjectSounds.Cues.begin(), m_CombatObjectSounds.Cues.end(),
		[&](const auto& cue) { return cue.strCombatObjectArchetypeId == archetypeId && cue.strHitId == hitId; });
	if (found == m_CombatObjectSounds.Cues.end())
	{ ImGui::TextDisabled("This hit has no authored Object Sound binding."); return; }
	// 값으로 복사해 두어 아래 조작부가 문서를 성공적으로 교체한 뒤에도 현재 편집 입력을 사용할 수 있다.
	const auto cue = *found;
	std::string event = cue.strSoundEvent;
	int offset = static_cast<int>(cue.iPlaybackOffsetMs);
	bool changed = false;
	ImGui::PushID(cue.strBindingId.c_str());
	ImGui::BeginDisabled(!editable);
	if (ImGui::BeginCombo("Explosion Sound event", event.c_str()))
	{
		for (const auto& value : m_PatternSoundEvents)
			if (ImGui::Selectable(value.c_str(), value == event))
			{ event = value; changed = event != cue.strSoundEvent; }
		ImGui::EndCombo();
	}
	changed |= ImGui::DragInt("Sound source offset (ms)", &offset, 1.f, 0, 600000, "%d", ImGuiSliderFlags_AlwaysClamp);
	if (changed)
	{
		(void)CValtanCombatObjectSoundCueDocument::Update_CueDraft(m_CombatObjectSounds,
			cue.strBindingId, event, static_cast<uint32_t>(offset), m_strCombatObjectSoundStatus);
		m_strStatus = m_strCombatObjectSoundStatus;
	}
	ImGui::EndDisabled();
	ImGui::TextDisabled("Sound fires with this hit. Source offset skips WAV lead-in; it does not delay the explosion.");
	ImGui::TextDisabled("Save & Publish commits Object Sound with the collider edits.");
	ImGui::PopID();
}

// Scene/Light/Camera 등 선택 종류에 맞는 Detail을 표시하고 해당 여부를 돌려준다. 호출자가 그 뒤 다른 Detail을 중복 표시하지 않도록 하는 분기 경계다.
bool_t Client::CValtanActionWorkbench::Render_AuxiliaryDetails(
	const VALTAN_PATTERN_VIEW& pattern, const VALTAN_STAGE_VIEW& stage, bool editable)
{
	const auto finish = [this, &pattern](bool applied, std::string& status)
	{
		if (applied)
		{
			Invalidate_TimelineCache();
			Refresh_PatternLocalPreviewAfterMutation(&pattern, status);
		}
		m_strStatus = status;
	};
	const auto timing = [&stage](uint32_t& start, uint32_t& duration)
	{
		int begin = static_cast<int>(start), length = static_cast<int>(duration);
		bool changed = ImGui::DragInt("Start (ms)", &begin, 10.f, 0,
			static_cast<int>(stage.iDurationMs > 0u ? stage.iDurationMs - 1u : 0u), "%d", ImGuiSliderFlags_AlwaysClamp);
		changed |= ImGui::DragInt("Duration (ms)", &length, 10.f, 1,
			static_cast<int>((std::max)(1u, stage.iDurationMs - static_cast<uint32_t>(begin))), "%d", ImGuiSliderFlags_AlwaysClamp);
		start = static_cast<uint32_t>(begin);
		duration = (std::min)(static_cast<uint32_t>(length), (std::max)(1u, stage.iDurationMs - start));
		return changed;
	};
	if (m_eDetailOwner == DETAIL_OWNER::SCENE_PROFILE)
	{
		auto cues = stage.SceneProfileOccurrences;
		auto found = std::find_if(cues.begin(), cues.end(), [&](const auto& x) { return x.strOccurrenceId == m_strSelectedStableId; });
		if (found == cues.end()) return false;
		ImGui::TextWrapped("%s", found->strProfileId.c_str());
		ImGui::BeginDisabled(!editable || !m_pBalanceTool);
		bool changed = timing(found->iStartMs, found->iDurationMs);
		if (ImGui::BeginCombo("Profile", found->strProfileId.c_str()))
		{
			for (const auto& id : m_SceneProfileResourceIds)
				if (ImGui::Selectable(id.c_str(), id == found->strProfileId)) { found->strProfileId = id; changed = true; }
			ImGui::EndCombo();
		}
		if (ImGui::Button("Remove occurrence")) { cues.erase(found); changed = true; }
		if (changed) { std::string status; finish(m_pBalanceTool->Set_ValtanStageSceneProfileOccurrences(pattern.strPatternId, stage.strStageId, cues, status), status); }
		ImGui::EndDisabled(); return true;
	}
	if (m_eDetailOwner == DETAIL_OWNER::LIGHT)
	{
		auto cues = stage.LightOccurrences;
		auto found = std::find_if(cues.begin(), cues.end(), [&](const auto& x) { return x.strOccurrenceId == m_strSelectedStableId; });
		if (found == cues.end()) return false;
		ImGui::TextWrapped("%s", found->strLightResourceId.c_str());
		ImGui::BeginDisabled(!editable || !m_pBalanceTool);
		bool changed = timing(found->iStartMs, found->iDurationMs);
		if (ImGui::BeginCombo("Light resource", found->strLightResourceId.c_str()))
		{
			for (const auto& id : m_LightResourceIds)
				if (ImGui::Selectable(id.c_str(), id == found->strLightResourceId)) { found->strLightResourceId = id; changed = true; }
			ImGui::EndCombo();
		}
		changed |= ImGui::DragFloat3("Position", found->Position.data(), .05f);
		changed |= ImGui::DragFloat3("Rotation", found->RotationDegrees.data(), 1.f);
		changed |= ImGui::DragFloat("Brightness", &found->fBrightnessMultiplier, .05f, 0.f, 100.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		changed |= ImGui::Checkbox("Follow boss", &found->bFollowBoss);
		int fadeIn = static_cast<int>(found->iFadeInMs), fadeOut = static_cast<int>(found->iFadeOutMs);
		changed |= ImGui::DragInt("Fade in (ms)", &fadeIn, 10.f, 0, static_cast<int>(found->iDurationMs), "%d", ImGuiSliderFlags_AlwaysClamp);
		changed |= ImGui::DragInt("Fade out (ms)", &fadeOut, 10.f, 0, static_cast<int>(found->iDurationMs), "%d", ImGuiSliderFlags_AlwaysClamp);
		found->iFadeInMs = (std::min)(static_cast<uint32_t>(fadeIn), found->iDurationMs);
		found->iFadeOutMs = (std::min)(static_cast<uint32_t>(fadeOut), found->iDurationMs);
		if (ImGui::Button("Remove occurrence")) { cues.erase(found); changed = true; }
		if (changed) { std::string status; finish(m_pBalanceTool->Set_ValtanStageLightOccurrences(pattern.strPatternId, stage.strStageId, cues, status), status); }
		ImGui::EndDisabled(); return true;
	}
	if (m_eDetailOwner == DETAIL_OWNER::CAMERA)
	{
		const auto found = std::find_if(stage.CameraInvocations.begin(), stage.CameraInvocations.end(),
			[&](const auto& x) { return x.strCameraInvocationId == m_strSelectedStableId; });
		if (found == stage.CameraInvocations.end()) return false;
		auto cue = *found;
		ImGui::TextWrapped("%s", cue.strCameraCueId.c_str());
		if (ImGui::Button("Edit camera resource")) { m_strCameraCueId = cue.strCameraCueId; m_bCameraToolOpenRequested = true; }
		ImGui::BeginDisabled(!editable || !m_pBalanceTool);
		bool changed = timing(cue.iStartOffsetMs, cue.iDurationMs);
		cue.strTrigger = "ENTER";
		cue.strDurationPolicy = "EXPLICIT";
		if (ImGui::Button("Remove occurrence"))
		{
			std::string status;
			finish(m_pBalanceTool->Remove_ValtanCameraInvocationDraft(pattern.strPatternId, stage.strStageId, cue, status), status);
		}
		else if (changed)
		{
			std::string status;
			finish(m_pBalanceTool->Upsert_ValtanCameraInvocationDraft(pattern.strPatternId, stage.strStageId, cue, status), status);
		}
		ImGui::EndDisabled(); return true;
	}
	if (m_eDetailOwner == DETAIL_OWNER::COMBAT_OBJECT)
	{
		const auto found = std::find_if(stage.CombatObjectEffects.begin(), stage.CombatObjectEffects.end(), [&](const auto& x) { return x.strCombatObjectArchetypeId == m_strSelectedStableId; });
		if (found == stage.CombatObjectEffects.end()) return false;
		auto cue = *found;
		ImGui::TextWrapped("%s", cue.strCombatObjectArchetypeId.c_str());
		ImGui::TextWrapped("Shared archetype lifetime changes affect all occurrences of this resource.");
		ImGui::TextWrapped("Active Effect: %s", cue.strEffectAssetId.c_str());
		for (const auto& sourceHit : cue.Hits)
		{
			if (sourceHit.strTriggerKind != "TIMED" || (sourceHit.strHitShape != "CIRCLE" && sourceHit.strHitShape != "RING")) continue;
			auto hit = sourceHit;
			ImGui::PushID(hit.strHitId.c_str());
			ImGui::SeparatorText("Explosion / Damage Collider");
			ImGui::TextWrapped("%s | %s", hit.strHitId.c_str(), hit.strServerDamageProfileId.c_str());
			ImGui::BeginDisabled(!editable || !m_pBalanceTool);
			if (cue.OwnerHitChain) ImGui::Text("Original preparation: %u ms. Direct rock explodes after preparation; other rocks add the delay below.", hit.iAtMs);
			int clock = static_cast<int>(cue.OwnerHitChain ? cue.OwnerHitChain->iDelayMs : hit.iAtMs);
			bool impactChanged = ImGui::DragInt(cue.OwnerHitChain ? "Other rocks preparation delay (ms)" : "Explosion after spawn (ms)",
				&clock, 1.f, cue.OwnerHitChain ? 1 : 0, static_cast<int>(cue.iLifetimeMs - 1u), "%d", ImGuiSliderFlags_AlwaysClamp);
			impactChanged |= ImGui::DragFloat("Collider radius (m)", &hit.fOuterRadiusM, .05f, .01f, 1000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			if (hit.strHitShape == "RING") impactChanged |= ImGui::DragFloat("Inner radius (m)", &hit.fInnerRadiusM, .05f, 0.f, 999.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			impactChanged |= ImGui::DragFloat("Knockback distance (m)", &hit.fPushRangeM, .05f, 0.f, 1000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			int pushMs = static_cast<int>(hit.iPushMs), downMs = static_cast<int>(hit.iDownMs);
			impactChanged |= ImGui::DragInt("Knockback duration (ms)", &pushMs, 1.f, 0, 600000, "%d", ImGuiSliderFlags_AlwaysClamp);
			impactChanged |= ImGui::Checkbox("Knockdown", &hit.bKnockdown);
			impactChanged |= ImGui::DragInt("Down duration (ms)", &downMs, 10.f, 0, 600000, "%d", ImGuiSliderFlags_AlwaysClamp);
			hit.iPushMs = static_cast<uint32_t>(pushMs); hit.iDownMs = static_cast<uint32_t>(downMs);
			if (!cue.OwnerHitChain) hit.iAtMs = static_cast<uint32_t>(clock);
			if (impactChanged)
			{
				std::string status;
				finish(m_pBalanceTool->Set_ValtanCombatObjectImpactDraft(pattern.strPatternId, stage.strStageId,
					cue.strCombatObjectArchetypeId, hit, cue.OwnerHitChain ? static_cast<uint32_t>(clock) : 0u, status), status);
			}
			ImGui::EndDisabled();
			Render_CombatObjectSoundDetails(cue.strCombatObjectArchetypeId, hit.strHitId, editable);
			ImGui::TextDisabled("Effect, Sound and damage share this Server hit. Preview wires do not damage players.");
			ImGui::PopID();
		}
		ImGui::BeginDisabled(!editable || !m_pBalanceTool);
		int start = static_cast<int>(cue.iFirstSpawnOffsetMs), lifetime = static_cast<int>(cue.iLifetimeMs);
		int count = static_cast<int>(cue.iSpawnValue), waves = static_cast<int>((std::max)(1u, cue.iSpawnScheduleCount)), interval = static_cast<int>(cue.iSpawnIntervalMs);
		bool changed = ImGui::DragInt("Spawn start (ms)", &start, 10.f, 0, static_cast<int>(stage.iDurationMs - 1u), "%d", ImGuiSliderFlags_AlwaysClamp);
		changed |= ImGui::DragInt("Lifetime (ms)", &lifetime, 10.f, 1, 600000, "%d", ImGuiSliderFlags_AlwaysClamp);
		changed |= ImGui::DragInt("Count", &count, 1.f, 1, 64, "%d", ImGuiSliderFlags_AlwaysClamp);
		changed |= ImGui::DragInt("Waves", &waves, 1.f, 1, 8, "%d", ImGuiSliderFlags_AlwaysClamp);
		changed |= ImGui::DragInt("Wave interval (ms)", &interval, 10.f, 1, 600000, "%d", ImGuiSliderFlags_AlwaysClamp);
		cue.iFirstSpawnOffsetMs = static_cast<uint32_t>(start); cue.iLifetimeMs = static_cast<uint32_t>(lifetime);
		cue.iSpawnValue = static_cast<uint32_t>(count); cue.iSpawnScheduleCount = static_cast<uint32_t>(waves); cue.iSpawnIntervalMs = static_cast<uint32_t>(interval);
		if (ImGui::Button("Remove summon"))
		{
			std::string status; finish(m_pBalanceTool->Remove_ValtanSummonDraft(pattern.strPatternId, stage.strStageId, cue, status), status);
		}
		else if (changed)
		{
			std::string status; finish(m_pBalanceTool->Upsert_ValtanSummonDraft(pattern.strPatternId, stage.strStageId, cue, status), status);
		}
		ImGui::EndDisabled(); return true;
	}
	if (m_eDetailOwner == DETAIL_OWNER::GAMEPLAY_STAGE)
	{
		const auto found = std::find_if(stage.Actions.begin(), stage.Actions.end(), [&](const auto& x)
		{ return stage.strStageId + "/action/" + x.strKind + "/" + x.strTargetId + "/" + x.strTrigger == m_strSelectedStableId; });
		if (found == stage.Actions.end()) return false;
		auto action = *found;
		ImGui::TextWrapped("%s | %s [%s]", action.strKind.c_str(), action.strTargetId.c_str(), action.strTrigger.c_str());
		const bool dedicated = action.strKind.find("SPAWN_COMBAT_OBJECT") == 0u ||
			(action.strKind == "SET_BOSS_FLAG" && (action.strTargetId == "boss.flag.counterable" || action.strTargetId == "boss.flag.groggy"));
		ImGui::BeginDisabled(!editable || !m_pBalanceTool || dedicated);
		bool changed = false;
		if (action.strKind == "RELEASE_GRABBED_PLAYERS")
		{
			const char* modes[] = {"HOLD", "OPPOSITE_KNOCKBACK", "ARENA_EJECTION"};
			if (ImGui::BeginCombo("Release mode", action.strReleaseMode.c_str()))
			{
				for (auto mode : modes) if (ImGui::Selectable(mode, action.strReleaseMode == mode)) { action.strReleaseMode = mode; changed = true; }
				ImGui::EndCombo();
			}
			changed |= ImGui::DragFloat("Speed (m/s)", &action.fSpeedMps, .1f, 0.f, 100.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			int duration = static_cast<int>(action.iDurationMs);
			changed |= ImGui::DragInt("Duration (ms)", &duration, 10.f, 0, 600000, "%d", ImGuiSliderFlags_AlwaysClamp);
			action.iDurationMs = static_cast<uint32_t>(duration);
			changed |= ImGui::DragFloat("Yaw offset", &action.fYawOffsetDegrees, 1.f, -360.f, 360.f);
		}
		else if (action.strKind == "SET_BOSS_FLAG" || action.strKind == "SET_PLAYER_BIND" || action.strKind == "SET_PLAYER_SILENCE" || action.strKind == "SUPPRESS_INTER_STEP_PURSUIT")
		{
			bool enabled = action.fValue != 0.f;
			changed = ImGui::Checkbox("Enabled", &enabled); action.fValue = enabled ? 1.f : 0.f;
		}
		else if (action.strKind == "SET_STAGGER_GAUGE" || action.strKind == "DAMAGE_GRABBED_PLAYERS" || action.strKind == "SET_GAMEPLAY_PHASE")
			changed = ImGui::DragFloat("Value", &action.fValue, 1.f, 0.f, 1000000.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
		if (ImGui::Button("Remove action"))
		{
			std::string status;
			finish(m_pBalanceTool->Remove_ValtanStageActionDraft(pattern.strPatternId, stage.strStageId, action, status), status);
		}
		else if (changed)
		{
			std::string status;
			finish(m_pBalanceTool->Upsert_ValtanStageActionDraft(pattern.strPatternId, stage.strStageId, action, status), status);
		}
		ImGui::EndDisabled();
		if (dedicated) ImGui::TextWrapped("Use the Counter or Summon box to edit this linked action.");
		else ImGui::TextWrapped("Drag this box to a Stage enter or exit boundary.");
		return true;
	}
	return false;
}

// 현재 선택의 Detail UI를 조율한다. frame-local Pattern/Stage와 외부 draft를 차용하며 적용·preview·도구 열기는 기존 명령 함수로 보낸다.
void Client::CValtanActionWorkbench::Render_Details(
	const VALTAN_PATTERN_VIEW* const pFramePattern,
	const VALTAN_STAGE_VIEW* const pFrameStage,
	const bool_t bMutationAdmitted,
	const bool_t bPatternMutationAdmitted)
{
    const VALTAN_PATTERN_VIEW* pPattern = pFramePattern;
    const VALTAN_STAGE_VIEW* pStage = pFrameStage;
    std::optional<VALTAN_PATTERN_VIEW> refreshedPattern;
    // 이 pane보다 앞선 Sequencer 편집이 draft를 이미 갱신했을 수 있다.
    // 공유 불변 view는 살아 있게 두되 그 안의 오래된 cue로 Details 입력을 다시 초기화하지 않는다.
    if (m_bWorkbenchFrameActive && bPatternMutationAdmitted && pFramePattern && m_pBalanceTool &&
        m_iWorkbenchFrameDraftGeneration != m_pBalanceTool->Get_ValtanDraftGeneration())
    {
        refreshedPattern.emplace();
        std::string status;
        if (!m_pBalanceTool->Get_ValtanPatternDraft(
                pFramePattern->strPatternId, *refreshedPattern, status))
        {
            ImGui::TextWrapped("Box Detail is waiting for the updated Pattern draft: %s", status.c_str());
            return;
        }
        pPattern = &*refreshedPattern;
        pStage = Find_SelectedStage(pPattern);
    }
	ImGui::SeparatorText("Box Detail");
    const bool saveReady = pPattern && bPatternMutationAdmitted && m_pBalanceTool &&
        !m_pBalanceTool->Is_ValtanSaveJobBlockingAuthoring() && !m_pBalanceTool->Is_ServerRuntimeSetPublishRunning() &&
        (m_bAuthoringDraftDirty || m_bPatternSoundDependencyDirty || m_CombatObjectSounds.bDraftDirty || CEffectV2Catalog::Get().Has_BossValtanBindingDraft() ||
            Has_PendingSelectedEffectDetails(pPattern));
    ImGui::BeginDisabled(!saveReady);
    if (ImGui::Button("Save Source##ValtanBoxDetail"))
    {
        m_bPublishWithPendingSave = false;
        m_bSavePatternRequested = true;
    }
    ImGui::EndDisabled();
    if (saveReady && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
        !ImGui::GetIO().WantTextInput && ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false))
    {
        m_bPublishWithPendingSave = false;
        m_bSavePatternRequested = true;
    }
    ImGui::SameLine();
	ImGui::Text("Owner: %s", Owner_Label(m_eDetailOwner));
	if (nullptr == pPattern)
	{
		ImGui::TextDisabled("Select one canonical Pattern.");
		return;
	}
    if (ImGui::CollapsingHeader("Bone / weapon animation"))
    {
        const auto boss = CAnimationTargetService::Resolve_Boss();
        if (!boss || CAnimationTargetService::Resolve_AssetName() != "Valtan")
        {
            ImGui::TextWrapped("Stage the local Valtan Model View in Preview, then edit its body or b_wp_r_01 axe socket here.");
        }
        else
        {
            m_BoneEditor.Select("Valtan", boss->Get_BodyModel());
            const bool wasActive = m_BoneEditor.Is_Active();
            m_BoneEditor.Render();
            if (m_BoneEditor.Consume_Installed() && m_pAnimationTool)
            {
                std::vector<COMPOSITION_ANIMATION_RESOURCE> resources;
                std::string status;
                m_pAnimationTool->Read_CompositionAnimationResources(resources, status);
            }
            if (m_BoneEditor.Is_Active())
            {
                if (!wasActive && m_pAnimationTool)
                {
                    std::string stopStatus; m_pAnimationTool->Stop_ValtanCompositionPattern(stopStatus);
                    m_BoneEditor.Update(0.f);
                }
                m_bPreviewOwnerClaimRequested = true;
            }
            ImGui::TextWrapped("Bone Clips saves authored model animation separately. The axe follows b_wp_r_01. After saving, refresh Composition Resources and reload the Pattern source revision before adding the authored clip. Publish installs a validated copy for Server Pattern presentation; re-enter the arena to reload model prototypes.");
        }
    }
	if (pStage && Render_AuxiliaryDetails(*pPattern, *pStage, bPatternMutationAdmitted)) return;
	bool_t bPatternSoundDirty = false;
	std::string PatternSoundStatus;
	const VALTAN_PATTERN_SOUND_CUE_DOCUMENT* const pPatternSounds =
		VALTAN_VIEW_ADMISSION::ADMITTED != m_eAdmission ||
		nullptr == m_pAnimationTool ? nullptr :
		m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(
			bPatternSoundDirty, PatternSoundStatus);
	if (!PatternSoundStatus.empty())
		m_strSoundStatus = PatternSoundStatus;
	std::string strFirstSoundOccurrence;
	if (nullptr != pStage && nullptr != pPatternSounds)
	{
		const auto FirstSound = std::find_if(
			pPatternSounds->Cues.begin(), pPatternSounds->Cues.end(),
			[pPattern, pStage](const VALTAN_PATTERN_SOUND_CUE& Cue)
			{
				return Cue.strPatternId == pPattern->strPatternId &&
					Cue.strStageId == pStage->strStageId;
			});
		if (FirstSound != pPatternSounds->Cues.end())
			strFirstSoundOccurrence = FirstSound->strOccurrenceId;
	}
	bool_t bStageSelectionChanged = false;
	const char_t* const pStagePreview = nullptr == pStage ?
		"Select stable Stage" : pStage->strStageId.c_str();
	ImGui::SetNextItemWidth(-1.f);
	if (ImGui::BeginCombo(
			"Selected Stage##CompositionStableStage", pStagePreview))
	{
		for (const VALTAN_STAGE_VIEW& Candidate : pPattern->Stages)
		{
			const bool_t bSelected = nullptr != pStage &&
				Candidate.strStageId == pStage->strStageId;
			const std::string Label = Candidate.strStageId + " | " +
				StageSequenceRoleLabel(Candidate) + " | " +
				std::to_string(Candidate.iDurationMs) + " ms";
			if (ImGui::Selectable(Label.c_str(), bSelected) && !bSelected)
			{
				Select_Stage(*pPattern, Candidate);
				bStageSelectionChanged = true;
			}
			if (bSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	if (bStageSelectionChanged)
	{
		ImGui::TextDisabled(
			"Resolved the new Stage by stable ID; its Stage/Collider/Camera categories open on the next frame.");
		return;
	}
	const auto OwnerButton = [this](const char_t* pLabel,
		const DETAIL_OWNER eOwner, const bool_t bEnabled,
		const std::string& strStableId)
	{
		ImGui::BeginDisabled(!bEnabled);
		bool_t bActive = m_eDetailOwner == eOwner;
		if (bActive && DETAIL_OWNER::GAMEPLAY_STAGE == eOwner)
		{
			const bool_t bButtonCollider = std::string::npos !=
				strStableId.find("/collider");
			const bool_t bSelectionCollider = std::string::npos !=
				m_strSelectedStableId.find("/collider");
			bActive = bButtonCollider == bSelectionCollider;
		}
		if (bActive)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.43f, 0.72f, 1.f));
		if (ImGui::SmallButton(pLabel))
		{
			m_eDetailOwner = eOwner;
			m_strSelectedStableId = strStableId;
			m_bDetailFocusRequested = true;
		}
		if (bActive)
			ImGui::PopStyleColor();
		ImGui::EndDisabled();
	};
	OwnerButton("Pattern Root", DETAIL_OWNER::PATTERN, true,
		pPattern->strPatternId);
	ImGui::SameLine();
	OwnerButton("Stage / Logic", DETAIL_OWNER::GAMEPLAY_STAGE,
		nullptr != pStage,
		nullptr == pStage ? std::string{} : pStage->strStageId);
	ImGui::SameLine();
	OwnerButton("Collider", DETAIL_OWNER::GAMEPLAY_STAGE,
		nullptr != pStage,
		nullptr == pStage ? std::string{} :
			pStage->strStageId + "/collider");
	ImGui::SameLine();
	OwnerButton("Animation", DETAIL_OWNER::ANIMATION,
		nullptr != pStage && (pPattern->bManualServerAudition ||
			!pStage->ClipOccurrences.empty()),
		nullptr == pStage ? std::string{} :
			(pStage->ClipOccurrences.empty() ? pStage->strStageId :
				pStage->ClipOccurrences.front().strClipOccurrenceId));
	OwnerButton("Effect", DETAIL_OWNER::EFFECT,
		nullptr != pStage,
		nullptr == pStage || pStage->ProductCues.empty() ? std::string{} :
			pStage->ProductCues.front().strOccurrenceId);
	ImGui::SameLine();
	OwnerButton("Sound", DETAIL_OWNER::SOUND,
		nullptr != pStage && nullptr != pPatternSounds &&
			!pStage->ClipOccurrences.empty(),
		strFirstSoundOccurrence);
	ImGui::SameLine();
	OwnerButton("Camera", DETAIL_OWNER::CAMERA,
		nullptr != pStage && !pStage->CameraInvocations.empty(),
		nullptr == pStage || pStage->CameraInvocations.empty() ? std::string{} :
			pStage->CameraInvocations.front().strCameraInvocationId);
	if (DETAIL_OWNER::PATTERN == m_eDetailOwner || nullptr == pStage)
	{
		ImGui::TextWrapped("%s", pPattern->strDisplayName.c_str());
		ImGui::TextDisabled("%s", pPattern->strPatternId.c_str());
		ImGui::Text("Category: %s", pPattern->strCategory.c_str());
		ImGui::Text("Phase: %u .. %u", pPattern->iMinimumPhase, pPattern->iMaximumPhase);
		ImGui::Text("Target: %s | Aim: %s",
			pPattern->strTargetPolicy.c_str(), pPattern->strAimPolicy.c_str());
		ImGui::Text("Range: %.2f .. %.2f m",
			pPattern->fMinimumRange, pPattern->fMaximumRange);
		ImGui::Text("Stages: %zu | Source Sequence: %u",
			pPattern->Stages.size(), pPattern->iSourceSequenceIndex);
		ImGui::SeparatorText("Pattern Timeline");
		constexpr ImGuiTableFlags PATTERN_TIMELINE_FLAGS =
			ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
			ImGuiTableFlags_SizingStretchProp;
		if (ImGui::BeginTable("##PatternTimeline", 6, PATTERN_TIMELINE_FLAGS))
		{
			ImGui::TableSetupColumn("Stage", ImGuiTableColumnFlags_WidthStretch, 2.f);
			ImGui::TableSetupColumn("Role", ImGuiTableColumnFlags_WidthStretch, 1.2f);
			ImGui::TableSetupColumn("Start", ImGuiTableColumnFlags_WidthStretch, 1.f);
			ImGui::TableSetupColumn("End", ImGuiTableColumnFlags_WidthStretch, 1.f);
			ImGui::TableSetupColumn("Duration (ms)", ImGuiTableColumnFlags_WidthStretch, 1.4f);
			ImGui::TableSetupColumn("Collider", ImGuiTableColumnFlags_WidthStretch, 1.6f);
			ImGui::TableHeadersRow();
			uint64_t iCursorMs = 0u;
			for (const VALTAN_STAGE_VIEW& Row : pPattern->Stages)
			{
				ImGui::PushID(Row.strStageId.c_str());
				CBalanceTool::PATTERN_STAGE_EDIT RowDraft;
				std::string RowDraftStatus;
				const bool_t bRowDraft = bPatternMutationAdmitted &&
					nullptr != m_pBalanceTool &&
					m_pBalanceTool->Get_ValtanStageDraft(
						pPattern->strPatternId, Row.strStageId,
						RowDraft, RowDraftStatus);
				const uint32_t iRowDurationMs =
					bRowDraft ? RowDraft.durationMs : Row.iDurationMs;
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				const bool_t bRowSelected = nullptr != pStage &&
					pStage->strStageId == Row.strStageId;
				if (ImGui::Selectable(Row.strStageId.c_str(), bRowSelected))
					Select_Stage(*pPattern, Row);
				ImGui::TableNextColumn();
				const std::string RowRole = StageSequenceRoleLabel(Row);
				ImGui::TextUnformatted(RowRole.c_str());
				ImGui::TableNextColumn();
				ImGui::Text("%llu", static_cast<unsigned long long>(iCursorMs));
				ImGui::TableNextColumn();
				ImGui::Text("%llu",
					static_cast<unsigned long long>(iCursorMs + iRowDurationMs));
				ImGui::TableNextColumn();
				if (bRowDraft && RowDraft.durationEditable &&
					!RowDraft.portalRushMotionEditable)
				{
					int32_t iDurationInput = static_cast<int32_t>((std::min)(
						iRowDurationMs, static_cast<uint32_t>(INT32_MAX)));
					ImGui::SetNextItemWidth(-1.f);
					if (ImGui::DragInt("##PatternTimelineDuration", &iDurationInput,
							10.f, 1, 120000, "%d", ImGuiSliderFlags_AlwaysClamp))
					{
						std::string ClockStatus;
						if (ApplyStageClockPolicy(
								RowDraft, static_cast<uint32_t>(iDurationInput),
								ClockStatus) &&
							SetValtanStageDraftWithSoundDependencyAdmission(
								m_pAnimationTool, m_pBalanceTool,
								m_bPatternShakesReady ? &m_PatternShakes : nullptr,
								*pPattern, Row, RowDraft, ClockStatus))
						{
							Invalidate_TimelineCache();
						}
						m_strStatus = std::move(ClockStatus);
					}
				}
				else
				{
					ImGui::Text("%u", iRowDurationMs);
				}
				ImGui::TableNextColumn();
				std::string ColliderSummary;
				if (Row.Has_HitShape())
				{
					ColliderSummary = std::string(
						"CAPTURE" == Row.strPlayerResponse ? "GRAB " : "DAMAGE ") +
						Row.strHitShape;
				}
				if (Row.CounterProxy.has_value())
					ColliderSummary += ColliderSummary.empty() ? "COUNTER" : " + COUNTER";
				ImGui::TextUnformatted(
					ColliderSummary.empty() ? "-" : ColliderSummary.c_str());
				iCursorMs += iRowDurationMs;
				ImGui::PopID();
			}
			ImGui::EndTable();
			ImGui::Text("Total: %llu ms", static_cast<unsigned long long>(iCursorMs));
		}
		if (pPattern->bManualServerAudition)
		{
			/* lane만 봐서는 알 수 없는 저작 개수 제한을 표시한다. */
			ImGui::TextDisabled(
				"Grab release action creation is unavailable in this revision; release mode/velocity/duration/yaw become editable once a RELEASE_GRABBED_PLAYERS action is admitted.");
		}
		ImGui::SeparatorText("Sequence Sources");
		for (const VALTAN_PRESENTATION_SOURCE_VIEW& Source :
			pPattern->PresentationSources)
		{
			ImGui::BulletText("%s | action %u | sequence %u",
				Source.strRole.c_str(), Source.iSourceActionId,
				Source.iSequenceIndex);
		}
		if (pPattern->ServerMotion.has_value())
		{
			const VALTAN_PATTERN_SERVER_MOTION_VIEW& Motion = *pPattern->ServerMotion;
			ImGui::SeparatorText("Pattern Motion");
			ImGui::Text("%s | anchor %s", Motion.strKind.c_str(), Motion.strAnchorId.c_str());
			ImGui::Text("Landing %.2f, %.2f, %.2f | apex %.2f",
				Motion.LandingPosition[0], Motion.LandingPosition[1],
				Motion.LandingPosition[2], Motion.fApexHeight);
		}
		ImGui::TextWrapped(
			"Select a box in Sequencer to edit only that Stage, Animation, Effect, Sound, Logic, Collider, or Camera item.");
		return;
	}

	if (DETAIL_OWNER::GAMEPLAY_STAGE == m_eDetailOwner)
	{
		Render_GameplayStageDetails(
			*pPattern, *pStage, bPatternMutationAdmitted);
		return;
	}
	if (DETAIL_OWNER::ANIMATION == m_eDetailOwner)
	{
		Render_AnimationStageDetails(
			*pPattern, *pStage, bPatternMutationAdmitted);
		return;
	}
	if (DETAIL_OWNER::EFFECT == m_eDetailOwner)
	{
		const auto SelectedV2Item = std::find_if(
			m_TimelineItems.begin(), m_TimelineItems.end(),
			[this, pPattern, pStage](const TIMELINE_ITEM& Item)
			{
				return Item.bEffectV2Binding &&
					Item.strPatternId == pPattern->strPatternId &&
					Item.strStageId == pStage->strStageId &&
					Item.strStableId == m_strSelectedStableId;
			});
		if (SelectedV2Item != m_TimelineItems.end())
		{
			const std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pSnapshot =
				CEffectV2Catalog::Get().Get_Snapshot();
			const EFFECT_V2_BINDING* const pBinding = nullptr == pSnapshot ?
				nullptr : ResolveEffectV2Binding(
					*pSnapshot, SelectedV2Item->strStableId,
					SelectedV2Item->strEffectV2ClipOccurrenceId);
			ImGui::SeparatorText("Selected Effect V2 Box");
			if (nullptr == pBinding)
			{
				ImGui::TextDisabled(
					"The exact V2 binding row changed. Reload Effect Resources and select the box again.");
				return;
			}
			const bool_t bGroup = !pBinding->strGroupId.empty();
			const std::string& strResourceId = bGroup ?
				pBinding->strGroupId : pBinding->strEffectId;
			ImGui::Text("%s: %s", bGroup ? "Group" : "Leaf",
				strResourceId.c_str());
			ImGui::TextWrapped("Binding: %s", pBinding->strBindingId.c_str());
			ImGui::TextDisabled("Scope: %s / %s", pBinding->strPatternId.c_str(), pBinding->strStageId.c_str());
			if (m_strEffectV2BindingEditId != pBinding->strBindingId ||
				m_iEffectV2BindingEditRevision != pSnapshot->Get_Revision())
			{
				m_EffectV2BindingEditDraft = *pBinding;
				m_strEffectV2BindingEditId = pBinding->strBindingId;
				m_iEffectV2BindingEditRevision = pSnapshot->Get_Revision();
				std::snprintf(m_EffectV2AnchorSlot.data(), m_EffectV2AnchorSlot.size(),
					"%s", pBinding->strAnchorSlotId.c_str());
			}
			EFFECT_V2_BINDING& Draft = m_EffectV2BindingEditDraft;
			ImGui::BeginDisabled(!bMutationAdmitted);
			int32_t Clock = static_cast<int32_t>(Draft.eClockBasis);
			if (ImGui::Combo("Clock", &Clock, "Stage\0Animation occurrence\0"))
			{
				Draft.eClockBasis = static_cast<EFFECT_V2_CLOCK_BASIS>(Clock);
				Draft.eRepeatPolicy = EFFECT_V2_REPEAT_POLICY::ONCE;
				Draft.eStopPolicy = EFFECT_V2_STOP_POLICY::NATURAL;
				Draft.strClipOccurrenceId.clear();
				Draft.iStartMs = 0u;
				if (EFFECT_V2_CLOCK_BASIS::CLIP_OCCURRENCE == Draft.eClockBasis &&
					!pStage->ClipOccurrences.empty())
				{
					Draft.strClipOccurrenceId = pStage->ClipOccurrences.front().strClipOccurrenceId;
					Draft.iStartMs = pStage->ClipOccurrences.front().iSourceStartMs;
				}
			}
			const bool ClipClock = EFFECT_V2_CLOCK_BASIS::CLIP_OCCURRENCE == Draft.eClockBasis;
			if (ClipClock && ImGui::BeginCombo("Animation occurrence", Draft.strClipOccurrenceId.c_str()))
			{
				for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip : pStage->ClipOccurrences)
				{
					const std::string Label = Clip.strClipName + " | " + Clip.strClipOccurrenceId;
					if (ImGui::Selectable(Label.c_str(), Clip.strClipOccurrenceId == Draft.strClipOccurrenceId))
					{
						Draft.strClipOccurrenceId = Clip.strClipOccurrenceId;
						Draft.iStartMs = Clip.iSourceStartMs;
						Draft.eRepeatPolicy = EFFECT_V2_REPEAT_POLICY::ONCE;
					}
				}
				ImGui::EndCombo();
			}
			ImGui::InputScalar(ClipClock ? "Clip source start (ms)" : "Stage start (ms)",
				ImGuiDataType_U32, &Draft.iStartMs);
			int32_t Repeat = static_cast<int32_t>(Draft.eRepeatPolicy);
			ImGui::BeginDisabled(!ClipClock);
			if (ImGui::Combo("Repeat", &Repeat, "Once\0Each loop\0"))
				Draft.eRepeatPolicy = static_cast<EFFECT_V2_REPEAT_POLICY>(Repeat);
			ImGui::EndDisabled();
			ImGui::InputText("Anchor slot / bone", m_EffectV2AnchorSlot.data(), m_EffectV2AnchorSlot.size());
			int32_t Follow = static_cast<int32_t>(Draft.eFollowPolicy);
			if (ImGui::Combo("Follow", &Follow, "Follow slot\0Snapshot at start\0"))
				Draft.eFollowPolicy = static_cast<EFFECT_V2_FOLLOW_POLICY>(Follow);
			int32_t Rotation = static_cast<int32_t>(Draft.eRotationBasis);
			if (ImGui::Combo("Rotation basis", &Rotation, "Slot\0Target yaw\0World\0"))
				Draft.eRotationBasis = static_cast<EFFECT_V2_ROTATION_BASIS>(Rotation);
			ImGui::DragFloat3("Local translation", &Draft.LocalTransform.vTranslation.x, 0.01f);
            bool applyTransform = ImGui::IsItemDeactivatedAfterEdit();
			ImGui::DragFloat3("Local rotation (degrees)", &Draft.LocalTransform.vRotation.x, 0.25f);
            applyTransform |= ImGui::IsItemDeactivatedAfterEdit();
			ImGui::DragFloat3("Local scale", &Draft.LocalTransform.vScale.x, 0.01f, 0.001f, 1000.f);
            applyTransform |= ImGui::IsItemDeactivatedAfterEdit();
			int32_t Stop = static_cast<int32_t>(Draft.eStopPolicy);
			if (ImGui::Combo("Stop", &Stop, "Natural\0Stage end\0Animation occurrence end\0Explicit\0"))
				Draft.eStopPolicy = static_cast<EFFECT_V2_STOP_POLICY>(Stop);
			if (ImGui::Button("Apply V2 Binding") || applyTransform)
			{
				EFFECT_V2_BINDING Candidate = Draft;
				Candidate.strAnchorSlotId = m_EffectV2AnchorSlot.data();
				std::string Status;
				if (Validate_EffectV2BindingClock(*pStage, Candidate, Status) &&
					CEffectV2Catalog::Get().Stage_UpdateBossValtanBinding(Candidate, Status))
				{
					m_iEffectV2CatalogRevision = CEffectV2Catalog::Get().Get_Revision();
					m_strSelectedStableId = BuildEffectV2BindingStableId(Candidate,
						ClipClock ? Candidate.strClipOccurrenceId : std::string{});
					Refresh_EffectV2LocalPreviewAfterMutation(pPattern, Status);
				}
				m_strStatus = std::move(Status);
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset V2 Inputs"))
				m_strEffectV2BindingEditId.clear();
			ImGui::EndDisabled();
			ImGui::TextDisabled("Apply stages this exact binding. Save commits it with the current Composition owners.");
			return;
		}

		ImGui::SeparatorText("Stage Effect Invocations");
		ImGui::TextWrapped(
			"Typed source owner: Data/Valtan/Valtan.presentation.json");
		ImGui::TextDisabled(
			"Valtan.patterneffectcues.json is a read-only generated Product. Effect asset bodies remain owned by Effect Tool.");
		if (!m_bSemanticValtanEffectLoadAttempted)
			Reload_SemanticValtanEffects();
		ImGui::InputTextWithHint(
			"##ValtanEffectSearch", "Filter authored effect.valtan.*...",
			m_EffectSearch.data(), m_EffectSearch.size());
		ImGui::SameLine();
		if (ImGui::SmallButton("Refresh Effect Catalog"))
			Reload_SemanticValtanEffects();
		ImGui::TextDisabled(
			"%zu direct-authored Valtan Effects (semantic catalog only).",
			m_SemanticValtanEffectAssetIds.size());

		if (!pStage->ProductCues.empty())
		{
			const auto selected = std::find_if(
				pStage->ProductCues.begin(), pStage->ProductCues.end(),
				[this](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& cue)
				{
					return cue.strOccurrenceId == m_strSelectedStableId;
				});
			if (pStage->ProductCues.end() == selected)
				m_strSelectedStableId = pStage->ProductCues.front().strOccurrenceId;
			const auto current = std::find_if(
				pStage->ProductCues.begin(), pStage->ProductCues.end(),
				[this](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& cue)
				{
					return cue.strOccurrenceId == m_strSelectedStableId;
				});
			const char_t* const pPreview = current == pStage->ProductCues.end() ?
				"Select invocation" : current->strEffectAssetId.c_str();
			if (ImGui::BeginCombo("Existing invocation", pPreview))
			{
				for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& cue :
					pStage->ProductCues)
				{
					const bool_t bSelected =
						cue.strOccurrenceId == m_strSelectedStableId;
					const std::string label = cue.strEffectAssetId + "##" +
						cue.strOccurrenceId;
					if (ImGui::Selectable(label.c_str(), bSelected))
					{
						m_strSelectedStableId = cue.strOccurrenceId;
						m_strEffectEditIdentity.clear();
					}
					if (bSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}

		const auto RenderEffectAssetPicker = [this](
			const char_t* const pLabel, std::string& selection)
		{
			const char_t* const pPreview = selection.empty() ?
				"Select authored Valtan Effect" : selection.c_str();
			if (!ImGui::BeginCombo(pLabel, pPreview))
				return false;
            bool changed = false;
			for (const std::string& EffectAssetId :
				m_SemanticValtanEffectAssetIds)
			{
				if (!ContainsInsensitive(EffectAssetId, m_EffectSearch.data()))
					continue;
				const bool_t bSelected = EffectAssetId == selection;
				if (ImGui::Selectable(EffectAssetId.c_str(), bSelected))
                { selection = EffectAssetId; changed = true; }
				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
            return changed;
		};
		const auto RenderClipOccurrencePicker = [pStage](
			const char_t* const pLabel, std::string& selection)
		{
			const char_t* const pPreview = selection.empty() ?
				"Select exact Sequence slot" : selection.c_str();
			if (!ImGui::BeginCombo(pLabel, pPreview))
				return false;
			bool_t bChanged = false;
			for (const VALTAN_CLIP_OCCURRENCE_VIEW& clip :
				pStage->ClipOccurrences)
			{
				const bool_t bSelected =
					clip.strClipOccurrenceId == selection;
				const std::string label = clip.strClipName + " | " +
					clip.strClipOccurrenceId + "##" + pLabel;
				if (ImGui::Selectable(label.c_str(), bSelected))
				{
					selection = clip.strClipOccurrenceId;
					bChanged = true;
				}
				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
			return bChanged;
		};

		const auto Found = std::find_if(
			pStage->ProductCues.begin(), pStage->ProductCues.end(),
			[this](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& cue)
			{
				return cue.strOccurrenceId == m_strSelectedStableId;
			});
		if (Found != pStage->ProductCues.end())
		{
			ImGui::SeparatorText("Selected Invocation Details");
			const std::string editIdentity = Found->strBindingId + "/" +
				Found->strOccurrenceId;
			if (m_strEffectEditIdentity != editIdentity)
			{
				m_EffectCueEditDraft = *Found;
				m_strEffectEditIdentity = editIdentity;
			}
			VALTAN_PRODUCT_EFFECT_CUE_VIEW& Draft = m_EffectCueEditDraft;
            bool bApplyDetailEdit = false;
            ImGui::BeginDisabled(!bPatternMutationAdmitted || nullptr == m_pBalanceTool);
			ImGui::TextDisabled("Cue: %s", Draft.strBindingId.c_str());
			ImGui::TextDisabled("Occurrence: %s", Draft.strOccurrenceId.c_str());
			bApplyDetailEdit |= RenderEffectAssetPicker("Effect asset", Draft.strEffectAssetId);
			bool linkedAnimation = !Draft.bUsesStageClock;
			ImGui::BeginDisabled(pStage->ClipOccurrences.empty());
			if (ImGui::Checkbox("Link timing to Animation", &linkedAnimation))
			{
				bApplyDetailEdit = true;
				const uint32_t stageStart = Draft.bUsesStageClock ? Draft.iStageOffsetMs :
					Resolve_ClipSourceToStageMs(*pStage, Draft.strClipOccurrenceId, Draft.iSourceStartMs);
				const uint32_t stageEnd = Draft.bHasSourceEnd ? (Draft.bUsesStageClock ? Draft.iSourceEndMs :
					Resolve_ClipSourceToStageMs(*pStage, Draft.strClipOccurrenceId, Draft.iSourceEndMs)) : 0u;
				Draft.bUsesStageClock = !linkedAnimation;
				if (Draft.bUsesStageClock)
				{
					Draft.strClipOccurrenceId.clear(); Draft.iStageOffsetMs = stageStart;
					Draft.iSourceStartMs = stageStart; Draft.iSourceEndMs = stageEnd;
					Draft.strRepeatPolicy = "once";
				}
				else
				{
					Draft.strClipOccurrenceId = pStage->ClipOccurrences.front().strClipOccurrenceId;
					Draft.iSourceStartMs = pStage->ClipOccurrences.front().iSourceStartMs;
					Draft.iStageOffsetMs = 0u; Draft.iSourceEndMs = 0u; Draft.bHasSourceEnd = false;
					Draft.strStopPolicy = "natural"; Draft.eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
				}
			}
			ImGui::EndDisabled();
			ImGui::TextDisabled("Clock: %s", Draft.bUsesStageClock ? "Stage (independent Effect)" : "Animation source");
			if (!Draft.bUsesStageClock && RenderClipOccurrencePicker(
					"Clip occurrence", Draft.strClipOccurrenceId))
			{
                bApplyDetailEdit = true;
				const auto clip = std::find_if(
					pStage->ClipOccurrences.begin(), pStage->ClipOccurrences.end(),
					[&Draft](const VALTAN_CLIP_OCCURRENCE_VIEW& candidate)
					{
						return candidate.strClipOccurrenceId ==
							Draft.strClipOccurrenceId;
					});
				if (pStage->ClipOccurrences.end() != clip)
				{
					Draft.iSourceStartMs = clip->iSourceStartMs;
					Draft.iSourceEndMs = 0u;
					Draft.bHasSourceEnd = false;
					Draft.strStopPolicy = "natural";
					Draft.eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
					if (!clip->bLoop)
						Draft.strRepeatPolicy = "once";
				}
			}
			ImGui::InputScalar(
				Draft.bUsesStageClock ? "Stage start (ms)" : "Source start (ms)", ImGuiDataType_U32,
				Draft.bUsesStageClock ? &Draft.iStageOffsetMs : &Draft.iSourceStartMs);
			if (Draft.bUsesStageClock) Draft.iSourceStartMs = Draft.iStageOffsetMs;
            bApplyDetailEdit |= ImGui::IsItemDeactivatedAfterEdit();
            if (ImGui::Checkbox("Explicit Effect In", &Draft.bHasPlaybackOffset))
            {
                if (!Draft.bHasPlaybackOffset) Draft.iPlaybackOffsetMs = 0u;
                bApplyDetailEdit = true;
            }
            ImGui::BeginDisabled(!Draft.bHasPlaybackOffset);
            ImGui::InputScalar("Effect In (ms)", ImGuiDataType_U32, &Draft.iPlaybackOffsetMs);
            bApplyDetailEdit |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::EndDisabled();
            ImGui::TextDisabled("Effect In selects the Effect's playback age. Start places this box on its selected clock.");
			bool_t bExplicitEnd = Draft.bHasSourceEnd;
			if (ImGui::Checkbox("Explicit cue end", &bExplicitEnd))
			{
                bApplyDetailEdit = true;
				Draft.bHasSourceEnd = bExplicitEnd;
				Draft.strStopPolicy = bExplicitEnd ? "cue_end" : "natural";
				Draft.eStopPolicy = bExplicitEnd ?
					EFFECT_STOP_POLICY::CUE_END : EFFECT_STOP_POLICY::NATURAL;
				Draft.iSourceEndMs = bExplicitEnd ?
					Draft.iSourceStartMs + 1u : 0u;
			}
			ImGui::BeginDisabled(!Draft.bHasSourceEnd);
			ImGui::InputScalar(
				Draft.bUsesStageClock ? "Stage end (ms)" : "Source end (ms)", ImGuiDataType_U32, &Draft.iSourceEndMs);
            bApplyDetailEdit |= ImGui::IsItemDeactivatedAfterEdit();
			ImGui::EndDisabled();

			const bool_t bTargetAnchorAdmitted =
				"LOCK_NEAREST_ON_START" == pPattern->strTargetPolicy ||
				"LOCK_RANDOM_ALIVE_ON_START" == pPattern->strTargetPolicy ||
				"LOCK_RANDOM_ALIVE_BEHIND_ON_START" == pPattern->strTargetPolicy;
			const bool_t bFixedArenaAnchorAdmitted =
				pPattern->ServerMotion.has_value() &&
				("LEAP_TO_ANCHOR" == pPattern->ServerMotion->strKind ||
				 "LEAP_TO_TARGET" == pPattern->ServerMotion->strKind);
			const bool_t bArenaApproachAdmitted = pPattern->ServerMotion.has_value() &&
				"LEAP_TO_ANCHOR" == pPattern->ServerMotion->strKind &&
				pPattern->ServerMotion->bMoveToAnchorBeforeTakeoff;
			const bool_t bArenaFacingAnchorAdmitted = bArenaApproachAdmitted &&
				"LOCK_FACING_ON_START" == pPattern->strAimPolicy &&
				"LOCK_RANDOM_ALIVE_ON_START" == pPattern->strTargetPolicy;
			const auto StageOwner = std::find_if(pPattern->Stages.begin(), pPattern->Stages.end(),
				[&Draft](const VALTAN_STAGE_VIEW& Candidate) { return Candidate.strActionId == Draft.strActionId; });
			const bool_t bStageCenterFollow = StageOwner != pPattern->Stages.end() &&
				("NEAREST_EACH_TICK" == StageOwner->strAimTargetPolicy ||
				 "PATTERN_TARGET" == StageOwner->strAimTargetPolicy) &&
				std::any_of(pPattern->Stages.begin(), StageOwner + 1u,
					[](const VALTAN_STAGE_VIEW& Candidate) {
						return Candidate.Motion.has_value() &&
							"TO_ARENA_CENTER" == Candidate.Motion->strKind;
					});
			const bool_t bArenaTargetFollowAnchorAdmitted =
				bStageCenterFollow || (bArenaApproachAdmitted &&
				"TRACK_TARGET_EACH_TICK" == pPattern->strAimPolicy &&
				"LOCK_RANDOM_ALIVE_ON_START" == pPattern->strTargetPolicy);
			const char* anchorLabel = Draft.strAnchorSlotId == "root" ? "Boss" :
				Draft.strAnchorSlotId == "map" ? "Map (world coordinates)" : Draft.strAnchorSlotId.c_str();
			if (ImGui::BeginCombo("Anchor", anchorLabel))
			{
				for (const char_t* const pAnchor :
					{ "root", "map", "pattern.target.snapshot", "pattern.landing.snapshot", "arena.center",
					  "arena.center.facing", "arena.center.target-follow" })
				{
					const bool_t bAllowed = "root" == std::string_view(pAnchor) || "map" == std::string_view(pAnchor) ||
						("pattern.target.snapshot" == std::string_view(pAnchor) &&
						 bTargetAnchorAdmitted) ||
						("pattern.landing.snapshot" == std::string_view(pAnchor) &&
						 bFixedArenaAnchorAdmitted) ||
						("arena.center" == std::string_view(pAnchor) &&
						 bFixedArenaAnchorAdmitted) ||
						("arena.center.facing" == std::string_view(pAnchor) &&
						 bArenaFacingAnchorAdmitted) ||
						("arena.center.target-follow" == std::string_view(pAnchor) &&
						 bArenaTargetFollowAnchorAdmitted);
					ImGui::BeginDisabled(!bAllowed);
					if (ImGui::Selectable(
							"root" == std::string_view(pAnchor) ? "Boss" :
							"map" == std::string_view(pAnchor) ? "Map (world coordinates)" : pAnchor,
							Draft.strAnchorSlotId == pAnchor) && bAllowed)
					{
                        bApplyDetailEdit = true;
						Draft.strAnchorSlotId = pAnchor;
						if ("root" != Draft.strAnchorSlotId)
						{
							const bool_t bTargetFollow =
								"arena.center.target-follow" == Draft.strAnchorSlotId;
							Draft.strFollowPolicy =
								bTargetFollow ? "follow" : "snapshot";
							Draft.eFollowPolicy = bTargetFollow ?
								EFFECT_FOLLOW_POLICY::FOLLOW :
								EFFECT_FOLLOW_POLICY::SNAPSHOT;
						}
					}
					ImGui::EndDisabled();
				}
				ImGui::EndCombo();
			}
			if ("map" == Draft.strAnchorSlotId)
				ImGui::TextDisabled("Position and Rotation are fixed world coordinates; this Effect does not follow the Boss.");
			if ("arena.center.facing" == Draft.strAnchorSlotId)
			{
				ImGui::TextDisabled(
					"Position Basis = Authored Landing Center");
				ImGui::TextDisabled(
					"Facing Basis = Server Locked Pattern Facing");
			}
			else if ("arena.center.target-follow" == Draft.strAnchorSlotId)
			{
				ImGui::TextDisabled(
					"Position Basis = Authored Landing Center");
				ImGui::TextDisabled(
					"Facing Basis = Current Locked Target (Server Tick)");
			}
			else if ("pattern.landing.snapshot" == Draft.strAnchorSlotId)
			{
				ImGui::TextDisabled("Position Basis = Server Locked Landing (Preview: authored landing)");
				ImGui::TextDisabled("Facing Basis = Locked Pattern Facing");
			}
			else if ("pattern.target.snapshot" == Draft.strAnchorSlotId)
			{
				ImGui::TextDisabled(
					"Position Basis = Target Snapshot Position");
				ImGui::TextDisabled(
					"Facing Basis = Player Snapshot Yaw");
			}
			if ("root" == Draft.strAnchorSlotId)
			{
				if (ImGui::BeginCombo(
						"Follow policy", Draft.strFollowPolicy.c_str()))
				{
					for (const char_t* const pPolicy : { "follow", "snapshot" })
					{
						if (ImGui::Selectable(
								pPolicy, Draft.strFollowPolicy == pPolicy))
						{
                            bApplyDetailEdit = true;
							Draft.strFollowPolicy = pPolicy;
							Draft.eFollowPolicy = "snapshot" ==
								Draft.strFollowPolicy ?
								EFFECT_FOLLOW_POLICY::SNAPSHOT :
								EFFECT_FOLLOW_POLICY::FOLLOW;
						}
					}
					ImGui::EndCombo();
				}
			}
			else
			{
				ImGui::TextDisabled("Follow policy: %s (required by anchor)",
					"arena.center.target-follow" == Draft.strAnchorSlotId ?
						"follow" : "snapshot");
			}
			const auto selectedClip = std::find_if(
				pStage->ClipOccurrences.begin(), pStage->ClipOccurrences.end(),
				[&Draft](const VALTAN_CLIP_OCCURRENCE_VIEW& clip)
				{
					return clip.strClipOccurrenceId ==
						Draft.strClipOccurrenceId;
				});
			const bool_t bSelectedClipLoops = !Draft.bUsesStageClock &&
				pStage->ClipOccurrences.end() != selectedClip && selectedClip->bLoop;
			if (ImGui::BeginCombo("Repeat policy", Draft.strRepeatPolicy.c_str()))
			{
				if (ImGui::Selectable("once", "once" == Draft.strRepeatPolicy))
                { Draft.strRepeatPolicy = "once"; bApplyDetailEdit = true; }
				ImGui::BeginDisabled(!bSelectedClipLoops);
				if (ImGui::Selectable(
						"each_loop", "each_loop" == Draft.strRepeatPolicy) &&
					bSelectedClipLoops)
				{
					Draft.strRepeatPolicy = "each_loop";
                    bApplyDetailEdit = true;
				}
				ImGui::EndDisabled();
				ImGui::EndCombo();
			}
			ImGui::DragFloat3(
				"Position", &Draft.LocalTransform.vPosition.x, 0.01f);
            bApplyDetailEdit |= ImGui::IsItemDeactivatedAfterEdit();
			ImGui::DragFloat3(
				"Rotation (deg)",
				&Draft.LocalTransform.vRotationDegrees.x, 0.5f);
            bApplyDetailEdit |= ImGui::IsItemDeactivatedAfterEdit();
			ImGui::DragFloat3(
				"Scale", &Draft.LocalTransform.vScale.x, 0.01f, 0.001f, 1000.f);
            bApplyDetailEdit |= ImGui::IsItemDeactivatedAfterEdit();
			const bool_t bCompositionScalePolicy =
				0u == Draft.strBindingId.rfind("cue.valtan.composition.", 0u);
			ImGui::BeginDisabled(!bCompositionScalePolicy);
			if (ImGui::BeginCombo("Scale policy", Draft.strScalePolicy.c_str()))
			{
				for (const char_t* const pPolicy :
					{ "OWNER_RELATIVE", "GAMEPLAY_FOOTPRINT", "ARENA_ABSOLUTE" })
				{
					if (!ImGui::Selectable(
							pPolicy, Draft.strScalePolicy == pPolicy))
						continue;
                    bApplyDetailEdit = true;
					Draft.strScalePolicy = pPolicy;
					Draft.bHasExplicitScalePolicy = true;
					if ("OWNER_RELATIVE" == Draft.strScalePolicy)
					{
						Draft.eScalePolicy =
							VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE;
						Draft.vWorldScale = { 1.f, 1.f, 1.f };
					}
					else
					{
						Draft.eScalePolicy = "GAMEPLAY_FOOTPRINT" ==
							Draft.strScalePolicy ?
							VALTAN_PATTERN_EFFECT_SCALE_POLICY::GAMEPLAY_FOOTPRINT :
							VALTAN_PATTERN_EFFECT_SCALE_POLICY::ARENA_ABSOLUTE;
						Draft.vWorldScale = { 1.5f, 1.5f, 1.5f };
					}
				}
				ImGui::EndCombo();
			}
			ImGui::EndDisabled();
			if (!bCompositionScalePolicy)
			{
				ImGui::TextDisabled(
					"This admitted legacy cue keeps its canonical scalePolicy kind; tune local Scale above or create a composition cue.");
			}
            ImGui::EndDisabled();
            ImGui::TextDisabled("Changes apply to the preview when the edit ends. Save Source commits the staged values.");
			ImGui::BeginDisabled(
				!bPatternMutationAdmitted || nullptr == m_pBalanceTool);
			if (ImGui::Button("Update Invocation") || bApplyDetailEdit)
			{
				std::string Status;
                if (Apply_EffectOccurrenceDetails(*pPattern, *pStage, *Found, Draft, Status))
				{
					m_strStatus = std::move(Status);
					m_strEffectEditIdentity.clear();
					ImGui::EndDisabled();
					return;
				}
				m_strStatus = std::move(Status);
			}
			ImGui::SameLine();
			if (ImGui::Button("Remove Invocation"))
			{
				std::string Status;
				if (m_pBalanceTool->Remove_ValtanStageEffectCue(
						pPattern->strPatternId, pStage->strStageId,
						pStage->strActionId, Found->strBindingId,
						Found->strOccurrenceId, Found->strEffectAssetId,
						Found->strClipOccurrenceId, Status))
				{
					m_strStatus = std::move(Status);
					m_strSelectedStableId = pStage->strStageId;
					m_strEffectEditIdentity.clear();
					ImGui::EndDisabled();
					return;
				}
				m_strStatus = std::move(Status);
			}
			ImGui::EndDisabled();

			const VALTAN_PATTERN_VIEW* pSavedPattern = nullptr;
			const VALTAN_STAGE_VIEW* pSavedStage = nullptr;
			const VALTAN_PRODUCT_EFFECT_CUE_VIEW* pSavedCue = nullptr;
			if (VALTAN_VIEW_ADMISSION::ADMITTED == m_eAdmission)
			{
				pSavedPattern = Find_SelectedPattern();
				if (nullptr != pSavedPattern &&
					pSavedPattern->strPatternId == pPattern->strPatternId)
				{
					const auto savedStage = std::find_if(
						pSavedPattern->Stages.begin(), pSavedPattern->Stages.end(),
						[pStage](const VALTAN_STAGE_VIEW& candidate)
						{
							return candidate.strStageId == pStage->strStageId &&
								candidate.strActionId == pStage->strActionId;
						});
					if (pSavedPattern->Stages.end() != savedStage)
					{
						pSavedStage = &*savedStage;
						const std::size_t iSavedOccurrenceCount =
							static_cast<std::size_t>(std::count_if(
								pSavedStage->ProductCues.begin(),
								pSavedStage->ProductCues.end(),
								[Found](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& cue)
								{
									return cue.strOccurrenceId ==
										Found->strOccurrenceId;
								}));
						const auto savedCue = std::find_if(
							pSavedStage->ProductCues.begin(),
							pSavedStage->ProductCues.end(),
							[Found](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& cue)
							{
								return cue.strOccurrenceId ==
									Found->strOccurrenceId;
							});
						if (1u == iSavedOccurrenceCount &&
							pSavedStage->ProductCues.end() != savedCue &&
							SameSavedValtanEffectCue(*Found, *savedCue))
						{
							pSavedCue = &*savedCue;
						}
					}
				}
			}
			const bool_t bEffectBodyDeepLinkAdmitted =
				nullptr != pSavedPattern && nullptr != pSavedStage &&
				nullptr != pSavedCue;
			ImGui::BeginDisabled(!bEffectBodyDeepLinkAdmitted);
			if (ImGui::Button(bEffectBodyDeepLinkAdmitted ?
					"Open Saved Product Effect Body" :
					"Save + Reload Before Opening Effect Body") &&
				bEffectBodyDeepLinkAdmitted)
			{
				Request_EffectOwner(*pSavedPattern, *pSavedStage, *pSavedCue);
			}
			ImGui::EndDisabled();
			if (!bEffectBodyDeepLinkAdmitted)
			{
				ImGui::TextDisabled(
					"Effect body deep-link requires this invocation to exist in the saved Pattern. Use Save, then reload.");
			}
		}

		ImGui::SeparatorText("Add Invocation");
		RenderEffectAssetPicker("New Effect asset", m_strEffectAddAssetId);
		ImGui::TextDisabled("Adds an independent Stage-clock Effect; Animation can be linked in its details.");
		ImGui::InputScalar("Stage start (ms)##NewEffect", ImGuiDataType_U32, &m_iEffectV2AddStartMs);
		ImGui::BeginDisabled(!bPatternMutationAdmitted || nullptr == m_pBalanceTool ||
			m_strEffectAddAssetId.empty() || m_iEffectV2AddStartMs >= pStage->iDurationMs);
		if (ImGui::Button("Add Effect Invocation"))
		{
			VALTAN_PRODUCT_EFFECT_CUE_VIEW Cue;
			Cue.strBindingId = BuildNextCompositionEffectCueId(*pPattern, *pStage);
			Cue.strOccurrenceId = Cue.strBindingId + ".occurrence.01";
			Cue.strPatternId = pPattern->strPatternId; Cue.strStageId = pStage->strStageId;
			Cue.strActionId = pStage->strActionId; Cue.strEffectAssetId = m_strEffectAddAssetId;
			Cue.strAnchorSlotId = "root"; Cue.eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
			Cue.strFollowPolicy = "follow"; Cue.eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
			Cue.strStopPolicy = "natural"; Cue.strRepeatPolicy = "once";
			Cue.eScalePolicy = VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE;
			Cue.strScalePolicy = "OWNER_RELATIVE"; Cue.bHasExplicitScalePolicy = true;
			Cue.bUsesStageClock = true; Cue.iStageOffsetMs = m_iEffectV2AddStartMs;
			Cue.iSourceStartMs = Cue.iStageOffsetMs; Cue.iStageDurationMs = pStage->iDurationMs;
			std::string Status;
			if (ValidateValtanEffectInvocationClock(m_pAnimationTool, *pStage, Cue, Status) &&
				m_pBalanceTool->Add_ValtanStageEffectCue(pPattern->strPatternId, pStage->strStageId,
					pStage->strActionId, Cue, Status))
			{
				m_strSelectedStableId = Cue.strOccurrenceId; m_strEffectEditIdentity.clear();
				Refresh_PatternLocalPreviewAfterMutation(pPattern, Status);
			}
			m_strStatus = std::move(Status);
		}
		ImGui::EndDisabled();

		CBalanceTool::PATTERN_STAGE_EDIT ColliderDraft;
		std::string ColliderDraftStatus;
		const bool_t bColliderDraftReady = nullptr != m_pBalanceTool &&
			m_pBalanceTool->Get_ValtanStageDraft(
				pPattern->strPatternId, pStage->strStageId,
				ColliderDraft, ColliderDraftStatus);
		const bool_t bHasServerCollider = bColliderDraftReady ?
			"NONE" != ColliderDraft.hitShape : pStage->Has_HitShape();
		const bool_t bColliderAddAdmitted = bColliderDraftReady &&
			ColliderDraft.colliderAddAdmitted;
		const bool_t bColliderTuneAdmitted = bColliderDraftReady &&
			ColliderDraft.colliderTuneAdmitted;
		const bool_t bColliderRemoveAdmitted = bColliderDraftReady &&
			ColliderDraft.colliderRemoveAdmitted;
		const char_t* const pColliderOwnerLabel = bHasServerCollider ?
			(bColliderRemoveAdmitted ?
				"Tune / Remove Existing Server Collider / Hit Timing" :
				(bColliderTuneAdmitted ?
					"Tune Existing Server Collider / Hit Timing" :
					"View Existing Server Collider Authority")) :
			(bColliderAddAdmitted ?
				"Add Manual Audition Server Collider / Hit Schedule" :
				"View Collider Authority (New Add Unavailable)");
		if (ImGui::Button(pColliderOwnerLabel))
		{
			m_eDetailOwner = DETAIL_OWNER::GAMEPLAY_STAGE;
			m_strSelectedStableId = pStage->strStageId + "/collider";
		}
		ImGui::TextDisabled(
			bHasServerCollider ?
				(bColliderRemoveAdmitted ?
					"This manual audition Stage admits collider tune/remove; Effect is visual only." :
					(bColliderTuneAdmitted ?
						"This existing collider admits typed tuning. Canonical removal remains unavailable; Effect is visual only." :
						"This existing Server collider is inspection-only in the current admission state.")) :
				(bColliderAddAdmitted ?
					"No Server collider exists. New Add is available only because this is a non-WAIT manual audition Stage; no geometry is inferred from the Effect." :
					"No Server collider exists. New Add is unavailable on this canonical/WAIT Stage; Workbench never infers hit geometry from a visual Effect or its anchor."));
		if (!bColliderDraftReady && !ColliderDraftStatus.empty())
			ImGui::TextWrapped("Collider admission: %s", ColliderDraftStatus.c_str());
		return;
	}
	if (DETAIL_OWNER::SOUND == m_eDetailOwner)
	{
		ImGui::SeparatorText("Pattern Sound Typed Source");
		ImGui::TextWrapped(
			"Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json");
		ImGui::TextDisabled(
			"Edit Sound rows here. Sequencer Save joins only dirty Pattern, Sound, and Effect V2 owners; Camera stays in its typed read-only/deep-link boundary.");
		if (nullptr == pStage || nullptr == pPatternSounds ||
			nullptr == m_pAnimationTool)
		{
			ImGui::TextDisabled(
				"The strict Pattern Sound source draft is not admitted for this Stage.");
			if (!m_strSoundStatus.empty())
				ImGui::TextWrapped("Sound: %s", m_strSoundStatus.c_str());
			return;
		}
		const auto Found = std::find_if(
			pPatternSounds->Cues.begin(), pPatternSounds->Cues.end(),
			[this](const VALTAN_PATTERN_SOUND_CUE& Cue)
			{
				return Cue.strOccurrenceId == m_strSelectedStableId;
			});
		const bool_t bSelectedSoundExact =
			Found != pPatternSounds->Cues.end() &&
			Found->strPatternId == pPattern->strPatternId &&
			Found->strStageId == pStage->strStageId &&
			Found->strActionId == pStage->strActionId;

		const bool_t bFreshSoundOwnerAdmitted =
			VALTAN_VIEW_ADMISSION::ADMITTED == m_eAdmission && bMutationAdmitted;
		const bool_t bSoundMutationAdmitted =
			bMutationAdmitted && bFreshSoundOwnerAdmitted;
		std::string SoundLifecycleStatus;
		const bool_t bSoundSourceCommitAdmitted =
			bFreshSoundOwnerAdmitted &&
			m_pAnimationTool->
				Can_CommitValtanCompositionPatternSoundGeneration(
					SoundLifecycleStatus);
		if (bPatternSoundDirty)
		{
			ImGui::TextColored(ImVec4(1.f, 0.72f, 0.2f, 1.f),
				"Unsaved Sound changes (saved by the Sequencer Save button)");
		}
		if (!bSoundSourceCommitAdmitted && !SoundLifecycleStatus.empty())
		{
			ImGui::TextColored(ImVec4(1.f, 0.58f, 0.25f, 1.f),
				"LIVE OCCURRENCE PIN: %s", SoundLifecycleStatus.c_str());
		}

		if (!bPatternSoundDirty)
			m_bConfirmDiscardPatternSoundDraft = false;
		ImGui::BeginDisabled(!bSoundSourceCommitAdmitted);
		if (bSoundSourceCommitAdmitted && ImGui::Button(bPatternSoundDirty ?
				"Review Discard Sound Draft" : "Reload Sound Owner"))
		{
			if (bPatternSoundDirty)
			{
				m_bConfirmDiscardPatternSoundDraft = true;
			}
			else
			{
				std::string Status;
				const bool_t bReloaded = m_pAnimationTool->
					Reload_ValtanCompositionPatternSounds(Status);
				if (bReloaded)
				{
					m_PatternSoundEvents =
						m_pAnimationTool->Collect_ValtanCompositionPatternSoundEvents();
					m_bSoundFilterDirty = true;
				}
				m_strSoundStatus = std::move(Status);
				Invalidate_TimelineCache();
				ImGui::EndDisabled();
				return;
			}
		}
		ImGui::EndDisabled();
		if (bPatternSoundDirty && m_bConfirmDiscardPatternSoundDraft)
		{
			ImGui::TextColored(
				ImVec4(1.f, 0.45f, 0.25f, 1.f),
				"This permanently discards the unsaved Sound owner draft only.");
			ImGui::BeginDisabled(!bSoundSourceCommitAdmitted);
			if (bSoundSourceCommitAdmitted &&
				ImGui::Button("Confirm Discard + Reload Sound Owner"))
			{
				std::string Status;
				const bool_t bReloaded =
					m_pAnimationTool->Reload_ValtanCompositionPatternSounds(
						Status);
				if (bReloaded)
				{
					m_PatternSoundEvents =
						m_pAnimationTool->Collect_ValtanCompositionPatternSoundEvents();
					m_bSoundFilterDirty = true;
				}
				m_bConfirmDiscardPatternSoundDraft = false;
				m_strSoundStatus = std::move(Status);
				Invalidate_TimelineCache();
				ImGui::EndDisabled();
				return;
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Button("Cancel Discard"))
				m_bConfirmDiscardPatternSoundDraft = false;
		}

		ImGui::SeparatorText("Add Sound Row");
		auto AddClip = std::find_if(
			pStage->ClipOccurrences.begin(), pStage->ClipOccurrences.end(),
			[this](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
			{
				return Clip.strClipOccurrenceId ==
					m_strSoundAddClipOccurrenceId;
			});
		if (AddClip == pStage->ClipOccurrences.end() &&
			!pStage->ClipOccurrences.empty())
		{
			AddClip = pStage->ClipOccurrences.begin();
			m_strSoundAddClipOccurrenceId = AddClip->strClipOccurrenceId;
			m_iSoundAddStartMs = AddClip->iSourceStartMs;
			m_eSoundAddRepeatPolicy =
				VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
		}
		if (m_PatternSoundEvents.end() == std::find(
				m_PatternSoundEvents.begin(), m_PatternSoundEvents.end(),
				m_strSoundAddEvent) && !m_PatternSoundEvents.empty())
		{
			m_strSoundAddEvent = m_PatternSoundEvents.front();
		}

		ImGui::BeginDisabled(!bSoundMutationAdmitted);
		ImGui::SetNextItemWidth(-1.f);
		const char_t* const pAddClipLabel =
			AddClip == pStage->ClipOccurrences.end() ?
				"Select clip occurrence" : AddClip->strClipOccurrenceId.c_str();
		if (ImGui::BeginCombo("Clip Occurrence##AddSoundRow", pAddClipLabel))
		{
			for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
				pStage->ClipOccurrences)
			{
				const bool_t bSelected = Clip.strClipOccurrenceId ==
					m_strSoundAddClipOccurrenceId;
				if (ImGui::Selectable(
						Clip.strClipOccurrenceId.c_str(), bSelected) && !bSelected)
				{
					m_strSoundAddClipOccurrenceId = Clip.strClipOccurrenceId;
					m_iSoundAddStartMs = Clip.iSourceStartMs;
					m_eSoundAddRepeatPolicy =
						VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
				}
				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		AddClip = std::find_if(
			pStage->ClipOccurrences.begin(), pStage->ClipOccurrences.end(),
			[this](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
			{
				return Clip.strClipOccurrenceId ==
					m_strSoundAddClipOccurrenceId;
			});

		ImGui::SetNextItemWidth(-1.f);
		const char_t* const pAddEventLabel = m_strSoundAddEvent.empty() ?
			"Select admitted Valtan event" : m_strSoundAddEvent.c_str();
		if (ImGui::BeginCombo("Sound Event##AddSoundRow", pAddEventLabel))
		{
			for (const std::string& Event : m_PatternSoundEvents)
			{
				const bool_t bSelected = Event == m_strSoundAddEvent;
				if (ImGui::Selectable(Event.c_str(), bSelected) && !bSelected)
					m_strSoundAddEvent = Event;
				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		uint32_t iAddMinimumStartMs = 0u;
		uint32_t iAddMaximumStartMs = 0u;
		bool_t bAddLoopClip = false;
		std::string AddWindowStatus;
		const bool_t bAddWindowReady =
			AddClip != pStage->ClipOccurrences.end() &&
			m_pAnimationTool->Resolve_ValtanCompositionPatternSoundWindow(
				*pStage, AddClip->strClipOccurrenceId,
				iAddMinimumStartMs, iAddMaximumStartMs,
				bAddLoopClip, AddWindowStatus);
		if (bAddWindowReady)
		{
			m_iSoundAddStartMs = std::clamp(
				m_iSoundAddStartMs, iAddMinimumStartMs, iAddMaximumStartMs);
			if (!bAddLoopClip &&
				VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
					m_eSoundAddRepeatPolicy)
			{
				m_eSoundAddRepeatPolicy =
					VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
			}
		}
		ImGui::BeginDisabled(!bAddWindowReady);
		const uint32_t iAddStepMs = 1u;
		const uint32_t iAddFastStepMs = 100u;
		ImGui::SetNextItemWidth(210.f);
		if (ImGui::InputScalar("Source startMs##AddSoundRow",
				ImGuiDataType_U32, &m_iSoundAddStartMs,
				&iAddStepMs, &iAddFastStepMs, "%u"))
		{
			m_iSoundAddStartMs = std::clamp(
				m_iSoundAddStartMs, iAddMinimumStartMs, iAddMaximumStartMs);
		}
		ImGui::EndDisabled();

		const char_t* const pAddRepeatLabel =
			VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
				m_eSoundAddRepeatPolicy ? "each_loop" : "once";
		ImGui::SetNextItemWidth(210.f);
		if (ImGui::BeginCombo(
				"Repeat Policy##AddSoundRow", pAddRepeatLabel))
		{
			const bool_t bOnce = VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE ==
				m_eSoundAddRepeatPolicy;
			if (ImGui::Selectable("once", bOnce) && !bOnce)
			{
				m_eSoundAddRepeatPolicy =
					VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
			}
			ImGui::BeginDisabled(!bAddLoopClip);
			const bool_t bEachLoop =
				VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
					m_eSoundAddRepeatPolicy;
			if (ImGui::Selectable("each_loop", bEachLoop) && !bEachLoop)
			{
				m_eSoundAddRepeatPolicy =
					VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP;
			}
			ImGui::EndDisabled();
			ImGui::EndCombo();
		}
		const bool_t bCanAddSoundRow = bAddWindowReady &&
			!m_strSoundAddEvent.empty();
		ImGui::BeginDisabled(!bCanAddSoundRow);
		if (ImGui::Button("Add Sound Row"))
		{
			VALTAN_PATTERN_SOUND_CUE_ROW_ID CreatedRowId;
			std::string Status;
			if (m_pAnimationTool->Add_ValtanCompositionPatternSound(
					*pPattern, *pStage, m_strSoundAddClipOccurrenceId,
					m_strSoundAddEvent, m_iSoundAddStartMs,
					m_eSoundAddRepeatPolicy, CreatedRowId, Status))
			{
				m_strSelectedStableId = CreatedRowId.strOccurrenceId;
			}
			m_strSoundStatus = std::move(Status);
			Invalidate_TimelineCache();
			ImGui::EndDisabled();
			ImGui::EndDisabled();
			return;
		}
		ImGui::EndDisabled();
		ImGui::EndDisabled();
		if (!bAddWindowReady && !AddWindowStatus.empty())
		{
			ImGui::TextColored(ImVec4(1.f, 0.35f, 0.25f, 1.f),
				"%s", AddWindowStatus.c_str());
		}

		ImGui::SeparatorText("Selected Sound Row");
		if (!bSelectedSoundExact)
		{
			ImGui::TextDisabled(
				"Select an existing Sound timeline row or add one for this exact Pattern/Stage/clip occurrence.");
			if (!m_strSoundStatus.empty())
				ImGui::TextWrapped("Sound: %s", m_strSoundStatus.c_str());
			return;
		}
		ImGui::BeginDisabled(!bSoundMutationAdmitted);
		if (ImGui::Button("Remove Selected Sound Row"))
		{
			VALTAN_PATTERN_SOUND_CUE_ROW_ID RowId;
			RowId.strBindingId = Found->strBindingId;
			RowId.strOccurrenceId = Found->strOccurrenceId;
			std::string Status;
			if (m_pAnimationTool->Remove_ValtanCompositionPatternSound(
					*pPattern, *pStage, RowId, Status))
			{
				m_strSelectedStableId.clear();
			}
			m_strSoundStatus = std::move(Status);
			Invalidate_TimelineCache();
			ImGui::EndDisabled();
			return;
		}
		ImGui::EndDisabled();

		ImGui::TextDisabled("bindingId: %s", Found->strBindingId.c_str());
		ImGui::TextDisabled("occurrenceId: %s", Found->strOccurrenceId.c_str());
		ImGui::TextDisabled("clipOccurrenceId: %s",
			Found->strClipOccurrenceId.c_str());
		ImGui::TextDisabled("Sound Bank (derived): %s",
			Found->strSoundBank.c_str());

		std::string CandidateEvent = Found->strSoundEvent;
		uint32_t iCandidateStartMs = Found->iStartMs;
		VALTAN_PATTERN_SOUND_REPEAT_POLICY eCandidateRepeat =
			Found->eRepeatPolicy;
		bool_t bSubmitPatch = false;
		ImGui::BeginDisabled(!bSoundMutationAdmitted);
		ImGui::SetNextItemWidth(-1.f);
		if (ImGui::BeginCombo("Sound Event", CandidateEvent.c_str()))
		{
			for (const std::string& Event : m_PatternSoundEvents)
			{
				const bool_t bSelected = Event == CandidateEvent;
				if (ImGui::Selectable(Event.c_str(), bSelected) && !bSelected)
				{
					CandidateEvent = Event;
					bSubmitPatch = true;
				}
				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		uint32_t iMinimumStartMs = 0u;
		uint32_t iMaximumStartMs = 0u;
		bool_t bLoopClip = false;
		std::string WindowStatus;
		const bool_t bWindowReady =
			m_pAnimationTool->Resolve_ValtanCompositionPatternSoundWindow(
				*pStage, Found->strClipOccurrenceId,
				iMinimumStartMs, iMaximumStartMs, bLoopClip, WindowStatus);
		ImGui::BeginDisabled(!bWindowReady);
		const uint32_t iStepMs = 1u;
		const uint32_t iFastStepMs = 100u;
		ImGui::SetNextItemWidth(210.f);
		if (ImGui::InputScalar("Source startMs", ImGuiDataType_U32,
				&iCandidateStartMs, &iStepMs, &iFastStepMs, "%u"))
		{
			iCandidateStartMs = std::clamp(
				iCandidateStartMs, iMinimumStartMs, iMaximumStartMs);
			bSubmitPatch = true;
		}
		ImGui::EndDisabled();
		if (bWindowReady)
		{
			ImGui::TextDisabled("Admitted source window: %u..%u ms | loop=%s",
				iMinimumStartMs, iMaximumStartMs,
				bLoopClip ? "true" : "false");
		}
		else
		{
			ImGui::TextColored(ImVec4(1.f, 0.35f, 0.25f, 1.f),
				"%s", WindowStatus.c_str());
		}

		const char_t* const pRepeatLabel =
			VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
				eCandidateRepeat ? "each_loop" : "once";
		ImGui::SetNextItemWidth(210.f);
		if (ImGui::BeginCombo("Repeat Policy", pRepeatLabel))
		{
			const bool_t bOnce =
				VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE == eCandidateRepeat;
			if (ImGui::Selectable("once", bOnce) && !bOnce)
			{
				eCandidateRepeat = VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
				bSubmitPatch = true;
			}
			ImGui::BeginDisabled(!bLoopClip);
			const bool_t bEachLoop =
				VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
					eCandidateRepeat;
			if (ImGui::Selectable("each_loop", bEachLoop) && !bEachLoop)
			{
				eCandidateRepeat =
					VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP;
				bSubmitPatch = true;
			}
			ImGui::EndDisabled();
			ImGui::EndCombo();
		}
		ImGui::EndDisabled();

		if (bSubmitPatch)
		{
			std::string Status;
			(void)m_pAnimationTool->Patch_ValtanCompositionPatternSound(
				*pPattern, *pStage, Found->strOccurrenceId,
				CandidateEvent, iCandidateStartMs,
				eCandidateRepeat, Status);
			m_strSoundStatus = std::move(Status);
			Invalidate_TimelineCache();
			return;
		}
		if (!m_strSoundStatus.empty())
			ImGui::TextWrapped("Sound: %s", m_strSoundStatus.c_str());
		ImGui::TextDisabled(
			"Timeline playback remains inspection-only for Sound because the engine has no seek-safe per-cue stop handle; source authoring and Save are fully typed here.");
		return;
	}
	if (DETAIL_OWNER::CAMERA == m_eDetailOwner)
	{
		const auto Found = std::find_if(
			pStage->CameraInvocations.begin(), pStage->CameraInvocations.end(),
			[this](const VALTAN_CAMERA_INVOCATION_VIEW& Camera)
			{
				return Camera.strCameraInvocationId == m_strSelectedStableId;
			});
		if (Found != pStage->CameraInvocations.end())
		{
			ImGui::TextWrapped("%s", Found->strCameraCueId.c_str());
			ImGui::Text("Start %u ms | Duration %u ms | %s",
				Found->iStartOffsetMs, Found->iDurationMs,
				Found->strDurationPolicy.c_str());
			if (ImGui::Button("Open Camera Sequence Detail"))
			{
				m_strCameraCueId = Found->strCameraCueId;
				m_bCameraToolOpenRequested = true;
			}
		}
		else
		{
			ImGui::TextDisabled("Selected Camera row is a shake cue; open Animation source detail for its payload.");
		}
		return;
	}
	if (DETAIL_OWNER::WORLD == m_eDetailOwner)
	{
		ImGui::TextWrapped("World event: %s", m_strSelectedStableId.c_str());
		ImGui::TextDisabled(
			"World/Light rows remain Server/Map/Camera-owner data. The Sequencer never writes a duplicate sequence Product.");
		return;
	}
	if (DETAIL_OWNER::COMBAT_OBJECT == m_eDetailOwner)
	{
		const auto Found = std::find_if(
			pStage->CombatObjectEffects.begin(), pStage->CombatObjectEffects.end(),
			[this](const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Object)
			{
				return Object.strCombatObjectArchetypeId == m_strSelectedStableId;
			});
		if (Found != pStage->CombatObjectEffects.end())
		{
			ImGui::TextWrapped("%s", Found->strCombatObjectArchetypeId.c_str());
			if (!Found->strEffectV2GroupId.empty())
				ImGui::Text("Product Effect V2 Group: %s",
					Found->strEffectV2GroupId.c_str());
			ImGui::Text("Saved V1 presentation: %s", Found->strEffectAssetId.c_str());
			ImGui::Text("World origin: %s | direction: %s",
				Found->strOriginPolicy.c_str(), Found->strDirectionPolicy.c_str());
			if (!Found->strArenaAnchor.empty())
				ImGui::Text("Map World anchor: %s", Found->strArenaAnchor.c_str());
			ImGui::TextDisabled("The object owns its world transform and lifetime; Pattern Play schedules its Server spawn.");
			ImGui::Text("Spawn +%u ms | count %u",
				Found->iFirstSpawnOffsetMs, Found->iSpawnValue);
			ImGui::Text("Motion %s | %.2f m/s | %.2f m | life %u ms",
				Found->strKind.c_str(), Found->fSpeedMps,
				Found->fMaximumDistanceM, Found->iLifetimeMs);
			ImGui::Text("Waves %u | interval %u ms | hit offsets %u",
				(std::max)(Found->iSpawnScheduleCount, 1u),
				Found->iSpawnIntervalMs,
				static_cast<uint32_t>(Found->HitOffsetsMs.size()));
			if (bPatternMutationAdmitted && nullptr != m_pBalanceTool &&
				nullptr != pPattern)
			{
				for (const VALTAN_COMBAT_OBJECT_HIT_VIEW& Hit : Found->Hits)
				{
					if ("RING" != Hit.strHitShape)
						continue;
					CBalanceTool::VALTAN_COMBAT_OBJECT_RING_HIT_EDIT RingDraft;
					std::string RingStatus;
					if (!m_pBalanceTool->Get_ValtanCombatObjectRingHitDraft(
							pPattern->strPatternId, pStage->strStageId,
							Found->strCombatObjectArchetypeId, Hit.strHitId,
							RingDraft, RingStatus))
					{
						ImGui::TextDisabled("%s", RingStatus.c_str());
						continue;
					}
					ImGui::PushID(Hit.strHitId.c_str());
					f32_t fInner = static_cast<f32_t>(RingDraft.innerRadiusM);
					f32_t fOuter = static_cast<f32_t>(RingDraft.outerRadiusM);
					bool_t bRingChanged = false;
					ImGui::SetNextItemWidth(140.f);
					bRingChanged |= ImGui::DragFloat("Ring inner (m)", &fInner,
						0.1f, 0.f, 80.f, "%.2f");
					ImGui::SetNextItemWidth(140.f);
					bRingChanged |= ImGui::DragFloat("Ring outer (m)", &fOuter,
						0.1f, 0.f, 80.f, "%.2f");
					if (bRingChanged)
					{
						RingDraft.innerRadiusM = fInner;
						RingDraft.outerRadiusM = fOuter;
						m_strStatus =
							m_pBalanceTool->Set_ValtanCombatObjectRingHitDraft(
								pPattern->strPatternId, pStage->strStageId,
								RingDraft, RingStatus) ?
							RingStatus : "Ring edit rejected: " + RingStatus;
						Invalidate_TimelineCache();
					}
					ImGui::PopID();
				}
				if ("combatobject.valtan.high-jump.target-axe" ==
					Found->strCombatObjectArchetypeId)
				{
					uint32_t iDraftCount = 0u;
					uint32_t iSavedCount = 0u;
					uint32_t iArenaRandomCount = 0u;
					uint32_t iMaximumTotalObjects = 0u;
					std::string AxeStatus;
					if (m_pBalanceTool->Get_ValtanHighJumpAxeCountDraft(
							iDraftCount, iSavedCount, iArenaRandomCount,
							iMaximumTotalObjects, AxeStatus))
					{
						int32_t iCount = static_cast<int32_t>(iDraftCount);
						ImGui::SetNextItemWidth(160.f);
						if (ImGui::SliderInt("Axes per alive player", &iCount,
								1, 8))
						{
							m_strStatus =
								m_pBalanceTool->Set_ValtanHighJumpAxeCountDraft(
									static_cast<uint32_t>(iCount), AxeStatus) ?
								AxeStatus : "Axe edit rejected: " + AxeStatus;
							Invalidate_TimelineCache();
						}
						ImGui::TextDisabled(
							"saved %u | arena random %u | capacity %u",
							iSavedCount, iArenaRandomCount,
							iMaximumTotalObjects);
					}
					else
					{
						ImGui::TextDisabled("%s", AxeStatus.c_str());
					}
				}
			}
			ImGui::TextDisabled(
				"Collider and hit clocks are Server combat-object owner data; Effect visibility does not become hit authority.");
		}
	}
}

// Pattern/Stage에 연결된 semantic 행을 읽어 표시한다. 행의 stable ID를 선택·도구 연결에 사용하고 표시 순서를 저장 identity로 사용하지 않는다.
void Client::CValtanActionWorkbench::Render_SemanticLinkedRows(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const VALTAN_STAGE_VIEW* const pStage)
{
	ImGui::SeparatorText("Selected Pattern / Stage Semantic Links");
	if (nullptr == pPattern)
	{
		ImGui::TextDisabled(
			"Select a Pattern to resolve its typed Animation, Effect, Sound, Camera, World and Server collider links.");
		return;
	}

	ImGui::TextDisabled("Pattern: %s | Stage: %s | Selected occurrence: %s",
		pPattern->strPatternId.c_str(),
		nullptr == pStage ? "NONE" : pStage->strStageId.c_str(),
		m_strSelectedStableId.empty() ? "NONE" :
			m_strSelectedStableId.c_str());
	if (nullptr == pStage)
	{
		ImGui::TextDisabled("Select one Stage to resolve occurrence-level links.");
		return;
	}

	const auto SelectLinkedDetail = [this](
		const DETAIL_OWNER eOwner,
		const std::string& strStableId,
		const std::string& strLabel)
	{
		const bool_t bSelected = eOwner == m_eDetailOwner &&
			strStableId == m_strSelectedStableId;
		if (ImGui::Selectable(strLabel.c_str(), bSelected))
		{
			m_eDetailOwner = eOwner;
			m_strSelectedStableId = strStableId;
		}
	};
	const auto BeginLinkedTable = [](const char_t* const pId)
	{
		if (!ImGui::BeginTable(
				pId, 2,
				ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg |
				ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
		{
			return false;
		}
		ImGui::TableSetupColumn(
			"Semantic linked row", ImGuiTableColumnFlags_WidthStretch, 0.82f);
		ImGui::TableSetupColumn(
			"Typed owner", ImGuiTableColumnFlags_WidthStretch, 0.18f);
		ImGui::TableHeadersRow();
		return true;
	};
	const auto LinkedRow = [&SelectLinkedDetail](
		const DETAIL_OWNER eOwner,
		const std::string& strStableId,
		const std::string& strLabel,
		const char_t* const pOwnerLabel)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		SelectLinkedDetail(eOwner, strStableId, strLabel);
		ImGui::TableSetColumnIndex(1);
		ImGui::TextDisabled("%s", pOwnerLabel);
	};

	if (ImGui::CollapsingHeader(
			"Animation Sequence Slots", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (pStage->ClipOccurrences.empty())
		{
			ImGui::TextDisabled("No Animation occurrence is linked to this Stage.");
		}
		else if (BeginLinkedTable("##CompositionSemanticAnimationRows"))
		{
			for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
				pStage->ClipOccurrences)
			{
				ImGui::PushID(Clip.strClipOccurrenceId.c_str());
				const std::string PlayLabel = 0u == Clip.iPlayMs ?
					"native" : std::to_string(Clip.iPlayMs) + " ms";
				LinkedRow(
					DETAIL_OWNER::ANIMATION, Clip.strClipOccurrenceId,
					Clip.strClipOccurrenceId + " | " + Clip.strClipName +
					" | source " + std::to_string(Clip.iSourceStartMs) +
					" ms | play " + PlayLabel,
					"Animation Detail");
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
	}

	if ((!pStage->ProductCues.empty() ||
			!pStage->IndependentEffectIds.empty()) &&
		ImGui::CollapsingHeader(
			"Exact Effect Invocations", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (!pStage->ProductCues.empty() &&
			BeginLinkedTable("##CompositionSemanticEffectRows"))
		{
			for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue : pStage->ProductCues)
			{
				ImGui::PushID(Cue.strOccurrenceId.c_str());
				const std::string Clock = Cue.bUsesStageClock ?
					"stage clock" : Cue.strClipOccurrenceId;
				LinkedRow(
					DETAIL_OWNER::EFFECT, Cue.strOccurrenceId,
					Cue.strOccurrenceId + " | " + Cue.strEffectAssetId +
					" | " + Clock,
					"Effect Detail");
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
		if (!pStage->IndependentEffectIds.empty())
		{
			for (const std::string& IndependentEffectId :
				pStage->IndependentEffectIds)
			{
				ImGui::BulletText(
					"Independent read-only ref: %s", IndependentEffectId.c_str());
			}
		}
	}

	bool_t bSoundDirty = false;
	std::string SoundStatus;
	const VALTAN_PATTERN_SOUND_CUE_DOCUMENT* const pSounds =
		VALTAN_VIEW_ADMISSION::ADMITTED == m_eAdmission &&
		nullptr != m_pAnimationTool ?
		m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(
			bSoundDirty, SoundStatus) : nullptr;
	bool_t bFoundSound = false;
	if (nullptr != pSounds)
	{
		bFoundSound = std::any_of(
			pSounds->Cues.begin(), pSounds->Cues.end(),
			[pPattern, pStage](const VALTAN_PATTERN_SOUND_CUE& Cue)
			{
				return Cue.strPatternId == pPattern->strPatternId &&
					Cue.strStageId == pStage->strStageId;
			});
	}
	if (bFoundSound && ImGui::CollapsingHeader(
			"Pattern Sound Events", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (BeginLinkedTable("##CompositionSemanticSoundRows"))
		{
			for (const VALTAN_PATTERN_SOUND_CUE& Cue : pSounds->Cues)
			{
				if (Cue.strPatternId != pPattern->strPatternId ||
					Cue.strStageId != pStage->strStageId)
				{
					continue;
				}
				bFoundSound = true;
				ImGui::PushID(Cue.strOccurrenceId.c_str());
				const std::string Resolution = Cue.ResolvedAssetIds.empty() ?
					"UNRESOLVED" :
					std::to_string(Cue.ResolvedAssetIds.size()) +
					" resolved variant(s)";
				LinkedRow(
					DETAIL_OWNER::SOUND, Cue.strOccurrenceId,
					Cue.strOccurrenceId + " | " + Cue.strSoundEvent +
					" | " + Resolution + " | " + Cue.strClipOccurrenceId,
					"Sound Detail");
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
		if (bSoundDirty)
		{
			ImGui::TextDisabled(
				"Rows include the unsaved typed Sound-owner draft generation.");
		}
	}

	bool_t bFoundCamera = !pStage->CameraInvocations.empty();
	if (!bFoundCamera && m_bPatternShakesReady)
	{
		bFoundCamera = std::any_of(
			m_PatternShakes.Cues.begin(), m_PatternShakes.Cues.end(),
			[pPattern, pStage](const VALTAN_PATTERN_SHAKE_CUE& Cue)
			{
				return Cue.strPatternId == pPattern->strPatternId &&
					Cue.strStageId == pStage->strStageId;
			});
	}
	if (bFoundCamera && ImGui::CollapsingHeader("Camera / Light References"))
	{
		if ((!pStage->CameraInvocations.empty() || m_bPatternShakesReady) &&
			BeginLinkedTable("##CompositionSemanticCameraRows"))
		{
			for (const VALTAN_CAMERA_INVOCATION_VIEW& Camera :
				pStage->CameraInvocations)
			{
				bFoundCamera = true;
				ImGui::PushID(Camera.strCameraInvocationId.c_str());
				LinkedRow(
					DETAIL_OWNER::CAMERA, Camera.strCameraInvocationId,
					Camera.strCameraInvocationId + " | " +
					Camera.strCameraCueId + " | start " +
					std::to_string(Camera.iStartOffsetMs) + " ms",
					"Camera Detail");
				ImGui::PopID();
			}
			if (m_bPatternShakesReady)
			{
				for (const VALTAN_PATTERN_SHAKE_CUE& Cue : m_PatternShakes.Cues)
				{
					if (Cue.strPatternId != pPattern->strPatternId ||
						Cue.strStageId != pStage->strStageId)
					{
						continue;
					}
					bFoundCamera = true;
					ImGui::PushID(Cue.strOccurrenceId.c_str());
					LinkedRow(
						DETAIL_OWNER::CAMERA, Cue.strOccurrenceId,
						Cue.strOccurrenceId + " | camera shake | " +
						Cue.strClipOccurrenceId,
						"Camera Detail");
					ImGui::PopID();
				}
			}
			ImGui::EndTable();
		}
	}

	if (ImGui::CollapsingHeader("World / Light Event References"))
	{
		bool_t bFoundWorld = false;
		if (!pPattern->WorldEventTriggerRefs.empty() &&
			BeginLinkedTable("##CompositionSemanticWorldRows"))
		{
			for (const VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW& Event :
				pPattern->WorldEventTriggerRefs)
			{
				if (Event.strStageId != pStage->strStageId)
					continue;
				bFoundWorld = true;
				ImGui::PushID(Event.strTriggerKind.c_str());
				LinkedRow(
					DETAIL_OWNER::WORLD, Event.strTriggerKind,
					Event.strTriggerKind + " | " + Event.strStageId,
					"Server / Map Detail");
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
		if (!bFoundWorld)
			ImGui::TextDisabled("No World Event reference is linked to this Stage.");
	}

	if (ImGui::CollapsingHeader(
			"Server Collider Owner", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const std::string ColliderLabel = pStage->Has_HitShape() ?
			pStage->strStageId + " | " + pStage->strHitShape +
			" | damage " + pStage->strServerDamageProfileId + " | hits " +
			std::to_string(pStage->iHitCount) :
			pStage->strStageId + " | NONE | no Server hit shape";
		if (BeginLinkedTable("##CompositionSemanticColliderRows"))
		{
			ImGui::PushID(pStage->strStageId.c_str());
			LinkedRow(
				DETAIL_OWNER::GAMEPLAY_STAGE, pStage->strStageId,
				ColliderLabel, "Gameplay / Collider");
			ImGui::PopID();
			ImGui::EndTable();
		}
		ImGui::TextDisabled(
			"Server gameplay owns collider geometry, hit clocks and player response; Effect rows remain visual invocations.");
	}
}

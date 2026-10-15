// 저작 Pattern의 preview 준비·seek·transport와 Server 검증 요청을 기존 소유자에 연결한다.
// preview는 Animation Tool의 local 재생과 MainApp의 Server 검증 명령을 연결한다.
// 같은 재생 버튼처럼 보여도 준비할 snapshot·허용 revision·실행 권위가 다르므로 두 경로의 조건을 각각 검사한다.
#include "imgui.h"
#include "ValtanActionWorkbench.h"
#include "BalanceTool.h"
#include "Animation_Tool.h"
#include "Effect_Catalog.h"
#include "Effect_PresentationService.h"
#include "EffectV2_Catalog.h"
#include "MainApp.h"
#include "Level_ValtanArena.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "ValtanActionWorkbench_EditSupport.h"

// 재생 중 여부와 경로 이름은 본체·Details도 쓰는 구현 보조 이름을 참조한다. 이 선언으로 세션 상태가 새로 생성되지는 않는다.
using namespace Client::ValtanWorkbenchDetail;



// ruler와 Detail 입력이 요청한 ms 위치로 preview를 맞춘다. draft 세대나 경로가 달라지면 먼저 재생 snapshot을 준비하며, 준비 대기 중에는 위치·pause를 값으로 보관한다.
bool_t Client::CValtanActionWorkbench::Seek_EffectivePreview(
	const VALTAN_PATTERN_VIEW& Pattern,
	const uint32_t iPositionMs,
	const bool_t bPause,
	std::string& status)
{
	if (IsValtanServerPlaybackBusy(m_pValtanBossTool))
	{
		status = "Preview is unavailable while a Server Pattern owns playback.";
		return false;
	}
	if (nullptr == m_pAnimationTool)
	{
		status = "Animation preview owner is unavailable.";
		return false;
	}
	const CAnimation_Tool::COMPOSITION_PREVIEW_STATE Preview =
		m_pAnimationTool->Get_ValtanCompositionPreviewState();
	const std::uint64_t iDraftGeneration = nullptr == m_pBalanceTool ? 0u :
		m_pBalanceTool->Get_ValtanDraftGeneration();
	if (!Preview.bPlaying || Preview.strPatternId != Pattern.strPatternId ||
		m_iPreviewDraftGeneration != iDraftGeneration ||
		m_eStagedPreviewPath != m_ePreviewPath)
	{
		if (!Play_EffectivePreview(Pattern, status))
		{
			if (m_PendingLocalPreview)
			{
				m_PendingLocalPreview->positionMs = iPositionMs;
				m_PendingLocalPreview->pause = bPause;
			}
			return false;
		}
	}
	const bool_t bSeeked =
		m_pAnimationTool->Seek_ValtanCompositionPattern(
			Pattern.strPatternId, iPositionMs, bPause, status);
	if (bSeeked)
		m_bPreviewOwnerClaimRequested = true;
	return bSeeked;
}

// 현재 Pattern을 기존 Animation Tool preview 경로에 stage한다. Effect 준비가 끝나야 시작하고 대기 상태는 ID·세대·revision을 값으로 소유하여 프레임 밖에 Pattern 포인터를 남기지 않는다.
bool_t Client::CValtanActionWorkbench::Play_EffectivePreview(
	const VALTAN_PATTERN_VIEW& Pattern,
	std::string& status)
{
	if (IsValtanServerPlaybackBusy(m_pValtanBossTool))
	{
		status = "Preview is unavailable while a Server Pattern is preparing or running.";
		return false;
	}
    m_BoneEditor.Stop();
	if (nullptr == m_pAnimationTool)
	{
		status = "Animation preview owner is unavailable.";
		return false;
	}
	std::string EffectV2Status;
	if (!CEffectV2Catalog::Get().Has_BossValtanBindingDraft() &&
		!CEffectV2Catalog::Get().Reload_BossValtan(EffectV2Status))
	{
		status =
			"Arena Clone play rejected because the current Effect V2 authoring set did not validate. The previous snapshot was preserved: " +
			EffectV2Status;
		return false;
	}
	// catalog가 제공한 불변 snapshot을 shared_ptr로 보유해 이 호출의 확인 동안 저장소 수명을 유지한다.
	// 반면 previewStages의 원소는 입력 Pattern 내부 주소이므로 프레임을 넘기는 대기 상태에는 복사하지 않는다.
	const std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pEffectV2Snapshot =
		CEffectV2Catalog::Get().Get_Snapshot();
	if (nullptr == pEffectV2Snapshot || !pEffectV2Snapshot->Is_Ready())
	{
		status =
			"Arena Clone play rejected because no ready Effect V2 authoring snapshot was staged. The previous preview was preserved.";
		return false;
	}
	std::vector<const VALTAN_STAGE_VIEW*> previewStages;
	if (!CValtanPatternTree::Build_PreviewStagePath(Pattern, m_ePreviewPath, previewStages, status))
		return false;
	// 같은 Effect가 여러 Stage/cue에 등장해도 준비 대상 ID는 한 번만 넣는다.
	// set의 정렬된 ID를 vector로 바꿔 기존 준비 API에 전달하며 이 집합은 한 번의 재생 준비 호출에만 존재한다.
	std::set<std::string> targets;
	for (const auto* stage : previewStages)
		for (const auto& cue : stage->ProductCues)
			if (!cue.strEffectAssetId.empty()) targets.insert(cue.strEffectAssetId);
	std::vector<std::string> preparedTargets;
	if (!targets.empty() && !CEffectPresentationService::Queue_ProductTargets_Priority(
		std::vector<std::string>(targets.begin(), targets.end()), preparedTargets, status))
	{
		m_PendingLocalPreview.reset();
		status = "Preview Effect preparation rejected: " + status;
		return false;
	}
	const auto preparation = CEffectPresentationService::Get_ProductCuePreparationProbe(preparedTargets);
	if (!preparedTargets.empty() && (preparation.iFailedCount || preparation.iUnavailableCount ||
		!preparation.strBlockingFailure.empty()))
	{
		m_PendingLocalPreview.reset();
		status = "Preview Effect preparation failed: " + preparation.strBlockingFailure;
		for (const auto& id : preparedTargets)
		{
			const auto failure = CEffectPresentationService::Get_ProductCuePreparationFailure(id);
			if (!failure.empty()) status += " " + id + ": " + failure;
		}
		return false;
	}
	if (!preparedTargets.empty() && (!preparation.bCatalogRevisionCurrent || !preparation.bSettled ||
		preparation.iPreparedCount != preparedTargets.size()))
	{
		PENDING_LOCAL_PREVIEW pending;
		pending.patternId = Pattern.strPatternId; pending.effectIds = std::move(preparedTargets);
		// 보존한 저장 view의 generation은 0이므로 더 새로운 live draft generation과 비교하지 않는다.
		pending.draftGeneration = &Pattern == Find_SelectedPattern() ? 0u :
			(m_pBalanceTool ? m_pBalanceTool->Get_ValtanDraftGeneration() : 0u);
		pending.canonicalGeneration = m_iCanonicalDisplayGeneration;
		pending.v1Revision = CEffectCatalog::Get_RuntimeRevision();
		pending.v2Revision = CEffectV2Catalog::Get().Get_Revision(); pending.path = m_ePreviewPath;
		m_PendingLocalPreview = std::move(pending);
		status = "Preparing Preview Effects " + std::to_string(preparation.iPreparedCount) + "/" +
			std::to_string(m_PendingLocalPreview->effectIds.size()) + "; playback starts when ready.";
		return false;
	}
	m_PendingLocalPreview.reset();
	Ensure_TimelineCache(&Pattern);
	if (!m_pAnimationTool->Play_ValtanCompositionDraftPattern(
			Pattern, m_ePreviewPath, status, m_iTimelineDurationMs,
			m_bCombatObjectSoundsReady ? &m_CombatObjectSounds : nullptr))
	{
		return false;
	}
#ifdef _DEBUG
	if (auto* arena = CLevel_ValtanArena::Get_Active(); arena &&
		!arena->Debug_PrepareActionWorkbenchDestruction(Pattern, status))
	{
		std::string stopped;
		m_pAnimationTool->Stop_ValtanCompositionPattern(stopped);
		return false;
	}
#endif
	m_iPreviewDraftGeneration = nullptr == m_pBalanceTool ? 0u :
		m_pBalanceTool->Get_ValtanDraftGeneration();
	m_eStagedPreviewPath = m_ePreviewPath;
	m_bPreviewOwnerClaimRequested = true;
	return true;
}

// 저장·게시 준비 상태를 확인한 뒤 기존 MainApp의 Server 검증 명령을 호출한다. local preview를 Server 권위 실행으로 대체하거나 독립 네트워크 경로를 만들지 않는다.
bool_t Client::CValtanActionWorkbench::Play_ServerVerification(
	const VALTAN_PATTERN_VIEW& Pattern,
	std::string& status)
{
#ifdef _DEBUG
	if (m_bAuthoringDraftDirty || m_bPatternSoundDependencyDirty || m_CombatObjectSounds.bDraftDirty || CEffectV2Catalog::Get().Has_BossValtanBindingDraft())
	{
		status = "Save and publish the Pattern before Play Pattern. Play Preview uses the current draft.";
		return false;
	}
	if (!m_bProductSourceReady)
	{
		status = "Play Pattern requires the saved source and published Product to agree. Publish, then reload the Pattern. " + m_strProductReadiness;
		return false;
	}
	if (IsValtanServerPlaybackBusy(m_pValtanBossTool))
	{
		status = "Another Server Pattern or preparation owns playback.";
		return false;
	}
	CMainApp* const app = CMainApp::Get_Active();
	if (nullptr == app)
	{
		status = "Server-exact Pattern verification owner is unavailable.";
		return false;
	}
	if (!app->Debug_SelectCompletePlayPattern(Pattern.strPatternId))
	{
		status =
			"The selected Pattern is absent from the Server-admitted verification inventory.";
		return false;
	}
	return app->Debug_CompletePlaySelected(status);
#else
	(void)Pattern;
	status =
		"Server-exact Pattern verification is unavailable outside a Debug Client.";
	return false;
#endif
}

// V2 초안 변경 뒤 현재 재생 위치·pause를 값으로 보관하고 기존 preview 경로를 다시 요청한다. 실패 진단은 초안 보존과 재생 재시작 실패를 구분한다.
void Client::CValtanActionWorkbench::
Refresh_EffectV2LocalPreviewAfterMutation(
	const VALTAN_PATTERN_VIEW* const pPattern,
	std::string& strInOutStatus)
{
	const CAnimation_Tool::COMPOSITION_PREVIEW_STATE Preview =
		nullptr == m_pAnimationTool ?
			CAnimation_Tool::COMPOSITION_PREVIEW_STATE{} :
			m_pAnimationTool->Get_ValtanCompositionPreviewState();
	const bool_t bRestartLocalClone = nullptr != pPattern &&
		Preview.bPlaying && Preview.strPatternId == pPattern->strPatternId;
	if (bRestartLocalClone)
	{
		std::string StopStatus;
		m_pAnimationTool->Stop_ValtanCompositionPattern(StopStatus);
	}
	Invalidate_TimelineCache();

	const std::string ResourceStatus = strInOutStatus;
	if (!bRestartLocalClone)
	{
		strInOutStatus = ResourceStatus +
			" Local V2 draft restaged; the next Arena Clone Play reads it. Use Save to commit it with Pattern and Sound.";
		return;
	}

	std::string PreviewStatus;
	const uint32_t iRestorePositionMs = (std::min)(
		Preview.iPositionMs, m_iTimelineDurationMs);
	if (!Seek_EffectivePreview(
			*pPattern, iRestorePositionMs, Preview.bPaused, PreviewStatus))
	{
		if (m_PendingLocalPreview)
		{
			strInOutStatus = ResourceStatus + " " + PreviewStatus;
			return;
		}
		strInOutStatus = ResourceStatus +
			" V2 draft was preserved, but Arena Clone restart failed: " +
			PreviewStatus +
			" Saved data is unchanged until Save succeeds.";
		return;
	}
	m_iPlayheadMs = iRestorePositionMs;
	strInOutStatus = ResourceStatus +
		" Local V2 draft was restaged and the Arena Clone was restarted at the same timeline position. Use Save to commit every owner together.";
}

// Pattern 변경 뒤 BalanceTool의 유효 초안을 다시 조립해 현재 preview 위치를 복원한다. 저장 파일 반영은 이 함수의 책임이 아니다.
void Client::CValtanActionWorkbench::
Refresh_PatternLocalPreviewAfterMutation(
	const VALTAN_PATTERN_VIEW* const pPattern,
	std::string& strInOutStatus)
{
	const CAnimation_Tool::COMPOSITION_PREVIEW_STATE Preview =
		nullptr == m_pAnimationTool ?
			CAnimation_Tool::COMPOSITION_PREVIEW_STATE{} :
			m_pAnimationTool->Get_ValtanCompositionPreviewState();
	const bool_t bRestartLocalClone = nullptr != pPattern &&
		Preview.bPlaying && Preview.strPatternId == pPattern->strPatternId;
	Invalidate_TimelineCache();
	const std::string MutationStatus = strInOutStatus;
	if (!bRestartLocalClone)
	{
		strInOutStatus = MutationStatus +
			" The next Arena Clone Play Preview reads this Pattern draft; Save commits it to the physical Pattern JSON.";
		return;
	}

	VALTAN_PATTERN_VIEW EffectivePattern;
	std::string EffectiveStatus;
	if (nullptr == m_pBalanceTool ||
		!m_pBalanceTool->Get_ValtanPatternDraft(
			pPattern->strPatternId, EffectivePattern, EffectiveStatus))
	{
		strInOutStatus = MutationStatus +
			" Pattern draft was preserved, but the effective Pattern could not be assembled for Arena Clone restage: " +
			EffectiveStatus + " Use Play Preview to retry.";
		return;
	}
	std::string StopStatus;
	m_pAnimationTool->Stop_ValtanCompositionPattern(StopStatus);
	std::string PreviewStatus;
	const uint32_t iRestorePositionMs = (std::min)(
		Preview.iPositionMs, m_iTimelineDurationMs);
	if (!Seek_EffectivePreview(
			EffectivePattern, iRestorePositionMs, Preview.bPaused, PreviewStatus))
	{
		if (m_PendingLocalPreview)
		{
			strInOutStatus = MutationStatus + " " + PreviewStatus;
			return;
		}
		strInOutStatus = MutationStatus +
			" Pattern draft was preserved, but Arena Clone restart failed: " +
			PreviewStatus +
			" Use Play Preview to retry; saved data is unchanged until Save succeeds.";
		return;
	}
	m_iPlayheadMs = iRestorePositionMs;
	strInOutStatus = MutationStatus +
		" The Pattern draft was restaged and Arena Clone resumed at the same timeline position. Use Save to commit the physical Pattern JSON.";
}

// 호출자가 준 ms를 현재 타임라인 길이 안으로 제한하고 선택 Pattern의 정지 preview를 요청한다. cursor는 멤버에 저장하고 seek 성공 여부를 반환한다.
bool_t Client::CValtanActionWorkbench::Set_PlayheadMs(
	const uint32_t iPlayheadMs)
{
	m_iPlayheadMs = (std::min)(iPlayheadMs, m_iTimelineDurationMs);
	const VALTAN_PATTERN_VIEW* const pPattern = Find_SelectedPattern();
	if (nullptr == pPattern || nullptr == m_pAnimationTool ||
		!Can_MutateValtanView(m_eAdmission))
	{
		return false;
	}
	std::string Status;
	const bool_t bSought = Seek_EffectivePreview(
		*pPattern, m_iPlayheadMs, true, Status);
	m_strStatus = std::move(Status);
	return bSought;
}

// preview의 재생 상태와 transport 입력을 표시한다. ImGui 범위와 명령 순서는 유지하며 pose·Effect 실행은 기존 Animation Tool 소유자에 요청한다.
void Client::CValtanActionWorkbench::Render_Preview(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const bool_t bLocalPreviewAdmitted,
	const bool_t bServerVerificationAdmitted)
{
	ImGui::SeparatorText("Server-Exact Pattern Verification");
	ImGui::TextWrapped(
		"Use this Play for Pattern order verification. It resets the authoritative Valtan from boss.valtan.center and runs the saved Server stages, movement, combat objects, animation, Effect, Sound, camera and world events.");
	ImGui::BeginDisabled(nullptr == pPattern || !bServerVerificationAdmitted || !m_bProductSourceReady ||
		m_bAuthoringDraftDirty || m_bPatternSoundDependencyDirty || m_CombatObjectSounds.bDraftDirty || CEffectV2Catalog::Get().Has_BossValtanBindingDraft() ||
		IsValtanServerPlaybackBusy(m_pValtanBossTool));
	if (ImGui::Button("Play Pattern##PreviewPanel"))
	{
		std::string Status;
		(void)Play_ServerVerification(*pPattern, Status);
		m_strStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine(); Render_EditHistoryButtons("PreviewHistory");
	if (!bServerVerificationAdmitted || !m_bProductSourceReady || m_bAuthoringDraftDirty ||
		m_bPatternSoundDependencyDirty || m_CombatObjectSounds.bDraftDirty || CEffectV2Catalog::Get().Has_BossValtanBindingDraft())
	{
		ImGui::TextDisabled(
			"Save and publish the selected Pattern, start the Debug Server, enter Valtan, and keep a living combat-ready player in the arena.");
	}

	ImGui::SeparatorText("Local Timeline Editing Preview");
	if (!m_strCinematicPreviewStatus.empty())
		ImGui::TextWrapped("%s", m_strCinematicPreviewStatus.c_str());
	ImGui::TextDisabled(
		"This transport supports pause, seek and unsaved draft inspection. It is not Server verification; its clone uses the canonical arena boss transform instead of the player position.");
	CAnimation_Tool::COMPOSITION_PREVIEW_STATE Preview;
	if (nullptr != m_pAnimationTool)
	{
		if (bLocalPreviewAdmitted)
			m_pAnimationTool->Set_ValtanCompositionLoop(m_bLoopPreview);
		Preview = m_pAnimationTool->Get_ValtanCompositionPreviewState();
	}
	const bool_t bSourcePreview = Preview.bSourceSequencePlaying;
	const bool_t bTransportReady = nullptr != m_pAnimationTool &&
		!IsValtanServerPlaybackBusy(m_pValtanBossTool) && bLocalPreviewAdmitted &&
		(bSourcePreview || nullptr != pPattern);
	ImGui::TextColored(
		Preview.bModelReady ? ImVec4(0.35f, 0.86f, 0.45f, 1.f) :
			ImVec4(1.f, 0.72f, 0.24f, 1.f),
		"Dedicated Valtan Model View: %s",
		Preview.bModelReady ? "READY" : "DATA-ONLY");
	ImGui::TextDisabled(
		"The game viewport remains the real preview surface; this panel owns only typed transport and selection.");
	ImGui::TextColored(
		m_bPreviewOwnerActive ? ImVec4(0.35f, 0.86f, 0.45f, 1.f) :
			ImVec4(1.f, 0.72f, 0.24f, 1.f),
		"Viewport preview owner: %s",
		m_bPreviewOwnerActive ? "ACTIVE" :
			(m_bPreviewOwnerClaimRequested ? "CLAIM PENDING" : "INACTIVE"));
	ImGui::SameLine();
	ImGui::BeginDisabled(
		m_bPreviewOwnerActive || m_bPreviewOwnerClaimRequested);
	if (ImGui::Button("Claim Preview Owner"))
		m_bPreviewOwnerClaimRequested = true;
	ImGui::EndDisabled();
	if (!m_bPreviewOwnerActive)
	{
		ImGui::TextDisabled(
			"Effect/Camera/Animation deep-links transfer viewport input. Claim it here before using Workbench transport again.");
	}
	ImGui::TextColored(ImVec4(0.35f, 0.86f, 0.45f, 1.f),
		"LIVE transport: effective Animation slots + collider mirror + V1 Effect cue timing/yaw + Product V2 stage bindings.");
	ImGui::TextDisabled(
		"Preview samples Animation, Effects, Sound, Camera and object lifetimes on the selected Stage path. Preview Outcome selects conditional Logic results; live collisions, player decisions and damage run in Play Pattern.");
	int32_t iPreviewPath = static_cast<int32_t>(m_ePreviewPath);
	ImGui::SetNextItemWidth(260.f);
	if (ImGui::Combo(
			"Preview Outcome",
			&iPreviewPath,
			"Normal / Timeout\0Counter Hit -> Groggy\0Wall Contact -> Groggy\0Wall Contact -> Part Break\0Capture Success -> Throw\0"))
	{
		if (Preview.bPlaying && nullptr != m_pAnimationTool)
		{
			std::string StopStatus;
			m_pAnimationTool->Stop_ValtanCompositionPattern(StopStatus);
		}
		m_ePreviewPath = static_cast<VALTAN_PATTERN_PREVIEW_PATH>(iPreviewPath);
		m_iPreviewDraftGeneration = 0u;
		m_iPlayheadMs = 0u;
		Invalidate_TimelineCache();
	}
	ImGui::SameLine();
	ImGui::TextDisabled("%s", PreviewPathLabel(m_ePreviewPath));

	ImGui::BeginDisabled(nullptr == m_pAnimationTool || !bLocalPreviewAdmitted);
	if (ImGui::Button("Stage Valtan Model View"))
	{
		std::string Status;
		(void)m_pAnimationTool->Stage_ValtanCompositionPreview(Status);
		m_strStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!bTransportReady);
	if (ImGui::Button(Preview.bPlaying && !Preview.bPaused ?
			"Pause Preview" : "Play Preview"))
	{
		std::string Status;
		if (bSourcePreview)
		{
			const uint32_t position = Preview.bPaused && Preview.iPositionMs >= Preview.iDurationMs ?
				0u : Preview.iPositionMs;
			(void)m_pAnimationTool->Seek_ValtanCompositionSourceSequence(position, !Preview.bPaused, Status);
		}
		else if (Preview.bPlaying)
		{
			(void)Seek_EffectivePreview(
				*pPattern,
				Preview.iPositionMs,
				!Preview.bPaused,
				Status);
		}
		else
		{
			(void)Seek_EffectivePreview(*pPattern, m_iPlayheadMs >= m_iTimelineDurationMs ? 0u : m_iPlayheadMs, false, Status);
		}
		m_strStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	/* Stop은 이미 실행 중인 local preview를 종료하는 조작이다.
	   승인 상태가 STALE_PRESERVED로 바뀌어도 이 종료 조작은 계속 허용한다. */
	ImGui::BeginDisabled(nullptr == m_pAnimationTool || (!Preview.bPlaying && !m_PendingLocalPreview));
	if (ImGui::Button("Reset##PreviewPanel"))
	{
		m_PendingLocalPreview.reset();
		std::string Status;
		m_pAnimationTool->Stop_ValtanCompositionPattern(Status);
		m_iPlayheadMs = 0u;
		m_iPreviewDraftGeneration = 0u;
		m_strStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!bTransportReady);
	if (ImGui::Button("Restart Local Timeline"))
	{
		std::string Status;
		if (bSourcePreview)
			(void)m_pAnimationTool->Seek_ValtanCompositionSourceSequence(0u, false, Status);
		else
			(void)Play_EffectivePreview(*pPattern, Status);
		m_strStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	if (!m_strStatus.empty())
	{
		ImGui::TextWrapped("Preview status: %s", m_strStatus.c_str());
	}

	if (m_pAnimationTool)
		Preview = m_pAnimationTool->Get_ValtanCompositionPreviewState();
	if (Preview.bPlaying && (Preview.bSourceSequencePlaying ||
		(Preview.strPatternId == m_strSelectedPatternId && m_eStagedPreviewPath == m_ePreviewPath)))
		m_iPlayheadMs = Preview.iPositionMs;
	const uint32_t iDurationMs = (std::max)(bSourcePreview ? Preview.iDurationMs : m_iTimelineDurationMs, 1u);
	int32_t iPlayhead = static_cast<int32_t>((std::min)(
		m_iPlayheadMs, static_cast<uint32_t>(INT32_MAX)));
	const int32_t iMaximum = static_cast<int32_t>((std::min)(
		iDurationMs, static_cast<uint32_t>(INT32_MAX)));
	ImGui::SetNextItemWidth(-1.f);
	if (ImGui::SliderInt(
			"##CompositionPlayhead", &iPlayhead, 0, iMaximum,
			"%d ms", ImGuiSliderFlags_AlwaysClamp))
	{
		m_iPlayheadMs = static_cast<uint32_t>((std::max)(iPlayhead, 0));
		if (bTransportReady && Preview.bModelReady)
		{
			std::string Status;
			if (bSourcePreview)
				(void)m_pAnimationTool->Seek_ValtanCompositionSourceSequence(m_iPlayheadMs, true, Status);
			else
				(void)Seek_EffectivePreview(*pPattern, m_iPlayheadMs, true, Status);
			m_strStatus = std::move(Status);
		}
	}
	ImGui::Text(
		"%u / %u ms", m_iPlayheadMs, iDurationMs);
	ImGui::SameLine();
	ImGui::BeginDisabled(nullptr == m_pAnimationTool || !bLocalPreviewAdmitted);
	if (ImGui::Checkbox("Loop Preview", &m_bLoopPreview) &&
		nullptr != m_pAnimationTool)
	{
		m_pAnimationTool->Set_ValtanCompositionLoop(m_bLoopPreview);
	}
	ImGui::EndDisabled();
	if (!Preview.strStatus.empty())
		ImGui::TextWrapped("Preview: %s", Preview.strStatus.c_str());
}

// 상위 화면이 preview 소유권 요청을 한 번 소비한다. 요청 flag만 비우고 preview 객체의 수명을 이 함수에서 변경하지 않는다.
bool_t Client::CValtanActionWorkbench::Consume_PreviewOwnerClaimRequest()
{
	const bool_t bRequested = m_bPreviewOwnerClaimRequested;
	m_bPreviewOwnerClaimRequested = false;
	return bRequested;
}

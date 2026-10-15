#pragma once

// Valtan Workbench 구현 파일 사이에서만 쓰는 저작 보조 함수 선언이다.
// 문서·preview·저장 상태를 별도로 소유하지 않으며 반환된 행 포인터는 호출 중 차용한다.
// CBalanceTool의 nested draft 타입을 선언에 사용하므로 완전한 클래스 선언이 필요하다.
#include "BalanceTool.h"
#include "ValtanPatternTree.h"
#include "EffectV2_Catalog.h"
#include "ValtanPatternShakeCueDocument.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// Boss Tool은 차용 포인터만 공개하므로 전방 선언한다. 중첩 draft 타입을 쓰는 BalanceTool과 필요한 정의 범위가 다르다.
namespace Client { class CValtanBossTool; }

// 같은 Workbench의 여러 CPP가 공유하는 구현 전용 이름 공간이다.
// 클래스의 private 상태를 옮기는 장소가 아니며 helper는 필요한 owner·snapshot·draft를 인자로 받는다.
// 선언을 헤더에 공유하고 정의는 본체 CPP 한 곳에 둬 링크 시 같은 구현을 사용한다.
namespace Client::ValtanWorkbenchDetail
{
bool IsValtanServerPlaybackBusy(const Client::CValtanBossTool* boss);

bool ContainsInsensitive(
		const std::string_view text,
		const std::string_view query);

std::string BuildBranchTargetLabel(
		const Client::VALTAN_STAGE_BRANCH_VIEW& Branch);

std::string BuildBranchStableId(
		const std::string& stageId, const std::string& outcome);

std::string BuildNextCompositionSlotId(
		const std::string& patternId,
		const std::string& stageId,
		const std::vector<Client::CBalanceTool::ANIMATION_SLOT_EDIT>& Slots);

std::string BuildEffectV2BindingStableId(
		const Client::EFFECT_V2_BINDING& Binding,
		const std::string_view strClipOccurrenceId = {});

// 반환 포인터는 Snapshot 내부 binding을 차용한다. Snapshot 소유권을 넘기거나 새 binding을 할당하지 않는다.
// 호출자가 snapshot을 보유한 동안만 사용하고 catalog 교체 뒤에는 stable ID로 다시 조회한다. 빈 clip ID는 기본 조회 조건이다.
const Client::EFFECT_V2_BINDING* ResolveEffectV2Binding(
		const Client::EFFECT_V2_CATALOG_SNAPSHOT& Snapshot,
		const std::string_view strStableId,
		const std::string_view strClipOccurrenceId = {});

std::string StageSequenceRoleLabel(
		const Client::VALTAN_STAGE_VIEW& Stage);

std::string BuildNextCompositionEffectCueId(
		const Client::VALTAN_PATTERN_VIEW& pattern,
		const Client::VALTAN_STAGE_VIEW& stage);

bool_t ComputeExactAnimationWallMs(
		const Client::CBalanceTool::PATTERN_STAGE_EDIT& Draft,
		uint32_t& iOutWallMs);

bool_t ApplyStageClockPolicy(
		Client::CBalanceTool::PATTERN_STAGE_EDIT& Draft,
		const uint32_t iRequestedDurationMs,
		std::string& Status);

bool_t InitializeDefaultValtanColliderBoxDraft(
		Client::CBalanceTool::PATTERN_STAGE_EDIT& Draft,
		const std::vector<std::string>& DamageProfiles,
		const int32_t iRequestedDamageProfile,
		std::string& Status);

bool_t SameValtanColliderDraft(
		const Client::CBalanceTool::PATTERN_STAGE_EDIT& Left,
		const Client::CBalanceTool::PATTERN_STAGE_EDIT& Right);

bool_t ValidateValtanEffectInvocationClock(
		Client::CAnimation_Tool* animation, const Client::VALTAN_STAGE_VIEW& stage,
		const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& cue, std::string& status);

// Details·timeline 등 여러 입력 경로가 같은 Stage 변경 검증을 사용하도록 모은 함수다.
// Pattern/Stage/Draft는 읽기 입력이고 포인터 owner의 API에 변경을 요청한다. bValidateOnly이면 검증만 한다.
// Sound·Effect·Shake의 연결 시각을 먼저 대조해 한 pane만 종속 검증을 생략하지 않게 한다.
bool_t SetValtanStageDraftWithSoundDependencyAdmission(
		Client::CAnimation_Tool* const pAnimationTool,
		Client::CBalanceTool* const pBalanceTool,
		const Client::VALTAN_PATTERN_SHAKE_CUE_DOCUMENT* const pPatternShakes,
		const Client::VALTAN_PATTERN_VIEW& BaselinePattern,
		const Client::VALTAN_STAGE_VIEW& BaselineStage,
		const Client::CBalanceTool::PATTERN_STAGE_EDIT& Draft,
		std::string& Status,
		const uint32_t iSourceActionId = 0u,
		const uint32_t iSourceSequenceIndex = 0u,
		const bool_t bValidateOnly = false);

const char* PreviewPathLabel(
		const Client::VALTAN_PATTERN_PREVIEW_PATH ePath);

}

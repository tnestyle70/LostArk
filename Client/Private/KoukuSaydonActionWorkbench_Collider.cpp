// Collider 그룹의 기하 preview와 Logic/Result 연결 편집을 담당한다. 드래그 overlay와 Apply할 입력을 분리해 무관한 편집을 확정하지 않는다.
#include "imgui.h"
#include "KoukuSaydonActionWorkbench.h"
#include "KoukuSaydonActionWorkbench_Internal.h"

// 후보 값 복사, stable ID 조회, 시간/좌표 계산에 필요한 표준 타입이다.
#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <unordered_set>
#include <utility>
#include <string>
#include <string_view>
#include <vector>

using namespace Client::KoukuWorkbenchDetail;

// 그룹 UI와 Transform_SelectedColliders가 선택 ID를 실제 편집 배치로 해석할 때 사용한다.
// draft 행에 미저장 기하 overlay를 합친 사본을 모으고 공통 Boss anchor/회전 기준을 검증한다.
// 실패 때 outBoxes에는 앞서 모은 일부 사본이 남을 수 있으므로 호출자는 성공한 목록만 사용한다.
bool_t Client::CKoukuSaydonActionWorkbench::Collect_ColliderPlacements(const std::string_view patternId,
	const std::vector<std::string> &occurrenceIds,
	std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE> &outBoxes, std::string &outStatus) const
{
	outBoxes.clear();
	const auto reject = [&](const char *reason) {
		outStatus = reason;
		return false;
	};
	const auto *pattern = Find_Pattern(m_Draft, patternId);
	if (!pattern || !pattern->strLoadError.empty())
		return reject("Select a valid Pattern before grouping Colliders.");
	// set은 중복 ID를 한 번만 처리하기 위한 보조 자료다. outBoxes에는 입력 vector 순서대로 값 사본을 쌓아 기하 검증 후 일괄 사용한다.
	std::unordered_set<std::string> selected;
	for (const auto &id : occurrenceIds)
	{
		if (!selected.insert(id).second)
			continue;
		const auto *source = Find_PresentationBox(*pattern, id);
		const auto *resource = source ? Find_PresentationResource(m_Draft, source->strResourceId) : nullptr;
		if (!source || !resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER)
			return reject("Select only Collider boxes from this Pattern.");
		auto box = *source;
		for (const auto &staged : m_StagedPresentationGeometry)
			if (staged.strPatternId == patternId && staged.Occurrence.strOccurrenceId == id)
			{
				Copy_PresentationPlacement(staged.Occurrence, box, false);
				break;
			}
		if (!Valid_PresentationGeometry(box))
			return reject("Correct or revert invalid Collider geometry before grouping or moving the selection.");
		if (box.strAnchorKind != "BOSS")
			return reject(
				"Collider groups currently require a Boss anchor. World Object scale can change the rotation basis.");
		if (!outBoxes.empty() && !Same_ColliderGroupFrame(outBoxes.front(), box))
			return reject(
				"Collider groups require the same anchor, bone and Follow setting. Frozen anchors also require the same start time.");
		outBoxes.push_back(std::move(box));
	}
	if (outBoxes.size() < 2u)
		return reject("Select at least two Collider boxes.");
	return true;
}

// Collider 그룹 버튼의 요청을 공통 그룹 편집으로 전달한다. Collider 기하/anchor 검증은
// Set_PresentationSelectionGroup이 이 종류를 보고 수행하며 이 래퍼는 별도 상태를 만들지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Set_ColliderSelectionGroup(const std::string_view patternId,
	const std::vector<std::string> &occurrenceIds, const bool_t grouped, std::string &outStatus)
{
	return Set_PresentationSelectionGroup(
		patternId, occurrenceIds, KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER, grouped, outStatus);
}

// Collider 그룹 Detail의 중심 이동/Y 회전 입력을 선택된 모든 박스에 적용한다.
// 계산과 범위 검증을 사본에서 끝낸 뒤 history와 기하 overlay를 갱신하며 m_Draft 교체는 하지 않는다.
// preview만 불가능한 경우에도 staged 값은 Save 대상으로 남기고 true와 상태 메시지를 돌려준다.
bool_t Client::CKoukuSaydonActionWorkbench::Transform_SelectedColliders(
	const std::array<double, 3u> &translation, const double yawDeltaDegrees, std::string &outStatus)
{
	const auto reject = [&](const char *reason) {
		outStatus = m_strStatus = reason;
		return false;
	};
	if (m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN ||
		m_strTimelineSelectionPatternId != m_strSelectedPatternId || !m_TimelineSelectedStageIds.empty())
		return reject("Select only Collider boxes from the current Pattern.");
	if (!std::isfinite(yawDeltaDegrees) || std::abs(yawDeltaDegrees) > 36000.0 ||
		!std::all_of(translation.begin(), translation.end(), [](double value) {
			return std::isfinite(value);
		}))
		return reject("Group translation and Y rotation must be finite and within the authoring range.");
	std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE> boxes;
	if (!Collect_ColliderPlacements(m_strSelectedPatternId, m_TimelineSelectedOccurrenceIds, boxes, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	std::array<double, 3u> center{};
	for (const auto &box : boxes)
	{
		if (yawDeltaDegrees != 0.0 && (box.RotationDegrees[0] != 0.0 || box.RotationDegrees[2] != 0.0))
			return reject(
				"Group Y rotation requires zero X/Z rotation on every Collider. Ungroup to correct those individual rotations.");
		for (std::size_t axis = 0u; axis < 3u; ++axis)
			center[axis] += box.PositionOffset[axis] / double(boxes.size());
	}
	const double radians = yawDeltaDegrees * 3.14159265358979323846 / 180.0;
	const double cosine = std::cos(radians), sine = std::sin(radians);
	for (auto &box : boxes)
	{
		const double dx = box.PositionOffset[0] - center[0], dz = box.PositionOffset[2] - center[2];
		// DirectX 행 벡터의 Y축 회전이다. +Z 전방이 +X 쪽으로 회전한다.
		box.PositionOffset[0] = center[0] + cosine * dx + sine * dz + translation[0];
		box.PositionOffset[1] += translation[1];
		box.PositionOffset[2] = center[2] - sine * dx + cosine * dz + translation[2];
		if (box.strColliderMotion == "LINEAR")
		{
			const double endX = box.ColliderEndPositionOffset[0] - center[0],
						 endZ = box.ColliderEndPositionOffset[2] - center[2];
			box.ColliderEndPositionOffset[0] = center[0] + cosine * endX + sine * endZ + translation[0];
			box.ColliderEndPositionOffset[1] += translation[1];
			box.ColliderEndPositionOffset[2] = center[2] - sine * endX + cosine * endZ + translation[2];
		}
		box.RotationDegrees[1] += yawDeltaDegrees;
		if (!Valid_PresentationGeometry(box))
			return reject(
				"Group transform exceeds a Collider's position or rotation range. The entire selection is unchanged.");
	}
	const std::string patternId = m_strSelectedPatternId;
	// 묶음 전체를 먼저 검증한다. 드래그 중에는 작은 기하 overlay만 바꾸고,
	// Save에서 문서 사본에 합쳐 기존 Composition 검증/원자적 저장을 수행한다.
	Begin_EditHistory();
	for (const auto &box : boxes)
	{
		const auto staged = std::find_if(
			m_StagedPresentationGeometry.begin(), m_StagedPresentationGeometry.end(), [&](const auto &row) {
				return row.strPatternId == patternId && row.Occurrence.strOccurrenceId == box.strOccurrenceId;
			});
		KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST request{patternId, box};
		if (staged == m_StagedPresentationGeometry.end())
			m_StagedPresentationGeometry.push_back(std::move(request));
		else
			*staged = std::move(request);
		if (m_PresentationBoxEdit.strOccurrenceId == box.strOccurrenceId)
			Copy_PresentationPlacement(box, m_PresentationBoxEdit, false);
	}
	for (const auto &box : boxes)
		if (!Request_PresentationGeometryPreview(patternId, box, outStatus))
		{
			outStatus = m_strStatus = "Group geometry is staged for Save; preview is unavailable: " + outStatus;
			return true;
		}
	outStatus = m_strStatus =
		"Moved the Collider selection around its shared center. Timing and sizes are unchanged; Save keeps the geometry.";
	return true;
}

// Render_Details가 단일 박스 화면보다 먼저 호출한다. 반환 true는 이 선택의 Detail 처리를
// 그룹 화면이 맡았다는 뜻이며 변경 성공 여부가 아니다. commit/선택 변경 뒤에는 즉시 반환하여
// 입력 pattern에서 차용한 행을 다시 쓰지 않고 다음 프레임에 새 선택을 조회한다.
bool_t Client::CKoukuSaydonActionWorkbench::Render_ColliderGroupDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern)
{
	if (m_strTimelineSelectionPatternId != pattern.strPatternId ||
		m_TimelineSelectedStageIds.size() + m_TimelineSelectedOccurrenceIds.size() < 2u)
		return false;
	bool anyCollider = false, anyGroup = false, onlyColliders = m_TimelineSelectedStageIds.empty();
	for (const auto &id : m_TimelineSelectedOccurrenceIds)
	{
		const auto *box = Find_PresentationBox(pattern, id);
		const auto *resource = box ? Find_PresentationResource(m_Draft, box->strResourceId) : nullptr;
		anyCollider |= resource && resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER;
		onlyColliders &= resource && resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER;
		anyGroup |= box && !box->strSelectionGroupId.empty();
	}
	if (!anyCollider)
		return false;
	ImGui::SeparatorText("Collider selection");
	ImGui::Text("%zu boxes selected", m_TimelineSelectedOccurrenceIds.size() + m_TimelineSelectedStageIds.size());
	std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE> boxes;
	std::string reason;
	const bool valid = m_TimelineSelectedStageIds.empty() &&
		Collect_ColliderPlacements(pattern.strPatternId, m_TimelineSelectedOccurrenceIds, boxes, reason);
	if (!m_TimelineSelectedStageIds.empty())
		reason = "Select only Collider boxes to group or transform them.";
	const bool sameGroup = valid && !boxes.front().strSelectionGroupId.empty() &&
		std::all_of(boxes.begin(), boxes.end(), [&](const auto &box) {
			return box.strSelectionGroupId == boxes.front().strSelectionGroupId;
		});
	const std::string patternId = pattern.strPatternId;
	ImGui::BeginDisabled(!valid || sameGroup);
	const bool setGroup = ImGui::Button("Set Group");
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!anyGroup || !m_TimelineSelectedStageIds.empty());
	const bool ungroup = ImGui::Button("Ungroup");
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!onlyColliders);
	const bool duplicate = ImGui::Button("Duplicate (same time)##ColliderGroup");
	ImGui::EndDisabled();
	if (duplicate)
	{
		std::string status;
		(void)Dispatch_CompositionEdit(
			COMPOSITION_EDIT_COMMAND::DUPLICATE_SELECTION, {}, {},
			[&](std::string &message) {
				return Duplicate_TimelineSelection(patternId, {}, m_TimelineSelectedOccurrenceIds, message, true);
			},
			status);
		return true;
	}
	if (setGroup || ungroup)
	{
		std::string status;
		(void)Set_ColliderSelectionGroup(patternId, m_TimelineSelectedOccurrenceIds, setGroup, status);
		return true;
	}
	if (onlyColliders && anyGroup)
	{
		ImGui::SeparatorText("Individual Colliders");
		ImGui::TextWrapped(
			"Edit a member to tune its Collider and Logic while keeping the group. Horizontal distance is in 4. Logic.");
		std::size_t memberIndex = 0u;
		for (const auto &id : m_TimelineSelectedOccurrenceIds)
		{
			const auto *member = Find_PresentationBox(pattern, id);
			const auto *definition = member ? Find_PresentationResource(m_Draft, member->strResourceId) : nullptr;
			if (!member || !definition || member->strSelectionGroupId.empty())
				continue;
			const std::string label = "Edit " + std::to_string(++memberIndex) + ": " + definition->strDisplayName +
				" (" + std::to_string(member->iStartMs) + " ms)##" + id;
			if (ImGui::Selectable(label.c_str()))
			{
				std::string status;
				(void)Select_ColliderGroupMemberById(patternId, id, status);
				return true;
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s", id.c_str());
		}
	}
	if (!valid)
	{
		ImGui::TextWrapped("%s", reason.c_str());
		return true;
	}
	if (sameGroup)
		ImGui::TextDisabled("%s", boxes.front().strSelectionGroupId.c_str());
	ImGui::TextWrapped(
		"One click selects a saved group. Drag its middle to move the complete timeline group. Edit a member above for individual values, or Ungroup to remove the group.");
	std::string selectionKey = patternId;
	std::vector<std::string> sortedIds = m_TimelineSelectedOccurrenceIds;
	std::sort(sortedIds.begin(), sortedIds.end());
	for (const auto &id : sortedIds)
		selectionKey += "\n" + id;
	if (selectionKey != m_strColliderSelectionTransformKey)
	{
		m_strColliderSelectionTransformKey = std::move(selectionKey);
		m_fColliderSelectionYaw = 0.f;
	}
	std::array<double, 3u> center{};
	for (const auto &box : boxes)
		for (std::size_t axis = 0u; axis < 3u; ++axis)
			center[axis] += box.PositionOffset[axis] / double(boxes.size());
	float position[3] = {float(center[0]), float(center[1]), float(center[2])};
	const bool moved = ImGui::DragFloat3("Group center offset (m)", position, .05f, -100000.f, 100000.f, "%.3f");
	float yaw = m_fColliderSelectionYaw;
	const bool rotated = ImGui::DragFloat("Group Y rotation (degrees)", &yaw, .1f, -36000.f, 36000.f, "%.3f");
	ImGui::TextDisabled(
		"Pivot: average of Collider centers in the shared Boss anchor. Rotation is relative to this selection.");
	if (moved || rotated)
	{
		std::array<double, 3u> translation{};
		if (moved)
			for (std::size_t axis = 0u; axis < 3u; ++axis)
				translation[axis] = double(position[axis]) - center[axis];
		std::string status;
		if (Transform_SelectedColliders(translation, double(yaw) - m_fColliderSelectionYaw, status))
			m_fColliderSelectionYaw = yaw;
	}
	return true;
}

// 저장된 Logic ID로 연결을 요청하는 진입점이다. 정의가 없으면 상태만 알리고 연결을 유지한다.
// 정의를 찾은 뒤의 값/시간 구간/Collider 연결 검증과 commit은 Set_ColliderLogicValues에 모은다.
bool_t Client::CKoukuSaydonActionWorkbench::Connect_ColliderLogic(const std::string &patternId,
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &occurrence, const std::string &logicId,
	std::string &outStatus)
{
	const auto *logic = Find_Logic(m_Draft, logicId);
	if (!logic)
	{
		outStatus = m_strStatus = "Selected Logic definition is unavailable; previous Collider preserved.";
		return false;
	}
	return Set_ColliderLogicValues(patternId, occurrence, *logic, outStatus);
}

// Collider Detail과 Render_LogicDefinitionValues의 Apply가 함께 사용하는 commit 경계다.
// 입력 occurrence와 정의 값을 후보 문서에 복사하고, 재사용/신규 Logic 구간 및 공유 시간 연결을 정한다.
// 검증 실패는 기존 draft와 미적용 입력을 유지하며, 성공한 뒤에만 Collider 편집 식별자와 dirty를 초기화한다.
bool_t Client::CKoukuSaydonActionWorkbench::Set_ColliderLogicValues(const std::string_view patternId,
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &occurrence,
	const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &values, std::string &outStatus)
{
	const auto reject = [&](const char *reason) {
		outStatus = m_strStatus = reason;
		return false;
	};
	auto candidate = m_Draft;
	auto *pattern = Find_Pattern(candidate, patternId);
	const auto definition = std::find_if(candidate.Logics.begin(), candidate.Logics.end(), [&](const auto &logic) {
		return logic.strLogicId == values.strLogicId;
	});
	const auto *resource = Find_PresentationResource(candidate, occurrence.strResourceId);
	if (!m_bHasDraft || !pattern || !pattern->strLoadError.empty() || definition == candidate.Logics.end() ||
		!resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER)
		return reject("Select an existing Collider and Logic definition; previous draft preserved.");
	const auto collider = std::find_if(
		pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(), [&](const auto &row) {
			return row.strOccurrenceId == occurrence.strOccurrenceId && row.strResourceId == occurrence.strResourceId;
		});
	if (collider == pattern->PresentationOccurrences.end())
		return reject("Collider identity changed; previous draft preserved.");
	auto updated = values;
	updated.strLogicId = definition->strLogicId;
	updated.strDisplayName = definition->strDisplayName;
	updated.strLogicType = definition->strLogicType;
	if (!Kouku_LogicAcceptsColliders(updated))
		return reject(
			"Collider connection needs a Duration or ENTER_AREA/OBJECT_CONTACT Trigger. Complete the Trigger settings before Apply; previous draft preserved.");
	*definition = std::move(updated);

	// 명시적으로 고른 Shared 항목은 자신의 시계를 유지한다. 현재 연결이나 연결되지 않은
	// 단일 구간은 ID를 유지하면서 이 Collider의 시계를 사용한다.
	bool_t adoptColliderClock = false;
	auto window = pattern->LogicOccurrences.end();
	if (!occurrence.strLogicOccurrenceId.empty())
	{
		window = std::find_if(pattern->LogicOccurrences.begin(), pattern->LogicOccurrences.end(), [&](const auto &row) {
			return row.strOccurrenceId == occurrence.strLogicOccurrenceId;
		});
		if (window == pattern->LogicOccurrences.end())
			return reject("Selected shared Logic window is unavailable; previous draft preserved.");
		if (window->strLogicId != definition->strLogicId)
			window = pattern->LogicOccurrences.end();
	}
	if (window == pattern->LogicOccurrences.end())
		for (auto row = pattern->LogicOccurrences.begin(); row != pattern->LogicOccurrences.end(); ++row)
			if (row->strLogicId == definition->strLogicId && row->iStartMs == occurrence.iStartMs &&
				row->iDurationMs == occurrence.iDurationMs)
			{
				if (window != pattern->LogicOccurrences.end())
					return reject(
						"Several Logic windows match this interval. Select the intended Shared Logic window; previous draft preserved.");
				window = row;
			}
	if (window == pattern->LogicOccurrences.end())
	{
		for (auto row = pattern->LogicOccurrences.begin(); row != pattern->LogicOccurrences.end(); ++row)
			if (row->strLogicId == definition->strLogicId &&
				std::none_of(pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(),
					[&](const auto &other) {
						return other.strLogicOccurrenceId == row->strOccurrenceId;
					}))
			{
				if (window != pattern->LogicOccurrences.end())
					return reject(
						"Several unconnected Logic windows use this definition. Select the intended Shared Logic window; previous draft preserved.");
				window = row;
				adoptColliderClock = true;
			}
	}
	if (window == pattern->LogicOccurrences.end())
	{
		if (pattern->iNextLogicOccurrenceOrdinal >= 1000000u)
			return reject("Logic window IDs are exhausted; previous draft preserved.");
		KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE created;
		created.strOccurrenceId =
			std::string(patternId) + ".logic." + std::to_string(pattern->iNextLogicOccurrenceOrdinal++);
		created.strLogicId = definition->strLogicId;
		created.iStartMs = occurrence.iStartMs;
		created.iDurationMs = occurrence.iDurationMs;
		pattern->LogicOccurrences.push_back(std::move(created));
		window = std::prev(pattern->LogicOccurrences.end());
	}
	if (adoptColliderClock || collider->strLogicOccurrenceId == window->strOccurrenceId)
	{
		window->iStartMs = occurrence.iStartMs;
		window->iDurationMs = occurrence.iDurationMs;
		for (auto &other : pattern->PresentationOccurrences)
			if (other.strLogicOccurrenceId == window->strOccurrenceId)
			{
				other.iStartMs = occurrence.iStartMs;
				other.iDurationMs = occurrence.iDurationMs;
			}
	}
	*collider = occurrence;
	collider->strLogicOccurrenceId = window->strOccurrenceId;
	collider->iStartMs = window->iStartMs;
	collider->iDurationMs = window->iDurationMs;
	if (!Valid_GameplayColliderScale(*resource, *collider))
		return reject(
			"Apply rejected: Collider scale must be finite and positive. Adjust Scale / size; pending edits and previous draft are preserved.");
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate),
			"Applied Collider values, Logic definition and window link together. Save keeps the connection.",
			outStatus))
		return false;
	m_strColliderExecutionEditId.clear();
	m_bColliderDamageDirty = false;
	return true;
}

// Collider Detail의 Debug Render 체크박스가 표시 플래그 하나를 즉시 commit할 때 호출한다.
// 박스/resource ID를 다시 검증하고 실패하면 draft를 유지한다. 성공 후에도 다른 Detail 임시 입력은
// 복원하므로 크기나 시간 입력까지 이 체크박스 동작에 섞여 확정되지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Set_PresentationBoxDebugRender(
	const std::string_view patternId, const std::string_view occurrenceId, const bool_t visible, std::string &outStatus)
{
	auto candidate = m_Draft;
	auto *pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern || !pattern->strLoadError.empty())
	{
		outStatus = m_strStatus = "Collider debug target Pattern is unavailable.";
		return false;
	}
	const auto box = std::find_if(pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(),
		[occurrenceId](const auto &row) {
			return row.strOccurrenceId == occurrenceId;
		});
	const auto *resource = box == pattern->PresentationOccurrences.end()
		? nullptr
		: Find_PresentationResource(candidate, box->strResourceId);
	if (nullptr == resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER)
	{
		outStatus = m_strStatus = "Debug Render requires an existing Collider box.";
		return false;
	}
	auto pendingEdit = m_PresentationBoxEdit;
	box->bDebugRender = visible;
	if (!Commit_Candidate(std::move(candidate), "Updated Collider Debug Render. Save keeps this flag.", outStatus))
		return false;
	// Synchronize_EditorFields는 commit된 행에서 입력을 다시 읽는다. 이 체크박스가
	// 미적용 크기/회전/시간까지 Apply하지 않게 해당 임시 입력을 복원한다.
	if (pendingEdit.strOccurrenceId == occurrenceId)
		pendingEdit.bDebugRender = visible;
	m_PresentationBoxEdit = std::move(pendingEdit);
	return true;
}

// Collider Detail의 Damage Apply가 접촉 Trigger, 성공 Damage Result, 시간 구간과 박스를
// 하나의 후보로 구성하는 경계다. 기존 정의 자체를 덮어쓰지 않고 일치하는 정의를 재사용하거나 새로 만든다.
// ID/연결/피해 값 검증이 실패하면 후보를 버리며, 같은 구간을 공유한 Collider의 시간은 함께 맞춘다.
bool_t Client::CKoukuSaydonActionWorkbench::Set_ColliderTriggerDamage(const std::string &patternId,
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &occurrence, const KOUKU_COLLIDER_DAMAGE_SETTINGS &settings,
	std::string &outStatus)
{
	const auto percent = settings.bFixedDamage ? 0u : static_cast<std::uint32_t>(settings.iPercent);
	const auto amount = settings.bFixedDamage ? static_cast<std::uint32_t>(settings.iDamageAmount) : 0u;
	const std::string damageKind = settings.bFixedDamage ? "FIXED_DAMAGE" : "MAX_HP_PERCENT_DAMAGE";
	if ((!settings.bFixedDamage && (settings.iPercent < 1 || settings.iPercent > 100)) ||
		(settings.bFixedDamage && (settings.iDamageAmount < 1 || settings.iDamageAmount > 1000000000)) ||
		(settings.bRearmOnExit && settings.bRepeatAfterKnockback) ||
		(settings.iRepeatIntervalMs &&
			(settings.iRepeatIntervalMs < 34u || settings.iRepeatIntervalMs > MAX_EDITOR_TIME_MS ||
				settings.bRearmOnExit || settings.bRepeatAfterKnockback)) ||
		!std::isfinite(settings.fPushHeightM) || settings.fPushHeightM < 0.0 || settings.fPushHeightM > 100.0 ||
		(settings.fPushHeightM > 0.0 && !settings.bPushBallistic) ||
		(settings.bRepeatAfterKnockback && (settings.fPushRangeM <= 0.0 && settings.fPushHeightM <= 0.0)) ||
		!std::isfinite(settings.fPushRangeM) || settings.fPushRangeM < 0.0 ||
		settings.fPushRangeM > (settings.bPushBallistic ? 100.0 : 20.0) || settings.iPushMs > MAX_EDITOR_TIME_MS ||
		((settings.fPushRangeM == 0.0 && settings.fPushHeightM == 0.0) != (settings.iPushMs == 0u)) ||
		(settings.strPushDirection != "AWAY_FROM_BOSS" && settings.strPushDirection != "BOSS_FORWARD" &&
			settings.strPushDirection != "AWAY_FROM_CONTACT") ||
		(settings.strPushDirection != "AWAY_FROM_BOSS" &&
			(settings.fPushRangeM <= 0.0 && settings.fPushHeightM <= 0.0)) ||
		((settings.bForcePush || settings.bPushCanLeaveArena) &&
			(settings.fPushRangeM <= 0.0 && settings.fPushHeightM <= 0.0)) ||
		(settings.bPushBallistic &&
			((settings.fPushRangeM <= 0.0 && settings.fPushHeightM <= 0.0) || settings.iPushMs < 100u ||
				settings.iPushMs > 5000u)) ||
		!std::isfinite(settings.fPushYawOffsetDegrees) || std::abs(settings.fPushYawOffsetDegrees) > 360.0 ||
		(settings.fPushYawOffsetDegrees != 0.0 &&
			(settings.strPushDirection != "BOSS_FORWARD" ||
				(settings.fPushRangeM <= 0.0 && settings.fPushHeightM <= 0.0))))
	{
		outStatus = m_strStatus = "Damage settings require bounded paired push values and one repeat policy.";
		return false;
	}
	auto candidate = m_Draft;
	auto *pattern = Find_Pattern(candidate, patternId);
	const auto *resource = Find_PresentationResource(candidate, occurrence.strResourceId);
	if (!m_bHasDraft || nullptr == pattern || !pattern->strLoadError.empty() || nullptr == resource ||
		resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER || resource->strColliderKind != "GEOMETRY")
	{
		outStatus = m_strStatus = "Trigger damage requires a Geometry Collider and bounded damage values.";
		return false;
	}
	auto collider = std::find_if(
		pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(), [&](const auto &row) {
			return row.strOccurrenceId == occurrence.strOccurrenceId && row.strResourceId == occurrence.strResourceId;
		});
	if (collider == pattern->PresentationOccurrences.end())
	{
		outStatus = m_strStatus = "Trigger Collider identity changed; previous draft preserved.";
		return false;
	}
	const auto oldWindowId = collider->strLogicOccurrenceId;
	auto *linked = Find_LogicBox(*pattern, occurrence.strLogicOccurrenceId);
	const auto *linkedLogic = nullptr == linked ? nullptr : Find_Logic(candidate, linked->strLogicId);
	if (!occurrence.strLogicOccurrenceId.empty() && nullptr == linked)
	{
		outStatus = m_strStatus = "Selected Trigger window is unavailable.";
		return false;
	}
	const std::string contactRole = settings.fPushRangeM > 0.0 || settings.fPushHeightM > 0.0 ? "KNOCKBACK" : "DAMAGE";
	const auto isPlainContact = [](const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &row) {
		KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION expected;
		expected.strLogicId = row.strLogicId;
		expected.strDisplayName = row.strDisplayName;
		expected.strLogicType = "TRIGGER";
		expected.strTriggerKind = "ENTER_AREA";
		expected.strColliderDamageContactRole = row.strColliderDamageContactRole;
		expected.bRearmOnExit = row.bRearmOnExit;
		expected.bRepeatAfterKnockback = row.bRepeatAfterKnockback;
		expected.iRepeatIntervalMs = row.iRepeatIntervalMs;
		return row == expected;
	};
	const bool linkedContact =
		linkedLogic && linkedLogic->strLogicType == "TRIGGER" && linkedLogic->strTriggerKind == "ENTER_AREA";
	const bool replacePlainContact = linkedContact && isPlainContact(*linkedLogic) &&
		linked->strHoldLogicOccurrenceId.empty() && !linked->RoomPlayerArrival && linked->OnFailLogicIds.empty() &&
		linked->OnTimeoutLogicIds.empty() &&
		std::all_of(linked->OnSuccessLogicIds.begin(), linked->OnSuccessLogicIds.end(), [&](const auto &id) {
			const auto *result = Find_Logic(candidate, id);
			return result && result->strLogicType == "RESULT" &&
				(result->strOutcomeKind == "MAX_HP_PERCENT_DAMAGE" || result->strOutcomeKind == "FIXED_DAMAGE");
		});
	if (!linkedContact || replacePlainContact)
	{
		// 명시적 damage contact만 재사용한다. grab 이름이나 기본값 일치는 소유 관계의 근거가 아니다.
		auto logic = std::find_if(candidate.Logics.begin(), candidate.Logics.end(), [&](const auto &row) {
			return row.strColliderDamageContactRole == contactRole && isPlainContact(row) &&
				row.bRearmOnExit == settings.bRearmOnExit &&
				row.bRepeatAfterKnockback == settings.bRepeatAfterKnockback &&
				row.iRepeatIntervalMs == settings.iRepeatIntervalMs;
		});
		if (logic == candidate.Logics.end())
		{
			if (candidate.iNextLogicOrdinal >= 1000000u)
			{
				outStatus = m_strStatus = "Logic IDs are exhausted.";
				return false;
			}
			KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION created;
			created.strLogicId = "koukusaydon.g1.logic." + std::to_string(candidate.iNextLogicOrdinal++);
			created.strDisplayName = contactRole == "KNOCKBACK"
				? "\xED\x8A\xB8\xEB\xA6\xAC\xEA\xB1\xB0\x5F\xEB\x8C\x80\xEB\xAF\xB8\xEC\xA7\x80\x5F\xEB\x84\x89\xEB\xB0\xB1"
				: "\xED\x8A\xB8\xEB\xA6\xAC\xEA\xB1\xB0\x5F\xEB\x8C\x80\xEB\xAF\xB8\xEC\xA7\x80";
			created.strLogicType = "TRIGGER";
			created.strTriggerKind = "ENTER_AREA";
			created.strColliderDamageContactRole = contactRole;
			created.bRearmOnExit = settings.bRearmOnExit;
			created.bRepeatAfterKnockback = settings.bRepeatAfterKnockback;
			created.iRepeatIntervalMs = settings.iRepeatIntervalMs;
			candidate.Logics.push_back(std::move(created));
			logic = std::prev(candidate.Logics.end());
		}
		if (replacePlainContact)
			linked->strLogicId = logic->strLogicId;
		else
		{
			if (pattern->iNextLogicOccurrenceOrdinal >= 1000000u)
			{
				outStatus = m_strStatus = "Logic window IDs are exhausted.";
				return false;
			}
			KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE window;
			window.strOccurrenceId = patternId + ".logic." + std::to_string(pattern->iNextLogicOccurrenceOrdinal++);
			window.strLogicId = logic->strLogicId;
			pattern->LogicOccurrences.push_back(std::move(window));
			linked = &pattern->LogicOccurrences.back();
		}
	}
	// 이 박스 편집이 공유 Trigger 정의를 변경하지 않도록 분리한다.
	const auto *currentTrigger = Find_Logic(candidate, linked->strLogicId);
	if (!currentTrigger)
	{
		outStatus = m_strStatus = "Damage Trigger is missing.";
		return false;
	}
	if (currentTrigger->bRearmOnExit != settings.bRearmOnExit ||
		currentTrigger->bRepeatAfterKnockback != settings.bRepeatAfterKnockback ||
		currentTrigger->iRepeatIntervalMs != settings.iRepeatIntervalMs)
	{
		if (candidate.iNextLogicOrdinal >= 1000000u)
		{
			outStatus = m_strStatus = "Logic IDs are exhausted.";
			return false;
		}
		auto changed = *currentTrigger;
		changed.strLogicId = "koukusaydon.g1.logic." + std::to_string(candidate.iNextLogicOrdinal++);
		changed.strDisplayName = resource->strDisplayName + " Damage contact";
		changed.bRearmOnExit = settings.bRearmOnExit;
		changed.bRepeatAfterKnockback = settings.bRepeatAfterKnockback;
		changed.iRepeatIntervalMs = settings.iRepeatIntervalMs;
		linked->strLogicId = changed.strLogicId;
		candidate.Logics.push_back(std::move(changed));
	}
	const auto windowId = linked->strOccurrenceId;
	auto damageSlot = linked->OnSuccessLogicIds.end();
	for (auto slot = linked->OnSuccessLogicIds.begin(); slot != linked->OnSuccessLogicIds.end(); ++slot)
	{
		const auto *result = Find_Logic(candidate, *slot);
		if (nullptr == result ||
			(result->strOutcomeKind != "MAX_HP_PERCENT_DAMAGE" && result->strOutcomeKind != "FIXED_DAMAGE"))
			continue;
		if (damageSlot != linked->OnSuccessLogicIds.end())
		{
			outStatus = m_strStatus =
				"Trigger has several damage Results; choose one in Success before editing damage.";
			return false;
		}
		damageSlot = slot;
	}
	if (damageSlot == linked->OnSuccessLogicIds.end() && linked->OnSuccessLogicIds.size() >= 4u)
	{
		outStatus = m_strStatus = "Success already has four Results; remove one before adding damage.";
		return false;
	}
	auto damage = std::find_if(candidate.Logics.begin(), candidate.Logics.end(), [&](const auto &row) {
		KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION desired;
		desired.strLogicId = row.strLogicId;
		desired.strDisplayName = row.strDisplayName;
		desired.strLogicType = "RESULT";
		desired.strOutcomeKind = damageKind;
		desired.iPercent = percent;
		desired.iDamageAmount = amount;
		desired.fPushRangeM = settings.fPushRangeM;
		desired.iPushMs = settings.iPushMs;
		desired.strPushDirection = settings.strPushDirection;
		desired.bForcePush = settings.bForcePush;
		desired.bPushCanLeaveArena = settings.bPushCanLeaveArena;
		desired.bPushBallistic = settings.bPushBallistic;
		desired.fPushHeightM = settings.fPushHeightM;
		desired.fPushYawOffsetDegrees = settings.fPushYawOffsetDegrees;
		return row == desired;
	});
	if (damage == candidate.Logics.end())
	{
		if (candidate.iNextLogicOrdinal >= 1000000u)
		{
			outStatus = m_strStatus = "Logic IDs are exhausted.";
			return false;
		}
		KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION result;
		result.strLogicId = "koukusaydon.g1.logic." + std::to_string(candidate.iNextLogicOrdinal++);
		result.strDisplayName = resource->strDisplayName + " Damage " +
			std::to_string(settings.bFixedDamage ? amount : percent) + (settings.bFixedDamage ? " HP" : "% max HP");
		result.strLogicType = "RESULT";
		result.strOutcomeKind = damageKind;
		result.iPercent = percent;
		result.iDamageAmount = amount;
		result.fPushRangeM = settings.fPushRangeM;
		result.iPushMs = settings.iPushMs;
		result.strPushDirection = settings.strPushDirection;
		result.bForcePush = settings.bForcePush;
		result.bPushCanLeaveArena = settings.bPushCanLeaveArena;
		result.bPushBallistic = settings.bPushBallistic;
		result.fPushHeightM = settings.fPushHeightM;
		result.fPushYawOffsetDegrees = settings.fPushYawOffsetDegrees;
		candidate.Logics.push_back(std::move(result));
		damage = std::prev(candidate.Logics.end());
	}
	// 다른 구간이 쓰는 Result를 바꾸지 않고 이 구간의 연결을 교체한다.
	if (damageSlot == linked->OnSuccessLogicIds.end())
		linked->OnSuccessLogicIds.push_back(damage->strLogicId);
	else
		*damageSlot = damage->strLogicId;
	linked->iStartMs = occurrence.iStartMs;
	linked->iDurationMs = occurrence.iDurationMs;
	*collider = occurrence;
	collider->strLogicOccurrenceId = windowId;
	if (!Valid_GameplayColliderScale(*resource, *collider))
	{
		outStatus = m_strStatus =
			"Apply rejected: Collider scale must be finite and positive. Adjust Scale / size; pending edits and previous draft are preserved.";
		return false;
	}
	for (auto &other : pattern->PresentationOccurrences)
		if (other.strLogicOccurrenceId == windowId)
		{
			other.iStartMs = occurrence.iStartMs;
			other.iDurationMs = occurrence.iDurationMs;
		}
	if (!oldWindowId.empty() && oldWindowId != windowId &&
		std::none_of(
			pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(), [&](const auto &row) {
				return row.strLogicOccurrenceId == oldWindowId;
			}))
	{
		const auto *old = Find_LogicBox(*pattern, oldWindowId);
		const auto *oldLogic = nullptr == old ? nullptr : Find_Logic(candidate, old->strLogicId);
		if (nullptr != oldLogic && oldLogic->strJudgementKind == "AREA_OVERLAP")
			std::erase_if(pattern->LogicOccurrences, [&](const auto &row) {
				return row.strOccurrenceId == oldWindowId;
			});
	}
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(
			std::move(candidate), "Saved Trigger geometry, ENTER_AREA window and damage Result.", outStatus))
		return false;
	m_strColliderExecutionEditId.clear();
	m_bColliderDamageDirty = false;
	return true;
}

// Render_PresentationBoxDetails가 Collider 종류를 만나면 위임하는 전용 Detail 화면이다.
// 크기/위치 드래그의 preview, 미적용 Logic/Damage 입력, 명시적 Apply를 구분해 보관한다.
// Apply나 연결 변경으로 draft generation이 바뀌면 즉시 반환해 이전 pattern/definition 차용을 끝낸다.
void Client::CKoukuSaydonActionWorkbench::Render_ColliderBoxDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern)
{
	const auto *box = Find_PresentationBox(pattern, m_strSelectedPresentationOccurrenceId);
	const auto *resource = box ? Find_PresentationResource(m_Draft, box->strResourceId) : nullptr;
	if (!resource)
		return;
	const auto definition = *resource;
	const auto patternId = pattern.strPatternId, occurrenceId = box->strOccurrenceId;
	auto &edit = m_PresentationBoxEdit;
	if (edit.strOccurrenceId != occurrenceId)
		edit = *box;
	const auto previewGeometry = [&]() {
		std::string status;
		(void)Request_PresentationGeometryPreview(patternId, edit, status);
	};
	const auto vectorControl = [](const char *label, std::array<double, 3u> &values, float minimum, float maximum) {
		float v[3] = {float(values[0]), float(values[1]), float(values[2])};
		if (!ImGui::DragFloat3(label, v, .05f, minimum, maximum, "%.3f"))
			return false;
		for (std::size_t axis = 0; axis < 3; ++axis)
			values[axis] = v[axis];
		return true;
	};
	const auto *linkedBox = Find_LogicBox(pattern, edit.strLogicOccurrenceId);
	const auto *linkedLogic = nullptr == linkedBox ? nullptr : Find_Logic(m_Draft, linkedBox->strLogicId);
	if (m_strColliderExecutionEditId != occurrenceId)
	{
		m_strColliderExecutionEditId = occurrenceId;
		m_strColliderExecutionType = nullptr == linkedLogic ? "DURATION" : linkedLogic->strLogicType;
		m_strColliderLogicDefinitionId = nullptr == linkedLogic ? "" : linkedLogic->strLogicId;
		m_ColliderDamageSettings = {};
		m_bColliderDamageMode = false;
		m_bColliderDetachDamage = linkedBox == nullptr;
		if (linkedLogic)
		{
			m_ColliderDamageSettings.bRearmOnExit = linkedLogic->bRearmOnExit;
			m_ColliderDamageSettings.bRepeatAfterKnockback = linkedLogic->bRepeatAfterKnockback;
			m_ColliderDamageSettings.iRepeatIntervalMs = linkedLogic->iRepeatIntervalMs;
		}
		m_bColliderDamageDirty = false;
		if (nullptr != linkedBox)
			for (const auto &id : linkedBox->OnSuccessLogicIds)
				if (const auto *result = Find_Logic(m_Draft, id); result &&
					(result->strOutcomeKind == "MAX_HP_PERCENT_DAMAGE" || result->strOutcomeKind == "FIXED_DAMAGE"))
				{
					m_ColliderDamageSettings.iPercent = static_cast<int32_t>(result->iPercent ? result->iPercent : 10u);
					m_ColliderDamageSettings.bFixedDamage = result->strOutcomeKind == "FIXED_DAMAGE";
					m_ColliderDamageSettings.iDamageAmount =
						static_cast<int32_t>(result->iDamageAmount ? result->iDamageAmount : 500u);
					m_ColliderDamageSettings.fPushRangeM = result->fPushRangeM;
					m_ColliderDamageSettings.iPushMs = result->iPushMs;
					m_ColliderDamageSettings.strPushDirection = result->strPushDirection;
					m_ColliderDamageSettings.bForcePush = result->bForcePush;
					m_ColliderDamageSettings.bPushCanLeaveArena = result->bPushCanLeaveArena;
					m_ColliderDamageSettings.bPushBallistic = result->bPushBallistic;
					m_ColliderDamageSettings.fPushHeightM = result->fPushHeightM;
					m_ColliderDamageSettings.fPushYawOffsetDegrees = result->fPushYawOffsetDegrees;
					m_bColliderDamageMode = linkedLogic && linkedLogic->strTriggerKind == "ENTER_AREA";
					break;
				}
	}

	if (!m_bColliderDamageDirty)
		if (const auto *contact = Find_Logic(m_Draft, m_strColliderLogicDefinitionId);
			contact && contact->strTriggerKind == "ENTER_AREA")
		{
			m_ColliderDamageSettings.bRearmOnExit = contact->bRearmOnExit;
			m_ColliderDamageSettings.bRepeatAfterKnockback = contact->bRepeatAfterKnockback;
			m_ColliderDamageSettings.iRepeatIntervalMs = contact->iRepeatIntervalMs;
		}
	const auto applyCollider = [&](std::string &status) {
		const auto value = edit;
		if (m_bColliderDamageMode)
			return Set_ColliderTriggerDamage(patternId, value, m_ColliderDamageSettings, status);
		if (m_bColliderDetachDamage)
		{
			auto detached = value;
			detached.strLogicOccurrenceId.clear();
			return Set_PresentationBox(patternId, detached, status);
		}
		if (const auto *selected = Find_Logic(m_Draft, m_strColliderLogicDefinitionId))
		{
			auto values =
				m_strColliderLogicValueDraftId == selected->strLogicId ? m_ColliderLogicValueDraft : *selected;
			if (values.strTriggerKind == "ENTER_AREA" && m_bColliderDamageDirty)
			{
				values.bRearmOnExit = m_ColliderDamageSettings.bRearmOnExit;
				values.bRepeatAfterKnockback = m_ColliderDamageSettings.bRepeatAfterKnockback;
				values.iRepeatIntervalMs = m_ColliderDamageSettings.iRepeatIntervalMs;
			}
			return Set_ColliderLogicValues(patternId, value, values, status);
		}
		return Set_PresentationBox(patternId, value, status);
	};
	ImGui::TextWrapped("Collider: %s", definition.strDisplayName.c_str());
	const bool grouped = !box->strSelectionGroupId.empty();
	if (grouped)
	{
		if (ImGui::Button("Back to Group##ColliderDetails"))
		{
			std::string status;
			(void)Select_PresentationBoxById(patternId, occurrenceId, status);
			return;
		}
		ImGui::SameLine();
	}
	ImGui::BeginDisabled(!grouped);
	const bool ungroup = ImGui::Button(grouped ? "Ungroup##ColliderDetails" : "Set Group##ColliderDetails");
	ImGui::EndDisabled();
	if (!grouped && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("Ctrl/Shift-click two or more Collider boxes, then Set Group.");
	if (ungroup)
	{
		std::string status;
		(void)Set_ColliderSelectionGroup(patternId, {occurrenceId}, false, status);
		return;
	}
	ImGui::SameLine();
	if (ImGui::Button("Duplicate##ColliderDetails"))
	{
		std::string status;
		if (applyCollider(status))
			(void)Dispatch_CompositionEdit(
				COMPOSITION_EDIT_COMMAND::DUPLICATE_SELECTION, {}, {},
				[&](std::string &message) {
					return Duplicate_TimelineSelection(patternId, {}, {occurrenceId}, message, true);
				},
				status);
		return;
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(
			"Apply current Collider settings and duplicate at the same time, including its connected Logic and linked Colliders. The copy is selected for editing.");
	const char *sections[] = {"1. Lifetime", "2. Motion / shape", "3. Trigger", "4. Logic"};
	for (int i = 0; i < 4; ++i)
	{
		if (i % 2)
			ImGui::SameLine();
		if (ImGui::RadioButton(sections[i], m_iColliderDetailSection == i))
			m_iColliderDetailSection = i;
	}
	ImGui::SeparatorText("Collider values");
	int startMs = int(edit.iStartMs), lifetimeMs = int(edit.iDurationMs);
	if (ImGui::InputInt("Start ms", &startMs))
		edit.iStartMs = std::uint32_t(std::clamp(startMs, 0, int(MAX_EDITOR_TIME_MS)));
	if (ImGui::InputInt("Lifetime ms", &lifetimeMs))
		edit.iDurationMs = std::uint32_t(std::clamp(lifetimeMs, 1, int(MAX_EDITOR_TIME_MS)));
	ImGui::Text(
		"Active: %u to %llu ms", edit.iStartMs, static_cast<unsigned long long>(edit.iStartMs) + edit.iDurationMs);
	const bool movable = edit.strColliderMotion == "LINEAR";
	bool changed = vectorControl("Position offset (m)", edit.PositionOffset, -100000.f, 100000.f);
	changed |= vectorControl("Rotation (degrees)", edit.RotationDegrees, -36000.f, 36000.f);
	changed |= vectorControl("Scale / size", edit.Scale, .001f, 10000.f);
	const auto beforeAnchor = edit;
	Render_PresentationAnchor(pattern, edit, false);
	if (edit.strAnchorKind != "MAP")
		ImGui::Checkbox("Follow anchor", &edit.bFollowBoss);
	if (edit.strAnchorKind == "BOSS" && edit.strBone.empty())
		ImGui::TextWrapped(edit.bFollowBoss ? "Follows the boss position and facing."
											: "Keeps the boss position and facing captured when this Collider starts.");
	if (movable && (definition.strShape == "CYLINDER" || definition.strShape == "CIRCLE"))
		ImGui::TextDisabled("Circular motion requires equal X/Z scale at both ends.");
	changed |= beforeAnchor.strAnchorKind != edit.strAnchorKind || beforeAnchor.bFollowBoss != edit.bFollowBoss ||
		beforeAnchor.strBone != edit.strBone || beforeAnchor.strBoneTarget != edit.strBoneTarget ||
		beforeAnchor.strBoneRotation != edit.strBoneRotation || beforeAnchor.strWorldId != edit.strWorldId ||
		beforeAnchor.strWorldOccurrenceId != edit.strWorldOccurrenceId ||
		beforeAnchor.iWorldEmissionIndex != edit.iWorldEmissionIndex ||
		beforeAnchor.strWorldEffectTrackId != edit.strWorldEffectTrackId;
	ImGui::BeginDisabled(!movable);
	changed |= vectorControl("End position offset (m)", edit.ColliderEndPositionOffset, -100000.f, 100000.f);
	changed |= vectorControl("End size (scale)", edit.ColliderEndScale, .001f, 10000.f);
	ImGui::EndDisabled();
	if (movable)
	{
		ImGui::TextWrapped(
			"Position and size interpolate from start to end over Lifetime. Y is height; both positions use the selected anchor.");
		if (definition.strShape == "CYLINDER" || definition.strShape == "BOX")
		{
			if (ImGui::Button("Keep bottom fixed while growing"))
			{
				edit.ColliderEndPositionOffset = edit.PositionOffset;
				edit.ColliderEndPositionOffset[1] +=
					definition.HalfExtents[1] * (edit.ColliderEndScale[1] - edit.Scale[1]);
				changed = true;
			}
		}
	}
	if (changed)
		previewGeometry();
	if ((edit.RotationDegrees[0] != 0.0 || edit.RotationDegrees[2] != 0.0) && edit.strBoneRotation != "BONE")
		ImGui::TextWrapped("Server contact supports Y rotation, or a BONE frame projected after local rotation.");
	if (!movable)
		ImGui::TextDisabled("End position and size: choose Movable / growing in 2. Motion / shape.");
	ImGui::Separator();
	if (m_iColliderDetailSection == 0)
	{
		ImGui::TextWrapped(
			"Start is measured from this Pattern. The Collider and its connected contact window share this lifetime.");
	}
	else if (m_iColliderDetailSection == 1)
	{
		if (ImGui::BeginCombo("Shape", definition.strShape.c_str()))
		{
			std::string selectedShape;
			for (const char *shape : {"BOX", "CIRCLE", "CYLINDER", "SECTOR", "REVERSE_SECTOR"})
				if (ImGui::Selectable(shape, definition.strShape == shape) && definition.strShape != shape)
					selectedShape = shape;
			ImGui::EndCombo();
			if (!selectedShape.empty())
			{
				auto candidate = m_Draft;
				if (candidate.iNextPresentationResourceOrdinal >= 1000000u)
				{
					m_strStatus = "Collider resource IDs exhausted.";
					return;
				}
				auto dedicated = definition;
				dedicated.strResourceId =
					"koukusaydon.g1.presentation." + std::to_string(candidate.iNextPresentationResourceOrdinal++);
				dedicated.strShape = selectedShape;
				if (selectedShape == "BOX" || selectedShape == "CYLINDER" || selectedShape == "REVERSE_SECTOR")
					dedicated.fInnerRadiusM = 0.0;
				auto pending = edit;
				pending.strResourceId = dedicated.strResourceId;
				candidate.PresentationResources.push_back(std::move(dedicated));
				auto *owner = Find_Pattern(candidate, patternId);
				for (auto &row : owner->PresentationOccurrences)
					if (row.strOccurrenceId == occurrenceId)
						row.strResourceId = pending.strResourceId;
				Mark_Draft(candidate, *owner);
				std::string status;
				if (Commit_Candidate(
						std::move(candidate), "Changed this Collider shape. Other boxes keep their shape.", status))
					m_PresentationBoxEdit = pending;
				return;
			}
		}
		int motion = edit.strColliderMotion == "LINEAR" ? 1 : 0;
		if (ImGui::Combo("Type", &motion, "Static\0Movable / growing\0"))
		{
			edit.strColliderMotion = motion ? "LINEAR" : "STATIC";
			if (motion)
			{
				edit.ColliderEndPositionOffset = edit.PositionOffset;
				edit.ColliderEndScale = edit.Scale;
			}
			previewGeometry();
		}
		if (definition.strShape == "BOX")
		{
			float fullSize[3]{};
			for (std::size_t i = 0u; i < 3u; ++i)
				fullSize[i] = static_cast<float>(2.0 * definition.HalfExtents[i] * edit.Scale[i]);
			if (ImGui::DragFloat3("Width / height / depth (m)", fullSize, 0.05f, 0.01f, 10000.f))
			{
				for (std::size_t i = 0u; i < 3u; ++i)
					edit.Scale[i] = fullSize[i] / (2.0 * definition.HalfExtents[i]);
				previewGeometry();
			}
		}
		else
		{
			if ((definition.strShape == "CIRCLE" || definition.strShape == "CYLINDER"))
			{
				float radius = static_cast<float>(definition.fRadiusM * (std::max)(edit.Scale[0], edit.Scale[2]));
				if (ImGui::DragFloat("Radius (m)", &radius, .05f, .01f, 10000.f))
				{
					edit.Scale[0] = edit.Scale[2] = radius / definition.fRadiusM;
					previewGeometry();
				}
			}
			else
			{
				float axes[2] = {static_cast<float>(definition.fRadiusM * edit.Scale[0]),
					static_cast<float>(definition.fRadiusM * edit.Scale[2])};
				if (definition.fInnerRadiusM > 0.0)
				{
					if (ImGui::DragFloat("Outer radius (m)", &axes[0], .05f, .01f, 10000.f))
					{
						edit.Scale[0] = edit.Scale[2] = axes[0] / definition.fRadiusM;
						previewGeometry();
					}
				}
				else if (ImGui::DragFloat2("Radius X / Z (m)", axes, .05f, .01f, 10000.f))
				{
					edit.Scale[0] = axes[0] / definition.fRadiusM;
					edit.Scale[2] = axes[1] / definition.fRadiusM;
					previewGeometry();
				}
				float angle = static_cast<float>(definition.fHalfAngleDegrees * 2.0);
				if (ImGui::DragFloat(
						definition.strShape == "REVERSE_SECTOR" ? "Safe angle (degrees)" : "Sector angle (degrees)",
						&angle, .1f, definition.strShape == "REVERSE_SECTOR" ? 0.f : .002f, 360.f))
				{
					auto candidate = m_Draft;
					for (auto &item : candidate.PresentationResources)
						if (item.strResourceId == definition.strResourceId)
							item.fHalfAngleDegrees =
								std::clamp(angle, definition.strShape == "REVERSE_SECTOR" ? 0.f : .002f, 360.f) * .5;
					for (auto &affected : candidate.Patterns)
						if (std::any_of(affected.PresentationOccurrences.begin(),
								affected.PresentationOccurrences.end(), [&](const auto &row) {
									return row.strResourceId == definition.strResourceId;
								}))
							Mark_Draft(candidate, affected);
					std::string status;
					const auto pending = edit;
					(void)Commit_Candidate(std::move(candidate), "Updated shared Collider angle.", status);
					m_PresentationBoxEdit = pending;
					return;
				}
				if (definition.strShape == "REVERSE_SECTOR")
					ImGui::TextDisabled(
						"Safe angle 0 = full ellipse; 360 = empty. X/Z scale changes each axis independently.");
			}
		}
		if (definition.strShape == "CYLINDER")
		{
			float height = float(2.0 * definition.HalfExtents[1] * edit.Scale[1]);
			if (ImGui::DragFloat("Height (m)", &height, .05f, .001f, 10000.f))
			{
				edit.Scale[1] = height / (2.0 * definition.HalfExtents[1]);
				previewGeometry();
			}
		}
		if (definition.strShape == "CIRCLE" || definition.strShape == "SECTOR")
		{
			float inner = static_cast<float>(definition.fInnerRadiusM * edit.Scale[0]);
			const float limit = static_cast<float>(definition.fRadiusM * edit.Scale[0] * .999);
			if (ImGui::DragFloat("Inner radius (m)##Collider", &inner, .01f, 0.f, limit))
			{
				auto candidate = m_Draft;
				for (auto &item : candidate.PresentationResources)
					if (item.strResourceId == definition.strResourceId)
						item.fInnerRadiusM = std::clamp(inner, 0.f, limit) / edit.Scale[0];
				for (auto &affected : candidate.Patterns)
					if (std::any_of(affected.PresentationOccurrences.begin(), affected.PresentationOccurrences.end(),
							[&](const auto &row) {
								return row.strResourceId == definition.strResourceId;
							}))
						Mark_Draft(candidate, affected);
				std::string status;
				const auto pending = edit;
				(void)Commit_Candidate(std::move(candidate), "Updated shared Collider inner radius.", status);
				m_PresentationBoxEdit = pending;
				return;
			}
			ImGui::TextDisabled("Inner radius 0 fills the center. A hollow region requires equal X/Z scale.");
		}

		bool debugRender = edit.bDebugRender;
		if (ImGui::Checkbox("Debug Render", &debugRender))
		{
			std::string status;
			(void)Set_PresentationBoxDebugRender(patternId, occurrenceId, debugRender, status);
			return;
		}
	}
	else if (m_iColliderDetailSection == 2)
	{
		auto &settings = m_ColliderDamageSettings;
		const auto *selectedLogic = Find_Logic(m_Draft, m_strColliderLogicDefinitionId);
		const bool otherMechanic =
			!m_bColliderDamageMode && selectedLogic && selectedLogic->strTriggerKind != "ENTER_AREA";
		if (otherMechanic)
			ImGui::TextWrapped(
				"This mechanic uses its own conditions in 4. Logic. Player contact ticks require ENTER_AREA.");
		ImGui::BeginDisabled(otherMechanic);
		int repeat = settings.iRepeatIntervalMs ? 3
			: settings.bRepeatAfterKnockback	? 2
			: settings.bRearmOnExit				? 1
												: 0;
		if (ImGui::Combo("Player contact", &repeat,
				"Once per lifetime\0On exit and re-entry\0After knockback finishes\0Every interval while touching\0"))
		{
			settings.bRearmOnExit = repeat == 1;
			settings.bRepeatAfterKnockback = repeat == 2;
			settings.iRepeatIntervalMs = repeat == 3 ? 500u : 0u;
			if (repeat == 2 && settings.fPushRangeM == 0.0)
			{
				settings.fPushRangeM = 2.0;
				settings.iPushMs = 242u;
			}
			m_bColliderDamageDirty = true;
		}
		if (repeat == 3)
		{
			int interval = int(settings.iRepeatIntervalMs);
			if (ImGui::InputInt("Tick interval ms", &interval, 10, 100))
			{
				settings.iRepeatIntervalMs = std::uint32_t(std::clamp(interval, 34, int(MAX_EDITOR_TIME_MS)));
				m_bColliderDamageDirty = true;
			}
		}
		ImGui::EndDisabled();
		ImGui::TextWrapped(
			"Each player has a separate contact history. Choose Damage / knockback or an ENTER_AREA Logic in 4. Logic, then Apply.");
	}
	else
	{
		if (linkedBox)
		{
			const auto shared = std::count_if(
				pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(), [&](const auto &row) {
					return row.strLogicOccurrenceId == linkedBox->strOccurrenceId;
				});
			if (shared > 1)
				ImGui::TextWrapped(
					"%d Colliders share this contact window. Damage and Horizontal distance changes apply to this whole row.",
					static_cast<int>(shared));
		}
		int mode = m_bColliderDamageMode ? 1 : m_bColliderDetachDamage ? 0 : 2;
		if (ImGui::Combo(
				"Logic connection", &mode, "None (preview only)\0Damage / knockback\0Existing Logic / mechanic\0"))
		{
			m_bColliderDamageMode = mode == 1;
			m_bColliderDetachDamage = mode == 0;
			if (mode == 0)
			{
				edit.strLogicOccurrenceId.clear();
				m_strColliderLogicDefinitionId.clear();
			}
			if (mode == 2)
			{
				m_bColliderDetachDamage = false;
				m_strColliderExecutionType = "TRIGGER";
			}
			m_bColliderDamageDirty = true;
		}
		if (m_bColliderDamageMode)
		{
			auto &damage = m_ColliderDamageSettings;
			const auto previousDamage = damage;
			int damageType = damage.bFixedDamage ? 1 : 0;
			if (ImGui::Combo("Damage type", &damageType, "Max HP percent\0Fixed HP\0"))
				damage.bFixedDamage = damageType == 1;
			if (damage.bFixedDamage)
			{
				if (ImGui::InputInt("Damage (HP)", &damage.iDamageAmount, 50, 500))
					damage.iDamageAmount = std::clamp(damage.iDamageAmount, 1, 1000000000);
			}
			else if (ImGui::InputInt("Damage (% max HP)", &damage.iPercent))
				damage.iPercent = std::clamp(damage.iPercent, 1, 100);
			bool push = damage.fPushRangeM > 0.0 || damage.fPushHeightM > 0.0;
			if (ImGui::Checkbox("Knockback", &push))
			{
				damage.fPushRangeM = push ? 2.0 : 0.0;
				damage.iPushMs = push ? 500u : 0u;
				if (!push)
				{
					damage.strPushDirection = "AWAY_FROM_BOSS";
					damage.bRepeatAfterKnockback = false;
					damage.bForcePush = damage.bPushCanLeaveArena = damage.bPushBallistic = false;
					damage.fPushYawOffsetDegrees = damage.fPushHeightM = 0.0;
				}
			}
			if (push)
			{
				if (ImGui::Checkbox("Rise and fall", &damage.bPushBallistic))
				{
					damage.fPushHeightM = damage.bPushBallistic ? 2.0 : 0.0;
					if (!damage.bPushBallistic)
						damage.fPushRangeM = std::clamp(damage.fPushRangeM, .01, 20.0);
					damage.iPushMs = std::clamp(damage.iPushMs, 100u, 5000u);
				}
				float distance = float(damage.fPushRangeM);
				int duration = int(damage.iPushMs);
				if (ImGui::DragFloat("Horizontal distance (m)", &distance, .05f, damage.bPushBallistic ? 0.f : .01f,
						damage.bPushBallistic ? 100.f : 20.f))
					damage.fPushRangeM = distance;
				if (ImGui::InputInt("Knockback lifetime ms", &duration, 10, 100))
					damage.iPushMs = std::uint32_t(std::clamp(duration, damage.bPushBallistic ? 100 : 1,
						damage.bPushBallistic ? 5000 : int(MAX_EDITOR_TIME_MS)));
				if (damage.bPushBallistic)
				{
					float height = float(damage.fPushHeightM);
					if (ImGui::DragFloat("Rise height (m)", &height, .05f, .01f, 100.f))
						damage.fPushHeightM = height;
				}
				int direction = damage.strPushDirection == "BOSS_FORWARD" ? 1
					: damage.strPushDirection == "AWAY_FROM_CONTACT"	  ? 2
																		  : 0;
				if (ImGui::Combo("Direction", &direction, "Away from boss\0Boss forward\0Away from contact\0"))
				{
					damage.strPushDirection = direction == 1 ? "BOSS_FORWARD"
						: direction == 2					 ? "AWAY_FROM_CONTACT"
															 : "AWAY_FROM_BOSS";
					if (direction != 1)
						damage.fPushYawOffsetDegrees = 0.0;
				}
				if (direction == 1)
				{
					float yaw = float(damage.fPushYawOffsetDegrees);
					if (ImGui::SliderFloat("Direction offset (degrees)", &yaw, -360.f, 360.f))
						damage.fPushYawOffsetDegrees = yaw;
				}
				ImGui::Checkbox("Force knockback reaction", &damage.bForcePush);
				if (pattern.strGateId == "GATE1")
				{
					damage.bPushCanLeaveArena = false;
					ImGui::TextDisabled("Gate 1: landing stays inside the arena fence.");
				}
				else
					ImGui::Checkbox("Allow arena-edge fall", &damage.bPushCanLeaveArena);
			}
			// Apply 전에는 현재 임시 입력을 유지한다. 특히 knockback을 끌 때
			// 이전 repeat-after-knockback 정책을 다시 읽어 오지 않는다.
			if (damage.bFixedDamage != previousDamage.bFixedDamage ||
				damage.iDamageAmount != previousDamage.iDamageAmount || damage.iPercent != previousDamage.iPercent ||
				damage.fPushRangeM != previousDamage.fPushRangeM ||
				damage.fPushHeightM != previousDamage.fPushHeightM || damage.iPushMs != previousDamage.iPushMs ||
				damage.strPushDirection != previousDamage.strPushDirection ||
				damage.bForcePush != previousDamage.bForcePush ||
				damage.bPushCanLeaveArena != previousDamage.bPushCanLeaveArena ||
				damage.bPushBallistic != previousDamage.bPushBallistic ||
				damage.fPushYawOffsetDegrees != previousDamage.fPushYawOffsetDegrees ||
				damage.bRepeatAfterKnockback != previousDamage.bRepeatAfterKnockback)
				m_bColliderDamageDirty = true;
		}
		else if (!m_bColliderDetachDamage)
		{
			if (ImGui::CollapsingHeader("Choose existing Logic / outcomes"))
			{
				if (ImGui::BeginCombo("Execution", m_strColliderExecutionType.c_str()))
				{
					for (const char *type : {"DURATION", "TRIGGER"})
						if (ImGui::Selectable(type, m_strColliderExecutionType == type))
						{
							m_strColliderExecutionType = type;
							m_strColliderLogicDefinitionId.clear();
							m_bColliderDamageDirty = false;
						}
					ImGui::EndCombo();
				}
				const auto compatibleWindowCount = std::count_if(
					pattern.LogicOccurrences.begin(), pattern.LogicOccurrences.end(), [&](const auto &window) {
						const auto *logic = Find_Logic(m_Draft, window.strLogicId);
						return logic && logic->strLogicType == m_strColliderExecutionType &&
							(Kouku_LogicAcceptsColliders(*logic) ||
								(logic->strLogicType == "TRIGGER" && logic->strTriggerKind.empty()));
					});
				if (ImGui::BeginCombo(
						"Shared Logic window", nullptr == linkedBox ? "(none)" : linkedBox->strOccurrenceId.c_str()))
				{
					if (ImGui::Selectable("(none)", edit.strLogicOccurrenceId.empty()))
					{
						edit.strLogicOccurrenceId.clear();
						m_strColliderLogicDefinitionId.clear();
						m_bColliderDamageDirty = false;
					}
					for (const auto &window : pattern.LogicOccurrences)
					{
						const auto *logic = Find_Logic(m_Draft, window.strLogicId);
						const bool nameOnlyTrigger =
							logic && logic->strLogicType == "TRIGGER" && logic->strTriggerKind.empty();
						if (nullptr == logic || logic->strLogicType != m_strColliderExecutionType ||
							(!Kouku_LogicAcceptsColliders(*logic) && !nameOnlyTrigger))
							continue;
						const auto label = logic->strDisplayName + (nameOnlyTrigger ? " (set Trigger kind)" : "") +
							" | " + window.strOccurrenceId;
						if (ImGui::Selectable(label.c_str(), edit.strLogicOccurrenceId == window.strOccurrenceId))
						{
							edit.strLogicOccurrenceId = window.strOccurrenceId;
							m_strColliderLogicDefinitionId = window.strLogicId;
							m_bColliderDamageDirty = false;
							edit.iStartMs = window.iStartMs;
							edit.iDurationMs = window.iDurationMs;
						}
					}
					ImGui::EndCombo();
				}
				if (m_strColliderExecutionType == "TRIGGER" && compatibleWindowCount == 0)
					ImGui::TextWrapped(
						"No reusable Trigger window is placed yet. Choose a Logic definition below and Apply Values to create its window and connect this Collider.");
				if (m_strColliderExecutionType == "TRIGGER")
					ImGui::TextWrapped(
						"ENTER_AREA detects players entering this Collider, including a following boss-body region. OBJECT_CONTACT detects authored World card placements for hammer reactions.");
				const auto *definitionLogic = Find_Logic(m_Draft, m_strColliderLogicDefinitionId);
				std::string definitionLabel = definitionLogic ? definitionLogic->strDisplayName : "(select Logic)";
				if (definitionLogic && definitionLogic->strLogicType == "TRIGGER" &&
					definitionLogic->strTriggerKind.empty())
					definitionLabel += " (set Trigger kind)";
				if (ImGui::BeginCombo("Logic definition", definitionLabel.c_str()))
				{
					for (const auto &logic : m_Draft.Logics)
					{
						const bool nameOnlyTrigger = logic.strLogicType == "TRIGGER" && logic.strTriggerKind.empty();
						if (logic.strLogicType != m_strColliderExecutionType ||
							(!Kouku_LogicAcceptsColliders(logic) && !nameOnlyTrigger))
							continue;
						const std::string label = logic.strDisplayName +
							(nameOnlyTrigger ? " (set Trigger kind)" : "") + "##" + logic.strLogicId;
						if (ImGui::Selectable(label.c_str(), m_strColliderLogicDefinitionId == logic.strLogicId))
						{
							m_strColliderLogicDefinitionId = logic.strLogicId;
							m_bColliderDamageDirty = false;
						}
					}
					ImGui::EndCombo();
				}
				// 위쪽 선택 처리에서 같은 프레임에 정의가 바뀌었을 수 있어 다시 조회한다.
				definitionLogic = Find_Logic(m_Draft, m_strColliderLogicDefinitionId);
				if (definitionLogic && definitionLogic->strLogicType == "TRIGGER")
				{
					const auto definitionCopy = *definitionLogic;
					const auto pendingPresentation = edit;
					const auto generation = m_iDraftGeneration;
					Render_LogicDefinitionValues(definitionCopy, patternId, &pendingPresentation);
					// 하나의 Apply에서 정의, 시간 구간, Collider 연결을 함께 commit한다.
					if (generation != m_iDraftGeneration)
						return;
					m_PresentationBoxEdit = pendingPresentation;
				}
				const auto *connectionLogic = definitionLogic;
				if (definitionLogic && definitionLogic->strLogicType == "TRIGGER" &&
					m_strColliderLogicValueDraftId == definitionLogic->strLogicId)
					connectionLogic = &m_ColliderLogicValueDraft;
				ImGui::BeginDisabled(!connectionLogic || !Kouku_LogicAcceptsColliders(*connectionLogic));
				if (ImGui::Button("Append / reuse Logic window"))
				{
					std::string status;
					const auto value = edit;
					(void)Set_ColliderLogicValues(patternId, value, *connectionLogic, status);
					ImGui::EndDisabled();
					return;
				}
				ImGui::EndDisabled();
				ImGui::TextWrapped(
					"Apply keeps this Collider's 3D size and synchronizes its start/lifetime with the current Logic window and linked Colliders. Selecting a different Shared Logic window first adopts that window's time. Result slots remain shared.");
				if (nullptr != linkedLogic && linkedLogic->strJudgementKind == "AREA_OVERLAP" &&
					ImGui::BeginCombo("Inside outcome##ColliderBox", linkedLogic->strInsideOutcome.c_str()))
				{
					for (const char *outcome : {"SUCCESS", "FAIL"})
						if (ImGui::Selectable(outcome, linkedLogic->strInsideOutcome == outcome))
						{
							auto value = *linkedLogic;
							value.strInsideOutcome = outcome;
							std::string status;
							(void)Set_LogicDefinitionValues(value.strLogicId, value, status);
							ImGui::EndCombo();
							return;
						}
					ImGui::EndCombo();
				}
				if (!box->strLogicOccurrenceId.empty() &&
					Render_LogicOutcomeSlots(patternId, box->strLogicOccurrenceId))
					return;
			}
		}
		if (definition.strColliderKind == "ROULETTE_CARD_REGION")
		{
			char regionId[256]{};
			(void)Copy_Text(regionId, std::size(regionId), edit.strRegionId);
			if (ImGui::InputText("Region ID", regionId, std::size(regionId)))
				edit.strRegionId = regionId;
			if (ImGui::BeginCombo("Card symbol", edit.strCardSymbol.c_str()))
			{
				for (const char *symbol : {"NONE", "HEART", "SPADE", "CLUB", "DIAMOND"})
					if (ImGui::Selectable(symbol, edit.strCardSymbol == symbol))
						edit.strCardSymbol = symbol;
				ImGui::EndCombo();
			}
			if (ImGui::BeginCombo("Card color", edit.strCardColor.c_str()))
			{
				for (const char *color : {"NONE", "RED", "BLACK"})
					if (ImGui::Selectable(color, edit.strCardColor == color))
						edit.strCardColor = color;
				ImGui::EndCombo();
			}
			ImGui::TextWrapped(
				"Map the observed floor symbol and color, then connect this Region ID in Roulette Logic. Unmapped regions stay DRAFT.");
		}
	}
	ImGui::Separator();
	if (ImGui::Button("Apply##ColliderDetails"))
	{
		std::string status;
		(void)applyCollider(status);
		return;
	}
	ImGui::SameLine();
	if (ImGui::Button("Preview##ColliderDetails"))
		Queue_PresentationPreview(definition, &edit);
	ImGui::SameLine();
	if (ImGui::Button("Revert##ColliderDetails"))
	{
		edit = *box;
		m_strColliderExecutionEditId.clear();
		previewGeometry();
		return;
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete##ColliderDetails"))
	{
		std::string status;
		(void)Delete_PresentationBox(patternId, occurrenceId, status);
		return;
	}
	ImGui::TextWrapped(
		"Apply Collider values, then use Play Pattern to test this draft on the Server. Play Preview and timeline scrubbing inspect the local presentation. Save and Publish remain separate.");
	ImGui::TextWrapped("%s", m_strStatus.c_str());
}

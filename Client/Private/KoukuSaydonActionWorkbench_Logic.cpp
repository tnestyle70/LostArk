// Logic 정의·타임라인 박스와 선택 패널 draft를 편집한다. 적용은 검증한 후보를 세션에 commit한다.
#include "imgui.h"
#include "KoukuSaydonActionWorkbench.h"
#include "KoukuSaydonActionWorkbench_Internal.h"

// 격리된 Pattern의 보존 JSON에서도 정의 참조를 확인해 삭제 가능 여부를 판단한다.
#include "DataJson.h"

// stable ID 조회, 선택 후보의 값 복사와 시간 범위 계산에 필요한 표준 타입이다.
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace Client::KoukuWorkbenchDetail;

namespace
{
using namespace Client;

// 춤 슬롯이 섞이기 전 pose index와 Q/W/E/R 원본 clip의 대응이다. 화면 label이며 저장 ID를 바꾸지 않는다.
constexpr std::array<const char_t *, 4u> DANCE_POSE_LABELS = {
	"0: Q Arms folded (25_03)", "1: W Superman (25_04)", "2: E Arms open (25_05)", "3: R One leg (25_06)"};

// Delete_Logic와 리소스 목록이 사용하는 참조 집계다. 정상 문서는 typed occurrence를, 격리 문서는 보존 JSON을 읽는다.
// outUnresolved는 참조가 없다는 결론을 내릴 수 없는 상태를 별도로 전달해 잘못된 정의 삭제를 막는다.
std::size_t Count_LogicReferences(const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const std::string_view logicId,
	bool_t *const outUnresolved = nullptr)
{
	std::size_t count = 0u;
	bool_t unresolved = false;
	for (const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern : document.Patterns)
	{
		if (pattern.strLoadError.empty())
		{
			for (const auto &box : pattern.LogicOccurrences)
			{
				if (box.strLogicId == logicId)
					++count;
				for (const KOUKU_SAYDON_OUTCOME_SLOT slot : {KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS,
						 KOUKU_SAYDON_OUTCOME_SLOT::FAIL, KOUKU_SAYDON_OUTCOME_SLOT::TIMEOUT})
				{
					for (const std::string &target : box.Outcomes(slot))
						if (target == logicId)
							++count;
				}
			}
			continue;
		}
		// 로드에 실패해 격리된 Pattern도 보존한 원본 JSON 안에 참조가 남아 있으므로 삭제 판정에서 제외하지 않는다.
		DATA_JSON_VALUE preserved;
		std::string error;
		if (!CDataJson::Parse(pattern.strPreservedJson, preserved, error) || !preserved.Is_Object())
		{
			unresolved = true;
			continue;
		}
		const auto *boxes = preserved.Find("logicOccurrences");
		if (nullptr == boxes)
			continue;
		if (!boxes->Is_Array())
		{
			unresolved = true;
			continue;
		}
		for (const auto &box : boxes->Get_Array())
		{
			const auto *reference = box.Find("logicId");
			if (nullptr == reference || !reference->Is_String())
				unresolved = true;
			else if (reference->Get_String() == logicId)
				++count;
			// 구형 단일 Outcome 키는 생략할 수 있지만, 존재하면서 문자열이 아니면 참조 여부를 확정할 수 없다.
			for (const char_t *const outcomeKey : {"onSuccessLogicId", "onTimeoutLogicId"})
			{
				const auto *outcome = box.Find(outcomeKey);
				if (nullptr == outcome)
					continue;
				if (!outcome->Is_String())
					unresolved = true;
				else if (outcome->Get_String() == logicId)
					++count;
			}
			for (const char_t *const outcomeKey : {"onSuccessLogicIds", "onFailLogicIds", "onTimeoutLogicIds"})
			{
				const auto *outcomes = box.Find(outcomeKey);
				if (nullptr == outcomes)
					continue;
				if (!outcomes->Is_Array())
				{
					unresolved = true;
					continue;
				}
				for (const auto &outcome : outcomes->Get_Array())
				{
					if (!outcome.Is_String())
						unresolved = true;
					else if (outcome.Get_String() == logicId)
						++count;
				}
			}
		}
	}
	if (nullptr != outUnresolved)
		*outUnresolved = unresolved;
	return count;
}

// Render_LogicDefinitionValues가 소유한 편집 draft의 공격 목록을 표시하고 수정한다.
// 여기서는 미완성 값을 경고와 함께 유지한다. 실제 문서 반영은 Apply Values의
// Set_LogicDefinitionValues가 검증하며, 이 UI는 Server 피해 판정을 실행하지 않는다.
void Render_AttackHitTemplates(const char *label, std::vector<LostArk::Shared::ATTACK_HIT_TEMPLATE> &hits)
{
	ImGui::PushID(label);
	if (ImGui::TreeNode(label))
	{
		ImGui::TextWrapped(
			"One shape per attack; time is relative to each spawned object. Offsets and direction follow that object's pivot. Damage is Server-owned.");
		const auto integer = [](const char *name, std::uint32_t &value, const int minimum, const int maximum) {
			int editedValue = int(value);
			if (ImGui::InputInt(name, &editedValue))
				value = std::uint32_t(std::clamp(editedValue, minimum, maximum));
		};
		const auto choice = [](const char *name, std::string &value, std::initializer_list<const char *> options) {
			if (ImGui::BeginCombo(name, value.c_str()))
			{
				for (const char *option : options)
				{
					if (ImGui::Selectable(option, value == option))
						value = option;
				}
				ImGui::EndCombo();
			}
		};

		for (std::size_t index = 0; index < hits.size();)
		{
			auto &hit = hits[index];
			ImGui::PushID(int(index));
			bool removed = false;
			if (ImGui::TreeNode(hit.strHitId.c_str()))
			{
				// 각 spawn을 기준으로 예약 시각 또는 접촉 구간을 편집한다.
				choice("Trigger", hit.strTrigger, {"TIMED", "CONTACT"});
				integer("Start / pulse (ms)", hit.iAtMs, 0, 600000);
				if (hit.strTrigger == "CONTACT")
					integer("Contact end (ms)", hit.iEndMs, 1, 600000);
				else
					hit.iEndMs = 0u;
				integer("Maximum hits per player", hit.iRepeatCount, 1, 64);
				integer("Repeat interval (ms)", hit.iRepeatIntervalMs, 0, 600000);

				// 도형마다 필요한 입력만 표시하며 사용하지 않는 편집값을 임의로 재설정하지 않는다.
				choice("Shape", hit.strShape, {"CIRCLE", "RING", "BOX", "CONE"});
				if (hit.strShape == "CIRCLE" || hit.strShape == "RING")
					ImGui::InputDouble("Radius (m)", &hit.fRadiusM, .1, 1, "%.3f");
				if (hit.strShape == "RING" || hit.strShape == "CONE")
					ImGui::InputDouble("Inner radius (m)", &hit.fInnerRadiusM, .1, 1, "%.3f");
				if (hit.strShape == "BOX" || hit.strShape == "CONE")
					ImGui::InputDouble("Length (m)", &hit.fLengthM, .1, 1, "%.3f");
				if (hit.strShape == "BOX")
					ImGui::InputDouble("Half width (m)", &hit.fHalfWidthM, .1, 1, "%.3f");
				if (hit.strShape == "CONE")
					ImGui::InputDouble("Angle (deg)", &hit.fAngleDegrees, 1, 10, "%.2f");
				ImGui::InputDouble("Forward offset (m)", &hit.fOffsetForwardM, .1, 1, "%.3f");
				ImGui::InputDouble("Right offset (m)", &hit.fOffsetRightM, .1, 1, "%.3f");
				ImGui::InputDouble("Yaw offset (deg)", &hit.fYawOffsetDegrees, 1, 10, "%.2f");

				// 피해 종류를 바꾸면 해당 정책이 허용하지 않는 percent/profile 값을 기존 규칙대로 비운다.
				choice("Damage", hit.strDamageKind, {"PROFILE", "MAX_HP_PERCENT", "INSTANT_DEATH"});
				if (hit.strDamageKind == "MAX_HP_PERCENT")
					integer("Maximum HP damage (%)", hit.iDamagePercent, 1, 100);
				else
					hit.iDamagePercent = 0u;
				if (hit.strDamageKind == "PROFILE")
				{
					char value[129]{};
					std::copy_n(hit.strDamageProfileId.data(),
						(std::min)(hit.strDamageProfileId.size(), std::size_t(128)), value);
					if (ImGui::InputText("Damage profile ID", value, sizeof(value)))
						hit.strDamageProfileId = value;
				}
				else
					hit.strDamageProfileId.clear();
				bool rise = hit.fRiseHeightM > 0.0;
				if (ImGui::Checkbox("Rise and fall", &rise))
				{
					hit.fRiseHeightM = rise ? 3.0 : 0.0;
					hit.iPushMs = rise ? 1200u : 0u;
				}
				if (rise)
				{
					ImGui::InputDouble("Rise height (m)", &hit.fRiseHeightM, .1, 1., "%.3f");
					integer("Flight time (ms)", hit.iPushMs, 100, 5000);
				}
				if (ImGui::SmallButton("Remove attack hit"))
					removed = true;
				ImGui::TreePop();
			}
			ImGui::PopID();
			// erase로 다음 행이 같은 index로 이동한다. 제거한 프레임에는 증가시키지 않아 행을 건너뛰지 않는다.
			if (removed)
				hits.erase(hits.begin() + index);
			else
				++index;
		}
		if (hits.size() < 32u && ImGui::Button("Add attack hit"))
		{
			LostArk::Shared::ATTACK_HIT_TEMPLATE hit;
			std::uint32_t ordinal = 1u;
			// 저장 ID는 vector 위치가 아니다. 현재 목록에서 사용하지 않는 ID를 찾아 새 행에 부여한다.
			do
			{
				hit.strHitId = "attack.hit." + std::to_string(ordinal++);
			} while (std::any_of(hits.begin(), hits.end(), [&](const auto &row) {
				return row.strHitId == hit.strHitId;
			}));
			hits.push_back(std::move(hit));
		}
		if (!LostArk::Shared::Validate_AttackHitTemplates(hits))
			ImGui::TextWrapped(
				"Attack values are invalid; correct the shape, damage policy or bounded hit window before applying.");
		ImGui::TreePop();
	}
	ImGui::PopID();
}
} // 이 파일의 보조 구현 범위 끝

// Logic 리소스 패널의 Create가 부른다. 새 stable ID와 정의를 후보 문서에서 만들고 검증·commit 성공 뒤에만 선택과 출력 ID를 바꾼다.
// ordinal 소진이나 문서 검증 실패는 현재 draft와 기존 선택을 보존한다. 여기서는 timeline occurrence를 만들지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Create_Logic(const std::string_view displayName,
	const std::string_view logicType, std::string &outLogicId, std::string &outStatus)
{
	if (!m_bHasDraft || m_Draft.iNextLogicOrdinal >= 1000000u)
	{
		outStatus = m_strStatus = "Logic creation requires a loaded draft and an available ordinal.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION logic;
	logic.strLogicId = "koukusaydon.g1.logic." + std::to_string(candidate.iNextLogicOrdinal++);
	logic.strDisplayName = std::string(displayName);
	logic.strLogicType = std::string(logicType);
	const std::string logicId = logic.strLogicId;
	candidate.Logics.push_back(std::move(logic));
	if (!Commit_Candidate(std::move(candidate),
			"Created Logic " + logicId + ". Select it and press Append Logic at Cursor.", outStatus))
	{
		return false;
	}
	m_strSelectedLogicId = logicId;
	outLogicId = logicId;
	return true;
}

// Logic 리소스 패널의 Delete가 부른다. 정의를 지우기 전에 정상·격리 Pattern의 직접 참조와 Outcome 참조를 함께 센다.
// 참조가 있거나 원본을 읽지 못해 참조 여부를 확정할 수 없으면 보존하고, 참조 없는 후보만 공통 commit에 넘긴다.
bool_t Client::CKoukuSaydonActionWorkbench::Delete_Logic(const std::string_view logicId, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	const auto found = std::find_if(candidate.Logics.begin(), candidate.Logics.end(),
		[logicId](const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &logic) {
			return logic.strLogicId == logicId;
		});
	if (found == candidate.Logics.end())
	{
		outStatus = m_strStatus = "Logic delete target is absent.";
		return false;
	}
	bool_t unresolvedReferences = false;
	const std::size_t references = Count_LogicReferences(candidate, logicId, &unresolvedReferences);
	if (unresolvedReferences)
	{
		outStatus = m_strStatus = "Repair the invalid Pattern Logic references before deleting a Logic.";
		return false;
	}
	if (0u != references)
	{
		outStatus = m_strStatus =
			"Delete its " + std::to_string(references) + " Logic box(es) first; a referenced Logic is preserved.";
		return false;
	}
	candidate.Logics.erase(found);
	if (!Commit_Candidate(std::move(candidate), "Deleted Logic.", outStatus))
		return false;
	if (m_strSelectedLogicId == logicId)
		m_strSelectedLogicId.clear();
	return true;
}

// 리소스 Append와 기존 typed 편집 경로가 재사용하는 occurrence 생성이다. 정의 ID를 Pattern의 새 stable box ID와 시간창에 연결한다.
// 시간은 Pattern 기준 ms이며 정상 수명 안으로 제한한다. 전체 후보가 검증된 뒤에만 새 box 선택과 상세 입력을 동기화한다.
bool_t Client::CKoukuSaydonActionWorkbench::Append_LogicBox(const std::string_view patternId,
	const std::string_view logicId, const std::uint32_t startMs, const std::uint32_t durationMs,
	std::string &outOccurrenceId, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern || !pattern->strLoadError.empty())
	{
		outStatus = m_strStatus = "Append Logic requires an editable Pattern selected in Patterns.";
		return false;
	}
	if (nullptr == Find_Logic(candidate, logicId))
	{
		outStatus = m_strStatus = "Append Logic target definition is absent.";
		return false;
	}
	if (pattern->iNextLogicOccurrenceOrdinal >= 1000000u)
	{
		outStatus = m_strStatus = "Logic box stable ID ordinals are exhausted.";
		return false;
	}
	const std::uint32_t lifetimeMs = Pattern_DurationMs(*pattern);
	if (0u == lifetimeMs)
	{
		outStatus = m_strStatus = "Append Logic requires a Pattern lifetime. Append an animation action first.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE box;
	box.strOccurrenceId = pattern->strPatternId + ".logic." + std::to_string(pattern->iNextLogicOccurrenceOrdinal++);
	box.strLogicId = std::string(logicId);
	box.iStartMs = (std::min)(startMs, lifetimeMs - 1u);
	box.iDurationMs = std::clamp(durationMs, 1u, lifetimeMs - box.iStartMs);
	const std::string occurrenceId = box.strOccurrenceId;
	// 종류를 정한 판정은 PRODUCT Pattern에 추가할 수 있다. 이름뿐인 DURATION은 실행할 판정이 없으므로 아래에서 DRAFT로 돌린다.
	const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION *const owner = Find_Logic(candidate, logicId);
	// 네 방향 child로 참조되는 facing Trigger의 기존 34ms 상한을 적용한다. 일반 Pattern의 입력 수명은 여기서 줄이지 않는다.
	if (owner && owner->strLogicType == "TRIGGER" && owner->strTriggerKind == "BOSS_TRACK_TARGET")
	{
		const auto targetsChild = [&](const auto &definition) {
			return std::find(definition.DirectionPatternIds.begin(), definition.DirectionPatternIds.end(), patternId) !=
				definition.DirectionPatternIds.end();
		};
		if (std::any_of(candidate.Summons.begin(), candidate.Summons.end(), targetsChild) ||
			std::any_of(candidate.Logics.begin(), candidate.Logics.end(), targetsChild))
			box.iDurationMs = (std::min)(box.iDurationMs, 34u);
	}
	if (owner && owner->strTriggerKind == "ROOM_PLAYER_ARRIVAL")
	{
		if (!m_bSequenceWorkspace)
		{
			outStatus = m_strStatus = "Player arrival belongs to the Sequence workspace.";
			return false;
		}
		KOUKU_SAYDON_ROOM_PLAYER_ARRIVAL arrival;
		if (m_WorldPlacementResolver)
		{
			KOUKU_SAYDON_WORLD_PLACEMENT placement;
			if (!m_WorldPlacementResolver(placement, outStatus))
			{
				m_strStatus = outStatus;
				return false;
			}
			arrival.Position = placement.Position;
		}
		// 슬롯 번호는 저장된 arrival occurrence에서 모은 뒤 첫 빈 칸을 찾는다. 다 찼으면 후보를 버린다.
		std::array<bool, 4> occupiedSlots{};
		for (const auto &existing : pattern->LogicOccurrences)
			if (existing.RoomPlayerArrival && existing.RoomPlayerArrival->iPlayerSlot < occupiedSlots.size())
				occupiedSlots[existing.RoomPlayerArrival->iPlayerSlot] = true;
		while (arrival.iPlayerSlot < occupiedSlots.size() && occupiedSlots[arrival.iPlayerSlot])
			++arrival.iPlayerSlot;
		if (arrival.iPlayerSlot >= 4u)
		{
			outStatus = m_strStatus = "This Sequence already has four player arrival slots.";
			return false;
		}
		box.RoomPlayerArrival = arrival;
	}
	const bool_t untypedDuration =
		nullptr != owner && "DURATION" == owner->strLogicType && owner->strJudgementKind.empty();
	const bool_t returnedToDraft = untypedDuration && "PRODUCT" == pattern->strAuthoringStatus;
	pattern->LogicOccurrences.push_back(std::move(box));
	if (returnedToDraft)
		Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate),
			returnedToDraft
				? "Appended Logic box and returned the Pattern to DRAFT; give the Logic a judgement kind in Resources > Logic."
				: "Appended Logic box at the cursor. Drag its edges on the Logic lane or edit Lifetime in Box Detail.",
			outStatus))
	{
		return false;
	}
	m_strSelectedPatternId = std::string(patternId);
	m_TimelineSelectedStageIds.clear();
	m_TimelineSelectedOccurrenceIds.clear();
	m_strSelectedStageId.clear();
	m_strSelectedOccurrenceId.clear();
	m_strSelectedLogicOccurrenceId = occurrenceId;
	m_strSelectedSummonOccurrenceId.clear();
	m_strSelectedWorldOccurrenceId.clear();
	m_strSelectedSceneProfileOccurrenceId.clear();
	m_strSelectedPresentationOccurrenceId.clear();
	m_strSelectedPatternOccurrenceId.clear();
	m_strTimelineSelectionPatternId = m_strSelectedPatternId;
	Synchronize_EditorFields();
	outOccurrenceId = occurrenceId;
	return true;
}

// 타임라인 이동·trim과 상세 Apply Window가 공유한다. 64-bit 끝 시각으로 상한을 확인한 뒤 Logic과 연결 Collider를 같은 후보에서 이동한다.
// Bingo board control의 기존 수명 확장 예외를 보존하며 실패하면 현재 문서의 어느 시간창도 교체하지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Set_LogicBoxWindow(const std::string_view patternId,
	const std::string_view occurrenceId, const std::uint32_t startMs, const std::uint32_t durationMs,
	std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE *const box =
		nullptr == pattern ? nullptr : Find_LogicBox(*pattern, occurrenceId);
	if (nullptr == box)
	{
		outStatus = m_strStatus = "Logic box window target is absent.";
		return false;
	}
	const std::uint64_t lifetimeMs = Pattern_DurationMs(*pattern);
	const std::uint64_t endMs = static_cast<std::uint64_t>(startMs) + durationMs;
	if (0u == durationMs || endMs > MAX_EDITOR_TIME_MS ||
		(endMs > lifetimeMs && !Is_BingoBoardControl(candidate, *pattern)))
	{
		outStatus = m_strStatus =
			"Logic box window must stay inside the Pattern lifetime of " + std::to_string(lifetimeMs) + " ms.";
		return false;
	}
	if (box->iStartMs == startMs && box->iDurationMs == durationMs)
	{
		outStatus = m_strStatus = "Logic box window is unchanged.";
		return true;
	}
	box->iStartMs = startMs;
	box->iDurationMs = durationMs;
	if (endMs > lifetimeMs && !Extend_PatternLifetimeForAuthoredLanes(*pattern, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	// Collider의 접촉 활성 시간과 그 결과를 실행할 Logic 창은 하나의 편집 transaction으로 함께 바뀐다.
	for (auto &collider : pattern->PresentationOccurrences)
		if (collider.strLogicOccurrenceId == occurrenceId)
		{
			collider.iStartMs = startMs;
			collider.iDurationMs = durationMs;
		}
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate), "Updated Logic window and its linked Collider boxes.", outStatus);
}

// Outcome 상세의 Hold 선택이 ENTER_AREA Trigger를 ATTACHMENT_HOLD occurrence에 연결한다.
// ID만 후보에 기록하고 대상의 의미·시간 관계는 공통 문서 검증에 맡긴다. 실패하면 기존 연결을 보존한다.
bool_t Client::CKoukuSaydonActionWorkbench::Set_LogicBoxHold(const std::string_view patternId,
	const std::string_view occurrenceId, const std::string_view holdOccurrenceId, std::string &outStatus)
{
	auto candidate = m_Draft;
	auto *pattern = Find_Pattern(candidate, patternId);
	auto *box = pattern ? Find_LogicBox(*pattern, occurrenceId) : nullptr;
	const auto *logic = box ? Find_Logic(candidate, box->strLogicId) : nullptr;
	if (!logic || logic->strTriggerKind != "ENTER_AREA")
	{
		outStatus = m_strStatus = "Select an ENTER_AREA Trigger window to connect its Hold.";
		return false;
	}
	box->strHoldLogicOccurrenceId = holdOccurrenceId;
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate), "Updated capture Hold connection. Press Save.", outStatus);
}

// Logic 상세의 Delete를 전체 타임라인 삭제 경계로 보낸다. 같은 소유 관계의 Collider·Hold 등 연결 정리 규칙을 별도로 복제하지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Delete_LogicBox(
	const std::string_view patternId, const std::string_view occurrenceId, std::string &outStatus)
{
	return Delete_TimelineSelection(patternId, {}, {std::string(occurrenceId)}, outStatus);
}

// Outcome 상세가 편집한 순서 있는 RESULT ID 목록을 적용한다. owner 종류·허용 slot·네 개 상한·각 RESULT 정의 존재를 먼저 확인한다.
// 선택 순서를 그대로 후보에 저장하며, 같은 값은 no-op으로 끝내고 검증 실패는 기존 연결을 남긴다.
bool_t Client::CKoukuSaydonActionWorkbench::Set_LogicBoxOutcomes(const std::string_view patternId,
	const std::string_view occurrenceId, const KOUKU_SAYDON_OUTCOME_SLOT slot,
	const std::vector<std::string> &resultLogicIds, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE *const box =
		nullptr == pattern ? nullptr : Find_LogicBox(*pattern, occurrenceId);
	if (nullptr == box)
	{
		outStatus = m_strStatus = "Logic box outcome target is absent.";
		return false;
	}
	const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION *const owner = Find_Logic(candidate, box->strLogicId);
	if (nullptr == owner || !Kouku_LogicOwnsOutcomes(*owner))
	{
		outStatus = m_strStatus = "Only a Duration or Collider Trigger window owns result slots.";
		return false;
	}
	if (!Kouku_LogicOutcomeKind(*owner).empty() && !resultLogicIds.empty() &&
		!Kouku_IsOutcomeSlotAllowed(Kouku_LogicOutcomeKind(*owner), slot, owner->bGazeDuringWindow))
	{
		outStatus = m_strStatus =
			std::string(Outcome_SlotLabel(slot)) + " is not an outcome of " + owner->strJudgementKind + ".";
		return false;
	}
	if (resultLogicIds.size() > KOUKU_SAYDON_MAX_OUTCOMES_PER_SLOT)
	{
		outStatus = m_strStatus = "An outcome slot holds at most four RESULT Logics.";
		return false;
	}
	for (const std::string &resultLogicId : resultLogicIds)
	{
		const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION *const result = Find_Logic(candidate, resultLogicId);
		if (nullptr == result || "RESULT" != result->strLogicType)
		{
			outStatus = m_strStatus = "An outcome must name an existing RESULT Logic.";
			return false;
		}
	}
	std::vector<std::string> &current = box->Outcomes(slot);
	if (current == resultLogicIds)
	{
		outStatus = m_strStatus = "Logic box outcome is unchanged.";
		return true;
	}
	current = resultLogicIds;
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate),
		std::string("Wired the ") + Outcome_SlotLabel(slot) + " outcomes. Press Save to keep them.", outStatus);
}

// 값 패널의 Apply가 공유 Logic 정의를 수정한다. 입력 values에서 값만 받으며 stable ID·표시 이름·Logic 타입은 기존 정의 것을 보존한다.
// 이 정의를 직접 또는 Outcome으로 참조하는 Pattern의 전용 연결과 DRAFT 상태를 같은 후보에서 정리한 뒤 검증·commit한다.
bool_t Client::CKoukuSaydonActionWorkbench::Set_LogicDefinitionValues(
	const std::string_view logicId, const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &values, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	const auto found = std::find_if(candidate.Logics.begin(), candidate.Logics.end(),
		[logicId](const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &logic) {
			return logic.strLogicId == logicId;
		});
	if (found == candidate.Logics.end())
	{
		outStatus = m_strStatus = "Logic value target is absent.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION updated = values;
	updated.strLogicId = found->strLogicId;
	updated.strDisplayName = found->strDisplayName;
	updated.strLogicType = found->strLogicType;
	if (updated == *found)
	{
		outStatus = m_strStatus = "Logic values are unchanged.";
		return true;
	}
	*found = std::move(updated);
	// 공유 정의의 종류가 바뀌면 이를 사용하는 occurrence 연결이 더 이상 맞지 않을 수 있다.
	// 여기서 기존 종류의 전용 연결을 정리하고 사용 Pattern을 DRAFT로 표시한다. 게시 가능 판정은 저장 후 publisher가 한다.
	for (auto &pattern : candidate.Patterns)
	{
		if (!pattern.strLoadError.empty())
			continue;
		const bool usesDefinition =
			std::any_of(pattern.LogicOccurrences.begin(), pattern.LogicOccurrences.end(), [&](const auto &box) {
				if (box.strLogicId == logicId)
					return true;
				for (const auto slot : {KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS, KOUKU_SAYDON_OUTCOME_SLOT::FAIL,
						 KOUKU_SAYDON_OUTCOME_SLOT::TIMEOUT})
					if (std::find(box.Outcomes(slot).begin(), box.Outcomes(slot).end(), logicId) !=
						box.Outcomes(slot).end())
						return true;
				return false;
			});
		if (usesDefinition)
		{
			// 정의 종류가 바뀔 때 occurrence가 소유하던 arrival·Hold·Outcome·Collider 연결을 현재 종류에 맞춘다.
			for (auto &box : pattern.LogicOccurrences)
			{
				if (box.strLogicId == logicId)
				{
					if (found->strTriggerKind != "ROOM_PLAYER_ARRIVAL")
						box.RoomPlayerArrival.reset();
					else if (!box.RoomPlayerArrival)
						box.RoomPlayerArrival = KOUKU_SAYDON_ROOM_PLAYER_ARRIVAL{};
				}
				if (box.strLogicId == logicId && found->strTriggerKind != "ENTER_AREA")
					box.strHoldLogicOccurrenceId.clear();
				if (const auto *hold = Find_LogicBox(pattern, box.strHoldLogicOccurrenceId);
					hold && hold->strLogicId == logicId && found->strJudgementKind != "ATTACHMENT_HOLD")
					box.strHoldLogicOccurrenceId.clear();
				if (box.strLogicId == logicId && found->strJudgementKind == "INVULNERABILITY_ZONE")
				{
					box.OnSuccessLogicIds.clear();
					box.OnFailLogicIds.clear();
					box.OnTimeoutLogicIds.clear();
				}
				if (box.strLogicId == logicId &&
					(found->strJudgementKind == "ATTACHMENT_HOLD" ||
						found->strJudgementKind == "SHOWTIME_PLAYER_TARGETS" ||
						found->strJudgementKind == "BOSS_TRACK_TARGET" ||
						found->strJudgementKind == "CROSS_DIRECTION_CLONES" ||
						found->strJudgementKind == "PURSUIT_PROJECTILES" || found->strJudgementKind == "BINGO_BOARD"))
				{
					box.OnSuccessLogicIds.clear();
					box.OnFailLogicIds.clear();
					box.OnTimeoutLogicIds.clear();
					for (auto &collider : pattern.PresentationOccurrences)
						if (collider.strLogicOccurrenceId == box.strOccurrenceId)
							collider.strLogicOccurrenceId.clear();
				}
			}
			Mark_Draft(candidate, pattern);
		}
	}
	return Commit_Candidate(std::move(candidate),
		m_bSequenceWorkspace ? "Applied the Logic values. Save stores them in the Sequence workspace."
							 : "Applied the Logic values. Save stores them; Publish All Patterns updates the F1 tree.",
		outStatus);
}

// Duration와 Trigger가 공통으로 호출하는 추적 카드 입력이다. 카드 Effect와 문양의 같은 index를 함께 추가·제거해 대응을 유지한다.
// 호출자가 소유한 미적용 draft만 바꾸며, 공격 정의의 최종 검증과 문서 교체는 Apply Values에 남긴다.
void Client::CKoukuSaydonActionWorkbench::Render_LogicPursuitValues(KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &draft)
{
	const auto choose = [&](const char *label, std::string &id) {
		const auto *selected = Find_PresentationResource(m_Draft, id);
		if (ImGui::BeginCombo(label, selected ? selected->strDisplayName.c_str() : "(choose Effect)"))
		{
			for (const auto &resource : m_Draft.PresentationResources)
				if (resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT &&
					resource.strResourceKind == "V1_EFFECT" &&
					ImGui::Selectable((resource.strDisplayName + "##" + resource.strResourceId).c_str(),
						resource.strResourceId == id))
					id = resource.strResourceId;
			ImGui::EndCombo();
		}
	};
	bool matchSymbol = !draft.PursuitCardSymbols.empty();
	if (ImGui::Checkbox("Matching card symbol takes no damage", &matchSymbol))
	{
		if (matchSymbol)
			draft.PursuitCardSymbols.assign(draft.PursuitVisualIds.size(), "HEART");
		else
			draft.PursuitCardSymbols.clear();
	}
	if (matchSymbol)
		draft.PursuitCardSymbols.resize(draft.PursuitVisualIds.size(), "HEART");
	for (std::size_t i = 0; i < draft.PursuitVisualIds.size();)
	{
		ImGui::PushID(static_cast<int>(i));
		choose("Card Effect", draft.PursuitVisualIds[i]);
		if (matchSymbol && ImGui::BeginCombo("Card symbol", draft.PursuitCardSymbols[i].c_str()))
		{
			for (const char *symbol : {"HEART", "SPADE", "CLUB", "DIAMOND"})
				if (ImGui::Selectable(symbol, draft.PursuitCardSymbols[i] == symbol))
					draft.PursuitCardSymbols[i] = symbol;
			ImGui::EndCombo();
		}
		ImGui::SameLine();
		const bool remove = ImGui::SmallButton("Remove");
		ImGui::PopID();
		if (remove)
		{
			draft.PursuitVisualIds.erase(draft.PursuitVisualIds.begin() + i);
			if (matchSymbol)
				draft.PursuitCardSymbols.erase(draft.PursuitCardSymbols.begin() + i);
		}
		else
			++i;
	}
	if (draft.PursuitVisualIds.size() < 4u && ImGui::Button("Add card Effect"))
	{
		draft.PursuitVisualIds.emplace_back();
		if (matchSymbol)
			draft.PursuitCardSymbols.emplace_back("HEART");
	}
	if (matchSymbol)
		ImGui::TextWrapped(
			"Each card compares its symbol to each player. Matching symbols deal zero damage regardless of color; other symbols use the authored attack damage.");
	choose("Contact explosion", draft.strContactVisualId);
	ImGui::Checkbox("Follow selected player", &draft.bPursuitHoming);
	const auto number = [](const char *label, double &value, float low, float high) {
		float edit = static_cast<float>(value);
		if (ImGui::InputFloat(label, &edit, .1f, 1.f))
			value = std::clamp(edit, low, high);
	};
	number("Travel speed (m/s)", draft.fPursuitSpeedMps, .01f, 100.f);
	number("Travel limit (m, 0 = none)", draft.fPursuitMaxDistanceM, 0.f, 1000.f);
	number("Contact radius (m)", draft.fContactRadiusM, .01f, 10.f);
	number("Spawn radius (m)", draft.fSpawnRadiusM, 0.f, 100.f);
	const auto integer = [](const char *label, std::uint32_t &value, int maximum) {
		int edit = static_cast<int>(value);
		if (ImGui::InputInt(label, &edit))
			value = static_cast<std::uint32_t>(std::clamp(edit, 0, maximum));
	};
	integer("Lifetime (ms, 0 = until contact)", draft.iPursuitLifetimeMs, 600000);
	if (draft.strLogicType == "DURATION")
		integer("Spawn interval (ms, 0 = once)", draft.iSpawnIntervalMs, 600000);
	else
		ImGui::TextWrapped("This Trigger emits one wave at the box Start ms. Box length does not end the cards.");
	integer("Cards per wave (0 = all types)", draft.iCountPerWave, 16);
	Render_AttackHitTemplates("Projectile attack hits", draft.ProjectileHits);
	ImGui::TextWrapped(
		"The Server selects targets, moves cards and detects contact. Card animation loops until the object ends. Zero lifetime requires one homing volley without a travel limit; Stop clears it.");
}

// 룰렛 판정에 연결할 Collider region ID를 현재 Pattern에서 찾는다. 중복 region은 한 번 표시하고 사라진 참조는 경고만 한다.
// draft의 선택 목록만 편집하므로 미완성 매핑도 Apply 전까지 유지된다.
void Client::CKoukuSaydonActionWorkbench::Render_LogicRouletteRegionValues(
	KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &draft)
{
	ImGui::TextWrapped(
		"Select eight authored Roulette Card Regions. Each region has a symbol and a RED/BLACK color in its Collider Box Detail.");
	const auto *pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	std::unordered_set<std::string> shown;
	if (nullptr != pattern)
		for (const auto &region : pattern->PresentationOccurrences)
		{
			const auto *resource = Find_PresentationResource(m_Draft, region.strResourceId);
			if (nullptr == resource || resource->strColliderKind != "ROULETTE_CARD_REGION" ||
				region.strRegionId.empty() || !shown.insert(region.strRegionId).second)
				continue;
			bool selected =
				std::find(draft.RegionIds.begin(), draft.RegionIds.end(), region.strRegionId) != draft.RegionIds.end();
			const std::string label = region.strRegionId + " | " + region.strCardSymbol + " / " + region.strCardColor;
			if (ImGui::Checkbox(label.c_str(), &selected))
			{
				if (selected && draft.RegionIds.size() < 8u)
					draft.RegionIds.push_back(region.strRegionId);
				else if (!selected)
					std::erase(draft.RegionIds, region.strRegionId);
			}
		}
	for (const auto &id : draft.RegionIds)
		if (!shown.contains(id))
			ImGui::TextColored(ImVec4(1.f, 0.5f, 0.3f, 1.f), "Unavailable region: %s", id.c_str());
	ImGui::Text("Mapped regions: %zu / 8", draft.RegionIds.size());
	if (ImGui::Button("Clear region references"))
		draft.RegionIds.clear();
	if (draft.iSectorCount != 0u || !draft.SectorSymbols.empty())
		ImGui::TextDisabled(
			"Legacy sector data is retained for disabled/DRAFT editing only; it does not define the new eight regions.");
	ImGui::TextWrapped(
		"Only the position at the window end is judged: matching card = Success, another card = Fail, outside all regions = Timeout.");
}

// 쇼타임의 순서 있는 volley set과 각 set의 공격 정의를 편집한다. set 삭제·순서 변경 시 병렬 Hits 목록도 기존 조건대로 따라간다.
// pattern은 상위 쇼타임 패널이 현재 m_Draft에서 잠시 차용하며 이 함수는 문서를 commit하거나 참조를 보관하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Render_LogicShowtimeVolleyValues(
	KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &draft, const KOUKU_SAYDON_COMPOSITION_PATTERN *pattern)
{
	ImGui::SeparatorText("Arena volley sequence");
	ImGui::TextWrapped(
		"Cycles one complete set per interval at a Server-selected walkable arena position. MAP effects move with that position; following BOSS effects retain the boss anchor.");
	for (std::size_t set = 0; set < draft.RandomVolleyOccurrenceSets.size();)
	{
		ImGui::PushID(static_cast<int>(set));
		auto &ids = draft.RandomVolleyOccurrenceSets[set];
		const auto label = "Set " + std::to_string(set + 1u) + " (" + std::to_string(ids.size()) + " rows)";
		if (ImGui::BeginCombo("Effect / Sound occurrences", label.c_str()))
		{
			if (pattern)
				for (const auto &row : pattern->PresentationOccurrences)
				{
					const auto *resource = Find_PresentationResource(m_Draft, row.strResourceId);
					if (!resource ||
						(resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT &&
							!(resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::SOUND && row.strAnchorKind == "MAP" &&
								!row.bFollowBoss)) ||
						!row.strBone.empty() || row.strBoneTarget != "BODY" || !row.strWorldId.empty() ||
						!row.strWorldOccurrenceId.empty() || row.iWorldEmissionIndex ||
						!row.strLogicOccurrenceId.empty() ||
						!((row.strAnchorKind == "MAP" && !row.bFollowBoss) ||
							(row.strAnchorKind == "BOSS" && row.bFollowBoss)))
						continue;
					const auto selected = std::find(ids.begin(), ids.end(), row.strOccurrenceId);
					const bool wasSelected = selected != ids.end();
					const auto choice = resource->strDisplayName + " | " + std::to_string(row.iStartMs) + " ms | " +
						row.strAnchorKind + "##" + row.strOccurrenceId;
					if (ImGui::Selectable(choice.c_str(), wasSelected, ImGuiSelectableFlags_DontClosePopups))
					{
						if (wasSelected)
							ids.erase(selected);
						else if (ids.size() < 64u)
							ids.push_back(row.strOccurrenceId);
					}
				}
			ImGui::EndCombo();
		}
		std::vector<LostArk::Shared::ATTACK_HIT_TEMPLATE> setHits = set < draft.RandomVolleyHits.size()
			? draft.RandomVolleyHits[set]
			: std::vector<LostArk::Shared::ATTACK_HIT_TEMPLATE>{};
		const auto beforeHits = setHits;
		Render_AttackHitTemplates("Volley attack hits", setHits);
		if (setHits != beforeHits)
		{
			draft.RandomVolleyHits.resize(draft.RandomVolleyOccurrenceSets.size());
			draft.RandomVolleyHits[set] = std::move(setHits);
		}
		bool removed = false;
		if (ImGui::SmallButton("Remove set"))
		{
			draft.RandomVolleyOccurrenceSets.erase(draft.RandomVolleyOccurrenceSets.begin() + set);
			if (set < draft.RandomVolleyHits.size())
				draft.RandomVolleyHits.erase(draft.RandomVolleyHits.begin() + set);
			removed = true;
		}
		if (!removed && set > 0u)
		{
			ImGui::SameLine();
			if (ImGui::SmallButton("Earlier"))
			{
				std::swap(draft.RandomVolleyOccurrenceSets[set], draft.RandomVolleyOccurrenceSets[set - 1u]);
				if (draft.RandomVolleyHits.size() == draft.RandomVolleyOccurrenceSets.size())
					std::swap(draft.RandomVolleyHits[set], draft.RandomVolleyHits[set - 1u]);
			}
		}
		if (!removed && set + 1u < draft.RandomVolleyOccurrenceSets.size())
		{
			ImGui::SameLine();
			if (ImGui::SmallButton("Later"))
			{
				std::swap(draft.RandomVolleyOccurrenceSets[set], draft.RandomVolleyOccurrenceSets[set + 1u]);
				if (draft.RandomVolleyHits.size() == draft.RandomVolleyOccurrenceSets.size())
					std::swap(draft.RandomVolleyHits[set], draft.RandomVolleyHits[set + 1u]);
			}
		}
		ImGui::PopID();
		if (!removed)
			++set;
	}
	if (draft.RandomVolleyOccurrenceSets.size() < 32u && ImGui::Button("Add arena volley set"))
	{
		draft.RandomVolleyOccurrenceSets.emplace_back();
		if (!draft.RandomVolleyHits.empty())
			draft.RandomVolleyHits.emplace_back();
		if (!draft.iRandomSpawnIntervalMs)
			draft.iRandomSpawnIntervalMs = 500u;
	}
	if (!draft.RandomVolleyOccurrenceSets.empty())
	{
		int randomInterval = static_cast<int>(draft.iRandomSpawnIntervalMs);
		if (ImGui::InputInt("Arena spawn interval (ms)", &randomInterval, 100, 1000))
			draft.iRandomSpawnIntervalMs = static_cast<std::uint32_t>(std::clamp(randomInterval, 1, 600000));
		ImGui::InputDouble("Arena search radius (m)", &draft.fRandomArenaRadiusM, 1, 10, "%.3f");
		ImGui::InputDouble("Arena height tolerance (m)", &draft.fRandomArenaHeightToleranceM, .1, 1, "%.3f");
		bool aroundBoss = draft.strRandomAnchorKind == "BOSS";
		if (ImGui::Checkbox("Follow current boss position", &aroundBoss))
			draft.strRandomAnchorKind = aroundBoss ? "BOSS" : "BOSS_SPAWN";
		ImGui::InputDouble("Random scale minimum", &draft.fRandomScaleMin, .1, 1, "%.3f");
		ImGui::InputDouble("Random scale maximum", &draft.fRandomScaleMax, .1, 1, "%.3f");
		ImGui::TextDisabled("Each set needs a MAP row; radius (0,1000] m and height tolerance (0,10] m are required.");
	}
	else
	{
		draft.iRandomSpawnIntervalMs = 0u;
		draft.fRandomArenaRadiusM = 0.0;
		draft.fRandomArenaHeightToleranceM = 0.0;
		draft.strRandomAnchorKind = "BOSS_SPAWN";
		draft.fRandomScaleMin = draft.fRandomScaleMax = 1.0;
	}
}

// 쇼타임 Duration의 고정 공격 그룹·추적 Effect와 반복 간격을 설정한다. Collider 상세에서는 그 Collider의 Pattern을 선택한다.
// 선택지는 현재 문서에서 읽고 변경은 패널 draft에만 남겨, 세 종류 공격 목록을 한 번의 Apply로 검증하게 한다.
void Client::CKoukuSaydonActionWorkbench::Render_LogicShowtimeValues(
	KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &draft, const std::string_view colliderPatternId)
{
	const auto *pattern = Find_Pattern(m_Draft, colliderPatternId.empty() ? m_strSelectedPatternId : colliderPatternId);
	if (ImGui::BeginCombo("Fixed drop group",
			draft.strFixedSelectionGroupId.empty() ? "(none)" : draft.strFixedSelectionGroupId.c_str()))
	{
		if (ImGui::Selectable("(none)", draft.strFixedSelectionGroupId.empty()))
			draft.strFixedSelectionGroupId.clear();
		std::unordered_set<std::string> shown;
		if (pattern)
			for (const auto &box : pattern->PresentationOccurrences)
			{
				const auto *resource = Find_PresentationResource(m_Draft, box.strResourceId);
				if (!resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
					box.strSelectionGroupId.empty() || !shown.insert(box.strSelectionGroupId).second)
					continue;
				const auto count = std::count_if(pattern->PresentationOccurrences.begin(),
					pattern->PresentationOccurrences.end(), [&](const auto &member) {
						return member.strSelectionGroupId == box.strSelectionGroupId;
					});
				const std::string label =
					resource->strDisplayName + " | " + std::to_string(count) + " effects##" + box.strSelectionGroupId;
				if (ImGui::Selectable(label.c_str(), draft.strFixedSelectionGroupId == box.strSelectionGroupId))
					draft.strFixedSelectionGroupId = box.strSelectionGroupId;
			}
		ImGui::EndCombo();
	}
	if (ImGui::BeginCombo("Tracking target",
			draft.strTrackingPresentationOccurrenceId.empty() ? "(none)"
															  : draft.strTrackingPresentationOccurrenceId.c_str()))
	{
		if (ImGui::Selectable("(none)", draft.strTrackingPresentationOccurrenceId.empty()))
			draft.strTrackingPresentationOccurrenceId.clear();
		if (pattern)
			for (const auto &box : pattern->PresentationOccurrences)
			{
				const auto *resource = Find_PresentationResource(m_Draft, box.strResourceId);
				if (!resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)
					continue;
				const std::string label =
					resource->strDisplayName + " | " + std::to_string(box.iStartMs) + " ms##" + box.strOccurrenceId;
				if (ImGui::Selectable(label.c_str(), draft.strTrackingPresentationOccurrenceId == box.strOccurrenceId))
					draft.strTrackingPresentationOccurrenceId = box.strOccurrenceId;
			}
		ImGui::EndCombo();
	}
	int interval = static_cast<int>(draft.iSpawnIntervalMs);
	if (ImGui::InputInt("Spawn interval (ms)", &interval, 100, 1000))
		draft.iSpawnIntervalMs = static_cast<std::uint32_t>(std::clamp(interval, 1, 600000));
	float speed = static_cast<float>(draft.fFollowSpeedScale);
	if (ImGui::InputFloat("Player move speed multiplier", &speed, .05f, .1f, "%.2f"))
		draft.fFollowSpeedScale = std::clamp(static_cast<double>(speed), .01, 10.0);
	ImGui::TextWrapped(
		"Creates one fixed group per alive player at each interval. One tracking target follows each player until Duration ends. The selected Effect rows become templates for Server playback; their relative timing and transforms are preserved.");
	Render_AttackHitTemplates("Fixed attack hits", draft.FixedHits);
	Render_AttackHitTemplates("Tracking attack hits", draft.TrackingHits);
	Render_LogicShowtimeVolleyValues(draft, pattern);
}

// 네 방향 분신에 허용된 child Pattern과 Summon 배치, 복제 종료 Stage를 선택한다. 방향 배열은 앞·뒤·왼쪽·오른쪽 네 칸을 유지한다.
// 기존 child 검증을 통과한 후보만 보여 주고 실제 actor 생성이나 Server 판정은 실행하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Render_LogicCrossDirectionValues(
	KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &draft)
{
	draft.DirectionPatternIds.resize(4u);
	constexpr std::array<const char *, 4u> labels{"Front Pattern", "Back Pattern", "Left Pattern", "Right Pattern"};
	const auto *parent = Find_Pattern(m_Draft, m_strSelectedPatternId);
	for (std::size_t direction = 0; direction < labels.size(); ++direction)
	{
		auto &id = draft.DirectionPatternIds[direction];
		const auto *selected = Find_Pattern(m_Draft, id);
		const char *name = selected ? selected->strDisplayName.c_str() : (id.empty() ? "(select Pattern)" : id.c_str());
		if (ImGui::BeginCombo(labels[direction], name))
		{
			for (const auto &candidate : m_Draft.Patterns)
			{
				if (!candidate.strLoadError.empty() || candidate.strCategory != "MECHANIC" ||
					!candidate.PatternOccurrences.empty() || !candidate.SummonOccurrences.empty() ||
					!candidate.WorldOccurrences.empty() || !candidate.SceneProfileOccurrences.empty() ||
					candidate.BossMotion || candidate.bResetBossToSpawn || candidate.ResetBossYawDegrees ||
					candidate.bEnterCombatOnFinish || Find_TimelineParent(m_Draft, candidate.strPatternId))
					continue;
				if (parent &&
					(candidate.strPatternId == parent->strPatternId || candidate.strGateId != parent->strGateId ||
						candidate.strActorProfileId != parent->strActorProfileId ||
						candidate.strTargetBossPlacementId != parent->strTargetBossPlacementId))
					continue;
				std::string logicStatus;
				if (!CKoukuSaydonCompositionDocument::Validate_CrossDirectionChildLogic(
						m_Draft, candidate, logicStatus))
					continue;
				if (ImGui::Selectable((candidate.strDisplayName + "##" + candidate.strPatternId).c_str(),
						id == candidate.strPatternId))
					id = candidate.strPatternId;
			}
			ImGui::EndCombo();
		}
	}
	const auto *summon = parent ? Find_SummonBox(*parent, draft.strSummonOccurrenceId) : nullptr;
	if (ImGui::BeginCombo("Summon placement", summon ? summon->strOccurrenceId.c_str() : "(select Summon box)"))
	{
		if (parent)
			for (const auto &box : parent->SummonOccurrences)
			{
				if (!box.PatternSpawns.empty())
					continue;
				const auto *definition = Find_Summon(m_Draft, box.strSummonId);
				const std::string label =
					(definition ? definition->strDisplayName : box.strSummonId) + "##" + box.strOccurrenceId;
				if (ImGui::Selectable(label.c_str(), box.strOccurrenceId == draft.strSummonOccurrenceId))
					draft.strSummonOccurrenceId = box.strOccurrenceId;
			}
		ImGui::EndCombo();
	}
	if (ImGui::BeginCombo("Clones play through",
			draft.strCloneEndStageId.empty() ? "(select Stage)" : draft.strCloneEndStageId.c_str()))
	{
		if (const auto *first = Find_Pattern(m_Draft, draft.DirectionPatternIds.front()))
			for (const auto &stage : first->Stages)
				if (ImGui::Selectable(stage.strStageId.c_str(), stage.strStageId == draft.strCloneEndStageId))
					draft.strCloneEndStageId = stage.strStageId;
		ImGui::EndCombo();
	}
	ImGui::TextWrapped(
		"The Server compares each direction's movement endpoint with the arena center. One real body plays its full Pattern; three Summon clones stop after the selected Stage. Duration must cover all four complete Patterns.");
}

// 추적 Duration의 이동 배율과 종료 시 폭탄 부채꼴 판정 입력을 편집한다. 폭탄 모드 전환 시 관련 Effect 참조와 기본값을 함께 정리한다.
// Effect 선택은 현재 Pattern을 차용하며 반경은 m, 각도는 degree로 기존 draft 필드에 저장한다.
void Client::CKoukuSaydonActionWorkbench::Render_LogicTrackTargetValues(
	KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &draft, const std::string_view colliderPatternId)
{
	float followSpeed = static_cast<float>(draft.fFollowSpeedScale);
	if (ImGui::InputFloat("Follow speed x player##KoukuLogicValue", &followSpeed, .05f, .25f, "%.2f"))
		draft.fFollowSpeedScale = followSpeed <= 0.f ? 0.0 : double(std::clamp(followSpeed, .01f, 10.f));
	ImGui::TextWrapped(
		"Rotate toward the Server-selected target during this Duration. Rotate-only response is 10x the previous remaining-arc rate; movement-enabled tracking turns at 180 degrees/second. Uses the shortest arc.");
	ImGui::TextWrapped(
		"Follow speed 0 only rotates. Above 0 the Server also walks the body along its current facing at that multiple of the tracked player's own move speed, so BOSS-anchored Effects travel and turn with it.");
	bool bomb = !draft.strBombPresentationOccurrenceId.empty() || draft.fBombSectorRadiusM > 0.0;
	if (ImGui::Checkbox("Bomb sector test at Duration end", &bomb))
	{
		draft.strBombPresentationOccurrenceId.clear();
		draft.strBombExplosionPresentationOccurrenceId.clear();
		draft.strBombSectorPresentationOccurrenceId.clear();
		draft.fBombSectorRadiusM = bomb ? 11.0 : 0.0;
		draft.fBombSectorHalfAngleDegrees = bomb ? 22.5 : 0.0;
		if (bomb)
			draft.fFollowSpeedScale = 0.0;
	}
	if (bomb)
	{
		const auto *pattern =
			Find_Pattern(m_Draft, colliderPatternId.empty() ? m_strSelectedPatternId : colliderPatternId);
		const auto selectEffect = [&](const char *label, std::string &selected, const bool sector) {
			if (!ImGui::BeginCombo(label, selected.empty() ? "(select Effect)" : selected.c_str()))
				return;
			if (pattern)
				for (const auto &box : pattern->PresentationOccurrences)
				{
					const auto *resource = Find_PresentationResource(m_Draft, box.strResourceId);
					if (!resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
						(resource->strResourceKind != "V1_EFFECT" && resource->strResourceKind != "V1_ELEMENT") ||
						box.strAnchorKind != (sector ? "BOSS" : "MAP") || box.bFollowBoss != sector ||
						!box.strBone.empty() || !box.strWorldId.empty() || !box.strWorldOccurrenceId.empty() ||
						!box.strLogicOccurrenceId.empty() || !box.strAnchorPresentationOccurrenceId.empty())
						continue;
					const std::string text =
						resource->strDisplayName + " | " + std::to_string(box.iStartMs) + " ms##" + box.strOccurrenceId;
					if (ImGui::Selectable(text.c_str(), selected == box.strOccurrenceId))
						selected = box.strOccurrenceId;
				}
			ImGui::EndCombo();
		};
		selectEffect("Bomb body and fuse", draft.strBombPresentationOccurrenceId, false);
		selectEffect("Bomb failure explosion", draft.strBombExplosionPresentationOccurrenceId, false);
		selectEffect("Facing sector", draft.strBombSectorPresentationOccurrenceId, true);
		float radius = float(draft.fBombSectorRadiusM), half = float(draft.fBombSectorHalfAngleDegrees);
		if (ImGui::InputFloat("Sector local radius (m)", &radius, .1f, 1.f, "%.2f"))
			draft.fBombSectorRadiusM = double(std::clamp(radius, .001f, 1000.f));
		if (ImGui::InputFloat("Sector half angle (deg)", &half, .5f, 5.f, "%.2f"))
			draft.fBombSectorHalfAngleDegrees = double(std::clamp(half, .001f, 179.999f));
		ImGui::TextWrapped(
			"At Duration end the Server removes the bomb inside the sector; outside it explodes and wipes the raid. Preview shows authored timing; Server Play performs the test.");
	}
}

// 피해 Result에 붙는 밀어내기 반응을 편집한다. 비활성화·탄도 전환은 서로 맞지 않는 높이·시간·방향 값을 기존 규칙대로 정리한다.
// 이곳은 m·ms·degree 입력만 보관하며 이동·항법·피격 반응의 실행 주체는 Server다.
void Client::CKoukuSaydonActionWorkbench::Render_LogicKnockbackValues(KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &draft)
{
	bool knockback = draft.fPushRangeM > 0.0 || draft.fPushHeightM > 0.0;
	if (ImGui::Checkbox("Knockback##KoukuLogicValue", &knockback))
	{
		draft.fPushRangeM = knockback ? 2.0 : 0.0;
		draft.iPushMs = knockback ? 242u : 0u;
		if (!knockback)
		{
			draft.strPushDirection = "AWAY_FROM_BOSS";
			draft.bForcePush = false;
			draft.bPushCanLeaveArena = false;
			draft.bPushBallistic = false;
			draft.fPushYawOffsetDegrees = draft.fPushHeightM = 0.0;
		}
	}
	if (knockback)
	{
		if (ImGui::Checkbox("Ballistic push##Logic", &draft.bPushBallistic))
		{
			draft.fPushHeightM = draft.bPushBallistic ? 2.0 : 0.0;
			draft.fPushRangeM =
				std::clamp(draft.fPushRangeM, draft.bPushBallistic ? 0.0 : .01, draft.bPushBallistic ? 100.0 : 20.0);
			draft.iPushMs = std::clamp(
				draft.iPushMs, draft.bPushBallistic ? 100u : 1u, draft.bPushBallistic ? 5000u : MAX_EDITOR_TIME_MS);
		}
		const float maxDistance = draft.bPushBallistic ? 100.f : 20.f;
		const int minTime = draft.bPushBallistic ? 100 : 1;
		const int maxTime = draft.bPushBallistic ? 5000 : static_cast<int>(MAX_EDITOR_TIME_MS);
		float distance = static_cast<float>(draft.fPushRangeM);
		int timeMs = static_cast<int>(draft.iPushMs);
		if (ImGui::InputFloat("Knockback distance (m)", &distance, .1f, 1.f, "%.2f"))
			draft.fPushRangeM = std::clamp(distance, draft.bPushBallistic ? 0.f : .01f, maxDistance);
		if (ImGui::InputInt("Knockback time (ms)", &timeMs, 10, 100))
			draft.iPushMs = static_cast<std::uint32_t>(std::clamp(timeMs, minTime, maxTime));
		int direction = draft.strPushDirection == "BOSS_FORWARD" ? 1
			: draft.strPushDirection == "AWAY_FROM_CONTACT"		 ? 2
																 : 0;
		if (ImGui::Combo(
				"Push direction##Logic", &direction, "Away from boss\0Boss current forward\0Away from contact\0"))
		{
			draft.strPushDirection = direction == 1 ? "BOSS_FORWARD"
				: direction == 2					? "AWAY_FROM_CONTACT"
													: "AWAY_FROM_BOSS";
			if (direction != 1)
				draft.fPushYawOffsetDegrees = 0.0;
		}
		if (direction == 1)
		{
			float yaw = static_cast<float>(draft.fPushYawOffsetDegrees);
			if (ImGui::InputFloat("Push yaw offset (deg)", &yaw, 1.f, 15.f, "%.1f"))
				draft.fPushYawOffsetDegrees = std::clamp(yaw, -360.f, 360.f);
		}
		ImGui::Checkbox("Force knockback reaction", &draft.bForcePush);
		if (draft.bPushBallistic)
		{
			float height = float(draft.fPushHeightM);
			if (ImGui::DragFloat("Rise height (m)##Logic", &height, .05f, .01f, 100.f))
				draft.fPushHeightM = height;
		}
		ImGui::Checkbox("Allow arena-edge fall", &draft.bPushCanLeaveArena);
		ImGui::TextWrapped("Uses the Server hit reaction, navigation and collision rules.");
	}
}

// 공포 Result가 참조할 Scene Profile·Light·Effect·Sound와 지연을 한곳에서 설정한다. 선택 목록은 현재 문서의 stable ID를 사용한다.
// Effect를 해제하면 지연도 비우며, 지연 상한은 공포 지속시간에서 계산한다. 미완성 입력의 거부는 Apply 경계가 담당한다.
void Client::CKoukuSaydonActionWorkbench::Render_LogicFearValues(KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &draft)
{
	int durationMs = static_cast<int>(draft.iDurationMs);
	if (ImGui::InputInt("Fear duration ms", &durationMs, 100, 1000))
		draft.iDurationMs = static_cast<std::uint32_t>(std::clamp(durationMs, 1, static_cast<int>(MAX_EDITOR_TIME_MS)));
	const auto *profile = Find_SceneProfile(m_Draft, draft.strSceneProfileId);
	if (ImGui::BeginCombo("Fear Scene Profile",
			profile ? profile->strDisplayName.c_str()
					: (draft.strSceneProfileId.empty() ? "(none)" : "(missing saved profile)")))
	{
		if (ImGui::Selectable("(none)", draft.strSceneProfileId.empty()))
			draft.strSceneProfileId.clear();
		for (const auto &item : m_Draft.SceneProfiles)
			if (ImGui::Selectable((item.strDisplayName + "##" + item.strSceneProfileId).c_str(),
					draft.strSceneProfileId == item.strSceneProfileId))
				draft.strSceneProfileId = item.strSceneProfileId;
		ImGui::EndCombo();
	}
	const auto *effect = Find_PresentationResource(m_Draft, draft.strEffectResourceId);
	const auto *light = Find_PresentationResource(m_Draft, draft.strLightResourceId);
	if (ImGui::BeginCombo("Fear Character Light",
			light ? light->strDisplayName.c_str()
				  : (draft.strLightResourceId.empty() ? "(none)" : "(missing saved Light)")))
	{
		if (ImGui::Selectable("(none)", draft.strLightResourceId.empty()))
			draft.strLightResourceId.clear();
		for (const auto &item : m_Draft.PresentationResources)
			if (item.eKind == KOUKU_SAYDON_PRESENTATION_KIND::LIGHT && item.strDefaultAnchorKind == "PLAYER" &&
				ImGui::Selectable((item.strDisplayName + "##" + item.strResourceId).c_str(),
					draft.strLightResourceId == item.strResourceId))
				draft.strLightResourceId = item.strResourceId;
		ImGui::EndCombo();
	}
	if (ImGui::BeginCombo("Fear Effect",
			effect ? effect->strDisplayName.c_str()
				   : (draft.strEffectResourceId.empty() ? "(none)" : "(missing saved Effect)")))
	{
		if (ImGui::Selectable("(none)", draft.strEffectResourceId.empty()))
		{
			draft.strEffectResourceId.clear();
			draft.iEffectDelayMs = 0u;
		}
		for (const auto &item : m_Draft.PresentationResources)
			if (item.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT &&
				ImGui::Selectable((item.strDisplayName + "##" + item.strResourceId).c_str(),
					draft.strEffectResourceId == item.strResourceId))
				draft.strEffectResourceId = item.strResourceId;
		ImGui::EndCombo();
	}
	const auto *sound = Find_PresentationResource(m_Draft, draft.strSoundResourceId);
	if (ImGui::BeginCombo("Fear Sound",
			sound ? sound->strDisplayName.c_str()
				  : (draft.strSoundResourceId.empty() ? "(none)" : "(missing saved Sound)")))
	{
		if (ImGui::Selectable("(none)", draft.strSoundResourceId.empty()))
			draft.strSoundResourceId.clear();
		for (const auto &item : m_Draft.PresentationResources)
			if (item.eKind == KOUKU_SAYDON_PRESENTATION_KIND::SOUND &&
				ImGui::Selectable((item.strDisplayName + "##" + item.strResourceId).c_str(),
					draft.strSoundResourceId == item.strResourceId))
				draft.strSoundResourceId = item.strResourceId;
		ImGui::EndCombo();
	}
	ImGui::BeginDisabled(draft.strEffectResourceId.empty());
	int delayMs = static_cast<int>(draft.iEffectDelayMs);
	if (ImGui::InputInt("Effect delay from Fear start ms", &delayMs, 100, 1000))
		draft.iEffectDelayMs =
			static_cast<std::uint32_t>(std::clamp(delayMs, 0, (std::max)(0, static_cast<int>(draft.iDurationMs) - 1)));
	ImGui::EndDisabled();
	ImGui::TextWrapped(
		"Server Fear locks movement and skills and plays the character's fear animation. The affected local player sees this Scene Profile and optional delayed Effect for the same duration. The optional Sound plays once when the delayed Effect starts. A full-screen face belongs to a Screen Effect resource.");
}

// World inventory의 대상 instance와 같은 Object 자원에 속한 저장 Motion을 짝지어 선택한다.
// 이 목록은 현재 Pattern의 배치 여부까지 거르지는 않는다. 대상 변경 시 맞지 않는 기존 Motion 선택을 비운다.
// World inventory의 포인터는 이 호출 안에서만 차용하고 실제 외부 World 문서나 재생 상태는 변경하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Render_LogicWorldMotionValues(
	KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &draft)
{
	const auto findWorld = [this](const std::string &id) -> const KOUKU_WORLD_SEQUENCE_RESOURCE * {
		const auto found =
			std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(), [&id](const auto &row) {
				return !id.empty() && row.bEnabled && row.strInstanceId == id;
			});
		return found == m_WorldSequenceResources.end() ? nullptr : &*found;
	};
	const auto *target = findWorld(draft.strTargetWorldInstanceId);
	const char *targetLabel = target ? target->strDisplayName.c_str() : "(pick spawned World Object)";
	if (ImGui::BeginCombo("Target World Object##KoukuMotionResult", targetLabel))
	{
		for (const auto &candidate : m_WorldSequenceResources)
		{
			if (!candidate.bEnabled || candidate.strInstanceId.empty() || candidate.strObjectResourceId.empty())
				continue;
			const std::string label = candidate.strDisplayName + "##" + candidate.strInstanceId;
			if (ImGui::Selectable(label.c_str(), draft.strTargetWorldInstanceId == candidate.strInstanceId))
			{
				draft.strTargetWorldInstanceId = candidate.strInstanceId;
				target = &candidate;
				const auto *motion = findWorld(draft.strMotionInstanceId);
				if (!motion || motion->strObjectResourceId != candidate.strObjectResourceId)
					draft.strMotionInstanceId.clear();
			}
		}
		ImGui::EndCombo();
	}
	const auto *motion = findWorld(draft.strMotionInstanceId);
	if (ImGui::BeginCombo(
			"Saved Motion##KoukuMotionResult", motion ? motion->strDisplayName.c_str() : "(pick same-object Motion)"))
	{
		if (target && !target->strObjectResourceId.empty())
			for (const auto &candidate : m_WorldSequenceResources)
				if (candidate.bEnabled && !candidate.strInstanceId.empty() &&
					candidate.strObjectResourceId == target->strObjectResourceId &&
					ImGui::Selectable((candidate.strDisplayName + "##" + candidate.strInstanceId).c_str(),
						draft.strMotionInstanceId == candidate.strInstanceId))
					draft.strMotionInstanceId = candidate.strInstanceId;
		ImGui::EndCombo();
	}
	ImGui::TextWrapped(
		"Server applies this saved Motion once per window to the existing target instance. Start that target in the WORLD lane first.");
	if (!target || !motion || target->strObjectResourceId.empty() ||
		target->strObjectResourceId != motion->strObjectResourceId)
		ImGui::TextDisabled("Choose a target and Motion bound to the same World Object in this Area.");
}

// OBJECT_CONTACT Trigger의 대상 배치를 Result의 반응 Motion 목록으로 옮긴다. 같은 대상에 이미 정한 Motion은 보존한다.
// 각 mapping은 World occurrence ID를 소유하며 선택 가능한 Motion은 해당 Object 자원이 같은 항목으로 제한한다.
void Client::CKoukuSaydonActionWorkbench::Render_LogicContactMotionValues(
	KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &draft)
{
	const auto *pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	if (ImGui::BeginCombo("Copy targets from Trigger", "Choose OBJECT_CONTACT"))
	{
		if (pattern)
			for (const auto &window : pattern->LogicOccurrences)
			{
				const auto *trigger = Find_Logic(m_Draft, window.strLogicId);
				if (!trigger || trigger->strTriggerKind != "OBJECT_CONTACT")
					continue;
				if (ImGui::Selectable((trigger->strDisplayName + "##" + window.strOccurrenceId).c_str()))
				{
					std::vector<KOUKU_SAYDON_CONTACT_MOTION> mappings;
					for (const auto &target : trigger->TargetWorldOccurrenceIds)
					{
						const auto existing = std::find_if(
							draft.ContactMotions.begin(), draft.ContactMotions.end(), [&](const auto &row) {
								return row.strTargetWorldOccurrenceId == target;
							});
						mappings.push_back(existing == draft.ContactMotions.end()
								? KOUKU_SAYDON_CONTACT_MOTION{target, {}}
								: *existing);
					}
					draft.ContactMotions = std::move(mappings);
				}
			}
		ImGui::EndCombo();
	}
	for (auto &mapping : draft.ContactMotions)
	{
		const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE *targetBox = nullptr;
		if (pattern)
			for (const auto &row : pattern->WorldOccurrences)
				if (row.strOccurrenceId == mapping.strTargetWorldOccurrenceId)
				{
					targetBox = &row;
					break;
				}
		const auto *world = targetBox ? Find_World(m_Draft, targetBox->strWorldId) : nullptr;
		const auto targetResource =
			std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(), [&](const auto &row) {
				return world && row.strInstanceId == world->strSequenceInstanceId;
			});
		const auto motionResource =
			std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(), [&](const auto &row) {
				return row.strInstanceId == mapping.strMotionInstanceId;
			});
		ImGui::PushID(mapping.strTargetWorldOccurrenceId.c_str());
		ImGui::TextWrapped("%s | %s", world ? world->strDisplayName.c_str() : "Missing WORLD placement",
			mapping.strTargetWorldOccurrenceId.c_str());
		if (ImGui::BeginCombo("Reaction Motion",
				motionResource == m_WorldSequenceResources.end() ? "Choose saved Motion"
																 : motionResource->strDisplayName.c_str()))
		{
			for (const auto &candidate : m_WorldSequenceResources)
				if (targetResource != m_WorldSequenceResources.end() && candidate.bEnabled &&
					!candidate.strInstanceId.empty() && !candidate.strObjectResourceId.empty() &&
					candidate.strObjectResourceId == targetResource->strObjectResourceId &&
					ImGui::Selectable((candidate.strDisplayName + "##" + candidate.strInstanceId).c_str(),
						candidate.strInstanceId == mapping.strMotionInstanceId))
					mapping.strMotionInstanceId = candidate.strInstanceId;
			ImGui::EndCombo();
		}
		ImGui::PopID();
	}
	ImGui::TextWrapped(
		"Choose a saved reaction Motion for every selected card placement. On contact, only the hit card receives its Motion. Use Flip for the center Trigger and Hop for the edge Trigger.");
}

// 접촉 Trigger의 판정 반경·타격 그룹·우선순위와 대상 World occurrence 목록을 편집한다. 동일한 저장 인스턴스의 개별 배치를 ID로 구별한다.
// 이 패널은 대상 선택만 기록하며 실제 접촉 여부와 한 타격 안의 우선순위 판정은 Server가 소비한다.
void Client::CKoukuSaydonActionWorkbench::Render_LogicObjectContactValues(
	KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &draft)
{
	float radius = static_cast<float>(draft.fTargetRadiusM);
	if (ImGui::InputFloat("Card target radius (m)", &radius, .1f, 1.f, "%.2f"))
		draft.fTargetRadiusM = std::clamp(radius, .01f, 1000.f);
	char group[256]{};
	Copy_Text(group, std::size(group), draft.strContactGroupId);
	if (ImGui::InputText("Strike group (optional)", group, std::size(group)))
		draft.strContactGroupId = group;
	int priority = static_cast<int>(draft.iContactPriority);
	if (ImGui::InputInt("Contact priority", &priority))
		draft.iContactPriority = static_cast<std::uint32_t>(std::clamp(priority, 0, 1000));
	ImGui::TextWrapped(
		"Use the same group and time for one strike's center/edge windows; higher priority wins for each card. The group may be reused at another strike time.");
	ImGui::SeparatorText("Target card placements");
	const auto *pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	if (pattern)
		for (const auto &worldBox : pattern->WorldOccurrences)
		{
			const auto *world = Find_World(m_Draft, worldBox.strWorldId);
			const auto resource =
				std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(), [&](const auto &row) {
					return world && row.strInstanceId == world->strSequenceInstanceId && row.bEnabled &&
						!row.strObjectResourceId.empty();
				});
			if (resource == m_WorldSequenceResources.end())
				continue;
			bool selected = std::find(draft.TargetWorldOccurrenceIds.begin(), draft.TargetWorldOccurrenceIds.end(),
								worldBox.strOccurrenceId) != draft.TargetWorldOccurrenceIds.end();
			if (ImGui::Checkbox((world->strDisplayName + " | " + worldBox.strOccurrenceId).c_str(), &selected))
			{
				if (selected && draft.TargetWorldOccurrenceIds.size() < 64u)
					draft.TargetWorldOccurrenceIds.push_back(worldBox.strOccurrenceId);
				else if (!selected)
					std::erase(draft.TargetWorldOccurrenceIds, worldBox.strOccurrenceId);
			}
		}
	ImGui::TextWrapped(
		"Append card WORLD boxes to this Pattern first. Select the placements that may be hit. Each is judged once during the Trigger window; no contact produces no reaction. Bone anchors follow the hammer tip in XZ. Match the window to the strike moment; height alone does not activate contact.");
}

// Albion 원형 공격의 플레이어별 배치와 아레나 무작위 배치 입력을 편집한다. 개수가 0 또는 1로 바뀔 때 불필요해지는 반경을 함께 정리한다.
// 반경·높이 허용차·간격은 m, 수명은 ms이며 이곳에서는 배치 위치를 계산하거나 객체를 생성하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Render_LogicAlbionCircleValues(
	KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &draft)
{
	Render_AttackHitTemplates("Circle impact hits", draft.FixedHits);
	int playerSelection = draft.bRandomPlayerOnly ? 1 : 0;
	if (ImGui::Combo("Player selection", &playerSelection, "Every living player\0One random living player\0"))
	{
		draft.bRandomPlayerOnly = playerSelection == 1;
		if (draft.bRandomPlayerOnly)
		{
			draft.iCountPerPlayer = 1u;
			draft.fPlayerEffectRadiusM = 0.0;
		}
	}
	ImGui::BeginDisabled(draft.bRandomPlayerOnly);
	int count = static_cast<int>(draft.iCountPerPlayer);
	if (ImGui::InputInt("Circles per living player", &count))
	{
		draft.iCountPerPlayer = static_cast<std::uint32_t>(std::clamp(count, 1, 8));
		if (draft.iCountPerPlayer == 1u)
			draft.fPlayerEffectRadiusM = 0.0;
		else if (draft.fPlayerEffectRadiusM <= 0.0)
			draft.fPlayerEffectRadiusM = 2.0;
	}
	float radius = static_cast<float>(draft.fPlayerEffectRadiusM);
	if (draft.iCountPerPlayer > 1u && ImGui::InputFloat("Ring radius (m)", &radius, .25f, 1.f))
		draft.fPlayerEffectRadiusM = std::clamp(radius, .01f, 20.f);
	ImGui::EndDisabled();
	int arenaCount = static_cast<int>(draft.iArenaRandomCount);
	if (ImGui::InputInt("Random arena circles", &arenaCount))
	{
		draft.iArenaRandomCount = static_cast<std::uint32_t>(std::clamp(arenaCount, 0, 32));
		if (draft.iArenaRandomCount == 0u)
		{
			draft.fArenaRandomRadiusM = 0.0;
			draft.fArenaHeightToleranceM = 0.0;
			draft.fArenaMinimumSpacingM = 0.0;
		}
		else
		{
			if (draft.fArenaRandomRadiusM <= 0.0)
				draft.fArenaRandomRadiusM = 14.0;
			if (draft.fArenaHeightToleranceM <= 0.0)
				draft.fArenaHeightToleranceM = 1.0;
			if (draft.fArenaMinimumSpacingM <= 0.0)
				draft.fArenaMinimumSpacingM = 3.2;
		}
	}
	if (draft.iArenaRandomCount > 0u)
	{
		float arenaRadius = static_cast<float>(draft.fArenaRandomRadiusM);
		if (ImGui::InputFloat("Arena radius (m)", &arenaRadius, .5f, 1.f))
			draft.fArenaRandomRadiusM = std::clamp(arenaRadius, .01f, 100.f);
		float heightTolerance = static_cast<float>(draft.fArenaHeightToleranceM);
		if (ImGui::InputFloat("Ground height tolerance (m)", &heightTolerance, .1f, 1.f))
			draft.fArenaHeightToleranceM = std::clamp(heightTolerance, .01f, 10.f);
		float minimumSpacing = static_cast<float>(draft.fArenaMinimumSpacingM);
		if (ImGui::InputFloat("Arena circle spacing (m)", &minimumSpacing, .1f, 1.f))
			draft.fArenaMinimumSpacingM = std::clamp(minimumSpacing, .01f, 20.f);
	}
	int lifetime = static_cast<int>(draft.iEffectLifetimeMs);
	if (ImGui::InputInt("Effect lifetime (ms)", &lifetime))
		draft.iEffectLifetimeMs = static_cast<std::uint32_t>(std::clamp(lifetime, 1, 600000));
	ImGui::TextWrapped(
		"Player circles stay at the positions selected when the Trigger starts. Arena circles use navigable ground around the boss spawn position, within the radius and height tolerance, with spacing between arena circles. The Server creates the whole set together after validating every position. The existing Effect includes its warning, explosion and tail; this Trigger adds no damage.");
}

// 미로 진입 Effect를 플레이어 슬롯 순서대로 편집한다. 같은 Logic 정의를 쓰는 가장 늦은 Trigger 이후에 시작하는 고정 MAP Effect만 허용한다.
// 삭제 뒤 같은 index를 다시 처리하고 ImGui ID를 먼저 닫아 다음 행과 UI 스택을 함께 보존한다.
void Client::CKoukuSaydonActionWorkbench::Render_LogicMazeEntryValues(
	const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &logic, KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &draft)
{
	const auto *pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	std::uint32_t latestTriggerStartMs = 0u;
	if (pattern)
		for (const auto &box : pattern->LogicOccurrences)
			if (box.strLogicId == logic.strLogicId)
				latestTriggerStartMs = (std::max)(latestTriggerStartMs, box.iStartMs);
	ImGui::TextWrapped(
		"Choose 1..4 entry Effects in player-slot order. Their MAP positions are used when this Trigger starts; each Effect must start at or after the Trigger. Coordinates stay owned by the Effect boxes.");
	for (std::size_t i = 0u; i < draft.PlayerEntryEffectOccurrenceIds.size();)
	{
		ImGui::PushID(static_cast<int>(i));
		ImGui::Text("Player %zu: %s", i + 1u, draft.PlayerEntryEffectOccurrenceIds[i].c_str());
		ImGui::SameLine();
		if (ImGui::SmallButton("Remove"))
		{
			draft.PlayerEntryEffectOccurrenceIds.erase(draft.PlayerEntryEffectOccurrenceIds.begin() + i);
			ImGui::PopID();
			continue;
		}
		ImGui::SameLine();
		ImGui::BeginDisabled(i == 0u);
		if (ImGui::SmallButton("Up"))
			std::swap(draft.PlayerEntryEffectOccurrenceIds[i], draft.PlayerEntryEffectOccurrenceIds[i - 1u]);
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(i + 1u == draft.PlayerEntryEffectOccurrenceIds.size());
		if (ImGui::SmallButton("Down"))
			std::swap(draft.PlayerEntryEffectOccurrenceIds[i], draft.PlayerEntryEffectOccurrenceIds[i + 1u]);
		ImGui::EndDisabled();
		ImGui::PopID();
		++i;
	}
	ImGui::BeginDisabled(!pattern || draft.PlayerEntryEffectOccurrenceIds.size() >= 4u);
	if (ImGui::BeginCombo("Add entry Effect", "Choose a fixed MAP Effect"))
	{
		if (pattern)
			for (const auto &box : pattern->PresentationOccurrences)
			{
				const auto *resource = Find_PresentationResource(m_Draft, box.strResourceId);
				if (!resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
					box.strAnchorKind != "MAP" || box.bFollowBoss || !box.strBone.empty() ||
					box.strBoneTarget != "BODY" || !box.strWorldId.empty() || !box.strWorldOccurrenceId.empty() ||
					box.iWorldEmissionIndex != 0u || !box.strLogicOccurrenceId.empty() ||
					box.iStartMs < latestTriggerStartMs ||
					std::find(draft.PlayerEntryEffectOccurrenceIds.begin(), draft.PlayerEntryEffectOccurrenceIds.end(),
						box.strOccurrenceId) != draft.PlayerEntryEffectOccurrenceIds.end())
					continue;
				const std::string label =
					resource->strDisplayName + " | " + std::to_string(box.iStartMs) + " ms | " + box.strOccurrenceId;
				if (ImGui::Selectable(label.c_str()))
					draft.PlayerEntryEffectOccurrenceIds.push_back(box.strOccurrenceId);
			}
		ImGui::EndCombo();
	}
	ImGui::EndDisabled();
}

// Albion 공중 동작의 phase와 그 phase가 사용하는 높이·상승시간·대상 위치 정책을 편집한다. phase 변경은 이전 phase의 전용 값을 함께 초기화한다.
// 선택 시 고정되는 지면 Effect 그룹과 나중에 플레이어를 다시 읽는 정책을 구별하며 실제 위치 선택은 재생 owner가 맡는다.
void Client::CKoukuSaydonActionWorkbench::Render_LogicAirborneValues(
	KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &draft, const std::string_view colliderPatternId)
{
	if (ImGui::BeginCombo("Airborne phase", draft.strAirbornePhase.c_str()))
	{
		for (const char *phase : {"SELECT_PLAYER", "JUMP", "APPEAR_PLAYER", "DISAPPEAR", "CENTER", "SLAM"})
			if (ImGui::Selectable(phase, draft.strAirbornePhase == phase) && draft.strAirbornePhase != phase)
			{
				draft.strAirbornePhase = phase;
				draft.fAirborneHeightM = 0.0;
				draft.iAirborneDurationMs = 0u;
				draft.TeleportPosition = {};
				draft.strAirborneTargetPositionPolicy = "APPEAR";
				draft.strSelectedEffectGroupId.clear();
			}
		ImGui::EndCombo();
	}
	if (draft.strAirbornePhase == "SELECT_PLAYER")
	{
		if (ImGui::BeginCombo("Capture target position",
				draft.strAirborneTargetPositionPolicy == "SELECT" ? "On selection" : "On appearance"))
		{
			if (ImGui::Selectable("On appearance", draft.strAirborneTargetPositionPolicy == "APPEAR"))
			{
				draft.strAirborneTargetPositionPolicy = "APPEAR";
				draft.strSelectedEffectGroupId.clear();
			}
			if (ImGui::Selectable("On selection", draft.strAirborneTargetPositionPolicy == "SELECT"))
				draft.strAirborneTargetPositionPolicy = "SELECT";
			ImGui::EndCombo();
		}
		if (draft.strAirborneTargetPositionPolicy == "SELECT")
		{
			const auto *pattern =
				Find_Pattern(m_Draft, colliderPatternId.empty() ? m_strSelectedPatternId : colliderPatternId);
			if (ImGui::BeginCombo("Effect group at selected point",
					draft.strSelectedEffectGroupId.empty() ? "(none)" : draft.strSelectedEffectGroupId.c_str()))
			{
				if (ImGui::Selectable("(none)", draft.strSelectedEffectGroupId.empty()))
					draft.strSelectedEffectGroupId.clear();
				std::unordered_set<std::string> shown;
				if (pattern)
					for (const auto &box : pattern->PresentationOccurrences)
					{
						const auto *resource = Find_PresentationResource(m_Draft, box.strResourceId);
						if (!resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
							box.strAnchorKind != "MAP" || box.bFollowBoss || box.strSelectionGroupId.empty() ||
							!shown.insert(box.strSelectionGroupId).second)
							continue;
						const std::string label =
							resource->strDisplayName + "##selected-ground:" + box.strSelectionGroupId;
						if (ImGui::Selectable(label.c_str(), draft.strSelectedEffectGroupId == box.strSelectionGroupId))
							draft.strSelectedEffectGroupId = box.strSelectionGroupId;
					}
				ImGui::EndCombo();
			}
			ImGui::TextWrapped(
				"The selected ground point stays fixed for landing and the Effect group, even after the player moves.");
		}
	}
	if (draft.strAirbornePhase == "JUMP" || draft.strAirbornePhase == "APPEAR_PLAYER")
		ImGui::InputDouble("Airborne height (m)", &draft.fAirborneHeightM, .1, 1.0, "%.6f");
	if (draft.strAirbornePhase == "JUMP")
	{
		int duration = int(draft.iAirborneDurationMs);
		if (ImGui::InputInt("Rise time (ms, 0 = start high)", &duration))
			draft.iAirborneDurationMs = uint32_t(std::clamp(duration, 0, 600000));
	}
	if (draft.strAirbornePhase == "CENTER")
		ImGui::InputScalarN("Center destination XYZ (m)", ImGuiDataType_Double, draft.TeleportPosition.data(), 3,
			nullptr, nullptr, "%.3f");
	ImGui::TextWrapped(
		"Play Preview pins replicated player positions for its preview actor. Server Play selects live players and validates destinations. Animation time is unchanged; airborne phases own the actor height.");
}

// DURATION 패널의 종류 선택과 종류별 입력 호출을 조정한다. 종류가 바뀌면 원래 Logic의 ID·이름·타입만 남기고 해당 종류 기본값을 넣는다.
// 하위 입력 함수는 같은 draft를 차용하므로 ImGui ID 범위와 위젯 호출 순서를 여기서 바꾸지 않는다.
void Client::CKoukuSaydonActionWorkbench::Render_LogicJudgementValues(
	const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &logic, KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &draft,
	const std::string_view colliderPatternId)
{
	if (ImGui::BeginCombo("Judgement kind##KoukuLogicValue",
			draft.strJudgementKind.empty() ? "(name only)" : draft.strJudgementKind.c_str()))
	{
		if (ImGui::Selectable("(name only)", draft.strJudgementKind.empty()))
			draft =
				KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION{logic.strLogicId, logic.strDisplayName, logic.strLogicType};
		for (const char_t *const kind : KOUKU_SAYDON_JUDGEMENT_KINDS)
		{
			if (ImGui::Selectable(kind, draft.strJudgementKind == kind) && draft.strJudgementKind != kind)
			{
				draft = KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION{
					logic.strLogicId, logic.strDisplayName, logic.strLogicType};
				draft.strJudgementKind = kind;

				if ("GAZE_REAL_BOSS" == draft.strJudgementKind)
				{
					draft.fHalfAngleDegrees = 45.0;
					draft.fMaxDistanceM = 30.0;
				}
				else if ("EXTERNAL_SIGNAL" == draft.strJudgementKind || "COUNTER_WINDOW" == draft.strJudgementKind)
					draft.bEndsPatternOnSuccess = true;
				else if ("SHOWTIME_PLAYER_TARGETS" == draft.strJudgementKind)
				{
					draft.iSpawnIntervalMs = 2000u;
					draft.fFollowSpeedScale = .5;
				}
				else if ("PURSUIT_PROJECTILES" == draft.strJudgementKind)
				{
					draft.fPursuitSpeedMps = 3.0;
					draft.fContactRadiusM = .4;
					draft.fSpawnRadiusM = 2.0;
					draft.bPursuitHoming = true;
				}
				else if ("CROSS_DIRECTION_CLONES" == draft.strJudgementKind)
				{
					draft.DirectionPatternIds.resize(4u);
					draft.strCloneEndStageId = "STAGE_1";
				}
				else if ("PATTERN_COMPLETION_COUNT" == draft.strJudgementKind)
					draft.iCompletionCount = 1u;
				else if ("BINGO_COMPLETED_LINES" == draft.strJudgementKind)
					draft.iThreshold = 3u;
				else if ("STAGGER_WINDOW" == draft.strJudgementKind)
				{
					draft.iThreshold = 1000u;
					draft.fShieldArcDegrees = 90.0;
					draft.bEndsPatternOnSuccess = true;
				}
			}
		}
		ImGui::EndCombo();
	}
	if ("ROULETTE_CARD_MATCH" == draft.strJudgementKind)
	{
		Render_LogicRouletteRegionValues(draft);
	}
	else if ("OBJECT_OVERLAP" == draft.strJudgementKind)
	{
		if (ImGui::BeginCombo("Target World Object##KoukuObjectOverlap",
				draft.strTargetWorldInstanceId.empty() ? "(pick saved target)"
													   : draft.strTargetWorldInstanceId.c_str()))
		{
			for (const auto &candidate : m_WorldSequenceResources)
				if (candidate.bEnabled && !candidate.strInstanceId.empty() && !candidate.strObjectResourceId.empty() &&
					ImGui::Selectable((candidate.strDisplayName + "##" + candidate.strInstanceId).c_str(),
						draft.strTargetWorldInstanceId == candidate.strInstanceId))
					draft.strTargetWorldInstanceId = candidate.strInstanceId;
			ImGui::EndCombo();
		}
		float radius = static_cast<float>(draft.fTargetRadiusM);
		if (ImGui::InputFloat("Target radius (m)##KoukuObjectOverlap", &radius, .1f, 1.f, "%.2f"))
			draft.fTargetRadiusM = std::clamp(radius, .01f, 1000.f);
		if (ImGui::BeginCombo("Overlap outcome##KoukuObjectOverlap", draft.strInsideOutcome.c_str()))
		{
			for (const char *outcome : {"SUCCESS", "FAIL"})
				if (ImGui::Selectable(outcome, draft.strInsideOutcome == outcome))
					draft.strInsideOutcome = outcome;
			ImGui::EndCombo();
		}
		ImGui::TextWrapped(
			"The Server checks linked Colliders against this authored fixed circle every tick. First overlap runs the selected Result once; no overlap runs Timeout. Keep one static target WORLD box alive for the whole window. Animation pose does not change this circle.");
	}
	else if ("EXTERNAL_SIGNAL" == draft.strJudgementKind)
	{
		ImGui::Checkbox("End Pattern on success", &draft.bEndsPatternOnSuccess);
		ImGui::TextWrapped(
			"Waits for COMPLETE_LOGIC_WINDOW from a contact Trigger. Success runs this window's Results and cancels its Timeout. Without a signal, Timeout runs at the deadline.");
	}
	else if ("COUNTER_WINDOW" == draft.strJudgementKind)
	{
		ImGui::Checkbox("End Pattern on counter success", &draft.bEndsPatternOnSuccess);
		ImGui::TextWrapped(
			"A Server-approved counter hit during this window runs Success. Connect a FOLLOWUP_PATTERN Result to play groggy; without a counter the window runs Timeout.");
	}
	else if ("PURSUIT_PROJECTILES" == draft.strJudgementKind)
		Render_LogicPursuitValues(draft);
	else if ("SHOWTIME_PLAYER_TARGETS" == draft.strJudgementKind)
	{
		Render_LogicShowtimeValues(draft, colliderPatternId);
	}
	else if ("CROSS_DIRECTION_CLONES" == draft.strJudgementKind)
	{
		Render_LogicCrossDirectionValues(draft);
	}
	else if ("BOSS_RANDOM_TARGET" == draft.strJudgementKind)
	{
		const auto *pattern =
			Find_Pattern(m_Draft, colliderPatternId.empty() ? m_strSelectedPatternId : colliderPatternId);
		if (ImGui::BeginCombo("Target-facing Effect",
				draft.strTrackingPresentationOccurrenceId.empty() ? "(select Effect)"
																  : draft.strTrackingPresentationOccurrenceId.c_str()))
		{
			if (pattern)
				for (const auto &box : pattern->PresentationOccurrences)
				{
					const auto *resource = Find_PresentationResource(m_Draft, box.strResourceId);
					if (!resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
						box.strAnchorKind != "BOSS" || !box.bFollowBoss || !box.strBone.empty() ||
						!box.strAnchorPresentationOccurrenceId.empty())
						continue;
					const auto label = resource->strDisplayName + "##" + box.strOccurrenceId;
					if (ImGui::Selectable(
							label.c_str(), draft.strTrackingPresentationOccurrenceId == box.strOccurrenceId))
						draft.strTrackingPresentationOccurrenceId = box.strOccurrenceId;
				}
			ImGui::EndCombo();
		}
		ImGui::TextWrapped(
			"Selects one living player when this Duration starts. The head marker and selected Effect follow that target until the Duration ends. The boss body keeps its authored direction.");
	}
	else if ("BOSS_TRACK_TARGET" == draft.strJudgementKind)
	{
		Render_LogicTrackTargetValues(draft, colliderPatternId);
	}
	else if ("ATTACHMENT_HOLD" == draft.strJudgementKind)
		ImGui::TextWrapped(
			"Keeps captured players attached until this window ends. It has no Collider or outcome slots. Select this window in the capture Trigger's Hold connection.");
	else if ("INVULNERABILITY_ZONE" == draft.strJudgementKind)
	{
		int exactPlayers = static_cast<int>(draft.iThreshold);
		if (ImGui::InputInt("Exact players (0: unrestricted)##KoukuLogicValue", &exactPlayers))
			draft.iThreshold = static_cast<std::uint32_t>(std::clamp(exactPlayers, 0, 4));
		ImGui::TextWrapped(
			"Each linked Collider protects only while exactly this many living, judgeable players occupy it. Zero allows any count. Leaving or a different count removes this Pattern's protection immediately.");
		ImGui::TextWrapped(
			"Linked Colliders block instant death, fear and percentage damage from this Pattern during the window. The Server repeats the blue immunity word every two seconds. This window has no Result slots.");
	}
	else if ("AREA_OVERLAP" == draft.strJudgementKind)
	{
		if (ImGui::BeginCombo("Inside outcome", draft.strInsideOutcome.c_str()))
		{
			for (const char *outcome : {"SUCCESS", "FAIL"})
				if (ImGui::Selectable(outcome, draft.strInsideOutcome == outcome))
					draft.strInsideOutcome = outcome;
			ImGui::EndCombo();
		}
		bool pulse = draft.iRepeatIntervalMs != 0u;
		if (ImGui::Checkbox("Repeat results while touching", &pulse))
			draft.iRepeatIntervalMs = pulse ? 100u : 0u;
		if (pulse)
		{
			int interval = static_cast<int>(draft.iRepeatIntervalMs);
			if (ImGui::InputInt("Result tick interval (ms)", &interval, 10, 100))
				draft.iRepeatIntervalMs = static_cast<std::uint32_t>(std::clamp(interval, 34, int(MAX_EDITOR_TIME_MS)));
			ImGui::TextWrapped(
				"While this Duration is alive, each touching player receives the selected Result slot at this interval. Tune damage, madness and knockback in the connected Results below.");
		}
		else
			ImGui::TextDisabled("At the window end, inside runs this Result slot; outside runs Timeout.");
	}
	else if ("GAZE_REAL_BOSS" == draft.strJudgementKind)
	{
		float halfAngle = static_cast<float>(draft.fHalfAngleDegrees);
		if (ImGui::InputFloat("Half angle deg##KoukuLogicValue", &halfAngle, 1.f, 5.f, "%.1f"))
			draft.fHalfAngleDegrees = std::clamp(halfAngle, 1.f, 180.f);
		float distance = static_cast<float>(draft.fMaxDistanceM);
		if (ImGui::InputFloat("Max distance m##KoukuLogicValue", &distance, 1.f, 5.f, "%.1f"))
			draft.fMaxDistanceM = std::clamp(distance, 0.f, 1000.f);
		if (ImGui::BeginCombo("Facing boss outcome##KoukuLogicValue", draft.strInsideOutcome.c_str()))
		{
			for (const char *outcome : {"SUCCESS", "FAIL"})
				if (ImGui::Selectable(outcome, draft.strInsideOutcome == outcome))
					draft.strInsideOutcome = outcome;
			ImGui::EndCombo();
		}
		if (ImGui::Checkbox("Judge gaze during window", &draft.bGazeDuringWindow) && draft.bGazeDuringWindow)
			draft.strInsideOutcome = "FAIL";
		if (draft.bGazeDuringWindow)
		{
			draft.strInsideOutcome = "FAIL";
			ImGui::TextWrapped(
				"During this Duration, looking at the real boss runs Fail once per player. Facing uses the player's view direction. Success and Timeout are unused.");
		}
		else
			ImGui::TextDisabled(
				"At the window end, the real boss inside this cone runs the selected Result slot; outside runs the opposite slot.");
	}
	else if ("POSE_INPUT" == draft.strJudgementKind)
	{
		const std::size_t poseIndex = (std::min<std::size_t>)(draft.iPoseIndex, DANCE_POSE_LABELS.size() - 1u);
		if (ImGui::BeginCombo("Pose##KoukuLogicValue", DANCE_POSE_LABELS[poseIndex]))
		{
			for (std::size_t index = 0u; index < DANCE_POSE_LABELS.size(); ++index)
				if (ImGui::Selectable(DANCE_POSE_LABELS[index], poseIndex == index))
					draft.iPoseIndex = static_cast<std::uint32_t>(index);
			ImGui::EndCombo();
		}
		ImGui::TextDisabled("The boss pose the window shows; players answer through their shuffled Q/W/E/R slots.");
	}
	else if ("PATTERN_COMPLETION_COUNT" == draft.strJudgementKind)
	{
		int count = static_cast<int>(draft.iCompletionCount);
		if (ImGui::SliderInt("Completed patterns", &count, 1, (std::max)(1, static_cast<int>(draft.PatternIds.size()))))
			draft.iCompletionCount = static_cast<std::uint32_t>(count);
		for (const auto &pattern : m_Draft.Patterns)
		{
			auto selected = std::find(draft.PatternIds.begin(), draft.PatternIds.end(), pattern.strPatternId);
			bool included = selected != draft.PatternIds.end();
			ImGui::PushID(pattern.strPatternId.c_str());
			if (ImGui::Checkbox(pattern.strDisplayName.c_str(), &included))
			{
				if (included && draft.PatternIds.size() < 16u)
					draft.PatternIds.push_back(pattern.strPatternId);
				else if (!included)
					draft.PatternIds.erase(selected);
				draft.iCompletionCount =
					(std::min)(draft.iCompletionCount, static_cast<std::uint32_t>(draft.PatternIds.size()));
			}
			ImGui::PopID();
		}
		ImGui::TextWrapped(
			"Success fires after this many actual Server pattern completions. Duration does not advance the phase.");
	}
	else if ("BINGO_BOARD" == draft.strJudgementKind)
	{
		if (ImGui::BeginCombo("Active mode", draft.strBingoActiveMode.c_str()))
		{
			for (const char *mode : {"ENCOUNTER", "WINDOW"})
				if (ImGui::Selectable(mode, draft.strBingoActiveMode == mode))
					draft.strBingoActiveMode = mode;
			ImGui::EndCombo();
		}
		const auto input = [](const char *label, std::uint32_t &value, int minimum, int maximum) {
			int edit = static_cast<int>(value);
			if (ImGui::InputInt(label, &edit, 100, 1000))
				value = static_cast<std::uint32_t>(std::clamp(edit, minimum, maximum));
		};
		input("First bomb delay (ms)", draft.iBingoFirstBombDelayMs, 0, 600000);
		input("Bomb interval (ms)", draft.iBingoBombIntervalMs, 1, 600000);
		input("Head mark duration (ms)", draft.iBingoBombMarkMs, 1, 600000);
		input("Drop delay (ms)", draft.iBingoBombDropDelayMs, 1, 600000);
		input("Planted bomb fuse (ms)", draft.iBingoBombFuseMs, 250, 80000);
		input("Initial black cells", draft.iBingoInitialMarkedCells, 0, 25);
		ImGui::TextWrapped(
			"ENCOUNTER schedules bombs until the battle ends. WINDOW uses this Logic box's Start and Lifetime. Existing bombs finish after the window closes.");
		ImGui::TextWrapped(
			"First delay starts when the board window opens. Interval is measured between head marks; every third mark runs the special Parent. Initial cells are chosen when the board starts.");
		ImGui::TextDisabled("Mark + drop + fuse must fit within four bomb intervals.");
	}
	else if ("BINGO_COMPLETED_LINES" == draft.strJudgementKind)
	{
		int lines = static_cast<int>(draft.iThreshold);
		if (ImGui::InputInt("Completed red rows / columns", &lines))
			draft.iThreshold = static_cast<std::uint32_t>(std::clamp(lines, 1, 10));
		ImGui::TextWrapped(
			"Judges the board at the special third-bomb checkpoint. Connect Success to a Result. Separately, each newly completed unused row or column grants the Server board reward.");
	}
	else if ("STAGGER_WINDOW" == draft.strJudgementKind)
	{
		int32_t threshold = static_cast<int32_t>((std::min<std::uint32_t>)(draft.iThreshold, 2000000000u));
		if (ImGui::InputInt("Damage threshold##KoukuLogicValue", &threshold, 100, 1000))
			draft.iThreshold = static_cast<std::uint32_t>((std::max)(1, threshold));
		float arc = static_cast<float>(draft.fShieldArcDegrees);
		if (ImGui::InputFloat("Shield arc deg##KoukuLogicValue", &arc, 5.f, 15.f, "%.1f"))
			draft.fShieldArcDegrees = std::clamp(arc, 0.f, 360.f);
		float normalYaw = static_cast<float>(draft.fNormalYawOffsetDegrees);
		if (ImGui::InputFloat("Shield normal yaw offset (degrees)", &normalYaw, 1.f, 15.f))
			draft.fNormalYawOffsetDegrees = std::clamp(normalYaw, -360.f, 360.f);
		ImGui::Checkbox("Ends the Pattern on success##KoukuLogicValue", &draft.bEndsPatternOnSuccess);
		ImGui::TextDisabled(
			"Hits inside the frontal shield arc reflect to their caster; damage taken elsewhere accumulates toward the threshold.");
	}
}

// RESULT 패널의 종류와 피해·상태·후속 동작 입력을 조정한다. 기존의 독립 if 순서를 유지해 한 Result의 공통 피해 입력도 함께 표시한다.
// 하위 입력 함수가 돌려준 draft를 이곳에서 commit하지 않으며 공통 Apply와 Revert가 마지막에 한 번 처리한다.
void Client::CKoukuSaydonActionWorkbench::Render_LogicOutcomeValues(
	const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &logic, KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &draft)
{
	if (ImGui::BeginCombo("Outcome kind##KoukuLogicValue",
			draft.strOutcomeKind.empty() ? "(name only)" : draft.strOutcomeKind.c_str()))
	{
		if (ImGui::Selectable("(name only)", draft.strOutcomeKind.empty()))
			draft =
				KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION{logic.strLogicId, logic.strDisplayName, logic.strLogicType};
		for (const char_t *const kind : KOUKU_SAYDON_OUTCOME_KINDS)
		{
			if (ImGui::Selectable(kind, draft.strOutcomeKind == kind) && draft.strOutcomeKind != kind)
			{
				draft = KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION{
					logic.strLogicId, logic.strDisplayName, logic.strLogicType};
				draft.strOutcomeKind = kind;
				if ("MAX_HP_PERCENT_DAMAGE" == draft.strOutcomeKind ||
					"MADNESS_GAUGE_ADD_PERCENT" == draft.strOutcomeKind)
					draft.iPercent = 10u;
				if ("FIXED_DAMAGE" == draft.strOutcomeKind)
					draft.iDamageAmount = 500u;
				if ("FEAR" == draft.strOutcomeKind)
					draft.iDurationMs = 3000u;
				if ("PLAYER_INVULNERABILITY" == draft.strOutcomeKind)
					draft.iDurationMs = 30000u;
				if ("CAPTURE_PLAYER" == draft.strOutcomeKind)
					draft.strAttachmentSlot = "BOSS_LEFT_HAND";
			}
		}
		ImGui::EndCombo();
	}
	const bool_t percentKind =
		"MAX_HP_PERCENT_DAMAGE" == draft.strOutcomeKind || "MADNESS_GAUGE_ADD_PERCENT" == draft.strOutcomeKind;
	if (percentKind)
	{
		int32_t percent = static_cast<int32_t>(draft.iPercent);
		if (ImGui::InputInt("Percent##KoukuLogicValue", &percent, 1, 10))
			draft.iPercent = static_cast<std::uint32_t>(
				std::clamp(percent, "MADNESS_GAUGE_ADD_PERCENT" == draft.strOutcomeKind ? 0 : 1, 100));
	}
	if ("FIXED_DAMAGE" == draft.strOutcomeKind)
	{
		int amount = static_cast<int>(draft.iDamageAmount);
		if (ImGui::InputInt("Damage HP##KoukuLogicValue", &amount, 50, 500))
			draft.iDamageAmount = static_cast<std::uint32_t>(std::clamp(amount, 1, 1000000000));
	}
	if ("MAX_HP_PERCENT_DAMAGE" == draft.strOutcomeKind || "FIXED_DAMAGE" == draft.strOutcomeKind)
	{
		Render_LogicKnockbackValues(draft);
	}
	if ("CLOWN_TRANSFORM" == draft.strOutcomeKind)
	{
		int32_t durationMs = static_cast<int32_t>(draft.iDurationMs);
		if (ImGui::InputInt("Hold ms (0 = encounter policy)##KoukuLogicValue", &durationMs, 100, 1000))
			draft.iDurationMs =
				static_cast<std::uint32_t>(std::clamp(durationMs, 0, static_cast<int32_t>(MAX_EDITOR_TIME_MS)));
	}
	if ("MARIO_ENTER" == draft.strOutcomeKind)
	{
		int stage = static_cast<int>(draft.iMarioEntryStage);
		if (ImGui::SliderInt("Mario stage (0 = live room counter)##KoukuLogicValue", &stage, 0, 4))
			draft.iMarioEntryStage = static_cast<std::uint32_t>(std::clamp(stage, 0, 4));
		ImGui::TextWrapped(
			"Connect this Result alone to an ENTER_AREA Success that has a Collider box, on a Gate 3 Pattern. Play on that Pattern runs on the Server (Save, then Publish All Patterns first); walking into the Collider enters Mario stage N.");
	}
	if ("CAPTURE_PLAYER" == draft.strOutcomeKind)
	{
		ImGui::TextUnformatted("Attachment: BOSS_LEFT_HAND");
		float offset[3] = {
			float(draft.GripLocalOffset[0]), float(draft.GripLocalOffset[1]), float(draft.GripLocalOffset[2])};
		if (ImGui::InputFloat3("Grip forward / up / right (m)", offset, "%.3f"))
			for (std::size_t axis = 0u; axis < 3u; ++axis)
				draft.GripLocalOffset[axis] = std::clamp(offset[axis], -10.f, 10.f);
		ImGui::TextWrapped(
			"Connect this Result alone to ENTER_AREA Success and select that Trigger's Hold window. Zero offset uses the left-hand socket. Release happens at the Hold end, even for players captured later.");
	}
	if ("PLAYER_INVULNERABILITY" == draft.strOutcomeKind)
	{
		int duration = static_cast<int>(draft.iDurationMs);
		if (ImGui::InputInt("Invulnerability duration (ms)", &duration, 1000, 10000))
			draft.iDurationMs = static_cast<std::uint32_t>(std::clamp(duration, 1, 600000));
	}
	if ("FEAR" == draft.strOutcomeKind)
	{
		Render_LogicFearValues(draft);
	}
	if ("FOLLOWUP_PATTERN" == draft.strOutcomeKind)
	{
		if (ImGui::BeginCombo("Follow-up Pattern##KoukuLogicValue",
				draft.strFollowupPatternId.empty() ? "(pick a Pattern)" : draft.strFollowupPatternId.c_str()))
		{
			for (const KOUKU_SAYDON_COMPOSITION_PATTERN &candidate : m_Draft.Patterns)
			{
				if (!candidate.strLoadError.empty())
					continue;
				const std::string item = candidate.strDisplayName + "##" + candidate.strPatternId;
				if (ImGui::Selectable(item.c_str(), draft.strFollowupPatternId == candidate.strPatternId))
					draft.strFollowupPatternId = candidate.strPatternId;
			}
			ImGui::EndCombo();
		}
		if (!m_bSequenceWorkspace)
			ImGui::TextDisabled(
				"Counter, stagger and supported success windows may start a follow-up; Publish All Patterns validates the target.");
	}
	if ("PLAY_WORLD_OBJECT_MOTION" == draft.strOutcomeKind)
	{
		Render_LogicWorldMotionValues(draft);
	}
	if (draft.strOutcomeKind == "PLAY_CONTACT_WORLD_OBJECT_MOTION")
	{
		Render_LogicContactMotionValues(draft);
	}
	else if (draft.strOutcomeKind == "COMPLETE_LOGIC_WINDOW")
	{
		const auto *pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
		const auto *target = pattern ? Find_LogicBox(*pattern, draft.strTargetLogicOccurrenceId) : nullptr;
		const auto *targetLogic = target ? Find_Logic(m_Draft, target->strLogicId) : nullptr;
		if (ImGui::BeginCombo("Complete Duration window",
				targetLogic ? targetLogic->strDisplayName.c_str() : "Choose EXTERNAL_SIGNAL window"))
		{
			if (pattern)
				for (const auto &window : pattern->LogicOccurrences)
				{
					const auto *definition = Find_Logic(m_Draft, window.strLogicId);
					if (window.bEnabled && definition && definition->strLogicType == "DURATION" &&
						definition->strJudgementKind == "EXTERNAL_SIGNAL" &&
						ImGui::Selectable((definition->strDisplayName + "##" + window.strOccurrenceId).c_str(),
							draft.strTargetLogicOccurrenceId == window.strOccurrenceId))
						draft.strTargetLogicOccurrenceId = window.strOccurrenceId;
				}
			ImGui::EndCombo();
		}
		if (ImGui::BeginCombo("Only when this card is hit",
				draft.strContactTargetWorldOccurrenceId.empty() ? "Any selected target"
																: draft.strContactTargetWorldOccurrenceId.c_str()))
		{
			if (ImGui::Selectable("Any selected target", draft.strContactTargetWorldOccurrenceId.empty()))
				draft.strContactTargetWorldOccurrenceId.clear();
			if (pattern)
				for (const auto &worldBox : pattern->WorldOccurrences)
				{
					const auto *world = Find_World(m_Draft, worldBox.strWorldId);
					if (world &&
						ImGui::Selectable((world->strDisplayName + " | " + worldBox.strOccurrenceId).c_str(),
							draft.strContactTargetWorldOccurrenceId == worldBox.strOccurrenceId))
						draft.strContactTargetWorldOccurrenceId = worldBox.strOccurrenceId;
				}
			ImGui::EndCombo();
		}
		ImGui::TextWrapped(
			"For Joker Search, select the Joker placement here. Add this Result after the card Motion in the center Trigger's Success slot. The receiving Duration runs its own Success Results and cannot later time out.");
	}
	if ("INSTANT_DEATH" == draft.strOutcomeKind)
		ImGui::TextDisabled("Applied to each judged player; on a stagger timeout to the whole raid.");
}

// TRIGGER 패널의 종류 선택과 종류별 입력 호출을 조정한다. Sequencer 전용 종류와 Collider가 공급할 수 있는 접촉 종류의 기존 제한을 유지한다.
// collider는 호출자가 차용한 상세 선택이며 입력 중에는 보관·변경하지 않고 Apply 경계에서 연결 시간창을 함께 검증한다.
void Client::CKoukuSaydonActionWorkbench::Render_LogicTriggerValues(
	const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &logic, KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &draft,
	const std::string_view colliderPatternId, const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE *collider)
{
	const std::array<std::pair<const char *, const char *>, 21u> triggerKinds = {
		{{"", "(choose what activates this Trigger)"}, {"ANIMATION_BLEND", "Animation clip blending (ANIMATION_BLEND)"},
			{"ROOM_PLAYER_ARRIVAL", "Sequence player arrival (ROOM_PLAYER_ARRIVAL)"},
			{"ENTER_AREA", "Player contact (ENTER_AREA)"}, {"OBJECT_CONTACT", "World object contact (OBJECT_CONTACT)"},
			{"HUD_ENTER", "Switch player HUD at start (HUD_ENTER)"},
			{"CARD_RAIN_SOLDIERS", "Spawn card-rain soldiers (CARD_RAIN_SOLDIERS)"},
			{"BOSS_TRACK_TARGET", "Face nearest living player (BOSS_TRACK_TARGET)"},
			{"BOSS_RANDOM_TARGET", "Select random living target (BOSS_RANDOM_TARGET)"},
			{"BINGO_DETONATION", "Detonate Bingo black hole (BINGO_DETONATION)"},
			{"CARD_MAZE_STAGE_PLAYERS", "Place players at card maze entry Effects (CARD_MAZE_STAGE_PLAYERS)"},
			{"CARD_MAZE_HIDE_NEXT", "Hide next player from the right (CARD_MAZE_HIDE_NEXT)"},
			{"CARD_MAZE_ENTER", "Move entry participants into the card maze (CARD_MAZE_ENTER)"},
			{"ALBION_AIRBORNE", "Albion jump, player appearance and landing (ALBION_AIRBORNE)"},
			{"BOSS_TELEPORT_XZ", "Teleport boss XZ and preserve animation height (BOSS_TELEPORT_XZ)"},
			{"BOSS_TELEPORT_GROUNDED", "Teleport boss onto the destination floor"},
			{"BOSS_TELEPORT_FACE_CENTER", "Teleport boss and face arena center"},
			{"MARIO_PHASE2_PLAYERS", "Mario: place non-entrants at Iron Maiden"},
			{"REAL_GAZE_TELEPORT", "Teleport real Saydon and spawn decoys (REAL_GAZE_TELEPORT)"},
			{"ALBION_BLUE_CIRCLE", "Albion player and arena circles (ALBION_BLUE_CIRCLE)"},
			{"PURSUIT_PROJECTILES", "Spawn tracking cards once (PURSUIT_PROJECTILES)"}}};
	const auto selectedKind = std::find_if(triggerKinds.begin(), triggerKinds.end(), [&](const auto &entry) {
		return draft.strTriggerKind == entry.first;
	});
	if (ImGui::BeginCombo(
			"Trigger kind", selectedKind == triggerKinds.end() ? draft.strTriggerKind.c_str() : selectedKind->second))
	{
		for (const auto &[kind, label] : triggerKinds)
		{
			if (!m_bSequenceWorkspace && std::string_view(kind) == "ROOM_PLAYER_ARRIVAL")
				continue;
			if (collider && *kind != '\0' && std::string_view(kind) != "ENTER_AREA" &&
				std::string_view(kind) != "OBJECT_CONTACT")
				continue;
			if (ImGui::Selectable(label, draft.strTriggerKind == kind) && draft.strTriggerKind != kind)
			{
				draft = KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION{
					logic.strLogicId, logic.strDisplayName, logic.strLogicType};
				draft.strTriggerKind = kind;
				if (draft.strTriggerKind == "PURSUIT_PROJECTILES")
				{
					draft.fPursuitSpeedMps = 3.0;
					draft.fContactRadiusM = .5;
					draft.fSpawnRadiusM = 3.0;
					draft.bPursuitHoming = true;
					draft.iCountPerWave = 1u;
				}
				if (draft.strTriggerKind == "ALBION_AIRBORNE")
					draft.strAirbornePhase = "SELECT_PLAYER";
				if (draft.strTriggerKind == "HUD_ENTER")
					draft.strHudMode = "NONE";
				if (draft.strTriggerKind == "ALBION_BLUE_CIRCLE")
				{
					draft.iCountPerPlayer = 1u;
					draft.iEffectLifetimeMs = 7000u;
				}
				if (draft.strTriggerKind == "CARD_MAZE_ENTER")
					draft.TeleportPosition = {.28, -.01, 1351.65};
				if (draft.strTriggerKind == "OBJECT_CONTACT")
					draft.fTargetRadiusM = 1.0;
				if (draft.strTriggerKind == "REAL_GAZE_TELEPORT")
					draft.ClockHours = {4u, 7u, 10u};
			}
		}
		ImGui::EndCombo();
	}
	if (collider)
		ImGui::TextWrapped(
			"This Collider supplies the contact shape, position and size. Apply Values also matches the linked Trigger's start/lifetime to the Collider. HUD and teleport Triggers are placed on the Logic lane without a Collider.");
	if (draft.strTriggerKind == "PURSUIT_PROJECTILES")
		Render_LogicPursuitValues(draft);
	else if (draft.strTriggerKind == "ANIMATION_BLEND")
		ImGui::TextWrapped(
			"The Logic box defines the actual 1..1000 ms clip transition, including time before the next clip starts. Place it across exactly one consecutive pose-owner boundary. Both source ranges keep their own delay, crop, rate and hold/loop policy. Overlapping Logic/clip transitions are rejected.");
	else if (draft.strTriggerKind == "ROOM_PLAYER_ARRIVAL")
		ImGui::TextWrapped(
			"Each Logic box selects a player slot 1..4 and a destination. Play sends it once to the Server; Pause and scrub do not move players. Empty room slots are skipped.");
	else if (draft.strTriggerKind == "ENTER_AREA")
	{
		int repeat = draft.iRepeatIntervalMs ? 3 : draft.bRepeatAfterKnockback ? 2 : draft.bRearmOnExit ? 1 : 0;
		if (ImGui::Combo("Contact repetition", &repeat,
				"Once per player\0On exit and re-entry\0After knockback finishes (while inside)\0Every interval while touching\0"))
		{
			draft.bRearmOnExit = repeat == 1;
			draft.bRepeatAfterKnockback = repeat == 2;
			draft.iRepeatIntervalMs = repeat == 3 ? 500u : 0u;
		}
		if (repeat == 3)
		{
			int interval = int(draft.iRepeatIntervalMs);
			if (ImGui::InputInt("Tick interval ms##Logic", &interval, 10, 100))
				draft.iRepeatIntervalMs = std::uint32_t(std::clamp(interval, 34, int(MAX_EDITOR_TIME_MS)));
		}
		float distance = static_cast<float>(draft.fBossChargeDistanceM);
		if (ImGui::InputFloat("Charge distance (m)", &distance, 0.5f, 1.f, "%.2f"))
		{
			draft.fBossChargeDistanceM = std::clamp(distance, 0.f, 1000.f);
			if (draft.fBossChargeDistanceM == 0.0)
				draft.fChargeYawOffsetDegrees = 0.0;
		}
		if (draft.fBossChargeDistanceM > 0.0)
		{
			float yawOffset = static_cast<float>(draft.fChargeYawOffsetDegrees);
			if (ImGui::InputFloat("Charge facing offset (degrees)", &yawOffset, 5.f, 90.f, "%.1f"))
				draft.fChargeYawOffsetDegrees = std::clamp(yawOffset, -360.f, 360.f);
			ImGui::TextWrapped("Rotates the boss body without changing its captured travel direction.");
		}
		ImGui::TextWrapped(
			"A linked Collider supplies the player-contact region. Entry runs Success for each player. After knockback finishes repeats while inside, with no hit during knockback. Connect one damage Result with Knockback enabled. Exit/re-entry mode requires leaving all linked Colliders first. No entry before the window ends is Timeout. Charge distance 0 keeps the boss still; positive distance uses this Trigger window and captures its target direction once at the window start.");
	}
	else if (draft.strTriggerKind == "OBJECT_CONTACT")
	{
		Render_LogicObjectContactValues(draft);
	}
	else if (draft.strTriggerKind == "ALBION_BLUE_CIRCLE")
	{
		Render_LogicAlbionCircleValues(draft);
	}
	else if (draft.strTriggerKind == "CARD_MAZE_STAGE_PLAYERS")
	{
		Render_LogicMazeEntryValues(logic, draft);
	}
	else if (draft.strTriggerKind == "CARD_RAIN_SOLDIERS")
	{
		const char *labels[] = {"Club soldier count", "Heart soldier count", "Diamond soldier count"};
		for (std::size_t i = 0u; i < draft.SoldierCounts.size(); ++i)
		{
			int count = static_cast<int>(draft.SoldierCounts[i]);
			if (ImGui::InputInt(labels[i], &count))
				draft.SoldierCounts[i] = static_cast<std::uint32_t>(std::clamp(count, 0, 32));
		}
		int soldierMaxHp = static_cast<int>(draft.iSoldierMaxHp);
		int soldierDamage = static_cast<int>(draft.iSoldierDamage);
		if (ImGui::InputInt("Soldier HP (0 = profile)", &soldierMaxHp))
			draft.iSoldierMaxHp = static_cast<std::uint32_t>(std::clamp(soldierMaxHp, 0, 2000000000));
		if (ImGui::InputInt("Soldier fixed damage (0 = profile)", &soldierDamage))
			draft.iSoldierDamage = static_cast<std::uint32_t>(std::clamp(soldierDamage, 0, 2000000000));
		ImGui::InputDouble("Spawn radius minimum (m)", &draft.fSpawnRadiusMinM, .1, 1.0, "%.6f");
		ImGui::InputDouble("Spawn radius maximum (m)", &draft.fSpawnRadiusMaxM, .1, 1.0, "%.6f");
		ImGui::TextWrapped(
			"The Server places the requested club, heart and diamond soldiers on navigation around the current boss. Counts total 1..64; radius is 0..100 m. Existing defaults are one of each, 3..6 m. They remain until killed or their owning boss is removed. Fixed damage bypasses defense but still respects protection and shields.");
	}
	else if (draft.strTriggerKind == "BOSS_TRACK_TARGET")
		ImGui::TextWrapped(
			"When this Trigger starts, the Server faces the nearest living player immediately. Subsequent boss-anchored effects use that direction.");
	else if (draft.strTriggerKind == "BOSS_RANDOM_TARGET")
		ImGui::TextWrapped(
			"Select and face one living player once at this Trigger start. Later occurrences choose a different player when more than one is available. The selected target stays until another selection.");
	else if (draft.strTriggerKind == "BINGO_DETONATION")
		ImGui::TextWrapped(
			"The Server resolves the current Bingo board when this Trigger starts: completed-line protection, player damage and thirteen boss health bars.");
	else if (draft.strTriggerKind == "CARD_MAZE_HIDE_NEXT")
		ImGui::TextWrapped(
			"Each occurrence hides one participant. The first occurrence fixes the order by world X descending, then PlayerId. Stop restores visibility.");
	else if (draft.strTriggerKind == "CARD_MAZE_ENTER")
	{
		float position[3] = {
			float(draft.TeleportPosition[0]), float(draft.TeleportPosition[1]), float(draft.TeleportPosition[2])};
		if (ImGui::InputFloat3("Maze central position (m)", position))
			for (std::size_t axis = 0; axis < 3; ++axis)
				draft.TeleportPosition[axis] = position[axis];
		ImGui::TextWrapped(
			"The Server validates central maze navigation, moves the captured living participants together and reveals them. Q on the central telescope begins the existing maze mechanic.");
	}
	else if (draft.strTriggerKind == "HUD_ENTER")
	{
		if (ImGui::BeginCombo("HUD mode", draft.strHudMode.c_str()))
		{
			for (const char *mode : {"NONE", "POLYMORPH", "MARIO", "DANCE", "MAZE"})
				if (ImGui::Selectable(mode, draft.strHudMode == mode))
					draft.strHudMode = mode;
			ImGui::EndCombo();
		}
	}
	else if (draft.strTriggerKind == "ALBION_AIRBORNE")
	{
		Render_LogicAirborneValues(draft, colliderPatternId);
	}
	else if (draft.strTriggerKind == "MARIO_PHASE2_PLAYERS" || draft.strTriggerKind == "BOSS_TELEPORT_FACE_CENTER")
	{
		ImGui::InputScalarN(
			"Destination XYZ (m)", ImGuiDataType_Double, draft.TeleportPosition.data(), 3, nullptr, nullptr, "%.3f");
		ImGui::TextWrapped(draft.strTriggerKind == "MARIO_PHASE2_PLAYERS"
				? "Places living non-entrants 1.25 m apart. With 3-4 players, one non-entrant is bound until this pattern ends."
				: "Moves the boss to this position and faces its arena spawn center using the actor's native forward axis.");
	}
	else if (draft.strTriggerKind == "BOSS_TELEPORT_GROUNDED")
	{
		ImGui::InputScalarN("Grounded boss destination XYZ (m)", ImGuiDataType_Double, draft.TeleportPosition.data(), 3,
			nullptr, nullptr, "%.3f");
		ImGui::TextWrapped(
			"Places the boss on the destination floor. Reference Y must be within 1 m of the server floor; yaw and animation clock continue.");
	}
	else if (draft.strTriggerKind == "BOSS_TELEPORT_XZ")
	{
		double position[2] = {draft.TeleportPosition[0], draft.TeleportPosition[2]};
		if (ImGui::InputScalarN("Boss destination XZ (m)", ImGuiDataType_Double, position, 2, nullptr, nullptr, "%.3f"))
		{
			draft.TeleportPosition[0] = position[0];
			draft.TeleportPosition[2] = position[1];
		}
		ImGui::TextWrapped(
			"Moves the boss horizontally when this Logic box starts. Its current height and animation continue at the destination.");
	}
	else if (draft.strTriggerKind == "REAL_GAZE_TELEPORT")
	{
		float position[3] = {static_cast<float>(draft.TeleportPosition[0]),
			static_cast<float>(draft.TeleportPosition[1]), static_cast<float>(draft.TeleportPosition[2])};
		if (ImGui::InputFloat3("Real boss teleport position (m)", position))
			draft.TeleportPosition = {position[0], position[1], position[2]};
		float yaw = static_cast<float>(draft.fFaceCenterYawOffsetDegrees);
		if (ImGui::InputFloat("Face center yaw offset (degrees)", &yaw, 1.f, 15.f))
			draft.fFaceCenterYawOffsetDegrees = std::clamp(yaw, -360.f, 360.f);
		if (ImGui::BeginCombo("Clone Pattern",
				draft.strClonePatternId.empty() ? "(select Pattern)" : draft.strClonePatternId.c_str()))
		{
			for (const auto &pattern : m_Draft.Patterns)
				if (pattern.strLoadError.empty() &&
					ImGui::Selectable((pattern.strDisplayName + "##" + pattern.strPatternId).c_str(),
						draft.strClonePatternId == pattern.strPatternId))
					draft.strClonePatternId = pattern.strPatternId;
			ImGui::EndCombo();
		}
		if (draft.ClockHours.size() == 3u)
		{
			int hours[3] = {static_cast<int>(draft.ClockHours[0]), static_cast<int>(draft.ClockHours[1]),
				static_cast<int>(draft.ClockHours[2])};
			if (ImGui::InputInt3("Clone clock hours", hours))
				for (std::size_t i = 0u; i < 3u; ++i)
					draft.ClockHours[i] = static_cast<std::uint32_t>(std::clamp(hours[i], 1, 12));
		}
	}
}

// Logic 리소스·Logic Box·Collider·inline Outcome 상세가 공유하는 값 편집 진입점이다.
// 하위 함수는 선택한 패널 draft만 수정한다. 검증·문서 교체는 마지막 Apply가 맡고 실패하면 입력을 남긴다.
// Apply가 문서와 inline draft를 교체할 수 있으므로 그 뒤에는 빌린 logic·draft·collider를 다시 읽지 않는다.
void Client::CKoukuSaydonActionWorkbench::Render_LogicDefinitionValues(
	const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &logic, const std::string_view colliderPatternId,
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE *collider, const bool_t inlineDetail)
{
	// 같은 프레임에 여러 inline Result가 열려도 stable Logic ID별로 미적용 입력을 유지한다.
	// 일반·Collider·inline 패널의 draft 수명은 이 진입점이 선택하고 Apply 또는 Revert에서만 정리한다.
	std::string inlineDraftId = logic.strLogicId;
	auto &draftId = inlineDetail ? inlineDraftId : collider ? m_strColliderLogicValueDraftId : m_strLogicValueDraftId;
	// try_emplace는 처음 열린 ID만 원본으로 초기화해 다음 프레임의 미적용 입력을 덮어쓰지 않는다. draft는 선택한 저장소의 값 참조다. Apply 뒤 Revert 구간은 ID만 사용하고 이 draft 값을 다시 읽지 않는다.
	auto &draft = inlineDetail ? m_InlineLogicValueDrafts.try_emplace(logic.strLogicId, logic).first->second
		: collider			   ? m_ColliderLogicValueDraft
							   : m_LogicValueDraft;
	if (draftId != logic.strLogicId)
	{
		draft = logic;
		draftId = logic.strLogicId;
	}
	ImGui::SeparatorText("DURATION" == logic.strLogicType ? "Judgement"
			: "TRIGGER" == logic.strLogicType			  ? "Trigger"
														  : "Outcome");
	if ("DURATION" == logic.strLogicType)
	{
		Render_LogicJudgementValues(logic, draft, colliderPatternId);
	}
	else if ("RESULT" == logic.strLogicType)
	{
		Render_LogicOutcomeValues(logic, draft);
	}
	else if ("TRIGGER" == logic.strLogicType)
	{
		Render_LogicTriggerValues(logic, draft, colliderPatternId, collider);
	}
	if (ImGui::Button("Apply Values##KoukuLogicValue"))
	{
		std::string status;
		if (collider)
			(void)Set_ColliderLogicValues(colliderPatternId, *collider, draft, status);
		else
			(void)Set_LogicDefinitionValues(logic.strLogicId, draft, status);
	}
	ImGui::SameLine();
	if (ImGui::Button("Revert##KoukuLogicValue"))
	{
		if (inlineDetail)
			m_InlineLogicValueDrafts.erase(inlineDraftId);
		else
			draftId.clear();
	}
}

// Resources의 Logic 탭이다. 정의 목록·생성·선택·값 편집·Pattern 배치를 이어 주며 이 화면의 선택은 timeline occurrence와 구별한다.
// 문서를 교체할 수 있는 동작 뒤에는 차용 포인터를 다시 찾거나 현재 그리기를 끝내고 다음 프레임에서 새 문서를 읽는다.
void Client::CKoukuSaydonActionWorkbench::Render_LogicResources()
{
	ImGui::SeparatorText("Logic Catalog");
	ImGui::TextDisabled("%zu logics | Create names a reusable Logic; Append places its box on the selected Pattern.",
		m_Draft.Logics.size());
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	const KOUKU_SAYDON_COMPOSITION_PATTERN *const selectedPattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	const bool_t patternEditable = nullptr != selectedPattern && selectedPattern->strLoadError.empty();
	const std::string targetPatternName =
		nullptr == selectedPattern ? std::string("none") : selectedPattern->strDisplayName;

	if (ImGui::BeginChild("##KoukuLogicList", ImVec2(0.f, 540.f), ImGuiChildFlags_Borders))
	{
		if (m_Draft.Logics.empty())
			ImGui::TextDisabled("No Logic yet. Choose a type, name it, and press Create Logic.");
		for (const char_t *const type : KOUKU_SAYDON_LOGIC_TYPES)
		{
			if (!ImGui::TreeNodeEx(type, ImGuiTreeNodeFlags_DefaultOpen))
				continue;
			for (const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &logic : m_Draft.Logics)
			{
				if (logic.strLogicType != type)
					continue;
				const std::string label = logic.strDisplayName + "##" + logic.strLogicId;
				if (ImGui::Selectable(label.c_str(), m_strSelectedLogicId == logic.strLogicId))
					m_strSelectedLogicId = logic.strLogicId;
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s | %s | %zu box(es)", logic.strLogicId.c_str(), logic.strLogicType.c_str(),
						Count_LogicReferences(m_Draft, logic.strLogicId));
			}
			ImGui::TreePop();
		}
	}
	ImGui::EndChild();

	ImGui::SeparatorText("Create Logic");
	ImGui::TextUnformatted("Type");
	for (int32_t index = 0; index < static_cast<int32_t>(KOUKU_SAYDON_LOGIC_TYPES.size()); ++index)
	{
		if (0 != index)
			ImGui::SameLine();
		if (ImGui::RadioButton(KOUKU_SAYDON_LOGIC_TYPES[index], m_iNewLogicType == index))
			m_iNewLogicType = index;
	}
	ImGui::InputTextWithHint("##NewKoukuLogicName", "Logic display name", m_NewLogicName, std::size(m_NewLogicName));
	ImGui::SameLine();
	ImGui::BeginDisabled('\0' == m_NewLogicName[0] || !m_bHasDraft);
	if (ImGui::Button("Create Logic"))
	{
		std::string logicId;
		std::string status;
		if (Create_Logic(m_NewLogicName, KOUKU_SAYDON_LOGIC_TYPES[m_iNewLogicType], logicId, status))
			m_NewLogicName[0] = '\0';
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();

	const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION *const selectedLogic = Find_Logic(m_Draft, m_strSelectedLogicId);
	if (nullptr == selectedLogic)
		return;
	// 아래 Rename·Apply가 m_Draft를 교체해도 ID와 표시 문자열은 계속 필요하므로 값으로 복사한다.
	const std::string logicId = selectedLogic->strLogicId;
	const std::string logicName = selectedLogic->strDisplayName;
	const std::string logicType = selectedLogic->strLogicType;
	bool_t unresolvedReferences = false;
	const std::size_t references = Count_LogicReferences(m_Draft, logicId, &unresolvedReferences);
	ImGui::SeparatorText("Selected Logic");
	ImGui::TextWrapped("%s", logicName.c_str());
	if (Render_RenameControl(RENAME_TARGET::LOGIC, logicId, logicName))
		return;
	ImGui::TextDisabled("%s | %s | %zu box(es)", logicId.c_str(), logicType.c_str(), references);
	Render_LogicDefinitionValues(*selectedLogic);
	if (nullptr == Find_Logic(m_Draft, logicId))
		return;
	ImGui::SeparatorText("Place");
	ImGui::SetNextItemWidth(120.f);
	ImGui::InputInt("Box ms##NewKoukuLogicBox", &m_iNewLogicBoxDurationMs, 100, 1000);
	m_iNewLogicBoxDurationMs = std::clamp(m_iNewLogicBoxDurationMs, 1, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	ImGui::SameLine();
	ImGui::BeginDisabled(!patternEditable);
	if (ImGui::Button("Append Logic at Cursor"))
	{
		std::string occurrenceId;
		std::string status;
		(void)Append_LogicBox(m_strSelectedPatternId, logicId, m_iCursorMs,
			static_cast<std::uint32_t>(m_iNewLogicBoxDurationMs), occurrenceId, status);
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(0u != references || unresolvedReferences);
	if (ImGui::Button("Delete Logic"))
	{
		std::string status;
		(void)Delete_Logic(logicId, status);
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();
	if (unresolvedReferences)
		ImGui::TextDisabled("Repair invalid Pattern Logic references before deleting a Logic.");
	if (!patternEditable)
		ImGui::TextDisabled("Select an editable Pattern in Patterns to append this Logic.");
	ImGui::TextDisabled("Append target: %s | cursor %u ms", targetPatternName.c_str(), m_iCursorMs);
}

// Logic Box와 Collider 상세가 공유하는 Hold·Outcome 연결 화면이다. slot의 ID 목록을 값으로 복사해 편집한 뒤 typed setter로 제출한다.
// true는 문서 변경을 시도했거나 inline Apply로 generation이 바뀌어 호출자가 현재 차용 참조의 사용을 멈춰야 한다는 뜻이다.
bool_t Client::CKoukuSaydonActionWorkbench::Render_LogicOutcomeSlots(
	const std::string &patternId, const std::string &occurrenceId)
{
	const auto *pattern = Find_Pattern(m_Draft, patternId);
	const auto *box = nullptr == pattern ? nullptr : Find_LogicBox(*pattern, occurrenceId);
	if (nullptr == box)
		return false;
	const auto *logic = Find_Logic(m_Draft, box->strLogicId);
	if (logic && logic->strJudgementKind == "ATTACHMENT_HOLD")
		ImGui::TextWrapped(
			"Attachment Hold ends at %u ms. It owns the release deadline and has no Collider or outcome slots.",
			box->iStartMs + box->iDurationMs);
	if (logic && logic->strTriggerKind == "ENTER_AREA")
	{
		const auto *hold = Find_LogicBox(*pattern, box->strHoldLogicOccurrenceId);
		const auto *holdLogic = hold ? Find_Logic(m_Draft, hold->strLogicId) : nullptr;
		std::string selectedHold = box->strHoldLogicOccurrenceId;
		bool changed = false;
		if (ImGui::BeginCombo("Capture Hold window", holdLogic ? holdLogic->strDisplayName.c_str() : "(none)"))
		{
			if (ImGui::Selectable("(none)", selectedHold.empty()))
			{
				selectedHold.clear();
				changed = true;
			}
			for (const auto &candidate : pattern->LogicOccurrences)
			{
				const auto *definition = Find_Logic(m_Draft, candidate.strLogicId);
				if (definition && definition->strLogicType == "DURATION" &&
					definition->strJudgementKind == "ATTACHMENT_HOLD" &&
					ImGui::Selectable((definition->strDisplayName + " | " + std::to_string(candidate.iStartMs) + ".." +
										  std::to_string(candidate.iStartMs + candidate.iDurationMs) + " ms##" +
										  candidate.strOccurrenceId)
										  .c_str(),
						selectedHold == candidate.strOccurrenceId))
				{
					selectedHold = candidate.strOccurrenceId;
					changed = true;
				}
			}
			ImGui::EndCombo();
		}
		if (changed)
		{
			std::string status;
			(void)Set_LogicBoxHold(patternId, occurrenceId, selectedHold, status);
			return true;
		}
	}
	if (nullptr != logic && Kouku_LogicOwnsOutcomes(*logic))
	{
		// 각 slot의 RESULT ID 순서는 Server 적용 순서다. 최대 네 개를 유지하며 현재 판정 종류가 도달할 수 없는 slot은 비활성화한다.
		// gaze의 during-window 여부까지 기존 허용 검사에 넘겨 실제 가능한 결과만 연결하게 한다.
		ImGui::SeparatorText("Outcomes");
		if (Kouku_LogicOutcomeKind(*logic).empty())
			ImGui::TextColored(ImVec4(1.f, 0.72f, 0.24f, 1.f),
				"Give this Logic a judgement kind in Resources > Logic before this Pattern can run on the Server.");
		else
			ImGui::TextDisabled("%s", Kouku_LogicOutcomeKind(*logic).c_str());
		const bool_t hasResult = m_Draft.Logics.end() !=
			std::find_if(m_Draft.Logics.begin(), m_Draft.Logics.end(),
				[](const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &candidate) {
					return "RESULT" == candidate.strLogicType;
				});
		for (const KOUKU_SAYDON_OUTCOME_SLOT slot :
			{KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS, KOUKU_SAYDON_OUTCOME_SLOT::FAIL, KOUKU_SAYDON_OUTCOME_SLOT::TIMEOUT})
		{
			const bool_t allowed = Kouku_LogicOutcomeKind(*logic).empty() ||
				Kouku_IsOutcomeSlotAllowed(Kouku_LogicOutcomeKind(*logic), slot, logic->bGazeDuringWindow);
			// Combo와 삭제는 임시 목록에서 처리한다. 한 행이 바뀌면 순회를 마치고 setter 한 번으로 순서까지 검증한다.
			std::vector<std::string> targets = box->Outcomes(slot);
			ImGui::PushID(static_cast<int32_t>(slot));
			ImGui::BeginDisabled(!allowed);
			ImGui::Text("%s%s", Outcome_SlotLabel(slot), allowed ? "" : " (not an outcome of this kind)");
			bool_t changed = false;
			for (std::size_t index = 0u; index < targets.size(); ++index)
			{
				ImGui::PushID(static_cast<int32_t>(index));
				const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION *const current = Find_Logic(m_Draft, targets[index]);
				ImGui::SetNextItemWidth(220.f);
				if (ImGui::BeginCombo(
						"##KoukuOutcome", nullptr == current ? "(missing RESULT)" : current->strDisplayName.c_str()))
				{
					for (const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &candidate : m_Draft.Logics)
					{
						if ("RESULT" != candidate.strLogicType)
							continue;
						const std::string item = candidate.strDisplayName + "##" + candidate.strLogicId;
						if (ImGui::Selectable(item.c_str(), targets[index] == candidate.strLogicId) &&
							targets[index] != candidate.strLogicId)
						{
							targets[index] = candidate.strLogicId;
							changed = true;
						}
					}
					ImGui::EndCombo();
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("x##KoukuOutcomeRemove"))
				{
					targets.erase(targets.begin() + static_cast<std::ptrdiff_t>(index));
					changed = true;
				}
				if (!changed && current && ImGui::TreeNodeEx("Result values", ImGuiTreeNodeFlags_DefaultOpen))
				{
					// inline Apply는 공유 문서를 교체할 수 있다. 정의를 복사하고 복귀 후 세대를 비교한 뒤 모든 열린 UI 범위를 닫는다.
					const auto definition = *current;
					const auto generation = m_iDraftGeneration;
					ImGui::TextWrapped("Shared Result: edits apply to every box connected to this definition.");
					Render_LogicDefinitionValues(definition, patternId, nullptr, true);
					ImGui::TreePop();
					if (generation != m_iDraftGeneration)
					{
						ImGui::PopID();
						ImGui::EndDisabled();
						ImGui::PopID();
						return true;
					}
				}
				ImGui::PopID();
				if (changed)
					break;
			}
			if (!changed && targets.size() < KOUKU_SAYDON_MAX_OUTCOMES_PER_SLOT)
			{
				ImGui::SetNextItemWidth(220.f);
				ImGui::BeginDisabled(!hasResult);
				if (ImGui::BeginCombo("##KoukuOutcomeAdd", "+ add RESULT"))
				{
					for (const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &candidate : m_Draft.Logics)
					{
						if ("RESULT" != candidate.strLogicType)
							continue;
						const std::string item = candidate.strDisplayName + "##" + candidate.strLogicId;
						if (ImGui::Selectable(item.c_str(), false))
						{
							targets.push_back(candidate.strLogicId);
							changed = true;
						}
					}
					ImGui::EndCombo();
				}
				ImGui::EndDisabled();
			}
			ImGui::EndDisabled();
			ImGui::PopID();
			if (changed)
			{
				std::string status;
				(void)Set_LogicBoxOutcomes(patternId, occurrenceId, slot, targets, status);
				return true;
			}
		}
		if (!hasResult)
			ImGui::TextDisabled("Create a RESULT Logic in Resources > Logic to wire an outcome.");
	}
	else if (nullptr != logic)
	{
		ImGui::TextDisabled("%s boxes carry no outcomes; only Duration or Collider Trigger owns result slots.",
			logic->strLogicType.c_str());
	}
	return false;
}

// Details dispatcher가 선택 Pattern을 차용해 부른다. occurrence 활성화·시간창·도착 슬롯·공유 정의·Outcome 연결을 편집한다.
// ID를 먼저 복사하고 commit 시도 또는 generation 변화 뒤에는 즉시 돌아가 m_Draft 교체 전 포인터를 다시 사용하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Render_LogicBoxDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern)
{
	const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE *const box = Find_LogicBox(pattern, m_strSelectedLogicOccurrenceId);
	if (nullptr == box)
		return;
	const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION *const logic = Find_Logic(m_Draft, box->strLogicId);
	const std::string patternId = pattern.strPatternId;
	const std::string occurrenceId = box->strOccurrenceId;
	const std::uint32_t lifetimeMs = Pattern_DurationMs(pattern);
	ImGui::SeparatorText("Logic Box");
	ImGui::Text("%s", occurrenceId.c_str());
	ImGui::Text("%s | %s", nullptr == logic ? "(missing Logic)" : logic->strDisplayName.c_str(),
		nullptr == logic ? "?" : logic->strLogicType.c_str());
	ImGui::TextDisabled("%s", box->strLogicId.c_str());
	bool_t enabled = box->bEnabled;
	if (ImGui::Checkbox("Enabled in gameplay##KoukuLogicBox", &enabled))
	{
		auto candidate = m_Draft;
		auto *editablePattern = Find_Pattern(candidate, patternId);
		auto *editableBox = editablePattern ? Find_LogicBox(*editablePattern, occurrenceId) : nullptr;
		if (editableBox)
		{
			editableBox->bEnabled = enabled;
			std::string status;
			(void)Commit_Candidate(std::move(candidate), "Updated Logic gameplay enable state.", status);
		}
		return;
	}
	ImGui::InputInt("Start ms##KoukuLogicBox", &m_iLogicBoxStartMs, 10, 100);
	ImGui::InputInt("Lifetime ms##KoukuLogicBox", &m_iLogicBoxDurationMs, 10, 100);
	m_iLogicBoxStartMs = std::clamp(m_iLogicBoxStartMs, 0, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	m_iLogicBoxDurationMs = std::clamp(m_iLogicBoxDurationMs, 1, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	if (ImGui::Button("Apply Window##KoukuLogicBox"))
	{
		std::string status;
		(void)Set_LogicBoxWindow(patternId, occurrenceId, static_cast<std::uint32_t>(m_iLogicBoxStartMs),
			static_cast<std::uint32_t>(m_iLogicBoxDurationMs), status);
		return;
	}
	ImGui::TextDisabled("Pattern lifetime %u ms. Drag the box edges on the Logic lane to resize it.", lifetimeMs);
	if (logic && logic->strTriggerKind == "ROOM_PLAYER_ARRIVAL" && box->RoomPlayerArrival)
	{
		auto arrival = *box->RoomPlayerArrival;
		// 화면은 1..4, 저장은 0..3인 슬롯 번호다. 좌표와 함께 후보에 복사한 뒤 공통 검증을 통과해야 반영된다.
		int slot = static_cast<int>(arrival.iPlayerSlot) + 1;
		float position[3] = {float(arrival.Position[0]), float(arrival.Position[1]), float(arrival.Position[2])};
		bool changed = ImGui::InputInt("Player slot (1..4)", &slot);
		changed = ImGui::InputFloat3("Arrival position (m)", position) || changed;
		if (changed)
		{
			arrival.iPlayerSlot = static_cast<uint32_t>(std::clamp(slot, 1, 4) - 1);
			arrival.Position = {position[0], position[1], position[2]};
			auto candidate = m_Draft;
			auto *editablePattern = Find_Pattern(candidate, patternId);
			auto *editableBox = editablePattern ? Find_LogicBox(*editablePattern, occurrenceId) : nullptr;
			if (editableBox)
			{
				editableBox->RoomPlayerArrival = arrival;
				std::string status;
				(void)Commit_Candidate(
					std::move(candidate), "Updated player arrival. Save Sequence to retain it.", status);
			}
			return;
		}
		ImGui::TextWrapped(
			"PlayerId order is fixed at this run's first arrival. Missing players are skipped. The Server validates the destination.");
	}
	else
	{
		if (logic && ImGui::TreeNodeEx("Logic values", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const auto definition = *logic;
			const auto generation = m_iDraftGeneration;
			ImGui::TextWrapped(
				"These are the current shared definition values. Apply updates every box using this Logic; Save keeps the change.");
			Render_LogicDefinitionValues(definition, patternId, nullptr, true);
			ImGui::TreePop();
			if (generation != m_iDraftGeneration)
				return;
		}
		if (Render_LogicOutcomeSlots(patternId, occurrenceId))
			return;
	}
	if (ImGui::Button("Delete Logic Box"))
	{
		std::string status;
		(void)Delete_LogicBox(patternId, occurrenceId, status);
	}
}

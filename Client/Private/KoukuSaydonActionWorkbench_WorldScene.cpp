// World occurrence와 Scene profile의 저작 입력을 담당한다. 배치된 Object의 stable ID와 부속 표현 참조를 문서 후보에 함께 보존한다.
#include "imgui.h"
#include "KoukuSaydonActionWorkbench.h"
#include "KoukuSaydonActionWorkbench_Internal.h"

// 이 도메인에서 직접 사용하는 런타임/저작 자료의 선언이다.
#include "DataJson.h"

// 후보 값 복사, stable ID 조회, 시간/좌표 계산에 필요한 표준 타입이다.
#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <utility>
#include <optional>
#include <string>
#include <string_view>

using namespace Client::KoukuWorkbenchDetail;

namespace
{
using namespace Client;

/* Summon과 같은 규칙으로 typed 박스와 격리 Pattern에 보존된 원문을 함께 확인한다.
	   인자의 lane 박스 목록과 참조 key에 해당하는 사용 관계를 수집한다. */
// definitionId와 JSON key는 호출 동안 읽기만 하는 입력이다. outUnresolved는 선택적 출력이므로 nullptr을 허용하며, 조회 불가능을 참조 0개와 구분해 삭제 판단에 넘긴다.
std::size_t Count_LaneReferences(const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const std::string_view definitionId,
	const char_t *const occurrencesKey, const char_t *const referenceKey, bool_t *const outUnresolved)
{
	std::size_t count = 0u;
	bool_t unresolved = false;
	for (const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern : document.Patterns)
	{
		if (pattern.strLoadError.empty())
		{
			if (std::string_view("worldOccurrences") == occurrencesKey)
			{
				for (const auto &box : pattern.WorldOccurrences)
					if (box.strWorldId == definitionId)
						++count;
			}
			else
			{
				for (const auto &box : pattern.SceneProfileOccurrences)
					if (box.strSceneProfileId == definitionId)
						++count;
			}
			continue;
		}
		DATA_JSON_VALUE preserved;
		std::string error;
		if (!CDataJson::Parse(pattern.strPreservedJson, preserved, error) || !preserved.Is_Object())
		{
			unresolved = true;
			continue;
		}
		const auto *boxes = preserved.Find(occurrencesKey);
		if (nullptr == boxes)
			continue;
		if (!boxes->Is_Array())
		{
			unresolved = true;
			continue;
		}
		for (const auto &box : boxes->Get_Array())
		{
			const auto *reference = box.Find(referenceKey);
			if (nullptr == reference || !reference->Is_String())
				unresolved = true;
			else if (reference->Get_String() == definitionId)
				++count;
		}
	}
	if (nullptr != outUnresolved)
		*outUnresolved = unresolved;
	return count;
}

std::size_t Count_WorldReferences(const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const std::string_view worldId,
	bool_t *const outUnresolved = nullptr)
{
	return Count_LaneReferences(document, worldId, "worldOccurrences", "worldId", outUnresolved);
}

std::size_t Count_SceneProfileReferences(const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document,
	const std::string_view sceneProfileId, bool_t *const outUnresolved = nullptr)
{
	bool_t unresolved = false;
	auto count =
		Count_LaneReferences(document, sceneProfileId, "sceneProfileOccurrences", "sceneProfileId", &unresolved);
	for (const auto &bundle : document.Bundles)
	{
		if (bundle.strLoadError.empty())
		{
			for (const auto &box : bundle.SceneProfileOccurrences)
				if (box.strSceneProfileId == sceneProfileId)
					++count;
			continue;
		}
		DATA_JSON_VALUE preserved;
		std::string error;
		if (!CDataJson::Parse(bundle.strPreservedJson, preserved, error) || !preserved.Is_Object())
		{
			unresolved = true;
			continue;
		}
		const auto *boxes = preserved.Find("sceneProfileOccurrences");
		if (!boxes)
			continue;
		if (!boxes->Is_Array())
		{
			unresolved = true;
			continue;
		}
		for (const auto &box : boxes->Get_Array())
		{
			const auto *reference = box.Find("sceneProfileId");
			if (!reference || !reference->Is_String())
				unresolved = true;
			else if (reference->Get_String() == sceneProfileId)
				++count;
		}
	}
	if (outUnresolved)
		*outUnresolved = unresolved;
	return count;
}

bool_t Append_WorldCompanionEffect(KOUKU_SAYDON_COMPOSITION_PATTERN &pattern,
	const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION &world, const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE &owner,
	std::string &status)
{
	if (world.strCompanionEffectResourceId.empty())
		return true;
	if (pattern.iNextPresentationOccurrenceOrdinal >= 1000000u ||
		static_cast<std::uint64_t>(owner.iStartMs) + owner.iDurationMs > Pattern_DurationMs(pattern))
	{
		status = "World companion needs an available Effect box ID and a window inside the Pattern lifetime.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE effect;
	effect.strOccurrenceId =
		pattern.strPatternId + ".presentation." + std::to_string(pattern.iNextPresentationOccurrenceOrdinal++);
	effect.strResourceId = world.strCompanionEffectResourceId;
	effect.strWorldOccurrenceId = owner.strOccurrenceId;
	effect.iStartMs = owner.iStartMs;
	effect.iDurationMs = owner.iDurationMs;
	pattern.PresentationOccurrences.push_back(std::move(effect));
	return true;
}
} // 이 파일의 보조 구현 범위 끝

// 새 World occurrence의 배치 후보를 구한다. 상대 anchor는 기본 offset을 쓰고 독립 배치는 연결된 resolver가 반환한 현재 character 위치를 사용한다.
bool_t Client::CKoukuSaydonActionWorkbench::Stage_NewWorldPlacement(const std::string_view instanceId,
	std::optional<KOUKU_SAYDON_WORLD_PLACEMENT> &outPlacement, std::string &outStatus) const
{
	outPlacement.reset();
	const auto source =
		std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(), [&](const auto &row) {
			return row.strInstanceId == instanceId;
		});
	if (source == m_WorldSequenceResources.end() || !source->bSupportsPlacement)
		return true;
	if (source->strAnchorKind == "BOSS" || source->strAnchorKind == "PLAYER")
	{
		outPlacement = KOUKU_SAYDON_WORLD_PLACEMENT{};
		return true;
	}
	if (!source->bEnabled || !m_WorldPlacementResolver)
	{
		outStatus = "A live character placement is required to append this Object.";
		return false;
	}
	KOUKU_SAYDON_WORLD_PLACEMENT placement;
	if (!m_WorldPlacementResolver(placement, outStatus))
		return false;
	outPlacement = placement;
	return true;
}

// 외부 Object 편집 화면도 같은 Pattern World 행을 수정하는 진입점이다. m_Draft가 없으면 로드하고 변경은 공통 Set_WorldBoxPlacement/Save 경로로 보낸다.
void Client::CKoukuSaydonActionWorkbench::Render_WorldPlacementTuning(
	const std::string_view patternId, const std::string_view occurrenceId)
{
	if (!m_bHasDraft)
	{
		std::string status;
		if (!Reload(status))
		{
			ImGui::TextWrapped("%s", status.c_str());
			return;
		}
	}
	const auto *pattern = Find_Pattern(m_Draft, patternId);
	const auto *box = pattern ? Find_WorldBox(*pattern, occurrenceId) : nullptr;
	if (!box)
	{
		ImGui::TextDisabled("Saved Pattern World box is unavailable.");
		return;
	}
	auto placement = box->Placement.value_or(KOUKU_SAYDON_WORLD_PLACEMENT{});
	const auto edit = [](const char *label, std::array<double, 3u> &values, float step, float minimum, float maximum) {
		float v[3] = {float(values[0]), float(values[1]), float(values[2])};
		if (!ImGui::DragFloat3(label, v, step, minimum, maximum, "%.3f"))
			return false;
		for (size_t i = 0; i < 3u; ++i)
			values[i] = v[i];
		return true;
	};
	ImGui::BeginDisabled(Is_PublishRunning() || m_bServerPlayPreparationPending);
	bool changed = edit("Local position (m)", placement.Position, .01f, -100000.f, 100000.f);
	changed |= edit("Rotation (degrees)", placement.RotationDegrees, .5f, -36000.f, 36000.f);
	changed |= edit("Scale", placement.Scale, .01f, .001f, 1000.f);
	std::string status;
	if (changed)
		(void)Set_WorldBoxPlacement(patternId, occurrenceId, placement, status);
	if (ImGui::Button("Preview placement"))
		Queue_WorldBoxPreview(patternId, occurrenceId);
	ImGui::SameLine();
	if (ImGui::Button("Save Pattern changes"))
		(void)Save(status);
	ImGui::EndDisabled();
	ImGui::TextWrapped(
		"This edits the same Pattern World box as Action Workbench. The saved hand Motion stays separate. Save includes other Pattern draft changes.");
	ImGui::TextWrapped("%s", m_strStatus.c_str());
}

// stable Pattern/occurrence ID로 문서 사본을 찾아 Transform을 갱신한다. 검증/commit 성공 후에만 새 배치 preview를 요청한다.
bool_t Client::CKoukuSaydonActionWorkbench::Set_WorldBoxPlacement(const std::string_view patternId,
	const std::string_view occurrenceId, const KOUKU_SAYDON_WORLD_PLACEMENT &placement, std::string &outStatus)
{
	auto candidate = m_Draft;
	auto *pattern = Find_Pattern(candidate, patternId);
	if (!pattern || !pattern->strLoadError.empty())
	{
		outStatus = m_strStatus = "Select an editable Pattern for this Object placement.";
		return false;
	}
	const auto found =
		std::find_if(pattern->WorldOccurrences.begin(), pattern->WorldOccurrences.end(), [&](const auto &row) {
			return row.strOccurrenceId == occurrenceId;
		});
	if (found == pattern->WorldOccurrences.end())
	{
		outStatus = m_strStatus = "The World occurrence no longer exists.";
		return false;
	}
	if (found->Placement && *found->Placement == placement)
	{
		outStatus = "World occurrence placement is unchanged.";
		return true;
	}
	found->Placement = placement;
	if (!Commit_Candidate(
			std::move(candidate), "Updated this Object box's Transform. Save keeps this placement.", outStatus))
		return false;
	Queue_WorldBoxPreview(patternId, occurrenceId);
	return true;
}

// 독립 WORLD 배치는 character resolver의 위치를 쓰고 BOSS/PLAYER anchor는 기본 상대 offset을 쓴다. 기존 occurrence의 회전/scale은 유지하며 배치 override 미지원 원본은 변경하지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Place_WorldBoxNearCharacter(
	const std::string_view patternId, const std::string_view occurrenceId, std::string &outStatus)
{
	const auto *pattern = Find_Pattern(m_Draft, patternId);
	const auto *box = pattern ? Find_WorldBox(*pattern, occurrenceId) : nullptr;
	const auto *world = box ? Find_World(m_Draft, box->strWorldId) : nullptr;
	if (!world)
	{
		outStatus = m_strStatus = "Select an existing World box to place near the character.";
		return false;
	}
	std::optional<KOUKU_SAYDON_WORLD_PLACEMENT> placement;
	if (!Stage_NewWorldPlacement(world->strSequenceInstanceId, placement, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	if (!placement)
	{
		outStatus = m_strStatus =
			"Independent placement requires a WORLD Object resource; existing map bindings retain their saved placement.";
		return false;
	}
	if (box->Placement)
	{
		placement->RotationDegrees = box->Placement->RotationDegrees;
		placement->Scale = box->Placement->Scale;
	}
	return Set_WorldBoxPlacement(patternId, occurrenceId, *placement, outStatus);
}

// Object의 저장된 기본 animation을 찾아 World resource append로 연결한다. 기본 motion이 없거나 비활성이면 문서를 변경하지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Append_WorldObject(
	const std::string_view objectResourceId, std::string &outStatus)
{
	const auto source =
		std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(), [&](const auto &row) {
			return row.strObjectResourceId == objectResourceId && row.bDefaultMotion;
		});
	if (objectResourceId.empty() || source == m_WorldSequenceResources.end() || !source->bEnabled ||
		source->strInstanceId.empty())
	{
		outStatus = m_strStatus = "Set and Save this Object's default animation in Object Tool before Append.";
		return false;
	}
	if (!source->strAppendGroupObjectId.empty())
		return Append_WorldObject(source->strAppendGroupObjectId, outStatus);
	return Append_WorldResource(source->strInstanceId, outStatus, true);
}

// 저장된 sequence를 World 정의/박스로 등록한다. 배치 alias와 companion Effect 참조도 후보에서 함께 구성하여 원본 표현을 잃지 않게 한다.
bool_t Client::CKoukuSaydonActionWorkbench::Append_WorldResource(
	const std::string_view instanceId, std::string &outStatus, const bool_t asObject)
{
	const auto source =
		std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(), [instanceId](const auto &row) {
			return row.strInstanceId == instanceId;
		});
	if (source == m_WorldSequenceResources.end() || source->strInstanceId.empty() || !source->bEnabled ||
		(asObject && (!source->bDefaultMotion || source->strObjectResourceId.empty())))
	{
		outStatus = m_strStatus = "Select an Object with an enabled saved default animation first.";
		return false;
	}
	if (asObject && !source->strAppendGroupObjectId.empty())
		return Append_WorldObject(source->strAppendGroupObjectId, outStatus);
	auto candidate = m_Draft;
	auto *pattern = Find_Pattern(candidate, m_strSelectedPatternId);
	if (!m_bHasDraft || !pattern || !pattern->strLoadError.empty() || Pattern_DurationMs(*pattern) == 0u ||
		pattern->iNextWorldOccurrenceOrdinal >= 1000000u)
	{
		outStatus = m_strStatus = "Append needs an editable Pattern with an animation lifetime.";
		return false;
	}
	auto world = std::find_if(candidate.Worlds.begin(), candidate.Worlds.end(), [&](const auto &row) {
		return row.strSequenceInstanceId == instanceId &&
			(asObject ? row.strObjectResourceId == source->strObjectResourceId : row.strObjectResourceId.empty());
	});
	if (world == candidate.Worlds.end())
	{
		if (candidate.iNextWorldOrdinal >= 1000000u)
		{
			outStatus = m_strStatus = "World resource stable ID ordinals are exhausted.";
			return false;
		}
		KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION value;
		value.strWorldId = "koukusaydon.g1.world." + std::to_string(candidate.iNextWorldOrdinal++);
		value.strDisplayName = asObject ? source->strObjectDisplayName : source->strDisplayName;
		value.strSequenceInstanceId = source->strInstanceId;
		if (asObject)
			value.strObjectResourceId = source->strObjectResourceId;
		candidate.Worlds.push_back(std::move(value));
		world = candidate.Worlds.end() - 1;
	}
	// 배치된 맵 alias는 저장된 World의 screen/presentation companion을 재사용한다.
	// Object 등록에서 원본 sequence의 Effect를 버리지 않도록 함께 확인한다.
	if (asObject && !source->bSupportsPlacement && world->strCompanionEffectResourceId.empty())
	{
		std::string companionId;
		for (const auto &definition : candidate.Worlds)
		{
			if (definition.strSequenceInstanceId != instanceId || !definition.strObjectResourceId.empty() ||
				definition.strCompanionEffectResourceId.empty())
				continue;
			if (!companionId.empty() && companionId != definition.strCompanionEffectResourceId)
			{
				outStatus = m_strStatus =
					"This Object alias has conflicting source companion Effects. Select its World resource explicitly.";
				return false;
			}
			companionId = definition.strCompanionEffectResourceId;
		}
		world->strCompanionEffectResourceId = std::move(companionId);
	}
	KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE box;
	box.strOccurrenceId = pattern->strPatternId + ".world." + std::to_string(pattern->iNextWorldOccurrenceOrdinal++);
	box.strWorldId = world->strWorldId;
	box.iStartMs = (std::min)(m_iCursorMs, Pattern_DurationMs(*pattern));
	box.iDurationMs = asObject && !source->bMotionGroup ? (std::max)(1u, Pattern_DurationMs(*pattern) - box.iStartMs)
														: (std::max)(1u, source->iDurationMs);
	if (!Stage_NewWorldPlacement(instanceId, box.Placement, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	const std::string worldId = box.strWorldId;
	const std::string occurrenceId = box.strOccurrenceId;
	pattern->WorldOccurrences.push_back(box);
	if (source->bMotionGroup && !Extend_PatternLifetimeForAuthoredLanes(*pattern, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	if (!Append_WorldCompanionEffect(*pattern, *world, box, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	if (!asObject || source->bMotionGroup)
		Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate),
			asObject ? "Appended Object with its default animation. Adjust this box's Transform, then Save."
					 : "Appended saved World Object state at the cursor. Tune lifetime in Box Detail.",
			outStatus))
		return false;
	m_strSelectedWorldId = worldId;
	m_strSelectedWorldOccurrenceId = occurrenceId;
	m_strSelectedStageId.clear();
	m_strSelectedOccurrenceId.clear();
	m_strSelectedLogicOccurrenceId.clear();
	m_strSelectedSummonOccurrenceId.clear();
	m_strSelectedSceneProfileOccurrenceId.clear();
	m_strSelectedPresentationOccurrenceId.clear();
	m_strSelectedPatternOccurrenceId.clear();
	m_TimelineSelectedStageIds.clear();
	m_TimelineSelectedOccurrenceIds.clear();
	m_strTimelineSelectionPatternId = m_strSelectedPatternId;
	Synchronize_EditorFields();
	if (asObject && box.Placement)
		Queue_WorldBoxPreview(m_strSelectedPatternId, occurrenceId);
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Create_World(const std::string_view displayName,
	const std::string_view sequenceInstanceId, std::string &outWorldId, std::string &outStatus)
{
	if (!m_bHasDraft || m_Draft.iNextWorldOrdinal >= 1000000u)
	{
		outStatus = m_strStatus = "World creation requires a loaded draft and an available ordinal.";
		return false;
	}
	if (sequenceInstanceId.empty())
	{
		outStatus = m_strStatus = "Create World needs one authored world sequence instance.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION world;
	world.strWorldId = "koukusaydon.g1.world." + std::to_string(candidate.iNextWorldOrdinal++);
	world.strDisplayName = std::string(displayName);
	world.strSequenceInstanceId = std::string(sequenceInstanceId);
	const std::string worldId = world.strWorldId;
	candidate.Worlds.push_back(std::move(world));
	if (!Commit_Candidate(std::move(candidate),
			"Created World " + worldId + ". Select it and press Append World at Cursor.", outStatus))
	{
		return false;
	}
	m_strSelectedWorldId = worldId;
	outWorldId = worldId;
	return true;
}

// World 정의가 소유할 companion Effect 연결을 편집한다. 해당 World occurrence들의 부속 표현도 같은 후보에서 갱신한다.
bool_t Client::CKoukuSaydonActionWorkbench::Set_WorldCompanionEffect(
	const std::string_view worldId, const std::string_view resourceId, std::string &outStatus)
{
	auto candidate = m_Draft;
	const auto world = std::find_if(candidate.Worlds.begin(), candidate.Worlds.end(), [worldId](const auto &row) {
		return row.strWorldId == worldId;
	});
	const auto *resource = resourceId.empty() ? nullptr : Find_PresentationResource(candidate, resourceId);
	if (world == candidate.Worlds.end() ||
		(!resourceId.empty() && (nullptr == resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)))
	{
		outStatus = m_strStatus = "World companion requires an existing World and an EFFECT resource.";
		return false;
	}
	world->strCompanionEffectResourceId = std::string(resourceId);
	for (auto &pattern : candidate.Patterns)
	{
		if (!pattern.strLoadError.empty())
			continue;
		bool changed = false;
		for (const auto &owner : pattern.WorldOccurrences)
		{
			if (owner.strWorldId != worldId)
				continue;
			const auto linked = std::find_if(
				pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(), [&](const auto &row) {
					const auto *effect = Find_PresentationResource(candidate, row.strResourceId);
					return row.strAnchorKind != "WORLD" && row.strWorldOccurrenceId == owner.strOccurrenceId &&
						effect && effect->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT;
				});
			if (linked != pattern.PresentationOccurrences.end())
			{
				if (resourceId.empty())
					linked->strWorldOccurrenceId.clear();
				else
					linked->strResourceId = std::string(resourceId);
				changed = true;
			}
			else if (!resourceId.empty())
			{
				if (!Append_WorldCompanionEffect(pattern, *world, owner, outStatus))
				{
					m_strStatus = outStatus;
					return false;
				}
				changed = true;
			}
		}
		if (changed)
			Mark_Draft(candidate, pattern);
	}
	return Commit_Candidate(std::move(candidate),
		resourceId.empty() ? "Removed the World companion link; existing Effect boxes remain independent."
						   : "Connected the World companion and its explicit Effect boxes.",
		outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Render_WorldCompanionSelector(
	const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION &world)
{
	const auto *selected = Find_PresentationResource(m_Draft, world.strCompanionEffectResourceId);
	if (ImGui::BeginCombo("Companion Effect", nullptr == selected ? "(none)" : selected->strDisplayName.c_str()))
	{
		std::string status;
		if (ImGui::Selectable("(none)", world.strCompanionEffectResourceId.empty()))
		{
			const std::string worldId = world.strWorldId;
			(void)Set_WorldCompanionEffect(worldId, "", status);
			ImGui::EndCombo();
			return true;
		}
		for (const auto &resource : m_Draft.PresentationResources)
			if (resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT &&
				ImGui::Selectable((resource.strDisplayName + "##" + resource.strResourceId).c_str(),
					resource.strResourceId == world.strCompanionEffectResourceId))
			{
				const auto worldId = world.strWorldId, resourceId = resource.strResourceId;
				(void)Set_WorldCompanionEffect(worldId, resourceId, status);
				ImGui::EndCombo();
				return true;
			}
		ImGui::EndCombo();
	}
	ImGui::TextDisabled(
		"World Append adds an Effect box. World timing edits sync it; its own Effect tuning remains editable.");
	return false;
}

// typed occurrence와 격리된 원문 참조를 확인하여 사용 중인 World 정의 삭제를 막는다. 참조가 없는 정의만 후보에서 제거한다.
bool_t Client::CKoukuSaydonActionWorkbench::Delete_World(const std::string_view worldId, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	const auto found = std::find_if(candidate.Worlds.begin(), candidate.Worlds.end(),
		[worldId](const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION &world) {
			return world.strWorldId == worldId;
		});
	if (found == candidate.Worlds.end())
	{
		outStatus = m_strStatus = "World delete target is absent.";
		return false;
	}
	bool_t unresolvedReferences = false;
	const std::size_t references = Count_WorldReferences(candidate, worldId, &unresolvedReferences);
	if (unresolvedReferences)
	{
		outStatus = m_strStatus = "Repair the invalid Pattern World references before deleting a World.";
		return false;
	}
	if (0u != references)
	{
		outStatus = m_strStatus =
			"Delete its " + std::to_string(references) + " World box(es) first; a referenced World is preserved.";
		return false;
	}
	candidate.Worlds.erase(found);
	if (!Commit_Candidate(std::move(candidate), "Deleted World.", outStatus))
		return false;
	if (m_strSelectedWorldId == worldId)
		m_strSelectedWorldId.clear();
	return true;
}

// World 정의의 새 occurrence와 소유 companion을 같은 후보에 추가한다. 시작 시각/기간과 배치 허용 조건을 검증한 뒤 한 번 commit한다.
bool_t Client::CKoukuSaydonActionWorkbench::Append_WorldBox(const std::string_view patternId,
	const std::string_view worldId, const std::uint32_t startMs, const std::uint32_t durationMs,
	std::string &outOccurrenceId, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern || !pattern->strLoadError.empty())
	{
		outStatus = m_strStatus = "Append World requires an editable Pattern selected in Patterns.";
		return false;
	}
	const auto *world = Find_World(candidate, worldId);
	if (nullptr == world)
	{
		outStatus = m_strStatus = "Append World target definition is absent.";
		return false;
	}
	if (pattern->iNextWorldOccurrenceOrdinal >= 1000000u)
	{
		outStatus = m_strStatus = "World box stable ID ordinals are exhausted.";
		return false;
	}
	const std::uint32_t lifetimeMs = Pattern_DurationMs(*pattern);
	if (0u == lifetimeMs)
	{
		outStatus = m_strStatus = "Append World requires a Pattern lifetime. Append an animation action first.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE box;
	box.strOccurrenceId = pattern->strPatternId + ".world." + std::to_string(pattern->iNextWorldOccurrenceOrdinal++);
	box.strWorldId = std::string(worldId);
	box.iStartMs = (std::min)(startMs, lifetimeMs);
	box.iDurationMs = (std::max)(1u, durationMs);
	box.fPlaybackSpeed = 1.f;
	if (!Stage_NewWorldPlacement(world->strSequenceInstanceId, box.Placement, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	const std::string occurrenceId = box.strOccurrenceId;
	pattern->WorldOccurrences.push_back(std::move(box));
	if (!Append_WorldCompanionEffect(*pattern, *world, pattern->WorldOccurrences.back(), outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate),
			"Appended World box. The sequence starts at the box start and runs at the box speed; edit both in Box Detail.",
			outStatus))
	{
		return false;
	}
	m_strSelectedPatternId = std::string(patternId);
	m_TimelineSelectedStageIds.clear();
	m_TimelineSelectedOccurrenceIds.clear();
	m_strSelectedStageId.clear();
	m_strSelectedOccurrenceId.clear();
	m_strSelectedLogicOccurrenceId.clear();
	m_strSelectedSummonOccurrenceId.clear();
	m_strSelectedWorldOccurrenceId = occurrenceId;
	m_strSelectedSceneProfileOccurrenceId.clear();
	m_strSelectedPresentationOccurrenceId.clear();
	m_strSelectedPatternOccurrenceId.clear();
	m_strTimelineSelectionPatternId = m_strSelectedPatternId;
	Synchronize_EditorFields();
	outOccurrenceId = occurrenceId;
	return true;
}

// World 박스의 ms 구간과 재생 속도를 후보에서 갱신한다. 연결된 companion의 시간도 같은 편집에 포함한다.
bool_t Client::CKoukuSaydonActionWorkbench::Set_WorldBoxWindow(const std::string_view patternId,
	const std::string_view occurrenceId, const std::uint32_t startMs, const std::uint32_t durationMs,
	const f32_t playbackSpeed, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE *const box =
		nullptr == pattern ? nullptr : Find_WorldBox(*pattern, occurrenceId);
	if (nullptr == box)
	{
		outStatus = m_strStatus = "World box window target is absent.";
		return false;
	}
	const std::uint64_t lifetimeMs = Pattern_DurationMs(*pattern);
	if (0u == durationMs || startMs > lifetimeMs || !std::isfinite(playbackSpeed) || playbackSpeed < 0.05f ||
		playbackSpeed > 16.f)
	{
		outStatus = m_strStatus = "World box must start inside the Pattern lifetime of " + std::to_string(lifetimeMs) +
			" ms and play at 0.05..16 x.";
		return false;
	}
	if (box->iStartMs == startMs && box->iDurationMs == durationMs && box->fPlaybackSpeed == playbackSpeed)
	{
		outStatus = m_strStatus = "World box is unchanged.";
		return true;
	}
	const bool_t timingChanged = box->iStartMs != startMs || box->iDurationMs != durationMs;
	box->iStartMs = startMs;
	box->iDurationMs = durationMs;
	box->fPlaybackSpeed = playbackSpeed;
	for (auto &effect : pattern->PresentationOccurrences)
		if (timingChanged && effect.strWorldOccurrenceId == occurrenceId)
		{
			effect.iStartMs = startMs;
			effect.iDurationMs = durationMs;
			effect.iFadeInMs = (std::min)(effect.iFadeInMs, durationMs);
			effect.iFadeOutMs = (std::min)(effect.iFadeOutMs, durationMs - effect.iFadeInMs);
		}
	return Commit_Candidate(std::move(candidate), "Updated World and companion Effect timing.", outStatus);
}

// World occurrence를 지울 때 그 occurrence가 소유한 companion 표현도 함께 제거하여 끊어진 owner 참조를 남기지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Delete_WorldBox(
	const std::string_view patternId, const std::string_view occurrenceId, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern)
	{
		outStatus = m_strStatus = "World box delete Pattern is absent.";
		return false;
	}
	const auto found = std::find_if(pattern->WorldOccurrences.begin(), pattern->WorldOccurrences.end(),
		[occurrenceId](const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE &box) {
			return box.strOccurrenceId == occurrenceId;
		});
	if (found == pattern->WorldOccurrences.end())
	{
		outStatus = m_strStatus = "World box delete target is absent.";
		return false;
	}
	pattern->WorldOccurrences.erase(found);
	std::erase_if(pattern->PresentationOccurrences, [occurrenceId](const auto &row) {
		return row.strWorldOccurrenceId == occurrenceId;
	});
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate), "Deleted World and its linked Effect box.", outStatus))
		return false;
	if (m_strSelectedWorldOccurrenceId == occurrenceId)
	{
		m_strSelectedWorldOccurrenceId.clear();
		Synchronize_EditorFields();
	}
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Create_SceneProfile(const std::string_view displayName,
	const std::string_view renderingProfileId, std::string &outSceneProfileId, std::string &outStatus)
{
	if (!m_bHasDraft || m_Draft.iNextSceneProfileOrdinal >= 1000000u)
	{
		outStatus = m_strStatus = "Scene Profile creation requires a loaded draft and an available ordinal.";
		return false;
	}
	if (renderingProfileId.empty())
	{
		outStatus = m_strStatus = "Create Scene Profile needs one rendering profile.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION profile;
	profile.strSceneProfileId = "koukusaydon.g1.sceneprofile." + std::to_string(candidate.iNextSceneProfileOrdinal++);
	profile.strDisplayName = std::string(displayName);
	profile.strRenderingProfileId = std::string(renderingProfileId);
	const std::string sceneProfileId = profile.strSceneProfileId;
	candidate.SceneProfiles.push_back(std::move(profile));
	if (!Commit_Candidate(std::move(candidate),
			"Created Scene Profile " + sceneProfileId + ". Select it and press Append Scene Profile at Cursor.",
			outStatus))
	{
		return false;
	}
	m_strSelectedSceneProfileId = sceneProfileId;
	outSceneProfileId = sceneProfileId;
	return true;
}

// Scene profile의 typed 행과 격리된 원본 참조를 먼저 확인한다. 사용하는 Pattern이 있으면 정의를 유지하고 오류를 돌려준다.
bool_t Client::CKoukuSaydonActionWorkbench::Delete_SceneProfile(
	const std::string_view sceneProfileId, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	const auto found = std::find_if(candidate.SceneProfiles.begin(), candidate.SceneProfiles.end(),
		[sceneProfileId](const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION &profile) {
			return profile.strSceneProfileId == sceneProfileId;
		});
	if (found == candidate.SceneProfiles.end())
	{
		outStatus = m_strStatus = "Scene Profile delete target is absent.";
		return false;
	}
	bool_t unresolvedReferences = false;
	const std::size_t references = Count_SceneProfileReferences(candidate, sceneProfileId, &unresolvedReferences);
	if (unresolvedReferences)
	{
		outStatus = m_strStatus = "Repair the invalid Pattern Scene Profile references before deleting one.";
		return false;
	}
	if (0u != references)
	{
		outStatus = m_strStatus = "Delete its " + std::to_string(references) +
			" Scene Profile box(es) first; a referenced Scene Profile is preserved.";
		return false;
	}
	candidate.SceneProfiles.erase(found);
	if (!Commit_Candidate(std::move(candidate), "Deleted Scene Profile.", outStatus))
		return false;
	if (m_strSelectedSceneProfileId == sceneProfileId)
		m_strSelectedSceneProfileId.clear();
	return true;
}

// Scene profile의 Pattern 구간은 전체가 해당 lifetime 안에 들도록 제한한다. Pattern ID 없이 선택 Bundle에 추가하면 공통 시계의 600000 ms 상한을 사용하며 후보를 한 번 commit한다.
bool_t Client::CKoukuSaydonActionWorkbench::Append_SceneProfileBox(const std::string_view patternId,
	const std::string_view sceneProfileId, const std::uint32_t startMs, const std::uint32_t durationMs,
	std::string &outOccurrenceId, std::string &outStatus)
{
	if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE && patternId.empty())
	{
		auto candidate = m_Draft;
		auto *bundle = Find_Bundle(candidate, m_strSelectedBundleId);
		if (!bundle || !bundle->strLoadError.empty() || !Find_SceneProfile(candidate, sceneProfileId) ||
			bundle->iNextSceneProfileOccurrenceOrdinal >= 1000000u)
		{
			outStatus = m_strStatus = "Select an editable bundle and a valid Scene Profile resource.";
			return false;
		}
		KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE row;
		row.strOccurrenceId =
			bundle->strBundleId + ".sceneprofile." + std::to_string(bundle->iNextSceneProfileOccurrenceOrdinal++);
		row.strSceneProfileId = std::string(sceneProfileId);
		row.iStartMs = (std::min)(startMs, MAX_EDITOR_TIME_MS - 1u);
		row.iDurationMs = (std::clamp)(durationMs, 1u, MAX_EDITOR_TIME_MS - row.iStartMs);
		row.iBlendMs = 500u;
		const auto id = row.strOccurrenceId;
		bundle->SceneProfileOccurrences.push_back(std::move(row));
		bundle->strAuthoringStatus = "DRAFT";
		if (!Commit_Candidate(std::move(candidate), "Appended common Scene Profile on the bundle clock.", outStatus))
			return false;
		outOccurrenceId = id;
		return true;
	}
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern || !pattern->strLoadError.empty())
	{
		outStatus = m_strStatus = "Append Scene Profile requires an editable Pattern selected in Patterns.";
		return false;
	}
	if (nullptr == Find_SceneProfile(candidate, sceneProfileId))
	{
		outStatus = m_strStatus = "Append Scene Profile target definition is absent.";
		return false;
	}
	if (pattern->iNextSceneProfileOccurrenceOrdinal >= 1000000u)
	{
		outStatus = m_strStatus = "Scene Profile box stable ID ordinals are exhausted.";
		return false;
	}
	const std::uint32_t lifetimeMs = Pattern_DurationMs(*pattern);
	if (0u == lifetimeMs)
	{
		outStatus = m_strStatus = "Append Scene Profile requires a Pattern lifetime. Append an animation action first.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE box;
	box.strOccurrenceId =
		pattern->strPatternId + ".sceneprofile." + std::to_string(pattern->iNextSceneProfileOccurrenceOrdinal++);
	box.strSceneProfileId = std::string(sceneProfileId);
	box.iStartMs = (std::min)(startMs, lifetimeMs - 1u);
	box.iDurationMs = std::clamp(durationMs, 1u, lifetimeMs - box.iStartMs);
	box.iBlendMs = 500u;
	const std::string occurrenceId = box.strOccurrenceId;
	pattern->SceneProfileOccurrences.push_back(std::move(box));
	if (!Commit_Candidate(std::move(candidate),
			"Appended Scene Profile box. The profile applies instantly at the box start and restores when the box ends. Blend ms is reserved.",
			outStatus))
	{
		return false;
	}
	m_strSelectedPatternId = std::string(patternId);
	m_TimelineSelectedStageIds.clear();
	m_TimelineSelectedOccurrenceIds.clear();
	m_strSelectedStageId.clear();
	m_strSelectedOccurrenceId.clear();
	m_strSelectedLogicOccurrenceId.clear();
	m_strSelectedSummonOccurrenceId.clear();
	m_strSelectedWorldOccurrenceId.clear();
	m_strSelectedSceneProfileOccurrenceId = occurrenceId;
	m_strTimelineSelectionPatternId = m_strSelectedPatternId;
	Synchronize_EditorFields();
	outOccurrenceId = occurrenceId;
	return true;
}

bool_t Client::CKoukuSaydonActionWorkbench::Set_SceneProfileBoxWindow(const std::string_view patternId,
	const std::string_view occurrenceId, const std::uint32_t startMs, const std::uint32_t durationMs,
	const std::uint32_t blendMs, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE *const box =
		nullptr == pattern ? nullptr : Find_SceneProfileBox(*pattern, occurrenceId);
	if (nullptr == box)
	{
		outStatus = m_strStatus = "Scene Profile box window target is absent.";
		return false;
	}
	const std::uint64_t lifetimeMs = Pattern_DurationMs(*pattern);
	const std::uint64_t endMs = static_cast<std::uint64_t>(startMs) + durationMs;
	if (0u == durationMs || endMs > lifetimeMs || blendMs > MAX_EDITOR_TIME_MS)
	{
		outStatus = m_strStatus =
			"Scene Profile box window must stay inside the Pattern lifetime of " + std::to_string(lifetimeMs) + " ms.";
		return false;
	}
	if (box->iStartMs == startMs && box->iDurationMs == durationMs && box->iBlendMs == blendMs)
	{
		outStatus = m_strStatus = "Scene Profile box is unchanged.";
		return true;
	}
	box->iStartMs = startMs;
	box->iDurationMs = durationMs;
	box->iBlendMs = blendMs;
	return Commit_Candidate(std::move(candidate), "Updated Scene Profile box.", outStatus);
}

bool_t Client::CKoukuSaydonActionWorkbench::Delete_SceneProfileBox(
	const std::string_view patternId, const std::string_view occurrenceId, std::string &outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN *const pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern)
	{
		outStatus = m_strStatus = "Scene Profile box delete Pattern is absent.";
		return false;
	}
	const auto found = std::find_if(pattern->SceneProfileOccurrences.begin(), pattern->SceneProfileOccurrences.end(),
		[occurrenceId](const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE &box) {
			return box.strOccurrenceId == occurrenceId;
		});
	if (found == pattern->SceneProfileOccurrences.end())
	{
		outStatus = m_strStatus = "Scene Profile box delete target is absent.";
		return false;
	}
	pattern->SceneProfileOccurrences.erase(found);
	if (!Commit_Candidate(std::move(candidate), "Deleted Scene Profile box.", outStatus))
		return false;
	if (m_strSelectedSceneProfileOccurrenceId == occurrenceId)
	{
		m_strSelectedSceneProfileOccurrenceId.clear();
		m_strSelectedPresentationOccurrenceId.clear();
		m_strSelectedPatternOccurrenceId.clear();
		Synchronize_EditorFields();
	}
	return true;
}

void Client::CKoukuSaydonActionWorkbench::Render_WorldResources()
{
	ImGui::InputTextWithHint(
		"##WorldObjectFilter", "Search Worlds and Objects", m_WorldObjectFilter, std::size(m_WorldObjectFilter));
	const auto *selectedPattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	const bool patternEditable =
		selectedPattern && selectedPattern->strLoadError.empty() && Pattern_DurationMs(*selectedPattern) > 0u;
	ImGui::SeparatorText("Composition World Resources");
	ImGui::BeginChild("##CompositionWorldNames", ImVec2(0.f, 190.f), ImGuiChildFlags_Borders);
	std::size_t shownWorlds = 0u;
	for (const auto &world : m_Draft.Worlds)
	{
		if (!ContainsInsensitive(world.strDisplayName, m_WorldObjectFilter) &&
			!ContainsInsensitive(world.strWorldId, m_WorldObjectFilter) &&
			!ContainsInsensitive(world.strSequenceInstanceId, m_WorldObjectFilter))
			continue;
		++shownWorlds;
		if (ImGui::Selectable(
				(world.strDisplayName + "##" + world.strWorldId).c_str(), m_strSelectedWorldId == world.strWorldId))
			m_strSelectedWorldId = world.strWorldId;
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s\n%s", world.strWorldId.c_str(), world.strSequenceInstanceId.c_str());
	}
	if (!shownWorlds)
		ImGui::TextDisabled("No matching saved World resources.");
	ImGui::EndChild();
	if (const auto *world = Find_World(m_Draft, m_strSelectedWorldId))
	{
		if (Render_RenameControl(RENAME_TARGET::WORLD, world->strWorldId, world->strDisplayName))
			return;
		const auto motion =
			std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(), [&](const auto &row) {
				return row.strInstanceId == world->strSequenceInstanceId;
			});
		const bool savedMotion = motion != m_WorldSequenceResources.end() && motion->bEnabled;
		ImGui::BeginDisabled(!patternEditable || !savedMotion);
		if (ImGui::Button("Append Selected World"))
		{
			const std::string patternId = selectedPattern->strPatternId, worldId = world->strWorldId;
			const auto start = (std::min)(m_iCursorMs, Pattern_DurationMs(*selectedPattern));
			const auto duration = (std::max)(1u, Pattern_DurationMs(*selectedPattern) - start);
			std::string occurrenceId, status;
			if (Append_WorldBox(patternId, worldId, start, duration, occurrenceId, status))
				Queue_WorldBoxPreview(patternId, occurrenceId);
			ImGui::EndDisabled();
			return;
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(!savedMotion);
		if (ImGui::Button("Preview Selected World"))
		{
			KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE preview;
			preview.eKind = KOUKU_SAYDON_PRESENTATION_KIND::WORLD;
			preview.strAssetId = world->strSequenceInstanceId;
			preview.strDisplayName = world->strDisplayName;
			preview.iDurationMs = (std::max)(1u, motion->iDurationMs);
			Queue_PresentationPreview(preview);
		}
		ImGui::EndDisabled();
		ImGui::TextDisabled("Uses this resource's saved motion and Object, including non-default motions.");
	}
	ImGui::SeparatorText("Default World Objects");
	const KOUKU_WORLD_SEQUENCE_RESOURCE *selected = nullptr;
	for (const auto &resource : m_WorldSequenceResources)
		if (resource.strObjectResourceId == m_strNewWorldObjectId && !resource.strAppendGroupObjectId.empty())
		{
			m_strNewWorldObjectId = resource.strAppendGroupObjectId;
			break;
		}
	if (ImGui::BeginChild("##WorldObjects", ImVec2(0.f, 160.f), ImGuiChildFlags_Borders))
	{
		std::size_t shown = 0u;
		for (const auto &resource : m_WorldSequenceResources)
		{
			if (resource.strObjectResourceId.empty() || !resource.strAppendGroupObjectId.empty() ||
				(!resource.bDefaultMotion && !resource.strInstanceId.empty()))
				continue;
			if (resource.strObjectResourceId == m_strNewWorldObjectId)
				selected = &resource;
			if (!ContainsInsensitive(resource.strObjectDisplayName, m_WorldObjectFilter) &&
				!ContainsInsensitive(resource.strObjectResourceId, m_WorldObjectFilter))
				continue;
			++shown;
			ImGui::PushID(resource.strObjectResourceId.c_str());
			const std::string label = resource.strObjectDisplayName + "###WorldObject";
			if (ImGui::Selectable(label.c_str(), m_strNewWorldObjectId == resource.strObjectResourceId))
			{
				m_strNewWorldObjectId = resource.strObjectResourceId;
				selected = &resource;
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s%s", resource.strObjectResourceId.c_str(),
					resource.bEnabled && resource.bDefaultMotion
						? ""
						: " | Set an enabled default animation in Object Tool, then Save.");
			ImGui::PopID();
		}
		if (!shown)
			ImGui::TextDisabled("No matching Objects. Create and Save them in Object Tool.");
	}
	ImGui::EndChild();
	if (!selected)
	{
		for (const auto &resource : m_WorldSequenceResources)
			if (resource.strObjectResourceId == m_strNewWorldObjectId &&
				(resource.bDefaultMotion || resource.strInstanceId.empty()))
			{
				selected = &resource;
				break;
			}
	}
	const bool ready = selected && selected->bEnabled && selected->bDefaultMotion && !selected->strInstanceId.empty();
	if (selected)
	{
		ImGui::TextUnformatted(selected->strObjectDisplayName.c_str());
		ImGui::TextDisabled("%s",
			selected->strAnchorKind == "BOSS"		  ? "Anchor / Boss BODY Bone"
				: selected->strAnchorKind == "PLAYER" ? "Anchor / Character"
													  : "Map");
		if (selected->bUntilDestroyed)
			ImGui::TextWrapped("HP: %u | After spawning, the Server keeps this Object alive until it is destroyed.",
				selected->iMaximumHp);
		if (ready)
			ImGui::TextWrapped("Default: %s", selected->strDisplayName.c_str());
		else
			ImGui::TextWrapped("Set an enabled default animation in Object Tool, then Save.");
	}
	ImGui::BeginDisabled(!ready || !patternEditable);
	if (ImGui::Button("Append Object"))
	{
		std::string status;
		(void)Append_WorldObject(m_strNewWorldObjectId, status);
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!ready);
	if (ImGui::Button("Preview Object"))
	{
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE preview;
		preview.eKind = KOUKU_SAYDON_PRESENTATION_KIND::WORLD;
		preview.strAssetId = selected->strInstanceId;
		preview.strDisplayName = selected->strObjectDisplayName;
		preview.iDurationMs = (std::max)(1u, selected->iDurationMs);
		Queue_PresentationPreview(preview);
	}
	ImGui::EndDisabled();
	ImGui::TextDisabled("Pattern: %s | Start: %u ms",
		selectedPattern ? selectedPattern->strDisplayName.c_str() : "Select a Pattern", m_iCursorMs);
	ImGui::TextWrapped(
		"Append creates one Object box. Select the box to adjust its Transform and view its linked animations, then Save.");
	if (!m_strWorldSequenceResourceStatus.empty())
		ImGui::TextDisabled("%s", m_strWorldSequenceResourceStatus.c_str());
	if (!m_strStatus.empty())
		ImGui::TextWrapped("%s", m_strStatus.c_str());
}

// Details pane의 World 시간/배치 입력이다. 모델은 생성하지 않고 문서 setter와 preview 요청을 통해 기존 소비자에 연결한다.
void Client::CKoukuSaydonActionWorkbench::Render_WorldBoxDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern)
{
	const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE *const box = Find_WorldBox(pattern, m_strSelectedWorldOccurrenceId);
	if (nullptr == box)
		return;
	const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION *const world = Find_World(m_Draft, box->strWorldId);
	const std::string patternId = pattern.strPatternId;
	const std::string occurrenceId = box->strOccurrenceId;
	const std::uint32_t lifetimeMs = Pattern_DurationMs(pattern);
	const bool untilDestroyed =
		world && std::any_of(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(), [&](const auto &row) {
			return row.strInstanceId == world->strSequenceInstanceId && row.bUntilDestroyed;
		});
	ImGui::SeparatorText("Object Box");
	ImGui::Text("%s", nullptr == world ? "(missing World)" : world->strDisplayName.c_str());
	ImGui::TextDisabled(
		"%s -> %s", box->strWorldId.c_str(), nullptr == world ? "?" : world->strSequenceInstanceId.c_str());
	if (nullptr != world)
	{
		const auto source =
			std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(), [&](const auto &row) {
				return row.strInstanceId == world->strSequenceInstanceId;
			});
		const bool canEditClips = source != m_WorldSequenceResources.end() && !source->bMotionGroup &&
			(!world->strObjectResourceId.empty() || !source->strObjectResourceId.empty());
		ImGui::BeginDisabled(!canEditClips);
		if (ImGui::Button("Edit Animation Clips##WorldBox"))
			Queue_WorldObjectEdit(patternId, occurrenceId, {}, true);
		ImGui::EndDisabled();
		if (canEditClips)
			ImGui::TextWrapped("Choose native clips and trim their source time on this shared World Motion.");
		else if (source != m_WorldSequenceResources.end() && source->bMotionGroup)
			ImGui::TextDisabled("Choose a linked Motion below to edit its clips.");
		ImGui::SeparatorText("Transform");
		const bool supportsPlacement = source != m_WorldSequenceResources.end() && source->bSupportsPlacement;
		const bool localPlacement = supportsPlacement && source->strAnchorKind != "WORLD";
		ImGui::BeginDisabled(!supportsPlacement || localPlacement);
		const bool placeNear = ImGui::Button("Place near character##WorldBox");
		ImGui::EndDisabled();
		if (placeNear)
		{
			std::string status;
			(void)Place_WorldBoxNearCharacter(patternId, occurrenceId, status);
			return;
		}
		ImGui::SameLine();
		if (ImGui::Button("Preview placements##WorldBox"))
			Queue_WorldBoxPreview(patternId, occurrenceId);
		const bool savedMapPlacement = supportsPlacement && !localPlacement && world->strAnchorKind == "NONE";
		if (box->Placement || localPlacement || savedMapPlacement)
		{
			auto placement = box->Placement.value_or(KOUKU_SAYDON_WORLD_PLACEMENT{});
			if (!box->Placement && savedMapPlacement)
				for (std::size_t axis = 0u; axis < 3u; ++axis)
					placement.Position[axis] = source->SavedPosition[axis] + world->PositionOffset[axis];
			const auto vectorControl = [](const char *label, std::array<double, 3u> &values, const float step,
										   const float minimum, const float maximum) {
				float v[3] = {
					static_cast<float>(values[0]), static_cast<float>(values[1]), static_cast<float>(values[2])};
				if (!ImGui::DragFloat3(label, v, step, minimum, maximum, "%.3f"))
					return false;
				for (std::size_t i = 0u; i < 3u; ++i)
					values[i] = v[i];
				return true;
			};
			bool changed =
				vectorControl(localPlacement ? "Local position (m)##WorldBox" : "MAP group position (m)##WorldBox",
					placement.Position, .05f, -100000.f, 100000.f);
			changed |= vectorControl("Rotation (degrees)##WorldBox", placement.RotationDegrees, .5f, -36000.f, 36000.f);
			changed |= vectorControl("Scale##WorldBox", placement.Scale, .01f, .001f, 1000.f);
			if (changed)
			{
				std::string status;
				(void)Set_WorldBoxPlacement(patternId, occurrenceId, placement, status);
				return;
			}
			ImGui::TextWrapped(localPlacement
					? "This box adds a local transform after its saved Motion and before the live Boss/Character anchor. Scale multiplies the authored Object size."
					: "Position and rotation move every Object in this box together. Scale also changes group spacing and movement. Numeric edits update Preview immediately; Save keeps this box's transform.");
		}
		else
		{
			ImGui::TextWrapped(supportsPlacement
					? "This older box uses the saved Motion position and World definition offset. Place near character to create an independent transform for this box."
					: "This World uses existing map placements or a Character anchor. Its saved placement is preserved; resource Preview can be played near the character.");
		}
	}
	if (world)
	{
		const auto initial =
			std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(), [&](const auto &row) {
				return row.strInstanceId == world->strSequenceInstanceId;
			});
		const std::string objectId = !world->strObjectResourceId.empty()
			? world->strObjectResourceId
			: (initial != m_WorldSequenceResources.end() ? initial->strObjectResourceId : std::string{});
		const auto queueObjectEdit = [&](const std::string &motionId) {
			Queue_WorldObjectEdit(patternId, occurrenceId, motionId);
		};
		if (initial != m_WorldSequenceResources.end())
		{
			ImGui::TextWrapped(
				"Initial Motion: %s | Objects: %u", initial->strDisplayName.c_str(), initial->iEmissionCount);
			if (!initial->AnimationTracks.empty())
			{
				ImGui::SeparatorText("Animation clips owned by this World");
				ImGui::TextWrapped("Model: %s", initial->strModelAssetId.c_str());
				for (const auto &clip : initial->AnimationTracks)
				{
					const double start = box->iStartMs + clip.fStartMs / box->fPlaybackSpeed;
					const double end =
						box->iStartMs + (std::min)(double(box->iDurationMs), clip.fEndMs / box->fPlaybackSpeed);
					ImGui::BulletText("%s | slot %s", clip.strClipName.c_str(), clip.strSlotId.c_str());
					ImGui::Text("  Window %.0f - %.0f ms | Source In %u ms | speed x%.3f", start, end,
						clip.iSourceStartMs, clip.fPlaybackRate * box->fPlaybackSpeed);
					if (clip.iSourceEndMs > 0u)
						ImGui::Text("  Source Out %u ms", clip.iSourceEndMs);
					else
						ImGui::TextDisabled("  Source Out: native clip end");
					ImGui::TextDisabled(
						"  %s", clip.bLoop ? "Loop" : (clip.bHoldLastFrame ? "Hold last frame" : "Stop at clip end"));
				}
				ImGui::TextWrapped(
					"Drag the Animation lane clips directly: center moves, left/right edges trim. Save commits these edits.");
			}
			if (initial->bUntilDestroyed)
				ImGui::TextWrapped("HP: %u | Server lifetime: until destroyed", initial->iMaximumHp);
			if (!objectId.empty() && !initial->bMotionGroup && ImGui::Button("Edit This Motion##WorldBox"))
				queueObjectEdit(initial->strInstanceId);
		}
		if (!objectId.empty())
		{
			if (initial != m_WorldSequenceResources.end())
				ImGui::SameLine();
			if (ImGui::Button("Edit Object##WorldBox"))
				queueObjectEdit({});
			ImGui::TextWrapped(
				"Object size is shared. This box places the whole group; its Scale also scales spacing and movement. Edit the Motion to tune count, spacing, direction and speed.");
		}
		if (!objectId.empty() && ImGui::CollapsingHeader("Linked animations", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (const auto &motion : m_WorldSequenceResources)
			{
				if (motion.strObjectResourceId != objectId || motion.strInstanceId.empty() || motion.bMotionGroup)
					continue;
				ImGui::PushID(motion.strInstanceId.c_str());
				ImGui::BulletText("%s%s | Objects: %u", motion.strDisplayName.c_str(),
					motion.bEnabled ? "" : " (disabled)", motion.iEmissionCount);
				ImGui::SameLine();
				if (ImGui::SmallButton("Edit Motion"))
					queueObjectEdit(motion.strInstanceId);
				ImGui::PopID();
				for (const auto &clip : motion.AnimationClips)
					ImGui::TextDisabled("    %s", clip.c_str());
				if (motion.AnimationClips.empty())
					ImGui::TextDisabled("    Transform animation");
			}
			ImGui::TextDisabled("Edit animations in Object Tool. Collider Logic selects the reaction for this Object.");
		}
		if (ImGui::CollapsingHeader("Attached Effect"))
			if (Render_WorldCompanionSelector(*world))
				return;
	}
	ImGui::SeparatorText("Timing");
	ImGui::InputInt("Start ms##KoukuWorldBox", &m_iWorldBoxStartMs, 10, 100);
	ImGui::InputInt(untilDestroyed ? "Box length ms##KoukuWorldBox" : "Lifetime ms##KoukuWorldBox",
		&m_iWorldBoxDurationMs, 10, 100);
	ImGui::InputFloat("Playback speed x##KoukuWorldBox", &m_fWorldBoxPlaybackSpeed, 0.05f, 0.25f, "%.2f");
	m_iWorldBoxStartMs = std::clamp(m_iWorldBoxStartMs, 0, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	m_iWorldBoxDurationMs = std::clamp(m_iWorldBoxDurationMs, 1, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	m_fWorldBoxPlaybackSpeed = std::clamp(m_fWorldBoxPlaybackSpeed, 0.05f, 16.f);
	if (ImGui::Button("Apply##KoukuWorldBox"))
	{
		std::string status;
		(void)Set_WorldBoxWindow(patternId, occurrenceId, static_cast<std::uint32_t>(m_iWorldBoxStartMs),
			static_cast<std::uint32_t>(m_iWorldBoxDurationMs), m_fWorldBoxPlaybackSpeed, status);
		return;
	}
	if (untilDestroyed)
		ImGui::TextWrapped(
			"Start ms sets the spawn time. In Server Play, the box end and normal Pattern end do not remove this Object; HP 0 or encounter reset does. Object Tool controls its repeating animation and Effect cycle.");
	else
		ImGui::TextWrapped(m_bSequenceWorkspace
				? "Preview uses this box placement at Start ms with its playback speed. Lifetime ms limits the active World box; sequence motion uses its authored length and speed. Sequence lifetime %u ms."
				: "Preview and Server Play use this box placement at Start ms with its playback speed. Lifetime ms limits the active World box; sequence motion uses its authored length and speed. Pattern lifetime %u ms.",
			lifetimeMs);
	if (ImGui::Button("Delete World Box"))
	{
		std::string status;
		(void)Delete_WorldBox(patternId, occurrenceId, status);
	}
}

void Client::CKoukuSaydonActionWorkbench::Render_SceneProfileResources()
{
	ImGui::SeparatorText("Scene Profile Catalog");
	ImGui::TextDisabled(
		"%zu scene profiles | Create names a rendering profile; Append applies it for a window of the Pattern.",
		m_Draft.SceneProfiles.size());
	if (!m_strRenderingProfileResourceStatus.empty())
		ImGui::TextWrapped("%s", m_strRenderingProfileResourceStatus.c_str());
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	const KOUKU_SAYDON_COMPOSITION_PATTERN *const selectedPattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	const auto *selectedBundle =
		m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE ? Find_Bundle(m_Draft, m_strSelectedBundleId) : nullptr;
	const bool_t patternEditable = (selectedPattern && selectedPattern->strLoadError.empty()) ||
		(selectedBundle && selectedBundle->strLoadError.empty());
	const std::string targetPatternName = selectedBundle ? selectedBundle->strDisplayName + " (Common)"
		: selectedPattern								 ? selectedPattern->strDisplayName
														 : "none";
	const std::uint32_t lifetimeMs = selectedBundle ? Bundle_DurationMs(m_Draft, *selectedBundle)
		: selectedPattern							? Pattern_DurationMs(*selectedPattern)
													: 0u;

	if (ImGui::BeginChild("##KoukuSceneProfileList", ImVec2(0.f, 140.f), ImGuiChildFlags_Borders))
	{
		if (m_Draft.SceneProfiles.empty())
			ImGui::TextDisabled(
				"No Scene Profile yet. Name it, pick a rendering profile and press Create Scene Profile.");
		for (const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION &profile : m_Draft.SceneProfiles)
		{
			const std::string label = profile.strDisplayName + "##" + profile.strSceneProfileId;
			if (ImGui::Selectable(label.c_str(), m_strSelectedSceneProfileId == profile.strSceneProfileId))
				m_strSelectedSceneProfileId = profile.strSceneProfileId;
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s | %s | %zu box(es)", profile.strSceneProfileId.c_str(),
					profile.strRenderingProfileId.c_str(),
					Count_SceneProfileReferences(m_Draft, profile.strSceneProfileId));
		}
	}
	ImGui::EndChild();

	ImGui::SeparatorText("Create Scene Profile");
	ImGui::InputTextWithHint("##NewKoukuSceneProfileName", "Scene Profile display name", m_NewSceneProfileName,
		std::size(m_NewSceneProfileName));
	if (ImGui::BeginCombo("Rendering profile##NewKoukuSceneProfile",
			m_strNewSceneProfileRenderingId.empty() ? "(pick a rendering profile)"
													: m_strNewSceneProfileRenderingId.c_str()))
	{
		if (m_RenderingProfileIds.empty())
			ImGui::TextDisabled("No rendering profile is loaded.");
		for (const std::string &profileId : m_RenderingProfileIds)
		{
			if (ImGui::Selectable(profileId.c_str(), m_strNewSceneProfileRenderingId == profileId))
				m_strNewSceneProfileRenderingId = profileId;
		}
		ImGui::EndCombo();
	}
	ImGui::BeginDisabled(m_strNewSceneProfileRenderingId.empty());
	if (ImGui::Button("Preview Rendering Profile##NewSceneProfile"))
	{
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE preview;
		preview.eKind = KOUKU_SAYDON_PRESENTATION_KIND::SCENE_PROFILE;
		preview.strAssetId = m_strNewSceneProfileRenderingId;
		preview.strDisplayName = m_strNewSceneProfileRenderingId;
		preview.iDurationMs = 3000u;
		Queue_PresentationPreview(preview);
	}
	ImGui::EndDisabled();
	ImGui::BeginDisabled('\0' == m_NewSceneProfileName[0] || !m_bHasDraft || m_strNewSceneProfileRenderingId.empty());
	if (ImGui::Button("Create Scene Profile"))
	{
		std::string sceneProfileId;
		std::string status;
		if (Create_SceneProfile(m_NewSceneProfileName, m_strNewSceneProfileRenderingId, sceneProfileId, status))
			m_NewSceneProfileName[0] = '\0';
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();

	const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION *const selectedProfile =
		Find_SceneProfile(m_Draft, m_strSelectedSceneProfileId);
	if (nullptr == selectedProfile)
		return;
	const std::string sceneProfileId = selectedProfile->strSceneProfileId;
	const std::string profileName = selectedProfile->strDisplayName;
	const std::string renderingId = selectedProfile->strRenderingProfileId;
	bool_t unresolvedReferences = false;
	const std::size_t references = Count_SceneProfileReferences(m_Draft, sceneProfileId, &unresolvedReferences);
	ImGui::SeparatorText("Selected Scene Profile");
	ImGui::TextWrapped("%s", profileName.c_str());
	if (Render_RenameControl(RENAME_TARGET::SCENE_PROFILE, sceneProfileId, profileName))
		return;
	ImGui::TextDisabled("%s | %s | %zu box(es)", sceneProfileId.c_str(), renderingId.c_str(), references);
	if (!m_RenderingProfileIds.empty() &&
		m_RenderingProfileIds.end() ==
			std::find(m_RenderingProfileIds.begin(), m_RenderingProfileIds.end(), renderingId))
	{
		ImGui::TextColored(ImVec4(1.f, 0.72f, 0.24f, 1.f),
			"This rendering profile is not in the loaded runtime catalog; publish RenderingProfiles first.");
	}
	if (ImGui::Button("Preview Scene Profile"))
	{
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE preview;
		preview.eKind = KOUKU_SAYDON_PRESENTATION_KIND::SCENE_PROFILE;
		preview.strResourceId = sceneProfileId;
		preview.strAssetId = renderingId;
		preview.strDisplayName = profileName;
		preview.iDurationMs = 3000u;
		Queue_PresentationPreview(preview);
	}
	ImGui::Checkbox("Until sequence end##NewKoukuSceneProfileBox", &m_bNewSceneProfileBoxToPatternEnd);
	if (!m_bNewSceneProfileBoxToPatternEnd)
	{
		ImGui::SameLine();
		ImGui::SetNextItemWidth(120.f);
		ImGui::InputInt("Box ms##NewKoukuSceneProfileBox", &m_iNewSceneProfileBoxDurationMs, 100, 1000);
		m_iNewSceneProfileBoxDurationMs =
			std::clamp(m_iNewSceneProfileBoxDurationMs, 1, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	}
	ImGui::BeginDisabled(!patternEditable);
	if (ImGui::Button("Append Scene Profile at Cursor"))
	{
		const std::uint32_t durationMs = m_bNewSceneProfileBoxToPatternEnd
			? (lifetimeMs > m_iCursorMs ? lifetimeMs - m_iCursorMs : 1u)
			: static_cast<std::uint32_t>(m_iNewSceneProfileBoxDurationMs);
		std::string occurrenceId;
		std::string status;
		(void)Append_SceneProfileBox(
			m_strSelectedPatternId, sceneProfileId, m_iCursorMs, durationMs, occurrenceId, status);
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(0u != references || unresolvedReferences);
	if (ImGui::Button("Delete Scene Profile"))
	{
		std::string status;
		(void)Delete_SceneProfile(sceneProfileId, status);
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();
	if (!patternEditable)
		ImGui::TextDisabled("Select an editable Pattern in Patterns to append this Scene Profile.");
	ImGui::TextDisabled(
		"Append target: %s | cursor %u ms | lifetime %u ms", targetPatternName.c_str(), m_iCursorMs, lifetimeMs);
}

// Scene profile 구간의 입력 buffer와 Apply/Delete를 연결한다. 시각화 선택 자체로 공유 scene 설정을 저장하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Render_SceneProfileBoxDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern)
{
	const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE *const box =
		Find_SceneProfileBox(pattern, m_strSelectedSceneProfileOccurrenceId);
	if (nullptr == box)
		return;
	const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION *const profile =
		Find_SceneProfile(m_Draft, box->strSceneProfileId);
	const std::string patternId = pattern.strPatternId;
	const std::string occurrenceId = box->strOccurrenceId;
	const std::uint32_t lifetimeMs = Pattern_DurationMs(pattern);
	ImGui::SeparatorText("Scene Profile Box");
	ImGui::Text("%s", occurrenceId.c_str());
	ImGui::Text("%s", nullptr == profile ? "(missing Scene Profile)" : profile->strDisplayName.c_str());
	ImGui::TextDisabled(
		"%s -> %s", box->strSceneProfileId.c_str(), nullptr == profile ? "?" : profile->strRenderingProfileId.c_str());
	ImGui::InputInt("Start ms##KoukuSceneProfileBox", &m_iSceneProfileBoxStartMs, 10, 100);
	ImGui::InputInt("Lifetime ms##KoukuSceneProfileBox", &m_iSceneProfileBoxDurationMs, 10, 100);
	ImGui::InputInt("Blend ms (reserved)##KoukuSceneProfileBox", &m_iSceneProfileBoxBlendMs, 10, 100);
	m_iSceneProfileBoxStartMs = std::clamp(m_iSceneProfileBoxStartMs, 0, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	m_iSceneProfileBoxDurationMs =
		std::clamp(m_iSceneProfileBoxDurationMs, 1, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	m_iSceneProfileBoxBlendMs = std::clamp(m_iSceneProfileBoxBlendMs, 0, static_cast<int32_t>(MAX_EDITOR_TIME_MS));
	if (ImGui::Button("Apply##KoukuSceneProfileBox"))
	{
		std::string status;
		(void)Set_SceneProfileBoxWindow(patternId, occurrenceId, static_cast<std::uint32_t>(m_iSceneProfileBoxStartMs),
			static_cast<std::uint32_t>(m_iSceneProfileBoxDurationMs),
			static_cast<std::uint32_t>(m_iSceneProfileBoxBlendMs), status);
		return;
	}
	ImGui::SameLine();
	if (ImGui::Button("To Pattern End##KoukuSceneProfileBox"))
	{
		std::string status;
		(void)Set_SceneProfileBoxWindow(patternId, occurrenceId, box->iStartMs,
			lifetimeMs > box->iStartMs ? lifetimeMs - box->iStartMs : 1u, box->iBlendMs, status);
		return;
	}
	ImGui::TextWrapped(
		"The profile applies instantly at Start ms and the previous profile returns when Lifetime ends. Blend ms is reserved. Pattern lifetime %u ms.",
		lifetimeMs);
	ImGui::TextWrapped(
		"Brightness: Rendering Workbench -> Light Resources -> Scene Profile -> Light Detail -> Map Light Intensity Multiplier.");
	if (ImGui::Button("Delete Scene Profile Box"))
	{
		std::string status;
		(void)Delete_SceneProfileBox(patternId, occurrenceId, status);
	}
}

// Effect/Sound/Subtitle 리소스와 occurrence 배치, 그룹 preview를 담당한다. 리소스 조회 캐시와 저장 전 overlay의 수명을 세션이 소유한다.
#include "imgui.h"
#include "KoukuSaydonActionWorkbench.h"
#include "KoukuSaydonActionWorkbench_Internal.h"
#include "KoukuSaydonActionWorkbench_CameraLookup.h"

// 이 도메인에서 직접 사용하는 런타임/저작 자료의 선언이다.
#include "Level_KoukuSaydonArena.h"
#include "RuntimeAssetRoot.h"
#include "WorldSequenceDocument.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "Transform.h"
#include "Model.h"
#include "Effect_Catalog.h"
#include "Effect_Playback.h"
#include "Effect_AuthoringDocument.h"
#include "GameInstance.h"
#include "Profiler.h"
#include "DirectXTK/SimpleMath.h"

// 후보 값 복사, stable ID 조회, 시간/좌표 계산에 필요한 표준 타입이다.
#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <iterator>
#include <limits>
#include <map>
#include <unordered_set>
#include <utility>
#include <memory>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

using namespace Client::KoukuWorkbenchDetail;

namespace
{
using namespace Client;

// 리소스 목록의 owner 필터가 사용한다. 저장된 분류 경로의 첫 항목을 우선하고, 없으면
// asset ID 접두사로 분류한다. 알 수 없는 항목은 Other이며 원본 분류 자료를 변경하지 않는다.
std::string Effect_ResourceOwner(const std::string_view asset, const std::vector<std::string> &path)
{
	const auto normalize = [](std::string_view value) {
		std::string result;
		for (unsigned char c : value)
			if (!std::isspace(c))
				result += static_cast<char>(std::tolower(c));
		return result;
	};
	struct OWNER_TEXT final
	{
		std::string_view label;
		std::string normalizedLabel, assetPrefix;
	};
	// 정적 owner 메뉴 label은 resource refresh로 바뀌지 않는다.
	static const auto owners = [&]() {
		std::array<OWNER_TEXT, EFFECT_TOOL_ALL_EFFECTS_OWNER_OPTIONS.size()> values;
		for (std::size_t i = 0u; i < values.size(); ++i)
		{
			const auto &option = EFFECT_TOOL_ALL_EFFECTS_OWNER_OPTIONS[i];
			values[i].label = option.strLabel;
			values[i].normalizedLabel = normalize(option.strLabel);
			values[i].assetPrefix = option.eKind == EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::KOUKU_BOSS
				? "effect.kouku."
				: "effect." + values[i].normalizedLabel + ".";
		}
		return values;
	}();
	if (!path.empty())
	{
		const auto category = normalize(path.front());
		for (const auto &owner : owners)
			if (owner.normalizedLabel == category)
				return std::string(owner.label);
	}
	for (const auto &owner : owners)
		if (asset.starts_with(owner.assetPrefix))
			return std::string(owner.label);
	return "Other";
}

// V1 리소스 길이를 갱신할 때 element의 재생 종료 초를 밀리초로 올림한다.
// 유한하지 않거나 음수인 종료값은 실패 표시 0으로 돌려주고 유효한 길이는 편집기 시간 범위로 제한한다.
std::uint32_t V1_ElementDurationMs(const EFFECT_ELEMENT_DESC &element)
{
	// Effect Tool의 Element_PreviewEndSeconds와 같은 carrier 허용 조건을 사용한다.
	const auto &recipe = element.SourceRecipe;
	const bool sourceParticleCarrier = recipe.bEnabled &&
		(recipe.strRendererShape == "sprite" || recipe.strRendererShape == "mesh" ||
			recipe.strRendererShape == "decal");
	const auto seconds = CEffectPlayback::Calculate_ElementEndSeconds(element, sourceParticleCarrier);
	if (!std::isfinite(seconds) || seconds < 0.f)
		return 0u;
	return static_cast<std::uint32_t>(
		std::clamp(std::ceil(1000.0 * seconds), 1.0, static_cast<double>(MAX_EDITOR_TIME_MS)));
}

// 리소스 생성/추가/preview 요청이 V1 원본의 현재 길이를 확인할 때 사용한다.
// catalog 문서와 element는 호출 동안만 차용하며 계산이 끝난 뒤 resource.iDurationMs만 교체한다.
// 파일/element가 없거나 계산이 실패하면 입력 resource의 이전 길이를 보존하고 상태를 반환한다.
bool Refresh_V1ResourceDuration(KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE &resource, std::string &outStatus)
{
	if (resource.eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
		(resource.strResourceKind != "V1_EFFECT" && resource.strResourceKind != "V1_ELEMENT"))
		return true;
	const auto document = CEffectCatalog::Find(resource.strAssetId);
	if (!document)
	{
		outStatus = "Effect duration unavailable: " + resource.strAssetId + "; " + CEffectCatalog::Get_Status();
		return false;
	}
	std::uint32_t durationMs = 1u;
	bool elementFound = resource.strResourceKind == "V1_EFFECT";
	for (const auto &element : document->Elements)
	{
		if (resource.strResourceKind == "V1_ELEMENT" ? element.strElementId != resource.strElementId
													 : !element.bVisible)
			continue;
		elementFound = true;
		const auto elementMs = V1_ElementDurationMs(element);
		if (!elementMs)
		{
			outStatus = "Effect duration is not finite: " + element.strElementId;
			return false;
		}
		durationMs = (std::max)(durationMs, elementMs);
	}
	if (!elementFound)
	{
		outStatus = "Effect element duration unavailable: " + resource.strAssetId + " | " + resource.strElementId;
		return false;
	}
	if (resource.strResourceKind == "V1_EFFECT")
		for (const auto &cue : document->ModelCues)
			if (cue.bVisible)
				durationMs = (std::max)(durationMs,
					static_cast<std::uint32_t>(std::clamp(std::ceil(1000.0 * Effect_ModelCueEndSeconds(cue)), 1.0,
						static_cast<double>(MAX_EDITOR_TIME_MS))));
	resource.iDurationMs = durationMs;
	return true;
}

// 피킹 결과와 Effect anchor 전환에서 사용하는 공통 초기화다. 전달된 편집 사본을 MAP 위치로
// 바꾸면서 이전 bone/World 참조를 지운다. 문서 commit이나 preview 요청은 호출자가 결정한다.
void Set_FixedWorldEffectPlacement(
	KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &edit, const std::array<double, 3u> &position)
{
	edit.strAnchorKind = "MAP";
	edit.PositionOffset = position;
	edit.bFollowBoss = false;
	edit.strBone.clear();
	edit.strBoneTarget = "BODY";
	edit.strBoneRotation = "TARGET_YAW";
	edit.strWorldId.clear();
	edit.strWorldOccurrenceId.clear();
	edit.iWorldEmissionIndex = 0u;
	edit.strWorldEffectTrackId.clear();
}
} // 이 파일의 보조 구현 범위 끝

// MainApp의 선택 Effect 표시가 읽는 공개 조회다. 현재 Detail 입력이 유효한 MAP 배치일 때만
// 값 snapshot을 반환하며 WORLD anchor에서 고정 위치를 새로 고르는 경우는 요청 경로가 따로 허용한다.
bool_t Client::CKoukuSaydonActionWorkbench::Get_SelectedMapEffectPlacement(
	KOUKU_MAP_EFFECT_PLACEMENT &outPlacement) const
{
	return Get_MapEffectPlacementSelection(outPlacement, false);
}

// 표시/피킹 요청/요청 유효성 검사가 공유하는 선택 해석이다. draft의 stable ID와 현재 Detail
// 입력이 같은 박스를 가리키는지 검증한 뒤 값과 draft generation을 복사한다. 실패하면 outPlacement를 유지한다.
bool_t Client::CKoukuSaydonActionWorkbench::Get_MapEffectPlacementSelection(
	KOUKU_MAP_EFFECT_PLACEMENT &outPlacement, const bool_t allowWorldPositionPick) const
{
	if (!m_bHasDraft || m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN)
		return false;
	const auto *pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
	const auto *box = pattern ? Find_PresentationBox(*pattern, m_strSelectedPresentationOccurrenceId) : nullptr;
	const auto *resource = box ? Find_PresentationResource(m_Draft, box->strResourceId) : nullptr;
	const auto &edit = m_PresentationBoxEdit;
	if (!box || !resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
		edit.strOccurrenceId != box->strOccurrenceId || edit.strResourceId != box->strResourceId ||
		(edit.strAnchorKind != "MAP" && !(allowWorldPositionPick && edit.strAnchorKind == "WORLD")) ||
		!Valid_PresentationGeometry(edit) || !Valid_PresentationPlacement(*resource, edit))
		return false;
	KOUKU_MAP_EFFECT_PLACEMENT candidate;
	candidate.strPatternId = pattern->strPatternId;
	candidate.strOccurrenceId = edit.strOccurrenceId;
	candidate.strResourceId = edit.strResourceId;
	candidate.strEffectAssetId = resource->strAssetId;
	candidate.Position = edit.PositionOffset;
	candidate.Scale = edit.Scale;
	candidate.iStartMs = edit.iStartMs;
	candidate.iDurationMs = edit.iDurationMs;
	candidate.iDraftGeneration = m_iDraftGeneration;
	outPlacement = std::move(candidate);
	return true;
}

// Anchor 화면의 Pick/Focus 버튼은 월드 입력을 직접 읽지 않고 이 일회 요청을 예약한다.
// 토큰, 선택 snapshot, Detail 전체 입력을 함께 저장하여 MainApp이 처리하는 동안 다른 편집이 끼면
// 결과를 거부할 수 있게 한다. 잘못된 선택이나 토큰 고갈은 상태만 바꾸고 기존 배치를 유지한다.
bool_t Client::CKoukuSaydonActionWorkbench::Request_MapEffectPlacement(const KOUKU_MAP_EFFECT_PLACEMENT_ACTION action)
{
	KOUKU_MAP_EFFECT_PLACEMENT selection;
	if ((action != KOUKU_MAP_EFFECT_PLACEMENT_ACTION::PICK && action != KOUKU_MAP_EFFECT_PLACEMENT_ACTION::FOCUS) ||
		!Get_MapEffectPlacementSelection(selection, action == KOUKU_MAP_EFFECT_PLACEMENT_ACTION::PICK))
	{
		m_strStatus = "Select a valid World Effect box before picking or focusing its position.";
		return false;
	}
	if (m_iNextMapEffectPlacementToken == (std::numeric_limits<std::uint64_t>::max)())
	{
		m_strStatus = "Placement request token limit reached; reopen this Workbench session.";
		return false;
	}
	m_MapEffectPlacementRequest = {action, ++m_iNextMapEffectPlacementToken, std::move(selection)};
	m_MapEffectPlacementEditSnapshot = m_PresentationBoxEdit;
	m_bMapEffectPlacementRequestPending = true;
	m_strStatus = action == KOUKU_MAP_EFFECT_PLACEMENT_ACTION::PICK
		? "Click a visible world surface outside the tool. Esc or right-click cancels placement."
		: "Requested camera focus on the selected World Effect pivot.";
	return true;
}

// MainApp의 대기 피킹과 완료 처리에서 사용한다. 토큰/명령/선택 generation뿐 아니라
// 요청 당시 Detail 전체 값도 대조하므로 같은 박스를 계속 선택해도 입력이 바뀌면 이전 요청은 만료된다.
bool_t Client::CKoukuSaydonActionWorkbench::Is_MapEffectPlacementRequestCurrent(
	const KOUKU_MAP_EFFECT_PLACEMENT_REQUEST &request) const
{
	KOUKU_MAP_EFFECT_PLACEMENT selection;
	return request.iRequestToken != 0u && request.iRequestToken == m_MapEffectPlacementRequest.iRequestToken &&
		request.eAction == m_MapEffectPlacementRequest.eAction &&
		request.Selection == m_MapEffectPlacementRequest.Selection &&
		Get_MapEffectPlacementSelection(selection, request.eAction == KOUKU_MAP_EFFECT_PLACEMENT_ACTION::PICK) &&
		selection == request.Selection && m_PresentationBoxEdit == m_MapEffectPlacementEditSnapshot;
}

// MainApp이 새 Pick/Focus 요청을 한 번 꺼낸다. pending 표시만 내리고 snapshot은 완료/취소까지
// 남겨 유효성을 다시 검사한다. 이미 만료된 요청은 취소하며 outRequest에는 성공한 요청만 복사한다.
bool_t Client::CKoukuSaydonActionWorkbench::Consume_MapEffectPlacementRequest(
	KOUKU_MAP_EFFECT_PLACEMENT_REQUEST &outRequest)
{
	if (!m_bMapEffectPlacementRequestPending)
		return false;
	m_bMapEffectPlacementRequestPending = false;
	if (!Is_MapEffectPlacementRequestCurrent(m_MapEffectPlacementRequest))
	{
		Cancel_MapEffectPlacementRequest();
		return false;
	}
	outRequest = m_MapEffectPlacementRequest;
	return true;
}

// MainApp 취소와 Workbench 상태 정리가 공유한다. 지정 토큰이 현재 요청과 다르면 무시하여
// 늦은 취소가 새 요청을 지우지 않게 하며, 토큰 0은 현재 요청을 조건 없이 정리한다.
void Client::CKoukuSaydonActionWorkbench::Cancel_MapEffectPlacementRequest(const std::uint64_t token)
{
	if (token && token != m_MapEffectPlacementRequest.iRequestToken)
		return;
	m_MapEffectPlacementRequest = {};
	m_MapEffectPlacementEditSnapshot = {};
	m_bMapEffectPlacementRequestPending = false;
}

// MainApp이 화면 밖 한 번의 피킹으로 구한 world m 위치를 돌려주는 지점이다.
// 요청과 좌표가 유효하면 Detail을 MAP 배치로 바꾸고 기하 preview를 요청한다. 이 단계는 draft 저장이
// 아니며 preview 실패가 반환되어도 이미 갱신한 Detail 입력과 요청 취소를 되돌리지는 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Complete_MapEffectPlacementRequest(
	const KOUKU_MAP_EFFECT_PLACEMENT_REQUEST &request, const std::array<double, 3u> &position, std::string &outStatus)
{
	if (request.eAction != KOUKU_MAP_EFFECT_PLACEMENT_ACTION::PICK || !Is_MapEffectPlacementRequestCurrent(request))
	{
		outStatus = "Placement request expired after the selected box or its edit changed.";
		return false;
	}
	if (!std::all_of(position.begin(), position.end(), [](const double v) {
			return std::isfinite(v) && std::abs(v) <= 100000.0;
		}))
	{
		outStatus = "Picked world position is outside the finite authoring range.";
		return false;
	}
	const auto patternId = request.Selection.strPatternId;
	auto edit = m_PresentationBoxEdit;
	Set_FixedWorldEffectPlacement(edit, position);
	Cancel_MapEffectPlacementRequest(request.iRequestToken);
	m_PresentationBoxEdit = edit;
	// 기하 overlay는 시간 필드와 현재 preview 시계를 유지한다.
	return Request_PresentationGeometryPreview(patternId, edit, outStatus);
}

// MainApp의 preview dispatch가 값 snapshot을 한 번 이동해 받는다. 새 presentation preview를
// 소비할 때 기존 Complete Sequence 예약을 취소하고 pending 내용을 비운다. 요청이 없으면 outRequest를 유지한다.
bool_t Client::CKoukuSaydonActionWorkbench::Consume_PresentationPreviewRequest(
	KOUKU_PRESENTATION_PREVIEW_REQUEST &outRequest)
{
	if (!m_bPresentationPreviewRequestPending)
		return false;
	Cancel_CompleteSequencePlay();
	outRequest = std::move(m_PendingPresentationPreviewRequest);
	m_PendingPresentationPreviewRequest = {};
	m_bPresentationPreviewRequestPending = false;
	return true;
}

// MainApp이 Object 편집 도구를 여는 데 필요한 ID/배치 snapshot을 이동해 받는다.
// 전달 뒤 세션 요청을 비우므로 같은 버튼 입력이 여러 프레임에 중복 처리되지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Consume_WorldObjectEditRequest(KOUKU_WORLD_OBJECT_EDIT_REQUEST &outRequest)
{
	if (m_PendingWorldObjectEditRequest.strObjectId.empty())
		return false;
	outRequest = std::move(m_PendingWorldObjectEditRequest);
	m_PendingWorldObjectEditRequest = {};
	return true;
}

// MainApp의 리소스 수집 결과를 세션에 넘기는 경계다. V1 inventory/organization 읽기에
// 실패한 부분은 이전 메타데이터를 유지하지만 전달받은 주 리소스 목록은 교체한다. 목록 파생 view,
// owner/길이 캐시와 확장 element 선택을 함께 무효화하여 다음 표시가 새 목록을 사용하게 한다.
void Client::CKoukuSaydonActionWorkbench::Set_PresentationResources(
	std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE> resources, std::string status)
{
	// 읽기 결과는 먼저 임시 vector로 받는다. 성공한 자료만 ID 기반 조회 map에 옮겨 실패한 쪽의 이전 메타데이터를 보존한다.
	std::vector<CEffectAuthoringResourceTree::RESOURCE> inventory, organization;
	std::string inventoryStatus, organizationStatus;
	if (CEffectAuthoringResourceTree::Read_V1Inventory(inventory, inventoryStatus))
	{
		m_EffectSourceInventory.clear();
		for (auto &row : inventory)
			m_EffectSourceInventory.emplace(row.strAssetId, std::move(row));
	}
	if (CEffectAuthoringResourceTree::Read_V1Organization(organization, organizationStatus))
	{
		m_EffectSourceOrganization.clear();
		for (auto &row : organization)
			m_EffectSourceOrganization.emplace(row.strAssetId, std::move(row));
	}
	m_strEffectSourceMetadataStatus = inventoryStatus;
	if (!organizationStatus.empty())
		m_strEffectSourceMetadataStatus += (m_strEffectSourceMetadataStatus.empty() ? "" : "\n") + organizationStatus;
	const auto addSource = [&](const auto &entry) {
		if (std::any_of(resources.begin(), resources.end(), [&](const auto &row) {
				return row.strResourceKind == "V1_EFFECT" && row.strAssetId == entry.strAssetId;
			}))
			return;
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE row;
		row.eKind = KOUKU_SAYDON_PRESENTATION_KIND::EFFECT;
		row.strResourceKind = "V1_EFFECT";
		row.strResourceId = "v1:" + entry.strAssetId;
		row.strAssetId = entry.strAssetId;
		row.strDisplayName = entry.strAssetId;
		row.iDurationMs = 3000u;
		resources.push_back(std::move(row));
	};
	for (const auto &[id, entry] : m_EffectSourceInventory)
		addSource(entry);
	for (const auto &[id, entry] : m_EffectSourceOrganization)
		addSource(entry);
	for (auto &row : resources)
	{
		if (row.strResourceKind != "V1_EFFECT")
			continue;
		const auto organized = m_EffectSourceOrganization.find(row.strAssetId);
		const auto saved = m_EffectSourceInventory.find(row.strAssetId);
		if (organized != m_EffectSourceOrganization.end() && !organized->second.strDisplayName.empty())
			row.strDisplayName = organized->second.strDisplayName;
		else if (saved != m_EffectSourceInventory.end() && !saved->second.strDisplayName.empty())
			row.strDisplayName = saved->second.strDisplayName;
	}
	std::vector<EFFECT_TOOL_KOUKU_EFFECT_VIEW> koukuViews;
	for (const auto &row : resources)
	{
		if (row.strResourceKind != "V1_EFFECT")
			continue;
		const auto organized = m_EffectSourceOrganization.find(row.strAssetId);
		const auto saved = m_EffectSourceInventory.find(row.strAssetId);
		const auto path =
			organized == m_EffectSourceOrganization.end() ? std::vector<std::string>{} : organized->second.CategoryPath;
		if (Effect_ResourceOwner(row.strAssetId, path) != "KoukuSaydon")
			continue;
		const std::string name =
			organized != m_EffectSourceOrganization.end() && !organized->second.strDisplayName.empty()
			? organized->second.strDisplayName
			: saved != m_EffectSourceInventory.end() && !saved->second.strDisplayName.empty()
			? saved->second.strDisplayName
			: row.strAssetId;
		koukuViews.push_back(Describe_KoukuSavedEffect(row.strAssetId, name, path));
	}
	m_KoukuEffectResourceTree = Build_KoukuSavedEffectTree(std::move(koukuViews));
	m_PresentationResourceInventory = std::move(resources);
	m_EffectSourceView = {};
	m_CreatedEffectView = {};
	m_SoundSourceView = {};
	m_CreatedSoundView = {};
	m_SoundNativeDurations.clear();
	m_EffectResourceOwners.clear();
	m_strPresentationResourceStatus = std::move(status);
	m_strExpandedV1EffectId.clear();
	m_strV1ElementResourceStatus.clear();
	m_V1ElementResources.clear();
}

// MainApp 리소스 수집이 Refresh 요청을 읽는 일회 플래그다. 실제 목록 교체는 수집 뒤
// Set_PresentationResources에서 수행하며 이 조회만으로 draft나 inventory를 지우지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Consume_PresentationResourceRefreshRequest()
{
	const bool_t requested = m_bPresentationResourceRefreshRequested;
	m_bPresentationResourceRefreshRequested = false;
	return requested;
}

// Effect 그룹 화면의 요청을 공통 그룹 commit으로 전달한다. 하나의 Effect도 그룹 ID를
// 가질 수 있다는 종류별 규칙은 Set_PresentationSelectionGroup에서 판단한다.
bool_t Client::CKoukuSaydonActionWorkbench::Set_EffectSelectionGroup(const std::string_view patternId,
	const std::vector<std::string> &occurrenceIds, const bool_t grouped, std::string &outStatus)
{
	return Set_PresentationSelectionGroup(
		patternId, occurrenceIds, KOUKU_SAYDON_PRESENTATION_KIND::EFFECT, grouped, outStatus);
}

// Sound 그룹 화면이 타임라인 선택을 보존할 그룹 ID를 요청하는 진입점이다.
// 시간/음량은 그대로 두고 공통 그룹 편집이 선택 종류와 개수를 검증해 commit한다.
bool_t Client::CKoukuSaydonActionWorkbench::Set_SoundSelectionGroup(const std::string_view patternId,
	const std::vector<std::string> &occurrenceIds, const bool_t grouped, std::string &outStatus)
{
	return Set_PresentationSelectionGroup(
		patternId, occurrenceIds, KOUKU_SAYDON_PRESENTATION_KIND::SOUND, grouped, outStatus);
}

// Effect/Sound/Collider 그룹 버튼의 공통 문서 편집이다. 기존 그룹의 member까지 선택을
// 확장한 뒤 같은 종류인지 검증하고 후보에 그룹 ID만 지정/해제한다. 성공하면 Timeline 선택을 복구하고
// 무관한 Detail 임시 입력도 되돌려 놓으며 실패하면 이전 문서와 선택을 유지한다.
bool_t Client::CKoukuSaydonActionWorkbench::Set_PresentationSelectionGroup(const std::string_view patternId,
	const std::vector<std::string> &occurrenceIds, const KOUKU_SAYDON_PRESENTATION_KIND kind, const bool_t grouped,
	std::string &outStatus)
{
	const auto reject = [&](const char *reason) {
		outStatus = m_strStatus = reason;
		return false;
	};
	const auto *sourcePattern = Find_Pattern(m_Draft, patternId);
	if (!sourcePattern || !sourcePattern->strLoadError.empty())
		return reject("Select a valid Pattern before grouping boxes.");
	auto selectedIds = occurrenceIds;
	Expand_PresentationSelection(*sourcePattern, selectedIds);
	std::unordered_set<std::string> selected;
	std::erase_if(selectedIds, [&](const auto &id) {
		return !selected.insert(id).second;
	});
	std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE> placements;
	if (grouped && kind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER &&
		!Collect_ColliderPlacements(patternId, selectedIds, placements, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	bool hasGroup = false;
	for (const auto &id : selected)
	{
		const auto *box = Find_PresentationBox(*sourcePattern, id);
		const auto *resource = box ? Find_PresentationResource(m_Draft, box->strResourceId) : nullptr;
		if (!box || !resource || resource->eKind != kind)
			return reject("Select only boxes of the same Effect, Sound or Collider kind.");
		hasGroup |= !box->strSelectionGroupId.empty();
	}
	if (grouped && (selected.empty() || (kind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT && selected.size() < 2u)))
		return reject("Select at least one Effect, two Sounds or two Collider boxes to create a group.");
	if (!grouped && !hasGroup)
		return reject("The selected boxes are not grouped.");
	auto candidate = m_Draft;
	auto *pattern = Find_Pattern(candidate, patternId);
	std::string groupId;
	if (grouped)
	{
		do
		{
			if (pattern->iNextPresentationOccurrenceOrdinal >= 1000000u)
				return reject("Selection group ID limit reached.");
			const auto suffix = kind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT
				? ".effectgroup."
				: (kind == KOUKU_SAYDON_PRESENTATION_KIND::SOUND ? ".soundgroup." : ".collidergroup.");
			groupId = pattern->strPatternId + suffix + std::to_string(pattern->iNextPresentationOccurrenceOrdinal++);
		} while (std::any_of(
			pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(), [&](const auto &box) {
				return box.strSelectionGroupId == groupId;
			}));
	}
	for (auto &box : pattern->PresentationOccurrences)
		if (selected.contains(box.strOccurrenceId))
			box.strSelectionGroupId = groupId;
	const auto detail = m_PresentationBoxEdit;
	const std::string ownerId(patternId);
	const std::string focus = selected.contains(m_strSelectedPresentationOccurrenceId)
		? m_strSelectedPresentationOccurrenceId
		: selectedIds.back();
	if (!Commit_Candidate(std::move(candidate),
			grouped ? "Grouped the selected boxes. One click selects the whole group; Save keeps the group."
					: "Ungrouped the selected boxes. Their positions, rotations and timing are unchanged. Press Save.",
			outStatus))
		return false;
	if (m_strSelectedPatternId != ownerId || m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN)
	{
		std::string selectionStatus;
		(void)Select_PatternById(ownerId, selectionStatus);
	}
	Select_TimelineBox({}, focus, false);
	m_TimelineSelectedOccurrenceIds = std::move(selectedIds);
	m_strTimelineSelectionPatternId = ownerId;
	if (detail.strOccurrenceId == m_strSelectedPresentationOccurrenceId)
	{
		// 메타데이터 명령이 무관한 Detail 입력을 적용하거나 버리지 않도록 보관한다.
		m_PresentationBoxEdit = detail;
		m_PresentationBoxEdit.strSelectionGroupId = groupId;
	}
	m_strColliderSelectionTransformKey.clear();
	m_strStatus = outStatus;
	return true;
}

// Timeline 그룹 드래그가 시작 때의 선택 ID와 generation을 들고 호출한다. 여전히 같은
// 선택일 때만 공통 이동량을 구하고 연결된 Collider Logic 구간도 같은 ms만큼 옮겨 한번에 commit한다.
// 경계에서 이동량이 0이면 성공으로 끝내되 문서 교체/history를 추가하지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Move_PresentationTimelineSelection(const std::string_view patternId,
	const std::vector<std::string> &occurrenceIds, const std::int64_t deltaMs, const std::uint64_t generation,
	std::string &outStatus)
{
	const auto reject = [&](const char *reason) {
		outStatus = m_strStatus = reason;
		return false;
	};
	if (!m_bHasDraft || generation != m_iDraftGeneration || m_strSelectedPatternId != patternId ||
		m_strTimelineSelectionPatternId != patternId || !m_TimelineSelectedStageIds.empty() ||
		m_TimelineSelectedOccurrenceIds != occurrenceIds)
		return reject("The draft or presentation selection changed during dragging; previous timing preserved.");
	auto candidate = m_Draft;
	auto *pattern = Find_Pattern(candidate, patternId);
	if (!pattern)
		return reject("Pattern is unavailable; previous timing preserved.");
	PRESENTATION_TIMELINE_MOVE move;
	if (!Prepare_PresentationTimelineMove(candidate, *pattern, occurrenceIds, deltaMs, move, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	if (move.deltaMs == 0)
	{
		outStatus = m_strStatus = "Selection is at the Pattern time boundary.";
		return true;
	}
	for (const auto &id : move.occurrenceIds)
	{
		auto box = std::find_if(
			pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(), [&](const auto &row) {
				return row.strOccurrenceId == id;
			});
		box->iStartMs = static_cast<std::uint32_t>(std::int64_t(box->iStartMs) + move.deltaMs);
	}
	for (const auto &id : move.logicIds)
	{
		auto *box = Find_LogicBox(*pattern, id);
		box->iStartMs = static_cast<std::uint32_t>(std::int64_t(box->iStartMs) + move.deltaMs);
	}
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate),
		"Moved the selection and any linked Collider Logic windows together. Relative timing and anchors are preserved; press Save.",
		outStatus);
}

// Effect 그룹 화면과 Transform_SelectedEffects가 사용할 사본 목록을 만든다. 그룹 member를
// 확장하고 staged placement를 합친 뒤 같은 좌표계인지 검증한다. 실패한 outBoxes의 부분 결과는
// 사용하지 않아야 하며 이 조회 자체는 원본 draft나 overlay를 바꾸지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Collect_EffectPlacements(const std::string_view patternId,
	const std::vector<std::string> &occurrenceIds,
	std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE> &outBoxes, std::string &outStatus) const
{
	outBoxes.clear();
	const auto reject = [&](const char *reason) {
		outStatus = reason;
		return false;
	};
	const auto *pattern = Find_Pattern(m_Draft, patternId);
	if (!pattern || !pattern->strLoadError.empty() || !Pattern_DurationMs(*pattern))
		return reject("Select a valid Pattern before moving Effect groups.");
	auto expandedIds = occurrenceIds;
	Expand_PresentationSelection(*pattern, expandedIds);
	std::unordered_set<std::string> selected;
	for (const auto &id : expandedIds)
	{
		if (!selected.insert(id).second)
			continue;
		const auto *source = Find_PresentationBox(*pattern, id);
		const auto *resource = source ? Find_PresentationResource(m_Draft, source->strResourceId) : nullptr;
		if (!source || !resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)
			return reject("Select only Effect boxes from this Pattern.");
		auto box = *source;
		for (const auto &staged : m_StagedPresentationGeometry)
			if (staged.strPatternId == patternId && staged.Occurrence.strOccurrenceId == id)
			{
				if (staged.Occurrence.strResourceId != source->strResourceId)
					return reject("Effect placement identity changed. Reopen the selected group.");
				Copy_PresentationPlacement(staged.Occurrence, box, true);
				break;
			}
		if (!Valid_PresentationPlacement(*resource, box))
			return reject("Correct or revert invalid Effect placement before moving the group.");
		// MAP 위치는 Effect 시작 시각이 달라도 같은 world 좌표계를 사용한다.
		// 이동/고정 BOSS와 WORLD offset은 샘플한 anchor 좌표계가 정확히 같아야 한다.
		if (!outBoxes.empty() && !(outBoxes.front().strAnchorKind == "MAP" && box.strAnchorKind == "MAP") &&
			!Same_ColliderGroupFrame(outBoxes.front(), box))
			return reject(
				"Group position requires one coordinate frame: fixed MAP, or the same Boss/Object, bone and Follow setting. Frozen Boss/Object anchors also require the same start time. Grouping and timeline movement remain available.");
		outBoxes.push_back(std::move(box));
	}
	if (outBoxes.size() < 2u)
		return reject("Select at least two Effect boxes.");
	return true;
}

// Effect 그룹 Detail의 이동량(m)과 회전 quaternion을 받아 공통 중심 주위의 배치를 계산한다.
// 모든 사본 검증 뒤 history/overlay를 갱신하고 preview를 요청한다. preview를 시작할 수 없어도
// staged 배치는 Save 대상으로 유지하므로 이 경우 true와 별도 상태 메시지를 반환한다.
bool_t Client::CKoukuSaydonActionWorkbench::Transform_SelectedEffects(
	const std::array<double, 3u> &translation, const std::array<double, 4u> &rotationDelta, std::string &outStatus)
{
	const auto reject = [&](const char *reason) {
		outStatus = m_strStatus = reason;
		return false;
	};
	if (m_ePatternSelection != KOUKU_PATTERN_SELECTION::PATTERN ||
		m_strTimelineSelectionPatternId != m_strSelectedPatternId || !m_TimelineSelectedStageIds.empty())
		return reject("Select only Effect boxes from the current Pattern.");
	if (!std::all_of(translation.begin(), translation.end(),
			[](double value) {
				return std::isfinite(value);
			}) ||
		!std::all_of(rotationDelta.begin(), rotationDelta.end(), [](double value) {
			return std::isfinite(value);
		}))
		return reject("Group translation and rotation must be finite. Previous placements are preserved.");
	const double rotationLength =
		std::hypot(std::hypot(rotationDelta[0], rotationDelta[1]), std::hypot(rotationDelta[2], rotationDelta[3]));
	if (!std::isfinite(rotationLength) || rotationLength <= std::numeric_limits<double>::epsilon())
		return reject("Group rotation must be a valid quaternion. Previous placements are preserved.");
	const auto delta = XMVectorSet(float(rotationDelta[0] / rotationLength), float(rotationDelta[1] / rotationLength),
		float(rotationDelta[2] / rotationLength), float(rotationDelta[3] / rotationLength));
	const bool rotated = rotationDelta[0] != 0.0 || rotationDelta[1] != 0.0 || rotationDelta[2] != 0.0;
	DirectX::XMFLOAT4X4 rotation;
	XMStoreFloat4x4(&rotation, XMMatrixRotationQuaternion(delta));
	std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE> boxes;
	if (!Collect_EffectPlacements(m_strSelectedPatternId, m_TimelineSelectedOccurrenceIds, boxes, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	std::array<double, 3u> center{};
	for (const auto &box : boxes)
		for (std::size_t axis = 0u; axis < 3u; ++axis)
			center[axis] += box.PositionOffset[axis] / double(boxes.size());
	for (auto &box : boxes)
	{
		if (rotated)
		{
			const std::array<double, 3u> relative = {box.PositionOffset[0] - center[0],
				box.PositionOffset[1] - center[1], box.PositionOffset[2] - center[2]};
			for (std::size_t axis = 0u; axis < 3u; ++axis)
				box.PositionOffset[axis] = center[axis] + relative[0] * rotation.m[0][axis] +
					relative[1] * rotation.m[1][axis] + relative[2] * rotation.m[2][axis];
			const auto previous = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(float(box.RotationDegrees[0])),
				XMConvertToRadians(float(box.RotationDegrees[1])), XMConvertToRadians(float(box.RotationDegrees[2])));
			// DirectX 행 벡터 순서에 따라 local 회전 뒤 같은 anchor 좌표계의 그룹 회전 변화량을 합성한다.
			DirectX::SimpleMath::Quaternion composed;
			XMStoreFloat4(&composed, XMQuaternionNormalize(XMQuaternionMultiply(previous, delta)));
			const auto angles = composed.ToEuler();
			box.RotationDegrees = {double(XMConvertToDegrees(angles.x)), double(XMConvertToDegrees(angles.y)),
				double(XMConvertToDegrees(angles.z))};
		}
		for (std::size_t axis = 0u; axis < 3u; ++axis)
			box.PositionOffset[axis] += translation[axis];
		if (!Valid_PresentationGeometry(box))
			return reject("Group transform exceeds an Effect's authoring range. The entire selection is unchanged.");
	}
	const std::string patternId = m_strSelectedPatternId;
	// 검증한 모든 member를 stage한 뒤 preview를 요청한다. 최초 preview 준비와
	// Save 모두 pending 편집을 포함한 전체 그룹을 보게 한다.
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
			Copy_PresentationPlacement(box, m_PresentationBoxEdit, true);
	}
	for (const auto &box : boxes)
		if (!Request_PresentationGeometryPreview(patternId, box, outStatus))
		{
			outStatus = m_strStatus = "Group transform is staged for Save; preview is unavailable: " + outStatus;
			return true;
		}
	outStatus = m_strStatus =
		"Transformed the Effect group. Relative placement, timing and scale are preserved; Save keeps the placement.";
	return true;
}

// Render_Details가 여러 Sound 선택을 단일 박스 화면보다 먼저 처리한다. 반환 true는 그룹
// 화면이 Detail을 맡았다는 뜻이다. 그룹 commit 뒤에는 차용한 pattern을 더 사용하지 않고 끝낸다.
bool_t Client::CKoukuSaydonActionWorkbench::Render_SoundGroupDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern)
{
	if (m_strTimelineSelectionPatternId != pattern.strPatternId || !m_TimelineSelectedStageIds.empty() ||
		m_TimelineSelectedOccurrenceIds.size() < 2u)
		return false;
	bool anyGroup = false, sameGroup = true;
	std::string firstGroup;
	for (const auto &id : m_TimelineSelectedOccurrenceIds)
	{
		const auto *box = Find_PresentationBox(pattern, id);
		const auto *resource = box ? Find_PresentationResource(m_Draft, box->strResourceId) : nullptr;
		if (!resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::SOUND)
			return false;
		if (id == m_TimelineSelectedOccurrenceIds.front())
			firstGroup = box->strSelectionGroupId;
		anyGroup |= !box->strSelectionGroupId.empty();
		sameGroup &= !box->strSelectionGroupId.empty() && box->strSelectionGroupId == firstGroup;
	}
	ImGui::SeparatorText("Sound selection");
	ImGui::Text("%zu boxes selected", m_TimelineSelectedOccurrenceIds.size());
	ImGui::BeginDisabled(sameGroup);
	const bool setGroup = ImGui::Button("Set Group");
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!anyGroup);
	const bool ungroup = ImGui::Button("Ungroup");
	ImGui::EndDisabled();
	if (setGroup || ungroup)
	{
		std::string status;
		(void)Set_SoundSelectionGroup(pattern.strPatternId, m_TimelineSelectedOccurrenceIds, setGroup, status);
		return true;
	}
	if (sameGroup)
		ImGui::TextDisabled("%s", firstGroup.c_str());
	ImGui::TextWrapped(
		"Drag the middle of any selected Sound box to move the whole selection. Set Group keeps that selection after Save. Each source window, duration, fade and volume stays unchanged. Ungroup to trim individual boxes.");
	return true;
}

// Render_Details의 Effect 그룹 편집기다. 그룹 메타데이터, 전체 배치 preview, 복제/Save 명령을
// 각 경계 함수로 보낸다. singleton의 그룹 버튼을 누르지 않은 프레임은 false로 반환하여 아래의
// 일반 박스 시간/배치 입력도 표시하고, 문서 교체를 요청한 분기는 즉시 반환한다.
bool_t Client::CKoukuSaydonActionWorkbench::Render_EffectGroupDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern)
{
	if (m_strTimelineSelectionPatternId != pattern.strPatternId || !m_TimelineSelectedStageIds.empty() ||
		m_TimelineSelectedOccurrenceIds.empty())
		return false;
	if (m_TimelineSelectedOccurrenceIds.size() == 1u)
	{
		const auto *box = Find_PresentationBox(pattern, m_TimelineSelectedOccurrenceIds.front());
		const auto *resource = box ? Find_PresentationResource(m_Draft, box->strResourceId) : nullptr;
		if (!resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)
			return false;
		const bool grouped = !box->strSelectionGroupId.empty();
		if (ImGui::Button(grouped ? "Ungroup Effect" : "Set Effect Group"))
		{
			std::string status;
			(void)Set_EffectSelectionGroup(pattern.strPatternId, m_TimelineSelectedOccurrenceIds, !grouped, status);
			return true;
		}
		if (grouped)
			ImGui::TextDisabled("%s", box->strSelectionGroupId.c_str());
		return false; // 하나만 선택했으면 아래의 일반 시간/배치 입력도 계속 표시한다.
	}
	bool anyGroup = false, sameGroup = true;
	std::string firstGroup;
	for (const auto &id : m_TimelineSelectedOccurrenceIds)
	{
		const auto *box = Find_PresentationBox(pattern, id);
		const auto *resource = box ? Find_PresentationResource(m_Draft, box->strResourceId) : nullptr;
		if (!resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)
			return false;
		if (id == m_TimelineSelectedOccurrenceIds.front())
			firstGroup = box->strSelectionGroupId;
		anyGroup |= !box->strSelectionGroupId.empty();
		sameGroup &= !box->strSelectionGroupId.empty() && box->strSelectionGroupId == firstGroup;
	}
	ImGui::SeparatorText("Effect selection");
	ImGui::Text("%zu boxes selected", m_TimelineSelectedOccurrenceIds.size());
	ImGui::BeginDisabled(sameGroup);
	const bool setGroup = ImGui::Button("Set Group");
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!anyGroup);
	const bool ungroup = ImGui::Button("Ungroup");
	ImGui::EndDisabled();
	if (setGroup || ungroup)
	{
		std::string status;
		(void)Set_EffectSelectionGroup(pattern.strPatternId, m_TimelineSelectedOccurrenceIds, setGroup, status);
		return true;
	}
	if (sameGroup)
		ImGui::TextDisabled("%s", firstGroup.c_str());
	ImGui::TextWrapped(
		"Drag a grouped box in the timeline to move its timing. Group position and rotation move every member together, keeping its relative placement and timing. Ungroup to edit one member.");
	std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE> boxes;
	std::string reason;
	const bool movable = Collect_EffectPlacements(pattern.strPatternId, m_TimelineSelectedOccurrenceIds, boxes, reason);
	if (movable)
	{
		std::vector<std::string> ids;
		for (const auto &box : boxes)
			ids.push_back(box.strOccurrenceId);
		std::sort(ids.begin(), ids.end());
		std::string key = pattern.strPatternId;
		for (const auto &id : ids)
			key += "|" + id;
		if (key != m_strEffectSelectionTransformKey)
		{
			m_strEffectSelectionTransformKey = std::move(key);
			m_EffectSelectionRotationDegrees = {};
		}
		std::array<double, 3u> center{};
		for (const auto &box : boxes)
			for (std::size_t axis = 0u; axis < 3u; ++axis)
				center[axis] += box.PositionOffset[axis] / double(boxes.size());
		float position[3] = {float(center[0]), float(center[1]), float(center[2])};
		const bool fixedMap = boxes.front().strAnchorKind == "MAP";
		const bool moved = ImGui::DragFloat3(fixedMap ? "Group position (world m)" : "Group center offset (m)",
			position, .05f, -100000.f, 100000.f, "%.3f");
		auto angles = m_EffectSelectionRotationDegrees;
		const bool rotated = ImGui::DragFloat3("Group rotation (deg)", angles.data(), .25f, -36000.f, 36000.f, "%.2f");
		if (moved || rotated)
		{
			std::array<double, 3u> translation{};
			if (moved)
				for (std::size_t axis = 0u; axis < 3u; ++axis)
					translation[axis] = double(position[axis]) - center[axis];
			std::array<double, 4u> delta = {0.0, 0.0, 0.0, 1.0};
			if (!std::all_of(angles.begin(), angles.end(), [](float value) {
					return std::isfinite(value) && std::abs(value) <= 36000.f;
				}))
				m_strStatus =
					"Group rotation must be finite and within +/-36000 degrees. Previous placements are preserved.";
			else
			{
				if (rotated)
				{
					const auto previous =
						XMQuaternionRotationRollPitchYaw(XMConvertToRadians(m_EffectSelectionRotationDegrees[0]),
							XMConvertToRadians(m_EffectSelectionRotationDegrees[1]),
							XMConvertToRadians(m_EffectSelectionRotationDegrees[2]));
					const auto next = XMQuaternionRotationRollPitchYaw(
						XMConvertToRadians(angles[0]), XMConvertToRadians(angles[1]), XMConvertToRadians(angles[2]));
					DirectX::XMFLOAT4 quaternion;
					// 같은 anchor 좌표계에서 이전 그룹 회전을 풀고 새 XYZ 회전을 적용한다.
					XMStoreFloat4(
						&quaternion, XMQuaternionNormalize(XMQuaternionMultiply(XMQuaternionInverse(previous), next)));
					delta = {quaternion.x, quaternion.y, quaternion.z, quaternion.w};
				}
				std::string status;
				if (Transform_SelectedEffects(translation, delta, status))
					m_EffectSelectionRotationDegrees = angles;
			}
		}
		ImGui::TextDisabled(fixedMap ? "Position is the average of all Effect pivots in world coordinates."
									 : "Position is the average of all Effect offsets in the shared anchor.");
		ImGui::TextWrapped(
			"Rotation is relative to this selection and turns every Effect around that center in the shared anchor. Scale and timing stay unchanged.");
	}
	else
		ImGui::TextWrapped("%s", reason.c_str());
	ImGui::BeginDisabled(!sameGroup);
	const bool duplicate = ImGui::Button("Duplicate Group (same time)");
	ImGui::EndDisabled();
	if (duplicate)
	{
		std::string status;
		(void)Dispatch_CompositionEdit(
			COMPOSITION_EDIT_COMMAND::DUPLICATE_SELECTION, {}, {},
			[&](std::string &message) {
				return Duplicate_TimelineSelection(
					pattern.strPatternId, {}, m_TimelineSelectedOccurrenceIds, message, true);
			},
			status);
		return true;
	}
	ImGui::TextWrapped(
		"The copy keeps the same start times and gets its own group. Change its position to place the sequence at another spot. Ctrl+D duplicates after the selection inside this Pattern; Ctrl+C then Ctrl+V pastes the same linked set at the end of another Pattern.");
	ImGui::BeginDisabled(!Is_Dirty() || Is_PublishRunning());
	const bool save = ImGui::Button(m_bSequenceWorkspace ? "Save Sequence" : "Save Composition");
	ImGui::EndDisabled();
	if (save)
	{
		std::string status;
		(void)Save(status);
		return true;
	}
	return true;
}

// Resources의 Create 버튼이 source 사본에 새 stable resource ID와 표시 이름을 부여한다.
// V1 길이 갱신과 전체 후보 검증을 통과한 뒤에만 draft/선택을 바꾼다. 실제 Timeline 박스는 아직 만들지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Create_PresentationResource(
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE &source, const std::string_view displayName,
	std::string &outStatus)
{
	auto candidate = m_Draft;
	if (candidate.iNextPresentationResourceOrdinal >= 1000000u)
	{
		outStatus = m_strStatus = "Presentation resource IDs are exhausted.";
		return false;
	}
	auto row = source;
	if (!Refresh_V1ResourceDuration(row, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	row.strResourceId = "koukusaydon.g1.presentation." + std::to_string(candidate.iNextPresentationResourceOrdinal++);
	row.strDisplayName = std::string(displayName);
	const auto id = row.strResourceId;
	candidate.PresentationResources.push_back(std::move(row));
	if (!Commit_Candidate(
			std::move(candidate), "Created presentation resource. Append places it on the Pattern clock.", outStatus))
		return false;
	m_strSelectedPresentationResourceId = id;
	return true;
}

// Resources에서 이미 만든 resource를 선택해 Append할 때 사용한다. draft 사본을 넘기며
// 실제 박스 ID 발급, 시간 범위 계산과 commit은 Append_PresentationCandidate가 담당한다.
bool_t Client::CKoukuSaydonActionWorkbench::Append_PresentationBox(
	const std::string_view resourceId, std::string &outStatus)
{
	return Append_PresentationCandidate(m_Draft, resourceId, outStatus);
}

// Source 직접 Append와 Sequence Effect preview가 리소스를 준비하는 공통 단계다.
// 종류/asset/element와 Light anchor가 같은 resource를 재사용하고 없으면 후보에 추가하여 ID를 반환한다.
// 전달받은 후보만 변경하며 session draft에 commit할지 preview snapshot으로 쓸지는 호출자가 정한다.
bool_t Client::CKoukuSaydonActionWorkbench::Stage_PresentationSource(KOUKU_SAYDON_COMPOSITION_DOCUMENT &candidate,
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE &source, std::string &resourceId, std::string &outStatus)
{
	auto currentSource = source;
	if (!Refresh_V1ResourceDuration(currentSource, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	const auto existing = std::find_if(
		candidate.PresentationResources.begin(), candidate.PresentationResources.end(), [&](const auto &item) {
			return item.eKind == source.eKind && item.strAssetId == source.strAssetId &&
				item.strResourceKind == source.strResourceKind && item.strElementId == source.strElementId &&
				(item.eKind != KOUKU_SAYDON_PRESENTATION_KIND::LIGHT ||
					item.strDefaultAnchorKind == source.strDefaultAnchorKind);
		});
	if (existing != candidate.PresentationResources.end())
	{
		resourceId = existing->strResourceId;
		if (source.strResourceKind == "V1_EFFECT" || source.strResourceKind == "V1_ELEMENT")
			existing->iDurationMs = currentSource.iDurationMs;
	}
	else
	{
		if (candidate.iNextPresentationResourceOrdinal >= 1000000u)
		{
			outStatus = m_strStatus = "Presentation resource IDs are exhausted.";
			return false;
		}
		auto resource = currentSource;
		resourceId = "koukusaydon.g1.presentation." + std::to_string(candidate.iNextPresentationResourceOrdinal++);
		resource.strResourceId = resourceId;
		candidate.PresentationResources.push_back(std::move(resource));
	}
	return true;
}

// Resources 또는 Camera 창에서 source를 바로 Timeline에 추가하는 경로다.
// Stage_PresentationSource와 박스 생성을 같은 후보에서 처리하므로 박스 추가 실패로 리소스만 남지 않는다.
// 성공한 뒤에만 생성/재사용한 resource를 세션 선택으로 지정한다.
bool_t Client::CKoukuSaydonActionWorkbench::Append_PresentationSource(
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE &source, std::string &outStatus)
{
	auto candidate = m_Draft;
	std::string resourceId;
	if (!Stage_PresentationSource(candidate, source, resourceId, outStatus))
		return false;
	// 리소스 참조와 occurrence를 하나의 commit으로 draft에 추가한다.
	// GROUP은 고유 ID를 유지하며 자식과 세부 튜닝은 V2가 계속 소유한다.
	if (!Append_PresentationCandidate(std::move(candidate), resourceId, outStatus))
		return false;
	m_strSelectedPresentationResourceId = resourceId;
	return true;
}

// Source/기존 resource Append의 공통 commit 경계다. Bundle에는 CAMERA 공통 박스를,
// Pattern에는 현재 커서와 유효 수명 안의 occurrence를 만든다. Effect의 초기 anchor/위치 결정에
// 실패하면 전달받은 후보를 버리며 기존 draft의 ordinal과 박스 목록은 보존한다.
bool_t Client::CKoukuSaydonActionWorkbench::Append_PresentationCandidate(
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate, const std::string_view resourceId, std::string &outStatus)
{
	if (!m_bHasDraft)
	{
		outStatus = m_strStatus = "Load a composition before appending a presentation resource.";
		return false;
	}
	if (m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE)
	{
		auto *bundle = Find_Bundle(candidate, m_strSelectedBundleId);
		const auto *resource = Find_PresentationResource(candidate, resourceId);
		if (!bundle || !bundle->strLoadError.empty() || bundle->iNextPresentationOccurrenceOrdinal >= 1000000u ||
			!resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::CAMERA)
		{
			outStatus = m_strStatus =
				"A bundle accepts common Camera and Scene Profile lanes. Select a child for actor lanes.";
			return false;
		}
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE row;
		row.strOccurrenceId =
			bundle->strBundleId + ".presentation." + std::to_string(bundle->iNextPresentationOccurrenceOrdinal++);
		row.strResourceId = std::string(resourceId);
		row.iStartMs = (std::min)(m_iCursorMs, MAX_EDITOR_TIME_MS - 1u);
		row.iDurationMs = (std::clamp)(Camera_DefaultDuration(*resource), 1u, MAX_EDITOR_TIME_MS - row.iStartMs);
		row.strAnchorKind = "BOSS";
		row.bFollowBoss = false;
		bundle->PresentationOccurrences.push_back(row);
		bundle->strAuthoringStatus = "DRAFT";
		return Commit_Candidate(std::move(candidate), "Appended common Camera.", outStatus);
	}
	auto *pattern = Find_Pattern(candidate, m_strSelectedPatternId);
	const auto *resource = Find_PresentationResource(candidate, resourceId);
	if (nullptr == pattern || !pattern->strLoadError.empty() || nullptr == resource ||
		pattern->iNextPresentationOccurrenceOrdinal >= 1000000u || Pattern_DurationMs(*pattern) == 0u)
	{
		outStatus = m_strStatus = "Append requires a valid resource and a Pattern with a lifetime.";
		return false;
	}
	auto currentResource = *resource;
	if (!Refresh_V1ResourceDuration(currentResource, outStatus))
	{
		m_strStatus = outStatus;
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE row;
	row.strOccurrenceId =
		pattern->strPatternId + ".presentation." + std::to_string(pattern->iNextPresentationOccurrenceOrdinal++);
	row.strResourceId = std::string(resourceId);
	row.iStartMs = (std::min)(m_iCursorMs, Pattern_DurationMs(*pattern) - 1u);
	row.iDurationMs = (std::min)(Camera_DefaultDuration(currentResource), Pattern_DurationMs(*pattern) - row.iStartMs);
	if (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::LIGHT ||
		resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
		resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::SUBTITLE)
	{
		row.strAnchorKind = resource->strDefaultAnchorKind;
		row.bFollowBoss = row.strAnchorKind != "MAP";
	}
	if (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT &&
		(m_bSequenceWorkspace || row.strAnchorKind == "WORLD"))
	{
		// 리소스 기본값만으로 배치된 Object를 식별할 수 없다. 이 occurrence에
		// Object를 명시적으로 고르기 전까지 유효한 고정 위치를 사용한다.
		Set_FixedWorldEffectPlacement(row, row.PositionOffset);
		if (m_bSequenceWorkspace)
			Configure_SequenceEffectOccurrence(*pattern, currentResource, row);
	}
	if (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT && row.strAnchorKind == "MAP")
	{
		if (!Resolve_PresentationBossSpawnPosition(candidate.strAreaId, *pattern, row.PositionOffset, outStatus))
		{
			m_strStatus = outStatus;
			return false;
		}
	}
	if (resource->strColliderKind == "ROULETTE_CARD_REGION")
	{
		row.strRegionId = row.strOccurrenceId + ".region";
		row.strAnchorKind = "WORLD";
	}
	const auto id = row.strOccurrenceId;
	pattern->PresentationOccurrences.push_back(row);
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(
			std::move(candidate), "Appended presentation box. Select it to tune timing and presentation.", outStatus))
		return false;
	Clear_TimelineSelection();
	m_strSelectedPresentationOccurrenceId = id;
	m_PresentationBoxEdit = row;
	m_strTimelineSelectionPatternId = m_strSelectedPatternId;
	return true;
}

// Timeline 양끝 드래그와 Detail의 길이 입력이 편집 사본만 조정하는 계산 단계다.
// ms 단위 시작/길이와 source 시작을 함께 바꾸고 Effect 시간 key가 있으면 남긴 구간을 다시 기준화한다.
// 호출자는 유효한 기존 박스를 넘겨야 하며 결과의 문서 commit과 preview는 호출자가 수행한다.
void Client::CKoukuSaydonActionWorkbench::Trim_PresentationWindow(KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &row,
	const std::int64_t deltaMs, const bool_t trimStart, const std::uint32_t timelineDurationMs,
	const std::uint32_t sourceDurationMs, const bool_t effect)
{
	if (effect && !row.EffectSourceTimeKeys.empty())
	{
		const double first = trimStart ? double(std::clamp<std::int64_t>(deltaMs, 0, row.iDurationMs - 1u)) : 0.;
		const double last = trimStart
			? double(row.iDurationMs)
			: double(std::clamp<std::int64_t>(std::int64_t(row.iDurationMs) + deltaMs, 1, row.iDurationMs));
		std::vector<std::array<double, 2u>> keys{{0., row.Effect_SourceTimeMs(first)}};
		for (const auto &key : row.EffectSourceTimeKeys)
			if (key[0] > first && key[0] < last)
				keys.push_back({key[0] - first, key[1]});
		keys.push_back({last - first, row.Effect_SourceTimeMs(last)});
		row.iStartMs += static_cast<std::uint32_t>(first);
		row.iDurationMs = static_cast<std::uint32_t>(last - first);
		row.EffectSourceTimeKeys = std::move(keys);
		return;
	}
	std::int64_t start = row.iStartMs, length = row.iDurationMs;
	std::int64_t source = effect ? row.iEffectSourceStartMs : row.iSoundSourceStartMs;
	if (length < 1 || (sourceDurationMs && source >= sourceDurationMs))
		return;
	const bool fit = effect && row.bFitEffectToDuration;
	const bool loop = effect && row.bLoopEffectToDuration;
	const double rate = fit && sourceDurationMs ? double(sourceDurationMs - source) / double(length) : 1.0;
	if (trimStart)
	{
		// fit된 박스도 잘라 남긴 구간의 원본 재생 속도를 유지한다.
		const auto minimum = (std::max)(-start, -std::int64_t(std::floor(double(source) / rate)));
		auto maximum = length - 1;
		if (sourceDurationMs)
			maximum = (std::min)(maximum, std::int64_t(std::floor(double(sourceDurationMs - source - 1) / rate)));
		const auto delta = std::clamp(deltaMs, minimum, maximum);
		start += delta;
		source += std::llround(double(delta) * rate);
		length -= delta;
	}
	else
	{
		auto maximum = (std::max)(std::int64_t{1}, std::int64_t(timelineDurationMs) - start);
		if (sourceDurationMs && !fit && !loop)
			maximum = (std::min)(maximum, (std::max)(std::int64_t{1}, std::int64_t(sourceDurationMs) - source));
		length = std::clamp(length + deltaMs, std::int64_t{1}, maximum);
	}
	row.iStartMs = static_cast<std::uint32_t>(start);
	row.iDurationMs = static_cast<std::uint32_t>(length);
	if (effect)
		row.iEffectSourceStartMs = static_cast<std::uint32_t>(source);
	else
		row.iSoundSourceStartMs = static_cast<std::uint32_t>(source);
}

// Detail Apply와 Timeline 편집이 occurrence 값 전체를 확정하는 경계다. 현재 resource ID를
// 재검증하고 연결 해제된 전용 접촉 구간 정리와 공유 Logic 시간 동기화를 같은 후보에서 처리한다.
// 검증/commit 실패는 기존 draft와 호출자의 미적용 입력을 유지한다.
bool_t Client::CKoukuSaydonActionWorkbench::Set_PresentationBox(const std::string_view patternId,
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &value, std::string &outStatus)
{
	auto candidate = m_Draft;
	auto *pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern || !pattern->strLoadError.empty())
	{
		outStatus = m_strStatus = "Presentation box Pattern is unavailable.";
		return false;
	}
	const auto found = std::find_if(
		pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(), [&](const auto &row) {
			return row.strOccurrenceId == value.strOccurrenceId;
		});
	if (found == pattern->PresentationOccurrences.end() || found->strResourceId != value.strResourceId)
	{
		outStatus = m_strStatus = "Presentation box identity changed; previous row preserved.";
		return false;
	}
	const auto previousWindowId = found->strLogicOccurrenceId;
	*found = value;
	if (!previousWindowId.empty() && value.strLogicOccurrenceId.empty() &&
		std::none_of(
			pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(), [&](const auto &row) {
				return row.strLogicOccurrenceId == previousWindowId;
			}))
	{
		const auto *previous = Find_LogicBox(*pattern, previousWindowId);
		const auto *logic = previous ? Find_Logic(candidate, previous->strLogicId) : nullptr;
		if (logic && logic->strTriggerKind == "ENTER_AREA")
			std::erase_if(pattern->LogicOccurrences, [&](const auto &row) {
				return row.strOccurrenceId == previousWindowId;
			});
	}
	const auto *resource = Find_PresentationResource(candidate, found->strResourceId);
	if (resource && !Valid_GameplayColliderScale(*resource, *found))
	{
		outStatus = m_strStatus =
			"Apply rejected: Collider scale must be finite and positive. Adjust Scale / size; pending edits and previous draft are preserved.";
		return false;
	}
	if (!value.strLogicOccurrenceId.empty())
	{
		auto *linked = Find_LogicBox(*pattern, value.strLogicOccurrenceId);
		if (nullptr == linked)
		{
			outStatus = m_strStatus = "Linked Logic window is unavailable.";
			return false;
		}
		linked->iStartMs = value.iStartMs;
		linked->iDurationMs = value.iDurationMs;
		for (auto &collider : pattern->PresentationOccurrences)
			if (collider.strLogicOccurrenceId == value.strLogicOccurrenceId)
			{
				collider.iStartMs = value.iStartMs;
				collider.iDurationMs = value.iDurationMs;
			}
	}
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate), "Updated presentation box. Save keeps its tuning.", outStatus);
}

// Presentation Detail의 Delete가 occurrence ID로 후보 행을 제거한다. 전체 후보 commit이
// 성공한 경우에만 관련 선택 ID를 비우며 resource 정의 자체는 이 경로에서 삭제하지 않는다.
bool_t Client::CKoukuSaydonActionWorkbench::Delete_PresentationBox(
	const std::string_view patternId, const std::string_view occurrenceId, std::string &outStatus)
{
	auto candidate = m_Draft;
	auto *pattern = Find_Pattern(candidate, patternId);
	if (nullptr == pattern || !pattern->strLoadError.empty())
		return false;
	const auto removed = std::erase_if(pattern->PresentationOccurrences, [occurrenceId](const auto &row) {
		return row.strOccurrenceId == occurrenceId;
	});
	if (removed == 0u)
	{
		outStatus = m_strStatus = "Presentation box was not found.";
		return false;
	}
	Mark_Draft(candidate, *pattern);
	if (!Commit_Candidate(std::move(candidate), "Deleted presentation box.", outStatus))
		return false;
	m_strSelectedPresentationOccurrenceId.clear();
	m_strSelectedPatternOccurrenceId.clear();
	return true;
}

// World Detail의 Object/Motion 편집 버튼이 MainApp에 넘길 요청을 구성한다. resource의
// Object ID와 필요한 Motion을 찾지 못하면 기존 편집을 유지한다. 성공 시 ID와 preview 배치를 값으로
// 보관하므로 다른 도구가 요청을 소비할 때 Workbench 문서 포인터를 차용하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Queue_WorldObjectEdit(const std::string_view patternId,
	const std::string_view occurrenceId, const std::string_view motionId, const bool focusAnimationClips)
{
	const auto *pattern = Find_Pattern(m_Draft, patternId);
	const auto *box = pattern ? Find_WorldBox(*pattern, occurrenceId) : nullptr;
	const auto *world = box ? Find_World(m_Draft, box->strWorldId) : nullptr;
	if (!world)
		return;
	const auto source =
		std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(), [&](const auto &row) {
			return row.strInstanceId == world->strSequenceInstanceId;
		});
	KOUKU_WORLD_OBJECT_EDIT_REQUEST request;
	request.strObjectId = !world->strObjectResourceId.empty()
		? world->strObjectResourceId
		: (source != m_WorldSequenceResources.end() ? source->strObjectResourceId : std::string{});
	if (request.strObjectId.empty())
	{
		m_strStatus = "The owning Object is unavailable. Existing World edits are preserved.";
		return;
	}
	request.strMotionInstanceId = std::string(motionId);
	if (focusAnimationClips && request.strMotionInstanceId.empty())
	{
		if (source == m_WorldSequenceResources.end() || source->bMotionGroup)
		{
			m_strStatus = "Choose a linked Motion to edit its Animation Clips.";
			return;
		}
		request.strMotionInstanceId = source->strInstanceId;
	}
	request.strPatternId = std::string(patternId);
	request.strOccurrenceId = std::string(occurrenceId);
	request.bSequenceWorkspace = m_bSequenceWorkspace;
	request.bExplicitPlacement = box->Placement.has_value();
	request.bFocusAnimationClips = focusAnimationClips;
	if (source != m_WorldSequenceResources.end() && source->bSupportsPlacement && source->strAnchorKind == "WORLD" &&
		(box->Placement || world->strAnchorKind == "NONE"))
	{
		request.PreviewPlacement = box->Placement.value_or(KOUKU_SAYDON_WORLD_PLACEMENT{});
		if (!box->Placement)
			for (std::size_t axis = 0u; axis < 3u; ++axis)
				request.PreviewPlacement->Position[axis] = source->SavedPosition[axis] + world->PositionOffset[axis];
	}
	m_PendingWorldObjectEditRequest = std::move(request);
}

// World Detail과 월드 배치 편집이 장면에서 Object를 확인할 때 호출한다. 선택 World와
// 명시적 Placement가 있는 동료 박스를 preview 요청에 복사하고 시간은 편집 확인용 긴 구간으로 맞춘다.
// draft의 실제 Timeline 시간은 바꾸지 않으며 재생은 MainApp의 요청 소비 경로가 맡는다.
void Client::CKoukuSaydonActionWorkbench::Queue_WorldBoxPreview(
	const std::string_view patternId, const std::string_view occurrenceId)
{
	const auto *pattern = Find_Pattern(m_Draft, patternId);
	const auto *box = pattern ? Find_WorldBox(*pattern, occurrenceId) : nullptr;
	const auto *world = box ? Find_World(m_Draft, box->strWorldId) : nullptr;
	if (!world)
		return;
	KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE resource;
	resource.eKind = KOUKU_SAYDON_PRESENTATION_KIND::WORLD;
	resource.strResourceId = world->strWorldId;
	resource.strAssetId = world->strSequenceInstanceId;
	resource.strDisplayName = world->strDisplayName;
	resource.iDurationMs = MAX_EDITOR_TIME_MS;
	Queue_PresentationPreview(resource);
	m_PendingPresentationPreviewRequest.strEditedOccurrenceId = std::string(occurrenceId);
	m_PendingPresentationPreviewRequest.strSourcePatternId = std::string(patternId);
	for (auto placed : pattern->WorldOccurrences)
	{
		if (!placed.Placement && placed.strOccurrenceId != occurrenceId)
			continue;
		placed.iStartMs = 0u;
		placed.iDurationMs = MAX_EDITOR_TIME_MS;
		m_PendingPresentationPreviewRequest.WorldBoxes.push_back(std::move(placed));
	}
}

// Sequence source Append/preview가 공유하는 초기 배치 계산이다. 선택한 World 박스 또는
// Animation/Stage, Sequence 시작, 커서 중 UI 설정에 맞는 시각과 anchor를 출력 row에 기록한다.
// 호출자가 Pattern 수명 > 0을 확인한 뒤 호출하며 이 함수는 문서나 선택 상태를 교체하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Configure_SequenceEffectOccurrence(
	const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE &resource,
	KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &row) const
{
	std::uint32_t startMs = m_iSequenceEffectStart == 2 ? m_iCursorMs : 0u;
	const auto *selectedEffect =
		m_iSequenceEffectStart == 0 ? Find_PresentationBox(pattern, m_strSelectedPresentationOccurrenceId) : nullptr;
	const auto *world = m_iSequenceEffectStart == 0 ? Find_WorldBox(pattern, m_strSelectedWorldOccurrenceId) : nullptr;
	if (!world && selectedEffect && selectedEffect->strAnchorKind == "WORLD")
		world = Find_WorldBox(pattern, selectedEffect->strWorldOccurrenceId);
	if (world)
	{
		startMs = world->iStartMs;
		row.strAnchorKind = "WORLD";
		row.strWorldId = world->strWorldId;
		row.strWorldOccurrenceId = world->strOccurrenceId;
		row.bFollowBoss = true;
	}
	else if (m_iSequenceEffectStart == 0)
	{
		if (selectedEffect)
			startMs = selectedEffect->iStartMs;
		std::uint32_t stageStartMs = 0u;
		for (const auto &stage : pattern.Stages)
		{
			const auto animation = std::find_if(
				stage.AnimationOccurrences.begin(), stage.AnimationOccurrences.end(), [&](const auto &value) {
					return value.strOccurrenceId == m_strSelectedOccurrenceId;
				});
			if (animation != stage.AnimationOccurrences.end())
			{
				startMs = stageStartMs + animation->iStartOffsetMs;
				break;
			}
			if (stage.strStageId == m_strSelectedStageId)
			{
				startMs = stageStartMs;
				break;
			}
			stageStartMs += stage.iDurationMs;
		}
	}
	const auto lifetimeMs = Pattern_DurationMs(pattern);
	row.iStartMs = (std::min)(startMs, lifetimeMs - 1u);
	row.iDurationMs = (std::min)(Camera_DefaultDuration(resource), lifetimeMs - row.iStartMs);
	if (world)
		row.iDurationMs = (std::min)(row.iDurationMs, world->iDurationMs);
}

// Sequence Resources의 Play Effect가 사용할 임시 전체 문서를 만든다. source resource와
// 필요한 occurrence를 사본에 준비하여 검증한 뒤 Queue_PatternDocumentPreview로 넘긴다.
// 실패는 상태만 알리며 성공해도 원본 draft/저장 기준 문서에는 preview용 박스를 추가하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Queue_SequenceEffectPreview(
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE &source)
{
	auto candidate = m_Draft;
	auto *pattern = Find_Pattern(candidate, m_strSelectedPatternId);
	if (!pattern || !pattern->strLoadError.empty() || !Pattern_DurationMs(*pattern) ||
		pattern->iNextPresentationOccurrenceOrdinal >= 1000000u)
	{
		m_strStatus = "Select an editable Sequence with a model or Object lifetime to play the Effect together.";
		return;
	}
	std::string resourceId, status;
	if (!Stage_PresentationSource(candidate, source, resourceId, status))
		return;
	const auto *resource = Find_PresentationResource(candidate, resourceId);
	const auto selected = std::find_if(
		pattern->PresentationOccurrences.begin(), pattern->PresentationOccurrences.end(), [&](const auto &value) {
			return value.strOccurrenceId == m_strSelectedPresentationOccurrenceId && value.strResourceId == resourceId;
		});
	KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE row;
	if (selected != pattern->PresentationOccurrences.end())
		row = *selected;
	else
	{
		row.strOccurrenceId =
			pattern->strPatternId + ".presentation." + std::to_string(pattern->iNextPresentationOccurrenceOrdinal++);
		row.strResourceId = resourceId;
		Set_FixedWorldEffectPlacement(row, row.PositionOffset);
	}
	if (selected == pattern->PresentationOccurrences.end())
		Configure_SequenceEffectOccurrence(*pattern, *resource, row);
	if (selected == pattern->PresentationOccurrences.end() && row.strAnchorKind == "MAP" &&
		!Resolve_PresentationBossSpawnPosition(candidate.strAreaId, *pattern, row.PositionOffset, status))
	{
		m_strStatus = status;
		return;
	}
	if (selected != pattern->PresentationOccurrences.end())
		*selected = row;
	else
		pattern->PresentationOccurrences.push_back(row);
	if (!CKoukuSaydonCompositionDocument::Validate(candidate, m_ResourceReferences, status))
	{
		m_strStatus = status;
		return;
	}
	// Preview용 전체 snapshot만 구성하며 저작 draft와 저장 기준 문서는 변경하지 않는다.
	if (Queue_PatternDocumentPreview(std::move(candidate), m_strSelectedPatternId, row.iStartMs, status, false))
		m_strStatus = "Playing Effect with Sequence models from " + std::to_string(row.iStartMs) +
			" ms. Append stores this Effect.";
}

// Resources/Box Detail/Camera/World 요청의 preview 분기점이다. Sequence Effect와 bone Collider,
// WORLD Light처럼 전체 문맥이 필요한 종류는 Pattern preview로 보내고 나머지는 resource/occurrence를
// 일회 요청에 복사한다. 저작 입력을 검증한 뒤 MainApp이 재생하며 이 함수는 문서를 commit하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Queue_PresentationPreview(
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE &resource,
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE *occurrence)
{
	if (m_bSequenceWorkspace && nullptr == occurrence && resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)
	{
		Queue_SequenceEffectPreview(resource);
		return;
	}
	if (nullptr != occurrence && resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER &&
		(!occurrence->strBone.empty() || occurrence->strBoneTarget == "WEAPON"))
	{
		std::string status;
		(void)Request_ColliderBoxPreview(m_strSelectedPatternId, *occurrence, status);
		return;
	}
	if (nullptr != occurrence && resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::LIGHT &&
		occurrence->strAnchorKind == "WORLD")
	{
		std::string status;
		// 실제 World timeline과 Detail 편집을 임시 snapshot에 합쳐 검증한다.
		(void)Request_PatternPreview(m_strSelectedPatternId, occurrence->iStartMs, status, false, occurrence);
		return;
	}
	if (nullptr != occurrence && resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)
	{
		std::string status;
		// 이 박스 시작에서 audition하여 Effect의 local age를 0으로 맞춘다.
		// actor와 WORLD 의존성은 저작된 Pattern 시계를 그대로 사용한다.
		(void)Request_PatternPreview(m_strSelectedPatternId, occurrence->iStartMs, status, false, occurrence);
		return;
	}
	auto currentResource = resource;
	if (nullptr == occurrence && !Refresh_V1ResourceDuration(currentResource, m_strStatus))
		return;
	m_bBundlePreviewRequestPending = false;
	if (nullptr != occurrence)
	{
		const auto finite = [](const auto &values, const double minimum, const double maximum) {
			return std::all_of(values.begin(), values.end(), [=](const double v) {
				return std::isfinite(v) && v >= minimum && v <= maximum;
			});
		};
		if (occurrence->iDurationMs == 0u || occurrence->iDurationMs > MAX_EDITOR_TIME_MS ||
			static_cast<std::uint64_t>(occurrence->iFadeInMs) + occurrence->iFadeOutMs > occurrence->iDurationMs ||
			!std::isfinite(occurrence->fDissolveStart) || !std::isfinite(occurrence->fDissolveEnd) ||
			occurrence->fDissolveStart < 0.0 || occurrence->fDissolveStart > occurrence->fDissolveEnd ||
			occurrence->fDissolveEnd > 1.0 || !finite(occurrence->PositionOffset, -100000.0, 100000.0) ||
			!finite(occurrence->RotationDegrees, -36000.0, 36000.0) || !finite(occurrence->Scale, 0.001, 10000.0) ||
			!std::isfinite(occurrence->fVolume) || occurrence->fVolume < 0.0 || occurrence->fVolume > 1.0)
		{
			m_strStatus = "Preview rejected: check lifetime, fade sum, dissolve range and finite transforms.";
			return;
		}
	}
	m_PendingPresentationPreviewRequest = {};
	m_PendingPresentationPreviewRequest.Resource = currentResource;
	m_PendingPresentationPreviewRequest.Occurrence =
		nullptr != occurrence ? *occurrence : KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE{};
	if (nullptr == occurrence)
	{
		m_PendingPresentationPreviewRequest.Occurrence.strResourceId = resource.strResourceId;
		m_PendingPresentationPreviewRequest.Occurrence.iDurationMs = Camera_DefaultDuration(currentResource);
		if (resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::LIGHT ||
			resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::SUBTITLE)
		{
			m_PendingPresentationPreviewRequest.Occurrence.strAnchorKind = resource.strDefaultAnchorKind;
			m_PendingPresentationPreviewRequest.Occurrence.bFollowBoss = resource.strDefaultAnchorKind != "MAP";
		}
		else if (resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)
		{
			m_PendingPresentationPreviewRequest.Occurrence.strAnchorKind = "BOSS";
			m_PendingPresentationPreviewRequest.Occurrence.bFollowBoss = false;
		}
	}
	m_bPresentationPreviewRequestPending = true;
	m_strStatus = std::string(Presentation_Label(resource.eKind)) + " preview requested: " + resource.strDisplayName;
}

// Sound Detail과 Timeline trim이 원본 길이(ms)를 필요로 할 때 사용한다. asset ID별 첫 조회만
// 런타임 경로를 해석해 오디오 길이를 읽고 0 실패도 캐시한다. 캐시 수명은 Set_PresentationResources까지다.
std::uint32_t Client::CKoukuSaydonActionWorkbench::Resolve_SoundDurationMs(
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE &resource)
{
	// 파일 목록의 duration은 임시값이다. refresh마다 선택 WAV만 한 번 읽고
	// 실패도 캐시하여 매 프레임 다시 읽지 않는다.
	const auto [entry, inserted] = m_SoundNativeDurations.try_emplace(resource.strAssetId, 0u);
	if (inserted)
	{
		const auto path = CRuntimeAssetRoot::Resolve(resource.strAssetId);
		if (!path.empty())
			(void)CGameInstance::Get().Get_SoundDurationMs(path.wstring(), entry->second);
	}
	return entry->second;
}

// Resources pane의 Effect/Sound 등 선택과 append를 처리한다. 목록 갱신 조건을 캐시하여 표시 프레임마다 원본 파일을 다시 읽지 않는다.
// Render_Resources가 현재 lane 종류를 전달한다. source 목록/생성된 resource 선택과
// Create/Append/Preview 명령을 연결하며 목록은 세션 캐시를 사용한다. Rename 등 draft 교체를 요청한
// 분기는 ImGui ID를 닫고 반환하여 현재 목록에서 차용한 resource를 다음 입력에 재사용하지 않는다.
void Client::CKoukuSaydonActionWorkbench::Render_PresentationResources(const KOUKU_SAYDON_PRESENTATION_KIND kind)
{
	Engine::CProfilerScope paneScope(CGameInstance::Get().Get_Profiler(), "ImGui.Composition.PresentationResources");
	ImGui::PushID(static_cast<int>(kind));
	ImGui::SeparatorText(Presentation_Label(kind));
	if (ImGui::Button("Refresh Resources"))
		m_bPresentationResourceRefreshRequested = true;
	ImGui::TextWrapped("%s", m_strPresentationResourceStatus.c_str());
	std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE> transientSources;
	const bool lightFamily = kind == KOUKU_SAYDON_PRESENTATION_KIND::LIGHT;
	const bool effectFamily = kind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT;
	const bool soundFamily = kind == KOUKU_SAYDON_PRESENTATION_KIND::SOUND;
	if (soundFamily)
	{
		ImGui::InputTextWithHint("##SoundResourceSearch", "Search sound name / WAV path / resource ID",
			m_SoundResourceSearch, std::size(m_SoundResourceSearch));
		ImGui::SameLine();
		if (ImGui::Button("Clear##SoundSearch"))
			m_SoundResourceSearch[0] = '\0';
	}
	const auto soundMatches = [&](const auto &item) {
		return !soundFamily || ContainsInsensitive(item.strDisplayName, m_SoundResourceSearch) ||
			ContainsInsensitive(item.strAssetId, m_SoundResourceSearch) ||
			ContainsInsensitive(item.strResourceId, m_SoundResourceSearch);
	};
	const auto categoryFor = [&](const auto &item) -> const std::vector<std::string> & {
		static const std::vector<std::string> empty;
		const auto found = m_EffectSourceOrganization.find(item.strAssetId);
		return found == m_EffectSourceOrganization.end() ? empty : found->second.CategoryPath;
	};
	const auto ownerFor = [&](const auto &item) -> const std::string & {
		const auto [entry, inserted] = m_EffectResourceOwners.try_emplace(item.strAssetId);
		if (inserted)
			entry->second = Effect_ResourceOwner(item.strAssetId, categoryFor(item));
		return entry->second;
	};
	std::map<std::string, EFFECT_TOOL_KOUKU_EFFECT_VIEW> missingKoukuViews;
	const auto koukuViewFor = [&](const auto &item) -> const EFFECT_TOOL_KOUKU_EFFECT_VIEW & {
		const auto cached = m_KoukuEffectResourceTree.assetIndices.find(item.strAssetId);
		if (cached != m_KoukuEffectResourceTree.assetIndices.end())
			return m_KoukuEffectResourceTree.views[cached->second];
		auto [entry, inserted] = missingKoukuViews.try_emplace(item.strAssetId);
		if (!inserted)
			return entry->second;
		const auto organized = m_EffectSourceOrganization.find(item.strAssetId);
		const auto saved = m_EffectSourceInventory.find(item.strAssetId);
		const std::string name =
			organized != m_EffectSourceOrganization.end() && !organized->second.strDisplayName.empty()
			? organized->second.strDisplayName
			: saved != m_EffectSourceInventory.end() && !saved->second.strDisplayName.empty()
			? saved->second.strDisplayName
			: item.strAssetId;
		return entry->second = Describe_KoukuSavedEffect(item.strAssetId, name, categoryFor(item));
	};
	const auto ownerMatches = [&](const auto &item) {
		return !effectFamily || m_iEffectResourceVersion != 1 || m_strEffectResourceOwner == "All" ||
			ownerFor(item) == m_strEffectResourceOwner;
	};
	const char *lightAnchors[] = {"MAP", "PLAYER", "BOSS"};
	if (lightFamily)
		ImGui::Combo("Category##LightResources", &m_iLightResourceCategory, "Map\0Character\0Boss\0");
	if (effectFamily)
	{
		if (ImGui::RadioButton("V2", m_iEffectResourceVersion == 0))
			m_iEffectResourceVersion = 0;
		ImGui::SameLine();
		if (ImGui::RadioButton("V1", m_iEffectResourceVersion == 1))
			m_iEffectResourceVersion = 1;
		if (m_iEffectResourceVersion == 1)
		{
			if (ImGui::BeginCombo("Character / Boss / World", m_strEffectResourceOwner.c_str()))
			{
				if (ImGui::Selectable("All", m_strEffectResourceOwner == "All"))
					m_strEffectResourceOwner = "All";
				for (const auto &owner : EFFECT_TOOL_ALL_EFFECTS_OWNER_OPTIONS)
					if (ImGui::Selectable(owner.strLabel.data(), m_strEffectResourceOwner == owner.strLabel))
						m_strEffectResourceOwner = owner.strLabel;
				if (ImGui::Selectable("Other", m_strEffectResourceOwner == "Other"))
					m_strEffectResourceOwner = "Other";
				ImGui::EndCombo();
			}
			if (!m_strEffectSourceMetadataStatus.empty())
				ImGui::TextWrapped("%s", m_strEffectSourceMetadataStatus.c_str());
		}
		ImGui::InputTextWithHint("##EffectSourceSearch", "Search saved Effects", m_PresentationResourceSearch,
			std::size(m_PresentationResourceSearch));
		if (m_bSequenceWorkspace)
		{
			ImGui::Combo("Effect start", &m_iSequenceEffectStart,
				"Selected model / Object start\0Sequence start (0 ms)\0Cursor\0");
			const auto *pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
			if (pattern && pattern->strLoadError.empty() && Pattern_DurationMs(*pattern))
			{
				KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE timing;
				KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE row;
				Configure_SequenceEffectOccurrence(*pattern, timing, row);
				ImGui::TextDisabled("Play / Append: %u ms | %s", row.iStartMs,
					row.strWorldId.empty() ? "Sequence animation" : "selected Object anchor");
			}
			ImGui::TextWrapped(
				"Play Effect auditions it with this Sequence's models. Append saves it at the same start. With no model box selected, Selected model starts at 0 ms.");
		}
	}
	const auto versionMatches = [&](const auto &item) {
		return !effectFamily ||
			(m_iEffectResourceVersion == 0
					? (item.strResourceKind == "GROUP" || item.strResourceKind == "LEAF")
					: (item.strResourceKind == "V1_EFFECT" || item.strResourceKind == "V1_ELEMENT"));
	};
	const auto sourceId = [](const auto &item) {
		return std::to_string(static_cast<int>(item.eKind)) + ":" + item.strResourceKind + ":" +
			(item.strAssetId.empty() ? item.strResourceId : item.strAssetId) +
			(item.strElementId.empty() ? "" : ":" + item.strElementId);
	};
	// 저장된 Effect/Sound 목록은 resource refresh와 filter 변경 때만 재구성한다.
	// Animation 재생이나 박스 편집은 이 목록을 변경하지 않는다.
	auto &sources = effectFamily ? m_EffectSourceView.rows : soundFamily ? m_SoundSourceView.rows : transientSources;
	const bool rebuildSources = effectFamily
		? (!m_EffectSourceView.valid || m_EffectSourceView.version != m_iEffectResourceVersion ||
			  m_EffectSourceView.owner != m_strEffectResourceOwner ||
			  m_EffectSourceView.search != m_PresentationResourceSearch)
		: soundFamily ? (!m_SoundSourceView.valid || m_SoundSourceView.search != m_SoundResourceSearch)
					  : true;
	if (rebuildSources)
	{
		sources.clear();
		Engine::CProfilerScope filterScope(CGameInstance::Get().Get_Profiler(), "ImGui.Composition.Resources.Filter");
		for (const auto &item : m_PresentationResourceInventory)
		{
			// Effect 종류가 아닌 행은 이름/경로 검색 전에 제외한다.
			if (item.eKind != kind || !versionMatches(item) ||
				(lightFamily && item.strDefaultAnchorKind != lightAnchors[m_iLightResourceCategory]) ||
				!ownerMatches(item))
				continue;
			const auto &categories = categoryFor(item);
			const bool searchMatches = !effectFamily || !m_PresentationResourceSearch[0] ||
				(m_iEffectResourceVersion == 1 && ownerFor(item) == "KoukuSaydon"
						? Matches_KoukuSavedEffect(koukuViewFor(item), m_PresentationResourceSearch) ||
							ContainsInsensitive(item.strElementId, m_PresentationResourceSearch)
						: ContainsInsensitive(item.strDisplayName, m_PresentationResourceSearch) ||
							ContainsInsensitive(item.strAssetId, m_PresentationResourceSearch) ||
							ContainsInsensitive(item.strElementId, m_PresentationResourceSearch) ||
							std::any_of(categories.begin(), categories.end(), [&](const auto &name) {
								return ContainsInsensitive(name, m_PresentationResourceSearch);
							}));
			if (searchMatches && soundMatches(item))
				sources.push_back(item);
		}
		if (effectFamily && m_iEffectResourceVersion == 1 && m_strEffectResourceOwner != "KoukuSaydon")
			std::stable_sort(sources.begin(), sources.end(), [](const auto &left, const auto &right) {
				return left.strDisplayName == right.strDisplayName ? left.strAssetId < right.strAssetId
																   : left.strDisplayName < right.strDisplayName;
			});
		if (effectFamily)
		{
			m_EffectSourceView.sourceIndices.clear();
			for (std::size_t index = 0u; index < sources.size(); ++index)
				m_EffectSourceView.sourceIndices.emplace(sourceId(sources[index]), index);
			m_EffectSourceView.version = m_iEffectResourceVersion;
			m_EffectSourceView.owner = m_strEffectResourceOwner;
			m_EffectSourceView.search = m_PresentationResourceSearch;
			m_EffectSourceView.valid = true;
			m_EffectSourceView.treeValid = false;
		}
		if (soundFamily)
		{
			m_SoundSourceView.sourceIndices.clear();
			for (std::size_t index = 0u; index < sources.size(); ++index)
				m_SoundSourceView.sourceIndices.emplace(sourceId(sources[index]), index);
			m_SoundSourceView.search = m_SoundResourceSearch;
			m_SoundSourceView.valid = true;
		}
	}
	if (soundFamily)
		ImGui::TextDisabled("%zu matching sound files", sources.size());
	if (kind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA)
	{
		auto *level = CLevel_KoukuSaydonArena::Get_Active();
		ImGui::InputText("New Camera name", m_NewPresentationName, std::size(m_NewPresentationName));
		ImGui::BeginDisabled(!level || !m_NewPresentationName[0]);
		if (ImGui::Button("Create Camera"))
		{
			std::string id;
			if (level->Create_CameraShot(m_NewPresentationName, id, m_strStatus))
			{
				m_strSelectedPresentationSourceId = std::to_string(static_cast<int>(kind)) + "::" + id;
				m_bPresentationResourceRefreshRequested = true;
			}
		}
		ImGui::EndDisabled();
		if (level && level->Ensure_CameraShotAuthoring(m_strPresentationResourceStatus))
		{
			sources.clear();
			for (const auto &shot : level->Get_CameraShots())
			{
				KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE source;
				source.eKind = kind;
				source.strResourceKind.clear();
				source.strAssetId = shot.strShotId;
				source.strDisplayName = shot.strDisplayName;
				source.iDurationMs =
					shot.hasCameraTrack ? shot.CameraTrack.iDurationMs : shot.iBlendInMs + shot.iDefaultHoldMs;
				sources.push_back(std::move(source));
			}
		}
	}
	if (kind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER)
	{
		for (int i = 0; i < 7; ++i)
		{
			KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE preset;
			preset.eKind = kind;
			preset.strResourceId = "preset.collider." + std::to_string(i);
			preset.strDisplayName = i == 0 ? "Rectangle"
				: i == 1				   ? "Semicircle"
				: i == 2				   ? "Sector"
				: i == 3				   ? "Roulette Card Region"
				: i == 4				   ? "Circle"
				: i == 5				   ? "Reverse Sector"
										   : "Cylinder";
			preset.strShape = i == 0 ? "BOX"
				: i == 4			 ? "CIRCLE"
				: i == 5			 ? "REVERSE_SECTOR"
				: i == 6			 ? "CYLINDER"
									 : "SECTOR";
			preset.fHalfAngleDegrees = i == 1 ? 90.0 : i == 3 ? 22.5 : 45.0;
			preset.strColliderKind = i == 3 ? "ROULETTE_CARD_REGION" : "GEOMETRY";
			preset.strResourceKind.clear();
			sources.push_back(preset);
		}
		ImGui::TextDisabled("Select a preset, then tune Lifetime, Motion / shape, Trigger and Logic in Box Detail.");
	}
	const auto selectSource = [&](const auto &item) {
		m_strSelectedPresentationSourceId = sourceId(item);
		(void)Copy_Text(m_NewPresentationName, std::size(m_NewPresentationName), item.strDisplayName);
	};
	const auto offerEffect = [&](const auto &item) {
		if (item.eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
			(item.strResourceKind != "V1_EFFECT" && item.strResourceKind != "LEAF" && item.strResourceKind != "GROUP"))
			return;
		Offer_CompositionResourceDrag(item.strDisplayName.c_str(), [item]() -> COMPOSITION_TRANSFER {
			auto transfer = std::make_shared<COMPOSITION_EFFECT_TRANSFER>();
			transfer->label = item.strDisplayName;
			COMPOSITION_EFFECT_ITEM value;
			value.resourceKind = item.strResourceKind;
			value.resourceId = item.strAssetId;
			value.displayName = item.strDisplayName;
			value.durationMs = item.iDurationMs;
			transfer->items.push_back(std::move(value));
			return transfer;
		});
	};
	const auto drawSource = [&](const auto &item) {
		const std::string id = sourceId(item);
		const std::string label = (kind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ? item.strResourceKind + "  " : "") +
			item.strDisplayName + "##" + id;
		if (ImGui::Selectable(label.c_str(), m_strSelectedPresentationSourceId == id))
			selectSource(item);
		offerEffect(item);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s%s%s | native %u ms", item.strAssetId.c_str(), item.strElementId.empty() ? "" : " | ",
				item.strElementId.c_str(), item.iDurationMs);
	};
	ImGui::BeginChild("##PresentationSources", ImVec2(0.f, effectFamily ? 360.f : 180.f), ImGuiChildFlags_Borders);
	const auto drawEffectSource = [&](auto &item) {
		if (!effectFamily || item.strResourceKind != "V1_EFFECT")
		{
			drawSource(item);
			return;
		}
		ImGui::SetNextItemOpen(m_strExpandedV1EffectId == item.strAssetId ||
				(m_bLocatePresentationSource && m_strSelectedPresentationSourceId == sourceId(item)),
			ImGuiCond_Always);
		const auto flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
			(m_strSelectedPresentationSourceId == sourceId(item) ? ImGuiTreeNodeFlags_Selected : 0);
		const std::string displayName = ownerFor(item) == "KoukuSaydon" ? koukuViewFor(item).text : item.strDisplayName;
		const bool open = ImGui::TreeNodeEx((displayName + "##" + sourceId(item)).c_str(), flags);
		offerEffect(item);
		if (ImGui::IsItemHovered())
		{
			const auto saved = m_EffectSourceInventory.find(item.strAssetId);
			ImGui::SetTooltip("%s\n%s", item.strAssetId.c_str(),
				saved == m_EffectSourceInventory.end() ? "Source file unavailable; the saved reference is retained."
													   : saved->second.strStatus.c_str());
		}
		const bool select = ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen();
		if (select)
			selectSource(item);
		if ((open || select) && m_strExpandedV1EffectId != item.strAssetId)
		{
			m_strExpandedV1EffectId = item.strAssetId;
			m_V1ElementResources.clear();
			m_strV1ElementResourceStatus.clear();
			if (const auto document = CEffectCatalog::Find(item.strAssetId))
			{
				if (Refresh_V1ResourceDuration(item, m_strV1ElementResourceStatus))
				{
					for (const auto &element : document->Elements)
					{
						auto child = item;
						child.strResourceKind = "V1_ELEMENT";
						child.strElementId = element.strElementId;
						child.strDisplayName =
							element.strDisplayName.empty() ? element.strElementId : element.strDisplayName;
						child.iDurationMs = V1_ElementDurationMs(element);
						m_V1ElementResources.push_back(std::move(child));
					}
					for (auto &row : m_PresentationResourceInventory)
						if (row.strAssetId == item.strAssetId && row.strResourceKind == "V1_EFFECT")
							row.iDurationMs = item.iDurationMs;
				}
			}
			else
				m_strV1ElementResourceStatus = "Effect unavailable: " + CEffectCatalog::Get_Status();
		}
		if (!open)
		{
			if (!select && ImGui::IsItemToggledOpen() && m_strExpandedV1EffectId == item.strAssetId)
				m_strExpandedV1EffectId.clear();
			return;
		}
		if (m_strExpandedV1EffectId == item.strAssetId)
		{
			if (!m_strV1ElementResourceStatus.empty())
				ImGui::TextWrapped("%s", m_strV1ElementResourceStatus.c_str());
			for (const auto &child : m_V1ElementResources)
				drawSource(child);
		}
		ImGui::TreePop();
	};
	const auto renderEffectTree = [&](EFFECT_RESOURCE_VIEW_CACHE &cache, const std::function<void(std::size_t)> &draw) {
		if (!cache.treeValid || m_bLocatePresentationSource)
		{
			Engine::CProfilerScope treeScope(
				CGameInstance::Get().Get_Profiler(), "ImGui.Composition.Resources.Tree.Rebuild");
			cache.categories = {};
			cache.koukuRows.clear();
			std::vector<EFFECT_TOOL_KOUKU_EFFECT_VIEW> koukuViews;
			for (std::size_t i = 0u; i < cache.rows.size(); ++i)
			{
				const auto &item = cache.rows[i];
				auto path = categoryFor(item);
				const auto &owner = ownerFor(item);
				const bool selected = m_strSelectedPresentationSourceId == sourceId(item);
				if (owner == "KoukuSaydon")
				{
					auto &node =
						m_strEffectResourceOwner == "All" ? cache.categories.children[owner] : cache.categories;
					node.kouku = true;
					node.selected |= selected;
					koukuViews.push_back(koukuViewFor(item));
					cache.koukuRows.push_back(i);
					continue;
				}
				if (!path.empty() && path.front() == owner)
					path.erase(path.begin());
				if (m_strEffectResourceOwner == "All")
					path.insert(path.begin(), owner);
				auto *node = &cache.categories;
				for (const auto &name : path)
				{
					node = &node->children[name];
					node->selected |= selected;
				}
				node->rows.push_back(i);
			}
			cache.koukuTree = Build_KoukuSavedEffectTree(std::move(koukuViews));
			if (m_strEffectResourceOwner == "KoukuSaydon")
				cache.categories.kouku = true;
			cache.treeValid = true;
		}
		Engine::CProfilerScope drawScope(CGameInstance::Get().Get_Profiler(), "ImGui.Composition.Resources.Tree.Draw");
		std::function<void(const EFFECT_RESOURCE_CATEGORY_NODE &)> render = [&](const auto &node) {
			if (node.kouku)
			{
				const std::string_view reveal =
					m_bLocatePresentationSource ? m_strExpandedV1EffectId : std::string_view{};
				Render_KoukuSavedEffectTree(
					cache.koukuTree,
					[&](const auto index) {
						draw(cache.koukuRows[index]);
					},
					m_PresentationResourceSearch[0] != '\0', reveal);
			}
			for (const auto &[name, child] : node.children)
			{
				if (m_PresentationResourceSearch[0] || (m_bLocatePresentationSource && child.selected))
					ImGui::SetNextItemOpen(true, ImGuiCond_Always);
				if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow))
				{
					render(child);
					ImGui::TreePop();
				}
			}
			for (auto index : node.rows)
				draw(index);
		};
		render(cache.categories);
	};
	if (effectFamily && m_iEffectResourceVersion == 1)
		renderEffectTree(m_EffectSourceView, [&](std::size_t index) {
			drawEffectSource(sources[index]);
		});
	else if (soundFamily)
	{
		ImGuiListClipper clipper;
		clipper.Begin(static_cast<int>(sources.size()));
		while (clipper.Step())
			for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
				drawSource(sources[index]);
	}
	else
		for (auto &item : sources)
			drawEffectSource(item);
	ImGui::EndChild();
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE *source = nullptr;
	if (effectFamily)
	{
		const auto selectedIndex = m_EffectSourceView.sourceIndices.find(m_strSelectedPresentationSourceId);
		if (selectedIndex != m_EffectSourceView.sourceIndices.end())
			source = &sources[selectedIndex->second];
		if (!source && m_iEffectResourceVersion == 1)
			for (const auto &child : m_V1ElementResources)
				if (sourceId(child) == m_strSelectedPresentationSourceId)
				{
					source = &child;
					break;
				}
	}
	else if (soundFamily)
	{
		const auto selectedIndex = m_SoundSourceView.sourceIndices.find(m_strSelectedPresentationSourceId);
		if (selectedIndex != m_SoundSourceView.sourceIndices.end())
		{
			auto &selectedSound = sources[selectedIndex->second];
			if (const auto duration = Resolve_SoundDurationMs(selectedSound))
				selectedSound.iDurationMs = duration;
			source = &selectedSound;
		}
	}
	else
		for (const auto &item : sources)
			if (sourceId(item) == m_strSelectedPresentationSourceId)
			{
				source = &item;
				break;
			}
	if (soundFamily && source)
		ImGui::TextWrapped("WAV / asset: %s", source->strAssetId.c_str());
	ImGui::InputText(effectFamily ? "Name in this composition" : "Display name", m_NewPresentationName,
		std::size(m_NewPresentationName));
	ImGui::BeginDisabled(source == nullptr);
	if (ImGui::Button(effectFamily ? "Play Effect###Preview Source" : "Preview Source") && source != nullptr)
		Queue_PresentationPreview(*source);
	ImGui::SameLine();
	ImGui::BeginDisabled(!m_bHasDraft || m_NewPresentationName[0] == '\0');
	if (ImGui::Button(kind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA ? "Register Camera Resource" : "Create") &&
		source != nullptr)
	{
		std::string status;
		(void)Create_PresentationResource(*source, m_NewPresentationName, status);
	}
	ImGui::EndDisabled();
	ImGui::EndDisabled();
	if (kind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA && source != nullptr)
		Render_CameraAuthoring(source->strAssetId);
	if (lightFamily || soundFamily || kind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA ||
		kind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)
	{
		const auto *pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
		const bool cameraBundle =
			kind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA && m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE;
		ImGui::BeginDisabled(!m_bHasDraft || source == nullptr ||
			(!cameraBundle &&
				(m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE || !pattern || !pattern->strLoadError.empty() ||
					Pattern_DurationMs(*pattern) == 0u)));
		const char *appendLabel = lightFamily ? "Append selected Light"
			: soundFamily					  ? "Append Sound at Cursor"
			: kind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT
			? (m_bSequenceWorkspace ? "Append Effect" : "Append Effect at Cursor")
			: "Append Camera at Cursor";
		if (ImGui::Button(appendLabel) && source != nullptr)
		{
			std::string status;
			(void)Append_PresentationSource(*source, status);
		}
		ImGui::EndDisabled();
		if (kind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)
			ImGui::TextWrapped(m_iEffectResourceVersion == 0
					? "V2 Leaf appends one saved Effect; Group appends its saved children together. Play the Sequence to replay all authored lanes."
					: "V1 Effect appends the complete saved effect; V1 Element appends the selected stable element. Source tuning is preserved.");
		if (lightFamily)
			ImGui::TextDisabled("Light shape and RGB are saved in Rendering Workbench. Boxes own timing and anchor.");
	}
	m_bLocatePresentationSource = false;
	ImGui::SeparatorText("Created Resources");
	ImGui::BeginChild(
		"##CreatedPresentationResources", ImVec2(0.f, effectFamily ? 330.f : 140.f), ImGuiChildFlags_Borders);
	std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE> transientCreated;
	auto &created = effectFamily ? m_CreatedEffectView.rows : soundFamily ? m_CreatedSoundView.rows : transientCreated;
	const bool rebuildCreated = effectFamily
		? (!m_CreatedEffectView.valid || m_CreatedEffectView.draftGeneration != m_iDraftGeneration ||
			  m_CreatedEffectView.version != m_iEffectResourceVersion ||
			  m_CreatedEffectView.owner != m_strEffectResourceOwner)
		: soundFamily ? (!m_CreatedSoundView.valid || m_CreatedSoundView.draftGeneration != m_iDraftGeneration ||
							m_CreatedSoundView.search != m_SoundResourceSearch)
					  : true;
	if (rebuildCreated)
	{
		created.clear();
		for (const auto &item : m_Draft.PresentationResources)
			if (item.eKind == kind && versionMatches(item) && ownerMatches(item) &&
				(!lightFamily || item.strDefaultAnchorKind == lightAnchors[m_iLightResourceCategory]) &&
				soundMatches(item))
				created.push_back(item);
		if (effectFamily)
		{
			m_CreatedEffectView.draftGeneration = m_iDraftGeneration;
			m_CreatedEffectView.version = m_iEffectResourceVersion;
			m_CreatedEffectView.owner = m_strEffectResourceOwner;
			m_CreatedEffectView.valid = true;
			m_CreatedEffectView.treeValid = false;
		}
		if (soundFamily)
		{
			m_CreatedSoundView.draftGeneration = m_iDraftGeneration;
			m_CreatedSoundView.search = m_SoundResourceSearch;
			m_CreatedSoundView.valid = true;
		}
	}
	const auto drawCreated = [&](std::size_t index) {
		const auto &item = created[index];
		const auto label = item.strDisplayName + "##" + item.strResourceId;
		if (ImGui::Selectable(label.c_str(), m_strSelectedPresentationResourceId == item.strResourceId))
			m_strSelectedPresentationResourceId = item.strResourceId;
	};
	if (effectFamily && m_iEffectResourceVersion == 1)
		renderEffectTree(m_CreatedEffectView, drawCreated);
	else if (soundFamily)
	{
		ImGuiListClipper clipper;
		clipper.Begin(static_cast<int>(created.size()));
		while (clipper.Step())
			for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
				drawCreated(index);
	}
	else
		for (std::size_t index = 0; index < created.size(); ++index)
			drawCreated(index);
	ImGui::EndChild();
	const auto *selected = Find_PresentationResource(m_Draft, m_strSelectedPresentationResourceId);
	if (nullptr != selected && selected->eKind == kind && versionMatches(*selected))
	{
		const auto resource = *selected;
		if (effectFamily && (resource.strResourceKind == "V1_EFFECT" || resource.strResourceKind == "V1_ELEMENT"))
		{
			const auto original = std::find_if(
				m_PresentationResourceInventory.begin(), m_PresentationResourceInventory.end(), [&](const auto &row) {
					return row.strAssetId == resource.strAssetId && row.strResourceKind == "V1_EFFECT";
				});
			const auto saved = m_EffectSourceInventory.find(resource.strAssetId);
			const auto path = categoryFor(resource);
			std::string category = ownerFor(resource);
			for (const auto &part : path)
				if (part != category)
					category += " / " + part;
			ImGui::TextWrapped("Asset: %s", resource.strAssetId.c_str());
			ImGui::TextWrapped("Category: %s", category.c_str());
			if (!resource.strElementId.empty())
				ImGui::TextWrapped("Element: %s", resource.strElementId.c_str());
			if (saved == m_EffectSourceInventory.end())
				ImGui::TextWrapped("Source file is unavailable; this composition keeps its saved reference.");
			else if (!saved->second.strStatus.empty())
				ImGui::TextWrapped("%s", saved->second.strStatus.c_str());
			if (original != m_PresentationResourceInventory.end())
			{
				std::string sourceName =
					resource.strElementId.empty() ? original->strDisplayName : resource.strElementId;
				bool sourceNameResolved = resource.strResourceKind == "V1_EFFECT";
				if (!resource.strElementId.empty())
				{
					const auto expanded = std::find_if(
						m_V1ElementResources.begin(), m_V1ElementResources.end(), [&](const auto &element) {
							return element.strAssetId == resource.strAssetId &&
								element.strElementId == resource.strElementId;
						});
					if (expanded != m_V1ElementResources.end())
					{
						sourceName =
							expanded->strDisplayName.empty() ? expanded->strElementId : expanded->strDisplayName;
						sourceNameResolved = true;
					}
					// label을 그릴 때 실패한 저작 파일 로드를 매 프레임 반복하지 않는다.
					else if (const auto document = CEffectCatalog::Find_Loaded(resource.strAssetId))
						for (const auto &element : document->Elements)
							if (element.strElementId == resource.strElementId)
							{
								sourceName =
									element.strDisplayName.empty() ? element.strElementId : element.strDisplayName;
								sourceNameResolved = true;
								break;
							}
				}
				ImGui::TextWrapped("Source: %s", sourceName.c_str());
				if (!sourceNameResolved)
					ImGui::TextDisabled("Saved element ID; Locate Source to load its current name.");
				if (sourceNameResolved && resource.strDisplayName != sourceName)
				{
					if (ImGui::Button("Use Source Name"))
					{
						std::string status;
						(void)Rename_Item(RENAME_TARGET::PRESENTATION, resource.strResourceId, sourceName, status);
						ImGui::PopID();
						return;
					}
					ImGui::SameLine();
				}
				if (ImGui::Button("Locate Source"))
				{
					m_iEffectResourceVersion = 1;
					m_strEffectResourceOwner = ownerFor(*original);
					m_PresentationResourceSearch[0] = '\0';
					selectSource(*original);
					m_bLocatePresentationSource = true;
				}
			}
		}
		if (soundFamily)
			ImGui::TextWrapped("WAV / asset: %s", resource.strAssetId.c_str());
		if (kind == KOUKU_SAYDON_PRESENTATION_KIND::SUBTITLE)
			ImGui::TextWrapped("%s", resource.strSubtitleText.c_str());
		if (Render_RenameControl(RENAME_TARGET::PRESENTATION, resource.strResourceId, resource.strDisplayName))
		{
			ImGui::PopID();
			return;
		}
		if (kind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA && source == nullptr)
			Render_CameraAuthoring(resource.strAssetId);
		if (ImGui::Button(effectFamily ? "Play Effect###Preview" : "Preview"))
			Queue_PresentationPreview(resource);
		ImGui::SameLine();
		const auto *pattern = Find_Pattern(m_Draft, m_strSelectedPatternId);
		const auto *bundle = m_ePatternSelection == KOUKU_PATTERN_SELECTION::BUNDLE
			? Find_Bundle(m_Draft, m_strSelectedBundleId)
			: nullptr;
		const bool appendable = (pattern && pattern->strLoadError.empty() && Pattern_DurationMs(*pattern) > 0u) ||
			(kind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA && bundle && bundle->strLoadError.empty());
		ImGui::BeginDisabled(!appendable);
		if (ImGui::Button(
				effectFamily && m_bSequenceWorkspace ? "Append Effect###Append at Cursor" : "Append at Cursor"))
		{
			std::string status;
			(void)Append_PresentationBox(resource.strResourceId, status);
		}
		ImGui::EndDisabled();
	}
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::PopID();
}

// Presentation/Collider anchor 입력이 공유하는 WORLD 대상 선택 UI다. 정의, occurrence,
// emission index를 전달받은 edit에 기록하며 문서 commit은 하지 않는다. World 정의를 바꾸면 이전
// occurrence/track 참조를 지워 다른 Object의 연결이 남지 않게 한다.
void Client::CKoukuSaydonActionWorkbench::Render_PresentationWorldAnchor(
	const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &edit)
{
	if (edit.strAnchorKind == "WORLD")
	{
		if (ImGui::BeginCombo(
				"World object", edit.strWorldId.empty() ? "Select world object" : edit.strWorldId.c_str()))
		{
			for (const auto &world : m_Draft.Worlds)
				if (ImGui::Selectable(
						(world.strDisplayName + "##" + world.strWorldId).c_str(), edit.strWorldId == world.strWorldId))
				{
					edit.strWorldId = world.strWorldId;
					edit.strWorldOccurrenceId.clear();
					edit.strWorldEffectTrackId.clear();
				}
			ImGui::EndCombo();
		}
		if (edit.strWorldId.empty())
			ImGui::TextDisabled("Select a world object from this Pattern before saving or playing.");
		if (!edit.strWorldId.empty() &&
			ImGui::BeginCombo("World box##Anchor",
				edit.strWorldOccurrenceId.empty() ? "(single matching box)" : edit.strWorldOccurrenceId.c_str()))
		{
			if (ImGui::Selectable("(single matching box)", edit.strWorldOccurrenceId.empty()))
			{
				edit.strWorldOccurrenceId.clear();
				edit.strWorldEffectTrackId.clear();
			}
			for (const auto &worldBox : pattern.WorldOccurrences)
				if (worldBox.strWorldId == edit.strWorldId &&
					ImGui::Selectable(
						worldBox.strOccurrenceId.c_str(), edit.strWorldOccurrenceId == worldBox.strOccurrenceId))
					edit.strWorldOccurrenceId = worldBox.strOccurrenceId;
			ImGui::EndCombo();
		}
		if (!edit.strWorldId.empty())
		{
			const auto *emissionWorld = Find_World(m_Draft, edit.strWorldId);
			const auto emissionSource = nullptr != emissionWorld
				? std::find_if(m_WorldSequenceResources.begin(), m_WorldSequenceResources.end(),
					  [&](const auto &row) {
						  return row.strInstanceId == emissionWorld->strSequenceInstanceId;
					  })
				: m_WorldSequenceResources.end();
			const int emissionCount =
				emissionSource != m_WorldSequenceResources.end() ? static_cast<int>(emissionSource->iEmissionCount) : 1;
			if (emissionCount > 1)
			{
				int emissionIndex = static_cast<int>(edit.iWorldEmissionIndex);
				if (ImGui::DragInt("Emission index##Anchor", &emissionIndex, 1.f, 0, emissionCount - 1, "%d",
						ImGuiSliderFlags_AlwaysClamp))
					edit.iWorldEmissionIndex =
						static_cast<std::uint32_t>((std::clamp)(emissionIndex, 0, emissionCount - 1));
				ImGui::TextDisabled(
					"This Object motion emits %d authored rows; the box follows the selected row.", emissionCount);
			}
			else
				edit.iWorldEmissionIndex = 0u;
		}
	}
}

// Presentation/Collider Detail의 Boss/World/고정 위치와 bone 연결을 편집한다.
// 전달받은 edit를 갱신하고 Pick/Focus는 MainApp용 요청으로 보낸다. 고정 위치 초기화가 실패하면
// 기존 입력을 유지하며 선택한 anchor의 저장/전체 검증은 Apply 또는 Save 경로에서 수행한다.
void Client::CKoukuSaydonActionWorkbench::Render_PresentationAnchor(const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern,
	KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &edit, const bool_t effect)
{
	const auto chooseFixedWorld = [&]() {
		std::array<double, 3u> position;
		bool resolved = false;
		const auto character = CAnimationTargetService::Resolve_SceneCharacter();
		const auto transform = character ? character->Get_Transform() : nullptr;
		if (transform)
		{
			const auto &world = *transform->Get_WorldMatrixPtr();
			position = {world._41, world._42, world._43};
			resolved = std::all_of(position.begin(), position.end(), [](double value) {
				return std::isfinite(value) && std::abs(value) <= 100000.0;
			});
		}
		if (!resolved && !Resolve_PresentationBossSpawnPosition(m_Draft.strAreaId, pattern, position, m_strStatus))
			return false;
		Set_FixedWorldEffectPlacement(edit, position);
		m_strStatus = resolved ? "World position initialized from the player."
							   : "World position initialized from this Pattern's boss spawn.";
		return true;
	};
	if (effect)
	{
		const bool boss = edit.strAnchorKind == "BOSS";
		if (ImGui::BeginCombo("Anchor", boss ? "Boss" : "World"))
		{
			if (ImGui::Selectable("Boss", boss) && !boss)
			{
				edit.strAnchorKind = "BOSS";
				edit.bFollowBoss = true;
				edit.PositionOffset = {0.0, 0.0, 0.0};
				edit.strWorldId.clear();
				edit.strWorldOccurrenceId.clear();
				edit.iWorldEmissionIndex = 0u;
				edit.strWorldEffectTrackId.clear();
			}
			if (ImGui::Selectable("World", !boss) && boss)
				(void)chooseFixedWorld();
			ImGui::EndCombo();
		}
		if (edit.strAnchorKind != "BOSS")
		{
			int mode = edit.strAnchorKind == "WORLD" ? 1 : 0;
			if (ImGui::Combo("Position mode", &mode, "Fixed position\0Follow world object\0"))
			{
				if (mode == 0)
					(void)chooseFixedWorld();
				else
				{
					edit.strAnchorKind = "WORLD";
					edit.bFollowBoss = true;
					edit.PositionOffset = {0.0, 0.0, 0.0};
					edit.strWorldId.clear();
					edit.strWorldOccurrenceId.clear();
					edit.iWorldEmissionIndex = 0u;
					edit.strWorldEffectTrackId.clear();
					edit.strBone.clear();
					edit.strBoneTarget = "BODY";
				}
			}
		}
	}
	else if (ImGui::BeginCombo("Anchor",
				 edit.strAnchorKind == "BOSS"	   ? "Boss"
					 : edit.strAnchorKind == "MAP" ? "Map (fixed position)"
												   : "World Object"))
	{
		for (const char *kind : {"BOSS", "WORLD", "MAP"})
			if (ImGui::Selectable(std::string_view(kind) == "BOSS" ? "Boss"
						: std::string_view(kind) == "MAP"		   ? "Map (fixed position)"
																   : "World Object",
					edit.strAnchorKind == kind))
			{
				if (std::string_view(kind) == "MAP")
				{
					if (edit.strAnchorKind != "MAP")
						(void)chooseFixedWorld();
				}
				else
				{
					edit.strAnchorKind = kind;
					edit.strWorldOccurrenceId.clear();
					edit.iWorldEmissionIndex = 0u;
					edit.strWorldEffectTrackId.clear();
					if (edit.strAnchorKind != "WORLD")
						edit.strWorldId.clear();
					if (edit.strAnchorKind != "BOSS")
					{
						edit.strBone.clear();
						edit.strBoneTarget = "BODY";
					}
				}
			}
		ImGui::EndCombo();
	}
	if (effect && (edit.strAnchorKind == "MAP" || edit.strAnchorKind == "WORLD"))
	{
		ImGui::TextWrapped(
			"Player/Mouse Position sets a fixed world pivot without an object anchor. Authored emitters keep their offsets from this pivot.");
		if (ImGui::Button("Use Boss Spawn Position##EffectMapAnchor"))
		{
			std::array<double, 3u> position;
			std::string status;
			if (Resolve_PresentationBossSpawnPosition(m_Draft.strAreaId, pattern, position, status))
			{
				Set_FixedWorldEffectPlacement(edit, position);
				m_strStatus =
					"Copied this Pattern's boss spawn position. The World Effect stays at this fixed position.";
			}
			else
				m_strStatus = status;
		}
		const auto character = CAnimationTargetService::Resolve_SceneCharacter();
		const auto transform = character ? character->Get_Transform() : nullptr;
		ImGui::TextWrapped(
			"Use Player Position copies the player. Use Mouse Position picks a visible world surface; Save keeps this placement.");
		if (transform)
		{
			float3_t position;
			XMStoreFloat3(&position, transform->Get_State(STATE::POSITION));
			if (edit.strAnchorKind == "MAP" && std::isfinite(position.x) && std::isfinite(position.y) &&
				std::isfinite(position.z))
			{
				const double dx = edit.PositionOffset[0] - position.x;
				const double dy = edit.PositionOffset[1] - position.y;
				const double dz = edit.PositionOffset[2] - position.z;
				ImGui::Text("Player XYZ: %.3f / %.3f / %.3f m", position.x, position.y, position.z);
				ImGui::Text("Effect pivot distance from player: %.3f m", std::sqrt(dx * dx + dy * dy + dz * dz));
			}
		}
		ImGui::BeginDisabled(!transform);
		if (ImGui::Button("Use Player Position##EffectMapAnchor") && transform)
		{
			float3_t position;
			XMStoreFloat3(&position, transform->Get_State(STATE::POSITION));
			if (std::isfinite(position.x) && std::isfinite(position.y) && std::isfinite(position.z))
				Set_FixedWorldEffectPlacement(edit, {position.x, position.y, position.z});
		}
		ImGui::EndDisabled();
		if (ImGui::Button("Use Mouse Position##EffectMapAnchor"))
			(void)Request_MapEffectPlacement(KOUKU_MAP_EFFECT_PLACEMENT_ACTION::PICK);
		ImGui::BeginDisabled(edit.strAnchorKind != "MAP");
		if (ImGui::Button("Focus Position (F)##EffectMapAnchor"))
			(void)Request_MapEffectPlacement(KOUKU_MAP_EFFECT_PLACEMENT_ACTION::FOCUS);
		const auto &io = ImGui::GetIO();
		if (edit.strAnchorKind == "MAP" && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
			!io.WantTextInput && !ImGui::IsAnyItemActive() && !io.KeyCtrl && !io.KeyAlt && !io.KeyShift &&
			!io.KeySuper && ImGui::IsKeyPressed(ImGuiKey_F, false))
			(void)Request_MapEffectPlacement(KOUKU_MAP_EFFECT_PLACEMENT_ACTION::FOCUS);
		ImGui::EndDisabled();
		if (edit.strAnchorKind == "MAP")
			ImGui::Text("World XYZ: X %.3f  Y %.3f  Z %.3f m", edit.PositionOffset[0], edit.PositionOffset[1],
				edit.PositionOffset[2]);
		if (m_MapEffectPlacementRequest.eAction == KOUKU_MAP_EFFECT_PLACEMENT_ACTION::PICK &&
			Is_MapEffectPlacementRequestCurrent(m_MapEffectPlacementRequest))
			ImGui::TextWrapped(
				"Pick a visible world surface. Esc or right-click cancels; empty space keeps the request active.");
	}
	Render_PresentationWorldAnchor(pattern, edit);
	if ((edit.strAnchorKind != "BOSS" && (effect || edit.strAnchorKind != "WORLD")) || edit.strBone.empty())
		edit.strBoneRotation = "TARGET_YAW";
	if (edit.strAnchorKind == "BOSS")
	{
		ImGui::SeparatorText("Bone Anchor");
		ANIMATION_MODEL_TARGET_VIEW bodyView, weaponView;
		const auto resolve = [&](ANIMATION_BONE_TARGET target, ANIMATION_MODEL_TARGET_VIEW &view) {
			if (m_PresentationModelResolver && m_PresentationModelResolver(pattern, target, view))
				return true;
			return CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(
					   CAnimationTargetService::Resolve_AssetName()) == pattern.strActorProfileId &&
				CAnimationTargetService::Resolve_ModelTarget(target, view);
		};
		bool hasBody = resolve(ANIMATION_BONE_TARGET::BODY, bodyView);
		bool hasWeapon = resolve(ANIMATION_BONE_TARGET::WEAPON, weaponView);
		ImGui::Text("Model: %s", pattern.strActorProfileId.c_str());
		ImGui::BeginDisabled(!m_PresentationModelLoader);
		if (ImGui::Button("Load anchor model##PresentationAnchor"))
		{
			// 모델 로드와 본 선택을 분리하여 아직 Play하지 않은 Pattern에서도 실제 골격을 열거한다.
			// 실패는 edit의 기존 bone을 그대로 두며 공용 Preview 소유자가 이전 모델을 보존한다.
			if (m_PresentationModelLoader(pattern, m_strStatus))
			{
				hasBody = resolve(ANIMATION_BONE_TARGET::BODY, bodyView);
				hasWeapon = resolve(ANIMATION_BONE_TARGET::WEAPON, weaponView);
			}
		}
		ImGui::EndDisabled();
		if (ImGui::BeginCombo("Bone target##PresentationAnchor", edit.strBoneTarget.c_str()))
		{
			if (ImGui::Selectable("BODY", edit.strBoneTarget == "BODY") && edit.strBoneTarget != "BODY")
			{
				edit.strBoneTarget = "BODY";
				edit.strBone.clear();
				edit.strBoneRotation = "TARGET_YAW";
				m_PresentationBoneSearch[0] = '\0';
			}
			ImGui::BeginDisabled(!hasWeapon);
			if (ImGui::Selectable("WEAPON", edit.strBoneTarget == "WEAPON") && edit.strBoneTarget != "WEAPON")
			{
				edit.strBoneTarget = "WEAPON";
				edit.strBone.clear();
				edit.bFollowBoss = true;
				edit.strBoneRotation = "TARGET_YAW";
				m_PresentationBoneSearch[0] = '\0';
			}
			ImGui::EndDisabled();
			ImGui::EndCombo();
		}
		const auto &targetView = edit.strBoneTarget == "WEAPON" ? weaponView : bodyView;
		const bool hasTarget = edit.strBoneTarget == "WEAPON" ? hasWeapon : hasBody;
		ImGui::BeginDisabled(!hasTarget);
		ImGui::InputTextWithHint("Find bone##PresentationAnchor", "hand, weapon, head...",
			m_PresentationBoneSearch.data(), m_PresentationBoneSearch.size());
		if (ImGui::BeginCombo("Anchor bone##PresentationAnchor", edit.strBone.empty() ? "(boss pivot)" : edit.strBone.c_str()))
		{
			const std::vector<std::string> bones = targetView.Model ? targetView.Model->Get_BoneNames() : std::vector<std::string>{};
			if (edit.strBoneTarget == "BODY" && ImGui::Selectable("(boss pivot)", edit.strBone.empty()))
			{
				edit.strBone.clear();
				edit.strBoneRotation = "TARGET_YAW";
			}
			for (const auto &bone : bones)
			{
				if (!ContainsInsensitive(bone, m_PresentationBoneSearch.data()))
				{
					continue;
				}
				if (ImGui::Selectable(bone.c_str(), edit.strBone == bone))
				{
					edit.strBone = bone;
					edit.bFollowBoss = true;
				}
			}
			ImGui::EndCombo();
		}
		ImGui::EndDisabled();
		if (!hasBody)
			ImGui::TextWrapped("Load anchor model to select a hand or socket before playing this Pattern.");
		else if (edit.strBoneTarget == "WEAPON" && !hasWeapon)
			ImGui::TextDisabled("This preview model has no registered weapon.");
		else if (!edit.strBone.empty() &&
			(!targetView.Model || targetView.Model->Find_BoneIndex(edit.strBone.c_str()) < 0))
			ImGui::TextDisabled("The saved bone is unavailable on this target.");
		else
			ImGui::TextDisabled("Actual %s model bones. The selected name is saved with this box.", edit.strBoneTarget.c_str());
		{
			ImGui::BeginDisabled(edit.strBone.empty());
			bool boneRotation = edit.strBoneRotation == "BONE";
			if (ImGui::Checkbox("Rotate with this bone##PresentationAnchor", &boneRotation))
				edit.strBoneRotation = boneRotation && !edit.strBone.empty() ? "BONE" : "TARGET_YAW";
			ImGui::EndDisabled();
		}
		ImGui::TextWrapped("%s",
			edit.strBoneRotation == "BONE"
				? "This bone owns position and orientation. The box offset and rotation are applied in its frame; Server Colliders use the sampled planar frame."
				: "BODY (pivot) follows the boss body. Named bones follow their position; rotation uses boss facing and this box's rotation.");
	}
	if (!effect && edit.strAnchorKind == "WORLD")
	{
		char bone[256]{};
		const auto size = (std::min)(edit.strBone.size(), sizeof(bone) - 1u);
		std::copy_n(edit.strBone.data(), size, bone);
		if (ImGui::InputText("World BODY bone", bone, sizeof(bone)))
		{
			edit.strBone = bone;
			edit.strBoneTarget = "BODY";
			edit.bFollowBoss = true;
		}
		bool rotation = edit.strBoneRotation == "BONE";
		ImGui::BeginDisabled(edit.strBone.empty());
		if (ImGui::Checkbox("Rotate with World bone", &rotation))
			edit.strBoneRotation = rotation ? "BONE" : "TARGET_YAW";
		ImGui::EndDisabled();
		ImGui::TextWrapped(
			"The active World Object model supplies this bone's position and planar heading. An unavailable bone keeps the contact unpublished.");
		if (edit.strBone.empty() || edit.strBoneRotation != "BONE" || !edit.bFollowBoss ||
			edit.strWorldOccurrenceId.empty())
			edit.strWorldEffectTrackId.clear();
		const auto *world = Find_World(m_Draft, edit.strWorldId);
		const auto *level = CLevel_KoukuSaydonArena::Get_Active();
		const auto *sequences = level ? &level->Get_WorldSequenceDocument() : nullptr;
		const WORLD_SEQUENCE_INSTANCE *instance = nullptr;
		const WORLD_SEQUENCE_TEMPLATE *motion = nullptr;
		if (sequences && world)
		{
			for (const auto &row : sequences->Get_Instances())
				if (row.instanceId == world->strSequenceInstanceId)
				{
					instance = &row;
					break;
				}
			if (instance)
				for (const auto &row : sequences->Get_Templates())
					if (row.sequenceId == instance->templateId)
					{
						motion = &row;
						break;
					}
		}
		ImGui::BeginDisabled(edit.strBone.empty() || edit.strBoneRotation != "BONE" || !edit.bFollowBoss ||
			edit.strWorldOccurrenceId.empty());
		if (ImGui::BeginCombo("World Effect frame",
				edit.strWorldEffectTrackId.empty() ? "Object frame" : edit.strWorldEffectTrackId.c_str()))
		{
			if (ImGui::Selectable("Object frame", edit.strWorldEffectTrackId.empty()))
				edit.strWorldEffectTrackId.clear();
			if (instance && motion && instance->anchorKind == "WORLD" && instance->bindings.size() == 1u &&
				instance->bindings.front().targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
				for (const auto &track : motion->effectTracks)
				{
					if (track.slotId != instance->bindings.front().slotId || track.resourceKind != "V1_EFFECT" ||
						!track.followObject || !track.inheritObjectRotation || !track.bone.empty() ||
						std::abs(track.rotationDegrees.x) > .00001f || std::abs(track.rotationDegrees.z) > .00001f ||
						track.scale.x <= 0.f || std::abs(track.scale.x - track.scale.y) > .00001f ||
						std::abs(track.scale.x - track.scale.z) > .00001f ||
						std::count_if(motion->effectTracks.begin(), motion->effectTracks.end(), [&](const auto &other) {
							return other.effectTrackId == track.effectTrackId;
						}) != 1)
						continue;
					if (ImGui::Selectable(
							track.effectTrackId.c_str(), edit.strWorldEffectTrackId == track.effectTrackId))
						edit.strWorldEffectTrackId = track.effectTrackId;
				}
			ImGui::EndCombo();
		}
		ImGui::EndDisabled();
		ImGui::TextWrapped(
			"Select an exact World box to choose its Effect frame. This contact follows that Effect's position, yaw and uniform size; Save keeps the track ID.");
	}
	if (effect && ImGui::BeginCombo("Copy Collider anchor", "(choose Collider box)"))
	{
		for (const auto &collider : pattern.PresentationOccurrences)
		{
			const auto *resource = Find_PresentationResource(m_Draft, collider.strResourceId);
			if (!resource || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER)
				continue;
			if (ImGui::Selectable((resource->strDisplayName + " | " + collider.strOccurrenceId).c_str()))
			{
				edit.strAnchorKind = collider.strAnchorKind;
				edit.strBoneTarget = collider.strBoneTarget;
				edit.strBone = collider.strBone;
				edit.strWorldId = collider.strWorldId;
				edit.strWorldOccurrenceId = collider.strWorldOccurrenceId;
				edit.bFollowBoss = collider.bFollowBoss;
				edit.PositionOffset = collider.PositionOffset;
				edit.RotationDegrees = collider.RotationDegrees;
				if (edit.strAnchorKind != "BOSS" || edit.strBone.empty())
					edit.strBoneRotation = "TARGET_YAW";
			}
		}
		ImGui::EndCombo();
	}
	if (effect)
		ImGui::TextDisabled("Copy takes anchor, position offset and rotation. Apply and Save keep it.");
}

// Render_Details가 선택한 presentation occurrence를 전달한다. Collider는 전용 화면에
// 위임하고 나머지는 resource를 값으로 복사한 뒤 m_PresentationBoxEdit를 편집한다. placement preview와
// 시간/연출 값 Apply를 분리하며 Apply 뒤에는 즉시 반환해 draft에서 차용한 pattern/box를 더 쓰지 않는다.
void Client::CKoukuSaydonActionWorkbench::Render_PresentationBoxDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern)
{
	const auto *box = Find_PresentationBox(pattern, m_strSelectedPresentationOccurrenceId);
	if (nullptr == box)
		return;
	const auto *resource = Find_PresentationResource(m_Draft, box->strResourceId);
	if (nullptr == resource)
		return;
	if (resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER)
	{
		Render_ColliderBoxDetails(pattern);
		return;
	}
	const auto definition = *resource;
	const auto patternId = pattern.strPatternId;
	const auto occurrenceId = box->strOccurrenceId;
	ImGui::SeparatorText(Presentation_Label(definition.eKind));
	const bool cameraBox = definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA;
	if (cameraBox)
		Render_CameraAuthoring(definition.strAssetId, patternId, occurrenceId);
	if (cameraBox)
		if (const auto *shot = Find_AuthoringCamera(definition.strAssetId))
			ImGui::Text("Entry %u ms | hold %u ms | return ends %u ms", shot->iBlendInMs,
				box->iDurationMs > shot->iBlendInMs ? box->iDurationMs - shot->iBlendInMs : 0u,
				box->iStartMs + box->iDurationMs + shot->iBlendOutMs);
	ImGui::TextWrapped("%s", definition.strDisplayName.c_str());
	ImGui::TextDisabled("%s", occurrenceId.c_str());
	auto &edit = m_PresentationBoxEdit;
	if (edit.strOccurrenceId != occurrenceId)
		edit = *box;
	const auto placementBeforeControls = edit;
	int start = static_cast<int>(edit.iStartMs), duration = static_cast<int>(edit.iDurationMs);
	if (ImGui::InputInt("Start ms##PresentationBox", &start))
		edit.iStartMs = static_cast<std::uint32_t>((std::max)(0, start));
	if (ImGui::InputInt(cameraBox ? "Entry + hold ms##PresentationBox" : "Lifetime ms##PresentationBox", &duration))
	{
		if (edit.EffectSourceTimeKeys.empty())
			edit.iDurationMs = static_cast<std::uint32_t>((std::max)(1, duration));
		else
			Trim_PresentationWindow(edit, std::int64_t(duration) - edit.iDurationMs, false, Pattern_DurationMs(pattern),
				definition.iDurationMs, true);
	}
	if (definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT && edit.EffectSourceTimeKeys.empty())
	{
		const auto sourceLimit = (std::max)(1u, (std::min)(definition.iDurationMs, MAX_EDITOR_TIME_MS));
		int sourceIn = static_cast<int>(edit.iEffectSourceStartMs);
		if (ImGui::InputInt("Source In ms##Effect", &sourceIn, 10, 100))
		{
			edit.iEffectSourceStartMs =
				static_cast<std::uint32_t>(std::clamp(sourceIn, 0, static_cast<int>(sourceLimit) - 1));
			if (!edit.bFitEffectToDuration && !edit.bLoopEffectToDuration)
				edit.iDurationMs = (std::min)(edit.iDurationMs, sourceLimit - edit.iEffectSourceStartMs);
		}
		ImGui::TextDisabled("Source %u ms. Source In skips the beginning of this whole Effect, keeping the box start.",
			definition.iDurationMs);
		ImGui::TextWrapped(
			"Drag the left edge to trim the Effect source; drag the middle to move without trimming. Apply and Save keep this box's source segment.");
	}
	if (!edit.EffectSourceTimeKeys.empty())
		ImGui::TextWrapped(
			"Original scene speed curve: %.3f..%.3f ms of Effect source. Move this box or trim its edges to preserve synchronization.",
			edit.EffectSourceTimeKeys.front()[1], edit.EffectSourceTimeKeys.back()[1]);
	if (definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT &&
		(definition.strResourceKind == "V1_EFFECT" || definition.strResourceKind == "V1_ELEMENT"))
	{
		ImGui::BeginDisabled(!edit.EffectSourceTimeKeys.empty());
		if (ImGui::Checkbox("Fit Effect lifetime to box", &edit.bFitEffectToDuration) && edit.bFitEffectToDuration)
			edit.bLoopEffectToDuration = false;
		if (edit.bFitEffectToDuration)
			ImGui::TextDisabled("Source %u..%u ms plays once over this box. Change Lifetime, then Apply and Save.",
				edit.iEffectSourceStartMs, definition.iDurationMs);
		if (ImGui::Checkbox("Loop Effect through lifetime", &edit.bLoopEffectToDuration) && edit.bLoopEffectToDuration)
			edit.bFitEffectToDuration = false;
		if (edit.bLoopEffectToDuration)
		{
			ImGui::TextDisabled("Continuous source emitters keep their original speed until this box ends.");
			std::uint32_t animationEnd = 0u, stageStart = 0u;
			for (const auto &stage : pattern.Stages)
			{
				for (const auto &animation : stage.AnimationOccurrences)
					animationEnd = (std::max)(animationEnd, stageStart + animation.iStartOffsetMs + animation.iPlayMs);
				stageStart += stage.iDurationMs;
			}
			ImGui::BeginDisabled(animationEnd <= edit.iStartMs);
			if (ImGui::Button("Match remaining animation time"))
				edit.iDurationMs = animationEnd - edit.iStartMs;
			ImGui::EndDisabled();
		}
		ImGui::EndDisabled();
	}
	const auto vectorControl = [](const char *label, std::array<double, 3u> &values, const float minimum,
								   const float maximum) {
		float v[3] = {static_cast<float>(values[0]), static_cast<float>(values[1]), static_cast<float>(values[2])};
		if (!ImGui::DragFloat3(label, v, 0.05f, minimum, maximum, "%.3f"))
			return false;
		for (std::size_t i = 0u; i < 3u; ++i)
			values[i] = v[i];
		return true;
	};
	const bool subtitleBox = definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::SUBTITLE;
	bool geometryChanged = false;
	if (subtitleBox)
	{
		float offset[2] = {float(edit.PositionOffset[0]), float(edit.PositionOffset[1])};
		if (ImGui::DragFloat2("Screen X / Y (px at 1080p)##SubtitleBox", offset, 1.f, -100000.f, 100000.f, "%.1f"))
		{
			edit.PositionOffset[0] = offset[0];
			edit.PositionOffset[1] = offset[1];
			geometryChanged = true;
		}
		float textScale = float(edit.Scale[0]);
		if (ImGui::DragFloat("Text scale##SubtitleBox", &textScale, .01f, .001f, 10000.f, "%.2f"))
		{
			edit.Scale = {textScale, textScale, textScale};
			geometryChanged = true;
		}
		ImGui::TextDisabled(
			"Positive Y moves down. Scale 2 doubles the text height. Preview and Save keep layout edits.");
	}
	else
		geometryChanged = vectorControl(edit.strAnchorKind == "MAP" ? "World X / Y / Z (m)##PresentationBox"
																	: "Position offset (m)##PresentationBox",
			edit.PositionOffset, -100000.f, 100000.f);
	const bool gameplayCollider = definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER &&
		(!edit.strLogicOccurrenceId.empty() || !m_strColliderLogicDefinitionId.empty() ||
			(m_strColliderExecutionEditId == occurrenceId && m_bColliderDamageMode));
	if (gameplayCollider)
	{
		float yaw = static_cast<float>(edit.RotationDegrees[1]);
		if (ImGui::DragFloat("Yaw (degrees)##PresentationBox", &yaw, 0.05f, -36000.f, 36000.f, "%.3f"))
		{
			edit.RotationDegrees[1] = yaw;
			geometryChanged = true;
		}
		if (edit.RotationDegrees[0] != 0.0 || edit.RotationDegrees[2] != 0.0)
		{
			ImGui::TextWrapped(
				"Gameplay Collider supports Y rotation only. X %.3f / Z %.3f makes this Pattern unavailable after publish.",
				edit.RotationDegrees[0], edit.RotationDegrees[2]);
			if (ImGui::Button("Clear X/Z rotation##GameplayCollider"))
			{
				edit.RotationDegrees[0] = edit.RotationDegrees[2] = 0.0;
				geometryChanged = true;
			}
		}
	}
	else if (!cameraBox && !subtitleBox)
		geometryChanged |=
			vectorControl("Rotation (degrees)##PresentationBox", edit.RotationDegrees, -36000.f, 36000.f);
	const bool light = definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::LIGHT;
	if (!light && !cameraBox && !subtitleBox)
		geometryChanged |= vectorControl("Scale / size##PresentationBox", edit.Scale, 0.001f, 10000.f);
	const auto previewGeometry = [&]() {
		std::string status;
		(void)Request_PresentationGeometryPreview(patternId, edit, status);
	};
	if (geometryChanged && (definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER || subtitleBox))
		previewGeometry();
	if (light)
	{
		const char *kinds[] = {"MAP", "PLAYER", "BOSS", "WORLD"};
		const char *labels[] = {"Map", "Character", "Boss", "World Object"};
		int selected = edit.strAnchorKind == "MAP" ? 0
			: edit.strAnchorKind == "PLAYER"	   ? 1
			: edit.strAnchorKind == "BOSS"		   ? 2
												   : 3;
		if (ImGui::Combo("Anchor##LightBox", &selected, labels, 4))
		{
			edit.strAnchorKind = kinds[selected];
			edit.strWorldId.clear();
			edit.strBone.clear();
			edit.strWorldOccurrenceId.clear();
			edit.iWorldEmissionIndex = 0u;
			edit.strBoneTarget = "BODY";
			edit.bFollowBoss = selected != 0;
		}
		if (edit.strAnchorKind == "MAP")
			ImGui::TextDisabled("Map uses a fixed world position from the resource plus this offset.");
		if (edit.strAnchorKind == "PLAYER")
			ImGui::TextDisabled("One following light per living replicated character.");
		if (edit.strAnchorKind == "BOSS")
		{
			ImGui::TextWrapped(
				"Boss target: %s | %s", pattern.strGateId.c_str(), pattern.strTargetBossPlacementId.c_str());
			ImGui::TextDisabled("Boss uses this Pattern actor. Actors in the World track are separate targets.");
		}
		Render_PresentationWorldAnchor(pattern, edit);
		if (edit.strAnchorKind == "WORLD")
			ImGui::TextWrapped(
				"World Object follows the selected actor in this Pattern's World track. Preview starts at this Light box time with the complete World timeline.");
		if (definition.strDefaultAnchorKind == "MAP" && edit.strAnchorKind != "MAP")
			ImGui::TextWrapped(
				"Map light reuse aims its center at the selected target and retains the saved light height, direction and shape. Position Offset adjusts that placement.");
		float brightness = static_cast<float>(edit.fBrightnessMultiplier);
		if (ImGui::DragFloat("Brightness multiplier", &brightness, 0.05f, 0.f, 16.f))
			edit.fBrightnessMultiplier = brightness;
		int fadeIn = static_cast<int>(edit.iFadeInMs), fadeOut = static_cast<int>(edit.iFadeOutMs);
		if (ImGui::InputInt("Fade in ms##Light", &fadeIn))
			edit.iFadeInMs = static_cast<std::uint32_t>((std::max)(0, fadeIn));
		if (ImGui::InputInt("Fade out ms##Light", &fadeOut))
			edit.iFadeOutMs = static_cast<std::uint32_t>((std::max)(0, fadeOut));
		ImGui::Checkbox("Debug wire##Light", &edit.bDebugRender);
		if (edit.strAnchorKind == "BOSS")
		{
			char bone[128]{};
			(void)Copy_Text(bone, std::size(bone), edit.strBone);
			if (ImGui::InputText("Bone (empty = pivot)##Light", bone, std::size(bone)))
				edit.strBone = bone;
		}
	}
	if (definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
		definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER)
		Render_PresentationAnchor(pattern, edit, definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT);
	if (!cameraBox && edit.strAnchorKind != "MAP" &&
		(!light || edit.strAnchorKind == "BOSS" || edit.strAnchorKind == "WORLD"))
		ImGui::Checkbox("Follow anchor##PresentationBox", &edit.bFollowBoss);
	if (definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)
	{
		if (!Same_PresentationPlacement(placementBeforeControls, edit, true))
			previewGeometry();
		ImGui::BeginDisabled(!edit.EffectSourceTimeKeys.empty());
		int fadeIn = static_cast<int>(edit.iFadeInMs), fadeOut = static_cast<int>(edit.iFadeOutMs);
		if (ImGui::InputInt("Fade in ms", &fadeIn))
			edit.iFadeInMs = static_cast<std::uint32_t>((std::max)(0, fadeIn));
		if (ImGui::InputInt("Fade out ms", &fadeOut))
			edit.iFadeOutMs = static_cast<std::uint32_t>((std::max)(0, fadeOut));
		float startRatio = static_cast<float>(edit.fDissolveStart), endRatio = static_cast<float>(edit.fDissolveEnd);
		ImGui::BeginDisabled(edit.iFadeOutMs == 0u);
		if (ImGui::SliderFloat("Dissolve out start", &startRatio, 0.f, 1.f))
			edit.fDissolveStart = startRatio;
		if (ImGui::SliderFloat("Dissolve out end", &endRatio, 0.f, 1.f))
			edit.fDissolveEnd = endRatio;
		ImGui::EndDisabled();
		ImGui::TextDisabled(
			"Fade 0 keeps authored alpha/dissolve. Positive fades override; dissolve-out follows Fade Out.");
		ImGui::EndDisabled();
	}
	if (definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::SOUND)
	{
		const auto nativeMs = Resolve_SoundDurationMs(definition);
		const auto sourceLimit = nativeMs ? (std::min)(nativeMs, MAX_EDITOR_TIME_MS) : MAX_EDITOR_TIME_MS;
		const auto patternMs = Pattern_DurationMs(pattern);
		const auto timelineRemaining = patternMs > edit.iStartMs ? patternMs - edit.iStartMs : 1u;
		int sourceIn = static_cast<int>(edit.iSoundSourceStartMs);
		if (ImGui::InputInt("Source In ms##Sound", &sourceIn, 10, 100))
		{
			edit.iSoundSourceStartMs =
				static_cast<std::uint32_t>(std::clamp(sourceIn, 0, static_cast<int>(sourceLimit) - 1));
			edit.iDurationMs =
				(std::min)({edit.iDurationMs, sourceLimit - edit.iSoundSourceStartMs, timelineRemaining});
		}
		int sourceOut = static_cast<int>(edit.iSoundSourceStartMs + edit.iDurationMs);
		if (ImGui::InputInt("Source Out ms##Sound", &sourceOut, 10, 100))
		{
			const auto maximum = (std::max)(edit.iSoundSourceStartMs + 1u,
				(std::min)(sourceLimit, edit.iSoundSourceStartMs + timelineRemaining));
			sourceOut =
				std::clamp(sourceOut, static_cast<int>(edit.iSoundSourceStartMs + 1u), static_cast<int>(maximum));
			edit.iDurationMs = static_cast<std::uint32_t>(sourceOut) - edit.iSoundSourceStartMs;
		}
		ImGui::TextDisabled(
			"Drag the left/right edge to trim; drag the middle to move. Apply and Save keep the WAV segment.");
		ImGui::TextWrapped("Event: %s", definition.strSoundEvent.c_str());
		ImGui::TextWrapped("WAV / asset: %s", definition.strAssetId.c_str());
		float volume = static_cast<float>(edit.fVolume);
		if (ImGui::SliderFloat("Volume", &volume, 0.f, 1.f))
			edit.fVolume = volume;
	}

	if (ImGui::Button("Apply##PresentationBox"))
	{
		std::string status;
		const auto value = edit;
		(void)Set_PresentationBox(patternId, value, status);
		return;
	}
	ImGui::SameLine();
	if (ImGui::Button("Preview##PresentationBox"))
		Queue_PresentationPreview(definition, &edit);
	if (definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER ||
		definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT || subtitleBox)
	{
		ImGui::SameLine();
		if (ImGui::Button(subtitleBox											 ? "Revert layout##PresentationBox"
					: definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ? "Revert placement##PresentationBox"
																				 : "Revert geometry##PresentationBox"))
		{
			std::string status;
			(void)Request_PresentationGeometryPreview(patternId, *box, status);
			Copy_PresentationPlacement(*box, edit, definition.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT);
		}
		ImGui::TextDisabled(
			"Preview and Save keep Effect placement, Collider geometry and Subtitle layout. Apply keeps timing, fades and other fields.");
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete##PresentationBox"))
	{
		std::string status;
		(void)Delete_PresentationBox(patternId, occurrenceId, status);
	}
	ImGui::TextWrapped("%s", m_strStatus.c_str());
}

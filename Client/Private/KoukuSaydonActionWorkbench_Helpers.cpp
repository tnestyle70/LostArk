// 여러 편집 도메인이 함께 쓰는 stable ID 조회, 시간/배치 검증이다. 반환 포인터는 호출자가 전달한 문서가 교체되기 전까지만 유효하다.
#include "imgui.h"
#include "KoukuSaydonActionWorkbench.h"
#include "KoukuSaydonActionWorkbench_Internal.h"
#include "KoukuSaydonActionWorkbench_CameraLookup.h"
#include "CompositionTimeline.h"

// 이 도메인에서 직접 사용하는 런타임/저작 자료의 선언이다.
#include "Level_KoukuSaydonArena.h"
#include "ProjectDataRoot.h"
#include "WorldGameplayDocument.h"

// 후보 값 복사, stable ID 조회, 시간/좌표 계산에 필요한 표준 타입이다.
#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <iterator>
#include <limits>
#include <unordered_set>
#include <utility>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

using namespace Client::KoukuWorkbenchDetail;

namespace
{
using namespace Client;

constexpr f32_t TIMELINE_LANE_HEIGHT = CompositionTimeline::LaneHeight;
constexpr f32_t TIMELINE_LABEL_WIDTH = 92.f;
constexpr const char_t *DEFAULT_BOSS_VARIANT_LABEL = "\xEC\x84\xB8\xEC\x9D\xB4\xED\x8A\xBC - 1\xEA\xB4\x80\xEB\xAC\xB8";

bool Has_CompletionChain(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern)
{
	if (!pattern.strLoopStartPatternOccurrenceId.empty() || pattern.bPlayChildrenSequentially)
		return true;
	return std::any_of(pattern.LogicOccurrences.begin(), pattern.LogicOccurrences.end(), [&](const auto &box) {
		const auto *logic = Find_Logic(document, box.strLogicId);
		return box.bEnabled && logic && logic->strJudgementKind == "PATTERN_COMPLETION_COUNT";
	});
}

// Server의 진입 조건과 같이 활성 ENTER_AREA 박스의 유일한 Success가 MARIO_ENTER Result인지 확인한다.
// 해당 결과는 Server에서 Mario 진입으로 처리하므로 로컬 preview만으로 표현할 수 없다.
bool Owns_ServerMarioEntry(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern)
{
	return std::any_of(pattern.LogicOccurrences.begin(), pattern.LogicOccurrences.end(), [&](const auto &box) {
		const auto *logic = Find_Logic(document, box.strLogicId);
		if (!box.bEnabled || !logic || logic->strTriggerKind != "ENTER_AREA" || box.OnSuccessLogicIds.size() != 1u)
			return false;
		const auto *result = Find_Logic(document, box.OnSuccessLogicIds.front());
		return result && result->strOutcomeKind == "MARIO_ENTER";
	});
}

// 부모의 자식 박스는 occurrence별 Logic ID를 부여한 확장 timeline에 들어 있다.
// 자식의 Mario 진입을 보려면 확장이 필요하며 문서를 복사하므로 클릭 처리에서만 호출한다.
bool Has_ServerMarioEntry(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern)
{
	if (Owns_ServerMarioEntry(document, pattern))
		return true;
	if (pattern.PatternOccurrences.empty())
		return false;
	KOUKU_SAYDON_COMPOSITION_DOCUMENT expanded;
	std::string status;
	if (!CKoukuSaydonCompositionDocument::Try_ExpandPatternDocument(document, pattern.strPatternId, expanded, status))
		return false;
	const auto *row = Find_Pattern(expanded, pattern.strPatternId);
	return row && Owns_ServerMarioEntry(expanded, *row);
}

// Server에서만 결과가 생기는 Pattern은 Server 시계를 따라 표시한다.
bool Follows_ServerClock(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern)
{
	const auto ownsServerCombat = [&](const KOUKU_SAYDON_COMPOSITION_DOCUMENT &source,
									  const KOUKU_SAYDON_COMPOSITION_PATTERN &owner) {
		return std::any_of(owner.LogicOccurrences.begin(), owner.LogicOccurrences.end(), [&](const auto &box) {
			if (!box.bEnabled)
				return false;
			const auto *logic = Find_Logic(source, box.strLogicId);
			if (logic &&
				(logic->strTriggerKind == "PURSUIT_PROJECTILES" || logic->strJudgementKind == "PURSUIT_PROJECTILES"))
				return true;
			const auto hasPush = [&](const auto &ids) {
				return std::any_of(ids.begin(), ids.end(), [&](const auto &id) {
					const auto *result = Find_Logic(source, id);
					return result && result->strOutcomeKind == "MAX_HP_PERCENT_DAMAGE" && result->fPushRangeM > 0.0 &&
						result->iPushMs > 0u;
				});
			};
			return hasPush(box.OnSuccessLogicIds) || hasPush(box.OnFailLogicIds) || hasPush(box.OnTimeoutLogicIds);
		});
	};
	if (Has_CompletionChain(document, pattern) || Has_ServerMarioEntry(document, pattern) ||
		ownsServerCombat(document, pattern))
		return true;
	if (pattern.PatternOccurrences.empty())
		return false;
	KOUKU_SAYDON_COMPOSITION_DOCUMENT expanded;
	std::string status;
	if (!CKoukuSaydonCompositionDocument::Try_ExpandPatternDocument(document, pattern.strPatternId, expanded, status))
		return false;
	const auto *row = Find_Pattern(expanded, pattern.strPatternId);
	return row && ownsServerCombat(expanded, *row);
}

using TIMELINE_DISPLAY_INTERVAL = CompositionTimeline::DISPLAY_INTERVAL;
using TIMELINE_DISPLAY_LAYOUT = CompositionTimeline::DISPLAY_LAYOUT;
} // 이 파일의 보조 구현 범위 끝

// Internal.h의 공유 선언을 여기 한 번 정의한다. 이름 있는 namespace는 다른 CPP의 같은 이름과 구분하며 문서나 Level을 소유하는 객체는 아니다.
namespace Client::KoukuWorkbenchDetail
{

// 활성 Level의 카메라 저작 자료를 준비한 뒤 stable shot ID로 조회한다. 반환 포인터는 Level 소유 목록의 다음 변경 전까지만 차용한다.
const CLevel_KoukuSaydonArena::KOUKU_CAMERA_SHOT *Find_AuthoringCamera(const std::string_view shotId)
{
	auto *level = CLevel_KoukuSaydonArena::Get_Active();
	std::string status;
	if (!level || !level->Ensure_CameraShotAuthoring(status))
		return nullptr;
	const auto &shots = level->Get_CameraShots();
	const auto found = std::find_if(shots.begin(), shots.end(), [&](const auto &shot) {
		return shot.strShotId == shotId;
	});
	return found != shots.end() ? &*found : nullptr;
}
// 카메라 리소스는 활성 Level의 저작 track 길이 또는 blend-in+hold(ms)를 우선 사용한다.
// shot을 조회할 수 없거나 카메라가 아니면 리소스 자체의 길이를 반환한다.
std::uint32_t Camera_DefaultDuration(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE &resource)
{
	if (resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::CAMERA)
		if (const auto *shot = Find_AuthoringCamera(resource.strAssetId))
			return shot->hasCameraTrack ? shot->CameraTrack.iDurationMs : shot->iBlendInMs + shot->iDefaultHoldMs;
	return resource.iDurationMs;
}

// 월드 고정 Effect 배치가 참조할 보스 spawn을 정본 Gameplay.world.json에서 읽는다.
// Gate·placement·actor·archetype과 좌표 범위를 모두 확인한 뒤에만 outPosition을 교체해 실패 시 이전 배치를 보존한다.
bool_t Resolve_PresentationBossSpawnPosition(const std::string &areaId, const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern,
	std::array<double, 3u> &outPosition, std::string &outStatus)
{
	if (areaId.empty() || pattern.strTargetBossPlacementId.empty() || pattern.strActorProfileId.empty() ||
		CKoukuSaydonCompositionDocument::Resolve_DefaultPlacementId(pattern.strGateId, pattern.strActorProfileId) !=
			pattern.strTargetBossPlacementId ||
		CKoukuSaydonCompositionDocument::Resolve_ActorProfileForPlacement(pattern.strTargetBossPlacementId) !=
			pattern.strActorProfileId)
	{
		outStatus = "World Effect requires this Pattern's exact Gate and boss target; previous placement preserved.";
		return false;
	}
	CWorldGameplayDocument world;
	std::string error;
	if (!world.Load(
			CProjectDataRoot::Resolve(std::filesystem::path("Worlds") / areaId / "Gameplay.world.json"), areaId, error))
	{
		outStatus = "Cannot resolve World Effect boss spawn: " + error;
		return false;
	}
	const auto *placement = world.Find(pattern.strTargetBossPlacementId);
	if (!placement || placement->eKind != WORLD_PLACEMENT_KIND::BOSS ||
		placement->archetypeId !=
			CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(pattern.strTargetBossPlacementId) ||
		CKoukuSaydonCompositionDocument::Resolve_ActorProfileForPlacement(placement->placementId) !=
			pattern.strActorProfileId)
	{
		outStatus = "World Effect boss spawn is unavailable or has a different actor profile: " +
			pattern.strTargetBossPlacementId;
		return false;
	}
	const std::array<double, 3u> position = {placement->position.x, placement->position.y, placement->position.z};
	if (!std::all_of(position.begin(), position.end(), [](const double value) {
			return std::isfinite(value) && value >= -100000.0 && value <= 100000.0;
		}))
	{
		outStatus =
			"World Effect boss spawn is outside the supported map position range; previous placement preserved.";
		return false;
	}
	outPosition = position;
	outStatus.clear();
	return true;
}

// Stage 누적 길이와 각 lane의 절대 ms 끝을 64비트로 비교한다. 상한을 넘으면 실패하고, 허용된 길이는 Pattern lifetime에 반영한다.
bool_t Extend_PatternLifetimeForAuthoredLanes(KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, std::string &status)
{
	std::uint64_t stageDurationMs = 0u;
	for (const auto &stage : pattern.Stages)
		stageDurationMs += stage.iDurationMs;
	std::uint64_t lifetime = (std::max)(stageDurationMs, std::uint64_t(pattern.iDurationMs));
	const auto includeLane = [&](const auto &rows) {
		for (const auto &row : rows)
			lifetime = (std::max)(lifetime, std::uint64_t(row.iStartMs) + row.iDurationMs);
	};
	includeLane(pattern.LogicOccurrences);
	includeLane(pattern.SummonOccurrences);
	includeLane(pattern.WorldOccurrences);
	includeLane(pattern.PresentationOccurrences);
	includeLane(pattern.PatternOccurrences);
	includeLane(pattern.SceneProfileOccurrences);
	if (pattern.BossMotion)
		lifetime = (std::max)(lifetime, std::uint64_t(pattern.BossMotion->iEndMs));
	if (lifetime > MAX_EDITOR_TIME_MS)
	{
		status = "Stage edit would exceed the 600000 ms Pattern lifetime.";
		return false;
	}
	if (pattern.iDurationMs || lifetime > stageDurationMs)
		pattern.iDurationMs = static_cast<std::uint32_t>(lifetime);
	return true;
}

// 아래 Pattern/Stage/occurrence 조회는 입력 문서가 소유한 vector의 행을 잠시 차용한다.
// 없는 stable ID는 nullptr 또는 빈 MUTABLE_OCCURRENCE로 알리며, vector 변경이나 문서 교체를 넘어 포인터를 보관하지 않는다.
KOUKU_SAYDON_COMPOSITION_PATTERN *Find_Pattern(
	KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const std::string_view patternId)
{
	const auto found = std::find_if(document.Patterns.begin(), document.Patterns.end(),
		[patternId](const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern) {
			return pattern.strPatternId == patternId;
		});
	return found == document.Patterns.end() ? nullptr : &*found;
}

const KOUKU_SAYDON_COMPOSITION_PATTERN *Find_Pattern(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const std::string_view patternId)
{
	const auto found = std::find_if(document.Patterns.begin(), document.Patterns.end(),
		[patternId](const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern) {
			return pattern.strPatternId == patternId;
		});
	return found == document.Patterns.end() ? nullptr : &*found;
}

KOUKU_SAYDON_COMPOSITION_STAGE *Find_Stage(KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view stageId)
{
	const auto found = std::find_if(
		pattern.Stages.begin(), pattern.Stages.end(), [stageId](const KOUKU_SAYDON_COMPOSITION_STAGE &stage) {
			return stage.strStageId == stageId;
		});
	return found == pattern.Stages.end() ? nullptr : &*found;
}

const KOUKU_SAYDON_COMPOSITION_STAGE *Find_Stage(
	const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view stageId)
{
	const auto found = std::find_if(
		pattern.Stages.begin(), pattern.Stages.end(), [stageId](const KOUKU_SAYDON_COMPOSITION_STAGE &stage) {
			return stage.strStageId == stageId;
		});
	return found == pattern.Stages.end() ? nullptr : &*found;
}

MUTABLE_OCCURRENCE Find_Occurrence(KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view occurrenceId)
{
	for (KOUKU_SAYDON_COMPOSITION_STAGE &stage : pattern.Stages)
	{
		const auto found = std::find_if(stage.AnimationOccurrences.begin(), stage.AnimationOccurrences.end(),
			[occurrenceId](const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE &occurrence) {
				return occurrence.strOccurrenceId == occurrenceId;
			});
		if (found != stage.AnimationOccurrences.end())
			return {&stage, &*found};
	}
	return {};
}

const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE *Find_Occurrence(const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern,
	const std::string_view occurrenceId, const KOUKU_SAYDON_COMPOSITION_STAGE **const ppStage)
{
	for (const KOUKU_SAYDON_COMPOSITION_STAGE &stage : pattern.Stages)
	{
		const auto found = std::find_if(stage.AnimationOccurrences.begin(), stage.AnimationOccurrences.end(),
			[occurrenceId](const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE &occurrence) {
				return occurrence.strOccurrenceId == occurrenceId;
			});
		if (found != stage.AnimationOccurrences.end())
		{
			if (nullptr != ppStage)
				*ppStage = &stage;
			return &*found;
		}
	}
	return nullptr;
}

const KOUKU_SAYDON_COMPOSITION_PATTERN_OCCURRENCE *Find_PatternBox(
	const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view id)
{
	const auto found =
		std::find_if(pattern.PatternOccurrences.begin(), pattern.PatternOccurrences.end(), [&](const auto &row) {
			return row.strOccurrenceId == id;
		});
	return found == pattern.PatternOccurrences.end() ? nullptr : &*found;
}

// Pattern을 자신의 timeline으로 지정한 Folder를 찾는다. PatternOccurrences의 부모 그래프 탐색과는
// 다른 조회이며 반환값은 문서 Folder vector의 차용 포인터다.
const KOUKU_SAYDON_COMPOSITION_FOLDER *Find_TimelineParent(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const std::string_view patternId)
{
	const auto found = std::find_if(document.Folders.begin(), document.Folders.end(), [&](const auto &row) {
		return !row.strTimelinePatternId.empty() && row.strTimelinePatternId == patternId;
	});
	return found == document.Folders.end() ? nullptr : &*found;
}

// 여러 lane의 stable occurrence ID를 같은 선택 공간에서 확인한다. Stage ID는 별도 Find_Stage로
// 확인하므로 Stage와 box를 혼동하지 않고, 존재 여부만 돌려 문서 상태를 바꾸지 않는다.
bool Has_TimelineOccurrence(const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view id)
{
	return Find_PatternBox(pattern, id) || Find_Occurrence(pattern, id) ||
		Has_TimelineBox(pattern.LogicOccurrences, id) || Has_TimelineBox(pattern.SummonOccurrences, id) ||
		Has_TimelineBox(pattern.WorldOccurrences, id) || Has_TimelineBox(pattern.SceneProfileOccurrences, id) ||
		Has_TimelineBox(pattern.PresentationOccurrences, id);
}

const char_t *Actor_Label(const std::string_view actorProfileId, const std::string_view gateId)
{
	if ("MN_RPCZ_00" == actorProfileId)
		return "Kouku";
	if ("MN_RPCT_05" == actorProfileId)
		return gateId == "GATE3" ? "KoukuSaydon" : "Saydon";
	if ("MN_RPCT_06" == actorProfileId)
		return "Large Saydon";
	return "Unknown model";
}

// 리소스 검색의 바이트 단위 부분 문자열 비교다. tolower에는 unsigned char를 넘겨 signed char의
// 잘못된 입력을 피한다. Unicode 정규화나 다국어 대소문자 변환을 수행하는 검색은 아니다.
bool_t ContainsInsensitive(const std::string_view text, const std::string_view query)
{
	if (query.empty())
		return true;
	if (query.size() > text.size())
		return false;
	for (std::size_t offset = 0u; offset + query.size() <= text.size(); ++offset)
	{
		std::size_t index = 0u;
		for (; index < query.size(); ++index)
		{
			const unsigned char left = static_cast<unsigned char>(text[offset + index]);
			const unsigned char right = static_cast<unsigned char>(query[index]);
			if (std::tolower(left) != std::tolower(right))
				break;
		}
		if (index == query.size())
			return true;
	}
	return false;
}

// 문서의 현재 순서대로 정상 PRODUCT Pattern ID만 Play All 목록에 다시 모은다.
// DRAFT 또는 읽기 실패 행은 제외하므로 Mark_Draft가 배포 상태를 바꾼 뒤 이 목록도 갱신한다.
void Rebuild_PlayAllPatternIds(KOUKU_SAYDON_COMPOSITION_DOCUMENT &document)
{
	document.PlayAllPatternIds.clear();
	for (const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern : document.Patterns)
	{
		if (pattern.strLoadError.empty() && "PRODUCT" == pattern.strAuthoringStatus)
			document.PlayAllPatternIds.push_back(pattern.strPatternId);
	}
}

// 수정 Pattern을 포함하는 부모를 반복 탐색하여 DRAFT 표시를 전파한다. 방문 집합으로 이미 포함한 ID의 재추가를 막는다.
void Mark_Draft(KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, KOUKU_SAYDON_COMPOSITION_PATTERN &pattern)
{
	pattern.strAuthoringStatus = "DRAFT";
	std::unordered_set<std::string> changed{pattern.strPatternId};
	bool added = true;
	while (added)
	{
		added = false;
		for (auto &parent : document.Patterns)
			if (parent.strLoadError.empty() && !changed.contains(parent.strPatternId) &&
				std::any_of(parent.PatternOccurrences.begin(), parent.PatternOccurrences.end(), [&](const auto &box) {
					return changed.contains(box.strPatternId);
				}))
			{
				parent.strAuthoringStatus = "DRAFT";
				changed.insert(parent.strPatternId);
				added = true;
			}
	}
	for (auto &bundle : document.Bundles)
		if (std::any_of(bundle.Members.begin(), bundle.Members.end(), [&](const auto &member) {
				return changed.contains(member.strPatternId);
			}))
			bundle.strAuthoringStatus = "DRAFT";
	Rebuild_PlayAllPatternIds(document);
}

// 명시 lifetime이 있으면 그것을 사용하고 없으면 Stage 길이를 누적한다. 반환값은 ms이며
// 표시 가능한 uint32 범위로 제한한다. 저작 상한 검사는 Extend_PatternLifetimeForAuthoredLanes가 별도로 맡는다.
std::uint32_t Pattern_DurationMs(const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern)
{
	if (pattern.iDurationMs)
		return pattern.iDurationMs;
	std::uint64_t duration = 0u;
	for (const KOUKU_SAYDON_COMPOSITION_STAGE &stage : pattern.Stages)
		duration += stage.iDurationMs;
	return static_cast<std::uint32_t>(
		(std::min)(duration, static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)())));
}

// 이하 정의·lane 조회 함수들은 ID의 종류에 맞는 vector만 검색한다. 정의 ID와 occurrence ID는
// 서로 대체할 수 없으며 반환 포인터는 해당 문서/Pattern 소유 행의 현재 수명 안에서만 사용한다.
const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION *Find_Logic(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const std::string_view logicId)
{
	const auto found = std::find_if(document.Logics.begin(), document.Logics.end(),
		[logicId](const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION &logic) {
			return logic.strLogicId == logicId;
		});
	return found == document.Logics.end() ? nullptr : &*found;
}

const KOUKU_SAYDON_COMPOSITION_PATTERN_FLOW *Find_BingoFlow(const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document)
{
	const auto found = std::find_if(document.PatternFlows.begin(), document.PatternFlows.end(), [](const auto &flow) {
		return flow.strGateId == "BINGO";
	});
	return found == document.PatternFlows.end() ? nullptr : &*found;
}

// BINGO의 DURATION/BINGO_BOARD Logic 행이 정확히 하나인 전용 제어 Pattern인지 판정한다.
// 해당 행은 enabled 여부와 무관하게 세고, 다른 활성 Logic이나 다른 lane·보스 이동/복귀 설정이 있으면 제외한다.
bool Is_BingoBoardControl(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern)
{
	if (pattern.strGateId != "BINGO" || pattern.bEnterCombatOnFinish || pattern.bResetBossToSpawn ||
		pattern.ResetBossYawDegrees || pattern.BossMotion || !pattern.PatternOccurrences.empty())
		return false;
	for (const auto &stage : pattern.Stages)
		if (!stage.AnimationOccurrences.empty())
			return false;
	if (!pattern.SummonOccurrences.empty() || !pattern.WorldOccurrences.empty() ||
		!pattern.SceneProfileOccurrences.empty() || !pattern.PresentationOccurrences.empty())
		return false;
	size_t count = 0;
	for (const auto &box : pattern.LogicOccurrences)
	{
		const auto *logic = Find_Logic(document, box.strLogicId);
		if (logic && logic->strLogicType == "DURATION" && logic->strJudgementKind == "BINGO_BOARD")
			++count;
		else if (box.bEnabled)
			return false;
	}
	return count == 1;
}

KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE *Find_LogicBox(
	KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view occurrenceId)
{
	const auto found = std::find_if(pattern.LogicOccurrences.begin(), pattern.LogicOccurrences.end(),
		[occurrenceId](const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE &box) {
			return box.strOccurrenceId == occurrenceId;
		});
	return found == pattern.LogicOccurrences.end() ? nullptr : &*found;
}

const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE *Find_LogicBox(
	const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view occurrenceId)
{
	const auto found = std::find_if(pattern.LogicOccurrences.begin(), pattern.LogicOccurrences.end(),
		[occurrenceId](const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE &box) {
			return box.strOccurrenceId == occurrenceId;
		});
	return found == pattern.LogicOccurrences.end() ? nullptr : &*found;
}

const KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION *Find_Summon(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const std::string_view summonId)
{
	const auto found = std::find_if(document.Summons.begin(), document.Summons.end(),
		[summonId](const KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION &summon) {
			return summon.strSummonId == summonId;
		});
	return found == document.Summons.end() ? nullptr : &*found;
}

KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE *Find_SummonBox(
	KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view occurrenceId)
{
	const auto found = std::find_if(pattern.SummonOccurrences.begin(), pattern.SummonOccurrences.end(),
		[occurrenceId](const KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE &box) {
			return box.strOccurrenceId == occurrenceId;
		});
	return found == pattern.SummonOccurrences.end() ? nullptr : &*found;
}

const KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE *Find_SummonBox(
	const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view occurrenceId)
{
	const auto found = std::find_if(pattern.SummonOccurrences.begin(), pattern.SummonOccurrences.end(),
		[occurrenceId](const KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE &box) {
			return box.strOccurrenceId == occurrenceId;
		});
	return found == pattern.SummonOccurrences.end() ? nullptr : &*found;
}

const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION *Find_World(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const std::string_view worldId)
{
	const auto found = std::find_if(document.Worlds.begin(), document.Worlds.end(),
		[worldId](const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION &world) {
			return world.strWorldId == worldId;
		});
	return found == document.Worlds.end() ? nullptr : &*found;
}

KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE *Find_WorldBox(
	KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view occurrenceId)
{
	const auto found = std::find_if(pattern.WorldOccurrences.begin(), pattern.WorldOccurrences.end(),
		[occurrenceId](const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE &box) {
			return box.strOccurrenceId == occurrenceId;
		});
	return found == pattern.WorldOccurrences.end() ? nullptr : &*found;
}

const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE *Find_WorldBox(
	const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view occurrenceId)
{
	const auto found = std::find_if(pattern.WorldOccurrences.begin(), pattern.WorldOccurrences.end(),
		[occurrenceId](const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE &box) {
			return box.strOccurrenceId == occurrenceId;
		});
	return found == pattern.WorldOccurrences.end() ? nullptr : &*found;
}

const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION *Find_SceneProfile(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const std::string_view sceneProfileId)
{
	const auto found = std::find_if(document.SceneProfiles.begin(), document.SceneProfiles.end(),
		[sceneProfileId](const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION &profile) {
			return profile.strSceneProfileId == sceneProfileId;
		});
	return found == document.SceneProfiles.end() ? nullptr : &*found;
}

KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE *Find_SceneProfileBox(
	KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view occurrenceId)
{
	const auto found = std::find_if(pattern.SceneProfileOccurrences.begin(), pattern.SceneProfileOccurrences.end(),
		[occurrenceId](const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE &box) {
			return box.strOccurrenceId == occurrenceId;
		});
	return found == pattern.SceneProfileOccurrences.end() ? nullptr : &*found;
}

const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE *Find_SceneProfileBox(
	const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view occurrenceId)
{
	const auto found = std::find_if(pattern.SceneProfileOccurrences.begin(), pattern.SceneProfileOccurrences.end(),
		[occurrenceId](const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE &box) {
			return box.strOccurrenceId == occurrenceId;
		});
	return found == pattern.SceneProfileOccurrences.end() ? nullptr : &*found;
}

const char *Presentation_Label(const KOUKU_SAYDON_PRESENTATION_KIND kind)
{
	switch (kind)
	{
	case KOUKU_SAYDON_PRESENTATION_KIND::EFFECT:
		return "Effect";
	case KOUKU_SAYDON_PRESENTATION_KIND::SOUND:
		return "Sound";
	case KOUKU_SAYDON_PRESENTATION_KIND::SUBTITLE:
		return "Subtitle";
	case KOUKU_SAYDON_PRESENTATION_KIND::CAMERA:
		return "Camera";
	case KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER:
		return "Collider";
	case KOUKU_SAYDON_PRESENTATION_KIND::LIGHT:
		return "Light";
	case KOUKU_SAYDON_PRESENTATION_KIND::WORLD:
		return "World";
	case KOUKU_SAYDON_PRESENTATION_KIND::SCENE_PROFILE:
		return "Scene Profile";
	default:
		return "Unavailable";
	}
}

const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE *Find_PresentationResource(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const std::string_view id)
{
	const auto found = std::find_if(
		document.PresentationResources.begin(), document.PresentationResources.end(), [id](const auto &row) {
			return row.strResourceId == id;
		});
	return found == document.PresentationResources.end() ? nullptr : &*found;
}

const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE *Find_PresentationBox(
	const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view id)
{
	const auto found = std::find_if(
		pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(), [id](const auto &row) {
			return row.strOccurrenceId == id;
		});
	return found == pattern.PresentationOccurrences.end() ? nullptr : &*found;
}

const char_t *Outcome_SlotLabel(const KOUKU_SAYDON_OUTCOME_SLOT slot)
{
	return KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS == slot ? "Success"
													  : (KOUKU_SAYDON_OUTCOME_SLOT::FAIL == slot ? "Fail" : "Timeout");
}

// ImGui 고정 입력 버퍼에 들어갈지 먼저 확인한 뒤 문자열과 종료 문자를 복사한다.
// 용량 부족이나 null 출력 버퍼는 false로 알리고 기존 버퍼 내용은 건드리지 않는다.
bool_t Copy_Text(char_t *const destination, const std::size_t capacity, const std::string_view source)
{
	if (nullptr == destination || 0u == capacity || source.size() >= capacity)
		return false;
	memcpy(destination, source.data(), source.size());
	destination[source.size()] = '\0';
	return true;
}

// 선택한 presentation이 그룹에 속하면 같은 selection group의 행 ID도 함께 넣는다.
// 기하나 문서를 변경하지 않고 ID 집합만 확장하므로 Copy/Move/선택 UI가 같은 묶음 경계를 사용할 수 있다.
void Expand_PresentationSelection(const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, std::vector<std::string> &ids)
{
	std::unordered_set<std::string> selected(ids.begin(), ids.end()), groups;
	for (const auto &box : pattern.PresentationOccurrences)
		if (selected.contains(box.strOccurrenceId) && !box.strSelectionGroupId.empty())
			groups.insert(box.strSelectionGroupId);
	for (const auto &box : pattern.PresentationOccurrences)
		if (groups.contains(box.strSelectionGroupId) && selected.insert(box.strOccurrenceId).second)
			ids.push_back(box.strOccurrenceId);
}

// 그룹과 연결 Logic의 이동 범위를 먼저 계산한다. 유효한 공통 delta를 결과에 담고 실제 행 변경은 호출자의 후보에서 수행한다.
bool Prepare_PresentationTimelineMove(const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document,
	const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::vector<std::string> &selectedIds,
	const std::int64_t requestedDeltaMs, PRESENTATION_TIMELINE_MOVE &outMove, std::string &outStatus)
{
	const auto reject = [&](const char *reason) {
		outStatus = reason;
		return false;
	};
	if (!pattern.strLoadError.empty() || selectedIds.empty())
		return reject("Select Effects, Sounds or a complete saved Collider group before moving its timeline.");
	const std::unordered_set<std::string> selected(selectedIds.begin(), selectedIds.end());
	if (selected.size() != selectedIds.size())
		return reject("Selection contains duplicate IDs; previous timing preserved.");
	PRESENTATION_TIMELINE_MOVE staged;
	std::unordered_set<std::string> linkedIds, groups;
	std::optional<KOUKU_SAYDON_PRESENTATION_KIND> kind;
	std::string colliderGroup;
	std::int64_t earliest = Pattern_DurationMs(pattern), latest = 0;
	for (const auto &id : selectedIds)
	{
		const auto *box = Find_PresentationBox(pattern, id);
		const auto *resource = box ? Find_PresentationResource(document, box->strResourceId) : nullptr;
		if (!box || !resource ||
			(resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER &&
				resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT &&
				resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::SOUND))
			return reject("Select Effects and Sounds or one saved Collider group; previous timing preserved.");
		if (kind &&
			((*kind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER) !=
				(resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER)))
			return reject("Collider groups move separately from Effect and Sound selections.");
		kind = resource->eKind;
		if (*kind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER)
		{
			if (selectedIds.size() < 2u || box->strSelectionGroupId.empty())
				return reject("Save a Collider selection group before dragging.");
			if (colliderGroup.empty())
				colliderGroup = box->strSelectionGroupId;
			if (colliderGroup != box->strSelectionGroupId)
				return reject("Drag one saved Collider group at a time.");
		}
		if (!box->strSelectionGroupId.empty())
			groups.insert(box->strSelectionGroupId);
		const auto end = std::int64_t(box->iStartMs) + box->iDurationMs;
		if (!box->iDurationMs || end > Pattern_DurationMs(pattern))
			return reject("A selected window is outside the Pattern lifetime; previous timing preserved.");
		earliest = (std::min)(earliest, std::int64_t(box->iStartMs));
		latest = (std::max)(latest, end);
		staged.occurrenceIds.push_back(id);
		if (*kind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER && !box->strLogicOccurrenceId.empty())
		{
			const auto *logic = Find_LogicBox(pattern, box->strLogicOccurrenceId);
			if (!logic || logic->iStartMs != box->iStartMs || logic->iDurationMs != box->iDurationMs)
				return reject(
					"A linked Logic window is unavailable or has different timing; previous timing preserved.");
			if (linkedIds.insert(logic->strOccurrenceId).second)
				staged.logicIds.push_back(logic->strOccurrenceId);
		}
	}
	for (const auto &box : pattern.PresentationOccurrences)
	{
		if (groups.contains(box.strSelectionGroupId) && !selected.contains(box.strOccurrenceId))
			return reject("Group membership changed; select the whole group again before dragging.");
		if (linkedIds.contains(box.strLogicOccurrenceId) && !selected.contains(box.strOccurrenceId))
			return reject(
				"This group's Logic window is shared with a Collider outside the group. Group those boxes together or unlink that shared window before moving.");
	}
	staged.deltaMs = std::clamp(requestedDeltaMs, -earliest, std::int64_t(Pattern_DurationMs(pattern)) - latest);
	outMove = std::move(staged);
	outStatus.clear();
	return true;
}

// Collider를 같은 기하 편집 그룹으로 다룰 수 있는지 anchor·bone·world·부착 Effect 기준을 대조한다.
// owner를 따라가지 않는 경우에는 시작 시각도 같아야 같은 고정 frame으로 취급한다.
bool Same_ColliderGroupFrame(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &a,
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &b)
{
	return a.strAnchorKind == b.strAnchorKind && a.bFollowBoss == b.bFollowBoss && a.strBone == b.strBone &&
		a.strBoneTarget == b.strBoneTarget && a.strBoneRotation == b.strBoneRotation && a.strWorldId == b.strWorldId &&
		a.strWorldOccurrenceId == b.strWorldOccurrenceId && a.iWorldEmissionIndex == b.iWorldEmissionIndex &&
		a.strWorldEffectTrackId == b.strWorldEffectTrackId &&
		a.strAnchorPresentationOccurrenceId == b.strAnchorPresentationOccurrenceId &&
		(a.bFollowBoss || a.iStartMs == b.iStartMs);
}
// 편집 값의 위치·회전·scale 및 선형 이동 끝이 유한한 허용 범위 안인지 검사한다.
// 입력은 값 배열이며 검사만 수행한다. anchor 정책의 의미 검증은 Valid_PresentationPlacement가 이어서 담당한다.
bool Valid_PresentationGeometry(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &value)
{
	const auto finite = [](const auto &values, const double minimum, const double maximum) {
		return std::all_of(values.begin(), values.end(), [=](const double v) {
			return std::isfinite(v) && v >= minimum && v <= maximum;
		});
	};
	return finite(value.PositionOffset, -100000.0, 100000.0) && finite(value.RotationDegrees, -36000.0, 36000.0) &&
		finite(value.Scale, 0.001, 10000.0) &&
		(value.strColliderMotion == "STATIC" ||
			(value.strColliderMotion == "LINEAR" && finite(value.ColliderEndPositionOffset, -100000.0, 100000.0) &&
				finite(value.ColliderEndScale, .001, 10000.0)));
}

bool Valid_GameplayColliderScale(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE &resource,
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &value)
{
	return resource.eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER || Valid_PresentationGeometry(value);
}

// pending geometry를 candidate/preview 값에 합칠 때 배치·anchor·motion 필드만 복사한다.
// 시간 창과 stable ID는 유지하며 Effect 전용 follow/bone 설정은 effect 인자가 true일 때만 복사한다.
void Copy_PresentationPlacement(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &source,
	KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &target, const bool effect)
{
	target.PositionOffset = source.PositionOffset;
	target.RotationDegrees = source.RotationDegrees;
	target.Scale = source.Scale;
	target.strColliderMotion = source.strColliderMotion;
	target.strAnchorPresentationOccurrenceId = source.strAnchorPresentationOccurrenceId;
	target.strWorldEffectTrackId = source.strWorldEffectTrackId;
	target.ColliderEndPositionOffset = source.ColliderEndPositionOffset;
	target.ColliderEndScale = source.ColliderEndScale;
	if (!effect)
		return;
	target.strAnchorKind = source.strAnchorKind;
	target.bFollowBoss = source.bFollowBoss;
	target.strBone = source.strBone;
	target.strBoneTarget = source.strBoneTarget;
	target.strBoneRotation = source.strBoneRotation;
	target.strWorldId = source.strWorldId;
	target.strWorldOccurrenceId = source.strWorldOccurrenceId;
	target.iWorldEmissionIndex = source.iWorldEmissionIndex;
}

// 저장된 배치와 pending 배치가 같은지 복사 대상과 대응되는 필드로 비교한다.
// Effect 이외 종류는 Effect 전용 anchor 차이를 비교에 포함하지 않는다.
bool Same_PresentationPlacement(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &left,
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &right, const bool effect)
{
	return left.PositionOffset == right.PositionOffset && left.RotationDegrees == right.RotationDegrees &&
		left.Scale == right.Scale && left.strColliderMotion == right.strColliderMotion &&
		left.strAnchorPresentationOccurrenceId == right.strAnchorPresentationOccurrenceId &&
		left.strWorldEffectTrackId == right.strWorldEffectTrackId &&
		left.ColliderEndPositionOffset == right.ColliderEndPositionOffset &&
		left.ColliderEndScale == right.ColliderEndScale &&
		(!effect ||
			(left.strAnchorKind == right.strAnchorKind && left.bFollowBoss == right.bFollowBoss &&
				left.strBone == right.strBone && left.strBoneTarget == right.strBoneTarget &&
				left.strBoneRotation == right.strBoneRotation && left.strWorldId == right.strWorldId &&
				left.strWorldOccurrenceId == right.strWorldOccurrenceId &&
				left.iWorldEmissionIndex == right.iWorldEmissionIndex));
}

// 수치 기하 검증에 더해 WORLD/BOSS/MAP anchor, bone, follow 정책의 조합을 검사한다.
// 호출자가 리소스 종류에 맞지 않는 기하 요청을 거절할 수 있게 값을 변경하지 않고 bool 결과만 반환한다.
bool Valid_PresentationPlacement(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE &resource,
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &value)
{
	if (!Valid_PresentationGeometry(value))
		return false;
	const bool collider = resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER;
	const bool worldBone =
		collider && value.strAnchorKind == "WORLD" && value.bFollowBoss && value.strBoneTarget == "BODY";
	if (!value.strWorldEffectTrackId.empty() &&
		(!worldBone || value.strBone.empty() || value.strBoneRotation != "BONE" || value.strWorldOccurrenceId.empty()))
		return false;
	if (value.strBoneRotation != "TARGET_YAW" &&
		(value.strBoneRotation != "BONE" || (resource.eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT && !collider) ||
			(value.strAnchorKind != "BOSS" && !worldBone) || value.strBone.empty() || (collider && !value.bFollowBoss)))
		return false;
	if (resource.eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)
		return true;
	if (value.strAnchorKind == "BOSS" || value.strAnchorKind == "WORLD")
		return true;
	return value.strAnchorKind == "MAP" && !value.bFollowBoss && value.strBone.empty() &&
		value.strBoneTarget == "BODY" && value.strWorldId.empty() && value.strWorldOccurrenceId.empty() &&
		value.iWorldEmissionIndex == 0u;
}

// 저장/preview 후보의 stable occurrence ID에 overlay를 합친다. 시간 필드는 유지하고 없는 행이나 유효하지 않은 배치는 실패로 전달한다.
bool Apply_PresentationPreviewPlacements(KOUKU_SAYDON_COMPOSITION_DOCUMENT &candidate,
	const std::vector<KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST> &edits, const std::vector<std::string> &patternIds,
	std::string &outStatus)
{
	std::unordered_set<std::string> owners;
	auto pendingOwners = patternIds;
	while (!pendingOwners.empty())
	{
		const auto id = std::move(pendingOwners.back());
		pendingOwners.pop_back();
		if (!owners.insert(id).second)
			continue;
		if (const auto *pattern = Find_Pattern(candidate, id))
			for (const auto &child : pattern->PatternOccurrences)
				pendingOwners.push_back(child.strPatternId);
	}
	for (const auto &edit : edits)
	{
		if (!owners.contains(edit.strPatternId))
			continue;
		auto *pattern = Find_Pattern(candidate, edit.strPatternId);
		const auto *source = pattern ? Find_PresentationBox(*pattern, edit.Occurrence.strOccurrenceId) : nullptr;
		const auto *resource = source ? Find_PresentationResource(candidate, source->strResourceId) : nullptr;
		if (!source || source->strResourceId != edit.Occurrence.strResourceId || !resource ||
			!Valid_PresentationPlacement(*resource, edit.Occurrence))
		{
			outStatus =
				"Preview rejected: correct or revert the pending presentation placement. Applied draft and source are unchanged.";
			return false;
		}
		for (auto &box : pattern->PresentationOccurrences)
			if (box.strOccurrenceId == edit.Occurrence.strOccurrenceId)
			{
				Copy_PresentationPlacement(
					edit.Occurrence, box, resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT);
				break;
			}
	}
	return true;
}
const char *Gate_Label(const std::string_view gate)
{
	if (gate == "GATE1")
		return "\x31\xEA\xB4\x80\xEB\xAC\xB8";
	if (gate == "GATE2")
		return "\x32\xEA\xB4\x80\xEB\xAC\xB8";
	if (gate == "GATE3")
		return "\x33\xEA\xB4\x80\xEB\xAC\xB8";
	if (gate == "BINGO")
		return "\xEB\xB9\x99\xEA\xB3\xA0";
	return "Unknown Gate";
}
// Bundle 멤버의 offset+Pattern 길이와 scene/presentation lane의 끝을 비교해 가장 늦은 ms를 반환한다.
// 없는 Pattern은 멤버 계산에서 건너뛰며 반환값은 편집 상한으로 제한한다. 참조 유효성 검증을 대신하지 않는다.
std::uint32_t Bundle_DurationMs(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &doc, const KOUKU_SAYDON_COMPOSITION_BUNDLE &bundle)
{
	std::uint64_t end = 0;
	for (const auto &member : bundle.Members)
		if (const auto *pattern = Find_Pattern(doc, member.strPatternId))
			end = (std::max)(end, std::uint64_t(member.iStartOffsetMs) + Pattern_DurationMs(*pattern));
	for (const auto &row : bundle.SceneProfileOccurrences)
		end = (std::max)(end, std::uint64_t(row.iStartMs) + row.iDurationMs);
	for (const auto &row : bundle.PresentationOccurrences)
		end = (std::max)(end, std::uint64_t(row.iStartMs) + row.iDurationMs);
	return static_cast<std::uint32_t>((std::min)(end, std::uint64_t(MAX_EDITOR_TIME_MS)));
}
} // 공용 저작 보조 함수 범위 끝: Client::KoukuWorkbenchDetail

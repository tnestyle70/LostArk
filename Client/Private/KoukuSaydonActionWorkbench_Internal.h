#pragma once

// 이 헤더는 분리된 Workbench 구현끼리만 공유한다. 문서 행은 호출 동안 차용하며
// Commit_Candidate/Reload/Undo가 문서를 교체한 뒤에는 포인터를 다시 조회해야 한다.
#include "KoukuSaydonActionWorkbench.h"
// 문서 행·요청 구조와 기본 별칭을 가져온다. Find_Bundle/Find_Folder의 템플릿 본문은 실제 container 멤버를 읽는다.
// 아래 template의 선형 stable ID 조회 및 고정 label 목록이다.
#include <algorithm>
// std::any_of와 std::find_if로 해당 vector만 순회하며 결과를 찾으면 종료한다.
#include <array>
// 분류/종료 정책 개수를 타입에 고정하고 위치 출력의 세 좌표를 같은 값으로 전달한다.
#include <unordered_set>
// 현재 이 헤더 선언 자체에는 set 값이 없다. Helpers.cpp의 방문/선택 중복 제거 집합도 같은 표준 타입을 사용한다.

// 여러 CPP가 같은 문서 조회·검증 구현을 호출하므로 이름 있는 내부 도메인으로 연결한다.
// 일반 함수 정의는 Helpers.cpp 한 곳에 두고, 타입에 따라 생성되는 아래 템플릿 정의만 헤더에 둔다.
namespace Client::KoukuWorkbenchDetail
{
// 편집 입력과 lane 수명 검증이 공유하는 600000 ms(10분) 상한이다. 실제 재생 시계나 측정된 성능 한도가 아니다.
// constexpr 값과 아래 고정 배열은 번역 단위마다 같은 값으로 읽으며 변경 가능한 전역 상태를 만들지 않는다.
constexpr std::uint32_t MAX_EDITOR_TIME_MS = 600000u;

// Create/Details가 같은 저장 토큰을 선택한다. 항목 수가 고정되어 검색 중 목록을 증감할 상태가 필요하지 않다.
constexpr std::array<const char_t *, 2u> PATTERN_CATEGORIES = {"NORMAL", "MECHANIC"};

// Stage와 그 안의 animation 행을 함께 차용한다. 문서 교체 뒤 보관해서 사용하지 않는다.
struct MUTABLE_OCCURRENCE final
{
	KOUKU_SAYDON_COMPOSITION_STAGE *pStage = nullptr;
	KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE *pOccurrence = nullptr;
};

// Has_TimelineOccurrence가 여러 lane의 존재 여부를 묻는다. 공통 strOccurrenceId 필드만 요구해 행 전체를 변환·복사하지 않는다.
// 각 lane의 현재 행을 앞에서부터 확인한다. 없는 ID는 false이며 입력 목록은 바꾸지 않는다.
template <class Rows> bool Has_TimelineBox(const Rows &rows, const std::string_view id)
{
	return std::any_of(rows.begin(), rows.end(), [&](const auto &row) {
		return row.strOccurrenceId == id;
	});
}

// Animation 입력 UI와 Set_AnimationPlayback의 허용값 검사가 같은 세 저장 토큰을 사용한다.
constexpr std::array<const char_t *, 3u> END_POLICIES = {"EXACT", "HOLD_LAST_POSE", "LOOP_TO_WINDOW"};

// 이동 전 검증 결과다. 선택/종속 ID와 모든 행에 적용할 공통 ms 차이만 값으로 보관한다.
struct PRESENTATION_TIMELINE_MOVE
{
	// occurrenceIds는 Presentation 배치, logicIds는 함께 움직여야 할 Logic 배치 ID다. 공유 Logic 정의 ID가 아니다.
	std::vector<std::string> occurrenceIds, logicIds;
	std::int64_t deltaMs = 0;
};

// const 문서와 편집 후보 모두 같은 조회 규칙을 사용한다. ID가 없으면 nullptr를 돌려준다.
// Document의 const 여부가 반환 포인터에도 유지된다. 읽기용 문서에서 수정 가능한 행을 얻지 못하게 하는 타입 경계다.
// decltype의 front()는 반환 타입을 구하는 식이라 실행되지 않는다. 실제 빈 vector는 end 비교로 nullptr가 된다.
template <class Document>
auto Find_Bundle(Document &document, const std::string_view id) -> decltype(&document.Bundles.front())
{
	auto it = std::find_if(document.Bundles.begin(), document.Bundles.end(), [&](const auto &item) {
		return item.strBundleId == id;
	});
	return it == document.Bundles.end() ? nullptr : &*it;
}

// 계층 편집/표시에서 folder ID를 찾는다. Bundle 조회와 같은 const 전파·선형 검색·부재 시 nullptr 계약을 사용한다.
template <class Document>
auto Find_Folder(Document &document, const std::string_view id) -> decltype(&document.Folders.front())
{
	auto it = std::find_if(document.Folders.begin(), document.Folders.end(), [&](const auto &item) {
		return item.strFolderId == id;
	});
	return it == document.Folders.end() ? nullptr : &*it;
}

// 카메라 box의 기본 길이(ms)를 정한다. 활성 Level의 shot을 읽을 수 있으면 track 또는 blend-in+hold를 쓰고, 없으면 resource.iDurationMs로 돌아간다.
std::uint32_t Camera_DefaultDuration(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE &resource);

// World 고정 Effect의 기준 위치를 정본 Gameplay.world.json에서 구한다. 관문·배치·actor·좌표를 모두 확인한 뒤 outPosition(m)을 바꾸며 실패하면 이전 출력 좌표를 보존한다.
bool_t Resolve_PresentationBossSpawnPosition(const std::string &areaId, const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern,
	std::array<double, 3u> &outPosition, std::string &outStatus);

// Stage 편집 후보가 다른 lane의 끝을 잘라내지 않도록 전체 끝 시각을 64비트 ms로 모은다. 상한 초과는 status와 false로 알리고 iDurationMs를 바꾸지 않는다.
bool_t Extend_PatternLifetimeForAuthoredLanes(KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, std::string &status);

// 공개 편집 명령과 UI가 stable Pattern ID로 문서의 행을 찾는다. 아래 const/가변 쌍은 모두 같은 vector를 선형 조회하며 부재 시 nullptr를 돌려준다.
KOUKU_SAYDON_COMPOSITION_PATTERN *Find_Pattern(
	KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const std::string_view patternId);

const KOUKU_SAYDON_COMPOSITION_PATTERN *Find_Pattern(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const std::string_view patternId);

// Animation 편집은 먼저 소유 Pattern 안에서 Stage ID를 찾는다. 반환 행은 그 Pattern의 Stages에 속하며 다른 Pattern의 같은 ID를 대신 찾지 않는다.
KOUKU_SAYDON_COMPOSITION_STAGE *Find_Stage(KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view stageId);

const KOUKU_SAYDON_COMPOSITION_STAGE *Find_Stage(
	const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view stageId);

// Animation은 Stage마다 나뉘어 저장된다. 모든 Stage의 자식 목록을 차례로 찾아 소유 Stage와 행을 함께 돌려주고, 부재 시 두 포인터가 빈 값을 돌려준다.
MUTABLE_OCCURRENCE Find_Occurrence(KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view occurrenceId);

// 읽기 경로도 같은 소유 Stage를 알 수 있게 선택적 ppStage 출력을 받는다. 찾은 경우에만 *ppStage를 쓰므로 nullptr 반환 시 호출자는 기존 출력 포인터를 성공값으로 해석하면 안 된다.
const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE *Find_Occurrence(const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern,
	const std::string_view occurrenceId, const KOUKU_SAYDON_COMPOSITION_STAGE **const ppStage = nullptr);

// 부모 안의 자식 배치 occurrence를 찾는다. 참조 대상 Pattern ID와 배치 ID는 다르며 같은 자식이 여러 시각에 놓일 수 있다.
const KOUKU_SAYDON_COMPOSITION_PATTERN_OCCURRENCE *Find_PatternBox(
	const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view id);

// Folder.strTimelinePatternId가 이 Pattern을 지정하는지 찾는다. 부모/자식 PatternOccurrences 그래프의 역방향 탐색과는 다른 소유 관계다.
const KOUKU_SAYDON_COMPOSITION_FOLDER *Find_TimelineParent(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const std::string_view patternId);

// 선택/삭제 검증이 Animation과 각 lane의 occurrence 존재를 묻는다. Stage 자체의 ID는 포함하지 않으므로 Find_Stage로 따로 확인한다.
bool Has_TimelineOccurrence(const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view id);

// Resources와 계층 UI의 표시 문자열을 고른다. 반환값은 정적 문자열이며 actorProfileId나 gateId의 저장 계약을 바꾸지 않는다.
const char_t *Actor_Label(const std::string_view actorProfileId, const std::string_view gateId = {});

// 자원 검색에서 바이트 단위 부분 문자열을 비교한다. 빈 검색어는 일치하며 Unicode 정규화나 다국어 대소문자 접기까지 수행하지 않는다.
bool_t ContainsInsensitive(const std::string_view text, const std::string_view query);

// 문서 순서대로 읽기 오류 없는 PRODUCT Pattern만 다시 모은다. 편집으로 DRAFT가 된 항목이 이전 Play All 목록에 남지 않게 한다.
void Rebuild_PlayAllPatternIds(KOUKU_SAYDON_COMPOSITION_DOCUMENT &document);

// 바뀐 Pattern을 참조하는 부모·Bundle까지 DRAFT 표시를 전파하고 Play All을 갱신한다. 방문한 ID를 집합에 넣어 같은 부모를 계속 추가하지 않는다.
void Mark_Draft(KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, KOUKU_SAYDON_COMPOSITION_PATTERN &pattern);

// 표시/이동 계산에 쓸 ms 길이다. 명시 iDurationMs가 있으면 우선하고 없으면 Stage 길이를 합산한다. 편집 10분 상한 검증은 Extend_PatternLifetimeForAuthoredLanes가 담당한다.
std::uint32_t Pattern_DurationMs(const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern);

// Resources의 Logic 정의 ID를 document.Logics에서 찾는다. 아래 Box 조회가 사용하는 실행 배치 ID와 섞지 않으며 반환 포인터는 문서 소유다.
const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION *Find_Logic(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const std::string_view logicId);

// Bingo 부모 UI가 strGateId가 BINGO인 Flow 행을 찾는다. 없으면 nullptr로 알리며 새 Flow를 생성하지 않는다.
const KOUKU_SAYDON_COMPOSITION_PATTERN_FLOW *Find_BingoFlow(const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document);

// Bingo 전용 제어 Pattern인지 lane 구성과 유일한 BINGO_BOARD Logic을 검사한다. 전투·다른 활성 Logic이 섞이면 전용 UI 대상으로 취급하지 않는다.
bool Is_BingoBoardControl(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern);

// Pattern.LogicOccurrences의 실행 창을 찾는 가변/const 쌍이다. 같은 정의를 쓰더라도 각 box의 시간·결과 연결은 별개다.
KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE *Find_LogicBox(
	KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view occurrenceId);

const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE *Find_LogicBox(
	const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view occurrenceId);

// 공유 소환 정책은 document.Summons의 정의 ID로 찾는다. 부재 시 nullptr이며 개별 Pattern의 소환 배치를 만들지 않는다.
const KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION *Find_Summon(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const std::string_view summonId);

// 소환의 실제 시작·수명을 편집할 때 Pattern.SummonOccurrences에서 배치 ID를 찾는다. 정의와 배치의 소유 container를 구분한다.
KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE *Find_SummonBox(
	KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view occurrenceId);

const KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE *Find_SummonBox(
	const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view occurrenceId);

// 외부 Motion을 참조하는 저작 정의를 document.Worlds에서 찾는다. 실제 World 객체 포인터를 찾아 소유권을 가져오는 함수가 아니다.
const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION *Find_World(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const std::string_view worldId);

// World 정의를 재생할 시간·배속·배치를 가진 Pattern.WorldOccurrences의 행을 차용한다. 없는 ID는 nullptr다.
KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE *Find_WorldBox(
	KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view occurrenceId);

const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE *Find_WorldBox(
	const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view occurrenceId);

// rendering profile ID를 참조하는 정의를 document.SceneProfiles에서 찾는다. 렌더링 품질 옵션 자체는 수정하지 않는다.
const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION *Find_SceneProfile(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const std::string_view sceneProfileId);

// Pattern.SceneProfileOccurrences의 적용 창·보간 시간을 편집할 행을 차용한다. document의 정의 ID와 occurrence ID를 구분한다.
KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE *Find_SceneProfileBox(
	KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view occurrenceId);

const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE *Find_SceneProfileBox(
	const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view occurrenceId);

// Resources/상세 UI의 종류 표시용 정적 문자열이다. 알려지지 않은 종류는 Unavailable로 표시하며 새로운 자원 정의를 만들지 않는다.
const char *Presentation_Label(const KOUKU_SAYDON_PRESENTATION_KIND kind);

// box가 참조하는 공유 원본을 document.PresentationResources에서 선형 조회한다. 부재 시 nullptr이므로 호출자는 자원 종류를 읽기 전에 확인한다.
const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE *Find_PresentationResource(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document, const std::string_view id);

// 기하/시간 편집이 Pattern.PresentationOccurrences의 배치 ID를 찾는다. 반환 행 주소는 그 vector의 변경이나 문서 교체를 넘겨 보관하지 않는다.
const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE *Find_PresentationBox(
	const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::string_view id);

// 이미 선택한 결과 slot을 UI 문자열로 바꾼다. slot 값의 유효성 검증이나 결과 연결 변경은 이 함수의 책임이 아니다.
const char_t *Outcome_SlotLabel(const KOUKU_SAYDON_OUTCOME_SLOT slot);

// ImGui의 고정 char 버퍼에 종료 문자까지 들어갈 때만 복사한다. destination이 없거나 용량이 부족하면 false이고 기존 버퍼를 유지한다.
bool_t Copy_Text(char_t *const destination, const std::size_t capacity, const std::string_view source);

// Copy/Move가 저장된 selection group을 함께 다루도록 ids에 누락 멤버를 추가한다. set으로 중복을 막고 새 ID는 Pattern 순서로 붙이며 문서 행은 바꾸지 않는다.
void Expand_PresentationSelection(const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, std::vector<std::string> &ids);

// 전체 선택·group·연결 Logic 창을 먼저 확인하고 모두에 적용할 공통 이동량(ms)을 Pattern 경계로 제한한다. 성공 시에만 outMove를 교체하며 실제 문서 변경/commit은 호출자 몫이다.
bool Prepare_PresentationTimelineMove(const KOUKU_SAYDON_COMPOSITION_DOCUMENT &document,
	const KOUKU_SAYDON_COMPOSITION_PATTERN &pattern, const std::vector<std::string> &selectedIds,
	const std::int64_t requestedDeltaMs, PRESENTATION_TIMELINE_MOVE &outMove, std::string &outStatus);

// 그룹 회전/이동을 같은 좌표 기준에서 처리할 수 있는지 anchor·bone·World 연결을 비교한다. owner를 따라가지 않으면 시작 시각도 같아야 한다.
bool Same_ColliderGroupFrame(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &a,
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &b);

// 위치(m)·회전(degree)·배율 및 선형 이동 끝의 유한값/범위를 검사한다. 값 검증만 수행하며 anchor 조합은 Valid_PresentationPlacement가 이어서 확인한다.
bool Valid_PresentationGeometry(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &value);

// Collider 종류에만 기하 검증을 적용하는 조건부 검사다. 다른 종류가 true라고 모든 배치 계약을 통과한 것으로 해석하지 않는다.
bool Valid_GameplayColliderScale(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE &resource,
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &value);

// staged 배치를 후보에 합칠 때 기하·이동 필드만 복사한다. effect=true일 때만 Effect 전용 anchor/follow/bone 값도 옮기며 시간과 stable ID는 유지한다.
void Copy_PresentationPlacement(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &source,
	KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &target, const bool effect);

// 위 복사 함수가 반영할 필드와 같은 집합을 비교해 배치 변경 여부를 정한다. 행 전체나 저장 문서의 동등성을 검사하는 함수는 아니다.
bool Same_PresentationPlacement(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &left,
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &right, const bool effect);

// 기하와 자원 종류에 따른 anchor·bone·follow 조합을 함께 검사한다. 잘못된 입력은 false이며 입력값 자체를 보정해 저장하지 않는다.
bool Valid_PresentationPlacement(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE &resource,
	const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE &value);

// Save/preview가 넘긴 후보의 root와 자식 Pattern에만 staged 배치를 합친다. 일부 행을 합친 뒤 실패할 수 있으므로 호출자는 실패한 후보를 버리고 원본 draft를 유지해야 한다.
bool Apply_PresentationPreviewPlacements(KOUKU_SAYDON_COMPOSITION_DOCUMENT &candidate,
	const std::vector<KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST> &edits, const std::vector<std::string> &patternIds,
	std::string &outStatus);

// 관문 ID를 계층 UI의 표시 문자열로 바꾼다. 반환 포인터는 정적 문자열이고 저장용 gate ID를 대체하지 않는다.
const char *Gate_Label(const std::string_view gate);

// Bundle 멤버 offset+Pattern 길이와 Scene/Presentation 끝 중 가장 늦은 ms를 구한다. 없는 Pattern은 건너뛰므로 반환 길이가 참조 유효성을 보증하지 않는다.
std::uint32_t Bundle_DurationMs(
	const KOUKU_SAYDON_COMPOSITION_DOCUMENT &doc, const KOUKU_SAYDON_COMPOSITION_BUNDLE &bundle);
} // Client::KoukuWorkbenchDetail 이름공간 끝

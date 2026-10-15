#pragma once

#include "KoukuSaydonActionWorkbench.h"
// 공개 H에서 이름만 선언한 private 중첩 타입을 여기서 정의한다. WORLD_CLIP_ROW는 KOUKU_WORLD_ANIMATION_EDIT 값을 포함한다.
#include "CompositionTimeline.h"
// LANE이 DISPLAY_LAYOUT을 값으로 소유하고 DISPLAY_INTERVAL 목록을 만들기 때문에 두 완전한 정의가 필요하다.
// 이 의존성에는 ImGui 도형 타입도 있어 공개 Workbench 헤더 대신 이 구현용 헤더에서 읽는다.

// 화면 레이아웃 캐시의 완전한 타입이다. 생성·소멸과 Timeline 구현에서만 필요하다.
// box의 실제 시각은 겹칠 수 있지만 화면 선택은 구분되어야 한다. Ensure_TimelineLayout이 구간을 모으고
// Allocate_TimelineDisplayRows가 화면 행을 배정한 결과만 캐시한다. 이 값은 JSON 저장이나 Server 실행 상태가 아니다.
struct Client::CKoukuSaydonActionWorkbench::TIMELINE_LAYOUT_CACHE
{
	// 패턴 ms 구간을 겹침 없는 화면 행으로 배치한 결과다. 문서 행 포인터는 저장하지 않는다.
	struct LANE
	{
		// 재구축 중 쌓은 Pattern 기준 ms 구간을 행 배정 함수에 move한다. 배정 뒤에는 layout의 ID→행 결과를 사용한다.
		std::vector<CompositionTimeline::DISPLAY_INTERVAL> intervals;
		CompositionTimeline::DISPLAY_LAYOUT layout;
		// firstRow는 전체 타임라인의 시작 행, rowCount는 이 lane이 차지할 행 수다. 저장된 occurrence 순번과 다르다.
		std::size_t firstRow = 0u, rowCount = 1u;
	};
	// World box ID와 원본 Animation slot ID를 함께 키로 삼아 서로 다른 actor/slot의 clip이 한 행으로 섞이지 않게 한다.
	struct WORLD_CLIP_LANE
	{
		std::string occurrenceId, slotId, label;
		std::vector<CompositionTimeline::DISPLAY_INTERVAL> intervals;
		// 여기의 firstRow는 전체 화면이 아니라 Animation lane 내부의 시작 행이다. 그릴 때 LANE.firstRow와 합친다.
		std::size_t firstRow = 0u;
	};
	// World 원본 slot을 편집 callback에 전달할 값이다. 각 속도는 world/clip 시계를 환산할 때 사용한다.
	struct WORLD_CLIP_ROW
	{
		std::string id, occurrenceId, label, actorLabel;
		// World box 시작 + 원본 표시 시각 / box 배속으로 얻은 Pattern 기준 ms다. 끝은 해당 box 수명으로 자른다.
		std::uint32_t startMs = 0u, durationMs = 0u;
		KOUKU_WORLD_ANIMATION_EDIT edit;
		// 모두 무차원 배속이다. localSpeed=instance×box는 드래그 ms를 원본 local start로,
		// nativeSpeed=원본 합성 배속×box는 표시 길이를 clip source 구간으로 바꾼다. trackRate는 local 시간과 source trim을 연결한다.
		double localSpeed = 1., nativeSpeed = 1., trackRate = 1.;
		// 비반복·유효 source 구간·양수 배속·외부 편집 callback이 모두 있어야 드래그 입력을 받는다.
		bool editable = false;
	};
	// 문서/World 세대뿐 아니라 줌에 따른 최소 폭과 callback/카메라 tail도 행 배치에 영향을 준다.
	// Ensure_TimelineLayout은 이 여섯 입력이 모두 같을 때만 행 배치를 재사용한다.
	// minimumBox는 최소 픽셀 폭을 현재 확대율로 나눈 ms이고, cameraTails는 별도 소유 카메라의 blend-out 목록이다.
	// 클래스 안 정의라 inline으로 공유되며 비교만 수행한다. false이면 호출자가 cache를 비우고 다시 구성한다.
	bool Matches(const std::uint64_t draft, const std::uint64_t worlds, const std::string &pattern,
		const std::uint64_t minimumBox, const bool editWorldAnimation,
		const std::vector<std::uint32_t> &cameraTails) const
	{
		return draftGeneration == draft && worldGeneration == worlds && patternId == pattern &&
			minimumBoxMs == minimumBox && canEditWorldAnimation == editWorldAnimation &&
			cameraBlendOutMs == cameraTails;
	}
	// 두 generation의 초기 최댓값은 세션의 초기 0 세대와 구분해 첫 재구성을 유도한다. 숫자 크기를 시간으로 해석하지 않는다.
	std::uint64_t draftGeneration = UINT64_MAX, worldGeneration = UINT64_MAX, minimumBoxMs = 0u;
	std::string patternId;
	std::vector<std::uint32_t> cameraBlendOutMs;
	bool canEditWorldAnimation = false, hasPatternLane = false, hasBossAnimations = false;
	// 0 Animation, 1 Logic, 2 Summon, 3 World, 4 Scene Profile, 5~10 Presentation 종류, 11 자식 Pattern의 고정 분류다.
	// 종류 수는 고정 배열로, 종류 안에서 늘어나는 실제 행과 World clip은 vector로 표현한다.
	std::array<LANE, 12u> lanes;
	std::vector<WORLD_CLIP_LANE> worldClipLanes;
	std::vector<WORLD_CLIP_ROW> worldClipRows;
	// 0번은 순차 Stage strip이므로 일반 box는 1번부터 놓는다. 최종 nextRow는 화면 높이와 확장 자식 행의 시작점이 된다.
	std::size_t nextRow = 1u;
};

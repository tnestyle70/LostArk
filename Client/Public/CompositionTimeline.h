#pragma once

// 이 파일은 저작 화면의 픽셀 기하·글자 크기·draw list를 다룬다. 보스 판정과 실제 재생 시계는 포함하지 않는다.
#include "imgui.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <string>
// stable occurrence/group ID에서 표시 행 또는 임시 group index를 찾는 파생 조회표다.
#include <unordered_map>
#include <utility>
// 가변 개수의 시간 구간과 행 끝 시각을 연속 저장해 정렬·순회한다. 반환 뒤 원본 문서는 변경되지 않는다.
#include <vector>

// 두 보스와 여러 저작 도구가 공유하는 화면 알고리즘 이름 공간이다. 클래스 instance나 문서 소유자를 추가하지 않는다.
namespace Client::CompositionTimeline
{

// 헤더 정의를 여러 CPP가 포함하므로 inline을 붙인다. constexpr는 픽셀 기본값이며 동적 font/DPI 계산을 대신하지 않는다.
inline constexpr float LaneHeight = 24.f;
inline constexpr float LabelWidth = 180.f;
inline constexpr float MinimumBoxWidth = 8.f;
inline constexpr float BoxHeight = 22.f;

struct ROW_METRICS
{
	float boxHeight, laneHeight, rulerHeight;
};

// 현재 font 크기에는 viewport DPI가 반영되어 있다.
// 모니터를 옮길 때 고정 행 높이에 DPI를 다시 곱하지 않고 현재 글자 높이에서 계산한다.
inline ROW_METRICS GetCompactRowMetrics()
{
	const auto& style = ImGui::GetStyle();
	const float textHeight = ImGui::GetTextLineHeight();
	const float boxHeight = std::ceil((std::max)(BoxHeight,
		textHeight + 2.f * (std::max)(1.f, style.FramePadding.y)));

	return {boxHeight,
		std::ceil((std::max)(LaneHeight, boxHeight + (std::max)(2.f, style.ItemSpacing.y))),
		std::ceil((std::max)(LaneHeight, textHeight + 6.f))};
}

// 각 label의 현재 font 너비를 재서 가장 긴 이름과 여백 16px를 수용한다. 결과는 timeline 왼쪽 이름표 폭(px)이다.
// const char* const*: labels는 문자열 포인터 배열을 차용한다. 첫 const는 문자 변경을, 두 번째 const는 labels[i] 재지정을 막는다.
// labels 변수 자체는 지역 포인터여서 이동 가능하다. count개 포인터는 유효해야 하며, 개별 null 항목은 건너뛴다. 배열 길이는 포인터에 없어서 count를 별도로 받는다.
inline float GetCompactLabelWidth(const char* const* labels, const std::size_t count)
{
	float width = LabelWidth;

	for (std::size_t i = 0u; i < count; ++i)
		if (labels[i]) width = (std::max)(width, ImGui::CalcTextSize(labels[i]).x + 16.f);

	return std::ceil(width);
}

// 시간 단위 ms를 1000으로 초 단위에 맞춰 px/초 배율을 구한다. 유효하지 않은 길이는 minimum을 반환하고 문서 시각은 수정하지 않는다.
inline float FitPixelsPerSecond(const float availableWidth, const double durationMs,
	const float minimum = 1.f, const float maximum = 500.f)
{
	if (!std::isfinite(availableWidth) || !std::isfinite(durationMs) || durationMs <= 0.)
		return minimum;

	return static_cast<float>(std::clamp(double(availableWidth) * 1000. / durationMs,
		double(minimum), double(maximum)));
}

inline void DrawTrackLabel(ImDrawList* draw, const ImVec2 origin, const float width,
	const float height, const char* label, const ImU32 color)
{
	if (!draw || !label || width <= 8.f || height <= 0.f) return;
	draw->PushClipRect(origin, ImVec2(origin.x + width - 4.f, origin.y + height), true);
	draw->AddText(ImVec2(origin.x + 4.f, origin.y + 4.f), color, label);
	draw->PopClipRect();
}

// category는 이름표 하나와 여러 표시 행의 기하 정보만 묶는다.
// stable ID, 시간 구간의 행 배치, 실제 편집 transaction은 호출한 저작 소유자가 담당한다.
struct CATEGORY_LANE_GEOMETRY
{
	ImVec2 origin, contentMin, contentMax;
	float rowHeight = 0.f;
	std::size_t rowCount = 0u;

	ImVec2 SubrowOrigin(const std::size_t row) const
	{
		return {contentMin.x, contentMin.y + rowHeight * static_cast<float>(row)};
	}
};

inline CATEGORY_LANE_GEOMETRY DrawCategoryLane(ImDrawList* draw,
	const ImVec2 origin, const float totalWidth, const float labelWidth,
	const std::size_t rowCount, const float rowHeight, const char* label,
	const ImU32 labelColor, const bool alternate = false,
	const float labelRightPadding = 0.f)
{
	const CATEGORY_LANE_GEOMETRY geometry{origin,
		{origin.x + labelWidth, origin.y},
		{origin.x + totalWidth, origin.y + rowHeight * static_cast<float>(rowCount)},
		rowHeight, rowCount};
	if (!draw || !rowCount || rowHeight <= 0.f || totalWidth <= labelWidth)
		return geometry;
	draw->AddRectFilled(origin, geometry.contentMax,
		alternate ? IM_COL32(31, 34, 41, 255) : IM_COL32(38, 41, 49, 255));
	draw->AddLine(origin, {geometry.contentMax.x, origin.y}, IM_COL32(64, 68, 76, 255));
	draw->AddLine(geometry.contentMin, {geometry.contentMin.x, geometry.contentMax.y},
		IM_COL32(64, 68, 76, 255));
	DrawTrackLabel(draw, origin, labelWidth - labelRightPadding, rowHeight, label, labelColor);
	return geometry;
}

// 직접 행 기하를 정하는 기존 편집기를 위해 기본 상수는 유지한다.
// font 크기를 반영하는 호출자는 그리기와 hit test에 같은 계산 결과를 사용한다.
inline float GetBoxHeight()
{
	const float padding = (std::max)(4.f, ImGui::GetStyle().FramePadding.y);
	return std::ceil((std::max)(32.f, ImGui::GetTextLineHeight() + padding * 2.f));
}

inline float GetLaneHeight()
{
	const float spacing = (std::max)(6.f, ImGui::GetStyle().ItemSpacing.y);
	return std::ceil((std::max)(38.f, GetBoxHeight() + spacing));
}

inline constexpr ImU32 StageColor = IM_COL32(96, 96, 112, 255);
inline constexpr ImU32 AnimationColor = IM_COL32(72, 128, 200, 255);
inline constexpr ImU32 LogicColor = IM_COL32(196, 118, 64, 255);
inline constexpr ImU32 SummonColor = IM_COL32(88, 156, 116, 255);
inline constexpr ImU32 WorldColor = IM_COL32(84, 132, 196, 255);
inline constexpr ImU32 SceneProfileColor = IM_COL32(156, 108, 196, 255);
inline constexpr ImU32 PresentationColor = IM_COL32(94, 165, 151, 255);
inline constexpr ImU32 StageLabelColor = IM_COL32_WHITE;
inline constexpr ImU32 AnimationLabelColor = IM_COL32(240, 188, 98, 255);
inline constexpr ImU32 LogicLabelColor = IM_COL32(236, 170, 110, 255);
inline constexpr ImU32 SummonLabelColor = IM_COL32(150, 220, 180, 255);
inline constexpr ImU32 WorldLabelColor = IM_COL32(150, 190, 240, 255);
inline constexpr ImU32 SceneProfileLabelColor = IM_COL32(200, 170, 240, 255);
inline constexpr ImU32 PresentationLabelColor = IM_COL32(180, 210, 200, 255);

// 표시 행 번호는 저장 ID가 아니다. 호출자가 actor/slot별 입력 목록을 나눠 전달한다.
struct DISPLAY_INTERVAL
{
	std::string occurrenceId;
	double startMs = 0., durationMs = 0.;
	double tailMs = 0.;
	std::string groupId;
};
struct DISPLAY_GROUP
{
	std::string groupId;
	double startMs = 0., endMs = 0.;
	std::size_t firstRow = 0u, rowCount = 0u;
};
// occurrenceRows는 표시할 행을 찾는 인덱스이고 groups는 group 기하를 순회할 값 목록이다. 둘 다 다시 만들 수 있는 화면 파생 값이다.
struct DISPLAY_LAYOUT
{
	std::unordered_map<std::string, std::size_t> occurrenceRows;
	std::vector<DISPLAY_GROUP> groups;
	std::size_t rowCount = 1u;
};

// intervals를 값으로 받아 함수가 소유한 vector를 정렬한다. lvalue는 복사하고, 현재 호출자처럼 std::move로 넘기면 버퍼 소유권을 옮긴다.
// 시작순 stable_sort 뒤 group 안의 첫 빈 행을 찾고, group 전체를 연속된 행 묶음에 배치한다. rowEnds는 각 행이 다시 비는 ms다.
// hash map은 ID를 찾는 용도이며 반복 순서에 의존하지 않는다. 행 탐색은 선형이므로 모든 입력에서 최적 시간·최소 행 수를 보장하는 알고리즘은 아니다.
inline DISPLAY_LAYOUT AllocateDisplayRows(
	std::vector<DISPLAY_INTERVAL> intervals, const double minimumBoxMs)
{
	// 저장된 group의 전체 시간 범위가 연속된 행 묶음을 예약한다.
	// group 내부에서 시간이 겹치는 항목은 서로 다른 hit-test 행에 배치하며 저작 시각은 바꾸지 않는다.
	std::stable_sort(intervals.begin(), intervals.end(),
		[](const auto& a, const auto& b) { return a.startMs < b.startMs; });
	struct UNIT
	{
		DISPLAY_GROUP span;
		std::vector<std::pair<std::string, std::size_t>> memberRows;
		std::vector<double> rowEnds;
	};
	std::vector<UNIT> units;
	std::unordered_map<std::string, std::size_t> groupUnits;
	for (const auto& interval : intervals)
	{
		std::size_t unitIndex = units.size();
		if (!interval.groupId.empty())
		{
			const auto [found, inserted] = groupUnits.emplace(interval.groupId, unitIndex);
			unitIndex = found->second;
		}
		if (unitIndex == units.size())
		{
			UNIT unit;
			unit.span.groupId = interval.groupId; unit.span.startMs = interval.startMs;
			units.push_back(std::move(unit));
		}
		auto& unit = units[unitIndex];
		const auto end = interval.startMs + (std::max)(interval.durationMs, minimumBoxMs) + interval.tailMs;
		std::size_t row = 0u;
		while (row < unit.rowEnds.size() && unit.rowEnds[row] > interval.startMs) ++row;
		if (row == unit.rowEnds.size()) unit.rowEnds.push_back(end); else unit.rowEnds[row] = end;
		unit.span.endMs = (std::max)(unit.span.endMs, end);
		unit.memberRows.emplace_back(interval.occurrenceId, row);
	}
	DISPLAY_LAYOUT layout;
	std::vector<double> rowEnds;
	for (auto& unit : units)
	{
		unit.span.rowCount = unit.rowEnds.size();
		std::size_t first = 0u;
		for (;; ++first)
		{
			bool available = true;
			for (std::size_t offset = 0u; offset < unit.span.rowCount && first + offset < rowEnds.size(); ++offset)
				if (rowEnds[first + offset] > unit.span.startMs) { available = false; break; }
			if (available) break;
		}
		rowEnds.resize((std::max)(rowEnds.size(), first + unit.span.rowCount), 0u);
		for (std::size_t offset = 0u; offset < unit.span.rowCount; ++offset) rowEnds[first + offset] = unit.span.endMs;
		for (const auto& [id, row] : unit.memberRows) layout.occurrenceRows.emplace(id, first + row);
		unit.span.firstRow = first;
		if (!unit.span.groupId.empty()) layout.groups.push_back(std::move(unit.span));
	}
	layout.rowCount = (std::max)(std::size_t{1u}, rowEnds.size());
	return layout;
}

// 드래그 사각 선택은 label ID가 아니라 실제 그린 box 사각형으로 검사한다. 짧은 point box도 포함한다.
inline bool MarqueeIntersectsBox(const ImVec2 anchor, const ImVec2 pointer,
    const ImVec2 boxMin, const ImVec2 boxMax)
{
    if (!std::isfinite(anchor.x) || !std::isfinite(anchor.y) ||
        !std::isfinite(pointer.x) || !std::isfinite(pointer.y) ||
        !std::isfinite(boxMin.x) || !std::isfinite(boxMin.y) ||
        !std::isfinite(boxMax.x) || !std::isfinite(boxMax.y) ||
        boxMax.x <= boxMin.x || boxMax.y <= boxMin.y) return false;
    return boxMax.x >= (std::min)(anchor.x, pointer.x) &&
        boxMin.x <= (std::max)(anchor.x, pointer.x) &&
        boxMax.y >= (std::min)(anchor.y, pointer.y) &&
        boxMin.y <= (std::max)(anchor.y, pointer.y);
}

enum class BoxGesture : std::uint8_t
{
	MOVE,
	TRIM_START,
	TRIM_END
};

// 행 hover·편집 권한·drag 시작 상태는 호출자가 소유한다.
// 짧은 box의 시간상 끝점도 잡을 수 있도록 box 바깥의 거리까지 검사한다.
// 양 끝 손잡이가 겹치면 가까운 쪽을, 거리가 같으면 끝 손잡이를 고른다.
inline BoxGesture HitBoxGesture(const float mouseX, const float startX,
	const float endX, const float edgeWidth, const bool allowStart,
	const bool allowEnd)
{
	if (!std::isfinite(mouseX) || !std::isfinite(startX) ||
		!std::isfinite(endX) || !std::isfinite(edgeWidth) ||
		endX < startX || edgeWidth < 0.f)
		return BoxGesture::MOVE;
	const float startDistance = std::abs(mouseX - startX);
	const float endDistance = std::abs(mouseX - endX);
	const bool start = allowStart && startDistance <= edgeWidth;
	const bool end = allowEnd && endDistance <= edgeWidth;
	if (end && (!start || endDistance <= startDistance))
		return BoxGesture::TRIM_END;
	return start ? BoxGesture::TRIM_START : BoxGesture::MOVE;
}

// 순차 playlist는 항목마다 독립적인 재생 시작 offset을 저장하지 않는다.
// 시작 trim은 delta에 따라 원본 시작점을 앞뒤로 옮기며 원본 끝점을 유지한다. 뒤 slot의 배치는 호출자가 적용한다.
// 끝 trim은 원본 끝점만 바꾼다. Stage 시간의 최종 적용은 호출자가 담당한다.
// wallDeltaMs는 화면에서 이동한 재생 시간(ms), playRate를 곱한 delta는 원본 clip 시간(ms)이다.
// 출력 참조는 모든 입력 검증 뒤에만 쓴다. false면 호출자가 가진 이전 시작/길이를 유지할 수 있다.
inline bool TrimSequentialSourceWindow(const std::uint32_t sourceStartMs,
    const std::uint32_t sourcePlayMs, const std::uint32_t nativeDurationMs,
    const double playRate, const bool startEdge, const std::int64_t wallDeltaMs,
    std::uint32_t& nextStartMs, std::uint32_t& nextPlayMs)
{
    if (!std::isfinite(playRate) || playRate < .01 || playRate > 16.0 ||
        !nativeDurationMs || nativeDurationMs > 600000u || sourceStartMs >= nativeDurationMs ||
        sourcePlayMs > nativeDurationMs - sourceStartMs) return false;
    const std::uint32_t sourceEnd = sourcePlayMs ? sourceStartMs + sourcePlayMs : nativeDurationMs;
    const auto delta = std::llround(static_cast<double>((std::clamp)(wallDeltaMs,
        std::int64_t{-600000}, std::int64_t{600000})) * playRate);
    const auto begin = startEdge ? (std::clamp)(std::int64_t(sourceStartMs) + delta,
        std::int64_t{0}, std::int64_t(sourceEnd) - 1) : std::int64_t(sourceStartMs);
    const auto end = startEdge ? std::int64_t(sourceEnd) : (std::clamp)(std::int64_t(sourceEnd) + delta,
        begin + 1, std::int64_t(nativeDurationMs));
    nextStartMs = static_cast<std::uint32_t>(begin);
    nextPlayMs = static_cast<std::uint32_t>(end - begin);
    return true;
}

inline void DrawRuler(ImDrawList* draw, const ImVec2 min, const ImVec2 max,
	const std::uint32_t durationMs, const float pxPerSecond)
{
	if (nullptr == draw || !std::isfinite(pxPerSecond) || pxPerSecond <= 0.f ||
		max.x <= min.x || max.y <= min.y)
		return;

	draw->PushClipRect(min, max, true);
	draw->AddRectFilled(min, max, IM_COL32(31, 34, 40, 255));
	const float visibleMinX = draw->GetClipRectMin().x;
	const float visibleMaxX = draw->GetClipRectMax().x;
	if (visibleMaxX > visibleMinX)
	{
		// 일반 zoom에서는 기존 발탄 눈금 간격을 유지한다.
		// 낮은 zoom에서는 눈금 간격을 넓혀 긴 시간 범위도 두 보스 편집기에서 읽을 수 있게 한다.
		std::uint64_t tickMs = pxPerSecond >= 180.f ? 500u : 1000u;
		while (tickMs < (std::numeric_limits<std::uint32_t>::max)() &&
			static_cast<double>(tickMs) * pxPerSecond * 0.001 < 64.0)
			tickMs *= 2u;
		const double scale = static_cast<double>(pxPerSecond) * 0.001;
		const double startMs = (std::clamp)(
			(static_cast<double>(visibleMinX) - min.x) / scale,
			0.0, static_cast<double>(durationMs));
		const double endMs = (std::clamp)(
			(static_cast<double>(visibleMaxX) - min.x) / scale,
			0.0, static_cast<double>(durationMs));
		const auto firstTick = static_cast<std::uint64_t>(startMs / tickMs);
		const auto lastTick = static_cast<std::uint64_t>(endMs / tickMs);
		for (std::uint64_t tick = firstTick; tick <= lastTick; ++tick)
		{
			const std::uint64_t clockMs = tick * tickMs;
			const float x = min.x + static_cast<float>(clockMs * scale);
			draw->AddLine(ImVec2(x, min.y), ImVec2(x, max.y),
				IM_COL32(91, 96, 108, 255));
			char label[32]{};
			std::snprintf(label, sizeof(label), "%u ms",
				static_cast<unsigned>(clockMs));
			draw->AddText(ImVec2(x + 3.f, min.y + 3.f),
				IM_COL32(200, 204, 212, 255), label);
		}
	}
	draw->PopClipRect();
}

// 기하와 종류별 색은 호출자가 전달한다. 이 함수는 hit item을 등록하거나
// 저작 데이터를 바꾸지 않으며 두 Workbench의 stable ID 소유권도 바꾸지 않는다.
inline void DrawBox(ImDrawList* draw, const ImVec2 min, const ImVec2 max,
	const ImU32 fill, const bool selected, const char* label,
	const bool leftGrip = true, const bool rightGrip = true)
{
	if (nullptr == draw || max.x <= min.x || max.y <= min.y)
		return;
	const auto clipMin = draw->GetClipRectMin(), clipMax = draw->GetClipRectMax();
	// 선택 테두리는 box 바깥까지 그려진다. hit item의 등록과 처리는 계속 호출자가 담당한다.
	if (max.x + 2.f < clipMin.x || min.x - 2.f > clipMax.x ||
		max.y + 2.f < clipMin.y || min.y - 2.f > clipMax.y) return;
	draw->AddRectFilled(min, max, fill, 3.f);
	if (selected)
		draw->AddRect(min, max, IM_COL32(255, 224, 92, 255), 3.f, 0, 2.f);

	const float gripInsetX = (std::min)(3.f, (max.x - min.x) * 0.25f);
	const float gripInsetY = (std::min)(3.f, (max.y - min.y) * 0.25f);
	if (leftGrip)
		draw->AddLine(ImVec2(min.x + gripInsetX, min.y + gripInsetY),
			ImVec2(min.x + gripInsetX, max.y - gripInsetY), IM_COL32_WHITE);
	if (rightGrip)
		draw->AddLine(ImVec2(max.x - gripInsetX, min.y + gripInsetY),
			ImVec2(max.x - gripInsetX, max.y - gripInsetY), IM_COL32_WHITE);

	const float textMinX = min.x + (leftGrip ? 7.f : 4.f);
	const float textMaxX = max.x - (rightGrip ? 6.f : 1.f);
	if (nullptr != label && textMaxX > textMinX)
	{
		const ImVec4 clip(textMinX, min.y, textMaxX, max.y);
		const float textY = min.y + (std::max)(1.f,
			(max.y - min.y - ImGui::GetTextLineHeight()) * 0.5f);
		draw->AddText(nullptr, 0.f, ImVec2(textMinX, textY),
			IM_COL32(247, 247, 249, 255), label, nullptr, 0.f, &clip);
	}
}

}

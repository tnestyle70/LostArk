#pragma once

#include "imgui.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Client::CompositionTimeline
{

inline constexpr float LaneHeight = 24.f;
inline constexpr float LabelWidth = 180.f;
inline constexpr float MinimumBoxWidth = 8.f;
inline constexpr float BoxHeight = 22.f;
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

// Display rows never become authored identities. Callers partition actor/slot lanes.
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
struct DISPLAY_LAYOUT
{
	std::unordered_map<std::string, std::size_t> occurrenceRows;
	std::vector<DISPLAY_GROUP> groups;
	std::size_t rowCount = 1u;
};

inline DISPLAY_LAYOUT AllocateDisplayRows(
	std::vector<DISPLAY_INTERVAL> intervals, const double minimumBoxMs)
{
	// A saved group reserves one contiguous row block for its complete span.
	// Internal overlaps still get separate hit-test rows; no authored clock changes.
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

enum class BoxGesture : std::uint8_t
{
	MOVE,
	TRIM_START,
	TRIM_END
};

// The caller owns row hovering, edit permissions, and the captured drag state.
// Distances may extend outside the box so a short semantic endpoint is still
// reachable. When the two grips overlap, the nearest wins; ties choose the end.
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

// A sequential playlist has no independently saved wall-clock offset. Left
// trim advances its source start and ripples later slots; the source end stays
// fixed. Right trim moves only the source end. Callers retain Stage authority.
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
		// Preserve the original Valtan ruler at normal zoom. Wider intervals at
		// low zoom keep a ten-minute canvas readable in either boss editor.
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

// Geometry and family colors come from the caller. This draws no hit item and
// makes no authoring mutation; both workbenches retain their own stable IDs.
inline void DrawBox(ImDrawList* draw, const ImVec2 min, const ImVec2 max,
	const ImU32 fill, const bool selected, const char* label,
	const bool leftGrip = true, const bool rightGrip = true)
{
	if (nullptr == draw || max.x <= min.x || max.y <= min.y)
		return;
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

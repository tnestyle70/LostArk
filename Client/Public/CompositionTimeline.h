#pragma once

#include "imgui.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>

namespace Client::CompositionTimeline
{

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

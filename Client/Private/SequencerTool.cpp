#include "imgui.h"

#include "SequencerTool.h"
#include "BossTool.h"

#include <algorithm>
#include <array>

namespace
{
	constexpr float LANE_LABEL_WIDTH = 96.f;
	constexpr float ROW_HEIGHT = 22.f;
	constexpr float RULER_HEIGHT = 18.f;
	using LANE = Client::CActionCompositionWorkbench::TIMELINE_LANE;
	constexpr std::array<LANE, 7u> LANE_ORDER = {
		LANE::STAGE, LANE::ANIMATION, LANE::COLLIDER, LANE::EFFECT,
		LANE::SOUND, LANE::LOGIC, LANE::CAMERA,
	};
}

Client::CSequencerTool::CSequencerTool(
	CActionCompositionWorkbench* pWorkbench,
	CBossTool* pBossTool)
	: m_pWorkbench(pWorkbench)
	, m_pBossTool(pBossTool)
{
}

const char_t* Client::CSequencerTool::Lane_Label(const LANE eLane)
{
	switch (eLane)
	{
	case LANE::STAGE: return "Stage";
	case LANE::ANIMATION: return "Animation";
	case LANE::EFFECT: return "Effect";
	case LANE::SOUND: return "Sound";
	case LANE::LOGIC: return "Logic";
	case LANE::COLLIDER: return "Collider";
	case LANE::CAMERA: return "Camera";
	default: return "?";
	}
}

uint32_t Client::CSequencerTool::Lane_Color(const LANE eLane)
{
	switch (eLane)
	{
	case LANE::STAGE: return IM_COL32(96, 96, 112, 255);
	case LANE::ANIMATION: return IM_COL32(72, 128, 200, 255);
	case LANE::EFFECT: return IM_COL32(196, 120, 64, 255);
	case LANE::SOUND: return IM_COL32(120, 168, 72, 255);
	case LANE::LOGIC: return IM_COL32(168, 96, 176, 255);
	case LANE::COLLIDER: return IM_COL32(76, 176, 184, 255);
	case LANE::CAMERA: return IM_COL32(184, 160, 72, 255);
	default: return IM_COL32(128, 128, 128, 255);
	}
}

bool_t Client::CSequencerTool::Consume_CompositionOpenRequest()
{
	const bool_t bRequested = m_bCompositionOpenRequested;
	m_bCompositionOpenRequested = false;
	return bRequested;
}

void Client::CSequencerTool::Render()
{
	if (!m_bOpen)
		return;
	ImGui::SetNextWindowSize(ImVec2(1100.f, 520.f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Sequencer###LostArkSequencerV0", &m_bOpen))
	{
		ImGui::End();
		return;
	}
	if (nullptr == m_pWorkbench)
	{
		ImGui::TextUnformatted("Action Composition Workbench is unavailable.");
		ImGui::End();
		return;
	}
	/* Sibling view: the Workbench owns the canonical load, selection, drafts
	   and the timeline cache; this window only chooses which Pattern that
	   shared session shows and never writes a second document. */
	std::string SessionStatus;
	if (!m_pWorkbench->Ensure_CanonicalLoaded(SessionStatus))
	{
		ImGui::TextWrapped("Composition session is unavailable: %s",
			SessionStatus.c_str());
		ImGui::End();
		return;
	}
	const std::vector<std::string> PatternIds = m_pWorkbench->Get_PatternIds();
	const std::string strPatternId = m_pWorkbench->Get_SelectedPatternId();
	ImGui::SetNextItemWidth(360.f);
	if (ImGui::BeginCombo("Pattern##SequencerPattern",
			strPatternId.empty() ? "(select a Pattern)" : strPatternId.c_str()))
	{
		for (const std::string& strCandidate : PatternIds)
		{
			const bool_t bSelected = strCandidate == strPatternId;
			if (ImGui::Selectable(strCandidate.c_str(), bSelected) &&
				!m_pWorkbench->Select_PatternById(strCandidate, SessionStatus))
			{
				m_strStatus = SessionStatus;
			}
			if (bSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("Open in Composition"))
		m_bCompositionOpenRequested = true;
	if (!m_strStatus.empty())
		ImGui::TextWrapped("%s", m_strStatus.c_str());
	if (strPatternId.empty())
	{
		ImGui::TextDisabled("Choose a Pattern to lay out its lanes.");
		ImGui::End();
		return;
	}
	if (!m_pWorkbench->Ensure_SelectedTimeline(SessionStatus))
	{
		ImGui::TextWrapped("%s", SessionStatus.c_str());
		ImGui::End();
		return;
	}
	const std::vector<CActionCompositionWorkbench::TIMELINE_ITEM>& Items =
		m_pWorkbench->Get_TimelineItems();
	const uint32_t iDurationMs = m_pWorkbench->Get_TimelineDurationMs();
	if (Items.empty())
	{
		ImGui::TextDisabled("The selected Pattern has no timeline items.");
		ImGui::End();
		return;
	}
	Render_Transport(iDurationMs);
	Render_Lanes(Items, iDurationMs);
	Render_Selection(Items);
	ImGui::End();
}

void Client::CSequencerTool::Render_Transport(const uint32_t iDurationMs)
{
	int32_t iPlayheadMs = static_cast<int32_t>((std::min)(
		m_pWorkbench->Get_PlayheadMs(), static_cast<uint32_t>(INT32_MAX)));
	ImGui::SetNextItemWidth(-220.f);
	if (ImGui::SliderInt("##SequencerPlayhead", &iPlayheadMs, 0,
			static_cast<int32_t>((std::min)(iDurationMs,
				static_cast<uint32_t>(INT32_MAX))), "%d ms",
			ImGuiSliderFlags_AlwaysClamp))
	{
		(void)m_pWorkbench->Set_PlayheadMs(
			static_cast<uint32_t>((std::max)(iPlayheadMs, 0)));
	}
	ImGui::SameLine();
	ImGui::Text("%u / %u ms", m_pWorkbench->Get_PlayheadMs(), iDurationMs);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(90.f);
	ImGui::DragFloat("px/s", &m_fPixelsPerSecond, 2.f, 20.f, 1200.f, "%.0f",
		ImGuiSliderFlags_AlwaysClamp);
}

void Client::CSequencerTool::Render_Lanes(
	const std::vector<CActionCompositionWorkbench::TIMELINE_ITEM>& Items,
	const uint32_t iDurationMs)
{
	std::array<std::size_t, static_cast<std::size_t>(LANE::COUNT)> SubrowCounts{};
	for (const CActionCompositionWorkbench::TIMELINE_ITEM& Item : Items)
	{
		std::size_t& iCount = SubrowCounts[static_cast<std::size_t>(Item.eLane)];
		iCount = (std::max)(iCount, Item.iSubrow + 1u);
	}
	const float fCanvasWidth = (std::max)(
		200.f, static_cast<float>(iDurationMs) * m_fPixelsPerSecond * 0.001f);
	float fTotalHeight = RULER_HEIGHT;
	for (const LANE eLane : LANE_ORDER)
	{
		fTotalHeight += ROW_HEIGHT *
			static_cast<float>((std::max)(
				SubrowCounts[static_cast<std::size_t>(eLane)], std::size_t{ 1u }));
	}
	if (!ImGui::BeginChild("##SequencerLanes",
			ImVec2(0.f, (std::min)(fTotalHeight + 24.f, 340.f)), true,
			ImGuiWindowFlags_HorizontalScrollbar))
	{
		ImGui::EndChild();
		return;
	}
	ImDrawList* const pDraw = ImGui::GetWindowDrawList();
	const ImVec2 Origin = ImGui::GetCursorScreenPos();
	ImGui::Dummy(ImVec2(LANE_LABEL_WIDTH + fCanvasWidth, fTotalHeight));
	const float fCanvasX = Origin.x + LANE_LABEL_WIDTH;

	/* Ruler: one tick per second, half-second minor ticks. */
	pDraw->AddRectFilled(ImVec2(fCanvasX, Origin.y),
		ImVec2(fCanvasX + fCanvasWidth, Origin.y + RULER_HEIGHT),
		IM_COL32(40, 40, 48, 255));
	for (uint32_t iMs = 0u; iMs <= iDurationMs; iMs += 500u)
	{
		const float fX = fCanvasX +
			static_cast<float>(iMs) * m_fPixelsPerSecond * 0.001f;
		const bool_t bMajor = 0u == iMs % 1000u;
		pDraw->AddLine(ImVec2(fX, Origin.y + (bMajor ? 4.f : 10.f)),
			ImVec2(fX, Origin.y + RULER_HEIGHT), IM_COL32(150, 150, 160, 255));
		if (bMajor)
		{
			const std::string Label = std::to_string(iMs / 1000u) + "s";
			pDraw->AddText(ImVec2(fX + 2.f, Origin.y), IM_COL32(200, 200, 210, 255),
				Label.c_str());
		}
	}

	float fLaneY = Origin.y + RULER_HEIGHT;
	for (const LANE eLane : LANE_ORDER)
	{
		const std::size_t iSubrows = (std::max)(
			SubrowCounts[static_cast<std::size_t>(eLane)], std::size_t{ 1u });
		const float fLaneHeight = ROW_HEIGHT * static_cast<float>(iSubrows);
		pDraw->AddRectFilled(ImVec2(Origin.x, fLaneY),
			ImVec2(fCanvasX + fCanvasWidth, fLaneY + fLaneHeight),
			(static_cast<int32_t>(eLane) & 1) ?
				IM_COL32(30, 30, 36, 255) : IM_COL32(36, 36, 42, 255));
		pDraw->AddText(ImVec2(Origin.x + 4.f, fLaneY + 4.f),
			IM_COL32(220, 220, 230, 255), Lane_Label(eLane));
		for (std::size_t iItem = 0u; iItem < Items.size(); ++iItem)
		{
			const CActionCompositionWorkbench::TIMELINE_ITEM& Item = Items[iItem];
			if (Item.eLane != eLane)
				continue;
			const float fX0 = fCanvasX +
				static_cast<float>(Item.iStartMs) * m_fPixelsPerSecond * 0.001f;
			const float fX1 = (std::max)(fX0 + 4.f, fCanvasX +
				static_cast<float>(Item.iEndMs) * m_fPixelsPerSecond * 0.001f);
			const float fY0 = fLaneY + ROW_HEIGHT * static_cast<float>(Item.iSubrow) + 2.f;
			const float fY1 = fY0 + ROW_HEIGHT - 4.f;
			const bool_t bSelected = Item.strStableId == m_strSelectedStableId &&
				Item.strStageId == m_strSelectedStageId && Item.eLane == m_eSelectedLane;
			pDraw->AddRectFilled(ImVec2(fX0, fY0), ImVec2(fX1, fY1),
				Lane_Color(eLane), 3.f);
			if (bSelected)
				pDraw->AddRect(ImVec2(fX0, fY0), ImVec2(fX1, fY1),
					IM_COL32(255, 255, 255, 255), 3.f, 0, 2.f);
			pDraw->PushClipRect(ImVec2(fX0, fY0), ImVec2(fX1, fY1), true);
			pDraw->AddText(ImVec2(fX0 + 3.f, fY0 + 2.f),
				IM_COL32(255, 255, 255, 255), Item.strLabel.c_str());
			pDraw->PopClipRect();
			ImGui::SetCursorScreenPos(ImVec2(fX0, fY0));
			ImGui::PushID(static_cast<int32_t>(iItem));
			if (ImGui::InvisibleButton("##SequencerItem",
					ImVec2(fX1 - fX0, fY1 - fY0)))
			{
				m_strSelectedStableId = Item.strStableId;
				m_strSelectedStageId = Item.strStageId;
				m_eSelectedLane = Item.eLane;
				(void)m_pWorkbench->Set_PlayheadMs(Item.iStartMs);
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s\n%s / %s\n%u .. %u ms",
					Item.strLabel.c_str(), Item.strStageId.c_str(),
					Item.strStableId.c_str(), Item.iStartMs, Item.iEndMs);
			}
			ImGui::PopID();
		}
		fLaneY += fLaneHeight;
	}

	/* Playhead over every lane. */
	const float fPlayheadX = fCanvasX +
		static_cast<float>(m_pWorkbench->Get_PlayheadMs()) *
			m_fPixelsPerSecond * 0.001f;
	pDraw->AddLine(ImVec2(fPlayheadX, Origin.y),
		ImVec2(fPlayheadX, Origin.y + fTotalHeight),
		IM_COL32(255, 80, 80, 255), 2.f);
	ImGui::EndChild();
}

void Client::CSequencerTool::Render_Selection(
	const std::vector<CActionCompositionWorkbench::TIMELINE_ITEM>& Items) const
{
	const auto Found = std::find_if(Items.begin(), Items.end(),
		[this](const CActionCompositionWorkbench::TIMELINE_ITEM& Item)
		{
			return Item.strStableId == m_strSelectedStableId &&
				Item.strStageId == m_strSelectedStageId &&
				Item.eLane == m_eSelectedLane;
		});
	if (Items.end() == Found)
	{
		ImGui::TextDisabled("Click a box to inspect it. Editing stays in the owner window (Composition, Camera Tool, Effect Tool).");
		return;
	}
	ImGui::SeparatorText("Selected");
	ImGui::Text("%s | %s", Lane_Label(Found->eLane), Found->strLabel.c_str());
	ImGui::Text("Stage %s | id %s", Found->strStageId.c_str(),
		Found->strStableId.c_str());
	if (!Found->strAssetId.empty())
		ImGui::Text("Asset %s", Found->strAssetId.c_str());
	ImGui::Text("%u .. %u ms (%u ms)", Found->iStartMs, Found->iEndMs,
		Found->iEndMs >= Found->iStartMs ? Found->iEndMs - Found->iStartMs : 0u);
	ImGui::Text("%s", Found->bEditable ? "editable in Composition" : "read-only");
}

#pragma once

#include "Client_Defines.h"
#include "ActionCompositionWorkbench.h"

#include <string>
#include <vector>

NS_BEGIN(Client)

class CBossTool;

/* Sequencer v0: one playhead over every lane of the Composition Workbench's
   selected Pattern (Stage, Animation, Collider, Effect, Sound, Logic, Camera).
   It owns no document and no draft. Reading goes through the Workbench's
   read-only timeline projection, seeking goes through the Workbench playhead,
   and editing deep-links back to the owner windows. v1 adds its own
   Data/Sequences document with actor/camera/world tracks. */
class CSequencerTool final
{
public:
	CSequencerTool(
		CActionCompositionWorkbench* pWorkbench,
		CBossTool* pBossTool);

	void Open() { m_bOpen = true; }
	[[nodiscard]] bool_t Is_Open() const noexcept { return m_bOpen; }
	void Render();
	bool_t Consume_CompositionOpenRequest();

private:
	static const char_t* Lane_Label(CActionCompositionWorkbench::TIMELINE_LANE eLane);
	static uint32_t Lane_Color(CActionCompositionWorkbench::TIMELINE_LANE eLane);
	void Render_Transport(uint32_t iDurationMs);
	void Render_Lanes(
		const std::vector<CActionCompositionWorkbench::TIMELINE_ITEM>& Items,
		uint32_t iDurationMs);
	void Render_Selection(
		const std::vector<CActionCompositionWorkbench::TIMELINE_ITEM>& Items) const;

private:
	CActionCompositionWorkbench* m_pWorkbench = nullptr;
	CBossTool* m_pBossTool = nullptr;
	bool_t m_bOpen = true;
	bool_t m_bCompositionOpenRequested = false;
	std::string m_strStatus;
	float m_fPixelsPerSecond = 160.f;
	std::string m_strSelectedStableId;
	std::string m_strSelectedStageId;
	CActionCompositionWorkbench::TIMELINE_LANE m_eSelectedLane =
		CActionCompositionWorkbench::TIMELINE_LANE::STAGE;
};

NS_END

#pragma once

#include "Client_Defines.h"
#include "ValtanActionWorkbench.h"
#include "BossCompositionDocument.h"

#include <cstdint>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CValtanBossTool;

/* Sequencer: one playhead over every lane of the Composition Workbench's
   selected Pattern (Stage, Animation, Collider, Effect, Sound, Logic, Camera).
   Valtan editing still goes through that single admitted Workbench session;
   the Boss Composition and Arena Sequencer documents are a read-only
   authoring facade over the same typed owners, not a second runtime/writer. */
class CSequencerTool final
{
public:
	CSequencerTool(
		CValtanActionWorkbench* pValtanWorkbench,
		CValtanBossTool* pValtanBossTool);

	void Open() { m_bOpen = true; }
	[[nodiscard]] bool_t Is_Open() const noexcept { return m_bOpen; }
	void Render();
	bool_t Consume_CompositionOpenRequest();
	bool_t Consume_KoukuSaydonAnimationOpenRequest(std::string& outProfileId);

private:
	enum class SOURCE_BOSS : uint8_t
	{
		VALTAN,
		KOUKU_SAYDON,
	};

	static const char_t* Lane_Label(CValtanActionWorkbench::TIMELINE_LANE eLane);
	static uint32_t Lane_Color(CValtanActionWorkbench::TIMELINE_LANE eLane);
	static const char_t* SourceBoss_Label(SOURCE_BOSS boss);
	void Reload_SourceDocuments();
	void Synchronize_SourceDocumentsWithCanonicalGeneration();
	void Render_SourceDocumentHeader();
	void Render_AdvancedSourceInspector();
	void Render_SourcePatternSummary();
	void Render_ArenaSequencerSummary() const;
	void Render_Transport(uint32_t iDurationMs);
	void Render_Lanes(
		const std::vector<CValtanActionWorkbench::TIMELINE_ITEM>& Items,
		uint32_t iDurationMs);
	void Render_Selection(
		const std::vector<CValtanActionWorkbench::TIMELINE_ITEM>& Items) const;

private:
	CValtanActionWorkbench* m_pValtanWorkbench = nullptr;
	CValtanBossTool* m_pValtanBossTool = nullptr;
	bool_t m_bOpen = true;
	bool_t m_bCompositionOpenRequested = false;
	std::string m_strKoukuSaydonAnimationOpenProfileId;
	bool_t m_bSourceDocumentsLoaded = false;
	bool_t m_bSourceDocumentsParsed = false;
	SOURCE_BOSS m_eSourceBoss = SOURCE_BOSS::VALTAN;
	std::uint64_t m_iObservedCanonicalDisplayGeneration =
		~std::uint64_t{ 0u };
	CCompositionDocumentCatalog m_SourceCatalog;
	CBossCompositionDocument m_BossComposition;
	CArenaSequencerDocument m_ArenaSequencer;
	std::string m_strSourceDocumentStatus;
	std::string m_strSourcePatternId;
	std::string m_strStatus;
	float m_fPixelsPerSecond = 160.f;
	std::string m_strSelectedStableId;
	std::string m_strSelectedStageId;
	CValtanActionWorkbench::TIMELINE_LANE m_eSelectedLane =
		CValtanActionWorkbench::TIMELINE_LANE::STAGE;
};

NS_END

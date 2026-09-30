#pragma once

#include "Client_Defines.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "MapPlacementRuntime.h"
#include "Network/PacketMessages.h"
#include "ProjectDataRoot.h"
#include "UILabelFont.h"
#include "UILayoutRuntime.h"
#include "UITextOcclusion.h"
#include "WorldPlayerNameplateView.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

NS_BEGIN(Client)

/* The Colosseum "match starts" beat that follows the intro cutscene, restored from the retail
   in-match HUD (EFUI_COLOSSEUM colosseumplaying_loc_int) and the retail arena
   (Data/Camera/ColosseumMatchStart.json):

     - a top banner "Colosseum / N seconds left until the match" with a draining amber bar,
     - the red announce "N seconds until the match starts" (3, 2, 1) on its blood splash,
     - "Match start!" on the same splash when the count reaches zero,
     - the wooden gate leaves (ARENADOOR01) in front of both holding pens sinking into the floor
       a moment after the start text.

	Presentation only: the Server match state gates movement/combat and supplies this clock. The
   gate leaves are ordinary map placements; their pose is written through
   CMapPlacementRuntime::Apply_PlacementTransform (the same path a sequence or the Map Editor
   uses), and the placed pose, the closed gate, is restored by Reset(). A missing or invalid
   document only skips the beat. */
class CColosseumMatchStart final
{
public:
	CColosseumMatchStart() = default;
	CColosseumMatchStart(const CColosseumMatchStart&) = delete;
	CColosseumMatchStart& operator=(const CColosseumMatchStart&) = delete;
	~CColosseumMatchStart() { Release(); }

	bool_t Initialize(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
		const uint32_t iLevelIndex, CMapPlacementRuntime* pMap)
	{
		m_pDevice = std::move(pDevice);
		m_pContext = std::move(pContext);
		m_iLevelIndex = iLevelIndex;
		m_pMap = pMap;
		return Load_All();
	}

	/* F1 replay: re-reads the document, closes the gate and starts the count from 3. */
	bool_t Restart()
	{
		Reset();
		if (!Load_All())
			return false;
		Begin();
		return true;
	}

	/* Hides the banner and the announce and puts the gate leaves back at the placed (closed) pose. */
	void Reset()
	{
		m_bRunning = false;
		m_bPaused = false;
		m_fClockMs = 0.f;
		Show_View(false, false);
		Apply_Gate(0.f);
	}

	/* Starts the count now. The first frame after Begin adds nothing to the clock. */
	void Begin()
	{
		if (!m_bReady)
			return;
		Reset();
		m_bRunning = true;
		m_bSkipStep = true;
	}

    // Server already admitted combat; show its start beat without inventing a second countdown.
    void Begin_ApprovedMatch()
    {
        Begin();
        Seek(m_Doc.fTotalMs);
    }

	void Set_Paused(const bool_t bPaused) { if (m_bRunning) m_bPaused = bPaused; }
	void Seek(const f32_t fMs)
	{
		if (m_bRunning && std::isfinite(fMs))
			m_fClockMs = std::clamp(fMs, 0.f, Get_TimelineMs());
	}
	bool_t Is_Ready() const { return m_bReady; }
	bool_t Is_Running() const { return m_bRunning; }
	bool_t Is_Paused() const { return m_bPaused; }
	f32_t Get_ClockMs() const { return m_fClockMs; }
	f32_t Get_TimelineMs() const
	{
		return (std::max)(m_Doc.fTotalMs + m_Doc.fStartHoldMs + m_Doc.fStartFadeMs,
			m_Doc.fTotalMs + m_Doc.fGateDelayMs + m_Doc.fGateDurationMs);
	}
	size_t Get_GateCount() const { return m_Gates.size(); }
	f32_t Get_GateSinkMeters() const { return m_fAppliedSink; }
	const std::string& Get_Status() const { return m_strStatus; }
	const char* Get_PhaseLabel() const
	{
		if (!m_bReady)
			return "not loaded";
		if (!m_bRunning)
			return "waiting for the intro";
		if (m_bPaused)
			return "paused";
		if (m_fClockMs < m_Doc.fTotalMs)
			return "counting down";
		return m_fClockMs < Get_TimelineMs() ? "match start" : "finished";
	}
	/* Whole seconds still to count, 3 -> 1 during the count and 0 afterwards. */
	uint32_t Get_CountDigit() const
	{
		if (m_fClockMs >= m_Doc.fTotalMs)
			return 0u;
		return static_cast<uint32_t>(std::ceil((m_Doc.fTotalMs - m_fClockMs) / 1000.f));
	}

	void Update_ServerTimeline(const LostArk::Shared::S2C_COLOSSEUM_MATCH_STATE& state, const double tick)
	{
		using P = LostArk::Shared::COLOSSEUM_MATCH_PHASE;
		if (!m_bReady) return;
		if (state.ePhase == P::LOADING || state.ePhase == P::RECRUITING ||
			state.ePhase == P::ENTRY_COUNTDOWN || state.ePhase == P::INTRO)
		{
			if (m_bRunning) Reset();
			return;
		}
		if (state.ePhase != P::COUNTDOWN && state.ePhase != P::PLAYING && state.ePhase != P::FINISHED) return;
		m_bRunning = true;
		m_bPaused = false;
		m_bSkipStep = true;
		m_bServerClock = true;
		if (state.ePhase == P::COUNTDOWN)
		{
			m_Doc.fTotalMs = static_cast<f32_t>(state.iPhaseEndTick - state.iPhaseStartTick) * (1000.f/30.f);
			m_fClockMs = static_cast<f32_t>((std::max)(0.0, tick-state.iPhaseStartTick)*(1000.0/30.0));
		}
		else if (state.ePhase == P::PLAYING)
			m_fClockMs = m_Doc.fTotalMs + static_cast<f32_t>((std::max)(0.0,tick-state.iPhaseStartTick)*(1000.0/30.0));
		else m_fClockMs = Get_TimelineMs();
		m_fClockMs = (std::min)(m_fClockMs,Get_TimelineMs());
		Update(0.f);
	}

	void Update(const f32_t fTimeDelta)
	{
		if (!m_bReady || !m_bRunning)
			return;
		/* A level-entry hitch is load time, not match time. */
		if (m_bSkipStep)
			m_bSkipStep = false;
		else if (!m_bPaused && std::isfinite(fTimeDelta) && fTimeDelta > 0.f)
			m_fClockMs = (std::min)(m_fClockMs + (std::min)(fTimeDelta * 1000.f, MAX_STEP_MS),
				Get_TimelineMs());

		const bool_t bBanner = m_fClockMs < m_Doc.fTotalMs;
		const f32_t fAnnounceEnd = m_Doc.fTotalMs + m_Doc.fStartHoldMs + m_Doc.fStartFadeMs;
		Show_View(bBanner, m_fClockMs < fAnnounceEnd);
		Place_Slots();
		if (bBanner)
			Update_Bar();
		Update_Announce_Alpha();

		const f32_t fSink = Compute_GateSink();
		if (std::abs(fSink - m_fAppliedSink) > 0.0001f)
			Apply_Gate(fSink);
	}

	void Render()
	{
		if (!m_bReady || !m_bRunning)
			return;
		const f32_t fAnnounceEnd = m_Doc.fTotalMs + m_Doc.fStartHoldMs + m_Doc.fStartFadeMs;
		const bool_t bBanner = m_fClockMs < m_Doc.fTotalMs;
		if (!bBanner && m_fClockMs >= fAnnounceEnd)
			return;
		CGameInstance& gameInstance = CGameInstance::Get();
		const float2_t vViewport = gameInstance.Get_ViewportSize();
		if (vViewport.x <= 0.f || vViewport.y <= 0.f)
			return;
		const f32_t fScale = vViewport.y / STAGE_H;
		const f32_t fCenterX = vViewport.x * 0.5f;
		Refit_Fonts(gameInstance, vViewport.y);

		CUITextLayerScope textScope(UI_TEXT_LAYER::HUD);
		if (bBanner)
		{
			const std::wstring strDigits = Two_Digits(Get_CountDigit());
			Draw_Line(gameInstance, m_Doc.strTitle, fCenterX, m_Doc.fTitleCenterY * fScale, m_fTitlePx,
				m_Doc.Title, m_Doc.Shadow, 1.f, fScale, 0.f);
			Draw_Line(gameInstance, Fill_Number(m_Doc.strRemainLine, strDigits), fCenterX,
				m_Doc.fRemainCenterY * fScale, m_fRemainPx, m_Doc.Remain, m_Doc.Shadow, 1.f, fScale, 0.f);
		}
		const f32_t fAlpha = Announce_Alpha();
		if (fAlpha > 0.001f)
		{
			const std::wstring strLine = bBanner
				? Fill_Number(m_Doc.strAnnounceLine, std::to_wstring(Get_CountDigit()))
				: m_Doc.strStartText;
			Draw_Line(gameInstance, strLine, fCenterX, m_Doc.fAnnounceCenterY * fScale, m_fAnnouncePx,
				m_Doc.Announce, m_Doc.AnnounceGlow, fAlpha, fScale, 1.6f);
		}
	}

private:
	enum class CURVE { LINEAR, OUT_CUBIC, IN_CUBIC, SMOOTH };
	struct BAR_KEY { f32_t fRemainingMs = 0.f, fFraction = 0.f; };
	struct STAGE_RECT { f32_t fDx = 0.f, fY = 0.f, fW = 0.f, fH = 0.f; };
	struct DOCUMENT
	{
		f32_t fTotalMs = 3000.f, fStartHoldMs = 0.f, fStartFadeMs = 0.f;
		std::wstring strTitle, strRemainLine, strAnnounceLine, strStartText;
		std::vector<BAR_KEY> Bar;
		f32_t fTitleCenterY = 0.f, fRemainCenterY = 0.f, fAnnounceCenterY = 0.f;
		f32_t fTitleWidth = 1.f, fRemainWidth = 1.f, fAnnounceWidth = 1.f;
		STAGE_RECT Gradient, BarFrame, BarFill, AnnounceBg;
		float4_t Title = {}, Remain = {}, Announce = {}, AnnounceGlow = {}, Shadow = {};
		f32_t fGateDelayMs = 0.f, fGateDurationMs = 1.f, fGateSinkMeters = 0.f;
		CURVE eGateCurve = CURVE::OUT_CUBIC;
		std::vector<std::string> GatePlacements;
	};
	struct GATE { uint64_t iPlacementId = 0u; MAP_PLACEMENT_RECORD Base; };

	static constexpr f32_t STAGE_H = 1080.f;
	static constexpr f32_t MAX_STEP_MS = 200.f;
	static constexpr f32_t FONT_PROBE_PX = 20.f;

	// ---- document ----------------------------------------------------------------------
	static const DATA_JSON_VALUE& Field(const DATA_JSON_VALUE& row, const char* pKey)
	{
		const DATA_JSON_VALUE* pValue = row.Find(pKey);
		if (nullptr == pValue)
			throw std::runtime_error(std::string("missing field: ") + pKey);
		return *pValue;
	}
	static f32_t Number(const DATA_JSON_VALUE& row, const char* pKey, double lo, double hi)
	{
		const DATA_JSON_VALUE& value = Field(row, pKey);
		if (!value.Is_Number() || !std::isfinite(value.Get_Number()) ||
			value.Get_Number() < lo || value.Get_Number() > hi)
			throw std::runtime_error(std::string("invalid number: ") + pKey);
		return static_cast<f32_t>(value.Get_Number());
	}
	static const DATA_JSON_VALUE::ARRAY& Array(const DATA_JSON_VALUE& row, const char* pKey)
	{
		const DATA_JSON_VALUE& value = Field(row, pKey);
		if (!value.Is_Array())
			throw std::runtime_error(std::string("invalid array: ") + pKey);
		return value.Get_Array();
	}
	static std::wstring Text(const DATA_JSON_VALUE& row, const char* pKey)
	{
		const DATA_JSON_VALUE& value = Field(row, pKey);
		std::wstring strOut;
		if (!value.Is_String() || !CWorldPlayerNameplateView::Try_ConvertUtf8(value.Get_String(), strOut) ||
			strOut.empty())
			throw std::runtime_error(std::string("invalid text: ") + pKey);
		return strOut;
	}
	static STAGE_RECT Rect(const DATA_JSON_VALUE& row, const char* pKey)
	{
		const DATA_JSON_VALUE& rect = Field(row, pKey);
		STAGE_RECT out;
		out.fDx = Number(rect, "dx", -5000, 5000);
		out.fY = Number(rect, "y", -5000, 5000);
		out.fW = Number(rect, "w", 0.5, 5000);
		out.fH = Number(rect, "h", 0.5, 5000);
		return out;
	}
	static float4_t Color(const DATA_JSON_VALUE& row, const char* pKey)
	{
		const DATA_JSON_VALUE::ARRAY& values = Array(row, pKey);
		if (4u != values.size())
			throw std::runtime_error(std::string("invalid colour: ") + pKey);
		f32_t fRgba[4] = {};
		for (size_t i = 0; i < 4u; ++i)
		{
			if (!values[i].Is_Number() || !std::isfinite(values[i].Get_Number()) ||
				values[i].Get_Number() < 0.0 || values[i].Get_Number() > 1.0)
				throw std::runtime_error(std::string("invalid colour value: ") + pKey);
			fRgba[i] = static_cast<f32_t>(values[i].Get_Number());
		}
		return float4_t(fRgba[0], fRgba[1], fRgba[2], fRgba[3]);
	}

	bool_t Load_Document()
	{
		try
		{
			const std::filesystem::path path = CProjectDataRoot::Resolve(L"Camera/ColosseumMatchStart.json");
			if (std::filesystem::file_size(path) > 1048576u)
				throw std::runtime_error("document exceeds 1 MiB");
			std::ifstream input(path, std::ios::binary);
			std::ostringstream bytes;
			bytes << input.rdbuf();
			DATA_JSON_VALUE root;
			std::string error;
			if (!input || !CDataJson::Parse(bytes.str(), root, error))
				throw std::runtime_error("parse failed: " + error);
			const DATA_JSON_VALUE& schema = Field(root, "schema");
			if (!schema.Is_String() || "lostark.colosseum-match-start" != schema.Get_String() ||
				1.f != Number(root, "formatVersion", 1, 1))
				throw std::runtime_error("wrong schema");

			DOCUMENT doc;
			const DATA_JSON_VALUE& count = Field(root, "countdown");
			doc.fTotalMs = Number(count, "totalMs", 1000, 30000);
			doc.fStartHoldMs = Number(count, "startTextHoldMs", 0, 30000);
			doc.fStartFadeMs = Number(count, "startTextFadeMs", 0, 30000);
			doc.strTitle = Text(count, "titleText");
			doc.strRemainLine = Text(count, "remainLine");
			doc.strAnnounceLine = Text(count, "announceLine");
			doc.strStartText = Text(count, "startText");
			for (const DATA_JSON_VALUE& key : Array(count, "bar"))
				doc.Bar.push_back({ Number(key, "remainingMs", 0, 60000), Number(key, "fraction", 0, 1) });
			if (doc.Bar.size() < 2u)
				throw std::runtime_error("the bar needs at least two keys");
			for (size_t i = 1; i < doc.Bar.size(); ++i)
				if (doc.Bar[i].fRemainingMs >= doc.Bar[i - 1].fRemainingMs)
					throw std::runtime_error("bar keys must run from the largest remainingMs down");
			const DATA_JSON_VALUE& layout = Field(count, "layout");
			doc.fTitleCenterY = Number(layout, "titleCenterY", -1000, 2000);
			doc.fRemainCenterY = Number(layout, "remainCenterY", -1000, 2000);
			doc.fAnnounceCenterY = Number(layout, "announceCenterY", -1000, 2000);
			doc.fTitleWidth = Number(layout, "titleWidth", 1, 4000);
			doc.fRemainWidth = Number(layout, "remainWidth", 1, 4000);
			doc.fAnnounceWidth = Number(layout, "announceWidth", 1, 4000);
			const DATA_JSON_VALUE& banner = Field(layout, "banner");
			doc.Gradient = Rect(banner, "gradient");
			doc.BarFrame = Rect(banner, "barFrame");
			doc.BarFill = Rect(banner, "barFill");
			doc.AnnounceBg = Rect(layout, "announceBg");
			const DATA_JSON_VALUE& colors = Field(count, "colors");
			doc.Title = Color(colors, "title");
			doc.Remain = Color(colors, "remainLine");
			doc.Announce = Color(colors, "announce");
			doc.AnnounceGlow = Color(colors, "announceGlow");
			doc.Shadow = Color(colors, "shadow");

			const DATA_JSON_VALUE& gate = Field(root, "gate");
			doc.fGateDelayMs = Number(gate, "delayAfterStartMs", 0, 30000);
			doc.fGateDurationMs = Number(gate, "durationMs", 1, 30000);
			doc.fGateSinkMeters = Number(gate, "sinkMeters", 0, 50);
			const DATA_JSON_VALUE& curve = Field(gate, "curve");
			if (!curve.Is_String())
				throw std::runtime_error("invalid gate curve");
			if ("LINEAR" == curve.Get_String()) doc.eGateCurve = CURVE::LINEAR;
			else if ("OUT_CUBIC" == curve.Get_String()) doc.eGateCurve = CURVE::OUT_CUBIC;
			else if ("IN_CUBIC" == curve.Get_String()) doc.eGateCurve = CURVE::IN_CUBIC;
			else if ("SMOOTH" == curve.Get_String()) doc.eGateCurve = CURVE::SMOOTH;
			else throw std::runtime_error("unknown gate curve");
			for (const DATA_JSON_VALUE& id : Array(gate, "placements"))
			{
				if (!id.Is_String() || id.Get_String().empty())
					throw std::runtime_error("invalid gate placement id");
				doc.GatePlacements.push_back(id.Get_String());
			}
			m_Doc = std::move(doc);
			return true;
		}
		catch (const std::exception& e)
		{
			m_strStatus = std::string("Colosseum match-start document rejected: ") + e.what();
			OutputDebugStringA(("[Colosseum.MatchStart] " + m_strStatus + "\n").c_str());
			return false;
		}
	}

	// ---- load --------------------------------------------------------------------------
	bool_t Load_All()
	{
		Release();
		m_bReady = false;
		if (nullptr == m_pMap || !Load_Document() || !Create_View())
			return false;
		Bind_Gates();
		m_strStatus.clear();
		if (m_Gates.size() != m_Doc.GatePlacements.size())
			m_strStatus = "gate: " + std::to_string(m_Doc.GatePlacements.size() - m_Gates.size()) +
				" of " + std::to_string(m_Doc.GatePlacements.size()) + " placements were not found in this map";
		m_bReady = true;
		Show_View(false, false);
		return true;
	}

	bool_t Create_View()
	{
		m_pView = std::make_unique<CUILayoutRuntime>(m_pDevice, m_pContext, m_iLevelIndex,
			TEXT("Layer_ColosseumMatchStart"), L"UI/Colosseum/MatchCountdown_Layout.json");
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		for (const char* pId : SLOT_IDS)
		{
			if (!m_pView->Get_SlotRect(pId, fX, fY, fW, fH))
			{
				m_strStatus = std::string("MatchCountdown_Layout.json is missing slot ") + pId;
				OutputDebugStringA(("[Colosseum.MatchStart] " + m_strStatus + "\n").c_str());
				m_pView->Release_Sprites();
				m_pView.reset();
				return false;
			}
		}
		m_pView->Set_UISortLayer(UI_TEXT_LAYER::HUD);
		return true;
	}

	void Bind_Gates()
	{
		m_Gates.clear();
		m_fAppliedSink = 0.f;
		for (const std::string& strSourceId : m_Doc.GatePlacements)
		{
			for (const MAP_RUNTIME_PLACED_ENTRY& entry : m_pMap->Get_MutablePlacements())
			{
				if (entry.record.sourcePlacementId != strSourceId)
					continue;
				GATE gate;
				gate.iPlacementId = entry.record.placementId;
				gate.Base = entry.record;
				m_Gates.push_back(std::move(gate));
				break;
			}
		}
	}

	// ---- gate --------------------------------------------------------------------------
	f32_t Compute_GateSink() const
	{
		const f32_t fStart = m_Doc.fTotalMs + m_Doc.fGateDelayMs;
		const f32_t fRatio = std::clamp((m_fClockMs - fStart) / m_Doc.fGateDurationMs, 0.f, 1.f);
		f32_t fEased = fRatio;
		switch (m_Doc.eGateCurve)
		{
		case CURVE::OUT_CUBIC: fEased = 1.f - (1.f - fRatio) * (1.f - fRatio) * (1.f - fRatio); break;
		case CURVE::IN_CUBIC: fEased = fRatio * fRatio * fRatio; break;
		case CURVE::SMOOTH: fEased = fRatio * fRatio * (3.f - 2.f * fRatio); break;
		default: break;
		}
		return m_Doc.fGateSinkMeters * fEased;
	}

	/* Writes every gate leaf at its placed pose lowered by fSink metres. 0 is the placed pose. */
	void Apply_Gate(const f32_t fSink)
	{
		if (nullptr == m_pMap || m_Gates.empty())
		{
			m_fAppliedSink = fSink;
			return;
		}
		std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements = m_pMap->Get_MutablePlacements();
		for (const GATE& gate : m_Gates)
		{
			MAP_RUNTIME_PLACED_ENTRY* pEntry = nullptr;
			for (MAP_RUNTIME_PLACED_ENTRY& entry : placements)
			{
				if (entry.record.placementId == gate.iPlacementId)
				{
					pEntry = &entry;
					break;
				}
			}
			if (nullptr == pEntry)
				continue;
			MAP_PLACEMENT_RECORD staged = gate.Base;
			staged.position.y -= fSink;
			std::string strStatus;
			if (CMapPlacementRuntime::PLACEMENT_TRANSFORM_RESULT::APPLIED !=
				CMapPlacementRuntime::Apply_PlacementTransform(m_iLevelIndex, m_pMap->Get_Catalog(),
					m_ModelCache, *pEntry, staged, strStatus))
				m_strStatus = "gate leaf could not move: " + strStatus;
		}
		/* A failed leaf is reported in the status line; retrying every frame would only repeat it. */
		m_fAppliedSink = fSink;
	}

	// ---- view --------------------------------------------------------------------------
	void Show_View(const bool_t bBanner, const bool_t bAnnounce)
	{
		if (nullptr == m_pView)
			return;
		m_pView->Set_SlotVisible("Count_TopGradient", bBanner);
		m_pView->Set_SlotVisible("Count_BarFrame", bBanner);
		m_pView->Set_SlotVisible("Count_BarFill", bBanner);
		m_pView->Set_SlotVisible("Count_AnnounceBg", bAnnounce);
	}

	/* The layout document only carries the 16:9 default. Every slot is re-placed in stage space
	   (1920 x 1080, x from the screen centre, uniform scale by height) so a wider window keeps the
	   art's proportions; CUILayoutRuntime maps its reference rect to the viewport per axis, so the
	   rect is converted back through that mapping. */
	void Place(const char* pId, const STAGE_RECT& rect)
	{
		const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
		if (nullptr == m_pView || vViewport.x <= 0.f || vViewport.y <= 0.f)
			return;
		const f32_t fScale = vViewport.y / STAGE_H;
		const f32_t fToRefX = 1280.f / vViewport.x, fToRefY = 720.f / vViewport.y;
		m_pView->Set_SlotRect(pId,
			(vViewport.x * 0.5f + rect.fDx * fScale) * fToRefX, rect.fY * fScale * fToRefY,
			rect.fW * fScale * fToRefX, rect.fH * fScale * fToRefY);
	}

	void Place_Slots()
	{
		Place("Count_TopGradient", m_Doc.Gradient);
		Place("Count_BarFrame", m_Doc.BarFrame);
		Place("Count_BarFill", m_Doc.BarFill);
		Place("Count_AnnounceBg", m_Doc.AnnounceBg);
	}

	f32_t Sample_Bar(const f32_t fRemainingMs) const
	{
		const std::vector<BAR_KEY>& keys = m_Doc.Bar;
		if (fRemainingMs >= keys.front().fRemainingMs)
			return keys.front().fFraction;
		for (size_t i = 1; i < keys.size(); ++i)
		{
			if (fRemainingMs >= keys[i].fRemainingMs)
			{
				const f32_t fSpan = keys[i - 1].fRemainingMs - keys[i].fRemainingMs;
				const f32_t fRatio = fSpan > 0.f ? (fRemainingMs - keys[i].fRemainingMs) / fSpan : 1.f;
				return keys[i].fFraction + (keys[i - 1].fFraction - keys[i].fFraction) * fRatio;
			}
		}
		return keys.back().fFraction;
	}

	void Update_Bar()
	{
		if (nullptr == m_pView)
			return;
		const f32_t fFraction = std::clamp(m_bServerClock ?
			(m_Doc.fTotalMs - m_fClockMs) / (std::max)(1.f,m_Doc.fTotalMs) :
			Sample_Bar((std::max)(0.f, m_Doc.fTotalMs - m_fClockMs)), 0.f, 1.f);
		m_pView->Set_SlotVisible("Count_BarFill", fFraction > 0.001f);
		m_pView->Set_SlotFillRatio("Count_BarFill", fFraction);
	}

	/* The announce (count digits and then the start text) holds, then fades after the start. */
	f32_t Announce_Alpha() const
	{
		const f32_t fHoldEnd = m_Doc.fTotalMs + m_Doc.fStartHoldMs;
		if (m_fClockMs < fHoldEnd)
			return 1.f;
		if (m_Doc.fStartFadeMs <= 0.f)
			return 0.f;
		return std::clamp(1.f - (m_fClockMs - fHoldEnd) / m_Doc.fStartFadeMs, 0.f, 1.f);
	}

	void Update_Announce_Alpha()
	{
		if (m_pView)
			m_pView->Set_SlotTintMultiplier("Count_AnnounceBg",
				float4_t(1.f, 1.f, 1.f, Announce_Alpha()));
	}

	// ---- text --------------------------------------------------------------------------
	static std::wstring Fill_Number(std::wstring strTemplate, const std::wstring& strNumber)
	{
		const std::wstring strToken = L"{0}";
		const size_t iAt = strTemplate.find(strToken);
		if (std::wstring::npos != iAt)
			strTemplate.replace(iAt, strToken.size(), strNumber);
		return strTemplate;
	}
	static std::wstring Two_Digits(const uint32_t iValue)
	{
		return (iValue < 10u ? L"0" : L"") + std::to_wstring(iValue);
	}

	/* Font sizes follow the measured retail text widths (stage px) instead of a guessed point size,
	   so the lines keep their proportions whatever font atlas is picked for the window size. */
	f32_t Fit_Px(CGameInstance& gameInstance, const std::wstring& strReference, const f32_t fTargetStagePx,
		const f32_t fViewportHeight) const
	{
		f32_t fScale = 1.f;
		const wstring_t strFont = UILabelFont::Resolve(TEXT("Font_YG760"), FONT_PROBE_PX, fScale);
		const float2_t vSize = gameInstance.Measure_Text(strFont, strReference.c_str());
		const f32_t fMeasured = vSize.x * fScale;
		const f32_t fTarget = fTargetStagePx * fViewportHeight / STAGE_H;
		return fMeasured > 0.001f ? FONT_PROBE_PX * fTarget / fMeasured : FONT_PROBE_PX;
	}

	void Refit_Fonts(CGameInstance& gameInstance, const f32_t fViewportHeight)
	{
		if (std::abs(fViewportHeight - m_fFittedViewportH) < 0.5f)
			return;
		m_fFittedViewportH = fViewportHeight;
		m_fTitlePx = Fit_Px(gameInstance, m_Doc.strTitle, m_Doc.fTitleWidth, fViewportHeight);
		m_fRemainPx = Fit_Px(gameInstance, Fill_Number(m_Doc.strRemainLine, L"03"), m_Doc.fRemainWidth,
			fViewportHeight);
		m_fAnnouncePx = Fit_Px(gameInstance, Fill_Number(m_Doc.strAnnounceLine, L"2"), m_Doc.fAnnounceWidth,
			fViewportHeight);
	}

	static void Draw_Line(CGameInstance& gameInstance, const std::wstring& strText, const f32_t fCenterX,
		const f32_t fCenterY, const f32_t fPx, const float4_t& color, const float4_t& outline, const f32_t fAlpha,
		const f32_t fStageScale, const f32_t fStrokeStage)
	{
		if (strText.empty() || fPx <= 0.f)
			return;
		f32_t fScale = 1.f;
		const wstring_t strFont = UILabelFont::Resolve(TEXT("Font_YG760"), fPx, fScale);
		const float2_t vSize = gameInstance.Measure_Text(strFont, strText.c_str());
		const float2_t vPosition(std::round(fCenterX - vSize.x * fScale * 0.5f),
			std::round(fCenterY - vSize.y * fScale * 0.5f));
		/* A soft dark edge: the retail glyphs carry a shadow (banner) or a red glow (announce). */
		const f32_t fStroke = (std::max)(1.f, fStrokeStage > 0.f ? fStrokeStage * fStageScale : fStageScale);
		static constexpr f32_t OFFSETS[8][2] = {
			{ -1.f, 0.f }, { 1.f, 0.f }, { 0.f, -1.f }, { 0.f, 1.f },
			{ -1.f, -1.f }, { 1.f, -1.f }, { -1.f, 1.f }, { 1.f, 1.f } };
		const fvector_t vOutline = XMVectorSet(outline.x, outline.y, outline.z, outline.w * fAlpha);
		for (const auto& offset : OFFSETS)
			gameInstance.Draw_Text(strFont, strText.c_str(),
				float2_t(vPosition.x + offset[0] * fStroke, vPosition.y + offset[1] * fStroke),
				vOutline, 0.f, float2_t(0.f, 0.f), fScale);
		const fvector_t vColor = XMVectorSet(color.x, color.y, color.z, color.w * fAlpha);
		gameInstance.Draw_Text(strFont, strText.c_str(), vPosition, vColor, 0.f, float2_t(0.f, 0.f), fScale);
	}

	void Release()
	{
		if (m_pView)
		{
			m_pView->Set_AllSlotsVisible(false);
			m_pView->Release_Sprites();
			m_pView.reset();
		}
		m_Gates.clear();
		m_ModelCache.clear();
		m_fAppliedSink = 0.f;
		m_fFittedViewportH = 0.f;
	}

private:
	static constexpr const char* SLOT_IDS[] = {
		"Count_TopGradient", "Count_BarFrame", "Count_BarFill", "Count_AnnounceBg" };

	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pContext;
	uint32_t m_iLevelIndex = 0u;
	CMapPlacementRuntime* m_pMap = nullptr;
	DOCUMENT m_Doc;
	std::unique_ptr<CUILayoutRuntime> m_pView;
	std::vector<GATE> m_Gates;
	std::unordered_map<std::string, shared_ptr<Engine::CModel>> m_ModelCache;
	bool_t m_bReady = false;
	bool_t m_bRunning = false;
	bool_t m_bPaused = false;
	bool_t m_bSkipStep = false;
	bool_t m_bServerClock = false;
	f32_t m_fClockMs = 0.f;
	f32_t m_fAppliedSink = 0.f;
	f32_t m_fFittedViewportH = 0.f;
	f32_t m_fTitlePx = 0.f, m_fRemainPx = 0.f, m_fAnnouncePx = 0.f;
	std::string m_strStatus;
};

NS_END

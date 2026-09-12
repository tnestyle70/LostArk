#include "MvpResultView.h"

#include "GameInstance.h"
#include "UILayoutRuntime.h"

#include <algorithm>
#include <cstdio>

namespace
{
	/* The movie's own stage is 1920x1080 and this project's authoring canvas is
	1280x720, so every measured coordinate is scaled by one uniform factor. The
	document is placed with pivotType "center" at widgetX 980, and its frame-1
	bounding box runs local x -20..2580 (centre 1280), which puts frame-local x at
	screen x - 300 before that scale. */
	constexpr f32_t STAGE_TO_CANVAS = 1280.f / 1920.f;
	constexpr f32_t FRAME_ORIGIN_X = -300.f;

	constexpr f32_t CanvasX(const f32_t fLocalX) { return (fLocalX + FRAME_ORIGIN_X) * STAGE_TO_CANVAS; }
	constexpr f32_t CanvasY(const f32_t fLocalY) { return fLocalY * STAGE_TO_CANVAS; }
	/* Authored point size -> canvas pixels. */
	constexpr f32_t CanvasPt(const f32_t fPoints) { return fPoints * STAGE_TO_CANVAS; }

	/* MvpResultFrame is 135 frames on a 40fps stage. */
	constexpr f32_t MOVIE_FPS = 40.f;
	constexpr f32_t INTRO_FRAMES = 135.f;

	const wchar_t* const FONT_YOON = TEXT("Font_YoonGasiIIM");
	const wchar_t* const FONT_YG760 = TEXT("Font_YG760");
	/* A label's scale must not depend on which glyphs it happens to contain, so
	both fonts are measured once against a fixed string. */
	const wchar_t* const METRIC_STRING = TEXT("Ag");

	/* sys.mvp.button_confirm. Universal character names: this file stays ASCII. */
	const wchar_t* const EXIT_LABEL = L"\uB098\uAC00\uAE30[Esc]";

	/* Authored colours, read from each DefineEditText's own initialText. */
	const fvector_t COLOR_WHITE = XMVectorSet(1.f, 1.f, 1.f, 1.f);
	/* #f2d694 -- the award title ("Fierce Bloodsport"). */
	const fvector_t COLOR_TITLE = XMVectorSet(0.949f, 0.839f, 0.580f, 1.f);
	/* #bacfd2 -- a party member's contribution name. */
	const fvector_t COLOR_PARTY_DESC = XMVectorSet(0.729f, 0.811f, 0.823f, 1.f);
	/* 45663 == #00b25f, set by MvpResultFrame.mvpData over the authored #00ca98. */
	const fvector_t COLOR_GUILD = XMVectorSet(0.f, 0.698f, 0.373f, 1.f);

	/* The three party columns share one motion -- y -50 -> 0 with a fade -- each
	starting seven frames after the one before it. */
	constexpr f32_t PARTY_IN_START[] = { 81.f, 88.f, 94.f };
	constexpr f32_t PARTY_IN_END[] = { 98.f, 105.f, 111.f };
	constexpr f32_t PARTY_RISE_LOCAL = -50.f;
	/* MVP stat rows fade in last. */
	constexpr f32_t STAT_IN_START = 122.f;
	constexpr f32_t STAT_IN_END = 133.f;
	/* The MVP render area slides in from local x 800 to its resting 320. */
	constexpr f32_t MAIN_BG_FROM_LOCAL_X = 800.f;
	constexpr f32_t MAIN_BG_TO_LOCAL_X = 320.f;
	constexpr f32_t MAIN_BG_SLIDE_START = 66.f;
	constexpr f32_t MAIN_BG_SLIDE_END = 92.f;

	/* MVP side, frame-local. */
	constexpr f32_t MVP_TITLE_CENTER_LOCAL_X = 800.f;
	constexpr f32_t MVP_TITLE_CENTER_LOCAL_Y = 697.f;
	/* Point sizes come from MvpResultFrame's __setProp_* blocks, not guessed. */
	constexpr f32_t MVP_CONTENT_LOCAL_X = 382.f;
	constexpr f32_t MVP_CONTENT_LOCAL_Y = 578.f;
	constexpr f32_t MVP_CONTENT_POINTS = 24.f;
	constexpr f32_t MVP_NAME_LOCAL_X = 610.f;
	constexpr f32_t MVP_NAME_LOCAL_Y = 732.f;
	constexpr f32_t MVP_NAME_POINTS = 32.f;
	constexpr f32_t MVP_GUILD_CENTER_LOCAL_X = 800.f;
	constexpr f32_t MVP_GUILD_CENTER_LOCAL_Y = 782.f;
	/* mvpStatItem0/1/2 origins; every field inside centres on originX + 156. */
	constexpr f32_t MVP_STAT_LOCAL_X[] = { 399.f, 643.f, 887.f };
	constexpr f32_t MVP_STAT_LOCAL_Y = 805.f;
	constexpr f32_t MVP_STAT_CENTER_DX = 156.f;
	constexpr f32_t MVP_STAT_TITLE_DY = 66.8f;
	constexpr f32_t MVP_STAT_DESC_DY = 101.4f;
	constexpr f32_t MVP_STAT_VALUE_DY = 151.4f;
	/* mvpBadgeList sits at local (326, 896) and its track is 910x198, so the strip
	   centres on 781, and nine icons -- where retail collapses the list -- give the
	   910/9 pitch. RollingRepositionList centres its content, so a shorter row stays
	   centred instead of starting at the left edge. */
	constexpr f32_t MVP_MEDAL_CENTER_LOCAL_X = 781.f;
	constexpr f32_t MVP_MEDAL_CENTER_LOCAL_Y = 995.f;
	constexpr f32_t MVP_MEDAL_PITCH_LOCAL = 101.f;
	/* MvpResult_BadgeListItem_L draws MvpBadgeIcon full size; the party columns use
	   MvpResult_BadgeListItem_small, which scales the same icon by 0.8099823. */
	constexpr f32_t MEDAL_ICON_LOCAL_MVP = 80.f;
	constexpr f32_t MEDAL_ICON_LOCAL_PARTY = 80.f * 0.8099823f;
	constexpr int32_t MEDAL_INDEX_MIN = 1;
	constexpr int32_t MEDAL_INDEX_MAX = 17;
	constexpr size_t MVP_MEDAL_MAX = 9u;

	/* Party columns, frame-local origins of otherStatItem0/1/2. */
	constexpr f32_t PARTY_LOCAL_X[] = { 1281.f, 1600.f, 1921.f };
	constexpr f32_t PARTY_NAME_CENTER_DX = 161.f;
	constexpr f32_t PARTY_NAME_LOCAL_Y = 738.f;
	constexpr f32_t PARTY_GUILD_LOCAL_Y = 758.f;
	constexpr f32_t PARTY_STAT_CENTER_DX = 160.f;
	constexpr f32_t PARTY_STAT_LOCAL_Y[] = { 772.f, 840.f, 908.f };
	constexpr f32_t PARTY_STAT_TITLE_DY = 37.f;
	constexpr f32_t PARTY_STAT_DESC_DY = 56.4f;
	/* OtherBadgeList sits at local (col + 54, 1003) and is 216 wide. */
	constexpr f32_t PARTY_MEDAL_CENTER_DX = 162.f;
	constexpr f32_t PARTY_MEDAL_CENTER_LOCAL_Y = 1043.f;
	constexpr f32_t PARTY_MEDAL_PITCH_LOCAL = 72.f;
	constexpr size_t PARTY_MEDAL_MAX = 3u;

	/* Only the render-target stand-ins live outside the keyframe document; the
	   column's own panel art rides its own layer in there. */
	constexpr size_t PARTY_SLOTS_PER_COLUMN = 1u;
	const char* const PARTY_COLUMN_SLOTS[] =
	{
		"MvpResult_Party_Bg_0", "MvpResult_Party_Bg_1", "MvpResult_Party_Bg_2",
	};

	/* confirmBtn's own textField is right-aligned and its box ends at local x 2184. */
	constexpr f32_t PARTICLE_BOOM_FRAME = 32.f;
	constexpr f32_t SUCCESS_BURST_FRAME = 99.f;
	constexpr f32_t EXIT_LABEL_RIGHT_LOCAL_X = 2184.f;
	constexpr f32_t EXIT_LABEL_CENTER_LOCAL_Y = 49.f;

	constexpr size_t PARTY_COLUMN_COUNT = 3u;
	constexpr size_t MVP_STAT_COUNT = 3u;
	constexpr size_t PARTY_STAT_COUNT = 3u;

	f32_t Ramp(const f32_t fFrame, const f32_t fStart, const f32_t fEnd)
	{
		if (fFrame <= fStart)
			return 0.f;
		if (fEnd <= fStart || fFrame >= fEnd)
			return 1.f;
		return (fFrame - fStart) / (fEnd - fStart);
	}

}

Client::CMvpResultView::CMvpResultView(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iGameObjectLevelIndex)
{
	m_pView = std::make_unique<CUILayoutRuntime>(pDevice, pContext, iGameObjectLevelIndex,
		TEXT("Layer_UI"), L"UI/MVP/MvpResult_Layout.json");
	m_pView->Set_AllSlotsVisible(false);

	const float2_t vYoon = CGameInstance::Get().Measure_Text(FONT_YOON, METRIC_STRING);
	const float2_t vYG760 = CGameInstance::Get().Measure_Text(FONT_YG760, METRIC_STRING);
	m_fYoonMetricHeight = vYoon.y;
	m_fYG760MetricHeight = vYG760.y;

	for (size_t i = 0; i < PARTY_COLUMN_COUNT * PARTY_SLOTS_PER_COLUMN; ++i)
	{
		AUTHORED_SLOT& Authored = m_PartyAuthored[i];
		f32_t fW = 0.f, fH = 0.f;
		if (m_pView->Get_SlotRect(
				PARTY_COLUMN_SLOTS[i], Authored.fX, Authored.fY, fW, fH))
			Authored.strId = PARTY_COLUMN_SLOTS[i];
	}
}

Client::CMvpResultView::~CMvpResultView()
{
}

void Client::CMvpResultView::Show(const MVP_RESULT_DATA& Data)
{
	m_Data = Data;
	if (m_Data.Party.size() > PARTY_COLUMN_COUNT)
		m_Data.Party.resize(PARTY_COLUMN_COUNT);
	m_bVisible = true;
	m_fElapsedSeconds = 0.f;
	m_bParticleBoomStarted = false;
	m_bSuccessBurstStarted = false;
	m_pView->Set_AllSlotsVisible(true);
	/* Every authored image layer of MvpResultFrame plays from the extracted
		keyframe document, so its own per-frame transform drives them instead of a
		hand-written envelope. */
	m_pView->Play_KeyframeAnimation("MvpResult_Frame", "intro");
	for (const string& strMedalSlot : m_MedalSlotIds)
		m_pView->Set_SlotVisible(strMedalSlot, false);
	Apply_Timeline();
}

void Client::CMvpResultView::Hide()
{
	m_bVisible = false;
	m_fElapsedSeconds = 0.f;
	m_pView->Set_AllSlotsVisible(false);
}

void Client::CMvpResultView::Update(const f32_t fTimeDelta)
{
	if (!m_bVisible)
		return;
	m_fElapsedSeconds += fTimeDelta;
	Apply_Timeline();
	m_pView->Update(fTimeDelta);
}

void Client::CMvpResultView::Apply_Timeline()
{
	const f32_t fFrame = (std::min)(m_fElapsedSeconds * MOVIE_FPS, INTRO_FRAMES);

	/* The burst is a one-shot flipbook, so it is rewound the frame it enters
	   rather than left wherever the previous play stopped. */
	if (!m_bParticleBoomStarted && fFrame >= PARTICLE_BOOM_FRAME)
	{
		m_pView->Restart_Animation("MvpResult_ParticleBoom");
		m_bParticleBoomStarted = true;
	}
	if (!m_bSuccessBurstStarted && fFrame >= SUCCESS_BURST_FRAME)
	{
		m_pView->Restart_Animation("MvpResult_SuccessBurst");
		m_bSuccessBurstStarted = true;
	}

	/* The MVP render area slides left while it is already opaque. */
	const f32_t fSlide = Ramp(fFrame, MAIN_BG_SLIDE_START, MAIN_BG_SLIDE_END);
	const f32_t fMainLocalX =
		MAIN_BG_FROM_LOCAL_X + (MAIN_BG_TO_LOCAL_X - MAIN_BG_FROM_LOCAL_X) * fSlide;
	f32_t fSlotX = 0.f, fSlotY = 0.f, fSlotW = 0.f, fSlotH = 0.f;
	if (m_pView->Get_SlotRect("MvpResult_MainBackground", fSlotX, fSlotY, fSlotW, fSlotH))
		m_pView->Set_SlotPosition("MvpResult_MainBackground", CanvasX(fMainLocalX), fSlotY);

	for (size_t i = 0; i < PARTY_COLUMN_COUNT; ++i)
	{
		const f32_t fT = Ramp(fFrame, PARTY_IN_START[i], PARTY_IN_END[i]);
		const f32_t fAlpha = (i < m_Data.Party.size()) ? fT : 0.f;
		/* Absolute, never relative: Get_SlotRect reads back what this function
		   already wrote, so adding the offset every frame would drift. */
		const f32_t fRiseCanvas = CanvasY(PARTY_RISE_LOCAL * (1.f - fT));
		for (size_t k = 0; k < PARTY_SLOTS_PER_COLUMN; ++k)
		{
			const AUTHORED_SLOT& Authored =
				m_PartyAuthored[i * PARTY_SLOTS_PER_COLUMN + k];
			if (Authored.strId.empty())
				continue;
			m_pView->Set_SlotTintMultiplier(Authored.strId,
				float4_t(1.f, 1.f, 1.f, fAlpha));
			m_pView->Set_SlotPosition(Authored.strId,
				Authored.fX, Authored.fY + fRiseCanvas);
		}
	}
}

void Client::CMvpResultView::Draw_Label(
	const wchar_t* const pFontTag,
	const wstring_t& strText,
	const f32_t fCenterX,
	const f32_t fCenterY,
	const f32_t fFontPx,
	const float2_t& vOrigin,
	const fvector_t vColor) const
{
	if (strText.empty())
		return;
	const f32_t fMetric =
		(pFontTag == FONT_YOON) ? m_fYoonMetricHeight : m_fYG760MetricHeight;
	if (fMetric <= 0.f)
		return;
	/* Same convention as CMainApp::RenderRaidClearText: authored 1280x720 coords are
	stretched per axis, while the glyph scale follows the smaller of the two so text
	never distorts. */
	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	const f32_t fScaleX = vViewport.x / 1280.f;
	const f32_t fScaleY = vViewport.y / 720.f;
	const f32_t fUiScale = (std::min)(fScaleX, fScaleY);
	CGameInstance::Get().Draw_Text(pFontTag, strText.c_str(),
		float2_t(fCenterX * fScaleX, fCenterY * fScaleY),
		vColor, 0.f, vOrigin, (fFontPx * fUiScale) / fMetric);
}

void Client::CMvpResultView::Render()
{
	if (!m_bVisible)
		return;
	Render_MvpSide();
	for (size_t i = 0; i < m_Data.Party.size(); ++i)
		Render_PartyColumn(i);
}

void Client::CMvpResultView::Render_MvpSide() const
{
	const f32_t fFrame = (std::min)(m_fElapsedSeconds * MOVIE_FPS, INTRO_FRAMES);
	const f32_t fPlateAlpha = Ramp(fFrame, 93.f, 108.f);
	const f32_t fStatAlpha = Ramp(fFrame, STAT_IN_START, STAT_IN_END);
	const float2_t vCenter(0.5f, 0.5f);
	const float2_t vTopLeft(0.f, 0.f);

	/* The window's own headline, "MVP" (sys.mvp.main_title). */
	Draw_Label(FONT_YOON, TEXT("MVP"),
		CanvasX(MVP_TITLE_CENTER_LOCAL_X), CanvasY(MVP_TITLE_CENTER_LOCAL_Y), CanvasPt(110.f), vCenter,
		XMVectorSetW(COLOR_WHITE, fPlateAlpha));

	/* contentNameTF and characterNameTF are host-substituted SimpleLabels, but their
	size and alignment are not lost with the symbol: MvpResultFrame's generated
	__setProp_* blocks carry them.
	  contentNameTF    size 24, color 0xFFFFFF, align centre / centre
	  characterNameTF  size 32, color 0xFFFFFF, align centre / top
	alignHorizontal "center" makes each PlaceObject translate that label's own box
	centre, so the two sit on their own x rather than a shared axis. */
	Draw_Label(FONT_YOON, m_Data.strContentName,
		CanvasX(MVP_CONTENT_LOCAL_X), CanvasY(MVP_CONTENT_LOCAL_Y),
		CanvasPt(MVP_CONTENT_POINTS), vCenter,
		XMVectorSetW(COLOR_WHITE, fPlateAlpha));
	/* alignVertical "top": this translate is the box top, not its middle. */
	Draw_Label(FONT_YOON, m_Data.Mvp.strCharacterName,
		CanvasX(MVP_NAME_LOCAL_X), CanvasY(MVP_NAME_LOCAL_Y),
		CanvasPt(MVP_NAME_POINTS), float2_t(0.5f, 0.f),
		XMVectorSetW(COLOR_WHITE, fPlateAlpha));

	/* sys.mvp.button_confirm, on confirmBtn's right-aligned field. */
	Draw_Label(FONT_YOON, EXIT_LABEL,
		CanvasX(EXIT_LABEL_RIGHT_LOCAL_X), CanvasY(EXIT_LABEL_CENTER_LOCAL_Y),
		CanvasPt(20.f), float2_t(1.f, 0.5f),
		XMVectorSetW(COLOR_WHITE, fPlateAlpha));

	/* guildNameTF and serverNameTF are authored on boxes that overlap by design in
	the source (both centre on local x 800, y 782 and 791). */
	Draw_Label(FONT_YG760, m_Data.Mvp.strGuildName,
		CanvasX(MVP_GUILD_CENTER_LOCAL_X), CanvasY(MVP_GUILD_CENTER_LOCAL_Y), CanvasPt(20.f), vCenter,
		XMVectorSetW(COLOR_GUILD, fPlateAlpha));

	const size_t iStatCount = (std::min)(m_Data.Mvp.Stats.size(), MVP_STAT_COUNT);
	for (size_t i = 0; i < iStatCount; ++i)
	{
		const MVP_RESULT_STAT& Stat = m_Data.Mvp.Stats[i];
		const f32_t fCenterLocalX = MVP_STAT_LOCAL_X[i] + MVP_STAT_CENTER_DX;
		Draw_Label(FONT_YOON, Stat.strTitle,
			CanvasX(fCenterLocalX), CanvasY(MVP_STAT_LOCAL_Y + MVP_STAT_TITLE_DY), CanvasPt(24.f), vCenter,
			XMVectorSetW(COLOR_TITLE, fStatAlpha));
		Draw_Label(FONT_YG760, Stat.strDescription,
			CanvasX(fCenterLocalX), CanvasY(MVP_STAT_LOCAL_Y + MVP_STAT_DESC_DY), CanvasPt(16.f), vCenter,
			XMVectorSetW(COLOR_WHITE, fStatAlpha));
		Draw_Label(FONT_YOON, Stat.strValue,
			CanvasX(fCenterLocalX), CanvasY(MVP_STAT_LOCAL_Y + MVP_STAT_VALUE_DY), CanvasPt(44.f), vCenter,
			XMVectorSetW(COLOR_WHITE, fStatAlpha));
	}

	Render_Medals(m_Data.Mvp.Medals, "Mvp", MVP_MEDAL_CENTER_LOCAL_X,
		MVP_MEDAL_CENTER_LOCAL_Y, MVP_MEDAL_PITCH_LOCAL, MEDAL_ICON_LOCAL_MVP,
		MVP_MEDAL_MAX);
}

void Client::CMvpResultView::Render_PartyColumn(const size_t iColumn) const
{
	if (iColumn >= PARTY_COLUMN_COUNT || iColumn >= m_Data.Party.size())
		return;
	const MVP_RESULT_ENTRY& Entry = m_Data.Party[iColumn];
	const f32_t fFrame = (std::min)(m_fElapsedSeconds * MOVIE_FPS, INTRO_FRAMES);
	const f32_t fAlpha = Ramp(fFrame, PARTY_IN_START[iColumn], PARTY_IN_END[iColumn]);
	const f32_t fColumnLocalX = PARTY_LOCAL_X[iColumn];
	const f32_t fRiseLocal = PARTY_RISE_LOCAL * (1.f - fAlpha);
	const float2_t vCenter(0.5f, 0.5f);

	Draw_Label(FONT_YOON, Entry.strCharacterName,
		CanvasX(fColumnLocalX + PARTY_NAME_CENTER_DX), CanvasY(PARTY_NAME_LOCAL_Y + fRiseLocal),
		CanvasPt(22.f), vCenter, XMVectorSetW(COLOR_WHITE, fAlpha));
	Draw_Label(FONT_YG760, Entry.strGuildName,
		CanvasX(fColumnLocalX + PARTY_NAME_CENTER_DX), CanvasY(PARTY_GUILD_LOCAL_Y + fRiseLocal),
		CanvasPt(14.f), vCenter, XMVectorSetW(COLOR_GUILD, fAlpha));

	const size_t iStatCount = (std::min)(Entry.Stats.size(), PARTY_STAT_COUNT);
	for (size_t j = 0; j < iStatCount; ++j)
	{
		const MVP_RESULT_STAT& Stat = Entry.Stats[j];
		const f32_t fCenterLocalX = fColumnLocalX + PARTY_STAT_CENTER_DX;
		Draw_Label(FONT_YOON, Stat.strTitle,
			CanvasX(fCenterLocalX), CanvasY(PARTY_STAT_LOCAL_Y[j] + PARTY_STAT_TITLE_DY + fRiseLocal),
			CanvasPt(20.f), vCenter, XMVectorSetW(COLOR_TITLE, fAlpha));
		Draw_Label(FONT_YG760, Stat.strDescription,
			CanvasX(fCenterLocalX), CanvasY(PARTY_STAT_LOCAL_Y[j] + PARTY_STAT_DESC_DY + fRiseLocal),
			CanvasPt(16.f), vCenter, XMVectorSetW(COLOR_PARTY_DESC, fAlpha));
	}

	char szOwner[16] = {};
	(void)sprintf_s(szOwner, "Party%zu", iColumn);
	Render_Medals(Entry.Medals, szOwner, fColumnLocalX + PARTY_MEDAL_CENTER_DX,
		PARTY_MEDAL_CENTER_LOCAL_Y + fRiseLocal, PARTY_MEDAL_PITCH_LOCAL,
		MEDAL_ICON_LOCAL_PARTY, PARTY_MEDAL_MAX);
}

void Client::CMvpResultView::Render_Medals(
	const vector<int32_t>& Medals,
	const char* const szOwner,
	const f32_t fCenterLocalX,
	const f32_t fCenterLocalY,
	const f32_t fPitchLocal,
	const f32_t fIconLocal,
	const size_t iMaxShown) const
{
	/* MvpResultFrame does not put the badge list on the timeline: mvpData arms a
	setTimeout of Shared_Setting element(0,1) seconds, default 2.7. */
	constexpr f32_t BADGE_DELAY_SECONDS = 2.7f;
	if (m_fElapsedSeconds < BADGE_DELAY_SECONDS)
		return;

	const size_t iCount = (std::min)(Medals.size(), iMaxShown);
	for (size_t i = 0; i < iCount; ++i)
	{
		const int32_t iMedal = Medals[i];
		if (iMedal < MEDAL_INDEX_MIN || iMedal > MEDAL_INDEX_MAX)
			continue;
		char szSlotId[96] = {};
		(void)sprintf_s(szSlotId, "MvpResult_MedalIcon_%s_%zu", szOwner, i);
		char szTexture[64] = {};
		(void)sprintf_s(szTexture, "UI/MVP/MvpResult_Medal_%02d.png", iMedal);
		const f32_t fRowLeft = fCenterLocalX
			- fPitchLocal * (static_cast<f32_t>(iCount) - 1.f) * 0.5f;
		const f32_t fLocalX = fRowLeft + fPitchLocal * static_cast<f32_t>(i)
			- fIconLocal * 0.5f;
		const f32_t fIconCanvas = fIconLocal * STAGE_TO_CANVAS;
		m_pView->Ensure_RuntimeSlot(szSlotId,
			CanvasX(fLocalX), CanvasY(fCenterLocalY - fIconLocal * 0.5f),
			fIconCanvas, fIconCanvas, szTexture);
		m_pView->Set_SlotVisible(szSlotId, true);
		if (m_MedalSlotIds.end() ==
				std::find(m_MedalSlotIds.begin(), m_MedalSlotIds.end(), szSlotId))
			m_MedalSlotIds.push_back(szSlotId);
	}
}

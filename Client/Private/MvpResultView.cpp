#include "MvpResultView.h"

#include "GameInstance.h"
#include "UILabelFont.h"
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
	/* CanvasPt already yields the value this page has always treated as a line
	spacing: the old code divided it by Measure_Text().y, which is the atlas line
	spacing, so a 110 pt title asked for a 110 pt line. UILabelFont wants the same
	quantity, so there is no em conversion here -- putting the other windows'
	1.25 em factor in front of it drew every label a quarter too large and ran the
	headline into the title. */

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
	/* It is also invisible until well after the explosion. Depth 13's own
	   alphaMultTerm keys are 0 through frame 45, then a straight line to 0.949 by
	   frame 64, held from there. Retail fires the burst at frame 32 onto a page that
	   is still empty apart from the dim, which is why the award content has to wait
	   here instead of standing at full opacity from frame 1. */
	constexpr f32_t MAIN_BG_FADE_START = 45.f;
	constexpr f32_t MAIN_BG_FADE_END = 64.f;
	constexpr f32_t MAIN_BG_HOLD_ALPHA = 0.949f;

	/* MVP side, frame-local.

	   The label positions below are measured off the reference capture, not read out
	   of mvp.gfx. The document places contentNameTF / characterNameTF / the stat items
	   as ARK components whose final position the host and MvpResultFrame.as decide at
	   runtime (LayoutSetter, and the AS that re-centres against measured text width),
	   so the authored translate is a starting point the client moves. Point sizes and
	   colours still come from the __setProp_* blocks. Capture pixels convert as
	   local_x = canvas * 1.5 + 300, local_y = canvas * 1.5. */
	constexpr f32_t MVP_TITLE_CENTER_LOCAL_X = 777.f;      // canvas 318.0
	constexpr f32_t MVP_TITLE_CENTER_LOCAL_Y = 679.5f;     // canvas 453.0
	constexpr f32_t MVP_CONTENT_LOCAL_X = 762.75f;         // canvas 308.5
	constexpr f32_t MVP_CONTENT_LOCAL_Y = 621.75f;         // canvas 414.5
	constexpr f32_t MVP_CONTENT_POINTS = 24.f;
	/* Drawn top-centred, so this y is the top of the glyph band (canvas 490). */
	constexpr f32_t MVP_NAME_LOCAL_X = 771.75f;            // canvas 314.5
	constexpr f32_t MVP_NAME_LOCAL_Y = 735.f;
	constexpr f32_t MVP_NAME_POINTS = 32.f;
	constexpr f32_t MVP_GUILD_CENTER_LOCAL_X = 771.75f;
	constexpr f32_t MVP_GUILD_CENTER_LOCAL_Y = 782.f;
	/* mvpStatItem0/1/2 origins; every field inside centres on originX + 156. The
	   three value columns land on canvas 154 / 316 / 478. */
	constexpr f32_t MVP_STAT_LOCAL_X[] = { 375.f, 618.f, 861.f };
	constexpr f32_t MVP_STAT_LOCAL_Y = 799.75f;
	constexpr f32_t MVP_STAT_CENTER_DX = 156.f;
	constexpr f32_t MVP_STAT_TITLE_DY = 66.8f;
	constexpr f32_t MVP_STAT_DESC_DY = 96.5f;
	constexpr f32_t MVP_STAT_VALUE_DY = 139.4f;
	/* The badge strip sits below the value row rather than across it; three icons
	   span canvas 165 in the capture, which is a 83.75 pitch at this icon size. */
	constexpr f32_t MVP_MEDAL_CENTER_LOCAL_X = 770.25f;    // canvas 313.5
	constexpr f32_t MVP_MEDAL_CENTER_LOCAL_Y = 1029.75f;   // canvas 686.5
	constexpr f32_t MVP_MEDAL_PITCH_LOCAL = 83.75f;
	/* MvpResult_BadgeListItem_L draws MvpBadgeIcon full size; the party columns use
	   MvpResult_BadgeListItem_small, which scales the same icon by 0.8099823. */
	constexpr f32_t MEDAL_ICON_LOCAL_MVP = 80.f;
	constexpr f32_t MEDAL_ICON_LOCAL_PARTY = 80.f * 0.8099823f;
	constexpr int32_t MEDAL_INDEX_MIN = 1;
	constexpr int32_t MEDAL_INDEX_MAX = 17;
	constexpr size_t MVP_MEDAL_MAX = 9u;

	/* Party columns, frame-local origins of otherStatItem0/1/2. The authored 319/321
	   pitch is right -- the three columns in the capture sit 213 canvas apart -- but
	   the whole block starts 45 local further left than the authored translate, which
	   puts the name centres on canvas 731 / 944 / 1158 against the capture's
	   734 / 942 / 1159. */
	constexpr f32_t PARTY_LOCAL_X[] = { 1236.f, 1555.f, 1876.f };
	constexpr f32_t PARTY_NAME_CENTER_DX = 161.f;
	constexpr f32_t PARTY_NAME_LOCAL_Y = 731.25f;          // canvas 487.5
	constexpr f32_t PARTY_GUILD_LOCAL_Y = 751.25f;
	constexpr f32_t PARTY_STAT_CENTER_DX = 160.f;
	constexpr f32_t PARTY_STAT_LOCAL_Y[] = { 767.5f, 835.5f, 903.5f };
	constexpr f32_t PARTY_STAT_TITLE_DY = 37.f;
	constexpr f32_t PARTY_STAT_DESC_DY = 56.4f;
	/* OtherBadgeList sits at local (col + 54, 1003) and is 216 wide. */
	constexpr f32_t PARTY_MEDAL_CENTER_DX = 162.f;
	constexpr f32_t PARTY_MEDAL_CENTER_LOCAL_Y = 1026.5f;  // canvas 684.3
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

	constexpr size_t PARTY_COLUMN_COUNT = MVP_RESULT_MAX_PARTY_COLUMNS;
	constexpr size_t MVP_STAT_COUNT = MVP_RESULT_MAX_STATS;
	constexpr size_t PARTY_STAT_COUNT = MVP_RESULT_MAX_STATS;

	f32_t Ramp(const f32_t fFrame, const f32_t fStart, const f32_t fEnd)
	{
		if (fFrame <= fStart)
			return 0.f;
		if (fEnd <= fStart || fFrame >= fEnd)
			return 1.f;
		return (fFrame - fStart) / (fEnd - fStart);
	}

	/* The badge stamp-in, straight off mvpBadgeList.motionPreset[5]
	   "...,6,150,0,0.3,0.1,1" as RollingRepositionList's case 5 reads it: scaleValue,
	   marginY, init_alpha, tween_time and the per-index delay. */
	constexpr f32_t BADGE_START_SCALE = 6.f;
	constexpr f32_t BADGE_START_RISE_LOCAL = 150.f;
	constexpr f32_t BADGE_TWEEN_SECONDS = 0.3f;
	constexpr f32_t BADGE_STAGGER_SECONDS = 0.1f;
	/* MvpResultFrame does not put the badge list on the timeline -- RollingRepositionList
	   tweens each item in when the list is populated. The first badge becomes visible at
	   capture frame 1608, 3.49s after the award page starts. */
	constexpr f32_t BADGE_DELAY_SECONDS = 3.45f;

	/* TweenMax Back.easeOut with its default overshoot, which is what gives the badge
	   the slight past-and-back settle. */
	f32_t Back_EaseOut(const f32_t fT)
	{
		constexpr f32_t fOvershoot = 1.70158f;
		const f32_t fU = fT - 1.f;
		return 1.f + fU * fU * ((fOvershoot + 1.f) * fU + fOvershoot);
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
	m_bBadgeEffectStarted = false;
	m_pView->Set_AllSlotsVisible(true);
	/* Every authored image layer of MvpResultFrame plays from the extracted
		keyframe document, so its own per-frame transform drives them instead of a
		hand-written envelope. */
	m_pView->Play_KeyframeAnimation("MvpResult_Frame", "intro");
	for (const string& strMedalSlot : m_MedalSlotIds)
		m_pView->Set_SlotVisible(strMedalSlot, false);
	/* A replay would otherwise resume these mid-sparkle from the previous run. */
	for (size_t i = 0; i < MVP_MEDAL_MAX; ++i)
	{
		char szSlotId[64] = {};
		(void)sprintf_s(szSlotId, "MvpResult_BadgeEffect_%zu", i);
		m_pView->Set_SlotVisible(szSlotId, false);
	}
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

	/* The badge sparkles. RollingRepositionList arms one setTimeout of
	   tween_and_delay_totalTime = min(column, count) * delay and then calls
	   playAllEffect(), which does iconEffect.gotoAndPlay(2) on every item at once --
	   so these are not staggered the way the stamp-in is. Only MvpResult_BadgeListItem_L
	   owns an iconEffect; the party columns' small item has none. */
	const size_t iMvpBadges = (std::min)(m_Data.Mvp.Medals.size(), MVP_MEDAL_MAX);
	const f32_t fBadgeEffectStart =
		BADGE_DELAY_SECONDS + BADGE_STAGGER_SECONDS * static_cast<f32_t>(iMvpBadges);
	if (!m_bBadgeEffectStarted && 0u < iMvpBadges && m_fElapsedSeconds >= fBadgeEffectStart)
	{
		const f32_t fRowLeft = MVP_MEDAL_CENTER_LOCAL_X
			- MVP_MEDAL_PITCH_LOCAL * (static_cast<f32_t>(iMvpBadges) - 1.f) * 0.5f;
		for (size_t i = 0; i < iMvpBadges; ++i)
		{
			char szSlotId[64] = {};
			(void)sprintf_s(szSlotId, "MvpResult_BadgeEffect_%zu", i);
			/* The effect's own coordinates are relative to the badge item's origin,
			   which is the icon's top-left. */
			const f32_t fCenterLocalX = fRowLeft + MVP_MEDAL_PITCH_LOCAL * static_cast<f32_t>(i);
			m_pView->Set_SlotPosition(szSlotId,
				CanvasX(fCenterLocalX - MEDAL_ICON_LOCAL_MVP * 0.5f),
				CanvasY(MVP_MEDAL_CENTER_LOCAL_Y - MEDAL_ICON_LOCAL_MVP * 0.5f));
			m_pView->Set_SlotVisible(szSlotId, true);
			m_pView->Play_KeyframeAnimation(szSlotId, "play");
		}
		m_bBadgeEffectStarted = true;
	}

	/* The MVP render area slides left while it is already opaque. */
	const f32_t fSlide = Ramp(fFrame, MAIN_BG_SLIDE_START, MAIN_BG_SLIDE_END);
	const f32_t fMainLocalX =
		MAIN_BG_FROM_LOCAL_X + (MAIN_BG_TO_LOCAL_X - MAIN_BG_FROM_LOCAL_X) * fSlide;
	f32_t fSlotX = 0.f, fSlotY = 0.f, fSlotW = 0.f, fSlotH = 0.f;
	if (m_pView->Get_SlotRect("MvpResult_MainBackground", fSlotX, fSlotY, fSlotW, fSlotH))
		m_pView->Set_SlotPosition("MvpResult_MainBackground", CanvasX(fMainLocalX), fSlotY);
	/* Multiplier, not Set_SlotAlpha: that one rewrites the tint to white and drops the
	   authored colour. */
	m_pView->Set_SlotTintMultiplier("MvpResult_MainBackground", float4_t(1.f, 1.f, 1.f,
		Ramp(fFrame, MAIN_BG_FADE_START, MAIN_BG_FADE_END) * MAIN_BG_HOLD_ALPHA));
	/* The content-name flash is a one-shot that only belongs on screen from its own
	   frame; leaving the slot visible parks it on its last frame until then. */
	m_pView->Set_SlotVisible("MvpResult_SuccessBurst", fFrame >= SUCCESS_BURST_FRAME);

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
	const fvector_t vColor,
	const bool_t bLatinDisplay) const
{
	if (strText.empty())
		return;
	/* Same convention as CMainApp::RenderRaidClearText: authored 1280x720 coords are
	stretched per axis, while the glyph scale follows the smaller of the two so text
	never distorts. */
	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	const f32_t fScaleX = vViewport.x / 1280.f;
	const f32_t fScaleY = vViewport.y / 720.f;
	const f32_t fUiScale = (std::min)(fScaleX, fScaleY);
	/* Every label on this page used to be one family atlas resampled to size, which is
	why the small white rows turned to mush (16pt off a 42px bake is a 0.38x minify with
	no mips) and the 110pt title looked like a blown-up image (3.4x magnify off a 27px
	glyph). UILabelFont picks the variant baked nearest the on-screen line spacing and
	draws it 1:1 when one matches. */
	f32_t fGlyphScale = 1.f;
	const f32_t fLineSpacingPx = fFontPx * fUiScale;
	const wstring_t strFont = bLatinDisplay
		? UILabelFont::Resolve_LatinDisplay(pFontTag, fLineSpacingPx, fGlyphScale)
		: UILabelFont::Resolve(pFontTag, fLineSpacingPx, fGlyphScale);
	/* CCustomFont::Draw calls SpriteBatch::Begin() with no blend state, so DirectXTK
	uses its premultiplied CommonStates::AlphaBlend (SrcBlend ONE). A straight
	(rgb, a) colour would draw at full strength no matter what a is -- the whole page
	of labels would sit there from frame one. Premultiply so the fade actually fades. */
	const f32_t fAlpha = XMVectorGetW(vColor);
	const fvector_t vPremultiplied = XMVectorSetW(
		XMVectorScale(vColor, fAlpha), fAlpha);
	if (fAlpha <= 0.f)
		return;
	CGameInstance::Get().Draw_Text(strFont, strText.c_str(),
		float2_t(fCenterX * fScaleX, fCenterY * fScaleY),
		vPremultiplied, 0.f, vOrigin, fGlyphScale);
}

/* The headline is one centred line made of differently coloured pieces, so it
cannot go through Draw_Label: the whole line has to be measured first to find
where it starts, then each run drawn left to right from there. */
void Client::CMvpResultView::Draw_ContentName(
	const f32_t fCenterX,
	const f32_t fCenterY,
	const f32_t fFontPx,
	const f32_t fAlpha) const
{
	if (m_Data.ContentName.empty() || fAlpha <= 0.f)
		return;

	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	const f32_t fScaleX = vViewport.x / 1280.f;
	const f32_t fScaleY = vViewport.y / 720.f;
	const f32_t fUiScale = (std::min)(fScaleX, fScaleY);

	f32_t fGlyphScale = 1.f;
	const wstring_t strFont = UILabelFont::Resolve(
		FONT_YOON, fFontPx * fUiScale, fGlyphScale);

	/* One space of separation between pieces, the way the retail string reads. */
	const wstring_t strGap = TEXT(" ");
	const f32_t fGapWidth =
		CGameInstance::Get().Measure_Text(strFont, strGap.c_str()).x * fGlyphScale;

	f32_t fTotalWidth = 0.f;
	for (size_t i = 0; i < m_Data.ContentName.size(); ++i)
	{
		if (m_Data.ContentName[i].strText.empty())
			continue;
		if (fTotalWidth > 0.f)
			fTotalWidth += fGapWidth;
		fTotalWidth += CGameInstance::Get().Measure_Text(
			strFont, m_Data.ContentName[i].strText.c_str()).x * fGlyphScale;
	}

	f32_t fPenX = fCenterX * fScaleX - fTotalWidth * 0.5f;
	const f32_t fPenY = fCenterY * fScaleY;
	bool_t bFirst = true;
	for (const MVP_TEXT_RUN& Run : m_Data.ContentName)
	{
		if (Run.strText.empty())
			continue;
		if (!bFirst)
			fPenX += fGapWidth;
		bFirst = false;

		const fvector_t vColor = XMVectorSet(
			Run.vColor.x, Run.vColor.y, Run.vColor.z, fAlpha);
		const fvector_t vPremultiplied = XMVectorSetW(
			XMVectorScale(vColor, fAlpha), fAlpha);
		CGameInstance::Get().Draw_Text(strFont, Run.strText.c_str(),
			float2_t(fPenX, fPenY), vPremultiplied, 0.f,
			float2_t(0.f, 0.5f), fGlyphScale);
		fPenX += CGameInstance::Get().Measure_Text(
			strFont, Run.strText.c_str()).x * fGlyphScale;
	}
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
		XMVectorSetW(COLOR_WHITE, fPlateAlpha), true);

	/* contentNameTF and characterNameTF are host-substituted SimpleLabels, but their
	size and alignment are not lost with the symbol: MvpResultFrame's generated
	__setProp_* blocks carry them.
	  contentNameTF    size 24, color 0xFFFFFF, align centre / centre
	  characterNameTF  size 32, color 0xFFFFFF, align centre / top
	alignHorizontal "center" makes each PlaceObject translate that label's own box
	centre, so the two sit on their own x rather than a shared axis. */
	Draw_ContentName(
		CanvasX(MVP_CONTENT_LOCAL_X), CanvasY(MVP_CONTENT_LOCAL_Y),
		CanvasPt(MVP_CONTENT_POINTS), fPlateAlpha);
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
			XMVectorSetW(COLOR_WHITE, fStatAlpha), true);
	}

	Render_Medals(m_Data.Mvp.Medals, "Mvp", MVP_MEDAL_CENTER_LOCAL_X,
		MVP_MEDAL_CENTER_LOCAL_Y, MVP_MEDAL_PITCH_LOCAL, MEDAL_ICON_LOCAL_MVP,
		MVP_MEDAL_MAX, true);
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
		MEDAL_ICON_LOCAL_PARTY, PARTY_MEDAL_MAX, false);
}

void Client::CMvpResultView::Render_Medals(
	const vector<int32_t>& Medals,
	const char* const szOwner,
	const f32_t fCenterLocalX,
	const f32_t fCenterLocalY,
	const f32_t fPitchLocal,
	const f32_t fIconLocal,
	const size_t iMaxShown,
	const bool_t bStampIn) const
{
	if (m_fElapsedSeconds < BADGE_DELAY_SECONDS)
		return;

	const size_t iCount = (std::min)(Medals.size(), iMaxShown);
	for (size_t i = 0; i < iCount; ++i)
	{
		const int32_t iMedal = Medals[i];
		if (iMedal < MEDAL_INDEX_MIN || iMedal > MEDAL_INDEX_MAX)
			continue;

		/* The stamp-in. RollingRepositionList's preset 5 reads its numbers out of
		   mvpBadgeList.motionPreset[5] = "...,6,150,0,0.3,0.1,1": start at 6x scale,
		   150 above the resting spot and alpha 0, then TweenMax.to over 0.3s with
		   Back.easeOut, each item delayed by its own index * 0.1s. Only the MVP's
		   items carry it -- MvpResult_BadgeListItem_small has no iconEffect and the
		   party columns just appear. */
		const f32_t fItemStart = BADGE_DELAY_SECONDS +
			(bStampIn ? BADGE_STAGGER_SECONDS * static_cast<f32_t>(i) : 0.f);
		const f32_t fT = bStampIn
			? Ramp(m_fElapsedSeconds, fItemStart, fItemStart + BADGE_TWEEN_SECONDS)
			: 1.f;
		if (fT <= 0.f)
			continue;
		const f32_t fEased = bStampIn ? Back_EaseOut(fT) : 1.f;
		const f32_t fScale = BADGE_START_SCALE + (1.f - BADGE_START_SCALE) * fEased;
		const f32_t fRiseLocal = -BADGE_START_RISE_LOCAL * (1.f - fEased);

		char szSlotId[96] = {};
		(void)sprintf_s(szSlotId, "MvpResult_MedalIcon_%s_%zu", szOwner, i);
		char szTexture[64] = {};
		(void)sprintf_s(szTexture, "UI/MVP/MvpResult_Medal_%02d.png", iMedal);
		const f32_t fRowLeft = fCenterLocalX
			- fPitchLocal * (static_cast<f32_t>(iCount) - 1.f) * 0.5f;
		const f32_t fSlotCenterLocalX = fRowLeft + fPitchLocal * static_cast<f32_t>(i);
		const f32_t fDrawnLocal = fIconLocal * fScale;
		const f32_t fIconCanvas = fIconLocal * STAGE_TO_CANVAS;
		m_pView->Ensure_RuntimeSlot(szSlotId,
			CanvasX(fSlotCenterLocalX - fIconLocal * 0.5f),
			CanvasY(fCenterLocalY - fIconLocal * 0.5f),
			fIconCanvas, fIconCanvas, szTexture);
		/* Grows about its own centre, so the rect is rewritten every frame rather
		   than only positioned. */
		m_pView->Set_SlotRect(szSlotId,
			CanvasX(fSlotCenterLocalX - fDrawnLocal * 0.5f),
			CanvasY(fCenterLocalY + fRiseLocal - fDrawnLocal * 0.5f),
			fDrawnLocal * STAGE_TO_CANVAS, fDrawnLocal * STAGE_TO_CANVAS);
		m_pView->Set_SlotTintMultiplier(szSlotId, float4_t(1.f, 1.f, 1.f, fEased));
		m_pView->Set_SlotVisible(szSlotId, true);
		if (m_MedalSlotIds.end() ==
				std::find(m_MedalSlotIds.begin(), m_MedalSlotIds.end(), szSlotId))
			m_MedalSlotIds.push_back(szSlotId);
	}
}

#include "RaidEntryPreviewView.h"

#include "GameInstance.h"
#include "MainApp.h"
#include "RaidBossShowcaseService.h"
#include "UIInputRouter.h"
#include "UILayoutRuntime.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace
{
	/* Every slot in ValtanRaidEntry_Layout.json that carries real art. The rest are position-only
	   markers (authored tint alpha 0 -- the text boxes) whose text RenderText() draws at their
	   rect; they stay out of this list because there is nothing to show or hide. Set_SlotVisible
	   itself only moves the visibility flag, so the per-raid grade tints Apply_RaidSelection
	   writes survive the show pass below. */
	constexpr const char* MAIN_ART_SLOTS[] =
	{
		/* Every slot the rebuilt document gives real art, in the document's own order so this
		   list stays readable next to it. Rects come from epicgatecommanderentrance.gfx itself
		   (PlaceObject matrices for position, DefineSubImage for size) traced on its 1920x1080
		   canvas, then scaled by a uniform 2/3 onto this project's shared 1280x720 one.

		   The remaining position-only markers (title/subtitle/description, the left column
		   captions and the right panel's condition lines) are TextFields in the source --
		   RenderText draws those at their rect. */
		"RaidEntry_DimBackdrop",
		"RaidEntry_WindowBg",
		"RaidEntry_BossNameBar",
		"RaidEntry_TopBorderStrip",
		"RaidEntry_BossPortrait",
		"RaidEntry_TabSelPlate",
		"RaidEntry_TopThumb_0",
		"RaidEntry_TopThumb_1",
		"RaidEntry_TopThumb_2",
		"RaidEntry_TopThumb_3",
		"RaidEntry_TopTitleArt",
		"RaidEntry_TopThumb_4",
		"RaidEntry_TabSelName",
		"RaidEntry_TabSelGlow",
		"RaidEntry_CloseButtonSlot",
		"RaidEntry_RaidIconSlot",
		"RaidEntry_LeftPanelEmblem",
		"RaidEntry_EstherPanelBg",
		"RaidEntry_EstherFrame_0",
		"RaidEntry_EstherFrame_1",
		"RaidEntry_EstherFrame_2",
		"RaidEntry_EstherPortrait_0",
		"RaidEntry_EstherPortrait_1",
		"RaidEntry_EstherPortrait_2",
		"RaidEntry_RewardArrowLeft",
		"RaidEntry_RewardArrowRight",
		"RaidEntry_RewardSlotBg_0",
		"RaidEntry_RewardIcon_0",
		"RaidEntry_RewardSlotBorder_0",
		"RaidEntry_RewardSlotBg_1",
		"RaidEntry_RewardIcon_1",
		"RaidEntry_RewardSlotBorder_1",
		"RaidEntry_RewardSlotBg_2",
		"RaidEntry_RewardIcon_2",
		"RaidEntry_RewardSlotBorder_2",
		"RaidEntry_RewardSlotBg_3",
		"RaidEntry_RewardIcon_3",
		"RaidEntry_RewardSlotBorder_3",
		"RaidEntry_RewardSlotBg_4",
		"RaidEntry_RewardIcon_4",
		"RaidEntry_RewardSlotBorder_4",
		"RaidEntry_RewardSlotBg_5",
		"RaidEntry_RewardIcon_5",
		"RaidEntry_RewardSlotBorder_5",
		"RaidEntry_RewardSlotBg_6",
		"RaidEntry_RewardIcon_6",
		"RaidEntry_RewardSlotBorder_6",
		"RaidEntry_RewardSlotBg_7",
		"RaidEntry_RewardIcon_7",
		"RaidEntry_RewardSlotBorder_7",
		"RaidEntry_SoloBadgeSlot",
		"RaidEntry_TierBadge_0",
		"RaidEntry_GateLine_0",
		"RaidEntry_GateLine_1",
		"RaidEntry_GateIcon_0",
		"RaidEntry_GateIcon_1",
		"RaidEntry_GateIcon_2",
		"RaidEntry_ConditionHeaderButton",
		"RaidEntry_MatchingButton",
		"RaidEntry_FindPartyButton",
		"RaidEntry_EntranceButton",
	};

	/* Item grade backing. A flat tinted square read far too solid next to the icons; the item
	   upgrade window already solves the same problem with the client's own per-grade gradient
	   art plus one shared slot border, so the reward row uses those instead of inventing a
	   colour. Indexed by RAID_DEF::iRewardGrades in this file order. */
	const char_t* GRADE_BACKGROUNDS[] =
	{
		"UI/ItemUpgrade/buildup_gradebg_normal.png",
		"UI/ItemUpgrade/buildup_gradebg_elite.png",
		"UI/ItemUpgrade/buildup_gradebg_rare.png",
		"UI/ItemUpgrade/buildup_gradebg_epic.png",
		"UI/ItemUpgrade/buildup_gradebg_unique.png",
		"UI/ItemUpgrade/buildup_gradebg_legend.png",
		"UI/ItemUpgrade/buildup_gradebg_avatar.png",
	};

	/* One raid the tab strip can select. Only the two with real content are listed -- the other
	   tabs stay drawn but inert, since selecting them would show another raid's boss art and
	   rewards that nothing has been extracted for yet. */
	struct RAID_DEF
	{
		int32_t			iTabIndex;
		const wchar_t*	pRaidName;		// left panel headline
		const wchar_t*	pBossTitle;		// small line above the boss name
		const wchar_t*	pBossName;
		const wchar_t*	pGateLevels;	// 관문별 아이템 레벨 row
		const wchar_t*	pMatchLimit;	// 아이템 레벨 N 미만 매칭 불가
		/* Live showcase boss for this raid, rendered over the portrait by
		   CRaidBossShowcaseService -- nullptr keeps the static key art (Kakul's model is
		   still in production). */
		const char_t*	pShowcaseArchetypeId;
		const char_t*	pBossPortrait;
		/* leftBg is a six-frame clip in the source and the raid's emblem is painted into each
		   frame, so the left panel swaps as one texture rather than an emblem on a shared plate.
		   Only frame 1 carries an extra emblem strip on top of its own panel. */
		const char_t*	pLeftPanel;
		bool_t			hasEmblemStrip;
		const char_t*	pEstherPortraits[3];
		const char_t*	pRewardIcons[8];
		int32_t			iRewardGrades[8];
		/* Animated commander-entrance background, decrypted from the client's own Bink movie and
		   cooked to a DXT1 DDS flipbook, played on the portrait slot. pBgMoviePrefix is the frame
		   path minus the "_NNN.dds" suffix; nullptr keeps the static key art. */
		const char_t*	pBgMoviePrefix;
		int32_t			iBgMovieFrames;
	};
	const RAID_DEF RAID_DEFS[] =
	{
		{
			2,
			// "한밤중의 서커스" / "광기군단장" / "쿠크세이튼"
			L"\xD55C\xBC24\xC911\xC758 \xC11C\xCEE4\xC2A4",
			L"\xAD11\xAE30\xAD70\xB2E8\xC7A5",
			L"\xCFE0\xD06C\xC138\xC774\xD2BC",
			L"1475  1475  1475",
			// "아이템 레벨 1475 미만 매칭 불가"
			L"\xC544\xC774\xD15C \xB808\xBCA8 1475 \xBBF8\xB9CC \xB9E4\xCE6D \xBD88\xAC00",
			nullptr,
			"UI/Bern/RaidEntry_BossPortrait.png",
			"UI/Bern/RaidEntry_LeftPanel_2.png", false,
			/* Kakul's three esther skills, in the order the real window lists them.
			   esther_icon_3/4 are cut from EFUI_ICONATLAS_E/esther_0.dds; only the three
			   already on disk had names, so the rest keep their atlas index. */
			{ "UI/Esther/esther_icon_3.png",
			  "UI/Esther/esther_portrait_wei.png",
			  "UI/Esther/esther_icon_4.png" },
			{ "UI/ItemUpgrade/lm_head_icon.png", "UI/ItemUpgrade/lm_shoulder_icon.png",
			  "UI/ItemUpgrade/lm_top_icon.png", "UI/ItemUpgrade/lm_bottom_icon.png",
			  "UI/ItemUpgrade/lm_glove_icon.png", "UI/ItemUpgrade/lm_weapon_icon.png",
			  "UI/ItemUpgrade/lm_head_icon.png", "UI/ItemUpgrade/lm_weapon_icon.png" },
			{ 5, 5, 5, 5, 5, 5, 4, 4 },
			"UI/Bern/RaidEntry_BG_Kukusaton/RaidEntry_BG_Kukusaton", 300,
		},
		{
			0,
			// "부활한 마수의 심장" / "마수군단장" / "발탄"
			L"\xBD80\xD65C\xD55C \xB9C8\xC218\xC758 \xC2EC\xC7A5",
			L"\xB9C8\xC218\xAD70\xB2E8\xC7A5",
			L"\xBC1C\xD0C4",
			L"1415  1415  1415",
			// "아이템 레벨 1415 미만 매칭 불가"
			L"\xC544\xC774\xD15C \xB808\xBCA8 1415 \xBBF8\xB9CC \xB9E4\xCE6D \xBD88\xAC00",
			/* Valtan's own locked-state key art. It is absent from the entrance movie -- bossImage
			   only carries frames 3..6 -- but not from the game: EFUI_BACKGROUNDIMG ships it as
			   lv_lut_commander_valtan_lock at 1200x848, which is this very slot's size. The
			   package names the two states the movie switches between in
			   satisfiedChangedHandler: _lock for entry-blocked, _special for the colour art.
			   The live 3D showcase (BOSS_VALTAN) is bypassed now that the real entrance movie is
			   decrypted and cooked to pBgMoviePrefix -- set it back to "BOSS_VALTAN" to prefer the
			   live model over the movie. */
			nullptr,
			"UI/Bern/RaidEntry_BossPortrait_Valtan.png",
			"UI/Bern/RaidEntry_LeftPanel_0.png", true,
			{ "UI/Esther/esther_portrait_sillian.png",
			  "UI/Esther/esther_portrait_wei.png",
			  "UI/Esther/esther_portrait_bahuntur.png" },
			{ "UI/ItemUpgrade/lm_weapon_icon.png", "UI/ItemUpgrade/lm_head_icon.png",
			  "UI/ItemUpgrade/lm_top_icon.png", "UI/ItemUpgrade/lm_glove_icon.png",
			  "UI/ItemUpgrade/lm_shoulder_icon.png", "UI/ItemUpgrade/lm_bottom_icon.png",
			  "UI/ItemUpgrade/lm_weapon_icon.png", "UI/ItemUpgrade/lm_head_icon.png" },
			{ 4, 4, 4, 4, 4, 4, 3, 3 },
			"UI/Bern/RaidEntry_BG_Valtan/RaidEntry_BG_Valtan", 300,
		},
	};
	constexpr int32_t RAID_COUNT = static_cast<int32_t>(sizeof(RAID_DEFS) / sizeof(RAID_DEFS[0]));
	/* Tabs the document draws. Five, not the movie's six: the sixth commander has no content
	   here, so its slot was removed from the document. */
	constexpr int32_t TAB_COUNT = 5;
	/* How much the selected tab grows, anchored on its own bottom edge so it rises out of the
	   strip rather than spreading both ways. */
	constexpr f32_t TAB_SELECTED_GROW = 0.22f;
	/* Unselected tabs sit back; the selected one is at full brightness. */
	const float4_t TAB_IDLE_TINT{ 0.55f, 0.55f, 0.58f, 1.f };
	const float4_t TAB_SELECTED_TINT{ 1.f, 1.f, 1.f, 1.f };

	constexpr const char* CONFIRM_ART_SLOTS[] =
	{
		"ValtanEntry_Panel", "ValtanEntry_TitleTextBox", "ValtanEntry_DescTextBox",
		"ValtanEntry_ConfirmButton", "ValtanEntry_CancelButton",
		"ValtanEntry_AcceptIcon", "ValtanEntry_DeclineIcon",
	};
}

CRaidEntryPreviewView::CRaidEntryPreviewView(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	uint32_t iOwnerLevelIndex)
	: m_pView(std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, iOwnerLevelIndex, TEXT("Layer_UI"),
		L"UI/Bern/ValtanRaidEntry_Layout.json"))
	, m_pConfirmView(std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, iOwnerLevelIndex, TEXT("Layer_UI"),
		L"UI/Bern/BernValtanEntry_Layout.json"))
{
	/* A CUI_Sprite is visible from construction, unlike the old ImGui path that simply did not
	   draw while closed -- this popup starts closed. */
	Hide_AllSlots();
	Hide_ConfirmSlots();
}

CRaidEntryPreviewView::~CRaidEntryPreviewView() = default;

void CRaidEntryPreviewView::Hide_AllSlots()
{
	if (nullptr == m_pView)
		return;
	for (const char* pSlotId : MAIN_ART_SLOTS)
		m_pView->Set_SlotVisible(pSlotId, false);
}

void CRaidEntryPreviewView::Hide_ConfirmSlots()
{
	if (nullptr == m_pConfirmView)
		return;
	for (const char* pSlotId : CONFIRM_ART_SLOTS)
		m_pConfirmView->Set_SlotVisible(pSlotId, false);
}

void CRaidEntryPreviewView::Open()
{
	m_isOpen = true;
	m_hasJustOpened = true;
	Apply_RaidSelection();
	/* Pay the boss model's one synchronous admission behind the open transition, so the
	   first click on its tab doesn't hitch. */
	for (const RAID_DEF& Raid : RAID_DEFS)
		if (nullptr != Raid.pShowcaseArchetypeId)
			CRaidBossShowcaseService::Request_Prewarm(Raid.pShowcaseArchetypeId);
}

void CRaidEntryPreviewView::Apply_RaidSelection()
{
	if (nullptr == m_pView || m_iSelectedRaid < 0 || m_iSelectedRaid >= RAID_COUNT)
		return;

	const RAID_DEF& Raid = RAID_DEFS[m_iSelectedRaid];
	/* Commander-entrance movie on the portrait slot when this raid has one, else the static key
	   art. The 30 fps flipbook loops the ~10 s decrypted Bink clip; the frame textures stream in
	   through the runtime's texture cache and stay resident while the popup is open. */
	if (nullptr != Raid.pBgMoviePrefix && Raid.iBgMovieFrames > 0)
	{
		vector<string> BgFrames;
		BgFrames.reserve(Raid.iBgMovieFrames);
		char_t szFrame[256] = {};
		for (int32_t i = 0; i < Raid.iBgMovieFrames; ++i)
		{
			(void)sprintf_s(szFrame, "%s_%03d.dds", Raid.pBgMoviePrefix, i);
			BgFrames.emplace_back(szFrame);
		}
		m_pView->Set_SlotAnimation("RaidEntry_BossPortrait", BgFrames, 30.f, true);
	}
	else
	{
		m_pView->Set_SlotAnimation("RaidEntry_BossPortrait", {}, 0.f, true);
		m_pView->Set_SlotTexture("RaidEntry_BossPortrait", Raid.pBossPortrait);
	}
	m_pView->Set_SlotTexture("RaidEntry_RaidIconSlot", Raid.pLeftPanel);

	char_t szSlot[64] = {};
	for (int32_t i = 0; i < 3; ++i)
	{
		(void)sprintf_s(szSlot, "RaidEntry_EstherPortrait_%d", i);
		m_pView->Set_SlotTexture(szSlot, Raid.pEstherPortraits[i]);
	}
	for (int32_t i = 0; i < 8; ++i)
	{
		(void)sprintf_s(szSlot, "RaidEntry_RewardIcon_%d", i);
		m_pView->Set_SlotTexture(szSlot, Raid.pRewardIcons[i]);

		const int32_t iGrade = Raid.iRewardGrades[i];
		const int32_t iGradeCount =
			static_cast<int32_t>(sizeof(GRADE_BACKGROUNDS) / sizeof(GRADE_BACKGROUNDS[0]));
		(void)sprintf_s(szSlot, "RaidEntry_RewardSlotBg_%d", i);
		m_pView->Set_SlotTexture(szSlot,
			GRADE_BACKGROUNDS[(iGrade >= 0 && iGrade < iGradeCount) ? iGrade : 0]);
	}

	/* Restart the grow so the newly selected tab rises instead of appearing already large. */
	m_fTabGrow = 0.f;
}

void CRaidEntryPreviewView::Update_TabStrip(f32_t fTimeDelta)
{
	if (nullptr == m_pView)
		return;

	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();

	char_t szSlot[64] = {};
	if (!m_hasTabBaseRects)
	{
		for (int32_t i = 0; i < TAB_COUNT; ++i)
		{
			(void)sprintf_s(szSlot, "RaidEntry_TopThumb_%d", i);
			(void)m_pView->Get_SlotRect(szSlot, m_TabBaseRect[i][0], m_TabBaseRect[i][1],
				m_TabBaseRect[i][2], m_TabBaseRect[i][3]);
		}
		m_hasTabBaseRects = true;
	}

	const int32_t iSelectedTab = (m_iSelectedRaid >= 0 && m_iSelectedRaid < RAID_COUNT)
		? RAID_DEFS[m_iSelectedRaid].iTabIndex : -1;

	/* Only tabs backed by a RAID_DEF answer a click; the rest still draw so the strip reads
	   as the full roster, but selecting one would show content nothing has been extracted for. */
	for (int32_t iRaid = 0; iRaid < RAID_COUNT; ++iRaid)
	{
		const int32_t iTab = RAID_DEFS[iRaid].iTabIndex;
		if (iTab < 0 || iTab >= TAB_COUNT)
			continue;
		const f32_t* pBase = m_TabBaseRect[iTab];
		if (pBase[2] <= 0.f)
			continue;
		if (Router.Is_Clicked(pBase[0], pBase[1], pBase[2], pBase[3], fRefWidth, fRefHeight) &&
			iRaid != m_iSelectedRaid)
		{
			CMainApp::Play_UIButtonClickSound();
			m_iSelectedRaid = iRaid;
			Apply_RaidSelection();
			break;
		}
	}

	m_fTabGrow = (std::min)(1.f, m_fTabGrow + (std::max)(0.f, fTimeDelta) * 6.f);
	/* Ease-out so the grow settles instead of arriving at constant speed. */
	const f32_t fEased = 1.f - (1.f - m_fTabGrow) * (1.f - m_fTabGrow);

	/* Re-applied per frame, not in Apply_RaidSelection: Render()'s own blanket show pass turns
	   every art slot back on, so a one-shot hide here would not survive it. */
	m_pView->Set_SlotVisible("RaidEntry_LeftPanelEmblem",
		(m_iSelectedRaid >= 0 && m_iSelectedRaid < RAID_COUNT)
		&& RAID_DEFS[m_iSelectedRaid].hasEmblemStrip);

	f32_t fGlowRect[4] = {};
	for (int32_t i = 0; i < TAB_COUNT; ++i)
	{
		const f32_t* pBase = m_TabBaseRect[i];
		if (pBase[2] <= 0.f)
			continue;
		(void)sprintf_s(szSlot, "RaidEntry_TopThumb_%d", i);

		const bool_t isSelected = (i == iSelectedTab);
		m_pView->Set_SlotTint(szSlot, isSelected ? TAB_SELECTED_TINT : TAB_IDLE_TINT);

		const f32_t fGrow = isSelected ? (TAB_SELECTED_GROW * fEased) : 0.f;
		const f32_t fWidth = pBase[2] * (1.f + fGrow);
		const f32_t fHeight = pBase[3] * (1.f + fGrow);
		/* Anchored on the bottom edge and horizontal centre: the tab rises out of the strip
		   and keeps its own column, which is the movement the reference reads as. */
		const f32_t fLeft = pBase[0] - (fWidth - pBase[2]) * 0.5f;
		const f32_t fTop = pBase[1] + (pBase[3] - fHeight);
		m_pView->Set_SlotRect(szSlot, fLeft, fTop, fWidth, fHeight);

		if (isSelected)
		{
			fGlowRect[0] = fLeft; fGlowRect[1] = fTop;
			fGlowRect[2] = fWidth; fGlowRect[3] = fHeight;
		}
	}

	/* The selected tab's own chrome, straight out of the movie's selected_up frame: the plate
	   behind the thumbnail, the lit overlay across it, and the gold name plate under it. All
	   three are placed from the selected tab's live rect using the source's own local offsets
	   against its 139x79 thumbnail, so none of them has to be positioned by hand -- the rects
	   the document carries for them are placeholders. */
	struct SELECTED_PIECE
	{
		const char_t* pSlotId;
		f32_t fOffsetY;		// source-local y against the 139x79 thumbnail
		f32_t fWidth;
		f32_t fHeight;
		bool_t isLit;		// additive, and pulses
	};
	static constexpr f32_t SOURCE_THUMB_W = 139.f;
	static constexpr f32_t SOURCE_THUMB_H = 79.f;
	static constexpr SELECTED_PIECE SELECTED_PIECES[] =
	{
		{ "RaidEntry_TabSelPlate", -27.f, 169.f, 106.f, false },
		{ "RaidEntry_TabSelGlow",    0.f, 162.f,  78.f, true  },
		{ "RaidEntry_TabSelName",   51.f, 149.f,  28.f, true  },
	};

	m_fGlowPhase += (std::max)(0.f, fTimeDelta);
	const f32_t fPulse = 0.72f + 0.28f * sinf(m_fGlowPhase * 3.2f);

	for (const SELECTED_PIECE& Piece : SELECTED_PIECES)
	{
		if (iSelectedTab < 0 || fGlowRect[2] <= 0.f)
		{
			m_pView->Set_SlotVisible(Piece.pSlotId, false);
			continue;
		}
		const f32_t fScaleX = fGlowRect[2] / SOURCE_THUMB_W;
		const f32_t fScaleY = fGlowRect[3] / SOURCE_THUMB_H;
		m_pView->Set_SlotRect(Piece.pSlotId,
			fGlowRect[0], fGlowRect[1] + Piece.fOffsetY * fScaleY,
			Piece.fWidth * fScaleX, Piece.fHeight * fScaleY);
		/* Both lit pieces fade in with the grow so a fresh selection lights up instead of
		   popping, and keep breathing once it has settled. */
		m_pView->Set_SlotTint(Piece.pSlotId, Piece.isLit
			? float4_t(1.f, 1.f, 1.f, fPulse * fEased)
			: float4_t(1.f, 1.f, 1.f, fEased));
		m_pView->Set_SlotVisible(Piece.pSlotId, true);
	}
}

bool_t CRaidEntryPreviewView::Render()
{
	if (nullptr == m_pView)
		return false;

	if (!m_isOpen)
	{
		Hide_AllSlots();
		Hide_ConfirmSlots();
		m_hasJustOpened = false;
		return false;
	}

	/* Modal semantics: the pointer belongs to this popup for the whole frame it is open (its
	   own dim backdrop swallows clicks), the same thing BeginPopupModal did for free. */
	CUIInputRouter& Router = CUIInputRouter::Get();
	Router.Claim_Mouse_This_Frame();

	/* The opening click itself must not also count as a click on whatever sits under the
	   cursor inside the popup on that same frame. */
	const bool_t wasJustOpened = m_hasJustOpened;
	m_hasJustOpened = false;

	/* ImGui closed its modal on Escape; "닫기 (Esc)" is drawn on this screen, so that gesture
	   is reproduced explicitly now that no ImGui popup owns the key. */
	const bool_t isEscapeDown =
		GetForegroundWindow() == g_hWnd &&
		0 != (GetAsyncKeyState(VK_ESCAPE) & 0x8000);
	const bool_t escapePressed = isEscapeDown && !m_wasEscapeDown;
	m_wasEscapeDown = isEscapeDown;
	if (escapePressed)
	{
		m_isConfirmStepOpen = false;
		m_isOpen = false;
		Hide_AllSlots();
		Hide_ConfirmSlots();
		return false;
	}

	/* Entrance already clicked once -- the small second-step 수락/거절 dialog owns the screen
	   instead of the full one underneath it. */
	if (m_isConfirmStepOpen)
	{
		Hide_AllSlots();
		return Render_ConfirmStep();
	}
	Hide_ConfirmSlots();

	for (const char* pSlotId : MAIN_ART_SLOTS)
		m_pView->Set_SlotVisible(pSlotId, true);

	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();

	/* Timer_Default is the frame clock Client.cpp registers at startup; the grow is eased over
	   real time so it settles the same way regardless of framerate. */
	const f32_t fFrameDelta = CGameInstance::Get().Get_TimeDelta(TEXT("Timer_Default"));
	/* Advances the boss-portrait commander-movie flipbook (and any other animation.frames slot);
	   without this the swapped frame set would just hold on frame 0. */
	m_pView->Update(fFrameDelta);
	Update_TabStrip(fFrameDelta);

	/* The live boss render over the portrait area. Requested per frame; silence (popup
	   closed, other raid selected) lets the service retire itself. */
	if (m_iSelectedRaid >= 0 && m_iSelectedRaid < RAID_COUNT &&
		nullptr != RAID_DEFS[m_iSelectedRaid].pShowcaseArchetypeId)
	{
		CRaidBossShowcaseService::Request_Frame(
			RAID_DEFS[m_iSelectedRaid].pShowcaseArchetypeId);
	}

	/* The original switches bossImage (locked greyscale) and imageTexture (live) exclusively
	   in satisfiedChangedHandler -- mirror that: once the live boss is actually staged and
	   drawing, the static key art goes away instead of layering under it. */
	const bool_t isLiveBossShowing =
		m_iSelectedRaid >= 0 && m_iSelectedRaid < RAID_COUNT &&
		nullptr != RAID_DEFS[m_iSelectedRaid].pShowcaseArchetypeId &&
		CRaidBossShowcaseService::Is_Live();
	m_pView->Set_SlotVisible("RaidEntry_BossPortrait", !isLiveBossShowing);

	/* Matching / Find Party are visual-only per PLAN scope (no matching system exists yet) --
	   static art, no hover swap, no click handling. Decline always closes internally; Entrance
	   is reported back to the caller via this function's return value instead of sending any
	   command itself. */
	struct MODAL_BUTTON
	{
		const char* pSlotId;
		/* Empty reverts to the slot's own authored texture (the real Close button has no
		   distinct hover art -- the reference only ever swaps Entrance). */
		const char* pHoverTexturePath;
		bool_t isConfirm;
	};
	static constexpr MODAL_BUTTON BUTTONS[2] =
	{
		{ "RaidEntry_EntranceButton", "UI/Bern/RaidEntry_ButtonGoldHover.png", true },
		/* The reference has no separate "거절" text button on this screen -- only the
		   top-right "닫기 (Esc)" X closes it. */
		{ "RaidEntry_CloseButtonSlot", "", false },
	};

	bool_t confirmClicked = false;
	bool_t cancelClicked = false;
	for (const MODAL_BUTTON& button : BUTTONS)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pView->Get_SlotRect(button.pSlotId, fX, fY, fW, fH))
			continue;
		const bool_t isHovered =
			Router.Is_Hovered(fX, fY, fW, fH, fRefWidth, fRefHeight);
		m_pView->Set_SlotTexture(
			button.pSlotId, isHovered ? button.pHoverTexturePath : "");
		if (isHovered && !wasJustOpened &&
			Router.Is_Clicked(fX, fY, fW, fH, fRefWidth, fRefHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			if (button.isConfirm)
				confirmClicked = true;
			else
				cancelClicked = true;
		}
	}

	if (cancelClicked)
	{
		m_isOpen = false;
		Hide_AllSlots();
	}
	else if (confirmClicked)
	{
		// Opens the small 수락/거절 step on the next Render() call instead of
		// reporting Entrance back immediately -- see Render_ConfirmStep().
		m_isConfirmStepOpen = true;
	}

	return false;
}

void CRaidEntryPreviewView::RenderText()
{
	if (!m_isOpen)
		return;

	if (m_isConfirmStepOpen)
	{
		RenderText_ConfirmStep();
		return;
	}

	if (nullptr == m_pView)
		return;

	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vViewportSize.x / 1920.f;
	const float textScaleY = vViewportSize.y / 1080.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	/* Every offset and target height below is in the source movie's own 1920x1080 units, traced
	from epicgatecommanderentrance.gfx alongside the art placement. Get_SlotRect answers in the
	layout document's units instead, and those two agreed only while the document still carried
	that same canvas -- it has since been scaled onto this project's shared 1280x720 authoring
	canvas. Convert the rects back here rather than restating every authored number as a fraction
	of itself, which would leave the traced font sizes unreadable as the values they came from. */
	const f32_t fDocumentWidth = m_pView->Get_ResolutionWidth();
	const f32_t fDocumentHeight = m_pView->Get_ResolutionHeight();
	const f32_t fToAuthoredX = (fDocumentWidth > 0.f) ? (1920.f / fDocumentWidth) : 1.f;
	const f32_t fToAuthoredY = (fDocumentHeight > 0.f) ? (1080.f / fDocumentHeight) : 1.f;
	const auto Fn_SlotRect = [&](const char_t* pSlotId,
		f32_t& fX, f32_t& fY, f32_t& fW, f32_t& fH) -> bool_t
	{
		if (!m_pView->Get_SlotRect(pSlotId, fX, fY, fW, fH))
			return false;
		fX *= fToAuthoredX;
		fW *= fToAuthoredX;
		fY *= fToAuthoredY;
		fH *= fToAuthoredY;
		return true;
	};

	const auto Fn_DrawCentered = [&](f32_t fCenterX, f32_t fCenterY,
		const wchar_t* pLabel, f32_t fTargetHeight, const fvector_t& vColor)
	{
		const float2_t vMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pLabel);
		const f32_t fScale = (vMeasured.y > 0.f) ?
			(fTargetHeight / vMeasured.y) : 1.f;
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pLabel,
			float2_t(fCenterX * textScaleX, fCenterY * textScaleY),
			vColor, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
	};

	const RAID_DEF& Raid = RAID_DEFS[
		(m_iSelectedRaid >= 0 && m_iSelectedRaid < RAID_COUNT) ? m_iSelectedRaid : 0];
	/* Boss title, boss name and the reward heading all sit on the same vertical spine as the
	   ornament under them, so they share one centre rather than each slot's own. */
	const fvector_t vGold = XMVectorSet(1.f, 0.85f, 0.42f, 1.f);
	f32_t fSpineX = 0.f;
	{
		f32_t fOrnX = 0.f, fOrnY = 0.f, fOrnW = 0.f, fOrnH = 0.f;
		fSpineX = Fn_SlotRect("RaidEntry_TopTitleArt", fOrnX, fOrnY, fOrnW, fOrnH)
			? (fOrnX + fOrnW * 0.5f) : 0.f;
	}

	f32_t fTitleX = 0.f, fTitleY = 0.f, fTitleW = 0.f, fTitleH = 0.f;
	const bool_t hasTitle =
		Fn_SlotRect("RaidEntry_TitleTextBox", fTitleX, fTitleY, fTitleW, fTitleH);

	f32_t fSubtitleX = 0.f, fSubtitleY = 0.f, fSubtitleW = 0.f, fSubtitleH = 0.f;
	if (Fn_SlotRect(
		"RaidEntry_SubtitleTextBox", fSubtitleX, fSubtitleY, fSubtitleW, fSubtitleH))
	{
		/* Pulled down against the boss name instead of sitting at its slot's own centre --
		   the two read as one block in the reference, not two separate lines. */
		const f32_t fSubtitleCentreY = hasTitle
			? (fTitleY - fSubtitleH * 0.35f) : (fSubtitleY + fSubtitleH * 0.5f);
		Fn_DrawCentered(
			(fSpineX > 0.f) ? fSpineX : (fSubtitleX + fSubtitleW * 0.5f),
			fSubtitleCentreY, Raid.pBossTitle, 21.6f, vGold);
	}

	if (hasTitle)
	{
		Fn_DrawCentered(
			(fSpineX > 0.f) ? fSpineX : (fTitleX + fTitleW * 0.5f),
			fTitleY + fTitleH * 0.5f, Raid.pBossName, 48.f, vGold);
	}

	f32_t fDescX = 0.f, fDescY = 0.f, fDescW = 0.f, fDescH = 0.f;
	if (Fn_SlotRect(
		"RaidEntry_DescTextBox", fDescX, fDescY, fDescW, fDescH))
	{
		/* On the same spine as the boss name and the reward heading -- this line belongs to
		   that block, not to its own slot's centre. */
		// "필요 인원 4명"
		Fn_DrawCentered((fSpineX > 0.f) ? fSpineX : (fDescX + fDescW * 0.5f),
			fDescY + fDescH * 0.5f, L"\xD544\xC694 \xC778\xC6D0 4\xBA85",
			20.f, Colors::White);
	}

	/* Left-side raid identity stack: category label, raid name, "엔드 콘텐츠"
	   caption, and the esther-skill caption below the round raid icon --
	   read directly off the actual reference screenshot (2026-08-30 캡처),
	   not the epicraidentrance.gfx sprite tree: that tree's own exhaustive
	   top-level PlaceObject list (EpicRaidEntranceContent's 39 named
	   children) has no field matching this content at all, so the earlier
	   attempt to source it from gfx coordinates was chasing the wrong
	   sprite -- these lines are static raid metadata read straight from the
	   real screen, not a live/fabricated game-state value. */
	struct LEFT_LABEL
	{
		const char_t* pSlotId;
		const wchar_t* pLabel;
		f32_t fTargetHeight;
		bool_t isGold;
	};
	const LEFT_LABEL LEFT_LABELS[] =
	{
		// "군단장 레이드" -- the panel's headline, so it outsizes the raid name below it.
		{ "RaidEntry_AdaptLabelBox", L"\xAD70\xB2E8\xC7A5 \xB808\xC774\xB4DC", 36.f, false },
		// The selected raid's own name, in gold under that headline.
		{ "RaidEntry_GoldLabelBox", Raid.pRaidName, 21.6f, true },
		// "엔드 콘텐츠"
		{ "RaidEntry_EndContentCaptionBox", L"\xC5D4\xB4DC \xCF58\xD150\xCE20", 14.f, false },
		// "사용 가능한 에스더 스킬"
		{ "RaidEntry_EstherCaptionBox",
			L"\xC0AC\xC6A9 \xAC00\xB2A5\xD55C \xC5D0\xC2A4\xB354 \xC2A4\xD0AC", 19.2f, false },
	};
	for (const LEFT_LABEL& label : LEFT_LABELS)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!Fn_SlotRect(label.pSlotId, fX, fY, fW, fH))
			continue;
		Fn_DrawCentered(fX + fW * 0.5f, fY + fH * 0.5f, label.pLabel,
			label.fTargetHeight, label.isGold ? vGold : Colors::White);
	}

	/* Tier "3" / "솔로" badges, "보상 더보기" link, and the top-right close
	   label -- all simple text/number overlays on their own image slots. */
	struct ICON_LABEL
	{
		const char_t* pSlotId;
		const wchar_t* pLabel;
		f32_t fTargetHeight;
	};
	const ICON_LABEL ICON_LABELS[] =
	{
		{ "RaidEntry_TierNumberBadgeSlot", L"3", 22.f },
		// "보상 더보기 >"
		{ "RaidEntry_RewardMoreLinkBox", L"\xBCF4\xC0C1 \xB354\xBCF4\xAE30 >", 16.f },
		// "추천 스킬"
		{ "RaidEntry_RecommendSkillBox", L"\xCD94\xCC9C \xC2A4\xD0AC", 16.f },
	};
	for (const ICON_LABEL& label : ICON_LABELS)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!Fn_SlotRect(label.pSlotId, fX, fY, fW, fH))
			continue;
		Fn_DrawCentered(fX + fW * 0.5f, fY + fH * 0.5f, label.pLabel,
			label.fTargetHeight, Colors::White);
	}
	/* Right-side condition panel: heading, item-level display (honest dash placeholder --
	   no item-level system exists yet), the real, currently-true entry facts (party size
	   cap, single-gate boss), a restriction list matching the reference's own bullet
	   lines, the weekly-limit warning text, and the preset/gold-limit lines -- none of
	   these gate anything server-side yet, they mirror the reference's own static copy
	   per PLAN "남은 미해결 사항".

	   Each line owns a slot rather than an offset from the panel's rect. The offsets
	   these replace were measured against an earlier, larger panel and no longer agreed
	   with it, and more to the point a hardcoded offset cannot be moved in the layout
	   tool: the whole block could only travel together. Same for the two below. */
	struct PANEL_LINE
	{
		const char_t* pSlotId;
		const wchar_t* pLabel;
		f32_t fTargetHeight;
		bool_t isAccent;
		bool_t isWarning;
	};
	/* "나의 아이템 레벨 N" -- N comes from m_iMyItemLevel, not the literal, so equipment upgrade
	   can drive it through Set_MyItemLevel once that system exists. */
	wchar_t szMyItemLevel[64] = {};
	(void)swprintf_s(szMyItemLevel,
		L"\xB098\xC758 \xC544\xC774\xD15C \xB808\xBCA8 %d", m_iMyItemLevel);

	/* The three restriction lines and the gold-limit line read a step small against the rest of
	   the panel, so they carry their own 1.2x over the base sizes. */
	constexpr f32_t RESTRICTION_HEIGHT = 14.f * 1.2f;
	/* And they sit a touch high under the item-level row. */
	constexpr f32_t RESTRICTION_DROP = 6.f;

	const PANEL_LINE PANEL_LINES[] =
	{
		// "입장 조건"
		{ "RaidEntry_CondLine_0", L"\xC785\xC7A5 \xC870\xAC74", 24.f, false, false },
		// "관문별 아이템 레벨"
		{ "RaidEntry_CondLine_1",
			L"\xAD00\xBB38\xBCC4 \xC544\xC774\xD15C \xB808\xBCA8", 16.f, false, false },
		{ "RaidEntry_CondLine_2", Raid.pGateLevels, 18.f, false, false },
		{ "RaidEntry_CondLine_3", szMyItemLevel, 18.f, true, false },
		{ "RaidEntry_CondLine_4", Raid.pMatchLimit, RESTRICTION_HEIGHT, false, false },
		// "일반 물약 사용 불가"
		{ "RaidEntry_CondLine_5",
			L"\xC77C\xBC18 \xBB3C\xC57D \xC0AC\xC6A9 \xBD88\xAC00",
			RESTRICTION_HEIGHT, false, false },
		// "장비 변경 불가"
		{ "RaidEntry_CondLine_6",
			L"\xC7A5\xBE44 \xBCC0\xACBD \xBD88\xAC00", RESTRICTION_HEIGHT, false, false },
		// "주간 입장 횟수 초과"
		{ "RaidEntry_CondLine_7",
			L"\xC8FC\xAC04 \xC785\xC7A5 \xD69F\xC218 \xCD08\xACFC", 16.f, false, true },
		// "통합 프리셋 설정 가능"
		{ "RaidEntry_CondLine_8",
			L"\xD1B5\xD569 \xD504\xB9AC\xC14B \xC124\xC815 \xAC00\xB2A5", 14.f, false, false },
		// "주간 골드 획득 제한 (1/6)"
		{ "RaidEntry_CondLine_9",
			L"\xC8FC\xAC04 \xACE8\xB4DC \xD68D\xB4DD \xC81C\xD55C (1/6)",
			14.f * 1.2f, true, false },
		// "닫기 (Esc)"
		{ "RaidEntry_CloseLabelBox", L"\xB2EB\xAE30 (Esc)", 16.f, false, false },
	};
	for (const PANEL_LINE& Line : PANEL_LINES)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!Fn_SlotRect(Line.pSlotId, fX, fY, fW, fH))
			continue;
		const bool_t isRestriction =
			(Line.fTargetHeight == RESTRICTION_HEIGHT) && !Line.isAccent && !Line.isWarning;
		const fvector_t vColor = Line.isWarning
			? XMVectorSet(1.f, 0.55f, 0.4f, 1.f)
			: (Line.isAccent ? vGold : Colors::White);
		Fn_DrawCentered(fX + fW * 0.5f,
			fY + fH * 0.5f + (isRestriction ? RESTRICTION_DROP : 0.f),
			Line.pLabel, Line.fTargetHeight, vColor);
	}

	/* "기대 보상" shares the boss name's spine so it lines up with the ornament under it. */
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (Fn_SlotRect("RaidEntry_RewardTitleBox", fX, fY, fW, fH))
		{
			Fn_DrawCentered((fSpineX > 0.f) ? fSpineX : (fX + fW * 0.5f),
				fY + fH * 0.5f, L"\xAE30\xB300 \xBCF4\xC0C1", 20.f, Colors::White);
		}
	}

	/* Matching / Find Party are visual-only per PLAN scope -- still labelled
	   so the static art doesn't read as blank buttons. */
	struct STATIC_BUTTON_LABEL
	{
		const char_t* pSlotId;
		const wchar_t* pLabel;
	};
	const STATIC_BUTTON_LABEL STATIC_BUTTON_LABELS[] =
	{
		// "매칭 신청"
		{ "RaidEntry_MatchingButton", L"\xB9E4\xCE6D \xC2E0\xCCAD" },
		// "파티 찾기"
		{ "RaidEntry_FindPartyButton", L"\xD30C\xD2F0 \xCC3E\xAE30" },
	};
	for (const STATIC_BUTTON_LABEL& label : STATIC_BUTTON_LABELS)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!Fn_SlotRect(label.pSlotId, fX, fY, fW, fH))
			continue;
		Fn_DrawCentered(fX + fW * 0.5f, fY + fH * 0.5f, label.pLabel,
			fH * 0.48f, Colors::White);
	}

	/* "입장하기" on the gold button. This used to require a RaidEntry_AcceptIcon slot to offset
	   the label past its icon, and bailed out when that slot was missing -- which is why the
	   button read as blank once the icon was removed from the document. The icon is optional
	   now: with one the label centres on the space to its right, without one on the button. */
	{
		f32_t fButtonX = 0.f, fButtonY = 0.f, fButtonW = 0.f, fButtonH = 0.f;
		if (Fn_SlotRect("RaidEntry_EntranceButton", fButtonX, fButtonY, fButtonW, fButtonH))
		{
			f32_t fIconX = 0.f, fIconY = 0.f, fIconW = 0.f, fIconH = 0.f;
			const f32_t fLeft = Fn_SlotRect("RaidEntry_AcceptIcon", fIconX, fIconY, fIconW, fIconH)
				? (fIconX + fIconW) : fButtonX;
			Fn_DrawCentered(
				(fLeft + fButtonX + fButtonW) * 0.5f, fButtonY + fButtonH * 0.5f,
				L"\xC785\xC7A5\xD558\xAE30", fButtonH * 0.48f, Colors::White);
		}
	}
}

/* Second-step simple confirm dialog opened by the main screen's own Entrance
   button. Reuses Data/UI/Bern/BernValtanEntry_Layout.json (still on disk,
   pre-dates the rich screen) and the exact same panel/button/icon art as the
   original single-step Bern flow -- this is not a second runtime, just the
   pre-existing simple dialog surfaced one step later. */
bool_t CRaidEntryPreviewView::Render_ConfirmStep()
{
	if (nullptr == m_pConfirmView)
		return false;

	for (const char* pSlotId : CONFIRM_ART_SLOTS)
		m_pConfirmView->Set_SlotVisible(pSlotId, true);

	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pConfirmView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pConfirmView->Get_ResolutionHeight();

	struct CONFIRM_BUTTON
	{
		const char* pSlotId;
		bool_t isConfirm;
	};
	static constexpr CONFIRM_BUTTON BUTTONS[2] =
	{
		{ "ValtanEntry_ConfirmButton", true },
		{ "ValtanEntry_CancelButton", false },
	};
	bool_t confirmClicked = false;
	bool_t cancelClicked = false;
	for (const CONFIRM_BUTTON& button : BUTTONS)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pConfirmView->Get_SlotRect(button.pSlotId, fX, fY, fW, fH))
			continue;
		const bool_t isHovered =
			Router.Is_Hovered(fX, fY, fW, fH, fRefWidth, fRefHeight);
		m_pConfirmView->Set_SlotTexture(button.pSlotId, isHovered ?
			"UI/ClassSelect/Common/NormalButtonHover.png" : "");
		if (isHovered && Router.Is_Clicked(fX, fY, fW, fH, fRefWidth, fRefHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			if (button.isConfirm)
				confirmClicked = true;
			else
				cancelClicked = true;
		}
	}

	if (cancelClicked)
	{
		m_isConfirmStepOpen = false;
		m_isOpen = false;
		Hide_ConfirmSlots();
		return false;
	}
	if (confirmClicked)
	{
		m_isConfirmStepOpen = false;
		m_isOpen = false;
		Hide_ConfirmSlots();
		return true;
	}
	return false;
}

void CRaidEntryPreviewView::RenderText_ConfirmStep()
{
	if (nullptr == m_pConfirmView)
		return;

	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vViewportSize.x / 1280.f;
	const float textScaleY = vViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	const auto Fn_DrawCentered = [&](f32_t fCenterX, f32_t fCenterY,
		const wchar_t* pLabel, f32_t fTargetHeight, const fvector_t& vColor)
	{
		const float2_t vMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pLabel);
		const f32_t fScale = (vMeasured.y > 0.f) ?
			(fTargetHeight / vMeasured.y) : 1.f;
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pLabel,
			float2_t(fCenterX * textScaleX, fCenterY * textScaleY),
			vColor, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
	};

	f32_t fTitleX = 0.f, fTitleY = 0.f, fTitleW = 0.f, fTitleH = 0.f;
	if (m_pConfirmView->Get_SlotRect(
		"ValtanEntry_TitleTextBox", fTitleX, fTitleY, fTitleW, fTitleH))
	{
		// "레이드 입장"
		Fn_DrawCentered(fTitleX + fTitleW * 0.5f, fTitleY + fTitleH * 0.5f,
			L"\xB808\xC774\xB4DC \xC785\xC7A5", 24.f, Colors::White);
	}

	f32_t fDescX = 0.f, fDescY = 0.f, fDescW = 0.f, fDescH = 0.f;
	if (m_pConfirmView->Get_SlotRect(
		"ValtanEntry_DescTextBox", fDescX, fDescY, fDescW, fDescH))
	{
		// "부활한 마수의 심장으로 이동하시겠습니까?"
		Fn_DrawCentered(fDescX + fDescW * 0.5f, fDescY + fDescH * 0.5f,
			L"\xBD80\xD65C\xD55C \xB9C8\xC218\xC758 \xC2EC\xC7A5\xC73C\xB85C "
			L"\xC774\xB3D9\xD558\xC2DC\xACA0\xC2B5\xB2C8\xAE4C?",
			18.f, Colors::White);
	}

	struct CONFIRM_LABEL
	{
		const char_t* pButtonSlotId;
		const char_t* pIconSlotId;
		const wchar_t* pLabel;
	};
	const CONFIRM_LABEL LABELS[] =
	{
		{ "ValtanEntry_ConfirmButton", "ValtanEntry_AcceptIcon", L"\xC218\xB77D" }, // 수락
		{ "ValtanEntry_CancelButton", "ValtanEntry_DeclineIcon", L"\xAC70\xC808" }, // 거절
	};
	for (const CONFIRM_LABEL& label : LABELS)
	{
		f32_t fButtonX = 0.f, fButtonY = 0.f, fButtonW = 0.f, fButtonH = 0.f;
		f32_t fIconX = 0.f, fIconY = 0.f, fIconW = 0.f, fIconH = 0.f;
		if (!m_pConfirmView->Get_SlotRect(
				label.pButtonSlotId, fButtonX, fButtonY, fButtonW, fButtonH) ||
			!m_pConfirmView->Get_SlotRect(
				label.pIconSlotId, fIconX, fIconY, fIconW, fIconH))
		{
			continue;
		}
		const f32_t fIconRight = fIconX + fIconW;
		const f32_t fButtonRight = fButtonX + fButtonW;
		Fn_DrawCentered(
			(fIconRight + fButtonRight) * 0.5f, fButtonY + fButtonH * 0.5f,
			label.pLabel, fButtonH * 0.48f, Colors::White);
	}
}

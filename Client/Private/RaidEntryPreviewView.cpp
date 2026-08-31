#include "RaidEntryPreviewView.h"

#include "GameInstance.h"
#include "MainApp.h"
#include "UIInputRouter.h"
#include "UILayoutRuntime.h"

#include <algorithm>

namespace
{
	/* Every slot in ValtanRaidEntry_Layout.json that carries real art. The rest are
	   position-only markers (authored tint alpha 0 -- boss portrait, text boxes, tier badges,
	   warning box, nav arrows, top border strip) whose text, where any, RenderText() draws at
	   their rect. Those must never be passed to Set_SlotVisible: showing a slot resets its tint
	   to opaque white, which would turn each marker into a white box. */
	constexpr const char* MAIN_ART_SLOTS[] =
	{
		"RaidEntry_Vignette",
		"RaidEntry_EstherFrame_0", "RaidEntry_EstherPortrait_0",
		"RaidEntry_EstherFrame_1", "RaidEntry_EstherPortrait_1",
		"RaidEntry_EstherFrame_2", "RaidEntry_EstherPortrait_2",
		/* TopRight carries flipX in the document itself now -- the old manual draw path never
		   consulted the JSON layer's own flipX and mirrored via swapped UVs in code instead. */
		"RaidEntry_PanelFrame_TopLeft", "RaidEntry_PanelFrame_TopRight",
		"RaidEntry_TopThumb_0", "RaidEntry_TopThumb_1", "RaidEntry_TopThumb_2",
		"RaidEntry_TopThumb_3", "RaidEntry_TopThumb_4", "RaidEntry_TopThumb_5",
		"RaidEntry_TopThumb_6",
		"RaidEntry_RewardIcon_0", "RaidEntry_RewardIcon_1", "RaidEntry_RewardIcon_2",
		"RaidEntry_RewardIcon_3", "RaidEntry_RewardIcon_4", "RaidEntry_RewardIcon_5",
		"RaidEntry_RewardIcon_6", "RaidEntry_RewardIcon_7",
		"RaidEntry_MatchingButton", "RaidEntry_FindPartyButton",
		"RaidEntry_EntranceButton", "RaidEntry_AcceptIcon", "RaidEntry_CloseButtonSlot",
	};

	/* Authored tint of RaidEntry_DimBackdrop -- a black wash, not a white sprite, so it is shown
	   with Set_SlotTint rather than Set_SlotVisible(true)'s opaque white. */
	const float4_t DIM_BACKDROP_TINT{ 0.f, 0.f, 0.f, 0.72f };

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
	m_pView->Set_SlotTint("RaidEntry_DimBackdrop", float4_t(0.f, 0.f, 0.f, 0.f));
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

	m_pView->Set_SlotTint("RaidEntry_DimBackdrop", DIM_BACKDROP_TINT);
	for (const char* pSlotId : MAIN_ART_SLOTS)
		m_pView->Set_SlotVisible(pSlotId, true);

	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();

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

	/* Content pivoted from "발탄" to "쿠크세이튼" ("한밤중의 서커스") on request --
	   the team's already bringing in real Kakul-Saydon map resources
	   (LV_LUT_MIDNIGHTC_ED), so this screen previews that raid's own identity
	   instead of Valtan's, even though the boss portrait itself is still a
	   mood-only placeholder (no real Kakul key art has been extracted yet). */
	/* Real coordinates for both lines, traced from monsterInfo's own children
	   in the root sprite (titleTxt local (68,728), commanderNameTxt local
	   (68,759), monsterInfo itself at root (0,0) -- so these y values are the
	   real absolute ones). X centering (rather than the real field's own
	   left-anchored x=68) is kept since the real textField's width/alignment
	   isn't recoverable from PlaceObject data alone, and centered clearly
	   matches the reference screenshot. */
	f32_t fSubtitleX = 0.f, fSubtitleY = 0.f, fSubtitleW = 0.f, fSubtitleH = 0.f;
	if (m_pView->Get_SlotRect(
		"RaidEntry_SubtitleTextBox", fSubtitleX, fSubtitleY, fSubtitleW, fSubtitleH))
	{
		// "광기군단장"
		Fn_DrawCentered(fSubtitleX + fSubtitleW * 0.5f, fSubtitleY + fSubtitleH * 0.5f,
			L"\xAD11\xAE30\xAD70\xB2E8\xC7A5", 18.f, Colors::White);
	}

	f32_t fTitleX = 0.f, fTitleY = 0.f, fTitleW = 0.f, fTitleH = 0.f;
	if (m_pView->Get_SlotRect(
		"RaidEntry_TitleTextBox", fTitleX, fTitleY, fTitleW, fTitleH))
	{
		// "쿠크세이튼"
		Fn_DrawCentered(fTitleX + fTitleW * 0.5f, fTitleY + fTitleH * 0.5f,
			L"\xCFE0\xD06C\xC138\xC774\xD2BC",
			40.f, Colors::White);
	}

	f32_t fDescX = 0.f, fDescY = 0.f, fDescW = 0.f, fDescH = 0.f;
	if (m_pView->Get_SlotRect(
		"RaidEntry_DescTextBox", fDescX, fDescY, fDescW, fDescH))
	{
		// "필요 인원 4명"
		Fn_DrawCentered(fDescX + fDescW * 0.5f, fDescY + fDescH * 0.5f,
			L"\xD544\xC694 \xC778\xC6D0 4\xBA85",
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
	};
	const LEFT_LABEL LEFT_LABELS[] =
	{
		// "군단장 레이드"
		{ "RaidEntry_AdaptLabelBox", L"\xAD70\xB2E8\xC7A5 \xB808\xC774\xB4DC", 16.f },
		// "한밤중의 서커스"
		{ "RaidEntry_GoldLabelBox", L"\xD55C\xBC24\xC911\xC758 \xC11C\xCEE4\xC2A4", 26.f },
		// "엔드 콘텐츠"
		{ "RaidEntry_EndContentCaptionBox", L"\xC5D4\xB4DC \xCF58\xD150\xCE20", 14.f },
		// "사용 가능한 에스더 스킬"
		{ "RaidEntry_EstherCaptionBox",
			L"\xC0AC\xC6A9 \xAC00\xB2A5\xD55C \xC5D0\xC2A4\xB354 \xC2A4\xD0AC", 16.f },
	};
	for (const LEFT_LABEL& label : LEFT_LABELS)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pView->Get_SlotRect(label.pSlotId, fX, fY, fW, fH))
			continue;
		Fn_DrawCentered(fX + fW * 0.5f, fY + fH * 0.5f, label.pLabel,
			label.fTargetHeight, Colors::White);
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
		// "솔로"
		{ "RaidEntry_SoloBadgeSlot", L"\xC194\xB85C", 13.f },
		// "보상 더보기 >"
		{ "RaidEntry_RewardMoreLinkBox", L"\xBCF4\xC0C1 \xB354\xBCF4\xAE30 >", 16.f },
		// "추천 스킬"
		{ "RaidEntry_RecommendSkillBox", L"\xCD94\xCC9C \xC2A4\xD0AC", 16.f },
	};
	for (const ICON_LABEL& label : ICON_LABELS)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pView->Get_SlotRect(label.pSlotId, fX, fY, fW, fH))
			continue;
		Fn_DrawCentered(fX + fW * 0.5f, fY + fH * 0.5f, label.pLabel,
			label.fTargetHeight, Colors::White);
	}
	// "닫기 (Esc)" -- sits just left of RaidEntry_CloseButtonSlot's X icon.
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (m_pView->Get_SlotRect("RaidEntry_CloseButtonSlot", fX, fY, fW, fH))
		{
			Fn_DrawCentered(fX - 70.f, fY + fH * 0.5f,
				L"\xB2EB\xAE30 (Esc)", 16.f, Colors::White);
		}
	}

	/* Right-side condition panel: heading, the 3 tier badges drawn above,
	   item-level display (honest dash placeholder -- no item-level system
	   exists yet), the real, currently-true entry facts (party size cap,
	   single-gate boss), a restriction list matching the reference's own
	   bullet lines, the weekly-limit warning box's text, and the
	   preset/gold-limit lines -- none of these gate anything server-side yet,
	   they mirror the reference's own static copy per PLAN "남은 미해결 사항". */
	f32_t fCondX = 0.f, fCondY = 0.f, fCondW = 0.f, fCondH = 0.f;
	if (m_pView->Get_SlotRect(
		"RaidEntry_ConditionTextBox", fCondX, fCondY, fCondW, fCondH))
	{
		const f32_t fCenterX = fCondX + fCondW * 0.5f;
		// "입장 조건"
		Fn_DrawCentered(fCenterX, fCondY + 30.f,
			L"\xC785\xC7A5 \xC870\xAC74", 24.f, Colors::White);
		// "관문별 아이템 레벨" -- below the 3 tier badges (fixed at y=300..364)
		Fn_DrawCentered(fCenterX, fCondY + 150.f,
			L"\xAD00\xBB38\xBCC4 \xC544\xC774\xD15C \xB808\xBCA8", 16.f, Colors::White);
		// "1475  1475  1475"
		Fn_DrawCentered(fCenterX, fCondY + 175.f,
			L"1475  1475  1475", 18.f, Colors::White);
		// "나의 아이템 레벨 1556"
		Fn_DrawCentered(fCenterX, fCondY + 205.f,
			L"\xB098\xC758 \xC544\xC774\xD15C \xB808\xBCA8 1556",
			18.f, XMVectorSet(1.f, 0.85f, 0.4f, 1.f));
		// "아이템 레벨 1475 미만 매칭 불가"
		Fn_DrawCentered(fCenterX, fCondY + 240.f,
			L"\xC544\xC774\xD15C \xB808\xBCA8 1475 \xBBF8\xB9CC \xB9E4\xCE6D \xBD88\xAC00",
			14.f, Colors::White);
		// "일반 물약 사용 불가"
		Fn_DrawCentered(fCenterX, fCondY + 264.f,
			L"\xC77C\xBC18 \xBB3C\xC57D \xC0AC\xC6A9 \xBD88\xAC00", 14.f, Colors::White);
		// "장비 변경 불가"
		Fn_DrawCentered(fCenterX, fCondY + 288.f,
			L"\xC7A5\xBE44 \xBCC0\xACBD \xBD88\xAC00", 14.f, Colors::White);
		// "주간 입장 횟수 초과" -- centered inside RaidEntry_WarningBoxSlot
		// (fixed at y=560, height 60).
		Fn_DrawCentered(fCenterX, fCondY + 345.f,
			L"\xC8FC\xAC04 \xC785\xC7A5 \xD69F\xC218 \xCD08\xACFC",
			16.f, XMVectorSet(1.f, 0.55f, 0.4f, 1.f));
		// "통합 프리셋 설정 가능"
		Fn_DrawCentered(fCenterX, fCondY + 410.f,
			L"\xD1B5\xD569 \xD504\xB9AC\xC14B \xC124\xC815 \xAC00\xB2A5",
			14.f, Colors::White);
		// "주간 골드 획득 제한 (1/6)"
		Fn_DrawCentered(fCenterX, fCondY + 438.f,
			L"\xC8FC\xAC04 \xACE8\xB4DC \xD68D\xB4DD \xC81C\xD55C (1/6)",
			14.f, Colors::White);
	}

	f32_t fRewardX = 0.f, fRewardY = 0.f, fRewardW = 0.f, fRewardH = 0.f;
	if (m_pView->Get_SlotRect(
		"RaidEntry_RewardPanel", fRewardX, fRewardY, fRewardW, fRewardH))
	{
		// "기대 보상"
		Fn_DrawCentered(fRewardX + fRewardW * 0.5f, fRewardY + 18.f,
			L"\xAE30\xB300 \xBCF4\xC0C1", 20.f, Colors::White);
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
		// "매칭"
		{ "RaidEntry_MatchingButton", L"\xB9E4\xCE6D" },
		// "파티 찾기"
		{ "RaidEntry_FindPartyButton", L"\xD30C\xD2F0 \xCC3E\xAE30" },
	};
	for (const STATIC_BUTTON_LABEL& label : STATIC_BUTTON_LABELS)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pView->Get_SlotRect(label.pSlotId, fX, fY, fW, fH))
			continue;
		Fn_DrawCentered(fX + fW * 0.5f, fY + fH * 0.5f, label.pLabel,
			fH * 0.48f, Colors::White);
	}

	/* Label sits to the right of the icon inside the same button, not centered
	   on the whole button -- Get_SlotRect gives both rects so no hardcoded
	   offset is needed. */
	struct MODAL_BUTTON_LABEL
	{
		const char_t* pButtonSlotId;
		const char_t* pIconSlotId;
		const wchar_t* pLabel;
	};
	const MODAL_BUTTON_LABEL BUTTON_LABELS[] =
	{
		// "입장하기"
		{ "RaidEntry_EntranceButton", "RaidEntry_AcceptIcon", L"\xC785\xC7A5\xD558\xAE30" },
	};
	for (const MODAL_BUTTON_LABEL& Label : BUTTON_LABELS)
	{
		f32_t fButtonX = 0.f, fButtonY = 0.f, fButtonW = 0.f, fButtonH = 0.f;
		f32_t fIconX = 0.f, fIconY = 0.f, fIconW = 0.f, fIconH = 0.f;
		if (!m_pView->Get_SlotRect(
				Label.pButtonSlotId, fButtonX, fButtonY, fButtonW, fButtonH) ||
			!m_pView->Get_SlotRect(
				Label.pIconSlotId, fIconX, fIconY, fIconW, fIconH))
		{
			continue;
		}
		const f32_t fIconRight = fIconX + fIconW;
		const f32_t fButtonRight = fButtonX + fButtonW;
		Fn_DrawCentered(
			(fIconRight + fButtonRight) * 0.5f, fButtonY + fButtonH * 0.5f,
			Label.pLabel, fButtonH * 0.48f, Colors::White);
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

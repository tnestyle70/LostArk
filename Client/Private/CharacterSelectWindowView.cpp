#include "CharacterSelectWindowView.h"

#include "GameInstance.h"
#include "MainApp.h"
#include "UIInputRouter.h"
#include "UILayoutRuntime.h"

#include <algorithm>
#include <cstdio>

namespace
{
	/* Every slot in CharacterSelectWindow_Layout.json that carries real art, in document
	   order. The two *TextBox markers (authored tint alpha 0) stay out -- RenderText only
	   reads their rects. */
	constexpr const char* MAIN_ART_SLOTS[] =
	{
		"CharSel_Wallpaper",
		"CharSel_FooterBg",
		"CharSel_CardSlot_0",
		"CharSel_CardSlot_1",
		"CharSel_CardSlot_2",
		"CharSel_CardSlot_3",
		"CharSel_CardSlot_4",
		"CharSel_CardSlot_5",
		"CharSel_StartButton",
		"CharSel_ServerBackIcon",
	};

	constexpr int32_t CARD_COUNT = 6;

	/* "게임 시작" has no selectable character yet (all six slots are empty), so its plate
	   sits dimmed and ignores clicks -- the source's own disabled state is a darker plate
	   of the same art. */
	const float4_t START_DISABLED_TINT{ 0.45f, 0.45f, 0.45f, 1.f };
}

CCharacterSelectWindowView::CCharacterSelectWindowView(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	uint32_t iOwnerLevelIndex)
	: m_pView(std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, iOwnerLevelIndex, TEXT("Layer_UI"),
		L"UI/CharacterSelect/CharacterSelectWindow_Layout.json"))
{
	/* A CUI_Sprite is visible from construction; this window starts closed. */
	Hide_AllSlots();
}

CCharacterSelectWindowView::~CCharacterSelectWindowView() = default;

void CCharacterSelectWindowView::Hide_AllSlots()
{
	if (nullptr == m_pView)
		return;
	for (const char* pSlotId : MAIN_ART_SLOTS)
		m_pView->Set_SlotVisible(pSlotId, false);
}

void CCharacterSelectWindowView::Open()
{
	m_isOpen = true;
	m_hasJustOpened = true;
	m_eIntent = INTENT::NONE;
}

void CCharacterSelectWindowView::Close()
{
	m_isOpen = false;
	m_iHoveredCard = -1;
	Hide_AllSlots();
}

CCharacterSelectWindowView::INTENT CCharacterSelectWindowView::Consume_Intent()
{
	const INTENT eIntent = m_eIntent;
	m_eIntent = INTENT::NONE;
	return eIntent;
}

void CCharacterSelectWindowView::Update(f32_t fTimeDelta)
{
	(void)fTimeDelta;
	if (nullptr == m_pView)
		return;

	if (!m_isOpen)
	{
		Hide_AllSlots();
		m_hasJustOpened = false;
		return;
	}

	/* Modal semantics: the pointer belongs to this window for the whole frame it is
	   open (its wallpaper covers the screen), so the Lobby buttons underneath never
	   see hover or click. */
	CUIInputRouter& Router = CUIInputRouter::Get();
	Router.Claim_Mouse_This_Frame();

	const bool_t wasJustOpened = m_hasJustOpened;
	m_hasJustOpened = false;

	/* ESC returns to the Lobby (the future server-select screen), mirroring the
	   source's own exit gesture. Edge-detected with this view's own state. */
	const bool_t isEscapeDown =
		GetForegroundWindow() == g_hWnd &&
		0 != (GetAsyncKeyState(VK_ESCAPE) & 0x8000);
	const bool_t escapePressed = isEscapeDown && !m_wasEscapeDown;
	m_wasEscapeDown = isEscapeDown;
	if (escapePressed)
	{
		m_eIntent = INTENT::CLOSE;
		return;
	}

	for (const char* pSlotId : MAIN_ART_SLOTS)
		m_pView->Set_SlotVisible(pSlotId, true);

	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();

	/* Empty-slot cards: hover swaps to the source's lit addSlotMc art; a click is the
	   original's RequestNewCharacter -- report it and let CMainApp submit the Lobby
	   command (approval failure keeps the Lobby status line, not this view). */
	m_iHoveredCard = -1;
	char_t szSlot[64] = {};
	for (int32_t i = 0; i < CARD_COUNT; ++i)
	{
		(void)sprintf_s(szSlot, "CharSel_CardSlot_%d", i);
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pView->Get_SlotRect(szSlot, fX, fY, fW, fH))
			continue;
		const bool_t isHovered =
			Router.Is_Hovered(fX, fY, fW, fH, fRefWidth, fRefHeight);
		m_pView->Set_SlotTexture(szSlot, isHovered ?
			"UI/CharacterSelect/charsel_card_new_hover.png" : "");
		if (!isHovered)
			continue;
		m_iHoveredCard = i;
		if (!wasJustOpened &&
			Router.Is_Clicked(fX, fY, fW, fH, fRefWidth, fRefHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			m_eIntent = INTENT::NEW_CHARACTER;
		}
	}

	/* 게임 시작 -- disabled while no character exists: dimmed plate, no hover art,
	   clicks ignored. */
	m_pView->Set_SlotTintMultiplier("CharSel_StartButton", START_DISABLED_TINT);

	/* 서버 선택 (back). */
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (m_pView->Get_SlotRect("CharSel_ServerBackIcon", fX, fY, fW, fH))
		{
			/* The label under the icon clicks too -- one merged hit rect covering both. */
			f32_t fLX = fX, fLY = fY, fLW = fW, fLH = fH;
			f32_t fBX = 0.f, fBY = 0.f, fBW = 0.f, fBH = 0.f;
			if (m_pView->Get_SlotRect("CharSel_ServerBackLabelBox", fBX, fBY, fBW, fBH))
			{
				fLX = (std::min)(fX, fBX);
				fLY = (std::min)(fY, fBY);
				fLW = (std::max)(fX + fW, fBX + fBW) - fLX;
				fLH = (std::max)(fY + fH, fBY + fBH) - fLY;
			}
			const bool_t isHovered =
				Router.Is_Hovered(fLX, fLY, fLW, fLH, fRefWidth, fRefHeight);
			m_pView->Set_SlotTexture("CharSel_ServerBackIcon", isHovered ?
				"UI/CharacterSelect/charsel_server_back_icon_hover.png" : "");
			if (isHovered && !wasJustOpened &&
				Router.Is_Clicked(fLX, fLY, fLW, fLH, fRefWidth, fRefHeight))
			{
				CMainApp::Play_UIButtonClickSound();
				m_eIntent = INTENT::CLOSE;
			}
		}
	}
}

void CCharacterSelectWindowView::RenderText()
{
	if (!m_isOpen || nullptr == m_pView)
		return;

	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	const f32_t fScaleX = vViewportSize.x / m_pView->Get_ResolutionWidth();
	const f32_t fScaleY = vViewportSize.y / m_pView->Get_ResolutionHeight();
	const f32_t fUiScale = (std::min)(fScaleX, fScaleY);

	const auto DrawCentered = [&](const wchar_t* pText, f32_t fX, f32_t fY,
		f32_t fW, f32_t fH, f32_t fTextHeightRatio, FXMVECTOR vColor)
	{
		const float2_t vMeasured = CGameInstance::Get().Measure_Text(
			TEXT("Font_YoonGasiIIM"), pText);
		const f32_t fScaleByHeight = (vMeasured.y > 0.f) ?
			(fH * fTextHeightRatio / vMeasured.y) : 1.f;
		const f32_t fScaleByWidth = (vMeasured.x > 0.f) ?
			(fW * 0.92f / vMeasured.x) : 1.f;
		const f32_t fScale = (std::min)(fScaleByHeight, fScaleByWidth);
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pText,
			float2_t((fX + fW * 0.5f) * fScaleX, (fY + fH * 0.5f) * fScaleY),
			vColor, 0.f, float2_t(0.5f, 0.5f), fScale * fUiScale);
	};

	f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;

	// "신규 캐릭터 생성" on every empty-slot card; the hovered card's label lights up
	// with its art.
	char_t szSlot[64] = {};
	for (int32_t i = 0; i < CARD_COUNT; ++i)
	{
		(void)sprintf_s(szSlot, "CharSel_CardSlot_%d", i);
		if (!m_pView->Get_SlotRect(szSlot, fX, fY, fW, fH))
			continue;
		const bool_t isHovered = (i == m_iHoveredCard);
		DrawCentered(L"\xC2E0\xADDC \xCE90\xB9AD\xD130 \xC0DD\xC131",
			fX, fY, fW, fH, 0.30f,
			isHovered ?
				XMVectorSet(1.f, 235.f / 255.f, 170.f / 255.f, 1.f) :
				XMVectorSet(200.f / 255.f, 200.f / 255.f, 205.f / 255.f, 1.f));
	}

	// "게임 시작" -- dimmed with its disabled plate.
	if (m_pView->Get_SlotRect("CharSel_StartButton", fX, fY, fW, fH))
	{
		DrawCentered(L"\xAC8C\xC784 \xC2DC\xC791", fX, fY, fW, fH, 0.42f,
			XMVectorSet(150.f / 255.f, 140.f / 255.f, 120.f / 255.f, 1.f));
	}

	// "서버 선택" under the back arrow.
	if (m_pView->Get_SlotRect("CharSel_ServerBackLabelBox", fX, fY, fW, fH))
	{
		DrawCentered(L"\xC11C\xBC84 \xC120\xD0DD", fX, fY, fW, fH, 0.75f,
			XMVectorSet(220.f / 255.f, 220.f / 255.f, 225.f / 255.f, 1.f));
	}

	// "0 / 6" slot count at the card bar's right edge, like the reference's "5/6".
	if (m_pView->Get_SlotRect("CharSel_SlotCountTextBox", fX, fY, fW, fH))
	{
		DrawCentered(L"0 / 6", fX, fY, fW, fH, 0.75f,
			XMVectorSet(190.f / 255.f, 190.f / 255.f, 195.f / 255.f, 1.f));
	}
}

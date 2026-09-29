#include "SystemMenuButtonsView.h"

#include "GameInstance.h"
#include "MainApp.h"
#include "UIInputRouter.h"
#include "UILabelFont.h"
#include "UILayoutRuntime.h"

namespace
{
	struct BUTTON
	{
		const char* pSlotId;
		Client::CSystemMenuButtonsView::INTENT eIntent;
		const wchar_t* pLabel;
	};
	/* Left to right: character select, options ("Character Select" / "Options" as wide escapes). */
	constexpr BUTTON BUTTONS[] = {
		{ "SMB_CharacterSelect", Client::CSystemMenuButtonsView::INTENT::RETURN_TO_CHARACTER_SELECT,
			L"\xCE90\xB9AD\xD130 \xC120\xD0DD" },
		{ "SMB_Options", Client::CSystemMenuButtonsView::INTENT::OPEN_OPTIONS,
			L"\xD658\xACBD\xC124\xC815" },
	};
	constexpr f32_t LABEL_PX = 12.f;	// reference pixels, scaled with the viewport height
	constexpr f32_t LABEL_GAP = 4.f;	// between the icon's bottom edge and the caption
	const wstring_t LABEL_FONT = TEXT("Font_YoonGasiIIM");
	/* The icons are flat grey art with no hover frame of their own, so hover brightens them. */
	const float4_t TINT_NORMAL = float4_t(1.f, 1.f, 1.f, 1.f);
	const float4_t TINT_HOVER = float4_t(1.45f, 1.45f, 1.45f, 1.f);
}

void Client::CSystemMenuButtonsView::Initialize(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iOwnerLevelIndex)
{
	m_pView = std::make_unique<CUILayoutRuntime>(pDevice, pContext, iOwnerLevelIndex,
		TEXT("Layer_UI"), L"UI/SystemMenu/SystemMenuButtons_Layout.json");
}

Client::CSystemMenuButtonsView::INTENT Client::CSystemMenuButtonsView::Update(const bool_t bVisible)
{
	m_bPointerOver = false;
	if (nullptr == m_pView)
		return INTENT::NONE;
	if (bVisible != m_bShown)
	{
		for (const BUTTON& Button : BUTTONS)
			m_pView->Set_SlotVisible(Button.pSlotId, bVisible);
		m_bShown = bVisible;
	}
	if (!bVisible)
		return INTENT::NONE;

	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	INTENT eIntent = INTENT::NONE;
	for (const BUTTON& Button : BUTTONS)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pView->Get_SlotRect(Button.pSlotId, fX, fY, fW, fH))
			continue;
		/* A window or modal that already took the pointer this frame keeps it. */
		const bool_t bHovered = !Router.Is_MouseClaimedThisFrame() &&
			Router.Is_Hovered(fX, fY, fW, fH, fRefWidth, fRefHeight);
		m_pView->Set_SlotTintMultiplier(Button.pSlotId, bHovered ? TINT_HOVER : TINT_NORMAL);
		if (!bHovered)
			continue;
		m_bPointerOver = true;
		if (INTENT::NONE == eIntent && Router.Is_Clicked(fX, fY, fW, fH, fRefWidth, fRefHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			eIntent = Button.eIntent;
		}
	}
	return eIntent;
}

void Client::CSystemMenuButtonsView::Render_Text() const
{
	if (nullptr == m_pView || !m_bShown)
		return;
	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	if (vViewport.x <= 0.f || vViewport.y <= 0.f || fRefWidth <= 0.f || fRefHeight <= 0.f)
		return;
	const f32_t fScaleX = vViewport.x / fRefWidth;
	const f32_t fScaleY = vViewport.y / fRefHeight;
	for (const BUTTON& Button : BUTTONS)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pView->Get_SlotRect(Button.pSlotId, fX, fY, fW, fH))
			continue;
		(void)UILabelFont::Draw_Centered(LABEL_FONT, Button.pLabel,
			(fX + fW * 0.5f) * fScaleX, (fY + fH + LABEL_GAP + LABEL_PX * 0.5f) * fScaleY,
			LABEL_PX * fScaleY, DirectX::Colors::White);
	}
}

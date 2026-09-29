#include "SystemMenuButtonsView.h"

#include "MainApp.h"
#include "UIInputRouter.h"
#include "UILayoutRuntime.h"

namespace
{
	struct BUTTON
	{
		const char* pSlotId;
		Client::CSystemMenuButtonsView::INTENT eIntent;
	};
	constexpr BUTTON BUTTONS[] = {
		{ "SMB_Options", Client::CSystemMenuButtonsView::INTENT::OPEN_OPTIONS },
		{ "SMB_CharacterSelect", Client::CSystemMenuButtonsView::INTENT::RETURN_TO_CHARACTER_SELECT },
	};
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

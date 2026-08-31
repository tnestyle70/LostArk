#include "PartyInteractionView.h"

#include "Character.h"
#include "GameInstance.h"
#include "MainApp.h"
#include "PlayerCommandSink.h"
#include "PlayerController.h"
#include "PartyTransferNotice.h"
#include "RuntimeAssetRoot.h"
#include "Transform.h"
#include "UIInputRouter.h"
#include "UILayoutRuntime.h"

#include <cfloat>
#include <filesystem>

namespace
{
	/* Same MultiByteToWideChar pattern as Level_ValtanArena.cpp's own file-local
	   ConvertUtf8ToWide (this project already has that conversion duplicated per-file rather
	   than shared, e.g. WorldPlayerNameplateView.cpp's own Try_ConvertUtf8) -- for
	   m_strContextMenuTargetNickname (real player nickname, UTF-8) since
	   CGameInstance::Draw_Text needs a wide string. */
	bool_t Convert_NicknameToWide(const std::string& strUtf8, std::wstring& outWide)
	{
		outWide.clear();
		if (strUtf8.empty())
			return false;
		const int iRequiredLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			strUtf8.data(), static_cast<int>(strUtf8.size()), nullptr, 0);
		if (iRequiredLength <= 0)
			return false;
		std::wstring staged(static_cast<size_t>(iRequiredLength), L'\0');
		if (iRequiredLength != MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			strUtf8.data(), static_cast<int>(strUtf8.size()), staged.data(), iRequiredLength))
		{
			return false;
		}
		outWide = std::move(staged);
		return true;
	}
}

void Client::CPartyInteractionView::Initialize(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iOwnerLevelIndex)
{
	m_pInviteView = std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, iOwnerLevelIndex, TEXT("Layer_UI"),
		L"UI/Party/PartyInviteConfirm_Layout.json");
	m_pInviteView->Set_SlotVisible("PartyInvite_Panel", false);
	m_pInviteView->Set_SlotVisible("PartyInvite_ConfirmButton", false);
	m_pInviteView->Set_SlotVisible("PartyInvite_CancelButton", false);
	m_pInviteView->Set_SlotVisible("PartyInvite_TitleTextBox", false);
	m_pInviteView->Set_SlotVisible("PartyInvite_DescTextBox", false);
	m_pInviteView->Set_SlotVisible("PartyInvite_AcceptIcon", false);
	m_pInviteView->Set_SlotVisible("PartyInvite_DeclineIcon", false);

	/* Migrated off CHUDRuntimeView/ImGui -- real CUI_Sprite GameObjects on the calling Level's
	   own "Layer_UI" (see .md/TJ/08-31/2026-08-31_ImGui_런타임UI_전환_PLAN.md). Hidden at
	   construction since a CUI_Sprite defaults to fully visible/opaque, unlike the old ImGui
	   path which simply never drew anything outside BeginPopupModal. */
	m_pContextMenuView = std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, iOwnerLevelIndex, TEXT("Layer_UI"),
		L"UI/Party/PartyContextMenu_Layout.json");
	m_pContextMenuView->Set_SlotVisible("PartyContextMenu_Panel", false);
	m_pContextMenuView->Set_SlotVisible("PartyContextMenu_HoverHighlight", false);

	/* Captured once, before either slot ever moves -- see this member's own declaration
	   comment for why re-reading this at open time would be wrong after the first open. */
	f32_t fPanelX = 0.f, fPanelY = 0.f, fPanelW = 0.f, fPanelH = 0.f;
	f32_t fHighlightX = 0.f, fHighlightY = 0.f, fHighlightW = 0.f, fHighlightH = 0.f;
	if (m_pContextMenuView->Get_SlotRect(
			"PartyContextMenu_Panel", fPanelX, fPanelY, fPanelW, fPanelH) &&
		m_pContextMenuView->Get_SlotRect(
			"PartyContextMenu_HoverHighlight", fHighlightX, fHighlightY, fHighlightW, fHighlightH))
	{
		m_fContextMenuHighlightOffsetX = fHighlightX - fPanelX;
		m_fContextMenuHighlightOffsetY = fHighlightY - fPanelY;
	}

	/* Same first-draw-invisible issue CLevel_Bern's Valtan-entry popup had --
	   see Level_Bern::Initialize's own warm-up comment. A bare space doesn't
	   actually warm anything (DirectXTK's SpriteFont::DrawString skips the
	   SpriteBatch::Draw() call for a whitespace glyph, so Begin()/End() ran
	   with nothing queued) -- a real non-degenerate glyph is required. */
	CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), L"\xD30C\xD2F0\xCD08\xB300",
		float2_t(-1000.f, -1000.f), Colors::White, 0.f,
		float2_t(0.5f, 0.5f), 1.f);
}

bool_t Client::CPartyInteractionView::Update(
	CClientReplication& Replication,
	const std::shared_ptr<IPlayerCommandSink>& pCommandSink,
	const std::vector<REPLICATED_PLAYER_VIEW>& OtherPlayers,
	const bool_t worldInteractionAllowed)
{
	UNREFERENCED_PARAMETER(pCommandSink);
	LostArk::Shared::S2C_PARTY_TRANSFER_RESULT transferResult{};
	if (Replication.Try_Consume_PartyTransferResult(transferResult))
	{
		const wchar_t* notice = Get_PartyTransferFailureText(transferResult.eResult);
		m_strTransferFailureNotice = nullptr != notice ? notice : L"";
		m_TransferNoticeExpiresAt = std::chrono::steady_clock::now() +
			std::chrono::seconds(8);
	}

	LostArk::Shared::S2C_PARTY_INVITE_RECEIVED received{};
	if (Replication.Try_Consume_PartyInviteReceived(received))
	{
		m_PendingInvite = received;
		m_isInvitePopupOpen = true;
		m_hasInvitePopupJustOpened = true;

		const std::filesystem::path soundPath = CRuntimeAssetRoot::Resolve(
			L"Sound/UI/System/sys_party_request1__654410772.wav");
		CGameInstance::Get().Play_Sound(soundPath.wstring(), 1.f);
	}

	(void)Update_ContextMenuTrigger(OtherPlayers, worldInteractionAllowed);
	return m_hasContextMenuTarget || m_isInvitePopupOpen;
}

bool_t Client::CPartyInteractionView::Update_ContextMenuTrigger(
	const std::vector<REPLICATED_PLAYER_VIEW>& OtherPlayers,
	const bool_t worldInteractionAllowed)
{
	const bool_t isRightMouseDown =
		0 != (CGameInstance::Get().Get_DIMouseStateRaw(DIM::RB) & 0x80);
	const bool_t isRightMousePressed =
		isRightMouseDown && !m_wasRightMouseDown;
	m_wasRightMouseDown = isRightMouseDown;

	if (!worldInteractionAllowed || !isRightMousePressed ||
		m_hasContextMenuTarget || m_isInvitePopupOpen ||
		0 == (CGameInstance::Get().Get_DIMouseState(DIM::RB) & 0x80))
		return false;

	vector_t rayOrigin{}, rayDirection{};
	if (!CPlayerController::Try_PickWorldRay(rayOrigin, rayDirection))
		return false;
	rayDirection = XMVector3Normalize(rayDirection);

	/* Distance-independent: a player click has to work no matter how far
	   away the target is on screen, unlike the Valtan-entry NPC click which
	   is deliberately close-range only. CHARACTER_PICK_RADIUS is a generous
	   clickable capsule radius around the character's own root position. */
	constexpr f32_t CHARACTER_PICK_RADIUS = 1.2f;
	f32_t fBestRayParameter = FLT_MAX;
	const REPLICATED_PLAYER_VIEW* pHit = nullptr;
	for (const REPLICATED_PLAYER_VIEW& player : OtherPlayers)
	{
		if (player.isLocal)
			continue;
		const std::shared_ptr<CCharacter> pCharacter = player.pCharacter.lock();
		if (nullptr == pCharacter)
			continue;
		const std::shared_ptr<Engine::CTransform> pTransform =
			pCharacter->Get_Transform();
		if (nullptr == pTransform)
			continue;

		const vector_t vCharacterPos = pTransform->Get_State(STATE::POSITION);
		const f32_t fRayParameter = XMVectorGetX(XMVector3Dot(
			XMVectorSubtract(vCharacterPos, rayOrigin), rayDirection));
		if (fRayParameter < 0.f)
			continue;

		const vector_t vClosestPoint = XMVectorAdd(
			rayOrigin, XMVectorScale(rayDirection, fRayParameter));
		const f32_t fDistanceSq = XMVectorGetX(XMVector3LengthSq(
			XMVectorSubtract(vCharacterPos, vClosestPoint)));
		if (fDistanceSq > CHARACTER_PICK_RADIUS * CHARACTER_PICK_RADIUS)
			continue;

		/* Nearest-to-camera hit wins when characters overlap on screen. */
		if (fRayParameter < fBestRayParameter)
		{
			fBestRayParameter = fRayParameter;
			pHit = &player;
		}
	}
	if (nullptr == pHit)
		return false;

	/* Native cursor position (GetCursorPos/ScreenToClient), not ImGui::GetMousePos -- matches
	   Try_PickWorldRay's own source a few lines above and CUIInputRouter's, since this class no
	   longer depends on ImGui for its context menu. Converted to the menu document's own
	   reference-resolution units (Get_SlotRect/Set_SlotPosition's unit) via the same viewport
	   scale CUIInputRouter derives internally. */
	::POINT cursor{};
	if (!GetCursorPos(&cursor) || !ScreenToClient(g_hWnd, &cursor))
		return false;
	if (nullptr == m_pContextMenuView)
		return false;
	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	const f32_t fResolutionWidth = m_pContextMenuView->Get_ResolutionWidth();
	const f32_t fResolutionHeight = m_pContextMenuView->Get_ResolutionHeight();
	if (vViewportSize.x <= 0.f || vViewportSize.y <= 0.f ||
		fResolutionWidth <= 0.f || fResolutionHeight <= 0.f)
	{
		return false;
	}
	m_fContextMenuScreenX = static_cast<f32_t>(cursor.x) * (fResolutionWidth / vViewportSize.x);
	m_fContextMenuScreenY = static_cast<f32_t>(cursor.y) * (fResolutionHeight / vViewportSize.y);

	m_pContextMenuView->Set_SlotPosition(
		"PartyContextMenu_Panel", m_fContextMenuScreenX, m_fContextMenuScreenY);
	m_pContextMenuView->Set_SlotPosition(
		"PartyContextMenu_HoverHighlight",
		m_fContextMenuScreenX + m_fContextMenuHighlightOffsetX,
		m_fContextMenuScreenY + m_fContextMenuHighlightOffsetY);
	m_pContextMenuView->Set_SlotVisible("PartyContextMenu_Panel", true);
	m_pContextMenuView->Set_SlotVisible("PartyContextMenu_HoverHighlight", true);

	m_iContextMenuTargetNetEntityId = pHit->iNetEntityId;
	m_strContextMenuTargetNickname = pHit->strNickname;
	m_hasContextMenuTarget = true;
	m_hasContextMenuJustOpened = true;
	return true;
}

void Client::CPartyInteractionView::Render(
	const std::shared_ptr<IPlayerCommandSink>& pCommandSink)
{
	Render_ContextMenu(pCommandSink);
	Render_InvitePopup(pCommandSink);
}

void Client::CPartyInteractionView::Render_ContextMenu(
	const std::shared_ptr<IPlayerCommandSink>& pCommandSink)
{
	if (nullptr == m_pContextMenuView)
		return;

	/* Captured before clearing -- the outside-right-click-close check below must not fire on
	   this same frame the menu opened on (opening is also a right-click, so
	   Router.Is_RightClickEdge() would otherwise read true the instant the menu appears and
	   close it immediately). */
	const bool_t wasJustOpenedThisFrame = m_hasContextMenuJustOpened;
	m_hasContextMenuJustOpened = false;

	if (!m_hasContextMenuTarget)
		return;

	/* Panel (Name Box, shows the target's nickname) and Button (Black box, the actual
	   "파티초대" invite action) already sit at wherever Update_ContextMenuTrigger's
	   Set_SlotPosition calls moved them (the click point + their authored relative offset) --
	   this function only needs their current rects for hover/click, not any repositioning. */
	f32_t fPanelX = 0.f, fPanelY = 0.f, fPanelW = 0.f, fPanelH = 0.f;
	f32_t fButtonX = 0.f, fButtonY = 0.f, fButtonW = 0.f, fButtonH = 0.f;
	if (!m_pContextMenuView->Get_SlotRect(
			"PartyContextMenu_Panel", fPanelX, fPanelY, fPanelW, fPanelH) ||
		!m_pContextMenuView->Get_SlotRect(
			"PartyContextMenu_HoverHighlight",
			fButtonX, fButtonY, fButtonW, fButtonH))
	{
		m_hasContextMenuTarget = false;
		return;
	}

	const f32_t fResolutionWidth = m_pContextMenuView->Get_ResolutionWidth();
	const f32_t fResolutionHeight = m_pContextMenuView->Get_ResolutionHeight();
	CUIInputRouter& Router = CUIInputRouter::Get();
	const bool_t isButtonHovered = Router.Is_Hovered(
		fButtonX, fButtonY, fButtonW, fButtonH, fResolutionWidth, fResolutionHeight);

	/* Same darken-when-not-hovered feedback as the old IM_COL32(210,210,210,255) tint. */
	m_pContextMenuView->Set_SlotTint("PartyContextMenu_HoverHighlight",
		isButtonHovered ?
			float4_t(1.f, 1.f, 1.f, 1.f) :
			float4_t(210.f / 255.f, 210.f / 255.f, 210.f / 255.f, 1.f));

	bool_t closeMenu = false;
	if (isButtonHovered)
	{
		/* Is_Hovered(button) above already true, so this call's own internal hover check
		   claims the mouse for this frame regardless of whether the click edge fires too --
		   the menu keeps owning input while the cursor sits over its button. */
		if (Router.Is_Clicked(
				fButtonX, fButtonY, fButtonW, fButtonH, fResolutionWidth, fResolutionHeight))
		{
			if (nullptr != pCommandSink)
			{
				pCommandSink->Request_PartyInvite(
					m_iNextRequestSequence++, m_iContextMenuTargetNetEntityId);
			}
			CMainApp::Play_UIButtonClickSound();
			closeMenu = true;
		}
	}
	else
	{
		/* Still owns the mouse while open even when hovering neither its own button nor
		   nothing at all -- matches the old BeginPopupModal's own always-capture behavior. */
		Router.Claim_Mouse_This_Frame();

		/* Right-click anywhere outside the menu's own art (Panel ∪ HoverHighlight) dismisses
		   it without sending an invite -- the real dismiss gesture, not a left click.
		   wasJustOpenedThisFrame excludes the opening right-click itself, which would
		   otherwise read as "right-click outside" on this exact frame (the cursor is over the
		   clicked character in the 3D world, not this 2D menu rect) and instantly re-close the
		   menu the same frame it opened. */
		const f32_t fMenuX = (std::min)(fPanelX, fButtonX);
		const f32_t fMenuY = (std::min)(fPanelY, fButtonY);
		const f32_t fMenuRight = (std::max)(fPanelX + fPanelW, fButtonX + fButtonW);
		const f32_t fMenuBottom = (std::max)(fPanelY + fPanelH, fButtonY + fButtonH);
		const bool_t isMenuHovered = Router.Is_Hovered(
			fMenuX, fMenuY, fMenuRight - fMenuX, fMenuBottom - fMenuY,
			fResolutionWidth, fResolutionHeight);
		if (!wasJustOpenedThisFrame && !isMenuHovered && Router.Is_RightClickEdge())
			closeMenu = true;
	}

	if (closeMenu)
	{
		m_hasContextMenuTarget = false;
		m_pContextMenuView->Set_SlotVisible("PartyContextMenu_Panel", false);
		m_pContextMenuView->Set_SlotVisible("PartyContextMenu_HoverHighlight", false);
	}
}

void Client::CPartyInteractionView::Render_InvitePopup(
	const std::shared_ptr<IPlayerCommandSink>& pCommandSink)
{
	if (nullptr == m_pInviteView)
		return;

	if (m_hasInvitePopupJustOpened)
	{
		static constexpr const char* SLOTS[7] =
		{
			"PartyInvite_Panel", "PartyInvite_ConfirmButton", "PartyInvite_CancelButton",
			"PartyInvite_TitleTextBox", "PartyInvite_DescTextBox",
			"PartyInvite_AcceptIcon", "PartyInvite_DeclineIcon",
		};
		for (const char* pSlotId : SLOTS)
			m_pInviteView->Set_SlotVisible(pSlotId, true);
		m_hasInvitePopupJustOpened = false;
	}

	if (!m_isInvitePopupOpen)
		return;

	const f32_t fResolutionWidth = m_pInviteView->Get_ResolutionWidth();
	const f32_t fResolutionHeight = m_pInviteView->Get_ResolutionHeight();
	CUIInputRouter& Router = CUIInputRouter::Get();
	Router.Claim_Mouse_This_Frame();

	struct MODAL_BUTTON
	{
		const char* pSlotId;
		bool_t isConfirm;
	};
	static constexpr MODAL_BUTTON BUTTONS[2] =
	{
		{ "PartyInvite_ConfirmButton", true },
		{ "PartyInvite_CancelButton", false },
	};

	bool_t acceptClicked = false;
	bool_t declineClicked = false;
	for (const MODAL_BUTTON& button : BUTTONS)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pInviteView->Get_SlotRect(button.pSlotId, fX, fY, fW, fH))
			continue;
		const bool_t isHovered = Router.Is_Hovered(
			fX, fY, fW, fH, fResolutionWidth, fResolutionHeight);
		m_pInviteView->Set_SlotTexture(button.pSlotId, isHovered ?
			"UI/ClassSelect/Common/NormalButtonHover.png" :
			"UI/ClassSelect/Common/NormalButton.png");
		if (isHovered && Router.Is_Clicked(fX, fY, fW, fH, fResolutionWidth, fResolutionHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			if (button.isConfirm)
				acceptClicked = true;
			else
				declineClicked = true;
		}
	}

	if (acceptClicked || declineClicked)
	{
		if (nullptr != pCommandSink)
		{
			pCommandSink->Request_PartyInviteRespond(
				m_iNextRequestSequence++,
				m_PendingInvite.iFromNetEntityId,
				acceptClicked);
		}
		if (acceptClicked)
		{
			const std::filesystem::path soundPath = CRuntimeAssetRoot::Resolve(
				L"Sound/UI/System/sys_party_request_accept1__233892976.wav");
			CGameInstance::Get().Play_Sound(soundPath.wstring(), 1.f);
		}
		m_isInvitePopupOpen = false;
		static constexpr const char* SLOTS[7] =
		{
			"PartyInvite_Panel", "PartyInvite_ConfirmButton", "PartyInvite_CancelButton",
			"PartyInvite_TitleTextBox", "PartyInvite_DescTextBox",
			"PartyInvite_AcceptIcon", "PartyInvite_DeclineIcon",
		};
		for (const char* pSlotId : SLOTS)
			m_pInviteView->Set_SlotVisible(pSlotId, false);
	}
}

void Client::CPartyInteractionView::Render_InvitePopupText()
{
	if (!m_strTransferFailureNotice.empty() &&
		std::chrono::steady_clock::now() < m_TransferNoticeExpiresAt)
	{
		const float2_t viewport = CGameInstance::Get().Get_ViewportSize();
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"),
			m_strTransferFailureNotice.c_str(), float2_t(viewport.x * 0.5f, viewport.y * 0.2f),
			Colors::Orange, 0.f, float2_t(0.5f, 0.5f), viewport.y / 1080.f);
	}
	if (!m_isInvitePopupOpen || nullptr == m_pInviteView)
		return;

	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	const f32_t textScaleX = vViewportSize.x / 1280.f;
	const f32_t textScaleY = vViewportSize.y / 720.f;
	const f32_t textUiScale = (std::min)(textScaleX, textScaleY);
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
	if (m_pInviteView->Get_SlotRect(
		"PartyInvite_TitleTextBox", fTitleX, fTitleY, fTitleW, fTitleH))
	{
		// "파티초대"
		Fn_DrawCentered(fTitleX + fTitleW * 0.5f, fTitleY + fTitleH * 0.5f,
			L"\xD30C\xD2F0\xCD08\xB300", 16.8f, Colors::White);
	}

	f32_t fDescX = 0.f, fDescY = 0.f, fDescW = 0.f, fDescH = 0.f;
	if (m_pInviteView->Get_SlotRect(
		"PartyInvite_DescTextBox", fDescX, fDescY, fDescW, fDescH))
	{
		// "161기의 마지막 여정을 함께 하시겠습니까?"
		Fn_DrawCentered(fDescX + fDescW * 0.5f, fDescY + fDescH * 0.5f,
			L"161\xAE30\xC758 \xB9C8\xC9C0\xB9C9 \xC5EC\xC815\xC744 \xD568\xAED8 "
			L"\xD558\xC2DC\xACA0\xC2B5\xB2C8\xAE4C?", 12.6f, Colors::White);
	}

	struct MODAL_BUTTON_LABEL
	{
		const char_t* pButtonSlotId;
		const char_t* pIconSlotId;
		const wchar_t* pLabel;
	};
	const MODAL_BUTTON_LABEL BUTTON_LABELS[] =
	{
		{ "PartyInvite_ConfirmButton", "PartyInvite_AcceptIcon", L"\xC218\xB77D" }, // "수락"
		{ "PartyInvite_CancelButton", "PartyInvite_DeclineIcon", L"\xAC70\xC808" }, // "거절"
	};
	for (const MODAL_BUTTON_LABEL& Label : BUTTON_LABELS)
	{
		f32_t fButtonX = 0.f, fButtonY = 0.f, fButtonW = 0.f, fButtonH = 0.f;
		f32_t fIconX = 0.f, fIconY = 0.f, fIconW = 0.f, fIconH = 0.f;
		if (!m_pInviteView->Get_SlotRect(
				Label.pButtonSlotId, fButtonX, fButtonY, fButtonW, fButtonH) ||
			!m_pInviteView->Get_SlotRect(
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

void Client::CPartyInteractionView::Render_ContextMenuText()
{
	if (!m_hasContextMenuTarget || nullptr == m_pContextMenuView)
		return;

	f32_t fPanelX = 0.f, fPanelY = 0.f, fPanelW = 0.f, fPanelH = 0.f;
	f32_t fButtonX = 0.f, fButtonY = 0.f, fButtonW = 0.f, fButtonH = 0.f;
	if (!m_pContextMenuView->Get_SlotRect(
			"PartyContextMenu_Panel", fPanelX, fPanelY, fPanelW, fPanelH) ||
		!m_pContextMenuView->Get_SlotRect(
			"PartyContextMenu_HoverHighlight", fButtonX, fButtonY, fButtonW, fButtonH))
	{
		return;
	}

	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	const f32_t fResolutionWidth = m_pContextMenuView->Get_ResolutionWidth();
	const f32_t fResolutionHeight = m_pContextMenuView->Get_ResolutionHeight();
	if (fResolutionWidth <= 0.f || fResolutionHeight <= 0.f)
		return;
	const f32_t textScaleX = vViewportSize.x / fResolutionWidth;
	const f32_t textScaleY = vViewportSize.y / fResolutionHeight;
	const f32_t textUiScale = (std::min)(textScaleX, textScaleY);

	std::wstring strNickname;
	if (Convert_NicknameToWide(m_strContextMenuTargetNickname, strNickname))
	{
		const float2_t vMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strNickname.c_str());
		const f32_t fScale = (vMeasured.y > 0.f) ? (fPanelH * 0.5f / vMeasured.y) : 1.f;
		// Same IM_COL32(255,255,0,255) yellow the old ImGui draw used.
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strNickname.c_str(),
			float2_t((fPanelX + fPanelW * 0.5f) * textScaleX,
				(fPanelY + fPanelH * 0.5f) * textScaleY),
			XMVectorSet(1.f, 1.f, 0.f, 1.f), 0.f, float2_t(0.5f, 0.5f),
			fScale * textUiScale);
	}

	// "파티초대"
	constexpr wchar_t INVITE_LABEL[] =
		L"\xD30C\xD2F0\xCD08\xB300";
	const float2_t vLabelMeasured =
		CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), INVITE_LABEL);
	const f32_t fLabelScale = (vLabelMeasured.y > 0.f) ? (fButtonH * 0.5f / vLabelMeasured.y) : 1.f;
	CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), INVITE_LABEL,
		float2_t((fButtonX + fButtonW * 0.5f) * textScaleX,
			(fButtonY + fButtonH * 0.5f) * textScaleY),
		Colors::White, 0.f, float2_t(0.5f, 0.5f), fLabelScale * textUiScale);
}

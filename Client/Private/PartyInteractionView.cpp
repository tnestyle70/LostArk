#include "imgui.h"

#include "PartyInteractionView.h"

#include "Character.h"
#include "GameInstance.h"
#include "HUDRuntimeView.h"
#include "MainApp.h"
#include "PlayerCommandSink.h"
#include "PlayerController.h"
#include "RuntimeAssetRoot.h"
#include "Transform.h"

#include <cfloat>
#include <filesystem>

void Client::CPartyInteractionView::Initialize(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	m_pInviteView = std::make_unique<CHUDRuntimeView>(
		pDevice, pContext, L"UI/Party/PartyInviteConfirm_Layout.json");
	m_pContextMenuView = std::make_unique<CHUDRuntimeView>(
		pDevice, pContext, L"UI/Party/PartyContextMenu_Layout.json");

	/* Same first-draw-invisible issue CLevel_Bern's Valtan-entry popup had --
	   see Level_Bern::Initialize's own warm-up comment. */
	CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), L" ",
		float2_t(-1000.f, -1000.f), Colors::White, 0.f,
		float2_t(0.5f, 0.5f), 1.f);
}

bool_t Client::CPartyInteractionView::Update(
	CClientReplication& Replication,
	const std::shared_ptr<IPlayerCommandSink>& pCommandSink,
	const std::vector<REPLICATED_PLAYER_VIEW>& OtherPlayers)
{
	UNREFERENCED_PARAMETER(pCommandSink);

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

	return Update_ContextMenuTrigger(OtherPlayers);
}

bool_t Client::CPartyInteractionView::Update_ContextMenuTrigger(
	const std::vector<REPLICATED_PLAYER_VIEW>& OtherPlayers)
{
	const bool_t isRightMouseDown =
		!CGameInstance::Get().IsMouseInputBlocked() &&
		0 != (CGameInstance::Get().Get_DIMouseState(DIM::RB) & 0x80);
	const bool_t isRightMousePressed =
		isRightMouseDown && !m_wasRightMouseDown;
	m_wasRightMouseDown = isRightMouseDown;

	if (!isRightMousePressed || m_hasContextMenuTarget || m_isInvitePopupOpen)
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

	const ImVec2 mouse = ImGui::GetMousePos();
	m_fContextMenuScreenX = mouse.x;
	m_fContextMenuScreenY = mouse.y;
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

	/* Panel (Name Box, shows the target's nickname) and Button (Black box,
	   the actual "파티초대" invite action) are authored stacked in one
	   1280x720 canvas -- their relative offset is what carries over here,
	   not their absolute authored position, since the menu has to appear
	   wherever the player was right-clicked. */
	f32_t fPanelX = 0.f, fPanelY = 0.f, fPanelW = 0.f, fPanelH = 0.f;
	f32_t fButtonX = 0.f, fButtonY = 0.f, fButtonW = 0.f, fButtonH = 0.f;
	if (!m_pContextMenuView->Get_SlotRect(
			"PartyContextMenu_Panel", fPanelX, fPanelY, fPanelW, fPanelH) ||
		!m_pContextMenuView->Get_SlotRect(
			"PartyContextMenu_HoverHighlight",
			fButtonX, fButtonY, fButtonW, fButtonH))
	{
		return;
	}

	if (m_hasContextMenuJustOpened)
	{
		ImGui::OpenPopup("PartyContextMenu");
		m_hasContextMenuJustOpened = false;
	}

	if (!m_hasContextMenuTarget)
		return;

	/* Full-viewport invisible modal, same pattern as Render_InvitePopup /
	   Level_Bern's Render_ValtanEntryModal -- a non-modal BeginPopup
	   auto-positioned at the click point closed itself within a frame or two
	   here, because its own "click outside closes" heuristic doesn't
	   reconcile cleanly against a raw-DirectInput-driven OpenPopup (no ImGui
	   item ever "opened" it from ImGui's own point of view). Owning open/close
	   manually below sidesteps that fragility entirely, matching the two
	   other popups in this same codebase that already work reliably. */
	const ImGuiViewport* pViewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(pViewport->WorkPos);
	ImGui::SetNextWindowSize(pViewport->WorkSize);

	const ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoSavedSettings;
	/* Same dim-rect suppression as Render_InvitePopup -- BeginPopupModal draws
	   it before returning regardless of ImGuiWindowFlags_NoBackground. */
	ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.f, 0.f, 0.f, 0.f));
	const bool_t isMenuOpen =
		ImGui::BeginPopupModal("PartyContextMenu", nullptr, flags);
	ImGui::PopStyleColor();
	if (!isMenuOpen)
	{
		m_hasContextMenuTarget = false;
		return;
	}

	const f32_t fScaleX = pViewport->WorkSize.x / 1280.f;
	const f32_t fScaleY = pViewport->WorkSize.y / 720.f;
	const auto Fn_ToScreen = [&](f32_t fX, f32_t fY)
	{
		return ImVec2(
			m_fContextMenuScreenX + (fX - fPanelX) * fScaleX,
			m_fContextMenuScreenY + (fY - fPanelY) * fScaleY);
	};

	ImDrawList* pDrawList = ImGui::GetWindowDrawList();

	const ImVec2 vPanelMin = Fn_ToScreen(fPanelX, fPanelY);
	const ImVec2 vPanelMax = Fn_ToScreen(fPanelX + fPanelW, fPanelY + fPanelH);
	if (ID3D11ShaderResourceView* pPanel =
		m_pContextMenuView->Load_Texture("UI/Party/Name Box.png"))
	{
		pDrawList->AddImage(
			reinterpret_cast<ImTextureID>(pPanel), vPanelMin, vPanelMax);
	}
	{
		const f32_t fNicknameFontSize = ImGui::GetFontSize() * 1.3f;
		const ImVec2 vNicknameSize = ImGui::GetFont()->CalcTextSizeA(
			fNicknameFontSize, FLT_MAX, 0.f, m_strContextMenuTargetNickname.c_str());
		pDrawList->AddText(
			ImGui::GetFont(), fNicknameFontSize,
			ImVec2(
				(vPanelMin.x + vPanelMax.x) * 0.5f - vNicknameSize.x * 0.5f,
				(vPanelMin.y + vPanelMax.y) * 0.5f - vNicknameSize.y * 0.5f),
			IM_COL32(255, 255, 0, 255), m_strContextMenuTargetNickname.c_str());
	}

	const ImVec2 vButtonMin = Fn_ToScreen(fButtonX, fButtonY);
	const ImVec2 vButtonMax = Fn_ToScreen(fButtonX + fButtonW, fButtonY + fButtonH);
	const bool_t isButtonHovered =
		ImGui::IsMouseHoveringRect(vButtonMin, vButtonMax);
	if (ID3D11ShaderResourceView* pButton =
		m_pContextMenuView->Load_Texture("UI/Party/Black box.png"))
	{
		pDrawList->AddImage(
			reinterpret_cast<ImTextureID>(pButton), vButtonMin, vButtonMax,
			ImVec2(0.f, 0.f), ImVec2(1.f, 1.f),
			isButtonHovered ?
				IM_COL32(255, 255, 255, 255) : IM_COL32(210, 210, 210, 255));
	}
	constexpr const char* INVITE_LABEL = "\xed\x8c\x8c\xed\x8b\xb0\xec\xb4\x88\xeb\x8c\x80"; // "파티초대"
	const ImVec2 vLabelSize = ImGui::CalcTextSize(INVITE_LABEL);
	pDrawList->AddText(
		ImVec2(
			(vButtonMin.x + vButtonMax.x) * 0.5f - vLabelSize.x * 0.5f,
			(vButtonMin.y + vButtonMax.y) * 0.5f - vLabelSize.y * 0.5f),
		IM_COL32(255, 255, 255, 255), INVITE_LABEL);

	bool_t closeMenu = false;
	if (isButtonHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		if (nullptr != pCommandSink)
		{
			pCommandSink->Request_PartyInvite(
				m_iNextRequestSequence++, m_iContextMenuTargetNetEntityId);
		}
		CMainApp::Play_UIButtonClickSound();
		closeMenu = true;
	}
	else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		/* Left-click anywhere outside the menu's own art dismisses it without
		   sending an invite. The opening right-click itself never reaches
		   here (wrong button), so this can't immediately re-close the popup
		   the same frame it opens. */
		const ImVec2 vMenuMin(
			(std::min)(vPanelMin.x, vButtonMin.x),
			(std::min)(vPanelMin.y, vButtonMin.y));
		const ImVec2 vMenuMax(
			(std::max)(vPanelMax.x, vButtonMax.x),
			(std::max)(vPanelMax.y, vButtonMax.y));
		if (!ImGui::IsMouseHoveringRect(vMenuMin, vMenuMax))
			closeMenu = true;
	}

	if (closeMenu)
	{
		m_hasContextMenuTarget = false;
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

void Client::CPartyInteractionView::Render_InvitePopup(
	const std::shared_ptr<IPlayerCommandSink>& pCommandSink)
{
	if (nullptr == m_pInviteView)
		return;

	if (m_hasInvitePopupJustOpened)
	{
		ImGui::OpenPopup("PartyInviteConfirm");
		m_hasInvitePopupJustOpened = false;
	}

	if (!m_isInvitePopupOpen)
		return;

	const ImGuiViewport* pViewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(pViewport->WorkPos);
	ImGui::SetNextWindowSize(pViewport->WorkSize);

	const ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoSavedSettings;

	/* BeginPopupModal draws its own full-viewport dim rect before returning,
	   independent of ImGuiWindowFlags_NoBackground (which only covers the
	   popup window itself) -- StyleColorsDark's default ModalWindowDimBg is a
	   light translucent grey, which reads as a wash of white over the game
	   behind it. Suppressed here since this popup only wants its own panel
	   art visible, not a dimmed backdrop. */
	ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.f, 0.f, 0.f, 0.f));
	const bool_t isPopupOpen =
		ImGui::BeginPopupModal("PartyInviteConfirm", nullptr, flags);
	ImGui::PopStyleColor();
	if (!isPopupOpen)
	{
		m_isInvitePopupOpen = false;
		return;
	}

	const auto Fn_ToScreen = [pViewport](f32_t fX, f32_t fY)
	{
		const f32_t fScaleX = pViewport->WorkSize.x / 1280.f;
		const f32_t fScaleY = pViewport->WorkSize.y / 720.f;
		return ImVec2(
			pViewport->WorkPos.x + fX * fScaleX,
			pViewport->WorkPos.y + fY * fScaleY);
	};
	const auto Fn_HitTest = [](const ImVec2& corner0, const ImVec2& corner1)
	{
		const ImVec2 mouse = ImGui::GetMousePos();
		return mouse.x >= corner0.x && mouse.x < corner1.x &&
			mouse.y >= corner0.y && mouse.y < corner1.y;
	};

	ImDrawList* pDrawList = ImGui::GetWindowDrawList();

	f32_t fPanelX = 0.f, fPanelY = 0.f, fPanelW = 0.f, fPanelH = 0.f;
	if (m_pInviteView->Get_SlotRect(
		"PartyInvite_Panel", fPanelX, fPanelY, fPanelW, fPanelH))
	{
		ID3D11ShaderResourceView* pPanel = m_pInviteView->Load_Texture(
			"UI/ClassSelect/Common/CreateCharacterModalPanel.png");
		if (nullptr != pPanel)
		{
			pDrawList->AddImage(
				reinterpret_cast<ImTextureID>(pPanel),
				Fn_ToScreen(fPanelX, fPanelY),
				Fn_ToScreen(fPanelX + fPanelW, fPanelY + fPanelH));
		}
	}

	/* PartyInvite_TitleTextBox/DescTextBox are position-only markers for the
	   text pass below -- no background image, same as Valtan Entry. */

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
		const ImVec2 corner0 = Fn_ToScreen(fX, fY);
		const ImVec2 corner1 = Fn_ToScreen(fX + fW, fY + fH);
		const bool_t isHovered = Fn_HitTest(corner0, corner1);
		ID3D11ShaderResourceView* pTexture = m_pInviteView->Load_Texture(
			isHovered ?
				"UI/ClassSelect/Common/NormalButtonHover.png" :
				"UI/ClassSelect/Common/NormalButton.png");
		if (nullptr != pTexture)
		{
			pDrawList->AddImage(
				reinterpret_cast<ImTextureID>(pTexture), corner0, corner1);
		}
		if (isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			CMainApp::Play_UIButtonClickSound();
			if (button.isConfirm)
				acceptClicked = true;
			else
				declineClicked = true;
		}
	}

	/* Drawn after the buttons so the button art doesn't paint over them. */
	struct MODAL_ICON_SLOT
	{
		const char* pSlotId;
		const char* pTexturePath;
	};
	static constexpr MODAL_ICON_SLOT ICON_SLOTS[2] =
	{
		{ "PartyInvite_AcceptIcon", "UI/Bern/Accept.png" },
		{ "PartyInvite_DeclineIcon", "UI/Bern/Decline.png" },
	};
	for (const MODAL_ICON_SLOT& icon : ICON_SLOTS)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pInviteView->Get_SlotRect(icon.pSlotId, fX, fY, fW, fH))
			continue;
		ID3D11ShaderResourceView* pTexture =
			m_pInviteView->Load_Texture(icon.pTexturePath);
		if (nullptr != pTexture)
		{
			pDrawList->AddImage(
				reinterpret_cast<ImTextureID>(pTexture),
				Fn_ToScreen(fX, fY), Fn_ToScreen(fX + fW, fY + fH));
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
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

void Client::CPartyInteractionView::Render_InvitePopupText()
{
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

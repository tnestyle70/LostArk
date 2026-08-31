#include "imgui.h"

#include "RaidEntryPreviewView.h"

#include "GameInstance.h"
#include "HUDRuntimeView.h"
#include "MainApp.h"

CRaidEntryPreviewView::CRaidEntryPreviewView(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: m_pView(std::make_unique<CHUDRuntimeView>(
		pDevice, pContext, L"UI/Bern/ValtanRaidEntry_Layout.json"))
	, m_pConfirmView(std::make_unique<CHUDRuntimeView>(
		pDevice, pContext, L"UI/Bern/BernValtanEntry_Layout.json"))
{
}

CRaidEntryPreviewView::~CRaidEntryPreviewView() = default;

void CRaidEntryPreviewView::Open()
{
	m_isOpen = true;
	m_hasJustOpened = true;
}

bool_t CRaidEntryPreviewView::Render()
{
	if (nullptr == m_pView)
		return false;

	if (m_hasJustOpened)
	{
		ImGui::OpenPopup("RaidEntryPreview");
		m_hasJustOpened = false;
	}

	if (!m_isOpen)
		return false;

	ImGuiViewport* pViewport = ImGui::GetMainViewport();
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
	   behind it. Suppressed since this popup only wants its own panel art
	   visible, not a dimmed backdrop -- same fix as CPartyInteractionView's
	   two modals. */
	ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.f, 0.f, 0.f, 0.f));
	const bool_t isModalOpen =
		ImGui::BeginPopupModal("RaidEntryPreview", nullptr, flags);
	ImGui::PopStyleColor();
	if (!isModalOpen)
	{
		m_isOpen = false;
		return false;
	}

	/* Entrance already clicked once -- draw the small second-step 수락/거절
	   dialog instead of the full screen underneath it (same single popup, so
	   there is no nested BeginPopupModal). */
	if (m_isConfirmStepOpen)
	{
		const bool_t entranceConfirmed = Render_ConfirmStep();
		ImGui::EndPopup();
		return entranceConfirmed;
	}

	/* Real 1920x1080 canvas traced from the shipped "epicRaidEntrance" gfx
	   (.md/TJ/08-30/2026-08-30_레이드입장창_PLAN.md) -- ValtanRaidEntry_Layout.json
	   declares the same reference resolution. */
	const auto Fn_ToScreen = [pViewport](f32_t fX, f32_t fY)
	{
		const f32_t fScaleX = pViewport->WorkSize.x / 1920.f;
		const f32_t fScaleY = pViewport->WorkSize.y / 1080.f;
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
	const auto Fn_DrawSlot = [&](const char* pSlotId, const char* pTexturePath,
		ImU32 tint = IM_COL32_WHITE)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pView->Get_SlotRect(pSlotId, fX, fY, fW, fH))
			return;
		ID3D11ShaderResourceView* pTexture = m_pView->Load_Texture(pTexturePath);
		if (nullptr == pTexture)
			return;
		/* GetForegroundDrawList (not GetWindowDrawList) -- every always-on combat
		   HUD element (RenderCombatHUD/RenderBossHealthBar/RenderSkillIcons/
		   RenderQuickSlot, MainApp.cpp) draws to the shared foreground list,
		   which ImGui composites above every regular window's own drawlist
		   regardless of submission order. Submission order among foreground-list
		   users still matters, which is why the caller's Render() call site must
		   run after those HUD renders (see CMainApp::Render()). */
		ImGui::GetForegroundDrawList(pViewport)->AddImage(
			reinterpret_cast<ImTextureID>(pTexture),
			Fn_ToScreen(fX, fY), Fn_ToScreen(fX + fW, fY + fH),
			ImVec2(0.f, 0.f), ImVec2(1.f, 1.f), tint);
	};
	/* RaidEntry_PanelFrame_TopRight reuses the same corner-ornament texture as
	   TopLeft, mirrored via swapped U coordinates -- this manual per-slot draw
	   path never consulted the JSON layer's own "flipX" field, so the mirror
	   happens here instead of being inherited from the layout document. */
	const auto Fn_DrawSlotFlippedX = [&](const char* pSlotId, const char* pTexturePath)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pView->Get_SlotRect(pSlotId, fX, fY, fW, fH))
			return;
		ID3D11ShaderResourceView* pTexture = m_pView->Load_Texture(pTexturePath);
		if (nullptr == pTexture)
			return;
		ImGui::GetForegroundDrawList(pViewport)->AddImage(
			reinterpret_cast<ImTextureID>(pTexture),
			Fn_ToScreen(fX, fY), Fn_ToScreen(fX + fW, fY + fH),
			ImVec2(1.f, 0.f), ImVec2(0.f, 1.f));
	};

	/* Every slot drawn below is backed by a real extracted or already-shipped
	   asset (epicRaidEntrance i1/i6/i245.tga crops, EFUI_ShareImage button
	   skins, this project's own Esther/ItemUpgrade icons). Slots with NO real
	   asset found (boss portrait, portrait frame, name plate, raid icon, tier
	   badges, warning box, nav arrows, top border strip) are intentionally
	   left undrawn here -- they stay in ValtanRaidEntry_Layout.json as
	   position-only markers (see .md/TJ/08-30 PLAN's asset gap list) until a
	   real asset is sourced for each, instead of being filled with invented
	   placeholder art. */
	Fn_DrawSlot("RaidEntry_DimBackdrop", "UI/Common/White1x1.png",
		IM_COL32(0, 0, 0, 184));

	/* Top raid-select carousel -- real boss-portrait thumbnails cropped from
	   the same epicRaidEntrance i1.tga atlas page (already pre-masked into
	   parallelograms with transparent corners in the source alpha). Purely
	   decorative here: this project only has one raid to preview, so there is
	   no click-to-switch behind it, unlike the real multi-raid carousel. */
	static constexpr const char* TOP_THUMB_SLOTS[7] =
	{
		"RaidEntry_TopThumb_0", "RaidEntry_TopThumb_1", "RaidEntry_TopThumb_2",
		"RaidEntry_TopThumb_3", "RaidEntry_TopThumb_4", "RaidEntry_TopThumb_5",
		"RaidEntry_TopThumb_6",
	};
	static constexpr const char* TOP_THUMB_PATHS[7] =
	{
		"UI/Bern/thumb_a.png", "UI/Bern/thumb_b.png", "UI/Bern/thumb_c.png",
		"UI/Bern/RaidEntry_KakulThumb.png", "UI/Bern/thumb_e.png",
		"UI/Bern/thumb_f.png", "UI/Bern/thumb_g.png",
	};
	for (std::size_t i = 0; i < 7; ++i)
		Fn_DrawSlot(TOP_THUMB_SLOTS[i], TOP_THUMB_PATHS[i]);

	// RaidEntry_BossPortrait, RaidEntry_PortraitFrame, RaidEntry_RaidIconSlot:
	// no real asset found -- position markers only.
	Fn_DrawSlot("RaidEntry_Vignette", "UI/Bern/RaidEntry_Vignette.png");

	/* Real esther-skill portrait assets this project already ships and uses
	   elsewhere (Character Select's own identity/HUD row) -- reused here
	   instead of another procedural placeholder, matching the reference
	   screen's "사용 가능한 에스더 스킬" row under the raid icon. */
	static constexpr const char* ESTHER_FRAME_SLOTS[3] =
	{
		"RaidEntry_EstherFrame_0", "RaidEntry_EstherFrame_1", "RaidEntry_EstherFrame_2",
	};
	static constexpr const char* ESTHER_PORTRAIT_SLOTS[3] =
	{
		"RaidEntry_EstherPortrait_0", "RaidEntry_EstherPortrait_1", "RaidEntry_EstherPortrait_2",
	};
	static constexpr const char* ESTHER_PORTRAIT_PATHS[3] =
	{
		"UI/Esther/esther_portrait_bahuntur.png",
		"UI/Esther/esther_portrait_sillian.png",
		"UI/Esther/esther_portrait_wei.png",
	};
	for (std::size_t i = 0; i < 3; ++i)
	{
		Fn_DrawSlot(ESTHER_FRAME_SLOTS[i], "UI/Esther/esther_slot_frame.png");
		Fn_DrawSlot(ESTHER_PORTRAIT_SLOTS[i], ESTHER_PORTRAIT_PATHS[i]);
	}
	Fn_DrawSlot("RaidEntry_PanelFrame_TopLeft", "UI/Bern/RaidEntry_CornerFrame.png");
	Fn_DrawSlotFlippedX("RaidEntry_PanelFrame_TopRight", "UI/Bern/RaidEntry_CornerFrame.png");
	// RaidEntry_TopBorderStrip: no real asset found -- position marker only.

	// RaidEntry_TierBadge_0/1/2, RaidEntry_WarningBoxSlot,
	// RaidEntry_RewardArrowLeft/Right: no real asset found -- position
	// markers only (their text, where any, is still drawn in RenderText()).

	static constexpr const char* REWARD_ICON_SLOTS[6] =
	{
		"RaidEntry_RewardIcon_0", "RaidEntry_RewardIcon_1",
		"RaidEntry_RewardIcon_2", "RaidEntry_RewardIcon_3",
		"RaidEntry_RewardIcon_4", "RaidEntry_RewardIcon_5",
	};
	static constexpr const char* REWARD_ICON_PATHS[6] =
	{
		"UI/ItemUpgrade/lm_head_icon.png", "UI/ItemUpgrade/lm_shoulder_icon.png",
		"UI/ItemUpgrade/lm_top_icon.png", "UI/ItemUpgrade/lm_bottom_icon.png",
		"UI/ItemUpgrade/lm_glove_icon.png", "UI/ItemUpgrade/lm_weapon_icon.png",
	};
	for (std::size_t i = 0; i < 6; ++i)
		Fn_DrawSlot(REWARD_ICON_SLOTS[i], REWARD_ICON_PATHS[i]);

	/* Matching / Find Party are visual-only per PLAN scope (no matching system
	   exists yet) -- static art, no hover swap, no click handling. Decline
	   always closes internally; Entrance is reported back to the caller via
	   this function's return value instead of sending any command itself. */
	Fn_DrawSlot("RaidEntry_MatchingButton", "UI/Bern/RaidEntry_ButtonSteel.png");
	Fn_DrawSlot("RaidEntry_FindPartyButton", "UI/Bern/RaidEntry_ButtonSteel.png");

	struct MODAL_BUTTON
	{
		const char* pSlotId;
		const char* pNormalTexturePath;
		const char* pHoverTexturePath;
		bool_t isConfirm;
	};
	static constexpr MODAL_BUTTON BUTTONS[2] =
	{
		{ "RaidEntry_EntranceButton", "UI/Bern/RaidEntry_ButtonGold.png",
			"UI/Bern/RaidEntry_ButtonGoldHover.png", true },
		/* The reference has no separate "거절" text button on this screen --
		   only the top-right "닫기 (Esc)" X closes it. An earlier pass wrongly
		   duplicated that with its own steel Decline button; removed. */
		{ "RaidEntry_CloseButtonSlot", "UI/Bern/Decline.png",
			"UI/Bern/Decline.png", false },
	};

	bool_t confirmClicked = false;
	bool_t cancelClicked = false;
	for (const MODAL_BUTTON& button : BUTTONS)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pView->Get_SlotRect(button.pSlotId, fX, fY, fW, fH))
			continue;
		const ImVec2 corner0 = Fn_ToScreen(fX, fY);
		const ImVec2 corner1 = Fn_ToScreen(fX + fW, fY + fH);
		const bool_t isHovered = Fn_HitTest(corner0, corner1);
		ID3D11ShaderResourceView* pTexture = m_pView->Load_Texture(
			isHovered ? button.pHoverTexturePath : button.pNormalTexturePath);
		if (nullptr != pTexture)
		{
			ImGui::GetForegroundDrawList(pViewport)->AddImage(
				reinterpret_cast<ImTextureID>(pTexture), corner0, corner1);
		}
		if (isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			CMainApp::Play_UIButtonClickSound();
			if (button.isConfirm)
				confirmClicked = true;
			else
				cancelClicked = true;
		}
	}

	/* Drawn after the buttons (not before) so the button art doesn't paint
	   over these and hide them. */
	struct MODAL_ICON_SLOT
	{
		const char* pSlotId;
		const char* pTexturePath;
	};
	static constexpr MODAL_ICON_SLOT ICON_SLOTS[1] =
	{
		{ "RaidEntry_AcceptIcon", "UI/Bern/Accept.png" },
	};
	for (const MODAL_ICON_SLOT& icon : ICON_SLOTS)
		Fn_DrawSlot(icon.pSlotId, icon.pTexturePath);

	if (cancelClicked)
	{
		m_isOpen = false;
		ImGui::CloseCurrentPopup();
	}
	else if (confirmClicked)
	{
		// Opens the small 수락/거절 step on the next Render() call instead of
		// reporting Entrance back immediately -- see Render_ConfirmStep().
		m_isConfirmStepOpen = true;
	}

	ImGui::EndPopup();
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
   pre-existing simple dialog surfaced one step later. Drawn inside the same
   outer BeginPopupModal Render() already opened, so no second popup wrapper
   is needed here. */
bool_t CRaidEntryPreviewView::Render_ConfirmStep()
{
	if (nullptr == m_pConfirmView)
		return false;

	ImGuiViewport* pViewport = ImGui::GetMainViewport();
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

	f32_t fPanelX = 0.f, fPanelY = 0.f, fPanelW = 0.f, fPanelH = 0.f;
	if (m_pConfirmView->Get_SlotRect(
		"ValtanEntry_Panel", fPanelX, fPanelY, fPanelW, fPanelH))
	{
		ID3D11ShaderResourceView* pPanel = m_pConfirmView->Load_Texture(
			"UI/ClassSelect/Common/CreateCharacterModalPanel.png");
		if (nullptr != pPanel)
		{
			ImGui::GetForegroundDrawList(pViewport)->AddImage(
				reinterpret_cast<ImTextureID>(pPanel),
				Fn_ToScreen(fPanelX, fPanelY),
				Fn_ToScreen(fPanelX + fPanelW, fPanelY + fPanelH));
		}
	}

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
		const ImVec2 corner0 = Fn_ToScreen(fX, fY);
		const ImVec2 corner1 = Fn_ToScreen(fX + fW, fY + fH);
		const bool_t isHovered = Fn_HitTest(corner0, corner1);
		ID3D11ShaderResourceView* pTexture = m_pConfirmView->Load_Texture(
			isHovered ?
				"UI/ClassSelect/Common/NormalButtonHover.png" :
				"UI/ClassSelect/Common/NormalButton.png");
		if (nullptr != pTexture)
		{
			ImGui::GetForegroundDrawList(pViewport)->AddImage(
				reinterpret_cast<ImTextureID>(pTexture), corner0, corner1);
		}
		if (isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			CMainApp::Play_UIButtonClickSound();
			if (button.isConfirm)
				confirmClicked = true;
			else
				cancelClicked = true;
		}
	}

	struct CONFIRM_ICON
	{
		const char* pSlotId;
		const char* pTexturePath;
	};
	static constexpr CONFIRM_ICON ICONS[2] =
	{
		{ "ValtanEntry_AcceptIcon", "UI/Bern/Accept.png" },
		{ "ValtanEntry_DeclineIcon", "UI/Bern/Decline.png" },
	};
	for (const CONFIRM_ICON& icon : ICONS)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pConfirmView->Get_SlotRect(icon.pSlotId, fX, fY, fW, fH))
			continue;
		ID3D11ShaderResourceView* pTexture =
			m_pConfirmView->Load_Texture(icon.pTexturePath);
		if (nullptr != pTexture)
		{
			ImGui::GetForegroundDrawList(pViewport)->AddImage(
				reinterpret_cast<ImTextureID>(pTexture),
				Fn_ToScreen(fX, fY), Fn_ToScreen(fX + fW, fY + fH));
		}
	}

	if (cancelClicked)
	{
		m_isConfirmStepOpen = false;
		m_isOpen = false;
		ImGui::CloseCurrentPopup();
		return false;
	}
	if (confirmClicked)
	{
		m_isConfirmStepOpen = false;
		m_isOpen = false;
		ImGui::CloseCurrentPopup();
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

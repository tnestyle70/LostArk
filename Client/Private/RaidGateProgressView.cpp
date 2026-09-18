#include "RaidGateProgressView.h"

#pragma push_macro("new")
#undef new
#include <DirectXColors.h>
#pragma pop_macro("new")

#include "GameInstance.h"
#include "MainApp.h"
#include "UIInputRouter.h"
#include "UILayoutRuntime.h"

#include <algorithm>

namespace
{
	/* Retail stage px -> this project's 1280x720 reference, the scale every runtime UI uses. */
	constexpr f32_t STAGE_TO_REF = 2.f / 3.f;
	/* EpicGateCommanderProgressFrame labels: dungeonName 18 px at (11,7) centred over the frame,
	   the second line 22 px under it. */
	constexpr f32_t NAME_PX = 18.f * STAGE_TO_REF;
	constexpr f32_t NAME_Y = 7.f * STAGE_TO_REF;
	constexpr f32_t DIFFICULTY_PX = 16.f * STAGE_TO_REF;
	constexpr f32_t DIFFICULTY_Y = 29.f * STAGE_TO_REF;
	constexpr f32_t PROMPT_TITLE_PX = 16.8f;
	constexpr f32_t PROMPT_DESC_PX = 12.6f;
	constexpr f32_t PROMPT_BUTTON_PX = 12.f;
	constexpr f32_t NOTICE_PX = 18.f;

	constexpr const char* ART_ICON_CHECK = "UI/RaidGateProgress/GateIcon_Check.png";
	constexpr const char* ART_ICON_ACTIVE = "UI/RaidGateProgress/GateIcon_Active.png";
	constexpr const char* ART_ICON_INACTIVE = "UI/RaidGateProgress/GateIcon_Inactive.png";
	constexpr const char* ART_BUTTON = "UI/ClassSelect/Common/NormalButton.png";
	constexpr const char* ART_BUTTON_HOVER = "UI/ClassSelect/Common/NormalButtonHover.png";
	constexpr const char* PROMPT_SLOTS[] = { "RGV_Panel", "RGV_ConfirmButton", "RGV_CancelButton" };

	/* GameMsg sys.commander.progress_vote / progress_vote_dialog_desc / _desc_1,
	   stop_dungeon_progress; sys.common accept / decline. Wide hex escapes keep this file ASCII. */
	constexpr const wchar_t* TEXT_PROGRESS = L"\xB358\xC804 \xC9C4\xD589";
	constexpr const wchar_t* TEXT_STOP = L"\xC911\xB2E8\xD558\xAE30";
	constexpr const wchar_t* TEXT_ACCEPT = L"\xC218\xB77D";
	constexpr const wchar_t* TEXT_DECLINE = L"\xAC70\xC808";
	constexpr const wchar_t* TEXT_ASK = L"\xB358\xC804\xC744 \xACC4\xC18D \xC9C4\xD589\xD558\xC2DC\xACA0\xC2B5\xB2C8\xAE4C?";
	constexpr const wchar_t* TEXT_APPLIED = L"\xB2D8\xC774 \xB358\xC804 \xC9C4\xD589\xC744 \xC2E0\xCCAD\xD558\xC600\xC2B5\xB2C8\xB2E4.";

	const wstring_t FONT_YOON = TEXT("Font_YoonGasiIIM");
	const fvector_t COLOR_DIFFICULTY = DirectX::XMVectorSet(1.f, 0.82f, 0.f, 1.f);   // #FFD200 like the retail label

	void Draw_Centered(const wstring_t& strFont, const wchar_t* pText, const f32_t fRefX, const f32_t fRefY,
		const f32_t fRefPx, const fvector_t vColor)
	{
		CGameInstance& Instance = CGameInstance::Get();
		const float2_t vViewport = Instance.Get_ViewportSize();
		if (vViewport.x <= 0.f || vViewport.y <= 0.f || nullptr == pText || L'\0' == pText[0])
			return;
		const float2_t vMeasured = Instance.Measure_Text(strFont, pText);
		if (vMeasured.y <= 0.f)
			return;
		const f32_t fScaleY = vViewport.y / 720.f;
		Instance.Draw_Text(strFont, pText,
			float2_t(fRefX * vViewport.x / 1280.f, fRefY * fScaleY),
			vColor, 0.f, float2_t(0.5f, 0.5f), fRefPx * fScaleY / vMeasured.y);
	}
}

void Client::CRaidGateProgressView::Initialize(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const uint32_t iOwnerLevelIndex)
{
	m_pWidget = std::make_unique<CUILayoutRuntime>(pDevice, pContext, iOwnerLevelIndex,
		TEXT("Layer_UI"), L"UI/RaidGateProgress/RaidGateProgress_Layout.json");
	m_pPrompt = std::make_unique<CUILayoutRuntime>(pDevice, pContext, iOwnerLevelIndex,
		TEXT("Layer_UI"), L"UI/RaidGateProgress/RaidGateVote_Layout.json");
	Set_PromptVisible(false);
	Set_Progress(0u, 0u);
}

void Client::CRaidGateProgressView::Set_Raid(
	const wstring_t& strName, const wstring_t& strDifficulty, const uint8_t iGateCount)
{
	m_strName = strName;
	m_strDifficulty = strDifficulty;
	m_iGateCount = (std::min<uint8_t>)(iGateCount, 3u);
	Set_Progress(m_iCurrentGate, m_iClearedMask);
}

void Client::CRaidGateProgressView::Set_Progress(const uint8_t iCurrentGate, const uint8_t iClearedMask)
{
	m_iCurrentGate = iCurrentGate;
	m_iClearedMask = iClearedMask;
	if (nullptr == m_pWidget)
		return;
	for (uint8_t i = 0u; i < 3u; ++i)
	{
		const string strSlot = "RGP_Icon" + std::to_string(i);
		const bool_t bShown = i < m_iGateCount;
		m_pWidget->Set_SlotVisible(strSlot, bShown);
		if (!bShown)
			continue;
		const bool_t bCleared = 0u != (m_iClearedMask & (1u << i));
		const bool_t bCurrent = m_iCurrentGate == i + 1u;
		m_pWidget->Set_SlotTexture(strSlot, bCleared ? ART_ICON_CHECK : (bCurrent ? ART_ICON_ACTIVE : ART_ICON_INACTIVE));
	}
}

void Client::CRaidGateProgressView::Set_PromptVisible(const bool_t bVisible)
{
	if (nullptr == m_pPrompt)
		return;
	for (const char* pSlot : PROMPT_SLOTS)
		m_pPrompt->Set_SlotVisible(pSlot, bVisible);
}

void Client::CRaidGateProgressView::Open_Prompt(const PROMPT eKind, const wstring_t& strProposerName)
{
	m_ePrompt = eKind;
	m_strProposer = strProposerName;
	Set_PromptVisible(PROMPT::NONE != eKind);
}

void Client::CRaidGateProgressView::Close_Prompt()
{
	m_ePrompt = PROMPT::NONE;
	Set_PromptVisible(false);
}

void Client::CRaidGateProgressView::Show_Notice(const wstring_t& strText, const f32_t fSeconds)
{
	m_strNotice = strText;
	m_fNoticeSeconds = fSeconds;
}

Client::CRaidGateProgressView::INTENT Client::CRaidGateProgressView::Update(const f32_t fTimeDelta)
{
	if (m_fNoticeSeconds > 0.f)
		m_fNoticeSeconds = (std::max)(0.f, m_fNoticeSeconds - fTimeDelta);
	if (PROMPT::NONE == m_ePrompt || nullptr == m_pPrompt)
		return INTENT::NONE;

	/* A modal: nothing underneath sees the pointer while it is up. */
	CUIInputRouter& Router = CUIInputRouter::Get();
	Router.Claim_Mouse_This_Frame();
	const f32_t fRefWidth = m_pPrompt->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pPrompt->Get_ResolutionHeight();
	INTENT eIntent = INTENT::NONE;
	for (const char* pSlot : { "RGV_ConfirmButton", "RGV_CancelButton" })
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pPrompt->Get_SlotRect(pSlot, fX, fY, fW, fH))
			continue;
		const bool_t bHovered = Router.Is_Hovered(fX, fY, fW, fH, fRefWidth, fRefHeight);
		m_pPrompt->Set_SlotTexture(pSlot, bHovered ? ART_BUTTON_HOVER : ART_BUTTON);
		if (bHovered && Router.Is_Clicked(fX, fY, fW, fH, fRefWidth, fRefHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			const bool_t bConfirm = 0 == strcmp(pSlot, "RGV_ConfirmButton");
			eIntent = PROMPT::PROPOSE == m_ePrompt ?
				(bConfirm ? INTENT::PROPOSE : INTENT::STOP) :
				(bConfirm ? INTENT::ACCEPT : INTENT::DECLINE);
		}
	}
	if (INTENT::NONE != eIntent)
		Close_Prompt();
	return eIntent;
}

void Client::CRaidGateProgressView::Render_Text() const
{
	if (nullptr != m_pWidget)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (m_pWidget->Get_SlotRect("RGP_Bg", fX, fY, fW, fH))
		{
			const f32_t fCenterX = fX + fW * 0.5f;
			Draw_Centered(FONT_YOON, m_strName.c_str(), fCenterX, fY + NAME_Y + NAME_PX * 0.5f, NAME_PX, DirectX::Colors::White);
			Draw_Centered(FONT_YOON, m_strDifficulty.c_str(), fCenterX, fY + DIFFICULTY_Y + DIFFICULTY_PX * 0.5f,
				DIFFICULTY_PX, COLOR_DIFFICULTY);
		}
	}
	if (m_fNoticeSeconds > 0.f)
	{
		Draw_Centered(FONT_YOON, m_strNotice.c_str(), 640.f, 144.f, NOTICE_PX, DirectX::Colors::Orange);
	}
	if (PROMPT::NONE == m_ePrompt || nullptr == m_pPrompt)
		return;

	f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
	if (m_pPrompt->Get_SlotRect("RGV_Panel", fX, fY, fW, fH))
	{
		/* Same title / description bands as the party invite modal on this panel art. */
		Draw_Centered(FONT_YOON, TEXT_PROGRESS, fX + fW * 0.5f, fY + 23.f, PROMPT_TITLE_PX, DirectX::Colors::White);
		if (PROMPT::PROPOSE == m_ePrompt)
		{
			Draw_Centered(FONT_YOON, TEXT_ASK, fX + fW * 0.5f, fY + 66.f, PROMPT_DESC_PX, DirectX::Colors::White);
		}
		else
		{
			const wstring_t strApplied = m_strProposer + TEXT_APPLIED;
			Draw_Centered(FONT_YOON, strApplied.c_str(), fX + fW * 0.5f, fY + 56.f, PROMPT_DESC_PX, DirectX::Colors::White);
			Draw_Centered(FONT_YOON, TEXT_ASK, fX + fW * 0.5f, fY + 74.f, PROMPT_DESC_PX, DirectX::Colors::White);
		}
	}
	const wchar_t* pConfirm = PROMPT::PROPOSE == m_ePrompt ? TEXT_PROGRESS : TEXT_ACCEPT;
	const wchar_t* pCancel = PROMPT::PROPOSE == m_ePrompt ? TEXT_STOP : TEXT_DECLINE;
	if (m_pPrompt->Get_SlotRect("RGV_ConfirmButton", fX, fY, fW, fH))
		Draw_Centered(FONT_YOON, pConfirm, fX + fW * 0.5f, fY + fH * 0.5f, PROMPT_BUTTON_PX, DirectX::Colors::White);
	if (m_pPrompt->Get_SlotRect("RGV_CancelButton", fX, fY, fW, fH))
		Draw_Centered(FONT_YOON, pCancel, fX + fW * 0.5f, fY + fH * 0.5f, PROMPT_BUTTON_PX, DirectX::Colors::White);
}

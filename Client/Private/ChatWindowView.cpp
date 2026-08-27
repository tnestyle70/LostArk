#include "imgui.h"

#include "ChatWindowView.h"

#include "GameInstance.h"
#include "ImGuiLayer.h"
#include "RuntimeAssetRoot.h"
#include "UITextureCache.h"

#include <cstdio>
#include <ctime>
#include <filesystem>
#include <imm.h>

#pragma comment(lib, "imm32.lib")

namespace
{
	/* Queries the real OS IME conversion mode for the game window instead of tracking a local
	toggle, so the icon always matches whatever the user actually has active (including
	switching mid-sentence). IME_CMODE_NATIVE means Hangul composition is active; its absence
	means direct alphanumeric input. */
	bool_t Is_KoreanInputActive()
	{
		if (nullptr == g_hWnd)
			return false;

		HIMC hImc = ImmGetContext(g_hWnd);
		if (nullptr == hImc)
			return false;

		DWORD conversion = 0, sentence = 0;
		const BOOL result = ImmGetConversionStatus(hImc, &conversion, &sentence);
		ImmReleaseContext(g_hWnd, hImc);

		return result && (0 != (conversion & IME_CMODE_NATIVE));
	}
}

Client::CChatWindowView::CChatWindowView(ComPtr<ID3D11Device> pDevice)
	: m_pTextureCache{ make_unique<CUITextureCache>(pDevice) }
{
}

Client::CChatWindowView::~CChatWindowView()
{
}

void Client::CChatWindowView::Open_Input()
{
	m_bInputOpen = true;
	m_bFocusPending = true;
	m_HideDeadline = std::chrono::steady_clock::now() + HIDE_AFTER;
}

void Client::CChatWindowView::Close_Input()
{
	m_bInputOpen = false;
	m_bFocusPending = false;
	m_InputBuffer[0] = '\0';
}

void Client::CChatWindowView::Render()
{
	/* The whole window (log + input bar) disappears once nothing has kept it alive for
	HIDE_AFTER -- a submitted message, opening the input, or (below) having it actively
	focused all push the deadline back out. */
	if (std::chrono::steady_clock::now() >= m_HideDeadline)
		return;

	constexpr f32_t REF_WIDTH = 1280.f;
	constexpr f32_t REF_HEIGHT = 720.f;
	constexpr f32_t PANEL_X = 8.f;
	constexpr f32_t PANEL_BOTTOM_MARGIN = 6.f;
	/* Narrowed to clear the HUD's HP bar (now at x=403.64 after the 0.75 HUD shrink) -- 375
	keeps the panel's right edge at 383, a ~20px gap before the bar starts. */
	constexpr f32_t PANEL_WIDTH = 375.f;
	constexpr f32_t LOG_HEIGHT = 130.f;
	constexpr f32_t INPUT_ROW_HEIGHT = 32.f;
	constexpr f32_t PANEL_Y = REF_HEIGHT - LOG_HEIGHT - INPUT_ROW_HEIGHT - PANEL_BOTTOM_MARGIN;

	/* Native sizes of the extracted bottom-bar art (Client/Bin/Resources/UI/Chat) -- Normal
	Bar is the "일반" channel label to the left of the input; Input Bar stretches to fill
	whatever width is left so its right edge lines up with the log panel's, and the IME icon
	sits inset at its right edge, same layout as the reference screenshot. */
	constexpr f32_t NORMAL_BAR_WIDTH = 96.f;
	constexpr f32_t NORMAL_BAR_HEIGHT = 32.f;
	constexpr f32_t INPUT_BAR_HEIGHT = 30.f;
	constexpr f32_t BAR_GAP = 4.f;
	constexpr f32_t IME_ICON_WIDTH = 17.f;
	constexpr f32_t IME_ICON_HEIGHT = 16.f;
	constexpr f32_t IME_ICON_INSET_X = 8.f;

	const ImGuiViewport* pViewport = ImGui::GetMainViewport();
	const f32_t fScaleX = pViewport->WorkSize.x / REF_WIDTH;
	const f32_t fScaleY = pViewport->WorkSize.y / REF_HEIGHT;

	const f32_t fPanelScreenX = pViewport->WorkPos.x + PANEL_X * fScaleX;
	const f32_t fPanelScreenY = pViewport->WorkPos.y + PANEL_Y * fScaleY;
	const f32_t fPanelScreenWidth = PANEL_WIDTH * fScaleX;

	ImGui::SetNextWindowPos(
		ImVec2(fPanelScreenX, fPanelScreenY), ImGuiCond_Always);
	ImGui::SetNextWindowSize(
		ImVec2(fPanelScreenWidth, (LOG_HEIGHT + INPUT_ROW_HEIGHT) * fScaleY),
		ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.f);

	constexpr ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
	const bool_t bWindowVisible = ImGui::Begin("##ChatWindow", nullptr, flags);
	ImGui::PopStyleVar();
	if (bWindowVisible)
	{
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();

		/* Log panel background -- the extracted dark panel, drawn at a low tint alpha (on top
		of its own baked-in ~75% alpha) so the map floor behind the chat window reads through
		clearly instead of a near-opaque box. */
		const ImVec2 vLogTopLeft = ImGui::GetCursorScreenPos();
		const ImVec2 vLogBotRight(
			vLogTopLeft.x + fPanelScreenWidth, vLogTopLeft.y + LOG_HEIGHT * fScaleY);
		if (ID3D11ShaderResourceView* pLogBg =
			m_pTextureCache->Get_Or_Load("UI/Chat/LogPanelBg.png"))
		{
			pDrawList->AddImage(
				pLogBg, vLogTopLeft, vLogBotRight,
				ImVec2(0.f, 0.f), ImVec2(1.f, 1.f), IM_COL32(255, 255, 255, 130));
		}
		else
		{
			pDrawList->AddRectFilled(vLogTopLeft, vLogBotRight, IM_COL32(0, 0, 0, 110));
		}

		ImGui::SetCursorScreenPos(vLogTopLeft);
		ImGui::BeginChild(
			"##ChatLog", ImVec2(fPanelScreenWidth, LOG_HEIGHT * fScaleY), false,
			ImGuiWindowFlags_NoScrollbar);
		for (const CHAT_LOG_LINE& Line : m_LogLines)
		{
			ImGui::TextColored(
				ImVec4(0.62f, 0.65f, 0.7f, 1.f), "[%s]", Line.strTimestamp.c_str());
			ImGui::SameLine();
			ImGui::TextWrapped("%s", Line.strText.c_str());
		}
		if (!m_LogLines.empty())
			ImGui::SetScrollHereY(1.f);
		ImGui::EndChild();

		/* Input row -- always drawn (the fade-out above is what hides it now, not focus
		state), matching the reference where the bar stays visible between messages. */
		const ImVec2 vRowTopLeft(vLogTopLeft.x, vLogBotRight.y);

		const ImVec2 vNormalMin = vRowTopLeft;
		const ImVec2 vNormalMax(
			vNormalMin.x + NORMAL_BAR_WIDTH * fScaleX,
			vNormalMin.y + NORMAL_BAR_HEIGHT * fScaleY);
		if (ID3D11ShaderResourceView* pNormalBar =
			m_pTextureCache->Get_Or_Load("UI/Chat/Normal Bar.png"))
		{
			pDrawList->AddImage(pNormalBar, vNormalMin, vNormalMax);
		}
		{
			constexpr const char_t* NORMAL_LABEL = "\xec\x9d\xbc\xeb\xb0\x98";
			const ImVec2 vLabelSize = ImGui::CalcTextSize(NORMAL_LABEL);
			pDrawList->AddText(
				ImVec2(
					vNormalMin.x + (vNormalMax.x - vNormalMin.x - vLabelSize.x) * 0.5f,
					vNormalMin.y + (vNormalMax.y - vNormalMin.y - vLabelSize.y) * 0.5f),
				IM_COL32(255, 255, 255, 255), NORMAL_LABEL);
		}

		/* Input Bar stretches to the log panel's own right edge instead of using its native
		width, so the bottom row lines up flush with the log above it. */
		const ImVec2 vInputMin(
			vNormalMax.x + BAR_GAP * fScaleX, vRowTopLeft.y);
		const ImVec2 vInputMax(
			vLogBotRight.x,
			vInputMin.y + INPUT_BAR_HEIGHT * fScaleY);
		if (ID3D11ShaderResourceView* pInputBar =
			m_pTextureCache->Get_Or_Load("UI/Chat/Input Bar.png"))
		{
			pDrawList->AddImage(pInputBar, vInputMin, vInputMax);
		}

		const char_t* pImeIconPath = Is_KoreanInputActive() ?
			"UI/Chat/Korean Input.png" : "UI/Chat/English Input.png";
		const ImVec2 vImeMax(
			vInputMax.x - IME_ICON_INSET_X * fScaleX,
			vInputMin.y + (vInputMax.y - vInputMin.y + IME_ICON_HEIGHT * fScaleY) * 0.5f);
		const ImVec2 vImeMin(
			vImeMax.x - IME_ICON_WIDTH * fScaleX,
			vImeMax.y - IME_ICON_HEIGHT * fScaleY);
		if (ID3D11ShaderResourceView* pImeIcon =
			m_pTextureCache->Get_Or_Load(pImeIconPath))
		{
			pDrawList->AddImage(pImeIcon, vImeMin, vImeMax);
		}

		if (m_bFocusPending)
		{
			ImGui::SetKeyboardFocusHere();
			m_bFocusPending = false;
		}

		ImGui::SetCursorScreenPos(
			ImVec2(vInputMin.x + 6.f * fScaleX, vInputMin.y + 4.f * fScaleY));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushItemWidth(
			(vImeMin.x - vInputMin.x) - 12.f * fScaleX);
		const bool_t submitted = ImGui::InputText(
			"##ChatInput", m_InputBuffer, INPUT_BUFFER_SIZE,
			ImGuiInputTextFlags_EnterReturnsTrue);
		/* Real games never show Windows' own floating IME composition box -- they draw the
		still-composing (uncommitted) text inline themselves, right after what's already typed.
		WM_IME_SETCONTEXT (imgui_impl_win32.cpp) already suppresses the OS box; this is the other
		half, same technique as CLevel_CharacterSelect::Render_CreateCharacterModal. */
		if (ImGui::IsItemActive())
		{
			const wchar_t* pComposition = Engine::CImGuiLayer::Get_ImeCompositionString();
			if (nullptr != pComposition && L'\0' != pComposition[0])
			{
				const ImVec2 vCommittedSize = ImGui::CalcTextSize(m_InputBuffer);
				const ImVec2 vCompositionPos(
					vInputMin.x + 6.f * fScaleX + vCommittedSize.x,
					vInputMin.y + 4.f * fScaleY);
				char compositionUtf8[64] = {};
				::WideCharToMultiByte(CP_UTF8, 0, pComposition, -1,
					compositionUtf8, sizeof(compositionUtf8), nullptr, nullptr);
				const ImVec2 vCompositionSize = ImGui::CalcTextSize(compositionUtf8);
				pDrawList->AddText(vCompositionPos, IM_COL32(255, 255, 255, 255), compositionUtf8);
				pDrawList->AddLine(
					ImVec2(vCompositionPos.x, vCompositionPos.y + vCompositionSize.y),
					ImVec2(vCompositionPos.x + vCompositionSize.x, vCompositionPos.y + vCompositionSize.y),
					IM_COL32(255, 255, 255, 200));
			}
		}
		ImGui::PopItemWidth();
		ImGui::PopStyleColor(3);

		/* Actively typing keeps the window from fading out mid-sentence even past 30s, and
		explicitly re-asserts the engine's keyboard block every frame the box is active --
		CPlayerController normally picks this up itself via IsKeyboardInputBlocked()/
		WantTextInput (set from MainApp::Update()'s own WantsCaptureKeyboard() check), but that
		read happens once per frame before this Render() call updates it, so a fast Q/W/E/R
		press right as focus lands could still slip through on that frame. Setting it directly
		here removes that gap. */
		const bool_t bChatInputActive = ImGui::IsItemActive();
		if (bChatInputActive)
		{
			m_HideDeadline = std::chrono::steady_clock::now() + HIDE_AFTER;
			CGameInstance::Get().SetInputBlocked(true, false);
		}

		if (submitted)
		{
			const string strLine(m_InputBuffer);
			if (strLine.empty())
			{
				Close_Input();
			}
			else
			{
				const filesystem::path soundPath = CRuntimeAssetRoot::Resolve(
					L"Sound/UI/System/sys_chat_text_enter1__926892930.wav");
				CGameInstance::Get().Play_Sound(soundPath.wstring(), 1.f);

				/* Local echo only -- Shared's C2S_CHAT/S2C_CHAT packet types are declared but
				have no Server relay behind them yet, so this does not leave the process.
				Keeps the input open and refocused so a chat session can send several lines
				without re-pressing Enter to reopen each time. */
				const time_t rawTime = time(nullptr);
				tm localTime{};
				localtime_s(&localTime, &rawTime);
				char_t strTimestamp[8];
				snprintf(strTimestamp, sizeof(strTimestamp), "%02d:%02d",
					localTime.tm_hour, localTime.tm_min);

				m_LogLines.push_back(CHAT_LOG_LINE{ strTimestamp, strLine });
				if (m_LogLines.size() > MAX_LOG_LINES)
					m_LogLines.erase(m_LogLines.begin());
				m_InputBuffer[0] = '\0';
				m_bFocusPending = true;
				m_HideDeadline = std::chrono::steady_clock::now() + HIDE_AFTER;
			}
		}
	}
	ImGui::End();
}

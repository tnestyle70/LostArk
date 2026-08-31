#include "ChatWindowView.h"

#include "GameInstance.h"
#include "ImGuiLayer.h"
#include "PlayerCommandSink.h"
#include "RuntimeAssetRoot.h"
#include "UIInputRouter.h"
#include "UILayoutRuntime.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
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

	constexpr const char_t* CHAT_SLOTS[] =
	{
		"Chat_LogPanelBg", "Chat_NormalBar", "Chat_InputBar", "Chat_ImeIcon",
	};

	/* Authored tint of Chat_LogPanelBg -- the drawlist multiplied this same 130/255 alpha on
	top of the art's own baked ~75%, so it is restored with Set_SlotTint rather than
	Set_SlotVisible(true)'s opaque white. */
	const float4_t LOG_PANEL_TINT{ 1.f, 1.f, 1.f, 130.f / 255.f };

	bool_t Convert_Utf8ToWide(const string& strUtf8, wstring_t& outWide)
	{
		if (strUtf8.empty())
			return false;
		const int32_t iLength = ::MultiByteToWideChar(
			CP_UTF8, 0, strUtf8.c_str(), -1, nullptr, 0);
		if (iLength <= 1)
			return false;
		outWide.assign(static_cast<size_t>(iLength - 1), L'\0');
		::MultiByteToWideChar(
			CP_UTF8, 0, strUtf8.c_str(), -1, outWide.data(), iLength);
		return true;
	}
}

Client::CChatWindowView::CChatWindowView(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pView{ make_unique<CUILayoutRuntime>(
		pDevice, pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/Chat/ChatWindow_Layout.json") }
{
	/* A CUI_Sprite is visible from construction; this window starts faded out. */
	Hide_AllSlots();
}

Client::CChatWindowView::~CChatWindowView()
{
}

void Client::CChatWindowView::Hide_AllSlots()
{
	if (nullptr == m_pView)
		return;
	for (const char_t* pSlotId : CHAT_SLOTS)
		m_pView->Set_SlotVisible(pSlotId, false);
}

bool_t Client::CChatWindowView::Is_Visible() const
{
	return std::chrono::steady_clock::now() < m_HideDeadline;
}

void Client::CChatWindowView::Open_Input()
{
	m_bInputOpen = true;
	m_bFocusPending = true;
	m_HideDeadline = std::chrono::steady_clock::now() + HIDE_AFTER;
	/* Takes the WM_CHAR stream for this field -- the same capture the Create Character
	nickname box uses, replacing what ImGui::SetKeyboardFocusHere/InputText did. */
	CUIInputRouter::Get().Start_TextInput();
}

void Client::CChatWindowView::Close_Input()
{
	m_bInputOpen = false;
	m_bFocusPending = false;
	m_InputBuffer[0] = '\0';
	m_InputDraftW.clear();
	CUIInputRouter::Get().Stop_TextInput();
}

void Client::CChatWindowView::Render(
	const std::shared_ptr<IPlayerCommandSink>& pCommandSink)
{
	if (nullptr == m_pView)
		return;

	/* The whole window (log + input bar) disappears once nothing has kept it alive for
	HIDE_AFTER -- a submitted message, opening the input, or having it actively focused all
	push the deadline back out. */
	if (!Is_Visible())
	{
		Hide_AllSlots();
		if (m_bInputOpen)
			Close_Input();
		return;
	}

	m_pView->Set_SlotTint("Chat_LogPanelBg", LOG_PANEL_TINT);
	m_pView->Set_SlotVisible("Chat_NormalBar", true);
	m_pView->Set_SlotVisible("Chat_InputBar", true);
	m_pView->Set_SlotVisible("Chat_ImeIcon", true);
	m_pView->Set_SlotTexture("Chat_ImeIcon", Is_KoreanInputActive() ?
		"UI/Chat/Korean Input.png" : "UI/Chat/English Input.png");

	m_bFocusPending = false;
	if (!m_bInputOpen)
		return;

	/* Actively typing keeps the window from fading out mid-sentence even past 30s, and
	re-asserts the engine's keyboard block every frame the field owns input -- MainApp's own
	per-frame SetInputBlocked runs before this, so a fast Q/W/E/R press right as focus lands
	could otherwise slip through on that frame. */
	m_HideDeadline = std::chrono::steady_clock::now() + HIDE_AFTER;
	CGameInstance::Get().SetInputBlocked(true, false);

	/* Committed text arrives as ordinary WM_CHAR units -- Hangul included, via the IME's own
	GCS_RESULTSTR -> WM_IME_CHAR -> WM_CHAR chain -- so no IME handling is needed for it here;
	the still-composing string is drawn separately in RenderText. Backspace/Enter/Escape ride
	the same stream ('\b'/'\r'/27), the classic Win32 edit loop. */
	const auto Fn_EraseLastCodePoint = [this]()
	{
		if (m_InputDraftW.empty())
			return;
		size_t iErase = 1;
		/* A supplementary-plane character is two UTF-16 units -- erase the whole pair, or the
		leftover half re-encodes as garbage. */
		if (m_InputDraftW.size() >= 2 &&
			m_InputDraftW.back() >= 0xDC00 && m_InputDraftW.back() <= 0xDFFF &&
			m_InputDraftW[m_InputDraftW.size() - 2] >= 0xD800 &&
			m_InputDraftW[m_InputDraftW.size() - 2] <= 0xDBFF)
		{
			iErase = 2;
		}
		m_InputDraftW.resize(m_InputDraftW.size() - iErase);
	};

	bool_t submitted = false;
	bool_t escaped = false;
	bool_t textChanged = false;
	const wstring_t typed = CUIInputRouter::Get().Take_TypedChars();
	for (const wchar_t ch : typed)
	{
		if (L'\r' == ch || L'\n' == ch)
		{
			submitted = true;
		}
		else if (L'\x1b' == ch)
		{
			escaped = true;
		}
		else if (L'\b' == ch)
		{
			if (!m_InputDraftW.empty())
			{
				Fn_EraseLastCodePoint();
				textChanged = true;
			}
		}
		else if (ch >= L' ' && L'\x7f' != ch)
		{
			m_InputDraftW.push_back(ch);
			textChanged = true;
		}
	}

	if (textChanged)
	{
		/* Re-encode into the UTF-8 buffer the send path and the log lines use. If the draft
		outgrew the buffer, drop the newest code point(s) until it fits -- the same hard stop
		InputText's fixed byte buffer imposed at the same size. */
		for (;;)
		{
			if (m_InputDraftW.empty())
			{
				m_InputBuffer[0] = '\0';
				break;
			}
			char_t utf8[INPUT_BUFFER_SIZE]{};
			const int32_t iBytes = ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
				m_InputDraftW.c_str(), -1, utf8,
				static_cast<int32_t>(std::size(utf8)), nullptr, nullptr);
			if (iBytes > 0)
			{
				memcpy(m_InputBuffer, utf8, sizeof(utf8));
				break;
			}
			/* Doesn't fit (or a stray lone surrogate slipped in) -- trim and retry. */
			Fn_EraseLastCodePoint();
		}
	}

	/* Escape drops focus without hiding the window early, exactly as before. */
	if (escaped)
	{
		Close_Input();
		return;
	}
	if (!submitted)
		return;

	const string strLine(m_InputBuffer);
	if (strLine.empty())
	{
		Close_Input();
		return;
	}

	const filesystem::path soundPath = CRuntimeAssetRoot::Resolve(
		L"Sound/UI/System/sys_chat_text_enter1__926892930.wav");
	CGameInstance::Get().Play_Sound(soundPath.wstring(), 1.f);

	if (nullptr != pCommandSink)
		pCommandSink->Request_SendChat(strLine);

	/* Local echo happens regardless of send success -- this is your own scrollback, not the
	head bubble (that reads the Server's broadcast back, same as everyone else's). Keeps the
	input open so a chat session can send several lines without re-pressing Enter each time. */
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
	m_InputDraftW.clear();
	m_HideDeadline = std::chrono::steady_clock::now() + HIDE_AFTER;
}

void Client::CChatWindowView::RenderText()
{
	if (nullptr == m_pView || !Is_Visible())
		return;

	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	if (fRefWidth <= 0.f || fRefHeight <= 0.f)
		return;
	const f32_t fScaleX = vViewportSize.x / fRefWidth;
	const f32_t fScaleY = vViewportSize.y / fRefHeight;
	const f32_t fUiScale = (std::min)(fScaleX, fScaleY);

	constexpr f32_t TEXT_HEIGHT = 13.f;
	/* Returns the drawn advance in screen pixels so a following piece starts where this one
	ended -- position and advance both live in screen space, since fScaleX and fUiScale differ
	on a non-16:9 viewport and mixing the two would drift. */
	const auto Fn_DrawLeft = [&](f32_t fScreenX, f32_t fScreenCenterY, const wchar_t* pText,
		f32_t fTargetHeight, const fvector_t& vColor) -> f32_t
	{
		const float2_t vMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pText);
		if (vMeasured.y <= 0.f)
			return 0.f;
		const f32_t fScale = (fTargetHeight / vMeasured.y) * fUiScale;
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pText,
			float2_t(fScreenX, fScreenCenterY),
			vColor, 0.f, float2_t(0.f, 0.5f), fScale);
		return vMeasured.x * fScale;
	};

	/* Scrollback log -- oldest at the top, newest at the bottom edge of the panel, which is
	what the old auto-scrolled child window showed. */
	f32_t fLogX = 0.f, fLogY = 0.f, fLogW = 0.f, fLogH = 0.f;
	if (m_pView->Get_SlotRect("Chat_LogPanelBg", fLogX, fLogY, fLogW, fLogH) &&
		!m_LogLines.empty())
	{
		constexpr f32_t LINE_HEIGHT = 15.f;
		const size_t iMaxLines = fLogH > 0.f ?
			static_cast<size_t>(fLogH / LINE_HEIGHT) : 0u;
		const size_t iFirst = (iMaxLines > 0u && m_LogLines.size() > iMaxLines) ?
			m_LogLines.size() - iMaxLines : 0u;
		f32_t fLineY = fLogY + 6.f + LINE_HEIGHT * 0.5f;
		for (size_t i = iFirst; i < m_LogLines.size(); ++i)
		{
			const CHAT_LOG_LINE& Line = m_LogLines[i];
			f32_t fScreenX = (fLogX + 6.f) * fScaleX;
			const f32_t fScreenY = fLineY * fScaleY;

			const wstring_t strStamp =
				L"[" + wstring_t(Line.strTimestamp.begin(), Line.strTimestamp.end()) + L"] ";
			fScreenX += Fn_DrawLeft(fScreenX, fScreenY, strStamp.c_str(), TEXT_HEIGHT,
				XMVectorSet(0.62f, 0.65f, 0.7f, 1.f));

			wstring_t strText;
			if (Convert_Utf8ToWide(Line.strText, strText))
				Fn_DrawLeft(fScreenX, fScreenY, strText.c_str(), TEXT_HEIGHT, Colors::White);

			fLineY += LINE_HEIGHT;
		}
	}

	// "일반" channel label, centered on its own bar.
	f32_t fNormalX = 0.f, fNormalY = 0.f, fNormalW = 0.f, fNormalH = 0.f;
	if (m_pView->Get_SlotRect(
		"Chat_NormalBar", fNormalX, fNormalY, fNormalW, fNormalH))
	{
		constexpr const wchar_t* NORMAL_LABEL = L"\xC77C\xBC18";
		const float2_t vMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), NORMAL_LABEL);
		if (vMeasured.y > 0.f)
		{
			CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), NORMAL_LABEL,
				float2_t((fNormalX + fNormalW * 0.5f) * fScaleX,
					(fNormalY + fNormalH * 0.5f) * fScaleY),
				Colors::White, 0.f, float2_t(0.5f, 0.5f),
				(TEXT_HEIGHT / vMeasured.y) * fUiScale);
		}
	}

	/* Typed text + the IME's still-composing syllable + a blinking caret, over the input bar.
	Only while the field owns input -- the bar itself stays visible between messages. */
	f32_t fInputX = 0.f, fInputY = 0.f, fInputW = 0.f, fInputH = 0.f;
	if (!m_bInputOpen || !m_pView->Get_SlotRect(
		"Chat_InputBar", fInputX, fInputY, fInputW, fInputH))
	{
		return;
	}

	const f32_t fCenterScreenY = (fInputY + fInputH * 0.5f) * fScaleY;
	f32_t fCursorScreenX = (fInputX + 6.f) * fScaleX;
	if (!m_InputDraftW.empty())
	{
		fCursorScreenX += Fn_DrawLeft(fCursorScreenX, fCenterScreenY,
			m_InputDraftW.c_str(), TEXT_HEIGHT, Colors::White);
	}
	/* In-progress (uncommitted) Hangul straight from the OS IME, gold so it reads as
	not-yet-committed -- the inline preview the old InputText overlay drew. */
	const wchar_t* pComposition = Engine::CImGuiLayer::Get_ImeCompositionString();
	if (nullptr != pComposition && L'\0' != pComposition[0])
	{
		fCursorScreenX += Fn_DrawLeft(fCursorScreenX, fCenterScreenY,
			pComposition, TEXT_HEIGHT, Colors::Gold);
	}
	const int64_t iHalfSeconds =
		std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now().time_since_epoch()).count() / 500;
	if (0 == (iHalfSeconds % 2))
		Fn_DrawLeft(fCursorScreenX + 1.f, fCenterScreenY, L"|", TEXT_HEIGHT, Colors::White);
}

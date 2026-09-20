#include "ChatWindowView.h"
#pragma push_macro("new")
#undef new
#include <DirectXColors.h>
#pragma pop_macro("new")

#include "GameInstance.h"
#include "ImGuiLayer.h"
#include "Network/PacketType.h"
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

	/* Every drawn piece of the retail widget (Tools/LpkPipeline/build_chat_ui.py): the 20% black
	log panel, its two scroll buttons, the channel combobox arrow, the tab strip and the three
	title buttons. The text boxes are markers the text pass draws into. */
	constexpr const char_t* CHAT_SLOTS[] =
	{
		"Chat_LogPanel", "Chat_LogPanelArt", "Chat_ScrollUp", "Chat_ScrollUpArrow",
		"Chat_ScrollDown", "Chat_ScrollDownArrow", "Chat_ComboBg", "Chat_ComboArrow",
		"Chat_InputBoxOuter", "Chat_InputBoxInner", "Chat_LockButton", "Chat_OptionButton",
		"Chat_MinimizeButton", "Chat_Tab0", "Chat_TabAdd", "Chat_TabAddPlus", "Chat_LangIcon",
	};

	/* chattingControl's textField is YG760 14 px on the 1920x1080 stage. The panel itself is
	authored larger than retail (build_chat_ui.py's WIDGET_SCALE), so the lines follow it by the
	same factor -- otherwise small text would float in a bigger box. */
	constexpr f32_t WIDGET_SCALE = 1.25f;
	constexpr f32_t CHAT_TEXT_STAGE_PX = 14.f * WIDGET_SCALE;
	constexpr f32_t STAGE_TO_REF = 2.f / 3.f;
	/* textField [86] is fontHeight 280 with leading 40, i.e. a 16 stage px line. */
	constexpr f32_t LOG_LINE_HEIGHT = 16.f * WIDGET_SCALE * STAGE_TO_REF;

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
	m_SlotIds = m_pView->Get_SlotIds();
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
	m_bSwallowOpenEnter = true;
	m_HideDeadline = std::chrono::steady_clock::now() + HIDE_AFTER;
	/* Takes the WM_CHAR stream for this field -- the same capture the Create Character
	nickname box uses, replacing what ImGui::SetKeyboardFocusHere/InputText did. */
	CUIInputRouter::Get().Start_TextInput();
}

void Client::CChatWindowView::Close_Input()
{
	m_bInputOpen = false;
	m_bFocusPending = false;
	m_bSwallowOpenEnter = false;
	m_InputBuffer[0] = '\0';
	m_InputDraftW.clear();
	CUIInputRouter::Get().Stop_TextInput();
}

void Client::CChatWindowView::Append_ReceivedLine(
	const string& strNickname, const string& strText)
{
	if (strText.empty())
		return;
	const time_t rawTime = time(nullptr);
	tm localTime{};
	localtime_s(&localTime, &rawTime);
	char_t strTimestamp[8];
	snprintf(strTimestamp, sizeof(strTimestamp), "%02d:%02d",
		localTime.tm_hour, localTime.tm_min);

	m_LogLines.push_back(CHAT_LOG_LINE{ strTimestamp, strNickname, strText });
	if (m_LogLines.size() > MAX_LOG_LINES)
		m_LogLines.erase(m_LogLines.begin());
	/* A new line pins the view back to the bottom, as the retail list does, and keeps the
	window on screen so an arriving message is actually readable. */
	m_iLogScrollBack = 0u;
	m_HideDeadline = std::chrono::steady_clock::now() + HIDE_AFTER;
}

void Client::CChatWindowView::Update_TitleButtons()
{
	if (nullptr == m_pView)
		return;
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefW = m_pView->Get_ResolutionWidth();
	const f32_t fRefH = m_pView->Get_ResolutionHeight();
	f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
	if (m_pView->Get_SlotRect("Chat_LockButton", fX, fY, fW, fH) &&
		Router.Is_Clicked(fX, fY, fW, fH, fRefW, fRefH))
	{
		m_bLocked = !m_bLocked;
		m_HideDeadline = std::chrono::steady_clock::now() + HIDE_AFTER;
	}
	if (m_pView->Get_SlotRect("Chat_MinimizeButton", fX, fY, fW, fH) &&
		Router.Is_Clicked(fX, fY, fW, fH, fRefW, fRefH))
	{
		m_bMinimised = !m_bMinimised;
		m_HideDeadline = std::chrono::steady_clock::now() + HIDE_AFTER;
	}
	/* An unlocked window shows the lock dimmed, the way the retail toggle reads. */
	m_pView->Set_SlotTint("Chat_LockButton", m_bLocked ?
		float4_t(1.f, 1.f, 1.f, 1.f) : float4_t(1.f, 1.f, 1.f, 0.5f));
	/* Minimised: only the input row and the title buttons stay. */
	for (const char_t* pSlotId : { "Chat_LogPanel", "Chat_LogPanelArt", "Chat_ScrollUp",
		"Chat_ScrollUpArrow", "Chat_ScrollDown", "Chat_ScrollDownArrow", "Chat_Tab0" })
		m_pView->Set_SlotVisible(pSlotId, !m_bMinimised);
}

void Client::CChatWindowView::Update_LogScroll()
{
	if (nullptr == m_pView)
		return;
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefW = m_pView->Get_ResolutionWidth();
	const f32_t fRefH = m_pView->Get_ResolutionHeight();
	int32_t iLines = 0;
	f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
	if (m_pView->Get_SlotRect("Chat_LogPanel", fX, fY, fW, fH) &&
		Router.Is_Hovered(fX, fY, fW, fH, fRefW, fRefH))
		iLines += Router.Get_MouseWheelNotches();
	if (m_pView->Get_SlotRect("Chat_ScrollUp", fX, fY, fW, fH) &&
		Router.Is_Clicked(fX, fY, fW, fH, fRefW, fRefH))
		++iLines;
	if (m_pView->Get_SlotRect("Chat_ScrollDown", fX, fY, fW, fH) &&
		Router.Is_Clicked(fX, fY, fW, fH, fRefW, fRefH))
		--iLines;
	if (0 == iLines)
		return;

	/* Only the lines that do not fit can be scrolled to. */
	f32_t fTextX = 0.f, fTextY = 0.f, fTextW = 0.f, fTextH = 0.f;
	size_t iVisible = m_LogLines.size();
	if (m_pView->Get_SlotRect("Chat_LogTextBox", fTextX, fTextY, fTextW, fTextH) && fTextH > 0.f)
		iVisible = static_cast<size_t>(fTextH / LOG_LINE_HEIGHT);
	const size_t iMaxScroll = m_LogLines.size() > iVisible ? m_LogLines.size() - iVisible : 0u;
	const int64_t iNext = static_cast<int64_t>(m_iLogScrollBack) + iLines;
	m_iLogScrollBack = static_cast<size_t>((std::max<int64_t>)(0,
		(std::min<int64_t>)(iNext, static_cast<int64_t>(iMaxScroll))));
	m_HideDeadline = std::chrono::steady_clock::now() + HIDE_AFTER;
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
		m_Drag.Reset();
		return;
	}

	/* Drag by the log panel whenever the window is shown (focused or not); a drag keeps the
	window alive the same way typing does. */
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_bLocked && !m_bMinimised &&
			m_pView->Get_SlotRect("Chat_LogPanel", fX, fY, fWidth, fHeight) &&
			m_Drag.Update(*m_pView, m_SlotIds, fX, fY, fWidth, fHeight))
			m_HideDeadline = std::chrono::steady_clock::now() + HIDE_AFTER;
	}

	for (const char_t* pSlotId : CHAT_SLOTS)
		m_pView->Set_SlotVisible(pSlotId, true);
	/* One channel ("일반") means one tab and no filter window, so the add-tab and option
	buttons have nothing to open. */
	for (const char_t* pSlotId : { "Chat_TabAdd", "Chat_TabAddPlus", "Chat_OptionButton" })
		m_pView->Set_SlotVisible(pSlotId, false);
	/* languageIcon_mc: frame 2 is the Korean glyph, frame 1 is an empty box, so Latin input
	shows nothing at all in retail. The field has to own input for either to mean anything. */
	m_pView->Set_SlotVisible("Chat_LangIcon", m_bInputOpen && Is_KoreanInputActive());
	/* The input row's own art only belongs on screen while the field is open. */
	for (const char_t* pSlotId : { "Chat_InputBoxOuter", "Chat_InputBoxInner" })
		m_pView->Set_SlotVisible(pSlotId, m_bInputOpen);
	Update_TitleButtons();
	if (!m_bMinimised)
		Update_LogScroll();

	m_bFocusPending = false;
	if (!m_bInputOpen)
		return;

	/* While the field owns input, the cursor over the log/input bar belongs to the chat window
	-- a click there must not reach CPlayerController (CMainApp folds the claim into its gameplay
	mouse block; last frame's, since this runs after its Update). */
	for (const char* pSlotId : { "Chat_LogPanel", "Chat_InputTextBox" })
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (m_pView->Get_SlotRect(pSlotId, fX, fY, fWidth, fHeight) &&
			CUIInputRouter::Get().Is_Hovered(fX, fY, fWidth, fHeight,
				m_pView->Get_ResolutionWidth(), m_pView->Get_ResolutionHeight()))
		{
			CUIInputRouter::Get().Claim_Mouse_This_Frame();
		}
	}

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
			/* The keystroke that opens the field also delivers its own WM_CHAR, and the capture
			is already live when that message is pumped. Submitting it would close the empty line
			again a frame later, which is what made Enter feel like it toggled at random -- the
			field looked open, the next key went to gameplay. The opening Enter is eaten once. */
			if (m_bSwallowOpenEnter)
			{
				m_bSwallowOpenEnter = false;
				continue;
			}
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
			/* C2S_CHAT carries at most MAX_CHAT_TEXT_BYTES of UTF-8, so the field stops there
			instead of letting a line grow into a message the Server would reject. */
			m_InputDraftW.push_back(ch);
			const int32_t iBytes = ::WideCharToMultiByte(CP_UTF8, 0, m_InputDraftW.c_str(),
				static_cast<int32_t>(m_InputDraftW.size()), nullptr, 0, nullptr, nullptr);
			if (iBytes > static_cast<int32_t>(LostArk::Shared::MAX_CHAT_TEXT_BYTES))
				m_InputDraftW.pop_back();
			else
				textChanged = true;
		}
	}

	/* Anything that arrived after the opening keystroke means it is behind us. */
	if (!typed.empty())
		m_bSwallowOpenEnter = false;

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

	/* No local echo: the room broadcasts the line back to its sender too, and that broadcast
	carries the Server's nickname, so the log gets it through Append_ReceivedLine with the same
	"<name> : <text>" shape as everyone else's. Focus stays so a chat session can send several
	lines without re-pressing Enter each time. */
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

	/* chatting.gfx draws every chat line, the channel label and the typed text with YG760 at
	14 stage px, which is this on the 1280x720 reference. */
	constexpr f32_t TEXT_HEIGHT = CHAT_TEXT_STAGE_PX * STAGE_TO_REF;
	/* Returns the drawn advance in screen pixels so a following piece starts where this one
	ended -- position and advance both live in screen space, since fScaleX and fUiScale differ
	on a non-16:9 viewport and mixing the two would drift. */
	const auto Fn_DrawLeft = [&](f32_t fScreenX, f32_t fScreenCenterY, const wchar_t* pText,
		f32_t fTargetHeight, const fvector_t& vColor) -> f32_t
	{
		const float2_t vMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YG760"), pText);
		if (vMeasured.y <= 0.f)
			return 0.f;
		const f32_t fScale = (fTargetHeight / vMeasured.y) * fUiScale;
		/* Under the character info window (drawn over this panel) the piece is skipped but
		still advances -- this text pass runs after every sprite. */
		if (!CUIInputRouter::Get().Is_UnderTopWindow(fScreenX, fScreenCenterY))
		{
			CGameInstance::Get().Draw_Text(TEXT("Font_YG760"), pText,
				float2_t(fScreenX, fScreenCenterY),
				vColor, 0.f, float2_t(0.f, 0.5f), fScale);
		}
		return vMeasured.x * fScale;
	};

	/* Scrollback log -- oldest at the top, newest at the bottom edge of the panel, which is
	what the old auto-scrolled child window showed. */
	f32_t fLogX = 0.f, fLogY = 0.f, fLogW = 0.f, fLogH = 0.f;
	if (!m_bMinimised &&
		m_pView->Get_SlotRect("Chat_LogTextBox", fLogX, fLogY, fLogW, fLogH) &&
		!m_LogLines.empty())
	{
		constexpr f32_t LINE_HEIGHT = LOG_LINE_HEIGHT;
		const size_t iMaxLines = fLogH > 0.f ?
			static_cast<size_t>(fLogH / LINE_HEIGHT) : 0u;
		/* Update_LogScroll's offset counts lines back from the newest. */
		const size_t iEnd = m_LogLines.size() - (std::min)(m_iLogScrollBack, m_LogLines.size());
		const size_t iFirst = (iMaxLines > 0u && iEnd > iMaxLines) ? iEnd - iMaxLines : 0u;
		f32_t fLineY = fLogY + 6.f + LINE_HEIGHT * 0.5f;
		for (size_t i = iFirst; i < iEnd; ++i)
		{
			const CHAT_LOG_LINE& Line = m_LogLines[i];
			f32_t fScreenX = (fLogX + 6.f) * fScaleX;
			const f32_t fScreenY = fLineY * fScaleY;

			const wstring_t strStamp =
				L"[" + wstring_t(Line.strTimestamp.begin(), Line.strTimestamp.end()) + L"] ";
			fScreenX += Fn_DrawLeft(fScreenX, fScreenY, strStamp.c_str(), TEXT_HEIGHT,
				XMVectorSet(0.62f, 0.65f, 0.7f, 1.f));

			/* "<name> : <text>", the name in the channel's own colour like the retail log. */
			wstring_t strSpeaker;
			if (!Line.strSpeaker.empty() && Convert_Utf8ToWide(Line.strSpeaker, strSpeaker))
			{
				strSpeaker += L" : ";
				fScreenX += Fn_DrawLeft(fScreenX, fScreenY, strSpeaker.c_str(), TEXT_HEIGHT,
					XMVectorSet(0.85f, 0.88f, 0.93f, 1.f));
			}

			wstring_t strText;
			if (Convert_Utf8ToWide(Line.strText, strText))
				Fn_DrawLeft(fScreenX, fScreenY, strText.c_str(), TEXT_HEIGHT, Colors::White);

			fLineY += LINE_HEIGHT;
		}
	}

	/* The one tab is the one channel: ChattingTabButton's own label is YoonGasiIIM 16 stage px
	in #919ca6, which is this on the reference. */
	f32_t fTabX = 0.f, fTabY = 0.f, fTabW = 0.f, fTabH = 0.f;
	if (!m_bMinimised && m_pView->Get_SlotRect("Chat_Tab0TextBox", fTabX, fTabY, fTabW, fTabH))
	{
		constexpr const wchar_t* TAB_LABEL = L"\xC77C\xBC18";
		const float2_t vTabMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), TAB_LABEL);
		if (vTabMeasured.y > 0.f)
		{
			CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), TAB_LABEL,
				float2_t((fTabX + fTabW * 0.5f) * fScaleX, (fTabY + fTabH * 0.5f) * fScaleY),
				XMVectorSet(145.f / 255.f, 156.f / 255.f, 166.f / 255.f, 1.f), 0.f,
				float2_t(0.5f, 0.5f),
				(16.f * WIDGET_SCALE * STAGE_TO_REF / vTabMeasured.y) * fUiScale);
		}
	}

	// "일반" channel label, centered on its own bar.
	f32_t fNormalX = 0.f, fNormalY = 0.f, fNormalW = 0.f, fNormalH = 0.f;
	if (m_pView->Get_SlotRect(
		"Chat_ChannelTextBox", fNormalX, fNormalY, fNormalW, fNormalH))
	{
		constexpr const wchar_t* NORMAL_LABEL = L"\xC77C\xBC18";
		const float2_t vMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YG760"), NORMAL_LABEL);
		const float2_t vNormalPos((fNormalX + fNormalW * 0.5f) * fScaleX,
			(fNormalY + fNormalH * 0.5f) * fScaleY);
		if (vMeasured.y > 0.f && !CUIInputRouter::Get().Is_UnderTopWindow(vNormalPos.x, vNormalPos.y))
		{
			CGameInstance::Get().Draw_Text(TEXT("Font_YG760"), NORMAL_LABEL,
				vNormalPos,
				Colors::White, 0.f, float2_t(0.5f, 0.5f),
				(TEXT_HEIGHT / vMeasured.y) * fUiScale);
		}
	}

	/* Typed text + the IME's still-composing syllable + a blinking caret, over the input bar.
	Only while the field owns input -- the bar itself stays visible between messages. */
	f32_t fInputX = 0.f, fInputY = 0.f, fInputW = 0.f, fInputH = 0.f;
	if (!m_bInputOpen || !m_pView->Get_SlotRect(
		"Chat_InputTextBox", fInputX, fInputY, fInputW, fInputH))
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

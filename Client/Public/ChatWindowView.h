#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CUITextureCache;
class IPlayerCommandSink;

/* Release-safe chat overlay: a scrollback log over the extracted semi-transparent panel art
(UI/Chat/LogPanelBg.png) and an always-visible input bar, both of which fade out together after
HIDE_AFTER_SECONDS of no chat activity (no message sent and not actively focused) and reappear
on the next Enter/submit/focus. Enter appends the typed line to the local log immediately (no
round trip needed for your own scrollback) and, on a real non-empty send, also submits it via
IPlayerCommandSink::Request_SendChat so the Server relays it to the rest of the room -- that
relay is what CWorldPlayerChatBubbleView reads to show a bubble above senders' heads. */
class CChatWindowView final
{
public:
	explicit CChatWindowView(ComPtr<ID3D11Device> pDevice);
	~CChatWindowView();

public:
	/* True while the input line is focused for typing (not whether it is drawn -- the input
	bar itself always draws unless the whole window has faded from inactivity). */
	bool_t Is_Open() const { return m_bInputOpen; }

	/* Focuses the input line next Render() and marks the window active (resets the
	inactivity fade timer). Caller is expected to only call this on an Enter edge while
	nothing else already wants text input (mirrors CSkillWindowView's K-toggle guard), so
	Enter cannot hijack an unrelated focused text field. */
	void Open_Input();

	/* Drops focus from the input line without submitting whatever was being typed. */
	void Close_Input();

	/* No-op once HIDE_AFTER_SECONDS have passed since the last activity (a submitted message
	or the input line being focused) with no active focus -- the whole window (log + input
	bar) disappears until the next Open_Input()/submit. Otherwise draws the scrollback log and
	the input bar every call. Submitting a non-empty line (Enter inside the focused InputText)
	timestamps it, appends it to the log, clears the buffer, refreshes the fade timer, and
	keeps focus for the next line -- Escape drops focus via Close_Input() without hiding the
	window early (the fade timer still governs that). */
	void Render(const std::shared_ptr<IPlayerCommandSink>& pCommandSink);

private:
	struct CHAT_LOG_LINE
	{
		string strTimestamp;
		string strText;
	};

private:
	static constexpr size_t MAX_LOG_LINES = 20;
	static constexpr size_t INPUT_BUFFER_SIZE = 256;
	static constexpr std::chrono::seconds HIDE_AFTER{ 30 };

private:
	unique_ptr<CUITextureCache> m_pTextureCache;

	bool_t m_bInputOpen = false;
	bool_t m_bFocusPending = false;
	char_t m_InputBuffer[INPUT_BUFFER_SIZE] = {};
	vector<CHAT_LOG_LINE> m_LogLines;
	std::chrono::steady_clock::time_point m_HideDeadline =
		std::chrono::steady_clock::now() + HIDE_AFTER;
};

NS_END

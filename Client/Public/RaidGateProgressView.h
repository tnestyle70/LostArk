#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <memory>
#include <string>

NS_BEGIN(Client)

class CUILayoutRuntime;

/* Commander raid gate progress on screen: the top-left "<raid> [<difficulty>]" panel (retail
EpicGateCommanderProgressFrame) that stays up for the whole raid, with one icon per gate
(cleared = crest with the green check, current = blue glow, later = blue door) and one button
whose meaning follows the raid state: "restart" while a gate is being fought, "dungeon
progress" once a gate short of the last is cleared, "exit" once the last gate is cleared.
Restart and progress open a confirm dialog and then a party vote; a member sees the
accept / decline prompt for whichever vote the leader started.

Presentation only. Who is the leader, whether a gate is cleared and what the vote decided all
come from the Server's S2C_GATE_PROGRESS_STATE through the owning Level; this view draws the
state it is given and reports which button was pressed. Art and rects come from
Data/UI/RaidGateProgress/*.json (build_raid_gate_progress_ui.py). */
class CRaidGateProgressView final
{
public:
	enum class BUTTON { NONE, RESTART, PROGRESS, EXIT };
	enum class PROMPT { NONE, CONFIRM_ADVANCE, CONFIRM_RESTART, VOTE_ADVANCE, VOTE_RESTART };
	enum class INTENT { NONE, PROPOSE_ADVANCE, PROPOSE_RESTART, EXIT, ACCEPT, DECLINE };

public:
	void Initialize(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iOwnerLevelIndex);
	/* Raid name and difficulty text for the panel and how many gate icons to show (1..3). */
	void Set_Raid(const wstring_t& strName, const wstring_t& strDifficulty, uint8_t iGateCount);
	/* 1-based current gate (0 = none raised yet) and the cleared bit mask, per gate. */
	void Set_Progress(uint8_t iCurrentGate, uint8_t iClearedMask);
	/* Which button the panel shows. bEnabled false draws it but ignores the pointer (a party
	   member who is not the leader still sees the same panel). */
	void Set_Button(BUTTON eButton, bool_t bEnabled);
	void Open_Prompt(PROMPT eKind, const wstring_t& strProposerName);
	void Close_Prompt();
	PROMPT Get_Prompt() const { return m_ePrompt; }
	/* Centre-top line for a few seconds (vote failed, gate cleared). */
	void Show_Notice(const wstring_t& strText, f32_t fSeconds);
	/* Owns the pointer over the button and while a prompt is up. Returns what was decided
	   this frame; the widget button itself only opens the confirm prompt (EXIT is direct). */
	INTENT Update(f32_t fTimeDelta);
	/* LOA-font text pass; call from the Level's Render after its own sprites. */
	void Render_Text() const;

private:
	void Set_PromptVisible(bool_t bVisible);
	INTENT Update_Prompt();
	INTENT Update_Button();
	const wchar_t* Button_Text() const;

private:
	unique_ptr<CUILayoutRuntime>	m_pWidget;
	unique_ptr<CUILayoutRuntime>	m_pPrompt;
	wstring_t						m_strName;
	wstring_t						m_strDifficulty;
	wstring_t						m_strProposer;
	wstring_t						m_strNotice;
	f32_t							m_fNoticeSeconds = 0.f;
	uint8_t							m_iGateCount = 0u;
	uint8_t							m_iCurrentGate = 0u;
	uint8_t							m_iClearedMask = 0u;
	BUTTON							m_eButton = BUTTON::NONE;
	bool_t							m_bButtonEnabled = false;
	bool_t							m_bButtonHovered = false;
	PROMPT							m_ePrompt = PROMPT::NONE;
};

NS_END

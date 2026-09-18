#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <memory>
#include <string>

NS_BEGIN(Client)

class CUILayoutRuntime;

/* Commander raid gate progress on screen: the top-left "<raid> [<difficulty>]" panel with one
icon per gate (retail EpicGateCommanderProgressFrame: cleared = crest with the green check,
current = blue glow, later = blue door) and, after a gate clears, the "dungeon progress" prompt
-- the leader / solo player's proceed-or-stop choice, or a member's accept / decline vote.

Presentation only. Who is the leader, whether a gate is cleared and what the vote decided all
come from the Server's S2C_GATE_PROGRESS_STATE through the owning Level; this view draws the
state it is given and reports which button was pressed. Art and rects come from
Data/UI/RaidGateProgress/*.json (build_raid_gate_progress_ui.py). */
class CRaidGateProgressView final
{
public:
	enum class PROMPT { NONE, PROPOSE, VOTE };
	enum class INTENT { NONE, PROPOSE, STOP, ACCEPT, DECLINE };

public:
	void Initialize(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iOwnerLevelIndex);
	/* Raid name and difficulty text for the panel and how many gate icons to show (1..3). */
	void Set_Raid(const wstring_t& strName, const wstring_t& strDifficulty, uint8_t iGateCount);
	/* 1-based current gate (0 = none raised yet) and the cleared bit mask, per gate. */
	void Set_Progress(uint8_t iCurrentGate, uint8_t iClearedMask);
	void Open_Prompt(PROMPT eKind, const wstring_t& strProposerName);
	void Close_Prompt();
	PROMPT Get_Prompt() const { return m_ePrompt; }
	/* Centre-top line for a few seconds (vote failed, gate cleared). */
	void Show_Notice(const wstring_t& strText, f32_t fSeconds);
	/* Owns the pointer while the prompt is up. Returns the button pressed this frame. */
	INTENT Update(f32_t fTimeDelta);
	/* LOA-font text pass; call from the Level's Render after its own sprites. */
	void Render_Text() const;

private:
	void Set_PromptVisible(bool_t bVisible);

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
	PROMPT							m_ePrompt = PROMPT::NONE;
};

NS_END

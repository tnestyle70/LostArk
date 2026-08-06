#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketMessages.h"

#include <string>
#include <unordered_map>
#include <vector>

NS_BEGIN(Client)

class CUITextureCache;
class CHUDRuntimeView;

/* Runtime (Release-safe) skill window overlay, toggled with K. Shows the local player's full
skill roster from Data/UI/SkillWindow/<ClassId>.skillroster.json -- a display-only catalog,
separate from Data/Balance/PlayerSkills.json's gameplay-active subset -- next to static
tripod and rune panels that mirror the reference layout's structure but carry no selection
state yet (CLAUDE.md scopes actual tripod/rune application out of this pass). Quick-slot
registration (clicking a skill onto a HUD slot) is a separate follow-up, not wired here. */
class CSkillWindowView final
{
public:
	/* Owns a second CHUDRuntimeView pointed at Data/UI/SkillWindow/SkillWindow_Layout.json
	(authored the same way as HUD_Layout.json, via CHUDLayoutTool's "Skill Window" document)
	so the panel/tripod-plate art you place there draws as this window's own background
	instead of a plain ImGui panel. */
	CSkillWindowView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	~CSkillWindowView();

public:
	bool_t Is_Open() const { return m_bOpen; }
	void Toggle() { m_bOpen = !m_bOpen; }

	/* No-op while closed. characterClass selects which class's roster document loads and
	which rows show, since each class owns a different skill set. */
	void Render(LostArk::Shared::CHARACTER_CLASS_ID characterClass);

private:
	struct SKILL_ROSTER_ENTRY
	{
		LostArk::Shared::SKILL_ID iSkillId = LostArk::Shared::INVALID_SKILL_ID;
		string strDisplayName;
		string strStance;
		string strIconPath;
	};

private:
	/* Parses Data/UI/SkillWindow/<ClassId>.skillroster.json for characterClass if it has not
	been loaded yet this run. A document that fails parse/validate leaves that class absent
	from m_Rosters, so Render shows a status line instead of a half-built list. */
	const vector<SKILL_ROSTER_ENTRY>* Get_Roster(
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);

private:
	unique_ptr<CUITextureCache> m_pTextureCache;
	unique_ptr<CHUDRuntimeView> m_pBackgroundView;
	unordered_map<int32_t, vector<SKILL_ROSTER_ENTRY>> m_Rosters;
	unordered_map<int32_t, string> m_RosterLoadStatus;

	bool_t m_bOpen = false;
	LostArk::Shared::SKILL_ID m_SelectedSkillId =
		LostArk::Shared::INVALID_SKILL_ID;
};

NS_END

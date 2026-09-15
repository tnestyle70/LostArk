#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "UIWindowDrag.h"

#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CUILayoutRuntime;
struct HUD_PLAYER_STATE;

/* Retail combat analyzer in its reduced mode (combatanalysisframe.gfx CombatAnalysisFrame,
470x114 stage px) rebuilt as CUI_Sprite slots (Data/UI/CombatAnalysis/CombatAnalysis_Layout.json,
written by Tools/LpkPipeline/build_combat_analysis_ui.py) under LEVEL::STATIC, drawn over the
combat HUD and under the runtime windows.

Shows the fight timer and four stat tiles -- damage, DPS, stagger dealt, counter successes
(the project's choice; retail shows damage / DPS / 1-minute damage / 1-minute DPS in the same
four category colours). Every number is CCombatHUDViewModel::Get_CombatAnalysis(), the local
player's own DAMAGE_EVENTs totalled from the Server snapshot; nothing is measured here. Always
shown while the combat HUD shows (user decision for now); X hides it until the next fight
starts. The title strip drags the frame. The retail ghost toggle / maximise / record buttons
are drawn as art only. */
class CCombatAnalysisFrameView final
{
public:
	CCombatAnalysisFrameView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	~CCombatAnalysisFrameView();

public:
	/* Per frame from Update_CombatHUD while its level/player gate passes. */
	void Update(f32_t fTimeDelta, const HUD_PLAYER_STATE& Player);
	/* LOA-font text pass -- after CImGuiLayer::EndFrame(), before the windows' own passes. */
	void Render_Text();
	/* Forces every owned sprite invisible (HUD gate closed). */
	void Hide();

private:
	struct STAT_TILE
	{
		string strId;
		wstring strTitle;
		bool_t bCount = false;
	};

private:
	void Load_Data();
	void Update_Chrome();
	/* Value text for a tile: 1.23만 / 4.56억 / ... (retail sys.analyzer.ui_unit_*), a plain
	integer below ten thousand, "-" while no fight has started. */
	wstring Format_Amount(uint64_t iValue) const;
	bool_t Get_FrameOrigin(f32_t& fX, f32_t& fY) const;
	f32_t Ref_X(f32_t fRetailX) const;
	f32_t Ref_Y(f32_t fRetailY) const;
	void Draw_Label(const wstring_t& strFont, const wstring& strText, f32_t fRetailX,
		f32_t fRetailY, f32_t fRetailPx, fvector_t vColor, const float2_t& vOrigin);

private:
	ComPtr<ID3D11Device>			m_pDevice;
	ComPtr<ID3D11DeviceContext>		m_pContext;
	unique_ptr<CUILayoutRuntime>	m_pView;
	vector<string>					m_SlotIds;
	CUIWindowDrag					m_Drag;
	vector<STAT_TILE>				m_Stats;
	wstring							m_strValueNull;
	vector<wstring>					m_Units;

	/* Retail px -> reference px, read from the CA_Bg slot. */
	f32_t m_fRetailScale = 2.f / 3.f;
	bool_t m_bHidden = false;
	/* Raid the X hid (its first-hit tick); a different start tick shows the frame again. */
	uint32_t m_iHiddenFightStartTick = 0u;
	/* Snapshot for the label pass. */
	uint32_t m_iServerTick = 0u;
	bool_t m_bShown = false;
};

NS_END

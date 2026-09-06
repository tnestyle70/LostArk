#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "CombatHUDViewModel.h"

#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CUILayoutRuntime;

/* Boss pattern check gauge (retail indicator.gfx BaseIndicator.immuneGauge): the small
orange bar that appears under a boss while a pattern demands accumulated damage, and
drains as the party lands it. It is driven only by the Server-replicated
HUD_BOSS_STATE response progress/threshold -- a threshold of 0 means no such pattern is
running and the gauge hides. Slots are authored in Data/UI/BossUI/ImmuneGauge_Layout.json
relative to "Immune_Anchor"; every frame the anchor is re-projected from the boss's
replicated position (or stays at the authored anchor for the Debug boss preview). */
class CBossImmuneGaugeView final
{
public:
	CBossImmuneGaugeView(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iGameObjectLevelIndex);
	~CBossImmuneGaugeView();

public:
	/* bAllowed is the caller's level/window gate (same one the boss health bar uses). */
	void Update(f32_t fTimeDelta, const HUD_BOSS_STATE& Boss, bool_t bAllowed);
	void Hide();

private:
	void Capture_SlotOffsets();

private:
	struct SLOT_OFFSET
	{
		string	strId;
		f32_t	fDx = 0.f;
		f32_t	fDy = 0.f;
	};

	unique_ptr<CUILayoutRuntime>	m_pView;
	vector<SLOT_OFFSET>				m_SlotOffsets;
	f32_t							m_fAnchorX = 0.f;
	f32_t							m_fAnchorY = 0.f;
	bool_t							m_bVisible = false;
};

NS_END

#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "CombatHUDViewModel.h"

#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CCharacter;
class CUILayoutRuntime;

/* KoukuSaydon madness gauge drawn under the local character (retail indicator.gfx
BaseIndicator.madnessGauge): a 3-state clown-face frame, a black track, a coloured
fill strip and a white section-pass flash, all authored in
Data/UI/KoukuSaydon/MadnessGauge_Layout.json relative to the "Madness_Anchor" slot.
Every frame the anchor is re-projected from the character's nameplate head point
and the whole group is moved with Set_SlotPosition, so the authored rects stay the
retail offsets. The view owns no gameplay value: gauge/maximum come from
HUD_KOUKU_GIMMICK_STATE and the state thresholds from KoukuHudModes.json. */
class CKoukuMadnessGaugeView final
{
public:
	CKoukuMadnessGaugeView(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iGameObjectLevelIndex);
	~CKoukuMadnessGaugeView();

public:
	/* pLocalCharacter may be null (not spawned yet); the gauge then hides. */
	void Update(
		f32_t fTimeDelta,
		const shared_ptr<CCharacter>& pLocalCharacter,
		const HUD_KOUKU_GIMMICK_STATE& State);
	void Hide();
	/* Reference pixels after projection; head height remains in world meters. */
	void Get_Position(float2_t& screenOffset, f32_t& feetOffsetMeters) const;
	bool_t Set_Position(const float2_t& screenOffset, f32_t feetOffsetMeters);
	bool_t Save_Position(string& status);
	bool_t Reload_Position(string& status);

private:
	HRESULT Load_Config();
	/* Authored offset of every slot from the anchor slot, captured once so the
	per-frame move is a pure translation. */
	void Capture_SlotOffsets();
	int32_t Resolve_State(uint32_t iGauge) const;

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
	/* Gauge value at which state 1 / state 2 begin (retail UIGaugeStatus1/2). */
	vector<uint32_t>				m_Thresholds;
	/* Fill strip texture per state; a state past the list keeps the last strip. */
	vector<string>					m_FillTextures;
	f32_t							m_fFeetOffsetMeters = 0.f;
	float2_t m_vScreenOffset = {};
	float2_t m_vSavedScreenOffset = {};
	f32_t m_fSavedFeetOffsetMeters = 0.f;
	bool_t							m_bConfigLoaded = false;
	int32_t							m_iLastState = -1;
	f32_t							m_fFlashAlpha = 0.f;
	bool_t							m_bVisible = false;
};

NS_END

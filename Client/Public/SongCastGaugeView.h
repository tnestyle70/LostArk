#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "CombatHUDViewModel.h"

#include <memory>
#include <string>

NS_BEGIN(Client)

class CUILayoutRuntime;

/* The retail action casting bar (commonobject.gfx CommonActionTimingBar: dark pointed plate
379x44, orange-red fill track 352x22, white $YG760 13 px caption centred over the track) shown
while the local player is locked in the square-hole song (PLAYER_ACTION_STATE::SQUAREHOLE_SONG).
The fill is the replicated action age over the Shared SQUAREHOLE_SONG_DURATION_MS; the caption is
Data/UI/WorldMap/WorldMapPanels.json strings.songCasting. Layout:
Data/UI/WorldMap/SongCastGauge_Layout.json, written by Tools/LpkPipeline/build_worldmap_ui.py.
The same song also drives the screen blackout that hides the Server-owned landing: the screen
finishes fading to black when the song ends (Shared SQUAREHOLE_BLACKOUT_FADE_MS before it),
stays black while the Server holds the action, and fades back in once the action ends.
The view decides nothing: it reads CCombatHUDViewModel's player state and draws. */
class CSongCastGaugeView final
{
public:
	CSongCastGaugeView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	~CSongCastGaugeView();

public:
	/* Per frame with the current HUD player state; hides itself outside the song action. */
	void Update(f32_t fTimeDelta, const HUD_PLAYER_STATE& Player);
	/* LOA-font caption pass -- after CImGuiLayer::EndFrame() like the other runtime windows. */
	void Render_Text();
	void Hide();

private:
	void Load_Strings();
	void Update_Fade(f32_t fTimeDelta, bool_t bInSong, int32_t iAgeTicks);

private:
	ComPtr<ID3D11Device>			m_pDevice;
	ComPtr<ID3D11DeviceContext>		m_pContext;
	unique_ptr<CUILayoutRuntime>	m_pView;
	wstring_t						m_strCaption;
	bool_t							m_bVisible = false;
	/* Edge state so the song sound plays once per lock, not every frame. */
	uint32_t						m_iShownActionStartTick = 0u;
	f32_t							m_fFill = 0.f;
	/* Full-screen black quad (the same authored slot the Kouku trigger fade uses). */
	unique_ptr<CUILayoutRuntime>	m_pFadeView;
	f32_t							m_fFadeAlpha = 0.f;
	bool_t							m_bFadingOut = false;
	bool_t							m_bFadeShown = false;
};

NS_END

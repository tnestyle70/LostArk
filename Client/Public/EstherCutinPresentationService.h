#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)
class CUILayoutRuntime;

/* Full-screen Esther skill cutin: the retail Esther_Skill_<Name> Bink movie
(1920x1080 with alpha, character bottom-right, pinned to the screen's
bottom-right corner by epicSkillAni.gfx) cooked to a 1280x720 DXT5 flipbook and
played once through one UI/Esther/EstherCutin.json slot. Owned by CMainApp next
to the other STATIC-level UI documents; CCombatHUDViewModel's
HUD_ESTHER_CUTIN_REQUEST generation is the only trigger, and the movie to play
comes from the NPC catalog's cutinMovie of the strike's archetype. */
class CEstherCutinPresentationService final
{
public:
	CEstherCutinPresentationService(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	~CEstherCutinPresentationService();

public:
	/* Per-frame from CMainApp::Update: consumes a new request, advances the
	flipbook, and hides it once every frame has shown or the level changes. */
	void Update(f32_t fTimeDelta);

#ifdef _DEBUG
	/* Re-issues the strike request for an archetype so the movie replays. */
	static bool_t Debug_Preview(const std::string& archetypeId);
#endif

private:
	void Begin(const std::string& archetypeId);
	void Show();
	void End();

private:
	unique_ptr<CUILayoutRuntime> m_pView;
	uint32_t m_iConsumedGeneration = 0;
	uint32_t m_iActiveLevelIndex = 0;
	/* Armed at the strike spawn; the flipbook is shown once m_fElapsedSeconds
	passes m_fDelaySeconds and hidden again after m_fDurationSeconds more. */
	vector<string> m_Frames;
	f32_t m_fFps = 0.f;
	f32_t m_fDelaySeconds = 0.f;
	f32_t m_fDurationSeconds = 0.f;
	f32_t m_fElapsedSeconds = 0.f;
	bool_t m_isActive = false;
	bool_t m_isShowing = false;
};

NS_END

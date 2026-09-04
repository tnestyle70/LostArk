#include "EstherCutinPresentationService.h"

#include "ActorCatalog.h"
#include "CombatHUDViewModel.h"
#include "GameInstance.h"
#include "UILayoutRuntime.h"

#include <algorithm>
#include <cstdio>
#include <vector>

namespace
{
	constexpr const char_t* CUTIN_SLOT_ID = "Esther_Cutin";
	/* A stalled frame (loader hitch, window drag) must not skip movie frames. */
	constexpr f32_t CUTIN_MAX_STEP_SECONDS = 0.1f;
}

Client::CEstherCutinPresentationService::CEstherCutinPresentationService(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	m_pView = std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/Esther/EstherCutin.json");
	/* The authored layer is a transparent placeholder, but the slot still stays
	hidden until a strike so an idle sprite isn't drawn every frame. */
	m_pView->Set_SlotVisible(CUTIN_SLOT_ID, false);
}

Client::CEstherCutinPresentationService::~CEstherCutinPresentationService() = default;

void Client::CEstherCutinPresentationService::Update(f32_t fTimeDelta)
{
	if (nullptr == m_pView)
		return;

	const HUD_ESTHER_CUTIN_REQUEST& request =
		CCombatHUDViewModel::Get().Get_EstherCutinRequest();
	if (request.iGeneration != m_iConsumedGeneration)
	{
		m_iConsumedGeneration = request.iGeneration;
		End();
		if (0u != request.iGeneration)
			Begin(request.strArchetypeId);
	}
	if (!m_isActive)
		return;

	if (CGameInstance::Get().Get_CurrentLevelID() != m_iActiveLevelIndex)
	{
		End();
		return;
	}

	const f32_t fStep = std::clamp(fTimeDelta, 0.f, CUTIN_MAX_STEP_SECONDS);
	m_fElapsedSeconds += fStep;
	if (!m_isShowing)
	{
		if (m_fElapsedSeconds < m_fDelaySeconds)
			return;
		Show();
	}
	m_pView->Update(fStep);
	if (m_fElapsedSeconds >= m_fDelaySeconds + m_fDurationSeconds)
		End();
}

void Client::CEstherCutinPresentationService::Begin(const std::string& archetypeId)
{
	const NPC_ACTOR_ENTRY* pActor = CActorCatalog::Find_Npc(archetypeId);
	if (nullptr == pActor || 0u == pActor->cutinMovie.frameCount ||
		pActor->cutinMovie.fps <= 0.f)
	{
		return;
	}
	const NPC_ACTOR_ENTRY::CUTIN_MOVIE& movie = pActor->cutinMovie;

	m_Frames.clear();
	m_Frames.reserve(movie.frameCount);
	char_t szFrame[512] = {};
	for (uint32_t i = 0; i < movie.frameCount; ++i)
	{
		(void)sprintf_s(szFrame, "%s_%03u.dds", movie.framePrefix.c_str(), i);
		m_Frames.emplace_back(szFrame);
	}
	m_fFps = movie.fps;
	m_iActiveLevelIndex = CGameInstance::Get().Get_CurrentLevelID();
	m_fDelaySeconds = static_cast<f32_t>(movie.delayMs) / 1000.f;
	m_fDurationSeconds = static_cast<f32_t>(movie.frameCount) / movie.fps;
	m_fElapsedSeconds = 0.f;
	m_isActive = true;
	m_isShowing = false;
	if (0.f == m_fDelaySeconds)
		Show();
}

void Client::CEstherCutinPresentationService::Show()
{
	m_pView->Set_SlotAnimation(CUTIN_SLOT_ID, m_Frames, m_fFps, false);
	m_pView->Set_SlotVisible(CUTIN_SLOT_ID, true);
	m_isShowing = true;
}

void Client::CEstherCutinPresentationService::End()
{
	if (!m_isActive)
		return;
	if (m_isShowing)
	{
		m_pView->Set_SlotVisible(CUTIN_SLOT_ID, false);
		/* Drop the frame list so the slot falls back to its transparent placeholder;
		the texture cache keeps the DDS frames resident for the next strike. */
		m_pView->Set_SlotAnimation(CUTIN_SLOT_ID, {}, 0.f, false);
	}
	m_Frames.clear();
	m_fFps = 0.f;
	m_fDelaySeconds = 0.f;
	m_fDurationSeconds = 0.f;
	m_fElapsedSeconds = 0.f;
	m_isActive = false;
	m_isShowing = false;
}

#ifdef _DEBUG
bool_t Client::CEstherCutinPresentationService::Debug_Preview(
	const std::string& archetypeId)
{
	const NPC_ACTOR_ENTRY* pActor = CActorCatalog::Find_Npc(archetypeId);
	if (nullptr == pActor || 0u == pActor->cutinMovie.frameCount)
		return false;
	CCombatHUDViewModel::Get().Apply_EstherCutinAction(archetypeId);
	return true;
}
#endif

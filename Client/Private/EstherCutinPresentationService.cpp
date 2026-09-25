#include "EstherCutinPresentationService.h"

#include "ActorCatalog.h"
#include "CombatHUDViewModel.h"
#include "GameInstance.h"
#include "UILayoutRuntime.h"
#include "UITextureCache.h"

#include <algorithm>
#include <cstdio>
#include <iterator>
#include <mutex>
#include <set>
#include <utility>
#include <vector>

namespace
{
	constexpr const char_t* CUTIN_SLOT_ID = "Esther_Cutin";
	/* A stalled frame (loader hitch, window drag) must not skip movie frames. */
	constexpr f32_t CUTIN_MAX_STEP_SECONDS = 0.1f;

	std::mutex g_PreloadMutex;
	std::set<std::string> g_PreloadedArchetypes;
	std::vector<std::pair<std::string, ComPtr<ID3D11ShaderResourceView>>> g_PendingFrames;

	void BuildFramePaths(const Client::NPC_ACTOR_ENTRY::CUTIN_MOVIE& movie, std::vector<std::string>& outFrames)
	{
		outFrames.clear();
		outFrames.reserve(movie.frameCount);
		char_t szFrame[512] = {};
		for (uint32_t i = 0; i < movie.frameCount; ++i)
		{
			(void)sprintf_s(szFrame, "%s_%03u.dds", movie.framePrefix.c_str(), i);
			outFrames.emplace_back(szFrame);
		}
	}
}

std::size_t Client::CEstherCutinPresentationService::Preload_Frames(
	ID3D11Device* pDevice, const std::string& archetypeId)
{
	const NPC_ACTOR_ENTRY* pActor = CActorCatalog::Find_Npc(archetypeId);
	if (nullptr == pDevice || nullptr == pActor || 0u == pActor->cutinMovie.frameCount)
		return 0u;
	{
		std::lock_guard<std::mutex> lock(g_PreloadMutex);
		if (!g_PreloadedArchetypes.insert(archetypeId).second)
			return 0u;
	}
	std::vector<std::string> frames;
	BuildFramePaths(pActor->cutinMovie, frames);
	std::vector<std::pair<std::string, ComPtr<ID3D11ShaderResourceView>>> loaded;
	loaded.reserve(frames.size());
	for (std::string& frame : frames)
	{
		ComPtr<ID3D11ShaderResourceView> pSRV = CUITextureCache::Load_Texture(pDevice, frame);
		if (nullptr != pSRV)
			loaded.emplace_back(std::move(frame), std::move(pSRV));
	}
	const std::size_t count = loaded.size();
	std::lock_guard<std::mutex> lock(g_PreloadMutex);
	std::move(loaded.begin(), loaded.end(), std::back_inserter(g_PendingFrames));
	return count;
}

void Client::CEstherCutinPresentationService::Adopt_PreloadedFrames()
{
	std::vector<std::pair<std::string, ComPtr<ID3D11ShaderResourceView>>> pending;
	{
		std::lock_guard<std::mutex> lock(g_PreloadMutex);
		pending.swap(g_PendingFrames);
	}
	for (auto& [path, pSRV] : pending)
		m_pView->Adopt_Texture(path, std::move(pSRV));
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

	Adopt_PreloadedFrames();
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

	BuildFramePaths(movie, m_Frames);
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

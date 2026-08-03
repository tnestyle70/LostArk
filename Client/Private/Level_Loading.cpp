#include "imgui.h"

#include "Level_Loading.h"
#include "ClientLaunchOptions.h"
#include "LevelRegistry.h"
#include "Loader.h"
#include "NetworkManager.h"
#include "SceneTransitionService.h"

#include "GameInstance.h"

CLevel_Loading::CLevel_Loading(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CLevel { pDevice, pContext }
{
}

CLevel_Loading::~CLevel_Loading()
{
}

HRESULT CLevel_Loading::Initialize(LEVEL eNextLevelID)
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	m_eNextLevelID = eNextLevelID;

	m_pLoader = CLoader::Create(m_pDevice, m_pContext, m_eNextLevelID);

	if (nullptr == m_pLoader)
		return E_FAIL;

	return S_OK;
}

void CLevel_Loading::Update(f32_t fTimeDelta)
{
	if (m_isRetryRequested)
	{
		m_isRetryRequested = false;
		Retry_LobbyLoad();
		return;
	}

	if (nullptr == m_pLoader)
		return;

	if (m_pLoader->Failed())
	{
		Recover_FromFailure(m_pLoader->Get_Result());
		return;
	}

	const bool_t isContinueRequested =
		LEVEL::LOBBY == m_eNextLevelID ||
		CClientLaunchOptions::Get().isAutoActivate ||
		(!CGameInstance::Get().IsKeyboardInputBlocked() &&
			(GetKeyState(VK_RETURN) & 0x8000));

	if (isContinueRequested &&
		true == m_pLoader->Finished())
	{
		unique_ptr<CLevel> pNewLevel =
			CLevelRegistry::Create_Level(
				m_eNextLevelID,
				m_pDevice,
				m_pContext);

		if (nullptr == pNewLevel)
		{
			Recover_FromFailure(E_FAIL);
			return;
		}

		if (FAILED(CGameInstance::Get().Change_Level(ETOUI(m_eNextLevelID), move(pNewLevel))))
		{
			Recover_FromFailure(E_FAIL);
			return;
		}

		return;
	}

	__super::Update(fTimeDelta);
}

HRESULT CLevel_Loading::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	if (m_isFailureReported && LEVEL::LOBBY == m_eNextLevelID)
	{
		ImGui::SetNextWindowPos(
			ImVec2(24.f, 24.f), ImGuiCond_Always);
		if (ImGui::Begin(
			"Loading recovery",
			nullptr,
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoCollapse))
		{
			ImGui::TextWrapped(
				"Lobby resources could not be loaded. The active world session was closed and partial resources were rolled back.");
			if (ImGui::Button("Retry Lobby"))
				m_isRetryRequested = true;
		}
		ImGui::End();
	}

#ifdef _DEBUG
	if(nullptr != m_pLoader)
		m_pLoader->Print_Text();
#endif

	return S_OK;
}

void CLevel_Loading::Recover_FromFailure(const HRESULT result)
{
	if (m_isFailureReported)
		return;

	m_isFailureReported = true;
	CSceneTransitionService::Report_LoadFailure(result);
	CNetworkManager::Get().Close_ServerConnection();

	if (FAILED(CGameInstance::Get().Clear_Resources(
		ETOUI(m_eNextLevelID))))
	{
		OutputDebugStringA(
			"[Level_Loading] Failed to clear partial target resources.\n");
	}

	OutputDebugStringA(
		"[Level_Loading] Load failed; session closed and partial resources rolled back.\n");

	if (LEVEL::LOBBY != m_eNextLevelID)
		Retry_LobbyLoad();
}

void CLevel_Loading::Retry_LobbyLoad()
{
	unique_ptr<CLevel_Loading> pRecoveryLevel =
		CLevel_Loading::Create(
			m_pDevice,
			m_pContext,
			LEVEL::LOBBY);
	if (nullptr == pRecoveryLevel ||
		FAILED(CGameInstance::Get().Change_Level(
			ETOUI(LEVEL::LOADING),
			move(pRecoveryLevel))))
	{
		CSceneTransitionService::Report_LoadFailure(E_FAIL);
		OutputDebugStringA(
			"[Level_Loading] Failed to start Lobby recovery.\n");
	}
}

unique_ptr<CLevel_Loading> CLevel_Loading::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, LEVEL eNextLevelID)
{
	auto pInstance = unique_ptr<CLevel_Loading>(new CLevel_Loading(pDevice, pContext));

	if (FAILED(pInstance->Initialize(eNextLevelID)))
	{
		OutputDebugStringA("[Level_Loading] Create failed.\n");
		return nullptr;
	}

	return pInstance;
}

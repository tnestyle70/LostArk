#include "imgui.h"

#include "Level_Loading.h"

#include "GameInstance.h"
#include "LevelTransitionService.h"
#include "Loader.h"
#include "NetworkManager.h"

CLevel_Loading::CLevel_Loading(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel{ pDevice, pContext }
{
}

CLevel_Loading::~CLevel_Loading()
{
}

HRESULT CLevel_Loading::Initialize(
	const LEVEL eNextLevelID,
	const LOBBY_COMMAND_TOKEN lobbyCommandToken)
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;
	if (INVALID_LOBBY_COMMAND_TOKEN != lobbyCommandToken &&
		LEVEL::LOBBY != eNextLevelID)
	{
		return E_INVALIDARG;
	}

	m_eNextLevelID = eNextLevelID;
	m_iLobbyCommandToken = lobbyCommandToken;
	m_pLoader = CLoader::Create(m_pDevice, m_pContext, m_eNextLevelID);
	return nullptr == m_pLoader ? E_FAIL : S_OK;
}

void CLevel_Loading::Update(const f32_t fTimeDelta)
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

	if (m_pLoader->Finished() && !m_isActivationRequested)
	{
		if (CLevelTransitionService::Request_Activation(
			m_eNextLevelID,
			"loading.complete",
			m_iLobbyCommandToken))
		{
			m_isActivationRequested = true;
			m_iLobbyCommandToken = INVALID_LOBBY_COMMAND_TOKEN;
		}
		else
		{
			OutputDebugStringA(
				"[Level_Loading] Activation request was rejected; retrying.\n");
		}
	}

	__super::Update(fTimeDelta);
}

HRESULT CLevel_Loading::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	if (nullptr != viewport && nullptr != m_pLoader)
	{
		const std::string loadingStatus = CLoader::Get_ActiveStatus();
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::SetNextWindowPos(
			ImVec2(
				viewport->WorkPos.x + viewport->WorkSize.x * 0.5f,
				viewport->WorkPos.y + 16.f),
			ImGuiCond_Always,
			ImVec2(0.5f, 0.f));
		ImGui::SetNextWindowBgAlpha(0.82f);
		if (ImGui::Begin(
			"Loading progress",
			nullptr,
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing))
		{
			ImGui::TextUnformatted(loadingStatus.c_str());
		}
		ImGui::End();
	}

	if (m_isFailureReported && LEVEL::LOBBY == m_eNextLevelID)
	{
		ImGui::SetNextWindowPos(ImVec2(24.f, 24.f), ImGuiCond_Always);
		if (ImGui::Begin(
			"Loading recovery",
			nullptr,
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoCollapse))
		{
			ImGui::TextWrapped(
				"Lobby resources could not be loaded. Partial resources were rolled back.");
			if (ImGui::Button("Retry Lobby"))
				m_isRetryRequested = true;
		}
		ImGui::End();
	}

#ifdef _DEBUG
	if (nullptr != m_pLoader)
		m_pLoader->Print_Text();
#endif
	return S_OK;
}

void CLevel_Loading::Recover_FromFailure(const HRESULT result)
{
	if (m_isFailureReported)
		return;

	m_isFailureReported = true;
	Cancel_LobbyCommand("target level loading failed");
	CLevelTransitionService::Report_LoadFailure(result);
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

void CLevel_Loading::Cancel_LobbyCommand(const char_t* pReason)
{
	if (INVALID_LOBBY_COMMAND_TOKEN == m_iLobbyCommandToken)
		return;

	CLobbyCommandService::Cancel(m_iLobbyCommandToken, pReason);
	m_iLobbyCommandToken = INVALID_LOBBY_COMMAND_TOKEN;
}

void CLevel_Loading::Retry_LobbyLoad()
{
	if (!CLevelTransitionService::Request_Load(
		LEVEL::LOBBY,
		"loading.recovery"))
	{
		OutputDebugStringA(
			"[Level_Loading] Failed to stage Lobby recovery.\n");
	}
}

unique_ptr<CLevel_Loading> CLevel_Loading::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const LEVEL eNextLevelID,
	const LOBBY_COMMAND_TOKEN lobbyCommandToken)
{
	auto instance = unique_ptr<CLevel_Loading>(
		new CLevel_Loading(pDevice, pContext));
	if (FAILED(instance->Initialize(
		eNextLevelID,
		lobbyCommandToken)))
		return nullptr;
	return instance;
}

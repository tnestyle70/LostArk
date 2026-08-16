#include "imgui.h"

#include "Level_Lobby.h"

#include "CharacterSelectionState.h"
#include "LevelTransitionService.h"
#include "MainApp.h"
#include "NetworkManager.h"

#ifdef _DEBUG
#include "MapEditorWorkspaceService.h"
#endif

namespace
{
	string Describe_ServerEndpoint()
	{
		return CNetworkManager::Resolve_ServerHost() + ":" +
			to_string(CNetworkManager::DEFAULT_SERVER_PORT);
	}

#ifdef _DEBUG
	string Describe_MapEditorServerEndpoint()
	{
		return CNetworkManager::Resolve_MapEditorServerHost() + ":" +
			to_string(CNetworkManager::DEFAULT_SERVER_PORT);
	}
#endif

	const char_t* Get_CharacterClassName(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (characterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER:
			return "Lance Master";
		case CHARACTER_CLASS_ID::GUNSLINGER:
			return "Gunslinger";
		case CHARACTER_CLASS_ID::SLAYER:
			return "Slayer";
		case CHARACTER_CLASS_ID::ARTIST:
			return "Artist";
		case CHARACTER_CLASS_ID::DIMENSIONMASTER:
			return "Dimension Master";
		case CHARACTER_CLASS_ID::WARLORD:
			return "Warlord";
		default:
			return "Not selected";
		}
	}
}

CLevel_Lobby::CLevel_Lobby(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel{ pDevice, pContext }
{
}

CLevel_Lobby::~CLevel_Lobby()
{
}

HRESULT CLevel_Lobby::Initialize()
{
	return __super::Initialize();
}

void CLevel_Lobby::Update(const f32_t fTimeDelta)
{
	/* A refused load or activation drops back here with the socket already
	closed, so without this the bounce is indistinguishable from a dropped
	connection and the reason is thrown away. */
	HRESULT loadFailure = S_OK;
	std::string loadFailureDetail;
	if (CLevelTransitionService::Try_ConsumeLoadFailure(
		loadFailure, loadFailureDetail))
	{
		m_strStatus = loadFailureDetail.empty() ?
			"Stage loading failed. Lobby remains active." :
			"Stage loading failed: " + loadFailureDetail;
	}

	LOBBY_COMMAND command{};
	if (CLobbyCommandService::Try_Consume(command) &&
		!Begin_StageRequest(command) &&
		LOBBY_STAGE::BERN == command.eStage)
	{
		CCharacterSelectionState::Cancel_PendingCreation();
	}

	Consume_EnterRejected();
	Consume_EnterAccepted();

	if (ENTRY_STATE::WAITING_FOR_APPROVAL == m_eEntryState)
	{
		if (!CNetworkManager::Get().Is_Connected())
		{
			Cancel_PendingEntry(
				"Server disconnected before approving entry. Lobby remains active.");
		}
		else if (std::chrono::steady_clock::now() >= m_ApprovalDeadline)
		{
			Cancel_PendingEntry(
				"Server entry approval timed out after 5 seconds. Lobby remains active.");
		}
	}

	__super::Update(fTimeDelta);
}

HRESULT CLevel_Lobby::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

#ifdef _DEBUG
	CMainApp::Update_DebugWindowTitleWithFps(TEXT("LostArk Lobby"));
#endif

	Render_StagePanel();
	return S_OK;
}

void CLevel_Lobby::Consume_EnterRejected()
{
	using namespace LostArk::Shared;
	S2C_ENTER_REJECTED rejected{};
	if (!CNetworkManager::Get().Try_Consume_EnterRejected(rejected))
		return;

	if (ENTRY_STATE::WAITING_FOR_APPROVAL != m_eEntryState ||
		NETWORK_PROTOCOL_VERSION != rejected.iProtocolVersion ||
		rejected.eWorldId != m_ePendingWorldId ||
		ENTER_WORLD_REJECTION_REASON::ROOM_FULL != rejected.eReason)
	{
		Cancel_PendingEntry("Server returned an invalid world-entry rejection.");
		return;
	}

	if (WORLD_ID::VALTAN_ARENA == rejected.eWorldId)
	{
		Cancel_PendingEntry(
			"Valtan raid is full (4/4). Lobby remains active.");
		return;
	}

	Cancel_PendingEntry("The selected world is full. Lobby remains active.");
}

bool_t CLevel_Lobby::Begin_StageRequest(const LOBBY_COMMAND& command)
{
	if (ENTRY_STATE::IDLE != m_eEntryState ||
		CLevelTransitionService::Is_Pending())
	{
		m_strStatus = "Another entry or level transition is already pending.";
		return false;
	}

	LostArk::Shared::WORLD_ID worldId = LostArk::Shared::WORLD_ID::END;
	LEVEL targetLevel = LEVEL::END;
	if (!Resolve_Stage(
		command.eStage,
		command.ePurpose,
		worldId,
		targetLevel))
	{
		m_strStatus = "The selected stage is not registered.";
		return false;
	}

	return Begin_NetworkEntry(worldId, targetLevel, command.ePurpose);
}

bool_t CLevel_Lobby::Begin_NetworkEntry(
	const LostArk::Shared::WORLD_ID eWorldId,
	const LEVEL eTargetLevel,
	const LOBBY_COMMAND_PURPOSE purpose)
{
	if (LOBBY_COMMAND_PURPOSE::END == purpose ||
		(LOBBY_COMMAND_PURPOSE::MAP_EDITOR_WORKSPACE == purpose &&
			LEVEL::DEVELOPMENT != eTargetLevel))
	{
		m_strStatus = "The entry purpose is not valid for the selected stage.";
		return false;
	}
#ifndef _DEBUG
	if (LOBBY_COMMAND_PURPOSE::MAP_EDITOR_WORKSPACE == purpose)
	{
		m_strStatus = "Map Editor workspace is available only in Debug.";
		return false;
	}
#endif

	CHARACTER_ENTRY_IDENTITY identity{};
	if (!CCharacterSelectionState::Try_Resolve_ForWorld(
		eWorldId, identity))
	{
		m_strStatus = LostArk::Shared::WORLD_ID::BERN == eWorldId ?
			"Create a character before entering Bern." :
			"The entry identity could not be resolved.";
		return false;
	}
	const bool_t usesPendingCreation =
		CHARACTER_ENTRY_IDENTITY_SOURCE::PENDING_CREATION ==
		identity.eSource;

	CNetworkManager& networkManager = CNetworkManager::Get();
	networkManager.Close_ServerConnection();
	const string serverHost =
		LOBBY_COMMAND_PURPOSE::MAP_EDITOR_WORKSPACE == purpose ?
		CNetworkManager::Resolve_MapEditorServerHost() :
		CNetworkManager::Resolve_ServerHost();
	if (!networkManager.Connect_To_Server(
		serverHost,
		CNetworkManager::DEFAULT_SERVER_PORT))
	{
		if (usesPendingCreation)
			CCharacterSelectionState::Cancel_PendingCreation();
		m_strStatus = "Server connection failed for " +
			serverHost + ":" +
			to_string(CNetworkManager::DEFAULT_SERVER_PORT) + " (WSA " +
			to_string(networkManager.Get_LastErrorCode()) + ").";
		return false;
	}

	if (!networkManager.Send_EnterWorld(
		eWorldId,
		identity.eCharacterClass,
		identity.strNickname))
	{
		if (usesPendingCreation)
			CCharacterSelectionState::Cancel_PendingCreation();
		m_strStatus = "C2S_ENTER_WORLD send failed (WSA " +
			to_string(networkManager.Get_LastErrorCode()) + ").";
		networkManager.Close_ServerConnection();
		return false;
	}

	m_eEntryState = ENTRY_STATE::WAITING_FOR_APPROVAL;
	m_ePendingWorldId = eWorldId;
	m_ePendingLevel = eTargetLevel;
	m_ePendingPurpose = purpose;
	m_hasPendingCharacterCreationEntry = usesPendingCreation;
	m_ApprovalDeadline =
		std::chrono::steady_clock::now() + std::chrono::seconds(5);
	m_strStatus = "C2S_ENTER_WORLD sent for " +
		identity.strNickname + ". Waiting for server approval.";
	return true;
}

bool_t CLevel_Lobby::Resolve_Stage(
	const LOBBY_STAGE eStage,
	const LOBBY_COMMAND_PURPOSE purpose,
	LostArk::Shared::WORLD_ID& outWorldId,
	LEVEL& outTargetLevel) const
{
	using LostArk::Shared::WORLD_ID;
	outWorldId = WORLD_ID::END;
	outTargetLevel = LEVEL::END;

	switch (eStage)
	{
	case LOBBY_STAGE::CHARACTER_SELECT:
		if (LOBBY_COMMAND_PURPOSE::GAMEPLAY != purpose)
			return false;
		outWorldId = WORLD_ID::CHARACTER_SELECT_ARENA;
		outTargetLevel = LEVEL::CHARACTER_SELECT;
		return true;
	case LOBBY_STAGE::TEST:
		if (LOBBY_COMMAND_PURPOSE::MAP_EDITOR_WORKSPACE == purpose)
		{
			outWorldId = WORLD_ID::TRAINING_GROUND;
			outTargetLevel = LEVEL::DEVELOPMENT;
		}
		else
		{
			outWorldId = WORLD_ID::CHARACTER_SELECT_ARENA;
			outTargetLevel = LEVEL::CHARACTER_SELECT;
		}
		return true;
	case LOBBY_STAGE::VALTAN:
		outWorldId = WORLD_ID::VALTAN_ARENA;
		outTargetLevel = LEVEL::VALTAN_ARENA;
		return true;
	case LOBBY_STAGE::BERN:
		outWorldId = WORLD_ID::BERN;
		outTargetLevel = LEVEL::BERN;
		return true;
	default:
		return false;
	}
}

void CLevel_Lobby::Consume_EnterAccepted()
{
	LostArk::Shared::S2C_ENTER_ACCEPTED accepted{};
	CNetworkManager& networkManager = CNetworkManager::Get();
	if (!networkManager.Try_Consume_EnterAccepted(accepted))
		return;

	if (ENTRY_STATE::WAITING_FOR_APPROVAL != m_eEntryState)
	{
		Cancel_PendingEntry("Unexpected server approval was rejected.");
		return;
	}

	if (accepted.iProtocolVersion != LostArk::Shared::NETWORK_PROTOCOL_VERSION ||
		accepted.eWorldId != m_ePendingWorldId ||
		accepted.iPlayerId == LostArk::Shared::INVALID_PLAYER_ID ||
		accepted.iNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID)
	{
		Cancel_PendingEntry("Server returned an invalid world approval.");
		return;
	}

	const LEVEL approvedLevel = m_ePendingLevel;
#ifdef _DEBUG
	const bool_t opensMapEditorWorkspace =
		LOBBY_COMMAND_PURPOSE::MAP_EDITOR_WORKSPACE ==
		m_ePendingPurpose;
	if (opensMapEditorWorkspace)
		CMapEditorWorkspaceService::Request();
#endif
	if (!CLevelTransitionService::Request_Load(
		approvedLevel,
		"lobby.enter-accepted"))
	{
#ifdef _DEBUG
		if (opensMapEditorWorkspace)
			CMapEditorWorkspaceService::Cancel();
#endif
		Cancel_PendingEntry(CLevelTransitionService::Get_Status());
		return;
	}

#ifdef _DEBUG
	if (opensMapEditorWorkspace)
		networkManager.Close_ServerConnection();
#endif

	m_eEntryState = ENTRY_STATE::IDLE;
	m_ePendingWorldId = LostArk::Shared::WORLD_ID::END;
	m_ePendingLevel = LEVEL::END;
	m_ePendingPurpose = LOBBY_COMMAND_PURPOSE::GAMEPLAY;
	m_hasPendingCharacterCreationEntry = false;
	m_ApprovalDeadline = {};
	m_strStatus = "Server approved the world. Loading the stage.";
}

void CLevel_Lobby::Cancel_PendingEntry(const string& reason)
{
	CNetworkManager::Get().Close_ServerConnection();
	if (m_hasPendingCharacterCreationEntry)
		CCharacterSelectionState::Cancel_PendingCreation();
	m_eEntryState = ENTRY_STATE::IDLE;
	m_ePendingWorldId = LostArk::Shared::WORLD_ID::END;
	m_ePendingLevel = LEVEL::END;
	m_ePendingPurpose = LOBBY_COMMAND_PURPOSE::GAMEPLAY;
	m_hasPendingCharacterCreationEntry = false;
	m_ApprovalDeadline = {};
	m_strStatus = reason;
}

void CLevel_Lobby::Render_StagePanel()
{
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	if (nullptr != viewport)
	{
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::SetNextWindowPos(
			ImVec2(viewport->WorkPos.x + 24.f, viewport->WorkPos.y + 24.f),
			ImGuiCond_Always);
	}

	if (!ImGui::Begin(
		"LostArk Lobby",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::End();
		return;
	}

	CHARACTER_ENTRY_IDENTITY entryIdentity{};
	const bool_t hasEntryIdentity =
		CCharacterSelectionState::Try_Resolve_ForWorld(
			LostArk::Shared::WORLD_ID::CHARACTER_SELECT_ARENA,
			entryIdentity);
	ImGui::Text(
		"Entry character: %s",
		hasEntryIdentity ?
			Get_CharacterClassName(entryIdentity.eCharacterClass) :
			"Unavailable");
	if (hasEntryIdentity)
	{
		ImGui::TextDisabled(
			"Entry nickname: %s",
			entryIdentity.strNickname.c_str());
	}
	const string serverEndpoint = Describe_ServerEndpoint();
	ImGui::TextDisabled("Server: %s", serverEndpoint.c_str());
#ifdef _DEBUG
	const string mapEditorEndpoint = Describe_MapEditorServerEndpoint();
	if (mapEditorEndpoint != serverEndpoint)
	{
		ImGui::TextDisabled(
			"Test (Map Editor): %s", mapEditorEndpoint.c_str());
	}
#endif
	ImGui::Separator();

	const bool_t isBusy = ENTRY_STATE::IDLE != m_eEntryState ||
		CLevelTransitionService::Is_Pending();
	ImGui::BeginDisabled(isBusy);
	if (ImGui::Button("Test"))
	{
#ifdef _DEBUG
		CLobbyCommandService::Request(
			LOBBY_STAGE::TEST,
			LOBBY_COMMAND_PURPOSE::MAP_EDITOR_WORKSPACE);
#else
		CLobbyCommandService::Request(LOBBY_STAGE::TEST);
#endif
	}
	ImGui::SameLine();
	if (ImGui::Button("Character Select"))
		CLobbyCommandService::Request(LOBBY_STAGE::CHARACTER_SELECT);
	ImGui::SameLine();
	if (ImGui::Button("Valtan"))
		CLobbyCommandService::Request(LOBBY_STAGE::VALTAN);
	ImGui::SameLine();
	if (ImGui::Button("Bern"))
		CLobbyCommandService::Request(LOBBY_STAGE::BERN);
	ImGui::EndDisabled();

	ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::End();
}

unique_ptr<CLevel_Lobby> CLevel_Lobby::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto instance = unique_ptr<CLevel_Lobby>(
		new CLevel_Lobby(pDevice, pContext));
	if (FAILED(instance->Initialize()))
		return nullptr;
	return instance;
}

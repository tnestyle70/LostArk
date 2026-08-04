#include "imgui.h"

#include "Level_Lobby.h"

#include "CharacterSelectionState.h"
#include "LevelTransitionService.h"
#include "NetworkManager.h"

#ifdef _DEBUG
#include "MapEditorWorkspaceService.h"
#endif

namespace
{
	constexpr char_t DEFAULT_SERVER_HOST[] = "127.0.0.1";
	constexpr char_t SERVER_HOST_ENVIRONMENT[] = "LOSTARK_SERVER_HOST";
	constexpr std::uint16_t SERVER_PORT = 7777;
	constexpr char_t PLAYER_NICKNAME[] = "Player";
	constexpr LostArk::Shared::CHARACTER_CLASS_ID DEFAULT_ENTRY_CLASS =
		LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER;

	string Resolve_ServerHost()
	{
		char_t configuredHost[64]{};
		const DWORD configuredLength = ::GetEnvironmentVariableA(
			SERVER_HOST_ENVIRONMENT,
			configuredHost,
			static_cast<DWORD>(std::size(configuredHost)));
		if (0 == configuredLength ||
			configuredLength >= std::size(configuredHost) ||
			"0.0.0.0" == string_view{ configuredHost })
		{
			return DEFAULT_SERVER_HOST;
		}

		return configuredHost;
	}

	string Describe_ServerEndpoint()
	{
		return Resolve_ServerHost() + ":" + to_string(SERVER_PORT);
	}

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
		case CHARACTER_CLASS_ID::DIMENSIONIST:
			return "Dimensionist";
		default:
			return "Not selected";
		}
	}

	bool_t Resolve_EntryCharacterClass(
		LostArk::Shared::CHARACTER_CLASS_ID& outCharacterClass,
		bool_t& outUsedDefault)
	{
		outUsedDefault = false;
		if (CCharacterSelectionState::Try_Get_SelectedClass(
			outCharacterClass))
		{
			return true;
		}

		if (!CCharacterSelectionState::Select(DEFAULT_ENTRY_CLASS))
		{
			outCharacterClass =
				LostArk::Shared::CHARACTER_CLASS_ID::END;
			return false;
		}

		outCharacterClass = DEFAULT_ENTRY_CLASS;
		outUsedDefault = true;
		return true;
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
	LOBBY_COMMAND command{};
	if (CLobbyCommandService::Try_Consume(command))
		Begin_StageRequest(command.eStage);

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

	Render_StagePanel();
	return S_OK;
}

bool_t CLevel_Lobby::Begin_StageRequest(const LOBBY_STAGE eStage)
{
	if (ENTRY_STATE::IDLE != m_eEntryState ||
		CLevelTransitionService::Is_Pending())
	{
		m_strStatus = "Another entry or level transition is already pending.";
		return false;
	}

	if (LOBBY_STAGE::CHARACTER_SELECT == eStage)
	{
		if (!CLevelTransitionService::Request_Load(
			LEVEL::CHARACTER_SELECT,
			"lobby.character-select"))
		{
			m_strStatus = CLevelTransitionService::Get_Status();
			return false;
		}

		m_strStatus = "Opening Character Select.";
		return true;
	}

	LostArk::Shared::WORLD_ID worldId = LostArk::Shared::WORLD_ID::END;
	LEVEL targetLevel = LEVEL::END;
	if (!Resolve_Stage(eStage, worldId, targetLevel))
	{
		m_strStatus = "The selected stage is not registered.";
		return false;
	}

	return Begin_NetworkEntry(worldId, targetLevel);
}

bool_t CLevel_Lobby::Begin_NetworkEntry(
	const LostArk::Shared::WORLD_ID eWorldId,
	const LEVEL eTargetLevel)
{
	LostArk::Shared::CHARACTER_CLASS_ID characterClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	bool_t usedDefaultClass = false;
	if (!Resolve_EntryCharacterClass(characterClass, usedDefaultClass))
	{
		m_strStatus = "The default entry class could not be committed.";
		return false;
	}

	CNetworkManager& networkManager = CNetworkManager::Get();
	networkManager.Close_ServerConnection();
	const string serverHost = Resolve_ServerHost();
	if (!networkManager.Connect_To_Server(serverHost, SERVER_PORT))
	{
		m_strStatus = "Server connection failed for " +
			serverHost + ":" + to_string(SERVER_PORT) + " (WSA " +
			to_string(networkManager.Get_LastErrorCode()) + ").";
		return false;
	}

	if (!networkManager.Send_EnterWorld(
		eWorldId,
		characterClass,
		PLAYER_NICKNAME))
	{
		m_strStatus = "C2S_ENTER_WORLD send failed (WSA " +
			to_string(networkManager.Get_LastErrorCode()) + ").";
		networkManager.Close_ServerConnection();
		return false;
	}

	m_eEntryState = ENTRY_STATE::WAITING_FOR_APPROVAL;
	m_ePendingWorldId = eWorldId;
	m_ePendingLevel = eTargetLevel;
	m_ApprovalDeadline =
		std::chrono::steady_clock::now() + std::chrono::seconds(5);
	m_strStatus = usedDefaultClass ?
		"No class was selected. Lance Master was committed and entry approval is pending." :
		"C2S_ENTER_WORLD sent. Waiting for server approval.";
	return true;
}

bool_t CLevel_Lobby::Resolve_Stage(
	const LOBBY_STAGE eStage,
	LostArk::Shared::WORLD_ID& outWorldId,
	LEVEL& outTargetLevel) const
{
	using LostArk::Shared::WORLD_ID;
	outWorldId = WORLD_ID::END;
	outTargetLevel = LEVEL::END;

	switch (eStage)
	{
	case LOBBY_STAGE::TEST:
		outWorldId = WORLD_ID::TRAINING_GROUND;
		outTargetLevel = LEVEL::DEVELOPMENT;
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

	if (accepted.eWorldId != m_ePendingWorldId)
	{
		Cancel_PendingEntry("Server approved a different world. Entry was rejected.");
		return;
	}

	const LEVEL approvedLevel = m_ePendingLevel;
#ifdef _DEBUG
	const bool_t opensMapEditorWorkspace =
		LEVEL::DEVELOPMENT == approvedLevel;
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
	m_ApprovalDeadline = {};
	m_strStatus = "Server approved the world. Loading the stage.";
}

void CLevel_Lobby::Cancel_PendingEntry(const string& reason)
{
	CNetworkManager::Get().Close_ServerConnection();
	m_eEntryState = ENTRY_STATE::IDLE;
	m_ePendingWorldId = LostArk::Shared::WORLD_ID::END;
	m_ePendingLevel = LEVEL::END;
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

	LostArk::Shared::CHARACTER_CLASS_ID selectedClass =
		DEFAULT_ENTRY_CLASS;
	const bool_t hasExplicitSelection =
		CCharacterSelectionState::Try_Get_SelectedClass(selectedClass);
	ImGui::Text(
		"Entry character: %s%s",
		Get_CharacterClassName(selectedClass),
		hasExplicitSelection ? "" : " (default)");
	const string serverEndpoint = Describe_ServerEndpoint();
	ImGui::TextDisabled("Server: %s", serverEndpoint.c_str());
	ImGui::Separator();

	const bool_t isBusy = ENTRY_STATE::IDLE != m_eEntryState ||
		CLevelTransitionService::Is_Pending();
	ImGui::BeginDisabled(isBusy);
	if (ImGui::Button("Test"))
		CLobbyCommandService::Request(LOBBY_STAGE::TEST);
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

	if (!hasExplicitSelection)
	{
		ImGui::TextDisabled(
			"Direct entry commits Lance Master. Character Select changes it.");
	}
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

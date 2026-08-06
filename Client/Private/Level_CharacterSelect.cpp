#include "imgui.h"

#include "Level_CharacterSelect.h"

#include "AnimationTargetService.h"
#include "Camera_Free.h"
#include "Character.h"
#include "CharacterCatalog.h"
#include "CharacterSelectionState.h"
#include "CombatHUDViewModel.h"
#include "GameInstance.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "LobbyCommandService.h"
#include "NetworkManager.h"
#include "NetworkPlayerCommandSink.h"
#include "NetworkWorldEntityCommandSink.h"
#include "PlayableCharacterAssetService.h"
#include "Transform.h"
#include "ValtanPresentationAssetService.h"

#include <algorithm>

namespace
{
	constexpr f32_t PREVIEW_POSITION_X = -772.017f;
	constexpr f32_t PREVIEW_POSITION_Y = -142.55f;
	constexpr f32_t PREVIEW_POSITION_Z = 197.538f;
	constexpr f32_t PREVIEW_CAMERA_HEIGHT = 1.9f;
	constexpr f32_t PREVIEW_CAMERA_DISTANCE = 3.8f;
	constexpr f32_t PREVIEW_CAMERA_LOOK_HEIGHT = 1.05f;
	constexpr f32_t SERVER_CAMERA_SIDE = 0.4f;
	constexpr f32_t SERVER_CAMERA_HEIGHT = 4.f;
	constexpr f32_t SERVER_CAMERA_DISTANCE = 3.8f;
	constexpr std::chrono::seconds CONNECTION_TIMEOUT{ 5 };
	constexpr std::chrono::seconds VALTAN_REQUEST_TIMEOUT{ 5 };
	constexpr char_t PLAYER_NICKNAME[] = "Player";
	constexpr char_t VALTAN_PLACEMENT_ID[] =
		"boss.valtan.character-select.lazy";

	const char_t* Get_CharacterClassName(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (characterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER: return "Lance Master";
		case CHARACTER_CLASS_ID::GUNSLINGER: return "Gunslinger";
		case CHARACTER_CLASS_ID::SLAYER: return "Slayer";
		case CHARACTER_CLASS_ID::ARTIST: return "Artist";
		case CHARACTER_CLASS_ID::DIMENSIONMASTER: return "Dimension Master";
		case CHARACTER_CLASS_ID::WARLORD: return "Warlord";
		default: return "Unknown";
		}
	}

	const char_t* Get_StageName(const LOBBY_STAGE stage)
	{
		switch (stage)
		{
		case LOBBY_STAGE::TEST: return "Character Select Server Play";
		case LOBBY_STAGE::CHARACTER_SELECT: return "Character Select Preview";
		case LOBBY_STAGE::BERN: return "Bern";
		case LOBBY_STAGE::VALTAN: return "Valtan";
		default: return "Unknown";
		}
	}

	const char_t* Get_StageTransitionSource(const LOBBY_STAGE stage)
	{
		switch (stage)
		{
		case LOBBY_STAGE::TEST: return "character-select.server-play";
		case LOBBY_STAGE::CHARACTER_SELECT: return "character-select.return-preview";
		case LOBBY_STAGE::BERN: return "character-select.enter-bern";
		case LOBBY_STAGE::VALTAN: return "character-select.enter-valtan";
		default: return nullptr;
		}
	}
}

CLevel_CharacterSelect::CLevel_CharacterSelect(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel{ pDevice, pContext }
{
}

CLevel_CharacterSelect::~CLevel_CharacterSelect()
{
	CAnimationTargetService::Unbind(m_pPreviewCharacter);
	CAnimationTargetService::Unbind(m_Replication.Get_LocalCharacter());
	CNetworkManager::Get().Close_ServerConnection();
	m_Replication.Reset();
	CCombatHUDViewModel::Get().Reset_RuntimeState();
	m_MapRuntime.Clear();
}

HRESULT CLevel_CharacterSelect::Initialize()
{
	if (FAILED(__super::Initialize()) ||
		!CCombatHUDViewModel::Get().Initialize_Definitions())
	{
		return E_FAIL;
	}

	const CLIENT_LEVEL_DESCRIPTOR* entry =
		CLevelRegistry::Find(LEVEL::CHARACTER_SELECT);
	if (nullptr == entry || nullptr == entry->pMapAreaId ||
		!m_MapRuntime.Load_Area(
			ETOUI(LEVEL::CHARACTER_SELECT),
			entry->pMapAreaId,
			entry->MapLoadScope))
	{
		OutputDebugStringA((
			"[Level_CharacterSelect] " +
			m_MapRuntime.Get_Status() + "\n").c_str());
		return E_FAIL;
	}
	if (FAILED(Ready_Lights()) || FAILED(Ready_ServerGameplay()))
		return E_FAIL;

	LostArk::Shared::CHARACTER_CLASS_ID initialClass =
		SUPPORTED_CLASSES.front();
	if (CCharacterSelectionState::Try_Get_SelectedClass(initialClass))
	{
		const auto selected = std::find(
			SUPPORTED_CLASSES.begin(),
			SUPPORTED_CLASSES.end(),
			initialClass);
		if (SUPPORTED_CLASSES.end() == selected)
			return E_INVALIDARG;
		m_iPreviewIndex = static_cast<size_t>(
			std::distance(SUPPORTED_CLASSES.begin(), selected));
	}

	if (FAILED(Ready_Preview(initialClass)) || FAILED(Ready_Camera()))
		return E_FAIL;

	CHARACTER_TEST_ENTRY_MODE entryMode = CHARACTER_TEST_ENTRY_MODE::NONE;
	if (CCharacterSelectionState::Try_Consume_TestEntryMode(entryMode) &&
		CHARACTER_TEST_ENTRY_MODE::SERVER_GAMEPLAY == entryMode)
	{
		m_eMode = MODE::CONNECTING;
		m_isValtanSpawnRequested = false;
		m_ConnectionDeadline =
			std::chrono::steady_clock::now() + CONNECTION_TIMEOUT;
		m_strStatus =
			"Lobby-approved Server Arena handoff; waiting for replicated character...";
	}
	return S_OK;
}

void CLevel_CharacterSelect::Update(const f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);
	switch (m_eMode)
	{
	case MODE::CONNECTING:
		Update_Connecting();
		break;
	case MODE::SERVER_ARENA:
		Update_ServerArena();
		break;
	case MODE::RETURNING_TO_LOBBY:
		break;
	case MODE::PREVIEW:
	default:
		break;
	}
}

HRESULT CLevel_CharacterSelect::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

#ifdef _DEBUG
	SetWindowText(
		g_hWnd,
		MODE::SERVER_ARENA == m_eMode ?
		TEXT("LostArk Character Select - Server Arena") :
		TEXT("LostArk Character Select - Preview"));
#endif
	Render_SelectionPanel();
	return S_OK;
}

HRESULT CLevel_CharacterSelect::Ready_Lights()
{
	return CMapPlacementRuntime::Ensure_DefaultLight();
}

HRESULT CLevel_CharacterSelect::Ready_Camera()
{
	if (nullptr == m_pPreviewCharacter ||
		nullptr == m_pPreviewCharacter->Get_Transform())
	{
		return E_FAIL;
	}

	CCamera_Free::CAMERA_FREE_DESC desc{};
	desc.vEye = float3_t(
		PREVIEW_POSITION_X,
		PREVIEW_POSITION_Y + PREVIEW_CAMERA_HEIGHT,
		PREVIEW_POSITION_Z - PREVIEW_CAMERA_DISTANCE);
	desc.vAt = float3_t(
		PREVIEW_POSITION_X,
		PREVIEW_POSITION_Y + PREVIEW_CAMERA_LOOK_HEIGHT,
		PREVIEW_POSITION_Z);
	desc.fFovy = 45.f;
	desc.fNear = 0.1f;
	desc.fFar = 2000.f;
	desc.fSpeedPerSec = 20.f;
	desc.fRotationPerSec = 90.f;
	desc.fMouseSensor = 0.1f;
	desc.pFollowTarget = m_pPreviewCharacter->Get_Transform();
	desc.vPositionOffset = float3_t(
		0.f,
		PREVIEW_CAMERA_HEIGHT,
		-PREVIEW_CAMERA_DISTANCE);
	desc.vLookOffset = float3_t(
		0.f,
		PREVIEW_CAMERA_LOOK_HEIGHT,
		0.f);
	desc.fFollowResponse = 18.f;
	desc.isFollowEnabled = true;
	desc.allowCapturedKeyboardInput = true;

	shared_ptr<CGameObject> gameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::CHARACTER_SELECT),
		TEXT("Prototype_GameObject_Camera_Free"),
		ETOUI(LEVEL::CHARACTER_SELECT),
		TEXT("Layer_Camera"),
		&desc,
		&gameObject)))
	{
		return E_FAIL;
	}
	m_pCamera = dynamic_pointer_cast<CCamera_Free>(gameObject);
	if (nullptr == m_pCamera)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::CHARACTER_SELECT),
			TEXT("Layer_Camera"),
			gameObject);
		return E_FAIL;
	}
	m_pCameraTarget = m_pPreviewCharacter;
	return S_OK;
}

HRESULT CLevel_CharacterSelect::Ready_ServerGameplay()
{
	CClientReplication::DESC desc{};
	desc.pDevice = m_pDevice;
	desc.pContext = m_pContext;
	desc.iPrototypeLevelIndex = ETOUI(LEVEL::CHARACTER_SELECT);
	desc.iLayerLevelIndex = ETOUI(LEVEL::CHARACTER_SELECT);
	desc.strPlayerLayerTag = TEXT("Layer_Player");
	desc.strWorldEntityLayerTag = TEXT("Layer_WorldEntity");
	if (!m_Replication.Initialize(desc))
		return E_FAIL;

	m_pPlayerCommandSink = make_shared<CNetworkPlayerCommandSink>();
	m_pWorldEntityCommandSink =
		make_shared<CNetworkWorldEntityCommandSink>();
	m_PlayerController.Set_CommandSink(m_pPlayerCommandSink);
	m_PlayerController.Set_AllowCapturedKeyboardInput(true);
	return S_OK;
}

bool_t CLevel_CharacterSelect::Bind_CameraTarget(
	const shared_ptr<CCharacter>& character,
	const float3_t& positionOffset)
{
	if (nullptr == m_pCamera || nullptr == character ||
		nullptr == character->Get_Transform())
	{
		return false;
	}
	if (m_pCameraTarget.lock() == character)
		return true;

	const bool_t wasFollowEnabled = m_pCamera->Is_FollowRequested();
	m_pCamera->Set_FollowTarget(character->Get_Transform());
	m_pCamera->Set_PositionOffset(positionOffset);
	m_pCamera->Set_FollowEnabled(wasFollowEnabled);
	m_pCameraTarget = character;
	return true;
}

HRESULT CLevel_CharacterSelect::Ready_Preview(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	shared_ptr<CCharacter> stagedCharacter;
	if (FAILED(Stage_Preview(characterClass, stagedCharacter)))
		return E_FAIL;
	if (FAILED(Commit_Preview(characterClass, stagedCharacter)))
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::CHARACTER_SELECT),
			TEXT("Layer_PreviewCharacter"),
			stagedCharacter);
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CLevel_CharacterSelect::Stage_Preview(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	shared_ptr<CCharacter>& outCharacter)
{
	outCharacter.reset();
	if (!LostArk::Shared::Is_Supported_Playable_Character_Class(characterClass))
		return E_INVALIDARG;
	const CHARACTER_SPEC* spec = CCharacterCatalog::Find_Spec(characterClass);
	if (nullptr == spec)
		return E_FAIL;

	CCharacter::CHARACTER_DESC desc{};
	desc.iPrototypeLevelIndex = ETOUI(LEVEL::CHARACTER_SELECT);
	desc.pSpec = spec;
	desc.pNavigationPrototypeTag = nullptr;
	desc.fSpeedPerSec = 0.f;
	desc.fRotationPerSec = 90.f;
	desc.vPosition = float3_t(
		PREVIEW_POSITION_X,
		PREVIEW_POSITION_Y,
		PREVIEW_POSITION_Z);
	desc.strNickName = Get_CharacterClassName(characterClass);
	desc.isLocallyControlled = false;

	shared_ptr<CGameObject> stagedObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::CHARACTER_SELECT),
		TEXT("Prototype_GameObject_Character"),
		ETOUI(LEVEL::CHARACTER_SELECT),
		TEXT("Layer_PreviewCharacter"),
		&desc,
		&stagedObject)))
	{
		return E_FAIL;
	}
	const shared_ptr<CCharacter> stagedCharacter =
		dynamic_pointer_cast<CCharacter>(stagedObject);
	if (nullptr == stagedCharacter ||
		nullptr == stagedCharacter->Get_Transform() ||
		!stagedCharacter->Set_Animation(CHARACTER_ANIM::IDLE, true))
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::CHARACTER_SELECT),
			TEXT("Layer_PreviewCharacter"),
			stagedObject);
		return E_FAIL;
	}
	stagedCharacter->Get_Transform()->Rotation(0.f, 180.f, 0.f);
	outCharacter = stagedCharacter;
	return S_OK;
}

HRESULT CLevel_CharacterSelect::Commit_Preview(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	const shared_ptr<CCharacter>& stagedCharacter)
{
	if (nullptr == stagedCharacter ||
		!CCombatHUDViewModel::Get().Apply_CharacterPreview(characterClass))
	{
		return E_FAIL;
	}
	const shared_ptr<CCharacter> previous = m_pPreviewCharacter;
	m_pPreviewCharacter = stagedCharacter;
	if (nullptr != previous)
		CAnimationTargetService::Unbind(previous);
	CAnimationTargetService::Bind(m_pPreviewCharacter);
	if (nullptr != m_pCamera)
	{
		Bind_CameraTarget(
			m_pPreviewCharacter,
			float3_t(
				0.f,
				PREVIEW_CAMERA_HEIGHT,
				-PREVIEW_CAMERA_DISTANCE));
	}
	if (nullptr != previous)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::CHARACTER_SELECT),
			TEXT("Layer_PreviewCharacter"),
			previous);
	}
	return S_OK;
}

bool_t CLevel_CharacterSelect::Select_Preview(const size_t index)
{
	if (MODE::PREVIEW != m_eMode || index >= SUPPORTED_CLASSES.size())
		return false;
	if (index == m_iPreviewIndex && nullptr != m_pPreviewCharacter)
		return true;

	const auto characterClass = SUPPORTED_CLASSES[index];
	if (!CPlayableCharacterAssetService::Is_Ready(
		ETOUI(LEVEL::CHARACTER_SELECT), characterClass) &&
		FAILED(CPlayableCharacterAssetService::Ensure_Prototypes(
			m_pDevice,
			m_pContext,
			ETOUI(LEVEL::CHARACTER_SELECT),
			characterClass)))
	{
		m_strStatus =
			"The selected class assets failed to load. Previous preview kept.";
		return false;
	}
	if (FAILED(Ready_Preview(characterClass)))
	{
		m_strStatus =
			"The selected preview failed to stage. Previous preview kept.";
		return false;
	}
	m_iPreviewIndex = index;
	m_strStatus = std::string("Previewing ") +
		Get_CharacterClassName(characterClass) + ".";
	return true;
}

void CLevel_CharacterSelect::Update_Connecting()
{
	CNetworkManager& network = CNetworkManager::Get();
	if (!network.Is_Connected())
	{
		Fail_ServerArena("Server disconnected before arena admission.");
		return;
	}
	if (!m_Replication.Update())
	{
		Fail_ServerArena("Arena replication failed while staging the character.");
		return;
	}
	if (nullptr != m_Replication.Get_LocalCharacter())
	{
		if (!Commit_ServerArena())
			Fail_ServerArena("Replicated character could not replace the preview.");
		return;
	}
	if (std::chrono::steady_clock::now() >= m_ConnectionDeadline)
		Fail_ServerArena("Server arena admission timed out after 5 seconds.");
}

bool_t CLevel_CharacterSelect::Commit_ServerArena()
{
	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	if (nullptr == localCharacter ||
		!Bind_CameraTarget(
			localCharacter,
			float3_t(
				SERVER_CAMERA_SIDE,
				SERVER_CAMERA_HEIGHT,
				SERVER_CAMERA_DISTANCE)))
	{
		return false;
	}

	const shared_ptr<CCharacter> preview = m_pPreviewCharacter;
	CAnimationTargetService::Unbind(preview);
	CAnimationTargetService::Bind(localCharacter);
	m_PlayerController.Set_LocalCharacter(localCharacter);
	if (nullptr != preview &&
		FAILED(CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::CHARACTER_SELECT),
			TEXT("Layer_PreviewCharacter"),
			preview)))
	{
		return false;
	}
	m_pPreviewCharacter.reset();

	m_eMode = MODE::SERVER_ARENA;
	m_strStatus =
		"Server Arena active. Skill keys and F6 remain available with F1 tools.";
	return true;
}

void CLevel_CharacterSelect::Update_ServerArena()
{
	LostArk::Shared::S2C_WORLD_ENTITY_SPAWN_RESULT spawnResult{};
	while (CNetworkManager::Get().Try_Consume_WorldEntitySpawnResult(
		spawnResult))
	{
		if (spawnResult.strPlacementId != VALTAN_PLACEMENT_ID)
			continue;
		if (LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT::REJECTED ==
			spawnResult.eResult)
		{
			m_isValtanSpawnRequested = false;
			m_strStatus = "Server rejected the Valtan spawn request.";
		}
		else
		{
			m_strStatus =
				"Server accepted the Valtan placement; awaiting presentation.";
		}
	}
	if (!m_Replication.Update())
	{
		Fail_ServerArena("Server presentation failed.");
		return;
	}
	if (m_Replication.Has_PendingConnectionLoss() ||
		!CNetworkManager::Get().Is_Connected())
	{
		Fail_ServerArena("Server disconnected.");
		return;
	}

	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	m_PlayerController.Set_LocalCharacter(localCharacter);
	m_PlayerController.Update(
		nullptr != m_pCamera && m_pCamera->Is_FollowEnabled());
	if (m_Replication.Has_WorldEntity("BOSS_VALTAN"))
	{
		if (m_isValtanSpawnRequested)
			m_strStatus = "Valtan spawned from the Server world template.";
		m_isValtanSpawnRequested = false;
	}
	else if (m_isValtanSpawnRequested &&
		std::chrono::steady_clock::now() >= m_ValtanRequestDeadline)
	{
		m_isValtanSpawnRequested = false;
		m_strStatus = "Valtan spawn was not observed; retry is available.";
	}
}

void CLevel_CharacterSelect::Fail_ServerArena(const string& reason)
{
	CAnimationTargetService::Unbind(m_Replication.Get_LocalCharacter());
	CNetworkManager::Get().Close_ServerConnection();
	m_Replication.Reset();
	m_PlayerController.Set_LocalCharacter(nullptr);
	m_eMode = MODE::RETURNING_TO_LOBBY;
	m_isValtanSpawnRequested = false;
	m_strStatus = reason + " Returning to Lobby; local gameplay fallback is disabled.";
	if (!CLevelTransitionService::Request_Load(
		LEVEL::LOBBY,
		"character-select.server-disconnect"))
	{
		m_strStatus += " " + CLevelTransitionService::Get_Status();
	}
}

bool_t CLevel_CharacterSelect::Request_ValtanSpawn()
{
	if (MODE::SERVER_ARENA != m_eMode || m_isValtanSpawnRequested ||
		m_Replication.Has_WorldEntity("BOSS_VALTAN"))
	{
		return false;
	}
	m_strStatus = "Preparing Valtan presentation assets...";
	if (FAILED(CValtanPresentationAssetService::Ensure_Prototypes(
		m_pDevice,
		m_pContext,
		ETOUI(LEVEL::CHARACTER_SELECT))))
	{
		m_strStatus =
			"Valtan assets failed to prepare; no Server spawn was requested.";
		return false;
	}
	if (nullptr == m_pWorldEntityCommandSink ||
		!m_pWorldEntityCommandSink->Request_SpawnWorldEntity(
			VALTAN_PLACEMENT_ID))
	{
		m_strStatus = "Valtan spawn request could not be sent.";
		return false;
	}
	m_isValtanSpawnRequested = true;
	m_ValtanRequestDeadline =
		std::chrono::steady_clock::now() + VALTAN_REQUEST_TIMEOUT;
	m_strStatus = "Valtan spawn requested from Server.";
	return true;
}

bool_t CLevel_CharacterSelect::Enter_Stage(const LOBBY_STAGE stage)
{
	const char_t* stageName = Get_StageName(stage);
	const char_t* transitionSource = Get_StageTransitionSource(stage);
	if (nullptr == transitionSource ||
		m_iPreviewIndex >= SUPPORTED_CLASSES.size())
	{
		m_strStatus = "The selected stage is not supported here.";
		return false;
	}
	if (!CCharacterSelectionState::Select(SUPPORTED_CLASSES[m_iPreviewIndex]))
		return false;
	if (MODE::PREVIEW != m_eMode)
	{
		CAnimationTargetService::Unbind(m_Replication.Get_LocalCharacter());
		CNetworkManager::Get().Close_ServerConnection();
		m_Replication.Reset();
		m_PlayerController.Set_LocalCharacter(nullptr);
		m_eMode = MODE::RETURNING_TO_LOBBY;
	}

	LOBBY_COMMAND_TOKEN token = INVALID_LOBBY_COMMAND_TOKEN;
	if (!CLobbyCommandService::Request(stage, token))
	{
		m_strStatus = CLobbyCommandService::Get_Status();
		return false;
	}
	if (!CLevelTransitionService::Request_Load(
		LEVEL::LOBBY,
		transitionSource,
		token))
	{
		CLobbyCommandService::Cancel(
			token,
			"Lobby load request was rejected");
		m_strStatus = CLevelTransitionService::Get_Status();
		return false;
	}
	m_strStatus = std::string("Lobby will request ") +
		stageName + " from Server.";
	return true;
}

void CLevel_CharacterSelect::Render_SelectionPanel()
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
		"Character Select",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::End();
		return;
	}

	ImGui::TextUnformatted("Mode");
	const bool_t isPreview = MODE::PREVIEW == m_eMode;
	const bool_t isConnecting = MODE::CONNECTING == m_eMode;
	const bool_t isServerArena = MODE::SERVER_ARENA == m_eMode;
	const bool_t isReturning = MODE::RETURNING_TO_LOBBY == m_eMode;
	const bool_t transitionPending = CLevelTransitionService::Is_Pending();
	const bool_t isModeTransitioning =
		isConnecting || isReturning || transitionPending;
	const bool_t isServerSelected =
		isConnecting || isServerArena || isReturning;

	ImGui::BeginDisabled(isModeTransitioning);
	if (ImGui::RadioButton("Preview", isPreview) && !isPreview)
		Enter_Stage(LOBBY_STAGE::CHARACTER_SELECT);
	ImGui::SameLine();
	if (ImGui::RadioButton(
		"Server Play (Lobby-approved)",
		isServerSelected) && isPreview)
	{
		Enter_Stage(LOBBY_STAGE::TEST);
	}
	ImGui::EndDisabled();
	if (isConnecting)
		ImGui::TextDisabled("Connecting... existing preview is preserved");
	else if (isReturning)
		ImGui::TextDisabled("Returning to socket-free Preview...");

	ImGui::Separator();
	ImGui::TextUnformatted("Playable class");
	ImGui::BeginDisabled(!isPreview || transitionPending);
	for (size_t index = 0; index < SUPPORTED_CLASSES.size(); ++index)
	{
		if (ImGui::Selectable(
			Get_CharacterClassName(SUPPORTED_CLASSES[index]),
			index == m_iPreviewIndex))
		{
			Select_Preview(index);
		}
	}
	ImGui::EndDisabled();
	if (!isPreview)
		ImGui::TextDisabled("Return to Preview to change the Server session class.");

	if (isServerArena)
	{
		ImGui::Separator();
		const bool_t valtanSpawned =
			m_Replication.Has_WorldEntity("BOSS_VALTAN");
		ImGui::BeginDisabled(
			m_isValtanSpawnRequested || valtanSpawned);
		if (ImGui::Button("Summon Valtan (Lazy)"))
			Request_ValtanSpawn();
		ImGui::EndDisabled();
		if (valtanSpawned)
			ImGui::SameLine(), ImGui::TextDisabled("Spawned");
		else if (m_isValtanSpawnRequested)
			ImGui::SameLine(), ImGui::TextDisabled("Requested");
	}

	ImGui::Separator();
	ImGui::BeginDisabled(isConnecting || isReturning || transitionPending);
	if (ImGui::Button("Enter Bern"))
		Enter_Stage(LOBBY_STAGE::BERN);
	ImGui::SameLine();
	if (ImGui::Button("Enter Valtan Map"))
		Enter_Stage(LOBBY_STAGE::VALTAN);
	ImGui::SameLine();
	if (ImGui::Button("Back"))
	{
		if (MODE::PREVIEW != m_eMode)
		{
			Fail_ServerArena("Leaving Server Arena.");
		}
		else if (!CLevelTransitionService::Request_Load(
			LEVEL::LOBBY,
			"character-select.back"))
		{
			m_strStatus = CLevelTransitionService::Get_Status();
		}
	}
	ImGui::EndDisabled();

	ImGui::TextDisabled(
		"F1: tools  |  F6: follow/free  |  Server Play: skill input enabled");
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::End();
}

unique_ptr<CLevel_CharacterSelect> CLevel_CharacterSelect::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto instance = unique_ptr<CLevel_CharacterSelect>(
		new CLevel_CharacterSelect(pDevice, pContext));
	if (FAILED(instance->Initialize()))
		return nullptr;
	return instance;
}

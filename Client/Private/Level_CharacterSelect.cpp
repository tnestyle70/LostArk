#include "imgui.h"

#include "Level_CharacterSelect.h"

#include "AnimationEffectCueDocument.h"
#include "AnimationTargetService.h"
#include "Camera_Free.h"
#include "Character.h"
#include "CharacterCatalog.h"
#include "CharacterSelectArenaSpawnGate.h"
#include "CharacterSelectionState.h"
#include "CharacterSpec.h"
#include "CombatHUDViewModel.h"
#include "Effect_PresentationService.h"
#include "GameInstance.h"
#include "HUDRuntimeView.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "LobbyCommandService.h"
#include "MainApp.h"
#include "Network/PacketMessages.h"
#include "NetworkManager.h"
#include "NetworkPlayerCommandSink.h"
#include "NetworkWorldEntityCommandSink.h"
#include "PlayableCharacterAssetService.h"
#include "Transform.h"
#include "ValtanPatternEffectCueDocument.h"
#include "ValtanPresentationAssetService.h"

#include <algorithm>
#include <cstring>

namespace
{
	constexpr f32_t ARENA_INITIAL_TARGET_X = -772.017f;
	constexpr f32_t ARENA_INITIAL_TARGET_Y = -142.55f;
	constexpr f32_t ARENA_INITIAL_TARGET_Z = 197.538f;
	constexpr f32_t CHARACTER_SELECT_CAMERA_SIDE = 0.4f;
	constexpr f32_t CHARACTER_SELECT_CAMERA_HEIGHT = 7.5f;
	constexpr f32_t CHARACTER_SELECT_CAMERA_DISTANCE = 4.5f;
	constexpr f32_t CHARACTER_SELECT_CAMERA_LOOK_HEIGHT = 1.05f;
	constexpr f32_t CHARACTER_SELECT_CAMERA_FOV_Y = 45.f;
	constexpr std::chrono::seconds CONNECTION_TIMEOUT{ 5 };
	constexpr std::chrono::seconds CLASS_CHANGE_TIMEOUT{ 5 };
	constexpr std::chrono::seconds ARENA_SPAWN_REQUEST_TIMEOUT{ 5 };

	struct ARENA_SPAWN_OPTION final
	{
		const char_t* pLabel = nullptr;
		const char_t* pStableId = nullptr;
		const char_t* pArchetypeId = nullptr;
		bool_t requiresValtanPrewarm = false;
	};

	constexpr std::array<ARENA_SPAWN_OPTION, 3> ARENA_SPAWN_OPTIONS =
	{
		ARENA_SPAWN_OPTION{
			"Monster", "spawn.character-select.monster",
			"MONSTER_VALTAN_PADD_01", false },
		ARENA_SPAWN_OPTION{
			"Mid Boss (Lugaru)", "spawn.character-select.miniboss",
			"MINIBOSS_LUGARU", false },
		ARENA_SPAWN_OPTION{
			"Valtan", "boss.valtan.character-select.lazy",
			"BOSS_VALTAN", true }
	};

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

	float3_t CharacterSelectCameraPositionOffset()
	{
		return float3_t(
			CHARACTER_SELECT_CAMERA_SIDE,
			CHARACTER_SELECT_CAMERA_HEIGHT,
			CHARACTER_SELECT_CAMERA_DISTANCE);
	}

	float3_t CharacterSelectCameraLookOffset()
	{
		return float3_t(0.f, CHARACTER_SELECT_CAMERA_LOOK_HEIGHT, 0.f);
	}

	const char_t* Get_StageName(const LOBBY_STAGE stage)
	{
		switch (stage)
		{
		case LOBBY_STAGE::TEST: return "Character Select";
		case LOBBY_STAGE::CHARACTER_SELECT: return "Character Select";
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
		case LOBBY_STAGE::CHARACTER_SELECT: return "character-select.server-entry";
		case LOBBY_STAGE::BERN: return "character-select.enter-bern";
		case LOBBY_STAGE::VALTAN: return "character-select.enter-valtan";
		default: return nullptr;
		}
	}
}

CLevel_CharacterSelect::CLevel_CharacterSelect(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel{ pDevice, pContext },
	m_pArenaSpawnGate{ std::make_unique<CCharacterSelectArenaSpawnGate>() }
{
}

CLevel_CharacterSelect::~CLevel_CharacterSelect()
{
	CAnimationTargetService::Unbind(m_pActiveCharacter);
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
		m_iSelectedClassIndex = static_cast<size_t>(
			std::distance(SUPPORTED_CLASSES.begin(), selected));
	}

	if (FAILED(Ready_Camera()))
		return E_FAIL;

	m_pClassSelectView = std::make_unique<CHUDRuntimeView>(
		m_pDevice, m_pContext,
		L"UI/ClassSelect/ClassSelect_Layout.json",
		CHUDRuntimeView::DRAW_TARGET::FOREGROUND);

	m_eMode = MODE::CONNECTING;
	Reset_ArenaSpawnRequest();
	m_ArenaSpawnAccepted.fill(false);
	m_ConnectionDeadline =
		std::chrono::steady_clock::now() + CONNECTION_TIMEOUT;
	m_strStatus =
		"Lobby-approved Server Arena; waiting for replicated character...";
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
	default:
		break;
	}
}

HRESULT CLevel_CharacterSelect::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

#ifdef _DEBUG
	CMainApp::Update_DebugWindowTitleWithFps(
		TEXT("LostArk Character Select - Server Arena"));
#endif
	Render_SelectionPanel();
	Render_ClassList();
	Render_ArenaSpawnButtons();
	return S_OK;
}

HRESULT CLevel_CharacterSelect::Ready_Lights()
{
	return S_OK;
}

HRESULT CLevel_CharacterSelect::Ready_Camera()
{
	CCamera_Free::CAMERA_FREE_DESC desc{};
	const float3_t positionOffset = CharacterSelectCameraPositionOffset();
	const float3_t lookOffset = CharacterSelectCameraLookOffset();
	desc.vEye = float3_t(
		ARENA_INITIAL_TARGET_X + positionOffset.x,
		ARENA_INITIAL_TARGET_Y + positionOffset.y,
		ARENA_INITIAL_TARGET_Z + positionOffset.z);
	desc.vAt = float3_t(
		ARENA_INITIAL_TARGET_X + lookOffset.x,
		ARENA_INITIAL_TARGET_Y + lookOffset.y,
		ARENA_INITIAL_TARGET_Z + lookOffset.z);
	desc.fFovy = CHARACTER_SELECT_CAMERA_FOV_Y;
	desc.fNear = 0.1f;
	desc.fFar = 2000.f;
	desc.fSpeedPerSec = 20.f;
	desc.fRotationPerSec = 90.f;
	desc.fMouseSensor = 0.1f;
	desc.pFollowTarget = nullptr;
	desc.vPositionOffset = positionOffset;
	desc.vLookOffset = lookOffset;
	desc.fFollowResponse = 18.f;
	desc.isFollowEnabled = false;
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
	m_pCameraTarget.reset();
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
	desc.bDeferLocalCharacterClassReplacement = true;
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

	m_pCamera->Set_FollowTarget(character->Get_Transform());
	m_pCamera->Set_PositionOffset(positionOffset);
	m_pCamera->Set_FollowEnabled(true);
	m_pCameraTarget = character;
	return true;
}

bool_t CLevel_CharacterSelect::Request_ClassChange(const size_t index)
{
	if (m_isCreateCharacterModalOpen ||
		MODE::SERVER_ARENA != m_eMode || m_iPendingClassIndex.has_value() ||
		Is_ClassPresentationPreparationPending() ||
		index >= SUPPORTED_CLASSES.size() || nullptr == m_pPlayerCommandSink)
	{
		return false;
	}
	if (index == m_iSelectedClassIndex)
	{
		m_strStatus = "The selected class is already active.";
		return true;
	}
	const auto characterClass = SUPPORTED_CLASSES[index];
	if (FAILED(CPlayableCharacterAssetService::Ensure_Prototypes(
		m_pDevice,
		m_pContext,
		ETOUI(LEVEL::CHARACTER_SELECT),
		characterClass)))
	{
		m_strStatus =
			"The selected class assets failed to load. The active character was kept.";
		return false;
	}

	const std::uint32_t sequence = m_iNextClassChangeSequence++;
	if (0u == m_iNextClassChangeSequence)
		m_iNextClassChangeSequence = 1u;
	if (!m_pPlayerCommandSink->Request_ChangeCharacterClass(
		sequence, characterClass))
	{
		m_strStatus = "The class change request could not be sent.";
		return false;
	}
	m_iPendingClassIndex = index;
	m_iPendingClassChangeSequence = sequence;
	m_ClassChangeDeadline =
		std::chrono::steady_clock::now() + CLASS_CHANGE_TIMEOUT;
	m_strStatus = std::string("Server class change requested: ") +
		Get_CharacterClassName(characterClass) + ".";
	return true;
}

void CLevel_CharacterSelect::Consume_ClassChangeResults()
{
	using namespace LostArk::Shared;
	S2C_CHARACTER_CLASS_CHANGE_RESULT result{};
	while (CNetworkManager::Get().Try_Consume_CharacterClassChangeResult(result))
	{
		if (!m_iPendingClassIndex.has_value() ||
			result.iClientSequence != m_iPendingClassChangeSequence ||
			result.eRequestedClass != SUPPORTED_CLASSES[*m_iPendingClassIndex])
		{
			continue;
		}
		if (CHARACTER_CLASS_CHANGE_RESULT::ACCEPTED == result.eResult)
		{
			m_strStatus = "Server approved the class change; waiting for snapshot.";
			continue;
		}
		m_iPendingClassIndex.reset();
		m_iPendingClassChangeSequence = 0u;
		switch (result.eResult)
		{
		case CHARACTER_CLASS_CHANGE_RESULT::REJECTED_SAME_CLASS:
			m_strStatus = "Server reports that class is already active.";
			break;
		case CHARACTER_CLASS_CHANGE_RESULT::REJECTED_STALE_SEQUENCE:
			m_strStatus = "Server rejected a stale class change request.";
			break;
		case CHARACTER_CLASS_CHANGE_RESULT::REJECTED_UNSUPPORTED_CLASS:
			m_strStatus = "Server rejected an unsupported class.";
			break;
		case CHARACTER_CLASS_CHANGE_RESULT::REJECTED_WRONG_WORLD:
			m_strStatus = "Class changes are unavailable in this Server world.";
			break;
		default:
			m_strStatus = "Server rejected the class change; the active character was kept.";
			break;
		}
	}
}

bool_t CLevel_CharacterSelect::Advance_DeferredClassPresentation()
{
	using namespace LostArk::Shared;
	DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_VIEW Pending;
	if (!m_Replication.Try_Get_DeferredLocalCharacterClassReplacement(Pending))
	{
		Reset_ClassPresentationPreparation();
		return true;
	}
	if (0u == Pending.iGeneration || 0u == Pending.iServerTick ||
		INVALID_NET_ENTITY_ID == Pending.iNetEntityId ||
		!Is_Supported_Playable_Character_Class(Pending.eCharacterClass))
	{
		m_strStatus =
			"Deferred class presentation snapshot identity is invalid.";
		return false;
	}

	const bool_t bNewGeneration =
		CLASS_PRESENTATION_PREPARATION_STATE::IDLE ==
			m_eClassPresentationPreparationState ||
		m_iClassPresentationPreparationGeneration != Pending.iGeneration ||
		m_iClassPresentationNetEntityId != Pending.iNetEntityId ||
		m_eClassPresentationTargetClass != Pending.eCharacterClass;
	if (bNewGeneration)
	{
		Reset_ClassPresentationPreparation();
		m_strClassPresentationCommitWarning.clear();
		m_iClassPresentationPreparationGeneration = Pending.iGeneration;
		m_iClassPresentationNetEntityId = Pending.iNetEntityId;
		m_eClassPresentationTargetClass = Pending.eCharacterClass;
		m_eClassPresentationPreparationState =
			CLASS_PRESENTATION_PREPARATION_STATE::WAITING_FOR_PRODUCT_EFFECTS;

		const auto IsolateRegistrationFailure =
			[this](const std::string& Status)
		{
			m_eClassPresentationPreparationState =
				CLASS_PRESENTATION_PREPARATION_STATE::
					REGISTRATION_FAILURE_ISOLATED;
			m_ClassPresentationEffectTargets.clear();
			m_strClassPresentationPreparationFailure = Status;
			OutputDebugStringA((
				"[Level_CharacterSelect] Class Effect preparation isolated: " +
				Status + "\n").c_str());
		};

		const CHARACTER_SPEC* pSpec =
			CCharacterCatalog::Find_Spec(Pending.eCharacterClass);
		if (nullptr == pSpec || nullptr == pSpec->pAssetName)
		{
			IsolateRegistrationFailure(
				"Server class has no animation asset spec.");
		}
		else
		{
			ANIMATION_EFFECT_CUE_DOCUMENT CueDocument;
			std::string Status;
			if (!CAnimationEffectCueDocument::Load_ForProductPrewarm(
					pSpec->pAssetName, CueDocument, Status) ||
				!CEffectPresentationService::Queue_ProductCues_Priority(
					CueDocument.Cues,
					m_ClassPresentationEffectTargets,
					Status))
			{
				IsolateRegistrationFailure(Status);
			}
		}
	}

	const EFFECT_PRODUCT_PREWARM_TARGET_PROBE Probe =
		CEffectPresentationService::Get_ProductCuePreparationProbe(
			m_ClassPresentationEffectTargets);
	const bool_t bRegistrationFailureIsolated =
		CLASS_PRESENTATION_PREPARATION_STATE::
			REGISTRATION_FAILURE_ISOLATED ==
		m_eClassPresentationPreparationState;
	if (!Is_ProductPrewarmActivationReady(
			Probe, bRegistrationFailureIsolated))
	{
		if (bRegistrationFailureIsolated)
		{
			m_strStatus = std::string(
				"Class Effect preparation was isolated; draining ") +
				std::to_string(Probe.iQueuePendingCount) +
				" queued Product Effect(s) before presentation commit.";
		}
		else
		{
			m_strStatus = std::string("Preparing ") +
				Get_CharacterClassName(Pending.eCharacterClass) +
				" Product Effects " +
				std::to_string(Probe.iPreparedCount + Probe.iFailedCount +
					Probe.iUnavailableCount) + "/" +
				std::to_string(Probe.iTargetCount) +
				" (selected pending " +
				std::to_string(Probe.iPendingCount) +
				", queue pending " +
				std::to_string(Probe.iQueuePendingCount) + ").";
		}
		return true;
	}

	/* Replacement is attempted once per stable class/entity generation.  Repeated
	   snapshots update the staged state, but cannot create a 20 Hz clone-failure
	   loop; only a different authoritative target generation may retry. */
	if (m_hasClassPresentationCommitAttempted)
		return true;
	m_hasClassPresentationCommitAttempted = true;
	const DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT CommitResult =
		m_Replication.Commit_DeferredLocalCharacterClassReplacement();
	switch (CommitResult)
	{
	case DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT::NO_PENDING:
		Reset_ClassPresentationPreparation();
		return true;

	case DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT::COMMITTED:
		if (bRegistrationFailureIsolated)
		{
			m_strClassPresentationCommitWarning =
				"Effect preparation was isolated: " +
				m_strClassPresentationPreparationFailure;
		}
		else if (0u != Probe.iFailedCount + Probe.iUnavailableCount)
		{
			m_strClassPresentationCommitWarning =
				std::to_string(
					Probe.iFailedCount + Probe.iUnavailableCount) +
				" Product Effect target(s) were isolated.";
		}
		m_strStatus = std::string("Prepared and committed ") +
			Get_CharacterClassName(Pending.eCharacterClass) +
			" from Server snapshot generation " +
			std::to_string(Pending.iGeneration) + ".";
		Reset_ClassPresentationPreparation();
		return true;

	case DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT::RECOVERED_FAILURE:
		m_strStatus =
			"Class presentation commit failed; returning to Lobby instead of leaving Character Select input blocked.";
		return false;

	case DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT::FATAL_FAILURE:
	default:
		m_strStatus = "Deferred class presentation commit failed fatally.";
		return false;
	}
}

bool_t CLevel_CharacterSelect::Is_ClassPresentationPreparationPending() const
{
	if (CLASS_PRESENTATION_PREPARATION_STATE::IDLE !=
		m_eClassPresentationPreparationState)
	{
		return true;
	}
	DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_VIEW Pending;
	return m_Replication.Try_Get_DeferredLocalCharacterClassReplacement(Pending);
}

void CLevel_CharacterSelect::Reset_ClassPresentationPreparation()
{
	m_eClassPresentationPreparationState =
		CLASS_PRESENTATION_PREPARATION_STATE::IDLE;
	m_iClassPresentationPreparationGeneration = 0u;
	m_iClassPresentationNetEntityId =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	m_eClassPresentationTargetClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	m_hasClassPresentationCommitAttempted = false;
	m_ClassPresentationEffectTargets.clear();
	m_strClassPresentationPreparationFailure.clear();
}

bool_t CLevel_CharacterSelect::Synchronize_LocalCharacter()
{
	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	if (nullptr == localCharacter || nullptr == localCharacter->Get_Spec())
		return false;
	const auto selected = std::find(
		SUPPORTED_CLASSES.begin(), SUPPORTED_CLASSES.end(),
		localCharacter->Get_Spec()->eCharacterClass);
	if (SUPPORTED_CLASSES.end() == selected)
		return false;
	const size_t selectedIndex = static_cast<size_t>(
		std::distance(SUPPORTED_CLASSES.begin(), selected));

	const bool_t bPresentationChanged = m_pActiveCharacter != localCharacter;
	if (bPresentationChanged)
	{
		CAnimationTargetService::Unbind(m_pActiveCharacter);
		CAnimationTargetService::Bind(localCharacter);
		if (!Bind_CameraTarget(
			localCharacter, CharacterSelectCameraPositionOffset()))
		{
			return false;
		}
		m_PlayerController.Rebind_LocalCharacter(localCharacter);
		m_pActiveCharacter = localCharacter;
	}
	m_iSelectedClassIndex = selectedIndex;
	if (!CCharacterSelectionState::Select(
		localCharacter->Get_Spec()->eCharacterClass))
	{
		return false;
	}
	if (m_iPendingClassIndex.has_value() &&
		*m_iPendingClassIndex == selectedIndex)
	{
		m_iPendingClassIndex.reset();
		m_iPendingClassChangeSequence = 0u;
		m_strStatus = std::string("Class changed to ") +
			Get_CharacterClassName(localCharacter->Get_Spec()->eCharacterClass) +
			". Skills now resolve from the new class.";
		if (!m_strClassPresentationCommitWarning.empty())
		{
			m_strStatus += " " + m_strClassPresentationCommitWarning;
			m_strClassPresentationCommitWarning.clear();
		}
	}
	else if (bPresentationChanged &&
		!m_strClassPresentationCommitWarning.empty())
	{
		m_strStatus += " " + m_strClassPresentationCommitWarning;
		m_strClassPresentationCommitWarning.clear();
	}
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
			Fail_ServerArena("Replicated character could not bind to Server Arena.");
		return;
	}
	if (std::chrono::steady_clock::now() >= m_ConnectionDeadline)
		Fail_ServerArena("Server arena admission timed out after 5 seconds.");
}

bool_t CLevel_CharacterSelect::Commit_ServerArena()
{
	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	if (nullptr == localCharacter)
	{
		return false;
	}
	m_PlayerController.Set_LocalCharacter(localCharacter);
	m_pActiveCharacter.reset();
	if (!Synchronize_LocalCharacter())
		return false;

	m_eMode = MODE::SERVER_ARENA;
	m_strStatus =
		"Server Arena active. Select a class thumbnail, then test its skill keys.";
	return true;
}

void CLevel_CharacterSelect::Update_ServerArena()
{
	Consume_ClassChangeResults();
	LostArk::Shared::S2C_WORLD_ENTITY_SPAWN_RESULT spawnResult{};
	while (CNetworkManager::Get().Try_Consume_WorldEntitySpawnResult(
		spawnResult))
	{
		const auto option = std::find_if(
			ARENA_SPAWN_OPTIONS.begin(),
			ARENA_SPAWN_OPTIONS.end(),
			[&spawnResult](const ARENA_SPAWN_OPTION& candidate)
			{
				return spawnResult.strPlacementId == candidate.pStableId;
			});
		if (ARENA_SPAWN_OPTIONS.end() == option)
			continue;
		const size_t optionIndex = static_cast<size_t>(
			std::distance(ARENA_SPAWN_OPTIONS.begin(), option));
		if (LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT::REJECTED ==
			spawnResult.eResult)
		{
			m_ArenaSpawnAccepted[optionIndex] = false;
			m_strStatus = std::string{ "Server rejected " } +
				option->pLabel + " spawn; retry is available.";
		}
		else
		{
			m_ArenaSpawnAccepted[optionIndex] = true;
			m_strStatus = std::string{ "Server accepted " } +
				option->pLabel + " spawn.";
		}
		if (m_iPendingArenaSpawnIndex == optionIndex)
		{
			if (LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT::REJECTED ==
				spawnResult.eResult)
			{
				m_pArenaSpawnGate->Mark_RequestFailed();
				m_iPendingArenaSpawnIndex.reset();
				m_iArenaSpawnIntentIndex.reset();
				m_ValtanEffectPreparationTargets.clear();
			}
			else
			{
				Reset_ArenaSpawnRequest();
			}
		}
	}
	if (!m_Replication.Update())
	{
		Fail_ServerArena("Server presentation failed.");
		return;
	}
	if (!Advance_DeferredClassPresentation())
	{
		Fail_ServerArena("Deferred Server class presentation failed.");
		return;
	}
	string presentationFailure;
	if (m_Replication.Try_Consume_PresentationFailure(presentationFailure))
		m_strStatus = std::move(presentationFailure);
	if (m_Replication.Has_PendingConnectionLoss() ||
		!CNetworkManager::Get().Is_Connected())
	{
		Fail_ServerArena("Server disconnected.");
		return;
	}

	if (!Synchronize_LocalCharacter())
	{
		Fail_ServerArena("The replicated local character is unavailable.");
		return;
	}
	if (!m_isCreateCharacterModalOpen &&
		!Is_ClassPresentationPreparationPending())
	{
		m_PlayerController.Update(
			nullptr != m_pCamera && m_pCamera->Is_FollowEnabled());
	}
	if (m_iPendingClassIndex.has_value() &&
		!Is_ClassPresentationPreparationPending() &&
		std::chrono::steady_clock::now() >= m_ClassChangeDeadline)
	{
		m_iPendingClassIndex.reset();
		m_iPendingClassChangeSequence = 0u;
		m_strStatus =
			"Class change was not observed within 5 seconds; the active presentation was kept.";
	}
	for (size_t index = 0; index < ARENA_SPAWN_OPTIONS.size(); ++index)
	{
		if (m_Replication.Has_WorldEntity(
			ARENA_SPAWN_OPTIONS[index].pArchetypeId))
		{
			m_ArenaSpawnAccepted[index] = true;
			if (m_iPendingArenaSpawnIndex == index ||
				m_iArenaSpawnIntentIndex == index)
			{
				Reset_ArenaSpawnRequest();
			}
		}
	}
	Advance_ArenaSpawnRequest();
	if (m_iPendingArenaSpawnIndex.has_value() &&
		std::chrono::steady_clock::now() >= m_ArenaSpawnRequestDeadline)
	{
		m_pArenaSpawnGate->Mark_ResponseTimedOut();
		m_iPendingArenaSpawnIndex.reset();
		m_iArenaSpawnIntentIndex.reset();
		m_ValtanEffectPreparationTargets.clear();
		m_strStatus =
			"Arena spawn response timed out; retry is available.";
	}
}

void CLevel_CharacterSelect::Fail_ServerArena(const string& reason)
{
	CAnimationTargetService::Unbind(m_pActiveCharacter);
	m_pActiveCharacter.reset();
	CNetworkManager::Get().Close_ServerConnection();
	m_Replication.Reset();
	m_PlayerController.Set_LocalCharacter(nullptr);
	m_eMode = MODE::RETURNING_TO_LOBBY;
	Reset_ArenaSpawnRequest();
	m_iPendingClassIndex.reset();
	m_iPendingClassChangeSequence = 0u;
	Reset_ClassPresentationPreparation();
	m_strClassPresentationCommitWarning.clear();
	m_strStatus = reason + " Returning to Lobby; local gameplay fallback is disabled.";
	if (!CLevelTransitionService::Request_Load(
		LEVEL::LOBBY,
		"character-select.server-disconnect"))
	{
		m_strStatus += " " + CLevelTransitionService::Get_Status();
	}
}

bool_t CLevel_CharacterSelect::Request_SelectedArenaSpawn()
{
	if (m_isCreateCharacterModalOpen || MODE::SERVER_ARENA != m_eMode ||
		Is_ClassPresentationPreparationPending() ||
		m_iSelectedArenaSpawnIndex >= ARENA_SPAWN_OPTIONS.size() ||
		m_iPendingArenaSpawnIndex.has_value() ||
		m_pArenaSpawnGate->Is_Busy() ||
		m_ArenaSpawnAccepted[m_iSelectedArenaSpawnIndex])
	{
		return false;
	}
	const ARENA_SPAWN_OPTION& option =
		ARENA_SPAWN_OPTIONS[m_iSelectedArenaSpawnIndex];
	if (!m_pArenaSpawnGate->Begin(option.requiresValtanPrewarm))
		return false;
	m_iArenaSpawnIntentIndex = m_iSelectedArenaSpawnIndex;

	if (option.requiresValtanPrewarm)
	{
		VALTAN_PATTERN_EFFECT_CUE_DOCUMENT CueDocument;
		std::string Status;
		if (!CValtanPatternEffectCueDocument::Load_ForProductPrewarm(
				CueDocument, Status) ||
			CueDocument.Cues.size() !=
				CCharacterSelectArenaSpawnGate::PRODUCT_EFFECT_TARGET_COUNT)
		{
			if (CueDocument.Cues.size() !=
					CCharacterSelectArenaSpawnGate::PRODUCT_EFFECT_TARGET_COUNT &&
				!CueDocument.Cues.empty())
			{
				Status = "Valtan Product Effect cue contract expected " +
					std::to_string(CCharacterSelectArenaSpawnGate::
						PRODUCT_EFFECT_TARGET_COUNT) +
					" targets, but loaded " +
					std::to_string(CueDocument.Cues.size()) + ".";
			}
			Isolate_ValtanSpawnPreparationFailure(Status, false);
			return false;
		}

		std::vector<std::string> EffectAssetIds;
		EffectAssetIds.reserve(CueDocument.Cues.size());
		for (const VALTAN_PATTERN_EFFECT_CUE& Cue : CueDocument.Cues)
			EffectAssetIds.push_back(Cue.strEffectAssetId);
		if (!CEffectPresentationService::Queue_ProductTargets_Priority(
				EffectAssetIds,
				m_ValtanEffectPreparationTargets,
				Status) ||
			m_ValtanEffectPreparationTargets.size() !=
				CCharacterSelectArenaSpawnGate::PRODUCT_EFFECT_TARGET_COUNT)
		{
			if (m_ValtanEffectPreparationTargets.size() !=
					CCharacterSelectArenaSpawnGate::PRODUCT_EFFECT_TARGET_COUNT &&
				!Status.empty())
			{
				Status += " Exact 99-target registration was not committed.";
			}
			Isolate_ValtanSpawnPreparationFailure(Status, false);
			return false;
		}
		m_ValtanPrewarmDeadline = std::chrono::steady_clock::now() +
			CCharacterSelectArenaSpawnGate::PREWARM_TIMEOUT;
		m_strStatus = "Priority-prewarming 99 Valtan Product Effects before "
			"the Server spawn request.";
		return true;
	}

	Advance_ArenaSpawnRequest();
	return m_iPendingArenaSpawnIndex == m_iSelectedArenaSpawnIndex;
}

void CLevel_CharacterSelect::Advance_ArenaSpawnRequest()
{
	if (!m_iArenaSpawnIntentIndex.has_value() ||
		*m_iArenaSpawnIntentIndex >= ARENA_SPAWN_OPTIONS.size())
	{
		return;
	}
	const size_t optionIndex = *m_iArenaSpawnIntentIndex;
	const ARENA_SPAWN_OPTION& option = ARENA_SPAWN_OPTIONS[optionIndex];

	if (m_pArenaSpawnGate->Is_Preparing())
	{
		const EFFECT_PRODUCT_PREWARM_TARGET_PROBE Probe =
			CEffectPresentationService::Get_ProductCuePreparationProbe(
				m_ValtanEffectPreparationTargets);
		if (Probe.bSettled)
		{
			const bool_t bAllTargetsPrepared =
				Probe.iTargetCount == CCharacterSelectArenaSpawnGate::
					PRODUCT_EFFECT_TARGET_COUNT &&
				Probe.iPreparedCount == CCharacterSelectArenaSpawnGate::
					PRODUCT_EFFECT_TARGET_COUNT &&
				0u == Probe.iPendingCount && 0u == Probe.iFailedCount &&
				0u == Probe.iUnavailableCount;
			if (!bAllTargetsPrepared ||
				!m_pArenaSpawnGate->Mark_PrewarmReady())
			{
				Isolate_ValtanSpawnPreparationFailure(
					"One or more Valtan Product Effects failed closed during "
					"priority prewarm.", false);
				return;
			}
		}
		else
		{
			if (std::chrono::steady_clock::now() >= m_ValtanPrewarmDeadline)
			{
				Isolate_ValtanSpawnPreparationFailure(
					"Valtan Product Effect priority prewarm timed out after "
					"30 seconds.", true);
				return;
			}
			m_strStatus = "Priority-prewarming Valtan Product Effects " +
				std::to_string(Probe.iPreparedCount) + "/" +
				std::to_string(Probe.iTargetCount) +
				" (boss pending " + std::to_string(Probe.iPendingCount) +
				", total preserved queue pending " +
				std::to_string(Probe.iQueuePendingCount) + ").";
			return;
		}
	}

	if (CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::REQUEST_READY !=
		m_pArenaSpawnGate->Get_State())
	{
		return;
	}
	if (option.requiresValtanPrewarm &&
		FAILED(CValtanPresentationAssetService::Ensure_Prototypes(
			m_pDevice,
			m_pContext,
			ETOUI(LEVEL::CHARACTER_SELECT))))
	{
		Isolate_ValtanSpawnPreparationFailure(
			"Valtan prototypes failed to prepare after Product Effect prewarm.",
			false);
		return;
	}

	if (!m_pArenaSpawnGate->Try_ConsumeServerRequest())
		return;
	if (nullptr == m_pWorldEntityCommandSink ||
		!m_pWorldEntityCommandSink->Request_SpawnWorldEntity(option.pStableId))
	{
		m_pArenaSpawnGate->Mark_RequestFailed();
		m_iArenaSpawnIntentIndex.reset();
		m_ValtanEffectPreparationTargets.clear();
		m_strStatus = std::string{ option.pLabel } +
			" spawn request could not be sent; retry is available.";
		return;
	}
	m_iPendingArenaSpawnIndex = optionIndex;
	m_ArenaSpawnRequestDeadline =
		std::chrono::steady_clock::now() + ARENA_SPAWN_REQUEST_TIMEOUT;
	m_strStatus = std::string{ option.pLabel } +
		" spawn requested from Server.";
}

void CLevel_CharacterSelect::Isolate_ValtanSpawnPreparationFailure(
	const std::string& reason,
	const bool_t bTimedOut)
{
	m_pArenaSpawnGate->Mark_PrewarmFailed(bTimedOut);
	m_iArenaSpawnIntentIndex.reset();
	m_ValtanEffectPreparationTargets.clear();
	m_strStatus = reason +
		" No Server spawn was requested; retry is available.";
	OutputDebugStringA((
		"[Level_CharacterSelect] Valtan spawn preparation isolated: " +
		m_strStatus + "\n").c_str());
}

void CLevel_CharacterSelect::Reset_ArenaSpawnRequest()
{
	m_pArenaSpawnGate->Reset();
	m_iArenaSpawnIntentIndex.reset();
	m_iPendingArenaSpawnIndex.reset();
	m_ValtanEffectPreparationTargets.clear();
}

void CLevel_CharacterSelect::Open_CreateCharacterModal()
{
	if (MODE::SERVER_ARENA != m_eMode ||
		m_iSelectedClassIndex >= SUPPORTED_CLASSES.size() ||
		m_iPendingClassIndex.has_value() ||
		Is_ClassPresentationPreparationPending() ||
		CLevelTransitionService::Is_Pending())
	{
		m_strStatus =
			"Character creation is unavailable while another action is pending.";
		return;
	}

	m_isCreateCharacterModalOpen = true;
	ImGui::OpenPopup("Create Character");
}

bool_t CLevel_CharacterSelect::Confirm_CreateCharacter()
{
	if (MODE::SERVER_ARENA != m_eMode ||
		m_iSelectedClassIndex >= SUPPORTED_CLASSES.size() ||
		Is_ClassPresentationPreparationPending())
	{
		m_strStatus = "The selected class is unavailable.";
		return false;
	}

	const std::string nickname{ m_NicknameDraft.data() };
	if (!LostArk::Shared::Is_Valid_PlayerNickname(nickname))
	{
		m_strStatus =
			"Nickname must be valid UTF-8, contain 1-32 bytes, and contain no control or edge whitespace.";
		return false;
	}

	if (!CCharacterSelectionState::Stage_Creation(
		SUPPORTED_CLASSES[m_iSelectedClassIndex], nickname))
	{
		m_strStatus = "The character identity could not be staged.";
		return false;
	}

	if (!Enter_Stage(LOBBY_STAGE::BERN))
	{
		CCharacterSelectionState::Cancel_PendingCreation();
		return false;
	}
	return true;
}

void CLevel_CharacterSelect::Cancel_CreateCharacter()
{
	CCharacterSelectionState::Cancel_PendingCreation();
	m_isCreateCharacterModalOpen = false;
	ImGui::CloseCurrentPopup();
}

void CLevel_CharacterSelect::Render_CreateCharacterModal()
{
	if (!m_isCreateCharacterModalOpen)
		return;

	if (!ImGui::BeginPopupModal(
		"Create Character",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings))
	{
		return;
	}

	ImGui::TextUnformatted("Enter a nickname for Bern.");
	const bool_t confirmFromEnter = ImGui::InputText(
		"Nickname",
		m_NicknameDraft.data(),
		m_NicknameDraft.size(),
		ImGuiInputTextFlags_EnterReturnsTrue);

	const bool_t confirmFromButton =
		ImGui::Button("Create and Enter Bern");
	ImGui::SameLine();
	const bool_t cancel = ImGui::Button("Cancel");

	if (cancel)
	{
		Cancel_CreateCharacter();
	}
	else if ((confirmFromEnter || confirmFromButton) &&
		Confirm_CreateCharacter())
	{
		m_isCreateCharacterModalOpen = false;
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

bool_t CLevel_CharacterSelect::Enter_Stage(const LOBBY_STAGE stage)
{
	const char_t* stageName = Get_StageName(stage);
	const char_t* transitionSource = Get_StageTransitionSource(stage);
	if (MODE::SERVER_ARENA != m_eMode || nullptr == transitionSource ||
		m_iSelectedClassIndex >= SUPPORTED_CLASSES.size() ||
		m_iPendingClassIndex.has_value() ||
		Is_ClassPresentationPreparationPending() ||
		CLevelTransitionService::Is_Pending())
	{
		m_strStatus = "The selected stage is not available here.";
		return false;
	}
	if (!CCharacterSelectionState::Select(
		SUPPORTED_CLASSES[m_iSelectedClassIndex]))
	{
		m_strStatus = "The selected class could not be preserved for entry.";
		return false;
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

	m_eMode = MODE::RETURNING_TO_LOBBY;
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
			ImVec2(viewport->WorkPos.x + 224.f, viewport->WorkPos.y + 24.f),
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

	ImGui::TextUnformatted("Server-authorized Character Select");
	const bool_t isConnecting = MODE::CONNECTING == m_eMode;
	const bool_t isServerArena = MODE::SERVER_ARENA == m_eMode;
	const bool_t isReturning = MODE::RETURNING_TO_LOBBY == m_eMode;
	const bool_t transitionPending = CLevelTransitionService::Is_Pending();
	if (isConnecting)
		ImGui::TextDisabled("Waiting for the approved Server character...");
	else if (isReturning)
		ImGui::TextDisabled("Returning to Lobby...");

	ImGui::Separator();
	ImGui::TextUnformatted("Playable class");
	ImGui::BeginDisabled(!isServerArena || transitionPending ||
		m_isCreateCharacterModalOpen ||
		m_iPendingClassIndex.has_value() ||
		Is_ClassPresentationPreparationPending());
	for (size_t index = 0; index < SUPPORTED_CLASSES.size(); ++index)
	{
		if (ImGui::Selectable(
			Get_CharacterClassName(SUPPORTED_CLASSES[index]),
			index == m_iSelectedClassIndex))
		{
			Request_ClassChange(index);
		}
	}
	ImGui::EndDisabled();
	if (Is_ClassPresentationPreparationPending())
		ImGui::TextDisabled("Preparing the Server-approved class presentation...");
	else if (m_iPendingClassIndex.has_value())
		ImGui::TextDisabled("Waiting for Server class-change approval and snapshot...");

	if (isServerArena)
	{
		ImGui::Separator();
		ImGui::TextUnformatted("Server arena spawn");
		ImGui::BeginDisabled(m_pArenaSpawnGate->Is_Busy());
		for (size_t index = 0; index < ARENA_SPAWN_OPTIONS.size(); ++index)
		{
			if (ImGui::RadioButton(
				ARENA_SPAWN_OPTIONS[index].pLabel,
				m_iSelectedArenaSpawnIndex == index))
			{
				m_iSelectedArenaSpawnIndex = index;
			}
			if (index + 1u < ARENA_SPAWN_OPTIONS.size())
				ImGui::SameLine();
		}
		ImGui::EndDisabled();
		ImGui::BeginDisabled(
			Is_ClassPresentationPreparationPending() ||
			m_iPendingArenaSpawnIndex.has_value() ||
			m_pArenaSpawnGate->Is_Busy() ||
			m_ArenaSpawnAccepted[m_iSelectedArenaSpawnIndex]);
		if (ImGui::Button("Spawn Selected"))
			Request_SelectedArenaSpawn();
		ImGui::EndDisabled();
		if (m_ArenaSpawnAccepted[m_iSelectedArenaSpawnIndex])
			ImGui::SameLine(), ImGui::TextDisabled("Spawned");
		else if (m_iPendingArenaSpawnIndex == m_iSelectedArenaSpawnIndex)
			ImGui::SameLine(), ImGui::TextDisabled("Requested");
		else if (m_pArenaSpawnGate->Is_Preparing() &&
			m_iArenaSpawnIntentIndex == m_iSelectedArenaSpawnIndex)
		{
			ImGui::SameLine();
			ImGui::TextDisabled("Prewarming 99 boss Effects");
		}
		else if (m_pArenaSpawnGate->Can_Retry())
			ImGui::SameLine(), ImGui::TextDisabled("Retry available");
#ifdef _DEBUG
		if (ImGui::Checkbox(
			"Show Combat Colliders",
			&m_isCombatColliderDebugVisible))
		{
			m_Replication.Set_CombatColliderDebugVisible(
				m_isCombatColliderDebugVisible);
		}
		if (ImGui::Checkbox(
			"Show Skill Hit Areas",
			&m_isSkillHitAreaDebugVisible))
		{
			m_Replication.Set_SkillHitAreaDebugVisible(
				m_isSkillHitAreaDebugVisible);
		}
#endif
	}

	ImGui::Separator();
	ImGui::BeginDisabled(isConnecting || isReturning || transitionPending ||
		m_isCreateCharacterModalOpen ||
		m_iPendingClassIndex.has_value() ||
		Is_ClassPresentationPreparationPending());
	if (ImGui::Button("Create Character"))
		Open_CreateCharacterModal();
	ImGui::SameLine();
	if (ImGui::Button("Enter Valtan Map"))
		Enter_Stage(LOBBY_STAGE::VALTAN);
	ImGui::SameLine();
	if (ImGui::Button("Back"))
		Fail_ServerArena("Leaving Server Arena.");
	ImGui::EndDisabled();

	ImGui::TextDisabled(
		"F1: tools  |  F6: follow/free  |  Server-authorized skill input enabled");
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	Render_CreateCharacterModal();
	ImGui::End();
}

namespace
{
	/* Mirrors the rects CHUDLayoutTool wrote for the same slot ids in
	Data/UI/ClassSelect/ClassSelect_Layout.json -- CHUDRuntimeView has no public slot query, so
	the click targets (and the hand-drawn category list / thumbnail / confirm button, which
	aren't slot-driven at all) are kept in sync with that JSON by hand here. */
	struct CLASS_LIST_ENTRY
	{
		f32_t fX, fWidth, fHeight;
		size_t iSupportedClassIndex;
		/* "Warlord" etc -- matches ClassSelect_Layout.json's "classes" array and each per-class
		slot's ownerClass, not Get_CharacterClassName()'s display text ("Dimension Master" has a
		space CHUDRuntimeView's ownerClass match would never see). */
		const char* pJsonClassName;
		const char* pCategoryLabel;
		const char* pClassLabel;
		/* File under UI/ClassSelect/Common/, or nullptr where no category symbol has been cut
		yet (Specialist(M) -- text-only per an earlier explicit "substitute for now"). Drawn as a
		small square at the row's left edge, not stretched across the row: that stretch (a
		280x48 slot layer scaling a ~76x72 source) was the exact "horizontally squashed symbol"
		this replaces. */
		const char* pCategorySymbolFile;
	};

	/* No fY here: rows accordion (a click pushes every row below it down by the expanded
	thumbnail's height instead of the thumbnail always drawing in one fixed spot), so each row's
	actual y is only known at render time -- see Render_ClassList's running fRowY. */
	constexpr CLASS_LIST_ENTRY CLASS_LIST_ENTRIES[] =
	{
		{ 950.f, 280.f, 48.f, 5, "Warlord",         "\xec\xa0\x84\xec\x82\xac(\xeb\x82\xa8)", "\xec\x9b\x8c\xeb\xa1\x9c\xeb\x93\x9c", "CategorySymbol_Warrior.png" },
		{ 950.f, 280.f, 48.f, 2, "Slayer",          "\xec\xa0\x84\xec\x82\xac(\xec\x97\xac)", "\xec\x8a\xac\xeb\xa0\x88\xec\x9d\xb4\xec\x96\xb4", "CategorySymbol_Warrior.png" },
		{ 950.f, 280.f, 48.f, 0, "LanceMaster",     "\xeb\xac\xb4\xeb\x8f\x84\xea\xb0\x80(\xec\x97\xac)", "\xec\xb0\xbd\xec\x88\xa0\xec\x82\xac", "CategorySymbol_MartialW.png" },
		{ 950.f, 280.f, 48.f, 1, "Gunslinger",      "\xed\x97\x8c\xed\x84\xb0(\xec\x97\xac)", "\xea\xb1\xb4\xec\x8a\xac\xeb\xa7\x81\xea\xb1\xb0", nullptr },
		{ 950.f, 280.f, 48.f, 3, "Artist",          "\xec\x8a\xa4\xed\x8e\x98\xec\x85\x9c\xeb\xa6\xac\xec\x8a\xa4\xed\x8a\xb8(\xec\x97\xac)", "\xeb\x8f\x84\xed\x99\x94\xea\xb0\x80", "CategorySymbol_SpecialistF.png" },
		{ 950.f, 280.f, 48.f, 4, "DimensionMaster", "\xec\x8a\xa4\xed\x8e\x98\xec\x85\x9c\xeb\xa6\xac\xec\x8a\xa4\xed\x8a\xb8(\xeb\x82\xa8)", "\xec\xb0\xa8\xec\x9b\x90\xec\x88\xa0\xec\x82\xac", "CategorySymbol_Specialist_M.png" },
	};

	constexpr f32_t REF_WIDTH = 1280.f;
	constexpr f32_t REF_HEIGHT = 720.f;

	/* <Class>_IdentityDescription's rect ended up identical for every class once LanceMaster/
	Artist/DimensionMaster were lined up on Warlord's values, so one shared rect (rather than
	one per class) drives where this centers each class's identity blurb. */
	constexpr f32_t IDENTITY_DESC_X = 19.2857151f;
	constexpr f32_t IDENTITY_DESC_Y = 568.571472f;
	constexpr f32_t IDENTITY_DESC_WIDTH = 210.f;
	constexpr f32_t IDENTITY_DESC_LINE_HEIGHT = 18.f;

	constexpr const char* WARLORD_IDENTITY_DESC[] = {
		"\xec\xa0\x81\xec\x9d\x84\x20\xea\xb3\xb5\xea\xb2\xa9\xed\x95\xb4\x20\xec\x8b\xa4\xeb\x93\x9c\x20\xea\xb2\x8c\xec\x9d\xb4\xec\xa7\x80\xeb\xa5\xbc\x20\xeb\xaa\xa8\xec\x9d\x80\x20\xeb\x92\xa4",
		"\x5a\xed\x82\xa4\xeb\xa1\x9c\x20\xec\x9e\x90\xec\x8b\xa0\xec\x9d\x84\x20\xeb\xb3\xb4\xed\x98\xb8\xed\x95\x98\xea\xb3\xa0\x20\x58\xed\x82\xa4\xeb\xa1\x9c\x20\xed\x8c\x8c\xed\x8b\xb0\xec\x9b\x90\xec\x9d\x84\x20\xec\xa7\x80\xec\xbc\x9c",
		"\xec\xa4\x84\x20\xec\x88\x98\x20\xec\x9e\x88\xec\x8a\xb5\xeb\x8b\x88\xeb\x8b\xa4",
	};
	constexpr const char* LANCEMASTER_IDENTITY_DESC[] = {
		"\x5a\xed\x82\xa4\xeb\xa5\xbc\x20\xec\x82\xac\xec\x9a\xa9\xed\x95\x98\xec\x97\xac\x20\xeb\x82\x9c\xeb\xac\xb4\xec\x99\x80\x20\xec\xa7\x91\xec\xa4\x91\x20\xec\x8a\xa4\xed\x83\xa0\xec\x8a\xa4\xeb\xa1\x9c",
		"\xec\x9e\x90\xec\x9c\xa0\xeb\xa1\xad\xea\xb2\x8c\x20\xeb\xb3\x80\xea\xb2\xbd\xed\x95\xa0\x20\xec\x88\x98\x20\xec\x9e\x88\xea\xb3\xa0\x20\xea\xb2\x8c\xec\x9d\xb4\xec\xa7\x80\xeb\xa5\xbc\x20\xeb\xaa\xa8\xec\x9d\x80",
		"\xed\x9b\x84\x20\xeb\xb3\x80\xea\xb2\xbd\x20\xec\x8b\x9c\x20\xec\xb6\x94\xea\xb0\x80\xed\x9a\xa8\xea\xb3\xbc\xeb\xa5\xbc\x20\xec\x96\xbb\xec\x9d\x84\x20\xec\x88\x98\x20\xec\x9e\x88\xec\x8a\xb5\xeb\x8b\x88\xeb\x8b\xa4\x2e",
	};
	constexpr const char* ARTIST_IDENTITY_DESC[] = {
		"\xec\xa0\x81\xec\x9d\x84\x20\xea\xb3\xb5\xea\xb2\xa9\xed\x95\xb4\x20\xec\xa1\xb0\xed\x99\x94\x20\xea\xb2\x8c\xec\x9d\xb4\xec\xa7\x80\xeb\xa5\xbc\x20\xeb\xaa\xa8\xec\x9d\x80\x20\xeb\x92\xa4\x20\x5a\xed\x82\xa4\xeb\xa1\x9c",
		"\xed\x8c\x8c\xed\x8b\xb0\xec\x9b\x90\xec\x9d\x98\x20\xea\xb3\xb5\xea\xb2\xa9\xeb\xa0\xa5\xec\x9d\x84\x20\xec\xa6\x9d\xea\xb0\x80\xec\x8b\x9c\xed\x82\xa4\xea\xb1\xb0\xeb\x82\x98\x2c\x20\x58\xed\x82\xa4\xeb\xa1\x9c",
		"\xed\x8c\x8c\xed\x8b\xb0\xec\x9b\x90\x20\xed\x95\x9c\x20\xeb\xaa\x85\xec\x9d\x84\x20\xed\x9a\x8c\xeb\xb3\xb5\xec\x8b\x9c\xed\x82\xac\x20\xec\x88\x98\x20\xec\x9e\x88\xec\x8a\xb5\xeb\x8b\x88\xeb\x8b\xa4\x2e",
	};
	constexpr const char* DIMENSIONMASTER_IDENTITY_DESC[] = {
		"\xec\xb0\xa8\xec\x9b\x90\xec\x88\xa0\xec\x82\xac\xec\x9d\x98\x20\xec\x95\x84\xec\x9d\xb4\xeb\x8d\xb4\xed\x8b\xb0\xed\x8b\xb0\xec\x9d\xb8\x20\x27\xec\xb0\xa8\xec\x9b\x90\xec\x8b\x9c\xea\xb3\x84\x27\xeb\x8a\x94\x20\xec\xa0\x81\xec\x97\x90\xea\xb2\x8c",
		"\xec\x8a\xa4\xed\x82\xac\xec\x9d\x84\x20\xec\xa0\x81\xec\xa4\x91\xec\x8b\x9c\xed\x82\xac\x20\xeb\x95\x8c\x20\xeb\xa7\x88\xeb\x8b\xa4\x20\xec\x8b\x9c\xea\xb0\x84\xec\x9d\xb4\x20\xea\xb0\x80\xec\x86\x8d\xeb\x90\xa9\xeb\x8b\x88\xeb\x8b\xa4\x2e",
		"\xec\xb0\xa8\xec\x9b\x90\x20\xec\x8b\x9c\xea\xb3\x84\xeb\x8a\x94\x20\xec\xb0\xa8\xec\x9b\x90\xec\x88\xa0\xec\x82\xac\xea\xb0\x80\x20\xec\x86\x8d\xed\x95\x9c\x20\xec\x8b\x9c\xea\xb0\x84\xea\xb3\xbc\x20\xeb\x8f\x99\xea\xb8\xb0\xed\x99\x94",
		"\xeb\x90\x98\xec\x96\xb4\x20\xec\x9e\x88\xec\x96\xb4\x2c\x20\xec\xb0\xa8\xec\x9b\x90\xec\x8b\x9c\xea\xb3\x84\xec\x9d\x98\x20\xec\x8b\x9c\xea\xb0\x84\xec\x9d\xb4\x20\xeb\xb9\xa0\xeb\xa5\xb4\xea\xb2\x8c\x20\xed\x9d\x90\xeb\xa5\xb4\xeb\xa9\xb4",
		"\xec\xb0\xa8\xec\x9b\x90\xec\x88\xa0\xec\x82\xac\x20\xeb\xb3\xb8\xec\x9d\xb8\x20\xeb\x98\x90\xed\x95\x9c\x20\xea\xb0\x80\xec\x86\x8d\xeb\x90\x98\xeb\x8a\x94\x20\xed\x8a\xb9\xec\xa7\x95\xec\x9d\x84\x20\xea\xb0\x80\xec\xa7\x80\xea\xb3\xa0",
		"\xec\x9e\x88\xec\x8a\xb5\xeb\x8b\x88\xeb\x8b\xa4\x2e",
	};

	struct IDENTITY_DESCRIPTION
	{
		const char* pJsonClassName;
		const char* const* ppLines;
		int32_t iLineCount;
	};

	constexpr IDENTITY_DESCRIPTION IDENTITY_DESCRIPTIONS[] = {
		{ "Warlord", WARLORD_IDENTITY_DESC, static_cast<int32_t>(std::size(WARLORD_IDENTITY_DESC)) },
		{ "LanceMaster", LANCEMASTER_IDENTITY_DESC, static_cast<int32_t>(std::size(LANCEMASTER_IDENTITY_DESC)) },
		{ "Artist", ARTIST_IDENTITY_DESC, static_cast<int32_t>(std::size(ARTIST_IDENTITY_DESC)) },
		{ "DimensionMaster", DIMENSIONMASTER_IDENTITY_DESC, static_cast<int32_t>(std::size(DIMENSIONMASTER_IDENTITY_DESC)) },
	};

	constexpr f32_t ROW_Y_START = 60.f;
	constexpr f32_t ROW_GAP = 7.f;
	constexpr f32_t THUMB_W = 134.f;
	constexpr f32_t THUMB_H = 78.f;
	constexpr f32_t THUMB_MARGIN_TOP = 10.f;
	constexpr f32_t THUMB_MARGIN_BOTTOM = 10.f;

	string Build_ClassSelectAssetPath(const char* pClassName, const char* pFileName)
	{
		return string("UI/ClassSelect/") + pClassName + "/" + pFileName;
	}
}

void CLevel_CharacterSelect::Render_ClassList()
{
	if (nullptr == m_pClassSelectView)
		return;

	const string strSelectedClass = m_iSelectedClassIndex < SUPPORTED_CLASSES.size()
		? [this]() -> string
		{
			for (const CLASS_LIST_ENTRY& Entry : CLASS_LIST_ENTRIES)
				if (Entry.iSupportedClassIndex == m_iSelectedClassIndex)
					return Entry.pJsonClassName;
			return {};
		}()
		: string{};

	m_pClassSelectView->Render(strSelectedClass, 0);

	ImGuiViewport* pViewport = ImGui::GetMainViewport();
	const f32_t fScaleX = pViewport->WorkSize.x / REF_WIDTH;
	const f32_t fScaleY = pViewport->WorkSize.y / REF_HEIGHT;
	ImDrawList* pDrawList = ImGui::GetForegroundDrawList(pViewport);

	const auto Fn_ToScreen = [&](f32_t fX, f32_t fY) -> ImVec2
	{
		return ImVec2(
			pViewport->WorkPos.x + fX * fScaleX,
			pViewport->WorkPos.y + fY * fScaleY);
	};

	const ImVec2 vMouse = ImGui::GetMousePos();
	const bool_t bClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
	const bool_t bInteractable = MODE::SERVER_ARENA == m_eMode &&
		!m_isCreateCharacterModalOpen &&
		!m_iPendingClassIndex.has_value() &&
		!Is_ClassPresentationPreparationPending() &&
		!CLevelTransitionService::Is_Pending();

	/* ImGui only ships the one HANYoonGothic330 weight (see ImGuiLayer::Initialize), so "bold"
	here is the standard faux-bold trick: the same glyphs redrawn a few pixels apart so their
	strokes overlap and thicken, instead of a real heavier-weight font asset. */
	const auto Fn_DrawBoldText = [&](f32_t fX, f32_t fY, f32_t fSize, ImU32 iColor, const char* pText)
	{
		ImFont* pFont = ImGui::GetFont();
		const ImVec2 vPos = Fn_ToScreen(fX, fY);
		const f32_t fScreenSize = fSize * (std::min)(fScaleX, fScaleY);
		constexpr f32_t fOffsets[][2] = { {0.f, 0.f}, {1.f, 0.f}, {0.f, 1.f}, {1.f, 1.f} };
		for (const f32_t (&Offset)[2] : fOffsets)
		{
			pDrawList->AddText(pFont, fScreenSize,
				ImVec2(vPos.x + Offset[0], vPos.y + Offset[1]), iColor, pText);
		}
	};

	/* Centers pText within [fRectX, fRectX + fRectWidth) at reference scale, using this text's
	own font-size metrics (CalcTextSizeA) rather than assuming a fixed glyph width -- Korean
	glyphs at a non-default size don't have a simple char-count-based width. */
	const auto Fn_DrawBoldTextCentered = [&](f32_t fRectX, f32_t fRectWidth, f32_t fY, f32_t fSize,
		ImU32 iColor, const char* pText)
	{
		ImFont* pFont = ImGui::GetFont();
		const f32_t fScreenSize = fSize * (std::min)(fScaleX, fScaleY);
		const ImVec2 vTextSize = pFont->CalcTextSizeA(fScreenSize, FLT_MAX, 0.f, pText);
		const f32_t fRectCenterX = fRectX + fRectWidth * 0.5f;
		const ImVec2 vScreenCenter = Fn_ToScreen(fRectCenterX, fY);
		constexpr f32_t fOffsets[][2] = { {0.f, 0.f}, {1.f, 0.f}, {0.f, 1.f}, {1.f, 1.f} };
		for (const f32_t (&Offset)[2] : fOffsets)
		{
			pDrawList->AddText(pFont, fScreenSize,
				ImVec2(vScreenCenter.x - vTextSize.x * 0.5f + Offset[0], vScreenCenter.y + Offset[1]),
				iColor, pText);
		}
	};

	/* Left panel text: JSON slots only carry images, so the class name, the three yellow section
	labels, and the identity blurb are drawn here against the rects CHUDLayoutTool wrote for
	<Class>_NameSymbol / _IdentityDescription in ClassSelect_Layout.json. <Class>_Description's
	rect is currently unused/overlapped by IdentityID after the identity section moved up under
	the tags, so no text is drawn there. */
	for (const CLASS_LIST_ENTRY& Entry : CLASS_LIST_ENTRIES)
	{
		if (Entry.iSupportedClassIndex != m_iSelectedClassIndex)
			continue;

		/* Aligned against Warlord_NameSymbol's current rect (50x50 at y=191.29): text sits to
		the symbol's right, vertically centered on its 50px height. */
		Fn_DrawBoldText(72.f, 203.f, 32.f, IM_COL32(255, 255, 255, 255), Entry.pClassLabel);
		Fn_DrawBoldText(15.f, 262.f, 20.f, IM_COL32(255, 220, 140, 255),
			"\xec\xa1\xb0\xec\x9e\x91 \xeb\x82\x9c\xec\x9d\xb4\xeb\x8f\x84");
		Fn_DrawBoldText(15.f, 335.f, 20.f, IM_COL32(255, 220, 140, 255),
			"\xea\xb8\xb0\xeb\xb3\xb8 \xec\xa0\x95\xeb\xb3\xb4");
		Fn_DrawBoldText(15.f, 453.f, 20.f, IM_COL32(255, 220, 140, 255),
			"\xec\x95\x84\xec\x9d\xb4\xeb\x8d\xb4\xed\x8b\xb0\xed\x8b\xb0");

		for (const IDENTITY_DESCRIPTION& Desc : IDENTITY_DESCRIPTIONS)
		{
			if (0 != strcmp(Desc.pJsonClassName, Entry.pJsonClassName))
				continue;

			for (int32_t iLine = 0; iLine < Desc.iLineCount; ++iLine)
			{
				Fn_DrawBoldTextCentered(IDENTITY_DESC_X, IDENTITY_DESC_WIDTH,
					IDENTITY_DESC_Y + static_cast<f32_t>(iLine) * IDENTITY_DESC_LINE_HEIGHT,
					16.f, IM_COL32(220, 220, 220, 255), Desc.ppLines[iLine]);
			}
			break;
		}
		break;
	}

	pDrawList->AddText(Fn_ToScreen(1000.f, 20.f), IM_COL32(255, 220, 140, 255),
		"\xed\x81\xb4\xeb\x9e\x98\xec\x8a\xa4 \xec\x84\xa0\xed\x83\x9d");

	/* Accordion: fRowY advances past each row, and past the expanded row's thumbnail block too,
	so a category expanding pushes every row beneath it down instead of the thumbnail always
	drawing in one fixed spot regardless of which category opened it. */
	const int32_t iExpandedBefore = m_iExpandedCategory;
	f32_t fRowY = ROW_Y_START;
	for (int32_t i = 0; i < static_cast<int32_t>(std::size(CLASS_LIST_ENTRIES)); ++i)
	{
		const CLASS_LIST_ENTRY& Entry = CLASS_LIST_ENTRIES[i];
		const ImVec2 vTopLeft = Fn_ToScreen(Entry.fX, fRowY);
		const ImVec2 vBotRight = Fn_ToScreen(Entry.fX + Entry.fWidth, fRowY + Entry.fHeight);

		const bool_t bHovered = bInteractable &&
			vMouse.x >= vTopLeft.x && vMouse.x < vBotRight.x &&
			vMouse.y >= vTopLeft.y && vMouse.y < vBotRight.y;
		const bool_t bExpanded = i == iExpandedBefore;

		if (ID3D11ShaderResourceView* pRowBgSRV = m_pClassSelectView->Load_Texture(
			bExpanded ? "UI/ClassSelect/Common/CategorySelected.png" : "UI/ClassSelect/Common/Category.png"))
		{
			pDrawList->AddImage(pRowBgSRV, vTopLeft, vBotRight);
		}

		if (!bExpanded && bHovered)
		{
			pDrawList->AddRect(vTopLeft, vBotRight, IM_COL32(255, 220, 90, 160), 4.f, 0, 1.5f);
		}

		if (nullptr != Entry.pCategorySymbolFile)
		{
			if (ID3D11ShaderResourceView* pSymbolSRV = m_pClassSelectView->Load_Texture(
				string("UI/ClassSelect/Common/") + Entry.pCategorySymbolFile))
			{
				pDrawList->AddImage(pSymbolSRV,
					Fn_ToScreen(Entry.fX + 8.f, fRowY + 6.f),
					Fn_ToScreen(Entry.fX + 44.f, fRowY + 42.f));
			}
		}

		pDrawList->AddText(Fn_ToScreen(Entry.fX + 52.f, fRowY + 16.f),
			IM_COL32(230, 230, 230, 255), Entry.pCategoryLabel);

		if (bHovered && bClicked)
			m_iExpandedCategory = bExpanded ? -1 : i;

		fRowY += Entry.fHeight + ROW_GAP;

		if (bExpanded)
		{
			const bool_t bConfirmed = Entry.iSupportedClassIndex == m_iSelectedClassIndex;
			const f32_t fThumbY = fRowY + THUMB_MARGIN_TOP;

			const ImVec2 vThumbTopLeft = Fn_ToScreen(Entry.fX, fThumbY);
			const ImVec2 vThumbBotRight = Fn_ToScreen(Entry.fX + THUMB_W, fThumbY + THUMB_H);

			ID3D11ShaderResourceView* pThumbSRV = m_pClassSelectView->Load_Texture(
				Build_ClassSelectAssetPath(Entry.pJsonClassName, "IllustrationSmall.png"));
			if (nullptr == pThumbSRV)
				pThumbSRV = m_pClassSelectView->Load_Texture(
					"UI/ClassSelect/Common/CategorySelected.png");
			if (nullptr != pThumbSRV)
			{
				pDrawList->AddImage(pThumbSRV, vThumbTopLeft, vThumbBotRight);
			}

			if (ID3D11ShaderResourceView* pSymbolSRV = m_pClassSelectView->Load_Texture(
				Build_ClassSelectAssetPath(Entry.pJsonClassName, "IdentitySymbol.png")))
			{
				const ImVec2 vSymbolTopLeft = Fn_ToScreen(Entry.fX + THUMB_W - 22.f, fThumbY - 4.f);
				const ImVec2 vSymbolBotRight = Fn_ToScreen(Entry.fX + THUMB_W + 4.f, fThumbY + 22.f);
				pDrawList->AddImage(pSymbolSRV, vSymbolTopLeft, vSymbolBotRight);
			}

			pDrawList->AddText(Fn_ToScreen(Entry.fX + 4.f, fThumbY + THUMB_H - 18.f),
				IM_COL32(255, 255, 255, 255), Entry.pClassLabel);

			const bool_t bThumbHovered = bInteractable &&
				vMouse.x >= vThumbTopLeft.x && vMouse.x < vThumbBotRight.x &&
				vMouse.y >= vThumbTopLeft.y && vMouse.y < vThumbBotRight.y;

			/* Small illust selected.png is the one hover/confirm frame -- both states use the
			same authored art instead of a placeholder AddRect() outline. */
			if (bConfirmed || bThumbHovered)
			{
				if (ID3D11ShaderResourceView* pSelectedFrameSRV = m_pClassSelectView->Load_Texture(
					"UI/ClassSelect/Common/SmallIllustSelected.png"))
				{
					const ImVec2 vFrameTopLeft = Fn_ToScreen(Entry.fX - 2.f, fThumbY - 2.f);
					const ImVec2 vFrameBotRight = Fn_ToScreen(Entry.fX + THUMB_W + 2.f, fThumbY + THUMB_H + 2.f);
					pDrawList->AddImage(pSelectedFrameSRV, vFrameTopLeft, vFrameBotRight);
				}
			}

			if (bThumbHovered && bClicked)
				Request_ClassChange(Entry.iSupportedClassIndex);

			fRowY = fThumbY + THUMB_H + THUMB_MARGIN_BOTTOM;
		}
	}
}

void CLevel_CharacterSelect::Render_ArenaSpawnButtons()
{
	if (nullptr == m_pClassSelectView)
		return;

	ImGuiViewport* pViewport = ImGui::GetMainViewport();
	const f32_t fScaleX = pViewport->WorkSize.x / REF_WIDTH;
	const f32_t fScaleY = pViewport->WorkSize.y / REF_HEIGHT;
	ImDrawList* pDrawList = ImGui::GetForegroundDrawList(pViewport);

	const auto Fn_ToScreen = [&](f32_t fX, f32_t fY) -> ImVec2
	{
		return ImVec2(
			pViewport->WorkPos.x + fX * fScaleX,
			pViewport->WorkPos.y + fY * fScaleY);
	};

	const ImVec2 vMouse = ImGui::GetMousePos();
	const bool_t bClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
	const bool_t bInteractable = MODE::SERVER_ARENA == m_eMode &&
		!m_isCreateCharacterModalOpen &&
		!Is_ClassPresentationPreparationPending() &&
		!CLevelTransitionService::Is_Pending();

	const auto Fn_HitTest = [&](f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight,
		ImVec2& outTopLeft, ImVec2& outBotRight) -> bool_t
	{
		outTopLeft = Fn_ToScreen(fX, fY);
		outBotRight = Fn_ToScreen(fX + fWidth, fY + fHeight);
		return vMouse.x >= outTopLeft.x && vMouse.x < outBotRight.x &&
			vMouse.y >= outTopLeft.y && vMouse.y < outBotRight.y;
	};

	/* ARENA_SPAWN_OPTIONS[1] (Mid Boss / Lugaru) has no button in this row -- only reachable
	through the ImGui debug radio list -- so the middle button maps to option index 2 (Valtan),
	not the array's own middle entry. */
	struct SPAWN_BUTTON final { const char_t* pSlotId; const char_t* pHoverPath; size_t iOptionIndex; };
	constexpr SPAWN_BUTTON SPAWN_BUTTONS[] =
	{
		{ "SpawnMonsterButton", "UI/ClassSelect/Common/SpawnMonsterButtonHover.png", 0u },
		{ "BossSpawnButton", "UI/ClassSelect/Common/BossSpawnButtonHover.png", 2u },
	};

	for (const SPAWN_BUTTON& Button : SPAWN_BUTTONS)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pClassSelectView->Get_SlotRect(Button.pSlotId, fX, fY, fWidth, fHeight))
			continue;

		ImVec2 vTopLeft, vBotRight;
		const bool_t bHitTest = Fn_HitTest(fX, fY, fWidth, fHeight, vTopLeft, vBotRight);
		const bool_t bHovered = bInteractable && bHitTest;

		if (bHovered)
		{
			if (ID3D11ShaderResourceView* pHoverSRV =
				m_pClassSelectView->Load_Texture(Button.pHoverPath))
			{
				pDrawList->AddImage(pHoverSRV, vTopLeft, vBotRight);
			}
		}

		if (bHovered && bClicked)
		{
			m_iSelectedArenaSpawnIndex = Button.iOptionIndex;
			Request_SelectedArenaSpawn();
		}
	}

	/* SpawnCancelButton (rightmost): hover only. No despawn/revert command exists anywhere in
	this codebase (IWorldEntityCommandSink only has Request_SpawnWorldEntity; Server's
	ROOM_COMMAND_TYPE has no despawn entry) -- wiring a click here would mean inventing a new
	Client/Server capability, which wasn't asked for. Leave it unbound until that command exists. */
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (m_pClassSelectView->Get_SlotRect("SpawnCancelButton", fX, fY, fWidth, fHeight))
		{
			ImVec2 vTopLeft, vBotRight;
			if (bInteractable && Fn_HitTest(fX, fY, fWidth, fHeight, vTopLeft, vBotRight))
			{
				if (ID3D11ShaderResourceView* pHoverSRV = m_pClassSelectView->Load_Texture(
					"UI/ClassSelect/Common/SpawnCancelButtonHover.png"))
				{
					pDrawList->AddImage(pHoverSRV, vTopLeft, vBotRight);
				}
			}
		}
	}

	/* Back: same Fail_ServerArena() the ImGui "Back" button already calls. Not gated by
	bInteractable -- an escape hatch should stay clickable through pending/preparing states, only
	guarded against firing again mid-transition. */
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (m_pClassSelectView->Get_SlotRect("GoBackIcon", fX, fY, fWidth, fHeight))
		{
			ImVec2 vTopLeft, vBotRight;
			const bool_t bHovered = !CLevelTransitionService::Is_Pending() &&
				Fn_HitTest(fX, fY, fWidth, fHeight, vTopLeft, vBotRight);
			if (bHovered)
			{
				if (ID3D11ShaderResourceView* pHoverSRV =
					m_pClassSelectView->Load_Texture("UI/ClassSelect/Common/GoBackIconHover.png"))
				{
					pDrawList->AddImage(pHoverSRV, vTopLeft, vBotRight);
				}
			}
			if (bHovered && bClicked)
				Fail_ServerArena("Leaving Server Arena.");
		}
	}
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

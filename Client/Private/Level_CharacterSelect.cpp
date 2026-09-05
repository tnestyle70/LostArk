#include "imgui.h"

#include "Level_CharacterSelect.h"

#include "AnimationEffectCueDocument.h"
#include "AnimationTargetService.h"
#include "ActorCatalog.h"
#include "Camera_Free.h"
#include "Character.h"
#include "CharacterCatalog.h"
#include "CharacterSelectArenaSpawnGate.h"
#include "CharacterSelectionState.h"
#include "CharacterSpec.h"
#include "CombatHUDViewModel.h"
#include "Effect_PresentationService.h"
#include "GameInstance.h"
#include "ImGuiLayer.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "LobbyCommandService.h"
#include "MainApp.h"
#include "Network/PacketMessages.h"
#include "NetworkManager.h"
#include "NetworkPlayerCommandSink.h"
#include "NetworkWorldEntityCommandSink.h"
#include "PlayableCharacterAssetService.h"
#include "RaidEntryPreviewView.h"
#include "Transform.h"
#include "UIInputRouter.h"
#include "UILayoutRuntime.h"
#include "ValtanPatternEffectCueDocument.h"
#include "ValtanPatternTree.h"
#include "ValtanPresentationAssetService.h"

#include <algorithm>
#include <cmath>
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

CLevel_CharacterSelect* CLevel_CharacterSelect::s_pActiveInstance = nullptr;

CLevel_CharacterSelect::CLevel_CharacterSelect(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel{ pDevice, pContext },
	m_pArenaSpawnGate{ std::make_unique<CCharacterSelectArenaSpawnGate>() }
{
	s_pActiveInstance = this;
}

CLevel_CharacterSelect::~CLevel_CharacterSelect()
{
	if (this == s_pActiveInstance)
		s_pActiveInstance = nullptr;
	/* A level teardown with the Create Character modal still open (disconnect -> Lobby, world
	transfer) must release the WM_CHAR capture, or Is_TextInputActive stays stuck true and every
	keybind it gates stays dead for the rest of the process. */
	if (m_isCreateCharacterModalOpen)
		CUIInputRouter::Get().Stop_TextInput();
	CAnimationTargetService::Unbind(m_pActiveCharacter);
	if (!m_preserveServerConnectionForTransfer)
		CNetworkManager::Get().Close_ServerConnection();
	m_Replication.Reset();
	CCombatHUDViewModel::Get().Reset_RuntimeState();
	m_MapRuntime.Clear();
}

HRESULT CLevel_CharacterSelect::Initialize()
{
	if (FAILED(__super::Initialize()))
	{
		OutputDebugStringA(
			"[Level_CharacterSelect] base CLevel::Initialize failed.\n");
		return E_FAIL;
	}
	if (!CCombatHUDViewModel::Get().Initialize_Definitions())
	{
		OutputDebugStringA((
			"[Level_CharacterSelect] CombatHUDViewModel::Initialize_Definitions failed: " +
			CCombatHUDViewModel::Get().Get_Status() + "\n").c_str());
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
		{
			OutputDebugStringA(
				"[Level_CharacterSelect] Selected class is not in SUPPORTED_CLASSES.\n");
			return E_INVALIDARG;
		}
		m_iSelectedClassIndex = static_cast<size_t>(
			std::distance(SUPPORTED_CLASSES.begin(), selected));
	}

	if (FAILED(Ready_Camera()))
		return E_FAIL;

	m_pClassSelectView = std::make_unique<CUILayoutRuntime>(
		m_pDevice, m_pContext, ETOUI(LEVEL::CHARACTER_SELECT), TEXT("Layer_UI"),
		L"UI/ClassSelect/ClassSelect_Layout.json");
	/* Authored layer tints are opaque -- every accordion/right-panel/product-button slot would
	otherwise sit fully visible from this Level's own load until Update_ServerArena's first tick
	flips m_eMode to SERVER_ARENA. Update_ClassList/Update_ArenaSpawnButtons's own early-return
	(still MODE::CONNECTING here) already hides everything they own, so this just runs that path
	once up front instead of duplicating the same slot list a second time. */
	Update_ClassList();
	Update_ArenaSpawnButtons();

#ifdef _DEBUG
	m_pDebugRaidEntryPreviewView =
		std::make_unique<CRaidEntryPreviewView>(
			m_pDevice, m_pContext, ETOUI(LEVEL::CHARACTER_SELECT));
#endif

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
#ifdef _DEBUG
	Update_RaidEntryDebugPreviewKey();
#endif
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

	/* Update_ClassList/Update_ArenaSpawnButtons drive real CUI_Sprite GameObjects now -- same
	Is_DebugRaidEntryPreviewOpen() reasoning Render() used to gate their old ImGui image draws
	with (the O-key preview's own left info column and panel frame occupy this same screen
	region). Unlike the old ImGui pass (which simply stopped drawing that frame), these sprites
	keep showing their last state unless told otherwise, so the preview being open explicitly
	hides them (also stopping their hover/click handling) instead of just skipping the call.
	Is_DebugRaidEntryPreviewOpen() only exists in Debug, so the gate itself has to stay
	Debug-only too. */
#ifdef _DEBUG
	const bool_t isRaidEntryDebugPreviewOpenForClassList = Is_DebugRaidEntryPreviewOpen();
#else
	const bool_t isRaidEntryDebugPreviewOpenForClassList = false;
#endif
	if (isRaidEntryDebugPreviewOpenForClassList)
	{
		Hide_ClassList();
		Hide_ArenaSpawnButtons();
	}
	else
	{
		Update_ClassList();
		Update_ArenaSpawnButtons();
	}
}

HRESULT CLevel_CharacterSelect::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

#ifdef _DEBUG
	CMainApp::Update_DebugWindowTitleWithFps(
		TEXT("LostArk Character Select - Server Arena"));
	/* The debug status window would otherwise sit right where the O-key raid-
	   entry preview's left info column and panel frame want to draw -- hidden
	   while that preview is open instead of fighting it for the same space. */
	if (!Is_DebugRaidEntryPreviewOpen())
		Render_SelectionPanel();
#endif
	/* No matching Render_ClassList()/Render_ArenaSpawnButtons() -- migrated to real CUI_Sprite
	GameObjects on this Level's own "Layer_UI" (see Update_ClassList/Update_ArenaSpawnButtons,
	called from Update() instead), so CObject_Manager's normal Update/Late_Update/Render cycle
	draws them without an explicit call here. Render_ClassListText (ImGui-font text only) still
	needs the same O-key preview gate its old combined function used, since it draws over that
	same screen region. */
#ifdef _DEBUG
	if (!Is_DebugRaidEntryPreviewOpen())
		Render_ClassListText();
#else
	Render_ClassListText();
#endif
	Render_CreateCharacterProductInputHost();
	Render_ProductStatus();
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
		OutputDebugStringA(
			"[Level_CharacterSelect] Ready_Camera: Add_GameObject_to_Layer failed.\n");
		return E_FAIL;
	}
	m_pCamera = dynamic_pointer_cast<CCamera_Free>(gameObject);
	if (nullptr == m_pCamera)
	{
		OutputDebugStringA(
			"[Level_CharacterSelect] Ready_Camera: spawned object was not a CCamera_Free.\n");
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
	const CLIENT_LEVEL_DESCRIPTOR* entry =
		CLevelRegistry::Find(LEVEL::CHARACTER_SELECT);
	if (nullptr == entry || nullptr == entry->pMapAreaId)
		return E_FAIL;

	CClientReplication::DESC desc{};
	desc.pDevice = m_pDevice;
	desc.pContext = m_pContext;
	desc.iPrototypeLevelIndex = ETOUI(LEVEL::CHARACTER_SELECT);
	desc.iLayerLevelIndex = ETOUI(LEVEL::CHARACTER_SELECT);
	desc.strMapAreaId = entry->pMapAreaId;
	desc.strPlayerLayerTag = TEXT("Layer_Player");
	desc.strWorldEntityLayerTag = TEXT("Layer_WorldEntity");
	desc.bDeferLocalCharacterClassReplacement = true;
	if (!m_Replication.Initialize(desc))
	{
		OutputDebugStringA(
			"[Level_CharacterSelect] Ready_ServerGameplay: CClientReplication::Initialize failed.\n");
		return E_FAIL;
	}

	m_pPlayerCommandSink = make_shared<CNetworkPlayerCommandSink>();
	m_pWorldEntityCommandSink =
		make_shared<CNetworkWorldEntityCommandSink>();
	m_PlayerController.Set_CommandSink(m_pPlayerCommandSink);
	m_PlayerController.Set_AllowCapturedKeyboardInput(true);
	if (!m_PlayerController.Initialize_TargetingPreview(
			ETOUI(LEVEL::CHARACTER_SELECT)))
	{
		OutputDebugStringA(
			"[Level_CharacterSelect] Ready_ServerGameplay: Initialize_TargetingPreview failed.\n");
		return E_FAIL;
	}
	if (!m_PlayerController.Initialize_ClickMoveEffect(
			ETOUI(LEVEL::CHARACTER_SELECT)))
	{
		OutputDebugStringA(
			"[Level_CharacterSelect] Ready_ServerGameplay: Initialize_ClickMoveEffect failed.\n");
		return E_FAIL;
	}
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
	if (!Is_ProductPrewarmTargetActivationReady(
			Probe, bRegistrationFailureIsolated))
	{
		if (bRegistrationFailureIsolated)
		{
			m_strStatus =
				"Class Effect preparation was isolated; waiting for the "
				"current Product Effect catalog revision before presentation commit.";
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
				", background pending " +
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
		Fail_ServerArena(
			"Server disconnected before arena admission.",
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
				CLIENT_CONNECTION_LOST);
		return;
	}
	if (!m_Replication.Update())
	{
		Fail_ServerArena(
			"Arena replication failed while staging the character.",
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
				CLIENT_REPLICATION_FAILED);
		return;
	}
	if (nullptr != m_Replication.Get_LocalCharacter())
	{
		if (!Commit_ServerArena())
			Fail_ServerArena(
				"Replicated character could not bind to Server Arena.",
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
					CLIENT_REPLICATION_FAILED);
		return;
	}
	if (std::chrono::steady_clock::now() >= m_ConnectionDeadline)
		Fail_ServerArena(
			"Server arena admission timed out after 5 seconds.",
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
				CLIENT_APPROVAL_TIMEOUT);
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
	/* A Debug arena transfer publishes S2C_ENTER_ACCEPTED before the target
	world's spawn/snapshot events. Consume that authority edge first so this
	Character Select replication instance never tries to present Kakul entities
	with the Character Select map/prototype scope. */
	const SERVER_WORLD_TRANSFER_PUMP_RESULT transferResult =
		CLevelTransitionService::Pump_ServerApprovedWorldTransfer(
			LEVEL::CHARACTER_SELECT);
	if (SERVER_WORLD_TRANSFER_PUMP_RESULT::REQUESTED == transferResult)
	{
		m_preserveServerConnectionForTransfer = true;
		return;
	}
	if (SERVER_WORLD_TRANSFER_PUMP_RESULT::RECOVERY_REQUESTED == transferResult)
	{
		return;
	}

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
		Fail_ServerArena(
			"Server presentation failed.",
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
				CLIENT_REPLICATION_FAILED);
		return;
	}
	if (!Advance_DeferredClassPresentation())
	{
		Fail_ServerArena(
			"Deferred Server class presentation failed.",
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
				CLIENT_REPLICATION_FAILED);
		return;
	}
	string presentationFailure;
	if (m_Replication.Try_Consume_PresentationFailure(presentationFailure))
		m_strStatus = std::move(presentationFailure);
	if (m_Replication.Has_PendingConnectionLoss() ||
		!CNetworkManager::Get().Is_Connected())
	{
		Fail_ServerArena(
			"Server disconnected.",
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
				CLIENT_CONNECTION_LOST);
		return;
	}

	if (!Synchronize_LocalCharacter())
	{
		Fail_ServerArena(
			"The replicated local character is unavailable.",
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
				CLIENT_REPLICATION_FAILED);
		return;
	}
	if (Is_ProductPointerHovered())
		CGameInstance::Get().SetMouseButtonBlocked(DIM::LB, true);
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

void CLevel_CharacterSelect::Fail_ServerArena(
	const string& reason,
	const LostArk::Shared::SESSION_DIAGNOSTIC_REASON diagnosticReason)
{
	CLevelTransitionService::Report_Recovery(
		diagnosticReason,
		"character-select.server-arena-failure",
		reason);
	Return_ServerArenaToLobby(
		reason, "character-select.server-disconnect");
}

void CLevel_CharacterSelect::Leave_ServerArena()
{
	Return_ServerArenaToLobby(
		"Leaving Server Arena.", "character-select.back");
}

void CLevel_CharacterSelect::Return_ServerArenaToLobby(
	const string& reason,
	const char_t* pTransitionSource)
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
		pTransitionSource))
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
		CValtanCanonicalProductReadAdmission ProductAdmission;
		std::string Status;
		VALTAN_CANONICAL_READ_DIAGNOSTIC ProductDiagnostic;
		if (!ProductAdmission.Acquire(ProductDiagnostic))
		{
			Isolate_ValtanSpawnPreparationFailure(
				ProductDiagnostic.strStatus, false);
			return false;
		}
		VALTAN_PATTERN_EFFECT_CUE_DOCUMENT CueDocument;
		if (!CValtanPatternEffectCueDocument::Load_ForProductPrewarm(
				CueDocument, Status) || CueDocument.Cues.empty())
		{
			if (Status.empty())
				Status = "Valtan Product Effect cue contract has no targets.";
			Isolate_ValtanSpawnPreparationFailure(Status, false);
			return false;
		}

		const BOSS_ACTOR_ENTRY* pBossActor = CActorCatalog::Find_Boss(
			CueDocument.strOwnerArchetypeId);
		if (nullptr == pBossActor ||
			pBossActor->combatObjectVisuals.empty())
		{
			Status = nullptr == pBossActor ? CActorCatalog::Get_Status() :
				"Valtan BossCatalog has no combat-object visuals to prepare.";
			Isolate_ValtanSpawnPreparationFailure(Status, false);
			return false;
		}
		std::vector<std::string> EffectAssetIds;
		EffectAssetIds.reserve(CueDocument.Cues.size() +
			pBossActor->combatObjectVisuals.size());
#ifdef _DEBUG
		std::vector<std::string> OptionalV1EffectAssetIds;
		OptionalV1EffectAssetIds.reserve(CueDocument.Cues.size());
#endif
		for (const VALTAN_PATTERN_EFFECT_CUE& Cue : CueDocument.Cues)
		{
			EffectAssetIds.push_back(Cue.strEffectAssetId);
		#ifdef _DEBUG
			if (!Cue.strV1EffectAssetId.empty())
				OptionalV1EffectAssetIds.push_back(Cue.strV1EffectAssetId);
		#endif
		}
		for (const BOSS_COMBAT_OBJECT_VISUAL_ENTRY& Visual :
			pBossActor->combatObjectVisuals)
		{
			if (BOSS_COMBAT_OBJECT_ACTIVE_EFFECT_KIND::EFFECT_V1 ==
				Visual.activeEffectKind)
			{
				EffectAssetIds.push_back(Visual.effectAssetId);
			}
			if (!Visual.hitEffectAssetId.empty())
				EffectAssetIds.push_back(Visual.hitEffectAssetId);
		}
		if (!ProductAdmission.Validate_StillCurrent(Status))
		{
			Isolate_ValtanSpawnPreparationFailure(Status, false);
			return false;
		}
#ifdef _DEBUG
		/* The Debug V1 audition is background preparation only. The required
		   V0 enqueue below is deliberately last so it owns the priority FIFO and
		   the spawn gate observes only V0/combat-object targets. */
		if (!OptionalV1EffectAssetIds.empty())
		{
			std::vector<std::string> IgnoredV1Targets;
			std::string V1Status;
			if (!CEffectPresentationService::Queue_ProductTargets_Priority(
					OptionalV1EffectAssetIds, IgnoredV1Targets, V1Status))
			{
				OutputDebugStringA((
					"[Level_CharacterSelect] Optional Valtan Material V1 prewarm registration isolated: " +
					V1Status + "\n").c_str());
			}
		}
#endif
		if (!CEffectPresentationService::Queue_ProductTargets_Priority(
				EffectAssetIds,
				m_ValtanEffectPreparationTargets,
				Status) || m_ValtanEffectPreparationTargets.empty())
		{
			if (Status.empty())
				Status = "No unique Valtan Product Effect target was registered.";
			Isolate_ValtanSpawnPreparationFailure(Status, false);
			return false;
		}
		m_ValtanPrewarmDeadline = std::chrono::steady_clock::now() +
			CCharacterSelectArenaSpawnGate::PREWARM_TIMEOUT;
		m_strStatus = "Priority-prewarming " +
			std::to_string(m_ValtanEffectPreparationTargets.size()) +
			" unique Valtan Product Effects before the Server spawn request.";
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
			const size_t iExpectedTargetCount =
				m_ValtanEffectPreparationTargets.size();
			const bool_t bAllTargetsPrepared =
				0u != iExpectedTargetCount &&
				Probe.iTargetCount == iExpectedTargetCount &&
				Probe.iPreparedCount == iExpectedTargetCount &&
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
	m_strStatus = "Enter a 1-32 byte nickname, then confirm.";
	/* Seed the UTF-16 edit buffer from whatever UTF-8 draft survived a previous open -- the
	same persistence ImGui::InputText's member char buffer used to give for free. */
	m_NicknameDraftW.clear();
	if ('\0' != m_NicknameDraft[0])
	{
		wchar_t wide[LostArk::Shared::MAX_NICKNAME_BYTES + 1u]{};
		const int32_t iWideLength = ::MultiByteToWideChar(CP_UTF8, 0,
			m_NicknameDraft.data(), -1, wide, static_cast<int32_t>(std::size(wide)));
		if (iWideLength > 1)
			m_NicknameDraftW.assign(wide, static_cast<size_t>(iWideLength - 1));
	}
	CUIInputRouter::Get().Start_TextInput();
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
		m_strStatus = "Use 1-32 UTF-8 bytes with no control or edge whitespace.";
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
	m_strStatus = "Character creation canceled.";
	CUIInputRouter::Get().Stop_TextInput();
}

void CLevel_CharacterSelect::Render_CreateCharacterModal()
{
	if (nullptr == m_pClassSelectView)
		return;
	if (!m_isCreateCharacterModalOpen)
	{
		/* Sole owner of these 4 slots' visibility (see the class comment on
		m_isCreateCharacterModalOpen) -- Update_ClassList's own generic pass must never touch
		them, the same double-draw boundary Esther_GaugeFill/Ready glows use. */
		m_pClassSelectView->Set_SlotVisible("CreateCharacterModal_Panel", false);
		m_pClassSelectView->Set_SlotVisible("CreateCharacterModal_TextBox", false);
		m_pClassSelectView->Set_SlotVisible("CreateCharacterModal_ConfirmButton", false);
		m_pClassSelectView->Set_SlotVisible("CreateCharacterModal_CancelButton", false);
		return;
	}

	/* Panel/TextBox/Confirm/Cancel are real CUI_Sprite GameObjects (same rects, same authored
	art) rendering through the normal engine pipeline; this keeps their visibility/hover-texture
	state current. No ImGui popup remains: text entry is CUIInputRouter's WM_CHAR capture (below),
	and the nickname/composition/caret/status text draws in Render_ClassListText's LOA-font pass
	with everything else. */
	m_pClassSelectView->Set_SlotVisible("CreateCharacterModal_Panel", true);
	m_pClassSelectView->Set_SlotVisible("CreateCharacterModal_TextBox", true);

	CUIInputRouter& Router = CUIInputRouter::Get();
	/* Modal semantics: the pointer belongs to this popup every frame it's open (its dim/panel
	swallow clicks), exactly as BeginPopupModal behaved -- not only while a button is hovered. */
	Router.Claim_Mouse_This_Frame();
	const f32_t fRefWidth = m_pClassSelectView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pClassSelectView->Get_ResolutionHeight();
	bool_t confirmFromButton = false;
	bool_t cancel = false;
	struct MODAL_BUTTON { const char_t* pSlotId; bool_t* pOutClicked; };
	const MODAL_BUTTON MODAL_BUTTONS[] =
	{
		{ "CreateCharacterModal_ConfirmButton", &confirmFromButton },
		{ "CreateCharacterModal_CancelButton", &cancel },
	};
	for (const MODAL_BUTTON& Button : MODAL_BUTTONS)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pClassSelectView->Get_SlotRect(Button.pSlotId, fX, fY, fWidth, fHeight))
			continue;
		m_pClassSelectView->Set_SlotVisible(Button.pSlotId, true);
		const bool_t bHovered = Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
		m_pClassSelectView->Set_SlotTexture(Button.pSlotId, bHovered ?
			"UI/ClassSelect/Common/NormalButtonHover.png" :
			"UI/ClassSelect/Common/NormalButton.png");
		if (bHovered)
		{
			Router.Claim_Mouse_This_Frame();
			if (Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
			{
				CMainApp::Play_UIButtonClickSound();
				*Button.pOutClicked = true;
			}
		}
	}

	/* Runtime nickname editing -- CUIInputRouter's WM_CHAR capture replaces ImGui::InputText.
	Committed Hangul syllables arrive as ordinary WM_CHAR units (the IME's GCS_RESULTSTR ->
	WM_IME_CHAR -> WM_CHAR default chain), so the committed text needs no IME handling here; the
	still-composing string is drawn separately from CImGuiLayer::Get_ImeCompositionString().
	Backspace/Enter/Escape ride the same WM_CHAR stream ('\b'/'\r'/27), the classic Win32 edit
	loop. */
	const auto Fn_EraseLastCodePoint = [this]()
	{
		if (m_NicknameDraftW.empty())
			return;
		size_t iErase = 1;
		/* A supplementary-plane character is two UTF-16 units -- erase the whole pair, or the
		leftover half re-encodes as garbage. */
		if (m_NicknameDraftW.size() >= 2 &&
			m_NicknameDraftW.back() >= 0xDC00 && m_NicknameDraftW.back() <= 0xDFFF &&
			m_NicknameDraftW[m_NicknameDraftW.size() - 2] >= 0xD800 &&
			m_NicknameDraftW[m_NicknameDraftW.size() - 2] <= 0xDBFF)
		{
			iErase = 2;
		}
		m_NicknameDraftW.resize(m_NicknameDraftW.size() - iErase);
	};

	bool_t confirmFromEnter = false;
	bool_t textChanged = false;
	const wstring_t typed = Router.Take_TypedChars();
	for (const wchar_t ch : typed)
	{
		if (L'\r' == ch || L'\n' == ch)
		{
			confirmFromEnter = true;
		}
		else if (L'\x1b' == ch)
		{
			/* Escape closed the old BeginPopupModal too. */
			cancel = true;
		}
		else if (L'\b' == ch)
		{
			if (!m_NicknameDraftW.empty())
			{
				Fn_EraseLastCodePoint();
				textChanged = true;
			}
		}
		else if (ch >= L' ' && L'\x7f' != ch)
		{
			m_NicknameDraftW.push_back(ch);
			textChanged = true;
		}
	}
	if (textChanged)
	{
		/* Re-encode into the UTF-8 buffer Confirm_CreateCharacter validates/sends. If the draft
		outgrew the 32-byte wire cap, drop the newest code point(s) until it fits -- the same
		hard stop InputText's fixed byte buffer imposed at the same limit. */
		for (;;)
		{
			if (m_NicknameDraftW.empty())
			{
				m_NicknameDraft.fill('\0');
				break;
			}
			std::array<char_t, LostArk::Shared::MAX_NICKNAME_BYTES + 1u> utf8{};
			const int32_t iBytes = ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
				m_NicknameDraftW.c_str(), -1, utf8.data(),
				static_cast<int32_t>(utf8.size()), nullptr, nullptr);
			if (iBytes > 0)
			{
				m_NicknameDraft = utf8;
				break;
			}
			/* Doesn't fit (or a stray lone surrogate slipped in) -- trim and retry. */
			Fn_EraseLastCodePoint();
		}
	}

	if (cancel)
	{
		Cancel_CreateCharacter();
	}
	else if ((confirmFromEnter || confirmFromButton) &&
		Confirm_CreateCharacter())
	{
		m_isCreateCharacterModalOpen = false;
		CUIInputRouter::Get().Stop_TextInput();
	}

	/* Status text moved to Render_ClassListText's modal block -- same LOA-font pass that
	already draws the modal's title/labels, since this function no longer owns an ImGui draw
	list to put it on. */
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

#ifdef _DEBUG
bool_t CLevel_CharacterSelect::Debug_Request_KakulSaydonArena()
{
	if (MODE::SERVER_ARENA != m_eMode ||
		m_iSelectedClassIndex >= SUPPORTED_CLASSES.size() ||
		m_iPendingClassIndex.has_value() ||
		Is_ClassPresentationPreparationPending() ||
		m_isCreateCharacterModalOpen ||
		CLevelTransitionService::Is_Pending())
	{
		m_strStatus =
			"KoukuSaydon transfer requires an idle, Server-approved Character Select arena.";
		return false;
	}
	if (nullptr == m_pWorldEntityCommandSink)
	{
		m_strStatus =
			"KoukuSaydon world-transfer command owner is unavailable.";
		return false;
	}

	const std::uint32_t requestSequence =
		m_iNextKakulArenaRequestSequence++;
	if (0u == m_iNextKakulArenaRequestSequence)
		m_iNextKakulArenaRequestSequence = 1u;
	if (!m_pWorldEntityCommandSink->Request_EnterKakulSaydonArena(
		requestSequence))
	{
		m_strStatus =
			"KoukuSaydon Server arena request could not be sent.";
		return false;
	}

	m_strStatus = "KoukuSaydon Server arena transfer requested.";
	return true;
}
#endif

void CLevel_CharacterSelect::Render_CreateCharacterProductInputHost()
{
	/* Used to be an invisible ImGui window purely so OpenPopup/BeginPopupModal had a host --
	the modal is now CUI_Sprite art + CUIInputRouter WM_CHAR capture with no ImGui in it, so
	only the click-consume and per-frame modal drive remain. */
	if (m_hasCreateCharacterButtonClick)
	{
		m_hasCreateCharacterButtonClick = false;
		Open_CreateCharacterModal();
	}
	Render_CreateCharacterModal();
}

void CLevel_CharacterSelect::Render_ProductStatus()
{
	/* Release-only: in Debug the same text is already visible in the F1 selection panel.
	Drawn with the LOA font like every other product label -- ImGui is Debug-tool-only, and
	this is the one product screen element that used to draw through it in Release builds. */
#ifndef _DEBUG
	if (nullptr == m_pClassSelectView || m_isCreateCharacterModalOpen || m_strStatus.empty())
		return;

	f32_t fX = 300.f;
	f32_t fY = 24.f;
	f32_t fWidth = 600.f;
	f32_t fHeight = 54.f;
	f32_t fAuthoredX = 0.f, fAuthoredY = 0.f;
	f32_t fAuthoredWidth = 0.f, fAuthoredHeight = 0.f;
	if (m_pClassSelectView->Get_SlotRect(
		"CharacterSelect_StatusText", fAuthoredX, fAuthoredY,
		fAuthoredWidth, fAuthoredHeight) &&
		std::isfinite(fAuthoredX) && std::isfinite(fAuthoredY) &&
		std::isfinite(fAuthoredWidth) && std::isfinite(fAuthoredHeight) &&
		fAuthoredWidth > 0.f && fAuthoredHeight > 0.f)
	{
		fX = fAuthoredX;
		fY = fAuthoredY;
		fWidth = fAuthoredWidth;
		fHeight = fAuthoredHeight;
	}
	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	const f32_t fScaleX = vViewportSize.x / 1280.f;
	const f32_t fScaleY = vViewportSize.y / 720.f;
	const f32_t fUiScale = (std::min)(fScaleX, fScaleY);
	/* The status strings are ASCII, so the byte-wise widen is exact. */
	const wstring_t strStatusWide(m_strStatus.begin(), m_strStatus.end());
	const float2_t vMeasured =
		CGameInstance::Get().Measure_Text(TEXT("Font_YG330"), strStatusWide.c_str());
	if (vMeasured.y <= 0.f)
		return;
	f32_t fScale = (16.f / vMeasured.y) * fUiScale;
	/* Draw_Text has no wrapping, so an over-long line shrinks to fit its authored width
	instead of running past it the way the old AddText wrap would have folded it. */
	const f32_t fMaxWidth = (fWidth - 16.f) * fScaleX;
	if (vMeasured.x * fScale > fMaxWidth && vMeasured.x > 0.f)
		fScale = fMaxWidth / vMeasured.x;
	const float2_t vPos((fX + 8.f) * fScaleX, (fY + 6.f) * fScaleY);
	CGameInstance::Get().Draw_Text(TEXT("Font_YG330"), strStatusWide.c_str(),
		float2_t(vPos.x + 1.f, vPos.y + 1.f),
		XMVectorSet(0.f, 0.f, 0.f, 220.f / 255.f), 0.f, float2_t(0.f, 0.f), fScale);
	CGameInstance::Get().Draw_Text(TEXT("Font_YG330"), strStatusWide.c_str(),
		vPos,
		XMVectorSet(1.f, 225.f / 255.f, 150.f / 255.f, 1.f), 0.f,
		float2_t(0.f, 0.f), fScale);
#endif
}

#ifdef _DEBUG
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
			ImGui::TextDisabled("Prewarming %zu unique boss Effects",
				m_ValtanEffectPreparationTargets.size());
		}
		else if (m_pArenaSpawnGate->Can_Retry())
			ImGui::SameLine(), ImGui::TextDisabled("Retry available");
#ifdef _DEBUG
		ImGui::TextDisabled(
			"Combat geometry: F1 > Diagnostics > Live Combat Geometry");
#endif
	}

	ImGui::Separator();
	ImGui::BeginDisabled(isConnecting || isReturning || transitionPending ||
		m_isCreateCharacterModalOpen ||
		m_iPendingClassIndex.has_value() ||
		Is_ClassPresentationPreparationPending());
	if (ImGui::Button("Create Character"))
		Request_CreateCharacterButtonClick();
	ImGui::SameLine();
	if (ImGui::Button("Enter Valtan Map"))
		Enter_Stage(LOBBY_STAGE::VALTAN);
	ImGui::SameLine();
	if (ImGui::Button("Enter KoukuSaydon Arena"))
		(void)Debug_Request_KakulSaydonArena();
	ImGui::SameLine();
	if (ImGui::Button("Back"))
		Leave_ServerArena();
	ImGui::EndDisabled();

	ImGui::TextDisabled(
		"F1: tools  |  F6: follow/free  |  Server-authorized skill input enabled");
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::End();
}
#endif

namespace
{
	/* Same fX/fWidth/fHeight ClassList_RowN's own JSON rect was authored with (see
	Update_ClassList) -- Update_ClassList already owns writing that rect back via
	Set_SlotPosition every frame (fY moves with the accordion), so this array stays the single
	source of the constant values instead of a round trip that would just read back what this
	same array already wrote. */
	struct CLASS_LIST_ENTRY
	{
		f32_t fX, fWidth, fHeight;
		size_t iSupportedClassIndex;
		/* "Warlord" etc -- matches ClassSelect_Layout.json's "classes" array and each per-class
		slot's ownerClass, not Get_CharacterClassName()'s display text ("Dimension Master" has a
		space an ownerClass match would never see). */
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
	actual y is only known at update time -- see Update_ClassList's running fRowY. fX/fWidth/
	fHeight still aren't read from the JSON's own ClassList_RowN slots even though those exist
	now (added alongside this migration) -- Update_ClassList already owns writing that rect via
	Set_SlotPosition every frame, so reading it back from the same slot would be a pointless
	round trip through the exact values this array already holds. */
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

	struct CLASS_OWNED_SLOT final { const char* pSlotId; const char* pOwnerClass; };

	/* ClassSelect_Layout.json's own ownerClass-tagged right panel slots -- CHUDRuntimeView's
	Render(strSelectedClass, revision) used to filter these automatically (show if ownerClass ==
	strSelectedClass); CUILayoutRuntime has no such pass, so Update_ClassList applies the same
	filter by hand against this mirror of the JSON's own ownerClass field. Slayer/Gunslinger
	have no slots here at all (never authored) -- selecting either one simply shows none of this,
	same gap as before this migration. Tag_5 only exists for Warlord/Artist (5 tags); LanceMaster/
	DimensionMaster stop at Tag_4. */
	constexpr CLASS_OWNED_SLOT CLASS_OWNED_SLOTS[] = {
		{ "Warlord_DifficultyFill", "Warlord" },
		{ "LanceMaster_DifficultyFill", "LanceMaster" },
		{ "Artist_DifficultyFill", "Artist" },
		{ "DimensionMaster_DifficultyFill", "DimensionMaster" },
		{ "Warlord_Tag_1", "Warlord" }, { "Warlord_Tag_2", "Warlord" },
		{ "Warlord_Tag_3", "Warlord" }, { "Warlord_Tag_4", "Warlord" }, { "Warlord_Tag_5", "Warlord" },
		{ "LanceMaster_Tag_1", "LanceMaster" }, { "LanceMaster_Tag_2", "LanceMaster" },
		{ "LanceMaster_Tag_3", "LanceMaster" }, { "LanceMaster_Tag_4", "LanceMaster" },
		{ "Artist_Tag_1", "Artist" }, { "Artist_Tag_2", "Artist" }, { "Artist_Tag_3", "Artist" },
		{ "Artist_Tag_4", "Artist" }, { "Artist_Tag_5", "Artist" },
		{ "DimensionMaster_Tag_1", "DimensionMaster" }, { "DimensionMaster_Tag_2", "DimensionMaster" },
		{ "DimensionMaster_Tag_3", "DimensionMaster" }, { "DimensionMaster_Tag_4", "DimensionMaster" },
		{ "Warlord_Illustration", "Warlord" }, { "Warlord_NameSymbol", "Warlord" },
		{ "Warlord_Description", "Warlord" }, { "Warlord_IdentityID", "Warlord" },
		{ "Warlord_IdentityDescription", "Warlord" },
		{ "LanceMaster_Illustration", "LanceMaster" }, { "LanceMaster_NameSymbol", "LanceMaster" },
		{ "LanceMaster_Description", "LanceMaster" }, { "LanceMaster_IdentityID", "LanceMaster" },
		{ "LanceMaster_IdentityDescription", "LanceMaster" },
		{ "Artist_Illustration", "Artist" }, { "Artist_NameSymbol", "Artist" },
		{ "Artist_Description", "Artist" }, { "Artist_IdentityID", "Artist" },
		{ "Artist_IdentityDescription", "Artist" },
		{ "DimensionMaster_Illustration", "DimensionMaster" }, { "DimensionMaster_NameSymbol", "DimensionMaster" },
		{ "DimensionMaster_Description", "DimensionMaster" }, { "DimensionMaster_IdentityID", "DimensionMaster" },
		{ "DimensionMaster_IdentityDescription", "DimensionMaster" },
	};

	/* ownerClass == null chrome that used to show via the same generic Render(strSelectedClass,
	revision) pass -- always visible while SERVER_ARENA regardless of selected class. */
	constexpr const char* ALWAYS_VISIBLE_CLASS_LIST_CHROME[] = {
		"PanelBgLeft", "PanelBgRight", "PanelBgRightBottom", "Frame",
		"DifficultyBg", "TagsBg", "New_Slot", "InfoSeparateBar", "IdentitySeparateBar",
	};

	constexpr f32_t ROW_Y_START = 60.f;
	constexpr f32_t ROW_GAP = 7.f;
	constexpr f32_t THUMB_W = 134.f;
	constexpr f32_t THUMB_H = 78.f;
	constexpr f32_t THUMB_MARGIN_TOP = 10.f;
	constexpr f32_t THUMB_MARGIN_BOTTOM = 10.f;

	struct CHARACTER_SELECT_PRODUCT_SLOT final
	{
		const char_t* pSlotId;
		const char_t* pIdlePath;
		const char_t* pHoverPath;
		f32_t fX;
		f32_t fY;
		f32_t fWidth;
		f32_t fHeight;
	};

	constexpr std::array<CHARACTER_SELECT_PRODUCT_SLOT, 5> PRODUCT_BUTTON_SLOTS =
	{{
		{ "SpawnMonsterButton", "UI/ClassSelect/Common/SpawnMonsterButton.png",
			"UI/ClassSelect/Common/SpawnMonsterButtonHover.png",
			1036.84436f, 624.437317f, 70.f, 69.f },
		{ "BossSpawnButton", "UI/ClassSelect/Common/BossSpawnButton.png",
			"UI/ClassSelect/Common/BossSpawnButtonHover.png",
			1099.70178f, 624.437317f, 70.f, 69.f },
		{ "SpawnCancelButton", "UI/ClassSelect/Common/SpawnCancelButton.png",
			"UI/ClassSelect/Common/SpawnCancelButtonHover.png",
			1165.41553f, 624.437317f, 70.f, 69.f },
		{ "CreateCharacterButton", "UI/ClassSelect/Common/CreateCharacterButton.png",
			"UI/ClassSelect/Common/CreateCharacterButtonHover.png",
			1105.71472f, 565.714478f, 140.f, 47.f },
		{ "GoBackIcon", "UI/ClassSelect/Common/GoBackIcon.png",
			"UI/ClassSelect/Common/GoBackIconHover.png",
			44.7142868f, 674.648804f, 40.f, 39.f },
	}};

	bool_t Is_ValidProductSlotRect(
		const f32_t fX, const f32_t fY, const f32_t fWidth, const f32_t fHeight)
	{
		return std::isfinite(fX) && std::isfinite(fY) &&
			std::isfinite(fWidth) && std::isfinite(fHeight) &&
			fWidth > 0.f && fHeight > 0.f;
	}

	const CHARACTER_SELECT_PRODUCT_SLOT* Find_ProductButtonSlot(const char_t* pSlotId)
	{
		for (const CHARACTER_SELECT_PRODUCT_SLOT& Slot : PRODUCT_BUTTON_SLOTS)
		{
			if (0 == std::strcmp(Slot.pSlotId, pSlotId))
				return &Slot;
		}
		return nullptr;
	}

	bool_t Has_CompleteProductButtonSlots(CUILayoutRuntime* pView)
	{
		if (nullptr == pView)
			return false;
		for (const CHARACTER_SELECT_PRODUCT_SLOT& Slot : PRODUCT_BUTTON_SLOTS)
		{
			f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
			if (!pView->Get_SlotRect(Slot.pSlotId, fX, fY, fWidth, fHeight) ||
				!Is_ValidProductSlotRect(fX, fY, fWidth, fHeight))
			{
				return false;
			}
		}
		return true;
	}

	void Resolve_ProductButtonRect(
		CUILayoutRuntime* pView,
		const CHARACTER_SELECT_PRODUCT_SLOT& Slot,
		const bool_t hasCompleteAuthoredSlots,
		f32_t& outX, f32_t& outY, f32_t& outWidth, f32_t& outHeight)
	{
		if (hasCompleteAuthoredSlots && nullptr != pView &&
			pView->Get_SlotRect(Slot.pSlotId, outX, outY, outWidth, outHeight) &&
			Is_ValidProductSlotRect(outX, outY, outWidth, outHeight))
		{
			return;
		}
		outX = Slot.fX;
		outY = Slot.fY;
		outWidth = Slot.fWidth;
		outHeight = Slot.fHeight;
	}

	string Build_ClassSelectAssetPath(const char* pClassName, const char* pFileName)
	{
		return string("UI/ClassSelect/") + pClassName + "/" + pFileName;
	}
}

bool_t CLevel_CharacterSelect::Is_ProductPointerHovered() const
{
	if (MODE::SERVER_ARENA != m_eMode || nullptr == m_pClassSelectView)
		return false;

	/* Same router every other product widget on this screen hit-tests through -- this is
	gameplay-path code (it gates whether an LMB also reaches PlayerController), so it reads the
	native cursor, not ImGui's. */
	const auto IsHovered = [](const f32_t fX, const f32_t fY,
		const f32_t fWidth, const f32_t fHeight)
	{
		return CUIInputRouter::Get().Is_Hovered(
			fX, fY, fWidth, fHeight, REF_WIDTH, REF_HEIGHT);
	};

	const bool_t hasCompleteAuthoredButtons =
		Has_CompleteProductButtonSlots(m_pClassSelectView.get());
	for (const CHARACTER_SELECT_PRODUCT_SLOT& Slot : PRODUCT_BUTTON_SLOTS)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		Resolve_ProductButtonRect(m_pClassSelectView.get(), Slot,
			hasCompleteAuthoredButtons, fX, fY, fWidth, fHeight);
		if (IsHovered(fX, fY, fWidth, fHeight))
			return true;
	}

	/* Class rows and their expanded thumbnail occupy this stable authored right-side panel.
	Blocking the panel before PlayerController::Update keeps a class-selection LMB from also
	becoming a Server basic-attack command in the same frame. */
	return IsHovered(940.f, 48.f, 320.f, 540.f);
}

namespace
{
	string Get_SelectedJsonClassName(const size_t iSelectedClassIndex)
	{
		for (const CLASS_LIST_ENTRY& Entry : CLASS_LIST_ENTRIES)
			if (Entry.iSupportedClassIndex == iSelectedClassIndex)
				return Entry.pJsonClassName;
		return {};
	}
}

void CLevel_CharacterSelect::Hide_ClassList()
{
	if (nullptr == m_pClassSelectView)
		return;

	for (const char* pId : ALWAYS_VISIBLE_CLASS_LIST_CHROME)
		m_pClassSelectView->Set_SlotVisible(pId, false);
	for (const CLASS_OWNED_SLOT& Slot : CLASS_OWNED_SLOTS)
		m_pClassSelectView->Set_SlotVisible(Slot.pSlotId, false);
	for (int32_t i = 0; i < static_cast<int32_t>(std::size(CLASS_LIST_ENTRIES)); ++i)
	{
		m_pClassSelectView->Set_SlotVisible("ClassList_Row" + std::to_string(i), false);
		m_pClassSelectView->Set_SlotVisible("ClassList_Symbol" + std::to_string(i), false);
	}
	m_pClassSelectView->Set_SlotVisible("ClassList_Thumb", false);
	m_pClassSelectView->Set_SlotVisible("ClassList_ThumbSymbol", false);
	m_pClassSelectView->Set_SlotVisible("ClassList_ThumbFrame", false);
}

void CLevel_CharacterSelect::Update_ClassList()
{
	/* MODE::SERVER_ARENA is the real "scene has finished transitioning" signal --
	Commit_ServerArena only sets it once the Server-replicated local character has
	actually spawned and bound to the camera. Showing this panel any earlier (still
	MODE::CONNECTING, still waiting on that network round trip) put the fully-formed
	side panels on screen while the arena itself had not finished settling, which read
	as the UI arriving before the scene transition had actually completed. */
	if (nullptr == m_pClassSelectView || MODE::SERVER_ARENA != m_eMode)
	{
		Hide_ClassList();
		return;
	}

	const string strSelectedClass = m_iSelectedClassIndex < SUPPORTED_CLASSES.size()
		? Get_SelectedJsonClassName(m_iSelectedClassIndex)
		: string{};

	for (const char* pId : ALWAYS_VISIBLE_CLASS_LIST_CHROME)
		m_pClassSelectView->Set_SlotVisible(pId, true);
	/* ownerClass filter CHUDRuntimeView's own Render(strSelectedClass, revision) used to apply
	automatically -- CUILayoutRuntime has no such pass, so this shows only the selected class's
	own right-panel slots and hides every other class's. */
	for (const CLASS_OWNED_SLOT& Slot : CLASS_OWNED_SLOTS)
		m_pClassSelectView->Set_SlotVisible(Slot.pSlotId, strSelectedClass == Slot.pOwnerClass);

	const bool_t hasCompleteProductButtons =
		Has_CompleteProductButtonSlots(m_pClassSelectView.get());
	for (const CHARACTER_SELECT_PRODUCT_SLOT& Slot : PRODUCT_BUTTON_SLOTS)
		m_pClassSelectView->Set_SlotVisible(Slot.pSlotId, hasCompleteProductButtons);

	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pClassSelectView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pClassSelectView->Get_ResolutionHeight();
	const bool_t bInteractable = MODE::SERVER_ARENA == m_eMode &&
		!m_isCreateCharacterModalOpen &&
		!m_iPendingClassIndex.has_value() &&
		!Is_ClassPresentationPreparationPending() &&
		!CLevelTransitionService::Is_Pending();

	/* Accordion: fRowY advances past each row, and past the expanded row's thumbnail block too,
	so a category expanding pushes every row beneath it down instead of the thumbnail always
	drawing in one fixed spot regardless of which category opened it. Render_ClassListText below
	recomputes this exact same fRowY sequence for its own text-only pass -- both are pure
	functions of m_iExpandedCategory/CLASS_LIST_ENTRIES, so recomputing instead of caching stays
	safe as long as nothing else touches m_iExpandedCategory between here and Render(). */
	/* Default hidden -- only the loop's own bExpanded branch below shows these, and unlike the
	old ImGui pass (which simply drew nothing when no category was expanded), a CUI_Sprite would
	otherwise keep showing whichever class was expanded last even after every row collapses. */
	m_pClassSelectView->Set_SlotVisible("ClassList_Thumb", false);
	m_pClassSelectView->Set_SlotVisible("ClassList_ThumbSymbol", false);
	m_pClassSelectView->Set_SlotVisible("ClassList_ThumbFrame", false);

	const int32_t iExpandedBefore = m_iExpandedCategory;
	f32_t fRowY = ROW_Y_START;
	for (int32_t i = 0; i < static_cast<int32_t>(std::size(CLASS_LIST_ENTRIES)); ++i)
	{
		const CLASS_LIST_ENTRY& Entry = CLASS_LIST_ENTRIES[i];
		const string strRowId = "ClassList_Row" + std::to_string(i);
		const string strSymbolId = "ClassList_Symbol" + std::to_string(i);
		const bool_t bExpanded = i == iExpandedBefore;

		m_pClassSelectView->Set_SlotVisible(strRowId, true);
		m_pClassSelectView->Set_SlotPosition(strRowId, Entry.fX, fRowY);
		m_pClassSelectView->Set_SlotTexture(strRowId, bExpanded ?
			"UI/ClassSelect/Common/CategorySelected.png" : "UI/ClassSelect/Common/Category.png");

		const bool_t bHovered = bInteractable &&
			Router.Is_Hovered(Entry.fX, fRowY, Entry.fWidth, Entry.fHeight, fRefWidth, fRefHeight);
		/* Same hover callout as the old AddRect(...,160,...) outline -- a brighten tint instead
		of a border image CUILayoutRuntime has no primitive for. Only while collapsed: the
		expanded row already reads as selected via CategorySelected.png. */
		m_pClassSelectView->Set_SlotTint(strRowId,
			(!bExpanded && bHovered) ? float4_t(1.15f, 1.15f, 1.05f, 1.f) : float4_t(1.f, 1.f, 1.f, 1.f));

		if (nullptr != Entry.pCategorySymbolFile)
		{
			m_pClassSelectView->Set_SlotVisible(strSymbolId, true);
			m_pClassSelectView->Set_SlotPosition(strSymbolId, Entry.fX + 8.f, fRowY + 6.f);
			m_pClassSelectView->Set_SlotTexture(strSymbolId,
				string("UI/ClassSelect/Common/") + Entry.pCategorySymbolFile);
		}
		else
		{
			m_pClassSelectView->Set_SlotVisible(strSymbolId, false);
		}

		if (bHovered)
		{
			Router.Claim_Mouse_This_Frame();
			if (Router.Is_Clicked(Entry.fX, fRowY, Entry.fWidth, Entry.fHeight, fRefWidth, fRefHeight))
			{
				CMainApp::Play_UIButtonClickSound();
				m_iExpandedCategory = bExpanded ? -1 : i;
			}
		}

		fRowY += Entry.fHeight + ROW_GAP;

		if (bExpanded)
		{
			const bool_t bConfirmed = Entry.iSupportedClassIndex == m_iSelectedClassIndex;
			const f32_t fThumbY = fRowY + THUMB_MARGIN_TOP;

			m_pClassSelectView->Set_SlotVisible("ClassList_Thumb", true);
			m_pClassSelectView->Set_SlotPosition("ClassList_Thumb", Entry.fX, fThumbY);
			/* Set_SlotTexture(missingPath) resolves to a null SRV and reverts the sprite to its
			own authored default (CategorySelected.png, same fallback as the old
			Load_Texture-returned-null branch) -- Slayer/Gunslinger have no IllustrationSmall.png
			today, same gap as before this migration. */
			m_pClassSelectView->Set_SlotTexture("ClassList_Thumb",
				Build_ClassSelectAssetPath(Entry.pJsonClassName, "IllustrationSmall.png"));

			/* No missing-asset fallback for the identity symbol overlay (unlike Thumb just
			above) -- a failed load here reverts to ClassList_ThumbSymbol's own authored
			placeholder texture instead of hiding, a minor cosmetic gap versus the old
			Load_Texture-returns-null-so-skip-drawing behavior, only reachable for a class
			missing this one asset. */
			m_pClassSelectView->Set_SlotVisible("ClassList_ThumbSymbol", true);
			m_pClassSelectView->Set_SlotPosition(
				"ClassList_ThumbSymbol", Entry.fX + THUMB_W - 22.f, fThumbY - 4.f);
			m_pClassSelectView->Set_SlotTexture("ClassList_ThumbSymbol",
				Build_ClassSelectAssetPath(Entry.pJsonClassName, "IdentitySymbol.png"));

			const bool_t bThumbHovered = bInteractable &&
				Router.Is_Hovered(Entry.fX, fThumbY, THUMB_W, THUMB_H, fRefWidth, fRefHeight);

			/* Small illust selected.png is the one hover/confirm frame -- both states use the
			same authored art instead of a placeholder AddRect() outline. */
			m_pClassSelectView->Set_SlotVisible("ClassList_ThumbFrame", bConfirmed || bThumbHovered);
			m_pClassSelectView->Set_SlotPosition(
				"ClassList_ThumbFrame", Entry.fX - 2.f, fThumbY - 2.f);

			if (bThumbHovered)
			{
				Router.Claim_Mouse_This_Frame();
				if (Router.Is_Clicked(Entry.fX, fThumbY, THUMB_W, THUMB_H, fRefWidth, fRefHeight))
				{
					CMainApp::Play_UIButtonClickSound();
					Request_ClassChange(Entry.iSupportedClassIndex);
				}
			}

			fRowY = fThumbY + THUMB_H + THUMB_MARGIN_BOTTOM;
		}
	}
}

void CLevel_CharacterSelect::Render_ClassListText()
{
	/* Same MODE::SERVER_ARENA gate as Update_ClassList -- these are that panel's own text, so
	they must disappear and reappear together with it instead of floating on screen without the
	art underneath. Draws with the LOA font (CGameInstance::Draw_Text) like every other product
	label; ImGui is Debug-tool-only. */
	if (nullptr == m_pClassSelectView || MODE::SERVER_ARENA != m_eMode)
		return;

	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	const f32_t fScaleX = vViewportSize.x / REF_WIDTH;
	const f32_t fScaleY = vViewportSize.y / REF_HEIGHT;
	const f32_t fUiScale = (std::min)(fScaleX, fScaleY);

	/* The labels are UTF-8 byte literals (this file has no BOM and the project builds without
	/utf-8), and Draw_Text takes wide text. */
	const auto Fn_Widen = [](const char* pUtf8) -> wstring_t
	{
		if (nullptr == pUtf8 || '\0' == pUtf8[0])
			return {};
		const int32_t iLength = ::MultiByteToWideChar(CP_UTF8, 0, pUtf8, -1, nullptr, 0);
		if (iLength <= 1)
			return {};
		wstring_t wide(static_cast<size_t>(iLength - 1), L'\0');
		::MultiByteToWideChar(CP_UTF8, 0, pUtf8, -1, wide.data(), iLength);
		return wide;
	};

	/* fSize is the reference-resolution pixel height -- Draw_Text scales relative to the font's
	own measured height instead of taking a size directly. Anchored top-left (0,0 pivot) unless
	a pivot is given. One draw per label with the real weight the retail classselect.gfx uses
	(section headers $YoonGasiIIM, identity blurb $YG760) -- the earlier four-offset faux-bold
	pass on YG330 read as smeared/fat. bShadow adds the 1px dark drop the retail
	LabelEx_*_shadow labels carry. */
	const auto Fn_DrawPanelTextAt = [&](const wstring& strFont, f32_t fX, f32_t fY,
		f32_t fSize, const fvector_t& vColor, const char* pUtf8, const float2_t& vPivot,
		bool_t bShadow)
	{
		const wstring_t wide = Fn_Widen(pUtf8);
		if (wide.empty())
			return;
		const float2_t vMeasured =
			CGameInstance::Get().Measure_Text(strFont, wide.c_str());
		if (vMeasured.y <= 0.f)
			return;
		const f32_t fScale = (fSize / vMeasured.y) * fUiScale;
		const float2_t vPos(fX * fScaleX, fY * fScaleY);
		if (bShadow)
		{
			CGameInstance::Get().Draw_Text(strFont, wide.c_str(),
				float2_t(vPos.x + 1.f, vPos.y + 1.f),
				XMVectorSet(0.f, 0.f, 0.f, 0.8f), 0.f, vPivot, fScale);
		}
		CGameInstance::Get().Draw_Text(strFont, wide.c_str(), vPos, vColor, 0.f, vPivot, fScale);
	};

	const auto Fn_DrawPanelText = [&](const wstring& strFont, f32_t fX, f32_t fY, f32_t fSize,
		const fvector_t& vColor, const char* pUtf8)
	{
		Fn_DrawPanelTextAt(strFont, fX, fY, fSize, vColor, pUtf8, float2_t(0.f, 0.f), false);
	};

	/* Centers pText within [fRectX, fRectX + fRectWidth) at reference scale -- a 0.5 x-pivot
	does what the old explicit CalcTextSizeA half-width offset did. */
	const auto Fn_DrawPanelTextCentered = [&](const wstring& strFont, f32_t fRectX,
		f32_t fRectWidth, f32_t fY, f32_t fSize, const fvector_t& vColor, const char* pUtf8)
	{
		Fn_DrawPanelTextAt(strFont, fRectX + fRectWidth * 0.5f, fY, fSize, vColor, pUtf8,
			float2_t(0.5f, 0.f), false);
	};

	/* Class list row / thumbnail labels: retail ClassSelectClassButtonTop/Bottom textField is
	$YoonGasiIIM 16px at 1080p -> 10.667px in 1280 reference units, single draw. */
	const auto Fn_DrawText = [&](f32_t fX, f32_t fY, const fvector_t& vColor, const char* pUtf8)
	{
		Fn_DrawPanelText(TEXT("Font_YoonGasiIIM"), fX, fY, 16.f, vColor, pUtf8);
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
		/* Same IM_COL32 values the ImGui draws used, as normalized RGBA. */
		const fvector_t vSectionLabelColor =
			XMVectorSet(1.f, 220.f / 255.f, 140.f / 255.f, 1.f);
		Fn_DrawPanelText(TEXT("Font_YoonGasiIIM"), 72.f, 203.f, 32.f, Colors::White,
			Entry.pClassLabel);
		Fn_DrawPanelText(TEXT("Font_YoonGasiIIM"), 15.f, 262.f, 20.f, vSectionLabelColor,
			"\xec\xa1\xb0\xec\x9e\x91 \xeb\x82\x9c\xec\x9d\xb4\xeb\x8f\x84");
		Fn_DrawPanelText(TEXT("Font_YoonGasiIIM"), 15.f, 335.f, 20.f, vSectionLabelColor,
			"\xea\xb8\xb0\xeb\xb3\xb8 \xec\xa0\x95\xeb\xb3\xb4");
		Fn_DrawPanelText(TEXT("Font_YoonGasiIIM"), 15.f, 453.f, 20.f, vSectionLabelColor,
			"\xec\x95\x84\xec\x9d\xb4\xeb\x8d\xb4\xed\x8b\xb0\xed\x8b\xb0");

		for (const IDENTITY_DESCRIPTION& Desc : IDENTITY_DESCRIPTIONS)
		{
			if (0 != strcmp(Desc.pJsonClassName, Entry.pJsonClassName))
				continue;

			for (int32_t iLine = 0; iLine < Desc.iLineCount; ++iLine)
			{
				Fn_DrawPanelTextCentered(TEXT("Font_YG760"), IDENTITY_DESC_X, IDENTITY_DESC_WIDTH,
					IDENTITY_DESC_Y + static_cast<f32_t>(iLine) * IDENTITY_DESC_LINE_HEIGHT,
					16.f,
					XMVectorSet(220.f / 255.f, 220.f / 255.f, 220.f / 255.f, 1.f),
					Desc.ppLines[iLine]);
			}
			break;
		}
		break;
	}

	/* "클래스 선택" -- retail classselect.gfx puts it as a LabelEx_YGasiIIM_shadow label
	scaled 2.7x (about 32px at 1080p, 22px here) centered over the frame art's title band;
	centered on the authored Frame slot so it stays aligned if the tool moves the frame. */
	{
		f32_t fFrameX = 940.f, fFrameY = -8.57f, fFrameW = 381.f, fFrameH = 67.f;
		(void)m_pClassSelectView->Get_SlotRect("Frame", fFrameX, fFrameY, fFrameW, fFrameH);
		Fn_DrawPanelTextAt(TEXT("Font_YoonGasiIIM"), fFrameX + fFrameW * 0.5f, fFrameY + 36.f,
			24.f, XMVectorSet(1.f, 220.f / 255.f, 140.f / 255.f, 1.f),
			"\xed\x81\xb4\xeb\x9e\x98\xec\x8a\xa4 \xec\x84\xa0\xed\x83\x9d",
			float2_t(0.5f, 0.5f), true);
	}

	/* Same fRowY recompute as Update_ClassList's own accordion loop, purely for these two text
	draws -- see that function's comment for why recomputing instead of caching stays safe. */
	const int32_t iExpandedBefore = m_iExpandedCategory;
	f32_t fRowY = ROW_Y_START;
	for (int32_t i = 0; i < static_cast<int32_t>(std::size(CLASS_LIST_ENTRIES)); ++i)
	{
		const CLASS_LIST_ENTRY& Entry = CLASS_LIST_ENTRIES[i];
		const bool_t bExpanded = i == iExpandedBefore;

		Fn_DrawText(Entry.fX + 52.f, fRowY + 16.f,
			XMVectorSet(230.f / 255.f, 230.f / 255.f, 230.f / 255.f, 1.f),
			Entry.pCategoryLabel);

		fRowY += Entry.fHeight + ROW_GAP;

		if (bExpanded)
		{
			const f32_t fThumbY = fRowY + THUMB_MARGIN_TOP;
			Fn_DrawText(Entry.fX + 4.f, fThumbY + THUMB_H - 18.f,
				Colors::White, Entry.pClassLabel);
			fRowY = fThumbY + THUMB_H + THUMB_MARGIN_BOTTOM;
		}
	}
}

void CLevel_CharacterSelect::Hide_ArenaSpawnButtons()
{
	if (nullptr == m_pClassSelectView)
		return;
	for (const CHARACTER_SELECT_PRODUCT_SLOT& Slot : PRODUCT_BUTTON_SLOTS)
		m_pClassSelectView->Set_SlotVisible(Slot.pSlotId, false);
}

void CLevel_CharacterSelect::Update_ArenaSpawnButtons()
{
	/* Same MODE::SERVER_ARENA gate as Update_ClassList -- these debug spawn buttons and the
	Create Character button are gameplay-tied and meaningless (their click handlers all touch
	m_pWorldEntityCommandSink/m_pPlayerCommandSink, which are only useful once the arena has
	actually admitted the local character) before the scene has really finished transitioning. */
	if (nullptr == m_pClassSelectView || MODE::SERVER_ARENA != m_eMode)
	{
		Hide_ArenaSpawnButtons();
		return;
	}

	const bool_t hasCompleteAuthoredButtons =
		Has_CompleteProductButtonSlots(m_pClassSelectView.get());
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pClassSelectView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pClassSelectView->Get_ResolutionHeight();
	const bool_t bInteractable = MODE::SERVER_ARENA == m_eMode &&
		!m_isCreateCharacterModalOpen &&
		!Is_ClassPresentationPreparationPending() &&
		!CLevelTransitionService::Is_Pending();

	/* The authored spawn icon rects overlap by a few pixels. Resolve the visually topmost slot
	first (later draw order wins) so one physical click can submit exactly one typed command. */
	const char_t* pHoveredSpawnSlotId = nullptr;
	constexpr const char_t* SPAWN_HIT_ORDER[] =
	{
		"SpawnCancelButton", "BossSpawnButton", "SpawnMonsterButton"
	};
	if (bInteractable)
	{
		for (const char_t* pSlotId : SPAWN_HIT_ORDER)
		{
			const CHARACTER_SELECT_PRODUCT_SLOT* pSlot = Find_ProductButtonSlot(pSlotId);
			if (nullptr == pSlot)
				continue;
			f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
			Resolve_ProductButtonRect(m_pClassSelectView.get(), *pSlot,
				hasCompleteAuthoredButtons, fX, fY, fWidth, fHeight);
			if (Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
			{
				pHoveredSpawnSlotId = pSlotId;
				break;
			}
		}
	}

	/* ARENA_SPAWN_OPTIONS[1] (Mid Boss / Lugaru) has no button in this row -- only reachable
	through the ImGui debug radio list -- so the middle button maps to option index 2 (Valtan),
	not the array's own middle entry. */
	struct SPAWN_BUTTON final { const char_t* pSlotId; size_t iOptionIndex; };
	constexpr SPAWN_BUTTON SPAWN_BUTTONS[] =
	{
		{ "SpawnMonsterButton", 0u },
		{ "BossSpawnButton", 2u },
	};

	for (const SPAWN_BUTTON& Button : SPAWN_BUTTONS)
	{
		const CHARACTER_SELECT_PRODUCT_SLOT* pSlot =
			Find_ProductButtonSlot(Button.pSlotId);
		if (nullptr == pSlot)
			continue;
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		Resolve_ProductButtonRect(m_pClassSelectView.get(), *pSlot,
			hasCompleteAuthoredButtons, fX, fY, fWidth, fHeight);

		const bool_t bHovered = nullptr != pHoveredSpawnSlotId &&
			0 == std::strcmp(pHoveredSpawnSlotId, Button.pSlotId);

		/* Visibility is Update_ClassList's own concern (hasCompleteProductButtons, called just
		before this from Update()) -- this only owns which texture shows. Empty path reverts the
		sprite to its own authored default, which is this same pIdlePath art, so only the hover
		swap needs an explicit override. */
		m_pClassSelectView->Set_SlotTexture(Button.pSlotId, bHovered ? pSlot->pHoverPath : "");

		if (bHovered)
		{
			Router.Claim_Mouse_This_Frame();
			if (Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
			{
				CMainApp::Play_UIButtonClickSound();
				m_iSelectedArenaSpawnIndex = Button.iOptionIndex;
				Request_SelectedArenaSpawn();
			}
		}
	}

	/* SpawnCancelButton (rightmost, "되돌리기"): despawns every world entity the two buttons to
	its left created in this room -- C2S_DESPAWN_ALL_WORLD_ENTITIES, mirroring how the other two
	buttons already call Request_SelectedArenaSpawn/IWorldEntityCommandSink. */
	{
		const CHARACTER_SELECT_PRODUCT_SLOT* pSlot =
			Find_ProductButtonSlot("SpawnCancelButton");
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (nullptr != pSlot)
		{
			Resolve_ProductButtonRect(m_pClassSelectView.get(), *pSlot,
				hasCompleteAuthoredButtons, fX, fY, fWidth, fHeight);
			const bool_t bHovered = nullptr != pHoveredSpawnSlotId &&
				0 == std::strcmp(pHoveredSpawnSlotId, pSlot->pSlotId);
			m_pClassSelectView->Set_SlotTexture(pSlot->pSlotId, bHovered ? pSlot->pHoverPath : "");
			if (bHovered)
			{
				Router.Claim_Mouse_This_Frame();
				if (Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight) &&
					nullptr != m_pWorldEntityCommandSink)
				{
					CMainApp::Play_UIButtonClickSound();
					m_pWorldEntityCommandSink->Request_DespawnAllWorldEntities(
						m_iNextDespawnRequestSequence++);
					/* Request_SelectedArenaSpawn refuses to resend once
					m_ArenaSpawnAccepted[index] is true (see its own gate check) -- that flag only
					meant "don't ask the Server to spawn something it already told us exists", but
					never got cleared on despawn, so re-spawning either option silently no-op'd
					after a revert even though the Server-side entity was really gone. */
					m_ArenaSpawnAccepted.fill(false);
				}
			}
		}
	}

	/* CreateCharacterButton stages the same one-shot request as the Debug "Create Character"
	button; Render_CreateCharacterProductInputHost consumes it and drives the (now ImGui-free)
	modal in both configurations. bInteractable already covers
	isConnecting/isReturning/transitionPending via MODE::SERVER_ARENA and the other two checks;
	m_iPendingClassIndex is the one condition bInteractable does not already include. */
	{
		const CHARACTER_SELECT_PRODUCT_SLOT* pSlot =
			Find_ProductButtonSlot("CreateCharacterButton");
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (nullptr != pSlot)
		{
			Resolve_ProductButtonRect(m_pClassSelectView.get(), *pSlot,
				hasCompleteAuthoredButtons, fX, fY, fWidth, fHeight);
			const bool_t bHovered = bInteractable && !m_iPendingClassIndex.has_value() &&
				Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
			m_pClassSelectView->Set_SlotTexture(pSlot->pSlotId, bHovered ? pSlot->pHoverPath : "");
			if (bHovered)
			{
				Router.Claim_Mouse_This_Frame();
				if (Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
				{
					CMainApp::Play_UIButtonClickSound();
					Request_CreateCharacterButtonClick();
				}
			}
		}
	}

	/* Back: same expected Leave_ServerArena() the ImGui "Back" button already calls. Not gated by
	bInteractable -- an escape hatch should stay clickable through pending/preparing states, only
	guarded against firing again mid-transition. */
	{
		const CHARACTER_SELECT_PRODUCT_SLOT* pSlot =
			Find_ProductButtonSlot("GoBackIcon");
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (nullptr != pSlot)
		{
			Resolve_ProductButtonRect(m_pClassSelectView.get(), *pSlot,
				hasCompleteAuthoredButtons, fX, fY, fWidth, fHeight);
			const bool_t bHovered = !m_isCreateCharacterModalOpen &&
				!CLevelTransitionService::Is_Pending() &&
				Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
			m_pClassSelectView->Set_SlotTexture(pSlot->pSlotId, bHovered ? pSlot->pHoverPath : "");
			if (bHovered)
			{
				Router.Claim_Mouse_This_Frame();
				if (Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
				{
					CMainApp::Play_UIButtonClickSound();
					Leave_ServerArena();
				}
			}
		}
	}
}

void CLevel_CharacterSelect::Render_ArenaSpawnLabels()
{
	/* Same MODE::SERVER_ARENA gate as Update_ClassList/Update_ArenaSpawnButtons -- these are
	just the text captions for that same button art, so they must disappear and reappear
	together with it instead of floating on screen without their buttons underneath. */
	if (nullptr == m_pClassSelectView || MODE::SERVER_ARENA != m_eMode)
		return;

	struct SPAWN_LABEL { const char_t* pSlotId; const wchar_t* pLabel; };
	constexpr SPAWN_LABEL LABELS[] = {
		/* "몬스터 소환" */ { "SpawnMonsterButton", L"\xBAAC\xC2A4\xD130 \xC18C\xD658" },
		/* "보스 소환" */   { "BossSpawnButton",    L"\xBCF4\xC2A4 \xC18C\xD658" },
		/* "되돌리기" */    { "SpawnCancelButton",  L"\xB418\xB3CC\xB9AC\xAE30" },
	};

	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vViewportSize.x / 1280.f;
	const float textScaleY = vViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);
	const bool_t hasCompleteAuthoredButtons =
		Has_CompleteProductButtonSlots(m_pClassSelectView.get());

	if (!m_isCreateCharacterModalOpen)
	{
		for (const SPAWN_LABEL& Label : LABELS)
		{
			const CHARACTER_SELECT_PRODUCT_SLOT* pSlot =
				Find_ProductButtonSlot(Label.pSlotId);
			if (nullptr == pSlot)
				continue;
			f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
			Resolve_ProductButtonRect(m_pClassSelectView.get(), *pSlot,
				hasCompleteAuthoredButtons, fX, fY, fWidth, fHeight);

			const f32_t fLabelCenterX = fX + fWidth * 0.5f;

			const float2_t vMeasured =
				CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), Label.pLabel);
			/* "작게" -- small label under a 70px icon, so cap by height too (same lesson as the
			Lobby button label: height-only scaling on a wide multi-glyph string runs it too big).
			Width cap tightened from 1.3x to 0.75x icon width -- the icons themselves sit only ~63px
			apart center-to-center (real extracted rects), so the wider cap let neighboring labels'
			edges overlap and read as one run-on string. */
			/* Retail caption size ($YoonGasiIIM 16 at 1080p -> 10.667 here); the width cap
			only guards against a neighbour overlap. */
			const f32_t fScaleByHeight = (vMeasured.y > 0.f) ?
				(fHeight * 0.2f / vMeasured.y) : 1.f;
			const f32_t fScaleByWidth = (vMeasured.x > 0.f) ?
				(fWidth * 0.75f / vMeasured.x) : 1.f;
			const f32_t fScale = (std::min)(fScaleByHeight, fScaleByWidth);
			/* Centered anchor like every other Draw_Text call in this codebase (see
			RenderQuickSlotKeyLabels), offset down by half the scaled glyph height so the label sits
			just under the icon instead of straddling its bottom edge. */
			const f32_t fLabelCenterY =
				fY + fHeight + 4.f + vMeasured.y * fScale * 0.5f;
			CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), Label.pLabel,
				float2_t(fLabelCenterX * textScaleX, fLabelCenterY * textScaleY),
				Colors::White, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
		}

		/* CreateCharacterButton: label centered inside the button itself (Lobby's
		Lobby_CreateCharacterButton uses the same literal/font for the same text). */
		{
			const CHARACTER_SELECT_PRODUCT_SLOT* pSlot =
				Find_ProductButtonSlot("CreateCharacterButton");
			f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
			if (nullptr != pSlot)
			{
				Resolve_ProductButtonRect(m_pClassSelectView.get(), *pSlot,
					hasCompleteAuthoredButtons, fX, fY, fWidth, fHeight);
				const wchar_t* pLabel = L"\xCE90\xB9AD\xD130 \xC0DD\xC131"; // "캐릭터 생성"
				const float2_t vMeasured =
					CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pLabel);
				const f32_t fScaleByHeight = (vMeasured.y > 0.f) ?
					(fHeight * 0.32f / vMeasured.y) : 1.f;
				const f32_t fScaleByWidth = (vMeasured.x > 0.f) ?
					(fWidth * 0.8f / vMeasured.x) : 1.f;
				const f32_t fScale = (std::min)(fScaleByHeight, fScaleByWidth);
				CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pLabel,
					float2_t((fX + fWidth * 0.5f) * textScaleX,
						(fY + fHeight * 0.5f) * textScaleY),
					Colors::White, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
			}
		}
	}

	/* Create Character modal text -- real Draw_Text same as everything else above.
	Render_CreateCharacterModal owns the CUI_Sprite art state and the WM_CHAR editing; every
	glyph the modal shows (title/labels, the nickname itself, the IME's in-progress syllable,
	caret, status line) draws here in the LOA-font pass. */
	if (m_isCreateCharacterModalOpen)
	{
		const auto Fn_DrawCentered = [&](f32_t fCenterX, f32_t fCenterY, const wchar_t* pLabel,
			f32_t fTargetHeight, const fvector_t& vColor)
		{
			const float2_t vMeasured =
				CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pLabel);
			const f32_t fScale = (vMeasured.y > 0.f) ? (fTargetHeight / vMeasured.y) : 1.f;
			CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pLabel,
				float2_t(fCenterX * textScaleX, fCenterY * textScaleY),
				vColor, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
		};

		f32_t fPanelX = 0.f, fPanelY = 0.f, fPanelW = 0.f, fPanelH = 0.f;
		if (m_pClassSelectView->Get_SlotRect(
			"CreateCharacterModal_Panel", fPanelX, fPanelY, fPanelW, fPanelH))
		{
			const f32_t fPanelCenterX = fPanelX + fPanelW * 0.5f;
			/* Real divider line sits at 43/131 of the panel art's own height (measured from the
			source pixels) -- subtitle goes just above it, inside the panel. Title sits clearly
			above the panel's own top edge instead (fLineY-based offset put it only 3px below
			fPanelY, overlapping the panel art). */
			const f32_t fLineY = fPanelY + fPanelH * (43.f / 131.f);
			Fn_DrawCentered(fPanelCenterX, fPanelY - 20.f,
				L"\xCE90\xB9AD\xD130 \xC774\xB984 \xC785\xB825", // "캐릭터 이름 입력"
				22.f, Colors::White);
			Fn_DrawCentered(fPanelCenterX, fLineY - 15.f,
				L"\xD55C\xAE00, \xC601\xBB38, \xC22B\xC790 12\xC790\xAE4C\xC9C0 \xC785\xB825 \xAC00\xB2A5", // "한글, 영문, 숫자 12자까지 입력 가능"
				15.f, Colors::Gold);
		}

		struct MODAL_BUTTON_LABEL { const char_t* pSlotId; const wchar_t* pLabel; };
		const MODAL_BUTTON_LABEL BUTTON_LABELS[] = {
			{ "CreateCharacterModal_ConfirmButton", L"\xD655\xC778" }, // "확인"
			{ "CreateCharacterModal_CancelButton", L"\xCDE8\xC18C" },  // "취소"
		};
		for (const MODAL_BUTTON_LABEL& Label : BUTTON_LABELS)
		{
			f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
			if (!m_pClassSelectView->Get_SlotRect(Label.pSlotId, fX, fY, fWidth, fHeight))
				continue;
			Fn_DrawCentered(fX + fWidth * 0.5f, fY + fHeight * 0.5f, Label.pLabel,
				fHeight * 0.32f, Colors::White);
		}

		/* The nickname text itself + the IME's still-composing syllable + a blinking caret --
		the drawing half of the runtime text field (editing lives in Render_CreateCharacterModal,
		fed by CUIInputRouter's WM_CHAR queue). Left-aligned inside the TextBox art the way the
		old transparent InputText sat over it. */
		f32_t fBoxX = 0.f, fBoxY = 0.f, fBoxW = 0.f, fBoxH = 0.f;
		if (m_pClassSelectView->Get_SlotRect(
			"CreateCharacterModal_TextBox", fBoxX, fBoxY, fBoxW, fBoxH))
		{
			constexpr f32_t TEXT_HEIGHT = 18.f;
			const f32_t fCenterScreenY = (fBoxY + fBoxH * 0.5f) * textScaleY;
			/* Returns the drawn advance in screen pixels so the next piece starts where this
			one ended -- position and advance both live in screen space (textScaleX vs
			textUiScale differ on a non-16:9 viewport, so mixing spaces would drift). */
			const auto Fn_DrawLeft = [&](f32_t fScreenX, const wchar_t* pText,
				const fvector_t& vColor) -> f32_t
			{
				const float2_t vMeasured =
					CGameInstance::Get().Measure_Text(TEXT("Font_YG330"), pText);
				if (vMeasured.y <= 0.f)
					return 0.f;
				const f32_t fScale = (TEXT_HEIGHT / vMeasured.y) * textUiScale;
				CGameInstance::Get().Draw_Text(TEXT("Font_YG330"), pText,
					float2_t(fScreenX, fCenterScreenY),
					vColor, 0.f, float2_t(0.f, 0.5f), fScale);
				return vMeasured.x * fScale;
			};

			f32_t fCursorScreenX = (fBoxX + 12.f) * textScaleX;
			if (!m_NicknameDraftW.empty())
				fCursorScreenX += Fn_DrawLeft(fCursorScreenX, m_NicknameDraftW.c_str(),
					Colors::White);
			/* In-progress (uncommitted) Hangul straight from the OS IME, gold so it reads as
			not-yet-committed -- the inline preview the old InputText overlay drew. */
			const wchar_t* pComposition = Engine::CImGuiLayer::Get_ImeCompositionString();
			if (nullptr != pComposition && L'\0' != pComposition[0])
				fCursorScreenX += Fn_DrawLeft(fCursorScreenX, pComposition, Colors::Gold);
			/* Blinking caret on its own wall clock (steady_clock, not ImGui's). */
			const int64_t iHalfSeconds =
				std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::steady_clock::now().time_since_epoch()).count() / 500;
			if (0 == (iHalfSeconds % 2))
				Fn_DrawLeft(fCursorScreenX + 1.f, L"|", Colors::White);
		}

		/* Status line -- moved out of the removed ImGui popup (same slot/fallback rect and
		amber tone its AddText version used). The messages are ASCII, so the byte-wise widen is
		exact. */
		if (!m_strStatus.empty())
		{
			f32_t fStatusX = 400.f, fStatusY = 340.f;
			f32_t fStatusWidth = 480.f, fStatusHeight = 20.f;
			f32_t fAuthoredX = 0.f, fAuthoredY = 0.f;
			f32_t fAuthoredWidth = 0.f, fAuthoredHeight = 0.f;
			if (m_pClassSelectView->Get_SlotRect(
				"CreateCharacterModal_StatusText", fAuthoredX, fAuthoredY,
				fAuthoredWidth, fAuthoredHeight) &&
				std::isfinite(fAuthoredX) && std::isfinite(fAuthoredY) &&
				std::isfinite(fAuthoredWidth) && std::isfinite(fAuthoredHeight) &&
				fAuthoredWidth > 0.f && fAuthoredHeight > 0.f)
			{
				fStatusX = fAuthoredX;
				fStatusY = fAuthoredY;
				fStatusWidth = fAuthoredWidth;
				fStatusHeight = fAuthoredHeight;
			}
			const wstring_t strStatusWide(m_strStatus.begin(), m_strStatus.end());
			const float2_t vMeasured =
				CGameInstance::Get().Measure_Text(TEXT("Font_YG330"), strStatusWide.c_str());
			if (vMeasured.y > 0.f)
			{
				f32_t fScale = (14.f / vMeasured.y) * textUiScale;
				/* Keep the whole line inside the authored width instead of the popup's old
				wrap -- Draw_Text has no wrapping, shrink-to-fit reads better than clipping. */
				const f32_t fMaxWidth = (fStatusWidth - 8.f) * textScaleX;
				if (vMeasured.x * fScale > fMaxWidth && vMeasured.x > 0.f)
					fScale = fMaxWidth / vMeasured.x;
				/* Centered on the panel's own middle, the same x every Korean line in this
				modal (title, subtitle, button labels) is centered on -- left-anchored inside
				the status slot put it visibly off-axis from the text right above it. */
				f32_t fCenterX = fStatusX + fStatusWidth * 0.5f;
				f32_t fPanelX = 0.f, fPanelY = 0.f, fPanelW = 0.f, fPanelH = 0.f;
				if (m_pClassSelectView->Get_SlotRect(
					"CreateCharacterModal_Panel", fPanelX, fPanelY, fPanelW, fPanelH))
				{
					fCenterX = fPanelX + fPanelW * 0.5f;
				}
				const float2_t vPosition(
					fCenterX * textScaleX,
					(fStatusY + fStatusHeight * 0.5f) * textScaleY);
				CGameInstance::Get().Draw_Text(TEXT("Font_YG330"), strStatusWide.c_str(),
					float2_t(vPosition.x + 1.f, vPosition.y + 1.f),
					XMVectorSet(0.f, 0.f, 0.f, 0.9f), 0.f, float2_t(0.5f, 0.5f), fScale);
				CGameInstance::Get().Draw_Text(TEXT("Font_YG330"), strStatusWide.c_str(),
					vPosition,
					XMVectorSet(1.f, 210.f / 255.f, 120.f / 255.f, 1.f), 0.f,
					float2_t(0.5f, 0.5f), fScale);
			}
		}
	}
}

#ifdef _DEBUG
void CLevel_CharacterSelect::Update_RaidEntryDebugPreviewKey()
{
	if (nullptr == m_pDebugRaidEntryPreviewView ||
		m_pDebugRaidEntryPreviewView->Is_Open() || ImGui::GetIO().WantTextInput ||
		CUIInputRouter::Get().Is_TextInputActive())
	{
		return;
	}
	/* Raw, not the blocked accessor: the ImGui debug panels this level keeps open grab the
	   keyboard and zero Get_DIKeyState, which ate this debug key entirely. The text-input
	   gate above still keeps a typed 'O' from opening the popup. */
	const bool_t isODown =
		0 != (CGameInstance::Get().Get_DIKeyStateRaw(DIK_O) & 0x80);
	const bool_t wasOPressed = isODown && !m_wasODownForRaidEntryDebugPreview;
	m_wasODownForRaidEntryDebugPreview = isODown;
	if (wasOPressed)
		m_pDebugRaidEntryPreviewView->Open();
}

void CLevel_CharacterSelect::Render_RaidEntryDebugPreview()
{
	if (nullptr == m_pDebugRaidEntryPreviewView)
		return;
	// Visual-only: no real NPC, no command sink -- Entrance just closes it too.
	(void)m_pDebugRaidEntryPreviewView->Render();
}

void CLevel_CharacterSelect::Render_RaidEntryDebugPreviewText()
{
	if (nullptr != m_pDebugRaidEntryPreviewView)
		m_pDebugRaidEntryPreviewView->RenderText();
}

bool_t CLevel_CharacterSelect::Is_DebugRaidEntryPreviewOpen() const
{
	return nullptr != m_pDebugRaidEntryPreviewView &&
		m_pDebugRaidEntryPreviewView->Is_Open();
}
#endif

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

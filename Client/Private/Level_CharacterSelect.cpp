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
#include "PlayableCharacterAssetService.h"
#include "Transform.h"

#include <algorithm>

namespace
{
	constexpr f32_t PREVIEW_POSITION_X = -772.017f;
	constexpr f32_t PREVIEW_POSITION_Y = -142.55f;
	constexpr f32_t PREVIEW_POSITION_Z = 197.538f;

	const char_t* Get_CharacterClassName(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (characterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER:
			return "Lance Master";
		// TODO: display-only relabel until real character assets exist; underlying class/binding is unchanged.
		case CHARACTER_CLASS_ID::GUNSLINGER:
			return "War Lord";
		case CHARACTER_CLASS_ID::SLAYER:
			return "Alchemist";
		case CHARACTER_CLASS_ID::ARTIST:
			return "Artist";
		case CHARACTER_CLASS_ID::DIMENSIONIST:
			return "Dimension Master";
		default:
			return "Unknown";
		}
	}

	const char_t* Get_StageName(const LOBBY_STAGE eStage)
	{
		switch (eStage)
		{
		case LOBBY_STAGE::TEST:
			return "Test";
		case LOBBY_STAGE::BERN:
			return "Bern";
		case LOBBY_STAGE::VALTAN:
			return "Valtan";
		default:
			return "Unknown";
		}
	}

	const char_t* Get_StageTransitionSource(const LOBBY_STAGE eStage)
	{
		switch (eStage)
		{
		case LOBBY_STAGE::TEST:
			return "character-select.enter-test";
		case LOBBY_STAGE::BERN:
			return "character-select.enter-bern";
		case LOBBY_STAGE::VALTAN:
			return "character-select.enter-valtan";
		default:
			return nullptr;
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
	CCombatHUDViewModel::Get().Reset_RuntimeState();
	m_MapRuntime.Clear();
}

HRESULT CLevel_CharacterSelect::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;
	if (!CCombatHUDViewModel::Get().Initialize_Definitions())
		return E_FAIL;

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::CHARACTER_SELECT);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		!m_MapRuntime.Load_Area(
			ETOUI(LEVEL::CHARACTER_SELECT),
			pEntry->pMapAreaId,
			pEntry->MapLoadScope))
	{
		OutputDebugStringA((
			"[Level_CharacterSelect] " +
			m_MapRuntime.Get_Status() + "\n").c_str());
		return E_FAIL;
	}

	if (FAILED(Ready_Lights()))
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

	return S_OK;
}

void CLevel_CharacterSelect::Update(const f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);
}

HRESULT CLevel_CharacterSelect::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

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

	CCamera_Free::CAMERA_FREE_DESC cameraDesc{};
	cameraDesc.vEye = float3_t(
		PREVIEW_POSITION_X,
		PREVIEW_POSITION_Y + 2.4f,
		PREVIEW_POSITION_Z - 6.f);
	cameraDesc.vAt = float3_t(
		PREVIEW_POSITION_X,
		PREVIEW_POSITION_Y + 1.f,
		PREVIEW_POSITION_Z);
	cameraDesc.fFovy = 45.f;
	cameraDesc.fNear = 0.1f;
	cameraDesc.fFar = 1000.f;
	cameraDesc.fSpeedPerSec = 8.f;
	cameraDesc.fRotationPerSec = 90.f;
	cameraDesc.fMouseSensor = 0.1f;
	cameraDesc.pFollowTarget = m_pPreviewCharacter->Get_Transform();
	cameraDesc.vPositionOffset = float3_t(0.f, 2.4f, -6.f);
	cameraDesc.vLookOffset = float3_t(0.f, 1.f, 0.f);
	cameraDesc.fFollowResponse = 18.f;
	cameraDesc.isFollowEnabled = true;

	shared_ptr<CGameObject> gameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::CHARACTER_SELECT),
		TEXT("Prototype_GameObject_Camera_Free"),
		ETOUI(LEVEL::CHARACTER_SELECT),
		TEXT("Layer_Camera"),
		&cameraDesc,
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

	return S_OK;
}

HRESULT CLevel_CharacterSelect::Ready_Preview(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	if (!LostArk::Shared::Is_Supported_Playable_Character_Class(
		characterClass))
	{
		return E_INVALIDARG;
	}

	const CHARACTER_SPEC* pSpec = CCharacterCatalog::Find_Spec(characterClass);
	if (nullptr == pSpec)
		return E_FAIL;

	CCharacter::CHARACTER_DESC characterDesc{};
	characterDesc.iPrototypeLevelIndex = ETOUI(LEVEL::CHARACTER_SELECT);
	characterDesc.pSpec = pSpec;
	characterDesc.pNavigationPrototypeTag = nullptr;
	characterDesc.fSpeedPerSec = 0.f;
	characterDesc.fRotationPerSec = 90.f;
	characterDesc.vPosition = float3_t(
		PREVIEW_POSITION_X,
		PREVIEW_POSITION_Y,
		PREVIEW_POSITION_Z);
	characterDesc.strNickName = Get_CharacterClassName(characterClass);
	characterDesc.isLocallyControlled = false;

	shared_ptr<CGameObject> stagedObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::CHARACTER_SELECT),
		TEXT("Prototype_GameObject_Character"),
		ETOUI(LEVEL::CHARACTER_SELECT),
		TEXT("Layer_PreviewCharacter"),
		&characterDesc,
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
	if (!CCombatHUDViewModel::Get().Apply_CharacterPreview(characterClass))
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::CHARACTER_SELECT),
			TEXT("Layer_PreviewCharacter"),
			stagedObject);
		return E_FAIL;
	}

	stagedCharacter->Get_Transform()->Rotation(0.f, 180.f, 0.f);

	const shared_ptr<CCharacter> previousCharacter = m_pPreviewCharacter;
	m_pPreviewCharacter = stagedCharacter;
	if (nullptr != previousCharacter)
		CAnimationTargetService::Unbind(previousCharacter);
	CAnimationTargetService::Bind(m_pPreviewCharacter);

	if (nullptr != m_pCamera)
	{
		m_pCamera->Set_FollowTarget(m_pPreviewCharacter->Get_Transform());
		m_pCamera->Set_FollowEnabled(true);
	}

	if (nullptr != previousCharacter)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::CHARACTER_SELECT),
			TEXT("Layer_PreviewCharacter"),
			previousCharacter);
	}

	return S_OK;
}

bool_t CLevel_CharacterSelect::Select_Preview(const size_t index)
{
	if (index >= SUPPORTED_CLASSES.size())
		return false;
	if (index == m_iPreviewIndex && nullptr != m_pPreviewCharacter)
		return true;

	const auto characterClass = SUPPORTED_CLASSES[index];
	if (!CPlayableCharacterAssetService::Is_Ready(
		ETOUI(LEVEL::CHARACTER_SELECT),
		characterClass))
	{
		m_strStatus = std::string("Preparing ") +
			Get_CharacterClassName(characterClass) +
			" binary assets for the first preview...";
		if (FAILED(CPlayableCharacterAssetService::Ensure_Prototypes(
			m_pDevice,
			m_pContext,
			ETOUI(LEVEL::CHARACTER_SELECT),
			characterClass)))
		{
			m_strStatus =
				"The selected class assets could not be prepared. "
				"The previous preview was kept.";
			return false;
		}
	}

	if (FAILED(Ready_Preview(characterClass)))
	{
		m_strStatus =
			"The new preview could not be created. The previous preview was kept.";
		return false;
	}

	m_iPreviewIndex = index;
	m_strStatus = std::string("Previewing ") +
		Get_CharacterClassName(SUPPORTED_CLASSES[index]) +
		". Choose Test, Bern, or Valtan.";
	return true;
}

bool_t CLevel_CharacterSelect::Enter_Stage(const LOBBY_STAGE eStage)
{
	const char_t* pStageName = Get_StageName(eStage);
	const char_t* pTransitionSource = Get_StageTransitionSource(eStage);
	if (nullptr == pTransitionSource)
	{
		m_strStatus = "The selected stage is not supported here.";
		return false;
	}
	if (m_iPreviewIndex >= SUPPORTED_CLASSES.size() ||
		nullptr == m_pPreviewCharacter)
	{
		m_strStatus = "There is no valid preview to enter with.";
		return false;
	}

	if (!CCharacterSelectionState::Select(
		SUPPORTED_CLASSES[m_iPreviewIndex]))
	{
		m_strStatus = "The selected class is not supported.";
		return false;
	}

	LOBBY_COMMAND_TOKEN commandToken = INVALID_LOBBY_COMMAND_TOKEN;
	if (!CLobbyCommandService::Request(
		eStage,
		commandToken))
	{
		m_strStatus = CLobbyCommandService::Get_Status();
		return false;
	}

	if (!CLevelTransitionService::Request_Load(
		LEVEL::LOBBY,
		pTransitionSource,
		commandToken))
	{
		CLobbyCommandService::Cancel(
			commandToken,
			"Lobby load request was rejected");
		m_strStatus = CLevelTransitionService::Get_Status();
		return false;
	}

	m_strStatus = std::string("Selection committed. Lobby will request ") +
		pStageName + " from Server.";
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

	ImGui::TextUnformatted("Choose a playable class");
	for (size_t index = 0; index < SUPPORTED_CLASSES.size(); ++index)
	{
		const bool_t isSelected = index == m_iPreviewIndex;
		if (ImGui::Selectable(
			Get_CharacterClassName(SUPPORTED_CLASSES[index]),
			isSelected))
		{
			Select_Preview(index);
		}
	}

	ImGui::Separator();
	const bool_t transitionPending = CLevelTransitionService::Is_Pending();
	const bool_t enterPressed =
		ImGui::IsKeyPressed(ImGuiKey_Enter, false) ||
		ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false);
	ImGui::BeginDisabled(transitionPending);
	if (ImGui::Button("Enter Test") ||
		(!transitionPending && enterPressed))
	{
		Enter_Stage(LOBBY_STAGE::TEST);
	}
	ImGui::SameLine();
	if (ImGui::Button("Enter Bern"))
		Enter_Stage(LOBBY_STAGE::BERN);
	ImGui::SameLine();
	if (ImGui::Button("Enter Valtan"))
		Enter_Stage(LOBBY_STAGE::VALTAN);
	ImGui::SameLine();
	if (ImGui::Button("Back"))
	{
		if (!CLevelTransitionService::Request_Load(
			LEVEL::LOBBY,
			"character-select.back"))
		{
			m_strStatus = CLevelTransitionService::Get_Status();
		}
	}
	ImGui::EndDisabled();

	ImGui::TextDisabled(
		"Enter: Test  |  Bern/Valtan: server-approved entry  |  Back: Lobby");
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

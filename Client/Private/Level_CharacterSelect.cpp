#include "imgui.h"

#include "Level_CharacterSelect.h"

#include "AnimationTargetService.h"
#include "Camera_Free.h"
#include "Character.h"
#include "CharacterCatalog.h"
#include "CharacterSelectionState.h"
#include "GameInstance.h"
#include "LevelTransitionService.h"
#include "Transform.h"

#include <algorithm>

namespace
{
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
		default:
			return "Unknown";
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
}

HRESULT CLevel_CharacterSelect::Initialize()
{
	if (FAILED(__super::Initialize()) || FAILED(Ready_Lights()))
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
	LIGHT_DESC light{};
	light.eType = LIGHT::DIRECTIONAL;
	light.vDirection = float4_t(0.5f, -1.f, 0.35f, 0.f);
	light.vDiffuse = float4_t(0.9f, 0.9f, 0.9f, 1.f);
	light.vAmbient = float4_t(0.3f, 0.3f, 0.35f, 1.f);
	light.vSpecular = float4_t(0.7f, 0.7f, 0.7f, 1.f);
	return CGameInstance::Get().Add_Light(light);
}

HRESULT CLevel_CharacterSelect::Ready_Camera()
{
	if (nullptr == m_pPreviewCharacter ||
		nullptr == m_pPreviewCharacter->Get_Transform())
	{
		return E_FAIL;
	}

	CCamera_Free::CAMERA_FREE_DESC cameraDesc{};
	cameraDesc.vEye = float3_t(0.f, 2.f, -6.f);
	cameraDesc.vAt = float3_t(0.f, 1.f, 0.f);
	cameraDesc.fFovy = 45.f;
	cameraDesc.fNear = 0.1f;
	cameraDesc.fFar = 1000.f;
	cameraDesc.fSpeedPerSec = 8.f;
	cameraDesc.fRotationPerSec = 90.f;
	cameraDesc.fMouseSensor = 0.1f;
	cameraDesc.pFollowTarget = m_pPreviewCharacter->Get_Transform();
	cameraDesc.vPositionOffset = float3_t(0.f, 2.f, -6.f);
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
	characterDesc.vPosition = float3_t(0.f, 0.f, 0.f);
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

	if (FAILED(Ready_Preview(SUPPORTED_CLASSES[index])))
	{
		m_strStatus =
			"The new preview could not be created. The previous preview was kept.";
		return false;
	}

	m_iPreviewIndex = index;
	m_strStatus = std::string("Previewing ") +
		Get_CharacterClassName(SUPPORTED_CLASSES[index]) + ".";
	return true;
}

bool_t CLevel_CharacterSelect::Confirm_Selection()
{
	if (m_iPreviewIndex >= SUPPORTED_CLASSES.size() ||
		nullptr == m_pPreviewCharacter)
	{
		m_strStatus = "There is no valid preview to confirm.";
		return false;
	}

	if (!CCharacterSelectionState::Select(
		SUPPORTED_CLASSES[m_iPreviewIndex]))
	{
		m_strStatus = "The selected class is not supported.";
		return false;
	}

	if (!CLevelTransitionService::Request_Load(
		LEVEL::LOBBY,
		"character-select.confirm"))
	{
		m_strStatus = CLevelTransitionService::Get_Status();
		return false;
	}

	m_strStatus = "Selection confirmed. Returning to Lobby.";
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
	ImGui::BeginDisabled(transitionPending);
	if (ImGui::Button("Confirm"))
		Confirm_Selection();
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

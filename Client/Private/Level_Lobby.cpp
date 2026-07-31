#ifdef _DEBUG
#include "imgui.h"
#endif

#include "Level_Lobby.h"

#include "Level_Loading.h"
#include "GameInstance.h"
#include "Character.h"
#include "CharacterSpec.h"
#include "Logic_LanceMaster.h"
#include "Camera_Free.h"

namespace
{
    const char_t* Get_CharacterClassName(const CHARACTER_CLASS eClass)
    {
        switch (eClass)
        {
        case CHARACTER_CLASS::LANCE_MASTER:
            return "Lance Master";
        case CHARACTER_CLASS::GUNSLINGER:
            return "Gunslinger";
        case CHARACTER_CLASS::DESTROYER:
            return "Destroyer";
        case CHARACTER_CLASS::ARTIST:
            return "Artist";
        default:
            return "Unknown";
        }
    }
}

CLevel_Lobby::CLevel_Lobby(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    :CLevel{pDevice, pContext}
{}

CLevel_Lobby::~CLevel_Lobby()
{}

HRESULT CLevel_Lobby::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    if (FAILED(Ready_Lights()))
        return E_FAIL;

    if (FAILED(Ready_Characters()))
        return E_FAIL;

    if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
        return E_FAIL;

    return S_OK;
}

HRESULT CLevel_Lobby::Ready_Lights()
{
    LIGHT_DESC light{};

    light.eType = LIGHT::DIRECTIONAL;
    light.vDirection = float4_t(0.5f, -1.f, 0.5f, 0.f);
    light.vDiffuse = float4_t(0.8f, 0.8f, 0.8f, 1.f);
    light.vAmbient = float4_t(0.35f, 0.35f, 0.35f, 1.f);
    light.vSpecular = float4_t(0.5f, 0.5f, 0.5f, 1.f);

    return CGameInstance::Get().Add_Light(light);
}

HRESULT CLevel_Lobby::Ready_Layer_Camera(const wstring& strLayerTag)
{
    if (m_vecCharacters.empty() || nullptr == m_vecCharacters.front().m_pCharacter)
        return E_FAIL;
    //Character 기준으로 camera 생성
    const shared_ptr<CTransform> pCharacterTransform =
        m_vecCharacters.front().m_pCharacter->Get_Transform();

    if (nullptr == pCharacterTransform)
        return E_FAIL;

    //고정 Camera 생성
    CCamera_Free::CAMERA_FREE_DESC CameraDesc{};

    CameraDesc.vEye = { -4.f, 2.5f, -6.f };
    CameraDesc.vAt = { 0.f, 1.f, 0.f };

    CameraDesc.fFovy = 60.f;
    CameraDesc.fNear = 0.1f;
    CameraDesc.fFar = 1000.f;
    
    CameraDesc.fSpeedPerSec = 10.f;
    CameraDesc.fRotationPerSec = 90.f;
    CameraDesc.fMouseSensor = 0.1f;
    CameraDesc.pFollowTarget = pCharacterTransform;
    CameraDesc.vPositionOffset = { -4.f, 2.5f, -6.f };
    CameraDesc.vLookOffset = { 0.f, 1.f, 0.f };
    CameraDesc.fFollowResponse = 18.f;
    CameraDesc.isFollowEnabled = true;

    shared_ptr<CGameObject> pGameObject;
    
    if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(ETOUI(LEVEL::LOBBY),
        TEXT("Prototype_GameObject_Camera_Free"),
        ETOUI(LEVEL::LOBBY), strLayerTag, &CameraDesc, &pGameObject)))
        return E_FAIL;

    const shared_ptr<CCamera_Free> camera =
        dynamic_pointer_cast<CCamera_Free>(pGameObject);

    if (nullptr == camera)
        return E_FAIL;

    m_pCamera = camera;

    return Select_Character(0) ? S_OK : E_FAIL;
}

HRESULT CLevel_Lobby::Ready_Characters()
{
    //Lance_Master를 등록
    CCharacter::CHARACTER_DESC desc{};
    desc.iPrototypeLevelIndex = ETOUI(LEVEL::LOBBY);
    desc.pSpec = &Spec_LanceMaster;
    desc.pNavigationPrototypeTag = nullptr;
    desc.fSpeedPerSec = 5.f;
    desc.fRotationPerSec = 180.f;
    desc.vPosition = float3_t(0.f, 0.f, 0.f);

    shared_ptr<CGameObject> pGameObject;

    if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
        ETOUI(LEVEL::LOBBY),
        TEXT("Prototype_GameObject_Character"),
        ETOUI(LEVEL::LOBBY),
        TEXT("Layer_Player"),
        &desc,
        &pGameObject)))
        return E_FAIL;

    //m_vecCharactersInfo.push_back(pGameObject);

    const shared_ptr<CCharacter> pCharacter = dynamic_pointer_cast<CCharacter>(pGameObject);

    if (nullptr == pCharacter)
        return E_FAIL;

    LobbyCharacterInfo characterInfo{};

    characterInfo.m_iSlotIndex = 0;
    characterInfo.m_eClass = CHARACTER_CLASS::LANCE_MASTER;
    characterInfo.m_pCharacter = pCharacter;
    characterInfo.m_strNickName.clear();

    m_vecCharacters.push_back(move(characterInfo));

    return S_OK;
}

bool_t CLevel_Lobby::Select_Character(const int32_t iCharacterIndex)
{
    if (iCharacterIndex < 0 ||
        static_cast<size_t>(iCharacterIndex) >= m_vecCharacters.size())
        return false;

    const LobbyCharacterInfo& characterInfo = m_vecCharacters[iCharacterIndex];

    if (nullptr == characterInfo.m_pCharacter)
        return false;

    const shared_ptr<CTransform> pCharacterTransform =
        characterInfo.m_pCharacter->Get_Transform();

    if (nullptr == pCharacterTransform)
        return false;

    m_iSelectedCharacterIndex = iCharacterIndex;
    strncpy_s(m_szNickName, characterInfo.m_strNickName.c_str(), _TRUNCATE);

    if (nullptr != m_pCamera)
    {
        m_pCamera->Set_FollowTarget(pCharacterTransform);
        m_pCamera->Set_FollowEnabled(true);
    }

    return true;
}

void CLevel_Lobby::Update(f32_t fTimeDelta)
{
    //캐릭터 선택과 Lobby에서 해야 하는 작업들 전부 수행하기!
    if (m_isEnterRequested)
    {
        m_isEnterRequested = false;

        if (FAILED(CGameInstance::Get().Change_Level(
            ETOUI(LEVEL::LOADING),
            CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::BAREN))))
        {
            MSG_BOX("Failed to Change Level");
            return;
        }

        return;
    }

    __super::Update(fTimeDelta);
}

HRESULT CLevel_Lobby::Render()
{
#ifdef _DEBUG
    Render_CharacterSelectPanel();
#endif

    return S_OK;
}
#ifdef _DEBUG
void CLevel_Lobby::Render_CharacterSelectPanel()
{
    if (!ImGui::Begin(
        "Character Select",
        nullptr,
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::End();
        return;
    }

    ImGui::Text("Select Character");

    for (size_t i = 0; i < m_vecCharacters.size(); ++i)
    {
        const LobbyCharacterInfo& characterInfo = m_vecCharacters[i];

        const string label =
            string(Get_CharacterClassName(characterInfo.m_eClass)) +
            "##CharacterSlot" +
            to_string(characterInfo.m_iSlotIndex);

        const bool_t isSelected =
            static_cast<int32_t>(i) == m_iSelectedCharacterIndex;

        if (ImGui::Selectable(label.c_str(), isSelected))
        {
            Select_Character(static_cast<int32_t>(i));
        }
    }

    ImGui::Separator();

    const bool_t hasSelection =
        m_iSelectedCharacterIndex >= 0 &&
        static_cast<size_t>(m_iSelectedCharacterIndex) < m_vecCharacters.size();

    ImGui::BeginDisabled(!hasSelection);

    if (ImGui::InputText(
        "Nickname",
        m_szNickName,
        sizeof(m_szNickName)))
    {
        m_vecCharacters[m_iSelectedCharacterIndex].m_strNickName =
            m_szNickName;
    }

    ImGui::EndDisabled();

    const bool_t hasNickName =
        hasSelection &&
        m_szNickName[0] != '\0';

    ImGui::BeginDisabled(!hasNickName);

    if (ImGui::Button("Enter Baren"))
    {
        m_vecCharacters[m_iSelectedCharacterIndex].m_strNickName =
            m_szNickName;

        m_isEnterRequested = true;
    }

    ImGui::EndDisabled();

    if (!hasNickName)
    {
        ImGui::TextDisabled(
            "Select a character and enter a nickname.");
    }

    ImGui::End();
}
#endif

unique_ptr<CLevel_Lobby> CLevel_Lobby::Create(ComPtr<ID3D11Device> pDevice, 
    ComPtr<ID3D11DeviceContext> pContext)
{
    auto pInstance = unique_ptr<CLevel_Lobby>(new CLevel_Lobby(pDevice, pContext));


    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to create : Level Lobby");
        return nullptr;
    }

    return pInstance;
}

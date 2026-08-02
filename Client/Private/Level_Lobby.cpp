#ifdef _DEBUG
#include "imgui.h"
#endif

#include "NetworkManager.h"
#include "Level_Lobby.h"

#include "Level_Loading.h"
#include "GameInstance.h"
#include "Character.h"
#include "Camera_Free.h"

namespace
{
    const char_t* Get_CharacterClassName(
        const LostArk::Shared::
        CHARACTER_CLASS_ID eClass)
    {
        using LostArk::Shared::
            CHARACTER_CLASS_ID;

        switch (eClass)
        {
        case CHARACTER_CLASS_ID::LANCE_MASTER:
            return "Lance Master";

        case CHARACTER_CLASS_ID::GUNSLINGER:
            return "Gunslinger";

        case CHARACTER_CLASS_ID::DESTROYER:
            return "Destroyer";

        case CHARACTER_CLASS_ID::ARTIST:
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

    if (FAILED(Ready_CharacterSlots()))
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
    if (m_vecCharacters.empty())
        return E_FAIL;

    // Lobby network validation does not need to decode or clone a character.
    // Start as a free camera and attach a follow target later when a preview exists.
    CCamera_Free::CAMERA_FREE_DESC CameraDesc{};

    CameraDesc.vEye = { -4.f, 2.5f, -6.f };
    CameraDesc.vAt = { 0.f, 1.f, 0.f };

    CameraDesc.fFovy = 60.f;
    CameraDesc.fNear = 0.1f;
    CameraDesc.fFar = 1000.f;
    
    CameraDesc.fSpeedPerSec = 10.f;
    CameraDesc.fRotationPerSec = 90.f;
    CameraDesc.fMouseSensor = 0.1f;
    CameraDesc.pFollowTarget = nullptr;
    CameraDesc.vPositionOffset = { -4.f, 2.5f, -6.f };
    CameraDesc.vLookOffset = { 0.f, 1.f, 0.f };
    CameraDesc.fFollowResponse = 18.f;
    CameraDesc.isFollowEnabled = false;

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

HRESULT CLevel_Lobby::Ready_CharacterSlots()
{
    LobbyCharacterInfo characterInfo{};

    characterInfo.m_iSlotIndex = 0;
    characterInfo.m_eClass = LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER;
    characterInfo.m_pCharacter = nullptr;
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

    m_iSelectedCharacterIndex = iCharacterIndex;
    strncpy_s(m_szNickName, characterInfo.m_strNickName.c_str(), _TRUNCATE);

    if (nullptr != m_pCamera)
    {
        shared_ptr<CTransform> pCharacterTransform;
        if (nullptr != characterInfo.m_pCharacter)
            pCharacterTransform = characterInfo.m_pCharacter->Get_Transform();

        m_pCamera->Set_FollowTarget(pCharacterTransform);
        m_pCamera->Set_FollowEnabled(nullptr != pCharacterTransform);
    }

    return true;
}

bool_t CLevel_Lobby::Request_EnterWorld()
{
    if (m_iSelectedCharacterIndex < 0 ||
        static_cast<size_t>(m_iSelectedCharacterIndex) >= m_vecCharacters.size())
    {
        m_strNetworkStatus = "Select a character first.";
        return false;
    }

    LobbyCharacterInfo& characterInfo =
        m_vecCharacters[m_iSelectedCharacterIndex];
    characterInfo.m_strNickName =
        '\0' == m_szNickName[0] ? "Player" : m_szNickName;

    CNetworkManager& networkManager = CNetworkManager::Get();
    if (!networkManager.Is_Connected() &&
        !networkManager.Connect_To_Server(7777))
    {
        m_strNetworkStatus =
            "Connect failed. WSA error: " +
            to_string(networkManager.Get_LastErrorCode());
        return false;
    }

    if (!networkManager.Send_EnterWorld(
        characterInfo.m_eClass,
        characterInfo.m_strNickName))
    {
        m_strNetworkStatus =
            "Send failed. WSA error: " +
            to_string(networkManager.Get_LastErrorCode());
        return false;
    }

    m_strNetworkStatus =
        "C2S_ENTER_WORLD sent. Check the server console.";
    return true;
}

void CLevel_Lobby::Update(f32_t fTimeDelta)
{
    // F8: 베른성, F9: 발탄 아레나로 바로 이동한다.
    // 서버 접속 없이 맵만 확인할 때 쓰는 디버그 단축키다.
    if (!CGameInstance::Get().IsKeyboardInputBlocked())
    {
        LEVEL eShortcutLevelID = LEVEL::END;

        if (GetKeyState(VK_F8) & 0x8000)
            eShortcutLevelID = LEVEL::BAREN;
        else if (GetKeyState(VK_F9) & 0x8000)
            eShortcutLevelID = LEVEL::VALTAN_ARENA;

        if (LEVEL::END != eShortcutLevelID)
        {
            if (FAILED(CGameInstance::Get().Change_Level(
                ETOUI(LEVEL::LOADING),
                CLevel_Loading::Create(
                    m_pDevice, m_pContext, eShortcutLevelID))))
            {
                MSG_BOX("Failed to Change Level");
            }

            return;
        }
    }

    LostArk::Shared::S2C_ENTER_ACCEPTED accepted;
    //networkmanager에서 update를 돌면서 enter 여부 판단
    if (CNetworkManager::Get().Try_Consume_EnterAccepted(accepted))
    {
        m_strNetworkStatus =
            "S2C_ENTER_ACCEPTED received. "
            "PlayerId=" +
            to_string(
                accepted.iPlayerId) +
            ", NetEntityId=" +
            to_string(
                accepted.iNetEntityId);

        m_isEnterRequested = true;
    }

    // 서버 승인 메시지를 받은 뒤에만 true로 변경한다.
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

    ImGui::BeginDisabled(!hasSelection);

    if (ImGui::Button("Enter Baren"))
    {
        Request_EnterWorld();
    }

    ImGui::EndDisabled();

    if (!hasSelection)
    {
        ImGui::TextDisabled(
            "Select a character first.");
    }

    ImGui::TextWrapped("%s", m_strNetworkStatus.c_str());

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

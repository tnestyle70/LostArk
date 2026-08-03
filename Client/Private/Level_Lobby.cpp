#include "imgui.h"

#include "NetworkManager.h"
#include "ClientLaunchOptions.h"
#include "Level_Lobby.h"

#include "GameInstance.h"
#include "Character.h"
#include "Camera_Free.h"
#include "LobbyCommandService.h"
#include "SceneTransitionService.h"

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
            // TODO: displays as War Lord until real WarLord character assets exist; underlying class/binding is still Gunslinger.
            return "War Lord";

        case CHARACTER_CLASS_ID::SLAYER:
            // TODO: displays as Alchemist until real Alchemist character assets exist; underlying class/binding is still Slayer.
            return "Alchemist";

        case CHARACTER_CLASS_ID::ARTIST:
            // TODO: displays as Dimension Master until real DimensionMaster character assets exist; underlying class/binding is still Artist.
            return "Dimension Master";

        default:
            return "Unknown";
        }
    }

    bool_t Try_ResolveWorldScene(
        const LostArk::Shared::WORLD_ID eWorldId,
        LEVEL& eTargetLevel,
        Client::CLIENT_SCENARIO& eTargetScenario)
    {
        using LostArk::Shared::WORLD_ID;

        eTargetLevel = LEVEL::END;
        eTargetScenario = Client::CLIENT_SCENARIO::END;

        switch (eWorldId)
        {
        case WORLD_ID::BERN:
            eTargetLevel = LEVEL::BERN;
            eTargetScenario = Client::CLIENT_SCENARIO::WORLD_BERN;
            return true;

        case WORLD_ID::VALTAN_ARENA:
            eTargetLevel = LEVEL::VALTAN_ARENA;
            eTargetScenario = Client::CLIENT_SCENARIO::RAID_VALTAN_ARENA;
            return true;

        case WORLD_ID::TRAINING_GROUND:
            eTargetLevel = LEVEL::DEVELOPMENT;
            eTargetScenario =
                Client::CLIENT_SCENARIO::DEVELOPMENT_TRAINING_GROUND;
            return true;

        default:
            return false;
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

    int32_t selectedIndex = 0;
    if (CClientLaunchOptions::Get().SelectedCharacterClass.has_value())
    {
        const auto selected = find_if(
            m_vecCharacters.begin(),
            m_vecCharacters.end(),
            [](const LobbyCharacterInfo& character)
            {
                return character.m_eClass ==
                    *CClientLaunchOptions::Get().SelectedCharacterClass;
            });
        if (selected == m_vecCharacters.end())
            return E_FAIL;
        selectedIndex = static_cast<int32_t>(
            distance(m_vecCharacters.begin(), selected));
    }

    return Select_Character(selectedIndex) ? S_OK : E_FAIL;
}

HRESULT CLevel_Lobby::Ready_CharacterSlots()
{
    using LostArk::Shared::CHARACTER_CLASS_ID;
    constexpr CHARACTER_CLASS_ID classes[] =
    {
        CHARACTER_CLASS_ID::LANCE_MASTER,
        CHARACTER_CLASS_ID::GUNSLINGER,
        CHARACTER_CLASS_ID::SLAYER,
        CHARACTER_CLASS_ID::ARTIST,
    };

    m_vecCharacters.clear();
    m_vecCharacters.reserve(4);
    for (size_t index = 0; index < 4; ++index)
    {
        LobbyCharacterInfo characterInfo{};
        characterInfo.m_iSlotIndex = static_cast<int32_t>(index);
        characterInfo.m_eClass = classes[index];
        m_vecCharacters.push_back(move(characterInfo));
    }

    return S_OK;
}

bool_t CLevel_Lobby::Select_Character(const int32_t iCharacterIndex)
{
    if (iCharacterIndex < 0 ||
        static_cast<size_t>(iCharacterIndex) >= m_vecCharacters.size())
        return false;

    const LobbyCharacterInfo& characterInfo = m_vecCharacters[iCharacterIndex];

    if (!CClientLaunchOptions::Select_RuntimeCharacterClass(
        characterInfo.m_eClass))
    {
        return false;
    }

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

bool_t CLevel_Lobby::Request_EnterWorld(
    const LostArk::Shared::WORLD_ID eWorldId)
{
    if (m_iSelectedCharacterIndex < 0 ||
        static_cast<size_t>(m_iSelectedCharacterIndex) >= m_vecCharacters.size())
    {
        m_strNetworkStatus = "Select a character first.";
        return false;
    }

    LobbyCharacterInfo& characterInfo =
        m_vecCharacters[m_iSelectedCharacterIndex];
    if (!LostArk::Shared::Is_Supported_Playable_Character_Class(
        characterInfo.m_eClass))
    {
        m_strNetworkStatus =
            "Selected class is reserved; its runtime asset and server profile are not admitted.";
        return false;
    }
    characterInfo.m_strNickName =
        '\0' == m_szNickName[0] ? "Player" : m_szNickName;

    LEVEL targetLevel = LEVEL::END;
    CLIENT_SCENARIO targetScenario = CLIENT_SCENARIO::END;
    if (!Try_ResolveWorldScene(eWorldId, targetLevel, targetScenario))
    {
        m_strNetworkStatus = "Unsupported world selection.";
        return false;
    }
    if (!CClientLaunchOptions::Select_RuntimeEntryMode(m_eEntryMode))
    {
        m_strNetworkStatus = "Invalid entry mode.";
        return false;
    }

    CNetworkManager& networkManager = CNetworkManager::Get();
    m_isEnterRequested = false;
    m_isAwaitingEnterAcceptance = false;
    m_EnterAcceptanceDeadline = {};
    if (CLIENT_ENTRY_MODE::LOCAL_PREVIEW == m_eEntryMode)
    {
        networkManager.Close_ServerConnection();
        if (!CSceneTransitionService::Request(
            targetLevel, targetScenario, "lobby.local-preview"))
        {
            m_strNetworkStatus = CSceneTransitionService::Get_Status();
            return false;
        }

        m_ePendingWorldId = LostArk::Shared::WORLD_ID::END;
        m_isEnterRequested = false;
        m_strNetworkStatus =
            "Entering Local Preview. Network gameplay, skills, damage, and boss authority are disabled.";
        return true;
    }

    if (!CClientLaunchOptions::Set_RuntimeServerEndpoint(
        m_szServerHost,
        static_cast<uint16_t>(m_iServerPort)))
    {
        m_strNetworkStatus = "Invalid Multiplayer server IP or port.";
        return false;
    }

    networkManager.Close_ServerConnection();
    if (!networkManager.Connect_To_Server(
        m_szServerHost,
        static_cast<uint16_t>(m_iServerPort)))
    {
        m_strNetworkStatus =
            "Multiplayer connection failed for " +
            string(m_szServerHost) + ":" +
            to_string(m_iServerPort) + " (WSA " +
            to_string(networkManager.Get_LastErrorCode()) +
            "). Local Preview was not selected automatically.";
        return false;
    }

    if (!networkManager.Send_EnterWorld(
        eWorldId,
        characterInfo.m_eClass,
        characterInfo.m_strNickName))
    {
        m_strNetworkStatus =
            "Send failed. WSA error: " +
            to_string(networkManager.Get_LastErrorCode());
        networkManager.Close_ServerConnection();
        return false;
    }

    m_strNetworkStatus =
        "C2S_ENTER_WORLD sent to " +
        string(m_szServerHost) + ":" +
        to_string(m_iServerPort) + ". Waiting for server approval.";
    m_ePendingWorldId = eWorldId;
    m_isAwaitingEnterAcceptance = true;
    m_EnterAcceptanceDeadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(5);
    return true;
}
void CLevel_Lobby::Update(f32_t fTimeDelta)
{
    LOBBY_ENTER_COMMAND enterCommand;
    if (CLobbyCommandService::Try_Consume(enterCommand))
    {
        if (!Select_Character(enterCommand.iCharacterSlot))
        {
            m_strNetworkStatus = "Invalid character slot.";
        }
        else
        {
            strncpy_s(
                m_szNickName,
                enterCommand.strNickName.c_str(),
                _TRUNCATE);
            m_eEntryMode = enterCommand.eEntryMode;
            if (CLIENT_ENTRY_MODE::MULTIPLAYER == m_eEntryMode)
            {
                strncpy_s(
                    m_szServerHost,
                    enterCommand.strServerHost.c_str(),
                    _TRUNCATE);
                m_iServerPort = enterCommand.iServerPort;
            }
            Request_EnterWorld(enterCommand.eWorldId);
        }
    }

    CNetworkManager& networkManager = CNetworkManager::Get();
    LostArk::Shared::S2C_ENTER_ACCEPTED accepted;
    if (networkManager.Try_Consume_EnterAccepted(accepted))
    {
        if (!m_isAwaitingEnterAcceptance)
        {
            m_strNetworkStatus = "Unexpected server acceptance was rejected.";
            networkManager.Close_ServerConnection();
            m_ePendingWorldId = LostArk::Shared::WORLD_ID::END;
        }
        else if (accepted.eWorldId != m_ePendingWorldId)
        {
            m_strNetworkStatus = "Rejected mismatched world acceptance.";
            m_isAwaitingEnterAcceptance = false;
            networkManager.Close_ServerConnection();
            m_ePendingWorldId = LostArk::Shared::WORLD_ID::END;
        }
        else
        {
            m_strNetworkStatus =
                "S2C_ENTER_ACCEPTED received. PlayerId=" +
                to_string(accepted.iPlayerId) +
                ", NetEntityId=" +
                to_string(accepted.iNetEntityId);
            m_isAwaitingEnterAcceptance = false;
            m_isEnterRequested = true;
        }
    }

    if (m_isAwaitingEnterAcceptance)
    {
        if (!networkManager.Is_Connected())
        {
            m_isAwaitingEnterAcceptance = false;
            m_ePendingWorldId = LostArk::Shared::WORLD_ID::END;
            networkManager.Close_ServerConnection();
            m_strNetworkStatus =
                "Server disconnected before approving entry. Lobby remains active.";
        }
        else
        {
            if (std::chrono::steady_clock::now() >=
                m_EnterAcceptanceDeadline)
            {
                m_isAwaitingEnterAcceptance = false;
                m_ePendingWorldId = LostArk::Shared::WORLD_ID::END;
                networkManager.Close_ServerConnection();
                m_strNetworkStatus =
                    "Server entry approval timed out after 5 seconds. Lobby remains active.";
            }
        }
    }

    // 서버 승인 메시지를 받은 뒤에만 true로 변경한다.
    if (m_isEnterRequested)
    {
        m_isEnterRequested = false;

        LEVEL targetLevel = LEVEL::END;
        CLIENT_SCENARIO targetScenario = CLIENT_SCENARIO::END;
        if (!Try_ResolveWorldScene(
            m_ePendingWorldId,
            targetLevel,
            targetScenario))
        {
            m_strNetworkStatus = "Accepted unknown world.";
            return;
        }
        if (!CSceneTransitionService::Request(
            targetLevel,
            targetScenario,
            "lobby.enter-accepted"))
        {
            m_strNetworkStatus =
                CSceneTransitionService::Get_Status();
            networkManager.Close_ServerConnection();
            m_ePendingWorldId = LostArk::Shared::WORLD_ID::END;
            return;
        }

        m_ePendingWorldId = LostArk::Shared::WORLD_ID::END;
        return;
    }

    __super::Update(fTimeDelta);
}

HRESULT CLevel_Lobby::Render()
{
    Render_CharacterSelectPanel();

    return S_OK;
}
void CLevel_Lobby::Render_CharacterSelectPanel()
{
    const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
    if (nullptr != mainViewport)
    {
        ImGui::SetNextWindowViewport(mainViewport->ID);
        ImGui::SetNextWindowPos(
            ImVec2(
                mainViewport->WorkPos.x + 24.f,
                mainViewport->WorkPos.y + 24.f),
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

    ImGui::Text("Select Character");

    for (size_t i = 0; i < m_vecCharacters.size(); ++i)
    {
        const LobbyCharacterInfo& characterInfo = m_vecCharacters[i];

        const bool_t isRuntimeSupported =
            LostArk::Shared::Is_Supported_Playable_Character_Class(
                characterInfo.m_eClass);
        const string label =
            string(Get_CharacterClassName(characterInfo.m_eClass)) +
            (isRuntimeSupported ? " [Ready]" : " [Asset Pending]") +
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
    const bool_t canEnterWorld =
        hasSelection &&
        LostArk::Shared::Is_Supported_Playable_Character_Class(
            m_vecCharacters[m_iSelectedCharacterIndex].m_eClass);

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

    ImGui::Separator();
    ImGui::Text("Entry Mode");
    if (ImGui::RadioButton("Local Preview", CLIENT_ENTRY_MODE::LOCAL_PREVIEW == m_eEntryMode))
        m_eEntryMode = CLIENT_ENTRY_MODE::LOCAL_PREVIEW;
    ImGui::SameLine();
    if (ImGui::RadioButton("Multiplayer", CLIENT_ENTRY_MODE::MULTIPLAYER == m_eEntryMode))
        m_eEntryMode = CLIENT_ENTRY_MODE::MULTIPLAYER;

    const bool_t isMultiplayer = CLIENT_ENTRY_MODE::MULTIPLAYER == m_eEntryMode;
    ImGui::BeginDisabled(!isMultiplayer);
    ImGui::InputText("Server IP", m_szServerHost, sizeof(m_szServerHost));
    ImGui::InputInt("Server Port", &m_iServerPort);
    ImGui::EndDisabled();
    ImGui::TextDisabled(isMultiplayer ?
        "Connects only after this IP:port accepts the session." :
        "No socket. Spawns one presentation-only local character.");

    const bool_t hasValidEndpoint = !isMultiplayer ||
        ('\0' != m_szServerHost[0] && m_iServerPort > 0 && m_iServerPort <= UINT16_MAX);
    ImGui::BeginDisabled(!canEnterWorld || !hasValidEndpoint);

    if (ImGui::Button("Enter Bern"))
    {
        const string nickName =
            '\0' == m_szNickName[0] ? "Player" : m_szNickName;
        CLobbyCommandService::Request_EnterWorld(
            m_iSelectedCharacterIndex,
            LostArk::Shared::WORLD_ID::BERN,
            nickName,
            m_eEntryMode,
            m_szServerHost,
            static_cast<uint16_t>(m_iServerPort));
    }

    ImGui::SameLine();
    if (ImGui::Button("Enter Valtan"))
    {
        const string nickName =
            '\0' == m_szNickName[0] ? "Player" : m_szNickName;
        CLobbyCommandService::Request_EnterWorld(
            m_iSelectedCharacterIndex,
            LostArk::Shared::WORLD_ID::VALTAN_ARENA,
            nickName,
            m_eEntryMode,
            m_szServerHost,
            static_cast<uint16_t>(m_iServerPort));
    }

    ImGui::SameLine();
    if (ImGui::Button("Enter Training"))
    {
        const string nickName =
            '\0' == m_szNickName[0] ? "Player" : m_szNickName;
        CLobbyCommandService::Request_EnterWorld(
            m_iSelectedCharacterIndex,
            LostArk::Shared::WORLD_ID::TRAINING_GROUND,
            nickName,
            m_eEntryMode,
            m_szServerHost,
            static_cast<uint16_t>(m_iServerPort));
    }
    ImGui::EndDisabled();

    if (!hasSelection)
    {
        ImGui::TextDisabled(
            "Select a character first.");
    }
    else if (!canEnterWorld)
    {
        ImGui::TextDisabled(
            "This class needs its resource pack, CharacterSpec, Loader prototypes, and Server player profile before entry.");
    }

    ImGui::TextWrapped("%s", m_strNetworkStatus.c_str());

    ImGui::End();
}

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

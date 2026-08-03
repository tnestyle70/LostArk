#ifdef _DEBUG
#include "imgui.h"
#endif

#ifndef _DEBUG
#include "imgui.h"
#endif

#include "NetworkManager.h"
#include "NetworkPlayerCommandSink.h"
#include "ClientLaunchOptions.h"
#include "MainApp.h"
#include "GameInstance.h"
#include "Profiler.h"
#include "LobbyCommandService.h"
#include "RuntimeAssetRoot.h"
#include "SceneTransitionService.h"
#include "Loader.h"
#include "LevelCatalog.h"
#include "Camera_Free.h"
#include "Character.h"
#include "CharacterCatalog.h"
#include "CombatHUDViewModel.h"
#include "OfflinePlayerPreview.h"
#include "Valtan.h"

#include "Level_Loading.h"

#include "ImGuiLayer.h"
#ifdef _DEBUG
#include "ProfilerCaptureIO.h"
#include "MapTool.h"
#include "Animation_Tool.h"
#include "Effect_Tool.h"
#include "HUDLayoutTool.h"
#endif

#include <algorithm>
#include <fstream>

namespace
{
#ifdef _DEBUG
    bool_t IsWindowOwnedByCurrentProcess(HWND hWnd)
    {
        if (nullptr == hWnd)
            return false;

        DWORD dwProcessId = {};
        if (0 == GetWindowThreadProcessId(hWnd, &dwProcessId))
            return false;

        return GetCurrentProcessId() == dwProcessId;
    }
#endif

    const char_t* GetCharacterClassDisplayName(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (characterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER: return "Lance Master";
		case CHARACTER_CLASS_ID::GUNSLINGER: return "Gunslinger";
		case CHARACTER_CLASS_ID::SLAYER: return "Slayer";
		case CHARACTER_CLASS_ID::ARTIST: return "Artist";
		default: return "Unknown";
		}
	}
}

Client::CMainApp::CMainApp()
{
    /* wireframe or solid, cullmode -> cw, ccw */
   // D3D11_RASTERIZER_DESC           RSDesc{};
   // ComPtr<ID3D11RasterizerState>    pRSState = {};

   // if (FAILED(m_pDevice->CreateRasterizerState(&RSDesc, &pRSState)))
   //     return;

    
   // m_pContext->RSSetState(pRSState.Get());

   // /* 깊이 비교 x or o, 깊이 기록 x or o */
   // D3D11_DEPTH_STENCIL_DESC
   // m_pContext->OMSetDepthStencilState();

   // /* 블렌딩에 대한 설정. */
   // D3D11_BLEND_DESC
   // m_pContext->OMSetBlendState();
   // 
   // /*
   // m_pDevice->SetSamplerState(D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
   // m_pDevice->SetSamplerState(D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
   // m_pDevice->SetSamplerState(D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);*/

   ////D3D11_SAMPLER_DESC

   // //D3D11_SAMPLER_DESC      SamplerDesc{};
   // //ID3D11SamplerState* pSamplerState = {};

   // //m_pDevice->CreateSamplerState(&SamplerDesc, &pSamplerState);
}

Client::CMainApp::~CMainApp()
{
    Free();
}

HRESULT CMainApp::Initialize()
{
    ENGINE_DESC         EngineDesc{};
    EngineDesc.hInstance = g_hInst;
    EngineDesc.hWnd = g_hWnd;
    EngineDesc.eWinMode = WINMODE::WIN;
    EngineDesc.iNumLevels = ETOUI(LEVEL::END);
    EngineDesc.iWinSizeX = g_iWinSizeX;
    EngineDesc.iWinSizeY = g_iWinSizeY;

    if (FAILED(CGameInstance::Get().Initialize_Engine(EngineDesc, m_pDevice, m_pContext)))
        return E_FAIL;

    //NetworkManager
    if (!CNetworkManager::Get().Initialize())
    {
        return E_FAIL;
    }

    if (FAILED(CClientLaunchOptions::Initialize()))
    {
        MessageBoxW(
            g_hWnd,
            CClientLaunchOptions::Get_Error().c_str(),
            L"Invalid client launch options",
            MB_OK | MB_ICONERROR);
        return E_INVALIDARG;
    }

    m_eSmokeScenario = CClientLaunchOptions::Get().eScenario;
    m_SmokeStartTime = std::chrono::steady_clock::now();

    if (FAILED(ReadyImGuiRuntime()))
        return E_FAIL;

#ifdef _DEBUG
    if (FAILED(ReadyDebugTools()))
        return E_FAIL;
#endif

    if (FAILED(Ready_Fonts()))
        return E_FAIL;

    if (FAILED(Ready_Prototype_For_Static()))
        return E_FAIL;

    if (FAILED(Start_Level(LEVEL::LOBBY)))
        return E_FAIL;

    return S_OK;
}

void CMainApp::Update(f32_t fTimeDelta)
{
#ifdef _DEBUG
    UpdateDebugToolShortcut();
#endif

    if (nullptr != m_pImGuiLayer)
        m_pImGuiLayer->BeginFrame();

#ifdef _DEBUG
    const bool_t bMapToolOpen = m_bDeveloperToolsVisible &&
        nullptr != m_pMapTool && m_pMapTool->IsOpen();
    const HWND hForegroundWindow = GetForegroundWindow();
    const bool_t bExternalToolFocused = bMapToolOpen &&
        nullptr != hForegroundWindow &&
        hForegroundWindow != g_hWnd &&
        IsWindowOwnedByCurrentProcess(hForegroundWindow);

    const bool_t bWorldLeftMouseConsumed =
        nullptr != m_pMapTool &&
        m_pMapTool->ConsumesWorldLeftMouse();
#else
    constexpr bool_t bExternalToolFocused = false;
    constexpr bool_t bWorldLeftMouseConsumed = false;
#endif

    const bool_t bKeyboardCaptured =
        nullptr != m_pImGuiLayer &&
        (m_pImGuiLayer->WantsCaptureKeyboard() || bExternalToolFocused);
    const bool_t bMouseCapturedByUi =
        nullptr != m_pImGuiLayer &&
        (m_pImGuiLayer->WantsCaptureMouse() || bExternalToolFocused);

    CGameInstance::Get().SetInputBlocked(
        bKeyboardCaptured,
        bMouseCapturedByUi);
    CGameInstance::Get().SetMouseButtonBlocked(
        DIM::LB,
        bWorldLeftMouseConsumed);

    CNetworkManager::Get().Update();

    Apply_PendingSceneTransition();

    CGameInstance::Get().Update_Engine(fTimeDelta);

    UpdateSmokeHarness();

#ifdef _DEBUG
    if (nullptr != m_pMapTool)
        m_pMapTool->Update(fTimeDelta);
#endif
}

HRESULT CMainApp::Render()
{
	float4_t        vClearColor = { 0.008f, 0.012f, 0.025f, 1.f };

    if (FAILED(CGameInstance::Get().Render_Begin(&vClearColor)))
    {
        if (nullptr != m_pImGuiLayer)
            m_pImGuiLayer->CancelFrame();
        return E_FAIL;
    }

    if (FAILED(CGameInstance::Get().Render()))
    {
        if (nullptr != m_pImGuiLayer)
            m_pImGuiLayer->CancelFrame();
        return E_FAIL;
    }

    if (nullptr != m_pImGuiLayer)
    {
		RenderCombatHUD();
		RenderOfflinePreviewOverlay();

#ifdef _DEBUG
        if (m_bDeveloperToolsVisible)
        {
            RenderDeveloperTools();

            if (DEBUG_TOOL::MAP == m_eActiveDebugTool &&
                nullptr != m_pMapTool)
                m_pMapTool->Render();
            else if (DEBUG_TOOL::EFFECT == m_eActiveDebugTool &&
                nullptr != m_pEffectTool)
                m_pEffectTool->Render();
            else if (DEBUG_TOOL::ANIMATION == m_eActiveDebugTool &&
                nullptr != m_pAnimationTool)
                m_pAnimationTool->Render();
            else if (DEBUG_TOOL::UI == m_eActiveDebugTool &&
                nullptr != m_pHUDLayoutTool)
                m_pHUDLayoutTool->Render();

            if (m_bProfilerVisible)
            {
                RenderProfilerOverlay();
                RenderProfilerSettings();
            }
        }
#endif

        m_pImGuiLayer->EndFrame();
    }

    if (FAILED(CGameInstance::Get().Render_End()))
        return E_FAIL;

    return S_OK;
}

void CMainApp::RenderCombatHUD()
{
	const uint32_t currentLevel =
		CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::BERN) &&
		currentLevel != ETOUI(LEVEL::VALTAN_ARENA) &&
		currentLevel != ETOUI(LEVEL::DEVELOPMENT))
	{
		return;
	}

	const Client::HUD_PLAYER_STATE& player =
		Client::CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid ||
		0u == player.iMaximumHp ||
		0u == player.iMaximumResource)
	{
		return;
	}

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowPos(
		ImVec2(viewport->WorkPos.x + 20.f,
			viewport->WorkPos.y + viewport->WorkSize.y - 20.f),
		ImGuiCond_Always,
		ImVec2(0.f, 1.f));
	ImGui::SetNextWindowBgAlpha(0.78f);

	constexpr ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoInputs;

	if (ImGui::Begin("##RuntimeCombatHUD", nullptr, flags))
	{
		ImGui::Text("%s", GetCharacterClassDisplayName(
			player.eCharacterClass));
		ImGui::Text("HP  %u / %u", player.iCurrentHp, player.iMaximumHp);
		ImGui::ProgressBar(
			static_cast<float>(player.iCurrentHp) /
			static_cast<float>(player.iMaximumHp),
			ImVec2(280.f, 0.f));
		ImGui::Text(
			"Resource  %u / %u",
			player.iCurrentResource,
			player.iMaximumResource);
		ImGui::ProgressBar(
			static_cast<float>(player.iCurrentResource) /
			static_cast<float>(player.iMaximumResource),
			ImVec2(280.f, 0.f));

		if (player.Skills.empty())
		{
			ImGui::TextDisabled("No server skill bindings for this class.");
		}
		else
		{
			for (const Client::HUD_SKILL_STATE& skill : player.Skills)
			{
				const std::uint32_t remainingTicks =
					skill.iCooldownEndTick > player.iServerTick ?
					skill.iCooldownEndTick - player.iServerTick : 0u;
				if (0u == remainingTicks)
				{
					ImGui::Text(
						"[%s] %s  READY  DMG %u",
						skill.strInputSlot.c_str(),
						skill.strDisplayName.c_str(),
						skill.iDamage);
				}
				else
				{
					ImGui::Text(
						"[%s] %s  %.1fs  DMG %u",
						skill.strInputSlot.c_str(),
						skill.strDisplayName.c_str(),
						static_cast<float>(remainingTicks) / 30.f,
						skill.iDamage);
				}
			}
		}
	}
	ImGui::End();
}

void CMainApp::RenderOfflinePreviewOverlay()
{
	const CLIENT_LAUNCH_OPTIONS& options = CClientLaunchOptions::Get();
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	const bool_t isPlayerStageScenario =
		CLIENT_SCENARIO::WORLD_BERN == options.eScenario ||
		CLIENT_SCENARIO::RAID_VALTAN_ARENA == options.eScenario ||
		CLIENT_SCENARIO::DEVELOPMENT_TRAINING_GROUND == options.eScenario;
	if (!options.isOfflinePreview ||
		!isPlayerStageScenario ||
		ETOUI(LEVEL::LOBBY) == currentLevel ||
		ETOUI(LEVEL::LOADING) == currentLevel ||
		COfflinePlayerPreview::Get_ActivePlacementId().empty())
	{
		return;
	}

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowPos(
		ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 20.f,
			viewport->WorkPos.y + 20.f),
		ImGuiCond_Always,
		ImVec2(1.f, 0.f));
	ImGui::SetNextWindowBgAlpha(0.82f);
	constexpr ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoInputs;
	if (ImGui::Begin("##OfflinePreviewStatus", nullptr, flags))
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.72f, 0.18f, 1.f),
			"LOCAL PREVIEW");
		ImGui::TextDisabled("Server gameplay is disabled");
		ImGui::TextDisabled("Spawn: %s",
			COfflinePlayerPreview::Get_ActivePlacementId().c_str());
	}
	ImGui::End();
}

HRESULT CMainApp::Ready_Fonts()
{
    /*
MakeSpriteFont "넥슨lv1고딕 Bold" /FontSize:20 /FastPack /CharacterRegion:0x0020-0x00FF /CharacterRegion:0x3131-0x3163 /CharacterRegion:0xAC00-0xD800 /DefaultCharacter:0xAC00 161ex.spritefont
*/


    const filesystem::path fontPath =
        CRuntimeAssetRoot::Resolve_Font(L"161ex.spritefont");
    if (fontPath.empty() ||
        FAILED(CGameInstance::Get().Add_Font(
            TEXT("Font_Default"),
            fontPath.c_str())))
        return E_FAIL;
       

    return S_OK;
}

HRESULT CMainApp::Ready_Prototype_For_Static()
{
    /* For.Prototype_Component_Shader_VtxTex */

    if(FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxTex"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxTex.hlsl"), VTXTEX::Elements, VTXTEX::iNumElements))))
        return E_FAIL;

    /* For.Prototype_Component_VIBuffer_Rect */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect"),
        CVIBuffer_Rect::Create(m_pDevice, m_pContext))))
        return E_FAIL;



    return S_OK;
}

HRESULT CMainApp::Start_Level(LEVEL eStartLevelID)
{
    if (FAILED(CGameInstance::Get().Change_Level(ETOUI(LEVEL::LOADING),
        CLevel_Loading::Create(m_pDevice, m_pContext, eStartLevelID))))
        return E_FAIL;

    return S_OK;
}

void CMainApp::Apply_PendingSceneTransition()
{
    SCENE_TRANSITION_REQUEST request;
    if (!CSceneTransitionService::Try_Consume(request))
        return;

    if (ETOUI(LEVEL::LOADING) ==
        CGameInstance::Get().Get_CurrentLevelID())
    {
        OutputDebugStringA(
            "[SceneTransition] Rejected while loading is active.\n");
#ifdef _DEBUG
        m_strSceneTransitionStatus =
            "Transition rejected while Loading is active.";
#endif
        return;
    }

    if (!CClientLaunchOptions::Select_RuntimeScenario(
        request.eScenario) ||
        FAILED(Start_Level(request.eTargetLevel)))
    {
        OutputDebugStringA(
            "[SceneTransition] Failed to enter Loading.\n");
#ifdef _DEBUG
        m_strSceneTransitionStatus =
            "Transition failed before Loading could start.";
#endif
        return;
    }

#ifdef _DEBUG
    m_strSceneTransitionStatus =
        "Loading request accepted from " + request.strSource + ".";
#endif
}

void CMainApp::UpdateSmokeHarness()
{
    const CLIENT_LAUNCH_OPTIONS& options =
        CClientLaunchOptions::Get();
    if (!options.isSmokeRun || m_isSmokeComplete)
        return;

    HRESULT loadFailure = S_OK;
    if (CSceneTransitionService::Try_ConsumeLoadFailure(loadFailure))
    {
        CompleteSmokeHarness(false, "level-load-failed");
        return;
    }

    const auto elapsed = std::chrono::duration_cast<
        std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - m_SmokeStartTime);
    if (elapsed.count() > options.iTimeoutMs)
    {
        CompleteSmokeHarness(false, "timeout");
        return;
    }

    const uint32_t currentLevel =
        CGameInstance::Get().Get_CurrentLevelID();

    LEVEL targetLevel = LEVEL::LOBBY;
    switch (m_eSmokeScenario)
    {
    case CLIENT_SCENARIO::WORLD_BERN:
        targetLevel = LEVEL::BERN;
        break;
    case CLIENT_SCENARIO::RAID_VALTAN_ARENA:
        targetLevel = LEVEL::VALTAN_ARENA;
        break;
    case CLIENT_SCENARIO::DEVELOPMENT_TRAINING_GROUND:
    case CLIENT_SCENARIO::DEVELOPMENT_MAP:
    case CLIENT_SCENARIO::DEVELOPMENT_CHARACTER:
    case CLIENT_SCENARIO::DEVELOPMENT_HDR:
    case CLIENT_SCENARIO::DEVELOPMENT_EFFECT:
    case CLIENT_SCENARIO::DEVELOPMENT_UI:
        targetLevel = LEVEL::DEVELOPMENT;
        break;
    default:
        break;
    }
	const bool_t isPlayerStageScenario =
		CLIENT_SCENARIO::WORLD_BERN == m_eSmokeScenario ||
		CLIENT_SCENARIO::RAID_VALTAN_ARENA == m_eSmokeScenario ||
		CLIENT_SCENARIO::DEVELOPMENT_TRAINING_GROUND == m_eSmokeScenario;

    if (m_isSmokeDispatched)
    {
		if (options.isEnterApprovalTimeoutSmoke)
		{
			if (ETOUI(LEVEL::LOBBY) != currentLevel)
				return;
			if (CNetworkManager::Get().Is_Connected())
			{
				m_hasSmokeObservedPendingEnterConnection = true;
				return;
			}
			if (m_hasSmokeObservedPendingEnterConnection)
			{
				CompleteSmokeHarness(
					true,
					"network-enter-approval-timeout-returned-to-lobby");
			}
			return;
		}

		if (options.isDisconnectRecoverySmoke &&
			m_hasSmokeObservedNetworkStage &&
			ETOUI(LEVEL::LOBBY) == currentLevel &&
			!CNetworkManager::Get().Is_Connected())
		{
			CompleteSmokeHarness(
				true,
				"network-disconnect-returned-to-lobby");
			return;
		}

		if (ETOUI(targetLevel) != currentLevel)
			return;

		if (options.isOfflinePreview && isPlayerStageScenario)
		{
			const shared_ptr<CCharacter> localCharacter =
				dynamic_pointer_cast<CCharacter>(
					CGameInstance::Get().Get_GameObject(
						currentLevel,
						TEXT("Layer_Player"),
						0));
			const shared_ptr<CCamera_Free> camera =
				dynamic_pointer_cast<CCamera_Free>(
					CGameInstance::Get().Get_GameObject(
						currentLevel,
						TEXT("Layer_Camera"),
						0));
			if (CNetworkManager::Get().Is_Connected() ||
				0u != CNetworkPlayerCommandSink::Get_LiveInstanceCount() ||
				nullptr == localCharacter ||
				!localCharacter->Is_LocallyControlled() ||
				!options.SelectedCharacterClass.has_value() ||
				localCharacter->Get_Spec() !=
					CCharacterCatalog::Find_Spec(
						*options.SelectedCharacterClass) ||
				!COfflinePlayerPreview::Matches_ActiveCharacter(
					localCharacter) ||
				COfflinePlayerPreview::Get_ActivePlacementId().empty() ||
				nullptr == camera ||
				!camera->Is_FollowEnabled() ||
				camera->Get_FollowTarget() != localCharacter->Get_Transform())
			{
				return;
			}
			if (CLIENT_SCENARIO::RAID_VALTAN_ARENA == m_eSmokeScenario &&
				nullptr != CGameInstance::Get().Get_GameObject(
					currentLevel,
					TEXT("Layer_WorldEntity"),
					0))
			{
				CompleteSmokeHarness(false, "offline-created-network-world-entity");
				return;
			}
			CompleteSmokeHarness(
				true,
				"offline-stage-local-player-and-camera-ready");
			return;
		}

		if (isPlayerStageScenario)
		{
			const shared_ptr<CCharacter> localCharacter =
				dynamic_pointer_cast<CCharacter>(
					CGameInstance::Get().Get_GameObject(
						currentLevel,
						TEXT("Layer_Player"),
						0));
			if (nullptr == localCharacter)
				return;
			if (options.isDisconnectRecoverySmoke)
			{
				m_hasSmokeObservedNetworkStage = true;
				return;
			}

			if (CLIENT_SCENARIO::DEVELOPMENT_TRAINING_GROUND == m_eSmokeScenario)
			{
				static bool_t isTrainingActionDispatched = false;
				if (!isTrainingActionDispatched)
				{
					if (!CNetworkManager::Get().Send_UseSkill(
						1u, 34060u, 0.f, 0.f))
					{
						CompleteSmokeHarness(false, "training-action-send-failed");
						return;
					}
					isTrainingActionDispatched = true;
					return;
				}

				const Client::HUD_PLAYER_STATE& playerState =
					Client::CCombatHUDViewModel::Get().Get_Player();
				const auto skill = std::find_if(
					playerState.Skills.begin(),
					playerState.Skills.end(),
					[](const Client::HUD_SKILL_STATE& state)
					{
						return 34060u == state.iSkillId;
					});
				if (!playerState.isValid ||
					playerState.Skills.end() == skill ||
					skill->iCooldownEndTick <= playerState.iServerTick)
				{
					return;
				}
				CompleteSmokeHarness(
					true,
					"map-player-action-cooldown-and-hud-ready");
				return;
			}

			if (CLIENT_SCENARIO::RAID_VALTAN_ARENA == m_eSmokeScenario)
			{
				const shared_ptr<CValtan> valtan =
					dynamic_pointer_cast<CValtan>(
						CGameInstance::Get().Get_GameObject(
							currentLevel,
							TEXT("Layer_WorldEntity"),
							0));
				if (nullptr == valtan ||
					valtan->Get_ServerActionId() !=
						"valtan.attack.basic-swing" ||
					(CValtan::VALTAN_STATE::PATTERN_WINDUP != valtan->Get_State() &&
					 CValtan::VALTAN_STATE::PATTERN_ACTIVE != valtan->Get_State() &&
					 CValtan::VALTAN_STATE::PATTERN_RECOVERY != valtan->Get_State()))
				{
					return;
				}
			}

			CompleteSmokeHarness(
				true,
				CLIENT_SCENARIO::WORLD_BERN == m_eSmokeScenario ?
				"map-and-player-replication-ready" :
				"map-player-and-valtan-action-ready");
			return;
		}

#ifdef _DEBUG
		bool_t isToolReady = false;
		switch (m_eSmokeScenario)
		{
		case CLIENT_SCENARIO::DEVELOPMENT_MAP:
			isToolReady = nullptr != m_pMapTool;
			break;
		case CLIENT_SCENARIO::DEVELOPMENT_CHARACTER:
			isToolReady = nullptr != m_pAnimationTool;
			break;
		case CLIENT_SCENARIO::DEVELOPMENT_HDR:
		case CLIENT_SCENARIO::DEVELOPMENT_EFFECT:
			isToolReady = nullptr != m_pEffectTool;
			break;
		case CLIENT_SCENARIO::DEVELOPMENT_UI:
			isToolReady = nullptr != m_pHUDLayoutTool;
			break;
		default:
			break;
		}
		if (isToolReady)
			CompleteSmokeHarness(true, "development-level-and-tool-ready");
#else
		CompleteSmokeHarness(false, "developer-tools-not-shipped-in-release");
#endif
        return;
    }

    if (ETOUI(LEVEL::LOBBY) != currentLevel)
        return;

    if (CLIENT_SCENARIO::FRONT_LOBBY == m_eSmokeScenario)
    {
        CompleteSmokeHarness(true, "lobby-active");
        return;
    }

    if (isPlayerStageScenario)
    {
        LostArk::Shared::WORLD_ID worldId =
            LostArk::Shared::WORLD_ID::END;
        if (CLIENT_SCENARIO::WORLD_BERN == m_eSmokeScenario)
            worldId = LostArk::Shared::WORLD_ID::BERN;
        else if (CLIENT_SCENARIO::RAID_VALTAN_ARENA == m_eSmokeScenario)
            worldId = LostArk::Shared::WORLD_ID::VALTAN_ARENA;
        else
            worldId = LostArk::Shared::WORLD_ID::TRAINING_GROUND;
        m_isSmokeDispatched =
            CLobbyCommandService::Request_EnterWorld(
                0,
                worldId,
				"SmokeClient",
				options.eEntryMode,
				options.strServerHost,
				options.iServerPort);
    }
    else
    {
        if (!CClientLaunchOptions::Select_RuntimeScenario(
            m_eSmokeScenario))
        {
            CompleteSmokeHarness(false, "invalid-smoke-scenario");
            return;
        }
#ifdef _DEBUG
        if (FAILED(EnsureDebugTool(m_eSmokeScenario)))
        {
            CompleteSmokeHarness(false, "debug-tool-init-failed");
            return;
        }
#endif
        m_isSmokeDispatched = CSceneTransitionService::Request(
            LEVEL::DEVELOPMENT,
            m_eSmokeScenario,
            "smoke-harness");
    }

    if (!m_isSmokeDispatched)
        CompleteSmokeHarness(false, "dispatch-failed");
}

void CMainApp::CompleteSmokeHarness(
    const bool_t succeeded,
    const char_t* pReason)
{
    if (m_isSmokeComplete)
        return;
    m_isSmokeComplete = true;

    const CLIENT_LAUNCH_OPTIONS& options =
        CClientLaunchOptions::Get();
    const auto elapsed = std::chrono::duration_cast<
        std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - m_SmokeStartTime);
	const uint32_t currentLevel =
		CGameInstance::Get().Get_CurrentLevelID();
	const shared_ptr<CCharacter> localCharacter =
		dynamic_pointer_cast<CCharacter>(
			CGameInstance::Get().Get_GameObject(
				currentLevel,
				TEXT("Layer_Player"),
				0));
	const shared_ptr<CCamera_Free> camera =
		dynamic_pointer_cast<CCamera_Free>(
			CGameInstance::Get().Get_GameObject(
				currentLevel,
				TEXT("Layer_Camera"),
				0));
	const bool_t isCameraFollowReady =
		nullptr != localCharacter &&
		nullptr != camera &&
		camera->Is_FollowEnabled() &&
		camera->Get_FollowTarget() == localCharacter->Get_Transform();
	const LEVEL_CATALOG_ENTRY* pRequestedSmokeEntry =
		CLevelCatalog::Find(m_eSmokeScenario);
	const std::string requestedSmokeScenarioId =
		nullptr == pRequestedSmokeEntry ?
		"unknown" : pRequestedSmokeEntry->strStableId;

    if (options.ReportPath.has_value())
    {
        const std::filesystem::path reportPath =
            options.ReportPath->lexically_normal();
        std::error_code error;
        if (!reportPath.parent_path().empty())
        {
            std::filesystem::create_directories(
                reportPath.parent_path(),
                error);
        }

        if (!error)
        {
            std::filesystem::path temporaryPath = reportPath;
            temporaryPath += L".tmp";
            std::ofstream output(
                temporaryPath,
                std::ios::binary | std::ios::trunc);
            if (output.is_open())
            {
                output
                    << "{\n"
                    << "  \"schema\": \"lostark.client-smoke\",\n"
                    << "  \"version\": 2,\n"
                    << "  \"scenarioId\": \""
                    << options.strScenarioId << "\",\n"
					<< "  \"requestedScenarioId\": \""
					<< requestedSmokeScenarioId << "\",\n"
                    << "  \"status\": \""
                    << (succeeded ? "passed" : "failed") << "\",\n"
                    << "  \"reason\": \""
                    << (nullptr == pReason ? "unknown" : pReason)
                    << "\",\n"
                    << "  \"elapsedMs\": " << elapsed.count() << ",\n"
					<< "  \"networkConnected\": "
					<< (CNetworkManager::Get().Is_Connected() ? "true" : "false")
					<< ",\n"
					<< "  \"networkCommandSinkLiveCount\": "
					<< CNetworkPlayerCommandSink::Get_LiveInstanceCount()
					<< ",\n"
					<< "  \"selectedCharacterClassId\": ";
				if (options.SelectedCharacterClass.has_value())
				{
					output << static_cast<uint32_t>(
						*options.SelectedCharacterClass);
				}
				else
				{
					output << "null";
				}
				output
					<< ",\n"
					<< "  \"localPreviewPlacementId\": \""
					<< COfflinePlayerPreview::Get_ActivePlacementId()
					<< "\",\n"
					<< "  \"cameraFollowReady\": "
					<< (isCameraFollowReady ? "true" : "false")
					<< ",\n"
                    << "  \"currentLevelId\": "
                    << CGameInstance::Get().Get_CurrentLevelID() << ",\n"
                    << "  \"loadingStage\": \""
                    << CLoader::Get_ActiveStatus() << "\"\n"
                    << "}\n";
                output.close();

                MoveFileExW(
                    temporaryPath.c_str(),
                    reportPath.c_str(),
                    MOVEFILE_REPLACE_EXISTING |
                    MOVEFILE_WRITE_THROUGH);
            }
        }
    }

    OutputDebugStringA(
        succeeded ?
        "[Smoke] PASS\n" :
        "[Smoke] FAIL\n");
    PostMessageW(g_hWnd, WM_CLOSE, 0, 0);
}

HRESULT CMainApp::ReadyImGuiRuntime()
{
    m_pImGuiLayer = std::make_unique<Engine::CImGuiLayer>();
    if (!m_pImGuiLayer->Initialize(g_hWnd, m_pDevice.Get(), m_pContext.Get()))
    {
        OutputDebugStringA("[ImGui] Failed to initialize Win32/DX11 runtime.\n");
        return E_FAIL;
    }

    return S_OK;
}

#ifdef _DEBUG
HRESULT CMainApp::ReadyDebugTools()
{
    const CLIENT_LAUNCH_OPTIONS& launchOptions =
        CClientLaunchOptions::Get();

    const bool_t isEffectProfileRequested =
        launchOptions.isEffectProfile;

    if (Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler())
    {
        pProfiler->Reset_History();
        pProfiler->Set_Enabled(isEffectProfileRequested);
    }
    m_bProfilerVisible = isEffectProfileRequested;

    return S_OK;
}

HRESULT CMainApp::EnsureDebugTool(
    const CLIENT_SCENARIO eScenario)
{
	const LEVEL_CATALOG_ENTRY* pEntry = CLevelCatalog::Find(eScenario);
	if (nullptr == pEntry || pEntry->Tools.size() != 1u)
		return E_INVALIDARG;
	const std::string& tool = pEntry->Tools.front();

	if (tool == "MapTool")
    {
        if (nullptr == m_pMapTool)
        {
            auto mapTool = std::make_unique<CMapTool>();
            if (FAILED(mapTool->Initialize(m_pDevice, m_pContext)))
                return E_FAIL;
            m_pMapTool = std::move(mapTool);
        }
        m_pMapTool->SetOpen(true);
        m_eActiveDebugTool = DEBUG_TOOL::MAP;
        return S_OK;
    }
	if (tool == "AnimationTool")
	{
        if (nullptr == m_pAnimationTool)
            m_pAnimationTool = std::make_unique<CAnimation_Tool>();
        m_eActiveDebugTool = DEBUG_TOOL::ANIMATION;
        return S_OK;
    }
	if (tool == "EffectTool")
	{
        if (nullptr == m_pEffectTool)
            m_pEffectTool = std::make_unique<CEffect_Tool>(m_pDevice);
        m_eActiveDebugTool = DEBUG_TOOL::EFFECT;
        return S_OK;
    }
	if (tool == "HUDLayoutTool")
	{
        if (nullptr == m_pHUDLayoutTool)
        {
            m_pHUDLayoutTool = std::make_unique<CHUDLayoutTool>(
                m_pDevice,
                m_pContext);
        }
        m_eActiveDebugTool = DEBUG_TOOL::UI;
        return S_OK;
    }

	m_eActiveDebugTool = DEBUG_TOOL::NONE;
	return E_INVALIDARG;
}

void CMainApp::RenderDeveloperTools()
{
    if (!ImGui::Begin(
        "LostArk Developer Tools",
        &m_bDeveloperToolsVisible,
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::End();
        return;
    }

    const uint32_t currentLevelId =
        CGameInstance::Get().Get_CurrentLevelID();
    const bool_t transitionBlocked =
        ETOUI(LEVEL::LOADING) == currentLevelId ||
        CSceneTransitionService::Is_Pending();

    ImGui::Text("Current level id: %u", currentLevelId);
    ImGui::TextWrapped("%s", m_strSceneTransitionStatus.c_str());
    ImGui::SeparatorText("Local scene previews (server authority bypassed)");

    const auto RequestScene = [this, transitionBlocked](
        const char_t* pLabel,
        const LEVEL eLevel,
        const CLIENT_SCENARIO eScenario)
    {
        ImGui::BeginDisabled(transitionBlocked);
        if (ImGui::Button(pLabel))
        {
            if (!CClientLaunchOptions::Select_RuntimeScenario(eScenario))
            {
                m_strSceneTransitionStatus =
                    "Scenario selection was rejected.";
            }
            else if (FAILED(EnsureDebugTool(eScenario)))
            {
                m_strSceneTransitionStatus =
                    "The selected tool could not be initialized.";
            }
            else if (!CSceneTransitionService::Request(
                eLevel,
                eScenario,
                "imgui.scene-navigator"))
            {
                m_strSceneTransitionStatus =
                    CSceneTransitionService::Get_Status();
            }
            else
            {
                m_strSceneTransitionStatus =
                    "Scene transition staged. It will enter Loading next frame.";
            }
        }
        ImGui::EndDisabled();
    };

    RequestScene(
        "Lobby##SceneLobby",
        LEVEL::LOBBY,
        CLIENT_SCENARIO::FRONT_LOBBY);
    ImGui::SameLine();
    RequestScene(
        "Bern##SceneBern",
        LEVEL::BERN,
        CLIENT_SCENARIO::WORLD_BERN);
    ImGui::SameLine();
    RequestScene(
        "Valtan Arena##SceneValtan",
        LEVEL::VALTAN_ARENA,
        CLIENT_SCENARIO::RAID_VALTAN_ARENA);

    ImGui::SeparatorText("Development scenes");
    RequestScene(
        "Map Tool##SceneMapTool",
        LEVEL::DEVELOPMENT,
        CLIENT_SCENARIO::DEVELOPMENT_MAP);
    ImGui::SameLine();
    RequestScene(
        "Character Animation##SceneAnimation",
        LEVEL::DEVELOPMENT,
        CLIENT_SCENARIO::DEVELOPMENT_CHARACTER);
    RequestScene(
        "Effect Preview##SceneEffect",
        LEVEL::DEVELOPMENT,
        CLIENT_SCENARIO::DEVELOPMENT_EFFECT);
    ImGui::SameLine();
    RequestScene(
        "HUD Layout##SceneHud",
        LEVEL::DEVELOPMENT,
        CLIENT_SCENARIO::DEVELOPMENT_UI);
    ImGui::SameLine();
    RequestScene(
        "HDR Readback##SceneHdr",
        LEVEL::DEVELOPMENT,
        CLIENT_SCENARIO::DEVELOPMENT_HDR);

    ImGui::SeparatorText("Diagnostics");
    bool profilerVisible = m_bProfilerVisible;
    if (ImGui::Checkbox("Profiler", &profilerVisible))
    {
        m_bProfilerVisible = profilerVisible;
        if (Engine::CProfiler* pProfiler =
            CGameInstance::Get().Get_Profiler())
        {
            if (m_bProfilerVisible)
                pProfiler->Reset_History();
            pProfiler->Set_Enabled(m_bProfilerVisible);
        }
    }
    ImGui::TextDisabled(
        "F1: Developer Tools  |  F6: Follow/Free Camera");

    ImGui::End();
}

void CMainApp::RenderProfilerOverlay()
{
    if (!m_bProfilerVisible)
        return;

    Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler();
    if (nullptr == pProfiler)
        return;

    Engine::FProfilerLiveStats stats{};
    const bool_t bHasStats = pProfiler->Get_LiveStats(stats);
    const ImGuiIO& io = ImGui::GetIO();
	const ImGuiViewport* pViewport = ImGui::GetMainViewport();

	ImGui::SetNextWindowViewport(pViewport->ID);
	ImGui::SetNextWindowPos(
		ImVec2(pViewport->WorkPos.x + 10.f, pViewport->WorkPos.y + 30.f),
		ImGuiCond_Always);

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoInputs;

    if (ImGui::Begin("##LostArkPerfOverlay", nullptr, flags))
    {
        ImGui::TextUnformatted("LostArk Profiler");
        ImGui::Separator();
        ImGui::Text(
            "FPS: %.1f  (%.3f ms)",
            io.Framerate,
            io.Framerate > 0.f ? 1000.f / io.Framerate : 0.f);

        if (!bHasStats)
        {
            ImGui::TextUnformatted("Waiting for first frame...");
        }
        else
        {
            ImGui::Text("CPU work: %.3f ms", stats.CpuFrameMs);
            if (stats.GpuValid)
            {
                ImGui::Text(
                    "GPU: %.3f ms  (%u-frame delayed)",
                    stats.GpuFrameMs,
                    stats.GpuLatencyFrames);
            }
            else
            {
                ImGui::TextUnformatted("GPU: warming up...");
            }

            const auto Counter = [&stats](Engine::EProfilerCounter counter)
            {
                return stats.Counters[static_cast<size_t>(counter)];
            };

            ImGui::Text(
                "Draws: %llu  (instanced %llu)",
                static_cast<unsigned long long>(Counter(
                    Engine::EProfilerCounter::DrawCalls)),
                static_cast<unsigned long long>(Counter(
                    Engine::EProfilerCounter::InstancedDrawCalls)));
            ImGui::Text(
                "Indices: %llu  Instances: %llu",
                static_cast<unsigned long long>(Counter(
                    Engine::EProfilerCounter::Indices)),
                static_cast<unsigned long long>(Counter(
                    Engine::EProfilerCounter::Instances)));

            if (stats.GpuValid)
            {
                ImGui::Text(
                    "IA vertices: %llu  primitives: %llu",
                    static_cast<unsigned long long>(stats.Pipeline.IAVertices),
                    static_cast<unsigned long long>(stats.Pipeline.IAPrimitives));
            }
        }
    }
    ImGui::End();
}

void CMainApp::RenderProfilerSettings()
{
    if (!m_bProfilerVisible)
        return;

    if (!ImGui::Begin(
        "LostArk Profiler Details",
        nullptr,
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::End();
        return;
    }

    Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler();
    Engine::FProfilerLiveStats stats{};
    const bool_t bHasStats =
        nullptr != pProfiler && pProfiler->Get_LiveStats(stats);

    ImGui::TextUnformatted(
        "Profiler is controlled from the F1 Developer Tools window.");
    ImGui::Separator();

    if (!bHasStats)
    {
        ImGui::TextUnformatted("Waiting for first captured frame...");
    }
    else
    {
        const auto Counter = [&stats](Engine::EProfilerCounter counter)
        {
            return stats.Counters[static_cast<size_t>(counter)];
        };

        ImGui::Text("Frame: %llu",
            static_cast<unsigned long long>(stats.FrameNumber));
        ImGui::Text("CPU: %.3f ms", stats.CpuFrameMs);
        ImGui::Text("GPU: %s",
            stats.GpuValid ? "available" : "warming up");
        if (stats.GpuValid)
        {
            ImGui::SameLine();
            ImGui::Text("%.3f ms", stats.GpuFrameMs);
        }

        ImGui::SeparatorText("Renderer");
        ImGui::Text("Draw calls: %llu",
            static_cast<unsigned long long>(Counter(
                Engine::EProfilerCounter::DrawCalls)));
        ImGui::Text("Instanced draws: %llu / instances: %llu",
            static_cast<unsigned long long>(Counter(
                Engine::EProfilerCounter::InstancedDrawCalls)),
            static_cast<unsigned long long>(Counter(
                Engine::EProfilerCounter::Instances)));
        ImGui::Text("Indices: %llu",
            static_cast<unsigned long long>(Counter(
                Engine::EProfilerCounter::Indices)));

        ImGui::SeparatorText("Map");
        ImGui::Text("Placements: %llu / visible: %llu",
            static_cast<unsigned long long>(Counter(
                Engine::EProfilerCounter::MapPlacements)),
            static_cast<unsigned long long>(Counter(
                Engine::EProfilerCounter::MapVisibleInstances)));
        ImGui::Text("Batches: %llu / fallback: %llu",
            static_cast<unsigned long long>(Counter(
                Engine::EProfilerCounter::MapBatchCount)),
            static_cast<unsigned long long>(Counter(
                Engine::EProfilerCounter::MapFallbackObjects)));

        ImGui::SeparatorText("Navigation");
        ImGui::Text("Queries: %llu / expanded: %llu",
            static_cast<unsigned long long>(Counter(
                Engine::EProfilerCounter::NavigationQueries)),
            static_cast<unsigned long long>(Counter(
                Engine::EProfilerCounter::NavigationExpandedNodes)));
        ImGui::Text("Query time: %llu us / path cells: %llu",
            static_cast<unsigned long long>(Counter(
                Engine::EProfilerCounter::NavigationQueryMicroseconds)),
            static_cast<unsigned long long>(Counter(
                Engine::EProfilerCounter::NavigationPathCells)));
    }

    if (ImGui::Button("Reset profiler history"))
    {
        if (nullptr != pProfiler)
            pProfiler->Reset_History();
    }

    ImGui::SameLine();
    if (ImGui::Button("Save profiler JSON"))
    {
        if (nullptr == pProfiler)
        {
            m_strProfilerCaptureStatus = "Profiler is not available.";
        }
        else
        {
            const Engine::FProfilerCaptureSnapshot Snapshot =
                pProfiler->Snapshot();
            const uint64_t iFrameNumber = Snapshot.Frames.empty() ?
                0 : Snapshot.Frames.back().FrameNumber;
            const filesystem::path OutputPath =
                CProfilerCaptureIO::Make_DefaultPath(iFrameNumber);
            string strError;
            if (CProfilerCaptureIO::Save_Json(
                Snapshot,
                OutputPath,
                &strError))
            {
                m_strProfilerCaptureStatus =
                    "Saved: " + OutputPath.string();
            }
            else
            {
                m_strProfilerCaptureStatus =
                    "Save failed: " + strError;
            }
        }
    }

    if (false == m_strProfilerCaptureStatus.empty())
        ImGui::TextWrapped("%s", m_strProfilerCaptureStatus.c_str());

    ImGui::End();
}

void CMainApp::UpdateDebugToolShortcut()
{
    const bool_t bWindowFocused =
        IsWindowOwnedByCurrentProcess(GetForegroundWindow());

    const bool_t bF1Down = bWindowFocused &&
        0 != (GetAsyncKeyState(VK_F1) & 0x8000);

    if (bF1Down && !m_bF1Down)
        m_bDeveloperToolsVisible = !m_bDeveloperToolsVisible;

    m_bF1Down = bF1Down;
}
#endif

unique_ptr<CMainApp> CMainApp::Create()
{
    auto pInstance = unique_ptr<CMainApp>(new CMainApp());

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CMainApp");
        return nullptr;
    }

    return pInstance;
}

void CMainApp::Free()
{
    // NetworkManager 종료
    CNetworkManager::Get().Shutdown();
    CGameInstance::Get().SetInputBlocked(false, false);

#ifdef _DEBUG
    if (Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler())
        pProfiler->Set_Enabled(false);

    m_pAnimationTool.reset();
    m_pEffectTool.reset();
    m_pHUDLayoutTool.reset();
    m_pMapTool.reset();
#endif

    if (nullptr != m_pImGuiLayer)
        m_pImGuiLayer->Shutdown();
    m_pImGuiLayer.reset();

    CGameInstance::Get().Release_Engine();
}

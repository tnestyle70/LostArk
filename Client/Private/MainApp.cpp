#ifdef _DEBUG
#include "imgui.h"
#endif

#include "MainApp.h"
#include "GameInstance.h"
#include "Profiler.h"

#include "Level_Loading.h"

#ifdef _DEBUG
#include "ImGuiLayer.h"
#include "ProfilerCaptureIO.h"
#include "MapTool.h"
#include "Animation_Tool.h"
#include "Effect_Tool.h"
#include "HUDLayoutTool.h"
#endif

#ifdef _DEBUG
namespace
{
    bool_t IsWindowOwnedByCurrentProcess(HWND hWnd)
    {
        if (nullptr == hWnd)
            return false;

        DWORD dwProcessId = {};
        if (0 == GetWindowThreadProcessId(hWnd, &dwProcessId))
            return false;

        return GetCurrentProcessId() == dwProcessId;
    }
}
#endif

Client::CMainApp::CMainApp()
{
    /* wireframe or solid, cullmode -> cw, ccw */
   // D3D11_RASTERIZER_DESC           RSDesc{};
   // ComPtr<ID3D11RasterizerState>    pRSState = {};

   // if (FAILED(m_pDevice->CreateRasterizerState(&RSDesc, &pRSState)))
   //     return;

    
   // m_pContext->RSSetState(pRSState.Get());

   // /* ±íÀÌ ºñ±³ x or o, ±íÀÌ ±â·Ï x or o */
   // D3D11_DEPTH_STENCIL_DESC
   // m_pContext->OMSetDepthStencilState();

   // /* ºí·»µù¿¡ ´ëÇÑ ¼³Á¤. */
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

#ifdef _DEBUG
    if (FAILED(ReadyDebugTools()))
        return E_FAIL;
#endif

    if (FAILED(Ready_Gara()))
        return E_FAIL;

    if (FAILED(Ready_Fonts()))
        return E_FAIL;

    if (FAILED(Ready_Prototype_For_Static()))
        return E_FAIL;

    const wchar_t* pCommandLine = GetCommandLineW();
    const LEVEL eStartLevel =
        nullptr != pCommandLine &&
        nullptr != wcsstr(pCommandLine, L"--hdr-readback")
        ? LEVEL::TEST_LEVEL2
        : LEVEL::LOBBY;

    if (FAILED(Start_Level(eStartLevel)))
        return E_FAIL;

    return S_OK;
}

void CMainApp::Update(f32_t fTimeDelta)
{
#ifdef _DEBUG
    UpdateDebugToolShortcut();

    if (nullptr != m_pImGuiLayer)
        m_pImGuiLayer->BeginFrame();

    const bool_t bMapToolOpen = nullptr != m_pMapTool && m_pMapTool->IsOpen();
    const HWND hForegroundWindow = GetForegroundWindow();
    const bool_t bExternalToolFocused = bMapToolOpen &&
        nullptr != hForegroundWindow &&
        hForegroundWindow != g_hWnd &&
        IsWindowOwnedByCurrentProcess(hForegroundWindow);

    const bool_t bImGuiPanelOpen = bMapToolOpen || m_bProfilerVisible;
    const bool_t bKeyboardCaptured = bImGuiPanelOpen &&
        nullptr != m_pImGuiLayer &&
        (m_pImGuiLayer->WantsCaptureKeyboard() || bExternalToolFocused);
    const bool_t bMouseCapturedByUi = bImGuiPanelOpen &&
        nullptr != m_pImGuiLayer &&
        (m_pImGuiLayer->WantsCaptureMouse() ||
            bExternalToolFocused);
    const bool_t bWorldLeftMouseConsumed =
        nullptr != m_pMapTool &&
        m_pMapTool->ConsumesWorldLeftMouse();

    CGameInstance::Get().SetInputBlocked(
        bKeyboardCaptured,
        bMouseCapturedByUi);
    CGameInstance::Get().SetMouseButtonBlocked(
        DIM::LB,
        bWorldLeftMouseConsumed);
#endif

    CGameInstance::Get().Update_Engine(fTimeDelta);

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
#ifdef _DEBUG
        if (nullptr != m_pImGuiLayer)
            m_pImGuiLayer->CancelFrame();
#endif
        return E_FAIL;
    }

    if (FAILED(CGameInstance::Get().Render()))
    {
#ifdef _DEBUG
        if (nullptr != m_pImGuiLayer)
            m_pImGuiLayer->CancelFrame();
#endif
        return E_FAIL;
    }

#ifndef _DEBUG
    CGameInstance::Get().Draw_Text(TEXT("Font_Default"), TEXT("ÇÑ±Û ÀÌ´Ù12abd"), float2_t(0.f, 0.f));
#endif

#ifdef _DEBUG
    if (nullptr != m_pImGuiLayer)
    {
        if (nullptr != m_pMapTool)
            m_pMapTool->Render();

        if (nullptr != m_pMapTool &&
            m_pMapTool->IsOpen() &&
            nullptr != m_pEffectTool)
        {
            m_pEffectTool->Render();
        }

        if (nullptr != m_pMapTool &&
            m_pMapTool->IsOpen() &&
            nullptr != m_pAnimationTool)
        {
            m_pAnimationTool->Render();
        }

        if (nullptr != m_pMapTool &&
            m_pMapTool->IsOpen() &&
            nullptr != m_pHUDLayoutTool)
        {
            m_pHUDLayoutTool->Render();
        }

        if (m_bProfilerVisible)
        {
            RenderProfilerOverlay();
            RenderProfilerSettings();
        }

        m_pImGuiLayer->EndFrame();
    }
#endif

    if (FAILED(CGameInstance::Get().Render_End()))
        return E_FAIL;

    return S_OK;
}

HRESULT CMainApp::Ready_Gara()
{   
    uint32_t        iByte = {};
    HANDLE          hFile = CreateFile(TEXT("../Bin/DataFiles/Navigation.dat"), GENERIC_WRITE, 0 ,nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (0 == hFile)
        return E_FAIL;

    float3_t          vPoints[3] = {};

    vPoints[0] = float3_t(0.f, 0.f, 10.f);
    vPoints[1] = float3_t(10.f, 0.f, 0.f);
    vPoints[2] = float3_t(0.f, 0.f, 0.f);
    WriteFile(hFile, vPoints, sizeof(float3_t) * 3, reinterpret_cast<DWORD*>(&iByte), nullptr);

    vPoints[0] = float3_t(0.f, 0.f, 10.f);
    vPoints[1] = float3_t(10.f, 0.f, 10.f);
    vPoints[2] = float3_t(10.f, 0.f, 0.f);
    WriteFile(hFile, vPoints, sizeof(float3_t) * 3, reinterpret_cast<DWORD*>(&iByte), nullptr);

    vPoints[0] = float3_t(0.f, 0.f, 20.f);
    vPoints[1] = float3_t(10.f, 0.f, 10.f);
    vPoints[2] = float3_t(0.f, 0.f, 10.f);
    WriteFile(hFile, vPoints, sizeof(float3_t) * 3, reinterpret_cast<DWORD*>(&iByte), nullptr);

    vPoints[0] = float3_t(10.f, 0.f, 10.f);
    vPoints[1] = float3_t(20.f, 0.f, 0.f);
    vPoints[2] = float3_t(10.f, 0.f, 0.f);
    WriteFile(hFile, vPoints, sizeof(float3_t) * 3, reinterpret_cast<DWORD*>(&iByte), nullptr);

    CloseHandle(hFile);

    ComPtr<ID3D11Texture2D>         pTexture2D = { nullptr };

    D3D11_TEXTURE2D_DESC            TextureDesc{};

    TextureDesc.Width = 256;
    TextureDesc.Height = 256;
    TextureDesc.MipLevels = 1;
    TextureDesc.ArraySize = 1;
    TextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    TextureDesc.SampleDesc.Quality = 0;
    TextureDesc.SampleDesc.Count = 1;  
    TextureDesc.Usage = D3D11_USAGE_STAGING;
    TextureDesc.BindFlags = 0;
    TextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    TextureDesc.MiscFlags = 0;

    shared_ptr<uint32_t[]>      pPixels = make_shared<uint32_t[]>(TextureDesc.Width * TextureDesc.Height);

    pPixels[0] = 0xffffffff;

    D3D11_SUBRESOURCE_DATA      InitialDesc{};
    InitialDesc.pSysMem = pPixels.get();
    InitialDesc.SysMemPitch = TextureDesc.Width * sizeof(uint32_t);

    if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, &InitialDesc, &pTexture2D)))
        return E_FAIL;

    D3D11_MAPPED_SUBRESOURCE            SubResource{};

    m_pContext->Map(pTexture2D.Get(), 0, D3D11_MAP_READ_WRITE, 0, &SubResource);

    auto        pLockPixels = static_cast<uint32_t*>(SubResource.pData);

    for (uint32_t i = 0; i < 256; i++)
    {
        for (uint32_t j = 0; j < 256; j++)
        {
            uint32_t        iIndex = i * 256 + j;

            /* a b g r */
            if(j < 128)
                pLockPixels[iIndex] = 0xffffffff;
            else
                pLockPixels[iIndex] = 0xff000000;
        }
    }

    if (FAILED(DirectX::SaveDDSTextureToFile(m_pContext.Get(), pTexture2D.Get(), TEXT("../Bin/Resources/Textures/Terrain/MyMask.dds"))))
        return E_FAIL;

    return S_OK;
}

HRESULT CMainApp::Ready_Fonts()
{
    /*
MakeSpriteFont "³Ø½¼lv1°íµñ Bold" /FontSize:20 /FastPack /CharacterRegion:0x0020-0x00FF /CharacterRegion:0x3131-0x3163 /CharacterRegion:0xAC00-0xD800 /DefaultCharacter:0xAC00 161ex.spritefont
*/


    if (FAILED(CGameInstance::Get().Add_Font(TEXT("Font_Default"), TEXT("../Bin/Resources/Fonts/161ex.spritefont"))))
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

#ifdef _DEBUG
HRESULT CMainApp::ReadyDebugTools()
{
    m_pImGuiLayer = std::make_unique<Engine::CImGuiLayer>();
    if (!m_pImGuiLayer->Initialize(g_hWnd, m_pDevice.Get(), m_pContext.Get()))
    {
        OutputDebugStringA("[ImGui] Failed to initialize Win32/DX11 runtime.\n");
        return E_FAIL;
    }

    auto mapTool = std::make_unique<CMapTool>();
    if (FAILED(mapTool->Initialize(m_pDevice, m_pContext)))
        return E_FAIL;
    m_pMapTool = std::move(mapTool);

    m_pAnimationTool = std::make_unique<CAnimation_Tool>();

    m_pEffectTool = std::make_unique<CEffect_Tool>(m_pDevice);
    m_pHUDLayoutTool = std::make_unique<CHUDLayoutTool>(m_pDevice, m_pContext);

    const wchar_t* pCommandLine = GetCommandLineW();
    const bool_t isEffectProfileRequested =
        nullptr != pCommandLine &&
        nullptr != wcsstr(pCommandLine, L"--effect-profile");
    if (nullptr != pCommandLine &&
        nullptr != wcsstr(pCommandLine, L"--effect-open") &&
        nullptr != m_pMapTool &&
        !m_pMapTool->IsOpen())
    {
        m_pMapTool->Toggle();
    }

    if (Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler())
    {
        pProfiler->Reset_History();
        pProfiler->Set_Enabled(isEffectProfileRequested);
    }
    m_bProfilerVisible = isEffectProfileRequested;

    return S_OK;
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

    ImGui::TextUnformatted("F4: profiler on / off");
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

    const bool_t bF4Down = bWindowFocused &&
        0 != (GetAsyncKeyState(VK_F4) & 0x8000);

    if (bF1Down && !m_bF1Down && nullptr != m_pMapTool)
        m_pMapTool->Toggle();

    if (bF4Down && !m_bF4Down)
    {
        m_bProfilerVisible = !m_bProfilerVisible;

        if (Engine::CProfiler* pProfiler =
            CGameInstance::Get().Get_Profiler())
        {
            if (m_bProfilerVisible)
                pProfiler->Reset_History();
            pProfiler->Set_Enabled(m_bProfilerVisible);
        }
    }

    m_bF1Down = bF1Down;
    m_bF4Down = bF4Down;
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
#ifdef _DEBUG
    CGameInstance::Get().SetInputBlocked(false, false);

    if (Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler())
        pProfiler->Set_Enabled(false);

    m_pAnimationTool.reset();
    m_pEffectTool.reset();
    m_pHUDLayoutTool.reset();
    m_pMapTool.reset();

    if (nullptr != m_pImGuiLayer)
        m_pImGuiLayer->Shutdown();
    m_pImGuiLayer.reset();
#endif

    CGameInstance::Get().Release_Engine();
}

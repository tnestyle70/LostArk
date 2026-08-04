// Client.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "Client.h"

#include "Client_Defines.h"
#include "Effect_AssetIO.h"
#include "Effect_ParticleSimulator.h"
#include "MainApp.h"
#include "GameInstance.h"
#include "Profiler.h"

#include <fstream>

#include "ImGuiLayer.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE g_hInst; // 현재 인스턴스입니다.
HWND    g_hWnd;
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

namespace
{
    /* 지정한 플래그 뒤의 파일 경로를 잘라낸다. 따옴표를 허용한다. */
    filesystem::path Parse_FlagPath(
        const wchar_t* pCmdLine, const wchar_t* pFlag)
    {
        const wchar_t* pFound = wcsstr(pCmdLine, pFlag);
        if (nullptr == pFound)
            return {};

        const wchar_t* pCursor = pFound + wcslen(pFlag);
        while (L' ' == *pCursor || L'\t' == *pCursor)
            ++pCursor;

        if (L'\0' == *pCursor)
            return {};

        if (L'"' == *pCursor)
        {
            ++pCursor;
            const wchar_t* pEnd = wcschr(pCursor, L'"');
            if (nullptr == pEnd)
                return {};
            return filesystem::path(wstring_t(pCursor, pEnd));
        }
        return filesystem::path(wstring_t(pCursor));
    }

    /* 재직렬화 결과를 바이트로 비교하기 위해 파일 전체를 읽는다. */
    bool_t Read_FileBytes(const filesystem::path& Path, string& OutBytes)
    {
        ifstream File(Path, ios::binary);
        if (!File.is_open())
            return false;

        OutBytes.assign(
            istreambuf_iterator<char_t>(File),
            istreambuf_iterator<char_t>());
        return true;
    }

    size_t Count_Modules(const EFFECT_ASSET_DESC& Asset)
    {
        size_t iCount = 0;
        for (const auto& Emitter : Asset.Emitters)
            iCount += Emitter.Modules.size();
        return iCount;
    }

    /*
     * 에셋을 실제로 시뮬레이션해 파티클이 태어나는지 확인한다.
     * 툴 화면에서 Alive 0 이 보일 때 시뮬레이터 문제인지 툴의 재생 루프
     * 문제인지 갈라내기 위한 것이다.
     */
    int Run_EffectSimulation(const wchar_t* pCmdLine)
    {
        const filesystem::path SourcePath =
            Parse_FlagPath(pCmdLine, L"--effect-sim");
        const filesystem::path WorkRoot =
            filesystem::temp_directory_path() / "lostark_effect_roundtrip";

        error_code Error;
        filesystem::create_directories(WorkRoot, Error);
        ofstream Report(WorkRoot / "sim.txt", ios::trunc);

        EFFECT_ASSET_DESC Asset;
        string strError;
        if (!CEffect_AssetIO::Load_Authoring(SourcePath, Asset, &strError))
        {
            Report << "[FAIL] Load : " << strError << "\n";
            return EXIT_FAILURE;
        }

        Report << "asset=" << Asset.strAssetId
            << " duration=" << Asset.fDuration
            << " emitters=" << Asset.Emitters.size() << "\n";

        vector<unique_ptr<CEffect_ParticleSimulator>> Simulators;
        for (const EFFECT_EMITTER_DESC& Emitter : Asset.Emitters)
        {
            auto pSimulator = make_unique<CEffect_ParticleSimulator>();
            const bool_t isReady = pSimulator->Initialize(Emitter);
            Report << "  emitter '" << Emitter.strName
                << "' delay=" << Emitter.fDelay
                << " duration=" << Emitter.fDuration
                << " loops=" << Emitter.iLoopCount
                << " init=" << (isReady ? "OK" : "REJECTED") << "\n";
            Simulators.push_back(isReady ? move(pSimulator) : nullptr);
        }

        /* 1. 리셋 없이 1/60초로 굴린다. 시뮬레이터 단독 동작 확인. */
        const f32_t fStep = 1.f / 60.f;
        size_t iPeakAlive = 0;
        for (uint32_t iFrame = 1; iFrame <= 90; ++iFrame)
        {
            size_t iAlive = 0;
            for (const auto& pSimulator : Simulators)
            {
                if (nullptr == pSimulator)
                    continue;
                pSimulator->Update(fStep);
                iAlive += pSimulator->Get_Particles().size();
            }
            iPeakAlive = max(iPeakAlive, iAlive);
            if (0 == iFrame % 15)
            {
                Report << "  frame " << iFrame << " t="
                    << static_cast<f32_t>(iFrame) * fStep
                    << " alive=" << iAlive << "\n";
            }
        }

        /*
         * 에미터별 파티클 색을 그대로 찍는다. HDR 검증에서 1 / 5 / 25 같은
         * 1 초과 값이 시뮬레이터를 통과해 tint까지 살아오는지 보기 위한 것이다.
         */
        for (size_t i = 0; i < Simulators.size(); ++i)
        {
            if (nullptr == Simulators[i])
                continue;

            const auto& Particles = Simulators[i]->Get_Particles();
            if (Particles.empty())
                continue;

            f32_t fMaxChannel = 0.f;
            for (const EFFECT_PARTICLE& Particle : Particles)
            {
                fMaxChannel = max(fMaxChannel, Particle.vColor.x);
                fMaxChannel = max(fMaxChannel, Particle.vColor.y);
                fMaxChannel = max(fMaxChannel, Particle.vColor.z);
            }

            Report << "  colour '" << Asset.Emitters[i].strName
                << "' rgba=" << Particles[0].vColor.x
                << "," << Particles[0].vColor.y
                << "," << Particles[0].vColor.z
                << "," << Particles[0].vColor.w
                << " maxChannel=" << fMaxChannel
                << (1.f < fMaxChannel ? "  [>1 SURVIVED]" : "") << "\n";
        }
        Report << "[" << (0 < iPeakAlive ? "OK" : "FAIL")
            << "]   raw 1/60 tick : peak alive=" << iPeakAlive << "\n";

        /*
         * 2. 툴의 재생 루프를 그대로 흉내낸다. 프레임 델타를 바꿔가며,
         *    렌더 시점(리셋 이후)에 남아 있는 파티클 수를 본다.
         */
        for (const f32_t fDelta : { 1.f / 60.f, 1.f / 30.f, 0.1f, 0.3f, 0.5f })
        {
            for (auto& pSimulator : Simulators)
            {
                if (nullptr != pSimulator)
                    pSimulator->Reset();
            }

            f32_t fPreviewTime = 0.f;
            size_t iPeakAtRender = 0;
            for (uint32_t iFrame = 0; iFrame < 120; ++iFrame)
            {
                fPreviewTime += fDelta;
                for (const auto& pSimulator : Simulators)
                {
                    if (nullptr != pSimulator)
                        pSimulator->Update(fDelta);
                }

                if (Asset.fDuration > 0.f && fPreviewTime >= Asset.fDuration)
                {
                    fPreviewTime = 0.f;
                    for (auto& pSimulator : Simulators)
                    {
                        if (nullptr != pSimulator)
                            pSimulator->Reset();
                    }
                }

                size_t iAlive = 0;
                for (const auto& pSimulator : Simulators)
                {
                    if (nullptr != pSimulator)
                        iAlive += pSimulator->Get_Particles().size();
                }
                iPeakAtRender = max(iPeakAtRender, iAlive);
            }

            Report << "  tool loop dt=" << fDelta << " ("
                << static_cast<int32_t>(1.f / fDelta) << " fps) : peak alive at render="
                << iPeakAtRender << "\n";
        }

        return EXIT_SUCCESS;
    }

    /*
     * 실제 .effect 파일 하나를 Load -> Save -> Load -> Cook -> Binary Load 로
     * 왕복시킨다. 필드를 하나씩 비교하는 대신 각 단계 결과를 다시 저장해
     * 바이트로 대조하므로, 어느 필드가 유실되어도 차이가 드러난다.
     */
    int Run_EffectRoundTrip(const wchar_t* pCmdLine)
    {
        const filesystem::path SourcePath =
            Parse_FlagPath(pCmdLine, L"--effect-roundtrip");
        const filesystem::path WorkRoot =
            filesystem::temp_directory_path() / "lostark_effect_roundtrip";

        error_code Error;
        filesystem::create_directories(WorkRoot, Error);

        ofstream Report(WorkRoot / "report.txt", ios::trunc);
        Report << "source = " << SourcePath.string() << "\n";

        if (SourcePath.empty() ||
            !filesystem::is_regular_file(SourcePath, Error))
        {
            Report << "[FAIL] source file not found\n";
            return EXIT_FAILURE;
        }

        string strError;
        EFFECT_ASSET_DESC Original;
        if (!CEffect_AssetIO::Load_Authoring(SourcePath, Original, &strError))
        {
            Report << "[FAIL] Load : " << strError << "\n";
            return EXIT_FAILURE;
        }
        Report << "[OK]   Load : schema=" << Original.iSchemaVersion
            << " emitters=" << Original.Emitters.size()
            << " modules=" << Count_Modules(Original)
            << " assetId=" << Original.strAssetId << "\n";

        const filesystem::path SavedPath = WorkRoot / "saved.effect";
        if (!CEffect_AssetIO::Save_Authoring(SavedPath, Original, &strError))
        {
            Report << "[FAIL] Save : " << strError << "\n";
            return EXIT_FAILURE;
        }

        string strSaved;
        if (!Read_FileBytes(SavedPath, strSaved))
        {
            Report << "[FAIL] Save : cannot read back saved file\n";
            return EXIT_FAILURE;
        }
        Report << "[OK]   Save : " << strSaved.size() << " bytes\n";

        bool_t isPassed = true;

        auto Compare = [&](const string& strLeft, const string& strRight,
            const char_t* pLabel)
        {
            if (strLeft == strRight)
            {
                Report << "[OK]   " << pLabel << " : identical ("
                    << strLeft.size() << " bytes)\n";
                return;
            }

            size_t iOffset = 0;
            while (iOffset < strLeft.size() && iOffset < strRight.size() &&
                strLeft[iOffset] == strRight[iOffset])
                ++iOffset;

            Report << "[FAIL] " << pLabel << " : first difference at byte "
                << iOffset << " (" << strLeft.size() << " vs "
                << strRight.size() << " bytes)\n";
            isPassed = false;
        };

        /* Save -> Load -> Save 가 같은 바이트를 내야 한다. */
        EFFECT_ASSET_DESC Reloaded;
        if (!CEffect_AssetIO::Load_Authoring(SavedPath, Reloaded, &strError))
        {
            Report << "[FAIL] Reload : " << strError << "\n";
            return EXIT_FAILURE;
        }
        Report << "[OK]   Reload : emitters=" << Reloaded.Emitters.size()
            << " modules=" << Count_Modules(Reloaded) << "\n";

        const filesystem::path ReSavedPath = WorkRoot / "resaved.effect";
        string strReSaved;
        if (!CEffect_AssetIO::Save_Authoring(
                ReSavedPath, Reloaded, &strError) ||
            !Read_FileBytes(ReSavedPath, strReSaved))
        {
            Report << "[FAIL] Resave : " << strError << "\n";
            return EXIT_FAILURE;
        }
        Compare(strSaved, strReSaved, "Save->Load->Save");

        /* Cook -> Binary Load 도 같은 내용을 복원해야 한다. */
        const filesystem::path CookedPath = WorkRoot / "cooked.weffect";
        if (!CEffect_AssetIO::Save_Binary(CookedPath, Original, &strError))
        {
            Report << "[FAIL] Cook : " << strError << "\n";
            return EXIT_FAILURE;
        }
        Report << "[OK]   Cook : "
            << filesystem::file_size(CookedPath, Error) << " bytes\n";

        EFFECT_ASSET_DESC BinaryLoaded;
        if (!CEffect_AssetIO::Load_Binary(CookedPath, BinaryLoaded, &strError))
        {
            Report << "[FAIL] BinaryLoad : " << strError << "\n";
            return EXIT_FAILURE;
        }
        Report << "[OK]   BinaryLoad : emitters="
            << BinaryLoaded.Emitters.size()
            << " modules=" << Count_Modules(BinaryLoaded) << "\n";

        const filesystem::path FromBinaryPath = WorkRoot / "frombinary.effect";
        string strFromBinary;
        if (!CEffect_AssetIO::Save_Authoring(
                FromBinaryPath, BinaryLoaded, &strError) ||
            !Read_FileBytes(FromBinaryPath, strFromBinary))
        {
            Report << "[FAIL] BinaryLoad resave : " << strError << "\n";
            return EXIT_FAILURE;
        }
        Compare(strSaved, strFromBinary, "Cook->BinaryLoad");

        Report << (isPassed ? "RESULT = PASS\n" : "RESULT = FAIL\n");
        return isPassed ? EXIT_SUCCESS : EXIT_FAILURE;
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    if (nullptr != lpCmdLine &&
        nullptr != wcsstr(lpCmdLine, L"--effect-sim"))
    {
        return Run_EffectSimulation(lpCmdLine);
    }

    if (nullptr != lpCmdLine &&
        nullptr != wcsstr(lpCmdLine, L"--effect-roundtrip"))
    {
        return Run_EffectRoundTrip(lpCmdLine);
    }

    if (nullptr != lpCmdLine &&
        nullptr != wcsstr(lpCmdLine, L"--effect-phase2-test"))
    {
        const auto Result = Run_Phase2ParticleCountValidation();

        EFFECT_ASSET_DESC EmptyAsset;
        EmptyAsset.strAssetId = "headless_round_trip";
        EmptyAsset.strName = "Headless Round Trip";

        const filesystem::path TestRoot =
            filesystem::temp_directory_path() /
            "lostark_effect_tool_headless";

        const filesystem::path AuthoringPath =
            TestRoot / "roundtrip.effect";

        const filesystem::path BinaryPath =
            TestRoot / "roundtrip.weffect";

        EFFECT_ASSET_DESC JsonLoaded;

        EFFECT_ASSET_DESC BinaryLoaded;

        const bool_t isRoundTripPassed =
            CEffect_AssetIO::Validate_RoundTrip(EmptyAsset) &&
            CEffect_AssetIO::Save_Authoring(AuthoringPath, EmptyAsset) &&
            CEffect_AssetIO::Load_Authoring(AuthoringPath, JsonLoaded) &&
            CEffect_AssetIO::Save_Binary(BinaryPath, JsonLoaded) &&
            CEffect_AssetIO::Load_Binary(BinaryPath, BinaryLoaded) &&
            BinaryLoaded.strAssetId == EmptyAsset.strAssetId;

        error_code CleanupError;

        filesystem::remove_all(TestRoot, CleanupError);

        return Result.isPassed && isRoundTripPassed
            ? EXIT_SUCCESS : EXIT_FAILURE;
    }

#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    auto    pMainApp = CMainApp::Create();
    if (nullptr == pMainApp)
        return FALSE;

    if (FAILED(CGameInstance::Get().Add_Timer(TEXT("Timer_Default"))))
        return FALSE;
    if (FAILED(CGameInstance::Get().Add_Timer(TEXT("Timer_60"))))
        return FALSE;


    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENT));

    MSG msg;

    f32_t       fTimeAcc = {};

    // 기본 메시지 루프입니다:
    while (true)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (WM_QUIT == msg.message)
                break;
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        CGameInstance::Get().Update_TimeDelta(TEXT("Timer_Default"));

        fTimeAcc += CGameInstance::Get().Get_TimeDelta(TEXT("Timer_Default"));

        if (fTimeAcc >= 1.f / 60.f /* 1초에 60번만들어와. */)
        {
            CGameInstance::Get().Update_TimeDelta(TEXT("Timer_60"));

            Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler();
            if (nullptr != pProfiler)
                pProfiler->Begin_Frame();

            {
                Engine::CProfilerScope scope(pProfiler, "Client.Update");
                pMainApp->Update(CGameInstance::Get().Get_TimeDelta(TEXT("Timer_60")));
            }

            HRESULT hRenderResult = S_OK;
            {
                Engine::CProfilerScope scope(pProfiler, "Client.Render");
                hRenderResult = pMainApp->Render();
            }

            if (nullptr != pProfiler)
                pProfiler->End_Frame();

            if (FAILED(hRenderResult))
                break;

            fTimeAcc = 0.f;
        }        
    }

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIENT));
    wcex.hCursor        = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR_DEFAULT));
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CLIENT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    g_hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   RECT     rcWindow = { 0, 0, g_iWinSizeX, g_iWinSizeY };

   AdjustWindowRect(&rcWindow, WS_OVERLAPPEDWINDOW, true);

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   g_hWnd = hWnd;

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (CImGuiLayer::HandleWindowMessage(hWnd, message, wParam, lParam))
        return 1;

    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

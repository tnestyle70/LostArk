// Client.exe의 창과 메시지 루프를 구성하고, 게임 프레임은 MainApp에 맡긴다.

// 창 생성·OS 메시지 처리에 필요한 Win32 선언을 사용한다.
#include "framework.h"
// 기존 COM 헤더다. 이 파일은 COM API를 직접 호출하지 않으며 초기화는 MainApp에서 한다.
#include <objbase.h>
// EXE의 제목·아이콘·단축키 리소스 ID를 사용한다.
#include "Client.h"

// 초기 창 크기와 Client 전역 창 handle 선언을 사용한다.
#include "Client_Defines.h"
// Client 초기화·프레임 갱신·렌더링·종료를 맡긴다.
#include "MainApp.h"
// 엔진이 관리하는 프레임 타이머와 Profiler에 접근한다.
#include "GameInstance.h"
// 한 프레임의 Update·Render 시간을 측정한다.
#include "Profiler.h"
// 문자·마우스 휠 메시지를 제품 UI에 전달한다.
#include "UIInputRouter.h"
// 저장된 화면 설정과 창 크기 변경을 적용한다.
#include "ClientWindowDisplay.h"
// 창 생성 전에 저장된 화면 설정을 읽는다.
#include "UserSettingsDocument.h"
// EXE 경로로 크래시 기록을 준비한다.
#include "ClientDiagnostics.h"

// 도구 창의 Win32 입력 메시지를 ImGui에 전달한다.
#include "ImGuiLayer.h"

// Debug 할당 중단점의 숫자 변환을 사용한다.
#include <cstdlib>

// 창 제목과 클래스 이름의 버퍼 크기다.
#define MAX_LOADSTRING 100

// MainApp과 화면 설정 서비스가 같은 주 창을 참조한다.
HINSTANCE g_hInst; // 현재 인스턴스입니다.
HWND    g_hWnd;
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 창 생성은 wWinMain이 요청하고, 창·대화상자 메시지는 아래 콜백으로 받는다.
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// 창 생성 → MainApp 준비 → 메시지/프레임 반복 → 화면·앱 종료를 연결한다.
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    // 엔진 생성 전 설치해 초기화·종료 정리 중의 치명적 예외도 기록한다.
    Client::PrepareCrashReport();

#ifdef _DEBUG
    // Engine의 static 자원이 해제된 뒤 DebugLeakReport가 누수를 보고하도록 EXE 자동 보고는 끈다.
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF);
    // LOSTARK_BREAK_ALLOC에 지정한 할당 번호에서 디버거를 중단한다.
    char_t szBreakAlloc[32]{};
    size_t iBreakAllocLength = 0;

    if (0 == getenv_s(&iBreakAllocLength, szBreakAlloc, sizeof(szBreakAlloc),
            "LOSTARK_BREAK_ALLOC") && iBreakAllocLength > 1)
    {
        const long iBreakAlloc = std::strtol(szBreakAlloc, nullptr, 10);
        if (iBreakAlloc > 0)
            _CrtSetBreakAlloc(iBreakAlloc);
    }
#endif

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 창 리소스를 읽고 OS 메시지를 받을 창 클래스를 등록한다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 창을 만들기 전에 저장된 화면 설정을 읽는다. 실패하면 현재 설정으로 계속한다.
    std::string settingsStatus;
    if (!Client::CUserSettings::Get().Load_Persisted(settingsStatus))
        OutputDebugStringA(("[Settings] " + settingsStatus + "\n").c_str());

    // 엔진 장치를 연결할 주 창을 먼저 만든다.
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    // MainApp이 엔진 관리자·공용 원형·제품 UI를 준비하고 Lobby 진입을 시작한다.
    auto    pMainApp = CMainApp::Create();
    if (nullptr == pMainApp)
    {
        return 1;
    }

    // Update에 전달할 경과 시간을 엔진 타이머로 측정한다. Timer_60은 FPS 제한값이 아니다.
    if (FAILED(CGameInstance::Get().Add_Timer(TEXT("Timer_60"))))
    {
        return FALSE;
    }

    // 리소스 단축키와 OS 메시지 큐를 연결한다.
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENT));
    MSG msg{};

    // 대기 메시지를 처리한 뒤 게임 프레임을 실행한다.
    while (true)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (WM_QUIT == msg.message)
                break;

            // 단축키가 소비하지 않은 메시지를 WndProc으로 전달한다.
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        if (WM_QUIT == msg.message)
        {
            break;
        }

        // WndProc이 예약한 창 크기를 프레임 시작 전에 GPU에 적용한다.
        std::string displayStatus;
        if (!Client::CClientWindowDisplay::Process_PendingResize(displayStatus))
        {
            break;
        }

        // 최소화해도 네트워크·프레임 처리를 이어가되 대기를 넣어 실행 빈도를 낮춘다.
        if (Client::CClientWindowDisplay::Is_Minimized())
            MsgWaitForMultipleObjects(0, nullptr, FALSE, 50, QS_ALLINPUT);

        // 지난 프레임 처리와 제한 대기를 포함한 실제 경과 시간을 갱신한다.
        CGameInstance::Get().Update_TimeDelta(TEXT("Timer_60"));
        // 엔진 Profiler가 이번 Update·Render 구간을 측정한다.
        Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler();
        if (nullptr != pProfiler)
            pProfiler->Begin_Frame();

        {
            // MainApp → GameInstance → Object_Manager/Level_Manager 순서로 갱신한다.
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

        // 시스템 옵션의 프레임 제한 대기는 Profiler 프레임 구간 밖에서 수행한다.
        pMainApp->Limit_FrameRate();

        if (FAILED(hRenderResult))
        {
            break;
        }
    }

    // 엔진 종료 전에 화면 설정 연결을 해제한다. MainApp 종료가 UI·도구와 엔진 정리를 이어간다.
    Client::CClientWindowDisplay::Shutdown();
    return (int) msg.wParam;
}



// wWinMain이 호출해 창 모양과 WndProc 연결을 Windows에 등록한다.
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
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

// 주 창을 만들고 저장된 화면 설정을 적용한다. 설정 실패 시 만든 창도 닫는다.
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    g_hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   // 원하는 게임 화면 크기에 창 테두리·제목 표시줄을 더한다.
   RECT     rcWindow = { 0, 0, g_iWinSizeX, g_iWinSizeY };

   AdjustWindowRect(&rcWindow, WS_OVERLAPPEDWINDOW, FALSE);

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   g_hWnd = hWnd;
   std::string displayStatus;
   if (!Client::CClientWindowDisplay::Configure_InitialWindow(hWnd,
       Client::CUserSettings::Get().Get_DisplaySettings(), displayStatus))
   {
       DestroyWindow(hWnd);
       return FALSE;
   }

   // 준비된 주 창을 표시한 뒤 MainApp 초기화로 넘어간다.
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   g_hWnd = hWnd;

   return TRUE;
}

// 창 크기·DPI·UI 입력·종료 메시지를 각 담당 서비스에 전달한다.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // GPU 크기 변경은 즉시 실행하지 않고 다음 프레임 경계로 예약한다.
    if (message == WM_SIZE)
        Client::CClientWindowDisplay::Queue_Size(LOWORD(lParam), HIWORD(lParam), wParam == SIZE_MINIMIZED);
    // 모니터 DPI 변경에 따른 Windows 권장 창 영역을 적용한다.
    if (message == WM_DPICHANGED)
    {
        Client::CClientWindowDisplay::On_DpiChanged(HIWORD(wParam), *reinterpret_cast<const RECT*>(lParam));
        return 0;
    }
    // Client 영역의 크기 제한을 DPI·테두리가 포함된 창 크기로 변환한다.
    if (message == WM_GETMINMAXINFO)
    {
        auto* limits = reinterpret_cast<MINMAXINFO*>(lParam);
        RECT minimum{0, 0, 640, 480};
        AdjustWindowRectExForDpi(&minimum, static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_STYLE)),
            FALSE, static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_EXSTYLE)), GetDpiForWindow(hWnd));
        limits->ptMinTrackSize.x = minimum.right - minimum.left;
        limits->ptMinTrackSize.y = minimum.bottom - minimum.top;
        // 지정 해상도가 모니터보다 커도 명시적인 창 크기 조절을 허용한다.
        RECT maximum{0, 0, 16384, 16384};
        AdjustWindowRectExForDpi(&maximum, static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_STYLE)),
            FALSE, static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_EXSTYLE)), GetDpiForWindow(hWnd));
        limits->ptMaxTrackSize.x = maximum.right - maximum.left;
        limits->ptMaxTrackSize.y = maximum.bottom - maximum.top;
        return 0;
    }

    // ImGui가 소비하지 않은 메시지를 제품 UI와 기본 창 처리로 넘긴다.
    if (CImGuiLayer::HandleWindowMessage(hWnd, message, wParam, lParam))
        return 1;

    switch (message)
    {
    case WM_CHAR:
        // 입력 중인 제품 UI에 확정 문자를 전달한다.
        Client::CUIInputRouter::Get().On_Char(static_cast<wchar_t>(wParam));
        break;
    case WM_MOUSEWHEEL:
        // 제품 UI 목록의 스크롤 입력을 전달한다.
        Client::CUIInputRouter::Get().On_MouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
        break;
    // 리소스 단축키의 정보 창·종료 명령을 처리한다.
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
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
    // Windows 갱신 영역을 처리한다. 게임 화면은 MainApp::Render가 그린다.
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
    // wWinMain의 프레임 루프에 종료를 알린다.
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화상자의 확인·취소를 처리한다.
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

// Client.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "Client.h"

#include "Client_Defines.h"
#include "MainApp.h"
#include "GameInstance.h"
#include "Profiler.h"
#include "UIInputRouter.h"

#include "ImGuiLayer.h"

#include <fstream>
#include <iomanip>

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
    void WriteExitDiagnostic(const char* reason, const HRESULT result = S_OK)
    {
#ifdef _DEBUG
        std::ofstream output(
            "ClientExit.user.log",
            std::ios::binary | std::ios::app);
        if (!output)
            return;

        SYSTEMTIME time{};
        ::GetLocalTime(&time);
        output << std::setfill('0')
            << time.wYear << '-'
            << std::setw(2) << time.wMonth << '-'
            << std::setw(2) << time.wDay << ' '
            << std::setw(2) << time.wHour << ':'
            << std::setw(2) << time.wMinute << ':'
            << std::setw(2) << time.wSecond
            << " reason=" << reason
            << " hr=0x" << std::hex << std::uppercase
            << static_cast<unsigned long>(result)
            << std::dec
            << " level=" << CGameInstance::Get().Get_CurrentLevelID()
            << '\n';
#else
        UNREFERENCED_PARAMETER(reason);
        UNREFERENCED_PARAMETER(result);
#endif
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
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
        WriteExitDiagnostic("InitInstance failed", E_FAIL);
        return FALSE;
    }

    auto    pMainApp = CMainApp::Create();
    if (nullptr == pMainApp)
    {
        WriteExitDiagnostic("MainApp creation failed", E_FAIL);
        return 1;
    }

    if (FAILED(CGameInstance::Get().Add_Timer(TEXT("Timer_Default"))))
    {
        WriteExitDiagnostic("Timer_Default creation failed", E_FAIL);
        return FALSE;
    }
    if (FAILED(CGameInstance::Get().Add_Timer(TEXT("Timer_60"))))
    {
        WriteExitDiagnostic("Timer_60 creation failed", E_FAIL);
        return FALSE;
    }


    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENT));

    MSG msg{};

    f32_t       fTimeAcc = {};

    // 기본 메시지 루프입니다:
    while (true)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (WM_QUIT == msg.message)
            {
                WriteExitDiagnostic("WM_QUIT", S_OK);
                break;
            }
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
            {
                WriteExitDiagnostic("Render failed", hRenderResult);
                break;
            }

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
    case WM_CHAR:
        /* Committed text (ASCII and IME-composed Hangul alike arrive here; ImGui's handler
        above queues but never consumes WM_CHAR) for the runtime UI's own text fields -- the
        router drops it unless one of them is active. */
        Client::CUIInputRouter::Get().On_Char(static_cast<wchar_t>(wParam));
        break;
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

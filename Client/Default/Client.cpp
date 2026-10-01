// Client.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include <objbase.h>
#include "Client.h"

#include "Client_Defines.h"
#include "MainApp.h"
#include "GameInstance.h"
#include "Profiler.h"
#include "UIInputRouter.h"
#include "ClientWindowDisplay.h"
#include "UserSettingsDocument.h"

#include "ImGuiLayer.h"

#include <cstdlib>
#include <exception>
#include <csignal>
#include <fstream>
#include <filesystem>
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
    // Prepare the file before a fatal C++ path; crash reporting must not depend
    // on Engine lifetime, iostreams, a heap-built path or symbol-loader state.
    wchar_t g_CrashLogPath[32768]{};
    HANDLE g_CrashLogFile = INVALID_HANDLE_VALUE;
    std::terminate_handler g_PreviousTerminateHandler = nullptr;
    using AbortSignalHandler = void (*)(int);
    AbortSignalHandler g_PreviousAbortHandler = SIG_DFL;
    volatile LONG g_TerminateDiagnosticEntered = 0;
    char g_CrashLogText[65536]{};

    struct CrashText final
    {
        DWORD length = 0;

        void Character(const char value) noexcept
        {
            if (length < sizeof(g_CrashLogText))
                g_CrashLogText[length++] = value;
        }

        void Text(const char* value, const size_t limit = 4096u) noexcept
        {
            if (!value) value = "<null>";
            for (size_t index = 0; index < limit && value[index]; ++index)
                Character(value[index]);
        }

        void Escaped(const char* value, const size_t limit = 4096u) noexcept
        {
            if (!value) value = "<null>";
            size_t index = 0;
            for (; index < limit && value[index]; ++index)
            {
                const unsigned char ch = static_cast<unsigned char>(value[index]);
                if (ch == '\r') Text("\\r");
                else if (ch == '\n') Text("\\n");
                else if (ch == '\t') Text("\\t");
                else if (ch < 32u || ch == 127u) Character('?');
                else Character(static_cast<char>(ch));
            }
            if (index == limit) Text("<truncated>");
        }

        void Number(ULONGLONG value, const unsigned int width = 0u,
            const unsigned int radix = 10u) noexcept
        {
            char reversed[32]{};
            unsigned int count = 0;
            do
            {
                reversed[count++] = "0123456789ABCDEF"[value % radix];
                value /= radix;
            } while (value && count < sizeof(reversed));
            for (unsigned int padding = count; padding < width; ++padding) Character('0');
            while (count) Character(reversed[--count]);
        }

        void Address(const ULONG_PTR value) noexcept
        {
            Text("0x");
            Number(value, static_cast<unsigned int>(sizeof(void*) * 2u), 16u);
        }
    };

    void RecordFatalDiagnostic(const char* kind) noexcept
    {
        CrashText text;
        SYSTEMTIME time{};
        GetLocalTime(&time);
        text.Text("\r\n"); text.Text(kind); text.Text(" local=");
        text.Number(time.wYear, 4); text.Character('-');
        text.Number(time.wMonth, 2); text.Character('-');
        text.Number(time.wDay, 2); text.Character(' ');
        text.Number(time.wHour, 2); text.Character(':');
        text.Number(time.wMinute, 2); text.Character(':');
        text.Number(time.wSecond, 2); text.Character('.');
        text.Number(time.wMilliseconds, 3);
        text.Text(" pid="); text.Number(GetCurrentProcessId());
        text.Text(" tid="); text.Number(GetCurrentThreadId());
        text.Text(" uncaught="); text.Number(static_cast<unsigned int>(std::uncaught_exceptions()));
        text.Text("\r\nexception=");

        // The standard exception facility can itself have implementation costs.
        // The writer around it uses fixed storage, with a recursion guard outside.
        try
        {
            const std::exception_ptr exception = std::current_exception();
            if (exception) std::rethrow_exception(exception);
            text.Text("<no active C++ exception>");
        }
        catch (const std::exception& exception)
        {
            text.Text("std::exception what=");
            text.Escaped(exception.what());
        }
        catch (...)
        {
            text.Text("<non-std C++ exception>");
        }
        text.Text("\r\n");

        void* frames[64]{};
        const USHORT count = CaptureStackBackTrace(0, 64, frames, nullptr);
        text.Text("current_stack_frames="); text.Number(count); text.Text("\r\n");
        for (USHORT index = 0; index < count; ++index)
        {
            text.Character('#'); text.Number(index, 2); text.Text(" address=");
            const ULONG_PTR address = reinterpret_cast<ULONG_PTR>(frames[index]);
            text.Address(address);
            MEMORY_BASIC_INFORMATION memory{};
            if (VirtualQuery(frames[index], &memory, sizeof(memory)) &&
                memory.Type == MEM_IMAGE && memory.AllocationBase)
            {
                const ULONG_PTR base = reinterpret_cast<ULONG_PTR>(memory.AllocationBase);
                text.Text(" module_base="); text.Address(base);
                text.Text(" rva="); text.Address(address - base);
                wchar_t modulePath[2048]{};
                const DWORD pathLength = GetModuleFileNameW(
                    static_cast<HMODULE>(memory.AllocationBase), modulePath,
                    static_cast<DWORD>(std::size(modulePath)));
                char utf8Path[8192]{};
                const int utf8Length = pathLength > 0 && pathLength < std::size(modulePath) ?
                    WideCharToMultiByte(CP_UTF8, 0, modulePath, static_cast<int>(pathLength),
                        utf8Path, static_cast<int>(sizeof(utf8Path) - 1u), nullptr, nullptr) : 0;
                text.Text(" module=");
                text.Escaped(utf8Length > 0 ? utf8Path : "<unavailable or truncated>", 512u);
            }
            else text.Text(" module=<not an image>");
            text.Text("\r\n");
        }
        text.Text("END_FATAL_DIAGNOSTIC\r\n");

        if (g_CrashLogFile != INVALID_HANDLE_VALUE)
        {
            DWORD offset = 0;
            while (offset < text.length)
            {
                DWORD written = 0;
                if (!WriteFile(g_CrashLogFile, g_CrashLogText + offset,
                    text.length - offset, &written, nullptr) || written == 0) break;
                offset += written;
            }
            FlushFileBuffers(g_CrashLogFile);
        }
    }

    [[noreturn]] void ClientTerminateDiagnostic() noexcept
    {
        if (InterlockedCompareExchange(&g_TerminateDiagnosticEntered, 1, 0) == 0)
        {
            RecordFatalDiagnostic("CXX_TERMINATE");
            if (g_PreviousTerminateHandler && g_PreviousTerminateHandler != &ClientTerminateDiagnostic)
                g_PreviousTerminateHandler();
        }
        // A diagnostic must never turn a fatal path into a successful return.
        std::abort();
    }

    void ClientAbortDiagnostic(const int signal) noexcept
    {
        // MSVC's terminate handler is thread-local. The process SIGABRT hook
        // also observes default-terminate/explicit-abort in loader workers.
        if (InterlockedCompareExchange(&g_TerminateDiagnosticEntered, 1, 0) == 0)
            RecordFatalDiagnostic("CRT_SIGABRT");
        if (g_PreviousAbortHandler != SIG_DFL && g_PreviousAbortHandler != SIG_IGN &&
            g_PreviousAbortHandler != SIG_ERR && g_PreviousAbortHandler != &ClientAbortDiagnostic)
            g_PreviousAbortHandler(signal);
        // Returning to abort preserves its fatal termination behavior.
    }

    void PrepareTerminateDiagnostic() noexcept
    {
        DWORD length = GetModuleFileNameW(nullptr, g_CrashLogPath,
            static_cast<DWORD>(std::size(g_CrashLogPath)));
        bool absolutePath = length > 0 && length < std::size(g_CrashLogPath);
        for (unsigned int parent = 0; absolutePath && parent < 3u; ++parent)
        {
            while (length > 0 && g_CrashLogPath[length - 1u] != L'\\' &&
                g_CrashLogPath[length - 1u] != L'/') --length;
            if (length == 0) absolutePath = false;
            else g_CrashLogPath[--length] = L'\0';
        }
        constexpr wchar_t suffix[] = L"\\Default\\ClientCrash.user.log";
        if (absolutePath && length + std::size(suffix) <= std::size(g_CrashLogPath))
        {
            for (size_t index = 0; index < std::size(suffix); ++index)
                g_CrashLogPath[length + index] = suffix[index];
        }
        else
        {
            constexpr wchar_t fallback[] = L"ClientCrash.user.log";
            for (size_t index = 0; index < std::size(fallback); ++index)
                g_CrashLogPath[index] = fallback[index];
        }
        g_CrashLogFile = CreateFileW(g_CrashLogPath, FILE_APPEND_DATA,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
            OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (g_CrashLogFile == INVALID_HANDLE_VALUE)
            OutputDebugStringW(L"[CrashDiagnostic] Unable to open ClientCrash.user.log\n");
        // The OS closes this process-lifetime handle after fatal/static teardown.
        g_PreviousAbortHandler = std::signal(SIGABRT, &ClientAbortDiagnostic);
        g_PreviousTerminateHandler = std::set_terminate(&ClientTerminateDiagnostic);
    }

    void WriteExitDiagnostic(const char* reason, const HRESULT result = S_OK)
    {
        // Exit reporting must also work before Engine initialization succeeds.
        wchar_t modulePath[32768]{};
        const DWORD pathLength = GetModuleFileNameW(nullptr, modulePath,
            static_cast<DWORD>(std::size(modulePath)));
        std::filesystem::path logPath = "ClientExit.user.log";
        if (pathLength > 0 && pathLength < std::size(modulePath))
            logPath = std::filesystem::path(modulePath).parent_path().parent_path().parent_path() /
                L"Default" / L"ClientExit.user.log";
        std::ofstream output(logPath, std::ios::binary | std::ios::app);
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
            << " pid=" << GetCurrentProcessId()
            << '\n';
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    PrepareTerminateDiagnostic();

#ifdef _DEBUG
    /* Tracking on, but not the CRT's own end-of-executable report: that runs before
    Engine.dll is detached, so everything an Engine static still held was listed as a leak
    although it is freed moments later. Engine/Private/DebugLeakReport.cpp reports instead,
    from the last static destroyed in that module. */
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF);
    /* The leak dump names an allocation number but not a call stack, and the blocks it
    reports are container bookkeeping (_Container_proxy: one pointer to the container plus
    a null iterator list), which an address alone cannot attribute. Set LOSTARK_BREAK_ALLOC
    to one of the numbers in braces from the dump and the debugger stops on that exact
    allocation, so the next run says where it came from. Off unless the variable is set. */
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

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    std::string settingsStatus;
    if (!Client::CUserSettings::Get().Load_Persisted(settingsStatus))
        OutputDebugStringA(("[Settings] " + settingsStatus + "\n").c_str());

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

    // Timer_60 remains the shared rendered-frame clock used by raid-entry animation.
    if (FAILED(CGameInstance::Get().Add_Timer(TEXT("Timer_60"))))
    {
        WriteExitDiagnostic("Timer_60 creation failed", E_FAIL);
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENT));
    MSG msg{};

    while (true)
    {
        // Drain queued input before the next frame; rendering is not capped at 60 Hz.
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (WM_QUIT == msg.message)
                break;
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        if (WM_QUIT == msg.message)
        {
            WriteExitDiagnostic("WM_QUIT", S_OK);
            break;
        }

        std::string displayStatus;
        if (!Client::CClientWindowDisplay::Process_PendingResize(displayStatus))
        {
            WriteExitDiagnostic("Display resize failed", E_FAIL);
            break;
        }
        // Keep consuming network events and frame-owned render submissions while minimized.
        // The last nonzero GPU size remains valid; throttle these background frames to 20 Hz.
        if (Client::CClientWindowDisplay::Is_Minimized())
            MsgWaitForMultipleObjects(0, nullptr, FALSE, 50, QS_ALLINPUT);

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

        // System option frame limit (off by default): outside the profiled frame.
        pMainApp->Limit_FrameRate();

        if (FAILED(hRenderResult))
        {
            WriteExitDiagnostic("Render failed", hRenderResult);
            break;
        }
    }

    Client::CClientWindowDisplay::Shutdown();
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
    wcex.lpszMenuName   = nullptr;
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
       WriteExitDiagnostic("Initial display configuration failed", E_FAIL);
       DestroyWindow(hWnd);
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
    if (message == WM_SIZE)
        Client::CClientWindowDisplay::Queue_Size(LOWORD(lParam), HIWORD(lParam), wParam == SIZE_MINIMIZED);
    if (message == WM_DPICHANGED)
    {
        Client::CClientWindowDisplay::On_DpiChanged(HIWORD(wParam), *reinterpret_cast<const RECT*>(lParam));
        return 0;
    }
    if (message == WM_GETMINMAXINFO)
    {
        auto* limits = reinterpret_cast<MINMAXINFO*>(lParam);
        RECT minimum{0, 0, 640, 480};
        AdjustWindowRectExForDpi(&minimum, static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_STYLE)),
            FALSE, static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_EXSTYLE)), GetDpiForWindow(hWnd));
        limits->ptMinTrackSize.x = minimum.right - minimum.left;
        limits->ptMinTrackSize.y = minimum.bottom - minimum.top;
        // A physical client resolution can need a larger outer frame than the desktop.
        // Keep maximize's monitor bounds, but allow explicit resolution/window resizing.
        RECT maximum{0, 0, 16384, 16384};
        AdjustWindowRectExForDpi(&maximum, static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_STYLE)),
            FALSE, static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_EXSTYLE)), GetDpiForWindow(hWnd));
        limits->ptMaxTrackSize.x = maximum.right - maximum.left;
        limits->ptMaxTrackSize.y = maximum.bottom - maximum.top;
        return 0;
    }

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
    case WM_MOUSEWHEEL:
        /* Runtime UI lists scroll from this message (see CUIInputRouter::On_MouseWheel); ImGui's
        handler above only records the wheel for its own windows and never consumes it. */
        Client::CUIInputRouter::Get().On_MouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
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

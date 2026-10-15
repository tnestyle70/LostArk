#include "ClientWindowDisplay.h"

//Engine의 fullscreen 전환과 viewport/render target 크기 변경을 호출한다.
#include "GameInstance.h"

//창을 작업 영역 중앙에 배치할 때 음수 여백을 0으로 제한한다.
#include <algorithm>

//객체를 만들지 않고 창 하나의 상태를 공유한다. 실제 저장 공간은 이 CPP에서 한 번 정의한다.
HWND Client::CClientWindowDisplay::m_hWnd = nullptr;
bool Client::CClientWindowDisplay::m_bEngineReady = false;
bool Client::CClientWindowDisplay::m_bMinimized = false;
bool Client::CClientWindowDisplay::m_bApplying = false;
bool Client::CClientWindowDisplay::m_bPendingResize = false;
uint32_t Client::CClientWindowDisplay::m_iPendingWidth = 0;
uint32_t Client::CClientWindowDisplay::m_iPendingHeight = 0;
Client::USER_DISPLAY_SETTINGS Client::CClientWindowDisplay::m_AppliedSettings{};

bool Client::CClientWindowDisplay::Fail(string& status, const char* operation, HRESULT result)
{
    //설정 화면과 디버거에서 같은 실패 원인을 읽도록 상태를 함께 남긴다.
    status = string(operation) + " failed (" + std::to_string(static_cast<unsigned long>(result)) + ").";
    OutputDebugStringA(("[Display] " + status + "\n").c_str());
    return false;
}

bool Client::CClientWindowDisplay::Set_Window(const USER_DISPLAY_SETTINGS& settings, string& status)
{
    //window가 없거나 settings에 대한 범위가 맞지 않을 경우
    if (!m_hWnd || settings.width < 640 || settings.height < 480 ||
        settings.width > 16384 || settings.height > 16384)
    {
        return Fail(status, "Display dimensions", E_INVALIDARG);
    }

    //창이 있는 모니터를 고른다. 화면 밖의 창은 가장 가까운 모니터를 사용한다.
    const HMONITOR hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);

    //cbSize로 구조체 크기를 알리고, 모니터 전체 영역과 작업 영역을 받는다.
    MONITORINFO MonitorInfo{};
    MonitorInfo.cbSize = sizeof(MONITORINFO);

    if (!GetMonitorInfoW(hMonitor, &MonitorInfo))
    {
        const DWORD dwError = GetLastError();
        const HRESULT hrResult = HRESULT_FROM_WIN32(dwError);
        return Fail(status, "Read monitor", hrResult);
    }

    //windowed만 테두리와 제목 표시줄을 사용한다. 기존 표시 여부는 유지한다.
    const bool bWindowed = settings.mode == USER_WINDOW_MODE::WINDOWED;
    const LONG_PTR lPreviousStyle = GetWindowLongPtrW(m_hWnd, GWL_STYLE);
    const LONG_PTR lStyle = (bWindowed ? WS_OVERLAPPEDWINDOW : WS_POPUP) |
        (lPreviousStyle & WS_VISIBLE);

    //이전 style이 0이어도 정상일 수 있으므로 오류를 비운 뒤 반환값과 오류를 함께 확인한다.
    SetLastError(0);
    const LONG_PTR lChangedStyle = SetWindowLongPtrW(m_hWnd, GWL_STYLE, lStyle);

    if (!lChangedStyle)
    {
        const DWORD dwError = GetLastError();
        if (dwError != 0)
        {
            const HRESULT hrResult = HRESULT_FROM_WIN32(dwError);
            return Fail(status, "Change window style", hrResult);
        }
    }

    //요청 해상도는 client 영역의 물리 픽셀이다. windowed는 DPI에 맞는 창틀 크기를 더한다.
    RECT rcWindow{ 0, 0, static_cast<LONG>(settings.width), static_cast<LONG>(settings.height) };

    if (bWindowed)
    {
        const DWORD dwExtendedStyle = static_cast<DWORD>(GetWindowLongPtrW(m_hWnd, GWL_EXSTYLE));
        const UINT iDpi = GetDpiForWindow(m_hWnd);

        if (!AdjustWindowRectExForDpi(&rcWindow, static_cast<DWORD>(lStyle), FALSE,
            dwExtendedStyle, iDpi))
        {
            const DWORD dwError = GetLastError();
            const HRESULT hrResult = HRESULT_FROM_WIN32(dwError);
            return Fail(status, "Calculate window frame", hrResult);
        }
    }

    int iWidth = rcWindow.right - rcWindow.left;
    int iHeight = rcWindow.bottom - rcWindow.top;
    int iX = MonitorInfo.rcMonitor.left;
    int iY = MonitorInfo.rcMonitor.top;

    //borderless는 모니터 전체를 채우고, windowed는 작업 표시줄을 제외한 영역 중앙에 놓는다.
    if (settings.mode == USER_WINDOW_MODE::BORDERLESS)
    {
        iWidth = MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left;
        iHeight = MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top;
    }
    else if (bWindowed)
    {
        const LONG iWorkWidth = MonitorInfo.rcWork.right - MonitorInfo.rcWork.left;
        const LONG iWorkHeight = MonitorInfo.rcWork.bottom - MonitorInfo.rcWork.top;
        iX = MonitorInfo.rcWork.left + (std::max)(0L, (iWorkWidth - iWidth) / 2);
        iY = MonitorInfo.rcWork.top + (std::max)(0L, (iWorkHeight - iHeight) / 2);
    }

    //최대화·최소화를 해제한 뒤 새 style과 바깥 창 영역을 반영한다.
    if (IsZoomed(m_hWnd) || IsIconic(m_hWnd))
    {
        ShowWindow(m_hWnd, SW_RESTORE);
    }

    if (!SetWindowPos(m_hWnd, HWND_NOTOPMOST, iX, iY, iWidth, iHeight,
        SWP_FRAMECHANGED | SWP_NOACTIVATE))
    {
        const DWORD dwError = GetLastError();
        const HRESULT hrResult = HRESULT_FROM_WIN32(dwError);
        return Fail(status, "Resize window", hrResult);
    }

    return true;
}

bool Client::CClientWindowDisplay::Resize_ToClient(string& status)
{
    //창 모드 변경 뒤 실제 client 크기를 읽는다. 최소화된 0 크기는 GPU에 전달하지 않는다.
    RECT rcClient{};
    if (!GetClientRect(m_hWnd, &rcClient))
    {
        const DWORD dwError = GetLastError();
        const HRESULT hrResult = HRESULT_FROM_WIN32(dwError);
        return Fail(status, "Read client size", hrResult);
    }

    const uint32_t iWidth = static_cast<uint32_t>(rcClient.right - rcClient.left);
    const uint32_t iHeight = static_cast<uint32_t>(rcClient.bottom - rcClient.top);

    if (!iWidth || !iHeight)
    {
        return Fail(status, "Empty client area", E_INVALIDARG);
    }

    //Engine 적용이 성공한 뒤에만 메시지로 예약됐던 resize를 소비한다.
    const HRESULT hrResult = Engine::CGameInstance::Get().Resize_Viewport(iWidth, iHeight);
    if (FAILED(hrResult))
    {
        return Fail(status, "Resize render targets", hrResult);
    }

    m_bPendingResize = false;
    m_bMinimized = false;

    const UINT iDpi = GetDpiForWindow(m_hWnd);
    status = "Rendering at " + std::to_string(iWidth) + " x " + std::to_string(iHeight) +
        " physical pixels; monitor DPI " + std::to_string(iDpi) + ".";
    OutputDebugStringA(("[Display] " + status + "\n").c_str());
    return true;
}

bool Client::CClientWindowDisplay::Configure_InitialWindow(HWND window,
    const USER_DISPLAY_SETTINGS& settings, string& status)
{
    //Client.cpp가 만든 창을 빌려 사용한다. 실패한 창의 파괴는 호출자가 맡는다.
    m_hWnd = window;
    if (!Set_Window(settings, status))
    {
        return false;
    }

    //Engine 생성 전에는 exclusive fullscreen을 시작하지 않는다.
    m_AppliedSettings = settings;
    if (m_AppliedSettings.mode == USER_WINDOW_MODE::FULLSCREEN)
    {
        m_AppliedSettings.mode = USER_WINDOW_MODE::WINDOWED;
    }

    return true;
}

bool Client::CClientWindowDisplay::Attach_Engine(string& status)
{
    //MainApp이 장치를 준비한 뒤 저장된 창 모드와 viewport를 함께 적용한다.
    m_bEngineReady = true;
    if (Apply(CUserSettings::Get().Get_DisplaySettings(), status))
    {
        return true;
    }

    //저장 설정이 실패해도 windowed로 재시도해 설정 화면에 들어갈 수 있게 한다.
    const string strRequestedFailure = status;
    USER_DISPLAY_SETTINGS FallbackSettings = m_AppliedSettings;
    FallbackSettings.mode = USER_WINDOW_MODE::WINDOWED;
    string strFallbackStatus;

    if (!Apply(FallbackSettings, strFallbackStatus))
    {
        return false;
    }

    CUserSettings::Get().Adopt_AppliedDisplay(FallbackSettings);
    status = strRequestedFailure + " Started in windowed mode; choose a supported display mode in Settings.";
    OutputDebugStringA(("[Display] " + status + "\n").c_str());
    return true;
}

bool Client::CClientWindowDisplay::Apply(const USER_DISPLAY_SETTINGS& settings, string& status)
{
    //창 변경은 메시지를 다시 발생시킬 수 있다. Engine 준비 전이나 적용 중에는 새 적용을 거절한다.
    if (!m_bEngineReady || m_bApplying)
    {
        return Fail(status, "Display apply state", E_UNEXPECTED);
    }

    m_bApplying = true;

    //실패하면 되돌릴 수 있도록 이전 창 배치·style·viewport를 보관한다.
    const USER_DISPLAY_SETTINGS PreviousSettings = m_AppliedSettings;
    WINDOWPLACEMENT PreviousPlacement{ sizeof(WINDOWPLACEMENT) };
    GetWindowPlacement(m_hWnd, &PreviousPlacement);
    const LONG_PTR lPreviousStyle = GetWindowLongPtrW(m_hWnd, GWL_STYLE);
    RECT rcPreviousWindow{};
    GetWindowRect(m_hWnd, &rcPreviousWindow);
    auto& GameInstance = Engine::CGameInstance::Get();
    const auto PreviousViewport = GameInstance.Get_ViewportSize();

    //기존 exclusive를 해제한 상태에서 창을 바꾸고, 요청이 fullscreen이면 다시 진입한다.
    HRESULT hrResult = GameInstance.Set_FullscreenMode(false, PreviousSettings.width, PreviousSettings.height);
    bool bSuccess = SUCCEEDED(hrResult);

    if (!bSuccess)
    {
        Fail(status, "Leave fullscreen", hrResult);
    }

    if (bSuccess)
    {
        bSuccess = Set_Window(settings, status);
    }

    if (bSuccess && settings.mode == USER_WINDOW_MODE::FULLSCREEN)
    {
        hrResult = GameInstance.Set_FullscreenMode(true, settings.width, settings.height);
        bSuccess = SUCCEEDED(hrResult);

        if (!bSuccess)
        {
            Fail(status, "Enter fullscreen", hrResult);
        }
    }

    //실제 창 크기를 GPU에 적용하고, borderless 외에는 요청한 물리 해상도와 같은지 확인한다.
    if (bSuccess)
    {
        bSuccess = Resize_ToClient(status);
    }

    if (bSuccess && settings.mode != USER_WINDOW_MODE::BORDERLESS)
    {
        const auto ActualViewport = GameInstance.Get_ViewportSize();
        if (ActualViewport.x != static_cast<float>(settings.width) ||
            ActualViewport.y != static_cast<float>(settings.height))
        {
            bSuccess = Fail(status, "Requested physical resolution was not realized", E_FAIL);
        }
    }

    if (bSuccess)
    {
        //창과 GPU가 모두 준비된 설정만 다음 복원의 기준으로 삼는다.
        m_AppliedSettings = settings;
    }
    else
    {
        //Win32 변경이 일부 끝난 상태일 수 있어 창·모드·viewport를 순서대로 복원 시도한다.
        const HRESULT hrLeaveResult = GameInstance.Set_FullscreenMode(
            false, PreviousSettings.width, PreviousSettings.height);
        SetWindowLongPtrW(m_hWnd, GWL_STYLE, lPreviousStyle);
        const BOOL bRestoredWindow = SetWindowPos(m_hWnd, HWND_NOTOPMOST,
            rcPreviousWindow.left, rcPreviousWindow.top,
            rcPreviousWindow.right - rcPreviousWindow.left,
            rcPreviousWindow.bottom - rcPreviousWindow.top, SWP_FRAMECHANGED | SWP_NOACTIVATE);
        SetWindowPlacement(m_hWnd, &PreviousPlacement);

        HRESULT hrModeResult = S_OK;
        if (PreviousSettings.mode == USER_WINDOW_MODE::FULLSCREEN)
        {
            hrModeResult = GameInstance.Set_FullscreenMode(
                true, PreviousSettings.width, PreviousSettings.height);
        }

        const HRESULT hrResizeResult = GameInstance.Resize_Viewport(
            static_cast<uint32_t>(PreviousViewport.x), static_cast<uint32_t>(PreviousViewport.y));

        //복원 호출도 실패할 수 있다. 완전 복원을 보장하지 않고 실패 상태에 남긴다.
        if (FAILED(hrLeaveResult) || !bRestoredWindow || FAILED(hrModeResult) || FAILED(hrResizeResult))
        {
            status += " Previous display could not be fully restored.";
        }

        m_bPendingResize = false;
    }

    m_bApplying = false;
    return bSuccess;
}

void Client::CClientWindowDisplay::Queue_Size(uint32_t width, uint32_t height, bool minimized)
{
    //최소화된 크기는 GPU에 적용하지 않는다. 정상 크기 메시지가 오면 다시 예약한다.
    m_bMinimized = minimized || width == 0 || height == 0;
    if (m_bMinimized)
    {
        return;
    }

    m_iPendingWidth = width;
    m_iPendingHeight = height;
    m_bPendingResize = true;
}

void Client::CClientWindowDisplay::On_DpiChanged(uint32_t dpi, const RECT& suggested)
{
    //모드 적용 중의 재진입과 exclusive 상태에서는 Windows의 DPI 배치를 적용하지 않는다.
    if (!m_hWnd || m_bApplying || !dpi || m_AppliedSettings.mode == USER_WINDOW_MODE::FULLSCREEN)
    {
        return;
    }

    //borderless는 새 위치가 속한 모니터 전체 영역을 채운다.
    if (m_AppliedSettings.mode == USER_WINDOW_MODE::BORDERLESS)
    {
        const HMONITOR hMonitor = MonitorFromRect(&suggested, MONITOR_DEFAULTTONEAREST);
        MONITORINFO MonitorInfo{};
        MonitorInfo.cbSize = sizeof(MONITORINFO);

        if (GetMonitorInfoW(hMonitor, &MonitorInfo))
        {
            SetWindowPos(m_hWnd, nullptr, MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top, SWP_NOZORDER | SWP_NOACTIVATE);
        }

        return;
    }

    RECT rcClient{};
    if (!GetClientRect(m_hWnd, &rcClient))
    {
        return;
    }

    //선택 해상도는 물리 픽셀이다. client 크기는 유지하고 새 DPI에 맞는 창틀만 계산한다.
    const DWORD dwStyle = static_cast<DWORD>(GetWindowLongPtrW(m_hWnd, GWL_STYLE));
    const DWORD dwExtendedStyle = static_cast<DWORD>(GetWindowLongPtrW(m_hWnd, GWL_EXSTYLE));

    if (AdjustWindowRectExForDpi(&rcClient, dwStyle, FALSE, dwExtendedStyle, dpi))
    {
        SetWindowPos(m_hWnd, nullptr, suggested.left, suggested.top,
            rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

bool Client::CClientWindowDisplay::Process_PendingResize(string& status)
{
    //메시지 처리 중에는 기록만 하고, Engine이 준비된 다음 프레임 경계에서 GPU를 변경한다.
    if (!m_bEngineReady || !m_bPendingResize || m_bMinimized || m_bApplying)
    {
        return true;
    }

    m_bPendingResize = false;
    const HRESULT hrResult = Engine::CGameInstance::Get().Resize_Viewport(m_iPendingWidth, m_iPendingHeight);

    if (SUCCEEDED(hrResult))
    {
        return true;
    }

    //새 크기가 실패하면 마지막 성공 설정으로 복원한다. 반환값은 이 복원의 성공 여부다.
    const string strFailure = "Window render resize failed (" +
        std::to_string(static_cast<unsigned long>(hrResult)) + ").";
    string strRestoreStatus;
    const bool bRestored = Apply(m_AppliedSettings, strRestoreStatus);
    status = strFailure + (bRestored ? " Previous display restored." : " " + strRestoreStatus);
    OutputDebugStringA(("[Display] " + status + "\n").c_str());
    return bRestored;
}

bool Client::CClientWindowDisplay::Is_Minimized()
{
    return m_bMinimized;
}

void Client::CClientWindowDisplay::Shutdown()
{
    //Engine이 살아 있을 때 exclusive를 해제하고 설정 화면의 적용 연결을 끊는다.
    if (m_bEngineReady)
    {
        (void)Engine::CGameInstance::Get().Set_FullscreenMode(
            false, m_AppliedSettings.width, m_AppliedSettings.height);
    }

    m_bEngineReady = false;
    CUserSettings::Get().Set_DisplayApplyCallback({});
}

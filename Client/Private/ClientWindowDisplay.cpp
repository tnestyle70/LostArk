#include "ClientWindowDisplay.h"

#include "GameInstance.h"

#include <algorithm>

namespace
{
    HWND s_window = nullptr;
    bool s_engineReady = false;
    bool s_minimized = false;
    bool s_applying = false;
    bool s_pending = false;
    uint32_t s_pendingWidth = 0, s_pendingHeight = 0;
    Client::USER_DISPLAY_SETTINGS s_applied{};

    bool Fail(std::string& status, const char* operation, const HRESULT result)
    {
        status = std::string(operation) + " failed (" + std::to_string(static_cast<unsigned long>(result)) + ").";
        OutputDebugStringA(("[Display] " + status + "\n").c_str());
        return false;
    }

    bool Set_Window(const Client::USER_DISPLAY_SETTINGS& settings, std::string& status)
    {
        if (!s_window || settings.width < 640 || settings.height < 480 ||
            settings.width > 16384 || settings.height > 16384)
            return Fail(status, "Display dimensions", E_INVALIDARG);
        MONITORINFO monitor{ sizeof(MONITORINFO) };
        if (!GetMonitorInfoW(MonitorFromWindow(s_window, MONITOR_DEFAULTTONEAREST), &monitor))
            return Fail(status, "Read monitor", HRESULT_FROM_WIN32(GetLastError()));
        const bool windowed = settings.mode == Client::USER_WINDOW_MODE::WINDOWED;
        const LONG_PTR style = (windowed ? WS_OVERLAPPEDWINDOW : WS_POPUP) |
            (GetWindowLongPtrW(s_window, GWL_STYLE) & WS_VISIBLE);
        SetLastError(0);
        if (!SetWindowLongPtrW(s_window, GWL_STYLE, style) && GetLastError())
            return Fail(status, "Change window style", HRESULT_FROM_WIN32(GetLastError()));
        RECT rectangle{0, 0, static_cast<LONG>(settings.width), static_cast<LONG>(settings.height)};
        if (windowed && !AdjustWindowRectExForDpi(&rectangle, static_cast<DWORD>(style), FALSE,
            static_cast<DWORD>(GetWindowLongPtrW(s_window, GWL_EXSTYLE)), GetDpiForWindow(s_window)))
            return Fail(status, "Calculate window frame", HRESULT_FROM_WIN32(GetLastError()));
        int width = rectangle.right - rectangle.left, height = rectangle.bottom - rectangle.top;
        int x = monitor.rcMonitor.left, y = monitor.rcMonitor.top;
        if (settings.mode == Client::USER_WINDOW_MODE::BORDERLESS)
        {
            width = monitor.rcMonitor.right - monitor.rcMonitor.left;
            height = monitor.rcMonitor.bottom - monitor.rcMonitor.top;
        }
        else if (windowed)
        {
            x = monitor.rcWork.left + (std::max)(0L, (monitor.rcWork.right - monitor.rcWork.left - width) / 2);
            y = monitor.rcWork.top + (std::max)(0L, (monitor.rcWork.bottom - monitor.rcWork.top - height) / 2);
        }
        if (IsZoomed(s_window) || IsIconic(s_window)) ShowWindow(s_window, SW_RESTORE);
        if (!SetWindowPos(s_window, HWND_NOTOPMOST, x, y, width, height, SWP_FRAMECHANGED | SWP_NOACTIVATE))
            return Fail(status, "Resize window", HRESULT_FROM_WIN32(GetLastError()));
        return true;
    }

    bool Resize_ToClient(std::string& status)
    {
        RECT client{};
        if (!GetClientRect(s_window, &client))
            return Fail(status, "Read client size", HRESULT_FROM_WIN32(GetLastError()));
        const auto width = static_cast<uint32_t>(client.right - client.left);
        const auto height = static_cast<uint32_t>(client.bottom - client.top);
        if (!width || !height) return Fail(status, "Empty client area", E_INVALIDARG);
        const HRESULT result = Engine::CGameInstance::Get().Resize_Viewport(width, height);
        if (FAILED(result)) return Fail(status, "Resize render targets", result);
        s_pending = false;
        s_minimized = false;
        status = "Rendering at " + std::to_string(width) + " x " + std::to_string(height) +
            " physical pixels; monitor DPI " + std::to_string(GetDpiForWindow(s_window)) + ".";
        OutputDebugStringA(("[Display] " + status + "\n").c_str());
        return true;
    }
}

bool Client::CClientWindowDisplay::Configure_InitialWindow(HWND window,
    const USER_DISPLAY_SETTINGS& settings, string& status)
{
    s_window = window;
    if (!Set_Window(settings, status)) return false;
    s_applied = settings;
    if (s_applied.mode == USER_WINDOW_MODE::FULLSCREEN) s_applied.mode = USER_WINDOW_MODE::WINDOWED;
    return true;
}

bool Client::CClientWindowDisplay::Attach_Engine(string& status)
{
    s_engineReady = true;
    if (Apply(CUserSettings::Get().Get_DisplaySettings(), status)) return true;
    // A disconnected monitor must not make a saved exclusive mode prevent opening ESC.
    const string requestedFailure = status;
    USER_DISPLAY_SETTINGS fallback = s_applied;
    fallback.mode = USER_WINDOW_MODE::WINDOWED;
    string fallbackStatus;
    if (!Apply(fallback, fallbackStatus)) return false;
    CUserSettings::Get().Adopt_AppliedDisplay(fallback);
    status = requestedFailure + " Started in windowed mode; choose a supported display mode in Settings.";
    OutputDebugStringA(("[Display] " + status + "\n").c_str());
    return true;
}

bool Client::CClientWindowDisplay::Apply(const USER_DISPLAY_SETTINGS& settings, string& status)
{
    if (!s_engineReady || s_applying) return Fail(status, "Display apply state", E_UNEXPECTED);
    s_applying = true;
    const USER_DISPLAY_SETTINGS previous = s_applied;
    WINDOWPLACEMENT previousPlacement{ sizeof(WINDOWPLACEMENT) };
    GetWindowPlacement(s_window, &previousPlacement);
    const LONG_PTR previousStyle = GetWindowLongPtrW(s_window, GWL_STYLE);
    RECT previousWindow{};
    GetWindowRect(s_window, &previousWindow);
    auto& game = Engine::CGameInstance::Get();
    const auto previousViewport = game.Get_ViewportSize();
    HRESULT result = game.Set_FullscreenMode(false, previous.width, previous.height);
    bool success = SUCCEEDED(result);
    if (!success) Fail(status, "Leave fullscreen", result);
    if (success) success = Set_Window(settings, status);
    if (success && settings.mode == USER_WINDOW_MODE::FULLSCREEN)
    {
        result = game.Set_FullscreenMode(true, settings.width, settings.height);
        success = SUCCEEDED(result);
        if (!success) Fail(status, "Enter fullscreen", result);
    }
    if (success) success = Resize_ToClient(status);
    if (success && settings.mode != USER_WINDOW_MODE::BORDERLESS)
    {
        const auto actual = game.Get_ViewportSize();
        if (actual.x != static_cast<float>(settings.width) || actual.y != static_cast<float>(settings.height))
            success = Fail(status, "Requested physical resolution was not realized", E_FAIL);
    }
    if (success) s_applied = settings;
    else
    {
        const HRESULT leaveResult = game.Set_FullscreenMode(false, previous.width, previous.height);
        SetWindowLongPtrW(s_window, GWL_STYLE, previousStyle);
        const BOOL restoredWindow = SetWindowPos(s_window, HWND_NOTOPMOST,
            previousWindow.left, previousWindow.top, previousWindow.right - previousWindow.left,
            previousWindow.bottom - previousWindow.top, SWP_FRAMECHANGED | SWP_NOACTIVATE);
        SetWindowPlacement(s_window, &previousPlacement);
        HRESULT modeResult = S_OK;
        if (previous.mode == USER_WINDOW_MODE::FULLSCREEN)
            modeResult = game.Set_FullscreenMode(true, previous.width, previous.height);
        const HRESULT resizeResult = game.Resize_Viewport(
            static_cast<uint32_t>(previousViewport.x), static_cast<uint32_t>(previousViewport.y));
        if (FAILED(leaveResult) || !restoredWindow || FAILED(modeResult) || FAILED(resizeResult))
            status += " Previous display could not be fully restored.";
        s_pending = false;
    }
    s_applying = false;
    return success;
}

void Client::CClientWindowDisplay::Queue_Size(uint32_t width, uint32_t height, bool minimized)
{
    s_minimized = minimized || width == 0 || height == 0;
    if (s_minimized) return;
    s_pendingWidth = width;
    s_pendingHeight = height;
    s_pending = true;
}

void Client::CClientWindowDisplay::On_DpiChanged(uint32_t dpi, const RECT& suggested)
{
    if (!s_window || s_applying || !dpi || s_applied.mode == USER_WINDOW_MODE::FULLSCREEN) return;
    if (s_applied.mode == USER_WINDOW_MODE::BORDERLESS)
    {
        MONITORINFO monitor{ sizeof(MONITORINFO) };
        if (GetMonitorInfoW(MonitorFromRect(&suggested, MONITOR_DEFAULTTONEAREST), &monitor))
            SetWindowPos(s_window, nullptr, monitor.rcMonitor.left, monitor.rcMonitor.top,
                monitor.rcMonitor.right - monitor.rcMonitor.left, monitor.rcMonitor.bottom - monitor.rcMonitor.top,
                SWP_NOZORDER | SWP_NOACTIVATE);
        return;
    }
    RECT client{};
    if (!GetClientRect(s_window, &client)) return;
    // The selected resolution is in physical pixels, not DIP. Only the non-client frame changes.
    if (AdjustWindowRectExForDpi(&client, static_cast<DWORD>(GetWindowLongPtrW(s_window, GWL_STYLE)),
        FALSE, static_cast<DWORD>(GetWindowLongPtrW(s_window, GWL_EXSTYLE)), dpi))
        SetWindowPos(s_window, nullptr, suggested.left, suggested.top, client.right - client.left,
            client.bottom - client.top, SWP_NOZORDER | SWP_NOACTIVATE);
}

bool Client::CClientWindowDisplay::Process_PendingResize(string& status)
{
    if (!s_engineReady || !s_pending || s_minimized || s_applying) return true;
    s_pending = false;
    const HRESULT result = Engine::CGameInstance::Get().Resize_Viewport(s_pendingWidth, s_pendingHeight);
    if (SUCCEEDED(result)) return true;
    const string failure = "Window render resize failed (" + std::to_string(static_cast<unsigned long>(result)) + ").";
    string restoreStatus;
    const bool restored = Apply(s_applied, restoreStatus);
    status = failure + (restored ? " Previous display restored." : " " + restoreStatus);
    OutputDebugStringA(("[Display] " + status + "\n").c_str());
    return restored;
}

bool Client::CClientWindowDisplay::Is_Minimized() { return s_minimized; }

void Client::CClientWindowDisplay::Shutdown()
{
    if (s_engineReady) (void)Engine::CGameInstance::Get().Set_FullscreenMode(false, s_applied.width, s_applied.height);
    s_engineReady = false;
    CUserSettings::Get().Set_DisplayApplyCallback({});
}

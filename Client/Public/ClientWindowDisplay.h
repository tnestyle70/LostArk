#pragma once

#include "UserSettingsDocument.h"

NS_BEGIN(Client)

/* Owns the physical Win32 client area. UI documents keep their reference coordinates. */
class CClientWindowDisplay final
{
public:
    static bool Configure_InitialWindow(HWND window, const USER_DISPLAY_SETTINGS& settings, string& status);
    static bool Attach_Engine(string& status);
    static bool Apply(const USER_DISPLAY_SETTINGS& settings, string& status);
    static void Queue_Size(uint32_t width, uint32_t height, bool minimized);
    static void On_DpiChanged(uint32_t dpi, const RECT& suggested);
    static bool Process_PendingResize(string& status);
    static bool Is_Minimized();
    static void Shutdown();
};

NS_END

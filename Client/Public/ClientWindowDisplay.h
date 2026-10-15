#pragma once

#include <string>

//창 모드·물리 해상도 설정과 적용 상태 문자열을 사용한다.
#include "UserSettingsDocument.h"

NS_BEGIN(Client)

//Win32 창의 물리 client 영역과 Engine viewport를 맞춘다. UI 문서의 기준 좌표는 바꾸지 않는다.
//창 생성·파괴는 Client.cpp가 맡으며, 이 클래스는 그 HWND를 빌려 사용한다.
class CClientWindowDisplay final
{
public:
    //Engine 생성 전에 창 위치와 style을 적용한다. 중간 실패의 창 변경은 되돌리지 않는다.
    static bool Configure_InitialWindow(HWND window, const USER_DISPLAY_SETTINGS& settings, std::string& status);

    //MainApp의 Engine 초기화 뒤 저장 설정을 적용하고, 실패하면 windowed로 재시도한다.
    static bool Attach_Engine(std::string& status);

    //설정 화면에서 창·fullscreen·viewport를 바꾼다. 실패하면 이전 상태를 복원 시도한다.
    static bool Apply(const USER_DISPLAY_SETTINGS& settings, std::string& status);

    //WM_SIZE에서 최신 크기만 보관한다. render target 재생성은 프레임 루프에서 수행한다.
    static void Queue_Size(uint32_t width, uint32_t height, bool minimized);

    //WM_DPICHANGED의 새 DPI와 제안 위치를 적용한다. windowed의 물리 client 크기는 유지한다.
    static void On_DpiChanged(uint32_t dpi, const RECT& suggested);

    //대기 크기를 적용한다. 실패 뒤 이전 설정 복원에 성공해도 true이며 실패 이력은 status에 남긴다.
    static bool Process_PendingResize(std::string& status);

    //Client 프레임 루프가 최소화 동안 대기 시간을 넣을 때 조회한다.
    static bool Is_Minimized();

    //Engine 해제 전에 exclusive fullscreen과 설정 callback을 해제한다. 창은 파괴하지 않는다.
    static void Shutdown();

private:
    //실패한 작업과 HRESULT를 호출자의 상태 문자열과 디버그 출력에 남긴다.
    static bool Fail(std::string& status, const char* operation, HRESULT result);

    //Win32 창의 style·위치·크기를 바꾼다. 중간 실패의 복원은 Apply가 담당한다.
    static bool Set_Window(const USER_DISPLAY_SETTINGS& settings, std::string& status);

    //실제 client 크기를 Engine에 적용한다. 성공하면 대기 resize와 최소화 표시를 해제한다.
    static bool Resize_ToClient(std::string& status);

private:
    //창 하나의 공유 상태다. Win32 메시지와 적용 함수 모두 Client 프레임 스레드에서 접근한다.
    static HWND m_hWnd; //Client.cpp 소유 창의 비소유 handle. 이 클래스에서 해제하지 않는다.
    static bool m_bEngineReady; //Engine 준비 전에는 viewport와 fullscreen을 변경하지 않는다.
    static bool m_bMinimized; //최소화 또는 크기 0이면 resize를 보류한다.
    static bool m_bApplying; //창 변경 중 들어오는 메시지가 모드 적용을 다시 시작하지 못하게 한다.

    //WM_SIZE가 최신 물리 픽셀 크기를 기록하고 다음 프레임이 한 번 소비한다.
    static bool m_bPendingResize;
    static uint32_t m_iPendingWidth;
    static uint32_t m_iPendingHeight;

    //마지막으로 적용한 설정이다. 새 적용 실패 시 복원 기준으로 사용한다.
    static USER_DISPLAY_SETTINGS m_AppliedSettings;
};

NS_END

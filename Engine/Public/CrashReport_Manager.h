#pragma once

//종료 콜백 타입을 사용한다.
#include <exception>
#include "Engine_Defines.h"

NS_BEGIN(Engine)

//게임 준비 전과 GameInstance 정리 중에도 치명적 종료를 기록한다.
//객체를 생성하지 않는 정적 기능이므로 singleton과 GameInstance 소유가 필요 없다.
class CCrashReport_Manager final
{
private:
    CCrashReport_Manager() = delete;

public:
    //wWinMain의 단일 스레드 시작 구간에서 한 번 설치한다. 경로 문자열은 저장하지 않는다.
    //준비 실패는 false로 알리며 기존 종료 처리는 유지한다.
    static ENGINE_DLL bool Install(const wchar_t* logPath) noexcept;

private:
    //고정 버퍼에 문자열을 만드는 내부 작성기다. 전체 정의는 CPP에 둔다.
    struct CrashText;
    using AbortSignalHandler = void (*)(int);

    static void RecordFatalDiagnostic(const char* kind) noexcept;
    [[noreturn]] static void OnTerminate() noexcept;
    static void OnAbort(int signal) noexcept;

private:
    //설치는 worker 생성 전에 끝낸다. callback에서 재설치하지 않는다.
    static bool m_bInstalled;

    //파일과 출력 공간은 게임 객체보다 오래 유지하며 파일은 프로세스 종료 때 OS가 회수한다.
    static HANDLE m_hLogFile;
    static char m_szTextBuffer[65536];

    //Interlocked로 작성자 한 명만 통과시켜 중첩 종료와 동시 버퍼 접근을 막는다.
    static volatile LONG m_lRecordEntered;

    //진단 기록 뒤 기존 종료 동작으로 이어간다.
    static std::terminate_handler m_pPreviousTerminateHandler;
    static AbortSignalHandler m_pPreviousAbortHandler;
};

NS_END

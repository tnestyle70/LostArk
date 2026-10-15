//CRT 종료 콜백과 고정 버퍼의 크기 조회를 사용한다.
#include <csignal>
#include <cstdlib>
#include <exception>
#include <iterator>
#include "CrashReport_Manager.h"

NS_BEGIN(Engine)

//클래스가 공유할 저장 공간을 한 번 정의한다.
bool CCrashReport_Manager::m_bInstalled = false;
HANDLE CCrashReport_Manager::m_hLogFile = INVALID_HANDLE_VALUE;
char CCrashReport_Manager::m_szTextBuffer[65536]{};
volatile LONG CCrashReport_Manager::m_lRecordEntered = 0;
std::terminate_handler CCrashReport_Manager::m_pPreviousTerminateHandler = nullptr;
CCrashReport_Manager::AbortSignalHandler CCrashReport_Manager::m_pPreviousAbortHandler = SIG_DFL;

//출력 버퍼는 Manager가 소유한다. 작성기는 한 번의 기록 동안만 주소를 빌린다.
struct CCrashReport_Manager::CrashText final
{
    char* m_pBuffer = nullptr;
    DWORD m_dwCapacity = 0;
    DWORD m_dwLength = 0;

    //남은 공간 안에 한 글자를 쓴다. 버퍼가 차면 이후 내용은 생략한다.
    void Character(const char value) noexcept
    {
        if (m_dwLength < m_dwCapacity)
        {
            m_pBuffer[m_dwLength++] = value;
        }
    }

    void Text(const char* value, const size_t limit = 4096u) noexcept
    {
        if (!value)
        {
            value = "<null>";
        }

        for (size_t iIndex = 0; iIndex < limit && value[iIndex]; ++iIndex)
        {
            Character(value[iIndex]);
        }
    }

    //예외 문구의 개행과 제어 문자가 로그의 행 구분을 깨지 않게 한다.
    void Escaped(const char* value, const size_t limit = 4096u) noexcept
    {
        if (!value)
        {
            value = "<null>";
        }

        size_t iIndex = 0;

        for (; iIndex < limit && value[iIndex]; ++iIndex)
        {
            const unsigned char ch = static_cast<unsigned char>(value[iIndex]);

            if (ch == '\r')
            {
                Text("\\r");
            }
            else if (ch == '\n')
            {
                Text("\\n");
            }
            else if (ch == '\t')
            {
                Text("\\t");
            }
            else if (ch < 32u || ch == 127u)
            {
                Character('?');
            }
            else
            {
                Character(static_cast<char>(ch));
            }
        }

        if (iIndex == limit)
        {
            Text("<truncated>");
        }
    }

    //10진수 정보와 16진수 주소를 작은 지역 배열에서 변환한다.
    void Number(ULONGLONG value, const unsigned int width = 0u,
        const unsigned int radix = 10u) noexcept
    {
        char szReversed[32]{};

        unsigned int iCount = 0;

        do
        {
            szReversed[iCount++] = "0123456789ABCDEF"[value % radix];
            value /= radix;
        }
        while (value && iCount < sizeof(szReversed));

        for (unsigned int iPadding = iCount; iPadding < width; ++iPadding)
        {
            Character('0');
        }

        while (iCount)
        {
            Character(szReversed[--iCount]);
        }
    }

    void Address(const ULONG_PTR value) noexcept
    {
        Text("0x");
        Number(value, static_cast<unsigned int>(sizeof(void*) * 2u), 16u);
    }
};
//종료 요청을 Client가 exe가 종료되는 시점에 logPath로 매개 변수로 받았을 때를 기준으로,
//logPath에 해당하는 log를 넣어서 관리를 하는 게 맞는데, logPath에 들어가는 내용은
//CreateFileW에 들어가는 걸까?
bool CCrashReport_Manager::Install(const wchar_t* logPath) noexcept
{
    //재등록으로 기존 콜백과 파일을 덮어쓰지 않는다.
    if (!logPath || logPath[0] == L'\0' || m_bInstalled)
    {
        return false;
    }

    //종료 중 경로를 만들거나 파일을 다시 열지 않도록 미리 준비한다.
    m_hLogFile = CreateFileW(logPath, FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (m_hLogFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    //abort 등록에 실패하면 파일을 닫아 재시도 가능한 상태로 되돌린다.
    m_pPreviousAbortHandler = std::signal(SIGABRT, &CCrashReport_Manager::OnAbort);

    if (m_pPreviousAbortHandler == SIG_ERR)
    {
        CloseHandle(m_hLogFile);
        m_hLogFile = INVALID_HANDLE_VALUE;
        m_pPreviousAbortHandler = SIG_DFL;
        return false;
    }

    m_pPreviousTerminateHandler = std::set_terminate(&CCrashReport_Manager::OnTerminate);
    m_bInstalled = true;
    return true;
}

//두 종료 콜백이 종류·현재 C++ 예외·진단 시점의 스택을 파일에 남긴다.
void CCrashReport_Manager::RecordFatalDiagnostic(const char* kind) noexcept
{
    CrashText TextWriter{ m_szTextBuffer, static_cast<DWORD>(sizeof(m_szTextBuffer)), 0 };

    //종료한 프로세스와 스레드를 구분할 시각과 ID를 기록한다.
    SYSTEMTIME LocalTime{};
    GetLocalTime(&LocalTime);
    TextWriter.Text("\r\n");
    TextWriter.Text(kind);
    TextWriter.Text(" local=");
    TextWriter.Number(LocalTime.wYear, 4);
    TextWriter.Character('-');
    TextWriter.Number(LocalTime.wMonth, 2);
    TextWriter.Character('-');
    TextWriter.Number(LocalTime.wDay, 2);
    TextWriter.Character(' ');
    TextWriter.Number(LocalTime.wHour, 2);
    TextWriter.Character(':');
    TextWriter.Number(LocalTime.wMinute, 2);
    TextWriter.Character(':');
    TextWriter.Number(LocalTime.wSecond, 2);
    TextWriter.Character('.');
    TextWriter.Number(LocalTime.wMilliseconds, 3);
    TextWriter.Text(" pid=");
    TextWriter.Number(GetCurrentProcessId());
    TextWriter.Text(" tid=");
    TextWriter.Number(GetCurrentThreadId());
    TextWriter.Text(" uncaught=");
    TextWriter.Number(static_cast<unsigned int>(std::uncaught_exceptions()));
    TextWriter.Text("\r\nexception=");

    //현재 C++ 예외가 있으면 원인 문구를 얻는다.
    try
    {
        const std::exception_ptr pException = std::current_exception();
        if (pException)
        {
            std::rethrow_exception(pException);
        }

        TextWriter.Text("<no active C++ exception>");
    }
    catch (const std::exception& Exception)
    {
        TextWriter.Text("std::exception what=");
        TextWriter.Escaped(Exception.what());
    }
    catch (...)
    {
        TextWriter.Text("<non-std C++ exception>");
    }
    TextWriter.Text("\r\n");

    //진단 시점의 주소를 남긴다. 함수 이름 해석에는 같은 빌드의 EXE/DLL과 PDB가 필요하다.
    void* pFrames[64]{};
    const USHORT iFrameCount = CaptureStackBackTrace(0, 64, pFrames, nullptr);
    TextWriter.Text("current_stack_frames=");
    TextWriter.Number(iFrameCount);
    TextWriter.Text("\r\n");

    for (USHORT iIndex = 0; iIndex < iFrameCount; ++iIndex)
    {
        TextWriter.Character('#');
        TextWriter.Number(iIndex, 2);
        TextWriter.Text(" address=");

        const ULONG_PTR iAddress = reinterpret_cast<ULONG_PTR>(pFrames[iIndex]);
        TextWriter.Address(iAddress);

        //EXE/DLL에 속한 주소는 모듈 시작 주소와 상대 위치를 함께 기록한다.
        MEMORY_BASIC_INFORMATION MemoryInfo{};
        const SIZE_T iQuerySize = VirtualQuery(pFrames[iIndex], &MemoryInfo, sizeof(MemoryInfo));
        if (iQuerySize != 0 && MemoryInfo.Type == MEM_IMAGE && MemoryInfo.AllocationBase)
        {
            const ULONG_PTR iModuleBase = reinterpret_cast<ULONG_PTR>(MemoryInfo.AllocationBase);
            TextWriter.Text(" module_base=");
            TextWriter.Address(iModuleBase);
            TextWriter.Text(" rva=");
            TextWriter.Address(iAddress - iModuleBase);

            //모듈의 Windows 경로를 로그의 UTF-8 문자열로 변환한다.
            wchar_t szModulePath[2048]{};
            const HMODULE hModule = static_cast<HMODULE>(MemoryInfo.AllocationBase);
            const DWORD dwPathLength = GetModuleFileNameW(hModule, szModulePath,
                static_cast<DWORD>(std::size(szModulePath)));

            char szUtf8Path[8192]{};
            int iUtf8Length = 0;
            if (dwPathLength > 0 && dwPathLength < std::size(szModulePath))
            {
                iUtf8Length = WideCharToMultiByte(CP_UTF8, 0, szModulePath,
                    static_cast<int>(dwPathLength), szUtf8Path,
                    static_cast<int>(sizeof(szUtf8Path) - 1u), nullptr, nullptr);
            }

            TextWriter.Text(" module=");
            if (iUtf8Length > 0)
            {
                TextWriter.Escaped(szUtf8Path, 512u);
            }
            else
            {
                TextWriter.Escaped("<unavailable or truncated>", 512u);
            }
        }
        else
        {
            TextWriter.Text(" module=<not an image>");
        }

        TextWriter.Text("\r\n");
    }
    TextWriter.Text("END_FATAL_DIAGNOSTIC\r\n");

    if (m_hLogFile == INVALID_HANDLE_VALUE)
    {
        return;
    }

    //부분 쓰기는 이어 쓴다. 실패하거나 0바이트를 쓰면 멈춘다.
    DWORD dwOffset = 0;
    while (dwOffset < TextWriter.m_dwLength)
    {
        DWORD dwWritten = 0;
        const BOOL bWritten = WriteFile(m_hLogFile, m_szTextBuffer + dwOffset,
            TextWriter.m_dwLength - dwOffset, &dwWritten, nullptr);
        if (!bWritten || dwWritten == 0)
        {
            break;
        }

        dwOffset += dwWritten;
    }
    FlushFileBuffers(m_hLogFile);
}

//C++ 치명적 종료를 한 번 기록한 뒤 기존 종료 처리를 이어간다.
[[noreturn]] void CCrashReport_Manager::OnTerminate() noexcept
{
    if (InterlockedCompareExchange(&m_lRecordEntered, 1, 0) == 0)
    {
        RecordFatalDiagnostic("CXX_TERMINATE");

        if (m_pPreviousTerminateHandler &&
            m_pPreviousTerminateHandler != &CCrashReport_Manager::OnTerminate)
        {
            m_pPreviousTerminateHandler();
        }
    }

    std::abort();
}

//직접 abort한 경로도 받는다. terminate에서 이미 기록했다면 중복 작성하지 않는다.
void CCrashReport_Manager::OnAbort(const int signal) noexcept
{
    if (InterlockedCompareExchange(&m_lRecordEntered, 1, 0) == 0)
    {
        RecordFatalDiagnostic("CRT_SIGABRT");
    }

    if (m_pPreviousAbortHandler != SIG_DFL && m_pPreviousAbortHandler != SIG_IGN &&
        m_pPreviousAbortHandler != SIG_ERR &&
        m_pPreviousAbortHandler != &CCrashReport_Manager::OnAbort)
    {
        m_pPreviousAbortHandler(signal);
    }
}

//Engine DLL이 로드된 동안 유지한다. 별도 DLL unload는 이 계약에 포함하지 않는다.
NS_END

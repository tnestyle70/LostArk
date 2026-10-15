#include "ClientDiagnostics.h"

// 파일 열기와 치명적 종료 콜백은 Engine의 기록기에 맡긴다.
#include "CrashReport_Manager.h"

void Client::PrepareCrashReport() noexcept
{
    // Client/Bin/Debug 또는 Release의 EXE 위치에서 Client/Default 로그 경로를 만든다.
    // 설치 중에만 사용하는 지역 버퍼이며 Engine은 이 포인터를 보관하지 않는다.
    wchar_t szLogPath[32768]{};
    DWORD iLength = GetModuleFileNameW(nullptr, szLogPath, _countof(szLogPath));
    bool bAbsolutePath = iLength > 0 && iLength < _countof(szLogPath);

    // 실행 파일 이름과 출력 폴더 두 단계를 제거해 Client 폴더를 찾는다.
    for (unsigned int iParent = 0; bAbsolutePath && iParent < 3; ++iParent)
    {
        while (iLength > 0 && szLogPath[iLength - 1] != L'\\' && szLogPath[iLength - 1] != L'/')
        {
            --iLength;
        }

        if (iLength == 0)
        {
            bAbsolutePath = false;
        }
        else
        {
            szLogPath[--iLength] = L'\0';
        }
    }

    // 배치 경로를 얻지 못했을 때만 기존처럼 현재 작업 폴더의 파일명을 사용한다.
    constexpr wchar_t szSuffix[] = L"\\Default\\ClientCrash.user.log";
    if (bAbsolutePath && iLength + _countof(szSuffix) <= _countof(szLogPath))
    {
        for (size_t iIndex = 0; iIndex < _countof(szSuffix); ++iIndex)
        {
            szLogPath[iLength + iIndex] = szSuffix[iIndex];
        }
    }
    else
    {
        constexpr wchar_t szFallback[] = L"ClientCrash.user.log";
        for (size_t iIndex = 0; iIndex < _countof(szFallback); ++iIndex)
        {
            szLogPath[iIndex] = szFallback[iIndex];
        }
    }

    // 설치 실패는 디버거에 알리고 원래의 게임 초기화 흐름을 이어간다.
    if (!Engine::CCrashReport_Manager::Install(szLogPath))
    {
        OutputDebugStringW(L"[CrashReport] Unable to install ClientCrash.user.log diagnostic\n");
    }
}

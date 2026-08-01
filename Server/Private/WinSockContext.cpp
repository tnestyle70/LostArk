#include "WinSockContext.h"

#include <WinSock2.h>

LostArk::Server::CWinSockContext::~CWinSockContext()
{
    //Resource Acquisition is initialization RAII 패턴 
    //소멸시 shut down으로 WSACleanup 호출해서 정리
    Shutdown();
}

//Windows의 Winsock 기능을 현재 프로세스에서 사용할 수 있도록 초기화하는 함수이다.
//CWinSockConetxt 객체 생성 -> Initialize -> WSAStartUp->
//이후 socket, bind, listen, accept, send, recv 사용이 가능!

bool LostArk::Server::CWinSockContext::Initialize()
{
    //이미 초기화된 상태라면 return true
    if (m_isInitialized)
        return true;
    //WSADATA가 가지고 있는 정보가 어떻게 되는 걸까?
    WSADATA winSockData{};
    //WSA Startup 이 함수의 정확한 동작과 역할이 뭘까?
    //WSADATA 내부에 정보를 채워준다?MAKEWORD의 2, 2는 뭐지?
    //두 개의 8비트 숫자를 16비트 값으로 묶는다.
    //현재 프로세스에서 winsock 사용을 시작한다
    //요청한 버전 확인 -> winsockdata에 선택된 구현 정보를 기록한다.
    //성공하면 0, 실패하면 오류 코드를 반환한다.
    const int result = ::WSAStartup(
        MAKEWORD(2, 2),
        &winSockData);
    //result 값이 0이 아니라면 WSA Startup 초기화 실패?
    if (0 != result)
        return false;
    //version supported가 의미하는 것이 정확하게 뭐지?
    //버전 검사, LOBYTE : 낮은 8비트, 주 버전 2
    //HIBYTE : 높은 8비트, 부 버전 2
    //실제로 협상된 버전이 2.2인지 확인한다. 지원하지 않는 버전이면 성공한 WSAStartup을 취소하기 위해
    //즉시 WSACleanup을 호출한다.
    const bool isVersionSupported =
        2 == LOBYTE(winSockData.wVersion) &&
        2 == HIBYTE(winSockData.wVersion);

    if (!isVersionSupported)
    {
        ::WSACleanup();
        return false;
    }

    m_isInitialized = true;

    return true;
}

void LostArk::Server::CWinSockContext::Shutdown()
{
    //winsock 닫기
    if (!m_isInitialized)
        return;

    ::WSACleanup();

    m_isInitialized = false;
}

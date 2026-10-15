// 프로세스 Winsock 사용권의 초기화·정리 책임을 구현한다.
#include "WinSockContext.h"

// WSAStartup/WSACleanup과 실제 협상 버전을 받는 WSADATA를 사용한다.
#include <WinSock2.h>

LostArk::Server::CWinSockContext::~CWinSockContext()
{
    //Resource Acquisition is initialization RAII 패턴 
    //소멸시 shut down으로 WSACleanup 호출해서 정리
    // Run이 중간 반환한 경로에서도 성공한 초기화가 남아 있으면 이 소멸자가 회수한다.
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
    // wVersion은 실제 사용할 버전, wHighVersion은 구현이 지원하는 최고 버전이다. 여기서는 wVersion만 승인에 사용한다.
    WSADATA winSockData{};
    //WSA Startup 이 함수의 정확한 동작과 역할이 뭘까?
    //WSADATA 내부에 정보를 채워준다?MAKEWORD의 2, 2는 뭐지?
    // MAKEWORD(2, 2)는 낮은 바이트와 높은 바이트에 각각 2를 넣어 Winsock 2.2를 요청한다.
    //두 개의 8비트 숫자를 16비트 값으로 묶는다.
    //현재 프로세스에서 winsock 사용을 시작한다
    //요청한 버전 확인 -> winsockdata에 선택된 구현 정보를 기록한다.
    //성공하면 0, 실패하면 오류 코드를 반환한다.
    const int result = ::WSAStartup(
        MAKEWORD(2, 2),
        &winSockData);
    //result 값이 0이 아니라면 WSA Startup 초기화 실패?
    // WSAStartup은 실패 코드를 직접 반환한다. 실패 시에는 이 객체의 초기화 소유권을 만들지 않는다.
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

    // 성공한 초기화와 버전 검사를 모두 통과한 뒤에만 이후 Shutdown의 정리 의무를 기록한다.
    m_isInitialized = true;

    return true;
}

// ServerApp이 연결과 listener를 모두 닫은 뒤 호출한다. 반복 호출도 자신이 소유한 초기화만 한 번 해제한다.
void LostArk::Server::CWinSockContext::Shutdown()
{
    //winsock 닫기
    if (!m_isInitialized)
        return;

    ::WSACleanup();

    m_isInitialized = false;
}

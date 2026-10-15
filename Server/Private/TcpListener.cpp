// listening socket의 수명을 구현한다. 접속한 Client의 송수신은 ClientSession이 담당한다.
#include "TcpListener.h"

// string_view 주소를 InetPtonA가 요구하는 널 종료 문자열로 보관한다.
#include <string>

LostArk::Server::CTcpListener::~CTcpListener()
{
    // 객체 소멸 시 소유한 listener를 Close로 회수하는 RAII 정리 경로다.
    Close();
}

// ServerApp::Run이 Winsock 초기화 뒤 호출한다. 모든 준비가 성공할 때만 멤버 handle을 공개한다.
bool LostArk::Server::CTcpListener::Open(
	const std::string_view bindAddress,
	const std::uint16_t port)
{
    if (Is_Open())
        return true;
    //atomic으로 바꿨으므로 store atomic 전부 설명!
    // 이전 오류를 먼저 지운다. store는 다른 스레드의 오류 조회와 값 접근을 동기화한다.
    m_iLastErrorCode.store(0);

    //listen socket 생성
    const SOCKET listenSocket = ::socket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP);

    if (INVALID_SOCKET == listenSocket)
    {
        m_iLastErrorCode.store(::WSAGetLastError());
        return false;
    }

    // IPv4 주소와 포트를 운영체제의 sockaddr 입력 형식으로 구성한다.
    sockaddr_in address{};

    address.sin_family = AF_INET;

    const std::string bindAddressText{ bindAddress };
    if ("0.0.0.0" == bindAddressText)
    {
        address.sin_addr.s_addr = ::htonl(INADDR_ANY);
    }
    else if ("localhost" == bindAddressText)
    {
        address.sin_addr.s_addr = ::htonl(INADDR_LOOPBACK);
    }
    else if (1 != ::InetPtonA(
        AF_INET,
        bindAddressText.c_str(),
        &address.sin_addr))
    {
        m_iLastErrorCode.store(WSAEINVAL);
        ::closesocket(listenSocket);
        return false;
    }

    //host to 누구?
    // host-to-network-short 변환이다. CPU의 바이트 순서와 관계없이 포트를 네트워크 바이트 순서로 넣는다.
    address.sin_port = ::htons(port);

    //listen socket binding 실패 여부 판단
    if (SOCKET_ERROR == ::bind(
        listenSocket,
        reinterpret_cast<const sockaddr*>(
            &address),
        sizeof(address)))
    {
        m_iLastErrorCode.store(::WSAGetLastError());
        ::closesocket(listenSocket);
        return false;
    }

    //SOMAXCONN의 의미는 뭘까?
    // accept 전의 연결 대기열 크기를 시스템 권장 최대값으로 요청한다. 게임 방의 최대 인원과는 별개다.
    if (SOCKET_ERROR == ::listen(
        listenSocket,
        SOMAXCONN))
    {
        m_iLastErrorCode.store(::WSAGetLastError());
        ::closesocket(listenSocket);
        return false;
    }

    // 이제부터 Close가 정리를 책임진다. 위 실패 경로는 아직 멤버에 공개하지 않은 임시 socket을 직접 닫는다.
    m_hListenSocket.store(listenSocket);
    return true;
}

// AcceptThread에서 연결을 기다린다. 새 socket은 성공 반환 때 호출자에게 넘기고 listener는 계속 보유한다.
SOCKET LostArk::Server::CTcpListener::Accept()
{
    
    const SOCKET listenSocket = m_hListenSocket.load();

    if (INVALID_SOCKET == listenSocket)
    {
        m_iLastErrorCode.store(WSAENOTSOCK);
        return INVALID_SOCKET;
    }

    //Open에서 열었던 listensocket을 accept해서, clientsocket을 return
    // accept는 listen socket을 연결 socket으로 바꾸지 않고 별도 client socket을 새로 반환한다.
    const SOCKET clientSocket = ::accept(
        listenSocket,
        nullptr,
        nullptr);

    if (INVALID_SOCKET == clientSocket)
    {
        m_iLastErrorCode.store(::WSAGetLastError());
        return INVALID_SOCKET;
    }

    m_iLastErrorCode.store(0);

    return clientSocket;
}

// ServerApp 종료와 소멸자가 공유하는 정리 경로다. exchange가 handle 정리 권한을 한 호출에만 넘긴다.
void LostArk::Server::CTcpListener::Close()
{
    const SOCKET listenSocket =
        m_hListenSocket.exchange(INVALID_SOCKET);

    if (INVALID_SOCKET != listenSocket)
        ::closesocket(listenSocket);
}

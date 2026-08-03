#include "TcpListener.h"

#include <string>

LostArk::Server::CTcpListener::~CTcpListener()
{
    //Resource Acquition is initialization 
    Close();
}

bool LostArk::Server::CTcpListener::Open(
	const std::string_view bindAddress,
	const std::uint16_t port)
{
    if (Is_Open())
        return true;
    //atomic으로 바꿨으므로 store atomic 전부 설명!
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
    if (SOCKET_ERROR == ::listen(
        listenSocket,
        SOMAXCONN))
    {
        m_iLastErrorCode.store(::WSAGetLastError());
        ::closesocket(listenSocket);
        return false;
    }

    m_hListenSocket.store(listenSocket);
    return true;
}

SOCKET LostArk::Server::CTcpListener::Accept()
{
    
    const SOCKET listenSocket = m_hListenSocket.load();

    if (INVALID_SOCKET == listenSocket)
    {
        m_iLastErrorCode.store(WSAENOTSOCK);
        return INVALID_SOCKET;
    }

    //Open에서 열었던 listensocket을 accept해서, clientsocket을 return
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

void LostArk::Server::CTcpListener::Close()
{
    const SOCKET listenSocket =
        m_hListenSocket.exchange(INVALID_SOCKET);

    if (INVALID_SOCKET != listenSocket)
        ::closesocket(listenSocket);
}

# S2C_ENTER_ACCEPTED 정답지

이 문서는 C2S 성공 체크포인트로 원복하면서 제거한 `S2C_ENTER_ACCEPTED` 런타임 구현만 보존한다.
Shared의 메시지 계약과 Harness는 원복하지 않는다.

## Server/Public/ClientSession.h

```cpp
#include <cstdint>
#include <span>

bool Send_Frame(
    LostArk::Shared::PACKET_TYPE packetType,
    std::span<const std::uint8_t> payload);

private:
    bool Send_All(std::span<const std::uint8_t> bytes);
```

## Server/Private/ClientSession.cpp

```cpp
#include <vector>

bool LostArk::Server::CClientSession::Send_Frame(
    LostArk::Shared::PACKET_TYPE packetType,
    std::span<const std::uint8_t> payload)
{
    std::vector<std::uint8_t> frameBytes;

    if (!LostArk::Shared::Build_Packet_Frame(
        packetType,
        payload,
        frameBytes))
    {
        return false;
    }

    return Send_All(frameBytes);
}

bool LostArk::Server::CClientSession::Send_All(
    std::span<const std::uint8_t> bytes)
{
    if (!Is_Open())
        return false;

    std::size_t sentByteCount = 0;

    while (sentByteCount < bytes.size())
    {
        const int result = ::send(
            m_hClientSocket,
            reinterpret_cast<const char*>(
                bytes.data() + sentByteCount),
            static_cast<int>(bytes.size() - sentByteCount),
            0);

        if (SOCKET_ERROR == result)
        {
            m_iLastErrorCode = ::WSAGetLastError();
            return false;
        }

        if (0 == result)
            return false;

        sentByteCount += static_cast<std::size_t>(result);
    }

    return true;
}
```

## Server/Private/ServerApp.cpp

`C2S_ENTER_WORLD`를 정상적으로 읽고 출력한 직후에 들어갈 코드다.

```cpp
S2C_ENTER_ACCEPTED accepted{};
accepted.iPlayerId = 1;
accepted.iNetEntityId = 100;

CPacketWriter acceptedWriter;
if (!Write_Message(acceptedWriter, accepted))
{
    std::cerr << "Failed to encode S2C_ENTER_ACCEPTED.\n";
    return 1;
}

if (!clientSession.Send_Frame(
    PACKET_TYPE::S2C_ENTER_ACCEPTED,
    acceptedWriter.Get_Buffer()))
{
    std::cerr
        << "Failed to send S2C_ENTER_ACCEPTED. Error: "
        << clientSession.Get_LastErrorCode()
        << '\n';
    return 1;
}

std::cout
    << "S2C_ENTER_ACCEPTED sent.\n"
    << "Player ID: " << accepted.iPlayerId << '\n'
    << "Net Entity ID: " << accepted.iNetEntityId << '\n';
```

추가 include:

```cpp
#include "Network/PacketWriter.h"
```

## Client/Public/NetworkManager.h

추가 include:

```cpp
#include "Network/PacketFrame.h"
#include "Network/PacketStreamParser.h"

#include <atomic>
#include <deque>
#include <mutex>
#include <thread>
```

추가 public 함수:

```cpp
void Update();

bool Try_Consume_EnterAccepted(
    LostArk::Shared::S2C_ENTER_ACCEPTED& message);

[[nodiscard]] LostArk::Shared::PLAYER_ID
Get_LocalPlayerId() const;

[[nodiscard]] LostArk::Shared::NET_ENTITY_ID
Get_LocalNetEntityId() const;
```

추가 private 함수:

```cpp
void Receive_Loop();
void Handle_Frame(const LostArk::Shared::PACKET_FRAME& frame);
```

추가 및 변경 멤버:

```cpp
std::atomic<int> m_iLastErrorCode{ 0 };

std::thread m_ReceiveThread;
std::atomic_bool m_isReceiveRunning{ false };
LostArk::Shared::CPacketStreamParser m_StreamParser;

std::mutex m_InboundMutex;
std::deque<LostArk::Shared::PACKET_FRAME> m_InboundFrames;

bool m_hasPendingEnterAccepted = false;
LostArk::Shared::S2C_ENTER_ACCEPTED m_PendingEnterAccepted{};

LostArk::Shared::PLAYER_ID m_iLocalPlayerId =
    LostArk::Shared::INVALID_PLAYER_ID;

LostArk::Shared::NET_ENTITY_ID m_iLocalNetEntityId =
    LostArk::Shared::INVALID_NET_ENTITY_ID;
```

## Client/Private/NetworkManager.cpp

```cpp
#include <array>
#include <utility>

void CNetworkManager::Update()
{
    std::deque<LostArk::Shared::PACKET_FRAME> receivedFrames;

    {
        std::scoped_lock lock{ m_InboundMutex };
        receivedFrames.swap(m_InboundFrames);
    }

    for (const LostArk::Shared::PACKET_FRAME& frame : receivedFrames)
        Handle_Frame(frame);
}

bool CNetworkManager::Try_Consume_EnterAccepted(
    LostArk::Shared::S2C_ENTER_ACCEPTED& message)
{
    if (!m_hasPendingEnterAccepted)
        return false;

    message = m_PendingEnterAccepted;
    m_hasPendingEnterAccepted = false;
    return true;
}

LostArk::Shared::PLAYER_ID CNetworkManager::Get_LocalPlayerId() const
{
    return m_iLocalPlayerId;
}

LostArk::Shared::NET_ENTITY_ID
CNetworkManager::Get_LocalNetEntityId() const
{
    return m_iLocalNetEntityId;
}

void CNetworkManager::Receive_Loop()
{
    using namespace LostArk::Shared;

    std::array<std::uint8_t, 4096> receiveBuffer{};

    while (m_isReceiveRunning.load())
    {
        const int receivedByteCount = ::recv(
            m_hServerSocket,
            reinterpret_cast<char*>(receiveBuffer.data()),
            static_cast<int>(receiveBuffer.size()),
            0);

        if (0 == receivedByteCount)
            break;

        if (SOCKET_ERROR == receivedByteCount)
        {
            const int errorCode = ::WSAGetLastError();
            if (m_isReceiveRunning.load())
                m_iLastErrorCode.store(errorCode);
            break;
        }

        const std::span<const std::uint8_t> receivedBytes{
            receiveBuffer.data(),
            static_cast<std::size_t>(receivedByteCount)
        };

        if (!m_StreamParser.Append(receivedBytes))
        {
            m_iLastErrorCode.store(WSAEMSGSIZE);
            break;
        }

        for (;;)
        {
            PACKET_FRAME frame{};
            const PACKET_PARSE_RESULT parseResult =
                m_StreamParser.Try_Pop(frame);

            if (PACKET_PARSE_RESULT::NEED_MORE_DATA == parseResult)
                break;

            if (PACKET_PARSE_RESULT::INVALID_FRAME == parseResult)
            {
                m_iLastErrorCode.store(WSAEPROTONOSUPPORT);
                m_isReceiveRunning.store(false);
                return;
            }

            std::scoped_lock lock{ m_InboundMutex };
            m_InboundFrames.push_back(std::move(frame));
        }
    }

    m_isReceiveRunning.store(false);
}

void CNetworkManager::Handle_Frame(
    const LostArk::Shared::PACKET_FRAME& frame)
{
    using namespace LostArk::Shared;

    if (PACKET_TYPE::S2C_ENTER_ACCEPTED != frame.ePacketType)
        return;

    CPacketReader reader{ frame.Payload };
    S2C_ENTER_ACCEPTED accepted{};

    if (!Read_Message(reader, accepted) ||
        0 != reader.Get_RemainingSize())
    {
        m_iLastErrorCode.store(WSAEINVAL);
        return;
    }

    m_iLocalPlayerId = accepted.iPlayerId;
    m_iLocalNetEntityId = accepted.iNetEntityId;
    m_PendingEnterAccepted = accepted;
    m_hasPendingEnterAccepted = true;
}
```

`Connect_To_Server()`의 `connect()` 성공 직후:

```cpp
m_StreamParser.Reset();
m_hasPendingEnterAccepted = false;
m_PendingEnterAccepted = {};
m_iLocalPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
m_iLocalNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
m_iLastErrorCode.store(0);

m_isReceiveRunning.store(true);
m_ReceiveThread = std::thread(
    &CNetworkManager::Receive_Loop,
    this);
```

`Close_ServerConnection()` 정답:

```cpp
void CNetworkManager::Close_ServerConnection()
{
    m_isReceiveRunning.store(false);

    if (INVALID_SOCKET != m_hServerSocket)
        ::shutdown(m_hServerSocket, SD_BOTH);

    if (m_ReceiveThread.joinable())
        m_ReceiveThread.join();

    if (INVALID_SOCKET != m_hServerSocket)
    {
        ::closesocket(m_hServerSocket);
        m_hServerSocket = INVALID_SOCKET;
    }

    {
        std::scoped_lock lock{ m_InboundMutex };
        m_InboundFrames.clear();
    }

    m_StreamParser.Reset();
    m_hasPendingEnterAccepted = false;
    m_PendingEnterAccepted = {};
    m_iLocalPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
    m_iLocalNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
}
```

## Client/Private/MainApp.cpp

`CGameInstance::Get().Update_Engine()` 호출 직전:

```cpp
CNetworkManager::Get().Update();
```

## Client/Private/Level_Lobby.cpp

`CLevel_Lobby::Update()`의 첫 부분:

```cpp
LostArk::Shared::S2C_ENTER_ACCEPTED accepted{};
if (CNetworkManager::Get().Try_Consume_EnterAccepted(accepted))
{
    m_strNetworkStatus =
        "S2C_ENTER_ACCEPTED received. PlayerId=" +
        to_string(accepted.iPlayerId) +
        ", NetEntityId=" +
        to_string(accepted.iNetEntityId);

    m_isEnterRequested = true;
}
```

## 기대 바이트

```text
0e 00 00 00    total size = 14
02 00          S2C_ENTER_ACCEPTED
01 00 00 00    PlayerId = 1
64 00 00 00    NetEntityId = 100
```

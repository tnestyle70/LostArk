```cpp
// FILE: Server/Public/IocpOperation.h

#pragma once

#include <WinSock2.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace LostArk::Server
{
	class CIocpService;
	enum class IOCP_OPERATION_TYPE
	{
		ACCEPT,
		RECEIVE,
		SEND
	};

	class IIocpCompletionSink;

	struct IOCP_OPERATION
	{
		OVERLAPPED Overlapped{};
		IOCP_OPERATION_TYPE eType = IOCP_OPERATION_TYPE::RECEIVE;
		IIocpCompletionSink* pSink = nullptr;
		CIocpService* pService = nullptr;
		std::shared_ptr<IIocpCompletionSink> pKeepAlive;
		SOCKET hAcceptedSocket = INVALID_SOCKET;
		std::array<std::uint8_t, 4096> ReceiveBuffer{};
		std::vector<std::uint8_t> SendBuffer;
		WSABUF Buffer{};
		std::size_t iOffset = 0;

		void Bind_ReceiveBuffer();
		void Bind_SendBuffer();
	};

	class IIocpCompletionSink
	{
	public:
		virtual ~IIocpCompletionSink() = default;
		virtual void On_IocpCompleted(
			std::unique_ptr<IOCP_OPERATION> operation,
			std::uint32_t transferredBytes,
			std::uint32_t errorCode) = 0;
	};
}
```

```cpp
// FILE: Server/Private/IocpOperation.cpp

#include "IocpOperation.h"

void LostArk::Server::IOCP_OPERATION::Bind_ReceiveBuffer()
{
	Buffer.buf = reinterpret_cast<char*>(ReceiveBuffer.data());
	Buffer.len = static_cast<ULONG>(ReceiveBuffer.size());
}

void LostArk::Server::IOCP_OPERATION::Bind_SendBuffer()
{
	Buffer.buf = reinterpret_cast<char*>(SendBuffer.data() + iOffset);
	Buffer.len = static_cast<ULONG>(SendBuffer.size() - iOffset);
}
```

```cpp
// FILE: Server/Public/IocpService.h

#pragma once

#include "IocpOperation.h"

#include <WinSock2.h>
#include <Windows.h>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

namespace LostArk::Server
{
	class CIocpService final
	{
	public:
		~CIocpService();
		bool Start(std::size_t workerCount);
		void Stop();
		bool Associate(SOCKET socketHandle);
		HANDLE Get_Handle() const;
		void Track_PostedOperation();
		void Track_FailedPost();
		bool Wait_ForIdle(std::chrono::milliseconds timeout);

	private:
		void Worker_Loop();

	private:
		HANDLE m_hCompletionPort = nullptr;
		std::atomic_bool m_isRunning{ false };
		std::vector<std::thread> m_Workers;
		std::atomic_size_t m_iOutstandingOperations{ 0 };
		std::mutex m_IdleMutex;
		std::condition_variable m_IdleCondition;
	};
}
```

```cpp
// FILE: Server/Private/IocpService.cpp

#include "IocpService.h"

#include <algorithm>
#include <memory>

LostArk::Server::CIocpService::~CIocpService()
{
	Stop();
}

bool LostArk::Server::CIocpService::Start(std::size_t workerCount)
{
	if (nullptr != m_hCompletionPort) return true;
	m_hCompletionPort = CreateIoCompletionPort(
		INVALID_HANDLE_VALUE, nullptr, 0, 0);
	if (nullptr == m_hCompletionPort) return false;

	workerCount = (std::max<std::size_t>)(1, workerCount);
	m_isRunning.store(true);
	try
	{
		m_Workers.reserve(workerCount);
		for (std::size_t index = 0; index < workerCount; ++index)
			m_Workers.emplace_back(&CIocpService::Worker_Loop, this);
	}
	catch (...)
	{
		Stop();
		return false;
	}
	return true;
}

void LostArk::Server::CIocpService::Stop()
{
	if (nullptr == m_hCompletionPort) return;
	m_isRunning.store(false);
	for (std::size_t index = 0; index < m_Workers.size(); ++index)
		PostQueuedCompletionStatus(m_hCompletionPort, 0, 0, nullptr);
	for (std::thread& worker : m_Workers)
		if (worker.joinable()) worker.join();
	m_Workers.clear();
	CloseHandle(m_hCompletionPort);
	m_hCompletionPort = nullptr;
}

bool LostArk::Server::CIocpService::Associate(SOCKET socketHandle)
{
	return nullptr != m_hCompletionPort && INVALID_SOCKET != socketHandle &&
		nullptr != CreateIoCompletionPort(
			reinterpret_cast<HANDLE>(socketHandle),
			m_hCompletionPort, 0, 0);
}

HANDLE LostArk::Server::CIocpService::Get_Handle() const
{
	return m_hCompletionPort;
}

void LostArk::Server::CIocpService::Track_PostedOperation()
{
	m_iOutstandingOperations.fetch_add(1);
}

void LostArk::Server::CIocpService::Track_FailedPost()
{
	if (1 == m_iOutstandingOperations.fetch_sub(1))
		m_IdleCondition.notify_all();
}

bool LostArk::Server::CIocpService::Wait_ForIdle(
	std::chrono::milliseconds timeout)
{
	std::unique_lock lock{ m_IdleMutex };
	return m_IdleCondition.wait_for(lock, timeout, [this]
	{
		return 0 == m_iOutstandingOperations.load();
	});
}

void LostArk::Server::CIocpService::Worker_Loop()
{
	while (m_isRunning.load())
	{
		DWORD transferred = 0;
		ULONG_PTR completionKey = 0;
		OVERLAPPED* overlapped = nullptr;
		const BOOL succeeded = GetQueuedCompletionStatus(
			m_hCompletionPort,
			&transferred,
			&completionKey,
			&overlapped,
			INFINITE);
		(void)completionKey;
		if (nullptr == overlapped)
		{
			if (!m_isRunning.load()) break;
			continue;
		}

		std::unique_ptr<IOCP_OPERATION> operation(
			CONTAINING_RECORD(overlapped, IOCP_OPERATION, Overlapped));
		IIocpCompletionSink* sink = operation->pSink;
		CIocpService* operationService = operation->pService;
		const std::uint32_t errorCode = succeeded ? 0u : GetLastError();
		if (nullptr != sink)
			sink->On_IocpCompleted(std::move(operation), transferred, errorCode);
		if (nullptr != operationService)
			operationService->Track_FailedPost();
	}
}
```

```cpp
// FILE: Server/Public/IocpListener.h

#pragma once

#include "IocpOperation.h"
#include "IocpService.h"

#include <WinSock2.h>
#include <MSWSock.h>

#include <cstdint>
#include <functional>

namespace LostArk::Server
{
	class CIocpListener final : public IIocpCompletionSink
	{
	public:
		using ACCEPT_HANDLER = std::function<void(SOCKET)>;

		~CIocpListener();
		bool Open(CIocpService& service, std::uint16_t port, ACCEPT_HANDLER handler);
		void Close();
		void On_IocpCompleted(std::unique_ptr<IOCP_OPERATION> operation,
			std::uint32_t transferredBytes, std::uint32_t errorCode) override;

	private:
		bool Load_AcceptEx();
		bool Post_Accept();

	private:
		CIocpService* m_pService = nullptr;
		SOCKET m_hListenSocket = INVALID_SOCKET;
		LPFN_ACCEPTEX m_pAcceptEx = nullptr;
		ACCEPT_HANDLER m_OnAccepted;
	};
}
```

```cpp
// FILE: Server/Private/IocpListener.cpp

#include "IocpListener.h"

#include <array>
#include <utility>

LostArk::Server::CIocpListener::~CIocpListener()
{
	Close();
}

bool LostArk::Server::CIocpListener::Open(
	CIocpService& service, std::uint16_t port, ACCEPT_HANDLER handler)
{
	if (INVALID_SOCKET != m_hListenSocket) return false;
	m_hListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
		nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_hListenSocket) return false;

	sockaddr_in address{};
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);
	if (SOCKET_ERROR == bind(m_hListenSocket,
		reinterpret_cast<const sockaddr*>(&address), sizeof(address)) ||
		SOCKET_ERROR == listen(m_hListenSocket, SOMAXCONN))
	{
		Close(); return false;
	}

	m_pService = &service;
	m_OnAccepted = std::move(handler);
	if (!service.Associate(m_hListenSocket) || !Load_AcceptEx() || !Post_Accept())
	{
		Close(); return false;
	}
	return true;
}

void LostArk::Server::CIocpListener::Close()
{
	if (INVALID_SOCKET != m_hListenSocket)
	{
		closesocket(m_hListenSocket);
		m_hListenSocket = INVALID_SOCKET;
	}
	m_pService = nullptr;
	m_pAcceptEx = nullptr;
	m_OnAccepted = {};
}

bool LostArk::Server::CIocpListener::Load_AcceptEx()
{
	GUID guid = WSAID_ACCEPTEX;
	DWORD bytes = 0;
	return 0 == WSAIoctl(m_hListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guid, sizeof(guid), &m_pAcceptEx, sizeof(m_pAcceptEx),
		&bytes, nullptr, nullptr) && nullptr != m_pAcceptEx;
}

bool LostArk::Server::CIocpListener::Post_Accept()
{
	if (nullptr == m_pAcceptEx) return false;
	auto operation = std::make_unique<IOCP_OPERATION>();
	operation->eType = IOCP_OPERATION_TYPE::ACCEPT;
	operation->pSink = this;
	operation->pService = m_pService;
	operation->hAcceptedSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
		nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == operation->hAcceptedSocket) return false;

	DWORD received = 0;
	constexpr DWORD ADDRESS_BYTES = sizeof(sockaddr_in) + 16;
	m_pService->Track_PostedOperation();
	const BOOL result = m_pAcceptEx(m_hListenSocket,
		operation->hAcceptedSocket,
		operation->ReceiveBuffer.data(),
		0, ADDRESS_BYTES, ADDRESS_BYTES,
		&received, &operation->Overlapped);
	if (!result && WSAGetLastError() != ERROR_IO_PENDING)
	{
		m_pService->Track_FailedPost();
		closesocket(operation->hAcceptedSocket);
		return false;
	}
	operation.release();
	return true;
}

void LostArk::Server::CIocpListener::On_IocpCompleted(
	std::unique_ptr<IOCP_OPERATION> operation,
	std::uint32_t transferredBytes,
	std::uint32_t errorCode)
{
	(void)transferredBytes;
	if (nullptr == operation || operation->eType != IOCP_OPERATION_TYPE::ACCEPT) return;
	SOCKET accepted = operation->hAcceptedSocket;
	operation->hAcceptedSocket = INVALID_SOCKET;
	if (0 == errorCode && INVALID_SOCKET != accepted)
	{
		setsockopt(accepted, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
			reinterpret_cast<const char*>(&m_hListenSocket), sizeof(m_hListenSocket));
		if (m_OnAccepted) m_OnAccepted(accepted);
		else closesocket(accepted);
	}
	else if (INVALID_SOCKET != accepted)
	{
		closesocket(accepted);
	}
	if (INVALID_SOCKET != m_hListenSocket) Post_Accept();
}
```

```cpp
// FILE: Server/Public/ClientSession.h
// REPLACE THREAD-BASED CLASS WITH IOCP IMPLEMENTATION; KEEP PUBLIC Room CONTRACTS

#pragma once

#include "IocpOperation.h"
#include "IocpService.h"
#include "ServerIds.h"

#include "Network/NetworkIds.h"
#include "Network/PacketFrame.h"
#include "Network/PacketStreamParser.h"

#include <WinSock2.h>

#include <atomic>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <span>
#include <vector>

namespace LostArk::Server
{
	class CClientSession final : public IIocpCompletionSink,
		public std::enable_shared_from_this<CClientSession>
	{
	public:
		using FRAME_HANDLER = std::function<void(SESSION_ID,
			const LostArk::Shared::PACKET_FRAME&)>;
		using CLOSED_HANDLER = std::function<void(SESSION_ID)>;

		CClientSession(SESSION_ID sessionId, SOCKET socketHandle,
			CIocpService& service, FRAME_HANDLER onFrame, CLOSED_HANDLER onClosed);
		~CClientSession();
		bool Start();
		void Request_Close();
		void Stop();
		bool Send_Frame(LostArk::Shared::PACKET_TYPE packetType,
			std::span<const std::uint8_t> payload);
		void Bind_PlayerId(LostArk::Shared::PLAYER_ID playerId);
		SESSION_ID Get_SessionId() const;
		LostArk::Shared::PLAYER_ID Get_PlayerId() const;
		bool Is_Open() const;
		int Get_LastErrorCode() const;
		void On_IocpCompleted(std::unique_ptr<IOCP_OPERATION> operation,
			std::uint32_t transferredBytes, std::uint32_t errorCode) override;

	private:
		bool Post_Receive();
		bool Start_NextSend();
		bool Post_SendOperation(std::unique_ptr<IOCP_OPERATION> operation);
		void Handle_ReceivedBytes(std::span<const std::uint8_t> bytes);
		void Notify_Closed();

	private:
		SESSION_ID m_iSessionId = INVALID_SESSION_ID;
		SOCKET m_hClientSocket = INVALID_SOCKET;
		CIocpService* m_pService = nullptr;
		FRAME_HANDLER m_OnFrame;
		CLOSED_HANDLER m_OnClosed;
		LostArk::Shared::CPacketStreamParser m_StreamParser;
		std::atomic<LostArk::Shared::PLAYER_ID> m_iPlayerId{
			LostArk::Shared::INVALID_PLAYER_ID };
		std::atomic<int> m_iLastErrorCode{ 0 };
		std::atomic_bool m_isOpen{ false };
		std::atomic_bool m_hasNotifiedClosed{ false };
		std::mutex m_SendMutex;
		std::deque<std::vector<std::uint8_t>> m_SendQueue;
		bool m_isSendPending = false;
	};
}
```

```cpp
// FILE: Server/Private/ClientSession.cpp
// COMPLETE IOCP RECEIVE/SEND CORE

LostArk::Server::CClientSession::CClientSession(
	SESSION_ID sessionId,
	SOCKET socketHandle,
	CIocpService& service,
	FRAME_HANDLER onFrame,
	CLOSED_HANDLER onClosed)
	: m_iSessionId{ sessionId }
	, m_hClientSocket{ socketHandle }
	, m_pService{ &service }
	, m_OnFrame{ std::move(onFrame) }
	, m_OnClosed{ std::move(onClosed) }
{}

LostArk::Server::CClientSession::~CClientSession()
{
	Stop();
}

bool LostArk::Server::CClientSession::Start()
{
	if (nullptr == m_pService || INVALID_SOCKET == m_hClientSocket ||
		!m_pService->Associate(m_hClientSocket)) return false;
	m_isOpen.store(true);
	return Post_Receive();
}

bool LostArk::Server::CClientSession::Post_Receive()
{
	if (!m_isOpen.load()) return false;
	auto operation = std::make_unique<IOCP_OPERATION>();
	operation->eType = IOCP_OPERATION_TYPE::RECEIVE;
	operation->pSink = this;
	operation->pService = m_pService;
	operation->pKeepAlive = shared_from_this();
	operation->Bind_ReceiveBuffer();
	DWORD flags = 0, received = 0;
	m_pService->Track_PostedOperation();
	const int result = WSARecv(m_hClientSocket, &operation->Buffer, 1,
		&received, &flags, &operation->Overlapped, nullptr);
	if (SOCKET_ERROR == result && WSAGetLastError() != WSA_IO_PENDING)
	{
		m_pService->Track_FailedPost();
		return false;
	}
	operation.release();
	return true;
}

bool LostArk::Server::CClientSession::Send_Frame(
	LostArk::Shared::PACKET_TYPE packetType,
	std::span<const std::uint8_t> payload)
{
	std::vector<std::uint8_t> frame;
	if (!m_isOpen.load() || !LostArk::Shared::Build_Packet_Frame(packetType, payload, frame))
		return false;
	std::scoped_lock lock{ m_SendMutex };
	m_SendQueue.push_back(std::move(frame));
	return m_isSendPending || Start_NextSend();
}

bool LostArk::Server::CClientSession::Start_NextSend()
{
	if (m_SendQueue.empty()) { m_isSendPending = false; return true; }
	auto operation = std::make_unique<IOCP_OPERATION>();
	operation->eType = IOCP_OPERATION_TYPE::SEND;
	operation->pSink = this;
	operation->pService = m_pService;
	operation->pKeepAlive = shared_from_this();
	operation->SendBuffer = std::move(m_SendQueue.front());
	m_SendQueue.pop_front();
	return Post_SendOperation(std::move(operation));
}

bool LostArk::Server::CClientSession::Post_SendOperation(
	std::unique_ptr<IOCP_OPERATION> operation)
{
	if (nullptr == operation || operation->iOffset >= operation->SendBuffer.size())
		return false;
	operation->Bind_SendBuffer();
	DWORD sent = 0;
	m_pService->Track_PostedOperation();
	const int result = WSASend(m_hClientSocket, &operation->Buffer, 1,
		&sent, 0, &operation->Overlapped, nullptr);
	if (SOCKET_ERROR == result && WSAGetLastError() != WSA_IO_PENDING)
	{
		m_pService->Track_FailedPost();
		return false;
	}
	m_isSendPending = true;
	operation.release();
	return true;
}

void LostArk::Server::CClientSession::On_IocpCompleted(
	std::unique_ptr<IOCP_OPERATION> operation,
	std::uint32_t transferredBytes,
	std::uint32_t errorCode)
{
	if (nullptr == operation) return;
	if (0 != errorCode || 0 == transferredBytes)
	{
		m_iLastErrorCode.store(static_cast<int>(errorCode));
		Request_Close(); Notify_Closed(); return;
	}
	if (operation->eType == IOCP_OPERATION_TYPE::RECEIVE)
	{
		Handle_ReceivedBytes(std::span<const std::uint8_t>(
			operation->ReceiveBuffer.data(), transferredBytes));
		if (!Post_Receive()) { Request_Close(); Notify_Closed(); }
	}
	else if (operation->eType == IOCP_OPERATION_TYPE::SEND)
	{
		operation->iOffset += transferredBytes;
		if (operation->iOffset < operation->SendBuffer.size())
		{
			if (!Post_SendOperation(std::move(operation)))
			{ Request_Close(); Notify_Closed(); }
			return;
		}
		std::scoped_lock lock{ m_SendMutex };
		m_isSendPending = false;
		if (!Start_NextSend()) { Request_Close(); Notify_Closed(); }
	}
}

void LostArk::Server::CClientSession::Request_Close()
{
	if (!m_isOpen.exchange(false)) return;
	if (INVALID_SOCKET != m_hClientSocket)
	{
		CancelIoEx(reinterpret_cast<HANDLE>(m_hClientSocket), nullptr);
		shutdown(m_hClientSocket, SD_BOTH);
	}
}

void LostArk::Server::CClientSession::Stop()
{
	Request_Close();
	std::scoped_lock lock{ m_SendMutex };
	m_SendQueue.clear();
	if (INVALID_SOCKET != m_hClientSocket)
	{
		closesocket(m_hClientSocket);
		m_hClientSocket = INVALID_SOCKET;
	}
}

void LostArk::Server::CClientSession::Notify_Closed()
{
	if (m_hasNotifiedClosed.exchange(true)) return;
	if (m_OnClosed) m_OnClosed(m_iSessionId);
}

void LostArk::Server::CClientSession::Bind_PlayerId(
	LostArk::Shared::PLAYER_ID playerId)
{
	m_iPlayerId.store(playerId);
}

LostArk::Server::SESSION_ID
LostArk::Server::CClientSession::Get_SessionId() const
{
	return m_iSessionId;
}

LostArk::Shared::PLAYER_ID
LostArk::Server::CClientSession::Get_PlayerId() const
{
	return m_iPlayerId.load();
}

bool LostArk::Server::CClientSession::Is_Open() const
{
	return m_isOpen.load() && INVALID_SOCKET != m_hClientSocket;
}

int LostArk::Server::CClientSession::Get_LastErrorCode() const
{
	return m_iLastErrorCode.load();
}

void LostArk::Server::CClientSession::Handle_ReceivedBytes(
	std::span<const std::uint8_t> bytes)
{
	using namespace LostArk::Shared;
	if (!m_StreamParser.Append(bytes)) { Request_Close(); return; }
	for (;;)
	{
		PACKET_FRAME frame{};
		const PACKET_PARSE_RESULT result = m_StreamParser.Try_Pop(frame);
		if (result == PACKET_PARSE_RESULT::NEED_MORE_DATA) break;
		if (result == PACKET_PARSE_RESULT::INVALID_FRAME) { Request_Close(); break; }
		if (m_OnFrame) m_OnFrame(m_iSessionId, frame);
	}
}
```

```cpp
// FILE: Server/Public/ServerApp.h
// REPLACE LISTENER/ACCEPT THREAD MEMBERS; KEEP Room Thread AND CALLBACKS

#include "IocpListener.h"
#include "IocpService.h"

CIocpService m_IocpService;
CIocpListener m_IocpListener;
// REMOVE m_AcceptThread AND Accept_Loop()
```

```cpp
// FILE: Server/Private/ServerApp.cpp
// Run() STARTUP REPLACEMENT

const std::size_t workerCount = (std::max)(2u, std::thread::hardware_concurrency());
if (!m_IocpService.Start(workerCount) ||
	!m_IocpListener.Open(m_IocpService, 7777,
		[this](SOCKET socketHandle)
		{
			const SESSION_ID sessionId = m_iNextSessionId.fetch_add(1);
			auto session = std::make_shared<CClientSession>(
				sessionId, socketHandle, m_IocpService,
				[this](SESSION_ID id, const LostArk::Shared::PACKET_FRAME& frame)
				{ On_SessionFrame(id, frame); },
				[this](SESSION_ID id) { On_SessionClosed(id); });
			{
				std::scoped_lock lock{ m_SessionsMutex };
				m_Sessions.emplace(sessionId, session);
			}
			ROOM_COMMAND registerCommand{};
			registerCommand.eType = ROOM_COMMAND_TYPE::REGISTER_SESSION;
			registerCommand.iSessionId = sessionId;
			registerCommand.pSession = session;
			if (!m_GameRoom.Enqueue(std::move(registerCommand)) || !session->Start())
				session->Request_Close();
		}))
{
	return 1;
}
```

```cpp
// FILE: Server/Private/ServerApp.cpp
// Shutdown() ORDER

m_isRunning.store(false);
m_IocpListener.Close();
// Request_Close every session while IOCP workers are still alive.
// Drain/reap closed sessions.
if (m_RoomThread.joinable()) m_RoomThread.join();
m_IocpService.Wait_ForIdle(std::chrono::milliseconds{ 5000 });
m_IocpService.Stop();
m_WinSockContext.Shutdown();
```

```xml
<!-- FILE: Server/Default/Server.vcxproj -->
<ClInclude Include="..\Public\IocpOperation.h" />
<ClInclude Include="..\Public\IocpService.h" />
<ClInclude Include="..\Public\IocpListener.h" />
<ClCompile Include="..\Private\IocpOperation.cpp" />
<ClCompile Include="..\Private\IocpService.cpp" />
<ClCompile Include="..\Private\IocpListener.cpp" />
```

```xml
<!-- FILE: Server/Default/Server.vcxproj.filters -->
<Filter Include="Public\Transport"><UniqueIdentifier>{5E0C70E4-F410-4E73-9C44-584B370EF533}</UniqueIdentifier></Filter>
<Filter Include="Private\Transport"><UniqueIdentifier>{BB041580-4AFF-456B-9F06-28EB5558BA61}</UniqueIdentifier></Filter>
<ClInclude Include="..\Public\IocpOperation.h"><Filter>Public\Transport</Filter></ClInclude>
<ClInclude Include="..\Public\IocpService.h"><Filter>Public\Transport</Filter></ClInclude>
<ClInclude Include="..\Public\IocpListener.h"><Filter>Public\Transport</Filter></ClInclude>
<ClCompile Include="..\Private\IocpOperation.cpp"><Filter>Private\Transport</Filter></ClCompile>
<ClCompile Include="..\Private\IocpService.cpp"><Filter>Private\Transport</Filter></ClCompile>
<ClCompile Include="..\Private\IocpListener.cpp"><Filter>Private\Transport</Filter></ClCompile>
```

```powershell
$msbuild = "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
& $msbuild Shared\Default\Shared.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
& $msbuild Tools\NetworkProtocolHarness\Default\NetworkProtocolHarness.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
& Tools\NetworkProtocolHarness\Bin\Debug\NetworkProtocolHarness.exe
& $msbuild Server\Default\Server.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
& $msbuild Client\Default\Client.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
```

```text
Server has no per-session Receive thread
AcceptEx/WSARecv/WSASend all complete through one IOCP service
IOCP worker only parses frames and enqueues RoomCommand through ServerApp
IOCP worker never mutates GameRoom players/monsters/boss/party/dungeon state
GameRoom Tick remains the only server truth writer
Room -> CClientSession::Send_Frame contract unchanged
split frame, merged frames, partial send ordering, disconnect, reconnect all pass
Server 1 + Client 32 soak test for 10 minutes
all clients move/chat/skill snapshots converge
shutdown has no hang, use-after-free, double close, or lost completion
```

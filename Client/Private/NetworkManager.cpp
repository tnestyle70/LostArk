#include "NetworkManager.h"

#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"

#include <string>
#include <vector>

#include <array>
#include <utility>

//Socket worker thread와 client main thread를 분리하기 위해서 존재한다.
//workter thread -> byte 수신과 frame 조립만 수행
//main thread -> frame 해석과 replication event 생성

CNetworkManager& CNetworkManager::Get()
{
	static CNetworkManager instance;
	return instance;
}

std::string CNetworkManager::Resolve_ServerHost()
{
	constexpr char DEFAULT_SERVER_HOST[] = "127.0.0.1";
	constexpr char SERVER_HOST_ENVIRONMENT[] = "LOSTARK_SERVER_HOST";
	char configuredHost[64]{};
	const DWORD configuredLength = ::GetEnvironmentVariableA(
		SERVER_HOST_ENVIRONMENT,
		configuredHost,
		static_cast<DWORD>(std::size(configuredHost)));
	if (0 == configuredLength ||
		configuredLength >= std::size(configuredHost) ||
		"0.0.0.0" == std::string_view{ configuredHost })
	{
		return DEFAULT_SERVER_HOST;
	}
	return configuredHost;
}

bool CNetworkManager::Initialize()
{
	if (m_isWinSocketInitialized)
		return true;

	WSADATA winSockData{};
	const int result = ::WSAStartup(MAKEWORD(2, 2), &winSockData);
	if (0 != result)
	{
		m_iLastErrorCode = result;
		return false;
	}

	const bool isVersionSupported =
		2 == LOBYTE(winSockData.wVersion) &&
		2 == HIBYTE(winSockData.wVersion);

	if (!isVersionSupported)
	{
		m_iLastErrorCode = WSAVERNOTSUPPORTED;
		::WSACleanup();
		return false;
	}

	m_isWinSocketInitialized = true;
	m_iLastErrorCode = 0;
	return true;
}

void CNetworkManager::Shutdown()
{
	Close_ServerConnection();

	if (!m_isWinSocketInitialized)
		return;

	::WSACleanup();
	m_isWinSocketInitialized = false;
}
//매 프레임마다 main thread에서 호출
void CNetworkManager::Update()
{
	//Inbound mutex 잠금 -> Worker가 넣은 raw frame queue를 지역 queue와 swap
	//mutex 해제 -> frame을 도착 순서대로 handle_frame에 전달

	std::deque<LostArk::Shared::PACKET_FRAME> receivedFrames;
	// Worker가 완성한 Frame을 Main Thread로 옮겨 Packet 메시지와
	// Replication Event로 번역한다. Engine 객체는 여기서 직접 생성하지 않는다.
	{
		std::scoped_lock lock
		{
			m_InboundMutex
		};
		//swap을 통해서 frame을 해석하는 동안 network workter가 계속
		//새 frame을 넣을 수 있다.
		receivedFrames.swap(m_InboundFrames);
	}
	
	for (const auto& frame : receivedFrames)
	{
		Handle_Frame(frame);
	}
}

bool CNetworkManager::Connect_To_Server(
	const std::string_view host,
	const std::uint16_t port)
{
	if (!m_isWinSocketInitialized)
	{
		m_iLastErrorCode = WSANOTINITIALISED;
		return false;
	}

	if (Is_Connected())
		return true;
	if (host.empty() || host.size() > 63u || 0u == port)
	{
		m_iLastErrorCode = WSAEINVAL;
		return false;
	}

	// 상대가 먼저 연결을 닫으면 Receive Thread는 끝났어도 joinable 상태일 수 있다.
	// 새 Socket과 Thread를 만들기 전에 이전 연결 자원을 완전히 회수한다.
	if (INVALID_SOCKET != m_hServerSocket ||
		m_ReceiveThread.joinable())
	{
		Close_ServerConnection();
	}

	m_hServerSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == m_hServerSocket)
	{
		m_iLastErrorCode = ::WSAGetLastError();
		return false;
	}

	sockaddr_in serverAddress{};
	serverAddress.sin_family = AF_INET;
	const std::string hostText{ host };
	if ("localhost" == hostText)
	{
		serverAddress.sin_addr.s_addr = ::htonl(INADDR_LOOPBACK);
	}
	else if (1 != ::InetPtonA(
		AF_INET,
		hostText.c_str(),
		&serverAddress.sin_addr))
	{
		m_iLastErrorCode = WSAEINVAL;
		Close_ServerConnection();
		return false;
	}
	serverAddress.sin_port = ::htons(port);

	u_long nonBlocking = 1;
	if (SOCKET_ERROR == ::ioctlsocket(
		m_hServerSocket,
		FIONBIO,
		&nonBlocking))
	{
		m_iLastErrorCode = ::WSAGetLastError();
		Close_ServerConnection();
		return false;
	}

	const int connectResult = ::connect(
		m_hServerSocket,
		reinterpret_cast<const sockaddr*>(&serverAddress),
		sizeof(serverAddress));
	if (SOCKET_ERROR == connectResult)
	{
		const int connectError = ::WSAGetLastError();
		if (WSAEWOULDBLOCK != connectError)
		{
			m_iLastErrorCode = connectError;
			Close_ServerConnection();
			return false;
		}

		fd_set writableSockets;
		FD_ZERO(&writableSockets);
		FD_SET(m_hServerSocket, &writableSockets);
		fd_set errorSockets;
		FD_ZERO(&errorSockets);
		FD_SET(m_hServerSocket, &errorSockets);
		timeval timeout{};
		timeout.tv_sec = 1;
		timeout.tv_usec = 500000;
		const int selectResult = ::select(
			0,
			nullptr,
			&writableSockets,
			&errorSockets,
			&timeout);
		if (selectResult <= 0)
		{
			m_iLastErrorCode = 0 == selectResult ?
				WSAETIMEDOUT : ::WSAGetLastError();
			Close_ServerConnection();
			return false;
		}

		int socketError = 0;
		int socketErrorSize = sizeof(socketError);
		if (SOCKET_ERROR == ::getsockopt(
			m_hServerSocket,
			SOL_SOCKET,
			SO_ERROR,
			reinterpret_cast<char*>(&socketError),
			&socketErrorSize) ||
			0 != socketError)
		{
			m_iLastErrorCode = 0 != socketError ?
				socketError : ::WSAGetLastError();
			Close_ServerConnection();
			return false;
		}
	}

	nonBlocking = 0;
	if (SOCKET_ERROR == ::ioctlsocket(
		m_hServerSocket,
		FIONBIO,
		&nonBlocking))
	{
		m_iLastErrorCode = ::WSAGetLastError();
		Close_ServerConnection();
		return false;
	}

	m_StreamParser.Reset();
	m_ReplicationEvents.clear();
	m_WorldEntitySpawnResults.clear();
	m_hasPendingEnterAccepted = false;
	m_PendingEnterAccepted = {};
	m_iLocalPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
	m_iLocalNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
	m_eWorldId = LostArk::Shared::WORLD_ID::END;
	m_eLocalCharacterClass = LostArk::Shared::CHARACTER_CLASS_ID::END;
	m_iLastErrorCode.store(0);
	m_isReceiveRunning.store(true);
	m_ReceiveThread = std::thread(
		&CNetworkManager::Receive_Loop,
		this,
		m_hServerSocket);
	return true;
}
bool CNetworkManager::Send_EnterWorld(
	LostArk::Shared::WORLD_ID worldId,
	LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	std::string_view nickName)
{
	using namespace LostArk::Shared;

	if (!Is_Connected())
		return false;

	C2S_ENTER_WORLD message{};
	message.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
	message.eWorldId = worldId;
	message.eCharacterClass = characterClass;
	message.strNickName = std::string{ nickName };

	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;

	std::vector<std::uint8_t> frameBytes;
	if (!Build_Packet_Frame(
		PACKET_TYPE::C2S_ENTER_WORLD,
		payloadWriter.Get_Buffer(),
		frameBytes))
	{
		return false;
	}

	if (!Send_All(frameBytes))
		return false;

	m_eLocalCharacterClass = characterClass;
	return true;
}

bool CNetworkManager::Send_MoveGoal(std::uint32_t clientSequence, float goalX, float goalZ)
{
	//연결 상태 검사 -> C2S_MOVE 값 구조체 생성 -> sequence와 goal XZ 저장
	//packetwriter로 payload 직렬화 -> C2S_MOVE frame 생성 -> send_all
	//client sequence가 animation의 정보를 담고 있는 건가? 애니메이션 1 2 3 4 형태의 정수를 담고 있다?
	//이 애니메이션에 대한 부분은 어떻게 처리해야 할까?
	using namespace LostArk::Shared;

	if (!Is_Connected())
		return false;

	C2S_MOVE message{};
	message.iClientSequence = clientSequence;
	message.fGoalX = goalX;
	message.fGoalZ = goalZ;

	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;

	std::vector<std::uint8_t> frameBytes;
	if (!Build_Packet_Frame(
		PACKET_TYPE::C2S_MOVE,
		payloadWriter.Get_Buffer(),
		frameBytes))
	{
		return false;
	}

	return Send_All(frameBytes);
}

bool CNetworkManager::Send_UseSkill(
	const std::uint32_t clientSequence,
	const LostArk::Shared::SKILL_ID skillId,
	const float aimX,
	const float aimZ)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	C2S_USE_SKILL message{};
	message.iClientSequence = clientSequence;
	message.iSkillId = skillId;
	message.fAimX = aimX;
	message.fAimZ = aimZ;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_USE_SKILL,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_SpawnWorldEntity(
	const std::string_view placementId)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;

	C2S_SPAWN_WORLD_ENTITY message{};
	message.strPlacementId = std::string{ placementId };
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;

	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_SPAWN_WORLD_ENTITY,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Try_Consume_EnterAccepted(LostArk::Shared::S2C_ENTER_ACCEPTED& message)
{
	// 승인 하나를 한 번만 소비하여 Lobby가 같은 승인으로 Level을 중복 전환하지 않게 한다.
	if (!m_hasPendingEnterAccepted)
		return false;

	message = m_PendingEnterAccepted;

	m_hasPendingEnterAccepted = false;

	return true;
}

bool CNetworkManager::Try_Consume_WorldEntitySpawnResult(
	LostArk::Shared::S2C_WORLD_ENTITY_SPAWN_RESULT& message)
{
	if (m_WorldEntitySpawnResults.empty())
		return false;
	message = std::move(m_WorldEntitySpawnResults.front());
	m_WorldEntitySpawnResults.pop_front();
	return true;
}

bool CNetworkManager::Try_Consume_ReplicationEvent(Client::CLIENT_REPLICATION_EVENT& event)
{
	if (m_ReplicationEvents.empty())
	{
		return false;
	}

	event = std::move(m_ReplicationEvents.front());
	m_ReplicationEvents.pop_front();
	return true;
}

void CNetworkManager::Close_ServerConnection()
{
	m_isReceiveRunning.store(false);
	const SOCKET socketToClose = m_hServerSocket;
	m_hServerSocket = INVALID_SOCKET;

	if (INVALID_SOCKET != socketToClose)
	{
		::shutdown(socketToClose, SD_BOTH);
		::closesocket(socketToClose);
	}

	if (m_ReceiveThread.joinable())
		m_ReceiveThread.join();

	{
		std::scoped_lock lock{ m_InboundMutex };
		m_InboundFrames.clear();
	}

	m_StreamParser.Reset();
	m_ReplicationEvents.clear();
	m_WorldEntitySpawnResults.clear();
	m_hasPendingEnterAccepted = false;
	m_PendingEnterAccepted = {};
	m_iLocalPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
	m_iLocalNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
	m_eWorldId = LostArk::Shared::WORLD_ID::END;
	m_eLocalCharacterClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
}

bool CNetworkManager::Is_Connected() const
{
	return
		INVALID_SOCKET != m_hServerSocket &&
		m_isReceiveRunning.load();
}

int CNetworkManager::Get_LastErrorCode() const
{
	return m_iLastErrorCode.load();
}

LostArk::Shared::PLAYER_ID CNetworkManager::Get_LocalPlayerId() const
{
	return m_iLocalPlayerId;
}

LostArk::Shared::NET_ENTITY_ID CNetworkManager::Get_LocalEntityId() const
{
	return m_iLocalNetEntityId;
}

LostArk::Shared::CHARACTER_CLASS_ID
CNetworkManager::Get_LocalCharacterClass() const
{
	return m_eLocalCharacterClass;
}

void CNetworkManager::Receive_Loop(const SOCKET serverSocket)
{
	//recv()로 server가 보낸 바이트를 받는다
	//받은 바이트를 PacketStreamParser에 추가한다.
	//parser에서 완성된 프레임을 가능한만큼 꺼낸다.
	//완성된 프레임을 inbound queue에 넣는다.
	using namespace LostArk::Shared;

	std::array<std::uint8_t, 4096> receiveBuffer{};

	// Main Thread의 Connect/Close가 값을 바꾸고 Receive Worker가 반복 조건으로 읽는다.
	while (m_isReceiveRunning.load())
	{
		//serversocket에 있는 data recv로 읽기
		const int receiveByteCount = ::recv(
			serverSocket,
			reinterpret_cast<char*>(
				receiveBuffer.data()),
			static_cast<int>(
				receiveBuffer.size()),
			0);
		//ByteCount를 통해서 오류 및 연결 여부 판단

		//상대가 정상적으로 연결을 종료햇다.
		if (0 == receiveByteCount)
			break;

		//socket I/O 오류 또는 shutdown으로 recv가 해제됐다.
		if (SOCKET_ERROR == receiveByteCount)
		{
			const int errorCode = ::WSAGetLastError();

			//사용자가 종료한 경우의 오류는 실제 통신 오류로 기록 X
			if (m_isReceiveRunning.load())
				m_iLastErrorCode.store(errorCode);

			break;
		}

		// recv 결과는 Header와 Payload 경계를 보장하지 않는 TCP 바이트 조각이다.
		// Parser가 여러 recv 조각을 누적하여 완성된 Frame으로 복원한다.
		const std::span<const std::uint8_t> receiveBytes
		{
			receiveBuffer.data(), static_cast<std::size_t>(receiveByteCount)
		};
		//TCP에서 받은 바이트를 Parser의 누적 버퍼에 붙인다.
		if (!m_StreamParser.Append(receiveBytes))
		{
			m_iLastErrorCode.store(WSAEMSGSIZE);
			break;
		}
		//이번 recv에 완성된 프레임이 여러 개 들어왔을 수 있다. ;; 무한 루프 돌면서 파악
		for (;;)
		{
			// PACKET_FRAME은 Header에서 복원한 PacketType과 Payload를 가진 의미 단위다.
			PACKET_FRAME frame{};

			const PACKET_PARSE_RESULT parseResult = m_StreamParser.Try_Pop(frame);

			if (PACKET_PARSE_RESULT::NEED_MORE_DATA == parseResult)
			{
				//아직 프레임 하나가 완성되지 않았기 때문에, 다음 결과를 기다린다.
				break;
			}
			if (PACKET_PARSE_RESULT::INVALID_FRAME == parseResult)
			{
				//잘못된 크기 또는 패킷 타입이 발견됏다.
				m_iLastErrorCode.store(WSAEPROTONOSUPPORT);
				
				m_isReceiveRunning.store(false);
				return;
			}
			//FRAME_READY인 경우에만 main thread 전달 큐에 넣는다.
			{
				std::scoped_lock lock{
				   m_InboundMutex
				};

				m_InboundFrames.push_back(
					std::move(frame));
			}
		}
	}
	m_isReceiveRunning.store(false);
}

void CNetworkManager::Handle_Frame(const LostArk::Shared::PACKET_FRAME & frame)
{
	using namespace LostArk::Shared;

	//frame의 payload 정보를 읽는다. packet - 정보를 담는 header와 payload - class,strName 이렇게 2개로 나뉜다
	CPacketReader reader{ frame.Payload };

	switch (frame.ePacketType)
	{
	//Server Enter
	case PACKET_TYPE::S2C_ENTER_ACCEPTED:
	{
		S2C_ENTER_ACCEPTED accepted{};

		if (!Read_Message(reader, accepted) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}

		m_iLocalPlayerId = accepted.iPlayerId;
		m_iLocalNetEntityId = accepted.iNetEntityId;
		m_eWorldId = accepted.eWorldId;
		m_hasPendingEnterAccepted = true;
		m_PendingEnterAccepted = accepted;
		break;
	}
	//Player Spawn
	case PACKET_TYPE::S2C_PLAYER_SPAWNED:
	{
		S2C_PLAYER_SPAWNED spawned{};

		if (!Read_Message(reader, spawned) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		//Client Replication Event 생성
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::PLAYER_SPAWNED;
		event.PlayerSpawned = std::move(spawned);
		m_ReplicationEvents.push_back(std::move(event));
		break;
	}
	case PACKET_TYPE::S2C_WORLD_ENTITY_SPAWNED:
	{
		S2C_WORLD_ENTITY_SPAWNED spawned{};
		if (!Read_Message(reader, spawned) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType =
			Client::CLIENT_REPLICATION_EVENT_TYPE::WORLD_ENTITY_SPAWNED;
		event.WorldEntitySpawned = std::move(spawned);
		m_ReplicationEvents.push_back(std::move(event));
		break;
	}
	case PACKET_TYPE::S2C_WORLD_ENTITY_DESPAWNED:
	{
		S2C_WORLD_ENTITY_DESPAWNED despawned{};
		if (!Read_Message(reader, despawned) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType =
			Client::CLIENT_REPLICATION_EVENT_TYPE::WORLD_ENTITY_DESPAWNED;
		event.WorldEntityDespawned = despawned;
		m_ReplicationEvents.push_back(std::move(event));
		break;
	}
	case PACKET_TYPE::S2C_WORLD_ENTITY_SPAWN_RESULT:
	{
		S2C_WORLD_ENTITY_SPAWN_RESULT result{};
		if (!Read_Message(reader, result) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		m_WorldEntitySpawnResults.push_back(std::move(result));
		break;
	}
	//snapshot
	case PACKET_TYPE::S2C_WORLD_SNAPSHOT:
	{
		//world의 snapshot에 대한 구조체 생성
		S2C_WORLD_SNAPSHOT snapshot{};

		if (!Read_Message(reader, snapshot) ||
			0 != reader.Get_RemainingSize() ||
			snapshot.eWorldId != m_eWorldId)
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}

		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType =
			Client::CLIENT_REPLICATION_EVENT_TYPE::WORLD_SNAPSHOT;
		event.WorldSnapshot = std::move(snapshot);
		m_ReplicationEvents.push_back(std::move(event));
		break;
	}
	//player despawn
	case PACKET_TYPE::S2C_PLAYER_DESPAWNED:
	{
		S2C_PLAYER_DESPAWNED despawned{};

		if (!Read_Message(reader, despawned) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::PLAYER_DESPAWNED;
		event.PlayerDespawned = despawned;
		m_ReplicationEvents.push_back(std::move(event));
		break;
	}
	default:
		break;
	}
}

bool CNetworkManager::Send_All(
	std::span<const std::uint8_t> bytes)
{
	if (!Is_Connected())
		return false;

	std::size_t sentByteCount = 0;

	while (sentByteCount < bytes.size())
	{
		const int result = ::send(
			m_hServerSocket,
			reinterpret_cast<const char*>(
				bytes.data() + sentByteCount),
			static_cast<int>(bytes.size() - sentByteCount),
			0);

		if (SOCKET_ERROR == result)
		{
			m_iLastErrorCode.store(::WSAGetLastError());
			return false;
		}

		if (0 == result)
			return false;

		sentByteCount += static_cast<std::size_t>(result);
	}

	return true;
}

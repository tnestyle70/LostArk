#include "NetworkManager.h"

#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"

#include <string>
#include <vector>

#include <array>
#include <utility>

CNetworkManager& CNetworkManager::Get()
{
	static CNetworkManager instance;
	return instance;
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

void CNetworkManager::Update()
{
	std::deque<LostArk::Shared::PACKET_FRAME> receivedFrames;
	//어떤 역할과 기능을 update tick 마다 수행하는 건지 모르겠음
	//-> main thread가 worker의 inbound queue를 가져와 처리한다.
	{
		std::scoped_lock lock{
			m_InboundMutex
		};

		receivedFrames.swap(m_InboundFrames);
	}
	
	for (const auto& frame : receivedFrames)
	{
		Handle_Frame(frame);
	}
}

bool CNetworkManager::Connect_To_Server(std::uint16_t port)
{
	if (!m_isWinSocketInitialized)
	{
		m_iLastErrorCode = WSANOTINITIALISED;
		return false;
	}

	if (Is_Connected())
		return true;

	m_hServerSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == m_hServerSocket)
	{
		m_iLastErrorCode = ::WSAGetLastError();
		return false;
	}

	sockaddr_in serverAddress{};
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = ::htonl(INADDR_LOOPBACK);
	serverAddress.sin_port = ::htons(port);

	if (SOCKET_ERROR == ::connect(
		m_hServerSocket,
		reinterpret_cast<const sockaddr*>(&serverAddress),
		sizeof(serverAddress)))
	{
		m_iLastErrorCode = ::WSAGetLastError();
		Close_ServerConnection();
		return false;
	}

	m_StreamParser.Reset();

	m_hasPendingEnterAccepted = false;

	m_PendingEnterAccepted = {};

	m_iLocalPlayerId =
		LostArk::Shared::INVALID_PLAYER_ID;

	m_iLocalNetEntityId =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	//atomic이라서 store?
	m_iLastErrorCode.store(0);

	m_isReceiveRunning.store(true);

	m_ReceiveThread = std::thread(
		&CNetworkManager::Receive_Loop,
		this);

	return true;
}

bool CNetworkManager::Send_EnterWorld(
	LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	std::string_view nickName)
{
	using namespace LostArk::Shared;

	if (!Is_Connected())
		return false;

	C2S_ENTER_WORLD message{};
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

	return Send_All(frameBytes);
}

bool CNetworkManager::Try_Consume_EnterAccepted(LostArk::Shared::S2C_ENTER_ACCEPTED& message)
{
	//중복 enter를 방지하기 위함?
	if (!m_hasPendingEnterAccepted)
		return false;

	message = m_PendingEnterAccepted;

	m_hasPendingEnterAccepted = false;

	return true;
}

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

void CNetworkManager::Receive_Loop()
{
	//recv()로 server가 보낸 바이트를 받는다
	//받은 바이트를 PacketStreamParser에 추가한다.
	//parser에서 완성된 프레임을 가능한만큼 꺼낸다.
	//완성된 프레임을 inbound queue에 넣는다.
	using namespace LostArk::Shared;

	std::array<std::uint8_t, 4096> receiveBuffer{};

	//이 bool 상태는 누가 어떻게 체크함?
	while (m_isReceiveRunning.load())
	{
		//serversocket에 있는 data recv로 읽기
		const int receiveByteCount = ::recv(
			m_hServerSocket,
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

		//실질적인 payload? 이거는 header랑 전부 같이 있는 거 맞지? Byte를 읽어서 receiveBytes에 저장한다.

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
			//packet의 type과 payload 실질적인 내용으로 구성된 packet frame frame이 틀 맞지?
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

	//현재 수직 슬라이스는 승인 패킷 하나만 처리한다.
	if (PACKET_TYPE::S2C_ENTER_ACCEPTED != frame.ePacketType)
	{
		return;
	}
	//frame의 payload 정보를 읽는다. packet - 정보를 담는 header와 payload - class,strName 이렇게 2개로 나뉜다
	CPacketReader reader{ frame.Payload };

	S2C_ENTER_ACCEPTED accepted{};

	//역직렬화와 payload 완전 소비를 모두 검증한다.
	if (!Read_Message(reader, accepted) ||
		0 != reader.Get_RemainingSize())
	{
		m_iLastErrorCode.store(WSAEINVAL);
		return;
	}

	//commit해서 플레이어의 상태를 변경한다.검증 성공 전에는 기존 상태를 바꾸지 않음.
	m_iLocalPlayerId = accepted.iPlayerId;

	m_iLocalNetEntityId = accepted.iNetEntityId;

	m_PendingEnterAccepted = accepted;
	m_hasPendingEnterAccepted = true;
}

bool CNetworkManager::Send_All(
	std::span<const std::uint8_t> bytes)
{
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
			m_iLastErrorCode = ::WSAGetLastError();
			return false;
		}

		if (0 == result)
			return false;

		sentByteCount += static_cast<std::size_t>(result);
	}

	return true;
}

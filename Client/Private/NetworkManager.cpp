#include "NetworkManager.h"

#include "DataJson.h"

#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"

#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include <array>
#include <cmath>
#include <utility>

namespace
{
#ifdef _DEBUG
	std::string Resolve_DebugLocalServerHost()
	{
		constexpr char OVERRIDE_PATH[] = "LocalServerEndpoint.user.json";
		std::ifstream input(OVERRIDE_PATH, std::ios::binary);
		if (!input)
			return {};

		const std::string text(
			(std::istreambuf_iterator<char>(input)),
			std::istreambuf_iterator<char>());
		if (text.empty() || text.size() > 1024u)
			return {};

		Client::DATA_JSON_VALUE root;
		std::string error;
		if (!Client::CDataJson::Parse(text, root, error) ||
			!root.Is_Object() || 4u != root.Get_Object().size())
		{
			return {};
		}

		const Client::DATA_JSON_VALUE* schema = root.Find("schema");
		const Client::DATA_JSON_VALUE* formatVersion =
			root.Find("formatVersion");
		const Client::DATA_JSON_VALUE* enabled = root.Find("enabled");
		const Client::DATA_JSON_VALUE* host = root.Find("host");
		if (nullptr == schema || !schema->Is_String() ||
			"lostark.local-server-endpoint" != schema->Get_String() ||
			nullptr == formatVersion || !formatVersion->Is_Number() ||
			formatVersion->Was_FloatingPointToken() ||
			1.0 != formatVersion->Get_Number() ||
			nullptr == enabled || !enabled->Is_Boolean() ||
			!enabled->Get_Boolean() ||
			nullptr == host || !host->Is_String() ||
			"127.0.0.1" != host->Get_String())
		{
			return {};
		}

		return host->Get_String();
	}
#endif
}

//Socket worker thread�� client main thread�� �и��ϱ� ���ؼ� �����Ѵ�.
//workter thread -> byte ���Ű� frame ������ ����
//main thread -> frame �ؼ��� replication event ����

CNetworkManager& CNetworkManager::Get()
{
	static CNetworkManager instance;
	return instance;
}

std::string CNetworkManager::Resolve_ServerHost()
{
	/* The shared LAN Server was retired on 2026-08-17; each developer runs their
	   own. LOSTARK_SERVER_HOST still wins for anyone pointing at a real host. */
	constexpr char DEFAULT_SERVER_HOST[] = "127.0.0.1";
	constexpr char SERVER_HOST_ENVIRONMENT[] = "LOSTARK_SERVER_HOST";
	char configuredHost[64]{};
	const DWORD configuredLength = ::GetEnvironmentVariableA(
		SERVER_HOST_ENVIRONMENT,
		configuredHost,
		static_cast<DWORD>(std::size(configuredHost)));
	if (0u != configuredLength &&
		configuredLength < std::size(configuredHost) &&
		"0.0.0.0" != std::string_view{ configuredHost })
	{
		return configuredHost;
	}
#ifdef _DEBUG
	/* The VS debugger environment is the team endpoint authority. A developer
	   may still opt into a loopback Server when launching outside VS, but that
	   local convenience file must never silently override an explicit host. */
	if (const std::string localHost = Resolve_DebugLocalServerHost();
		!localHost.empty())
	{
		return localHost;
	}
#endif
	return DEFAULT_SERVER_HOST;
}

std::string CNetworkManager::Resolve_MapEditorServerHost()
{
	// Test(Map Editor) and product worlds must enter through the same
	// authoritative Server endpoint. Keep this compatibility entry point so
	// existing lobby code cannot reintroduce a private loopback route.
	return Resolve_ServerHost();
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
//�� �����Ӹ��� main thread���� ȣ��
void CNetworkManager::Update()
{
	if (m_hasProtocolFailure.load())
	{
		if (INVALID_SOCKET != m_hServerSocket)
			Fail_Protocol(m_iLastErrorCode.load());
		return;
	}

	//Inbound mutex ��� -> Worker�� ���� raw frame queue�� ���� queue�� swap
	//mutex ���� -> frame�� ���� ������� handle_frame�� ����

	std::deque<LostArk::Shared::PACKET_FRAME> receivedFrames;
	// Worker�� �ϼ��� Frame�� Main Thread�� �Ű� Packet �޽�����
	// Replication Event�� �����Ѵ�. Engine ��ü�� ���⼭ ���� �������� �ʴ´�.
	{
		std::scoped_lock lock
		{
			m_InboundMutex
		};
		//swap�� ���ؼ� frame�� �ؼ��ϴ� ���� network workter�� ���
		//�� frame�� ���� �� �ִ�.
		receivedFrames.swap(m_InboundFrames);
	}
	
	for (const auto& frame : receivedFrames)
	{
		Handle_Frame(frame);
		if (m_hasProtocolFailure.load())
			break;
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

	// ��밡 ���� ������ ������ Receive Thread�� ����� joinable ������ �� �ִ�.
	// �� Socket�� Thread�� ����� ���� ���� ���� �ڿ��� ������ ȸ���Ѵ�.
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
	Reset_WorldInboundState();
	m_iLastErrorCode.store(0);
	m_hasProtocolFailure.store(false);
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

	// The request is now committed to the socket. From this point only its
	// future acceptance may establish a world; older room events are stale.
	Reset_WorldInboundState();
	m_eLocalCharacterClass = characterClass;
	return true;
}

bool CNetworkManager::Send_MoveGoal(std::uint32_t clientSequence, float goalX, float goalZ)
{
	//���� ���� �˻� -> C2S_MOVE �� ����ü ���� -> sequence�� goal XZ ����
	//packetwriter�� payload ����ȭ -> C2S_MOVE frame ���� -> send_all
	//client sequence�� animation�� ������ ��� �ִ� �ǰ�? �ִϸ��̼� 1 2 3 4 ������ ������ ��� �ִ�?
	//�� �ִϸ��̼ǿ� ���� �κ��� ��� ó���ؾ� �ұ�?
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

bool CNetworkManager::Send_ReleaseSkill(
	const std::uint32_t clientSequence,
	const LostArk::Shared::SKILL_ID skillId)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	C2S_RELEASE_SKILL message{};
	message.iClientSequence = clientSequence;
	message.iSkillId = skillId;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_RELEASE_SKILL,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_SkillAim(
	const std::uint32_t clientSequence,
	const LostArk::Shared::SKILL_ID skillId,
	const float aimX,
	const float aimZ)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	C2S_UPDATE_SKILL_AIM message{};
	message.iClientSequence = clientSequence;
	message.iSkillId = skillId;
	message.fAimX = aimX;
	message.fAimZ = aimZ;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_UPDATE_SKILL_AIM,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_EstherSkill(
	const std::uint32_t clientSequence,
	const std::uint8_t slotIndex,
	const float aimX,
	const float aimZ)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	C2S_USE_ESTHER_SKILL message{};
	message.iClientSequence = clientSequence;
	message.iSlotIndex = slotIndex;
	message.fAimX = aimX;
	message.fAimZ = aimZ;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_USE_ESTHER_SKILL,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_RevivePlayer(
	const std::uint32_t clientSequence)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	C2S_REVIVE_PLAYER message{};
	message.iClientSequence = clientSequence;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_REVIVE_PLAYER,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_ChangeCharacterClass(
	const std::uint32_t clientSequence,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	C2S_CHANGE_CHARACTER_CLASS message{};
	message.iClientSequence = clientSequence;
	message.eCharacterClass = characterClass;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_CHANGE_CHARACTER_CLASS,
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

bool CNetworkManager::Send_DebugGiveItem(
	const std::uint32_t requestSequence,
	const std::string_view itemId,
	const std::uint32_t quantity)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;

	C2S_DEBUG_GIVE_ITEM message{};
	message.iRequestSequence = requestSequence;
	message.strItemId = std::string{ itemId };
	message.iQuantity = quantity;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;

	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_DEBUG_GIVE_ITEM,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_UseItem(
	const std::uint32_t requestSequence,
	const std::string_view itemId)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;

	C2S_USE_ITEM message{};
	message.iRequestSequence = requestSequence;
	message.strItemId = std::string{ itemId };
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;

	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_USE_ITEM,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_ValtanAudition(
	const std::uint32_t requestSequence,
	const LostArk::Shared::VALTAN_AUDITION_OPERATION operation,
	const std::uint32_t targetHealthBar)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;

	C2S_VALTAN_AUDITION_REQUEST message{};
	message.iRequestSequence = requestSequence;
	message.eOperation = operation;
	message.iTargetHealthBar = targetHealthBar;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;

	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_VALTAN_AUDITION_REQUEST,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Try_Consume_EnterAccepted(LostArk::Shared::S2C_ENTER_ACCEPTED& message)
{
	// ���� �ϳ��� �� ���� �Һ��Ͽ� Lobby�� ���� �������� Level�� �ߺ� ��ȯ���� �ʰ� �Ѵ�.
	if (!m_hasPendingEnterAccepted)
		return false;

	message = m_PendingEnterAccepted;

	m_hasPendingEnterAccepted = false;

	return true;
}

bool CNetworkManager::Try_Consume_EnterRejected(
	LostArk::Shared::S2C_ENTER_REJECTED& message)
{
	if (!m_hasPendingEnterRejected)
		return false;

	message = m_PendingEnterRejected;
	m_hasPendingEnterRejected = false;
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

bool CNetworkManager::Try_Consume_CharacterClassChangeResult(
	LostArk::Shared::S2C_CHARACTER_CLASS_CHANGE_RESULT& message)
{
	if (m_CharacterClassChangeResults.empty())
		return false;
	message = std::move(m_CharacterClassChangeResults.front());
	m_CharacterClassChangeResults.pop_front();
	return true;
}

bool CNetworkManager::Try_Consume_ValtanAuditionResult(
	LostArk::Shared::S2C_VALTAN_AUDITION_RESULT& message)
{
	if (m_ValtanAuditionResults.empty())
		return false;
	message = m_ValtanAuditionResults.front();
	m_ValtanAuditionResults.pop_front();
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

bool CNetworkManager::Enqueue_ReplicationEvent(
	Client::CLIENT_REPLICATION_EVENT&& event)
{
	/* Loading levels do not own a CClientReplication consumer yet, while the
	   admitted Server room continues to publish WORLD_SNAPSHOT at 30 Hz. Keep
	   only the newest adjacent snapshot: spawn/despawn/destruction events stay
	   as ordering barriers, but a cold level load can no longer fill the queue
	   and disconnect with WSAENOBUFS before activation. */
	if (Client::CLIENT_REPLICATION_EVENT_TYPE::WORLD_SNAPSHOT == event.eType &&
		!m_ReplicationEvents.empty() &&
		Client::CLIENT_REPLICATION_EVENT_TYPE::WORLD_SNAPSHOT ==
			m_ReplicationEvents.back().eType)
	{
		m_ReplicationEvents.back() = std::move(event);
		return true;
	}
	if (m_ReplicationEvents.size() >= MAX_REPLICATION_EVENT_QUEUE)
	{
		Fail_Protocol(WSAENOBUFS);
		return false;
	}
	m_ReplicationEvents.push_back(std::move(event));
	return true;
}

void CNetworkManager::Fail_Protocol(const int errorCode)
{
	m_iLastErrorCode.store(errorCode);
	m_hasProtocolFailure.store(true);
	m_isReceiveRunning.store(false);
	const SOCKET socketToClose = m_hServerSocket;
	m_hServerSocket = INVALID_SOCKET;
	if (INVALID_SOCKET != socketToClose)
	{
		::shutdown(socketToClose, SD_BOTH);
		::closesocket(socketToClose);
	}
	{
		std::scoped_lock lock{ m_InboundMutex };
		m_InboundFrames.clear();
	}
	m_StreamParser.Reset();
	Reset_WorldInboundState();
}

void CNetworkManager::Reset_WorldInboundState()
{
	m_ReplicationEvents.clear();
	m_WorldEntitySpawnResults.clear();
	m_CharacterClassChangeResults.clear();
	m_ValtanAuditionResults.clear();
	m_hasPendingEnterAccepted = false;
	m_PendingEnterAccepted = {};
	m_hasPendingEnterRejected = false;
	m_PendingEnterRejected = {};
	m_iLocalPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
	m_iLocalNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
	m_eWorldId = LostArk::Shared::WORLD_ID::END;
	m_eLocalCharacterClass = LostArk::Shared::CHARACTER_CLASS_ID::END;
	m_hasLocalSpawn = false;
	m_LocalSpawn = {};
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
	Reset_WorldInboundState();
	m_hasProtocolFailure.store(false);
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

bool CNetworkManager::Try_Get_LocalSpawn(
	LostArk::Shared::S2C_PLAYER_SPAWNED& outSpawn) const
{
	if (!m_hasLocalSpawn)
		return false;

	outSpawn = m_LocalSpawn;
	return true;
}

void CNetworkManager::Receive_Loop(const SOCKET serverSocket)
{
	//recv()�� server�� ���� ����Ʈ�� �޴´�
	//���� ����Ʈ�� PacketStreamParser�� �߰��Ѵ�.
	//parser���� �ϼ��� �������� �����Ѹ�ŭ ������.
	//�ϼ��� �������� inbound queue�� �ִ´�.
	using namespace LostArk::Shared;

	std::array<std::uint8_t, 4096> receiveBuffer{};

	// Main Thread�� Connect/Close�� ���� �ٲٰ� Receive Worker�� �ݺ� �������� �д´�.
	while (m_isReceiveRunning.load())
	{
		//serversocket�� �ִ� data recv�� �б�
		const int receiveByteCount = ::recv(
			serverSocket,
			reinterpret_cast<char*>(
				receiveBuffer.data()),
			static_cast<int>(
				receiveBuffer.size()),
			0);
		//ByteCount�� ���ؼ� ���� �� ���� ���� �Ǵ�

		//��밡 ���������� ������ �����޴�.
		if (0 == receiveByteCount)
			break;

		//socket I/O ���� �Ǵ� shutdown���� recv�� �����ƴ�.
		if (SOCKET_ERROR == receiveByteCount)
		{
			const int errorCode = ::WSAGetLastError();

			//����ڰ� ������ ����� ������ ���� ��� ������ ��� X
			if (m_isReceiveRunning.load())
				m_iLastErrorCode.store(errorCode);

			break;
		}

		// recv ����� Header�� Payload ��踦 �������� �ʴ� TCP ����Ʈ �����̴�.
		// Parser�� ���� recv ������ �����Ͽ� �ϼ��� Frame���� �����Ѵ�.
		const std::span<const std::uint8_t> receiveBytes
		{
			receiveBuffer.data(), static_cast<std::size_t>(receiveByteCount)
		};
		//TCP���� ���� ����Ʈ�� Parser�� ���� ���ۿ� ���δ�.
		if (!m_StreamParser.Append(receiveBytes))
		{
			m_iLastErrorCode.store(WSAEMSGSIZE);
			break;
		}
		//�̹� recv�� �ϼ��� �������� ���� �� ������ �� �ִ�. ;; ���� ���� ���鼭 �ľ�
		for (;;)
		{
			// PACKET_FRAME�� Header���� ������ PacketType�� Payload�� ���� �ǹ� ������.
			PACKET_FRAME frame{};

			const PACKET_PARSE_RESULT parseResult = m_StreamParser.Try_Pop(frame);

			if (PACKET_PARSE_RESULT::NEED_MORE_DATA == parseResult)
			{
				//���� ������ �ϳ��� �ϼ����� �ʾұ� ������, ���� ����� ��ٸ���.
				break;
			}
			if (PACKET_PARSE_RESULT::INVALID_FRAME == parseResult)
			{
				//�߸��� ũ�� �Ǵ� ��Ŷ Ÿ���� �߰߉Ѵ�.
				m_iLastErrorCode.store(WSAEPROTONOSUPPORT);
				
				m_isReceiveRunning.store(false);
				return;
			}
			//FRAME_READY�� ��쿡�� main thread ���� ť�� �ִ´�.
			{
				std::scoped_lock lock{
				   m_InboundMutex
				};
				/* Level activation can synchronously prepare GPU resources before the
				   main thread resumes Update().  Coalesce adjacent snapshots at the
				   worker boundary as well as the parsed-event boundary so that cold
				   loading cannot exhaust the raw frame queue.  Any lifecycle or
				   destruction frame remains an ordering barrier. */
				if (PACKET_TYPE::S2C_WORLD_SNAPSHOT == frame.ePacketType &&
					!m_InboundFrames.empty() &&
					PACKET_TYPE::S2C_WORLD_SNAPSHOT ==
						m_InboundFrames.back().ePacketType)
				{
					m_InboundFrames.back() = std::move(frame);
					continue;
				}
				if (m_InboundFrames.size() >= MAX_INBOUND_FRAME_QUEUE)
				{
					m_iLastErrorCode.store(WSAENOBUFS);
					m_hasProtocolFailure.store(true);
					m_isReceiveRunning.store(false);
					return;
				}
				m_InboundFrames.push_back(
					std::move(frame));
			}
		}
	}
	m_isReceiveRunning.store(false);
}

void CNetworkManager::Handle_Frame(const LostArk::Shared::PACKET_FRAME & frame)
{
	if (m_hasProtocolFailure.load())
		return;

	using namespace LostArk::Shared;

	//frame�� payload ������ �д´�. packet - ������ ��� header�� payload - class,strName �̷��� 2���� ������
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
			Fail_Protocol(WSAEINVAL);
			return;
		}

		// Acceptance is the generation boundary. It intentionally drops every
		// queued event that may have arrived for the previous room while the
		// loading transition was pending. The requested class belongs to this
		// new generation, so preserve it for the target Level loader.
		const CHARACTER_CLASS_ID requestedCharacterClass =
			m_eLocalCharacterClass;
		Reset_WorldInboundState();
		m_eLocalCharacterClass = requestedCharacterClass;
		m_iLocalPlayerId = accepted.iPlayerId;
		m_iLocalNetEntityId = accepted.iNetEntityId;
		m_eWorldId = accepted.eWorldId;
		m_hasLocalSpawn = false;
		m_LocalSpawn = {};
		m_hasPendingEnterAccepted = true;
		m_PendingEnterAccepted = accepted;
		break;
	}
	case PACKET_TYPE::S2C_ENTER_REJECTED:
	{
		S2C_ENTER_REJECTED rejected{};
		if (!Read_Message(reader, rejected) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		m_hasPendingEnterRejected = true;
		m_PendingEnterRejected = rejected;
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
		//Client Replication Event ����
		if (spawned.iPlayerId == m_iLocalPlayerId &&
			spawned.iNetEntityId == m_iLocalNetEntityId &&
			std::isfinite(spawned.fPositionX) &&
			std::isfinite(spawned.fPositionY) &&
			std::isfinite(spawned.fPositionZ) &&
			std::isfinite(spawned.fYawDegrees))
		{
			m_LocalSpawn = spawned;
			m_hasLocalSpawn = true;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::PLAYER_SPAWNED;
		event.PlayerSpawned = std::move(spawned);
		Enqueue_ReplicationEvent(std::move(event));
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
		Enqueue_ReplicationEvent(std::move(event));
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
		Enqueue_ReplicationEvent(std::move(event));
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
	case PACKET_TYPE::S2C_VALTAN_AUDITION_RESULT:
	{
		S2C_VALTAN_AUDITION_RESULT result{};
		if (!Read_Message(reader, result) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		m_ValtanAuditionResults.push_back(result);
		break;
	}
	case PACKET_TYPE::S2C_CHARACTER_CLASS_CHANGE_RESULT:
	{
		S2C_CHARACTER_CLASS_CHANGE_RESULT result{};
		if (!Read_Message(reader, result) || 0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		if (CHARACTER_CLASS_CHANGE_RESULT::ACCEPTED == result.eResult)
		{
			m_eLocalCharacterClass = result.eActiveClass;
			if (m_hasLocalSpawn)
				m_LocalSpawn.eCharacterClass = result.eActiveClass;
		}
		m_CharacterClassChangeResults.push_back(std::move(result));
		break;
	}
	case PACKET_TYPE::S2C_INVENTORY_SNAPSHOT:
	{
		S2C_INVENTORY_SNAPSHOT snapshot{};
		if (!Read_Message(reader, snapshot) || 0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType =
			Client::CLIENT_REPLICATION_EVENT_TYPE::INVENTORY_SNAPSHOT;
		event.InventorySnapshot = std::move(snapshot);
		Enqueue_ReplicationEvent(std::move(event));
		break;
	}
	//snapshot
	case PACKET_TYPE::S2C_WORLD_SNAPSHOT:
	{
		//world�� snapshot�� ���� ����ü ����
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
		Enqueue_ReplicationEvent(std::move(event));
		break;
	}
	case PACKET_TYPE::S2C_WORLD_DESTRUCTION_FULL_SYNC:
	{
		S2C_WORLD_DESTRUCTION_FULL_SYNC sync{};
		if (!Read_Message(reader, sync) ||
			0 != reader.Get_RemainingSize() ||
			WORLD_ID::VALTAN_ARENA != m_eWorldId)
		{
			Fail_Protocol(WSAEINVAL);
			return;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::
			WORLD_DESTRUCTION_FULL_SYNC;
		event.WorldDestructionFullSync = std::move(sync);
		Enqueue_ReplicationEvent(std::move(event));
		break;
	}
	case PACKET_TYPE::S2C_ENCOUNTER_PROP_SYNC:
	{
		S2C_ENCOUNTER_PROP_SYNC sync{};
		if (!Read_Message(reader, sync) ||
			0 != reader.Get_RemainingSize() ||
			WORLD_ID::VALTAN_ARENA != m_eWorldId)
		{
			Fail_Protocol(WSAEINVAL);
			return;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::
			ENCOUNTER_PROP_SYNC;
		event.EncounterPropSync = std::move(sync);
		Enqueue_ReplicationEvent(std::move(event));
		break;
	}
	case PACKET_TYPE::S2C_WORLD_DESTRUCTION_DELTA:
	{
		S2C_WORLD_DESTRUCTION_DELTA delta{};
		if (!Read_Message(reader, delta) ||
			0 != reader.Get_RemainingSize() ||
			WORLD_ID::VALTAN_ARENA != m_eWorldId)
		{
			Fail_Protocol(WSAEINVAL);
			return;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::
			WORLD_DESTRUCTION_DELTA;
		event.WorldDestructionDelta = std::move(delta);
		Enqueue_ReplicationEvent(std::move(event));
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
		if (despawned.iNetEntityId == m_iLocalNetEntityId)
		{
			m_hasLocalSpawn = false;
			m_LocalSpawn = {};
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::PLAYER_DESPAWNED;
		event.PlayerDespawned = despawned;
		Enqueue_ReplicationEvent(std::move(event));
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

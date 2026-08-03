#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>

#include "ClientReplicationEvent.h"

#include "Network/PacketFrame.h"
#include "Network/PacketMessages.h"
#include "Network/PacketStreamParser.h"

//race를 방지하기 위해서 atomic header를 추가
#include <atomic>
#include <deque>
//동기 접근 race를 막기 위해서 mutex 선언 후 사용
#include <mutex>
#include <thread>
#include <cstdint>
#include <span>
#include <string_view>


class CNetworkManager final
{
public:
	CNetworkManager() = default;
	CNetworkManager(const CNetworkManager&) = delete;
	CNetworkManager& operator=(const CNetworkManager&) = delete;

private:
	~CNetworkManager() = default;

public:
	static CNetworkManager& Get();

	bool Initialize();
	void Shutdown();
	void Update();

	bool Connect_To_Server(
		std::string_view host,
		std::uint16_t port);
	bool Connect_To_Server(std::uint16_t port)
	{
		return Connect_To_Server("127.0.0.1", port);
	}

	bool Send_EnterWorld(
		LostArk::Shared::WORLD_ID worldId,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass,
		std::string_view nickName);
	//playercontroller가 목표 XZ를 전송하는 public 경계
	bool Send_MoveGoal(
		std::uint32_t clientSequence,
		float goalX,
		float goalZ);
	bool Send_UseSkill(
		std::uint32_t clientSequence,
		LostArk::Shared::SKILL_ID skillId,
		float aimX,
		float aimZ);

	bool Try_Consume_EnterAccepted(
		LostArk::Shared::S2C_ENTER_ACCEPTED& message);

	bool Try_Consume_ReplicationEvent(
		Client::CLIENT_REPLICATION_EVENT& event);

	void Close_ServerConnection();

	[[nodiscard]] bool Is_Connected() const;
	[[nodiscard]] int Get_LastErrorCode() const;
	[[nodiscard]] LostArk::Shared::PLAYER_ID Get_LocalPlayerId() const;
	[[nodiscard]] LostArk::Shared::NET_ENTITY_ID Get_LocalEntityId() const;
	[[nodiscard]] LostArk::Shared::CHARACTER_CLASS_ID
		Get_LocalCharacterClass() const;


private:
	bool Send_All(std::span<const std::uint8_t> bytes);
	//수신 worker 하나가 4096-byte 지역 버퍼로 Server의 TCP byte stream을 읽는다.
	void Receive_Loop(SOCKET serverSocket);
	void Handle_Frame(const LostArk::Shared::PACKET_FRAME& frame);

private:
	SOCKET m_hServerSocket = INVALID_SOCKET;
	//main thread와 Receive worker가 오류 코드를 함께 읽고 쓰므로 atomic으로 보호한다.
	std::atomic<int> m_iLastErrorCode{ 0 };
	bool m_isWinSocketInitialized = false;

	std::thread m_ReceiveThread;

	std::atomic_bool m_isReceiveRunning{ false };

	LostArk::Shared::CPacketStreamParser m_StreamParser;

	std::mutex m_InboundMutex;
	std::deque<LostArk::Shared::PACKET_FRAME> m_InboundFrames;

	//Handle Frame과 소비자 모두 main thread이다.
	std::deque<Client::CLIENT_REPLICATION_EVENT> m_ReplicationEvents;

	bool m_hasPendingEnterAccepted = false;

	LostArk::Shared::S2C_ENTER_ACCEPTED m_PendingEnterAccepted{};

	LostArk::Shared::PLAYER_ID m_iLocalPlayerId = LostArk::Shared::INVALID_PLAYER_ID;

	LostArk::Shared::NET_ENTITY_ID m_iLocalNetEntityId =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	LostArk::Shared::WORLD_ID m_eWorldId =
		LostArk::Shared::WORLD_ID::END;
	LostArk::Shared::CHARACTER_CLASS_ID m_eLocalCharacterClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;

};

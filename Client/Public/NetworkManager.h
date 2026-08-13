#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>

#include "ClientReplicationEvent.h"

#include "Network/PacketFrame.h"
#include "Network/PacketMessages.h"
#include "Network/PacketStreamParser.h"

//race�� �����ϱ� ���ؼ� atomic header�� �߰�
#include <atomic>
#include <deque>
//���� ���� race�� ���� ���ؼ� mutex ���� �� ���
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
	static constexpr std::uint16_t DEFAULT_SERVER_PORT = 7777;
	static std::string Resolve_ServerHost();
	static std::string Resolve_MapEditorServerHost();

	bool Initialize();
	void Shutdown();
	void Update();

	bool Connect_To_Server(
		std::string_view host,
		std::uint16_t port);
	bool Connect_To_Server(std::uint16_t port)
	{
		return Connect_To_Server("192.168.200.103", port);
	}

	bool Send_EnterWorld(
		LostArk::Shared::WORLD_ID worldId,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass,
		std::string_view nickName);
	//playercontroller�� ��ǥ XZ�� �����ϴ� public ���
	bool Send_MoveGoal(
		std::uint32_t clientSequence,
		float goalX,
		float goalZ);
	bool Send_UseSkill(
		std::uint32_t clientSequence,
		LostArk::Shared::SKILL_ID skillId,
		float aimX,
		float aimZ);
	bool Send_ReleaseSkill(
		std::uint32_t clientSequence,
		LostArk::Shared::SKILL_ID skillId);
	bool Send_SkillAim(
		std::uint32_t clientSequence,
		LostArk::Shared::SKILL_ID skillId,
		float aimX,
		float aimZ);
	bool Send_RevivePlayer(std::uint32_t clientSequence);
	bool Send_ChangeCharacterClass(
		std::uint32_t clientSequence,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
	bool Send_SpawnWorldEntity(std::string_view placementId);

	bool Try_Consume_EnterAccepted(
		LostArk::Shared::S2C_ENTER_ACCEPTED& message);
	bool Try_Consume_EnterRejected(
		LostArk::Shared::S2C_ENTER_REJECTED& message);
	bool Try_Consume_WorldEntitySpawnResult(
		LostArk::Shared::S2C_WORLD_ENTITY_SPAWN_RESULT& message);
	bool Try_Consume_CharacterClassChangeResult(
		LostArk::Shared::S2C_CHARACTER_CLASS_CHANGE_RESULT& message);

	bool Try_Consume_ReplicationEvent(
		Client::CLIENT_REPLICATION_EVENT& event);

	void Close_ServerConnection();

	[[nodiscard]] bool Is_Connected() const;
	[[nodiscard]] int Get_LastErrorCode() const;
	[[nodiscard]] LostArk::Shared::PLAYER_ID Get_LocalPlayerId() const;
	[[nodiscard]] LostArk::Shared::NET_ENTITY_ID Get_LocalEntityId() const;
	[[nodiscard]] LostArk::Shared::CHARACTER_CLASS_ID
		Get_LocalCharacterClass() const;
	[[nodiscard]] bool Try_Get_LocalSpawn(
		LostArk::Shared::S2C_PLAYER_SPAWNED& outSpawn) const;


private:
	bool Send_All(std::span<const std::uint8_t> bytes);
	//���� worker �ϳ��� 4096-byte ���� ���۷� Server�� TCP byte stream�� �д´�.
	void Receive_Loop(SOCKET serverSocket);
	void Handle_Frame(const LostArk::Shared::PACKET_FRAME& frame);

private:
	SOCKET m_hServerSocket = INVALID_SOCKET;
	//main thread�� Receive worker�� ���� �ڵ带 �Բ� �а� ���Ƿ� atomic���� ��ȣ�Ѵ�.
	std::atomic<int> m_iLastErrorCode{ 0 };
	bool m_isWinSocketInitialized = false;

	std::thread m_ReceiveThread;

	std::atomic_bool m_isReceiveRunning{ false };

	LostArk::Shared::CPacketStreamParser m_StreamParser;

	std::mutex m_InboundMutex;
	std::deque<LostArk::Shared::PACKET_FRAME> m_InboundFrames;

	//Handle Frame�� �Һ��� ��� main thread�̴�.
	std::deque<Client::CLIENT_REPLICATION_EVENT> m_ReplicationEvents;
	std::deque<LostArk::Shared::S2C_WORLD_ENTITY_SPAWN_RESULT>
		m_WorldEntitySpawnResults;
	std::deque<LostArk::Shared::S2C_CHARACTER_CLASS_CHANGE_RESULT>
		m_CharacterClassChangeResults;

	bool m_hasPendingEnterAccepted = false;

	LostArk::Shared::S2C_ENTER_ACCEPTED m_PendingEnterAccepted{};
	bool m_hasPendingEnterRejected = false;
	LostArk::Shared::S2C_ENTER_REJECTED m_PendingEnterRejected{};

	LostArk::Shared::PLAYER_ID m_iLocalPlayerId = LostArk::Shared::INVALID_PLAYER_ID;

	LostArk::Shared::NET_ENTITY_ID m_iLocalNetEntityId =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	LostArk::Shared::WORLD_ID m_eWorldId =
		LostArk::Shared::WORLD_ID::END;
	LostArk::Shared::CHARACTER_CLASS_ID m_eLocalCharacterClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	bool m_hasLocalSpawn = false;
	LostArk::Shared::S2C_PLAYER_SPAWNED m_LocalSpawn = {};

};

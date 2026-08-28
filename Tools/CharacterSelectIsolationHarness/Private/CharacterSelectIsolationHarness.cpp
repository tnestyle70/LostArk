#include "Network/PacketFrame.h"
#include "Network/PacketMessages.h"
#include "Network/PacketReader.h"
#include "Network/PacketStreamParser.h"
#include "Network/PacketWriter.h"

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

namespace
{
	using namespace LostArk::Shared;

	constexpr std::string_view CHARACTER_SELECT_MONSTER_GROUP =
		"spawn.character-select.monster";
	constexpr SKILL_ID CHARACTER_SELECT_DAMAGE_PROBE_SKILL_ID = 34110u;
	constexpr float DAMAGE_PROBE_HIT_REACH_METERS = 3.f;

	struct HARNESS_OPTIONS final
	{
		std::string strHost = "127.0.0.1";
		std::uint16_t iPort = 7777u;
		std::uint32_t iTimeoutMilliseconds = 4000u;
		std::uint32_t iBernPartySize = 0u;
		bool isG02IdentityFast = false;
	};

	bool Parse_Unsigned(
		const std::string_view text,
		std::uint32_t& value)
	{
		std::uint32_t parsed = 0;
		const auto result = std::from_chars(
			text.data(), text.data() + text.size(), parsed);
		if (result.ec != std::errc{} ||
			result.ptr != text.data() + text.size())
		{
			return false;
		}
		value = parsed;
		return true;
	}

	bool Parse_Options(
		const int argumentCount,
		char** arguments,
		HARNESS_OPTIONS& options,
		std::string& error)
	{
		bool hasHost = false;
		bool hasPort = false;
		bool hasTimeout = false;
		bool hasG02IdentityFast = false;
		bool hasBernPartySize = false;
		for (int index = 1; index < argumentCount; ++index)
		{
			const std::string_view argument(arguments[index]);
			if ("--host" == argument && !hasHost && index + 1 < argumentCount)
			{
				options.strHost = arguments[++index];
				hasHost = true;
				continue;
			}
			if ("--port" == argument && !hasPort && index + 1 < argumentCount)
			{
				std::uint32_t port = 0;
				if (!Parse_Unsigned(arguments[++index], port) ||
					0u == port || port > 65535u)
				{
					error = "--port must be an integer from 1 to 65535";
					return false;
				}
				options.iPort = static_cast<std::uint16_t>(port);
				hasPort = true;
				continue;
			}
			if ("--timeout-ms" == argument && !hasTimeout &&
				index + 1 < argumentCount)
			{
				std::uint32_t timeout = 0;
				if (!Parse_Unsigned(arguments[++index], timeout) ||
					timeout < 1000u || timeout > 4000u)
				{
					error = "--timeout-ms must be an integer from 1000 to 4000";
					return false;
				}
				options.iTimeoutMilliseconds = timeout;
				hasTimeout = true;
				continue;
			}
			if ("--g02-identity-fast" == argument &&
				!hasG02IdentityFast)
			{
				options.isG02IdentityFast = true;
				hasG02IdentityFast = true;
				continue;
			}
			if ("--bern-party-size" == argument && !hasBernPartySize &&
				index + 1 < argumentCount)
			{
				std::uint32_t partySize = 0u;
				if (!Parse_Unsigned(arguments[++index], partySize) ||
					(partySize != 2u && partySize != 4u))
				{
					error = "--bern-party-size must be 2 or 4";
					return false;
				}
				options.iBernPartySize = partySize;
				hasBernPartySize = true;
				continue;
			}

			error = "Usage: CharacterSelectIsolationHarness [--host IPv4] "
				"[--port 1..65535] [--timeout-ms 1000..4000] "
				"[--g02-identity-fast | --bern-party-size 2|4]";
			return false;
		}
		if (hasG02IdentityFast && hasBernPartySize)
		{
			error = "--g02-identity-fast and --bern-party-size cannot be combined";
			return false;
		}

		IN_ADDR address{};
		if (options.strHost.empty() ||
			1 != ::InetPtonA(AF_INET, options.strHost.c_str(), &address))
		{
			error = "--host must be one IPv4 address";
			return false;
		}
		return true;
	}

	class CWinsockScope final
	{
	public:
		CWinsockScope()
		{
			WSADATA data{};
			m_isReady = 0 == ::WSAStartup(MAKEWORD(2, 2), &data) &&
				LOBYTE(data.wVersion) == 2 && HIBYTE(data.wVersion) == 2;
		}

		~CWinsockScope()
		{
			if (m_isReady)
				::WSACleanup();
		}

		[[nodiscard]] bool Is_Ready() const
		{
			return m_isReady;
		}

	private:
		bool m_isReady = false;
	};

	class CTestClient final
	{
	public:
		explicit CTestClient(std::string label)
			: m_strLabel(std::move(label))
		{}

		~CTestClient()
		{
			Close();
		}

		CTestClient(const CTestClient&) = delete;
		CTestClient& operator=(const CTestClient&) = delete;

		bool Connect_AndEnter(
			const HARNESS_OPTIONS& options,
			const WORLD_ID worldId,
			const CHARACTER_CLASS_ID characterClass,
			std::string& error)
		{
			Close();
			m_eExpectedWorld = worldId;
			m_hSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (INVALID_SOCKET == m_hSocket)
				return Set_SocketError("socket", error);

			const DWORD socketTimeout = 2000u;
			::setsockopt(m_hSocket, SOL_SOCKET, SO_SNDTIMEO,
				reinterpret_cast<const char*>(&socketTimeout),
				static_cast<int>(sizeof(socketTimeout)));
			::setsockopt(m_hSocket, SOL_SOCKET, SO_RCVTIMEO,
				reinterpret_cast<const char*>(&socketTimeout),
				static_cast<int>(sizeof(socketTimeout)));

			sockaddr_in address{};
			address.sin_family = AF_INET;
			address.sin_port = ::htons(options.iPort);
			if (1 != ::InetPtonA(
				AF_INET, options.strHost.c_str(), &address.sin_addr))
			{
				error = m_strLabel + ": invalid IPv4 address";
				Close();
				return false;
			}
			if (SOCKET_ERROR == ::connect(
				m_hSocket,
				reinterpret_cast<const sockaddr*>(&address),
				static_cast<int>(sizeof(address))))
			{
				return Set_SocketError("connect", error);
			}

			C2S_ENTER_WORLD enter{};
			enter.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
			enter.eWorldId = worldId;
			enter.eCharacterClass = characterClass;
			enter.strNickName = m_strLabel;
			if (!Send_Message(PACKET_TYPE::C2S_ENTER_WORLD, enter, error))
			{
				Close();
				return false;
			}
			return true;
		}

		bool Send_SpawnRequest(
			const std::string_view placementId,
			std::string& error)
		{
			C2S_SPAWN_WORLD_ENTITY request{};
			request.strPlacementId = placementId;
			return Send_Message(
				PACKET_TYPE::C2S_SPAWN_WORLD_ENTITY, request, error);
		}

		bool Send_UseSkill(
			const std::uint32_t clientSequence,
			const SKILL_ID skillId,
			const float aimX,
			const float aimZ,
			std::string& error)
		{
			C2S_USE_SKILL request{};
			request.iClientSequence = clientSequence;
			request.iSkillId = skillId;
			request.fAimX = aimX;
			request.fAimZ = aimZ;
			return Send_Message(PACKET_TYPE::C2S_USE_SKILL, request, error);
		}

		bool Send_Move(
			const std::uint32_t clientSequence,
			const float goalX,
			const float goalZ,
			std::string& error)
		{
			C2S_MOVE request{};
			request.iClientSequence = clientSequence;
			request.fGoalX = goalX;
			request.fGoalZ = goalZ;
			return Send_Message(PACKET_TYPE::C2S_MOVE, request, error);
		}

		bool Send_ConfirmNpcEntry(
			const std::uint32_t requestSequence,
			const std::string_view npcPlacementId,
			std::string& error)
		{
			C2S_CONFIRM_NPC_ENTRY request{};
			request.iRequestSequence = requestSequence;
			request.strNpcPlacementId = std::string(npcPlacementId);
			return Send_Message(
				PACKET_TYPE::C2S_CONFIRM_NPC_ENTRY, request, error);
		}

		bool Send_PartyInvite(
			const std::uint32_t requestSequence,
			const NET_ENTITY_ID targetEntityId,
			std::string& error)
		{
			C2S_PARTY_INVITE request{};
			request.iRequestSequence = requestSequence;
			request.iTargetNetEntityId = targetEntityId;
			return Send_Message(PACKET_TYPE::C2S_PARTY_INVITE, request, error);
		}

		bool Accept_PartyInvite(
			const NET_ENTITY_ID inviterEntityId,
			std::string& error)
		{
			C2S_PARTY_INVITE_RESPOND response{};
			response.iRequestSequence = 1u;
			response.iFromNetEntityId = inviterEntityId;
			response.bAccepted = true;
			return Send_Message(PACKET_TYPE::C2S_PARTY_INVITE_RESPOND, response, error);
		}

		[[nodiscard]] bool Has_PartyInviteFrom(const NET_ENTITY_ID entityId) const
		{
			return entityId == m_iPendingPartyInviter;
		}

		[[nodiscard]] bool Has_PartyTransferFailure() const
		{
			return m_hasPartyTransferFailure;
		}

		[[nodiscard]] bool Has_ExactPartyRoster(
			const std::vector<NET_ENTITY_ID>& expected) const
		{
			if (m_PartyRoster.Members.size() != expected.size())
				return false;
			for (std::size_t index = 0u; index < expected.size(); ++index)
			{
				if (m_PartyRoster.Members[index].iNetEntityId != expected[index] ||
					!Has_ExactSpawnNickname(expected[index],
						m_PartyRoster.Members[index].strNickname))
				{
					return false;
				}
			}
			return true;
		}

		bool Poll(std::string& error)
		{
			if (INVALID_SOCKET == m_hSocket)
			{
				error = m_strLabel + ": socket is not open";
				return false;
			}

			fd_set readable{};
			FD_ZERO(&readable);
			FD_SET(m_hSocket, &readable);
			timeval noWait{};
			const int selected = ::select(0, &readable, nullptr, nullptr, &noWait);
			if (SOCKET_ERROR == selected)
				return Set_SocketError("select", error);
			if (0 == selected)
				return true;

			std::array<std::uint8_t, 8192> receiveBuffer{};
			const int received = ::recv(
				m_hSocket,
				reinterpret_cast<char*>(receiveBuffer.data()),
				static_cast<int>(receiveBuffer.size()),
				0);
			if (0 == received)
			{
				error = m_strLabel + ": server closed the connection";
				return false;
			}
			if (SOCKET_ERROR == received)
				return Set_SocketError("recv", error);

			if (!m_StreamParser.Append(std::span<const std::uint8_t>(
				receiveBuffer.data(), static_cast<std::size_t>(received))))
			{
				error = m_strLabel + ": receive buffer exceeded protocol bound";
				return false;
			}
			for (;;)
			{
				PACKET_FRAME frame{};
				const PACKET_PARSE_RESULT result = m_StreamParser.Try_Pop(frame);
				if (PACKET_PARSE_RESULT::NEED_MORE_DATA == result)
					break;
				if (PACKET_PARSE_RESULT::INVALID_FRAME == result)
				{
					error = m_strLabel + ": received an invalid packet frame";
					return false;
				}
				if (!Handle_Frame(frame, error))
					return false;
			}
			return true;
		}

		void Close()
		{
			if (INVALID_SOCKET == m_hSocket)
				return;
			::shutdown(m_hSocket, SD_BOTH);
			::closesocket(m_hSocket);
			m_hSocket = INVALID_SOCKET;
		}

		[[nodiscard]] bool Is_Accepted() const
		{
			return m_hasEnterAccepted && !m_hasEnterRejected;
		}

		[[nodiscard]] bool Is_Rejected() const
		{
			return m_hasEnterRejected;
		}

		[[nodiscard]] WORLD_ID Get_CurrentWorld() const
		{
			return m_eExpectedWorld;
		}

		[[nodiscard]] std::size_t Get_AcceptanceCount() const
		{
			return m_iAcceptanceCount;
		}

		[[nodiscard]] NET_ENTITY_ID Get_NetEntityId() const
		{
			return m_EnterAccepted.iNetEntityId;
		}

		[[nodiscard]] bool Has_ExactSpawnNickname(
			const NET_ENTITY_ID entityId,
			const std::string_view expected) const
		{
			const auto iter = m_ObservedSpawnedPlayerNicknames.find(entityId);
			return iter != m_ObservedSpawnedPlayerNicknames.end() &&
				std::string_view(iter->second) == expected;
		}

		[[nodiscard]] std::uint32_t Get_ServerTick() const
		{
			return m_iLastServerTick;
		}

		[[nodiscard]] float Get_OwnPositionX() const
		{
			return m_fOwnPositionX;
		}

		[[nodiscard]] float Get_OwnPositionZ() const
		{
			return m_fOwnPositionZ;
		}

		[[nodiscard]] std::size_t Get_DamageEventCount() const
		{
			return m_iDamageEventCount;
		}

		[[nodiscard]] bool Has_OutgoingDamageTo(
			const NET_ENTITY_ID entityId) const
		{
			return m_OutgoingDamageTargetIds.contains(entityId);
		}

		[[nodiscard]] bool Has_OwnOnlyPlayerSnapshot() const
		{
			return Is_Accepted() && m_hasWorldSnapshot &&
				1u == m_LatestPlayerEntityIds.size() &&
				m_LatestPlayerEntityIds.contains(Get_NetEntityId());
		}

		[[nodiscard]] bool Has_ExactPlayerSnapshot(
			const std::set<NET_ENTITY_ID>& expected) const
		{
			return Is_Accepted() && m_hasWorldSnapshot &&
				m_LatestPlayerEntityIds == expected;
		}

		[[nodiscard]] bool Has_PrivatePlayerLeak() const
		{
			const bool observedForeignSpawn = std::any_of(
				m_ObservedSpawnedPlayerEntityIds.begin(),
				m_ObservedSpawnedPlayerEntityIds.end(),
				[this](const NET_ENTITY_ID entityId)
				{
					return entityId != Get_NetEntityId();
				});
			return observedForeignSpawn ||
				(m_hasWorldSnapshot && !Has_OwnOnlyPlayerSnapshot());
		}

		[[nodiscard]] bool Has_AnyWorldActivity() const
		{
			return !m_ObservedSpawnedWorldEntityIds.empty() ||
				!m_LatestWorldEntityIds.empty() ||
				!m_SpawnResults.empty() || 0u != m_iDamageEventCount;
		}

		[[nodiscard]] bool Has_SpawnResult(
			const std::string_view placementId,
			const WORLD_ENTITY_SPAWN_RESULT expected) const
		{
			const auto iter = m_SpawnResults.find(std::string(placementId));
			return iter != m_SpawnResults.end() && expected == iter->second;
		}

		[[nodiscard]] bool Has_AnyWorldEntity() const
		{
			return !m_LatestWorldEntityIds.empty();
		}

		[[nodiscard]] bool Has_WorldEntity(const NET_ENTITY_ID entityId) const
		{
			return m_LatestWorldEntityIds.contains(entityId);
		}

		[[nodiscard]] NET_ENTITY_ID Get_FirstWorldEntityId() const
		{
			return m_LatestWorldEntityIds.empty() ? INVALID_NET_ENTITY_ID :
				*m_LatestWorldEntityIds.begin();
		}

		[[nodiscard]] bool Get_WorldEntityPosition(
			const NET_ENTITY_ID entityId,
			float& outX,
			float& outZ) const
		{
			const auto iter = m_LatestWorldEntityPositions.find(entityId);
			if (iter == m_LatestWorldEntityPositions.end())
				return false;
			outX = iter->second.first;
			outZ = iter->second.second;
			return true;
		}

		[[nodiscard]] bool Get_WorldEntityPositionByPlacement(
			const std::string_view placementId,
			float& outX,
			float& outZ) const
		{
			const auto entity = m_WorldEntityIdByPlacement.find(
				std::string(placementId));
			return entity != m_WorldEntityIdByPlacement.end() &&
				Get_WorldEntityPosition(entity->second, outX, outZ);
		}

	private:
		template<typename MESSAGE>
		bool Send_Message(
			const PACKET_TYPE packetType,
			const MESSAGE& message,
			std::string& error)
		{
			CPacketWriter payloadWriter;
			std::vector<std::uint8_t> frameBytes;
			if (!Write_Message(payloadWriter, message) ||
				!Build_Packet_Frame(
					packetType, payloadWriter.Get_Buffer(), frameBytes))
			{
				error = m_strLabel + ": failed to encode packet";
				return false;
			}
			return Send_All(frameBytes, error);
		}

		bool Send_All(
			const std::span<const std::uint8_t> bytes,
			std::string& error)
		{
			std::size_t sent = 0;
			while (sent < bytes.size())
			{
				const int result = ::send(
					m_hSocket,
					reinterpret_cast<const char*>(bytes.data() + sent),
					static_cast<int>(bytes.size() - sent),
					0);
				if (SOCKET_ERROR == result)
					return Set_SocketError("send", error);
				if (0 == result)
				{
					error = m_strLabel + ": send returned zero bytes";
					return false;
				}
				sent += static_cast<std::size_t>(result);
			}
			return true;
		}

		bool Handle_Frame(
			const PACKET_FRAME& frame,
			std::string& error)
		{
			CPacketReader reader{ frame.Payload };
			if (PACKET_TYPE::S2C_ENTER_ACCEPTED == frame.ePacketType)
			{
				S2C_ENTER_ACCEPTED accepted{};
				if (!Read_Message(reader, accepted) ||
					0u != reader.Get_RemainingSize() ||
					(!m_hasEnterAccepted &&
						m_eExpectedWorld != accepted.eWorldId) ||
					(m_hasEnterAccepted &&
						!(WORLD_ID::BERN == m_eExpectedWorld &&
							WORLD_ID::VALTAN_ARENA == accepted.eWorldId)))
				{
					error = m_strLabel + ": invalid or duplicate enter acceptance";
					return false;
				}
				m_EnterAccepted = accepted;
				m_eExpectedWorld = accepted.eWorldId;
				m_LatestPlayerEntityIds.clear();
				m_LatestWorldEntityIds.clear();
				m_ObservedSpawnedPlayerEntityIds.clear();
				m_ObservedSpawnedPlayerNicknames.clear();
				m_ObservedSpawnedWorldEntityIds.clear();
				m_WorldEntityIdByPlacement.clear();
				m_OutgoingDamageTargetIds.clear();
				m_PartyRoster.Members.clear();
				m_iPendingPartyInviter = INVALID_NET_ENTITY_ID;
				m_hasPartyTransferFailure = false;
				m_iLastServerTick = 0;
				m_hasWorldSnapshot = false;
				m_hasEnterAccepted = true;
				++m_iAcceptanceCount;
				return true;
			}
			if (PACKET_TYPE::S2C_ENTER_REJECTED == frame.ePacketType)
			{
				S2C_ENTER_REJECTED rejected{};
				if (m_hasEnterRejected || !Read_Message(reader, rejected) ||
					0u != reader.Get_RemainingSize())
				{
					error = m_strLabel + ": invalid or duplicate enter rejection";
					return false;
				}
				m_hasEnterRejected = true;
				return true;
			}
			if (PACKET_TYPE::S2C_PLAYER_SPAWNED == frame.ePacketType)
			{
				S2C_PLAYER_SPAWNED spawned{};
				if (!Read_Message(reader, spawned) ||
					0u != reader.Get_RemainingSize())
				{
					error = m_strLabel + ": invalid player-spawn packet";
					return false;
				}
				m_ObservedSpawnedPlayerEntityIds.insert(spawned.iNetEntityId);
				m_ObservedSpawnedPlayerNicknames.insert_or_assign(
					spawned.iNetEntityId, spawned.strNickName);
				return true;
			}
			if (PACKET_TYPE::S2C_WORLD_ENTITY_SPAWNED == frame.ePacketType)
			{
				S2C_WORLD_ENTITY_SPAWNED spawned{};
				if (!Read_Message(reader, spawned) ||
					0u != reader.Get_RemainingSize())
				{
					error = m_strLabel + ": invalid world-entity spawn packet";
					return false;
				}
				m_ObservedSpawnedWorldEntityIds.insert(spawned.iNetEntityId);
				if (!spawned.strPlacementId.empty())
				{
					m_WorldEntityIdByPlacement.insert_or_assign(
						spawned.strPlacementId, spawned.iNetEntityId);
				}
				return true;
			}
			if (PACKET_TYPE::S2C_WORLD_ENTITY_SPAWN_RESULT == frame.ePacketType)
			{
				S2C_WORLD_ENTITY_SPAWN_RESULT result{};
				if (!Read_Message(reader, result) ||
					0u != reader.Get_RemainingSize())
				{
					error = m_strLabel + ": invalid world-entity spawn result";
					return false;
				}
				m_SpawnResults.insert_or_assign(
					result.strPlacementId, result.eResult);
				return true;
			}
			if (PACKET_TYPE::S2C_WORLD_SNAPSHOT == frame.ePacketType)
			{
				S2C_WORLD_SNAPSHOT snapshot{};
				if (!Read_Message(reader, snapshot) ||
					0u != reader.Get_RemainingSize() ||
					m_eExpectedWorld != snapshot.eWorldId)
				{
					error = m_strLabel + ": invalid world snapshot";
					return false;
				}
				m_iLastServerTick = snapshot.iServerTick;
				m_LatestPlayerEntityIds.clear();
				for (const PLAYER_SNAPSHOT& player : snapshot.Players)
				{
					m_LatestPlayerEntityIds.insert(player.iNetEntityId);
					if (player.iNetEntityId == Get_NetEntityId())
					{
						m_fOwnPositionX = player.fPositionX;
						m_fOwnPositionZ = player.fPositionZ;
					}
				}
				m_LatestWorldEntityIds.clear();
				m_LatestWorldEntityPositions.clear();
				for (const WORLD_ENTITY_SNAPSHOT& entity : snapshot.Entities)
				{
					m_LatestWorldEntityIds.insert(entity.iNetEntityId);
					m_LatestWorldEntityPositions.insert_or_assign(
						entity.iNetEntityId,
						std::pair{ entity.fPositionX, entity.fPositionZ });
				}
				m_iDamageEventCount += snapshot.DamageEvents.size();
				for (const DAMAGE_EVENT& damage : snapshot.DamageEvents)
				{
					if (damage.isOutgoing)
						m_OutgoingDamageTargetIds.insert(
							damage.iTargetNetEntityId);
				}
				m_hasWorldSnapshot = true;
				return true;
			}
			if (PACKET_TYPE::S2C_PARTY_INVITE_RECEIVED == frame.ePacketType)
			{
				S2C_PARTY_INVITE_RECEIVED invite{};
				if (!Read_Message(reader, invite) || 0u != reader.Get_RemainingSize())
				{
					error = m_strLabel + ": invalid party invite";
					return false;
				}
				m_iPendingPartyInviter = invite.iFromNetEntityId;
				return true;
			}
			if (PACKET_TYPE::S2C_PARTY_ROSTER == frame.ePacketType)
			{
				S2C_PARTY_ROSTER roster{};
				if (!Read_Message(reader, roster) || 0u != reader.Get_RemainingSize())
				{
					error = m_strLabel + ": invalid party roster";
					return false;
				}
				m_PartyRoster = std::move(roster);
				return true;
			}
			if (PACKET_TYPE::S2C_PARTY_TRANSFER_RESULT == frame.ePacketType)
			{
				S2C_PARTY_TRANSFER_RESULT result{};
				if (!Read_Message(reader, result) || 0u != reader.Get_RemainingSize())
				{
					error = m_strLabel + ": invalid party transfer result";
					return false;
				}
				m_hasPartyTransferFailure = true;
				return true;
			}

			// Despawn, boss and class-change packets are legal but do not carry an
			// isolation assertion for this harness.
			return true;
		}

		bool Set_SocketError(
			const char* operation,
			std::string& error)
		{
			const int code = ::WSAGetLastError();
			error = m_strLabel + ": " + operation + " failed, WSA=" +
				std::to_string(code);
			Close();
			return false;
		}

	private:
		std::string m_strLabel;
		SOCKET m_hSocket = INVALID_SOCKET;
		WORLD_ID m_eExpectedWorld = WORLD_ID::END;
		CPacketStreamParser m_StreamParser;
		S2C_ENTER_ACCEPTED m_EnterAccepted{};
		S2C_PARTY_ROSTER m_PartyRoster{};
		NET_ENTITY_ID m_iPendingPartyInviter = INVALID_NET_ENTITY_ID;
		bool m_hasPartyTransferFailure = false;
		std::set<NET_ENTITY_ID> m_LatestPlayerEntityIds;
		std::set<NET_ENTITY_ID> m_LatestWorldEntityIds;
		std::map<NET_ENTITY_ID, std::pair<float, float>>
			m_LatestWorldEntityPositions;
		std::set<NET_ENTITY_ID> m_ObservedSpawnedPlayerEntityIds;
		std::map<NET_ENTITY_ID, std::string>
			m_ObservedSpawnedPlayerNicknames;
		std::set<NET_ENTITY_ID> m_ObservedSpawnedWorldEntityIds;
		std::map<std::string, NET_ENTITY_ID> m_WorldEntityIdByPlacement;
		std::set<NET_ENTITY_ID> m_OutgoingDamageTargetIds;
		std::map<std::string, WORLD_ENTITY_SPAWN_RESULT> m_SpawnResults;
		std::uint32_t m_iLastServerTick = 0;
		float m_fOwnPositionX = 0.f;
		float m_fOwnPositionZ = 0.f;
		std::size_t m_iDamageEventCount = 0;
		std::size_t m_iAcceptanceCount = 0;
		bool m_hasEnterAccepted = false;
		bool m_hasEnterRejected = false;
		bool m_hasWorldSnapshot = false;
	};

	enum class PROBE_RESULT
	{
		WAIT,
		PASS,
		FAIL
	};

	template<typename PROBE>
	bool Pump_Until(
		const std::span<CTestClient*> clients,
		const std::chrono::milliseconds timeout,
		PROBE probe,
		const std::string_view stage,
		std::string& error)
	{
		const auto deadline = std::chrono::steady_clock::now() + timeout;
		while (std::chrono::steady_clock::now() < deadline)
		{
			for (CTestClient* client : clients)
			{
				if (nullptr != client && !client->Poll(error))
				{
					error = std::string(stage) + ": " + error;
					return false;
				}
			}

			const PROBE_RESULT result = probe(error);
			if (PROBE_RESULT::PASS == result)
				return true;
			if (PROBE_RESULT::FAIL == result)
			{
				error = std::string(stage) + ": " + error;
				return false;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		}
		const std::string lastProbeDetail = error;
		error = std::string(stage) + ": timed out after " +
			std::to_string(timeout.count()) + " ms";
		if (!lastProbeDetail.empty())
			error += "; last probe: " + lastProbeDetail;
		return false;
	}

	std::unique_ptr<CTestClient> Connect_Client(
		const HARNESS_OPTIONS& options,
		const std::string& label,
		const WORLD_ID worldId,
		const CHARACTER_CLASS_ID characterClass,
		std::string& error)
	{
		auto client = std::make_unique<CTestClient>(label);
		if (!client->Connect_AndEnter(
			options, worldId, characterClass, error))
		{
			return {};
		}
		return client;
	}

	bool Check_PrivateClient(
		const CTestClient& client,
		const std::string_view label,
		std::string& error)
	{
		if (client.Is_Rejected())
		{
			error = std::string(label) + " was rejected";
			return false;
		}
		if (client.Has_PrivatePlayerLeak())
		{
			error = std::string(label) +
				" observed another Character Select player";
			return false;
		}
		return true;
	}

	bool Run_CharacterSelectIsolation(
		const HARNESS_OPTIONS& options,
		std::string& error)
	{
		const auto timeout =
			std::chrono::milliseconds(options.iTimeoutMilliseconds);
		auto clientA = Connect_Client(
			options,
			"CS-Isolation-A",
			WORLD_ID::CHARACTER_SELECT_ARENA,
			CHARACTER_CLASS_ID::LANCE_MASTER,
			error);
		if (nullptr == clientA)
			return false;
		auto clientB = Connect_Client(
			options,
			"CS-Isolation-B",
			WORLD_ID::CHARACTER_SELECT_ARENA,
			CHARACTER_CLASS_ID::GUNSLINGER,
			error);
		if (nullptr == clientB)
			return false;

		std::array privateClients{ clientA.get(), clientB.get() };
		if (!Pump_Until(privateClients, timeout,
			[&](std::string& probeError)
			{
				if (!Check_PrivateClient(*clientA, "client A", probeError) ||
					!Check_PrivateClient(*clientB, "client B", probeError))
				{
					return PROBE_RESULT::FAIL;
				}
				if (clientA->Has_AnyWorldActivity() ||
					clientB->Has_AnyWorldActivity())
				{
					probeError = "a fresh private arena was not empty";
					return PROBE_RESULT::FAIL;
				}
				return clientA->Has_OwnOnlyPlayerSnapshot() &&
					clientB->Has_OwnOnlyPlayerSnapshot() &&
					clientA->Has_ExactSpawnNickname(
						clientA->Get_NetEntityId(), "CS-Isolation-A") &&
					clientB->Has_ExactSpawnNickname(
						clientB->Get_NetEntityId(), "CS-Isolation-B") ?
					PROBE_RESULT::PASS : PROBE_RESULT::WAIT;
			},
			"simultaneous Character Select admission", error))
		{
			return false;
		}
		std::cout << "[PASS] simultaneous Character Select clients received "
			"one-player private snapshots\n";

		if (!clientA->Send_SpawnRequest(
			CHARACTER_SELECT_MONSTER_GROUP, error))
		{
			return false;
		}
		if (!Pump_Until(privateClients, timeout,
			[&](std::string& probeError)
			{
				if (!Check_PrivateClient(*clientA, "client A", probeError) ||
					!Check_PrivateClient(*clientB, "client B", probeError))
				{
					return PROBE_RESULT::FAIL;
				}
				if (clientB->Has_AnyWorldActivity())
				{
					probeError = "client B observed client A's entity or damage";
					return PROBE_RESULT::FAIL;
				}
				if (clientA->Has_SpawnResult(
					CHARACTER_SELECT_MONSTER_GROUP,
					WORLD_ENTITY_SPAWN_RESULT::REJECTED))
				{
					probeError = "client A's monster spawn was rejected";
					return PROBE_RESULT::FAIL;
				}
				return clientA->Has_SpawnResult(
						CHARACTER_SELECT_MONSTER_GROUP,
						WORLD_ENTITY_SPAWN_RESULT::ACTIVATED) &&
					clientA->Has_AnyWorldEntity() ?
					PROBE_RESULT::PASS : PROBE_RESULT::WAIT;
			},
			"client A entity isolation", error))
		{
			return false;
		}

		const NET_ENTITY_ID clientAMonsterId =
			clientA->Get_FirstWorldEntityId();
		float monsterX = 0.f;
		float monsterZ = 0.f;
		if (INVALID_NET_ENTITY_ID == clientAMonsterId ||
			!clientA->Get_WorldEntityPosition(
				clientAMonsterId, monsterX, monsterZ))
		{
			error = "client A did not expose a monster entity ID";
			return false;
		}
		if (!clientA->Send_Move(1u, monsterX, monsterZ, error))
			return false;
		if (!Pump_Until(privateClients, timeout,
			[&](std::string& probeError)
			{
				if (!Check_PrivateClient(*clientA, "client A", probeError) ||
					!Check_PrivateClient(*clientB, "client B", probeError))
				{
					return PROBE_RESULT::FAIL;
				}
				if (clientB->Has_AnyWorldActivity())
				{
					probeError = "client B observed client A's entity or damage";
					return PROBE_RESULT::FAIL;
				}
				/* The monster now owns a real chase path and can leave its spawn
				coordinate while the player approaches. Keep the isolation proof
				bound to the replicated entity instead of waiting at a stale point. */
				if (!clientA->Get_WorldEntityPosition(
						clientAMonsterId, monsterX, monsterZ))
				{
					probeError = "client A lost the moving monster snapshot";
					return PROBE_RESULT::FAIL;
				}
				const float deltaX = clientA->Get_OwnPositionX() - monsterX;
				const float deltaZ = clientA->Get_OwnPositionZ() - monsterZ;
				const float distanceSquared = deltaX * deltaX + deltaZ * deltaZ;
				if (distanceSquared <= DAMAGE_PROBE_HIT_REACH_METERS *
					DAMAGE_PROBE_HIT_REACH_METERS)
				{
					return PROBE_RESULT::PASS;
				}
				probeError = "distance to moving monster is " +
					std::to_string(std::sqrt(distanceSquared)) + " m";
				return PROBE_RESULT::WAIT;
			},
			"client A approach into damage-probe hit reach", error))
		{
			return false;
		}
		if (!clientA->Send_UseSkill(
			1u, CHARACTER_SELECT_DAMAGE_PROBE_SKILL_ID,
			monsterX, monsterZ, error))
		{
			return false;
		}
		error.clear();
		std::uint32_t clientADamageTick = 0;
		if (!Pump_Until(privateClients, timeout,
			[&](std::string& probeError)
			{
				if (!Check_PrivateClient(*clientA, "client A", probeError) ||
					!Check_PrivateClient(*clientB, "client B", probeError))
				{
					return PROBE_RESULT::FAIL;
				}
				if (clientB->Has_AnyWorldActivity())
				{
					probeError = "client B observed client A's entity or damage";
					return PROBE_RESULT::FAIL;
				}
				if (!clientA->Has_OutgoingDamageTo(clientAMonsterId))
					return PROBE_RESULT::WAIT;
				if (0u == clientADamageTick)
					clientADamageTick = clientA->Get_ServerTick();
				return clientA->Get_ServerTick() > clientADamageTick + 10u ?
					PROBE_RESULT::PASS : PROBE_RESULT::WAIT;
			},
			"client A server-authoritative skill damage isolation", error))
		{
			return false;
		}
		std::cout << "[PASS] client A's spawned monster and damage stayed in "
			"client A's private arena\n";

		if (!clientB->Send_SpawnRequest(
			CHARACTER_SELECT_MONSTER_GROUP, error))
		{
			return false;
		}
		if (!Pump_Until(privateClients, timeout,
			[&](std::string& probeError)
			{
				if (!Check_PrivateClient(*clientA, "client A", probeError) ||
					!Check_PrivateClient(*clientB, "client B", probeError))
				{
					return PROBE_RESULT::FAIL;
				}
				if (clientB->Has_SpawnResult(
					CHARACTER_SELECT_MONSTER_GROUP,
					WORLD_ENTITY_SPAWN_RESULT::REJECTED))
				{
					probeError = "client B's monster spawn was rejected";
					return PROBE_RESULT::FAIL;
				}
				return clientB->Has_SpawnResult(
						CHARACTER_SELECT_MONSTER_GROUP,
						WORLD_ENTITY_SPAWN_RESULT::ACTIVATED) &&
					clientB->Has_AnyWorldEntity() ?
					PROBE_RESULT::PASS : PROBE_RESULT::WAIT;
			},
			"client B private entity setup", error))
		{
			return false;
		}

		const NET_ENTITY_ID retainedEntityId =
			clientB->Get_FirstWorldEntityId();
		const std::uint32_t retainedArenaTick = clientB->Get_ServerTick();
		if (INVALID_NET_ENTITY_ID == retainedEntityId)
		{
			error = "client B did not expose a retained entity ID";
			return false;
		}

		clientA->Close();
		clientA.reset();
		auto reenteredA = Connect_Client(
			options,
			"CS-Isolation-A",
			WORLD_ID::CHARACTER_SELECT_ARENA,
			CHARACTER_CLASS_ID::LANCE_MASTER,
			error);
		if (nullptr == reenteredA)
			return false;
		std::array reentryClients{ clientB.get(), reenteredA.get() };
		if (!Pump_Until(reentryClients, timeout,
			[&](std::string& probeError)
			{
				if (!Check_PrivateClient(*clientB, "client B", probeError) ||
					!Check_PrivateClient(*reenteredA,
						"re-entered client A", probeError))
				{
					return PROBE_RESULT::FAIL;
				}
				if (reenteredA->Has_AnyWorldActivity())
				{
					probeError = "re-entered client A observed client B's arena";
					return PROBE_RESULT::FAIL;
				}
				if (!clientB->Has_WorldEntity(retainedEntityId))
				{
					probeError = "client B's entity disappeared during client A re-entry";
					return PROBE_RESULT::FAIL;
				}
				const bool bContinued =
					clientB->Get_ServerTick() > retainedArenaTick + 10u;
				return reenteredA->Has_OwnOnlyPlayerSnapshot() && bContinued ?
					PROBE_RESULT::PASS : PROBE_RESULT::WAIT;
			},
			"disconnect and Character Select re-entry", error))
		{
			return false;
		}
		std::cout << "[PASS] client A re-entry did not reset or observe "
			"client B's live private arena\n";

		clientB->Close();
		reenteredA->Close();
		return true;
	}

	bool Run_BernSharedProof(
		const HARNESS_OPTIONS& options,
		std::string& error)
	{
		const auto timeout =
			std::chrono::milliseconds(options.iTimeoutMilliseconds);
		auto first = Connect_Client(
			options,
			"Bern-Shared-A",
			WORLD_ID::BERN,
			CHARACTER_CLASS_ID::ARTIST,
			error);
		if (nullptr == first)
			return false;
		auto second = Connect_Client(
			options,
			"Bern-Shared-B",
			WORLD_ID::BERN,
			CHARACTER_CLASS_ID::WARLORD,
			error);
		if (nullptr == second)
			return false;

		std::array clients{ first.get(), second.get() };
		if (!Pump_Until(clients, timeout,
			[&](std::string& probeError)
			{
				if (first->Is_Rejected() || second->Is_Rejected())
				{
					probeError = "a Bern client was rejected";
					return PROBE_RESULT::FAIL;
				}
				if (!first->Is_Accepted() || !second->Is_Accepted())
					return PROBE_RESULT::WAIT;
				const std::set<NET_ENTITY_ID> expected
				{
					first->Get_NetEntityId(), second->Get_NetEntityId()
				};
				if (2u != expected.size())
				{
					probeError = "Bern assigned duplicate player entity IDs";
					return PROBE_RESULT::FAIL;
				}
				return first->Has_ExactPlayerSnapshot(expected) &&
					second->Has_ExactPlayerSnapshot(expected) &&
					first->Has_ExactSpawnNickname(
						first->Get_NetEntityId(), "Bern-Shared-A") &&
					first->Has_ExactSpawnNickname(
						second->Get_NetEntityId(), "Bern-Shared-B") &&
					second->Has_ExactSpawnNickname(
						first->Get_NetEntityId(), "Bern-Shared-A") &&
					second->Has_ExactSpawnNickname(
						second->Get_NetEntityId(), "Bern-Shared-B") ?
					PROBE_RESULT::PASS : PROBE_RESULT::WAIT;
			},
			"Bern shared simulation", error))
		{
			return false;
		}

		std::cout << "[PASS] Bern clients converged on the same two-player "
			"shared snapshot\n";
		first->Close();
		second->Close();
		return true;
	}

	bool Run_BernToValtanTransferProof(
		const HARNESS_OPTIONS& options,
		std::string& error)
	{
		const auto timeout =
			std::chrono::milliseconds(options.iTimeoutMilliseconds);
		const auto transitionTimeout = timeout * 5;
		auto client = Connect_Client(
			options,
			"Bern-To-Valtan",
			WORLD_ID::BERN,
			CHARACTER_CLASS_ID::LANCE_MASTER,
			error);
		if (nullptr == client)
			return false;

		std::array clients{ client.get() };
		if (!Pump_Until(clients, timeout,
			[&](std::string& probeError)
			{
				if (client->Is_Rejected())
				{
					probeError = "Bern transfer client was rejected";
					return PROBE_RESULT::FAIL;
				}
				return client->Is_Accepted() &&
					client->Has_OwnOnlyPlayerSnapshot() &&
					client->Has_ExactSpawnNickname(
						client->Get_NetEntityId(), "Bern-To-Valtan") ?
					PROBE_RESULT::PASS : PROBE_RESULT::WAIT;
			},
			"Bern transfer admission", error))
		{
			return false;
		}
		constexpr std::string_view VALTAN_GUIDE_PLACEMENT =
			"npc.bern.beda.guide";
		float guideX = 0.f;
		float guideZ = 0.f;
		if (!client->Get_WorldEntityPositionByPlacement(
				VALTAN_GUIDE_PLACEMENT, guideX, guideZ))
		{
			error = "Bern transfer guide was not replicated";
			return false;
		}
		if (!client->Send_Move(1u, guideX, guideZ, error))
			return false;
		if (!Pump_Until(clients, transitionTimeout,
			[&](std::string& probeError)
			{
				if (!client->Get_WorldEntityPositionByPlacement(
						VALTAN_GUIDE_PLACEMENT, guideX, guideZ))
				{
					probeError = "Bern transfer guide disappeared";
					return PROBE_RESULT::FAIL;
				}
				const float deltaX = client->Get_OwnPositionX() - guideX;
				const float deltaZ = client->Get_OwnPositionZ() - guideZ;
				const float distanceSquared = deltaX * deltaX + deltaZ * deltaZ;
				if (distanceSquared <= 8.41f)
					return PROBE_RESULT::PASS;
				probeError = "player=(" +
					std::to_string(client->Get_OwnPositionX()) + "," +
					std::to_string(client->Get_OwnPositionZ()) +
					"), guide=(" + std::to_string(guideX) + "," +
					std::to_string(guideZ) + "), distance=" +
					std::to_string(std::sqrt(distanceSquared));
				return PROBE_RESULT::WAIT;
			},
			"Bern transfer guide approach", error))
		{
			return false;
		}
		if (!client->Send_ConfirmNpcEntry(
				2u, VALTAN_GUIDE_PLACEMENT, error))
		{
			return false;
		}

		if (!Pump_Until(clients, transitionTimeout,
			[&](std::string& probeError)
			{
				if (client->Is_Rejected())
				{
					probeError = "Bern-to-Valtan transfer was rejected";
					return PROBE_RESULT::FAIL;
				}
				const bool hasAcceptedTarget =
					2u == client->Get_AcceptanceCount() &&
					WORLD_ID::VALTAN_ARENA == client->Get_CurrentWorld() &&
					client->Has_OwnOnlyPlayerSnapshot();
				if (!hasAcceptedTarget)
				{
					probeError = "acceptances=" +
						std::to_string(client->Get_AcceptanceCount()) +
						", world=" +
						std::to_string(static_cast<unsigned>(
							client->Get_CurrentWorld())) +
						", position=(" +
						std::to_string(client->Get_OwnPositionX()) + "," +
						std::to_string(client->Get_OwnPositionZ()) + ")";
					return PROBE_RESULT::WAIT;
				}
				if (!client->Has_ExactSpawnNickname(
					client->Get_NetEntityId(), "Bern-To-Valtan"))
				{
					probeError =
						"Valtan spawn did not preserve the submitted nickname";
					return PROBE_RESULT::FAIL;
				}
				return PROBE_RESULT::PASS;
			},
			"Bern-to-Valtan accepted transition", error))
		{
			return false;
		}

		const std::uint32_t beforeCommandTick = client->Get_ServerTick();
		const float beforeCommandX = client->Get_OwnPositionX();
		const float beforeCommandZ = client->Get_OwnPositionZ();
		/* The current Valtan spawn is on the boss deck. The old (20, -30)
		approach waypoint is disconnected from that deck, so Find_Path correctly
		rejects it. Probe a one-metre move on the admitted deck instead. */
		if (!client->Send_Move(
				2u, beforeCommandX + 1.f, beforeCommandZ, error))
			return false;
		if (!Pump_Until(clients, timeout,
			[&](std::string& probeError)
			{
				if (client->Is_Rejected())
				{
					probeError = "post-transfer gameplay command closed the session";
					return PROBE_RESULT::FAIL;
				}
				const float deltaX =
					client->Get_OwnPositionX() - beforeCommandX;
				const float deltaZ =
					client->Get_OwnPositionZ() - beforeCommandZ;
				probeError = "before=(" + std::to_string(beforeCommandX) +
					"," + std::to_string(beforeCommandZ) + "), current=(" +
					std::to_string(client->Get_OwnPositionX()) + "," +
					std::to_string(client->Get_OwnPositionZ()) + "), tick=" +
					std::to_string(client->Get_ServerTick()) + ", initialTick=" +
					std::to_string(beforeCommandTick);
				return client->Get_ServerTick() > beforeCommandTick + 10u &&
					deltaX * deltaX + deltaZ * deltaZ > 0.01f &&
					client->Has_OwnOnlyPlayerSnapshot() ?
					PROBE_RESULT::PASS : PROBE_RESULT::WAIT;
			},
			"post-transfer Valtan gameplay command", error))
		{
			return false;
		}

		std::cout << "[PASS] Bern-to-Valtan transfer retained the live session "
			"for post-transfer gameplay commands\n";
		client->Close();
		return true;
	}

	bool Run_BernPartyTransferProof(
		const HARNESS_OPTIONS& options,
		const std::size_t memberCount,
		std::string& error)
	{
		const auto timeout =
			std::chrono::milliseconds(options.iTimeoutMilliseconds);
		const auto transitionTimeout = timeout * 5;
		const std::array classes{
			CHARACTER_CLASS_ID::LANCE_MASTER, CHARACTER_CLASS_ID::GUNSLINGER,
			CHARACTER_CLASS_ID::ARTIST, CHARACTER_CLASS_ID::WARLORD };
		if (memberCount < 2u || memberCount > classes.size())
			return false;
		std::vector<std::unique_ptr<CTestClient>> ownedClients;
		std::vector<CTestClient*> clients;
		for (std::size_t index = 0u; index < memberCount; ++index)
		{
			auto client = Connect_Client(options,
				"Party-" + std::to_string(memberCount) + "-" + std::to_string(index),
				WORLD_ID::BERN, classes[index], error);
			if (nullptr == client)
				return false;
			clients.push_back(client.get());
			ownedClients.push_back(std::move(client));
		}
		if (!Pump_Until(clients, timeout,
			[&](std::string& probeError)
			{
				std::set<NET_ENTITY_ID> expected;
				for (const CTestClient* client : clients)
				{
					if (client->Is_Rejected())
					{
						probeError = "party member Bern admission rejected";
						return PROBE_RESULT::FAIL;
					}
					if (!client->Is_Accepted())
						return PROBE_RESULT::WAIT;
					expected.insert(client->Get_NetEntityId());
				}
				return expected.size() == memberCount &&
					std::all_of(clients.begin(), clients.end(),
						[&](const CTestClient* client)
						{ return client->Has_ExactPlayerSnapshot(expected); }) ?
					PROBE_RESULT::PASS : PROBE_RESULT::WAIT;
			}, "party Bern admission", error))
		{
			return false;
		}

		CTestClient* leader = clients.front();
		std::vector<NET_ENTITY_ID> roster{ leader->Get_NetEntityId() };
		for (std::size_t index = 1u; index < memberCount; ++index)
		{
			if (!leader->Send_PartyInvite(static_cast<std::uint32_t>(index),
					clients[index]->Get_NetEntityId(), error) ||
				!Pump_Until(clients, timeout,
					[&](std::string&)
					{
						return clients[index]->Has_PartyInviteFrom(leader->Get_NetEntityId()) ?
							PROBE_RESULT::PASS : PROBE_RESULT::WAIT;
					}, "party invite delivery", error) ||
				!clients[index]->Accept_PartyInvite(leader->Get_NetEntityId(), error))
			{
				return false;
			}
			roster.push_back(clients[index]->Get_NetEntityId());
			if (!Pump_Until(clients, timeout,
				[&](std::string&)
				{
					for (std::size_t member = 0u; member <= index; ++member)
					{
						if (!clients[member]->Has_ExactPartyRoster(roster))
							return PROBE_RESULT::WAIT;
					}
					return PROBE_RESULT::PASS;
				}, "party roster after accept", error))
			{
				return false;
			}
		}
		constexpr std::string_view GUIDE = "npc.bern.beda.guide";
		float guideX = 0.f;
		float guideZ = 0.f;
		if (!leader->Get_WorldEntityPositionByPlacement(GUIDE, guideX, guideZ) ||
			!leader->Send_Move(1u, guideX, guideZ, error))
		{
			error = "party leader could not resolve/approach Bern guide: " + error;
			return false;
		}
		if (!Pump_Until(clients, transitionTimeout,
			[&](std::string&)
			{
				const float dx = leader->Get_OwnPositionX() - guideX;
				const float dz = leader->Get_OwnPositionZ() - guideZ;
				return dx * dx + dz * dz <= 8.41f ?
					PROBE_RESULT::PASS : PROBE_RESULT::WAIT;
			}, "party leader guide approach", error) ||
			!leader->Send_ConfirmNpcEntry(1u, GUIDE, error))
		{
			return false;
		}
		if (!Pump_Until(clients, transitionTimeout,
			[&](std::string& probeError)
			{
				std::vector<NET_ENTITY_ID> targetRoster;
				for (const CTestClient* client : clients)
				{
					if (client->Is_Rejected() || client->Has_PartyTransferFailure())
					{
						probeError = "party transfer rejected with available target capacity";
						return PROBE_RESULT::FAIL;
					}
					if (client->Get_AcceptanceCount() != 2u ||
						client->Get_CurrentWorld() != WORLD_ID::VALTAN_ARENA)
					{
						return PROBE_RESULT::WAIT;
					}
					targetRoster.push_back(client->Get_NetEntityId());
				}
				const std::set<NET_ENTITY_ID> expected(targetRoster.begin(), targetRoster.end());
				return expected.size() == memberCount &&
					std::all_of(clients.begin(), clients.end(),
						[&](const CTestClient* client)
						{
							return client->Has_ExactPlayerSnapshot(expected) &&
								client->Has_ExactPartyRoster(targetRoster);
						}) ? PROBE_RESULT::PASS : PROBE_RESULT::WAIT;
			}, "party Valtan roster and leader preservation", error))
		{
			return false;
		}

		std::vector<std::pair<float, float>> beforePositions;
		for (CTestClient* client : clients)
		{
			beforePositions.emplace_back(client->Get_OwnPositionX(), client->Get_OwnPositionZ());
			if (!client->Send_Move(2u, client->Get_OwnPositionX() + 1.f,
					client->Get_OwnPositionZ(), error))
			{
				return false;
			}
		}
		if (!Pump_Until(clients, timeout,
			[&](std::string& probeError)
			{
				for (std::size_t index = 0u; index < clients.size(); ++index)
				{
					const float dx = clients[index]->Get_OwnPositionX() - beforePositions[index].first;
					const float dz = clients[index]->Get_OwnPositionZ() - beforePositions[index].second;
					if (dx * dx + dz * dz <= 0.01f)
					{
						probeError = "party member " + std::to_string(index) + " has not moved";
						return PROBE_RESULT::WAIT;
					}
				}
				return PROBE_RESULT::PASS;
			}, "post-transfer party gameplay commands", error))
		{
			return false;
		}
		std::cout << "[PASS] " << memberCount << "-member Bern party retained exact "
			"roster/leader and live commands after Valtan transfer\n";
		return true;
	}
}

int main(const int argumentCount, char** arguments)
{
	std::cout.setf(std::ios_base::unitbuf);
	HARNESS_OPTIONS options{};
	std::string error;
	if (!Parse_Options(argumentCount, arguments, options, error))
	{
		std::cerr << "[FAILURE] " << error << '\n';
		return 2;
	}

	const CWinsockScope winsock;
	if (!winsock.Is_Ready())
	{
		std::cerr << "[FAILURE] WSAStartup 2.2 failed\n";
		return 1;
	}

	const bool passed = options.iBernPartySize != 0u ?
		Run_BernPartyTransferProof(options, options.iBernPartySize, error) :
		(Run_CharacterSelectIsolation(options, error) &&
			Run_BernSharedProof(options, error) &&
			(options.isG02IdentityFast || Run_BernToValtanTransferProof(options, error)));
	if (!passed)
	{
		std::cerr << "[FAILURE] " << error << '\n';
		return 1;
	}

	std::cout << "failures : 0\n";
	return 0;
}

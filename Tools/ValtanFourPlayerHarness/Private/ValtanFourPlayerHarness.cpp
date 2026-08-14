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
#include <cstdint>
#include <iostream>
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

	struct HARNESS_OPTIONS final
	{
		std::string strHost = "127.0.0.1";
		std::uint16_t iPort = 7777u;
		std::uint32_t iTimeoutMilliseconds = 10000u;
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
					timeout < 1000u || timeout > 30000u)
				{
					error = "--timeout-ms must be an integer from 1000 to 30000";
					return false;
				}
				options.iTimeoutMilliseconds = timeout;
				hasTimeout = true;
				continue;
			}

			error = "Usage: ValtanFourPlayerHarness [--host IPv4] "
				"[--port 1..65535] [--timeout-ms 1000..30000]";
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
			const CHARACTER_CLASS_ID characterClass,
			std::string& error)
		{
			Close();
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
			enter.eWorldId = WORLD_ID::VALTAN_ARENA;
			enter.eCharacterClass = characterClass;
			enter.strNickName = m_strLabel;
			CPacketWriter payloadWriter;
			std::vector<std::uint8_t> frameBytes;
			if (!Write_Message(payloadWriter, enter) ||
				!Build_Packet_Frame(
					PACKET_TYPE::C2S_ENTER_WORLD,
					payloadWriter.Get_Buffer(),
					frameBytes) ||
				!Send_All(frameBytes, error))
			{
				Close();
				return false;
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
				if (m_hasEnterRejected)
					return true;
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
			return m_hasEnterAccepted;
		}

		[[nodiscard]] bool Is_RoomFullRejected() const
		{
			return m_hasEnterRejected &&
				WORLD_ID::VALTAN_ARENA == m_EnterRejected.eWorldId &&
				ENTER_WORLD_REJECTION_REASON::ROOM_FULL ==
					m_EnterRejected.eReason;
		}

		[[nodiscard]] bool Has_PlayerCount(const std::size_t expected) const
		{
			return m_hasWorldSnapshot && m_iSnapshotPlayerCount == expected &&
				m_iSnapshotUniqueEntityCount == expected;
		}

		[[nodiscard]] PLAYER_ID Get_PlayerId() const
		{
			return m_EnterAccepted.iPlayerId;
		}

		[[nodiscard]] NET_ENTITY_ID Get_NetEntityId() const
		{
			return m_EnterAccepted.iNetEntityId;
		}

		[[nodiscard]] const std::string& Get_Label() const
		{
			return m_strLabel;
		}

	private:
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
				if (m_hasEnterAccepted || !Read_Message(reader, accepted) ||
					0u != reader.Get_RemainingSize() ||
					WORLD_ID::VALTAN_ARENA != accepted.eWorldId)
				{
					error = m_strLabel + ": invalid or duplicate enter acceptance";
					return false;
				}
				m_EnterAccepted = accepted;
				m_hasEnterAccepted = true;
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
				m_EnterRejected = rejected;
				m_hasEnterRejected = true;
				return true;
			}
			if (PACKET_TYPE::S2C_WORLD_SNAPSHOT == frame.ePacketType)
			{
				S2C_WORLD_SNAPSHOT snapshot{};
				if (!Read_Message(reader, snapshot) ||
					0u != reader.Get_RemainingSize() ||
					WORLD_ID::VALTAN_ARENA != snapshot.eWorldId)
				{
					error = m_strLabel + ": invalid Valtan world snapshot";
					return false;
				}
				std::set<NET_ENTITY_ID> entityIds;
				for (const PLAYER_SNAPSHOT& player : snapshot.Players)
					entityIds.insert(player.iNetEntityId);
				m_iSnapshotPlayerCount = snapshot.Players.size();
				m_iSnapshotUniqueEntityCount = entityIds.size();
				m_hasWorldSnapshot = true;
				return true;
			}

			// Spawn/despawn and boss packets are intentionally consumed by the
			// production stream parser but are not cohort admission assertions.
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
		CPacketStreamParser m_StreamParser;
		S2C_ENTER_ACCEPTED m_EnterAccepted{};
		S2C_ENTER_REJECTED m_EnterRejected{};
		std::size_t m_iSnapshotPlayerCount = 0;
		std::size_t m_iSnapshotUniqueEntityCount = 0;
		bool m_hasEnterAccepted = false;
		bool m_hasEnterRejected = false;
		bool m_hasWorldSnapshot = false;
	};

	template<typename PREDICATE>
	bool Pump_Until(
		const std::span<CTestClient*> clients,
		const std::chrono::milliseconds timeout,
		PREDICATE predicate,
		const std::string_view stage,
		std::string& error)
	{
		const auto deadline = std::chrono::steady_clock::now() + timeout;
		while (std::chrono::steady_clock::now() < deadline)
		{
			if (predicate())
				return true;
			for (CTestClient* client : clients)
			{
				if (nullptr != client && !client->Poll(error))
				{
					error = std::string(stage) + ": " + error;
					return false;
				}
			}
			if (predicate())
				return true;
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		}
		error = std::string(stage) + ": timed out after " +
			std::to_string(timeout.count()) + " ms";
		return false;
	}

	bool All_AcceptedWithSnapshotCount(
		const std::span<CTestClient*> clients,
		const std::size_t expectedCount)
	{
		std::set<PLAYER_ID> playerIds;
		std::set<NET_ENTITY_ID> entityIds;
		for (const CTestClient* client : clients)
		{
			if (nullptr == client || !client->Is_Accepted() ||
				!client->Has_PlayerCount(expectedCount))
			{
				return false;
			}
			playerIds.insert(client->Get_PlayerId());
			entityIds.insert(client->Get_NetEntityId());
		}
		return playerIds.size() == clients.size() &&
			entityIds.size() == clients.size();
	}

	std::unique_ptr<CTestClient> Connect_Client(
		const HARNESS_OPTIONS& options,
		const std::string& label,
		const CHARACTER_CLASS_ID characterClass,
		std::string& error)
	{
		auto client = std::make_unique<CTestClient>(label);
		if (!client->Connect_AndEnter(options, characterClass, error))
			return {};
		return client;
	}

	bool Run_FourPlayerCohort(
		const HARNESS_OPTIONS& options,
		std::string& error)
	{
		const auto timeout =
			std::chrono::milliseconds(options.iTimeoutMilliseconds);
		constexpr std::array CHARACTER_CLASSES
		{
			CHARACTER_CLASS_ID::LANCE_MASTER,
			CHARACTER_CLASS_ID::GUNSLINGER,
			CHARACTER_CLASS_ID::SLAYER,
			CHARACTER_CLASS_ID::ARTIST,
			CHARACTER_CLASS_ID::DIMENSIONMASTER,
			CHARACTER_CLASS_ID::WARLORD
		};

		std::vector<std::unique_ptr<CTestClient>> firstGeneration;
		std::vector<CTestClient*> firstActive;
		for (std::size_t index = 0; index < 4u; ++index)
		{
			auto client = Connect_Client(
				options,
				"G1-Player-" + std::to_string(index + 1u),
				CHARACTER_CLASSES[index],
				error);
			if (nullptr == client)
				return false;
			firstActive.push_back(client.get());
			firstGeneration.push_back(std::move(client));
		}
		if (!Pump_Until(firstActive, timeout,
			[&firstActive]()
			{
				return All_AcceptedWithSnapshotCount(firstActive, 4u);
			},
			"first generation reaches four-player snapshot", error))
		{
			return false;
		}
		std::cout << "[PASS] four initial clients accepted and converged at 4/4\n";

		auto overflow = Connect_Client(
			options,
			"G1-Overflow-5",
			CHARACTER_CLASSES[4],
			error);
		if (nullptr == overflow)
			return false;
		std::array overflowSpan{ overflow.get() };
		if (!Pump_Until(overflowSpan, timeout,
			[&overflow]() { return overflow->Is_RoomFullRejected(); },
			"fifth client receives typed ROOM_FULL", error))
		{
			return false;
		}
		std::cout << "[PASS] fifth client received S2C_ENTER_REJECTED ROOM_FULL\n";
		overflow->Close();

		firstGeneration[1]->Close();
		firstActive.erase(firstActive.begin() + 1);
		if (!Pump_Until(firstActive, timeout,
			[&firstActive]()
			{
				return std::all_of(
					firstActive.begin(), firstActive.end(),
					[](const CTestClient* client)
					{
						return nullptr != client && client->Has_PlayerCount(3u);
					});
			},
			"remaining clients observe disconnect", error))
		{
			return false;
		}

		auto replacement = Connect_Client(
			options,
			"G1-Replacement-5",
			CHARACTER_CLASSES[5],
			error);
		if (nullptr == replacement)
			return false;
		firstActive.push_back(replacement.get());
		if (!Pump_Until(firstActive, timeout,
			[&firstActive]()
			{
				return All_AcceptedWithSnapshotCount(firstActive, 4u);
			},
			"replacement restores four-player snapshot", error))
		{
			return false;
		}
		std::cout << "[PASS] disconnect and replacement reconverged at 4/4\n";

		for (CTestClient* client : firstActive)
			client->Close();
		replacement->Close();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		std::vector<std::unique_ptr<CTestClient>> secondGeneration;
		std::vector<CTestClient*> secondActive;
		for (std::size_t index = 0; index < 4u; ++index)
		{
			auto client = Connect_Client(
				options,
				"G2-Player-" + std::to_string(index + 1u),
				CHARACTER_CLASSES[(index + 2u) % CHARACTER_CLASSES.size()],
				error);
			if (nullptr == client)
				return false;
			secondActive.push_back(client.get());
			secondGeneration.push_back(std::move(client));
		}
		if (!Pump_Until(secondActive, timeout,
			[&secondActive]()
			{
				return All_AcceptedWithSnapshotCount(secondActive, 4u);
			},
			"second generation reaches four-player snapshot", error))
		{
			return false;
		}
		std::cout << "[PASS] empty-room reset admitted a second 4/4 generation\n";
		for (CTestClient* client : secondActive)
			client->Close();
		return true;
	}
}

int main(const int argumentCount, char** arguments)
{
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

	if (!Run_FourPlayerCohort(options, error))
	{
		std::cerr << "[FAILURE] " << error << '\n';
		return 1;
	}

	std::cout << "failures : 0\n";
	return 0;
}

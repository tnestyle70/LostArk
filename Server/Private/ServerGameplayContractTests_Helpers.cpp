#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "SpawnGroupBootstrap.h"
#include "WorldDestructionBootstrapContractTests.h"
#include <Windows.h>
#include <process.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <sstream>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>


using namespace LostArk::Server;
using namespace LostArk::Shared;

namespace ServerGameplayContractDetail
{


	unsigned __stdcall Run_ContractStackWork(void* opaque) noexcept
	{
		const CONTRACT_STACK_WORK* work =
			static_cast<const CONTRACT_STACK_WORK*>(opaque);
		if (nullptr == work || nullptr == work->pFunction) return 1u;
		work->pFunction(work->pContext);
		return 0u;
	}

	bool Run_WithReservedWorkerStack(
		void (*function)(void*), void* context,
		const unsigned stackReserve)
	{
		CONTRACT_STACK_WORK work{ function, context };
		const uintptr_t rawHandle = _beginthreadex(
			nullptr, stackReserve, Run_ContractStackWork,
			&work, STACK_SIZE_PARAM_IS_A_RESERVATION, nullptr);
		if (0u == rawHandle) return false;
		const HANDLE handle = reinterpret_cast<HANDLE>(rawHandle);
		const DWORD waitResult = ::WaitForSingleObject(handle, INFINITE);
		DWORD exitCode = 1u;
		const bool completed = WAIT_OBJECT_0 == waitResult &&
			FALSE != ::GetExitCodeThread(handle, &exitCode) && 0u == exitCode;
		::CloseHandle(handle);
		return completed;
	}

	bool Run_WithContractWorkerStack(
		void (*function)(void*), void* context)
	{
		/* Synthetic catalog generations run deep parse/validate call trees. Give
			the worker an explicit reserve without weakening Server.exe's normal 1 MiB
			main-thread stack contract or hiding oversized locals in the driver. */
		constexpr unsigned CONTRACT_WORKER_STACK_RESERVE = 2u * 1024u * 1024u;
		return Run_WithReservedWorkerStack(
			function, context, CONTRACT_WORKER_STACK_RESERVE);
	}

	bool Run_WithProductionServerStack(
		void (*function)(void*), void* context)
	{
		constexpr unsigned SERVER_STACK_RESERVE = 1u * 1024u * 1024u;
		return Run_WithReservedWorkerStack(
			function, context, SERVER_STACK_RESERVE);
	}

	std::uint32_t Calculate_TestTimelineCommandId(const std::string_view rowId)
	{
		std::uint32_t hash = 2166136261u;
		for (const unsigned char character : rowId)
		{
			hash ^= character;
			hash *= 16777619u;
		}
		return hash;
	}

	std::uint32_t Count_TestTimelineRows(const TEST_TIMELINE_VARIANT variant)
	{
		switch (variant)
		{
		case TEST_TIMELINE_VARIANT::VALID:
		case TEST_TIMELINE_VARIANT::MISSING_ROW:
		case TEST_TIMELINE_VARIANT::OVERSIZED_ACTION_INDEX:
		case TEST_TIMELINE_VARIANT::COMMAND_HASH_MISMATCH:
			return 3u;
		case TEST_TIMELINE_VARIANT::MISSING_ACTION:
			return 4u;
		case TEST_TIMELINE_VARIANT::COMMAND_HASH_COLLISION:
			return 5u;
		}
		return 0u;
	}

	void Write_TestValtanTimelineRows(
		std::ofstream& bootstrap,
		const TEST_TIMELINE_VARIANT variant)
	{
		const std::string rowId = "valtan.timeline.test";
		const bool twoRows = TEST_TIMELINE_VARIANT::MISSING_ROW == variant ||
			TEST_TIMELINE_VARIANT::MISSING_ACTION == variant ||
			TEST_TIMELINE_VARIANT::COMMAND_HASH_COLLISION == variant;
		bootstrap <<
			"VALTANTIMELINE\tENCOUNTER_VALTAN\tVALTAN_TIMELINE\t" <<
			(twoRows ? 2u : 1u) << '\n' <<
			/* Publisher rows are lexical, so actions precede their occurrence.
			The runtime parser must stage either order without partial commit. */
			"VALTANTIMELINEPATTERN\tENCOUNTER_VALTAN\tVALTAN_TIMELINE\t1\t" <<
			(TEST_TIMELINE_VARIANT::OVERSIZED_ACTION_INDEX == variant ? 9u : 1u) <<
			"\tVALTAN_TEST\t1\n";
		if (TEST_TIMELINE_VARIANT::COMMAND_HASH_COLLISION == variant)
		{
			bootstrap <<
				"VALTANTIMELINEPATTERN\tENCOUNTER_VALTAN\tVALTAN_TIMELINE\t2\t1\tVALTAN_TEST\t1\n";
			const std::string firstCollisionId =
				"valtan.timeline.collision.1b38rfb.15jd";
			const std::string secondCollisionId =
				"valtan.timeline.collision.15o0dig.1jiw";
			bootstrap <<
				"VALTANTIMELINEROW\tENCOUNTER_VALTAN\tVALTAN_TIMELINE\t" <<
				Calculate_TestTimelineCommandId(firstCollisionId) << "\t1\t" <<
				firstCollisionId <<
				"\t160\tMECHANIC\tFRESH\tHIDDEN\t1\n"
				"VALTANTIMELINEROW\tENCOUNTER_VALTAN\tVALTAN_TIMELINE\t" <<
				Calculate_TestTimelineCommandId(secondCollisionId) << "\t2\t" <<
				secondCollisionId <<
				"\t159\tNORMAL\tFRESH\tHIDDEN\t1\n";
			return;
		}
		const std::uint32_t commandId = Calculate_TestTimelineCommandId(rowId) +
			(TEST_TIMELINE_VARIANT::COMMAND_HASH_MISMATCH == variant ? 1u : 0u);
		bootstrap <<
			"VALTANTIMELINEROW\tENCOUNTER_VALTAN\tVALTAN_TIMELINE\t" <<
			commandId << "\t1\t" << rowId <<
			"\t160\tMECHANIC\tFRESH\tHIDDEN\t1\n";
		if (TEST_TIMELINE_VARIANT::MISSING_ACTION == variant)
		{
			const std::string secondRowId = "valtan.timeline.test.second";
			bootstrap <<
				"VALTANTIMELINEROW\tENCOUNTER_VALTAN\tVALTAN_TIMELINE\t" <<
				Calculate_TestTimelineCommandId(secondRowId) << "\t2\t" <<
				secondRowId <<
				"\t159\tNORMAL\tFRESH\tHIDDEN\t1\n";
		}
	}

	void Write_ValidValtanTimelineRows(std::ofstream& bootstrap)
	{
		Write_TestValtanTimelineRows(
			bootstrap, TEST_TIMELINE_VARIANT::VALID);
	}

	void Close_TestSocket(SOCKET& socket)
	{
		if (INVALID_SOCKET == socket)
			return;
		::shutdown(socket, SD_BOTH);
		::closesocket(socket);
		socket = INVALID_SOCKET;
	}

	void Abort_TestSocket(SOCKET& socket)
	{
		if (INVALID_SOCKET == socket)
			return;
		linger abortiveClose{};
		abortiveClose.l_onoff = 1u;
		abortiveClose.l_linger = 0u;
		(void)::setsockopt(
			socket,
			SOL_SOCKET,
			SO_LINGER,
			reinterpret_cast<const char*>(&abortiveClose),
			static_cast<int>(sizeof(abortiveClose)));
		::closesocket(socket);
		socket = INVALID_SOCKET;
	}

	bool Create_LoopbackSocketPair(
		SOCKET& outSessionSocket,
		SOCKET& outPeerSocket)
	{
		outSessionSocket = INVALID_SOCKET;
		outPeerSocket = INVALID_SOCKET;
		SOCKET listener = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == listener)
			return false;

		sockaddr_in address{};
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = ::htonl(INADDR_LOOPBACK);
		address.sin_port = 0u;
		if (SOCKET_ERROR == ::bind(
			listener,
			reinterpret_cast<const sockaddr*>(&address),
			static_cast<int>(sizeof(address))) ||
			SOCKET_ERROR == ::listen(listener, 1))
		{
			Close_TestSocket(listener);
			return false;
		}

		int addressBytes = static_cast<int>(sizeof(address));
		if (SOCKET_ERROR == ::getsockname(
			listener,
			reinterpret_cast<sockaddr*>(&address),
			&addressBytes))
		{
			Close_TestSocket(listener);
			return false;
		}

		SOCKET peer = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == peer ||
			SOCKET_ERROR == ::connect(
				peer,
				reinterpret_cast<const sockaddr*>(&address),
				static_cast<int>(sizeof(address))))
		{
			Close_TestSocket(peer);
			Close_TestSocket(listener);
			return false;
		}

		SOCKET session = ::accept(listener, nullptr, nullptr);
		Close_TestSocket(listener);
		if (INVALID_SOCKET == session)
		{
			Close_TestSocket(peer);
			return false;
		}

		const DWORD receiveTimeoutMilliseconds = 1500u;
		if (SOCKET_ERROR == ::setsockopt(
			peer,
			SOL_SOCKET,
			SO_RCVTIMEO,
			reinterpret_cast<const char*>(&receiveTimeoutMilliseconds),
			static_cast<int>(sizeof(receiveTimeoutMilliseconds))))
		{
			Close_TestSocket(session);
			Close_TestSocket(peer);
			return false;
		}

		outSessionSocket = session;
		outPeerSocket = peer;
		return true;
	}

	bool Receive_Exact(
		const SOCKET socket,
		std::span<std::uint8_t> bytes)
	{
		std::size_t receivedBytes = 0u;
		while (receivedBytes < bytes.size())
		{
			const int result = ::recv(
				socket,
				reinterpret_cast<char*>(bytes.data() + receivedBytes),
				static_cast<int>(bytes.size() - receivedBytes),
				0);
			if (result <= 0)
				return false;
			receivedBytes += static_cast<std::size_t>(result);
		}
		return true;
	}

	bool Receive_TestFrame(
		const SOCKET socket,
		RECEIVED_TEST_FRAME& outFrame)
	{
		using namespace LostArk::Shared;
		std::array<std::uint8_t, PACKET_HEADER_BYTES> headerBytes{};
		if (!Receive_Exact(socket, headerBytes))
			return false;
		PACKET_HEADER header{};
		if (!Read_Packet_Header(headerBytes, header))
			return false;

		RECEIVED_TEST_FRAME decoded{};
		decoded.packetType = header.ePacketType;
		decoded.payload.resize(
			static_cast<std::size_t>(header.iTotalSize) - PACKET_HEADER_BYTES);
		if (!decoded.payload.empty() &&
			!Receive_Exact(socket, decoded.payload))
		{
			return false;
		}
		outFrame = std::move(decoded);
		return true;
	}

	bool Replace_FirstSpawnAnchorField(
		std::string& bootstrapText,
		const std::size_t fieldIndex,
		const std::string_view replacement)
	{
		std::size_t rowStart = bootstrapText.find("ANCHOR\t");
		while (std::string::npos != rowStart && 0u != rowStart &&
			'\n' != bootstrapText[rowStart - 1u])
		{
			rowStart = bootstrapText.find("ANCHOR\t", rowStart + 1u);
		}
		if (std::string::npos == rowStart)
			return false;

		std::size_t rowEnd = bootstrapText.find('\n', rowStart);
		if (std::string::npos == rowEnd)
			rowEnd = bootstrapText.size();
		std::size_t valueEnd = rowEnd;
		if (valueEnd > rowStart && '\r' == bootstrapText[valueEnd - 1u])
			--valueEnd;

		std::size_t fieldStart = rowStart;
		for (std::size_t index = 0u; index < fieldIndex; ++index)
		{
			const std::size_t tab = bootstrapText.find('\t', fieldStart);
			if (std::string::npos == tab || tab >= valueEnd)
				return false;
			fieldStart = tab + 1u;
		}
		std::size_t fieldEnd = bootstrapText.find('\t', fieldStart);
		if (std::string::npos == fieldEnd || fieldEnd > valueEnd)
			fieldEnd = valueEnd;
		if (fieldStart >= fieldEnd)
			return false;
		bootstrapText.replace(
			fieldStart, fieldEnd - fieldStart, replacement);
		return true;
	}

	bool Reject_CorruptSpawnAnchorReloadTransactionally(
		LostArk::Server::CSpawnGroupBootstrap& bootstrap)
	{
		namespace fs = std::filesystem;
		using LostArk::Server::SPAWN_GROUP_ANCHOR;
		using LostArk::Shared::WORLD_ID;

		const auto& groups = bootstrap.Get_Groups();
		std::string preservedAnchorId;
		for (const auto& group : groups)
		{
			for (const auto& wave : group.Waves)
			{
				if (!wave.Entries.empty())
				{
					preservedAnchorId = wave.Entries.front().strAnchorId;
					break;
				}
			}
			if (!preservedAnchorId.empty())
				break;
		}
		const SPAWN_GROUP_ANCHOR* preservedAnchor =
			bootstrap.Find_Anchor(preservedAnchorId);
		if (groups.empty() || nullptr == preservedAnchor)
			return false;
		const SPAWN_GROUP_ANCHOR expectedAnchor = *preservedAnchor;
		const std::size_t expectedGroupCount = groups.size();
		const std::string expectedFirstGroupId =
			groups.front().strSpawnGroupId;
		const std::uint32_t expectedRevision = bootstrap.Get_Revision();

		std::vector<wchar_t> environmentBuffer(32768u);
		const DWORD configuredLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", environmentBuffer.data(),
			static_cast<DWORD>(environmentBuffer.size()));
		if (configuredLength >= environmentBuffer.size())
			return false;
		const bool hadConfiguredRoot = 0u != configuredLength;
		const std::wstring previousRoot = hadConfiguredRoot ?
			environmentBuffer.data() : L"";
		fs::path packagedDataRoot;
		if (hadConfiguredRoot)
		{
			packagedDataRoot = fs::path(previousRoot).lexically_normal();
		}
		else
		{
			std::vector<wchar_t> moduleBuffer(32768u);
			const DWORD moduleLength = GetModuleFileNameW(
				nullptr, moduleBuffer.data(),
				static_cast<DWORD>(moduleBuffer.size()));
			if (0u == moduleLength || moduleLength >= moduleBuffer.size())
				return false;
			packagedDataRoot = fs::path(moduleBuffer.data()).parent_path().
				parent_path() / L"DataFiles";
		}

		std::ifstream input(
			packagedDataRoot / L"World" /
				L"VALTAN_ARENA.spawngroupsbootstrap",
			std::ios::binary);
		if (!input)
			return false;
		const std::string validText{
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>() };
		if (validText.empty())
			return false;

		const fs::path fixtureRoot = fs::temp_directory_path() /
			(L"LostArkSpawnAnchorContractTest-" +
				std::to_wstring(GetCurrentProcessId()));
		std::error_code fixtureError;
		fs::remove_all(fixtureRoot, fixtureError);
		if (fixtureError)
			return false;
		fs::create_directories(fixtureRoot / L"World", fixtureError);
		if (fixtureError || !SetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", fixtureRoot.c_str()))
		{
			return false;
		}

		struct ANCHOR_CORRUPTION final
		{
			std::size_t fieldIndex = 0u;
			std::string_view replacement;
		};
		constexpr std::array corruptions{
			ANCHOR_CORRUPTION{ 2u, "100001" },
			ANCHOR_CORRUPTION{ 3u, "-100001" },
			ANCHOR_CORRUPTION{ 4u, "1e20" },
			ANCHOR_CORRUPTION{ 5u, "1e20" },
			ANCHOR_CORRUPTION{ 2u, "nan" },
			ANCHOR_CORRUPTION{ 3u, "nan" },
			ANCHOR_CORRUPTION{ 4u, "nan" },
			ANCHOR_CORRUPTION{ 5u, "nan" }
		};
		bool rejectedAll = true;
		for (const ANCHOR_CORRUPTION& corruption : corruptions)
		{
			std::string corruptText = validText;
			if (!Replace_FirstSpawnAnchorField(
				corruptText, corruption.fieldIndex, corruption.replacement))
			{
				rejectedAll = false;
				break;
			}
			bool wroteFixture = false;
			{
				std::ofstream output(
					fixtureRoot / L"World" /
						L"VALTAN_ARENA.spawngroupsbootstrap",
					std::ios::binary | std::ios::trunc);
				output.write(corruptText.data(),
					static_cast<std::streamsize>(corruptText.size()));
				wroteFixture = output.good();
			}
			const bool rejected = wroteFixture &&
				!bootstrap.Load(WORLD_ID::VALTAN_ARENA);
			const SPAWN_GROUP_ANCHOR* currentAnchor =
				bootstrap.Find_Anchor(preservedAnchorId);
			rejectedAll = rejectedAll && rejected &&
				bootstrap.Get_Status() ==
					"Spawn group anchor row is invalid" &&
				expectedRevision == bootstrap.Get_Revision() &&
				expectedGroupCount == bootstrap.Get_Groups().size() &&
				!bootstrap.Get_Groups().empty() &&
				expectedFirstGroupId ==
					bootstrap.Get_Groups().front().strSpawnGroupId &&
				nullptr != currentAnchor &&
				expectedAnchor.fPositionX == currentAnchor->fPositionX &&
				expectedAnchor.fPositionY == currentAnchor->fPositionY &&
				expectedAnchor.fPositionZ == currentAnchor->fPositionZ &&
				expectedAnchor.fYawDegrees == currentAnchor->fYawDegrees;
			if (!rejectedAll)
				break;
		}

		const bool restoredEnvironment = SetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT",
			hadConfiguredRoot ? previousRoot.c_str() : nullptr);
		fixtureError.clear();
		fs::remove_all(fixtureRoot, fixtureError);
		return rejectedAll && restoredEnvironment && !fixtureError;
	}
}

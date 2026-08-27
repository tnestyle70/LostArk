#include "ServerGameplayContractTests.h"

#include "ClientSession.h"
#include "BossCombatRuntime.h"
#include "EncounterPropRuntime.h"
#include "Gameplay/CombatCollisionContract.h"
#include "Gameplay/WorldCollisionContract.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "NpcBehaviorRuntime.h"
#include "PlayerSkillSystem.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
#include "ServerNavigation.h"
#include "ServerCollisionSystem.h"
#include "ServerCombatHitRuntime.h"
#include "ServerApp.h"
#include "ServerTriggerSystem.h"
#include "SpawnGroupBootstrap.h"
#include "SpawnGroupRuntime.h"
#include "ValtanBrain.h"
#include "WinSockContext.h"
#include "WorldBootstrap.h"
#include "WorldDestructionRuntime.h"
#include "WorldDestructionBootstrapContractTests.h"

#include <Windows.h>
#include <process.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

namespace
{
	struct TESTS
	{
		explicit TESTS(const bool groundTargetOnly = false)
			: groundTargetOnly(groundTargetOnly)
		{
		}

		void Require(const bool condition, const char* name)
		{
			if (groundTargetOnly)
				return;
			Record(condition, name);
		}

		void Require_GroundTarget(const bool condition, const char* name)
		{
			Record(condition, name);
		}

		void Record(const bool condition, const char* name)
		{
			std::cout << (condition ? "[PASS] " : "[FAILURE] ") << name << '\n';
			if (!condition)
				++failures;
		}
		int failures = 0;
		bool groundTargetOnly = false;
	};

	struct CONTRACT_STACK_WORK final
	{
		void (*pFunction)(void*) = nullptr;
		void* pContext = nullptr;
	};

	unsigned __stdcall Run_ContractStackWork(void* opaque) noexcept
	{
		const CONTRACT_STACK_WORK* work =
			static_cast<const CONTRACT_STACK_WORK*>(opaque);
		if (nullptr == work || nullptr == work->pFunction) return 1u;
		work->pFunction(work->pContext);
		return 0u;
	}

	bool Run_WithContractWorkerStack(
		void (*function)(void*), void* context)
	{
		/* Synthetic catalog generations run deep parse/validate call trees. Give
		the worker an explicit reserve without weakening Server.exe's normal 1 MiB
		main-thread stack contract or hiding oversized locals in the driver. */
		constexpr unsigned CONTRACT_WORKER_STACK_RESERVE = 2u * 1024u * 1024u;
		CONTRACT_STACK_WORK work{ function, context };
		const uintptr_t rawHandle = _beginthreadex(
			nullptr, CONTRACT_WORKER_STACK_RESERVE, Run_ContractStackWork,
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

	/* Run_ServerGameplayContractTests is intentionally one broad executable
	contract and is already close to the Windows default stack budget. Keep the
	hot-reload occurrence simulation on the heap so extending that contract does
	not make test startup depend on compiler stack-slot reuse. */
	struct PINNED_MECHANIC_GENERATION_FIXTURE final
	{
		LostArk::Server::SERVER_WORLD_ENTITY Boss;
		LostArk::Server::SERVER_WORLD_ENTITY ReconcileBoss;
		LostArk::Server::SERVER_WORLD_ENTITY ThresholdBoss;
		std::map<LostArk::Shared::PLAYER_ID,
			LostArk::Server::SERVER_PLAYER> Players;
		std::map<LostArk::Shared::PLAYER_ID,
			LostArk::Server::SERVER_PLAYER> ReconcilePlayers;
		LostArk::Server::SERVER_PLAYER Target;
		LostArk::Server::CValtanBrain Brain;
		std::vector<LostArk::Shared::DAMAGE_EVENT> DamageEvents;
		std::vector<LostArk::Shared::GameplayDataRevision> RequiredPins;
	};

	constexpr std::uint32_t VALID_VALTAN_TIMELINE_ROW_COUNT = 3u;
	enum class TEST_TIMELINE_VARIANT : std::uint8_t
	{
		VALID,
		MISSING_ROW,
		MISSING_ACTION,
		OVERSIZED_ACTION_INDEX,
		COMMAND_HASH_MISMATCH,
		COMMAND_HASH_COLLISION
	};

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

	struct RECEIVED_TEST_FRAME final
	{
		LostArk::Shared::PACKET_TYPE packetType =
			LostArk::Shared::PACKET_TYPE::INVALID;
		std::vector<std::uint8_t> payload;
	};

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

	template <typename PREDICATE>
	bool Wait_Until(
		const std::chrono::milliseconds timeout,
		PREDICATE&& predicate)
	{
		const auto deadline = std::chrono::steady_clock::now() + timeout;
		do
		{
			if (predicate())
				return true;
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		} while (std::chrono::steady_clock::now() < deadline);
		return predicate();
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

	struct QUICK_SKILL_CONTRACT final
	{
		LostArk::Shared::CHARACTER_CLASS_ID characterClass;
		LostArk::Shared::SKILL_ID skillId;
		const char* inputSlot;
	};

	constexpr std::array QUICK_SKILLS
	{
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34040, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34540, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34090, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34550, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34100, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34560, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34160, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34570, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34140, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34580, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34120, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34590, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34110, "D" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34150, "F" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34000, "Z" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34500, "Z" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34020, "SPACE" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34520, "SPACE" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34650, "T" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34610, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34630, "ALT_V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38020, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38050, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38120, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38200, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38140, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38180, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38210, "D" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38260, "F" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38290, "T" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38250, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38320, "ALT_V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45050, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45060, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45620, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45210, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45300, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45070, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45190, "D" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45600, "F" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45810, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45820, "ALT_V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31200, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31430, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31480, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31210, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31460, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31420, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31490, "D" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31470, "F" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31950, "T" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31110, "X" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31050, "Z" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31910, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31930, "ALT_V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31020, "SPACE" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050100, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050120, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050160, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050180, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050210, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050220, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050240, "D" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050230, "F" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050500, "T" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050520, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050540, "ALT_V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050020, "SPACE" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17030, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17060, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17080, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17110, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17090, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17040, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17100, "D" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17140, "F" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17240, "T" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17820, "X" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17170, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17250, "ALT_V" }
	};

	struct BASIC_ATTACK_CONTRACT final
	{
		LostArk::Shared::CHARACTER_CLASS_ID characterClass;
		LostArk::Shared::SKILL_ID skillId;
		std::size_t stageCount;
	};

	constexpr std::array BASIC_ATTACKS
	{
		BASIC_ATTACK_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17000, 3 },
		BASIC_ATTACK_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34010, 4 },
		BASIC_ATTACK_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34510, 3 },
		BASIC_ATTACK_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38000, 3 },
		BASIC_ATTACK_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45000, 4 },
		BASIC_ATTACK_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31000, 4 },
		BASIC_ATTACK_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050010, 3 }
	};
}

int LostArk::Server::Run_ServerGameplayContractTests(
	const bool dimensionMasterGroundTargetOnly)
{
	struct CONTRACT_TEST_RUN_CONTEXT final
	{
		bool dimensionMasterGroundTargetOnly = false;
		int result = 1;
	};

	CONTRACT_TEST_RUN_CONTEXT context{ dimensionMasterGroundTargetOnly, 1 };
	const auto runContract = [](void* opaque)
	{
		CONTRACT_TEST_RUN_CONTEXT& context =
			*static_cast<CONTRACT_TEST_RUN_CONTEXT*>(opaque);
		const bool dimensionMasterGroundTargetOnly =
			context.dimensionMasterGroundTargetOnly;
		context.result = [&]() -> int
		{
	using namespace LostArk::Shared;
	TESTS tests{ dimensionMasterGroundTargetOnly };
	CGameplayCatalog catalog;
	const bool catalogLoaded = catalog.Load();
	if (!catalogLoaded)
	{
		std::cout << "[STATUS] " << catalog.Get_Status() << std::endl;
		if (dimensionMasterGroundTargetOnly)
			tests.Require_GroundTarget(
				false, "Load gameplay balance bootstrap for DimensionMaster T");
		else
			tests.Require(false, "Load gameplay balance bootstrap");
		std::cout << "failures : " << tests.failures << '\n';
		return 1;
	}
	if (dimensionMasterGroundTargetOnly)
		tests.Require_GroundTarget(
			catalogLoaded, "Load gameplay balance bootstrap for DimensionMaster T");
	else
		tests.Require(catalogLoaded, "Load gameplay balance bootstrap");
	tests.Require(
		catalog.Get_ActiveRevision().Is_Valid(),
		"Derive a nonzero gameplay data revision from admitted bootstrap bytes");
	{
		const BOSS_PATTERN_SEQUENCE_DEFINITION* sequence =
			catalog.Find_BossPatternSequence("ENCOUNTER_VALTAN");
		const std::vector<std::string> expectedPatternIds{
			"VALTAN_ENTRANCE_CINEMATIC",
			"VALTAN_WHIRLWIND",
			"VALTAN_FOUR_SLASH",
			"VALTAN_FIST_IN_OUT",
			"VALTAN_HIGH_JUMP",
			"VALTAN_FLOOR_WIPE_130",
			"VALTAN_DASH_CHARGE",
			"VALTAN_ARENA_BREAK_109",
			"VALTAN_SIX_PIZZA_106",
			"VALTAN_ATTACK_WHIRLWIND",
			"VALTAN_CHARGE",
			"VALTAN_SEQUENCE_FOUR",
			"VALTAN_ROAR_CHARGE",
			"VALTAN_SEQUENCE_RUSH",
			"VALTAN_THREE",
			"VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK",
			"VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK",
			"VALTAN_TERRAIN_DESTRUCTION",
			"VALTAN_SEQUENCE_FRONT_BACK_FRONT",
			"VALTAN_WARP",
			"VALTAN_SEQUENCE_TWOHAND",
			"VALTAN_SEQUENCE_WHIRLWIND",
			"VALTAN_TRASH",
			"VALTAN_TRASH_CATCH_SUCCESS",
			"VALTAN_TRASH_CATCH_FAIL",
			"VALTAN_TRASH_CATCH_IF",
			"VALTAN_CATCH_BREATH",
			"VALTAN_COUNTER",
			"VALTAN_CHARGE_2" };
		tests.Require(
			nullptr != sequence &&
			"sequence.valtan.server-authored.v1" == sequence->strSequenceId &&
			BOSS_PATTERN_SEQUENCE_MODE::ORDERED_ONCE_THEN_IDLE ==
				sequence->eMode &&
			1000u == sequence->iInterStepPursuitMs &&
			30u == sequence->iInterStepPursuitTicks &&
			29u == sequence->iExpectedStepCount &&
			sequence->PatternIds == expectedPatternIds,
			"Load the entrance camera gate, seven-pattern Phase-1 review, and 21 Phase-2 animation sequence from bootstrap v24");
	}
	{
		const auto* patterns =
			catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const auto findPattern = [patterns](const char* patternId)
			-> const BOSS_PATTERN_DEFINITION*
			{
				if (nullptr == patterns) return nullptr;
				const auto found = std::find_if(
					patterns->begin(), patterns->end(),
					[patternId](const BOSS_PATTERN_DEFINITION& pattern)
					{ return pattern.strPatternId == patternId; });
				return patterns->end() == found ? nullptr : &*found;
			};
		const auto findStage = [](const BOSS_PATTERN_DEFINITION* pattern,
			const char* stageId) -> const BOSS_PATTERN_STAGE_DEFINITION*
			{
				if (nullptr == pattern) return nullptr;
				const auto found = std::find_if(
					pattern->Stages.begin(), pattern->Stages.end(),
					[stageId](const BOSS_PATTERN_STAGE_DEFINITION& stage)
					{ return stage.strStageId == stageId; });
				return pattern->Stages.end() == found ? nullptr : &*found;
			};
		const BOSS_PATTERN_DEFINITION* trash = findPattern("VALTAN_TRASH");
		const BOSS_PATTERN_DEFINITION* pizza =
			findPattern("VALTAN_SIX_PIZZA_106");
		const BOSS_PATTERN_STAGE_DEFINITION* trashCounter =
			findStage(trash, "STEP_06");
		const BOSS_PATTERN_STAGE_DEFINITION* trashRelease =
			findStage(trash, "STEP_07");
		const BOSS_PATTERN_STAGE_DEFINITION* trashRush =
			findStage(trash, "STEP_08");
		const BOSS_PATTERN_DEFINITION* catchBreath =
			findPattern("VALTAN_CATCH_BREATH");
		const BOSS_PATTERN_DEFINITION* dash =
			findPattern("VALTAN_DASH_CHARGE");
		const BOSS_PATTERN_STAGE_DEFINITION* dashGroggy =
			findStage(dash, "GROGGY");
		const BOSS_PATTERN_STAGE_DEFINITION* catchGrab =
			findStage(catchBreath, "STEP_02");
		const BOSS_PATTERN_STAGE_DEFINITION* catchRelease =
			findStage(catchBreath, "STEP_04");
		const auto hasAction = [](const BOSS_PATTERN_STAGE_DEFINITION* stage,
			const BOSS_GRABBED_RELEASE_MODE mode,
			const float speed, const std::uint32_t durationMs)
			{
				return nullptr != stage && stage->Actions.end() != std::find_if(
					stage->Actions.begin(), stage->Actions.end(),
					[mode, speed, durationMs](
						const BOSS_PATTERN_STAGE_ACTION& action)
					{
						return BOSS_PATTERN_STAGE_ACTION_KIND::
							RELEASE_GRABBED_PLAYERS == action.eKind &&
							BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER ==
								action.eTrigger &&
							"boss.attachment.left-hand" == action.strTargetId &&
							action.eReleaseMode == mode &&
							action.fReleaseSpeedMps == speed &&
							action.iDurationMs == durationMs;
					});
			};
		const auto hasBossFlagAction = [](
			const BOSS_PATTERN_STAGE_DEFINITION* stage,
			const BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			const std::string_view targetId, const std::uint32_t value)
			{
				return nullptr != stage && stage->Actions.end() != std::find_if(
					stage->Actions.begin(), stage->Actions.end(),
					[trigger, targetId, value](
						const BOSS_PATTERN_STAGE_ACTION& action)
					{
						return BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG ==
							action.eKind && trigger == action.eTrigger &&
							targetId == action.strTargetId && value == action.iValue;
					});
			};
		tests.Require(
			nullptr != pizza &&
			BOSS_PATTERN_TARGET_POLICY::LOCK_RANDOM_ALIVE_ON_START ==
				pizza->eTargetPolicy &&
			BOSS_PATTERN_AIM_POLICY::TRACK_TARGET_EACH_TICK ==
				pizza->eAimPolicy,
			"Load the six-pizza target as one random alive player whose facing is tracked without reselection");
		tests.Require(
			nullptr != trash &&
			BOSS_PATTERN_TARGET_POLICY::LOCK_RANDOM_ALIVE_ON_START ==
				trash->eTargetPolicy &&
			BOSS_PATTERN_AIM_POLICY::LOCK_FACING_ON_START ==
				trash->eAimPolicy &&
			nullptr != trashCounter && nullptr != trashRelease &&
			nullptr != trashRush &&
			BOSS_PATTERN_STAGE_KIND::WINDUP == trashCounter->eStageKind &&
			trashCounter->bHasCounterProxy &&
			std::abs(trashCounter->fCounterProxyForwardOffsetM - 1.f) < 1.0e-6f &&
			std::abs(trashCounter->fCounterProxyRightOffsetM + 1.5f) < 1.0e-6f &&
			std::abs(trashCounter->fCounterProxyRadiusM - 2.25f) < 1.0e-6f &&
			hasBossFlagAction(
				trashCounter, BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
				"boss.flag.counterable", 1u) &&
			hasBossFlagAction(
				trashCounter, BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT,
				"boss.flag.counterable", 0u) &&
			BOSS_PATTERN_PLAYER_RESPONSE::CAPTURE ==
				trashRush->ePlayerResponse &&
			PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND ==
				trashRush->eAttachmentSlot &&
			BOSS_PATTERN_HIT_SHAPE::BOX == trashRush->eHitShape &&
			7u == trashRush->HitOffsetsMs.size() &&
			hasAction(
				trashRelease, BOSS_GRABBED_RELEASE_MODE::HOLD, 0.f, 0u),
			"Load the counter-looping frontal Valtan grab, boss-local counter proxy, and hold release contract");
		tests.Require(
			nullptr != dashGroggy &&
			BOSS_PATTERN_PART_DAMAGE_POLICY::DESTROY_FIRST_ELIGIBLE ==
				dashGroggy->ePartDamagePolicy &&
			hasBossFlagAction(
				dashGroggy, BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
				"boss.flag.groggy", 1u) &&
			hasBossFlagAction(
				dashGroggy, BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT,
				"boss.flag.groggy", 0u),
			"Load the Dash GROGGY part-damage refinement and its closed groggy window");
		tests.Require(
			nullptr != catchBreath && nullptr != catchGrab &&
			nullptr != catchRelease &&
			BOSS_PATTERN_TARGET_POLICY::LOCK_RANDOM_ALIVE_BEHIND_ON_START ==
				catchBreath->eTargetPolicy &&
			BOSS_PATTERN_AIM_POLICY::LOCK_FACING_ON_START ==
				catchBreath->eAimPolicy &&
			BOSS_PATTERN_PLAYER_RESPONSE::CAPTURE ==
				catchGrab->ePlayerResponse &&
			PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND ==
				catchGrab->eAttachmentSlot &&
			BOSS_PATTERN_HIT_SHAPE::CONE == catchGrab->eHitShape &&
			hasAction(
				catchRelease,
				BOSS_GRABBED_RELEASE_MODE::OPPOSITE_KNOCKBACK,
				12.f, 500u),
			"Load the held rear-cone grab and opposite knockback release contract");
	}
	{
		CGameRoom grabRoom{ WORLD_ID::VALTAN_ARENA };
		SERVER_WORLD_ENTITY grabBoss{};
		grabBoss.iNetEntityId = 9100u;
		grabBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		grabBoss.eAction = SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
		grabBoss.strArchetypeId = "BOSS_VALTAN";
		grabBoss.strEncounterId = "ENCOUNTER_VALTAN";
		grabBoss.iCurrentHp = 1000u;
		grabBoss.iMaximumHp = 1000u;
		grabBoss.fPositionX = 0.f;
		grabBoss.fPositionY = 5.f;
		grabBoss.fPositionZ = 0.f;
		grabBoss.fYawDegrees = 0.f;
		grabRoom.m_WorldEntities.push_back(grabBoss);
		for (std::uint32_t ordinal = 0u; ordinal < 2u; ++ordinal)
		{
			SERVER_PLAYER player{};
			player.iPlayerId = 9200u + ordinal;
			player.iNetEntityId = 9300u + ordinal;
			player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			player.iCurrentHp = 1000u;
			player.iMaximumHp = 1000u;
			player.iCurrentResource = 100u;
			player.iMaximumResource = 100u;
			player.isCombatReady = true;
			player.fPositionX = 1.f - 2.f * static_cast<float>(ordinal);
			player.fPositionY = 6.f + static_cast<float>(ordinal);
			player.fPositionZ = 2.f + static_cast<float>(ordinal);
			grabRoom.m_PlayerIdByEntityId.emplace(
				player.iNetEntityId, player.iPlayerId);
			grabRoom.m_Players.emplace(player.iPlayerId, player);
		}
		const bool capturedFirst = grabRoom.Capture_PlayerAttachment(
			9300u, 9100u, PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND, 10u);
		const bool capturedSecond = grabRoom.Capture_PlayerAttachment(
			9301u, 9100u, PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND, 10u);
		SERVER_WORLD_ENTITY& movingBoss = grabRoom.m_WorldEntities.front();
		movingBoss.fPositionX = 10.f;
		movingBoss.fPositionZ = 20.f;
		movingBoss.fYawDegrees = 90.f;
		SERVER_PLAYER& first = grabRoom.m_Players.at(9200u);
		SERVER_PLAYER& second = grabRoom.m_Players.at(9201u);
		const bool followedFirst =
			grabRoom.Update_PlayerAttachment(first, 11u);
		const bool followedSecond =
			grabRoom.Update_PlayerAttachment(second, 11u);
		tests.Require(
			capturedFirst && capturedSecond && followedFirst && followedSecond &&
			PLAYER_ACTION_STATE::GRABBED == first.eAction &&
			PLAYER_ACTION_STATE::GRABBED == second.eAction &&
			!first.isCombatReady && !second.isCombatReady &&
			(std::abs(first.fPositionX - second.fPositionX) > 0.001f ||
			 std::abs(first.fPositionZ - second.fPositionZ) > 0.001f),
			"Freeze two captured players and preserve their distinct boss-local offsets");
		const std::size_t released = grabRoom.Release_PlayerAttachments(
			9100u, 6.f, 500u, false, 0u, 12u);
		grabRoom.m_PlayerIdBySessionId.emplace(9400u, first.iPlayerId);
		C2S_MOVE blockedMove{};
		blockedMove.iClientSequence = 1u;
		blockedMove.fGoalX = 100.f;
		blockedMove.fGoalZ = 100.f;
		grabRoom.Handle_Move(9400u, blockedMove);
		C2S_USE_SKILL blockedSkill{};
		blockedSkill.iClientSequence = 1u;
		blockedSkill.iSkillId = 34010u;
		blockedSkill.fAimX = 1.f;
		blockedSkill.fAimZ = 0.f;
		grabRoom.Handle_UseSkill(9400u, blockedSkill);
		tests.Require(
			2u == released && PLAYER_ACTION_STATE::NONE == first.eAction &&
			PLAYER_ATTACHMENT_SLOT::NONE == first.eAttachmentSlot &&
			INVALID_NET_ENTITY_ID == first.iAttachmentOwnerNetEntityId &&
			std::abs(first.fKnockbackRemainingSeconds - 0.5f) < 0.000001f &&
			std::abs(first.fKnockbackSpeed - 12.f) < 0.000001f &&
			!first.hasMoveGoal && INVALID_SKILL_ID == first.iCurrentSkillId,
			"Release every captured player outward and reject move or skill overlap during knockback");
		first.fKnockbackRemainingSeconds = 0.f;
		first.fKnockbackSpeed = 0.f;
		first.isCombatReady = true;
		const bool recaptured = grabRoom.Capture_PlayerAttachment(
			first.iNetEntityId, 9100u,
			PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND, 13u);
		grabRoom.m_WorldEntities.clear();
		const bool keptMissingOwner =
			grabRoom.Update_PlayerAttachment(first, 14u);
		tests.Require(
			recaptured && !keptMissingOwner &&
			PLAYER_ACTION_STATE::NONE == first.eAction &&
			INVALID_NET_ENTITY_ID == first.iAttachmentOwnerNetEntityId &&
			PLAYER_ATTACHMENT_SLOT::NONE == first.eAttachmentSlot,
			"Release a captured player fail-closed when its boss owner disappears");
	}
	{
		namespace fs = std::filesystem;
		CGameplayCatalog stagedCatalog;
		const bool stagedLoaded = stagedCatalog.Load();
		const GameplayDataRevision bootstrapRevision =
			stagedCatalog.Get_ActiveRevision();
		GameplayDataRevision parentRevision = bootstrapRevision;
		parentRevision.Bytes[0] ^= 0x40u;
		if (!parentRevision.Is_Valid() || parentRevision == bootstrapRevision)
			parentRevision.Bytes[1] ^= 1u;
		GameplayDataRevision wrongBootstrapRevision = bootstrapRevision;
		wrongBootstrapRevision.Bytes[1] ^= 0x20u;
		if (!wrongBootstrapRevision.Is_Valid() ||
			wrongBootstrapRevision == bootstrapRevision)
		{
			wrongBootstrapRevision.Bytes[2] ^= 1u;
		}
		std::vector<wchar_t> pathBuffer(32768u);
		fs::path dataRoot;
		const DWORD configuredLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", pathBuffer.data(),
			static_cast<DWORD>(pathBuffer.size()));
		if (0u != configuredLength && configuredLength < pathBuffer.size())
			dataRoot = fs::path(pathBuffer.data()).lexically_normal();
		else
		{
			const DWORD moduleLength = GetModuleFileNameW(
				nullptr, pathBuffer.data(), static_cast<DWORD>(pathBuffer.size()));
			if (0u != moduleLength && moduleLength < pathBuffer.size())
			{
				dataRoot = fs::path(pathBuffer.data()).parent_path().parent_path() /
					L"DataFiles";
			}
		}
		std::error_code pathError;
		const fs::path bootstrapPath = fs::canonical(
			dataRoot / L"Gameplay" / L"Gameplay.bootstrap", pathError);
		const bool rejectedWrongContent = stagedLoaded && !pathError &&
			!stagedCatalog.Load_FromBootstrap(
				bootstrapPath, wrongBootstrapRevision, parentRevision) &&
			bootstrapRevision == stagedCatalog.Get_ActiveRevision();
		const GameplayDataRevision invalidRevision{};
		const bool rejectedInvalidParent = !stagedCatalog.Load_FromBootstrap(
			bootstrapPath, bootstrapRevision, invalidRevision) &&
			bootstrapRevision == stagedCatalog.Get_ActiveRevision();
		const fs::path nonCanonicalPath = bootstrapPath.parent_path() /
			L".." / L"Gameplay" / L"Gameplay.bootstrap";
		const bool rejectedNonCanonicalPath =
			!stagedCatalog.Load_FromBootstrap(
				nonCanonicalPath, bootstrapRevision, parentRevision) &&
			bootstrapRevision == stagedCatalog.Get_ActiveRevision();
		const bool admittedParent = stagedCatalog.Load_FromBootstrap(
			bootstrapPath, bootstrapRevision, parentRevision) &&
			parentRevision == stagedCatalog.Get_ActiveRevision();
		tests.Require(
			rejectedWrongContent && rejectedInvalidParent &&
			rejectedNonCanonicalPath && admittedParent,
			"Load an immutable staged bootstrap and expose its verified parent revision only");
	}
	{
		const BOSS_RUNTIME_PROFILE* activeValtan =
			catalog.Find_Boss("BOSS_VALTAN");
		std::string compatibilityStatus;
		const auto rejectsResetField =
			[activeValtan, &compatibilityStatus](const auto& mutate)
		{
			if (nullptr == activeValtan) return false;
			BOSS_RUNTIME_PROFILE candidate = *activeValtan;
			mutate(candidate);
			compatibilityStatus.clear();
			return !CServerApp::Validate_ValtanHotReloadBaseProfile(
				activeValtan, &candidate, compatibilityStatus) &&
				std::string::npos !=
					compatibilityStatus.find("ENCOUNTER_RESET required");
		};
		BOSS_RUNTIME_PROFILE unchanged = nullptr == activeValtan ?
			BOSS_RUNTIME_PROFILE{} : *activeValtan;
		const bool acceptsUnchanged = nullptr != activeValtan &&
			CServerApp::Validate_ValtanHotReloadBaseProfile(
				activeValtan, &unchanged, compatibilityStatus) &&
			compatibilityStatus.empty();
		const bool rejectsEveryResetField =
			rejectsResetField([](BOSS_RUNTIME_PROFILE& value)
				{ ++value.iMaximumHp; }) &&
			rejectsResetField([](BOSS_RUNTIME_PROFILE& value)
				{ ++value.iMaximumHealthBars; }) &&
			rejectsResetField([](BOSS_RUNTIME_PROFILE& value)
				{ ++value.iAttackPower; }) &&
			rejectsResetField([](BOSS_RUNTIME_PROFILE& value)
				{ value.fCollisionRadius += 0.25f; }) &&
			rejectsResetField([](BOSS_RUNTIME_PROFILE& value)
				{ value.fEngageDistance += 0.25f; }) &&
			rejectsResetField([](BOSS_RUNTIME_PROFILE& value)
				{ value.fMoveSpeed += 0.25f; }) &&
			!CServerApp::Validate_ValtanHotReloadBaseProfile(
				activeValtan, nullptr, compatibilityStatus);
		tests.Require(
			acceptsUnchanged && rejectsEveryResetField,
			"Reject every live Valtan base-field change as ENCOUNTER_RESET instead of falsely committing HOT_RELOAD");
	}
	{
		auto revisionRoomStorage =
			std::make_unique<CGameRoom>(WORLD_ID::TRAINING_GROUND);
		CGameRoom& revisionRoom = *revisionRoomStorage;
		const GameplayDataRevision activeRevision =
			revisionRoom.m_GameplayCatalog.Get_ActiveRevision();
		GameplayDataRevision candidateRevision = activeRevision;
		candidateRevision.Bytes[0] ^= 0x80u;
		if (!candidateRevision.Is_Valid() ||
			candidateRevision == activeRevision)
		{
			candidateRevision.Bytes[0] = 1u;
		}

		constexpr SESSION_ID ACCEPTED_SESSION = 71001u;
		auto acceptedSession = std::make_shared<CClientSession>(
			ACCEPTED_SESSION, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		acceptedSession->m_isSendRunning.store(true);
		SERVER_PLAYER acceptedPlayer{};
		acceptedPlayer.iPlayerId = 71001u;
		acceptedPlayer.iNetEntityId = 71001u;
		SERVER_WORLD_ENTITY requiredPinEntity{};
		requiredPinEntity.PinnedDefinitionRevision = candidateRevision;
		revisionRoom.m_WorldEntities.push_back(requiredPinEntity);
		const bool sentAccepted = revisionRoom.Send_Accepted(
			acceptedSession, acceptedPlayer);
		S2C_ENTER_ACCEPTED decodedAccepted{};
		bool acceptedPins = false;
		if (sentAccepted && 1u == acceptedSession->m_OutboundFrames.size())
		{
			const auto& bytes =
				acceptedSession->m_OutboundFrames.front().Bytes;
			PACKET_HEADER header{};
			if (Read_Packet_Header(bytes, header) &&
				PACKET_TYPE::S2C_ENTER_ACCEPTED == header.ePacketType)
			{
				CPacketReader reader{ std::span<const std::uint8_t>(
					bytes.data() + PACKET_HEADER_BYTES,
					bytes.size() - PACKET_HEADER_BYTES) };
				acceptedPins = Read_Message(reader, decodedAccepted) &&
					0u == reader.Get_RemainingSize() &&
					activeRevision == decodedAccepted.ActiveGameplayRevision &&
					1u == decodedAccepted.RequiredPinnedGameplayRevisions.size() &&
					candidateRevision ==
						decodedAccepted.RequiredPinnedGameplayRevisions.front();
			}
		}
		revisionRoom.m_WorldEntities.pop_back();
		acceptedSession->Request_Close();
		tests.Require(acceptedPins,
			"Publish active and required gameplay revision identities on enter accepted");

		namespace fs = std::filesystem;
		std::vector<wchar_t> pathBuffer(32768u);
		fs::path dataRoot;
		const DWORD configuredLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", pathBuffer.data(),
			static_cast<DWORD>(pathBuffer.size()));
		if (0u != configuredLength && configuredLength < pathBuffer.size())
			dataRoot = fs::path(pathBuffer.data()).lexically_normal();
		else
		{
			const DWORD moduleLength = GetModuleFileNameW(nullptr,
				pathBuffer.data(), static_cast<DWORD>(pathBuffer.size()));
			if (0u != moduleLength && moduleLength < pathBuffer.size())
				dataRoot = fs::path(pathBuffer.data()).parent_path().parent_path() /
					L"DataFiles";
		}
		std::error_code pathError;
		const fs::path bootstrapPath = fs::canonical(
			dataRoot / L"Gameplay" / L"Gameplay.bootstrap", pathError);
		const fs::path domainHashRoot = fs::temp_directory_path() /
			(L"LostArkValtanDomainHashContract-" +
				std::to_wstring(GetCurrentProcessId()));
		std::error_code domainHashError;
		fs::remove_all(domainHashRoot, domainHashError);
		fs::create_directories(domainHashRoot, domainHashError);
		std::string bootstrapText;
		if (!pathError && !domainHashError)
		{
			std::ifstream source(bootstrapPath, std::ios::binary);
			bootstrapText.assign(std::istreambuf_iterator<char>(source),
				std::istreambuf_iterator<char>());
		}
		std::string valtanOnlyText = bootstrapText;
		std::string nonValtanText = bootstrapText;
		const std::string valtanNeedle =
			"DAMAGE\tdamage.valtan.swing\t220";
		const std::string nonValtanNeedle =
			"DAMAGE\tdamage.player.17000\t100";
		const std::size_t valtanNeedleAt = valtanOnlyText.find(valtanNeedle);
		const std::size_t nonValtanNeedleAt =
			nonValtanText.find(nonValtanNeedle);
		if (std::string::npos != valtanNeedleAt)
			valtanOnlyText.replace(
				valtanNeedleAt, valtanNeedle.size(),
				"DAMAGE\tdamage.valtan.swing\t221");
		if (std::string::npos != nonValtanNeedleAt)
			nonValtanText.replace(
				nonValtanNeedleAt, nonValtanNeedle.size(),
				"DAMAGE\tdamage.player.17000\t101");
		const fs::path valtanOnlyBootstrap =
			domainHashRoot / L"ValtanOnly.bootstrap";
		const fs::path nonValtanBootstrap =
			domainHashRoot / L"NonValtan.bootstrap";
		if (!bootstrapText.empty())
		{
			std::ofstream valtanOnly(
				valtanOnlyBootstrap, std::ios::binary | std::ios::trunc);
			valtanOnly.write(
				valtanOnlyText.data(),
				static_cast<std::streamsize>(valtanOnlyText.size()));
			std::ofstream nonValtan(
				nonValtanBootstrap, std::ios::binary | std::ios::trunc);
			nonValtan.write(
				nonValtanText.data(),
				static_cast<std::streamsize>(nonValtanText.size()));
		}
		GameplayDataRevision baseNonValtanRevision{};
		GameplayDataRevision valtanOnlyNonValtanRevision{};
		GameplayDataRevision changedNonValtanRevision{};
		std::string domainHashStatus;
		const bool domainHashContract =
			std::string::npos != valtanNeedleAt &&
			std::string::npos != nonValtanNeedleAt &&
			CServerApp::Build_NonValtanGameplayRevisionForAdmission(
				bootstrapPath, baseNonValtanRevision, domainHashStatus) &&
			CServerApp::Build_NonValtanGameplayRevisionForAdmission(
				valtanOnlyBootstrap, valtanOnlyNonValtanRevision,
				domainHashStatus) &&
			CServerApp::Build_NonValtanGameplayRevisionForAdmission(
				nonValtanBootstrap, changedNonValtanRevision,
				domainHashStatus) &&
			baseNonValtanRevision == valtanOnlyNonValtanRevision &&
			baseNonValtanRevision != changedNonValtanRevision;
		tests.Require(domainHashContract,
			"Allow repeated Valtan-only bootstrap changes while rejecting stale non-Valtan gameplay drift");
		fs::remove_all(domainHashRoot, domainHashError);
		auto baseGeneration = revisionRoom.Get_ActiveGameplayGeneration();
		auto candidateGeneration = std::make_shared<CGameplayCatalog>();
		const bool candidateLoaded = nullptr != baseGeneration && !pathError &&
			candidateGeneration->Load_FromBootstrap(
				bootstrapPath, activeRevision, candidateRevision);

		auto valtanRoom = std::make_shared<CGameRoom>(
			WORLD_ID::VALTAN_ARENA, baseGeneration);
		auto privateRoom = std::make_shared<CGameRoom>(
			WORLD_ID::CHARACTER_SELECT_ARENA, baseGeneration);
		auto pinFixture =
			std::make_unique<PINNED_MECHANIC_GENERATION_FIXTURE>();
		pinFixture->Boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		pinFixture->Boss.strArchetypeId = "BOSS_VALTAN";
		pinFixture->Boss.strEncounterId = "ENCOUNTER_VALTAN";
		pinFixture->Boss.iCurrentHp = 49125u;
		pinFixture->Boss.iMaximumHp = 60000u;
		pinFixture->Boss.iMaximumHealthBars = 160u;
		pinFixture->Boss.iLastEvaluatedHealthBar = 131u;
		pinFixture->Boss.iPhase = 1u;
		pinFixture->Boss.fPositionX = 151.f;
		pinFixture->Boss.fPositionY = 22.97f;
		pinFixture->Boss.fPositionZ = -122.f;
		pinFixture->Boss.fEngageDistance = 35.f;
		pinFixture->Boss.fMoveSpeed = 3.f;
		pinFixture->Boss.bIntroPatternConsumed = true;
		pinFixture->Boss.bAutomaticPatternSequenceAuditionOverride = true;
		pinFixture->Boss.PinnedDefinitionRevision = activeRevision;
		pinFixture->Target.iPlayerId = 71004u;
		pinFixture->Target.iNetEntityId = 71004u;
		pinFixture->Target.iCurrentHp = 10000u;
		pinFixture->Target.iMaximumHp = 10000u;
		pinFixture->Target.fPositionX = 151.f;
		pinFixture->Target.fPositionY = 22.97f;
		pinFixture->Target.fPositionZ = -128.f;
		pinFixture->Target.isCombatReady = true;
		pinFixture->Players.emplace(
			pinFixture->Target.iPlayerId, pinFixture->Target);
		pinFixture->Brain.Update(
			pinFixture->Boss, pinFixture->Players, *baseGeneration,
			valtanRoom->m_ServerNavigation, 1.f / 30.f, 10u, {},
			pinFixture->DamageEvents);
		pinFixture->Boss.iCurrentHp = 48750u;
		pinFixture->Brain.Update(
			pinFixture->Boss, pinFixture->Players, *baseGeneration,
			valtanRoom->m_ServerNavigation, 1.f / 30.f, 11u, {},
			pinFixture->DamageEvents);
		const auto queuedUnderActive = std::find_if(
			pinFixture->Boss.MechanicOccurrences.begin(),
			pinFixture->Boss.MechanicOccurrences.end(),
			[](const SERVER_BOSS_MECHANIC_OCCURRENCE& occurrence)
			{
				return "VALTAN_FLOOR_WIPE_130" == occurrence.strPatternId;
			});
		const bool capturedQueuedOccurrenceGeneration =
			pinFixture->Boss.MechanicOccurrences.end() != queuedUnderActive &&
			activeRevision == queuedUnderActive->PinnedDefinitionRevision &&
			SERVER_BOSS_MECHANIC_STATE::QUEUED == queuedUnderActive->eState &&
			!pinFixture->Boss.strPatternId.empty();
		valtanRoom->m_WorldEntities.push_back(pinFixture->Boss);

		const fs::path runtimePersistenceRoot =
			fs::temp_directory_path() /
			(L"lostark-server-runtime-active-contract-" +
			 std::to_wstring(::GetCurrentProcessId()));
		std::error_code runtimePersistenceError;
		fs::remove_all(runtimePersistenceRoot, runtimePersistenceError);
		auto appStorage = std::make_unique<CServerApp>();
		CServerApp& app = *appStorage;
		app.m_RuntimeActiveGameplayRootOverride = runtimePersistenceRoot;
		app.m_isRuntimeActivePersistenceEnabled = true;
		app.m_pActiveGameplayGeneration = baseGeneration;
		app.m_ActiveGameplayBootstrapContentRevision = activeRevision;
		app.m_ActiveNonValtanGameplayRevision = activeRevision;
		app.m_SharedGameRooms.emplace(WORLD_ID::VALTAN_ARENA, valtanRoom);
		constexpr SESSION_ID REQUESTER_SESSION = 71002u;
		constexpr SESSION_ID PARTICIPANT_SESSION = 71003u;
		auto requester = std::make_shared<CClientSession>(
			REQUESTER_SESSION, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		auto participant = std::make_shared<CClientSession>(
			PARTICIPANT_SESSION, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		requester->m_isSendRunning.store(true);
		participant->m_isSendRunning.store(true);
		app.m_Sessions.emplace(REQUESTER_SESSION, requester);
		app.m_Sessions.emplace(PARTICIPANT_SESSION, participant);
		CServerApp::SESSION_GAMEPLAY_BINDING requesterBinding{};
		requesterBinding.eWorldId = WORLD_ID::VALTAN_ARENA;
		requesterBinding.pSimulation = valtanRoom;
		CServerApp::SESSION_GAMEPLAY_BINDING participantBinding{};
		participantBinding.eWorldId = WORLD_ID::CHARACTER_SELECT_ARENA;
		participantBinding.iPrivateArenaOwnerSessionId = PARTICIPANT_SESSION;
		participantBinding.pSimulation = privateRoom;
		app.m_GameplayBindingBySessionId.emplace(
			REQUESTER_SESSION, requesterBinding);
		app.m_GameplayBindingBySessionId.emplace(
			PARTICIPANT_SESSION, participantBinding);
		app.m_CharacterSelectArenas.emplace(PARTICIPANT_SESSION, privateRoom);

		C2S_DATA_REVISION_PREPARE_REQUEST request{};
		request.iTransactionSequence = 71u;
		request.BaseRevision = activeRevision;
		request.CandidateRevision = candidateRevision;
		request.iRequiredPresentationLaneMask =
			GAMEPLAY_PRESENTATION_KNOWN_LANE_MASK;
		CServerApp::SERVER_CONTROL_EVENT begin{};
		begin.eKind =
			CServerApp::SERVER_CONTROL_EVENT_KIND::DATA_REVISION_REQUEST;
		begin.iSessionId = REQUESTER_SESSION;
		begin.RevisionRequest = request;
		begin.pCandidateGeneration = candidateGeneration;
		begin.BaseBootstrapContentRevision = activeRevision;
		begin.CandidateBootstrapContentRevision = activeRevision;
		begin.BaseNonValtanGameplayRevision = activeRevision;
		begin.CandidateNonValtanGameplayRevision = activeRevision;
		const bool queuedBegin = app.Queue_ServerControlEvent(std::move(begin));
		app.Advance_ServerControlTransactions();
		const bool preparedEveryClient = queuedBegin && candidateLoaded &&
			app.m_DataRevisionTransaction.Is_Active() &&
			1u == requester->m_OutboundFrames.size() &&
			1u == participant->m_OutboundFrames.size();

		const auto queueReady = [&app, &request](const SESSION_ID sessionId)
		{
			CServerApp::SERVER_CONTROL_EVENT event{};
			event.eKind =
				CServerApp::SERVER_CONTROL_EVENT_KIND::DATA_REVISION_RESPONSE;
			event.iSessionId = sessionId;
			event.RevisionResponse.iTransactionSequence =
				request.iTransactionSequence;
			event.RevisionResponse.CandidateRevision =
				request.CandidateRevision;
			event.RevisionResponse.eStatus =
				DATA_REVISION_PREPARE_STATUS::READY;
			event.RevisionResponse.iRequiredPresentationLaneMask =
				request.iRequiredPresentationLaneMask;
			event.RevisionResponse.iPreparedPresentationLaneMask =
				request.iRequiredPresentationLaneMask;
			return app.Queue_ServerControlEvent(std::move(event));
		};
		const bool queuedReady = queueReady(REQUESTER_SESSION) &&
			queueReady(PARTICIPANT_SESSION);
		app.Advance_ServerControlTransactions();
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION packagedRuntime{};
		packagedRuntime.eSource =
			RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE;
		packagedRuntime.Revision = activeRevision;
		packagedRuntime.BootstrapContentRevision = activeRevision;
		packagedRuntime.NonValtanGameplayRevision = activeRevision;
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION interruptedCandidateRuntime{};
		interruptedCandidateRuntime.eSource =
			RUNTIME_GAMEPLAY_GENERATION_SOURCE::CANDIDATE;
		interruptedCandidateRuntime.Revision = candidateRevision;
		interruptedCandidateRuntime.BootstrapContentRevision = activeRevision;
		interruptedCandidateRuntime.NonValtanGameplayRevision = activeRevision;
		const fs::path interruptedRuntimeRoot = fs::temp_directory_path() /
			(L"lostark-server-runtime-interrupted-contract-" +
			 std::to_wstring(::GetCurrentProcessId()));
		std::error_code interruptedRuntimeError;
		fs::remove_all(interruptedRuntimeRoot, interruptedRuntimeError);
		std::string interruptedRuntimeStatus;
		const bool persistedBeforeJournalCleanup =
			CServerApp::Persist_RuntimeGameplayActivation(
				interruptedRuntimeRoot, 7001u, packagedRuntime,
				interruptedCandidateRuntime, interruptedRuntimeStatus);
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION recoveredInterruptedRuntime{};
		bool recoveredInterruptedPointer = false;
		const bool recoveredPromotedPointerWithJournal =
			persistedBeforeJournalCleanup &&
			CServerApp::Recover_RuntimeActiveGameplayPointer(
				interruptedRuntimeRoot, packagedRuntime,
				recoveredInterruptedRuntime, recoveredInterruptedPointer,
				interruptedRuntimeStatus) &&
			recoveredInterruptedPointer &&
			candidateRevision == recoveredInterruptedRuntime.Revision;
		CServerApp::Complete_RuntimeGameplayActivation(interruptedRuntimeRoot);
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION recoveredAfterJournalCleanup{};
		bool recoveredAfterJournalCleanupPointer = false;
		const bool recoveredCandidateAfterRestartCleanup =
			CServerApp::Recover_RuntimeActiveGameplayPointer(
				interruptedRuntimeRoot, packagedRuntime,
				recoveredAfterJournalCleanup,
				recoveredAfterJournalCleanupPointer,
				interruptedRuntimeStatus) &&
			recoveredAfterJournalCleanupPointer &&
			candidateRevision == recoveredAfterJournalCleanup.Revision;
		fs::remove_all(interruptedRuntimeRoot, interruptedRuntimeError);
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION recoveredRuntime{};
		bool recoveredPointer = false;
		std::string runtimeRecoveryStatus;
		const bool recoveredCommittedCandidate =
			CServerApp::Recover_RuntimeActiveGameplayPointer(
				runtimePersistenceRoot, packagedRuntime, recoveredRuntime,
				recoveredPointer, runtimeRecoveryStatus) && recoveredPointer &&
			RUNTIME_GAMEPLAY_GENERATION_SOURCE::CANDIDATE ==
				recoveredRuntime.eSource &&
			candidateRevision == recoveredRuntime.Revision &&
			activeRevision == recoveredRuntime.BootstrapContentRevision &&
			activeRevision == recoveredRuntime.NonValtanGameplayRevision;
		const bool committedEveryRoom = queuedReady &&
			!app.m_DataRevisionTransaction.Is_Active() &&
			candidateRevision ==
				app.m_pActiveGameplayGeneration->Get_ActiveRevision() &&
			candidateRevision == valtanRoom->Get_ActiveGameplayGeneration()->
				Get_ActiveRevision() &&
			candidateRevision == privateRoom->Get_ActiveGameplayGeneration()->
				Get_ActiveRevision() &&
			nullptr != valtanRoom->Resolve_GameplayGeneration(activeRevision) &&
			2u == requester->m_OutboundFrames.size() &&
			2u == participant->m_OutboundFrames.size();
		tests.Require(
			capturedQueuedOccurrenceGeneration && preparedEveryClient &&
			committedEveryRoom && recoveredCommittedCandidate &&
			recoveredPromotedPointerWithJournal &&
			recoveredCandidateAfterRestartCleanup,
			"Commit one candidate across every room and durable restart pointer only after every bound client READY");

		/* Model an A occurrence that held the boss at 158 bars while the process
		committed B. The first idle B evaluation must reconcile the untriggered
		159 mechanic even though there can no longer be a 160 -> 159 crossing.
		No target deliberately delays consumption for one tick, proving the queued
		occurrence itself, rather than the idle boss pin, owns B. */
		SERVER_WORLD_ENTITY& reconcileBoss = pinFixture->ReconcileBoss;
		reconcileBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		reconcileBoss.strArchetypeId = "BOSS_VALTAN";
		reconcileBoss.strEncounterId = "ENCOUNTER_VALTAN";
		reconcileBoss.iCurrentHp = 59250u;
		reconcileBoss.iMaximumHp = 60000u;
		reconcileBoss.iMaximumHealthBars = 160u;
		reconcileBoss.iLastEvaluatedHealthBar = 158u;
		reconcileBoss.iLastHealthMechanicGenerationEpoch = 1u;
		reconcileBoss.iPhase = 1u;
		reconcileBoss.fPositionX = 151.f;
		reconcileBoss.fPositionY = 22.97f;
		reconcileBoss.fPositionZ = -122.f;
		reconcileBoss.fEngageDistance = 35.f;
		reconcileBoss.fMoveSpeed = 3.f;
		reconcileBoss.bIntroPatternConsumed = true;
		reconcileBoss.bAutomaticPatternSequenceAuditionOverride = true;
		reconcileBoss.PinnedDefinitionRevision = activeRevision;
		pinFixture->Brain.Update(
			reconcileBoss, pinFixture->ReconcilePlayers, *candidateGeneration,
			valtanRoom->m_ServerNavigation, 1.f / 30.f, 20u, {},
			pinFixture->DamageEvents, candidateGeneration.get(), 2u);
		const auto reconciledOccurrence = std::find_if(
			reconcileBoss.MechanicOccurrences.begin(),
			reconcileBoss.MechanicOccurrences.end(),
			[](const SERVER_BOSS_MECHANIC_OCCURRENCE& occurrence)
			{
				return "VALTAN_ARMOR_BREAK_OPENING" == occurrence.strPatternId;
			});
		const bool reconciledBelowThresholdWithoutTarget =
			reconcileBoss.MechanicOccurrences.end() != reconciledOccurrence &&
			SERVER_BOSS_MECHANIC_STATE::QUEUED ==
				reconciledOccurrence->eState &&
			candidateRevision ==
				reconciledOccurrence->PinnedDefinitionRevision &&
			reconcileBoss.strPatternId.empty();
		pinFixture->ReconcilePlayers.emplace(
			pinFixture->Target.iPlayerId, pinFixture->Target);
		pinFixture->Brain.Update(
			reconcileBoss, pinFixture->ReconcilePlayers, *candidateGeneration,
			valtanRoom->m_ServerNavigation, 1.f / 30.f, 21u, {},
			pinFixture->DamageEvents, candidateGeneration.get(), 2u);
		const bool delayedReconciledOccurrenceUsesCandidate =
			reconcileBoss.MechanicOccurrences.end() != reconciledOccurrence &&
			SERVER_BOSS_MECHANIC_STATE::ACTIVE ==
				reconciledOccurrence->eState &&
			"VALTAN_ARMOR_BREAK_OPENING" == reconcileBoss.strPatternId &&
			candidateRevision == reconcileBoss.PinnedDefinitionRevision;
		tests.Require(
			reconciledBelowThresholdWithoutTarget &&
			delayedReconciledOccurrenceUsesCandidate,
			"Reconcile an untriggered threshold below current HP on generation change and pin delayed consumption to the evaluating catalog");

		/* Keep occurrence execution on A while changing the non-transition armour
		break from 159 to 158 in B. Falling to 159 must not be interpreted by A's
		threshold table; the exact 158 crossing must queue one B occurrence and
		pin B. The 109 phase boundary is independently topology-locked below. */
		const fs::path thresholdRoot = fs::temp_directory_path() /
			(L"LostArkValtanThresholdGenerationContract-" +
			 std::to_wstring(GetCurrentProcessId()));
		std::error_code thresholdError;
		fs::remove_all(thresholdRoot, thresholdError);
		fs::create_directories(thresholdRoot, thresholdError);
		std::string thresholdBootstrapText = bootstrapText;
		const std::string thresholdA =
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_ARMOR_BREAK_OPENING\tvaltan.mechanic.armor-break-opening\tHEALTH_BAR\t0\t0\t159\t1\t0\t0\t0\t100\t4\tANY\tANY\t0";
		const std::string thresholdB =
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_ARMOR_BREAK_OPENING\tvaltan.mechanic.armor-break-opening\tHEALTH_BAR\t0\t0\t158\t1\t0\t0\t0\t100\t4\tANY\tANY\t0";
		const std::size_t thresholdRowAt =
			thresholdBootstrapText.find(thresholdA);
		if (std::string::npos != thresholdRowAt)
			thresholdBootstrapText.replace(
				thresholdRowAt, thresholdA.size(), thresholdB);
		const fs::path thresholdBootstrap =
			thresholdRoot / L"Gameplay.bootstrap";
		if (!thresholdError && std::string::npos != thresholdRowAt)
		{
			std::ofstream stream(
				thresholdBootstrap, std::ios::binary | std::ios::trunc);
			stream.write(thresholdBootstrapText.data(),
				static_cast<std::streamsize>(thresholdBootstrapText.size()));
		}
		GameplayDataRevision thresholdBootstrapRevision{};
		GameplayDataRevision thresholdDefinitionRevision = candidateRevision;
		thresholdDefinitionRevision.Bytes[8] ^= 0x40u;
		if (!thresholdDefinitionRevision.Is_Valid() ||
			thresholdDefinitionRevision == activeRevision ||
			thresholdDefinitionRevision == candidateRevision)
		{
			thresholdDefinitionRevision.Bytes[9] ^= 1u;
		}
		std::string thresholdStatus;
		auto thresholdGeneration = std::make_shared<CGameplayCatalog>();
		const bool thresholdAdmissionInputsReady = !thresholdError &&
			std::string::npos != thresholdRowAt;
		bool thresholdGenerationLoaded = false;
		if (thresholdAdmissionInputsReady)
		{
			// Hashing and catalog validation have sizeable but bounded call trees. Keep
			// this synthetic second generation off the already-large contract-test
			// driver frame so the executable remains valid on the default 1 MiB
			// Windows thread stack used by production Server.exe.
			struct THRESHOLD_GENERATION_LOAD_CONTEXT final
			{
				std::shared_ptr<CGameplayCatalog>* pGeneration = nullptr;
				const fs::path* pBootstrap = nullptr;
				GameplayDataRevision* pBootstrapRevision = nullptr;
				const GameplayDataRevision* pDefinitionRevision = nullptr;
				std::string* pStatus = nullptr;
				bool isLoaded = false;
			};
			THRESHOLD_GENERATION_LOAD_CONTEXT loadContext{
				&thresholdGeneration, &thresholdBootstrap,
				&thresholdBootstrapRevision, &thresholdDefinitionRevision,
				&thresholdStatus, false };
			const auto loadThresholdGeneration = [](void* opaque)
			{
				THRESHOLD_GENERATION_LOAD_CONTEXT& context =
					*static_cast<THRESHOLD_GENERATION_LOAD_CONTEXT*>(opaque);
				context.isLoaded =
					CServerApp::Hash_GameplayFileForAdmission(
						*context.pBootstrap, *context.pBootstrapRevision,
						*context.pStatus) &&
					(*context.pGeneration)->Load_FromBootstrap(
						*context.pBootstrap, *context.pBootstrapRevision,
						*context.pDefinitionRevision);
			};
			thresholdGenerationLoaded = Run_WithContractWorkerStack(
				loadThresholdGeneration, &loadContext) && loadContext.isLoaded;
		}
		const auto findHealthThreshold = [](const CGameplayCatalog& source,
			const std::string_view patternId)
		{
			const auto* definitions =
				source.Find_BossPatterns("ENCOUNTER_VALTAN");
			if (nullptr == definitions) return std::uint32_t{ 0u };
			const auto found = std::find_if(
				definitions->begin(), definitions->end(),
				[patternId](const BOSS_PATTERN_DEFINITION& definition)
				{
					return definition.strPatternId == patternId;
				});
			return definitions->end() == found ?
				std::uint32_t{ 0u } : found->iTriggerHealthBar;
		};
		tests.Require(
			thresholdGenerationLoaded &&
			159u == findHealthThreshold(*baseGeneration,
				"VALTAN_ARMOR_BREAK_OPENING") &&
			158u == findHealthThreshold(*thresholdGeneration,
				"VALTAN_ARMOR_BREAK_OPENING"),
			"Stage exact A159 and B158 non-transition threshold generations for routing");
		SERVER_WORLD_ENTITY& thresholdBoss = pinFixture->ThresholdBoss;
		thresholdBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		thresholdBoss.strArchetypeId = "BOSS_VALTAN";
		thresholdBoss.strEncounterId = "ENCOUNTER_VALTAN";
		thresholdBoss.iCurrentHp = 59625u;
		thresholdBoss.iMaximumHp = 60000u;
		thresholdBoss.iMaximumHealthBars = 160u;
		thresholdBoss.iLastEvaluatedHealthBar = 160u;
		thresholdBoss.iLastHealthMechanicGenerationEpoch = 1u;
		thresholdBoss.iPhase = 1u;
		thresholdBoss.fPositionX = 151.f;
		thresholdBoss.fPositionY = 22.97f;
		thresholdBoss.fPositionZ = -122.f;
		thresholdBoss.fEngageDistance = 35.f;
		thresholdBoss.fMoveSpeed = 3.f;
		thresholdBoss.bIntroPatternConsumed = true;
		thresholdBoss.bAutomaticPatternSequenceAuditionOverride = true;
		thresholdBoss.PinnedDefinitionRevision = activeRevision;
		pinFixture->ReconcilePlayers.clear();
		if (thresholdGenerationLoaded)
		{
			const auto* definitions = thresholdGeneration->Find_BossPatterns(
				"ENCOUNTER_VALTAN");
			if (nullptr != definitions)
			{
				for (const BOSS_PATTERN_DEFINITION& definition : *definitions)
				{
					if (BOSS_PATTERN_SELECTION::HEALTH_BAR !=
						definition.eSelection ||
						"VALTAN_ARMOR_BREAK_OPENING" == definition.strPatternId)
						continue;
					SERVER_BOSS_MECHANIC_OCCURRENCE completed{};
					completed.strPatternId = definition.strPatternId;
					completed.PinnedDefinitionRevision =
						thresholdDefinitionRevision;
					completed.eState =
						SERVER_BOSS_MECHANIC_STATE::COMPLETED;
					completed.iTriggerHealthBar =
						definition.iTriggerHealthBar;
					thresholdBoss.MechanicOccurrences.push_back(
						std::move(completed));
				}
			}
			pinFixture->Brain.Update(
				thresholdBoss, pinFixture->ReconcilePlayers, *baseGeneration,
				valtanRoom->m_ServerNavigation, 1.f / 30.f, 30u, {},
				pinFixture->DamageEvents, thresholdGeneration.get(), 2u);
		}
		const auto countThresholdOccurrence = [&thresholdBoss]()
		{
			return static_cast<std::size_t>(std::count_if(
				thresholdBoss.MechanicOccurrences.begin(),
				thresholdBoss.MechanicOccurrences.end(),
				[](const SERVER_BOSS_MECHANIC_OCCURRENCE& occurrence)
				{
					return "VALTAN_ARMOR_BREAK_OPENING" ==
						occurrence.strPatternId;
				}));
		};
		const bool oldOnlyCrossingDidNotQueue =
			thresholdGenerationLoaded && 0u == countThresholdOccurrence() &&
			159u == thresholdBoss.iLastEvaluatedHealthBar &&
			2u == thresholdBoss.iLastHealthMechanicGenerationEpoch;
		tests.Require(oldOnlyCrossingDidNotQueue,
			"Ignore the old A159-only crossing while active B owns threshold evaluation");
		thresholdBoss.iCurrentHp = 59250u;
		if (thresholdGenerationLoaded)
		{
			pinFixture->Brain.Update(
				thresholdBoss, pinFixture->ReconcilePlayers, *baseGeneration,
				valtanRoom->m_ServerNavigation, 1.f / 30.f, 31u, {},
				pinFixture->DamageEvents, thresholdGeneration.get(), 2u);
		}
		const auto thresholdOccurrence = std::find_if(
			thresholdBoss.MechanicOccurrences.begin(),
			thresholdBoss.MechanicOccurrences.end(),
			[](const SERVER_BOSS_MECHANIC_OCCURRENCE& occurrence)
			{
				return "VALTAN_ARMOR_BREAK_OPENING" == occurrence.strPatternId;
			});
		const bool activeCrossingQueuedExactCandidate =
			1u == countThresholdOccurrence() &&
			thresholdBoss.MechanicOccurrences.end() != thresholdOccurrence &&
			158u == thresholdOccurrence->iTriggerHealthBar &&
			thresholdDefinitionRevision ==
				thresholdOccurrence->PinnedDefinitionRevision;
		tests.Require(
			activeCrossingQueuedExactCandidate,
			"Evaluate new threshold crossings from active B while an old A occurrence catalog advances independently");
		fs::remove_all(thresholdRoot, thresholdError);

		const auto replaceBootstrapRow = [](
			std::string& text, const std::string_view before,
			const std::string_view after)
		{
			const std::size_t at = text.find(before);
			if (std::string::npos == at || std::string::npos !=
				text.find(before, at + before.size()))
			{
				return false;
			}
			text.replace(at, before.size(), after);
			return true;
		};
		const auto removeBootstrapRow = [](
			std::string& text, const std::string_view row)
		{
			const std::size_t at = text.find(row);
			if (std::string::npos == at ||
				(0u != at && '\n' != text[at - 1u]) ||
				std::string::npos != text.find(row, at + row.size()))
			{
				return false;
			}
			const std::size_t lineEnd = text.find('\n', at);
			text.erase(at, std::string::npos == lineEnd ?
				text.size() - at : lineEnd - at + 1u);

			const std::size_t headerEnd = text.find('\n');
			const std::size_t countTab = text.rfind('\t', headerEnd);
			std::size_t countEnd = headerEnd;
			if (std::string::npos == headerEnd ||
				std::string::npos == countTab || countTab + 1u >= countEnd)
			{
				return false;
			}
			if (0u != countEnd && '\r' == text[countEnd - 1u])
				--countEnd;
			const unsigned long count = std::stoul(text.substr(
				countTab + 1u, countEnd - countTab - 1u));
			if (0u == count) return false;
			text.replace(
				countTab + 1u, countEnd - countTab - 1u,
				std::to_string(count - 1u));
			return true;
		};
		const auto loadBootstrapVariant = [&candidateRevision](
			const std::wstring_view suffix,
			const std::string& bytes,
			std::shared_ptr<CGameplayCatalog> generation = nullptr)
			-> std::shared_ptr<CGameplayCatalog>
		{
			const fs::path root = fs::temp_directory_path() /
				(L"LostArkGameplayV19Variant-" +
				 std::to_wstring(GetCurrentProcessId()) + L"-" +
				 std::wstring(suffix));
			std::error_code error;
			fs::remove_all(root, error);
			error.clear();
			fs::create_directories(root, error);
			const fs::path bootstrap = root / L"Gameplay.bootstrap";
			if (!error)
			{
				std::ofstream stream(
					bootstrap, std::ios::binary | std::ios::trunc);
				stream.write(bytes.data(),
					static_cast<std::streamsize>(bytes.size()));
				if (!stream.good()) error = std::make_error_code(
					std::errc::io_error);
			}
			const fs::path canonical = error ? fs::path{} :
				fs::canonical(bootstrap, error);
			if (nullptr == generation)
				generation = std::make_shared<CGameplayCatalog>();
			GameplayDataRevision bootstrapRevision{};
			std::string status;
			struct VARIANT_LOAD_CONTEXT final
			{
				std::shared_ptr<CGameplayCatalog>* pGeneration = nullptr;
				const fs::path* pBootstrap = nullptr;
				GameplayDataRevision* pBootstrapRevision = nullptr;
				const GameplayDataRevision* pDefinitionRevision = nullptr;
				std::string* pStatus = nullptr;
				bool isLoaded = false;
			};
			VARIANT_LOAD_CONTEXT context{
				&generation, &canonical, &bootstrapRevision,
				&candidateRevision, &status, false };
			const auto load = [](void* opaque)
			{
				VARIANT_LOAD_CONTEXT& context =
					*static_cast<VARIANT_LOAD_CONTEXT*>(opaque);
				context.isLoaded =
					CServerApp::Hash_GameplayFileForAdmission(
						*context.pBootstrap, *context.pBootstrapRevision,
						*context.pStatus) &&
					(*context.pGeneration)->Load_FromBootstrap(
						*context.pBootstrap, *context.pBootstrapRevision,
						*context.pDefinitionRevision);
			};
			const bool loaded = !error && nullptr != generation &&
				Run_WithContractWorkerStack(load, &context) && context.isLoaded;
			fs::remove_all(root, error);
			return loaded ? generation : nullptr;
		};
		std::string truncatedMotionBootstrap = bootstrapText;
		const std::string motionRowPrefix =
			"PATTERNMOTION\tENCOUNTER_VALTAN\tVALTAN_HIGH_JUMP\t";
		const std::size_t motionRowAt =
			truncatedMotionBootstrap.find(motionRowPrefix);
		const std::size_t motionRowEnd = std::string::npos == motionRowAt ?
			std::string::npos :
			truncatedMotionBootstrap.find('\n', motionRowAt);
		std::size_t tenthMotionTab = motionRowAt;
		for (std::uint32_t tabIndex = 0u;
			tabIndex < 10u && std::string::npos != tenthMotionTab;
			++tabIndex)
		{
			tenthMotionTab = truncatedMotionBootstrap.find(
				'\t', 0u == tabIndex ? tenthMotionTab : tenthMotionTab + 1u);
		}
		const bool madeTruncatedMotionRow =
			std::string::npos != motionRowAt &&
			std::string::npos != motionRowEnd &&
			std::string::npos != tenthMotionTab &&
			tenthMotionTab < motionRowEnd;
		if (madeTruncatedMotionRow)
		{
			truncatedMotionBootstrap.erase(
				tenthMotionTab, motionRowEnd - tenthMotionTab);
		}
		auto motionRollbackCatalog = std::make_shared<CGameplayCatalog>();
		const bool motionRollbackBaselineLoaded =
			motionRollbackCatalog->Load();
		const GameplayDataRevision motionRollbackRevision =
			motionRollbackCatalog->Get_ActiveRevision();
		const bool rejectedTruncatedMotionRow =
			madeTruncatedMotionRow && motionRollbackBaselineLoaded &&
			nullptr == loadBootstrapVariant(
				L"truncated-pattern-motion", truncatedMotionBootstrap,
				motionRollbackCatalog) &&
			motionRollbackRevision ==
				motionRollbackCatalog->Get_ActiveRevision() &&
			std::string::npos != motionRollbackCatalog->Get_Status().find(
				"Boss pattern motion row is invalid");
		tests.Require(
			rejectedTruncatedMotionRow,
			"Reject a truncated PATTERNMOTION row and preserve the active gameplay generation");

		const std::string sequenceHeader =
			"PATTERNSEQUENCE\tENCOUNTER_VALTAN\t"
			"sequence.valtan.server-authored.v1\t"
			"ORDERED_ONCE_THEN_IDLE\t1000\t29";
		const std::vector<std::string> sequenceSteps{
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t0\tVALTAN_ENTRANCE_CINEMATIC",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t1\tVALTAN_WHIRLWIND",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t2\tVALTAN_FOUR_SLASH",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t3\tVALTAN_FIST_IN_OUT",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t4\tVALTAN_HIGH_JUMP",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t5\tVALTAN_FLOOR_WIPE_130",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t6\tVALTAN_DASH_CHARGE",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t7\tVALTAN_ARENA_BREAK_109",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t8\tVALTAN_SIX_PIZZA_106",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t9\tVALTAN_ATTACK_WHIRLWIND",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t10\tVALTAN_CHARGE",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t11\tVALTAN_SEQUENCE_FOUR",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t12\tVALTAN_ROAR_CHARGE",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t13\tVALTAN_SEQUENCE_RUSH",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t14\tVALTAN_THREE",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t15\tVALTAN_TERRAIN_DESTRUCTION_3_OCLOCK",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t16\tVALTAN_TERRAIN_DESTRUCTION_9_OCLOCK",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t17\tVALTAN_TERRAIN_DESTRUCTION",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t18\tVALTAN_SEQUENCE_FRONT_BACK_FRONT",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t19\tVALTAN_WARP",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t20\tVALTAN_SEQUENCE_TWOHAND",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t21\tVALTAN_SEQUENCE_WHIRLWIND",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t22\tVALTAN_TRASH",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t23\tVALTAN_TRASH_CATCH_SUCCESS",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t24\tVALTAN_TRASH_CATCH_FAIL",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t25\tVALTAN_TRASH_CATCH_IF",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t26\tVALTAN_CATCH_BREATH",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t27\tVALTAN_COUNTER",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.server-authored.v1\t28\tVALTAN_CHARGE_2" };
		std::string missingSequenceBootstrap = bootstrapText;
		bool removedSequence = removeBootstrapRow(
			missingSequenceBootstrap, sequenceHeader);
		for (const std::string& row : sequenceSteps)
		{
			removedSequence = removeBootstrapRow(
				missingSequenceBootstrap, row) && removedSequence;
		}
		std::string missingSequenceStepBootstrap = bootstrapText;
		const bool removedFinalSequenceStep = removeBootstrapRow(
			missingSequenceStepBootstrap, sequenceSteps.back());
		std::string invalidSequencePursuitBootstrap = bootstrapText;
		const bool madeInvalidSequencePursuit = replaceBootstrapRow(
			invalidSequencePursuitBootstrap, sequenceHeader,
			"PATTERNSEQUENCE\tENCOUNTER_VALTAN\t"
			"sequence.valtan.server-authored.v1\t"
			"ORDERED_ONCE_THEN_IDLE\t0\t29");
		std::string duplicateSequenceStepBootstrap = bootstrapText;
		const bool madeDuplicateSequenceStep = replaceBootstrapRow(
			duplicateSequenceStepBootstrap, sequenceSteps[1u],
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\t"
			"sequence.valtan.server-authored.v1\t1\tVALTAN_ENTRANCE_CINEMATIC");
		std::string nonContiguousSequenceBootstrap = bootstrapText;
		const bool madeNonContiguousSequence = replaceBootstrapRow(
			nonContiguousSequenceBootstrap, sequenceSteps[2u],
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\t"
			"sequence.valtan.server-authored.v1\t3\tVALTAN_FIST_IN_OUT");
		std::string unknownSequenceStepBootstrap = bootstrapText;
		const bool madeUnknownSequenceStep = replaceBootstrapRow(
			unknownSequenceStepBootstrap, sequenceSteps[1u],
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\t"
			"sequence.valtan.server-authored.v1\t1\tVALTAN_UNKNOWN_SEQUENCE_STEP");
		const std::string partDamageRow =
			"PATTERNSTAGEPARTDAMAGE\tENCOUNTER_VALTAN\tVALTAN_DASH_CHARGE\t"
			"valtan.attack.dash-charge.groggy\tDESTROY_FIRST_ELIGIBLE";
		std::string invalidPartDamageBootstrap = bootstrapText;
		const bool madeInvalidPartDamage = replaceBootstrapRow(
			invalidPartDamageBootstrap, partDamageRow,
			"PATTERNSTAGEPARTDAMAGE\tENCOUNTER_VALTAN\tVALTAN_DASH_CHARGE\t"
			"valtan.attack.dash-charge.groggy\tUNKNOWN");
		const std::string counterProxyRow =
			"PATTERNSTAGECOUNTERPROXY\tENCOUNTER_VALTAN\tVALTAN_TRASH\t"
			"valtan.sequence.center-trash-rush-if.step-06\t1\t-1.5\t2.25";
		std::string invalidCounterProxyBootstrap = bootstrapText;
		const bool madeInvalidCounterProxy = replaceBootstrapRow(
			invalidCounterProxyBootstrap, counterProxyRow,
			"PATTERNSTAGECOUNTERPROXY\tENCOUNTER_VALTAN\tVALTAN_TRASH\t"
			"valtan.sequence.center-trash-rush-if.step-06\t1\t-1.5\t21");
		const std::string rearTargetPolicyRow =
			"PATTERNPOLICY\tENCOUNTER_VALTAN\tVALTAN_CATCH_BREATH\t"
			"NORMAL\t1\t3\tLOCK_RANDOM_ALIVE_BEHIND_ON_START\t"
			"LOCK_FACING_ON_START";
		std::string invalidRearTargetPolicyBootstrap = bootstrapText;
		const bool madeInvalidRearTargetPolicy = replaceBootstrapRow(
			invalidRearTargetPolicyBootstrap, rearTargetPolicyRow,
			"PATTERNPOLICY\tENCOUNTER_VALTAN\tVALTAN_CATCH_BREATH\t"
			"NORMAL\t1\t3\tLOCK_RANDOM_ALIVE_BEHIND_ON_START_BROKEN\t"
			"LOCK_FACING_ON_START");
		const auto rejectsSequenceVariant = [
			&loadBootstrapVariant](const std::wstring_view suffix,
				const std::string& bytes, const std::string_view statusNeedle)
		{
			auto rollbackCatalog = std::make_shared<CGameplayCatalog>();
			if (!rollbackCatalog->Load()) return false;
			const GameplayDataRevision before =
				rollbackCatalog->Get_ActiveRevision();
			return nullptr == loadBootstrapVariant(
					suffix, bytes, rollbackCatalog) &&
				before == rollbackCatalog->Get_ActiveRevision() &&
				std::string::npos != rollbackCatalog->Get_Status().find(
					statusNeedle);
		};
		tests.Require(
			removedSequence && removedFinalSequenceStep &&
			madeInvalidSequencePursuit &&
			madeDuplicateSequenceStep && madeNonContiguousSequence &&
			madeUnknownSequenceStep &&
			rejectsSequenceVariant(
				L"sequence-missing", missingSequenceBootstrap,
				"exclusive pattern sequence") &&
			rejectsSequenceVariant(
				L"sequence-final-step-missing", missingSequenceStepBootstrap,
				"tagged shape is incomplete") &&
			rejectsSequenceVariant(
				L"sequence-invalid-pursuit", invalidSequencePursuitBootstrap,
				"sequence row is invalid") &&
			rejectsSequenceVariant(
				L"sequence-duplicate", duplicateSequenceStepBootstrap,
				"ordered contract") &&
			rejectsSequenceVariant(
				L"sequence-non-contiguous", nonContiguousSequenceBootstrap,
				"ordered contract") &&
			rejectsSequenceVariant(
				L"sequence-unknown-step", unknownSequenceStepBootstrap,
				"ordered contract"),
			"Reject missing, invalid-pursuit, incomplete, duplicate, non-contiguous, or unknown Valtan sequence rows without replacing the active generation");
		tests.Require(
			madeInvalidPartDamage && madeInvalidCounterProxy &&
			madeInvalidRearTargetPolicy &&
			rejectsSequenceVariant(
				L"stage-part-damage-invalid", invalidPartDamageBootstrap,
				"part-damage row is invalid") &&
			rejectsSequenceVariant(
				L"stage-counter-proxy-invalid", invalidCounterProxyBootstrap,
				"counter-proxy row is invalid") &&
			rejectsSequenceVariant(
				L"rear-target-policy-invalid", invalidRearTargetPolicyBootstrap,
				"pattern policy row is invalid"),
			"Reject invalid Dash part-damage, trash counter-proxy, and rear-target policy refinements transactionally");

		constexpr std::string_view AUDITION_PATTERN_ID =
			"VALTAN_TEST_ANIMATION_AUDITION";
		constexpr std::string_view AUDITION_STAGE_ACTION_ID =
			"valtan.test.animation-audition.step-01";
		const auto appendAuditionPattern = [](
			std::string text,
			const std::string_view patternRow,
			const std::string_view category)
		{
			const std::size_t headerEnd = text.find('\n');
			if (std::string::npos == headerEnd)
				return std::string{};
			const std::size_t rowCountTab = text.rfind('\t', headerEnd);
			std::size_t rowCountEnd = headerEnd;
			if (0u != rowCountEnd && '\r' == text[rowCountEnd - 1u])
				--rowCountEnd;
			if (std::string::npos == rowCountTab ||
				rowCountTab + 1u >= rowCountEnd)
			{
				return std::string{};
			}
			const unsigned long rowCount = std::stoul(text.substr(
				rowCountTab + 1u, rowCountEnd - rowCountTab - 1u));
			text.replace(
				rowCountTab + 1u, rowCountEnd - rowCountTab - 1u,
				std::to_string(rowCount + 5u));
			if (!text.empty() && '\n' != text.back()) text.push_back('\n');
			text.append(patternRow);
			text.append(
				"\nPATTERNPOLICY\tENCOUNTER_VALTAN\t"
				"VALTAN_TEST_ANIMATION_AUDITION\t");
			text.append(category);
			text.append(
				"\t1\t3\tNONE\tNONE"
				"\nPATTERNSOURCE\tENCOUNTER_VALTAN\t"
				"VALTAN_TEST_ANIMATION_AUDITION\t420638\t0\t0\t0\t0\t0\t0"
				"\nPATTERNSTAGE\tENCOUNTER_VALTAN\t"
				"VALTAN_TEST_ANIMATION_AUDITION\t0\tSTEP_01\t"
				"valtan.test.animation-audition.step-01\tACTIVE\t100\tNONE\t"
				"0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0"
				"\nPATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\t"
				"VALTAN_TEST_ANIMATION_AUDITION\t"
				"valtan.test.animation-audition.step-01\tTIMEOUT\t-\n");
			return text;
		};
		const std::string validAuditionRow =
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST_ANIMATION_AUDITION\t"
			"valtan.test.animation-audition\tAUDITION_ONLY\t"
			"0\t0\t0\t0\t0\t0\t0\t1\t1\tANY\tANY\t0";
		const std::string validAuditionBootstrap = appendAuditionPattern(
			bootstrapText, validAuditionRow, "NORMAL");
		const std::shared_ptr<CGameplayCatalog> auditionCatalog =
			loadBootstrapVariant(L"audition-only", validAuditionBootstrap);
		const std::string invalidAuditionHealthBootstrap = appendAuditionPattern(
			bootstrapText,
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST_ANIMATION_AUDITION\t"
			"valtan.test.animation-audition\tAUDITION_ONLY\t"
			"1\t160\t0\t0\t0\t0\t0\t1\t1\tANY\tANY\t0",
			"NORMAL");
		const std::string invalidAuditionTriggerBootstrap = appendAuditionPattern(
			bootstrapText,
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST_ANIMATION_AUDITION\t"
			"valtan.test.animation-audition\tAUDITION_ONLY\t"
			"0\t0\t100\t1\t0\t0\t0\t1\t1\tANY\tANY\t0",
			"NORMAL");
		const std::string invalidAuditionWeightBootstrap = appendAuditionPattern(
			bootstrapText,
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST_ANIMATION_AUDITION\t"
			"valtan.test.animation-audition\tAUDITION_ONLY\t"
			"0\t0\t0\t0\t1\t1\t0\t1\t1\tANY\tANY\t0",
			"NORMAL");
		const std::string invalidAuditionMechanicBootstrap =
			appendAuditionPattern(
				bootstrapText, validAuditionRow, "MECHANIC");
		std::string invalidAuditionIntroBootstrap = validAuditionBootstrap;
		const bool madeAuditionIntro = replaceBootstrapRow(
			invalidAuditionIntroBootstrap,
			"ENCOUNTERINTRO\tENCOUNTER_VALTAN\tVALTAN_ENTRANCE_WHIRLWIND",
			"ENCOUNTERINTRO\tENCOUNTER_VALTAN\t"
			"VALTAN_TEST_ANIMATION_AUDITION");
		const std::shared_ptr<CGameplayCatalog> invalidAuditionHealthCatalog =
			loadBootstrapVariant(
				L"audition-health-fields", invalidAuditionHealthBootstrap);
		const std::shared_ptr<CGameplayCatalog> invalidAuditionTriggerCatalog =
			loadBootstrapVariant(
				L"audition-trigger-fields", invalidAuditionTriggerBootstrap);
		const std::shared_ptr<CGameplayCatalog> invalidAuditionWeightCatalog =
			loadBootstrapVariant(
				L"audition-weight-fields", invalidAuditionWeightBootstrap);
		const std::shared_ptr<CGameplayCatalog> invalidAuditionMechanicCatalog =
			loadBootstrapVariant(
				L"audition-mechanic", invalidAuditionMechanicBootstrap);
		const std::shared_ptr<CGameplayCatalog> invalidAuditionIntroCatalog =
			madeAuditionIntro ? loadBootstrapVariant(
				L"audition-intro", invalidAuditionIntroBootstrap) : nullptr;
		const std::vector<BOSS_PATTERN_DEFINITION>* auditionPatterns =
			nullptr == auditionCatalog ? nullptr :
				auditionCatalog->Find_BossPatterns("ENCOUNTER_VALTAN");
		const auto auditionPattern = nullptr == auditionPatterns ?
			std::vector<BOSS_PATTERN_DEFINITION>::const_iterator{} :
			std::find_if(
				auditionPatterns->begin(), auditionPatterns->end(),
				[](const BOSS_PATTERN_DEFINITION& pattern)
				{
					return "VALTAN_TEST_ANIMATION_AUDITION" ==
						pattern.strPatternId;
				});
		const bool loadedAuditionOnly = nullptr != auditionPatterns &&
			auditionPatterns->end() != auditionPattern &&
			BOSS_PATTERN_SELECTION::AUDITION_ONLY ==
				auditionPattern->eSelection &&
			0u == auditionPattern->iMinimumHealthBar &&
			0u == auditionPattern->iMaximumHealthBar &&
			0u == auditionPattern->iTriggerHealthBar &&
			0u == auditionPattern->iTriggerOrder &&
			0u == auditionPattern->iSelectionWeight &&
			0u == auditionPattern->iMaximumConsecutiveUses;
		tests.Require(
			loadedAuditionOnly && madeAuditionIntro &&
			nullptr == invalidAuditionHealthCatalog &&
			nullptr == invalidAuditionTriggerCatalog &&
			nullptr == invalidAuditionWeightCatalog &&
			nullptr == invalidAuditionMechanicCatalog &&
			nullptr == invalidAuditionIntroCatalog,
			"Admit zero-gated audition-only patterns and reject health, trigger, weight, mechanic, or intro promotion fields");

		if (loadedAuditionOnly)
		{
			std::map<PLAYER_ID, SERVER_PLAYER> auditionPlayers;
			SERVER_PLAYER auditionTarget{};
			auditionTarget.iPlayerId = 74002u;
			auditionTarget.iNetEntityId = 74002u;
			auditionTarget.iCurrentHp = 10000u;
			auditionTarget.iMaximumHp = 10000u;
			auditionTarget.fPositionX = 151.f;
			auditionTarget.fPositionY = 22.97f;
			auditionTarget.fPositionZ = -128.f;
			auditionTarget.isCombatReady = true;
			auditionPlayers.emplace(
				auditionTarget.iPlayerId, auditionTarget);
			const auto makeAuditionBoss = [&auditionCatalog]()
			{
				SERVER_WORLD_ENTITY boss{};
				boss.iNetEntityId = 74003u;
				boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
				boss.eAction = SERVER_ENTITY_ACTION::IDLE;
				boss.strArchetypeId = "BOSS_VALTAN";
				boss.strEncounterId = "ENCOUNTER_VALTAN";
				boss.iCurrentHp = 60000u;
				boss.iMaximumHp = 60000u;
				boss.iMaximumHealthBars = 160u;
				boss.iLastEvaluatedHealthBar = 160u;
				boss.iLastHealthMechanicGenerationEpoch = 1u;
				boss.iPhase = 1u;
				boss.fPositionX = 151.f;
				boss.fPositionY = 22.97f;
				boss.fPositionZ = -122.f;
				boss.fEngageDistance = 35.f;
				boss.fMoveSpeed = 3.f;
				boss.bIntroPatternConsumed = true;
				boss.bScriptedPatternPlayback = true;
				boss.PinnedDefinitionRevision =
					auditionCatalog->Get_ActiveRevision();
				return boss;
			};
			SERVER_WORLD_ENTITY automaticBoss = makeAuditionBoss();
			CValtanBrain automaticBrain;
			std::vector<DAMAGE_EVENT> auditionDamageEvents;
			automaticBrain.Update(
				automaticBoss, auditionPlayers, *auditionCatalog,
				valtanRoom->m_ServerNavigation, 1.f / 30.f, 22000u, {},
				auditionDamageEvents);
			const VALTAN_DECISION_TRACE* automaticTrace =
				automaticBrain.Get_LatestDecisionTrace();
			const auto automaticCandidate = nullptr == automaticTrace ?
				std::vector<VALTAN_DECISION_CANDIDATE_TRACE>::const_iterator{} :
				std::find_if(
					automaticTrace->Candidates.begin(),
					automaticTrace->Candidates.end(),
					[](const VALTAN_DECISION_CANDIDATE_TRACE& candidate)
					{
						return "VALTAN_TEST_ANIMATION_AUDITION" ==
							candidate.strPatternId;
					});
			const bool excludedFromAutomaticSelection =
				nullptr != automaticTrace &&
				automaticTrace->Candidates.end() != automaticCandidate &&
				0u != (automaticCandidate->iExclusionMask &
					VALTAN_EXCLUDE_WRONG_SELECTION_KIND) &&
				0u == automaticCandidate->iEffectiveWeight &&
				!automaticCandidate->bSelected &&
				AUDITION_PATTERN_ID != automaticBoss.strPatternId &&
				AUDITION_PATTERN_ID != automaticTrace->strSelectedPatternId;

			SERVER_WORLD_ENTITY forcedBoss = makeAuditionBoss();
			forcedBoss.iNetEntityId = 74004u;
			forcedBoss.PendingPatternIds.emplace_back(AUDITION_PATTERN_ID);
			CValtanBrain forcedBrain;
			auditionDamageEvents.clear();
			const std::uint32_t targetHpBefore =
				auditionPlayers.begin()->second.iCurrentHp;
			forcedBrain.Update(
				forcedBoss, auditionPlayers, *auditionCatalog,
				valtanRoom->m_ServerNavigation, 1.f / 30.f, 22001u, {},
				auditionDamageEvents);
			const VALTAN_DECISION_TRACE* forcedTrace =
				forcedBrain.Get_LatestDecisionTrace();
			const bool forcedStableIdExecuted =
				AUDITION_PATTERN_ID == forcedBoss.strPatternId &&
				AUDITION_STAGE_ACTION_ID == forcedBoss.strActionId &&
				SERVER_ENTITY_ACTION::PATTERN_ACTIVE == forcedBoss.eAction &&
				1u == forcedBoss.iPatternSequence &&
				forcedBoss.PendingPatternIds.empty() &&
				forcedBoss.TriggeredPatternIds.empty() &&
				forcedBoss.MechanicOccurrences.empty() &&
				nullptr != forcedTrace &&
				VALTAN_DECISION_SOURCE::FORCED_AUDITION == forcedTrace->eSource &&
				VALTAN_DECISION_RESULT::SELECTED == forcedTrace->eResult &&
				AUDITION_PATTERN_ID == forcedTrace->strPendingPatternId &&
				AUDITION_PATTERN_ID == forcedTrace->strSelectedPatternId &&
				auditionDamageEvents.empty() &&
				targetHpBefore == auditionPlayers.begin()->second.iCurrentHp;
			tests.Require(
				excludedFromAutomaticSelection && forcedStableIdExecuted,
				"Exclude audition-only from every automatic weight roll and execute it only through its pending stable ID");
		}

		std::string mixedTaggedRows = bootstrapText;
		const bool madeMixedTaggedRows = replaceBootstrapRow(
			mixedTaggedRows,
			"PATTERNROTATIONSTEP\tENCOUNTER_VALTAN\trotation.valtan.28.14\t0\tVALTAN_WHIRLWIND",
			"PATTERNROTATIONSTEP\tENCOUNTER_VALTAN\trotation.valtan.160.130\t0\tVALTAN_WHIRLWIND");
		std::string duplicateManagedCandidate = bootstrapText;
		const bool madeDuplicateManagedCandidate = replaceBootstrapRow(
			duplicateManagedCandidate,
			"PATTERNROTATIONCANDIDATE\tENCOUNTER_VALTAN\trotation.valtan.160.130\t1\tVALTAN_DASH_CHARGE\t30\t0",
			"PATTERNROTATIONCANDIDATE\tENCOUNTER_VALTAN\trotation.valtan.160.130\t1\tVALTAN_WHIRLWIND\t30\t0");
		std::string missingManagedOrdinal = bootstrapText;
		const bool madeMissingManagedOrdinal = replaceBootstrapRow(
			missingManagedOrdinal,
			"PATTERNROTATIONCANDIDATE\tENCOUNTER_VALTAN\trotation.valtan.160.130\t4\tVALTAN_HIGH_JUMP\t14\t1",
			"PATTERNROTATIONCANDIDATE\tENCOUNTER_VALTAN\trotation.valtan.160.130\t5\tVALTAN_HIGH_JUMP\t14\t1");
		std::string mismatchedManagedWindow = bootstrapText;
		const bool madeMismatchedManagedWindow = replaceBootstrapRow(
			mismatchedManagedWindow,
			"PATTERNROTATIONWINDOW\tENCOUNTER_VALTAN\trotation.valtan.160.130\twindow.valtan.phase1.160.130\t1\tselectionset.valtan.160.130\t160\t130\t5",
			"PATTERNROTATIONWINDOW\tENCOUNTER_VALTAN\trotation.valtan.160.130\twindow.valtan.phase1.160.130\t1\tselectionset.valtan.160.130\t159\t130\t5");
		std::string invalidManagedPhase = bootstrapText;
		const bool madeInvalidManagedPhase = replaceBootstrapRow(
			invalidManagedPhase,
			"PATTERNROTATIONWINDOW\tENCOUNTER_VALTAN\trotation.valtan.160.130\twindow.valtan.phase1.160.130\t1\tselectionset.valtan.160.130\t160\t130\t5",
			"PATTERNROTATIONWINDOW\tENCOUNTER_VALTAN\trotation.valtan.160.130\twindow.valtan.phase1.160.130\t4\tselectionset.valtan.160.130\t160\t130\t5");
		std::string collidingHealthMechanic = bootstrapText;
		const bool madeCollidingHealthMechanic = replaceBootstrapRow(
			collidingHealthMechanic,
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_FOUR_PILLARS_105\tvaltan.mechanic.four-pillars-105\tHEALTH_BAR\t0\t0\t100\t1\t0\t0\t0\t100\t4\tANY\tANY\t0",
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_FOUR_PILLARS_105\tvaltan.mechanic.four-pillars-105\tHEALTH_BAR\t0\t0\t84\t1\t0\t0\t0\t100\t4\tANY\tANY\t0");
		std::string divergentPhaseTopology = bootstrapText;
		const bool madeDivergentPhaseTopology = replaceBootstrapRow(
			divergentPhaseTopology,
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_ARENA_BREAK_109\tvaltan.mechanic.arena-break-109\tHEALTH_BAR\t0\t0\t109\t1\t0\t0\t0\t100\t6\tANY\tANY\t0",
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_ARENA_BREAK_109\tvaltan.mechanic.arena-break-109\tHEALTH_BAR\t0\t0\t100\t2\t0\t0\t0\t100\t6\tANY\tANY\t0");
		tests.Require(
			madeMixedTaggedRows && madeDuplicateManagedCandidate &&
			madeMissingManagedOrdinal && madeMismatchedManagedWindow &&
			madeInvalidManagedPhase && madeCollidingHealthMechanic &&
			madeDivergentPhaseTopology &&
			nullptr == loadBootstrapVariant(L"mixed", mixedTaggedRows) &&
			nullptr == loadBootstrapVariant(
				L"duplicate", duplicateManagedCandidate) &&
			nullptr == loadBootstrapVariant(
				L"missing", missingManagedOrdinal) &&
			nullptr == loadBootstrapVariant(
				L"window-mismatch", mismatchedManagedWindow) &&
			nullptr == loadBootstrapVariant(
				L"phase-four", invalidManagedPhase) &&
			nullptr == loadBootstrapVariant(
				L"health-order-collision", collidingHealthMechanic) &&
			nullptr == loadBootstrapVariant(
				L"phase-topology", divergentPhaseTopology),
			"Reject v20 mixed tags, malformed managed windows, duplicate health mechanic order, and divergent phase-transition topology");

		std::string managedWeightVariant = bootstrapText;
		bool managedWeightRowsReady = replaceBootstrapRow(
			managedWeightVariant,
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_FIST_IN_OUT\tvaltan.attack.fist-in-out\tNORMAL\t1\t130\t0\t0\t14\t1\t0\t16\t1\tANY\tANY\t0",
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_FIST_IN_OUT\tvaltan.attack.fist-in-out\tNORMAL\t1\t160\t0\t0\t14\t1\t0\t16\t1\tANY\tANY\t0");
		const auto tuneManagedCandidate = [&managedWeightVariant,
			&replaceBootstrapRow, &managedWeightRowsReady](
			const std::string_view rotationId, const std::uint32_t ordinal,
			const std::string_view patternId, const std::uint32_t oldWeight,
			const std::uint32_t newWeight, const bool enabled)
		{
			const std::string before =
				"PATTERNROTATIONCANDIDATE\tENCOUNTER_VALTAN\t" +
				std::string(rotationId) + "\t" + std::to_string(ordinal) + "\t" +
				std::string(patternId) + "\t" + std::to_string(oldWeight) +
				("VALTAN_DASH_CHARGE" == patternId ? "\t0" : "\t1");
			const std::string after =
				"PATTERNROTATIONCANDIDATE\tENCOUNTER_VALTAN\t" +
				std::string(rotationId) + "\t" + std::to_string(ordinal) + "\t" +
				std::string(patternId) + "\t" + std::to_string(newWeight) +
				"\t" + (enabled ? "1" : "0");
			managedWeightRowsReady = managedWeightRowsReady &&
				replaceBootstrapRow(managedWeightVariant, before, after);
		};
		tuneManagedCandidate(
			"rotation.valtan.160.130", 0u, "VALTAN_WHIRLWIND", 20u, 100u, true);
		tuneManagedCandidate(
			"rotation.valtan.160.130", 1u, "VALTAN_DASH_CHARGE", 30u, 1u, true);
		tuneManagedCandidate(
			"rotation.valtan.160.130", 2u, "VALTAN_FOUR_SLASH", 12u, 1u, true);
		tuneManagedCandidate(
			"rotation.valtan.160.130", 3u, "VALTAN_FIST_IN_OUT", 14u, 1u, true);
		tuneManagedCandidate(
			"rotation.valtan.160.130", 4u, "VALTAN_HIGH_JUMP", 14u, 1u, true);
		tuneManagedCandidate(
			"rotation.valtan.130.109", 0u, "VALTAN_WHIRLWIND", 20u, 1u, true);
		tuneManagedCandidate(
			"rotation.valtan.130.109", 1u, "VALTAN_DASH_CHARGE", 30u, 100u, true);
		tuneManagedCandidate(
			"rotation.valtan.130.109", 2u, "VALTAN_FOUR_SLASH", 12u, 1u, true);
		tuneManagedCandidate(
			"rotation.valtan.130.109", 3u, "VALTAN_FIST_IN_OUT", 14u, 2u, true);
		tuneManagedCandidate(
			"rotation.valtan.130.109", 4u, "VALTAN_HIGH_JUMP", 14u, 1u, false);
		const std::shared_ptr<CGameplayCatalog> managedWeightCatalog =
			managedWeightRowsReady ?
				loadBootstrapVariant(L"managed-window-weights",
					managedWeightVariant) : nullptr;

		struct MANAGED_WEIGHT_TRACE_FIXTURE final
		{
			VALTAN_DECISION_TRACE TicketZero;
			VALTAN_DECISION_TRACE TicketBoundary;
			VALTAN_DECISION_TRACE OpeningDifference;
			VALTAN_DECISION_TRACE SecondDifference;
			bool hasTicketZero = false;
			bool hasTicketBoundary = false;
			bool hasWindowDifference = false;
		};
		auto managedTraceFixture =
			std::make_unique<MANAGED_WEIGHT_TRACE_FIXTURE>();
		const auto evaluateManagedWindow = [&managedWeightCatalog,
			&valtanRoom, &pinFixture](
			const std::uint32_t healthBar, const std::uint32_t serverTick,
			VALTAN_DECISION_TRACE& outTrace)
		{
			if (nullptr == managedWeightCatalog) return false;
			auto boss = std::make_unique<SERVER_WORLD_ENTITY>();
			boss->iNetEntityId = 74001u;
			boss->eKind = WORLD_BOOTSTRAP_KIND::BOSS;
			boss->eAction = SERVER_ENTITY_ACTION::IDLE;
			boss->strArchetypeId = "BOSS_VALTAN";
			boss->strEncounterId = "ENCOUNTER_VALTAN";
			boss->iMaximumHp = 60000u;
			boss->iMaximumHealthBars = 160u;
			boss->iCurrentHp = CValtanBrain::Resolve_HealthBarHp(
				*boss, healthBar);
			boss->iLastEvaluatedHealthBar = healthBar;
			boss->iLastHealthMechanicGenerationEpoch = 1u;
			boss->iPhase = 1u;
			boss->fPositionX = 151.f;
			boss->fPositionY = 22.97f;
			boss->fPositionZ = -122.f;
			boss->fCollisionRadius = 2.6f;
			boss->fEngageDistance = 35.f;
			boss->fMoveSpeed = 3.f;
			boss->bIntroPatternConsumed = true;
			boss->bAutomaticPatternSequenceAuditionOverride = true;
			boss->PinnedDefinitionRevision =
				managedWeightCatalog->Get_ActiveRevision();
			boss->ArmorPlates.push_back({ 0u, 4000u, 50u });
			boss->ArmorPlates.push_back({ 1u, 4000u, 50u });
			std::map<PLAYER_ID, SERVER_PLAYER> players;
			SERVER_PLAYER player = pinFixture->Target;
			player.iCurrentHp = (std::max)(player.iCurrentHp, 1u);
			player.isCombatReady = true;
			player.fPositionX = boss->fPositionX + 6.f;
			player.fPositionY = boss->fPositionY;
			player.fPositionZ = boss->fPositionZ;
			players.emplace(player.iPlayerId, std::move(player));
			auto brain = std::make_unique<CValtanBrain>();
			std::vector<DAMAGE_EVENT> damageEvents;
			brain->Update(
				*boss, players, *managedWeightCatalog,
				valtanRoom->m_ServerNavigation, 1.f / 30.f, serverTick, {},
				damageEvents, managedWeightCatalog.get(), 1u);
			const VALTAN_DECISION_TRACE* trace =
				brain->Get_LatestDecisionTrace();
			if (nullptr == trace ||
				VALTAN_DECISION_SOURCE::WEIGHTED != trace->eSource ||
				VALTAN_DECISION_RESULT::SELECTED != trace->eResult)
			{
				return false;
			}
			outTrace = *trace;
			return true;
		};
		if (nullptr != managedWeightCatalog)
		{
			for (std::uint32_t tick = 1u; tick <= 20000u &&
				(!managedTraceFixture->hasTicketZero ||
				 !managedTraceFixture->hasTicketBoundary ||
				 !managedTraceFixture->hasWindowDifference); ++tick)
			{
				VALTAN_DECISION_TRACE opening{};
				if (!evaluateManagedWindow(159u, tick, opening) ||
					104u != opening.iTotalWeight)
				{
					continue;
				}
				if (!managedTraceFixture->hasTicketZero &&
					0u == opening.iRandomTicket)
				{
					managedTraceFixture->TicketZero = opening;
					managedTraceFixture->hasTicketZero = true;
				}
				if (!managedTraceFixture->hasTicketBoundary &&
					100u == opening.iRandomTicket)
				{
					managedTraceFixture->TicketBoundary = opening;
					managedTraceFixture->hasTicketBoundary = true;
				}
				if (!managedTraceFixture->hasWindowDifference &&
					opening.iRandomTicket >= 1u &&
					opening.iRandomTicket < 100u)
				{
					VALTAN_DECISION_TRACE second{};
					if (evaluateManagedWindow(129u, tick, second) &&
						104u == second.iTotalWeight &&
						opening.iRandomTicket == second.iRandomTicket)
					{
						managedTraceFixture->OpeningDifference = opening;
						managedTraceFixture->SecondDifference = second;
						managedTraceFixture->hasWindowDifference = true;
					}
				}
			}
		}
		const auto findTraceCandidate = [](
			const VALTAN_DECISION_TRACE& trace,
			const std::string_view patternId)
			-> const VALTAN_DECISION_CANDIDATE_TRACE*
		{
			const auto found = std::find_if(
				trace.Candidates.begin(), trace.Candidates.end(),
				[patternId](const VALTAN_DECISION_CANDIDATE_TRACE& candidate)
				{ return candidate.strPatternId == patternId; });
			return trace.Candidates.end() == found ? nullptr : &*found;
		};
		const VALTAN_DECISION_CANDIDATE_TRACE* zeroWhirlwind =
			findTraceCandidate(
				managedTraceFixture->TicketZero, "VALTAN_WHIRLWIND");
		const VALTAN_DECISION_CANDIDATE_TRACE* boundaryDash =
			findTraceCandidate(
				managedTraceFixture->TicketBoundary, "VALTAN_DASH_CHARGE");
		const VALTAN_DECISION_CANDIDATE_TRACE* openingWhirlwind =
			findTraceCandidate(
				managedTraceFixture->OpeningDifference, "VALTAN_WHIRLWIND");
		const VALTAN_DECISION_CANDIDATE_TRACE* openingDash =
			findTraceCandidate(
				managedTraceFixture->OpeningDifference, "VALTAN_DASH_CHARGE");
		const VALTAN_DECISION_CANDIDATE_TRACE* secondWhirlwind =
			findTraceCandidate(
				managedTraceFixture->SecondDifference, "VALTAN_WHIRLWIND");
		const VALTAN_DECISION_CANDIDATE_TRACE* secondDash =
			findTraceCandidate(
				managedTraceFixture->SecondDifference, "VALTAN_DASH_CHARGE");
		const VALTAN_DECISION_CANDIDATE_TRACE* secondHighJump =
			findTraceCandidate(
				managedTraceFixture->SecondDifference, "VALTAN_HIGH_JUMP");
		const auto* managedPatterns = nullptr == managedWeightCatalog ? nullptr :
			managedWeightCatalog->Find_BossPatterns("ENCOUNTER_VALTAN");
		const auto findCompatibilityWeight = [managedPatterns](
			const std::string_view patternId)
		{
			if (nullptr == managedPatterns) return std::uint32_t{ 0u };
			const auto found = std::find_if(
				managedPatterns->begin(), managedPatterns->end(),
				[patternId](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == patternId; });
			return managedPatterns->end() == found ?
				std::uint32_t{ 0u } : found->iSelectionWeight;
		};
		const std::array<std::string_view, 5u> expectedCandidateOrder{
			"VALTAN_WHIRLWIND", "VALTAN_DASH_CHARGE", "VALTAN_FOUR_SLASH",
			"VALTAN_FIST_IN_OUT", "VALTAN_HIGH_JUMP" };
		bool exactManagedCandidateOrder =
			managedTraceFixture->OpeningDifference.Candidates.size() >=
				expectedCandidateOrder.size();
		for (std::size_t index = 0u;
			exactManagedCandidateOrder && index < expectedCandidateOrder.size();
			++index)
		{
			exactManagedCandidateOrder = expectedCandidateOrder[index] ==
				managedTraceFixture->OpeningDifference.Candidates[index].strPatternId;
		}
		tests.Require(
			managedWeightRowsReady && nullptr != managedWeightCatalog &&
			managedTraceFixture->hasTicketZero &&
			managedTraceFixture->hasTicketBoundary &&
			managedTraceFixture->hasWindowDifference &&
			exactManagedCandidateOrder && nullptr != zeroWhirlwind &&
			nullptr != boundaryDash && nullptr != openingWhirlwind &&
			nullptr != openingDash && nullptr != secondWhirlwind &&
			nullptr != secondDash && nullptr != secondHighJump &&
			"VALTAN_WHIRLWIND" ==
				managedTraceFixture->TicketZero.strSelectedPatternId &&
			zeroWhirlwind->bSelected && 100u == zeroWhirlwind->iAuthoredWeight &&
			0u == zeroWhirlwind->iWeightBeginInclusive &&
			100u == zeroWhirlwind->iWeightEndExclusive &&
			"VALTAN_DASH_CHARGE" ==
				managedTraceFixture->TicketBoundary.strSelectedPatternId &&
			boundaryDash->bSelected &&
			100u == boundaryDash->iWeightBeginInclusive &&
			101u == boundaryDash->iWeightEndExclusive &&
			"VALTAN_WHIRLWIND" ==
				managedTraceFixture->OpeningDifference.strSelectedPatternId &&
			"VALTAN_DASH_CHARGE" ==
				managedTraceFixture->SecondDifference.strSelectedPatternId &&
			100u == openingWhirlwind->iAuthoredWeight &&
			100u == openingWhirlwind->iEffectiveWeight &&
			1u == openingDash->iAuthoredWeight &&
			1u == secondWhirlwind->iAuthoredWeight &&
			100u == secondDash->iAuthoredWeight &&
			0u == secondWhirlwind->iWeightBeginInclusive &&
			1u == secondWhirlwind->iWeightEndExclusive &&
			1u == secondDash->iWeightBeginInclusive &&
			101u == secondDash->iWeightEndExclusive &&
			0u != (secondHighJump->iExclusionMask &
				VALTAN_EXCLUDE_DISABLED) &&
			1u == secondHighJump->iAuthoredWeight &&
			0u == secondHighJump->iEffectiveWeight &&
			20u == findCompatibilityWeight("VALTAN_WHIRLWIND") &&
			30u == findCompatibilityWeight("VALTAN_DASH_CHARGE"),
			"Consume strict v22 managed candidate ordinal, per-window weight/enabled overrides, exact ticket boundaries, and truthful trace intervals without mutating compatibility weights");

		SERVER_WORLD_ENTITY& forcedOccurrenceBoss =
			valtanRoom->m_WorldEntities.back();
		const bool runningOldPatternStayedPinned =
			activeRevision == forcedOccurrenceBoss.PinnedDefinitionRevision;
		/* Model the ordinary end-of-tick retirement of the pattern that crossed
		   130 bars. The room may publish B on the idle entity, but the queued
		   occurrence itself must keep A live and select from A next tick. */
		forcedOccurrenceBoss.strPatternId.clear();
		forcedOccurrenceBoss.strPatternStageId.clear();
		forcedOccurrenceBoss.strActionId.clear();
		forcedOccurrenceBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		forcedOccurrenceBoss.PinnedDefinitionRevision = candidateRevision;
		const CGameplayCatalog* forcedOccurrenceCatalog =
			valtanRoom->Resolve_ValtanGameplayCatalog(forcedOccurrenceBoss);
		const bool queuedOldGenerationStayedLive =
			valtanRoom->Build_RequiredPinnedGameplayRevisions(
				pinFixture->RequiredPins) &&
			pinFixture->RequiredPins.end() != std::find(
				pinFixture->RequiredPins.begin(),
				pinFixture->RequiredPins.end(), activeRevision) &&
			nullptr != forcedOccurrenceCatalog &&
			activeRevision == forcedOccurrenceCatalog->Get_ActiveRevision();
		if (nullptr != forcedOccurrenceCatalog)
		{
			pinFixture->Brain.Update(
				forcedOccurrenceBoss, pinFixture->Players,
				*forcedOccurrenceCatalog,
				valtanRoom->m_ServerNavigation, 1.f / 30.f, 12u, {},
				pinFixture->DamageEvents);
		}
		const auto forcedOccurrence = std::find_if(
			forcedOccurrenceBoss.MechanicOccurrences.begin(),
			forcedOccurrenceBoss.MechanicOccurrences.end(),
			[](const SERVER_BOSS_MECHANIC_OCCURRENCE& occurrence)
			{
				return "VALTAN_FLOOR_WIPE_130" == occurrence.strPatternId;
			});
		const bool forcedMechanicStartedFromOldGeneration =
			forcedOccurrenceBoss.MechanicOccurrences.end() != forcedOccurrence &&
			SERVER_BOSS_MECHANIC_STATE::ACTIVE == forcedOccurrence->eState &&
			"VALTAN_FLOOR_WIPE_130" == forcedOccurrenceBoss.strPatternId &&
			activeRevision == forcedOccurrenceBoss.PinnedDefinitionRevision;
		if (forcedOccurrenceBoss.MechanicOccurrences.end() != forcedOccurrence)
			forcedOccurrence->eState = SERVER_BOSS_MECHANIC_STATE::COMPLETED;
		forcedOccurrenceBoss.strPatternId.clear();
		forcedOccurrenceBoss.strPatternStageId.clear();
		forcedOccurrenceBoss.strActionId.clear();
		forcedOccurrenceBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		forcedOccurrenceBoss.PinnedDefinitionRevision = candidateRevision;
		const CGameplayCatalog* nextNormalCatalog =
			valtanRoom->Resolve_ValtanGameplayCatalog(forcedOccurrenceBoss);
		const bool laterNormalUsesNewGeneration =
			nullptr != nextNormalCatalog && candidateRevision ==
				nextNormalCatalog->Get_ActiveRevision();
		tests.Require(
			runningOldPatternStayedPinned && queuedOldGenerationStayedLive &&
			forcedMechanicStartedFromOldGeneration &&
			laterNormalUsesNewGeneration,
			"Run a threshold mechanic from its queued old generation after commit, then return normal selection to the new generation");

		GameplayDataRevision secondCandidateRevision = candidateRevision;
		secondCandidateRevision.Bytes[2] ^= 0x20u;
		if (!secondCandidateRevision.Is_Valid() ||
			secondCandidateRevision == candidateRevision ||
			secondCandidateRevision == activeRevision)
			secondCandidateRevision.Bytes[3] ^= 1u;
		auto secondCandidate = std::make_shared<CGameplayCatalog>();
		const bool secondCandidateLoaded = secondCandidate->Load_FromBootstrap(
			bootstrapPath, activeRevision, secondCandidateRevision);
		C2S_DATA_REVISION_PREPARE_REQUEST abortRequest{};
		abortRequest.iTransactionSequence = 72u;
		abortRequest.BaseRevision = candidateRevision;
		abortRequest.CandidateRevision = secondCandidateRevision;
		abortRequest.iRequiredPresentationLaneMask =
			GAMEPLAY_PRESENTATION_KNOWN_LANE_MASK;
		CServerApp::SERVER_CONTROL_EVENT abortBegin{};
		abortBegin.eKind =
			CServerApp::SERVER_CONTROL_EVENT_KIND::DATA_REVISION_REQUEST;
		abortBegin.iSessionId = REQUESTER_SESSION;
		abortBegin.RevisionRequest = abortRequest;
		abortBegin.pCandidateGeneration = secondCandidate;
		abortBegin.BaseBootstrapContentRevision = activeRevision;
		abortBegin.CandidateBootstrapContentRevision = activeRevision;
		abortBegin.BaseNonValtanGameplayRevision = activeRevision;
		abortBegin.CandidateNonValtanGameplayRevision = activeRevision;
		(void)app.Queue_ServerControlEvent(std::move(abortBegin));
		app.Advance_ServerControlTransactions();
		CServerApp::SERVER_CONTROL_EVENT nack{};
		nack.eKind =
			CServerApp::SERVER_CONTROL_EVENT_KIND::DATA_REVISION_RESPONSE;
		nack.iSessionId = REQUESTER_SESSION;
		nack.RevisionResponse.iTransactionSequence =
			abortRequest.iTransactionSequence;
		nack.RevisionResponse.CandidateRevision = secondCandidateRevision;
		nack.RevisionResponse.eStatus = DATA_REVISION_PREPARE_STATUS::NACK;
		nack.RevisionResponse.iRequiredPresentationLaneMask =
			abortRequest.iRequiredPresentationLaneMask;
		nack.RevisionResponse.strReason = "contract NACK";
		CServerApp::SERVER_CONTROL_EVENT innocentReady{};
		innocentReady.eKind =
			CServerApp::SERVER_CONTROL_EVENT_KIND::DATA_REVISION_RESPONSE;
		innocentReady.iSessionId = PARTICIPANT_SESSION;
		innocentReady.RevisionResponse.iTransactionSequence =
			abortRequest.iTransactionSequence;
		innocentReady.RevisionResponse.CandidateRevision =
			secondCandidateRevision;
		innocentReady.RevisionResponse.eStatus =
			DATA_REVISION_PREPARE_STATUS::READY;
		innocentReady.RevisionResponse.iRequiredPresentationLaneMask =
			abortRequest.iRequiredPresentationLaneMask;
		innocentReady.RevisionResponse.iPreparedPresentationLaneMask =
			abortRequest.iRequiredPresentationLaneMask;
		(void)app.Queue_ServerControlEvent(std::move(nack));
		app.Advance_ServerControlTransactions();
		const bool abortedBeforeLateReady =
			!app.m_DataRevisionTransaction.Is_Active();
		(void)app.Queue_ServerControlEvent(std::move(innocentReady));
		app.Advance_ServerControlTransactions();
		const bool rolledBackWithoutClosingInnocent = secondCandidateLoaded &&
			abortedBeforeLateReady &&
			!app.m_DataRevisionTransaction.Is_Active() &&
			candidateRevision ==
				app.m_pActiveGameplayGeneration->Get_ActiveRevision() &&
			candidateRevision == valtanRoom->Get_ActiveGameplayGeneration()->
				Get_ActiveRevision() && participant->m_isSendRunning.load();
		tests.Require(
			rolledBackWithoutClosingInnocent,
			"Abort every room on NACK and ignore an innocent next-drain READY for the exact aborted transaction");

		C2S_DATA_REVISION_PREPARE_REQUEST staleBootstrapRequest =
			abortRequest;
		staleBootstrapRequest.iTransactionSequence = 73u;
		GameplayDataRevision staleBootstrapBaseline = activeRevision;
		staleBootstrapBaseline.Bytes[6] ^= 0x40u;
		if (!staleBootstrapBaseline.Is_Valid() ||
			staleBootstrapBaseline == activeRevision)
		{
			staleBootstrapBaseline.Bytes[7] ^= 1u;
		}
		CServerApp::SERVER_CONTROL_EVENT staleBootstrap{};
		staleBootstrap.eKind =
			CServerApp::SERVER_CONTROL_EVENT_KIND::DATA_REVISION_REQUEST;
		staleBootstrap.iSessionId = REQUESTER_SESSION;
		staleBootstrap.RevisionRequest = staleBootstrapRequest;
		staleBootstrap.pCandidateGeneration = secondCandidate;
		staleBootstrap.BaseBootstrapContentRevision = activeRevision;
		staleBootstrap.CandidateBootstrapContentRevision = activeRevision;
		staleBootstrap.BaseNonValtanGameplayRevision = activeRevision;
		staleBootstrap.CandidateNonValtanGameplayRevision =
			staleBootstrapBaseline;
		const std::size_t beforeStaleBootstrapResult =
			requester->m_OutboundFrames.size();
		(void)app.Queue_ServerControlEvent(std::move(staleBootstrap));
		app.Advance_ServerControlTransactions();
		tests.Require(
			!app.m_DataRevisionTransaction.Is_Active() &&
			requester->m_isSendRunning.load() &&
			requester->m_OutboundFrames.size() ==
				beforeStaleBootstrapResult + 1u &&
			candidateRevision ==
				app.m_pActiveGameplayGeneration->Get_ActiveRevision() &&
			activeRevision ==
				app.m_ActiveGameplayBootstrapContentRevision &&
			activeRevision == app.m_ActiveNonValtanGameplayRevision,
			"Reject a stale full-bootstrap candidate baseline before PREPARE can roll back non-Valtan rows");

		const auto makeFrame = [](const C2S_DATA_REVISION_PREPARE_REQUEST& value)
		{
			CPacketWriter writer;
			PACKET_FRAME frame{};
			frame.ePacketType =
				PACKET_TYPE::C2S_DATA_REVISION_PREPARE_REQUEST;
			if (Write_Message(writer, value))
			{
				const auto payload = writer.Get_Buffer();
				frame.Payload.assign(payload.begin(), payload.end());
			}
			return frame;
		};
		const std::size_t beforeIdempotent =
			requester->m_OutboundFrames.size();
		app.On_SessionFrame(REQUESTER_SESSION, makeFrame(request));
		S2C_DATA_REVISION_RESULT idempotentResult{};
		bool decodedIdempotentResult = false;
		if (requester->m_OutboundFrames.size() == beforeIdempotent + 1u)
		{
			const auto& bytes = requester->m_OutboundFrames.back().Bytes;
			PACKET_HEADER header{};
			if (Read_Packet_Header(bytes, header) &&
				PACKET_TYPE::S2C_DATA_REVISION_RESULT == header.ePacketType)
			{
				CPacketReader resultReader{ std::span<const std::uint8_t>(
					bytes.data() + PACKET_HEADER_BYTES,
					bytes.size() - PACKET_HEADER_BYTES) };
				decodedIdempotentResult =
					Read_Message(resultReader, idempotentResult) &&
					0u == resultReader.Get_RemainingSize();
			}
		}
		const bool idempotentVerdict = decodedIdempotentResult &&
			DATA_REVISION_RESULT::COMMITTED == idempotentResult.eResult &&
			candidateRevision == idempotentResult.ActiveRevision;
		const char* idempotentContract =
			"Answer an already-active retry with the only truthful typed COMMITTED terminal state";
		tests.Require(
			requester->m_isSendRunning.load() && idempotentVerdict,
			idempotentContract);

		C2S_DATA_REVISION_PREPARE_REQUEST repeatedRequest = abortRequest;
		repeatedRequest.iTransactionSequence = 74u;
		CServerApp::SERVER_CONTROL_EVENT repeatedBegin{};
		repeatedBegin.eKind =
			CServerApp::SERVER_CONTROL_EVENT_KIND::DATA_REVISION_REQUEST;
		repeatedBegin.iSessionId = REQUESTER_SESSION;
		repeatedBegin.RevisionRequest = repeatedRequest;
		repeatedBegin.pCandidateGeneration = secondCandidate;
		repeatedBegin.BaseBootstrapContentRevision = activeRevision;
		repeatedBegin.CandidateBootstrapContentRevision = activeRevision;
		repeatedBegin.BaseNonValtanGameplayRevision = activeRevision;
		repeatedBegin.CandidateNonValtanGameplayRevision = activeRevision;
		(void)app.Queue_ServerControlEvent(std::move(repeatedBegin));
		app.Advance_ServerControlTransactions();
		const bool repeatedPrepared =
			app.m_DataRevisionTransaction.Is_Active();
		const auto queueRepeatedReady =
			[&app, &repeatedRequest](const SESSION_ID sessionId)
		{
			CServerApp::SERVER_CONTROL_EVENT event{};
			event.eKind =
				CServerApp::SERVER_CONTROL_EVENT_KIND::DATA_REVISION_RESPONSE;
			event.iSessionId = sessionId;
			event.RevisionResponse.iTransactionSequence =
				repeatedRequest.iTransactionSequence;
			event.RevisionResponse.CandidateRevision =
				repeatedRequest.CandidateRevision;
			event.RevisionResponse.eStatus =
				DATA_REVISION_PREPARE_STATUS::READY;
			event.RevisionResponse.iRequiredPresentationLaneMask =
				repeatedRequest.iRequiredPresentationLaneMask;
			event.RevisionResponse.iPreparedPresentationLaneMask =
				repeatedRequest.iRequiredPresentationLaneMask;
			return app.Queue_ServerControlEvent(std::move(event));
		};
		const bool repeatedReady =
			queueRepeatedReady(REQUESTER_SESSION) &&
			queueRepeatedReady(PARTICIPANT_SESSION);
		app.Advance_ServerControlTransactions();
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION recoveredSecondRuntime{};
		bool recoveredSecondPointer = false;
		runtimeRecoveryStatus.clear();
		const bool recoveredSecondCommit =
			CServerApp::Recover_RuntimeActiveGameplayPointer(
				runtimePersistenceRoot, packagedRuntime,
				recoveredSecondRuntime, recoveredSecondPointer,
				runtimeRecoveryStatus) && recoveredSecondPointer &&
			secondCandidateRevision == recoveredSecondRuntime.Revision;
		tests.Require(
			repeatedPrepared && repeatedReady &&
			!app.m_DataRevisionTransaction.Is_Active() &&
			secondCandidateRevision ==
				app.m_pActiveGameplayGeneration->Get_ActiveRevision() &&
			secondCandidateRevision ==
				valtanRoom->Get_ActiveGameplayGeneration()->
					Get_ActiveRevision() &&
			secondCandidateRevision ==
				privateRoom->Get_ActiveGameplayGeneration()->
					Get_ActiveRevision() &&
			activeRevision == app.m_ActiveNonValtanGameplayRevision &&
			recoveredSecondCommit,
			"Commit a second Valtan-only candidate in the same Server process without requiring the packaged bootstrap baseline");

		std::string resetStatus;
		const bool resetEscapesMissingOrRetiredDurableCandidate =
			CServerApp::Reset_RuntimeGameplayActivationToPackaged(
				runtimePersistenceRoot, packagedRuntime, resetStatus) &&
			CServerApp::Reset_RuntimeGameplayActivationToPackaged(
				runtimePersistenceRoot, packagedRuntime, resetStatus);
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION packagedAfterCandidateReset{};
		bool packagedPointerAfterCandidateReset = false;
		const bool candidateResetSelectedPackaged =
			resetEscapesMissingOrRetiredDurableCandidate &&
			CServerApp::Recover_RuntimeActiveGameplayPointer(
				runtimePersistenceRoot, packagedRuntime,
				packagedAfterCandidateReset,
				packagedPointerAfterCandidateReset, resetStatus) &&
			packagedPointerAfterCandidateReset &&
			RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE ==
				packagedAfterCandidateReset.eSource &&
			packagedRuntime.Revision == packagedAfterCandidateReset.Revision &&
			packagedRuntime.BootstrapContentRevision ==
				packagedAfterCandidateReset.BootstrapContentRevision &&
			packagedRuntime.NonValtanGameplayRevision ==
				packagedAfterCandidateReset.NonValtanGameplayRevision;

		const fs::path packagedResetRoot = fs::temp_directory_path() /
			(L"lostark-server-runtime-packaged-reset-contract-" +
			 std::to_wstring(::GetCurrentProcessId()));
		std::error_code packagedResetError;
		fs::remove_all(packagedResetRoot, packagedResetError);
		fs::create_directories(packagedResetRoot, packagedResetError);
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION oldPackaged = packagedRuntime;
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION newPackaged = packagedRuntime;
		newPackaged.Revision.Bytes[16] ^= 0x20u;
		newPackaged.BootstrapContentRevision.Bytes[17] ^= 0x40u;
		newPackaged.NonValtanGameplayRevision.Bytes[18] ^= 0x80u;
		const bool wroteOldPackagedPointer = !packagedResetError &&
			CServerApp::Rollback_RuntimeGameplayActivation(
				packagedResetRoot, oldPackaged, resetStatus);
		const bool resetAcrossPackagedDrift = wroteOldPackagedPointer &&
			CServerApp::Reset_RuntimeGameplayActivationToPackaged(
				packagedResetRoot, newPackaged, resetStatus) &&
			CServerApp::Reset_RuntimeGameplayActivationToPackaged(
				packagedResetRoot, newPackaged, resetStatus);
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION recoveredNewPackaged{};
		bool recoveredNewPackagedPointer = false;
		const bool recoveredResetPackage = resetAcrossPackagedDrift &&
			CServerApp::Recover_RuntimeActiveGameplayPointer(
				packagedResetRoot, newPackaged, recoveredNewPackaged,
				recoveredNewPackagedPointer, resetStatus) &&
			recoveredNewPackagedPointer &&
			RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE ==
				recoveredNewPackaged.eSource &&
			newPackaged.Revision == recoveredNewPackaged.Revision &&
			newPackaged.BootstrapContentRevision ==
				recoveredNewPackaged.BootstrapContentRevision &&
			newPackaged.NonValtanGameplayRevision ==
				recoveredNewPackaged.NonValtanGameplayRevision;

		const bool restoredOldPointerForCorruptJournal =
			CServerApp::Rollback_RuntimeGameplayActivation(
				packagedResetRoot, oldPackaged, resetStatus);
		bool wroteCorruptJournal = false;
		if (restoredOldPointerForCorruptJournal)
		{
			std::ofstream journal(
				packagedResetRoot / L"active-generation.journal.json",
				std::ios::binary | std::ios::trunc);
			constexpr std::string_view corruptJournal =
				"{\"schema\":\"corrupt-runtime-journal\"}";
			journal.write(corruptJournal.data(),
				static_cast<std::streamsize>(corruptJournal.size()));
			wroteCorruptJournal = journal.good();
		}
		const bool corruptJournalResetRejected = wroteCorruptJournal &&
			!CServerApp::Reset_RuntimeGameplayActivationToPackaged(
				packagedResetRoot, newPackaged, resetStatus);
		std::error_code corruptJournalRemoveError;
		fs::remove(packagedResetRoot / L"active-generation.journal.json",
			corruptJournalRemoveError);
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION pointerAfterCorruptJournal{};
		bool pointerAfterCorruptJournalPresent = false;
		const bool corruptJournalPreservedOldPointer =
			corruptJournalResetRejected && !corruptJournalRemoveError &&
			CServerApp::Recover_RuntimeActiveGameplayPointer(
				packagedResetRoot, oldPackaged, pointerAfterCorruptJournal,
				pointerAfterCorruptJournalPresent, resetStatus) &&
			pointerAfterCorruptJournalPresent &&
			oldPackaged.Revision == pointerAfterCorruptJournal.Revision;

		bool wroteCorruptPointer = false;
		{
			std::ofstream pointer(
				packagedResetRoot / L"active-generation.json",
				std::ios::binary | std::ios::trunc);
			constexpr std::string_view corruptPointer =
				"{\"schema\":\"corrupt-runtime-pointer\"}";
			pointer.write(corruptPointer.data(),
				static_cast<std::streamsize>(corruptPointer.size()));
			wroteCorruptPointer = pointer.good();
		}
		const bool corruptPointerResetRejected = wroteCorruptPointer &&
			!CServerApp::Reset_RuntimeGameplayActivationToPackaged(
				packagedResetRoot, newPackaged, resetStatus);
		fs::remove_all(packagedResetRoot, packagedResetError);

		void* ownerMutex = nullptr;
		std::string mutexStatus;
		const bool acquiredOwnerMutex =
			CServerApp::Acquire_RuntimeGameplayProcessMutex(
				ownerMutex, mutexStatus);
		std::atomic_bool secondThreadWasRefused{ false };
		std::thread competingResetThread([&secondThreadWasRefused]()
		{
			void* competingMutex = nullptr;
			std::string competingStatus;
			secondThreadWasRefused.store(
				!CServerApp::Acquire_RuntimeGameplayProcessMutex(
					competingMutex, competingStatus));
			CServerApp::Release_RuntimeGameplayProcessMutex(competingMutex);
		});
		competingResetThread.join();
		CServerApp::Release_RuntimeGameplayProcessMutex(ownerMutex);
		tests.Require(
			candidateResetSelectedPackaged && recoveredResetPackage &&
			corruptJournalPreservedOldPointer &&
			corruptPointerResetRejected && acquiredOwnerMutex &&
			secondThreadWasRefused.load(),
			"Reset a missing or retired candidate to packaged idempotently, reject corrupt durable state without mutation, and refuse a concurrent Server owner");
		fs::remove_all(runtimePersistenceRoot, runtimePersistenceError);

		valtanRoom->m_WorldEntities.pop_back();
		valtanRoom->m_GameplayCatalog.Collect_Garbage({ candidateRevision });
		tests.Require(
			nullptr == valtanRoom->Resolve_GameplayGeneration(activeRevision),
			"Collect an old immutable gameplay generation after its final occurrence pin is released");
		auto capacityRoomStorage = std::make_unique<CGameRoom>(
			WORLD_ID::TRAINING_GROUND, baseGeneration);
		CGameRoom& capacityRoom = *capacityRoomStorage;
		SERVER_WORLD_ENTITY capacityBasePin{};
		capacityBasePin.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		capacityBasePin.strPatternId = "VALTAN_GENERATION_CAPACITY_PIN";
		capacityBasePin.PinnedDefinitionRevision = activeRevision;
		capacityRoom.m_WorldEntities.push_back(capacityBasePin);
		bool filledGenerationCapacity = capacityRoom.Is_Ready();
		GameplayDataRevision capacityActive = activeRevision;
		for (std::uint32_t ordinal = 1u;
			filledGenerationCapacity && ordinal <
				CGameplayCatalogGenerations::MAX_GENERATION_COUNT;
			++ordinal)
		{
			GameplayDataRevision generationRevision = activeRevision;
			generationRevision.Bytes[28] ^= 0x5au;
			generationRevision.Bytes[29] =
				static_cast<std::uint8_t>(ordinal);
			auto generation = std::make_shared<CGameplayCatalog>();
			std::string capacityStatus;
			filledGenerationCapacity = generation->Load_FromBootstrap(
				bootstrapPath, activeRevision, generationRevision) &&
				capacityRoom.Stage_GameplayGeneration(
					100u + ordinal, capacityActive, generation, capacityStatus) &&
				capacityRoom.Commit_GameplayGeneration(100u + ordinal);
			if (!filledGenerationCapacity) break;
			SERVER_WORLD_ENTITY pin = capacityBasePin;
			pin.PinnedDefinitionRevision = generationRevision;
			capacityRoom.m_WorldEntities.push_back(std::move(pin));
			capacityActive = generationRevision;
		}
		GameplayDataRevision overflowRevision = activeRevision;
		overflowRevision.Bytes[28] ^= 0xa5u;
		overflowRevision.Bytes[29] ^= 0xffu;
		auto overflowGeneration = std::make_shared<CGameplayCatalog>();
		std::string overflowStatus;
		const bool rejectedGenerationOverflow = filledGenerationCapacity &&
			CGameplayCatalogGenerations::MAX_GENERATION_COUNT ==
				capacityRoom.m_GameplayCatalog.Get_GenerationCount() &&
			overflowGeneration->Load_FromBootstrap(
				bootstrapPath, activeRevision, overflowRevision) &&
			!capacityRoom.Stage_GameplayGeneration(
				999u, capacityActive, overflowGeneration, overflowStatus) &&
			overflowStatus.find("capacity") != std::string::npos;
		if (!rejectedGenerationOverflow)
		{
			std::cout << "[STATUS] generation capacity count=" <<
				capacityRoom.m_GameplayCatalog.Get_GenerationCount() <<
				", filled=" << filledGenerationCapacity <<
				", status=" << overflowStatus << '\n';
		}
		tests.Require(rejectedGenerationOverflow,
			"Fail-close the seventeenth immutable gameplay generation while sixteen occurrence revisions remain pinned");

		const fs::path pathContractRoot = fs::temp_directory_path() /
			(L"LostArkCandidatePathContract-" +
				std::to_wstring(GetCurrentProcessId()));
		const fs::path siblingRoot = fs::path(
			pathContractRoot.native() + L"Evil");
		std::error_code pathContractError;
		fs::remove_all(pathContractRoot, pathContractError);
		pathContractError.clear();
		fs::remove_all(siblingRoot, pathContractError);
		fs::create_directories(pathContractRoot / L"nested", pathContractError);
		fs::create_directories(siblingRoot, pathContractError);
		{
			std::ofstream validFile(pathContractRoot / L"nested" / L"payload.bin",
				std::ios::binary | std::ios::trunc);
			validFile << "candidate-path-contract";
			std::ofstream siblingFile(siblingRoot / L"payload.bin",
				std::ios::binary | std::ios::trunc);
			siblingFile << "sibling-prefix-contract";
		}
		const fs::path canonicalContractRoot =
			fs::canonical(pathContractRoot, pathContractError);
		fs::path resolvedArtifact;
		std::string pathContractStatus;
		const bool admittedNested = !pathContractError &&
			CServerApp::Resolve_CandidateArtifactForAdmission(
				canonicalContractRoot, "nested/payload.bin",
				resolvedArtifact, pathContractStatus);
		const bool rejectedDriveQualified =
			!CServerApp::Resolve_CandidateArtifactForAdmission(
				canonicalContractRoot, "C:/Windows/System32/kernel32.dll",
				resolvedArtifact, pathContractStatus);
		const bool rejectedParent =
			!CServerApp::Resolve_CandidateArtifactForAdmission(
				canonicalContractRoot, "../LostArkCandidatePathContract-" +
					std::to_string(GetCurrentProcessId()) + "Evil/payload.bin",
				resolvedArtifact, pathContractStatus);
		const bool rejectedSiblingAbsolute =
			!CServerApp::Resolve_CandidateArtifactForAdmission(
				canonicalContractRoot, siblingRoot.string() + "/payload.bin",
				resolvedArtifact, pathContractStatus);
		bool rejectedParentLink = true;
		pathContractError.clear();
		fs::create_directory_symlink(
			siblingRoot, pathContractRoot / L"parent-link", pathContractError);
		if (!pathContractError)
		{
			rejectedParentLink =
				!CServerApp::Resolve_CandidateArtifactForAdmission(
					canonicalContractRoot, "parent-link/payload.bin",
					resolvedArtifact, pathContractStatus);
		}
		tests.Require(
			admittedNested && rejectedDriveQualified && rejectedParent &&
			rejectedSiblingAbsolute && rejectedParentLink,
			"Admit only canonical descendant candidate files and reject drive, sibling-prefix, parent, and parent-link escapes");
		pathContractError.clear();
		fs::remove_all(pathContractRoot, pathContractError);
		pathContractError.clear();
		fs::remove_all(siblingRoot, pathContractError);
		requester->Request_Close();
		participant->Request_Close();
		app.m_Sessions.clear();
		app.m_GameplayBindingBySessionId.clear();
		app.m_CharacterSelectArenas.clear();
		app.m_SharedGameRooms.clear();
		app.m_pActiveGameplayGeneration.reset();
	}
	{
		const VALTAN_TIMELINE_DEFINITION* timeline =
			catalog.Find_ValtanTimeline("ENCOUNTER_VALTAN");
		bool hasExactOrder = nullptr != timeline &&
			timeline->strTimelineId == "VALTAN_AUDITION_TIMELINE" &&
			timeline->Rows.size() == 52u;
		if (hasExactOrder)
		{
			for (std::uint32_t ordinal = 1u; ordinal <= 52u; ++ordinal)
			{
				const VALTAN_TIMELINE_ROW& row = timeline->Rows[ordinal - 1u];
				hasExactOrder = row.iOrdinal == ordinal &&
					!row.strRowId.empty() &&
					row.iCommandId ==
						Calculate_TestTimelineCommandId(row.strRowId) &&
					&row == catalog.Find_ValtanTimelineRow(
						"ENCOUNTER_VALTAN", row.iCommandId) &&
					!row.PatternActions.empty();
				if (!hasExactOrder)
					break;
			}
		}
		tests.Require(
			hasExactOrder && 160u == timeline->Rows[0].iSectionHealthBar &&
			"valtan.timeline.160-entrance-whirlwind" ==
				timeline->Rows[0].strRowId &&
			VALTAN_TIMELINE_ARENA_STATE::ORDINARY_WALLS_GONE ==
				timeline->Rows[19].eArenaState &&
			VALTAN_TIMELINE_ARENA_STATE::FLOOR84_AND_30_GONE ==
				timeline->Rows[42].eArenaState &&
			2u == timeline->Rows[28].PatternActions.size() &&
			"VALTAN_MAGIC_CHOICE" ==
				timeline->Rows[28].PatternActions[0].strPatternId &&
			"VALTAN_RED_BLADE_WAVE" ==
				timeline->Rows[28].PatternActions[1].strPatternId &&
			2u == timeline->Rows[2].PatternActions.size() &&
			1u == timeline->Rows[15].PatternActions.size() &&
			2u == timeline->Rows[31].PatternActions.size(),
			"Load the 52-row Valtan timeline with pattern-before-row bootstrap ordering");
	}
	{
		const std::vector<BOSS_PATTERN_DEFINITION>* patterns =
			catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const auto findStage = [patterns](
			const std::string& patternId,
			const std::string& stageId) -> const BOSS_PATTERN_STAGE_DEFINITION*
		{
			if (nullptr == patterns)
				return nullptr;
			for (const BOSS_PATTERN_DEFINITION& pattern : *patterns)
			{
				if (pattern.strPatternId != patternId)
					continue;
				const auto stage = std::find_if(
					pattern.Stages.begin(), pattern.Stages.end(),
					[&stageId](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
					{ return candidate.strStageId == stageId; });
				return stage == pattern.Stages.end() ? nullptr : &*stage;
			}
			return nullptr;
		};
		const BOSS_PATTERN_STAGE_DEFINITION* swing =
			findStage("VALTAN_SWING", "SWEEP");
		const BOSS_PATTERN_STAGE_DEFINITION* downSmash =
			findStage("VALTAN_DOWN_SMASH", "IMPACT");
		const BOSS_PATTERN_STAGE_DEFINITION* floorWipe =
			findStage("VALTAN_FLOOR_WIPE_130", "FIRST_SMASH");
		const BOSS_PATTERN_STAGE_DEFINITION* roar =
			findStage("VALTAN_IMPRISON_ROAR", "ROAR");
		const BOSS_PATTERN_STAGE_DEFINITION* fourSlashes =
			findStage("VALTAN_FOUR_SLASH", "SLASHES");
		const BOSS_PATTERN_STAGE_DEFINITION* fourSlashSpin =
			findStage("VALTAN_FOUR_SLASH", "SPIN");
		const BOSS_PATTERN_STAGE_DEFINITION* highJumpTakeoff =
			findStage("VALTAN_HIGH_JUMP", "TAKEOFF");
		const BOSS_PATTERN_STAGE_DEFINITION* highJumpAirborne =
			findStage("VALTAN_HIGH_JUMP", "AIRBORNE");
		const BOSS_PATTERN_STAGE_DEFINITION* highJumpLand =
			findStage("VALTAN_HIGH_JUMP", "LAND");
		const BOSS_PATTERN_STAGE_DEFINITION* highJumpRecovery =
			findStage("VALTAN_HIGH_JUMP", "RECOVERY");
		const BOSS_COMBAT_OBJECT_DEFINITION* highJumpTargetAxe =
			catalog.Find_BossCombatObject(
				"combatobject.valtan.high-jump.target-axe");
		const auto findPattern = [patterns](
			const std::string& patternId) -> const BOSS_PATTERN_DEFINITION*
		{
			if (nullptr == patterns)
				return nullptr;
			const auto found = std::find_if(
				patterns->begin(), patterns->end(),
				[&patternId](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == patternId; });
			return patterns->end() == found ? nullptr : &*found;
		};
		const BOSS_PATTERN_DEFINITION* swingPattern =
			findPattern("VALTAN_SWING");
		const BOSS_PATTERN_DEFINITION* fourSlashPattern =
			findPattern("VALTAN_FOUR_SLASH");
		const BOSS_PATTERN_DEFINITION* highJumpPattern =
			findPattern("VALTAN_HIGH_JUMP");
		const BOSS_PATTERN_DEFINITION* arenaBreakPattern =
			findPattern("VALTAN_ARENA_BREAK_109");
		const BOSS_PATTERN_DEFINITION* entranceCinematic =
			findPattern("VALTAN_ENTRANCE_CINEMATIC");
		const BOSS_PATTERN_STAGE_DEFINITION* entranceEstablish =
			findStage("VALTAN_ENTRANCE_CINEMATIC", "ESTABLISH");
		const BOSS_PATTERN_STAGE_DEFINITION* entranceArenaReveal =
			findStage("VALTAN_ENTRANCE_CINEMATIC", "ARENA_REVEAL");
		const BOSS_PATTERN_STAGE_DEFINITION* entranceHeroHandoff =
			findStage("VALTAN_ENTRANCE_CINEMATIC", "HERO_HANDOFF");
		const auto hasAction = [](
			const BOSS_PATTERN_STAGE_DEFINITION* stage,
			const BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			const BOSS_PATTERN_STAGE_ACTION_KIND kind,
			const std::string_view targetId,
			const std::uint32_t value)
		{
			return nullptr != stage && std::any_of(
				stage->Actions.begin(), stage->Actions.end(),
				[trigger, kind, targetId, value](
					const BOSS_PATTERN_STAGE_ACTION& action)
				{
					return action.eTrigger == trigger && action.eKind == kind &&
						action.strTargetId == targetId && action.iValue == value &&
						0u == action.iDurationMs;
				});
		};
		const auto hasBranch = [](
			const BOSS_PATTERN_STAGE_DEFINITION* stage,
			const BOSS_PATTERN_STAGE_OUTCOME outcome,
			const std::string_view nextActionId)
		{
			return nullptr != stage && std::any_of(
				stage->Branches.begin(), stage->Branches.end(),
				[outcome, nextActionId](
					const BOSS_PATTERN_STAGE_BRANCH& branch)
				{
					return branch.eOutcome == outcome &&
						branch.strNextActionId == nextActionId;
				});
		};
		const auto hasClosedFlag = [&hasAction](
			const BOSS_PATTERN_STAGE_DEFINITION* stage,
			const std::string_view targetId)
		{
			return nullptr != stage && 2u == stage->Actions.size() &&
				hasAction(stage, BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
					BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG,
					targetId, 1u) &&
				hasAction(stage, BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT,
					BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG,
					targetId, 0u);
		};
		std::size_t valtanLivePatternCount = 0u;
		std::size_t valtanStageCount = 0u;
		std::size_t valtanStageActionCount = 0u;
		std::size_t valtanStageVolleyActionCount = 0u;
		std::size_t valtanRuntimeBranchCount = 0u;
		std::size_t valtanMotionCount = 0u;
		if (nullptr != patterns)
		{
			for (const BOSS_PATTERN_DEFINITION& pattern : *patterns)
			{
				if (BOSS_PATTERN_SELECTION::AUDITION_ONLY == pattern.eSelection)
					continue;
				++valtanLivePatternCount;
				valtanStageCount += pattern.Stages.size();
				for (const BOSS_PATTERN_STAGE_DEFINITION& stage : pattern.Stages)
				{
					valtanStageActionCount += stage.Actions.size();
					valtanStageVolleyActionCount += static_cast<std::size_t>(std::count_if(
						stage.Actions.begin(), stage.Actions.end(), [](const BOSS_PATTERN_STAGE_ACTION& action)
						{ return BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY == action.eKind; }));
					valtanRuntimeBranchCount += stage.Branches.size();
					if (BOSS_PATTERN_STAGE_MOTION_KIND::NONE != stage.Motion.eKind)
						++valtanMotionCount;
				}
			}
		}
		const BOSS_PATTERN_STAGE_DEFINITION* parryStance =
			findStage("VALTAN_PARRY", "STANCE");
		const BOSS_PATTERN_STAGE_DEFINITION* parrySlash =
			findStage("VALTAN_PARRY", "COUNTER_SLASH");
		const BOSS_PATTERN_STAGE_DEFINITION* parryNormal =
			findStage("VALTAN_PARRY", "NORMAL_SLASH");
		const BOSS_PATTERN_STAGE_DEFINITION* tripleFirst =
			findStage("VALTAN_TRIPLE_COUNTER", "COUNTER_1");
		const BOSS_PATTERN_STAGE_DEFINITION* tripleSecond =
			findStage("VALTAN_TRIPLE_COUNTER", "COUNTER_2");
		const BOSS_PATTERN_STAGE_DEFINITION* tripleThird =
			findStage("VALTAN_TRIPLE_COUNTER", "COUNTER_3");
		const BOSS_PATTERN_STAGE_DEFINITION* armorCharge =
			findStage("VALTAN_ARMOR_BREAK_OPENING", "WALL_CHARGE");
		const BOSS_PATTERN_STAGE_DEFINITION* armorGroggy =
			findStage("VALTAN_ARMOR_BREAK_OPENING", "GROGGY");
		const BOSS_PATTERN_STAGE_DEFINITION* orbShield =
			findStage("VALTAN_MAGIC_ORB_STAGGER_76", "SHIELD");
		const BOSS_PATTERN_STAGE_DEFINITION* orbWindow =
			findStage("VALTAN_MAGIC_ORB_STAGGER_76", "STAGGER_WINDOW");
		const BOSS_PATTERN_STAGE_DEFINITION* orbGroggy =
			findStage("VALTAN_MAGIC_ORB_STAGGER_76", "GROGGY");
		const BOSS_PATTERN_STAGE_DEFINITION* orbWipe =
			findStage("VALTAN_MAGIC_ORB_STAGGER_76", "WIPE");
		const BOSS_PATTERN_STAGE_DEFINITION* centerCounter =
			findStage("VALTAN_CENTER_GRAB_COUNTER_64", "COUNTER_WINDOW");
		const BOSS_PATTERN_STAGE_DEFINITION* counterSlam =
			findStage("VALTAN_COUNTER", "STEP_03");
		const bool entranceCameraGateExact =
			nullptr != entranceCinematic &&
			BOSS_PATTERN_SELECTION::NORMAL == entranceCinematic->eSelection &&
			entranceCinematic->bInvulnerableWhileRunning &&
			BOSS_PATTERN_TARGET_POLICY::NONE ==
				entranceCinematic->eTargetPolicy &&
			BOSS_PATTERN_AIM_POLICY::NONE == entranceCinematic->eAimPolicy &&
			3u == entranceCinematic->Stages.size() &&
			nullptr != entranceEstablish &&
			8600u == entranceEstablish->iDurationMs &&
			BOSS_PATTERN_HIT_SHAPE::NONE == entranceEstablish->eHitShape &&
			entranceEstablish->Actions.empty() &&
			hasBranch(entranceEstablish, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.cinematic.entrance.arena-reveal") &&
			nullptr != entranceArenaReveal &&
			5800u == entranceArenaReveal->iDurationMs &&
			BOSS_PATTERN_HIT_SHAPE::NONE == entranceArenaReveal->eHitShape &&
			entranceArenaReveal->Actions.empty() &&
			hasBranch(entranceArenaReveal, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.cinematic.entrance.hero-handoff") &&
			nullptr != entranceHeroHandoff &&
			5467u == entranceHeroHandoff->iDurationMs &&
			BOSS_PATTERN_HIT_SHAPE::NONE == entranceHeroHandoff->eHitShape &&
			entranceHeroHandoff->Actions.empty() &&
			hasBranch(entranceHeroHandoff, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT, "");
		tests.Require(
			entranceCameraGateExact,
			"Load the exact invulnerable 19.867-second Valtan entrance camera gate");

		const bool reactiveTopologyExact = nullptr != patterns &&
			34u == valtanLivePatternCount && 132u == valtanStageCount &&
			// PATTERNSTAGEVOLLEY also materializes in Actions, in addition to
			// the 24 PATTERNSTAGEACTION rows in the published bootstrap.
			25u == valtanStageActionCount && 1u == valtanStageVolleyActionCount &&
			142u == valtanRuntimeBranchCount && 2u == valtanMotionCount &&
			nullptr != parryStance && 2u == parryStance->Actions.size() &&
			2u == parryStance->Branches.size() &&
			hasAction(parryStance,
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
				BOSS_PATTERN_STAGE_ACTION_KIND::SET_STAGGER_GAUGE,
				"boss.gauge.stagger", 30u) &&
			hasAction(parryStance,
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT,
				BOSS_PATTERN_STAGE_ACTION_KIND::SET_STAGGER_GAUGE,
				"boss.gauge.stagger", 0u) &&
			hasBranch(parryStance, BOSS_PATTERN_STAGE_OUTCOME::STAGGER_BROKEN,
				"valtan.reactive.parry.slash") &&
			hasBranch(parryStance, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.reactive.parry.normal-slash") &&
			nullptr != parrySlash && 1u == parrySlash->Branches.size() &&
			hasBranch(parrySlash, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.reactive.parry.recovery") &&
			nullptr != parryNormal && 1u == parryNormal->Branches.size() &&
			hasBranch(parryNormal, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.reactive.parry.recovery") &&
			hasClosedFlag(tripleFirst, "boss.flag.counterable") &&
			2u == tripleFirst->Branches.size() &&
			hasBranch(tripleFirst, BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT,
				"valtan.reactive.triple-counter.second") &&
			hasBranch(tripleFirst, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.reactive.triple-counter.first-fail") &&
			hasClosedFlag(tripleSecond, "boss.flag.counterable") &&
			2u == tripleSecond->Branches.size() &&
			hasBranch(tripleSecond, BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT,
				"valtan.reactive.triple-counter.third") &&
			hasBranch(tripleSecond, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.reactive.triple-counter.second-fail") &&
			hasClosedFlag(tripleThird, "boss.flag.counterable") &&
			2u == tripleThird->Branches.size() &&
			hasBranch(tripleThird, BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT,
				"valtan.reactive.triple-counter.recovery") &&
			hasBranch(tripleThird, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.reactive.triple-counter.third-fail") &&
			nullptr != armorCharge &&
			BOSS_PATTERN_STAGE_MOTION_KIND::FORWARD == armorCharge->Motion.eKind &&
			std::abs(armorCharge->Motion.fDistance - 20.f) < 1.0e-6f &&
			2u == armorCharge->Branches.size() &&
			hasBranch(armorCharge, BOSS_PATTERN_STAGE_OUTCOME::WALL_CONTACT,
				"valtan.mechanic.armor-break-opening.groggy") &&
			hasBranch(armorCharge, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT, "") &&
			hasClosedFlag(armorGroggy, "boss.flag.groggy") &&
			2u == armorGroggy->Branches.size() &&
			hasBranch(armorGroggy, BOSS_PATTERN_STAGE_OUTCOME::PART_DESTROYED,
				"valtan.mechanic.armor-break-opening.recovery") &&
			hasBranch(armorGroggy, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.mechanic.armor-break-opening.recovery") &&
			nullptr != orbShield && 2u == orbShield->Actions.size() &&
			hasAction(orbShield, BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
				BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG,
				"boss.flag.invulnerable", 1u) &&
			hasAction(orbShield, BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
				BOSS_PATTERN_STAGE_ACTION_KIND::SET_SHIELD,
				"boss.gauge.shield", 6000u) &&
			nullptr != orbWindow && 4u == orbWindow->Actions.size() &&
			2u == orbWindow->Branches.size() &&
			hasAction(orbWindow, BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
				BOSS_PATTERN_STAGE_ACTION_KIND::SET_STAGGER_GAUGE,
				"boss.gauge.stagger", 100u) &&
			hasAction(orbWindow, BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT,
				BOSS_PATTERN_STAGE_ACTION_KIND::SET_STAGGER_GAUGE,
				"boss.gauge.stagger", 0u) &&
			hasAction(orbWindow, BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT,
				BOSS_PATTERN_STAGE_ACTION_KIND::SET_SHIELD,
				"boss.gauge.shield", 0u) &&
			hasAction(orbWindow, BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT,
				BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG,
				"boss.flag.invulnerable", 0u) &&
			hasBranch(orbWindow, BOSS_PATTERN_STAGE_OUTCOME::STAGGER_BROKEN,
				"valtan.mechanic.magic-orb-stagger-76.groggy") &&
			hasBranch(orbWindow, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.mechanic.magic-orb-stagger-76.wipe") &&
			hasClosedFlag(orbGroggy, "boss.flag.groggy") &&
			1u == orbGroggy->Branches.size() &&
			hasBranch(orbGroggy, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.mechanic.magic-orb-stagger-76.recovery") &&
			nullptr != orbWipe && 1u == orbWipe->Branches.size() &&
			hasBranch(orbWipe, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.mechanic.magic-orb-stagger-76.recovery") &&
			hasClosedFlag(centerCounter, "boss.flag.counterable") &&
			2u == centerCounter->Branches.size() &&
			hasBranch(centerCounter, BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT,
				"valtan.mechanic.center-grab-counter-64.recovery") &&
			hasBranch(centerCounter, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.mechanic.center-grab-counter-64.failed-charge") &&
			nullptr != counterSlam && 1u == counterSlam->Branches.size() &&
			hasBranch(counterSlam, BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT, "");
		tests.Require(reactiveTopologyExact,
			"Load all 132 current Valtan stages with exactly 25 runtime actions (including one volley), 142 branches, two stage motions, and terminal counter slam");
		if (!reactiveTopologyExact)
		{
			std::cout << "[STATUS] Valtan topology counts: patterns=" << valtanLivePatternCount
				<< ", stages=" << valtanStageCount << ", actions=" << valtanStageActionCount
				<< ", volleys=" << valtanStageVolleyActionCount << ", branches=" << valtanRuntimeBranchCount
				<< ", motions=" << valtanMotionCount << '\n';
		}
		tests.Require(
			nullptr != fourSlashes && 3u == fourSlashes->iHitCount &&
			0u == fourSlashes->iHitDelayMs &&
			0u == fourSlashes->iHitIntervalMs &&
			std::vector<std::uint32_t>{ 1790u, 2560u, 3330u } ==
				fourSlashes->HitOffsetsMs &&
			nullptr != fourSlashSpin && 1u == fourSlashSpin->iHitCount &&
			0u == fourSlashSpin->iHitDelayMs &&
			0u == fourSlashSpin->iHitIntervalMs &&
			std::vector<std::uint32_t>{ 600u } ==
				fourSlashSpin->HitOffsetsMs &&
			fourSlashes->bWallContact && fourSlashSpin->bWallContact,
			"Compile the rejoined four-slash explicit Server hit schedule exactly");
		const bool hasExactHighJumpVolley = nullptr != highJumpAirborne &&
			1u == highJumpAirborne->Actions.size() &&
			BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER ==
				highJumpAirborne->Actions.front().eTrigger &&
			BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY ==
				highJumpAirborne->Actions.front().eKind &&
			"combatobject.valtan.high-jump.target-axe" ==
				highJumpAirborne->Actions.front().strTargetId &&
			1u == highJumpAirborne->Actions.front().iValue &&
			0u == highJumpAirborne->Actions.front().iDurationMs &&
			BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER ==
				highJumpAirborne->Actions.front().Volley.ePolicy &&
			1u == highJumpAirborne->Actions.front().Volley.iCountPerResolvedTarget &&
			BOSS_COMBAT_OBJECT_LAYOUT_KIND::SINGLE ==
				highJumpAirborne->Actions.front().Volley.eLayout &&
			0.f == highJumpAirborne->Actions.front().Volley.fRadiusM &&
			0.f == highJumpAirborne->Actions.front().Volley.fStartAngleDegrees &&
			0.f == highJumpAirborne->Actions.front().Volley.fAngleStepDegrees &&
			!highJumpAirborne->Actions.front().Volley.bAllowOverlap &&
			36u == highJumpAirborne->Actions.front().Volley.iMaximumTotalObjects &&
			3u == highJumpAirborne->Actions.front().Volley.iSpawnCount &&
			1333u == highJumpAirborne->Actions.front().Volley.iSpawnIntervalMs &&
			4u == highJumpAirborne->Actions.front().Volley.iArenaRandomCount &&
			std::abs(highJumpAirborne->Actions.front().Volley.
				fArenaRandomRadiusM - 14.f) < 1.0e-6f &&
			std::abs(highJumpAirborne->Actions.front().Volley.
				fArenaHeightToleranceM - 1.f) < 1.0e-6f &&
			BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::BOSS_SPAWN_POSITION ==
				highJumpAirborne->Actions.front().Volley.eArenaAnchorPolicy;
		tests.Require(
			nullptr != highJumpTakeoff &&
			1933u == highJumpTakeoff->iDurationMs &&
			nullptr != highJumpAirborne &&
			4000u == highJumpAirborne->iDurationMs &&
			hasExactHighJumpVolley &&
			nullptr != highJumpLand && 3200u == highJumpLand->iDurationMs &&
			nullptr != highJumpRecovery &&
			400u == highJumpRecovery->iDurationMs &&
			nullptr != highJumpTargetAxe &&
			4000u == highJumpTargetAxe->iLifeMs &&
			BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_PER_ALIVE_PLAYER ==
				highJumpTargetAxe->eOriginPolicy &&
			1u == highJumpTargetAxe->Hits.size() &&
			BOSS_COMBAT_OBJECT_HIT_TRIGGER::TIMED ==
				highJumpTargetAxe->Hits.front().eTrigger &&
			1200u == highJumpTargetAxe->Hits.front().iAtMs &&
			1u == highJumpTargetAxe->Hits.front().iRepeatCount,
			"Keep three mixed target-axe waves inside the 4-second AIRBORNE hold, hit each at 1.2 seconds, and leave LAND at 3.2 seconds");
		tests.Require(
			nullptr != fourSlashPattern && nullptr != swingPattern &&
			420609u == fourSlashPattern->iSourcePrimaryActionId &&
			fourSlashPattern->iSourcePrimaryActionId !=
				swingPattern->iSourcePrimaryActionId,
			"Compile one four-slash source-action cooldown family and keep other sources independent");
		tests.Require(
			nullptr != highJumpPattern &&
			2u == highJumpPattern->Motion.iTravelStageIndex &&
			nullptr != arenaBreakPattern &&
			1u == arenaBreakPattern->Motion.iTravelStageIndex,
			"Compile the authored high-jump LAND descent and arena-break DROP descent stage indices");
		const bool everyPatternHasSourceTiming = nullptr != patterns &&
			std::all_of(
				patterns->begin(), patterns->end(),
				[](const BOSS_PATTERN_DEFINITION& pattern)
				{
					return 0u != pattern.iSourcePrimaryActionId &&
						pattern.iSourceShapeCount <= 256u &&
						pattern.iSourceCooldownTicks ==
						static_cast<std::uint32_t>(
							(static_cast<std::uint64_t>(
								pattern.iSourceCooldownMs) * 30u + 999u) /
							1000u);
				});
		tests.Require(
			nullptr != swing && swing->bWallContact &&
			BOSS_PATTERN_HIT_SHAPE::CONE == swing->eHitShape &&
			nullptr != downSmash && downSmash->bWallContact &&
			BOSS_PATTERN_HIT_SHAPE::CROSS == downSmash->eHitShape &&
			downSmash->fHitLength >= 9.9f &&
			downSmash->fHitHalfWidth >= 1.7f &&
			nullptr != roar && !roar->bWallContact,
			"Compile the down-smash and other allowlisted physical axe stages as wall contacts");
		tests.Require(
			nullptr != floorWipe && !floorWipe->bWallContact &&
			BOSS_PATTERN_HIT_SHAPE::SIX_DIRECTIONS == floorWipe->eHitShape &&
			floorWipe->fHitLength >= 13.9f &&
			floorWipe->fHitHalfWidth >= 2.1f,
			"Compile the 130 floor wipe as six Server-authoritative directions");
		tests.Require(
			everyPatternHasSourceTiming && nullptr != swingPattern &&
			420601u == swingPattern->iSourcePrimaryActionId &&
			12u == swingPattern->iSourceShapeCount &&
			5000u == swingPattern->iSourceCooldownMs &&
			150u == swingPattern->iSourceCooldownTicks &&
			350u == swingPattern->iSourceRangeUnits &&
			300u == swingPattern->iSourceApproachUnits &&
			180u == swingPattern->iSourceTurnDegrees &&
			nullptr != arenaBreakPattern &&
			420629u == arenaBreakPattern->iSourcePrimaryActionId &&
			0u == arenaBreakPattern->iSourceCooldownTicks &&
			10000u == arenaBreakPattern->iSourceRangeUnits,
			"Compile all Valtan entry-action timing records from Valtan.skilltiming");
		std::uint32_t damagingStageCount = 0u;
		std::uint32_t authoredHitPulseCount = 0u;
		bool everyDamagingStageResolves = nullptr != patterns;
		if (nullptr != patterns)
		{
			for (const BOSS_PATTERN_DEFINITION& pattern : *patterns)
			{
				for (const BOSS_PATTERN_STAGE_DEFINITION& stage : pattern.Stages)
				{
					if (BOSS_PATTERN_HIT_SHAPE::NONE == stage.eHitShape)
						continue;
					++damagingStageCount;
					authoredHitPulseCount += stage.iHitCount;
					everyDamagingStageResolves = everyDamagingStageResolves &&
						!stage.strDamageProfileId.empty() &&
						0u != catalog.Find_DamageRatePercent(
							stage.strDamageProfileId);
				}
			}
		}
		tests.Require(
			everyDamagingStageResolves && 66u == damagingStageCount &&
			102u == authoredHitPulseCount &&
			700u == catalog.Find_DamageRatePercent(
				"damage.valtan.arena-destroy-109") &&
			450u == catalog.Find_DamageRatePercent(
				"damage.valtan.six-direction-130") &&
			900u == catalog.Find_DamageRatePercent(
				"damage.valtan.ghost-transition-15"),
			"Resolve all 66 Valtan hit stages and 102 pulses through project-tuned damage profiles");
	}
	{
		CClientSession session{ 90000u, INVALID_SOCKET, {}, {} };
		session.Record_InboundPacket(PACKET_TYPE::C2S_MOVE);
		session.Request_Close(
			SESSION_DIAGNOSTIC_REASON::SERVER_RECEIVE_ERROR,
			WSAECONNRESET,
			"first terminal cause");
		session.Request_Close(
			SESSION_DIAGNOSTIC_REASON::SERVER_APPLICATION_CLOSE,
			WSAEINVAL,
			"later cleanup must not overwrite");
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC diagnostic =
			session.Get_CloseDiagnostic();
		tests.Require(
			SESSION_DIAGNOSTIC_REASON::SERVER_RECEIVE_ERROR ==
				diagnostic.eReason &&
			PACKET_TYPE::C2S_MOVE == diagnostic.eLastInboundPacket &&
			WSAECONNRESET == diagnostic.iNativeErrorCode &&
			"first terminal cause" == diagnostic.strContext &&
			0u != diagnostic.iLastInboundUnixMilliseconds &&
			diagnostic.iLastInboundUnixMilliseconds ==
				session.Get_LastInboundUnixMilliseconds() &&
			0u != diagnostic.iOccurredUnixMilliseconds,
			"Preserve the first typed session terminal cause across later cleanup");
	}
	{
		auto app = std::make_unique<CServerApp>();
		auto protocolSession = std::make_shared<CClientSession>(
			89995u, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		auto malformedSession = std::make_shared<CClientSession>(
			89996u, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		auto unknownSession = std::make_shared<CClientSession>(
			89997u, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		app->m_Sessions.emplace(89995u, protocolSession);
		app->m_Sessions.emplace(89996u, malformedSession);
		app->m_Sessions.emplace(89997u, unknownSession);

		PACKET_FRAME protocolFrame{};
		protocolFrame.ePacketType = PACKET_TYPE::C2S_ENTER_WORLD;
		protocolFrame.Payload = { 0xffu, 0xffu, 1u, 0u };
		protocolSession->Record_InboundPacket(protocolFrame.ePacketType);
		app->On_SessionFrame(89995u, protocolFrame);

		PACKET_FRAME malformedFrame{};
		malformedFrame.ePacketType = PACKET_TYPE::C2S_MOVE;
		malformedSession->Record_InboundPacket(malformedFrame.ePacketType);
		app->On_SessionFrame(89996u, malformedFrame);

		PACKET_FRAME unknownFrame{};
		unknownFrame.ePacketType = static_cast<PACKET_TYPE>(0xffffu);
		unknownSession->Record_InboundPacket(unknownFrame.ePacketType);
		app->On_SessionFrame(89997u, unknownFrame);

		const CLIENT_SESSION_CLOSE_DIAGNOSTIC protocolDiagnostic =
			protocolSession->Get_CloseDiagnostic();
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC malformedDiagnostic =
			malformedSession->Get_CloseDiagnostic();
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC unknownDiagnostic =
			unknownSession->Get_CloseDiagnostic();
		tests.Require(
			SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_MESSAGE_DECODE_FAILED ==
				protocolDiagnostic.eReason &&
			WSAEPROTONOSUPPORT == protocolDiagnostic.iNativeErrorCode &&
			std::string::npos !=
				protocolDiagnostic.strContext.find("protocol mismatch") &&
			SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_MESSAGE_DECODE_FAILED ==
				malformedDiagnostic.eReason &&
			WSAEINVAL == malformedDiagnostic.iNativeErrorCode &&
			std::string::npos != malformedDiagnostic.strContext.find("C2S_MOVE") &&
			SESSION_DIAGNOSTIC_REASON::SERVER_UNKNOWN_PACKET ==
				unknownDiagnostic.eReason &&
			WSAEPROTONOSUPPORT == unknownDiagnostic.iNativeErrorCode,
			"Classify protocol mismatch, malformed message, and unknown packet separately");
	}
	{
		CGameRoom room{ WORLD_ID::TRAINING_GROUND };
		auto missingSkillBindingSession = std::make_shared<CClientSession>(
			89979u, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		auto missingBindingSession = std::make_shared<CClientSession>(
			89980u, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		auto staleSession = std::make_shared<CClientSession>(
			89981u, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		auto nonFiniteSession = std::make_shared<CClientSession>(
			89982u, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		auto outOfRangeSession = std::make_shared<CClientSession>(
			89983u, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		room.m_Sessions.emplace(89979u, missingSkillBindingSession);
		room.m_Sessions.emplace(89980u, missingBindingSession);
		room.m_Sessions.emplace(89981u, staleSession);
		room.m_Sessions.emplace(89982u, nonFiniteSession);
		room.m_Sessions.emplace(89983u, outOfRangeSession);
		const auto addPlayer = [&room](
			const SESSION_ID sessionId,
			const PLAYER_ID playerId,
			const std::uint32_t lastMoveSequence)
			{
				SERVER_PLAYER player{};
				player.iSessionId = sessionId;
				player.iPlayerId = playerId;
				player.iNetEntityId = playerId;
				player.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
				player.iLastMoveSequence = lastMoveSequence;
				player.iCurrentHp = 100u;
				player.iMaximumHp = 100u;
				room.m_Players.emplace(playerId, player);
				room.m_PlayerIdBySessionId.emplace(sessionId, playerId);
				room.m_PlayerIdByEntityId.emplace(playerId, playerId);
			};
		addPlayer(89981u, 89981u, 10u);
		addPlayer(89982u, 89982u, 0u);
		addPlayer(89983u, 89983u, 0u);

		C2S_USE_SKILL missingSkillBinding{};
		missingSkillBinding.iClientSequence = 1u;
		missingSkillBinding.iSkillId = 34010u;
		missingSkillBindingSession->Record_InboundPacket(
			PACKET_TYPE::C2S_USE_SKILL);
		room.Handle_UseSkill(89979u, missingSkillBinding);
		C2S_MOVE missingBindingMove{};
		missingBindingMove.iClientSequence = 1u;
		missingBindingSession->Record_InboundPacket(PACKET_TYPE::C2S_MOVE);
		room.Handle_Move(89980u, missingBindingMove);
		C2S_MOVE staleMove{};
		staleMove.iClientSequence = 10u;
		staleSession->Record_InboundPacket(PACKET_TYPE::C2S_MOVE);
		room.Handle_Move(89981u, staleMove);
		C2S_MOVE nonFiniteMove{};
		nonFiniteMove.iClientSequence = 1u;
		nonFiniteMove.fGoalX =
			(std::numeric_limits<float>::quiet_NaN)();
		nonFiniteSession->Record_InboundPacket(PACKET_TYPE::C2S_MOVE);
		room.Handle_Move(89982u, nonFiniteMove);
		C2S_MOVE outOfRangeMove{};
		outOfRangeMove.iClientSequence = 1u;
		outOfRangeMove.fGoalX = 10001.f;
		outOfRangeSession->Record_InboundPacket(PACKET_TYPE::C2S_MOVE);
		room.Handle_Move(89983u, outOfRangeMove);

		const CLIENT_SESSION_CLOSE_DIAGNOSTIC missingSkillBindingDiagnostic =
			missingSkillBindingSession->Get_CloseDiagnostic();
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC missingBindingDiagnostic =
			missingBindingSession->Get_CloseDiagnostic();
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC staleDiagnostic =
			staleSession->Get_CloseDiagnostic();
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC nonFiniteDiagnostic =
			nonFiniteSession->Get_CloseDiagnostic();
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC outOfRangeDiagnostic =
			outOfRangeSession->Get_CloseDiagnostic();
		tests.Require(
			SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED ==
				missingSkillBindingDiagnostic.eReason &&
			std::string::npos != missingSkillBindingDiagnostic.strContext.find(
				"packet=C2S_USE_SKILL validation=missing-player-binding") &&
			SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED ==
				missingBindingDiagnostic.eReason &&
			WSAENOTCONN == missingBindingDiagnostic.iNativeErrorCode &&
			std::string::npos != missingBindingDiagnostic.strContext.find(
				"validation=missing-player-binding") &&
			SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_COMMAND_VALIDATION_FAILED ==
				staleDiagnostic.eReason &&
			WSAEINVAL == staleDiagnostic.iNativeErrorCode &&
			std::string::npos != staleDiagnostic.strContext.find(
				"validation=stale-sequence") &&
			std::string::npos != staleDiagnostic.strContext.find(
				"receivedSequence=10 lastSequence=10") &&
			SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_COMMAND_VALIDATION_FAILED ==
				nonFiniteDiagnostic.eReason &&
			std::string::npos != nonFiniteDiagnostic.strContext.find(
				"validation=non-finite-goal") &&
			SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_COMMAND_VALIDATION_FAILED ==
				outOfRangeDiagnostic.eReason &&
			std::string::npos != outOfRangeDiagnostic.strContext.find(
				"validation=out-of-range-goal") &&
			PACKET_TYPE::C2S_MOVE == staleDiagnostic.eLastInboundPacket,
			"Classify decoded C2S_MOVE binding and command validation failures exactly");
	}
	{
		CServerApp app;
		auto simulation = std::make_shared<CGameRoom>(
			WORLD_ID::TRAINING_GROUND);
		app.m_SharedGameRooms.emplace(WORLD_ID::TRAINING_GROUND, simulation);
		app.m_pActiveGameplayGeneration =
			simulation->Get_ActiveGameplayGeneration();
		constexpr SESSION_ID cleanupFirstSessionId = 89984u;
		auto cleanupFirstSession = std::make_shared<CClientSession>(
			cleanupFirstSessionId, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		app.m_Sessions.emplace(cleanupFirstSessionId, cleanupFirstSession);
		ROOM_COMMAND cleanupFirst{};
		cleanupFirst.eType = ROOM_COMMAND_TYPE::LEAVE;
		cleanupFirst.iSessionId = cleanupFirstSessionId;
		cleanupFirst.eLeaveReason = PLAYER_DESPAWN_REASON::DISCONNECTED;
		const bool cleanupQueued =
			simulation->Enqueue(std::move(cleanupFirst));
		C2S_ENTER_WORLD enterWorld{};
		enterWorld.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
		enterWorld.eWorldId = WORLD_ID::TRAINING_GROUND;
		enterWorld.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
		enterWorld.strNickName = "CleanupFirstFixture";
		SESSION_DIAGNOSTIC_REASON cleanupFirstReason =
			SESSION_DIAGNOSTIC_REASON::NONE;
		int cleanupFirstNativeError = 0;
		std::string cleanupFirstContext;
		const bool entryRejectedBehindCleanup =
			!app.Bind_AndEnqueueEntry(
				cleanupFirstSessionId,
				WORLD_ID::TRAINING_GROUND,
				simulation,
				enterWorld,
				cleanupFirstReason,
				cleanupFirstNativeError,
				cleanupFirstContext);
		const bool noEntryCommittedBehindCleanup =
			!app.m_GameplayBindingBySessionId.contains(cleanupFirstSessionId) &&
			simulation->m_InboundCommands.empty() &&
			1u == simulation->m_CleanupCommands.size();
		simulation->Tick(1.f / 30.f);

		constexpr SESSION_ID alreadyClosingSessionId = 89985u;
		auto alreadyClosingSession = std::make_shared<CClientSession>(
			alreadyClosingSessionId, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		alreadyClosingSession->Request_Close(
			SESSION_DIAGNOSTIC_REASON::SERVER_PEER_CLOSED,
			0,
			"entry race fixture closed first");
		app.m_Sessions.emplace(alreadyClosingSessionId, alreadyClosingSession);
		enterWorld.strNickName = "AlreadyClosingFixture";
		SESSION_DIAGNOSTIC_REASON alreadyClosingReason =
			SESSION_DIAGNOSTIC_REASON::NONE;
		int alreadyClosingNativeError = 0;
		std::string alreadyClosingContext;
		const bool terminalRecheckRejectedEntry =
			!app.Bind_AndEnqueueEntry(
				alreadyClosingSessionId,
				WORLD_ID::TRAINING_GROUND,
				simulation,
				enterWorld,
				alreadyClosingReason,
				alreadyClosingNativeError,
				alreadyClosingContext);
		tests.Require(
			cleanupQueued && entryRejectedBehindCleanup &&
			noEntryCommittedBehindCleanup &&
			SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_INGRESS_OVERFLOW ==
				cleanupFirstReason &&
			WSAENOBUFS == cleanupFirstNativeError &&
			std::string::npos != cleanupFirstContext.find(
				"stage=register-ingress") &&
			simulation->m_CleanupCommands.empty() &&
			simulation->m_QueuedCleanupSessionIds.empty() &&
			terminalRecheckRejectedEntry &&
			SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED ==
				alreadyClosingReason &&
			WSAESHUTDOWN == alreadyClosingNativeError &&
			std::string::npos != alreadyClosingContext.find(
				"stage=terminal-recheck") &&
			!app.m_GameplayBindingBySessionId.contains(
				alreadyClosingSessionId) &&
			SESSION_DIAGNOSTIC_REASON::SERVER_PEER_CLOSED ==
				alreadyClosingSession->Get_CloseDiagnostic().eReason,
			"Reject REGISTER and ENTER behind queued or terminal session cleanup");
	}
	{
		std::array<wchar_t, 32768u> modulePath{};
		const DWORD moduleLength = ::GetModuleFileNameW(
			nullptr, modulePath.data(), static_cast<DWORD>(modulePath.size()));
		const std::filesystem::path diagnosticPath =
			0u == moduleLength || moduleLength >= modulePath.size() ?
			std::filesystem::path{} :
			std::filesystem::path{ modulePath.data() }.parent_path() /
				L"Diagnostics" /
				(L"server-session-" +
					std::to_wstring(::GetCurrentProcessId()) + L".jsonl");
		std::error_code removeError;
		if (!diagnosticPath.empty())
			std::filesystem::remove(diagnosticPath, removeError);

		auto app = std::make_unique<CServerApp>();
		auto session = std::make_shared<CClientSession>(
			89994u, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		constexpr PLAYER_ID diagnosticPlayerId = 90094u;
		constexpr NET_ENTITY_ID diagnosticEntityId = 90194u;
		session->Bind_PlayerId(diagnosticPlayerId);
		session->Record_InboundPacket(PACKET_TYPE::C2S_MOVE);
		session->Request_Close(
			SESSION_DIAGNOSTIC_REASON::SERVER_RECEIVE_ERROR,
			WSAECONNRESET,
			"JSON schema contract fixture");
		auto simulation = std::make_shared<CGameRoom>(
			WORLD_ID::TRAINING_GROUND);
		SERVER_PLAYER diagnosticPlayer{};
		diagnosticPlayer.iSessionId = 89994u;
		diagnosticPlayer.iPlayerId = diagnosticPlayerId;
		diagnosticPlayer.iNetEntityId = diagnosticEntityId;
		diagnosticPlayer.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
		diagnosticPlayer.iCurrentHp = 100u;
		diagnosticPlayer.iMaximumHp = 100u;
		simulation->m_Sessions.emplace(89994u, session);
		simulation->m_Players.emplace(diagnosticPlayerId, diagnosticPlayer);
		simulation->m_PlayerIdBySessionId.emplace(
			89994u, diagnosticPlayerId);
		simulation->m_PlayerIdByEntityId.emplace(
			diagnosticEntityId, diagnosticPlayerId);
		CServerApp::SESSION_GAMEPLAY_BINDING binding{};
		binding.eWorldId = WORLD_ID::TRAINING_GROUND;
		binding.pSimulation = simulation;
		app->m_Sessions.emplace(89994u, session);
		app->m_GameplayBindingBySessionId.emplace(89994u, binding);
		app->On_SessionClosed(89994u);
		const bool cleanupGuaranteedForBoundClose =
			!app->m_GameplayBindingBySessionId.contains(89994u) &&
			1u == simulation->m_CleanupCommands.size() &&
			1u == simulation->m_QueuedCleanupSessionIds.size();

		std::string jsonLine;
		if (!diagnosticPath.empty())
		{
			std::ifstream log{ diagnosticPath, std::ios::binary };
			(void)std::getline(log, jsonLine);
		}
		std::string jsonStatus;
		const bool parsed = CServerApp::Validate_ServerSessionDiagnosticJson(
			jsonLine, jsonStatus);
		const bool schemaExact = std::string::npos != jsonLine.find(
			"\"schema\":\"lostark.server-session-diagnostic\"") &&
			std::string::npos != jsonLine.find("\"formatVersion\":1") &&
			std::string::npos == jsonLine.find("\"schemaVersion\"") &&
			std::string::npos != jsonLine.find("\"wasBound\":true") &&
			std::string::npos != jsonLine.find("\"leaveEnqueued\":true");
		simulation->Tick(1.f / 30.f);
		const bool cleanupReleasedBoundPlayer =
			simulation->m_CleanupCommands.empty() &&
			simulation->m_QueuedCleanupSessionIds.empty() &&
			!simulation->m_Sessions.contains(89994u) &&
			!simulation->m_Players.contains(diagnosticPlayerId) &&
			!simulation->m_PlayerIdBySessionId.contains(89994u) &&
			!simulation->m_PlayerIdByEntityId.contains(diagnosticEntityId) &&
			INVALID_PLAYER_ID == session->Get_PlayerId();
		if (!diagnosticPath.empty())
			std::filesystem::remove(diagnosticPath, removeError);
		tests.Require(
			!diagnosticPath.empty() && parsed && schemaExact &&
			jsonStatus.empty() && cleanupGuaranteedForBoundClose &&
			cleanupReleasedBoundPlayer,
			"Write the versioned diagnostic and guarantee cleanup for a bound close");
	}
	{
		CWinSockContext winSock;
		SOCKET sessionSocket = INVALID_SOCKET;
		SOCKET peerSocket = INVALID_SOCKET;
		const bool socketReady = winSock.Initialize() &&
			Create_LoopbackSocketPair(sessionSocket, peerSocket);
		std::atomic_uint32_t closedCount{ 0u };
		std::unique_ptr<CClientSession> session;
		if (socketReady)
		{
			session = std::make_unique<CClientSession>(
				89998u,
				sessionSocket,
				CClientSession::FRAME_HANDLER{},
				[&closedCount](const SESSION_ID) { ++closedCount; });
			sessionSocket = INVALID_SOCKET;
		}
		const bool started = socketReady && session->Start();
		CLIENT_SESSION_PEER_ENDPOINT peerEndpoint{};
		if (started)
		{
			peerEndpoint = session->Get_PeerEndpoint();
			Close_TestSocket(peerSocket);
			(void)Wait_Until(
				std::chrono::milliseconds(1500),
				[&closedCount]() { return 1u == closedCount.load(); });
		}
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC diagnostic =
			nullptr == session ? CLIENT_SESSION_CLOSE_DIAGNOSTIC{} :
			session->Get_CloseDiagnostic();
		if (session)
			session->Stop();
		session.reset();
		Close_TestSocket(sessionSocket);
		Close_TestSocket(peerSocket);
		tests.Require(
			started && 1u == closedCount.load() &&
			"127.0.0.1" == peerEndpoint.strAddress &&
			0u != peerEndpoint.iPort &&
			SESSION_DIAGNOSTIC_REASON::SERVER_PEER_CLOSED ==
				diagnostic.eReason &&
			0 == diagnostic.iNativeErrorCode,
			"Capture immutable peer endpoint and an orderly peer-close reason");
	}
	{
		CWinSockContext winSock;
		SOCKET sessionSocket = INVALID_SOCKET;
		SOCKET peerSocket = INVALID_SOCKET;
		const bool socketReady = winSock.Initialize() &&
			Create_LoopbackSocketPair(sessionSocket, peerSocket);
		std::atomic_uint32_t closedCount{ 0u };
		std::unique_ptr<CClientSession> session;
		if (socketReady)
		{
			session = std::make_unique<CClientSession>(
				89999u,
				sessionSocket,
				CClientSession::FRAME_HANDLER{},
				[&closedCount](const SESSION_ID) { ++closedCount; });
			sessionSocket = INVALID_SOCKET;
		}
		const bool started = socketReady && session->Start();
		bool invalidFrameSent = false;
		if (started)
		{
			const std::array<std::uint8_t, PACKET_HEADER_BYTES> invalidFrame
			{
				5u, 0u, 0u, 0u,
				static_cast<std::uint8_t>(PACKET_TYPE::C2S_ENTER_WORLD), 0u
			};
			invalidFrameSent = static_cast<int>(invalidFrame.size()) == ::send(
				peerSocket,
				reinterpret_cast<const char*>(invalidFrame.data()),
				static_cast<int>(invalidFrame.size()),
				0);
			(void)Wait_Until(
				std::chrono::milliseconds(1500),
				[&closedCount]() { return 1u == closedCount.load(); });
		}
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC diagnostic =
			nullptr == session ? CLIENT_SESSION_CLOSE_DIAGNOSTIC{} :
			session->Get_CloseDiagnostic();
		if (session)
			session->Stop();
		session.reset();
		Close_TestSocket(sessionSocket);
		Close_TestSocket(peerSocket);
		tests.Require(
			started && invalidFrameSent && 1u == closedCount.load() &&
			SESSION_DIAGNOSTIC_REASON::SERVER_INVALID_FRAME ==
				diagnostic.eReason &&
			WSAEPROTONOSUPPORT == diagnostic.iNativeErrorCode &&
			PACKET_TYPE::INVALID == diagnostic.eLastInboundPacket,
			"Classify an invalid TCP frame before closing only its session");
	}
	{
		CClientSession session{ 90001u, INVALID_SOCKET, {}, {} };
		session.m_isSendRunning.store(true);
		const auto firstSnapshot = session.Queue_OutboundFrame(
			PACKET_TYPE::S2C_WORLD_SNAPSHOT, { 1u });
		const auto firstReliable = session.Queue_OutboundFrame(
			PACKET_TYPE::S2C_CHAT, { 10u });
		const auto secondSnapshot = session.Queue_OutboundFrame(
			PACKET_TYPE::S2C_WORLD_SNAPSHOT, { 2u });
		const auto secondReliable = session.Queue_OutboundFrame(
			PACKET_TYPE::S2C_PLAYER_DESPAWNED, { 11u });
		const auto thirdSnapshot = session.Queue_OutboundFrame(
			PACKET_TYPE::S2C_WORLD_SNAPSHOT, { 3u });
		const CLIENT_SESSION_OUTBOUND_METRICS metrics =
			session.Get_OutboundMetrics();
		const bool latestWinsAndReliableOrder =
			CClientSession::OUTBOUND_ENQUEUE_RESULT::QUEUED == firstSnapshot &&
			CClientSession::OUTBOUND_ENQUEUE_RESULT::QUEUED == firstReliable &&
			CClientSession::OUTBOUND_ENQUEUE_RESULT::COALESCED == secondSnapshot &&
			CClientSession::OUTBOUND_ENQUEUE_RESULT::QUEUED == secondReliable &&
			CClientSession::OUTBOUND_ENQUEUE_RESULT::COALESCED == thirdSnapshot &&
			3u == session.m_OutboundFrames.size() &&
			PACKET_TYPE::S2C_CHAT ==
				session.m_OutboundFrames[0u].ePacketType &&
			10u == session.m_OutboundFrames[0u].Bytes.front() &&
			PACKET_TYPE::S2C_PLAYER_DESPAWNED ==
				session.m_OutboundFrames[1u].ePacketType &&
			11u == session.m_OutboundFrames[1u].Bytes.front() &&
			PACKET_TYPE::S2C_WORLD_SNAPSHOT ==
				session.m_OutboundFrames[2u].ePacketType &&
			3u == session.m_OutboundFrames[2u].Bytes.front() &&
			3u == metrics.iSnapshotEnqueuedFrameCount &&
			2u == metrics.iSnapshotCoalescedFrameCount &&
			2u == metrics.iReliableEnqueuedFrameCount;
		session.Request_Close();
		tests.Require(
			latestWinsAndReliableOrder,
			"Coalesce queued snapshots to latest-wins without reordering reliable frames");
	}
	{
		CClientSession session{ 90002u, INVALID_SOCKET, {}, {} };
		session.m_isSendRunning.store(true);
		constexpr std::size_t SNAPSHOT_FRAME_LIMIT =
			CClientSession::MAX_OUTBOUND_FRAME_COUNT -
			CClientSession::RELIABLE_FRAME_RESERVE;
		bool admittedSnapshotBand = true;
		for (std::size_t index = 0u; index < SNAPSHOT_FRAME_LIMIT; ++index)
		{
			admittedSnapshotBand = admittedSnapshotBand &&
				CClientSession::OUTBOUND_ENQUEUE_RESULT::QUEUED ==
				session.Queue_OutboundFrame(
					PACKET_TYPE::S2C_CHAT,
					{ static_cast<std::uint8_t>(index) });
		}
		const auto droppedSnapshot = session.Queue_OutboundFrame(
			PACKET_TYPE::S2C_WORLD_SNAPSHOT, { 1u });
		bool admittedReliableReserve = true;
		while (session.m_OutboundFrames.size() <
			CClientSession::MAX_OUTBOUND_FRAME_COUNT)
		{
			admittedReliableReserve = admittedReliableReserve &&
				CClientSession::OUTBOUND_ENQUEUE_RESULT::QUEUED ==
				session.Queue_OutboundFrame(
					PACKET_TYPE::S2C_CHAT, { 2u });
		}
		const auto overflow = session.Queue_OutboundFrame(
			PACKET_TYPE::S2C_CHAT, { 3u });
		const bool queuePreservedOnOverflow =
			CClientSession::MAX_OUTBOUND_FRAME_COUNT ==
				session.m_OutboundFrames.size();
		const std::array<std::uint8_t, 1u> payload{ 4u };
		const bool publicOverflowFailedClosed =
			!session.Send_Frame(PACKET_TYPE::S2C_CHAT, payload) &&
			!session.m_isSendRunning.load() &&
			session.m_OutboundFrames.empty();
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC closeDiagnostic =
			session.Get_CloseDiagnostic();
		const CLIENT_SESSION_OUTBOUND_METRICS metrics =
			session.Get_OutboundMetrics();
		tests.Require(
			admittedSnapshotBand && admittedReliableReserve &&
			CClientSession::OUTBOUND_ENQUEUE_RESULT::DROPPED_SNAPSHOT ==
				droppedSnapshot &&
			CClientSession::OUTBOUND_ENQUEUE_RESULT::RELIABLE_OVERFLOW ==
				overflow &&
			queuePreservedOnOverflow && publicOverflowFailedClosed &&
			SESSION_DIAGNOSTIC_REASON::SERVER_RELIABLE_OUTBOUND_OVERFLOW ==
				closeDiagnostic.eReason &&
			WSAENOBUFS == closeDiagnostic.iNativeErrorCode &&
			CClientSession::MAX_OUTBOUND_FRAME_COUNT ==
				closeDiagnostic.iQueuedFrameCountAtClose &&
			1u == metrics.iSnapshotDroppedFrameCount &&
			2u == metrics.iReliableRejectedFrameCount &&
			CClientSession::MAX_OUTBOUND_FRAME_COUNT ==
				metrics.iQueuedFrameHighWatermark,
			"Reserve outbound capacity for reliable FIFO and fail-close only the overflowing session");
	}
	{
		CWinSockContext winSock;
		const bool initialized = winSock.Initialize();
		SOCKET slowSessionSocket = INVALID_SOCKET;
		SOCKET slowPeerSocket = INVALID_SOCKET;
		SOCKET fastSessionSocket = INVALID_SOCKET;
		SOCKET fastPeerSocket = INVALID_SOCKET;
		bool socketsReady = initialized &&
			Create_LoopbackSocketPair(slowSessionSocket, slowPeerSocket) &&
			Create_LoopbackSocketPair(fastSessionSocket, fastPeerSocket);
		if (socketsReady)
		{
			const int smallBufferBytes = 1024;
			socketsReady =
				SOCKET_ERROR != ::setsockopt(
					slowSessionSocket,
					SOL_SOCKET,
					SO_SNDBUF,
					reinterpret_cast<const char*>(&smallBufferBytes),
					static_cast<int>(sizeof(smallBufferBytes))) &&
				SOCKET_ERROR != ::setsockopt(
					slowPeerSocket,
					SOL_SOCKET,
					SO_RCVBUF,
					reinterpret_cast<const char*>(&smallBufferBytes),
					static_cast<int>(sizeof(smallBufferBytes)));
		}

		std::atomic_uint32_t slowClosedCount{ 0u };
		std::atomic_uint32_t fastClosedCount{ 0u };
		std::unique_ptr<CClientSession> slowSession;
		std::unique_ptr<CClientSession> fastSession;
		if (socketsReady)
		{
			slowSession = std::make_unique<CClientSession>(
				90003u,
				slowSessionSocket,
				CClientSession::FRAME_HANDLER{},
				[&slowClosedCount](const SESSION_ID)
				{
					++slowClosedCount;
				});
			slowSessionSocket = INVALID_SOCKET;
			fastSession = std::make_unique<CClientSession>(
				90004u,
				fastSessionSocket,
				CClientSession::FRAME_HANDLER{},
				[&fastClosedCount](const SESSION_ID)
				{
					++fastClosedCount;
				});
			fastSessionSocket = INVALID_SOCKET;
		}

		const bool sessionsStarted = socketsReady &&
			slowSession->Start() && fastSession->Start();
		bool slowEnqueueStayedBounded = false;
		bool slowCoalescedSnapshots = false;
		bool fastReliableFifo = false;
		bool slowClosureWasIsolated = false;
		bool cleanShutdownWasBounded = false;
		if (sessionsStarted)
		{
			std::vector<std::uint8_t> snapshotPayload(
				static_cast<std::size_t>(MAX_PACKET_BYTES) -
					PACKET_HEADER_BYTES,
				0xA5u);
			const auto enqueueStart = std::chrono::steady_clock::now();
			std::size_t acceptedSnapshotCount = 0u;
			for (std::size_t index = 0u; index < 256u; ++index)
			{
				snapshotPayload.front() = static_cast<std::uint8_t>(index);
				if (!slowSession->Send_Frame(
					PACKET_TYPE::S2C_WORLD_SNAPSHOT,
					snapshotPayload))
				{
					break;
				}
				++acceptedSnapshotCount;
			}
			const auto enqueueElapsed = std::chrono::steady_clock::now() -
				enqueueStart;
			const CLIENT_SESSION_OUTBOUND_METRICS slowMetrics =
				slowSession->Get_OutboundMetrics();
			slowEnqueueStayedBounded = acceptedSnapshotCount > 1u &&
				enqueueElapsed < std::chrono::milliseconds(1500);
			slowCoalescedSnapshots =
				0u < slowMetrics.iSnapshotCoalescedFrameCount &&
				1u >= slowMetrics.iCurrentQueuedFrameCount;

			const std::array<std::uint8_t, 1u> firstPayload{ 21u };
			const std::array<std::uint8_t, 1u> secondPayload{ 22u };
			const std::array<std::uint8_t, 1u> thirdPayload{ 23u };
			const bool queuedFastFrames =
				fastSession->Send_Frame(PACKET_TYPE::S2C_CHAT, firstPayload) &&
				fastSession->Send_Frame(
					PACKET_TYPE::S2C_ENTER_REJECTED, secondPayload) &&
				fastSession->Send_Frame(
					PACKET_TYPE::S2C_PLAYER_DESPAWNED, thirdPayload);
			RECEIVED_TEST_FRAME receivedFirst{};
			RECEIVED_TEST_FRAME receivedSecond{};
			RECEIVED_TEST_FRAME receivedThird{};
			fastReliableFifo = queuedFastFrames &&
				Receive_TestFrame(fastPeerSocket, receivedFirst) &&
				Receive_TestFrame(fastPeerSocket, receivedSecond) &&
				Receive_TestFrame(fastPeerSocket, receivedThird) &&
				PACKET_TYPE::S2C_CHAT == receivedFirst.packetType &&
				1u == receivedFirst.payload.size() &&
				firstPayload.front() == receivedFirst.payload.front() &&
				PACKET_TYPE::S2C_ENTER_REJECTED == receivedSecond.packetType &&
				1u == receivedSecond.payload.size() &&
				secondPayload.front() == receivedSecond.payload.front() &&
				PACKET_TYPE::S2C_PLAYER_DESPAWNED == receivedThird.packetType &&
				1u == receivedThird.payload.size() &&
				thirdPayload.front() == receivedThird.payload.front();

			Close_TestSocket(slowPeerSocket);
			const bool slowSessionClosed = Wait_Until(
				std::chrono::milliseconds(1500),
				[&slowClosedCount]()
				{
					return 0u < slowClosedCount.load();
				});
			const std::array<std::uint8_t, 1u> survivorPayload{ 24u };
			RECEIVED_TEST_FRAME survivorFrame{};
			slowClosureWasIsolated = slowSessionClosed &&
				fastSession->Send_Frame(PACKET_TYPE::S2C_CHAT, survivorPayload) &&
				Receive_TestFrame(fastPeerSocket, survivorFrame) &&
				PACKET_TYPE::S2C_CHAT == survivorFrame.packetType &&
				1u == survivorFrame.payload.size() &&
				survivorPayload.front() == survivorFrame.payload.front();

			const auto shutdownStart = std::chrono::steady_clock::now();
			slowSession->Stop();
			fastSession->Stop();
			const auto shutdownElapsed = std::chrono::steady_clock::now() -
				shutdownStart;
			cleanShutdownWasBounded =
				shutdownElapsed < std::chrono::milliseconds(3000) &&
				1u == slowClosedCount.load() &&
				1u == fastClosedCount.load();
		}
		else
		{
			if (slowSession)
				slowSession->Stop();
			if (fastSession)
				fastSession->Stop();
		}

		slowSession.reset();
		fastSession.reset();
		Close_TestSocket(slowSessionSocket);
		Close_TestSocket(slowPeerSocket);
		Close_TestSocket(fastSessionSocket);
		Close_TestSocket(fastPeerSocket);
		tests.Require(
			initialized && socketsReady && sessionsStarted &&
			slowEnqueueStayedBounded && slowCoalescedSnapshots &&
			fastReliableFifo && slowClosureWasIsolated &&
			cleanShutdownWasBounded,
			"Isolate a slow reader while preserving another session FIFO and bounded shutdown");
	}
	{
		CWinSockContext winSock;
		SOCKET sessionSocket = INVALID_SOCKET;
		SOCKET peerSocket = INVALID_SOCKET;
		bool socketReady = winSock.Initialize() &&
			Create_LoopbackSocketPair(sessionSocket, peerSocket);
		if (socketReady)
		{
			const int smallBufferBytes = 1024;
			socketReady = SOCKET_ERROR != ::setsockopt(
				sessionSocket, SOL_SOCKET, SO_SNDBUF,
				reinterpret_cast<const char*>(&smallBufferBytes),
				static_cast<int>(sizeof(smallBufferBytes))) &&
				SOCKET_ERROR != ::setsockopt(
					peerSocket, SOL_SOCKET, SO_RCVBUF,
					reinterpret_cast<const char*>(&smallBufferBytes),
					static_cast<int>(sizeof(smallBufferBytes)));
		}
		std::unique_ptr<CClientSession> session;
		bool sendTimeoutConfigured = false;
		if (socketReady)
		{
			session = std::make_unique<CClientSession>(
				90008u,
				sessionSocket,
				CClientSession::FRAME_HANDLER{},
				CClientSession::CLOSED_HANDLER{});
			sessionSocket = INVALID_SOCKET;
			sendTimeoutConfigured = session->Configure_SendTimeout();
		}

		bool slowReaderTimedOut = false;
		bool shutdownWasBounded = false;
		if (sendTimeoutConfigured)
		{
			session->m_isSendRunning.store(true);
			std::vector<std::uint8_t> bytes(
				static_cast<std::size_t>(MAX_PACKET_BYTES), 0x5au);
			bool sendFailed = false;
			for (std::uint32_t index = 0u; index < 64u; ++index)
			{
				bytes.front() = static_cast<std::uint8_t>(index);
				if (!session->Send_All(bytes))
				{
					sendFailed = true;
					break;
				}
			}
			const CLIENT_SESSION_CLOSE_DIAGNOSTIC diagnostic =
				session->Get_CloseDiagnostic();
			slowReaderTimedOut = sendFailed &&
				SESSION_DIAGNOSTIC_REASON::SERVER_SEND_ERROR_OR_TIMEOUT ==
					diagnostic.eReason &&
				0 != diagnostic.iNativeErrorCode;
			const auto shutdownStart = std::chrono::steady_clock::now();
			session->Stop();
			shutdownWasBounded =
				std::chrono::steady_clock::now() - shutdownStart <
				std::chrono::milliseconds(3000);
		}
		else if (session)
		{
			session->Stop();
		}

		session.reset();
		Close_TestSocket(sessionSocket);
		Close_TestSocket(peerSocket);
		tests.Require(
			socketReady && sendTimeoutConfigured && slowReaderTimedOut &&
			shutdownWasBounded,
			"Classify a non-draining peer as an isolated bounded send timeout");
	}
	{
		CWinSockContext winSock;
		SOCKET sessionSocket = INVALID_SOCKET;
		SOCKET peerSocket = INVALID_SOCKET;
		const bool socketReady = winSock.Initialize() &&
			Create_LoopbackSocketPair(sessionSocket, peerSocket);
		std::atomic_uint32_t closedCount{ 0u };
		std::unique_ptr<CClientSession> session;
		if (socketReady)
		{
			session = std::make_unique<CClientSession>(
				90006u,
				sessionSocket,
				CClientSession::FRAME_HANDLER{},
				[&closedCount](const SESSION_ID)
				{
					++closedCount;
				});
			sessionSocket = INVALID_SOCKET;
		}

		const bool started = socketReady && session->Start();
		bool reliableFlushedBeforeClose = false;
		bool rejectedLateEnqueue = false;
		bool shutdownWasBounded = false;
		if (started)
		{
			const std::array<std::uint8_t, 1u> payload{ 0x46u };
			const bool queued = session->Send_Frame(
				PACKET_TYPE::S2C_ENTER_REJECTED, payload);
			session->Request_Close_After_Flush();
			RECEIVED_TEST_FRAME received{};
			reliableFlushedBeforeClose = queued &&
				Receive_TestFrame(peerSocket, received) &&
				PACKET_TYPE::S2C_ENTER_REJECTED == received.packetType &&
				1u == received.payload.size() &&
				payload.front() == received.payload.front() &&
				Wait_Until(
					std::chrono::milliseconds(1500),
					[&session, &closedCount]()
					{
						return 1u == closedCount.load() &&
							!session->m_isSendRunning.load();
					});
			rejectedLateEnqueue = !session->Send_Frame(
				PACKET_TYPE::S2C_CHAT, payload);
			const auto shutdownStart = std::chrono::steady_clock::now();
			session->Stop();
			shutdownWasBounded =
				std::chrono::steady_clock::now() - shutdownStart <
					std::chrono::milliseconds(3000);
		}
		else if (session)
		{
			session->Stop();
		}

		session.reset();
		Close_TestSocket(sessionSocket);
		Close_TestSocket(peerSocket);
		tests.Require(
			started && reliableFlushedBeforeClose && rejectedLateEnqueue &&
			shutdownWasBounded && 1u == closedCount.load(),
			"Flush a terminal reliable frame before sender-owned graceful close");
	}
	{
		std::atomic_uint32_t closedCount{ 0u };
		CClientSession session{
			90007u,
			INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{},
			[&closedCount](const SESSION_ID)
			{
				++closedCount;
			} };
		session.m_isSendRunning.store(true);
		session.Request_Close_After_Flush();
		session.Request_Close();
		session.Sender_Loop();
		session.Notify_Closed();
		tests.Require(
			1u == closedCount.load() &&
			!session.m_isSendRunning.load() &&
			session.m_hasSenderExited,
			"Notify close exactly once when a hard stop overtakes graceful flush");
	}
	{
		CWinSockContext winSock;
		SOCKET sessionSocket = INVALID_SOCKET;
		SOCKET peerSocket = INVALID_SOCKET;
		const bool socketReady = winSock.Initialize() &&
			Create_LoopbackSocketPair(sessionSocket, peerSocket);
		std::atomic_uint32_t closedCount{ 0u };
		std::unique_ptr<CClientSession> session;
		bool senderStarted = false;
		if (socketReady)
		{
			session = std::make_unique<CClientSession>(
				90005u,
				sessionSocket,
				CClientSession::FRAME_HANDLER{},
				[&closedCount](const SESSION_ID)
				{
					++closedCount;
				});
			sessionSocket = INVALID_SOCKET;
			senderStarted = session->Configure_SendTimeout();
			if (senderStarted)
			{
				{
					std::scoped_lock lock{ session->m_OutboundMutex };
					session->m_hasSenderExited = false;
				}
				session->m_isSendRunning.store(true);
				try
				{
					session->m_SendThread = std::thread(
						&CClientSession::Sender_Loop,
						session.get());
				}
				catch (...)
				{
					senderStarted = false;
				}
			}
		}

		bool sendFailureWasIsolated = false;
		bool sendFailureWasClassified = false;
		bool shutdownWasBounded = false;
		if (senderStarted)
		{
			Abort_TestSocket(peerSocket);
			const std::array<std::uint8_t, 1u> payload{ 31u };
			const bool queued = session->Send_Frame(
				PACKET_TYPE::S2C_CHAT, payload);
			sendFailureWasIsolated = queued && Wait_Until(
				std::chrono::milliseconds(1500),
				[&session, &closedCount]()
				{
					return 0u < session->Get_OutboundMetrics().iSendFailureCount &&
					1u == closedCount.load() &&
					!session->m_isSendRunning.load();
				});
			const CLIENT_SESSION_CLOSE_DIAGNOSTIC diagnostic =
				session->Get_CloseDiagnostic();
			sendFailureWasClassified =
				SESSION_DIAGNOSTIC_REASON::SERVER_SEND_ERROR_OR_TIMEOUT ==
					diagnostic.eReason &&
				0 != diagnostic.iNativeErrorCode &&
				!diagnostic.strContext.empty();
			const auto shutdownStart = std::chrono::steady_clock::now();
			session->Stop();
			shutdownWasBounded =
				std::chrono::steady_clock::now() - shutdownStart <
					std::chrono::milliseconds(3000);
		}
		else if (session)
		{
			session->Stop();
		}

		session.reset();
		Close_TestSocket(sessionSocket);
		Close_TestSocket(peerSocket);
		tests.Require(
			socketReady && senderStarted && sendFailureWasIsolated &&
			sendFailureWasClassified &&
			shutdownWasBounded && 1u == closedCount.load(),
			"Isolate a sender socket failure to its session and notify close exactly once");
	}
	{
		CGameRoom room{ WORLD_ID::TRAINING_GROUND };
		auto moveCommand = [](const SESSION_ID sessionId,
			const std::uint32_t sequence)
			{
				ROOM_COMMAND command{};
				command.eType = ROOM_COMMAND_TYPE::MOVE;
				command.iSessionId = sessionId;
				command.Move.iClientSequence = sequence;
				command.Move.fGoalX = static_cast<float>(sequence);
				return command;
			};
		auto skillCommand = [](const SESSION_ID sessionId,
			const std::uint32_t sequence)
			{
				ROOM_COMMAND command{};
				command.eType = ROOM_COMMAND_TYPE::USE_SKILL;
				command.iSessionId = sessionId;
				command.UseSkill.iClientSequence = sequence;
				command.UseSkill.iSkillId = 34010u;
				return command;
			};
		auto aimCommand = [](const SESSION_ID sessionId,
			const std::uint32_t sequence,
			const SKILL_ID skillId)
			{
				ROOM_COMMAND command{};
				command.eType = ROOM_COMMAND_TYPE::UPDATE_SKILL_AIM;
				command.iSessionId = sessionId;
				command.UpdateSkillAim.iClientSequence = sequence;
				command.UpdateSkillAim.iSkillId = skillId;
				return command;
			};

		const bool enqueued = room.Is_Ready() &&
			room.Enqueue(moveCommand(1u, 1u)) &&
			room.Enqueue(skillCommand(2u, 10u)) &&
			room.Enqueue(moveCommand(1u, 2u)) &&
			room.Enqueue(skillCommand(1u, 20u)) &&
			room.Enqueue(moveCommand(1u, 3u)) &&
			room.Enqueue(aimCommand(1u, 30u, 34590u)) &&
			room.Enqueue(aimCommand(1u, 31u, 34590u)) &&
			room.Enqueue(aimCommand(1u, 32u, 34580u));
		const SERVER_ROOM_PERFORMANCE_METRICS metrics =
			room.Get_PerformanceMetrics();
		const bool preservedBarriers =
			6u == room.m_InboundCommands.size() &&
			ROOM_COMMAND_TYPE::USE_SKILL ==
				room.m_InboundCommands[0u].eType &&
			2u == room.m_InboundCommands[0u].iSessionId &&
			ROOM_COMMAND_TYPE::MOVE ==
				room.m_InboundCommands[1u].eType &&
			2u == room.m_InboundCommands[1u].Move.iClientSequence &&
			ROOM_COMMAND_TYPE::USE_SKILL ==
				room.m_InboundCommands[2u].eType &&
			ROOM_COMMAND_TYPE::MOVE ==
				room.m_InboundCommands[3u].eType &&
			3u == room.m_InboundCommands[3u].Move.iClientSequence &&
			ROOM_COMMAND_TYPE::UPDATE_SKILL_AIM ==
				room.m_InboundCommands[4u].eType &&
			31u == room.m_InboundCommands[4u].UpdateSkillAim.iClientSequence &&
			ROOM_COMMAND_TYPE::UPDATE_SKILL_AIM ==
				room.m_InboundCommands[5u].eType &&
			34580u == room.m_InboundCommands[5u].UpdateSkillAim.iSkillId;
		tests.Require(
			enqueued && preservedBarriers &&
			1u == metrics.iCoalescedMoveCommandCount &&
			1u == metrics.iCoalescedAimCommandCount,
			"Coalesce only same-stream movement and aim without crossing a same-session reliable barrier");
	}
	{
		CGameRoom room{ WORLD_ID::TRAINING_GROUND };
		bool admittedBestEffort = room.Is_Ready();
		for (std::size_t index = 0u;
			index < CGameRoom::MAX_BEST_EFFORT_COMMAND_COUNT; ++index)
		{
			ROOM_COMMAND command{};
			command.eType = ROOM_COMMAND_TYPE::MOVE;
			command.iSessionId = static_cast<SESSION_ID>(index + 1u);
			command.Move.iClientSequence = 1u;
			const bool accepted = room.Enqueue(std::move(command));
			admittedBestEffort = admittedBestEffort && accepted;
			if (!accepted)
				break;
		}
		ROOM_COMMAND droppedMove{};
		droppedMove.eType = ROOM_COMMAND_TYPE::MOVE;
		droppedMove.iSessionId = 5000u;
		droppedMove.Move.iClientSequence = 1u;
		const bool bestEffortDropWasNonFatal =
			room.Enqueue(std::move(droppedMove));

		bool admittedReliableReserve = true;
		while (room.m_InboundCommands.size() <
			CGameRoom::MAX_RELIABLE_COMMAND_COUNT)
		{
			ROOM_COMMAND command{};
			command.eType = ROOM_COMMAND_TYPE::USE_SKILL;
			command.iSessionId = static_cast<SESSION_ID>(
				room.m_InboundCommands.size() + 10000u);
			command.UseSkill.iClientSequence = 1u;
			command.UseSkill.iSkillId = 34010u;
			const bool accepted = room.Enqueue(std::move(command));
			admittedReliableReserve =
				admittedReliableReserve && accepted;
			if (!accepted)
				break;
		}
		ROOM_COMMAND rejectedReliable{};
		rejectedReliable.eType = ROOM_COMMAND_TYPE::USE_SKILL;
		rejectedReliable.iSessionId = 20000u;
		rejectedReliable.UseSkill.iClientSequence = 1u;
		rejectedReliable.UseSkill.iSkillId = 34010u;
		const bool reliableRejectedWithoutMutation =
			!room.Enqueue(std::move(rejectedReliable)) &&
			CGameRoom::MAX_RELIABLE_COMMAND_COUNT ==
				room.m_InboundCommands.size();

		SERVER_PLAYER cleanupPlayer{};
		cleanupPlayer.iSessionId = 40000u;
		cleanupPlayer.iPlayerId = 40000u;
		cleanupPlayer.iNetEntityId = 40000u;
		cleanupPlayer.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
		cleanupPlayer.iCurrentHp = 100u;
		cleanupPlayer.iMaximumHp = 100u;
		room.m_Players.emplace(cleanupPlayer.iPlayerId, cleanupPlayer);
		room.m_PlayerIdBySessionId.emplace(
			cleanupPlayer.iSessionId, cleanupPlayer.iPlayerId);
		room.m_PlayerIdByEntityId.emplace(
			cleanupPlayer.iNetEntityId, cleanupPlayer.iPlayerId);

		ROOM_COMMAND cleanup{};
		cleanup.eType = ROOM_COMMAND_TYPE::LEAVE;
		cleanup.iSessionId = cleanupPlayer.iSessionId;
		cleanup.eLeaveReason = PLAYER_DESPAWN_REASON::DISCONNECTED;
		const bool cleanupAcceptedAtReliableHardCap =
			room.Enqueue(std::move(cleanup));
		ROOM_COMMAND duplicateCleanup{};
		duplicateCleanup.eType = ROOM_COMMAND_TYPE::LEAVE;
		duplicateCleanup.iSessionId = cleanupPlayer.iSessionId;
		duplicateCleanup.eLeaveReason = PLAYER_DESPAWN_REASON::LEVEL_CHANGED;
		const bool duplicateCleanupDeduplicated =
			room.Enqueue(std::move(duplicateCleanup));
		const bool cleanupWasSeparated =
			CGameRoom::MAX_RELIABLE_COMMAND_COUNT ==
				room.m_InboundCommands.size() &&
			1u == room.m_CleanupCommands.size() &&
			1u == room.m_QueuedCleanupSessionIds.size();
		room.Tick(1.f / 30.f);
		const SERVER_ROOM_PERFORMANCE_METRICS metrics =
			room.Get_PerformanceMetrics();
		const bool cleanupDrainedBeforeBoundedIngress =
			!room.m_Players.contains(cleanupPlayer.iPlayerId) &&
			!room.m_PlayerIdBySessionId.contains(cleanupPlayer.iSessionId) &&
			!room.m_PlayerIdByEntityId.contains(cleanupPlayer.iNetEntityId) &&
			room.m_CleanupCommands.empty() &&
			room.m_QueuedCleanupSessionIds.empty() &&
			CGameRoom::MAX_RELIABLE_COMMAND_COUNT -
				CGameRoom::MAX_COMMANDS_DRAINED_PER_TICK ==
				room.m_InboundCommands.size();
		tests.Require(
			admittedBestEffort && bestEffortDropWasNonFatal &&
			admittedReliableReserve && reliableRejectedWithoutMutation &&
			cleanupAcceptedAtReliableHardCap &&
			duplicateCleanupDeduplicated && cleanupWasSeparated &&
			cleanupDrainedBeforeBoundedIngress &&
			1u == metrics.iDroppedBestEffortCommandCount &&
			1u == metrics.iRejectedReliableCommandCount &&
			0u == metrics.iRejectedCleanupCommandCount &&
			1u == metrics.iDeduplicatedCleanupCommandCount &&
			1u == metrics.iCleanupIngressHighWatermark &&
			1u == metrics.iLastCleanupIngressDepth &&
			1u == metrics.iLastDrainedCleanupCommandCount &&
			0u == metrics.iLastRemainingCleanupCommandCount &&
			CGameRoom::MAX_RELIABLE_COMMAND_COUNT ==
				metrics.iIngressHighWatermark,
			"Keep reliable ingress bounded while accepting, deduplicating, and priority-draining cleanup at hard cap");
	}
	{
		CServerApp app;
		auto sourceSimulation = std::make_shared<CGameRoom>(WORLD_ID::BERN);
		auto targetSimulation =
			std::make_shared<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		constexpr SESSION_ID transferSessionId = 41000u;
		auto session = std::make_shared<CClientSession>(
			transferSessionId, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		app.m_Sessions.emplace(transferSessionId, session);
		app.m_SharedGameRooms.emplace(WORLD_ID::BERN, sourceSimulation);
		app.m_SharedGameRooms.emplace(WORLD_ID::VALTAN_ARENA, targetSimulation);
		CServerApp::SESSION_GAMEPLAY_BINDING binding{};
		binding.eWorldId = WORLD_ID::BERN;
		binding.pSimulation = sourceSimulation;
		app.m_GameplayBindingBySessionId.emplace(transferSessionId, binding);

		bool filledTargetIngress = targetSimulation->Is_Ready();
		for (std::size_t index = 0u;
			index + 1u < CGameRoom::MAX_RELIABLE_COMMAND_COUNT; ++index)
		{
			ROOM_COMMAND command{};
			command.eType = ROOM_COMMAND_TYPE::USE_SKILL;
			command.iSessionId = static_cast<SESSION_ID>(50000u + index);
			command.UseSkill.iClientSequence = 1u;
			command.UseSkill.iSkillId = 34010u;
			filledTargetIngress = filledTargetIngress &&
				targetSimulation->Enqueue(std::move(command));
		}
		SERVER_WORLD_TRANSFER_REQUEST transfer{};
		transfer.iSessionId = transferSessionId;
		transfer.eTargetWorldId = WORLD_ID::VALTAN_ARENA;
		transfer.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
		transfer.strNickName = "TransferRollbackFixture";
		sourceSimulation->m_PendingWorldTransfers.push_back(transfer);
		app.Handle_WorldTransfers(sourceSimulation);

		const CLIENT_SESSION_CLOSE_DIAGNOSTIC diagnostic =
			session->Get_CloseDiagnostic();
		const bool rollbackGuaranteed =
			SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_INGRESS_OVERFLOW ==
				diagnostic.eReason &&
			WSAENOBUFS == diagnostic.iNativeErrorCode &&
			std::string::npos != diagnostic.strContext.find(
				"stage=target-enter-ingress") &&
			std::string::npos != diagnostic.strContext.find(
				"rollbackCleanupRequired=true") &&
			std::string::npos != diagnostic.strContext.find(
				"rollbackCleanupEnqueued=true") &&
			1u == targetSimulation->m_CleanupCommands.size() &&
			transferSessionId ==
				targetSimulation->m_CleanupCommands.front().iSessionId &&
			PLAYER_DESPAWN_REASON::LEVEL_CHANGED ==
				targetSimulation->m_CleanupCommands.front().eLeaveReason &&
			std::none_of(
				targetSimulation->m_InboundCommands.begin(),
				targetSimulation->m_InboundCommands.end(),
				[](const ROOM_COMMAND& command)
				{
					return transferSessionId == command.iSessionId;
				});
		ROOM_COMMAND commandAfterCleanup{};
		commandAfterCleanup.eType = ROOM_COMMAND_TYPE::USE_SKILL;
		commandAfterCleanup.iSessionId = transferSessionId;
		commandAfterCleanup.UseSkill.iClientSequence = 2u;
		commandAfterCleanup.UseSkill.iSkillId = 34010u;
		const bool commandRejectedBehindCleanup =
			!targetSimulation->Enqueue(std::move(commandAfterCleanup));
		targetSimulation->Tick(1.f / 30.f);
		tests.Require(
			filledTargetIngress && rollbackGuaranteed &&
			commandRejectedBehindCleanup &&
			targetSimulation->m_CleanupCommands.empty() &&
			targetSimulation->m_QueuedCleanupSessionIds.empty() &&
			app.m_GameplayBindingBySessionId.contains(transferSessionId) &&
			app.m_GameplayBindingBySessionId.at(transferSessionId).pSimulation ==
				sourceSimulation,
			"Classify transfer ingress failure and guarantee target rollback cleanup");
	}
	{
		CGameRoom room{ WORLD_ID::TRAINING_GROUND };
		const std::size_t commandCount =
			CGameRoom::MAX_COMMANDS_DRAINED_PER_TICK + 5u;
		bool enqueued = room.Is_Ready();
		for (std::size_t index = 0u; index < commandCount; ++index)
		{
			ROOM_COMMAND command{};
			command.eType = ROOM_COMMAND_TYPE::USE_SKILL;
			command.iSessionId = static_cast<SESSION_ID>(50000u + index);
			command.UseSkill.iClientSequence = 1u;
			command.UseSkill.iSkillId = 34010u;
			const bool accepted = room.Enqueue(std::move(command));
			enqueued = enqueued && accepted;
			if (!accepted)
				break;
		}
		room.Tick(1.f / 30.f);
		const SERVER_ROOM_PERFORMANCE_METRICS firstTickMetrics =
			room.Get_PerformanceMetrics();
		const bool retainedFifoTail =
			5u == room.m_InboundCommands.size() &&
			static_cast<SESSION_ID>(
				50000u + CGameRoom::MAX_COMMANDS_DRAINED_PER_TICK) ==
				room.m_InboundCommands.front().iSessionId;
		room.Tick(1.f / 30.f);
		const SERVER_ROOM_PERFORMANCE_METRICS secondTickMetrics =
			room.Get_PerformanceMetrics();

		SERVER_PLAYER player{};
		player.iSessionId = 60000u;
		player.iPlayerId = 60000u;
		player.iNetEntityId = 60000u;
		player.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
		room.m_Players.emplace(player.iPlayerId, player);
		room.Broadcast_WorldSnapshot();
		const SERVER_ROOM_PERFORMANCE_METRICS snapshotMetrics =
			room.Get_PerformanceMetrics();
		tests.Require(
			enqueued && retainedFifoTail &&
			commandCount == firstTickMetrics.iLastIngressDepth &&
			CGameRoom::MAX_COMMANDS_DRAINED_PER_TICK ==
				firstTickMetrics.iLastDrainedCommandCount &&
			5u == firstTickMetrics.iLastRemainingCommandCount &&
			1u == firstTickMetrics.iDrainLimitedTickCount &&
			5u == secondTickMetrics.iLastIngressDepth &&
			5u == secondTickMetrics.iLastDrainedCommandCount &&
			0u == secondTickMetrics.iLastRemainingCommandCount &&
			2u == secondTickMetrics.iTickCount &&
			1u == snapshotMetrics.iSnapshotEncodeCount &&
			0u == snapshotMetrics.iSnapshotEncodeFailureCount &&
			1u == snapshotMetrics.iSnapshotEnqueueBatchCount &&
			0u == snapshotMetrics.iSnapshotRecipientCount,
			"Drain a deterministic FIFO prefix per tick and record tick, ingress, encode, and enqueue metrics");
	}
	{
		using namespace LostArk::Shared::CombatCollision;

		const CIRCLE_XZ circle{ 0.f, 0.f, 2.f };
		const BODY_CIRCLE_XZ tangentCircle{ 3.f, 0.f, 1.f };
		const BODY_CIRCLE_XZ missedCircle{ 3.001f, 0.f, 1.f };
		tests.Require(
			Circles_Overlap(circle, tangentCircle) &&
			!Circles_Overlap(circle, missedCircle),
			"Treat circle tangency as contact and reject a separated circle");

		const BODY_CIRCLE_XZ innerRingTangent{ 2.f, 0.f, 1.f };
		const BODY_CIRCLE_XZ insideRingHole{ 1.9f, 0.f, 1.f };
		const BODY_CIRCLE_XZ outerRingTangent{ 6.f, 0.f, 1.f };
		const BODY_CIRCLE_XZ outsideRing{ 6.001f, 0.f, 1.f };
		tests.Require(
			Circle_IntersectsRing(innerRingTangent, 0.f, 0.f, 3.f, 5.f) &&
			!Circle_IntersectsRing(insideRingHole, 0.f, 0.f, 3.f, 5.f) &&
			Circle_IntersectsRing(outerRingTangent, 0.f, 0.f, 3.f, 5.f) &&
			!Circle_IntersectsRing(outsideRing, 0.f, 0.f, 3.f, 5.f),
			"Respect both inclusive ring boundaries and reject both misses");

		const BODY_CIRCLE_XZ rotatedShapeHit{ 2.f, 2.f, 0.25f };
		const BODY_CIRCLE_XZ rotatedShapeMiss{ 0.f, 2.5f, 0.25f };
		tests.Require(
			Circle_IntersectsForwardBox(
				rotatedShapeHit, 0.f, 0.f, 1.f, 1.f, 4.f, 0.5f) &&
			!Circle_IntersectsForwardBox(
				rotatedShapeMiss, 0.f, 0.f, 1.f, 1.f, 4.f, 0.5f),
			"Evaluate a forward box in its rotated basis");
		tests.Require(
			Circle_IntersectsCone(
				rotatedShapeHit, 0.f, 0.f, 1.f, 1.f, 5.f, 60.f) &&
			!Circle_IntersectsCone(
				rotatedShapeMiss, 0.f, 0.f, 1.f, 1.f, 5.f, 60.f),
			"Evaluate a cone in its rotated basis");
		tests.Require(
			Circle_IntersectsCross(
				rotatedShapeHit, 0.f, 0.f, 1.f, 1.f, 4.f, 0.5f) &&
			!Circle_IntersectsCross(
				rotatedShapeMiss, 0.f, 0.f, 1.f, 1.f, 4.f, 0.5f),
			"Evaluate a cross in its rotated basis");

		constexpr float ROOT_THREE_OVER_TWO = 0.8660254f;
		const std::array<BODY_CIRCLE_XZ, 6u> sixDirectionArms = {{
			{ 0.f, 5.f, 0.2f },
			{ 0.f, -5.f, 0.2f },
			{ 5.f * ROOT_THREE_OVER_TWO, 2.5f, 0.2f },
			{ -5.f * ROOT_THREE_OVER_TWO, -2.5f, 0.2f },
			{ -5.f * ROOT_THREE_OVER_TWO, 2.5f, 0.2f },
			{ 5.f * ROOT_THREE_OVER_TWO, -2.5f, 0.2f }
		}};
		const std::array<BODY_CIRCLE_XZ, 6u> sixDirectionGaps = {{
			{ 2.5f, 5.f * ROOT_THREE_OVER_TWO, 0.2f },
			{ -2.5f, -5.f * ROOT_THREE_OVER_TWO, 0.2f },
			{ 5.f, 0.f, 0.2f },
			{ -5.f, 0.f, 0.2f },
			{ 2.5f, -5.f * ROOT_THREE_OVER_TWO, 0.2f },
			{ -2.5f, 5.f * ROOT_THREE_OVER_TWO, 0.2f }
		}};
		const auto hitsSixDirections = [](const BODY_CIRCLE_XZ& body)
		{
			return Circle_IntersectsSixDirections(
				body, 0.f, 0.f, 0.f, 1.f, 6.f, 0.5f);
		};
		tests.Require(
			std::all_of(
				sixDirectionArms.begin(), sixDirectionArms.end(),
				hitsSixDirections) &&
			std::none_of(
				sixDirectionGaps.begin(), sixDirectionGaps.end(),
				hitsSixDirections),
			"Hit all six centered-strip arms and preserve all six angular gaps");
	}
	{
		CServerNavigation navigation;
		const bool navigationLoaded =
			navigation.Load("LV_LOBBY_CLASSSELECT_SL00");
		SERVER_WORLD_ENTITY monster{};
		monster.iNetEntityId = 700u;
		monster.eKind = WORLD_BOOTSTRAP_KIND::MONSTER;
		monster.iCurrentHp = 100u;
		monster.iMaximumHp = 100u;
		monster.fCollisionRadius = 0.6f;
		monster.fAttackRange = 1.f;
		monster.fEngageDistance = 8.f;
		monster.fMoveSpeed = 2.f;
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER protectedPlayer{};
		protectedPlayer.iPlayerId = 701u;
		protectedPlayer.iNetEntityId = 702u;
		protectedPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		protectedPlayer.iCurrentHp = 100u;
		protectedPlayer.iMaximumHp = 100u;
		protectedPlayer.fPositionX = 1.f;
		protectedPlayer.isCombatReady = false;
		players.emplace(protectedPlayer.iPlayerId, protectedPlayer);
		std::vector<DAMAGE_EVENT> damageEvents;
		/* This case owns one loose monster rather than a room's entity list, so
		it has no neighbour to separate from. */
		CMonsterBrain monsterBrain;
		monsterBrain.Update(
			monster, players, catalog, navigation,
			1.f / 30.f, 1u, damageEvents);
		const bool ignoredProtectedPlayer =
			INVALID_NET_ENTITY_ID == monster.iTargetEntityId &&
			SERVER_ENTITY_ACTION::IDLE == monster.eAction;
		players.begin()->second.isCombatReady = true;
		monsterBrain.Update(
			monster, players, catalog, navigation,
			1.f / 30.f, 2u, damageEvents);
		tests.Require(
			navigationLoaded && ignoredProtectedPlayer &&
			SERVER_ENTITY_ACTION::PATTERN_WINDUP == monster.eAction &&
			players.begin()->second.iNetEntityId == monster.iTargetEntityId,
			"Ignore protected players and acquire the same player after combat admission");
	}
	{
		CServerNavigation navigation;
		const bool navigationLoaded =
			navigation.Load("LV_LOBBY_CLASSSELECT_SL00");
		SERVER_WORLD_ENTITY monster{};
		monster.iNetEntityId = 750u;
		monster.eKind = WORLD_BOOTSTRAP_KIND::MONSTER;
		monster.eAction = SERVER_ENTITY_ACTION::CHASE;
		monster.iCurrentHp = 100u;
		monster.iMaximumHp = 100u;
		monster.iTargetEntityId = 752u;
		monster.iNextPathReplanTick = 100u;
		monster.fYawDegrees = 1e20f;
		monster.fCollisionRadius = 0.1f;
		monster.fAttackRange = 0.01f;
		monster.fEngageDistance = 10.f;
		monster.fTargetReleaseDistance = 14.f;
		monster.fMoveSpeed = 2.f;
		monster.fTurnSpeedDegreesPerSecond = 180.f;
		monster.fMoveAcceleration = 4.f;
		monster.fMoveDeceleration = 6.f;
		monster.fArrivalSlowRadius = 1.f;
		monster.MovePath.push_back({ 2.f, 0.f, 0.f });

		SERVER_PLAYER player{};
		player.iPlayerId = 751u;
		player.iNetEntityId = 752u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = 100u;
		player.iMaximumHp = 100u;
		player.fPositionX = 5.f;
		player.isCombatReady = true;
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		players.emplace(player.iPlayerId, player);
		std::vector<DAMAGE_EVENT> damageEvents;
		CMonsterBrain monsterBrain;
		monsterBrain.Update(
			monster, players, catalog, navigation,
			1.f / 30.f, 10u, damageEvents);
		tests.Require(
			navigationLoaded && std::isfinite(monster.fYawDegrees) &&
			std::fabs(monster.fYawDegrees) <= 180.f,
			"Normalize an extreme monster yaw in bounded time");
	}
	{
		CServerNavigation navigation;
		const bool navigationLoaded =
			navigation.Load("LV_LOBBY_CLASSSELECT_SL00");
		SERVER_WORLD_ENTITY monster{};
		monster.iNetEntityId = 800u;
		monster.eKind = WORLD_BOOTSTRAP_KIND::MONSTER;
		monster.iCurrentHp = 100u;
		monster.iMaximumHp = 100u;
		monster.fCollisionRadius = 0.1f;
		monster.fAttackRange = 0.01f;
		monster.fEngageDistance = 10.f;
		monster.fTargetReleaseDistance = 14.f;
		monster.fMoveSpeed = 2.f;
		monster.fTurnSpeedDegreesPerSecond = 180.f;
		monster.fMoveAcceleration = 4.f;
		monster.fMoveDeceleration = 6.f;
		monster.fArrivalSlowRadius = 1.f;
		monster.iPatternTelegraphMs = 1000u;
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		for (std::uint32_t index = 0u; index < 2u; ++index)
		{
			SERVER_PLAYER player{};
			player.iPlayerId = 810u + index;
			player.iNetEntityId = 820u + index;
			player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			player.iCurrentHp = 100u;
			player.iMaximumHp = 100u;
			player.fPositionX = 5.f + static_cast<float>(index);
			player.isCombatReady = true;
			players.emplace(player.iPlayerId, player);
		}
		std::vector<DAMAGE_EVENT> damageEvents;
		CMonsterBrain monsterBrain;
		monsterBrain.Update(
			monster, players, catalog, navigation,
			1.f / 30.f, 10u, damageEvents);
		const NET_ENTITY_ID firstTarget = monster.iTargetEntityId;
		players.find(811u)->second.fPositionX = 4.f;
		monsterBrain.Update(
			monster, players, catalog, navigation,
			1.f / 30.f, 11u, damageEvents);
		const bool retainedTarget = firstTarget == monster.iTargetEntityId;
		players.find(810u)->second.fPositionX = 20.f;
		monsterBrain.Update(
			monster, players, catalog, navigation,
			1.f / 30.f, 12u, damageEvents);
		const bool releasedAndRetargeted =
			821u == monster.iTargetEntityId;
		monster.eAction = SERVER_ENTITY_ACTION::PATTERN_WINDUP;
		monster.fActionElapsedSeconds = 0.f;
		players.find(810u)->second.fPositionX = 0.5f;
		monsterBrain.Update(
			monster, players, catalog, navigation,
			1.f / 30.f, 13u, damageEvents);
		tests.Require(
			navigationLoaded && 820u == firstTarget && retainedTarget &&
			releasedAndRetargeted && 821u == monster.iTargetEntityId,
			"Retain a valid monster target, release it at hysteresis range, and lock the replacement through windup");
	}
	{
		CGameRoom room{ WORLD_ID::CHARACTER_SELECT_ARENA };
		tests.Require(room.Is_Ready(),
			"Initialize Character Select room for class changes");
		const WORLD_BOOTSTRAP_PLACEMENT* spawn =
			room.Find_AvailablePlayerSpawn();
		tests.Require(nullptr != spawn,
			"Resolve Character Select class-change respawn placement");
		if (room.Is_Ready() && nullptr != spawn)
		{
			SERVER_PLAYER player{};
			player.iSessionId = 11u;
			player.iPlayerId = 12u;
			player.iNetEntityId = 112u;
			player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			player.strNickName = "ClassSwitch";
			player.strSpawnPlacementId = spawn->strPlacementId;
			player.fPositionX = 17.f;
			player.fPositionY = 3.f;
			player.fPositionZ = -9.f;
			player.fYawDegrees = 33.f;
			player.iCurrentHp = 10u;
			player.iMaximumHp = 100u;
			player.iCurrentResource = 2u;
			player.iMaximumResource = 10u;
			player.eAction = PLAYER_ACTION_STATE::SKILL;
			player.iCurrentSkillId = 34120u;
			player.iActionStartTick = 9u;
			player.iLastMoveSequence = 7u;
			player.iLastSkillSequence = 8u;
			player.hasMoveGoal = true;
			player.CooldownEndTickBySkillId.emplace(34120u, 100u);
			C2S_MOVE pendingBeforeClassChange{};
			pendingBeforeClassChange.iClientSequence = 8u;
			pendingBeforeClassChange.fGoalX = 20.f;
			pendingBeforeClassChange.fGoalZ = -10.f;
			player.PendingCommand.Set_Move(pendingBeforeClassChange);

			C2S_CHANGE_CHARACTER_CLASS request{};
			request.iClientSequence = 1u;
			request.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
			tests.Require(
				CHARACTER_CLASS_CHANGE_RESULT::ACCEPTED ==
					room.Apply_CharacterClassChange(player, request) &&
				CHARACTER_CLASS_ID::ARTIST == player.eCharacterClass &&
				17.f == player.fPositionX && -9.f == player.fPositionZ &&
				12u == player.iPlayerId && 112u == player.iNetEntityId &&
				7u == player.iLastMoveSequence &&
				8u == player.iLastSkillSequence &&
				PLAYER_ACTION_STATE::NONE == player.eAction &&
				INVALID_SKILL_ID == player.iCurrentSkillId &&
				PLAYER_PENDING_COMMAND_KIND::NONE == player.PendingCommand.eKind &&
				!player.hasMoveGoal && player.CooldownEndTickBySkillId.empty() &&
				player.iCurrentHp == player.iMaximumHp &&
				player.iCurrentResource == player.iMaximumResource,
				"Change class during action, preserve identity/position/sequences, and reset state");

			C2S_USE_SKILL oldClassSkill{};
			oldClassSkill.iClientSequence = 9u;
			oldClassSkill.iSkillId = 34120u;
			oldClassSkill.fAimX = 1.f;
			oldClassSkill.fAimZ = 0.f;
			C2S_USE_SKILL newClassSkill = oldClassSkill;
			newClassSkill.iSkillId = 31200u;
			tests.Require(
				!room.m_PlayerSkillSystem.Try_Start(
					player, oldClassSkill, room.m_GameplayCatalog, 10u) &&
				room.m_PlayerSkillSystem.Try_Start(
					player, newClassSkill, room.m_GameplayCatalog, 10u) &&
				31200u == player.iCurrentSkillId &&
				9u == player.iLastSkillSequence,
				"Reject old-class skill and approve new-class skill after class change");

			const SERVER_PLAYER accepted = player;
			tests.Require(
				CHARACTER_CLASS_CHANGE_RESULT::REJECTED_STALE_SEQUENCE ==
					room.Apply_CharacterClassChange(player, request) &&
				accepted.eCharacterClass == player.eCharacterClass &&
				accepted.iCurrentHp == player.iCurrentHp,
				"Reject stale class change without mutating player");

			player.iCurrentHp = 0u;
			player.eAction = PLAYER_ACTION_STATE::DEAD;
			player.fPositionX = 999.f;
			player.fPositionY = 999.f;
			player.fPositionZ = 999.f;
			request.iClientSequence = 2u;
			request.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
			SERVER_NAV_POINT projected{};
			const bool projectedSpawn = room.m_ServerNavigation.Project_Point(
				spawn->fPositionX, spawn->fPositionZ, projected);
			tests.Require(projectedSpawn &&
				CHARACTER_CLASS_CHANGE_RESULT::ACCEPTED ==
					room.Apply_CharacterClassChange(player, request) &&
				CHARACTER_CLASS_ID::WARLORD == player.eCharacterClass &&
				projected.x == player.fPositionX &&
				projected.y == player.fPositionY &&
				projected.z == player.fPositionZ &&
				PLAYER_ACTION_STATE::NONE == player.eAction &&
				0u != player.iCurrentHp,
				"Change dead player class and respawn at projected original spawn");

			CGameRoom bernRoom{ WORLD_ID::BERN };
			const SERVER_PLAYER beforeWrongWorld = player;
			request.iClientSequence = 3u;
			request.eCharacterClass = CHARACTER_CLASS_ID::SLAYER;
			tests.Require(bernRoom.Is_Ready() &&
				CHARACTER_CLASS_CHANGE_RESULT::REJECTED_WRONG_WORLD ==
					bernRoom.Apply_CharacterClassChange(player, request) &&
				beforeWrongWorld.eCharacterClass == player.eCharacterClass &&
				beforeWrongWorld.iCurrentHp == player.iCurrentHp,
				"Reject class change outside Character Select without mutation");
		}
	}
	{
		auto bernEntryRoomStorage = std::make_unique<CGameRoom>(WORLD_ID::BERN);
		CGameRoom& bernEntryRoom = *bernEntryRoomStorage;
		const auto& bernEntryPlacements =
			bernEntryRoom.m_WorldBootstrap.Get_Placements();
		const std::size_t expectedNpcCount = static_cast<std::size_t>(
			std::count_if(
				bernEntryPlacements.begin(), bernEntryPlacements.end(),
				[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
				{
					return placement.isEnabled &&
						WORLD_BOOTSTRAP_KIND::NPC == placement.eKind;
				}));
		std::size_t serializedNpcCount = 0u;
		bool allWorldSpawnsSerializable = bernEntryRoom.Is_Ready();
		SERVER_WORLD_ENTITY invalidNpc{};
		bool foundNpc = false;
		for (const SERVER_WORLD_ENTITY& entity : bernEntryRoom.m_WorldEntities)
		{
			std::vector<std::uint8_t> payload;
			S2C_WORLD_ENTITY_SPAWNED decoded{};
			if (!CGameRoom::Build_WorldEntitySpawnedPayload(entity, payload))
			{
				allWorldSpawnsSerializable = false;
				continue;
			}
			CPacketReader reader{ payload };
			if (!Read_Message(reader, decoded) ||
				0u != reader.Get_RemainingSize() ||
				decoded.iNetEntityId != entity.iNetEntityId ||
				decoded.strPlacementId != entity.strPlacementId)
			{
				allWorldSpawnsSerializable = false;
				continue;
			}
			if (WORLD_BOOTSTRAP_KIND::NPC == entity.eKind)
			{
				++serializedNpcCount;
				allWorldSpawnsSerializable = allWorldSpawnsSerializable &&
					WORLD_ENTITY_KIND::NPC == decoded.eKind &&
					0.f == entity.fCollisionRadius &&
					0.f == decoded.fCollisionRadius;
				if (!foundNpc)
				{
					invalidNpc = entity;
					foundNpc = true;
				}
			}
		}
		std::vector<std::uint8_t> unchangedPayload{ 0x7fu };
		if (foundNpc)
		{
			invalidNpc.fCollisionRadius =
				LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_X;
			allWorldSpawnsSerializable = allWorldSpawnsSerializable &&
				!CGameRoom::Build_WorldEntitySpawnedPayload(
					invalidNpc, unchangedPayload) &&
				1u == unchangedPayload.size() && 0x7fu == unchangedPayload.front();
		}
		tests.Require(
			allWorldSpawnsSerializable && foundNpc &&
			0u != expectedNpcCount && expectedNpcCount == serializedNpcCount,
			"Preflight every Bern world spawn and keep town NPC wire radius zero");

		constexpr SESSION_ID BERN_ENTRY_SESSION = 73001u;
		auto bernEntrySession = std::make_shared<CClientSession>(
			BERN_ENTRY_SESSION, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		bernEntrySession->m_isSendRunning.store(true);
		bernEntryRoom.m_Sessions.insert_or_assign(
			BERN_ENTRY_SESSION, bernEntrySession);
		C2S_ENTER_WORLD bernEnter{};
		bernEnter.eWorldId = WORLD_ID::BERN;
		bernEnter.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
		bernEnter.strNickName = "BernEntryContract";
		const bool joinedBern = bernEntryRoom.Join(
			BERN_ENTRY_SESSION, bernEnter);
		std::size_t worldSpawnFrameCount = 0u;
		bool sawLocalPlayerSpawn = false;
		for (const CClientSession::OUTBOUND_FRAME& frame :
			bernEntrySession->m_OutboundFrames)
		{
			if (PACKET_TYPE::S2C_WORLD_ENTITY_SPAWNED == frame.ePacketType)
				++worldSpawnFrameCount;
			else if (PACKET_TYPE::S2C_PLAYER_SPAWNED == frame.ePacketType)
				sawLocalPlayerSpawn = true;
		}
		const bool completeBernEntryStream = joinedBern && sawLocalPlayerSpawn &&
			worldSpawnFrameCount == bernEntryRoom.m_WorldEntities.size();
		bernEntrySession->Request_Close();
		tests.Require(completeBernEntryStream,
			"Join Bern with every world spawn and the local player spawn queued");
	}
	{
		struct PARTY_FIXTURE final
		{
			std::unique_ptr<CServerApp> App = std::make_unique<CServerApp>();
			std::shared_ptr<CGameRoom> Source = std::make_shared<CGameRoom>(WORLD_ID::BERN);
			std::shared_ptr<CGameRoom> Target = std::make_shared<CGameRoom>(WORLD_ID::VALTAN_ARENA);
			std::vector<std::shared_ptr<CClientSession>> Sessions;
			SERVER_WORLD_TRANSFER_REQUEST Request;
			bool Ready = false;
		};
		const auto clearOutbound = [](CClientSession& session)
		{
			session.m_OutboundFrames.clear();
			session.m_iQueuedOutboundBytes = 0u;
			session.m_OutboundMetrics.iCurrentQueuedByteCount = 0u;
			session.m_OutboundMetrics.iCurrentQueuedFrameCount = 0u;
		};
		{
			auto first = std::make_shared<CClientSession>(95991u, INVALID_SOCKET,
				CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
			auto second = std::make_shared<CClientSession>(95992u, INVALID_SOCKET,
				CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
			first->m_isSendRunning.store(true);
			second->m_isSendRunning.store(true);
			// Reproduce the close interval after terminal diagnostic publication,
			// before Request_Close has changed running flags or cleared its queue.
			second->Record_TerminalDiagnostic(SESSION_DIAGNOSTIC_REASON::SERVER_PEER_CLOSED,
				0, "contract terminal admission race");
			S2C_PARTY_TRANSFER_RESULT notice{ 81u, WORLD_ID::VALTAN_ARENA,
				PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE };
			CPacketWriter writer;
			const bool encoded = Write_Message(writer, notice);
			const PACKET_FRAME frame{ PACKET_TYPE::S2C_PARTY_TRANSFER_RESULT, writer.Get_Buffer() };
			CClientSession::RELIABLE_BATCH_TRANSACTION admission;
			std::string status;
			tests.Require(encoded && !admission.Prepare({ { first, { frame } }, { second, { frame } } }, status) &&
				second->m_isSendRunning.load() && first->m_OutboundFrames.empty() &&
				second->m_OutboundFrames.empty() && !status.empty(),
				"Reject terminal-in-progress admission and release every staged participant unchanged");
		}
		const auto makeParty = [&clearOutbound](const std::size_t count, const bool formParty)
		{
			auto fixture = std::make_unique<PARTY_FIXTURE>();
			fixture->Ready = fixture->Source->Is_Ready() && fixture->Target->Is_Ready();
			fixture->App->m_SharedGameRooms.emplace(WORLD_ID::BERN, fixture->Source);
			fixture->App->m_SharedGameRooms.emplace(WORLD_ID::VALTAN_ARENA, fixture->Target);
			const CHARACTER_CLASS_ID classes[] = { CHARACTER_CLASS_ID::LANCE_MASTER,
				CHARACTER_CLASS_ID::ARTIST, CHARACTER_CLASS_ID::WARLORD,
				CHARACTER_CLASS_ID::DIMENSIONMASTER };
			for (std::size_t index = 0; index < count && fixture->Ready; ++index)
			{
				const SESSION_ID id = 96001u + index;
				auto session = std::make_shared<CClientSession>(id, INVALID_SOCKET,
					CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
				session->m_isSendRunning.store(true);
				fixture->Source->Handle_Register(session);
				C2S_ENTER_WORLD enter{};
				enter.eWorldId = WORLD_ID::BERN;
				enter.eCharacterClass = classes[index];
				enter.strNickName = "PartyContract" + std::to_string(index);
				fixture->Ready = fixture->Source->Join(id, enter);
				fixture->Sessions.push_back(session);
				fixture->App->m_Sessions.emplace(id, session);
				CServerApp::SESSION_GAMEPLAY_BINDING binding{};
				binding.eWorldId = WORLD_ID::BERN;
				binding.pSimulation = fixture->Source;
				fixture->App->m_GameplayBindingBySessionId.emplace(id, binding);
				for (const auto& active : fixture->Sessions) clearOutbound(*active);
			}
			if (!fixture->Ready) return fixture;
			const auto& leader = fixture->Source->m_Players.at(fixture->Sessions.front()->Get_PlayerId());
			fixture->Request.iSessionId = leader.iSessionId;
			fixture->Request.eTargetWorldId = WORLD_ID::VALTAN_ARENA;
			fixture->Request.eCharacterClass = leader.eCharacterClass;
			fixture->Request.strNickName = leader.strNickName;
			fixture->Request.iPartyRequestSequence = 81u;
			for (const auto& session : fixture->Sessions)
				fixture->Request.PartyBatchSessionIds.push_back(session->Get_SessionId());
			if (formParty)
			{
				for (std::size_t index = 1; index < count; ++index)
				{
					C2S_PARTY_INVITE invite{};
					invite.iRequestSequence = static_cast<std::uint32_t>(index);
					invite.iTargetNetEntityId = fixture->Source->m_Players.at(
						fixture->Sessions[index]->Get_PlayerId()).iNetEntityId;
					fixture->Source->Handle_PartyInvite(leader.iSessionId, invite);
					C2S_PARTY_INVITE_RESPOND respond{};
					respond.iRequestSequence = static_cast<std::uint32_t>(index);
					respond.iFromNetEntityId = leader.iNetEntityId;
					respond.bAccepted = true;
					fixture->Source->Handle_PartyInviteRespond(
						fixture->Sessions[index]->Get_SessionId(), respond);
				}
				fixture->Ready = 1u == fixture->Source->m_PartyMembersByPartyId.size() &&
					count == fixture->Source->m_PartyMembersByPartyId.begin()->second.size();
			}
			for (const auto& active : fixture->Sessions) clearOutbound(*active);
			return fixture;
		};
		for (const std::size_t count : { 2u, 4u })
		{
			auto fixture = makeParty(count, true);
			tests.Require(fixture->Ready, "Create a real two/four-player invited party fixture");
			if (!fixture->Ready) continue;
			const auto guide = std::find_if(fixture->Source->m_WorldEntities.begin(),
				fixture->Source->m_WorldEntities.end(), [](const SERVER_WORLD_ENTITY& entity)
				{ return "npc.bern.beda.guide" == entity.strPlacementId; });
			bool leaderBatch = guide != fixture->Source->m_WorldEntities.end();
			if (leaderBatch)
			{
				for (const auto& session : fixture->Sessions)
				{
					auto& player = fixture->Source->m_Players.at(session->Get_PlayerId());
					player.fPositionX = guide->fPositionX;
					player.fPositionZ = guide->fPositionZ;
				}
				C2S_CONFIRM_NPC_ENTRY confirm{};
				confirm.iRequestSequence = 81u;
				confirm.strNpcPlacementId = guide->strPlacementId;
				fixture->Source->Handle_ConfirmNpcEntry(
					fixture->Sessions[1]->Get_SessionId(), confirm);
				bool hasFailureNotice = false;
				for (const auto& frame : fixture->Sessions[1]->m_OutboundFrames)
				{
					if (PACKET_TYPE::S2C_PARTY_TRANSFER_RESULT != frame.ePacketType) continue;
					CPacketReader reader{ std::span<const std::uint8_t>{ frame.Bytes }.subspan(PACKET_HEADER_BYTES) };
					S2C_PARTY_TRANSFER_RESULT result{};
					hasFailureNotice = Read_Message(reader, result) &&
						PARTY_TRANSFER_RESULT::REJECTED_NOT_LEADER == result.eResult;
				}
				tests.Require(fixture->Source->m_PendingWorldTransfers.empty() && hasFailureNotice,
					"Reject a non-leader NPC entry without splitting or silently ignoring the party");
				for (const auto& session : fixture->Sessions) clearOutbound(*session);
				fixture->Source->Handle_ConfirmNpcEntry(
					fixture->Sessions.front()->Get_SessionId(), confirm);
				leaderBatch = 1u == fixture->Source->m_PendingWorldTransfers.size() &&
					fixture->Source->Try_DequeueWorldTransfer(fixture->Request) &&
					count == fixture->Request.PartyBatchSessionIds.size();
			}
			CServerApp::SESSION_WORLD_TRANSFER_FAILURE failure{};
			const bool committed = leaderBatch && fixture->App->Transfer_SessionWorld(
				fixture->Source, fixture->Request, failure);
			bool exactRoster = committed && fixture->Source->m_Players.empty() &&
				fixture->Source->m_PartyMembersByPartyId.empty() &&
				count == fixture->Target->m_Players.size() &&
				1u == fixture->Target->m_PartyMembersByPartyId.size();
			for (const auto& session : fixture->Sessions)
			{
				std::size_t accepted = 0u;
				S2C_PARTY_ROSTER roster{};
				for (const auto& frame : session->m_OutboundFrames)
				{
					if (PACKET_TYPE::S2C_ENTER_ACCEPTED == frame.ePacketType) ++accepted;
					if (PACKET_TYPE::S2C_PARTY_ROSTER != frame.ePacketType) continue;
					CPacketReader reader{ std::span<const std::uint8_t>{ frame.Bytes }.subspan(PACKET_HEADER_BYTES) };
					exactRoster = Read_Message(reader, roster) && exactRoster;
				}
				exactRoster = exactRoster && 1u == accepted && count == roster.Members.size() &&
					WORLD_ID::VALTAN_ARENA == fixture->App->m_GameplayBindingBySessionId.at(
						session->Get_SessionId()).eWorldId;
				for (std::size_t index = 0; index < roster.Members.size(); ++index)
					exactRoster = exactRoster && roster.Members[index].strNickname ==
						"PartyContract" + std::to_string(index);
			}
			tests.Require(exactRoster,
				"Atomically transfer the full party and preserve leader-first roster for every member");
		}
		{
			auto fixture = makeParty(2u, true);
			tests.Require(fixture->Ready, "Create party transfer rollback fixture");
			if (fixture->Ready)
			{
				for (auto& [id, player] : fixture->Source->m_Players)
				{
					(void)id;
					player.iCurrentHp = 37u;
					player.fPositionX += 0.25f;
				}
				const auto sourcePlayers = fixture->Source->m_Players;
				const auto sourceParties = fixture->Source->m_PartyMembersByPartyId;
				const auto originalCatalog = fixture->Target->m_GameplayCatalog;
				const auto originalWorldEntities = fixture->Target->m_WorldEntities;
				auto& placements = const_cast<std::vector<WORLD_BOOTSTRAP_PLACEMENT>&>(
					fixture->Target->m_WorldBootstrap.Get_Placements());
				std::vector<WORLD_BOOTSTRAP_PLACEMENT*> spawns;
				for (auto& placement : placements)
					if (placement.isEnabled && WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind)
						spawns.push_back(&placement);
				const char* names[] = { "Reject full target without source departure",
					"Reject missing target profile without source departure",
					"Rollback a second-member navigation failure without source departure",
					"Reject a terminal party member without moving the leader",
					"Rollback a second-member reliable queue failure without publishing acceptance",
					"Reject malformed initial world payload before moving any party member" };
				for (std::size_t scenario = 0; scenario < 6u; ++scenario)
				{
					if (spawns.size() < 3u) { tests.Require(false, names[scenario]); continue; }
					const float secondSpawnX = spawns[1]->fPositionX;
					if (0u == scenario)
					{
						for (std::size_t index = 0; index < 3u; ++index)
						{
							SERVER_PLAYER occupied{};
							occupied.iPlayerId = static_cast<PLAYER_ID>(98000u + index);
							occupied.strSpawnPlacementId = spawns[index]->strPlacementId;
							fixture->Target->m_Players.emplace(occupied.iPlayerId, occupied);
						}
					}
					else if (1u == scenario) fixture->Target->m_GameplayCatalog = CGameplayCatalogGenerations{};
					else if (2u == scenario) spawns[1]->fPositionX = std::numeric_limits<float>::quiet_NaN();
					else if (3u == scenario) fixture->Sessions[1]->Request_Close();
					else if (4u == scenario)
					{
						const std::array<std::uint8_t, 1> payload{ 1u };
						for (std::size_t index = 0; index < CClientSession::MAX_OUTBOUND_FRAME_COUNT; ++index)
							(void)fixture->Sessions[1]->Send_Frame(PACKET_TYPE::S2C_CHAT, payload);
					}
					else if (5u == scenario)
					{
						// Valtan can start with no enabled actors. Always insert the
						// malformed payload instead of silently skipping fault injection.
						SERVER_WORLD_ENTITY malformed{};
						malformed.eKind = WORLD_BOOTSTRAP_KIND::NPC;
						malformed.strArchetypeId = "npc.contract.invalid";
						malformed.strPlacementId = "contract.party.malformed";
						malformed.iNetEntityId = INVALID_NET_ENTITY_ID;
						fixture->Target->m_WorldEntities.push_back(std::move(malformed));
					}
					const std::size_t targetCount = fixture->Target->m_Players.size();
					const auto targetNextId = fixture->Target->m_iNextPlayerId;
					CServerApp::SESSION_WORLD_TRANSFER_FAILURE failure{};
					bool preserved = !fixture->App->Transfer_SessionWorld(fixture->Source,
						fixture->Request, failure) && !failure.strContext.empty() &&
						fixture->Source->m_Players.size() == sourcePlayers.size() &&
						fixture->Source->m_PartyMembersByPartyId == sourceParties &&
						fixture->Target->m_Players.size() == targetCount &&
						fixture->Target->m_iNextPlayerId == targetNextId;
					for (std::size_t index = 0; index < fixture->Sessions.size(); ++index)
					{
						const auto& session = fixture->Sessions[index];
						const auto found = fixture->Source->m_Players.find(session->Get_PlayerId());
						preserved = preserved && found != fixture->Source->m_Players.end() &&
							WORLD_ID::BERN == fixture->App->m_GameplayBindingBySessionId.at(session->Get_SessionId()).eWorldId &&
							fixture->Source == fixture->App->m_GameplayBindingBySessionId.at(session->Get_SessionId()).pSimulation;
						if (found != fixture->Source->m_Players.end())
						{
							const auto& before = sourcePlayers.at(found->first);
							preserved = preserved && before.iCurrentHp == found->second.iCurrentHp &&
								before.fPositionX == found->second.fPositionX && before.fPositionZ == found->second.fPositionZ;
						}
						for (const auto& frame : session->m_OutboundFrames)
							preserved = preserved && PACKET_TYPE::S2C_ENTER_ACCEPTED != frame.ePacketType;
						if (3u != scenario || 1u != index) preserved = preserved && !session->Is_Closing();
					}
					tests.Require(preserved, names[scenario]);
					fixture->Target->m_Players.clear();
					fixture->Target->m_GameplayCatalog = originalCatalog;
					spawns[1]->fPositionX = secondSpawnX;
					fixture->Target->m_WorldEntities = originalWorldEntities;
					fixture->Sessions[1]->m_CloseDiagnostic = {};
					fixture->Sessions[1]->m_isSendRunning.store(true);
					for (const auto& session : fixture->Sessions) clearOutbound(*session);
				}
				const std::array<std::uint8_t, 1> payload{ 1u };
				for (std::size_t index = 0; index < CClientSession::MAX_OUTBOUND_FRAME_COUNT; ++index)
					(void)fixture->Sessions[0]->Send_Frame(PACKET_TYPE::S2C_CHAT, payload);
				fixture->Source->Notify_PartyTransferFailure(fixture->Sessions[0]->Get_SessionId(),
					81u, WORLD_ID::VALTAN_ARENA, PARTY_TRANSFER_RESULT::REJECTED_OUTBOUND_BUSY);
				const bool pendingNotice = !fixture->Sessions[0]->Is_Closing() &&
					1u == fixture->Source->m_PendingPartyTransferResults.size();
				clearOutbound(*fixture->Sessions[0]);
				fixture->Source->Flush_PartyTransferResults();
				tests.Require(pendingNotice && fixture->Source->m_PendingPartyTransferResults.empty() &&
					1u == fixture->Sessions[0]->m_OutboundFrames.size() &&
					PACKET_TYPE::S2C_PARTY_TRANSFER_RESULT == fixture->Sessions[0]->m_OutboundFrames.front().ePacketType,
					"Delay a busy-queue failure notice without disconnecting the preserved source party");
			}
		}
		{
			auto fixture = makeParty(3u, false);
			tests.Require(fixture->Ready, "Create replaced-invitation identity fixture");
			if (fixture->Ready)
			{
				const auto firstId = fixture->Sessions[0]->Get_PlayerId();
				const auto secondId = fixture->Sessions[1]->Get_PlayerId();
				const auto targetId = fixture->Sessions[2]->Get_PlayerId();
				C2S_PARTY_INVITE invite{};
				invite.iRequestSequence = 1u;
				invite.iTargetNetEntityId = fixture->Source->m_Players.at(targetId).iNetEntityId;
				fixture->Source->Handle_PartyInvite(fixture->Sessions[0]->Get_SessionId(), invite);
				fixture->Source->Handle_PartyInvite(fixture->Sessions[1]->Get_SessionId(), invite);
				C2S_PARTY_INVITE_RESPOND respond{};
				respond.iRequestSequence = 2u;
				respond.iFromNetEntityId = fixture->Source->m_Players.at(firstId).iNetEntityId;
				bool preserved = true;
				for (const bool accepted : { false, true })
				{
					respond.bAccepted = accepted;
					fixture->Source->Handle_PartyInviteRespond(fixture->Sessions[2]->Get_SessionId(), respond);
					const auto pending = fixture->Source->m_PendingPartyInviteByTargetPlayerId.find(targetId);
					preserved = preserved && pending != fixture->Source->m_PendingPartyInviteByTargetPlayerId.end() &&
						pending->second == secondId && fixture->Source->m_PartyMembersByPartyId.empty();
				}
				respond.iFromNetEntityId = fixture->Source->m_Players.at(secondId).iNetEntityId;
				fixture->Source->Handle_PartyInviteRespond(fixture->Sessions[2]->Get_SessionId(), respond);
				tests.Require(preserved && fixture->Source->m_PendingPartyInviteByTargetPlayerId.empty() &&
					1u == fixture->Source->m_PartyMembersByPartyId.size(),
					"Stale accept and decline preserve a replacement invite until its exact inviter is answered");
			}
		}
	}
	CServerNavigation groundTargetNavigation;
	const bool groundTargetNavigationLoaded =
		groundTargetNavigation.Load("LV_LUT_HEARTRB_ED");
	tests.Require_GroundTarget(groundTargetNavigationLoaded,
		"Load authoritative navigation for ground-target skill contracts");
	for (const QUICK_SKILL_CONTRACT& contract : QUICK_SKILLS)
	{
		const PLAYER_SKILL_DEFINITION* skill =
			catalog.Find_Skill(contract.skillId);
		tests.Require(
			nullptr != skill &&
			skill->eCharacterClass == contract.characterClass &&
			skill->strInputSlot == contract.inputSlot,
			"Resolve playable skill binding");
		tests.Require(
			nullptr != skill &&
			(skill->strDamageProfileId.empty() ?
				0u == catalog.Find_DamageRatePercent(skill->strDamageProfileId) :
				0u != catalog.Find_DamageRatePercent(skill->strDamageProfileId)),
			"Resolve playable skill damage policy");

		SERVER_PLAYER quickPlayer{};
		quickPlayer.eCharacterClass = contract.characterClass;
		quickPlayer.eStance = nullptr != skill ?
			skill->eRequiredStance : PLAYER_STANCE_ID::NONE;
		quickPlayer.iCurrentHp = 1;
		quickPlayer.iMaximumHp = 1;
		/* Official CostMp runs 206..938 at the reference level, so the test pool
		matches the published class pool rather than the old 100. */
		quickPlayer.iCurrentResource = 1000;
		quickPlayer.iMaximumResource = 1000;
		/* Same idea for identityCost (Artist's moon/sun orbs): a class without
		a gauge finds nullptr and stays at 0, which is correct there too. */
		const PLAYER_RUNTIME_PROFILE* quickIdentityProfile =
			catalog.Find_Player(contract.characterClass);
		quickPlayer.iMaximumIdentity = nullptr != quickIdentityProfile ?
			quickIdentityProfile->iMaximumIdentity : 0u;
		quickPlayer.iCurrentIdentity = quickPlayer.iMaximumIdentity;
		C2S_USE_SKILL quickCommand{};
		quickCommand.iClientSequence = 1;
		quickCommand.iSkillId = contract.skillId;
		const bool isGroundTargetSkill = nullptr != skill &&
			SKILL_TARGET_INTENT_KIND::GROUND_POINT == skill->eTargetIntent;
		if (isGroundTargetSkill)
		{
			quickPlayer.fPositionX = 147.75f;
			quickPlayer.fPositionZ = -117.25f;
			quickCommand.eTargetIntent =
				SKILL_TARGET_INTENT_KIND::GROUND_POINT;
			quickCommand.fAimX = 156.25f;
			quickCommand.fAimZ = -122.25f;
		}
		else
		{
			quickCommand.fAimX = 1.f;
			quickCommand.fAimZ = 0.f;
		}
		CPlayerSkillSystem quickSkillSystem;
		tests.Require(
			quickSkillSystem.Try_Start(
				quickPlayer,
				quickCommand,
				catalog,
				1,
				isGroundTargetSkill ? &groundTargetNavigation : nullptr),
			"Approve playable skill command");
	}
	{
		constexpr SKILL_ID DIMENSIONMASTER_T = 2050500u;
		const PLAYER_SKILL_DEFINITION* skill =
			catalog.Find_Skill(DIMENSIONMASTER_T);
		SERVER_NAV_POINT approvedTarget{};
		const bool targetSampled = groundTargetNavigation.Sample_Position(
			156.25f, -122.25f, approvedTarget);
		const bool exactTargetingDefinition = nullptr != skill &&
			SKILL_TARGET_INTENT_KIND::GROUND_POINT == skill->eTargetIntent &&
			std::fabs(skill->fTargetMaximumRange - 11.f) < 0.0001f &&
			skill->requiresWalkableTarget && 938u == skill->iResourceCost &&
			4367u == skill->iActionDurationMs &&
			2858u == skill->iHitTimeMs;
		tests.Require_GroundTarget(exactTargetingDefinition && targetSampled,
			"Resolve DimensionMaster T as exact 11m walkable ground-target skill");

		auto makePlayer = []()
		{
			SERVER_PLAYER player{};
			player.eCharacterClass = CHARACTER_CLASS_ID::DIMENSIONMASTER;
			player.iCurrentHp = 1000u;
			player.iMaximumHp = 1000u;
			player.iCurrentResource = 1000u;
			player.iMaximumResource = 1000u;
			player.fPositionX = 147.75f;
			player.fPositionY = 0.f;
			player.fPositionZ = -117.25f;
			return player;
		};
		const auto gameplayStateUnchanged = [](
			const SERVER_PLAYER& before,
			const SERVER_PLAYER& after)
		{
			return before.iLastSkillSequence == after.iLastSkillSequence &&
				before.iCurrentResource == after.iCurrentResource &&
				before.eAction == after.eAction &&
				before.iCurrentSkillId == after.iCurrentSkillId &&
				before.CooldownEndTickBySkillId ==
					after.CooldownEndTickBySkillId &&
				before.hasSkillTarget == after.hasSkillTarget;
		};

		CPlayerSkillSystem skills;
		SERVER_PLAYER rejected = makePlayer();
		C2S_USE_SKILL command{};
		command.iClientSequence = 1u;
		command.iSkillId = DIMENSIONMASTER_T;
		command.fAimX = approvedTarget.x;
		command.fAimZ = approvedTarget.z;
		const SERVER_PLAYER beforeWrongIntent = rejected;
		const bool wrongIntentRejected = !skills.Try_Start(
			rejected, command, catalog, 10u, &groundTargetNavigation) &&
			gameplayStateUnchanged(beforeWrongIntent, rejected);

		command.eTargetIntent = SKILL_TARGET_INTENT_KIND::GROUND_POINT;
		command.fAimX = rejected.fPositionX + 11.01f;
		command.fAimZ = rejected.fPositionZ;
		const SERVER_PLAYER beforeOutOfRange = rejected;
		const bool outOfRangeRejected = !skills.Try_Start(
			rejected, command, catalog, 10u, &groundTargetNavigation) &&
			gameplayStateUnchanged(beforeOutOfRange, rejected);

		command.fAimX = std::numeric_limits<float>::quiet_NaN();
		const SERVER_PLAYER beforeNonFinite = rejected;
		const bool nonFiniteRejected = !skills.Try_Start(
			rejected, command, catalog, 10u, &groundTargetNavigation) &&
			gameplayStateUnchanged(beforeNonFinite, rejected);

		command.fAimX = approvedTarget.x;
		command.fAimZ = approvedTarget.z;
		command.iSkillId = 0xfefefefeu;
		const SERVER_PLAYER beforeUnknown = rejected;
		const bool unknownRejected = !skills.Try_Start(
			rejected, command, catalog, 10u, &groundTargetNavigation) &&
			gameplayStateUnchanged(beforeUnknown, rejected);
		tests.Require_GroundTarget(
			wrongIntentRejected && outOfRangeRejected &&
			nonFiniteRejected && unknownRejected,
			"Reject wrong intent, out-of-range, non-finite and unknown T without mutation");

		float lastWalkableX = 152.f;
		float firstBlockedX = 0.f;
		SERVER_NAV_POINT navProbe{};
		bool foundNearbyBlockedPoint = groundTargetNavigation.Sample_Position(
			lastWalkableX, -137.f, navProbe);
		const float probeStep = groundTargetNavigation.Get_CellSize() * 0.25f;
		if (foundNearbyBlockedPoint)
		{
			foundNearbyBlockedPoint = false;
			const float probeLimitX = lastWalkableX + 60.f;
			for (float x = lastWalkableX + probeStep;
				x <= probeLimitX; x += probeStep)
			{
				if (groundTargetNavigation.Sample_Position(x, -137.f, navProbe))
				{
					lastWalkableX = x;
					continue;
				}
				firstBlockedX = x;
				foundNearbyBlockedPoint = true;
				break;
			}
		}
		SERVER_PLAYER navRejected = makePlayer();
		navRejected.fPositionX = lastWalkableX;
		navRejected.fPositionZ = -137.f;
		command.iSkillId = DIMENSIONMASTER_T;
		command.iClientSequence = 1u;
		command.fAimX = firstBlockedX;
		command.fAimZ = -137.f;
		const SERVER_PLAYER beforeNavReject = navRejected;
		const bool navInvalidRejected = foundNearbyBlockedPoint &&
			!skills.Try_Start(navRejected, command, catalog, 10u,
				&groundTargetNavigation) &&
			gameplayStateUnchanged(beforeNavReject, navRejected);
		tests.Require_GroundTarget(navInvalidRejected,
			"Reject a within-range but non-walkable DimensionMaster T target transactionally");

		SERVER_PLAYER approved = makePlayer();
		command.iClientSequence = 1u;
		command.iSkillId = DIMENSIONMASTER_T;
		command.fAimX = approvedTarget.x;
		command.fAimZ = approvedTarget.z;
		const bool started = skills.Try_Start(
			approved, command, catalog, 20u, &groundTargetNavigation);
		const std::uint32_t resourceAfterStart = approved.iCurrentResource;
		const auto cooldownsAfterStart = approved.CooldownEndTickBySkillId;
		const bool approvedTargetPreserved = started &&
			PLAYER_ACTION_STATE::SKILL == approved.eAction &&
			approved.hasSkillTarget &&
			std::fabs(approved.fSkillTargetX - approvedTarget.x) < 0.0001f &&
			std::fabs(approved.fSkillTargetY - approvedTarget.y) < 0.0001f &&
			std::fabs(approved.fSkillTargetZ - approvedTarget.z) < 0.0001f &&
			1000u - skill->iResourceCost == resourceAfterStart &&
			cooldownsAfterStart.contains(DIMENSIONMASTER_T);
		const bool duplicateRejected = !skills.Try_Start(
			approved, command, catalog, 21u, &groundTargetNavigation) &&
			resourceAfterStart == approved.iCurrentResource &&
			cooldownsAfterStart == approved.CooldownEndTickBySkillId &&
			1u == approved.iLastSkillSequence;
		tests.Require_GroundTarget(approvedTargetPreserved && duplicateRejected,
			"Commit approved T target/cost once and reject duplicate sequence without mutation");

		CGameRoom snapshotRoom{ WORLD_ID::TRAINING_GROUND };
		constexpr SESSION_ID SNAPSHOT_SESSION = 91001u;
		constexpr PLAYER_ID SNAPSHOT_PLAYER = 91001u;
		auto snapshotSession = std::make_shared<CClientSession>(
			SNAPSHOT_SESSION, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		snapshotSession->m_isSendRunning.store(true);
		SERVER_PLAYER snapshotPlayer = approved;
		snapshotPlayer.iSessionId = SNAPSHOT_SESSION;
		snapshotPlayer.iPlayerId = SNAPSHOT_PLAYER;
		snapshotPlayer.iNetEntityId = 91001u;
		snapshotRoom.m_iServerTick = 20u;
		snapshotRoom.m_Sessions.emplace(SNAPSHOT_SESSION, snapshotSession);
		snapshotRoom.m_PlayerIdBySessionId.emplace(
			SNAPSHOT_SESSION, SNAPSHOT_PLAYER);
		snapshotRoom.m_Players.emplace(SNAPSHOT_PLAYER, snapshotPlayer);
		snapshotRoom.Broadcast_WorldSnapshot();
		S2C_WORLD_SNAPSHOT decodedSnapshot{};
		bool snapshotTargetPreserved = false;
		if (1u == snapshotSession->m_OutboundFrames.size())
		{
			const std::vector<std::uint8_t>& frame =
				snapshotSession->m_OutboundFrames.front().Bytes;
			PACKET_HEADER header{};
			if (Read_Packet_Header(frame, header) &&
				PACKET_TYPE::S2C_WORLD_SNAPSHOT == header.ePacketType &&
				header.iTotalSize == frame.size())
			{
				CPacketReader reader{ std::span<const std::uint8_t>(
					frame.data() + PACKET_HEADER_BYTES,
					frame.size() - PACKET_HEADER_BYTES) };
				if (Read_Message(reader, decodedSnapshot) &&
					0u == reader.Get_RemainingSize())
				{
					const auto replicated = std::find_if(
						decodedSnapshot.Players.begin(),
						decodedSnapshot.Players.end(),
						[](const PLAYER_SNAPSHOT& player)
						{ return 91001u == player.iNetEntityId; });
					snapshotTargetPreserved =
						decodedSnapshot.Players.end() != replicated &&
						replicated->hasSkillTarget &&
						std::fabs(replicated->fSkillTargetX - approvedTarget.x) < 0.0001f &&
						std::fabs(replicated->fSkillTargetY - approvedTarget.y) < 0.0001f &&
						std::fabs(replicated->fSkillTargetZ - approvedTarget.z) < 0.0001f &&
						decodedSnapshot.ActiveGameplayRevision ==
							snapshotRoom.m_GameplayCatalog.Get_ActiveRevision() &&
						decodedSnapshot.RequiredPinnedGameplayRevisions.empty() &&
						std::all_of(
							decodedSnapshot.Entities.begin(),
							decodedSnapshot.Entities.end(),
							[&decodedSnapshot](const WORLD_ENTITY_SNAPSHOT& entity)
							{
								return entity.PinnedDefinitionRevision ==
									decodedSnapshot.ActiveGameplayRevision;
							});
				}
			}
		}
		snapshotSession->Request_Close();
		tests.Require_GroundTarget(snapshotTargetPreserved,
			"Replicate the approved T target XYZ through the canonical world snapshot");

		SERVER_WORLD_ENTITY atTarget{};
		atTarget.iNetEntityId = 92001u;
		atTarget.eKind = WORLD_BOOTSTRAP_KIND::MONSTER;
		atTarget.eAction = SERVER_ENTITY_ACTION::IDLE;
		atTarget.iCurrentHp = 100000u;
		atTarget.iMaximumHp = 100000u;
		atTarget.fPositionX = approvedTarget.x;
		atTarget.fPositionY = approvedTarget.y;
		atTarget.fPositionZ = approvedTarget.z;
		atTarget.fCollisionRadius = 0.25f;
		SERVER_WORLD_ENTITY atCaster = atTarget;
		atCaster.iNetEntityId = 92002u;
		atCaster.fPositionX = approved.fPositionX;
		atCaster.fPositionY = approved.fPositionY;
		atCaster.fPositionZ = approved.fPositionZ;
		std::vector<SERVER_WORLD_ENTITY> targets{ atTarget, atCaster };
		std::vector<DAMAGE_EVENT> damageEvents;
		skills.Update(approved, targets, catalog, &groundTargetNavigation,
			nullptr, 2.859f, 21u, damageEvents);
		const bool damagedAtApprovedRoot = 1u == damageEvents.size() &&
			92001u == damageEvents.front().iTargetNetEntityId &&
			targets[0].iCurrentHp < targets[0].iMaximumHp &&
			targets[1].iCurrentHp == targets[1].iMaximumHp;
		tests.Require_GroundTarget(damagedAtApprovedRoot,
			"Resolve DimensionMaster T damage at approved target root, not caster root");

		skills.Update(approved, targets, catalog, &groundTargetNavigation,
			nullptr, 2.f, 22u, damageEvents);
		tests.Require_GroundTarget(
			PLAYER_ACTION_STATE::NONE == approved.eAction &&
			!approved.hasSkillTarget &&
			0.f == approved.fSkillTargetX && 0.f == approved.fSkillTargetY &&
			0.f == approved.fSkillTargetZ,
			"Clear approved T target atomically when the action ends");

		SERVER_PLAYER pending = makePlayer();
		pending.eAction = PLAYER_ACTION_STATE::SKILL;
		pending.iCurrentSkillId = 2050010u;
		pending.iLastSkillSequence = 1u;
		command.iClientSequence = 2u;
		command.iSkillId = DIMENSIONMASTER_T;
		command.eTargetIntent = SKILL_TARGET_INTENT_KIND::GROUND_POINT;
		command.fAimX = approvedTarget.x;
		command.fAimZ = approvedTarget.z;
		const bool pendingStaged = skills.Try_StagePendingSkill(
			pending, command, catalog, &groundTargetNavigation);
		pending.eAction = PLAYER_ACTION_STATE::NONE;
		pending.iCurrentSkillId = INVALID_SKILL_ID;
		pending.fPositionX = approvedTarget.x - 20.f;
		pending.fPositionZ = approvedTarget.z;
		pending.PendingCommand.Clear();
		const std::uint32_t pendingResource = pending.iCurrentResource;
		const auto pendingCooldowns = pending.CooldownEndTickBySkillId;
		const bool pendingRevalidated = !skills.Try_StartPending(
			pending, command, catalog, 30u, &groundTargetNavigation) &&
			pendingResource == pending.iCurrentResource &&
			pendingCooldowns == pending.CooldownEndTickBySkillId &&
			!pending.hasSkillTarget &&
			PLAYER_ACTION_STATE::NONE == pending.eAction;
		tests.Require_GroundTarget(pendingStaged && pendingRevalidated,
			"Revalidate a buffered T target at actual start and reject stale range without cost");

		SERVER_PLAYER interrupted = makePlayer();
		command.iClientSequence = 1u;
		const bool interruptedStarted = skills.Try_Start(
			interrupted, command, catalog, 40u, &groundTargetNavigation);
		interrupted.iCurrentHp = 0u;
		std::vector<SERVER_WORLD_ENTITY> noTargets;
		std::vector<DAMAGE_EVENT> noDamageEvents;
		skills.Update(interrupted, noTargets, catalog, &groundTargetNavigation,
			nullptr, 0.f, 41u, noDamageEvents);
		const bool deathCleared = interruptedStarted &&
			PLAYER_ACTION_STATE::DEAD == interrupted.eAction &&
			INVALID_SKILL_ID == interrupted.iCurrentSkillId &&
			!interrupted.hasSkillTarget &&
			0.f == interrupted.fSkillTargetX &&
			0.f == interrupted.fSkillTargetY &&
			0.f == interrupted.fSkillTargetZ;

		SERVER_PLAYER knockedDown = makePlayer();
		command.iClientSequence = 1u;
		const bool knockdownStarted = skills.Try_Start(
			knockedDown, command, catalog, 50u, &groundTargetNavigation);
		CPlayerSkillSystem::Arm_PlayerHitReaction(
			knockedDown, knockedDown.fPositionX - 1.f,
			knockedDown.fPositionZ, 1.f, 100u, true, 1000u, 51u);
		const bool knockdownCleared = knockdownStarted &&
			PLAYER_ACTION_STATE::KNOCKDOWN == knockedDown.eAction &&
			INVALID_SKILL_ID == knockedDown.iCurrentSkillId &&
			!knockedDown.hasSkillTarget &&
			0.f == knockedDown.fSkillTargetX &&
			0.f == knockedDown.fSkillTargetY &&
			0.f == knockedDown.fSkillTargetZ;
		tests.Require_GroundTarget(deathCleared && knockdownCleared,
			"Clear approved T target on death and knockdown interruption");
	}
	if (dimensionMasterGroundTargetOnly)
	{
		std::cout << "failures : " << tests.failures << '\n';
		return 0 == tests.failures ? 0 : 1;
	}
	for (const BASIC_ATTACK_CONTRACT& contract : BASIC_ATTACKS)
	{
		const PLAYER_SKILL_DEFINITION* combo = catalog.Find_Skill(contract.skillId);
		bool stageTimingsValid = nullptr != combo &&
			combo->ComboStages.size() == contract.stageCount;
		if (stageTimingsValid)
		{
			for (const PLAYER_COMBO_STAGE& stage : combo->ComboStages)
			{
				stageTimingsValid = stageTimingsValid &&
					stage.iHitTimeMs <= stage.iComboAdvanceMs &&
					stage.iComboAdvanceMs <= stage.iActionDurationMs;
			}
			stageTimingsValid = stageTimingsValid &&
				combo->ComboStages.back().iComboAdvanceMs ==
					combo->ComboStages.back().iActionDurationMs;
		}
		tests.Require(
			nullptr != combo &&
			combo->eCharacterClass == contract.characterClass &&
			combo->strInputSlot == "LMB" &&
			PLAYER_SKILL_KIND::COMBO == combo->eSkillKind &&
			stageTimingsValid &&
			0u == combo->ComboStages.back().iInputCloseMs,
			"Resolve playable basic attack combo with explicit stage boundaries");
		tests.Require(
			nullptr != combo &&
			0u != catalog.Find_DamageRatePercent(combo->strDamageProfileId),
			"Resolve playable basic attack damage rate");
		if (nullptr == combo || combo->ComboStages.size() < 2u)
			continue;

		SERVER_PLAYER comboPlayer{};
		comboPlayer.eCharacterClass = contract.characterClass;
		comboPlayer.eStance = combo->eRequiredStance;
		comboPlayer.iCurrentHp = 1000;
		comboPlayer.iMaximumHp = 1000;
		comboPlayer.iCurrentResource = 1000;
		comboPlayer.iMaximumResource = 1000;
		C2S_USE_SKILL press{};
		press.iClientSequence = 1;
		press.iSkillId = contract.skillId;
		press.fAimX = 1.f;
		press.fAimZ = 0.f;
		CPlayerSkillSystem comboSystem;
		tests.Require(
			comboSystem.Try_Start(comboPlayer, press, catalog, 10) &&
			1u == comboPlayer.iComboStage,
			"Approve playable basic attack first stage");
		SERVER_PLAYER tappedPlayer = comboPlayer;

		const PLAYER_COMBO_STAGE& firstStage = combo->ComboStages.front();
		const bool automaticFirstStage = 0u == firstStage.iInputOpenMs &&
			0u == firstStage.iInputCloseMs;
		if (automaticFirstStage)
		{
			press.iClientSequence = 2u;
			comboSystem.Try_Start(comboPlayer, press, catalog, 11u);
			tests.Require(
				!comboPlayer.hasBufferedComboInput &&
				1u == comboPlayer.iLastSkillSequence &&
				1u == comboPlayer.iComboStage,
				"Ignore repeated input while an automatic basic attack owns its sequence");
		}
		else
		{
			comboPlayer.fActionElapsedSeconds =
				static_cast<float>(firstStage.iInputOpenMs +
					firstStage.iInputCloseMs) * 0.0005f;
			press.iClientSequence = 2;
			comboSystem.Try_Start(comboPlayer, press, catalog, 11);
			tests.Require(
				comboPlayer.hasBufferedComboInput,
				"Buffer playable basic attack inside its input window");

			/* COMBO uses another USE_SKILL as its continuation. A mouse-up packet is
			HOLD-only and must not consume sequence or revoke a repeated click/hold. */
			C2S_RELEASE_SKILL release{};
			release.iClientSequence = 3u;
			release.iSkillId = contract.skillId;
			comboSystem.Release(comboPlayer, release, catalog);
			tests.Require(
				comboPlayer.hasBufferedComboInput &&
				2u == comboPlayer.iLastSkillSequence &&
				1u == comboPlayer.iComboStage,
				"Ignore COMBO mouse-up without consuming its continuation sequence");

			std::vector<SERVER_WORLD_ENTITY> noTargets;
			std::vector<DAMAGE_EVENT> noDamageEvents;
			tappedPlayer.fActionElapsedSeconds =
				static_cast<float>(firstStage.iActionDurationMs - 1u) * 0.001f;
			comboSystem.Update(
				tappedPlayer, noTargets, catalog, nullptr, nullptr,
				0.002f, 12u, noDamageEvents);
			tests.Require(
				PLAYER_ACTION_STATE::NONE == tappedPlayer.eAction &&
				0u == tappedPlayer.iComboStage,
				"End a buffered-input basic attack at stage one without continuation");

			for (std::uint32_t tick = 12;
				tick < 132 && comboPlayer.iComboStage < 2u;
				++tick)
			{
				comboSystem.Update(
					comboPlayer,
					noTargets,
					catalog,
					nullptr,
					nullptr,
					1.f / 30.f,
					tick,
					noDamageEvents);
			}
			tests.Require(
				2u == comboPlayer.iComboStage,
				"Advance buffered-input basic attack from its Server-owned window");
		}

		SERVER_PLAYER wrongClassPlayer{};
		wrongClassPlayer.eCharacterClass =
			CHARACTER_CLASS_ID::LANCE_MASTER == contract.characterClass ?
			CHARACTER_CLASS_ID::GUNSLINGER :
			CHARACTER_CLASS_ID::LANCE_MASTER;
		wrongClassPlayer.iCurrentHp = 1000;
		wrongClassPlayer.iMaximumHp = 1000;
		wrongClassPlayer.iCurrentResource = 1000;
		wrongClassPlayer.iMaximumResource = 1000;
		CPlayerSkillSystem wrongClassSystem;
			tests.Require(
				!wrongClassSystem.Try_Start(
					wrongClassPlayer, press, catalog, 10),
				"Reject another class's basic attack");
	}
	{
		const PLAYER_SKILL_DEFINITION* warlordBasicAttack =
			catalog.Find_Skill(17000u);
		bool preservedRepeatedStageDamage = nullptr != warlordBasicAttack &&
			warlordBasicAttack->ComboStages.size() == 3u &&
			600u == warlordBasicAttack->ComboStages[1].iComboAdvanceMs &&
			warlordBasicAttack->ComboStages[1].Hits.size() == 1u &&
			3u == warlordBasicAttack->ComboStages[1].Hits.front().iRepeatCount;
		if (preservedRepeatedStageDamage)
		{
			SERVER_PLAYER player{};
			player.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
			player.eStance = PLAYER_STANCE_ID::WARLORD_NORMAL;
			player.iCurrentHp = 1000u;
			player.iMaximumHp = 1000u;
			player.iCurrentResource = 1000u;
			player.iMaximumResource = 1000u;
			C2S_USE_SKILL command{};
			command.iClientSequence = 1u;
			command.iSkillId = 17000u;
			command.fAimX = 4.f;
			command.fAimZ = 0.f;
			CPlayerSkillSystem skills;
			preservedRepeatedStageDamage =
				skills.Try_Start(player, command, catalog, 80u);
			player.iComboStage = 2u;
			player.fActionElapsedSeconds = 0.199f;
			player.hasBufferedComboInput = true;
			player.hasAppliedSkillDamage = false;
			player.iAppliedHitMask = 0u;

			SERVER_WORLD_ENTITY target{};
			target.iNetEntityId = 701u;
			target.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
			target.eAction = SERVER_ENTITY_ACTION::IDLE;
			target.strArchetypeId = "BOSS_VALTAN";
			target.iCurrentHp = 100000u;
			target.iMaximumHp = 100000u;
			target.fPositionX = 1.f;
			target.fCollisionRadius = 1.f;
			std::vector<SERVER_WORLD_ENTITY> targets{ target };
			std::vector<DAMAGE_EVENT> events;
			skills.Update(player, targets, catalog, nullptr, nullptr,
				0.002f, 81u, events);
			const bool heldAfterFirst = 2u == player.iComboStage &&
				1u == events.size() && !player.hasAppliedSkillDamage;
			skills.Update(player, targets, catalog, nullptr, nullptr,
				0.2f, 82u, events);
			const bool heldAfterSecond = 2u == player.iComboStage &&
				2u == events.size() && !player.hasAppliedSkillDamage;
			skills.Update(player, targets, catalog, nullptr, nullptr,
				0.2f, 83u, events);
			preservedRepeatedStageDamage = preservedRepeatedStageDamage &&
				heldAfterFirst && heldAfterSecond && 3u == events.size() &&
				3u == player.iComboStage;
		}
		tests.Require(preservedRepeatedStageDamage,
			"Preserve all repeated Warlord BA hits before combo boundary advance");
	}
	{
		const PLAYER_SKILL_DEFINITION* lanceBasicAttack =
			catalog.Find_Skill(34010u);
		bool explicitWaitedForAnimation = nullptr != lanceBasicAttack &&
			lanceBasicAttack->ComboStages.size() == 4u &&
			470u == lanceBasicAttack->ComboStages.front().iComboAdvanceMs &&
			1633u == lanceBasicAttack->ComboStages.front().iActionDurationMs;
		if (explicitWaitedForAnimation)
		{
			SERVER_PLAYER player{};
			player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			player.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
			player.iCurrentHp = 1000u;
			player.iMaximumHp = 1000u;
			player.iCurrentResource = 1000u;
			player.iMaximumResource = 1000u;
			C2S_USE_SKILL command{};
			command.iClientSequence = 1u;
			command.iSkillId = 34010u;
			command.fAimX = 4.f;
			command.fAimZ = 0.f;
			CPlayerSkillSystem skills;
			explicitWaitedForAnimation =
				skills.Try_Start(player, command, catalog, 90u);
			player.fActionElapsedSeconds = 0.4f;
			command.iClientSequence = 2u;
			skills.Try_Start(player, command, catalog, 91u);
			C2S_MOVE pendingMove{};
			pendingMove.iClientSequence = 1u;
			pendingMove.fGoalX = 5.f;
			pendingMove.fGoalZ = 0.f;
			player.PendingCommand.Set_Move(pendingMove);

			std::vector<SERVER_WORLD_ENTITY> noTargets;
			std::vector<DAMAGE_EVENT> noDamageEvents;
			player.fActionElapsedSeconds = 0.469f;
			skills.Update(player, noTargets, catalog, nullptr, nullptr,
				0.002f, 92u, noDamageEvents);
			const bool ignoredEarlyComboBoundary =
				PLAYER_ACTION_STATE::SKILL == player.eAction &&
				1u == player.iComboStage && player.hasBufferedComboInput &&
				PLAYER_PENDING_COMMAND_KIND::MOVE == player.PendingCommand.eKind;
			player.fActionElapsedSeconds = 1.632f;
			skills.Update(player, noTargets, catalog, nullptr, nullptr,
				0.002f, 93u, noDamageEvents);
			explicitWaitedForAnimation = explicitWaitedForAnimation &&
				ignoredEarlyComboBoundary &&
				PLAYER_ACTION_STATE::NONE == player.eAction &&
				0u == player.iComboStage &&
				PLAYER_PENDING_COMMAND_KIND::MOVE == player.PendingCommand.eKind;
		}
		tests.Require(explicitWaitedForAnimation,
			"Commit explicit command after one full BA animation, not its early combo boundary");
	}
	{
		const PLAYER_SKILL_DEFINITION* dimensionMasterBasicAttack =
			catalog.Find_Skill(2050010u);
		constexpr std::array<std::uint32_t, 3u> expectedDurationMs =
			{ 1500u, 1067u, 1700u };
		constexpr std::array<std::uint32_t, 3u> expectedHitMs =
			{ 50u, 28u, 335u };
		constexpr std::array<std::uint32_t, 3u> expectedComboAdvanceMs =
			{ 1500u, 1067u, 1700u };
		constexpr std::array<std::uint32_t, 3u> expectedOpenMs =
			{ 0u, 0u, 0u };
		constexpr std::array<std::uint32_t, 3u> expectedCloseMs =
			{ 0u, 0u, 0u };
		bool exactDimensionMasterTiming =
			nullptr != dimensionMasterBasicAttack &&
			1500u == dimensionMasterBasicAttack->iActionDurationMs &&
			50u == dimensionMasterBasicAttack->iHitTimeMs &&
			dimensionMasterBasicAttack->ComboStages.size() ==
				expectedDurationMs.size();
		if (exactDimensionMasterTiming)
		{
			for (std::size_t index = 0u; index < expectedDurationMs.size(); ++index)
			{
				const PLAYER_COMBO_STAGE& stage =
					dimensionMasterBasicAttack->ComboStages[index];
				exactDimensionMasterTiming = exactDimensionMasterTiming &&
					stage.iActionDurationMs == expectedDurationMs[index] &&
					stage.iHitTimeMs == expectedHitMs[index] &&
					stage.iComboAdvanceMs == expectedComboAdvanceMs[index] &&
					stage.iInputOpenMs == expectedOpenMs[index] &&
					stage.iInputCloseMs == expectedCloseMs[index] &&
					!stage.RootMotion.empty() &&
					stage.RootMotion.back().iTimeMs == expectedDurationMs[index];
			}
		}
		tests.Require(exactDimensionMasterTiming,
			"Resolve full automatic DimensionMaster BA motions and stage-aligned root motion");

		if (nullptr != dimensionMasterBasicAttack)
		{
			CPlayerSkillSystem skills;
			std::vector<SERVER_WORLD_ENTITY> noTargets;
			std::vector<DAMAGE_EVENT> noDamageEvents;
			auto makePlayer = []()
			{
				SERVER_PLAYER player{};
				player.eCharacterClass = CHARACTER_CLASS_ID::DIMENSIONMASTER;
				player.iCurrentHp = 1000u;
				player.iMaximumHp = 1000u;
				player.iCurrentResource = 1000u;
				player.iMaximumResource = 1000u;
				return player;
			};
			C2S_USE_SKILL basicAttack{};
			basicAttack.iClientSequence = 1u;
			basicAttack.iSkillId = 2050010u;
			basicAttack.fAimX = 1.f;
			basicAttack.fAimZ = 0.f;

			SERVER_PLAYER automatic = makePlayer();
			const bool automaticStarted =
				skills.Try_Start(automatic, basicAttack, catalog, 100u);
			bool chainedEveryStage = automaticStarted &&
				!automatic.hasBufferedComboInput;
			std::uint32_t automaticTick = 101u;
			for (std::size_t stageIndex = 0u;
				chainedEveryStage &&
					stageIndex + 1u < dimensionMasterBasicAttack->ComboStages.size();
				++stageIndex)
			{
				const PLAYER_COMBO_STAGE& stage =
					dimensionMasterBasicAttack->ComboStages[stageIndex];
				const std::uint32_t previousStartTick =
					automatic.iActionStartTick;
				automatic.fActionElapsedSeconds =
					static_cast<float>(stage.iComboAdvanceMs - 1u) * 0.001f;
				skills.Update(automatic, noTargets, catalog, nullptr, nullptr,
					0.f, automaticTick++, noDamageEvents);
				chainedEveryStage = chainedEveryStage &&
					PLAYER_ACTION_STATE::SKILL == automatic.eAction &&
					automatic.iComboStage == stageIndex + 1u &&
					previousStartTick == automatic.iActionStartTick;
				skills.Update(automatic, noTargets, catalog, nullptr, nullptr,
					0.002f, automaticTick++, noDamageEvents);
				chainedEveryStage = chainedEveryStage &&
					PLAYER_ACTION_STATE::SKILL == automatic.eAction &&
					automatic.iComboStage == stageIndex + 2u &&
					previousStartTick != automatic.iActionStartTick &&
					!automatic.hasBufferedComboInput;
			}

			const PLAYER_COMBO_STAGE& finalStage =
				dimensionMasterBasicAttack->ComboStages.back();
			automatic.fActionElapsedSeconds =
				static_cast<float>(finalStage.iActionDurationMs - 1u) * 0.001f;
			skills.Update(automatic, noTargets, catalog, nullptr, nullptr,
				0.f, automaticTick++, noDamageEvents);
			const bool heldFinalFullMotion =
				PLAYER_ACTION_STATE::SKILL == automatic.eAction &&
				3u == automatic.iComboStage;
			skills.Update(automatic, noTargets, catalog, nullptr, nullptr,
				0.002f, automaticTick++, noDamageEvents);
			tests.Require(
				chainedEveryStage && heldFinalFullMotion &&
					PLAYER_ACTION_STATE::NONE == automatic.eAction &&
					0u == automatic.iComboStage,
				"Advance one DimensionMaster LMB through the three project-tuned BA motions");
		}
	}
	{
		const PLAYER_SKILL_DEFINITION* basicAttackDefinition =
			catalog.Find_Skill(2050010u);
		const PLAYER_SKILL_DEFINITION* pendingSkillDefinition =
			catalog.Find_Skill(2050120u);
		SERVER_PLAYER player{};
		player.eCharacterClass = CHARACTER_CLASS_ID::DIMENSIONMASTER;
		player.iCurrentHp = 1000u;
		player.iMaximumHp = 1000u;
		player.iCurrentResource = 1000u;
		player.iMaximumResource = 1000u;
		CPlayerSkillSystem skills;
		C2S_USE_SKILL basicAttack{};
		basicAttack.iClientSequence = 1u;
		basicAttack.iSkillId = 2050010u;
		basicAttack.fAimX = 1.f;
		basicAttack.fAimZ = 0.f;
		const bool started = skills.Try_Start(player, basicAttack, catalog, 200u);

		C2S_USE_SKILL firstExplicit{};
		firstExplicit.iClientSequence = 3u;
		firstExplicit.iSkillId = 2050100u;
		firstExplicit.fAimX = 2.f;
		firstExplicit.fAimZ = 0.f;
		C2S_USE_SKILL latestExplicit = firstExplicit;
		latestExplicit.iClientSequence = 4u;
		latestExplicit.iSkillId = 2050120u;
		const std::uint32_t resourceBeforePending = player.iCurrentResource;
		const auto cooldownsBeforePending = player.CooldownEndTickBySkillId;
		const bool firstStaged =
			skills.Try_StagePendingSkill(player, firstExplicit, catalog);
		const bool latestStaged =
			skills.Try_StagePendingSkill(player, latestExplicit, catalog);
		firstExplicit.iClientSequence = 3u;
		const bool staleRejected =
			!skills.Try_StagePendingSkill(player, firstExplicit, catalog);
		tests.Require(
			started && firstStaged && latestStaged && staleRejected &&
			PLAYER_PENDING_COMMAND_KIND::SKILL == player.PendingCommand.eKind &&
			2050120u == player.PendingCommand.iSkillId &&
			4u == player.iLastSkillSequence && !player.hasBufferedComboInput &&
			resourceBeforePending == player.iCurrentResource &&
			cooldownsBeforePending == player.CooldownEndTickBySkillId,
			"Stage only the latest explicit skill without spending gameplay costs");

		std::vector<SERVER_WORLD_ENTITY> noTargets;
		std::vector<DAMAGE_EVENT> noDamageEvents;
		bool explicitWaitedForAutomaticChain =
			nullptr != basicAttackDefinition &&
				!basicAttackDefinition->ComboStages.empty();
		std::uint32_t pendingBoundaryTick = 202u;
		if (explicitWaitedForAutomaticChain)
		{
			for (std::size_t stageIndex = 0u;
				stageIndex < basicAttackDefinition->ComboStages.size();
				++stageIndex)
			{
				const PLAYER_COMBO_STAGE& stage =
					basicAttackDefinition->ComboStages[stageIndex];
				player.fActionElapsedSeconds =
					static_cast<float>(stage.iActionDurationMs - 1u) * 0.001f;
				skills.Update(player, noTargets, catalog, nullptr, nullptr,
					0.f, pendingBoundaryTick++, noDamageEvents);
				explicitWaitedForAutomaticChain =
					explicitWaitedForAutomaticChain &&
					PLAYER_ACTION_STATE::SKILL == player.eAction &&
					player.iComboStage == stageIndex + 1u &&
					PLAYER_PENDING_COMMAND_KIND::SKILL ==
						player.PendingCommand.eKind;

				skills.Update(player, noTargets, catalog, nullptr, nullptr,
					0.002f, pendingBoundaryTick++, noDamageEvents);
				const bool isFinalStage = stageIndex + 1u ==
					basicAttackDefinition->ComboStages.size();
				explicitWaitedForAutomaticChain =
					explicitWaitedForAutomaticChain &&
					PLAYER_PENDING_COMMAND_KIND::SKILL ==
						player.PendingCommand.eKind &&
					(isFinalStage ?
						(PLAYER_ACTION_STATE::NONE == player.eAction &&
							0u == player.iComboStage) :
						(PLAYER_ACTION_STATE::SKILL == player.eAction &&
							player.iComboStage == stageIndex + 2u));
			}
		}
		const bool pendingStarted =
			skills.Try_StartPending(
				player, latestExplicit, catalog, pendingBoundaryTick++);
		tests.Require(
			explicitWaitedForAutomaticChain && pendingStarted &&
			PLAYER_ACTION_STATE::SKILL == player.eAction &&
			2050120u == player.iCurrentSkillId &&
			PLAYER_PENDING_COMMAND_KIND::NONE == player.PendingCommand.eKind &&
			nullptr != pendingSkillDefinition &&
			resourceBeforePending - pendingSkillDefinition->iResourceCost ==
				player.iCurrentResource &&
			player.CooldownEndTickBySkillId.contains(2050120u),
			"Commit the latest explicit skill after the full automatic BA chain and spend costs once");

		C2S_MOVE pendingMove{};
		pendingMove.iClientSequence = 1u;
		pendingMove.fGoalX = 3.f;
		pendingMove.fGoalZ = 4.f;
		player.PendingCommand.Set_Move(pendingMove);
		CPlayerSkillSystem::Arm_PlayerHitReaction(
			player, 0.f, 0.f, 1.f, 100u, false, 0u, 1000u);
		tests.Require(
			PLAYER_PENDING_COMMAND_KIND::NONE == player.PendingCommand.eKind,
			"Clear pending explicit command on forced movement");
	}
	{
		CGameRoom room{ WORLD_ID::CHARACTER_SELECT_ARENA };
		const WORLD_BOOTSTRAP_PLACEMENT* spawn =
			room.Is_Ready() ? room.Find_AvailablePlayerSpawn() : nullptr;
		bool stagedAndCommittedMove = false;
		bool failedPendingSkillWasIsolated = false;
		if (nullptr != spawn)
		{
			constexpr SESSION_ID sessionId = 88001u;
			SERVER_PLAYER player{};
			player.iSessionId = sessionId;
			player.iPlayerId = 88002u;
			player.iNetEntityId = 88003u;
			player.eCharacterClass = CHARACTER_CLASS_ID::DIMENSIONMASTER;
			player.iCurrentHp = 1000u;
			player.iMaximumHp = 1000u;
			player.iCurrentResource = 1000u;
			player.iMaximumResource = 1000u;
			player.fPositionX = spawn->fPositionX;
			player.fPositionY = spawn->fPositionY;
			player.fPositionZ = spawn->fPositionZ;
			room.m_Players.emplace(player.iPlayerId, player);
			room.m_PlayerIdBySessionId.emplace(sessionId, player.iPlayerId);
			SERVER_PLAYER& live = room.m_Players.at(player.iPlayerId);

			C2S_USE_SKILL basicAttack{};
			basicAttack.iClientSequence = 1u;
			basicAttack.iSkillId = 2050010u;
			basicAttack.fAimX = live.fPositionX + 1.f;
			basicAttack.fAimZ = live.fPositionZ;
			const bool roomAttackStarted = room.m_PlayerSkillSystem.Try_Start(
				live, basicAttack, room.m_GameplayCatalog, 300u);
			const PLAYER_SKILL_DEFINITION* roomBasicAttackDefinition =
				room.m_GameplayCatalog.Find_Skill(2050010u);
			const std::size_t cooldownCountBeforePending =
				live.CooldownEndTickBySkillId.size();
			C2S_MOVE move{};
			move.iClientSequence = 1u;
			move.fGoalX = spawn->fPositionX;
			move.fGoalZ = spawn->fPositionZ;
			room.Handle_Move(sessionId, move);
			C2S_USE_SKILL pendingSkill{};
			pendingSkill.iClientSequence = 3u;
			pendingSkill.iSkillId = 2050100u;
			pendingSkill.fAimX = live.fPositionX + 2.f;
			pendingSkill.fAimZ = live.fPositionZ;
			room.Handle_UseSkill(sessionId, pendingSkill);
			move.iClientSequence = 2u;
			room.Handle_Move(sessionId, move);
			const bool latestMoveReplacedSkill =
				PLAYER_PENDING_COMMAND_KIND::MOVE == live.PendingCommand.eKind &&
				2u == live.PendingCommand.iClientSequence &&
				2u == live.iLastMoveSequence &&
				1000u == live.iCurrentResource &&
				cooldownCountBeforePending ==
					live.CooldownEndTickBySkillId.size() &&
				!live.CooldownEndTickBySkillId.contains(2050100u);
			std::vector<SERVER_WORLD_ENTITY> noTargets;
			std::vector<DAMAGE_EVENT> noDamageEvents;
			bool moveWaitedForAutomaticChain =
				nullptr != roomBasicAttackDefinition &&
					!roomBasicAttackDefinition->ComboStages.empty();
			std::uint32_t roomBoundaryTick = 302u;
			if (moveWaitedForAutomaticChain)
			{
				for (std::size_t stageIndex = 0u;
					stageIndex < roomBasicAttackDefinition->ComboStages.size();
					++stageIndex)
				{
					const PLAYER_COMBO_STAGE& stage =
						roomBasicAttackDefinition->ComboStages[stageIndex];
					live.fActionElapsedSeconds =
						static_cast<float>(stage.iActionDurationMs - 1u) * 0.001f;
					room.m_PlayerSkillSystem.Update(
						live, noTargets, room.m_GameplayCatalog, nullptr, nullptr,
						0.002f, roomBoundaryTick++, noDamageEvents);
					const bool isFinalStage = stageIndex + 1u ==
						roomBasicAttackDefinition->ComboStages.size();
					moveWaitedForAutomaticChain =
						moveWaitedForAutomaticChain &&
						PLAYER_PENDING_COMMAND_KIND::MOVE ==
							live.PendingCommand.eKind &&
						(isFinalStage ?
							(PLAYER_ACTION_STATE::NONE == live.eAction &&
								0u == live.iComboStage) :
							(PLAYER_ACTION_STATE::SKILL == live.eAction &&
								live.iComboStage == stageIndex + 2u));
				}
			}
			room.Commit_PendingPlayerCommand(live, roomBoundaryTick++);
			stagedAndCommittedMove = roomAttackStarted &&
				latestMoveReplacedSkill && moveWaitedForAutomaticChain &&
				PLAYER_PENDING_COMMAND_KIND::NONE == live.PendingCommand.eKind &&
				PLAYER_ACTION_STATE::NONE == live.eAction &&
				0u == live.iComboStage && live.hasMoveGoal;

			SERVER_PLAYER invalidPending = live;
			invalidPending.hasMoveGoal = false;
			invalidPending.MovePath.clear();
			invalidPending.iCurrentResource = 0u;
			invalidPending.PendingCommand.Set_Skill(pendingSkill);
			room.Commit_PendingPlayerCommand(invalidPending, 303u);
			failedPendingSkillWasIsolated =
				PLAYER_PENDING_COMMAND_KIND::NONE ==
					invalidPending.PendingCommand.eKind &&
				PLAYER_ACTION_STATE::NONE == invalidPending.eAction &&
				0u == invalidPending.iCurrentResource;
		}
		tests.Require(stagedAndCommittedMove,
			"Keep latest MOVE through automatic BA and commit it from the final boundary position");
		tests.Require(failedPendingSkillWasIsolated,
			"Discard only a pending skill that fails boundary revalidation");
	}
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::LANCE_MASTER),
		"Resolve LanceMaster player profile");
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::GUNSLINGER),
		"Resolve Gunslinger player profile");
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::SLAYER),
		"Resolve Slayer player profile");
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::ARTIST),
		"Resolve Artist player profile");
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::DIMENSIONMASTER),
		"Resolve DimensionMaster player profile");
	tests.Require(nullptr == catalog.Find_Player(CHARACTER_CLASS_ID::DESTROYER),
		"Reject unsupported Destroyer player profile");
	tests.Require(361u == catalog.Find_DamageRatePercent("damage.player.34120"),
		"Resolve player damage rate");
	tests.Require(361u == CGameplayCatalog::Resolve_Damage(100u, 361u),
		"Resolve damage as attack power times rate");
	tests.Require(100u == CGameplayCatalog::Resolve_Damage(100u, 100u),
		"Resolve basic attack rate as exactly one attack power");
	tests.Require(0u == CGameplayCatalog::Resolve_Damage(0u, 361u),
		"Resolve zero attack power as no damage");
	tests.Require(1u == CGameplayCatalog::Resolve_Damage(1u, 1u),
		"Clamp a connected hit to at least one damage");
	tests.Require(170u == CGameplayCatalog::Apply_Defense(350u, 105u),
		"Apply the documented project defense curve");
	tests.Require(1u == CGameplayCatalog::Apply_Defense(1u, 100000u),
		"Clamp a mitigated connected hit to at least one damage");

	CServerNavigation navigation;
	CWorldBootstrap world;
	tests.Require(world.Load(WORLD_ID::VALTAN_ARENA) &&
		world.Get_AreaId() == "LV_LUT_HEARTRB_ED",
		"Preserve world area ID across placement parsing");
	tests.Require(
		4u == static_cast<std::size_t>(std::count_if(
			world.Get_Placements().begin(),
			world.Get_Placements().end(),
			[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
					placement.isEnabled;
			})),
		"Load exactly four enabled Valtan player spawns");
	tests.Require(navigation.Load("LV_LUT_HEARTRB_ED"),
		"Load Valtan server navigation");
	std::vector<SERVER_NAV_POINT> path;
	/* Both endpoints are open arena floor on the same connected component. The
	   old start sat outside the completed 109 ring, which only pathed while the
	   ring still had its six-slab gap; the arena interior is now sealed until
	   the 109 collapse, so the test walks a route that stays inside it. */
	tests.Require(navigation.Find_Path(147.75f, -117.25f, 156.25f, -122.25f, path) &&
		!path.empty(), "Find authoritative navigation path");
	SERVER_NAV_POINT rejected{};
	tests.Require(!navigation.Project_Point(10000.f, 10000.f, rejected),
		"Reject navigation point outside projection radius");

	/* Walk out from a point the path test already proved walkable until the grid
	stops answering, so the boundary is whatever the current bake says it is
	rather than a cell index frozen into this file. */
	const float navCellSize = navigation.Get_CellSize();
	const float boundaryProbeStep = navCellSize * 0.25f;
	const float boundaryProbeZ = -137.f;
	const float boundaryProbeOriginX = 152.f;
	const float boundaryProbeLimitX = boundaryProbeOriginX + 60.f;
	float lastWalkableX = boundaryProbeOriginX;
	float firstBlockedX = 0.f;
	bool foundBoundary = false;
	SERVER_NAV_POINT boundaryProbe{};
	for (float probeX = boundaryProbeOriginX + boundaryProbeStep;
		probeX < boundaryProbeLimitX;
		probeX += boundaryProbeStep)
	{
		if (!navigation.Sample_Position(probeX, boundaryProbeZ, boundaryProbe))
		{
			firstBlockedX = probeX;
			foundBoundary = true;
			break;
		}
		lastWalkableX = probeX;
	}
	tests.Require(
		navCellSize > 0.f && foundBoundary,
		"Find a Valtan navigation boundary to clamp root motion against");

	SERVER_NAV_POINT rootMotionStop{ 0.f, 0.f, 0.f };
	bool rootMotionClamped = false;
	CPlayerSkillSystem::Clamp_StepToWalkable(
		navigation,
		lastWalkableX,
		boundaryProbeZ,
		firstBlockedX + navCellSize,
		boundaryProbeZ,
		rootMotionStop,
		rootMotionClamped);
	tests.Require(
		rootMotionClamped &&
		rootMotionStop.x >= lastWalkableX &&
		rootMotionStop.x < firstBlockedX &&
		firstBlockedX - rootMotionStop.x < navCellSize &&
		std::abs(rootMotionStop.z - boundaryProbeZ) < 0.001f,
		"Stop root motion against the Valtan non-walkable boundary");

	SERVER_NAV_POINT rootMotionOpen{ 0.f, 0.f, 0.f };
	bool openClamped = true;
	CPlayerSkillSystem::Clamp_StepToWalkable(
		navigation,
		152.f,
		boundaryProbeZ,
		152.f + boundaryProbeStep,
		boundaryProbeZ,
		rootMotionOpen,
		openClamped);
	tests.Require(
		!openClamped &&
		std::abs(rootMotionOpen.x - (152.f + boundaryProbeStep)) < 0.001f,
		"Preserve root motion that stays on walkable navigation");

	/* Keep marching past the blocked band to the open floor behind it. A step
	that spans the whole band is the case a plain bisection would wave through,
	so the clamp has to answer with the near wall, not the far side. */
	float farSideX = 0.f;
	bool foundFarSide = false;
	for (float probeX = firstBlockedX + boundaryProbeStep;
		probeX < firstBlockedX + 60.f;
		probeX += boundaryProbeStep)
	{
		if (navigation.Sample_Position(probeX, boundaryProbeZ, boundaryProbe))
		{
			farSideX = probeX;
			foundFarSide = true;
			break;
		}
	}
	tests.Require(foundFarSide,
		"Find walkable navigation beyond the blocked band");
	SERVER_NAV_POINT rootMotionTunnel{ 0.f, 0.f, 0.f };
	bool tunnelClamped = false;
	CPlayerSkillSystem::Clamp_StepToWalkable(
		navigation,
		lastWalkableX,
		boundaryProbeZ,
		farSideX,
		boundaryProbeZ,
		rootMotionTunnel,
		tunnelClamped);
	tests.Require(
		tunnelClamped &&
		rootMotionTunnel.x >= lastWalkableX &&
		rootMotionTunnel.x < firstBlockedX,
		"Refuse root motion that would cross a blocked band in one step");

	constexpr const char* VALTAN_WALL_RECEIVER =
		"collision.valtan.wallgroup.11047903315509031966.15719065619666776634.receiver";
	constexpr const char* VALTAN_WALL_COLLISION_STATE =
		"collision.valtan.wallgroup.11047903315509031966.15719065619666776634";
	constexpr const char* VALTAN_WALL_CONDITION =
		"condition.valtan.wall159.15719065619666776634.destroyed";
	constexpr float VALTAN_WALL_CENTER_X = 161.402061f;
	constexpr float VALTAN_WALL_CENTER_Y = 23.04f;
	constexpr float VALTAN_WALL_CENTER_Z = -133.312236f;
	/* The approach used to stand outside the ring and reach the charge wall
	through the six-slab gap. The completed 109 ring seals that gap until the
	collapse, so the sweep now starts inside the arena: it still crosses the
	charge wall's boxes and touches no ring slab, which keeps this test about
	the 159 wall's own collision state instead of the ring's. */
	constexpr float VALTAN_WALL_APPROACH_X = 164.25f;
	constexpr float VALTAN_WALL_APPROACH_Z = -125.25f;
	constexpr float VALTAN_WALL_EXIT_X = 159.488644f;
	constexpr float VALTAN_WALL_EXIT_Z = -128.692839f;
	std::string dynamicWorldStatus;
	SERVER_NAVIGATION_CONDITION_STAGE navigationStage{};
	std::vector<SERVER_NAV_POINT> wallPassagePath;
	tests.Require(
		navigation.Has_Condition(VALTAN_WALL_CONDITION) &&
		!navigation.Is_PointWalkableExact(
			VALTAN_WALL_CENTER_X, VALTAN_WALL_CENTER_Z) &&
		!navigation.Find_Path(
			160.25f, -130.75f, 162.25f, -135.75f,
			wallPassagePath),
		"Keep the intact Valtan wall footprint and its cross-wall path dynamically blocked");
	const std::uint64_t navigationRevisionBeforeReject =
		navigation.Get_Revision();
	tests.Require(
		!navigation.Prepare_ConditionChanges(
			{ { "condition.valtan.wall.unknown", true } },
			navigationStage, dynamicWorldStatus) &&
		navigationRevisionBeforeReject == navigation.Get_Revision() &&
		!navigation.Is_PointWalkableExact(
			VALTAN_WALL_CENTER_X, VALTAN_WALL_CENTER_Z),
		"Reject an unknown navigation condition without changing the live blocker");
	tests.Require(
		navigation.Prepare_ConditionChanges(
			{ { VALTAN_WALL_CONDITION, true } },
			navigationStage, dynamicWorldStatus) &&
		navigationStage.bChanged &&
		navigationStage.iNextRevision == navigation.Get_Revision() + 1u,
		"Stage one runtime navigation condition without mutating the live grid");
	navigation.Commit_ConditionChanges(std::move(navigationStage));
	tests.Require(
		navigation.Is_PointWalkableExact(
			VALTAN_WALL_CENTER_X, VALTAN_WALL_CENTER_Z) &&
		!navigation.Find_Path(
			160.25f, -130.75f, 162.25f, -135.75f,
			wallPassagePath),
		"Expose only the selected wall cells while adjacent independent walls keep the full barrier closed");
	navigation.Reset_RuntimeBlockers();
	tests.Require(
		!navigation.Is_PointWalkableExact(
			VALTAN_WALL_CENTER_X, VALTAN_WALL_CENTER_Z) &&
		!navigation.Find_Path(
			160.25f, -130.75f, 162.25f, -135.75f,
			wallPassagePath),
		"Restore the intact Valtan wall blocker and closed path on encounter reset");

	CServerCollisionSystem valtanCollisionSystem;
	tests.Require(
		valtanCollisionSystem.Initialize(
			world.Get_Placements(), dynamicWorldStatus) &&
		/* 69 interior wall boxes plus ten independent 159 impact receivers,
		one box per 109 outer ring slab and one impact receiver twinning each
		of those thirty, and the two entrance front walls' own receivers. */
		141u == valtanCollisionSystem.Get_CollisionBoxCount() &&
		valtanCollisionSystem.Has_CollisionBox(VALTAN_WALL_RECEIVER),
		"Load the stable Valtan wall impact receiver and player blocker");
	{
		WORLD_BOOTSTRAP_PLACEMENT wall{};
		wall.strPlacementId = "collision.contract.axe.wall";
		wall.eKind = WORLD_BOOTSTRAP_KIND::COLLISION_BOX;
		wall.isEnabled = true;
		wall.fPositionX = 0.f;
		wall.fPositionY = 2.f;
		wall.fPositionZ = 5.f;
		wall.fYawDegrees = 30.f;
		wall.fHalfExtentX = 1.f;
		wall.fHalfExtentY = 2.f;
		wall.fHalfExtentZ = 0.5f;
		WORLD_BOOTSTRAP_PLACEMENT receiver = wall;
		receiver.strPlacementId = "collision.contract.axe.wall.receiver";
		CServerCollisionSystem axeCollision;
		std::vector<std::string> axeContacts;
		std::string axeStatus;
		const bool axeInitialized = axeCollision.Initialize(
			{ wall, receiver }, axeStatus);
		axeCollision.Collect_BossPatternHitContacts(
			BOSS_PATTERN_HIT_SHAPE::CONE,
			0.f, 0.f, 0.f, 0.f, 1.5f,
			0.f, 0.f, 80.f, 7.f, 0.f, axeContacts);
		const bool coneHit = 1u == axeContacts.size() &&
			axeContacts.front() == wall.strPlacementId;
		axeCollision.Collect_BossPatternHitContacts(
			BOSS_PATTERN_HIT_SHAPE::CONE,
			0.f, 0.f, 0.f, 180.f, 1.5f,
			0.f, 0.f, 80.f, 7.f, 0.f, axeContacts);
		const bool rearMiss = axeContacts.empty();
		axeCollision.Collect_BossPatternHitContacts(
			BOSS_PATTERN_HIT_SHAPE::CONE,
			0.f, 20.f, 0.f, 0.f, 1.5f,
			0.f, 0.f, 80.f, 7.f, 0.f, axeContacts);
		const bool highMiss = axeContacts.empty();
		tests.Require(
			axeInitialized && coneHit && rearMiss && highMiss,
			"Intersect a rotated wall with the Server axe cone while rejecting receiver, rear and high-Y false contacts");
	}
	{
		WORLD_BOOTSTRAP_PLACEMENT armWall{};
		armWall.strPlacementId = "collision.contract.six-directions.arm";
		armWall.eKind = WORLD_BOOTSTRAP_KIND::COLLISION_BOX;
		armWall.isEnabled = true;
		armWall.fPositionX = 4.330127f;
		armWall.fPositionY = 2.f;
		armWall.fPositionZ = 2.5f;
		armWall.fHalfExtentX = 0.2f;
		armWall.fHalfExtentY = 0.5f;
		armWall.fHalfExtentZ = 0.2f;
		WORLD_BOOTSTRAP_PLACEMENT gapWall = armWall;
		gapWall.strPlacementId = "collision.contract.six-directions.gap";
		gapWall.fPositionX = 2.5f;
		gapWall.fPositionZ = 4.330127f;
		CServerCollisionSystem sixDirectionCollision;
		std::vector<std::string> sixDirectionContacts;
		std::string sixDirectionStatus;
		const bool sixDirectionInitialized = sixDirectionCollision.Initialize(
			{ armWall, gapWall }, sixDirectionStatus);
		sixDirectionCollision.Collect_BossPatternHitContacts(
			BOSS_PATTERN_HIT_SHAPE::SIX_DIRECTIONS,
			0.f, 0.f, 0.f, 0.f, 1.5f,
			0.f, 0.f, 0.f, 6.f, 0.5f, sixDirectionContacts);
		tests.Require(
			sixDirectionInitialized && 1u == sixDirectionContacts.size() &&
			sixDirectionContacts.front() == armWall.strPlacementId,
			"Project six-direction Server contacts onto an arm without filling its adjacent gap");
	}
	{
		WORLD_BOOTSTRAP_PLACEMENT wall{};
		wall.strPlacementId = "collision.contract.charge-wall";
		wall.eKind = WORLD_BOOTSTRAP_KIND::COLLISION_BOX;
		wall.isEnabled = true;
		wall.fPositionY = 1.f;
		wall.fPositionZ = 5.f;
		wall.fHalfExtentX = 2.f;
		wall.fHalfExtentY = 1.f;
		wall.fHalfExtentZ = 0.5f;
		WORLD_BOOTSTRAP_PLACEMENT receiver = wall;
		receiver.strPlacementId = wall.strPlacementId + ".receiver";
		CServerCollisionSystem chargeWallCollision;
		std::string chargeWallStatus;
		SERVER_BOSS_WALL_HIT wallHit{};
		const bool chargeWallsInitialized = chargeWallCollision.Initialize(
			{ receiver, wall }, chargeWallStatus);
		tests.Require(
			chargeWallsInitialized &&
			chargeWallCollision.Sweep_BossCircleAgainstWalls(
				0.f, 0.f, 0.f, 0.f, 0.f, 10.f, 1.f, wallHit) &&
			wall.strPlacementId == wallHit.strCollisionPlacementId &&
			receiver.strPlacementId ==
				wallHit.strImpactReceiverPlacementId &&
			wallHit.fHitRatio > 0.f && wallHit.fHitRatio < 1.f,
			"Keep the base wall and co-located active receiver on one deterministic boss sweep");
		SERVER_COLLISION_STATE_STAGE receiverDisabled{};
		const bool stagedReceiverDisabled =
			chargeWallCollision.Prepare_StateChanges(
				{ { receiver.strPlacementId, true, false } },
				receiverDisabled, chargeWallStatus);
		if (stagedReceiverDisabled)
			chargeWallCollision.Commit_StateChanges(std::move(receiverDisabled));
		tests.Require(
			stagedReceiverDisabled &&
			chargeWallCollision.Sweep_BossCircleAgainstWalls(
				0.f, 0.f, 0.f, 0.f, 0.f, 10.f, 1.f, wallHit) &&
			wall.strPlacementId == wallHit.strCollisionPlacementId &&
			wallHit.strImpactReceiverPlacementId.empty(),
			"Keep an ordinary blocking wall eligible after its impact receiver is disabled");
	}
	SERVER_BOSS_RECEIVER_HIT receiverHit{};
	/* Both sweeps start clear of the 109 outer ring. Every slab of it now
	carries a receiver, and a sweep begun inside one resolves at ratio zero,
	which would stop exercising the travel this contract is named for. */
	tests.Require(
		valtanCollisionSystem.Sweep_BossCircleAgainstReceivers(
			148.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
			175.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
			1.f, receiverHit) &&
		0u == receiverHit.strReceiverPlacementId.rfind(
			"collision.valtan.wallgroup.11047903315509031966.", 0u) &&
		receiverHit.fHitRatio > 0.f && receiverHit.fHitRatio < 1.f &&
		!valtanCollisionSystem.Sweep_BossCircleAgainstReceivers(
			148.f, 100.f, VALTAN_WALL_CENTER_Z,
			175.f, 100.f, VALTAN_WALL_CENTER_Z,
			1.f, receiverHit),
		"Sweep a fast Valtan body into the deterministic earliest independent receiver without tunneling or high-Y false hits");
	SERVER_PLAYER valtanWallPlayer{};
	valtanWallPlayer.fPositionX = VALTAN_WALL_APPROACH_X;
	valtanWallPlayer.fPositionY = VALTAN_WALL_CENTER_Y;
	valtanWallPlayer.fPositionZ = VALTAN_WALL_APPROACH_Z;
	float wallResolvedX = 0.f;
	float wallResolvedY = 0.f;
	float wallResolvedZ = 0.f;
	bool wallMoveBlocked = false;
	tests.Require(
		valtanCollisionSystem.Resolve_PlayerMove(
			valtanWallPlayer, VALTAN_WALL_EXIT_X, VALTAN_WALL_CENTER_Y,
			VALTAN_WALL_EXIT_Z, wallResolvedX, wallResolvedY,
			wallResolvedZ, wallMoveBlocked) && wallMoveBlocked,
		"Block player movement through the intact Valtan wall receiver");
	SERVER_COLLISION_STATE_STAGE collisionStage{};
	const std::uint64_t collisionRevisionBeforeReject =
		valtanCollisionSystem.Get_Revision();
	tests.Require(
		!valtanCollisionSystem.Prepare_StateChanges(
			{ { "receiver.valtan.wall.unknown", false, false } },
			collisionStage, dynamicWorldStatus) &&
		collisionRevisionBeforeReject ==
			valtanCollisionSystem.Get_Revision(),
		"Reject an unknown collision state target without mutating the receiver");
	tests.Require(
		valtanCollisionSystem.Prepare_StateChanges(
			{ { VALTAN_WALL_COLLISION_STATE, true, false } },
			collisionStage, dynamicWorldStatus),
		"Stage BREAKING collision channels with receiver impact disabled");
	valtanCollisionSystem.Commit_StateChanges(std::move(collisionStage));
	SERVER_BOSS_RECEIVER_HIT remainingReceiverHit{};
	tests.Require(
		(!valtanCollisionSystem.Sweep_BossCircleAgainstReceivers(
			145.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
			175.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
			1.f, remainingReceiverHit) ||
		 remainingReceiverHit.strReceiverPlacementId != VALTAN_WALL_RECEIVER) &&
		!valtanCollisionSystem.Is_ImpactReceiverEnabled(VALTAN_WALL_RECEIVER) &&
		valtanCollisionSystem.Is_PlayerBlocking(VALTAN_WALL_COLLISION_STATE) &&
		valtanCollisionSystem.Resolve_PlayerMove(
			valtanWallPlayer, VALTAN_WALL_EXIT_X, VALTAN_WALL_CENTER_Y,
			VALTAN_WALL_EXIT_Z, wallResolvedX, wallResolvedY,
			wallResolvedZ, wallMoveBlocked) && wallMoveBlocked,
		"Keep the selected BREAKING wall blocking while suppressing only its receiver");
	tests.Require(
		valtanCollisionSystem.Prepare_StateChanges(
			{ { VALTAN_WALL_COLLISION_STATE, false, false } },
			collisionStage, dynamicWorldStatus),
		"Stage the persistent FRACTURED collision state");
	valtanCollisionSystem.Commit_StateChanges(std::move(collisionStage));
	tests.Require(
		!valtanCollisionSystem.Is_PlayerBlocking(VALTAN_WALL_COLLISION_STATE) &&
		!valtanCollisionSystem.Is_PlayerBlocking(VALTAN_WALL_RECEIVER) &&
		valtanCollisionSystem.Resolve_PlayerMove(
			valtanWallPlayer, VALTAN_WALL_EXIT_X, VALTAN_WALL_CENTER_Y,
			VALTAN_WALL_EXIT_Z, wallResolvedX, wallResolvedY,
			wallResolvedZ, wallMoveBlocked) && wallMoveBlocked,
		"Disable only the selected wall collision while adjacent independent walls remain solid");
	valtanCollisionSystem.Reset_RuntimeStates();
	tests.Require(
		valtanCollisionSystem.Sweep_BossCircleAgainstReceivers(
			145.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
			175.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
			1.f, receiverHit),
		"Restore the Valtan impact receiver when the room resets");
	SERVER_WORLD_ENTITY impactMotionBoss{};
	impactMotionBoss.strEncounterId = "ENCOUNTER_VALTAN";
	impactMotionBoss.strPatternId = "VALTAN_ARMOR_BREAK_OPENING";
	impactMotionBoss.strPatternStageId = "WALL_CHARGE";
	impactMotionBoss.strActionId =
		"valtan.mechanic.armor-break-opening.charge";
	impactMotionBoss.eAction = SERVER_ENTITY_ACTION::PATTERN_WINDUP;
	/* The authored stage, not the pattern name, is what makes a stage charge.
	Enter_PatternStage copies this out of the catalog in production. */
	impactMotionBoss.bPatternChargeImpact = true;
	impactMotionBoss.fPositionX = 150.f;
	impactMotionBoss.fPositionZ = -133.f;
	impactMotionBoss.fYawDegrees = 90.f;
	impactMotionBoss.fPatternForcedMotionSpeed = 30.f;
	float impactProposedX = 0.f;
	float impactProposedZ = 0.f;
	CValtanBrain impactMotionBrain;
	tests.Require(
		impactMotionBrain.Try_BuildImpactMotion(
			impactMotionBoss, 1.f / 30.f,
			impactProposedX, impactProposedZ) &&
		std::abs(impactProposedX - 151.f) <= 0.001f &&
		std::abs(impactProposedZ + 133.f) <= 0.001f,
		"Advance the opening charge from Server-authored fixed-tick motion");
	tests.Require(
		impactMotionBrain.Complete_ImpactStage(
			impactMotionBoss, catalog, 500u) &&
		impactMotionBoss.strPatternStageId == "GROGGY" &&
		impactMotionBoss.bPatternGroggy &&
		!impactMotionBoss.bPatternChargeImpact &&
		0.f == impactMotionBoss.fPatternForcedMotionSpeed,
		"Advance the authoritative charge action to GROGGY only after impact");
	SERVER_WORLD_ENTITY plainStageBoss = impactMotionBoss;
	plainStageBoss.strPatternStageId = "WALL_CHARGE";
	plainStageBoss.iPatternStageIndex = 0u;
	plainStageBoss.bPatternChargeImpact = false;
	plainStageBoss.fPatternForcedMotionSpeed = 30.f;
	float plainProposedX = 0.f;
	float plainProposedZ = 0.f;
	tests.Require(
		!impactMotionBrain.Try_BuildImpactMotion(
			plainStageBoss, 1.f / 30.f, plainProposedX, plainProposedZ) &&
		!impactMotionBrain.Complete_ImpactStage(
			plainStageBoss, catalog, 500u) &&
		plainStageBoss.strPatternStageId == "WALL_CHARGE",
		"Leave a stage the encounter never authored as a charge without impact motion");

	CWorldBootstrap bernWorld;
	const bool bernLoaded = bernWorld.Load(WORLD_ID::BERN);
	const auto& bernPlacements = bernWorld.Get_Placements();
	const auto bernNpc = std::find_if(
		bernPlacements.begin(), bernPlacements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId == "npc.bern.beda.guide";
		});
	const auto bernAylara = std::find_if(
		bernPlacements.begin(), bernPlacements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId == "npc.bern.aylara";
		});
	const auto bernSchmidt = std::find_if(
		bernPlacements.begin(), bernPlacements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId == "npc.bern.schmidt";
		});
	const auto bernPlayerEntrySpawn = std::find_if(
		bernPlacements.begin(), bernPlacements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId == "player_1";
		});
	const auto bernLeftGuardPatrol = std::find_if(
		bernPlacements.begin(), bernPlacements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId == "npc.bern.25287";
		});
	const auto bernRightGuardPatrol = std::find_if(
		bernPlacements.begin(), bernPlacements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId == "npc.bern.25287.2";
		});
	std::set<std::string> bernPlazaArchetypeIds;
	std::set<std::string> bernPlazaActionIds;
	std::array<std::size_t, 4u> bernPlazaClusterCounts{};
	std::size_t bernPlazaNpcCount = 0u;
	std::size_t bernPlazaLookTargetCount = 0u;
	bool bernPlazaNpcContractValid = true;
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement : bernPlacements)
	{
		if (0u != placement.strPlacementId.rfind("npc.bern.plaza.", 0u))
			continue;
		++bernPlazaNpcCount;
		bool belongsToAuthoredCluster = false;
		if (placement.fPositionX >= 130.5f &&
			placement.fPositionX <= 134.5f &&
			placement.fPositionZ >= -86.f &&
			placement.fPositionZ <= -81.5f)
		{
			++bernPlazaClusterCounts[0u];
			belongsToAuthoredCluster = true;
		}
		else if (placement.fPositionX >= 138.f &&
			placement.fPositionX <= 141.f &&
			placement.fPositionZ >= -86.5f &&
			placement.fPositionZ <= -83.4f)
		{
			++bernPlazaClusterCounts[1u];
			belongsToAuthoredCluster = true;
		}
		else if (placement.fPositionX >= 142.f &&
			placement.fPositionX <= 144.5f &&
			placement.fPositionZ >= -82.5f &&
			placement.fPositionZ <= -76.5f)
		{
			++bernPlazaClusterCounts[2u];
			belongsToAuthoredCluster = true;
		}
		else if (placement.fPositionX >= 131.5f &&
			placement.fPositionX <= 134.5f &&
			placement.fPositionZ >= -75.5f &&
			placement.fPositionZ <= -73.4f)
		{
			++bernPlazaClusterCounts[3u];
			belongsToAuthoredCluster = true;
		}
		bernPlazaNpcContractValid = bernPlazaNpcContractValid &&
			placement.isEnabled &&
			WORLD_BOOTSTRAP_KIND::NPC == placement.eKind &&
			belongsToAuthoredCluster &&
			placement.bHasNpcBehavior &&
			NPC_BEHAVIOR_MODE::STATIONARY == placement.NpcBehavior.eMode &&
			placement.NpcBehavior.Waypoints.empty() &&
			0.f == placement.NpcBehavior.fWanderRadius &&
			1u == placement.NpcBehavior.Actions.size() &&
			600000u == placement.NpcBehavior.Actions.front().iDurationMs &&
			0u == placement.NpcBehavior.Actions.front().iWaitAfterMs;
		bernPlazaArchetypeIds.insert(placement.strArchetypeId);
		if (!placement.NpcBehavior.Actions.empty())
		{
			bernPlazaActionIds.insert(
				placement.NpcBehavior.Actions.front().strActionId);
		}
		if (!placement.NpcBehavior.strLookTargetPlacementId.empty())
			++bernPlazaLookTargetCount;
	}
	bool bernGuardPatrolsHeadToPlayerEntry = false;
	if (bernLeftGuardPatrol != bernPlacements.end() &&
		bernRightGuardPatrol != bernPlacements.end() &&
		bernPlayerEntrySpawn != bernPlacements.end() &&
		bernLeftGuardPatrol->bHasNpcBehavior &&
		bernRightGuardPatrol->bHasNpcBehavior &&
		2u == bernLeftGuardPatrol->NpcBehavior.Waypoints.size() &&
		2u == bernRightGuardPatrol->NpcBehavior.Waypoints.size())
	{
		const auto& leftWaypoints =
			bernLeftGuardPatrol->NpcBehavior.Waypoints;
		const auto& rightWaypoints =
			bernRightGuardPatrol->NpcBehavior.Waypoints;
		const float leftLaneX = leftWaypoints[0u].fPositionX;
		const float rightLaneX = rightWaypoints[0u].fPositionX;
		const float corridorMinZ = (std::min)(
			leftWaypoints[0u].fPositionZ,
			leftWaypoints[1u].fPositionZ);
		const float corridorMaxZ = (std::max)(
			leftWaypoints[0u].fPositionZ,
			leftWaypoints[1u].fPositionZ);
		constexpr float requiredNpcLaneClearance =
			LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_X * 2.f + 0.05f;
		const bool routesAreParallelAndSeparated =
			std::abs(leftWaypoints[0u].fPositionX -
				leftWaypoints[1u].fPositionX) < 0.001f &&
			std::abs(rightWaypoints[0u].fPositionX -
				rightWaypoints[1u].fPositionX) < 0.001f &&
			std::abs(leftWaypoints[0u].fPositionZ -
				rightWaypoints[0u].fPositionZ) < 0.001f &&
			std::abs(leftWaypoints[1u].fPositionZ -
				rightWaypoints[1u].fPositionZ) < 0.001f &&
			std::abs(rightLaneX - leftLaneX) >= requiredNpcLaneClearance &&
			std::abs(leftWaypoints[1u].fPositionZ -
				leftWaypoints[0u].fPositionZ) >= 30.f;
		const auto distanceSquaredFromPlayerEntry =
			[&bernPlayerEntrySpawn](
				const WORLD_NPC_BEHAVIOR_WAYPOINT& waypoint)
			{
				const float dx = waypoint.fPositionX -
					bernPlayerEntrySpawn->fPositionX;
				const float dz = waypoint.fPositionZ -
					bernPlayerEntrySpawn->fPositionZ;
				return dx * dx + dz * dz;
			};
		const bool routesHeadTowardPlayerEntry =
			leftWaypoints[1u].fPositionZ > leftWaypoints[0u].fPositionZ &&
			rightWaypoints[1u].fPositionZ > rightWaypoints[0u].fPositionZ &&
			distanceSquaredFromPlayerEntry(leftWaypoints[1u]) <
				distanceSquaredFromPlayerEntry(leftWaypoints[0u]) &&
			distanceSquaredFromPlayerEntry(rightWaypoints[1u]) <
				distanceSquaredFromPlayerEntry(rightWaypoints[0u]);
		bernGuardPatrolsHeadToPlayerEntry =
			routesAreParallelAndSeparated && routesHeadTowardPlayerEntry &&
			std::all_of(
				bernPlacements.begin(), bernPlacements.end(),
				[leftLaneX, rightLaneX, corridorMinZ, corridorMaxZ,
				 requiredNpcLaneClearance](
					const WORLD_BOOTSTRAP_PLACEMENT& placement)
				{
					if (WORLD_BOOTSTRAP_KIND::NPC != placement.eKind ||
						!placement.isEnabled ||
						placement.strPlacementId == "npc.bern.25287" ||
						placement.strPlacementId == "npc.bern.25287.2" ||
						placement.fPositionZ < corridorMinZ ||
						placement.fPositionZ > corridorMaxZ)
					{
						return true;
					}
					return (std::min)(
						std::abs(placement.fPositionX - rightLaneX),
						std::abs(placement.fPositionX - leftLaneX)) >=
						requiredNpcLaneClearance;
				});
	}
	const auto bernValtanTrigger = std::find_if(
		bernPlacements.begin(), bernPlacements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId == "valtan";
		});
	const auto bernCollision = std::find_if(
		bernPlacements.begin(), bernPlacements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId ==
				"collision.bern.editor-proof";
		});
	tests.Require(
		bernLoaded && bernPlacements.size() == 36u &&
		4u == static_cast<size_t>(std::count_if(
			bernPlacements.begin(), bernPlacements.end(),
			[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
					placement.isEnabled;
			})) &&
		30u == static_cast<size_t>(std::count_if(
			bernPlacements.begin(), bernPlacements.end(),
			[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::NPC == placement.eKind &&
					placement.isEnabled;
			})) &&
		bernNpc != bernPlacements.end() &&
		WORLD_BOOTSTRAP_KIND::NPC == bernNpc->eKind &&
		bernNpc->strArchetypeId == "NPC_BEDA" &&
		bernAylara != bernPlacements.end() &&
		WORLD_BOOTSTRAP_KIND::NPC == bernAylara->eKind &&
		bernAylara->strArchetypeId == "NPC_AYLARA" &&
		bernAylara->isEnabled &&
		bernSchmidt != bernPlacements.end() &&
		WORLD_BOOTSTRAP_KIND::NPC == bernSchmidt->eKind &&
		bernSchmidt->strArchetypeId == "NPC_SCHMIDT" &&
		bernSchmidt->isEnabled &&
		bernPlayerEntrySpawn != bernPlacements.end() &&
		WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == bernPlayerEntrySpawn->eKind &&
		bernLeftGuardPatrol != bernPlacements.end() &&
		bernLeftGuardPatrol->bHasNpcBehavior &&
		NPC_BEHAVIOR_MODE::PATROL == bernLeftGuardPatrol->NpcBehavior.eMode &&
		NPC_ROUTE_MODE::PING_PONG ==
			bernLeftGuardPatrol->NpcBehavior.eRouteMode &&
		2u == bernLeftGuardPatrol->NpcBehavior.Waypoints.size() &&
		std::abs(bernLeftGuardPatrol->NpcBehavior.fMoveSpeed - 1.5f) < 0.001f &&
		bernRightGuardPatrol != bernPlacements.end() &&
		bernRightGuardPatrol->bHasNpcBehavior &&
		NPC_BEHAVIOR_MODE::PATROL == bernRightGuardPatrol->NpcBehavior.eMode &&
		NPC_ROUTE_MODE::PING_PONG ==
			bernRightGuardPatrol->NpcBehavior.eRouteMode &&
		2u == bernRightGuardPatrol->NpcBehavior.Waypoints.size() &&
		std::abs(bernRightGuardPatrol->NpcBehavior.fMoveSpeed - 1.5f) < 0.001f &&
		bernGuardPatrolsHeadToPlayerEntry &&
		20u == bernPlazaNpcCount && bernPlazaNpcContractValid &&
		8u == bernPlazaClusterCounts[0u] &&
		3u == bernPlazaClusterCounts[1u] &&
		5u == bernPlazaClusterCounts[2u] &&
		4u == bernPlazaClusterCounts[3u] &&
		20u == bernPlazaArchetypeIds.size() &&
		20u == bernPlazaActionIds.size() &&
		19u == bernPlazaLookTargetCount &&
		bernValtanTrigger != bernPlacements.end() &&
		WORLD_BOOTSTRAP_KIND::TRIGGER_BOX == bernValtanTrigger->eKind &&
		!bernValtanTrigger->isEnabled &&
		1u == bernValtanTrigger->TriggerActions.size() &&
		bernCollision != bernPlacements.end() &&
		WORLD_BOOTSTRAP_KIND::COLLISION_BOX == bernCollision->eKind,
		"Load Bern spawns, Schmidt, four irregular stationary plaza crowds, two guard ping-pong patrols toward the player entry, disabled legacy trigger, and collision box");
	CServerCollisionSystem bernCollisionSystem;
	std::string bernCollisionStatus;
	tests.Require(
		bernCollisionSystem.Initialize(bernPlacements, bernCollisionStatus) &&
		1u == bernCollisionSystem.Get_CollisionBoxCount() &&
		std::all_of(
			bernPlacements.begin(), bernPlacements.end(),
			[&bernCollisionSystem](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != placement.eKind ||
					bernCollisionSystem.Is_PlayerSpawnClear(placement);
			}),
		"Stage Bern collision box without overlapping player spawns");
	SERVER_PLAYER collisionPlayer{};
	collisionPlayer.fPositionX = 138.f;
	collisionPlayer.fPositionY = 42.7f;
	collisionPlayer.fPositionZ = -65.3f;
	float resolvedX = 0.f;
	float resolvedY = 0.f;
	float resolvedZ = 0.f;
	bool wasBlocked = false;
	tests.Require(
		bernCollisionSystem.Resolve_PlayerMove(
			collisionPlayer,
			143.f,
			42.7f,
			-65.3f,
			resolvedX,
			resolvedY,
			resolvedZ,
			wasBlocked) &&
		wasBlocked && resolvedX < 139.851f && resolvedX > 138.f,
		"Stop a fast player sweep before the Bern collision box");
	collisionPlayer.fPositionZ = -60.f;
	tests.Require(
		bernCollisionSystem.Resolve_PlayerMove(
			collisionPlayer,
			143.f,
			42.7f,
			-60.f,
			resolvedX,
			resolvedY,
			resolvedZ,
			wasBlocked) &&
		!wasBlocked && std::abs(resolvedX - 143.f) < 0.001f,
		"Preserve movement that passes outside the collision box");

	{
		/* Living monster and boss bodies block the player the same way, on the
		XZ plane, with the player's own half extent added to the body radius. */
		CServerCollisionSystem bodyCollision;
		std::string bodyStatus;
		tests.Require(bodyCollision.Initialize({}, bodyStatus),
			"Initialize an empty collision system for body blocking");
		bodyCollision.Set_BlockingBodies({ SERVER_BLOCKING_BODY{ 0.f, 3.f, 0.55f } });
		SERVER_PLAYER walker{};
		walker.fPositionX = 0.f;
		walker.fPositionY = 0.f;
		walker.fPositionZ = 0.f;
		float bodyX = 0.f;
		float bodyY = 0.f;
		float bodyZ = 0.f;
		bool bodyBlocked = false;
		/* Dead-on: reach the combined radius (z = 2 minus the contact margin),
		then the remaining 4 m of the step deflects to a fixed side at full
		length instead of parking, so the walk keeps its goal. */
		tests.Require(
			bodyCollision.Resolve_PlayerMove(
				walker, 0.f, 0.f, 6.f, bodyX, bodyY, bodyZ, bodyBlocked) &&
			!bodyBlocked && bodyZ > 1.9f && bodyZ < 2.f &&
			std::abs(bodyX - 4.f) < 0.01f,
			"Deflect a dead-on walk into a monster body around its side at full speed");
		tests.Require(
			bodyCollision.Resolve_PlayerMove(
				walker, 6.f, 0.f, 0.f, bodyX, bodyY, bodyZ, bodyBlocked) &&
			!bodyBlocked && std::abs(bodyX - 6.f) < 0.001f,
			"Preserve a player move that passes beside a monster body");
		SERVER_PLAYER overlapped = walker;
		overlapped.fPositionZ = 2.7f;
		tests.Require(
			bodyCollision.Resolve_PlayerMove(
				overlapped, 0.f, 0.f, 0.f, bodyX, bodyY, bodyZ, bodyBlocked) &&
			!bodyBlocked && std::abs(bodyZ) < 0.001f,
			"Let a player already inside a body step away from its centre");
		tests.Require(
			bodyCollision.Resolve_PlayerMove(
				overlapped, 0.f, 0.f, 3.f, bodyX, bodyY, bodyZ, bodyBlocked) &&
			!bodyBlocked && std::abs(bodyZ - 2.7f) < 0.001f &&
			std::abs(bodyX - 0.3f) < 0.001f,
			"Turn a step toward the centre of a body the player is inside into a sideways step");
		/* Off-centre approach: contact at z = 3 - sqrt(1 - 0.25), then the rest
		of the step slides along the tangent, away from the body and past its
		side, and the walk is not reported as blocked. */
		SERVER_PLAYER slider = walker;
		slider.fPositionX = -0.5f;
		tests.Require(
			bodyCollision.Resolve_PlayerMove(
				slider, -0.5f, 0.f, 6.f, bodyX, bodyY, bodyZ, bodyBlocked) &&
			!bodyBlocked && bodyX < -1.5f && bodyZ > 2.5f &&
			std::sqrt(bodyX * bodyX + (bodyZ - 3.f) * (bodyZ - 3.f)) > 0.999f,
			"Slide a player along a monster body instead of parking against it");
		SERVER_PLAYER insideSlider = walker;
		insideSlider.fPositionX = -0.3f;
		insideSlider.fPositionZ = 2.7f;
		tests.Require(
			bodyCollision.Resolve_PlayerMove(
				insideSlider, 0.3f, 0.f, 3.f, bodyX, bodyY, bodyZ, bodyBlocked) &&
			!bodyBlocked &&
			std::abs(bodyX - 0.174f) < 0.01f && std::abs(bodyZ - 2.226f) < 0.01f,
			"Slide a player already inside a body along the tangent at full step length");
		bodyCollision.Set_BlockingBodies({});
		tests.Require(
			bodyCollision.Resolve_PlayerMove(
				walker, 0.f, 0.f, 6.f, bodyX, bodyY, bodyZ, bodyBlocked) &&
			!bodyBlocked && std::abs(bodyZ - 6.f) < 0.001f,
			"Clear body blocking when the tick has no living bodies");
	}

	CServerNavigation bernNavigation;
	tests.Require(
		bernNavigation.Load("LV_BER_BERNCASTLE"),
		"Load Bern server navigation");
	const auto bernFirstSpawn = std::find_if(
		bernPlacements.begin(), bernPlacements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
				placement.isEnabled;
		});
	const auto hasReachableGuideApproach =
		[&bernNavigation, &bernFirstSpawn, &bernPlacements](
			const WORLD_BOOTSTRAP_PLACEMENT& guide)
	{
		if (bernFirstSpawn == bernPlacements.end())
			return false;
		constexpr float INTERACTION_RADIUS = 3.f;
		constexpr float CANDIDATE_STEP = 0.5f;
		constexpr int CANDIDATE_RADIUS_STEPS = 6;
		for (int zStep = -CANDIDATE_RADIUS_STEPS;
			zStep <= CANDIDATE_RADIUS_STEPS; ++zStep)
		{
			for (int xStep = -CANDIDATE_RADIUS_STEPS;
				xStep <= CANDIDATE_RADIUS_STEPS; ++xStep)
			{
				const float offsetX = static_cast<float>(xStep) * CANDIDATE_STEP;
				const float offsetZ = static_cast<float>(zStep) * CANDIDATE_STEP;
				if (offsetX * offsetX + offsetZ * offsetZ >
					INTERACTION_RADIUS * INTERACTION_RADIUS)
				{
					continue;
				}
				std::vector<SERVER_NAV_POINT> path;
				if (!bernNavigation.Find_Path(
					bernFirstSpawn->fPositionX,
					bernFirstSpawn->fPositionZ,
					guide.fPositionX + offsetX,
					guide.fPositionZ + offsetZ,
					path) || path.empty())
				{
					continue;
				}
				const SERVER_NAV_POINT& endpoint = path.back();
				if (std::hypot(
					endpoint.x - guide.fPositionX,
					endpoint.z - guide.fPositionZ) <= INTERACTION_RADIUS)
				{
					return true;
				}
			}
		}
		return false;
	};
	const bool bernGuideReachable =
		(bernNpc != bernPlacements.end() &&
			hasReachableGuideApproach(*bernNpc)) &&
		(bernAylara != bernPlacements.end() &&
			hasReachableGuideApproach(*bernAylara));
	tests.Require(
		bernGuideReachable,
		"Reach every Bern Valtan-entry guide NPC through authoritative navigation");
	bool bernSpawnsOnNavigation = bernLoaded;
	for (const WORLD_BOOTSTRAP_PLACEMENT& spawn : bernPlacements)
	{
		if (WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != spawn.eKind)
			continue;
		SERVER_NAV_POINT projected{};
		bernSpawnsOnNavigation =
			bernSpawnsOnNavigation &&
			bernNavigation.Project_Point(
				spawn.fPositionX,
				spawn.fPositionZ,
				projected) &&
			std::abs(projected.y - spawn.fPositionY) <= 0.25f;
	}
	tests.Require(
		bernSpawnsOnNavigation,
		"Project all Bern player spawns to baked navigation");
	/* The visible north-entry stair continues beyond the former z=-9.438004
	navigation boundary. Its authored final stair placement is centered at
	(136.86001, -3.30000488), so this exact world position must remain connected
	to the Bern spawn instead of being clipped by the grid extent. */
	constexpr float BERN_NORTH_ENTRY_X = 136.86001f;
	constexpr float BERN_NORTH_ENTRY_Z = -3.30000488f;
	SERVER_NAV_POINT bernNorthEntryGround{};
	std::vector<SERVER_NAV_POINT> bernNorthEntryPath;
	const bool bernNorthEntryReachable =
		bernNavigation.Sample_Position(
			BERN_NORTH_ENTRY_X,
			BERN_NORTH_ENTRY_Z,
			bernNorthEntryGround) &&
		bernNavigation.Find_Path(
			137.586334f,
			-22.4640217f,
			BERN_NORTH_ENTRY_X,
			BERN_NORTH_ENTRY_Z,
			bernNorthEntryPath) &&
		!bernNorthEntryPath.empty();
	tests.Require(
		bernNorthEntryReachable &&
		std::abs(bernNorthEntryGround.y - 42.2766418f) <= 0.25f &&
		std::abs(bernNorthEntryPath.back().x - 136.738007f) <= 0.26f &&
		std::abs(bernNorthEntryPath.back().z - (-3.1880035f)) <= 0.26f,
		"Reach the visible Bern north-entry stair through authoritative navigation");
	/* The castle approach is why Bern needs a grid at all: without one the room
	keeps the spawn height for the whole session and straight-line movement walks
	through the staircase. The authoritative path from the spawn to the top of the
	baked stair run must therefore carry a real climb, and the stair run itself
	must stay a walkable slope instead of one vertical jump. */
	std::vector<SERVER_NAV_POINT> bernStairPath;
	const bool bernStairPathFound = bernNavigation.Find_Path(
		137.586334f,
		-22.4640217f,
		137.238007f,
		-116.688004f,
		bernStairPath);
	float bernStairClimb = 0.f;
	float bernStairRunStep = 0.f;
	if (bernStairPathFound && !bernStairPath.empty())
	{
		bernStairClimb = bernStairPath.back().y - bernStairPath.front().y;
		for (size_t index = 1u; index < bernStairPath.size(); ++index)
		{
			if (bernStairPath[index].y <= 47.f)
				continue;
			bernStairRunStep = (std::max)(
				bernStairRunStep,
				std::abs(bernStairPath[index].y -
					bernStairPath[index - 1u].y));
		}
	}
	tests.Require(
		bernStairPathFound && bernStairPath.size() > 100u &&
		bernStairPath.back().y > 49.f && bernStairClimb > 6.f,
		"Climb the Bern castle stairs along the authoritative path");

	CWorldBootstrap trainingWorld;
	CServerNavigation trainingNavigation;
	tests.Require(trainingWorld.Load(WORLD_ID::TRAINING_GROUND) &&
		trainingWorld.Get_AreaId() == "LV_DEV_TRAINING_GROUND" &&
		std::all_of(
			trainingWorld.Get_Placements().begin(),
			trainingWorld.Get_Placements().end(),
			[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
					placement.strArchetypeId.empty();
			}),
		"Load class-neutral training player spawns");
	tests.Require(trainingNavigation.Load("LV_DEV_TRAINING_GROUND"),
		"Load training server navigation");
	SERVER_NAV_POINT trainingPoint{};
	tests.Require(trainingNavigation.Project_Point(0.f, -4.f, trainingPoint),
		"Project training spawn to walkable cell");
	tests.Require(!trainingNavigation.Project_Point(16.01f, 0.f, trainingPoint),
		"Reject training point beyond arena navigation bounds");

	CWorldBootstrap characterSelectWorld;
	CServerNavigation characterSelectNavigation;
	const bool characterSelectWorldLoaded =
		characterSelectWorld.Load(WORLD_ID::CHARACTER_SELECT_ARENA);
	const auto& characterSelectSpawns =
		characterSelectWorld.Get_Placements();
	const auto lazyValtan = std::find_if(
		characterSelectSpawns.begin(),
		characterSelectSpawns.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId ==
				"boss.valtan.character-select.lazy";
		});
	tests.Require(
		characterSelectWorldLoaded &&
		characterSelectWorld.Get_AreaId() ==
			"LV_LOBBY_CLASSSELECT_SL00" &&
		characterSelectSpawns.size() == 5 &&
		4u == static_cast<size_t>(std::count_if(
			characterSelectSpawns.begin(),
			characterSelectSpawns.end(),
			[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
					placement.strArchetypeId.empty() &&
					placement.isEnabled;
			})),
		"Load class-neutral Character Select arena player spawns");
	tests.Require(
		characterSelectSpawns.end() != lazyValtan &&
		!lazyValtan->isEnabled &&
		lazyValtan->eKind == WORLD_BOOTSTRAP_KIND::BOSS &&
		lazyValtan->strArchetypeId == "BOSS_VALTAN" &&
		lazyValtan->strEncounterId == "ENCOUNTER_VALTAN",
		"Load disabled Character Select Valtan lazy template");
	tests.Require(
		characterSelectNavigation.Load("LV_LOBBY_CLASSSELECT_SL00"),
		"Load Character Select arena server navigation");
	bool characterSelectSpawnsOnNavigation =
		characterSelectWorldLoaded && characterSelectSpawns.size() == 5;
	SERVER_NAV_POINT characterSelectPoint{};
	for (const WORLD_BOOTSTRAP_PLACEMENT& spawn : characterSelectSpawns)
	{
		if (WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != spawn.eKind)
			continue;
		SERVER_NAV_POINT projected{};
		characterSelectSpawnsOnNavigation =
			characterSelectSpawnsOnNavigation &&
			characterSelectNavigation.Project_Point(
				spawn.fPositionX,
				spawn.fPositionZ,
				projected) &&
			std::abs(projected.y - spawn.fPositionY) <= 0.25f;
	}
	tests.Require(
		characterSelectSpawnsOnNavigation,
		"Project all Character Select spawns to baked navigation");
	SERVER_NAV_POINT lazyValtanPoint{};
	tests.Require(
		characterSelectSpawns.end() != lazyValtan &&
		characterSelectNavigation.Project_Point(
			lazyValtan->fPositionX,
			lazyValtan->fPositionZ,
			lazyValtanPoint) &&
		std::abs(lazyValtanPoint.y - lazyValtan->fPositionY) <= 0.25f,
		"Project disabled Character Select Valtan template to navigation");
	if (!characterSelectSpawns.empty())
	{
		characterSelectNavigation.Project_Point(
			characterSelectSpawns.front().fPositionX,
			characterSelectSpawns.front().fPositionZ,
			characterSelectPoint);
	}
	std::vector<SERVER_NAV_POINT> characterSelectPath;
	tests.Require(
		characterSelectSpawns.size() >= 2 &&
		characterSelectNavigation.Find_Path(
			characterSelectSpawns.front().fPositionX,
			characterSelectSpawns.front().fPositionZ,
			characterSelectSpawns[1].fPositionX,
			characterSelectSpawns[1].fPositionZ,
			characterSelectPath) &&
		characterSelectPath.size() >= 2 &&
		std::adjacent_find(
			characterSelectPath.begin(),
			characterSelectPath.end(),
			[](const SERVER_NAV_POINT& left, const SERVER_NAV_POINT& right)
			{
				return std::abs(left.y - right.y) > 0.6f;
			}) == characterSelectPath.end(),
		"Find Character Select arena navigation path");
	SERVER_NAV_POINT characterSelectOutside{};
	tests.Require(
		!characterSelectNavigation.Project_Point(
			-787.6f,
			197.5f,
			characterSelectOutside),
		"Reject point beyond Character Select arena navigation bounds");

	SERVER_PLAYER arenaSkillPlayer{};
	arenaSkillPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
	arenaSkillPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
	arenaSkillPlayer.iCurrentHp = 1000;
	arenaSkillPlayer.iMaximumHp = 1000;
	arenaSkillPlayer.iCurrentResource = 1000;
	arenaSkillPlayer.iMaximumResource = 1000;
	arenaSkillPlayer.fPositionX = characterSelectPoint.x;
	arenaSkillPlayer.fPositionY = characterSelectPoint.y;
	arenaSkillPlayer.fPositionZ = characterSelectPoint.z;
	C2S_USE_SKILL arenaSkillCommand{};
	arenaSkillCommand.iClientSequence = 1;
	arenaSkillCommand.iSkillId = 34120;
	arenaSkillCommand.fAimX = characterSelectPoint.x + 3.f;
	arenaSkillCommand.fAimZ = characterSelectPoint.z;
	CPlayerSkillSystem arenaSkillSystem;
	std::vector<SERVER_WORLD_ENTITY> arenaEntities;
	tests.Require(
		arenaSkillSystem.Try_Start(
			arenaSkillPlayer,
			arenaSkillCommand,
			catalog,
			10) &&
		PLAYER_ACTION_STATE::SKILL == arenaSkillPlayer.eAction &&
		34120u == arenaSkillPlayer.iCurrentSkillId &&
		10u == arenaSkillPlayer.iActionStartTick,
		"Start Character Select arena skill action");
	std::vector<DAMAGE_EVENT> arenaDamageEvents;
	arenaSkillSystem.Update(
		arenaSkillPlayer,
		arenaEntities,
		catalog,
		&characterSelectNavigation,
		nullptr,
		1.f / 30.f,
		11,
		arenaDamageEvents);
	SERVER_NAV_POINT arenaSkillPoint{};
	tests.Require(
		characterSelectNavigation.Project_Point(
			arenaSkillPlayer.fPositionX,
			arenaSkillPlayer.fPositionZ,
			arenaSkillPoint),
		"Keep Character Select skill action position on baked navigation");

	SERVER_PLAYER player{};
	player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
	player.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
	player.iCurrentResource = 1000;
	player.iMaximumResource = 1000;
	player.fPositionX = 151.f;
	player.fPositionY = 22.97f;
	player.fPositionZ = -129.f;
	SERVER_WORLD_ENTITY boss{};
	boss.iNetEntityId = 900u;
	boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
	boss.eAction = SERVER_ENTITY_ACTION::IDLE;
	boss.strArchetypeId = "BOSS_VALTAN";
	boss.iCurrentHp = 10000;
	boss.iMaximumHp = 10000;
	boss.fPositionX = 151.f;
	boss.fPositionY = 22.97f;
	boss.fPositionZ = -125.f;
	boss.fCollisionRadius = 3.f;
	std::vector<SERVER_WORLD_ENTITY> entities{ boss };
	C2S_USE_SKILL useSkill{};
	useSkill.iClientSequence = 1;
	useSkill.iSkillId = 34120;
	useSkill.fAimX = boss.fPositionX;
	useSkill.fAimZ = boss.fPositionZ;
	CPlayerSkillSystem skills;
	tests.Require(skills.Try_Start(player, useSkill, catalog, 10),
		"Approve valid skill command");
	tests.Require(!skills.Try_Start(player, useSkill, catalog, 10),
		"Reject duplicate skill command while action is active");
	std::vector<DAMAGE_EVENT> damageEvents;
	for (std::uint32_t tick = 11; tick < 70; ++tick)
		skills.Update(player, entities, catalog, &navigation, nullptr,
			1.f / 30.f, tick, damageEvents);
	/* 34120 is official rate 361 at project-tuned attack power 1000, split across its three
	authored hit shapes so the sum stays exact. */
	const PLAYER_SKILL_DEFINITION* talonStrike = catalog.Find_Skill(34120);
	tests.Require(
		nullptr != talonStrike && 3u == talonStrike->Hits.size() &&
		talonStrike->Hits[0].iTimeMs < talonStrike->Hits[1].iTimeMs &&
		3u == talonStrike->Hits[0].iAreaType &&
		2u == talonStrike->Hits[2].iAreaType,
		"Load authored hit shapes for the skill from the gameplay bootstrap");
	tests.Require(6390u == entities[0].iCurrentHp,
		"Apply server-authoritative player damage across authored hits");
	std::uint32_t outgoingTotal = 0;
	for (const DAMAGE_EVENT& damageEvent : damageEvents)
		outgoingTotal += damageEvent.iAmount;
	tests.Require(
		3u == damageEvents.size() &&
		3610u == outgoingTotal &&
		damageEvents[0].isOutgoing &&
		entities[0].iNetEntityId == damageEvents[0].iTargetNetEntityId,
		"Emit one outgoing damage event per authored hit summing to the profile rate");
	{
		/* Armour is server state. It mitigates every incoming hit while a plate is
		intact and only loses durability inside a GROGGY stage, so the numbers here
		come from the published bootstrap and the same skill path the arena runs. */
		const BOSS_RUNTIME_PROFILE* armorProfile = catalog.Find_Boss("BOSS_VALTAN");
		tests.Require(
			nullptr != armorProfile &&
			2u == armorProfile->ArmorPlates.size() &&
			0u == armorProfile->ArmorPlates[0].iPlateIndex &&
			1u == armorProfile->ArmorPlates[1].iPlateIndex &&
			20u == armorProfile->ArmorPlates[0].iDurability &&
			20u == armorProfile->ArmorPlates[1].iDurability &&
			50u == armorProfile->ArmorPlates[0].iDefense &&
			50u == armorProfile->ArmorPlates[1].iDefense,
			"Load Valtan's two authored armour plates from the gameplay bootstrap");

		std::uint32_t armorSequence = 10u;
		const auto strikeArmoredBoss =
			[&](const bool groggy,
				const std::uint32_t firstDurability,
				const std::uint32_t secondDurability,
				SERVER_WORLD_ENTITY& outBoss) -> std::uint32_t
		{
			SERVER_PLAYER armorPlayer{};
			armorPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			armorPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
			armorPlayer.iCurrentResource = 1000;
			armorPlayer.iMaximumResource = 1000;
			armorPlayer.fPositionX = 151.f;
			armorPlayer.fPositionY = 22.97f;
			armorPlayer.fPositionZ = -129.f;
			SERVER_WORLD_ENTITY armored = boss;
			armored.bPatternGroggy = groggy;
			armored.ArmorPlates.clear();
			for (const BOSS_ARMOR_PLATE& plate : armorProfile->ArmorPlates)
			{
				SERVER_BOSS_ARMOR_PLATE_STATE state{};
				state.iPlateIndex = plate.iPlateIndex;
				state.iDefense = plate.iDefense;
				state.iRemainingDurability = 0u == plate.iPlateIndex ?
					firstDurability : secondDurability;
				armored.ArmorPlates.push_back(state);
			}
			std::vector<SERVER_WORLD_ENTITY> armorEntities{ armored };
			C2S_USE_SKILL armorSkill = useSkill;
			armorSkill.iClientSequence = ++armorSequence;
			CPlayerSkillSystem armorSkills;
			armorSkills.Try_Start(armorPlayer, armorSkill, catalog, 10);
			std::vector<DAMAGE_EVENT> armorEvents;
			for (std::uint32_t tick = 11; tick < 70; ++tick)
			{
				armorSkills.Update(armorPlayer, armorEntities, catalog,
					&navigation, nullptr, 1.f / 30.f, tick, armorEvents);
			}
			outBoss = armorEntities[0];
			return armored.iCurrentHp - armorEntities[0].iCurrentHp;
		};

		SERVER_WORLD_ENTITY strippedBoss{};
		const std::uint32_t strippedDamage =
			strikeArmoredBoss(false, 0u, 0u, strippedBoss);
		SERVER_WORLD_ENTITY halfArmoredBoss{};
		const std::uint32_t halfArmoredDamage =
			strikeArmoredBoss(false, 0u, 4000u, halfArmoredBoss);
		SERVER_WORLD_ENTITY fullyArmoredBoss{};
		const std::uint32_t fullyArmoredDamage =
			strikeArmoredBoss(false, 4000u, 4000u, fullyArmoredBoss);
		tests.Require(
			3610u == strippedDamage &&
			fullyArmoredDamage < halfArmoredDamage &&
			halfArmoredDamage < strippedDamage,
			"Mitigate a boss hit by each intact plate and by nothing once all break");
		{
			/* An invulnerable pattern absorbs the hit whole: no HP moves and no
			damage event reaches the snapshot, so the raid can see the mechanic
			has to be answered rather than outraced. */
			SERVER_PLAYER immunePlayer{};
			immunePlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			immunePlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
			immunePlayer.iCurrentResource = 1000;
			immunePlayer.iMaximumResource = 1000;
			immunePlayer.fPositionX = 151.f;
			immunePlayer.fPositionY = 22.97f;
			immunePlayer.fPositionZ = -129.f;
			SERVER_WORLD_ENTITY immuneBoss = boss;
			immuneBoss.bPatternInvulnerable = true;
			std::vector<SERVER_WORLD_ENTITY> immuneEntities{ immuneBoss };
			C2S_USE_SKILL immuneSkill = useSkill;
			immuneSkill.iClientSequence = 41;
			CPlayerSkillSystem immuneSkills;
			immuneSkills.Try_Start(immunePlayer, immuneSkill, catalog, 10);
			std::vector<DAMAGE_EVENT> immuneEvents;
			for (std::uint32_t tick = 11; tick < 70; ++tick)
			{
				immuneSkills.Update(immunePlayer, immuneEntities, catalog,
					&navigation, nullptr, 1.f / 30.f, tick, immuneEvents);
			}
			tests.Require(
				immuneBoss.iCurrentHp == immuneEntities[0].iCurrentHp &&
				immuneEvents.empty(),
				"Absorb every player hit while an invulnerable pattern runs");
		}
		tests.Require(
			4000u == fullyArmoredBoss.ArmorPlates[0].iRemainingDurability &&
			4000u == fullyArmoredBoss.ArmorPlates[1].iRemainingDurability,
			"Leave armour durability untouched by damage outside a groggy stage");

		SERVER_WORLD_ENTITY groggyBoss{};
		const std::uint32_t groggyDamage =
			strikeArmoredBoss(true, 4000u, 4000u, groggyBoss);
		tests.Require(
			groggyDamage == fullyArmoredDamage &&
			4000u - groggyDamage ==
				groggyBoss.ArmorPlates[0].iRemainingDurability &&
			4000u == groggyBoss.ArmorPlates[1].iRemainingDurability,
			"Spend groggy damage on the front plate only, at the mitigated amount");

		SERVER_WORLD_ENTITY brokenBoss{};
		(void)strikeArmoredBoss(true, 1u, 4000u, brokenBoss);
		tests.Require(
			0u == brokenBoss.ArmorPlates[0].iRemainingDurability &&
			4000u == brokenBoss.ArmorPlates[1].iRemainingDurability &&
			brokenBoss.bPendingArmorBreakReaction &&
			!brokenBoss.bPatternGroggy,
			"Break one plate per groggy window, close it, and queue the part-break reaction");
	}
	{
		/* The Arena boss owns typed parts. Exercise the ordinary authored skill
		path inside Dash GROGGY: its first landed sub-hit must destroy one eligible
		part, publish the typed outcome, and drive GROGGY -> PART_BREAK directly. */
		const BOSS_RUNTIME_PROFILE* armorProfile =
			catalog.Find_Boss("BOSS_VALTAN");
		const std::vector<BOSS_PART_DEFINITION>* partDefinitions =
			catalog.Find_BossParts("BOSS_VALTAN");
		SERVER_WORLD_ENTITY typedDashBoss = boss;
		typedDashBoss.iNetEntityId = 88001u;
		typedDashBoss.strPatternId = "VALTAN_DASH_CHARGE";
		typedDashBoss.strEncounterId = "ENCOUNTER_VALTAN";
		typedDashBoss.strPatternStageId = "GROGGY";
		typedDashBoss.strActionId = "valtan.attack.dash-charge.groggy";
		typedDashBoss.iPatternSequence = 901u;
		typedDashBoss.iPatternStageIndex = 2u;
		typedDashBoss.iPatternStageDurationMs = 5000u;
		typedDashBoss.iPatternStageFirstEvaluationTick = 11u;
		typedDashBoss.eAction = SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
		typedDashBoss.ePatternPartDamagePolicy =
			BOSS_PATTERN_PART_DAMAGE_POLICY::DESTROY_FIRST_ELIGIBLE;
		typedDashBoss.bPatternGroggy = true;
		typedDashBoss.bIntroPatternConsumed = true;
		typedDashBoss.bAutomaticPatternSequenceAuditionOverride = true;
		typedDashBoss.fEngageDistance = 35.f;
		typedDashBoss.PinnedDefinitionRevision = catalog.Get_ActiveRevision();
		typedDashBoss.ArmorPlates.clear();
		if (nullptr != armorProfile)
		{
			for (const BOSS_ARMOR_PLATE& plate : armorProfile->ArmorPlates)
			{
				typedDashBoss.ArmorPlates.push_back({
					plate.iPlateIndex, plate.iDurability, plate.iDefense });
			}
		}
		std::string typedDashStatus;
		const bool initializedTypedDash = nullptr != partDefinitions &&
			CBossCombatRuntime::Initialize(
				typedDashBoss.BossCombat, *partDefinitions, typedDashStatus) &&
			!typedDashBoss.BossCombat.Parts.empty();
		if (initializedTypedDash)
		{
			typedDashBoss.BossCombat.Parts.front().iCurrentDurability = 100u;
			(void)CBossCombatRuntime::Set_Flag(
				typedDashBoss.BossCombat,
				SERVER_BOSS_COMBAT_FLAG::GROGGY, true);
		}
		const std::uint32_t aliveMaskBefore =
			typedDashBoss.BossCombat.iAlivePartMask;
		const std::uint32_t hpBeforeTypedDash = typedDashBoss.iCurrentHp;
		std::vector<SERVER_WORLD_ENTITY> typedDashEntities{ typedDashBoss };

		SERVER_PLAYER typedDashPlayer{};
		typedDashPlayer.iPlayerId = 88002u;
		typedDashPlayer.iNetEntityId = 88003u;
		typedDashPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		typedDashPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		typedDashPlayer.iCurrentHp = 1000u;
		typedDashPlayer.iMaximumHp = 1000u;
		typedDashPlayer.isCombatReady = true;
		typedDashPlayer.iCurrentResource = 1000u;
		typedDashPlayer.iMaximumResource = 1000u;
		typedDashPlayer.fPositionX = boss.fPositionX;
		typedDashPlayer.fPositionY = boss.fPositionY;
		typedDashPlayer.fPositionZ = boss.fPositionZ - 4.f;
		typedDashEntities.front().iPatternTargetEntityId =
			typedDashPlayer.iNetEntityId;
		C2S_USE_SKILL typedDashSkill = useSkill;
		typedDashSkill.iClientSequence = 75u;
		CPlayerSkillSystem typedDashSkills;
		const bool startedTypedDashSkill = typedDashSkills.Try_Start(
			typedDashPlayer, typedDashSkill, catalog, 10u);
		std::vector<DAMAGE_EVENT> typedDashEvents;
		for (std::uint32_t tick = 11u; tick < 70u; ++tick)
		{
			typedDashSkills.Update(
				typedDashPlayer, typedDashEntities, catalog, &navigation,
				nullptr, 1.f / 30.f, tick, typedDashEvents);
		}
		SERVER_WORLD_ENTITY& resolvedTypedDash = typedDashEntities.front();
		const std::uint32_t destroyedMask = aliveMaskBefore &
			~resolvedTypedDash.BossCombat.iAlivePartMask;
		const bool mirroredLegacyPlate = std::any_of(
			resolvedTypedDash.ArmorPlates.begin(),
			resolvedTypedDash.ArmorPlates.end(),
			[destroyedMask](const SERVER_BOSS_ARMOR_PLATE_STATE& plate)
			{
				return plate.iPlateIndex < 32u &&
					0u != (destroyedMask & (1u << plate.iPlateIndex)) &&
					0u == plate.iRemainingDurability;
			});
		std::uint32_t typedDashEventDamage = 0u;
		for (const DAMAGE_EVENT& event : typedDashEvents)
			typedDashEventDamage += event.iAmount;
		SERVER_WORLD_ENTITY typedDashOutcomeProbe = resolvedTypedDash;
		const bool publishedPartDestroyed =
			CBossCombatRuntime::Consume_PatternOutcome(
				typedDashOutcomeProbe, "valtan.attack.dash-charge.groggy",
				BOSS_PATTERN_STAGE_OUTCOME::PART_DESTROYED);
		const bool closedGroggyAfterPartHit =
			"GROGGY" == resolvedTypedDash.strPatternStageId &&
			2u == resolvedTypedDash.iPatternStageIndex &&
			!resolvedTypedDash.bPatternGroggy &&
			resolvedTypedDash.bPendingArmorBreakReaction;
		std::map<PLAYER_ID, SERVER_PLAYER> typedDashBranchPlayers;
		typedDashBranchPlayers.emplace(
			typedDashPlayer.iPlayerId, typedDashPlayer);
		CValtanBrain typedDashBrain;
		std::vector<DAMAGE_EVENT> typedDashBranchEvents;
		typedDashBrain.Update(
			resolvedTypedDash, typedDashBranchPlayers, catalog, navigation,
			1.f / 30.f, 70u, {}, typedDashBranchEvents);
		const bool enteredPartBreakFromGroggy =
			"PART_BREAK" == resolvedTypedDash.strPatternStageId &&
			4u == resolvedTypedDash.iPatternStageIndex &&
			!resolvedTypedDash.bPatternGroggy &&
			!resolvedTypedDash.bPendingArmorBreakReaction;
		tests.Require(
			initializedTypedDash && startedTypedDashSkill &&
			(1u == destroyedMask || 2u == destroyedMask) &&
			mirroredLegacyPlate && publishedPartDestroyed &&
			closedGroggyAfterPartHit && enteredPartBreakFromGroggy &&
			!CBossCombatRuntime::Has_Flag(
				typedDashOutcomeProbe.BossCombat,
				SERVER_BOSS_COMBAT_FLAG::GROGGY) &&
			hpBeforeTypedDash - typedDashOutcomeProbe.iCurrentHp ==
				typedDashEventDamage,
			"Route a real skill hit through Dash GROGGY and directly into PART_BREAK without double HP damage");
	}
	{
		/* The only authored counter-power skill reaches the boss through the same
		path after its guard promotes to stage two.  A live boss counter window
		publishes one typed edge and closes on that landed hit. */
		SERVER_WORLD_ENTITY counterBoss = boss;
		counterBoss.iNetEntityId = 88101u;
		counterBoss.fPositionX = 1.f;
		counterBoss.fPositionY = 0.f;
		counterBoss.fPositionZ = 0.f;
		counterBoss.strPatternId = "VALTAN_COUNTER";
		counterBoss.strPatternStageId = "WINDUP";
		counterBoss.strActionId = "valtan.sequence.counter.step-01";
		counterBoss.iPatternSequence = 902u;
		counterBoss.ArmorPlates.clear();
		std::string counterBossStatus;
		const std::vector<BOSS_PART_DEFINITION> noCounterParts;
		const bool initializedCounterBoss = CBossCombatRuntime::Initialize(
			counterBoss.BossCombat, noCounterParts, counterBossStatus);
		(void)CBossCombatRuntime::Set_Flag(
			counterBoss.BossCombat,
			SERVER_BOSS_COMBAT_FLAG::COUNTERABLE, true);
		const std::uint32_t hpBeforeBossCounter = counterBoss.iCurrentHp;
		std::vector<SERVER_WORLD_ENTITY> counterBossEntities{ counterBoss };

		SERVER_PLAYER bossCounterPlayer{};
		bossCounterPlayer.iPlayerId = 88102u;
		bossCounterPlayer.iNetEntityId = 88103u;
		bossCounterPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		bossCounterPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		bossCounterPlayer.iCurrentHp = 1000u;
		bossCounterPlayer.iMaximumHp = 1000u;
		bossCounterPlayer.iCurrentResource = 1000u;
		bossCounterPlayer.iMaximumResource = 1000u;
		bossCounterPlayer.fPositionX = 0.f;
		bossCounterPlayer.fPositionY = 0.f;
		bossCounterPlayer.fPositionZ = 0.f;
		C2S_USE_SKILL bossCounterSkill{};
		bossCounterSkill.iClientSequence = 76u;
		bossCounterSkill.iSkillId = 34580u;
		bossCounterSkill.fAimX = counterBoss.fPositionX;
		bossCounterSkill.fAimZ = counterBoss.fPositionZ;
		CPlayerSkillSystem bossCounterSkills;
		const bool startedBossCounter = bossCounterSkills.Try_Start(
			bossCounterPlayer, bossCounterSkill, catalog, 10u) &&
			CPlayerSkillSystem::Try_Counter(
				bossCounterPlayer, catalog, 12u);
		std::vector<DAMAGE_EVENT> bossCounterEvents;
		for (std::uint32_t tick = 13u; tick < 35u; ++tick)
		{
			bossCounterSkills.Update(
				bossCounterPlayer, counterBossEntities, catalog, nullptr,
				nullptr, 1.f / 30.f, tick, bossCounterEvents);
		}
		SERVER_WORLD_ENTITY& resolvedCounterBoss =
			counterBossEntities.front();
		std::uint32_t bossCounterEventDamage = 0u;
		for (const DAMAGE_EVENT& event : bossCounterEvents)
			bossCounterEventDamage += event.iAmount;
		const bool publishedCounterHit =
			CBossCombatRuntime::Consume_PatternOutcome(
				resolvedCounterBoss, "valtan.sequence.counter.step-01",
				BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT);
		tests.Require(
			initializedCounterBoss && startedBossCounter &&
			publishedCounterHit &&
			!CBossCombatRuntime::Has_Flag(
				resolvedCounterBoss.BossCombat,
				SERVER_BOSS_COMBAT_FLAG::COUNTERABLE) &&
			!bossCounterEvents.empty() &&
			hpBeforeBossCounter - resolvedCounterBoss.iCurrentHp ==
				bossCounterEventDamage,
			"Publish and close a boss counter window from the real player skill hit path");
	}
	{
		/* Make one otherwise-eligible row the only row not on cooldown, then
		mark it at its repeat limit. The same weighted evaluator must relax only
		the soft repeat reason and expose the original block plus the relaxation. */
		const auto* repeatPatterns =
			catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const BOSS_PATTERN_DEFINITION* repeatPattern = nullptr;
		if (nullptr != repeatPatterns)
		{
			for (const BOSS_PATTERN_DEFINITION& candidate : *repeatPatterns)
			{
				if (BOSS_PATTERN_SELECTION::NORMAL != candidate.eSelection ||
					"VALTAN_ENTRANCE_WHIRLWIND" == candidate.strPatternId ||
					candidate.iMinimumPhase > 1u || candidate.iMaximumPhase < 1u ||
					candidate.iMinimumHealthBar > 160u ||
					candidate.iMaximumHealthBar < 160u ||
					BOSS_PATTERN_ARMOR_REQUIREMENT::ARMORED ==
						candidate.eArmorRequirement ||
					BOSS_PATTERN_PHASE_REQUIREMENT::PHASE_TWO ==
						candidate.ePhaseRequirement ||
					0u == candidate.iMaximumConsecutiveUses)
				{
					continue;
				}
				const std::size_t sourceFamilyCount =
					static_cast<std::size_t>(std::count_if(
						repeatPatterns->begin(), repeatPatterns->end(),
						[&candidate](const BOSS_PATTERN_DEFINITION& other)
						{
							return BOSS_PATTERN_SELECTION::NORMAL ==
								other.eSelection &&
								other.iSourcePrimaryActionId ==
									candidate.iSourcePrimaryActionId;
						}));
				if (1u == sourceFamilyCount)
				{
					repeatPattern = &candidate;
					break;
				}
			}
		}
		std::map<PLAYER_ID, SERVER_PLAYER> repeatPlayers;
		SERVER_PLAYER repeatTarget{};
		repeatTarget.iPlayerId = 7001u;
		repeatTarget.iNetEntityId = 7002u;
		repeatTarget.iCurrentHp = 1000000u;
		repeatTarget.iMaximumHp = 1000000u;
		repeatTarget.isCombatReady = true;
		if (nullptr != repeatPattern)
			repeatTarget.fPositionX = (repeatPattern->fMinimumRange +
				repeatPattern->fMaximumRange) * 0.5f;
		repeatPlayers.emplace(repeatTarget.iPlayerId, repeatTarget);
		SERVER_WORLD_ENTITY repeatBoss{};
		repeatBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		repeatBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		repeatBoss.strArchetypeId = "BOSS_VALTAN";
		repeatBoss.strEncounterId = "ENCOUNTER_VALTAN";
		repeatBoss.iCurrentHp = 60000u;
		repeatBoss.iMaximumHp = 60000u;
		repeatBoss.iMaximumHealthBars = 160u;
		repeatBoss.iLastEvaluatedHealthBar = 160u;
		repeatBoss.iPhase = 1u;
		repeatBoss.fEngageDistance = 1000.f;
		repeatBoss.bIntroPatternConsumed = true;
		repeatBoss.bScriptedPatternPlayback = true;
		if (nullptr != repeatPattern && nullptr != repeatPatterns)
		{
			repeatBoss.strLastPatternId = repeatPattern->strPatternId;
			repeatBoss.iConsecutivePatternUses =
				repeatPattern->iMaximumConsecutiveUses;
			for (const BOSS_PATTERN_DEFINITION& candidate : *repeatPatterns)
			{
				if (BOSS_PATTERN_SELECTION::NORMAL != candidate.eSelection ||
					candidate.strPatternId == repeatPattern->strPatternId)
				{
					continue;
				}
				SERVER_BOSS_PATTERN_COOLDOWN cooldown{};
				cooldown.iSourcePrimaryActionId =
					candidate.iSourcePrimaryActionId;
				cooldown.iReadyTick = 22000u;
				repeatBoss.PatternCooldowns.push_back(std::move(cooldown));
			}
		}
		std::vector<DAMAGE_EVENT> repeatDamageEvents;
		auto repeatBrainStorage = std::make_unique<CValtanBrain>();
		CValtanBrain& repeatBrain = *repeatBrainStorage;
		repeatBrain.Update(
			repeatBoss, repeatPlayers, catalog, navigation,
			1.f / 30.f, 21000u, {}, repeatDamageEvents);
		const VALTAN_DECISION_TRACE* repeatTrace =
			repeatBrain.Get_LatestDecisionTrace();
		const auto repeatCandidate = nullptr == repeatTrace ||
			nullptr == repeatPattern ?
				VALTAN_DECISION_CANDIDATE_TRACE{} :
				[repeatTrace, repeatPattern]()
				{
					const auto found = std::find_if(
						repeatTrace->Candidates.begin(),
						repeatTrace->Candidates.end(),
						[repeatPattern](
							const VALTAN_DECISION_CANDIDATE_TRACE& candidate)
						{
							return candidate.strPatternId ==
								repeatPattern->strPatternId;
						});
					return repeatTrace->Candidates.end() == found ?
						VALTAN_DECISION_CANDIDATE_TRACE{} : *found;
				}();
		bool weightIntervalsPartition = nullptr != repeatTrace;
		std::uint64_t nextWeightBegin = 0u;
		std::size_t effectiveCandidateCount = 0u;
		if (nullptr != repeatTrace)
		{
			for (const VALTAN_DECISION_CANDIDATE_TRACE& candidate :
				repeatTrace->Candidates)
			{
				if (0u == candidate.iEffectiveWeight) continue;
				++effectiveCandidateCount;
				weightIntervalsPartition = weightIntervalsPartition &&
					candidate.iWeightBeginInclusive == nextWeightBegin &&
					candidate.iWeightEndExclusive >
						candidate.iWeightBeginInclusive &&
					candidate.iWeightEndExclusive -
						candidate.iWeightBeginInclusive ==
						candidate.iEffectiveWeight &&
					candidate.iWeightEndExclusive <= repeatTrace->iTotalWeight &&
					(!candidate.bSelected ||
						(repeatTrace->iRandomTicket >=
							candidate.iWeightBeginInclusive &&
						 repeatTrace->iRandomTicket <
							candidate.iWeightEndExclusive));
				nextWeightBegin = candidate.iWeightEndExclusive;
			}
			weightIntervalsPartition = weightIntervalsPartition &&
				0u != effectiveCandidateCount &&
				nextWeightBegin == repeatTrace->iTotalWeight;
		}
		tests.Require(
			nullptr != repeatPattern && nullptr != repeatTrace &&
			weightIntervalsPartition &&
			repeatTrace->bMaximumConsecutiveRelaxed &&
			VALTAN_DECISION_RESULT::SELECTED == repeatTrace->eResult &&
			repeatPattern->strPatternId == repeatTrace->strSelectedPatternId &&
			repeatCandidate.bSoftRepeatBlocked &&
			repeatCandidate.bSoftRepeatRelaxed && repeatCandidate.bSelected &&
			0u != (repeatCandidate.iExclusionMask &
				VALTAN_EXCLUDE_SOFT_REPEAT_BLOCKED) &&
			0u != (repeatCandidate.iExclusionMask &
				VALTAN_EXCLUDE_SOFT_REPEAT_RELAXED),
			"Relax soft repeat avoidance and partition every effective decision weight interval before selection");

		auto traceRoomStorage =
			std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& traceRoom = *traceRoomStorage;
		SERVER_WORLD_ENTITY traceBoss = repeatBoss;
		traceBoss.iNetEntityId = 79991u;
		traceBoss.strPlacementId = "boss.valtan.decision-trace-contract";
		traceBoss.PinnedDefinitionRevision =
			traceRoom.m_GameplayCatalog.Get_ActiveRevision();
		traceRoom.m_WorldEntities.push_back(traceBoss);
		traceRoom.m_ValtanBrain = repeatBrain;
		if (nullptr != repeatTrace)
		{
			traceRoom.m_ValtanDecisionTraceRevision.iBossEntityId =
				traceBoss.iNetEntityId;
			traceRoom.m_ValtanDecisionTraceRevision.strBossPlacementId =
				traceBoss.strPlacementId;
			traceRoom.m_ValtanDecisionTraceRevision.iTraceSequence =
				repeatTrace->iTraceSequence;
			traceRoom.m_ValtanDecisionTraceRevision.DefinitionRevision =
				traceBoss.PinnedDefinitionRevision;
		}
		C2S_VALTAN_DECISION_TRACE_QUERY traceQuery{};
		traceQuery.iRequestSequence = 91u;
		traceQuery.strBossPlacementId = traceBoss.strPlacementId;
		S2C_VALTAN_DECISION_TRACE_RESPONSE traceResponse{};
		std::string traceStatus;
		CPacketWriter traceWriter;
		const bool mappedTrace = nullptr != repeatTrace &&
			traceRoom.Build_ValtanDecisionTraceResponse(
				traceQuery, traceResponse, traceStatus) &&
			VALTAN_DECISION_TRACE_QUERY_RESULT::TRACE == traceResponse.eResult &&
			traceBoss.PinnedDefinitionRevision ==
				traceResponse.DefinitionRevision &&
			repeatTrace->iTraceSequence == traceResponse.Trace.iTraceSequence &&
			repeatTrace->Candidates.size() == traceResponse.Trace.Candidates.size() &&
			Write_Message(traceWriter, traceResponse);
		C2S_VALTAN_DECISION_TRACE_QUERY unchangedQuery = traceQuery;
		unchangedQuery.iRequestSequence = 92u;
		unchangedQuery.iAfterTraceSequence = nullptr == repeatTrace ? 0u :
			repeatTrace->iTraceSequence;
		S2C_VALTAN_DECISION_TRACE_RESPONSE unchangedResponse{};
		const bool mappedUnchanged =
			traceRoom.Build_ValtanDecisionTraceResponse(
				unchangedQuery, unchangedResponse, traceStatus) &&
			VALTAN_DECISION_TRACE_QUERY_RESULT::UNCHANGED ==
				unchangedResponse.eResult &&
			!unchangedResponse.DefinitionRevision.Is_Valid() &&
			0u == unchangedResponse.Trace.iTraceSequence;
		GameplayDataRevision collectedSelectorRevision =
			traceBoss.PinnedDefinitionRevision;
		collectedSelectorRevision.Bytes[7] ^= 0x80u;
		if (!collectedSelectorRevision.Is_Valid() ||
			collectedSelectorRevision == traceBoss.PinnedDefinitionRevision)
		{
			collectedSelectorRevision.Bytes[8] ^= 1u;
		}
		traceRoom.m_ValtanDecisionTraceRevision.DefinitionRevision =
			collectedSelectorRevision;
		S2C_VALTAN_DECISION_TRACE_RESPONSE collectedTraceResponse{};
		const bool mappedCollectedTrace =
			nullptr == traceRoom.Resolve_GameplayGeneration(
				collectedSelectorRevision) &&
			traceRoom.Build_ValtanDecisionTraceResponse(
				traceQuery, collectedTraceResponse, traceStatus) &&
			VALTAN_DECISION_TRACE_QUERY_RESULT::TRACE ==
				collectedTraceResponse.eResult &&
			collectedSelectorRevision ==
				collectedTraceResponse.DefinitionRevision;
		traceRoom.m_ValtanDecisionTraceRevision.DefinitionRevision =
			traceBoss.PinnedDefinitionRevision;
		bool rejectedUnknownExclusion = true;
		VALTAN_DECISION_TRACE* mutableTrace = const_cast<VALTAN_DECISION_TRACE*>(
			traceRoom.m_ValtanBrain.Get_LatestDecisionTrace());
		if (nullptr != mutableTrace && !mutableTrace->Candidates.empty())
		{
			const std::uint32_t originalMask =
				mutableTrace->Candidates.front().iExclusionMask;
			mutableTrace->Candidates.front().iExclusionMask |= 0x80000000u;
			S2C_VALTAN_DECISION_TRACE_RESPONSE invalidResponse{};
			rejectedUnknownExclusion =
				!traceRoom.Build_ValtanDecisionTraceResponse(
					traceQuery, invalidResponse, traceStatus);
			mutableTrace->Candidates.front().iExclusionMask = originalMask;
		}
		tests.Require(
			mappedTrace && mappedUnchanged && mappedCollectedTrace &&
			rejectedUnknownExclusion,
			"Map the authoritative Valtan decision trace after selector-generation GC and fail-close unknown exclusion bits");
	}

	{
		/* Both charges use the same typed impact marker and enter GROGGY directly
		on a wall contact. Dash timeout still bypasses the groggy part window and
		enters ordinary RECOVERY. */
		const auto* chargePatterns = catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const auto findChargeStage =
			[&](const char* patternId,
				const BOSS_PATTERN_STAGE_KIND expectedNextKind,
				const std::string_view expectedNextActionId)
				-> const BOSS_PATTERN_STAGE_DEFINITION*
		{
			if (nullptr == chargePatterns)
				return nullptr;
			for (const BOSS_PATTERN_DEFINITION& pattern : *chargePatterns)
			{
				if (pattern.strPatternId != patternId)
					continue;
				for (std::size_t index = 0; index < pattern.Stages.size(); ++index)
				{
					if (!pattern.Stages[index].bChargeImpact)
						continue;
					return index + 1u < pattern.Stages.size() &&
						expectedNextKind == pattern.Stages[index + 1u].eStageKind &&
						expectedNextActionId ==
							pattern.Stages[index + 1u].strActionId ?
						&pattern.Stages[index] : nullptr;
				}
			}
			return nullptr;
		};
		const BOSS_PATTERN_STAGE_DEFINITION* openingCharge =
			findChargeStage(
				"VALTAN_ARMOR_BREAK_OPENING", BOSS_PATTERN_STAGE_KIND::GROGGY,
				"valtan.mechanic.armor-break-opening.groggy");
		const BOSS_PATTERN_STAGE_DEFINITION* dashCharge =
			findChargeStage(
				"VALTAN_DASH_CHARGE", BOSS_PATTERN_STAGE_KIND::GROGGY,
				"valtan.attack.dash-charge.groggy");
		tests.Require(
			nullptr != openingCharge && nullptr != dashCharge &&
			openingCharge->strStageId == "WALL_CHARGE" &&
			dashCharge->strStageId == "CHARGE",
			"Load both wall-contact charge graphs into their immediate GROGGY windows");

		std::uint32_t chargeStageCount = 0u;
		std::uint32_t groggyStageCount = 0u;
		if (nullptr != chargePatterns)
		{
			for (const BOSS_PATTERN_DEFINITION& pattern : *chargePatterns)
			{
				if (pattern.strPatternId != "VALTAN_ARMOR_BREAK_OPENING" &&
					pattern.strPatternId != "VALTAN_DASH_CHARGE")
					continue;
				for (const BOSS_PATTERN_STAGE_DEFINITION& stage : pattern.Stages)
				{
					if (stage.bChargeImpact)
						++chargeStageCount;
					if (BOSS_PATTERN_STAGE_KIND::GROGGY == stage.eStageKind)
						++groggyStageCount;
				}
			}
		}
		tests.Require(
			2u == chargeStageCount && 2u == groggyStageCount,
			"Author exactly one groggy reaction in each of the two charge graphs");

		/* The dash repeats, so a fight can open more than one window and strip
		more than one plate; the scripted opening alone never could. */
		const BOSS_PATTERN_DEFINITION* dashPattern = nullptr;
		if (nullptr != chargePatterns)
		{
			for (const BOSS_PATTERN_DEFINITION& pattern : *chargePatterns)
			{
				if (pattern.strPatternId == "VALTAN_DASH_CHARGE")
					dashPattern = &pattern;
			}
		}
		const auto hasDashBranch = [](
			const BOSS_PATTERN_STAGE_DEFINITION& stage,
			const BOSS_PATTERN_STAGE_OUTCOME outcome,
			const std::string_view nextActionId)
		{
			return std::any_of(
				stage.Branches.begin(), stage.Branches.end(),
				[outcome, nextActionId](const BOSS_PATTERN_STAGE_BRANCH& branch)
				{
					return branch.eOutcome == outcome &&
						branch.strNextActionId == nextActionId;
				});
		};
		const auto hasDashFlagAction = [](
			const BOSS_PATTERN_STAGE_DEFINITION& stage,
			const BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			const std::uint32_t value)
		{
			return std::any_of(
				stage.Actions.begin(), stage.Actions.end(),
				[trigger, value](const BOSS_PATTERN_STAGE_ACTION& action)
				{
					return BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG ==
						action.eKind && trigger == action.eTrigger &&
						"boss.flag.groggy" == action.strTargetId &&
						value == action.iValue;
				});
		};
		tests.Require(
			nullptr != dashPattern &&
			BOSS_PATTERN_SELECTION::NORMAL == dashPattern->eSelection &&
			0u != dashPattern->iSelectionWeight &&
			dashPattern->fMaximumRange > 0.f,
			"Keep the armour-opening dash a repeatable weighted pattern with travel distance");
		tests.Require(
			nullptr != dashPattern && 5u == dashPattern->Stages.size() &&
			nullptr != dashCharge && &dashPattern->Stages[1] == dashCharge &&
			420604u == dashPattern->iSourcePrimaryActionId &&
			BOSS_PATTERN_TARGET_POLICY::LOCK_NEAREST_ON_START ==
				dashPattern->eTargetPolicy &&
			BOSS_PATTERN_AIM_POLICY::LOCK_FACING_ON_START ==
				dashPattern->eAimPolicy &&
			BOSS_PATTERN_STAGE_KIND::WINDUP ==
				dashPattern->Stages[0].eStageKind &&
			3650u == dashPattern->Stages[0].iDurationMs &&
			BOSS_PATTERN_HIT_SHAPE::NONE ==
				dashPattern->Stages[0].eHitShape &&
			0u == dashPattern->Stages[0].iHitCount &&
			BOSS_PATTERN_STAGE_MOTION_KIND::NONE ==
				dashPattern->Stages[0].Motion.eKind &&
			1500u == dashCharge->iDurationMs &&
			BOSS_PATTERN_HIT_SHAPE::BOX == dashCharge->eHitShape &&
			std::abs(dashCharge->fHitLength - 10.f) < 1.0e-6f &&
			std::abs(dashCharge->fHitHalfWidth - 2.5f) < 1.0e-6f &&
			1u == dashCharge->iHitCount && 0u == dashCharge->iHitDelayMs &&
			dashCharge->HitOffsetsMs.empty() &&
			"damage.valtan.dash-charge" == dashCharge->strDamageProfileId &&
			std::abs(dashCharge->fPushRangeM - 2.f) < 1.0e-6f &&
			150u == dashCharge->iPushMs && dashCharge->bKnockdown &&
			1000u == dashCharge->iDownMs && dashCharge->bChargeImpact &&
			BOSS_PATTERN_STAGE_MOTION_KIND::FORWARD ==
				dashCharge->Motion.eKind &&
			std::abs(dashCharge->Motion.fDistance - 20.f) < 1.0e-6f &&
			BOSS_PATTERN_STAGE_KIND::GROGGY ==
				dashPattern->Stages[2].eStageKind &&
			5000u == dashPattern->Stages[2].iDurationMs &&
			BOSS_PATTERN_HIT_SHAPE::NONE ==
				dashPattern->Stages[2].eHitShape &&
			0u == dashPattern->Stages[2].iHitCount &&
			BOSS_PATTERN_STAGE_MOTION_KIND::NONE ==
				dashPattern->Stages[2].Motion.eKind &&
			BOSS_PATTERN_PART_DAMAGE_POLICY::DESTROY_FIRST_ELIGIBLE ==
				dashPattern->Stages[2].ePartDamagePolicy &&
			hasDashFlagAction(
				dashPattern->Stages[2],
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 1u) &&
			hasDashFlagAction(
				dashPattern->Stages[2],
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT, 0u) &&
			BOSS_PATTERN_STAGE_KIND::RECOVERY ==
				dashPattern->Stages[3].eStageKind &&
			900u == dashPattern->Stages[3].iDurationMs &&
			BOSS_PATTERN_HIT_SHAPE::NONE ==
				dashPattern->Stages[3].eHitShape &&
			0u == dashPattern->Stages[3].iHitCount &&
			BOSS_PATTERN_STAGE_MOTION_KIND::NONE ==
				dashPattern->Stages[3].Motion.eKind &&
			BOSS_PATTERN_PART_DAMAGE_POLICY::NORMAL ==
				dashPattern->Stages[3].ePartDamagePolicy &&
			BOSS_PATTERN_STAGE_KIND::PART_BREAK ==
				dashPattern->Stages[4].eStageKind,
			"Keep Dash Charge ordered as WINDUP, CHARGE, GROGGY, RECOVERY, and PART_BREAK with part damage confined to GROGGY");
		tests.Require(
			nullptr != dashPattern &&
			2u == dashPattern->Stages[1].Branches.size() &&
			hasDashBranch(dashPattern->Stages[1],
				BOSS_PATTERN_STAGE_OUTCOME::WALL_CONTACT,
				"valtan.attack.dash-charge.groggy") &&
			hasDashBranch(dashPattern->Stages[1],
				BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.attack.dash-charge.recovery") &&
			2u == dashPattern->Stages[2].Branches.size() &&
			hasDashBranch(dashPattern->Stages[2],
				BOSS_PATTERN_STAGE_OUTCOME::PART_DESTROYED,
				"valtan.attack.dash-charge.part-break") &&
			hasDashBranch(dashPattern->Stages[2],
				BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"valtan.attack.dash-charge.recovery") &&
			1u == dashPattern->Stages[3].Branches.size() &&
			hasDashBranch(dashPattern->Stages[3],
				BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT,
				"") &&
			1u == dashPattern->Stages[4].Branches.size() &&
			hasDashBranch(dashPattern->Stages[4],
				BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT, ""),
			"Compile Dash wall-contact GROGGY, timeout RECOVERY, direct part-break, and terminal branches exactly");
	}

	{
		/* A plate coming off is its own beat: the boss plays the source's
		part-destruction reaction instead of the ordinary recovery. The clock must
		never reach that stage, or a charge that hit nothing would play it too. */
		const auto* reactionPatterns =
			catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		std::uint32_t partBreakStageCount = 0u;
		bool everyPartBreakIsLast = true;
		bool everyChargePatternHasOne = true;
		if (nullptr != reactionPatterns)
		{
			for (const BOSS_PATTERN_DEFINITION& pattern : *reactionPatterns)
			{
				std::uint32_t patternPartBreaks = 0u;
				bool hasCharge = false;
				for (std::size_t index = 0; index < pattern.Stages.size(); ++index)
				{
					if (pattern.Stages[index].bChargeImpact)
						hasCharge = true;
					if (BOSS_PATTERN_STAGE_KIND::PART_BREAK !=
						pattern.Stages[index].eStageKind)
					{
						continue;
					}
					++patternPartBreaks;
					++partBreakStageCount;
					if (index + 1u != pattern.Stages.size())
						everyPartBreakIsLast = false;
				}
				if (hasCharge && 1u != patternPartBreaks)
					everyChargePatternHasOne = false;
			}
		}
		tests.Require(
			2u == partBreakStageCount && everyPartBreakIsLast &&
			everyChargePatternHasOne,
			"Author one trailing part-break reaction for each charge pattern");
		tests.Require(
			Is_EventEnteredStage(BOSS_PATTERN_STAGE_KIND::GROGGY) &&
			Is_EventEnteredStage(BOSS_PATTERN_STAGE_KIND::PART_BREAK) &&
			!Is_EventEnteredStage(BOSS_PATTERN_STAGE_KIND::WINDUP) &&
			!Is_EventEnteredStage(BOSS_PATTERN_STAGE_KIND::ACTIVE) &&
			!Is_EventEnteredStage(BOSS_PATTERN_STAGE_KIND::RECOVERY),
			"Keep the stun and the part-break reaction out of the pattern clock");
	}

	{
		/* A weighted pattern can require an armour state, and only a weighted one:
		a scripted health-bar mechanic gated on armour would silently vanish from a
		fight that depends on it. */
		const auto* gatePatterns = catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const BOSS_PATTERN_DEFINITION* dashPattern = nullptr;
		bool everyScriptedPatternIsUngated = true;
		std::uint32_t gatedCount = 0u;
		if (nullptr != gatePatterns)
		{
			for (const BOSS_PATTERN_DEFINITION& pattern : *gatePatterns)
			{
				if (BOSS_PATTERN_ARMOR_REQUIREMENT::ANY !=
					pattern.eArmorRequirement)
				{
					++gatedCount;
					if (BOSS_PATTERN_SELECTION::NORMAL != pattern.eSelection)
						everyScriptedPatternIsUngated = false;
				}
				if (pattern.strPatternId == "VALTAN_DASH_CHARGE")
					dashPattern = &pattern;
			}
		}
		tests.Require(
			nullptr != dashPattern &&
			BOSS_PATTERN_ARMOR_REQUIREMENT::ARMORED ==
				dashPattern->eArmorRequirement &&
			1u == gatedCount && everyScriptedPatternIsUngated,
			"Offer the armour-opening dash only while a plate is still on");

		/* The gate reads live plate state, so the same boss stops meeting an
		ARMORED requirement the moment its last plate breaks, and a boss that
		wears no plates at all reads as stripped rather than as armoured. */
		SERVER_WORLD_ENTITY gateBoss{};
		gateBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		gateBoss.strArchetypeId = "BOSS_VALTAN";
		gateBoss.strEncounterId = "ENCOUNTER_VALTAN";
		const BOSS_RUNTIME_PROFILE* gateProfile =
			catalog.Find_Boss("BOSS_VALTAN");
		if (nullptr != gateProfile)
		{
			for (const BOSS_ARMOR_PLATE& plate : gateProfile->ArmorPlates)
			{
				SERVER_BOSS_ARMOR_PLATE_STATE state{};
				state.iPlateIndex = plate.iPlateIndex;
				state.iDefense = plate.iDefense;
				state.iRemainingDurability = plate.iDurability;
				gateBoss.ArmorPlates.push_back(state);
			}
		}
		SERVER_WORLD_ENTITY halfGateBoss = gateBoss;
		halfGateBoss.ArmorPlates[0].iRemainingDurability = 0u;
		SERVER_WORLD_ENTITY strippedGateBoss = gateBoss;
		for (SERVER_BOSS_ARMOR_PLATE_STATE& plate : strippedGateBoss.ArmorPlates)
			plate.iRemainingDurability = 0u;
		SERVER_WORLD_ENTITY plainGateBoss = gateBoss;
		plainGateBoss.ArmorPlates.clear();
		using ARMOR_REQUIREMENT = BOSS_PATTERN_ARMOR_REQUIREMENT;
		tests.Require(
			2u == gateBoss.ArmorPlates.size() &&
			CValtanBrain::Is_ArmorRequirementMet(
				gateBoss, ARMOR_REQUIREMENT::ARMORED) &&
			CValtanBrain::Is_ArmorRequirementMet(
				halfGateBoss, ARMOR_REQUIREMENT::ARMORED) &&
			!CValtanBrain::Is_ArmorRequirementMet(
				strippedGateBoss, ARMOR_REQUIREMENT::ARMORED) &&
			!CValtanBrain::Is_ArmorRequirementMet(
				plainGateBoss, ARMOR_REQUIREMENT::ARMORED),
			"Meet an ARMORED requirement only while some plate is still on");
		tests.Require(
			!CValtanBrain::Is_ArmorRequirementMet(
				gateBoss, ARMOR_REQUIREMENT::STRIPPED) &&
			CValtanBrain::Is_ArmorRequirementMet(
				strippedGateBoss, ARMOR_REQUIREMENT::STRIPPED) &&
			CValtanBrain::Is_ArmorRequirementMet(
				plainGateBoss, ARMOR_REQUIREMENT::STRIPPED) &&
			CValtanBrain::Is_ArmorRequirementMet(
				gateBoss, ARMOR_REQUIREMENT::ANY) &&
			CValtanBrain::Is_ArmorRequirementMet(
				strippedGateBoss, ARMOR_REQUIREMENT::ANY),
			"Read a boss with no plate left, and one with none authored, as stripped");
		SERVER_WORLD_ENTITY phaseOneBoss{};
		phaseOneBoss.iPhase = 1u;
		SERVER_WORLD_ENTITY phaseTwoBoss{};
		phaseTwoBoss.iPhase = 2u;
		using PHASE_REQUIREMENT = BOSS_PATTERN_PHASE_REQUIREMENT;
		tests.Require(
			CValtanBrain::Is_PhaseRequirementMet(
				phaseOneBoss, PHASE_REQUIREMENT::PHASE_ONE) &&
			!CValtanBrain::Is_PhaseRequirementMet(
				phaseOneBoss, PHASE_REQUIREMENT::PHASE_TWO) &&
			!CValtanBrain::Is_PhaseRequirementMet(
				phaseTwoBoss, PHASE_REQUIREMENT::PHASE_ONE) &&
			CValtanBrain::Is_PhaseRequirementMet(
				phaseTwoBoss, PHASE_REQUIREMENT::PHASE_TWO) &&
			CValtanBrain::Is_PhaseRequirementMet(
				phaseOneBoss, PHASE_REQUIREMENT::ANY) &&
			CValtanBrain::Is_PhaseRequirementMet(
				phaseTwoBoss, PHASE_REQUIREMENT::ANY),
			"Offer a phase-gated pattern only in the phase it names");

		/* Valtan carries no duplicate percent threshold. Exactly one typed edge
		on the 109 central IMPACT owns the phase-two transition. */
		const BOSS_RUNTIME_PROFILE* thresholdProfile =
			catalog.Find_Boss("BOSS_VALTAN");
		const auto* thresholdPatterns =
			catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		std::uint32_t transitionBar = 0u;
		std::size_t phaseActionCount = 0u;
		bool exactPhaseAction = false;
		if (nullptr != thresholdPatterns)
		{
			for (const BOSS_PATTERN_DEFINITION& pattern : *thresholdPatterns)
			{
				if (pattern.strPatternId == "VALTAN_ARENA_BREAK_109")
				{
					transitionBar = pattern.iTriggerHealthBar;
					for (const BOSS_PATTERN_STAGE_DEFINITION& stage : pattern.Stages)
					{
						for (const BOSS_PATTERN_STAGE_ACTION& action : stage.Actions)
						{
							if (BOSS_PATTERN_STAGE_ACTION_KIND::SET_GAMEPLAY_PHASE !=
								action.eKind)
							{
								continue;
							}
							++phaseActionCount;
							exactPhaseAction = "IMPACT" == stage.strStageId &&
								"valtan.mechanic.arena-break-109.impact" ==
									stage.strActionId &&
								BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER ==
									action.eTrigger &&
								"boss.phase.gameplay" == action.strTargetId &&
								2u == action.iValue && 0u == action.iDurationMs;
						}
					}
				}
			}
		}
		tests.Require(
			nullptr != thresholdProfile &&
			BOSS_PHASE_POLICY_KIND::AUTHORED_PATTERN_EVENT ==
				thresholdProfile->PhasePolicy.eKind &&
			0u == thresholdProfile->PhasePolicy.iThresholdPercent &&
			109u == transitionBar && 1u == phaseActionCount && exactPhaseAction,
			"Advance Valtan to phase two only on 109 IMPACT ENTER");

		/* The wipe and both authored terrain-destruction auditions are protected
		while they run. The wipe still fires on the bar its own name carries. */
		const BOSS_PATTERN_DEFINITION* wipePattern = nullptr;
		const BOSS_PATTERN_DEFINITION* terrainThreePattern = nullptr;
		const BOSS_PATTERN_DEFINITION* terrainNinePattern = nullptr;
		std::uint32_t invulnerableCount = 0u;
		if (nullptr != thresholdPatterns)
		{
			for (const BOSS_PATTERN_DEFINITION& pattern : *thresholdPatterns)
			{
				if (pattern.bInvulnerableWhileRunning)
					++invulnerableCount;
				if (pattern.strPatternId == "VALTAN_FLOOR_WIPE_130")
					wipePattern = &pattern;
				else if (pattern.strPatternId ==
					"VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK")
					terrainThreePattern = &pattern;
				else if (pattern.strPatternId ==
					"VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK")
					terrainNinePattern = &pattern;
			}
		}
		tests.Require(
			nullptr != wipePattern && wipePattern->bInvulnerableWhileRunning &&
			nullptr != terrainThreePattern &&
			terrainThreePattern->bInvulnerableWhileRunning &&
			nullptr != terrainNinePattern &&
			terrainNinePattern->bInvulnerableWhileRunning &&
			130u == wipePattern->iTriggerHealthBar && 4u == invulnerableCount,
			"Protect the entrance camera gate, 130-bar wipe, and both terrain-destruction auditions");

		/* Its second smash is arena wide and lethal to any authored player pool,
		which is what makes it a wipe rather than a large hit. */
		const std::uint32_t wipeRate = catalog.Find_DamageRatePercent(
			"damage.valtan.omnidirectional-wipe-130");
		const BOSS_RUNTIME_PROFILE* wipeBoss = catalog.Find_Boss("BOSS_VALTAN");
		std::uint32_t toughestPlayerHp = 0u;
		for (const CHARACTER_CLASS_ID characterClass : {
				CHARACTER_CLASS_ID::LANCE_MASTER,
				CHARACTER_CLASS_ID::GUNSLINGER,
				CHARACTER_CLASS_ID::SLAYER,
				CHARACTER_CLASS_ID::ARTIST,
				CHARACTER_CLASS_ID::DIMENSIONMASTER })
		{
			const PLAYER_RUNTIME_PROFILE* playerProfile =
				catalog.Find_Player(characterClass);
			if (nullptr != playerProfile)
			{
				toughestPlayerHp = (std::max)(
					toughestPlayerHp, playerProfile->iMaximumHp);
			}
		}
		const BOSS_PATTERN_STAGE_DEFINITION* wipeSmash = nullptr;
		if (nullptr != wipePattern)
		{
			for (const BOSS_PATTERN_STAGE_DEFINITION& stage : wipePattern->Stages)
			{
				if (stage.strDamageProfileId ==
					"damage.valtan.omnidirectional-wipe-130")
				{
					wipeSmash = &stage;
				}
			}
		}
		tests.Require(
			nullptr != wipeBoss && nullptr != wipeSmash && 0u != wipeRate &&
			0u != toughestPlayerHp &&
			BOSS_PATTERN_HIT_SHAPE::CIRCLE == wipeSmash->eHitShape &&
			wipeSmash->fHitOuterRadius >= 100.f &&
			CGameplayCatalog::Resolve_Damage(wipeBoss->iAttackPower, wipeRate) >
				toughestPlayerHp,
			"Reach the whole arena with a wipe hit that outdamages the toughest player pool");
	}
	{
		SERVER_PLAYER missPlayer{};
		missPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		missPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		missPlayer.iCurrentResource = 1000;
		missPlayer.iMaximumResource = 1000;
		missPlayer.fPositionX = 151.f;
		missPlayer.fPositionY = 22.97f;
		missPlayer.fPositionZ = -129.f;
		std::vector<SERVER_WORLD_ENTITY> behindEntities{ boss };
		behindEntities[0].fPositionZ = missPlayer.fPositionZ - 3.5f;
		C2S_USE_SKILL forwardSkill = useSkill;
		forwardSkill.iClientSequence = 3;
		CPlayerSkillSystem missSkills;
		std::vector<DAMAGE_EVENT> missEvents;
		tests.Require(missSkills.Try_Start(missPlayer, forwardSkill, catalog, 100),
			"Approve skill aimed away from the target behind the caster");
		for (std::uint32_t tick = 101; tick < 160; ++tick)
			missSkills.Update(missPlayer, behindEntities, catalog, &navigation, nullptr,
				1.f / 30.f, tick, missEvents);
		tests.Require(10000u == behindEntities[0].iCurrentHp && missEvents.empty(),
			"Leave a target outside the authored hit shape untouched");
	}
	{
		/* Knockback: only hits the source authored with a push move a monster,
		scaled by its profile; the slide runs over the hit's own push window,
		a zero-scale monster never moves, and a wall ends the slide early. */
		const auto makeMonster = [&boss](const float knockbackScale)
		{
			SERVER_WORLD_ENTITY monster{};
			monster.iNetEntityId = 910u;
			monster.eKind = WORLD_BOOTSTRAP_KIND::MONSTER;
			monster.eAction = SERVER_ENTITY_ACTION::IDLE;
			monster.strArchetypeId = "MONSTER_VALTAN_PADD_01";
			monster.iCurrentHp = 100000;
			monster.iMaximumHp = 100000;
			/* 1.5 m in front of the attacker: inside 34120's 2.2 m and 34540's
			3.2 m shapes even with a small monster body. */
			monster.fPositionX = 151.f;
			monster.fPositionY = boss.fPositionY;
			monster.fPositionZ = -127.5f;
			monster.fCollisionRadius = 0.6f;
			monster.fHitKnockbackScale = knockbackScale;
			return monster;
		};
		const auto hitWith = [&](const SKILL_ID skillId,
			const PLAYER_STANCE_ID stance,
			std::vector<SERVER_WORLD_ENTITY>& targets,
			std::vector<DAMAGE_EVENT>& events)
		{
			SERVER_PLAYER attacker{};
			attacker.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			attacker.eStance = stance;
			attacker.iCurrentResource = 1000;
			attacker.iMaximumResource = 1000;
			attacker.fPositionX = 151.f;
			attacker.fPositionY = 22.97f;
			attacker.fPositionZ = -129.f;
			C2S_USE_SKILL knockSkill = useSkill;
			knockSkill.iSkillId = skillId;
			knockSkill.iClientSequence = 5;
			CPlayerSkillSystem knockSkills;
			if (!knockSkills.Try_Start(attacker, knockSkill, catalog, 200))
				return false;
			for (std::uint32_t tick = 201; tick < 260; ++tick)
				knockSkills.Update(attacker, targets, catalog, &navigation, nullptr,
					1.f / 30.f, tick, events);
			return true;
		};
		/* 34540 (short spear Q) is authored push=130 ms, pushr=1.0 m on its one
		hit; 34120's three hits are authored push=0. */
		const PLAYER_SKILL_DEFINITION* pushSkill = catalog.Find_Skill(34540);
		tests.Require(
			nullptr != pushSkill && 1u == pushSkill->Hits.size() &&
			130u == pushSkill->Hits[0].iPushMs &&
			std::fabs(pushSkill->Hits[0].fPushRange - 1.f) < 0.0001f &&
			nullptr != talonStrike && 0u == talonStrike->Hits[0].iPushMs &&
			0.f == talonStrike->Hits[0].fPushRange,
			"Load the authored push window and range per hit shape from the gameplay bootstrap");

		std::vector<SERVER_WORLD_ENTITY> unpushed{ makeMonster(1.f) };
		std::vector<DAMAGE_EVENT> unpushedEvents;
		const bool unpushedStarted = hitWith(34120,
			PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR, unpushed, unpushedEvents);
		tests.Require(
			unpushedStarted && !unpushedEvents.empty() &&
			0.f == unpushed[0].fKnockbackRemainingSeconds,
			"Leave a monster unmoved by hits the source authored without a push");

		std::vector<SERVER_WORLD_ENTITY> pushed{ makeMonster(1.f) };
		std::vector<DAMAGE_EVENT> pushedEvents;
		const bool pushedStarted = hitWith(34540,
			PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR, pushed, pushedEvents);
		tests.Require(
			pushedStarted && !pushedEvents.empty() &&
			std::fabs(pushed[0].fKnockbackRemainingSeconds - 0.13f) < 0.0001f &&
			std::fabs(pushed[0].fKnockbackDirectionX) < 0.001f &&
			pushed[0].fKnockbackDirectionZ > 0.999f,
			"Arm a knockback away from the attacker on an authored push hit");
		const float startX = pushed[0].fPositionX;
		const float startZ = pushed[0].fPositionZ;
		std::uint32_t knockbackTicks = 0;
		while (CMonsterBrain::Advance_Knockback(pushed[0], navigation, 1.f / 30.f))
		{
			++knockbackTicks;
			if (knockbackTicks > 30u)
				break;
		}
		tests.Require(
			4u == knockbackTicks &&
			0.f == pushed[0].fKnockbackRemainingSeconds &&
			std::fabs(pushed[0].fPositionX - startX) < 0.01f &&
			std::fabs((pushed[0].fPositionZ - startZ) - 1.f) < 0.01f &&
			SERVER_ENTITY_ACTION::IDLE == pushed[0].eAction,
			"Slide the monster the authored push range over the push window and stop");

		std::vector<SERVER_WORLD_ENTITY> immune{ makeMonster(0.f) };
		std::vector<DAMAGE_EVENT> immuneEvents;
		const bool immuneStarted = hitWith(34540,
			PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR, immune, immuneEvents);
		tests.Require(
			immuneStarted && !immuneEvents.empty() &&
			0.f == immune[0].fKnockbackRemainingSeconds &&
			!CMonsterBrain::Advance_Knockback(immune[0], navigation, 1.f / 30.f) &&
			151.f == immune[0].fPositionX &&
			-127.5f == immune[0].fPositionZ,
			"Leave a zero-scale monster in place when an authored push hit lands");

		SERVER_WORLD_ENTITY walled = makeMonster(1.f);
		walled.fPositionX = lastWalkableX - navCellSize * 0.5f;
		walled.fPositionZ = boundaryProbeZ;
		walled.fKnockbackDirectionX = 1.f;
		walled.fKnockbackDirectionZ = 0.f;
		walled.fKnockbackSpeed = 4.f / 0.13f;
		walled.fKnockbackRemainingSeconds = 0.13f;
		std::uint32_t walledTicks = 0;
		while (CMonsterBrain::Advance_Knockback(walled, navigation, 1.f / 30.f))
		{
			++walledTicks;
			if (walledTicks > 30u)
				break;
		}
		SERVER_NAV_POINT walledPoint{};
		tests.Require(
			walledTicks >= 1u && walledTicks < 5u &&
			0.f == walled.fKnockbackRemainingSeconds &&
			walled.fPositionX < firstBlockedX &&
			navigation.Sample_Position(walled.fPositionX, walled.fPositionZ, walledPoint),
			"Stop a knockback at the non-walkable boundary and end the window");
	}
	{
		/* Player hit reaction: an authored boss/monster push arms away from the
		hit source over its own window, a negative range pulls toward it, a
		FallDown hit holds KNOCKDOWN until downMs, a running window never
		re-arms, and no command passes while the player is down. */
		SERVER_PLAYER victim{};
		victim.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		victim.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		victim.iCurrentHp = 5000;
		victim.iMaximumHp = 5000;
		victim.iCurrentResource = 1000;
		victim.iMaximumResource = 1000;
		victim.fPositionX = 10.f;
		victim.fPositionZ = 5.f;
		CPlayerSkillSystem::Arm_PlayerHitReaction(
			victim, 10.f, 3.f, 2.f, 242u, false, 0u, 300u);
		tests.Require(
			victim.fKnockbackDirectionZ > 0.999f &&
			std::fabs(victim.fKnockbackDirectionX) < 0.001f &&
			std::fabs(victim.fKnockbackRemainingSeconds - 0.242f) < 0.0001f &&
			std::fabs(victim.fKnockbackSpeed - 2.f / 0.242f) < 0.001f &&
			PLAYER_ACTION_STATE::NONE == victim.eAction,
			"Arm a player push away from the hit source without a knockdown");
		CPlayerSkillSystem::Arm_PlayerHitReaction(
			victim, 10.f, 7.f, 2.f, 242u, true, 2000u, 301u);
		tests.Require(
			victim.fKnockbackDirectionZ > 0.999f &&
			PLAYER_ACTION_STATE::NONE == victim.eAction,
			"Keep a running push window from being re-armed by a second hit");
		victim.fKnockbackRemainingSeconds = 0.f;
		victim.fKnockbackSpeed = 0.f;
		CPlayerSkillSystem::Arm_PlayerHitReaction(
			victim, 10.f, 3.f, -2.4f, 97u, true, 2000u, 310u);
		tests.Require(
			victim.fKnockbackDirectionZ < -0.999f &&
			std::fabs(victim.fKnockbackSpeed - 2.4f / 0.097f) < 0.001f &&
			PLAYER_ACTION_STATE::KNOCKDOWN == victim.eAction &&
			310u == victim.iActionStartTick &&
			370u == victim.iKnockdownEndTick &&
			LostArk::Shared::INVALID_SKILL_ID == victim.iCurrentSkillId,
			"Pull the player toward the hit source and hold the authored knockdown");
		C2S_USE_SKILL downSkill{};
		downSkill.iClientSequence = 9;
		downSkill.iSkillId = 34120;
		downSkill.fAimX = 10.f;
		downSkill.fAimZ = 50.f;
		CPlayerSkillSystem downSkills;
		tests.Require(
			!downSkills.Try_Start(victim, downSkill, catalog, 315u),
			"Reject a skill command while the player is knocked down");

		const PLAYER_SKILL_DEFINITION* standup = catalog.Find_Skill(34030);
		tests.Require(
			nullptr != standup &&
			PLAYER_SKILL_KIND::STANDUP == standup->eSkillKind &&
			3000u == standup->iCooldownMs &&
			!standup->RootMotion.empty() &&
			standup->RootMotion.back().fForward > 3.5f,
			"Load the official stand-up skill with its rolling root motion");
		C2S_USE_SKILL standupCommand{};
		standupCommand.iClientSequence = 10;
		standupCommand.iSkillId = 34030;
		standupCommand.fAimX = victim.fPositionX;
		standupCommand.fAimZ = victim.fPositionZ - 5.f;
		tests.Require(
			downSkills.Try_Start(victim, standupCommand, catalog, 320u) &&
			PLAYER_ACTION_STATE::SKILL == victim.eAction &&
			34030u == victim.iCurrentSkillId &&
			0u == victim.iComboStage &&
			0u == victim.iKnockdownEndTick &&
			0.f == victim.fKnockbackRemainingSeconds,
			"Stand the knocked-down player up through the STANDUP skill");

		SERVER_PLAYER standing{};
		standing.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		standing.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		standing.iCurrentHp = 5000;
		standing.iMaximumHp = 5000;
		standing.iCurrentResource = 1000;
		standing.iMaximumResource = 1000;
		C2S_USE_SKILL standingStandup = standupCommand;
		standingStandup.iClientSequence = 1;
		CPlayerSkillSystem standingSkills;
		tests.Require(
			!standingSkills.Try_Start(standing, standingStandup, catalog, 330u),
			"Reject the STANDUP skill while the player is on its feet");

		victim.eAction = PLAYER_ACTION_STATE::NONE;
		victim.iCurrentSkillId = LostArk::Shared::INVALID_SKILL_ID;
		CPlayerSkillSystem::Arm_PlayerHitReaction(
			victim, 10.f, 3.f, 2.f, 242u, true, 2000u, 330u);
		tests.Require(
			PLAYER_ACTION_STATE::NONE == victim.eAction &&
			0.f == victim.fKnockbackRemainingSeconds &&
			380u == victim.iHitReactionGraceEndTick,
			"Hold the get-up grace so a hit cannot chain a second knockdown");
		CPlayerSkillSystem::Arm_PlayerHitReaction(
			victim, 10.f, 3.f, 2.f, 242u, true, 2000u, 385u);
		tests.Require(
			PLAYER_ACTION_STATE::KNOCKDOWN == victim.eAction &&
			victim.fKnockbackRemainingSeconds > 0.f,
			"Arm the hit reaction again once the get-up grace has passed");

		const auto* valtanPatterns = catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const BOSS_PATTERN_DEFINITION* swing = nullptr;
		const BOSS_PATTERN_DEFINITION* roar = nullptr;
		if (nullptr != valtanPatterns)
		{
			for (const BOSS_PATTERN_DEFINITION& pattern : *valtanPatterns)
			{
				if ("VALTAN_SWING" == pattern.strPatternId)
					swing = &pattern;
				if ("VALTAN_IMPRISON_ROAR" == pattern.strPatternId)
					roar = &pattern;
			}
		}
		const auto findActivePush = [](const BOSS_PATTERN_DEFINITION* pattern)
			-> const BOSS_PATTERN_STAGE_DEFINITION*
		{
			if (nullptr == pattern)
				return nullptr;
			for (const BOSS_PATTERN_STAGE_DEFINITION& stage : pattern->Stages)
			{
				if (!stage.strDamageProfileId.empty())
					return &stage;
			}
			return nullptr;
		};
		const BOSS_PATTERN_STAGE_DEFINITION* swingHit = findActivePush(swing);
		const BOSS_PATTERN_STAGE_DEFINITION* roarHit = findActivePush(roar);
		tests.Require(
			nullptr != swingHit && nullptr != roarHit &&
			std::fabs(swingHit->fPushRangeM - 2.f) < 0.0001f &&
			242u == swingHit->iPushMs && swingHit->bKnockdown &&
			2000u == swingHit->iDownMs &&
			roarHit->fPushRangeM < 0.f && roarHit->bKnockdown,
			"Load the official swing push and the imprison-roar pull from the encounter");
	}
	{
		/* Projectiles: the Artist tiger (31490, official 314900) is a missile
		that leaves the caster 1147 ms in, runs 11 m at 6.5 m/s and carries a
		1.5 x 2.5 m box; the DimensionMaster nail (2050100, 20501001) is a
		fixed area that stands 0.8 m in front of the caster (AreaOrigin 0,
		AreaOffsetX 80) and fires its 1 m circle the moment it appears. */
		const PLAYER_SKILL_DEFINITION* tigerSkill = catalog.Find_Skill(31490);
		tests.Require(
			nullptr != tigerSkill && 1u == tigerSkill->Projectiles.size() &&
			PLAYER_PROJECTILE_KIND::MISSILE == tigerSkill->Projectiles[0].eKind &&
			1147u == tigerSkill->Projectiles[0].iTimeMs &&
			std::fabs(tigerSkill->Projectiles[0].fSpeed - 6.5f) < 0.001f &&
			std::fabs(tigerSkill->Projectiles[0].fMaxDistance - 11.f) < 0.001f &&
			3000u == tigerSkill->Projectiles[0].iLifeMs &&
			1u == tigerSkill->Projectiles[0].Hits.size() &&
			tigerSkill->Projectiles[0].Hits[0].isContact &&
			2u == tigerSkill->Projectiles[0].Hits[0].Hit.iAreaType,
			"Load the authored missile definition of a skill from the gameplay bootstrap");

		const auto makeArtist = []()
		{
			SERVER_PLAYER artist{};
			artist.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
			artist.iCurrentHp = 1000;
			artist.iMaximumHp = 1000;
			artist.iCurrentResource = 1000;
			artist.iMaximumResource = 1000;
			artist.iCurrentIdentity = 1000;
			artist.iMaximumIdentity = 1000;
			artist.fPositionX = 151.f;
			artist.fPositionY = 22.97f;
			artist.fPositionZ = -129.f;
			return artist;
		};
		const auto makeTargetAt = [&boss](const float distanceAhead)
		{
			SERVER_WORLD_ENTITY target = boss;
			target.iNetEntityId = 930u;
			target.iCurrentHp = 100000;
			target.iMaximumHp = 100000;
			target.fPositionZ = -129.f + distanceAhead;
			return target;
		};

		/* A target 8 m out with a 3 m body: the box (1.5 m long) reaches it
		once the tiger has run 3.5 m, at 1147 + 3500 / 6.5 = 1685 ms. */
		SERVER_PLAYER tigerCaster = makeArtist();
		std::vector<SERVER_WORLD_ENTITY> tigerTargets{ makeTargetAt(8.f) };
		C2S_USE_SKILL tigerCommand{};
		tigerCommand.iClientSequence = 1;
		tigerCommand.iSkillId = 31490;
		tigerCommand.fAimX = tigerCaster.fPositionX;
		tigerCommand.fAimZ = tigerCaster.fPositionZ + 8.f;
		CPlayerSkillSystem tigerSkills;
		std::vector<DAMAGE_EVENT> tigerEvents;
		tests.Require(tigerSkills.Try_Start(tigerCaster, tigerCommand, catalog, 10),
			"Approve the tiger skill aimed past a distant target");
		std::uint32_t firstHitTick = 0;
		for (std::uint32_t tick = 11; tick < 200; ++tick)
		{
			tigerSkills.Update(tigerCaster, tigerTargets, catalog, &navigation, nullptr,
				1.f / 30.f, tick, tigerEvents);
			if (0u == firstHitTick && !tigerEvents.empty())
				firstHitTick = tick;
			if (tick == 50u)
			{
				tests.Require(
					tigerEvents.empty() && 1u == tigerCaster.Projectiles.size(),
					"Spawn the missile at its authored time and land nothing before it arrives");
			}
		}
		/* tick 10 = action start, spawn on tick 45 (elapsed 1.167 s), then 17
		moves of 0.217 m to pass 3.5 m: tick 62, give or take one tick of float
		accumulation. */
		tests.Require(
			1u == tigerEvents.size() && firstHitTick >= 61u && firstHitTick <= 63u &&
			tigerTargets[0].iNetEntityId == tigerEvents[0].iTargetNetEntityId,
			"Land the missile's contact hit once when its box reaches the target");
		const std::uint32_t tigerRate = catalog.Find_DamageRatePercent(
			tigerSkill->strDamageProfileId);
		const PLAYER_RUNTIME_PROFILE* artistProfile =
			catalog.Find_Player(CHARACTER_CLASS_ID::ARTIST);
		const std::uint32_t tigerTotal = CGameplayCatalog::Resolve_Damage(
			nullptr == artistProfile ? 0u : artistProfile->iAttackPower, tigerRate);
		/* One caster hit and one missile hit share the rate: the missile lands
		the second half. */
		tests.Require(
			tigerEvents[0].iAmount == tigerTotal - tigerTotal / 2u &&
			100000u - tigerEvents[0].iAmount == tigerTargets[0].iCurrentHp,
			"Split the skill rate between the caster hit and the missile hit");
		tests.Require(
			tigerCaster.Projectiles.empty() &&
			PLAYER_ACTION_STATE::NONE == tigerCaster.eAction,
			"Retire the missile after its authored distance while the caster is already free");

		/* Behind the caster the tiger never turns: no hit, and the target keeps
		its HP after the whole flight. */
		SERVER_PLAYER missCaster = makeArtist();
		std::vector<SERVER_WORLD_ENTITY> missTargets{ makeTargetAt(-8.f) };
		CPlayerSkillSystem missSkills;
		std::vector<DAMAGE_EVENT> missEvents;
		tests.Require(missSkills.Try_Start(missCaster, tigerCommand, catalog, 10),
			"Approve the tiger skill aimed away from a target behind the caster");
		for (std::uint32_t tick = 11; tick < 200; ++tick)
			missSkills.Update(missCaster, missTargets, catalog, &navigation, nullptr,
				1.f / 30.f, tick, missEvents);
		tests.Require(missEvents.empty() && 100000u == missTargets[0].iCurrentHp,
			"Leave a target outside the missile's path untouched");

		const PLAYER_SKILL_DEFINITION* nailSkill = catalog.Find_Skill(2050100);
		tests.Require(
			nullptr != nailSkill && 1u == nailSkill->Projectiles.size() &&
			PLAYER_PROJECTILE_KIND::FIXAREA == nailSkill->Projectiles[0].eKind &&
			PLAYER_PROJECTILE_ORIGIN::CASTER == nailSkill->Projectiles[0].eOrigin &&
			std::fabs(nailSkill->Projectiles[0].fOffsetForward - 0.8f) < 0.001f &&
			280u == nailSkill->Projectiles[0].iTimeMs &&
			!nailSkill->Projectiles[0].Hits[0].isContact &&
			0u == nailSkill->Projectiles[0].Hits[0].Hit.iTimeMs,
			"Load the authored fixed-area definition of a skill from the gameplay bootstrap");
		SERVER_PLAYER nailCaster = makeArtist();
		nailCaster.eCharacterClass = CHARACTER_CLASS_ID::DIMENSIONMASTER;
		/* Aim 12 m out: the area still stands 0.8 m ahead, where a 3 m body
		centred at 4 m touches its 1 m circle while a body at 9.5 m does not. */
		std::vector<SERVER_WORLD_ENTITY> nailTargets{ makeTargetAt(4.f), makeTargetAt(9.5f) };
		nailTargets[1].iNetEntityId = 931u;
		C2S_USE_SKILL nailCommand{};
		nailCommand.iClientSequence = 1;
		nailCommand.iSkillId = 2050100;
		nailCommand.fAimX = nailCaster.fPositionX;
		nailCommand.fAimZ = nailCaster.fPositionZ + 12.f;
		CPlayerSkillSystem nailSkills;
		std::vector<DAMAGE_EVENT> nailEvents;
		tests.Require(nailSkills.Try_Start(nailCaster, nailCommand, catalog, 10),
			"Approve the nail skill aimed beyond the area's reach");
		for (std::uint32_t tick = 11; tick < 40; ++tick)
			nailSkills.Update(nailCaster, nailTargets, catalog, &navigation, nullptr,
				1.f / 30.f, tick, nailEvents);
		tests.Require(
			1u == nailEvents.size() &&
			nailTargets[0].iNetEntityId == nailEvents[0].iTargetNetEntityId &&
			100000u == nailTargets[1].iCurrentHp,
			"Drop the fixed area at the caster's authored offset and fire its timed hit there");
	}
	C2S_USE_SKILL cooldownAttempt = useSkill;
	cooldownAttempt.iClientSequence = 2;
	tests.Require(!skills.Try_Start(player, cooldownAttempt, catalog, 70),
		"Reject skill during authoritative cooldown");

	{
		const PLAYER_SKILL_DEFINITION* combo = catalog.Find_Skill(34010);
		tests.Require(
			nullptr != combo &&
			PLAYER_SKILL_KIND::COMBO == combo->eSkillKind &&
			4u == combo->ComboStages.size() &&
			0u == combo->ComboStages[3].iInputCloseMs,
			"Resolve LanceMaster basic attack combo stages");

		SERVER_PLAYER comboPlayer{};
		comboPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		comboPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		comboPlayer.iCurrentHp = 1000;
		comboPlayer.iMaximumHp = 1000;
		comboPlayer.iCurrentResource = 100;
		comboPlayer.iMaximumResource = 100;
		std::vector<SERVER_WORLD_ENTITY> comboEntities;
		std::vector<DAMAGE_EVENT> comboDamageEvents;
		CPlayerSkillSystem comboSkills;

		C2S_USE_SKILL press{};
		press.iClientSequence = 1;
		press.iSkillId = 34010;
		press.fAimX = 1.f;
		press.fAimZ = 0.f;
		tests.Require(
			comboSkills.Try_Start(comboPlayer, press, catalog, 10) &&
			1u == comboPlayer.iComboStage,
			"Approve basic attack first stage");

		// 329ms is where stage one opens; 100ms is deliberately before it.
		comboPlayer.fActionElapsedSeconds = 0.1f;
		press.iClientSequence = 2;
		comboSkills.Try_Start(comboPlayer, press, catalog, 12);
		tests.Require(!comboPlayer.hasBufferedComboInput,
			"Reject combo input before the window opens");

		comboPlayer.fActionElapsedSeconds = 0.4f;
		press.iClientSequence = 3;
		comboSkills.Try_Start(comboPlayer, press, catalog, 14);
		tests.Require(comboPlayer.hasBufferedComboInput,
			"Buffer combo input inside the window");

		press.iClientSequence = 4;
		comboSkills.Try_Start(comboPlayer, press, catalog, 15);
		tests.Require(1u == comboPlayer.iComboStage,
			"Ignore a second press inside the same window");

		C2S_USE_SKILL other{};
		other.iClientSequence = 5;
		other.iSkillId = 34120;
		other.fAimX = 1.f;
		other.fAimZ = 0.f;
		tests.Require(
			!comboSkills.Try_Start(comboPlayer, other, catalog, 16) &&
			34010u == comboPlayer.iCurrentSkillId,
			"Reject a different skill during a combo");

		/* This legacy stage authors comboAdvanceMs at its 470 ms hit, preserving
		the established cadence while the new field lets other stages keep their
		presentation longer. Twenty ticks is about 667 ms, past that boundary. */
		for (std::uint32_t tick = 17; tick < 37; ++tick)
			comboSkills.Update(comboPlayer, comboEntities, catalog, nullptr,
				nullptr, 1.f / 30.f, tick, comboDamageEvents);
		tests.Require(
			2u == comboPlayer.iComboStage &&
			PLAYER_ACTION_STATE::SKILL == comboPlayer.eAction,
			"Advance at the authored legacy combo boundary");

		/* Nothing is buffered now, so stage two has to run its whole 1367 ms
		instead of cutting at its hit. */
		for (std::uint32_t tick = 37; tick < 57; ++tick)
			comboSkills.Update(comboPlayer, comboEntities, catalog, nullptr,
				nullptr, 1.f / 30.f, tick, comboDamageEvents);
		tests.Require(
			2u == comboPlayer.iComboStage &&
			PLAYER_ACTION_STATE::SKILL == comboPlayer.eAction,
			"Hold the stage past its hit when no press was buffered");

		for (std::uint32_t tick = 57; tick < 120; ++tick)
			comboSkills.Update(comboPlayer, comboEntities, catalog, nullptr,
				nullptr, 1.f / 30.f, tick, comboDamageEvents);
		tests.Require(
			PLAYER_ACTION_STATE::NONE == comboPlayer.eAction &&
			0u == comboPlayer.iComboStage,
			"End the combo when no press was buffered");
	}

	std::map<PLAYER_ID, SERVER_PLAYER> players;
	SERVER_PLAYER target{};
	target.iPlayerId = 1;
	target.iNetEntityId = 100;
	target.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
	target.iCurrentHp = 1000;
	target.iMaximumHp = 1000;
	target.fPositionX = 151.f;
	target.fPositionY = 22.97f;
	target.fPositionZ = -128.f;
	target.isCombatReady = false;
	players.emplace(target.iPlayerId, target);
	SERVER_WORLD_ENTITY valtan{};
	valtan.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
	valtan.eAction = SERVER_ENTITY_ACTION::IDLE;
	/* The brain resolves damage through the boss's own catalog profile, so the
	test entity carries the archetype the room would have stamped on it. */
	valtan.strArchetypeId = "BOSS_VALTAN";
	valtan.strEncounterId = "ENCOUNTER_VALTAN";
	valtan.iCurrentHp = 48750;
	valtan.iMaximumHp = 60000;
	valtan.iMaximumHealthBars = 160;
	valtan.iLastEvaluatedHealthBar = 131;
	valtan.iPhaseTwoHpPercent = 50;
	valtan.iPhase = 1;
	valtan.fPositionX = 151.f;
	valtan.fPositionY = 22.97f;
	valtan.fPositionZ = -122.f;
	valtan.fEngageDistance = 35.f;
	valtan.fMoveSpeed = 3.f;
	/* This fixture asserts the observed 130-bar mechanic, so the encounter intro is
	staged as already consumed exactly as a Debug audition reset does. */
	valtan.bIntroPatternConsumed = true;
	valtan.bAutomaticPatternSequenceAuditionOverride = true;
	CValtanBrain brain;
	std::vector<DAMAGE_EVENT> valtanDamageEvents;
	brain.Update(valtan, players, catalog, navigation, 1.f / 30.f, 99,
		{}, valtanDamageEvents);
	tests.Require(
		SERVER_ENTITY_ACTION::IDLE == valtan.eAction &&
		1000u == players.begin()->second.iCurrentHp &&
		valtanDamageEvents.empty(),
		"Protect Valtan entrant until first accepted gameplay intent");
	const VALTAN_DECISION_TRACE* noTargetTrace =
		brain.Get_LatestDecisionTrace();
	const bool tracedNoTargetCandidates = nullptr != noTargetTrace &&
		std::any_of(
			noTargetTrace->Candidates.begin(), noTargetTrace->Candidates.end(),
			[](const VALTAN_DECISION_CANDIDATE_TRACE& candidate)
			{
				return 0u != (candidate.iExclusionMask &
					VALTAN_EXCLUDE_NO_TARGET);
			});
	tests.Require(
		nullptr != noTargetTrace &&
		VALTAN_DECISION_RESULT::NO_VALID_TARGET == noTargetTrace->eResult &&
		VALTAN_DECISION_SOURCE::NONE == noTargetTrace->eSource &&
		tracedNoTargetCandidates,
		"Record no-target candidate exclusions in the selector decision envelope");
	players.begin()->second.isCombatReady = true;
	for (std::uint32_t tick = 100; tick < 170 && valtanDamageEvents.empty(); ++tick)
		brain.Update(valtan, players, catalog, navigation, 1.f / 30.f, tick,
			{}, valtanDamageEvents);
	tests.Require(781u == players.begin()->second.iCurrentHp,
		"Apply the queued 130-bar Valtan six-direction hit once");
	tests.Require(
		1u == valtanDamageEvents.size() &&
		219u == valtanDamageEvents[0].iAmount &&
		!valtanDamageEvents[0].isOutgoing &&
		players.begin()->second.iNetEntityId ==
			valtanDamageEvents[0].iTargetNetEntityId,
		"Emit one incoming damage event for the 130-bar boss hit");
	tests.Require(
		"VALTAN_FLOOR_WIPE_130" == valtan.strPatternId &&
		valtan.PendingPatternIds.empty() &&
		1u == valtan.TriggeredPatternIds.size() &&
		1u == valtan.iPatternSequence &&
		1u == valtan.iPatternStageIndex,
		"Queue and advance the staged 130-bar scripted mechanic");
	const VALTAN_DECISION_TRACE* forcedHealthTrace =
		brain.Get_LatestDecisionTrace();
	const auto floorWipeOccurrence = std::find_if(
		valtan.MechanicOccurrences.begin(), valtan.MechanicOccurrences.end(),
		[](const SERVER_BOSS_MECHANIC_OCCURRENCE& occurrence)
		{
			return "VALTAN_FLOOR_WIPE_130" == occurrence.strPatternId;
		});
	tests.Require(
		nullptr != forcedHealthTrace &&
		VALTAN_DECISION_SOURCE::FORCED_HEALTH_BAR ==
			forcedHealthTrace->eSource &&
		VALTAN_DECISION_RESULT::SELECTED == forcedHealthTrace->eResult &&
		"VALTAN_FLOOR_WIPE_130" ==
			forcedHealthTrace->strSelectedPatternId &&
		"VALTAN_FLOOR_WIPE_130" ==
			forcedHealthTrace->strPendingPatternId &&
		1u == forcedHealthTrace->iExpectedPatternSequence &&
		valtan.MechanicOccurrences.end() != floorWipeOccurrence &&
		SERVER_BOSS_MECHANIC_STATE::ACTIVE == floorWipeOccurrence->eState &&
		1u == floorWipeOccurrence->iPatternSequence,
		"Trace forced health selection and advance its stable ledger occurrence");
	valtan.iCurrentHp = 30000;
	brain.Update(valtan, players, catalog, navigation, 1.f / 30.f, 171,
		{}, valtanDamageEvents);
	tests.Require(1u == valtan.iPhase,
		"Keep Valtan phase one until the authored 109 IMPACT ENTER edge");
	CValtanBrain::Fail_Mechanic(
		valtan, "VALTAN_FLOOR_WIPE_130",
		SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_PREFLIGHT, 172u);
	const std::uint32_t failedMechanicSequence = valtan.iPatternSequence;
	brain.Update(valtan, players, catalog, navigation, 1.f / 30.f, 173u,
		{}, valtanDamageEvents);
	const VALTAN_DECISION_TRACE* resetRequiredTrace =
		brain.Get_LatestDecisionTrace();
	const auto failedFloorWipeOccurrence = std::find_if(
		valtan.MechanicOccurrences.begin(), valtan.MechanicOccurrences.end(),
		[](const SERVER_BOSS_MECHANIC_OCCURRENCE& occurrence)
		{
			return "VALTAN_FLOOR_WIPE_130" == occurrence.strPatternId;
		});
	tests.Require(
		valtan.bMechanicLedgerRequiresReset &&
		valtan.MechanicOccurrences.end() != failedFloorWipeOccurrence &&
		SERVER_BOSS_MECHANIC_STATE::FAILED_REQUIRES_RESET ==
			failedFloorWipeOccurrence->eState &&
		failedMechanicSequence == valtan.iPatternSequence &&
		SERVER_ENTITY_ACTION::IDLE == valtan.eAction &&
		nullptr != resetRequiredTrace &&
		VALTAN_DECISION_RESULT::MECHANIC_RESET_REQUIRED ==
			resetRequiredTrace->eResult,
		"Fail a critical mechanic closed until an encounter reset replaces its ledger");

	/* A party wipe is the one mechanic failure this encounter recovers from on
	its own, because nothing else ever clears the ledger latch. It must stay
	closed while the party is down and while the failure was anything else, then
	release once a revived player is back in the fight. */
	const auto findFloorWipeOccurrence =
		[&valtan]() -> SERVER_BOSS_MECHANIC_OCCURRENCE*
		{
			const auto found = std::find_if(
				valtan.MechanicOccurrences.begin(),
				valtan.MechanicOccurrences.end(),
				[](const SERVER_BOSS_MECHANIC_OCCURRENCE& occurrence)
				{
					return "VALTAN_FLOOR_WIPE_130" == occurrence.strPatternId;
				});
			return valtan.MechanicOccurrences.end() == found ? nullptr : &*found;
		};
	tests.Require(
		valtan.bMechanicLedgerRequiresReset &&
		SERVER_ENTITY_ACTION::IDLE == valtan.eAction,
		"Hold a non-wipe mechanic failure latched even while a player is alive");
	SERVER_BOSS_MECHANIC_OCCURRENCE* wipedOccurrence = findFloorWipeOccurrence();
	tests.Require(nullptr != wipedOccurrence,
		"Keep the 130-bar occurrence addressable by its stable pattern ID");
	wipedOccurrence->eState =
		SERVER_BOSS_MECHANIC_STATE::FAILED_REQUIRES_RESET;
	wipedOccurrence->eFailure =
		SERVER_BOSS_MECHANIC_FAILURE::NO_VALID_TARGET;
	valtan.bMechanicLedgerRequiresReset = true;
	const LostArk::Shared::PLAYER_ACTION_STATE aliveAction =
		players.begin()->second.eAction;
	const std::uint32_t aliveHp = players.begin()->second.iCurrentHp;
	players.begin()->second.eAction =
		LostArk::Shared::PLAYER_ACTION_STATE::DEAD;
	brain.Update(valtan, players, catalog, navigation, 1.f / 30.f, 174u,
		{}, valtanDamageEvents);
	wipedOccurrence = findFloorWipeOccurrence();
	tests.Require(
		valtan.bMechanicLedgerRequiresReset &&
		nullptr != wipedOccurrence &&
		SERVER_BOSS_MECHANIC_STATE::FAILED_REQUIRES_RESET ==
			wipedOccurrence->eState &&
		SERVER_ENTITY_ACTION::IDLE == valtan.eAction,
		"Hold the wiped mechanic ledger latched while every player is dead");
	players.begin()->second.eAction = aliveAction;
	players.begin()->second.iCurrentHp = 0u == aliveHp ? 781u : aliveHp;
	players.begin()->second.isCombatReady = true;
	brain.Update(valtan, players, catalog, navigation, 1.f / 30.f, 175u,
		{}, valtanDamageEvents);
	const VALTAN_DECISION_TRACE* revivedTrace = brain.Get_LatestDecisionTrace();
	wipedOccurrence = findFloorWipeOccurrence();
	tests.Require(
		!valtan.bMechanicLedgerRequiresReset &&
		nullptr != wipedOccurrence &&
		SERVER_BOSS_MECHANIC_STATE::COMPLETED == wipedOccurrence->eState &&
		SERVER_BOSS_MECHANIC_FAILURE::NO_VALID_TARGET ==
			wipedOccurrence->eFailure &&
		nullptr != revivedTrace &&
		VALTAN_DECISION_RESULT::MECHANIC_RESET_REQUIRED !=
			revivedTrace->eResult,
		"Resume the Valtan rotation once a revived player ends the party wipe");
	tests.Require(
		valtan.PendingPatternIds.end() == std::find(
			valtan.PendingPatternIds.begin(), valtan.PendingPatternIds.end(),
			std::string("VALTAN_FLOOR_WIPE_130")),
		"Never re-arm a spent health-bar mechanic when the wipe latch releases");

	{
		/* Source cooldowns are selection gates, not display-only metadata. Run a
		deterministic normal-pattern simulation long enough to see positive-
		cooldown patterns repeat and reject any early repeat. */
		std::map<PLAYER_ID, SERVER_PLAYER> cooldownPlayers;
		SERVER_PLAYER cooldownTarget{};
		cooldownTarget.iPlayerId = 77u;
		cooldownTarget.iNetEntityId = 177u;
		cooldownTarget.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		cooldownTarget.iCurrentHp = 1000000u;
		cooldownTarget.iMaximumHp = 1000000u;
		cooldownTarget.fPositionX = 6.f;
		cooldownTarget.fPositionY = 22.97f;
		cooldownTarget.fPositionZ = 0.f;
		cooldownTarget.isCombatReady = true;
		cooldownPlayers.emplace(cooldownTarget.iPlayerId, cooldownTarget);
		SERVER_WORLD_ENTITY cooldownBoss{};
		cooldownBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		cooldownBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		cooldownBoss.strArchetypeId = "BOSS_VALTAN";
		cooldownBoss.strEncounterId = "ENCOUNTER_VALTAN";
		cooldownBoss.iCurrentHp = 60000u;
		cooldownBoss.iMaximumHp = 60000u;
		cooldownBoss.iMaximumHealthBars = 160u;
		cooldownBoss.iLastEvaluatedHealthBar = 160u;
		cooldownBoss.iPhaseTwoHpPercent = 50u;
		cooldownBoss.fPositionY = 22.97f;
		cooldownBoss.fSpawnPositionY = 22.97f;
		cooldownBoss.fEngageDistance = 35.f;
		cooldownBoss.fMoveSpeed = 3.f;
		cooldownBoss.bIntroPatternConsumed = true;
		cooldownBoss.bScriptedPatternPlayback = true;
		std::map<std::uint32_t, std::uint32_t> lastStartTickBySourceAction;
		std::uint32_t previousSequence = 0u;
		bool respectedCooldowns = true;
		bool observedPositiveCooldownRepeat = false;
		bool observedFourSlash = false;
		bool observedIndependentSource = false;
		bool replayedIntro = false;
		bool observedDeterministicDecisionTrace = false;
		bool observedCooldownExclusionTrace = false;
		std::vector<DAMAGE_EVENT> cooldownDamageEvents;
		const std::vector<BOSS_PATTERN_DEFINITION>* cooldownPatterns =
			catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		for (std::uint32_t tick = 1000u; tick < 11000u; ++tick)
		{
			SERVER_PLAYER& liveTarget = cooldownPlayers.begin()->second;
			liveTarget.iCurrentHp = liveTarget.iMaximumHp;
			liveTarget.eAction = PLAYER_ACTION_STATE::NONE;
			cooldownDamageEvents.clear();
			brain.Update(
				cooldownBoss, cooldownPlayers, catalog, navigation,
				1.f / 30.f, tick, {}, cooldownDamageEvents);
			if (cooldownBoss.iPatternSequence == previousSequence)
				continue;
			previousSequence = cooldownBoss.iPatternSequence;
			if (const VALTAN_DECISION_TRACE* trace =
				brain.Get_LatestDecisionTrace())
			{
				const auto selectedCandidate = std::find_if(
					trace->Candidates.begin(), trace->Candidates.end(),
					[trace](const VALTAN_DECISION_CANDIDATE_TRACE& candidate)
					{
						return candidate.strPatternId ==
							trace->strSelectedPatternId;
					});
				observedDeterministicDecisionTrace =
					observedDeterministicDecisionTrace ||
					(VALTAN_DECISION_RESULT::SELECTED == trace->eResult &&
					 0u != trace->iRawRandomInput &&
					 trace->iMixedRandomValue >= trace->iRandomTicket &&
					 trace->iTotalWeight > trace->iRandomTicket &&
					 trace->Candidates.end() != selectedCandidate &&
					 selectedCandidate->bSelected &&
					 selectedCandidate->iEffectiveWeight > 0u &&
					 trace->iRandomTicket >=
						selectedCandidate->iWeightBeginInclusive &&
					 trace->iRandomTicket <
						selectedCandidate->iWeightEndExclusive);
				observedCooldownExclusionTrace =
					observedCooldownExclusionTrace || std::any_of(
						trace->Candidates.begin(), trace->Candidates.end(),
						[](const VALTAN_DECISION_CANDIDATE_TRACE& candidate)
						{
							return candidate.iCooldownRemainingTicks > 0u &&
								0u != (candidate.iExclusionMask &
									VALTAN_EXCLUDE_COOLDOWN);
						});
			}
			replayedIntro = replayedIntro ||
				cooldownBoss.strPatternId == "VALTAN_ENTRANCE_WHIRLWIND";
			if (nullptr == cooldownPatterns)
			{
				respectedCooldowns = false;
				break;
			}
			const auto definition = std::find_if(
				cooldownPatterns->begin(), cooldownPatterns->end(),
				[&cooldownBoss](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == cooldownBoss.strPatternId; });
			if (cooldownPatterns->end() == definition)
			{
				respectedCooldowns = false;
				break;
			}
			const auto previous = lastStartTickBySourceAction.find(
				definition->iSourcePrimaryActionId);
			if (lastStartTickBySourceAction.end() != previous &&
				0u != definition->iSourceCooldownTicks)
			{
				observedPositiveCooldownRepeat = true;
				respectedCooldowns = respectedCooldowns &&
					static_cast<std::uint32_t>(tick - previous->second) >=
					definition->iSourceCooldownTicks;
			}
			lastStartTickBySourceAction[
				definition->iSourcePrimaryActionId] = tick;
			observedFourSlash = observedFourSlash ||
				"VALTAN_FOUR_SLASH" == definition->strPatternId;
			observedIndependentSource = observedIndependentSource ||
				420601u == definition->iSourcePrimaryActionId;
		}
		const std::size_t fourSlashCooldownCount =
			static_cast<std::size_t>(std::count_if(
				cooldownBoss.PatternCooldowns.begin(),
				cooldownBoss.PatternCooldowns.end(),
				[](const SERVER_BOSS_PATTERN_COOLDOWN& cooldown)
				{ return 420609u == cooldown.iSourcePrimaryActionId; }));
		const bool hasIndependentCooldown = std::any_of(
			cooldownBoss.PatternCooldowns.begin(),
			cooldownBoss.PatternCooldowns.end(),
			[](const SERVER_BOSS_PATTERN_COOLDOWN& cooldown)
			{ return 420601u == cooldown.iSourcePrimaryActionId; });
		tests.Require(
			respectedCooldowns && observedPositiveCooldownRepeat &&
			observedFourSlash && observedIndependentSource &&
			1u == fourSlashCooldownCount &&
			hasIndependentCooldown && !replayedIntro &&
			observedDeterministicDecisionTrace &&
			observedCooldownExclusionTrace,
			"Gate normal Valtan reselection by four-slash source-action cooldown, keep other sources independent, and never reroll the intro");
	}

	{
		/* Drive the published rejoined pattern through the real fixed-step brain.
		The authored millisecond offsets deliberately do not all fall on exact
		30 Hz boundaries, so each pulse must wait for the first crossing tick. */
		std::map<PLAYER_ID, SERVER_PLAYER> slashPlayers;
		SERVER_PLAYER slashTarget{};
		slashTarget.iPlayerId = 78u;
		slashTarget.iNetEntityId = 178u;
		slashTarget.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		slashTarget.iCurrentHp = 1000000u;
		slashTarget.iMaximumHp = 1000000u;
		slashTarget.fPositionY = 22.97f;
		slashTarget.fPositionZ = 4.f;
		slashTarget.isCombatReady = true;
		slashPlayers.emplace(slashTarget.iPlayerId, slashTarget);

		SERVER_WORLD_ENTITY slashBoss{};
		slashBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		slashBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		slashBoss.strArchetypeId = "BOSS_VALTAN";
		slashBoss.strEncounterId = "ENCOUNTER_VALTAN";
		slashBoss.iCurrentHp = 60000u;
		slashBoss.iMaximumHp = 60000u;
		slashBoss.iMaximumHealthBars = 160u;
		slashBoss.iLastEvaluatedHealthBar = 160u;
		slashBoss.iPhaseTwoHpPercent = 50u;
		slashBoss.fPositionY = 22.97f;
		slashBoss.fSpawnPositionY = 22.97f;
		slashBoss.fEngageDistance = 35.f;
		slashBoss.fMoveSpeed = 3.f;
		slashBoss.bIntroPatternConsumed = true;
		slashBoss.bScriptedPatternPlayback = true;
		slashBoss.PendingPatternIds.push_back("VALTAN_FOUR_SLASH");

		CValtanBrain slashBrain;
		std::vector<DAMAGE_EVENT> slashDamageEvents;
		std::uint32_t slashServerTick = 12000u;
		bool enteredSlashStage = false;
		bool windupStayedDamageFree = true;
		for (std::uint32_t tick = 0u; tick < 20u; ++tick)
		{
			slashDamageEvents.clear();
			slashBrain.Update(
				slashBoss, slashPlayers, catalog, navigation,
				1.f / 30.f, slashServerTick++, {}, slashDamageEvents);
			windupStayedDamageFree =
				windupStayedDamageFree && slashDamageEvents.empty();
			if ("SLASHES" == slashBoss.strPatternStageId)
			{
				enteredSlashStage = true;
				break;
			}
		}

		std::vector<std::uint32_t> observedContactTicks;
		std::size_t observedContactCount = 0u;
		bool crossingStateExact = enteredSlashStage;
		std::uint32_t slashTransitionTick = 0u;
		for (std::uint32_t activeTick = 1u;
			activeTick <= 120u && enteredSlashStage; ++activeTick)
		{
			slashDamageEvents.clear();
			slashBrain.Update(
				slashBoss, slashPlayers, catalog, navigation,
				1.f / 30.f, slashServerTick++, {}, slashDamageEvents);
			observedContactCount += slashDamageEvents.size();
			if (!slashDamageEvents.empty())
				observedContactTicks.push_back(activeTick);

			const bool expectsContact =
				54u == activeTick || 77u == activeTick || 100u == activeTick;
			crossingStateExact = crossingStateExact &&
				(expectsContact ?
					1u == slashDamageEvents.size() &&
					178u == slashDamageEvents.front().iTargetNetEntityId &&
					!slashDamageEvents.front().isOutgoing :
					slashDamageEvents.empty());
			if (53u == activeTick)
				crossingStateExact = crossingStateExact &&
					0u == slashBoss.iAppliedPatternHitCount;
			else if (54u == activeTick || 76u == activeTick)
				crossingStateExact = crossingStateExact &&
					1u == slashBoss.iAppliedPatternHitCount;
			else if (77u == activeTick || 99u == activeTick)
				crossingStateExact = crossingStateExact &&
					2u == slashBoss.iAppliedPatternHitCount;
			else if (100u == activeTick)
				crossingStateExact = crossingStateExact &&
					3u == slashBoss.iAppliedPatternHitCount;

			if ("SPIN" == slashBoss.strPatternStageId)
			{
				slashTransitionTick = activeTick;
				break;
			}
		}
		tests.Require(
			windupStayedDamageFree && crossingStateExact &&
			105u == slashTransitionTick && 3u == observedContactCount &&
			std::vector<std::uint32_t>{ 54u, 77u, 100u } ==
				observedContactTicks,
			"Consume 1790/2560/3330 ms slash contacts at ticks 54/77/100 and enter the joined SPIN at tick 105 with no extra pulse");

		/* SPIN is the next stage of the same pattern, not a separately rolled
		pattern. It owns one contact at 600 ms and then hands off to recovery. */
		std::vector<std::uint32_t> spinContactTicks;
		std::size_t spinContactCount = 0u;
		std::uint32_t spinTransitionTick = 0u;
		bool spinCrossingStateExact = true;
		for (std::uint32_t activeTick = 1u; activeTick <= 100u; ++activeTick)
		{
			slashDamageEvents.clear();
			slashBrain.Update(
				slashBoss, slashPlayers, catalog, navigation,
				1.f / 30.f, slashServerTick++, {}, slashDamageEvents);
			spinContactCount += slashDamageEvents.size();
			if (!slashDamageEvents.empty())
				spinContactTicks.push_back(activeTick);

			const bool expectsContact = 18u == activeTick;
			spinCrossingStateExact = spinCrossingStateExact &&
				(expectsContact ?
					1u == slashDamageEvents.size() &&
					178u == slashDamageEvents.front().iTargetNetEntityId &&
					!slashDamageEvents.front().isOutgoing :
					slashDamageEvents.empty());
			if (17u == activeTick)
				spinCrossingStateExact = spinCrossingStateExact &&
					0u == slashBoss.iAppliedPatternHitCount;
			else if (18u <= activeTick &&
				"SPIN" == slashBoss.strPatternStageId)
				spinCrossingStateExact = spinCrossingStateExact &&
					1u == slashBoss.iAppliedPatternHitCount;
			else if ("RECOVERY" == slashBoss.strPatternStageId)
				spinCrossingStateExact = spinCrossingStateExact &&
					0u == slashBoss.iAppliedPatternHitCount;

			if ("RECOVERY" == slashBoss.strPatternStageId)
			{
				spinTransitionTick = activeTick;
				break;
			}
		}
		tests.Require(
			spinCrossingStateExact && 96u == spinTransitionTick &&
			1u == spinContactCount &&
			std::vector<std::uint32_t>{ 18u } == spinContactTicks,
			"Consume the joined SPIN contact at tick 18 and enter recovery at ceil(3167ms*30Hz)=tick 96");
	}

	{
		/* The stele set is raised by the RECOVERY stage of the 100-bar mechanic
		and by nothing else, so the raid never sees a pillar unless that pattern
		actually runs and actually reaches that stage. Cross the bar and walk the
		pattern to the end so a break anywhere in that chain names itself. */
		std::map<PLAYER_ID, SERVER_PLAYER> pillarPlayers;
		SERVER_PLAYER pillarTarget{};
		pillarTarget.iPlayerId = 91u;
		pillarTarget.iNetEntityId = 191u;
		pillarTarget.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		pillarTarget.iCurrentHp = 1000000u;
		pillarTarget.iMaximumHp = 1000000u;
		pillarTarget.fPositionX = 6.f;
		pillarTarget.fPositionY = 22.97f;
		pillarTarget.fPositionZ = 0.f;
		pillarTarget.isCombatReady = true;
		pillarPlayers.emplace(pillarTarget.iPlayerId, pillarTarget);

		SERVER_WORLD_ENTITY pillarBoss{};
		pillarBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		pillarBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		pillarBoss.strArchetypeId = "BOSS_VALTAN";
		pillarBoss.strEncounterId = "ENCOUNTER_VALTAN";
		pillarBoss.iMaximumHp = 60000u;
		pillarBoss.iMaximumHealthBars = 160u;
		pillarBoss.iPhaseTwoHpPercent = 68u;
		pillarBoss.fPositionY = 22.97f;
		pillarBoss.fSpawnPositionY = 22.97f;
		pillarBoss.fEngageDistance = 35.f;
		pillarBoss.fMoveSpeed = 3.f;
		pillarBoss.bIntroPatternConsumed = true;
		pillarBoss.bAutomaticPatternSequenceAuditionOverride = true;
		/* Sit one bar above the trigger so the very next damage crosses it. */
		pillarBoss.iCurrentHp = CValtanBrain::Resolve_HealthBarHp(pillarBoss, 101u);
		pillarBoss.iLastEvaluatedHealthBar = 101u;

		CValtanBrain pillarBrain;
		std::vector<DAMAGE_EVENT> pillarEvents;
		pillarBoss.iCurrentHp = CValtanBrain::Resolve_HealthBarHp(pillarBoss, 100u);
		pillarBrain.Update(
			pillarBoss, pillarPlayers, catalog, navigation, 1.f / 30.f, 2000u,
			{}, pillarEvents);
		const bool queuedOrRunning =
			"VALTAN_FOUR_PILLARS_105" == pillarBoss.strPatternId ||
			pillarBoss.PendingPatternIds.end() != std::find(
				pillarBoss.PendingPatternIds.begin(),
				pillarBoss.PendingPatternIds.end(),
				std::string("VALTAN_FOUR_PILLARS_105"));
		tests.Require(
			queuedOrRunning,
			"Queue the 100-bar stele mechanic when Valtan crosses its trigger bar");

		bool reachedRecovery = false;
		bool ranPillarPattern = false;
		for (std::uint32_t tick = 2001u; tick < 2400u; ++tick)
		{
			pillarEvents.clear();
			pillarBrain.Update(
				pillarBoss, pillarPlayers, catalog, navigation, 1.f / 30.f, tick,
				{}, pillarEvents);
			if ("VALTAN_FOUR_PILLARS_105" != pillarBoss.strPatternId)
				continue;
			ranPillarPattern = true;
			if ("RECOVERY" == pillarBoss.strPatternStageId)
			{
				reachedRecovery = true;
				break;
			}
		}
		tests.Require(
			ranPillarPattern,
			"Start the 100-bar stele mechanic from the queue");
		tests.Require(
			reachedRecovery && 0u != pillarBoss.iPatternSequence,
			"Reach the stele RECOVERY stage that raises the four pillars");
	}

	{
		/* The stage edge the Brain reaches still has to reach the prop runtime.
		This is the room-side half of the raise: the same pattern and stage the
		previous test proved reachable, handed to the entry the room tick calls. */
		CGameRoom pillarRoom{ WORLD_ID::VALTAN_ARENA };
		SERVER_WORLD_ENTITY stageBoss{};
		stageBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		stageBoss.strArchetypeId = "BOSS_VALTAN";
		stageBoss.strEncounterId = "ENCOUNTER_VALTAN";
		stageBoss.strPatternId = "VALTAN_FOUR_PILLARS_105";
		stageBoss.strPatternStageId = "RECOVERY";
		stageBoss.iPatternStageIndex = 3u;
		stageBoss.iPatternSequence = 1u;
		stageBoss.PinnedDefinitionRevision =
			pillarRoom.m_GameplayCatalog.Get_ActiveRevision();
		const bool roomReady = pillarRoom.Is_Ready() &&
			pillarRoom.m_EncounterPropRuntime.Is_Initialized();
		const bool entered =
			pillarRoom.Apply_EncounterPropStageEntry(stageBoss, 500u);
		const auto& raisedSlots =
			pillarRoom.m_EncounterPropRuntime.Get_SlotStates();
		const bool everySlotIntact = 4u == raisedSlots.size() &&
			std::all_of(raisedSlots.begin(), raisedSlots.end(),
				[](const ENCOUNTER_PROP_SLOT_STATE& slot)
				{
					return ENCOUNTER_PROP_STATE::INTACT == slot.eState;
				});
		tests.Require(
			roomReady && entered && everySlotIntact,
			"Raise the four stele from the room stage entry the tick loop calls");
	}

	{
		/* Product automatic playback is one exact authored program. Debug entrance
		audition still yields that program explicitly, but ordinary encounter ticks
		must never run the entrance, a health interrupt, or a weighted fallback
		outside that program. */
		std::map<PLAYER_ID, SERVER_PLAYER> entrancePlayers;
		SERVER_PLAYER entranceTarget{};
		entranceTarget.iPlayerId = 78u;
		entranceTarget.iNetEntityId = 178u;
		entranceTarget.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		entranceTarget.iCurrentHp = 1000000u;
		entranceTarget.iMaximumHp = 1000000u;
		entranceTarget.fPositionX = 196.03f;
		entranceTarget.fPositionY = 22.97f;
		entranceTarget.fPositionZ = -122.06f;
		entranceTarget.isCombatReady = true;
		entrancePlayers.emplace(entranceTarget.iPlayerId, entranceTarget);
		const auto makeEntranceBoss = []()
		{
			SERVER_WORLD_ENTITY boss{};
			boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
			boss.eAction = SERVER_ENTITY_ACTION::IDLE;
			boss.strArchetypeId = "BOSS_VALTAN";
			boss.strEncounterId = "ENCOUNTER_VALTAN";
			boss.iCurrentHp = 60000u;
			boss.iMaximumHp = 60000u;
			boss.iMaximumHealthBars = 160u;
			boss.iLastEvaluatedHealthBar = 160u;
			boss.iPhaseTwoHpPercent = 68u;
			boss.iPhase = 1u;
			boss.fPositionX = 156.03f;
			boss.fPositionY = 22.97f;
			boss.fPositionZ = -122.06f;
			boss.fSpawnPositionX = boss.fPositionX;
			boss.fSpawnPositionY = 22.97f;
			boss.fSpawnPositionZ = boss.fPositionZ;
			boss.fEngageDistance = 35.f;
			boss.fMoveSpeed = 3.f;
			return boss;
		};
		std::uint32_t automaticTick = 1000u;
		const auto advanceBoss = [&] (
			SERVER_WORLD_ENTITY& boss, const std::uint32_t count,
			std::vector<std::string>* observedOrder = nullptr)
		{
			std::vector<DAMAGE_EVENT> events;
			CValtanBrain brain;
			for (std::uint32_t index = 0u; index < count; ++index)
			{
				events.clear();
				brain.Update(
					boss, entrancePlayers, catalog, navigation,
					1.f / 30.f, automaticTick++, {}, events);
				if (nullptr != observedOrder && !boss.strPatternId.empty() &&
					(observedOrder->empty() ||
					 observedOrder->back() != boss.strPatternId))
				{
					observedOrder->push_back(boss.strPatternId);
				}
			}
		};
		SERVER_WORLD_ENTITY waitingBoss = makeEntranceBoss();
		waitingBoss.bAutomaticPatternSequenceAuditionOverride = true;
		advanceBoss(waitingBoss, 60u);
		const bool heldAtSpawn =
			!waitingBoss.bIntroPatternConsumed &&
			waitingBoss.strPatternId.empty() &&
			SERVER_ENTITY_ACTION::IDLE == waitingBoss.eAction &&
			std::abs(waitingBoss.fPositionX - 156.03f) < 0.001f &&
			std::abs(waitingBoss.fPositionZ + 122.06f) < 0.001f;
		entrancePlayers.begin()->second.fPositionX = 164.03f;
		advanceBoss(waitingBoss, 2u);
		tests.Require(
			heldAtSpawn && waitingBoss.bIntroPatternConsumed &&
			"VALTAN_ENTRANCE_WHIRLWIND" == waitingBoss.strPatternId,
			"Keep explicit entrance audition available without admitting it to automatic playback");
		entrancePlayers.begin()->second.fPositionX = 196.03f;
		SERVER_WORLD_ENTITY hurtBoss = makeEntranceBoss();
		hurtBoss.bAutomaticPatternSequenceAuditionOverride = true;
		hurtBoss.iCurrentHp = hurtBoss.iMaximumHp - 1u;
		advanceBoss(hurtBoss, 2u);
		tests.Require(
			hurtBoss.bIntroPatternConsumed &&
			"VALTAN_ENTRANCE_WHIRLWIND" != hurtBoss.strPatternId,
			"Drop the pending Valtan entrance once the boss is already taking damage");

		entrancePlayers.begin()->second.fPositionX = 164.03f;
		entrancePlayers.begin()->second.iCurrentHp = 5500u;
		entrancePlayers.begin()->second.iMaximumHp = 5500u;
		SERVER_PLAYER pizzaSecondTarget = entrancePlayers.begin()->second;
		pizzaSecondTarget.iPlayerId = 79u;
		pizzaSecondTarget.iNetEntityId = 179u;
		pizzaSecondTarget.fPositionX = 166.03f;
		entrancePlayers.emplace(
			pizzaSecondTarget.iPlayerId, pizzaSecondTarget);
		SERVER_WORLD_ENTITY pacingBoss = makeEntranceBoss();
		pacingBoss.strRotationId = "sequence.valtan.server-authored.v1";
		pacingBoss.iRotationStepIndex = 7u;
		pacingBoss.iAutomaticPatternSequenceInterStepPursuitTicks = 30u;
		for (std::uint32_t tick = 0u;
			tick < 300u && 7u == pacingBoss.iRotationStepIndex; ++tick)
		{
			advanceBoss(pacingBoss, 1u);
		}
		const bool pursuitArmedAfterArenaBreak =
			8u == pacingBoss.iRotationStepIndex &&
			30u == pacingBoss.iAutomaticPatternSequencePursuitTicksRemaining &&
			pacingBoss.strPatternId.empty();
		const float pursuitStartX = pacingBoss.fPositionX;
		const float pursuitStartZ = pacingBoss.fPositionZ;
		advanceBoss(pacingBoss, 1u);
		const float pursuitDeltaX = pacingBoss.fPositionX - pursuitStartX;
		const float pursuitDeltaZ = pacingBoss.fPositionZ - pursuitStartZ;
		const bool chasedDuringPursuit =
			29u == pacingBoss.iAutomaticPatternSequencePursuitTicksRemaining &&
			pacingBoss.strPatternId.empty() &&
			SERVER_ENTITY_ACTION::CHASE == pacingBoss.eAction &&
			pursuitDeltaX * pursuitDeltaX + pursuitDeltaZ * pursuitDeltaZ > 0.f;
		advanceBoss(pacingBoss, 29u);
		const bool heldNextStepForExactWindow =
			0u == pacingBoss.iAutomaticPatternSequencePursuitTicksRemaining &&
			pacingBoss.strPatternId.empty() &&
			SERVER_ENTITY_ACTION::CHASE == pacingBoss.eAction;
		advanceBoss(pacingBoss, 1u);
		tests.Require(
			pursuitArmedAfterArenaBreak && chasedDuringPursuit &&
			heldNextStepForExactWindow &&
			"VALTAN_SIX_PIZZA_106" == pacingBoss.strPatternId,
			"Chase the nearest player for the authored one-second inter-step window without re-selecting the next Valtan pattern");
		const NET_ENTITY_ID lockedPizzaTarget =
			pacingBoss.iPatternTargetEntityId;
		auto lockedPizzaPlayer = std::find_if(
			entrancePlayers.begin(), entrancePlayers.end(),
			[lockedPizzaTarget](const auto& entry)
			{
				return entry.second.iNetEntityId == lockedPizzaTarget;
			});
		auto nonlockedPizzaPlayer = std::find_if(
			entrancePlayers.begin(), entrancePlayers.end(),
			[lockedPizzaTarget](const auto& entry)
			{
				return entry.second.iNetEntityId != lockedPizzaTarget;
			});
		const auto expectedYawToward = [](
			const SERVER_WORLD_ENTITY& boss, const SERVER_PLAYER& target)
		{
			return std::atan2(
				target.fPositionX - boss.fPositionX,
				target.fPositionZ - boss.fPositionZ) *
				180.f / 3.14159265358979323846f;
		};
		const auto yawMatches = [](const float actual, const float expected)
		{
			return std::abs(std::remainder(actual - expected, 360.f)) <
				0.001f;
		};
		bool retainedRandomTargetWhileNearestChanged = false;
		bool followedIndependentLockedTargetMove = false;
		bool retainedAndTrackedAcrossStage = false;
		if (entrancePlayers.end() != lockedPizzaPlayer &&
			entrancePlayers.end() != nonlockedPizzaPlayer)
		{
			SERVER_PLAYER& lockedPlayer = lockedPizzaPlayer->second;
			SERVER_PLAYER& nonlockedPlayer = nonlockedPizzaPlayer->second;
			lockedPlayer.fPositionX = pacingBoss.fPositionX + 12.f;
			lockedPlayer.fPositionZ = pacingBoss.fPositionZ + 8.f;
			nonlockedPlayer.fPositionX = pacingBoss.fPositionX + 0.25f;
			nonlockedPlayer.fPositionZ = pacingBoss.fPositionZ;
			advanceBoss(pacingBoss, 1u);
			retainedRandomTargetWhileNearestChanged =
				lockedPizzaTarget == pacingBoss.iPatternTargetEntityId &&
				nonlockedPlayer.iNetEntityId == pacingBoss.iTargetEntityId &&
				yawMatches(
					pacingBoss.fYawDegrees,
					expectedYawToward(pacingBoss, lockedPlayer));

			lockedPlayer.fPositionX = pacingBoss.fPositionX - 9.f;
			lockedPlayer.fPositionZ = pacingBoss.fPositionZ + 5.f;
			advanceBoss(pacingBoss, 1u);
			followedIndependentLockedTargetMove =
				lockedPizzaTarget == pacingBoss.iPatternTargetEntityId &&
				nonlockedPlayer.iNetEntityId == pacingBoss.iTargetEntityId &&
				yawMatches(
					pacingBoss.fYawDegrees,
					expectedYawToward(pacingBoss, lockedPlayer));

			const std::uint32_t initialPizzaStageIndex =
				pacingBoss.iPatternStageIndex;
			for (std::uint32_t tick = 0u;
				tick < 60u &&
				"VALTAN_SIX_PIZZA_106" == pacingBoss.strPatternId &&
				initialPizzaStageIndex == pacingBoss.iPatternStageIndex;
				++tick)
			{
				advanceBoss(pacingBoss, 1u);
			}
			const bool crossedIntoNextPizzaStage =
				"VALTAN_SIX_PIZZA_106" == pacingBoss.strPatternId &&
				initialPizzaStageIndex + 1u == pacingBoss.iPatternStageIndex &&
				"STEP_02" == pacingBoss.strPatternStageId;
			lockedPlayer.fPositionX = pacingBoss.fPositionX + 7.f;
			lockedPlayer.fPositionZ = pacingBoss.fPositionZ - 11.f;
			nonlockedPlayer.fPositionX = pacingBoss.fPositionX;
			nonlockedPlayer.fPositionZ = pacingBoss.fPositionZ + 0.25f;
			advanceBoss(pacingBoss, 1u);
			retainedAndTrackedAcrossStage =
				crossedIntoNextPizzaStage &&
				lockedPizzaTarget == pacingBoss.iPatternTargetEntityId &&
				nonlockedPlayer.iNetEntityId == pacingBoss.iTargetEntityId &&
				yawMatches(
					pacingBoss.fYawDegrees,
					expectedYawToward(pacingBoss, lockedPlayer));
		}
		tests.Require(
			INVALID_NET_ENTITY_ID != lockedPizzaTarget &&
			retainedRandomTargetWhileNearestChanged,
			"Keep six-pizza on its random start target when another alive player becomes nearest");
		tests.Require(
			followedIndependentLockedTargetMove,
			"Match six-pizza yaw to atan2 toward the moving locked player instead of the nearer player");
		tests.Require(
			retainedAndTrackedAcrossStage,
			"Retain and track the same six-pizza player after advancing from STEP_01 to STEP_02");
		entrancePlayers.erase(pizzaSecondTarget.iPlayerId);
		entrancePlayers.begin()->second.fPositionX = 164.03f;
		entrancePlayers.begin()->second.fPositionZ = -122.06f;

		entrancePlayers.begin()->second.iCurrentHp = 5500u;
		entrancePlayers.begin()->second.eAction = PLAYER_ACTION_STATE::NONE;
		entrancePlayers.begin()->second.isCombatReady = true;
		SERVER_WORLD_ENTITY automaticBoss = makeEntranceBoss();
		std::vector<std::string> observedOrder;
		bool observedFloorWipeDeath = false;
		for (std::uint32_t tick = 0u;
			tick < 4000u && !observedFloorWipeDeath; ++tick)
		{
			advanceBoss(automaticBoss, 1u, &observedOrder);
			const SERVER_PLAYER& player = entrancePlayers.begin()->second;
			observedFloorWipeDeath = 0u == player.iCurrentHp &&
				PLAYER_ACTION_STATE::DEAD == player.eAction &&
				"VALTAN_FLOOR_WIPE_130" == automaticBoss.strPatternId &&
				5u == automaticBoss.iRotationStepIndex &&
				automaticBoss.bAutomaticPatternSequenceStepRunning;
		}
		for (std::uint32_t tick = 0u;
			tick < 300u && automaticBoss.iRotationStepIndex < 6u; ++tick)
		{
			advanceBoss(automaticBoss, 1u, &observedOrder);
		}
		const std::vector<std::string> phaseOneThroughWipe{
			"VALTAN_ENTRANCE_CINEMATIC",
			"VALTAN_WHIRLWIND",
			"VALTAN_FOUR_SLASH",
			"VALTAN_FIST_IN_OUT",
			"VALTAN_HIGH_JUMP",
			"VALTAN_FLOOR_WIPE_130" };
		const bool heldDashCursorWhileDead =
			observedFloorWipeDeath && observedOrder == phaseOneThroughWipe &&
			6u == automaticBoss.iRotationStepIndex &&
			30u == automaticBoss.iAutomaticPatternSequencePursuitTicksRemaining &&
			!automaticBoss.bAutomaticPatternSequenceStepRunning &&
			!automaticBoss.bMechanicLedgerRequiresReset &&
			automaticBoss.strPatternId.empty() &&
			SERVER_ENTITY_ACTION::IDLE == automaticBoss.eAction &&
			PLAYER_ACTION_STATE::DEAD == entrancePlayers.begin()->second.eAction;
		advanceBoss(automaticBoss, 60u, &observedOrder);
		const bool stayedAtDashCursorUntilRevive =
			observedOrder == phaseOneThroughWipe &&
			6u == automaticBoss.iRotationStepIndex &&
			30u == automaticBoss.iAutomaticPatternSequencePursuitTicksRemaining &&
			!automaticBoss.bAutomaticPatternSequenceStepRunning &&
			!automaticBoss.bMechanicLedgerRequiresReset &&
			automaticBoss.strPatternId.empty() &&
			SERVER_ENTITY_ACTION::IDLE == automaticBoss.eAction;
		SERVER_PLAYER& revivedPlayer = entrancePlayers.begin()->second;
		revivedPlayer.iCurrentHp = revivedPlayer.iMaximumHp;
		revivedPlayer.eAction = PLAYER_ACTION_STATE::NONE;
		revivedPlayer.isCombatReady = true;
		advanceBoss(automaticBoss, 1u, &observedOrder);
		const bool resumedPursuitImmediatelyAfterRevive =
			6u == automaticBoss.iRotationStepIndex &&
			29u == automaticBoss.iAutomaticPatternSequencePursuitTicksRemaining &&
			automaticBoss.strPatternId.empty() &&
			SERVER_ENTITY_ACTION::CHASE == automaticBoss.eAction;
		/* VALTAN_TRASH deliberately repeats STEP_06 -> STEP_08 until a real
		   COUNTER_HIT arrives. The ordered review is still a gameplay sequence,
		   not a timeline-only auto-skip, so drive that one required player
		   outcome through the same typed BossCombat mailbox before asserting the
		   remaining cursor. Auto-finishing the pattern in CValtanBrain would make
		   the Product encounter disagree with the authored counter contract. */
		bool publishedTrashCounter = false;
		for (std::uint32_t tick = 0u; tick < 8000u; ++tick)
		{
			if (!publishedTrashCounter &&
				"VALTAN_TRASH" == automaticBoss.strPatternId &&
				"STEP_06" == automaticBoss.strPatternStageId)
			{
				publishedTrashCounter =
					CBossCombatRuntime::Publish_PatternOutcome(
						automaticBoss,
						BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT,
						automaticTick);
			}
			advanceBoss(automaticBoss, 1u, &observedOrder);
		}
		const std::vector<std::string> expectedOrder{
			"VALTAN_ENTRANCE_CINEMATIC",
			"VALTAN_WHIRLWIND",
			"VALTAN_FOUR_SLASH",
			"VALTAN_FIST_IN_OUT",
			"VALTAN_HIGH_JUMP",
			"VALTAN_FLOOR_WIPE_130",
			"VALTAN_DASH_CHARGE",
			"VALTAN_ARENA_BREAK_109",
			"VALTAN_SIX_PIZZA_106",
			"VALTAN_ATTACK_WHIRLWIND",
			"VALTAN_CHARGE",
			"VALTAN_SEQUENCE_FOUR",
			"VALTAN_ROAR_CHARGE",
			"VALTAN_SEQUENCE_RUSH",
			"VALTAN_THREE",
			"VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK",
			"VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK",
			"VALTAN_TERRAIN_DESTRUCTION",
			"VALTAN_SEQUENCE_FRONT_BACK_FRONT",
			"VALTAN_WARP",
			"VALTAN_SEQUENCE_TWOHAND",
			"VALTAN_SEQUENCE_WHIRLWIND",
			"VALTAN_TRASH",
			"VALTAN_TRASH_CATCH_SUCCESS",
			"VALTAN_TRASH_CATCH_FAIL",
			"VALTAN_TRASH_CATCH_IF",
			"VALTAN_CATCH_BREATH",
			"VALTAN_COUNTER",
			"VALTAN_CHARGE_2" };
		const bool stoppedAtTerminalStep =
			publishedTrashCounter && observedOrder == expectedOrder &&
			"sequence.valtan.server-authored.v1" ==
				automaticBoss.strRotationId &&
			29u == automaticBoss.iRotationStepIndex &&
			entrancePlayers.begin()->second.iCurrentHp > 0u &&
			!automaticBoss.bAutomaticPatternSequenceStepRunning &&
			automaticBoss.strPatternId.empty() &&
			automaticBoss.PendingPatternIds.empty() &&
			automaticBoss.MechanicOccurrences.empty() &&
			SERVER_ENTITY_ACTION::IDLE == automaticBoss.eAction;
		tests.Require(
			heldDashCursorWhileDead &&
			stayedAtDashCursorUntilRevive &&
			resumedPursuitImmediatelyAfterRevive && stoppedAtTerminalStep,
			"Finish the lethal floor wipe, hold Dash until revive, then run the remaining Phase-1 transition and all 21 Phase-2 animations exactly once before terminal IDLE");

		SERVER_PLAYER& pausePlayer = entrancePlayers.begin()->second;
		pausePlayer.iCurrentHp = pausePlayer.iMaximumHp;
		pausePlayer.eAction = PLAYER_ACTION_STATE::NONE;
		pausePlayer.isCombatReady = true;
		SERVER_WORLD_ENTITY pausedBoss = makeEntranceBoss();
		std::vector<std::string> pausedOrder;
		advanceBoss(pausedBoss, 1u, &pausedOrder);
		const std::string pausedPatternId = pausedBoss.strPatternId;
		const std::string pausedStageId = pausedBoss.strPatternStageId;
		const std::uint32_t pausedStageIndex = pausedBoss.iPatternStageIndex;
		const std::uint32_t pausedAppliedHits =
			pausedBoss.iAppliedPatternHitCount;
		const float pausedActionElapsed = pausedBoss.fActionElapsedSeconds;
		const std::uint32_t actionStartBeforePause = pausedBoss.iActionStartTick;
		pausePlayer.iCurrentHp = 0u;
		pausePlayer.eAction = PLAYER_ACTION_STATE::DEAD;
		pausePlayer.isCombatReady = false;
		advanceBoss(pausedBoss, 60u, &pausedOrder);
		const bool heldOrderedStepWhileDead =
			pausedOrder == std::vector<std::string>{ "VALTAN_ENTRANCE_CINEMATIC" } &&
			0u == pausedBoss.iRotationStepIndex &&
			pausedBoss.bAutomaticPatternSequenceStepRunning &&
			pausedBoss.bAutomaticPatternSequencePausedForRevive &&
			!pausedBoss.bMechanicLedgerRequiresReset &&
			pausedBoss.strPatternId == pausedPatternId &&
			pausedBoss.strPatternStageId == pausedStageId &&
			pausedBoss.iPatternStageIndex == pausedStageIndex &&
			pausedBoss.iAppliedPatternHitCount == pausedAppliedHits &&
			std::abs(pausedBoss.fActionElapsedSeconds - pausedActionElapsed) <
				0.0001f &&
			pausedBoss.iActionStartTick != actionStartBeforePause;
		pausePlayer.iCurrentHp = pausePlayer.iMaximumHp;
		pausePlayer.eAction = PLAYER_ACTION_STATE::NONE;
		pausePlayer.isCombatReady = true;
		advanceBoss(pausedBoss, 1u, &pausedOrder);
		const bool resumedAtSameStageSample =
			!pausedBoss.bAutomaticPatternSequencePausedForRevive &&
			0u == pausedBoss.iAutomaticPatternSequencePauseLastTick &&
			pausedBoss.strPatternId == pausedPatternId &&
			pausedBoss.strPatternStageId == pausedStageId &&
			pausedBoss.iPatternStageIndex == pausedStageIndex &&
			std::abs(pausedBoss.fActionElapsedSeconds - pausedActionElapsed) <
				0.0001f;
		for (std::uint32_t tick = 0u;
			tick < 600u && 0u == pausedBoss.iRotationStepIndex; ++tick)
		{
			advanceBoss(pausedBoss, 1u, &pausedOrder);
		}
		const bool advancedOnlyAfterRevive =
			1u == pausedBoss.iRotationStepIndex &&
			!pausedBoss.bMechanicLedgerRequiresReset;
		tests.Require(
			heldOrderedStepWhileDead && resumedAtSameStageSample &&
			advancedOnlyAfterRevive,
			"Pause any running ordered Valtan step at the same stage sample until revive, then resume its remaining timeline without losing the cursor");

		SERVER_WORLD_ENTITY disconnectedBoss = makeEntranceBoss();
		std::vector<std::string> disconnectedOrder;
		advanceBoss(disconnectedBoss, 1u, &disconnectedOrder);
		const SERVER_PLAYER disconnectedPlayer = entrancePlayers.begin()->second;
		entrancePlayers.clear();
		advanceBoss(disconnectedBoss, 1u, &disconnectedOrder);
		const bool emptyRoomFailedClosed =
			disconnectedOrder ==
				std::vector<std::string>{ "VALTAN_ENTRANCE_CINEMATIC" } &&
			0u == disconnectedBoss.iRotationStepIndex &&
			!disconnectedBoss.bAutomaticPatternSequenceStepRunning &&
			!disconnectedBoss.bAutomaticPatternSequencePausedForRevive &&
			0u == disconnectedBoss.iAutomaticPatternSequencePursuitTicksRemaining &&
			disconnectedBoss.bMechanicLedgerRequiresReset &&
			disconnectedBoss.strPatternId.empty();
		entrancePlayers.emplace(
			disconnectedPlayer.iPlayerId, disconnectedPlayer);
		tests.Require(
			emptyRoomFailedClosed,
			"Keep an empty-room or disconnect abort fail-closed instead of treating it as a revive pause");
		entrancePlayers.begin()->second.iCurrentHp = 1000000u;
		entrancePlayers.begin()->second.isCombatReady = true;
		entrancePlayers.begin()->second.fPositionX = 164.03f;
		entrancePlayers.begin()->second.fPositionZ = -122.06f;

		/* Rotations remain admitted for authoring and explicit Debug evaluation,
		but the Product sequence above is the only automatic selector consumer. */
		const BOSS_PATTERN_ROTATION_DEFINITION* openingSpan =
			catalog.Find_BossPatternRotation("ENCOUNTER_VALTAN", 1u, 160u);
		const BOSS_PATTERN_ROTATION_DEFINITION* secondSpan =
			catalog.Find_BossPatternRotation("ENCOUNTER_VALTAN", 1u, 129u);
		const BOSS_PATTERN_ROTATION_DEFINITION* firstLegacySpan =
			catalog.Find_BossPatternRotation("ENCOUNTER_VALTAN", 2u, 108u);
		const bool managedPoolsMatch = nullptr != openingSpan &&
			nullptr != secondSpan &&
			openingSpan->Candidates.size() == secondSpan->Candidates.size() &&
			std::equal(
				openingSpan->Candidates.begin(), openingSpan->Candidates.end(),
				secondSpan->Candidates.begin(),
				[](const BOSS_PATTERN_ROTATION_CANDIDATE& left,
					const BOSS_PATTERN_ROTATION_CANDIDATE& right)
				{
					return left.strPatternId == right.strPatternId &&
						left.iSelectionWeight == right.iSelectionWeight &&
						left.bEnabled == right.bEnabled;
				});
		tests.Require(
			nullptr != openingSpan &&
			BOSS_PATTERN_ROTATION_SELECTION_MODE::WEIGHTED_POOL ==
				openingSpan->eSelectionMode &&
			160u == openingSpan->iFromHealthBar &&
			130u == openingSpan->iToHealthBar &&
			5u == openingSpan->Candidates.size() &&
			openingSpan == catalog.Find_BossPatternRotation(
				"ENCOUNTER_VALTAN", 1u, 131u) &&
			nullptr != secondSpan && openingSpan != secondSpan &&
			BOSS_PATTERN_ROTATION_SELECTION_MODE::WEIGHTED_POOL ==
				secondSpan->eSelectionMode &&
			130u == secondSpan->iFromHealthBar &&
			109u == secondSpan->iToHealthBar &&
			secondSpan == catalog.Find_BossPatternRotation(
				"ENCOUNTER_VALTAN", 1u, 130u) &&
			managedPoolsMatch &&
			nullptr == catalog.Find_BossPatternRotation(
				"ENCOUNTER_VALTAN", 2u, 159u) &&
			nullptr != firstLegacySpan &&
			BOSS_PATTERN_ROTATION_SELECTION_MODE::
				ORDERED_INTRO_THEN_WEIGHTED == firstLegacySpan->eSelectionMode &&
			firstLegacySpan == catalog.Find_BossPatternRotation(
				"ENCOUNTER_VALTAN", 1u, 108u) &&
			nullptr == catalog.Find_BossPatternRotation(
				"ENCOUNTER_VALTAN", 1u, 1u),
			"Retain phase-owned rotation data for authoring without using it as automatic fallback");
	}
	{
		/* Each high-jump wave snapshots one axe per living raider and four static
		deterministic points around the authored arena centre. */
		CGameRoom volleyRoom{ LostArk::Shared::WORLD_ID::VALTAN_ARENA };
		tests.Require(volleyRoom.Initialize_WorldEntities(),
			"Initialize the Valtan room for the sky axe volley");
		/* The arena authors Valtan as a disabled Debug spawn, so the room owns
		no boss until one is activated. Stand one up directly. */
		SERVER_WORLD_ENTITY volleyEntity{};
		volleyEntity.iNetEntityId = 8300u;
		volleyEntity.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		volleyEntity.eAction = SERVER_ENTITY_ACTION::IDLE;
		volleyEntity.strArchetypeId = "BOSS_VALTAN";
		volleyEntity.strEncounterId = "ENCOUNTER_VALTAN";
		volleyEntity.iCurrentHp = 60000u;
		volleyEntity.iMaximumHp = 60000u;
		volleyEntity.iMaximumHealthBars = 160u;
		volleyEntity.iAttackPower = 100u;
		volleyEntity.iLastEvaluatedHealthBar = 160u;
		volleyEntity.fPositionX = 156.03f;
		volleyEntity.fPositionY = 22.99751f;
		volleyEntity.fPositionZ = -122.06f;
		volleyEntity.fSpawnPositionX = volleyEntity.fPositionX;
		volleyEntity.fSpawnPositionY = volleyEntity.fPositionY;
		volleyEntity.fSpawnPositionZ = volleyEntity.fPositionZ;
		/* A combat object belongs to a running pattern occurrence, so the boss
		must already have begun one. */
		volleyEntity.iPatternSequence = 1u;
		volleyEntity.strPatternId = "VALTAN_HIGH_JUMP";
		volleyEntity.strActionId = "valtan.attack.high-jump.airborne";
		volleyEntity.PinnedDefinitionRevision =
			volleyRoom.m_GameplayCatalog.Get_ActiveRevision();
		volleyRoom.m_WorldEntities.push_back(volleyEntity);
		SERVER_WORLD_ENTITY* volleyBoss = &volleyRoom.m_WorldEntities.back();
		for (std::uint32_t index = 0u; index < 4u; ++index)
		{
			SERVER_PLAYER raider{};
			raider.iPlayerId = 8100u + index;
			raider.iNetEntityId = 8200u + index;
			raider.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			raider.iCurrentHp = 5000u;
			raider.iMaximumHp = 5000u;
			raider.isCombatReady = true;
			raider.fPositionX = 137.f + static_cast<float>(index);
			raider.fPositionZ = -113.5f;
			volleyRoom.m_Players.emplace(raider.iPlayerId, raider);
		}
		/* One raider is down, so the volley must pass them over. */
		volleyRoom.m_Players.at(8103u).iCurrentHp = 0u;
		volleyRoom.m_Players.at(8103u).eAction = PLAYER_ACTION_STATE::DEAD;
		const bool staged = nullptr != volleyBoss &&
			volleyRoom.Apply_BossPatternStageActions(
				*volleyBoss, "VALTAN_HIGH_JUMP",
				"valtan.attack.high-jump.airborne",
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 900u);
		const auto& liveObjects =
			volleyRoom.m_CombatObjectRuntime.Get_LiveObjects();
		std::set<std::uint32_t> lockedTargets;
		std::vector<std::uint32_t> lockedTargetOrder;
		for (const SERVER_COMBAT_OBJECT& object : liveObjects)
		{
			lockedTargets.insert(object.iLockedTargetNetEntityId);
			lockedTargetOrder.push_back(
				static_cast<std::uint32_t>(object.iLockedTargetNetEntityId));
		}
		std::vector<S2C_COMBAT_OBJECT_SPAWNED> initialSpawned;
		std::vector<S2C_COMBAT_OBJECT_DESPAWNED> initialDespawned;
		volleyRoom.m_CombatObjectRuntime.Drain_Lifecycle(
			initialSpawned, initialDespawned);
		std::vector<COMBAT_OBJECT_SNAPSHOT> initialSnapshots;
		const bool builtInitialSnapshots =
			volleyRoom.m_CombatObjectRuntime.Build_Snapshots(initialSnapshots);
		const GameplayDataRevision volleyRevision =
			volleyRoom.m_GameplayCatalog.Get_ActiveRevision();
		bool arenaRandomValid = nullptr != volleyBoss && 7u == liveObjects.size();
		for (std::size_t left = 3u;
			arenaRandomValid && left < liveObjects.size(); ++left)
		{
			const SERVER_COMBAT_OBJECT& object = liveObjects[left];
			const float centerDeltaX =
				object.LiveState.CurrentPose.fPositionX -
				volleyBoss->fSpawnPositionX;
			const float centerDeltaZ =
				object.LiveState.CurrentPose.fPositionZ -
				volleyBoss->fSpawnPositionZ;
			arenaRandomValid =
				!object.bTrackLockedTargetUntilFirstPulse &&
				INVALID_NET_ENTITY_ID == object.iLockedTargetNetEntityId &&
				centerDeltaX * centerDeltaX + centerDeltaZ * centerDeltaZ <=
					14.f * 14.f + 0.001f &&
				volleyRoom.m_ServerNavigation.Is_PointWalkableExact(
					object.LiveState.CurrentPose.fPositionX,
					object.LiveState.CurrentPose.fPositionZ);
			for (std::size_t right = left + 1u;
				arenaRandomValid && right < liveObjects.size(); ++right)
			{
				const float deltaX = object.LiveState.CurrentPose.fPositionX -
					liveObjects[right].LiveState.CurrentPose.fPositionX;
				const float deltaZ = object.LiveState.CurrentPose.fPositionZ -
					liveObjects[right].LiveState.CurrentPose.fPositionZ;
				arenaRandomValid = deltaX * deltaX + deltaZ * deltaZ +
					0.0001f >= 49.f;
			}
		}
		const bool reliableAndSnapshotPins =
			7u == initialSpawned.size() && initialDespawned.empty() &&
			7u == initialSnapshots.size() &&
			std::all_of(initialSpawned.begin(), initialSpawned.end(),
				[&volleyRevision](const S2C_COMBAT_OBJECT_SPAWNED& message)
				{
					CPacketWriter writer;
					return message.PinnedDefinitionRevision == volleyRevision &&
						Write_Message(writer, message);
				}) &&
			std::all_of(initialSnapshots.begin(), initialSnapshots.end(),
				[&volleyRevision](const COMBAT_OBJECT_SNAPSHOT& snapshot)
				{
					return snapshot.PinnedDefinitionRevision == volleyRevision;
				});
		tests.Require(
			staged && 7u == liveObjects.size() &&
			4u == lockedTargets.size() &&
			0u == lockedTargets.count(8203u) &&
			lockedTargetOrder == std::vector<std::uint32_t>{
				8200u, 8201u, 8202u,
				static_cast<std::uint32_t>(INVALID_NET_ENTITY_ID),
				static_cast<std::uint32_t>(INVALID_NET_ENTITY_ID),
				static_cast<std::uint32_t>(INVALID_NET_ENTITY_ID),
				static_cast<std::uint32_t>(INVALID_NET_ENTITY_ID) } &&
			arenaRandomValid && builtInitialSnapshots && reliableAndSnapshotPins,
			"Deal spawn-position axes to living raiders plus four static arena axes");
		std::vector<SERVER_COMBAT_OBJECT_POSE> spawnPoses;
		for (const SERVER_COMBAT_OBJECT& object : liveObjects)
			spawnPoses.push_back(object.LiveState.CurrentPose);
		for (auto& [raiderId, raider] : volleyRoom.m_Players)
		{
			(void)raiderId;
			if (0u != raider.iCurrentHp)
				raider.fPositionZ += 0.5f;
		}
		std::vector<DAMAGE_EVENT> volleyDamageEvents;
		volleyRoom.m_CombatObjectRuntime.Update(
			volleyRoom.m_Players, volleyRoom.m_WorldEntities,
			volleyRoom.m_GameplayCatalog, 1.f / 30.f, 901u,
			volleyDamageEvents);
		bool fixedAtSpawn = spawnPoses.size() == liveObjects.size();
		for (std::size_t index = 0u;
			fixedAtSpawn && index < liveObjects.size(); ++index)
		{
			fixedAtSpawn =
				!liveObjects[index].bTrackLockedTargetUntilFirstPulse &&
				std::abs(spawnPoses[index].fPositionX -
					liveObjects[index].LiveState.CurrentPose.fPositionX) < 0.001f &&
				std::abs(spawnPoses[index].fPositionZ -
					liveObjects[index].LiveState.CurrentPose.fPositionZ) < 0.001f;
		}
		tests.Require(fixedAtSpawn,
			"Keep every sky axe at the pose resolved when its wave spawned");
		for (std::uint32_t tick = 902u; tick <= 936u; ++tick)
		{
			volleyRoom.m_CombatObjectRuntime.Update(
				volleyRoom.m_Players, volleyRoom.m_WorldEntities,
				volleyRoom.m_GameplayCatalog, 1.f / 30.f, tick,
				volleyDamageEvents);
		}
		std::vector<SERVER_COMBAT_OBJECT_POSE> firstPulsePoses;
		for (const SERVER_COMBAT_OBJECT& object : liveObjects)
			firstPulsePoses.push_back(object.LiveState.CurrentPose);
		for (auto& [raiderId, raider] : volleyRoom.m_Players)
		{
			(void)raiderId;
			if (0u != raider.iCurrentHp)
				raider.fPositionZ += 5.f;
		}
		volleyRoom.m_CombatObjectRuntime.Update(
			volleyRoom.m_Players, volleyRoom.m_WorldEntities,
			volleyRoom.m_GameplayCatalog, 1.f / 30.f, 937u,
			volleyDamageEvents);
		bool fixedAfterFirstPulse = firstPulsePoses.size() == liveObjects.size();
		for (std::size_t index = 0u;
			fixedAfterFirstPulse && index < liveObjects.size(); ++index)
		{
			fixedAfterFirstPulse =
				std::abs(firstPulsePoses[index].fPositionX -
					liveObjects[index].LiveState.CurrentPose.fPositionX) < 0.001f &&
				std::abs(firstPulsePoses[index].fPositionZ -
					liveObjects[index].LiveState.CurrentPose.fPositionZ) < 0.001f;
		}
		tests.Require(fixedAfterFirstPulse,
			"Keep each snapshotted sky axe fixed after its first timed pulse");

		auto* mutableVolleyPatterns = const_cast<
			std::vector<BOSS_PATTERN_DEFINITION>*>(
				volleyRoom.m_GameplayCatalog.Find_BossPatterns(
					"ENCOUNTER_VALTAN"));
		BOSS_PATTERN_STAGE_ACTION* typedVolleyAction = nullptr;
		if (nullptr != mutableVolleyPatterns)
		{
			for (BOSS_PATTERN_DEFINITION& pattern : *mutableVolleyPatterns)
			{
				if ("VALTAN_HIGH_JUMP" != pattern.strPatternId)
					continue;
				for (BOSS_PATTERN_STAGE_DEFINITION& stage : pattern.Stages)
				{
					if ("valtan.attack.high-jump.airborne" != stage.strActionId)
						continue;
					for (BOSS_PATTERN_STAGE_ACTION& action : stage.Actions)
					{
						if (BOSS_PATTERN_STAGE_ACTION_KIND::
							SPAWN_COMBAT_OBJECT_VOLLEY == action.eKind)
						{
							typedVolleyAction = &action;
						}
					}
				}
			}
		}
		tests.Require(nullptr != typedVolleyAction,
			"Publish HIGH_JUMP as a typed per-alive-player volley action");
		if (nullptr != typedVolleyAction && nullptr != volleyBoss)
		{
			const BOSS_PATTERN_STAGE_ACTION originalVolleyAction =
				*typedVolleyAction;
			volleyRoom.m_CombatObjectRuntime.Reset();
			volleyBoss->iActionStartTick = 1000u;
			volleyBoss->iAppliedPatternStageSpawnWaveCount = 0u;
			for (auto& [raiderId, raider] : volleyRoom.m_Players)
			{
				raider.iCurrentHp = raiderId <= 8101u ? 5000u : 0u;
				raider.eAction = raiderId <= 8101u ?
					PLAYER_ACTION_STATE::NONE : PLAYER_ACTION_STATE::DEAD;
				raider.fPositionX = 151.f +
					static_cast<float>(raiderId - 8100u) * 2.f;
				raider.fPositionY = volleyBoss->fSpawnPositionY;
				raider.fPositionZ = -122.f;
			}
			const bool firstWaveCommitted =
				volleyRoom.Apply_BossPatternStageActions(
					*volleyBoss, "VALTAN_HIGH_JUMP",
					"valtan.attack.high-jump.airborne",
					BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 1000u);
			volleyBoss->iAppliedPatternStageSpawnWaveCount =
				firstWaveCommitted ? 1u : 0u;
			const bool earlySecondWaveAccepted =
				volleyRoom.Apply_BossPatternScheduledSpawnWave(
					*volleyBoss, 1039u);
			const std::size_t beforeSecondWave =
				volleyRoom.m_CombatObjectRuntime.Get_LiveObjects().size();
			volleyRoom.m_Players.at(8101u).iCurrentHp = 0u;
			volleyRoom.m_Players.at(8101u).eAction =
				PLAYER_ACTION_STATE::DEAD;
			const bool secondWaveCommitted =
				volleyRoom.Apply_BossPatternScheduledSpawnWave(
					*volleyBoss, 1040u);
			volleyRoom.m_Players.at(8101u).iCurrentHp = 5000u;
			volleyRoom.m_Players.at(8101u).eAction =
				PLAYER_ACTION_STATE::NONE;
			volleyRoom.m_Players.at(8102u).iCurrentHp = 5000u;
			volleyRoom.m_Players.at(8102u).eAction =
				PLAYER_ACTION_STATE::NONE;
			const bool thirdWaveCommitted =
				volleyRoom.Apply_BossPatternScheduledSpawnWave(
					*volleyBoss, 1080u);
			const bool duplicateThirdWaveAccepted =
				volleyRoom.Apply_BossPatternScheduledSpawnWave(
					*volleyBoss, 1081u);
			std::array<std::size_t, 3u> playerAxesByWave{};
			std::array<std::size_t, 3u> arenaAxesByWave{};
			std::size_t trackingAxisCount = 0u;
			for (const SERVER_COMBAT_OBJECT& object :
				volleyRoom.m_CombatObjectRuntime.Get_LiveObjects())
			{
				std::size_t wave = 3u;
				if (1000u == object.iSpawnTick) wave = 0u;
				else if (1040u == object.iSpawnTick) wave = 1u;
				else if (1080u == object.iSpawnTick) wave = 2u;
				if (wave >= 3u) continue;
				if (object.bTrackLockedTargetUntilFirstPulse)
					++trackingAxisCount;
				if (INVALID_NET_ENTITY_ID != object.iLockedTargetNetEntityId)
					++playerAxesByWave[wave];
				else
					++arenaAxesByWave[wave];
			}
			std::vector<SERVER_COMBAT_OBJECT_LOCKED_TARGET> deterministicA;
			std::vector<SERVER_COMBAT_OBJECT_LOCKED_TARGET> deterministicB;
			const BOSS_COMBAT_OBJECT_DEFINITION* scheduledDefinition =
				volleyRoom.m_GameplayCatalog.Find_BossCombatObject(
					"combatobject.valtan.high-jump.target-axe");
			const bool deterministicResolved = nullptr != scheduledDefinition &&
				volleyRoom.Resolve_ArenaRandomVolleyOrigins(
					*volleyBoss, *typedVolleyAction, *scheduledDefinition,
					2u, deterministicA) &&
				volleyRoom.Resolve_ArenaRandomVolleyOrigins(
					*volleyBoss, *typedVolleyAction, *scheduledDefinition,
					2u, deterministicB);
			bool deterministicSame = deterministicResolved &&
				deterministicA.size() == deterministicB.size();
			for (std::size_t index = 0u;
				deterministicSame && index < deterministicA.size(); ++index)
			{
				deterministicSame =
					deterministicA[index].fPositionX ==
						deterministicB[index].fPositionX &&
					deterministicA[index].fPositionY ==
						deterministicB[index].fPositionY &&
					deterministicA[index].fPositionZ ==
						deterministicB[index].fPositionZ;
			}
			tests.Require(
				firstWaveCommitted && earlySecondWaveAccepted &&
				6u == beforeSecondWave && secondWaveCommitted &&
				thirdWaveCommitted && duplicateThirdWaveAccepted &&
				18u == volleyRoom.m_CombatObjectRuntime.Get_LiveObjects().size() &&
				3u == volleyBoss->iAppliedPatternStageSpawnWaveCount &&
				0u == trackingAxisCount &&
				playerAxesByWave == std::array<std::size_t, 3u>{ 2u, 1u, 3u } &&
				arenaAxesByWave == std::array<std::size_t, 3u>{ 4u, 4u, 4u } &&
					deterministicSame,
				"Commit snapshot-at-spawn player/random sky-axe waves at 0/40/80 ticks");

			/* Exercise the real room-tick order after every raider dies. The brain
			must keep the target-independent AIRBORNE stage alive long enough for
			the scheduler to commit its remaining arena-only waves. */
			volleyRoom.m_CombatObjectRuntime.Reset();
			*volleyBoss = volleyEntity;
			volleyBoss->eAction = SERVER_ENTITY_ACTION::PATTERN_WINDUP;
			volleyBoss->strPatternStageId = "AIRBORNE";
			volleyBoss->iPatternStageIndex = 1u;
			volleyBoss->iPatternStageDurationMs = 4000u;
			volleyBoss->iActionStartTick = 1000u;
			volleyBoss->iPatternStageFirstEvaluationTick = 1000u;
			volleyBoss->iAppliedPatternStageSpawnWaveCount = 0u;
			for (auto& [raiderId, raider] : volleyRoom.m_Players)
			{
				raider.iCurrentHp = 8100u == raiderId ? 5000u : 0u;
				raider.isCombatReady = true;
				raider.eAction = 8100u == raiderId ?
					PLAYER_ACTION_STATE::NONE : PLAYER_ACTION_STATE::DEAD;
			}
			const bool roomTickFirstWave =
				volleyRoom.Apply_BossPatternStageActions(
					*volleyBoss, "VALTAN_HIGH_JUMP",
					"valtan.attack.high-jump.airborne",
					BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 1000u);
			volleyBoss->iAppliedPatternStageSpawnWaveCount =
				roomTickFirstWave ? 1u : 0u;
			for (auto& [raiderId, raider] : volleyRoom.m_Players)
			{
				(void)raiderId;
				raider.iCurrentHp = 0u;
				raider.eAction = PLAYER_ACTION_STATE::DEAD;
			}
			volleyRoom.m_iServerTick = 1039u;
			volleyRoom.Update_WorldEntities(1.f / 30.f);
			const bool secondArenaOnlyWave =
				volleyRoom.m_isReady &&
				"VALTAN_HIGH_JUMP" == volleyBoss->strPatternId &&
				2u == volleyBoss->iAppliedPatternStageSpawnWaveCount &&
				9u == volleyRoom.m_CombatObjectRuntime.Get_LiveObjects().size();
			volleyRoom.m_iServerTick = 1079u;
			volleyRoom.Update_WorldEntities(1.f / 30.f);
			tests.Require(
				roomTickFirstWave && secondArenaOnlyWave && volleyRoom.m_isReady &&
				"VALTAN_HIGH_JUMP" == volleyBoss->strPatternId &&
				3u == volleyBoss->iAppliedPatternStageSpawnWaveCount &&
				13u == volleyRoom.m_CombatObjectRuntime.Get_LiveObjects().size(),
				"Keep HIGH_JUMP alive across a party wipe and commit both arena-only room-tick waves");

			volleyRoom.m_CombatObjectRuntime.Reset();
			for (auto& [raiderId, raider] : volleyRoom.m_Players)
			{
				raider.iCurrentHp = 8100u == raiderId ? 5000u : 0u;
				raider.eAction = 8100u == raiderId ?
					PLAYER_ACTION_STATE::NONE : PLAYER_ACTION_STATE::DEAD;
			}
			SERVER_PLAYER& radialTarget = volleyRoom.m_Players.at(8100u);
			radialTarget.fPositionX = 137.f;
			radialTarget.fPositionY = volleyBoss->fPositionY;
			radialTarget.fPositionZ = -105.5f;
			typedVolleyAction->iValue = 8u;
			typedVolleyAction->Volley.iCountPerResolvedTarget = 8u;
			typedVolleyAction->Volley.eLayout =
				BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL;
			typedVolleyAction->Volley.fRadiusM = 12.f;
			typedVolleyAction->Volley.fStartAngleDegrees = 0.f;
			typedVolleyAction->Volley.fAngleStepDegrees = 45.f;
			typedVolleyAction->Volley.bAllowOverlap = false;
			typedVolleyAction->Volley.iMaximumTotalObjects = 32u;
			typedVolleyAction->Volley.iArenaRandomCount = 0u;
			typedVolleyAction->Volley.fArenaRandomRadiusM = 0.f;
			typedVolleyAction->Volley.fArenaHeightToleranceM = 0.f;
			typedVolleyAction->Volley.eArenaAnchorPolicy =
				BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::NONE;
			const auto radialFirstCombatObjectId =
				volleyRoom.m_CombatObjectRuntime.Begin_Transaction().
					iNextCombatObjectId;
			const bool radialStaged =
				volleyRoom.Apply_BossPatternStageActions(
					*volleyBoss, "VALTAN_HIGH_JUMP",
					"valtan.attack.high-jump.airborne",
					BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 910u);
			const auto& radialObjects =
				volleyRoom.m_CombatObjectRuntime.Get_LiveObjects();
			std::set<std::pair<int, int>> quantizedRadialPositions;
			bool radialIdentityIsStable = radialObjects.size() == 8u;
			for (std::size_t ordinal = 0u;
				radialIdentityIsStable && ordinal < radialObjects.size(); ++ordinal)
			{
				const SERVER_COMBAT_OBJECT& object = radialObjects[ordinal];
				radialIdentityIsStable =
					object.iCombatObjectId ==
						radialFirstCombatObjectId + ordinal &&
					object.iLockedTargetNetEntityId == radialTarget.iNetEntityId &&
					910u == object.iSpawnTick;
				quantizedRadialPositions.emplace(
					static_cast<int>(std::lround(
						object.LiveState.CurrentPose.fPositionX * 1000.f)),
					static_cast<int>(std::lround(
						object.LiveState.CurrentPose.fPositionZ * 1000.f)));
			}
			tests.Require(
				radialStaged && radialIdentityIsStable &&
				8u == quantizedRadialPositions.size(),
				"Spawn all eight radial ordinals in deterministic identity order on one tick");

			volleyRoom.m_CombatObjectRuntime.Reset();
			for (auto& [raiderId, raider] : volleyRoom.m_Players)
			{
				(void)raiderId;
				raider.iCurrentHp = 5000u;
				raider.eAction = PLAYER_ACTION_STATE::NONE;
			}
			SERVER_PLAYER fifthRaider = radialTarget;
			fifthRaider.iPlayerId = 8104u;
			fifthRaider.iNetEntityId = 8204u;
			volleyRoom.m_Players.emplace(fifthRaider.iPlayerId, fifthRaider);
			const bool overTotalAccepted =
				volleyRoom.Apply_BossPatternStageActions(
					*volleyBoss, "VALTAN_HIGH_JUMP",
					"valtan.attack.high-jump.airborne",
					BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 920u);
			tests.Require(
				!overTotalAccepted &&
				volleyRoom.m_CombatObjectRuntime.Get_LiveObjects().empty(),
				"Reject a five-player eight-axe volley above the total-32 bound with zero spawn");

			volleyRoom.m_CombatObjectRuntime.Reset();
			for (auto& [raiderId, raider] : volleyRoom.m_Players)
			{
				raider.iCurrentHp = raiderId <= 8101u ? 5000u : 0u;
				raider.eAction = raiderId <= 8101u ?
					PLAYER_ACTION_STATE::NONE : PLAYER_ACTION_STATE::DEAD;
			}
			volleyRoom.m_Players.at(8100u).fPositionX = 137.f;
			volleyRoom.m_Players.at(8100u).fPositionZ = -105.5f;
			volleyRoom.m_Players.at(8101u).fPositionX = 138.f;
			volleyRoom.m_Players.at(8101u).fPositionZ = -105.5f;
			typedVolleyAction->iValue = 1u;
			typedVolleyAction->Volley.iCountPerResolvedTarget = 1u;
			typedVolleyAction->Volley.eLayout =
				BOSS_COMBAT_OBJECT_LAYOUT_KIND::SINGLE;
			typedVolleyAction->Volley.fRadiusM = 0.f;
			typedVolleyAction->Volley.fStartAngleDegrees = 0.f;
			typedVolleyAction->Volley.fAngleStepDegrees = 0.f;
			const bool overlappingAccepted =
				volleyRoom.Apply_BossPatternStageActions(
					*volleyBoss, "VALTAN_HIGH_JUMP",
					"valtan.attack.high-jump.airborne",
					BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 930u);
			tests.Require(
				overlappingAccepted &&
				2u == volleyRoom.m_CombatObjectRuntime.Get_LiveObjects().size(),
				"Allow different players' resolved volley positions to overlap");

			volleyRoom.m_CombatObjectRuntime.Reset();
			volleyRoom.m_Players.at(8101u).iCurrentHp = 0u;
			volleyRoom.m_Players.at(8101u).eAction = PLAYER_ACTION_STATE::DEAD;
			volleyRoom.m_Players.at(8100u).fPositionX = 100000.f;
			volleyRoom.m_Players.at(8100u).fPositionZ = 100000.f;
			const bool outsideNavigationAccepted =
				volleyRoom.Apply_BossPatternStageActions(
					*volleyBoss, "VALTAN_HIGH_JUMP",
					"valtan.attack.high-jump.airborne",
					BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 940u);
			tests.Require(
				!outsideNavigationAccepted &&
				volleyRoom.m_CombatObjectRuntime.Get_LiveObjects().empty(),
				"Reject a target-center volley outside navigation with zero spawn");

			volleyRoom.m_CombatObjectRuntime.Reset();
			const BOSS_COMBAT_OBJECT_DEFINITION* volleyDefinition =
				volleyRoom.m_GameplayCatalog.Find_BossCombatObject(
					"combatobject.valtan.high-jump.target-axe");
			SERVER_COMBAT_OBJECT_LOCKED_TARGET capacityTarget{};
			capacityTarget.iNetEntityId = radialTarget.iNetEntityId;
			capacityTarget.fPositionX = volleyBoss->fPositionX;
			capacityTarget.fPositionY = volleyBoss->fPositionY;
			capacityTarget.fPositionZ = volleyBoss->fPositionZ;
			BOSS_COMBAT_OBJECT_VOLLEY capacityVolley{};
			capacityVolley.ePolicy =
				BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER;
			capacityVolley.iCountPerResolvedTarget = 8u;
			capacityVolley.eLayout =
				BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL;
			capacityVolley.fRadiusM = 12.f;
			capacityVolley.fAngleStepDegrees = 45.f;
			capacityVolley.iMaximumTotalObjects = 32u;
			bool filledSnapshotCapacity = nullptr != volleyDefinition;
			for (std::uint32_t batch = 0u;
				filledSnapshotCapacity && batch < 16u; ++batch)
			{
				SERVER_COMBAT_OBJECT_TRANSACTION transaction =
					volleyRoom.m_CombatObjectRuntime.Begin_Transaction();
				std::string capacityStatus;
				filledSnapshotCapacity =
					volleyRoom.m_CombatObjectRuntime.Stage_BossCombatObject(
						transaction, *volleyBoss, &capacityTarget,
						*volleyDefinition, &capacityVolley,
						volleyRoom.m_GameplayCatalog, 8u, 950u + batch,
						capacityStatus) &&
					volleyRoom.m_CombatObjectRuntime.Commit(
						std::move(transaction));
			}
			volleyRoom.m_CombatObjectRuntime.Discard_PendingLifecycle();
			BOSS_COMBAT_OBJECT_VOLLEY oneMoreVolley{};
			oneMoreVolley.ePolicy =
				BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER;
			oneMoreVolley.iCountPerResolvedTarget = 1u;
			oneMoreVolley.eLayout =
				BOSS_COMBAT_OBJECT_LAYOUT_KIND::SINGLE;
			oneMoreVolley.iMaximumTotalObjects = 32u;
			SERVER_COMBAT_OBJECT_TRANSACTION rejectedTransaction =
				volleyRoom.m_CombatObjectRuntime.Begin_Transaction();
			std::string capacityStatus;
			const bool acceptedOverSnapshot = nullptr != volleyDefinition &&
				volleyRoom.m_CombatObjectRuntime.Stage_BossCombatObject(
					rejectedTransaction, *volleyBoss, &capacityTarget,
					*volleyDefinition, &oneMoreVolley,
					volleyRoom.m_GameplayCatalog, 1u, 999u, capacityStatus);
			std::vector<S2C_COMBAT_OBJECT_SPAWNED> pendingAfterFailure;
			std::vector<S2C_COMBAT_OBJECT_DESPAWNED> despawnedAfterFailure;
			volleyRoom.m_CombatObjectRuntime.Drain_Lifecycle(
				pendingAfterFailure, despawnedAfterFailure);
			tests.Require(
				filledSnapshotCapacity &&
				128u == volleyRoom.m_CombatObjectRuntime.Get_LiveObjects().size() &&
				!acceptedOverSnapshot && rejectedTransaction.Objects.empty() &&
				pendingAfterFailure.empty(),
				"Reject object 129 before allocation and emit no partial lifecycle edge");
			*typedVolleyAction = originalVolleyAction;
		}
	}
	{
		auto phaseRoomStorage = std::make_unique<CGameRoom>(
			LostArk::Shared::WORLD_ID::VALTAN_ARENA);
		CGameRoom& phaseRoom = *phaseRoomStorage;
		SERVER_WORLD_ENTITY phaseBoss{};
		phaseBoss.iNetEntityId = 8400u;
		phaseBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		phaseBoss.strArchetypeId = "BOSS_VALTAN";
		phaseBoss.strEncounterId = "ENCOUNTER_VALTAN";
		phaseBoss.strPatternId = "VALTAN_ARENA_BREAK_109";
		phaseBoss.strPatternStageId = "IMPACT";
		phaseBoss.strActionId =
			"valtan.mechanic.arena-break-109.impact";
		phaseBoss.iPatternStageIndex = 2u;
		phaseBoss.iPatternSequence = 1u;
		phaseBoss.iPhase = 1u;
		phaseBoss.BossCombat.iStateRevision = 10u;
		phaseBoss.PinnedDefinitionRevision =
			phaseRoom.m_GameplayCatalog.Get_ActiveRevision();
		const bool phaseCommitted =
			phaseRoom.Apply_BossPatternStageActions(
				phaseBoss, phaseBoss.strPatternId, phaseBoss.strActionId,
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 1000u);
		const std::uint32_t phaseRevision =
			phaseBoss.BossCombat.iStateRevision;
		const bool duplicatePhaseCommitted =
			phaseRoom.Apply_BossPatternStageActions(
				phaseBoss, phaseBoss.strPatternId, phaseBoss.strActionId,
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 1001u);
		tests.Require(
			phaseCommitted && duplicatePhaseCommitted &&
			2u == phaseBoss.iPhase && 11u == phaseRevision &&
			phaseRevision == phaseBoss.BossCombat.iStateRevision,
			"Commit phase two exactly once at 109 IMPACT ENTER");

		auto* mutablePhasePatterns = const_cast<
			std::vector<BOSS_PATTERN_DEFINITION>*>(
				phaseRoom.m_GameplayCatalog.Find_BossPatterns(
					"ENCOUNTER_VALTAN"));
		BOSS_PATTERN_STAGE_DEFINITION* impactStage = nullptr;
		if (nullptr != mutablePhasePatterns)
		{
			for (BOSS_PATTERN_DEFINITION& pattern : *mutablePhasePatterns)
			{
				if ("VALTAN_ARENA_BREAK_109" != pattern.strPatternId)
					continue;
				for (BOSS_PATTERN_STAGE_DEFINITION& stage : pattern.Stages)
				{
					if ("IMPACT" == stage.strStageId)
						impactStage = &stage;
				}
			}
		}
		if (nullptr != impactStage)
		{
			BOSS_PATTERN_STAGE_ACTION invalidAction{};
			invalidAction.eTrigger =
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER;
			invalidAction.eKind =
				BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG;
			invalidAction.strTargetId = "boss.flag.unresolved";
			invalidAction.iValue = 1u;
			impactStage->Actions.push_back(invalidAction);
		}
		phaseBoss.iPhase = 1u;
		phaseBoss.BossCombat.iStateRevision = 20u;
		phaseBoss.BossCombat.iFlags = 0u;
		const std::size_t objectCountBeforeRejectedPhase =
			phaseRoom.m_CombatObjectRuntime.Get_LiveObjects().size();
		const bool partialPhaseAccepted = nullptr != impactStage &&
			phaseRoom.Apply_BossPatternStageActions(
				phaseBoss, phaseBoss.strPatternId, phaseBoss.strActionId,
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 1002u);
		if (nullptr != impactStage)
			impactStage->Actions.pop_back();
		tests.Require(
			!partialPhaseAccepted && 1u == phaseBoss.iPhase &&
			20u == phaseBoss.BossCombat.iStateRevision &&
			0u == phaseBoss.BossCombat.iFlags &&
			objectCountBeforeRejectedPhase ==
				phaseRoom.m_CombatObjectRuntime.Get_LiveObjects().size(),
			"Reject an invalid second stage action without partial phase, flag, or object commit");
	}

	SERVER_WORLD_ENTITY openingChargeBoss{};
	openingChargeBoss.iNetEntityId = 901u;
	openingChargeBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
	openingChargeBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
	openingChargeBoss.strArchetypeId = "BOSS_VALTAN";
	openingChargeBoss.strEncounterId = "ENCOUNTER_VALTAN";
	openingChargeBoss.iCurrentHp = 59625u;
	openingChargeBoss.iMaximumHp = 60000u;
	openingChargeBoss.iMaximumHealthBars = 160u;
	openingChargeBoss.iLastEvaluatedHealthBar = 160u;
	openingChargeBoss.iPhaseTwoHpPercent = 50u;
	openingChargeBoss.fPositionX = 151.f;
	openingChargeBoss.fPositionY = 22.97f;
	openingChargeBoss.fPositionZ = -122.f;
	openingChargeBoss.fEngageDistance = 35.f;
	openingChargeBoss.fMoveSpeed = 3.f;
	openingChargeBoss.bIntroPatternConsumed = true;
	openingChargeBoss.bAutomaticPatternSequenceAuditionOverride = true;
	brain.Update(
		openingChargeBoss, players, catalog, navigation,
		1.f / 30.f, 200u, {}, valtanDamageEvents);
	tests.Require(
		openingChargeBoss.strPatternId == "VALTAN_ARMOR_BREAK_OPENING" &&
		openingChargeBoss.strPatternStageId == "WALL_CHARGE" &&
		openingChargeBoss.strActionId ==
			"valtan.mechanic.armor-break-opening.charge" &&
		std::abs(openingChargeBoss.fPatternForcedMotionSpeed -
			(20.f / 1.5f)) <= 0.001f,
		"Use the opening stage's authored arena charge distance");
	SERVER_WORLD_ENTITY continuousOpeningCharge = openingChargeBoss;
	const float openingStartX = continuousOpeningCharge.fPositionX;
	const float openingStartZ = continuousOpeningCharge.fPositionZ;
	const float openingStepDistance = (20.f / 1.5f) / 30.f;
	bool openingStepsContinuous = true;
	for (std::uint32_t tick = 0u; tick < 15u; ++tick)
	{
		float proposedX = 0.f;
		float proposedZ = 0.f;
		if (!brain.Try_BuildStageMotion(
				continuousOpeningCharge, 1.f / 30.f,
				proposedX, proposedZ))
		{
			openingStepsContinuous = false;
			break;
		}
		const float stepX = proposedX - continuousOpeningCharge.fPositionX;
		const float stepZ = proposedZ - continuousOpeningCharge.fPositionZ;
		openingStepsContinuous = openingStepsContinuous &&
			std::abs(std::sqrt(stepX * stepX + stepZ * stepZ) -
				openingStepDistance) <= 0.001f;
		continuousOpeningCharge.fPositionX = proposedX;
		continuousOpeningCharge.fPositionZ = proposedZ;
	}
	const float openingTravelX =
		continuousOpeningCharge.fPositionX - openingStartX;
	const float openingTravelZ =
		continuousOpeningCharge.fPositionZ - openingStartZ;
	tests.Require(
		openingStepsContinuous &&
		std::abs(std::sqrt(
			openingTravelX * openingTravelX + openingTravelZ * openingTravelZ) -
			openingStepDistance * 15.f) <= 0.002f,
		"Advance the opening charge through continuous fixed-tick intermediate positions");

	{
		SERVER_PLAYER meleePlayer{};
		meleePlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		meleePlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		meleePlayer.iCurrentHp = 1000;
		meleePlayer.iMaximumHp = 1000;
		meleePlayer.iCurrentResource = 1000;
		meleePlayer.iMaximumResource = 1000;
		meleePlayer.fPositionX = 0.f;
		meleePlayer.fPositionZ = 0.f;
		SERVER_WORLD_ENTITY meleeBoss{};
		meleeBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		meleeBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		meleeBoss.strArchetypeId = "BOSS_VALTAN";
		meleeBoss.iCurrentHp = 20000;
		meleeBoss.iMaximumHp = 20000;
		meleeBoss.fPositionX = 0.f;
		/* 34090 reaches 2.8 on its own; 3.5 is inside reach only because the
		boss's 3.0 collision radius extends the centre-to-centre test. */
		meleeBoss.fPositionZ = 3.5f;
		std::vector<SERVER_WORLD_ENTITY> meleeEntities{ meleeBoss };
		C2S_USE_SKILL melee{};
		melee.iClientSequence = 1;
		melee.iSkillId = 34090;
		melee.fAimX = 0.f;
		melee.fAimZ = 3.5f;
		CPlayerSkillSystem meleeSkills;
		std::vector<DAMAGE_EVENT> meleeDamageEvents;
		tests.Require(meleeSkills.Try_Start(meleePlayer, melee, catalog, 10),
			"Approve melee skill command");
		for (std::uint32_t tick = 11; tick < 60; ++tick)
		{
			meleeSkills.Update(
				meleePlayer, meleeEntities, catalog, nullptr, nullptr,
				1.f / 30.f, tick, meleeDamageEvents);
		}
		tests.Require(20000u - 10500u == meleeEntities[0].iCurrentHp,
			"Reach the boss through its collision radius");
	}

	{
		namespace fs = std::filesystem;
		const fs::path triggerRoot =
			fs::temp_directory_path() / L"LostArkWorldTriggerContractTest";
		std::error_code prepareError;
		fs::remove_all(triggerRoot, prepareError);
		fs::create_directories(triggerRoot / L"World");
		const fs::path bootstrapPath =
			triggerRoot / L"World" / L"VALTAN_ARENA.worldbootstrap";
		const auto writeTriggerBootstrap =
			[&bootstrapPath](
				const float durationSeconds,
				const std::uint32_t npcActionDurationMs = 100u,
				const float npcLookYawDegrees = 90.f,
				const std::uint32_t npcActionWeight = 1u,
				const bool referenceDisabledNpc = false,
				const char* npcActionId = "npc.ambient.look")
			{
				std::ofstream bootstrap(bootstrapPath, std::ios::binary);
				bootstrap <<
					"LOSTARK_WORLD_BOOTSTRAP\t7\tVALTAN_ARENA"
					"\tLV_LUT_HEARTRB_ED\t3\t" <<
					(referenceDisabledNpc ? 5 : 4) << "\n"
					"player.spawn.contract\tplayerSpawn\t-\t-\t0\t0\t0\t0\t1\n"
					"trigger.contract.jump\ttriggerBox\t-\t-\t0\t0\t0\t0\t1"
					"\t2\t2\t2\t0\t1\tmovePlayer\t5\t10\t0\t0\t"
					<< durationSeconds << "\t13\n"
					"collision.contract.wall\tcollisionBox\t-\t-\t4\t1\t0\t0\t1"
					"\t0.5\t1\t2\n"
					"npc.contract.ambient\tnpc\tNPC_BEDA\t-\t1\t0\t1\t0\t1"
					"\t1\tpatrol\tloop\tsequence\t1\t0\t7\t0\t0\t0\t" <<
					(referenceDisabledNpc ? "npc.contract.disabled-target" : "-") <<
					"\t2"
					"\tnpc.contract.wp.0\t1\t0\t1\t0\t1\t" <<
					npcLookYawDegrees <<
					"\tnpc.contract.wp.1\t2\t0\t1\t0\t0\t0"
					"\t1\t" << npcActionId << "\t" << npcActionDurationMs <<
					"\t0\t" << npcActionWeight << "\n";
				if (referenceDisabledNpc)
				{
					bootstrap <<
						"npc.contract.disabled-target\tnpc\tNPC_BEDA\t-"
						"\t3\t0\t1\t0\t0\t0\n";
				}
			};
		writeTriggerBootstrap(1.f);

		wchar_t previousRoot[32768]{};
		const DWORD previousLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", previousRoot,
			static_cast<DWORD>(std::size(previousRoot)));
		SetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", triggerRoot.c_str());
		CWorldBootstrap triggerBootstrap;
		const bool loadedTriggerBootstrap = triggerBootstrap.Load(
			WORLD_ID::VALTAN_ARENA);
		tests.Require(
			loadedTriggerBootstrap &&
			4u == triggerBootstrap.Get_Placements().size() &&
			WORLD_BOOTSTRAP_KIND::TRIGGER_BOX ==
				triggerBootstrap.Get_Placements()[1].eKind &&
			WORLD_BOOTSTRAP_KIND::COLLISION_BOX ==
				triggerBootstrap.Get_Placements()[2].eKind &&
			1u == triggerBootstrap.Get_Placements()[1].TriggerActions.size() &&
			triggerBootstrap.Get_Placements()[3].bHasNpcBehavior &&
			2u == triggerBootstrap.Get_Placements()[3].NpcBehavior.Waypoints.size() &&
			1u == triggerBootstrap.Get_Placements()[3].NpcBehavior.Actions.size(),
			"Parse trigger, collision and logical NPC behavior from world bootstrap v7");

		writeTriggerBootstrap(1.f, 0u);
		tests.Require(
			!triggerBootstrap.Load(WORLD_ID::VALTAN_ARENA) &&
			4u == triggerBootstrap.Get_Placements().size(),
			"Reject a zero-duration NPC action without replacing committed world");

		writeTriggerBootstrap(1.f, 100u, 360.01f);
		tests.Require(
			!triggerBootstrap.Load(WORLD_ID::VALTAN_ARENA) &&
			4u == triggerBootstrap.Get_Placements().size(),
			"Reject an NPC waypoint look yaw outside minus 360 to 360 degrees");

		writeTriggerBootstrap(1.f, 100u, 90.f, 100001u);
		tests.Require(
			!triggerBootstrap.Load(WORLD_ID::VALTAN_ARENA) &&
			4u == triggerBootstrap.Get_Placements().size(),
			"Reject an NPC action weight above the 100000 contract maximum");

		writeTriggerBootstrap(1.f, 100u, 90.f, 1u, true);
		tests.Require(
			!triggerBootstrap.Load(WORLD_ID::VALTAN_ARENA) &&
			4u == triggerBootstrap.Get_Placements().size(),
			"Reject a logical NPC look target that is disabled");

		writeTriggerBootstrap(
			1.f, 100u, 90.f, 1u, false,
			CNpcBehaviorRuntime::IDLE_ACTION_ID);
		tests.Require(
			!triggerBootstrap.Load(WORLD_ID::VALTAN_ARENA) &&
			4u == triggerBootstrap.Get_Placements().size(),
			"Reject authored npc.idle because the Server runtime owns that action ID");

		writeTriggerBootstrap(
			1.f, 100u, 90.f, 1u, false,
			CNpcBehaviorRuntime::WALK_ACTION_ID);
		tests.Require(
			!triggerBootstrap.Load(WORLD_ID::VALTAN_ARENA) &&
			4u == triggerBootstrap.Get_Placements().size(),
			"Reject authored npc.move.walk because the Server runtime owns that action ID");

		writeTriggerBootstrap(-1.f);
		tests.Require(
			!triggerBootstrap.Load(WORLD_ID::VALTAN_ARENA) &&
			4u == triggerBootstrap.Get_Placements().size(),
			"Reject invalid trigger bootstrap without replacing committed world");
		SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
			0u == previousLength || previousLength >= std::size(previousRoot) ?
				nullptr : previousRoot);
		std::error_code cleanupError;
		fs::remove_all(triggerRoot, cleanupError);
	}

	{
		CServerNavigation npcNavigation;
		SERVER_NAV_POINT patrolStart{};
		SERVER_NAV_POINT patrolEnd{};
		std::vector<SERVER_NAV_POINT> candidatePath;
		bool foundPatrolSegment =
			npcNavigation.Load("LV_BER_BERNCASTLE") &&
			npcNavigation.Project_Point(
				137.586334f, -22.4640217f, patrolStart);
		if (foundPatrolSegment)
		{
			foundPatrolSegment = false;
			for (int radius = 2; radius <= 20 && !foundPatrolSegment; ++radius)
			{
				for (int dz = -radius; dz <= radius && !foundPatrolSegment; ++dz)
				{
					for (int dx = -radius; dx <= radius; ++dx)
					{
						if (std::abs(dx) != radius && std::abs(dz) != radius)
							continue;
						SERVER_NAV_POINT projected{};
						if (!npcNavigation.Project_Point(
							patrolStart.x + static_cast<float>(dx) * 0.5f,
							patrolStart.z + static_cast<float>(dz) * 0.5f,
							projected))
						{
							continue;
						}
						const float distance = std::hypot(
							projected.x - patrolStart.x,
							projected.z - patrolStart.z);
						if (distance >= 1.f && npcNavigation.Find_Path(
							patrolStart.x, patrolStart.z,
							projected.x, projected.z, candidatePath))
						{
							patrolEnd = projected;
							foundPatrolSegment = true;
							break;
						}
					}
				}
			}
		}
		tests.Require(
			foundPatrolSegment,
			"Find a reachable Bern navigation segment for NPC behavior contracts");

		WORLD_BOOTSTRAP_PLACEMENT patrol{};
		patrol.strPlacementId = "npc.contract.patrol";
		patrol.strArchetypeId = "NPC_AYLARA";
		patrol.eKind = WORLD_BOOTSTRAP_KIND::NPC;
		patrol.isEnabled = true;
		patrol.bHasNpcBehavior = true;
		patrol.fPositionX = patrolStart.x;
		patrol.fPositionY = patrolStart.y;
		patrol.fPositionZ = patrolStart.z;
		patrol.NpcBehavior.eMode = NPC_BEHAVIOR_MODE::PATROL;
		patrol.NpcBehavior.eRouteMode = NPC_ROUTE_MODE::ONCE;
		patrol.NpcBehavior.eActionSelection = NPC_ACTION_SELECTION::SEQUENCE;
		patrol.NpcBehavior.fMoveSpeed = 4.f;
		patrol.NpcBehavior.iRandomSeed = 17u;
		patrol.NpcBehavior.Waypoints = {
			{ "npc.contract.wp.0", patrolStart.x, patrolStart.y,
				patrolStart.z, 0u, false, 0.f },
			{ "npc.contract.wp.1", patrolEnd.x, patrolEnd.y,
				patrolEnd.z, 0u, true, 90.f }
		};

		CNpcBehaviorRuntime npcRuntime;
		std::string npcStatus;
		CServerCollisionSystem npcCollision;
		const bool npcCollisionInitialized =
			npcCollision.Initialize({}, npcStatus);
		std::vector<WORLD_BOOTSTRAP_PLACEMENT> patrolPlacements{ patrol };
		SERVER_WORLD_ENTITY patrolEntity{};
		patrolEntity.strPlacementId = patrol.strPlacementId;
		patrolEntity.strArchetypeId = patrol.strArchetypeId;
		patrolEntity.eKind = WORLD_BOOTSTRAP_KIND::NPC;
		patrolEntity.fPositionX = patrol.fPositionX;
		patrolEntity.fPositionY = patrol.fPositionY;
		patrolEntity.fPositionZ = patrol.fPositionZ;
		bool patrolAdvanced = foundPatrolSegment && npcCollisionInitialized &&
			npcRuntime.Validate_Admission(
				patrolPlacements, npcNavigation, npcStatus) &&
			npcRuntime.Initialize(
				patrol, npcNavigation, 1u, patrolEntity, npcStatus);
		bool sawPatrolWalk = false;
		for (std::uint32_t tick = 2u;
			patrolAdvanced && tick < 2000u &&
			patrolEntity.NpcBehavior.ePhase !=
				SERVER_NPC_BEHAVIOR_PHASE::COMPLETE;
			++tick)
		{
			patrolAdvanced = npcRuntime.Update(
				patrol, nullptr, npcNavigation, npcCollision, 1.f / 30.f,
				tick, patrolEntity, npcStatus);
			sawPatrolWalk = sawPatrolWalk ||
				patrolEntity.strActionId == CNpcBehaviorRuntime::WALK_ACTION_ID;
		}
		tests.Require(
			patrolAdvanced && sawPatrolWalk &&
			patrolEntity.NpcBehavior.ePhase ==
				SERVER_NPC_BEHAVIOR_PHASE::COMPLETE &&
			std::hypot(
				patrolEntity.fPositionX - patrolEnd.x,
				patrolEntity.fPositionZ - patrolEnd.z) < 0.01f &&
			std::abs(patrolEntity.fYawDegrees - 90.f) < 0.01f,
			"Advance an authored once patrol through walk, arrival yaw and completion");

		WORLD_BOOTSTRAP_PLACEMENT loopPatrol = patrol;
		loopPatrol.strPlacementId = "npc.contract.loop";
		loopPatrol.NpcBehavior.eRouteMode = NPC_ROUTE_MODE::LOOP;
		loopPatrol.NpcBehavior.fMoveSpeed = 12.f;
		loopPatrol.NpcBehavior.Waypoints[0].iWaitMs = 100u;
		loopPatrol.NpcBehavior.Waypoints[1].iWaitMs = 0u;
		SERVER_WORLD_ENTITY loopEntity{};
		loopEntity.iNetEntityId = 7101u;
		loopEntity.strPlacementId = loopPatrol.strPlacementId;
		loopEntity.strArchetypeId = loopPatrol.strArchetypeId;
		loopEntity.eKind = WORLD_BOOTSTRAP_KIND::NPC;
		loopEntity.fPositionX = loopPatrol.fPositionX;
		loopEntity.fPositionY = loopPatrol.fPositionY;
		loopEntity.fPositionZ = loopPatrol.fPositionZ;
		bool loopAdvanced = npcRuntime.Initialize(
			loopPatrol, npcNavigation, 1u, loopEntity, npcStatus);
		std::vector<std::size_t> loopArrivalNextIndices;
		std::uint32_t firstLoopArrivalTick = 0u;
		std::uint32_t firstLoopWaitUntilTick = 0u;
		bool walkedBeforeWaitDeadline = false;
		bool walkedOnWaitDeadline = false;
		for (std::uint32_t tick = 2u;
			loopAdvanced && tick < 2000u &&
			loopArrivalNextIndices.size() < 3u; ++tick)
		{
			loopAdvanced = npcRuntime.Update(
				loopPatrol, nullptr, npcNavigation, npcCollision,
				1.f / 30.f, tick, loopEntity, npcStatus);
			if (0u != firstLoopArrivalTick &&
				tick < firstLoopWaitUntilTick &&
				loopEntity.strActionId == CNpcBehaviorRuntime::WALK_ACTION_ID)
			{
				walkedBeforeWaitDeadline = true;
			}
			if (0u != firstLoopArrivalTick &&
				tick == firstLoopWaitUntilTick &&
				loopEntity.strActionId == CNpcBehaviorRuntime::WALK_ACTION_ID)
			{
				walkedOnWaitDeadline = true;
			}
			if (loopEntity.NpcBehavior.ePhase ==
					SERVER_NPC_BEHAVIOR_PHASE::IDLE_WAIT &&
				loopEntity.strActionId == CNpcBehaviorRuntime::IDLE_ACTION_ID &&
				loopEntity.iActionStartTick == tick)
			{
				loopArrivalNextIndices.push_back(
					loopEntity.NpcBehavior.iWaypointIndex);
				if (0u == firstLoopArrivalTick)
				{
					firstLoopArrivalTick = tick;
					firstLoopWaitUntilTick =
						loopEntity.NpcBehavior.iWaitUntilTick;
				}
			}
		}
		tests.Require(
			loopAdvanced && loopArrivalNextIndices.size() == 3u &&
			loopArrivalNextIndices[0] == 1u &&
			loopArrivalNextIndices[1] == 0u &&
			loopArrivalNextIndices[2] == 1u &&
			firstLoopWaitUntilTick == firstLoopArrivalTick + 3u &&
			!walkedBeforeWaitDeadline && walkedOnWaitDeadline,
			"Loop patrol wraps waypoint indices and honors a 100ms wait for exactly three 30Hz ticks");

		WORLD_BOOTSTRAP_PLACEMENT pingPongPatrol = loopPatrol;
		pingPongPatrol.strPlacementId = "npc.contract.ping-pong";
		pingPongPatrol.NpcBehavior.eRouteMode = NPC_ROUTE_MODE::PING_PONG;
		pingPongPatrol.NpcBehavior.Waypoints = {
			{ "npc.contract.ping.0", patrolStart.x, patrolStart.y,
				patrolStart.z, 0u, false, 0.f },
			{ "npc.contract.ping.1", patrolEnd.x, patrolEnd.y,
				patrolEnd.z, 0u, false, 0.f },
			{ "npc.contract.ping.2", patrolStart.x, patrolStart.y,
				patrolStart.z, 0u, false, 0.f }
		};
		SERVER_WORLD_ENTITY pingPongEntity{};
		pingPongEntity.iNetEntityId = 7102u;
		pingPongEntity.strPlacementId = pingPongPatrol.strPlacementId;
		pingPongEntity.strArchetypeId = pingPongPatrol.strArchetypeId;
		pingPongEntity.eKind = WORLD_BOOTSTRAP_KIND::NPC;
		pingPongEntity.fPositionX = pingPongPatrol.fPositionX;
		pingPongEntity.fPositionY = pingPongPatrol.fPositionY;
		pingPongEntity.fPositionZ = pingPongPatrol.fPositionZ;
		bool pingPongAdvanced = npcRuntime.Initialize(
			pingPongPatrol, npcNavigation, 1u, pingPongEntity, npcStatus);
		std::vector<std::size_t> pingPongNextIndices;
		for (std::uint32_t tick = 2u;
			pingPongAdvanced && tick < 3000u &&
			pingPongNextIndices.size() < 4u; ++tick)
		{
			pingPongAdvanced = npcRuntime.Update(
				pingPongPatrol, nullptr, npcNavigation, npcCollision,
				1.f / 30.f, tick, pingPongEntity, npcStatus);
			if (pingPongEntity.NpcBehavior.ePhase ==
					SERVER_NPC_BEHAVIOR_PHASE::IDLE_WAIT &&
				pingPongEntity.strActionId == CNpcBehaviorRuntime::IDLE_ACTION_ID &&
				pingPongEntity.iActionStartTick == tick)
			{
				pingPongNextIndices.push_back(
					pingPongEntity.NpcBehavior.iWaypointIndex);
			}
		}
		tests.Require(
			pingPongAdvanced && pingPongNextIndices.size() == 4u &&
			pingPongNextIndices[0] == 1u &&
			pingPongNextIndices[1] == 2u &&
			pingPongNextIndices[2] == 1u &&
			pingPongNextIndices[3] == 0u &&
			!pingPongEntity.NpcBehavior.bRouteForward,
			"Ping-pong patrol reverses at the authored end instead of wrapping");

		using namespace LostArk::Shared::WorldCollision;
		const float segmentMiddleX = (patrolStart.x + patrolEnd.x) * 0.5f;
		const float segmentMiddleY = (patrolStart.y + patrolEnd.y) * 0.5f;
		const float segmentMiddleZ = (patrolStart.z + patrolEnd.z) * 0.5f;
		CServerCollisionSystem npcBodyCollision;
		const bool bodyCollisionInitialized =
			npcBodyCollision.Initialize({}, npcStatus);
		npcBodyCollision.Set_BlockingBodies({ SERVER_BLOCKING_BODY{
			segmentMiddleX, segmentMiddleZ, PLAYER_HALF_EXTENT_X,
			segmentMiddleY + PLAYER_CENTER_OFFSET_Y,
			PLAYER_HALF_EXTENT_Y, 7202u } });
		float bodyResolvedX = patrolStart.x;
		float bodyResolvedY = patrolStart.y;
		float bodyResolvedZ = patrolStart.z;
		bool bodyWasBlocked = false;
		const bool sameFloorBodyResolved = bodyCollisionInitialized &&
			npcBodyCollision.Resolve_CircleMove(
				patrolStart.x, patrolStart.y, patrolStart.z,
				patrolEnd.x, patrolEnd.y, patrolEnd.z,
				PLAYER_HALF_EXTENT_X, PLAYER_HALF_EXTENT_Y,
				PLAYER_CENTER_OFFSET_Y,
				bodyResolvedX, bodyResolvedY, bodyResolvedZ,
				bodyWasBlocked, 7201u);
		tests.Require(
			sameFloorBodyResolved &&
			std::hypot(
				bodyResolvedX - segmentMiddleX,
				bodyResolvedZ - segmentMiddleZ) >=
				PLAYER_HALF_EXTENT_X * 2.f - 0.002f,
			"Resolve a same-floor NPC circle without entering another living NPC body");

		npcBodyCollision.Set_BlockingBodies({ SERVER_BLOCKING_BODY{
			segmentMiddleX, segmentMiddleZ, PLAYER_HALF_EXTENT_X,
			segmentMiddleY + PLAYER_CENTER_OFFSET_Y + 20.f,
			PLAYER_HALF_EXTENT_Y, 7202u } });
		bodyResolvedX = patrolStart.x;
		bodyResolvedY = patrolStart.y;
		bodyResolvedZ = patrolStart.z;
		bodyWasBlocked = false;
		const bool otherFloorIgnored = npcBodyCollision.Resolve_CircleMove(
			patrolStart.x, patrolStart.y, patrolStart.z,
			patrolEnd.x, patrolEnd.y, patrolEnd.z,
			PLAYER_HALF_EXTENT_X, PLAYER_HALF_EXTENT_Y,
			PLAYER_CENTER_OFFSET_Y,
			bodyResolvedX, bodyResolvedY, bodyResolvedZ,
			bodyWasBlocked, 7201u) &&
			!bodyWasBlocked &&
			std::hypot(
				bodyResolvedX - patrolEnd.x,
				bodyResolvedZ - patrolEnd.z) < 0.001f;
		tests.Require(
			otherFloorIgnored,
			"Do not collide NPC bodies whose authoritative vertical spans are on different floors");

		npcBodyCollision.Set_BlockingBodies({ SERVER_BLOCKING_BODY{
			segmentMiddleX, segmentMiddleZ, PLAYER_HALF_EXTENT_X,
			segmentMiddleY + PLAYER_CENTER_OFFSET_Y,
			PLAYER_HALF_EXTENT_Y, 7201u } });
		bodyResolvedX = patrolStart.x;
		bodyResolvedY = patrolStart.y;
		bodyResolvedZ = patrolStart.z;
		bodyWasBlocked = false;
		tests.Require(
			npcBodyCollision.Resolve_CircleMove(
				patrolStart.x, patrolStart.y, patrolStart.z,
				patrolEnd.x, patrolEnd.y, patrolEnd.z,
				PLAYER_HALF_EXTENT_X, PLAYER_HALF_EXTENT_Y,
				PLAYER_CENTER_OFFSET_Y,
				bodyResolvedX, bodyResolvedY, bodyResolvedZ,
				bodyWasBlocked, 7201u) &&
			!bodyWasBlocked &&
			std::hypot(
				bodyResolvedX - patrolEnd.x,
				bodyResolvedZ - patrolEnd.z) < 0.001f,
			"Exclude the moving NPC from dynamic bodies by stable NetEntityId");

		WORLD_BOOTSTRAP_PLACEMENT npcWall{};
		npcWall.strPlacementId = "collision.contract.npc-wall";
		npcWall.eKind = WORLD_BOOTSTRAP_KIND::COLLISION_BOX;
		npcWall.isEnabled = true;
		npcWall.fPositionX = segmentMiddleX;
		npcWall.fPositionY = segmentMiddleY + PLAYER_CENTER_OFFSET_Y;
		npcWall.fPositionZ = segmentMiddleZ;
		npcWall.fHalfExtentX = 0.1f;
		npcWall.fHalfExtentY = 2.f;
		npcWall.fHalfExtentZ = 0.1f;
		CServerCollisionSystem npcWallCollision;
		const bool wallCollisionInitialized =
			npcWallCollision.Initialize({ npcWall }, npcStatus);
		float wallResolvedX = patrolStart.x;
		float wallResolvedY = patrolStart.y;
		float wallResolvedZ = patrolStart.z;
		bool wallWasBlocked = false;
		tests.Require(
			wallCollisionInitialized &&
			npcWallCollision.Resolve_CircleMove(
				patrolStart.x, patrolStart.y, patrolStart.z,
				patrolEnd.x, patrolEnd.y, patrolEnd.z,
				PLAYER_HALF_EXTENT_X, PLAYER_HALF_EXTENT_Y,
				PLAYER_CENTER_OFFSET_Y,
				wallResolvedX, wallResolvedY, wallResolvedZ,
				wallWasBlocked, 7201u) && wallWasBlocked &&
			std::hypot(
				wallResolvedX - patrolEnd.x,
				wallResolvedZ - patrolEnd.z) > 0.01f,
			"Stop an NPC circle at an authoritative collisionBox instead of crossing it");

		WORLD_BOOTSTRAP_PLACEMENT unreachable = patrol;
		unreachable.strPlacementId = "npc.contract.unreachable";
		unreachable.NpcBehavior.Waypoints[1].fPositionX = 100000.f;
		tests.Require(
			!npcRuntime.Validate_Admission(
				std::vector<WORLD_BOOTSTRAP_PLACEMENT>{ unreachable },
				npcNavigation, npcStatus),
			"Reject an NPC patrol waypoint outside authoritative navigation");

		WORLD_BOOTSTRAP_PLACEMENT ambient = patrol;
		ambient.strPlacementId = "npc.contract.ambient-runtime";
		ambient.NpcBehavior.eMode = NPC_BEHAVIOR_MODE::STATIONARY;
		ambient.NpcBehavior.eRouteMode = NPC_ROUTE_MODE::LOOP;
		ambient.NpcBehavior.Waypoints.clear();
		ambient.NpcBehavior.Actions = {
			{ "npc.ambient.look", 34u, 0u, 1u },
			{ "npc.ambient.greet", 34u, 0u, 1u }
		};
		CServerNavigation unloadedNpcNavigation;
		tests.Require(
			npcRuntime.Validate_Admission(
				std::vector<WORLD_BOOTSTRAP_PLACEMENT>{ ambient },
				unloadedNpcNavigation, npcStatus) &&
			!npcRuntime.Validate_Admission(
				std::vector<WORLD_BOOTSTRAP_PLACEMENT>{ patrol },
				unloadedNpcNavigation, npcStatus),
			"Allow stationary ambient NPCs without navigation but reject moving behaviors");
		SERVER_WORLD_ENTITY ambientEntity{};
		ambientEntity.strPlacementId = ambient.strPlacementId;
		ambientEntity.strArchetypeId = ambient.strArchetypeId;
		ambientEntity.eKind = WORLD_BOOTSTRAP_KIND::NPC;
		ambientEntity.fPositionX = ambient.fPositionX;
		ambientEntity.fPositionY = ambient.fPositionY;
		ambientEntity.fPositionZ = ambient.fPositionZ;
		bool ambientAdvanced = npcRuntime.Initialize(
			ambient, npcNavigation, 1u, ambientEntity, npcStatus);
		bool sawLook = false;
		bool sawGreet = false;
		std::uint32_t firstLookTick = 0u;
		std::uint32_t repeatedLookTick = 0u;
		for (std::uint32_t tick = 2u; ambientAdvanced && tick < 40u; ++tick)
		{
			ambientAdvanced = npcRuntime.Update(
				ambient, nullptr, npcNavigation, npcCollision, 1.f / 30.f,
				tick, ambientEntity, npcStatus);
			if (ambientEntity.strActionId == "npc.ambient.look")
			{
				sawLook = true;
				if (0u == firstLookTick)
					firstLookTick = ambientEntity.iActionStartTick;
				else if (firstLookTick != ambientEntity.iActionStartTick)
					repeatedLookTick = ambientEntity.iActionStartTick;
			}
			if (ambientEntity.strActionId == "npc.ambient.greet")
				sawGreet = true;
		}
		tests.Require(
			ambientAdvanced && sawLook && sawGreet &&
			0u != repeatedLookTick,
			"Sequence ambient actions and restart the same clip through actionStartTick edges");

		WORLD_BOOTSTRAP_PLACEMENT wander = ambient;
		wander.strPlacementId = "npc.contract.wander";
		wander.NpcBehavior.eMode = NPC_BEHAVIOR_MODE::WANDER;
		wander.NpcBehavior.eActionSelection = NPC_ACTION_SELECTION::WEIGHTED;
		wander.NpcBehavior.fWanderRadius = 3.f;
		wander.NpcBehavior.iRandomSeed = 991u;
		wander.NpcBehavior.Actions.clear();
		std::vector<SERVER_NAV_POINT> boundedWanderPath;
		const bool foundBoundedWanderPath =
			npcNavigation.Find_PathToReachablePointWithinRadius(
				patrolStart.x, patrolStart.z,
				patrolStart.x, patrolStart.z, 3.f, 0.05f,
				boundedWanderPath);
		const bool boundedWanderPathStayedInside =
			foundBoundedWanderPath && !boundedWanderPath.empty() &&
			std::all_of(
				boundedWanderPath.begin(), boundedWanderPath.end(),
				[&patrolStart](const SERVER_NAV_POINT& point)
				{
					return std::hypot(
						point.x - patrolStart.x,
						point.z - patrolStart.z) <= 3.001f;
				});
		tests.Require(
			boundedWanderPathStayedInside,
			"Find a deterministic nontrivial wander path whose every cell stays inside the authored radius");
		SERVER_WORLD_ENTITY wanderA{};
		wanderA.strPlacementId = wander.strPlacementId;
		wanderA.strArchetypeId = wander.strArchetypeId;
		wanderA.eKind = WORLD_BOOTSTRAP_KIND::NPC;
		wanderA.fPositionX = wander.fPositionX;
		wanderA.fPositionY = wander.fPositionY;
		wanderA.fPositionZ = wander.fPositionZ;
		SERVER_WORLD_ENTITY wanderB = wanderA;
		bool deterministicWander = npcRuntime.Initialize(
			wander, npcNavigation, 1u, wanderA, npcStatus) &&
			npcRuntime.Initialize(
				wander, npcNavigation, 1u, wanderB, npcStatus);
		bool sawWanderWalk = false;
		float maximumWanderDisplacement = 0.f;
		for (std::uint32_t tick = 2u;
			deterministicWander && tick < 300u; ++tick)
		{
				deterministicWander = npcRuntime.Update(
				wander, nullptr, npcNavigation, npcCollision, 1.f / 30.f,
				tick, wanderA, npcStatus) &&
				npcRuntime.Update(
					wander, nullptr, npcNavigation, npcCollision, 1.f / 30.f,
					tick, wanderB, npcStatus) &&
				wanderA.fPositionX == wanderB.fPositionX &&
				wanderA.fPositionZ == wanderB.fPositionZ &&
				std::hypot(
					wanderA.fPositionX - wanderA.fSpawnPositionX,
					wanderA.fPositionZ - wanderA.fSpawnPositionZ) <= 3.001f;
			sawWanderWalk = sawWanderWalk ||
				wanderA.strActionId == CNpcBehaviorRuntime::WALK_ACTION_ID;
			maximumWanderDisplacement = (std::max)(
				maximumWanderDisplacement,
				std::hypot(
					wanderA.fPositionX - wanderA.fSpawnPositionX,
					wanderA.fPositionZ - wanderA.fSpawnPositionZ));
		}
		tests.Require(
			deterministicWander && sawWanderWalk &&
			maximumWanderDisplacement > 0.1f,
			"Keep seeded NPC wander deterministic, walking, displaced and inside its authored radius");

		WORLD_BOOTSTRAP_PLACEMENT trivialWander = wander;
		trivialWander.strPlacementId = "npc.contract.wander-trivial";
		trivialWander.NpcBehavior.fWanderRadius = 0.001f;
		tests.Require(
			!npcRuntime.Validate_Admission(
				std::vector<WORLD_BOOTSTRAP_PLACEMENT>{ trivialWander },
				npcNavigation, npcStatus),
			"Reject wander admission when no nontrivial reachable destination exists");

		SERVER_WORLD_ENTITY exhaustedWander{};
		exhaustedWander.iNetEntityId = 7103u;
		exhaustedWander.strPlacementId = wander.strPlacementId;
		exhaustedWander.strArchetypeId = wander.strArchetypeId;
		exhaustedWander.eKind = WORLD_BOOTSTRAP_KIND::NPC;
		exhaustedWander.fPositionX = wander.fPositionX;
		exhaustedWander.fPositionY = wander.fPositionY;
		exhaustedWander.fPositionZ = wander.fPositionZ;
		const bool initializedExhaustedWander = npcRuntime.Initialize(
			wander, npcNavigation, 1u, exhaustedWander, npcStatus);
		WORLD_BOOTSTRAP_PLACEMENT exhaustedDescriptor = wander;
		exhaustedDescriptor.NpcBehavior.fWanderRadius = 0.001f;
		tests.Require(
			initializedExhaustedWander &&
			!npcRuntime.Update(
				exhaustedDescriptor, nullptr, npcNavigation, npcCollision,
				1.f / 30.f, 2u, exhaustedWander, npcStatus) &&
			npcStatus.find("destination became unavailable") != std::string::npos,
			"Surface exhausted wander destination search instead of reporting a successful permanent idle");
	}

	{
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "trigger.contract.jump";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = false;
		WORLD_TRIGGER_ACTION move{};
		move.eKind = WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER;
		move.fTargetX = 10.f;
		move.fTargetY = 0.f;
		move.fTargetZ = 0.f;
		move.fDurationSeconds = 1.f;
		move.fArcHeight = 4.f;
		trigger.TriggerActions.push_back(move);

		CServerTriggerSystem triggerSystem;
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus) &&
			1u == triggerSystem.Get_TriggerCount(),
			"Initialize enabled movePlayer trigger");
		std::map<PLAYER_ID, SERVER_PLAYER> triggerPlayers;
		SERVER_PLAYER triggerPlayer{};
		triggerPlayer.iPlayerId = 1;
		triggerPlayer.fPositionX = 2.4f;
		triggerPlayer.iCurrentHp = 100;
		triggerPlayer.iMaximumHp = 100;
		triggerPlayers.emplace(1, triggerPlayer);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		triggerSystem.Evaluate_Entries(triggerPlayers, 10, transfers, {});
		tests.Require(
			PLAYER_ACTION_STATE::TRIGGER_MOVE ==
				triggerPlayers.begin()->second.eAction &&
			triggerPlayers.begin()->second.TriggerMove.isActive &&
			10u == triggerPlayers.begin()->second.iActionStartTick,
			"Fire trigger on OBB entry");
		triggerSystem.Update_PlayerMotion(triggerPlayers.begin()->second, 0.5f);
		tests.Require(
			std::abs(triggerPlayers.begin()->second.fPositionX - 6.2f) < 0.001f &&
			std::abs(triggerPlayers.begin()->second.fPositionY - 4.f) < 0.001f,
			"Advance movePlayer with authored parabolic arc");
		triggerSystem.Update_PlayerMotion(triggerPlayers.begin()->second, 0.5f);
		tests.Require(
			std::abs(triggerPlayers.begin()->second.fPositionX - 10.f) < 0.001f &&
			std::abs(triggerPlayers.begin()->second.fPositionY) < 0.001f &&
			PLAYER_ACTION_STATE::NONE == triggerPlayers.begin()->second.eAction &&
			!triggerPlayers.begin()->second.TriggerMove.isActive,
			"Complete movePlayer at exact authored destination");
		triggerSystem.Evaluate_Entries(triggerPlayers, 11, transfers, {});
		triggerPlayers.begin()->second.fPositionX = 0.f;
		triggerSystem.Evaluate_Entries(triggerPlayers, 12, transfers, {});
		tests.Require(
			PLAYER_ACTION_STATE::TRIGGER_MOVE ==
				triggerPlayers.begin()->second.eAction &&
			12u == triggerPlayers.begin()->second.iActionStartTick,
			"Rearm non-once trigger after player exits");
	}

	{
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "trigger.contract.change-level";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = true;
		WORLD_TRIGGER_ACTION changeLevel{};
		changeLevel.eKind = WORLD_TRIGGER_ACTION_KIND::CHANGE_LEVEL;
		changeLevel.eTargetWorldId = WORLD_ID::VALTAN_ARENA;
		trigger.TriggerActions.push_back(changeLevel);

		CServerTriggerSystem triggerSystem;
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus),
			"Initialize enabled changeLevel trigger");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER player{};
		player.iSessionId = 7;
		player.iPlayerId = 3;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.strNickName = "TriggerTransfer";
		player.iCurrentHp = 100;
		player.iMaximumHp = 100;
		players.emplace(player.iPlayerId, player);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		triggerSystem.Evaluate_Entries(players, 20, transfers, {});
		tests.Require(
			1u == transfers.size() &&
			7u == transfers.front().iSessionId &&
			WORLD_ID::VALTAN_ARENA == transfers.front().eTargetWorldId &&
			CHARACTER_CLASS_ID::LANCE_MASTER ==
				transfers.front().eCharacterClass &&
			"TriggerTransfer" == transfers.front().strNickName,
			"Emit one typed Server world transfer request on OBB entry");
		triggerSystem.Evaluate_Entries(players, 21, transfers, {});
			tests.Require(
			transfers.empty(),
			"Do not repeat a triggerOnce world transfer while occupied");
	}

	{
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "trigger.contract.activate-spawn-group";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = true;
		WORLD_TRIGGER_ACTION activate{};
		activate.eKind = WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP;
		activate.strTargetId = "spawn.valtan.stage01";
		trigger.TriggerActions.push_back(activate);

		CServerTriggerSystem triggerSystem;
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus),
			"Initialize enabled activateSpawnGroup trigger");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER player{};
		player.iPlayerId = 4;
		player.iCurrentHp = 100;
		player.iMaximumHp = 100;
		players.emplace(player.iPlayerId, player);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::size_t activationCount = 0u;
		triggerSystem.Evaluate_Entries(
			players,
			30,
			transfers,
			[&activationCount](
				WORLD_TRIGGER_ACTION_KIND kind,
				const std::string& targetId)
			{
				if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP != kind ||
					"spawn.valtan.stage01" != targetId)
				{
					return false;
				}
				++activationCount;
				return true;
			});
		tests.Require(
			1u == activationCount && transfers.empty(),
			"Dispatch typed activateSpawnGroup target on OBB entry");
		triggerSystem.Evaluate_Entries(
			players,
			31,
			transfers,
			[&activationCount](WORLD_TRIGGER_ACTION_KIND, const std::string&)
			{
				++activationCount;
				return true;
			});
		tests.Require(
			1u == activationCount,
			"Do not repeat a triggerOnce spawn-group activation while occupied");
	}

	{
		/* Stage_1 is the stage whose wave is being built, so it runs its real
		activateSpawnGroup action in Debug as well. The later stages keep the
		shortcut, which is what still carries boss work to Valtan without
		clearing the corridor. Stage_MiniBoss stands in for those here. */
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "Stage_MiniBoss";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = true;
		WORLD_TRIGGER_ACTION activate{};
		activate.eKind = WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP;
		activate.strTargetId = "spawn.valtan.stage02.miniboss";
		trigger.TriggerActions.push_back(activate);

		CServerTriggerSystem triggerSystem;
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus, true),
			"Initialize the Debug Valtan stage-route bypass");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER player{};
		player.iPlayerId = 404u;
		player.iCurrentHp = 100u;
		player.iMaximumHp = 100u;
		players.emplace(player.iPlayerId, player);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::size_t activationCount = 0u;
		triggerSystem.Evaluate_Entries(
			players, 41u, transfers,
			[&activationCount](WORLD_TRIGGER_ACTION_KIND, const std::string&)
			{
				++activationCount;
				return true;
			});
#ifdef _DEBUG
		const SERVER_PLAYER& moving = players.begin()->second;
		tests.Require(
			0u == activationCount &&
			PLAYER_ACTION_STATE::TRIGGER_MOVE == moving.eAction &&
			moving.TriggerMove.isActive &&
			std::abs(moving.TriggerMove.fTargetX - 86.110f) < 0.001f &&
			std::abs(moving.TriggerMove.fTargetZ + 93.033f) < 0.001f,
			"Bypass a later Valtan stage group and move toward the next trigger in Debug");
		triggerSystem.Update_PlayerMotion(players.begin()->second, 1.f);
		tests.Require(
			PLAYER_ACTION_STATE::NONE == players.begin()->second.eAction &&
			std::abs(players.begin()->second.fPositionX - 86.110f) < 0.001f &&
			std::abs(players.begin()->second.fPositionZ + 93.033f) < 0.001f,
			"Complete the Debug stage bypass at the authored next-stage approach point");
#else
		tests.Require(
			1u == activationCount &&
			PLAYER_ACTION_STATE::NONE == players.begin()->second.eAction,
			"Keep the original Valtan spawn-group trigger unchanged in Release");
#endif
	}

	{
		/* Stage_1 is exempt from the Debug shortcut on purpose: it is the wave
		being built, so stepping into it has to run the real activation even with
		the bypass switched on. Without this the corridor's first fight is
		unreachable in a Debug session. */
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "Stage_1";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = true;
		WORLD_TRIGGER_ACTION activate{};
		activate.eKind = WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP;
		activate.strTargetId = "spawn.valtan.stage01";
		trigger.TriggerActions.push_back(activate);

		CServerTriggerSystem triggerSystem;
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus, true),
			"Initialize the Debug bypass with Stage_1 present");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER player{};
		player.iPlayerId = 405u;
		player.iCurrentHp = 100u;
		player.iMaximumHp = 100u;
		players.emplace(player.iPlayerId, player);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::string activatedTargetId;
		std::size_t activationCount = 0u;
		triggerSystem.Evaluate_Entries(
			players, 42u, transfers,
			[&activationCount, &activatedTargetId](
				const WORLD_TRIGGER_ACTION_KIND kind, const std::string& targetId)
			{
				if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP != kind)
					return false;
				++activationCount;
				activatedTargetId = targetId;
				return true;
			});
		tests.Require(
			1u == activationCount &&
			"spawn.valtan.stage01" == activatedTargetId &&
			PLAYER_ACTION_STATE::NONE == players.begin()->second.eAction,
			"Activate the Stage_1 wave instead of bypassing it, Debug included");
	}

	{
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "trigger.contract.activate-encounter";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = true;
		WORLD_TRIGGER_ACTION activate{};
		activate.eKind = WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER;
		activate.strTargetId = "boss.valtan.center";
		trigger.TriggerActions.push_back(activate);

		CServerTriggerSystem triggerSystem;
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus),
			"Initialize enabled activateEncounter trigger");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER player{};
		player.iPlayerId = 5;
		player.iCurrentHp = 100;
		player.iMaximumHp = 100;
		players.emplace(player.iPlayerId, player);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::size_t activationCount = 0u;
		triggerSystem.Evaluate_Entries(
			players,
			40,
			transfers,
			[&activationCount](
				WORLD_TRIGGER_ACTION_KIND kind,
				const std::string& targetId)
			{
				if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER != kind ||
					"boss.valtan.center" != targetId)
				{
					return false;
				}
				++activationCount;
				return true;
			});
		tests.Require(
			1u == activationCount && transfers.empty(),
			"Dispatch typed activateEncounter target on OBB entry");
	}

	{
		/* A bootstrap whose skill cost exceeds every class pool must fail load:
		the publisher enforces the same bound, so acceptance here would mean the
		two sides disagree about the same document. */
		namespace fs = std::filesystem;
		const fs::path overCostRoot =
			fs::temp_directory_path() / L"LostArkBalanceContractTest";
		std::error_code prepareError;
		fs::remove_all(overCostRoot, prepareError);
		fs::create_directories(overCostRoot / L"Gameplay");
		{
			std::ofstream bootstrap(
				overCostRoot / L"Gameplay" / L"Gameplay.bootstrap",
				std::ios::binary);
			bootstrap <<
				"LOSTARK_GAMEPLAY_BOOTSTRAP\t" << GAMEPLAY_BOOTSTRAP_VERSION <<
				"\t19\n"
				"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\tHEALTH_PERCENT_THRESHOLD\t50\n"
				"BOSSPART\tBOSS_VALTAN\tboss.part.valtan.arm-armor\t2\t1000\t15\tGROGGY_ONLY\n"
				"DAMAGE\tdamage.player.34120\t361\n"
				"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t2\tANY\tANY\t0\n"
				"PATTERNPOLICY\tENCOUNTER_VALTAN\tVALTAN_TEST\tNORMAL\t1\t3\tLOCK_NEAREST_ON_START\tLOCK_FACING_ON_START\n"
				"PATTERNSOURCE\tENCOUNTER_VALTAN\tVALTAN_TEST\t420601\t12\t5000\t150\t350\t300\t180\n"
				"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\t0\tdamage.player.34120\t2\t242\t1\t2000\n"
				"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
				"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
				"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
				"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
				"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34120\t0\t0\t0\t0\n"
				"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n"
				"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
				"SKILL\t34120\tLANCE_MASTER\tQ\tlancemaster.skill.34120\t10000\t2266"
				"\t1510\t2000\t0\t0\t8\tdamage.player.34120\tACTIVE\tLANCE_MASTER_LONG_SPEAR\tNONE\n"
				"SKILLCOMBATTRAITS\t34120\t0\t0\t0\n";
			Write_ValidValtanTimelineRows(bootstrap);
		}
		wchar_t previousRoot[32768]{};
		const DWORD previousLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", previousRoot,
			static_cast<DWORD>(std::size(previousRoot)));
		CGameplayCatalog rollbackCatalog;
		tests.Require(rollbackCatalog.Load(),
			"Stage a valid gameplay catalog before rollback test");
		const auto* committedPatterns =
			rollbackCatalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const size_t committedPatternCount =
			nullptr != committedPatterns ? committedPatterns->size() : 0u;
		const VALTAN_TIMELINE_DEFINITION* committedTimeline =
			rollbackCatalog.Find_ValtanTimeline("ENCOUNTER_VALTAN");
		const size_t committedTimelineRowCount =
			nullptr != committedTimeline ? committedTimeline->Rows.size() : 0u;
		const std::string committedIntroPatternId =
			rollbackCatalog.Find_IntroPatternId("ENCOUNTER_VALTAN");
		SetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", overCostRoot.c_str());
		CGameplayCatalog overCostCatalog;
		tests.Require(!overCostCatalog.Load(),
			"Reject bootstrap skill cost above every class pool");
		tests.Require(
			!rollbackCatalog.Load() &&
			nullptr != rollbackCatalog.Find_Skill(34010) &&
			nullptr != rollbackCatalog.Find_BossPatterns("ENCOUNTER_VALTAN") &&
			0u != committedPatternCount &&
			committedPatternCount ==
				rollbackCatalog.Find_BossPatterns("ENCOUNTER_VALTAN")->size() &&
			nullptr != rollbackCatalog.Find_ValtanTimeline(
				"ENCOUNTER_VALTAN") &&
			52u == committedTimelineRowCount &&
			committedTimelineRowCount ==
				rollbackCatalog.Find_ValtanTimeline(
					"ENCOUNTER_VALTAN")->Rows.size() &&
			!committedIntroPatternId.empty() &&
			committedIntroPatternId == rollbackCatalog.Find_IntroPatternId(
				"ENCOUNTER_VALTAN"),
			"Preserve the committed catalog after a corrupt replacement fails");
		SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
			0u == previousLength || previousLength >= std::size(previousRoot) ?
				nullptr : previousRoot);
		std::error_code cleanupError;
		fs::remove_all(overCostRoot, cleanupError);
	}

	{
		/* A skill with no damage profile never resolves a hit, so the hit time and
		reach that only describe that hit must be zero. Accepting a reach here would
		let a movement skill silently keep a damage window. */
		namespace fs = std::filesystem;
		const fs::path noDamageRoot =
			fs::temp_directory_path() / L"LostArkNoDamageContractTest";
		const auto loadWithMovementSkill =
			[&noDamageRoot](
				const char* hitTimeMs,
				const char* maximumRange,
				const TEST_TIMELINE_VARIANT timelineVariant)
		{
			std::error_code prepareError;
			fs::remove_all(noDamageRoot, prepareError);
			fs::create_directories(noDamageRoot / L"Gameplay");
			{
				std::ofstream bootstrap(
					noDamageRoot / L"Gameplay" / L"Gameplay.bootstrap",
					std::ios::binary);
				bootstrap <<
					"LOSTARK_GAMEPLAY_BOOTSTRAP\t" << GAMEPLAY_BOOTSTRAP_VERSION <<
					'\t' << (16u + Count_TestTimelineRows(timelineVariant)) << '\n' <<
					"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\tHEALTH_PERCENT_THRESHOLD\t50\n"
					"BOSSPART\tBOSS_VALTAN\tboss.part.valtan.arm-armor\t2\t1000\t15\tGROGGY_ONLY\n"
					"DAMAGE\tdamage.player.34120\t361\n"
					"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t2\tANY\tANY\t0\n"
					"PATTERNPOLICY\tENCOUNTER_VALTAN\tVALTAN_TEST\tNORMAL\t1\t3\tLOCK_NEAREST_ON_START\tLOCK_FACING_ON_START\n"
					"PATTERNSOURCE\tENCOUNTER_VALTAN\tVALTAN_TEST\t420601\t12\t5000\t150\t350\t300\t180\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\t0\tdamage.player.34120\t2\t242\t1\t2000\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
					"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
					"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
					"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
					"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34120\t0\t0\t0\t0\n"
					"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n"
					"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
					"SKILL\t34020\tLANCE_MASTER\tSPACE\tlancemaster.skill.34020"
					"\t8000\t900\t" << hitTimeMs << "\t242\t0\t6\t" << maximumRange <<
					"\t\tACTIVE\tLANCE_MASTER_LONG_SPEAR\tNONE\n"
					"SKILLCOMBATTRAITS\t34020\t0\t0\t0\n";
				Write_TestValtanTimelineRows(bootstrap, timelineVariant);
			}
			wchar_t previous[32768]{};
			const DWORD previousLength = GetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", previous,
				static_cast<DWORD>(std::size(previous)));
			SetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", noDamageRoot.c_str());
			CGameplayCatalog catalog;
			const bool loaded = catalog.Load();
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
				0u == previousLength || previousLength >= std::size(previous) ?
					nullptr : previous);
			return loaded;
		};
		tests.Require(loadWithMovementSkill(
			"0", "0", TEST_TIMELINE_VARIANT::VALID),
			"Accept a skill that carries no damage profile");
		tests.Require(!loadWithMovementSkill(
			"0", "3", TEST_TIMELINE_VARIANT::VALID),
			"Reject a damageless skill that still claims reach");
		tests.Require(!loadWithMovementSkill(
			"400", "0", TEST_TIMELINE_VARIANT::VALID),
			"Reject a damageless skill that still claims a hit time");
		tests.Require(!loadWithMovementSkill(
			"0", "0", TEST_TIMELINE_VARIANT::MISSING_ROW),
			"Reject a timeline whose declared occurrence row is missing");
		tests.Require(!loadWithMovementSkill(
			"0", "0", TEST_TIMELINE_VARIANT::MISSING_ACTION),
			"Reject a timeline occurrence whose pattern action is missing");
		tests.Require(!loadWithMovementSkill(
			"0", "0", TEST_TIMELINE_VARIANT::OVERSIZED_ACTION_INDEX),
			"Reject an oversized timeline action index before staging storage");
		tests.Require(!loadWithMovementSkill(
			"0", "0", TEST_TIMELINE_VARIANT::COMMAND_HASH_MISMATCH),
			"Reject a timeline command id that does not match its semantic row id");
		tests.Require(!loadWithMovementSkill(
			"0", "0", TEST_TIMELINE_VARIANT::COMMAND_HASH_COLLISION),
			"Reject distinct timeline row ids whose stable command ids collide");
		std::error_code noDamageCleanupError;
		fs::remove_all(noDamageRoot, noDamageCleanupError);
	}

	{
		/* Wall-contact admission is separate from player damage admission: a
		   malformed or duplicated allowlist row must fail before a room can use
		   the hit as an axe collider. */
		namespace fs = std::filesystem;
		const fs::path wallContactRoot =
			fs::temp_directory_path() / L"LostArkWallContactCatalogContractTest";
		const auto loadWithWallContactRows = [
			&wallContactRoot](
				const std::uint32_t version,
				const std::vector<std::string>& sourceRows,
				const std::vector<std::string>& wallRows)
		{
			std::error_code prepareError;
			fs::remove_all(wallContactRoot, prepareError);
			fs::create_directories(wallContactRoot / L"Gameplay");
			{
				std::ofstream bootstrap(
					wallContactRoot / L"Gameplay" / L"Gameplay.bootstrap",
					std::ios::binary);
				bootstrap << "LOSTARK_GAMEPLAY_BOOTSTRAP\t" << version << "\t" <<
					(15u + VALID_VALTAN_TIMELINE_ROW_COUNT +
					 sourceRows.size() + wallRows.size()) << "\n"
					"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\tHEALTH_PERCENT_THRESHOLD\t50\n"
					"BOSSPART\tBOSS_VALTAN\tboss.part.valtan.arm-armor\t2\t1000\t15\tGROGGY_ONLY\n"
					"DAMAGE\tdamage.player.34120\t361\n"
					"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t2\tANY\tANY\t0\n"
					"PATTERNPOLICY\tENCOUNTER_VALTAN\tVALTAN_TEST\tNORMAL\t1\t3\tLOCK_NEAREST_ON_START\tLOCK_FACING_ON_START\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\t0\tdamage.player.34120\t2\t242\t1\t2000\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
					"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
					"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
					"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
					"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34120\t0\t0\t0\t0\n"
					"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n"
					"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
					"SKILL\t34020\tLANCE_MASTER\tSPACE\tlancemaster.skill.34020\t8000\t900\t0\t242\t0\t6\t0\t\tACTIVE\tLANCE_MASTER_LONG_SPEAR\tNONE\n"
					"SKILLCOMBATTRAITS\t34020\t0\t0\t0\n";
				for (const std::string& row : sourceRows)
					bootstrap << row << '\n';
				for (const std::string& row : wallRows)
					bootstrap << row << '\n';
				Write_ValidValtanTimelineRows(bootstrap);
			}
			wchar_t previous[32768]{};
			const DWORD previousLength = GetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", previous,
				static_cast<DWORD>(std::size(previous)));
			SetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", wallContactRoot.c_str());
			CGameplayCatalog wallCatalog;
			const bool loaded = wallCatalog.Load();
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
				0u == previousLength || previousLength >= std::size(previous) ?
					nullptr : previous);
			return loaded;
		};
		const std::string validWallRow =
			"PATTERNWALLCONTACT\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active";
		const std::string validSourceRow =
			"PATTERNSOURCE\tENCOUNTER_VALTAN\tVALTAN_TEST\t420601\t12\t5000\t150\t350\t300\t180";
		tests.Require(
			loadWithWallContactRows(
				GAMEPLAY_BOOTSTRAP_VERSION, { validSourceRow }, { validWallRow }),
			"Accept one exact ACTIVE axe wall-contact row");
		tests.Require(
			!loadWithWallContactRows(
				GAMEPLAY_BOOTSTRAP_VERSION - 1u,
				{ validSourceRow }, { validWallRow }),
			"Reject the obsolete gameplay bootstrap version before wall-contact load");
		tests.Require(
			!loadWithWallContactRows(GAMEPLAY_BOOTSTRAP_VERSION,
				{ validSourceRow }, {
				"PATTERNWALLCONTACT\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.wrong" }),
			"Reject a wall-contact row whose action does not exactly join its stage");
		tests.Require(
			!loadWithWallContactRows(
				GAMEPLAY_BOOTSTRAP_VERSION,
				{ validSourceRow }, { validWallRow, validWallRow }),
			"Reject duplicate axe wall-contact ownership atomically");
		tests.Require(
			!loadWithWallContactRows(GAMEPLAY_BOOTSTRAP_VERSION, {}, {}),
			"Reject a boss pattern with no compiled source timing row");
		tests.Require(
			!loadWithWallContactRows(
				GAMEPLAY_BOOTSTRAP_VERSION,
				{ validSourceRow, validSourceRow }, {}),
			"Reject duplicate source timing ownership atomically");
		tests.Require(
			!loadWithWallContactRows(GAMEPLAY_BOOTSTRAP_VERSION, {
				"PATTERNSOURCE\tENCOUNTER_VALTAN\tVALTAN_TEST\t420601\t12\t5000\t149\t350\t300\t180" }, {}),
			"Reject a source cooldown whose 30 Hz tick conversion is inconsistent");

		const auto loadWithPatternStageRows = [&wallContactRoot](
			const std::string& stageRows,
			const std::vector<std::string>& hitOffsetRows)
		{
			std::error_code prepareError;
			fs::remove_all(wallContactRoot, prepareError);
			fs::create_directories(wallContactRoot / L"Gameplay");
			{
				std::ofstream bootstrap(
					wallContactRoot / L"Gameplay" / L"Gameplay.bootstrap",
					std::ios::binary);
				bootstrap << "LOSTARK_GAMEPLAY_BOOTSTRAP\t" <<
					GAMEPLAY_BOOTSTRAP_VERSION << "\t" <<
					(16u + VALID_VALTAN_TIMELINE_ROW_COUNT +
						hitOffsetRows.size()) << "\n"
					"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\tHEALTH_PERCENT_THRESHOLD\t50\n"
					"BOSSPART\tBOSS_VALTAN\tboss.part.valtan.arm-armor\t2\t1000\t15\tGROGGY_ONLY\n"
					"DAMAGE\tdamage.player.34120\t361\n"
					"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t2\tANY\tANY\t0\n"
					"PATTERNPOLICY\tENCOUNTER_VALTAN\tVALTAN_TEST\tNORMAL\t1\t3\tLOCK_NEAREST_ON_START\tLOCK_FACING_ON_START\n"
					"PATTERNSOURCE\tENCOUNTER_VALTAN\tVALTAN_TEST\t420601\t12\t5000\t150\t350\t300\t180\n"
					<< stageRows;
				for (const std::string& hitOffsetRow : hitOffsetRows)
					bootstrap << hitOffsetRow << '\n';
				bootstrap <<
					"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
					"SKILL\t34020\tLANCE_MASTER\tSPACE\tlancemaster.skill.34020\t8000\t900\t0\t242\t0\t6\t0\t\tACTIVE\tLANCE_MASTER_LONG_SPEAR\tNONE\n"
					"SKILLCOMBATTRAITS\t34020\t0\t0\t0\n";
				Write_ValidValtanTimelineRows(bootstrap);
			}
			wchar_t previous[32768]{};
			const DWORD previousLength = GetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", previous,
				static_cast<DWORD>(std::size(previous)));
			SetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", wallContactRoot.c_str());
			CGameplayCatalog stageCatalog;
			const bool loaded = stageCatalog.Load();
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
				0u == previousLength || previousLength >= std::size(previous) ?
					nullptr : previous);
			return loaded;
		};
		const std::string acceptedContactDelayRows =
			"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\t600\tdamage.player.34120\t2\t242\t1\t2000\n"
			"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
			"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
			"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
			"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
			"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34120\t0\t0\t0\t0\n"
			"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n";
		const bool acceptedContactDelay =
			loadWithPatternStageRows(acceptedContactDelayRows, {});
		tests.Require(
			acceptedContactDelay,
			"Accept a stage whose first hit lands at its authored contact delay");
		std::string invalidLegacySpawnCountRows = acceptedContactDelayRows;
		const std::string validLegacySpawnSuffix =
			"SPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n";
		const std::size_t legacySpawnSuffixAt =
			invalidLegacySpawnCountRows.find(validLegacySpawnSuffix);
		if (std::string::npos != legacySpawnSuffixAt)
		{
			invalidLegacySpawnCountRows.replace(
				legacySpawnSuffixAt, validLegacySpawnSuffix.size(),
				"SPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t2\t0\n");
		}
		tests.Require(
			std::string::npos != legacySpawnSuffixAt &&
			!loadWithPatternStageRows(invalidLegacySpawnCountRows, {}),
			"Reject legacy combat-object action counts above one");
		tests.Require(
			!loadWithPatternStageRows(
				"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\t1000\tdamage.player.34120\t2\t242\t1\t2000\n"
				"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
				"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
				"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
				"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
				"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34120\t0\t0\t0\t0\n"
				"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n",
				{}),
			"Reject a hit delay at or beyond its stage duration");
		tests.Require(
			!loadWithPatternStageRows(
				"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tWINDUP\tvaltan.test.active\tWINDUP\t1000\tNONE\t0\t0\t0\t0\t0\t0\t0\t600\t-\t0\t0\t0\t0\n"
				"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
				"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
				"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
				"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
				"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34120\t0\t0\t0\t0\n"
				"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n",
				{}),
			"Reject a hit delay on a stage without a hit shape");

		const std::string explicitHitStageRows =
			"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t3\t0\t0\tdamage.player.34120\t2\t242\t1\t2000\n"
			"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
			"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
			"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
			"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
			"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34120\t0\t0\t0\t0\n"
			"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n";
		const std::vector<std::string> validExplicitHitOffsets{
			"PATTERNSTAGEHITOFFSET\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\t0\t100",
			"PATTERNSTAGEHITOFFSET\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\t1\t450",
			"PATTERNSTAGEHITOFFSET\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\t2\t900"
		};
		tests.Require(
			loadWithPatternStageRows(
				explicitHitStageRows, validExplicitHitOffsets),
			"Accept one complete ordered explicit boss hit schedule");
		tests.Require(
			!loadWithPatternStageRows(explicitHitStageRows, {
				validExplicitHitOffsets[0], validExplicitHitOffsets[1] }),
			"Reject an explicit boss hit schedule whose count is incomplete");
		tests.Require(
			!loadWithPatternStageRows(explicitHitStageRows, {
				validExplicitHitOffsets[0],
				"PATTERNSTAGEHITOFFSET\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\t1\t90",
				validExplicitHitOffsets[2] }),
			"Reject explicit boss hit offsets that are not strictly increasing");
		tests.Require(
			!loadWithPatternStageRows(explicitHitStageRows, {
				"PATTERNSTAGEHITOFFSET\tENCOUNTER_VALTAN\tVALTAN_MISSING\tvaltan.test.active\t0\t100",
				validExplicitHitOffsets[1], validExplicitHitOffsets[2] }),
			"Reject an explicit boss hit offset whose pattern owner is unknown");
		const std::string mixedScheduleStageRows =
			"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t3\t300\t0\tdamage.player.34120\t2\t242\t1\t2000\n" +
			explicitHitStageRows.substr(
				explicitHitStageRows.find("PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1"));
		tests.Require(
			!loadWithPatternStageRows(
				mixedScheduleStageRows, validExplicitHitOffsets),
			"Reject simultaneous legacy and explicit boss hit schedules");
		std::error_code cleanupError;
		fs::remove_all(wallContactRoot, cleanupError);
	}

	{
		/* A staged skill carries movement per stage, because a stage advance
		resets the action clock the curve is sampled on. */
		const PLAYER_SKILL_DEFINITION* basicAttack = catalog.Find_Skill(34010);
		bool everyStageCurveFits = nullptr != basicAttack &&
			!basicAttack->ComboStages.empty() &&
			basicAttack->RootMotion.empty();
		bool anyStageMoves = false;
		if (nullptr != basicAttack)
		{
			for (const PLAYER_COMBO_STAGE& stage : basicAttack->ComboStages)
			{
				if (stage.RootMotion.empty())
					continue;
				anyStageMoves = true;
				everyStageCurveFits = everyStageCurveFits &&
					stage.RootMotion.size() >= 2u &&
					stage.RootMotion.back().iTimeMs <= stage.iActionDurationMs;
			}
		}
		tests.Require(everyStageCurveFits && anyStageMoves,
			"Resolve per-stage root motion inside each combo stage duration");

		namespace fs = std::filesystem;
		const fs::path stageRoot =
			fs::temp_directory_path() / L"LostArkStageRootMotionContractTest";
		std::error_code stagePrepareError;
		const auto loadWithStageRow = [&](
			const char* stageIndex, const char* comboAdvanceMs)
		{
			fs::remove_all(stageRoot, stagePrepareError);
			fs::create_directories(stageRoot / L"Gameplay");
			{
				std::ofstream bootstrap(
					stageRoot / L"Gameplay" / L"Gameplay.bootstrap",
					std::ios::binary);
				bootstrap <<
					"LOSTARK_GAMEPLAY_BOOTSTRAP\t" << GAMEPLAY_BOOTSTRAP_VERSION <<
					'\t' << (19u + VALID_VALTAN_TIMELINE_ROW_COUNT) << "\n"
					"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\tHEALTH_PERCENT_THRESHOLD\t50\n"
					"BOSSPART\tBOSS_VALTAN\tboss.part.valtan.arm-armor\t2\t1000\t15\tGROGGY_ONLY\n"
					"DAMAGE\tdamage.player.34010\t100\n"
					"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t2\tANY\tANY\t0\n"
					"PATTERNPOLICY\tENCOUNTER_VALTAN\tVALTAN_TEST\tNORMAL\t1\t3\tLOCK_NEAREST_ON_START\tLOCK_FACING_ON_START\n"
					"PATTERNSOURCE\tENCOUNTER_VALTAN\tVALTAN_TEST\t420601\t12\t5000\t150\t350\t300\t180\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\t0\tdamage.player.34010\t0\t0\t0\t0\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
					"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
					"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
					"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
					"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34010\t0\t0\t0\t0\n"
					"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n"
					"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
					"SKILL\t34010\tLANCE_MASTER\tLMB\tlancemaster.skill.34010"
					"\t0\t1633\t470\t0\t0\t0\t3\tdamage.player.34010\tCOMBO"
					"\tLANCE_MASTER_LONG_SPEAR\tNONE\n"
					"SKILLCOMBATTRAITS\t34010\t0\t0\t0\n"
					"SKILLSTAGE\t34010\t0\t1633\t470\t" << comboAdvanceMs <<
					"\t329\t658\n"
					"SKILLSTAGE\t34010\t1\t1367\t356\t1367\t0\t0\n"
					"SKILLSTAGEROOTMOTION\t34010\t" << stageIndex <<
					"\t2\t0:0:0,1600:1.5:0\n";
				Write_ValidValtanTimelineRows(bootstrap);
			}
			wchar_t previous[32768]{};
			const DWORD previousLength = GetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", previous,
				static_cast<DWORD>(std::size(previous)));
			SetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", stageRoot.c_str());
			CGameplayCatalog stageCatalog;
			const bool loaded = stageCatalog.Load();
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
				0u == previousLength || previousLength >= std::size(previous) ?
					nullptr : previous);
			return loaded;
		};
		tests.Require(loadWithStageRow("0", "470"),
			"Accept a root motion row that names an existing combo stage");
		tests.Require(!loadWithStageRow("2", "470"),
			"Reject a root motion row past the last combo stage");
		tests.Require(!loadWithStageRow("0", "469"),
			"Reject a combo boundary before its damage time atomically");
		tests.Require(!loadWithStageRow("0", "1634"),
			"Reject a combo boundary after its stage duration atomically");
		std::error_code stageCleanupError;
		fs::remove_all(stageRoot, stageCleanupError);
	}

	{
		/* ?덈！??guards, and a hit taken inside that window is what buys the
		counter: no press advances it and the guard itself lands nothing. */
		SERVER_PLAYER counterPlayer{};
		counterPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		counterPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		counterPlayer.iCurrentHp = 1000;
		counterPlayer.iMaximumHp = 1000;
		counterPlayer.iCurrentResource = 1000;
		counterPlayer.iMaximumResource = 1000;
		CPlayerSkillSystem counterSkills;

		C2S_USE_SKILL counterCommand{};
		counterCommand.iClientSequence = 1;
		counterCommand.iSkillId = 34580;
		counterCommand.fAimX = 1.f;
		counterCommand.fAimZ = 0.f;
		tests.Require(
			counterSkills.Try_Start(counterPlayer, counterCommand, catalog, 10) &&
			1u == counterPlayer.iComboStage,
			"Approve the counter guard stage");

		std::vector<SERVER_WORLD_ENTITY> counterEntities;
		std::vector<DAMAGE_EVENT> counterDamageEvents;
		counterSkills.Update(counterPlayer, counterEntities, catalog, nullptr,
			nullptr, 1.f / 30.f, 11, counterDamageEvents);
		tests.Require(
			1u == counterPlayer.iComboStage && counterDamageEvents.empty(),
			"Hold the guard stage and land no damage while it runs");

		const std::uint32_t hpBeforeCounter = counterPlayer.iCurrentHp;
		tests.Require(
			CPlayerSkillSystem::Try_Counter(counterPlayer, catalog, 12) &&
			2u == counterPlayer.iComboStage &&
			hpBeforeCounter == counterPlayer.iCurrentHp &&
			0.f == counterPlayer.fActionElapsedSeconds,
			"Absorb the hit inside the guard window and promote to the counter");
		tests.Require(
			!CPlayerSkillSystem::Try_Counter(counterPlayer, catalog, 13),
			"Do not counter twice from one guard");

		SERVER_PLAYER lateCounter{};
		lateCounter.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		lateCounter.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		lateCounter.iCurrentHp = 1000;
		lateCounter.iMaximumHp = 1000;
		lateCounter.iCurrentResource = 1000;
		lateCounter.iMaximumResource = 1000;
		CPlayerSkillSystem lateSkills;
		C2S_USE_SKILL lateCommand = counterCommand;
		lateSkills.Try_Start(lateCounter, lateCommand, catalog, 10);
		lateCounter.fActionElapsedSeconds = 1.5f;
		tests.Require(
			!CPlayerSkillSystem::Try_Counter(lateCounter, catalog, 20) &&
			1u == lateCounter.iComboStage,
			"Reject a hit that lands after the guard window closed");

		SERVER_PLAYER comboPlayer{};
		comboPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		comboPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		comboPlayer.iCurrentHp = 1000;
		comboPlayer.iMaximumHp = 1000;
		comboPlayer.iCurrentResource = 1000;
		comboPlayer.iMaximumResource = 1000;
		CPlayerSkillSystem comboSkills;
		C2S_USE_SKILL basicAttack{};
		basicAttack.iClientSequence = 1;
		basicAttack.iSkillId = 34010;
		basicAttack.fAimX = 1.f;
		basicAttack.fAimZ = 0.f;
		comboSkills.Try_Start(comboPlayer, basicAttack, catalog, 10);
		tests.Require(
			!CPlayerSkillSystem::Try_Counter(comboPlayer, catalog, 11),
			"Never counter out of a skill that is not a COUNTER");
	}

	{
		SERVER_PLAYER stancePlayer{};
		stancePlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		stancePlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		stancePlayer.iCurrentHp = 1000;
		stancePlayer.iMaximumHp = 1000;
		stancePlayer.iCurrentResource = 1000;
		stancePlayer.iMaximumResource = 1000;
		CPlayerSkillSystem stanceSkills;

		C2S_USE_SKILL shortOnlySkill{};
		shortOnlySkill.iClientSequence = 1;
		shortOnlySkill.iSkillId = 34540;
		shortOnlySkill.fAimX = 1.f;
		shortOnlySkill.fAimZ = 0.f;
		tests.Require(!stanceSkills.Try_Start(stancePlayer, shortOnlySkill, catalog, 10),
			"Reject a short spear skill while in the long spear stance");

		C2S_USE_SKILL switchToShort{};
		switchToShort.iClientSequence = 2;
		switchToShort.iSkillId = 34000;
		switchToShort.fAimX = 1.f;
		switchToShort.fAimZ = 0.f;
		tests.Require(stanceSkills.Try_Start(stancePlayer, switchToShort, catalog, 10),
			"Approve the long to short spear stance transition");
		std::vector<SERVER_WORLD_ENTITY> stanceEntities;
		std::vector<DAMAGE_EVENT> stanceDamageEvents;
		for (std::uint32_t tick = 11; tick < 40; ++tick)
		{
			stanceSkills.Update(stancePlayer, stanceEntities, catalog, nullptr,
				nullptr, 1.f / 30.f, tick, stanceDamageEvents);
		}
		tests.Require(
			PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR == stancePlayer.eStance &&
			PLAYER_ACTION_STATE::NONE == stancePlayer.eAction,
			"Flip to the short spear stance once the transition action completes");

		C2S_USE_SKILL longOnlySkill{};
		longOnlySkill.iClientSequence = 3;
		longOnlySkill.iSkillId = 34120;
		longOnlySkill.fAimX = 1.f;
		longOnlySkill.fAimZ = 0.f;
		tests.Require(!stanceSkills.Try_Start(stancePlayer, longOnlySkill, catalog, 40),
			"Reject a long spear skill after switching to the short spear stance");

		C2S_USE_SKILL shortSkillNow = shortOnlySkill;
		shortSkillNow.iClientSequence = 4;
		tests.Require(stanceSkills.Try_Start(stancePlayer, shortSkillNow, catalog, 40),
			"Approve a short spear skill after switching to the short spear stance");
	}

	{
		SERVER_PLAYER holdPlayer{};
		holdPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		holdPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		holdPlayer.iCurrentHp = 1000;
		holdPlayer.iMaximumHp = 1000;
		holdPlayer.iCurrentResource = 1000;
		holdPlayer.iMaximumResource = 1000;
		CPlayerSkillSystem holdSkills;

		C2S_USE_SKILL chargeStart{};
		chargeStart.iClientSequence = 1;
		chargeStart.iSkillId = 34590;
		chargeStart.fAimX = 1.f;
		chargeStart.fAimZ = 0.f;
		tests.Require(holdSkills.Try_Start(holdPlayer, chargeStart, catalog, 10),
			"Approve the short spear hold skill");

		C2S_UPDATE_SKILL_AIM turnedAim{};
		turnedAim.iClientSequence = 2;
		turnedAim.iSkillId = 34590;
		turnedAim.fAimX = 0.f;
		turnedAim.fAimZ = -1.f;
		const float chargeYaw = holdPlayer.fYawDegrees;
		holdSkills.Update_Aim(holdPlayer, turnedAim, catalog);
		tests.Require(
			holdPlayer.fYawDegrees != chargeYaw &&
			holdPlayer.fSkillAimDirectionZ < -0.99f,
			"Turn a charging hold skill toward a new aim");

		C2S_UPDATE_SKILL_AIM wrongSkillAim = turnedAim;
		wrongSkillAim.iClientSequence = 3;
		wrongSkillAim.iSkillId = 34540;
		wrongSkillAim.fAimX = 1.f;
		wrongSkillAim.fAimZ = 1.f;
		holdSkills.Update_Aim(holdPlayer, wrongSkillAim, catalog);
		tests.Require(holdPlayer.fSkillAimDirectionZ < -0.99f,
			"Ignore an aim update naming a skill that is not running");

		C2S_RELEASE_SKILL chargeRelease{};
		chargeRelease.iClientSequence = 2u;
		chargeRelease.iSkillId = 34590u;
		holdSkills.Release(holdPlayer, chargeRelease, catalog);
		tests.Require(
			holdPlayer.hasReleasedHold &&
			2u == holdPlayer.iLastSkillSequence,
			"Consume release for HOLD while COMBO release remains unsupported");
		C2S_UPDATE_SKILL_AIM releasedAim = turnedAim;
		releasedAim.iClientSequence = 4;
		releasedAim.fAimX = 1.f;
		releasedAim.fAimZ = 0.f;
		holdSkills.Update_Aim(holdPlayer, releasedAim, catalog);
		tests.Require(holdPlayer.fSkillAimDirectionZ < -0.99f,
			"Keep the last aim once the hold key is released");

		holdPlayer.hasReleasedHold = false;
		holdPlayer.iComboStage = 3u;
		C2S_UPDATE_SKILL_AIM firingAim = turnedAim;
		firingAim.iClientSequence = 5;
		firingAim.fAimX = 1.f;
		firingAim.fAimZ = 0.f;
		holdSkills.Update_Aim(holdPlayer, firingAim, catalog);
		tests.Require(holdPlayer.fSkillAimDirectionZ < -0.99f,
			"Keep the last aim through the firing stage");

		SERVER_PLAYER activePlayer{};
		activePlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		activePlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		activePlayer.iCurrentHp = 1000;
		activePlayer.iMaximumHp = 1000;
		activePlayer.iCurrentResource = 1000;
		activePlayer.iMaximumResource = 1000;
		CPlayerSkillSystem activeSkills;
		C2S_USE_SKILL activeStart{};
		activeStart.iClientSequence = 1;
		activeStart.iSkillId = 34540;
		activeStart.fAimX = 1.f;
		activeStart.fAimZ = 0.f;
		tests.Require(activeSkills.Try_Start(activePlayer, activeStart, catalog, 10),
			"Approve a non-hold short spear skill for the aim guard");
		const float activeAimX = activePlayer.fSkillAimDirectionX;
		C2S_UPDATE_SKILL_AIM activeAim{};
		activeAim.iClientSequence = 2;
		activeAim.iSkillId = 34540;
		activeAim.fAimX = 0.f;
		activeAim.fAimZ = -1.f;
		activeSkills.Update_Aim(activePlayer, activeAim, catalog);
		tests.Require(activeAimX == activePlayer.fSkillAimDirectionX,
			"Ignore an aim update on a skill that is not a HOLD");
	}

	{
		CSpawnGroupBootstrap spawnBootstrap;
		const bool loaded =
			spawnBootstrap.Load(WORLD_ID::CHARACTER_SELECT_ARENA);
		const auto& groups = spawnBootstrap.Get_Groups();
		const auto monsterGroup = std::find_if(
			groups.begin(), groups.end(),
			[](const SPAWN_GROUP_DEFINITION& group)
			{
				return group.strSpawnGroupId ==
					"spawn.character-select.monster";
			});
		const auto minibossGroup = std::find_if(
			groups.begin(), groups.end(),
			[](const SPAWN_GROUP_DEFINITION& group)
			{
				return group.strSpawnGroupId ==
					"spawn.character-select.miniboss";
			});
		const SPAWN_GROUP_ANCHOR* monsterAnchor = spawnBootstrap.Find_Anchor(
			"anchor.character-select.monster");
		const SPAWN_GROUP_ANCHOR* minibossAnchor = spawnBootstrap.Find_Anchor(
			"anchor.character-select.miniboss");
		const MONSTER_RUNTIME_PROFILE* monsterProfile =
			spawnBootstrap.Find_Profile("MONSTER_VALTAN_PADD_01");
		const MONSTER_RUNTIME_PROFILE* minibossProfile =
			spawnBootstrap.Find_Profile("MINIBOSS_LUGARU");
		/* Named rather than counted: the audition Area gains a group whenever a
		new archetype needs somewhere to be tried out, and a bare count would
		make every such addition look like a regression. */
		tests.Require(
			loaded && 1u == spawnBootstrap.Get_Revision() && !groups.empty() &&
			groups.end() != monsterGroup && groups.end() != minibossGroup &&
			nullptr != monsterAnchor && nullptr != minibossAnchor &&
			nullptr != monsterProfile && nullptr != minibossProfile,
			"Load the Character Select spawn groups, anchors, and profiles");
		tests.Require(
			nullptr != monsterProfile && nullptr != minibossProfile &&
			std::fabs(monsterProfile->fHitKnockbackScale - 1.f) < 0.0001f &&
			0.f == minibossProfile->fHitKnockbackScale,
			"Read the published hit knockback scale for the monster and the miniboss");
		tests.Require(
			nullptr != monsterProfile && nullptr != minibossProfile &&
			std::fabs(monsterProfile->fTargetReleaseRange - 24.f) < 0.0001f &&
			std::fabs(monsterProfile->fTurnSpeedDegreesPerSecond - 420.f) < 0.0001f &&
			std::fabs(monsterProfile->fAcceleration - 12.f) < 0.0001f &&
			std::fabs(monsterProfile->fDeceleration - 16.f) < 0.0001f &&
			std::fabs(monsterProfile->fArrivalSlowRadius - 1.6f) < 0.0001f &&
			std::fabs(minibossProfile->fTargetReleaseRange - 36.f) < 0.0001f,
			"Read monster target hysteresis, turn, acceleration, deceleration, and arrival slowdown from bootstrap v4");
		tests.Require(
			nullptr != monsterProfile && nullptr != minibossProfile &&
			std::fabs(monsterProfile->fAttackPushRangeM - 0.5f) < 0.0001f &&
			150u == monsterProfile->iAttackPushMs &&
			!monsterProfile->bAttackKnockdown &&
			0u == monsterProfile->iAttackDownMs &&
			std::fabs(minibossProfile->fAttackPushRangeM - 2.f) < 0.0001f &&
			250u == minibossProfile->iAttackPushMs &&
			minibossProfile->bAttackKnockdown &&
			2000u == minibossProfile->iAttackDownMs,
			"Read the published attack push and knockdown for the monster and the miniboss");

		const auto hasImmediateEntry = [](
			const SPAWN_GROUP_DEFINITION& group,
			const char* archetypeId,
			const char* anchorId)
		{
			return group.strRequiredCompletedGroupId.empty() &&
				1u == group.iMaxAlive && 1u == group.Waves.size() &&
				0u == group.Waves[0].iStartDelayMs &&
				1u == group.Waves[0].Entries.size() &&
				group.Waves[0].Entries[0].strArchetypeId == archetypeId &&
				1u == group.Waves[0].Entries[0].iCount &&
				group.Waves[0].Entries[0].strAnchorId == anchorId &&
				0u == group.Waves[0].Entries[0].iInitialDelayMs &&
				0u == group.Waves[0].Entries[0].iSpawnIntervalMs;
		};
		tests.Require(
			groups.end() != monsterGroup && groups.end() != minibossGroup &&
			hasImmediateEntry(
				*monsterGroup,
				"MONSTER_VALTAN_PADD_01",
				"anchor.character-select.monster") &&
			hasImmediateEntry(
				*minibossGroup,
				"MINIBOSS_LUGARU",
				"anchor.character-select.miniboss"),
			"Keep Character Select audition groups single-wave and zero-delay");

		CSpawnGroupRuntime immediateRuntime;
		std::string immediateStatus;
		std::uint32_t immediateSpawnCount = 0u;
		const bool immediateInitialized =
			immediateRuntime.Initialize(spawnBootstrap, immediateStatus);
		const bool failedImmediatePreservedDormant = immediateInitialized &&
			!immediateRuntime.Activate_Immediate(
				"spawn.character-select.monster",
				spawnBootstrap,
				[](const std::string&, const SPAWN_GROUP_ENTRY&,
					const SPAWN_GROUP_ANCHOR&,
					const MONSTER_RUNTIME_PROFILE&, std::uint32_t)
				{
					return false;
				}) &&
			!immediateRuntime.Is_ActiveOrCompleted(
				"spawn.character-select.monster");
		const auto countImmediateSpawn = [&immediateSpawnCount](
			const std::string&, const SPAWN_GROUP_ENTRY&,
			const SPAWN_GROUP_ANCHOR&, const MONSTER_RUNTIME_PROFILE&,
			std::uint32_t)
			{
				++immediateSpawnCount;
				return true;
			};
		tests.Require(
			failedImmediatePreservedDormant &&
			immediateRuntime.Activate_Immediate(
				"spawn.character-select.monster",
				spawnBootstrap,
				countImmediateSpawn) &&
			immediateRuntime.Activate_Immediate(
				"spawn.character-select.miniboss",
				spawnBootstrap,
				countImmediateSpawn) &&
			2u == immediateSpawnCount &&
			immediateRuntime.Is_ActiveOrCompleted(
				"spawn.character-select.monster") &&
			!immediateRuntime.Activate_Immediate(
				"spawn.character-select.monster",
				spawnBootstrap,
				countImmediateSpawn),
			"Commit immediate audition activation only after its spawn callback succeeds");

		CSpawnGroupRuntime spawnRuntime;
		std::string spawnStatus;
		const bool initialized =
			spawnRuntime.Initialize(spawnBootstrap, spawnStatus);
		tests.Require(
			initialized &&
			spawnRuntime.Activate("spawn.character-select.monster") &&
			spawnRuntime.Activate("spawn.character-select.miniboss") &&
			spawnRuntime.Is_ActiveOrCompleted(
				"spawn.character-select.monster") &&
			!spawnRuntime.Activate("spawn.character-select.monster"),
			"Activate both Character Select audition groups");
		std::array<std::uint32_t, 2> scheduledByGroup{};
		bool callbackContractValid = true;
		spawnRuntime.Update(
			1.f / 30.f,
			spawnBootstrap,
			[](const std::string&) { return 0u; },
			[&scheduledByGroup, &callbackContractValid](
				const std::string& spawnGroupId,
				const SPAWN_GROUP_ENTRY& entry,
				const SPAWN_GROUP_ANCHOR& anchor,
				const MONSTER_RUNTIME_PROFILE& profile,
				const std::uint32_t ordinal)
			{
				if (spawnGroupId == "spawn.character-select.monster")
				{
					++scheduledByGroup[0];
					callbackContractValid = callbackContractValid &&
						entry.strArchetypeId == "MONSTER_VALTAN_PADD_01" &&
						anchor.strAnchorId == "anchor.character-select.monster" &&
						profile.strArchetypeId == entry.strArchetypeId &&
						0u == ordinal;
				}
				else if (spawnGroupId == "spawn.character-select.miniboss")
				{
					++scheduledByGroup[1];
					callbackContractValid = callbackContractValid &&
						entry.strArchetypeId == "MINIBOSS_LUGARU" &&
						anchor.strAnchorId == "anchor.character-select.miniboss" &&
						profile.strArchetypeId == entry.strArchetypeId &&
						0u == ordinal;
				}
				else
				{
					callbackContractValid = false;
				}
				return true;
			});
		tests.Require(
			callbackContractValid && 1u == scheduledByGroup[0] &&
			1u == scheduledByGroup[1] &&
			2u == scheduledByGroup[0] + scheduledByGroup[1],
			"Schedule exactly two Character Select callbacks in the first update");

		CGameRoom resetRoom{ WORLD_ID::CHARACTER_SELECT_ARENA };
		SERVER_PLAYER resetPlayer{};
		resetPlayer.iSessionId = 501u;
		resetPlayer.iPlayerId = 502u;
		resetPlayer.iNetEntityId = 503u;
		resetPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		resetPlayer.strNickName = "ResetFixture";
		resetPlayer.iCurrentHp = 100u;
		resetPlayer.iMaximumHp = 100u;
		resetPlayer.isCombatReady = true;
		resetRoom.m_Players.emplace(resetPlayer.iPlayerId, resetPlayer);
		resetRoom.m_PlayerIdBySessionId.emplace(
			resetPlayer.iSessionId, resetPlayer.iPlayerId);
		resetRoom.m_PlayerIdByEntityId.emplace(
			resetPlayer.iNetEntityId, resetPlayer.iPlayerId);
		const bool resetGroupActivated =
			resetRoom.m_SpawnGroupRuntime.Activate_Immediate(
				"spawn.character-select.monster",
				resetRoom.m_SpawnGroupBootstrap,
				[&resetRoom](const std::string& spawnGroupId,
					const SPAWN_GROUP_ENTRY& entry,
					const SPAWN_GROUP_ANCHOR& anchor,
					const MONSTER_RUNTIME_PROFILE& profile,
					const std::uint32_t ordinal)
				{
					return resetRoom.Spawn_Monster(
						spawnGroupId, entry, anchor, profile, ordinal);
				});
		const bool spawnedBeforeLeave = std::any_of(
			resetRoom.m_WorldEntities.begin(),
			resetRoom.m_WorldEntities.end(),
			[](const SERVER_WORLD_ENTITY& entity)
			{
				return entity.strSpawnGroupId ==
					"spawn.character-select.monster";
			});
		resetRoom.Leave(
			resetPlayer.iSessionId,
			PLAYER_DESPAWN_REASON::DISCONNECTED);
		tests.Require(
			resetGroupActivated && spawnedBeforeLeave &&
			resetRoom.m_Players.empty() &&
			resetRoom.m_WorldEntities.empty() &&
			resetRoom.m_SpawnGroupRuntime.Activate(
				"spawn.character-select.monster"),
			"Reset Character Select dynamic entities and spawn groups after the room becomes empty");

		CGameRoom retirementRoom{ WORLD_ID::CHARACTER_SELECT_ARENA };
		ROOM_COMMAND queuedLeave{};
		queuedLeave.eType = ROOM_COMMAND_TYPE::LEAVE;
		queuedLeave.iSessionId = 601u;
		queuedLeave.eLeaveReason = PLAYER_DESPAWN_REASON::DISCONNECTED;
		const bool queuedBeforeRetirement =
			retirementRoom.Enqueue(std::move(queuedLeave));
		const bool sealedBeforeDrain =
			retirementRoom.Try_SealPrivateArenaForRetirement();
		retirementRoom.Tick(1.f / 30.f);
		const bool sealedAfterDrain =
			retirementRoom.Try_SealPrivateArenaForRetirement();

		ROOM_COMMAND commandAfterSeal{};
		commandAfterSeal.eType = ROOM_COMMAND_TYPE::LEAVE;
		commandAfterSeal.iSessionId = 602u;
		commandAfterSeal.eLeaveReason =
			PLAYER_DESPAWN_REASON::DISCONNECTED;
		tests.Require(
			queuedBeforeRetirement && !sealedBeforeDrain &&
			sealedAfterDrain &&
			!retirementRoom.Enqueue(std::move(commandAfterSeal)),
			"Retire a private Character Select arena only after queued leave work drains");

		CGameRoom sharedRoom{ WORLD_ID::BERN };
		tests.Require(
			!sharedRoom.Try_SealPrivateArenaForRetirement(),
			"Never seal a shared world through the private arena retirement path");
	}

	{
		CGameRoom raidRoom{ WORLD_ID::VALTAN_ARENA };
		const auto& placements = raidRoom.m_WorldBootstrap.Get_Placements();
		std::vector<const WORLD_BOOTSTRAP_PLACEMENT*> playerSpawns;
		for (const WORLD_BOOTSTRAP_PLACEMENT& placement : placements)
		{
			if (placement.isEnabled &&
				WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind)
			{
				playerSpawns.push_back(&placement);
			}
		}

		auto addPlayer = [&raidRoom](
			const SESSION_ID sessionId,
			const PLAYER_ID playerId,
			const NET_ENTITY_ID entityId,
			const WORLD_BOOTSTRAP_PLACEMENT& spawn)
		{
			SERVER_PLAYER player{};
			player.iSessionId = sessionId;
			player.iPlayerId = playerId;
			player.iNetEntityId = entityId;
			player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			player.strNickName = "RaidFixture" + std::to_string(playerId);
			player.strSpawnPlacementId = spawn.strPlacementId;
			player.fPositionX = spawn.fPositionX;
			player.fPositionY = spawn.fPositionY;
			player.fPositionZ = spawn.fPositionZ;
			player.iCurrentHp = 100u;
			player.iMaximumHp = 100u;
			player.isCombatReady = true;
			raidRoom.m_Players.emplace(playerId, player);
			raidRoom.m_PlayerIdBySessionId.emplace(sessionId, playerId);
			raidRoom.m_PlayerIdByEntityId.emplace(entityId, playerId);
		};

		if (raidRoom.Is_Ready() && 4u == playerSpawns.size())
		{
			for (std::size_t index = 0; index < playerSpawns.size(); ++index)
			{
				addPlayer(
					1001u + index,
					static_cast<PLAYER_ID>(2001u + index),
					static_cast<NET_ENTITY_ID>(3001u + index),
					*playerSpawns[index]);
			}
		}
		std::vector<std::shared_ptr<CClientSession>> registeredSessions;
		for (SESSION_ID sessionId = 1001u; sessionId <= 1005u; ++sessionId)
		{
			auto session = std::make_shared<CClientSession>(
				sessionId, INVALID_SOCKET,
				CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
			session->m_isSendRunning.store(true);
			raidRoom.Handle_Register(session);
			registeredSessions.push_back(std::move(session));
		}
		const PLAYER_ID allocatorBeforeFull = raidRoom.m_iNextPlayerId;
		const NET_ENTITY_ID entityAllocatorBeforeFull = raidRoom.m_iNextNetEntityId;
		const std::size_t playersBeforeFull = raidRoom.m_Players.size();
		const std::size_t sessionOwnersBeforeFull =
			raidRoom.m_PlayerIdBySessionId.size();
		const std::size_t entityOwnersBeforeFull =
			raidRoom.m_PlayerIdByEntityId.size();
		C2S_ENTER_WORLD fifthEntry{};
		fifthEntry.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
		fifthEntry.eWorldId = WORLD_ID::VALTAN_ARENA;
		fifthEntry.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		fifthEntry.strNickName = "FifthRaidFixture";
		const bool fifthRejected = !raidRoom.Join(1005u, fifthEntry);
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC fifthDiagnostic =
			registeredSessions.back()->Get_CloseDiagnostic();
		tests.Require(
			raidRoom.Is_Ready() && 4u == playerSpawns.size() &&
			raidRoom.Is_PlayerAdmissionFull() &&
			nullptr == raidRoom.Find_AvailablePlayerSpawn() &&
			fifthRejected &&
			SESSION_DIAGNOSTIC_REASON::SERVER_EXPECTED_ROOM_FULL ==
				fifthDiagnostic.eReason &&
			std::string::npos !=
				fifthDiagnostic.strContext.find("activePlayers=4") &&
			std::string::npos !=
				fifthDiagnostic.strContext.find(
					"registeredSessionsIncludingCandidate=5") &&
			std::string::npos !=
				fifthDiagnostic.strContext.find("candidateSessionId=1005") &&
			std::string::npos !=
				fifthDiagnostic.strContext.find("candidateRegistered=true") &&
			std::string::npos !=
				fifthDiagnostic.strContext.find("enabledPlayerSpawns=4") &&
			std::string::npos !=
				fifthDiagnostic.strContext.find("sessionId=1001") &&
			std::string::npos !=
				fifthDiagnostic.strContext.find("playerId=2001") &&
			std::string::npos !=
				fifthDiagnostic.strContext.find("peer=unknown:0") &&
			fifthDiagnostic.strContext.size() <= 1024u &&
			allocatorBeforeFull == raidRoom.m_iNextPlayerId &&
			entityAllocatorBeforeFull == raidRoom.m_iNextNetEntityId &&
			playersBeforeFull == raidRoom.m_Players.size() &&
			sessionOwnersBeforeFull == raidRoom.m_PlayerIdBySessionId.size() &&
			entityOwnersBeforeFull == raidRoom.m_PlayerIdByEntityId.size(),
			"Reject a fifth Valtan admission as room full without mutating room ownership or allocators");
		registeredSessions.back()->Request_Close();
		raidRoom.m_Sessions.erase(1005u);

		const std::string releasedSpawnId =
			playerSpawns.size() < 2u ? std::string{} :
			playerSpawns[1]->strPlacementId;
		raidRoom.Leave(1002u, PLAYER_DESPAWN_REASON::DISCONNECTED);
		const WORLD_BOOTSTRAP_PLACEMENT* releasedSpawn =
			raidRoom.Find_AvailablePlayerSpawn();
		const bool releasedSlotAvailable = nullptr != releasedSpawn &&
			releasedSpawn->strPlacementId == releasedSpawnId;
		if (releasedSlotAvailable)
		{
			addPlayer(1010u, 2010u, 3010u, *releasedSpawn);
		}
		tests.Require(
			releasedSlotAvailable && raidRoom.Is_PlayerAdmissionFull() &&
			4u == raidRoom.m_Players.size(),
			"Release a disconnected Valtan slot and admit a replacement into the same stable spawn");

		const WORLD_BOOTSTRAP_PLACEMENT* bossTrigger =
			raidRoom.Find_Placement("Stage_Boss");
		if (nullptr != bossTrigger && !raidRoom.m_Players.empty())
		{
			SERVER_PLAYER& triggerPlayer = raidRoom.m_Players.begin()->second;
			triggerPlayer.fPositionX = bossTrigger->fPositionX;
			triggerPlayer.fPositionY = bossTrigger->fPositionY -
				LostArk::Shared::WorldCollision::PLAYER_CENTER_OFFSET_Y;
			triggerPlayer.fPositionZ = bossTrigger->fPositionZ;
		}
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::uint32_t encounterActivationCount = 0u;
		raidRoom.m_ServerTriggerSystem.Evaluate_Entries(
			raidRoom.m_Players,
			700u,
			transfers,
			[&raidRoom, &encounterActivationCount](
				const WORLD_TRIGGER_ACTION_KIND kind,
				const std::string& targetId)
			{
				if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER != kind)
					return false;
				++encounterActivationCount;
				return raidRoom.Activate_Encounter(targetId);
			});
		auto bossBeforeReset = std::find_if(
			raidRoom.m_WorldEntities.begin(),
			raidRoom.m_WorldEntities.end(),
			[](const SERVER_WORLD_ENTITY& entity)
			{
				return "boss.valtan.center" == entity.strPlacementId;
			});
		const bool bossActivatedBeforeReset =
			1u == encounterActivationCount &&
			raidRoom.m_WorldEntities.end() != bossBeforeReset;
		if (raidRoom.m_WorldEntities.end() != bossBeforeReset)
		{
			bossBeforeReset->iCurrentHp = 1u;
			bossBeforeReset->iPhase = 2u;
			bossBeforeReset->eAction = SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
			bossBeforeReset->strPatternId = "reset.fixture.pattern";
		}
		const bool spawnGroupActivatedBeforeReset =
			raidRoom.m_SpawnGroupRuntime.Activate("spawn.valtan.stage01");
		SERVER_WORLD_ENTITY dynamicMonster{};
		dynamicMonster.iNetEntityId = 9001u;
		dynamicMonster.eKind = WORLD_BOOTSTRAP_KIND::MONSTER;
		dynamicMonster.strPlacementId = "reset.fixture.monster";
		dynamicMonster.strSpawnGroupId = "spawn.valtan.stage01";
		raidRoom.m_WorldEntities.push_back(std::move(dynamicMonster));
		DAMAGE_EVENT damageEvent{};
		damageEvent.iTargetNetEntityId = 3001u;
		damageEvent.iAmount = 1u;
		raidRoom.m_TickDamageEvents.push_back(damageEvent);
		raidRoom.m_iServerTick = 777u;
		SERVER_WORLD_TRANSFER_REQUEST pendingTransfer{};
		pendingTransfer.iSessionId = 8080u;
		pendingTransfer.eTargetWorldId = WORLD_ID::BERN;
		pendingTransfer.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
		pendingTransfer.strNickName = "PendingTransfer";
		raidRoom.m_PendingWorldTransfers.push_back(pendingTransfer);
		const PLAYER_ID playerAllocatorBeforeReset = raidRoom.m_iNextPlayerId;
		const NET_ENTITY_ID netAllocatorBeforeReset = raidRoom.m_iNextNetEntityId;

		std::vector<SESSION_ID> activeSessions;
		for (const auto& [sessionId, playerId] : raidRoom.m_PlayerIdBySessionId)
		{
			(void)playerId;
			activeSessions.push_back(sessionId);
		}
		for (const SESSION_ID sessionId : activeSessions)
		{
			raidRoom.Leave(sessionId, PLAYER_DESPAWN_REASON::DISCONNECTED);
		}
		const bool emptyResetPreservedMonotonicState =
			raidRoom.Is_Ready() && raidRoom.m_Players.empty() &&
			raidRoom.m_PlayerIdBySessionId.empty() &&
			raidRoom.m_PlayerIdByEntityId.empty() &&
			raidRoom.m_WorldEntities.empty() &&
			raidRoom.m_TickDamageEvents.empty() &&
			777u == raidRoom.m_iServerTick &&
			playerAllocatorBeforeReset == raidRoom.m_iNextPlayerId &&
			netAllocatorBeforeReset == raidRoom.m_iNextNetEntityId &&
			1u == raidRoom.m_PendingWorldTransfers.size() &&
			8080u == raidRoom.m_PendingWorldTransfers.front().iSessionId;
		const bool spawnGroupReactivated =
			raidRoom.m_SpawnGroupRuntime.Activate("spawn.valtan.stage01");

		if (!playerSpawns.empty() && nullptr != bossTrigger)
		{
			addPlayer(1020u, 2020u, 3020u, *playerSpawns.front());
			SERVER_PLAYER& nextGenerationPlayer =
				raidRoom.m_Players.find(2020u)->second;
			nextGenerationPlayer.fPositionX = bossTrigger->fPositionX;
			nextGenerationPlayer.fPositionY = bossTrigger->fPositionY -
				LostArk::Shared::WorldCollision::PLAYER_CENTER_OFFSET_Y;
			nextGenerationPlayer.fPositionZ = bossTrigger->fPositionZ;
		}
		encounterActivationCount = 0u;
		raidRoom.m_ServerTriggerSystem.Evaluate_Entries(
			raidRoom.m_Players,
			778u,
			transfers,
			[&raidRoom, &encounterActivationCount](
				const WORLD_TRIGGER_ACTION_KIND kind,
				const std::string& targetId)
			{
				if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER != kind)
					return false;
				++encounterActivationCount;
				return raidRoom.Activate_Encounter(targetId);
			});
		const auto bossAfterReset = std::find_if(
			raidRoom.m_WorldEntities.begin(),
			raidRoom.m_WorldEntities.end(),
			[](const SERVER_WORLD_ENTITY& entity)
			{
				return "boss.valtan.center" == entity.strPlacementId;
			});
		const bool encounterRestarted =
			1u == encounterActivationCount &&
			raidRoom.m_WorldEntities.end() != bossAfterReset &&
			bossAfterReset->iCurrentHp == bossAfterReset->iMaximumHp &&
			1u == bossAfterReset->iPhase &&
			SERVER_ENTITY_ACTION::IDLE == bossAfterReset->eAction &&
			bossAfterReset->strPatternId.empty();
		tests.Require(
			bossActivatedBeforeReset && spawnGroupActivatedBeforeReset &&
			emptyResetPreservedMonotonicState && spawnGroupReactivated &&
			encounterRestarted,
			"Reset Valtan boss, triggers, spawn groups, dynamic entities, and damage after the last disconnect while preserving IDs, tick, and transfers");
	}

	{
		CSpawnGroupBootstrap spawnBootstrap;
		const bool loaded = spawnBootstrap.Load(WORLD_ID::VALTAN_ARENA);
		tests.Require(
			loaded && 3u == spawnBootstrap.Get_Groups().size(),
			"Load three authored Valtan spawn groups");
		tests.Require(
			loaded && Reject_CorruptSpawnAnchorReloadTransactionally(
				spawnBootstrap),
			"Reject nonfinite or out-of-range spawn anchors and preserve the admitted bootstrap");
		CSpawnGroupRuntime spawnRuntime;
		std::string spawnStatus;
		tests.Require(
			spawnRuntime.Initialize(spawnBootstrap, spawnStatus),
			"Initialize Valtan spawn group runtime");
		tests.Require(
			!spawnRuntime.Activate("spawn.valtan.stage02.miniboss"),
			"Reject miniboss group before Stage 1 completion");
		tests.Require(
			spawnRuntime.Activate("spawn.valtan.stage01"),
			"Activate Stage 1 spawn group exactly once");
		/* Counted from the loaded groups rather than written down, because the
		wave composition is authoring that changes whenever the corridor is
		retuned. What the contract owns is that every authored entry is
		scheduled exactly once and the group then completes. */
		std::uint32_t authoredMonsterCount = 0;
		for (const SPAWN_GROUP_DEFINITION& definition : spawnBootstrap.Get_Groups())
		{
			if (definition.strSpawnGroupId != "spawn.valtan.stage01")
				continue;
			for (const SPAWN_GROUP_WAVE& wave : definition.Waves)
				for (const SPAWN_GROUP_ENTRY& groupEntry : wave.Entries)
					authoredMonsterCount += groupEntry.iCount;
		}
		std::uint32_t scheduledMonsterCount = 0;
		for (std::uint32_t step = 0; step < 64u &&
			!spawnRuntime.Is_Completed("spawn.valtan.stage01"); ++step)
		{
			spawnRuntime.Update(
				1.f,
				spawnBootstrap,
				[](const std::string&) { return 0u; },
				[&scheduledMonsterCount](const std::string&,
					const SPAWN_GROUP_ENTRY&,
					const SPAWN_GROUP_ANCHOR&,
					const MONSTER_RUNTIME_PROFILE&,
					const std::uint32_t)
				{
					++scheduledMonsterCount;
					return true;
				});
		}
		tests.Require(
			0u != authoredMonsterCount &&
			authoredMonsterCount == scheduledMonsterCount &&
			spawnRuntime.Is_Completed("spawn.valtan.stage01"),
			"Schedule all Stage 1 waves and complete after all entities clear");
		tests.Require(
			spawnRuntime.Activate("spawn.valtan.stage02.miniboss"),
			"Unlock miniboss group after Stage 1 completion");
	}

	{
		auto makeDestructionGraph = [](
			const std::uint32_t breakingDurationTicks,
			const WORLD_DESTRUCTION_TRIGGER_KIND triggerKind,
			const bool hasWorldMutationChannels = true)
		{
			WORLD_DESTRUCTION_DESCRIPTOR_GRAPH graph{};
			graph.Groups.push_back({
				"destroyable.group.valtan.wall.3705102",
				{
					"deploy.valtan.wall.3705102.0",
					"deploy.valtan.wall.3705102.1",
					"deploy.valtan.wall.3705102.2",
					"deploy.valtan.wall.3705102.3",
					"deploy.valtan.wall.3705102.4"
				},
				WORLD_DESTRUCTION_STATE::INTACT });
			graph.Mutations.push_back({
				"mutation.valtan.wall.3705102.fracture",
				"destroyable.group.valtan.wall.3705102",
				WORLD_DESTRUCTION_STATE::FRACTURED,
				breakingDurationTicks,
				hasWorldMutationChannels ?
					"collision.valtan.wall.3705102.fractured" : "",
				hasWorldMutationChannels ?
					"navigation.valtan.wall.3705102.open" : "" });
			graph.Bindings.push_back({
				"binding.valtan.wall.3705102.impact",
				"mutation.valtan.wall.3705102.fracture",
				triggerKind,
				"VALTAN_ARMOR_BREAK_OPENING",
				"WALL_CHARGE",
				"valtan.armor_break.wall_charge",
				1u,
				WORLD_DESTRUCTION_TRIGGER_KIND::BOSS_IMPACT == triggerKind ?
					"receiver.valtan.wall.3705102" : "" });
			return graph;
		};

		const WORLD_DESTRUCTION_ACTION_TUPLE exactAction{
			"VALTAN_ARMOR_BREAK_OPENING",
			"WALL_CHARGE",
			"valtan.armor_break.wall_charge",
			1u };
		constexpr std::uint64_t SOURCE_BOSS_ENTITY_ID = 7001u;
		CWorldDestructionRuntime runtime;
		std::string status;
		WORLD_DESTRUCTION_TRANSACTION transaction{};
		tests.Require(
			runtime.Initialize(makeDestructionGraph(
				3u, WORLD_DESTRUCTION_TRIGGER_KIND::BOSS_IMPACT), status) &&
			1u == runtime.Get_EncounterEpoch(),
			"Initialize a nonzero-epoch world destruction graph transactionally");

		WORLD_DESTRUCTION_ACTION_TUPLE wrongAction = exactAction;
		wrongAction.strActionId = "valtan.armor_break.wall_charge.wrong";
		tests.Require(
			WORLD_DESTRUCTION_PREPARE_RESULT::NO_MATCH ==
				runtime.Prepare_ImpactTrigger(
					wrongAction, "receiver.valtan.wall.3705102",
					SOURCE_BOSS_ENTITY_ID, 7u, 10u, transaction, status) &&
			transaction.Transitions.empty() &&
			WORLD_DESTRUCTION_PREPARE_RESULT::NO_MATCH ==
				runtime.Prepare_ImpactTrigger(
					exactAction, "receiver.valtan.wall.other",
					SOURCE_BOSS_ENTITY_ID, 7u, 10u, transaction, status),
			"Reject non-exact action tuples and impact receivers");

		const bool preparedBreaking =
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
			runtime.Prepare_ImpactTrigger(
				exactAction, "receiver.valtan.wall.3705102",
				SOURCE_BOSS_ENTITY_ID, 7u, 10u, transaction, status) &&
			1u == transaction.BindingApplications.size() &&
			1u == transaction.Transitions.size() &&
			WORLD_DESTRUCTION_STATE::INTACT ==
				transaction.Transitions.front().ePreviousState &&
			WORLD_DESTRUCTION_STATE::BREAKING ==
				transaction.Transitions.front().eNextState &&
			13u == transaction.Transitions.front().iCommitTick &&
			!transaction.Transitions.front().bApplyPersistentMutation &&
			5u == transaction.Transitions.front().MemberPlacementIds.size() &&
			transaction.Transitions.front().strCollisionStateId ==
				"collision.valtan.wall.3705102.fractured" &&
			transaction.Transitions.front().strNavigationStateId ==
				"navigation.valtan.wall.3705102.open";
		tests.Require(
			preparedBreaking && runtime.Commit(transaction, status),
			"Prepare and atomically commit one BREAKING transition");

		WORLD_DESTRUCTION_GROUP_STATE groupState{};
		tests.Require(
			runtime.Find_GroupState(
				"destroyable.group.valtan.wall.3705102", groupState) &&
			WORLD_DESTRUCTION_STATE::BREAKING == groupState.eState &&
			2u == groupState.iStateVersion && 10u == groupState.iStateStartTick &&
			13u == groupState.iCommitTick &&
			groupState.strPendingMutationId ==
				"mutation.valtan.wall.3705102.fracture",
			"Persist the BREAKING version and exact final commit tick");
		tests.Require(
			WORLD_DESTRUCTION_PREPARE_RESULT::DUPLICATE_REQUEST ==
				runtime.Prepare_ImpactTrigger(
					exactAction, "receiver.valtan.wall.3705102",
					SOURCE_BOSS_ENTITY_ID, 7u, 11u, transaction, status) &&
			transaction.Transitions.empty(),
			"Treat the same pattern-sequence binding request as an idempotent no-op");
		tests.Require(
			WORLD_DESTRUCTION_PREPARE_RESULT::NO_CHANGE ==
				runtime.Prepare_DueStateCommits(12u, transaction, status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				runtime.Prepare_DueStateCommits(13u, transaction, status) &&
			WORLD_DESTRUCTION_STATE::FRACTURED ==
				transaction.Transitions.front().eNextState &&
			transaction.Transitions.front().bApplyPersistentMutation &&
			runtime.Commit(transaction, status),
			"Commit the persistent wall, collision, and navigation plan at the exact tick");
		tests.Require(
			runtime.Find_GroupState(
				"destroyable.group.valtan.wall.3705102", groupState) &&
			WORLD_DESTRUCTION_STATE::FRACTURED == groupState.eState &&
			3u == groupState.iStateVersion && 13u == groupState.iStateStartTick &&
			0u == groupState.iCommitTick,
			"Converge on a persistent FRACTURED state with one final version");

		const std::uint32_t previousEpoch = runtime.Get_EncounterEpoch();
		tests.Require(
			runtime.Reset(status) &&
			previousEpoch + 1u == runtime.Get_EncounterEpoch() &&
			runtime.Find_GroupState(
				"destroyable.group.valtan.wall.3705102", groupState) &&
			WORLD_DESTRUCTION_STATE::INTACT == groupState.eState &&
			1u == groupState.iStateVersion && 1u == groupState.iStateStartTick &&
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				runtime.Prepare_ImpactTrigger(
					exactAction, "receiver.valtan.wall.3705102",
					SOURCE_BOSS_ENTITY_ID, 7u, 20u, transaction, status),
			"Reset to a new epoch/version baseline and admit the same sequence in the new encounter");
		tests.Require(
			1u == runtime.Get_GroupStates().size() &&
			"destroyable.group.valtan.wall.3705102" ==
				runtime.Get_GroupStates().front().strGroupId,
			"Enumerate persistent group state in canonical stable-ID order");

		CWorldDestructionRuntime zeroDurationRuntime;
		tests.Require(
			zeroDurationRuntime.Initialize(makeDestructionGraph(
				0u, WORLD_DESTRUCTION_TRIGGER_KIND::STAGE, false), status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				zeroDurationRuntime.Prepare_StageTrigger(
					exactAction, SOURCE_BOSS_ENTITY_ID, 1u, 30u,
					transaction, status) &&
			WORLD_DESTRUCTION_STATE::FRACTURED ==
				transaction.Transitions.front().eNextState &&
			30u == transaction.Transitions.front().iCommitTick &&
			transaction.Transitions.front().strCollisionStateId.empty() &&
			transaction.Transitions.front().strNavigationStateId.empty() &&
			transaction.Transitions.front().bApplyPersistentMutation &&
			zeroDurationRuntime.Commit(transaction, status) &&
			zeroDurationRuntime.Find_GroupState(
				"destroyable.group.valtan.wall.3705102", groupState) &&
			WORLD_DESTRUCTION_STATE::FRACTURED == groupState.eState &&
			2u == groupState.iStateVersion,
			"Commit a zero-duration stage binding directly in one version");

		CWorldDestructionRuntime wrapRuntime;
		const std::uint32_t beforeWrap =
			(std::numeric_limits<std::uint32_t>::max)() - 1u;
		tests.Require(
			wrapRuntime.Initialize(makeDestructionGraph(
				3u, WORLD_DESTRUCTION_TRIGGER_KIND::BOSS_IMPACT), status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				wrapRuntime.Prepare_ImpactTrigger(
					exactAction, "receiver.valtan.wall.3705102",
					SOURCE_BOSS_ENTITY_ID, 1u, beforeWrap,
					transaction, status) &&
			2u == transaction.Transitions.front().iCommitTick &&
			wrapRuntime.Commit(transaction, status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::NO_CHANGE ==
				wrapRuntime.Prepare_DueStateCommits(
					(std::numeric_limits<std::uint32_t>::max)(),
					transaction, status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::NO_CHANGE ==
				wrapRuntime.Prepare_DueStateCommits(1u, transaction, status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				wrapRuntime.Prepare_DueStateCommits(2u, transaction, status) &&
			wrapRuntime.Commit(transaction, status),
			"Skip reserved tick zero and commit exactly after uint32 wrap");
	}

	{
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		SERVER_WORLD_ENTITY boss{};
		boss.iNetEntityId = 7001u;
		boss.strPatternId = "VALTAN_ARENA_BREAK_109";
		boss.strPatternStageId = "IMPACT";
		boss.strActionId =
			"valtan.mechanic.arena-break-109.impact";
		boss.iPatternStageIndex = 2u;
		boss.iPatternSequence = 15u;
		boss.fPositionX = 151.25f;
		boss.fPositionY = 22.97f;
		boss.fPositionZ = -121.75f;
		boss.fYawDegrees = 90.f;

		const bool applied = room.Is_Ready() &&
			room.Apply_WorldDestructionStageEntry(boss, 450u);
		const auto breakingStates =
			room.m_WorldDestructionRuntime.Get_GroupStates();
		const std::size_t breakingCount = static_cast<std::size_t>(
			std::count_if(breakingStates.begin(), breakingStates.end(),
				[](const WORLD_DESTRUCTION_GROUP_STATE& state)
				{
					return WORLD_DESTRUCTION_STATE::BREAKING == state.eState;
				}));
		/* The 109 batch is the thirty outer ring walls plus every interior
		wall still standing, so the cutscene leaves no wall behind. A floor
		sector or an entrance wall leaving INTACT on this edge would take the
		arena's footing away a whole health-bar chain too early. */
		const std::size_t interiorBreakingCount = static_cast<std::size_t>(
			std::count_if(breakingStates.begin(), breakingStates.end(),
				[](const WORLD_DESTRUCTION_GROUP_STATE& state)
				{
					return WORLD_DESTRUCTION_STATE::INTACT != state.eState &&
						(0u == state.strGroupId.rfind(
							"destroyable.group.valtan.wall159.", 0u) ||
						0u == state.strGroupId.rfind(
							"destroyable.group.valtan.wall.", 0u));
				}));
		const std::size_t outsideBreakingCount = static_cast<std::size_t>(
			std::count_if(breakingStates.begin(), breakingStates.end(),
				[](const WORLD_DESTRUCTION_GROUP_STATE& state)
				{
					return WORLD_DESTRUCTION_STATE::INTACT != state.eState &&
						0u != state.strGroupId.rfind(
							"destroyable.group.valtan.outerwall109.", 0u) &&
						0u != state.strGroupId.rfind(
							"destroyable.group.valtan.wall159.", 0u) &&
						0u != state.strGroupId.rfind(
							"destroyable.group.valtan.wall.", 0u);
				}));
		tests.Require(
			applied && 97u == breakingCount && 67u == interiorBreakingCount &&
			0u == outsideBreakingCount &&
			98u == room.m_iNextWorldDestructionEventSequence,
			"Emit one monotonically sequenced live event for every 109-bar wall");

		const std::uint64_t sequenceAfterFirstEdge =
			room.m_iNextWorldDestructionEventSequence;
		tests.Require(
			room.Apply_WorldDestructionStageEntry(boss, 451u) &&
			sequenceAfterFirstEdge ==
				room.m_iNextWorldDestructionEventSequence,
			"Do not allocate a live event for a duplicate pattern-stage edge");

		WORLD_DESTRUCTION_ACTION_TUPLE action{};
		action.strPatternId = boss.strPatternId;
		action.strStageId = boss.strPatternStageId;
		action.strActionId = boss.strActionId;
		action.iStageIndex = boss.iPatternStageIndex;
		CWorldDestructionRuntime isolatedRuntime;
		std::string status;
		WORLD_DESTRUCTION_TRANSACTION transaction{};
		const bool prepared = isolatedRuntime.Initialize(
			room.m_WorldDestructionBootstrap.Get_DescriptorGraph(), status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				isolatedRuntime.Prepare_StageTrigger(
					action, boss.iNetEntityId, boss.iPatternSequence,
					450u, transaction, status);
		CWorldDestructionRuntime activeRuntime =
			std::move(room.m_WorldDestructionRuntime);
		room.m_WorldDestructionRuntime = std::move(isolatedRuntime);
		room.m_iNextWorldDestructionEventSequence = 1u;
		std::vector<WORLD_DESTRUCTION_EVENT_WIRE> firstEvents;
		std::vector<WORLD_DESTRUCTION_EVENT_WIRE> repeatedEvents;
		const bool builtFirst = prepared &&
			room.Build_WorldDestructionLiveEvents(
				transaction, boss, firstEvents, status);
		const bool builtRepeated =
			room.Build_WorldDestructionLiveEvents(
				transaction, boss, repeatedEvents, status);
		room.m_WorldDestructionRuntime = std::move(activeRuntime);
		tests.Require(
			builtFirst && builtRepeated && 97u == firstEvents.size() &&
			firstEvents.size() == repeatedEvents.size() &&
			1u == firstEvents.front().iEventSequence &&
			97u == firstEvents.back().iEventSequence &&
			firstEvents.front().iRandomSeed ==
				repeatedEvents.front().iRandomSeed &&
			firstEvents.front().fImpactOriginX == boss.fPositionX &&
			firstEvents.front().fImpactOriginY == boss.fPositionY &&
			firstEvents.front().fImpactOriginZ == boss.fPositionZ &&
			std::fabs(firstEvents.front().fImpactDirectionX - 1.f) <= 0.001f &&
			std::fabs(firstEvents.front().fImpactDirectionY) <= 0.001f &&
			std::fabs(firstEvents.front().fImpactDirectionZ) <= 0.001f,
			"Build canonical deterministic events from the authoritative boss pose");

		/* One sequence short of what this batch needs, derived from the batch
		itself so the ledger guard stays covered if the ring ever changes size. */
		room.m_iNextWorldDestructionEventSequence =
			(std::numeric_limits<std::uint64_t>::max)() -
			(static_cast<std::uint64_t>(transaction.Transitions.size()) - 2u);
		std::vector<WORLD_DESTRUCTION_EVENT_WIRE> exhaustedEvents;
		CWorldDestructionRuntime exhaustionRuntime;
		const bool preparedExhaustion = exhaustionRuntime.Initialize(
			room.m_WorldDestructionBootstrap.Get_DescriptorGraph(), status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				exhaustionRuntime.Prepare_StageTrigger(
					action, boss.iNetEntityId, boss.iPatternSequence,
					450u, transaction, status);
		activeRuntime = std::move(room.m_WorldDestructionRuntime);
		room.m_WorldDestructionRuntime = std::move(exhaustionRuntime);
		tests.Require(
			preparedExhaustion &&
			!room.Build_WorldDestructionLiveEvents(
				transaction, boss, exhaustedEvents, status) &&
			exhaustedEvents.empty(),
			"Fail closed before a destruction live-event sequence can wrap");
		room.m_WorldDestructionRuntime = std::move(activeRuntime);

		room.m_iNextWorldDestructionEventSequence = sequenceAfterFirstEdge;
		tests.Require(
			room.Commit_DueWorldDestruction(458u) &&
			sequenceAfterFirstEdge ==
				room.m_iNextWorldDestructionEventSequence,
			"Commit due FRACTURED states without emitting a second live event");
		const std::uint32_t previousEpoch =
			room.m_WorldDestructionRuntime.Get_EncounterEpoch();
		tests.Require(
			room.Reset_ValtanArenaWhenEmpty() &&
			previousEpoch + 1u ==
				room.m_WorldDestructionRuntime.Get_EncounterEpoch() &&
			1u == room.m_iNextWorldDestructionEventSequence,
			"Reset the room live-event ledger only with the encounter epoch");
	}

	{
		/* The wall sweep does not know where the ground stops. One charge stride
		out of the arena floor would otherwise leave the boss standing inside a wall's own
		navigation blocker, which nothing can then path out of without being
		projected first. */
		CGameRoom navigableStepRoom{ WORLD_ID::VALTAN_ARENA };
		constexpr float STEP_FROM_Z = -120.2f;
		constexpr float STEP_TARGET_Z = -117.9778f;
		float reachedX = 0.f;
		float reachedZ = 0.f;
		CGameRoom::Resolve_NavigableStep(
			navigableStepRoom.m_ServerNavigation,
			156.03f, STEP_FROM_Z, 156.03f, STEP_TARGET_Z, reachedX, reachedZ);
		/* A start the grid already refuses is passed through untouched, because
		refusing it there would strand a boss that is somehow off the floor. */
		float strandedX = 0.f;
		float strandedZ = 0.f;
		CGameRoom::Resolve_NavigableStep(
			navigableStepRoom.m_ServerNavigation,
			156.03f, STEP_TARGET_Z, 156.03f, STEP_FROM_Z, strandedX, strandedZ);
		tests.Require(
			navigableStepRoom.Is_Ready() &&
			navigableStepRoom.m_ServerNavigation.Is_PointWalkableExact(
				156.03f, STEP_FROM_Z) &&
			!navigableStepRoom.m_ServerNavigation.Is_PointWalkableExact(
				156.03f, STEP_TARGET_Z) &&
			navigableStepRoom.m_ServerNavigation.Is_PointWalkableExact(
				reachedX, reachedZ) &&
			reachedZ > STEP_FROM_Z && reachedZ < STEP_TARGET_Z &&
			156.03f == strandedX && STEP_FROM_Z == strandedZ,
			"Stop a boss pattern stride against the ground instead of inside a wall");
	}

	{
		/* Valtan walks its own animation the way a player skill does. A stage
		whose clip bakes travel carries that curve, and the stride is the
		difference between the curve at two ticks, so the transform arrives with
		the pose instead of sliding out from under it. A stage that authored its
		own distance keeps the constant slide, because those two carry the boss
		far past anything the bound clip animates. */
		const auto* rootMotionPatterns =
			catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const BOSS_PATTERN_DEFINITION* rushPattern = nullptr;
		const BOSS_PATTERN_DEFINITION* chargePattern = nullptr;
		if (nullptr != rootMotionPatterns)
		{
			for (const BOSS_PATTERN_DEFINITION& pattern : *rootMotionPatterns)
			{
				if ("VALTAN_PORTAL_RUSH" == pattern.strPatternId)
					rushPattern = &pattern;
				else if ("VALTAN_DASH_CHARGE" == pattern.strPatternId)
					chargePattern = &pattern;
			}
		}
		constexpr std::size_t RUSH_STAGE_INDEX = 1u;
		constexpr std::size_t CHARGE_STAGE_INDEX = 1u;
		const bool hasRushCurve =
			nullptr != rushPattern &&
			RUSH_STAGE_INDEX < rushPattern->Stages.size() &&
			!rushPattern->Stages[RUSH_STAGE_INDEX].Motion.RootMotion.empty();
		/* The authored charge is deliberately absent from the curve document. */
		const bool chargeKeepsAuthoredSlide =
			nullptr != chargePattern &&
			CHARGE_STAGE_INDEX < chargePattern->Stages.size() &&
			chargePattern->Stages[CHARGE_STAGE_INDEX].Motion.RootMotion.empty() &&
			BOSS_PATTERN_STAGE_MOTION_KIND::FORWARD ==
				chargePattern->Stages[CHARGE_STAGE_INDEX].Motion.eKind;

		CValtanBrain rootMotionBrain;
		constexpr float FIXED_DELTA_SECONDS = 1.f / 30.f;
		SERVER_WORLD_ENTITY curveBoss{};
		curveBoss.fYawDegrees = 0.f;
		curveBoss.ePatternStageMotionKind =
			BOSS_PATTERN_STAGE_MOTION_KIND::NONE;
		if (hasRushCurve)
		{
			curveBoss.PatternStageRootMotion =
				rushPattern->Stages[RUSH_STAGE_INDEX].Motion.RootMotion;
		}
		/* Walking the whole stage on the curve has to land on the travel the
		clip baked, and never on a constant slide the stage never declared. */
		float travelled = 0.f;
		bool everyStepFinite = true;
		for (std::uint32_t step = 0u; step < 60u; ++step)
		{
			curveBoss.fActionElapsedSeconds =
				static_cast<float>(step + 1u) * FIXED_DELTA_SECONDS;
			float proposedX = 0.f;
			float proposedZ = 0.f;
			if (!rootMotionBrain.Try_BuildStageMotion(
				curveBoss, FIXED_DELTA_SECONDS, proposedX, proposedZ))
			{
				continue;
			}
			if (!std::isfinite(proposedX) || !std::isfinite(proposedZ))
			{
				everyStepFinite = false;
				break;
			}
			travelled += proposedZ - curveBoss.fPositionZ;
			curveBoss.fPositionX = proposedX;
			curveBoss.fPositionZ = proposedZ;
		}
		const float bakedTravel = hasRushCurve ?
			rushPattern->Stages[RUSH_STAGE_INDEX].Motion.RootMotion.back().fForward :
			0.f;

		/* A stage with neither a curve nor an authored distance never proposes a
		step, so ordinary footwork cannot be mistaken for a charge. */
		SERVER_WORLD_ENTITY stillBoss{};
		stillBoss.fActionElapsedSeconds = 0.5f;
		float stillX = 0.f;
		float stillZ = 0.f;
		const bool stillHolds = !rootMotionBrain.Try_BuildStageMotion(
			stillBoss, FIXED_DELTA_SECONDS, stillX, stillZ);

		tests.Require(
			hasRushCurve && chargeKeepsAuthoredSlide && everyStepFinite &&
			stillHolds && bakedTravel > 1.f &&
			std::abs(travelled - bakedTravel) < 0.01f,
			"Walk a Valtan stage along the travel its clip baked and leave the "
			"authored charge on its own distance");
	}

	{
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		SERVER_WORLD_ENTITY boss{};
		boss.iNetEntityId = 7002u;
		boss.strPatternId = "VALTAN_ARMOR_BREAK_OPENING";
		boss.strPatternStageId = "WALL_CHARGE";
		boss.strActionId =
			"valtan.mechanic.armor-break-opening.charge";
		boss.iPatternStageIndex = 0u;
		boss.iPatternSequence = 159u;
		boss.fPositionX = 151.f;
		boss.fPositionY = 23.04f;
		boss.fPositionZ = -133.312236f;
		boss.fYawDegrees = 90.f;
		bool triggered = false;
		WORLD_DESTRUCTION_GROUP_STATE groupState{};
		std::vector<SERVER_NAV_POINT> wallPassagePath;
		tests.Require(
			room.Is_Ready() &&
			!room.m_ServerNavigation.Is_PointWalkableExact(
				161.402061f, -133.312236f) &&
			!room.m_ServerNavigation.Find_Path(
				160.25f, -130.75f, 162.25f, -135.75f,
				wallPassagePath) &&
			room.Apply_WorldDestructionImpact(
				boss, "collision.valtan.wallgroup.11047903315509031966.15719065619666776634.receiver",
				500u, triggered) && triggered &&
			room.m_WorldDestructionRuntime.Find_GroupState(
				"destroyable.group.valtan.wall159.15719065619666776634",
				groupState) &&
			WORLD_DESTRUCTION_STATE::BREAKING == groupState.eState &&
			!room.m_ServerNavigation.Is_PointWalkableExact(
				161.402061f, -133.312236f),
			"Commit one exact Valtan impact while keeping BREAKING navigation blocked");
		SERVER_BOSS_RECEIVER_HIT duplicateHit{};
		tests.Require(
			(!room.m_ServerCollisionSystem.Sweep_BossCircleAgainstReceivers(
				145.f, 23.04f, -133.312236f,
				175.f, 23.04f, -133.312236f,
				1.f, duplicateHit) ||
			 duplicateHit.strReceiverPlacementId !=
				"collision.valtan.wallgroup.11047903315509031966.15719065619666776634.receiver") &&
			!room.m_ServerCollisionSystem.Is_ImpactReceiverEnabled(
				"collision.valtan.wallgroup.11047903315509031966.15719065619666776634.receiver") &&
			room.Commit_DueWorldDestruction(507u) &&
			!room.m_ServerNavigation.Is_PointWalkableExact(
				161.402061f, -133.312236f),
			"Suppress only the struck receiver and keep its wall closed before the due tick");
		tests.Require(
			room.Commit_DueWorldDestruction(508u) &&
			room.m_WorldDestructionRuntime.Find_GroupState(
				"destroyable.group.valtan.wall159.15719065619666776634",
				groupState) &&
			WORLD_DESTRUCTION_STATE::DESPAWNED == groupState.eState &&
			room.m_ServerNavigation.Is_PointWalkableExact(
				161.402061f, -133.312236f) &&
			!room.m_ServerNavigation.Find_Path(
				160.25f, -130.75f, 162.25f, -135.75f,
				wallPassagePath) &&
			!room.m_ServerCollisionSystem.Is_PlayerBlocking(
				"collision.valtan.wallgroup.11047903315509031966.15719065619666776634"),
			"Atomically open only the struck wall cells and collision at the DESPAWNED due tick");
	}

	{
		/* Exercise the room's real fixed-tick ordering. The charge body first
		moves to the swept contact margin, then commits the exact destruction
		binding, and only that successful binding may publish WALL_CONTACT. */
		const auto stageDashCharge = [](
			CGameRoom& room,
			const float startX,
			const float startY,
			const float startZ,
			const float yawDegrees,
			const std::uint32_t startTick,
			const std::uint32_t patternSequence) -> SERVER_WORLD_ENTITY&
		{
			const BOSS_RUNTIME_PROFILE* profile =
				room.m_GameplayCatalog.Find_Boss("BOSS_VALTAN");
			room.m_WorldEntities.clear();
			room.m_Players.clear();
			SERVER_PLAYER target{};
			target.iPlayerId = 99001u;
			target.iNetEntityId = 99002u;
			target.iCurrentHp = 1000u;
			target.iMaximumHp = 1000u;
			target.isCombatReady = true;
			const float yawRadians = yawDegrees *
				3.14159265358979323846f / 180.f;
			target.fPositionX = startX + std::sin(yawRadians) * 2.f;
			target.fPositionY = startY;
			target.fPositionZ = startZ + std::cos(yawRadians) * 2.f;
			room.m_Players.emplace(target.iPlayerId, target);

			SERVER_WORLD_ENTITY boss{};
			boss.iNetEntityId = 99003u;
			boss.strPlacementId = "boss.valtan.dash-contract";
			boss.strArchetypeId = "BOSS_VALTAN";
			boss.strEncounterId = "ENCOUNTER_VALTAN";
			boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
			boss.eAction = SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
			boss.fPositionX = startX;
			boss.fPositionY = startY;
			boss.fPositionZ = startZ;
			boss.fSpawnPositionX = startX;
			boss.fSpawnPositionY = startY;
			boss.fSpawnPositionZ = startZ;
			boss.fYawDegrees = yawDegrees;
			boss.iCurrentHp = nullptr == profile ? 60000u : profile->iMaximumHp;
			boss.iMaximumHp = boss.iCurrentHp;
			boss.iMaximumHealthBars =
				nullptr == profile ? 160u : profile->iMaximumHealthBars;
			boss.iLastEvaluatedHealthBar = boss.iMaximumHealthBars;
			boss.iLastHealthMechanicGenerationEpoch =
				room.m_GameplayCatalog.Get_ActiveGenerationEpoch();
			boss.iPhase = 1u;
			boss.PhasePolicy = nullptr == profile ?
				BOSS_PHASE_POLICY{} : profile->PhasePolicy;
			boss.fCollisionRadius =
				nullptr == profile ? 3.f : profile->fCollisionRadius;
			boss.fEngageDistance =
				nullptr == profile ? 20.f : profile->fEngageDistance;
			boss.fMoveSpeed = nullptr == profile ? 2.6f : profile->fMoveSpeed;
			boss.bIntroPatternConsumed = true;
			boss.strPatternId = "VALTAN_DASH_CHARGE";
			boss.strPatternStageId = "CHARGE";
			boss.strActionId = "valtan.attack.dash-charge.active";
			boss.iPatternSequence = patternSequence;
			boss.iPatternStageIndex = 1u;
			boss.iPatternStageDurationMs = 1500u;
			boss.iPatternStageFirstEvaluationTick = startTick;
			boss.ePatternStageMotionKind =
				BOSS_PATTERN_STAGE_MOTION_KIND::FORWARD;
			boss.fPatternForcedMotionSpeed = 20.f / 1.5f;
			boss.fPatternMinimumRange = 5.f;
			boss.fPatternMaximumRange = 20.f;
			boss.bPatternChargeImpact = true;
			boss.iActionStartTick = startTick;
			boss.iPatternTargetEntityId = target.iNetEntityId;
			boss.bHasPatternTargetLastPosition = true;
			boss.fPatternTargetLastPositionX = target.fPositionX;
			boss.fPatternTargetLastPositionY = target.fPositionY;
			boss.fPatternTargetLastPositionZ = target.fPositionZ;
			boss.PinnedDefinitionRevision =
				room.m_GameplayCatalog.Get_ActiveRevision();
			boss.BossCombat.iStateRevision = 1u;
			room.m_WorldEntities.push_back(std::move(boss));
			room.m_iServerTick = startTick;
			return room.m_WorldEntities.back();
		};

		CGameRoom ordinaryWallRoom{ WORLD_ID::VALTAN_ARENA };
		constexpr float FRESH_CENTER_X = 156.03f;
		constexpr float FRESH_CENTER_Y = 22.99751f;
		constexpr float FRESH_CENTER_Z = -122.06f;
		constexpr float FRESH_DASH_DISTANCE = 20.f;
		SERVER_BOSS_WALL_HIT ordinaryWallContact{};
		const bool foundOrdinaryWall = ordinaryWallRoom.Is_Ready() &&
			ordinaryWallRoom.m_ServerCollisionSystem.
				Sweep_BossCircleAgainstWalls(
					FRESH_CENTER_X, FRESH_CENTER_Y, FRESH_CENTER_Z,
					FRESH_CENTER_X, FRESH_CENTER_Y,
					FRESH_CENTER_Z + FRESH_DASH_DISTANCE,
					3.f, ordinaryWallContact) &&
			!ordinaryWallContact.strCollisionPlacementId.empty() &&
			!ordinaryWallContact.strCollisionPlacementId.ends_with(".receiver");
		const float ordinaryWallContactDistance =
			FRESH_DASH_DISTANCE * ordinaryWallContact.fHitRatio;
		SERVER_WORLD_ENTITY& ordinaryWallBoss = stageDashCharge(
			ordinaryWallRoom, FRESH_CENTER_X, FRESH_CENTER_Y,
			FRESH_CENTER_Z, 0.f, 450u, 650u);
		(void)ordinaryWallBoss;
		bool ordinaryWallSameTick = false;
		for (std::uint32_t tick = 0u;
			tick < 45u && ordinaryWallRoom.m_isReady &&
			!ordinaryWallSameTick; ++tick)
		{
			const std::uint32_t updateTick =
				(std::numeric_limits<std::uint32_t>::max)() ==
					ordinaryWallRoom.m_iServerTick ?
				1u : ordinaryWallRoom.m_iServerTick + 1u;
			ordinaryWallRoom.Update_WorldEntities(1.f / 30.f);
			ordinaryWallRoom.m_iServerTick = updateTick;
			const SERVER_WORLD_ENTITY& liveBoss =
				ordinaryWallRoom.m_WorldEntities.front();
			const float travelledDistance =
				liveBoss.fPositionZ - FRESH_CENTER_Z;
			const auto states =
				ordinaryWallRoom.m_WorldDestructionRuntime.Get_GroupStates();
			const std::size_t breakingCount =
				static_cast<std::size_t>(std::count_if(
					states.begin(), states.end(),
					[](const WORLD_DESTRUCTION_GROUP_STATE& state)
					{
						return WORLD_DESTRUCTION_STATE::BREAKING == state.eState;
					}));
			ordinaryWallSameTick = 1u == breakingCount &&
				travelledDistance > 0.f &&
				travelledDistance < ordinaryWallContactDistance &&
				"VALTAN_DASH_CHARGE" == liveBoss.strPatternId &&
				"GROGGY" == liveBoss.strPatternStageId &&
				"valtan.attack.dash-charge.groggy" == liveBoss.strActionId &&
				2u == liveBoss.iPatternStageIndex &&
				updateTick == liveBoss.iActionStartTick &&
				liveBoss.bPatternGroggy && !liveBoss.bPatternChargeImpact &&
				0.f == liveBoss.fPatternForcedMotionSpeed &&
				CBossCombatRuntime::Has_Flag(
					liveBoss.BossCombat,
					SERVER_BOSS_COMBAT_FLAG::GROGGY);
		}
		tests.Require(
			foundOrdinaryWall && ordinaryWallContactDistance > 0.f &&
			ordinaryWallContactDistance < FRESH_DASH_DISTANCE,
			"Resolve a fresh centre-origin Dash to the first ordinary collision wall before the outer receivers");
		tests.Require(
			ordinaryWallRoom.m_isReady && ordinaryWallSameTick,
			"Break the first ordinary wall, stop the Dash, and enter GROGGY on that fixed tick");

		CGameRoom boundRoom{ WORLD_ID::VALTAN_ARENA };
		/* The 109 transition ring's impact receivers can precede the ten 159
		   charge walls. Disable only those receivers here so this fixture isolates
		   the exact Dash -> 159 mutation join; the separate outer-bound fixture
		   below keeps the ring enabled and proves its real first-contact join. */
		std::vector<SERVER_COLLISION_STATE_CHANGE> outerReceiverChanges;
		for (const WORLD_BOOTSTRAP_PLACEMENT& placement :
			boundRoom.m_WorldBootstrap.Get_Placements())
		{
			if (0u == placement.strPlacementId.rfind(
					"collision.valtan.wallgroup.sector", 0u) &&
				placement.strPlacementId.ends_with(".receiver"))
			{
				outerReceiverChanges.push_back(
					{ placement.strPlacementId, true, false });
			}
		}
		SERVER_COLLISION_STATE_STAGE isolatedReceiverStage{};
		std::string isolatedReceiverStatus;
		const bool isolatedBoundReceiver = 30u == outerReceiverChanges.size() &&
			boundRoom.m_ServerCollisionSystem.Prepare_StateChanges(
				outerReceiverChanges, isolatedReceiverStage,
				isolatedReceiverStatus);
		if (isolatedBoundReceiver)
			boundRoom.m_ServerCollisionSystem.Commit_StateChanges(
				std::move(isolatedReceiverStage));
		SERVER_BOSS_RECEIVER_HIT boundContact{};
		const bool foundBoundContact = boundRoom.Is_Ready() &&
			isolatedBoundReceiver &&
			boundRoom.m_ServerCollisionSystem.Sweep_BossCircleAgainstReceivers(
				145.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
				175.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
			3.f, boundContact) &&
			0u == boundContact.strReceiverPlacementId.rfind(
				"collision.valtan.wallgroup.11047903315509031966.", 0u);
		const std::size_t boundReceiverSuffix =
			boundContact.strReceiverPlacementId.rfind(".receiver");
		const std::string boundCollisionId =
			std::string::npos == boundReceiverSuffix ? std::string{} :
			boundContact.strReceiverPlacementId.substr(0u, boundReceiverSuffix);
		std::vector<SERVER_COLLISION_STATE_CHANGE> otherWallChanges;
		for (const WORLD_BOOTSTRAP_PLACEMENT& placement :
			boundRoom.m_WorldBootstrap.Get_Placements())
		{
			if (WORLD_BOOTSTRAP_KIND::COLLISION_BOX != placement.eKind ||
				placement.strPlacementId.ends_with(".receiver") ||
				placement.strPlacementId == boundCollisionId)
			{
				continue;
			}
			otherWallChanges.push_back(
				{ placement.strPlacementId, false, false });
		}
		SERVER_COLLISION_STATE_STAGE isolatedWallStage{};
		std::string isolatedWallStatus;
		const bool isolatedBoundWall = foundBoundContact &&
			98u == otherWallChanges.size() &&
			boundRoom.m_ServerCollisionSystem.Prepare_StateChanges(
				otherWallChanges, isolatedWallStage, isolatedWallStatus);
		if (isolatedBoundWall)
			boundRoom.m_ServerCollisionSystem.Commit_StateChanges(
				std::move(isolatedWallStage));
		const float boundContactX =
			145.f + 30.f * boundContact.fHitRatio;
		SERVER_WORLD_ENTITY& boundBoss = stageDashCharge(
			boundRoom, boundContactX - 0.2f, VALTAN_WALL_CENTER_Y,
			VALTAN_WALL_CENTER_Z, 90.f, 500u, 701u);
		const float boundStartX = boundBoss.fPositionX;
		const SERVER_WORLD_ENTITY consumedCharge = boundBoss;
		const std::uint32_t boundContactUpdateTick =
			boundRoom.m_iServerTick + 1u;
		boundRoom.Update_WorldEntities(1.f / 30.f);
		const auto boundStates =
			boundRoom.m_WorldDestructionRuntime.Get_GroupStates();
		const std::size_t breakingCount = static_cast<std::size_t>(std::count_if(
			boundStates.begin(), boundStates.end(),
			[](const WORLD_DESTRUCTION_GROUP_STATE& state)
			{
				return WORLD_DESTRUCTION_STATE::BREAKING == state.eState;
			}));
		const SERVER_WORLD_ENTITY& groggyBoss = boundRoom.m_WorldEntities.front();
		tests.Require(
			foundBoundContact && isolatedBoundWall &&
			boundRoom.m_isReady && 1u == breakingCount &&
			groggyBoss.fPositionX >= boundStartX &&
			groggyBoss.fPositionX < boundContactX &&
			"VALTAN_DASH_CHARGE" == groggyBoss.strPatternId &&
			"GROGGY" == groggyBoss.strPatternStageId &&
			"valtan.attack.dash-charge.groggy" == groggyBoss.strActionId &&
			2u == groggyBoss.iPatternStageIndex &&
			SERVER_ENTITY_ACTION::PATTERN_ACTIVE == groggyBoss.eAction &&
			boundContactUpdateTick == groggyBoss.iActionStartTick &&
			groggyBoss.bPatternGroggy && !groggyBoss.bPatternChargeImpact &&
			CBossCombatRuntime::Has_Flag(
				groggyBoss.BossCombat, SERVER_BOSS_COMBAT_FLAG::GROGGY),
			"Commit BREAKING and replace the charge action with GROGGY on the exact swept collision tick");

		SERVER_WORLD_ENTITY consumedProbe = consumedCharge;
		consumedProbe.iNetEntityId = 99004u;
		consumedProbe.iPatternSequence = 702u;
		bool consumedTriggered = true;
		const bool consumedAccepted = boundRoom.Apply_WorldDestructionImpact(
			consumedProbe, boundContact.strReceiverPlacementId,
			502u, consumedTriggered);
		tests.Require(
			consumedAccepted && !consumedTriggered &&
			"CHARGE" == consumedProbe.strPatternStageId &&
			!consumedProbe.bPatternGroggy,
			"Refuse a consumed dash receiver without manufacturing GROGGY");

	#ifdef _DEBUG
		CGameRoom outerRoom{ WORLD_ID::VALTAN_ARENA };
		constexpr float ARENA_CENTER_X = 156.03f;
		constexpr float ARENA_CENTER_Y = 22.99751f;
		constexpr float ARENA_CENTER_Z = -122.06f;
		constexpr float DASH_DISTANCE = 20.f;
		SERVER_WORLD_ENTITY arenaStateOwner{};
		arenaStateOwner.iNetEntityId = 99005u;
		arenaStateOwner.iPatternSequence = 800u;
		WORLD_DESTRUCTION_TRANSACTION ordinaryWallsGone{};
		std::vector<std::string> expectedOrdinaryGoneGroupIds;
		std::string ordinaryArenaStatus;
		const bool preparedOrdinaryArena = outerRoom.Is_Ready() &&
			outerRoom.Prepare_ValtanTimelineArenaState(
				outerRoom.m_WorldDestructionRuntime, arenaStateOwner,
				VALTAN_TIMELINE_ARENA_STATE::ORDINARY_WALLS_GONE, 580u,
				ordinaryWallsGone, expectedOrdinaryGoneGroupIds,
				ordinaryArenaStatus);
		std::uint32_t ordinaryArenaCommitTick = 580u;
		for (const WORLD_DESTRUCTION_STATE_TRANSITION& transition :
			ordinaryWallsGone.Transitions)
		{
			ordinaryArenaCommitTick = (std::max)(
				ordinaryArenaCommitTick, transition.iCommitTick);
		}
		const bool committedOrdinaryArena = preparedOrdinaryArena &&
			69u == expectedOrdinaryGoneGroupIds.size() &&
			outerRoom.Commit_WorldDestructionTransaction(
				ordinaryWallsGone, {}, 580u, ordinaryArenaStatus) &&
			outerRoom.Commit_DueWorldDestruction(ordinaryArenaCommitTick);
		std::size_t ordinaryGoneCount = 0u;
		for (const std::string& groupId : expectedOrdinaryGoneGroupIds)
		{
			WORLD_DESTRUCTION_GROUP_STATE state{};
			if (outerRoom.m_WorldDestructionRuntime.Find_GroupState(
					groupId, state) &&
				WORLD_DESTRUCTION_STATE::DESPAWNED == state.eState)
			{
				++ordinaryGoneCount;
			}
		}
		const auto stagedOuterStates =
			outerRoom.m_WorldDestructionRuntime.Get_GroupStates();
		const std::size_t intactOuterCount =
			static_cast<std::size_t>(std::count_if(
				stagedOuterStates.begin(), stagedOuterStates.end(),
				[](const WORLD_DESTRUCTION_GROUP_STATE& state)
				{
					return 0u == state.strGroupId.rfind(
						"destroyable.group.valtan.outerwall109.", 0u) &&
						WORLD_DESTRUCTION_STATE::INTACT == state.eState;
				}));
		const bool ordinaryArenaReady = committedOrdinaryArena &&
			69u == ordinaryGoneCount && 30u == intactOuterCount;
		tests.Require(
			ordinaryArenaReady,
			"Stage the product ORDINARY_WALLS_GONE arena state while keeping all thirty outer-ring walls intact");

		/* The product timeline removes the 69 ordinary walls before this replay;
		   the canonical centre-to-positive-Z lane then reaches the intact 109
		   transition ring as its first swept receiver contact. */
		constexpr float OUTER_CONTACT_YAW_DEGREES = 0.f;
		constexpr float outerDirectionX = 0.f;
		constexpr float outerDirectionZ = 1.f;
		constexpr float outerEndX = ARENA_CENTER_X;
		constexpr float outerEndZ = ARENA_CENTER_Z + DASH_DISTANCE;
		SERVER_BOSS_RECEIVER_HIT outerContact{};
		const bool foundOuterContact = ordinaryArenaReady &&
			outerRoom.m_ServerCollisionSystem.Sweep_BossCircleAgainstReceivers(
				ARENA_CENTER_X, ARENA_CENTER_Y, ARENA_CENTER_Z,
				outerEndX, ARENA_CENTER_Y, outerEndZ,
				3.f, outerContact) &&
			0u == outerContact.strReceiverPlacementId.rfind(
				"collision.valtan.wallgroup.sector", 0u);
		const std::size_t receiverSuffix =
			outerContact.strReceiverPlacementId.rfind(".receiver");
		const std::size_t memberSeparator =
			std::string::npos == receiverSuffix ? std::string::npos :
			outerContact.strReceiverPlacementId.rfind(
				'.', receiverSuffix - 1u);
		const std::string outerMemberId =
			std::string::npos == memberSeparator ||
			std::string::npos == receiverSuffix ?
			std::string{} : outerContact.strReceiverPlacementId.substr(
				memberSeparator + 1u,
				receiverSuffix - memberSeparator - 1u);
		const float outerContactDistance =
			DASH_DISTANCE * outerContact.fHitRatio;
		SERVER_WORLD_ENTITY& outerBoss = stageDashCharge(
			outerRoom, ARENA_CENTER_X, ARENA_CENTER_Y,
			ARENA_CENTER_Z, OUTER_CONTACT_YAW_DEGREES, 600u, 801u);
		(void)outerBoss;
		bool sameTickOuterImpact = false;
		for (std::uint32_t tick = 0u;
			tick < 45u && outerRoom.m_isReady && !sameTickOuterImpact; ++tick)
		{
			const std::uint32_t updateTick =
				(std::numeric_limits<std::uint32_t>::max)() ==
					outerRoom.m_iServerTick ?
				1u : outerRoom.m_iServerTick + 1u;
			/* Observe the fixed-tick collision transaction before Tick commits the
			   due destruction and advances BREAKING to DESPAWNED. This is the same
			   room update boundary used by the isolated 159 receiver fixture above. */
			outerRoom.Update_WorldEntities(1.f / 30.f);
			outerRoom.m_iServerTick = updateTick;
			const SERVER_WORLD_ENTITY& liveBoss =
				outerRoom.m_WorldEntities.front();
			const float travelledDistance =
				(liveBoss.fPositionX - ARENA_CENTER_X) * outerDirectionX +
				(liveBoss.fPositionZ - ARENA_CENTER_Z) * outerDirectionZ;
			const auto tickStates =
				outerRoom.m_WorldDestructionRuntime.Get_GroupStates();
			const std::size_t tickOuterBreakingCount =
				static_cast<std::size_t>(std::count_if(
					tickStates.begin(), tickStates.end(),
					[](const WORLD_DESTRUCTION_GROUP_STATE& state)
					{
						return 0u == state.strGroupId.rfind(
							"destroyable.group.valtan.outerwall109.", 0u) &&
							WORLD_DESTRUCTION_STATE::BREAKING == state.eState;
					}));
			sameTickOuterImpact = 1u == tickOuterBreakingCount &&
				travelledDistance > 0.f &&
				travelledDistance < outerContactDistance &&
				"VALTAN_DASH_CHARGE" == liveBoss.strPatternId &&
				"GROGGY" == liveBoss.strPatternStageId &&
				liveBoss.bPatternGroggy &&
				!liveBoss.bPatternChargeImpact &&
				0.f == liveBoss.fPatternForcedMotionSpeed &&
				CBossCombatRuntime::Has_Flag(
					liveBoss.BossCombat,
					SERVER_BOSS_COMBAT_FLAG::GROGGY);
		}
		const auto outerStates =
			outerRoom.m_WorldDestructionRuntime.Get_GroupStates();
		const std::size_t outerBreakingCount =
			static_cast<std::size_t>(std::count_if(
				outerStates.begin(), outerStates.end(),
				[](const WORLD_DESTRUCTION_GROUP_STATE& state)
				{
					return WORLD_DESTRUCTION_STATE::BREAKING == state.eState;
				}));
		tests.Require(
			foundOuterContact && !outerMemberId.empty(),
			"Resolve the centre-origin Dash Charge's first swept contact to one stable outer-ring receiver");
		tests.Require(
			outerRoom.m_isReady && sameTickOuterImpact,
			"Stop the centre-origin Dash Charge at its first outer-ring receiver and commit BREAKING plus GROGGY on that fixed tick");
		tests.Require(
			1u == outerBreakingCount,
			"Break exactly one outer-ring wall from the centre-origin Dash Charge");

	#endif

		/* This contract deliberately stays outside _DEBUG. First put one real
		   outer receiver into BREAKING, then expose its receiver again to emulate
		   a stale collision frame. The one-way mutation still answers NO_CHANGE,
		   but the intact collision surface is a real wall contact and must end the
		   charge in GROGGY. The same consumed group then proves the later 109 stage
		   admits only the other twenty-nine in Release as well. */
		CGameRoom partialOuterRoom{ WORLD_ID::VALTAN_ARENA };
		constexpr float PARTIAL_ARENA_CENTER_X = 156.03f;
		constexpr float PARTIAL_ARENA_CENTER_Y = 22.99751f;
		constexpr float PARTIAL_ARENA_CENTER_Z = -122.06f;
		constexpr float PARTIAL_DASH_DISTANCE = 20.f;
		SERVER_BOSS_RECEIVER_HIT partialOuterContact{};
		const bool foundPartialOuterContact = partialOuterRoom.Is_Ready() &&
			partialOuterRoom.m_ServerCollisionSystem.
				Sweep_BossCircleAgainstReceivers(
					PARTIAL_ARENA_CENTER_X, PARTIAL_ARENA_CENTER_Y,
					PARTIAL_ARENA_CENTER_Z, PARTIAL_ARENA_CENTER_X,
					PARTIAL_ARENA_CENTER_Y,
					PARTIAL_ARENA_CENTER_Z + PARTIAL_DASH_DISTANCE,
					3.f, partialOuterContact) &&
			0u == partialOuterContact.strReceiverPlacementId.rfind(
				"collision.valtan.wallgroup.sector", 0u);
		const std::size_t partialReceiverSuffix =
			partialOuterContact.strReceiverPlacementId.rfind(".receiver");
		const std::size_t partialMemberSeparator =
			std::string::npos == partialReceiverSuffix ? std::string::npos :
			partialOuterContact.strReceiverPlacementId.rfind(
				'.', partialReceiverSuffix - 1u);
		const std::string partialOuterMemberId =
			std::string::npos == partialMemberSeparator ||
			std::string::npos == partialReceiverSuffix ?
			std::string{} : partialOuterContact.strReceiverPlacementId.substr(
				partialMemberSeparator + 1u,
				partialReceiverSuffix - partialMemberSeparator - 1u);
		const std::string partialOuterGroupId = partialOuterMemberId.empty() ?
			std::string{} :
			"destroyable.group.valtan.outerwall109." + partialOuterMemberId;

		SERVER_WORLD_ENTITY partialImpactOwner{};
		partialImpactOwner.iNetEntityId = 99006u;
		partialImpactOwner.strPatternId = "VALTAN_DASH_CHARGE";
		partialImpactOwner.strPatternStageId = "CHARGE";
		partialImpactOwner.strActionId = "valtan.attack.dash-charge.active";
		partialImpactOwner.iPatternStageIndex = 1u;
		partialImpactOwner.iPatternSequence = 900u;
		partialImpactOwner.fPositionX = PARTIAL_ARENA_CENTER_X;
		partialImpactOwner.fPositionY = PARTIAL_ARENA_CENTER_Y;
		partialImpactOwner.fPositionZ = PARTIAL_ARENA_CENTER_Z;
		partialImpactOwner.fYawDegrees = 0.f;
		bool partialImpactTriggered = false;
		WORLD_DESTRUCTION_GROUP_STATE partialOuterState{};
		const bool consumedPartialOuter = foundPartialOuterContact &&
			!partialOuterGroupId.empty() &&
			partialOuterRoom.Apply_WorldDestructionImpact(
				partialImpactOwner,
				partialOuterContact.strReceiverPlacementId,
				880u, partialImpactTriggered) &&
			partialImpactTriggered &&
			partialOuterRoom.m_WorldDestructionRuntime.Find_GroupState(
				partialOuterGroupId, partialOuterState) &&
			WORLD_DESTRUCTION_STATE::BREAKING == partialOuterState.eState &&
			0u != partialOuterState.iCommitTick;

		SERVER_COLLISION_STATE_STAGE staleReceiverStage{};
		std::string staleReceiverStatus;
		std::vector<SERVER_COLLISION_STATE_CHANGE> staleReceiverChanges;
		for (const WORLD_BOOTSTRAP_PLACEMENT& placement :
			partialOuterRoom.m_WorldBootstrap.Get_Placements())
		{
			if (0u == placement.strPlacementId.rfind(
					"collision.valtan.wallgroup.sector", 0u) &&
				placement.strPlacementId.ends_with(".receiver"))
			{
				staleReceiverChanges.push_back({
					placement.strPlacementId, true,
					placement.strPlacementId ==
						partialOuterContact.strReceiverPlacementId });
			}
		}
		const bool stagedStaleReceiver = consumedPartialOuter &&
			30u == staleReceiverChanges.size() &&
			partialOuterRoom.m_ServerCollisionSystem.Prepare_StateChanges(
				staleReceiverChanges, staleReceiverStage, staleReceiverStatus);
		if (stagedStaleReceiver)
		{
			partialOuterRoom.m_ServerCollisionSystem.Commit_StateChanges(
				std::move(staleReceiverStage));
		}
		tests.Require(
			consumedPartialOuter,
			"Put one exact outer Dash receiver into BREAKING before the stale-receiver probe");
		tests.Require(
			stagedStaleReceiver &&
			partialOuterRoom.m_ServerCollisionSystem.Is_ImpactReceiverEnabled(
				partialOuterContact.strReceiverPlacementId),
			"Re-expose only the consumed outer impact receiver for the swept safe-stop probe");
		const float partialContactZ = PARTIAL_ARENA_CENTER_Z +
			PARTIAL_DASH_DISTANCE * partialOuterContact.fHitRatio;
		SERVER_WORLD_ENTITY& staleReceiverBoss = stageDashCharge(
			partialOuterRoom, PARTIAL_ARENA_CENTER_X,
			PARTIAL_ARENA_CENTER_Y, partialContactZ - 0.2f,
			0.f, 880u, 901u);
		const float staleReceiverStartZ = staleReceiverBoss.fPositionZ;
		partialOuterRoom.Update_WorldEntities(1.f / 30.f);
		const SERVER_WORLD_ENTITY& safeStoppedBoss =
			partialOuterRoom.m_WorldEntities.front();
		WORLD_DESTRUCTION_GROUP_STATE safeStoppedOuterState{};
		const bool stoppedOnConsumedOuter = stagedStaleReceiver &&
			partialOuterRoom.m_isReady &&
			partialOuterRoom.m_ServerCollisionSystem.Is_ImpactReceiverEnabled(
				partialOuterContact.strReceiverPlacementId) &&
			partialOuterRoom.m_WorldDestructionRuntime.Find_GroupState(
				partialOuterGroupId, safeStoppedOuterState) &&
			WORLD_DESTRUCTION_STATE::BREAKING == safeStoppedOuterState.eState &&
			safeStoppedBoss.fPositionZ >= staleReceiverStartZ &&
			safeStoppedBoss.fPositionZ < partialContactZ &&
			0.f == safeStoppedBoss.fPatternForcedMotionSpeed &&
			"VALTAN_DASH_CHARGE" == safeStoppedBoss.strPatternId &&
			"GROGGY" == safeStoppedBoss.strPatternStageId &&
			"valtan.attack.dash-charge.groggy" == safeStoppedBoss.strActionId &&
			2u == safeStoppedBoss.iPatternStageIndex &&
			!safeStoppedBoss.bPatternChargeImpact &&
			safeStoppedBoss.bPatternGroggy &&
			CBossCombatRuntime::Has_Flag(
				safeStoppedBoss.BossCombat,
				SERVER_BOSS_COMBAT_FLAG::GROGGY);
		tests.Require(
			partialOuterRoom.m_isReady &&
			partialOuterRoom.m_WorldDestructionRuntime.Find_GroupState(
				partialOuterGroupId, safeStoppedOuterState) &&
			WORLD_DESTRUCTION_STATE::BREAKING == safeStoppedOuterState.eState,
			"Keep the consumed outer mutation in BREAKING after its repeat swept contact");
		tests.Require(
			safeStoppedBoss.fPositionZ >= staleReceiverStartZ &&
			safeStoppedBoss.fPositionZ < partialContactZ &&
			0.f == safeStoppedBoss.fPatternForcedMotionSpeed,
			"Clamp the repeat Dash to its swept safe position and zero its forced speed");
		tests.Require(
			"GROGGY" == safeStoppedBoss.strPatternStageId &&
			!safeStoppedBoss.bPatternChargeImpact &&
			safeStoppedBoss.bPatternGroggy &&
			CBossCombatRuntime::Has_Flag(
				safeStoppedBoss.BossCombat,
				SERVER_BOSS_COMBAT_FLAG::GROGGY),
			"Enter GROGGY on a consumed receiver's still-blocking wall surface");
		tests.Require(
			stoppedOnConsumedOuter,
			"Stop at the swept safe position and enter GROGGY without repeating the consumed mutation");
		SERVER_COLLISION_STATE_STAGE normalizedOuterReceiverStage{};
		std::string normalizedOuterReceiverStatus;
		std::vector<SERVER_COLLISION_STATE_CHANGE>
			normalizedOuterReceiverChanges;
		for (const SERVER_COLLISION_STATE_CHANGE& staleChange :
			staleReceiverChanges)
		{
			normalizedOuterReceiverChanges.push_back({
				staleChange.strPlacementId, true,
				staleChange.strPlacementId !=
					partialOuterContact.strReceiverPlacementId });
		}
		const bool stagedNormalizedOuterReceivers = stoppedOnConsumedOuter &&
			30u == normalizedOuterReceiverChanges.size() &&
			partialOuterRoom.m_ServerCollisionSystem.Prepare_StateChanges(
				normalizedOuterReceiverChanges,
				normalizedOuterReceiverStage,
				normalizedOuterReceiverStatus);
		if (stagedNormalizedOuterReceivers)
		{
			partialOuterRoom.m_ServerCollisionSystem.Commit_StateChanges(
				std::move(normalizedOuterReceiverStage));
		}
		tests.Require(
			stagedNormalizedOuterReceivers &&
			!partialOuterRoom.m_ServerCollisionSystem.
				Is_ImpactReceiverEnabled(
					partialOuterContact.strReceiverPlacementId),
			"Restore the consumed outer receiver to its BREAKING collision state before the partial 109 batch");

		const std::uint32_t partialOuterCommitTick =
			consumedPartialOuter ? partialOuterState.iCommitTick : 0u;
		const bool committedPartialOuter = stagedNormalizedOuterReceivers &&
			partialOuterRoom.Commit_DueWorldDestruction(
				partialOuterCommitTick) &&
			partialOuterRoom.m_WorldDestructionRuntime.Find_GroupState(
				partialOuterGroupId, partialOuterState) &&
			WORLD_DESTRUCTION_STATE::DESPAWNED == partialOuterState.eState;
		const auto partialStatesBeforeArenaBreak =
			partialOuterRoom.m_WorldDestructionRuntime.Get_GroupStates();
		SERVER_WORLD_ENTITY partialArenaBreakBoss = safeStoppedBoss;
		partialArenaBreakBoss.strPatternId = "VALTAN_ARENA_BREAK_109";
		partialArenaBreakBoss.strPatternStageId = "IMPACT";
		partialArenaBreakBoss.strActionId =
			"valtan.mechanic.arena-break-109.impact";
		partialArenaBreakBoss.iPatternStageIndex = 2u;
		partialArenaBreakBoss.iPatternSequence = 902u;
		const std::uint32_t partialArenaBreakTick =
			partialOuterCommitTick + 1u;
		const bool appliedPartialArenaBreak = committedPartialOuter &&
			partialOuterRoom.Apply_WorldDestructionStageEntry(
				partialArenaBreakBoss, partialArenaBreakTick);
		const auto partialStatesAfterArenaBreak =
			partialOuterRoom.m_WorldDestructionRuntime.Get_GroupStates();
		std::size_t partialOuterBreakingCount = 0u;
		std::size_t partialOuterDespawnedCount = 0u;
		bool partialConsumedStayedDespawned = false;
		std::uint32_t partialArenaCommitTick = partialArenaBreakTick;
		for (const WORLD_DESTRUCTION_GROUP_STATE& state :
			partialStatesAfterArenaBreak)
		{
			if (0u != state.strGroupId.rfind(
					"destroyable.group.valtan.outerwall109.", 0u))
			{
				continue;
			}
			if (WORLD_DESTRUCTION_STATE::BREAKING == state.eState)
			{
				++partialOuterBreakingCount;
				partialArenaCommitTick = (std::max)(
					partialArenaCommitTick, state.iCommitTick);
			}
			if (WORLD_DESTRUCTION_STATE::DESPAWNED == state.eState)
				++partialOuterDespawnedCount;
			if (partialOuterGroupId == state.strGroupId)
			{
				partialConsumedStayedDespawned =
					WORLD_DESTRUCTION_STATE::DESPAWNED == state.eState;
			}
		}
		const bool partialUnrelatedGroupsUnchanged =
			partialStatesBeforeArenaBreak.size() ==
				partialStatesAfterArenaBreak.size() &&
			std::all_of(
				partialStatesAfterArenaBreak.begin(),
				partialStatesAfterArenaBreak.end(),
				[&partialStatesBeforeArenaBreak](
					const WORLD_DESTRUCTION_GROUP_STATE& after)
				{
					/* The 109 batch owns the outer ring and the interior walls, so
					only they may move here. What must survive the collapse is the
					floor, which its own 84/30 patterns drop later, and the entrance
					walls the first-appearance sweep already owns. */
					if (0u == after.strGroupId.rfind(
							"destroyable.group.valtan.outerwall109.", 0u) ||
					0u == after.strGroupId.rfind(
							"destroyable.group.valtan.wall159.", 0u) ||
					0u == after.strGroupId.rfind(
							"destroyable.group.valtan.wall.", 0u))
					{
						return true;
					}
					const auto beforeIt = std::find_if(
						partialStatesBeforeArenaBreak.begin(),
						partialStatesBeforeArenaBreak.end(),
						[&after](const WORLD_DESTRUCTION_GROUP_STATE& before)
						{
							return before.strGroupId == after.strGroupId;
						});
					return partialStatesBeforeArenaBreak.end() != beforeIt &&
						beforeIt->eState == after.eState &&
						beforeIt->iStateVersion == after.iStateVersion &&
						beforeIt->iStateStartTick == after.iStateStartTick &&
						beforeIt->iCommitTick == after.iCommitTick &&
						beforeIt->strPendingMutationId ==
							after.strPendingMutationId;
				});
		tests.Require(
			committedPartialOuter,
			"Commit the consumed outer wall from BREAKING to DESPAWNED at its due tick in every configuration");
		tests.Require(
			partialOuterRoom.m_isReady && appliedPartialArenaBreak,
			"Accept the later 109 impact after one shared outer mutation is already final in every configuration");
		tests.Require(
			partialConsumedStayedDespawned &&
			1u == partialOuterDespawnedCount &&
			29u == partialOuterBreakingCount,
			"Keep one consumed outer wall DESPAWNED and commit only the other twenty-nine to BREAKING");
		tests.Require(
			partialUnrelatedGroupsUnchanged,
			"Leave every floor sector and entrance wall unchanged across the Release-safe partial 109 batch");
		const bool committedPartialArena = appliedPartialArenaBreak &&
			partialOuterRoom.Commit_DueWorldDestruction(
				partialArenaCommitTick);
		const auto partialFinalStates =
			partialOuterRoom.m_WorldDestructionRuntime.Get_GroupStates();
		const std::size_t partialFinalDespawnedCount =
			static_cast<std::size_t>(std::count_if(
				partialFinalStates.begin(), partialFinalStates.end(),
				[](const WORLD_DESTRUCTION_GROUP_STATE& state)
				{
					return 0u == state.strGroupId.rfind(
							"destroyable.group.valtan.outerwall109.", 0u) &&
						WORLD_DESTRUCTION_STATE::DESPAWNED == state.eState;
				}));
		tests.Require(
			committedPartialArena && 30u == partialFinalDespawnedCount,
			"Finish the Release-safe partial 109 batch with all thirty outer walls DESPAWNED");
	}

	{
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		SERVER_WORLD_ENTITY boss{};
		boss.iNetEntityId = 7003u;
		/* Stand in front of the wall instead of inside it.  These are the exact
		   DOWN_SMASH authored proxy semantics compiled above: a ten-metre cross
		   with a 1.8-metre half-width. */
		boss.fPositionX = VALTAN_WALL_CENTER_X;
		boss.fPositionY = VALTAN_WALL_CENTER_Y;
		boss.fPositionZ = VALTAN_WALL_CENTER_Z - 4.f;
		boss.fYawDegrees = 0.f;
		boss.fCollisionRadius = 1.f;
		boss.ePatternHitShape = BOSS_PATTERN_HIT_SHAPE::CROSS;
		boss.fPatternHitLength = 10.f;
		boss.fPatternHitHalfWidth = 1.8f;
		WORLD_DESTRUCTION_GROUP_STATE groupState{};
		const std::string groupId =
			"destroyable.group.valtan.wall159.15719065619666776634";
		const bool initiallyIntact = room.Is_Ready() &&
			room.m_WorldDestructionRuntime.Find_GroupState(
				groupId, groupState) &&
			WORLD_DESTRUCTION_STATE::INTACT == groupState.eState;
		boss.bPatternWallContact = false;
		const bool refusedUnmarkedHit =
			room.Apply_WorldDestructionPatternHitContact(boss, 700u) &&
			room.m_WorldDestructionRuntime.Find_GroupState(
				groupId, groupState) &&
			WORLD_DESTRUCTION_STATE::INTACT == groupState.eState;
		boss.bPatternWallContact = true;
		std::vector<std::string> observedAxeContacts;
		room.m_ServerCollisionSystem.Collect_BossPatternHitContacts(
			boss.ePatternHitShape,
			boss.fPositionX, boss.fPositionY, boss.fPositionZ,
			boss.fYawDegrees, boss.fCollisionRadius,
			boss.fPatternHitOuterRadius, boss.fPatternHitInnerRadius,
			boss.fPatternHitAngleDegrees, boss.fPatternHitLength,
			boss.fPatternHitHalfWidth, observedAxeContacts);
		const bool foundTargetContact = observedAxeContacts.end() != std::find(
			observedAxeContacts.begin(), observedAxeContacts.end(),
			VALTAN_WALL_COLLISION_STATE);
		const bool acceptedAxeHit =
			room.Apply_WorldDestructionPatternHitContact(boss, 701u) &&
			room.m_WorldDestructionRuntime.Find_GroupState(
				groupId, groupState) &&
			WORLD_DESTRUCTION_STATE::BREAKING == groupState.eState &&
			room.m_ServerCollisionSystem.Is_PlayerBlocking(
				VALTAN_WALL_COLLISION_STATE);
		tests.Require(initiallyIntact,
			"Start the axe-contact wall integration from INTACT");
		tests.Require(refusedUnmarkedHit,
			"Keep a physical hit volume harmless when its action is not wall-contact authored");
		tests.Require(foundTargetContact,
			"Resolve the authored down-smash axe proxy against the wall in front of Valtan");
		tests.Require(acceptedAxeHit,
			"Commit BREAKING when an authored axe hit volume touches one wall collider");
		const bool committedAxeHit = room.Commit_DueWorldDestruction(709u) &&
			room.m_WorldDestructionRuntime.Find_GroupState(
				groupId, groupState) &&
			WORLD_DESTRUCTION_STATE::DESPAWNED == groupState.eState &&
			!room.m_ServerCollisionSystem.Is_PlayerBlocking(
				VALTAN_WALL_COLLISION_STATE) &&
			room.m_ServerNavigation.Is_PointWalkableExact(
				VALTAN_WALL_CENTER_X, VALTAN_WALL_CENTER_Z);
		tests.Require(committedAxeHit,
			"Gate axe wall contact by authored action and atomically open its collision and navigation at the due tick");
	}

	{
		/* Debug Valtan audition. The point of the two-step ARM/CROSS contract is
		that dropping straight onto a low bar crosses every threshold above it,
		so this checks the single-crossing property against the real encounter
		patterns rather than a synthetic pair. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr SESSION_ID AUDITION_SESSION = 4242u;
		constexpr PLAYER_ID AUDITION_PLAYER = 77u;
		constexpr std::uint32_t TARGET_BAR = 109u;

		const bool activated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* auditionBoss = room.Find_AuditionBoss();
		/* Drive one exact pattern: stage the encounter intro as already
		consumed so the first-appearance sweep is not the first sequence. */
		if (nullptr != auditionBoss)
			auditionBoss->bIntroPatternConsumed = true;
		tests.Require(
			activated && nullptr != auditionBoss &&
			60000u == (nullptr == auditionBoss ? 0u : auditionBoss->iMaximumHp) &&
			160u == (nullptr == auditionBoss ?
				0u : auditionBoss->iMaximumHealthBars),
			"Activate the audition Valtan with its authored health bar scale");

		tests.Require(
			nullptr != auditionBoss &&
			30375u == CValtanBrain::Resolve_HealthBarHp(*auditionBoss, 81u) &&
			30000u == CValtanBrain::Resolve_HealthBarHp(*auditionBoss, 80u) &&
			0u == CValtanBrain::Resolve_HealthBarHp(*auditionBoss, 161u),
			"Resolve authored health bar boundaries and reject bars off the scale");

		C2S_VALTAN_AUDITION_REQUEST arm{};
		arm.iRequestSequence = 1u;
		arm.eOperation = VALTAN_AUDITION_OPERATION::ARM_HEALTH_BAR;
		arm.iTargetHealthBar = TARGET_BAR;
		std::uint32_t reportedBar = 0u;

		SERVER_PLAYER auditionPlayer{};
		auditionPlayer.iPlayerId = AUDITION_PLAYER;
		auditionPlayer.iNetEntityId = 900u;
		auditionPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		auditionPlayer.iCurrentHp = 1000u;
		auditionPlayer.iMaximumHp = 1000u;
		/* SERVER_PLAYER starts combat-ready, so the not-engaged path has to be
		asked for explicitly rather than left to the default. */
		auditionPlayer.isCombatReady = false;
		if (nullptr != auditionBoss)
		{
			auditionPlayer.fPositionX = auditionBoss->fPositionX + 2.f;
			auditionPlayer.fPositionY = auditionBoss->fPositionY;
			auditionPlayer.fPositionZ = auditionBoss->fPositionZ;
		}
		room.m_Players.emplace(AUDITION_PLAYER, auditionPlayer);

#ifndef _DEBUG
		/* A Release Server never auditions. It still answers, because the packet
		type stays known so a Debug Client gets a verdict instead of a closed
		socket, but the boss must not move even for an otherwise valid request. */
		room.m_PlayerIdBySessionId.emplace(AUDITION_SESSION, AUDITION_PLAYER);
		room.m_Players.at(AUDITION_PLAYER).isCombatReady = true;
		const std::uint32_t releaseHpBefore =
			nullptr == auditionBoss ? 0u : auditionBoss->iCurrentHp;
		reportedBar = 12345u;
		tests.Require(
			VALTAN_AUDITION_RESULT::REJECTED_RELEASE_BUILD ==
				room.Evaluate_ValtanAudition(
					AUDITION_SESSION, arm, reportedBar) &&
			0u == reportedBar &&
			nullptr != auditionBoss &&
			releaseHpBefore ==
				(nullptr == auditionBoss ? 1u : auditionBoss->iCurrentHp),
			"Reject every Valtan audition in a Release Server without moving the boss");
#else
		tests.Require(
			VALTAN_AUDITION_RESULT::REJECTED_WRONG_WORLD ==
				room.Evaluate_ValtanAudition(AUDITION_SESSION, arm, reportedBar),
			"Reject a Valtan audition from a session that never joined the room");
		room.m_PlayerIdBySessionId.emplace(AUDITION_SESSION, AUDITION_PLAYER);

		tests.Require(
			VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED ==
				room.Evaluate_ValtanAudition(AUDITION_SESSION, arm, reportedBar),
			"Reject a Valtan audition the brain would drop for want of a target");
		room.m_Players.at(AUDITION_PLAYER).isCombatReady = true;

		C2S_VALTAN_AUDITION_REQUEST cross = arm;
		cross.iRequestSequence = 2u;
		cross.eOperation = VALTAN_AUDITION_OPERATION::CROSS_HEALTH_BAR;
		tests.Require(
			VALTAN_AUDITION_RESULT::REJECTED_NOT_ARMED ==
				room.Evaluate_ValtanAudition(
					AUDITION_SESSION, cross, reportedBar),
			"Reject a Valtan crossing that was never armed at the same bar");

		C2S_VALTAN_AUDITION_REQUEST unknownBar = arm;
		unknownBar.iRequestSequence = 3u;
		unknownBar.iTargetHealthBar = 79u;
		tests.Require(
			VALTAN_AUDITION_RESULT::REJECTED_UNKNOWN_HEALTH_BAR ==
				room.Evaluate_ValtanAudition(
					AUDITION_SESSION, unknownBar, reportedBar),
			"Reject a Valtan audition on a bar that carries no authored pattern");

		C2S_VALTAN_AUDITION_REQUEST play{};
		play.iRequestSequence = 4u;
		play.eOperation = VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR;
		play.iTargetHealthBar = 30u;
		tests.Require(
			VALTAN_AUDITION_RESULT::QUEUED ==
				room.Evaluate_ValtanAudition(
					AUDITION_SESSION, play, reportedBar) &&
			30u == reportedBar && nullptr != auditionBoss &&
			31u == (nullptr == auditionBoss ?
				0u : auditionBoss->iLastEvaluatedHealthBar) &&
			11250u == (nullptr == auditionBoss ?
				0u : auditionBoss->iCurrentHp),
			"Prime and cross one authored bar atomically for one-click audition");

		arm.iRequestSequence = 5u;
		const VALTAN_AUDITION_RESULT armResult =
			room.Evaluate_ValtanAudition(AUDITION_SESSION, arm, reportedBar);
		tests.Require(
			VALTAN_AUDITION_RESULT::ARMED == armResult &&
			110u == reportedBar &&
			nullptr != auditionBoss &&
			41250u == (nullptr == auditionBoss ? 0u : auditionBoss->iCurrentHp) &&
			110u == (nullptr == auditionBoss ?
				0u : auditionBoss->iLastEvaluatedHealthBar),
			"Arm the audition one bar above the target without crossing it");

		tests.Require(
			VALTAN_AUDITION_RESULT::DUPLICATE_IGNORED ==
				room.Evaluate_ValtanAudition(AUDITION_SESSION, arm, reportedBar) &&
			nullptr != auditionBoss &&
			41250u == (nullptr == auditionBoss ? 0u : auditionBoss->iCurrentHp),
			"Answer a resent Valtan audition sequence without moving the boss");

		cross.iRequestSequence = 6u;
		const VALTAN_AUDITION_RESULT crossResult =
			room.Evaluate_ValtanAudition(AUDITION_SESSION, cross, reportedBar);
		tests.Require(
			VALTAN_AUDITION_RESULT::QUEUED == crossResult &&
			TARGET_BAR == reportedBar &&
			nullptr != auditionBoss &&
			40875u == (nullptr == auditionBoss ? 0u : auditionBoss->iCurrentHp) &&
			110u == (nullptr == auditionBoss ?
				0u : auditionBoss->iLastEvaluatedHealthBar) &&
			(nullptr == auditionBoss || auditionBoss->PendingPatternIds.empty()),
			"Cross onto the target bar and leave the queueing to CValtanBrain");

		CValtanBrain auditionBrain;
		std::vector<DAMAGE_EVENT> auditionDamage;
		if (nullptr != auditionBoss)
		{
			auditionBrain.Update(
				*auditionBoss, room.m_Players, room.m_GameplayCatalog,
				room.m_ServerNavigation, 1.f / 30.f, 500u, {}, auditionDamage);
		}
		const bool queuedOnlyTarget = nullptr != auditionBoss &&
			1u == auditionBoss->TriggeredPatternIds.size() &&
			"VALTAN_ARENA_BREAK_109" == auditionBoss->TriggeredPatternIds.front();
		tests.Require(
			queuedOnlyTarget,
			"Queue only the 109-bar pattern from an armed single-bar crossing");
		tests.Require(
			nullptr != auditionBoss &&
			auditionBoss->TriggeredPatternIds.end() == std::find_if(
				auditionBoss->TriggeredPatternIds.begin(),
				auditionBoss->TriggeredPatternIds.end(),
				[](const std::string& patternId)
				{
					return "VALTAN_ARMOR_BREAK_OPENING" == patternId ||
						"VALTAN_FLOOR_WIPE_130" == patternId ||
						"VALTAN_FOUR_PILLARS_105" == patternId;
				}),
			"Leave the 159, 130 and 100 bar patterns unqueued by a 109-bar audition");
		tests.Require(
			nullptr != auditionBoss &&
			"VALTAN_ARENA_BREAK_109" ==
				(nullptr == auditionBoss ? std::string{} :
					auditionBoss->strPatternId) &&
			1u == (nullptr == auditionBoss ? 0u : auditionBoss->iPatternSequence),
			"Advance the audition pattern sequence exactly once");

		C2S_VALTAN_AUDITION_REQUEST whileRunning = arm;
		whileRunning.iRequestSequence = 7u;
		whileRunning.iTargetHealthBar = 30u;
		tests.Require(
			VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE ==
				room.Evaluate_ValtanAudition(
					AUDITION_SESSION, whileRunning, reportedBar),
			"Reject a Valtan audition while an authored pattern is still running");

		const std::uint32_t epochBeforeRepeatPlay =
			room.m_WorldDestructionRuntime.Get_EncounterEpoch();
		C2S_VALTAN_AUDITION_REQUEST repeatPlay{};
		repeatPlay.iRequestSequence = 8u;
		repeatPlay.eOperation = VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR;
		repeatPlay.iTargetHealthBar = TARGET_BAR;
		tests.Require(
			VALTAN_AUDITION_RESULT::QUEUED ==
				room.Evaluate_ValtanAudition(
					AUDITION_SESSION, repeatPlay, reportedBar) &&
			epochBeforeRepeatPlay + 1u ==
				room.m_WorldDestructionRuntime.Get_EncounterEpoch() &&
			TARGET_BAR == reportedBar && nullptr != auditionBoss &&
			auditionBoss->strPatternId.empty() &&
			auditionBoss->TriggeredPatternIds.empty() &&
			110u == auditionBoss->iLastEvaluatedHealthBar &&
			40875u == auditionBoss->iCurrentHp,
			"Reset a running audition and queue 109 again from one repeatable Play request");

		C2S_VALTAN_AUDITION_REQUEST wallAttack{};
		wallAttack.iRequestSequence = 9u;
		wallAttack.eOperation = VALTAN_AUDITION_OPERATION::PLAY_WALL_ATTACK;
		wallAttack.iTargetHealthBar = 0u;
		room.m_Players.at(AUDITION_PLAYER).iCurrentHp = 5500u;
		room.m_Players.at(AUDITION_PLAYER).iMaximumHp = 5500u;
		const bool queuedWallAttack =
			VALTAN_AUDITION_RESULT::QUEUED ==
				room.Evaluate_ValtanAudition(
					AUDITION_SESSION, wallAttack, reportedBar) &&
			nullptr != auditionBoss &&
			1u == auditionBoss->PendingPatternIds.size() &&
			"VALTAN_DOWN_SMASH" == auditionBoss->PendingPatternIds.front();
		tests.Require(
			queuedWallAttack,
			"Queue the exact authored down-smash from the Debug wall-attack button");

		WORLD_DESTRUCTION_GROUP_STATE attackWallState{};
		const std::string attackWallGroup =
			"destroyable.group.valtan.wall159.15719065619666776634";
		room.Tick(1.f / 30.f);
		const bool attackStartedBeforeContact = nullptr != auditionBoss &&
			"VALTAN_DOWN_SMASH" == auditionBoss->strPatternId &&
			room.m_WorldDestructionRuntime.Find_GroupState(
				attackWallGroup, attackWallState) &&
			WORLD_DESTRUCTION_STATE::INTACT == attackWallState.eState;
		tests.Require(
			attackStartedBeforeContact,
			"Keep the target wall intact during down-smash windup instead of breaking it from boss placement");
		for (std::uint32_t tick = 0u; tick < 79u; ++tick)
			room.Tick(1.f / 30.f);
		const std::vector<WORLD_DESTRUCTION_GROUP_STATE> postAttackStates =
			room.m_WorldDestructionRuntime.Get_GroupStates();
		const bool outerStayedIntact = std::all_of(
			postAttackStates.begin(), postAttackStates.end(),
			[](const WORLD_DESTRUCTION_GROUP_STATE& state)
			{
				return 0u != state.strGroupId.rfind(
					"destroyable.group.valtan.outerwall109.", 0u) ||
					WORLD_DESTRUCTION_STATE::INTACT == state.eState;
			});
		const bool attackOpenedTarget =
			room.m_WorldDestructionRuntime.Find_GroupState(
				attackWallGroup, attackWallState) &&
			WORLD_DESTRUCTION_STATE::DESPAWNED == attackWallState.eState &&
			!room.m_ServerCollisionSystem.Is_PlayerBlocking(
				"collision.valtan.wallgroup.11047903315509031966.15719065619666776634") &&
			room.m_ServerNavigation.Is_PointWalkableExact(
				VALTAN_WALL_CENTER_X, VALTAN_WALL_CENTER_Z);
		tests.Require(
			attackOpenedTarget && outerStayedIntact,
			"Break and remove an ordinary attack-contact wall while all thirty 109 outer walls remain intact");

		C2S_VALTAN_AUDITION_REQUEST finalArena{};
		finalArena.iRequestSequence = 10u;
		finalArena.eOperation = VALTAN_AUDITION_OPERATION::SHOW_FINAL_ARENA;
		finalArena.iTargetHealthBar = 0u;
		const bool acceptedFinalArena =
			VALTAN_AUDITION_RESULT::QUEUED ==
				room.Evaluate_ValtanAudition(
					AUDITION_SESSION, finalArena, reportedBar);
		const std::vector<WORLD_DESTRUCTION_GROUP_STATE> stagedFinalStates =
			room.m_WorldDestructionRuntime.Get_GroupStates();
		const bool stagedFinalArena = acceptedFinalArena &&
			std::all_of(
				stagedFinalStates.begin(), stagedFinalStates.end(),
				[](const WORLD_DESTRUCTION_GROUP_STATE& state)
				{
					return WORLD_DESTRUCTION_STATE::BREAKING == state.eState;
				});
		tests.Require(
			stagedFinalArena,
			"Stage every independent wall and floor sector as one Server-authoritative final-arena Debug transaction");
		for (std::uint32_t tick = 0u; tick < 9u; ++tick)
			room.Tick(1.f / 30.f);
		const std::vector<WORLD_DESTRUCTION_GROUP_STATE> finalArenaStates =
			room.m_WorldDestructionRuntime.Get_GroupStates();
		tests.Require(
			105u == finalArenaStates.size() &&
			std::all_of(
				finalArenaStates.begin(), finalArenaStates.end(),
				[](const WORLD_DESTRUCTION_GROUP_STATE& state)
				{
					return WORLD_DESTRUCTION_STATE::DESPAWNED == state.eState;
				}),
			"Commit all ninety-nine walls and six floor sectors to the disappeared final-arena state");

		/* The open-arena view resets first and then stages walls only, so the 84
		and 30 collapses can still be auditioned with nothing standing above. */
		C2S_VALTAN_AUDITION_REQUEST openArena{};
		openArena.iRequestSequence = 11u;
		openArena.eOperation = VALTAN_AUDITION_OPERATION::BREAK_EVERY_WALL;
		openArena.iTargetHealthBar = 0u;
		const bool acceptedOpenArena =
			VALTAN_AUDITION_RESULT::QUEUED ==
				room.Evaluate_ValtanAudition(
					AUDITION_SESSION, openArena, reportedBar);
		for (std::uint32_t tick = 0u; tick < 9u; ++tick)
			room.Tick(1.f / 30.f);
		const std::vector<WORLD_DESTRUCTION_GROUP_STATE> openArenaStates =
			room.m_WorldDestructionRuntime.Get_GroupStates();
		std::size_t removedWallCount = 0u;
		std::size_t intactFloorCount = 0u;
		for (const WORLD_DESTRUCTION_GROUP_STATE& state : openArenaStates)
		{
			if (0u == state.strGroupId.rfind(
				"destroyable.group.valtan.floor", 0u))
			{
				if (WORLD_DESTRUCTION_STATE::INTACT == state.eState)
					++intactFloorCount;
				continue;
			}
			if (WORLD_DESTRUCTION_STATE::DESPAWNED == state.eState)
				++removedWallCount;
		}
		tests.Require(
			acceptedOpenArena && 105u == openArenaStates.size() &&
			99u == removedWallCount && 6u == intactFloorCount,
			"Remove every wall for the open-arena view while all six floor sectors stay intact");
#endif

		room.m_Players.clear();
		room.m_PlayerIdBySessionId.clear();
		tests.Require(
			room.Reset_ValtanArenaWhenEmpty() &&
			0u == room.m_iValtanAuditionArmedHealthBar,
			"Drop the armed audition bar with the encounter reset");
	}

#ifdef _DEBUG
	{
		/* Effect Tool names the already-spawned private-room boss and pattern by
		stable ID. The request resets only that boss, then CValtanBrain starts the
		exact product pattern on the following fixed tick. */
		CGameRoom room{ WORLD_ID::CHARACTER_SELECT_ARENA };
		constexpr SESSION_ID TOOL_SESSION = 4343u;
		constexpr PLAYER_ID TOOL_PLAYER = 79u;
		constexpr NET_ENTITY_ID TOOL_PLAYER_ENTITY = 902u;
		constexpr NET_ENTITY_ID TOOL_BOSS_ENTITY = 904u;
		const std::string bossPlacementId =
			"boss.valtan.character-select.lazy";
		const WORLD_BOOTSTRAP_PLACEMENT* placement =
			room.Find_Placement(bossPlacementId);
		SERVER_WORLD_ENTITY stagedBoss{};
		const bool builtBoss = room.Is_Ready() && nullptr != placement &&
			!placement->isEnabled &&
			room.Build_WorldEntity(
				*placement, TOOL_BOSS_ENTITY, stagedBoss);
		if (builtBoss)
			room.m_WorldEntities.push_back(std::move(stagedBoss));
		SERVER_WORLD_ENTITY* boss =
			room.Find_AuditionBoss(bossPlacementId);

		SERVER_PLAYER player{};
		player.iSessionId = TOOL_SESSION;
		player.iPlayerId = TOOL_PLAYER;
		player.iNetEntityId = TOOL_PLAYER_ENTITY;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = 1000u;
		player.iMaximumHp = 1000u;
		player.isCombatReady = true;
		if (nullptr != boss)
		{
			player.fPositionX = boss->fPositionX + 2.f;
			player.fPositionY = boss->fPositionY;
			player.fPositionZ = boss->fPositionZ;
			boss->iPatternSequence = 9u;
			boss->BossCombat.iStateRevision = 15u;
			boss->iCurrentHp = 1u;
			boss->eAction = SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
			boss->strPatternId = "previous.pattern";
		}
		room.m_Players.emplace(TOOL_PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(TOOL_SESSION, TOOL_PLAYER);
		room.m_PlayerIdByEntityId.emplace(
			TOOL_PLAYER_ENTITY, TOOL_PLAYER);
		/* The legacy Level panel and Effect Tool own independent sequence
		counters. A high legacy sequence must not make the Tool's first request
		look stale. */
		room.m_ValtanAuditionSequenceBySessionId.emplace(TOOL_SESSION, 100u);

		C2S_VALTAN_AUDITION_REQUEST blocked{};
		blocked.iRequestSequence = 1u;
		blocked.eOperation = VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID;
		blocked.strBossPlacementId = bossPlacementId;
		blocked.strPatternId = "VALTAN_ARENA_BREAK_109";
		std::uint32_t reportedBar = 0u;
		const VALTAN_AUDITION_RESULT blockedResult =
			room.Evaluate_ValtanAudition(
				TOOL_SESSION, blocked, reportedBar);
		const bool blockedWithoutMutation = nullptr != boss &&
			VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE ==
				blockedResult &&
			1u == boss->iCurrentHp && 9u == boss->iPatternSequence &&
			"previous.pattern" == boss->strPatternId &&
			boss->PendingPatternIds.empty();

		C2S_VALTAN_AUDITION_REQUEST dash = blocked;
		dash.iRequestSequence = 2u;
		dash.strPatternId = "VALTAN_DASH_CHARGE";
		const VALTAN_AUDITION_RESULT queued =
			room.Evaluate_ValtanAudition(
				TOOL_SESSION, dash, reportedBar);
		boss = room.Find_AuditionBoss(bossPlacementId);
		const bool resetAndQueued = nullptr != boss &&
			VALTAN_AUDITION_RESULT::QUEUED == queued &&
			TOOL_BOSS_ENTITY == boss->iNetEntityId &&
			9u == boss->iPatternSequence &&
			boss->BossCombat.iStateRevision > 15u &&
			boss->iCurrentHp == boss->iMaximumHp &&
			SERVER_ENTITY_ACTION::IDLE == boss->eAction &&
			boss->strPatternId.empty() &&
			1u == boss->PendingPatternIds.size() &&
			"VALTAN_DASH_CHARGE" == boss->PendingPatternIds.front();
		const VALTAN_AUDITION_RESULT duplicate =
			room.Evaluate_ValtanAudition(
				TOOL_SESSION, dash, reportedBar);
		const bool duplicateDidNotQueueAgain = nullptr != boss &&
			VALTAN_AUDITION_RESULT::DUPLICATE_IGNORED == duplicate &&
			1u == boss->PendingPatternIds.size();

		room.Tick(1.f / 30.f);
		boss = room.Find_AuditionBoss(bossPlacementId);
		tests.Require(
			builtBoss && blockedWithoutMutation && resetAndQueued &&
			duplicateDidNotQueueAgain && nullptr != boss &&
			boss->PendingPatternIds.empty() &&
			"VALTAN_DASH_CHARGE" == boss->strPatternId &&
			10u == boss->iPatternSequence &&
			SERVER_ENTITY_ACTION::PATTERN_WINDUP == boss->eAction,
			"Play the stable-ID Dash Charge through the Character Select Server boss while blocking arena-only patterns");
	}

	{
		/* A shared Valtan room can receive commands from two sessions in one
		   command-drain tick. The first stable-ID request owns its pending/active
		   occurrence; the second verdict is consumed but cannot reset it away. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr SESSION_ID FIRST_SESSION = 4344u;
		constexpr SESSION_ID SECOND_SESSION = 4345u;
		constexpr PLAYER_ID FIRST_PLAYER = 81u;
		constexpr PLAYER_ID SECOND_PLAYER = 82u;
		const bool activated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();

		SERVER_PLAYER first{};
		first.iSessionId = FIRST_SESSION;
		first.iPlayerId = FIRST_PLAYER;
		first.iNetEntityId = 905u;
		first.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		first.iCurrentHp = 5500u;
		first.iMaximumHp = 5500u;
		first.isCombatReady = true;
		if (nullptr != boss)
		{
			first.fPositionX = boss->fPositionX + 2.f;
			first.fPositionY = boss->fPositionY;
			first.fPositionZ = boss->fPositionZ;
		}
		SERVER_PLAYER second = first;
		second.iSessionId = SECOND_SESSION;
		second.iPlayerId = SECOND_PLAYER;
		second.iNetEntityId = 906u;
		if (nullptr != boss)
			second.fPositionX = boss->fPositionX + 3.f;
		room.m_Players.emplace(FIRST_PLAYER, first);
		room.m_Players.emplace(SECOND_PLAYER, second);
		room.m_PlayerIdBySessionId.emplace(FIRST_SESSION, FIRST_PLAYER);
		room.m_PlayerIdBySessionId.emplace(SECOND_SESSION, SECOND_PLAYER);
		room.m_PlayerIdByEntityId.emplace(first.iNetEntityId, FIRST_PLAYER);
		room.m_PlayerIdByEntityId.emplace(second.iNetEntityId, SECOND_PLAYER);
		auto lifecycleSession = std::make_shared<CClientSession>(
			FIRST_SESSION, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		lifecycleSession->m_isSendRunning.store(true);
		room.m_Sessions.emplace(FIRST_SESSION, lifecycleSession);

		C2S_VALTAN_AUDITION_REQUEST firstRequest{};
		firstRequest.iRequestSequence = 1u;
		firstRequest.eOperation =
			VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID;
		firstRequest.strBossPlacementId = "boss.valtan.center";
		firstRequest.strPatternId = "VALTAN_DASH_CHARGE";
		C2S_VALTAN_AUDITION_REQUEST secondRequest = firstRequest;
		secondRequest.strPatternId = "VALTAN_WHIRLWIND";
		std::uint32_t reportedBar = 0u;
		const VALTAN_AUDITION_RESULT firstResult =
			room.Evaluate_ValtanAudition(
				FIRST_SESSION, firstRequest, reportedBar);
		const VALTAN_AUDITION_RESULT secondResult =
			room.Evaluate_ValtanAudition(
				SECOND_SESSION, secondRequest, reportedBar);
		boss = room.Find_AuditionBoss();
		const bool firstStillOwnsPending = nullptr != boss &&
			VALTAN_AUDITION_RESULT::QUEUED == firstResult &&
			VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE ==
				secondResult &&
			1u == boss->PendingPatternIds.size() &&
			"VALTAN_DASH_CHARGE" == boss->PendingPatternIds.front() &&
			CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING ==
				room.m_ValtanPatternIdAudition.ePhase &&
			FIRST_SESSION ==
				room.m_ValtanPatternIdAudition.iOwnerSessionId &&
			1u == room.m_ValtanPatternIdAuditionSequenceBySessionId.at(
				SECOND_SESSION);

		room.Tick(1.f / 30.f);
		boss = room.Find_AuditionBoss();
		tests.Require(
			activated && firstStillOwnsPending && nullptr != boss &&
			boss->PendingPatternIds.empty() &&
			"VALTAN_DASH_CHARGE" == boss->strPatternId &&
			SERVER_ENTITY_ACTION::PATTERN_WINDUP == boss->eAction &&
			CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE ==
				room.m_ValtanPatternIdAudition.ePhase &&
			FIRST_SESSION ==
				room.m_ValtanPatternIdAudition.iOwnerSessionId,
			"Keep the first shared-room stable-ID pattern when two sessions request different patterns before the fixed tick");

		if (nullptr != boss)
		{
			boss->strPatternId.clear();
			boss->strPatternStageId.clear();
			boss->strActionId.clear();
			boss->eAction = SERVER_ENTITY_ACTION::IDLE;
		}
		const bool completedDetected =
			!room.Refresh_ValtanPatternIdAuditionState() &&
			room.Flush_ValtanPatternIdAuditionLifecycle();

		C2S_VALTAN_AUDITION_REQUEST abortedRequest = firstRequest;
		abortedRequest.iRequestSequence = 2u;
		abortedRequest.strPatternId = "VALTAN_WHIRLWIND";
		const VALTAN_AUDITION_RESULT secondQueued =
			room.Evaluate_ValtanAudition(
				FIRST_SESSION, abortedRequest, reportedBar);
		boss = room.Find_AuditionBoss();
		if (nullptr != boss)
		{
			boss->iCurrentHp = 0u;
			boss->eAction = SERVER_ENTITY_ACTION::DEAD;
		}
		const bool abortedDetected =
			!room.Refresh_ValtanPatternIdAuditionState() &&
			room.Flush_ValtanPatternIdAuditionLifecycle();

		std::vector<S2C_VALTAN_AUDITION_LIFECYCLE> lifecycleMessages;
		for (const auto& outbound : lifecycleSession->m_OutboundFrames)
		{
			PACKET_HEADER header{};
			if (!Read_Packet_Header(outbound.Bytes, header) ||
				PACKET_TYPE::S2C_VALTAN_AUDITION_LIFECYCLE !=
					header.ePacketType)
			{
				continue;
			}
			CPacketReader reader{ std::span<const std::uint8_t>(
				outbound.Bytes.data() + PACKET_HEADER_BYTES,
				outbound.Bytes.size() - PACKET_HEADER_BYTES) };
			S2C_VALTAN_AUDITION_LIFECYCLE decoded{};
			if (Read_Message(reader, decoded) &&
				0u == reader.Get_RemainingSize())
			{
				lifecycleMessages.push_back(std::move(decoded));
			}
		}
		const GameplayDataRevision lifecycleRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		const bool completeLifecycle = 5u == lifecycleMessages.size() &&
			VALTAN_AUDITION_LIFECYCLE_STATE::PENDING ==
				lifecycleMessages[0].eState &&
			VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE ==
				lifecycleMessages[1].eState &&
			VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED ==
				lifecycleMessages[2].eState &&
			VALTAN_AUDITION_LIFECYCLE_STATE::PENDING ==
				lifecycleMessages[3].eState &&
			VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED ==
				lifecycleMessages[4].eState &&
			1u == lifecycleMessages[0].iRequestSequence &&
			1u == lifecycleMessages[0].iRoomAuditionEpoch &&
			1u == lifecycleMessages[0].iPatternSequence &&
			2u == lifecycleMessages[3].iRequestSequence &&
			2u == lifecycleMessages[3].iRoomAuditionEpoch &&
			2u == lifecycleMessages[3].iPatternSequence &&
			!lifecycleMessages[4].strReason.empty() &&
			std::all_of(lifecycleMessages.begin(), lifecycleMessages.end(),
				[&lifecycleRevision](
					const S2C_VALTAN_AUDITION_LIFECYCLE& lifecycle)
				{
					return lifecycle.PinnedDefinitionRevision ==
						lifecycleRevision;
				});
		tests.Require(
			completedDetected &&
			VALTAN_AUDITION_RESULT::QUEUED == secondQueued &&
			abortedDetected && completeLifecycle,
			"Emit correlated PENDING/ACTIVE/COMPLETED/ABORTED Valtan audition lifecycle edges with pinned revision");
		lifecycleSession->Request_Close();
	}

	{
		/* Exercise the 130-bar floor wipe through the same stable-ID command and
		   fixed room tick path the Effect Tool uses. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr SESSION_ID FLOOR_SESSION = 4346u;
		constexpr PLAYER_ID FLOOR_PLAYER = 83u;
		constexpr NET_ENTITY_ID FLOOR_PLAYER_ENTITY = 907u;
		constexpr std::uint32_t FLOOR_PLAYER_HP = 1000000000u;
		const bool activated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();

		const std::vector<BOSS_PATTERN_DEFINITION>* patterns =
			room.m_GameplayCatalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const BOSS_PATTERN_DEFINITION* floorPattern = nullptr;
		if (nullptr != patterns)
		{
			const auto found = std::find_if(
				patterns->begin(), patterns->end(),
				[](const BOSS_PATTERN_DEFINITION& pattern)
				{
					return "VALTAN_FLOOR_WIPE_130" == pattern.strPatternId;
				});
			if (patterns->end() != found)
				floorPattern = &*found;
		}
		const BOSS_PATTERN_STAGE_DEFINITION* intervalDefinition = nullptr;
		const BOSS_PATTERN_STAGE_DEFINITION* secondSmashDefinition = nullptr;
		if (nullptr != floorPattern)
		{
			for (const BOSS_PATTERN_STAGE_DEFINITION& stage :
				floorPattern->Stages)
			{
				if ("INTERVAL" == stage.strStageId)
					intervalDefinition = &stage;
				else if ("SECOND_SMASH" == stage.strStageId)
					secondSmashDefinition = &stage;
			}
		}
		const auto hasSameRootMotion = [](
			const std::vector<ROOT_MOTION_SAMPLE>& left,
			const std::vector<ROOT_MOTION_SAMPLE>& right)
		{
			return left.size() == right.size() && std::equal(
				left.begin(), left.end(), right.begin(),
				[](const ROOT_MOTION_SAMPLE& leftSample,
					const ROOT_MOTION_SAMPLE& rightSample)
				{
					return leftSample.iTimeMs == rightSample.iTimeMs &&
						leftSample.fForward == rightSample.fForward &&
						leftSample.fLateral == rightSample.fLateral;
				});
		};
		const bool catalogCarriesFloorMotions =
			nullptr != intervalDefinition &&
			2000u == intervalDefinition->iDurationMs &&
			!intervalDefinition->Motion.RootMotion.empty() &&
			2000u == intervalDefinition->Motion.RootMotion.back().iTimeMs &&
			nullptr != secondSmashDefinition &&
			500u == secondSmashDefinition->iDurationMs &&
			!secondSmashDefinition->Motion.RootMotion.empty() &&
			500u == secondSmashDefinition->Motion.RootMotion.back().iTimeMs;

		SERVER_PLAYER player{};
		player.iSessionId = FLOOR_SESSION;
		player.iPlayerId = FLOOR_PLAYER;
		player.iNetEntityId = FLOOR_PLAYER_ENTITY;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		/* The authored wipe still lands during this lifecycle test. Keep the
		   engaged observer alive so RECOVERY can finish naturally. */
		player.iCurrentHp = FLOOR_PLAYER_HP;
		player.iMaximumHp = FLOOR_PLAYER_HP;
		player.isCombatReady = true;
		if (nullptr != boss)
		{
			player.fPositionX = boss->fPositionX + 2.f;
			player.fPositionY = boss->fPositionY;
			player.fPositionZ = boss->fPositionZ;
		}
		room.m_Players.emplace(FLOOR_PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(FLOOR_SESSION, FLOOR_PLAYER);
		room.m_PlayerIdByEntityId.emplace(
			FLOOR_PLAYER_ENTITY, FLOOR_PLAYER);
		auto lifecycleSession = std::make_shared<CClientSession>(
			FLOOR_SESSION, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		lifecycleSession->m_isSendRunning.store(true);
		room.m_Sessions.emplace(FLOOR_SESSION, lifecycleSession);

		C2S_VALTAN_AUDITION_REQUEST request{};
		request.iRequestSequence = 1u;
		request.eOperation = VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID;
		request.strBossPlacementId = "boss.valtan.center";
		request.strPatternId = "VALTAN_FLOOR_WIPE_130";
		std::uint32_t reportedBar = 0u;
		const VALTAN_AUDITION_RESULT queued = room.Evaluate_ValtanAudition(
			FLOOR_SESSION, request, reportedBar);

		std::vector<std::string> observedStageIds;
		bool runtimeStageActionsMatch = true;
		bool intervalMotionActivated = false;
		bool secondSmashMotionActivated = false;
		bool observedPatternInvulnerability = false;
		bool keptPatternInvulnerability = true;
		std::uint32_t firstSmashDamagePulseCount = 0u;
		std::uint32_t secondSmashDamagePulseCount = 0u;
		std::uint64_t appliedFloorDamage = 0u;
		bool completedNaturally = false;
		for (std::uint32_t tick = 0u; tick < 240u && !completedNaturally;
			++tick)
		{
			room.Tick(1.f / 30.f);
			boss = room.Find_AuditionBoss();
			if (nullptr == boss)
			{
				runtimeStageActionsMatch = false;
				break;
			}
			if ("VALTAN_FLOOR_WIPE_130" == boss->strPatternId &&
				!boss->strPatternStageId.empty())
			{
				observedPatternInvulnerability = true;
				keptPatternInvulnerability = keptPatternInvulnerability &&
					boss->bPatternInvulnerable;
				if (observedStageIds.empty() ||
					observedStageIds.back() != boss->strPatternStageId)
				{
					observedStageIds.push_back(boss->strPatternStageId);
					const bool actionMatches =
						(("WINDUP" == boss->strPatternStageId ||
						  "INTERVAL" == boss->strPatternStageId) &&
						 SERVER_ENTITY_ACTION::PATTERN_WINDUP == boss->eAction) ||
						(("FIRST_SMASH" == boss->strPatternStageId ||
						  "SECOND_SMASH" == boss->strPatternStageId) &&
						 SERVER_ENTITY_ACTION::PATTERN_ACTIVE == boss->eAction) ||
						("RECOVERY" == boss->strPatternStageId &&
						 SERVER_ENTITY_ACTION::PATTERN_RECOVERY == boss->eAction);
					runtimeStageActionsMatch =
						runtimeStageActionsMatch && actionMatches;
				}
				if ("INTERVAL" == boss->strPatternStageId &&
					nullptr != intervalDefinition)
				{
					intervalMotionActivated =
						2000u == boss->iPatternStageDurationMs &&
						hasSameRootMotion(
							boss->PatternStageRootMotion,
							intervalDefinition->Motion.RootMotion);
				}
				else if ("SECOND_SMASH" == boss->strPatternStageId &&
					nullptr != secondSmashDefinition)
				{
					secondSmashMotionActivated =
						500u == boss->iPatternStageDurationMs &&
						hasSameRootMotion(
							boss->PatternStageRootMotion,
							secondSmashDefinition->Motion.RootMotion);
				}
			}
			for (const DAMAGE_EVENT& damage : room.m_TickDamageEvents)
			{
				if (FLOOR_PLAYER_ENTITY != damage.iTargetNetEntityId ||
					damage.isOutgoing)
				{
					continue;
				}
				appliedFloorDamage += damage.iAmount;
				if (nullptr != boss &&
					"FIRST_SMASH" == boss->strPatternStageId)
				{
					++firstSmashDamagePulseCount;
				}
				else if (nullptr != boss &&
					"SECOND_SMASH" == boss->strPatternStageId)
				{
					++secondSmashDamagePulseCount;
				}
			}
			completedNaturally =
				CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE ==
					room.m_ValtanPatternIdAudition.ePhase &&
				!observedStageIds.empty() &&
				"RECOVERY" == observedStageIds.back() &&
				boss->strPatternId.empty() &&
				SERVER_ENTITY_ACTION::IDLE == boss->eAction;
		}
		const auto finishedPlayer = room.m_Players.find(FLOOR_PLAYER);
		const bool bothDamagePulsesApplied =
			room.m_Players.end() != finishedPlayer &&
			1u == firstSmashDamagePulseCount &&
			1u == secondSmashDamagePulseCount &&
			0u != appliedFloorDamage &&
			static_cast<std::uint64_t>(finishedPlayer->second.iCurrentHp) +
				appliedFloorDamage == FLOOR_PLAYER_HP;
		const bool invulnerabilityLifecycle =
			observedPatternInvulnerability && keptPatternInvulnerability &&
			nullptr != boss && !boss->bPatternInvulnerable;

		std::vector<S2C_VALTAN_AUDITION_LIFECYCLE> lifecycleMessages;
		for (const auto& outbound : lifecycleSession->m_OutboundFrames)
		{
			PACKET_HEADER header{};
			if (!Read_Packet_Header(outbound.Bytes, header) ||
				PACKET_TYPE::S2C_VALTAN_AUDITION_LIFECYCLE !=
					header.ePacketType)
			{
				continue;
			}
			CPacketReader reader{ std::span<const std::uint8_t>(
				outbound.Bytes.data() + PACKET_HEADER_BYTES,
				outbound.Bytes.size() - PACKET_HEADER_BYTES) };
			S2C_VALTAN_AUDITION_LIFECYCLE decoded{};
			if (Read_Message(reader, decoded) &&
				0u == reader.Get_RemainingSize())
			{
				lifecycleMessages.push_back(std::move(decoded));
			}
		}
		const GameplayDataRevision lifecycleRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		const bool completedLifecycle = 3u == lifecycleMessages.size() &&
			VALTAN_AUDITION_LIFECYCLE_STATE::PENDING ==
				lifecycleMessages[0].eState &&
			VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE ==
				lifecycleMessages[1].eState &&
			VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED ==
				lifecycleMessages[2].eState &&
			std::all_of(lifecycleMessages.begin(), lifecycleMessages.end(),
				[&lifecycleRevision](
					const S2C_VALTAN_AUDITION_LIFECYCLE& lifecycle)
				{
					return 1u == lifecycle.iRequestSequence &&
						1u == lifecycle.iRoomAuditionEpoch &&
						1u == lifecycle.iPatternSequence &&
						"VALTAN_FLOOR_WIPE_130" == lifecycle.strPatternId &&
						lifecycle.PinnedDefinitionRevision == lifecycleRevision;
				});
		const std::uint32_t completedPatternSequence = nullptr == boss ?
			0u : boss->iPatternSequence;
		const std::uint32_t completedRotationStepIndex = nullptr == boss ?
			0u : boss->iRotationStepIndex;
		bool heldIdleAfterCompletion = nullptr != boss &&
			boss->bAutomaticPatternSequenceAuditionOverride &&
			boss->bAutomaticPatternSequenceAuditionHold;
		for (std::uint32_t tick = 0u; tick < 60u; ++tick)
		{
			room.Tick(1.f / 30.f);
			boss = room.Find_AuditionBoss();
			heldIdleAfterCompletion = heldIdleAfterCompletion &&
				nullptr != boss && boss->strPatternId.empty() &&
				boss->PendingPatternIds.empty() &&
				SERVER_ENTITY_ACTION::IDLE == boss->eAction &&
				completedPatternSequence == boss->iPatternSequence &&
				completedRotationStepIndex == boss->iRotationStepIndex &&
				boss->bAutomaticPatternSequenceAuditionOverride &&
				boss->bAutomaticPatternSequenceAuditionHold;
		}

		C2S_VALTAN_AUDITION_REQUEST replayRequest = request;
		replayRequest.iRequestSequence = 2u;
		const VALTAN_AUDITION_RESULT replayQueued =
			room.Evaluate_ValtanAudition(
				FLOOR_SESSION, replayRequest, reportedBar);
		boss = room.Find_AuditionBoss();
		const bool resetReleasedHold = nullptr != boss &&
			VALTAN_AUDITION_RESULT::QUEUED == replayQueued &&
			!boss->bAutomaticPatternSequenceAuditionOverride &&
			!boss->bAutomaticPatternSequenceAuditionHold &&
			1u == boss->PendingPatternIds.size() &&
			"VALTAN_FLOOR_WIPE_130" == boss->PendingPatternIds.front();
		room.Tick(1.f / 30.f);
		boss = room.Find_AuditionBoss();
		const std::uint32_t expectedReplayPatternSequence =
			(std::numeric_limits<std::uint32_t>::max)() ==
				completedPatternSequence ?
			1u : completedPatternSequence + 1u;
		const bool replayStarted = nullptr != boss &&
			boss->PendingPatternIds.empty() &&
			"VALTAN_FLOOR_WIPE_130" == boss->strPatternId &&
			expectedReplayPatternSequence == boss->iPatternSequence &&
			SERVER_ENTITY_ACTION::PATTERN_WINDUP == boss->eAction;
		const std::vector<std::string> expectedStageIds{
			"WINDUP", "FIRST_SMASH", "INTERVAL",
			"SECOND_SMASH", "RECOVERY" };
		tests.Require(
			activated && catalogCarriesFloorMotions &&
			VALTAN_AUDITION_RESULT::QUEUED == queued &&
			observedStageIds == expectedStageIds &&
			runtimeStageActionsMatch && intervalMotionActivated &&
			secondSmashMotionActivated && bothDamagePulsesApplied &&
			invulnerabilityLifecycle && completedNaturally && completedLifecycle,
			"Play FLOOR_WIPE_130 through all five room-ticked stages with its 2000/500 ms root-motion clips, both damage pulses, invulnerability, and completed lifecycle");
		tests.Require(
			heldIdleAfterCompletion && resetReleasedHold && replayStarted,
			"Hold a completed stable-ID audition idle without advancing the Product rotation, then reset and replay the next stable-ID request");
		lifecycleSession->Request_Close();
	}

	{
		/* Boss Tool flows are Debug authoring programs, but every slot still runs
		   through the Product ordered-sequence brain. The test starts from slot two,
		   repeats one pattern under two stable slot IDs, pauses that occurrence for
		   revive, and then verifies stop-after-current on a fresh run. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr SESSION_ID FLOW_SESSION = 4350u;
		constexpr PLAYER_ID FLOW_PLAYER = 90u;
		constexpr NET_ENTITY_ID FLOW_PLAYER_ENTITY = 920u;
		const bool activated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		SERVER_PLAYER player{};
		player.iSessionId = FLOW_SESSION;
		player.iPlayerId = FLOW_PLAYER;
		player.iNetEntityId = FLOW_PLAYER_ENTITY;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = 1u;
		player.iMaximumHp = 1000000000u;
		player.isCombatReady = false;
		room.m_Players.emplace(FLOW_PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(FLOW_SESSION, FLOW_PLAYER);
		room.m_PlayerIdByEntityId.emplace(FLOW_PLAYER_ENTITY, FLOW_PLAYER);

		C2S_DEBUG_VALTAN_PATTERN_FLOW_START request{};
		request.iRequestSequence = 1u;
		request.strBossPlacementId = "boss.valtan.center";
		request.strFlowId = "flow.valtan.server-contract";
		request.strFlowRevision = std::string(64u, 'a');
		request.strStartSlotId = "flow.slot.000002";
		request.iInterStepPursuitMs = 100u;
		request.Slots = {
			{ "flow.slot.000001", "VALTAN_DASH_CHARGE" },
			{ "flow.slot.000002", "VALTAN_WHIRLWIND" },
			{ "flow.slot.000003", "VALTAN_WHIRLWIND" }
		};
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START duplicateSlot = request;
		duplicateSlot.Slots[1].strSlotId = duplicateSlot.Slots[0].strSlotId;
		std::uint32_t roomFlowEpoch = 0u;
		GameplayDataRevision pinnedRevision{};
		std::string flowReason;
		const std::uint32_t patternSequenceBefore = nullptr == boss ?
			0u : boss->iPatternSequence;
		const VALTAN_PATTERN_FLOW_RESULT duplicateSlotResult =
			room.Evaluate_ValtanPatternFlowStart(
				FLOW_SESSION, duplicateSlot, roomFlowEpoch,
				pinnedRevision, flowReason);
		boss = room.Find_AuditionBoss();
		const bool invalidWasTransactional = nullptr != boss &&
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_INVALID_FLOW ==
				duplicateSlotResult &&
			patternSequenceBefore == boss->iPatternSequence &&
			!room.Is_ValtanPatternFlowRunning();

		request.iRequestSequence = 2u;
		auto lifecycleSession = std::make_shared<CClientSession>(
			FLOW_SESSION, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		lifecycleSession->m_isSendRunning.store(true);
		room.m_Sessions.emplace(FLOW_SESSION, lifecycleSession);
		const VALTAN_PATTERN_FLOW_RESULT queued =
			room.Evaluate_ValtanPatternFlowStart(
				FLOW_SESSION, request, roomFlowEpoch,
				pinnedRevision, flowReason);
		boss = room.Find_AuditionBoss();
		const bool stagedSuffix = nullptr != boss &&
			VALTAN_PATTERN_FLOW_RESULT::QUEUED == queued &&
			flowReason.empty() && 0u != roomFlowEpoch &&
			pinnedRevision.Is_Valid() &&
			CGameRoom::VALTAN_PATTERN_FLOW_AUDITION_PHASE::PENDING ==
				room.m_ValtanPatternFlowAudition.ePhase &&
			1u == room.m_ValtanPatternFlowAudition.iStartSlotIndex &&
			2u == room.m_ValtanPatternFlowAudition.Sequence.PatternIds.size() &&
			"VALTAN_WHIRLWIND" ==
				room.m_ValtanPatternFlowAudition.Sequence.PatternIds[0] &&
			"VALTAN_WHIRLWIND" ==
				room.m_ValtanPatternFlowAudition.Sequence.PatternIds[1] &&
			room.m_Players.at(FLOW_PLAYER).iCurrentHp ==
				room.m_Players.at(FLOW_PLAYER).iMaximumHp &&
			room.m_Players.at(FLOW_PLAYER).isCombatReady;
		std::uint32_t duplicateEpoch = 0u;
		GameplayDataRevision duplicateRevision{};
		std::string duplicateReason;
		const VALTAN_PATTERN_FLOW_RESULT duplicateStart =
			room.Evaluate_ValtanPatternFlowStart(
				FLOW_SESSION, request, duplicateEpoch,
				duplicateRevision, duplicateReason);
		const bool duplicateKeptOneProgram =
			VALTAN_PATTERN_FLOW_RESULT::DUPLICATE_IGNORED == duplicateStart &&
			roomFlowEpoch == duplicateEpoch && pinnedRevision == duplicateRevision &&
			room.Is_ValtanPatternFlowRunning();
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START mismatchedDuplicate = request;
		mismatchedDuplicate.strFlowId = "flow.valtan.other-contract";
		std::uint32_t mismatchedEpoch = 123u;
		GameplayDataRevision mismatchedRevision = pinnedRevision;
		std::string mismatchedReason;
		const VALTAN_PATTERN_FLOW_RESULT mismatchedDuplicateResult =
			room.Evaluate_ValtanPatternFlowStart(
				FLOW_SESSION, mismatchedDuplicate, mismatchedEpoch,
				mismatchedRevision, mismatchedReason);
		const bool mismatchedDuplicateRejected =
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW ==
				mismatchedDuplicateResult &&
			0u == mismatchedEpoch && !mismatchedRevision.Is_Valid() &&
			!mismatchedReason.empty() && room.Is_ValtanPatternFlowRunning();
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START mismatchedFlowRevision = request;
		mismatchedFlowRevision.strFlowRevision = std::string(64u, 'b');
		mismatchedEpoch = 123u;
		mismatchedRevision = pinnedRevision;
		mismatchedReason.clear();
		const VALTAN_PATTERN_FLOW_RESULT mismatchedFlowRevisionResult =
			room.Evaluate_ValtanPatternFlowStart(
				FLOW_SESSION, mismatchedFlowRevision, mismatchedEpoch,
				mismatchedRevision, mismatchedReason);
		const bool mismatchedFlowRevisionRejected =
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW ==
				mismatchedFlowRevisionResult &&
			0u == mismatchedEpoch && !mismatchedRevision.Is_Valid() &&
			!mismatchedReason.empty() && room.Is_ValtanPatternFlowRunning();
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START mismatchedSuffix = request;
		mismatchedSuffix.strStartSlotId = mismatchedSuffix.Slots.front().strSlotId;
		mismatchedEpoch = 123u;
		mismatchedRevision = pinnedRevision;
		mismatchedReason.clear();
		const VALTAN_PATTERN_FLOW_RESULT mismatchedSuffixResult =
			room.Evaluate_ValtanPatternFlowStart(
				FLOW_SESSION, mismatchedSuffix, mismatchedEpoch,
				mismatchedRevision, mismatchedReason);
		const bool mismatchedSuffixRejected =
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW ==
				mismatchedSuffixResult &&
			0u == mismatchedEpoch && !mismatchedRevision.Is_Valid() &&
			!mismatchedReason.empty() && room.Is_ValtanPatternFlowRunning();
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START mismatchedSlotPayload = request;
		mismatchedSlotPayload.Slots.front().strPatternId = "VALTAN_WHIRLWIND";
		mismatchedEpoch = 123u;
		mismatchedRevision = pinnedRevision;
		mismatchedReason.clear();
		const VALTAN_PATTERN_FLOW_RESULT mismatchedSlotPayloadResult =
			room.Evaluate_ValtanPatternFlowStart(
				FLOW_SESSION, mismatchedSlotPayload, mismatchedEpoch,
				mismatchedRevision, mismatchedReason);
		const bool mismatchedSlotPayloadRejected =
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW ==
				mismatchedSlotPayloadResult &&
			0u == mismatchedEpoch && !mismatchedRevision.Is_Valid() &&
			!mismatchedReason.empty() && room.Is_ValtanPatternFlowRunning();

		std::vector<std::string> observedStarts;
		std::uint32_t lastObservedPatternSequence = patternSequenceBefore;
		bool injectedDeath = false;
		bool observedPause = false;
		bool observedResume = false;
		bool markedStateBetweenSlots = false;
		bool stateSurvivedIntoSecondSlot = false;
		for (std::uint32_t tick = 0u;
			tick < 600u && room.Is_ValtanPatternFlowRunning(); ++tick)
		{
			room.Tick(1.f / 30.f);
			boss = room.Find_AuditionBoss();
			if (nullptr == boss)
				break;
			if (!boss->strPatternId.empty() &&
				boss->iPatternSequence != lastObservedPatternSequence)
			{
				observedStarts.push_back(boss->strPatternId);
				lastObservedPatternSequence = boss->iPatternSequence;
				if (2u == observedStarts.size())
					stateSurvivedIntoSecondSlot =
						boss->iCurrentHp == boss->iMaximumHp - 123u;
			}
			if (!injectedDeath && 1u == observedStarts.size())
			{
				SERVER_PLAYER& livePlayer = room.m_Players.at(FLOW_PLAYER);
				livePlayer.iCurrentHp = 0u;
				livePlayer.eAction = PLAYER_ACTION_STATE::DEAD;
				livePlayer.isCombatReady = false;
				injectedDeath = true;
				continue;
			}
			if (injectedDeath && !observedPause &&
				boss->bAutomaticPatternSequencePausedForRevive)
			{
				observedPause = true;
				C2S_REVIVE_PLAYER revive{};
				revive.iClientSequence = 1u;
				room.Handle_RevivePlayer(FLOW_SESSION, revive);
				continue;
			}
			if (observedPause &&
				!boss->bAutomaticPatternSequencePausedForRevive &&
				0u != room.m_Players.at(FLOW_PLAYER).iCurrentHp)
			{
				observedResume = true;
			}
			if (!markedStateBetweenSlots &&
				CGameRoom::VALTAN_PATTERN_FLOW_AUDITION_PHASE::PENDING ==
					room.m_ValtanPatternFlowAudition.ePhase &&
				1u == boss->iRotationStepIndex)
			{
				boss->iCurrentHp = boss->iMaximumHp - 123u;
				markedStateBetweenSlots = true;
			}
		}
		boss = room.Find_AuditionBoss();
		const std::vector<std::string> expectedStarts{
			"VALTAN_WHIRLWIND", "VALTAN_WHIRLWIND" };
		bool heldAfterNaturalCompletion = nullptr != boss &&
			!room.Is_ValtanPatternFlowRunning() &&
			boss->bAutomaticPatternSequenceAuditionOverride &&
			boss->bAutomaticPatternSequenceAuditionHold &&
			boss->strPatternId.empty() &&
			observedStarts == expectedStarts;
		const std::uint32_t completedPatternSequence = nullptr == boss ?
			0u : boss->iPatternSequence;
		for (std::uint32_t tick = 0u; tick < 30u; ++tick)
		{
			room.Tick(1.f / 30.f);
			boss = room.Find_AuditionBoss();
			heldAfterNaturalCompletion = heldAfterNaturalCompletion &&
				nullptr != boss && boss->strPatternId.empty() &&
				completedPatternSequence == boss->iPatternSequence;
		}

		std::vector<S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE> lifecycleMessages;
		for (const auto& outbound : lifecycleSession->m_OutboundFrames)
		{
			PACKET_HEADER header{};
			if (!Read_Packet_Header(outbound.Bytes, header) ||
				PACKET_TYPE::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE !=
					header.ePacketType)
			{
				continue;
			}
			CPacketReader reader{ std::span<const std::uint8_t>(
				outbound.Bytes.data() + PACKET_HEADER_BYTES,
				outbound.Bytes.size() - PACKET_HEADER_BYTES) };
			S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE decoded{};
			if (Read_Message(reader, decoded) &&
				0u == reader.Get_RemainingSize())
			{
				lifecycleMessages.push_back(std::move(decoded));
			}
		}
		const bool lifecycleCorrelated =
			!lifecycleMessages.empty() &&
			VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PENDING ==
				lifecycleMessages.front().eState &&
			VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::COMPLETED_HOLD ==
				lifecycleMessages.back().eState &&
			lifecycleMessages.end() != std::find_if(
				lifecycleMessages.begin(), lifecycleMessages.end(),
				[](const S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE& lifecycle)
				{
					return VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PAUSED_FOR_REVIVE ==
						lifecycle.eState;
				}) &&
			std::all_of(lifecycleMessages.begin(), lifecycleMessages.end(),
				[roomFlowEpoch, &pinnedRevision, &request](
					const S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE& lifecycle)
				{
					return lifecycle.iRequestSequence == request.iRequestSequence &&
						lifecycle.iRoomFlowEpoch == roomFlowEpoch &&
						lifecycle.strFlowId == request.strFlowId &&
						lifecycle.strFlowRevision == request.strFlowRevision &&
						lifecycle.strStartSlotId == request.strStartSlotId &&
						3u == lifecycle.iSlotCount &&
						lifecycle.iCurrentSlotOrdinal >= 2u &&
						lifecycle.iCurrentSlotOrdinal <= 3u &&
						lifecycle.PinnedDefinitionRevision == pinnedRevision;
				});

		request.iRequestSequence = 3u;
		request.strStartSlotId = request.Slots.front().strSlotId;
		std::uint32_t stopFlowEpoch = 0u;
		GameplayDataRevision stopPinnedRevision{};
		flowReason.clear();
		const VALTAN_PATTERN_FLOW_RESULT replayQueued =
			room.Evaluate_ValtanPatternFlowStart(
				FLOW_SESSION, request, stopFlowEpoch,
				stopPinnedRevision, flowReason);
		boss = room.Find_AuditionBoss();
		const std::uint32_t stopStartSequence = nullptr == boss ?
			0u : boss->iPatternSequence;
		for (std::uint32_t tick = 0u;
			tick < 30u && room.Is_ValtanPatternFlowRunning(); ++tick)
		{
			room.Tick(1.f / 30.f);
			boss = room.Find_AuditionBoss();
			if (nullptr != boss && boss->bAutomaticPatternSequenceStepRunning)
				break;
		}
		C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT stop{};
		stop.iControlSequence = 1u;
		stop.strFlowId = request.strFlowId;
		stop.iRoomFlowEpoch = stopFlowEpoch;
		std::uint32_t reportedStopEpoch = 0u;
		GameplayDataRevision reportedStopRevision{};
		std::string stopReason;
		const VALTAN_PATTERN_FLOW_RESULT stopQueued =
			room.Evaluate_ValtanPatternFlowStopAfterCurrent(
				FLOW_SESSION, stop, reportedStopEpoch,
				reportedStopRevision, stopReason);
		std::uint32_t duplicateStopEpoch = 0u;
		GameplayDataRevision duplicateStopRevision{};
		std::string duplicateStopReason;
		const VALTAN_PATTERN_FLOW_RESULT duplicateStop =
			room.Evaluate_ValtanPatternFlowStopAfterCurrent(
				FLOW_SESSION, stop, duplicateStopEpoch,
				duplicateStopRevision, duplicateStopReason);
		C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT mismatchedStop = stop;
		mismatchedStop.strFlowId = "flow.valtan.other-contract";
		std::uint32_t mismatchedStopEpoch = 123u;
		GameplayDataRevision mismatchedStopRevision = reportedStopRevision;
		std::string mismatchedStopReason;
		const VALTAN_PATTERN_FLOW_RESULT mismatchedStopResult =
			room.Evaluate_ValtanPatternFlowStopAfterCurrent(
				FLOW_SESSION, mismatchedStop, mismatchedStopEpoch,
				mismatchedStopRevision, mismatchedStopReason);
		C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT
			mismatchedStopEpochRequest = stop;
		++mismatchedStopEpochRequest.iRoomFlowEpoch;
		std::uint32_t mismatchedStopEpochReported = 123u;
		GameplayDataRevision mismatchedStopEpochRevision = reportedStopRevision;
		std::string mismatchedStopEpochReason;
		const VALTAN_PATTERN_FLOW_RESULT mismatchedStopEpochResult =
			room.Evaluate_ValtanPatternFlowStopAfterCurrent(
				FLOW_SESSION, mismatchedStopEpochRequest,
				mismatchedStopEpochReported, mismatchedStopEpochRevision,
				mismatchedStopEpochReason);
		const bool stopDuplicateIdentityExact =
			VALTAN_PATTERN_FLOW_RESULT::DUPLICATE_IGNORED == duplicateStop &&
			duplicateStopEpoch == reportedStopEpoch &&
			duplicateStopRevision == reportedStopRevision &&
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW ==
				mismatchedStopResult &&
			0u == mismatchedStopEpoch && !mismatchedStopRevision.Is_Valid() &&
			!mismatchedStopReason.empty() &&
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW ==
				mismatchedStopEpochResult &&
			0u == mismatchedStopEpochReported &&
			!mismatchedStopEpochRevision.Is_Valid() &&
			!mismatchedStopEpochReason.empty();
		for (std::uint32_t tick = 0u;
			tick < 600u && room.Is_ValtanPatternFlowRunning(); ++tick)
		{
			room.Tick(1.f / 30.f);
		}
		boss = room.Find_AuditionBoss();
		const bool stoppedAfterOne = nullptr != boss &&
			VALTAN_PATTERN_FLOW_RESULT::QUEUED == replayQueued &&
			VALTAN_PATTERN_FLOW_RESULT::QUEUED == stopQueued &&
			stopReason.empty() && stopFlowEpoch == reportedStopEpoch &&
			stopPinnedRevision == reportedStopRevision &&
			!room.Is_ValtanPatternFlowRunning() &&
			boss->iPatternSequence ==
				((std::numeric_limits<std::uint32_t>::max)() ==
					stopStartSequence ? 1u : stopStartSequence + 1u) &&
			boss->bAutomaticPatternSequenceAuditionHold;
		stop.iControlSequence = 2u;
		std::uint32_t staleEpoch = 0u;
		GameplayDataRevision staleRevision{};
		std::string staleReason;
		const VALTAN_PATTERN_FLOW_RESULT staleStop =
			room.Evaluate_ValtanPatternFlowStopAfterCurrent(
				FLOW_SESSION, stop, staleEpoch, staleRevision, staleReason);
		tests.Require(
			activated && invalidWasTransactional && stagedSuffix &&
			duplicateKeptOneProgram && mismatchedDuplicateRejected &&
			mismatchedFlowRevisionRejected && mismatchedSuffixRejected &&
			mismatchedSlotPayloadRejected &&
			observedPause && observedResume &&
			markedStateBetweenSlots && stateSurvivedIntoSecondSlot &&
			heldAfterNaturalCompletion && lifecycleCorrelated,
			"Run a saved slot-two suffix through duplicate ordered patterns with one reset, revive pause, preserved between-slot state, terminal hold, and correlated lifecycle");
		tests.Require(
			stopDuplicateIdentityExact && stoppedAfterOne &&
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW == staleStop &&
			!staleReason.empty(),
			"Stop a running Boss Tool flow after its current occurrence and reject a stale flow epoch without starting the next slot");
		lifecycleSession->Request_Close();
	}

	{
		auto roomStorage =
			std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		constexpr SESSION_ID SESSION = 4252u;
		constexpr PLAYER_ID PLAYER = 87u;
		SERVER_PLAYER player{};
		player.iSessionId = SESSION;
		player.iPlayerId = PLAYER;
		player.iNetEntityId = 912u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		room.m_Players.emplace(PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(SESSION, PLAYER);
		room.m_ValtanAuditionSequenceBySessionId.emplace(
			SESSION, (std::numeric_limits<std::uint32_t>::max)());
		C2S_VALTAN_AUDITION_REQUEST stop{};
		stop.iRequestSequence = 1u;
		stop.eOperation = VALTAN_AUDITION_OPERATION::STOP_TIMELINE_ROW;
		std::uint32_t reportedBar = 0u;
		const VALTAN_AUDITION_RESULT wrapped =
			room.Evaluate_ValtanAudition(SESSION, stop, reportedBar);
		const VALTAN_AUDITION_RESULT retried =
			room.Evaluate_ValtanAudition(SESSION, stop, reportedBar);
		stop.iRequestSequence =
			(std::numeric_limits<std::uint32_t>::max)();
		const VALTAN_AUDITION_RESULT stale =
			room.Evaluate_ValtanAudition(SESSION, stop, reportedBar);
		stop.iRequestSequence = 0u;
		const VALTAN_AUDITION_RESULT reservedZero =
			room.Evaluate_ValtanAudition(SESSION, stop, reportedBar);
		tests.Require(
			VALTAN_AUDITION_RESULT::QUEUED == wrapped &&
			VALTAN_AUDITION_RESULT::DUPLICATE_IGNORED == retried &&
			VALTAN_AUDITION_RESULT::DUPLICATE_IGNORED == stale &&
			VALTAN_AUDITION_RESULT::DUPLICATE_IGNORED == reservedZero &&
			1u == room.m_ValtanAuditionSequenceBySessionId.at(SESSION),
			"Valtan audition request sequence accepts forward wrap and rejects stale or reserved values");
	}

	{
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		const bool activated =
			room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		constexpr std::uint32_t START_TICK =
			(std::numeric_limits<std::uint32_t>::max)() - 100u;
		constexpr std::uint32_t WRAPPED_BREAK_TICK = 375u;
		if (nullptr != boss)
		{
			boss->iPatternSequence = 1u;
			boss->strPatternId = "VALTAN_FOUR_PILLARS_105";
			boss->strPatternStageId = "RECOVERY";
		}
		room.m_bPillarAuditionCycleArmed = true;
		const bool staged = nullptr != boss &&
			room.Apply_EncounterPropStageEntry(*boss, START_TICK);
		const bool scheduledAcrossWrap = staged &&
			WRAPPED_BREAK_TICK == room.m_iPillarAuditionBreakTick;
		const bool keptBeforeWrap = room.Commit_DueEncounterProps(
			(std::numeric_limits<std::uint32_t>::max)()) &&
			WRAPPED_BREAK_TICK == room.m_iPillarAuditionBreakTick;
		const bool keptBeforeDeadline =
			room.Commit_DueEncounterProps(WRAPPED_BREAK_TICK - 1u) &&
			WRAPPED_BREAK_TICK == room.m_iPillarAuditionBreakTick;
		const bool committedAtDeadline =
			room.Commit_DueEncounterProps(WRAPPED_BREAK_TICK) &&
			0u == room.m_iPillarAuditionBreakTick;
		tests.Require(
			activated && scheduledAcrossWrap &&
			keptBeforeWrap && keptBeforeDeadline && committedAtDeadline,
			"Pillar audition deadline skips reserved zero and commits after tick wrap");
	}

	{
		/* Automatic health interrupts are suppressed by the authored sequence.
		Stage the same explicit health-bar audition override the Debug command owns,
		then let the real tick loop queue, select and run the stele mechanic. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr SESSION_ID SESSION = 4319u;
		constexpr PLAYER_ID PLAYER = 97u;
		constexpr std::uint32_t PILLAR_TRIGGER_BAR = 100u;
		constexpr std::uint32_t MAX_PILLAR_TICKS = 3000u;
		const bool activated = room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* const spawnedBoss = room.Find_AuditionBoss();

		SERVER_PLAYER player{};
		player.iSessionId = SESSION;
		player.iPlayerId = PLAYER;
		player.iNetEntityId = 915u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		/* The boss kills an ordinary body long before the 100-bar mechanic
		comes round, and a dead player is filtered out of targeting, which
		parks the boss and strands the queue. The fixture keeps one body
		alive so the product selection path is what gets measured. */
		player.iCurrentHp = 100000000u;
		player.iMaximumHp = 100000000u;
		player.iCurrentResource = 100u;
		player.iMaximumResource = 100u;
		player.isCombatReady = true;
		player.strSpawnPlacementId = "player_1";
		if (nullptr != spawnedBoss)
		{
			player.fPositionX = spawnedBoss->fPositionX + 2.f;
			player.fPositionY = spawnedBoss->fPositionY;
			player.fPositionZ = spawnedBoss->fPositionZ;
		}
		room.m_Players.emplace(PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(SESSION, PLAYER);

		/* One hit puts the boss on the boundary the mechanic watches for. */
		if (nullptr != spawnedBoss)
		{
			spawnedBoss->bAutomaticPatternSequenceAuditionOverride = true;
			spawnedBoss->iCurrentHp = CValtanBrain::Resolve_HealthBarHp(
				*spawnedBoss, PILLAR_TRIGGER_BAR);
		}

		bool sawPillarPattern = false;
		bool sawRecoveryStage = false;
		bool raised = false;
		for (std::uint32_t tick = 0u;
			tick < MAX_PILLAR_TICKS && !raised; ++tick)
		{
			room.Tick(1.f / 30.f);
			/* Top the body back up so it never leaves the target set. */
			SERVER_PLAYER& live = room.m_Players.at(PLAYER);
			live.iCurrentHp = live.iMaximumHp;
			const SERVER_WORLD_ENTITY* const boss = room.Find_AuditionBoss();
			if (nullptr != boss &&
				"VALTAN_FOUR_PILLARS_105" == boss->strPatternId)
			{
				sawPillarPattern = true;
				sawRecoveryStage = sawRecoveryStage ||
					"RECOVERY" == boss->strPatternStageId;
			}
			const auto& slots =
				room.m_EncounterPropRuntime.Get_SlotStates();
			raised = 4u == slots.size() &&
				std::all_of(slots.begin(), slots.end(),
					[](const ENCOUNTER_PROP_SLOT_STATE& slot)
					{
						return ENCOUNTER_PROP_STATE::INTACT == slot.eState;
					});
		}
		tests.Require(
			activated && nullptr != spawnedBoss && sawPillarPattern &&
			sawRecoveryStage && raised,
			"Raise the four stele through an explicit 100-bar audition and the product tick loop");
	}
	{
		/* The 100-bar cinematic is still Server movement even while the Client
		camera looks away. Drive the product room path so clip root motion cannot
		quietly add a second planar step after the leap arc has written its pose. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr SESSION_ID SESSION = 4320u;
		constexpr PLAYER_ID PLAYER = 197u;
		constexpr std::uint32_t MAX_CINEMATIC_TICKS = 450u;
		const bool activated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();

		SERVER_PLAYER player{};
		player.iSessionId = SESSION;
		player.iPlayerId = PLAYER;
		player.iNetEntityId = 1915u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = 100000000u;
		player.iMaximumHp = 100000000u;
		player.iCurrentResource = 100u;
		player.iMaximumResource = 100u;
		player.isCombatReady = true;
		player.strSpawnPlacementId = "player_1";
		const float originX = nullptr == boss ? 0.f : boss->fPositionX;
		const float originY = nullptr == boss ? 0.f : boss->fPositionY;
		const float originZ = nullptr == boss ? 0.f : boss->fPositionZ;
		player.fPositionX = originX + 2.f;
		player.fPositionY = originY;
		player.fPositionZ = originZ;
		const float lockedLandingX = player.fPositionX;
		const float lockedLandingY = player.fPositionY;
		const float lockedLandingZ = player.fPositionZ;
		room.m_Players.emplace(PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(SESSION, PLAYER);

		const BOSS_PATTERN_DEFINITION* cinematicPattern = nullptr;
		if (const std::vector<BOSS_PATTERN_DEFINITION>* patterns =
			room.m_GameplayCatalog.Find_BossPatterns("ENCOUNTER_VALTAN"))
		{
			const auto found = std::find_if(
				patterns->begin(), patterns->end(),
				[](const BOSS_PATTERN_DEFINITION& candidate)
				{
					return "VALTAN_FOUR_PILLARS_105" ==
						candidate.strPatternId;
				});
			if (patterns->end() != found)
				cinematicPattern = &(*found);
		}
		const bool authoredTimeline = nullptr != cinematicPattern &&
			100u == cinematicPattern->iTriggerHealthBar &&
			BOSS_PATTERN_MOTION_KIND::LEAP_TO_TARGET ==
				cinematicPattern->Motion.eKind &&
			"anchor.valtan.four-pillars-105.landing" ==
				cinematicPattern->Motion.strAnchorId &&
			std::abs(cinematicPattern->Motion.fApexHeight - 46.f) < 0.001f &&
			4u == cinematicPattern->Stages.size() &&
			1933u == cinematicPattern->Stages[0].iDurationMs &&
			5500u == cinematicPattern->Stages[1].iDurationMs &&
			3200u == cinematicPattern->Stages[2].iDurationMs &&
			230u == cinematicPattern->Stages[2].iHitDelayMs &&
			1300u == cinematicPattern->Stages[3].iDurationMs;

		if (nullptr != boss)
		{
			boss->bIntroPatternConsumed = true;
			boss->bAutomaticPatternSequenceAuditionOverride = true;
			boss->iLastEvaluatedHealthBar = 101u;
			boss->iCurrentHp = CValtanBrain::Resolve_HealthBarHp(*boss, 100u);
		}
		room.Tick(1.f / 30.f);
		boss = room.Find_AuditionBoss();
		const bool lockedAtStart = nullptr != boss &&
			"VALTAN_FOUR_PILLARS_105" == boss->strPatternId &&
			"TAKEOFF" == boss->strPatternStageId &&
			player.iNetEntityId == boss->iPatternTargetEntityId &&
			std::abs(boss->fLeapLandingX - lockedLandingX) < 0.001f &&
			std::abs(boss->fLeapLandingY - lockedLandingY) < 0.001f &&
			std::abs(boss->fLeapLandingZ - lockedLandingZ) < 0.001f;
		/* Moving the player after the lock must not move the landing snapshot. */
		SERVER_PLAYER& moved = room.m_Players.at(PLAYER);
		moved.fPositionX = originX - 2.f;
		moved.fPositionZ = originZ;

		bool stayedVertical = true;
		bool sawLift = false;
		bool sawTravel = false;
		bool sawDescent = false;
		bool landedExactly = false;
		bool sawRecovery = false;
		bool raised = false;
		float peakY = originY;
		for (std::uint32_t tick = 0u;
			tick < MAX_CINEMATIC_TICKS && !raised; ++tick)
		{
			room.Tick(1.f / 30.f);
			SERVER_PLAYER& live = room.m_Players.at(PLAYER);
			live.iCurrentHp = live.iMaximumHp;
			boss = room.Find_AuditionBoss();
			if (nullptr == boss ||
				"VALTAN_FOUR_PILLARS_105" != boss->strPatternId)
			{
				continue;
			}
			peakY = (std::max)(peakY, boss->fPositionY);
			if ("TAKEOFF" == boss->strPatternStageId)
			{
				stayedVertical = stayedVertical &&
					std::abs(boss->fPositionX - originX) < 0.001f &&
					std::abs(boss->fPositionZ - originZ) < 0.001f;
				sawLift = sawLift || boss->fPositionY > originY + 5.f;
			}
			else if ("YELLOW_ZONE" == boss->strPatternStageId)
			{
				const float travelX = boss->fPositionX - originX;
				const float travelZ = boss->fPositionZ - originZ;
				sawTravel = sawTravel ||
					travelX * travelX + travelZ * travelZ > 0.25f;
				sawDescent = sawDescent || boss->fPositionY < peakY - 5.f;
			}
			else if ("TARGET_CONE" == boss->strPatternStageId)
			{
				landedExactly = landedExactly ||
					(std::abs(boss->fPositionX - lockedLandingX) < 0.001f &&
					 std::abs(boss->fPositionY - lockedLandingY) < 0.001f &&
					 std::abs(boss->fPositionZ - lockedLandingZ) < 0.001f &&
					 player.iNetEntityId == boss->iPatternTargetEntityId);
			}
			else if ("RECOVERY" == boss->strPatternStageId)
			{
				sawRecovery = true;
			}
			const auto& slots = room.m_EncounterPropRuntime.Get_SlotStates();
			raised = 4u == slots.size() &&
				std::all_of(slots.begin(), slots.end(),
					[](const ENCOUNTER_PROP_SLOT_STATE& slot)
					{
						return ENCOUNTER_PROP_STATE::INTACT == slot.eState;
					});
		}
		tests.Require(
			activated && authoredTimeline && lockedAtStart && stayedVertical &&
			sawLift && peakY > originY + 44.f && sawTravel && sawDescent &&
			landedExactly && sawRecovery && raised,
			"Run the auditioned 100-bar cutscene as a vertical leap, locked-target landing and four-stele recovery on the product room path");
	}
	{
		/* The one button a map owner actually presses. PLAY_PILLAR_CYCLE puts the
		boss on the authored 100-bar edge and queues the mechanic, but only the
		real tick loop can show the four slots reaching INTACT from it. The body
		is kept alive because a dead target parks the boss and strands the queue,
		which is exactly how this mechanic goes missing in a live session. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr SESSION_ID SESSION = 4321u;
		constexpr PLAYER_ID PLAYER = 98u;
		constexpr std::uint32_t MAX_CYCLE_TICKS = 900u;
		const bool activated = room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* const cycleBoss = room.Find_AuditionBoss();

		SERVER_PLAYER player{};
		player.iSessionId = SESSION;
		player.iPlayerId = PLAYER;
		player.iNetEntityId = 916u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = 100000000u;
		player.iMaximumHp = 100000000u;
		player.iCurrentResource = 100u;
		player.iMaximumResource = 100u;
		player.isCombatReady = true;
		player.strSpawnPlacementId = "player_1";
		if (nullptr != cycleBoss)
		{
			player.fPositionX = cycleBoss->fPositionX + 2.f;
			player.fPositionY = cycleBoss->fPositionY;
			player.fPositionZ = cycleBoss->fPositionZ;
		}
		room.m_Players.emplace(PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(SESSION, PLAYER);

		/* The Debug button carries no bar: the Server has to read 100 off the
		authored pattern itself. A zero here would resolve to zero HP. */
		C2S_VALTAN_AUDITION_REQUEST cycle{};
		cycle.iRequestSequence = 1u;
		cycle.eOperation = VALTAN_AUDITION_OPERATION::PLAY_PILLAR_CYCLE;
		cycle.iTargetHealthBar = 0u;
		std::uint32_t reportedBar = 0u;
		const VALTAN_AUDITION_RESULT queued =
			room.Evaluate_ValtanAudition(SESSION, cycle, reportedBar);
		const bool survivedTheEdge = nullptr != cycleBoss &&
			0u != cycleBoss->iCurrentHp &&
			SERVER_ENTITY_ACTION::DEAD != cycleBoss->eAction;

		bool raisedFromButton = false;
		for (std::uint32_t tick = 0u;
			tick < MAX_CYCLE_TICKS && !raisedFromButton; ++tick)
		{
			room.Tick(1.f / 30.f);
			SERVER_PLAYER& live = room.m_Players.at(PLAYER);
			live.iCurrentHp = live.iMaximumHp;
			const auto& slots = room.m_EncounterPropRuntime.Get_SlotStates();
			raisedFromButton = 4u == slots.size() &&
				std::all_of(slots.begin(), slots.end(),
					[](const ENCOUNTER_PROP_SLOT_STATE& slot)
					{
						return ENCOUNTER_PROP_STATE::INTACT == slot.eState;
					});
		}
		tests.Require(
			activated && VALTAN_AUDITION_RESULT::QUEUED == queued &&
			survivedTheEdge && raisedFromButton,
			"Raise the four stele from the Debug pillar-cycle button through the real tick loop");
	}

	{
		/* A selected normal row is one isolated product pattern. Invalid
		replacements preserve the active row; a valid replacement resets it, and
		STOP resets an already-running boss immediately. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr SESSION_ID SESSION = 4401u;
		constexpr PLAYER_ID PLAYER = 201u;
		constexpr PLAYER_ID SPECTATOR = 202u;
		SERVER_PLAYER player{};
		player.iSessionId = SESSION;
		player.iPlayerId = PLAYER;
		player.iNetEntityId = 1201u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = 5500u;
		player.iMaximumHp = 5500u;
		player.iCurrentResource = 1000u;
		player.iMaximumResource = 1000u;
		player.isCombatReady = true;
		player.strSpawnPlacementId = "player_1";
		room.m_Players.emplace(PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(SESSION, PLAYER);
		SERVER_PLAYER spectator = player;
		spectator.iSessionId = 4402u;
		spectator.iPlayerId = SPECTATOR;
		spectator.iNetEntityId = 1202u;
		spectator.eAction = PLAYER_ACTION_STATE::SKILL;
		spectator.iCurrentSkillId = 34120u;
		room.m_Players.emplace(SPECTATOR, spectator);
		room.m_PlayerIdBySessionId.emplace(4402u, SPECTATOR);

		C2S_VALTAN_AUDITION_REQUEST play{};
		play.iRequestSequence = 1u;
		play.eOperation = VALTAN_AUDITION_OPERATION::PLAY_TIMELINE_ROW;
		play.iTargetHealthBar = Calculate_TestTimelineCommandId(
			"valtan.timeline.160-dash-charge-opening");
		std::uint32_t reportedBar = 0u;
		const VALTAN_AUDITION_RESULT playResult =
			room.Evaluate_ValtanAudition(SESSION, play, reportedBar);
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		const bool rowPrepared = VALTAN_AUDITION_RESULT::QUEUED == playResult &&
			nullptr != boss && 160u == reportedBar &&
			CGameRoom::VALTAN_TIMELINE_AUDITION_PHASE::READY ==
				room.m_ValtanTimelineAudition.ePhase &&
			1u == room.m_ValtanTimelineAudition.iRowIndex;
		room.Tick(1.f / 30.f);
		boss = room.Find_AuditionBoss();
		bool driverStayedPlayable = false;
		if (nullptr != boss &&
			CGameRoom::VALTAN_TIMELINE_AUDITION_PHASE::WAITING_PATTERN_FINISH ==
				room.m_ValtanTimelineAudition.ePhase)
		{
			SERVER_PLAYER& liveDriver = room.m_Players.at(PLAYER);
			liveDriver.eAction = PLAYER_ACTION_STATE::SKILL;
			liveDriver.iCurrentSkillId = 34120u;
			liveDriver.fPositionX += 1.f;
			const float movedX = liveDriver.fPositionX;
			driverStayedPlayable =
				room.Prepare_ValtanTimelineRowBeforeBrain(
					*boss, room.m_iServerTick + 1u) &&
				PLAYER_ACTION_STATE::SKILL == liveDriver.eAction &&
				34120u == liveDriver.iCurrentSkillId &&
				movedX == liveDriver.fPositionX;
		}
		const std::uint32_t epochBeforeInvalid =
			room.m_WorldDestructionRuntime.Get_EncounterEpoch();
		const std::uint32_t sequenceBeforeInvalid =
			nullptr == boss ? 0u : boss->iPatternSequence;
		play.iRequestSequence = 2u;
		play.iTargetHealthBar = Calculate_TestTimelineCommandId(
			"valtan.timeline.missing");
		const VALTAN_AUDITION_RESULT invalidResult =
			room.Evaluate_ValtanAudition(SESSION, play, reportedBar);
		const bool invalidPreservedActive =
			VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE == invalidResult &&
			epochBeforeInvalid ==
				room.m_WorldDestructionRuntime.Get_EncounterEpoch() &&
			1u == room.m_ValtanTimelineAudition.iRowIndex &&
			nullptr != boss && sequenceBeforeInvalid == boss->iPatternSequence;
		play.iRequestSequence = 3u;
		play.iTargetHealthBar = Calculate_TestTimelineCommandId(
			"valtan.timeline.160-high-jump-swing");
		const VALTAN_AUDITION_RESULT replacementResult =
			room.Evaluate_ValtanAudition(SESSION, play, reportedBar);
		boss = room.Find_AuditionBoss();
		const bool replacementReset =
			VALTAN_AUDITION_RESULT::QUEUED == replacementResult &&
			epochBeforeInvalid !=
				room.m_WorldDestructionRuntime.Get_EncounterEpoch() &&
			2u == room.m_ValtanTimelineAudition.iRowIndex &&
			nullptr != boss && SERVER_ENTITY_ACTION::IDLE == boss->eAction &&
			boss->strPatternId.empty();
		room.Tick(1.f / 30.f);
		C2S_VALTAN_AUDITION_REQUEST stop{};
		stop.iRequestSequence = 4u;
		stop.eOperation = VALTAN_AUDITION_OPERATION::STOP_TIMELINE_ROW;
		const VALTAN_AUDITION_RESULT stopResult =
			room.Evaluate_ValtanAudition(SESSION, stop, reportedBar);
		boss = room.Find_AuditionBoss();
		const std::vector<WORLD_DESTRUCTION_GROUP_STATE> resetGroupStates =
			room.m_WorldDestructionRuntime.Get_GroupStates();
		const bool arenaReset = std::all_of(
			resetGroupStates.begin(), resetGroupStates.end(),
			[](const WORLD_DESTRUCTION_GROUP_STATE& state)
			{
				return WORLD_DESTRUCTION_STATE::INTACT == state.eState;
			});
		tests.Require(
			rowPrepared && driverStayedPlayable && invalidPreservedActive &&
			replacementReset && VALTAN_AUDITION_RESULT::QUEUED == stopResult &&
			arenaReset && nullptr != boss &&
			SERVER_ENTITY_ACTION::IDLE == boss->eAction &&
			boss->strPatternId.empty() && boss->PendingPatternIds.empty() &&
			CGameRoom::VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE ==
				room.m_ValtanTimelineAudition.ePhase &&
			PLAYER_ACTION_STATE::NONE ==
				room.m_Players.at(SPECTATOR).eAction,
			"Play, safely replace and immediately stop one selectable normal timeline row");
	}

	{
		/* The wire owns the semantic row key, not its present chronological
		position. Simulate an authoring reorder and prove the original key still
		selects the original product pattern. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr SESSION_ID SESSION = 4408u;
		constexpr PLAYER_ID PLAYER = 208u;
		SERVER_PLAYER player{};
		player.iSessionId = SESSION;
		player.iPlayerId = PLAYER;
		player.iNetEntityId = 1208u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = 5500u;
		player.iMaximumHp = 5500u;
		player.iCurrentResource = 1000u;
		player.iMaximumResource = 1000u;
		player.isCombatReady = true;
		player.strSpawnPlacementId = "player_1";
		room.m_Players.emplace(PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(SESSION, PLAYER);
		auto* timeline = const_cast<VALTAN_TIMELINE_DEFINITION*>(
			room.m_GameplayCatalog.Find_ValtanTimeline("ENCOUNTER_VALTAN"));
		std::uint32_t commandId = 0u;
		std::string expectedPatternId;
		if (nullptr != timeline && timeline->Rows.size() >= 3u)
		{
			commandId = timeline->Rows[1].iCommandId;
			expectedPatternId =
				timeline->Rows[1].PatternActions.front().strPatternId;
			std::swap(timeline->Rows[1], timeline->Rows[2]);
			timeline->Rows[1].iOrdinal = 2u;
			timeline->Rows[2].iOrdinal = 3u;
		}
		C2S_VALTAN_AUDITION_REQUEST play{};
		play.iRequestSequence = 1u;
		play.eOperation = VALTAN_AUDITION_OPERATION::PLAY_TIMELINE_ROW;
		play.iTargetHealthBar = commandId;
		std::uint32_t reportedBar = 0u;
		const bool queued = VALTAN_AUDITION_RESULT::QUEUED ==
			room.Evaluate_ValtanAudition(SESSION, play, reportedBar);
		room.Tick(1.f / 30.f);
		const SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		tests.Require(
			nullptr != timeline && 0u != commandId && !expectedPatternId.empty() &&
			queued && 160u == reportedBar && nullptr != boss &&
			expectedPatternId == boss->strPatternId &&
			2u == room.m_ValtanTimelineAudition.iRowIndex,
			"Select the same semantic timeline row after its ordinal is reordered");
	}

	{
		/* Row 20 starts the 109 mechanic after only the 69 ordinary walls are
		gone. The 30 pattern-owned outer walls and six floor groups must still be
		intact on the pattern start edge. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr SESSION_ID SESSION = 4403u;
		constexpr PLAYER_ID PLAYER = 203u;
		SERVER_PLAYER player{};
		player.iSessionId = SESSION;
		player.iPlayerId = PLAYER;
		player.iNetEntityId = 1203u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = 5500u;
		player.iMaximumHp = 5500u;
		player.iCurrentResource = 1000u;
		player.iMaximumResource = 1000u;
		player.isCombatReady = true;
		player.strSpawnPlacementId = "player_1";
		room.m_Players.emplace(PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(SESSION, PLAYER);
		C2S_VALTAN_AUDITION_REQUEST play{};
		play.iRequestSequence = 1u;
		play.eOperation = VALTAN_AUDITION_OPERATION::PLAY_TIMELINE_ROW;
		play.iTargetHealthBar = Calculate_TestTimelineCommandId(
			"valtan.timeline.109-arena-break");
		std::uint32_t reportedBar = 0u;
		const bool queued = VALTAN_AUDITION_RESULT::QUEUED ==
			room.Evaluate_ValtanAudition(SESSION, play, reportedBar);
		bool started = false;
		for (std::uint32_t tick = 0u; tick < 100u && !started; ++tick)
		{
			room.Tick(1.f / 30.f);
			const SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
			started = nullptr != boss &&
				"VALTAN_ARENA_BREAK_109" == boss->strPatternId;
		}
		std::size_t intact = 0u;
		std::size_t despawned = 0u;
		for (const WORLD_DESTRUCTION_GROUP_STATE& state :
			room.m_WorldDestructionRuntime.Get_GroupStates())
		{
			intact += WORLD_DESTRUCTION_STATE::INTACT == state.eState ? 1u : 0u;
			despawned += WORLD_DESTRUCTION_STATE::DESPAWNED == state.eState ? 1u : 0u;
		}
		tests.Require(
			queued && 109u == reportedBar && started && 69u == despawned &&
			36u == intact &&
			69u == room.m_ValtanTimelineAudition.ExpectedGoneGroupIds.size(),
			"Prepare only ordinary walls before the selectable 109-bar mechanic");
	}

	{
		/* Row 35 owns one HIGH_JUMP action with repeat=2 and the 84-floor
		precondition. It must start twice, leave only the three 30-floor groups
		intact, then hold instead of advancing to another timeline row. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr SESSION_ID SESSION = 4404u;
		constexpr PLAYER_ID PLAYER = 204u;
		SERVER_PLAYER player{};
		player.iSessionId = SESSION;
		player.iPlayerId = PLAYER;
		player.iNetEntityId = 1204u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = 1000000u;
		player.iMaximumHp = 1000000u;
		player.iCurrentResource = 1000u;
		player.iMaximumResource = 1000u;
		player.isCombatReady = true;
		player.strSpawnPlacementId = "player_1";
		room.m_Players.emplace(PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(SESSION, PLAYER);
		C2S_VALTAN_AUDITION_REQUEST play{};
		play.iRequestSequence = 1u;
		play.eOperation = VALTAN_AUDITION_OPERATION::PLAY_TIMELINE_ROW;
		play.iTargetHealthBar = Calculate_TestTimelineCommandId(
			"valtan.timeline.73-high-jump-double");
		std::uint32_t reportedBar = 0u;
		const bool queued = VALTAN_AUDITION_RESULT::QUEUED ==
			room.Evaluate_ValtanAudition(SESSION, play, reportedBar);
		std::uint32_t observedSequence = 0u;
		std::size_t startCount = 0u;
		for (std::uint32_t tick = 0u; tick < 3000u; ++tick)
		{
			room.Tick(1.f / 30.f);
			const SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
			if (nullptr != boss && boss->iPatternSequence != observedSequence)
			{
				observedSequence = boss->iPatternSequence;
				if ("VALTAN_HIGH_JUMP" == boss->strPatternId)
					++startCount;
			}
			if (CGameRoom::VALTAN_TIMELINE_AUDITION_PHASE::COMPLETED_HOLD ==
				room.m_ValtanTimelineAudition.ePhase)
			{
				break;
			}
		}
		std::size_t intactFloor30 = 0u;
		std::size_t despawnedGroups = 0u;
		for (const WORLD_DESTRUCTION_GROUP_STATE& state :
			room.m_WorldDestructionRuntime.Get_GroupStates())
		{
			if (WORLD_DESTRUCTION_STATE::DESPAWNED == state.eState)
				++despawnedGroups;
			if (0u == state.strGroupId.rfind(
				"destroyable.group.valtan.floor30.", 0u) &&
				WORLD_DESTRUCTION_STATE::INTACT == state.eState)
			{
				++intactFloor30;
			}
		}
		tests.Require(
			queued && 73u == reportedBar && 2u == startCount &&
			102u == despawnedGroups && 3u == intactFloor30 &&
			1u == room.m_ValtanTimelineAudition.iActionIndex &&
			0u == room.m_ValtanTimelineAudition.iRepeatIndex &&
			CGameRoom::VALTAN_TIMELINE_AUDITION_PHASE::COMPLETED_HOLD ==
				room.m_ValtanTimelineAudition.ePhase,
			"Repeat one selected timeline action twice and hold with the 84-floor precondition");
	}

	{
		/* Row 29 composes MAGIC_CHOICE then RED_BLADE_WAVE over four staged
		pillars. The scripted runner allows only this explicitly prepared product
		prop-break path, so the real wave retires both pairs. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr SESSION_ID SESSION = 4405u;
		constexpr PLAYER_ID PLAYER = 205u;
		SERVER_PLAYER player{};
		player.iSessionId = SESSION;
		player.iPlayerId = PLAYER;
		player.iNetEntityId = 1205u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = 1000000u;
		player.iMaximumHp = 1000000u;
		player.iCurrentResource = 1000u;
		player.iMaximumResource = 1000u;
		player.isCombatReady = true;
		player.strSpawnPlacementId = "player_1";
		room.m_Players.emplace(PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(SESSION, PLAYER);
		C2S_VALTAN_AUDITION_REQUEST play{};
		play.iRequestSequence = 1u;
		play.eOperation = VALTAN_AUDITION_OPERATION::PLAY_TIMELINE_ROW;
		play.iTargetHealthBar = Calculate_TestTimelineCommandId(
			"valtan.timeline.84-magic-choice-red-blade-wave");
		std::uint32_t reportedBar = 0u;
		const bool queued = VALTAN_AUDITION_RESULT::QUEUED ==
			room.Evaluate_ValtanAudition(SESSION, play, reportedBar);
		const bool pillarsPrepared = std::all_of(
			room.m_EncounterPropRuntime.Get_SlotStates().begin(),
			room.m_EncounterPropRuntime.Get_SlotStates().end(),
			[](const ENCOUNTER_PROP_SLOT_STATE& slot)
			{
				return ENCOUNTER_PROP_STATE::INTACT == slot.eState;
			});
		std::uint32_t observedSequence = 0u;
		std::vector<std::string> startedPatterns;
		for (std::uint32_t tick = 0u; tick < 4000u; ++tick)
		{
			room.Tick(1.f / 30.f);
			const SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
			if (nullptr != boss && boss->iPatternSequence != observedSequence)
			{
				observedSequence = boss->iPatternSequence;
				startedPatterns.push_back(boss->strPatternId);
			}
			if (CGameRoom::VALTAN_TIMELINE_AUDITION_PHASE::COMPLETED_HOLD ==
				room.m_ValtanTimelineAudition.ePhase)
			{
				break;
			}
		}
		for (std::uint32_t tick = 0u; tick < 12u; ++tick)
			room.Tick(1.f / 30.f);
		const std::vector<std::string> expectedPatterns{
			"VALTAN_MAGIC_CHOICE", "VALTAN_RED_BLADE_WAVE" };
		const bool pillarsRetired = std::all_of(
			room.m_EncounterPropRuntime.Get_SlotStates().begin(),
			room.m_EncounterPropRuntime.Get_SlotStates().end(),
			[](const ENCOUNTER_PROP_SLOT_STATE& slot)
			{
				return ENCOUNTER_PROP_STATE::HIDDEN == slot.eState;
			});
		tests.Require(
			queued && 84u == reportedBar && pillarsPrepared &&
			expectedPatterns == startedPatterns && pillarsRetired &&
			2u == room.m_ValtanTimelineAudition.iActionIndex &&
			CGameRoom::VALTAN_TIMELINE_AUDITION_PHASE::COMPLETED_HOLD ==
				room.m_ValtanTimelineAudition.ePhase,
			"Run a composite selected row and let red blade break its four prepared pillars");
	}

	{
		/* An invalid second action is rejected during staging, before boss
		activation, wall reset or pillar spawn. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr SESSION_ID SESSION = 4406u;
		constexpr PLAYER_ID PLAYER = 206u;
		SERVER_PLAYER player{};
		player.iSessionId = SESSION;
		player.iPlayerId = PLAYER;
		player.iNetEntityId = 1206u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = 5500u;
		player.iMaximumHp = 5500u;
		player.isCombatReady = true;
		player.strSpawnPlacementId = "player_1";
		room.m_Players.emplace(PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(SESSION, PLAYER);
		auto* timeline = const_cast<VALTAN_TIMELINE_DEFINITION*>(
			room.m_GameplayCatalog.Find_ValtanTimeline("ENCOUNTER_VALTAN"));
		std::string savedPatternId;
		if (nullptr != timeline && timeline->Rows.size() >= 29u &&
			timeline->Rows[28].PatternActions.size() >= 2u)
		{
			savedPatternId =
				timeline->Rows[28].PatternActions[1].strPatternId;
			timeline->Rows[28].PatternActions[1].strPatternId =
				"VALTAN_UNKNOWN_TEST_PATTERN";
		}
		const std::uint32_t destructionEpoch =
			room.m_WorldDestructionRuntime.Get_EncounterEpoch();
		const std::uint32_t propEpoch =
			room.m_EncounterPropRuntime.Get_EncounterEpoch();
		C2S_VALTAN_AUDITION_REQUEST play{};
		play.iRequestSequence = 1u;
		play.eOperation = VALTAN_AUDITION_OPERATION::PLAY_TIMELINE_ROW;
		play.iTargetHealthBar = Calculate_TestTimelineCommandId(
			"valtan.timeline.84-magic-choice-red-blade-wave");
		std::uint32_t reportedBar = 0u;
		const VALTAN_AUDITION_RESULT result =
			room.Evaluate_ValtanAudition(SESSION, play, reportedBar);
		const bool propsStayedHidden = std::all_of(
			room.m_EncounterPropRuntime.Get_SlotStates().begin(),
			room.m_EncounterPropRuntime.Get_SlotStates().end(),
			[](const ENCOUNTER_PROP_SLOT_STATE& slot)
			{
				return ENCOUNTER_PROP_STATE::HIDDEN == slot.eState;
			});
		if (!savedPatternId.empty())
			timeline->Rows[28].PatternActions[1].strPatternId = savedPatternId;
		tests.Require(
			nullptr != timeline && !savedPatternId.empty() &&
			VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE == result &&
			nullptr == room.Find_AuditionBoss() &&
			destructionEpoch ==
				room.m_WorldDestructionRuntime.Get_EncounterEpoch() &&
			propEpoch == room.m_EncounterPropRuntime.Get_EncounterEpoch() &&
			propsStayedHidden &&
			CGameRoom::VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE ==
				room.m_ValtanTimelineAudition.ePhase,
			"Rollback an invalid composite pillar and red-blade row before any live commit");
	}

	{
		/* Row 43 is after both floor collapses and asks for four intact pillars.
		Every destruction group must be gone before its first action starts. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr SESSION_ID SESSION = 4407u;
		constexpr PLAYER_ID PLAYER = 207u;
		SERVER_PLAYER player{};
		player.iSessionId = SESSION;
		player.iPlayerId = PLAYER;
		player.iNetEntityId = 1207u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = 1000000u;
		player.iMaximumHp = 1000000u;
		player.iCurrentResource = 1000u;
		player.iMaximumResource = 1000u;
		player.isCombatReady = true;
		player.strSpawnPlacementId = "player_1";
		room.m_Players.emplace(PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(SESSION, PLAYER);
		C2S_VALTAN_AUDITION_REQUEST play{};
		play.iRequestSequence = 1u;
		play.eOperation = VALTAN_AUDITION_OPERATION::PLAY_TIMELINE_ROW;
		play.iTargetHealthBar = Calculate_TestTimelineCommandId(
			"valtan.timeline.29-ledge-roar");
		std::uint32_t reportedBar = 0u;
		const bool queued = VALTAN_AUDITION_RESULT::QUEUED ==
			room.Evaluate_ValtanAudition(SESSION, play, reportedBar);
		bool started = false;
		for (std::uint32_t tick = 0u; tick < 100u && !started; ++tick)
		{
			room.Tick(1.f / 30.f);
			const SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
			started = nullptr != boss && "VALTAN_LEDGE_ROAR" == boss->strPatternId;
		}
		const std::vector<WORLD_DESTRUCTION_GROUP_STATE> lateGroupStates =
			room.m_WorldDestructionRuntime.Get_GroupStates();
		const bool everyGroupGone = std::all_of(
			lateGroupStates.begin(), lateGroupStates.end(),
			[](const WORLD_DESTRUCTION_GROUP_STATE& state)
			{
				return WORLD_DESTRUCTION_STATE::DESPAWNED == state.eState;
			});
		const bool fourPillarsIntact =
			4u == room.m_EncounterPropRuntime.Get_SlotStates().size() &&
			std::all_of(
				room.m_EncounterPropRuntime.Get_SlotStates().begin(),
				room.m_EncounterPropRuntime.Get_SlotStates().end(),
				[](const ENCOUNTER_PROP_SLOT_STATE& slot)
				{
					return ENCOUNTER_PROP_STATE::INTACT == slot.eState;
				});
		tests.Require(
			queued && 29u == reportedBar && started && everyGroupGone &&
			fourPillarsIntact &&
			lateGroupStates.size() ==
				room.m_ValtanTimelineAudition.ExpectedGoneGroupIds.size(),
			"Prepare both 84 and 30 floor collapses plus four pillars before a late timeline row");
	}

	{
		/* These four buttons are not isolated row playback. Each one installs the
		completed one-shot mechanics above its boundary, stages the matching arena,
		then releases the ordinary Brain so the boundary and later rotations keep
		running. */
		struct PAGE_CASE final
		{
			const char* pRowId;
			std::uint32_t iHealthBar;
			std::uint8_t iInitialPhase;
			const char* pExpectedPatternId;
		};
		const std::array<PAGE_CASE, 4u> cases{
			PAGE_CASE{ "valtan.timeline.160-entrance-whirlwind", 160u, 1u,
				"VALTAN_ENTRANCE_WHIRLWIND" },
			PAGE_CASE{ "valtan.timeline.109-arena-break", 109u, 1u,
				"VALTAN_ARENA_BREAK_109" },
			PAGE_CASE{ "valtan.timeline.62-center-grab-counter", 62u, 2u,
				"VALTAN_CENTER_GRAB_COUNTER_64" },
			PAGE_CASE{ "valtan.timeline.14-ghost-transition", 14u, 2u,
				"VALTAN_GHOST_TRANSITION_15" } };
		bool allPagesStarted = true;
		for (std::size_t index = 0u; index < cases.size(); ++index)
		{
			const PAGE_CASE& page = cases[index];
			CGameRoom room{ WORLD_ID::VALTAN_ARENA };
			const SESSION_ID session =
				4500u + static_cast<SESSION_ID>(index);
			const PLAYER_ID playerId =
				300u + static_cast<PLAYER_ID>(index);
			SERVER_PLAYER player{};
			player.iSessionId = session;
			player.iPlayerId = playerId;
			player.iNetEntityId =
				1300u + static_cast<NET_ENTITY_ID>(index);
			player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			player.iCurrentHp = 100000000u;
			player.iMaximumHp = 100000000u;
			player.iCurrentResource = 1000u;
			player.iMaximumResource = 1000u;
			player.isCombatReady = true;
			player.strSpawnPlacementId = "player_1";
			room.m_Players.emplace(playerId, player);
			room.m_PlayerIdBySessionId.emplace(session, playerId);

			C2S_VALTAN_AUDITION_REQUEST request{};
			request.iRequestSequence = 1u;
			request.eOperation = VALTAN_AUDITION_OPERATION::START_FIGHT_PAGE;
			request.iTargetHealthBar =
				Calculate_TestTimelineCommandId(page.pRowId);
			std::uint32_t reportedBar = 0u;
			const VALTAN_AUDITION_RESULT result =
				room.Evaluate_ValtanAudition(session, request, reportedBar);
			SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
			const std::vector<BOSS_PATTERN_DEFINITION>* patterns = nullptr == boss ?
				nullptr : room.m_GameplayCatalog.Find_BossPatterns(
					boss->strEncounterId);
			std::size_t expectedCompletedCount = 0u;
			if (nullptr != patterns)
			{
				expectedCompletedCount = static_cast<std::size_t>(std::count_if(
					patterns->begin(), patterns->end(),
					[&page](const BOSS_PATTERN_DEFINITION& pattern)
					{
						return BOSS_PATTERN_SELECTION::HEALTH_BAR ==
								pattern.eSelection &&
							pattern.iTriggerHealthBar > page.iHealthBar;
					}));
			}
			const bool staged = VALTAN_AUDITION_RESULT::QUEUED == result &&
				page.iHealthBar == reportedBar && nullptr != boss &&
				page.iInitialPhase == boss->iPhase &&
				expectedCompletedCount == boss->MechanicOccurrences.size() &&
				std::all_of(
					boss->MechanicOccurrences.begin(),
					boss->MechanicOccurrences.end(),
					[](const SERVER_BOSS_MECHANIC_OCCURRENCE& occurrence)
					{
						return SERVER_BOSS_MECHANIC_STATE::COMPLETED ==
							occurrence.eState;
					});

			bool started = false;
			for (std::uint32_t tick = 0u; tick < 120u && !started; ++tick)
			{
				room.Tick(1.f / 30.f);
				SERVER_PLAYER& livePlayer = room.m_Players.at(playerId);
				livePlayer.iCurrentHp = livePlayer.iMaximumHp;
				boss = room.Find_AuditionBoss();
				started = nullptr != boss &&
					page.pExpectedPatternId == boss->strPatternId;
			}
			const bool pageStarted = staged && started &&
				nullptr != boss && !boss->bScriptedPatternPlayback &&
				!room.m_ValtanFightPageStart.Is_Active() &&
				CGameRoom::VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE ==
					room.m_ValtanTimelineAudition.ePhase;
			allPagesStarted = allPagesStarted && pageStarted;
		}
		tests.Require(
			allPagesStarted,
			"Start all four Valtan fight pages with completed prior mechanics and release normal Brain playback");
	}

	{
		/* The wire key must resolve to one of the four reviewed mechanic rows. A
		normal 130-bar timeline row cannot become an accidental fifth page. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr SESSION_ID SESSION = 4510u;
		constexpr PLAYER_ID PLAYER = 310u;
		SERVER_PLAYER player{};
		player.iSessionId = SESSION;
		player.iPlayerId = PLAYER;
		player.iNetEntityId = 1310u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = 5500u;
		player.iMaximumHp = 5500u;
		player.isCombatReady = true;
		player.strSpawnPlacementId = "player_1";
		room.m_Players.emplace(PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(SESSION, PLAYER);
		C2S_VALTAN_AUDITION_REQUEST request{};
		request.iRequestSequence = 1u;
		request.eOperation = VALTAN_AUDITION_OPERATION::START_FIGHT_PAGE;
		request.iTargetHealthBar = Calculate_TestTimelineCommandId(
			"valtan.timeline.130-whirlwind-wall-break");
		std::uint32_t reportedBar = 0u;
		const VALTAN_AUDITION_RESULT result =
			room.Evaluate_ValtanAudition(SESSION, request, reportedBar);
		tests.Require(
			VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE == result &&
			nullptr == room.Find_AuditionBoss() &&
			!room.m_ValtanFightPageStart.Is_Active(),
			"Reject a non-boundary timeline row as a Valtan fight page start");
	}

#endif

#ifndef _DEBUG
	{
		/* The wire contract remains known in Release so a Debug Client receives
		an explicit verdict, but neither start nor control may stage room state. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START start{};
		start.iRequestSequence = 1u;
		start.strBossPlacementId = "boss.valtan.center";
		start.strFlowId = "flow.valtan.release-contract";
		start.strFlowRevision = std::string(64u, 'a');
		start.strStartSlotId = "flow.slot.000001";
		start.iInterStepPursuitMs =
			MIN_VALTAN_PATTERN_FLOW_INTER_STEP_PURSUIT_MS;
		start.Slots = {
			{ "flow.slot.000001", "VALTAN_WHIRLWIND" }
		};
		const std::string statusBefore = room.m_strStatus;
		const std::size_t worldEntityCountBefore = room.m_WorldEntities.size();
		std::uint32_t reportedEpoch = 123u;
		GameplayDataRevision reportedRevision{};
		reportedRevision.Bytes.front() = 1u;
		std::string reason;
		const VALTAN_PATTERN_FLOW_RESULT startResult =
			room.Evaluate_ValtanPatternFlowStart(
				1u, start, reportedEpoch, reportedRevision, reason);

		C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT stop{};
		stop.iControlSequence = 1u;
		stop.strFlowId = start.strFlowId;
		stop.iRoomFlowEpoch = 1u;
		std::uint32_t stopEpoch = 123u;
		GameplayDataRevision stopRevision{};
		stopRevision.Bytes.front() = 1u;
		std::string stopReason;
		const VALTAN_PATTERN_FLOW_RESULT stopResult =
			room.Evaluate_ValtanPatternFlowStopAfterCurrent(
				1u, stop, stopEpoch, stopRevision, stopReason);

		tests.Require(
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_RELEASE_BUILD == startResult &&
			0u == reportedEpoch && !reportedRevision.Is_Valid() &&
			!reason.empty() &&
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_RELEASE_BUILD == stopResult &&
			0u == stopEpoch && !stopRevision.Is_Valid() &&
			!stopReason.empty() && statusBefore == room.m_strStatus &&
			worldEntityCountBefore == room.m_WorldEntities.size(),
			"Reject Boss Tool pattern-flow start and stop commands in a Release Server without staging room state");
	}
#endif

	{
		/* A product revive is the authoritative signal that the only dead player
		is back in the raid. Drive the real room handler and tick instead of
		setting isCombatReady by hand, because the latter used to let the Brain
		test pass while the live room kept Valtan latched in IDLE. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr PLAYER_ID REVIVED_PLAYER = 211u;
		constexpr SESSION_ID REVIVED_SESSION = 4411u;
		constexpr NET_ENTITY_ID REVIVED_ENTITY = 1211u;
		const bool activated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();

		SERVER_PLAYER player{};
		player.iSessionId = REVIVED_SESSION;
		player.iPlayerId = REVIVED_PLAYER;
		player.iNetEntityId = REVIVED_ENTITY;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.strNickName = "PartyWipeRevive";
		player.strSpawnPlacementId = "player_1";
		player.iCurrentHp = 0u;
		player.iMaximumHp = 5500u;
		player.iCurrentResource = 0u;
		player.iMaximumResource = 1000u;
		player.eAction = PLAYER_ACTION_STATE::DEAD;
		player.isCombatReady = false;
		if (nullptr != boss)
		{
			player.fPositionX = boss->fPositionX + 2.f;
			player.fPositionY = boss->fPositionY;
			player.fPositionZ = boss->fPositionZ;
		}
		room.m_Players.emplace(REVIVED_PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(REVIVED_SESSION, REVIVED_PLAYER);
		room.m_PlayerIdByEntityId.emplace(REVIVED_ENTITY, REVIVED_PLAYER);

		if (nullptr != boss)
		{
			boss->bIntroPatternConsumed = true;
			boss->eAction = SERVER_ENTITY_ACTION::IDLE;
			boss->strPatternId.clear();
			boss->strPatternStageId.clear();
			boss->strActionId.clear();
			boss->PendingPatternIds.clear();
			boss->TriggeredPatternIds.push_back("VALTAN_FLOOR_WIPE_130");
			SERVER_BOSS_MECHANIC_OCCURRENCE wiped{};
			wiped.strPatternId = "VALTAN_FLOOR_WIPE_130";
			wiped.PinnedDefinitionRevision =
				room.m_GameplayCatalog.Get_ActiveRevision();
			wiped.eState =
				SERVER_BOSS_MECHANIC_STATE::FAILED_REQUIRES_RESET;
			wiped.eFailure = SERVER_BOSS_MECHANIC_FAILURE::NO_VALID_TARGET;
			wiped.iTriggerHealthBar = 130u;
			wiped.iQueuedTick = 10u;
			wiped.iStartedTick = 11u;
			wiped.iFinishedTick = 12u;
			wiped.iPatternSequence = 1u;
			boss->MechanicOccurrences.push_back(std::move(wiped));
			boss->bMechanicLedgerRequiresReset = true;
		}

		C2S_REVIVE_PLAYER revive{};
		revive.iClientSequence = 1u;
		room.Handle_RevivePlayer(REVIVED_SESSION, revive);
		const bool admittedOnRevive =
			room.m_Players.at(REVIVED_PLAYER).isCombatReady;
		room.Tick(1.f / 30.f);
		boss = room.Find_AuditionBoss();
		const auto occurrence = nullptr == boss ?
			std::vector<SERVER_BOSS_MECHANIC_OCCURRENCE>::const_iterator{} :
			std::find_if(
				boss->MechanicOccurrences.cbegin(),
				boss->MechanicOccurrences.cend(),
				[](const SERVER_BOSS_MECHANIC_OCCURRENCE& value)
				{
					return "VALTAN_FLOOR_WIPE_130" == value.strPatternId;
				});
		const bool occurrenceCompleted = nullptr != boss &&
			boss->MechanicOccurrences.cend() != occurrence &&
			SERVER_BOSS_MECHANIC_STATE::COMPLETED == occurrence->eState;
		tests.Require(
			activated && nullptr != boss && admittedOnRevive &&
			!boss->bMechanicLedgerRequiresReset && occurrenceCompleted &&
			SERVER_ENTITY_ACTION::IDLE != boss->eAction,
			"Resume the product Valtan rotation on the authoritative revive tick after a party wipe");
	}

	{
		/* The arena floor collapse is the only authored thing that takes ground
		away from a player, and it kills. Navigation owns where the hole is; the
		room owns the descent and the death tick. This runs in both configurations
		because a fall is product gameplay, not a Debug audition. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr PLAYER_ID FALL_PLAYER = 79u;
		constexpr SESSION_ID FALL_SESSION = 4243u;
		/* Exclusively inside navregion.valtan.floor30.rail.7000000000000000001:
		no other authored region owns this cell, so the stage-B collapse is the
		only thing that can open it. */
		constexpr float FALL_RAIL_X = 155.25f;
		constexpr float FALL_RAIL_Z = -107.25f;
		/* The same-level projection reaches at most twenty cells of 0.5 m, so a
		revive that steps out of the hole cannot land further than this away no
		matter how the floor is repainted. The arena entry spawn is over a hundred
		metres away, which is exactly the regression this bound is here to catch. */
		constexpr float ARENA_REVIVE_MAX_METERS = 10.5f;
		/* The arena core the audition bait stands on. It belongs to no collapse
		region at all and has to stay solid through both stages. */
		constexpr float ARENA_CORE_X = 154.296f;
		constexpr float ARENA_CORE_Z = -125.219f;
		const std::string railConditionId =
			"condition.valtan.floor30.rail.7000000000000000001.collapsed";

		std::string voidStatus;
		const bool rejectedObstaclePolarity =
			!room.m_ServerNavigation.Set_VoidConditions(
				{ "condition.valtan.entrance.frontwallA.destroyed" }, voidStatus);
		tests.Require(
			room.Is_Ready() && room.m_ServerNavigation.Is_Loaded() &&
			rejectedObstaclePolarity &&
			!room.m_ServerNavigation.Is_PointInVoidRegion(
				FALL_RAIL_X, FALL_RAIL_Z) &&
			!room.m_ServerNavigation.Is_PointInVoidRegion(
				ARENA_CORE_X, ARENA_CORE_Z),
			"Refuse a wall condition as a fall region and start the arena with no holes");

		SERVER_NAV_POINT railGround{};
		const bool projectedRail = room.m_ServerNavigation.Project_Point(
			FALL_RAIL_X, FALL_RAIL_Z, railGround);
		SERVER_PLAYER faller{};
		faller.iPlayerId = FALL_PLAYER;
		faller.iNetEntityId = 902u;
		faller.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		faller.iCurrentHp = 1000u;
		faller.iMaximumHp = 1000u;
		faller.isCombatReady = true;
		faller.strSpawnPlacementId = "player_1";
		faller.fPositionX = FALL_RAIL_X;
		faller.fPositionY = railGround.y;
		faller.fPositionZ = FALL_RAIL_Z;
		room.m_Players.emplace(FALL_PLAYER, faller);
		room.m_PlayerIdBySessionId.emplace(FALL_SESSION, FALL_PLAYER);

		room.Tick(1.f / 30.f);
		const SERVER_PLAYER& standing = room.m_Players.at(FALL_PLAYER);
		tests.Require(
			projectedRail &&
			PLAYER_ACTION_STATE::NONE == standing.eAction &&
			1000u == standing.iCurrentHp &&
			standing.fPositionY == railGround.y,
			"Leave a player standing on an intact stage-B rail sector alone");

		/* This is the exact navigation edge the collapse commit applies. Flipping
		it directly keeps the fall contract independent of the pattern schedule
		the destruction bootstrap tests already pin. */
		std::vector<SERVER_NAVIGATION_CONDITION_CHANGE> collapseChanges;
		collapseChanges.push_back({ railConditionId, true });
		SERVER_NAVIGATION_CONDITION_STAGE collapseStage{};
		std::string collapseStatus;
		const bool openedHole =
			room.m_ServerNavigation.Prepare_ConditionChanges(
				collapseChanges, collapseStage, collapseStatus);
		if (openedHole)
			room.m_ServerNavigation.Commit_ConditionChanges(
				std::move(collapseStage));
		tests.Require(
			openedHole &&
			room.m_ServerNavigation.Is_PointInVoidRegion(
				FALL_RAIL_X, FALL_RAIL_Z) &&
			!room.m_ServerNavigation.Is_PointInVoidRegion(
				ARENA_CORE_X, ARENA_CORE_Z),
			"Open a fall region only where the stage-B rail sector collapsed");

		const float heightBeforeFall =
			room.m_Players.at(FALL_PLAYER).fPositionY;
		room.Tick(1.f / 30.f);
		const SERVER_PLAYER& falling = room.m_Players.at(FALL_PLAYER);
		tests.Require(
			PLAYER_ACTION_STATE::FALLING == falling.eAction &&
			0u != falling.iActionStartTick &&
			0u != falling.iFallDeathTick &&
			falling.fPositionY < heightBeforeFall &&
			0u != falling.iCurrentHp &&
			!falling.isCombatReady && !falling.hasMoveGoal &&
			falling.MovePath.empty(),
			"Drop the player into a falling state on the tick after the rail sector collapses");

		/* Falling is not dying yet. The boss must not be able to reach the body
		on the way down, which is what the not-combat-ready flag above buys. */
		for (std::uint32_t tick = 0u; tick < 44u; ++tick)
			room.Tick(1.f / 30.f);
		const bool stillFallingBeforeDeadline =
			PLAYER_ACTION_STATE::FALLING ==
				room.m_Players.at(FALL_PLAYER).eAction &&
			0u != room.m_Players.at(FALL_PLAYER).iCurrentHp;
		room.Tick(1.f / 30.f);
		const SERVER_PLAYER& landed = room.m_Players.at(FALL_PLAYER);
		tests.Require(
			stillFallingBeforeDeadline &&
			PLAYER_ACTION_STATE::DEAD == landed.eAction &&
			0u == landed.iCurrentHp &&
			0u == landed.iFallDeathTick,
			"Kill a falling player at the authored death tick and not before it");

		C2S_REVIVE_PLAYER revive{};
		revive.iClientSequence = 1u;
		room.Handle_RevivePlayer(FALL_SESSION, revive);
		const SERVER_PLAYER& revived = room.m_Players.at(FALL_PLAYER);
		tests.Require(
			PLAYER_ACTION_STATE::NONE == revived.eAction &&
			revived.iCurrentHp == revived.iMaximumHp &&
			revived.isCombatReady &&
			0u == revived.iFallDeathTick &&
			room.m_ServerNavigation.Is_PointWalkableExact(
				revived.fPositionX, revived.fPositionZ) &&
			!room.m_ServerNavigation.Is_PointInVoidRegion(
				revived.fPositionX, revived.fPositionZ),
			"Revive a fall death on walkable ground and immediately restore Valtan target admission");

		/* The arena progression triggers are room-wide triggerOnce, so a revive
		that returns to the entry spawn leaves the player outside a boss fight no
		surviving trigger can let them back into. */
		const float reviveDeltaX = revived.fPositionX - FALL_RAIL_X;
		const float reviveDeltaZ = revived.fPositionZ - FALL_RAIL_Z;
		tests.Require(
			reviveDeltaX * reviveDeltaX + reviveDeltaZ * reviveDeltaZ <=
				ARENA_REVIVE_MAX_METERS * ARENA_REVIVE_MAX_METERS,
			"Revive a fall death beside the hole instead of at the arena entry spawn");

		room.Tick(1.f / 30.f);
		tests.Require(
			PLAYER_ACTION_STATE::NONE ==
				room.m_Players.at(FALL_PLAYER).eAction,
			"Keep a revived player standing instead of falling again");
	}
	{
		/* The 109 leap and roar now have exact Product clips, but their world arc
		remains Server state. The authoritative subwindows must carry the body
		from TAKEOFF through DROP to the authored placement before the joined
		IMPACT -> WIDE_REVEAL roar sequence can begin. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr PLAYER_ID LEAP_PLAYER = 78u;
		const bool activated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* leapBoss = room.Find_AuditionBoss();
		/* Drive one exact pattern: stage the encounter intro as already
		consumed so the first-appearance sweep is not the first sequence. */
		if (nullptr != leapBoss)
		{
			leapBoss->bIntroPatternConsumed = true;
			leapBoss->bAutomaticPatternSequenceAuditionOverride = true;
		}

		SERVER_PLAYER leapPlayer{};
		leapPlayer.iPlayerId = LEAP_PLAYER;
		leapPlayer.iNetEntityId = 901u;
		leapPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		leapPlayer.iCurrentHp = 1000u;
		leapPlayer.iMaximumHp = 1000u;
		leapPlayer.isCombatReady = true;
		if (nullptr != leapBoss)
		{
			leapPlayer.fPositionX = leapBoss->fPositionX + 2.f;
			leapPlayer.fPositionY = leapBoss->fPositionY;
			leapPlayer.fPositionZ = leapBoss->fPositionZ;
		}
		room.m_Players.emplace(LEAP_PLAYER, leapPlayer);

		/* The 109 leap lands on the pattern's compiled anchor, which is the
		measured centre of the outer ring. The boss spawns on that same centre,
		so the anchor is checked against the authored coordinate rather than
		against its distance from the spawn. */
		const BOSS_PATTERN_MOTION* leapMotion = nullptr;
		if (const std::vector<BOSS_PATTERN_DEFINITION>* leapPatterns =
			room.m_GameplayCatalog.Find_BossPatterns("ENCOUNTER_VALTAN"))
		{
			for (const BOSS_PATTERN_DEFINITION& candidate : *leapPatterns)
			{
				if ("VALTAN_ARENA_BREAK_109" == candidate.strPatternId)
				{
					leapMotion = &candidate.Motion;
					break;
				}
			}
		}
		const float anchorX = nullptr == leapMotion ? 0.f : leapMotion->fLandingX;
		const float anchorY = nullptr == leapMotion ? 0.f : leapMotion->fLandingY;
		const float anchorZ = nullptr == leapMotion ? 0.f : leapMotion->fLandingZ;
		const float groundY = anchorY;
		tests.Require(
			activated && nullptr != leapBoss && nullptr != leapMotion &&
			BOSS_PATTERN_MOTION_KIND::LEAP_TO_ANCHOR == leapMotion->eKind &&
			"anchor.valtan.arena-break-109.landing" == leapMotion->strAnchorId &&
			leapMotion->fApexHeight > 0.f &&
			0u == leapMotion->iTakeoffStartMs &&
			900u == leapMotion->iTakeoffEndMs &&
			0u == leapMotion->iTravelStartMs &&
			700u == leapMotion->iTravelEndMs &&
			std::abs(anchorX - 156.03f) < 0.01f &&
			std::abs(anchorZ + 122.06f) < 0.01f,
			"Compile the 109 landing anchor on the authored outer-ring centre");

		/* Drive the real pattern rather than assigning stages by hand, so the
		arc is exercised through the same edges the room replicates. */
		if (nullptr != leapBoss)
		{
			leapBoss->fPositionX = leapBoss->fSpawnPositionX + 6.f;
			leapBoss->fPositionZ = leapBoss->fSpawnPositionZ + 6.f;
			leapBoss->iCurrentHp =
				CValtanBrain::Resolve_HealthBarHp(*leapBoss, 109u);
			leapBoss->iLastEvaluatedHealthBar = 110u;
		}
		CValtanBrain leapBrain;
		std::vector<DAMAGE_EVENT> leapDamage;
		std::uint32_t leapTick = 600u;
		const auto tickLeap = [&](const std::uint32_t count)
		{
			for (std::uint32_t index = 0u; index < count; ++index)
			{
				if (nullptr == leapBoss)
					return;
				leapBrain.Update(
					*leapBoss, room.m_Players, room.m_GameplayCatalog,
					room.m_ServerNavigation, 1.f / 30.f, leapTick++, {},
					leapDamage);
			}
		};

		tickLeap(1u);
		tests.Require(
			nullptr != leapBoss &&
			"VALTAN_ARENA_BREAK_109" ==
				(nullptr == leapBoss ? std::string{} : leapBoss->strPatternId) &&
			"TAKEOFF" == (nullptr == leapBoss ?
				std::string{} : leapBoss->strPatternStageId),
			"Begin the 109 phase transition on the authored crossing");

		/* Half of the 900ms TAKEOFF stage at 30Hz. */
		tickLeap(13u);
		tests.Require(
			nullptr != leapBoss &&
			leapBoss->fPositionY > groundY + 1.f,
			"Lift Valtan off the ground during the authored TAKEOFF stage");

		/* Follow the rest of TAKEOFF and all of DROP one tick at a time so the
		arc itself is checked, not just its endpoints: it has to reach the
		authored apex and come all the way back down to the floor. */
		float peakY = nullptr == leapBoss ? 0.f : leapBoss->fPositionY;
		float peakPlanarError = 0.f;
		for (std::uint32_t index = 0u; index < 35u; ++index)
		{
			tickLeap(1u);
			if (nullptr == leapBoss)
				break;
			if (leapBoss->fPositionY > peakY)
				peakY = leapBoss->fPositionY;
			if ("DROP" == leapBoss->strPatternStageId)
			{
				const float dx = leapBoss->fPositionX - anchorX;
				const float dz = leapBoss->fPositionZ - anchorZ;
				peakPlanarError = (std::max)(
					peakPlanarError, std::sqrt(dx * dx + dz * dz));
			}
		}
		tests.Require(
			nullptr != leapBoss && nullptr != leapMotion &&
			std::abs(peakY - (groundY + leapMotion->fApexHeight)) < 0.5f &&
			peakPlanarError > 1.f &&
			leapBoss->fPositionY <= groundY + 0.001f,
			"Carry Valtan through the authored apex and back down to the floor");

		/* TAKEOFF (27 ticks) plus DROP (21) lands inside the 12-tick IMPACT. */
		tickLeap(1u);
		const bool landedExactly = nullptr != leapBoss &&
			"IMPACT" == leapBoss->strPatternStageId &&
			std::abs(leapBoss->fPositionX - anchorX) < 0.001f &&
			std::abs(leapBoss->fPositionY - anchorY) < 0.001f &&
			std::abs(leapBoss->fPositionZ - anchorZ) < 0.001f;
		tests.Require(
			landedExactly,
			"Land the 109 leap exactly on the compiled anchor at IMPACT");
	}

	{
		/* The high jump used to take off and land on its own feet because it
		owned no motion at all. It now follows the target it locked, so the arc
		has to end where that player stood and not where the boss started. */
		const std::vector<BOSS_PATTERN_DEFINITION>* leapPatterns =
			catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const BOSS_PATTERN_DEFINITION* highJump = nullptr;
		if (nullptr != leapPatterns)
		{
			const auto found = std::find_if(
				leapPatterns->begin(), leapPatterns->end(),
				[](const BOSS_PATTERN_DEFINITION& candidate)
				{ return candidate.strPatternId == "VALTAN_HIGH_JUMP"; });
			if (leapPatterns->end() != found)
				highJump = &(*found);
		}
		std::map<PLAYER_ID, SERVER_PLAYER> leapArcPlayers;
		SERVER_PLAYER leapArcTarget{};
		leapArcTarget.iPlayerId = 91;
		leapArcTarget.iNetEntityId = 9100;
		leapArcTarget.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		leapArcTarget.iCurrentHp = 1000;
		leapArcTarget.iMaximumHp = 1000;
		leapArcTarget.isCombatReady = true;
		leapArcTarget.fPositionX = 160.5f;
		leapArcTarget.fPositionY = 22.97f;
		leapArcTarget.fPositionZ = -125.5f;
		leapArcPlayers.emplace(leapArcTarget.iPlayerId, leapArcTarget);

		SERVER_WORLD_ENTITY leapArcBoss{};
		leapArcBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		leapArcBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		leapArcBoss.strArchetypeId = "BOSS_VALTAN";
		leapArcBoss.strEncounterId = "ENCOUNTER_VALTAN";
		leapArcBoss.iCurrentHp = 60000;
		leapArcBoss.iMaximumHp = 60000;
		leapArcBoss.iMaximumHealthBars = 160;
		leapArcBoss.iLastEvaluatedHealthBar = 160;
		leapArcBoss.iPhaseTwoHpPercent = 68;
		leapArcBoss.iPhase = 1;
		leapArcBoss.fPositionX = 156.03f;
		leapArcBoss.fPositionY = 22.97f;
		leapArcBoss.fPositionZ = -122.06f;
		leapArcBoss.fSpawnPositionX = leapArcBoss.fPositionX;
		leapArcBoss.fSpawnPositionY = leapArcBoss.fPositionY;
		leapArcBoss.fSpawnPositionZ = leapArcBoss.fPositionZ;
		leapArcBoss.fEngageDistance = 35.f;
		leapArcBoss.fMoveSpeed = 3.f;
		leapArcBoss.bIntroPatternConsumed = true;
		/* Queued the way a crossed health bar queues its mechanic, so the jump
		starts on the next tick without waiting for a weighted roll to name it. */
		leapArcBoss.PendingPatternIds.push_back("VALTAN_HIGH_JUMP");
		CValtanBrain leapArcBrain;
		std::vector<DAMAGE_EVENT> leapArcDamage;
		leapArcBrain.Update(
			leapArcBoss, leapArcPlayers, catalog, navigation,
			1.f / 30.f, 900u, {}, leapArcDamage);
		const bool startedTheJump =
			"VALTAN_HIGH_JUMP" == leapArcBoss.strPatternId;
		const bool landsOnTheTarget =
			std::abs(leapArcBoss.fLeapLandingX - leapArcTarget.fPositionX) < 0.01f &&
			std::abs(leapArcBoss.fLeapLandingZ - leapArcTarget.fPositionZ) < 0.01f;
		const bool leftItsOwnFeet =
			std::abs(leapArcBoss.fLeapLandingX - leapArcBoss.fSpawnPositionX) > 1.f ||
			std::abs(leapArcBoss.fLeapLandingZ - leapArcBoss.fSpawnPositionZ) > 1.f;
		tests.Require(
			nullptr != highJump &&
			BOSS_PATTERN_MOTION_KIND::LEAP_TO_TARGET == highJump->Motion.eKind &&
			1133u == highJump->Motion.iTakeoffStartMs &&
			1500u == highJump->Motion.iTakeoffEndMs &&
			0u == highJump->Motion.iTravelStartMs &&
			267u == highJump->Motion.iTravelEndMs &&
			startedTheJump && landsOnTheTarget && leftItsOwnFeet &&
			leapArcBoss.fPatternLeapApexHeight == highJump->Motion.fApexHeight,
			"Land the high jump on its locked player with the authored fast lift/drop windows");

		std::uint32_t leapArcTick = 901u;
		float heightAtOneSecond = leapArcBoss.fPositionY;
		float heightAtTwelveHundredMs = leapArcBoss.fPositionY;
		float heightAtFifteenHundredMs = leapArcBoss.fPositionY;
		for (std::uint32_t tick = 0u;
			tick < 70u && "TAKEOFF" == leapArcBoss.strPatternStageId; ++tick)
		{
			leapArcDamage.clear();
			leapArcBrain.Update(
				leapArcBoss, leapArcPlayers, catalog, navigation,
				1.f / 30.f, leapArcTick++, {}, leapArcDamage);
			if (29u == tick)
				heightAtOneSecond = leapArcBoss.fPositionY;
			else if (35u == tick)
				heightAtTwelveHundredMs = leapArcBoss.fPositionY;
			else if (44u == tick)
				heightAtFifteenHundredMs = leapArcBoss.fPositionY;
		}
		const float expectedApexY =
			leapArcBoss.fLeapOriginY +
			(nullptr == highJump ? 0.f : highJump->Motion.fApexHeight);
		tests.Require(
			std::abs(heightAtOneSecond - leapArcBoss.fLeapOriginY) < 0.01f &&
			heightAtTwelveHundredMs > leapArcBoss.fLeapOriginY + 1.f &&
			heightAtTwelveHundredMs < expectedApexY - 1.f &&
			std::abs(heightAtFifteenHundredMs - expectedApexY) < 0.1f,
			"Keep the high-jump anticipation grounded, then reach the apex in the authored short lift window");
		bool heldAtApex = "AIRBORNE" == leapArcBoss.strPatternStageId;
		bool enteredLand = false;
		for (std::uint32_t tick = 0u;
			tick < 200u && !enteredLand; ++tick)
		{
			if ("AIRBORNE" == leapArcBoss.strPatternStageId)
			{
				heldAtApex = heldAtApex &&
					std::abs(leapArcBoss.fPositionX - leapArcBoss.fLeapOriginX) <
						0.001f &&
					std::abs(leapArcBoss.fPositionY - expectedApexY) < 0.001f &&
					std::abs(leapArcBoss.fPositionZ - leapArcBoss.fLeapOriginZ) <
						0.001f;
			}
			leapArcDamage.clear();
			leapArcBrain.Update(
				leapArcBoss, leapArcPlayers, catalog, navigation,
				1.f / 30.f, leapArcTick++, {}, leapArcDamage);
			enteredLand = "LAND" == leapArcBoss.strPatternStageId;
		}
		bool enteredRecovery = false;
		bool landedInsideFastWindow = false;
		for (std::uint32_t tick = 0u;
			tick < 110u && !enteredRecovery; ++tick)
		{
			leapArcDamage.clear();
			leapArcBrain.Update(
				leapArcBoss, leapArcPlayers, catalog, navigation,
				1.f / 30.f, leapArcTick++, {}, leapArcDamage);
			if (8u == tick)
			{
				landedInsideFastWindow =
					std::abs(leapArcBoss.fPositionX - leapArcTarget.fPositionX) < 0.01f &&
					std::abs(leapArcBoss.fPositionY - leapArcTarget.fPositionY) < 0.01f &&
					std::abs(leapArcBoss.fPositionZ - leapArcTarget.fPositionZ) < 0.01f;
			}
			enteredRecovery = "RECOVERY" == leapArcBoss.strPatternStageId;
		}
		tests.Require(
			heldAtApex && enteredLand && landedInsideFastWindow && enteredRecovery &&
			2u == leapArcBoss.iPatternLeapTravelStageIndex &&
			std::abs(leapArcBoss.fPositionX - leapArcTarget.fPositionX) < 0.01f &&
			std::abs(leapArcBoss.fPositionY - leapArcTarget.fPositionY) < 0.01f &&
			std::abs(leapArcBoss.fPositionZ - leapArcTarget.fPositionZ) < 0.01f,
			"Hold the high jump at its apex for AIRBORNE and finish the drop in LAND's authored fast window");

		/* With nobody to lock, the arc still needs a real destination or the
		boss would drop through an uninitialised landing. */
		std::map<PLAYER_ID, SERVER_PLAYER> emptyLeapPlayers;
		SERVER_WORLD_ENTITY targetlessBoss = leapArcBoss;
		targetlessBoss.strPatternId.clear();
		targetlessBoss.strPatternStageId.clear();
		targetlessBoss.PendingPatternIds.clear();
		targetlessBoss.PendingPatternIds.push_back("VALTAN_HIGH_JUMP");
		targetlessBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		targetlessBoss.fLeapLandingX = 0.f;
		targetlessBoss.fLeapLandingZ = 0.f;
		std::vector<DAMAGE_EVENT> targetlessDamage;
		CValtanBrain targetlessBrain;
		targetlessBrain.Update(
			targetlessBoss, emptyLeapPlayers, catalog, navigation,
			1.f / 30.f, 940u, {}, targetlessDamage);
		tests.Require(
			nullptr != highJump &&
			0.f == targetlessBoss.fLeapLandingX &&
			0.f == targetlessBoss.fLeapLandingZ,
			"Leave a targetless high jump alone rather than starting a blind arc");
	}

	{
		/* The completed 109 outer ring encloses the arena on every bearing, so
		nothing walks in or out of it until the collapse. The player reaches the
		arena through Stage_Boss_ArenaEntry instead, and the 159 wall's own
		passage is proved inside the ring by the FRACTURED collision test above.
		All bearings here were measured against the published navgrid. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr float ARENA_CENTER_X = 156.03f;
		constexpr float ARENA_CENTER_Z = -122.06f;
		/* 131 degrees used to be the mouth of the walk-in corridor, back when
		the ring was still missing six slabs. The completed ring seals every
		bearing until the 109 collapse, and Stage_Boss_ArenaEntry now carries
		the player across it, so this bearing must block like the rest. */
		const auto sweepAcrossRing = [&](const float bearingDegrees)
		{
			const float radians = bearingDegrees * 3.14159265f / 180.f;
			SERVER_PLAYER walker{};
			walker.fPositionX = ARENA_CENTER_X + std::cos(radians) * 15.f;
			walker.fPositionY = 23.04f;
			walker.fPositionZ = ARENA_CENTER_Z + std::sin(radians) * 15.f;
			float resolvedX = 0.f;
			float resolvedY = 0.f;
			float resolvedZ = 0.f;
			bool blocked = false;
			const bool resolved = room.m_ServerCollisionSystem.Resolve_PlayerMove(
				walker,
				ARENA_CENTER_X + std::cos(radians) * 21.f,
				walker.fPositionY,
				ARENA_CENTER_Z + std::sin(radians) * 21.f,
				resolvedX, resolvedY, resolvedZ, blocked);
			return resolved && blocked;
		};
		tests.Require(
			room.Is_Ready() && sweepAcrossRing(0.f) && sweepAcrossRing(60.f) &&
			sweepAcrossRing(216.f),
			"Block outward movement through the intact 109 outer ring");
		tests.Require(
			room.Is_Ready() && sweepAcrossRing(131.f) &&
			sweepAcrossRing(150.f) && sweepAcrossRing(294.f),
			"Seal the former entrance and 159 gaps with the completed 109 ring");

		/* Every wall that stands on authored floor now owns a blocker region,
		so pathfinding stops at it instead of walking through, and both the
		Stage_Boss_ArenaEntry landing point and the boss spawn stay reachable
		inside the sealed ring because their cells were kept out of them. */
		std::vector<SERVER_NAV_POINT> wallPath;
		const bool bossActivated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		const SERVER_WORLD_ENTITY* spawnedBoss = room.Find_AuditionBoss();
		std::vector<SERVER_NAV_POINT> bossPath;
		tests.Require(
			bossActivated && nullptr != spawnedBoss &&
			room.m_ServerNavigation.Find_Path(
				147.75f, -117.25f, 156.25f, -122.25f, wallPath) &&
			!wallPath.empty() &&
			room.m_ServerNavigation.Find_Path(
				spawnedBoss->fSpawnPositionX, spawnedBoss->fSpawnPositionZ,
				156.25f, -122.25f, bossPath) &&
			!bossPath.empty(),
			"Keep the arena and the Valtan spawn connected under the wall blockers");
	}

	{
		/* The pillars are the one encounter prop that has to come back. Wall
		groups leave INTACT once and never return, so this runtime is checked
		on exactly that difference: the same four slots must cycle. */
		ENCOUNTER_PROP_SET_DESCRIPTOR descriptor{};
		descriptor.strPropSetId = "encounterprop.valtan.four-pillars";
		descriptor.strEncounterId = "ENCOUNTER_VALTAN";
		descriptor.fCoverRadiusMeters = 0.9f;
		/* Authored order is preserved, so the set is deliberately not sorted. */
		descriptor.Slots = {
			{ "pillar.valtan.slot03", 152.423755f, -118.453755f },
			{ "pillar.valtan.slot00", 159.636245f, -118.453755f },
			{ "pillar.valtan.slot02", 152.423755f, -125.666245f },
			{ "pillar.valtan.slot01", 159.636245f, -125.666245f } };

		std::string status;
		CEncounterPropRuntime runtime;
		const bool initialized = runtime.Initialize(descriptor, status, 1u);
		const std::vector<ENCOUNTER_PROP_SLOT_STATE>& slots =
			runtime.Get_SlotStates();
		tests.Require(
			initialized && 4u == slots.size() &&
			std::is_sorted(slots.begin(), slots.end(),
				[](const ENCOUNTER_PROP_SLOT_STATE& left,
					const ENCOUNTER_PROP_SLOT_STATE& right)
				{
					return left.strSlotId < right.strSlotId;
				}) &&
			std::all_of(slots.begin(), slots.end(),
				[](const ENCOUNTER_PROP_SLOT_STATE& slot)
				{
					return ENCOUNTER_PROP_STATE::HIDDEN == slot.eState &&
						0u == slot.iOccurrenceSequence;
				}),
			"Initialize the four pillar slots hidden in canonical slot order");

		ENCOUNTER_PROP_SET_DESCRIPTOR duplicated = descriptor;
		duplicated.Slots.push_back(
			{ "pillar.valtan.slot00", 152.423755f, -118.453755f });
		CEncounterPropRuntime rejected;
		ENCOUNTER_PROP_SET_DESCRIPTOR nameless = descriptor;
		nameless.Slots[1u].strSlotId.clear();
		CEncounterPropRuntime alsoRejected;
		tests.Require(
			!rejected.Initialize(duplicated, status, 1u) &&
			!rejected.Is_Initialized() &&
			!alsoRejected.Initialize(nameless, status, 1u) &&
			!alsoRejected.Is_Initialized(),
			"Reject a duplicate or nameless pillar slot without a partial set");

		ENCOUNTER_PROP_TRANSACTION raise{};
		const bool raised =
			ENCOUNTER_PROP_PREPARE_RESULT::READY == runtime.Prepare_Spawn(
				7u, 100u, raise, status) &&
			4u == raise.Slots.size() &&
			runtime.Commit(raise, status);
		ENCOUNTER_PROP_TRANSACTION repeated{};
		const ENCOUNTER_PROP_PREPARE_RESULT repeatedResult =
			runtime.Prepare_Spawn(7u, 104u, repeated, status);
		tests.Require(
			raised && 7u == runtime.Get_OccurrenceSequence() &&
			std::all_of(slots.begin(), slots.end(),
				[](const ENCOUNTER_PROP_SLOT_STATE& slot)
				{
					return ENCOUNTER_PROP_STATE::INTACT == slot.eState &&
						7u == slot.iOccurrenceSequence &&
						100u == slot.iStateStartTick;
				}) &&
			ENCOUNTER_PROP_PREPARE_RESULT::NO_CHANGE == repeatedResult &&
			repeated.Slots.empty(),
			"Raise the four pillars once and ignore the repeated raise edge");

		ENCOUNTER_PROP_TRANSACTION stale = raise;
		ENCOUNTER_PROP_TRANSACTION shatter{};
		const bool shattered =
			!runtime.Commit(stale, status) &&
			ENCOUNTER_PROP_PREPARE_RESULT::READY == runtime.Prepare_Break(
				7u, 200u, shatter, status) &&
			runtime.Commit(shatter, status);
		ENCOUNTER_PROP_TRANSACTION early{};
		ENCOUNTER_PROP_TRANSACTION due{};
		const ENCOUNTER_PROP_PREPARE_RESULT earlyResult =
			runtime.Prepare_DueRemoval(207u, 8u, early, status);
		const bool retired =
			ENCOUNTER_PROP_PREPARE_RESULT::READY == runtime.Prepare_DueRemoval(
				208u, 8u, due, status) &&
			runtime.Commit(due, status);
		tests.Require(
			shattered && ENCOUNTER_PROP_PREPARE_RESULT::NO_CHANGE ==
				earlyResult && early.Slots.empty() && retired &&
			std::all_of(slots.begin(), slots.end(),
				[](const ENCOUNTER_PROP_SLOT_STATE& slot)
				{
					return ENCOUNTER_PROP_STATE::HIDDEN == slot.eState;
				}),
			"Shatter the pillars and retire them only on the authored due tick");

		ENCOUNTER_PROP_TRANSACTION nextCycle{};
		const bool cycled =
			ENCOUNTER_PROP_PREPARE_RESULT::READY == runtime.Prepare_Spawn(
				8u, 300u, nextCycle, status) &&
			runtime.Commit(nextCycle, status);
		tests.Require(
			cycled && 8u == runtime.Get_OccurrenceSequence() &&
			std::all_of(slots.begin(), slots.end(),
				[](const ENCOUNTER_PROP_SLOT_STATE& slot)
				{
					return ENCOUNTER_PROP_STATE::INTACT == slot.eState &&
						8u == slot.iOccurrenceSequence;
				}),
			"Raise the same four slots again on the next pattern occurrence");

		ENCOUNTER_PROP_TRANSACTION crossEpoch{};
		const bool preparedBeforeReset =
			ENCOUNTER_PROP_PREPARE_RESULT::READY == runtime.Prepare_Break(
				8u, 320u, crossEpoch, status);
		const std::uint32_t epochBeforeReset = runtime.Get_EncounterEpoch();
		tests.Require(
			preparedBeforeReset && runtime.Reset(status, 400u) &&
			epochBeforeReset != runtime.Get_EncounterEpoch() &&
			0u == runtime.Get_OccurrenceSequence() &&
			!runtime.Commit(crossEpoch, status) &&
			std::all_of(slots.begin(), slots.end(),
				[](const ENCOUNTER_PROP_SLOT_STATE& slot)
				{
					return ENCOUNTER_PROP_STATE::HIDDEN == slot.eState &&
						0u == slot.iOccurrenceSequence;
				}),
			"Reset the pillars to hidden and refuse a transaction from the old epoch");
	}

	{
		/* Cover is the attack segment against the stele circle, not a
		containment test on either end. A player directly behind the stele is
		answered by it; one standing beside it, or in front of it, is not. */
		using LostArk::Shared::CombatCollision::CIRCLE_XZ;
		using LostArk::Shared::CombatCollision::Segment_IntersectsCircle;
		const CIRCLE_XZ stele{ 0.f, 5.f, 0.9f };
		const bool behindIsAnswered =
			Segment_IntersectsCircle(0.f, 0.f, 0.f, 10.f, stele);
		const bool besideIsExposed =
			!Segment_IntersectsCircle(0.f, 0.f, 6.f, 10.f, stele);
		const bool inFrontIsExposed =
			!Segment_IntersectsCircle(0.f, 0.f, 0.f, 3.f, stele);
		const bool zeroRadiusIsExposed = !Segment_IntersectsCircle(
			0.f, 0.f, 0.f, 10.f, CIRCLE_XZ{ 0.f, 5.f, 0.f });
		tests.Require(
			behindIsAnswered && besideIsExposed && inFrontIsExposed &&
			zeroRadiusIsExposed,
			"Answer a blow only where the stele stands between boss and player");
	}

	{
		/* A stele stops being cover on the tick it starts breaking, which is
		what hands the raid over to the opposite diagonal. */
		ENCOUNTER_PROP_SET_DESCRIPTOR coverSet{};
		coverSet.strPropSetId = "encounterprop.valtan.four-pillars";
		coverSet.strEncounterId = "ENCOUNTER_VALTAN";
		coverSet.fCoverRadiusMeters = 0.9f;
		coverSet.Slots = {
			{ "pillar.valtan.slot00", 159.636245f, -118.453755f },
			{ "pillar.valtan.slot01", 159.636245f, -125.666245f },
			{ "pillar.valtan.slot02", 152.423755f, -125.666245f },
			{ "pillar.valtan.slot03", 152.423755f, -118.453755f } };
		std::string coverStatus;
		CEncounterPropRuntime coverRuntime;
		const auto intactCount = [&coverRuntime]()
		{
			const std::vector<ENCOUNTER_PROP_SLOT_STATE>& live =
				coverRuntime.Get_SlotStates();
			return std::count_if(live.begin(), live.end(),
				[](const ENCOUNTER_PROP_SLOT_STATE& slot)
				{ return ENCOUNTER_PROP_STATE::INTACT == slot.eState; });
		};
		ENCOUNTER_PROP_TRANSACTION coverRaise{};
		const bool coverRaised =
			coverRuntime.Initialize(coverSet, coverStatus, 1u) &&
			ENCOUNTER_PROP_PREPARE_RESULT::READY == coverRuntime.Prepare_Spawn(
				1u, 10u, coverRaise, coverStatus) &&
			coverRuntime.Commit(coverRaise, coverStatus);
		/* The authored position has to survive the raise, because the cover
		circle is built from the live slot rather than a second lookup. */
		const std::vector<ENCOUNTER_PROP_SLOT_STATE>& coverSlots =
			coverRuntime.Get_SlotStates();
		const bool carriedPositions = coverRaised && 4u == coverSlots.size() &&
			std::all_of(coverSlots.begin(), coverSlots.end(),
				[](const ENCOUNTER_PROP_SLOT_STATE& slot)
				{
					return 0.f != slot.fPositionX && 0.f != slot.fPositionZ;
				});
		/* Counted before the shatter, because the same query answers both
		states and the order of the two checks would otherwise decide them. */
		const auto raisedCoverCount = intactCount();
		ENCOUNTER_PROP_TRANSACTION coverShatter{};
		const std::vector<std::string> firstDiagonal = {
			"pillar.valtan.slot00", "pillar.valtan.slot02" };
		const bool diagonalShattered =
			ENCOUNTER_PROP_PREPARE_RESULT::READY ==
				coverRuntime.Prepare_BreakSlots(
					firstDiagonal, 1u, 20u, coverShatter, coverStatus) &&
			coverRuntime.Commit(coverShatter, coverStatus);
		const auto remainingCoverCount = intactCount();
		tests.Require(
			carriedPositions && 4 == raisedCoverCount,
			"Raise four stele slots carrying the authored cover position");
		tests.Require(
			diagonalShattered && 2 == remainingCoverCount &&
			0.9f == coverRuntime.Get_CoverRadiusMeters(),
			"Leave only the opposite diagonal standing as cover");
	}

	{
		/* The stele contracts are only real once the publisher wrote them into
		the bootstrap the Server actually reads, so they are checked against the
		loaded catalog rather than against the authoring documents. */
		const std::vector<BOSS_PATTERN_DEFINITION>* valtanPatterns =
			catalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const auto findStage = [valtanPatterns](
			const char* patternId, const char* stageId)
			-> const BOSS_PATTERN_STAGE_DEFINITION*
		{
			if (nullptr == valtanPatterns)
				return nullptr;
			const auto pattern = std::find_if(
				valtanPatterns->begin(), valtanPatterns->end(),
				[patternId](const BOSS_PATTERN_DEFINITION& candidate)
				{ return candidate.strPatternId == patternId; });
			if (valtanPatterns->end() == pattern)
				return nullptr;
			const auto stage = std::find_if(
				pattern->Stages.begin(), pattern->Stages.end(),
				[stageId](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
				{ return candidate.strStageId == stageId; });
			return pattern->Stages.end() == stage ? nullptr : &(*stage);
		};
		const BOSS_PATTERN_STAGE_DEFINITION* pierceStage =
			findStage("VALTAN_FOUR_PILLARS_105", "TARGET_CONE");
		const BOSS_PATTERN_STAGE_DEFINITION* ringStage =
			findStage("VALTAN_FOUR_PILLARS_105", "YELLOW_ZONE");
		tests.Require(
			nullptr != pierceStage && nullptr != ringStage &&
			pierceStage->bPiercesCover && !ringStage->bPiercesCover &&
			pierceStage->strDamageProfileId !=
				ringStage->strDamageProfileId,
			"Grant cover piercing to the authored cone alone and on its own damage");

		const BOSS_PATTERN_STAGE_DEFINITION* firstBreak =
			findStage("VALTAN_RED_BLADE_WAVE", "PROJECTILE");
		const BOSS_PATTERN_STAGE_DEFINITION* secondBreak =
			findStage("VALTAN_RED_BLADE_WAVE", "RECOVERY");
		const BOSS_PATTERN_STAGE_DEFINITION* noBreak =
			findStage("VALTAN_RED_BLADE_WAVE", "WINDUP");
		bool pairsAreDisjoint =
			nullptr != firstBreak && nullptr != secondBreak;
		if (pairsAreDisjoint)
		{
			for (const std::string& slotId : firstBreak->PropBreakSlotIds)
			{
				pairsAreDisjoint = pairsAreDisjoint &&
					secondBreak->PropBreakSlotIds.end() == std::find(
						secondBreak->PropBreakSlotIds.begin(),
						secondBreak->PropBreakSlotIds.end(), slotId);
			}
		}
		tests.Require(
			nullptr != firstBreak && nullptr != secondBreak &&
			nullptr != noBreak &&
			2u == firstBreak->PropBreakSlotIds.size() &&
			2u == secondBreak->PropBreakSlotIds.size() &&
			noBreak->PropBreakSlotIds.empty() && pairsAreDisjoint &&
			firstBreak->strPropBreakSetId ==
				"encounterprop.valtan.four-pillars",
			"Break the stele two at a time on two disjoint authored stage edges");
	}

	{
		CGameRoom valtanRoom{ WORLD_ID::VALTAN_ARENA };
		CGameRoom bernRoom{ WORLD_ID::BERN };
		tests.Require(
			valtanRoom.Is_Ready() && bernRoom.Is_Ready() &&
			valtanRoom.m_EstherSkillSystem.Is_Enabled() &&
			1000u == valtanRoom.m_EstherSkillSystem.Get_GaugeMaximum() &&
			!bernRoom.m_EstherSkillSystem.Is_Enabled() &&
			0u == bernRoom.m_EstherSkillSystem.Get_GaugeMaximum(),
			"Own a shared Esther gauge only in the Valtan raid room");

		valtanRoom.m_EstherSkillSystem.Update(2.f, false);
		const std::uint32_t emptyRoomGauge =
			valtanRoom.m_EstherSkillSystem.Get_Gauge();
		for (int tick = 0; tick < 200; ++tick)
			valtanRoom.m_EstherSkillSystem.Update(1.f / 30.f, true);
		tests.Require(
			0u == emptyRoomGauge &&
			1000u == valtanRoom.m_EstherSkillSystem.Get_Gauge(),
			"Charge the shared gauge to full only while players occupy the room");

		const ESTHER_ROSTER_ENTRY* pRosterEntry = nullptr;
		const ESTHER_USE_REJECTION wrongSlot =
			valtanRoom.m_EstherSkillSystem.Try_Consume(4u, pRosterEntry);
		tests.Require(
			ESTHER_USE_REJECTION::UNSUPPORTED_SLOT == wrongSlot &&
			nullptr == pRosterEntry &&
			1000u == valtanRoom.m_EstherSkillSystem.Get_Gauge(),
			"Reject an out-of-roster Esther slot without touching the gauge");

		const ESTHER_USE_REJECTION disabledWorld =
			bernRoom.m_EstherSkillSystem.Try_Consume(1u, pRosterEntry);
		tests.Require(
			ESTHER_USE_REJECTION::DISABLED_WORLD == disabledWorld,
			"Reject an Esther use outside the raid world");

		const ESTHER_USE_REJECTION weiSlot =
			valtanRoom.m_EstherSkillSystem.Try_Consume(2u, pRosterEntry);
		tests.Require(
			ESTHER_USE_REJECTION::NONE == weiSlot &&
			nullptr != pRosterEntry &&
			std::string("NPC_58700") == pRosterEntry->pArchetypeId &&
			7100u == pRosterEntry->iStrikeMs &&
			0u == valtanRoom.m_EstherSkillSystem.Get_Gauge(),
			"Consume slot 2 into Wei's all-in-one clip timeline");

		for (int tick = 0; tick < 200; ++tick)
			valtanRoom.m_EstherSkillSystem.Update(1.f / 30.f, true);
		pRosterEntry = nullptr;
		const ESTHER_USE_REJECTION bahunturSlot =
			valtanRoom.m_EstherSkillSystem.Try_Consume(3u, pRosterEntry);
		tests.Require(
			ESTHER_USE_REJECTION::NONE == bahunturSlot &&
			nullptr != pRosterEntry &&
			std::string("NPC_59060") == pRosterEntry->pArchetypeId &&
			4100u == pRosterEntry->iStrikeMs &&
			0u == valtanRoom.m_EstherSkillSystem.Get_Gauge(),
			"Consume slot 3 into the Bahuntur summon with its authored strike length");

		/* Handler path: an authenticated caster with a full gauge summons at
		its own feet, aimed east, and the gauge drains to zero atomically. */
		constexpr SESSION_ID casterSessionId = 41u;
		constexpr LostArk::Shared::PLAYER_ID casterPlayerId = 9u;
		SERVER_PLAYER caster{};
		caster.iNetEntityId = 4100u;
		caster.fPositionX = 150.f;
		caster.fPositionY = 22.97f;
		caster.fPositionZ = -120.f;
		caster.fYawDegrees = 0.f;
		valtanRoom.m_Players.emplace(casterPlayerId, caster);
		valtanRoom.m_PlayerIdBySessionId.emplace(
			casterSessionId, casterPlayerId);

		const std::size_t entitiesBeforeSummon =
			valtanRoom.m_WorldEntities.size();
		LostArk::Shared::C2S_USE_ESTHER_SKILL notFullUse{};
		notFullUse.iClientSequence = 1u;
		notFullUse.iSlotIndex = 1u;
		notFullUse.fAimX = 160.f;
		notFullUse.fAimZ = -120.f;
		valtanRoom.m_EstherSkillSystem.Reset();
		valtanRoom.Handle_UseEstherSkill(casterSessionId, notFullUse);
		tests.Require(
			entitiesBeforeSummon == valtanRoom.m_WorldEntities.size(),
			"Reject an Esther use before the gauge is full");

		for (int tick = 0; tick < 200; ++tick)
			valtanRoom.m_EstherSkillSystem.Update(1.f / 30.f, true);
		valtanRoom.Handle_UseEstherSkill(casterSessionId, notFullUse);
		const auto findSummon = [&valtanRoom]() -> SERVER_WORLD_ENTITY*
		{
			for (SERVER_WORLD_ENTITY& entity : valtanRoom.m_WorldEntities)
			{
				if (entity.isEstherSummon)
					return &entity;
			}
			return nullptr;
		};
		tests.Require(
			nullptr == findSummon() &&
			0u == valtanRoom.m_EstherSkillSystem.Get_Gauge() &&
			1u == valtanRoom.m_PendingEstherSummons.size(),
			"Drain the Esther gauge at once but hold the summon for the landing delay");

		/* The accepted call locks the caster: ESTHER_CAST with no skill id,
		turned to the aim (east of the caster is +90 degrees). */
		SERVER_PLAYER& roomCaster = valtanRoom.m_Players.at(casterPlayerId);
		tests.Require(
			LostArk::Shared::PLAYER_ACTION_STATE::ESTHER_CAST ==
				roomCaster.eAction &&
			LostArk::Shared::INVALID_SKILL_ID == roomCaster.iCurrentSkillId &&
			0u != roomCaster.iActionStartTick &&
			std::abs(roomCaster.fYawDegrees - 90.f) < 0.01f,
			"Lock the caster into the Esther call turned to the aim");

		for (int tick = 0; tick < 200; ++tick)
			valtanRoom.m_EstherSkillSystem.Update(1.f / 30.f, true);
		valtanRoom.Handle_UseEstherSkill(casterSessionId, notFullUse);
		tests.Require(
			1000u == valtanRoom.m_EstherSkillSystem.Get_Gauge() &&
			1u == valtanRoom.m_PendingEstherSummons.size(),
			"Reject an Esther use while the caster is still casting");

		/* The room releases the cast through the player update once the call
		clip's 1500 ms have elapsed on the tick clock. */
		valtanRoom.m_iServerTick = 60u;
		valtanRoom.Update_Players(1.f / 30.f);
		tests.Require(
			LostArk::Shared::PLAYER_ACTION_STATE::NONE == roomCaster.eAction &&
			0u == roomCaster.iActionStartTick,
			"Release the caster to NONE once the call clip has run out");

		/* Drain the recharged gauge again so the emptied-gauge rejection
		below still tests the gauge and not the cast lock. */
		pRosterEntry = nullptr;
		(void)valtanRoom.m_EstherSkillSystem.Try_Consume(1u, pRosterEntry);

		/* The landing spot is two metres along the aim (east here), sampled on
		the navigation grid; an unwalkable sample falls back to the caster. */
		SERVER_NAV_POINT expectedLanding{
			caster.fPositionX, caster.fPositionY, caster.fPositionZ };
		(void)valtanRoom.m_ServerNavigation.Sample_Position(
			caster.fPositionX + ESTHER_SUMMON_FORWARD_METERS,
			caster.fPositionZ,
			expectedLanding);
		for (int tick = 0; tick < 29; ++tick)
			valtanRoom.Update_WorldEntities(1.f / 30.f);
		tests.Require(
			nullptr == findSummon(),
			"Keep the Esther summon pending until the full landing delay has elapsed");
		valtanRoom.Update_WorldEntities(1.f / 30.f);
		SERVER_WORLD_ENTITY* summon = findSummon();
		tests.Require(
			nullptr != summon &&
			valtanRoom.m_PendingEstherSummons.empty() &&
			"NPC_59030" == summon->strArchetypeId &&
			5300u == summon->iEstherStrikeMs &&
			WORLD_BOOTSTRAP_KIND::NPC == summon->eKind &&
			std::string(ESTHER_ACTION_STRIKE) == summon->strActionId &&
			SERVER_ENTITY_ACTION::PATTERN_ACTIVE == summon->eAction &&
			std::abs(summon->fPositionX - expectedLanding.x) < 0.001f &&
			std::abs(summon->fPositionY - expectedLanding.y) < 0.001f &&
			std::abs(summon->fPositionZ - expectedLanding.z) < 0.001f &&
			std::abs(summon->fYawDegrees - 90.f) < 0.01f,
			"Land Sillian two metres along the aim straight into its all-in-one strike");

		valtanRoom.Handle_UseEstherSkill(casterSessionId, notFullUse);
		std::size_t summonCount = 0u;
		for (const SERVER_WORLD_ENTITY& entity : valtanRoom.m_WorldEntities)
		{
			if (entity.isEstherSummon)
				++summonCount;
		}
		tests.Require(
			1u == summonCount && valtanRoom.m_PendingEstherSummons.empty(),
			"Reject a second Esther use on the emptied gauge");

		for (int tick = 0; tick < 162; ++tick)
			valtanRoom.Update_WorldEntities(1.f / 30.f);
		tests.Require(
			nullptr == findSummon(),
			"Despawn Sillian the moment its clip ends without the skyward rise");

		for (int tick = 0; tick < 200; ++tick)
			valtanRoom.m_EstherSkillSystem.Update(1.f / 30.f, true);
		LostArk::Shared::C2S_USE_ESTHER_SKILL bahunturUse{};
		bahunturUse.iClientSequence = 3u;
		bahunturUse.iSlotIndex = 3u;
		bahunturUse.fAimX = 160.f;
		bahunturUse.fAimZ = -120.f;
		valtanRoom.Handle_UseEstherSkill(casterSessionId, bahunturUse);
		for (int tick = 0; tick < 30; ++tick)
			valtanRoom.Update_WorldEntities(1.f / 30.f);
		SERVER_WORLD_ENTITY* bahunturSummon = findSummon();
		tests.Require(
			nullptr != bahunturSummon &&
			"NPC_59060" == bahunturSummon->strArchetypeId &&
			4100u == bahunturSummon->iEstherStrikeMs &&
			std::string(ESTHER_ACTION_STRIKE) == bahunturSummon->strActionId &&
			SERVER_ENTITY_ACTION::PATTERN_ACTIVE == bahunturSummon->eAction,
			"Spawn Bahuntur straight into the strike with no appear stage");

		for (int tick = 0; tick < 126; ++tick)
			valtanRoom.Update_WorldEntities(1.f / 30.f);
		tests.Require(
			nullptr == findSummon(),
			"Despawn Bahuntur the moment its clip ends without the skyward rise");

		/* Release the Bahuntur call before the next use; the caster would
		otherwise still hold ESTHER_CAST and reject it. */
		valtanRoom.m_iServerTick = 120u;
		valtanRoom.Update_Players(1.f / 30.f);
		for (int tick = 0; tick < 200; ++tick)
			valtanRoom.m_EstherSkillSystem.Update(1.f / 30.f, true);
		LostArk::Shared::C2S_USE_ESTHER_SKILL weiUse{};
		weiUse.iClientSequence = 2u;
		weiUse.iSlotIndex = 2u;
		weiUse.fAimX = 160.f;
		weiUse.fAimZ = -120.f;
		valtanRoom.Handle_UseEstherSkill(casterSessionId, weiUse);
		for (int tick = 0; tick < 30; ++tick)
			valtanRoom.Update_WorldEntities(1.f / 30.f);
		SERVER_WORLD_ENTITY* weiSummon = findSummon();
		tests.Require(
			nullptr != weiSummon &&
			"NPC_58700" == weiSummon->strArchetypeId &&
			std::string(ESTHER_ACTION_STRIKE) == weiSummon->strActionId &&
			SERVER_ENTITY_ACTION::PATTERN_ACTIVE == weiSummon->eAction,
			"Spawn Wei straight into the strike with no appear stage");

		for (int tick = 0; tick < 216; ++tick)
			valtanRoom.Update_WorldEntities(1.f / 30.f);
		tests.Require(
			nullptr == findSummon(),
			"Despawn Wei the moment its clip ends without the skyward rise");

		valtanRoom.m_Players.clear();
		valtanRoom.m_PlayerIdBySessionId.clear();
		for (int tick = 0; tick < 30; ++tick)
			valtanRoom.m_EstherSkillSystem.Update(1.f / 30.f, true);
		tests.Require(
			valtanRoom.Reset_ValtanArenaWhenEmpty() &&
			0u == valtanRoom.m_EstherSkillSystem.Get_Gauge(),
			"Re-arm the Esther gauge from zero when the arena empties");
	}

	tests.failures += Run_WorldDestructionBootstrapContractTests();
	std::cout << "failures : " << tests.failures << '\n';
	return 0 == tests.failures ? 0 : 1;
		}();
	};
	if (!Run_WithContractWorkerStack(runContract, &context))
	{
		std::cerr << "[FAILURE] Server gameplay contract worker did not complete\n";
		return 1;
	}
	return context.result;
}

int LostArk::Server::Run_ServerNavigationContractTests()
{
	TESTS tests;
	CServerNavigation bernNavigation;
	const bool loaded = bernNavigation.Load("LV_BER_BERNCASTLE");
	tests.Require(
		loaded &&
		std::abs(
			bernNavigation.Get_MaximumTraversalStepHeight() - 1.f) < 0.001f &&
		bernNavigation.Is_HeightTransitionAllowed(47.f, 47.9f) &&
		!bernNavigation.Is_HeightTransitionAllowed(47.f, 53.f),
		"Load Bern navigation with a one-metre deck-step guard");

	SERVER_NAV_POINT lockedGround{};
	SERVER_NAV_POINT sampledGround{};
	const bool lockedWalkStep = loaded &&
		bernNavigation.Sample_Position(
			138.238007f, -110.188004f, sampledGround) &&
		bernNavigation.Resolve_TraversalStep(
			138.238007f,
			-110.188004f,
			138.300003f,
			-110.099998f,
			lockedGround);
	tests.Require(
		lockedWalkStep &&
		std::abs(lockedGround.y - sampledGround.y) < 0.000001f,
		"Lock a live Bern walk step to the destination navigation ground");

	SERVER_NAV_POINT rejectedDeckJump{};
	tests.Require(
		loaded && !bernNavigation.Resolve_TraversalStep(
			125.238007f,
			-170.688004f,
			125.238007f,
			-170.188004f,
			rejectedDeckJump),
		"Reject the real Bern 17-metre adjacent deck jump at live movement time");

	SERVER_NAV_POINT clampedDeckStep{
		125.238007f, 39.04832f, -170.688004f };
	bool deckStepClamped = false;
	if (loaded)
	{
		CPlayerSkillSystem::Clamp_StepToWalkable(
			bernNavigation,
			125.238007f,
			-170.688004f,
			125.238007f,
			-170.188004f,
			clampedDeckStep,
			deckStepClamped);
	}
	tests.Require(
		loaded && deckStepClamped && clampedDeckStep.y < 40.f &&
		clampedDeckStep.z < -170.438004f,
		"Clamp Bern skill and knockback movement before the upper deck");

	std::vector<SERVER_NAV_POINT> stairPath;
	const bool stairPathFound = loaded && bernNavigation.Find_Path(
		137.586334f,
		-22.4640217f,
		137.238007f,
		-116.688004f,
		stairPath);
	bool transitionsSafe = stairPathFound && stairPath.size() > 100u;
	float maximumPathStep = 0.f;
	for (std::size_t index = 1u; index < stairPath.size(); ++index)
	{
		const float step = std::abs(
			stairPath[index].y - stairPath[index - 1u].y);
		maximumPathStep = (std::max)(maximumPathStep, step);
		transitionsSafe = transitionsSafe &&
			bernNavigation.Is_HeightTransitionAllowed(
				stairPath[index - 1u].y, stairPath[index].y);
	}
	tests.Require(
		transitionsSafe && maximumPathStep <= 1.000001f &&
		!stairPath.empty() && stairPath.back().y > 49.f &&
		stairPath.back().y - stairPath.front().y > 6.f,
		"Reach the Bern stairs without crossing an arch-roof height jump");

	SERVER_NAV_POINT firstRidge{};
	SERVER_NAV_POINT secondRidge{};
	SERVER_NAV_POINT thirdRidge{};
	tests.Require(
		loaded &&
		bernNavigation.Sample_Position(
			138.238007f, -110.188004f, firstRidge) &&
		bernNavigation.Sample_Position(
			138.238007f, -97.688004f, secondRidge) &&
		bernNavigation.Sample_Position(
			137.738007f, -67.688004f, thirdRidge) &&
		firstRidge.y < 48.f && secondRidge.y < 48.f && thirdRidge.y < 44.f,
		"Publish all three repaired Bern corridor ridges on the ground deck");

	namespace fs = std::filesystem;
	std::vector<wchar_t> pathBuffer(32768u);
	const DWORD configuredLength = GetEnvironmentVariableW(
		L"LOSTARK_SERVER_DATA_ROOT",
		pathBuffer.data(),
		static_cast<DWORD>(pathBuffer.size()));
	const bool hadConfiguredRoot =
		0u != configuredLength && configuredLength < pathBuffer.size();
	fs::path packagedDataRoot;
	if (hadConfiguredRoot)
	{
		packagedDataRoot = fs::path(pathBuffer.data()).lexically_normal();
	}
	else
	{
		const DWORD moduleLength = GetModuleFileNameW(
			nullptr,
			pathBuffer.data(),
			static_cast<DWORD>(pathBuffer.size()));
		if (0u != moduleLength && moduleLength < pathBuffer.size())
		{
			packagedDataRoot = fs::path(pathBuffer.data()).parent_path().
				parent_path() / L"DataFiles";
		}
	}
	const fs::path invalidPolicyRoot = fs::temp_directory_path() /
		(L"LostArkNavigationPolicyContractTest-" +
			std::to_wstring(_getpid()));
	std::error_code fixtureError;
	fs::remove_all(invalidPolicyRoot, fixtureError);
	fixtureError.clear();
	fs::create_directories(
		invalidPolicyRoot / L"Navigation", fixtureError);
	if (!fixtureError)
	{
		fs::copy_file(
			packagedDataRoot / L"Navigation" /
				L"LV_BER_BERNCASTLE.navgrid",
			invalidPolicyRoot / L"Navigation" /
				L"LV_BER_BERNCASTLE.navgrid",
			fs::copy_options::overwrite_existing,
			fixtureError);
	}
	if (!fixtureError)
	{
		std::ofstream invalidPolicy(
			invalidPolicyRoot / L"Navigation" /
				L"LV_BER_BERNCASTLE.navpolicy",
			std::ios::binary | std::ios::trunc);
		invalidPolicy <<
			"LOSTARK_NAVIGATION_POLICY 1 \"WRONG_AREA\" 1\n";
		if (!invalidPolicy.good())
			fixtureError = std::make_error_code(std::errc::io_error);
	}
	bool rejectedInvalidPolicyTransactionally = false;
	if (!fixtureError && SetEnvironmentVariableW(
		L"LOSTARK_SERVER_DATA_ROOT", invalidPolicyRoot.c_str()))
	{
		CServerNavigation rejectedNavigation;
		rejectedInvalidPolicyTransactionally =
			!rejectedNavigation.Load("LV_BER_BERNCASTLE") &&
			!rejectedNavigation.Is_Loaded();
		SetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT",
			hadConfiguredRoot ? pathBuffer.data() : nullptr);
	}
	fs::remove_all(invalidPolicyRoot, fixtureError);
	tests.Require(
		rejectedInvalidPolicyTransactionally,
		"Roll back Bern navigation when its runtime policy is invalid");

	std::cout << "navigation failures : " << tests.failures << '\n';
	return 0 == tests.failures ? 0 : 1;
}

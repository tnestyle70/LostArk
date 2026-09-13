#pragma once

#include <WinSock2.h>
#include "Network/NetworkIds.h"
#include "Network/PacketFrame.h"
#include <array>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace LostArk::Server
{
class CGameplayCatalog;
class CSpawnGroupBootstrap;
}

namespace ServerGameplayContractDetail
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

	unsigned __stdcall Run_ContractStackWork(void* opaque) noexcept;


	bool Run_WithReservedWorkerStack(
		void (*function)(void*), void* context,
		const unsigned stackReserve);


	bool Run_WithContractWorkerStack(
		void (*function)(void*), void* context);


	bool Run_WithProductionServerStack(
		void (*function)(void*), void* context);


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

	std::uint32_t Calculate_TestTimelineCommandId(const std::string_view rowId);


	std::uint32_t Count_TestTimelineRows(const TEST_TIMELINE_VARIANT variant);


	void Write_TestValtanTimelineRows(
		std::ofstream& bootstrap,
		const TEST_TIMELINE_VARIANT variant);


	void Write_ValidValtanTimelineRows(std::ofstream& bootstrap);


	struct RECEIVED_TEST_FRAME final
	{
		LostArk::Shared::PACKET_TYPE packetType =
			LostArk::Shared::PACKET_TYPE::INVALID;
		std::vector<std::uint8_t> payload;
	};

	void Close_TestSocket(SOCKET& socket);


	void Abort_TestSocket(SOCKET& socket);


	bool Create_LoopbackSocketPair(
		SOCKET& outSessionSocket,
		SOCKET& outPeerSocket);


	bool Receive_Exact(
		const SOCKET socket,
		std::span<std::uint8_t> bytes);


	bool Receive_TestFrame(
		const SOCKET socket,
		RECEIVED_TEST_FRAME& outFrame);


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
		const std::string_view replacement);


	bool Reject_CorruptSpawnAnchorReloadTransactionally(
		LostArk::Server::CSpawnGroupBootstrap& bootstrap);



	void Run_KoukuObjectOverlapContracts(
		TESTS& tests, const LostArk::Server::CGameplayCatalog& catalog);


	void Run_KoukuObjectContactContracts(TESTS& tests, const LostArk::Server::CGameplayCatalog& catalog);


	void Run_KoukuWorldPlacementContracts(TESTS& tests, const LostArk::Server::CGameplayCatalog& catalog);


	void Run_KoukuBoneContactContracts(TESTS& tests, const LostArk::Server::CGameplayCatalog& catalog);


	/* KoukuSaydon Logic runtime: synthetic windows judged on a fixed clock so
	the verdict rules never depend on which pattern the composition authors.
	A function of its own keeps these rooms and players off the contract
	frame, which already sits close to the 1 MiB production stack. */
    void Run_KoukuFearAndCounterContracts(TESTS& tests, const LostArk::Server::CGameplayCatalog& catalog);


	void Run_KoukuSaydonLogicRuntimeContracts(
		TESTS& tests, const LostArk::Server::CGameplayCatalog& catalog);

	struct CONTRACT_TEST_RUN_CONTEXT final
	{
		bool dimensionMasterGroundTargetOnly = false;
		bool debugTeleportOnly = false;
		bool koukuBundlesOnly = false;
		bool worldPlaybackOnly = false;
		int result = 1;
	};
}
using namespace ServerGameplayContractDetail;

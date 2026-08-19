#include "ServerGameplayContractTests.h"

#include "ClientSession.h"
#include "EncounterPropRuntime.h"
#include "Gameplay/CombatCollisionContract.h"
#include "Gameplay/WorldCollisionContract.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "PlayerSkillSystem.h"
#include "ServerNavigation.h"
#include "ServerCollisionSystem.h"
#include "ServerTriggerSystem.h"
#include "SpawnGroupBootstrap.h"
#include "SpawnGroupRuntime.h"
#include "ValtanBrain.h"
#include "WinSockContext.h"
#include "WorldBootstrap.h"
#include "WorldDestructionRuntime.h"
#include "WorldDestructionBootstrapContractTests.h"

#include <Windows.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <span>
#include <thread>
#include <utility>
#include <vector>

namespace
{
	struct TESTS
	{
		void Require(const bool condition, const char* name)
		{
			std::cout << (condition ? "[PASS] " : "[FAILURE] ") << name << '\n';
			if (!condition)
				++failures;
		}
		int failures = 0;
	};

	constexpr std::uint32_t VALID_VALTAN_DEBUG_AUDITION_ROW_COUNT = 68u;

	std::string Build_TestValtanDebugOccurrenceId(const std::uint32_t ordinal)
	{
		std::string occurrenceId = "valtan.video.";
		if (ordinal < 100u)
			occurrenceId.push_back('0');
		if (ordinal < 10u)
			occurrenceId.push_back('0');
		occurrenceId += std::to_string(ordinal);
		return occurrenceId;
	}

	void Write_ValidValtanDebugAuditionRows(std::ofstream& bootstrap)
	{
		bootstrap <<
			"VALTANDEBUGSEQUENCE\tENCOUNTER_VALTAN\tVALTAN_DEBUG_1_67\t67\n";
		for (std::uint32_t ordinal = 1u; ordinal <= 67u; ++ordinal)
		{
			const std::string occurrenceId =
				Build_TestValtanDebugOccurrenceId(ordinal);
			bootstrap <<
				"VALTANDEBUGSTEP\tENCOUNTER_VALTAN\tVALTAN_DEBUG_1_67\t" <<
				ordinal << '\t' << occurrenceId <<
				"\tPRODUCT_CANDIDATE\tVALTAN_TEST\t1\t" <<
				(1u == ordinal ? 160u : 0u) << "\t0\n";
		}
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
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050010, 4 }
	};
}

int LostArk::Server::Run_ServerGameplayContractTests()
{
	using namespace LostArk::Shared;
	TESTS tests{};
	CGameplayCatalog catalog;
	tests.Require(catalog.Load(), "Load gameplay balance bootstrap");
	{
		const VALTAN_DEBUG_AUDITION_DEFINITION* audition =
			catalog.Find_ValtanDebugAudition("ENCOUNTER_VALTAN");
		bool hasExactOrder = nullptr != audition &&
			audition->strSequenceId == "VALTAN_DEBUG_1_67" &&
			audition->Steps.size() == 67u;
		if (hasExactOrder)
		{
			for (std::uint32_t ordinal = 1u; ordinal <= 67u; ++ordinal)
			{
				const VALTAN_DEBUG_AUDITION_STEP& step =
					audition->Steps[ordinal - 1u];
				hasExactOrder = step.iOrdinal == ordinal &&
					step.strOccurrenceId ==
						Build_TestValtanDebugOccurrenceId(ordinal);
				if (!hasExactOrder)
					break;
			}
		}
		tests.Require(
			hasExactOrder && 160u == audition->Steps[0].iTargetHealthBar &&
			14u == audition->Steps[54].iTargetHealthBar &&
			40u == audition->Steps[55].iTargetHealthBar,
			"Load the exact ordered 1-to-67 Valtan Debug audition ledger");
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
		const BOSS_PATTERN_DEFINITION* arenaBreakPattern =
			findPattern("VALTAN_ARENA_BREAK_109");
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
			"Compile all 32 Valtan entry-action timing records from Valtan.skilltiming");
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
			everyDamagingStageResolves && 46u == damagingStageCount &&
			72u == authoredHitPulseCount &&
			700u == catalog.Find_DamageRatePercent(
				"damage.valtan.arena-destroy-109") &&
			450u == catalog.Find_DamageRatePercent(
				"damage.valtan.six-direction-130") &&
			900u == catalog.Find_DamageRatePercent(
				"damage.valtan.ghost-transition-15"),
			"Resolve all 46 Valtan hit stages and 72 pulses through project-tuned damage profiles");
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
		const CLIENT_SESSION_OUTBOUND_METRICS metrics =
			session.Get_OutboundMetrics();
		tests.Require(
			admittedSnapshotBand && admittedReliableReserve &&
			CClientSession::OUTBOUND_ENQUEUE_RESULT::DROPPED_SNAPSHOT ==
				droppedSnapshot &&
			CClientSession::OUTBOUND_ENQUEUE_RESULT::RELIABLE_OVERFLOW ==
				overflow &&
			queuePreservedOnOverflow && publicOverflowFailedClosed &&
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

		bool admittedCleanupReserve = true;
		while (room.m_InboundCommands.size() <
			CGameRoom::MAX_INBOUND_COMMAND_COUNT)
		{
			ROOM_COMMAND command{};
			command.eType = ROOM_COMMAND_TYPE::LEAVE;
			command.iSessionId = static_cast<SESSION_ID>(
				room.m_InboundCommands.size() + 20000u);
			const bool accepted = room.Enqueue(std::move(command));
			admittedCleanupReserve = admittedCleanupReserve && accepted;
			if (!accepted)
				break;
		}
		ROOM_COMMAND rejectedCleanup{};
		rejectedCleanup.eType = ROOM_COMMAND_TYPE::LEAVE;
		rejectedCleanup.iSessionId = 40000u;
		const bool cleanupRejectedAtHardCap =
			!room.Enqueue(std::move(rejectedCleanup)) &&
			CGameRoom::MAX_INBOUND_COMMAND_COUNT ==
				room.m_InboundCommands.size();
		const SERVER_ROOM_PERFORMANCE_METRICS metrics =
			room.Get_PerformanceMetrics();
		tests.Require(
			admittedBestEffort && bestEffortDropWasNonFatal &&
			admittedReliableReserve && reliableRejectedWithoutMutation &&
			admittedCleanupReserve && cleanupRejectedAtHardCap &&
			1u == metrics.iDroppedBestEffortCommandCount &&
			1u == metrics.iRejectedReliableCommandCount &&
			1u == metrics.iRejectedCleanupCommandCount &&
			CGameRoom::MAX_INBOUND_COMMAND_COUNT ==
				metrics.iIngressHighWatermark,
			"Bound ingress, reserve reliable and cleanup capacity, and preserve the queue on overflow");
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
		CMonsterBrain monsterBrain;
		monsterBrain.Update(
			monster, players, catalog, navigation, 1.f / 30.f, 1u, damageEvents);
		const bool ignoredProtectedPlayer =
			INVALID_NET_ENTITY_ID == monster.iTargetEntityId &&
			SERVER_ENTITY_ACTION::IDLE == monster.eAction;
		players.begin()->second.isCombatReady = true;
		monsterBrain.Update(
			monster, players, catalog, navigation, 1.f / 30.f, 2u, damageEvents);
		tests.Require(
			navigationLoaded && ignoredProtectedPlayer &&
			SERVER_ENTITY_ACTION::PATTERN_WINDUP == monster.eAction &&
			players.begin()->second.iNetEntityId == monster.iTargetEntityId,
			"Ignore protected players and acquire the same player after combat admission");
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
		quickCommand.fAimX = 1.f;
		quickCommand.fAimZ = 0.f;
		CPlayerSkillSystem quickSkillSystem;
		tests.Require(
			quickSkillSystem.Try_Start(
				quickPlayer,
				quickCommand,
				catalog,
				1),
			"Approve playable skill command");
	}
	for (const BASIC_ATTACK_CONTRACT& contract : BASIC_ATTACKS)
	{
		const PLAYER_SKILL_DEFINITION* combo = catalog.Find_Skill(contract.skillId);
		tests.Require(
			nullptr != combo &&
			combo->eCharacterClass == contract.characterClass &&
			combo->strInputSlot == "LMB" &&
			PLAYER_SKILL_KIND::COMBO == combo->eSkillKind &&
			combo->ComboStages.size() == contract.stageCount &&
			0u == combo->ComboStages.back().iInputCloseMs,
			"Resolve playable basic attack combo");
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

		const PLAYER_COMBO_STAGE& firstStage = combo->ComboStages.front();
		comboPlayer.fActionElapsedSeconds =
			static_cast<float>(firstStage.iInputOpenMs +
				firstStage.iInputCloseMs) * 0.0005f;
		press.iClientSequence = 2;
		comboSystem.Try_Start(comboPlayer, press, catalog, 11);
		tests.Require(
			comboPlayer.hasBufferedComboInput,
			"Buffer playable basic attack inside its input window");

		std::vector<SERVER_WORLD_ENTITY> noTargets;
		std::vector<DAMAGE_EVENT> noDamageEvents;
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
			"Advance playable basic attack from Server-owned combo window");

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
		one box per 109
		outer ring slab now that the ring is complete at thirty, and the two
		entrance front walls' own receivers. */
		111u == valtanCollisionSystem.Get_CollisionBoxCount() &&
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
	SERVER_BOSS_RECEIVER_HIT receiverHit{};
	tests.Require(
		valtanCollisionSystem.Sweep_BossCircleAgainstReceivers(
			145.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
			175.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
			1.f, receiverHit) &&
		0u == receiverHit.strReceiverPlacementId.rfind(
			"collision.valtan.wallgroup.11047903315509031966.", 0u) &&
		receiverHit.fHitRatio > 0.f && receiverHit.fHitRatio < 1.f &&
		!valtanCollisionSystem.Sweep_BossCircleAgainstReceivers(
			145.f, 100.f, VALTAN_WALL_CENTER_Z,
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
		0.f == impactMotionBoss.fPatternForcedMotionSpeed,
		"Advance the authoritative charge action to GROGGY only after impact");

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
		bernLoaded && bernPlacements.size() == 9u &&
		4u == static_cast<size_t>(std::count_if(
			bernPlacements.begin(), bernPlacements.end(),
			[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
					placement.isEnabled;
			})) &&
		bernNpc != bernPlacements.end() &&
		WORLD_BOOTSTRAP_KIND::NPC == bernNpc->eKind &&
		bernNpc->strArchetypeId == "NPC_BEDA" &&
		bernAylara != bernPlacements.end() &&
		WORLD_BOOTSTRAP_KIND::NPC == bernAylara->eKind &&
		bernAylara->strArchetypeId == "NPC_AYLARA" &&
		bernAylara->isEnabled &&
		bernValtanTrigger != bernPlacements.end() &&
		WORLD_BOOTSTRAP_KIND::TRIGGER_BOX == bernValtanTrigger->eKind &&
		bernValtanTrigger->isEnabled &&
		1u == bernValtanTrigger->TriggerActions.size() &&
		bernCollision != bernPlacements.end() &&
		WORLD_BOOTSTRAP_KIND::COLLISION_BOX == bernCollision->eKind,
		"Load Bern spawns, both NPCs, both triggers, and collision box");
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
	/* 34120 is official rate 361 at attack power 100, split across its three
	authored hit shapes so the sum stays exact. */
	const PLAYER_SKILL_DEFINITION* talonStrike = catalog.Find_Skill(34120);
	tests.Require(
		nullptr != talonStrike && 3u == talonStrike->Hits.size() &&
		talonStrike->Hits[0].iTimeMs < talonStrike->Hits[1].iTimeMs &&
		3u == talonStrike->Hits[0].iAreaType &&
		2u == talonStrike->Hits[2].iAreaType,
		"Load authored hit shapes for the skill from the gameplay bootstrap");
	tests.Require(9639u == entities[0].iCurrentHp,
		"Apply server-authoritative player damage across authored hits");
	std::uint32_t outgoingTotal = 0;
	for (const DAMAGE_EVENT& damageEvent : damageEvents)
		outgoingTotal += damageEvent.iAmount;
	tests.Require(
		3u == damageEvents.size() &&
		361u == outgoingTotal &&
		damageEvents[0].isOutgoing &&
		entities[0].iNetEntityId == damageEvents[0].iTargetNetEntityId,
		"Emit one outgoing damage event per authored hit summing to the profile rate");
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
			30000u == standup->iCooldownMs &&
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
			target.iCurrentHp = 10000;
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
			10000u - tigerEvents[0].iAmount == tigerTargets[0].iCurrentHp,
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
		tests.Require(missEvents.empty() && 10000u == missTargets[0].iCurrentHp,
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
			10000u == nailTargets[1].iCurrentHp,
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

		/* Stage one is 1633 ms long but its hit lands at 470 ms, so a buffered
		press has to cut in there rather than waiting out the clip. 20 ticks is
		about 667 ms: past the hit, nowhere near the full duration. */
		for (std::uint32_t tick = 17; tick < 37; ++tick)
			comboSkills.Update(comboPlayer, comboEntities, catalog, nullptr,
				nullptr, 1.f / 30.f, tick, comboDamageEvents);
		tests.Require(
			2u == comboPlayer.iComboStage &&
			PLAYER_ACTION_STATE::SKILL == comboPlayer.eAction,
			"Cancel into the next combo stage once the hit has landed");

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
	valtan.iCurrentHp = 43125;
	valtan.iMaximumHp = 60000;
	valtan.iMaximumHealthBars = 160;
	valtan.iLastEvaluatedHealthBar = 116;
	valtan.iPhaseTwoHpPercent = 50;
	valtan.iPhase = 1;
	valtan.fPositionX = 151.f;
	valtan.fPositionY = 22.97f;
	valtan.fPositionZ = -122.f;
	valtan.fEngageDistance = 35.f;
	valtan.fMoveSpeed = 3.f;
	/* This fixture asserts the observed 115-bar mechanic, so the encounter intro is
	staged as already consumed exactly as a Debug audition reset does. */
	valtan.bIntroPatternConsumed = true;
	CValtanBrain brain;
	std::vector<DAMAGE_EVENT> valtanDamageEvents;
	brain.Update(valtan, players, catalog, navigation, 0.1f, 99,
		valtanDamageEvents);
	tests.Require(
		SERVER_ENTITY_ACTION::IDLE == valtan.eAction &&
		1000u == players.begin()->second.iCurrentHp &&
		valtanDamageEvents.empty(),
		"Protect Valtan entrant until first accepted gameplay intent");
	players.begin()->second.isCombatReady = true;
	for (std::uint32_t tick = 100; tick < 140 && valtanDamageEvents.empty(); ++tick)
		brain.Update(valtan, players, catalog, navigation, 0.1f, tick,
			valtanDamageEvents);
	tests.Require(781u == players.begin()->second.iCurrentHp,
		"Apply the queued 115-bar Valtan six-direction hit once");
	tests.Require(
		1u == valtanDamageEvents.size() &&
		219u == valtanDamageEvents[0].iAmount &&
		!valtanDamageEvents[0].isOutgoing &&
		players.begin()->second.iNetEntityId ==
			valtanDamageEvents[0].iTargetNetEntityId,
		"Emit one incoming damage event for the 115-bar boss hit");
	tests.Require(
		"VALTAN_FLOOR_WIPE_130" == valtan.strPatternId &&
		valtan.PendingPatternIds.empty() &&
		1u == valtan.TriggeredPatternIds.size() &&
		1u == valtan.iPatternSequence &&
		1u == valtan.iPatternStageIndex,
		"Queue and advance the staged 115-bar scripted mechanic");
	valtan.iCurrentHp = 30000;
	brain.Update(valtan, players, catalog, navigation, 0.1f, 141,
		valtanDamageEvents);
	tests.Require(2u == valtan.iPhase, "Advance Valtan phase from server HP");

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
		std::map<std::string, std::uint32_t> lastStartTickByPattern;
		std::uint32_t previousSequence = 0u;
		bool respectedCooldowns = true;
		bool observedPositiveCooldownRepeat = false;
		bool replayedIntro = false;
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
				1.f / 30.f, tick, cooldownDamageEvents);
			if (cooldownBoss.iPatternSequence == previousSequence)
				continue;
			previousSequence = cooldownBoss.iPatternSequence;
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
			const auto previous =
				lastStartTickByPattern.find(definition->strPatternId);
			if (lastStartTickByPattern.end() != previous &&
				0u != definition->iSourceCooldownTicks)
			{
				observedPositiveCooldownRepeat = true;
				respectedCooldowns = respectedCooldowns &&
					static_cast<std::uint32_t>(tick - previous->second) >=
					definition->iSourceCooldownTicks;
			}
			lastStartTickByPattern[definition->strPatternId] = tick;
		}
		tests.Require(
			respectedCooldowns && observedPositiveCooldownRepeat &&
			!replayedIntro && !cooldownBoss.PatternCooldowns.empty(),
			"Gate normal Valtan reselection by source cooldown and never reroll the intro");
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
	brain.Update(
		openingChargeBoss, players, catalog, navigation,
		1.f / 30.f, 200u, valtanDamageEvents);
	tests.Require(
		openingChargeBoss.strPatternId == "VALTAN_ARMOR_BREAK_OPENING" &&
		openingChargeBoss.strPatternStageId == "WALL_CHARGE" &&
		openingChargeBoss.strActionId ==
			"valtan.mechanic.armor-break-opening.charge" &&
		std::abs(openingChargeBoss.fPatternForcedMotionSpeed -
			(100.f / 1.5f)) <= 0.001f,
		"Use authored charge maximum range instead of stopping at the bait player");

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
		meleeBoss.iCurrentHp = 10000;
		meleeBoss.iMaximumHp = 10000;
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
		tests.Require(10000u - 1050u == meleeEntities[0].iCurrentHp,
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
			[&bootstrapPath](const float durationSeconds)
			{
				std::ofstream bootstrap(bootstrapPath, std::ios::binary);
				bootstrap <<
					"LOSTARK_WORLD_BOOTSTRAP\t6\tVALTAN_ARENA"
					"\tLV_LUT_HEARTRB_ED\t3\t3\n"
					"player.spawn.contract\tplayerSpawn\t-\t-\t0\t0\t0\t0\t1\n"
					"trigger.contract.jump\ttriggerBox\t-\t-\t0\t0\t0\t0\t1"
					"\t2\t2\t2\t0\t1\tmovePlayer\t5\t10\t0\t0\t"
					<< durationSeconds << "\t4\n"
					"collision.contract.wall\tcollisionBox\t-\t-\t4\t1\t0\t0\t1"
					"\t0.5\t1\t2\n";
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
			3u == triggerBootstrap.Get_Placements().size() &&
			WORLD_BOOTSTRAP_KIND::TRIGGER_BOX ==
				triggerBootstrap.Get_Placements()[1].eKind &&
			WORLD_BOOTSTRAP_KIND::COLLISION_BOX ==
				triggerBootstrap.Get_Placements()[2].eKind &&
			1u == triggerBootstrap.Get_Placements()[1].TriggerActions.size(),
			"Parse trigger and collision box from world bootstrap v6");

		writeTriggerBootstrap(-1.f);
		tests.Require(
			!triggerBootstrap.Load(WORLD_ID::VALTAN_ARENA) &&
			3u == triggerBootstrap.Get_Placements().size(),
			"Reject invalid trigger bootstrap without replacing committed world");
		SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
			0u == previousLength || previousLength >= std::size(previousRoot) ?
				nullptr : previousRoot);
		std::error_code cleanupError;
		fs::remove_all(triggerRoot, cleanupError);
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
		/* The Valtan route has no playable monster damage yet. Debug replaces only
		the four progression spawn triggers with authored moves; Release must keep
		the product activation action unchanged. */
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
			std::abs(moving.TriggerMove.fTargetX - 46.741f) < 0.001f &&
			std::abs(moving.TriggerMove.fTargetZ + 61.417f) < 0.001f,
			"Bypass the unkillable Stage_1 group and move toward the next trigger in Debug");
		triggerSystem.Update_PlayerMotion(players.begin()->second, 1.f);
		tests.Require(
			PLAYER_ACTION_STATE::NONE == players.begin()->second.eAction &&
			std::abs(players.begin()->second.fPositionX - 46.741f) < 0.001f &&
			std::abs(players.begin()->second.fPositionZ + 61.417f) < 0.001f,
			"Complete the Debug stage bypass at the authored next-stage approach point");
#else
		tests.Require(
			1u == activationCount &&
			PLAYER_ACTION_STATE::NONE == players.begin()->second.eAction,
			"Keep the original Valtan spawn-group trigger unchanged in Release");
#endif
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
				"LOSTARK_GAMEPLAY_BOOTSTRAP\t11\t75\n"
				"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\t50\n"
				"DAMAGE\tdamage.player.34120\t361\n"
				"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t1\n"
				"PATTERNSOURCE\tENCOUNTER_VALTAN\tVALTAN_TEST\t420601\t12\t5000\t150\t350\t300\t180\n"
				"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\tdamage.player.34120\t2\t242\t1\t2000\n"
				"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
				"SKILL\t34120\tLANCE_MASTER\tQ\tlancemaster.skill.34120\t10000\t2266"
				"\t1510\t2000\t0\t0\t8\tdamage.player.34120\tACTIVE\tLANCE_MASTER_LONG_SPEAR\tNONE\n";
			Write_ValidValtanDebugAuditionRows(bootstrap);
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
		const VALTAN_DEBUG_AUDITION_DEFINITION* committedAudition =
			rollbackCatalog.Find_ValtanDebugAudition("ENCOUNTER_VALTAN");
		const size_t committedAuditionStepCount =
			nullptr != committedAudition ? committedAudition->Steps.size() : 0u;
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
			nullptr != rollbackCatalog.Find_ValtanDebugAudition(
				"ENCOUNTER_VALTAN") &&
			67u == committedAuditionStepCount &&
			committedAuditionStepCount ==
				rollbackCatalog.Find_ValtanDebugAudition(
					"ENCOUNTER_VALTAN")->Steps.size() &&
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
			[&noDamageRoot](const char* hitTimeMs, const char* maximumRange)
		{
			std::error_code prepareError;
			fs::remove_all(noDamageRoot, prepareError);
			fs::create_directories(noDamageRoot / L"Gameplay");
			{
				std::ofstream bootstrap(
					noDamageRoot / L"Gameplay" / L"Gameplay.bootstrap",
					std::ios::binary);
				bootstrap <<
					"LOSTARK_GAMEPLAY_BOOTSTRAP\t11\t75\n"
					"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\t50\n"
					"DAMAGE\tdamage.player.34120\t361\n"
					"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t1\n"
					"PATTERNSOURCE\tENCOUNTER_VALTAN\tVALTAN_TEST\t420601\t12\t5000\t150\t350\t300\t180\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\tdamage.player.34120\t2\t242\t1\t2000\n"
					"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
					"SKILL\t34020\tLANCE_MASTER\tSPACE\tlancemaster.skill.34020"
					"\t8000\t900\t" << hitTimeMs << "\t242\t0\t6\t" << maximumRange <<
					"\t\tACTIVE\tLANCE_MASTER_LONG_SPEAR\tNONE\n";
				Write_ValidValtanDebugAuditionRows(bootstrap);
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
		tests.Require(loadWithMovementSkill("0", "0"),
			"Accept a skill that carries no damage profile");
		tests.Require(!loadWithMovementSkill("0", "3"),
			"Reject a damageless skill that still claims reach");
		tests.Require(!loadWithMovementSkill("400", "0"),
			"Reject a damageless skill that still claims a hit time");
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
					(6u + VALID_VALTAN_DEBUG_AUDITION_ROW_COUNT +
					 sourceRows.size() + wallRows.size()) << "\n"
					"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\t50\n"
					"DAMAGE\tdamage.player.34120\t361\n"
					"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t1\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\tdamage.player.34120\t2\t242\t1\t2000\n"
					"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
					"SKILL\t34020\tLANCE_MASTER\tSPACE\tlancemaster.skill.34020\t8000\t900\t0\t242\t0\t6\t0\t\tACTIVE\tLANCE_MASTER_LONG_SPEAR\tNONE\n";
				for (const std::string& row : sourceRows)
					bootstrap << row << '\n';
				for (const std::string& row : wallRows)
					bootstrap << row << '\n';
				Write_ValidValtanDebugAuditionRows(bootstrap);
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
			loadWithWallContactRows(10u, { validSourceRow }, { validWallRow }),
			"Accept one exact ACTIVE axe wall-contact row");
		tests.Require(
			!loadWithWallContactRows(9u, { validSourceRow }, { validWallRow }),
			"Reject the obsolete gameplay bootstrap version before wall-contact load");
		tests.Require(
			!loadWithWallContactRows(10u, { validSourceRow }, {
				"PATTERNWALLCONTACT\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.wrong" }),
			"Reject a wall-contact row whose action does not exactly join its stage");
		tests.Require(
			!loadWithWallContactRows(
				10u, { validSourceRow }, { validWallRow, validWallRow }),
			"Reject duplicate axe wall-contact ownership atomically");
		tests.Require(
			!loadWithWallContactRows(10u, {}, {}),
			"Reject a boss pattern with no compiled source timing row");
		tests.Require(
			!loadWithWallContactRows(
				10u, { validSourceRow, validSourceRow }, {}),
			"Reject duplicate source timing ownership atomically");
		tests.Require(
			!loadWithWallContactRows(10u, {
				"PATTERNSOURCE\tENCOUNTER_VALTAN\tVALTAN_TEST\t420601\t12\t5000\t149\t350\t300\t180" }, {}),
			"Reject a source cooldown whose 30 Hz tick conversion is inconsistent");
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
		const auto loadWithStageRow = [&](const char* stageIndex)
		{
			fs::remove_all(stageRoot, stagePrepareError);
			fs::create_directories(stageRoot / L"Gameplay");
			{
				std::ofstream bootstrap(
					stageRoot / L"Gameplay" / L"Gameplay.bootstrap",
					std::ios::binary);
				bootstrap <<
					"LOSTARK_GAMEPLAY_BOOTSTRAP\t11\t77\n"
					"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\t50\n"
					"DAMAGE\tdamage.player.34010\t100\n"
					"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t1\n"
					"PATTERNSOURCE\tENCOUNTER_VALTAN\tVALTAN_TEST\t420601\t12\t5000\t150\t350\t300\t180\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\tdamage.player.34010\t0\t0\t0\t0\n"
					"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
					"SKILL\t34010\tLANCE_MASTER\tLMB\tlancemaster.skill.34010"
					"\t0\t1633\t470\t0\t0\t0\t3\tdamage.player.34010\tCOMBO"
					"\tLANCE_MASTER_LONG_SPEAR\tNONE\n"
					"SKILLSTAGE\t34010\t0\t1633\t470\t329\t658\n"
					"SKILLSTAGEROOTMOTION\t34010\t" << stageIndex <<
					"\t2\t0:0:0,1600:1.5:0\n";
				Write_ValidValtanDebugAuditionRows(bootstrap);
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
		tests.Require(loadWithStageRow("0"),
			"Accept a root motion row that names an existing combo stage");
		tests.Require(!loadWithStageRow("1"),
			"Reject a root motion row past the last combo stage");
		std::error_code stageCleanupError;
		fs::remove_all(stageRoot, stageCleanupError);
	}

	{
		/* 절룡세 guards, and a hit taken inside that window is what buys the
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

		holdPlayer.hasReleasedHold = true;
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
		tests.Require(
			loaded && 1u == spawnBootstrap.Get_Revision() && 2u == groups.size() &&
			groups.end() != monsterGroup && groups.end() != minibossGroup &&
			nullptr != monsterAnchor && nullptr != minibossAnchor &&
			nullptr != monsterProfile && nullptr != minibossProfile,
			"Load two Character Select spawn groups, anchors, and profiles");
		tests.Require(
			nullptr != monsterProfile && nullptr != minibossProfile &&
			std::fabs(monsterProfile->fHitKnockbackScale - 1.f) < 0.0001f &&
			0.f == minibossProfile->fHitKnockbackScale,
			"Read the published hit knockback scale for the monster and the miniboss");
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
		const PLAYER_ID allocatorBeforeFull = raidRoom.m_iNextPlayerId;
		const NET_ENTITY_ID entityAllocatorBeforeFull = raidRoom.m_iNextNetEntityId;
		const std::size_t playersBeforeFull = raidRoom.m_Players.size();
		const std::size_t sessionOwnersBeforeFull =
			raidRoom.m_PlayerIdBySessionId.size();
		const std::size_t entityOwnersBeforeFull =
			raidRoom.m_PlayerIdByEntityId.size();
		tests.Require(
			raidRoom.Is_Ready() && 4u == playerSpawns.size() &&
			raidRoom.Is_PlayerAdmissionFull() &&
			nullptr == raidRoom.Find_AvailablePlayerSpawn() &&
			allocatorBeforeFull == raidRoom.m_iNextPlayerId &&
			entityAllocatorBeforeFull == raidRoom.m_iNextNetEntityId &&
			playersBeforeFull == raidRoom.m_Players.size() &&
			sessionOwnersBeforeFull == raidRoom.m_PlayerIdBySessionId.size() &&
			entityOwnersBeforeFull == raidRoom.m_PlayerIdByEntityId.size(),
			"Reject a fifth Valtan admission as room full without mutating room ownership or allocators");

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
		tests.Require(
			spawnBootstrap.Load(WORLD_ID::VALTAN_ARENA) &&
			3u == spawnBootstrap.Get_Groups().size(),
			"Load three authored Valtan spawn groups");
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
			15u == scheduledMonsterCount &&
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
		/* The 109 batch is thirty independent outer ring walls. Interior groups
		are authored but dormant, so none of them may break on this edge. */
		const std::size_t interiorBreakingCount = static_cast<std::size_t>(
			std::count_if(breakingStates.begin(), breakingStates.end(),
				[](const WORLD_DESTRUCTION_GROUP_STATE& state)
				{
					return WORLD_DESTRUCTION_STATE::INTACT != state.eState &&
						0u != state.strGroupId.rfind(
							"destroyable.group.valtan.outerwall109.", 0u);
				}));
		tests.Require(
			applied && 30u == breakingCount && 0u == interiorBreakingCount &&
			31u == room.m_iNextWorldDestructionEventSequence,
			"Emit one monotonically sequenced live event for every independent 109-bar wall");

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
			builtFirst && builtRepeated && 30u == firstEvents.size() &&
			firstEvents.size() == repeatedEvents.size() &&
			1u == firstEvents.front().iEventSequence &&
			30u == firstEvents.back().iEventSequence &&
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

#ifdef _DEBUG
	{
		/* Entering Stage_Boss in Debug is the complete manual smoke route: the
		canonical trigger still activates the boss, the player is placed at the
		wall-charge bait point, and the next brain tick queues only 159. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		SERVER_PLAYER player{};
		player.iSessionId = 606u;
		player.iPlayerId = 607u;
		player.iNetEntityId = 608u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = 5500u;
		player.iMaximumHp = 5500u;
		player.isCombatReady = true;
		player.fPositionX = 130.181f;
		player.fPositionY = 23.0607529f;
		player.fPositionZ = -95.8730011f;
		room.m_Players.emplace(player.iPlayerId, player);
		room.Tick(1.f / 30.f);
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		const SERVER_PLAYER& baitPlayer = room.m_Players.begin()->second;
		float baitResolvedX = 0.f;
		float baitResolvedY = 0.f;
		float baitResolvedZ = 0.f;
		bool baitBlocked = false;
		const bool baitIsCollisionClear =
			room.m_ServerCollisionSystem.Resolve_PlayerMove(
				baitPlayer,
				baitPlayer.fPositionX + 0.01f,
				baitPlayer.fPositionY,
				baitPlayer.fPositionZ,
				baitResolvedX, baitResolvedY, baitResolvedZ, baitBlocked) &&
			!baitBlocked;
		/* The first thing a fresh Valtan does is its one entrance sweep, exactly
		as in the original: the 159 charge only follows it. */
		tests.Require(
			room.Is_Ready() && nullptr != boss &&
			"VALTAN_ENTRANCE_WHIRLWIND" ==
				(nullptr == boss ? std::string{} : boss->strPatternId) &&
			159u == (nullptr == boss ? 0u :
				CValtanBrain::Calculate_HealthBar(*boss)) &&
			std::abs(baitPlayer.fPositionX - 154.296f) < 0.001f &&
			std::abs(baitPlayer.fPositionZ + 125.219f) < 0.001f &&
			baitIsCollisionClear,
			"Enter the real Debug Stage_Boss trigger and run the entrance sweep first");

		/* The entrance pattern is 6160ms; drive past it and the 159 opening is
		the next authored crossing, still exactly once. */
		std::string openingPatternId;
		for (std::uint32_t index = 0u; index < 260u; ++index)
		{
			room.Tick(1.f / 30.f);
			boss = room.Find_AuditionBoss();
			if (nullptr == boss)
				break;
			if ("VALTAN_ARMOR_BREAK_OPENING" == boss->strPatternId)
			{
				openingPatternId = boss->strPatternId;
				break;
			}
		}
		tests.Require(
			"VALTAN_ARMOR_BREAK_OPENING" == openingPatternId &&
			nullptr != boss && boss->bIntroPatternConsumed,
			"Follow the consumed entrance sweep with the 159-bar opening exactly once");
	}
#endif

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
				room.m_ServerNavigation, 1.f / 30.f, 500u, auditionDamage);
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
			"Leave the 159, 115 and 100 bar patterns unqueued by a 109-bar audition");
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
		/* The ordered driver owns a fresh arena from its first request. Keep the
		fixture on the room tick boundary: the test observes the stable patterns
		the brain actually starts instead of advancing the private driver by hand. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr SESSION_ID ORDERED_SESSION = 4244u;
		constexpr PLAYER_ID ORDERED_PLAYER = 78u;
		constexpr SESSION_ID SPECTATOR_SESSION = 4245u;
		constexpr PLAYER_ID SPECTATOR_PLAYER = 80u;

		SERVER_PLAYER player{};
		player.iSessionId = ORDERED_SESSION;
		player.iPlayerId = ORDERED_PLAYER;
		player.iNetEntityId = 901u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = 1000u;
		player.iMaximumHp = 1000u;
		player.iCurrentResource = 100u;
		player.iMaximumResource = 100u;
		player.isCombatReady = true;
		player.strSpawnPlacementId = "player_1";
		room.m_Players.emplace(ORDERED_PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(ORDERED_SESSION, ORDERED_PLAYER);

		SERVER_PLAYER spectator{};
		spectator.iSessionId = SPECTATOR_SESSION;
		spectator.iPlayerId = SPECTATOR_PLAYER;
		spectator.iNetEntityId = 903u;
		spectator.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		spectator.eAction = PLAYER_ACTION_STATE::SKILL;
		spectator.iCurrentSkillId = 34120u;
		spectator.fActionElapsedSeconds = 0.5f;
		spectator.isCombatReady = true;
		spectator.TriggerMove.isActive = true;
		spectator.hasMoveGoal = true;
		spectator.MovePath.emplace_back();
		spectator.Projectiles.emplace_back();
		room.m_Players.emplace(SPECTATOR_PLAYER, spectator);
		room.m_PlayerIdBySessionId.emplace(
			SPECTATOR_SESSION, SPECTATOR_PLAYER);

		C2S_VALTAN_AUDITION_REQUEST start{};
		start.iRequestSequence = 1u;
		start.eOperation = VALTAN_AUDITION_OPERATION::PLAY_ORDERED_1_67;
		start.iTargetHealthBar = 0u;
		std::uint32_t reportedBar = 0u;
		const bool beganWithoutBoss = nullptr == room.Find_AuditionBoss();
		const VALTAN_AUDITION_RESULT startResult =
			room.Evaluate_ValtanAudition(
				ORDERED_SESSION, start, reportedBar);
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		tests.Require(
			beganWithoutBoss && VALTAN_AUDITION_RESULT::QUEUED == startResult &&
			nullptr != boss && 160u == reportedBar &&
			CGameRoom::VALTAN_ORDERED_AUDITION_PHASE::READY ==
				room.m_ValtanOrderedAudition.ePhase &&
			ORDERED_SESSION == room.m_ValtanOrderedAudition.iOwnerSessionId &&
			ORDERED_PLAYER == room.m_ValtanOrderedAudition.iOwnerPlayerId &&
			(nullptr == boss ||
				boss->iNetEntityId ==
					room.m_ValtanOrderedAudition.iBossEntityId),
			"Ordered 1-67 auto-activates Valtan and records its owner");
		const SERVER_PLAYER& frozenSpectator =
			room.m_Players.at(SPECTATOR_PLAYER);
		tests.Require(
			PLAYER_ACTION_STATE::NONE == frozenSpectator.eAction &&
			INVALID_SKILL_ID == frozenSpectator.iCurrentSkillId &&
			frozenSpectator.Projectiles.empty() &&
			!frozenSpectator.TriggerMove.isActive &&
			!frozenSpectator.hasMoveGoal && frozenSpectator.MovePath.empty() &&
			!frozenSpectator.isCombatReady,
			"Ordered 1-67 cancels and freezes every non-owner player action");

		const std::uint32_t sequenceBeforeFirstTick =
			nullptr == boss ? 0u : boss->iPatternSequence;
		room.Tick(1.f / 30.f);
		boss = room.Find_AuditionBoss();
		tests.Require(
			nullptr != boss &&
			sequenceBeforeFirstTick + 1u ==
				(nullptr == boss ? 0u : boss->iPatternSequence) &&
			"VALTAN_ENTRANCE_WHIRLWIND" ==
				(nullptr == boss ? std::string{} : boss->strPatternId) &&
			(nullptr == boss || boss->PendingPatternIds.empty()) &&
			CGameRoom::VALTAN_ORDERED_AUDITION_PHASE::WAITING_PATTERN_FINISH ==
				room.m_ValtanOrderedAudition.ePhase &&
			"VALTAN_ENTRANCE_WHIRLWIND" ==
				room.m_ValtanOrderedAudition.strExpectedPatternId,
			"Ordered 1-67 starts the first stable ledger pattern on the next room tick");

		const std::uint32_t patternSequenceBeforeDuplicate =
			nullptr == boss ? 0u : boss->iPatternSequence;
		const SERVER_ENTITY_ACTION actionBeforeDuplicate =
			nullptr == boss ? SERVER_ENTITY_ACTION::IDLE : boss->eAction;
		const std::string patternBeforeDuplicate =
			nullptr == boss ? std::string{} : boss->strPatternId;
		const auto phaseBeforeDuplicate = room.m_ValtanOrderedAudition.ePhase;
		const std::uint32_t epochBeforeRejectedOperation =
			room.m_WorldDestructionRuntime.Get_EncounterEpoch();
		C2S_VALTAN_AUDITION_REQUEST legacyPlay{};
		legacyPlay.iRequestSequence = 2u;
		legacyPlay.eOperation = VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR;
		legacyPlay.iTargetHealthBar = 109u;
		const VALTAN_AUDITION_RESULT legacyPlayResult =
			room.Evaluate_ValtanAudition(
				ORDERED_SESSION, legacyPlay, reportedBar);
		const VALTAN_AUDITION_RESULT retriedLegacyPlayResult =
			room.Evaluate_ValtanAudition(
				ORDERED_SESSION, legacyPlay, reportedBar);
		boss = room.Find_AuditionBoss();
		tests.Require(
			VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE ==
				legacyPlayResult &&
			VALTAN_AUDITION_RESULT::DUPLICATE_IGNORED ==
				retriedLegacyPlayResult &&
			epochBeforeRejectedOperation ==
				room.m_WorldDestructionRuntime.Get_EncounterEpoch() &&
			patternSequenceBeforeDuplicate ==
				(nullptr == boss ? 0u : boss->iPatternSequence) &&
			patternBeforeDuplicate ==
				(nullptr == boss ? std::string{} : boss->strPatternId) &&
			phaseBeforeDuplicate == room.m_ValtanOrderedAudition.ePhase,
			"Ordered 1-67 rejects and remembers a legacy audition operation without mutation");

		start.iRequestSequence = 3u;
		const VALTAN_AUDITION_RESULT duplicateStartResult =
			room.Evaluate_ValtanAudition(
				ORDERED_SESSION, start, reportedBar);
		boss = room.Find_AuditionBoss();
		tests.Require(
			VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE ==
				duplicateStartResult &&
			nullptr != boss &&
			patternSequenceBeforeDuplicate ==
				(nullptr == boss ? 0u : boss->iPatternSequence) &&
			actionBeforeDuplicate ==
				(nullptr == boss ? SERVER_ENTITY_ACTION::IDLE : boss->eAction) &&
			patternBeforeDuplicate ==
				(nullptr == boss ? std::string{} : boss->strPatternId) &&
			phaseBeforeDuplicate == room.m_ValtanOrderedAudition.ePhase &&
			ORDERED_SESSION == room.m_ValtanOrderedAudition.iOwnerSessionId,
			"Ordered 1-67 rejects a second start without resetting the live run");

		const std::vector<std::string> expectedFirstPatterns
		{
			"VALTAN_ENTRANCE_WHIRLWIND",
			"VALTAN_DASH_CHARGE",
			"VALTAN_HIGH_JUMP",
			"VALTAN_WHIRLWIND",
			"VALTAN_DASH_CHARGE",
			"VALTAN_FOUR_SLASH"
		};
		std::vector<std::string> startedPatterns;
		if (nullptr != boss)
			startedPatterns.push_back(boss->strPatternId);
		std::uint32_t observedPatternSequence =
			nullptr == boss ? 0u : boss->iPatternSequence;
		bool reachedFirstMarkerPause = false;
		for (std::uint32_t tick = 0u;
			tick < 3000u && !reachedFirstMarkerPause; ++tick)
		{
			room.Tick(1.f / 30.f);
			boss = room.Find_AuditionBoss();
			if (nullptr == boss)
				break;
			if (boss->iPatternSequence != observedPatternSequence)
			{
				observedPatternSequence = boss->iPatternSequence;
				startedPatterns.push_back(boss->strPatternId);
			}
			reachedFirstMarkerPause =
				7u == room.m_ValtanOrderedAudition.iStepIndex &&
				CGameRoom::VALTAN_ORDERED_AUDITION_PHASE::PAUSE ==
					room.m_ValtanOrderedAudition.ePhase &&
				SERVER_ENTITY_ACTION::IDLE == boss->eAction &&
				boss->strPatternId.empty() &&
				boss->PendingPatternIds.empty();
		}
		tests.Require(
			reachedFirstMarkerPause &&
			expectedFirstPatterns == startedPatterns &&
			room.m_ValtanOrderedAudition.strExpectedPatternId.empty(),
			"Ordered 1-67 preserves marker 7 as idle and excludes random patterns");

		C2S_VALTAN_AUDITION_REQUEST stop{};
		room.m_iPillarAuditionBreakTick = room.m_iServerTick + 10u;
		room.m_bPillarAuditionCycleArmed = true;
		stop.iRequestSequence = 4u;
		stop.eOperation = VALTAN_AUDITION_OPERATION::STOP_ORDERED_1_67;
		stop.iTargetHealthBar = 0u;
		const VALTAN_AUDITION_RESULT stopResult =
			room.Evaluate_ValtanAudition(
				ORDERED_SESSION, stop, reportedBar);
		boss = room.Find_AuditionBoss();
		const SERVER_PLAYER& stoppedOwner = room.m_Players.at(ORDERED_PLAYER);
		tests.Require(
			VALTAN_AUDITION_RESULT::QUEUED == stopResult &&
			CGameRoom::VALTAN_ORDERED_AUDITION_PHASE::INACTIVE ==
				room.m_ValtanOrderedAudition.ePhase &&
			0u == room.m_ValtanOrderedAudition.iOwnerSessionId &&
			INVALID_PLAYER_ID ==
				room.m_ValtanOrderedAudition.iOwnerPlayerId &&
			INVALID_NET_ENTITY_ID ==
				room.m_ValtanOrderedAudition.iBossEntityId &&
			!stoppedOwner.isCombatReady &&
			0u == room.m_iPillarAuditionBreakTick &&
			!room.m_bPillarAuditionCycleArmed &&
			(nullptr == boss || boss->PendingPatternIds.empty()),
			"Ordered 1-67 stop clears the run owner and leaves the boss unqueued");
	}

	{
		/* Executable rows may deliberately have no inter-occurrence pause. The
		publisher and catalog already accept zero; fresh runtime preflight must do
		the same before it auto-activates the disabled boss placement. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr SESSION_ID SESSION = 4250u;
		constexpr PLAYER_ID PLAYER = 85u;
		SERVER_PLAYER player{};
		player.iSessionId = SESSION;
		player.iPlayerId = PLAYER;
		player.iNetEntityId = 910u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		room.m_Players.emplace(PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(SESSION, PLAYER);
		auto* audition = const_cast<VALTAN_DEBUG_AUDITION_DEFINITION*>(
			room.m_GameplayCatalog.Find_ValtanDebugAudition(
				"ENCOUNTER_VALTAN"));
		if (nullptr != audition)
			audition->Steps.front().iPauseAfterMs = 0u;
		C2S_VALTAN_AUDITION_REQUEST start{};
		start.iRequestSequence = 1u;
		start.eOperation = VALTAN_AUDITION_OPERATION::PLAY_ORDERED_1_67;
		std::uint32_t reportedBar = 0u;
		tests.Require(
			nullptr != audition &&
			VALTAN_AUDITION_RESULT::QUEUED ==
				room.Evaluate_ValtanAudition(
					SESSION, start, reportedBar) &&
			nullptr != room.Find_AuditionBoss(),
			"Ordered 1-67 accepts an executable occurrence with zero pause");
	}

	{
		/* Catalog rejection is resolved against a staged boss before the disabled
		placement is activated. Only the processed request ledger may change. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr SESSION_ID SESSION = 4251u;
		constexpr PLAYER_ID PLAYER = 86u;
		SERVER_PLAYER player{};
		player.iSessionId = SESSION;
		player.iPlayerId = PLAYER;
		player.iNetEntityId = 911u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.eAction = PLAYER_ACTION_STATE::SKILL;
		player.iCurrentSkillId = 34120u;
		player.Projectiles.emplace_back();
		room.m_Players.emplace(PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(SESSION, PLAYER);
		auto* audition = const_cast<VALTAN_DEBUG_AUDITION_DEFINITION*>(
			room.m_GameplayCatalog.Find_ValtanDebugAudition(
				"ENCOUNTER_VALTAN"));
		if (nullptr != audition)
			audition->Steps.front().iOrdinal = 0u;
		const NET_ENTITY_ID nextEntityBefore = room.m_iNextNetEntityId;
		const std::uint32_t epochBefore =
			room.m_WorldDestructionRuntime.Get_EncounterEpoch();
		C2S_VALTAN_AUDITION_REQUEST start{};
		start.iRequestSequence = 1u;
		start.eOperation = VALTAN_AUDITION_OPERATION::PLAY_ORDERED_1_67;
		std::uint32_t reportedBar = 0u;
		const VALTAN_AUDITION_RESULT result =
			room.Evaluate_ValtanAudition(SESSION, start, reportedBar);
		const SERVER_PLAYER& unchangedPlayer = room.m_Players.at(PLAYER);
		tests.Require(
			nullptr != audition &&
			VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE == result &&
			nullptr == room.Find_AuditionBoss() &&
			nextEntityBefore == room.m_iNextNetEntityId &&
			epochBefore == room.m_WorldDestructionRuntime.Get_EncounterEpoch() &&
			CGameRoom::VALTAN_ORDERED_AUDITION_PHASE::INACTIVE ==
				room.m_ValtanOrderedAudition.ePhase &&
			PLAYER_ACTION_STATE::SKILL == unchangedPlayer.eAction &&
			34120u == unchangedPlayer.iCurrentSkillId &&
			1u == unchangedPlayer.Projectiles.size(),
			"Ordered 1-67 rejects invalid fresh preflight without activating or resetting the arena");
	}

	{
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
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
		stop.eOperation = VALTAN_AUDITION_OPERATION::STOP_ORDERED_1_67;
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
		/* Occurrence 40 repeats HIGH_JUMP twice. Jump directly to that authored
		row so the test can distinguish an immediate repeat from its one final
		250 ms occurrence pause without replaying the preceding recording. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr SESSION_ID SESSION = 4253u;
		constexpr PLAYER_ID PLAYER = 88u;
		SERVER_PLAYER player{};
		player.iSessionId = SESSION;
		player.iPlayerId = PLAYER;
		player.iNetEntityId = 913u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		room.m_Players.emplace(PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(SESSION, PLAYER);
		C2S_VALTAN_AUDITION_REQUEST start{};
		start.iRequestSequence = 1u;
		start.eOperation = VALTAN_AUDITION_OPERATION::PLAY_ORDERED_1_67;
		std::uint32_t reportedBar = 0u;
		const bool started = VALTAN_AUDITION_RESULT::QUEUED ==
			room.Evaluate_ValtanAudition(SESSION, start, reportedBar);
		room.m_ValtanOrderedAudition.iStepIndex = 39u;
		room.Tick(1.f / 30.f);
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		const std::uint32_t firstSequence =
			nullptr == boss ? 0u : boss->iPatternSequence;
		bool firstRepeatFinished = false;
		for (std::uint32_t tick = 0u;
			tick < 500u && !firstRepeatFinished; ++tick)
		{
			room.Tick(1.f / 30.f);
			boss = room.Find_AuditionBoss();
			firstRepeatFinished = nullptr != boss &&
				39u == room.m_ValtanOrderedAudition.iStepIndex &&
				1u == room.m_ValtanOrderedAudition.iRepeatIndex &&
				CGameRoom::VALTAN_ORDERED_AUDITION_PHASE::READY ==
					room.m_ValtanOrderedAudition.ePhase &&
				SERVER_ENTITY_ACTION::IDLE == boss->eAction &&
				boss->strPatternId.empty();
		}
		const std::uint32_t firstFinishTick = room.m_iServerTick;
		room.Tick(1.f / 30.f);
		boss = room.Find_AuditionBoss();
		const std::uint32_t expectedSecondStartTick =
			(std::numeric_limits<std::uint32_t>::max)() == firstFinishTick ?
			1u : firstFinishTick + 1u;
		const bool repeatedImmediately = nullptr != boss &&
			firstSequence + 1u == boss->iPatternSequence &&
			"VALTAN_HIGH_JUMP" == boss->strPatternId &&
			expectedSecondStartTick == boss->iActionStartTick;
		bool occurrencePaused = false;
		for (std::uint32_t tick = 0u;
			tick < 500u && !occurrencePaused; ++tick)
		{
			room.Tick(1.f / 30.f);
			boss = room.Find_AuditionBoss();
			occurrencePaused = nullptr != boss &&
				40u == room.m_ValtanOrderedAudition.iStepIndex &&
				0u == room.m_ValtanOrderedAudition.iRepeatIndex &&
				CGameRoom::VALTAN_ORDERED_AUDITION_PHASE::PAUSE ==
					room.m_ValtanOrderedAudition.ePhase &&
				room.m_ValtanOrderedAudition.iPauseUntilTick != 0u &&
				SERVER_ENTITY_ACTION::IDLE == boss->eAction;
		}
		tests.Require(
			started && nullptr != boss &&
			"VALTAN_HIGH_JUMP" ==
				(nullptr == boss ? std::string{} :
					boss->strLastPatternId) &&
			firstRepeatFinished && repeatedImmediately && occurrencePaused,
			"Ordered 1-67 repeats without an inner pause and pauses once after the occurrence");
	}

	{
		/* Run the authored ledger from a second fresh room all the way through.
		   This fixture never advances private driver fields: every observation is
		   made after the real room tick, Valtan brain and environment hooks. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr SESSION_ID SESSION = 4254u;
		constexpr PLAYER_ID PLAYER = 89u;
		constexpr std::uint32_t MAX_ORDERED_TICKS = 9000u;

		SERVER_PLAYER player{};
		player.iSessionId = SESSION;
		player.iPlayerId = PLAYER;
		player.iNetEntityId = 914u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = 1000u;
		player.iMaximumHp = 1000u;
		player.iCurrentResource = 100u;
		player.iMaximumResource = 100u;
		player.isCombatReady = true;
		player.strSpawnPlacementId = "player_1";
		room.m_Players.emplace(PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(SESSION, PLAYER);

		const VALTAN_DEBUG_AUDITION_DEFINITION* definition =
			room.m_GameplayCatalog.Find_ValtanDebugAudition(
				"ENCOUNTER_VALTAN");
		std::vector<std::uint32_t> expectedOrdinals;
		std::vector<std::uint32_t> expectedRepeatIndices;
		std::vector<std::string> expectedPatternIds;
		std::vector<std::uint32_t> expectedIdleOrdinals;
		if (nullptr != definition)
		{
			for (const VALTAN_DEBUG_AUDITION_STEP& step : definition->Steps)
			{
				const bool isIdle =
					VALTAN_DEBUG_AUDITION_MAPPING::MARKER == step.eMapping ||
					VALTAN_DEBUG_AUDITION_MAPPING::UNRESOLVED == step.eMapping;
				if (isIdle)
				{
					expectedIdleOrdinals.push_back(step.iOrdinal);
					continue;
				}
				for (std::uint32_t repeatIndex = 0u;
					repeatIndex < step.iRepeat; ++repeatIndex)
				{
					expectedOrdinals.push_back(step.iOrdinal);
					expectedRepeatIndices.push_back(repeatIndex);
					expectedPatternIds.push_back(step.strPatternId);
				}
			}
		}

		const std::vector<std::uint32_t> exactIdleOrdinals
		{
			7u, 9u, 12u, 16u, 28u, 35u, 41u, 46u
		};
		const bool ledgerShapeIsExact = nullptr != definition &&
			67u == definition->Steps.size() &&
			61u == expectedPatternIds.size() &&
			exactIdleOrdinals == expectedIdleOrdinals &&
			2 == std::count(
				expectedOrdinals.begin(), expectedOrdinals.end(), 40u) &&
			2 == std::count(
				expectedOrdinals.begin(), expectedOrdinals.end(), 43u);

		const std::uint32_t destructionEpochBeforeStart =
			room.m_WorldDestructionRuntime.Get_EncounterEpoch();
		const std::uint32_t propEpochBeforeStart =
			room.m_EncounterPropRuntime.Get_EncounterEpoch();
		C2S_VALTAN_AUDITION_REQUEST start{};
		start.iRequestSequence = 1u;
		start.eOperation = VALTAN_AUDITION_OPERATION::PLAY_ORDERED_1_67;
		std::uint32_t reportedBar = 0u;
		const VALTAN_AUDITION_RESULT startResult =
			room.Evaluate_ValtanAudition(SESSION, start, reportedBar);
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		const std::uint32_t firstPatternSequence =
			nullptr == boss ? 0u : boss->iPatternSequence;
		const std::uint32_t runDestructionEpoch =
			room.m_WorldDestructionRuntime.Get_EncounterEpoch();
		const std::uint32_t runPropEpoch =
			room.m_EncounterPropRuntime.Get_EncounterEpoch();

		const auto countFloorStates = [&room]()
		{
			/* rail intact/breaking/despawned, then brick in the same order. */
			std::array<std::size_t, 6u> counts{};
			for (const WORLD_DESTRUCTION_GROUP_STATE& state :
				room.m_WorldDestructionRuntime.Get_GroupStates())
			{
				const bool isRail = 0u == state.strGroupId.rfind(
					"destroyable.group.valtan.floor84.rail.", 0u);
				const bool isBrick = 0u == state.strGroupId.rfind(
					"destroyable.group.valtan.floor30.brick.", 0u);
				if (!isRail && !isBrick)
					continue;
				const std::size_t offset = isRail ? 0u : 3u;
				if (WORLD_DESTRUCTION_STATE::INTACT == state.eState)
					++counts[offset];
				else if (WORLD_DESTRUCTION_STATE::BREAKING == state.eState)
					++counts[offset + 1u];
				else if (WORLD_DESTRUCTION_STATE::DESPAWNED == state.eState)
					++counts[offset + 2u];
			}
			return counts;
		};
		const std::array<std::size_t, 6u> initialFloorStates =
			countFloorStates();
		tests.Require(
			ledgerShapeIsExact &&
			VALTAN_AUDITION_RESULT::QUEUED == startResult &&
			nullptr != boss && 160u == reportedBar &&
			destructionEpochBeforeStart != runDestructionEpoch &&
			propEpochBeforeStart != runPropEpoch &&
			2u == initialFloorStates[0u] &&
			4u == initialFloorStates[3u],
			"Ordered full run stages the exact 61-start and 8-idle ledger in one fresh arena");

		std::vector<std::uint32_t> observedOrdinals;
		std::vector<std::uint32_t> observedRepeatIndices;
		std::vector<std::string> observedPatternIds;
		std::vector<std::uint32_t> observedIdleOrdinals;
		std::vector<std::uint32_t> pillarPatternSequences;
		std::map<std::uint32_t, std::uint8_t> pillarStateMasks;
		std::uint32_t observedPatternSequence = firstPatternSequence;
		std::uint32_t healthAt55 = 0u;
		std::uint32_t healthAt56 = 0u;
		bool startEdgesExact = true;
		bool idleEdgesExact = true;
		bool pillarStatesConsistent = true;
		bool epochsStayedStable = true;
		bool roomStayedReady = true;
		bool sawFloor84Breaking = false;
		bool sawFloor84DespawnedBeforeFloor30 = false;
		bool sawFloor30Breaking = false;
		std::uint32_t ticksUsed = 0u;

		for (; ticksUsed < MAX_ORDERED_TICKS; ++ticksUsed)
		{
			if (CGameRoom::VALTAN_ORDERED_AUDITION_PHASE::COMPLETED_HOLD ==
					room.m_ValtanOrderedAudition.ePhase ||
				CGameRoom::VALTAN_ORDERED_AUDITION_PHASE::FAILED_HOLD ==
					room.m_ValtanOrderedAudition.ePhase)
			{
				break;
			}
			const std::size_t stepIndexBefore =
				room.m_ValtanOrderedAudition.iStepIndex;
			const std::uint32_t sequenceBefore =
				nullptr == boss ? 0u : boss->iPatternSequence;
			room.Tick(1.f / 30.f);
			boss = room.Find_AuditionBoss();
			if (nullptr == boss)
			{
				startEdgesExact = false;
				break;
			}

			epochsStayedStable = epochsStayedStable &&
				runDestructionEpoch ==
					room.m_WorldDestructionRuntime.Get_EncounterEpoch() &&
				runPropEpoch == room.m_EncounterPropRuntime.Get_EncounterEpoch();
			roomStayedReady = roomStayedReady && room.Is_Ready() &&
				CGameRoom::VALTAN_ORDERED_AUDITION_PHASE::FAILED_HOLD !=
					room.m_ValtanOrderedAudition.ePhase;

			if (boss->iPatternSequence != observedPatternSequence)
			{
				const std::size_t runtimeStepIndex =
					room.m_ValtanOrderedAudition.iStepIndex;
				const bool hasStep = nullptr != definition &&
					runtimeStepIndex < definition->Steps.size();
				startEdgesExact = startEdgesExact && hasStep &&
					boss->iPatternSequence == sequenceBefore + 1u &&
					CGameRoom::VALTAN_ORDERED_AUDITION_PHASE::WAITING_PATTERN_FINISH ==
						room.m_ValtanOrderedAudition.ePhase;
				if (hasStep)
				{
					const VALTAN_DEBUG_AUDITION_STEP& step =
						definition->Steps[runtimeStepIndex];
					startEdgesExact = startEdgesExact &&
						boss->strPatternId == step.strPatternId &&
						room.m_ValtanOrderedAudition.strExpectedPatternId ==
							step.strPatternId;
					observedOrdinals.push_back(step.iOrdinal);
					observedRepeatIndices.push_back(
						room.m_ValtanOrderedAudition.iRepeatIndex);
					observedPatternIds.push_back(boss->strPatternId);
					const std::uint32_t healthBar =
						CValtanBrain::Calculate_HealthBar(*boss);
					if (55u == step.iOrdinal)
						healthAt55 = healthBar;
					else if (56u == step.iOrdinal)
						healthAt56 = healthBar;
					if (25u == step.iOrdinal || 33u == step.iOrdinal ||
						49u == step.iOrdinal)
					{
						pillarPatternSequences.push_back(
							boss->iPatternSequence);
					}
				}
				observedPatternSequence = boss->iPatternSequence;
			}

			if (nullptr != definition &&
				stepIndexBefore < definition->Steps.size())
			{
				const VALTAN_DEBUG_AUDITION_STEP& previousStep =
					definition->Steps[stepIndexBefore];
				const bool wasIdle =
					VALTAN_DEBUG_AUDITION_MAPPING::MARKER ==
						previousStep.eMapping ||
					VALTAN_DEBUG_AUDITION_MAPPING::UNRESOLVED ==
						previousStep.eMapping;
				if (wasIdle &&
					room.m_ValtanOrderedAudition.iStepIndex ==
						stepIndexBefore + 1u)
				{
					idleEdgesExact = idleEdgesExact &&
						boss->iPatternSequence == sequenceBefore &&
						CGameRoom::VALTAN_ORDERED_AUDITION_PHASE::PAUSE ==
							room.m_ValtanOrderedAudition.ePhase &&
						boss->strPatternId.empty() &&
						boss->PendingPatternIds.empty();
					observedIdleOrdinals.push_back(previousStep.iOrdinal);
				}
			}

			const std::uint32_t propOccurrence =
				room.m_EncounterPropRuntime.Get_OccurrenceSequence();
			if (0u != propOccurrence)
			{
				const std::vector<ENCOUNTER_PROP_SLOT_STATE>& slots =
					room.m_EncounterPropRuntime.Get_SlotStates();
				pillarStatesConsistent = pillarStatesConsistent &&
					4u == slots.size() &&
					std::all_of(slots.begin(), slots.end(),
						[propOccurrence](const ENCOUNTER_PROP_SLOT_STATE& slot)
						{
							return slot.iOccurrenceSequence == propOccurrence;
						});
				if (std::all_of(slots.begin(), slots.end(),
					[](const ENCOUNTER_PROP_SLOT_STATE& slot)
					{
						return ENCOUNTER_PROP_STATE::INTACT == slot.eState;
					}))
				{
					pillarStateMasks[propOccurrence] |= 1u;
				}
				else if (std::all_of(slots.begin(), slots.end(),
					[](const ENCOUNTER_PROP_SLOT_STATE& slot)
					{
						return ENCOUNTER_PROP_STATE::BREAKING == slot.eState;
					}))
				{
					pillarStateMasks[propOccurrence] |= 2u;
				}
				else if (std::all_of(slots.begin(), slots.end(),
					[](const ENCOUNTER_PROP_SLOT_STATE& slot)
					{
						return ENCOUNTER_PROP_STATE::HIDDEN == slot.eState;
					}))
				{
					pillarStateMasks[propOccurrence] |= 4u;
				}
				else
				{
					pillarStatesConsistent = false;
				}
			}

			const std::array<std::size_t, 6u> floorStates =
				countFloorStates();
			if (31u == room.m_ValtanOrderedAudition.iStepIndex)
			{
				sawFloor84Breaking = sawFloor84Breaking ||
					2u == floorStates[1u] && 4u == floorStates[3u];
			}
			if (room.m_ValtanOrderedAudition.iStepIndex >= 32u &&
				room.m_ValtanOrderedAudition.iStepIndex <= 47u)
			{
				sawFloor84DespawnedBeforeFloor30 =
					sawFloor84DespawnedBeforeFloor30 ||
					2u == floorStates[2u] && 4u == floorStates[3u];
			}
			if (47u == room.m_ValtanOrderedAudition.iStepIndex)
			{
				sawFloor30Breaking = sawFloor30Breaking ||
					2u == floorStates[2u] && 4u == floorStates[4u];
			}
		}

		boss = room.Find_AuditionBoss();
		const std::array<std::size_t, 6u> finalFloorStates =
			countFloorStates();
		const std::vector<std::uint32_t> exactPillarSequences
		{
			21u, 28u, 43u
		};
		bool pillarCyclesExact = pillarStatesConsistent &&
			exactPillarSequences == pillarPatternSequences &&
			3u == pillarStateMasks.size();
		for (const std::uint32_t sequence : exactPillarSequences)
		{
			const auto observed = pillarStateMasks.find(sequence);
			pillarCyclesExact = pillarCyclesExact &&
				pillarStateMasks.end() != observed && 7u == observed->second;
		}
		const bool finalPillarsHidden = std::all_of(
			room.m_EncounterPropRuntime.Get_SlotStates().begin(),
			room.m_EncounterPropRuntime.Get_SlotStates().end(),
			[](const ENCOUNTER_PROP_SLOT_STATE& slot)
			{
				return ENCOUNTER_PROP_STATE::HIDDEN == slot.eState;
			});
		const bool expandedStartsExact = startEdgesExact &&
			expectedOrdinals == observedOrdinals &&
			expectedRepeatIndices == observedRepeatIndices &&
			expectedPatternIds == observedPatternIds &&
			61u == observedPatternIds.size() &&
			2 == std::count(
				observedOrdinals.begin(), observedOrdinals.end(), 40u) &&
			2 == std::count(
				observedOrdinals.begin(), observedOrdinals.end(), 43u);
		tests.Require(
			expandedStartsExact && idleEdgesExact &&
			expectedIdleOrdinals == observedIdleOrdinals &&
			8u == observedIdleOrdinals.size(),
			"Ordered full run executes exactly 61 stable starts, two repeats each at 40 and 43, and eight idle rows");
		tests.Require(
			14u == healthAt55 && 40u == healthAt56,
			"Ordered full run restores Valtan from 14 to 40 bars between occurrences 55 and 56");
		tests.Require(
			pillarCyclesExact && finalPillarsHidden &&
			0u == room.m_iPillarAuditionBreakTick &&
			!room.m_bPillarAuditionCycleArmed,
			"Ordered full run completes three independent pillar raise, break and hide cycles");
		tests.Require(
			sawFloor84Breaking && sawFloor84DespawnedBeforeFloor30 &&
			sawFloor30Breaking && 2u == finalFloorStates[2u] &&
			4u == finalFloorStates[5u],
			"Ordered full run removes the two floor-84 rails before the four floor-30 bricks");
		tests.Require(
			ticksUsed < MAX_ORDERED_TICKS && roomStayedReady &&
			epochsStayedStable && nullptr != boss &&
			CGameRoom::VALTAN_ORDERED_AUDITION_PHASE::COMPLETED_HOLD ==
				room.m_ValtanOrderedAudition.ePhase &&
			67u == room.m_ValtanOrderedAudition.iStepIndex &&
			0u == room.m_ValtanOrderedAudition.iRepeatIndex &&
			firstPatternSequence + 61u == boss->iPatternSequence &&
			SERVER_ENTITY_ACTION::IDLE == boss->eAction &&
			boss->strPatternId.empty() && boss->PendingPatternIds.empty() &&
			room.m_ValtanOrderedAudition.strExpectedPatternId.empty() &&
			0u == room.m_ValtanOrderedAudition.iExpectedPatternSequence &&
			19u == CValtanBrain::Calculate_HealthBar(*boss) &&
			!room.m_Players.at(PLAYER).isCombatReady,
			"Ordered full run reaches the 67-occurrence completion hold without a random pattern or arena reset");

		const std::uint32_t holdSequence =
			nullptr == boss ? 0u : boss->iPatternSequence;
		const std::uint32_t holdHp = nullptr == boss ? 0u : boss->iCurrentHp;
		for (std::uint32_t tick = 0u; tick < 30u; ++tick)
			room.Tick(1.f / 30.f);
		boss = room.Find_AuditionBoss();
		tests.Require(
			room.Is_Ready() && nullptr != boss &&
			CGameRoom::VALTAN_ORDERED_AUDITION_PHASE::COMPLETED_HOLD ==
				room.m_ValtanOrderedAudition.ePhase &&
			holdSequence == boss->iPatternSequence &&
			holdHp == boss->iCurrentHp && boss->strPatternId.empty() &&
			boss->PendingPatternIds.empty() &&
			runDestructionEpoch ==
				room.m_WorldDestructionRuntime.Get_EncounterEpoch() &&
			runPropEpoch == room.m_EncounterPropRuntime.Get_EncounterEpoch(),
			"Ordered full run holds its completed boss idle for thirty further ticks");

		room.Leave(SESSION, PLAYER_DESPAWN_REASON::LEVEL_CHANGED);
		const std::vector<WORLD_DESTRUCTION_GROUP_STATE> resetGroupStates =
			room.m_WorldDestructionRuntime.Get_GroupStates();
		const bool destructionResetToIntact = std::all_of(
			resetGroupStates.begin(), resetGroupStates.end(),
			[](const WORLD_DESTRUCTION_GROUP_STATE& state)
			{
				return WORLD_DESTRUCTION_STATE::INTACT == state.eState;
			});
		const bool propsResetToHidden = std::all_of(
			room.m_EncounterPropRuntime.Get_SlotStates().begin(),
			room.m_EncounterPropRuntime.Get_SlotStates().end(),
			[](const ENCOUNTER_PROP_SLOT_STATE& slot)
			{
				return ENCOUNTER_PROP_STATE::HIDDEN == slot.eState &&
					0u == slot.iOccurrenceSequence;
			});
		tests.Require(
			room.Is_Ready() && room.m_Players.empty() &&
			room.m_PlayerIdBySessionId.empty() &&
			nullptr == room.Find_AuditionBoss() &&
			CGameRoom::VALTAN_ORDERED_AUDITION_PHASE::INACTIVE ==
				room.m_ValtanOrderedAudition.ePhase &&
			!room.m_ValtanAuditionSequenceBySessionId.contains(SESSION) &&
			runDestructionEpoch !=
				room.m_WorldDestructionRuntime.Get_EncounterEpoch() &&
			runPropEpoch != room.m_EncounterPropRuntime.Get_EncounterEpoch() &&
			destructionResetToIntact && propsResetToHidden &&
			0u == room.m_EncounterPropRuntime.Get_OccurrenceSequence() &&
			0u == room.m_iPillarAuditionBreakTick &&
			!room.m_bPillarAuditionCycleArmed,
			"Ordered full run owner leave resets the empty Valtan arena and audition state");
	}
#endif

	{
		/* The arena floor collapse is the only authored thing that takes ground
		away from a player, and it kills. Navigation owns where the hole is; the
		room owns the descent and the death tick. This runs in both configurations
		because a fall is product gameplay, not a Debug audition. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr PLAYER_ID FALL_PLAYER = 79u;
		constexpr SESSION_ID FALL_SESSION = 4243u;
		/* Exclusively inside navregion.valtan.floor84.rail.7000000000000000001:
		no other authored region owns this cell, so the stage-A collapse is the
		only thing that can open it. */
		constexpr float FALL_RAIL_X = 155.25f;
		constexpr float FALL_RAIL_Z = -107.25f;
		/* The arena core the audition bait stands on. It belongs to no collapse
		region at all and has to stay solid through both stages. */
		constexpr float ARENA_CORE_X = 154.296f;
		constexpr float ARENA_CORE_Z = -125.219f;
		const std::string railConditionId =
			"condition.valtan.floor84.rail.7000000000000000001.collapsed";

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
			"Leave a player standing on an intact stage-A rail sector alone");

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
			"Open a fall region only where the stage-A rail sector collapsed");

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
			0u == revived.iFallDeathTick &&
			room.m_ServerNavigation.Is_PointWalkableExact(
				revived.fPositionX, revived.fPositionZ) &&
			!room.m_ServerNavigation.Is_PointInVoidRegion(
				revived.fPositionX, revived.fPositionZ),
			"Revive a fall death on walkable ground instead of inside the hole");

		room.Tick(1.f / 30.f);
		tests.Require(
			PLAYER_ACTION_STATE::NONE ==
				room.m_Players.at(FALL_PLAYER).eAction,
			"Keep a revived player standing instead of falling again");
	}
	{
		/* The 109 leap. No jump clip exists in the converted Valtan model, so
		the arc itself is Server state and has to be checked as authority, not
		as presentation: it must leave the ground during TAKEOFF and land back
		exactly on the authored placement by IMPACT. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr PLAYER_ID LEAP_PLAYER = 78u;
		const bool activated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* leapBoss = room.Find_AuditionBoss();
		/* Drive one exact pattern: stage the encounter intro as already
		consumed so the first-appearance sweep is not the first sequence. */
		if (nullptr != leapBoss)
			leapBoss->bIntroPatternConsumed = true;

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
		measured centre of the outer ring and deliberately not the boss
		placement: the two sit about 4.8m apart. */
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
			std::sqrt(
				(anchorX - (nullptr == leapBoss ? 0.f : leapBoss->fSpawnPositionX)) *
				(anchorX - (nullptr == leapBoss ? 0.f : leapBoss->fSpawnPositionX)) +
				(anchorZ - (nullptr == leapBoss ? 0.f : leapBoss->fSpawnPositionZ)) *
				(anchorZ - (nullptr == leapBoss ? 0.f : leapBoss->fSpawnPositionZ))) > 1.f,
			"Compile a 109 landing anchor that is separate from the boss placement");

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
					room.m_ServerNavigation, 1.f / 30.f, leapTick++, leapDamage);
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
		descriptor.SlotIds = {
			"pillar.valtan.slot03", "pillar.valtan.slot00",
			"pillar.valtan.slot02", "pillar.valtan.slot01" };

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
		duplicated.SlotIds.push_back("pillar.valtan.slot00");
		CEncounterPropRuntime rejected;
		ENCOUNTER_PROP_SET_DESCRIPTOR nameless = descriptor;
		nameless.SlotIds[1u].clear();
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

		std::string archetypeId;
		const ESTHER_USE_REJECTION wrongSlot =
			valtanRoom.m_EstherSkillSystem.Try_Consume(2u, archetypeId);
		tests.Require(
			ESTHER_USE_REJECTION::UNSUPPORTED_SLOT == wrongSlot &&
			archetypeId.empty() &&
			1000u == valtanRoom.m_EstherSkillSystem.Get_Gauge(),
			"Reject the unextracted Esther slots without touching the gauge");

		const ESTHER_USE_REJECTION disabledWorld =
			bernRoom.m_EstherSkillSystem.Try_Consume(1u, archetypeId);
		tests.Require(
			ESTHER_USE_REJECTION::DISABLED_WORLD == disabledWorld,
			"Reject an Esther use outside the raid world");

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
		SERVER_WORLD_ENTITY* summon = findSummon();
		tests.Require(
			nullptr != summon &&
			0u == valtanRoom.m_EstherSkillSystem.Get_Gauge() &&
			"NPC_59030" == summon->strArchetypeId &&
			WORLD_BOOTSTRAP_KIND::NPC == summon->eKind &&
			std::string(ESTHER_ACTION_APPEAR) == summon->strActionId &&
			std::abs(summon->fPositionX - caster.fPositionX) < 0.001f &&
			std::abs(summon->fPositionZ - caster.fPositionZ) < 0.001f &&
			std::abs(summon->fYawDegrees - 90.f) < 0.01f,
			"Summon Sillian at the caster aimed at the cursor and drain the gauge");

		valtanRoom.Handle_UseEstherSkill(casterSessionId, notFullUse);
		std::size_t summonCount = 0u;
		for (const SERVER_WORLD_ENTITY& entity : valtanRoom.m_WorldEntities)
		{
			if (entity.isEstherSummon)
				++summonCount;
		}
		tests.Require(
			1u == summonCount,
			"Reject a second Esther use on the emptied gauge");

		for (int tick = 0; tick < 25; ++tick)
			valtanRoom.Update_WorldEntities(1.f / 30.f);
		summon = findSummon();
		tests.Require(
			nullptr != summon &&
			std::string(ESTHER_ACTION_STRIKE) == summon->strActionId &&
			SERVER_ENTITY_ACTION::PATTERN_ACTIVE == summon->eAction,
			"Advance the summon from appear into the authored strike");

		for (int tick = 0; tick < 97; ++tick)
			valtanRoom.Update_WorldEntities(1.f / 30.f);
		summon = findSummon();
		const float leaveStartY =
			nullptr != summon ? summon->fPositionY : 0.f;
		tests.Require(
			nullptr != summon &&
			std::string(ESTHER_ACTION_LEAVE) == summon->strActionId &&
			SERVER_ENTITY_ACTION::IDLE == summon->eAction,
			"Advance the summon from the strike into the leave stage");

		for (int tick = 0; tick < 30; ++tick)
			valtanRoom.Update_WorldEntities(1.f / 30.f);
		summon = findSummon();
		tests.Require(
			nullptr != summon && summon->fPositionY > leaveStartY + 2.f,
			"Rise the summon skyward through the leave stage");

		for (int tick = 0; tick < 20; ++tick)
			valtanRoom.Update_WorldEntities(1.f / 30.f);
		tests.Require(
			nullptr == findSummon(),
			"Despawn the summon when the leave stage ends");

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
}

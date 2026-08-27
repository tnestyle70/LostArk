#include "Network/PacketFrame.h"
#include "Network/PacketMessages.h"
#include "Network/PacketReader.h"
#include "Network/SessionDiagnostic.h"
#include "Network/PacketStreamParser.h"
#include "Network/PacketWriter.h"

#include <array>
#include <cstdint>
//이 manip -> 콘솔 출력 형식을 제어한다.
//std::hex        // 이후 숫자를 16진수로 출력
//std::setw(2)    // 다음 값의 최소 폭을 2칸으로 설정
//std::setfill('0') // 빈 칸을 0으로 채움
//std::dec        // 다시 10진수 출력
#include <iomanip>
#include <iostream>
#include <limits>
#include <span>
#include <string>
#include <vector>

using namespace LostArk::Shared;

//ananimous namespace 왜? 
namespace
{
	struct TEST_RUNNER
	{
		void Require(
			bool condition,
			const char* testName)
		{
			if (condition)
			{
				std::cout
					<< "[PASS]"
					<< testName
					<< '\n';
			}
			else
			{
				++iFailureCount;

				std::cout
					<< "[FAILURE]"
					<< testName
					<< '\n';
			}
		}

		std::size_t iFailureCount = {};
	};


	//받은 packet을 읽어서 콘솔창에 출력한다.
	void Print_Bytes(
		const char* label,
		const std::vector<std::uint8_t>& bytes)
	{
		std::cout << label;

		for (const std::uint8_t value : bytes)
		{
			std::cout
				<< std::hex
				<< std::setw(2)
				<< std::setfill('0')
				<< static_cast<int>(value)
				<< ' ';
		}

		std::cout << std::dec << '\n';
	}

	bool Build_EnterWorldPayload(
		const C2S_ENTER_WORLD& message,
		std::vector<std::uint8_t>& payload)
	{
		CPacketWriter writer;

		if (!Write_Message(writer, message))
			return false;

		payload = writer.Get_Buffer();

		return true;
	}

	bool Build_EnterAcceptedPayload(
		const S2C_ENTER_ACCEPTED& message,
		std::vector<std::uint8_t>& payload)
	{
		CPacketWriter writer;

		if (!Write_Message(writer, message))
			return false;

		payload = writer.Get_Buffer();

		return true;
	}
	//플레이어 스폰
	bool Build_EnterRejectedPayload(
		const S2C_ENTER_REJECTED& message,
		std::vector<std::uint8_t>& payload)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
			return false;
		payload = writer.Get_Buffer();
		return true;
	}

	bool Build_PlayerSpawnedPayload(
		const S2C_PLAYER_SPAWNED& message,
		std::vector<std::uint8_t>& payload)
	{
		CPacketWriter writer;

		if (!Write_Message(writer, message))
			return false;

		payload = writer.Get_Buffer();

		return true;
	}
	bool Build_WorldEntitySpawnedPayload(
		const S2C_WORLD_ENTITY_SPAWNED& message,
		std::vector<std::uint8_t>& payload)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
			return false;
		payload = writer.Get_Buffer();
		return true;
	}
	bool Build_WorldEntityDespawnedPayload(
		const S2C_WORLD_ENTITY_DESPAWNED& message,
		std::vector<std::uint8_t>& payload)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
			return false;
		payload = writer.Get_Buffer();
		return true;
	}
	bool Build_CombatObjectSpawnedPayload(
		const S2C_COMBAT_OBJECT_SPAWNED& message,
		std::vector<std::uint8_t>& payload)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
			return false;
		payload = writer.Get_Buffer();
		return true;
	}
	bool Build_CombatObjectDespawnedPayload(
		const S2C_COMBAT_OBJECT_DESPAWNED& message,
		std::vector<std::uint8_t>& payload)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
			return false;
		payload = writer.Get_Buffer();
		return true;
	}
	//플레이어 디스폰
	bool Build_PlayerDespawnedPayload(
		const S2C_PLAYER_DESPAWNED& message,
		std::vector<std::uint8_t>& payload)
	{
		CPacketWriter writer;

		if (!Write_Message(writer, message))
			return false;

		payload = writer.Get_Buffer();
		return true;
	}

	//플레이어 move
	bool Build_MovePayload(
		const C2S_MOVE& message,
		std::vector<std::uint8_t>& payload)
	{
		CPacketWriter writer;

		if (!Write_Message(writer, message))
			return false;

		payload = writer.Get_Buffer();
		return true;
	}

	bool Build_UseSkillPayload(
		const C2S_USE_SKILL& message,
		std::vector<std::uint8_t>& payload)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
			return false;
		payload = writer.Get_Buffer();
		return true;
	}
	//플레이어 payload
	bool Build_WorldSnapshotPayload(
		const S2C_WORLD_SNAPSHOT& message,
		std::vector<std::uint8_t>& payload)
	{
		CPacketWriter writer;

		if (!Write_Message(writer, message))
			return false;

		payload = writer.Get_Buffer();
		return true;
	}

	GameplayDataRevision Make_GameplayDataRevision(
		const std::uint8_t seed)
	{
		GameplayDataRevision revision{};
		for (std::size_t index = 0; index < revision.Bytes.size(); ++index)
		{
			revision.Bytes[index] = static_cast<std::uint8_t>(seed + index);
		}
		return revision;
	}

	S2C_VALTAN_DECISION_TRACE_RESPONSE Make_ValtanDecisionTraceResponse()
	{
		S2C_VALTAN_DECISION_TRACE_RESPONSE response{};
		response.iRequestSequence = 91u;
		response.strBossPlacementId = "boss.valtan.center";
		response.eResult = VALTAN_DECISION_TRACE_QUERY_RESULT::TRACE;
		response.DefinitionRevision = Make_GameplayDataRevision(90u);
		response.Trace.iTraceSequence = 7001u;
		response.Trace.iServerTick = 1440u;
		response.Trace.iPatternSequenceBeforeDecision = 12u;
		response.Trace.iExpectedPatternSequence = 13u;
		response.Trace.iCurrentHp = 880000u;
		response.Trace.iMaximumHp = 1000000u;
		response.Trace.iHealthBar = 140u;
		response.Trace.iGameplayPhase = 1u;
		response.Trace.iTargetNetEntityId = 44u;
		response.Trace.fTargetDistance = 6.25f;
		response.Trace.isIntroPatternConsumed = true;
		response.Trace.iRotationStepIndex = 2u;
		response.Trace.eSource = VALTAN_DECISION_TRACE_SOURCE::WEIGHTED;
		response.Trace.eResult = VALTAN_DECISION_TRACE_RESULT::SELECTED;
		response.Trace.strRotationId = "rotation.valtan.phase-1";
		response.Trace.ePendingSource = VALTAN_DECISION_TRACE_SOURCE::NONE;
		response.Trace.strSelectedPatternId = "VALTAN_WHIRLWIND";
		response.Trace.iRawRandomInput = 0x1122334455667788ull;
		response.Trace.iMixedRandomValue = 0x8877665544332211ull;
		response.Trace.iTotalWeight = 70u;
		response.Trace.iRandomTicket = 42u;

		VALTAN_DECISION_TRACE_CANDIDATE_WIRE donut{};
		donut.strPatternId = "VALTAN_DONUT";
		donut.iExclusionMask = VALTAN_DECISION_TRACE_EXCLUDE_COOLDOWN;
		donut.iAuthoredWeight = 30u;
		donut.iCooldownRemainingTicks = 4u;
		donut.iConsecutiveUses = 1u;
		donut.iMaximumConsecutiveUses = 2u;
		response.Trace.Candidates.push_back(donut);

		VALTAN_DECISION_TRACE_CANDIDATE_WIRE whirlwind{};
		whirlwind.strPatternId = "VALTAN_WHIRLWIND";
		whirlwind.iAuthoredWeight = 70u;
		whirlwind.iEffectiveWeight = 70u;
		whirlwind.iWeightEndExclusive = 70u;
		whirlwind.isSelected = true;
		response.Trace.Candidates.push_back(whirlwind);
		return response;
	}

	std::string Make_CombatRuntimeRevision()
	{
		return std::string(63u, 'a') + "1";
	}

	WORLD_DESTRUCTION_STATE_WIRE Make_DestructionState(
		const char* groupId,
		const WORLD_DESTRUCTION_RUNTIME_STATE state,
		const std::uint32_t version)
	{
		WORLD_DESTRUCTION_STATE_WIRE result{};
		result.strGroupId = groupId;
		result.eState = state;
		result.iStateVersion = version;
		result.iStateStartTick = 100u + version;
		result.iCommitTick =
			WORLD_DESTRUCTION_RUNTIME_STATE::BREAKING == state ?
			150u + version : 0u;
		return result;
	}

	WORLD_DESTRUCTION_EVENT_WIRE Make_DestructionEvent(
		const std::uint64_t sequence)
	{
		WORLD_DESTRUCTION_EVENT_WIRE result{};
		result.iEventSequence = sequence;
		result.strGroupId = "destroyable.group.valtan.wall.3705102";
		result.strMutationId = "mutation.valtan.wall.3705102.break";
		result.strBindingId = "bind.valtan.wall.3705102.impact";
		result.iPatternSequence = 3u;
		result.iSourceNetEntityId = 900u;
		result.iServerTick = 120u;
		result.fImpactOriginX = 163.f;
		result.fImpactOriginY = 23.f;
		result.fImpactOriginZ = -129.f;
		result.fImpactDirectionX = 0.6f;
		result.fImpactDirectionY = 0.f;
		result.fImpactDirectionZ = 0.8f;
		result.iRandomSeed = 12345u;
		return result;
	}

	std::string Make_MaximumStableNetworkId(
		const char fill,
		const std::uint32_t ordinal)
	{
		std::string result(MAX_STABLE_NETWORK_ID_BYTES, fill);
		result[result.size() - 3u] = static_cast<char>(
			'0' + ordinal / 100u % 10u);
		result[result.size() - 2u] = static_cast<char>(
			'0' + ordinal / 10u % 10u);
		result[result.size() - 1u] = static_cast<char>(
			'0' + ordinal % 10u);
		return result;
	}

	S2C_WORLD_DESTRUCTION_DELTA Make_MaximumSizedDestructionDelta(
		const std::size_t stateCount,
		const std::size_t eventCount)
	{
		S2C_WORLD_DESTRUCTION_DELTA result{};
		result.strCombatRuntimeRevision = Make_CombatRuntimeRevision();
		result.iServerTick = 1u;
		result.iEncounterEpoch = 1u;
		result.ChangedStates.reserve(stateCount);
		for (std::size_t index = 0u; index < stateCount; ++index)
		{
			WORLD_DESTRUCTION_STATE_WIRE state{};
			state.strGroupId = Make_MaximumStableNetworkId(
				'g', static_cast<std::uint32_t>(index));
			state.eState = WORLD_DESTRUCTION_RUNTIME_STATE::DESPAWNED;
			state.iStateVersion = static_cast<std::uint32_t>(index) + 1u;
			state.iStateStartTick = static_cast<std::uint32_t>(index) + 1u;
			result.ChangedStates.push_back(std::move(state));
		}
		result.LiveEvents.reserve(eventCount);
		for (std::size_t index = 0u; index < eventCount; ++index)
		{
			WORLD_DESTRUCTION_EVENT_WIRE event = Make_DestructionEvent(index + 1u);
			event.strGroupId = Make_MaximumStableNetworkId('e', 0u);
			event.strMutationId = Make_MaximumStableNetworkId('m', 0u);
			event.strBindingId = Make_MaximumStableNetworkId('b', 0u);
			result.LiveEvents.push_back(std::move(event));
		}
		result.Diagnostics.iLastEventSequence = eventCount;
		return result;
	}

	//월드 진입에 대한 테스트
	void Test_EnterWorldRoundTrip(
		TEST_RUNNER& testRunner)
	{
		C2S_ENTER_WORLD source{};
		source.eWorldId = WORLD_ID::TRAINING_GROUND;

		source.eCharacterClass =
			CHARACTER_CLASS_ID::DIMENSIONMASTER;

		source.strNickName = "건보";

		std::vector<std::uint8_t> payload;

		testRunner.Require(
			Build_EnterWorldPayload(
				source,
				payload),
				"Writer Enter World");

		Print_Bytes(
			"Payload bytes : ",
			payload);

		CPacketReader reader{ payload };
		//character class와 nickname string
		C2S_ENTER_WORLD decoded{};

		testRunner.Require(
			Read_Message(reader, decoded),
			"Read Enter World");

		testRunner.Require(
			decoded.iProtocolVersion == NETWORK_PROTOCOL_VERSION &&
			decoded.eWorldId == WORLD_ID::TRAINING_GROUND,
			"Enter World Contract Round Trip");

		testRunner.Require(
			decoded.eCharacterClass ==
			source.eCharacterClass,
			"Character Class Round Trip");

		testRunner.Require(
			decoded.strNickName ==
			source.strNickName,
			"NickName Round Trip");

		testRunner.Require(
			0 == reader.Get_RemainingSize(),
			"Consume Entire Payload");
	}

	void Test_PlayerNicknameContract(TEST_RUNNER& testRunner)
	{
		const std::string korean = "\xED\x95\x9C\xEA\xB8\x80";
		const std::string exactLimit(MAX_NICKNAME_BYTES, 'N');
		testRunner.Require(
			Is_Valid_PlayerNickname("Test-1234") &&
			Is_Valid_PlayerNickname(korean) &&
			Is_Valid_PlayerNickname(exactLimit),
			"Accept ASCII Korean And Exact-Limit Nicknames");

		const std::vector<std::string> invalidNicknames
		{
			{},
			std::string(MAX_NICKNAME_BYTES + 1u, 'N'),
			std::string("A\0B", 3u),
			" leading",
			"trailing\t",
			"line\nbreak",
			std::string("\xC2\x85", 2u),
			std::string("\x80", 1u),
			std::string("\xC0\xAF", 2u),
			std::string("\xED\xA0\x80", 3u),
			std::string("\xF4\x90\x80\x80", 4u)
		};
		bool allInvalidRejected = true;
		for (const std::string& nickname : invalidNicknames)
		{
			C2S_ENTER_WORLD enter{};
			enter.eWorldId = WORLD_ID::BERN;
			enter.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
			enter.strNickName = nickname;
			CPacketWriter enterWriter;
			allInvalidRejected &= !Write_Message(enterWriter, enter);

			S2C_PLAYER_SPAWNED spawned{};
			spawned.iPlayerId = 1u;
			spawned.iNetEntityId = 101u;
			spawned.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
			spawned.strNickName = nickname;
			CPacketWriter spawnedWriter;
			allInvalidRejected &= !Write_Message(spawnedWriter, spawned);
		}
		testRunner.Require(allInvalidRejected,
			"C2S And Spawn Writers Share Exact Nickname Rejection Policy");

		bool allRoundTripsExact = true;
		for (const std::string& nickname :
			std::vector<std::string>{ "Test-1234", korean, exactLimit })
		{
			C2S_ENTER_WORLD enter{};
			enter.eWorldId = WORLD_ID::BERN;
			enter.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
			enter.strNickName = nickname;
			CPacketWriter enterWriter;
			C2S_ENTER_WORLD decodedEnter{};
			if (!Write_Message(enterWriter, enter))
			{
				allRoundTripsExact = false;
				continue;
			}
			CPacketReader enterReader{ enterWriter.Get_Buffer() };
			allRoundTripsExact &= Read_Message(enterReader, decodedEnter) &&
				0u == enterReader.Get_RemainingSize() &&
				decodedEnter.strNickName == nickname;

			S2C_PLAYER_SPAWNED spawned{};
			spawned.iPlayerId = 2u;
			spawned.iNetEntityId = 202u;
			spawned.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
			spawned.strNickName = nickname;
			CPacketWriter spawnedWriter;
			S2C_PLAYER_SPAWNED decodedSpawned{};
			if (!Write_Message(spawnedWriter, spawned))
			{
				allRoundTripsExact = false;
				continue;
			}
			CPacketReader spawnedReader{ spawnedWriter.Get_Buffer() };
			allRoundTripsExact &=
				Read_Message(spawnedReader, decodedSpawned) &&
				0u == spawnedReader.Get_RemainingSize() &&
				decodedSpawned.strNickName == nickname;
		}
		testRunner.Require(allRoundTripsExact,
			"C2S And Spawn Codecs Preserve Exact Nickname Bytes");

		const std::string malformed("\xC0\xAF", 2u);
		CPacketWriter rawEnter;
		rawEnter.Write_U16(NETWORK_PROTOCOL_VERSION);
		rawEnter.Write_U16(static_cast<std::uint16_t>(WORLD_ID::BERN));
		rawEnter.Write_U8(static_cast<std::uint8_t>(
			CHARACTER_CLASS_ID::ARTIST));
		const bool builtMalformedEnter = rawEnter.Write_String(
			malformed, MAX_NICKNAME_BYTES);
		CPacketReader malformedEnterReader{ rawEnter.Get_Buffer() };
		C2S_ENTER_WORLD unchangedEnter{};
		unchangedEnter.iProtocolVersion = 77u;
		unchangedEnter.eWorldId = WORLD_ID::TRAINING_GROUND;
		unchangedEnter.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
		unchangedEnter.strNickName = "keep-enter";
		testRunner.Require(
			builtMalformedEnter &&
			!Read_Message(malformedEnterReader, unchangedEnter) &&
			77u == unchangedEnter.iProtocolVersion &&
			WORLD_ID::TRAINING_GROUND == unchangedEnter.eWorldId &&
			CHARACTER_CLASS_ID::WARLORD == unchangedEnter.eCharacterClass &&
			"keep-enter" == unchangedEnter.strNickName,
			"Malformed C2S Nickname Decode Preserves Destination");

		CPacketWriter rawSpawn;
		rawSpawn.Write_U32(3u);
		rawSpawn.Write_U32(303u);
		rawSpawn.Write_U8(static_cast<std::uint8_t>(
			CHARACTER_CLASS_ID::ARTIST));
		const bool builtMalformedSpawn = rawSpawn.Write_String(
			malformed, MAX_NICKNAME_BYTES);
		rawSpawn.Write_F32(1.f);
		rawSpawn.Write_F32(2.f);
		rawSpawn.Write_F32(3.f);
		rawSpawn.Write_F32(90.f);
		CPacketReader malformedSpawnReader{ rawSpawn.Get_Buffer() };
		S2C_PLAYER_SPAWNED unchangedSpawn{};
		unchangedSpawn.iPlayerId = 9u;
		unchangedSpawn.iNetEntityId = 909u;
		unchangedSpawn.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
		unchangedSpawn.strNickName = "keep-spawn";
		testRunner.Require(
			builtMalformedSpawn &&
			!Read_Message(malformedSpawnReader, unchangedSpawn) &&
			9u == unchangedSpawn.iPlayerId &&
			909u == unchangedSpawn.iNetEntityId &&
			CHARACTER_CLASS_ID::WARLORD == unchangedSpawn.eCharacterClass &&
			"keep-spawn" == unchangedSpawn.strNickName,
			"Malformed Spawn Nickname Decode Preserves Destination");
	}

	void Test_PlayableCharacterRoster(TEST_RUNNER& testRunner)
	{
		testRunner.Require(
			Is_Supported_Playable_Character_Class(
				CHARACTER_CLASS_ID::LANCE_MASTER) &&
			Is_Supported_Playable_Character_Class(
				CHARACTER_CLASS_ID::GUNSLINGER) &&
			Is_Supported_Playable_Character_Class(
				CHARACTER_CLASS_ID::SLAYER) &&
			Is_Supported_Playable_Character_Class(
				CHARACTER_CLASS_ID::ARTIST) &&
			Is_Supported_Playable_Character_Class(
				CHARACTER_CLASS_ID::DIMENSIONMASTER),
			"Accept Six Playable Character Classes");

		testRunner.Require(
			!Is_Supported_Playable_Character_Class(
				CHARACTER_CLASS_ID::DESTROYER) &&
			!Is_Supported_Playable_Character_Class(
				CHARACTER_CLASS_ID::END),
			"Reject Reserved Character Classes");
	}
	//유효한 입력에 대한 테스트
	void Test_EnterAcceptedRoundTrip(
		TEST_RUNNER& testRunner)
	{
		S2C_ENTER_ACCEPTED source{};

		source.iPlayerId = 1;
		source.iNetEntityId = 100;
		source.ActiveGameplayRevision = Make_GameplayDataRevision(1u);
		source.RequiredPinnedGameplayRevisions.push_back(
			Make_GameplayDataRevision(2u));

		std::vector<std::uint8_t> payload;

		testRunner.Require(
			Build_EnterAcceptedPayload(
				source,
				payload),
			"Writer Enter Accepted");

		Print_Bytes(
			"Accepted payload bytes : ",
			payload);

		testRunner.Require(
			12u + GAMEPLAY_DATA_REVISION_BYTES + 1u +
				GAMEPLAY_DATA_REVISION_BYTES == payload.size(),
			"Enter Accepted Payload Size");

		CPacketReader reader{ payload };

		S2C_ENTER_ACCEPTED decoded{};

		testRunner.Require(
			Read_Message(reader, decoded),
			"Read Enter Accepted");

		testRunner.Require(
			source.iProtocolVersion == decoded.iProtocolVersion &&
			source.eWorldId == decoded.eWorldId,
			"Accepted Contract Round Trip");

		testRunner.Require(
			source.iPlayerId ==
			decoded.iPlayerId,
			"Player ID Round Trip");

		testRunner.Require(
			source.iNetEntityId ==
			decoded.iNetEntityId,
			"Net Entity ID Round Trip");

		testRunner.Require(
			source.ActiveGameplayRevision ==
				decoded.ActiveGameplayRevision &&
			source.RequiredPinnedGameplayRevisions ==
				decoded.RequiredPinnedGameplayRevisions,
			"Enter Accepted Revision Set Round Trip");

		testRunner.Require(
			0 == reader.Get_RemainingSize(),
			"Consume Entire Accepted Payload");
	}
	//플레이어 위치 
	void Test_EnterRejectedRoundTrip(TEST_RUNNER& testRunner)
	{
		S2C_ENTER_REJECTED source{};
		source.eWorldId = WORLD_ID::VALTAN_ARENA;
		source.eReason = ENTER_WORLD_REJECTION_REASON::ROOM_FULL;
		std::vector<std::uint8_t> payload;
		testRunner.Require(
			Build_EnterRejectedPayload(source, payload) && 5u == payload.size(),
			"Writer Enter Rejected Room Full");

		CPacketReader reader{ payload };
		S2C_ENTER_REJECTED decoded{};
		testRunner.Require(
			Read_Message(reader, decoded) &&
			decoded.iProtocolVersion == NETWORK_PROTOCOL_VERSION &&
			decoded.eWorldId == WORLD_ID::VALTAN_ARENA &&
			decoded.eReason == ENTER_WORLD_REJECTION_REASON::ROOM_FULL &&
			0u == reader.Get_RemainingSize(),
			"Enter Rejected Room Full Round Trip");
		testRunner.Require(
			Is_Known_Packet_Type(PACKET_TYPE::S2C_ENTER_REJECTED),
			"Register Enter Rejected Packet Type");

		S2C_ENTER_REJECTED invalid = source;
		invalid.eReason = ENTER_WORLD_REJECTION_REASON::END;
		CPacketWriter invalidReasonWriter;
		testRunner.Require(
			!Write_Message(invalidReasonWriter, invalid),
			"Reject Unknown Enter Rejection Reason From Writer");
		invalid = source;
		invalid.eWorldId = WORLD_ID::END;
		CPacketWriter invalidWorldWriter;
		testRunner.Require(
			!Write_Message(invalidWorldWriter, invalid),
			"Reject Unknown Enter Rejection World From Writer");
		invalid = source;
		invalid.iProtocolVersion = NETWORK_PROTOCOL_VERSION + 1u;
		CPacketWriter invalidProtocolWriter;
		testRunner.Require(
			!Write_Message(invalidProtocolWriter, invalid),
			"Reject Enter Rejection Protocol Mismatch From Writer");

		CPacketWriter unknownReasonPayload;
		unknownReasonPayload.Write_U16(NETWORK_PROTOCOL_VERSION);
		unknownReasonPayload.Write_U16(
			static_cast<std::uint16_t>(WORLD_ID::VALTAN_ARENA));
		unknownReasonPayload.Write_U8(
			static_cast<std::uint8_t>(ENTER_WORLD_REJECTION_REASON::END));
		CPacketReader unknownReasonReader{ unknownReasonPayload.Get_Buffer() };
		S2C_ENTER_REJECTED unchanged{};
		unchanged.iProtocolVersion = 77u;
		unchanged.eWorldId = WORLD_ID::BERN;
		unchanged.eReason = ENTER_WORLD_REJECTION_REASON::ROOM_FULL;
		testRunner.Require(
			!Read_Message(unknownReasonReader, unchanged) &&
			77u == unchanged.iProtocolVersion &&
			WORLD_ID::BERN == unchanged.eWorldId &&
			ENTER_WORLD_REJECTION_REASON::ROOM_FULL == unchanged.eReason,
			"Reject Unknown Enter Rejection Reason Without Mutation");

		payload.pop_back();
		CPacketReader truncatedReader{ payload };
		testRunner.Require(
			!Read_Message(truncatedReader, unchanged) &&
			77u == unchanged.iProtocolVersion &&
			WORLD_ID::BERN == unchanged.eWorldId &&
			ENTER_WORLD_REJECTION_REASON::ROOM_FULL == unchanged.eReason,
			"Reject Truncated Enter Rejection Without Mutation");
	}

	void Test_F32RoundTrip(
		TEST_RUNNER& testRunner)
	{
		CPacketWriter writer;

		writer.Write_F32(1.f);

		const std::vector<std::uint8_t> expectedBytes
		{
			0x00, 0x00, 0x80, 0x3F
		};

		testRunner.Require(
			writer.Get_Buffer() == expectedBytes,
			"F32 IEEE754 Bytes");

		CPacketReader reader
		{
			writer.Get_Buffer()
		};

		float decoded = 0.f;

		testRunner.Require(
			reader.Read_F32(decoded),
			"Read F32");

		testRunner.Require(
			decoded == 1.f,
			"F32 Round Trip");

		testRunner.Require(
			0 == reader.Get_RemainingSize(),
			"Consume Entire F32");

		const std::vector<std::uint8_t> truncatedBytes
		{
			0x00, 0x00, 0x80
		};

		CPacketReader truncatedReader
		{
			truncatedBytes
		};

		float unchanged = 77.f;

		testRunner.Require(
			!truncatedReader.Read_F32(unchanged),
			"Reject Truncated F32");

		testRunner.Require(
			unchanged == 77.f,
			"Failed F32 Does Not Mutate");
	}
	//플레이어 스폰
	void Test_PlayerSpawnedRoundTrip(
		TEST_RUNNER& testRunner)
	{
		S2C_PLAYER_SPAWNED source{};

		source.iPlayerId = 1;
		source.iNetEntityId = 100;
		source.eCharacterClass =
			CHARACTER_CLASS_ID::DIMENSIONMASTER;
		source.strNickName = "건보";
		source.fPositionX = 1.f;
		source.fPositionY = 2.f;
		source.fPositionZ = 3.f;
		source.fYawDegrees = 90.f;

		std::vector<std::uint8_t> payload;

		testRunner.Require(
			Build_PlayerSpawnedPayload(
				source,
				payload),
			"Writer Player Spawned");

		Print_Bytes(
			"Player Spawned payload bytes : ",
			payload);

		const std::vector<std::uint8_t> expectedPayload
		{
			0x01, 0x00, 0x00, 0x00,
			0x64, 0x00, 0x00, 0x00,
			0x05,
			0x06, 0x00,
			0xEA, 0xB1, 0xB4,
			0xEB, 0xB3, 0xB4,
			0x00, 0x00, 0x80, 0x3F,
			0x00, 0x00, 0x00, 0x40,
			0x00, 0x00, 0x40, 0x40,
			0x00, 0x00, 0xB4, 0x42
		};

		testRunner.Require(
			payload.size() == 33,
			"Player Spawned Payload Size");

		testRunner.Require(
			payload == expectedPayload,
			"Player Spawned Payload Layout");

		CPacketReader reader{ payload };
		S2C_PLAYER_SPAWNED decoded{};

		testRunner.Require(
			Read_Message(reader, decoded),
			"Read Player Spawned");

		testRunner.Require(
			decoded.iPlayerId == source.iPlayerId &&
			decoded.iNetEntityId == source.iNetEntityId,
			"Spawned IDs Round Trip");

		testRunner.Require(
			decoded.eCharacterClass == source.eCharacterClass &&
			decoded.strNickName == source.strNickName,
			"Spawned Identity Round Trip");

		testRunner.Require(
			decoded.fPositionX == source.fPositionX &&
			decoded.fPositionY == source.fPositionY &&
			decoded.fPositionZ == source.fPositionZ &&
			decoded.fYawDegrees == source.fYawDegrees,
			"Spawned Transform Round Trip");

		testRunner.Require(
			0 == reader.Get_RemainingSize(),
			"Consume Entire Spawned Payload");
	}
	//유효하지 않은 플레이어 스폰
	void Test_InvalidPlayerSpawnedPayloads(
		TEST_RUNNER& testRunner)
	{
		S2C_PLAYER_SPAWNED valid{};

		valid.iPlayerId = 1;
		valid.iNetEntityId = 100;
		valid.eCharacterClass =
			CHARACTER_CLASS_ID::LANCE_MASTER;
		valid.strNickName = "건보";
		valid.fPositionX = 1.f;
		valid.fPositionY = 2.f;
		valid.fPositionZ = 3.f;
		valid.fYawDegrees = 90.f;

		S2C_PLAYER_SPAWNED invalidId = valid;
		invalidId.iPlayerId = INVALID_PLAYER_ID;

		CPacketWriter invalidIdWriter;

		testRunner.Require(
			!Write_Message(invalidIdWriter, invalidId),
			"Reject Spawned Zero Player ID");

		S2C_PLAYER_SPAWNED invalidClass = valid;
		invalidClass.eCharacterClass =
			static_cast<CHARACTER_CLASS_ID>(0xFF);

		CPacketWriter invalidClassWriter;

		testRunner.Require(
			!Write_Message(invalidClassWriter, invalidClass),
			"Reject Spawned Invalid Class");

		S2C_PLAYER_SPAWNED invalidPosition = valid;
		invalidPosition.fPositionX =
			std::numeric_limits<float>::infinity();

		CPacketWriter invalidPositionWriter;

		testRunner.Require(
			!Write_Message(
				invalidPositionWriter,
				invalidPosition),
			"Reject Spawned Infinite Position");

		std::vector<std::uint8_t> truncatedPayload;

		const bool builtPayload =
			Build_PlayerSpawnedPayload(
				valid,
				truncatedPayload);

		testRunner.Require(
			builtPayload,
			"Build Spawned Truncation Source");

		if (!builtPayload)
			return;

		truncatedPayload.pop_back();

		CPacketReader truncatedReader
		{
			truncatedPayload
		};

		S2C_PLAYER_SPAWNED unchanged{};

		unchanged.iPlayerId = 77;
		unchanged.iNetEntityId = 88;
		unchanged.eCharacterClass =
			CHARACTER_CLASS_ID::ARTIST;
		unchanged.strNickName = "keep";
		unchanged.fPositionX = 10.f;
		unchanged.fPositionY = 20.f;
		unchanged.fPositionZ = 30.f;
		unchanged.fYawDegrees = 40.f;

		testRunner.Require(
			!Read_Message(
				truncatedReader,
				unchanged),
			"Reject Truncated Player Spawned");

		testRunner.Require(
			unchanged.iPlayerId == 77 &&
			unchanged.iNetEntityId == 88 &&
			unchanged.eCharacterClass ==
				CHARACTER_CLASS_ID::ARTIST &&
			unchanged.strNickName == "keep" &&
			unchanged.fPositionX == 10.f &&
			unchanged.fPositionY == 20.f &&
			unchanged.fPositionZ == 30.f &&
			unchanged.fYawDegrees == 40.f,
			"Failed Spawn Does Not Mutate Message");
	}

	void Test_WorldEntitySpawnedRoundTrip(TEST_RUNNER& testRunner)
	{
		S2C_WORLD_ENTITY_SPAWNED source{};
		source.iNetEntityId = 900;
		source.eKind = WORLD_ENTITY_KIND::BOSS;
		source.strArchetypeId = "BOSS_VALTAN";
		source.strEncounterId = "ENCOUNTER_VALTAN";
		source.strPlacementId = "boss_valtan_1";
		source.strActionId = "esther.strike";
		source.fPositionX = 151.25f;
		source.fPositionY = 22.97f;
		source.fPositionZ = -121.75f;
		source.fYawDegrees = 225.f;
		source.fCollisionRadius = 3.f;

		std::vector<std::uint8_t> payload;
		testRunner.Require(
			Build_WorldEntitySpawnedPayload(source, payload),
			"Writer World Entity Spawned");
		CPacketReader reader{ payload };
		S2C_WORLD_ENTITY_SPAWNED decoded{};
		testRunner.Require(
			Read_Message(reader, decoded),
			"Read World Entity Spawned");
		testRunner.Require(
			decoded.iNetEntityId == source.iNetEntityId &&
			decoded.eKind == source.eKind &&
			decoded.strArchetypeId == source.strArchetypeId &&
			decoded.strEncounterId == source.strEncounterId &&
			decoded.strPlacementId == source.strPlacementId &&
			decoded.strActionId == source.strActionId &&
			decoded.fPositionX == source.fPositionX &&
			decoded.fYawDegrees == source.fYawDegrees &&
			decoded.fCollisionRadius == source.fCollisionRadius,
			"World Entity Spawned Round Trip");
		testRunner.Require(
			0 == reader.Get_RemainingSize(),
			"Consume Entire World Entity Spawn");

		S2C_WORLD_ENTITY_SPAWNED invalid = source;
		invalid.strArchetypeId = "../BOSS_VALTAN";
		CPacketWriter invalidWriter;
		testRunner.Require(
			!Write_Message(invalidWriter, invalid),
			"Reject Unstable World Archetype ID");
		invalid = source;
		invalid.strPlacementId = "../boss_valtan_1";
		CPacketWriter invalidPlacementWriter;
		testRunner.Require(
			!Write_Message(invalidPlacementWriter, invalid),
			"Reject Unstable World Placement ID");
		invalid = source;
		invalid.strPlacementId.clear();
		CPacketWriter emptyPlacementWriter;
		testRunner.Require(
			Write_Message(emptyPlacementWriter, invalid),
			"Allow Dynamic Spawn Without Placement ID");
		invalid = source;
		invalid.strActionId = "../esther.strike";
		CPacketWriter invalidActionWriter;
		testRunner.Require(
			!Write_Message(invalidActionWriter, invalid),
			"Reject Unstable World Spawn Action ID");
		invalid = source;
		invalid.strActionId.clear();
		CPacketWriter emptyActionWriter;
		testRunner.Require(
			Write_Message(emptyActionWriter, invalid),
			"Allow Idle Spawn Without Action ID");
		invalid = source;
		invalid.fCollisionRadius = 0.f;
		CPacketWriter zeroRadiusWriter;
		testRunner.Require(
			!Write_Message(zeroRadiusWriter, invalid),
			"Reject Combat Entity Without Collision Radius");
		invalid = source;
		invalid.eKind = WORLD_ENTITY_KIND::NPC;
		CPacketWriter npcRadiusWriter;
		testRunner.Require(
			!Write_Message(npcRadiusWriter, invalid),
			"Reject NPC With Combat Collision Radius");

		payload.pop_back();
		CPacketReader truncatedReader{ payload };
		S2C_WORLD_ENTITY_SPAWNED unchanged{};
		unchanged.iNetEntityId = 77u;
		testRunner.Require(
			!Read_Message(truncatedReader, unchanged) &&
			unchanged.iNetEntityId == 77u,
			"Reject Truncated World Entity Collision Radius Without Mutation");
	}

	void Test_WorldEntityDespawnedRoundTrip(TEST_RUNNER& testRunner)
	{
		S2C_WORLD_ENTITY_DESPAWNED source{};
		source.iNetEntityId = 901u;
		std::vector<std::uint8_t> payload;
		testRunner.Require(
			Build_WorldEntityDespawnedPayload(source, payload),
			"Writer World Entity Despawned");
		testRunner.Require(
			payload == std::vector<std::uint8_t>{ 0x85, 0x03, 0x00, 0x00 },
			"World Entity Despawned Payload Layout");

		CPacketReader reader{ payload };
		S2C_WORLD_ENTITY_DESPAWNED decoded{};
		testRunner.Require(
			Read_Message(reader, decoded) &&
			decoded.iNetEntityId == source.iNetEntityId &&
			0u == reader.Get_RemainingSize(),
			"World Entity Despawned Round Trip");

		S2C_WORLD_ENTITY_DESPAWNED invalid{};
		CPacketWriter invalidWriter;
		testRunner.Require(
			!Write_Message(invalidWriter, invalid),
			"Reject World Entity Despawned Zero Entity ID");
	}

	void Test_CombatObjectLifecycleRoundTrip(TEST_RUNNER& testRunner)
	{
		S2C_COMBAT_OBJECT_SPAWNED source{};
		source.iCombatObjectId = 0x100000002ull;
		source.iSourceNetEntityId = 900u;
		source.iSpawnTick = 60u;
		source.iServerTick = 90u;
		source.strCombatObjectArchetypeId =
			"combatobject.valtan.high-jump.target-axe";
		source.strClientVisualId =
			"combatobject.visual.valtan.high-jump.target-axe.v1";
		source.fPositionX = 151.25f;
		source.fPositionY = 22.97f;
		source.fPositionZ = -121.75f;
		source.fYawDegrees = 225.f;
		source.PinnedDefinitionRevision = Make_GameplayDataRevision(3u);

		std::vector<std::uint8_t> payload;
		testRunner.Require(
			Build_CombatObjectSpawnedPayload(source, payload),
			"Writer Combat Object Spawned");
		CPacketReader reader{ payload };
		S2C_COMBAT_OBJECT_SPAWNED decoded{};
		testRunner.Require(
			Read_Message(reader, decoded) &&
			0u == reader.Get_RemainingSize() &&
			decoded.iCombatObjectId == source.iCombatObjectId &&
			decoded.iSourceNetEntityId == source.iSourceNetEntityId &&
			decoded.iSpawnTick == source.iSpawnTick &&
			decoded.iServerTick == source.iServerTick &&
			decoded.strCombatObjectArchetypeId ==
				source.strCombatObjectArchetypeId &&
			decoded.strClientVisualId == source.strClientVisualId &&
			decoded.fPositionX == source.fPositionX &&
			decoded.fPositionY == source.fPositionY &&
			decoded.fPositionZ == source.fPositionZ &&
			decoded.fYawDegrees == source.fYawDegrees &&
			decoded.PinnedDefinitionRevision ==
				source.PinnedDefinitionRevision,
			"Combat Object Spawned Round Trip");

		S2C_COMBAT_OBJECT_SPAWNED invalid = source;
		invalid.iCombatObjectId = INVALID_COMBAT_OBJECT_ID;
		CPacketWriter invalidIdWriter;
		testRunner.Require(!Write_Message(invalidIdWriter, invalid),
			"Reject Combat Object Spawn Without ID");
		invalid = source;
		invalid.iSourceNetEntityId = INVALID_NET_ENTITY_ID;
		CPacketWriter invalidSourceWriter;
		testRunner.Require(!Write_Message(invalidSourceWriter, invalid),
			"Reject Combat Object Spawn Without Source");
		invalid = source;
		invalid.iServerTick = 59u;
		CPacketWriter invalidTickWriter;
		testRunner.Require(!Write_Message(invalidTickWriter, invalid),
			"Reject Combat Object Spawn Before Spawn Tick");
		invalid = source;
		invalid.strClientVisualId = "../invalid";
		CPacketWriter invalidVisualWriter;
		testRunner.Require(!Write_Message(invalidVisualWriter, invalid),
			"Reject Unstable Combat Object Visual ID");
		invalid = source;
		invalid.fPositionY = std::numeric_limits<float>::infinity();
		CPacketWriter invalidPoseWriter;
		testRunner.Require(!Write_Message(invalidPoseWriter, invalid),
			"Reject Non-Finite Combat Object Spawn Pose");
		invalid = source;
		invalid.PinnedDefinitionRevision = {};
		CPacketWriter invalidRevisionWriter;
		testRunner.Require(!Write_Message(invalidRevisionWriter, invalid),
			"Reject Combat Object Spawn With Zero Definition Revision");

		payload.pop_back();
		CPacketReader truncatedReader{ payload };
		S2C_COMBAT_OBJECT_SPAWNED unchanged{};
		unchanged.iCombatObjectId = 77u;
		testRunner.Require(
			!Read_Message(truncatedReader, unchanged) &&
			unchanged.iCombatObjectId == 77u,
			"Reject Truncated Combat Object Spawn Atomically");

		S2C_COMBAT_OBJECT_DESPAWNED despawn{};
		despawn.iCombatObjectId = source.iCombatObjectId;
		std::vector<std::uint8_t> despawnPayload;
		testRunner.Require(
			Build_CombatObjectDespawnedPayload(despawn, despawnPayload),
			"Writer Combat Object Despawned");
		CPacketReader despawnReader{ despawnPayload };
		S2C_COMBAT_OBJECT_DESPAWNED decodedDespawn{};
		testRunner.Require(
			Read_Message(despawnReader, decodedDespawn) &&
			0u == despawnReader.Get_RemainingSize() &&
			decodedDespawn.iCombatObjectId == despawn.iCombatObjectId,
			"Combat Object Despawned Round Trip");
		S2C_COMBAT_OBJECT_DESPAWNED invalidDespawn{};
		CPacketWriter invalidDespawnWriter;
		testRunner.Require(!Write_Message(invalidDespawnWriter, invalidDespawn),
			"Reject Combat Object Despawn Without ID");
	}
	//유효하지 않은 입력에 대한 테스트 
	void Test_InvalidEnterAcceptedPayloads(
		TEST_RUNNER& testRunner)
	{
		S2C_ENTER_ACCEPTED invalidPlayer{};

		invalidPlayer.iPlayerId =
			INVALID_PLAYER_ID;

		invalidPlayer.iNetEntityId = 100;
		invalidPlayer.ActiveGameplayRevision = Make_GameplayDataRevision(1u);

		CPacketWriter invalidPlayerWriter;

		testRunner.Require(
			!Write_Message(
				invalidPlayerWriter,
				invalidPlayer),
			"Reject Zero Player ID From Writer");

		S2C_ENTER_ACCEPTED invalidEntity{};

		invalidEntity.iPlayerId = 1;

		invalidEntity.iNetEntityId =
			INVALID_NET_ENTITY_ID;
		invalidEntity.ActiveGameplayRevision = Make_GameplayDataRevision(1u);

		CPacketWriter invalidEntityWriter;

		testRunner.Require(
			!Write_Message(
				invalidEntityWriter,
				invalidEntity),
			"Reject Zero Net Entity ID From Writer");

		CPacketWriter zeroPlayerPayloadWriter;
		zeroPlayerPayloadWriter.Write_U16(NETWORK_PROTOCOL_VERSION);
		zeroPlayerPayloadWriter.Write_U16(
			static_cast<std::uint16_t>(WORLD_ID::BERN));

		zeroPlayerPayloadWriter.Write_U32(
			INVALID_PLAYER_ID);

		zeroPlayerPayloadWriter.Write_U32(100);

		CPacketReader zeroPlayerReader
		{
			zeroPlayerPayloadWriter.Get_Buffer()
		};

		S2C_ENTER_ACCEPTED decoded{};

		testRunner.Require(
			!Read_Message(
				zeroPlayerReader,
				decoded),
			"Reject Zero Player ID From Reader");

		CPacketWriter zeroEntityPayloadWriter;
		zeroEntityPayloadWriter.Write_U16(NETWORK_PROTOCOL_VERSION);
		zeroEntityPayloadWriter.Write_U16(
			static_cast<std::uint16_t>(WORLD_ID::BERN));

		zeroEntityPayloadWriter.Write_U32(1);

		zeroEntityPayloadWriter.Write_U32(
			INVALID_NET_ENTITY_ID);

		CPacketReader zeroEntityReader
		{
			zeroEntityPayloadWriter.Get_Buffer()
		};

		testRunner.Require(
			!Read_Message(
				zeroEntityReader,
				decoded),
			"Reject Zero Net Entity ID From Reader");

		const std::vector<std::uint8_t>
			truncatedAccepted
		{
			0x06, 0x00,
			0x01, 0x00,
			0x01, 0x00, 0x00, 0x00,
			0x64, 0x00, 0x00
		};

		CPacketReader truncatedReader
		{
			truncatedAccepted
		};

		S2C_ENTER_ACCEPTED unchanged{};

		unchanged.iPlayerId = 77;
		unchanged.iNetEntityId = 88;

		testRunner.Require(
			!Read_Message(
				truncatedReader,
				unchanged),
			"Reject Truncated Enter Accepted");

		testRunner.Require(
			77 == unchanged.iPlayerId &&
			88 == unchanged.iNetEntityId,
			"Failed Accepted Does Not Mutate Message");
	}

	void Test_InvalidPayloads(
		TEST_RUNNER& testRunner)
	{
		C2S_ENTER_WORLD tooLong{};

		tooLong.eCharacterClass =
			CHARACTER_CLASS_ID::LANCE_MASTER;
		// max nick name bytes + 1에 'A'가 의미하는 거는 뭐임?
		tooLong.strNickName =
			std::string(
				MAX_NICKNAME_BYTES + 1,
				'A');

		CPacketWriter longWriter;

		testRunner.Require(
			!Write_Message(
				longWriter,
				tooLong),
			"Reject Long Nickname");

		C2S_ENTER_WORLD invalidProtocol = tooLong;
		invalidProtocol.strNickName = "valid";
		invalidProtocol.iProtocolVersion =
			NETWORK_PROTOCOL_VERSION + 1;
		CPacketWriter protocolWriter;
		testRunner.Require(
			!Write_Message(protocolWriter, invalidProtocol),
			"Reject Unsupported Protocol Version");

		C2S_ENTER_WORLD invalidWorld = invalidProtocol;
		invalidWorld.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
		invalidWorld.eWorldId = WORLD_ID::END;
		CPacketWriter worldWriter;
		testRunner.Require(
			!Write_Message(worldWriter, invalidWorld),
			"Reject Unknown World ID");

		const std::vector<std::uint8_t> invalidClass
		{
			0x06, 0x00,
			0x01, 0x00,
			0xFF, 0x00, 0x00
		};

		CPacketReader invalidClassReader
		{
			invalidClass
		};

		C2S_ENTER_WORLD decoded{};

		testRunner.Require(
			!Read_Message(
				invalidClassReader,
				decoded),
			"Reject Invalid Character Class");
		//이 truncated string이 의미하는 게 뭐지?
		const std::vector<std::uint8_t> truncatedString
		{
			0x06, 0x00,
			0x01, 0x00,
			0x00, 0x06, 0x00, 0xEA
		};

		CPacketReader truncatedReader
		{
			truncatedString
		};


		testRunner.Require(
			!Read_Message(
				truncatedReader,
				decoded),
			"Reject Truncated Nickname");

		const std::vector<std::uint8_t>
			threeBytes
		{
			0x01,
			0x02,
			0x03
		};
		CPacketReader u32Reader{ threeBytes };
		std::uint32_t value = {};

		testRunner.Require(
			!u32Reader.Read_U32(value),
			"Reject Truncated U32");

		testRunner.Require(
			3 == u32Reader.Get_RemainingSize(),
			"Failed U32 Does Not Consume");
	}
	void Test_StreamFraming(TEST_RUNNER& testRunner)
	{
		C2S_ENTER_WORLD source{};

		source.eCharacterClass =
			CHARACTER_CLASS_ID::LANCE_MASTER;

		source.strNickName = "건보";

		std::vector<std::uint8_t> payload;
		//한 프레임에 필요한 payload를 구현한다는 것 맞나?
		testRunner.Require(
			Build_EnterWorldPayload(
				source,
				payload),
			"Build Payload For Frame");

		std::vector<std::uint8_t> frameBytes;

		testRunner.Require(
			Build_Packet_Frame(
				PACKET_TYPE::C2S_ENTER_WORLD,
				payload,
				frameBytes),
			"Build Packet Frame");
		Print_Bytes(
			"Frame bytes: ",
			frameBytes);

		CPacketStreamParser splitParser;

		bool needMoreDataWasCorrect = true;

		for (std::size_t i = 0;
			i + 1 < frameBytes.size();
			++i)
		{
			const std::span<const std::uint8_t>
				oneByte
			{
				frameBytes.data() + i,
				1
			};

			needMoreDataWasCorrect &=
				splitParser.Append(oneByte);

			PACKET_FRAME frame{};

			needMoreDataWasCorrect &=
				PACKET_PARSE_RESULT::
				NEED_MORE_DATA ==
				splitParser.Try_Pop(frame);
		}

		testRunner.Require(
			needMoreDataWasCorrect,
			"Wait For Split Frame");

		testRunner.Require(
			splitParser.Append(
				std::span<const std::uint8_t>
		{
			frameBytes.data() +
				frameBytes.size() - 1,
				1
		}),
			"Append Last Frame Byte");

		PACKET_FRAME splitFrame{};

		testRunner.Require(
			PACKET_PARSE_RESULT::FRAME_READY ==
			splitParser.Try_Pop(splitFrame),
			"Pop Split Frame");

		testRunner.Require(
			splitFrame.ePacketType ==
			PACKET_TYPE::C2S_ENTER_WORLD,
			"Split Frame Packet Type");

		testRunner.Require(
			splitFrame.Payload == payload,
			"Split Frame Payload");

		std::vector<std::uint8_t> combined;

		combined.insert(
			combined.end(),
			frameBytes.begin(),
			frameBytes.end());

		combined.insert(
			combined.end(),
			frameBytes.begin(),
			frameBytes.end());

		CPacketStreamParser combinedParser;

		testRunner.Require(
			combinedParser.Append(combined),
			"Append Combined Frames");

		PACKET_FRAME first{};
		PACKET_FRAME second{};
		PACKET_FRAME none{};

		testRunner.Require(
			PACKET_PARSE_RESULT::FRAME_READY ==
			combinedParser.Try_Pop(first),
			"Pop First Combined Frame");

		testRunner.Require(
			PACKET_PARSE_RESULT::FRAME_READY ==
			combinedParser.Try_Pop(second),
			"Pop Second Combined Frame");

		testRunner.Require(
			PACKET_PARSE_RESULT::NEED_MORE_DATA ==
			combinedParser.Try_Pop(none),
			"No Third Combined Frame");

		CPacketWriter invalidHeaderWriter;

		invalidHeaderWriter.Write_U32(
			static_cast<std::uint32_t>(
				PACKET_HEADER_BYTES));

		invalidHeaderWriter.Write_U16(
			0xFFFF);

		CPacketStreamParser invalidParser;

		invalidParser.Append(
			invalidHeaderWriter.Get_Buffer());

		PACKET_FRAME invalidFrame{};

		testRunner.Require(
			PACKET_PARSE_RESULT::INVALID_FRAME ==
			invalidParser.Try_Pop(
				invalidFrame),
			"Reject Unknown Packet Type");
	}

	//플레이어 디스폰
	void Test_PlayerDespawnedRoundTrip(
		TEST_RUNNER& testRunner)
	{
		S2C_PLAYER_DESPAWNED source{};
		source.iNetEntityId = 100;
		source.eReason =
			PLAYER_DESPAWN_REASON::DISCONNECTED;

		std::vector<std::uint8_t> payload;

		testRunner.Require(
			Build_PlayerDespawnedPayload(source, payload),
			"Writer Player Despawned");

		const std::vector<std::uint8_t> expected
		{
			0x64, 0x00, 0x00, 0x00,
			0x00
		};

		testRunner.Require(
			payload == expected,
			"Player Despawned Payload Layout");

		CPacketReader reader{ payload };
		S2C_PLAYER_DESPAWNED decoded{};

		testRunner.Require(
			Read_Message(reader, decoded),
			"Read Player Despawned");

		testRunner.Require(
			decoded.iNetEntityId == source.iNetEntityId &&
			decoded.eReason == source.eReason,
			"Player Despawned Round Trip");

		testRunner.Require(
			0 == reader.Get_RemainingSize(),
			"Consume Entire Despawned Payload");

		S2C_PLAYER_DESPAWNED invalidId = source;
		invalidId.iNetEntityId = INVALID_NET_ENTITY_ID;
		CPacketWriter invalidIdWriter;

		testRunner.Require(
			!Write_Message(invalidIdWriter, invalidId),
			"Reject Despawned Zero Entity ID");

		S2C_PLAYER_DESPAWNED invalidReason = source;
		invalidReason.eReason =
			PLAYER_DESPAWN_REASON::END;
		CPacketWriter invalidReasonWriter;

		testRunner.Require(
			!Write_Message(invalidReasonWriter, invalidReason),
			"Reject Invalid Despawn Reason");

		std::vector<std::uint8_t> truncated = payload;
		truncated.pop_back();

		CPacketReader truncatedReader{ truncated };
		S2C_PLAYER_DESPAWNED unchanged{};
		unchanged.iNetEntityId = 777;
		unchanged.eReason =
			PLAYER_DESPAWN_REASON::KICKED;

		testRunner.Require(
			!Read_Message(truncatedReader, unchanged),
			"Reject Truncated Player Despawned");

		testRunner.Require(
			unchanged.iNetEntityId == 777 &&
			unchanged.eReason ==
			PLAYER_DESPAWN_REASON::KICKED,
			"Failed Despawn Does Not Mutate");
	}

	void Test_MoveRoundTrip(TEST_RUNNER& testRunner)
	{
		C2S_MOVE source{};
		source.iClientSequence = 7;
		source.fGoalX = 10.f;
		source.fGoalZ = -5.f;

		std::vector<std::uint8_t> payload;

		testRunner.Require(
			Build_MovePayload(source, payload),
			"Writer Move Goal");

		const std::vector<std::uint8_t> expected
		{
			0x07, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x20, 0x41,
			0x00, 0x00, 0xA0, 0xC0
		};

		testRunner.Require(
			payload == expected,
			"Move Goal Payload Layout");

		CPacketReader reader{ payload };
		C2S_MOVE decoded{};

		testRunner.Require(
			Read_Message(reader, decoded),
			"Read Move Goal");

		testRunner.Require(
			decoded.iClientSequence == 7 &&
			decoded.fGoalX == 10.f &&
			decoded.fGoalZ == -5.f,
			"Move Goal Round Trip");

		testRunner.Require(
			0 == reader.Get_RemainingSize(),
			"Consume Entire Move Goal");

		C2S_MOVE invalidSequence = source;
		invalidSequence.iClientSequence = 0;
		CPacketWriter sequenceWriter;

		testRunner.Require(
			!Write_Message(sequenceWriter, invalidSequence),
			"Reject Zero Move Sequence");

		C2S_MOVE invalidGoal = source;
		invalidGoal.fGoalX =
			std::numeric_limits<float>::infinity();
		CPacketWriter goalWriter;

		testRunner.Require(
			!Write_Message(goalWriter, invalidGoal),
			"Reject Infinite Move Goal");

		payload.pop_back();
		CPacketReader truncatedReader{ payload };
		C2S_MOVE unchanged{};
		unchanged.iClientSequence = 99;
		unchanged.fGoalX = 77.f;
		unchanged.fGoalZ = 88.f;

		testRunner.Require(
			!Read_Message(truncatedReader, unchanged),
			"Reject Truncated Move Goal");

		testRunner.Require(
			unchanged.iClientSequence == 99 &&
			unchanged.fGoalX == 77.f &&
			unchanged.fGoalZ == 88.f,
			"Failed Move Does Not Mutate");
	}

	void Test_UseSkillRoundTrip(TEST_RUNNER& testRunner)
	{
		C2S_USE_SKILL source{};
		source.iClientSequence = 9;
		source.iSkillId = 2050500;
		source.eTargetIntent = SKILL_TARGET_INTENT_KIND::GROUND_POINT;
		source.fAimX = 151.f;
		source.fAimZ = -122.f;

		std::vector<std::uint8_t> payload;
		testRunner.Require(
			Build_UseSkillPayload(source, payload),
			"Writer Use Skill");
		testRunner.Require(
			17 == payload.size(),
			"Use Skill Payload Size");

		CPacketReader reader{ payload };
		C2S_USE_SKILL decoded{};
		testRunner.Require(
			Read_Message(reader, decoded),
			"Read Use Skill");
		testRunner.Require(
			decoded.iClientSequence == source.iClientSequence &&
			decoded.iSkillId == source.iSkillId &&
			decoded.eTargetIntent == source.eTargetIntent &&
			decoded.fAimX == source.fAimX &&
			decoded.fAimZ == source.fAimZ &&
			0 == reader.Get_RemainingSize(),
			"Use Skill Round Trip");

		C2S_USE_SKILL invalid = source;
		invalid.iSkillId = INVALID_SKILL_ID;
		CPacketWriter invalidWriter;
		testRunner.Require(
			!Write_Message(invalidWriter, invalid),
			"Reject Invalid Skill ID");

		invalid = source;
		invalid.eTargetIntent = SKILL_TARGET_INTENT_KIND::END;
		CPacketWriter invalidIntentWriter;
		testRunner.Require(
			!Write_Message(invalidIntentWriter, invalid),
			"Reject Unknown Skill Target Intent");

		payload.pop_back();
		CPacketReader truncatedReader{ payload };
		C2S_USE_SKILL unchanged{};
		unchanged.iClientSequence = 77;
		testRunner.Require(
			!Read_Message(truncatedReader, unchanged) &&
			77 == unchanged.iClientSequence,
			"Reject Truncated Skill Without Mutation");
	}

	void Test_ReleaseSkillRoundTrip(TEST_RUNNER& testRunner)
	{
		C2S_RELEASE_SKILL source{};
		source.iClientSequence = 12;
		source.iSkillId = 34590;

		CPacketWriter writer;
		testRunner.Require(
			Write_Message(writer, source),
			"Writer Release Skill");
		std::vector<std::uint8_t> payload = writer.Get_Buffer();
		testRunner.Require(
			8 == payload.size(),
			"Release Skill Payload Size");

		CPacketReader reader{ payload };
		C2S_RELEASE_SKILL decoded{};
		testRunner.Require(
			Read_Message(reader, decoded) &&
			decoded.iClientSequence == source.iClientSequence &&
			decoded.iSkillId == source.iSkillId &&
			0 == reader.Get_RemainingSize(),
			"Release Skill Round Trip");

		C2S_RELEASE_SKILL invalidSkill = source;
		invalidSkill.iSkillId = INVALID_SKILL_ID;
		CPacketWriter invalidSkillWriter;
		testRunner.Require(
			!Write_Message(invalidSkillWriter, invalidSkill),
			"Reject Release Without Skill ID");

		C2S_RELEASE_SKILL invalidSequence = source;
		invalidSequence.iClientSequence = 0;
		CPacketWriter invalidSequenceWriter;
		testRunner.Require(
			!Write_Message(invalidSequenceWriter, invalidSequence),
			"Reject Release Without Sequence");

		payload.pop_back();
		CPacketReader truncatedReader{ payload };
		C2S_RELEASE_SKILL unchanged{};
		unchanged.iClientSequence = 55;
		testRunner.Require(
			!Read_Message(truncatedReader, unchanged) &&
			55 == unchanged.iClientSequence,
			"Failed Release Does Not Mutate");
	}

	void Test_UpdateSkillAimRoundTrip(TEST_RUNNER& testRunner)
	{
		C2S_UPDATE_SKILL_AIM source{};
		source.iClientSequence = 14;
		source.iSkillId = 34590;
		source.fAimX = 148.5f;
		source.fAimZ = -119.25f;

		CPacketWriter writer;
		testRunner.Require(
			Write_Message(writer, source),
			"Writer Update Skill Aim");
		std::vector<std::uint8_t> payload = writer.Get_Buffer();
		testRunner.Require(
			16 == payload.size(),
			"Update Skill Aim Payload Size");

		CPacketReader reader{ payload };
		C2S_UPDATE_SKILL_AIM decoded{};
		testRunner.Require(
			Read_Message(reader, decoded) &&
			decoded.iClientSequence == source.iClientSequence &&
			decoded.iSkillId == source.iSkillId &&
			decoded.fAimX == source.fAimX &&
			decoded.fAimZ == source.fAimZ &&
			0 == reader.Get_RemainingSize(),
			"Update Skill Aim Round Trip");

		C2S_UPDATE_SKILL_AIM invalidSkill = source;
		invalidSkill.iSkillId = INVALID_SKILL_ID;
		CPacketWriter invalidSkillWriter;
		testRunner.Require(
			!Write_Message(invalidSkillWriter, invalidSkill),
			"Reject Aim Update Without Skill ID");

		C2S_UPDATE_SKILL_AIM invalidSequence = source;
		invalidSequence.iClientSequence = 0;
		CPacketWriter invalidSequenceWriter;
		testRunner.Require(
			!Write_Message(invalidSequenceWriter, invalidSequence),
			"Reject Aim Update Without Sequence");

		C2S_UPDATE_SKILL_AIM invalidAim = source;
		invalidAim.fAimX = std::numeric_limits<float>::quiet_NaN();
		CPacketWriter invalidAimWriter;
		testRunner.Require(
			!Write_Message(invalidAimWriter, invalidAim),
			"Reject Aim Update With Non-Finite Aim");

		payload.pop_back();
		CPacketReader truncatedReader{ payload };
		C2S_UPDATE_SKILL_AIM unchanged{};
		unchanged.iClientSequence = 66;
		testRunner.Require(
			!Read_Message(truncatedReader, unchanged) &&
			66 == unchanged.iClientSequence,
			"Failed Aim Update Does Not Mutate");
	}

	void Test_UseEstherSkillRoundTrip(TEST_RUNNER& testRunner)
	{
		C2S_USE_ESTHER_SKILL source{};
		source.iClientSequence = 21;
		source.iSlotIndex = 1;
		source.fAimX = 42.75f;
		source.fAimZ = -87.5f;

		CPacketWriter writer;
		testRunner.Require(
			Write_Message(writer, source),
			"Writer Use Esther Skill");
		std::vector<std::uint8_t> payload = writer.Get_Buffer();
		testRunner.Require(
			13 == payload.size(),
			"Use Esther Skill Payload Size");

		CPacketReader reader{ payload };
		C2S_USE_ESTHER_SKILL decoded{};
		testRunner.Require(
			Read_Message(reader, decoded) &&
			decoded.iClientSequence == source.iClientSequence &&
			decoded.iSlotIndex == source.iSlotIndex &&
			decoded.fAimX == source.fAimX &&
			decoded.fAimZ == source.fAimZ &&
			0 == reader.Get_RemainingSize(),
			"Use Esther Skill Round Trip");

		C2S_USE_ESTHER_SKILL invalidSlotLow = source;
		invalidSlotLow.iSlotIndex = 0;
		CPacketWriter invalidSlotLowWriter;
		testRunner.Require(
			!Write_Message(invalidSlotLowWriter, invalidSlotLow),
			"Reject Esther Slot Zero");

		C2S_USE_ESTHER_SKILL invalidSlotHigh = source;
		invalidSlotHigh.iSlotIndex = 4;
		CPacketWriter invalidSlotHighWriter;
		testRunner.Require(
			!Write_Message(invalidSlotHighWriter, invalidSlotHigh),
			"Reject Esther Slot Above Roster");

		C2S_USE_ESTHER_SKILL invalidSequence = source;
		invalidSequence.iClientSequence = 0;
		CPacketWriter invalidSequenceWriter;
		testRunner.Require(
			!Write_Message(invalidSequenceWriter, invalidSequence),
			"Reject Esther Skill Without Sequence");

		C2S_USE_ESTHER_SKILL invalidAim = source;
		invalidAim.fAimZ = std::numeric_limits<float>::infinity();
		CPacketWriter invalidAimWriter;
		testRunner.Require(
			!Write_Message(invalidAimWriter, invalidAim),
			"Reject Esther Skill With Non-Finite Aim");

		payload.pop_back();
		CPacketReader truncatedReader{ payload };
		C2S_USE_ESTHER_SKILL unchanged{};
		unchanged.iClientSequence = 77;
		testRunner.Require(
			!Read_Message(truncatedReader, unchanged) &&
			77 == unchanged.iClientSequence,
			"Failed Esther Skill Read Does Not Mutate");
	}

	void Test_RevivePlayerRoundTrip(TEST_RUNNER& testRunner)
	{
		C2S_REVIVE_PLAYER source{};
		source.iClientSequence = 17u;
		CPacketWriter writer;
		testRunner.Require(
			Write_Message(writer, source) && 4u == writer.Get_Buffer().size(),
			"Writer Revive Player");
		CPacketReader reader{ writer.Get_Buffer() };
		C2S_REVIVE_PLAYER decoded{};
		testRunner.Require(
			Read_Message(reader, decoded) &&
			decoded.iClientSequence == source.iClientSequence &&
			0u == reader.Get_RemainingSize(),
			"Revive Player Round Trip");
		C2S_REVIVE_PLAYER invalid{};
		CPacketWriter invalidWriter;
		testRunner.Require(!Write_Message(invalidWriter, invalid),
			"Reject Zero Revive Sequence");
		std::vector<std::uint8_t> truncated = writer.Get_Buffer();
		truncated.pop_back();
		CPacketReader truncatedReader{ truncated };
		C2S_REVIVE_PLAYER unchanged{};
		unchanged.iClientSequence = 99u;
		testRunner.Require(
			!Read_Message(truncatedReader, unchanged) &&
			99u == unchanged.iClientSequence,
			"Reject Truncated Revive Without Mutation");
	}

	void Test_PartyInviteProtocol(TEST_RUNNER& testRunner)
	{
		{
			testRunner.Require(41u == NETWORK_PROTOCOL_VERSION,
				"Party And Expanded Destruction Use A Fresh Protocol 41");
			C2S_ENTER_WORLD oldPeer{};
			oldPeer.iProtocolVersion = 40u;
			oldPeer.eWorldId = WORLD_ID::BERN;
			oldPeer.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
			oldPeer.strNickName = "OldPeer";
			CPacketWriter oldWriter;
			testRunner.Require(!Write_Message(oldWriter, oldPeer),
				"Reject Both Independently Shipped Protocol 40 Peers");
			S2C_PARTY_TRANSFER_RESULT failure{};
			failure.iRequestSequence = 12u;
			failure.eTargetWorldId = WORLD_ID::VALTAN_ARENA;
			failure.eResult = PARTY_TRANSFER_RESULT::REJECTED_ROOM_FULL;
			CPacketWriter writer;
			const bool encoded = Write_Message(writer, failure);
			CPacketReader reader{ writer.Get_Buffer() };
			S2C_PARTY_TRANSFER_RESULT decoded{};
			testRunner.Require(encoded && Read_Message(reader, decoded) &&
				12u == decoded.iRequestSequence &&
				PARTY_TRANSFER_RESULT::REJECTED_ROOM_FULL == decoded.eResult &&
				0u == reader.Get_RemainingSize(), "Party Transfer Failure Round Trip");
			for (std::size_t size = 0; size < writer.Get_Buffer().size(); ++size)
			{
				CPacketReader truncated{ std::span<const std::uint8_t>{ writer.Get_Buffer().data(), size } };
				S2C_PARTY_TRANSFER_RESULT unchanged = failure;
				testRunner.Require(!Read_Message(truncated, unchanged) &&
					unchanged.iRequestSequence == failure.iRequestSequence &&
					unchanged.eResult == failure.eResult,
					"Reject Truncated Party Failure Without Mutation");
			}
			CPacketWriter unknown;
			unknown.Write_U32(12u);
			unknown.Write_U16(static_cast<std::uint16_t>(WORLD_ID::VALTAN_ARENA));
			unknown.Write_U8(255u);
			CPacketReader unknownReader{ unknown.Get_Buffer() };
			testRunner.Require(!Read_Message(unknownReader, decoded),
				"Reject Unknown Party Transfer Failure Result");
		}
		{
			C2S_PARTY_INVITE source{};
			source.iRequestSequence = 7u;
			source.iTargetNetEntityId = 42u;
			CPacketWriter writer;
			testRunner.Require(
				Write_Message(writer, source), "Writer Party Invite");
			CPacketReader reader{ writer.Get_Buffer() };
			C2S_PARTY_INVITE decoded{};
			testRunner.Require(
				Read_Message(reader, decoded) &&
				decoded.iRequestSequence == source.iRequestSequence &&
				decoded.iTargetNetEntityId == source.iTargetNetEntityId &&
				0u == reader.Get_RemainingSize(),
				"Party Invite Round Trip");

			C2S_PARTY_INVITE invalid{};
			CPacketWriter invalidWriter;
			testRunner.Require(!Write_Message(invalidWriter, invalid),
				"Reject Zero Party Invite Sequence/Target");
		}
		{
			S2C_PARTY_INVITE_RECEIVED source{};
			source.iFromNetEntityId = 11u;
			source.strFromNickname = "Inviter";
			CPacketWriter writer;
			testRunner.Require(
				Write_Message(writer, source), "Writer Party Invite Received");
			CPacketReader reader{ writer.Get_Buffer() };
			S2C_PARTY_INVITE_RECEIVED decoded{};
			testRunner.Require(
				Read_Message(reader, decoded) &&
				decoded.iFromNetEntityId == source.iFromNetEntityId &&
				decoded.strFromNickname == source.strFromNickname &&
				0u == reader.Get_RemainingSize(),
				"Party Invite Received Round Trip");

			S2C_PARTY_INVITE_RECEIVED invalid{};
			invalid.iFromNetEntityId = 11u;
			invalid.strFromNickname = "";
			CPacketWriter invalidWriter;
			testRunner.Require(!Write_Message(invalidWriter, invalid),
				"Reject Empty Party Invite Nickname");
		}
		{
			C2S_PARTY_INVITE_RESPOND accept{};
			accept.iRequestSequence = 9u;
			accept.iFromNetEntityId = 11u;
			accept.bAccepted = true;
			CPacketWriter writer;
			testRunner.Require(
				Write_Message(writer, accept), "Writer Party Invite Respond");
			CPacketReader reader{ writer.Get_Buffer() };
			C2S_PARTY_INVITE_RESPOND decoded{};
			testRunner.Require(
				Read_Message(reader, decoded) &&
				decoded.iRequestSequence == accept.iRequestSequence &&
				decoded.iFromNetEntityId == accept.iFromNetEntityId &&
				true == decoded.bAccepted &&
				0u == reader.Get_RemainingSize(),
				"Party Invite Respond Round Trip");
		}
		{
			S2C_PARTY_ROSTER source{};
			source.Members.push_back(PARTY_ROSTER_MEMBER{
				42u, "Leader", CHARACTER_CLASS_ID::LANCE_MASTER });
			source.Members.push_back(PARTY_ROSTER_MEMBER{
				11u, "Member", CHARACTER_CLASS_ID::ARTIST });
			CPacketWriter writer;
			testRunner.Require(
				Write_Message(writer, source), "Writer Party Roster");
			CPacketReader reader{ writer.Get_Buffer() };
			S2C_PARTY_ROSTER decoded{};
			testRunner.Require(
				Read_Message(reader, decoded) &&
				2u == decoded.Members.size() &&
				decoded.Members[0].iNetEntityId == 42u &&
				decoded.Members[0].strNickname == "Leader" &&
				decoded.Members[0].eCharacterClass ==
					CHARACTER_CLASS_ID::LANCE_MASTER &&
				decoded.Members[1].iNetEntityId == 11u &&
				0u == reader.Get_RemainingSize(),
				"Party Roster Round Trip");

			S2C_PARTY_ROSTER tooMany{};
			for (std::size_t i = 0; i < MAX_PARTY_MEMBERS + 1u; ++i)
			{
				tooMany.Members.push_back(PARTY_ROSTER_MEMBER{
					static_cast<NET_ENTITY_ID>(i + 1u), "N",
					CHARACTER_CLASS_ID::ARTIST });
			}
			CPacketWriter tooManyWriter;
			testRunner.Require(!Write_Message(tooManyWriter, tooMany),
				"Reject Oversized Party Roster");
		}
	}

	void Test_ChatProtocol(TEST_RUNNER& testRunner)
	{
		{
			C2S_CHAT source{};
			source.strText = "hello room";
			CPacketWriter writer;
			testRunner.Require(
				Write_Message(writer, source), "Writer Chat Send");
			CPacketReader reader{ writer.Get_Buffer() };
			C2S_CHAT decoded{};
			testRunner.Require(
				Read_Message(reader, decoded) &&
				decoded.strText == source.strText &&
				0u == reader.Get_RemainingSize(),
				"Chat Send Round Trip");

			C2S_CHAT empty{};
			CPacketWriter emptyWriter;
			testRunner.Require(!Write_Message(emptyWriter, empty),
				"Reject Empty Chat Send");

			C2S_CHAT tooLong{};
			tooLong.strText = std::string(MAX_CHAT_TEXT_BYTES + 1u, 'a');
			CPacketWriter tooLongWriter;
			testRunner.Require(!Write_Message(tooLongWriter, tooLong),
				"Reject Oversized Chat Send");
		}
		{
			S2C_CHAT source{};
			source.iFromNetEntityId = 42u;
			source.strFromNickname = "Speaker";
			source.strText = "hello room";
			CPacketWriter writer;
			testRunner.Require(
				Write_Message(writer, source), "Writer Chat Received");
			CPacketReader reader{ writer.Get_Buffer() };
			S2C_CHAT decoded{};
			testRunner.Require(
				Read_Message(reader, decoded) &&
				decoded.iFromNetEntityId == source.iFromNetEntityId &&
				decoded.strFromNickname == source.strFromNickname &&
				decoded.strText == source.strText &&
				0u == reader.Get_RemainingSize(),
				"Chat Received Round Trip");

			S2C_CHAT invalid{};
			invalid.iFromNetEntityId = 42u;
			invalid.strFromNickname = "Speaker";
			invalid.strText = "";
			CPacketWriter invalidWriter;
			testRunner.Require(!Write_Message(invalidWriter, invalid),
				"Reject Empty Chat Received Text");
		}
	}

	void Test_CharacterClassChangeRoundTrip(TEST_RUNNER& testRunner)
	{
		C2S_CHANGE_CHARACTER_CLASS request{};
		request.iClientSequence = 41u;
		request.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
		CPacketWriter requestWriter;
		testRunner.Require(Write_Message(requestWriter, request),
			"Writer Character Class Change");
		CPacketReader requestReader{ requestWriter.Get_Buffer() };
		C2S_CHANGE_CHARACTER_CLASS decodedRequest{};
		testRunner.Require(Read_Message(requestReader, decodedRequest) &&
			0u == requestReader.Get_RemainingSize() &&
			request.iClientSequence == decodedRequest.iClientSequence &&
			request.eCharacterClass == decodedRequest.eCharacterClass,
			"Character Class Change Round Trip");

		S2C_CHARACTER_CLASS_CHANGE_RESULT result{};
		result.iClientSequence = request.iClientSequence;
		result.eResult = CHARACTER_CLASS_CHANGE_RESULT::ACCEPTED;
		result.eRequestedClass = request.eCharacterClass;
		result.eActiveClass = request.eCharacterClass;
		CPacketWriter resultWriter;
		testRunner.Require(Write_Message(resultWriter, result),
			"Writer Character Class Change Result");
		CPacketReader resultReader{ resultWriter.Get_Buffer() };
		S2C_CHARACTER_CLASS_CHANGE_RESULT decodedResult{};
		testRunner.Require(Read_Message(resultReader, decodedResult) &&
			0u == resultReader.Get_RemainingSize() &&
			result.iClientSequence == decodedResult.iClientSequence &&
			result.eResult == decodedResult.eResult &&
			result.eActiveClass == decodedResult.eActiveClass,
			"Character Class Change Result Round Trip");

		result.eActiveClass = CHARACTER_CLASS_ID::WARLORD;
		CPacketWriter inconsistentWriter;
		testRunner.Require(!Write_Message(inconsistentWriter, result),
			"Reject Accepted Class Change With Different Active Class");
		request.iClientSequence = 0u;
		CPacketWriter staleWriter;
		testRunner.Require(!Write_Message(staleWriter, request),
			"Reject Zero Class Change Sequence");
	}

	void Test_WorldSnapshotRoundTrip(
		TEST_RUNNER& testRunner)
	{
		S2C_WORLD_SNAPSHOT source{};
		source.iServerTick = 30;
		source.iEstherGauge = 640;
		source.iEstherGaugeMaximum = 1000;
		source.ActiveGameplayRevision = Make_GameplayDataRevision(1u);
		source.RequiredPinnedGameplayRevisions.push_back(
			Make_GameplayDataRevision(2u));

		PLAYER_SNAPSHOT first{};
		first.iNetEntityId = 100;
		first.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		first.fPositionX = 1.f;
		first.fPositionY = 0.f;
		first.fPositionZ = 2.f;
		first.fYawDegrees = 90.f;
		first.eLocomotionState =
			PLAYER_LOCOMOTION_STATE::MOVING;
		first.eAction = PLAYER_ACTION_STATE::SKILL;
		first.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		first.iSkillId = 34060;
		first.iActionStartTick = 25;
		first.hasSkillTarget = true;
		first.fSkillTargetX = 6.f;
		first.fSkillTargetY = 0.25f;
		first.fSkillTargetZ = -3.f;
		first.iCurrentHp = 875;
		first.iMaximumHp = 1000;
		first.iCurrentResource = 80;
		first.iMaximumResource = 100;
		first.iComboStage = 3;
		first.Cooldowns.push_back({ 34060, 330 });

		PLAYER_SNAPSHOT second{};
		second.iNetEntityId = 101;
		second.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
		second.fPositionX = 3.f;
		second.fPositionY = 0.f;
		second.fPositionZ = 4.f;
		second.fYawDegrees = 180.f;
		second.eLocomotionState =
			PLAYER_LOCOMOTION_STATE::MOVING;
		second.eAction = PLAYER_ACTION_STATE::TRIGGER_MOVE;
		second.iActionStartTick = 29;
		second.isCombatReady = false;

		source.Players.push_back(first);
		source.Players.push_back(second);

		WORLD_ENTITY_SNAPSHOT entity{};
		entity.iNetEntityId = 900;
		entity.eAction = WORLD_ENTITY_ACTION::PATTERN_ACTIVE;
		entity.strPatternId = "VALTAN_SWING";
		entity.strActionId = "valtan.attack.swing.active";
		entity.iPatternSequence = 7u;
		entity.iPatternStageIndex = 1u;
		entity.iPatternTargetNetEntityId = first.iNetEntityId;
		entity.fPositionX = 150.f;
		entity.fPositionY = 22.97f;
		entity.fPositionZ = -122.f;
		entity.fYawDegrees = 225.f;
		entity.iActionStartTick = 20;
		entity.iCurrentHp = 9500;
		entity.iMaximumHp = 10000;
		entity.iPhase = 1;
		/* Plate 0 destroyed, plate 1 still on: the mask has to survive as the
		exact plate identity, not as a count. */
		entity.iBrokenArmorMask = 0x01;
		entity.hasBossCombatState = true;
		entity.BossCombat.iStateRevision = 4u;
		entity.BossCombat.iAlivePartMask = 0x1u;
		entity.BossCombat.iFlags = static_cast<std::uint16_t>(
			BOSS_COMBAT_STATE_FLAG::COUNTERABLE);
		entity.BossCombat.iCurrentStagger = 320u;
		entity.BossCombat.iMaximumStagger = 1000u;
		entity.BossCombat.iCurrentShield = 0u;
		entity.BossCombat.iMaximumShield = 0u;
		entity.BossCombat.iGameplayPhase = 1u;
		entity.PinnedDefinitionRevision = source.ActiveGameplayRevision;
		source.Entities.push_back(entity);

		BOSS_COMBAT_EVENT partBroken{};
		partBroken.iEventSequence = 0x100000007ull;
		partBroken.iEventTick = 30u;
		partBroken.iBossNetEntityId = entity.iNetEntityId;
		partBroken.eKind = BOSS_COMBAT_EVENT_KIND::PART_BROKEN;
		partBroken.iPartMask = 0x2u;
		source.BossCombatEvents.push_back(partBroken);

		COMBAT_OBJECT_SNAPSHOT combatObject{};
		combatObject.iCombatObjectId = 0x100000002ull;
		combatObject.iSourceNetEntityId = entity.iNetEntityId;
		combatObject.fPositionX = 152.f;
		combatObject.fPositionY = 24.f;
		combatObject.fPositionZ = -120.f;
		combatObject.fYawDegrees = 180.f;
		combatObject.PinnedDefinitionRevision =
			source.RequiredPinnedGameplayRevisions.front();
		source.CombatObjects.push_back(combatObject);

		std::vector<std::uint8_t> payload;

		testRunner.Require(
			Build_WorldSnapshotPayload(source, payload),
			"Writer World Snapshot");

		constexpr std::size_t snapshotHeaderBytes =
			4 + 2 + 2 + 2 + 1 + 1 + 1 + 4 + 4;
		/* Protocol 38 adds one owner id, one typed slot, and four local-frame
		attachment floats to every player row. */
		constexpr std::size_t playerAttachmentBytes = 4 + 1 + (4 * 4);
		constexpr std::size_t playerFixedBytes =
			4 + 1 + (4 * 4) + 1 + 1 + 1 + (4 * 8) + 1 + (4 * 3) +
			1 + 1 + 1 + playerAttachmentBytes;
		constexpr std::size_t cooldownBytes = 4 + 4;
		/* Trailing 1 + 1 + 1 is iPhase, iBrokenArmorMask and the
		hasBossCombatState flag. The block after it is the boss combat
		snapshot the flag guards. */
		const std::size_t entityBytes =
			4 + 1 + 2 + entity.strPatternId.size() + 2 +
			entity.strActionId.size() + (4 * 4) + (4 * 6) + 1 + 1 + 1 +
			4 + 4 + 2 + (4 * 4) + 1 + GAMEPLAY_DATA_REVISION_BYTES;
		constexpr std::size_t bossCombatEventBytes =
			8 + 4 + 4 + 1 + 4;
		constexpr std::size_t combatObjectBytes =
			8 + 4 + (4 * 4) + GAMEPLAY_DATA_REVISION_BYTES;
		const std::size_t snapshotRevisionBytes =
			GAMEPLAY_DATA_REVISION_BYTES + 1u +
			(source.RequiredPinnedGameplayRevisions.size() *
				GAMEPLAY_DATA_REVISION_BYTES);
		const std::size_t expectedPayloadBytes =
			snapshotHeaderBytes +
			(playerFixedBytes * 2) +
			cooldownBytes +
			entityBytes +
			bossCombatEventBytes +
			combatObjectBytes +
			snapshotRevisionBytes;

		testRunner.Require(
			expectedPayloadBytes == payload.size(),
			"World Snapshot Payload Size");

		CPacketReader reader{ payload };
		S2C_WORLD_SNAPSHOT decoded{};

		testRunner.Require(
			Read_Message(reader, decoded),
			"Read World Snapshot");

		testRunner.Require(
			decoded.iServerTick == 30 &&
			decoded.eWorldId == WORLD_ID::BERN &&
			decoded.Players.size() == 2 &&
			decoded.Entities.size() == 1 &&
			decoded.BossCombatEvents.size() == 1 &&
			decoded.CombatObjects.size() == 1 &&
			decoded.iEstherGauge == 640 &&
			decoded.iEstherGaugeMaximum == 1000 &&
			decoded.ActiveGameplayRevision ==
				source.ActiveGameplayRevision &&
			decoded.RequiredPinnedGameplayRevisions ==
				source.RequiredPinnedGameplayRevisions &&
			decoded.Entities.front().PinnedDefinitionRevision ==
				entity.PinnedDefinitionRevision &&
			decoded.CombatObjects.front().PinnedDefinitionRevision ==
				combatObject.PinnedDefinitionRevision,
			"World Snapshot Header Round Trip");

		S2C_WORLD_SNAPSHOT overfullGauge = source;
		overfullGauge.iEstherGauge = 1001;
		std::vector<std::uint8_t> overfullPayload;
		testRunner.Require(
			!Build_WorldSnapshotPayload(overfullGauge, overfullPayload),
			"Reject Esther Gauge Above Maximum");

		S2C_WORLD_SNAPSHOT gaugeWithoutRoster = source;
		gaugeWithoutRoster.iEstherGaugeMaximum = 0;
		gaugeWithoutRoster.iEstherGauge = 1;
		std::vector<std::uint8_t> rosterlessPayload;
		testRunner.Require(
			!Build_WorldSnapshotPayload(gaugeWithoutRoster, rosterlessPayload),
			"Reject Esther Gauge Without Roster");
		if (2u != decoded.Players.size() || 1u != decoded.Entities.size() ||
			1u != decoded.BossCombatEvents.size() ||
			1u != decoded.CombatObjects.size())
			return;

		testRunner.Require(
			decoded.Players[0].iNetEntityId == 100 &&
			decoded.Players[0].eCharacterClass == CHARACTER_CLASS_ID::LANCE_MASTER &&
			decoded.Players[0].fPositionX == 1.f &&
			decoded.Players[0].fPositionZ == 2.f &&
			decoded.Players[0].eLocomotionState ==
			PLAYER_LOCOMOTION_STATE::MOVING &&
			decoded.Players[0].eAction == PLAYER_ACTION_STATE::SKILL &&
			decoded.Players[0].eStance ==
			PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR &&
			decoded.Players[0].iSkillId == 34060 &&
			decoded.Players[0].iActionStartTick == 25 &&
			decoded.Players[0].hasSkillTarget &&
			decoded.Players[0].fSkillTargetX == 6.f &&
			decoded.Players[0].fSkillTargetY == 0.25f &&
			decoded.Players[0].fSkillTargetZ == -3.f &&
			decoded.Players[0].iCurrentHp == 875 &&
			decoded.Players[0].Cooldowns.size() == 1 &&
			decoded.Players[0].Cooldowns[0].iCooldownEndTick == 330 &&
			decoded.Players[0].iComboStage == 3 &&
			decoded.Players[1].iComboStage == 0 &&
			decoded.Players[1].iNetEntityId == 101 &&
			decoded.Players[1].fPositionX == 3.f &&
			decoded.Players[1].fPositionZ == 4.f &&
			decoded.Players[1].eLocomotionState ==
			PLAYER_LOCOMOTION_STATE::MOVING &&
			decoded.Players[1].eAction ==
			PLAYER_ACTION_STATE::TRIGGER_MOVE &&
			decoded.Players[1].iActionStartTick == 29 &&
			!decoded.Players[1].hasSkillTarget &&
			!decoded.Players[1].isCombatReady,
			"World Snapshot Players Round Trip");

		testRunner.Require(
			decoded.Entities[0].iNetEntityId == 900 &&
			decoded.Entities[0].eAction == WORLD_ENTITY_ACTION::PATTERN_ACTIVE &&
			decoded.Entities[0].strPatternId == entity.strPatternId &&
			decoded.Entities[0].strActionId == entity.strActionId &&
			decoded.Entities[0].iPatternSequence == 7u &&
			decoded.Entities[0].iPatternStageIndex == 1u &&
			decoded.Entities[0].iPatternTargetNetEntityId ==
				first.iNetEntityId &&
			decoded.Entities[0].fPositionX == 150.f &&
			decoded.Entities[0].iCurrentHp == 9500 &&
			decoded.Entities[0].iMaximumHp == 10000 &&
			decoded.Entities[0].iPhase == 1 &&
			decoded.Entities[0].iBrokenArmorMask == 0x01 &&
			decoded.Entities[0].hasBossCombatState &&
			decoded.Entities[0].BossCombat.iStateRevision == 4u &&
			decoded.Entities[0].BossCombat.iAlivePartMask == 0x1u &&
			decoded.Entities[0].BossCombat.iFlags ==
				static_cast<std::uint16_t>(
					BOSS_COMBAT_STATE_FLAG::COUNTERABLE) &&
			decoded.Entities[0].BossCombat.iCurrentStagger == 320u &&
			decoded.Entities[0].BossCombat.iMaximumStagger == 1000u &&
			decoded.Entities[0].BossCombat.iGameplayPhase == 1u,
			"World Snapshot Entities Round Trip");

		S2C_WORLD_SNAPSHOT overArmoured = source;
		overArmoured.Entities[0].iBrokenArmorMask = static_cast<std::uint8_t>(
			1u << MAX_WORLD_ENTITY_ARMOR_PLATES);
		CPacketWriter overArmouredWriter;
		testRunner.Require(
			!Write_Message(overArmouredWriter, overArmoured),
			"Reject A Broken Armour Mask That Names An Unsupported Plate");

		S2C_WORLD_SNAPSHOT nonBossTarget = source;
		nonBossTarget.Entities[0].hasBossCombatState = false;
		nonBossTarget.Entities[0].BossCombat = {};
		CPacketWriter nonBossTargetWriter;
		testRunner.Require(
			!Write_Message(nonBossTargetWriter, nonBossTarget),
			"Reject A Pattern Target On A Non-Boss World Entity");

		S2C_WORLD_SNAPSHOT zeroEntityRevision = source;
		zeroEntityRevision.Entities[0].PinnedDefinitionRevision = {};
		CPacketWriter zeroEntityRevisionWriter;
		testRunner.Require(
			!Write_Message(zeroEntityRevisionWriter, zeroEntityRevision),
			"Reject World Entity Snapshot With Zero Pinned Revision");

		S2C_WORLD_SNAPSHOT zeroCombatRevision = source;
		zeroCombatRevision.CombatObjects[0].PinnedDefinitionRevision = {};
		CPacketWriter zeroCombatRevisionWriter;
		testRunner.Require(
			!Write_Message(zeroCombatRevisionWriter, zeroCombatRevision),
			"Reject Combat Object Snapshot With Zero Pinned Revision");

		S2C_WORLD_SNAPSHOT zeroActiveRevision = source;
		zeroActiveRevision.ActiveGameplayRevision = {};
		CPacketWriter zeroActiveRevisionWriter;
		testRunner.Require(
			!Write_Message(zeroActiveRevisionWriter, zeroActiveRevision),
			"Reject World Snapshot With Zero Active Revision");

		testRunner.Require(
			decoded.BossCombatEvents[0].iEventSequence ==
				partBroken.iEventSequence &&
			decoded.BossCombatEvents[0].iEventTick == 30u &&
			decoded.BossCombatEvents[0].iBossNetEntityId == 900u &&
			decoded.BossCombatEvents[0].eKind ==
				BOSS_COMBAT_EVENT_KIND::PART_BROKEN &&
			decoded.BossCombatEvents[0].iPartMask == 0x2u,
			"World Snapshot Boss Combat Event Round Trip");

		testRunner.Require(
			decoded.CombatObjects[0].iCombatObjectId ==
				combatObject.iCombatObjectId &&
			decoded.CombatObjects[0].iSourceNetEntityId == 900u &&
			decoded.CombatObjects[0].fPositionX == 152.f &&
			decoded.CombatObjects[0].fPositionY == 24.f &&
			decoded.CombatObjects[0].fPositionZ == -120.f &&
			decoded.CombatObjects[0].fYawDegrees == 180.f,
			"World Snapshot Combat Object Round Trip");

		testRunner.Require(
			0 == reader.Get_RemainingSize(),
			"Consume Entire World Snapshot");

		S2C_WORLD_SNAPSHOT empty{};
		empty.iServerTick = 1;
		CPacketWriter emptyWriter;

		testRunner.Require(
			!Write_Message(emptyWriter, empty),
			"Reject Empty World Snapshot");

		S2C_WORLD_SNAPSHOT invalid = source;
		invalid.Players[0].eLocomotionState =
			PLAYER_LOCOMOTION_STATE::END;
		CPacketWriter invalidWriter;

		testRunner.Require(
			!Write_Message(invalidWriter, invalid),
			"Reject Invalid Locomotion State");

		{
			S2C_WORLD_SNAPSHOT overflow = source;
			overflow.Players[0].iComboStage = 9;
			CPacketWriter overflowWriter;
			testRunner.Require(
				!Write_Message(overflowWriter, overflow),
				"Reject Combo Stage Above Maximum");

			S2C_WORLD_SNAPSHOT idleCombo = source;
			idleCombo.Players[1].iComboStage = 1;
			CPacketWriter idleComboWriter;
			testRunner.Require(
				!Write_Message(idleComboWriter, idleCombo),
				"Reject Combo Stage Without Skill Action");

			S2C_WORLD_SNAPSHOT triggerWithoutTick = source;
			triggerWithoutTick.Players[1].iActionStartTick = 0;
			CPacketWriter triggerWithoutTickWriter;
			testRunner.Require(
				!Write_Message(triggerWithoutTickWriter, triggerWithoutTick),
				"Reject Trigger Move Without Action Tick");

			S2C_WORLD_SNAPSHOT dirtyAbsentTarget = source;
			dirtyAbsentTarget.Players[0].hasSkillTarget = false;
			CPacketWriter dirtyAbsentTargetWriter;
			testRunner.Require(
				!Write_Message(dirtyAbsentTargetWriter, dirtyAbsentTarget),
				"Reject Hidden Skill Target Coordinates");

			S2C_WORLD_SNAPSHOT targetOutsideSkill = source;
			targetOutsideSkill.Players[1].hasSkillTarget = true;
			targetOutsideSkill.Players[1].fSkillTargetX = 1.f;
			CPacketWriter targetOutsideSkillWriter;
			testRunner.Require(
				!Write_Message(targetOutsideSkillWriter, targetOutsideSkill),
				"Reject Skill Target Outside Skill Action");
			/* A fall carries no skill and must carry the tick a late joiner seeks
			the descent from, so both halves of that rule are pinned here. */
			S2C_WORLD_SNAPSHOT falling = source;
			falling.Players[1].eAction = PLAYER_ACTION_STATE::FALLING;
			falling.Players[1].iActionStartTick = 31;
			std::vector<std::uint8_t> fallingPayload;
			S2C_WORLD_SNAPSHOT decodedFalling{};
			bool fallingRoundTrip =
				Build_WorldSnapshotPayload(falling, fallingPayload);
			if (fallingRoundTrip)
			{
				CPacketReader fallingReader{ fallingPayload };
				fallingRoundTrip = Read_Message(fallingReader, decodedFalling);
			}
			testRunner.Require(
				fallingRoundTrip &&
				2u == decodedFalling.Players.size() &&
				decodedFalling.Players[1].eAction ==
					PLAYER_ACTION_STATE::FALLING &&
				decodedFalling.Players[1].iSkillId == INVALID_SKILL_ID &&
				decodedFalling.Players[1].iActionStartTick == 31,
				"Falling Player Snapshot Round Trip");

			S2C_WORLD_SNAPSHOT fallingWithSkill = falling;
			fallingWithSkill.Players[1].iSkillId = 34060;
			std::vector<std::uint8_t> fallingWithSkillPayload;
			testRunner.Require(
				!Build_WorldSnapshotPayload(
					fallingWithSkill, fallingWithSkillPayload),
				"Reject A Falling Snapshot That Carries A Skill");

			S2C_WORLD_SNAPSHOT fallingWithoutTick = falling;
			fallingWithoutTick.Players[1].iActionStartTick = 0;
			std::vector<std::uint8_t> fallingWithoutTickPayload;
			testRunner.Require(
				!Build_WorldSnapshotPayload(
					fallingWithoutTick, fallingWithoutTickPayload),
				"Reject A Falling Snapshot Without A Start Tick");

			S2C_WORLD_SNAPSHOT knockdown = source;
			knockdown.Players[1].eAction = PLAYER_ACTION_STATE::KNOCKDOWN;
			knockdown.Players[1].iSkillId = INVALID_SKILL_ID;
			knockdown.Players[1].iActionStartTick = 77;
			CPacketWriter knockdownWriter;
			testRunner.Require(
				Write_Message(knockdownWriter, knockdown),
				"Writer Knockdown Player Snapshot");

			S2C_WORLD_SNAPSHOT knockdownWithoutTick = knockdown;
			knockdownWithoutTick.Players[1].iActionStartTick = 0;
			CPacketWriter knockdownWithoutTickWriter;
			testRunner.Require(
				!Write_Message(knockdownWithoutTickWriter, knockdownWithoutTick),
				"Reject Knockdown Without Action Tick");

			S2C_WORLD_SNAPSHOT knockdownWithSkill = knockdown;
			knockdownWithSkill.Players[1].iSkillId = 34010;
			CPacketWriter knockdownWithSkillWriter;
			testRunner.Require(
				!Write_Message(knockdownWithSkillWriter, knockdownWithSkill),
				"Reject Knockdown That Carries A Skill");

			/* GRABBED is the only action allowed to carry an attachment owner and
			typed socket. It remains a timed, skill-less, idle state so a late
			joiner can enter the same attachment edge without inferring anything. */
			S2C_WORLD_SNAPSHOT grabbed = source;
			grabbed.Players[1].eLocomotionState =
				PLAYER_LOCOMOTION_STATE::IDLE;
			grabbed.Players[1].eAction = PLAYER_ACTION_STATE::GRABBED;
			grabbed.Players[1].iSkillId = INVALID_SKILL_ID;
			grabbed.Players[1].iActionStartTick = 82u;
			grabbed.Players[1].iAttachmentOwnerNetEntityId =
				entity.iNetEntityId;
			grabbed.Players[1].eAttachmentSlot =
				PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND;
			grabbed.Players[1].fAttachmentLocalOffsetX = 1.25f;
			grabbed.Players[1].fAttachmentLocalOffsetY = 2.5f;
			grabbed.Players[1].fAttachmentLocalOffsetZ = -0.75f;
			grabbed.Players[1].fAttachmentYawOffsetDegrees = 35.f;
			grabbed.Players[1].isCombatReady = false;
			std::vector<std::uint8_t> grabbedPayload;
			S2C_WORLD_SNAPSHOT decodedGrabbed{};
			bool grabbedRoundTrip =
				Build_WorldSnapshotPayload(grabbed, grabbedPayload);
			if (grabbedRoundTrip)
			{
				CPacketReader grabbedReader{ grabbedPayload };
				grabbedRoundTrip =
					Read_Message(grabbedReader, decodedGrabbed) &&
					0u == grabbedReader.Get_RemainingSize();
			}
			testRunner.Require(
				grabbedRoundTrip && 2u == decodedGrabbed.Players.size() &&
				PLAYER_ACTION_STATE::GRABBED ==
					decodedGrabbed.Players[1].eAction &&
				entity.iNetEntityId == decodedGrabbed.Players[1].
					iAttachmentOwnerNetEntityId &&
				PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND ==
					decodedGrabbed.Players[1].eAttachmentSlot &&
				1.25f == decodedGrabbed.Players[1].fAttachmentLocalOffsetX &&
				2.5f == decodedGrabbed.Players[1].fAttachmentLocalOffsetY &&
				-0.75f == decodedGrabbed.Players[1].fAttachmentLocalOffsetZ &&
				35.f == decodedGrabbed.Players[1].fAttachmentYawOffsetDegrees,
				"Grabbed Player Attachment Round Trip");

			S2C_WORLD_SNAPSHOT grabbedWithSkill = grabbed;
			grabbedWithSkill.Players[1].iSkillId = 34010;
			CPacketWriter grabbedWithSkillWriter;
			testRunner.Require(
				!Write_Message(grabbedWithSkillWriter, grabbedWithSkill),
				"Reject A Grabbed Snapshot That Carries A Skill");

			S2C_WORLD_SNAPSHOT grabbedWithoutTick = grabbed;
			grabbedWithoutTick.Players[1].iActionStartTick = 0u;
			CPacketWriter grabbedWithoutTickWriter;
			testRunner.Require(
				!Write_Message(grabbedWithoutTickWriter, grabbedWithoutTick),
				"Reject A Grabbed Snapshot Without A Start Tick");

			S2C_WORLD_SNAPSHOT grabbedNonFiniteOffset = grabbed;
			grabbedNonFiniteOffset.Players[1].fAttachmentLocalOffsetX =
				(std::numeric_limits<float>::quiet_NaN)();
			CPacketWriter grabbedNonFiniteOffsetWriter;
			testRunner.Require(
				!Write_Message(
					grabbedNonFiniteOffsetWriter, grabbedNonFiniteOffset),
				"Reject Grabbed Player Non-Finite Attachment Offset");

			S2C_WORLD_SNAPSHOT grabbedWithoutOwner = grabbed;
			grabbedWithoutOwner.Players[1].iAttachmentOwnerNetEntityId =
				INVALID_NET_ENTITY_ID;
			CPacketWriter grabbedWithoutOwnerWriter;
			testRunner.Require(
				!Write_Message(grabbedWithoutOwnerWriter, grabbedWithoutOwner),
				"Reject Grabbed Player Without Attachment Owner");

			S2C_WORLD_SNAPSHOT grabbedWithoutSlot = grabbed;
			grabbedWithoutSlot.Players[1].eAttachmentSlot =
				PLAYER_ATTACHMENT_SLOT::NONE;
			CPacketWriter grabbedWithoutSlotWriter;
			testRunner.Require(
				!Write_Message(grabbedWithoutSlotWriter, grabbedWithoutSlot),
				"Reject Grabbed Player Without Attachment Slot");

			S2C_WORLD_SNAPSHOT grabbedSelfOwned = grabbed;
			grabbedSelfOwned.Players[1].iAttachmentOwnerNetEntityId =
				grabbedSelfOwned.Players[1].iNetEntityId;
			CPacketWriter grabbedSelfOwnedWriter;
			testRunner.Require(
				!Write_Message(grabbedSelfOwnedWriter, grabbedSelfOwned),
				"Reject Self-Owned Player Attachment");

			S2C_WORLD_SNAPSHOT grabbedCombatReady = grabbed;
			grabbedCombatReady.Players[1].isCombatReady = true;
			CPacketWriter grabbedCombatReadyWriter;
			testRunner.Require(
				!Write_Message(grabbedCombatReadyWriter, grabbedCombatReady),
				"Reject Combat-Ready Grabbed Player");

			S2C_WORLD_SNAPSHOT attachmentOutsideGrab = source;
			attachmentOutsideGrab.Players[0].iAttachmentOwnerNetEntityId =
				entity.iNetEntityId;
			attachmentOutsideGrab.Players[0].eAttachmentSlot =
				PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND;
			attachmentOutsideGrab.Players[0].fAttachmentLocalOffsetX = 1.f;
			CPacketWriter attachmentOutsideGrabWriter;
			testRunner.Require(
				!Write_Message(
					attachmentOutsideGrabWriter, attachmentOutsideGrab),
				"Reject Attachment Fields Outside Grabbed Action");

			/* Read-side admission is independent of the writer: mutate only the
			typed slot byte in an otherwise valid one-player payload. */
			S2C_WORLD_SNAPSHOT oneGrabbed = grabbed;
			oneGrabbed.Players = { grabbed.Players[1] };
			std::vector<std::uint8_t> invalidSlotPayload;
			bool wroteInvalidSlotFixture =
				Build_WorldSnapshotPayload(oneGrabbed, invalidSlotPayload);
			constexpr std::size_t worldSnapshotHeaderBytes =
				4u + 2u + 2u + 2u + 1u + 1u + 1u + 4u + 4u;
			constexpr std::size_t playerAttachmentSlotByte =
				worldSnapshotHeaderBytes +
				4u + 1u + (4u * 4u) + 1u + 1u + 1u + 4u + 4u + 4u;
			if (wroteInvalidSlotFixture &&
				playerAttachmentSlotByte < invalidSlotPayload.size())
			{
				invalidSlotPayload[playerAttachmentSlotByte] =
					static_cast<std::uint8_t>(PLAYER_ATTACHMENT_SLOT::END);
			}
			CPacketReader invalidSlotReader{ invalidSlotPayload };
			S2C_WORLD_SNAPSHOT unchangedInvalidSlot{};
			unchangedInvalidSlot.iServerTick = 999u;
			testRunner.Require(
				wroteInvalidSlotFixture &&
				!Read_Message(invalidSlotReader, unchangedInvalidSlot) &&
				999u == unchangedInvalidSlot.iServerTick,
				"Reject Unknown Attachment Slot On Read Atomically");

			/* The Esther call carries no skill id (the summon is a roster slot)
			but must carry its start tick, same shape as FALLING/KNOCKDOWN. */
			S2C_WORLD_SNAPSHOT estherCast = source;
			estherCast.Players[1].eAction = PLAYER_ACTION_STATE::ESTHER_CAST;
			estherCast.Players[1].iSkillId = INVALID_SKILL_ID;
			estherCast.Players[1].iActionStartTick = 45;
			std::vector<std::uint8_t> estherCastPayload;
			S2C_WORLD_SNAPSHOT decodedEstherCast{};
			bool estherCastRoundTrip =
				Build_WorldSnapshotPayload(estherCast, estherCastPayload);
			if (estherCastRoundTrip)
			{
				CPacketReader estherCastReader{ estherCastPayload };
				estherCastRoundTrip =
					Read_Message(estherCastReader, decodedEstherCast);
			}
			testRunner.Require(
				estherCastRoundTrip &&
				2u == decodedEstherCast.Players.size() &&
				decodedEstherCast.Players[1].eAction ==
					PLAYER_ACTION_STATE::ESTHER_CAST &&
				decodedEstherCast.Players[1].iSkillId == INVALID_SKILL_ID &&
				decodedEstherCast.Players[1].iActionStartTick == 45,
				"Esther Cast Player Snapshot Round Trip");

			S2C_WORLD_SNAPSHOT estherCastWithSkill = estherCast;
			estherCastWithSkill.Players[1].iSkillId = 34010;
			CPacketWriter estherCastWithSkillWriter;
			testRunner.Require(
				!Write_Message(estherCastWithSkillWriter, estherCastWithSkill),
				"Reject An Esther Cast That Carries A Skill");

			S2C_WORLD_SNAPSHOT estherCastWithoutTick = estherCast;
			estherCastWithoutTick.Players[1].iActionStartTick = 0;
			CPacketWriter estherCastWithoutTickWriter;
			testRunner.Require(
				!Write_Message(
					estherCastWithoutTickWriter, estherCastWithoutTick),
				"Reject An Esther Cast Without A Start Tick");

			S2C_WORLD_SNAPSHOT phaseMismatch = source;
			phaseMismatch.Entities[0].BossCombat.iGameplayPhase = 2u;
			CPacketWriter phaseMismatchWriter;
			testRunner.Require(
				!Write_Message(phaseMismatchWriter, phaseMismatch),
				"Reject Boss Combat Phase Mismatch");

			S2C_WORLD_SNAPSHOT unknownBossFlag = source;
			unknownBossFlag.Entities[0].BossCombat.iFlags |= 0x8000u;
			CPacketWriter unknownBossFlagWriter;
			testRunner.Require(
				!Write_Message(unknownBossFlagWriter, unknownBossFlag),
				"Reject Unknown Boss Combat Flag");

			S2C_WORLD_SNAPSHOT staggerOverflow = source;
			staggerOverflow.Entities[0].BossCombat.iCurrentStagger = 1001u;
			CPacketWriter staggerOverflowWriter;
			testRunner.Require(
				!Write_Message(staggerOverflowWriter, staggerOverflow),
				"Reject Boss Stagger Above Maximum");

			S2C_WORLD_SNAPSHOT shieldFlagMismatch = source;
			shieldFlagMismatch.Entities[0].BossCombat.iCurrentShield = 10u;
			shieldFlagMismatch.Entities[0].BossCombat.iMaximumShield = 10u;
			CPacketWriter shieldFlagMismatchWriter;
			testRunner.Require(
				!Write_Message(shieldFlagMismatchWriter, shieldFlagMismatch),
				"Reject Boss Shield Without Flag");

			S2C_WORLD_SNAPSHOT dirtyAbsentBossState = source;
			dirtyAbsentBossState.Entities[0].hasBossCombatState = false;
			CPacketWriter dirtyAbsentBossStateWriter;
			testRunner.Require(
				!Write_Message(dirtyAbsentBossStateWriter, dirtyAbsentBossState),
				"Reject Hidden Boss Combat State");

			S2C_WORLD_SNAPSHOT zeroPartMask = source;
			zeroPartMask.BossCombatEvents[0].iPartMask = 0u;
			CPacketWriter zeroPartMaskWriter;
			testRunner.Require(
				!Write_Message(zeroPartMaskWriter, zeroPartMask),
				"Reject Boss Part Event Without Mask");

			S2C_WORLD_SNAPSHOT duplicateBossEvent = source;
			duplicateBossEvent.BossCombatEvents.push_back(partBroken);
			CPacketWriter duplicateBossEventWriter;
			testRunner.Require(
				!Write_Message(duplicateBossEventWriter, duplicateBossEvent),
				"Reject Duplicate Boss Combat Event Sequence");

			S2C_WORLD_SNAPSHOT uncommittedPartBreak = source;
			uncommittedPartBreak.Entities[0].BossCombat.iAlivePartMask |= 0x2u;
			CPacketWriter uncommittedPartBreakWriter;
			testRunner.Require(
				!Write_Message(uncommittedPartBreakWriter, uncommittedPartBreak),
				"Reject Part Event Before Persistent Mask Commit");

			S2C_WORLD_SNAPSHOT unknownEventBoss = source;
			unknownEventBoss.BossCombatEvents[0].iBossNetEntityId = 901u;
			CPacketWriter unknownEventBossWriter;
			testRunner.Require(
				!Write_Message(unknownEventBossWriter, unknownEventBoss),
				"Reject Boss Combat Event Without Snapshot Owner");

			S2C_WORLD_SNAPSHOT duplicateCombatObject = source;
			duplicateCombatObject.CombatObjects.push_back(combatObject);
			CPacketWriter duplicateCombatObjectWriter;
			testRunner.Require(
				!Write_Message(
					duplicateCombatObjectWriter, duplicateCombatObject),
				"Reject Duplicate Combat Object Snapshot ID");

			S2C_WORLD_SNAPSHOT unknownCombatObjectSource = source;
			unknownCombatObjectSource.CombatObjects[0].iSourceNetEntityId = 901u;
			CPacketWriter unknownCombatObjectSourceWriter;
			testRunner.Require(
				!Write_Message(
					unknownCombatObjectSourceWriter, unknownCombatObjectSource),
				"Reject Combat Object Without Snapshot Source");

			S2C_WORLD_SNAPSHOT nonFiniteCombatObject = source;
			nonFiniteCombatObject.CombatObjects[0].fPositionX =
				std::numeric_limits<float>::quiet_NaN();
			CPacketWriter nonFiniteCombatObjectWriter;
			testRunner.Require(
				!Write_Message(
					nonFiniteCombatObjectWriter, nonFiniteCombatObject),
				"Reject Non-Finite Combat Object Snapshot");
		}

		std::vector<std::uint8_t> unknownBossEventKindPayload = payload;
		unknownBossEventKindPayload[
			unknownBossEventKindPayload.size() - snapshotRevisionBytes -
				combatObjectBytes - 5u] =
			static_cast<std::uint8_t>(BOSS_COMBAT_EVENT_KIND::END);
		CPacketReader unknownBossEventKindReader{ unknownBossEventKindPayload };
		S2C_WORLD_SNAPSHOT unchangedUnknownKind{};
		unchangedUnknownKind.iServerTick = 778u;
		testRunner.Require(
			!Read_Message(unknownBossEventKindReader, unchangedUnknownKind) &&
			unchangedUnknownKind.iServerTick == 778u &&
			unchangedUnknownKind.Entities.empty(),
			"Reject Unknown Boss Combat Event Kind Atomically");

		payload.pop_back();
		CPacketReader truncatedReader{ payload };
		S2C_WORLD_SNAPSHOT unchanged{};
		unchanged.iServerTick = 777;

		testRunner.Require(
			!Read_Message(truncatedReader, unchanged),
			"Reject Truncated World Snapshot");

		testRunner.Require(
			unchanged.iServerTick == 777 &&
			unchanged.Players.empty(),
			"Failed Snapshot Does Not Mutate");
	}

	void Test_WorldDestructionProtocol(TEST_RUNNER& testRunner)
	{
		/* What this owns is the packet table, not the version number: every other
		case here compares against NETWORK_PROTOCOL_VERSION itself, and pinning a
		literal only made an unrelated bump fail this row. */
		testRunner.Require(
			Is_Known_Packet_Type(
				PACKET_TYPE::S2C_WORLD_DESTRUCTION_FULL_SYNC) &&
			Is_Known_Packet_Type(
				PACKET_TYPE::S2C_WORLD_DESTRUCTION_DELTA),
			"World Destruction Packet Types At Current Protocol");

		S2C_WORLD_DESTRUCTION_FULL_SYNC full{};
		full.strCombatRuntimeRevision = Make_CombatRuntimeRevision();
		full.iServerTick = 120u;
		full.iEncounterEpoch = 7u;
		full.GroupStates.push_back(Make_DestructionState(
			"destroyable.group.a",
			WORLD_DESTRUCTION_RUNTIME_STATE::BREAKING,
			2u));
		full.GroupStates.push_back(Make_DestructionState(
			"destroyable.group.b",
			WORLD_DESTRUCTION_RUNTIME_STATE::FRACTURED,
			3u));
		full.Diagnostics.iActiveWallCollisionCount = 30u;
		full.Diagnostics.iActiveNavBlockerRegionCount = 8u;
		full.Diagnostics.iNavigationRevision = 41u;
		full.Diagnostics.iLastEventSequence = 0u;

		CPacketWriter fullWriter;
		testRunner.Require(
			Write_Message(fullWriter, full),
			"Writer World Destruction Full Sync");
		const std::vector<std::uint8_t> validFullPayload =
			fullWriter.Get_Buffer();
		CPacketReader fullReader{ validFullPayload };
		S2C_WORLD_DESTRUCTION_FULL_SYNC decodedFull{};
		testRunner.Require(
			Read_Message(fullReader, decodedFull) &&
			0u == fullReader.Get_RemainingSize() &&
			decodedFull.strCombatRuntimeRevision ==
				full.strCombatRuntimeRevision &&
			decodedFull.iServerTick == full.iServerTick &&
			decodedFull.iEncounterEpoch == full.iEncounterEpoch &&
			2u == decodedFull.GroupStates.size() &&
			decodedFull.GroupStates[0].eState ==
				WORLD_DESTRUCTION_RUNTIME_STATE::BREAKING &&
			decodedFull.GroupStates[0].iCommitTick == 152u &&
			decodedFull.GroupStates[1].strGroupId ==
				"destroyable.group.b" &&
			decodedFull.Diagnostics.iActiveWallCollisionCount == 30u &&
			decodedFull.Diagnostics.iActiveNavBlockerRegionCount == 8u &&
			decodedFull.Diagnostics.iNavigationRevision == 41u &&
			decodedFull.Diagnostics.iLastEventSequence == 0u,
			"World Destruction Full Sync Round Trip");

		{
			S2C_WORLD_DESTRUCTION_FULL_SYNC invalid = full;
			invalid.strCombatRuntimeRevision[0] = 'A';
			CPacketWriter writer;
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Uppercase Combat Runtime Revision");
			invalid = full;
			invalid.strCombatRuntimeRevision.assign(64u, '0');
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Zero Combat Runtime Revision");
			invalid = full;
			invalid.iEncounterEpoch = 0u;
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Zero Destruction Encounter Epoch");
			invalid = full;
			invalid.GroupStates[0].iStateVersion = 0u;
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Zero Destruction State Version");
			invalid = full;
			invalid.GroupStates[0].strGroupId.assign(
				MAX_STABLE_NETWORK_ID_BYTES + 1u, 'a');
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Oversize Destruction Group Stable ID");
			invalid = full;
			invalid.GroupStates[0].strGroupId = "destroyable/group/a";
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Noncanonical Destruction Group Stable ID");
			invalid = full;
			invalid.GroupStates[0].iStateStartTick = 0u;
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Zero Destruction State Start Tick");
			invalid = full;
			invalid.GroupStates[0].iCommitTick = 0u;
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Breaking State Without Commit Tick");
			invalid = full;
			invalid.GroupStates[1].iCommitTick = 200u;
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Final State With Commit Tick");
			invalid = full;
			invalid.GroupStates[0].eState =
				WORLD_DESTRUCTION_RUNTIME_STATE::END;
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Invalid Destruction State Enum");
			invalid = full;
			std::swap(invalid.GroupStates[0], invalid.GroupStates[1]);
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Noncanonical Destruction Group Order");
			invalid = full;
			invalid.GroupStates[1].strGroupId =
				invalid.GroupStates[0].strGroupId;
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Duplicate Destruction Group");
			invalid = full;
			invalid.GroupStates.clear();
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Empty Destruction Full Sync");
			invalid = full;
			invalid.GroupStates.assign(
				MAX_WORLD_DESTRUCTION_GROUPS + 1u,
				full.GroupStates[0]);
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Destruction Full Sync Count Overflow");
		}

		{
			std::vector<std::uint8_t> malformed = validFullPayload;
			malformed.pop_back();
			CPacketReader reader{ malformed };
			S2C_WORLD_DESTRUCTION_FULL_SYNC unchanged{};
			unchanged.iServerTick = 999u;
			testRunner.Require(!Read_Message(reader, unchanged) &&
				999u == unchanged.iServerTick,
				"Reject Truncated Destruction Full Sync Atomically");

			malformed = validFullPayload;
			malformed[0] = 65u;
			CPacketReader revisionLengthReader{ malformed };
			testRunner.Require(!Read_Message(revisionLengthReader, unchanged),
				"Reject Oversize Combat Runtime Revision On Wire");

			malformed = validFullPayload;
			// string64 prefix + revision + server tick + epoch
			malformed[74] = 129u;
			malformed[75] = 0u;
			CPacketReader countReader{ malformed };
			testRunner.Require(!Read_Message(countReader, unchanged),
				"Reject Destruction Full Sync Wire Count Overflow");

			malformed = validFullPayload;
			malformed[76] = 129u;
			malformed[77] = 0u;
			CPacketReader stableIdLengthReader{ malformed };
			testRunner.Require(!Read_Message(stableIdLengthReader, unchanged),
				"Reject Oversize Destruction Stable ID On Wire");

			malformed = validFullPayload;
			const std::size_t firstStateEnumOffset =
				76u + 2u + full.GroupStates[0].strGroupId.size();
			malformed[firstStateEnumOffset] =
				static_cast<std::uint8_t>(
					WORLD_DESTRUCTION_RUNTIME_STATE::END);
			CPacketReader enumReader{ malformed };
			testRunner.Require(!Read_Message(enumReader, unchanged),
				"Reject Invalid Destruction State Enum On Wire");

			malformed = validFullPayload;
			const std::size_t firstStateBytes =
				2u + full.GroupStates[0].strGroupId.size() + 1u + 12u;
			const std::size_t secondGroupContentOffset =
				76u + firstStateBytes + 2u;
			malformed[secondGroupContentOffset +
				full.GroupStates[1].strGroupId.size() - 1u] = 'a';
			CPacketReader duplicateReader{ malformed };
			testRunner.Require(!Read_Message(duplicateReader, unchanged),
				"Reject Duplicate Destruction Group On Wire");
		}

		S2C_WORLD_DESTRUCTION_DELTA delta{};
		delta.strCombatRuntimeRevision = Make_CombatRuntimeRevision();
		delta.iServerTick = 121u;
		delta.iEncounterEpoch = 7u;
		delta.ChangedStates.push_back(full.GroupStates[0]);
		delta.LiveEvents.push_back(Make_DestructionEvent(10u));
		delta.LiveEvents.push_back(Make_DestructionEvent(11u));
		delta.Diagnostics.iActiveWallCollisionCount = 0u;
		delta.Diagnostics.iActiveNavBlockerRegionCount = 0u;
		delta.Diagnostics.iNavigationRevision = 42u;
		delta.Diagnostics.iLastEventSequence = 11u;

		CPacketWriter deltaWriter;
		testRunner.Require(
			Write_Message(deltaWriter, delta),
			"Writer World Destruction Delta");
		const std::vector<std::uint8_t> validDeltaPayload =
			deltaWriter.Get_Buffer();
		CPacketReader deltaReader{ validDeltaPayload };
		S2C_WORLD_DESTRUCTION_DELTA decodedDelta{};
		testRunner.Require(
			Read_Message(deltaReader, decodedDelta) &&
			0u == deltaReader.Get_RemainingSize() &&
			1u == decodedDelta.ChangedStates.size() &&
			2u == decodedDelta.LiveEvents.size() &&
			10u == decodedDelta.LiveEvents[0].iEventSequence &&
			11u == decodedDelta.LiveEvents[1].iEventSequence &&
			decodedDelta.LiveEvents[0].fImpactDirectionX == 0.6f &&
			decodedDelta.LiveEvents[0].iRandomSeed == 12345u &&
			decodedDelta.Diagnostics.iActiveWallCollisionCount == 0u &&
			decodedDelta.Diagnostics.iActiveNavBlockerRegionCount == 0u &&
			decodedDelta.Diagnostics.iNavigationRevision == 42u &&
			decodedDelta.Diagnostics.iLastEventSequence == 11u,
			"World Destruction Delta Round Trip");

		{
			S2C_WORLD_DESTRUCTION_DELTA invalid = delta;
			invalid.ChangedStates.clear();
			invalid.LiveEvents.clear();
			CPacketWriter writer;
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Empty Destruction Delta");
			invalid = delta;
			invalid.LiveEvents[0].iEventSequence = 0u;
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Zero Destruction Event Sequence");
			invalid = delta;
			std::swap(invalid.LiveEvents[0], invalid.LiveEvents[1]);
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Noncanonical Destruction Event Order");
			invalid = delta;
			invalid.LiveEvents[1].iEventSequence =
				invalid.LiveEvents[0].iEventSequence;
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Duplicate Destruction Event Sequence");
			invalid = delta;
			invalid.LiveEvents[0].iSourceNetEntityId = 0u;
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Zero Destruction Source Entity ID");
			invalid = delta;
			invalid.LiveEvents[0].iServerTick = 0u;
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Zero Destruction Event Server Tick");
			invalid = delta;
			invalid.LiveEvents[0].fImpactOriginX =
				(std::numeric_limits<float>::quiet_NaN)();
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Nonfinite Destruction Impact Origin");
			invalid = delta;
			invalid.LiveEvents[0].fImpactDirectionX = 0.f;
			invalid.LiveEvents[0].fImpactDirectionZ = 0.f;
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Zero Destruction Impact Direction");
			invalid = delta;
			invalid.LiveEvents[0].fImpactDirectionX = 1.f;
			invalid.LiveEvents[0].fImpactDirectionZ = 1.f;
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Nonunit Destruction Impact Direction");
			invalid = delta;
			invalid.LiveEvents.assign(
				MAX_WORLD_DESTRUCTION_EVENTS + 1u,
				delta.LiveEvents[0]);
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Destruction Delta Event Count Overflow");
			invalid = delta;
			invalid.ChangedStates.assign(
				MAX_WORLD_DESTRUCTION_CHANGED_STATES + 1u,
				delta.ChangedStates[0]);
			testRunner.Require(!Write_Message(writer, invalid),
				"Reject Destruction Delta State Count Overflow");
		}

		{
			constexpr std::size_t MAXIMUM_DELTA_PAYLOAD_BYTES = 65258u;
			constexpr std::size_t MAXIMUM_DELTA_FRAME_BYTES = 65264u;
			const S2C_WORLD_DESTRUCTION_DELTA maximumDelta =
				Make_MaximumSizedDestructionDelta(
					MAX_WORLD_DESTRUCTION_CHANGED_STATES,
					MAX_WORLD_DESTRUCTION_EVENTS);
			CPacketWriter maximumWriter;
			std::vector<std::uint8_t> maximumFrame;
			const bool wroteMaximum = Write_Message(maximumWriter, maximumDelta);
			const bool framedMaximum = wroteMaximum && Build_Packet_Frame(
				PACKET_TYPE::S2C_WORLD_DESTRUCTION_DELTA,
				maximumWriter.Get_Buffer(), maximumFrame);
			CPacketReader maximumReader{ maximumWriter.Get_Buffer() };
			S2C_WORLD_DESTRUCTION_DELTA decodedMaximum{};
			const bool readMaximum = wroteMaximum &&
				Read_Message(maximumReader, decodedMaximum) &&
				0u == maximumReader.Get_RemainingSize() &&
				MAX_WORLD_DESTRUCTION_CHANGED_STATES ==
					decodedMaximum.ChangedStates.size() &&
				MAX_WORLD_DESTRUCTION_EVENTS ==
					decodedMaximum.LiveEvents.size();
			testRunner.Require(
				framedMaximum && readMaximum &&
				MAXIMUM_DELTA_PAYLOAD_BYTES ==
					maximumWriter.Get_Buffer().size() &&
				MAXIMUM_DELTA_FRAME_BYTES == maximumFrame.size() &&
				maximumFrame.size() <= MAX_PACKET_BYTES,
				"Frame 128 Maximum Destruction States And 106 Maximum Events");

			constexpr std::size_t COLLAPSE_PAYLOAD_BYTES = 56847u;
			constexpr std::size_t COLLAPSE_FRAME_BYTES = 56853u;
			const S2C_WORLD_DESTRUCTION_DELTA collapseDelta =
				Make_MaximumSizedDestructionDelta(97u, 97u);
			CPacketWriter collapseWriter;
			std::vector<std::uint8_t> collapseFrame;
			const bool wroteCollapse = Write_Message(
				collapseWriter, collapseDelta);
			const bool framedCollapse = wroteCollapse && Build_Packet_Frame(
				PACKET_TYPE::S2C_WORLD_DESTRUCTION_DELTA,
				collapseWriter.Get_Buffer(), collapseFrame);
			testRunner.Require(
				framedCollapse &&
				COLLAPSE_PAYLOAD_BYTES == collapseWriter.Get_Buffer().size() &&
				COLLAPSE_FRAME_BYTES == collapseFrame.size() &&
				collapseFrame.size() <= MAX_PACKET_BYTES,
				"Frame Maximum-Length 97-State 97-Event Collapse Delta");

			const S2C_WORLD_DESTRUCTION_DELTA overflowDelta =
				Make_MaximumSizedDestructionDelta(
					MAX_WORLD_DESTRUCTION_CHANGED_STATES,
					MAX_WORLD_DESTRUCTION_EVENTS + 1u);
			CPacketWriter overflowWriter;
			testRunner.Require(
				!Write_Message(overflowWriter, overflowDelta),
				"Reject Canonical 107-Event Destruction Delta");
		}

		{
			std::vector<std::uint8_t> malformed = validDeltaPayload;
			malformed.pop_back();
			CPacketReader reader{ malformed };
			S2C_WORLD_DESTRUCTION_DELTA unchanged{};
			unchanged.iServerTick = 888u;
			testRunner.Require(!Read_Message(reader, unchanged) &&
				888u == unchanged.iServerTick,
				"Reject Truncated Destruction Delta Atomically");

			malformed = validDeltaPayload;
			malformed[76] = static_cast<std::uint8_t>(
				MAX_WORLD_DESTRUCTION_EVENTS + 1u);
			malformed[77] = 0u;
			CPacketReader countReader{ malformed };
			testRunner.Require(!Read_Message(countReader, unchanged),
				"Reject Destruction Delta Wire Count Overflow");

			S2C_WORLD_DESTRUCTION_DELTA eventOnly = delta;
			eventOnly.ChangedStates.clear();
			eventOnly.LiveEvents.resize(1u);
			CPacketWriter eventOnlyWriter;
			testRunner.Require(Write_Message(eventOnlyWriter, eventOnly),
				"Writer Event-only Destruction Delta");
			malformed = eventOnlyWriter.Get_Buffer();
			const WORLD_DESTRUCTION_EVENT_WIRE& event =
				eventOnly.LiveEvents[0];
			const std::size_t directionOffset =
				78u + 8u +
				(2u + event.strGroupId.size()) +
				(2u + event.strMutationId.size()) +
				(2u + event.strBindingId.size()) +
				4u + 8u + 4u + 12u;
			malformed[directionOffset] = 0u;
			malformed[directionOffset + 1u] = 0u;
			malformed[directionOffset + 2u] = 0xc0u;
			malformed[directionOffset + 3u] = 0x7fu;
			CPacketReader nanReader{ malformed };
			testRunner.Require(!Read_Message(nanReader, unchanged),
				"Reject Nonfinite Destruction Direction On Wire");

			S2C_WORLD_DESTRUCTION_DELTA eventPair = delta;
			eventPair.ChangedStates.clear();
			CPacketWriter pairWriter;
			testRunner.Require(Write_Message(pairWriter, eventPair),
				"Writer Event-pair Destruction Delta");
			malformed = pairWriter.Get_Buffer();
			const std::size_t eventBytes =
				8u + (2u + event.strGroupId.size()) +
				(2u + event.strMutationId.size()) +
				(2u + event.strBindingId.size()) +
				4u + 8u + 4u + 12u + 12u + 4u;
			const std::size_t secondSequenceOffset = 78u + eventBytes;
			for (std::size_t i = 0; i < 8u; ++i)
				malformed[secondSequenceOffset + i] = 0u;
			malformed[secondSequenceOffset] = 10u;
			CPacketReader duplicateReader{ malformed };
			testRunner.Require(!Read_Message(duplicateReader, unchanged),
				"Reject Duplicate Destruction Event On Wire");
		}
	}

	void Test_EncounterPropSyncProtocol(TEST_RUNNER& testRunner)
	{
		/* The pillars are the one encounter prop that repeats, so the wire has
		to carry a state that can go back to HIDDEN and an occurrence that says
		which cycle it belongs to. */
		S2C_ENCOUNTER_PROP_SYNC sync{};
		sync.strPropSetId = "encounterprop.valtan.four-pillars";
		sync.iServerTick = 4210u;
		sync.iEncounterEpoch = 3u;
		sync.Slots = {
			{ "pillar.valtan.slot00", ENCOUNTER_PROP_STATE::INTACT, 5u, 4100u, 8u },
			{ "pillar.valtan.slot01", ENCOUNTER_PROP_STATE::BREAKING, 6u, 4200u, 8u },
			{ "pillar.valtan.slot02", ENCOUNTER_PROP_STATE::HIDDEN, 7u, 4205u, 8u },
			{ "pillar.valtan.slot03", ENCOUNTER_PROP_STATE::SPAWNING, 1u, 4209u, 8u } };
		CPacketWriter writer;
		S2C_ENCOUNTER_PROP_SYNC decoded{};
		const bool wrote = Write_Message(writer, sync);
		CPacketReader reader{ writer.Get_Buffer() };
		testRunner.Require(
			Is_Known_Packet_Type(PACKET_TYPE::S2C_ENCOUNTER_PROP_SYNC) && wrote &&
			Read_Message(reader, decoded) &&
			0u == reader.Get_RemainingSize() &&
			decoded.strPropSetId == sync.strPropSetId &&
			decoded.iServerTick == sync.iServerTick &&
			decoded.iEncounterEpoch == sync.iEncounterEpoch &&
			4u == decoded.Slots.size() &&
			ENCOUNTER_PROP_STATE::BREAKING == decoded.Slots[1].eState &&
			4205u == decoded.Slots[2].iStateStartTick &&
			8u == decoded.Slots[3].iOccurrenceSequence,
			"Encounter Prop Sync Round Trip Across Every Slot State");

		S2C_ENCOUNTER_PROP_SYNC empty = sync;
		empty.Slots.clear();
		S2C_ENCOUNTER_PROP_SYNC epochless = sync;
		epochless.iEncounterEpoch = 0u;
		S2C_ENCOUNTER_PROP_SYNC versionless = sync;
		versionless.Slots[2].iStateVersion = 0u;
		CPacketWriter rejectWriter;
		testRunner.Require(
			!Write_Message(rejectWriter, empty) &&
			!Write_Message(rejectWriter, epochless) &&
			!Write_Message(rejectWriter, versionless),
			"Reject An Empty Epochless Or Unversioned Encounter Prop Sync");

		S2C_ENCOUNTER_PROP_SYNC unordered = sync;
		std::swap(unordered.Slots[0], unordered.Slots[1]);
		S2C_ENCOUNTER_PROP_SYNC duplicated = sync;
		duplicated.Slots[1].strSlotId = duplicated.Slots[0].strSlotId;
		CPacketWriter orderWriter;
		testRunner.Require(
			!Write_Message(orderWriter, unordered) &&
			!Write_Message(orderWriter, duplicated),
			"Reject Encounter Prop Slots That Are Unordered Or Duplicated");

		std::vector<std::uint8_t> malformed = writer.Get_Buffer();
		const std::size_t stateOffset = malformed.size() - 13u;
		malformed[stateOffset] =
			static_cast<std::uint8_t>(ENCOUNTER_PROP_STATE::END);
		CPacketReader malformedReader{ malformed };
		S2C_ENCOUNTER_PROP_SYNC untouched{};
		untouched.strPropSetId = "kept";
		testRunner.Require(
			!Read_Message(malformedReader, untouched) &&
			"kept" == untouched.strPropSetId,
			"Reject An Unknown Encounter Prop State On Wire");
	}

	void Test_ValtanAuditionProtocol(TEST_RUNNER& testRunner)
	{
		/* Both configurations must recognise the audition types. A Release
		Server answers REJECTED_RELEASE_BUILD; only an unknown type closes the
		session, and that distinction is what keeps a Debug Client's mistake
		diagnosable instead of a dropped connection. */
		testRunner.Require(
			Is_Known_Packet_Type(
				PACKET_TYPE::C2S_VALTAN_AUDITION_REQUEST) &&
			Is_Known_Packet_Type(
				PACKET_TYPE::S2C_VALTAN_AUDITION_RESULT),
			"Valtan Audition Packet Types Known In Every Configuration");

		C2S_VALTAN_AUDITION_REQUEST request{};
		request.iRequestSequence = 9u;
		request.eOperation = VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR;
		request.iTargetHealthBar = 80u;
		CPacketWriter requestWriter;
		testRunner.Require(
			Write_Message(requestWriter, request),
			"Writer Valtan Audition Request");
		const std::vector<std::uint8_t> legacyRequestGolden{
			0x09u, 0x00u, 0x00u, 0x00u,
			0x02u,
			0x50u, 0x00u, 0x00u, 0x00u };
		testRunner.Require(
			requestWriter.Get_Buffer() == legacyRequestGolden,
			"Preserve Legacy Valtan Audition Request Golden Payload");
		CPacketReader requestReader{ requestWriter.Get_Buffer() };
		C2S_VALTAN_AUDITION_REQUEST decodedRequest{};
		testRunner.Require(
			Read_Message(requestReader, decodedRequest) &&
			0u == requestReader.Get_RemainingSize() &&
			decodedRequest.iRequestSequence == request.iRequestSequence &&
			decodedRequest.eOperation == request.eOperation &&
			decodedRequest.iTargetHealthBar == request.iTargetHealthBar,
			"Valtan Audition Request Round Trip");

		{
			/* The entrance sweep is the encounter intro, not a health-bar
			crossing, so it carries exactly zero and a bar makes it invalid. */
			C2S_VALTAN_AUDITION_REQUEST entrance{};
			entrance.iRequestSequence = 11u;
			entrance.eOperation = VALTAN_AUDITION_OPERATION::PLAY_ENTRANCE;
			entrance.iTargetHealthBar = 0u;
			CPacketWriter entranceWriter;
			C2S_VALTAN_AUDITION_REQUEST decodedEntrance{};
			const bool wroteEntrance = Write_Message(entranceWriter, entrance);
			CPacketReader entranceReader{ entranceWriter.Get_Buffer() };
			testRunner.Require(
				wroteEntrance &&
				Read_Message(entranceReader, decodedEntrance) &&
				0u == entranceReader.Get_RemainingSize() &&
				VALTAN_AUDITION_OPERATION::PLAY_ENTRANCE ==
					decodedEntrance.eOperation &&
				0u == decodedEntrance.iTargetHealthBar,
				"Valtan Entrance Audition Request Round Trip Without A Bar");

			C2S_VALTAN_AUDITION_REQUEST barredEntrance = entrance;
			barredEntrance.iTargetHealthBar = 159u;
			CPacketWriter barredWriter;
			testRunner.Require(
				!Write_Message(barredWriter, barredEntrance),
				"Reject A Valtan Entrance Audition Request That Carries A Bar");
		}

		{
			/* The pillar cycle names the mechanic. Its own authored pattern owns
			the bar, so the request carries none for the same reason. */
			C2S_VALTAN_AUDITION_REQUEST pillar{};
			pillar.iRequestSequence = 12u;
			pillar.eOperation = VALTAN_AUDITION_OPERATION::PLAY_PILLAR_CYCLE;
			pillar.iTargetHealthBar = 0u;
			CPacketWriter pillarWriter;
			C2S_VALTAN_AUDITION_REQUEST decodedPillar{};
			const bool wrotePillar = Write_Message(pillarWriter, pillar);
			CPacketReader pillarReader{ pillarWriter.Get_Buffer() };
			testRunner.Require(
				wrotePillar &&
				Read_Message(pillarReader, decodedPillar) &&
				0u == pillarReader.Get_RemainingSize() &&
				VALTAN_AUDITION_OPERATION::PLAY_PILLAR_CYCLE ==
					decodedPillar.eOperation &&
				0u == decodedPillar.iTargetHealthBar,
				"Valtan Pillar Cycle Audition Request Round Trip Without A Bar");

			C2S_VALTAN_AUDITION_REQUEST barredPillar = pillar;
			barredPillar.iTargetHealthBar = 100u;
			CPacketWriter barredPillarWriter;
			testRunner.Require(
				!Write_Message(barredPillarWriter, barredPillar),
				"Reject A Valtan Pillar Cycle Audition Request That Carries A Bar");
		}

		{
			/* Named Debug operations are not health-bar crossings. Each must
			round-trip with an empty bar and reject any accidental bar payload. */
			const std::array<VALTAN_AUDITION_OPERATION, 3u> barlessOperations{
				VALTAN_AUDITION_OPERATION::PLAY_WALL_ATTACK,
				VALTAN_AUDITION_OPERATION::SHOW_FINAL_ARENA,
				VALTAN_AUDITION_OPERATION::BREAK_EVERY_WALL };
			for (std::size_t index = 0u; index < barlessOperations.size(); ++index)
			{
				C2S_VALTAN_AUDITION_REQUEST barless{};
				barless.iRequestSequence = 20u +
					static_cast<std::uint32_t>(index);
				barless.eOperation = barlessOperations[index];
				barless.iTargetHealthBar = 0u;
				CPacketWriter barlessWriter;
				C2S_VALTAN_AUDITION_REQUEST decodedBarless{};
				const bool wrote = Write_Message(barlessWriter, barless);
				CPacketReader barlessReader{ barlessWriter.Get_Buffer() };
				testRunner.Require(
					wrote && Read_Message(barlessReader, decodedBarless) &&
					0u == barlessReader.Get_RemainingSize() &&
					decodedBarless.eOperation == barless.eOperation &&
					0u == decodedBarless.iTargetHealthBar,
					"Valtan Named Debug Auditions Round Trip Without A Bar");

				barless.iTargetHealthBar = 109u;
				CPacketWriter barredWriter;
				testRunner.Require(
					!Write_Message(barredWriter, barless),
					"Reject Valtan Named Debug Auditions That Carry A Bar");
			}
		}

		{
			/* Timeline PLAY addresses one authored chronological row by a stable
			non-zero command ID. STOP names the active playback itself, not a row. */
			testRunner.Require(
				8u == static_cast<std::uint8_t>(
					VALTAN_AUDITION_OPERATION::PLAY_TIMELINE_ROW) &&
				9u == static_cast<std::uint8_t>(
					VALTAN_AUDITION_OPERATION::STOP_TIMELINE_ROW) &&
				10u == static_cast<std::uint8_t>(
					VALTAN_AUDITION_OPERATION::PLAY_PATTERN) &&
				11u == static_cast<std::uint8_t>(
					VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID) &&
				12u == static_cast<std::uint8_t>(
					VALTAN_AUDITION_OPERATION::START_FIGHT_PAGE),
				"Valtan Timeline Operations Preserve Existing Wire Ordinals");

			C2S_VALTAN_AUDITION_REQUEST timelinePlay{};
			timelinePlay.iRequestSequence = 25u;
			timelinePlay.eOperation =
				VALTAN_AUDITION_OPERATION::PLAY_TIMELINE_ROW;
			constexpr std::uint32_t TIMELINE_COMMAND_ID = 0x5A17B33Fu;
			timelinePlay.iTargetHealthBar = TIMELINE_COMMAND_ID;
			CPacketWriter timelineWriter;
			C2S_VALTAN_AUDITION_REQUEST decodedTimeline{};
			const bool wroteTimeline = Write_Message(timelineWriter, timelinePlay);
			CPacketReader timelineReader{ timelineWriter.Get_Buffer() };
			testRunner.Require(
				wroteTimeline && Read_Message(timelineReader, decodedTimeline) &&
				0u == timelineReader.Get_RemainingSize() &&
				VALTAN_AUDITION_OPERATION::PLAY_TIMELINE_ROW ==
					decodedTimeline.eOperation &&
				TIMELINE_COMMAND_ID == decodedTimeline.iTargetHealthBar,
				"Valtan Timeline Row Round Trips Its Stable Command Id");

			C2S_VALTAN_AUDITION_REQUEST zeroTimeline = timelinePlay;
			zeroTimeline.iTargetHealthBar = 0u;
			CPacketWriter zeroTimelineWriter;
			testRunner.Require(
				!Write_Message(zeroTimelineWriter, zeroTimeline),
				"Reject A Valtan Timeline Play Without A Command Id");

			C2S_VALTAN_AUDITION_REQUEST timelineStop{};
			timelineStop.iRequestSequence = 26u;
			timelineStop.eOperation =
				VALTAN_AUDITION_OPERATION::STOP_TIMELINE_ROW;
			timelineStop.iTargetHealthBar = 0u;
			CPacketWriter stopWriter;
			C2S_VALTAN_AUDITION_REQUEST decodedStop{};
			const bool wroteStop = Write_Message(stopWriter, timelineStop);
			CPacketReader stopReader{ stopWriter.Get_Buffer() };
			testRunner.Require(
				wroteStop && Read_Message(stopReader, decodedStop) &&
				0u == stopReader.Get_RemainingSize() &&
				VALTAN_AUDITION_OPERATION::STOP_TIMELINE_ROW ==
					decodedStop.eOperation &&
				0u == decodedStop.iTargetHealthBar,
				"Valtan Timeline Stop Round Trips Without A Command Id");

			C2S_VALTAN_AUDITION_REQUEST numberedStop = timelineStop;
			numberedStop.iTargetHealthBar = TIMELINE_COMMAND_ID;
			CPacketWriter numberedStopWriter;
			testRunner.Require(
				!Write_Message(numberedStopWriter, numberedStop),
				"Reject A Valtan Timeline Stop That Carries A Command Id");

			S2C_VALTAN_AUDITION_RESULT timelineResult{};
			timelineResult.iRequestSequence = timelinePlay.iRequestSequence;
			timelineResult.eOperation = timelinePlay.eOperation;
			timelineResult.iTargetHealthBar = timelinePlay.iTargetHealthBar;
			timelineResult.eResult = VALTAN_AUDITION_RESULT::QUEUED;
			timelineResult.iCurrentHealthBar = 109u;
			CPacketWriter timelineResultWriter;
			S2C_VALTAN_AUDITION_RESULT decodedTimelineResult{};
			const bool wroteTimelineResult =
				Write_Message(timelineResultWriter, timelineResult);
			CPacketReader timelineResultReader{
				timelineResultWriter.Get_Buffer() };
			testRunner.Require(
				wroteTimelineResult &&
				Read_Message(timelineResultReader, decodedTimelineResult) &&
				0u == timelineResultReader.Get_RemainingSize() &&
				decodedTimelineResult.iRequestSequence ==
					timelineResult.iRequestSequence &&
				VALTAN_AUDITION_OPERATION::PLAY_TIMELINE_ROW ==
					decodedTimelineResult.eOperation &&
				TIMELINE_COMMAND_ID == decodedTimelineResult.iTargetHealthBar &&
				VALTAN_AUDITION_RESULT::QUEUED ==
					decodedTimelineResult.eResult &&
				109u == decodedTimelineResult.iCurrentHealthBar,
				"Valtan Timeline Result Echoes The Selected Command Id");

			C2S_VALTAN_AUDITION_REQUEST pageStart{};
			pageStart.iRequestSequence = 27u;
			pageStart.eOperation =
				VALTAN_AUDITION_OPERATION::START_FIGHT_PAGE;
			pageStart.iTargetHealthBar = TIMELINE_COMMAND_ID;
			CPacketWriter pageWriter;
			C2S_VALTAN_AUDITION_REQUEST decodedPage{};
			const bool wrotePage = Write_Message(pageWriter, pageStart);
			CPacketReader pageReader{ pageWriter.Get_Buffer() };
			testRunner.Require(
				wrotePage && Read_Message(pageReader, decodedPage) &&
				0u == pageReader.Get_RemainingSize() &&
				VALTAN_AUDITION_OPERATION::START_FIGHT_PAGE ==
					decodedPage.eOperation &&
				TIMELINE_COMMAND_ID == decodedPage.iTargetHealthBar,
				"Valtan Fight Page Start Round Trips Its Stable Row Id");

			C2S_VALTAN_AUDITION_REQUEST zeroPage = pageStart;
			zeroPage.iTargetHealthBar = 0u;
			CPacketWriter zeroPageWriter;
			testRunner.Require(
				!Write_Message(zeroPageWriter, zeroPage),
				"Reject A Valtan Fight Page Start Without A Stable Row Id");
		}

		{
			/* The pattern browser reuses the bar field as a one-based index into
			the authored pattern order, so it round-trips a non-zero payload and
			still rejects the zero that would mean "no bar". */
			C2S_VALTAN_AUDITION_REQUEST patternPlay{};
			patternPlay.iRequestSequence = 31u;
			patternPlay.eOperation = VALTAN_AUDITION_OPERATION::PLAY_PATTERN;
			patternPlay.iTargetHealthBar = 17u;
			CPacketWriter patternWriter;
			C2S_VALTAN_AUDITION_REQUEST decodedPattern{};
			const bool wrotePattern = Write_Message(patternWriter, patternPlay);
			CPacketReader patternReader{ patternWriter.Get_Buffer() };
			testRunner.Require(
				wrotePattern && Read_Message(patternReader, decodedPattern) &&
				0u == patternReader.Get_RemainingSize() &&
				VALTAN_AUDITION_OPERATION::PLAY_PATTERN ==
					decodedPattern.eOperation &&
				17u == decodedPattern.iTargetHealthBar,
				"Valtan Pattern Browser Round Trips Its One-Based Pattern Index");

			patternPlay.iTargetHealthBar = 0u;
			CPacketWriter emptyPatternWriter;
			testRunner.Require(
				!Write_Message(emptyPatternWriter, patternPlay),
				"Reject A Valtan Pattern Browser Request Without An Index");
		}

		{
			/* Effect Tool rows already own the authored stable pattern ID. This
			shape also names the stable boss placement, so no reordered Client
			vector position can select a different Server pattern. */
			C2S_VALTAN_AUDITION_REQUEST stablePlay{};
			stablePlay.iRequestSequence = 32u;
			stablePlay.eOperation =
				VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID;
			stablePlay.iTargetHealthBar = 0u;
			stablePlay.strBossPlacementId =
				"boss.valtan.character-select.lazy";
			stablePlay.strPatternId = "VALTAN_DASH_CHARGE";
			CPacketWriter stableWriter;
			C2S_VALTAN_AUDITION_REQUEST decodedStable{};
			const bool wroteStable = Write_Message(stableWriter, stablePlay);
			CPacketReader stableReader{ stableWriter.Get_Buffer() };
			testRunner.Require(
				wroteStable && Read_Message(stableReader, decodedStable) &&
				0u == stableReader.Get_RemainingSize() &&
				decodedStable.iRequestSequence == stablePlay.iRequestSequence &&
				decodedStable.eOperation == stablePlay.eOperation &&
				0u == decodedStable.iTargetHealthBar &&
				decodedStable.strBossPlacementId ==
					stablePlay.strBossPlacementId &&
				decodedStable.strPatternId == stablePlay.strPatternId,
				"Valtan Stable-ID Pattern Audition Request Round Trip");

			const auto requestDestinationUnchanged = [](
				const C2S_VALTAN_AUDITION_REQUEST& value)
				{
					return 777u == value.iRequestSequence &&
						VALTAN_AUDITION_OPERATION::PLAY_ENTRANCE ==
							value.eOperation &&
						159u == value.iTargetHealthBar &&
						"keep-boss" == value.strBossPlacementId &&
						"KEEP_PATTERN" == value.strPatternId;
				};
			const auto makeUnchangedRequest = []()
				{
					C2S_VALTAN_AUDITION_REQUEST unchanged{};
					unchanged.iRequestSequence = 777u;
					unchanged.eOperation =
						VALTAN_AUDITION_OPERATION::PLAY_ENTRANCE;
					unchanged.iTargetHealthBar = 159u;
					unchanged.strBossPlacementId = "keep-boss";
					unchanged.strPatternId = "KEEP_PATTERN";
					return unchanged;
				};
			constexpr std::size_t requestFixedBytes = 9u;
			std::vector<std::uint8_t> truncatedFirst =
				stableWriter.Get_Buffer();
			truncatedFirst.resize(
				requestFixedBytes + sizeof(std::uint16_t) +
				stablePlay.strBossPlacementId.size() - 1u);
			CPacketReader truncatedFirstReader{ truncatedFirst };
			C2S_VALTAN_AUDITION_REQUEST unchangedFirst =
				makeUnchangedRequest();
			testRunner.Require(
				!Read_Message(truncatedFirstReader, unchangedFirst) &&
				requestDestinationUnchanged(unchangedFirst),
				"Reject Truncated First Stable ID Without Mutating Valtan Request");

			std::vector<std::uint8_t> truncatedSecond =
				stableWriter.Get_Buffer();
			truncatedSecond.pop_back();
			CPacketReader truncatedSecondReader{ truncatedSecond };
			C2S_VALTAN_AUDITION_REQUEST unchangedSecond =
				makeUnchangedRequest();
			testRunner.Require(
				!Read_Message(truncatedSecondReader, unchangedSecond) &&
				requestDestinationUnchanged(unchangedSecond),
				"Reject Truncated Second Stable ID Without Mutating Valtan Request");

			C2S_VALTAN_AUDITION_REQUEST barredStable = stablePlay;
			barredStable.iTargetHealthBar = 1u;
			CPacketWriter barredStableWriter;
			testRunner.Require(
				!Write_Message(barredStableWriter, barredStable),
				"Reject Stable-ID Pattern Audition With A Legacy Index");

			C2S_VALTAN_AUDITION_REQUEST missingPattern = stablePlay;
			missingPattern.strPatternId.clear();
			CPacketWriter missingPatternWriter;
			testRunner.Require(
				!Write_Message(missingPatternWriter, missingPattern),
				"Reject Stable-ID Pattern Audition Without A Pattern ID");

			C2S_VALTAN_AUDITION_REQUEST oversizeBoss = stablePlay;
			oversizeBoss.strBossPlacementId.assign(
				MAX_STABLE_NETWORK_ID_BYTES + 1u, 'a');
			C2S_VALTAN_AUDITION_REQUEST oversizePattern = stablePlay;
			oversizePattern.strPatternId.assign(
				MAX_STABLE_NETWORK_ID_BYTES + 1u, 'A');
			CPacketWriter oversizeBossWriter;
			CPacketWriter oversizePatternWriter;
			testRunner.Require(
				!Write_Message(oversizeBossWriter, oversizeBoss) &&
				!Write_Message(oversizePatternWriter, oversizePattern),
				"Reject Oversize Stable IDs On Valtan Pattern Audition");

			C2S_VALTAN_AUDITION_REQUEST noncanonicalBoss = stablePlay;
			noncanonicalBoss.strBossPlacementId = "boss/valtan";
			C2S_VALTAN_AUDITION_REQUEST noncanonicalPattern = stablePlay;
			noncanonicalPattern.strPatternId = "VALTAN:DASH_CHARGE";
			CPacketWriter noncanonicalBossWriter;
			CPacketWriter noncanonicalPatternWriter;
			testRunner.Require(
				!Write_Message(noncanonicalBossWriter, noncanonicalBoss) &&
				!Write_Message(noncanonicalPatternWriter, noncanonicalPattern),
				"Reject Noncanonical Stable IDs On Valtan Pattern Audition");

			C2S_VALTAN_AUDITION_REQUEST hiddenIds = request;
			hiddenIds.strBossPlacementId = stablePlay.strBossPlacementId;
			hiddenIds.strPatternId = stablePlay.strPatternId;
			CPacketWriter hiddenIdsWriter;
			testRunner.Require(
				!Write_Message(hiddenIdsWriter, hiddenIds),
				"Reject Stable IDs On A Legacy Valtan Audition Shape");
		}

		{
			C2S_VALTAN_AUDITION_REQUEST invalid = request;
			invalid.iRequestSequence = 0u;
			CPacketWriter writer;
			testRunner.Require(
				!Write_Message(writer, invalid),
				"Reject Valtan Audition Request Without A Sequence");

			invalid = request;
			invalid.iTargetHealthBar = 0u;
			testRunner.Require(
				!Write_Message(writer, invalid),
				"Reject Valtan Audition Request On The Dead Bar");

			invalid = request;
			invalid.eOperation = VALTAN_AUDITION_OPERATION::END;
			testRunner.Require(
				!Write_Message(writer, invalid),
				"Reject Valtan Audition Request With An Unknown Operation");

			std::vector<std::uint8_t> malformed = requestWriter.Get_Buffer();
			malformed[4] = static_cast<std::uint8_t>(
				VALTAN_AUDITION_OPERATION::END);
			CPacketReader operationReader{ malformed };
			C2S_VALTAN_AUDITION_REQUEST unchanged{};
			unchanged.iRequestSequence = 123u;
			testRunner.Require(
				!Read_Message(operationReader, unchanged) &&
				123u == unchanged.iRequestSequence,
				"Reject Unknown Valtan Audition Operation On Wire");
		}

		S2C_VALTAN_AUDITION_RESULT result{};
		result.iRequestSequence = 9u;
		result.eOperation = VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR;
		result.iTargetHealthBar = 80u;
		result.eResult = VALTAN_AUDITION_RESULT::QUEUED;
		result.iCurrentHealthBar = 80u;
		CPacketWriter resultWriter;
		testRunner.Require(
			Write_Message(resultWriter, result),
			"Writer Valtan Audition Result");
		const std::vector<std::uint8_t> legacyResultGolden{
			0x09u, 0x00u, 0x00u, 0x00u,
			0x02u,
			0x50u, 0x00u, 0x00u, 0x00u,
			0x01u,
			0x50u, 0x00u, 0x00u, 0x00u };
		testRunner.Require(
			resultWriter.Get_Buffer() == legacyResultGolden,
			"Preserve Legacy Valtan Audition Result Golden Payload");
		CPacketReader resultReader{ resultWriter.Get_Buffer() };
		S2C_VALTAN_AUDITION_RESULT decodedResult{};
		testRunner.Require(
			Read_Message(resultReader, decodedResult) &&
			0u == resultReader.Get_RemainingSize() &&
			decodedResult.iRequestSequence == result.iRequestSequence &&
			decodedResult.eOperation == result.eOperation &&
			decodedResult.iTargetHealthBar == result.iTargetHealthBar &&
			decodedResult.eResult == result.eResult &&
			decodedResult.iCurrentHealthBar == result.iCurrentHealthBar,
			"Valtan Audition Result Round Trip");

		{
			S2C_VALTAN_AUDITION_RESULT stableResult{};
			stableResult.iRequestSequence = 32u;
			stableResult.eOperation =
				VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID;
			stableResult.iTargetHealthBar = 0u;
			stableResult.eResult = VALTAN_AUDITION_RESULT::QUEUED;
			stableResult.iCurrentHealthBar = 160u;
			stableResult.strBossPlacementId =
				"boss.valtan.character-select.lazy";
			stableResult.strPatternId = "VALTAN_DASH_CHARGE";
			CPacketWriter stableResultWriter;
			S2C_VALTAN_AUDITION_RESULT decodedStableResult{};
			const bool wroteStableResult =
				Write_Message(stableResultWriter, stableResult);
			CPacketReader stableResultReader{
				stableResultWriter.Get_Buffer() };
			testRunner.Require(
				wroteStableResult &&
				Read_Message(stableResultReader, decodedStableResult) &&
				0u == stableResultReader.Get_RemainingSize() &&
				decodedStableResult.iRequestSequence ==
					stableResult.iRequestSequence &&
				decodedStableResult.eOperation == stableResult.eOperation &&
				decodedStableResult.eResult == stableResult.eResult &&
				decodedStableResult.iCurrentHealthBar ==
					stableResult.iCurrentHealthBar &&
				decodedStableResult.strBossPlacementId ==
					stableResult.strBossPlacementId &&
				decodedStableResult.strPatternId == stableResult.strPatternId,
				"Valtan Stable-ID Pattern Audition Result Round Trip");

			const auto resultDestinationUnchanged = [](
				const S2C_VALTAN_AUDITION_RESULT& value)
				{
					return 888u == value.iRequestSequence &&
						VALTAN_AUDITION_OPERATION::PLAY_ENTRANCE ==
							value.eOperation &&
						159u == value.iTargetHealthBar &&
						VALTAN_AUDITION_RESULT::REJECTED_NO_BOSS ==
							value.eResult &&
						77u == value.iCurrentHealthBar &&
						"keep-result-boss" == value.strBossPlacementId &&
						"KEEP_RESULT_PATTERN" == value.strPatternId;
				};
			const auto makeUnchangedResult = []()
				{
					S2C_VALTAN_AUDITION_RESULT unchanged{};
					unchanged.iRequestSequence = 888u;
					unchanged.eOperation =
						VALTAN_AUDITION_OPERATION::PLAY_ENTRANCE;
					unchanged.iTargetHealthBar = 159u;
					unchanged.eResult =
						VALTAN_AUDITION_RESULT::REJECTED_NO_BOSS;
					unchanged.iCurrentHealthBar = 77u;
					unchanged.strBossPlacementId = "keep-result-boss";
					unchanged.strPatternId = "KEEP_RESULT_PATTERN";
					return unchanged;
				};
			constexpr std::size_t resultFixedBytes = 14u;
			std::vector<std::uint8_t> truncatedFirst =
				stableResultWriter.Get_Buffer();
			truncatedFirst.resize(
				resultFixedBytes + sizeof(std::uint16_t) +
				stableResult.strBossPlacementId.size() - 1u);
			CPacketReader truncatedFirstReader{ truncatedFirst };
			S2C_VALTAN_AUDITION_RESULT unchangedFirst =
				makeUnchangedResult();
			testRunner.Require(
				!Read_Message(truncatedFirstReader, unchangedFirst) &&
				resultDestinationUnchanged(unchangedFirst),
				"Reject Truncated First Stable ID Without Mutating Valtan Result");

			std::vector<std::uint8_t> truncatedSecond =
				stableResultWriter.Get_Buffer();
			truncatedSecond.pop_back();
			CPacketReader truncatedSecondReader{ truncatedSecond };
			S2C_VALTAN_AUDITION_RESULT unchangedSecond =
				makeUnchangedResult();
			testRunner.Require(
				!Read_Message(truncatedSecondReader, unchangedSecond) &&
				resultDestinationUnchanged(unchangedSecond),
				"Reject Truncated Second Stable ID Without Mutating Valtan Result");
		}

		{
			S2C_VALTAN_AUDITION_RESULT rejection = result;
			rejection.eResult =
				VALTAN_AUDITION_RESULT::REJECTED_RELEASE_BUILD;
			rejection.iCurrentHealthBar = 0u;
			CPacketWriter rejectionWriter;
			S2C_VALTAN_AUDITION_RESULT decodedRejection{};
			CPacketReader rejectionReader{
				(Write_Message(rejectionWriter, rejection),
					rejectionWriter.Get_Buffer()) };
			testRunner.Require(
				Read_Message(rejectionReader, decodedRejection) &&
				VALTAN_AUDITION_RESULT::REJECTED_RELEASE_BUILD ==
					decodedRejection.eResult &&
				0u == decodedRejection.iCurrentHealthBar,
				"Valtan Audition Release Rejection Round Trip");

			S2C_VALTAN_AUDITION_RESULT invalid = result;
			invalid.eResult = VALTAN_AUDITION_RESULT::END;
			CPacketWriter writer;
			testRunner.Require(
				!Write_Message(writer, invalid),
				"Reject Valtan Audition Result With An Unknown Verdict");

			std::vector<std::uint8_t> malformed = resultWriter.Get_Buffer();
			malformed[9] = static_cast<std::uint8_t>(
				VALTAN_AUDITION_RESULT::END);
			CPacketReader verdictReader{ malformed };
			S2C_VALTAN_AUDITION_RESULT unchanged{};
			unchanged.iRequestSequence = 321u;
			testRunner.Require(
				!Read_Message(verdictReader, unchanged) &&
				321u == unchanged.iRequestSequence,
				"Reject Unknown Valtan Audition Verdict On Wire");
		}
	}

	void Test_WorldEntitySpawnCommandRoundTrip(TEST_RUNNER& testRunner)
	{
		C2S_SPAWN_WORLD_ENTITY request{};
		request.strPlacementId = "boss.valtan.character-select.lazy";
		CPacketWriter requestWriter;
		testRunner.Require(
			Write_Message(requestWriter, request),
			"Writer World Entity Spawn Request");
		CPacketReader requestReader{ requestWriter.Get_Buffer() };
		C2S_SPAWN_WORLD_ENTITY decodedRequest{};
		testRunner.Require(
			Read_Message(requestReader, decodedRequest) &&
			decodedRequest.strPlacementId == request.strPlacementId &&
			0u == requestReader.Get_RemainingSize(),
			"World Entity Spawn Request Round Trip");

		C2S_SPAWN_WORLD_ENTITY invalidRequest{};
		CPacketWriter invalidRequestWriter;
		testRunner.Require(
			!Write_Message(invalidRequestWriter, invalidRequest),
			"Reject Empty World Entity Spawn Placement");
		invalidRequest.strPlacementId = "boss/valtan";
		testRunner.Require(
			!Write_Message(invalidRequestWriter, invalidRequest),
			"Reject Invalid World Entity Spawn Placement");
		invalidRequest.strPlacementId.assign(
			MAX_STABLE_NETWORK_ID_BYTES + 1u,
			'a');
		testRunner.Require(
			!Write_Message(invalidRequestWriter, invalidRequest),
			"Reject Oversize World Entity Spawn Placement");

		S2C_WORLD_ENTITY_SPAWN_RESULT result{};
		result.strPlacementId = request.strPlacementId;
		result.eResult = WORLD_ENTITY_SPAWN_RESULT::SPAWNED;
		result.iNetEntityId = 900u;
		CPacketWriter resultWriter;
		testRunner.Require(
			Write_Message(resultWriter, result),
			"Writer World Entity Spawn Result");
		CPacketReader resultReader{ resultWriter.Get_Buffer() };
		S2C_WORLD_ENTITY_SPAWN_RESULT decodedResult{};
		testRunner.Require(
			Read_Message(resultReader, decodedResult) &&
			decodedResult.eResult == WORLD_ENTITY_SPAWN_RESULT::SPAWNED &&
			decodedResult.iNetEntityId == 900u &&
			0u == resultReader.Get_RemainingSize(),
			"World Entity Spawn Result Round Trip");

		result.eResult = WORLD_ENTITY_SPAWN_RESULT::REJECTED;
		testRunner.Require(
			!Write_Message(resultWriter, result),
			"Reject Spawn Rejection With Entity ID");
		result.iNetEntityId = INVALID_NET_ENTITY_ID;
		CPacketWriter rejectionWriter;
		testRunner.Require(
			Write_Message(rejectionWriter, result),
			"Accept Explicit World Entity Spawn Rejection");

		result.eResult = WORLD_ENTITY_SPAWN_RESULT::ACTIVATED;
		CPacketWriter activationWriter;
		testRunner.Require(
			Write_Message(activationWriter, result),
			"Accept Spawn Group Activation Without Entity ID");
		CPacketReader activationReader{ activationWriter.Get_Buffer() };
		S2C_WORLD_ENTITY_SPAWN_RESULT activated{};
		testRunner.Require(
			Read_Message(activationReader, activated) &&
			activated.eResult == WORLD_ENTITY_SPAWN_RESULT::ACTIVATED &&
			activated.iNetEntityId == INVALID_NET_ENTITY_ID &&
			0u == activationReader.Get_RemainingSize(),
			"Spawn Group Activation Result Round Trip");
	}

	void Test_GameplayDataRevisionContract(TEST_RUNNER& testRunner)
	{
		const std::string upperHex =
			"0123456789ABCDEF0123456789ABCDEF"
			"0123456789ABCDEF0123456789ABCDEF";
		GameplayDataRevision parsed{};
		testRunner.Require(
			Try_Parse_GameplayDataRevision(upperHex, parsed) &&
			parsed.Is_Valid() &&
			Format_GameplayDataRevision(parsed) ==
				"0123456789abcdef0123456789abcdef"
				"0123456789abcdef0123456789abcdef",
			"Gameplay Revision Hex Parse And Canonical Format");

		GameplayDataRevision unchanged = Make_GameplayDataRevision(91u);
		const GameplayDataRevision sentinel = unchanged;
		testRunner.Require(
			!Try_Parse_GameplayDataRevision(std::string(64u, '0'), unchanged) &&
			unchanged == sentinel &&
			!Try_Parse_GameplayDataRevision("not-a-revision", unchanged) &&
			unchanged == sentinel,
			"Reject Zero Or Malformed Revision Hex Without Mutation");

		CPacketWriter writer;
		testRunner.Require(
			Write_GameplayDataRevision(writer, parsed) &&
			writer.Get_Buffer().size() == GAMEPLAY_DATA_REVISION_BYTES,
			"Gameplay Revision Fixed Width Wire");
		CPacketReader reader{ writer.Get_Buffer() };
		GameplayDataRevision decoded{};
		testRunner.Require(
			Read_GameplayDataRevision(reader, decoded) &&
			decoded == parsed && 0u == reader.Get_RemainingSize(),
			"Gameplay Revision Wire Round Trip");

		CPacketWriter zeroWriter;
		testRunner.Require(
			!Write_GameplayDataRevision(zeroWriter, GameplayDataRevision{}) &&
			zeroWriter.Get_Buffer().empty(),
			"Reject Reserved Zero Gameplay Revision On Write");
		std::vector<std::uint8_t> zeroBytes(GAMEPLAY_DATA_REVISION_BYTES, 0u);
		CPacketReader zeroReader{ zeroBytes };
		unchanged = sentinel;
		testRunner.Require(
			!Read_GameplayDataRevision(zeroReader, unchanged) &&
			unchanged == sentinel,
			"Reject Reserved Zero Gameplay Revision On Read Atomically");
		std::vector<std::uint8_t> truncated = writer.Get_Buffer();
		truncated.pop_back();
		CPacketReader truncatedReader{ truncated };
		unchanged = sentinel;
		testRunner.Require(
			!Read_GameplayDataRevision(truncatedReader, unchanged) &&
			unchanged == sentinel,
			"Reject Truncated Gameplay Revision Atomically");

		S2C_ENTER_ACCEPTED accepted{};
		accepted.iPlayerId = 1u;
		accepted.iNetEntityId = 100u;
		accepted.ActiveGameplayRevision = Make_GameplayDataRevision(1u);
		accepted.RequiredPinnedGameplayRevisions = {
			Make_GameplayDataRevision(2u), Make_GameplayDataRevision(2u) };
		CPacketWriter duplicateWriter;
		testRunner.Require(
			!Write_Message(duplicateWriter, accepted),
			"Reject Duplicate Required Pinned Revision On Write");

		CPacketWriter duplicateWire;
		duplicateWire.Write_U16(NETWORK_PROTOCOL_VERSION);
		duplicateWire.Write_U16(static_cast<std::uint16_t>(WORLD_ID::BERN));
		duplicateWire.Write_U32(1u);
		duplicateWire.Write_U32(100u);
		const GameplayDataRevision active = Make_GameplayDataRevision(1u);
		const GameplayDataRevision duplicate = Make_GameplayDataRevision(2u);
		const bool wroteActive =
			Write_GameplayDataRevision(duplicateWire, active);
		duplicateWire.Write_U8(2u);
		const bool wroteFirst =
			Write_GameplayDataRevision(duplicateWire, duplicate);
		const bool wroteSecond =
			Write_GameplayDataRevision(duplicateWire, duplicate);
		CPacketReader duplicateReader{ duplicateWire.Get_Buffer() };
		S2C_ENTER_ACCEPTED unchangedAccepted{};
		unchangedAccepted.iPlayerId = 77u;
		unchangedAccepted.ActiveGameplayRevision = sentinel;
		testRunner.Require(
			wroteActive && wroteFirst && wroteSecond &&
			!Read_Message(duplicateReader, unchangedAccepted) &&
			77u == unchangedAccepted.iPlayerId &&
			unchangedAccepted.ActiveGameplayRevision == sentinel,
			"Reject Duplicate Required Pinned Revision On Read Atomically");

		accepted.RequiredPinnedGameplayRevisions.assign(
			MAX_REQUIRED_PINNED_GAMEPLAY_REVISIONS + 1u,
			Make_GameplayDataRevision(3u));
		CPacketWriter oversizeListWriter;
		testRunner.Require(
			!Write_Message(oversizeListWriter, accepted),
			"Reject Oversize Required Pinned Revision List");
	}

	void Test_SessionDiagnosticReasonContract(TEST_RUNNER& testRunner)
	{
		struct EXPECTED_REASON
		{
			SESSION_DIAGNOSTIC_REASON eReason;
			const char* pszName;
		};

		constexpr std::array<EXPECTED_REASON, 46u> expectedReasons{
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_CONNECT_FAILED, "CLIENT_CONNECT_FAILED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_ENTER_SEND_FAILED, "CLIENT_ENTER_SEND_FAILED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_APPROVAL_TIMEOUT, "CLIENT_APPROVAL_TIMEOUT" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_CONNECTION_LOST, "CLIENT_CONNECTION_LOST" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_PEER_CLOSED, "CLIENT_PEER_CLOSED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_RECEIVE_ERROR, "CLIENT_RECEIVE_ERROR" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_SEND_ERROR, "CLIENT_SEND_ERROR" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_PARSER_OVERFLOW, "CLIENT_PARSER_OVERFLOW" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_INVALID_FRAME, "CLIENT_INVALID_FRAME" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_RAW_QUEUE_OVERFLOW, "CLIENT_RAW_QUEUE_OVERFLOW" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_EVENT_QUEUE_OVERFLOW, "CLIENT_EVENT_QUEUE_OVERFLOW" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_MESSAGE_DECODE_FAILED, "CLIENT_MESSAGE_DECODE_FAILED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_ENTRY_PRESENTATION_BASELINE_FAILED, "CLIENT_ENTRY_PRESENTATION_BASELINE_FAILED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_ENTRY_PRESENTATION_REVISION_FAILED, "CLIENT_ENTRY_PRESENTATION_REVISION_FAILED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_INVALID_SERVER_RESPONSE, "CLIENT_INVALID_SERVER_RESPONSE" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_EXPECTED_ROOM_FULL, "CLIENT_EXPECTED_ROOM_FULL" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_REPLICATION_FAILED, "CLIENT_REPLICATION_FAILED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_LOAD_FAILED, "CLIENT_LOAD_FAILED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_LOADING_START_FAILED, "CLIENT_LOADING_START_FAILED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_ACTIVATION_PROFILE_MISSING, "CLIENT_ACTIVATION_PROFILE_MISSING" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_ACTIVATION_LEVEL_CREATE_FAILED, "CLIENT_ACTIVATION_LEVEL_CREATE_FAILED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_ACTIVATION_PROFILE_FAILED, "CLIENT_ACTIVATION_PROFILE_FAILED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_ACTIVATION_CHANGE_LEVEL_FAILED, "CLIENT_ACTIVATION_CHANGE_LEVEL_FAILED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_IDENTITY_COMMIT_FAILED, "CLIENT_IDENTITY_COMMIT_FAILED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::CLIENT_WORLD_TRANSFER_FAILED, "CLIENT_WORLD_TRANSFER_FAILED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::SERVER_PEER_CLOSED, "SERVER_PEER_CLOSED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::SERVER_RECEIVE_ERROR, "SERVER_RECEIVE_ERROR" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::SERVER_INVALID_FRAME, "SERVER_INVALID_FRAME" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::SERVER_PARSER_OVERFLOW, "SERVER_PARSER_OVERFLOW" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::SERVER_SEND_ERROR_OR_TIMEOUT, "SERVER_SEND_ERROR_OR_TIMEOUT" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::SERVER_RELIABLE_OUTBOUND_OVERFLOW, "SERVER_RELIABLE_OUTBOUND_OVERFLOW" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_MESSAGE_DECODE_FAILED, "SERVER_CLIENT_MESSAGE_DECODE_FAILED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::SERVER_UNKNOWN_PACKET, "SERVER_UNKNOWN_PACKET" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_START_FAILED, "SERVER_SESSION_START_FAILED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED, "SERVER_SESSION_BIND_FAILED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_INGRESS_OVERFLOW, "SERVER_ROOM_INGRESS_OVERFLOW" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_COMMAND_VALIDATION_FAILED, "SERVER_CLIENT_COMMAND_VALIDATION_FAILED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED, "SERVER_JOIN_VALIDATION_FAILED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::SERVER_SPAWN_EXHAUSTED, "SERVER_SPAWN_EXHAUSTED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::SERVER_PROFILE_MISSING, "SERVER_PROFILE_MISSING" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::SERVER_NAVIGATION_FAILED, "SERVER_NAVIGATION_FAILED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_PREFLIGHT_FAILED, "SERVER_JOIN_PREFLIGHT_FAILED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::SERVER_INITIAL_SYNC_ENQUEUE_FAILED, "SERVER_INITIAL_SYNC_ENQUEUE_FAILED" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::SERVER_EXPECTED_ROOM_FULL, "SERVER_EXPECTED_ROOM_FULL" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::SERVER_APPLICATION_CLOSE, "SERVER_APPLICATION_CLOSE" },
			EXPECTED_REASON{ SESSION_DIAGNOSTIC_REASON::SERVER_SHUTDOWN, "SERVER_SHUTDOWN" }
		};

		static_assert(
			expectedReasons.size() ==
			static_cast<std::size_t>(SESSION_DIAGNOSTIC_REASON::END) - 1u,
			"Update the stable diagnostic reason table when appending a reason.");

		bool allReasonsAreKnown = true;
		bool allValuesAreContiguous = true;
		bool allNamesAreExact = true;
		bool allNamesAreUnique = true;
		for (std::size_t index = 0u; index < expectedReasons.size(); ++index)
		{
			const EXPECTED_REASON& expected = expectedReasons[index];
			allReasonsAreKnown = allReasonsAreKnown &&
				Is_Known_SessionDiagnosticReason(expected.eReason);
			allValuesAreContiguous = allValuesAreContiguous &&
				static_cast<std::size_t>(expected.eReason) == index + 1u;
			allNamesAreExact = allNamesAreExact &&
				To_SessionDiagnosticReasonName(expected.eReason) == expected.pszName &&
				To_SessionDiagnosticReasonName(expected.eReason) !=
					"UNKNOWN_SESSION_DIAGNOSTIC_REASON";

			for (std::size_t other = 0u; other < index; ++other)
			{
				allNamesAreUnique = allNamesAreUnique &&
					To_SessionDiagnosticReasonName(expected.eReason) !=
					To_SessionDiagnosticReasonName(expectedReasons[other].eReason);
			}
		}

		testRunner.Require(
			41u == NETWORK_PROTOCOL_VERSION,
			"Session Diagnostics Use Protocol Version 41");
		testRunner.Require(
			allReasonsAreKnown && allValuesAreContiguous,
			"Every Session Diagnostic Reason Is Known And Append Only");
		testRunner.Require(
			allNamesAreExact,
			"Every Session Diagnostic Reason Has Stable Exact Name");
		testRunner.Require(
			allNamesAreUnique,
			"Session Diagnostic Reason Names Are Unique");
		testRunner.Require(
			!Is_Known_SessionDiagnosticReason(SESSION_DIAGNOSTIC_REASON::NONE) &&
			To_SessionDiagnosticReasonName(SESSION_DIAGNOSTIC_REASON::NONE) == "NONE" &&
			!Is_Known_SessionDiagnosticReason(SESSION_DIAGNOSTIC_REASON::END) &&
			To_SessionDiagnosticReasonName(SESSION_DIAGNOSTIC_REASON::END) ==
				"UNKNOWN_SESSION_DIAGNOSTIC_REASON" &&
			!Is_Known_SessionDiagnosticReason(
				static_cast<SESSION_DIAGNOSTIC_REASON>(0xffffu)) &&
			To_SessionDiagnosticReasonName(
				static_cast<SESSION_DIAGNOSTIC_REASON>(0xffffu)) ==
				"UNKNOWN_SESSION_DIAGNOSTIC_REASON",
			"Reject None End And Invalid Session Diagnostic Reasons");
	}

	void Test_DataRevisionHotReloadProtocol(TEST_RUNNER& testRunner)
	{
		testRunner.Require(
			41u == NETWORK_PROTOCOL_VERSION,
			"Valtan Pattern Flow Contract Uses Protocol 41");
		const GameplayDataRevision base = Make_GameplayDataRevision(10u);
		const GameplayDataRevision candidate = Make_GameplayDataRevision(40u);
		const std::uint32_t required =
			static_cast<std::uint32_t>(GAMEPLAY_PRESENTATION_LANE::ANIMATION) |
			static_cast<std::uint32_t>(GAMEPLAY_PRESENTATION_LANE::EFFECT);

		testRunner.Require(
			Is_Known_Packet_Type(
				PACKET_TYPE::C2S_DATA_REVISION_PREPARE_REQUEST) &&
			Is_Known_Packet_Type(PACKET_TYPE::S2C_DATA_REVISION_PREPARE) &&
			Is_Known_Packet_Type(
				PACKET_TYPE::C2S_DATA_REVISION_PREPARE_RESPONSE) &&
			Is_Known_Packet_Type(PACKET_TYPE::S2C_DATA_REVISION_RESULT),
			"Data Revision Two Phase Packet Types Are Known");

		C2S_DATA_REVISION_PREPARE_REQUEST request{};
		request.iTransactionSequence = 7u;
		request.BaseRevision = base;
		request.CandidateRevision = candidate;
		request.iRequiredPresentationLaneMask = required;
		CPacketWriter requestWriter;
		CPacketReader requestReader{
			(Write_Message(requestWriter, request), requestWriter.Get_Buffer()) };
		C2S_DATA_REVISION_PREPARE_REQUEST decodedRequest{};
		testRunner.Require(
			Read_Message(requestReader, decodedRequest) &&
			0u == requestReader.Get_RemainingSize() &&
			decodedRequest.iTransactionSequence == request.iTransactionSequence &&
			decodedRequest.BaseRevision == base &&
			decodedRequest.CandidateRevision == candidate &&
			decodedRequest.iRequiredPresentationLaneMask == required,
			"Data Revision Prepare Request Round Trip");

		S2C_DATA_REVISION_PREPARE prepare{};
		prepare.iTransactionSequence = request.iTransactionSequence;
		prepare.BaseRevision = base;
		prepare.CandidateRevision = candidate;
		prepare.iRequiredPresentationLaneMask = required;
		CPacketWriter prepareWriter;
		CPacketReader prepareReader{
			(Write_Message(prepareWriter, prepare), prepareWriter.Get_Buffer()) };
		S2C_DATA_REVISION_PREPARE decodedPrepare{};
		testRunner.Require(
			Read_Message(prepareReader, decodedPrepare) &&
			0u == prepareReader.Get_RemainingSize() &&
			decodedPrepare.CandidateRevision == candidate,
			"Data Revision Server Prepare Round Trip");

		request.iRequiredPresentationLaneMask =
			GAMEPLAY_PRESENTATION_KNOWN_LANE_MASK | (1u << 31u);
		CPacketWriter unknownMaskWriter;
		testRunner.Require(
			!Write_Message(unknownMaskWriter, request),
			"Reject Unknown Required Presentation Lane On Write");
		std::vector<std::uint8_t> unknownMaskWire =
			prepareWriter.Get_Buffer();
		unknownMaskWire[unknownMaskWire.size() - 1u] |= 0x80u;
		CPacketReader unknownMaskReader{ unknownMaskWire };
		S2C_DATA_REVISION_PREPARE unchangedPrepare{};
		unchangedPrepare.iTransactionSequence = 99u;
		unchangedPrepare.CandidateRevision = base;
		testRunner.Require(
			!Read_Message(unknownMaskReader, unchangedPrepare) &&
			99u == unchangedPrepare.iTransactionSequence &&
			unchangedPrepare.CandidateRevision == base,
			"Reject Unknown Required Presentation Lane Atomically");

		request.iRequiredPresentationLaneMask = required;
		request.CandidateRevision = {};
		CPacketWriter zeroCandidateWriter;
		testRunner.Require(
			!Write_Message(zeroCandidateWriter, request),
			"Reject Zero Candidate Revision In Prepare");
		request.CandidateRevision = base;
		CPacketWriter sameRevisionWriter;
		testRunner.Require(
			!Write_Message(sameRevisionWriter, request),
			"Reject Prepare Whose Base Equals Candidate");

		C2S_DATA_REVISION_PREPARE_RESPONSE response{};
		response.iTransactionSequence = 7u;
		response.CandidateRevision = candidate;
		response.eStatus = DATA_REVISION_PREPARE_STATUS::READY;
		response.iRequiredPresentationLaneMask = required;
		response.iPreparedPresentationLaneMask = required;
		CPacketWriter readyWriter;
		CPacketReader readyReader{
			(Write_Message(readyWriter, response), readyWriter.Get_Buffer()) };
		C2S_DATA_REVISION_PREPARE_RESPONSE decodedResponse{};
		testRunner.Require(
			Read_Message(readyReader, decodedResponse) &&
			0u == readyReader.Get_RemainingSize() &&
			DATA_REVISION_PREPARE_STATUS::READY == decodedResponse.eStatus,
			"Data Revision READY Round Trip");

		response.eStatus = DATA_REVISION_PREPARE_STATUS::READY_DEGRADED;
		response.iFailedPresentationLaneMask =
			static_cast<std::uint32_t>(GAMEPLAY_PRESENTATION_LANE::CAMERA);
		response.strReason = "optional camera lane unavailable";
		CPacketWriter degradedWriter;
		CPacketReader degradedReader{
			(Write_Message(degradedWriter, response), degradedWriter.Get_Buffer()) };
		decodedResponse = {};
		testRunner.Require(
			Read_Message(degradedReader, decodedResponse) &&
			DATA_REVISION_PREPARE_STATUS::READY_DEGRADED ==
				decodedResponse.eStatus &&
			decodedResponse.iFailedPresentationLaneMask ==
				response.iFailedPresentationLaneMask,
			"Data Revision READY DEGRADED Round Trip");

		response.iFailedPresentationLaneMask =
			static_cast<std::uint32_t>(GAMEPLAY_PRESENTATION_LANE::EFFECT);
		CPacketWriter invalidDegradedWriter;
		testRunner.Require(
			!Write_Message(invalidDegradedWriter, response),
			"Reject READY DEGRADED With Failed Required Lane");

		response.eStatus = DATA_REVISION_PREPARE_STATUS::NACK;
		response.iPreparedPresentationLaneMask = 0u;
		response.iFailedPresentationLaneMask = required;
		response.strReason = "required lanes unavailable";
		CPacketWriter nackWriter;
		CPacketReader nackReader{
			(Write_Message(nackWriter, response), nackWriter.Get_Buffer()) };
		decodedResponse = {};
		testRunner.Require(
			Read_Message(nackReader, decodedResponse) &&
			DATA_REVISION_PREPARE_STATUS::NACK == decodedResponse.eStatus,
			"Data Revision NACK Round Trip");

		response.strReason.assign(
			MAX_DATA_REVISION_REASON_BYTES + 1u, 'x');
		CPacketWriter oversizeReasonWriter;
		testRunner.Require(
			!Write_Message(oversizeReasonWriter, response),
			"Reject Oversize Data Revision Response Reason");

		CPacketWriter oversizeReasonWire;
		oversizeReasonWire.Write_U32(7u);
		const bool wroteOversizeRevision = Write_GameplayDataRevision(
			oversizeReasonWire, candidate);
		oversizeReasonWire.Write_U8(static_cast<std::uint8_t>(
			DATA_REVISION_PREPARE_STATUS::NACK));
		oversizeReasonWire.Write_U32(required);
		oversizeReasonWire.Write_U32(0u);
		oversizeReasonWire.Write_U32(required);
		const std::string oversizeReason(
			MAX_DATA_REVISION_REASON_BYTES + 1u, 'x');
		const bool wroteOversizeReason = oversizeReasonWire.Write_String(
			oversizeReason, oversizeReason.size());
		CPacketReader oversizeReasonReader{
			oversizeReasonWire.Get_Buffer() };
		C2S_DATA_REVISION_PREPARE_RESPONSE unchangedOversize{};
		unchangedOversize.iTransactionSequence = 124u;
		unchangedOversize.CandidateRevision = base;
		testRunner.Require(
			wroteOversizeRevision && wroteOversizeReason &&
			!Read_Message(oversizeReasonReader, unchangedOversize) &&
			124u == unchangedOversize.iTransactionSequence &&
			unchangedOversize.CandidateRevision == base,
			"Reject Oversize Data Revision Reason On Read Atomically");

		std::vector<std::uint8_t> unknownStatusWire = nackWriter.Get_Buffer();
		unknownStatusWire[4u + GAMEPLAY_DATA_REVISION_BYTES] =
			static_cast<std::uint8_t>(DATA_REVISION_PREPARE_STATUS::END);
		CPacketReader unknownStatusReader{ unknownStatusWire };
		C2S_DATA_REVISION_PREPARE_RESPONSE unchangedResponse{};
		unchangedResponse.iTransactionSequence = 123u;
		unchangedResponse.CandidateRevision = base;
		testRunner.Require(
			!Read_Message(unknownStatusReader, unchangedResponse) &&
			123u == unchangedResponse.iTransactionSequence &&
			unchangedResponse.CandidateRevision == base,
			"Reject Unknown Data Revision Readiness Atomically");

		S2C_DATA_REVISION_RESULT result{};
		result.iTransactionSequence = 7u;
		result.CandidateRevision = candidate;
		result.ActiveRevision = candidate;
		result.eResult = DATA_REVISION_RESULT::COMMITTED;
		CPacketWriter commitWriter;
		CPacketReader commitReader{
			(Write_Message(commitWriter, result), commitWriter.Get_Buffer()) };
		S2C_DATA_REVISION_RESULT decodedResult{};
		testRunner.Require(
			Read_Message(commitReader, decodedResult) &&
			DATA_REVISION_RESULT::COMMITTED == decodedResult.eResult &&
			decodedResult.ActiveRevision == candidate,
			"Data Revision COMMIT Round Trip");

		result.ActiveRevision = base;
		result.eResult = DATA_REVISION_RESULT::ABORTED;
		result.strReason = "client readiness timeout";
		CPacketWriter abortWriter;
		CPacketReader abortReader{
			(Write_Message(abortWriter, result), abortWriter.Get_Buffer()) };
		decodedResult = {};
		testRunner.Require(
			Read_Message(abortReader, decodedResult) &&
			DATA_REVISION_RESULT::ABORTED == decodedResult.eResult &&
			decodedResult.ActiveRevision == base,
			"Data Revision ABORT Round Trip");

		std::vector<std::uint8_t> truncated = abortWriter.Get_Buffer();
		truncated.pop_back();
		CPacketReader truncatedReader{ truncated };
		S2C_DATA_REVISION_RESULT unchangedResult{};
		unchangedResult.iTransactionSequence = 456u;
		unchangedResult.ActiveRevision = candidate;
		testRunner.Require(
			!Read_Message(truncatedReader, unchangedResult) &&
			456u == unchangedResult.iTransactionSequence &&
			unchangedResult.ActiveRevision == candidate,
			"Reject Truncated Data Revision Result Atomically");
	}

	void Test_ValtanAuditionLifecycleProtocol(TEST_RUNNER& testRunner)
	{
		testRunner.Require(
			Is_Known_Packet_Type(
				PACKET_TYPE::S2C_VALTAN_AUDITION_LIFECYCLE),
			"Valtan Audition Lifecycle Packet Type Is Known");

		S2C_VALTAN_AUDITION_LIFECYCLE lifecycle{};
		lifecycle.iRequestSequence = 71u;
		lifecycle.iRoomAuditionEpoch = 9u;
		lifecycle.iPatternSequence = 14u;
		lifecycle.strPatternId = "HIGH_JUMP";
		lifecycle.eState = VALTAN_AUDITION_LIFECYCLE_STATE::PENDING;
		lifecycle.PinnedDefinitionRevision = Make_GameplayDataRevision(60u);
		S2C_VALTAN_AUDITION_LIFECYCLE pendingWithoutPattern = lifecycle;
		pendingWithoutPattern.iPatternSequence = 0u;
		CPacketWriter pendingWriter;
		CPacketReader pendingReader{
			(Write_Message(pendingWriter, pendingWithoutPattern),
				pendingWriter.Get_Buffer()) };
		S2C_VALTAN_AUDITION_LIFECYCLE decodedPending{};
		testRunner.Require(
			Read_Message(pendingReader, decodedPending) &&
			0u == decodedPending.iPatternSequence &&
			VALTAN_AUDITION_LIFECYCLE_STATE::PENDING == decodedPending.eState,
			"Valtan Pending Audition May Await Pattern Sequence Assignment");
		for (const VALTAN_AUDITION_LIFECYCLE_STATE state : {
			VALTAN_AUDITION_LIFECYCLE_STATE::PENDING,
			VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE,
			VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED })
		{
			lifecycle.eState = state;
			lifecycle.strReason.clear();
			CPacketWriter writer;
			CPacketReader reader{
				(Write_Message(writer, lifecycle), writer.Get_Buffer()) };
			S2C_VALTAN_AUDITION_LIFECYCLE decoded{};
			testRunner.Require(
				Read_Message(reader, decoded) &&
				0u == reader.Get_RemainingSize() &&
				decoded.eState == state &&
				decoded.iRequestSequence == lifecycle.iRequestSequence &&
				decoded.iRoomAuditionEpoch == lifecycle.iRoomAuditionEpoch &&
				decoded.iPatternSequence == lifecycle.iPatternSequence &&
				decoded.strPatternId == lifecycle.strPatternId &&
				decoded.PinnedDefinitionRevision ==
					lifecycle.PinnedDefinitionRevision,
				"Valtan Audition Lifecycle Edge Round Trip");
		}

		lifecycle.eState = VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED;
		lifecycle.strReason = "owner disconnected";
		CPacketWriter abortedWriter;
		CPacketReader abortedReader{
			(Write_Message(abortedWriter, lifecycle),
				abortedWriter.Get_Buffer()) };
		S2C_VALTAN_AUDITION_LIFECYCLE decodedAborted{};
		testRunner.Require(
			Read_Message(abortedReader, decodedAborted) &&
			VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED ==
				decodedAborted.eState &&
			decodedAborted.strReason == lifecycle.strReason,
			"Valtan Audition ABORTED Round Trip");

		lifecycle.PinnedDefinitionRevision = {};
		CPacketWriter zeroRevisionWriter;
		testRunner.Require(
			!Write_Message(zeroRevisionWriter, lifecycle),
			"Reject Valtan Lifecycle With Zero Pinned Revision");
		lifecycle.PinnedDefinitionRevision = Make_GameplayDataRevision(60u);
		lifecycle.strReason.assign(
			MAX_VALTAN_AUDITION_LIFECYCLE_REASON_BYTES + 1u, 'x');
		CPacketWriter oversizeReasonWriter;
		testRunner.Require(
			!Write_Message(oversizeReasonWriter, lifecycle),
			"Reject Oversize Valtan Lifecycle Abort Reason");
		lifecycle.eState = VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE;
		lifecycle.strReason = "owner disconnected";
		CPacketWriter activeWithReasonWriter;
		testRunner.Require(
			!Write_Message(activeWithReasonWriter, lifecycle),
			"Reject Non-Aborted Valtan Lifecycle Reason");
		lifecycle.strReason.clear();
		lifecycle.iPatternSequence = 0u;
		CPacketWriter activeWithoutPatternWriter;
		testRunner.Require(
			!Write_Message(activeWithoutPatternWriter, lifecycle),
			"Reject Active Valtan Lifecycle Without Pattern Sequence");
		lifecycle.iPatternSequence = 14u;

		std::vector<std::uint8_t> unknownStateWire =
			abortedWriter.Get_Buffer();
		const std::size_t stateOffset = 12u + sizeof(std::uint16_t) +
			lifecycle.strPatternId.size();
		unknownStateWire[stateOffset] = static_cast<std::uint8_t>(
			VALTAN_AUDITION_LIFECYCLE_STATE::END);
		CPacketReader unknownStateReader{ unknownStateWire };
		S2C_VALTAN_AUDITION_LIFECYCLE unchanged{};
		unchanged.iRequestSequence = 999u;
		unchanged.PinnedDefinitionRevision = Make_GameplayDataRevision(80u);
		testRunner.Require(
			!Read_Message(unknownStateReader, unchanged) &&
			999u == unchanged.iRequestSequence &&
			unchanged.PinnedDefinitionRevision ==
				Make_GameplayDataRevision(80u),
			"Reject Unknown Valtan Lifecycle State Atomically");
	}

	void Test_ValtanPatternFlowProtocol(TEST_RUNNER& testRunner)
	{
		testRunner.Require(
			Is_Known_Packet_Type(
				PACKET_TYPE::C2S_DEBUG_VALTAN_PATTERN_FLOW_START) &&
			Is_Known_Packet_Type(
				PACKET_TYPE::S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT) &&
			Is_Known_Packet_Type(
				PACKET_TYPE::
					C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT) &&
			Is_Known_Packet_Type(
				PACKET_TYPE::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE),
			"Valtan Pattern Flow Packet Types Are Known");

		const std::string flowRevision(
			VALTAN_PATTERN_FLOW_REVISION_HEX_BYTES, 'a');
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START start{};
		start.iRequestSequence = 701u;
		start.strBossPlacementId = "boss.valtan.center";
		start.strFlowId = "flow.valtan.boss-tool.default";
		start.strFlowRevision = flowRevision;
		start.strStartSlotId =
			"flow.valtan.boss-tool.default.slot.000001";
		start.iInterStepPursuitMs = 1000u;
		start.Slots = {
			{ "flow.valtan.boss-tool.default.slot.000001",
				"VALTAN_DASH_CHARGE" },
			{ "flow.valtan.boss-tool.default.slot.000002",
				"VALTAN_DASH_CHARGE" }
		};

		CPacketWriter startWriter;
		const bool wroteStart = Write_Message(startWriter, start);
		CPacketReader startReader{ startWriter.Get_Buffer() };
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START decodedStart{};
		testRunner.Require(
			wroteStart && Read_Message(startReader, decodedStart) &&
			0u == startReader.Get_RemainingSize() &&
			start.iRequestSequence == decodedStart.iRequestSequence &&
			start.strFlowRevision == decodedStart.strFlowRevision &&
			start.strStartSlotId == decodedStart.strStartSlotId &&
			2u == decodedStart.Slots.size() &&
			decodedStart.Slots[0].strPatternId ==
				decodedStart.Slots[1].strPatternId,
			"Valtan Pattern Flow Repeated Pattern Round Trip");

		C2S_DEBUG_VALTAN_PATTERN_FLOW_START duplicateSlot = start;
		duplicateSlot.Slots[1].strSlotId =
			duplicateSlot.Slots[0].strSlotId;
		CPacketWriter duplicateSlotWriter;
		CPacketReader duplicateSlotReader{
			(Write_Message(duplicateSlotWriter, duplicateSlot),
				duplicateSlotWriter.Get_Buffer()) };
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START decodedDuplicateSlot{};
		testRunner.Require(
			Read_Message(duplicateSlotReader, decodedDuplicateSlot) &&
			decodedDuplicateSlot.Slots[0].strSlotId ==
				decodedDuplicateSlot.Slots[1].strSlotId,
			"Valtan Pattern Flow Leaves Duplicate Slot Rejection To Server");

		C2S_DEBUG_VALTAN_PATTERN_FLOW_START maximum = start;
		maximum.Slots.clear();
		for (std::size_t index = 0u;
			index < MAX_VALTAN_PATTERN_FLOW_SLOTS; ++index)
		{
			VALTAN_PATTERN_FLOW_SLOT_WIRE slot{};
			slot.strSlotId = "flow.slot." + std::to_string(index + 1u);
			slot.strPatternId = "VALTAN_DASH_CHARGE";
			maximum.Slots.push_back(std::move(slot));
		}
		maximum.strStartSlotId = maximum.Slots.back().strSlotId;
		CPacketWriter maximumWriter;
		CPacketReader maximumReader{
			(Write_Message(maximumWriter, maximum),
				maximumWriter.Get_Buffer()) };
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START decodedMaximum{};
		testRunner.Require(
			Read_Message(maximumReader, decodedMaximum) &&
			MAX_VALTAN_PATTERN_FLOW_SLOTS == decodedMaximum.Slots.size(),
			"Valtan Pattern Flow 32 Slot Round Trip");

		C2S_DEBUG_VALTAN_PATTERN_FLOW_START invalidStart = start;
		invalidStart.Slots.clear();
		CPacketWriter invalidStartWriter;
		testRunner.Require(
			!Write_Message(invalidStartWriter, invalidStart),
			"Reject Empty Valtan Pattern Flow Start");
		invalidStart = maximum;
		invalidStart.Slots.push_back(
			{ "flow.slot.33", "VALTAN_DASH_CHARGE" });
		testRunner.Require(
			!Write_Message(invalidStartWriter, invalidStart),
			"Reject 33 Slot Valtan Pattern Flow Start");
		invalidStart = start;
		invalidStart.strFlowRevision.front() = 'A';
		testRunner.Require(
			!Write_Message(invalidStartWriter, invalidStart),
			"Reject Non-Lowercase Valtan Pattern Flow Revision");

		std::vector<std::uint8_t> oversizeCount =
			startWriter.Get_Buffer();
		const std::size_t slotCountOffset = sizeof(std::uint32_t) +
			(sizeof(std::uint16_t) + start.strBossPlacementId.size()) +
			(sizeof(std::uint16_t) + start.strFlowId.size()) +
			(sizeof(std::uint16_t) + start.strFlowRevision.size()) +
			(sizeof(std::uint16_t) + start.strStartSlotId.size()) +
			sizeof(std::uint32_t);
		if (slotCountOffset < oversizeCount.size())
		{
			oversizeCount[slotCountOffset] = static_cast<std::uint8_t>(
				MAX_VALTAN_PATTERN_FLOW_SLOTS + 1u);
		}
		CPacketReader oversizeCountReader{ oversizeCount };
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START unchangedStart{};
		unchangedStart.iRequestSequence = 999u;
		unchangedStart.strFlowId = "sentinel";
		testRunner.Require(
			!Read_Message(oversizeCountReader, unchangedStart) &&
			999u == unchangedStart.iRequestSequence &&
			"sentinel" == unchangedStart.strFlowId,
			"Reject Oversize Valtan Pattern Flow Slot Count Atomically");

		std::vector<std::uint8_t> truncatedStart =
			startWriter.Get_Buffer();
		truncatedStart.pop_back();
		CPacketReader truncatedStartReader{ truncatedStart };
		testRunner.Require(
			!Read_Message(truncatedStartReader, unchangedStart) &&
			999u == unchangedStart.iRequestSequence &&
			"sentinel" == unchangedStart.strFlowId,
			"Reject Truncated Valtan Pattern Flow Start Atomically");

		C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT stop{};
		stop.iControlSequence = 702u;
		stop.strFlowId = start.strFlowId;
		stop.iRoomFlowEpoch = 13u;
		CPacketWriter stopWriter;
		CPacketReader stopReader{
			(Write_Message(stopWriter, stop), stopWriter.Get_Buffer()) };
		C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT decodedStop{};
		testRunner.Require(
			Read_Message(stopReader, decodedStop) &&
			stop.iControlSequence == decodedStop.iControlSequence &&
			stop.strFlowId == decodedStop.strFlowId &&
			stop.iRoomFlowEpoch == decodedStop.iRoomFlowEpoch,
			"Valtan Pattern Flow Stop Identity Round Trip");
		stop.iRoomFlowEpoch = 0u;
		CPacketWriter invalidStopWriter;
		testRunner.Require(
			!Write_Message(invalidStopWriter, stop),
			"Reject Zero Epoch Valtan Pattern Flow Stop");

		S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT result{};
		result.iCommandSequence = start.iRequestSequence;
		result.eCommand = VALTAN_PATTERN_FLOW_COMMAND::START;
		result.eResult = VALTAN_PATTERN_FLOW_RESULT::QUEUED;
		result.strFlowId = start.strFlowId;
		result.strFlowRevision = flowRevision;
		result.iRoomFlowEpoch = 13u;
		result.PinnedDefinitionRevision = Make_GameplayDataRevision(70u);
		CPacketWriter resultWriter;
		CPacketReader resultReader{
			(Write_Message(resultWriter, result), resultWriter.Get_Buffer()) };
		S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT decodedResult{};
		testRunner.Require(
			Read_Message(resultReader, decodedResult) &&
			VALTAN_PATTERN_FLOW_RESULT::QUEUED == decodedResult.eResult &&
			flowRevision == decodedResult.strFlowRevision &&
			result.PinnedDefinitionRevision ==
				decodedResult.PinnedDefinitionRevision,
			"Valtan Pattern Flow Start Result Round Trip");

		S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT staleStop{};
		staleStop.iCommandSequence = 703u;
		staleStop.eCommand =
			VALTAN_PATTERN_FLOW_COMMAND::STOP_AFTER_CURRENT;
		staleStop.eResult =
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW;
		staleStop.strFlowId = start.strFlowId;
		staleStop.iRoomFlowEpoch = 12u;
		staleStop.strReason = "stale room flow epoch";
		CPacketWriter staleStopWriter;
		CPacketReader staleStopReader{
			(Write_Message(staleStopWriter, staleStop),
				staleStopWriter.Get_Buffer()) };
		S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT decodedStaleStop{};
		testRunner.Require(
			Read_Message(staleStopReader, decodedStaleStop) &&
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW ==
				decodedStaleStop.eResult &&
			decodedStaleStop.strFlowRevision.empty() &&
			"stale room flow epoch" == decodedStaleStop.strReason,
			"Valtan Pattern Flow Typed Stop Rejection Round Trip");

		std::vector<std::uint8_t> unknownResult =
			resultWriter.Get_Buffer();
		unknownResult[sizeof(std::uint32_t) + sizeof(std::uint8_t)] =
			static_cast<std::uint8_t>(VALTAN_PATTERN_FLOW_RESULT::END);
		CPacketReader unknownResultReader{ unknownResult };
		S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT unchangedResult{};
		unchangedResult.iCommandSequence = 998u;
		unchangedResult.strFlowId = "sentinel";
		testRunner.Require(
			!Read_Message(unknownResultReader, unchangedResult) &&
			998u == unchangedResult.iCommandSequence &&
			"sentinel" == unchangedResult.strFlowId,
			"Reject Unknown Valtan Pattern Flow Result Atomically");

		S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE lifecycle{};
		lifecycle.iRequestSequence = start.iRequestSequence;
		lifecycle.iRoomFlowEpoch = 13u;
		lifecycle.iPatternSequence = 41u;
		lifecycle.strBossPlacementId = start.strBossPlacementId;
		lifecycle.strFlowId = start.strFlowId;
		lifecycle.strFlowRevision = flowRevision;
		lifecycle.strStartSlotId = start.strStartSlotId;
		lifecycle.strCurrentSlotId = start.Slots[1].strSlotId;
		lifecycle.strCurrentPatternId = start.Slots[1].strPatternId;
		lifecycle.iCurrentSlotOrdinal = 2u;
		lifecycle.iSlotCount = 2u;
		lifecycle.eState = VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE;
		lifecycle.PinnedDefinitionRevision = Make_GameplayDataRevision(70u);
		CPacketWriter lifecycleWriter;
		CPacketReader lifecycleReader{
			(Write_Message(lifecycleWriter, lifecycle),
				lifecycleWriter.Get_Buffer()) };
		S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE decodedLifecycle{};
		testRunner.Require(
			Read_Message(lifecycleReader, decodedLifecycle) &&
			VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE ==
				decodedLifecycle.eState &&
			2u == decodedLifecycle.iCurrentSlotOrdinal &&
			start.Slots[1].strSlotId == decodedLifecycle.strCurrentSlotId &&
			start.Slots[1].strPatternId ==
				decodedLifecycle.strCurrentPatternId &&
			flowRevision == decodedLifecycle.strFlowRevision,
			"Valtan Pattern Flow Active Lifecycle Round Trip");

		S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE rejected = lifecycle;
		rejected.iRoomFlowEpoch = 0u;
		rejected.iPatternSequence = 0u;
		rejected.strCurrentSlotId.clear();
		rejected.strCurrentPatternId.clear();
		rejected.iCurrentSlotOrdinal = 0u;
		rejected.eState = VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::REJECTED;
		rejected.PinnedDefinitionRevision = {};
		rejected.strReason = "unknown pattern definition";
		CPacketWriter rejectedWriter;
		CPacketReader rejectedReader{
			(Write_Message(rejectedWriter, rejected),
				rejectedWriter.Get_Buffer()) };
		S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE decodedRejected{};
		testRunner.Require(
			Read_Message(rejectedReader, decodedRejected) &&
			VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::REJECTED ==
				decodedRejected.eState &&
			!decodedRejected.PinnedDefinitionRevision.Is_Valid() &&
			"unknown pattern definition" == decodedRejected.strReason,
			"Valtan Pattern Flow Rejected Lifecycle Round Trip");

		std::vector<std::uint8_t> unknownState =
			lifecycleWriter.Get_Buffer();
		const std::size_t lifecycleStateOffset =
			(sizeof(std::uint32_t) * 3u) +
			(sizeof(std::uint16_t) + lifecycle.strBossPlacementId.size()) +
			(sizeof(std::uint16_t) + lifecycle.strFlowId.size()) +
			(sizeof(std::uint16_t) + lifecycle.strFlowRevision.size()) +
			(sizeof(std::uint16_t) + lifecycle.strStartSlotId.size()) +
			(sizeof(std::uint16_t) + lifecycle.strCurrentSlotId.size()) +
			(sizeof(std::uint16_t) + lifecycle.strCurrentPatternId.size()) +
			(sizeof(std::uint16_t) * 2u);
		if (lifecycleStateOffset < unknownState.size())
		{
			unknownState[lifecycleStateOffset] = static_cast<std::uint8_t>(
				VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::END);
		}
		CPacketReader unknownStateReader{ unknownState };
		S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE unchangedLifecycle{};
		unchangedLifecycle.iRequestSequence = 997u;
		unchangedLifecycle.strFlowId = "sentinel";
		testRunner.Require(
			!Read_Message(unknownStateReader, unchangedLifecycle) &&
			997u == unchangedLifecycle.iRequestSequence &&
			"sentinel" == unchangedLifecycle.strFlowId,
			"Reject Unknown Valtan Pattern Flow State Atomically");

		std::vector<std::uint8_t> truncatedLifecycle =
			lifecycleWriter.Get_Buffer();
		truncatedLifecycle.pop_back();
		CPacketReader truncatedLifecycleReader{ truncatedLifecycle };
		testRunner.Require(
			!Read_Message(truncatedLifecycleReader, unchangedLifecycle) &&
			997u == unchangedLifecycle.iRequestSequence,
			"Reject Truncated Valtan Pattern Flow Lifecycle Atomically");
	}

	void Test_ValtanDecisionTraceProtocol(TEST_RUNNER& testRunner)
	{
		testRunner.Require(
			Is_Known_Packet_Type(
				PACKET_TYPE::C2S_VALTAN_DECISION_TRACE_QUERY) &&
			Is_Known_Packet_Type(
				PACKET_TYPE::S2C_VALTAN_DECISION_TRACE_RESPONSE),
			"Valtan Decision Trace Packet Types Are Known");

		C2S_VALTAN_DECISION_TRACE_QUERY query{};
		query.iRequestSequence = 91u;
		query.strBossPlacementId = "boss.valtan.center";
		query.iAfterTraceSequence = 7000u;
		CPacketWriter queryWriter;
		CPacketReader queryReader{
			(Write_Message(queryWriter, query), queryWriter.Get_Buffer()) };
		C2S_VALTAN_DECISION_TRACE_QUERY decodedQuery{};
		testRunner.Require(
			Read_Message(queryReader, decodedQuery) &&
			0u == queryReader.Get_RemainingSize() &&
			decodedQuery.iRequestSequence == query.iRequestSequence &&
			decodedQuery.strBossPlacementId == query.strBossPlacementId &&
			decodedQuery.iAfterTraceSequence == query.iAfterTraceSequence,
			"Valtan Decision Trace Query Round Trip");

		C2S_VALTAN_DECISION_TRACE_QUERY invalidQuery = query;
		invalidQuery.iRequestSequence = 0u;
		CPacketWriter invalidQueryWriter;
		testRunner.Require(
			!Write_Message(invalidQueryWriter, invalidQuery),
			"Reject Zero Valtan Decision Trace Query Sequence");
		invalidQuery = query;
		invalidQuery.strBossPlacementId.assign(
			MAX_STABLE_NETWORK_ID_BYTES + 1u, 'x');
		CPacketWriter overlongQueryWriter;
		testRunner.Require(
			!Write_Message(overlongQueryWriter, invalidQuery),
			"Reject Overlong Valtan Decision Trace Boss ID");

		std::vector<std::uint8_t> truncatedQuery = queryWriter.Get_Buffer();
		truncatedQuery.pop_back();
		CPacketReader truncatedQueryReader{ truncatedQuery };
		C2S_VALTAN_DECISION_TRACE_QUERY unchangedQuery{};
		unchangedQuery.iRequestSequence = 500u;
		unchangedQuery.strBossPlacementId = "sentinel";
		testRunner.Require(
			!Read_Message(truncatedQueryReader, unchangedQuery) &&
			500u == unchangedQuery.iRequestSequence &&
			"sentinel" == unchangedQuery.strBossPlacementId,
			"Reject Truncated Valtan Decision Trace Query Atomically");

		const S2C_VALTAN_DECISION_TRACE_RESPONSE response =
			Make_ValtanDecisionTraceResponse();
		CPacketWriter responseWriter;
		const bool wroteResponse = Write_Message(responseWriter, response);
		CPacketReader responseReader{ responseWriter.Get_Buffer() };
		S2C_VALTAN_DECISION_TRACE_RESPONSE decodedResponse{};
		testRunner.Require(
			wroteResponse && Read_Message(responseReader, decodedResponse) &&
			0u == responseReader.Get_RemainingSize() &&
			decodedResponse.DefinitionRevision ==
				response.DefinitionRevision &&
			decodedResponse.Trace.iTraceSequence == 7001u &&
			decodedResponse.Trace.iServerTick == 1440u &&
			decodedResponse.Trace.eSource ==
				VALTAN_DECISION_TRACE_SOURCE::WEIGHTED &&
			decodedResponse.Trace.eResult ==
				VALTAN_DECISION_TRACE_RESULT::SELECTED &&
			decodedResponse.Trace.strSelectedPatternId ==
				"VALTAN_WHIRLWIND" &&
			decodedResponse.Trace.iRawRandomInput ==
				0x1122334455667788ull &&
			decodedResponse.Trace.iRandomTicket == 42u &&
			2u == decodedResponse.Trace.Candidates.size() &&
			decodedResponse.Trace.Candidates.front().iExclusionMask ==
				VALTAN_DECISION_TRACE_EXCLUDE_COOLDOWN &&
			decodedResponse.Trace.Candidates.back().isSelected,
			"Valtan Decision Trace Response Round Trip");

		S2C_VALTAN_DECISION_TRACE_RESPONSE unchanged{};
		unchanged.iRequestSequence = 92u;
		unchanged.strBossPlacementId = "boss.valtan.center";
		unchanged.eResult = VALTAN_DECISION_TRACE_QUERY_RESULT::UNCHANGED;
		CPacketWriter unchangedWriter;
		CPacketReader unchangedReader{
			(Write_Message(unchangedWriter, unchanged),
				unchangedWriter.Get_Buffer()) };
		S2C_VALTAN_DECISION_TRACE_RESPONSE decodedUnchanged{};
		testRunner.Require(
			Read_Message(unchangedReader, decodedUnchanged) &&
			VALTAN_DECISION_TRACE_QUERY_RESULT::UNCHANGED ==
				decodedUnchanged.eResult &&
			!decodedUnchanged.DefinitionRevision.Is_Valid() &&
			0u == decodedUnchanged.Trace.iTraceSequence,
			"Valtan Decision Trace Unchanged Round Trip");

		unchanged.eResult =
			VALTAN_DECISION_TRACE_QUERY_RESULT::REJECTED_RELEASE_BUILD;
		CPacketWriter releaseRejectWriter;
		CPacketReader releaseRejectReader{
			(Write_Message(releaseRejectWriter, unchanged),
				releaseRejectWriter.Get_Buffer()) };
		S2C_VALTAN_DECISION_TRACE_RESPONSE decodedReleaseReject{};
		testRunner.Require(
			Read_Message(releaseRejectReader, decodedReleaseReject) &&
			VALTAN_DECISION_TRACE_QUERY_RESULT::REJECTED_RELEASE_BUILD ==
				decodedReleaseReject.eResult,
			"Release Policy Decision Trace Rejection Is Typed");

		S2C_VALTAN_DECISION_TRACE_RESPONSE invalidResponse = response;
		invalidResponse.Trace.Candidates.front().iExclusionMask |= 1u << 31u;
		CPacketWriter unknownMaskWriter;
		testRunner.Require(
			!Write_Message(unknownMaskWriter, invalidResponse),
			"Reject Unknown Valtan Decision Exclusion Bit");

		invalidResponse = response;
		invalidResponse.Trace.Candidates.back().strPatternId =
			invalidResponse.Trace.Candidates.front().strPatternId;
		CPacketWriter duplicateCandidateWriter;
		testRunner.Require(
			!Write_Message(duplicateCandidateWriter, invalidResponse),
			"Reject Duplicate Valtan Decision Candidate ID");

		invalidResponse = response;
		invalidResponse.Trace.Candidates.clear();
		invalidResponse.Trace.strSelectedPatternId.clear();
		invalidResponse.Trace.eResult =
			VALTAN_DECISION_TRACE_RESULT::NO_ELIGIBLE_PATTERN;
		invalidResponse.Trace.iTotalWeight = 0u;
		invalidResponse.Trace.iRandomTicket = 0u;
		for (std::size_t index = 0u;
			index <= MAX_VALTAN_DECISION_TRACE_CANDIDATES; ++index)
		{
			VALTAN_DECISION_TRACE_CANDIDATE_WIRE candidate{};
			candidate.strPatternId = "PATTERN_" + std::to_string(index);
			invalidResponse.Trace.Candidates.push_back(std::move(candidate));
		}
		CPacketWriter tooManyCandidatesWriter;
		testRunner.Require(
			!Write_Message(tooManyCandidatesWriter, invalidResponse),
			"Reject Oversize Valtan Decision Candidate Table");

		std::vector<std::uint8_t> unknownResult = responseWriter.Get_Buffer();
		const std::size_t resultOffset = sizeof(std::uint32_t) +
			sizeof(std::uint16_t) + response.strBossPlacementId.size();
		unknownResult[resultOffset] = static_cast<std::uint8_t>(
			VALTAN_DECISION_TRACE_QUERY_RESULT::END);
		CPacketReader unknownResultReader{ unknownResult };
		S2C_VALTAN_DECISION_TRACE_RESPONSE unchangedResponse{};
		unchangedResponse.iRequestSequence = 999u;
		unchangedResponse.strBossPlacementId = "sentinel";
		testRunner.Require(
			!Read_Message(unknownResultReader, unchangedResponse) &&
			999u == unchangedResponse.iRequestSequence &&
			"sentinel" == unchangedResponse.strBossPlacementId,
			"Reject Unknown Valtan Decision Trace Result Atomically");

		std::vector<std::uint8_t> truncatedResponse =
			responseWriter.Get_Buffer();
		truncatedResponse.pop_back();
		CPacketReader truncatedResponseReader{ truncatedResponse };
		unchangedResponse.iRequestSequence = 1000u;
		testRunner.Require(
			!Read_Message(truncatedResponseReader, unchangedResponse) &&
			1000u == unchangedResponse.iRequestSequence,
			"Reject Truncated Valtan Decision Trace Response Atomically");
	}
}

int main()
{
	TEST_RUNNER testRunner{};

	Test_EnterWorldRoundTrip(testRunner);
	Test_PlayerNicknameContract(testRunner);
	Test_PlayableCharacterRoster(testRunner);
	Test_InvalidPayloads(testRunner);

	Test_EnterAcceptedRoundTrip(testRunner);
	Test_InvalidEnterAcceptedPayloads(testRunner);
	Test_EnterRejectedRoundTrip(testRunner);

	Test_F32RoundTrip(testRunner);
	Test_PlayerSpawnedRoundTrip(testRunner);
	Test_InvalidPlayerSpawnedPayloads(testRunner);
	Test_WorldEntitySpawnedRoundTrip(testRunner);
	Test_WorldEntityDespawnedRoundTrip(testRunner);
	Test_CombatObjectLifecycleRoundTrip(testRunner);
	Test_PlayerDespawnedRoundTrip(testRunner);
	//Move, Snapshot roundtrip
	Test_MoveRoundTrip(testRunner);
	Test_UseSkillRoundTrip(testRunner);
	Test_ReleaseSkillRoundTrip(testRunner);
	Test_UpdateSkillAimRoundTrip(testRunner);
	Test_UseEstherSkillRoundTrip(testRunner);
	Test_RevivePlayerRoundTrip(testRunner);
	Test_CharacterClassChangeRoundTrip(testRunner);
	Test_WorldEntitySpawnCommandRoundTrip(testRunner);
	Test_WorldSnapshotRoundTrip(testRunner);
	Test_WorldDestructionProtocol(testRunner);
	Test_EncounterPropSyncProtocol(testRunner);
	Test_ValtanAuditionProtocol(testRunner);
	Test_SessionDiagnosticReasonContract(testRunner);
	Test_GameplayDataRevisionContract(testRunner);
	Test_DataRevisionHotReloadProtocol(testRunner);
	Test_ValtanAuditionLifecycleProtocol(testRunner);
	Test_ValtanPatternFlowProtocol(testRunner);
	Test_ValtanDecisionTraceProtocol(testRunner);
	Test_PartyInviteProtocol(testRunner);
	Test_ChatProtocol(testRunner);

	Test_StreamFraming(testRunner);

	std::cout
		<< "failures : "
		<< testRunner.iFailureCount
		<< '\n';

	return 0 == testRunner.iFailureCount ? 0 : 1;
}

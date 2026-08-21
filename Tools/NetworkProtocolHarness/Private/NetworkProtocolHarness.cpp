#include "Network/PacketFrame.h"
#include "Network/PacketMessages.h"
#include "Network/PacketReader.h"
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
			12 == payload.size(),
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
			decoded.fYawDegrees == source.fYawDegrees,
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
		source.iSkillId = 34060;
		source.fAimX = 151.f;
		source.fAimZ = -122.f;

		std::vector<std::uint8_t> payload;
		testRunner.Require(
			Build_UseSkillPayload(source, payload),
			"Writer Use Skill");
		testRunner.Require(
			16 == payload.size(),
			"Use Skill Payload Size");

		CPacketReader reader{ payload };
		C2S_USE_SKILL decoded{};
		testRunner.Require(
			Read_Message(reader, decoded),
			"Read Use Skill");
		testRunner.Require(
			decoded.iClientSequence == source.iClientSequence &&
			decoded.iSkillId == source.iSkillId &&
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
		source.CombatObjects.push_back(combatObject);

		std::vector<std::uint8_t> payload;

		testRunner.Require(
			Build_WorldSnapshotPayload(source, payload),
			"Writer World Snapshot");

		constexpr std::size_t snapshotHeaderBytes =
			4 + 2 + 2 + 2 + 1 + 1 + 1 + 4 + 4;
		constexpr std::size_t playerFixedBytes =
			4 + 1 + (4 * 4) + 1 + 1 + 1 + (4 * 8) + 1 + 1 + 1;
		constexpr std::size_t cooldownBytes = 4 + 4;
		/* Trailing 1 + 1 + 1 is iPhase, iBrokenArmorMask and the
		hasBossCombatState flag. The block after it is the boss combat
		snapshot the flag guards. */
		const std::size_t entityBytes =
			4 + 1 + 2 + entity.strPatternId.size() + 2 +
			entity.strActionId.size() + (4 * 4) + (4 * 5) + 1 + 1 + 1 +
			4 + 4 + 2 + (4 * 4) + 1;
		constexpr std::size_t bossCombatEventBytes =
			8 + 4 + 4 + 1 + 4;
		constexpr std::size_t combatObjectBytes =
			8 + 4 + (4 * 4);
		const std::size_t expectedPayloadBytes =
			snapshotHeaderBytes +
			(playerFixedBytes * 2) +
			cooldownBytes +
			entityBytes +
			bossCombatEventBytes +
			combatObjectBytes;

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
			decoded.iEstherGaugeMaximum == 1000,
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
			!decoded.Players[1].isCombatReady,
			"World Snapshot Players Round Trip");

		testRunner.Require(
			decoded.Entities[0].iNetEntityId == 900 &&
			decoded.Entities[0].eAction == WORLD_ENTITY_ACTION::PATTERN_ACTIVE &&
			decoded.Entities[0].strPatternId == entity.strPatternId &&
			decoded.Entities[0].strActionId == entity.strActionId &&
			decoded.Entities[0].iPatternSequence == 7u &&
			decoded.Entities[0].iPatternStageIndex == 1u &&
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
			unknownBossEventKindPayload.size() - combatObjectBytes - 5u] =
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
			30u == NETWORK_PROTOCOL_VERSION &&
			Is_Known_Packet_Type(
				PACKET_TYPE::S2C_WORLD_DESTRUCTION_FULL_SYNC) &&
			Is_Known_Packet_Type(
				PACKET_TYPE::S2C_WORLD_DESTRUCTION_DELTA),
			"World Destruction Packet Types At Protocol V30");

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
			std::vector<std::uint8_t> malformed = validDeltaPayload;
			malformed.pop_back();
			CPacketReader reader{ malformed };
			S2C_WORLD_DESTRUCTION_DELTA unchanged{};
			unchanged.iServerTick = 888u;
			testRunner.Require(!Read_Message(reader, unchanged) &&
				888u == unchanged.iServerTick,
				"Reject Truncated Destruction Delta Atomically");

			malformed = validDeltaPayload;
			malformed[76] = 65u;
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
			const std::array<VALTAN_AUDITION_OPERATION, 5u> barlessOperations{
				VALTAN_AUDITION_OPERATION::PLAY_WALL_ATTACK,
				VALTAN_AUDITION_OPERATION::SHOW_FINAL_ARENA,
				VALTAN_AUDITION_OPERATION::BREAK_EVERY_WALL,
				VALTAN_AUDITION_OPERATION::PLAY_ORDERED_1_67,
				VALTAN_AUDITION_OPERATION::STOP_ORDERED_1_67 };
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

	Test_StreamFraming(testRunner);

	std::cout
		<< "failures : "
		<< testRunner.iFailureCount
		<< '\n';

	return 0 == testRunner.iFailureCount ? 0 : 1;
}

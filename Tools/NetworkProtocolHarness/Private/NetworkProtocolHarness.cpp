#include "Network/PacketFrame.h"
#include "Network/PacketMessages.h"
#include "Network/PacketReader.h"
#include "Network/PacketStreamParser.h"
#include "Network/PacketWriter.h"

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
			"Accept Five Playable Character Classes");

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
		source.fPositionX = 151.25f;
		source.fPositionY = 22.97f;
		source.fPositionZ = -121.75f;
		source.fYawDegrees = 225.f;

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
			decoded.fPositionX == source.fPositionX &&
			decoded.fYawDegrees == source.fYawDegrees,
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

	void Test_WorldSnapshotRoundTrip(
		TEST_RUNNER& testRunner)
	{
		S2C_WORLD_SNAPSHOT source{};
		source.iServerTick = 30;

		PLAYER_SNAPSHOT first{};
		first.iNetEntityId = 100;
		first.fPositionX = 1.f;
		first.fPositionY = 0.f;
		first.fPositionZ = 2.f;
		first.fYawDegrees = 90.f;
		first.eLocomotionState =
			PLAYER_LOCOMOTION_STATE::MOVING;
		first.eAction = PLAYER_ACTION_STATE::SKILL;
		first.iSkillId = 34060;
		first.iActionStartTick = 25;
		first.iCurrentHp = 875;
		first.iMaximumHp = 1000;
		first.iCurrentResource = 80;
		first.iMaximumResource = 100;
		first.Cooldowns.push_back({ 34060, 330 });

		PLAYER_SNAPSHOT second{};
		second.iNetEntityId = 101;
		second.fPositionX = 3.f;
		second.fPositionY = 0.f;
		second.fPositionZ = 4.f;
		second.fYawDegrees = 180.f;
		second.eLocomotionState =
			PLAYER_LOCOMOTION_STATE::IDLE;

		source.Players.push_back(first);
		source.Players.push_back(second);

		WORLD_ENTITY_SNAPSHOT entity{};
		entity.iNetEntityId = 900;
		entity.eAction = WORLD_ENTITY_ACTION::CHASE;
		entity.fPositionX = 150.f;
		entity.fPositionY = 22.97f;
		entity.fPositionZ = -122.f;
		entity.fYawDegrees = 225.f;
		entity.iActionStartTick = 20;
		entity.iCurrentHp = 9500;
		entity.iMaximumHp = 10000;
		entity.iPhase = 1;
		source.Entities.push_back(entity);

		std::vector<std::uint8_t> payload;

		testRunner.Require(
			Build_WorldSnapshotPayload(source, payload),
			"Writer World Snapshot");

		testRunner.Require(
			148 == payload.size(),
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
			decoded.Entities.size() == 1,
			"World Snapshot Header Round Trip");

		testRunner.Require(
			decoded.Players[0].iNetEntityId == 100 &&
			decoded.Players[0].fPositionX == 1.f &&
			decoded.Players[0].fPositionZ == 2.f &&
			decoded.Players[0].eLocomotionState ==
			PLAYER_LOCOMOTION_STATE::MOVING &&
			decoded.Players[0].eAction == PLAYER_ACTION_STATE::SKILL &&
			decoded.Players[0].iSkillId == 34060 &&
			decoded.Players[0].iActionStartTick == 25 &&
			decoded.Players[0].iCurrentHp == 875 &&
			decoded.Players[0].Cooldowns.size() == 1 &&
			decoded.Players[0].Cooldowns[0].iCooldownEndTick == 330 &&
			decoded.Players[1].iNetEntityId == 101 &&
			decoded.Players[1].fPositionX == 3.f &&
			decoded.Players[1].fPositionZ == 4.f &&
			decoded.Players[1].eLocomotionState ==
			PLAYER_LOCOMOTION_STATE::IDLE,
			"World Snapshot Players Round Trip");

		testRunner.Require(
			decoded.Entities[0].iNetEntityId == 900 &&
			decoded.Entities[0].eAction == WORLD_ENTITY_ACTION::CHASE &&
			decoded.Entities[0].fPositionX == 150.f &&
			decoded.Entities[0].iCurrentHp == 9500 &&
			decoded.Entities[0].iMaximumHp == 10000 &&
			decoded.Entities[0].iPhase == 1,
			"World Snapshot Entities Round Trip");

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
}

int main()
{
	TEST_RUNNER testRunner{};

	Test_EnterWorldRoundTrip(testRunner);
	Test_PlayableCharacterRoster(testRunner);
	Test_InvalidPayloads(testRunner);

	Test_EnterAcceptedRoundTrip(testRunner);
	Test_InvalidEnterAcceptedPayloads(testRunner);

	Test_F32RoundTrip(testRunner);
	Test_PlayerSpawnedRoundTrip(testRunner);
	Test_InvalidPlayerSpawnedPayloads(testRunner);
	Test_WorldEntitySpawnedRoundTrip(testRunner);
	Test_PlayerDespawnedRoundTrip(testRunner);
	//Move, Snapshot roundtrip
	Test_MoveRoundTrip(testRunner);
	Test_UseSkillRoundTrip(testRunner);
	Test_WorldSnapshotRoundTrip(testRunner);

	Test_StreamFraming(testRunner);

	std::cout
		<< "failures : "
		<< testRunner.iFailureCount
		<< '\n';

	return 0 == testRunner.iFailureCount ? 0 : 1;
}

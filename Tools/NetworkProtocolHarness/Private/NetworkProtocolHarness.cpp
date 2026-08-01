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

	//월드 진입에 대한 테스트
	void Test_EnterWorldRoundTrip(
		TEST_RUNNER& testRunner)
	{
		C2S_ENTER_WORLD source{};

		source.eCharacterClass =
			CHARACTER_CLASS_ID::LANCE_MASTER;

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
			8 == payload.size(),
			"Enter Accepted Payload Size");

		CPacketReader reader{ payload };

		S2C_ENTER_ACCEPTED decoded{};

		testRunner.Require(
			Read_Message(reader, decoded),
			"Read Enter Accepted");

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
			CHARACTER_CLASS_ID::LANCE_MASTER;
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
			0x00,
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

		const std::vector<std::uint8_t> invalidClass
		{
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
}

int main()
{
	TEST_RUNNER testRunner{};

	Test_EnterWorldRoundTrip(testRunner);
	Test_InvalidPayloads(testRunner);

	Test_EnterAcceptedRoundTrip(testRunner);
	Test_InvalidEnterAcceptedPayloads(testRunner);

	Test_F32RoundTrip(testRunner);
	Test_PlayerSpawnedRoundTrip(testRunner);
	Test_InvalidPlayerSpawnedPayloads(testRunner);
	Test_PlayerDespawnedRoundTrip(testRunner);

	Test_StreamFraming(testRunner);

	std::cout
		<< "failures : "
		<< testRunner.iFailureCount
		<< '\n';

	return 0 == testRunner.iFailureCount ? 0 : 1;
}

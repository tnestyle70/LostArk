#pragma once

// wire 패킷 타입과 알려진 타입 검사, header/frame/누적 buffer의 byte 상한을 공유한다.
#include "Network/PacketType.h"

// header의 전체 길이와 소유 payload 원소의 고정 너비를 정의한다.
#include <cstdint>
// 송신 payload와 수신 header를 함수 호출 동안만 차용한다.
#include <span>
// 분리한 payload와 송신 frame 전체의 byte 복사본을 소유한다.
#include <vector>

namespace LostArk::Shared
{
	//packet의 전체 사이즈 기준으로 packet type과 total size에 대한 정보를 가지고 있다.
	//tcp를 경계가 없기 때문에, 이런 식으로 header에 정보를 담아서 읽어야 한다.
	// TCP에는 메시지 경계가 없으므로 [U32 전체 길이][U16 타입]의 6바이트로 경계를 복원한다.
	// 전체 길이는 header와 payload를 포함한다. 이 C++ 구조체의 sizeof를 그대로 전송하지 않는다.
	struct PACKET_HEADER
	{
		std::uint32_t iTotalSize = {};

		PACKET_TYPE ePacketType = PACKET_TYPE::INVALID;
	};

	//packet header가 tcp 사이의 경계를 나눠주는 거는 알겠는데, 
	//frame이 하는 역할은 뭐지? 이 구조체의 역할이 뭐야?
	//header 즉 tcp의 packet을 구분하는 정보와 frame, 즉 packet의 type과 실질적인 payload
	//를 나눠서 담는 역할을 하는 거 맞나?
	// parser가 완성된 한 메시지를 분리한 결과다. 타입과 payload 복사본을 소유하므로
	// parser의 내부 vector가 정리되거나 재할당되어도 상위 메시지 해석/큐 전달에 사용할 수 있다.
	struct PACKET_FRAME
	{
		PACKET_TYPE ePacketType = PACKET_TYPE::INVALID;

		std::vector<std::uint8_t> Payload;
	};

	// 송신 경로가 이미 직렬화한 payload에 header를 붙인다. 타입/크기 오류는 기존 frameBytes를 보존한다.
	// 입력 span은 호출 중 유효해야 한다. 반환한 vector는 입력과 수명이 독립적이며 실제 socket 전송은 수행하지 않는다.
	bool Build_Packet_Frame(
		PACKET_TYPE packetType, std::span<const std::uint8_t> payload, std::vector<std::uint8_t>& frameBytes);

	// parser가 길이/타입만 확인한다. 성공해도 전체 payload가 도착했다는 뜻은 아니다. false면 header를 보존한다.
	// 입력에 뒤따르는 payload를 복사하거나 소비 위치를 저장하지 않는다. StreamParser가 승인한 iTotalSize만큼 도착했는지 별도로 검사한다.
	bool Read_Packet_Header(std::span<const std::uint8_t> bytes, PACKET_HEADER& header);
}
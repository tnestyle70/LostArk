#pragma once

// 완성 frame의 소유 출력, header 검사 함수와 누적 byte 상한을 함께 사용한다.
#include "Network/PacketFrame.h"

// buffer 크기와 읽기 offset을 size_t로 보관하고 Compact에서 iterator 거리로 변환한다.
#include <cstddef>
// TCP에서 받은 byte를 uint8_t 원소로 보관한다.
#include <cstdint>
// recv의 임시 배열을 Append 입력으로 차용한다. 함수가 반환한 뒤 원본 배열을 재사용할 수 있다.
#include <span>
// 부분 frame과 뒤따르는 여러 frame을 연결별 누적 배열 하나에 소유한다.
#include <vector>

namespace LostArk::Shared
{
	//왜 packet parser의 결과가 need mode data, frame ready, invalid frame으로 나올까?
	// TCP 수신의 정상 미완성, 메시지 하나 완성, 잘못된 header를 구분한다.
	// INVALID_FRAME은 내부 누적 bytes를 버린 상태이며 연결 종료 판단은 수신 루프가 맡는다.
	enum class PACKET_PARSE_RESULT
	{
		NEED_MORE_DATA,
		FRAME_READY,
		INVALID_FRAME
	};

	// Client NetworkManager와 Server ClientSession의 수신 루프가 연결별로 소유한다.
	// recv가 나눠 주거나 합쳐 준 바이트를 누적한다. 내부 동기화가 없어 같은 parser의 동시 변경은 허용하지 않는다.
	class CPacketStreamParser final
	{
	public:
		// 소비한 앞부분을 정리한 뒤 입력을 복사한다. 상한 초과면 새 bytes를 붙이지 않으며 미소비 데이터는 유지한다.
		bool Append(std::span<const std::uint8_t> bytes);

		// 완성된 frame 한 개만 소비한다. NEED_MORE_DATA/INVALID_FRAME에서는 호출자의 기존 frame은 보존한다.
		PACKET_PARSE_RESULT Try_Pop(PACKET_FRAME& frame);

		// 연결 종료/재시작이나 잘못된 header 때 누적 내용과 offset을 비운다. vector capacity 해제를 보장하지 않는다.
		void Reset();

		// Try_Pop이 다음 frame의 도착 여부를 판단할 미소비 byte 수를 읽는다.
		// 내부 offset이 범위를 벗어나면 underflow 대신 0을 반환한다.
		std::size_t Get_BufferedByteCount() const;

	private:
		// Append가 새 수신 bytes를 붙이기 전에 호출한다. 소비한 prefix만 제거하고 미소비 frame 순서를 유지하며 offset을 0으로 돌린다.
		void Compact();

	private:
		// [0, readOffset)은 소비된 영역, [readOffset, size)는 다음 판정을 위해 기억할 미소비 영역이다.
		std::vector<std::uint8_t> m_Buffer;
		// 다음 header가 시작하는 buffer 내부 index다. FRAME_READY 때만 frame 전체 크기만큼 전진하고 Reset/Compact가 0으로 돌린다.
		std::size_t m_iReadOffset = {};
	};
}
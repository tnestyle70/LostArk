#pragma once

// 문자열 byte 길이 상한을 std::size_t로 받는다.
#include <cstddef>
// 직렬화할 정수의 고정 너비와 저장 byte 형식을 선언한다.
#include <cstdint>
// 메시지가 소유한 문자열을 호출 동안만 차용해 길이와 본문을 기록한다.
#include <string_view>
// payload 또는 frame의 직렬화 결과를 writer가 연속 byte 배열로 소유한다.
#include <vector>
//packetwriter에 raw bytes 기록 기능 추가, header 뒤에 완성된 payload를 붙이기 위해서 필요하다
// Write_Bytes는 std::span으로 받은 바이트를 길이 prefix 없이 내부 vector 끝에 복사한다.
#include <span>

namespace LostArk::Shared
{
	// Write_Message가 필드 순서대로 만든 payload 또는 frame 바이트를 vector에 소유한다.
	// 내부 잠금은 없으며 각 송신 직렬화가 자기 writer를 사용한다. 메모리 할당 예외를 bool 실패로 바꾸지 않는다.
	class CPacketWriter final
	{
	public:
		// U8/U16/U32는 각각 1/2/4바이트다. 여러 바이트의 정수는 낮은 바이트부터 기록한다.
		void Write_U8(std::uint8_t value);
		void Write_U16(std::uint16_t value);
		void Write_U32(std::uint32_t value);
		//player spawn과 이후 worldsnapshot에서 위치와 회전을 기록하기 위해서 필요하다.
		// 수치 반올림이나 문자열 변환 없이 float의 비트를 보존한다. 값의 유효성은 메시지 검증 책임이다.
		void Write_F32(float value);

		// 길이 prefix 없이 원본 바이트를 복사한다. 입력 범위는 내부 buffer와 겹치지 않아야 한다.
		void Write_Bytes(std::span<const std::uint8_t> bytes);

		// 두 길이 상한을 먼저 검사한다. false면 기존 buffer를 유지하고 성공하면 U16 길이와 본문을 붙인다.
		// 앞선 다른 필드까지 포함하는 메시지 전체 rollback은 상위 코드가 임시 writer로 처리한다.
		// 입력 view는 기록 중 재할당될 수 있는 이 writer의 buffer를 가리키지 않아야 한다. 메시지 문자열의 수명은 호출자가 유지한다.
		bool Write_String(std::string_view value, std::size_t maxByteLength);

		// writer가 소유한 vector를 차용한다. 이후 쓰기로 data 포인터가 바뀔 수 있고 writer보다 오래 보관할 수 없다.
		const std::vector<std::uint8_t>& Get_Buffer() const
		{
			return m_Buffer;
		}

	private:
		// 각 Write_*가 뒤에 이어 붙이는 저장소다. 전체 frame 상한/메시지 필드 유효성은 상위 serializer와 Build_Packet_Frame이 판정한다.
		std::vector<std::uint8_t> m_Buffer;
	};
}
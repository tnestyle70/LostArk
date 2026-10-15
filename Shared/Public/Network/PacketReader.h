#pragma once

// 읽은 byte 수와 문자열 상한을 std::size_t로 표현한다. wire에 size_t 자체를 기록하지 않는다.
#include <cstddef>
// wire 필드의 정확한 8/16/32비트 너비를 선언한다.
#include <cstdint>
// 수신 payload의 연속 byte 범위를 소유하지 않고 읽는다.
#include <span>
// 길이 prefix 뒤의 문자열을 호출자가 소유하는 std::string으로 복사한다.
#include <string>

//packetreader에서 수행해야 하는 동작 - Client로부터 온 packet을 8bytes 단위로
//읽어서 buffer에 어디까지 읽었는지를 담는다.
// Read_U8/U16/U32는 각각 1/2/4바이트를 읽는다. m_iOffset에는 소비한 바이트 수를 보관한다.
namespace LostArk::Shared
{
	//8bytes 단위로 packet을 끊어서 읽는다.
	// Client/Server의 Read_Message가 수신 payload를 필드 순서대로 읽는 도구다.
	// U8/U16/U32는 각각 1/2/4바이트를 소비한다. 고정 8바이트 단위로 읽는 것이 아니다.
	// 문자열 본문을 제외한 다중 바이트 수치는 writer와 같은 little-endian 순서를 사용한다.
	// 읽기 대상과 offset을 하나의 해석 흐름이 사용한다. 내부 잠금이 없어 같은 Reader의 동시 읽기/변경은 호출자가 막아야 한다.
	class CPacketReader final
	{
	public:
		// buffer는 차용한다. Reader를 사용하는 동안 원본 메모리가 살아 있고 재할당되지 않아야 한다.
		explicit CPacketReader(std::span<const std::uint8_t> buffer);

	public:
		// 수치 필드는 크기 부족 시 출력과 offset을 모두 보존한다. 성공하면 그 필드 크기만큼 전진한다.
		bool Read_U8(std::uint8_t& value);
		bool Read_U16(std::uint16_t& value);
		bool Read_U32(std::uint32_t& value);
		//Writer가 기록한 위치와 회전 float를 복원하기 위해서 필요
		// 32비트 패턴을 float로 복원한다. NaN/무한대와 위치 범위는 해당 메시지 검증자가 검사한다.
		bool Read_F32(float& value);

		// 문자열은 [U16 바이트 길이][그 길이의 본문]이다. 길이 prefix를 읽은 뒤 실패하면 offset은 복구하지 않는다.
		// false 경로에서 출력 string은 보존한다. 문자 수/UTF-8/ID 문법 검사는 이 계층의 역할이 아니다.
		bool Read_String(std::string& value, std::size_t maxByteLength);

		// Read_Message 이후 payload에 불필요한 바이트가 남았는지 상위 수신자가 검사할 때 사용한다.
		std::size_t Get_RemainingSize() const;

	private:
		// 각 Read_*가 필드에 접근하기 전에 남은 범위를 검사한다. 이 검사 자체는 출력이나 offset을 변경하지 않는다.
		bool Can_Read(std::size_t byteCount) const;

	private:
		//span은 데이터를 소유하지 않는 연속 메모리 보기이다.
		//개념적으로 다음 두 값을 묶은 것이다.
		//const uint8_t* data, size_t size
		//원본 vector가 span보다 오래 살아있어야 한다. 
		// 수신 frame이 메모리를 소유하고 Reader는 범위와 현재 byte offset만 보관한다.
		std::span<const std::uint8_t> m_Buffer;
		//offset 기준으로 읽는다.
		// m_iOffset은 원본 버퍼 시작부터 이미 소비한 바이트 수이며 다음 읽기의 시작 인덱스다.
		std::size_t m_iOffset = {};
	};
}
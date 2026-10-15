#pragma once

// 공통 namespace·byte/크기 타입과 vector·복사 가능 타입 검사를 읽는다. reader는 GPU 자원을 만들지 않는다.
#include "Engine_Defines.h"

// 파일 진입점이 OS 경로를 문자열 변환 없이 받도록 filesystem::path를 사용한다.
#include <filesystem>

NS_BEGIN(Engine)

// WINT reader들이 호출 동안 빌린 연속 바이트 구간을 앞에서부터 소비한다. 메모리를 소유하거나 파일 수명을 연장하지 않는다.
// 생성자는 포인터 유효성을 검사하지 않으므로 호출자가 크기만큼 읽을 수 있는 저장소를 유지해야 한다.
class CBinaryReader final
{
public:
	CBinaryReader(const void* pData, size_t size);

	// 디스크 구조체를 값으로 복사한다. endian/정렬 변환은 하지 않으며 ReadBytes의 범위 오류는 예외로 전달된다.
	template <typename T> T Read()
	{
		static_assert(is_trivially_copyable_v<T>, "CBinaryReader only accepts POD values.");

		T value{};
		ReadBytes(&value, sizeof(T));
		return value;
	}

	// 호출자가 소유한 비중첩 목적지에 byteCount 바이트를 복사하고 커서를 진행한다. 목적지 크기와 저장소 수명은 호출자 책임이다.
	void ReadBytes(void* pDestination, size_t byteCount);
	// reader가 해석하지 않을 필드를 건너뛴다. 남은 범위를 넘으면 예외를 던지고 커서는 유지한다.
	void Skip(size_t byteCount);

	// 현재 커서의 차용 포인터다. 다음 Read/Skip은 커서만 옮기고 원본 바이트는 변경하지 않는다.
	const uint8_t* Peek() const
	{
		return m_pCursor;
	}

	// 동일 차용 저장소의 끝에서 현재 커서를 빼 남은 byte 수를 반환한다. 포맷 reader가 길이와 배열 범위를 검증할 때 사용한다.
	size_t Remaining() const
	{
		return static_cast<size_t>(m_pEnd - m_pCursor);
	}

	// 출력을 먼저 비운다. 열기/빈 파일 실패는 빈 출력, read 실패는 크기가 잡힌 부분 바이트가 남을 수 있다.
	static bool_t LoadFile(const filesystem::path& path, vector<uint8_t>& outBytes);

private:
	// 동일 저장소 안의 현재 위치와 끝이다. 한 reader의 커서에 대한 동시 접근을 동기화하지 않는다.
	const uint8_t* m_pCursor = {nullptr};
	const uint8_t* m_pEnd = {nullptr};
};

NS_END

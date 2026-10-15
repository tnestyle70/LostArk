#pragma once

// revision 32 byte를 동적 할당 없이 값 안에 소유한다.
#include <array>
// revision byte 수와 16진 문자열 길이를 size_t로 선언한다.
#include <cstddef>
// revision 원소와 bootstrap 형식/상한의 정수 너비를 고정한다.
#include <cstdint>
// 로그와 게시 문서 연결에 사용할 64자리 소문자 표현을 소유 반환한다.
#include <string>
// loader가 읽은 16진 문자열을 파싱 호출 동안만 차용한다.
#include <string_view>

namespace LostArk::Shared
{
	class CPacketReader;
	class CPacketWriter;

	inline constexpr std::size_t GAMEPLAY_DATA_REVISION_BYTES = 32u;
	inline constexpr std::size_t GAMEPLAY_DATA_REVISION_HEX_BYTES = GAMEPLAY_DATA_REVISION_BYTES * 2u;

	// publisher·Server loader·Client admission이 함께 읽는 게시 형식 버전과 입력 크기 상한이다.
	// 콘텐츠 revision과 형식 version은 별개다. 이 상수를 선언한 것만으로 모든 파일의 내용이 검증되지는 않는다.
	//이 bootstrap format version을 업데이트해야 하는 이유가 뭘까? 
	// 행의 필드 구성·해석 규칙을 바꿀 때 구형 reader가 새 형식을 잘못 읽지 않도록 버전을 구분한다.
	// Server Catalog와 Client admission은 파일 헤더의 버전이 이 값과 정확히 같아야 수락한다.
	// 수치만 튜닝한 변경과는 다르며, 형식을 올릴 때는 게시 도구의 별도 버전과 reader를 함께 대조해야 한다.
	inline constexpr std::uint32_t GAMEPLAY_BOOTSTRAP_FORMAT_VERSION = 38u;
	inline constexpr std::uint32_t GAMEPLAY_BOOTSTRAP_MAX_ROWS = 131072u;
	inline constexpr std::uint32_t GAMEPLAY_BOOTSTRAP_MAX_BYTES = 67108864u;

	// 불변 gameplay/presentation 묶음의 SHA-256 콘텐츠 식별값 32바이트를 소유한다.
	// 전부 0이면 revision 없음이다. 이 타입의 Is_Valid는 0 여부만 검사하며 SHA-256을 계산하거나 파일 내용과 대조하지 않는다.
	struct GameplayDataRevision final
	{
		std::array<std::uint8_t, GAMEPLAY_DATA_REVISION_BYTES> Bytes{};

		[[nodiscard]] bool Is_Valid() const noexcept;

		// 32 byte의 값 비교다. 저장된 파일의 hash를 다시 계산하거나 현재 활성 세대인지 확인하지 않는다.
		friend bool operator==(const GameplayDataRevision&, const GameplayDataRevision&) = default;
	};

	// loader가 읽은 64자리 16진 문자열을 해석한다. 대소문자를 허용하되 공백/prefix는 허용하지 않는다.
	// 잘못된 문자·길이·전부 0인 입력이면 false이며 출력 revision은 기존 값 그대로다.
	[[nodiscard]] bool Try_Parse_GameplayDataRevision(std::string_view hex, GameplayDataRevision& revision) noexcept;

	// 로그·데이터 연결에 사용할 소문자 64자리 문자열을 만든다. 전부 0인 값은 빈 문자열로 반환한다.
	[[nodiscard]] std::string Format_GameplayDataRevision(const GameplayDataRevision& revision);

	// PacketMessages가 길이 prefix 없이 32바이트를 쓴다. invalid revision의 false는 writer를 변경하지 않는다.
	// Write_Bytes의 메모리 할당 예외는 이 bool 실패 계약으로 처리하지 않는다.
	[[nodiscard]] bool Write_GameplayDataRevision(CPacketWriter& writer, const GameplayDataRevision& revision);
	// 임시 값이 유효할 때만 revision을 교체한다. 실패해도 이미 읽은 바이트만큼 reader cursor는 진행할 수 있다.
	[[nodiscard]] bool Read_GameplayDataRevision(CPacketReader& reader, GameplayDataRevision& revision);
}

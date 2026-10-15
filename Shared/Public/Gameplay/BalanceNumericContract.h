#pragma once

// 조회 세대·편집 기준 세대·적용 결과 세대를 같은 32 byte 값으로 주고받는다.
#include "GameplayDataRevision.h"
// 요청 번호와 페이지 번호, domain/result의 wire 정수 너비를 고정한다.
#include <cstdint>
// 수치 key와 판정 사유를 메시지가 소유한다. 원본 문서나 수신 buffer의 수명에 묶이지 않는다.
#include <string>
// 페이지의 현재 수치와 변경 요청 목록을 각각 독립 소유한다.
#include <vector>

namespace LostArk::Shared
{
	class CPacketReader;
	class CPacketWriter;

	// 패킷의 목록과 문자열 크기를 제한한다. 실제 카탈로그 존재·수치 범위·권한 검사는 Server가 수행한다.
	inline constexpr std::size_t MAX_BALANCE_CHANGES = 128u;
	inline constexpr std::size_t MAX_BALANCE_PAGE_ENTRIES = 64u;
	inline constexpr std::uint32_t MAX_BALANCE_PAGES = 512u;
	inline constexpr std::size_t MAX_BALANCE_ID_BYTES = 128u;
	inline constexpr std::size_t MAX_BALANCE_FIELD_BYTES = 64u;
	inline constexpr std::size_t MAX_BALANCE_REASON_BYTES = 512u;

	// Server 수치 store가 key의 첫 요소로 쓰는 분류다. codec는 END 미만만 확인하며 해당 Catalog/필드의 존재는 store가 검사한다.
	enum class BALANCE_DOMAIN : std::uint8_t
	{
		PLAYER,
		SKILL,
		DAMAGE,
		BOSS,
		MADNESS,
		STAGGER,
		PATTERN_DAMAGE,
		END
	};
	// Server의 적용 판정을 Client 요청 결과 큐로 전달한다. APPLIED의 세대는 Client가 다음 전체 페이지 조회를 시작하는 기준이다.
	enum class BALANCE_APPLY_RESULT : std::uint8_t
	{
		APPLIED,
		STALE_REVISION,
		INVALID_CHANGE,
		BUSY,
		SAVE_FAILED,
		INVALID_SESSION,
		END
	};

	// 페이지 조회에 담는 현재 수치다. 식별자는 domain + id + field 조합이며 vector 위치는 저장 ID가 아니다.
	struct BALANCE_NUMERIC_ENTRY
	{
		BALANCE_DOMAIN eDomain = BALANCE_DOMAIN::PLAYER;
		std::string strId, strField;
		double fValue = 0.0;
		bool isIntegral = true;
	};

	// 편집 전 값과 요청 값을 함께 보내 낡은 draft가 현재 수치를 덮어쓰지 않도록 한다.
	struct BALANCE_NUMERIC_CHANGE
	{
		BALANCE_DOMAIN eDomain = BALANCE_DOMAIN::PLAYER;
		std::string strId, strField;
		double fBefore = 0.0, fValue = 0.0;
	};

	// Client의 Send_BalanceQueryPage가 한 페이지씩 요청한다. 번호는 0이 아니어야 하고 페이지 index는 0부터 시작한다.
	struct C2S_BALANCE_QUERY
	{
		std::uint32_t iRequestSequence = 0u, iPageIndex = 0u;
	};

	// Server BuildSnapshot이 만드는 단일 페이지다. Client는 요청 번호·다음 페이지·세대 일치를 확인한 뒤 전체 목록을 모아 교체한다.
	// codec의 key 중복 검사는 이 페이지 안에 한정되며 여러 페이지 사이 중복은 Receive_BalanceSnapshot이 거부한다.
	struct S2C_BALANCE_SNAPSHOT
	{
		std::uint32_t iRequestSequence = 0u;
		GameplayDataRevision NumericRevision{};
		std::uint32_t iPageIndex = 0u, iPageCount = 0u;
		std::vector<BALANCE_NUMERIC_ENTRY> Entries;
	};

	// Server는 BaseNumericRevision과 현재 세대를 대조한다. 이 패킷 타입 자체가 적용/저장을 수행하지는 않는다.
	struct C2S_BALANCE_PATCH
	{
		std::uint32_t iRequestSequence = 0u;
		GameplayDataRevision BaseNumericRevision{};
		std::vector<BALANCE_NUMERIC_CHANGE> Changes;
	};

	// Server Send_NumericBalanceResult가 요청 응답 또는 적용 알림을 보낸다. reason은 표시/진단 문자열이며 수치 저장 결과 자체는 eResult와 세대로 판단한다.
	struct S2C_BALANCE_RESULT
	{
		// 0은 특정 요청의 응답이 아닌 프로세스 전체 적용 세대 알림이며 APPLIED에서만 허용한다.
		std::uint32_t iRequestSequence = 0u;
		BALANCE_APPLY_RESULT eResult = BALANCE_APPLY_RESULT::INVALID_CHANGE;
		GameplayDataRevision ActiveNumericRevision{};
		std::string strReason;
	};

	// codec는 길이·형식·유한 수치·목록 중복을 검사한다. Read 실패는 출력 메시지를 보존한다.
	// reader 위치나 writer 버퍼까지 rollback하지는 않으므로 실패한 패킷 후보는 호출자가 폐기해야 한다.
	// 이 계층에는 연결·store·공유 잠금 상태가 없다. 호출자는 메시지와 reader/writer를 호출 동안 유효하게 유지하고 동시 변경을 막는다.
	// 남은 payload가 0인지 여부는 Client/Server의 패킷 분배기가 별도로 검사한다. 할당 예외를 bool false로 바꾸는 함수는 아니다.
	bool Write_Message(CPacketWriter&, const C2S_BALANCE_QUERY&);
	bool Read_Message(CPacketReader&, C2S_BALANCE_QUERY&);
	bool Write_Message(CPacketWriter&, const S2C_BALANCE_SNAPSHOT&);
	bool Read_Message(CPacketReader&, S2C_BALANCE_SNAPSHOT&);
	bool Write_Message(CPacketWriter&, const C2S_BALANCE_PATCH&);
	bool Read_Message(CPacketReader&, C2S_BALANCE_PATCH&);
	bool Write_Message(CPacketWriter&, const S2C_BALANCE_RESULT&);
	bool Read_Message(CPacketReader&, S2C_BALANCE_RESULT&);
}

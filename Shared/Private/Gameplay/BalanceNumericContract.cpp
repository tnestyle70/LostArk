// 수치 조회·편집·결과 메시지와 목록/문자열 상한의 직렬화 본문을 구현한다.
#include "Gameplay/BalanceNumericContract.h"
// 고정 너비 정수와 길이 prefix 문자열을 읽고 지역 메시지 후보를 구성한다.
#include "Network/PacketReader.h"
// 검증한 필드를 wire 순서로 소유 byte buffer 뒤에 추가한다.
#include "Network/PacketWriter.h"
// double과 uint64_t 사이를 수치 반올림 없이 비트 표현 그대로 변환한다.
#include <bit>
// NaN/Inf를 거부하고 integral 표시가 있는 값의 정수 여부를 floor로 검사한다.
#include <cmath>
// 메시지 목록 안의 domain/id/field 조합 중복을 삽입 결과로 검사한다.
#include <set>
// domain·id·field 세 값을 하나의 비교 가능한 key로 묶는다.
#include <tuple>
// 검증이 끝난 지역 메시지의 문자열/vector 소유권을 출력으로 이동한다.
#include <utility>

using namespace LostArk::Shared;

namespace
{
	// codec 내부 문법 검사다. 이 문자 집합에는 profile의 복합 key용 :, /, |도 포함한다.
	// AttackHitTemplate의 stable ID 규칙과 다르므로 이름이 비슷하다고 서로 대체하지 않는다.
	// Write/Read의 목록 검사가 재사용한다. byte 단위 허용 문자만 확인하므로 실제 domain별 ID 의미나 서버 권한은 판단하지 않는다.
	bool Key(BALANCE_DOMAIN domain, const std::string& id, const std::string& field)
	{
		const auto stable = [](const std::string& s, std::size_t limit)
		{
			if (s.empty() || s.size() > limit)
				return false;

			for (const unsigned char c : s)
				if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' ||
						c == '.' || c == '-' || c == ':' || c == '/' || c == '|'))
					return false;
			return true;
		};
		return domain < BALANCE_DOMAIN::END && stable(id, MAX_BALANCE_ID_BYTES) &&
			stable(field, MAX_BALANCE_FIELD_BYTES);
	}

	// double의 bit 표현을 두 U32로 나눈다. 실제 byte 순서는 PacketWriter의 U32 계약을 따른다.
	// 상위 Write_Message가 먼저 유한 수치를 검사한다. 낮은 32비트를 먼저 쓰며 부호/소수 부분을 별도 정수로 변환하지 않는다.
	void Double(CPacketWriter& writer, double value)
	{
		const auto bits = std::bit_cast<std::uint64_t>(value);
		writer.Write_U32(static_cast<std::uint32_t>(bits));
		writer.Write_U32(static_cast<std::uint32_t>(bits >> 32u));
	}

	// 두 U32를 합쳐 double로 해석하고 NaN/Inf를 거부한다. 실패해도 reader의 소비 위치는 되돌리지 않는다.
	// 두 U32를 다 읽은 뒤 value에 먼저 대입하므로 NaN/Inf 거절 때도 이 helper의 출력은 바뀐다.
	// Read_Message는 지역 후보의 필드만 전달해 최종 메시지 출력의 보존과 구분한다.
	bool Double(CPacketReader& reader, double& value)
	{
		std::uint32_t low{}, high{};
		if (!reader.Read_U32(low) || !reader.Read_U32(high))
			return false;

		value = std::bit_cast<double>((static_cast<std::uint64_t>(high) << 32u) | low);
		return std::isfinite(value);
	}

	// 같은 domain/id/field의 중복을 set 삽입 결과로 찾는다. 수치 의미나 Catalog 참조 존재는 이 검사 밖이다.
	// snapshot에서는 한 페이지, patch에서는 요청 전체가 검사 범위다. CHANGE의 fBefore 유한성은 해당 Write/Read_Message가 별도로 검사한다.
	template <class T>
	bool UniqueKeys(const std::vector<T>& entries)
	{
		std::set<std::tuple<BALANCE_DOMAIN, std::string, std::string>> keys;
		for (const auto& entry : entries)
			if (!Key(entry.eDomain, entry.strId, entry.strField) || !std::isfinite(entry.fValue) ||
				!keys.emplace(entry.eDomain, entry.strId, entry.strField).second)
				return false;
		return true;
	}

	// Write_Message가 문법·길이를 미리 검사한 행을 전달한다. domain 기록 뒤 문자열이 실패해도 writer의 앞 bytes는 남는다.
	template <class T>
	bool WriteKey(CPacketWriter& writer, const T& entry)
	{
		writer.Write_U8(static_cast<std::uint8_t>(entry.eDomain));
		return writer.Write_String(entry.strId, MAX_BALANCE_ID_BYTES) &&
			writer.Write_String(entry.strField, MAX_BALANCE_FIELD_BYTES);
	}

	// Read_Message의 지역 행을 채운다. 문자열 읽기 실패는 reader와 이 행을 일부 변경할 수 있어 실패한 후보는 폐기한다.
	// 모든 필드를 읽은 뒤 domain enum과 key 문자 집합을 함께 검사하며 Catalog 조회는 하지 않는다.
	template <class T>
	bool ReadKey(CPacketReader& reader, T& entry)
	{
		std::uint8_t domain{};
		if (!reader.Read_U8(domain) || !reader.Read_String(entry.strId, MAX_BALANCE_ID_BYTES) ||
			!reader.Read_String(entry.strField, MAX_BALANCE_FIELD_BYTES))
			return false;

		entry.eDomain = static_cast<BALANCE_DOMAIN>(domain);
		return Key(entry.eDomain, entry.strId, entry.strField);
	}
}

// 아래 Write_Message는 검증 뒤 필드를 순서대로 기록한다. 도중 실패하면 writer에 앞 필드가 남을 수 있다.
// Client Send_BalanceQueryPage가 보내는 요청이다. 두 필드의 거부 조건을 모두 검사한 뒤 byte 기록을 시작한다.
bool LostArk::Shared::Write_Message(CPacketWriter& w, const C2S_BALANCE_QUERY& m)
{
	if (!m.iRequestSequence || m.iPageIndex >= MAX_BALANCE_PAGES)
		return false;

	w.Write_U32(m.iRequestSequence);
	w.Write_U32(m.iPageIndex);
	return true;
}

// 아래 Read_Message는 지역 s에 읽고 모든 검사를 통과해야 m을 교체한다. r의 cursor는 별도 상태다.
// Server 수신 분배기가 제어 이벤트 후보를 읽는다. 실제 페이지가 존재하는지는 룸 경계의 store 조회가 판단한다.
bool LostArk::Shared::Read_Message(CPacketReader& r, C2S_BALANCE_QUERY& m)
{
	C2S_BALANCE_QUERY s;

	if (!r.Read_U32(s.iRequestSequence) || !r.Read_U32(s.iPageIndex) || !s.iRequestSequence ||
		s.iPageIndex >= MAX_BALANCE_PAGES)
		return false;

	m = s;
	return true;
}

// 페이지 수/번호·목록 상한·고유 key와 integral 표시를 검증한다. double 수치의 비트는 손실 없이 직렬화한다.
// Server의 조회 응답 경로가 BuildSnapshot 결과를 전달한다. 빈 Entries도 유효한 한 페이지이며 페이지 수 자체는 1 이상이어야 한다.
bool LostArk::Shared::Write_Message(CPacketWriter& w, const S2C_BALANCE_SNAPSHOT& m)
{
	if (!m.iRequestSequence || !m.NumericRevision.Is_Valid() || !m.iPageCount || m.iPageCount > MAX_BALANCE_PAGES ||
		m.iPageIndex >= m.iPageCount || m.Entries.size() > MAX_BALANCE_PAGE_ENTRIES || !UniqueKeys(m.Entries))
		return false;

	for (const auto& e : m.Entries)
		if (e.isIntegral && std::floor(e.fValue) != e.fValue)
			return false;

	w.Write_U32(m.iRequestSequence);
	if (!Write_GameplayDataRevision(w, m.NumericRevision))
		return false;
	w.Write_U32(m.iPageIndex);
	w.Write_U32(m.iPageCount);
	w.Write_U16(static_cast<std::uint16_t>(m.Entries.size()));

	for (const auto& e : m.Entries)
	{
		if (!WriteKey(w, e))
			return false;
		Double(w, e.fValue);
		w.Write_U8(e.isIntegral ? 1u : 0u);
	}
	return true;
}

// Client 수신 분배기가 읽는 단일 페이지다. header 상한을 통과한 뒤에만 지역 Entries를 할당하고 행의 수치·integral 표시·중복을 검사한다.
// 모든 행이 성공해야 m을 교체한다. 요청/페이지 간 순서와 같은 세대 여부는 Receive_BalanceSnapshot이 이어서 검사한다.
bool LostArk::Shared::Read_Message(CPacketReader& r, S2C_BALANCE_SNAPSHOT& m)
{
	S2C_BALANCE_SNAPSHOT s;
	std::uint16_t count{};

	if (!r.Read_U32(s.iRequestSequence) || !s.iRequestSequence || !Read_GameplayDataRevision(r, s.NumericRevision) ||
		!r.Read_U32(s.iPageIndex) || !r.Read_U32(s.iPageCount) || !s.iPageCount || s.iPageCount > MAX_BALANCE_PAGES ||
		s.iPageIndex >= s.iPageCount || !r.Read_U16(count) || count > MAX_BALANCE_PAGE_ENTRIES)
		return false;

	// count는 wire의 U16이지만 앞에서 페이지 상한 64로 제한했다. 각 행은 문자열과 값을 직접 소유해 수신 buffer를 빌리지 않는다.
	s.Entries.resize(count);
	for (auto& e : s.Entries)
	{
		std::uint8_t integral{};
		if (!ReadKey(r, e) || !Double(r, e.fValue) || !r.Read_U8(integral) || integral > 1u)
			return false;
		e.isIntegral = integral != 0u;
		if (e.isIntegral && std::floor(e.fValue) != e.fValue)
			return false;
	}

	if (!UniqueKeys(s.Entries))
		return false;

	m = std::move(s);
	return true;
}

// 빈 변경 목록과 중복 key를 거부하며 before/value는 모두 유한해야 한다. 실제 revision 일치는 Server 책임이다.
// Client Send_BalancePatch가 편집한 before/value 쌍을 보낸다. 모든 행의 문법과 유한성 검사가 끝난 뒤 요청 번호부터 기록한다.
bool LostArk::Shared::Write_Message(CPacketWriter& w, const C2S_BALANCE_PATCH& m)
{
	if (!m.iRequestSequence || !m.BaseNumericRevision.Is_Valid() || m.Changes.empty() ||
		m.Changes.size() > MAX_BALANCE_CHANGES || !UniqueKeys(m.Changes))
		return false;

	for (const auto& e : m.Changes)
		if (!std::isfinite(e.fBefore))
			return false;

	w.Write_U32(m.iRequestSequence);
	if (!Write_GameplayDataRevision(w, m.BaseNumericRevision))
		return false;
	w.Write_U16(static_cast<std::uint16_t>(m.Changes.size()));

	for (const auto& e : m.Changes)
	{
		if (!WriteKey(w, e))
			return false;
		Double(w, e.fBefore);
		Double(w, e.fValue);
	}
	return true;
}

// Server 수신 분배기가 지역 patch를 읽는다. 길이·형식 검사만 수행하고 실제 before 일치·정수 필드·revision CAS는 ServerBalanceNumericStore가 판단한다.
// ReadKey/Double이 지역 후보를 일부 바꾼 뒤 실패해도 호출자가 넘긴 m은 마지막 이동 대입 전까지 유지한다.
bool LostArk::Shared::Read_Message(CPacketReader& r, C2S_BALANCE_PATCH& m)
{
	C2S_BALANCE_PATCH s;
	std::uint16_t count{};

	if (!r.Read_U32(s.iRequestSequence) || !s.iRequestSequence ||
		!Read_GameplayDataRevision(r, s.BaseNumericRevision) || !r.Read_U16(count) || !count ||
		count > MAX_BALANCE_CHANGES)
		return false;

	// 0개 요청은 앞에서 거부했다. 상한 128 안에서 후보 행을 만들고 before/value 두 double을 각각 유한 수치로 읽는다.
	s.Changes.resize(count);
	for (auto& e : s.Changes)
		if (!ReadKey(r, e) || !Double(r, e.fBefore) || !Double(r, e.fValue))
			return false;

	if (!UniqueKeys(s.Changes))
		return false;

	m = std::move(s);
	return true;
}

// 요청 응답과 sequence 0의 전체 세대 알림을 같은 형식으로 쓰되, 전체 알림은 APPLIED만 허용한다.
// Server Send_NumericBalanceResult가 활성 수치 세대와 판정 사유를 전달한다. 이 serializer는 디스크 저장이나 연결 송신을 수행하지 않는다.
bool LostArk::Shared::Write_Message(CPacketWriter& w, const S2C_BALANCE_RESULT& m)
{
	if (m.eResult >= BALANCE_APPLY_RESULT::END || !m.ActiveNumericRevision.Is_Valid() ||
		m.strReason.size() > MAX_BALANCE_REASON_BYTES ||
		(!m.iRequestSequence && m.eResult != BALANCE_APPLY_RESULT::APPLIED))
		return false;

	w.Write_U32(m.iRequestSequence);
	w.Write_U8(static_cast<std::uint8_t>(m.eResult));
	return Write_GameplayDataRevision(w, m.ActiveNumericRevision) &&
		w.Write_String(m.strReason, MAX_BALANCE_REASON_BYTES);
}

// Client가 적용 판정과 현재 세대를 읽는다. sequence 0은 APPLIED 알림에만 허용하고 최종 검사 성공 뒤 문자열까지 소유 이동한다.
// 요청과의 대응·재조회 예약·결과 큐 반영은 NetworkManager의 수신 분배기가 처리한다.
bool LostArk::Shared::Read_Message(CPacketReader& r, S2C_BALANCE_RESULT& m)
{
	S2C_BALANCE_RESULT s;
	std::uint8_t result{};

	if (!r.Read_U32(s.iRequestSequence) || !r.Read_U8(result) ||
		result >= static_cast<std::uint8_t>(BALANCE_APPLY_RESULT::END) ||
		!Read_GameplayDataRevision(r, s.ActiveNumericRevision) || !r.Read_String(s.strReason, MAX_BALANCE_REASON_BYTES))
		return false;
	s.eResult = static_cast<BALANCE_APPLY_RESULT>(result);
	if (!s.iRequestSequence && s.eResult != BALANCE_APPLY_RESULT::APPLIED)
		return false;

	m = std::move(s);
	return true;
}

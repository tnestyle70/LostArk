/* 한국어 학습용 참고 복사본
 * 원본: Shared/Private/Network/PacketStreamParser.cpp
 * 제품 소스를 수정하지 않고 함수 본문을 그대로 보존한 문서용 코드다.
 * 원본과 줄 번호가 다르므로 함수 이름으로 대조한다. 제품 프로젝트에 컴파일 항목으로 추가하지 않는다.
 */

#include "Network/PacketStreamParser.h"

/* 학습 주석
 * 호출자: 세션 Receive_Frame이 recv로 받은 바이트를 넘긴다.
 * 왜: 한 frame이 여러 recv에 나뉘거나 여러 frame이 함께 도착할 수 있다.
 * 상태: 이미 소비한 앞부분을 정리한 뒤 누적 버퍼 뒤에 새 바이트를 복사한다.
 * 실패: 누적 상한을 넘는 입력은 추가하지 않고 false를 반환한다.
 */
bool LostArk::Shared::CPacketStreamParser::Append(std::span<const std::uint8_t> bytes)
{
	Compact();

	//누적 버퍼 상한을 넘기지 않는지 먼저 확인한다.
	if (bytes.size() > MAX_BUFFERED_PACKET_BYTES -
		m_Buffer.size())
		return false;

	m_Buffer.insert(m_Buffer.end(), bytes.begin(), bytes.end());

	return true;
}

/* 학습 주석
 * 호출자: 세션 Receive_Frame은 새 recv보다 먼저 이 함수를 부른다.
 * 왜: 현재 누적 바이트에서 완성된 게임 frame 하나만 꺼내는 경계를 분리한다.
 * 상태: 성공 시 payload를 frame 소유 메모리로 복사하고 읽기 offset을 전진한다.
 * 실패: 부족하면 NEED_MORE_DATA로 보존하고, 잘못된 header이면 버퍼를 비우고 INVALID_FRAME을 반환한다.
 */
LostArk::Shared::PACKET_PARSE_RESULT LostArk::Shared::CPacketStreamParser::Try_Pop(
	PACKET_FRAME& frame)
{
	const std::size_t remainingSize =
		Get_BufferedByteCount();
	//header bytes를 담을 수 있는 용량이 없음
	if (remainingSize < PACKET_HEADER_BYTES)
	{
		return PACKET_PARSE_RESULT::NEED_MORE_DATA;
	}

	//이전 frame을 읽고 남은 부분만 header 검사 대상으로 삼는다.
	const std::span<const std::uint8_t> unreadBytes
	{
		m_Buffer.data() + m_iReadOffset,
		remainingSize
	};

	PACKET_HEADER header{};

	if (!Read_Packet_Header(
		unreadBytes,
		header))
	{
		Reset();

		return PACKET_PARSE_RESULT::INVALID_FRAME;
	}

	if (remainingSize < header.iTotalSize)
	{
		return PACKET_PARSE_RESULT::NEED_MORE_DATA;
	}
	//packet 구조채가 담고 있는 멤버 변수 중 totalsize 기준으로 header bytes size를 빼면
	//payloadsize를 구할 수 있다.
	const std::size_t payloadSize =
		header.iTotalSize -
		PACKET_HEADER_BYTES;
	//subspan은 복사 없이 지정한 범위만 보는 view다. 여기서는 header 뒤의 payload 범위를 고른다.
	const auto payload = unreadBytes.subspan(
		PACKET_HEADER_BYTES,
		payloadSize);

	PACKET_FRAME decoded{};

	decoded.ePacketType = header.ePacketType;

	decoded.Payload.assign(
		payload.begin(),
		payload.end());

	frame = std::move(decoded);
	//방금 소비한 frame 전체 길이만큼 누적 버퍼의 읽기 위치를 옮긴다.
	m_iReadOffset += header.iTotalSize;

	if (m_iReadOffset == m_Buffer.size())
	{
		Reset();
	}

	return PACKET_PARSE_RESULT::FRAME_READY;
}

/* 학습 주석
 * 호출자: 잘못된 frame을 만났거나 누적 데이터를 모두 소비한 parser 경로.
 * 왜: 버퍼와 읽기 위치는 같은 상태이므로 함께 초기화해야 한다.
 */
void LostArk::Shared::CPacketStreamParser::Reset()
{
	//buffer와 readoffset 비워주기
	m_Buffer.clear();
	m_iReadOffset = 0;
}

/* 학습 주석
 * 호출자: Try_Pop. 버퍼 전체 길이가 아니라 아직 소비하지 않은 바이트 수를 반환한다.
 * 상태: 읽기 전용이며 offset이 범위를 벗어나면 안전하게 0을 반환한다.
 */
std::size_t LostArk::Shared::CPacketStreamParser::Get_BufferedByteCount() const
{
	if (m_iReadOffset > m_Buffer.size())
		return 0;

	return m_Buffer.size() - m_iReadOffset;
}

/* 학습 주석
 * 호출자: Append. 이미 읽은 앞부분만 제거하여 누적 버퍼 공간을 재사용한다.
 * 상태: 아직 덜 도착한 다음 frame의 바이트는 보존하고 offset을 0으로 맞춘다.
 */
void LostArk::Shared::CPacketStreamParser::Compact()
{
	if (0 == m_iReadOffset)
		return;

	if (m_iReadOffset >= m_Buffer.size())
	{
		Reset();
		return;
	}
	//iterator에 더할 거리의 정수형으로 변환해 이미 소비한 앞부분만 erase한다.
	m_Buffer.erase(
		m_Buffer.begin(),
		m_Buffer.begin() +
		static_cast<std::ptrdiff_t>(
			m_iReadOffset));

	m_iReadOffset = 0;
}

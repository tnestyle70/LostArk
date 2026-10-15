// 연결별 누적 byte 저장소와 parser 결과를 구현한다. Read_Packet_Header가 타입·크기를 확인하고 Try_Pop이 완전한 frame만 출력한다.
#include "Network/PacketStreamParser.h"

// recv 결과를 parser가 복사해 보관한다. 호출자의 임시 수신 배열은 반환 뒤 재사용할 수 있다.
bool LostArk::Shared::CPacketStreamParser::Append(std::span<const std::uint8_t> bytes)
{
	// Client의 Receive_Loop과 Server의 Receive_Frame이 recv 결과를 전달한다.
	// 먼저 소비한 prefix를 정리하므로 뒤의 상한 거부에서도 물리적 buffer/offset은 바뀔 수 있지만 미소비 byte 내용은 남는다.
	Compact();

	//전체 packet bytes 기준으로, 
	// Compact 뒤에도 미소비 bytes가 상한 이내라는 불변식을 유지한다. 초과한 입력은 일부도 추가하지 않는다.
	if (bytes.size() > MAX_BUFFERED_PACKET_BYTES - m_Buffer.size())
		return false;

	// 외부 수신 배열을 내부 소유 저장소에 복사한다. 할당 예외는 catch하지 않으며 이 함수가 false를 반환하는 조건은 앞의 크기 상한 검사다.
	m_Buffer.insert(m_Buffer.end(), bytes.begin(), bytes.end());

	return true;
}

// 수신 루프가 FRAME_READY 동안 반복 호출한다. 정상적으로 불완전하면 현재 데이터와 호출자 출력을 유지한다.
LostArk::Shared::PACKET_PARSE_RESULT LostArk::Shared::CPacketStreamParser::Try_Pop(PACKET_FRAME& frame)
{
	const std::size_t remainingSize = Get_BufferedByteCount();
	//header bytes를 담을 수 있는 용량이 없음
	// 여기서 remainingSize는 미소비 바이트 수다. 6바이트 header가 아직 덜 도착했으므로 내용을 버리지 않고 NEED_MORE_DATA를 돌려준다.
	if (remainingSize < PACKET_HEADER_BYTES)
	{
		return PACKET_PARSE_RESULT::NEED_MORE_DATA;
	}

	//이거를 왜 구하는 걸까? unread bytes를 구하는 이유가 뭘까?
	// 이전 frame의 bytes를 다시 해석하지 않도록 offset 이후 구간만 reader에 제공한다.
	const std::span<const std::uint8_t> unreadBytes{m_Buffer.data() + m_iReadOffset, remainingSize};

	PACKET_HEADER header{};

	if (!Read_Packet_Header(unreadBytes, header))
	{
		// 잘못된 header 뒤의 재동기화 지점을 추측하지 않는다. 누적 데이터를 버리고 수신 루프에 실패를 알린다.
		Reset();

		return PACKET_PARSE_RESULT::INVALID_FRAME;
	}

	// header는 정상이어도 payload가 덜 왔을 수 있다. 다음 recv 결과를 기다린다.
	if (remainingSize < header.iTotalSize)
	{
		return PACKET_PARSE_RESULT::NEED_MORE_DATA;
	}
	//packet 구조채가 담고 있는 멤버 변수 중 totalsize 기준으로 header bytes size를 빼면
	//payloadsize를 구할 수 있다.
	// 앞에서 전체 frame 수신과 header 길이를 확인했으므로 iTotalSize에서 6바이트 header를 빼 본문 길이를 얻는다.
	const std::size_t payloadSize = header.iTotalSize - PACKET_HEADER_BYTES;
	//subspan이 의미하는 바가 뭘까?
	// header 6바이트를 건너뛴 payload 구간을 차용한다. 뒤에서 vector로 복사한 다음에만 offset을 옮긴다.
	const auto payload = unreadBytes.subspan(PACKET_HEADER_BYTES, payloadSize);

	PACKET_FRAME decoded{};

	decoded.ePacketType = header.ePacketType;

	decoded.Payload.assign(payload.begin(), payload.end());

	// 독립 payload 복사본이 완성된 뒤 출력으로 이동하고 그 다음에 내부 cursor를 전진한다. 출력은 뒤의 Reset/Compact와 별개로 데이터를 소유한다.
	frame = std::move(decoded);
	//packet을 읽을 때의 offset을 의미하는 거 맞나?
	// 이번 frame만 소비한다. 뒤에 붙은 다음 frame은 다음 Try_Pop에서 처리한다.
	m_iReadOffset += header.iTotalSize;

	if (m_iReadOffset == m_Buffer.size())
	{
		Reset();
	}

	return PACKET_PARSE_RESULT::FRAME_READY;
}

// 연결별 parser의 내용과 소비 위치를 초기화한다. clear는 확보한 vector 메모리를 반드시 반납하지 않는다.
void LostArk::Shared::CPacketStreamParser::Reset()
{
	//buffer와 readoffset 비워주기
	// clear로 요소를 비우고 소비 offset을 0으로 초기화한다. vector가 확보한 capacity를 반드시 반납하는 것은 아니다.
	m_Buffer.clear();
	m_iReadOffset = 0;
}

// 누적 vector 전체 크기가 아니라 아직 소비하지 않은 크기를 반환한다.
std::size_t LostArk::Shared::CPacketStreamParser::Get_BufferedByteCount() const
{
	if (m_iReadOffset > m_Buffer.size())
		return 0;

	return m_Buffer.size() - m_iReadOffset;
}

// Append 직전에 소비된 prefix만 제거하고 미소비 bytes를 앞쪽으로 이동한다. 매 Pop마다 이동하지 않는다.
void LostArk::Shared::CPacketStreamParser::Compact()
{
	if (0 == m_iReadOffset)
		return;

	if (m_iReadOffset >= m_Buffer.size())
	{
		Reset();
		return;
	}
	//ptrdiff_t가 의미하는 게 뭘까?
	// iterator 차이는 부호 있는 ptrdiff_t를 사용한다. 이전 분기에서 범위 안임을 확인한 offset을 변환한다.
	m_Buffer.erase(m_Buffer.begin(), m_Buffer.begin() + static_cast<std::ptrdiff_t>(m_iReadOffset));

	m_iReadOffset = 0;
}

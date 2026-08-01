#include "Network/PacketStreamParser.h"

bool LostArk::Shared::CPacketStreamParser::Append(std::span<const std::uint8_t> bytes)
{
	Compact();

	//전체 packet bytes 기준으로, 
	if (bytes.size() > MAX_BUFFERED_PACKET_BYTES -
		m_Buffer.size())
		return false;

	m_Buffer.insert(m_Buffer.end(), bytes.begin(), bytes.end());

	return true;
}

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

	//이거를 왜 구하는 걸까? unread bytes를 구하는 이유가 뭘까?
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
	//subspan이 의미하는 바가 뭘까?
	const auto payload = unreadBytes.subspan(
		PACKET_HEADER_BYTES,
		payloadSize);

	PACKET_FRAME decoded{};

	decoded.ePacketType = header.ePacketType;

	decoded.Payload.assign(
		payload.begin(),
		payload.end());

	frame = std::move(decoded);
	//packet을 읽을 때의 offset을 의미하는 거 맞나?
	m_iReadOffset += header.iTotalSize;
	
	if (m_iReadOffset == m_Buffer.size())
	{
		Reset();
	}

	return PACKET_PARSE_RESULT::FRAME_READY;
}

void LostArk::Shared::CPacketStreamParser::Reset()
{
	//buffer와 readoffset 비워주기
	m_Buffer.clear();
	m_iReadOffset = 0;
}

std::size_t LostArk::Shared::CPacketStreamParser::Get_BufferedByteCount() const
{
	if (m_iReadOffset > m_Buffer.size())
		return 0;

	return m_Buffer.size() - m_iReadOffset;
}

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
	m_Buffer.erase(
		m_Buffer.begin(),
		m_Buffer.begin() +
		static_cast<std::ptrdiff_t>(
			m_iReadOffset));

	m_iReadOffset = 0; 
}

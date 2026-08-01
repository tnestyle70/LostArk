#include "Network/PacketFrame.h"

#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"

//함수의 역할이 그냥 packet을 생성하는 건가? 전체 payload와 packet header size를 바탕으로? 
bool LostArk::Shared::Build_Packet_Frame(
	PACKET_TYPE packetType, 
	std::span<const std::uint8_t> payload, 
	std::vector<std::uint8_t>& frameBytes)
{
	//[[nodiscard]] 이게 의미하는 게 뭐지?
	if (!Is_Known_Packet_Type(packetType))
		return false;
	//header bytes를 전체 packet bytes랑 빼서, payload, 즉 전체 payload의
	//사이즈를 구한다. header 정보 제외한 실질적인 정보를 가질 수 있는 데이터의 크기를 의미한다.
	const std::size_t maxPayloadSize =
		MAX_PACKET_BYTES -
		PACKET_HEADER_BYTES;
	//size 검사
	if (payload.size() > maxPayloadSize)
		return false;
	//packet header와 payload의 size를 검증
	const std::uint32_t totalSize =
		static_cast<std::uint32_t>(
			PACKET_HEADER_BYTES +
			payload.size());
	//packet을 작성한다.
	CPacketWriter writer;
	//전체 header + payload 포함 사이즈로 작성
	writer.Write_U32(totalSize);
	
	writer.Write_U16(
		static_cast<std::uint16_t>(
			packetType));

	writer.Write_Bytes(payload);

	frameBytes = writer.Get_Buffer();
	
	return true;
}

bool LostArk::Shared::Read_Packet_Header(
	std::span<const std::uint8_t> bytes, 
	PACKET_HEADER& header)
{
	//packet header의 size를 검사
	if (bytes.size() < PACKET_HEADER_BYTES)
		return false;
	//packetheader read
	CPacketReader reader{ bytes };

	std::uint32_t totalSize = {};
	std::uint16_t rawPacketType = {};

	if (!reader.Read_U32(totalSize))
		return false;

	if (!reader.Read_U16(rawPacketType))
		return false;

	const PACKET_TYPE packetType =
		static_cast<PACKET_TYPE>(
			rawPacketType);

	if (!Is_Known_Packet_Type(packetType))
		return false;

	if (totalSize < PACKET_HEADER_BYTES)
		return false;

	if (totalSize > MAX_PACKET_BYTES)
		return false;

	//왜 그냥 header에 넣지 않고, decoded에 넣을까?
	PACKET_HEADER decoded{};

	decoded.iTotalSize = totalSize;
	decoded.ePacketType = packetType;

	header = decoded;

	return true;
}

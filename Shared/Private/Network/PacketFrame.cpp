// header/frame 값 형식과 packet 타입·크기 계약을 사용해 송수신 경계를 구현한다.
#include "Network/PacketFrame.h"

// Read_Packet_Header가 U32 전체 길이와 U16 타입을 little-endian 순서로 읽는다.
#include "Network/PacketReader.h"
// Build_Packet_Frame이 header와 payload를 지역 writer에 완성한 뒤 출력 vector로 복사한다.
#include "Network/PacketWriter.h"

//함수의 역할이 그냥 packet을 생성하는 건가? 전체 payload와 packet header size를 바탕으로? 
// Client/Server의 송신 준비가 호출한다. 임시 writer를 완성한 뒤에만 출력 vector를 교체한다.
// 이 함수가 true여도 소켓 전송이 완료된 것은 아니며 송신 큐와 send loop는 별도 책임이다.
bool LostArk::Shared::Build_Packet_Frame(
	PACKET_TYPE packetType, std::span<const std::uint8_t> payload, std::vector<std::uint8_t>& frameBytes)
{
	//[[nodiscard]] 이게 의미하는 게 뭐지?
	// [[nodiscard]]는 반환값을 무시하지 않도록 알리는 C++ 속성이다. 여기서는 Is_Known_Packet_Type의 bool 결과를 조건으로 사용해 알 수 없는 타입을 거부한다.
	if (!Is_Known_Packet_Type(packetType))
		return false;
	//header bytes를 전체 packet bytes랑 빼서, payload, 즉 전체 payload의
	//사이즈를 구한다. header 정보 제외한 실질적인 정보를 가질 수 있는 데이터의 크기를 의미한다.
	// 전체 frame 상한에서 6바이트 header를 뺀 만큼만 payload로 허용한다.
	const std::size_t maxPayloadSize = MAX_PACKET_BYTES - PACKET_HEADER_BYTES;
	//size 검사
	// payload 크기가 계산한 상한을 넘으면 출력 frameBytes를 건드리기 전에 false를 반환한다.
	if (payload.size() > maxPayloadSize)
		return false;
	//packet header와 payload의 size를 검증
	// 이 대입은 검사가 아니라 전체 길이 계산이다. 앞에서 허용 크기를 검사한 header 크기와 payload 크기를 더한다.
	const std::uint32_t totalSize = static_cast<std::uint32_t>(PACKET_HEADER_BYTES + payload.size());
	//packet을 작성한다.
	// 임시 writer가 header와 payload를 완성한 뒤 frameBytes에 복사하므로 앞선 거부 분기는 기존 출력을 보존한다.
	CPacketWriter writer;
	//전체 header + payload 포함 사이즈로 작성
	// wire header는 구조체 memcpy가 아니라 명시한 두 수치 필드로 구성한다.
	writer.Write_U32(totalSize);

	writer.Write_U16(static_cast<std::uint16_t>(packetType));

	writer.Write_Bytes(payload);

	// 출력은 writer 소유 vector에서 복사하므로 지역 writer가 끝나도 유효하다. 할당 예외를 catch하는 함수는 아니며 bool false 경로는 위 검증 분기뿐이다.
	frameBytes = writer.Get_Buffer();

	return true;
}

// StreamParser가 현재 미소비 구간의 시작에서 부른다. 여기서는 header만 읽고 타입/전체 길이를 검사한다.
bool LostArk::Shared::Read_Packet_Header(std::span<const std::uint8_t> bytes, PACKET_HEADER& header)
{
	//packet header의 size를 검사
	// 최소 header 크기인 6바이트가 도착했는지 검사한다. 이 조건만으로 payload 전체 수신이 보장되지는 않는다.
	if (bytes.size() < PACKET_HEADER_BYTES)
		return false;
	// 원본 span을 차용하는 지역 reader다. 여기서 reader offset이 전진해도 호출자의 span이나 StreamParser offset은 바뀌지 않는다.
	CPacketReader reader{bytes};

	std::uint32_t totalSize = {};
	std::uint16_t rawPacketType = {};

	if (!reader.Read_U32(totalSize))
		return false;

	if (!reader.Read_U16(rawPacketType))
		return false;

	const PACKET_TYPE packetType = static_cast<PACKET_TYPE>(rawPacketType);

	if (!Is_Known_Packet_Type(packetType))
		return false;

	if (totalSize < PACKET_HEADER_BYTES)
		return false;

	if (totalSize > MAX_PACKET_BYTES)
		return false;

	//왜 그냥 header에 넣지 않고, decoded에 넣을까?
	// 모든 필드 검증이 끝난 결과만 출력하여 false 경로가 기존 header를 망가뜨리지 않게 한다.
	PACKET_HEADER decoded{};

	decoded.iTotalSize = totalSize;
	decoded.ePacketType = packetType;

	header = decoded;

	return true;
}

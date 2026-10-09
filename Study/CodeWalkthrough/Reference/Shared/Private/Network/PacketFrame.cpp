/* 한국어 학습용 참고 복사본
 * 원본: Shared/Private/Network/PacketFrame.cpp
 * 제품 소스를 수정하지 않고 함수 본문을 그대로 보존한 문서용 코드다.
 * 원본과 줄 번호가 다르므로 함수 이름으로 대조한다. 제품 프로젝트에 컴파일 항목으로 추가하지 않는다.
 */

#include "Network/PacketFrame.h"

#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"

//전체 길이와 종류를 header로 직렬화하고 뒤에 payload를 붙여 TCP로 보낼 한 frame을 만든다.
/* 학습 주석
 * 호출자: ClientSession::Send_Frame과 패킷을 구성하는 코드.
 * 왜: TCP에는 게임 메시지 경계가 없으므로 전체 길이와 종류를 payload 앞에 붙인다.
 * 상태: 입력 payload를 읽고 성공한 frame 바이트를 출력 vector에 복사한다.
 * 실패: 알 수 없는 종류나 크기 초과를 거부하며 출력 frame은 마지막 대입 전까지 보존한다.
 */
bool LostArk::Shared::Build_Packet_Frame(
	PACKET_TYPE packetType,
	std::span<const std::uint8_t> payload,
	std::vector<std::uint8_t>& frameBytes)
{
	//알려진 패킷 종류인지 확인한다. [[nodiscard]]는 결과를 무시하지 말라는 컴파일러 진단 요청이다.
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

/* 학습 주석
 * 호출자: PacketStreamParser::Try_Pop.
 * 왜: 누적 바이트에서 게임 frame 하나의 길이와 종류를 알아내야 한다.
 * 상태: 임시 decoded에 읽고 모든 검사를 통과한 뒤 출력 header를 변경한다.
 * 실패: 잘린 header, 알 수 없는 종류, 허용 범위 밖 크기이면 false를 반환한다.
 */
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

	//검증된 결과만 마지막에 출력 header에 대입하여 실패 시 호출자의 기존 값을 보존한다.
	PACKET_HEADER decoded{};

	decoded.iTotalSize = totalSize;
	decoded.ePacketType = packetType;

	header = decoded;

	return true;
}

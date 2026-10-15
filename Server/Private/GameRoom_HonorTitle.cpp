// 룸의 session/player 바인딩과 칭호 catalog를 사용하는 명령 처리 멤버를 구현한다.
#include "GameRoom.h"
// 룸 명령의 sequence 유효성·신규 여부를 판정하는 Is_NewerSequence 선언을 사용한다.
#include "GameRoom_Internal.h"

// 판정 응답 프레임을 송신하고 실패한 연결에 종료를 요청한다.
#include "ClientSession.h"
// 칭호 요청/응답 값과 판정 enum, Write_Message 직렬화 경계를 사용한다.
#include "Network/PacketMessages.h"
// 응답 payload를 소유하는 지역 byte buffer를 구성한다.
#include "Network/PacketWriter.h"

// 응답 처리 동안 연결의 수명을 유지하는 shared_ptr를 사용한다.
#include <memory>

using namespace GameRoomDetail;

// Server는 플레이어가 장착한 칭호 ID를 소유한다. 승인 상태는 snapshot으로 복제하고
// 월드 전환은 ROOM_COMMAND의 carried ID로 운반한다. 이 응답은 요청 sequence와 판정/현재 ID를 돌려준다.
// 칭호 해제를 제외한 새 ID는 대상 룸의 게시 목록에 있어야 하며 표시 이름은 Client가 해석한다.

// Tick이 룸 큐의 SET_HONOR_TITLE 명령을 소비하며 호출한다. 실제 session이 없으면 응답 없이 끝낸다.
// 연결은 shared_ptr로 처리 동안 유지하고 player 바인딩까지 맞아야 상태 변경 경로로 넘긴다.
// 직렬화/송신 실패는 연결 종료를 요청하며 이미 적용된 칭호 상태를 여기서 rollback하지 않는다.
void LostArk::Server::CGameRoom::Handle_SetHonorTitle(const SESSION_ID sessionId,
                                                      const LostArk::Shared::C2S_SET_HONOR_TITLE& request)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session)
		return;
	// 기본 거절 응답에 요청 번호와 현재 룸을 넣고 올바른 player 연결일 때만 실제 판정으로 교체한다.
	S2C_SET_HONOR_TITLE_RESULT result{};
	result.iRequestSequence = request.iRequestSequence;
	result.eWorldId = m_eWorldId;
	result.eResult = HONOR_TITLE_RESULT::REJECTED_SESSION;
	const auto binding = m_PlayerIdBySessionId.find(sessionId);
	const auto player = binding == m_PlayerIdBySessionId.end() ? m_Players.end() : m_Players.find(binding->second);
	if (player != m_Players.end() && player->second.iSessionId == sessionId)
		result = Apply_SetHonorTitle(player->second, request);
	// 판정은 요청자에게 보내고 다른 Client가 보는 지속 장착 상태는 기존 world snapshot 경로가 전달한다.
	CPacketWriter writer;
	if (!Write_Message(writer, result) ||
	    !session->Send_Frame(PACKET_TYPE::S2C_SET_HONOR_TITLE_RESULT, writer.Get_Buffer()))
	{
		session->Request_Close();
	}
}

// Handle_SetHonorTitle이 바인딩을 검증한 player와 요청을 전달한다. 요청 월드·sequence·catalog를 검사한다.
// 승인 때만 장착 ID를 바꾸고 캐릭터 복원 기회를 닫는다. 새 sequence의 같은 상태/미등록 ID 거절도
// LastHonorTitleResult에 보관해 같은 요청 재전송은 동일한 판정을 돌려준다.
LostArk::Shared::S2C_SET_HONOR_TITLE_RESULT LostArk::Server::CGameRoom::Apply_SetHonorTitle(
    SERVER_PLAYER& player, const LostArk::Shared::C2S_SET_HONOR_TITLE& request)
{
	using namespace LostArk::Shared;
	S2C_SET_HONOR_TITLE_RESULT result{};
	result.iRequestSequence = request.iRequestSequence;
	result.eWorldId = m_eWorldId;
	result.iActiveHonorTitleId = player.iHonorTitleId;
	if (request.eWorldId != m_eWorldId)
	{
		result.eResult = HONOR_TITLE_RESULT::REJECTED_WRONG_WORLD;
		return result;
	}
	// 다른 월드 요청은 위에서 거절한다. 같은 0이 아닌 sequence는 이전 결과를 그대로 돌려주고
	// 0 또는 오래된 sequence는 거절하되 마지막 판정 기록을 덮어쓰지 않는다.
	const S2C_SET_HONOR_TITLE_RESULT& previous = player.LastHonorTitleResult;
	if (0u != request.iRequestSequence && request.iRequestSequence == previous.iRequestSequence)
	{
		return previous;
	}
	if (!Is_NewerSequence(request.iRequestSequence, previous.iRequestSequence))
	{
		result.eResult = HONOR_TITLE_RESULT::REJECTED_STALE_SEQUENCE;
		return result;
	}
	// 새 요청의 판정과 그 시점의 장착 ID를 함께 기록한다. 외부 저장이나 패킷 전송은 수행하지 않는다.
	const auto commit = [&player, &result](const HONOR_TITLE_RESULT reason)
	{
		result.eResult = reason;
		result.iActiveHonorTitleId = player.iHonorTitleId;
		player.LastHonorTitleResult = result;
		return result;
	};
	// 현재와 같은 값도 재전송 가능한 거절 판정으로 남긴다. 무효 ID는 새 칭호가 아니라 해제 요청으로 허용한다.
	if (request.iHonorTitleId == player.iHonorTitleId)
		return commit(HONOR_TITLE_RESULT::REJECTED_SAME_STATE);
	if (INVALID_HONOR_TITLE_ID != request.iHonorTitleId && !m_HonorTitleCatalog.Has_Title(request.iHonorTitleId))
	{
		return commit(HONOR_TITLE_RESULT::REJECTED_UNKNOWN_TITLE);
	}
	// 장착/해제를 확정한 뒤 초기 입장용 1회 복원 기회를 닫아 예전 roster 값으로 다시 덮지 못하게 한다.
	player.iHonorTitleId = request.iHonorTitleId;
	player.bRestoreAvailable = false;
	return commit(HONOR_TITLE_RESULT::ACCEPTED);
}

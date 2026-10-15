// 전환 요청과 복구 진단의 공개 계약을 구현한다.
#include "LevelTransitionService.h"

// 등록된 Level만 전환 대상으로 허용한다.
#include "LevelRegistry.h"
// Server 승인 응답과 연결 종료 snapshot을 소비하고 전환 실패 시 연결을 닫는다.
#include "NetworkManager.h"

// 연결 진단에 종료 시각이 없을 때 Unix ms 시각을 만든다.
#include <chrono>
// 프로세스 공용 요청·진단·표시 자료의 조회와 교체를 동기화한다.
#include <mutex>
// 대기 요청과 복구 진단, 매치 자료의 존재 여부를 값과 함께 보관한다.
#include <optional>

namespace
{
	// 아래 상태는 이 번역 단위가 소유한다. 외부에는 복사본이나 COM 참조만 반환하며 모든 접근을 같은 mutex로 보호한다.
	std::mutex g_TransitionMutex;
	std::optional<Client::LEVEL_TRANSITION_REQUEST> g_PendingRequest;
	std::optional<Client::CLIENT_RECOVERY_DIAGNOSTIC> g_RecoveryDiagnostic;
	std::string g_Status = "No level transition is pending.";
	LEVEL g_LastWorldTransferOrigin = LEVEL::END;
	ComPtr<ID3D11ShaderResourceView> g_TransferPortraitSRV;
	std::optional<LostArk::Shared::S2C_COLOSSEUM_MATCH_FOUND> g_ColosseumMatch;

	// 복구 진단에 기록할 벽시계 시각이다. 게임 진행 시간이나 timeout 측정에는 사용하지 않는다.
	std::uint64_t Get_UnixMilliseconds() noexcept
	{
		using namespace std::chrono;
		return static_cast<std::uint64_t>(duration_cast<milliseconds>(
			system_clock::now().time_since_epoch()).count());
	}
}

// Level·메뉴 요청을 LOAD 단계로 전달한다. 유효성 검사와 중복 예약 거부는 Request에 모은다.
bool_t Client::CLevelTransitionService::Request_Load(
	const LEVEL eTargetLevel,
	const char_t* pSource,
	const LOBBY_COMMAND_TOKEN lobbyCommandToken)
{
	return Request(
		LEVEL_TRANSITION_PHASE::LOAD,
		eTargetLevel,
		pSource,
		lobbyCommandToken);
}

// Loading이 준비 성공을 알리는 ACTIVATE 단계다. 현재 프레임의 Level 객체를 여기서 교체하지 않는다.
bool_t Client::CLevelTransitionService::Request_Activation(
	const LEVEL eTargetLevel,
	const char_t* pSource,
	const LOBBY_COMMAND_TOKEN lobbyCommandToken)
{
	return Request(
		LEVEL_TRANSITION_PHASE::ACTIVATE,
		eTargetLevel,
		pSource,
		lobbyCommandToken);
}

// MainApp이 프레임 경계에서 한 요청을 이동해 받는다. 요청이 없으면 출력 인자를 보존한다.
bool_t Client::CLevelTransitionService::Try_Consume(
	LEVEL_TRANSITION_REQUEST& outRequest)
{
	std::scoped_lock lock{ g_TransitionMutex };
	if (!g_PendingRequest.has_value())
		return false;

	outRequest = std::move(*g_PendingRequest);
	g_PendingRequest.reset();
	g_Status = "Level transition request consumed.";
	return true;
}

// 전환 중 입력을 제한하는 소비자를 위한 읽기 전용 조회다. 반환 후 상태가 바뀔 수 있으므로 요청 시에도 검사한다.
bool_t Client::CLevelTransitionService::Is_Pending()
{
	std::scoped_lock lock{ g_TransitionMutex };
	return g_PendingRequest.has_value();
}

// 전환 직전 초상화 등 표시 준비를 위해 요청을 복사한다. 이후 MainApp의 소비를 위해 원본은 남긴다.
bool_t Client::CLevelTransitionService::Peek_Pending(
	LEVEL_TRANSITION_REQUEST& outRequest)
{
	std::scoped_lock lock{ g_TransitionMutex };
	if (!g_PendingRequest.has_value())
		return false;
	outRequest = *g_PendingRequest;
	return true;
}

// 직전 프레임의 초상화 COM 참조를 보관해 기존 Level 정리 이후에도 Loading이 사용할 수 있게 한다.
void Client::CLevelTransitionService::Set_TransferPortraitSRV(
	ComPtr<ID3D11ShaderResourceView> pSRV)
{
	std::scoped_lock lock{ g_TransitionMutex };
	g_TransferPortraitSRV = std::move(pSRV);
}

// 보관한 COM 참조를 복사해 반환하므로 mutex 해제 이후에도 호출자가 이미지 수명을 유지한다.
ComPtr<ID3D11ShaderResourceView> Client::CLevelTransitionService::Get_TransferPortraitSRV()
{
	std::scoped_lock lock{ g_TransitionMutex };
	return g_TransferPortraitSRV;
}

// Server 확정 참가 목록을 복사한다. 전달한 패킷 객체의 수명과 Loading 표시 상태를 분리한다.
void Client::CLevelTransitionService::Set_ColosseumMatch(
	const LostArk::Shared::S2C_COLOSSEUM_MATCH_FOUND& match)
{
	std::scoped_lock lock{ g_TransitionMutex };
	g_ColosseumMatch = match;
}

// Loading 카드 구성에 필요한 매치 값을 복사한다. 자료가 없으면 출력을 바꾸지 않는다.
bool_t Client::CLevelTransitionService::Try_Get_ColosseumMatch(
	LostArk::Shared::S2C_COLOSSEUM_MATCH_FOUND& outMatch)
{
	std::scoped_lock lock{ g_TransitionMutex };
	if (!g_ColosseumMatch.has_value())
		return false;
	outMatch = *g_ColosseumMatch;
	return true;
}

// 아레나 퇴장 경로가 보관한 참가 목록을 비운다. 초상화 수명은 별도 함수로 관리한다.
void Client::CLevelTransitionService::Clear_ColosseumMatch()
{
	std::scoped_lock lock{ g_TransitionMutex };
	g_ColosseumMatch.reset();
}

// 도구·오류 표시가 사용할 현재 전환 설명을 복사한다. 내부 문자열 참조를 노출하지 않는다.
std::string Client::CLevelTransitionService::Get_Status()
{
	std::scoped_lock lock{ g_TransitionMutex };
	return g_Status;
}

// Loader·활성화 실패를 공통 복구 진단으로 연결한다. 상태 문자열을 실제 로비 화면에서 소비할 수 있게 한다.
void Client::CLevelTransitionService::Report_LoadFailure(
	const HRESULT result,
	const std::string_view detail)
{
	Report_Recovery(
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_LOAD_FAILED,
		"level.loading",
		detail,
		result);
}

// 연결 진단 기록과 로비 표시용 최초 원인을 함께 갱신한다.
// 외부 NetworkManager 호출은 전환 mutex를 얻기 전에 끝내어 두 상태의 잠금을 겹치지 않는다.
void Client::CLevelTransitionService::Report_Recovery(
	const LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
	const std::string_view source,
	const std::string_view detail,
	const HRESULT result)
{
	if (!LostArk::Shared::Is_Known_SessionDiagnosticReason(reason) ||
		source.empty())
	{
		return;
	}

	std::string terminalDetail{ source };
	if (!detail.empty())
		terminalDetail += ": " + std::string{ detail };
	CNetworkManager& network = CNetworkManager::Get();
	network.Record_SessionTerminal(
		reason,
		0,
		LostArk::Shared::PACKET_TYPE::INVALID,
		terminalDetail);
	CLIENT_SESSION_DIAGNOSTIC_SNAPSHOT session =
		network.Get_SessionDiagnosticSnapshot();
	// 로비에 같은 연결 세대의 최초 진단이 있어도 프로세스 기록에는 이후의 모든 복구 보고를 남긴다.
	network.Record_SessionRecovery(reason, source,
		"hresult=" + std::to_string(static_cast<std::uint32_t>(result)) +
		" detail=" + std::string(detail));

	{
		std::scoped_lock lock{ g_TransitionMutex };
		// 최초 종료 보고를 근본 원인으로 유지한다. 이후의 일반 활성화 정리가 로드·전송 오류의 상세를 덮지 않는다.
		if (g_RecoveryDiagnostic.has_value())
			return;

		CLIENT_RECOVERY_DIAGNOSTIC recovery{};
		// 검증한 ROOM_FULL 응답은 수신 worker에서 먼저 관측할 수도 있는 종료 FIN보다 구체적인 원인이다.
		// Session에는 전송 종료 기록을 유지하고 로비에는 검증된 방 정원 초과 사유를 보여 준다.
		recovery.eReason =
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_EXPECTED_ROOM_FULL ==
				reason ? reason :
			(LostArk::Shared::Is_Known_SessionDiagnosticReason(session.eReason) ?
				session.eReason : reason);
		recovery.hResult = result;
		recovery.strSource.assign(source);
		recovery.strDetail.assign(detail);
		recovery.iOccurredUnixMs = 0u != session.iTerminalUnixMs ?
			session.iTerminalUnixMs : Get_UnixMilliseconds();
		recovery.Session = std::move(session);
		g_RecoveryDiagnostic = std::move(recovery);
		g_Status = "Lobby recovery recorded by " + std::string{ source } + ".";
	}
}

// 현재 연결 snapshot에서 원인을 선택해 공통 보고 경로로 전달한다. 이 함수 자체가 연결을 닫지는 않는다.
void Client::CLevelTransitionService::Report_NetworkRecovery(
	const std::string_view source,
	const std::string_view detail)
{
	const CLIENT_SESSION_DIAGNOSTIC_SNAPSHOT session =
		CNetworkManager::Get().Get_SessionDiagnosticSnapshot();
	const LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason =
		LostArk::Shared::Is_Known_SessionDiagnosticReason(session.eReason) ?
			session.eReason :
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_CONNECTION_LOST;
	Report_Recovery(reason, source, detail);
}

// 로비가 최초 복구 원인을 한 번 가져간다. 최신 snapshot은 같은 연결 세대일 때만 합친다.
bool_t Client::CLevelTransitionService::Try_ConsumeRecovery(
	CLIENT_RECOVERY_DIAGNOSTIC& outDiagnostic)
{
	const CLIENT_SESSION_DIAGNOSTIC_SNAPSHOT latestSession =
		CNetworkManager::Get().Get_SessionDiagnosticSnapshot();
	std::scoped_lock lock{ g_TransitionMutex };
	if (!g_RecoveryDiagnostic.has_value())
		return false;
	if (latestSession.iConnectionGeneration ==
		g_RecoveryDiagnostic->Session.iConnectionGeneration)
	{
		g_RecoveryDiagnostic->Session = latestSession;
	}
	outDiagnostic = std::move(*g_RecoveryDiagnostic);
	g_RecoveryDiagnostic.reset();
	return true;
}

// 호출자가 실패 HRESULT와 detail만 요구하는 호환 경로다. 성공 HRESULT 진단은 소비하지 않는다.
bool_t Client::CLevelTransitionService::Try_ConsumeLoadFailure(
	HRESULT& outResult,
	std::string& outDetail)
{
	std::scoped_lock lock{ g_TransitionMutex };
	if (!g_RecoveryDiagnostic.has_value() ||
		!FAILED(g_RecoveryDiagnostic->hResult))
		return false;

	outResult = g_RecoveryDiagnostic->hResult;
	outDetail = std::move(g_RecoveryDiagnostic->strDetail);
	g_RecoveryDiagnostic.reset();
	return true;
}

// Loading이 이전 월드의 종류에 맞는 표시를 선택할 때 출발 Level을 복사해 받는다.
LEVEL Client::CLevelTransitionService::Get_LastWorldTransferOrigin()
{
	std::scoped_lock lock{ g_TransitionMutex };
	return g_LastWorldTransferOrigin;
}

// 현재 Level의 갱신이 승인 응답을 확인한다. 응답 없음은 무변경, 유효한 다른 월드는 LOAD 예약으로 이어진다.
// 이미 소비한 승인으로 전환을 예약할 수 없으면 진단을 보존하고 연결을 닫은 뒤 로비 복구를 요청한다.
Client::SERVER_WORLD_TRANSFER_PUMP_RESULT
Client::CLevelTransitionService::Pump_ServerApprovedWorldTransfer(
	const LEVEL currentLevel)
{
	using namespace LostArk::Shared;
	CNetworkManager& networkManager = CNetworkManager::Get();
	S2C_ENTER_ACCEPTED accepted{};
	if (!networkManager.Try_Consume_EnterAccepted(accepted))
		return SERVER_WORLD_TRANSFER_PUMP_RESULT::NONE;

	// 프로토콜·플레이어·entity 식별자를 확인한 응답만 지원 월드에 매핑한다. 미지원 값은 END로 남긴다.
	LEVEL targetLevel = LEVEL::END;
	if (NETWORK_PROTOCOL_VERSION == accepted.iProtocolVersion &&
		INVALID_PLAYER_ID != accepted.iPlayerId &&
		INVALID_NET_ENTITY_ID != accepted.iNetEntityId)
	{
		switch (accepted.eWorldId)
		{
		case WORLD_ID::BERN:
			targetLevel = LEVEL::BERN;
			break;
		case WORLD_ID::VALTAN_ARENA:
			targetLevel = LEVEL::VALTAN_ARENA;
			break;
		case WORLD_ID::KOUKUSAYDON_ARENA:
			targetLevel = LEVEL::KOUKUSAYDON_ARENA;
			break;
		case WORLD_ID::MAHARAKA:
			targetLevel = LEVEL::MAHARAKA;
			break;
		case WORLD_ID::COLOSSEUM:
			targetLevel = LEVEL::COLOSSEUM;
			break;
		default:
			break;
		}
	}

	if (LEVEL::END != targetLevel && targetLevel != currentLevel &&
		Request_Load(targetLevel, "server.trigger.change-level"))
	{
		std::scoped_lock lock{ g_TransitionMutex };
		g_LastWorldTransferOrigin = currentLevel;
		return SERVER_WORLD_TRANSFER_PUMP_RESULT::REQUESTED;
	}

	Report_Recovery(
		SESSION_DIAGNOSTIC_REASON::CLIENT_WORLD_TRANSFER_FAILED,
		"server.trigger.change-level-recovery",
		"Accepted transfer target was invalid, unchanged, or could not be staged.");
	networkManager.Close_ServerConnection();
	Request_Load(LEVEL::LOBBY, "server.trigger.change-level-recovery");
	return SERVER_WORLD_TRANSFER_PUMP_RESULT::RECOVERY_REQUESTED;
}

// 등록 대상과 source·로비 토큰을 검증하고 빈 예약 슬롯에만 요청을 넣는다.
// 거부하면 기존 요청·복구 진단을 유지하고 g_Status만 거부 이유로 바꾼다.
bool_t Client::CLevelTransitionService::Request(
	const LEVEL_TRANSITION_PHASE ePhase,
	const LEVEL eTargetLevel,
	const char_t* pSource,
	const LOBBY_COMMAND_TOKEN lobbyCommandToken)
{
	const bool_t hasLobbyCommand =
		INVALID_LOBBY_COMMAND_TOKEN != lobbyCommandToken;
	if (nullptr == CLevelRegistry::Find(eTargetLevel) ||
		nullptr == pSource || '\0' == *pSource ||
		(hasLobbyCommand && LEVEL::LOBBY != eTargetLevel))
	{
		std::scoped_lock lock{ g_TransitionMutex };
		g_Status = "Rejected invalid level transition request.";
		return false;
	}

	std::scoped_lock lock{ g_TransitionMutex };
	if (g_PendingRequest.has_value())
	{
		g_Status =
			"Rejected level transition while another request is pending.";
		return false;
	}

	g_PendingRequest = LEVEL_TRANSITION_REQUEST{
		ePhase,
		eTargetLevel,
		pSource,
		lobbyCommandToken
	};
	// 새 요청은 이전 전환의 출발지를 지운다. Server 전환 pump는 자기 요청 예약 성공 직후 다시 설정한다.
	g_LastWorldTransferOrigin = LEVEL::END;
	// 대상 Level 실패 보고는 로비 복구 요청보다 먼저 들어온다.
	// 로비 요청에서 진단을 비우면 CLevel_Lobby가 원인을 읽지 못하므로 유지한다.
	// 로비 외 새 LOAD만 독립된 재시도로 보고 이전 진단을 비운다.
	if (LEVEL_TRANSITION_PHASE::LOAD == ePhase &&
		LEVEL::LOBBY != eTargetLevel)
	{
		g_RecoveryDiagnostic.reset();
	}
	g_Status = "Level transition request staged by " +
		g_PendingRequest->strSource + ".";
	return true;
}

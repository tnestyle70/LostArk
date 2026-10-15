#pragma once

// 전환 대상 LEVEL과 Client 공통 타입을 사용한다.
#include "Client_Defines.h"
// 복구 당시 연결 세대와 최초 종료 원인 snapshot을 값으로 보존한다.
#include "ClientSessionDiagnostic.h"
// HRESULT와 초상화 SRV의 COM 소유권 타입을 사용한다.
#include "Engine_Defines.h"
// 로비 명령과 전환을 연결하는 토큰 및 무효값을 사용한다.
#include "LobbyCommandService.h"
// Server가 승인한 콜로세움 참가 목록을 원래 패킷 값으로 보관한다.
#include "Network/PacketMessages.h"

// 요청·진단 문자열을 호출자 수명과 분리해 소유한다.
#include <string>
// 진단 입력은 호출 동안 차용하고 필요한 상태에만 복사한다.
#include <string_view>

NS_BEGIN(Client)

// LOAD는 Loading 진입 준비, ACTIVATE는 로드 완료 후 실제 대상 Level 생성 요청이다.
enum class LEVEL_TRANSITION_PHASE
{
	LOAD,
	ACTIVATE
};

// 한 번에 하나만 대기하는 전환 값이다. MainApp이 현재 Level 갱신을 마친 뒤 소비한다.
struct LEVEL_TRANSITION_REQUEST final
{
	LEVEL_TRANSITION_PHASE ePhase = LEVEL_TRANSITION_PHASE::LOAD;
	LEVEL eTargetLevel = LEVEL::END;
	std::string strSource;
	LOBBY_COMMAND_TOKEN iLobbyCommandToken =
		INVALID_LOBBY_COMMAND_TOKEN;
};

// 로비가 표시할 최초 복구 원인을 소유한다. 시각은 Unix epoch 기준 ms이며 Session은 연결 세대의 snapshot이다.
struct CLIENT_RECOVERY_DIAGNOSTIC final
{
	LostArk::Shared::SESSION_DIAGNOSTIC_REASON eReason =
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON::NONE;
	HRESULT hResult = S_OK;
	std::string strSource;
	std::string strDetail;
	std::uint64_t iOccurredUnixMs = 0u;
	CLIENT_SESSION_DIAGNOSTIC_SNAPSHOT Session;
};

// 승인 메시지 없음, 대상 로드 예약 성공, 실패 후 로비 복구 요청을 호출자에게 구분해 알린다.
enum class SERVER_WORLD_TRANSFER_PUMP_RESULT
{
	NONE,
	REQUESTED,
	RECOVERY_REQUESTED
};

// Level·Loader·네트워크가 실제 Change_Level을 직접 호출하지 않고 전환을 예약하는 공용 경계다.
// 대기 요청·진단·초상화·매치 상태는 CPP의 mutex로 보호하며 실제 Level 교체는 MainApp만 수행한다.
class CLevelTransitionService final
{
public:
	// 호출자가 Loading 진입을 예약한다. 잘못된 대상이나 기존 대기 요청이 있으면 기존 요청을 유지하고 false다.
	static bool_t Request_Load(
		LEVEL eTargetLevel,
		const char_t* pSource,
		LOBBY_COMMAND_TOKEN lobbyCommandToken =
			INVALID_LOBBY_COMMAND_TOKEN);
	// 로드가 완료된 Level이 활성화를 예약한다. 자원을 다시 읽거나 현재 Level을 즉시 바꾸지는 않는다.
	static bool_t Request_Activation(
		LEVEL eTargetLevel,
		const char_t* pSource,
		LOBBY_COMMAND_TOKEN lobbyCommandToken =
			INVALID_LOBBY_COMMAND_TOKEN);
	// MainApp이 대기 요청을 한 번 이동해 받는다. 없으면 false이고 outRequest는 바꾸지 않는다.
	static bool_t Try_Consume(LEVEL_TRANSITION_REQUEST& outRequest);
	// 입력·전환 요청자가 현재 예약 여부를 검사한다. 조회와 이후 요청 사이의 경쟁은 Request가 다시 검사한다.
	static bool_t Is_Pending();
	// 현재 Level의 표시 준비가 예약된 목적지를 조회한다. 비동기 준비 취소를 기다리는 동안 여러 Render에서 같은 요청을 볼 수 있다.
	// 요청을 소비하지 않고 복사하며, 없으면 outRequest를 보존하고 false를 반환한다.
	static bool_t Peek_Pending(LEVEL_TRANSITION_REQUEST& outRequest);
	// 베른을 떠나기 직전 그린 로컬 캐릭터 초상화의 COM 참조를 보관한다.
	// 원래 캐릭터가 정리된 뒤에도 Loading이 사용하며 nullptr은 참조를 비워 2D 그림을 유지하게 한다.
	static void Set_TransferPortraitSRV(ComPtr<ID3D11ShaderResourceView> pSRV);
	static ComPtr<ID3D11ShaderResourceView> Get_TransferPortraitSRV();
	// 베른에서 받은 Server 확정 참가 목록을 월드 전환 전에 복사한다.
	// Loading은 이 목록으로 카드 수를 정하며 표시 준비 경계를 지나서도 유지하고 아레나 퇴장 때 비운다.
	static void Set_ColosseumMatch(const LostArk::Shared::S2C_COLOSSEUM_MATCH_FOUND& match);
	static bool_t Try_Get_ColosseumMatch(LostArk::Shared::S2C_COLOSSEUM_MATCH_FOUND& outMatch);
	static void Clear_ColosseumMatch();
	static std::string Get_Status();
	// 실패한 준비 단계의 detail을 복구 진단으로 기록한다.
	// 이미 복구 진단이 있으면 후속 보고가 덮어쓰지 않아 최초 로드·연결 실패 이유를 보존한다.
	static void Report_LoadFailure(
		HRESULT result,
		std::string_view detail = {});
	// 유효한 원인과 source를 네트워크 기록에 남긴다. 로비 표시용 상태는 최초 보고만 유지한다.
	static void Report_Recovery(
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
		std::string_view source,
		std::string_view detail = {},
		HRESULT result = S_OK);
	// 현재 연결의 종료 원인을 우선 사용하며 알려진 원인이 없으면 연결 끊김으로 보고한다.
	static void Report_NetworkRecovery(
		std::string_view source,
		std::string_view detail = {});
	// 로비가 진단을 한 번 가져간다. 같은 연결 세대의 최신 snapshot만 합친 뒤 보관 상태를 비운다.
	static bool_t Try_ConsumeRecovery(
		CLIENT_RECOVERY_DIAGNOSTIC& outDiagnostic);
	// 실패 HRESULT를 가진 진단만 한 번 소비한다. 없거나 성공 HRESULT면 출력과 진단을 유지한다.
	static bool_t Try_ConsumeLoadFailure(
		HRESULT& outResult,
		std::string& outDetail);
	// 현재 월드가 Server 승인 응답을 소비해 다음 Level을 예약한다. 무효·중복 대상이면 연결을 닫고 로비 복구를 요청한다.
	static SERVER_WORLD_TRANSFER_PUMP_RESULT
		Pump_ServerApprovedWorldTransfer(LEVEL currentLevel);
	// 마지막 Server 승인 전환의 출발 Level이다. 일반 요청이 새로 들어오면 LEVEL::END로 초기화한다.
	// Loading이 마하라카에서 베른으로 복귀하는 경우를 일반 베른 입장과 구분할 때 사용한다.
	static LEVEL Get_LastWorldTransferOrigin();

private:
	static bool_t Request(
		LEVEL_TRANSITION_PHASE ePhase,
		LEVEL eTargetLevel,
		const char_t* pSource,
		LOBBY_COMMAND_TOKEN lobbyCommandToken);
};

NS_END

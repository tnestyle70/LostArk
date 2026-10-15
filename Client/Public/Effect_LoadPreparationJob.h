#pragma once

//Engine·Client namespace 표기 매크로를 직접 사용한다.
#include "Engine_Macro.h"

//condition_variable·mutex로 통신 상태를 보호하고 chrono로 단계 경과 시간을 잰다.
#include <condition_variable>
#include <chrono>
//size_t는 큐 크기, 고정 폭 정수는 세대·개수·상태 값에 사용한다.
#include <cstddef>
#include <cstdint>
//deque는 결과 FIFO, optional은 명령·실패 유무, shared_ptr는 payload 수명을 보관한다.
#include <deque>
#include <memory>
#include <mutex>
#include <optional>
//string·vector는 상태 문구와 대상 asset ID 목록을 소유한다.
#include <string>
#include <vector>

NS_BEGIN(Client)

//uint64_t의 별칭이다. 서비스가 발급한 작업 세대를 구분하며 0은 무효다. 시간값이 아니다.
using EFFECT_LOAD_JOB_EPOCH = uint64_t;

//미소비 결과를 두 개까지만 보관한다. 가득 차면 생산 worker가 기다린다.
constexpr size_t EFFECT_LOAD_RESULT_CHANNEL_CAPACITY = 2u;

//메인 스레드가 worker에 보내는 명령이다.
enum class EFFECT_LOAD_JOB_COMMAND_KIND : uint8_t
{
	ACCEPT_DISCOVERY, //검색 결과를 승인하고 대상 준비를 시작한다.
	REBASE, //더 새 작업 세대와 대상으로 교체한다.
	TARGET_COMMIT_ACK, //한 대상의 메인 스레드 반영이 끝났음을 알린다.
	CLOSE, //현재 작업의 정상 종료를 알린다.
	CANCEL, //작업을 취소하고 미소비 결과를 버린다.
	END //유효한 명령이 없는 기본값이다.
};

//worker가 메인 스레드에 전달하는 결과 종류다.
enum class EFFECT_LOAD_JOB_RESULT_KIND : uint8_t
{
	DISCOVERY, //준비할 대상을 검색한 결과다.
	TARGET_STAGED, //한 대상의 임시 준비 결과 또는 격리 가능한 실패다.
	EPOCH_STAGE_COMPLETE, //현재 작업 세대의 대상 준비를 마쳤다.
	STRUCTURAL_FAILURE, //개별 대상 격리로 계속할 수 없는 실패다.
	END //유효한 결과가 없는 기본값이다.
};

//worker의 결과 제출이 수락됐는지, 중단됐는지 구분한다.
enum class EFFECT_LOAD_RESULT_PUSH_RESULT : uint8_t
{
	PUSHED, //결과 큐에 소유권을 넘겼다.
	CANCELLED, //현재 작업이 취소됐다.
	REBASED, //결과의 작업 세대나 카탈로그 세대가 현재 값과 다르다.
	CLOSED, //현재 작업이 닫혔다.
	INVALID, //결과 형식이 잘못됐거나 아직 열지 않았다.
	END //미지정 값이다.
};

//명령 슬롯에 넣은 결과다. 교체된 명령은 호출자에게 돌려준다.
enum class EFFECT_LOAD_MAILBOX_POST_RESULT : uint8_t
{
	POSTED, //빈 슬롯에 명령을 넣었다.
	REPLACED, //미소비 명령을 새 명령으로 교체했다.
	CANCELLED, //이미 취소돼 명령을 받지 않는다.
	CLOSED, //이미 닫혀 명령을 받지 않는다.
	INVALID, //명령 형식·세대 또는 열린 상태가 맞지 않는다.
	END //미지정 값이다.
};

//명령을 기다린 worker가 깨어난 이유다.
enum class EFFECT_LOAD_MAILBOX_WAIT_RESULT : uint8_t
{
	COMMAND, //슬롯의 명령을 가져왔다.
	CANCELLED, //명령 없이 취소 상태를 확인했다.
	CLOSED, //명령 없이 종료 상태를 확인했다.
	END //미지정 값이다.
};

//로딩 화면에 표시하는 단계다. 실패 원인은 별도 receipt가 보관한다.
enum class EFFECT_LOAD_PROGRESS_PHASE : uint8_t
{
	IDLE, //열었지만 준비를 시작하지 않았다.
	DISCOVERY, //준비할 대상을 검색한다.
	WAITING_FOR_DISCOVERY_ACCEPT, //메인 스레드의 대상 승인을 기다린다.
	TARGET_STAGE, //승인된 대상을 임시 준비한다.
	EPOCH_STAGE_COMPLETE, //현재 세대의 준비를 마쳤다.
	FAILED, //준비 실패를 표시한다.
	CANCELLED, //취소 상태를 표시한다.
	CLOSED, //종료 상태를 표시한다.
	END //미지정 값이다.
};

//실패가 난 세대·대상·최초 오류를 값으로 보관한다. 취소 후에도 원인을 조회한다.
//catalog revision은 성공적으로 로드한 카탈로그의 세대이며 파일 포맷 버전이 아니다.
struct EFFECT_LOAD_FAILURE_RECEIPT final
{
	EFFECT_LOAD_JOB_EPOCH iJobEpoch = 0u;
	uint64_t iCatalogRevision = 0u; //입력 카탈로그 세대다.
	std::string strEffectAssetId;
	int64_t iRootCode = 0;
	std::string strRootMessage;

	bool Is_Valid() const;
};

//메인 스레드 → worker 명령이다. payload의 수명은 shared_ptr로 함께 유지한다.
//구체 타입은 서비스가 알고 세대와 종류를 검증한 뒤 변환한다. 이 통신부는 렌더러·D3D를 모른다.
struct EFFECT_LOAD_JOB_COMMAND final
{
	EFFECT_LOAD_JOB_COMMAND_KIND eKind =
		EFFECT_LOAD_JOB_COMMAND_KIND::END;
	EFFECT_LOAD_JOB_EPOCH iJobEpoch = 0u;
	uint64_t iCatalogRevision = 0u; //입력 카탈로그 세대다.
	std::vector<std::string> EffectAssetIds;
	std::shared_ptr<const void> pImmutablePayload; //읽기 전용 자료의 공유 소유 참조다.

	bool Is_Valid() const;

	//아래 함수는 메시지만 만든다. 전송과 새 epoch 발급은 호출자가 맡는다.
	static EFFECT_LOAD_JOB_COMMAND Accept_Discovery(
		EFFECT_LOAD_JOB_EPOCH iJobEpoch,
		uint64_t iCatalogRevision,
		std::vector<std::string> EffectAssetIds,
		std::shared_ptr<const void> pImmutablePayload = nullptr);
	static EFFECT_LOAD_JOB_COMMAND Rebase(
		EFFECT_LOAD_JOB_EPOCH iJobEpoch,
		uint64_t iCatalogRevision,
		std::vector<std::string> EffectAssetIds,
		std::shared_ptr<const void> pImmutablePayload = nullptr);
	static EFFECT_LOAD_JOB_COMMAND Target_CommitAck(
		EFFECT_LOAD_JOB_EPOCH iJobEpoch,
		uint64_t iCatalogRevision,
		std::string strEffectAssetId);
	static EFFECT_LOAD_JOB_COMMAND Close(
		EFFECT_LOAD_JOB_EPOCH iJobEpoch,
		uint64_t iCatalogRevision);
	static EFFECT_LOAD_JOB_COMMAND Cancel(
		EFFECT_LOAD_JOB_EPOCH iJobEpoch,
		uint64_t iCatalogRevision);
};

//worker → 메인 스레드 결과다. 임시 준비 자료의 수명을 반영·실패 처리까지 함께 유지한다.
struct EFFECT_LOAD_JOB_RESULT final
{
	EFFECT_LOAD_JOB_RESULT_KIND eKind = EFFECT_LOAD_JOB_RESULT_KIND::END;
	EFFECT_LOAD_JOB_EPOCH iJobEpoch = 0u;
	uint64_t iCatalogRevision = 0u; //입력 카탈로그 세대다.
	std::string strEffectAssetId;
	std::shared_ptr<const void> pImmutablePayload; //읽기 전용 자료의 공유 소유 참조다.
	std::optional<EFFECT_LOAD_FAILURE_RECEIPT> Failure;

	bool Is_Valid() const;
};

//잠금 안에서 복사해 로딩 화면에 전달한다. 실제 작업·오류의 소유 상태와 구분한다.
struct EFFECT_LOAD_PROGRESS_SNAPSHOT final
{
	EFFECT_LOAD_JOB_EPOCH iJobEpoch = 0u;
	uint64_t iCatalogRevision = 0u; //입력 카탈로그 세대다.
	EFFECT_LOAD_PROGRESS_PHASE ePhase = EFFECT_LOAD_PROGRESS_PHASE::IDLE;
	bool bDeterminate = false; //전체 개수가 정해졌을 때만 completed/total을 표시한다.
	uint32_t iCompleted = 0u;
	uint32_t iTotal = 0u;
	std::string strCurrentId;
	std::string strStatus;
	uint64_t iElapsedMilliseconds = 0u; //현재 단계가 시작된 뒤 지난 밀리초다.
	uint32_t iIsolatedFailureCount = 0u; //전체 작업 중단 없이 격리한 대상 실패 수다.
};

//Loader worker와 메인 스레드가 공유하는 통신·진행 상태다. 스레드를 만들거나 작업을 배분하지 않는다.
//결과는 worker → 메인 FIFO, 명령은 메인 → worker 단일 슬롯으로 전달한다.
//메인은 결과를 기다리지 않는다. worker는 TARGET_STAGED 뒤 해당 대상의 ACK를 기다린다.
//별도 준비 작업을 병렬 수행해도 합류 뒤 이 생산 worker 하나가 결과를 순서대로 제출한다.
class CEffectLoadPreparationJob final
{
public:
	CEffectLoadPreparationJob() = default;
	CEffectLoadPreparationJob(const CEffectLoadPreparationJob&) = delete;
	CEffectLoadPreparationJob& operator=(
		const CEffectLoadPreparationJob&) = delete;

	//Loader가 유효한 두 세대로 연다. 이전 종료·취소 상태를 초기화한다.
	bool Open(
		EFFECT_LOAD_JOB_EPOCH iInitialJobEpoch,
		uint64_t iCatalogRevision,
		std::string& strOutStatus);

	//메인이 명령을 넣고 기존 미소비 명령은 OutDisplacedCommand로 돌려받는다.
	EFFECT_LOAD_MAILBOX_POST_RESULT Post_Command(
		EFFECT_LOAD_JOB_COMMAND Command,
		std::optional<EFFECT_LOAD_JOB_COMMAND>& OutDisplacedCommand,
		std::string& strOutStatus);
	//worker가 명령 또는 종료 신호까지 기다린다. 기다리는 동안 잠금을 놓는다.
	EFFECT_LOAD_MAILBOX_WAIT_RESULT Wait_Pop_Command(
		EFFECT_LOAD_JOB_COMMAND& OutCommand);

	//worker가 결과를 이동한다. 큐가 찼으면 빈자리·취소·종료·세대 교체까지 기다린다.
	EFFECT_LOAD_RESULT_PUSH_RESULT Push_Result_Wait(
		EFFECT_LOAD_JOB_RESULT Result);
	//메인이 가장 오래된 결과를 가져온다. 큐가 비면 대기 없이 false다.
	bool Try_Pop_Result(EFFECT_LOAD_JOB_RESULT& OutResult);

	//최초 구조적 실패만 기록한다. 취소·종료 뒤에도 원인을 남기며 진행 문구로 덮어쓰지 않는다.
	bool Record_FirstFailure(EFFECT_LOAD_FAILURE_RECEIPT Failure);
	std::optional<EFFECT_LOAD_FAILURE_RECEIPT> Get_FirstFailure() const;

	//현재 세대의 진행값만 반영한다. 같은 단계·총개수에서 완료 수의 역행을 거부한다.
	bool Publish_Progress(const EFFECT_LOAD_PROGRESS_SNAPSHOT& Progress);
	//현재 단계 경과 시간을 갱신한 복사본을 반환한다.
	EFFECT_LOAD_PROGRESS_SNAPSHOT Get_Progress() const;

	//아래 조회는 각각 잠금 안에서 현재 상태를 복사한다.
	EFFECT_LOAD_JOB_EPOCH Get_CurrentEpoch() const;
	uint64_t Get_CurrentCatalogRevision() const;
	size_t Get_PendingResultCount() const;
	bool Is_Cancelled() const;
	bool Is_Closed() const;

private:
	//명령·결과·세대·진행 상태를 하나의 잠금으로 함께 보호한다.
	mutable std::mutex m_Mutex;
	//결과 큐의 빈자리 또는 작업 종료를 기다리는 worker를 깨운다.
	std::condition_variable m_ResultSpaceCondition;
	//새 명령 또는 작업 종료를 기다리는 worker를 깨운다.
	std::condition_variable m_MailboxCondition;
	//최대 두 결과를 FIFO로 소유한다. 소비 시 호출자에게 이동한다.
	std::deque<EFFECT_LOAD_JOB_RESULT> m_Results;
	//최신 미소비 명령 하나를 소유한다. 무제한 명령 큐가 아니다.
	std::optional<EFFECT_LOAD_JOB_COMMAND> m_Mailbox;
	EFFECT_LOAD_PROGRESS_SNAPSHOT m_Progress;
	//최초 구조적 실패를 보존한다. Open 또는 REBASE만 비운다.
	std::optional<EFFECT_LOAD_FAILURE_RECEIPT> m_FirstFailure;
	//표시용 경과 시간 기준이다. 작업 식별자인 epoch와 별개다.
	std::chrono::steady_clock::time_point m_ProgressPhaseStarted =
		std::chrono::steady_clock::now();
	//현재 작업과 입력 카탈로그의 세대다. 결과·명령의 오래된 pair를 걸러낸다.
	EFFECT_LOAD_JOB_EPOCH m_iCurrentJobEpoch = 0u;
	uint64_t m_iCurrentCatalogRevision = 0u;
	//Open 여부와 취소·정상 종료를 분리해 대기 해제 이유를 보존한다.
	bool m_bOpen = false;
	bool m_bCancelled = false;
	bool m_bClosed = false;
};

NS_END

//메시지 검증과 공유 통신 상태를 구현한다. 실제 준비 작업은 PresentationService가 수행한다.
#include "Effect_LoadPreparationJob.h"

//빈 asset ID 검사와 메시지·payload의 소유권 이동에 사용한다.
#include <algorithm>
#include <utility>

namespace
{

//명령 대상에 빈 ID가 섞이면 구체 자료를 읽기 전에 거부한다.
bool Has_EmptyEffectId(const std::vector<std::string>& EffectAssetIds)
{
	return std::any_of(
		EffectAssetIds.begin(), EffectAssetIds.end(),
		[](const std::string& EffectAssetId)
		{
			return EffectAssetId.empty();
		});
}

}

NS_BEGIN(Client)

//원인 추적에 필요한 세대·오류 코드·문구가 있는지 검사한다. 대상 ID는 없어도 된다.
bool EFFECT_LOAD_FAILURE_RECEIPT::Is_Valid() const
{
	return 0u != iJobEpoch && 0u != iCatalogRevision &&
		0 != iRootCode && !strRootMessage.empty();
}

//명령마다 필요한 대상 수와 payload 유무를 검사한다.
bool EFFECT_LOAD_JOB_COMMAND::Is_Valid() const
{
	if (EFFECT_LOAD_JOB_COMMAND_KIND::END == eKind ||
		0u == iJobEpoch || 0u == iCatalogRevision ||
		Has_EmptyEffectId(EffectAssetIds))
	{
		return false;
	}
	switch (eKind)
	{
	case EFFECT_LOAD_JOB_COMMAND_KIND::ACCEPT_DISCOVERY:
	case EFFECT_LOAD_JOB_COMMAND_KIND::REBASE:
		return !EffectAssetIds.empty() && nullptr != pImmutablePayload;
	case EFFECT_LOAD_JOB_COMMAND_KIND::TARGET_COMMIT_ACK:
		return 1u == EffectAssetIds.size() && nullptr == pImmutablePayload;
	case EFFECT_LOAD_JOB_COMMAND_KIND::CLOSE:
	case EFFECT_LOAD_JOB_COMMAND_KIND::CANCEL:
		return EffectAssetIds.empty() && nullptr == pImmutablePayload;
	default:
		return false;
	}
}

//서비스가 승인한 대상과 읽기 전용 준비 자료를 메시지로 이동한다.
EFFECT_LOAD_JOB_COMMAND
EFFECT_LOAD_JOB_COMMAND::Accept_Discovery(
	const EFFECT_LOAD_JOB_EPOCH iJobEpoch,
	const uint64_t iCatalogRevision,
	std::vector<std::string> EffectAssetIds,
	std::shared_ptr<const void> pImmutablePayload)
{
	EFFECT_LOAD_JOB_COMMAND Command;
	Command.eKind = EFFECT_LOAD_JOB_COMMAND_KIND::ACCEPT_DISCOVERY;
	Command.iJobEpoch = iJobEpoch;
	Command.iCatalogRevision = iCatalogRevision;
	Command.EffectAssetIds = std::move(EffectAssetIds);
	Command.pImmutablePayload = std::move(pImmutablePayload);
	return Command;
}

//더 새 세대로 바꿀 메시지를 만든다. 세대의 증가 여부는 Post_Command가 확인한다.
EFFECT_LOAD_JOB_COMMAND EFFECT_LOAD_JOB_COMMAND::Rebase(
	const EFFECT_LOAD_JOB_EPOCH iJobEpoch,
	const uint64_t iCatalogRevision,
	std::vector<std::string> EffectAssetIds,
	std::shared_ptr<const void> pImmutablePayload)
{
	EFFECT_LOAD_JOB_COMMAND Command;
	Command.eKind = EFFECT_LOAD_JOB_COMMAND_KIND::REBASE;
	Command.iJobEpoch = iJobEpoch;
	Command.iCatalogRevision = iCatalogRevision;
	Command.EffectAssetIds = std::move(EffectAssetIds);
	Command.pImmutablePayload = std::move(pImmutablePayload);
	return Command;
}

//메인이 반영을 끝낸 대상 하나를 worker에 알릴 메시지를 만든다.
EFFECT_LOAD_JOB_COMMAND
EFFECT_LOAD_JOB_COMMAND::Target_CommitAck(
	const EFFECT_LOAD_JOB_EPOCH iJobEpoch,
	const uint64_t iCatalogRevision,
	std::string strEffectAssetId)
{
	EFFECT_LOAD_JOB_COMMAND Command;
	Command.eKind = EFFECT_LOAD_JOB_COMMAND_KIND::TARGET_COMMIT_ACK;
	Command.iJobEpoch = iJobEpoch;
	Command.iCatalogRevision = iCatalogRevision;
	Command.EffectAssetIds.push_back(std::move(strEffectAssetId));
	return Command;
}

//정상 종료할 작업 세대와 카탈로그 세대를 묶는다.
EFFECT_LOAD_JOB_COMMAND EFFECT_LOAD_JOB_COMMAND::Close(
	const EFFECT_LOAD_JOB_EPOCH iJobEpoch,
	const uint64_t iCatalogRevision)
{
	EFFECT_LOAD_JOB_COMMAND Command;
	Command.eKind = EFFECT_LOAD_JOB_COMMAND_KIND::CLOSE;
	Command.iJobEpoch = iJobEpoch;
	Command.iCatalogRevision = iCatalogRevision;
	return Command;
}

//취소할 작업 세대를 묶는다. 실제 취소 상태는 명령 게시 시 바뀐다.
EFFECT_LOAD_JOB_COMMAND EFFECT_LOAD_JOB_COMMAND::Cancel(
	const EFFECT_LOAD_JOB_EPOCH iJobEpoch,
	const uint64_t iCatalogRevision)
{
	EFFECT_LOAD_JOB_COMMAND Command;
	Command.eKind = EFFECT_LOAD_JOB_COMMAND_KIND::CANCEL;
	Command.iJobEpoch = iJobEpoch;
	Command.iCatalogRevision = iCatalogRevision;
	return Command;
}

//결과 종류와 payload·실패 자료가 맞는지 확인하고 다른 세대의 실패가 섞이지 않게 한다.
bool EFFECT_LOAD_JOB_RESULT::Is_Valid() const
{
	if (EFFECT_LOAD_JOB_RESULT_KIND::END == eKind ||
		0u == iJobEpoch || 0u == iCatalogRevision)
	{
		return false;
	}
	if (Failure.has_value())
	{
		if (!Failure->Is_Valid() || Failure->iJobEpoch != iJobEpoch ||
			Failure->iCatalogRevision != iCatalogRevision ||
			(!Failure->strEffectAssetId.empty() &&
			 Failure->strEffectAssetId != strEffectAssetId))
		{
			return false;
		}
	}
	//TARGET_STAGED는 성공 payload와 격리 실패 중 정확히 하나만 가진다.
	switch (eKind)
	{
	case EFFECT_LOAD_JOB_RESULT_KIND::DISCOVERY:
		return strEffectAssetId.empty() && nullptr != pImmutablePayload &&
			!Failure.has_value();
	case EFFECT_LOAD_JOB_RESULT_KIND::TARGET_STAGED:
		return !strEffectAssetId.empty() &&
			((nullptr != pImmutablePayload) != Failure.has_value());
	case EFFECT_LOAD_JOB_RESULT_KIND::EPOCH_STAGE_COMPLETE:
		return strEffectAssetId.empty() && nullptr == pImmutablePayload &&
			!Failure.has_value();
	case EFFECT_LOAD_JOB_RESULT_KIND::STRUCTURAL_FAILURE:
		return nullptr == pImmutablePayload && Failure.has_value();
	default:
		return false;
	}
}

//Loader가 사용할 통신 상태를 연다. 진행 중인 작업은 덮어쓰지 않는다.
bool CEffectLoadPreparationJob::Open(
	const EFFECT_LOAD_JOB_EPOCH iInitialJobEpoch,
	const uint64_t iCatalogRevision,
	std::string& strOutStatus)
{
	if (0u == iInitialJobEpoch || 0u == iCatalogRevision)
	{
		strOutStatus = "Effect load job epoch or catalog revision is invalid.";
		return false;
	}

	std::lock_guard<std::mutex> Lock(m_Mutex);
	if (m_bOpen && !m_bCancelled && !m_bClosed)
	{
		strOutStatus = "Effect load job is already open.";
		return false;
	}
	//새 작업에 사용할 통신 자료와 최초 실패를 초기화한다.
	m_Results.clear();
	m_Mailbox.reset();
	m_FirstFailure.reset();
	m_Progress = {};
	m_Progress.iJobEpoch = iInitialJobEpoch;
	m_Progress.iCatalogRevision = iCatalogRevision;
	m_ProgressPhaseStarted = std::chrono::steady_clock::now();
	m_iCurrentJobEpoch = iInitialJobEpoch;
	m_iCurrentCatalogRevision = iCatalogRevision;
	m_bOpen = true;
	m_bCancelled = false;
	m_bClosed = false;
	strOutStatus = "Effect load preparation job opened.";
	return true;
}

//메인 스레드의 명령을 검증하고 단일 슬롯에 게시한 뒤 worker의 대기를 푼다.
EFFECT_LOAD_MAILBOX_POST_RESULT
CEffectLoadPreparationJob::Post_Command(
	EFFECT_LOAD_JOB_COMMAND Command,
	std::optional<EFFECT_LOAD_JOB_COMMAND>& OutDisplacedCommand,
	std::string& strOutStatus)
{
	OutDisplacedCommand.reset();
	if (!Command.Is_Valid())
	{
		strOutStatus = "Effect load mailbox command is invalid.";
		return EFFECT_LOAD_MAILBOX_POST_RESULT::INVALID;
	}

	EFFECT_LOAD_MAILBOX_POST_RESULT PostResult =
		EFFECT_LOAD_MAILBOX_POST_RESULT::POSTED;
	{
		std::lock_guard<std::mutex> Lock(m_Mutex);
		if (!m_bOpen)
		{
			strOutStatus = "Effect load job is not open.";
			return EFFECT_LOAD_MAILBOX_POST_RESULT::INVALID;
		}
		if (m_bCancelled)
		{
			strOutStatus = "Effect load job is already cancelled.";
			return EFFECT_LOAD_MAILBOX_POST_RESULT::CANCELLED;
		}
		if (m_bClosed)
		{
			strOutStatus = "Effect load job is already closed.";
			return EFFECT_LOAD_MAILBOX_POST_RESULT::CLOSED;
		}

		//REBASE만 세대 교체를 허용한다. 나머지 명령은 현재 두 세대가 모두 같아야 한다.
		if (EFFECT_LOAD_JOB_COMMAND_KIND::REBASE == Command.eKind)
		{
			if (Command.iJobEpoch <= m_iCurrentJobEpoch)
			{
				strOutStatus =
					"Effect load rebase epoch must be newer than the current epoch.";
				return EFFECT_LOAD_MAILBOX_POST_RESULT::INVALID;
			}
			m_iCurrentJobEpoch = Command.iJobEpoch;
			m_iCurrentCatalogRevision = Command.iCatalogRevision;
			m_Results.clear();
			m_FirstFailure.reset();
			m_Progress = {};
			m_Progress.iJobEpoch = Command.iJobEpoch;
			m_Progress.iCatalogRevision = Command.iCatalogRevision;
			m_ProgressPhaseStarted = std::chrono::steady_clock::now();
		}
		else if (Command.iJobEpoch != m_iCurrentJobEpoch ||
			Command.iCatalogRevision != m_iCurrentCatalogRevision)
		{
			strOutStatus =
				"Effect load mailbox command does not match the current epoch.";
			return EFFECT_LOAD_MAILBOX_POST_RESULT::INVALID;
		}

		//단일 슬롯이므로 이전 명령은 호출자에게 넘기고 최신 명령으로 교체한다.
		if (m_Mailbox.has_value())
		{
			OutDisplacedCommand = std::move(m_Mailbox);
			PostResult = EFFECT_LOAD_MAILBOX_POST_RESULT::REPLACED;
		}
		m_Mailbox = std::move(Command);
		//취소는 남은 결과를 버린다. CLOSE는 결과를 남기고 새 제출을 막는다.
		if (EFFECT_LOAD_JOB_COMMAND_KIND::CANCEL == m_Mailbox->eKind)
		{
			m_bCancelled = true;
			m_Results.clear();
			m_Progress.ePhase = EFFECT_LOAD_PROGRESS_PHASE::CANCELLED;
			m_ProgressPhaseStarted = std::chrono::steady_clock::now();
			m_Progress.iElapsedMilliseconds = 0u;
		}
		else if (EFFECT_LOAD_JOB_COMMAND_KIND::CLOSE == m_Mailbox->eKind)
		{
			m_bClosed = true;
			m_Progress.ePhase = EFFECT_LOAD_PROGRESS_PHASE::CLOSED;
			m_ProgressPhaseStarted = std::chrono::steady_clock::now();
			m_Progress.iElapsedMilliseconds = 0u;
		}
	}

	//잠금을 푼 뒤 명령 대기와 결과 공간 대기를 모두 깨운다.
	m_MailboxCondition.notify_all();
	m_ResultSpaceCondition.notify_all();
	strOutStatus = EFFECT_LOAD_MAILBOX_POST_RESULT::REPLACED == PostResult ?
		"Effect load mailbox command replaced the pending command." :
		"Effect load mailbox command posted.";
	return PostResult;
}

//worker가 다음 승인·ACK·종료 명령을 기다린 뒤 소유권을 가져간다.
EFFECT_LOAD_MAILBOX_WAIT_RESULT
CEffectLoadPreparationJob::Wait_Pop_Command(
	EFFECT_LOAD_JOB_COMMAND& OutCommand)
{
	OutCommand = {};
	std::unique_lock<std::mutex> Lock(m_Mutex);
	//wait는 잠금을 놓고 기다린다. 깨어나면 다시 잠근 뒤 조건을 확인한다.
	m_MailboxCondition.wait(Lock, [this]()
	{
		return m_Mailbox.has_value() || m_bCancelled || m_bClosed;
	});
	//종료 명령도 슬롯에 있으면 먼저 전달한다. 소비 후에는 종료 상태를 반환한다.
	if (m_Mailbox.has_value())
	{
		OutCommand = std::move(*m_Mailbox);
		m_Mailbox.reset();
		return EFFECT_LOAD_MAILBOX_WAIT_RESULT::COMMAND;
	}
	return m_bCancelled ? EFFECT_LOAD_MAILBOX_WAIT_RESULT::CANCELLED :
		EFFECT_LOAD_MAILBOX_WAIT_RESULT::CLOSED;
}

//worker 결과를 큐에 넘긴다. 대기 중 세대가 바뀌면 이전 결과를 넣지 않는다.
EFFECT_LOAD_RESULT_PUSH_RESULT
CEffectLoadPreparationJob::Push_Result_Wait(
	EFFECT_LOAD_JOB_RESULT Result)
{
	if (!Result.Is_Valid())
	{
		return EFFECT_LOAD_RESULT_PUSH_RESULT::INVALID;
	}

	const EFFECT_LOAD_JOB_EPOCH iResultEpoch = Result.iJobEpoch;
	const uint64_t iResultRevision = Result.iCatalogRevision;
	std::unique_lock<std::mutex> Lock(m_Mutex);
	if (!m_bOpen)
	{
		return EFFECT_LOAD_RESULT_PUSH_RESULT::INVALID;
	}
	//큐 상한은 미소비 결과만 제한한다. worker의 내부 준비 자료 개수는 여기서 관리하지 않는다.
	m_ResultSpaceCondition.wait(Lock, [this, iResultEpoch, iResultRevision]()
	{
		return m_Results.size() < EFFECT_LOAD_RESULT_CHANNEL_CAPACITY ||
			m_bCancelled || m_bClosed ||
			m_iCurrentJobEpoch != iResultEpoch ||
			m_iCurrentCatalogRevision != iResultRevision;
	});
	//빈자리 때문에 깨어난 경우라도 종료·세대 교체를 먼저 검사한다.
	if (m_bCancelled)
	{
		return EFFECT_LOAD_RESULT_PUSH_RESULT::CANCELLED;
	}
	if (m_bClosed)
	{
		return EFFECT_LOAD_RESULT_PUSH_RESULT::CLOSED;
	}
	if (m_iCurrentJobEpoch != iResultEpoch ||
		m_iCurrentCatalogRevision != iResultRevision)
	{
		return EFFECT_LOAD_RESULT_PUSH_RESULT::REBASED;
	}
	m_Results.push_back(std::move(Result));
	return EFFECT_LOAD_RESULT_PUSH_RESULT::PUSHED;
}

//메인이 결과 하나를 이동해 받고 빈자리를 기다리던 worker를 깨운다.
bool CEffectLoadPreparationJob::Try_Pop_Result(
	EFFECT_LOAD_JOB_RESULT& OutResult)
{
	OutResult = {};
	{
		std::lock_guard<std::mutex> Lock(m_Mutex);
		if (m_Results.empty())
		{
			return false;
		}
		OutResult = std::move(m_Results.front());
		m_Results.pop_front();
	}
	m_ResultSpaceCondition.notify_one();
	return true;
}

//현재 세대의 첫 구조적 실패만 보존해 후속 취소 문구가 원인을 가리지 않게 한다.
bool CEffectLoadPreparationJob::Record_FirstFailure(
	EFFECT_LOAD_FAILURE_RECEIPT Failure)
{
	if (!Failure.Is_Valid())
	{
		return false;
	}
	std::lock_guard<std::mutex> Lock(m_Mutex);
	if (!m_bOpen || m_bCancelled || m_bClosed || m_FirstFailure.has_value() ||
		Failure.iJobEpoch != m_iCurrentJobEpoch ||
		Failure.iCatalogRevision != m_iCurrentCatalogRevision)
	{
		return false;
	}
	m_FirstFailure = std::move(Failure);
	return true;
}

//취소·종료 후에도 최초 원인을 값으로 복사해 조회한다.
std::optional<EFFECT_LOAD_FAILURE_RECEIPT>
CEffectLoadPreparationJob::Get_FirstFailure() const
{
	std::lock_guard<std::mutex> Lock(m_Mutex);
	return m_FirstFailure;
}

//현재 작업의 진행만 반영한다. 완료 수의 범위와 같은 단계에서의 역행을 검사한다.
bool CEffectLoadPreparationJob::Publish_Progress(
	const EFFECT_LOAD_PROGRESS_SNAPSHOT& Progress)
{
	std::lock_guard<std::mutex> Lock(m_Mutex);
	if (!m_bOpen || m_bCancelled || m_bClosed ||
		Progress.iJobEpoch != m_iCurrentJobEpoch ||
		Progress.iCatalogRevision != m_iCurrentCatalogRevision ||
		(Progress.bDeterminate && Progress.iCompleted > Progress.iTotal) ||
		(!Progress.bDeterminate &&
		 (0u != Progress.iCompleted || 0u != Progress.iTotal)) ||
		(Progress.bDeterminate && m_Progress.bDeterminate &&
		 Progress.ePhase == m_Progress.ePhase &&
		 Progress.iTotal == m_Progress.iTotal &&
		 Progress.iCompleted < m_Progress.iCompleted))
	{
		return false;
	}
	//단계가 바뀔 때 시계를 다시 잡고 현재 단계의 경과 시간만 기록한다.
	const auto Now = std::chrono::steady_clock::now();
	if (Progress.ePhase != m_Progress.ePhase)
	{
		m_ProgressPhaseStarted = Now;
	}
	m_Progress = Progress;
	m_Progress.iElapsedMilliseconds = static_cast<uint64_t>(
		std::chrono::duration_cast<std::chrono::milliseconds>(
			Now - m_ProgressPhaseStarted).count());
	return true;
}

//로딩 화면이 읽을 복사본에 현재 단계 경과 시간을 계산한다.
EFFECT_LOAD_PROGRESS_SNAPSHOT
CEffectLoadPreparationJob::Get_Progress() const
{
	std::lock_guard<std::mutex> Lock(m_Mutex);
	EFFECT_LOAD_PROGRESS_SNAPSHOT Progress = m_Progress;
	Progress.iElapsedMilliseconds = static_cast<uint64_t>(
		std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() -
			m_ProgressPhaseStarted).count());
	return Progress;
}

//아래 조회는 같은 mutex로 상태를 보호한다. 호출 사이에 값이 바뀔 수 있다.
EFFECT_LOAD_JOB_EPOCH
CEffectLoadPreparationJob::Get_CurrentEpoch() const
{
	std::lock_guard<std::mutex> Lock(m_Mutex);
	return m_iCurrentJobEpoch;
}

uint64_t CEffectLoadPreparationJob::Get_CurrentCatalogRevision() const
{
	std::lock_guard<std::mutex> Lock(m_Mutex);
	return m_iCurrentCatalogRevision;
}

size_t CEffectLoadPreparationJob::Get_PendingResultCount() const
{
	std::lock_guard<std::mutex> Lock(m_Mutex);
	return m_Results.size();
}

bool CEffectLoadPreparationJob::Is_Cancelled() const
{
	std::lock_guard<std::mutex> Lock(m_Mutex);
	return m_bCancelled;
}

bool CEffectLoadPreparationJob::Is_Closed() const
{
	std::lock_guard<std::mutex> Lock(m_Mutex);
	return m_bClosed;
}

NS_END

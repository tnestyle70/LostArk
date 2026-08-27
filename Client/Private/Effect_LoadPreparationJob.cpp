#include "Effect_LoadPreparationJob.h"

#include <algorithm>
#include <utility>

namespace
{

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

bool Client::EFFECT_LOAD_FAILURE_RECEIPT::Is_Valid() const
{
	return 0u != iJobEpoch && 0u != iCatalogRevision &&
		0 != iRootCode && !strRootMessage.empty();
}

bool Client::EFFECT_LOAD_JOB_COMMAND::Is_Valid() const
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

Client::EFFECT_LOAD_JOB_COMMAND
Client::EFFECT_LOAD_JOB_COMMAND::Accept_Discovery(
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

Client::EFFECT_LOAD_JOB_COMMAND Client::EFFECT_LOAD_JOB_COMMAND::Rebase(
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

Client::EFFECT_LOAD_JOB_COMMAND
Client::EFFECT_LOAD_JOB_COMMAND::Target_CommitAck(
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

Client::EFFECT_LOAD_JOB_COMMAND Client::EFFECT_LOAD_JOB_COMMAND::Close(
	const EFFECT_LOAD_JOB_EPOCH iJobEpoch,
	const uint64_t iCatalogRevision)
{
	EFFECT_LOAD_JOB_COMMAND Command;
	Command.eKind = EFFECT_LOAD_JOB_COMMAND_KIND::CLOSE;
	Command.iJobEpoch = iJobEpoch;
	Command.iCatalogRevision = iCatalogRevision;
	return Command;
}

Client::EFFECT_LOAD_JOB_COMMAND Client::EFFECT_LOAD_JOB_COMMAND::Cancel(
	const EFFECT_LOAD_JOB_EPOCH iJobEpoch,
	const uint64_t iCatalogRevision)
{
	EFFECT_LOAD_JOB_COMMAND Command;
	Command.eKind = EFFECT_LOAD_JOB_COMMAND_KIND::CANCEL;
	Command.iJobEpoch = iJobEpoch;
	Command.iCatalogRevision = iCatalogRevision;
	return Command;
}

bool Client::EFFECT_LOAD_JOB_RESULT::Is_Valid() const
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

bool Client::CEffectLoadPreparationJob::Open(
	const EFFECT_LOAD_JOB_EPOCH iInitialJobEpoch,
	const uint64_t iCatalogRevision,
	std::string& strOutStatus)
{
	if (0u == iInitialJobEpoch || 0u == iCatalogRevision)
	{
		strOutStatus = "Effect load job epoch or catalog revision is invalid.";
		return false;
	}

	std::lock_guard Lock(m_Mutex);
	if (m_bOpen && !m_bCancelled && !m_bClosed)
	{
		strOutStatus = "Effect load job is already open.";
		return false;
	}
	m_Results.clear();
	m_Mailbox.reset();
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

Client::EFFECT_LOAD_MAILBOX_POST_RESULT
Client::CEffectLoadPreparationJob::Post_Command(
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
		std::lock_guard Lock(m_Mutex);
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

		if (m_Mailbox.has_value())
		{
			OutDisplacedCommand = std::move(m_Mailbox);
			PostResult = EFFECT_LOAD_MAILBOX_POST_RESULT::REPLACED;
		}
		m_Mailbox = std::move(Command);
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

	m_MailboxCondition.notify_all();
	m_ResultSpaceCondition.notify_all();
	strOutStatus = EFFECT_LOAD_MAILBOX_POST_RESULT::REPLACED == PostResult ?
		"Effect load mailbox command replaced the pending command." :
		"Effect load mailbox command posted.";
	return PostResult;
}

Client::EFFECT_LOAD_MAILBOX_WAIT_RESULT
Client::CEffectLoadPreparationJob::Wait_Pop_Command(
	EFFECT_LOAD_JOB_COMMAND& OutCommand)
{
	OutCommand = {};
	std::unique_lock Lock(m_Mutex);
	m_MailboxCondition.wait(Lock, [this]()
	{
		return m_Mailbox.has_value() || m_bCancelled || m_bClosed;
	});
	if (m_Mailbox.has_value())
	{
		OutCommand = std::move(*m_Mailbox);
		m_Mailbox.reset();
		return EFFECT_LOAD_MAILBOX_WAIT_RESULT::COMMAND;
	}
	return m_bCancelled ? EFFECT_LOAD_MAILBOX_WAIT_RESULT::CANCELLED :
		EFFECT_LOAD_MAILBOX_WAIT_RESULT::CLOSED;
}

Client::EFFECT_LOAD_RESULT_PUSH_RESULT
Client::CEffectLoadPreparationJob::Push_Result_Wait(
	EFFECT_LOAD_JOB_RESULT Result)
{
	if (!Result.Is_Valid())
		return EFFECT_LOAD_RESULT_PUSH_RESULT::INVALID;

	const EFFECT_LOAD_JOB_EPOCH iResultEpoch = Result.iJobEpoch;
	const uint64_t iResultRevision = Result.iCatalogRevision;
	std::unique_lock Lock(m_Mutex);
	if (!m_bOpen)
		return EFFECT_LOAD_RESULT_PUSH_RESULT::INVALID;
	m_ResultSpaceCondition.wait(Lock, [this, iResultEpoch, iResultRevision]()
	{
		return m_Results.size() < EFFECT_LOAD_RESULT_CHANNEL_CAPACITY ||
			m_bCancelled || m_bClosed ||
			m_iCurrentJobEpoch != iResultEpoch ||
			m_iCurrentCatalogRevision != iResultRevision;
	});
	if (m_bCancelled)
		return EFFECT_LOAD_RESULT_PUSH_RESULT::CANCELLED;
	if (m_bClosed)
		return EFFECT_LOAD_RESULT_PUSH_RESULT::CLOSED;
	if (m_iCurrentJobEpoch != iResultEpoch ||
		m_iCurrentCatalogRevision != iResultRevision)
	{
		return EFFECT_LOAD_RESULT_PUSH_RESULT::REBASED;
	}
	m_Results.push_back(std::move(Result));
	return EFFECT_LOAD_RESULT_PUSH_RESULT::PUSHED;
}

bool Client::CEffectLoadPreparationJob::Try_Pop_Result(
	EFFECT_LOAD_JOB_RESULT& OutResult)
{
	OutResult = {};
	{
		std::lock_guard Lock(m_Mutex);
		if (m_Results.empty())
			return false;
		OutResult = std::move(m_Results.front());
		m_Results.pop_front();
	}
	m_ResultSpaceCondition.notify_one();
	return true;
}

bool Client::CEffectLoadPreparationJob::Publish_Progress(
	const EFFECT_LOAD_PROGRESS_SNAPSHOT& Progress)
{
	std::lock_guard Lock(m_Mutex);
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
	const auto Now = std::chrono::steady_clock::now();
	if (Progress.ePhase != m_Progress.ePhase)
		m_ProgressPhaseStarted = Now;
	m_Progress = Progress;
	m_Progress.iElapsedMilliseconds = static_cast<uint64_t>(
		std::chrono::duration_cast<std::chrono::milliseconds>(
			Now - m_ProgressPhaseStarted).count());
	return true;
}

Client::EFFECT_LOAD_PROGRESS_SNAPSHOT
Client::CEffectLoadPreparationJob::Get_Progress() const
{
	std::lock_guard Lock(m_Mutex);
	EFFECT_LOAD_PROGRESS_SNAPSHOT Progress = m_Progress;
	Progress.iElapsedMilliseconds = static_cast<uint64_t>(
		std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() -
			m_ProgressPhaseStarted).count());
	return Progress;
}

Client::EFFECT_LOAD_JOB_EPOCH
Client::CEffectLoadPreparationJob::Get_CurrentEpoch() const
{
	std::lock_guard Lock(m_Mutex);
	return m_iCurrentJobEpoch;
}

uint64_t Client::CEffectLoadPreparationJob::Get_CurrentCatalogRevision() const
{
	std::lock_guard Lock(m_Mutex);
	return m_iCurrentCatalogRevision;
}

size_t Client::CEffectLoadPreparationJob::Get_PendingResultCount() const
{
	std::lock_guard Lock(m_Mutex);
	return m_Results.size();
}

bool Client::CEffectLoadPreparationJob::Is_Cancelled() const
{
	std::lock_guard Lock(m_Mutex);
	return m_bCancelled;
}

bool Client::CEffectLoadPreparationJob::Is_Closed() const
{
	std::lock_guard Lock(m_Mutex);
	return m_bClosed;
}

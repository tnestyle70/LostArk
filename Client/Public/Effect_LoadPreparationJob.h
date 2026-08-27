#pragma once

#include <condition_variable>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace Client
{

using EFFECT_LOAD_JOB_EPOCH = uint64_t;

constexpr size_t EFFECT_LOAD_RESULT_CHANNEL_CAPACITY = 2u;

enum class EFFECT_LOAD_JOB_COMMAND_KIND : uint8_t
{
	ACCEPT_DISCOVERY,
	REBASE,
	TARGET_COMMIT_ACK,
	CLOSE,
	CANCEL,
	END
};

enum class EFFECT_LOAD_JOB_RESULT_KIND : uint8_t
{
	DISCOVERY,
	TARGET_STAGED,
	EPOCH_STAGE_COMPLETE,
	STRUCTURAL_FAILURE,
	END
};

enum class EFFECT_LOAD_RESULT_PUSH_RESULT : uint8_t
{
	PUSHED,
	CANCELLED,
	REBASED,
	CLOSED,
	INVALID,
	END
};

enum class EFFECT_LOAD_MAILBOX_POST_RESULT : uint8_t
{
	POSTED,
	REPLACED,
	CANCELLED,
	CLOSED,
	INVALID,
	END
};

enum class EFFECT_LOAD_MAILBOX_WAIT_RESULT : uint8_t
{
	COMMAND,
	CANCELLED,
	CLOSED,
	END
};

enum class EFFECT_LOAD_PROGRESS_PHASE : uint8_t
{
	IDLE,
	DISCOVERY,
	WAITING_FOR_DISCOVERY_ACCEPT,
	TARGET_STAGE,
	EPOCH_STAGE_COMPLETE,
	FAILED,
	CANCELLED,
	CLOSED,
	END
};

struct EFFECT_LOAD_FAILURE_RECEIPT final
{
	EFFECT_LOAD_JOB_EPOCH iJobEpoch = 0u;
	uint64_t iCatalogRevision = 0u;
	std::string strEffectAssetId;
	int64_t iRootCode = 0;
	std::string strRootMessage;

	bool Is_Valid() const;
};

/* The payload is immutable and caller-owned.  This seam intentionally has no
   dependency on catalog, renderer or D3D types.  The integration layer owns
   the concrete payload type and casts it only after validating epoch/revision
   and result kind. */
struct EFFECT_LOAD_JOB_COMMAND final
{
	EFFECT_LOAD_JOB_COMMAND_KIND eKind =
		EFFECT_LOAD_JOB_COMMAND_KIND::END;
	EFFECT_LOAD_JOB_EPOCH iJobEpoch = 0u;
	uint64_t iCatalogRevision = 0u;
	std::vector<std::string> EffectAssetIds;
	std::shared_ptr<const void> pImmutablePayload;

	bool Is_Valid() const;

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

struct EFFECT_LOAD_JOB_RESULT final
{
	EFFECT_LOAD_JOB_RESULT_KIND eKind = EFFECT_LOAD_JOB_RESULT_KIND::END;
	EFFECT_LOAD_JOB_EPOCH iJobEpoch = 0u;
	uint64_t iCatalogRevision = 0u;
	std::string strEffectAssetId;
	std::shared_ptr<const void> pImmutablePayload;
	std::optional<EFFECT_LOAD_FAILURE_RECEIPT> Failure;

	bool Is_Valid() const;
};

struct EFFECT_LOAD_PROGRESS_SNAPSHOT final
{
	EFFECT_LOAD_JOB_EPOCH iJobEpoch = 0u;
	uint64_t iCatalogRevision = 0u;
	EFFECT_LOAD_PROGRESS_PHASE ePhase = EFFECT_LOAD_PROGRESS_PHASE::IDLE;
	bool bDeterminate = false;
	uint32_t iCompleted = 0u;
	uint32_t iTotal = 0u;
	std::string strCurrentId;
	std::string strStatus;
	uint64_t iElapsedMilliseconds = 0u;
	uint32_t iIsolatedFailureCount = 0u;
};

/* One Loader worker is the only result producer and the main thread is the
   only result consumer.  Push_Result_Wait may block only while the fixed-size
   result channel is full.  Cancel, rebase and close all wake that producer.
   The main thread never waits: it only calls Try_Pop_Result.  Commands travel
   in the opposite direction through a latest-value single-slot mailbox.  A
   TARGET_STAGED producer must not advance to its next target until the owner
   posts the matching TARGET_COMMIT_ACK (or a terminal/rebase command). */
class CEffectLoadPreparationJob final
{
public:
	CEffectLoadPreparationJob() = default;
	CEffectLoadPreparationJob(const CEffectLoadPreparationJob&) = delete;
	CEffectLoadPreparationJob& operator=(
		const CEffectLoadPreparationJob&) = delete;

	bool Open(
		EFFECT_LOAD_JOB_EPOCH iInitialJobEpoch,
		uint64_t iCatalogRevision,
		std::string& strOutStatus);

	EFFECT_LOAD_MAILBOX_POST_RESULT Post_Command(
		EFFECT_LOAD_JOB_COMMAND Command,
		std::optional<EFFECT_LOAD_JOB_COMMAND>& OutDisplacedCommand,
		std::string& strOutStatus);
	EFFECT_LOAD_MAILBOX_WAIT_RESULT Wait_Pop_Command(
		EFFECT_LOAD_JOB_COMMAND& OutCommand);

	EFFECT_LOAD_RESULT_PUSH_RESULT Push_Result_Wait(
		EFFECT_LOAD_JOB_RESULT Result);
	bool Try_Pop_Result(EFFECT_LOAD_JOB_RESULT& OutResult);

	bool Publish_Progress(const EFFECT_LOAD_PROGRESS_SNAPSHOT& Progress);
	EFFECT_LOAD_PROGRESS_SNAPSHOT Get_Progress() const;

	EFFECT_LOAD_JOB_EPOCH Get_CurrentEpoch() const;
	uint64_t Get_CurrentCatalogRevision() const;
	size_t Get_PendingResultCount() const;
	bool Is_Cancelled() const;
	bool Is_Closed() const;

private:
	mutable std::mutex m_Mutex;
	std::condition_variable m_ResultSpaceCondition;
	std::condition_variable m_MailboxCondition;
	std::deque<EFFECT_LOAD_JOB_RESULT> m_Results;
	std::optional<EFFECT_LOAD_JOB_COMMAND> m_Mailbox;
	EFFECT_LOAD_PROGRESS_SNAPSHOT m_Progress;
	std::chrono::steady_clock::time_point m_ProgressPhaseStarted =
		std::chrono::steady_clock::now();
	EFFECT_LOAD_JOB_EPOCH m_iCurrentJobEpoch = 0u;
	uint64_t m_iCurrentCatalogRevision = 0u;
	bool m_bOpen = false;
	bool m_bCancelled = false;
	bool m_bClosed = false;
};

}

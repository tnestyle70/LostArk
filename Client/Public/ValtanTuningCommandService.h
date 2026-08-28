#pragma once

#include "GameplayDataRevision.h"
#include "Network/PacketMessages.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace Client
{

enum class VALTAN_TUNING_COMMAND_STATE : uint8_t
{
	IDLE,
	PUBLISHING,
	UNCONFIRMED,
	PUBLISHED_APPLY_NEEDED,
	APPLY_PENDING,
	COMMITTED,
	ALREADY_ACTIVE,
	FAILED
};

struct VALTAN_TUNING_COMMAND_SNAPSHOT final
{
	VALTAN_TUNING_COMMAND_STATE eState = VALTAN_TUNING_COMMAND_STATE::IDLE;
	std::string strFlowRevision;
	std::string strCandidateRevision;
	std::string strApplyClass;
	std::string strStatus;
	uint32_t iRequestSequence = 0u;
	uint64_t iConnectionGeneration = 0u;
	uint64_t iWorldInboundGeneration = 0u;
	bool bCandidateIsServerActive = false;
};

#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
struct VALTAN_TUNING_COMMAND_HARNESS_INPUT final
{
	bool bActivationEnabled = true;
	bool bConnected = true;
	bool bSendSucceeds = true;
	bool bPublishStartSucceeds = true;
	bool bPublishRunning = true;
	bool bPublishPollSucceeds = true;
	bool bPublishOutputReadable = true;
	uint32_t iPublishExitCode = 0u;
	std::string strPublishOutput;
	uint64_t iNowMilliseconds = 100u;
	uint64_t iConnectionGeneration = 1u;
	uint64_t iWorldInboundGeneration = 1u;
	bool bOutstandingPrepareRequest = false;
	bool bStagedPresentationAlias = false;
	bool bRejectedPrepareAwaitingAbort = false;
	bool bHasLatestResult = false;
	uint32_t iLatestTransactionSequence = 0u;
	LostArk::Shared::GameplayDataRevision ServerActiveRevision{};
	LostArk::Shared::GameplayDataRevision LatestCandidateRevision{};
	LostArk::Shared::DATA_REVISION_RESULT eLatestResult =
		LostArk::Shared::DATA_REVISION_RESULT::ABORTED;
	std::string strLatestTransactionReason;
	std::vector<std::string> PublishedFlowRevisions;
	std::vector<LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST> SentRequests;
};
#endif

/* Main-thread owner for the saved-Flow publisher and the existing Valtan 2PC
   command. Both editor callers share one request sequence. A publisher result
   does not acknowledge Server application, and no uncertain helper is killed. */
class CValtanTuningCommandService final
{
public:
	static CValtanTuningCommandService& Get();
	bool Publish_SavedPatternFlow(
		std::string_view strSavedRevision, std::string& strOutStatus);
	bool ApplyCandidate(
		std::string_view strCandidateRevision,
		std::string_view strApplyClass,
		std::string& strOutStatus);
	void Update();
	[[nodiscard]] bool Has_PendingCommand() const;
	/* Playback may consume a saved Flow only while the exact candidate produced
	   from that save is still active in the current Server world. */
	[[nodiscard]] bool Is_SavedPatternFlowServerActive(
		std::string_view strSavedRevision) const;
	[[nodiscard]] const VALTAN_TUNING_COMMAND_SNAPSHOT& Get_Snapshot() const
	{
		return m_Snapshot;
	}

#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	void Harness_Reset();
	VALTAN_TUNING_COMMAND_HARNESS_INPUT& Harness_Input() { return m_HarnessInput; }
	void Harness_SetNextRequestSequence(uint32_t iSequence)
	{
		m_iNextRequestSequence = iSequence;
	}
#endif

private:
	struct REVISION_OBSERVATION final
	{
		bool bConnected = false;
		bool bOtherTransactionPending = false;
		bool bOutstandingPrepareRequest = false;
		bool bHasLatestResult = false;
		uint32_t iLatestTransactionSequence = 0u;
		uint64_t iConnectionGeneration = 0u;
		uint64_t iWorldInboundGeneration = 0u;
		LostArk::Shared::GameplayDataRevision ServerActiveRevision{};
		LostArk::Shared::GameplayDataRevision LatestCandidateRevision{};
		LostArk::Shared::DATA_REVISION_RESULT eLatestResult =
			LostArk::Shared::DATA_REVISION_RESULT::ABORTED;
		std::string strReason;
	};

	CValtanTuningCommandService() = default;
	~CValtanTuningCommandService();
	CValtanTuningCommandService(const CValtanTuningCommandService&) = delete;
	CValtanTuningCommandService& operator=(const CValtanTuningCommandService&) = delete;
	[[nodiscard]] bool Is_ActivationEnabled() const;
	[[nodiscard]] uint64_t Now_Milliseconds() const;
	[[nodiscard]] REVISION_OBSERVATION Read_RevisionObservation() const;
	bool Start_PublishProcess(std::string_view strFlowRevision, std::string& strOutStatus);
	bool Poll_PublishProcess(bool& bFinished, uint32_t& iExitCode,
		std::string& strOutput, std::string& strOutStatus);
	void Consume_PublishResult(uint32_t iExitCode, const std::string& strOutput);
	bool Submit_Candidate(const REVISION_OBSERVATION& Observation, std::string& strOutStatus);
	bool Send_PrepareRequest(const LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST& Request);
	void Refresh_ActiveCandidate(const REVISION_OBSERVATION& Observation);

	VALTAN_TUNING_COMMAND_SNAPSHOT m_Snapshot;
	LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST m_ApplyRequest;
	bool m_bPublishing = false;
	bool m_bApplyPending = false;
	uint64_t m_iPublishStartedAtMilliseconds = 0u;
	uint64_t m_iApplyStartedAtMilliseconds = 0u;
	uint32_t m_iNextRequestSequence = 1u;
	void* m_hPublishProcess = nullptr;
	std::filesystem::path m_PublishOutputPath;
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	VALTAN_TUNING_COMMAND_HARNESS_INPUT m_HarnessInput;
#endif
};

const char* Describe_ValtanTuningCommandState(VALTAN_TUNING_COMMAND_STATE eState);

}

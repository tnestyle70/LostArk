#pragma once

#include "GameplayDataRevision.h"
#include "Network/PacketMessages.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Client
{

enum class VALTAN_TUNING_COMMAND_STATE : uint8_t
{
	IDLE,
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
	std::vector<LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST> SentRequests;
};
#endif

/* Main-thread owner for the Valtan Product-candidate 2PC command. Canonical
   authoring save/publish is owned by its typed document service; this service
   accepts only the resulting immutable candidate revision and never invokes a
   second saved-Flow publisher. */
class CValtanTuningCommandService final
{
public:
	static CValtanTuningCommandService& Get();
	bool ApplyCandidate(
		std::string_view strCandidateRevision,
		std::string_view strApplyClass,
		std::string& strOutStatus);
	/* Records the Product candidate (or the absence of one) produced after an
	   immutable gameplay authoring save.  Complete Play uses this independent
	   gate because restart-class and failed publications never enter the live
	   ApplyCandidate transaction snapshot. */
	void Record_GameplaySourceActivationExpectation(
		std::string_view strCandidateRevision,
		std::string_view strApplyClass,
		std::string_view strStatus);
	[[nodiscard]] bool Is_LatestGameplaySourceServerActive(
		std::string& strOutStatus) const;
	/* Returns the exact immutable Product revision only when the latest saved
	   gameplay source expectation and the current Server-active revision match.
	   Callers compare this value again after their own presentation admission so
	   a concurrent revision change cannot authorize playback of another Product. */
	[[nodiscard]] bool Try_GetLatestGameplaySourceServerActiveRevision(
		LostArk::Shared::GameplayDataRevision& outRevision,
		std::string& strOutStatus) const;
	[[nodiscard]] bool Has_GameplaySourceActivationExpectation() const
	{
		return m_bGameplaySourceActivationObserved;
	}
	void Update();
	[[nodiscard]] bool Has_PendingCommand() const;
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
	~CValtanTuningCommandService() = default;
	CValtanTuningCommandService(const CValtanTuningCommandService&) = delete;
	CValtanTuningCommandService& operator=(const CValtanTuningCommandService&) = delete;
	[[nodiscard]] bool Is_ActivationEnabled() const;
	[[nodiscard]] uint64_t Now_Milliseconds() const;
	[[nodiscard]] REVISION_OBSERVATION Read_RevisionObservation() const;
	bool Submit_Candidate(const REVISION_OBSERVATION& Observation, std::string& strOutStatus);
	bool Send_PrepareRequest(const LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST& Request);
	void Refresh_ActiveCandidate(const REVISION_OBSERVATION& Observation);

	VALTAN_TUNING_COMMAND_SNAPSHOT m_Snapshot;
	LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST m_ApplyRequest;
	bool m_bApplyPending = false;
	uint64_t m_iApplyStartedAtMilliseconds = 0u;
	uint32_t m_iNextRequestSequence = 1u;
	bool m_bGameplaySourceActivationObserved = false;
	std::string m_strGameplayCandidateRevision;
	std::string m_strGameplayCandidateApplyClass;
	std::string m_strGameplayActivationStatus;
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	VALTAN_TUNING_COMMAND_HARNESS_INPUT m_HarnessInput;
#endif
};

const char* Describe_ValtanTuningCommandState(VALTAN_TUNING_COMMAND_STATE eState);

}

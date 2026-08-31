#pragma once

#include "GameplayDataRevision.h"
#include "Network/PacketMessages.h"
#include "ValtanPatternSoundCueDocument.h"

#include <cstdint>
#include <string>
#include <string_view>

#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
#include <deque>
#include <vector>
#endif

namespace Client
{

enum class VALTAN_PATTERN_AUDITION_STATE : uint8_t
{
	IDLE,
	REQUEST_PENDING,
	/* The exact Restart request may already have committed on the Server, but
	   neither its verdict nor replacement lifecycle arrived before the bound.
	   Keep the request/fallback identity and permit only an exact retry. */
	RESTART_UNCONFIRMED,
	QUEUED,
	ACTIVE,
	COMPLETED,
	REJECTED,
	ABORTED,
	END
};

struct VALTAN_PATTERN_AUDITION_SNAPSHOT final
{
	VALTAN_PATTERN_AUDITION_STATE eState =
		VALTAN_PATTERN_AUDITION_STATE::IDLE;
	uint32_t iRequestSequence = 0u;
	uint64_t iWorldInboundGeneration = 0u;
	uint32_t iRoomAuditionEpoch = 0u;
	uint32_t iObservedPatternSequence = 0u;
	LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
	VALTAN_PATTERN_SOUND_SOURCE_RECEIPT PinnedPatternSoundSourceReceipt{};
	bool isPresentationRevisionAvailable = false;
	std::string strConsumerId;
	std::string strBossPlacementId;
	std::string strPatternId;
	std::string strStatus;

	[[nodiscard]] bool Is_InFlight() const
	{
		return VALTAN_PATTERN_AUDITION_STATE::REQUEST_PENDING == eState ||
			VALTAN_PATTERN_AUDITION_STATE::RESTART_UNCONFIRMED == eState ||
			VALTAN_PATTERN_AUDITION_STATE::QUEUED == eState ||
			VALTAN_PATTERN_AUDITION_STATE::ACTIVE == eState;
	}
};

enum class VALTAN_NEXT_PATTERN_STATE : uint8_t
{
	NONE,
	RESERVED,
	WAITING_FOR_PLAYER,
	START_PENDING,
	REJECTED,
	ABORTED,
	END
};

struct VALTAN_NEXT_PATTERN_SNAPSHOT final
{
	VALTAN_NEXT_PATTERN_STATE eState = VALTAN_NEXT_PATTERN_STATE::NONE;
	uint32_t iRequestSequence = 0u;
	uint64_t iWorldInboundGeneration = 0u;
	uint32_t iRoomAuditionEpoch = 0u;
	uint32_t iPredecessorPatternSequence = 0u;
	uint32_t iExpectedPatternSequence = 0u;
	/* PENDING consumes the Server reservation even when its first tick must
	   wait for a player. Keep that occurrence visible until ACTIVE, but do not
	   send reservation controls against its retired predecessor token. */
	bool bReservationConsumed = false;
	LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
	VALTAN_PATTERN_SOUND_SOURCE_RECEIPT PinnedPatternSoundSourceReceipt{};
	bool isPresentationRevisionAvailable = false;
	std::string strConsumerId;
	std::string strBossPlacementId;
	std::string strPatternId;
	std::string strStatus;

	[[nodiscard]] bool Is_Live() const
	{
		return VALTAN_NEXT_PATTERN_STATE::RESERVED == eState ||
			VALTAN_NEXT_PATTERN_STATE::WAITING_FOR_PLAYER == eState ||
			VALTAN_NEXT_PATTERN_STATE::START_PENDING == eState;
	}
};

enum class VALTAN_NEXT_COMMAND_STATE : uint8_t
{
	NONE,
	WAITING_VERDICT,
	UNCONFIRMED,
	END
};

/* A pending replacement is not the approved reservation. Keep the exact
   payload until a verdict (or matching reservation lifecycle) resolves it;
   an unconfirmed command may only retry this same request identity. */
struct VALTAN_NEXT_PATTERN_COMMAND final
{
	VALTAN_NEXT_COMMAND_STATE eState = VALTAN_NEXT_COMMAND_STATE::NONE;
	LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST Request{};
	VALTAN_PATTERN_SOUND_SOURCE_RECEIPT PinnedPatternSoundSourceReceipt{};
	uint64_t iWorldInboundGeneration = 0u;
	uint64_t iSentAtMilliseconds = 0u;
	std::string strConsumerId;
	std::string strStatus;

	[[nodiscard]] bool Is_Pending() const
	{
		return VALTAN_NEXT_COMMAND_STATE::WAITING_VERDICT == eState ||
			VALTAN_NEXT_COMMAND_STATE::UNCONFIRMED == eState;
	}
};

#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
/* Only transport, time and other service inputs are replaced. The harness
   exercises Submit/Queue/Clear/Retry/Update and the production state machine. */
struct VALTAN_AUDITION_HARNESS_INPUT final
{
	bool bConnected = true;
	bool bSendSucceeds = true;
	bool bFlowInFlight = false;
	bool bFlowStartPending = false;
	bool bLiveBossValid = false;
	bool bLiveBossAlive = true;
	uint32_t iLivePatternSequence = 0u;
	bool bPresentationAvailable = true;
	uint64_t iWorldInboundGeneration = 1u;
	uint64_t iNowMilliseconds = 100u;
	std::deque<LostArk::Shared::S2C_VALTAN_AUDITION_RESULT> Results;
	std::deque<LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE> Lifecycles;
	std::vector<LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST> SentRequests;
};
#endif

/* The PLAY_PATTERN_ID wire has one result queue. Every authoring tool must go
   through this service so one panel cannot drain another panel's verdict.
   It deliberately reports the Client-local world generation separately from
   the Server roomAuditionEpoch; the two identities are not interchangeable.
   All methods are called from the Client main thread. */
class CValtanPatternAuditionService final
{
public:
	static CValtanPatternAuditionService& Get();

	bool Submit(
		std::string_view strConsumerId,
		std::string_view strBossPlacementId,
		std::string_view strPatternId,
		const LostArk::Shared::GameplayDataRevision&
			expectedActiveDefinitionRevision,
		const VALTAN_PATTERN_SOUND_SOURCE_RECEIPT&
			PinnedPatternSoundSourceReceipt,
		std::string& strOutStatus);
	/* Restart replaces only the exact authoritative ACTIVE or COMPLETED
	   occurrence currently owned by this consumer. It allocates a new wire
	   request identity and never clears an ordered Flow or reserved Next. */
	bool Restart_ActivePattern(
		std::string_view strConsumerId,
		std::string_view strBossPlacementId,
		std::string_view strPatternId,
		const VALTAN_PATTERN_SOUND_SOURCE_RECEIPT&
			ExpectedPatternSoundSourceReceipt,
		std::string& strOutStatus);
	/* Reuses the same wire identity and predecessor tuple after a bounded
	   ambiguous wait. The Server receipt replays the original verdict. */
	bool Retry_UnconfirmedRestart(
		const VALTAN_PATTERN_SOUND_SOURCE_RECEIPT&
			ExpectedPatternSoundSourceReceipt,
		std::string& strOutStatus);
	[[nodiscard]] bool Can_QueueNextPattern(
		std::string_view strBossPlacementId,
		const LostArk::Shared::GameplayDataRevision&
			expectedDefinitionRevision,
		std::string& strOutStatus) const;
	bool Queue_NextPattern(
		std::string_view strConsumerId,
		std::string_view strBossPlacementId,
		std::string_view strPatternId,
		const LostArk::Shared::GameplayDataRevision&
			expectedDefinitionRevision,
		const VALTAN_PATTERN_SOUND_SOURCE_RECEIPT&
			PinnedPatternSoundSourceReceipt,
		std::string& strOutStatus);
	bool Clear_NextPattern(std::string& strOutStatus);
	bool Retry_NextPatternCommand(
		const VALTAN_PATTERN_SOUND_SOURCE_RECEIPT&
			ExpectedPatternSoundSourceReceipt,
		std::string& strOutStatus);
	void Update();

	/* Pattern Sound is an independent live source owner.  These queries keep
	   its exact receipt tied to the Client-side occurrence controller even
	   though the Server wire remains gameplay-revision-only. */
	[[nodiscard]] bool Has_PatternSoundMutationBarrier() const;
	[[nodiscard]] bool Verify_PatternSoundSourceReceipt(
		const VALTAN_PATTERN_SOUND_SOURCE_RECEIPT& CurrentReceipt,
		std::string& strOutStatus) const;

	[[nodiscard]] const VALTAN_PATTERN_AUDITION_SNAPSHOT& Get_Snapshot() const
	{
		return m_Snapshot;
	}
	[[nodiscard]] const VALTAN_NEXT_PATTERN_SNAPSHOT& Get_NextSnapshot() const
	{
		return m_NextSnapshot;
	}
	[[nodiscard]] const VALTAN_NEXT_PATTERN_COMMAND& Get_NextCommand() const
	{
		return m_NextCommand;
	}
	[[nodiscard]] bool Has_PendingNextCommand() const
	{
		return m_NextCommand.Is_Pending();
	}
	[[nodiscard]] bool Has_PlaybackOwnership() const
	{
		return m_Snapshot.Is_InFlight() || m_NextSnapshot.Is_Live() ||
			Has_PendingNextCommand();
	}

#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	/* Existing service tests exercise gameplay identities.  Their compatibility
	   overloads inject one deterministic valid S receipt; receipt-specific
	   tests call the production signatures above. */
	bool Submit(
		std::string_view strConsumerId,
		std::string_view strBossPlacementId,
		std::string_view strPatternId,
		const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
		std::string& strOutStatus)
	{
		return Submit(strConsumerId, strBossPlacementId, strPatternId,
			ExpectedRevision, Harness_PatternSoundReceipt(), strOutStatus);
	}
	bool Restart_ActivePattern(
		std::string_view strConsumerId,
		std::string_view strBossPlacementId,
		std::string_view strPatternId,
		std::string& strOutStatus)
	{
		return Restart_ActivePattern(strConsumerId, strBossPlacementId,
			strPatternId, Harness_PatternSoundReceipt(), strOutStatus);
	}
	bool Retry_UnconfirmedRestart(std::string& strOutStatus)
	{
		return Retry_UnconfirmedRestart(
			Harness_PatternSoundReceipt(), strOutStatus);
	}
	bool Queue_NextPattern(
		std::string_view strConsumerId,
		std::string_view strBossPlacementId,
		std::string_view strPatternId,
		const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
		std::string& strOutStatus)
	{
		return Queue_NextPattern(strConsumerId, strBossPlacementId,
			strPatternId, ExpectedRevision, Harness_PatternSoundReceipt(),
			strOutStatus);
	}
	bool Retry_NextPatternCommand(std::string& strOutStatus)
	{
		return Retry_NextPatternCommand(
			Harness_PatternSoundReceipt(), strOutStatus);
	}
	void Harness_Reset();
	void Harness_ObserveBoss(
		bool bBossValid,
		std::string_view strPatternId,
		uint32_t iPatternSequence);
	VALTAN_AUDITION_HARNESS_INPUT& Harness_Input() { return m_HarnessInput; }
	void Harness_SetNextRequestSequence(uint32_t iSequence)
	{
		m_iNextRequestSequence = iSequence;
	}
#endif

private:
	CValtanPatternAuditionService() = default;
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	static VALTAN_PATTERN_SOUND_SOURCE_RECEIPT Harness_PatternSoundReceipt()
	{
		return { std::string(64u, 'a'), 1u };
	}
#endif
	void Observe_Boss(
		bool bBossValid,
		std::string_view strPatternId,
		uint32_t iPatternSequence);
	void Set_Terminal(
		VALTAN_PATTERN_AUDITION_STATE eState,
		std::string strStatus);
	bool Send_NextCommand(
		LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST Request,
		std::string_view strConsumerId,
		const VALTAN_PATTERN_SOUND_SOURCE_RECEIPT&
			PinnedPatternSoundSourceReceipt,
		std::string& strOutStatus);
	bool Prepare_NextCommand(
		std::string_view strBossPlacementId,
		const LostArk::Shared::GameplayDataRevision&
			expectedDefinitionRevision,
		LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& Request,
		std::string& strOutStatus) const;
	void Accept_NextCommand(uint32_t iRoomAuditionEpoch);
	void Apply_ServerResult(const LostArk::Shared::S2C_VALTAN_AUDITION_RESULT& Result);
	void Apply_ServerLifecycle(const LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE& Lifecycle);
	bool Apply_NextLifecycle(const LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE& Lifecycle);
	void Promote_NextPattern();
	void Abort_WorldSession(const std::string& strReason);
	void Advance_RequestSequence();
	[[nodiscard]] uint64_t Now_Milliseconds() const;
	[[nodiscard]] bool Is_Connected() const;
	[[nodiscard]] uint64_t World_InboundGeneration() const;
	[[nodiscard]] bool Is_FlowInFlight() const;
	[[nodiscard]] bool Is_FlowStartPending() const;
	[[nodiscard]] bool Read_LivePatternSequence(uint32_t& iOutPatternSequence) const;
	[[nodiscard]] bool Is_PresentationAvailable(const LostArk::Shared::GameplayDataRevision& Revision) const;
	bool Send_Request(const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& Request);
	bool Consume_Result(LostArk::Shared::S2C_VALTAN_AUDITION_RESULT& Result);
	bool Consume_Lifecycle(LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE& Lifecycle);

	VALTAN_PATTERN_AUDITION_SNAPSHOT m_Snapshot;
	/* Keep the exact predecessor until the Server either admits a replacement
	   or explicitly proves that preflight preserved the old occurrence. */
	VALTAN_PATTERN_AUDITION_SNAPSHOT m_RestartFallback;
	VALTAN_NEXT_PATTERN_SNAPSHOT m_NextSnapshot;
	VALTAN_NEXT_PATTERN_COMMAND m_NextCommand;
	uint32_t m_iNextRequestSequence = 1u;
	uint64_t m_iStateStartedAtMilliseconds = 0u;
	bool m_hasAuthoritativeLifecycle = false;
	bool m_hasRestartFallback = false;
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	VALTAN_AUDITION_HARNESS_INPUT m_HarnessInput;
#endif
};

const char* Describe_ValtanPatternAuditionState(
	VALTAN_PATTERN_AUDITION_STATE eState);
const char* Describe_ValtanNextPatternState(VALTAN_NEXT_PATTERN_STATE eState);
const char* Describe_ValtanNextCommandState(VALTAN_NEXT_COMMAND_STATE eState);

}

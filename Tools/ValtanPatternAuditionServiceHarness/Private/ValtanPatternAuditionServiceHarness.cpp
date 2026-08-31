#include "ValtanPatternAuditionService.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"

#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

int Run_ValtanPresentationContractTests();
int Run_ValtanEncounterReferenceContractTests();
int Run_ValtanCanonicalGraphContractTests();
int Run_ValtanPatternSoundCueDocumentContractTests();
int Run_ValtanPatternAnimationBindingDocumentContractTests();
int Run_ValtanPatternEffectCueAuthoringContractTests();
int Run_ValtanPresentationGenerationAdmissionContractTests();

using namespace Client;
using namespace LostArk::Shared;

int Run_ValtanPatternFlowServiceTests();
int Run_ValtanTuningCommandServiceTests();

namespace
{
	constexpr const char* BOSS = "boss.valtan.center";
	constexpr const char* A = "VALTAN_FOUR_SLASH";
	constexpr const char* B = "VALTAN_FIST_IN_OUT";
	constexpr const char* C = "VALTAN_TRASH";
	constexpr uint32_t EPOCH = 17u;
	constexpr uint32_t SEQUENCE = 31u;

	GameplayDataRevision ActiveRevision()
	{
		GameplayDataRevision Revision{};
		Revision.Bytes.fill(0x42u);
		return Revision;
	}

	VALTAN_PATTERN_SOUND_SOURCE_RECEIPT SoundReceipt(
		const char Value = 'a')
	{
		return { std::string(64u, Value), 1u };
	}

	void Require(bool Condition, const char* Message)
	{
		if (!Condition)
			throw std::runtime_error(Message);
	}

	S2C_VALTAN_AUDITION_RESULT Verdict(
		const C2S_VALTAN_AUDITION_REQUEST& Request,
		VALTAN_AUDITION_RESULT Result = VALTAN_AUDITION_RESULT::QUEUED)
	{
		S2C_VALTAN_AUDITION_RESULT Out{};
		Out.iRequestSequence = Request.iRequestSequence;
		Out.eOperation = Request.eOperation;
		Out.iTargetHealthBar = Request.iTargetHealthBar;
		Out.strBossPlacementId = Request.strBossPlacementId;
		Out.strPatternId = Request.strPatternId;
		Out.iPredecessorRoomAuditionEpoch = Request.iPredecessorRoomAuditionEpoch;
		Out.iPredecessorPatternSequence = Request.iPredecessorPatternSequence;
		Out.iExpectedNextRequestSequence = Request.iExpectedNextRequestSequence;
		Out.ExpectedDefinitionRevision = Request.ExpectedDefinitionRevision;
		Out.eResult = Result;
		return Out;
	}

	S2C_VALTAN_AUDITION_LIFECYCLE Event(
		const C2S_VALTAN_AUDITION_REQUEST& Request,
		VALTAN_AUDITION_LIFECYCLE_STATE State)
	{
		S2C_VALTAN_AUDITION_LIFECYCLE Out{};
		Out.iRequestSequence = Request.iRequestSequence;
		Out.iRoomAuditionEpoch =
			(VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID == Request.eOperation ||
			 VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID == Request.eOperation) ?
				EPOCH : Request.iPredecessorRoomAuditionEpoch;
		Out.iPatternSequence = VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID == Request.eOperation ?
			SEQUENCE : Request.iPredecessorPatternSequence + 1u;
		Out.strPatternId = Request.strPatternId;
		Out.eState = State;
		Out.PinnedDefinitionRevision = ActiveRevision();
		if (VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED == State)
			Out.strReason = "harness terminal abort";
		return Out;
	}

	std::vector<uint8_t> Wire(const C2S_VALTAN_AUDITION_REQUEST& Request)
	{
		CPacketWriter Writer;
		Require(Write_Message(Writer, Request), "production request codec rejected a service command");
		return Writer.Get_Buffer();
	}

	struct Fixture
	{
		CValtanPatternAuditionService& Service = CValtanPatternAuditionService::Get();
		std::string Status;
		C2S_VALTAN_AUDITION_REQUEST Current;

		Fixture() { Service.Harness_Reset(); }
		auto& Input() { return Service.Harness_Input(); }

		void StartAt(
			const char* Pattern,
			const uint32_t PatternSequence,
			const char* Consumer = "Boss Tool")
		{
			Require(Service.Submit(
				Consumer, BOSS, Pattern, ActiveRevision(), Status),
				"initial Play failed");
			Current = Input().SentRequests.back();
			Input().Results.push_back(Verdict(Current));
			auto Active = Event(Current, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE);
			Active.iPatternSequence = PatternSequence;
			Input().Lifecycles.push_back(Active);
			Service.Update();
			Require(Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE,
				"initial Play did not activate");
		}

		void Start(const char* Pattern = A, const char* Consumer = "Boss Tool")
		{
			StartAt(Pattern, SEQUENCE, Consumer);
		}

		C2S_VALTAN_AUDITION_REQUEST Queue(const char* Pattern = B)
		{
			Require(Service.Queue_NextPattern(
				"Boss Tool", BOSS, Pattern, ActiveRevision(), Status),
				"Next command failed");
			return Input().SentRequests.back();
		}

		C2S_VALTAN_AUDITION_REQUEST Reserve(const char* Pattern = B)
		{
			auto Request = Queue(Pattern);
			Input().Results.push_back(Verdict(Request));
			Service.Update();
			Require(Service.Get_NextSnapshot().Is_Live(), "approved reservation was not live");
			return Request;
		}

		void Complete()
		{
			Input().Lifecycles.push_back(Event(Current, VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED));
			Service.Update();
		}

		void Advance(uint64_t Milliseconds)
		{
			Input().iNowMilliseconds += Milliseconds;
			Service.Update();
		}
	};

	void VerifyAuthoritativePredecessorAndNoReset()
	{
		Fixture F;
		Require(!F.Service.Queue_NextPattern(
			"Boss Tool", BOSS, B, ActiveRevision(), F.Status),
			"Next accepted without a live boss or audition");
		Require(F.Service.Submit(
			"Effect Tool", BOSS, A, ActiveRevision(), F.Status),
			"Play failed");
		F.Current = F.Input().SentRequests.back();
		Require(!F.Service.Queue_NextPattern(
			"Boss Tool", BOSS, B, ActiveRevision(), F.Status),
			"Next accepted without an authoritative predecessor epoch");
		F.Input().Lifecycles.push_back(Event(F.Current, VALTAN_AUDITION_LIFECYCLE_STATE::PENDING));
		F.Service.Update();
		auto StaleRevision = ActiveRevision();
		StaleRevision.Bytes.front() ^= 0xffu;
		Require(!F.Service.Queue_NextPattern(
			"Boss Tool", BOSS, B, StaleRevision, F.Status),
			"Next accepted a revision other than the isolated predecessor pin");
		const auto Next = F.Queue();
		Require(Next.eOperation == VALTAN_AUDITION_OPERATION::QUEUE_NEXT_PATTERN_ID &&
			Next.iPredecessorRoomAuditionEpoch == EPOCH &&
			Next.iPredecessorPatternSequence == SEQUENCE &&
			Next.iExpectedNextRequestSequence == 0u &&
			Next.ExpectedDefinitionRevision == ActiveRevision(),
			"Next did not carry the authoritative predecessor");
		Require(F.Input().SentRequests.size() == 2u &&
			F.Service.Get_Snapshot().iRequestSequence == F.Current.iRequestSequence &&
			F.Service.Get_Snapshot().strConsumerId == "Effect Tool" &&
			!F.Service.Get_NextSnapshot().Is_Live(), "Queue reset or overwrote the current owner");
		(void)Wire(Next);
	}

	void VerifyCompletionBeforeVerdictAndPending()
	{
		Fixture F;
		F.Start();
		auto Next = F.Queue();
		F.Complete();
		Require(F.Service.Has_PlaybackOwnership(), "completion dropped pending Next ownership");
		F.Input().Results.push_back(Verdict(Next));
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::PENDING));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::COMPLETED &&
			F.Service.Get_NextSnapshot().eState == VALTAN_NEXT_PATTERN_STATE::START_PENDING &&
			F.Service.Get_NextSnapshot().bReservationConsumed,
			"Next PENDING replaced the completed current snapshot");
		const auto SentBeforeConsumedControls = F.Input().SentRequests.size();
		Require(!F.Service.Clear_NextPattern(F.Status) &&
			!F.Service.Queue_NextPattern(
				"Boss Tool", BOSS, C, ActiveRevision(), F.Status) &&
			SentBeforeConsumedControls == F.Input().SentRequests.size(),
			"promoted Next sent a clear or replacement against the consumed token");
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER));
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().eState == VALTAN_NEXT_PATTERN_STATE::WAITING_FOR_PLAYER &&
			F.Service.Get_NextSnapshot().bReservationConsumed &&
			!F.Service.Clear_NextPattern(F.Status) &&
			!F.Service.Queue_NextPattern(
				"Effect Tool", BOSS, C, ActiveRevision(), F.Status) &&
			SentBeforeConsumedControls == F.Input().SentRequests.size(),
			"target loss hid WAITING or reopened a consumed reservation");
		F.Advance(60000u);
		Require(F.Service.Get_NextSnapshot().Is_Live(), "Next inherited Play's 15s timeout");
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().strPatternId == B &&
			F.Service.Get_Snapshot().iRequestSequence == Next.iRequestSequence &&
			F.Service.Get_Snapshot().iObservedPatternSequence == SEQUENCE + 1u &&
			!F.Service.Get_NextSnapshot().Is_Live(), "Next ACTIVE did not transfer its exact identity");
		const auto Following = F.Queue(C);
		Require(Following.iPredecessorPatternSequence == SEQUENCE + 1u &&
			Following.iExpectedNextRequestSequence == 0u,
			"ACTIVE did not reopen Next with the promoted predecessor identity");
	}

	void VerifyVerdictBeforeCompletionAndDeadPlayerWait()
	{
		Fixture F;
		F.Start();
		auto Next = F.Reserve();
		F.Advance(60000u);
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE &&
			F.Service.Get_NextSnapshot().eState == VALTAN_NEXT_PATTERN_STATE::RESERVED,
			"long current pattern expired Next");
		F.Complete();
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER));
		F.Service.Update();
		F.Advance(120000u);
		Require(F.Service.Get_NextSnapshot().eState == VALTAN_NEXT_PATTERN_STATE::WAITING_FOR_PLAYER &&
			!F.Service.Get_NextSnapshot().bReservationConsumed &&
			F.Service.Has_PlaybackOwnership(), "dead-player wait lost reservation ownership");
		Require(!F.Service.Submit(
			"Effect Tool", BOSS, C, ActiveRevision(), F.Status),
			"another tool reset a waiting Next");
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().strPatternId == B, "revive-triggered ACTIVE was not consumed");
	}

	void VerifyLifecycleBeforeVerdict()
	{
		Fixture F;
		F.Start();
		auto Next = F.Queue();
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED));
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().iRequestSequence == Next.iRequestSequence &&
			!F.Service.Has_PendingNextCommand(), "matching lifecycle did not confirm the candidate");
		F.Complete();
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		F.Input().Results.push_back(Verdict(Next));
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE &&
			!F.Service.Get_NextSnapshot().Is_Live(), "late verdict recreated an activated reservation");
	}

	void VerifyRejectedReplacementPreservesReservation()
	{
		Fixture F;
		F.Start();
		auto Next = F.Reserve();
		auto Replacement = F.Queue(C);
		Require(Replacement.iExpectedNextRequestSequence == Next.iRequestSequence &&
			F.Service.Get_NextSnapshot().strPatternId == B, "replacement modified B before approval");
		F.Input().Results.push_back(Verdict(Replacement, VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE));
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().iRequestSequence == Next.iRequestSequence &&
			F.Service.Get_NextSnapshot().strPatternId == B && !F.Service.Has_PendingNextCommand(),
			"rejected replacement erased B");
		F.Complete();
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().strPatternId == B, "B could not activate after C rejection");
	}

	void VerifyAcceptedReplacementIgnoresOldAbort()
	{
		Fixture F;
		F.Start();
		auto Next = F.Reserve();
		F.Complete();
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER));
		F.Service.Update();
		auto Replacement = F.Queue(C);
		F.Input().Results.push_back(Verdict(Replacement));
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED));
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().strPatternId == C &&
			F.Service.Get_NextSnapshot().eState == VALTAN_NEXT_PATTERN_STATE::RESERVED,
			"old B abort erased accepted C");
		F.Complete();
		F.Input().Lifecycles.push_back(Event(Replacement, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().iRequestSequence == Replacement.iRequestSequence,
			"old B ACTIVE replaced C");
	}

	void VerifyClearTimeoutRetryAndReplayedVerdict()
	{
		Fixture F;
		F.Start();
		auto Next = F.Reserve();
		F.Complete();
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER));
		F.Service.Update();
		Require(F.Service.Clear_NextPattern(F.Status), "clear failed");
		const auto Clear = F.Input().SentRequests.back();
		Require(Clear.eOperation == VALTAN_AUDITION_OPERATION::CLEAR_NEXT_PATTERN_ID &&
			Clear.iExpectedNextRequestSequence == Next.iRequestSequence &&
			Clear.strPatternId == B, "clear did not name the exact B reservation");
		F.Advance(5001u);
		Require(F.Service.Get_NextCommand().eState == VALTAN_NEXT_COMMAND_STATE::UNCONFIRMED &&
			F.Service.Get_NextSnapshot().Is_Live() && F.Service.Has_PlaybackOwnership(),
			"clear timeout dropped ownership");
		Require(!F.Service.Queue_NextPattern(
			"Boss Tool", BOSS, C, ActiveRevision(), F.Status) &&
			!F.Service.Submit(
				"Effect Tool", BOSS, C, ActiveRevision(), F.Status),
			"unconfirmed clear allowed a new command");
		Require(F.Service.Retry_NextPatternCommand(F.Status), "same clear retry failed");
		Require(Wire(Clear) == Wire(F.Input().SentRequests.back()), "retry changed the original payload");
		F.Input().Results.push_back(Verdict(Clear, VALTAN_AUDITION_RESULT::CLEARED));
		F.Service.Update();
		F.Input().Results.push_back(Verdict(Clear, VALTAN_AUDITION_RESULT::CLEARED));
		F.Service.Update();
		Require(!F.Service.Get_NextSnapshot().Is_Live() && !F.Service.Has_PlaybackOwnership(),
			"replayed CLEARED left ownership behind");
		const auto NewNext = F.Queue(C);
		Require(NewNext.iRequestSequence == Clear.iRequestSequence + 1u,
			"retry allocated a new command sequence");
	}

	void VerifyRejectedClearPreservesReservation()
	{
		Fixture F;
		F.Start();
		auto Next = F.Reserve();
		Require(F.Service.Clear_NextPattern(F.Status), "clear failed");
		const auto Clear = F.Input().SentRequests.back();
		F.Input().Results.push_back(Verdict(Clear, VALTAN_AUDITION_RESULT::REJECTED_NEXT_CHANGED));
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().iRequestSequence == Next.iRequestSequence &&
			F.Service.Get_NextSnapshot().Is_Live(), "clear rejection erased approved B");
	}

	void VerifyFullEchoAndUnexpectedDuplicate()
	{
		Fixture F;
		F.Start();
		auto Next = F.Queue();
		for (int Field = 0; Field < 8; ++Field)
		{
			auto Wrong = Verdict(Next);
			switch (Field)
			{
			case 0: ++Wrong.iRequestSequence; break;
			case 1: Wrong.eOperation = VALTAN_AUDITION_OPERATION::CLEAR_NEXT_PATTERN_ID; break;
			case 2: Wrong.strBossPlacementId = "boss.valtan.other"; break;
			case 3: Wrong.strPatternId = C; break;
			case 4: ++Wrong.iPredecessorRoomAuditionEpoch; break;
			case 5: ++Wrong.iPredecessorPatternSequence; break;
			case 6: ++Wrong.iExpectedNextRequestSequence; break;
			case 7: ++Wrong.iTargetHealthBar; break;
			}
			F.Input().Results.push_back(Wrong);
			F.Service.Update();
			Require(F.Service.Has_PendingNextCommand() && !F.Service.Get_NextSnapshot().Is_Live(),
				"partial echoed identity was accepted");
		}
		F.Input().Results.push_back(Verdict(Next, VALTAN_AUDITION_RESULT::DUPLICATE_IGNORED));
		F.Service.Update();
		Require(F.Service.Get_NextCommand().eState == VALTAN_NEXT_COMMAND_STATE::UNCONFIRMED &&
			!F.Service.Get_NextSnapshot().Is_Live(), "legacy duplicate verdict was treated as Next approval");
		Require(F.Service.Retry_NextPatternCommand(F.Status), "unconfirmed duplicate could not retry");
		F.Input().Results.push_back(Verdict(Next));
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().Is_Live(), "original QUEUED verdict did not resolve retry");
	}

	void VerifyGenerationAndDisconnectBeforeDrain()
	{
		for (bool Disconnect : { false, true })
		{
			Fixture F;
			F.Start();
			auto Next = F.Reserve();
			F.Complete();
			Require(F.Service.Clear_NextPattern(F.Status), "clear failed");
			const auto Clear = F.Input().SentRequests.back();
			F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
			F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED));
			F.Input().Results.push_back(Verdict(Clear, VALTAN_AUDITION_RESULT::CLEARED));
			if (Disconnect)
				F.Input().bConnected = false;
			else
				++F.Input().iWorldInboundGeneration;
			F.Service.Update();
			Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ABORTED &&
				F.Service.Get_NextSnapshot().eState == VALTAN_NEXT_PATTERN_STATE::ABORTED &&
				!F.Service.Has_PlaybackOwnership(), "session change adopted late packets");
			F.Input().bConnected = true;
			F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
			F.Service.Update();
			Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ABORTED,
				"late packet resurrected the previous connection");
			Require(F.Service.Submit(
				"Effect Tool", BOSS, C, ActiveRevision(), F.Status),
				"new world could not start a new audition");
			Require(F.Service.Get_Snapshot().iRequestSequence > Clear.iRequestSequence,
				"new world reused an old request sequence");
		}
	}

	void VerifySamePatternOccurrencesRemainDistinct()
	{
		Fixture F;
		F.Start(A);
		auto Next = F.Reserve(A);
		F.Complete();
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		F.Input().Lifecycles.push_back(Event(F.Current, VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE &&
			F.Service.Get_Snapshot().iRequestSequence == Next.iRequestSequence &&
			F.Service.Get_Snapshot().iObservedPatternSequence == SEQUENCE + 1u,
			"A completion terminated the second A");
		F.Service.Harness_ObserveBoss(true, C, SEQUENCE + 3u);
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE,
			"HUD inference completed an authoritative Next");
		auto Wrong = Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED);
		++Wrong.iRoomAuditionEpoch;
		F.Input().Lifecycles.push_back(Wrong);
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE,
			"mismatched epoch completed the second A");
	}

	void VerifyFlowConflictSendRollbackAndExhaustion()
	{
		Fixture F;
		F.Input().bFlowInFlight = true;
		Require(!F.Service.Submit(
			"Boss Tool", BOSS, A, ActiveRevision(), F.Status) &&
			F.Input().SentRequests.empty(),
			"Play bypassed ordered Flow ownership");
		F.Input().bFlowInFlight = false;
		F.Start();
		auto Next = F.Reserve();
		F.Input().bFlowInFlight = true;
		F.Input().bFlowStartPending = true;
		Require(!F.Service.Queue_NextPattern(
			"Boss Tool", BOSS, C, ActiveRevision(), F.Status),
			"Next bypassed pending Flow restart");
		F.Input().bFlowStartPending = false;
		F.Input().bFlowInFlight = false;
		F.Input().bSendSucceeds = false;
		Require(!F.Service.Queue_NextPattern(
			"Boss Tool", BOSS, C, ActiveRevision(), F.Status) &&
			F.Service.Get_NextSnapshot().iRequestSequence == Next.iRequestSequence &&
			!F.Service.Has_PendingNextCommand(), "send failure mutated an approved reservation");
		F.Input().bSendSucceeds = true;
		F.Service.Harness_SetNextRequestSequence((std::numeric_limits<uint32_t>::max)());
		auto Last = F.Queue(C);
		F.Input().Results.push_back(Verdict(Last, VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE));
		F.Service.Update();
		const auto Sent = F.Input().SentRequests.size();
		Require(!F.Service.Clear_NextPattern(F.Status) && F.Input().SentRequests.size() == Sent,
			"request counter wrapped into a reused identity");
	}

	void VerifyNextAbortAndPresentationIsolation()
	{
		Fixture F;
		F.Start();
		auto Next = F.Reserve();
		F.Complete();
		F.Input().bPresentationAvailable = false;
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE &&
			!F.Service.Get_Snapshot().isPresentationRevisionAvailable,
			"missing presentation changed authoritative playback");
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED));
		F.Service.Update();
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ABORTED &&
			!F.Service.Has_PlaybackOwnership(), "aborted Next resurrected");
	}

	void VerifyLiveProductWithoutIsolatedPlay()
	{
		Fixture F;
		F.Input().bLiveBossValid = true;
		F.Input().iLivePatternSequence = SEQUENCE;
		Require(F.Service.Can_QueueNextPattern(
			BOSS, ActiveRevision(), F.Status),
			"live Product Next picker was disabled");
		const auto Next = F.Queue();
		Require(Next.eOperation == VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID &&
			Next.iPredecessorRoomAuditionEpoch == 0u && Next.iExpectedNextRequestSequence == 0u &&
			Next.iPredecessorPatternSequence == SEQUENCE, "live Next invented an audition predecessor");
		Require(F.Input().SentRequests.size() == 1u &&
			F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::IDLE &&
			F.Service.Has_PlaybackOwnership(), "live Next reset or fabricated the current audition");
		(void)Wire(Next);
		F.Input().Results.push_back(Verdict(Next));
		F.Service.Update();
		Require(F.Service.Has_PendingNextCommand() && !F.Service.Get_NextSnapshot().Is_Live(),
			"epoch-zero echo was treated as an authoritative reservation");
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED));
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().iRoomAuditionEpoch == EPOCH &&
			F.Service.Get_NextSnapshot().iPredecessorPatternSequence == SEQUENCE &&
			F.Service.Get_NextSnapshot().iExpectedPatternSequence == SEQUENCE + 1u &&
			!F.Service.Has_PendingNextCommand(), "live reservation did not adopt its Server-issued epoch");
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().strPatternId == B &&
			F.Service.Get_Snapshot().iObservedPatternSequence == SEQUENCE + 1u,
			"live Next did not promote its authoritative occurrence");
		const auto Following = F.Queue(C);
		Require(Following.eOperation == VALTAN_AUDITION_OPERATION::QUEUE_NEXT_PATTERN_ID &&
			Following.iPredecessorRoomAuditionEpoch == EPOCH &&
			Following.iPredecessorPatternSequence == SEQUENCE + 1u,
			"promoted live Next did not continue the same isolated chain");
	}

	void VerifyLiveEpochIdentityAndExactRetry()
	{
		Fixture F;
		F.Input().bLiveBossValid = true;
		F.Input().iLivePatternSequence = SEQUENCE;
		const auto Next = F.Queue();
		for (int Field = 0; Field < 4; ++Field)
		{
			auto Wrong = Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED);
			switch (Field)
			{
			case 0: ++Wrong.iRequestSequence; break;
			case 1: Wrong.iRoomAuditionEpoch = 0u; break;
			case 2: ++Wrong.iPatternSequence; break;
			case 3: Wrong.strPatternId = C; break;
			}
			F.Input().Lifecycles.push_back(Wrong);
			F.Service.Update();
			Require(F.Service.Has_PendingNextCommand() && !F.Service.Get_NextSnapshot().Is_Live(),
				"mismatched live lifecycle claimed the reservation");
		}
		auto WrongEcho = Verdict(Next);
		WrongEcho.iPredecessorRoomAuditionEpoch = EPOCH;
		F.Input().Results.push_back(WrongEcho);
		F.Service.Update();
		Require(F.Service.Has_PendingNextCommand(), "live result accepted a non-echoed epoch");
		F.Input().Results.push_back(Verdict(Next));
		F.Service.Update();
		F.Advance(5001u);
		Require(F.Service.Get_NextCommand().eState == VALTAN_NEXT_COMMAND_STATE::UNCONFIRMED &&
			F.Service.Has_PlaybackOwnership(), "missing live lifecycle silently dropped ownership");
		Require(F.Service.Retry_NextPatternCommand(F.Status) &&
			Wire(Next) == Wire(F.Input().SentRequests.back()), "live retry changed its original request");
		F.Input().bPresentationAvailable = false;
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED));
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().Is_Live() &&
			!F.Service.Get_NextSnapshot().isPresentationRevisionAvailable &&
			!F.Service.Has_PendingNextCommand(), "matching live lifecycle could not resolve the retry");
	}

	void VerifyLiveLifecycleBeforeVerdict()
	{
		for (const auto State : { VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED,
			VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER,
			VALTAN_AUDITION_LIFECYCLE_STATE::PENDING,
			VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE,
			VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED,
			VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED })
		{
			Fixture F;
			F.Input().bLiveBossValid = true;
			F.Input().iLivePatternSequence = SEQUENCE;
			const auto Next = F.Queue();
			F.Input().Lifecycles.push_back(Event(Next, State));
			F.Service.Update();
			Require(!F.Service.Has_PendingNextCommand(), "live lifecycle could not overtake its verdict");
			const bool bPromoted = VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE == State ||
				VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED == State;
			if (bPromoted)
				Require(F.Service.Get_Snapshot().iRequestSequence == Next.iRequestSequence &&
					F.Service.Get_Snapshot().iRoomAuditionEpoch == EPOCH,
					"live occurrence lost its identity without RESERVED or PENDING");
			else
				Require(F.Service.Get_NextSnapshot().iRequestSequence == Next.iRequestSequence &&
					F.Service.Get_NextSnapshot().iRoomAuditionEpoch == EPOCH,
					"live reservation lost its identity without a verdict");
			F.Input().Results.push_back(Verdict(Next));
			F.Service.Update();
			Require(!F.Service.Has_PendingNextCommand() &&
				(bPromoted || VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED == State ?
					!F.Service.Get_NextSnapshot().Is_Live() : F.Service.Get_NextSnapshot().Is_Live()),
				"late live verdict resurrected or regressed the reservation");
		}
	}

	void VerifyLiveReservationReplacementAndClear()
	{
		Fixture F;
		F.Input().bLiveBossValid = true;
		F.Input().iLivePatternSequence = SEQUENCE;
		const auto Next = F.Queue();
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED));
		F.Service.Update();
		const auto Pin = F.Service.Get_NextSnapshot().PinnedDefinitionRevision;
		const auto Replacement = F.Queue(C);
		Require(Replacement.eOperation == VALTAN_AUDITION_OPERATION::QUEUE_NEXT_PATTERN_ID &&
			Replacement.iPredecessorRoomAuditionEpoch == EPOCH &&
			Replacement.iPredecessorPatternSequence == SEQUENCE &&
			Replacement.iExpectedNextRequestSequence == Next.iRequestSequence &&
			F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::IDLE,
			"replacement needed a fabricated current audition instead of the reservation tuple");
		F.Input().Results.push_back(Verdict(Replacement, VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE));
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().iRequestSequence == Next.iRequestSequence,
			"rejected live replacement erased the approved reservation");
		const auto Accepted = F.Queue(C);
		F.Input().Results.push_back(Verdict(Accepted));
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().PinnedDefinitionRevision == Pin,
			"live replacement lost its pin because there was no current audition snapshot");
		Require(F.Service.Clear_NextPattern(F.Status), "live reservation could not be cancelled");
		const auto Clear = F.Input().SentRequests.back();
		Require(Clear.iPredecessorRoomAuditionEpoch == EPOCH &&
			Clear.iPredecessorPatternSequence == SEQUENCE &&
			Clear.iExpectedNextRequestSequence == Accepted.iRequestSequence &&
			Clear.ExpectedDefinitionRevision == Pin && Clear.strPatternId == C,
			"live clear did not use the adopted epoch and exact reservation identity");
		(void)Wire(Replacement);
		(void)Wire(Clear);
		F.Input().Results.push_back(Verdict(Clear, VALTAN_AUDITION_RESULT::CLEARED));
		F.Service.Update();
		Require(!F.Service.Has_PlaybackOwnership(), "live cancellation kept stale playback ownership");
	}

	void VerifyLiveIdleConsumedPlayerWait()
	{
		Fixture F;
		F.Input().bLiveBossValid = true;
		const auto Next = F.Queue();
		Require(Next.iPredecessorPatternSequence == 0u, "initial idle invented a completed predecessor");
		(void)Wire(Next);
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED));
		F.Service.Update();
		Require(!F.Service.Can_QueueNextPattern(
			BOSS, ActiveRevision(), F.Status) &&
			!F.Service.Clear_NextPattern(F.Status), "initial idle issued a forbidden zero-sequence CAS control");
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::PENDING));
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER));
		F.Service.Update();
		F.Advance(60000u);
		Require(F.Service.Get_NextSnapshot().eState == VALTAN_NEXT_PATTERN_STATE::WAITING_FOR_PLAYER &&
			F.Service.Get_NextSnapshot().bReservationConsumed &&
			!F.Service.Clear_NextPattern(F.Status) &&
			!F.Service.Queue_NextPattern(
				"Boss Tool", BOSS, C, ActiveRevision(), F.Status),
			"idle player wait timed out or reopened the consumed reservation");
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().iObservedPatternSequence == 1u &&
			F.Service.Get_Snapshot().iRoomAuditionEpoch == EPOCH && F.Input().SentRequests.size() == 1u,
			"idle Next did not start sequence one without a reset command");
	}

	void VerifyLiveFlowAndReadinessRevalidation()
	{
		Fixture F;
		F.Input().bLiveBossValid = true;
		F.Input().iLivePatternSequence = SEQUENCE;
		F.Input().bFlowInFlight = true;
		Require(F.Service.Can_QueueNextPattern(
			BOSS, ActiveRevision(), F.Status),
			"active Flow disabled live Next");
		F.Input().bFlowStartPending = true;
		Require(!F.Service.Can_QueueNextPattern(
			BOSS, ActiveRevision(), F.Status) &&
			!F.Service.Queue_NextPattern(
				"Boss Tool", BOSS, B, ActiveRevision(), F.Status),
			"Next submission did not recheck a newly pending Flow restart");
		F.Input().bFlowStartPending = false;
		const auto Rejected = F.Queue();
		Require(Rejected.eOperation == VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID,
			"active Flow used isolated Next without an authoritative epoch");
		F.Input().Results.push_back(Verdict(Rejected, VALTAN_AUDITION_RESULT::REJECTED_NOT_OWNER));
		F.Service.Update();
		Require(!F.Service.Has_PlaybackOwnership() &&
			F.Service.Get_NextSnapshot().eState == VALTAN_NEXT_PATTERN_STATE::REJECTED &&
			F.Input().bFlowInFlight, "rejected Next cancelled the original Flow locally");
		F.Input().bLiveBossAlive = false;
		Require(!F.Service.Can_QueueNextPattern(
			BOSS, ActiveRevision(), F.Status), "dead live boss enabled Next");
		F.Input().bLiveBossAlive = true;
		F.Input().iLivePatternSequence = (std::numeric_limits<uint32_t>::max)();
		Require(!F.Service.Can_QueueNextPattern(
			BOSS, ActiveRevision(), F.Status), "live predecessor sequence wrapped");
		F.Input().iLivePatternSequence = SEQUENCE;
		F.Input().bSendSucceeds = false;
		const auto Sent = F.Input().SentRequests.size();
		Require(!F.Service.Queue_NextPattern(
			"Boss Tool", BOSS, B, ActiveRevision(), F.Status) &&
			!F.Service.Has_PendingNextCommand() && F.Input().SentRequests.size() == Sent,
			"failed live send created ownership or consumed a request identity");
		F.Input().bSendSucceeds = true;
		const auto Next = F.Queue();
		Require(Next.iRequestSequence == Rejected.iRequestSequence + 1u,
			"failed live send consumed a request sequence");
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED));
		F.Service.Update();
		F.Input().bFlowInFlight = false;
		Require(F.Service.Get_NextSnapshot().Is_Live(), "live Flow Next lost ownership on Flow termination");
	}

	void VerifyNewWorldLiveNextAfterOldAudition()
	{
		for (const bool bPreviousLive : { false, true })
		{
			Fixture F;
			F.Input().bLiveBossValid = true;
			F.Input().iLivePatternSequence = SEQUENCE;
			C2S_VALTAN_AUDITION_REQUEST OldRequest{};
			if (bPreviousLive)
				OldRequest = F.Queue();
			else
			{
				F.Start();
				OldRequest = F.Current;
				F.Complete();
			}
			++F.Input().iWorldInboundGeneration;
			F.Input().Lifecycles.push_back(Event(OldRequest, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
			F.Service.Update();
			Require(!F.Service.Has_PlaybackOwnership(), "world change retained old live command ownership");
			const auto Next = F.Queue(C);
			F.Input().Results.push_back(Verdict(OldRequest));
			F.Input().Lifecycles.push_back(Event(OldRequest, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
			F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
			F.Service.Update();
			Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE &&
				F.Service.Get_Snapshot().iRequestSequence == Next.iRequestSequence &&
				F.Service.Get_Snapshot().iWorldInboundGeneration == F.Input().iWorldInboundGeneration,
				"old terminal generation kept discarding the new live Next lifecycle");
		}
	}

	void VerifyFlowResetInvalidatesCompletedAudition()
	{
		Fixture F;
		F.Start();
		F.Complete();
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::COMPLETED,
			"fixture did not retain the isolated completed hold");
		F.Input().bFlowInFlight = true;
		F.Input().Lifecycles.push_back(Event(F.Current, VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ABORTED,
			"Flow reset did not retire the exact completed isolated epoch");
		F.Input().bFlowInFlight = false;
		F.Input().bLiveBossValid = true;
		F.Input().iLivePatternSequence = SEQUENCE + 8u;
		const auto Next = F.Queue();
		Require(Next.eOperation == VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID &&
			Next.iPredecessorRoomAuditionEpoch == 0u &&
			Next.iPredecessorPatternSequence == F.Input().iLivePatternSequence &&
			Next.iExpectedNextRequestSequence == 0u,
			"Next after completed Flow reused a retired isolated predecessor tuple");
		(void)Wire(Next);
		auto LiveReservation = Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED);
		F.Input().Lifecycles.push_back(LiveReservation);
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().Is_Live() &&
			F.Service.Get_NextSnapshot().PinnedDefinitionRevision ==
				Next.ExpectedDefinitionRevision,
			"live Next did not retain the exact definition revision sent in its CAS request");
		auto WrongRevision = Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER);
		WrongRevision.PinnedDefinitionRevision.Bytes.fill(0x43u);
		F.Input().Lifecycles.push_back(WrongRevision);
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().eState == VALTAN_NEXT_PATTERN_STATE::RESERVED &&
			F.Service.Get_NextSnapshot().PinnedDefinitionRevision ==
				LiveReservation.PinnedDefinitionRevision,
			"a later live Next lifecycle changed its pinned definition revision");
	}

	void VerifyDataDrivenOccurrencesRetainRequestAndAdvanceNextPredecessor()
	{
		for (const char* PatternId : { A, "VALTAN_GHOST_FINALE" })
		{
			for (const uint32_t Initial : { SEQUENCE, (std::numeric_limits<uint32_t>::max)() })
			{
				for (const auto TerminalState : {
					VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED,
					VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED })
				{
					Fixture F;
					F.StartAt(PatternId, Initial);
					auto First = Event(F.Current, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE);
					First.iPatternSequence = Initial;
					auto ActiveOccurrence = First;
					ActiveOccurrence.iPatternSequence =
						Initial == (std::numeric_limits<uint32_t>::max)() ? 1u : Initial + 1u;
					F.Input().Lifecycles.push_back(ActiveOccurrence);
					F.Service.Update();
					Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE &&
						F.Service.Get_Snapshot().iObservedPatternSequence == ActiveOccurrence.iPatternSequence &&
						F.Service.Get_Snapshot().iRequestSequence == F.Current.iRequestSequence &&
						F.Service.Get_Snapshot().iRoomAuditionEpoch == EPOCH,
						"data-driven occurrence lost its request/epoch or failed wrap-safe advance");

					auto Stale = First;
					auto Ambiguous = ActiveOccurrence;
					Ambiguous.iPatternSequence += 0x80000000u;
					auto WrongEpoch = ActiveOccurrence;
					++WrongEpoch.iPatternSequence;
					++WrongEpoch.iRoomAuditionEpoch;
					auto WrongRequest = ActiveOccurrence;
					++WrongRequest.iPatternSequence;
					++WrongRequest.iRequestSequence;
					auto WrongPattern = ActiveOccurrence;
					++WrongPattern.iPatternSequence;
					WrongPattern.strPatternId = B;
					auto WrongRevision = ActiveOccurrence;
					++WrongRevision.iPatternSequence;
					WrongRevision.PinnedDefinitionRevision.Bytes.fill(0x43u);
					auto ForwardTerminal = ActiveOccurrence;
					++ForwardTerminal.iPatternSequence;
					ForwardTerminal.eState = VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED;
					for (const auto& Invalid : { Stale, Ambiguous, WrongEpoch, WrongRequest,
						WrongPattern, WrongRevision, ForwardTerminal })
					{
						F.Input().Lifecycles.push_back(Invalid);
					}
					F.Service.Update();
					Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE &&
						F.Service.Get_Snapshot().iObservedPatternSequence == ActiveOccurrence.iPatternSequence,
						"stale, ambiguous, foreign, or forward-terminal lifecycle changed the occurrence");

					auto PendingOccurrence = ActiveOccurrence;
					++PendingOccurrence.iPatternSequence;
					PendingOccurrence.eState = VALTAN_AUDITION_LIFECYCLE_STATE::PENDING;
					F.Input().Lifecycles.push_back(PendingOccurrence);
					F.Service.Update();
					Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::QUEUED &&
						F.Service.Get_Snapshot().iObservedPatternSequence == PendingOccurrence.iPatternSequence,
						"new PENDING occurrence did not become the authoritative cursor");
					const auto Next = F.Queue();
					Require(Next.iPredecessorPatternSequence == PendingOccurrence.iPatternSequence &&
						Next.iPredecessorRoomAuditionEpoch == EPOCH,
						"Next used an occurrence before the last emitted PENDING cursor");
					(void)Wire(Next);

					auto Terminal = PendingOccurrence;
					Terminal.eState = TerminalState;
					if (VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED == TerminalState)
						Terminal.strReason = "harness abort after repeated occurrence";
					F.Input().Lifecycles.push_back(Terminal);
					F.Service.Update();
					const VALTAN_PATTERN_AUDITION_STATE ExpectedState =
						VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED == TerminalState ?
						VALTAN_PATTERN_AUDITION_STATE::ABORTED : VALTAN_PATTERN_AUDITION_STATE::COMPLETED;
					Require(F.Service.Get_Snapshot().eState == ExpectedState &&
						F.Service.Get_Snapshot().iObservedPatternSequence == PendingOccurrence.iPatternSequence,
						"terminal lifecycle did not reuse the last emitted occurrence cursor");
				}
			}
		}
	}

	void VerifyInitialPlayTimeoutsStayBounded()
	{
		Fixture F;
		Require(F.Service.Submit(
			"Boss Tool", BOSS, A, ActiveRevision(), F.Status),
			"Play failed");
		F.Advance(5001u);
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ABORTED,
			"initial verdict no longer times out");
		Require(F.Service.Submit(
			"Boss Tool", BOSS, A, ActiveRevision(), F.Status),
			"second Play failed");
		F.Input().Results.push_back(Verdict(F.Input().SentRequests.back()));
		F.Service.Update();
		F.Advance(15001u);
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ABORTED,
			"initial queued start no longer times out");
	}

	void VerifyInitialPlayCarriesExactActiveRevision()
	{
		Fixture F;
		const GameplayDataRevision Revision = ActiveRevision();
		const GameplayDataRevision Missing{};
		Require(!F.Service.Submit(
				"Boss Tool", BOSS, A, Missing, F.Status) &&
			F.Input().SentRequests.empty(),
			"Play accepted a missing expected active definition revision");
		Require(F.Service.Submit(
				"Boss Tool", BOSS, A, Revision, F.Status),
			"Play rejected a valid expected active definition revision");
		const C2S_VALTAN_AUDITION_REQUEST Request =
			F.Input().SentRequests.back();
		Require(Request.ExpectedDefinitionRevision == Revision &&
			F.Service.Get_Snapshot().PinnedDefinitionRevision == Revision,
			"Play did not retain the exact expected active revision");

		auto WrongVerdict = Verdict(Request);
		WrongVerdict.ExpectedDefinitionRevision.Bytes[0] ^= 0xffu;
		F.Input().Results.push_back(WrongVerdict);
		auto WrongLifecycle = Event(
			Request, VALTAN_AUDITION_LIFECYCLE_STATE::PENDING);
		WrongLifecycle.PinnedDefinitionRevision.Bytes[0] ^= 0xffu;
		F.Input().Lifecycles.push_back(WrongLifecycle);
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState ==
				VALTAN_PATTERN_AUDITION_STATE::REQUEST_PENDING,
			"Play accepted a verdict or lifecycle for another definition revision");

		F.Input().Results.push_back(Verdict(Request));
		F.Input().Lifecycles.push_back(Event(
			Request, VALTAN_AUDITION_LIFECYCLE_STATE::PENDING));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState ==
				VALTAN_PATTERN_AUDITION_STATE::QUEUED &&
			F.Service.Get_Snapshot().PinnedDefinitionRevision == Revision,
			"Play did not admit the matching exact revision lifecycle");
	}

	void VerifyActiveRestartReplacesExactOccurrence()
	{
		Fixture F;
		F.Start();
		const VALTAN_PATTERN_AUDITION_SNAPSHOT Before =
			F.Service.Get_Snapshot();
		Require(F.Service.Restart_ActivePattern(
			"Boss Tool", BOSS, A, F.Status),
			"exact active restart was rejected");
		const auto Restart = F.Input().SentRequests.back();
		Require(Restart.eOperation ==
				VALTAN_AUDITION_OPERATION::RESTART_PATTERN_ID &&
			Restart.iRequestSequence == Before.iRequestSequence + 1u &&
			Restart.strBossPlacementId == BOSS && Restart.strPatternId == A &&
			Restart.iPredecessorRoomAuditionEpoch == Before.iRoomAuditionEpoch &&
			Restart.iPredecessorPatternSequence == Before.iObservedPatternSequence &&
			Restart.ExpectedDefinitionRevision == Before.PinnedDefinitionRevision &&
			F.Service.Get_Snapshot().eState ==
				VALTAN_PATTERN_AUDITION_STATE::REQUEST_PENDING,
			"restart did not allocate a fresh stable-ID request identity");
		const std::vector<uint8_t> RestartWire = Wire(Restart);
		CPacketReader RestartReader{ RestartWire };
		C2S_VALTAN_AUDITION_REQUEST DecodedRestart{};
		Require(Read_Message(RestartReader, DecodedRestart) &&
			0u == RestartReader.Get_RemainingSize() &&
			DecodedRestart.iRequestSequence == Restart.iRequestSequence &&
			DecodedRestart.eOperation ==
				VALTAN_AUDITION_OPERATION::RESTART_PATTERN_ID &&
			DecodedRestart.iPredecessorRoomAuditionEpoch ==
				Before.iRoomAuditionEpoch &&
			DecodedRestart.iPredecessorPatternSequence ==
				Before.iObservedPatternSequence &&
			DecodedRestart.ExpectedDefinitionRevision ==
				Before.PinnedDefinitionRevision,
			"restart wire dropped an exact predecessor CAS field");
		auto InvalidRestart = Restart;
		InvalidRestart.iPredecessorRoomAuditionEpoch = 0u;
		CPacketWriter InvalidEpochWriter;
		Require(!Write_Message(InvalidEpochWriter, InvalidRestart),
			"restart wire accepted a zero predecessor epoch");
		InvalidRestart = Restart;
		InvalidRestart.iPredecessorPatternSequence = 0u;
		CPacketWriter InvalidSequenceWriter;
		Require(!Write_Message(InvalidSequenceWriter, InvalidRestart),
			"restart wire accepted a zero predecessor sequence");
		InvalidRestart = Restart;
		InvalidRestart.ExpectedDefinitionRevision = {};
		CPacketWriter InvalidRevisionWriter;
		Require(!Write_Message(InvalidRevisionWriter, InvalidRestart),
			"restart wire accepted a missing predecessor revision");
		InvalidRestart = Restart;
		InvalidRestart.iExpectedNextRequestSequence = 7u;
		CPacketWriter HiddenNextWriter;
		Require(!Write_Message(HiddenNextWriter, InvalidRestart),
			"restart wire accepted a hidden Next reservation token");

		/* Server boss-only replacement aborts the old request before emitting the
		   new occurrence. That old edge must not terminate the replacement. */
		F.Input().Lifecycles.push_back(Event(
			F.Current, VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED));
		F.Input().Results.push_back(Verdict(Restart));
		auto Pending = Event(
			Restart, VALTAN_AUDITION_LIFECYCLE_STATE::PENDING);
		Pending.iRoomAuditionEpoch = EPOCH + 1u;
		Pending.iPatternSequence = SEQUENCE + 1u;
		F.Input().Lifecycles.push_back(Pending);
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState ==
				VALTAN_PATTERN_AUDITION_STATE::QUEUED &&
			F.Service.Get_Snapshot().iRequestSequence == Restart.iRequestSequence &&
			F.Service.Get_Snapshot().iRoomAuditionEpoch == EPOCH + 1u &&
			F.Service.Get_Snapshot().iObservedPatternSequence == SEQUENCE + 1u,
			"restart did not ignore the old abort and adopt the new pending occurrence");

		auto Active = Pending;
		Active.eState = VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE;
		F.Input().Lifecycles.push_back(Active);
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState ==
				VALTAN_PATTERN_AUDITION_STATE::ACTIVE &&
			F.Service.Get_Snapshot().iRequestSequence == Restart.iRequestSequence &&
			F.Service.Get_Snapshot().iObservedPatternSequence == SEQUENCE + 1u,
			"replacement occurrence did not become authoritative ACTIVE");
	}

	void VerifyActiveRestartRejectsConflictsAndRollsBack()
	{
		Fixture F;
		F.Start();
		const size_t SentBefore = F.Input().SentRequests.size();
		Require(!F.Service.Restart_ActivePattern(
			"Effect Tool", BOSS, A, F.Status) &&
			!F.Service.Restart_ActivePattern(
				"Boss Tool", "boss.valtan.other", A, F.Status) &&
			!F.Service.Restart_ActivePattern(
				"Boss Tool", BOSS, B, F.Status) &&
			SentBefore == F.Input().SentRequests.size(),
			"restart accepted another consumer or a non-exact active identity");

		F.Input().bFlowInFlight = true;
		Require(!F.Service.Restart_ActivePattern(
			"Boss Tool", BOSS, A, F.Status),
			"restart replaced an ordered Flow");
		F.Input().bFlowInFlight = false;
		F.Input().bFlowStartPending = true;
		Require(!F.Service.Restart_ActivePattern(
			"Boss Tool", BOSS, A, F.Status),
			"restart replaced a pending Flow start");
		F.Input().bFlowStartPending = false;
		(void)F.Reserve();
		Require(!F.Service.Restart_ActivePattern(
			"Boss Tool", BOSS, A, F.Status),
			"restart discarded an approved Next reservation");

		Fixture SendFailure;
		SendFailure.Start();
		const VALTAN_PATTERN_AUDITION_SNAPSHOT BeforeSendFailure =
			SendFailure.Service.Get_Snapshot();
		SendFailure.Input().bSendSucceeds = false;
		Require(!SendFailure.Service.Restart_ActivePattern(
				"Boss Tool", BOSS, A, SendFailure.Status) &&
			SendFailure.Service.Get_Snapshot().eState ==
				VALTAN_PATTERN_AUDITION_STATE::ACTIVE &&
			SendFailure.Service.Get_Snapshot().iRequestSequence ==
				BeforeSendFailure.iRequestSequence,
			"transport failure replaced the active occurrence locally");

		Fixture Rejected;
		Rejected.Start();
		const VALTAN_PATTERN_AUDITION_SNAPSHOT BeforeRejection =
			Rejected.Service.Get_Snapshot();
		Require(Rejected.Service.Restart_ActivePattern(
			"Boss Tool", BOSS, A, Rejected.Status),
			"restart request setup failed");
		const auto Restart = Rejected.Input().SentRequests.back();
		Require(!Rejected.Service.Restart_ActivePattern(
			"Boss Tool", BOSS, A, Rejected.Status),
			"a second restart replaced the pending verdict");
		Rejected.Input().Results.push_back(Verdict(
			Restart, VALTAN_AUDITION_RESULT::REJECTED_OCCURRENCE_PRESERVED));
		Rejected.Service.Update();
		const auto& Restored = Rejected.Service.Get_Snapshot();
		Require(Restored.eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE &&
			Restored.iRequestSequence == BeforeRejection.iRequestSequence &&
			Restored.iRoomAuditionEpoch == BeforeRejection.iRoomAuditionEpoch &&
			Restored.iObservedPatternSequence ==
				BeforeRejection.iObservedPatternSequence &&
			Restored.strStatus.find("previous Server occurrence remains authoritative") !=
				std::string::npos,
			"typed preserved Restart rejection did not restore the prior occurrence");

		Fixture Stale;
		Stale.Start();
		Require(Stale.Service.Restart_ActivePattern(
			"Boss Tool", BOSS, A, Stale.Status),
			"stale restart request setup failed");
		const auto StaleRequest = Stale.Input().SentRequests.back();
		Stale.Input().Results.push_back(Verdict(
			StaleRequest, VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION));
		Stale.Service.Update();
		Require(Stale.Service.Get_Snapshot().eState ==
				VALTAN_PATTERN_AUDITION_STATE::REJECTED &&
			!Stale.Service.Has_PlaybackOwnership() &&
			!Stale.Service.Restart_ActivePattern(
				"Boss Tool", BOSS, A, Stale.Status),
			"stale Restart verdict falsely restored a retired predecessor");
	}

	void VerifyCompletedRestartReplacesExactHold()
	{
		Fixture F;
		F.Start();
		F.Complete();
		const VALTAN_PATTERN_AUDITION_SNAPSHOT Before =
			F.Service.Get_Snapshot();
		Require(Before.eState == VALTAN_PATTERN_AUDITION_STATE::COMPLETED &&
			F.Service.Restart_ActivePattern(
				"Boss Tool", BOSS, A, F.Status),
			"authoritative completed hold could not restart");
		const auto Restart = F.Input().SentRequests.back();
		Require(Restart.eOperation ==
				VALTAN_AUDITION_OPERATION::RESTART_PATTERN_ID &&
			Restart.iPredecessorRoomAuditionEpoch == Before.iRoomAuditionEpoch &&
			Restart.iPredecessorPatternSequence == Before.iObservedPatternSequence &&
			Restart.ExpectedDefinitionRevision == Before.PinnedDefinitionRevision,
			"completed Restart lost its exact predecessor tuple");
		F.Input().Results.push_back(Verdict(Restart));
		auto Pending = Event(Restart, VALTAN_AUDITION_LIFECYCLE_STATE::PENDING);
		Pending.iRoomAuditionEpoch = Before.iRoomAuditionEpoch + 1u;
		Pending.iPatternSequence = Before.iObservedPatternSequence + 1u;
		F.Input().Lifecycles.push_back(Pending);
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState ==
				VALTAN_PATTERN_AUDITION_STATE::QUEUED &&
			F.Service.Get_Snapshot().iRequestSequence == Restart.iRequestSequence &&
			F.Service.Get_Snapshot().iObservedPatternSequence ==
				Before.iObservedPatternSequence + 1u,
			"completed Restart did not adopt the replacement occurrence");
	}

	void VerifyRestartTimeoutKeepsExactRetryIdentity()
	{
		Fixture F;
		F.Start();
		Require(F.Service.Restart_ActivePattern(
			"Boss Tool", BOSS, A, F.Status),
			"restart timeout fixture could not send its first request");
		const C2S_VALTAN_AUDITION_REQUEST Restart =
			F.Input().SentRequests.back();
		/* The predecessor may complete while the Restart verdict is lost. Its
		   old request identity must update the fallback instead of being dropped. */
		F.Input().Lifecycles.push_back(Event(
			F.Current, VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED));
		F.Service.Update();
		F.Advance(5100u);
		Require(F.Service.Get_Snapshot().eState ==
				VALTAN_PATTERN_AUDITION_STATE::RESTART_UNCONFIRMED &&
			F.Service.Has_PlaybackOwnership(),
			"ambiguous restart timeout discarded its in-flight ownership");
		Require(F.Service.Retry_UnconfirmedRestart(F.Status),
			"unconfirmed restart did not permit an exact retry");
		const C2S_VALTAN_AUDITION_REQUEST Retry =
			F.Input().SentRequests.back();
		Require(Retry.iRequestSequence == Restart.iRequestSequence &&
			Retry.eOperation == Restart.eOperation &&
			Retry.strBossPlacementId == Restart.strBossPlacementId &&
			Retry.strPatternId == Restart.strPatternId &&
			Retry.iPredecessorRoomAuditionEpoch ==
				Restart.iPredecessorRoomAuditionEpoch &&
			Retry.iPredecessorPatternSequence ==
				Restart.iPredecessorPatternSequence &&
			Retry.ExpectedDefinitionRevision ==
				Restart.ExpectedDefinitionRevision,
			"restart retry allocated or altered a wire identity");
		F.Input().Results.push_back(Verdict(
			Retry, VALTAN_AUDITION_RESULT::REJECTED_OCCURRENCE_PRESERVED));
		F.Service.Update();
		const auto& Reconciled = F.Service.Get_Snapshot();
		Require(Reconciled.eState == VALTAN_PATTERN_AUDITION_STATE::COMPLETED &&
			!F.Service.Has_PlaybackOwnership() &&
			Reconciled.iRequestSequence == F.Current.iRequestSequence &&
			Reconciled.iObservedPatternSequence == SEQUENCE,
			"preserved exact-retry verdict revived ACTIVE after predecessor completion");

		Fixture Queued;
		Queued.Start();
		Require(Queued.Service.Restart_ActivePattern(
			"Boss Tool", BOSS, A, Queued.Status),
			"queued timeout fixture could not send restart");
		const auto QueuedRequest = Queued.Input().SentRequests.back();
		Queued.Input().Results.push_back(Verdict(QueuedRequest));
		Queued.Service.Update();
		Queued.Advance(15100u);
		Require(Queued.Service.Get_Snapshot().eState ==
				VALTAN_PATTERN_AUDITION_STATE::RESTART_UNCONFIRMED &&
			Queued.Service.Has_PlaybackOwnership(),
			"queued restart lifecycle timeout became a false terminal abort");
	}

	void VerifyPatternSoundReceiptPinsOccurrenceRestartAndAutoNext()
	{
		Fixture Invalid;
		Require(!Invalid.Service.Submit(
				"Boss Tool", BOSS, A, ActiveRevision(),
				VALTAN_PATTERN_SOUND_SOURCE_RECEIPT{}, Invalid.Status) &&
			Invalid.Input().SentRequests.empty(),
			"Play accepted a missing S receipt or sent before fail-closed validation");
		Fixture F;
		const auto SoundA = SoundReceipt('a');
		const auto SoundB = SoundReceipt('b');
		Require(F.Service.Submit(
			"Boss Tool", BOSS, A, ActiveRevision(), SoundA, F.Status),
			"Play rejected a valid exact Pattern Sound receipt");
		F.Current = F.Input().SentRequests.back();
		Require(F.Service.Get_Snapshot().PinnedPatternSoundSourceReceipt ==
				SoundA && F.Service.Has_PatternSoundMutationBarrier(),
			"pending Play did not pin S or expose its mutation barrier");
		F.Input().Results.push_back(Verdict(F.Current));
		F.Input().Lifecycles.push_back(Event(
			F.Current, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().PinnedPatternSoundSourceReceipt ==
				SoundA &&
			F.Service.Verify_PatternSoundSourceReceipt(SoundA, F.Status) &&
			!F.Service.Verify_PatternSoundSourceReceipt(SoundB, F.Status),
			"ACTIVE occurrence did not retain or verify its exact S receipt");
		Require(!F.Service.Restart_ActivePattern(
				"Boss Tool", BOSS, A, SoundB, F.Status) &&
			F.Service.Restart_ActivePattern(
				"Boss Tool", BOSS, A, SoundA, F.Status),
			"Restart did not reject a foreign S or retain the predecessor S");
		F.Advance(5100u);
		Require(!F.Service.Retry_UnconfirmedRestart(SoundB, F.Status) &&
			F.Service.Retry_UnconfirmedRestart(SoundA, F.Status),
			"unconfirmed Restart did not bind exact retry to S");

		Fixture Next;
		Next.Start();
		Require(!Next.Service.Queue_NextPattern(
				"Boss Tool", BOSS, B, ActiveRevision(), SoundB, Next.Status) &&
			Next.Service.Queue_NextPattern(
				"Boss Tool", BOSS, B, ActiveRevision(), SoundA, Next.Status),
			"Next did not enforce its predecessor S receipt");
		const auto NextRequest = Next.Input().SentRequests.back();
		Require(Next.Service.Get_NextCommand().PinnedPatternSoundSourceReceipt ==
			SoundA, "pending Next command did not pin S");
		Next.Input().Results.push_back(Verdict(NextRequest));
		Next.Service.Update();
		Require(Next.Service.Get_NextSnapshot().
				PinnedPatternSoundSourceReceipt == SoundA,
			"approved Next reservation lost S");
		Next.Input().Lifecycles.push_back(Event(
			NextRequest, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
		Next.Service.Update();
		Require(Next.Service.Get_Snapshot().strPatternId == B &&
			Next.Service.Get_Snapshot().PinnedPatternSoundSourceReceipt == SoundA &&
			Next.Service.Verify_PatternSoundSourceReceipt(SoundA, Next.Status),
			"auto-promoted Next occurrence lost its pinned S receipt");
	}
}

int main()
{
	const std::vector<std::pair<const char*, std::function<void()>>> Tests{
		{ "authoritative predecessor and no reset", VerifyAuthoritativePredecessorAndNoReset },
		{ "A completed before B verdict and PENDING", VerifyCompletionBeforeVerdictAndPending },
		{ "B verdict before A completed and player wait", VerifyVerdictBeforeCompletionAndDeadPlayerWait },
		{ "B lifecycle before verdict", VerifyLifecycleBeforeVerdict },
		{ "rejected C preserves B", VerifyRejectedReplacementPreservesReservation },
		{ "accepted C ignores late B abort", VerifyAcceptedReplacementIgnoresOldAbort },
		{ "clear timeout same-payload retry and replay", VerifyClearTimeoutRetryAndReplayedVerdict },
		{ "rejected clear preserves B", VerifyRejectedClearPreservesReservation },
		{ "full echoed identity and legacy duplicate", VerifyFullEchoAndUnexpectedDuplicate },
		{ "generation/disconnect invalidates before drain", VerifyGenerationAndDisconnectBeforeDrain },
		{ "A to A occurrence identity and HUD isolation", VerifySamePatternOccurrencesRemainDistinct },
		{ "Flow conflict send rollback request exhaustion", VerifyFlowConflictSendRollbackAndExhaustion },
		{ "Next abort and presentation isolation", VerifyNextAbortAndPresentationIsolation },
		{ "initial Play 5s/15s timeouts", VerifyInitialPlayTimeoutsStayBounded },
		{ "initial Play exact active definition revision", VerifyInitialPlayCarriesExactActiveRevision },
		{ "exact active restart allocates a new occurrence", VerifyActiveRestartReplacesExactOccurrence },
		{ "active restart rejects conflicts and rolls back", VerifyActiveRestartRejectsConflictsAndRollsBack },
		{ "completed hold restart allocates a new occurrence", VerifyCompletedRestartReplacesExactHold },
		{ "restart timeout preserves exact retry identity", VerifyRestartTimeoutKeepsExactRetryIdentity },
		{ "Pattern Sound S pins Play Restart and auto-Next", VerifyPatternSoundReceiptPinsOccurrenceRestartAndAutoNext },
		{ "all data-driven occurrences keep request identity and refresh Next predecessor", VerifyDataDrivenOccurrencesRetainRequestAndAdvanceNextPredecessor },
		{ "live Product Next without isolated Play", VerifyLiveProductWithoutIsolatedPlay },
		{ "live epoch identity and exact retry", VerifyLiveEpochIdentityAndExactRetry },
		{ "live lifecycle before verdict", VerifyLiveLifecycleBeforeVerdict },
		{ "live reservation replacement and clear", VerifyLiveReservationReplacementAndClear },
		{ "live idle consumed player wait", VerifyLiveIdleConsumedPlayerWait },
		{ "live Flow and readiness revalidation", VerifyLiveFlowAndReadinessRevalidation },
		{ "new world live Next after old audition", VerifyNewWorldLiveNextAfterOldAudition },
		{ "Flow reset retires completed isolated predecessor", VerifyFlowResetInvalidatesCompletedAudition },
	};
	size_t Failed = 0u;
	for (const auto& [Name, Test] : Tests)
	{
		try { Test(); }
		catch (const std::exception& Error)
		{
			++Failed;
			std::cerr << "FAIL " << Name << ": " << Error.what() << '\n';
		}
	}
	std::cout << "ValtanPatternAuditionServiceHarness: " << Tests.size() - Failed
		<< "/" << Tests.size() << " passed\n";
	const int FlowFailures = Run_ValtanPatternFlowServiceTests();
	const int TuningFailures = Run_ValtanTuningCommandServiceTests();
	const int PresentationFailures = Run_ValtanPresentationContractTests();
	const int EncounterReferenceFailures =
		Run_ValtanEncounterReferenceContractTests();
	const int CanonicalGraphFailures =
		Run_ValtanCanonicalGraphContractTests();
	const int PatternSoundCueFailures =
		Run_ValtanPatternSoundCueDocumentContractTests();
	const int AnimationBindingDocumentFailures =
		Run_ValtanPatternAnimationBindingDocumentContractTests();
	const int EffectCueAuthoringFailures =
		Run_ValtanPatternEffectCueAuthoringContractTests();
	const int PresentationGenerationAdmissionFailures =
		Run_ValtanPresentationGenerationAdmissionContractTests();
	return 0u == Failed && 0 == FlowFailures && 0 == TuningFailures &&
		0 == PresentationFailures && 0 == EncounterReferenceFailures &&
		0 == CanonicalGraphFailures &&
		0 == PatternSoundCueFailures &&
		0 == AnimationBindingDocumentFailures &&
		0 == EffectCueAuthoringFailures &&
		0 == PresentationGenerationAdmissionFailures ? 0 : 1;
}

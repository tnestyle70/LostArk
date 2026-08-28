#include "ValtanPatternAuditionService.h"
#include "Network/PacketWriter.h"

#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace Client;
using namespace LostArk::Shared;

namespace
{
	constexpr const char* BOSS = "boss.valtan.center";
	constexpr const char* A = "VALTAN_FOUR_SLASH";
	constexpr const char* B = "VALTAN_FIST_IN_OUT";
	constexpr const char* C = "VALTAN_TRASH";
	constexpr uint32_t EPOCH = 17u;
	constexpr uint32_t SEQUENCE = 31u;

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
		Out.eResult = Result;
		return Out;
	}

	S2C_VALTAN_AUDITION_LIFECYCLE Event(
		const C2S_VALTAN_AUDITION_REQUEST& Request,
		VALTAN_AUDITION_LIFECYCLE_STATE State)
	{
		S2C_VALTAN_AUDITION_LIFECYCLE Out{};
		Out.iRequestSequence = Request.iRequestSequence;
		Out.iRoomAuditionEpoch = VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID == Request.eOperation ?
			EPOCH : Request.iPredecessorRoomAuditionEpoch;
		Out.iPatternSequence = VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID == Request.eOperation ?
			SEQUENCE : Request.iPredecessorPatternSequence + 1u;
		Out.strPatternId = Request.strPatternId;
		Out.eState = State;
		Out.PinnedDefinitionRevision.Bytes.fill(0x42u);
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

		void Start(const char* Pattern = A, const char* Consumer = "Boss Tool")
		{
			Require(Service.Submit(Consumer, BOSS, Pattern, Status), "initial Play failed");
			Current = Input().SentRequests.back();
			Input().Results.push_back(Verdict(Current));
			Input().Lifecycles.push_back(Event(Current, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE));
			Service.Update();
			Require(Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ACTIVE,
				"initial Play did not activate");
		}

		C2S_VALTAN_AUDITION_REQUEST Queue(const char* Pattern = B)
		{
			Require(Service.Queue_NextPattern("Boss Tool", BOSS, Pattern, Status), "Next command failed");
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
		Require(!F.Service.Queue_NextPattern("Boss Tool", BOSS, B, F.Status),
			"Next accepted without an audition");
		Require(F.Service.Submit("Effect Tool", BOSS, A, F.Status), "Play failed");
		F.Current = F.Input().SentRequests.back();
		Require(!F.Service.Queue_NextPattern("Boss Tool", BOSS, B, F.Status),
			"Next accepted without an authoritative predecessor epoch");
		F.Input().Lifecycles.push_back(Event(F.Current, VALTAN_AUDITION_LIFECYCLE_STATE::PENDING));
		F.Service.Update();
		const auto Next = F.Queue();
		Require(Next.eOperation == VALTAN_AUDITION_OPERATION::QUEUE_NEXT_PATTERN_ID &&
			Next.iPredecessorRoomAuditionEpoch == EPOCH &&
			Next.iPredecessorPatternSequence == SEQUENCE && Next.iExpectedNextRequestSequence == 0u,
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
			!F.Service.Queue_NextPattern("Boss Tool", BOSS, C, F.Status) &&
			SentBeforeConsumedControls == F.Input().SentRequests.size(),
			"promoted Next sent a clear or replacement against the consumed token");
		F.Input().Lifecycles.push_back(Event(Next, VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER));
		F.Service.Update();
		Require(F.Service.Get_NextSnapshot().eState == VALTAN_NEXT_PATTERN_STATE::WAITING_FOR_PLAYER &&
			F.Service.Get_NextSnapshot().bReservationConsumed &&
			!F.Service.Clear_NextPattern(F.Status) &&
			!F.Service.Queue_NextPattern("Effect Tool", BOSS, C, F.Status) &&
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
		Require(!F.Service.Submit("Effect Tool", BOSS, C, F.Status), "another tool reset a waiting Next");
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
		Require(!F.Service.Queue_NextPattern("Boss Tool", BOSS, C, F.Status) &&
			!F.Service.Submit("Effect Tool", BOSS, C, F.Status), "unconfirmed clear allowed a new command");
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
			Require(F.Service.Submit("Effect Tool", BOSS, C, F.Status), "new world could not start a new audition");
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
		Require(!F.Service.Submit("Boss Tool", BOSS, A, F.Status) && F.Input().SentRequests.empty(),
			"Play bypassed ordered Flow ownership");
		F.Input().bFlowInFlight = false;
		F.Start();
		auto Next = F.Reserve();
		F.Input().bFlowInFlight = true;
		Require(!F.Service.Queue_NextPattern("Boss Tool", BOSS, C, F.Status),
			"Next bypassed ordered Flow ownership");
		F.Input().bFlowInFlight = false;
		F.Input().bSendSucceeds = false;
		Require(!F.Service.Queue_NextPattern("Boss Tool", BOSS, C, F.Status) &&
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

	void VerifyInitialPlayTimeoutsStayBounded()
	{
		Fixture F;
		Require(F.Service.Submit("Boss Tool", BOSS, A, F.Status), "Play failed");
		F.Advance(5001u);
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ABORTED,
			"initial verdict no longer times out");
		Require(F.Service.Submit("Boss Tool", BOSS, A, F.Status), "second Play failed");
		F.Input().Results.push_back(Verdict(F.Input().SentRequests.back()));
		F.Service.Update();
		F.Advance(15001u);
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_AUDITION_STATE::ABORTED,
			"initial queued start no longer times out");
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
	return 0u == Failed ? 0 : 1;
}

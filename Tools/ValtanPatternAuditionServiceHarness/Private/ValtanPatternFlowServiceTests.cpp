#include "ValtanPatternFlowService.h"
#include "Network/PacketWriter.h"

#include <algorithm>
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
	constexpr uint32_t EPOCH = 17u;

	GameplayDataRevision MakeDefinitionRevision(const uint8_t Value = 0x42u)
	{
		GameplayDataRevision Revision{};
		Revision.Bytes.fill(Value);
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

	VALTAN_PATTERN_FLOW_DEFINITION MakeFlow()
	{
		VALTAN_PATTERN_FLOW_DEFINITION Flow;
		Flow.strFlowId = "flow.valtan.boss-tool.default";
		Flow.iNextSlotOrdinal = 100u;
		Flow.Slots = {
			{ "flow.valtan.boss-tool.default.slot.000099", "VALTAN_WHIRLWIND" },
			{ "flow.valtan.boss-tool.default.slot.000002", "VALTAN_FOUR_SLASH" },
			{ "flow.valtan.boss-tool.default.slot.000030", "VALTAN_DASH_CHARGE" }
		};
		return Flow;
	}

	template <class T>
	std::vector<uint8_t> Wire(const T& Message)
	{
		CPacketWriter Writer;
		Require(Write_Message(Writer, Message), "fixture violates the production Flow codec");
		return Writer.Get_Buffer();
	}

	S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT Verdict(
		const C2S_DEBUG_VALTAN_PATTERN_FLOW_START& Request,
		uint32_t Epoch = EPOCH,
		VALTAN_PATTERN_FLOW_RESULT Result = VALTAN_PATTERN_FLOW_RESULT::QUEUED)
	{
		S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT Out;
		Out.iCommandSequence = Request.iRequestSequence;
		Out.eCommand = VALTAN_PATTERN_FLOW_COMMAND::START;
		Out.eResult = Result;
		Out.strFlowId = Request.strFlowId;
		Out.strFlowRevision = Request.strFlowRevision;
		if (Result == VALTAN_PATTERN_FLOW_RESULT::QUEUED ||
			Result == VALTAN_PATTERN_FLOW_RESULT::DUPLICATE_IGNORED)
		{
			Out.iRoomFlowEpoch = Epoch;
			Out.PinnedDefinitionRevision.Bytes.fill(0x42u);
		}
		else
		{
			Out.strReason = "harness rejected replacement";
		}
		(void)Wire(Out);
		return Out;
	}

	S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE Event(
		const C2S_DEBUG_VALTAN_PATTERN_FLOW_START& Request,
		VALTAN_PATTERN_FLOW_LIFECYCLE_STATE State,
		uint16_t Ordinal = 1u,
		uint32_t Epoch = EPOCH)
	{
		S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE Out;
		Out.iRequestSequence = Request.iRequestSequence;
		Out.strBossPlacementId = Request.strBossPlacementId;
		Out.strFlowId = Request.strFlowId;
		Out.strFlowRevision = Request.strFlowRevision;
		Out.strStartSlotId = Request.strStartSlotId;
		Out.iSlotCount = static_cast<uint16_t>(Request.Slots.size());
		Out.eState = State;
		if (State == VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::REJECTED)
		{
			Out.strReason = "harness lifecycle rejection";
		}
		else
		{
			Out.iRoomFlowEpoch = Epoch;
			Out.iPatternSequence = 100u + Ordinal;
			Out.iCurrentSlotOrdinal = Ordinal;
			Out.strCurrentSlotId = Request.Slots.at(Ordinal - 1u).strSlotId;
			Out.strCurrentPatternId = Request.Slots.at(Ordinal - 1u).strPatternId;
			Out.PinnedDefinitionRevision.Bytes.fill(0x42u);
			if (State == VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ABORTED)
				Out.strReason = "harness abort";
		}
		(void)Wire(Out);
		return Out;
	}

	struct Fixture
	{
		CValtanPatternFlowService& Service = CValtanPatternFlowService::Get();
		VALTAN_PATTERN_FLOW_DEFINITION Flow = MakeFlow();
		GameplayDataRevision ExpectedDefinitionRevision =
			MakeDefinitionRevision();
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START Current;
		std::string Status;

		Fixture() { Service.Harness_Reset(); }
		auto& Input() { return Service.Harness_Input(); }

		C2S_DEBUG_VALTAN_PATTERN_FLOW_START Send(char Revision = 'a')
		{
			Require(Service.Start(BOSS, Flow, std::string(64u, Revision),
				Flow.Slots.front().strSlotId, ExpectedDefinitionRevision,
				Status), "Flow start failed");
			return Input().SentStarts.back();
		}

		void Activate()
		{
			Current = Send();
			Require(Service.Has_PendingStart() && Service.Has_PlaybackOwnership() &&
				Service.Get_Snapshot().eState == VALTAN_PATTERN_FLOW_STATE::IDLE,
				"unapproved initial request became the admitted run or lost ownership");
			Input().Results.push_back(Verdict(Current));
			Input().Lifecycles.push_back(Event(Current, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE));
			Service.Update();
			Require(Service.Get_Snapshot().eState == VALTAN_PATTERN_FLOW_STATE::ACTIVE &&
				!Service.Has_PendingStart(), "initial Flow did not activate");
		}

		void Advance(uint64_t Milliseconds)
		{
			Input().iNowMilliseconds += Milliseconds;
			Service.Update();
		}

		S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT StopVerdict() const
		{
			const auto& Stop = Service.Harness_Input().SentStops.back();
			auto Out = Verdict(Current);
			Out.eCommand = VALTAN_PATTERN_FLOW_COMMAND::STOP_AFTER_CURRENT;
			Out.iCommandSequence = Stop.iControlSequence;
			Out.iRoomFlowEpoch = Stop.iRoomFlowEpoch;
			(void)Wire(Out);
			return Out;
		}
	};

	void VerifySavedOrderRestartCommit()
	{
		Fixture F;
		F.Activate();
		std::rotate(F.Flow.Slots.begin(), F.Flow.Slots.begin() + 2, F.Flow.Slots.end());
		const auto Replacement = F.Send('b');
		Require(Replacement.Slots[0].strPatternId == "VALTAN_DASH_CHARGE" &&
			Replacement.Slots[1].strPatternId == "VALTAN_WHIRLWIND" &&
			Replacement.Slots[2].strPatternId == "VALTAN_FOUR_SLASH" &&
			Replacement.strStartSlotId == F.Flow.Slots.front().strSlotId &&
			Replacement.ExpectedDefinitionRevision ==
				F.ExpectedDefinitionRevision &&
			F.Service.Get_Snapshot().iRequestSequence == F.Current.iRequestSequence &&
			F.Service.Get_Snapshot().strFlowRevision == std::string(64u, 'a'),
			"restart reordered sparse IDs or replaced the current run before acceptance");
		F.Flow.Slots.clear();
		Require(F.Service.Get_PendingStart().Request.Slots.size() == 3u,
			"pending request aliases the authoring draft");
		F.Input().Lifecycles.push_back(Event(F.Current, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE, 2u));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().strCurrentPatternId == "VALTAN_FOUR_SLASH" &&
			F.Service.Has_PendingStart(), "pending restart stopped consuming the old lifecycle");
		F.Input().Results.push_back(Verdict(Replacement, EPOCH + 1u));
		F.Input().Lifecycles.push_back(Event(Replacement,
			VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE, 1u, EPOCH + 1u));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().strCurrentPatternId == "VALTAN_DASH_CHARGE" &&
			F.Service.Get_Snapshot().iRoomFlowEpoch == EPOCH + 1u &&
			F.Service.Get_Snapshot().strFlowRevision == std::string(64u, 'b') &&
			!F.Service.Has_PendingStart(), "accepted replacement did not commit its first saved slot");
	}

	void VerifyRejectedReplacementKeepsOldRunAndStop()
	{
		Fixture F;
		F.Activate();
		Require(F.Service.Stop_AfterCurrent(F.Status), "initial stop failed");
		const auto StopResult = F.StopVerdict();
		const auto Replacement = F.Send('b');
		F.Input().Lifecycles.push_back(Event(F.Current, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE, 2u));
		F.Service.Update();
		F.Input().Results.push_back(Verdict(Replacement, 0u,
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_INVALID_FLOW));
		F.Service.Update();
		Require(!F.Service.Has_PendingStart() && F.Service.Has_PlaybackOwnership() &&
			F.Service.Get_Snapshot().iRequestSequence == F.Current.iRequestSequence &&
			F.Service.Get_Snapshot().iRoomFlowEpoch == EPOCH &&
			F.Service.Get_Snapshot().iCurrentSlotOrdinal == 2u &&
			F.Service.Get_Snapshot().bStopAfterCurrentRequested &&
			!F.Service.Get_PendingStart().strStatus.empty(),
			"rejected restart lost the latest old run, epoch, stop, or rejection reason");
		F.Input().Results.push_back(StopResult);
		F.Service.Update();
		Require(F.Service.Get_Snapshot().strStatus == "Server accepted Stop After Current." &&
			!F.Service.Stop_AfterCurrent(F.Status), "old stop identity was discarded on rejected restart");
		F.Input().Lifecycles.push_back(Event(F.Current, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::STOPPED_HOLD, 2u));
		F.Service.Update();
		Require(!F.Service.Has_PlaybackOwnership(), "terminal old Flow retained a stale stop token");
	}

	void VerifyLocalFailuresPreserveCurrentRun()
	{
		Fixture F;
		F.Activate();
		F.Input().bSendSucceeds = false;
		Require(!F.Service.Start(BOSS, F.Flow, std::string(64u, 'b'),
			F.Flow.Slots.front().strSlotId, F.ExpectedDefinitionRevision,
			F.Status), "failed transport accepted restart");
		F.Input().bSendSucceeds = true;
		auto Invalid = F.Flow;
		Invalid.Slots[1].strSlotId = Invalid.Slots.front().strSlotId;
		Require(!F.Service.Start(BOSS, Invalid, std::string(64u, 'b'),
			Invalid.Slots.front().strSlotId, F.ExpectedDefinitionRevision,
			F.Status), "duplicate slot accepted");
		Invalid = F.Flow;
		Invalid.Slots[1].strPatternId = "../invalid";
		Require(!F.Service.Start(BOSS, Invalid, std::string(64u, 'b'),
			Invalid.Slots.front().strSlotId, F.ExpectedDefinitionRevision,
			F.Status), "invalid stable ID accepted");
		Require(!F.Service.Start(BOSS, F.Flow, "bad revision",
			F.Flow.Slots.front().strSlotId, F.ExpectedDefinitionRevision,
			F.Status),
			"invalid revision accepted");
		Require(!F.Service.Start(BOSS, F.Flow, std::string(64u, 'b'),
			"missing.slot", F.ExpectedDefinitionRevision, F.Status),
			"missing start slot accepted");
		Require(!F.Service.Start(BOSS, F.Flow, std::string(64u, 'b'),
			F.Flow.Slots.front().strSlotId, GameplayDataRevision{}, F.Status),
			"missing expected definition revision accepted");
		F.Input().bPendingNextCommand = true;
		Require(!F.Service.Start(BOSS, F.Flow, std::string(64u, 'b'),
			F.Flow.Slots.front().strSlotId, F.ExpectedDefinitionRevision,
			F.Status), "restart raced an unresolved Next control");
		Require(F.Input().SentStarts.size() == 1u && !F.Service.Has_PendingStart() &&
			F.Service.Get_Snapshot().eState == VALTAN_PATTERN_FLOW_STATE::ACTIVE &&
			F.Service.Get_Snapshot().iRoomFlowEpoch == EPOCH,
			"local preflight/send failure mutated the admitted Flow");
		F.Input().bPendingNextCommand = false;
		Require(F.Send('b').iRequestSequence == F.Current.iRequestSequence + 1u,
			"unsent requests consumed or reused the next identity");
	}

	void VerifyLifecycleFirstAndExactIdentity()
	{
		Fixture F;
		F.Activate();
		const auto Replacement = F.Send('b');
		auto WrongSlot = Event(Replacement, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE, 1u, EPOCH + 1u);
		WrongSlot.strCurrentPatternId = Replacement.Slots[1].strPatternId;
		F.Input().Lifecycles.push_back(WrongSlot);
		auto WrongDefinition = Event(
			Replacement, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE,
			1u, EPOCH + 1u);
		WrongDefinition.PinnedDefinitionRevision.Bytes.front() ^= 0xffu;
		F.Input().Lifecycles.push_back(WrongDefinition);
		F.Service.Update();
		Require(F.Service.Has_PendingStart() && F.Service.Get_Snapshot().iRoomFlowEpoch == EPOCH,
			"a valid-looking but wrong slot lifecycle confirmed restart");
		F.Input().Lifecycles.push_back(Event(Replacement,
			VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE, 1u, EPOCH + 1u));
		F.Service.Update();
		Require(!F.Service.Has_PendingStart() && F.Service.Get_Snapshot().iRoomFlowEpoch == EPOCH + 1u,
			"matching lifecycle could not confirm before its verdict");
		F.Input().Results.push_back(Verdict(Replacement, EPOCH + 1u));
		F.Input().Lifecycles.push_back(Event(F.Current, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ABORTED));
		auto WrongEpoch = Event(Replacement, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE, 2u, EPOCH);
		F.Input().Lifecycles.push_back(WrongEpoch);
		auto WrongRevision = WrongEpoch;
		WrongRevision.iRoomFlowEpoch = EPOCH + 1u;
		WrongRevision.PinnedDefinitionRevision.Bytes.fill(0x43u);
		F.Input().Lifecycles.push_back(WrongRevision);
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_FLOW_STATE::ACTIVE &&
			F.Service.Get_Snapshot().iCurrentSlotOrdinal == 1u &&
			F.Service.Get_Snapshot().iRoomFlowEpoch == EPOCH + 1u,
			"late old lifecycle/verdict or wrong epoch/revision damaged the new run");
	}

	void VerifyUnconfirmedRestartRetry()
	{
		Fixture F;
		F.Activate();
		const auto Replacement = F.Send('b');
		const auto OriginalWire = Wire(Replacement);
		F.Advance(5001u);
		Require(F.Service.Get_PendingStart().eState == VALTAN_PATTERN_FLOW_START_STATE::UNCONFIRMED &&
			F.Service.Get_Snapshot().iRoomFlowEpoch == EPOCH && F.Service.Has_PlaybackOwnership(),
			"restart timeout discarded the old run or unresolved ownership");
		Require(!F.Service.Start(BOSS, F.Flow, std::string(64u, 'c'),
			F.Flow.Slots.front().strSlotId, F.ExpectedDefinitionRevision,
			F.Status) &&
			!F.Service.Stop_AfterCurrent(F.Status), "pending restart allowed ambiguous new controls");
		F.Input().bSendSucceeds = false;
		Require(!F.Service.Retry_Start(F.Status) && F.Service.Has_PendingStart(),
			"retry transport failure lost the unresolved request");
		F.Input().bSendSucceeds = true;
		Require(F.Service.Retry_Start(F.Status) &&
			Wire(F.Input().SentStarts.back()) == OriginalWire && F.Input().SentStarts.size() == 3u,
			"retry allocated a different request or changed its ordered payload");
		F.Input().Results.push_back(Verdict(Replacement, EPOCH + 1u,
			VALTAN_PATTERN_FLOW_RESULT::DUPLICATE_IGNORED));
		F.Service.Update();
		Require(!F.Service.Has_PendingStart() && F.Service.Get_Snapshot().iRoomFlowEpoch == EPOCH + 1u,
			"replayed acceptance did not settle restart");
	}

	void VerifyInitialUnknownAndLongServerWait()
	{
		Fixture F;
		F.Current = F.Send();
		F.Advance(60000u);
		Require(F.Service.Has_PlaybackOwnership() && F.Service.Has_PendingStart() &&
			F.Service.Get_Snapshot().eState == VALTAN_PATTERN_FLOW_STATE::IDLE,
			"unconfirmed initial request was falsely aborted or confirmed");
		F.Input().Lifecycles.push_back(Event(F.Current, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PENDING));
		F.Service.Update();
		F.Advance(120000u);
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_FLOW_STATE::REQUEST_PENDING &&
			!F.Service.Has_PendingStart() && F.Service.Has_PlaybackOwnership(),
			"Server-admitted PENDING inherited a local 15 second abort");
		F.Input().Lifecycles.push_back(Event(F.Current, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PAUSED_FOR_REVIVE));
		F.Service.Update();
		F.Advance(120000u);
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_FLOW_STATE::PAUSED_FOR_REVIVE,
			"manual revive wait expired");
		F.Input().Lifecycles.push_back(Event(F.Current, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PENDING, 2u));
		F.Service.Update();
		F.Advance(120000u);
		Require(F.Service.Get_Snapshot().iCurrentSlotOrdinal == 2u && F.Service.Has_PlaybackOwnership(),
			"targetless inter-slot wait lost its Server program");
		F.Input().Lifecycles.push_back(Event(F.Current, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PENDING, 3u));
		F.Service.Update();
		F.Input().Lifecycles.push_back(Event(F.Current, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::COMPLETED_HOLD, 3u));
		F.Service.Update();
		F.Input().Lifecycles.push_back(Event(F.Current, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_FLOW_STATE::COMPLETED_HOLD &&
			!F.Service.Has_PlaybackOwnership(), "late lifecycle resurrected a completed Flow");
	}

	void VerifyWorldChangeAndDisconnect()
	{
		for (bool Disconnect : { false, true })
		{
			Fixture F;
			F.Activate();
			const auto Replacement = F.Send('b');
			F.Input().Results.push_back(Verdict(Replacement, EPOCH + 1u));
			F.Input().Lifecycles.push_back(Event(Replacement,
				VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE, 1u, EPOCH + 1u));
			if (Disconnect)
				F.Input().bConnected = false;
			else
				++F.Input().iWorldInboundGeneration;
			F.Service.Update();
			Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_FLOW_STATE::ABORTED &&
				!F.Service.Has_PendingStart() && !F.Service.Has_PlaybackOwnership(),
				"old-world queued approval survived cleanup");
			F.Input().bConnected = true;
			F.Input().Results.push_back(Verdict(Replacement, EPOCH + 1u));
			F.Input().Lifecycles.push_back(Event(F.Current, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE));
			F.Service.Update();
			Require(F.Service.Get_Snapshot().eState == VALTAN_PATTERN_FLOW_STATE::ABORTED,
				"late events resurrected a disconnected generation");
		}
	}

	void VerifyOldTerminalDuringReplacement()
	{
		for (auto Terminal : { VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::COMPLETED_HOLD,
			VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::STOPPED_HOLD })
		{
			Fixture F;
			F.Activate();
			const auto Replacement = F.Send('b');
			F.Input().Lifecycles.push_back(Event(F.Current, Terminal));
			F.Service.Update();
			Require(F.Service.Get_Snapshot().Is_TerminalHold() && F.Service.Has_PlaybackOwnership(),
				"old terminal lost the pending replacement or failed to update its own snapshot");
			F.Input().Lifecycles.push_back(Event(Replacement, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::REJECTED));
			F.Service.Update();
			Require(!F.Service.Has_PlaybackOwnership() && F.Service.Get_Snapshot().Is_TerminalHold() &&
				F.Service.Get_Snapshot().iRequestSequence == F.Current.iRequestSequence,
				"lifecycle rejection restored an obsolete active copy of the old run");
		}
	}

	void VerifyStopTimeoutAndRetiredControl()
	{
		Fixture F;
		F.Activate();
		Require(F.Service.Stop_AfterCurrent(F.Status), "stop failed");
		const auto OldStop = F.StopVerdict();
		F.Advance(5001u);
		Require(!F.Service.Stop_AfterCurrent(F.Status) && F.Service.Has_PlaybackOwnership(),
			"stop timeout allowed another control or discarded ownership");
		const auto Replacement = F.Send('b');
		F.Input().Results.push_back(Verdict(Replacement, EPOCH + 1u));
		F.Input().Lifecycles.push_back(Event(Replacement,
			VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE, 1u, EPOCH + 1u));
		F.Service.Update();
		Require(!F.Service.Get_Snapshot().bStopAfterCurrentRequested && F.Service.Stop_AfterCurrent(F.Status),
			"accepted restart inherited the previous run's stop flag");
		F.Input().Results.push_back(OldStop);
		F.Service.Update();
		Require(F.Service.Get_Snapshot().iRoomFlowEpoch == EPOCH + 1u &&
			F.Service.Get_Snapshot().bStopAfterCurrentRequested &&
			F.Service.Get_Snapshot().strStatus != "Server accepted Stop After Current.",
			"a retired stop token acknowledged the new run's control");
	}

	void VerifyDataDrivenSameSlotOccurrenceCursor()
	{
		for (const char* PatternId : { "VALTAN_WHIRLWIND", "VALTAN_GHOST_FINALE" })
		{
			for (const uint32_t Initial : { 101u, (std::numeric_limits<uint32_t>::max)() })
			{
				for (const auto TerminalState : {
					VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::STOPPED_HOLD,
					VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ABORTED })
				{
					Fixture F;
					F.Flow.Slots.front().strPatternId = PatternId;
					F.Current = F.Send();
					F.Input().Results.push_back(Verdict(F.Current));
					auto First = Event(F.Current, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE);
					First.iPatternSequence = Initial;
					F.Input().Lifecycles.push_back(First);
					F.Service.Update();

					auto ActiveOccurrence = First;
					ActiveOccurrence.iPatternSequence =
						Initial == (std::numeric_limits<uint32_t>::max)() ? 1u : Initial + 1u;
					F.Input().Lifecycles.push_back(ActiveOccurrence);
					F.Service.Update();
					Require(F.Service.Get_Snapshot().iPatternSequence == ActiveOccurrence.iPatternSequence &&
						F.Service.Get_Snapshot().eState == VALTAN_PATTERN_FLOW_STATE::ACTIVE,
						"same-slot data-driven ACTIVE occurrence did not advance wrap-safely");

					auto PendingOccurrence = ActiveOccurrence;
					PendingOccurrence.iPatternSequence = ActiveOccurrence.iPatternSequence + 1u;
					PendingOccurrence.eState = VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PENDING;
					F.Input().Lifecycles.push_back(PendingOccurrence);
					F.Service.Update();
					Require(F.Service.Get_Snapshot().iPatternSequence == PendingOccurrence.iPatternSequence &&
						F.Service.Get_Snapshot().eState == VALTAN_PATTERN_FLOW_STATE::REQUEST_PENDING,
						"same-slot PENDING occurrence did not become the terminal cursor");

					auto Stale = ActiveOccurrence;
					auto Ambiguous = PendingOccurrence;
					Ambiguous.iPatternSequence += 0x80000000u;
					auto WrongRequest = PendingOccurrence;
					++WrongRequest.iRequestSequence;
					++WrongRequest.iPatternSequence;
					auto WrongOwner = PendingOccurrence;
					WrongOwner.strBossPlacementId = "boss.valtan.other";
					++WrongOwner.iPatternSequence;
					auto WrongEpoch = PendingOccurrence;
					++WrongEpoch.iRoomFlowEpoch;
					++WrongEpoch.iPatternSequence;
					auto WrongFlowRevision = PendingOccurrence;
					WrongFlowRevision.strFlowRevision = std::string(64u, 'b');
					++WrongFlowRevision.iPatternSequence;
					auto WrongPinnedRevision = PendingOccurrence;
					WrongPinnedRevision.PinnedDefinitionRevision.Bytes.fill(0x43u);
					++WrongPinnedRevision.iPatternSequence;
					auto WrongSlot = PendingOccurrence;
					WrongSlot.strCurrentSlotId = "flow.valtan.boss-tool.default.slot.999999";
					++WrongSlot.iPatternSequence;
					for (const auto& Invalid : { Stale, Ambiguous, WrongRequest, WrongOwner,
						WrongEpoch, WrongFlowRevision, WrongPinnedRevision, WrongSlot })
					{
						(void)Wire(Invalid);
						F.Input().Lifecycles.push_back(Invalid);
					}
					F.Service.Update();
					Require(F.Service.Get_Snapshot().iPatternSequence == PendingOccurrence.iPatternSequence &&
						F.Service.Get_Snapshot().eState == VALTAN_PATTERN_FLOW_STATE::REQUEST_PENDING,
						"stale, ambiguous, or foreign lifecycle changed the same-slot cursor");

					auto Terminal = PendingOccurrence;
					Terminal.eState = TerminalState;
					if (VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ABORTED == TerminalState)
						Terminal.strReason = "harness abort after pending occurrence";
					F.Input().Lifecycles.push_back(Terminal);
					F.Service.Update();
					const VALTAN_PATTERN_FLOW_STATE ExpectedState =
						VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ABORTED == TerminalState ?
						VALTAN_PATTERN_FLOW_STATE::ABORTED : VALTAN_PATTERN_FLOW_STATE::STOPPED_HOLD;
					Require(F.Service.Get_Snapshot().eState == ExpectedState &&
						F.Service.Get_Snapshot().iPatternSequence == PendingOccurrence.iPatternSequence &&
						!F.Service.Has_PlaybackOwnership(),
						"terminal lifecycle did not reuse the last emitted PENDING cursor");
				}
			}
		}
	}

	void VerifyRequestIdentityExhaustion()
	{
		Fixture F;
		F.Activate();
		F.Service.Harness_SetNextRequestSequence((std::numeric_limits<uint32_t>::max)());
		const auto Replacement = F.Send('b');
		F.Input().Results.push_back(Verdict(Replacement, 0u, VALTAN_PATTERN_FLOW_RESULT::REJECTED_CONFLICT));
		F.Service.Update();
		Require(!F.Service.Start(BOSS, F.Flow, std::string(64u, 'c'),
			F.Flow.Slots.front().strSlotId, F.ExpectedDefinitionRevision,
			F.Status) &&
			F.Service.Get_Snapshot().iRequestSequence == F.Current.iRequestSequence,
			"Flow start sequence wrapped into an old wire identity");
	}

	void VerifyMaximumSlotBoundary()
	{
		Fixture F;
		F.Flow.Slots.clear();
		for (std::size_t Index = 0u;
			Index < MAX_VALTAN_PATTERN_FLOW_SLOTS + 1u; ++Index)
		{
			F.Flow.Slots.push_back({
				"flow.slot." + std::to_string(Index + 1u),
				"VALTAN_WHIRLWIND" });
		}
		const std::string Revision(64u, 'a');
		Require(!F.Service.Start(
				BOSS, F.Flow, Revision, F.Flow.Slots.front().strSlotId,
				F.ExpectedDefinitionRevision, F.Status) &&
			F.Input().SentStarts.empty() &&
			!F.Service.Has_PendingStart() &&
			!F.Service.Has_PlaybackOwnership(),
			"256-slot Flow escaped Client preflight or mutated service state");

		F.Flow.Slots.pop_back();
		Require(F.Service.Start(
				BOSS, F.Flow, Revision, F.Flow.Slots.front().strSlotId,
				F.ExpectedDefinitionRevision, F.Status) &&
			F.Input().SentStarts.size() == 1u &&
			F.Input().SentStarts.back().Slots.size() ==
				MAX_VALTAN_PATTERN_FLOW_SLOTS &&
			F.Input().SentStarts.back().Slots.front().strSlotId ==
				F.Flow.Slots.front().strSlotId &&
			F.Input().SentStarts.back().Slots.back().strSlotId ==
				F.Flow.Slots.back().strSlotId &&
			F.Service.Has_PendingStart() &&
			F.Service.Has_PlaybackOwnership(),
			"255-slot Flow did not preserve the exact ordered Client request");
	}

	void VerifyPatternSoundReceiptPinsFlowAndEverySlotOccurrence()
	{
		Fixture Invalid;
		Require(!Invalid.Service.Start(
				BOSS, Invalid.Flow, std::string(64u, 'a'),
				Invalid.Flow.Slots.front().strSlotId,
				Invalid.ExpectedDefinitionRevision,
				VALTAN_PATTERN_SOUND_SOURCE_RECEIPT{}, Invalid.Status) &&
			Invalid.Input().SentStarts.empty(),
			"Flow accepted a missing S receipt or sent before fail-closed validation");
		Fixture F;
		const auto SoundA = SoundReceipt('a');
		const auto SoundB = SoundReceipt('b');
		Require(F.Service.Start(
				BOSS, F.Flow, std::string(64u, 'a'),
				F.Flow.Slots.front().strSlotId,
				F.ExpectedDefinitionRevision, SoundA, F.Status),
			"Flow rejected a valid exact Pattern Sound receipt");
		F.Current = F.Input().SentStarts.back();
		Require(F.Service.Get_PendingStart().
				PinnedPatternSoundSourceReceipt == SoundA &&
			F.Service.Has_PatternSoundMutationBarrier(),
			"pending Flow Start did not pin S or expose its mutation barrier");
		F.Input().Results.push_back(Verdict(F.Current));
		F.Input().Lifecycles.push_back(Event(
			F.Current, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE, 1u));
		F.Service.Update();
		Require(F.Service.Get_Snapshot().PinnedPatternSoundSourceReceipt ==
				SoundA &&
			F.Service.Verify_PatternSoundSourceReceipt(SoundA, F.Status) &&
			!F.Service.Verify_PatternSoundSourceReceipt(SoundB, F.Status),
			"active Flow did not retain or verify its exact S receipt");
		auto SecondSlot = Event(
			F.Current, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE, 2u);
		F.Input().Lifecycles.push_back(SecondSlot);
		F.Service.Update();
		Require(F.Service.Get_Snapshot().iCurrentSlotOrdinal == 2u &&
			F.Service.Get_Snapshot().PinnedPatternSoundSourceReceipt == SoundA,
			"Flow auto-next slot transition lost its pinned S receipt");
		Require(!F.Service.Start(
				BOSS, F.Flow, std::string(64u, 'b'),
				F.Flow.Slots.front().strSlotId,
				F.ExpectedDefinitionRevision, SoundB, F.Status),
			"active Flow accepted a replacement pinned to a foreign S receipt");

		Fixture Retry;
		Require(Retry.Service.Start(
				BOSS, Retry.Flow, std::string(64u, 'a'),
				Retry.Flow.Slots.front().strSlotId,
				Retry.ExpectedDefinitionRevision, SoundA, Retry.Status),
			"Flow retry fixture could not submit Start");
		Retry.Advance(5100u);
		Require(!Retry.Service.Retry_Start(SoundB, Retry.Status) &&
			Retry.Service.Retry_Start(SoundA, Retry.Status),
			"unconfirmed Flow Start did not bind exact retry to S");
	}
}

int Run_ValtanPatternFlowServiceTests()
{
	const std::vector<std::pair<const char*, void (*)()>> Tests{
		{ "saved sparse-slot order and transactional restart", VerifySavedOrderRestartCommit },
		{ "rejected replacement preserves old lifecycle and stop", VerifyRejectedReplacementKeepsOldRunAndStop },
		{ "local validation/send/Next-control failures preserve old run", VerifyLocalFailuresPreserveCurrentRun },
		{ "lifecycle-first acceptance and exact old/new identity", VerifyLifecycleFirstAndExactIdentity },
		{ "unconfirmed restart retries the exact saved request", VerifyUnconfirmedRestartRetry },
		{ "initial unknown and long Server-admitted waits retain ownership", VerifyInitialUnknownAndLongServerWait },
		{ "world/disconnect invalidates before queue drain", VerifyWorldChangeAndDisconnect },
		{ "old terminal keeps its latest state during replacement", VerifyOldTerminalDuringReplacement },
		{ "stop timeout retains identity and retired controls stay retired", VerifyStopTimeoutAndRetiredControl },
		{ "all data-driven same-slot occurrences keep a correlated wrap-safe cursor", VerifyDataDrivenSameSlotOccurrenceCursor },
		{ "request identity exhaustion never wraps", VerifyRequestIdentityExhaustion },
		{ "255-slot Client request succeeds and 256 is atomic", VerifyMaximumSlotBoundary },
		{ "Pattern Sound S pins Flow and every slot occurrence", VerifyPatternSoundReceiptPinsFlowAndEverySlotOccurrence }
	};
	int Failed = 0;
	for (const auto& [Name, Test] : Tests)
	{
		try { Test(); }
		catch (const std::exception& Error)
		{
			++Failed;
			std::cerr << "FAIL Flow " << Name << ": " << Error.what() << '\n';
		}
	}
	std::cout << "ValtanPatternFlowServiceTests: " << Tests.size() - Failed << "/"
		<< Tests.size() << " passed\n";
	return Failed;
}

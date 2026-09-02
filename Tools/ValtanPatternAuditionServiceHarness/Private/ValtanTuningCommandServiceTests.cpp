#include "ValtanTuningCommandService.h"
#include "Network/PacketWriter.h"

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
	void Require(const bool condition, const char* message)
	{
		if (!condition)
			throw std::runtime_error(message);
	}

	std::string Revision(const char digit) { return std::string(64u, digit); }

	GameplayDataRevision Identity(const char digit)
	{
		GameplayDataRevision result;
		Require(Try_Parse_GameplayDataRevision(Revision(digit), result),
			"fixture revision is invalid");
		return result;
	}

	struct Fixture
	{
		CValtanTuningCommandService& Service = CValtanTuningCommandService::Get();
		std::string Status;

		Fixture()
		{
			Service.Harness_Reset();
			Input().ServerActiveRevision = Identity('d');
		}

		VALTAN_TUNING_COMMAND_HARNESS_INPUT& Input() { return Service.Harness_Input(); }
		const VALTAN_TUNING_COMMAND_SNAPSHOT& Snapshot() const { return Service.Get_Snapshot(); }

		void Apply(const char candidate = 'b')
		{
			Require(Service.ApplyCandidate(Revision(candidate), "HOT_RELOAD", Status),
				"candidate Apply was not accepted");
		}

		void Terminal(const DATA_REVISION_RESULT result = DATA_REVISION_RESULT::COMMITTED)
		{
			Require(!Input().SentRequests.empty(), "terminal fixture has no request");
			const auto& request = Input().SentRequests.back();
			Input().bOutstandingPrepareRequest = false;
			Input().bHasLatestResult = true;
			Input().iLatestTransactionSequence = request.iTransactionSequence;
			Input().LatestCandidateRevision = request.CandidateRevision;
			Input().eLatestResult = result;
			Input().strLatestTransactionReason = result == DATA_REVISION_RESULT::ABORTED ?
				"candidate artifact was not available on every participant" : "";
			if (result == DATA_REVISION_RESULT::COMMITTED)
				Input().ServerActiveRevision = request.CandidateRevision;
			Service.Update();
		}
	};

	void VerifyCandidateAndServerCommitAreSeparate()
	{
		Fixture f;
		f.Apply();
		Require(f.Service.Has_PendingCommand() &&
			f.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::APPLY_PENDING &&
			f.Snapshot().strCandidateRevision == Revision('b') &&
			f.Input().SentRequests.size() == 1u &&
			f.Input().ServerActiveRevision == Identity('d') &&
			!f.Snapshot().bCandidateIsServerActive,
			"candidate submission pretended the Server had committed it");

		const auto& request = f.Input().SentRequests.back();
		CPacketWriter writer;
		Require(Write_Message(writer, request) && request.BaseRevision == Identity('d') &&
			request.CandidateRevision == Identity('b') &&
			request.iRequiredPresentationLaneMask == GAMEPLAY_PRESENTATION_KNOWN_LANE_MASK,
			"Apply bypassed the typed revision boundary");
		f.Terminal();
		Require(f.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::COMMITTED &&
			f.Snapshot().bCandidateIsServerActive && !f.Service.Has_PendingCommand(),
			"matching Server commit did not settle the candidate");
	}

	void VerifyInputAndReleasePolicyFailClosed()
	{
		Fixture f;
		for (const auto& invalid :
			std::vector<std::string>{ "", "bad", Revision('A'), "a\" --candidate" })
		{
			Require(!f.Service.ApplyCandidate(invalid, "HOT_RELOAD", f.Status) &&
				f.Input().SentRequests.empty(), "invalid revision entered the transaction");
		}
		for (const char* applyClass : { "ENCOUNTER_RESET", "SERVER_RESTART", "UNKNOWN" })
		{
			Require(!f.Service.ApplyCandidate(Revision('b'), applyClass, f.Status) &&
				f.Input().SentRequests.empty(), "non-Hot-Reload candidate entered the transaction");
		}
		f.Input().bActivationEnabled = false;
		Require(!f.Service.ApplyCandidate(Revision('b'), "HOT_RELOAD", f.Status) &&
			f.Input().SentRequests.empty() &&
			f.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::PUBLISHED_APPLY_NEEDED,
			"Release activation policy submitted a command");
	}

	void VerifyPendingAndSequenceOwnership()
	{
		Fixture f;
		f.Apply();
		const auto first = f.Input().SentRequests.back();
		Require(!f.Service.ApplyCandidate(Revision('c'), "HOT_RELOAD", f.Status) &&
			f.Snapshot().strCandidateRevision == Revision('b') &&
			f.Snapshot().iRequestSequence == first.iTransactionSequence,
			"a second command replaced an unresolved candidate");
		f.Terminal();
		f.Apply('c');
		Require(f.Input().SentRequests.back().iTransactionSequence ==
				first.iTransactionSequence + 1u &&
			f.Input().SentRequests.back().BaseRevision == Identity('b'),
			"commands did not share current-base/request-sequence ownership");

		for (int kind = 0; kind < 3; ++kind)
		{
			Fixture other;
			other.Input().bOutstandingPrepareRequest = kind == 0;
			other.Input().bStagedPresentationAlias = kind == 1;
			other.Input().bRejectedPrepareAwaitingAbort = kind == 2;
			Require(other.Service.Has_PendingCommand() &&
				!other.Service.ApplyCandidate(Revision('b'), "HOT_RELOAD", other.Status) &&
				other.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::IDLE,
				"another participant's transaction was ignored or overwritten");
		}
	}

	void VerifyFailedSendRetainsCandidateAndSequence()
	{
		Fixture f;
		f.Input().bSendSucceeds = false;
		Require(!f.Service.ApplyCandidate(Revision('b'), "HOT_RELOAD", f.Status) &&
			f.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::PUBLISHED_APPLY_NEEDED &&
			f.Snapshot().strCandidateRevision == Revision('b') &&
			!f.Service.Has_PendingCommand() && f.Input().SentRequests.size() == 1u,
			"failed send erased the candidate or pretended it was pending");
		const auto failed = f.Input().SentRequests.back();
		f.Input().bSendSucceeds = true;
		f.Apply();
		Require(f.Input().SentRequests.size() == 2u &&
			f.Input().SentRequests.back().iTransactionSequence == failed.iTransactionSequence,
			"an unsent command consumed the request sequence");
	}

	void VerifyResultIdentityIsExact()
	{
		Fixture f;
		f.Input().bHasLatestResult = true;
		f.Input().iLatestTransactionSequence = 1u;
		f.Input().LatestCandidateRevision = Identity('b');
		f.Input().eLatestResult = DATA_REVISION_RESULT::COMMITTED;
		f.Apply();
		f.Service.Update();
		Require(f.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::APPLY_PENDING,
			"an old exact-looking result acknowledged a new request");
		f.Input().bOutstandingPrepareRequest = false;
		f.Service.Update();
		Require(f.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::APPLY_PENDING,
			"COMMITTED with a different active candidate was accepted");
		f.Input().ServerActiveRevision = Identity('b');
		f.Input().LatestCandidateRevision = Identity('c');
		f.Service.Update();
		Require(f.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::APPLY_PENDING,
			"a different candidate result acknowledged this Apply");
		f.Input().LatestCandidateRevision = Identity('b');
		f.Input().iLatestTransactionSequence = 2u;
		f.Service.Update();
		Require(f.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::APPLY_PENDING,
			"a different request result acknowledged this Apply");
		f.Input().iLatestTransactionSequence = 1u;
		f.Service.Update();
		Require(f.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::COMMITTED,
			"the exact candidate/request/active result was not accepted");
	}

	void VerifyTimeoutAndRejectionRemainTruthful()
	{
		Fixture f;
		f.Apply();
		f.Input().iNowMilliseconds += 10001u;
		f.Service.Update();
		Require(f.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::UNCONFIRMED &&
			f.Service.Has_PendingCommand() &&
			!f.Service.ApplyCandidate(Revision('c'), "HOT_RELOAD", f.Status),
			"timeout fabricated cancellation or allowed a competing Apply");
		f.Terminal(DATA_REVISION_RESULT::ABORTED);
		Require(f.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::FAILED &&
			!f.Service.Has_PendingCommand() &&
			f.Snapshot().strCandidateRevision == Revision('b') &&
			f.Snapshot().strStatus.find("candidate artifact") != std::string::npos &&
			f.Input().ServerActiveRevision == Identity('d'),
			"ABORT lost its reason, identity, or previous active revision");
	}

	void VerifyWorldGenerationAndFreshness()
	{
		for (const bool changeWorld : { false, true })
		{
			Fixture f;
			f.Apply();
			const auto old = f.Input().SentRequests.back();
			if (changeWorld) ++f.Input().iWorldInboundGeneration;
			else ++f.Input().iConnectionGeneration;
			f.Input().bOutstandingPrepareRequest = false;
			f.Input().bHasLatestResult = true;
			f.Input().iLatestTransactionSequence = old.iTransactionSequence;
			f.Input().LatestCandidateRevision = old.CandidateRevision;
			f.Input().eLatestResult = DATA_REVISION_RESULT::COMMITTED;
			f.Service.Update();
			Require(f.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::PUBLISHED_APPLY_NEEDED &&
				!f.Service.Has_PendingCommand() &&
				f.Snapshot().strStatus.find("does not cancel or confirm") != std::string::npos,
				"old-generation result claimed success in a new world");
			f.Apply();
			Require(f.Snapshot().iRequestSequence == old.iTransactionSequence + 1u,
				"new world retry reused the previous transaction identity");
		}

		Fixture current;
		current.Input().ServerActiveRevision = Identity('b');
		current.Apply();
		Require(current.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::ALREADY_ACTIVE &&
			current.Snapshot().bCandidateIsServerActive && current.Input().SentRequests.empty(),
			"already-active candidate forged a new transaction");
		current.Input().ServerActiveRevision = Identity('d');
		current.Service.Update();
		Require(current.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::PUBLISHED_APPLY_NEEDED &&
			!current.Snapshot().bCandidateIsServerActive,
			"another active revision left a stale applied display");
	}

	void VerifySequenceExhaustionDoesNotWrap()
	{
		Fixture f;
		f.Service.Harness_SetNextRequestSequence((std::numeric_limits<uint32_t>::max)());
		f.Apply();
		f.Terminal();
		Require(!f.Service.ApplyCandidate(Revision('c'), "HOT_RELOAD", f.Status) &&
			f.Input().SentRequests.size() == 1u &&
			f.Input().SentRequests.front().iTransactionSequence ==
				(std::numeric_limits<uint32_t>::max)(),
			"request identity wrapped into a previous session-local command");
	}

	void VerifyGameplaySourceActivationGate()
	{
		Fixture f;
		GameplayDataRevision exactRevision{};
		Require(!f.Service.Has_GameplaySourceActivationExpectation() &&
			f.Service.Is_LatestGameplaySourceServerActive(f.Status),
			"an untouched process fabricated an activation mismatch");
		Require(!f.Service.Try_GetLatestGameplaySourceServerActiveRevision(
				exactRevision, f.Status) && !exactRevision.Is_Valid(),
			"an untouched process fabricated an exact saved Product revision");
		f.Service.Record_GameplaySourceActivationExpectation({}, {}, "candidate projection failed");
		Require(f.Service.Has_GameplaySourceActivationExpectation() &&
			!f.Service.Is_LatestGameplaySourceServerActive(f.Status) &&
			f.Status.find("no admitted Product candidate") != std::string::npos,
			"a failed canonical save candidate authorized old Server playback");
		f.Service.Record_GameplaySourceActivationExpectation(
			Revision('b'), "ENCOUNTER_RESET", "controlled reset required");
		Require(!f.Service.Is_LatestGameplaySourceServerActive(f.Status) &&
			f.Status.find("ENCOUNTER_RESET") != std::string::npos,
			"a restart-class candidate authorized the previous revision");
		f.Input().ServerActiveRevision = Identity('b');
		Require(f.Service.Is_LatestGameplaySourceServerActive(f.Status),
			"the exact Server-active canonical candidate did not release playback");
		Require(f.Service.Try_GetLatestGameplaySourceServerActiveRevision(
				exactRevision, f.Status) && exactRevision == Identity('b'),
			"the saved gameplay gate did not return its exact Server-active Product identity");
		f.Input().bConnected = false;
		Require(!f.Service.Is_LatestGameplaySourceServerActive(f.Status),
			"a disconnected observation authorized gameplay playback");
		Require(!f.Service.Try_GetLatestGameplaySourceServerActiveRevision(
				exactRevision, f.Status) && !exactRevision.Is_Valid(),
			"a disconnected observation leaked a stale saved Product identity");
		f.Service.Record_GameplaySourceActivationExpectation(
			"not-a-revision", "HOT_RELOAD", "invalid candidate output");
		Require(!f.Service.Is_LatestGameplaySourceServerActive(f.Status) &&
			f.Status.find("invalid candidate revision") != std::string::npos,
			"an invalid canonical candidate failed open");
	}

	void VerifyTwoSaveRestartCandidateLiveness()
	{
		Fixture f;
		f.Service.Record_GameplaySourceActivationExpectation(
			Revision('b'), "HOT_RELOAD", "first Flow save");
		f.Apply('b');
		const auto first = f.Input().SentRequests.back();

		f.Service.Record_GameplaySourceActivationExpectation(
			Revision('c'), "HOT_RELOAD", "second Flow save");
		Require(f.Service.Queue_GameplaySourceCandidateAfterPending(
				Revision('c'), "HOT_RELOAD", f.Status) &&
			f.Input().SentRequests.size() == 1u,
			"the second Save either replaced the unresolved first transaction or was not queued");

		f.Terminal();
		Require(f.Input().SentRequests.size() == 2u &&
			f.Input().SentRequests.back().iTransactionSequence ==
				first.iTransactionSequence + 1u &&
			f.Input().SentRequests.back().BaseRevision == Identity('b') &&
			f.Input().SentRequests.back().CandidateRevision == Identity('c') &&
			f.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::APPLY_PENDING,
			"the second saved candidate did not begin after the first exact terminal result");

		f.Terminal();
		GameplayDataRevision exactRevision{};
		Require(f.Service.Try_GetLatestGameplaySourceServerActiveRevision(
				exactRevision, f.Status) && exactRevision == Identity('c') &&
			!f.Service.Has_PendingCommand(),
			"Save A -> restart A -> Save B -> restart B did not release the exact latest candidate gate");

		Fixture interrupted;
		interrupted.Service.Record_GameplaySourceActivationExpectation(
			Revision('b'), "HOT_RELOAD", "first Flow save");
		interrupted.Apply('b');
		interrupted.Service.Record_GameplaySourceActivationExpectation(
			Revision('c'), "HOT_RELOAD", "second Flow save");
		Require(interrupted.Service.Queue_GameplaySourceCandidateAfterPending(
			Revision('c'), "HOT_RELOAD", interrupted.Status),
			"interrupted second Save was not queued");
		++interrupted.Input().iWorldInboundGeneration;
		interrupted.Input().bOutstandingPrepareRequest = false;
		interrupted.Service.Update();
		Require(interrupted.Input().SentRequests.size() == 1u &&
			interrupted.Snapshot().eState ==
				VALTAN_TUNING_COMMAND_STATE::PUBLISHED_APPLY_NEEDED,
			"a world change was mistaken for an exact terminal result and submitted the queued candidate");
		Require(interrupted.Service.Queue_GameplaySourceCandidateAfterPending(
				Revision('c'), "HOT_RELOAD", interrupted.Status) &&
			interrupted.Input().SentRequests.size() == 2u,
			"an explicit retry in the fresh world did not release the still-latest saved candidate");
	}
}

int Run_ValtanTuningCommandServiceTests()
{
	const std::vector<std::pair<const char*, void (*)()>> tests{
		{ "candidate and exact Server commit are separate", VerifyCandidateAndServerCommitAreSeparate },
		{ "candidate input and Release policy fail closed", VerifyInputAndReleasePolicyFailClosed },
		{ "candidate commands share pending/request ownership", VerifyPendingAndSequenceOwnership },
		{ "failed typed send retains candidate and sequence", VerifyFailedSendRetainsCandidateAndSequence },
		{ "result identity is exact", VerifyResultIdentityIsExact },
		{ "Apply timeout and rejection preserve truthful state", VerifyTimeoutAndRejectionRemainTruthful },
		{ "connection/world freshness is scoped", VerifyWorldGenerationAndFreshness },
		{ "request identity exhaustion does not wrap", VerifySequenceExhaustionDoesNotWrap },
		{ "saved gameplay source requires exact Server-active candidate", VerifyGameplaySourceActivationGate },
		{ "two Save and Restart cycles keep latest candidate live", VerifyTwoSaveRestartCandidateLiveness }
	};
	int failed = 0;
	for (const auto& [name, test] : tests)
	{
		try { test(); }
		catch (const std::exception& error)
		{
			++failed;
			std::cerr << "FAIL Tuning " << name << ": " << error.what() << '\n';
		}
	}
	std::cout << "ValtanTuningCommandServiceTests: " << tests.size() - failed << "/"
		<< tests.size() << " passed\n";
	return failed;
}

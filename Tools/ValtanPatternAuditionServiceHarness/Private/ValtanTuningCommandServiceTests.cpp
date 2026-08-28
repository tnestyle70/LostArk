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
	void Require(const bool Condition, const char* Message)
	{
		if (!Condition)
			throw std::runtime_error(Message);
	}

	std::string Revision(const char Digit)
	{
		return std::string(64u, Digit);
	}

	GameplayDataRevision Identity(const char Digit)
	{
		GameplayDataRevision Result;
		Require(Try_Parse_GameplayDataRevision(Revision(Digit), Result), "fixture revision is invalid");
		return Result;
	}

	std::string PublishResult(const char Flow = 'a', const char Candidate = 'b')
	{
		return "{\"schema\":\"lostark.valtan-tuning-command-result\",\"formatVersion\":1,"
			"\"command\":\"PUBLISH_SAVED_FLOW\",\"ok\":true,\"sourceRevision\":\"" +
			Revision('e') + "\",\"candidateRevision\":\"" + Revision(Candidate) +
			"\",\"payload\":{\"flowRevision\":\"" + Revision(Flow) +
			"\",\"applyClass\":\"HOT_RELOAD\",\"splitJoinValidated\":true},\"errors\":[]}";
	}

	std::string Replace(std::string Text, const std::string& Before, const std::string& After)
	{
		const auto Offset = Text.find(Before);
		Require(Offset != std::string::npos, "fixture replacement token was not found");
		Text.replace(Offset, Before.size(), After);
		return Text;
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
		const auto& Snapshot() const { return Service.Get_Snapshot(); }

		void Publish(const char Flow = 'a')
		{
			Input().bPublishRunning = true;
			Require(Service.Publish_SavedPatternFlow(Revision(Flow), Status), "publish did not start");
		}

		void Finish(const std::string& Output = PublishResult(), const uint32_t ExitCode = 0u)
		{
			Input().bPublishRunning = false;
			Input().strPublishOutput = Output;
			Input().iPublishExitCode = ExitCode;
			Service.Update();
		}

		void Apply(const char Candidate = 'b')
		{
			Require(Service.ApplyCandidate(Revision(Candidate), "HOT_RELOAD", Status), "Apply was not accepted");
		}

		void Terminal(const DATA_REVISION_RESULT Result = DATA_REVISION_RESULT::COMMITTED)
		{
			const auto& Request = Input().SentRequests.back();
			Input().bOutstandingPrepareRequest = false;
			Input().bHasLatestResult = true;
			Input().iLatestTransactionSequence = Request.iTransactionSequence;
			Input().LatestCandidateRevision = Request.CandidateRevision;
			Input().eLatestResult = Result;
			Input().strLatestTransactionReason = Result == DATA_REVISION_RESULT::ABORTED ?
				"candidate artifact was not available on every participant" : "";
			if (Result == DATA_REVISION_RESULT::COMMITTED)
				Input().ServerActiveRevision = Request.CandidateRevision;
			Service.Update();
		}
	};

	void VerifyPublishAndServerCommitAreSeparate()
	{
		Fixture F;
		F.Publish();
		Require(F.Service.Has_PendingCommand() &&
			F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::PUBLISHING &&
			F.Snapshot().strFlowRevision == Revision('a') && F.Input().SentRequests.empty(),
			"starting a publisher pretended the candidate was applied");
		F.Finish();
		Require(F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::APPLY_PENDING &&
			F.Snapshot().strCandidateRevision == Revision('b') &&
			F.Snapshot().strFlowRevision == Revision('a') &&
			F.Input().SentRequests.size() == 1u && !F.Snapshot().bCandidateIsServerActive &&
			F.Input().ServerActiveRevision == Identity('d'),
			"publisher completion changed the runtime revision or lost save identity");
		const auto& Request = F.Input().SentRequests.back();
		CPacketWriter Writer;
		Require(Write_Message(Writer, Request) && Request.BaseRevision == Identity('d') &&
			Request.CandidateRevision == Identity('b') &&
			Request.iRequiredPresentationLaneMask == GAMEPLAY_PRESENTATION_KNOWN_LANE_MASK,
			"Apply bypassed the existing complete typed revision boundary");
		F.Terminal();
		Require(F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::COMMITTED &&
			F.Snapshot().bCandidateIsServerActive && !F.Service.Has_PendingCommand(),
			"matching Server commit did not settle the saved Flow");
	}

	void VerifyMalformedPublisherResultsFailClosed()
	{
		const std::vector<std::string> Invalid{
			"not JSON",
			Replace(PublishResult(), "lostark.valtan-tuning-command-result", "wrong-schema"),
			Replace(PublishResult(), "\"formatVersion\":1", "\"formatVersion\":2"),
			Replace(PublishResult(), "\"formatVersion\":1", "\"formatVersion\":1.0"),
			Replace(PublishResult(), "PUBLISH_SAVED_FLOW", "PUBLISH_CANDIDATE"),
			PublishResult('c'),
			Replace(PublishResult(), Revision('b'), "not-a-revision"),
			Replace(PublishResult(), Revision('e'), "bad-source"),
			Replace(PublishResult(), "\"splitJoinValidated\":true", "\"splitJoinValidated\":false"),
			Replace(PublishResult(), "HOT_RELOAD", "UNKNOWN"),
			Replace(PublishResult(), "\"ok\":true", "\"ok\":true,\"ok\":true"),
			Replace(PublishResult(), "\"candidateRevision\":\"" + Revision('b') + "\"", "\"candidateRevision\":null")
		};
		for (const auto& Output : Invalid)
		{
			Fixture F;
			F.Publish();
			F.Finish(Output);
			Require(F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::FAILED &&
				!F.Snapshot().strStatus.empty() && F.Snapshot().strCandidateRevision.empty() &&
				F.Input().SentRequests.empty() && !F.Service.Has_PendingCommand() &&
				F.Input().ServerActiveRevision == Identity('d'),
				"malformed or mismatched pipeline result requested Server application");
		}
	}

	void VerifyPublisherFailuresPreserveDiagnostics()
	{
		Fixture F;
		F.Publish();
		const std::string Failure = "{\"schema\":\"lostark.valtan-tuning-command-result\",\"formatVersion\":1,"
			"\"command\":\"PUBLISH_SAVED_FLOW\",\"ok\":false,\"sourceRevision\":null,"
			"\"candidateRevision\":null,\"payload\":{},\"errors\":[{\"errorCode\":\"FLOW_CAS_DRIFT\","
			"\"message\":\"saved Flow changed before publication\"}]}";
		F.Finish(Failure, 1u);
		Require(F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::FAILED &&
			F.Snapshot().strFlowRevision == Revision('a') &&
			F.Snapshot().strStatus.find("FLOW_CAS_DRIFT") != std::string::npos &&
			F.Snapshot().strStatus.find("saved Flow changed") != std::string::npos &&
			F.Input().SentRequests.empty(), "publisher failure lost the CAS reason or saved draft identity");
		F.Publish();
		F.Finish(PublishResult(), 7u);
		Require(F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::FAILED &&
			F.Input().SentRequests.empty(), "nonzero helper exit was accepted as success");
		F.Publish();
		F.Input().bPublishOutputReadable = false;
		F.Finish();
		Require(F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::FAILED &&
			!F.Service.Has_PendingCommand(), "known helper exit with unreadable output stayed falsely active");
	}

	void VerifyUnconfirmedPublisherRetainsOwnership()
	{
		Fixture F;
		F.Publish();
		F.Input().iNowMilliseconds += 120001u;
		F.Service.Update();
		Require(F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::UNCONFIRMED &&
			F.Service.Has_PendingCommand() && F.Input().bPublishRunning &&
			!F.Service.Publish_SavedPatternFlow(Revision('c'), F.Status) &&
			!F.Service.ApplyCandidate(Revision('c'), "HOT_RELOAD", F.Status) &&
			F.Input().PublishedFlowRevisions.size() == 1u && F.Input().SentRequests.empty(),
			"publisher timeout lost ownership, spawned another helper, or applied an unconfirmed result");
		F.Input().bPublishPollSucceeds = false;
		F.Service.Update();
		Require(F.Service.Has_PendingCommand(), "unknown process poll released the duplicate-command lock");
		F.Input().bPublishPollSucceeds = true;
		F.Finish();
		Require(F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::APPLY_PENDING &&
			F.Input().SentRequests.size() == 1u, "late helper completion could not resume the original command");
	}

	void VerifyOfflinePublishRequiresExplicitApply()
	{
		Fixture F;
		F.Input().bConnected = false;
		F.Publish();
		F.Finish();
		Require(F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::PUBLISHED_APPLY_NEEDED &&
			!F.Service.Has_PendingCommand() && F.Input().SentRequests.empty() &&
			F.Snapshot().strCandidateRevision == Revision('b'), "offline publish lost its staged candidate");
		F.Input().bConnected = true;
		++F.Input().iConnectionGeneration;
		F.Service.Update();
		Require(F.Input().SentRequests.empty(), "reconnect silently applied a candidate without a retry");
		F.Apply();
		Require(F.Snapshot().strFlowRevision == Revision('a') &&
			F.Snapshot().iConnectionGeneration == F.Input().iConnectionGeneration,
			"retry lost the published Flow or used the previous connection");
	}

	void VerifySharedPendingAndSequenceOwnership()
	{
		Fixture F;
		F.Apply();
		const auto First = F.Input().SentRequests.back();
		Require(!F.Service.Publish_SavedPatternFlow(Revision('a'), F.Status) &&
			!F.Service.ApplyCandidate(Revision('c'), "HOT_RELOAD", F.Status) &&
			F.Snapshot().strCandidateRevision == Revision('b') &&
			F.Snapshot().iRequestSequence == First.iTransactionSequence,
			"Flow or Balance replaced an unresolved candidate");
		F.Terminal();
		F.Publish();
		F.Finish(PublishResult('a', 'c'));
		Require(F.Input().SentRequests.back().iTransactionSequence == First.iTransactionSequence + 1u &&
			F.Input().SentRequests.back().BaseRevision == Identity('b'),
			"Balance and saved Flow did not share one current-base/request-sequence owner");
		for (int Kind = 0; Kind < 3; ++Kind)
		{
			Fixture Other;
			Other.Input().bOutstandingPrepareRequest = Kind == 0;
			Other.Input().bStagedPresentationAlias = Kind == 1;
			Other.Input().bRejectedPrepareAwaitingAbort = Kind == 2;
			Require(Other.Service.Has_PendingCommand() &&
				!Other.Service.Publish_SavedPatternFlow(Revision('a'), Other.Status) &&
				!Other.Service.ApplyCandidate(Revision('b'), "HOT_RELOAD", Other.Status) &&
				Other.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::IDLE,
				"another participant's revision transaction was ignored or overwritten");
		}
	}

	void VerifyFailedSendRetainsPublishedCandidate()
	{
		Fixture F;
		F.Input().bSendSucceeds = false;
		F.Publish();
		F.Finish();
		Require(F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::PUBLISHED_APPLY_NEEDED &&
			!F.Service.Has_PendingCommand() && F.Snapshot().strFlowRevision == Revision('a'),
			"failed send erased the saved candidate or pretended it was pending on Server");
		const auto Failed = F.Input().SentRequests.back();
		F.Input().bSendSucceeds = true;
		F.Apply();
		Require(F.Input().SentRequests.back().iTransactionSequence == Failed.iTransactionSequence,
			"an unsent command consumed the shared request sequence");
	}

	void VerifyResultIdentityAndOldSuccessAreNotReused()
	{
		Fixture F;
		F.Input().bHasLatestResult = true;
		F.Input().iLatestTransactionSequence = 1u;
		F.Input().LatestCandidateRevision = Identity('b');
		F.Input().eLatestResult = DATA_REVISION_RESULT::COMMITTED;
		F.Apply();
		F.Service.Update();
		Require(F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::APPLY_PENDING,
			"an old exact-looking result acknowledged a newly outstanding request");
		F.Input().bOutstandingPrepareRequest = false;
		F.Service.Update();
		Require(F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::APPLY_PENDING,
			"COMMITTED with a different active candidate was accepted");
		F.Input().ServerActiveRevision = Identity('b');
		F.Input().LatestCandidateRevision = Identity('c');
		F.Service.Update();
		Require(F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::APPLY_PENDING,
			"different candidate result acknowledged this Apply");
		F.Input().LatestCandidateRevision = Identity('b');
		F.Input().iLatestTransactionSequence = 2u;
		F.Service.Update();
		Require(F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::APPLY_PENDING,
			"different request result acknowledged this Apply");
		F.Input().iLatestTransactionSequence = 1u;
		F.Service.Update();
		Require(F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::COMMITTED,
			"exact candidate/request/active result was not accepted");
	}

	void VerifyApplyTimeoutAndRejection()
	{
		Fixture F;
		F.Publish();
		F.Finish();
		F.Input().iNowMilliseconds += 10001u;
		F.Service.Update();
		Require(F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::UNCONFIRMED &&
			F.Service.Has_PendingCommand() &&
			!F.Service.ApplyCandidate(Revision('c'), "HOT_RELOAD", F.Status),
			"timeout fabricated cancellation or allowed a competing Apply");
		F.Terminal(DATA_REVISION_RESULT::ABORTED);
		Require(F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::FAILED &&
			!F.Service.Has_PendingCommand() && F.Snapshot().strFlowRevision == Revision('a') &&
			F.Snapshot().strCandidateRevision == Revision('b') &&
			F.Snapshot().strStatus.find("candidate artifact") != std::string::npos &&
			F.Input().ServerActiveRevision == Identity('d'),
			"ABORT lost the failure reason, saved candidate, or old active revision");
	}

	void VerifyConnectionAndWorldChangeCannotAdoptOldCommit()
	{
		for (const bool ChangeWorld : { false, true })
		{
			Fixture F;
			F.Apply();
			const auto OldRequest = F.Input().SentRequests.back();
			if (ChangeWorld)
				++F.Input().iWorldInboundGeneration;
			else
				++F.Input().iConnectionGeneration;
			F.Input().bOutstandingPrepareRequest = false;
			F.Input().bHasLatestResult = true;
			F.Input().iLatestTransactionSequence = OldRequest.iTransactionSequence;
			F.Input().LatestCandidateRevision = OldRequest.CandidateRevision;
			F.Input().eLatestResult = DATA_REVISION_RESULT::COMMITTED;
			F.Service.Update();
			Require(F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::PUBLISHED_APPLY_NEEDED &&
				!F.Service.Has_PendingCommand() &&
				F.Snapshot().strStatus.find("does not cancel or confirm") != std::string::npos,
				"old-generation result claimed success or cancellation in a new world");
			F.Apply();
			Require(F.Snapshot().iRequestSequence == OldRequest.iTransactionSequence + 1u &&
				F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::APPLY_PENDING,
				"new world retry reused the previous transaction identity");
		}
	}

	void VerifyAlreadyActiveAndFreshnessAreTruthful()
	{
		Fixture F;
		Require(!F.Service.Is_SavedPatternFlowServerActive(Revision('a')),
			"an IDLE tuning snapshot authorized saved Flow playback");
		F.Input().ServerActiveRevision = Identity('b');
		F.Publish();
		Require(!F.Service.Is_SavedPatternFlowServerActive(Revision('a')),
			"a publishing Flow authorized playback before Server application");
		F.Finish();
		Require(F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::ALREADY_ACTIVE &&
			F.Snapshot().bCandidateIsServerActive && F.Snapshot().iRequestSequence == 0u &&
			F.Input().SentRequests.empty() && !F.Service.Has_PendingCommand() &&
			F.Service.Is_SavedPatternFlowServerActive(Revision('a')) &&
			!F.Service.Is_SavedPatternFlowServerActive(Revision('c')),
			"already-active candidate forged a COMMITTED result or an invalid same-base request");
		F.Input().ServerActiveRevision = Identity('d');
		Require(!F.Service.Is_SavedPatternFlowServerActive(Revision('a')),
			"a stale terminal snapshot authorized playback before the next Update");
		F.Service.Update();
		Require(F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::PUBLISHED_APPLY_NEEDED &&
			!F.Snapshot().bCandidateIsServerActive, "another active revision left a stale applied display");
		F.Apply();
		F.Terminal();
		Require(F.Service.Is_SavedPatternFlowServerActive(Revision('a')),
			"the exact saved Flow and committed candidate were not authorized");
		++F.Input().iWorldInboundGeneration;
		Require(!F.Service.Is_SavedPatternFlowServerActive(Revision('a')),
			"an earlier world generation authorized saved Flow playback");
		F.Service.Update();
		Require(F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::ALREADY_ACTIVE &&
			F.Snapshot().iRequestSequence == 0u && F.Snapshot().bCandidateIsServerActive &&
			F.Service.Is_SavedPatternFlowServerActive(Revision('a')),
			"new world inherited a previous world's COMMITTED transaction");
		F.Input().bConnected = false;
		Require(!F.Service.Is_SavedPatternFlowServerActive(Revision('a')),
			"a disconnected live observation authorized saved Flow playback");
		F.Service.Update();
		Require(!F.Snapshot().bCandidateIsServerActive &&
			F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::PUBLISHED_APPLY_NEEDED,
			"disconnected candidate stayed currently applied");
	}

	void VerifyValidationPolicyAndFailedSpawn()
	{
		Fixture F;
		for (const auto& Invalid : std::vector<std::string>{ "", "bad", Revision('A'), "a\" -Mode Publish" })
		{
			Require(!F.Service.Publish_SavedPatternFlow(Invalid, F.Status), "invalid Flow revision started a helper");
		}
		F.Apply();
		F.Terminal();
		Require(!F.Service.Is_SavedPatternFlowServerActive(Revision('a')),
			"a direct candidate Apply without saved Flow lineage authorized playback");
		F.Input().bPublishStartSucceeds = false;
		Require(!F.Service.Publish_SavedPatternFlow(Revision('a'), F.Status) &&
			F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::COMMITTED &&
			F.Snapshot().strCandidateRevision == Revision('b'), "helper creation failure overwrote confirmed state");
		F.Input().bActivationEnabled = false;
		Require(!F.Service.Publish_SavedPatternFlow(Revision('a'), F.Status) &&
			!F.Service.ApplyCandidate(Revision('c'), "HOT_RELOAD", F.Status) &&
			F.Input().SentRequests.size() == 1u, "Release activation policy submitted a command");
	}

	void VerifyUnsupportedApplyClassAndSequenceExhaustion()
	{
		for (const char* Class : { "ENCOUNTER_RESET", "SERVER_RESTART" })
		{
			Fixture F;
			F.Publish();
			F.Finish(Replace(PublishResult(), "HOT_RELOAD", Class));
			Require(F.Snapshot().eState == VALTAN_TUNING_COMMAND_STATE::PUBLISHED_APPLY_NEEDED &&
				F.Snapshot().strApplyClass == Class && F.Input().SentRequests.empty() &&
				!F.Service.ApplyCandidate(Revision('b'), Class, F.Status),
				"unsupported apply class entered the Hot Reload transaction");
		}
		Fixture F;
		F.Service.Harness_SetNextRequestSequence((std::numeric_limits<uint32_t>::max)());
		F.Apply();
		F.Terminal();
		Require(!F.Service.ApplyCandidate(Revision('c'), "HOT_RELOAD", F.Status) &&
			F.Input().SentRequests.size() == 1u &&
			F.Input().SentRequests.front().iTransactionSequence == (std::numeric_limits<uint32_t>::max)(),
			"request identity wrapped into a previous session-local command");
	}
}

int Run_ValtanTuningCommandServiceTests()
{
	const std::vector<std::pair<const char*, void (*)()>> Tests{
		{ "publish and exact Server commit are separate", VerifyPublishAndServerCommitAreSeparate },
		{ "production JSON parser rejects malformed/mismatched results", VerifyMalformedPublisherResultsFailClosed },
		{ "publisher errors preserve draft identity and diagnostics", VerifyPublisherFailuresPreserveDiagnostics },
		{ "unconfirmed owned publisher blocks duplicates until exit", VerifyUnconfirmedPublisherRetainsOwnership },
		{ "offline publication retains candidate for explicit Apply", VerifyOfflinePublishRequiresExplicitApply },
		{ "Flow and Balance share pending/request ownership", VerifySharedPendingAndSequenceOwnership },
		{ "failed typed send retains the published candidate", VerifyFailedSendRetainsPublishedCandidate },
		{ "stale result and wrong identity cannot acknowledge Apply", VerifyResultIdentityAndOldSuccessAreNotReused },
		{ "Apply timeout and exact rejection preserve truthful status", VerifyApplyTimeoutAndRejection },
		{ "new connection/world cannot adopt an old commit", VerifyConnectionAndWorldChangeCannotAdoptOldCommit },
		{ "already active is distinct and current-world checked", VerifyAlreadyActiveAndFreshnessAreTruthful },
		{ "input/Release policy and failed process creation", VerifyValidationPolicyAndFailedSpawn },
		{ "unsupported apply class and sequence exhaustion", VerifyUnsupportedApplyClassAndSequenceExhaustion }
	};
	int Failed = 0;
	for (const auto& [Name, Test] : Tests)
	{
		try { Test(); }
		catch (const std::exception& Error)
		{
			++Failed;
			std::cerr << "FAIL Tuning " << Name << ": " << Error.what() << '\n';
		}
	}
	std::cout << "ValtanTuningCommandServiceTests: " << Tests.size() - Failed << "/"
		<< Tests.size() << " passed\n";
	return Failed;
}

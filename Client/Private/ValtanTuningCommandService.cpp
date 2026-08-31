#include "DataJson.h"
#include "ValtanTuningCommandService.h"

#include <algorithm>
#include <fstream>
#include <limits>
#include <utility>

#include "Network/PacketWriter.h"
#if !defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
#include "NetworkManager.h"
#include "ProjectDataRoot.h"
#endif

namespace
{
	constexpr uint64_t PUBLISH_TIMEOUT_MILLISECONDS = 120000u;
	constexpr uint64_t APPLY_TIMEOUT_MILLISECONDS = 10000u;
	constexpr std::size_t MAX_PUBLISH_OUTPUT_BYTES = 2u * 1024u * 1024u;

	bool Is_LowerSha256(const std::string_view Value)
	{
		return Value.size() == 64u &&
			std::all_of(Value.begin(), Value.end(), [](const char Character)
			{
				return (Character >= '0' && Character <= '9') ||
					(Character >= 'a' && Character <= 'f');
			});
	}

	const Client::DATA_JSON_VALUE* Field(
		const Client::DATA_JSON_VALUE& Object,
		const std::string_view Name, const Client::DATA_JSON_TYPE Type)
	{
		const auto* Value = Object.Find(Name);
		return nullptr != Value && Value->Get_Type() == Type ? Value : nullptr;
	}

	bool Parse_PublishResult(const std::string& Output, const std::string& ExpectedFlow,
		std::string& CandidateRevision, std::string& ApplyClass, std::string& Status)
	{
		using namespace Client;
		DATA_JSON_VALUE Root;
		DATA_JSON_PARSE_LIMITS Limits;
		Limits.iMaximumBytes = MAX_PUBLISH_OUTPUT_BYTES;
		Limits.iMaximumDepth = 32u;
		Limits.iMaximumValues = 65536u;
		if (!CDataJson::Parse(Output, Root, Status, Limits) || !Root.Is_Object())
		{
			Status = "Saved Flow publisher returned malformed JSON: " + Status;
			return false;
		}
		const auto* Schema = Field(Root, "schema", DATA_JSON_TYPE::STRING);
		const auto* Version = Field(Root, "formatVersion", DATA_JSON_TYPE::NUMBER);
		const auto* Command = Field(Root, "command", DATA_JSON_TYPE::STRING);
		const auto* Ok = Field(Root, "ok", DATA_JSON_TYPE::BOOLEAN);
		const auto* Payload = Field(Root, "payload", DATA_JSON_TYPE::OBJECT);
		const auto* Errors = Field(Root, "errors", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Schema || Schema->Get_String() != "lostark.valtan-tuning-command-result" ||
			nullptr == Version || Version->Was_FloatingPointToken() || Version->Get_Number() != 1.0 ||
			nullptr == Command || Command->Get_String() != "PUBLISH_SAVED_FLOW" ||
			nullptr == Ok || nullptr == Payload || nullptr == Errors)
		{
			Status = "Saved Flow publisher returned an invalid command/result schema.";
			return false;
		}
		if (!Ok->Get_Boolean() || !Errors->Get_Array().empty())
		{
			Status = "Saved Flow publish failed; the saved draft is retained and Server application was not requested.";
			for (const auto& Error : Errors->Get_Array())
			{
				const auto* Code = Field(Error, "errorCode", DATA_JSON_TYPE::STRING);
				const auto* Message = Field(Error, "message", DATA_JSON_TYPE::STRING);
				if (nullptr != Code) Status += " " + Code->Get_String() + ":";
				if (nullptr != Message) Status += " " + Message->Get_String();
				if (Status.size() > 4096u)
				{
					Status.resize(4096u);
					break;
				}
			}
			return false;
		}
		const auto* Flow = Field(*Payload, "flowRevision", DATA_JSON_TYPE::STRING);
		const auto* Source = Field(Root, "sourceRevision", DATA_JSON_TYPE::STRING);
		const auto* Candidate = Field(Root, "candidateRevision", DATA_JSON_TYPE::STRING);
		const auto* Apply = Field(*Payload, "applyClass", DATA_JSON_TYPE::STRING);
		const auto* Joined = Field(*Payload, "splitJoinValidated", DATA_JSON_TYPE::BOOLEAN);
		if (nullptr == Flow || Flow->Get_String() != ExpectedFlow ||
			nullptr == Source || !Is_LowerSha256(Source->Get_String()) ||
			nullptr == Candidate || !Is_LowerSha256(Candidate->Get_String()) ||
			nullptr == Apply || (Apply->Get_String() != "HOT_RELOAD" &&
				Apply->Get_String() != "ENCOUNTER_RESET" && Apply->Get_String() != "SERVER_RESTART") ||
			nullptr == Joined || !Joined->Get_Boolean())
		{
			Status = "Saved Flow publisher identity, source join, or apply class does not match this save.";
			return false;
		}
		CandidateRevision = Candidate->Get_String();
		ApplyClass = Apply->Get_String();
		Status.clear();
		return true;
	}
}

Client::CValtanTuningCommandService& Client::CValtanTuningCommandService::Get()
{
	static CValtanTuningCommandService Service;
	return Service;
}

Client::CValtanTuningCommandService::~CValtanTuningCommandService()
{
#if !defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	// Closing the editor must not kill a helper inside its durable transaction.
	if (nullptr != m_hPublishProcess)
		CloseHandle(m_hPublishProcess);
#endif
}

bool Client::CValtanTuningCommandService::Has_PendingCommand() const
{
	return m_bPublishing || m_bApplyPending ||
		Read_RevisionObservation().bOtherTransactionPending;
}

bool Client::CValtanTuningCommandService::Is_SavedPatternFlowServerActive(
	const std::string_view strSavedRevision) const
{
	using namespace LostArk::Shared;
	const bool bTerminal =
		VALTAN_TUNING_COMMAND_STATE::COMMITTED == m_Snapshot.eState ||
		VALTAN_TUNING_COMMAND_STATE::ALREADY_ACTIVE == m_Snapshot.eState;
	GameplayDataRevision CandidateRevision;
	if (!Is_LowerSha256(strSavedRevision) ||
		m_Snapshot.strFlowRevision != strSavedRevision || !bTerminal ||
		!Try_Parse_GameplayDataRevision(
			m_Snapshot.strCandidateRevision, CandidateRevision))
	{
		return false;
	}

	const REVISION_OBSERVATION Observation = Read_RevisionObservation();
	return Observation.bConnected &&
		0u != Observation.iConnectionGeneration &&
		0u != Observation.iWorldInboundGeneration &&
		m_Snapshot.iConnectionGeneration == Observation.iConnectionGeneration &&
		m_Snapshot.iWorldInboundGeneration == Observation.iWorldInboundGeneration &&
		Observation.ServerActiveRevision.Is_Valid() &&
		Observation.ServerActiveRevision == CandidateRevision;
}

void Client::CValtanTuningCommandService::
	Record_GameplaySourceActivationExpectation(
		const std::string_view strCandidateRevision,
		const std::string_view strApplyClass,
		const std::string_view strStatus)
{
	m_bGameplaySourceActivationObserved = true;
	m_strGameplayCandidateRevision = std::string(strCandidateRevision);
	m_strGameplayCandidateApplyClass = std::string(strApplyClass);
	m_strGameplayActivationStatus = std::string(strStatus);
	if (!m_strGameplayCandidateRevision.empty() &&
		!Is_LowerSha256(m_strGameplayCandidateRevision))
	{
		m_strGameplayActivationStatus =
			"The saved gameplay source produced an invalid candidate revision; Server activation cannot be confirmed.";
		m_strGameplayCandidateRevision.clear();
		m_strGameplayCandidateApplyClass.clear();
	}
}

bool Client::CValtanTuningCommandService::
	Is_LatestGameplaySourceServerActive(std::string& strOutStatus) const
{
	using namespace LostArk::Shared;
	if (!m_bGameplaySourceActivationObserved)
	{
		strOutStatus.clear();
		return true;
	}

	GameplayDataRevision CandidateRevision;
	if (m_strGameplayCandidateRevision.empty() ||
		!Try_Parse_GameplayDataRevision(
			m_strGameplayCandidateRevision, CandidateRevision))
	{
		strOutStatus =
			"Complete Play is blocked because the latest saved gameplay source has no admitted Product candidate.";
		if (!m_strGameplayActivationStatus.empty())
			strOutStatus += " " + m_strGameplayActivationStatus;
		return false;
	}

	const REVISION_OBSERVATION Observation = Read_RevisionObservation();
	if (Observation.bConnected &&
		Observation.ServerActiveRevision.Is_Valid() &&
		Observation.ServerActiveRevision == CandidateRevision)
	{
		strOutStatus.clear();
		return true;
	}

	strOutStatus =
		"Complete Play is blocked until the latest saved gameplay candidate is the Server-active revision";
	if (!m_strGameplayCandidateApplyClass.empty())
		strOutStatus += " (apply class " +
			m_strGameplayCandidateApplyClass + ")";
	strOutStatus += ".";
	if (!m_strGameplayActivationStatus.empty())
		strOutStatus += " " + m_strGameplayActivationStatus;
	return false;
}

bool Client::CValtanTuningCommandService::Publish_SavedPatternFlow(
	const std::string_view strSavedRevision, std::string& strOutStatus)
{
	const std::string SavedRevision(strSavedRevision);
	Update();
	if (!Is_ActivationEnabled())
	{
		strOutStatus = "Saved Flow publication is a Debug editor command.";
		return false;
	}
	if (!Is_LowerSha256(SavedRevision))
	{
		strOutStatus = "Save a valid Flow revision before publishing the Product order.";
		return false;
	}
	if (Has_PendingCommand() || Read_RevisionObservation().bOtherTransactionPending)
	{
		strOutStatus = "A Valtan publish or revision transaction is still pending; the current command is retained.";
		return false;
	}
	if (!Start_PublishProcess(SavedRevision, strOutStatus))
		return false;
	VALTAN_TUNING_COMMAND_SNAPSHOT Pending;
	Pending.eState = VALTAN_TUNING_COMMAND_STATE::PUBLISHING;
	Pending.strFlowRevision = SavedRevision;
	Pending.strStatus = "Saved Flow is publishing. The Server revision is unchanged until application is confirmed.";
	m_Snapshot = std::move(Pending);
	m_bPublishing = true;
	m_iPublishStartedAtMilliseconds = Now_Milliseconds();
	strOutStatus = m_Snapshot.strStatus;
	return true;
}

bool Client::CValtanTuningCommandService::ApplyCandidate(
	const std::string_view strCandidateRevision, const std::string_view strApplyClass,
	std::string& strOutStatus)
{
	// Callers may pass string_views into the current snapshot.
	const std::string Candidate(strCandidateRevision);
	const std::string ApplyClass(strApplyClass);
	Update();
	if (Has_PendingCommand())
	{
		strOutStatus = "The current Valtan command has no terminal result yet.";
		return false;
	}
	if (!Is_LowerSha256(Candidate) || ApplyClass != "HOT_RELOAD")
	{
		strOutStatus = ApplyClass == "ENCOUNTER_RESET" || ApplyClass == "SERVER_RESTART" ?
			"This candidate requires a controlled encounter reset or Server restart; Hot Reload was not requested." :
			"Apply requires a valid immutable HOT_RELOAD candidate.";
		return false;
	}
	const REVISION_OBSERVATION Observation = Read_RevisionObservation();
	if (Observation.bOtherTransactionPending)
	{
		strOutStatus = "Another Valtan revision transaction is pending; its candidate was not replaced.";
		return false;
	}
	if (m_Snapshot.strCandidateRevision != Candidate)
	{
		m_Snapshot = {};
		m_Snapshot.strCandidateRevision = Candidate;
	}
	m_Snapshot.strApplyClass = ApplyClass;
	m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::PUBLISHED_APPLY_NEEDED;
	return Submit_Candidate(Observation, strOutStatus);
}

bool Client::CValtanTuningCommandService::Submit_Candidate(
	const REVISION_OBSERVATION& Observation, std::string& strOutStatus)
{
	using namespace LostArk::Shared;
	auto Reject = [&](std::string Reason)
	{
		m_Snapshot.strStatus = std::move(Reason);
		strOutStatus = m_Snapshot.strStatus;
		return false;
	};
	Refresh_ActiveCandidate(Observation);
	if (m_Snapshot.bCandidateIsServerActive)
	{
		m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::ALREADY_ACTIVE;
		m_Snapshot.iConnectionGeneration = Observation.iConnectionGeneration;
		m_Snapshot.iWorldInboundGeneration = Observation.iWorldInboundGeneration;
		m_Snapshot.iRequestSequence = 0u;
		m_Snapshot.strStatus = "The Server already reports this candidate active. No new revision transaction was submitted.";
		strOutStatus = m_Snapshot.strStatus;
		return true;
	}
	if (!Is_ActivationEnabled())
		return Reject("Release Client does not initiate gameplay data revision transactions.");
	if (m_Snapshot.strApplyClass != "HOT_RELOAD")
		return Reject("Candidate publication succeeded, but its apply class requires a controlled reset or Server restart.");
	if (!Observation.bConnected || Observation.iConnectionGeneration == 0u ||
		Observation.iWorldInboundGeneration == 0u || !Observation.ServerActiveRevision.Is_Valid())
	{
		return Reject("Candidate published. Connect to the Server in Valtan Arena, then retry Apply; runtime application is not confirmed.");
	}
	if (Observation.bOtherTransactionPending)
		return Reject("Candidate published, but another revision transaction is pending. Retry Apply after it settles.");
	if (m_iNextRequestSequence == 0u)
		return Reject("Valtan revision request identities are exhausted; restart the Client before another Apply.");
	C2S_DATA_REVISION_PREPARE_REQUEST Request;
	Request.iTransactionSequence = m_iNextRequestSequence;
	Request.BaseRevision = Observation.ServerActiveRevision;
	if (!Try_Parse_GameplayDataRevision(m_Snapshot.strCandidateRevision, Request.CandidateRevision))
		return Reject("The published candidate revision is invalid.");
	Request.iRequiredPresentationLaneMask = GAMEPLAY_PRESENTATION_KNOWN_LANE_MASK;
#if !defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	const CNetworkManager::PRESENTATION_CANDIDATE_PREFLIGHT_RESULT preflight =
		CNetworkManager::Get().Preflight_PresentationCandidate(
			Request.CandidateRevision,
			Request.iRequiredPresentationLaneMask,
			strOutStatus);
	if (CNetworkManager::PRESENTATION_CANDIDATE_PREFLIGHT_RESULT::
			CURRENT_GENERATION_READY != preflight)
	{
		return Reject(std::move(strOutStatus));
	}
#endif
	if (!Send_PrepareRequest(Request))
		return Reject("Candidate published, but the typed prepare request could not be sent. Runtime application is not confirmed.");
	m_iNextRequestSequence = (std::numeric_limits<uint32_t>::max)() == m_iNextRequestSequence ?
		0u : m_iNextRequestSequence + 1u;
	m_ApplyRequest = std::move(Request);
	m_bApplyPending = true;
	m_iApplyStartedAtMilliseconds = Now_Milliseconds();
	m_Snapshot.iRequestSequence = m_ApplyRequest.iTransactionSequence;
	m_Snapshot.iConnectionGeneration = Observation.iConnectionGeneration;
	m_Snapshot.iWorldInboundGeneration = Observation.iWorldInboundGeneration;
	m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::APPLY_PENDING;
	m_Snapshot.strStatus = "Candidate published; waiting for the matching Server tick commit and Client presentation result.";
	strOutStatus = m_Snapshot.strStatus;
	return true;
}

void Client::CValtanTuningCommandService::Consume_PublishResult(
	const uint32_t iExitCode, const std::string& strOutput)
{
	std::string Candidate;
	std::string ApplyClass;
	std::string Status;
	const bool Parsed = Parse_PublishResult(strOutput, m_Snapshot.strFlowRevision,
		Candidate, ApplyClass, Status);
	if (!Parsed || iExitCode != 0u)
	{
		m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::FAILED;
		m_Snapshot.strStatus = !Status.empty() ? Status :
			"Saved Flow publisher exited with code " + std::to_string(iExitCode) +
			". Server application was not requested; the saved draft is retained.";
		return;
	}
	m_Snapshot.strCandidateRevision = std::move(Candidate);
	m_Snapshot.strApplyClass = std::move(ApplyClass);
	m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::PUBLISHED_APPLY_NEEDED;
	Submit_Candidate(Read_RevisionObservation(), Status);
}

void Client::CValtanTuningCommandService::Refresh_ActiveCandidate(
	const REVISION_OBSERVATION& Observation)
{
	using namespace LostArk::Shared;
	GameplayDataRevision Candidate;
	m_Snapshot.bCandidateIsServerActive = Observation.bConnected &&
		Observation.iConnectionGeneration != 0u && Observation.iWorldInboundGeneration != 0u &&
		Try_Parse_GameplayDataRevision(m_Snapshot.strCandidateRevision, Candidate) &&
		Candidate == Observation.ServerActiveRevision;
	const bool WasConfirmed = m_Snapshot.eState == VALTAN_TUNING_COMMAND_STATE::COMMITTED ||
		m_Snapshot.eState == VALTAN_TUNING_COMMAND_STATE::ALREADY_ACTIVE;
	if (!WasConfirmed)
		return;
	if (!m_Snapshot.bCandidateIsServerActive)
	{
		m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::PUBLISHED_APPLY_NEEDED;
		m_Snapshot.strStatus = "This candidate is not confirmed as the current Server revision. The previous Apply result is historical; retry Apply if needed.";
	}
	else if (m_Snapshot.iConnectionGeneration != Observation.iConnectionGeneration ||
		m_Snapshot.iWorldInboundGeneration != Observation.iWorldInboundGeneration)
	{
		m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::ALREADY_ACTIVE;
		m_Snapshot.iConnectionGeneration = Observation.iConnectionGeneration;
		m_Snapshot.iWorldInboundGeneration = Observation.iWorldInboundGeneration;
		m_Snapshot.iRequestSequence = 0u;
		m_Snapshot.strStatus = "The current Server world reports this candidate active. The previous transaction belonged to an earlier connection or world.";
	}
}

void Client::CValtanTuningCommandService::Update()
{
	using namespace LostArk::Shared;
	if (m_bPublishing)
	{
		bool Finished = false;
		uint32_t ExitCode = 1u;
		std::string Output;
		std::string Status;
		const bool Polled = Poll_PublishProcess(Finished, ExitCode, Output, Status);
		if (Finished)
		{
			m_bPublishing = false;
			if (Polled)
				Consume_PublishResult(ExitCode, Output);
			else
			{
				m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::FAILED;
				m_Snapshot.strStatus = std::move(Status);
			}
		}
		else if (!Polled || Now_Milliseconds() - m_iPublishStartedAtMilliseconds >= PUBLISH_TIMEOUT_MILLISECONDS)
		{
			m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::UNCONFIRMED;
			m_Snapshot.strStatus = !Status.empty() ? Status :
				"Publisher outcome is still unconfirmed. Its owned process is retained; another Save/Apply is blocked until it exits.";
		}
	}
	const REVISION_OBSERVATION Observation = Read_RevisionObservation();
	if (m_bApplyPending)
	{
		if (m_Snapshot.iConnectionGeneration != Observation.iConnectionGeneration ||
			m_Snapshot.iWorldInboundGeneration != Observation.iWorldInboundGeneration)
		{
			m_bApplyPending = false;
			m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::PUBLISHED_APPLY_NEEDED;
			m_Snapshot.strStatus = "Connection or world changed before the matching Apply result was observed. This does not cancel or confirm the Server transaction; retry Apply in the current world.";
		}
		else if (Observation.bConnected && Observation.bHasLatestResult &&
			!Observation.bOutstandingPrepareRequest &&
			Observation.iLatestTransactionSequence == m_ApplyRequest.iTransactionSequence &&
			Observation.LatestCandidateRevision == m_ApplyRequest.CandidateRevision)
		{
			if (Observation.eLatestResult == DATA_REVISION_RESULT::ABORTED)
			{
				m_bApplyPending = false;
				m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::FAILED;
				m_Snapshot.strStatus = "Server rejected the candidate; its previous revision remains active. " + Observation.strReason;
			}
			else if (Observation.eLatestResult == DATA_REVISION_RESULT::COMMITTED &&
				Observation.ServerActiveRevision == m_ApplyRequest.CandidateRevision)
			{
				m_bApplyPending = false;
				m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::COMMITTED;
				m_Snapshot.strStatus = "Server committed this candidate. A running Product sequence keeps its old definition; the next encounter/reset uses the saved order.";
			}
		}
		if (m_bApplyPending && (!Observation.bConnected ||
			Now_Milliseconds() - m_iApplyStartedAtMilliseconds >= APPLY_TIMEOUT_MILLISECONDS))
		{
			m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::UNCONFIRMED;
			m_Snapshot.strStatus = "The Server Apply outcome is unconfirmed. No success or cancellation is assumed; another revision command is blocked.";
		}
	}
	Refresh_ActiveCandidate(Observation);
}

bool Client::CValtanTuningCommandService::Is_ActivationEnabled() const
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	return m_HarnessInput.bActivationEnabled;
#elif defined(_DEBUG)
	return true;
#else
	return false;
#endif
}

uint64_t Client::CValtanTuningCommandService::Now_Milliseconds() const
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	return m_HarnessInput.iNowMilliseconds;
#else
	return GetTickCount64();
#endif
}

Client::CValtanTuningCommandService::REVISION_OBSERVATION
Client::CValtanTuningCommandService::Read_RevisionObservation() const
{
	REVISION_OBSERVATION Observation;
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	Observation.bConnected = m_HarnessInput.bConnected;
	Observation.iConnectionGeneration = m_HarnessInput.iConnectionGeneration;
	Observation.iWorldInboundGeneration = m_HarnessInput.iWorldInboundGeneration;
	Observation.bOtherTransactionPending = m_HarnessInput.bOutstandingPrepareRequest ||
		m_HarnessInput.bStagedPresentationAlias || m_HarnessInput.bRejectedPrepareAwaitingAbort;
	Observation.bOutstandingPrepareRequest = m_HarnessInput.bOutstandingPrepareRequest;
	Observation.bHasLatestResult = m_HarnessInput.bHasLatestResult;
	Observation.iLatestTransactionSequence = m_HarnessInput.iLatestTransactionSequence;
	Observation.ServerActiveRevision = m_HarnessInput.ServerActiveRevision;
	Observation.LatestCandidateRevision = m_HarnessInput.LatestCandidateRevision;
	Observation.eLatestResult = m_HarnessInput.eLatestResult;
	Observation.strReason = m_HarnessInput.strLatestTransactionReason;
#else
	const CNetworkManager& Network = CNetworkManager::Get();
	const auto& State = Network.Get_GameplayRevisionState();
	Observation.bConnected = Network.Is_Connected();
	Observation.iConnectionGeneration = Network.Get_SessionDiagnosticSnapshot().iConnectionGeneration;
	Observation.iWorldInboundGeneration = Network.Get_WorldInboundGeneration();
	Observation.bOtherTransactionPending = State.hasOutstandingPrepareRequest ||
		State.hasStagedPresentationAlias || State.hasRejectedPrepareAwaitingAbort;
	Observation.bOutstandingPrepareRequest = State.hasOutstandingPrepareRequest;
	Observation.bHasLatestResult = State.hasLatestResult;
	Observation.iLatestTransactionSequence = State.iLatestTransactionSequence;
	Observation.ServerActiveRevision = State.ServerActiveRevision;
	Observation.LatestCandidateRevision = State.LatestCandidateRevision;
	Observation.eLatestResult = State.eLatestResult;
	Observation.strReason = State.strLatestTransactionReason;
#endif
	return Observation;
}

bool Client::CValtanTuningCommandService::Send_PrepareRequest(
	const LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST& Request)
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	LostArk::Shared::CPacketWriter Writer;
	if (!LostArk::Shared::Write_Message(Writer, Request))
		return false;
	m_HarnessInput.SentRequests.push_back(Request);
	if (!m_HarnessInput.bSendSucceeds)
		return false;
	m_HarnessInput.bOutstandingPrepareRequest = true;
	return true;
#else
	return CNetworkManager::Get().Send_DataRevisionPrepareRequest(Request);
#endif
}

bool Client::CValtanTuningCommandService::Start_PublishProcess(
	const std::string_view strFlowRevision, std::string& strOutStatus)
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	if (!m_HarnessInput.bPublishStartSucceeds)
	{
		strOutStatus = "Could not start the saved Flow publisher.";
		return false;
	}
	m_HarnessInput.PublishedFlowRevisions.emplace_back(strFlowRevision);
	return true;
#else
	std::error_code Error;
	const auto Root = std::filesystem::weakly_canonical(CProjectDataRoot::Get().parent_path(), Error);
	const auto Script = Root / L"Tools/ValtanPipeline/Publish-ValtanTuningRuntimeSet.ps1";
	if (Error || Root.empty() || !std::filesystem::is_regular_file(Script, Error))
	{
		strOutStatus = "The fixed saved Flow publisher script could not be resolved.";
		return false;
	}
	const auto TemporaryRoot = std::filesystem::temp_directory_path(Error);
	if (Error)
	{
		strOutStatus = "Could not resolve the publisher output directory: " + Error.message();
		return false;
	}
	const auto OutputPath = TemporaryRoot / (L"LostArk.ValtanSavedFlow." +
		std::to_wstring(GetCurrentProcessId()) + L"." + std::to_wstring(GetTickCount64()) + L".json");
	SECURITY_ATTRIBUTES Security{};
	Security.nLength = sizeof(Security);
	Security.bInheritHandle = TRUE;
	const HANDLE Output = CreateFileW(OutputPath.c_str(), GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_DELETE, &Security, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY, nullptr);
	if (Output == INVALID_HANDLE_VALUE)
	{
		strOutStatus = "Could not create publisher output (Win32 " + std::to_string(GetLastError()) + ").";
		return false;
	}
	const HANDLE Input = CreateFileW(L"NUL", GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, &Security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (Input == INVALID_HANDLE_VALUE)
	{
		const DWORD Code = GetLastError();
		CloseHandle(Output);
		std::filesystem::remove(OutputPath, Error);
		strOutStatus = "Could not create publisher input (Win32 " + std::to_string(Code) + ").";
		return false;
	}
	// Only the validated hex revision varies; script and repository paths are fixed.
	std::wstring Command = L"powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"" +
		Script.wstring() + L"\" -Mode PublishSavedFlow -ExpectedFlowRevision " +
		std::wstring(strFlowRevision.begin(), strFlowRevision.end()) + L" -RepositoryRoot \"" + Root.wstring() + L"\"";
	std::vector<wchar_t> MutableCommand(Command.begin(), Command.end());
	MutableCommand.push_back(L'\0');
	STARTUPINFOW Startup{};
	Startup.cb = sizeof(Startup);
	Startup.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	Startup.wShowWindow = SW_HIDE;
	Startup.hStdInput = Input;
	Startup.hStdOutput = Output;
	Startup.hStdError = Output;
	PROCESS_INFORMATION Process{};
	const BOOL Created = CreateProcessW(nullptr, MutableCommand.data(), nullptr,
		nullptr, TRUE, CREATE_NO_WINDOW, nullptr, Root.c_str(), &Startup, &Process);
	const DWORD Code = Created ? ERROR_SUCCESS : GetLastError();
	CloseHandle(Input);
	CloseHandle(Output);
	if (!Created)
	{
		std::filesystem::remove(OutputPath, Error);
		strOutStatus = "Could not start the saved Flow publisher (Win32 " + std::to_string(Code) + ").";
		return false;
	}
	CloseHandle(Process.hThread);
	m_hPublishProcess = Process.hProcess;
	m_PublishOutputPath = OutputPath;
	return true;
#endif
}

bool Client::CValtanTuningCommandService::Poll_PublishProcess(
	bool& bFinished, uint32_t& iExitCode, std::string& strOutput, std::string& strOutStatus)
{
	bFinished = false;
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	if (!m_HarnessInput.bPublishPollSucceeds)
	{
		strOutStatus = "Publisher process status is unconfirmed; the current operation is retained.";
		return false;
	}
	if (m_HarnessInput.bPublishRunning)
		return true;
	bFinished = true;
	if (!m_HarnessInput.bPublishOutputReadable)
	{
		strOutStatus = "Publisher exited, but its structured result could not be read. Server application was not requested.";
		return false;
	}
	iExitCode = m_HarnessInput.iPublishExitCode;
	strOutput = m_HarnessInput.strPublishOutput;
	return true;
#else
	const DWORD Wait = WaitForSingleObject(m_hPublishProcess, 0u);
	if (Wait == WAIT_TIMEOUT)
		return true;
	if (Wait != WAIT_OBJECT_0)
	{
		strOutStatus = "Publisher process status is unconfirmed (Win32 " +
			std::to_string(GetLastError()) + "). Its process and duplicate-command lock are retained.";
		return false;
	}
	bFinished = true;
	DWORD ExitCode = 1u;
	const bool ExitKnown = GetExitCodeProcess(m_hPublishProcess, &ExitCode) != FALSE;
	CloseHandle(m_hPublishProcess);
	m_hPublishProcess = nullptr;
	std::error_code Error;
	const auto Size = std::filesystem::file_size(m_PublishOutputPath, Error);
	if (!ExitKnown || Error || Size > MAX_PUBLISH_OUTPUT_BYTES)
	{
		strOutStatus = "Publisher exited without a readable bounded result. Server application was not requested; inspect " +
			m_PublishOutputPath.string();
		return false;
	}
	std::ifstream Stream(m_PublishOutputPath, std::ios::binary);
	strOutput.resize(static_cast<std::size_t>(Size));
	if (!Stream || (Size != 0u && !Stream.read(strOutput.data(), static_cast<std::streamsize>(Size))))
	{
		strOutStatus = "Publisher exited, but its structured output could not be read: " + m_PublishOutputPath.string();
		return false;
	}
	Stream.close();
	std::filesystem::remove(m_PublishOutputPath, Error);
	m_PublishOutputPath.clear();
	iExitCode = ExitCode;
	return true;
#endif
}

#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
void Client::CValtanTuningCommandService::Harness_Reset()
{
	m_Snapshot = {};
	m_ApplyRequest = {};
	m_bPublishing = false;
	m_bApplyPending = false;
	m_iPublishStartedAtMilliseconds = 0u;
	m_iApplyStartedAtMilliseconds = 0u;
	m_iNextRequestSequence = 1u;
	m_bGameplaySourceActivationObserved = false;
	m_strGameplayCandidateRevision.clear();
	m_strGameplayCandidateApplyClass.clear();
	m_strGameplayActivationStatus.clear();
	m_HarnessInput = {};
}
#endif

const char* Client::Describe_ValtanTuningCommandState(const VALTAN_TUNING_COMMAND_STATE eState)
{
	switch (eState)
	{
	case VALTAN_TUNING_COMMAND_STATE::IDLE: return "IDLE";
	case VALTAN_TUNING_COMMAND_STATE::PUBLISHING: return "PUBLISHING";
	case VALTAN_TUNING_COMMAND_STATE::UNCONFIRMED: return "UNCONFIRMED";
	case VALTAN_TUNING_COMMAND_STATE::PUBLISHED_APPLY_NEEDED: return "PUBLISHED / APPLY NEEDED";
	case VALTAN_TUNING_COMMAND_STATE::APPLY_PENDING: return "APPLY PENDING";
	case VALTAN_TUNING_COMMAND_STATE::COMMITTED: return "COMMITTED";
	case VALTAN_TUNING_COMMAND_STATE::ALREADY_ACTIVE: return "ALREADY ACTIVE";
	case VALTAN_TUNING_COMMAND_STATE::FAILED: return "FAILED";
	default: return "UNKNOWN";
	}
}

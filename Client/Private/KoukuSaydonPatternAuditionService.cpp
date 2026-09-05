#include "KoukuSaydonPatternAuditionService.h"

#include "NetworkManager.h"

#include <Windows.h>

#include <limits>
#include <utility>

namespace
{
	using namespace LostArk::Shared;

	constexpr std::uint64_t VERDICT_TIMEOUT_MILLISECONDS = 5000u;
	constexpr std::uint64_t QUEUED_START_TIMEOUT_MILLISECONDS = 15000u;
	constexpr std::string_view ENCOUNTER_ID = "ENCOUNTER_KAKULSAYDON_G1";
	constexpr std::string_view BOSS_PLACEMENT_ID =
		"boss.kakulsaydon.g1.kouku";
	constexpr std::string_view BOSS_ARCHETYPE_ID =
		"BOSS_KAKULSAYDON_G1_KOUKU";

	KOUKUSAYDON_PATTERN_AUDITION_SCOPE Make_Scope(
		const GameplayDataRevision& revision,
		const std::uint32_t sourceRevision)
	{
		KOUKUSAYDON_PATTERN_AUDITION_SCOPE scope{};
		scope.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
		scope.strEncounterId = ENCOUNTER_ID;
		scope.strBossPlacementId = BOSS_PLACEMENT_ID;
		scope.strBossArchetypeId = BOSS_ARCHETYPE_ID;
		scope.ExpectedGameplayRevision = revision;
		scope.iExpectedSourceRevision = sourceRevision;
		return scope;
	}

	bool Exact_Scope(
		const KOUKUSAYDON_PATTERN_AUDITION_SCOPE& left,
		const KOUKUSAYDON_PATTERN_AUDITION_SCOPE& right)
	{
		return left.eWorldId == right.eWorldId &&
			left.strEncounterId == right.strEncounterId &&
			left.strBossPlacementId == right.strBossPlacementId &&
			left.strBossArchetypeId == right.strBossArchetypeId &&
			left.ExpectedGameplayRevision == right.ExpectedGameplayRevision &&
			left.iExpectedSourceRevision == right.iExpectedSourceRevision;
	}

	std::string Describe_Rejection(const KOUKUSAYDON_PATTERN_AUDITION_RESULT result)
	{
		switch (result)
		{
		case KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_RELEASE_BUILD:
			return "Server Play is Debug-only; start the Debug Server.";
		case KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_SCOPE_MISMATCH:
			return "The Server room does not match the exact KoukuSaydon Gate 1 scope.";
		case KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_NO_BOSS:
			return "The admitted KoukuSaydon boss placement is not alive in this room.";
		case KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_BOSS_DEAD:
			return "The KoukuSaydon boss is dead; re-enter the Arena before replaying.";
		case KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_BUSY:
			return "Another KoukuSaydon Server audition already owns playback.";
		case KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_UNKNOWN_PATTERN:
			return "The Server Product does not contain this stable pattern ID.";
		case KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_NO_PRODUCT_SEQUENCE:
			return "The Server Product contains no admitted Play All sequence.";
		case KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_UNSUPPORTED_PATTERN:
			return "The selected Product pattern contains a stage family this Server does not support.";
		case KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_REVISION_MISMATCH:
			return "Saved Product and the Server-active gameplay revision differ; republish and restart the Server.";
		case KOUKUSAYDON_PATTERN_AUDITION_RESULT::
			REJECTED_SOURCE_REVISION_MISMATCH:
			return "Saved Product source revision differs from the Server catalog; Publish and restart the Server.";
		case KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_STALE_REQUEST:
			return "The Server rejected a stale or reused audition request identity.";
		case KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED:
		case KOUKUSAYDON_PATTERN_AUDITION_RESULT::DUPLICATE_IGNORED:
		case KOUKUSAYDON_PATTERN_AUDITION_RESULT::END:
		default:
			return "The Server returned an unexpected KoukuSaydon audition verdict.";
		}
	}
}

Client::CKoukuSaydonPatternAuditionService&
Client::CKoukuSaydonPatternAuditionService::Get()
{
	static CKoukuSaydonPatternAuditionService instance;
	return instance;
}

const char* Client::Describe_KoukuSaydonPatternAuditionState(
	const KOUKU_SAYDON_PATTERN_AUDITION_STATE state) noexcept
{
	switch (state)
	{
	case KOUKU_SAYDON_PATTERN_AUDITION_STATE::IDLE: return "IDLE";
	case KOUKU_SAYDON_PATTERN_AUDITION_STATE::REQUEST_PENDING:
		return "REQUEST_PENDING";
	case KOUKU_SAYDON_PATTERN_AUDITION_STATE::QUEUED: return "QUEUED";
	case KOUKU_SAYDON_PATTERN_AUDITION_STATE::ACTIVE: return "ACTIVE";
	case KOUKU_SAYDON_PATTERN_AUDITION_STATE::COMPLETED: return "COMPLETED";
	case KOUKU_SAYDON_PATTERN_AUDITION_STATE::REJECTED: return "REJECTED";
	case KOUKU_SAYDON_PATTERN_AUDITION_STATE::ABORTED: return "ABORTED";
	case KOUKU_SAYDON_PATTERN_AUDITION_STATE::END:
	default: return "INVALID";
	}
}

bool Client::CKoukuSaydonPatternAuditionService::Play_Selected(
	const std::string_view patternId,
	const LostArk::Shared::GameplayDataRevision& expectedGameplayRevision,
	const std::uint32_t expectedSourceRevision,
	std::string& outStatus)
{
	return Submit(
		LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED,
		patternId, expectedGameplayRevision, expectedSourceRevision, outStatus);
}

bool Client::CKoukuSaydonPatternAuditionService::Play_All(
	const LostArk::Shared::GameplayDataRevision& expectedGameplayRevision,
	const std::uint32_t expectedSourceRevision,
	std::string& outStatus)
{
	return Submit(
		LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_ALL,
		{}, expectedGameplayRevision, expectedSourceRevision, outStatus);
}

bool Client::CKoukuSaydonPatternAuditionService::Submit(
	const LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_OPERATION operation,
	const std::string_view patternId,
	const LostArk::Shared::GameplayDataRevision& expectedGameplayRevision,
	const std::uint32_t expectedSourceRevision,
	std::string& outStatus)
{
	using namespace LostArk::Shared;
	Update();
	if (KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED != operation &&
		KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_ALL != operation)
	{
		outStatus = "Unsupported KoukuSaydon audition operation.";
		return false;
	}
	if ((KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED == operation) ==
		patternId.empty())
	{
		outStatus = "Play Selected requires one stable pattern ID; Play All must not invent one.";
		return false;
	}
	if (!expectedGameplayRevision.Is_Valid())
	{
		outStatus = "Server Play requires the exact Server-active gameplay revision.";
		return false;
	}
	if (0u == expectedSourceRevision)
	{
		outStatus =
			"Server Play requires the exact saved Product source revision.";
		return false;
	}
	if (m_Snapshot.Is_InFlight())
	{
		outStatus = "A KoukuSaydon Server audition already owns playback.";
		return false;
	}
	CNetworkManager& network = CNetworkManager::Get();
	if (!network.Is_Connected())
	{
		outStatus = "Start and connect the Debug Server first.";
		return false;
	}
	if (0u == m_iNextRequestSequence)
	{
		outStatus = "KoukuSaydon audition request identities are exhausted; restart the Client session.";
		return false;
	}

	C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST request{};
	request.iRequestSequence = m_iNextRequestSequence;
	request.eOperation = operation;
	request.Scope = Make_Scope(expectedGameplayRevision, expectedSourceRevision);
	request.strPatternId = patternId;
	if (!network.Send_KoukuSaydonPatternAudition(request))
	{
		outStatus = "Could not send KoukuSaydon Server Play.";
		return false;
	}

	Advance_RequestSequence();
	m_Snapshot = {};
	m_Snapshot.eState = KOUKU_SAYDON_PATTERN_AUDITION_STATE::REQUEST_PENDING;
	m_Snapshot.eOperation = operation;
	m_Snapshot.iRequestSequence = request.iRequestSequence;
	m_Snapshot.iWorldInboundGeneration = network.Get_WorldInboundGeneration();
	m_Snapshot.ExpectedGameplayRevision = expectedGameplayRevision;
	m_Snapshot.iExpectedSourceRevision = expectedSourceRevision;
	m_Snapshot.strRequestedPatternId = patternId;
	m_Snapshot.strStatus =
		KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_ALL == operation ?
			"Waiting for the Server to admit the saved Play All order." :
			"Waiting for the Server to admit " + std::string(patternId) + ".";
	m_iStateStartedAtMilliseconds = Now_Milliseconds();
	outStatus = m_Snapshot.strStatus;
	return true;
}

void Client::CKoukuSaydonPatternAuditionService::Update()
{
	using namespace LostArk::Shared;
	CNetworkManager& network = CNetworkManager::Get();
	if (m_Snapshot.Is_InFlight() &&
		(!network.Is_Connected() ||
		 m_Snapshot.iWorldInboundGeneration != network.Get_WorldInboundGeneration()))
	{
		Set_Terminal(
			KOUKU_SAYDON_PATTERN_AUDITION_STATE::ABORTED,
			"KoukuSaydon Server Play aborted because the connected world session changed.");
	}

	S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result{};
	while (network.Try_Consume_KoukuSaydonPatternAuditionResult(result))
		Apply_Result(result);
	S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE lifecycle{};
	while (network.Try_Consume_KoukuSaydonPatternAuditionLifecycle(lifecycle))
		Apply_Lifecycle(lifecycle);

	if (!m_Snapshot.Is_InFlight())
		return;
	const std::uint64_t now = Now_Milliseconds();
	const std::uint64_t timeout =
		KOUKU_SAYDON_PATTERN_AUDITION_STATE::REQUEST_PENDING == m_Snapshot.eState ?
			VERDICT_TIMEOUT_MILLISECONDS : QUEUED_START_TIMEOUT_MILLISECONDS;
	if (now >= m_iStateStartedAtMilliseconds &&
		now - m_iStateStartedAtMilliseconds > timeout &&
		KOUKU_SAYDON_PATTERN_AUDITION_STATE::ACTIVE != m_Snapshot.eState)
	{
		Set_Terminal(
			KOUKU_SAYDON_PATTERN_AUDITION_STATE::ABORTED,
			"KoukuSaydon Server Play timed out before an exact ACTIVE lifecycle arrived.");
	}
}

void Client::CKoukuSaydonPatternAuditionService::Apply_Result(
	const LostArk::Shared::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT& result)
{
	using namespace LostArk::Shared;
	if (!m_Snapshot.Is_InFlight() ||
		result.iRequestSequence != m_Snapshot.iRequestSequence ||
		result.eOperation != m_Snapshot.eOperation ||
		result.strRequestedPatternId != m_Snapshot.strRequestedPatternId ||
		!Exact_Scope(result.Scope, Make_Scope(
			m_Snapshot.ExpectedGameplayRevision,
			m_Snapshot.iExpectedSourceRevision)))
	{
		return;
	}
	if (KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED != result.eResult &&
		KOUKUSAYDON_PATTERN_AUDITION_RESULT::DUPLICATE_IGNORED != result.eResult)
	{
		Set_Terminal(
			KOUKU_SAYDON_PATTERN_AUDITION_STATE::REJECTED,
			result.strReason.empty() ? Describe_Rejection(result.eResult) :
				result.strReason);
		return;
	}
	if (!result.PinnedGameplayRevision.Is_Valid() ||
		result.PinnedGameplayRevision != m_Snapshot.ExpectedGameplayRevision ||
		0u == result.iPinnedSourceRevision ||
		result.iPinnedSourceRevision != m_Snapshot.iExpectedSourceRevision ||
		INVALID_NET_ENTITY_ID == result.iBossNetEntityId ||
		0u == result.iRoomAuditionEpoch)
	{
		Set_Terminal(
			KOUKU_SAYDON_PATTERN_AUDITION_STATE::ABORTED,
			"The Server audition receipt did not preserve the exact boss, gameplay, and Product source revision tuple.");
		return;
	}
	m_Snapshot.eState = KOUKU_SAYDON_PATTERN_AUDITION_STATE::QUEUED;
	m_Snapshot.iRoomAuditionEpoch = result.iRoomAuditionEpoch;
	m_Snapshot.iBossNetEntityId = result.iBossNetEntityId;
	m_Snapshot.iPatternSequence = result.iPatternSequence;
	m_Snapshot.iStageIndex = result.iStageIndex;
	m_Snapshot.PinnedGameplayRevision = result.PinnedGameplayRevision;
	m_Snapshot.iPinnedSourceRevision = result.iPinnedSourceRevision;
	m_Snapshot.strStatus =
		KOUKUSAYDON_PATTERN_AUDITION_RESULT::DUPLICATE_IGNORED == result.eResult ?
			"The Server recognized the existing audition request; waiting for its lifecycle." :
			"The Server queued the KoukuSaydon audition.";
	m_iStateStartedAtMilliseconds = Now_Milliseconds();
}

void Client::CKoukuSaydonPatternAuditionService::Apply_Lifecycle(
	const LostArk::Shared::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE& lifecycle)
{
	using namespace LostArk::Shared;
	if (!m_Snapshot.Is_InFlight() ||
		lifecycle.iRequestSequence != m_Snapshot.iRequestSequence ||
		lifecycle.eOperation != m_Snapshot.eOperation ||
		!Exact_Scope(
			lifecycle.Scope, Make_Scope(
				m_Snapshot.ExpectedGameplayRevision,
				m_Snapshot.iExpectedSourceRevision)) ||
		(0u != m_Snapshot.iRoomAuditionEpoch &&
		 lifecycle.iRoomAuditionEpoch != m_Snapshot.iRoomAuditionEpoch) ||
		(INVALID_NET_ENTITY_ID != m_Snapshot.iBossNetEntityId &&
		 lifecycle.iBossNetEntityId != m_Snapshot.iBossNetEntityId) ||
		!lifecycle.PinnedGameplayRevision.Is_Valid() ||
		lifecycle.PinnedGameplayRevision != m_Snapshot.ExpectedGameplayRevision ||
		0u == lifecycle.iPinnedSourceRevision ||
		lifecycle.iPinnedSourceRevision != m_Snapshot.iExpectedSourceRevision)
	{
		return;
	}
	m_Snapshot.iRoomAuditionEpoch = lifecycle.iRoomAuditionEpoch;
	m_Snapshot.iBossNetEntityId = lifecycle.iBossNetEntityId;
	m_Snapshot.iPatternSequence = lifecycle.iPatternSequence;
	m_Snapshot.iStageIndex = lifecycle.iStageIndex;
	m_Snapshot.PinnedGameplayRevision = lifecycle.PinnedGameplayRevision;
	m_Snapshot.iPinnedSourceRevision = lifecycle.iPinnedSourceRevision;

	switch (lifecycle.eState)
	{
	case KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::PENDING:
		m_Snapshot.eState = KOUKU_SAYDON_PATTERN_AUDITION_STATE::QUEUED;
		m_Snapshot.strLivePatternId.clear();
		m_Snapshot.strStatus = "The Server accepted the audition and is waiting to start.";
		m_iStateStartedAtMilliseconds = Now_Milliseconds();
		break;
	case KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ACTIVE:
		if (lifecycle.strPatternId.empty() ||
			(KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED ==
				m_Snapshot.eOperation &&
			 lifecycle.strPatternId != m_Snapshot.strRequestedPatternId))
		{
			Set_Terminal(
				KOUKU_SAYDON_PATTERN_AUDITION_STATE::ABORTED,
				"The Server ACTIVE lifecycle named a different pattern identity.");
			break;
		}
		m_Snapshot.eState = KOUKU_SAYDON_PATTERN_AUDITION_STATE::ACTIVE;
		m_Snapshot.strLivePatternId = lifecycle.strPatternId;
		m_Snapshot.strStatus = "[Live] " + lifecycle.strPatternId +
			" / stage " + std::to_string(lifecycle.iStageIndex + 1u) + ".";
		m_iStateStartedAtMilliseconds = Now_Milliseconds();
		break;
	case KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::PATTERN_COMPLETED:
		m_Snapshot.strLivePatternId.clear();
		m_Snapshot.eState = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_ALL ==
			m_Snapshot.eOperation ? KOUKU_SAYDON_PATTERN_AUDITION_STATE::QUEUED :
			KOUKU_SAYDON_PATTERN_AUDITION_STATE::COMPLETED;
		m_Snapshot.strStatus = "Server pattern completed: " +
			lifecycle.strPatternId + ".";
		m_iStateStartedAtMilliseconds = Now_Milliseconds();
		break;
	case KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::COMPLETED:
		Set_Terminal(
			KOUKU_SAYDON_PATTERN_AUDITION_STATE::COMPLETED,
			"KoukuSaydon Server Play completed.");
		break;
	case KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ABORTED:
		Set_Terminal(
			KOUKU_SAYDON_PATTERN_AUDITION_STATE::ABORTED,
			lifecycle.strReason.empty() ?
				"The Server aborted KoukuSaydon playback." : lifecycle.strReason);
		break;
	case KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::END:
	default:
		Set_Terminal(
			KOUKU_SAYDON_PATTERN_AUDITION_STATE::ABORTED,
			"The Server returned an invalid KoukuSaydon lifecycle state.");
		break;
	}
}

void Client::CKoukuSaydonPatternAuditionService::Set_Terminal(
	const KOUKU_SAYDON_PATTERN_AUDITION_STATE state,
	std::string status)
{
	m_Snapshot.eState = state;
	m_Snapshot.strLivePatternId.clear();
	m_Snapshot.strStatus = std::move(status);
	m_iStateStartedAtMilliseconds = Now_Milliseconds();
}

void Client::CKoukuSaydonPatternAuditionService::Reset(
	const std::string_view reason)
{
	m_Snapshot = {};
	if (!reason.empty())
		m_Snapshot.strStatus = reason;
	m_iStateStartedAtMilliseconds = 0u;
}

void Client::CKoukuSaydonPatternAuditionService::Advance_RequestSequence()
{
	m_iNextRequestSequence =
		(std::numeric_limits<std::uint32_t>::max)() == m_iNextRequestSequence ?
			0u : m_iNextRequestSequence + 1u;
}

std::uint64_t
Client::CKoukuSaydonPatternAuditionService::Now_Milliseconds() const noexcept
{
	return static_cast<std::uint64_t>(::GetTickCount64());
}

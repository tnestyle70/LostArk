#include "Effect_ProductPrewarmQueue.h"

#include <algorithm>
#include <exception>
#include <utility>

bool Client::EFFECT_PRODUCT_PREWARM_FAILURE_RECEIPT::Is_Valid() const
{
	return 0u != iCatalogRevision && !strEffectAssetId.empty() &&
		0 != iRootCode && !strRootMessage.empty();
}

Client::EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN::
EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN(
	EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN&& Other) noexcept
{
	*this = std::move(Other);
}

Client::EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN::
~EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN() noexcept
{
	if (Is_Valid())
		std::terminate();
}

Client::EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN&
Client::EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN::operator=(
	EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN&& Other) noexcept
{
	if (this == &Other)
		return *this;
	if (Is_Valid())
		std::terminate();
	m_pOwner = Other.m_pOwner;
	m_iReservationNonce = Other.m_iReservationNonce;
	m_iCatalogRevision = Other.m_iCatalogRevision;
	m_iJobEpoch = Other.m_iJobEpoch;
	m_bPrepared = Other.m_bPrepared;
	Other.Invalidate();
	return *this;
}

bool Client::EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN::Is_Valid() const
{
	return nullptr != m_pOwner && 0u != m_iReservationNonce &&
		0u != m_iCatalogRevision;
}

uint64_t Client::EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN::
Get_CatalogRevision() const
{
	return m_iCatalogRevision;
}

uint64_t Client::EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN::Get_JobEpoch() const
{
	return m_iJobEpoch;
}

bool Client::EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN::
Is_PreparedOutcome() const
{
	return m_bPrepared;
}

void Client::EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN::Invalidate() noexcept
{
	m_pOwner = nullptr;
	m_iReservationNonce = 0u;
	m_iCatalogRevision = 0u;
	m_iJobEpoch = 0u;
	m_bPrepared = false;
}

bool Client::Is_ProductPrewarmTargetActivationReady(
	const EFFECT_PRODUCT_PREWARM_TARGET_PROBE& Probe,
	const bool bTargetRegistrationFailureIsolated)
{
	return Probe.bCatalogRevisionCurrent &&
		(Probe.bSettled || bTargetRegistrationFailureIsolated);
}

void Client::CEffectProductPrewarmQueue::Reset_ForCatalogRevision(
	const uint64_t iCatalogRevision)
{
	if (m_iCatalogRevision == iCatalogRevision)
		return;
	if (m_FrontReservation.has_value())
		std::terminate();
	Clear();
	m_iCatalogRevision = iCatalogRevision;
}

bool Client::CEffectProductPrewarmQueue::Enqueue(
	const std::vector<std::string>& EffectAssetIds,
	std::string& strOutStatus)
{
	if (m_FrontReservation.has_value())
	{
		strOutStatus =
			"Effect Product prewarm queue front is reserved for commit.";
		return false;
	}
	if (0u == m_iCatalogRevision || std::any_of(
			EffectAssetIds.begin(), EffectAssetIds.end(),
			[](const std::string& EffectAssetId)
			{
				return EffectAssetId.empty();
			}))
	{
		strOutStatus =
			"Effect Product prewarm queue revision or target is invalid.";
		return false;
	}

	uint32_t iQueuedCount = 0u;
	for (const std::string& EffectAssetId : EffectAssetIds)
	{
		m_Targets.insert(EffectAssetId);
		if (m_Records.contains(EffectAssetId))
			continue;
		m_Records.emplace(EffectAssetId, TARGET_RECORD{});
		m_Pending.push_back(EffectAssetId);
		++iQueuedCount;
	}
	if (0u != iQueuedCount)
		m_bYieldNextFrame = true;
	strOutStatus = "Queued " + std::to_string(iQueuedCount) +
		" new Product Effect targets for incremental prewarm.";
	return true;
}

bool Client::CEffectProductPrewarmQueue::Enqueue_Priority(
	const std::vector<std::string>& EffectAssetIds,
	std::string& strOutStatus)
{
	if (m_FrontReservation.has_value())
	{
		strOutStatus =
			"Effect Product prewarm queue front is reserved for commit.";
		return false;
	}
	if (0u == m_iCatalogRevision || std::any_of(
			EffectAssetIds.begin(), EffectAssetIds.end(),
			[](const std::string& EffectAssetId)
			{
				return EffectAssetId.empty();
			}))
	{
		strOutStatus =
			"Effect Product priority prewarm queue revision or target is invalid.";
		return false;
	}

	std::vector<std::string> PriorityPending;
	PriorityPending.reserve(EffectAssetIds.size());
	std::set<std::string, std::less<>> PriorityIds;
	uint32_t iQueuedCount = 0u;
	for (const std::string& EffectAssetId : EffectAssetIds)
	{
		m_Targets.insert(EffectAssetId);
		auto Record = m_Records.find(EffectAssetId);
		if (m_Records.end() == Record)
		{
			Record = m_Records.emplace(EffectAssetId, TARGET_RECORD{}).first;
			m_Pending.push_back(EffectAssetId);
			++iQueuedCount;
		}
		if (TARGET_STATE::PENDING != Record->second.eState ||
			!PriorityIds.insert(EffectAssetId).second)
		{
			continue;
		}
		PriorityPending.push_back(EffectAssetId);
	}

	/* Loading owns an immutable front block.  Later normal priority
	   registration must not move an unowned target ahead of that block or the
	   Loading consumer could no longer reach its reserved FIFO front. */
	std::deque<std::string> Reordered;
	for (const std::string& EffectAssetId : m_Pending)
	{
		const auto Record = m_Records.find(EffectAssetId);
		if (m_Records.end() != Record &&
			0u != Record->second.iLoadingOwnerEpoch)
		{
			Reordered.push_back(EffectAssetId);
		}
	}
	for (const std::string& EffectAssetId : PriorityPending)
	{
		const auto Record = m_Records.find(EffectAssetId);
		if (m_Records.end() != Record &&
			0u == Record->second.iLoadingOwnerEpoch)
		{
			Reordered.push_back(EffectAssetId);
		}
	}
	for (const std::string& EffectAssetId : m_Pending)
	{
		const auto Record = m_Records.find(EffectAssetId);
		if (m_Records.end() != Record &&
			0u == Record->second.iLoadingOwnerEpoch &&
			!PriorityIds.contains(EffectAssetId))
		{
			Reordered.push_back(EffectAssetId);
		}
	}
	m_Pending = std::move(Reordered);

	if (0u != iQueuedCount)
		m_bYieldNextFrame = true;
	strOutStatus = "Queued " + std::to_string(iQueuedCount) +
		" new and prioritized " + std::to_string(PriorityPending.size()) +
		" pending Product Effect targets.";
	return true;
}

Client::EFFECT_PRODUCT_PREWARM_STEP_RESULT
Client::CEffectProductPrewarmQueue::Begin_Frame(
	std::string& strOutEffectAssetId)
{
	strOutEffectAssetId.clear();
	if (m_Pending.empty() || m_FrontReservation.has_value())
		return EFFECT_PRODUCT_PREWARM_STEP_RESULT::IDLE;
	if (m_bYieldNextFrame)
	{
		m_bYieldNextFrame = false;
		return EFFECT_PRODUCT_PREWARM_STEP_RESULT::YIELDED;
	}
	const auto Record = m_Records.find(m_Pending.front());
	if (m_Records.end() == Record ||
		TARGET_STATE::PENDING != Record->second.eState ||
		0u != Record->second.iLoadingOwnerEpoch)
	{
		return EFFECT_PRODUCT_PREWARM_STEP_RESULT::IDLE;
	}
	strOutEffectAssetId = m_Pending.front();
	return EFFECT_PRODUCT_PREWARM_STEP_RESULT::READY;
}

Client::EFFECT_PRODUCT_PREWARM_STEP_RESULT
Client::CEffectProductPrewarmQueue::Begin_LoadingFrame(
	const uint64_t iJobEpoch,
	std::string& strOutEffectAssetId)
{
	strOutEffectAssetId.clear();
	if (0u == iJobEpoch || m_Pending.empty() ||
		m_FrontReservation.has_value())
	{
		return EFFECT_PRODUCT_PREWARM_STEP_RESULT::IDLE;
	}
	if (m_bYieldNextFrame)
	{
		m_bYieldNextFrame = false;
		return EFFECT_PRODUCT_PREWARM_STEP_RESULT::YIELDED;
	}
	const auto Record = m_Records.find(m_Pending.front());
	if (m_Records.end() == Record ||
		TARGET_STATE::PENDING != Record->second.eState ||
		Record->second.iLoadingOwnerEpoch != iJobEpoch)
	{
		return EFFECT_PRODUCT_PREWARM_STEP_RESULT::IDLE;
	}
	strOutEffectAssetId = m_Pending.front();
	return EFFECT_PRODUCT_PREWARM_STEP_RESULT::READY;
}

bool Client::CEffectProductPrewarmQueue::Claim_LoadingTargets(
	const uint64_t iJobEpoch,
	const uint64_t iExpectedCatalogRevision,
	const std::vector<std::string>& EffectAssetIds,
	std::string& strOutStatus)
{
	if (m_FrontReservation.has_value())
	{
		strOutStatus =
			"Effect Product queue front is reserved during Loading claim.";
		return false;
	}
	if (0u == iJobEpoch || 0u == iExpectedCatalogRevision ||
		m_iCatalogRevision != iExpectedCatalogRevision || std::any_of(
			EffectAssetIds.begin(), EffectAssetIds.end(),
			[](const std::string& EffectAssetId)
			{
				return EffectAssetId.empty();
			}))
	{
		strOutStatus =
			"Effect Product Loading owner claim identity is invalid.";
		return false;
	}
	if (!m_LoadingOwnerTargets.empty() &&
		!m_LoadingOwnerTargets.contains(iJobEpoch))
	{
		strOutStatus =
			"Another Effect Product Loading epoch still owns targets.";
		return false;
	}

	std::set<std::string, std::less<>> ClaimedTargets;
	std::vector<std::string> OrderedClaimedTargets;
	OrderedClaimedTargets.reserve(EffectAssetIds.size());
	for (const std::string& EffectAssetId : EffectAssetIds)
	{
		const auto Record = m_Records.find(EffectAssetId);
		if (m_Records.end() == Record ||
			TARGET_STATE::PENDING != Record->second.eState)
		{
			continue;
		}
		if (0u != Record->second.iLoadingOwnerEpoch &&
			Record->second.iLoadingOwnerEpoch != iJobEpoch)
		{
			strOutStatus =
				"Effect Product target is owned by another Loading epoch: " +
				EffectAssetId;
			return false;
		}
		if (ClaimedTargets.insert(EffectAssetId).second)
			OrderedClaimedTargets.push_back(EffectAssetId);
	}

	const auto ExistingOwner = m_LoadingOwnerTargets.find(iJobEpoch);
	if (m_LoadingOwnerTargets.end() != ExistingOwner &&
		ExistingOwner->second != ClaimedTargets)
	{
		strOutStatus =
			"Effect Product Loading epoch already owns a different target set.";
		return false;
	}

	std::deque<std::string> Reordered;
	for (const std::string& EffectAssetId : OrderedClaimedTargets)
		Reordered.push_back(EffectAssetId);
	for (const std::string& EffectAssetId : m_Pending)
	{
		if (!ClaimedTargets.contains(EffectAssetId))
			Reordered.push_back(EffectAssetId);
	}

	if (!ClaimedTargets.empty() &&
		m_LoadingOwnerTargets.end() == ExistingOwner)
	{
		m_LoadingOwnerTargets.emplace(iJobEpoch, ClaimedTargets);
	}
	for (const std::string& EffectAssetId : ClaimedTargets)
		m_Records.find(EffectAssetId)->second.iLoadingOwnerEpoch = iJobEpoch;
	m_Pending = std::move(Reordered);
	strOutStatus = "Claimed " + std::to_string(ClaimedTargets.size()) +
		" pending Product Effect targets for Loading epoch " +
		std::to_string(iJobEpoch) + ".";
	return true;
}

bool Client::CEffectProductPrewarmQueue::Release_LoadingOwner(
	const uint64_t iJobEpoch,
	const EFFECT_PRODUCT_LOADING_OWNER_RELEASE eDisposition,
	std::string& strOutStatus)
{
	if (0u == iJobEpoch ||
		EFFECT_PRODUCT_LOADING_OWNER_RELEASE::END == eDisposition)
	{
		strOutStatus = "Effect Product Loading owner release is invalid.";
		return false;
	}
	if (m_FrontReservation.has_value() &&
		m_FrontReservation->iJobEpoch == iJobEpoch)
	{
		strOutStatus =
			"Effect Product Loading owner cannot be released during front commit.";
		return false;
	}

	const auto Owner = m_LoadingOwnerTargets.find(iJobEpoch);
	if (m_LoadingOwnerTargets.end() == Owner)
	{
		strOutStatus = "Effect Product Loading owner was already released.";
		return true;
	}
	if (EFFECT_PRODUCT_LOADING_OWNER_RELEASE::TERMINAL == eDisposition &&
		!Owner->second.empty())
	{
		strOutStatus =
			"Effect Product Loading owner still has non-terminal targets.";
		return false;
	}
	const size_t iReleasedCount = Owner->second.size();
	for (const std::string& EffectAssetId : Owner->second)
	{
		auto Record = m_Records.find(EffectAssetId);
		if (m_Records.end() != Record &&
			Record->second.iLoadingOwnerEpoch == iJobEpoch)
		{
			Record->second.iLoadingOwnerEpoch = 0u;
		}
	}
	m_LoadingOwnerTargets.erase(Owner);
	strOutStatus = "Released " + std::to_string(iReleasedCount) +
		" Product Effect Loading owner targets.";
	return true;
}

bool Client::CEffectProductPrewarmQueue::Has_LoadingOwner(
	const uint64_t iJobEpoch) const
{
	return 0u != iJobEpoch && m_LoadingOwnerTargets.contains(iJobEpoch);
}

uint64_t Client::CEffectProductPrewarmQueue::Get_LoadingOwnerEpoch(
	const std::string& strEffectAssetId) const
{
	const auto Record = m_Records.find(strEffectAssetId);
	return m_Records.end() == Record ? 0u :
		Record->second.iLoadingOwnerEpoch;
}

bool Client::CEffectProductPrewarmQueue::Prevalidate_Front(
	const std::string& strEffectAssetId,
	const uint64_t iExpectedCatalogRevision,
	const uint64_t iJobEpoch,
	const bool bPrepared,
	const EFFECT_PRODUCT_PREWARM_FAILURE_RECEIPT* pFailure,
	EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN& OutToken,
	std::string& strOutStatus)
{
	if (OutToken.Is_Valid())
	{
		strOutStatus =
			"Effect Product front output token is already reserved.";
		return false;
	}
	if (m_FrontReservation.has_value())
	{
		strOutStatus = "Effect Product queue front already has a reservation.";
		return false;
	}
	if (strEffectAssetId.empty() || 0u == iExpectedCatalogRevision ||
		m_iCatalogRevision != iExpectedCatalogRevision || m_Pending.empty() ||
		m_Pending.front() != strEffectAssetId)
	{
		strOutStatus =
			"Effect Product front prevalidation does not match the FIFO identity.";
		return false;
	}

	auto Record = m_Records.find(strEffectAssetId);
	if (m_Records.end() == Record ||
		TARGET_STATE::PENDING != Record->second.eState ||
		0u != Record->second.iReservationNonce ||
		Record->second.iLoadingOwnerEpoch != iJobEpoch)
	{
		strOutStatus =
			"Effect Product front prevalidation owner or state is invalid.";
		return false;
	}
	if (bPrepared && nullptr != pFailure)
	{
		strOutStatus = "Prepared Product Effect cannot carry a failure receipt.";
		return false;
	}
	if (!bPrepared && (nullptr == pFailure || !pFailure->Is_Valid() ||
		pFailure->iCatalogRevision != iExpectedCatalogRevision ||
		pFailure->iJobEpoch != iJobEpoch ||
		pFailure->strEffectAssetId != strEffectAssetId))
	{
		strOutStatus =
			"Failed Product Effect requires its exact root failure receipt.";
		return false;
	}

	uint64_t iNonce = m_iNextReservationNonce++;
	if (0u == iNonce)
	{
		iNonce = m_iNextReservationNonce++;
		if (0u == iNonce)
			std::terminate();
	}
	Record->second.iReservationNonce = iNonce;
	Record->second.bReservedPrepared = bPrepared;
	if (bPrepared)
		Record->second.Failure.reset();
	else
		Record->second.Failure = *pFailure;
	m_FrontReservation = FRONT_RESERVATION{
		iNonce, iExpectedCatalogRevision, iJobEpoch,
		strEffectAssetId, bPrepared };
	OutToken.m_pOwner = this;
	OutToken.m_iReservationNonce = iNonce;
	OutToken.m_iCatalogRevision = iExpectedCatalogRevision;
	OutToken.m_iJobEpoch = iJobEpoch;
	OutToken.m_bPrepared = bPrepared;
	strOutStatus = "Reserved Product Effect FIFO front for terminal commit.";
	return true;
}

void Client::CEffectProductPrewarmQueue::Commit_PrevalidatedFront(
	EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN&& Token) noexcept
{
	if (!Is_ReservationTokenCurrent(Token))
		std::terminate();
	const FRONT_RESERVATION& Reservation = *m_FrontReservation;
	auto Record = m_Records.find(Reservation.strEffectAssetId);
	if (m_Records.end() == Record || m_Pending.empty() ||
		m_Pending.front() != Reservation.strEffectAssetId ||
		Record->second.iReservationNonce != Reservation.iNonce)
	{
		std::terminate();
	}

	Record->second.eState = Reservation.bPrepared ?
		TARGET_STATE::PREPARED : TARGET_STATE::FAILED;
	Record->second.iReservationNonce = 0u;
	Record->second.bReservedPrepared = false;
	const uint64_t iLoadingOwnerEpoch =
		Record->second.iLoadingOwnerEpoch;
	Record->second.iLoadingOwnerEpoch = 0u;
	m_Pending.pop_front();
	if (0u != iLoadingOwnerEpoch)
	{
		Remove_LoadingOwnerTarget(
			iLoadingOwnerEpoch, Reservation.strEffectAssetId);
	}
	m_FrontReservation.reset();
	Token.Invalidate();
}

bool Client::CEffectProductPrewarmQueue::Abort_PrevalidatedFront(
	EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN&& Token,
	std::string& strOutStatus)
{
	if (!Is_ReservationTokenCurrent(Token))
	{
		strOutStatus = "Effect Product front abort token is stale or invalid.";
		return false;
	}
	auto Record = m_Records.find(m_FrontReservation->strEffectAssetId);
	if (m_Records.end() == Record)
	{
		strOutStatus = "Effect Product reserved front record is absent.";
		return false;
	}
	Record->second.iReservationNonce = 0u;
	Record->second.bReservedPrepared = false;
	Record->second.Failure.reset();
	m_FrontReservation.reset();
	Token.Invalidate();
	strOutStatus = "Aborted Product Effect FIFO front reservation.";
	return true;
}

bool Client::CEffectProductPrewarmQueue::Complete_Front(
	const std::string& strEffectAssetId,
	const bool bPrepared,
	std::string& strOutStatus)
{
	EFFECT_PRODUCT_PREWARM_FAILURE_RECEIPT Failure;
	const EFFECT_PRODUCT_PREWARM_FAILURE_RECEIPT* pFailure = nullptr;
	if (!bPrepared)
	{
		Failure.iCatalogRevision = m_iCatalogRevision;
		Failure.strEffectAssetId = strEffectAssetId;
		Failure.iRootCode = -1;
		Failure.strRootMessage =
			"Legacy prewarm caller reported a failed target without a root receipt.";
		pFailure = &Failure;
	}
	EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN Token;
	if (!Prevalidate_Front(
			strEffectAssetId, m_iCatalogRevision, 0u, bPrepared,
			pFailure, Token, strOutStatus))
	{
		return false;
	}
	Commit_PrevalidatedFront(std::move(Token));
	strOutStatus = bPrepared ?
		"Product Effect target committed as prepared." :
		"Product Effect target failed closed for this catalog revision.";
	return true;
}

bool Client::CEffectProductPrewarmQueue::Commit_AllPrepared(
	const std::set<std::string, std::less<>>& EffectAssetIds,
	std::string& strOutStatus)
{
	if (m_FrontReservation.has_value() || 0u == m_iCatalogRevision ||
		EffectAssetIds.empty() || std::any_of(
			EffectAssetIds.begin(), EffectAssetIds.end(),
			[](const std::string& EffectAssetId)
			{
				return EffectAssetId.empty();
			}))
	{
		strOutStatus =
			"Effect Product batch-prepared target set is invalid.";
		return false;
	}
	std::map<std::string, TARGET_RECORD, std::less<>> PreparedRecords;
	for (const std::string& EffectAssetId : EffectAssetIds)
	{
		TARGET_RECORD Record;
		Record.eState = TARGET_STATE::PREPARED;
		PreparedRecords.emplace(EffectAssetId, std::move(Record));
	}
	m_Targets = EffectAssetIds;
	m_Records = std::move(PreparedRecords);
	m_Pending.clear();
	m_LoadingOwnerTargets.clear();
	m_bYieldNextFrame = false;
	strOutStatus = "Committed " + std::to_string(EffectAssetIds.size()) +
		" Product Effect targets as batch prepared.";
	return true;
}

bool Client::CEffectProductPrewarmQueue::Commit_HotReloadPrepared(
	const std::string& strEffectAssetId,
	std::string& strOutStatus)
{
	if (m_FrontReservation.has_value() ||
		0u == m_iCatalogRevision || strEffectAssetId.empty())
	{
		strOutStatus =
			"Effect Product hot-reload prepared target is invalid.";
		return false;
	}

	const auto ExistingRecord = m_Records.find(strEffectAssetId);
	if (m_Records.end() != ExistingRecord &&
		0u != ExistingRecord->second.iLoadingOwnerEpoch)
	{
		strOutStatus =
			"Effect Product hot reload is deferred while Loading owns the target.";
		return false;
	}

	m_Targets.insert(strEffectAssetId);
	m_Pending.erase(std::remove(
		m_Pending.begin(), m_Pending.end(), strEffectAssetId), m_Pending.end());
	auto Record =
		m_Records.try_emplace(strEffectAssetId, TARGET_RECORD{}).first;
	const uint64_t iLoadingOwnerEpoch =
		Record->second.iLoadingOwnerEpoch;
	Record->second = {};
	Record->second.eState = TARGET_STATE::PREPARED;
	if (0u != iLoadingOwnerEpoch)
		Remove_LoadingOwnerTarget(iLoadingOwnerEpoch, strEffectAssetId);
	strOutStatus =
		"Committed selected Product Effect target as hot-reload prepared.";
	return true;
}

bool Client::CEffectProductPrewarmQueue::Is_Prepared(
	const std::string& strEffectAssetId) const
{
	const auto Record = m_Records.find(strEffectAssetId);
	return m_Records.end() != Record &&
		TARGET_STATE::PREPARED == Record->second.eState;
}

const Client::EFFECT_PRODUCT_PREWARM_FAILURE_RECEIPT*
Client::CEffectProductPrewarmQueue::Find_FailureReceipt(
	const std::string& strEffectAssetId) const
{
	const auto Record = m_Records.find(strEffectAssetId);
	if (m_Records.end() == Record ||
		TARGET_STATE::FAILED != Record->second.eState ||
		!Record->second.Failure.has_value())
	{
		return nullptr;
	}
	return &*Record->second.Failure;
}

uint64_t Client::CEffectProductPrewarmQueue::Get_CatalogRevision() const
{
	return m_iCatalogRevision;
}

const std::set<std::string, std::less<>>&
Client::CEffectProductPrewarmQueue::Get_Targets() const
{
	return m_Targets;
}

Client::EFFECT_PRODUCT_PREWARM_QUEUE_PROBE
Client::CEffectProductPrewarmQueue::Get_Probe() const
{
	EFFECT_PRODUCT_PREWARM_QUEUE_PROBE Probe;
	Probe.iCatalogRevision = m_iCatalogRevision;
	Probe.iTargetCount = static_cast<uint32_t>(m_Targets.size());
	for (const auto& [EffectAssetId, Record] : m_Records)
	{
		if (TARGET_STATE::PENDING == Record.eState)
			++Probe.iPendingCount;
		else if (TARGET_STATE::PREPARED == Record.eState)
			++Probe.iPreparedCount;
		else if (TARGET_STATE::FAILED == Record.eState)
			++Probe.iFailedCount;
	}
	Probe.bYieldNextFrame = m_bYieldNextFrame;
	return Probe;
}

Client::EFFECT_PRODUCT_PREWARM_TARGET_PROBE
Client::CEffectProductPrewarmQueue::Get_TargetProbe(
	const std::vector<std::string>& EffectAssetIds,
	const uint64_t iExpectedCatalogRevision) const
{
	EFFECT_PRODUCT_PREWARM_TARGET_PROBE Probe;
	Probe.iCatalogRevision = m_iCatalogRevision;
	Probe.iQueuePendingCount =
		static_cast<uint32_t>(m_Pending.size());
	Probe.bCatalogRevisionCurrent = 0u != iExpectedCatalogRevision &&
		m_iCatalogRevision == iExpectedCatalogRevision;

	std::set<std::string, std::less<>> UniqueTargets;
	for (const std::string& EffectAssetId : EffectAssetIds)
	{
		if (!UniqueTargets.insert(EffectAssetId).second)
			continue;
		++Probe.iTargetCount;
		const auto Record = m_Records.find(EffectAssetId);
		if (m_Records.end() == Record)
			++Probe.iUnavailableCount;
		else if (TARGET_STATE::PENDING == Record->second.eState)
			++Probe.iPendingCount;
		else if (TARGET_STATE::PREPARED == Record->second.eState)
			++Probe.iPreparedCount;
		else if (TARGET_STATE::FAILED == Record->second.eState)
			++Probe.iFailedCount;
	}

	/* Unavailable targets are terminal presentation failures for the current
	   revision. They remain visible but do not hold a Level transition. */
	Probe.bSettled = Probe.bCatalogRevisionCurrent &&
		0u == Probe.iPendingCount;
	return Probe;
}

bool Client::CEffectProductPrewarmQueue::Is_ReservationTokenCurrent(
	const EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN& Token) const
{
	return Token.Is_Valid() && Token.m_pOwner == this &&
		m_FrontReservation.has_value() &&
		Token.m_iReservationNonce == m_FrontReservation->iNonce &&
		Token.m_iCatalogRevision == m_FrontReservation->iCatalogRevision &&
		Token.m_iJobEpoch == m_FrontReservation->iJobEpoch &&
		Token.m_bPrepared == m_FrontReservation->bPrepared;
}

void Client::CEffectProductPrewarmQueue::Remove_LoadingOwnerTarget(
	const uint64_t iJobEpoch,
	const std::string& strEffectAssetId) noexcept
{
	const auto Owner = m_LoadingOwnerTargets.find(iJobEpoch);
	if (m_LoadingOwnerTargets.end() == Owner)
		return;
	Owner->second.erase(strEffectAssetId);
	if (Owner->second.empty())
		m_LoadingOwnerTargets.erase(Owner);
}

void Client::CEffectProductPrewarmQueue::Clear()
{
	if (m_FrontReservation.has_value())
		std::terminate();
	m_iCatalogRevision = 0u;
	m_Pending.clear();
	m_Targets.clear();
	m_Records.clear();
	m_LoadingOwnerTargets.clear();
	m_iNextReservationNonce = 1u;
	m_bYieldNextFrame = false;
}

#include "Effect_ProductPrewarmQueue.h"

#include <algorithm>

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
	Clear();
	m_iCatalogRevision = iCatalogRevision;
}

bool Client::CEffectProductPrewarmQueue::Enqueue(
	const std::vector<std::string>& EffectAssetIds,
	std::string& strOutStatus)
{
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
		if (m_PreparedIds.contains(EffectAssetId) ||
			m_FailedIds.contains(EffectAssetId) ||
			!m_PendingIds.insert(EffectAssetId).second)
		{
			continue;
		}
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
		if (m_PreparedIds.contains(EffectAssetId) ||
			m_FailedIds.contains(EffectAssetId) ||
			!PriorityIds.insert(EffectAssetId).second)
		{
			continue;
		}
		if (m_PendingIds.insert(EffectAssetId).second)
			++iQueuedCount;
		PriorityPending.push_back(EffectAssetId);
	}

	std::deque<std::string> Reordered;
	for (const std::string& EffectAssetId : PriorityPending)
		Reordered.push_back(EffectAssetId);
	for (const std::string& EffectAssetId : m_Pending)
	{
		if (!PriorityIds.contains(EffectAssetId))
			Reordered.push_back(EffectAssetId);
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
	if (m_Pending.empty())
		return EFFECT_PRODUCT_PREWARM_STEP_RESULT::IDLE;
	if (m_bYieldNextFrame)
	{
		m_bYieldNextFrame = false;
		return EFFECT_PRODUCT_PREWARM_STEP_RESULT::YIELDED;
	}
	strOutEffectAssetId = m_Pending.front();
	return EFFECT_PRODUCT_PREWARM_STEP_RESULT::READY;
}

bool Client::CEffectProductPrewarmQueue::Complete_Front(
	const std::string& strEffectAssetId,
	const bool bPrepared,
	std::string& strOutStatus)
{
	if (strEffectAssetId.empty() || m_Pending.empty() ||
		m_Pending.front() != strEffectAssetId ||
		!m_PendingIds.contains(strEffectAssetId))
	{
		strOutStatus =
			"Effect Product prewarm completion does not match the FIFO front.";
		return false;
	}
	m_Pending.pop_front();
	m_PendingIds.erase(strEffectAssetId);
	if (bPrepared)
		m_PreparedIds.insert(strEffectAssetId);
	else
		m_FailedIds.insert(strEffectAssetId);
	strOutStatus = bPrepared ?
		"Product Effect target committed as prepared." :
		"Product Effect target failed closed for this catalog revision.";
	return true;
}

bool Client::CEffectProductPrewarmQueue::Commit_AllPrepared(
	const std::set<std::string, std::less<>>& EffectAssetIds,
	std::string& strOutStatus)
{
	if (0u == m_iCatalogRevision || EffectAssetIds.empty() || std::any_of(
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
	m_Targets = EffectAssetIds;
	m_PreparedIds = EffectAssetIds;
	m_Pending.clear();
	m_PendingIds.clear();
	m_FailedIds.clear();
	m_bYieldNextFrame = false;
	strOutStatus = "Committed " + std::to_string(EffectAssetIds.size()) +
		" Product Effect targets as batch prepared.";
	return true;
}

bool Client::CEffectProductPrewarmQueue::Is_Prepared(
	const std::string& strEffectAssetId) const
{
	return m_PreparedIds.contains(strEffectAssetId);
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
	Probe.iPendingCount = static_cast<uint32_t>(m_Pending.size());
	Probe.iPreparedCount = static_cast<uint32_t>(m_PreparedIds.size());
	Probe.iFailedCount = static_cast<uint32_t>(m_FailedIds.size());
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
	Probe.iQueuePendingCount = static_cast<uint32_t>(m_Pending.size());
	Probe.bCatalogRevisionCurrent = 0u != iExpectedCatalogRevision &&
		m_iCatalogRevision == iExpectedCatalogRevision;

	std::set<std::string, std::less<>> UniqueTargets;
	for (const std::string& EffectAssetId : EffectAssetIds)
	{
		if (!UniqueTargets.insert(EffectAssetId).second)
			continue;
		++Probe.iTargetCount;
		if (m_PendingIds.contains(EffectAssetId))
			++Probe.iPendingCount;
		else if (m_PreparedIds.contains(EffectAssetId))
			++Probe.iPreparedCount;
		else if (m_FailedIds.contains(EffectAssetId))
			++Probe.iFailedCount;
		else
			++Probe.iUnavailableCount;
	}

	/* Unavailable targets are terminal presentation failures for the current
	   revision. They must be visible in the probe, but must not hold a Level
	   transition forever. bSettled describes only the requested target set;
	   iQueuePendingCount remains diagnostic and includes unrelated background
	   work. A stale queue revision is not terminal because the regular main-frame
	   advance will rebase it before the next probe. */
	Probe.bSettled = Probe.bCatalogRevisionCurrent &&
		0u == Probe.iPendingCount;
	return Probe;
}

void Client::CEffectProductPrewarmQueue::Clear()
{
	m_iCatalogRevision = 0u;
	m_Pending.clear();
	m_Targets.clear();
	m_PendingIds.clear();
	m_PreparedIds.clear();
	m_FailedIds.clear();
	m_bYieldNextFrame = false;
}

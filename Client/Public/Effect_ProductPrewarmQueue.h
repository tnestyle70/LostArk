#pragma once

#include "Client_Defines.h"

#include <cstdint>
#include <deque>
#include <set>
#include <string>
#include <vector>

namespace Client
{

enum class EFFECT_PRODUCT_PREWARM_STEP_RESULT : uint8_t
{
	IDLE,
	YIELDED,
	READY,
	END
};

struct EFFECT_PRODUCT_PREWARM_QUEUE_PROBE final
{
	uint64_t iCatalogRevision = 0u;
	uint32_t iTargetCount = 0u;
	uint32_t iPendingCount = 0u;
	uint32_t iPreparedCount = 0u;
	uint32_t iFailedCount = 0u;
	bool bYieldNextFrame = false;
};

struct EFFECT_PRODUCT_PREWARM_TARGET_PROBE final
{
	uint64_t iCatalogRevision = 0u;
	uint32_t iTargetCount = 0u;
	uint32_t iPendingCount = 0u;
	uint32_t iPreparedCount = 0u;
	uint32_t iFailedCount = 0u;
	uint32_t iUnavailableCount = 0u;
	uint32_t iQueuePendingCount = 0u;
	bool bCatalogRevisionCurrent = false;
	bool bSettled = false;
};

/* Loading and in-level presentation switches share this gate.  An isolated
   target-registration failure may continue, but never across a stale catalog
   revision or while any Product resource work remains queued. */
bool Is_ProductPrewarmActivationReady(
	const EFFECT_PRODUCT_PREWARM_TARGET_PROBE& Probe,
	bool bTargetRegistrationFailureIsolated = false);

/* Main-thread scheduler state only.  Catalog parsing, drawable validation and
   GPU work remain in the presentation/renderer step that consumes READY. */
class CEffectProductPrewarmQueue final
{
public:
	void Reset_ForCatalogRevision(uint64_t iCatalogRevision);
	bool Enqueue(
		const std::vector<std::string>& EffectAssetIds,
		std::string& strOutStatus);
	bool Enqueue_Priority(
		const std::vector<std::string>& EffectAssetIds,
		std::string& strOutStatus);
	EFFECT_PRODUCT_PREWARM_STEP_RESULT Begin_Frame(
		std::string& strOutEffectAssetId);
	bool Complete_Front(
		const std::string& strEffectAssetId,
		bool bPrepared,
		std::string& strOutStatus);
	bool Commit_AllPrepared(
		const std::set<std::string, std::less<>>& EffectAssetIds,
		std::string& strOutStatus);
	bool Is_Prepared(const std::string& strEffectAssetId) const;
	uint64_t Get_CatalogRevision() const;
	const std::set<std::string, std::less<>>& Get_Targets() const;
	EFFECT_PRODUCT_PREWARM_QUEUE_PROBE Get_Probe() const;
	EFFECT_PRODUCT_PREWARM_TARGET_PROBE Get_TargetProbe(
		const std::vector<std::string>& EffectAssetIds,
		uint64_t iExpectedCatalogRevision) const;
	void Clear();

private:
	uint64_t m_iCatalogRevision = 0u;
	std::deque<std::string> m_Pending;
	std::set<std::string, std::less<>> m_Targets;
	std::set<std::string, std::less<>> m_PendingIds;
	std::set<std::string, std::less<>> m_PreparedIds;
	std::set<std::string, std::less<>> m_FailedIds;
	bool m_bYieldNextFrame = false;
};

}

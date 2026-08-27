#pragma once

#include "Client_Defines.h"

#include <cstdint>
#include <deque>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace Client
{

class CEffectProductPrewarmQueue;

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

struct EFFECT_PRODUCT_PREWARM_FAILURE_RECEIPT final
{
	uint64_t iCatalogRevision = 0u;
	uint64_t iJobEpoch = 0u;
	std::string strEffectAssetId;
	int64_t iRootCode = 0;
	std::string strRootMessage;

	bool Is_Valid() const;
};

enum class EFFECT_PRODUCT_LOADING_OWNER_RELEASE : uint8_t
{
	TERMINAL,
	REBASE,
	CANCELLED,
	STRUCTURAL_FAILURE,
	END
};

/* A successful prevalidation reserves the exact FIFO front and all terminal
   queue storage before the caller publishes its prepared renderer record.
   The token is move-only and may be committed exactly once. */
class EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN final
{
public:
	EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN() = default;
	~EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN() noexcept;
	EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN(
		const EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN&) = delete;
	EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN& operator=(
		const EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN&) = delete;
	EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN(
		EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN&& Other) noexcept;
	EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN& operator=(
		EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN&& Other) noexcept;

	bool Is_Valid() const;
	uint64_t Get_CatalogRevision() const;
	uint64_t Get_JobEpoch() const;
	bool Is_PreparedOutcome() const;

private:
	friend class CEffectProductPrewarmQueue;

	void Invalidate() noexcept;

	const CEffectProductPrewarmQueue* m_pOwner = nullptr;
	uint64_t m_iReservationNonce = 0u;
	uint64_t m_iCatalogRevision = 0u;
	uint64_t m_iJobEpoch = 0u;
	bool m_bPrepared = false;
};

/* Loading and in-level presentation switches share this target-set gate.
   Activation depends only on the requested targets settling under the current
   catalog revision. iQueuePendingCount is diagnostic background work and must
   never serialize one selected target behind unrelated Product targets. */
bool Is_ProductPrewarmTargetActivationReady(
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
	EFFECT_PRODUCT_PREWARM_STEP_RESULT Begin_LoadingFrame(
		uint64_t iJobEpoch,
		std::string& strOutEffectAssetId);
	bool Claim_LoadingTargets(
		uint64_t iJobEpoch,
		uint64_t iExpectedCatalogRevision,
		const std::vector<std::string>& EffectAssetIds,
		std::string& strOutStatus);
	bool Release_LoadingOwner(
		uint64_t iJobEpoch,
		EFFECT_PRODUCT_LOADING_OWNER_RELEASE eDisposition,
		std::string& strOutStatus);
	bool Has_LoadingOwner(uint64_t iJobEpoch) const;
	uint64_t Get_LoadingOwnerEpoch(
		const std::string& strEffectAssetId) const;
	bool Prevalidate_Front(
		const std::string& strEffectAssetId,
		uint64_t iExpectedCatalogRevision,
		uint64_t iJobEpoch,
		bool bPrepared,
		const EFFECT_PRODUCT_PREWARM_FAILURE_RECEIPT* pFailure,
		EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN& OutToken,
		std::string& strOutStatus);
	/* This is intentionally void/noexcept.  After prevalidation succeeds the
	   caller may publish its immutable prepared record, then this exact token
	   completes the FIFO terminal mutation without another fallible check or
	   allocation.  Token misuse is a structural invariant violation and fails
	   fast instead of leaving an externally published record permanently pending. */
	void Commit_PrevalidatedFront(
		EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN&& Token) noexcept;
	bool Abort_PrevalidatedFront(
		EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN&& Token,
		std::string& strOutStatus);
	bool Complete_Front(
		const std::string& strEffectAssetId,
		bool bPrepared,
		std::string& strOutStatus);
	bool Commit_AllPrepared(
		const std::set<std::string, std::less<>>& EffectAssetIds,
		std::string& strOutStatus);
	/* Same-revision selected-Effect hot reload.  Removes only this ID from any
	   pending/failed state and commits it prepared without disturbing unrelated
	   FIFO order or target state. */
	bool Commit_HotReloadPrepared(
		const std::string& strEffectAssetId,
		std::string& strOutStatus);
	bool Is_Prepared(const std::string& strEffectAssetId) const;
	const EFFECT_PRODUCT_PREWARM_FAILURE_RECEIPT* Find_FailureReceipt(
		const std::string& strEffectAssetId) const;
	uint64_t Get_CatalogRevision() const;
	const std::set<std::string, std::less<>>& Get_Targets() const;
	EFFECT_PRODUCT_PREWARM_QUEUE_PROBE Get_Probe() const;
	EFFECT_PRODUCT_PREWARM_TARGET_PROBE Get_TargetProbe(
		const std::vector<std::string>& EffectAssetIds,
		uint64_t iExpectedCatalogRevision) const;
	void Clear();

private:
	enum class TARGET_STATE : uint8_t
	{
		PENDING,
		PREPARED,
		FAILED
	};

	struct TARGET_RECORD final
	{
		TARGET_STATE eState = TARGET_STATE::PENDING;
		uint64_t iLoadingOwnerEpoch = 0u;
		uint64_t iReservationNonce = 0u;
		bool bReservedPrepared = false;
		std::optional<EFFECT_PRODUCT_PREWARM_FAILURE_RECEIPT> Failure;
	};

	struct FRONT_RESERVATION final
	{
		uint64_t iNonce = 0u;
		uint64_t iCatalogRevision = 0u;
		uint64_t iJobEpoch = 0u;
		std::string strEffectAssetId;
		bool bPrepared = false;
	};

	bool Is_ReservationTokenCurrent(
		const EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN& Token) const;
	void Remove_LoadingOwnerTarget(
		uint64_t iJobEpoch,
		const std::string& strEffectAssetId) noexcept;

	uint64_t m_iCatalogRevision = 0u;
	std::deque<std::string> m_Pending;
	std::set<std::string, std::less<>> m_Targets;
	std::map<std::string, TARGET_RECORD, std::less<>> m_Records;
	std::map<uint64_t, std::set<std::string, std::less<>>>
		m_LoadingOwnerTargets;
	std::optional<FRONT_RESERVATION> m_FrontReservation;
	uint64_t m_iNextReservationNonce = 1u;
	bool m_bYieldNextFrame = false;
};

}

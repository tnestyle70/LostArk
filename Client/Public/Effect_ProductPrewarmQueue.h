#pragma once

// Client 공통 타입과 큐의 ID·상태·단일 예약을 보관하는 표준 컨테이너를 사용한다.
#include "Client_Defines.h"
// 다른 include 순서에 기대지 않고 NS_BEGIN/NS_END 선언 매크로를 확보한다.
#include "Engine_Macro.h"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CEffectProductPrewarmQueue;

// READY는 준비를 시작할 FIFO 대상이 있다는 뜻이다. 렌더 자원 준비 성공은 별도 commit으로 기록한다.
enum class EFFECT_PRODUCT_PREWARM_STEP_RESULT : uint8_t
{
	IDLE,
	YIELDED,
	READY,
	END
};

// 전체 등록 대상의 상태를 값으로 복사한 진단 결과다. 큐가 렌더 자원을 소유하지는 않는다.
struct EFFECT_PRODUCT_PREWARM_QUEUE_PROBE final
{
	uint64_t iCatalogRevision = 0u;
	uint32_t iTargetCount = 0u;
	uint32_t iPendingCount = 0u;
	uint32_t iPreparedCount = 0u;
	uint32_t iFailedCount = 0u;
	bool bYieldNextFrame = false;
};

// 선택한 대상 집합의 완료 여부와 전체 큐의 잔여 작업량을 구분한다.
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
	// 서비스 전체가 진행하지 못하는 이유다. 개별 대상 실패는 별도 종결 상태로 남는다.
	std::string strBlockingFailure;
};

// 실패 원인을 대상 ID·catalog revision·Loading epoch에 묶어 보관한다. 일반 큐의 epoch는 0이다.
struct EFFECT_PRODUCT_PREWARM_FAILURE_RECEIPT final
{
	uint64_t iCatalogRevision = 0u;
	uint64_t iJobEpoch = 0u;
	std::string strEffectAssetId;
	int64_t iRootCode = 0;
	std::string strRootMessage;

	bool Is_Valid() const;
};

// TERMINAL은 남은 소유 대상이 없어야 한다. 나머지는 pending 상태를 유지하며 Loading 소유권만 해제한다.
enum class EFFECT_PRODUCT_LOADING_OWNER_RELEASE : uint8_t
{
	TERMINAL,
	REBASE,
	CANCELLED,
	STRUCTURAL_FAILURE,
	END
};

// 렌더러 준비 결과를 게시하기 전에 선두와 종결 기록 공간을 예약한다.
// 복사할 수 없는 토큰이며 commit 또는 abort가 정확히 한 번 소비해야 한다.
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

	// 예약한 큐를 차용한다. 큐보다 먼저 commit/abort해야 하며 유효한 토큰을 버리면 fail-fast한다.
	const CEffectProductPrewarmQueue* m_pOwner = nullptr;
	uint64_t m_iReservationNonce = 0u;
	uint64_t m_iCatalogRevision = 0u;
	uint64_t m_iJobEpoch = 0u;
	bool m_bPrepared = false;
};

// Loading과 레벨 내 외형 교체가 공유하는 대상별 진입 조건이다.
// 전체 큐가 아닌 현재 revision의 요청 대상 종결 여부를 본다. 등록 실패 격리는 호출자가 확정한다.
bool Is_ProductPrewarmTargetActivationReady(
	const EFFECT_PRODUCT_PREWARM_TARGET_PROBE& Probe,
	bool bTargetRegistrationFailureIsolated = false);

// PresentationService가 메인 스레드에서 갱신하는 준비 순서·결과 기록이다.
// 내부 동기화는 없으며 문서 파싱·표시 가능성 검증·GPU 작업은 READY를 소비하는 계층이 맡는다.
class CEffectProductPrewarmQueue final
{
public:
	// revision이 바뀔 때만 기존 대상과 완료 기록을 비운다. 활성 예약이 있으면 교체할 수 없다.
	void Reset_ForCatalogRevision(uint64_t iCatalogRevision);
	// 새 ID만 FIFO에 넣고 다음 Begin 호출 한 번을 양보한다. 같은 revision의 완료·실패 ID는 다시 넣지 않는다.
	bool Enqueue(
		const std::vector<std::string>& EffectAssetIds,
		std::string& strOutStatus);
	// Loading이 소유한 선두 묶음을 보존한 뒤 요청한 pending ID를 일반 대상보다 앞에 둔다.
	bool Enqueue_Priority(
		const std::vector<std::string>& EffectAssetIds,
		std::string& strOutStatus);
	// 일반 준비 소비자가 소유자 없는 선두를 조회한다. 이 호출은 FIFO를 제거하지 않는다.
	EFFECT_PRODUCT_PREWARM_STEP_RESULT Begin_Frame(
		std::string& strOutEffectAssetId);
	// 소유자 없는 선두 연속 구간을 복사한다. Loading 소유 대상이나 예약을 건너뛰지 않는다.
	std::vector<std::string> Collect_PendingFrontTargets(size_t iMaximumCount) const;
	// 지정 epoch가 소유한 FIFO 선두만 반환한다. 실제 작업은 PresentationService가 수행한다.
	EFFECT_PRODUCT_PREWARM_STEP_RESULT Begin_LoadingFrame(
		uint64_t iJobEpoch,
		std::string& strOutEffectAssetId);
	// 현재 revision의 pending 대상만 한 Loading epoch에 배정하고 요청 순서대로 선두에 모은다.
	bool Claim_LoadingTargets(
		uint64_t iJobEpoch,
		uint64_t iExpectedCatalogRevision,
		const std::vector<std::string>& EffectAssetIds,
		std::string& strOutStatus);
	// 종료·취소·재기준화 시 Loading 점유를 해제한다. 같은 epoch의 front commit 중에는 거부한다.
	bool Release_LoadingOwner(
		uint64_t iJobEpoch,
		EFFECT_PRODUCT_LOADING_OWNER_RELEASE eDisposition,
		std::string& strOutStatus);
	bool Has_LoadingOwner(uint64_t iJobEpoch) const;
	uint64_t Get_LoadingOwnerEpoch(
		const std::string& strEffectAssetId) const;
	// PresentationService가 catalog/renderer 결과를 게시하기 전에 정확한 선두·revision·epoch를 예약한다.
	// 실패 결과는 pFailure를 복사한다. 성공 시 받은 토큰은 반드시 commit 또는 abort해야 한다.
	bool Prevalidate_Front(
		const std::string& strEffectAssetId,
		uint64_t iExpectedCatalogRevision,
		uint64_t iJobEpoch,
		bool bPrepared,
		const EFFECT_PRODUCT_PREWARM_FAILURE_RECEIPT* pFailure,
		EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN& OutToken,
		std::string& strOutStatus);
	// 외부 결과 게시 뒤 추가 할당 없이 예약한 선두를 종결한다. noexcept 계약이다.
	// 토큰 불일치는 게시 결과와 큐 상태가 어긋난 구조 오류이므로 fail-fast한다.
	void Commit_PrevalidatedFront(
		EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN&& Token) noexcept;
	// catalog/renderer 게시가 실패했을 때 예약만 취소한다. 대상의 pending 상태·FIFO·Loading 소유권은 남는다.
	bool Abort_PrevalidatedFront(
		EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN&& Token,
		std::string& strOutStatus);
	// 일반 큐의 epoch 0 호출을 위한 간편 완료 경로다. 실패 원인이 없으면 호환용 영수증을 만든다.
	bool Complete_Front(
		const std::string& strEffectAssetId,
		bool bPrepared,
		std::string& strOutStatus);
	// 호출자가 준비를 끝낸 전체 집합으로 기록을 교체한다. 이 함수 자체는 자원 준비를 수행하지 않는다.
	bool Commit_AllPrepared(
		const std::set<std::string, std::less<>>& EffectAssetIds,
		std::string& strOutStatus);
	// 같은 revision에서 선택한 Effect의 hot reload 성공만 기록한다.
	// 그 ID의 대기·실패 상태를 대체하고 다른 대상 순서와 상태는 보존한다.
	bool Commit_HotReloadPrepared(
		const std::string& strEffectAssetId,
		std::string& strOutStatus);
	bool Is_Prepared(const std::string& strEffectAssetId) const;
	// 큐 내부 실패 기록을 차용한다. 대상 교체·Clear 전에만 사용하고 보관할 메시지는 복사한다.
	const EFFECT_PRODUCT_PREWARM_FAILURE_RECEIPT* Find_FailureReceipt(
		const std::string& strEffectAssetId) const;
	uint64_t Get_CatalogRevision() const;
	// 큐 내부 집합의 읽기 전용 참조다. 큐 변경을 가로질러 iterator나 참조를 저장하지 않는다.
	const std::set<std::string, std::less<>>& Get_Targets() const;
	EFFECT_PRODUCT_PREWARM_QUEUE_PROBE Get_Probe() const;
	// 중복 ID를 한 번만 집계한다. bSettled는 현재 revision에 pending이 없다는 뜻이며 전부 성공했다는 뜻은 아니다.
	EFFECT_PRODUCT_PREWARM_TARGET_PROBE Get_TargetProbe(
		const std::vector<std::string>& EffectAssetIds,
		uint64_t iExpectedCatalogRevision) const;
	// 활성 예약이 없는 상태에서만 전체 기록을 비운다. renderer 자원 해제는 상위 서비스가 맡는다.
	void Clear();

private:
	enum class TARGET_STATE : uint8_t
	{
		PENDING,
		PREPARED,
		FAILED
	};

	// ID별 종결 상태와 Loading 점유를 보관한다. 예약 동안에도 eState는 pending으로 유지된다.
	struct TARGET_RECORD final
	{
		TARGET_STATE eState = TARGET_STATE::PENDING;
		uint64_t iLoadingOwnerEpoch = 0u;
		uint64_t iReservationNonce = 0u;
		bool bReservedPrepared = false;
		std::optional<EFFECT_PRODUCT_PREWARM_FAILURE_RECEIPT> Failure;
	};

	// 한 번에 하나인 FIFO 선두 예약이다. nonce로 이전 토큰의 재사용을 거부한다.
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

	// 메인 스레드 전용 상태다. FIFO는 ID만, Records는 상태와 실패 값만 소유한다.
	uint64_t m_iCatalogRevision = 0u;
	std::deque<std::string> m_Pending;
	std::set<std::string, std::less<>> m_Targets;
	std::map<std::string, TARGET_RECORD, std::less<>> m_Records;
	// epoch별 미종결 대상 집합이다. 마지막 대상 commit 후 해당 소유자 항목도 제거한다.
	std::map<uint64_t, std::set<std::string, std::less<>>>
		m_LoadingOwnerTargets;
	std::optional<FRONT_RESERVATION> m_FrontReservation;
	uint64_t m_iNextReservationNonce = 1u;
	bool m_bYieldNextFrame = false;
};

NS_END

#include "Effect_ProductPrewarmQueue.h"

// 입력 ID 검사·큐 재정렬, 토큰 오용의 fail-fast, 소유권 이동에 필요한 표준 API다.
#include <algorithm>
#include <exception>
#include <utility>

NS_BEGIN(Client)

bool EFFECT_PRODUCT_PREWARM_FAILURE_RECEIPT::Is_Valid() const
{
	return 0u != iCatalogRevision && !strEffectAssetId.empty() &&
		0 != iRootCode && !strRootMessage.empty();
}

EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN::
EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN(
	EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN&& Other) noexcept
{
	*this = std::move(Other);
}

// 유효한 예약을 조용히 버리면 외부 게시 결과와 큐가 어긋난다. 호출자는 먼저 commit/abort해야 한다.
EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN::
~EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN() noexcept
{
	if (Is_Valid())
	{
		std::terminate();
	}
}

// 기존 예약을 덮어쓰지 않고 한 토큰에만 처리 의무를 남긴다. 이동된 원본은 즉시 무효화한다.
EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN&
EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN::operator=(
	EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN&& Other) noexcept
{
	if (this == &Other)
	{
		return *this;
	}
	if (Is_Valid())
	{
		std::terminate();
	}
	m_pOwner = Other.m_pOwner;
	m_iReservationNonce = Other.m_iReservationNonce;
	m_iCatalogRevision = Other.m_iCatalogRevision;
	m_iJobEpoch = Other.m_iJobEpoch;
	m_bPrepared = Other.m_bPrepared;
	Other.Invalidate();
	return *this;
}

bool EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN::Is_Valid() const
{
	return nullptr != m_pOwner && 0u != m_iReservationNonce &&
		0u != m_iCatalogRevision;
}

uint64_t EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN::
Get_CatalogRevision() const
{
	return m_iCatalogRevision;
}

uint64_t EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN::Get_JobEpoch() const
{
	return m_iJobEpoch;
}

bool EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN::
Is_PreparedOutcome() const
{
	return m_bPrepared;
}

void EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN::Invalidate() noexcept
{
	m_pOwner = nullptr;
	m_iReservationNonce = 0u;
	m_iCatalogRevision = 0u;
	m_iJobEpoch = 0u;
	m_bPrepared = false;
}

// 등록 실패를 상위 서비스가 격리한 경우도 허용하되 이전 catalog revision은 통과시키지 않는다.
bool Is_ProductPrewarmTargetActivationReady(
	const EFFECT_PRODUCT_PREWARM_TARGET_PROBE& Probe,
	const bool bTargetRegistrationFailureIsolated)
{
	return Probe.bCatalogRevisionCurrent &&
		(Probe.bSettled || bTargetRegistrationFailureIsolated);
}

// 새 catalog에 이전 prepared/failed 판정이 섞이지 않도록 revision 경계에서 기록을 초기화한다.
void CEffectProductPrewarmQueue::Reset_ForCatalogRevision(
	const uint64_t iCatalogRevision)
{
	if (m_iCatalogRevision == iCatalogRevision)
	{
		return;
	}
	if (m_FrontReservation.has_value())
	{
		std::terminate();
	}
	Clear();
	m_iCatalogRevision = iCatalogRevision;
}

// 등록 검증 뒤 새 ID만 pending으로 추가한다. 기존 terminal 결과는 같은 revision 동안 재사용한다.
bool CEffectProductPrewarmQueue::Enqueue(
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
		{
			continue;
		}
		m_Records.emplace(EffectAssetId, TARGET_RECORD{});
		m_Pending.push_back(EffectAssetId);
		++iQueuedCount;
	}
	if (0u != iQueuedCount)
	{
		m_bYieldNextFrame = true;
	}
	strOutStatus = "Queued " + std::to_string(iQueuedCount) +
		" new Product Effect targets for incremental prewarm.";
	return true;
}

// 우선 요청의 중복을 제거하되 이미 진행 중인 Loading 묶음은 앞으로 추월하지 않는다.
bool CEffectProductPrewarmQueue::Enqueue_Priority(
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

	// Loading 소유 묶음을 먼저 보존한다. 새 일반 우선 요청이 이를 추월하면 해당 epoch가 선두에 도달하지 못한다.
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
	{
		m_bYieldNextFrame = true;
	}
	strOutStatus = "Queued " + std::to_string(iQueuedCount) +
		" new and prioritized " + std::to_string(PriorityPending.size()) +
		" pending Product Effect targets.";
	return true;
}

// 일반 준비 루프는 소유자 없는 선두만 받는다. READY 뒤의 작업 성공/실패가 commit할 때 FIFO가 줄어든다.
EFFECT_PRODUCT_PREWARM_STEP_RESULT
CEffectProductPrewarmQueue::Begin_Frame(
	std::string& strOutEffectAssetId)
{
	strOutEffectAssetId.clear();
	if (m_Pending.empty() || m_FrontReservation.has_value())
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
		0u != Record->second.iLoadingOwnerEpoch)
	{
		return EFFECT_PRODUCT_PREWARM_STEP_RESULT::IDLE;
	}
	strOutEffectAssetId = m_Pending.front();
	return EFFECT_PRODUCT_PREWARM_STEP_RESULT::READY;
}

// 한 worker 창에 전달할 일반 pending 연속 구간을 복사한다. 다른 Loading 점유를 만나면 거기서 멈춘다.
std::vector<std::string>
CEffectProductPrewarmQueue::Collect_PendingFrontTargets(
	const size_t iMaximumCount) const
{
	std::vector<std::string> Targets;
	if (m_FrontReservation.has_value() || 0u == m_iCatalogRevision)
	{
		return Targets;
	}
	Targets.reserve((std::min)(iMaximumCount, m_Pending.size()));
	for (const std::string& EffectId : m_Pending)
	{
		if (Targets.size() == iMaximumCount)
		{
			break;
		}
		const auto Record = m_Records.find(EffectId);
		if (m_Records.end() == Record ||
			TARGET_STATE::PENDING != Record->second.eState ||
			0u != Record->second.iLoadingOwnerEpoch)
		{
			break;
		}
		Targets.push_back(EffectId);
	}
	return Targets;
}

// Loading 결과를 반영할 때도 현재 선두와 epoch를 확인해 이전 작업의 완료가 다음 대상을 소비하지 못하게 한다.
EFFECT_PRODUCT_PREWARM_STEP_RESULT
CEffectProductPrewarmQueue::Begin_LoadingFrame(
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

// PresentationService가 작업 묶음을 열 때 현재 pending ID를 한 epoch에 배정한다.
// 다른 epoch와 겹치거나 같은 epoch의 대상 집합이 달라지면 큐를 재배치하기 전에 거부한다.
bool CEffectProductPrewarmQueue::Claim_LoadingTargets(
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
		{
			OrderedClaimedTargets.push_back(EffectAssetId);
		}
	}

	const auto ExistingOwner = m_LoadingOwnerTargets.find(iJobEpoch);
	if (m_LoadingOwnerTargets.end() != ExistingOwner &&
		ExistingOwner->second != ClaimedTargets)
	{
		strOutStatus =
			"Effect Product Loading epoch already owns a different target set.";
		return false;
	}

	// 요청 순서를 Loading 선두로 만들고 나머지 일반 pending의 상대 순서는 유지한다.
	std::deque<std::string> Reordered;
	for (const std::string& EffectAssetId : OrderedClaimedTargets)
	{
		Reordered.push_back(EffectAssetId);
	}
	for (const std::string& EffectAssetId : m_Pending)
	{
		if (!ClaimedTargets.contains(EffectAssetId))
		{
			Reordered.push_back(EffectAssetId);
		}
	}

	if (!ClaimedTargets.empty() &&
		m_LoadingOwnerTargets.end() == ExistingOwner)
	{
		m_LoadingOwnerTargets.emplace(iJobEpoch, ClaimedTargets);
	}
	for (const std::string& EffectAssetId : ClaimedTargets)
	{
		m_Records.find(EffectAssetId)->second.iLoadingOwnerEpoch = iJobEpoch;
	}
	m_Pending = std::move(Reordered);
	strOutStatus = "Claimed " + std::to_string(ClaimedTargets.size()) +
		" pending Product Effect targets for Loading epoch " +
		std::to_string(iJobEpoch) + ".";
	return true;
}

// 작업 중단 시 소유권만 반납한다. pending을 임의로 성공/실패 처리하지 않아 후속 준비가 다시 맡을 수 있다.
bool CEffectProductPrewarmQueue::Release_LoadingOwner(
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

bool CEffectProductPrewarmQueue::Has_LoadingOwner(
	const uint64_t iJobEpoch) const
{
	return 0u != iJobEpoch && m_LoadingOwnerTargets.contains(iJobEpoch);
}

uint64_t CEffectProductPrewarmQueue::Get_LoadingOwnerEpoch(
	const std::string& strEffectAssetId) const
{
	const auto Record = m_Records.find(strEffectAssetId);
	return m_Records.end() == Record ? 0u :
		Record->second.iLoadingOwnerEpoch;
}

// 메인 스레드 게시 직전 호출한다. 대상·revision·epoch·실패 영수증이 모두 일치할 때만 예약을 발급한다.
bool CEffectProductPrewarmQueue::Prevalidate_Front(
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

	// 실패 영수증과 선두 예약의 문자열 저장은 여기서 끝낸다. 이후 terminal commit에는 새 할당이 필요하지 않다.
	uint64_t iNonce = m_iNextReservationNonce++;
	if (0u == iNonce)
	{
		iNonce = m_iNextReservationNonce++;
		if (0u == iNonce)
		{
			std::terminate();
		}
	}
	Record->second.iReservationNonce = iNonce;
	Record->second.bReservedPrepared = bPrepared;
	if (bPrepared)
	{
		Record->second.Failure.reset();
	}
	else
	{
		Record->second.Failure = *pFailure;
	}
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

// catalog/renderer 게시가 끝난 뒤 동일 예약을 종결한다. 무효 토큰은 복구 가능한 대상 실패와 구분한다.
void CEffectProductPrewarmQueue::Commit_PrevalidatedFront(
	EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN&& Token) noexcept
{
	if (!Is_ReservationTokenCurrent(Token))
	{
		std::terminate();
	}
	const FRONT_RESERVATION& Reservation = *m_FrontReservation;
	auto Record = m_Records.find(Reservation.strEffectAssetId);
	if (m_Records.end() == Record || m_Pending.empty() ||
		m_Pending.front() != Reservation.strEffectAssetId ||
		Record->second.iReservationNonce != Reservation.iNonce)
	{
		std::terminate();
	}

	// 결과 기록·FIFO 제거·Loading 점유 반환을 같은 메인 스레드 단계에서 끝내고 토큰을 무효화한다.
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

// 외부 게시 rollback 뒤 예약과 임시 실패 값만 버린다. 원래 pending 대상과 Loading 소유자는 그대로다.
bool CEffectProductPrewarmQueue::Abort_PrevalidatedFront(
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

// 원인 영수증을 받지 않는 기존 일반 큐 호출도 동일한 예약/종결 계약으로 처리한다.
bool CEffectProductPrewarmQueue::Complete_Front(
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

// 이미 준비된 전체 ID 집합으로 큐 기록을 교체한다. 이 경로에서는 모든 대기와 Loading 점유를 비운다.
bool CEffectProductPrewarmQueue::Commit_AllPrepared(
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

// 선택 Effect 하나의 준비 성공만 반영한다. Loading이 점유한 대상은 그 작업의 commit 순서를 보존한다.
bool CEffectProductPrewarmQueue::Commit_HotReloadPrepared(
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
	{
		Remove_LoadingOwnerTarget(iLoadingOwnerEpoch, strEffectAssetId);
	}
	strOutStatus =
		"Committed selected Product Effect target as hot-reload prepared.";
	return true;
}

bool CEffectProductPrewarmQueue::Is_Prepared(
	const std::string& strEffectAssetId) const
{
	const auto Record = m_Records.find(strEffectAssetId);
	return m_Records.end() != Record &&
		TARGET_STATE::PREPARED == Record->second.eState;
}

const EFFECT_PRODUCT_PREWARM_FAILURE_RECEIPT*
CEffectProductPrewarmQueue::Find_FailureReceipt(
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

uint64_t CEffectProductPrewarmQueue::Get_CatalogRevision() const
{
	return m_iCatalogRevision;
}

const std::set<std::string, std::less<>>&
CEffectProductPrewarmQueue::Get_Targets() const
{
	return m_Targets;
}

// 전체 큐 진단은 ID별 상태로 집계한다. 실패 대상도 삭제하지 않아 같은 revision의 결과를 조회할 수 있다.
EFFECT_PRODUCT_PREWARM_QUEUE_PROBE
CEffectProductPrewarmQueue::Get_Probe() const
{
	EFFECT_PRODUCT_PREWARM_QUEUE_PROBE Probe;
	Probe.iCatalogRevision = m_iCatalogRevision;
	Probe.iTargetCount = static_cast<uint32_t>(m_Targets.size());
	for (const auto& [EffectAssetId, Record] : m_Records)
	{
		if (TARGET_STATE::PENDING == Record.eState)
		{
			++Probe.iPendingCount;
		}
		else if (TARGET_STATE::PREPARED == Record.eState)
		{
			++Probe.iPreparedCount;
		}
		else if (TARGET_STATE::FAILED == Record.eState)
		{
			++Probe.iFailedCount;
		}
	}
	Probe.bYieldNextFrame = m_bYieldNextFrame;
	return Probe;
}

// 선택 대상만 중복 없이 집계한다. 무관한 background pending은 진입 완료 조건에 넣지 않는다.
EFFECT_PRODUCT_PREWARM_TARGET_PROBE
CEffectProductPrewarmQueue::Get_TargetProbe(
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
		{
			continue;
		}
		++Probe.iTargetCount;
		const auto Record = m_Records.find(EffectAssetId);
		if (m_Records.end() == Record)
		{
			++Probe.iUnavailableCount;
		}
		else if (TARGET_STATE::PENDING == Record->second.eState)
		{
			++Probe.iPendingCount;
		}
		else if (TARGET_STATE::PREPARED == Record->second.eState)
		{
			++Probe.iPreparedCount;
		}
		else if (TARGET_STATE::FAILED == Record->second.eState)
		{
			++Probe.iFailedCount;
		}
	}

	// 미등록·실패 대상도 현재 revision에서 종결된 표시 실패로 센다. pending이 없으면 레벨 전환을 붙잡지 않는다.
	Probe.bSettled = Probe.bCatalogRevisionCurrent &&
		0u == Probe.iPendingCount;
	return Probe;
}

// 큐 주소뿐 아니라 nonce·revision·epoch·결과를 대조해 이전 예약 토큰의 재사용을 거부한다.
bool CEffectProductPrewarmQueue::Is_ReservationTokenCurrent(
	const EFFECT_PRODUCT_PREWARM_FRONT_COMMIT_TOKEN& Token) const
{
	return Token.Is_Valid() && Token.m_pOwner == this &&
		m_FrontReservation.has_value() &&
		Token.m_iReservationNonce == m_FrontReservation->iNonce &&
		Token.m_iCatalogRevision == m_FrontReservation->iCatalogRevision &&
		Token.m_iJobEpoch == m_FrontReservation->iJobEpoch &&
		Token.m_bPrepared == m_FrontReservation->bPrepared;
}

// 한 대상 종결 후 남은 소유 ID가 없으면 epoch 항목도 제거해 다음 Loading 작업이 점유할 수 있게 한다.
void CEffectProductPrewarmQueue::Remove_LoadingOwnerTarget(
	const uint64_t iJobEpoch,
	const std::string& strEffectAssetId) noexcept
{
	const auto Owner = m_LoadingOwnerTargets.find(iJobEpoch);
	if (m_LoadingOwnerTargets.end() == Owner)
	{
		return;
	}
	Owner->second.erase(strEffectAssetId);
	if (Owner->second.empty())
	{
		m_LoadingOwnerTargets.erase(Owner);
	}
}

// 외부 결과 게시 도중 기록이 사라지면 안 되므로 활성 예약을 먼저 해소해야 한다.
void CEffectProductPrewarmQueue::Clear()
{
	if (m_FrontReservation.has_value())
	{
		std::terminate();
	}
	m_iCatalogRevision = 0u;
	m_Pending.clear();
	m_Targets.clear();
	m_Records.clear();
	m_LoadingOwnerTargets.clear();
	m_iNextReservationNonce = 1u;
	m_bYieldNextFrame = false;
}

NS_END

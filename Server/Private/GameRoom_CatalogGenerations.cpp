// CGameplayCatalogGenerations 선언과 불변 catalog의 shared_ptr 소유 목록을 사용한다. 룸 실행 스레드가 세대 조회·교체·회수를 직렬로 호출한다.
#include "GameRoom.h"

// 아래 세대 보관 본문은 ClientSession·피격·packet·충돌 API를 직접 사용하지 않는다. 기존 분리 파일의 include 구성을 보존한다.
#include "ClientSession.h"
#include "ServerCombatHitRuntime.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"
#include "Gameplay/WorldCollisionContract.h"

// 보관 세대 중복 검사와 live revision에 없는 세대 제거를 수행한다.
#include <algorithm>
// array·chrono·cctype·cmath는 현재 본문에서 직접 사용하지 않는 기존 include다.
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
// transaction sequence와 세대 epoch의 고정 폭 정수를 사용한다.
#include <cstdint>
// cstdlib·iostream은 현재 본문에서 직접 사용하지 않는 기존 include다.
#include <cstdlib>
#include <iostream>
// 16비트 epoch의 최댓값에 도달하면 새 choreography 세대 예약을 거절한다.
#include <limits>
// new·set·string_view는 현재 본문에서 직접 사용하지 않는 기존 include다.
#include <new>
#include <set>
#include <string_view>
// 공유 포인터와 지역 후보 vector의 소유권을 이동한다.
#include <utility>

// 현재 세대 보관 함수는 GameRoomDetail 도우미를 직접 호출하지 않는다. 기존 내부 include와 using을 유지한다.
#include "GameRoom_Internal.h"

using namespace GameRoomDetail;

// 룸이 생성될 때 최대 16세대의 vector 공간을 먼저 확보한다. Commit은 용량을 다시 검사해 push_back의 vector 재할당을 피한다.
LostArk::Server::CGameplayCatalogGenerations::CGameplayCatalogGenerations()
{
	m_Generations.reserve(MAX_GENERATION_COUNT);
}

// 초기화 호출이 새 catalog를 지역 shared_ptr로 로드한 뒤 Initialize로 넘긴다. 명시적 로드 실패는 현재 세대 목록을 유지하고 진단만 갱신한다.
// make_shared나 로드의 예외를 이 함수에서 false로 변환하지는 않는다.
bool LostArk::Server::CGameplayCatalogGenerations::Load()
{
	auto initial = std::make_shared<CGameplayCatalog>();
	if (nullptr == initial || !initial->Load())
	{
		m_strStatus = nullptr == initial ? "Gameplay catalog allocation failed" : initial->Get_Status();
		return false;
	}
	return Initialize(std::move(initial));
}

// CGameRoom 초기화가 유효한 초기 세대의 공유 소유권을 받는다. 잘못된 입력은 기존 세대를 유지하고 성공하면 보관 목록·active·일반 stage를 초기 상태(epoch1)로 바꾼다.
// 수치 변경 예약을 정리하는 함수는 아니므로 실행 중 transaction의 재초기화 용도로 호출하지 않는다.
bool LostArk::Server::CGameplayCatalogGenerations::Initialize(
    const std::shared_ptr<const CGameplayCatalog>& initialGeneration)
{
	if (nullptr == initialGeneration || !initialGeneration->Get_ActiveRevision().Is_Valid())
	{
		m_strStatus = "Initial gameplay generation is invalid";
		return false;
	}
	m_Generations.clear();
	m_Generations.push_back(initialGeneration);
	m_pActiveGeneration = initialGeneration;
	m_pStagedGeneration.reset();
	m_iStagedTransactionSequence = 0u;
	m_iActiveGenerationEpoch = 1u;
	m_strStatus = "Initialized immutable gameplay generation";
	return true;
}

// CGameRoom::Stage_GameplayGeneration이 live pin 회수 뒤 새 choreography 세대를 예약한다. active revision·sequence·epoch·수치 transaction 배제를 검사하며 아직 active를 바꾸지 않는다.
// 검증/용량 실패는 기존 stage를 보존한다. 이미 같은 sequence/revision이 예약된 재호출은 true지만 입력 status를 지우지 않는다.
bool LostArk::Server::CGameplayCatalogGenerations::Stage(
    const std::uint32_t transactionSequence, const LostArk::Shared::GameplayDataRevision& baseRevision,
    const std::shared_ptr<const CGameplayCatalog>& candidateGeneration, std::string& status)
{
	if (0u == transactionSequence || m_iNumericTransactionSequence || nullptr == m_pActiveGeneration ||
	    (std::numeric_limits<std::uint16_t>::max)() == m_iActiveGenerationEpoch ||
	    m_pActiveGeneration->Get_ActiveRevision() != baseRevision || nullptr == candidateGeneration ||
	    !candidateGeneration->Get_ActiveRevision().Is_Valid() ||
	    candidateGeneration->Get_ActiveRevision() == baseRevision)
	{
		status = "Room gameplay generation stage identity is invalid";
		return false;
	}
	if (nullptr != m_pStagedGeneration)
	{
		if (m_iStagedTransactionSequence == transactionSequence &&
		    m_pStagedGeneration->Get_ActiveRevision() == candidateGeneration->Get_ActiveRevision())
		{
			return true;
		}
		status = "Room already owns a different staged gameplay generation";
		return false;
	}
	// 같은 revision이 이미 보관되어 있으면 전달된 새 객체 대신 그 보관 객체를 다시 사용한다. 따라서 revision은 서로 같은 정의를 가리키는 식별자라는 호출 계약이 필요하다.
	const bool alreadyRetained = nullptr != Resolve(candidateGeneration->Get_ActiveRevision());
	if (!alreadyRetained && m_Generations.size() >= MAX_GENERATION_COUNT)
	{
		status = "Room gameplay generation capacity is exhausted";
		return false;
	}
	m_pStagedGeneration = alreadyRetained ? std::shared_ptr<const CGameplayCatalog>{} : candidateGeneration;
	if (alreadyRetained)
	{
		for (const auto& generation : m_Generations)
		{
			if (nullptr != generation && generation->Get_ActiveRevision() == candidateGeneration->Get_ActiveRevision())
			{
				m_pStagedGeneration = generation;
				break;
			}
		}
	}
	m_iStagedTransactionSequence = transactionSequence;
	status.clear();
	return nullptr != m_pStagedGeneration;
}

// ServerApp이 모든 방의 stage와 Client 승인·게시 준비를 마친 뒤 룸 경유로 같은 sequence를 확정한다. identity/용량 실패는 stage를 남긴다.
// 새 세대를 보관 목록에 추가하고 active로 옮기며 epoch를1 증가시킨다. noexcept이지만 마지막 진단 문자열 대입의 할당 예외를 복구하지는 않는다.
bool LostArk::Server::CGameplayCatalogGenerations::Commit(const std::uint32_t transactionSequence) noexcept
{
	if (0u == transactionSequence || transactionSequence != m_iStagedTransactionSequence ||
	    nullptr == m_pStagedGeneration)
	{
		return false;
	}
	const bool alreadyRetained =
	    std::any_of(m_Generations.begin(), m_Generations.end(),
	                [this](const std::shared_ptr<const CGameplayCatalog>& generation)
	                {
		                return nullptr != generation &&
		                       generation->Get_ActiveRevision() == m_pStagedGeneration->Get_ActiveRevision();
	                });
	if (!alreadyRetained)
	{
		if (m_Generations.size() >= m_Generations.capacity())
			return false;
		m_Generations.push_back(m_pStagedGeneration);
	}
	m_pActiveGeneration = std::move(m_pStagedGeneration);
	m_iStagedTransactionSequence = 0u;
	++m_iActiveGenerationEpoch;
	m_strStatus = "Committed immutable gameplay generation";
	return true;
}

// 실패한 전역 세대 교체가 일반 stage의 소유권만 해제한다. sequence0은 무조건 취소이고 다른 비영 sequence는 현재 예약을 건드리지 않는다. active/기존 보관 세대는 유지한다.
void LostArk::Server::CGameplayCatalogGenerations::Abort(const std::uint32_t transactionSequence) noexcept
{
	if (0u != transactionSequence && transactionSequence != m_iStagedTransactionSequence)
	{
		return;
	}
	m_pStagedGeneration.reset();
	m_iStagedTransactionSequence = 0u;
}

// 룸 tick 끝과 새 stage 직전이 실제 world entity·진행 occurrence·combat object에서 모은 revision pin을 전달한다.
// active/staged 및 pin에 남은 세대는 보존하고 나머지는 이 facade의 shared_ptr만 해제한다. 외부 shared_ptr 소유자가 있으면 객체는 계속 산다. null 항목도 현재 구현은 남긴다.
void LostArk::Server::CGameplayCatalogGenerations::Collect_Garbage(
    const std::vector<LostArk::Shared::GameplayDataRevision>& livePins)
{
	m_Generations.erase(std::remove_if(m_Generations.begin(), m_Generations.end(),
	                                   [this, &livePins](const std::shared_ptr<const CGameplayCatalog>& generation)
	                                   {
		                                   if (nullptr == generation || generation == m_pActiveGeneration ||
		                                       generation == m_pStagedGeneration)
		                                   {
			                                   return false;
		                                   }
		                                   return livePins.end() == std::find(livePins.begin(), livePins.end(),
		                                                                      generation->Get_ActiveRevision());
	                                   }),
	                    m_Generations.end());
}

// 실행 중 패턴이 캡처한 revision을 보관 목록, 그다음 일반 stage에서 찾는다. 없거나 invalid면 nullptr이며 수치 후보 목록은 아직 노출하지 않는다.
// 반환 포인터는 소유권을 늘리지 않는다. 회수·재초기화·수치 commit을 넘겨 보관하려면 별도의 세대 소유권이나 다시 조회할 revision이 필요하다.
const LostArk::Server::CGameplayCatalog* LostArk::Server::CGameplayCatalogGenerations::Resolve(
    const LostArk::Shared::GameplayDataRevision& revision) const noexcept
{
	if (!revision.Is_Valid())
		return nullptr;
	for (const auto& generation : m_Generations)
	{
		if (nullptr != generation && generation->Get_ActiveRevision() == revision)
		{
			return generation.get();
		}
	}
	if (nullptr != m_pStagedGeneration && m_pStagedGeneration->Get_ActiveRevision() == revision)
	{
		return m_pStagedGeneration.get();
	}
	return nullptr;
}

// 일반 player/새 패턴 조회가 현재 active를 차용한다. 초기화 전에는 함수 정적 빈 catalog를 반환한다. 참조는 해당 객체의 실제 소유 수명까지만 유효하다.
const LostArk::Server::CGameplayCatalog& LostArk::Server::CGameplayCatalogGenerations::Active() const noexcept
{
	static const CGameplayCatalog EMPTY_CATALOG{};
	return nullptr == m_pActiveGeneration ? EMPTY_CATALOG : *m_pActiveGeneration;
}

// player skill 실행이 현재 active의 stable skill ID를 조회한다. 아래 Find 계열은 진행 occurrence의 과거 pin을 선택하지 않으며 반환 포인터는 catalog 내부를 차용한다.
const LostArk::Server::PLAYER_SKILL_DEFINITION* LostArk::Server::CGameplayCatalogGenerations::Find_Skill(
    const LostArk::Shared::SKILL_ID skillId) const
{
	return Active().Find_Skill(skillId);
}

// 보스 생성·수치 갱신이 archetype의 현재 profile을 조회한다. 해당 ID가 없으면 nullptr다.
const LostArk::Server::BOSS_RUNTIME_PROFILE* LostArk::Server::CGameplayCatalogGenerations::Find_Boss(
    const std::string& archetypeId) const
{
	return Active().Find_Boss(archetypeId);
}

// 보스 구성 경로가 archetype의 부위 정의 목록을 차용한다. 목록 부재와 비어 있는 목록은 반환 포인터로 구분한다.
const std::vector<LostArk::Server::BOSS_PART_DEFINITION>* LostArk::Server::CGameplayCatalogGenerations::Find_BossParts(
    const std::string& archetypeId) const
{
	return Active().Find_BossParts(archetypeId);
}

// encounter의 현재 패턴 목록을 차용한다. 기존 occurrence의 정의가 필요하면 호출부가 Resolve(pin)한 catalog를 직접 조회해야 한다.
const std::vector<LostArk::Server::BOSS_PATTERN_DEFINITION>* LostArk::Server::CGameplayCatalogGenerations::
    Find_BossPatterns(const std::string& encounterId) const
{
	return Active().Find_BossPatterns(encounterId);
}

// 전투 개체 생성/검증이 archetype에 맞는 현재 정의를 조회한다. lookup 실패는 nullptr이며 개체를 생성하거나 소유하지 않는다.
const LostArk::Server::BOSS_COMBAT_OBJECT_DEFINITION* LostArk::Server::CGameplayCatalogGenerations::
    Find_BossCombatObject(const std::string& archetypeId) const
{
	return Active().Find_BossCombatObject(archetypeId);
}

// 발탄 timeline 시작/조회가 encounter의 현재 저작 timeline을 차용한다.
const LostArk::Server::VALTAN_TIMELINE_DEFINITION* LostArk::Server::CGameplayCatalogGenerations::Find_ValtanTimeline(
    const std::string& encounterId) const
{
	return Active().Find_ValtanTimeline(encounterId);
}

// 발탄 명령 조회가 encounter와 stable command ID로 현재 timeline 행을 찾는다. 누락은 nullptr다.
const LostArk::Server::VALTAN_TIMELINE_ROW* LostArk::Server::CGameplayCatalogGenerations::Find_ValtanTimelineRow(
    const std::string& encounterId, const std::uint32_t commandId) const
{
	return Active().Find_ValtanTimelineRow(encounterId, commandId);
}

// 패턴 선택이 encounter·gameplay phase·현재 체력 줄 수에 맞는 rotation을 조회한다. 구간/phase 판정은 catalog의 기존 규칙에 위임한다.
const LostArk::Server::BOSS_PATTERN_ROTATION_DEFINITION* LostArk::Server::CGameplayCatalogGenerations::
    Find_BossPatternRotation(const std::string& encounterId, const std::uint32_t gameplayPhase,
                             const std::uint32_t healthBar) const
{
	return Active().Find_BossPatternRotation(encounterId, gameplayPhase, healthBar);
}

// 보스 입장 준비가 encounter의 도입 pattern ID를 차용한다. 없으면 catalog 조회가 제공하는 정적 빈 문자열 참조를 반환한다.
const std::string& LostArk::Server::CGameplayCatalogGenerations::Find_IntroPatternId(
    const std::string& encounterId) const
{
	return Active().Find_IntroPatternId(encounterId);
}

// player 생성·class 변경·수치 갱신이 현재 class profile을 차용한다. 인스턴스 HP나 위치를 이 조회가 변경하지 않는다.
const LostArk::Server::PLAYER_RUNTIME_PROFILE* LostArk::Server::CGameplayCatalogGenerations::Find_Player(
    const LostArk::Shared::CHARACTER_CLASS_ID characterClass) const
{
	return Active().Find_Player(characterClass);
}

// 피해 계산 경로가 profile ID의 배율 percent를 값으로 받는다. 정의가 없는 경우0이라는 catalog의 반환값을 그대로 전달한다.
std::uint32_t LostArk::Server::CGameplayCatalogGenerations::Find_DamageRatePercent(
    const std::string& damageProfileId) const
{
	return Active().Find_DamageRatePercent(damageProfileId);
}

// snapshot·새 occurrence·transaction 검증이 현재 choreography revision을 차용한다. 수치만 바꾸는 commit은 이 revision이나 epoch를 새로 만들지 않는다.
const LostArk::Shared::GameplayDataRevision& LostArk::Server::CGameplayCatalogGenerations::Get_ActiveRevision()
    const noexcept
{
	return Active().Get_ActiveRevision();
}

// 초기화 진단 조회는 facade 자체 상태가 비어 있지 않으면 그것을 우선한다. Stage의 외부 status나 수치 commit 결과를 자동으로 최신 진단에 합치지는 않는다.
const std::string& LostArk::Server::CGameplayCatalogGenerations::Get_Status() const noexcept
{
	return m_strStatus.empty() ? Active().Get_Status() : m_strStatus;
}

// CGameRoom의 수치 transaction 준비가 같은 active choreography revision의 후보를 받는다. 일반 stage/다른 수치 예약과 겹치면 거절한다.
// 지역 vector에서 모든 보관 세대의 수치 복사본을 완성한 뒤 한꺼번에 예약한다. 명시적 false는 active/보관 목록을 유지하며 할당 예외는 잡지 않는다.
bool LostArk::Server::CGameplayCatalogGenerations::Stage_NumericBalance(
    const std::uint32_t transactionSequence, const std::shared_ptr<const CGameplayCatalog>& candidate,
    std::string& status)
{
	if (!transactionSequence || !candidate || !m_pActiveGeneration || m_pStagedGeneration ||
	    m_iNumericTransactionSequence || candidate->Get_ActiveRevision() != m_pActiveGeneration->Get_ActiveRevision())
	{
		status = "Numeric stage conflicts with the room's active or pending generation";
		return false;
	}
	std::vector<std::shared_ptr<const CGameplayCatalog>> staged;
	staged.reserve(m_Generations.capacity());
	// active 슬롯은 후보 shared_ptr를 그대로 쓰고 과거 revision은 각자의 choreography를 복사한 뒤 공통 숫자만 덮는다. 기존 세대 객체를 제자리 수정하지 않는다.
	// 보관 목록에는 Initialize/Commit으로 넣은 non-null 세대만 있다는 내부 불변식을 전제로 역참조한다.
	for (const auto& generation : m_Generations)
	{
		if (generation == m_pActiveGeneration)
			staged.push_back(candidate);
		else
		{
			auto updated = std::make_shared<CGameplayCatalog>();
			if (!updated->Load_NumericBalanceValues(*generation, *candidate))
			{
				status = "Retained occurrence rejects numeric balance: " + updated->Get_Status();
				return false;
			}
			staged.push_back(std::move(updated));
		}
	}
	m_NumericStagedGenerations = std::move(staged);
	m_pNumericStagedActive = candidate;
	m_iNumericTransactionSequence = transactionSequence;
	status.clear();
	return true;
}

// 전 방 준비·디스크 영속화 뒤 같은 수치 sequence의 후보 vector와 active를 교체한다. 잘못된 sequence/비어 있는 후보는 기존 상태를 유지한다.
// 기존 raw 조회 포인터의 수명은 별도 shared_ptr 소유 여부에 달려 있으므로 commit 후에는 revision으로 다시 조회한다.
bool LostArk::Server::CGameplayCatalogGenerations::Commit_NumericBalance(
    const std::uint32_t transactionSequence) noexcept
{
	if (!transactionSequence || transactionSequence != m_iNumericTransactionSequence || !m_pNumericStagedActive ||
	    m_NumericStagedGenerations.empty())
		return false;
	// choreography revision과 epoch를 유지한 채 그 revision에 대응하는 객체를 새 수치 복사본으로 바꾼다. 숫자 편집마다 보관 세대 수를 추가하지 않는다.
	m_Generations.swap(m_NumericStagedGenerations);
	m_pActiveGeneration = std::move(m_pNumericStagedActive);
	m_NumericStagedGenerations.clear();
	m_iNumericTransactionSequence = 0u;
	return true;
}

// 수치 저장/준비 실패가 예약 vector와 후보 active의 공유 소유권을 해제한다. sequence0은 무조건 취소, 다른 비영 sequence는 무시하며 일반 stage에는 영향이 없다.
void LostArk::Server::CGameplayCatalogGenerations::Abort_NumericBalance(
    const std::uint32_t transactionSequence) noexcept
{
	if (transactionSequence && transactionSequence != m_iNumericTransactionSequence)
		return;
	m_NumericStagedGenerations.clear();
	m_pNumericStagedActive.reset();
	m_iNumericTransactionSequence = 0u;
}

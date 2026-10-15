#include "EstherSkillSystem.h"

#include <cmath>

// GameRoom의 Debug 호출과 슬롯 소비가 사용하는 정적 roster 선형 조회다. 반환 pointer는 소유권을 이전하지 않는다.
const LostArk::Server::ESTHER_ROSTER_ENTRY*
LostArk::Server::Find_EstherDefinition(
	const LostArk::Shared::ESTHER_ID estherId)
{
	for (const ESTHER_ROSTER_ENTRY& entry : ESTHER_DEFINITIONS)
	{
		if (entry.eEstherId == estherId)
			return &entry;
	}
	return nullptr;
}

// GameRoom 초기화가 world를 전달한다. 지원 world와 슬롯 목록을 결정하고 이전 게이지/잔여량을 함께 지운다.
void LostArk::Server::CEstherSkillSystem::Initialize(
	const LostArk::Shared::WORLD_ID worldId)
{
	m_isEnabled =
		LostArk::Shared::WORLD_ID::VALTAN_ARENA == worldId ||
		LostArk::Shared::WORLD_ID::KOUKUSAYDON_ARENA == worldId ||
		LostArk::Shared::WORLD_ID::CHARACTER_SELECT_ARENA == worldId;
	m_Roster = LostArk::Shared::WORLD_ID::KOUKUSAYDON_ARENA == worldId ?
		ESTHER_ROSTER_KOUKUSAYDON : ESTHER_ROSTER_VALTAN;
	Reset();
}

// GameRoom fixed tick에서 호출하며 hasPlayers에는 인간 플레이어 존재 여부가 들어온다.
// 비활성/빈 방/잘못된 delta/가득 찬 상태면 변경하지 않는다. 빈 방 자체를 Reset으로 처리하지 않는다.
void LostArk::Server::CEstherSkillSystem::Update(
	const float fixedDeltaSeconds,
	const bool hasPlayers)
{
	if (!m_isEnabled || !hasPlayers ||
		!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f ||
		m_iGauge >= GAUGE_MAXIMUM)
	{
		return;
	}

	// 초당 충전량에 tick 초를 곱한다. 정수 부분만 게이지에 더하고 남은 소수는 다음 tick으로 넘긴다.
	m_fRegenRemainder += REGEN_PER_SECOND * fixedDeltaSeconds;
	const float wholePoints = std::floor(m_fRegenRemainder);
	m_fRegenRemainder -= wholePoints;
	const std::uint32_t gained = static_cast<std::uint32_t>(wholePoints);
	m_iGauge = (GAUGE_MAXIMUM - m_iGauge <= gained) ?
		GAUGE_MAXIMUM : m_iGauge + gained;
}

// 초기화/성공한 소비/발탄 빈 방 정리가 공유하는 게이지 초기화다. roster와 사용 가능 world는 유지한다.
void LostArk::Server::CEstherSkillSystem::Reset()
{
	m_iGauge = 0u;
	m_fRegenRemainder = 0.f;
}

// GameRoom::Handle_UseEstherSkill이 시전자 자격 검사 뒤 호출한다. 모든 거부 검사를 마칠 때까지 게이지와 출력은 변경하지 않는다.
// NONE은 게이지 소비 승인이다. 이후 지연 소환의 실제 생성/효과/복제는 GameRoom이 처리한다.
LostArk::Server::ESTHER_USE_REJECTION
LostArk::Server::CEstherSkillSystem::Try_Consume(
	const std::uint8_t slotIndex,
	const ESTHER_ROSTER_ENTRY*& outEntry)
{
	if (!m_isEnabled)
		return ESTHER_USE_REJECTION::DISABLED_WORLD;
	if (slotIndex < LostArk::Shared::MIN_ESTHER_SLOT_INDEX ||
		slotIndex > LostArk::Shared::MAX_ESTHER_SLOT_INDEX)
	{
		return ESTHER_USE_REJECTION::UNSUPPORTED_SLOT;
	}
	// wire slot의 범위를 확인한 뒤 최소 slot 번호를 빼 배열 index로 바꾼다. roster 정의의 ID가 누락된 경우도 거부한다.
	const ESTHER_ROSTER_ENTRY* pEntry = Find_EstherDefinition(
		m_Roster[slotIndex - LostArk::Shared::MIN_ESTHER_SLOT_INDEX]);
	if (nullptr == pEntry)
		return ESTHER_USE_REJECTION::UNSUPPORTED_SLOT;
	if (m_iGauge < GAUGE_MAXIMUM)
		return ESTHER_USE_REJECTION::GAUGE_NOT_FULL;

	Reset();
	outEntry = pEntry;
	return ESTHER_USE_REJECTION::NONE;
}

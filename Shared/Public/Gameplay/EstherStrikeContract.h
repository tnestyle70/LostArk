#pragma once

// Server 소환 상태와 Client NPC가 공유하는 ESTHER_ID 및 피해 원천 SKILL_ID 형식을 사용한다.
#include "Network/PacketMessages.h"

// 정적 타격 배열의 원소 수와 조회 index를 size_t로 표현한다.
#include <cstddef>
// ms·피해·피해 변화율·영역 구분의 고정 너비 정수 형식을 선언한다.
#include <cstdint>
// std::size로 타격 배열 길이를 계산하고 32비트 적용 mask에 들어가는지 정적으로 확인한다.
#include <iterator>

namespace LostArk::Shared::EstherStrike
{
	// 원본 근거: XMLData/Projectile/531000, 531100, 531300.loa가 참조하는 EFTable_SkillEffect 행이다.
	// GameRoom의 에스더 타격 소비가 아래 정적 정의를 읽고 경과 ms에 도달한 타격을 처리한다.
	inline constexpr std::uint8_t AREA_CIRCLE = 1u;
	inline constexpr std::uint8_t AREA_FORWARD_BOX = 2u;
	inline constexpr std::uint32_t DAMAGE_MULTIPLIER = 50u;

	// 시간은 소환 뒤 ms, 범위/폭/offset은 m다. BOX 소비자는 width의 절반을 halfWidth로 전달한다.
	// min/max 난수 피해에 DAMAGE_MULTIPLIER를 적용하는 곳은 Server의 실제 타격 소비 경로다.
	// Apply_EstherStrikeHits는 소환 yaw의 전방/오른쪽으로 offset을 옮긴 원점을 만든다.
	// CIRCLE의 range는 반경이고 FORWARD_BOX의 range는 전방 길이이며 width는 전체 너비다.
	struct HIT
	{
		std::uint32_t iTimeMs;
		std::uint8_t iAreaType;
		float fRangeM;
		float fWidthM;
		float fOffsetForwardM;
		float fOffsetRightM;
		std::uint32_t iDamageMin;
		std::uint32_t iDamageMax;
	};

	// 포인터는 아래 정적 테이블과 문자열을 차용한다. 정의와 타격 배열은 프로그램 수명 동안 유효하다.
	struct DEFINITION
	{
		ESTHER_ID eEstherId;
		const char* pArchetypeId;
		SKILL_ID iSkillId;
		const HIT* pHits;
		std::size_t iHitCount;
	};

	// Server는 각 배열 index를 소환 객체의 iEstherAppliedHitMask 한 bit에 대응시켜 이미 처리한 타격을 다시 적용하지 않는다.
	// 타격 시간은 배열에 저장하고 실제 경과 시간·대상 겹침·피해 처리는 룸이 소유한다.
	inline constexpr HIT SILLIAN_HITS[] = {
		{3100u, AREA_FORWARD_BOX, 20.f, 8.f, 1.2f, 0.f, 275625u, 281250u},
		{4200u, AREA_FORWARD_BOX, 20.f, 8.f, 1.2f, 0.f, 643125u, 656250u},
	};
	inline constexpr HIT WEI_HITS[] = {
		{1900u, AREA_CIRCLE, 4.f, 0.f, 2.5f, 1.f, 147000u, 150000u},
		{3100u, AREA_CIRCLE, 4.f, 0.f, 2.5f, -1.f, 147000u, 150000u},
		{4900u, AREA_CIRCLE, 4.f, 0.f, 3.f, 0.f, 220500u, 225000u},
		{5800u, AREA_CIRCLE, 5.5f, 0.f, 3.f, 0.f, 343000u, 350000u},
	};
	inline constexpr HIT NINAV_HITS[] = {
		{3500u, AREA_FORWARD_BOX, 20.f, 8.f, 1.2f, 0.f, 1453250u, 1482910u},
	};

	inline constexpr DEFINITION DEFINITIONS[] = {
		{ESTHER_ID::SILLIAN, "NPC_59030", 531000u, SILLIAN_HITS, std::size(SILLIAN_HITS)},
		{ESTHER_ID::WEI, "NPC_58700", 532000u, WEI_HITS, std::size(WEI_HITS)},
		{ESTHER_ID::NINAV, "NPC_59504", 534000u, NINAV_HITS, std::size(NINAV_HITS)},
	};

	static_assert(std::size(SILLIAN_HITS) <= 32u && std::size(WEI_HITS) <= 32u && std::size(NINAV_HITS) <= 32u);

	// 원본 근거: Projectile/531200.loa의 SkillEffect 531201 -> SkillBuff 555010(CombatEffect 5000, Key 34).
	// 부여 시점/지속 시간은 ms이며 iDamageTakenPercent는 피해 변화율이다. -50은 50% 감소다.
	struct GUARD
	{
		ESTHER_ID eEstherId;
		const char* pArchetypeId;
		std::uint32_t iGrantTimeMs;
		float fRadiusM;
		float fOffsetForwardM;
		std::uint32_t iDurationMs;
		std::int32_t iDamageTakenPercent;
	};

	inline constexpr GUARD GUARDS[] = {
		{ESTHER_ID::BAHUNTUR, "NPC_59060", 1000u, 7.f, 4.2f, 30000u, -50},
	};

	// 원본 근거: Projectile/531500.loa는 2초에 NPC 54050을 소환한다. aura 555030은 700cm/1000ms다.
	// 555032(Immune 20), 555034 -> SkillEffect 531513(광기 gauge 3708100, -10)을 사용한다.
	// 555031 ValueC 10000은 종료 시점, 555033의 3500은 해제 회복 입력이며 아래 제품 계약은 percent로 보관한다.
	struct ZONE
	{
		ESTHER_ID eEstherId;
		const char* pArchetypeId;
		std::uint32_t iStartMs;
		std::uint32_t iDurationMs;
		float fRadiusM;
		std::uint32_t iPulseMs;
		std::uint32_t iMadnessDrainPercent;
		std::uint32_t iReleaseHealPercent;
	};

	inline constexpr ZONE ZONES[] = {
		{ESTHER_ID::INANNA, "NPC_59620", 2000u, 10000u, 7.f, 1000u, 10u, 35u},
	};
	// ValtanBrain이 이 목록으로 타격의 bEstherGuardBlockable을 설정한다. 실제 보호 상태와 즉사 예외 판정은 ServerCombatHitRuntime이 처리한다.
	inline constexpr const char* GUARD_BLOCKED_DAMAGE_PROFILES[] = {
		"damage.valtan.omnidirectional-wipe-130",
		"damage.valtan.magic-orb-failure",
	};

	// 테이블 조회용 NUL 종료 byte 문자열 비교다. 둘 중 하나라도 nullptr면 false이며 메모리를 소유하지 않는다.
	// archetype/profile 조회가 전달하는 두 입력은 nullptr 또는 읽을 수 있는 NUL 종료 문자열이어야 한다. 대소문자나 공백을 정규화하지 않는다.
	[[nodiscard]] inline bool Same_Id(const char* left, const char* right) noexcept
	{
		if (nullptr == left || nullptr == right)
			return false;
		while (*left && *left == *right)
		{
			++left;
			++right;
		}
		return *left == *right;
	}

	// 아래 Find 함수는 작은 정적 배열을 선형 조회하고 const 포인터를 차용한다. 없으면 nullptr다.
	// 조회는 게이지 소비·소환·피해·보호 효과를 실행하지 않는다.
	// GameRoom Apply_EstherStrikeHits가 소환 ID의 보호 정의를 찾는다. 반환 포인터는 정적 테이블을 가리키며 플레이어 buff 상태와는 별개다.
	[[nodiscard]] inline const GUARD* Find_Guard(const ESTHER_ID estherId) noexcept
	{
		for (const GUARD& guard : GUARDS)
			if (guard.eEstherId == estherId)
				return &guard;
		return nullptr;
	}

	// Debug Client NPC가 archetype에 해당하는 보호 범위를 그릴 때 찾는다. 입력 문자열은 조회 중에만 차용하고 저장하지 않는다.
	[[nodiscard]] inline const GUARD* Find_GuardByArchetype(const char* pArchetypeId) noexcept
	{
		for (const GUARD& guard : GUARDS)
			if (Same_Id(guard.pArchetypeId, pArchetypeId))
				return &guard;
		return nullptr;
	}

	// GameRoom Apply_EstherStrikeHits가 소환 ID의 장판 정의를 찾는다. Open_EstherZone은 이 정적 정의를 차용하고 시각/위치만 별도로 보관한다.
	[[nodiscard]] inline const ZONE* Find_Zone(const ESTHER_ID estherId) noexcept
	{
		for (const ZONE& zone : ZONES)
			if (zone.eEstherId == estherId)
				return &zone;
		return nullptr;
	}

	// Debug Client NPC가 장판의 시작 시각·반경·지속 시간을 그릴 때 사용한다. 없으면 nullptr이며 Server 장판 상태를 생성하지 않는다.
	[[nodiscard]] inline const ZONE* Find_ZoneByArchetype(const char* pArchetypeId) noexcept
	{
		for (const ZONE& zone : ZONES)
			if (Same_Id(zone.pArchetypeId, pArchetypeId))
				return &zone;
		return nullptr;
	}

	// Valtan 명중 경로가 보호 효과로 막을 수 있는 특수 피해 profile인지 분류한다. 실제 buff 보유 검사는 호출자가 한다.
	[[nodiscard]] inline bool Is_GuardBlockedDamageProfile(const char* pDamageProfileId) noexcept
	{
		for (const char* blocked : GUARD_BLOCKED_DAMAGE_PROFILES)
			if (Same_Id(blocked, pDamageProfileId))
				return true;
		return false;
	}

	// GameRoom의 소환 타격 처리가 ID로 타격 배열을 찾는다. 보호/장판 전용 에스더는 이 타격 테이블에 없으므로 nullptr일 수 있다.
	[[nodiscard]] inline const DEFINITION* Find(const ESTHER_ID estherId) noexcept
	{
		for (const DEFINITION& definition : DEFINITIONS)
			if (definition.eEstherId == estherId)
				return &definition;
		return nullptr;
	}

	// Debug Client NPC가 archetype으로 타격 도형을 찾는다. 결과의 pHits와 pArchetypeId는 정적 저장소이므로 호출자 문자열 수명과 무관하다.
	[[nodiscard]] inline const DEFINITION* Find_ByArchetype(const char* pArchetypeId) noexcept
	{
		for (const DEFINITION& definition : DEFINITIONS)
			if (Same_Id(definition.pArchetypeId, pArchetypeId))
				return &definition;
		return nullptr;
	}
}

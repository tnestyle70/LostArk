#pragma once

#include "GameplayCatalog.h"
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"

#include <cstdint>
#include <vector>

namespace LostArk::Server
{
	struct SERVER_COLOSSEUM_RESOLVED_DAMAGE;
    struct SERVER_COLOSSEUM_COMBAT_CONTEXT;
	// 대상 거부, 흡수, 처리된 생존 타격, 사망을 구분한다. LANDED가 반드시 양수 HP 피해를 뜻하지는 않는다.
	enum class SERVER_COMBAT_HIT_RESULT : std::uint8_t
	{
		NOT_ADMITTED,
		ABSORBED,
		LANDED,
		KILLED
	};

	// 상위 스킬 피해와 보스 입력 경로가 같은 레이드 무력화 환산 대상을 판별한다.
	[[nodiscard]] inline bool Uses_RaidStaggerPolicy(const SERVER_WORLD_ENTITY& target) noexcept
	{
		return target.strArchetypeId == "BOSS_VALTAN" || target.strArchetypeId == "BOSS_VALTAN_GHOST" ||
			target.strArchetypeId.starts_with("BOSS_KOUKUSAYDON_");
	}

	// 플레이어 스킬/contact가 월드 entity에 보낼 한 타격의 계산 입력이다. 원점/밀기 거리는 m, 기간은 ms다.
	// 원형 도형이나 대상 목록을 담지 않아 이 구조만으로 새 명중 대상을 고르지는 않는다.
	struct SERVER_PLAYER_TO_WORLD_HIT final
	{
		LostArk::Shared::PLAYER_ID iSourcePlayerId =
			LostArk::Shared::INVALID_PLAYER_ID;
		LostArk::Shared::SKILL_ID iSkillId = LostArk::Shared::INVALID_SKILL_ID;
		std::uint32_t iRawDamage = 0u;
		std::uint32_t iStaggerDamage = 0u;
		std::uint32_t iStaggerMaximumDivisor = 0u;
		// HP와 무력화의 계산 기반을 유지하되 실제 적용 여부를 서로 독립적으로 제한한다.
		bool bHealthDamageDisabled = false;
		bool bStaggerDisabled = false;
		std::uint32_t iPartDamage = 0u;
		// Server가 확정한 파괴 폭탄 발사체 명중에서만 설정하는 정책 입력이다.
		bool bDestructionBombSource = false;
		std::uint32_t iCounterPower = 0u;
		// 승인된 스킬의 Catalog 슬롯으로 정한다. Client가 전달한 카운터 주장으로 취급하지 않는다.
		bool bCounterFromPrimarySlot = false;
		// 이번 타격의 치명타 표시다. iRawDamage에 배율이 이미 포함되어 있어 여기서 다시 곱하지 않는다.
		bool bCritical = false;
		float fSourceX = 0.f;
		float fSourceZ = 0.f;
		float fFallbackDirectionX = 0.f;
		float fFallbackDirectionZ = 1.f;
		float fPushRangeM = 0.f;
		std::uint32_t iPushMs = 0u;
		std::uint32_t iServerTick = 0u;
		// 보스 체력줄 등 이미 계산한 피해다. 갑옷 재계산을 건너뛰지만 보호막과 무적은 계속 적용한다.
		bool bHealthDamagePreResolved = false;
		// 안내 NPC의 공격은 HP 기여와 인간의 기믹/MVP 역할을 구분한다.
		bool bGuideSource = false;
	};

	// 보스/몬스터/전투 오브젝트가 플레이어에게 보낼 타격과 반응 입력이다.
	// 밀기 방향을 직접 지정하면 source 위치 대신 그 방향을 사용하고, 사망/보호 상태가 먼저 처리된다.
	struct SERVER_WORLD_TO_PLAYER_HIT final
	{
		LostArk::Shared::MARIO_HIT_SOURCE eMarioHitSource = LostArk::Shared::MARIO_HIT_SOURCE::NONE;
		std::uint32_t iRawDamage = 0u;
		float fSourceX = 0.f;
		float fSourceZ = 0.f;
		float fPushRangeM = 0.f;
		std::uint32_t iPushMs = 0u;
		bool bUsePushDirection = false;
		bool bForcePush = false;
		bool bPushCanLeaveArena = false;
		bool bPushBallistic = false;
		float fPushHeightM = 0.f;
		float fPushDirectionX = 0.f, fPushDirectionZ = 1.f;
		bool bKnockdown = false;
		std::uint32_t iDownMs = 0u;
		std::uint32_t iServerTick = 0u;
		// 저작된 판정 피해가 방어 계산과 카운터에 의해 바뀌지 않도록 각각 건너뛰는 옵션이다.
		// 이 두 옵션만으로 보호막/시간정지/다른 생존 효과까지 모두 우회하지는 않는다.
		bool bIgnoreDefense = false;
		bool bIgnoreCounter = false;
		// Server가 확정한 전멸 판정이다. 개인 생존 효과를 우회하지만 대상 참가 검사에서 안내 NPC는 제외한다.
		bool bEncounterWipe = false;
		bool bInstantDeath = false;
		bool bEstherGuardBlockable = false;
		// 정확히 이 보스/sequence가 손에 잡은 대상에게도 contact 피해를 허용할 때 사용하는 identity다.
		LostArk::Shared::NET_ENTITY_ID iCaptureOwnerId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint32_t iCapturePatternSequence = 0u;
	};

	// 현재 손 부착의 보스 ID/sequence/슬롯/기한을 검사한다. GRABBED라도 이 소유자와 다르면 피해 예외를 허용하지 않는다.
	[[nodiscard]] inline bool Is_CurrentBossHandCapture(const SERVER_PLAYER& player,
		const LostArk::Shared::NET_ENTITY_ID ownerId, const std::uint32_t patternSequence,
		const std::uint32_t serverTick) noexcept
	{
		using namespace LostArk::Shared;
		return ownerId != INVALID_NET_ENTITY_ID && patternSequence != 0u && player.iCurrentHp != 0u &&
			player.eAction == PLAYER_ACTION_STATE::GRABBED &&
			player.eAttachmentSlot == PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND &&
			player.iAttachmentOwnerNetEntityId == ownerId && player.iAttachmentPatternSequence == patternSequence &&
			player.iAttachmentEndTick != 0u && static_cast<std::int32_t>(serverTick - player.iAttachmentEndTick) < 0;
	}

	// Mario 장애물도 기존 밀기/넘어짐/착지 경로를 사용하도록 입력을 설정한다. 피해 적용이나 이동 자체는 하지 않는다.
	inline void Configure_MarioHazardLaunch(SERVER_WORLD_TO_PLAYER_HIT& hit)
	{
		if (hit.fPushRangeM <= 0.f || !hit.iPushMs)
			return;
		hit.bKnockdown = hit.bForcePush = hit.bPushBallistic = true;
		hit.bPushCanLeaveArena = false;
		hit.fPushHeightM = 2.f;
		if (hit.iDownMs < hit.iPushMs + 600u)
			hit.iDownMs = hit.iPushMs + 600u;
	}

	// 보유 buff를 공격/피격 경계에서 조회하는 공용 경로다. 보유자 vector는 호출자가 소유한다.
	class CServerBuffRuntime final
	{
	public:
		// GameRoom이 준비한 시전자/아군/적 목록에 스킬의 저작 buff를 적용한다.
		// 같은 buff ID는 중첩 행을 추가하지 않고 종료 tick을 연장하며 목록의 참가 범위는 호출자가 정한다.
		static void Apply_SkillBuffs(
			const CGameplayCatalog& catalog,
			std::uint32_t skillId,
			SERVER_PLAYER& caster,
			std::vector<SERVER_PLAYER*>& allies,
			std::vector<SERVER_WORLD_ENTITY*>& enemies,
			std::uint32_t serverTick);
        // PlayerSkillSystem이 승인된 콜로세움 상대의 명중 경로에서 호출한다. 이 함수도 상대 조건을 다시 확인한다.
        static void Apply_ColosseumEnemyBuffs(const CGameplayCatalog& catalog, std::uint32_t skillId,
            const SERVER_PLAYER& caster, SERVER_PLAYER& target, std::uint32_t serverTick,
            const SERVER_COLOSSEUM_COMBAT_CONTEXT& context);
		// GameRoom의 tick 처리가 종료 tick이 현재 tick 이하인 buff를 제거한다. 아래 수치 조회는 만료 여부를 다시 검사하지 않는다.
		static void Expire(
			std::vector<LostArk::Shared::ACTIVE_BUFF>& buffs,
			std::uint32_t serverTick);
		// Catalog에 남아 있는 보유 buff의 공격 피해 증감 percent를 합한다.
		[[nodiscard]] static std::int32_t Damage_DealtPercent(
			const CGameplayCatalog& catalog,
			const std::vector<LostArk::Shared::ACTIVE_BUFF>& buffs);
		// 피격 피해 증감 percent를 합한다. 음수는 감소를 뜻하며 이 함수 자체는 상태를 바꾸지 않는다.
		[[nodiscard]] static std::int32_t Damage_TakenPercent(
			const CGameplayCatalog& catalog,
			const std::vector<LostArk::Shared::ACTIVE_BUFF>& buffs);
		// 0 피해 또는 0 percent는 입력을 유지한다. 그 밖의 피해는 비율 적용 뒤 1~UINT32_MAX로 제한한다.
		[[nodiscard]] static std::uint32_t Scale_Damage(
			std::uint32_t damage, std::int32_t percent);
		// 보호막을 부여하는 buff가 하나도 없으면 현재 보호막을 0으로 정리한다.
		static void Settle_Shield(
			const CGameplayCatalog& catalog, SERVER_PLAYER& player);
	};

	// 플레이어→월드, 월드→플레이어와 일부 플레이어 간 전투가 피해/사망/반응 처리를 공유한다.
	// 도형 교차와 반복 contact의 허용 횟수는 호출자가 정하며, 이 클래스는 전체 호출의 rollback을 보장하지 않는다.
	class CServerCombatHitRuntime final
	{
	public:
		// 플레이어 공격을 받을 kind/archetype/생존 상태와 주 entity 여부를 검사한다. 도형은 검사하지 않는다.
		[[nodiscard]] static bool Is_PlayerDamageableWorldTarget(
			const SERVER_WORLD_ENTITY& target) noexcept;
		// Kouku 피해/기믹 입력이 광기를 누적한다. 소수 잔여량을 따로 보존하고 최대값에서 자른다.
		static void Add_MadnessGauge(SERVER_PLAYER& target, double gain);
		// 스킬/contact 입력을 보스 또는 일반 entity에 적용하고 피해 event를 출력 vector 뒤에 추가한다.
		// 보스의 부위/카운터/무력화는 CBossCombatRuntime에 맡기고 MVP·기존 갑옷 표시·사망 반응을 연결한다.
		static SERVER_COMBAT_HIT_RESULT Apply_PlayerToWorld(
			SERVER_WORLD_ENTITY& target,
			const SERVER_PLAYER_TO_WORLD_HIT& hit,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents);
		// Brain/contact/환경 피해의 보호 효과→피해→보호막→사망 방지→HP→반응 경로다.
		// pPvPResolved는 현재 호출 동안 차용하며, 이미 정산된 PvP 피해를 다시 방어/변동 계산하지 않게 구분한다.
		static SERVER_COMBAT_HIT_RESULT Apply_WorldToPlayer(
			SERVER_PLAYER& target,
			const SERVER_WORLD_TO_PLAYER_HIT& hit,
			const CGameplayCatalog& catalog,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
            const SERVER_COLOSSEUM_RESOLVED_DAMAGE* pPvPResolved = nullptr);
		// 같은 활성 경기에서 살아 있는 서로 다른 팀의 참가자인지 조회한다. 이 판별만으로 거리/도형을 승인하지 않는다.
		static bool Is_EnemyPlayer(const SERVER_PLAYER& source, const SERVER_PLAYER& target) noexcept;
		// 같은 경기의 적인지 확인하고 공용 플레이어 피해 경로로 전달한다.
		// 새로 추가된 event에 시전자 ID를 기록하고 사망이면 해당 killer ID를 남긴다.
		static SERVER_COMBAT_HIT_RESULT Apply_PlayerToPlayer(const SERVER_PLAYER& source, SERVER_PLAYER& target,
			const SERVER_PLAYER_TO_WORLD_HIT& hit, const CGameplayCatalog& catalog,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents);
	};
}

#pragma once

#include "GameplayCatalog.h"
#include "ServerCollisionSystem.h"
#include "ServerNavigation.h"
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"

#include <cstdint>
#include <vector>

namespace LostArk::Server
{
	struct SERVER_COLOSSEUM_COMBAT_CONTEXT;
	// 룸이 넘긴 플레이어·월드 개체 상태에 스킬 시작·단계 진행·투사체·피격 결과를 적용한다.
	// 지속 상태는 SERVER_PLAYER와 월드 개체에 남기며, 이 객체는 카탈로그·이동 검증을 연결하는 처리 책임을 맡는다.
	class CPlayerSkillSystem final
	{
	public:
        // CombatObject 투사체도 일반 skill과 같은 Colosseum 피해 정책을 적용한다.
        static void Apply_ColosseumObjectHit(SERVER_PLAYER& target, SERVER_PLAYER& caster,
            const PLAYER_SKILL_DEFINITION& skill, const CGameplayCatalog& catalog,
            const PLAYER_SKILL_HIT& hit, std::uint64_t wholeCastDamage, std::uint32_t subHitTotal,
            std::uint32_t subHitIndex, float sourceX, float sourceZ, std::uint32_t serverTick,
            const SERVER_COLOSSEUM_COMBAT_CONTEXT& context,
            std::vector<LostArk::Shared::DAMAGE_EVENT>& events);
		bool Try_Start(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_USE_SKILL& command,
			const CGameplayCatalog& catalog,
			std::uint32_t actionStartTick,
			const CServerNavigation* navigation = nullptr,
			LostArk::Shared::COOLDOWN_MODE cooldownMode = LostArk::Shared::COOLDOWN_MODE::RELEASE_AUTHORED) const;

		/* COMBO 중 다른 skill 입력은 sequence를 소비하고 하나의 대기 의도로 복사한다. 실제 시작 때 비용·cooldown을 다시 검사한다. */
		bool Try_StagePendingSkill(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_USE_SKILL& command,
			const CGameplayCatalog& catalog,
			const CServerNavigation* navigation = nullptr) const;

		bool Try_StartPending(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_USE_SKILL& command,
			const CGameplayCatalog& catalog,
			std::uint32_t actionStartTick,
			const CServerNavigation* navigation = nullptr,
			LostArk::Shared::COOLDOWN_MODE cooldownMode = LostArk::Shared::COOLDOWN_MODE::RELEASE_AUTHORED) const;

		static std::uint32_t Resolve_CooldownTicks(const PLAYER_SKILL_DEFINITION& skill,
			LostArk::Shared::COOLDOWN_MODE mode);
		static void Recalculate_Cooldowns(SERVER_PLAYER& player, const CGameplayCatalog& catalog,
			std::uint32_t serverTick, LostArk::Shared::COOLDOWN_MODE mode);

		void Release(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_RELEASE_SKILL& command,
			const CGameplayCatalog& catalog);

		/* HOLD 충전 1·2단계에서 키를 놓기 전까지 조준을 갱신한다. 발사 단계에서는 마지막 방향을 유지한다. */
		void Update_Aim(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_UPDATE_SKILL_AIM& command,
			const CGameplayCatalog& catalog) const;

		/* COUNTER 방어 window의 피격을 흡수하고 counter 단계로 넘긴다. true를 받으면 호출자는 해당 피해 적용을 생략해야 한다. */
		[[nodiscard]] static bool Try_Counter(
			SERVER_PLAYER& player,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick);

		/* 피해 적용 뒤 밀기·당기기와 선택적 KNOCKDOWN을 시작한다. 일반 반응은 기존 피격·면역 구간을 존중하며 유효한 forcePush는 별도 상태 조건으로 기존 반응을 교체한다. */
		static bool Can_ArmPlayerHitReaction(const SERVER_PLAYER& player, std::uint32_t serverTick, bool forcePush = false);
		static void Arm_PlayerHitReaction(
			SERVER_PLAYER& player,
			float sourceX,
			float sourceZ,
			float pushRangeM,
			std::uint32_t pushMs,
			bool knockdown,
			std::uint32_t downMs,
			std::uint32_t serverTick,
			bool forcePush = false, bool pushCanLeaveArena = false, bool pushBallistic = false,
			float pushHeightM = 0.f);

		/* 자세 전환 skill은 자연 종료·이동 취소·skill 취소 모두 같은 자세 변경을 commit한다. 자세 변경이 없는 skill은 처리하지 않는다. */
		static void Commit_StanceChange(
			SERVER_PLAYER& player,
			const PLAYER_SKILL_DEFINITION& skill,
			const CGameplayCatalog& catalog);

		/* 지속 소모율이 있고 기본 자세와 다를 때 true다. 전환 순간에만 비용을 내는 class는 포함하지 않는다. */
		[[nodiscard]] static bool Is_HoldingGaugedStance(
			const SERVER_PLAYER& player,
			const PLAYER_RUNTIME_PROFILE& profile);

		/* 행동과 별개로 매 tick 자세 유지 소모·회복·순환 gauge를 갱신한다. */
		static void Update_Identity(
			SERVER_PLAYER& player,
			const PLAYER_RUNTIME_PROFILE& profile);

		/* 가디언 identity는 자연 회복 대신 명중으로 채운다. 용 자세는 profile 기간 동안 소비하고 0이 되면 기본 자세로 돌아간다. */
		static void Update_EmberGauge(
			SERVER_PLAYER& player,
			const PLAYER_RUNTIME_PROFILE& profile,
			const GUARDIAN_EMBER_PROFILE& ember);

		/* 생성·부활·class 변경의 gauge 초기값이다. ember class는 identity 0·최대 orb·잠금 0, 그 외 class는 identity 최대값으로 시작한다. */
		static void Reset_Gauges(
			SERVER_PLAYER& player,
			const CGameplayCatalog& catalog);

		/* 기본 자세에서 명중하면 profile의 회복량을 identity 최대값 안에서 더한다. */
		static void Gain_EmberGauge(
			SERVER_PLAYER& player,
			const CGameplayCatalog& catalog);

		/* 이번 호출의 hit 결과를 room 소유 vector에 추가해 같은 tick의 snapshot으로 보낸다. */
		void Update(
			SERVER_PLAYER& player,
			std::vector<SERVER_WORLD_ENTITY>& worldEntities,
			const CGameplayCatalog& catalog,
			const CServerNavigation* navigation,
			const CServerCollisionSystem* collision,
			float fixedDeltaSeconds,
			std::uint32_t serverTick,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
			const SERVER_COLOSSEUM_COMBAT_CONTEXT* pPvPContext = nullptr) const;

		/* 시전 action이 끝나도 남은 투사체의 이동·접촉·시간 타격을 진행한다. 거리·수명이 끝난 객체는 제거한다. */
		static void Update_Projectiles(
			SERVER_PLAYER& player,
			std::vector<SERVER_WORLD_ENTITY>& worldEntities,
			const CGameplayCatalog& catalog,
			float fixedDeltaSeconds,
			std::uint32_t serverTick,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
			const SERVER_COLOSSEUM_COMBAT_CONTEXT* pPvPContext = nullptr);

		/* 이번 이동량 중 실제 도달 가능한 지점까지만 구한다. 첫 표본부터 막히면 시작점이며 startY로 선택한 높이 layer의 지면을 따라간다. */
		static void Clamp_StepToWalkable(
			const CServerNavigation& navigation,
			float startX,
			float startZ,
			float desiredX,
			float desiredZ,
			SERVER_NAV_POINT& outPoint,
			bool& outWasClamped,
			float startY = NAVIGATION_HEIGHT_UNKNOWN);

	private:
		/* 성공한 skill 시작에서 보유 ember만큼 비용을 내고 인간형 expression skill의 socket을 잠근 뒤 보충한다. 다른 class는 변경하지 않는다. */
		static void Apply_EmberOnStart(
			SERVER_PLAYER& player,
			const PLAYER_SKILL_DEFINITION& skill,
			const CGameplayCatalog& catalog);
		bool Try_StartInternal(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_USE_SKILL& command,
			const CGameplayCatalog& catalog,
			std::uint32_t actionStartTick,
			bool sequenceAlreadyConsumed,
			const CServerNavigation* navigation,
			LostArk::Shared::COOLDOWN_MODE cooldownMode) const;
	};
}

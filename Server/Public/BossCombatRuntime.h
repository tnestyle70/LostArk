#pragma once

#include "GameplayCatalog.h"
#include "Network/NetworkIds.h"

#include <cstdint>
#include <string>
#include <vector>

namespace LostArk::Server
{
	struct SERVER_WORLD_ENTITY;

	// 동시에 여러 전투 상태를 켤 수 있는 비트 집합이다. snapshot은 iFlags 값으로 상태를 전달한다.
	enum class SERVER_BOSS_COMBAT_FLAG : std::uint32_t
	{
		INVULNERABLE = 1u << 0u,
		SHIELDED = 1u << 1u,
		COUNTERABLE = 1u << 2u,
		GROGGY = 1u << 3u,
		GHOST_HIDDEN = 1u << 4u
	};

	// 불변 BOSS_PART_DEFINITION에서 생성한 부위별 실행 내구도다. state mask는 한 비트이며 부위끼리 겹치지 않는다.
	struct SERVER_BOSS_PART_STATE final
	{
		std::string strPartId;
		std::uint32_t iStateMask = 0u;
		std::uint32_t iCurrentDurability = 0u;
		std::uint32_t iMaximumDurability = 0u;
		std::uint32_t iDamageReductionPercent = 0u;
		BOSS_PART_DAMAGE_CONDITION eDamageCondition =
			BOSS_PART_DAMAGE_CONDITION::ALWAYS;
	};

	// 명중 경로가 만든 결과를 Brain의 다음 분기 처리로 넘긴다. pattern/action/sequence로 이전 실행의 신호와 구분한다.
	struct SERVER_BOSS_PATTERN_OUTCOME_SIGNAL final
	{
		std::string strPatternId;
		std::string strActionId;
		std::uint32_t iPatternSequence = 0u;
		std::uint32_t iServerTick = 0u;
		BOSS_PATTERN_STAGE_OUTCOME eOutcome =
			BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT;
	};

	// 부위 파괴는 지속 상태와 일회 표시 event를 따로 남긴다.
	// GameRoom은 방 전체 event sequence를 붙인 뒤 한 tick의 제한된 출력 vector에 옮긴다.
	// 출력이 가득 차면 아직 옮기지 않은 항목은 이 대기열에 남긴다.
	struct SERVER_BOSS_PART_BREAK_EDGE final
	{
		std::uint32_t iPartMask = 0u;
		std::uint32_t iServerTick = 0u;
		std::uint32_t iStateRevision = 0u;
	};

	// entity가 소유하는 보스 전투 상태다. HP는 entity에 있고 이 구조는 부위/무력화/보호막과 결과 대기열을 맡는다.
	// 각 vector의 요소를 차용한 참조는 삽입/삭제 이후까지 보관하지 않는다. 내부 동기화는 하지 않는다.
	struct SERVER_BOSS_COMBAT_STATE final
	{
		std::uint32_t iStateRevision = 0u;
		std::uint32_t iFlags = 0u;
		std::uint32_t iAlivePartMask = 0u;
		std::uint32_t iStaggerCurrent = 0u;
		std::uint32_t iStaggerMaximum = 0u;
		std::uint32_t iShieldCurrent = 0u;
		std::uint32_t iShieldMaximum = 0u;
		std::vector<SERVER_BOSS_PART_STATE> Parts;
		std::vector<SERVER_BOSS_PATTERN_OUTCOME_SIGNAL> PendingOutcomes;
		std::vector<SERVER_BOSS_PART_BREAK_EDGE> PendingPartBreakEdges;
	};

	// 상위 피해 경로 또는 Brain의 무피해 counter proxy가 전달하는 한 번의 입력이다.
	// HP 피해, 무력화, 부위 피해와 카운터 세기는 독립 채널이며 source XZ는 월드 m다.
	struct BOSS_INCOMING_HIT final
	{
		LostArk::Shared::PLAYER_ID iSourcePlayerId =
			LostArk::Shared::INVALID_PLAYER_ID;
		LostArk::Shared::SKILL_ID iSkillId =
			LostArk::Shared::INVALID_SKILL_ID;
		std::uint32_t iRawDamage = 0u;
		std::uint32_t iStaggerDamage = 0u;
		std::uint32_t iPartDamage = 0u;
		// Server 피해 정책이 허용된 모든 부위를 한 타격으로 파괴할 때만 설정한다.
		bool bDestroyAllEligibleParts = false;
		std::uint32_t iCounterPower = 0u;
		bool bCounterFromPrimarySlot = false;
		// Catalog의 COUNTER guard 창에서 실제 stage 승인이 끝난 입력임을 구분한다.
		bool bCounterFromActiveGuard = false;
		std::uint32_t iServerTick = 0u;
		// 상위 CServerCombatHitRuntime이 기존 갑옷 방어나 피해 정책을 이미 적용했는지 나타낸다.
		// true이면 typed 부위의 피해 감소를 다시 적용하지 않지만 보호막/HP/무력화/카운터/부위 처리는 계속한다.
		bool bHealthDamagePreResolved = false;
		float fSourceX = 0.f;
		float fSourceZ = 0.f;
	};

	// 적용한 피해량과 상태 변화 결과다. 무적이어도 무력화/카운터/부위 채널은 별도 조건에 따라 처리될 수 있다.
	// strDestroyedPartId는 첫 파괴 부위이고 iDestroyedPartMask는 이번에 파괴된 전체 부위 집합이다.
	struct BOSS_HIT_RESULT final
	{
		std::uint32_t iHealthDamage = 0u;
		std::uint32_t iShieldDamage = 0u;
		std::uint32_t iStaggerDamage = 0u;
		std::uint32_t iPartDamage = 0u;
		std::uint32_t iDestroyedPartMask = 0u;
		std::uint32_t iAlivePartMask = 0u;
		std::string strDestroyedPartId;
		bool bBlockedByInvulnerability = false;
		bool bHealthDamageThresholdReached = false;
		bool bCounterTriggered = false;
		bool bStaggerBroken = false;
		bool bPartDestroyed = false;
	};

	// GameRoom의 entity 생성/단계 액션, 상위 피해 경로와 Brain이 공유하는 보스 전투 상태 변경 경계다.
	// 전달된 상태를 동기적으로 변경하며 전체 피해 처리에 대한 예외 rollback을 제공하지 않는다.
	class CBossCombatRuntime final
	{
	public:
		// GameRoom의 생성 경로가 부위 정의를 검증하고 임시 상태에 정렬한 뒤 교체한다.
		// 검증 false이면 기존 state는 유지하고 status만 실패 사유로 갱신한다.
		static bool Initialize(
			SERVER_BOSS_COMBAT_STATE& state,
			const std::vector<BOSS_PART_DEFINITION>& definitions,
			std::string& status);
		// 피해 감소→보호막/HP→누적 피해 응답→카운터→무력화→부위 순서로 입력을 처리한다.
		// 보스가 아니거나 이미 HP가 0이면 피해 없이 현재 부위 mask만 돌려준다.
		static BOSS_HIT_RESULT Apply_PlayerHit(
			SERVER_WORLD_ENTITY& boss,
			const BOSS_INCOMING_HIT& hit);
		// 카운터 결과를 게시하고 해당 COUNTERABLE 창을 닫는다. 공간 범위 검사는 호출 경로의 책임이다.
		// 현재 보스에 적용할 수 없으면 살아 있는 Kouku 유지 소유자도 조회한다.
		static bool Try_TriggerCounter(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		// 현재 실행 identity가 있어야 신호를 추가한다. 같은 pattern/action/sequence/outcome 중복은 추가 없이 true다.
		static bool Publish_PatternOutcome(
			SERVER_WORLD_ENTITY& boss,
			BOSS_PATTERN_STAGE_OUTCOME outcome,
			std::uint32_t serverTick);
		// Brain이 현재 pattern/sequence와 요청 action/outcome에 맞는 첫 신호를 지운다. 없으면 false다.
		static bool Consume_PatternOutcome(
			SERVER_WORLD_ENTITY& boss,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_OUTCOME outcome);
		// stage 전환이 지난 pattern/sequence 신호와 지정한 이전 action의 신호를 함께 제거한다.
		static void Discard_PatternOutcomes(
			SERVER_WORLD_ENTITY& boss,
			const std::string& actionId);
		// 패턴 종료/logic 정리가 모든 결과 신호를 비운다. 부위 파괴 표시 대기열은 별도라 유지한다.
		static void Clear_PatternOutcomes(SERVER_WORLD_ENTITY& boss);
		// 입력 mask의 비트가 하나라도 켜져 있는지 읽는다. 상태나 revision은 바꾸지 않는다.
		static bool Has_Flag(
			const SERVER_BOSS_COMBAT_STATE& state,
			SERVER_BOSS_COMBAT_FLAG flag) noexcept;
		// 지정 비트를 켜거나 끈다. 실제 flags가 바뀐 경우에만 revision을 올리고 true를 반환한다.
		static bool Set_Flag(
			SERVER_BOSS_COMBAT_STATE& state,
			SERVER_BOSS_COMBAT_FLAG flag,
			bool enabled) noexcept;
		// 새 무력화 창은 누적값 0에서 시작한다. maximum 0은 창을 닫는다.
		// 현재값/최대값을 한 함수에서 갱신해 stage action이 일관된 snapshot을 만들도록 한다.
		static bool Set_StaggerGauge(
			SERVER_BOSS_COMBAT_STATE& state,
			std::uint32_t maximum) noexcept;
		// 최대값만큼 가득 찬 보호막을 연다. 0은 현재값/최대값/SHIELDED를 함께 지운다.
		static bool Set_Shield(
			SERVER_BOSS_COMBAT_STATE& state,
			std::uint32_t maximum) noexcept;
		// phase는 entity에 있지만 BOSS_COMBAT_SNAPSHOT에 포함되므로 전투 revision도 함께 갱신한다.
		// 같은 revision이 다른 내용을 의미하지 않도록 직접 대입 대신 이 경계를 사용한다.
		static bool Set_GameplayPhase(
			SERVER_WORLD_ENTITY& boss,
			std::uint8_t phase) noexcept;
	};
}

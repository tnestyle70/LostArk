#pragma once

#include "GameplayCatalog.h"
#include "ServerNavigation.h"
#include "ServerCollisionSystem.h"
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"

#include <map>

namespace LostArk::Server
{
	// GameRoom이 일반 MONSTER entity를 갱신할 때 사용하는 상태 없는 처리기다.
	// 위치/대상/경로/공격 진행 상태는 전달받은 entity가 소유하며 피해는 공용 hit runtime에 맡긴다.
	class CMonsterBrain final
	{
	public:
		// 별도 충돌 저장소가 없는 호출을 위한 overload다. 빈 충돌 시스템으로 공용 구현을 호출한다.
		void Update(SERVER_WORLD_ENTITY& monster,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const CGameplayCatalog& catalog,
			const CServerNavigation& navigation,
			float fixedDeltaSeconds,
			std::uint32_t serverTick,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents) const;
		// GameRoom의 제품 경로다. navigation과 충돌을 함께 검사하고 위치/행동/공격 결과를 갱신한다.
		// fixedDeltaSeconds는 초이며 잘못된 delta이면 상태를 변경하지 않는다. players와 출력 vector는 호출 중 차용한다.
		void Update(SERVER_WORLD_ENTITY& monster,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const CGameplayCatalog& catalog,
			const CServerNavigation& navigation,
			const CServerCollisionSystem& collision,
			float fixedDeltaSeconds,
			std::uint32_t serverTick,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents) const;

		// GameRoom이 일반 Update보다 먼저 실행하는 피격 밀림 한 tick이다.
		// 살아 있는 몬스터의 저장 방향으로 이동하고 navigation/충돌 결과에 맞춰 남은 시간을 줄인다.
		// true이면 이번 tick을 밀림 경로가 소비했으므로 호출자는 추적/공격 Update를 건너뛴다.
		// 막히면 남은 밀림은 닫지만 action의 경과 시간은 진행한다.
		static bool Advance_Knockback(
			SERVER_WORLD_ENTITY& monster,
			const CServerNavigation& navigation,
			float fixedDeltaSeconds);
		static bool Advance_Knockback(
			SERVER_WORLD_ENTITY& monster,
			const CServerNavigation& navigation,
			const CServerCollisionSystem& collision,
			float fixedDeltaSeconds);
	};
}

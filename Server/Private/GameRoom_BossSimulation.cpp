#include "GameRoom.h"

#include <cstdint>
#include <limits>

//룸 worker의 고정 tick 진입점. 순회 중 entity 저장소를 늘리거나 지우지 않고
//생성·제거를 앞뒤 경계에서 확정한다. 실패한 하위 갱신 이후 단계는 실행하지 않는다.
void LostArk::Server::CGameRoom::Update_WorldEntities(const float fixedDeltaSeconds)
{
    const std::uint32_t updateTick =
        (std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ? 1u : m_iServerTick + 1u;

    //플레이어 진입·이전 패턴 잔여 객체를 먼저 확정한 뒤 본체 순회
    // Idempotent for direct simulation callers; normal ticks already prepared before players.
    Prepare_KoukuAuditionTick(updateTick);
    // Commit portal contact before child attacks mutate the contacting player.
    // This boundary is outside entity iteration because entry may spawn stage objects.
    Commit_KoukuMarioEntries();
    Update_KoukuPatternTails(updateTick);
    Update_KoukuGazeClones(updateTick);

    if (!Update_DependentBosses(updateTick))
    {
        Mark_RuntimeFailure("world-update.dependent-bosses-before-primary");
        return;
    }

    Update_PendingEstherSummons(fixedDeltaSeconds);
    Update_EstherZones(updateTick);

    //차용한 entity 참조는 이번 호출 동안만 사용. vector 순서와 조기 실패 유지
    for (SERVER_WORLD_ENTITY& entity : m_WorldEntities)
    {
        if (!Update_WorldEntity(entity, fixedDeltaSeconds, updateTick))
        {
            return;
        }
    }

    Tick_MvpLedgers();
    Grant_ValtanRaidClearRewards();

    if (!Despawn_FinishedWorldEntities(updateTick))
    {
        return;
    }

    //순회 종료 뒤 순간이동·분신 생성 확정. 생성으로 무효화될 참조를 보관하지 않는다.
    Commit_KoukuMechanicTriggers(updateTick);

    if (!Update_DependentBosses(updateTick))
    {
        Mark_RuntimeFailure("world-update.dependent-bosses-after-primary");
    }
}

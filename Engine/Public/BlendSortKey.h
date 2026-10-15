#pragma once
// 수신 객체의 blend 우선순위를 보관할 고정 폭 부호 있는 정수다.
#include <cstdint>

namespace Engine
{
    // BLEND 표시 순서는 우선순위 오름차순, 같은 우선순위에서는 먼 거리부터다.
    // 비유한 거리의 대체값은 호출자가 먼저 정한다.
    struct BLEND_SORT_KEY final
    {
        std::int32_t priority = 0;
        float distanceSquared = 0.f;
    };

    // Renderer의 stable_sort가 호출한다. 우선순위가 작으면 먼저, 같으면 거리 제곱이 큰 객체부터 그린다.
    // 두 키가 같으면 false이며 제출 순서 보존은 comparator 자체가 아니라 stable_sort의 책임이다.
    inline bool BlendSortBefore(const BLEND_SORT_KEY& lhs, const BLEND_SORT_KEY& rhs)
    {
        if (lhs.priority != rhs.priority)
            return lhs.priority < rhs.priority;
        return lhs.distanceSquared > rhs.distanceSquared;
    }
}

#pragma once
#include <cstdint>

namespace Engine
{
    // BLEND draw-order key. Lower priority draws first; equal priorities keep
    // the existing far-to-near order. Callers normalize non-finite distances.
    struct BLEND_SORT_KEY final
    {
        std::int32_t priority = 0;
        float distanceSquared = 0.f;
    };

    inline bool BlendSortBefore(const BLEND_SORT_KEY& lhs, const BLEND_SORT_KEY& rhs)
    {
        if (lhs.priority != rhs.priority) return lhs.priority < rhs.priority;
        return lhs.distanceSquared > rhs.distanceSquared;
    }
}

#pragma once

#include "EffectResourceCatalog.h"

#include <array>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace Client
{
// The Arena preview and authoring library share the exact source sequence routes.
// Effect IDs are read from those sequences, never inferred from asset names.
struct VALTAN_SOURCE_CINEMATIC_ROUTE final
{
    std::string_view patternId, suffix, firstStageId;
    uint32_t stageOffsetMs;
    std::u8string_view displayName;
};
inline constexpr std::array<VALTAN_SOURCE_CINEMATIC_ROUTE, 5u> VALTAN_SOURCE_CINEMATIC_ROUTES = {{
    { "VALTAN_ENTRANCE_CINEMATIC", "entrance", "ESTABLISH", 0u, u8"\uc9c4\uc785 / Entrance" },
    { "VALTAN_ARENA_BREAK_109", "phase2", "IMPACT_HOLD", 600u, u8"2\ud398\uc774\uc988 / Phase 2" },
    { "VALTAN_SIX_PIZZA_106", "roar", "STEP_04", 0u, u8"\ud53c\uc790 / Roar" },
    { "VALTAN_TRASH", "trash", "STEP_05", 0u, u8"\ubc84\ub7ec\uc9c0 / Trash" },
    { "VALTAN_GHOST_DEATH_AUDITION", "finale", "STEP_01", 0u, u8"\uc0ac\ub9dd / Finale" },
}};

struct VALTAN_CINEMATIC_EFFECT_OCCURRENCE final
{
    std::string strTrackId;
    EFFECT_RESOURCE_KEY Key;
    uint32_t iStartMs = 0u, iDurationMs = 0u;
    bool bLoopEffectToDuration = false, bFitEffectToDuration = false;
};
struct VALTAN_CINEMATIC_EFFECT_GROUP final
{
    std::string strPatternId, strInstanceId, strSequenceId, strDisplayName;
    std::string strSourceDisplayName;
    uint32_t iDurationMs = 0u;
    std::vector<VALTAN_CINEMATIC_EFFECT_OCCURRENCE> Effects;
};
struct VALTAN_CINEMATIC_EFFECT_LIBRARY final
{
    std::filesystem::path SourcePath;
    std::vector<VALTAN_CINEMATIC_EFFECT_GROUP> Groups;
};

// Read-only authoring projection. The Arena still consumes its published Map
// document. Parse/validation failure leaves the caller's previous library intact.
bool Load_ValtanCinematicEffectLibrary(VALTAN_CINEMATIC_EFFECT_LIBRARY& output,
    std::string& status);
}

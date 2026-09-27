#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include <functional>
#include <string>
#include <vector>

namespace Client
{
// Read-only projection of the existing Movie owner; no editor owns another clock.
struct CLASS_MOVIE_WORLD_ITEM final
{
    std::string id, instanceId, slotId, objectId, label, modelAssetId;
    std::vector<std::string> materials;
    float3_t position{};
    bool sampled = false, authoredVisible = false, drawn = false;
    bool selected = false, solo = false, muted = false, excluded = false;
    uint32_t meshCount = 0u;
};
struct CLASS_MOVIE_INSPECTION_STATE final
{
    std::string classId, selectedId, status;
    bool loop = false, active = false, freeCamera = false, pickArmed = false;
    bool showBackground = true, showEffects = true, dirty = false;
    double movieMs = 0., sourceMs = 0.;
    float3_t cameraPosition{};
    uint32_t pickedMesh = UINT32_MAX;
    std::vector<CLASS_MOVIE_WORLD_ITEM> items;
};
enum class CLASS_MOVIE_INSPECTION_ACTION
{
    SELECT, SOLO, MUTE, EXCLUDE, CLEAR_PREVIEW, FREE_CAMERA, FOCUS,
    PICK_IN_SCENE, BACKGROUND, EFFECTS, SAVE, RELOAD
};
struct CLASS_MOVIE_INSPECTION_COMMAND final
{
    CLASS_MOVIE_INSPECTION_ACTION action = CLASS_MOVIE_INSPECTION_ACTION::SELECT;
    std::string itemId;
    bool enabled = true;
};
struct CLASS_MOVIE_INSPECTION_CALLBACKS final
{
    std::function<CLASS_MOVIE_INSPECTION_STATE(const std::string&, bool)> state;
    std::function<bool(const std::string&, bool, const CLASS_MOVIE_INSPECTION_COMMAND&, std::string&)> command;
};
}

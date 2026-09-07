#pragma once

#include "EffectV2_Document.h"
#include "WorldSequenceDocument.h"

namespace Client
{
struct EFFECT_COMPOSITION_WORLD_MOTION final
{
    std::string objectId, instanceId, templateId, displayName, slotId;
    std::uint32_t durationMs = 0u, startDelayMs = 0u;
    float playbackSpeed = 1.f;
    std::vector<WORLD_SEQUENCE_ANIMATION_TRACK> animationTracks;
    WORLD_SEQUENCE_OBJECT_MOTION objectMotion;
    std::vector<WORLD_SEQUENCE_TRACK> tracks;
    std::string anchorKind = "WORLD";
    float3_t position = {};
    WORLD_SEQUENCE_MOTION_END motionEnd = WORLD_SEQUENCE_MOTION_END::STOP;
    std::string nextMotionId, error;
};

struct EFFECT_COMPOSITION_WORLD_RESOURCE final
{
    WORLD_SEQUENCE_OBJECT_RESOURCE resource;
    std::vector<EFFECT_COMPOSITION_WORLD_MOTION> motions;
    std::string error;
};

// Reads only saved Object/Template/Instance fields. No placement admission,
// publisher, GPU model load or World Object writer participates.
bool Read_EffectCompositionWorldResources(const std::string& areaId,
    std::vector<EFFECT_COMPOSITION_WORLD_RESOURCE>& resources,
    std::uint32_t& sourceRevision, std::string& status);

// Applies supported fields to CPU drafts transactionally. IDs and unrelated
// Effect look settings are retained; unsupported source behavior is reported.
bool Apply_WorldMotionToEffect(const EFFECT_COMPOSITION_WORLD_RESOURCE& resource,
    const EFFECT_COMPOSITION_WORLD_MOTION& motion, EFFECT_V2_DOCUMENT& effect,
    EFFECT_V2_GROUP_CHILD& child, std::string& status);
}

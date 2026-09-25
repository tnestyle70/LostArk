#pragma once

#include "EffectRecoveryCamera.h"
#include <memory>
#include <optional>

namespace Client
{
// Read-only projection of the admitted movie. Movie milliseconds include the
// source time-dilation curve; source milliseconds address the original tracks.
struct CLASS_MOVIE_TIMELINE_BOX final
{
    std::string id, label, resource;
    double movieStartMs = 0., movieEndMs = 0., sourceStartMs = 0., sourceEndMs = 0.;
    double playbackRate = 1., sourceOffsetMs = 0.;
    std::vector<double> keyMovieTimes;
    std::optional<EFFECT_CAMERA_ROW> camera;
};
struct CLASS_MOVIE_TIMELINE_ROW final
{
    std::string kind, id, label;
    std::vector<CLASS_MOVIE_TIMELINE_BOX> boxes;
};
struct CLASS_MOVIE_TIMELINE final
{
    std::string classId;
    bool loop = false;
    double movieDurationMs = 0., sourceDurationMs = 0.;
    std::vector<CLASS_MOVIE_TIMELINE_ROW> rows;
};
struct CLASS_MOVIE_CAMERA_SAMPLE final
{
    bool valid = false;
    std::string rowId;
    double movieMs = 0., sourceMs = 0.;
    float aspect = 1.f;
    VALTAN_CINEMATIC_CAMERA_POSE pose;
    float3_t up{0.f, 1.f, 0.f};
};
}

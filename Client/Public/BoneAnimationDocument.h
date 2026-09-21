#pragma once
#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "BinaryAsset/ModelAssetData.h"
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace Engine { class CModel; }
namespace Client
{
// All transforms are bone-local deltas in the installed skeleton's units.
// They never rewrite the extracted WModel or its native clip channels.
struct BONE_ANIMATION_KEY final
{
    uint32_t timeMs = 0u;
    float3_t position{};
    float4_t rotation{0.f, 0.f, 0.f, 1.f};
    float3_t scale{1.f, 1.f, 1.f};
};
struct BONE_ANIMATION_TRACK final
{
    std::string bone;
    std::vector<BONE_ANIMATION_KEY> keys;
};
struct BONE_ANIMATION_SEGMENT final
{
    std::string id, sourceClip;
    uint32_t durationMs = 1000u, sourceStartMs = 0u;
    float playRate = 1.f;
    bool loop = false;
};
struct BONE_ANIMATION_CLIP final
{
    std::string name;
    uint32_t durationMs = 1000u;
    std::vector<BONE_ANIMATION_SEGMENT> segments;
    std::vector<BONE_ANIMATION_TRACK> tracks;
};
class CBoneAnimationDocument final
{
public:
    std::string asset;
    uint64_t skeletonHash = 0u;
    std::vector<BONE_ANIMATION_CLIP> clips;
    static std::filesystem::path Path(const std::string& asset);
    static bool Load_IntoModel(Engine::CModel& model, const std::string& asset, std::string& status);
    static bool Load_PublishedValtan(Engine::CModel& model, std::string& status);
    bool Load(const std::string& asset, const Engine::CModel& model, std::string& status);
    bool Parse(const std::string& text, std::string& status);
    std::string Serialize() const;
    bool Compile(const Engine::CModel& model, std::vector<Engine::MODEL_ANIMATION_DATA>& output, std::string& status) const;
    bool Install(Engine::CModel& model, std::string& status) const;
    bool Save(Engine::CModel& model, std::string& status);
    bool Sample(const Engine::CModel& model, const std::string& clip, float timeMs,
        std::vector<float4x4_t>& pose, std::string& status) const;
private:
    std::string m_Baseline;
    bool m_Existed = false;
};
}

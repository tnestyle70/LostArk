#include <WinSock2.h>
#include "EffectFailureDiagnostic.h"
#include "imgui.h"
#include "Engine_RenderTypes.h"
#include "KoukuSaydonPresentationPlayer.h"
#include <random>
#include "KoukuSaydonAnimationBlend.h"
#include "KoukuSaydonPresentationAssetService.h"
#include "Gameplay/KoukuTargetTracking.h"

#include "ActionPresentationTimeline.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "DataJson.h"
#include "EffectV2_Catalog.h"
#include "Effect_PresentationService.h"
#include "Effect_Catalog.h"
#include "EffectCompositionModelPreview.h"
#include "Effect_Playback.h"
#include "NetworkManager.h"
#include "EffectV2_Object.h"
#include "EffectV2_Runtime.h"
#include "GameInstance.h"
#include "CombatHUDViewModel.h"
#include "Level_KakulSaydonArena.h"
#include "LightResourceCatalog.h"
#include "MapAssetCatalog.h"
#include "Presentation_Manager.h"
#include "Model.h"
#include "Npc.h"
#include "WorldSequencePlayer.h"
#include "WorldGameplayDocument.h"
#include "ProjectDataRoot.h"
#include "RenderingProfileService.h"
#include "RuntimeAssetRoot.h"
#include "SoundCueCatalog.h"
#include "Transform.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iterator>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>

namespace
{
using namespace Client;
using RESOURCE = KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE;
using OCCURRENCE = KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE;
using KIND = KOUKU_SAYDON_PRESENTATION_KIND;
constexpr std::uint32_t MAX_TIMELINE_MS = 600000u;

// Owner-thread only, just like the existing V2 GPU resource cache. A new arena
// collection or explicit V2 authoring invalidation drops this CPU snapshot set.
std::map<std::string, std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>> g_RaidEffectSnapshots;
std::uint64_t g_RaidEffectGeneration = 0u;

float Counter_AfterimageAgeSeconds(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, const double clockMs)
{
    if (!std::isfinite(clockMs)) return -1.f;
    for (const auto& box : pattern.LogicOccurrences)
    {
        if (!box.bEnabled || !box.iDurationMs || clockMs < box.iStartMs ||
            clockMs >= double(box.iStartMs) + box.iDurationMs) continue;
        const auto logic = std::find_if(document.Logics.begin(), document.Logics.end(),
            [&](const auto& value) { return value.strLogicId == box.strLogicId; });
        if (logic != document.Logics.end() && logic->strLogicType == "DURATION" &&
            logic->strJudgementKind == "COUNTER_WINDOW")
            return float((clockMs - box.iStartMs) * .001);
    }
    return -1.f;
}

bool Charge_AfterimageActive(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, const float clockMs,
    bool& backstep)
{
    backstep = false;
    if (pattern.strActorProfileId != "MN_RPCT_05" || !std::isfinite(clockMs)) return false;
    double stageStart = 0.0;
    for (const auto& stage : pattern.Stages)
    {
        for (const auto& animation : stage.AnimationOccurrences)
        {
            // Cooked Action 4219863 TrailGhostEffect notify times, in source clip time.
            const double notifyMs = animation.strRuntimeClip == "rpct00_att_battle_22_01" ? 209.004 :
                animation.strRuntimeClip == "rpct00_att_battle_22_04" ? 198.055 : -1.0;
            const double localMs = clockMs - stageStart - animation.iStartOffsetMs;
            const double sourceMs = animation.iSourceStartMs + localMs * animation.fPlayRate;
            // Original TrailGhost at clip 34_04 time 0, emission duration 50ms.
            if (animation.strRuntimeClip == "rpct00_att_battle_34_04" &&
                localMs >= 0.0 && localMs < animation.iPlayMs && sourceMs >= 0.0 && sourceMs < 50.0)
            { backstep = true; return true; }
            if (notifyMs >= 0.0 && localMs >= 0.0 && localMs < animation.iPlayMs &&
                animation.iSourceStartMs + localMs * animation.fPlayRate >= notifyMs) return true;
        }
        stageStart += stage.iDurationMs;
    }
    return false;
}

bool Validate_EffectAnchor(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, const OCCURRENCE& box, std::string& status)
{
    const auto reject = [&](const char* reason) {
        status = std::string(reason) + ": " + box.strOccurrenceId;
        return false;
    };
    const auto stableId = [](const std::string& value) {
        return !value.empty() && value.size() <= 128u && value != "." && value != ".." &&
            std::all_of(value.begin(), value.end(), [](const unsigned char c) {
                return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
            });
    };
    if ((box.strAnchorKind != "BOSS" && box.strAnchorKind != "WORLD" && box.strAnchorKind != "MAP") ||
        (box.strBoneTarget != "BODY" && box.strBoneTarget != "WEAPON") ||
        (!box.strBone.empty() && !stableId(box.strBone)) || box.iWorldEmissionIndex > 127u)
        return reject("Invalid Effect anchor, bone or World emission index");
    if (box.strBoneRotation != "TARGET_YAW" && (box.strBoneRotation != "BONE" ||
        box.strAnchorKind != "BOSS" || box.strBone.empty()))
        return reject("Bone rotation requires a boss anchor with an explicit bone");
    if (box.strAnchorKind == "MAP" && (box.bFollowBoss || !box.strBone.empty() ||
        box.strBoneTarget != "BODY" || !box.strWorldId.empty() || !box.strWorldOccurrenceId.empty()))
        return reject("MAP Effect requires a fixed position without a bone or World dependency");
    if (box.strAnchorKind == "WORLD")
    {
        if (box.strWorldId.empty() || std::none_of(document.Worlds.begin(), document.Worlds.end(),
            [&](const auto& world) { return world.strWorldId == box.strWorldId; }))
            return reject("Effect needs an existing World anchor");
    }
    else if (!box.strWorldId.empty())
        return reject("Only a WORLD Effect can name a World anchor");
    if (box.strBoneTarget == "WEAPON" && (box.strAnchorKind != "BOSS" || box.strBone.empty()))
        return reject("WEAPON Effect requires a boss anchor and an explicit weapon bone");
    if (!box.strWorldOccurrenceId.empty())
    {
        const auto owner = std::find_if(pattern.WorldOccurrences.begin(), pattern.WorldOccurrences.end(),
            [&](const auto& world) { return world.strOccurrenceId == box.strWorldOccurrenceId; });
        const auto world = owner == pattern.WorldOccurrences.end() ? document.Worlds.end() :
            std::find_if(document.Worlds.begin(), document.Worlds.end(),
                [&](const auto& value) { return value.strWorldId == owner->strWorldId; });
        if (!stableId(box.strWorldOccurrenceId) || world == document.Worlds.end())
            return reject("Effect needs an existing World occurrence in the same Pattern");
        if (box.strAnchorKind == "WORLD")
        {
            if (box.strWorldId != owner->strWorldId)
                return reject("Effect World occurrence must match its World anchor");
        }
        else if (world->strCompanionEffectResourceId != box.strResourceId ||
            std::any_of(pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(),
                [&](const auto& other) {
                    if (other.strOccurrenceId == box.strOccurrenceId || other.strAnchorKind == "WORLD" ||
                        other.strWorldOccurrenceId != box.strWorldOccurrenceId) return false;
                    return std::any_of(document.PresentationResources.begin(), document.PresentationResources.end(),
                        [&](const auto& resource) {
                            return resource.strResourceId == other.strResourceId && resource.eKind == KIND::EFFECT;
                        });
                }))
            return reject("Effect companion needs one matching World box/resource in the same Pattern");
    }
    return true;
}

std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT> WorldPlacementFromOccurrence(
    const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE& box)
{
    if (!box.Placement) return {};
    const auto& value = *box.Placement;
    return CWorldSequencePlayer::OBJECT_PLACEMENT{
        {float(value.Position[0]), float(value.Position[1]), float(value.Position[2])},
        {float(value.RotationDegrees[0]), float(value.RotationDegrees[1]), float(value.RotationDegrees[2])},
        {float(value.Scale[0]), float(value.Scale[1]), float(value.Scale[2])}};
}

std::uint32_t Camera_ReturnMs(const RESOURCE& resource, const bool authoring)
{
    if (resource.eKind != KIND::CAMERA) return 0u;
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level) return 0u;
    if (authoring) { std::string status; if (!level->Ensure_CameraShotAuthoring(status)) return 0u; }
    const auto& shots = authoring ? level->Get_CameraShots() : level->Get_PublishedCameraShots();
    const auto found = std::find_if(shots.begin(), shots.end(), [&](const auto& shot) { return shot.strShotId == resource.strAssetId; });
    return found == shots.end() ? 0u : found->iBlendOutMs;
}


const DATA_JSON_VALUE& Field(const DATA_JSON_VALUE& row, const char* key)
{
    const auto* value = row.Find(key);
    if (!value) throw std::runtime_error(std::string("Missing presentation field: ") + key);
    return *value;
}

std::string Text(const DATA_JSON_VALUE& row, const char* key, bool allowEmpty = false)
{
    const auto& value = Field(row, key);
    if (!value.Is_String() || value.Get_String().size() > 512u ||
        (!allowEmpty && value.Get_String().empty()) ||
        value.Get_String().find('\0') != std::string::npos)
        throw std::runtime_error(std::string("Invalid presentation string: ") + key);
    return value.Get_String();
}

double Number(const DATA_JSON_VALUE& row, const char* key, double low, double high)
{
    const auto& value = Field(row, key);
    if (!value.Is_Number() || !std::isfinite(value.Get_Number()) ||
        value.Get_Number() < low || value.Get_Number() > high)
        throw std::runtime_error(std::string("Invalid presentation number: ") + key);
    return value.Get_Number();
}

std::uint32_t UInt(const DATA_JSON_VALUE& row, const char* key,
    std::uint32_t low, std::uint32_t high)
{
    const double value = Number(row, key, low, high);
    if (std::floor(value) != value)
        throw std::runtime_error(std::string("Presentation integer required: ") + key);
    return static_cast<std::uint32_t>(value);
}

std::array<double, 3u> Vector(const DATA_JSON_VALUE& row, const char* key,
    double low, double high)
{
    const auto& value = Field(row, key);
    if (!value.Is_Array() || value.Get_Array().size() != 3u)
        throw std::runtime_error(std::string("Presentation vector required: ") + key);
    std::array<double, 3u> result{};
    for (size_t index = 0; index < result.size(); ++index)
    {
        const auto& part = value.Get_Array()[index];
        if (!part.Is_Number() || !std::isfinite(part.Get_Number()) ||
            part.Get_Number() < low || part.Get_Number() > high)
            throw std::runtime_error(std::string("Invalid presentation vector: ") + key);
        result[index] = part.Get_Number();
    }
    return result;
}

KIND Read_Kind(const std::string& kind)
{
    if (kind == "EFFECT") return KIND::EFFECT;
    if (kind == "SOUND") return KIND::SOUND;
    if (kind == "SUBTITLE") return KIND::SUBTITLE;
    if (kind == "CAMERA") return KIND::CAMERA;
    if (kind == "COLLIDER") return KIND::COLLIDER;
    if (kind == "LIGHT") return KIND::LIGHT;
    if (kind == "SCENE_PROFILE") return KIND::SCENE_PROFILE;
    throw std::runtime_error("Unsupported presentation kind: " + kind);
}

RESOURCE Read_Resource(const DATA_JSON_VALUE& row)
{
    RESOURCE resource;
    resource.strResourceId = Text(row, "resourceId");
    resource.strDisplayName = resource.strResourceId;
    resource.eKind = Read_Kind(Text(row, "kind"));
    resource.strAssetId = Text(row, "assetId", resource.eKind == KIND::COLLIDER);
    if (row.Find("soundEvent")) resource.strSoundEvent = Text(row, "soundEvent", true);
    if (const auto* subtitle = row.Find("subtitleText"))
    {
        if (!subtitle->Is_String()) throw std::runtime_error("Invalid Subtitle text type.");
        resource.strSubtitleText = subtitle->Get_String();
    }
    if (row.Find("subtitlePosition")) resource.strSubtitlePosition = Text(row, "subtitlePosition");
    resource.strResourceKind = Text(row, "resourceKind", resource.eKind != KIND::EFFECT);
    if (row.Find("elementId")) resource.strElementId = Text(row, "elementId", true);
    resource.iDurationMs = UInt(row, "resourceDurationMs", 1u, MAX_TIMELINE_MS);
    resource.strShape = Text(row, "shape");
    resource.HalfExtents = Vector(row, "halfExtents", 0.001, 100000.0);
    resource.fRadiusM = Number(row, "radiusM", 0.001, 100000.0);
    if (row.Find("innerRadiusM")) resource.fInnerRadiusM = Number(row, "innerRadiusM", 0.0, resource.fRadiusM);
    if (resource.fInnerRadiusM >= resource.fRadiusM || (resource.fInnerRadiusM > 0.0 &&
        (resource.eKind != KIND::COLLIDER || (resource.strShape != "CIRCLE" && resource.strShape != "SECTOR"))))
        throw std::runtime_error("Invalid Collider inner radius");
    resource.fHalfAngleDegrees = Number(row, "halfAngleDegrees", resource.strShape == "REVERSE_SECTOR" ? 0.0 : 0.001, 180.0);
    const bool v1 = resource.strResourceKind == "V1_EFFECT" || resource.strResourceKind == "V1_ELEMENT";
    if ((resource.eKind == KIND::EFFECT && resource.strResourceKind != "GROUP" && resource.strResourceKind != "LEAF" && !v1) ||
        (resource.strShape != "BOX" && resource.strShape != "SECTOR" && resource.strShape != "REVERSE_SECTOR" && resource.strShape != "CIRCLE" && resource.strShape != "CYLINDER"))
        throw std::runtime_error("Invalid presentation resource type: " + resource.strResourceId);
    if (resource.eKind == KIND::EFFECT &&
        !CEffectV2Document::Is_ValidEffectId(resource.strAssetId))
        throw std::runtime_error("Invalid Effect identity: " + resource.strAssetId);
    const auto stableLightId = [](const std::string& id)
    {
        return !id.empty() && id.size() <= 128u && id != "." && id != ".." &&
            std::all_of(id.begin(), id.end(), [](unsigned char c) {
                return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.'; });
    };
    if ((resource.strResourceKind == "V1_ELEMENT" && !stableLightId(resource.strElementId)) ||
        (resource.strResourceKind != "V1_ELEMENT" && !resource.strElementId.empty()))
        throw std::runtime_error("Invalid Effect element identity: " + resource.strElementId);
    if (resource.eKind == KIND::LIGHT && (!resource.strResourceKind.empty() ||
        !stableLightId(resource.strAssetId)))
        throw std::runtime_error("Invalid Light resource identity: " + resource.strAssetId);
    if (!resource.strSoundEvent.empty() &&
        (resource.eKind != KIND::SOUND || !stableLightId(resource.strSoundEvent)))
        throw std::runtime_error("Invalid Sound event identity: " + resource.strSoundEvent);
    if ((resource.eKind == KIND::SUBTITLE &&
        (!stableLightId(resource.strAssetId) || !resource.strResourceKind.empty() ||
         !CKoukuSaydonCompositionDocument::Is_ValidSubtitleText(resource.strSubtitleText))) ||
        (resource.eKind != KIND::SUBTITLE && (!resource.strSubtitleText.empty() || resource.strSubtitlePosition != "NORMAL")) ||
        (resource.strSubtitlePosition != "NORMAL" && resource.strSubtitlePosition != "UPPER"))
        throw std::runtime_error("Invalid Subtitle resource: " + resource.strResourceId);
    if (resource.eKind == KIND::SOUND &&
        (resource.strAssetId.rfind("Sound/", 0u) != 0u ||
         CRuntimeAssetRoot::Resolve(resource.strAssetId).empty()))
        throw std::runtime_error("Invalid Sound asset ID: " + resource.strAssetId);
    if (resource.eKind == KIND::COLLIDER && !resource.strAssetId.empty())
        throw std::runtime_error("Collider presentation cannot name an asset.");
    return resource;
}

OCCURRENCE Read_Occurrence(const DATA_JSON_VALUE& row, std::uint32_t durationMs, KIND kind)
{
    OCCURRENCE box;
    box.strOccurrenceId = Text(row, "occurrenceId");
    box.strResourceId = Text(row, "resourceId");
    box.iStartMs = UInt(row, "startMs", 0u, durationMs);
    box.iDurationMs = UInt(row, "durationMs", 1u, durationMs);
    if (box.iDurationMs > durationMs - box.iStartMs)
        throw std::runtime_error("Presentation occurrence exceeds its pattern.");
    box.PositionOffset = Vector(row, "positionOffset", -100000.0, 100000.0);
    box.RotationDegrees = Vector(row, "rotationDegrees", -100000.0, 100000.0);
    box.Scale = Vector(row, "scale", 0.001, 100000.0);
    box.ColliderEndPositionOffset = box.PositionOffset;
    box.ColliderEndScale = box.Scale;
    if (row.Find("anchorPresentationOccurrenceId")) box.strAnchorPresentationOccurrenceId = Text(row, "anchorPresentationOccurrenceId");
    if (!box.strAnchorPresentationOccurrenceId.empty() && kind != KIND::COLLIDER)
        throw std::runtime_error("Only Collider can reference an Effect birth anchor.");
    if (row.Find("colliderMotion")) box.strColliderMotion = Text(row, "colliderMotion");
    if (row.Find("colliderEndPositionOffset")) box.ColliderEndPositionOffset = Vector(row, "colliderEndPositionOffset", -100000.0, 100000.0);
    if (row.Find("colliderEndScale")) box.ColliderEndScale = Vector(row, "colliderEndScale", 0.001, 100000.0);
    if ((box.strColliderMotion != "STATIC" && box.strColliderMotion != "LINEAR") ||
        (box.strColliderMotion != "STATIC" && kind != KIND::COLLIDER))
        throw std::runtime_error("Collider motion requires a Collider occurrence and STATIC/LINEAR type.");
    // SCENE_PROFILE projects its reserved blendMs metadata into fadeInMs.
    // Its authoring range is the whole timeline, independent of this box's
    // lifetime; applying Effect envelope bounds here rejects every Product.
    box.iFadeInMs = UInt(row, "fadeInMs", 0u,
        kind == KIND::SCENE_PROFILE ? MAX_TIMELINE_MS : box.iDurationMs);
    box.iFadeOutMs = UInt(row, "fadeOutMs", 0u, box.iDurationMs);
    if (kind != KIND::SCENE_PROFILE &&
        box.iFadeInMs + box.iFadeOutMs > box.iDurationMs)
        throw std::runtime_error("Presentation fades exceed the occurrence.");
    box.fDissolveStart = Number(row, "dissolveStart", 0.0, 1.0);
    box.fDissolveEnd = Number(row, "dissolveEnd", 0.0, 1.0);
    if (box.fDissolveStart > box.fDissolveEnd)
        throw std::runtime_error("Presentation dissolve interval is reversed.");
    box.fVolume = Number(row, "volume", 0.0, 1.0);
    if (row.Find("soundSourceStartMs")) box.iSoundSourceStartMs = UInt(row, "soundSourceStartMs", 0u, MAX_TIMELINE_MS);
    if (box.iSoundSourceStartMs && kind != KIND::SOUND) throw std::runtime_error("Sound Source In requires SOUND.");
    if (row.Find("brightnessMultiplier")) box.fBrightnessMultiplier = Number(row, "brightnessMultiplier", 0.0, 16.0);
    const auto& follow = Field(row, "followBoss");
    if (!follow.Is_Boolean()) throw std::runtime_error("followBoss must be Boolean.");
    box.bFollowBoss = follow.Get_Boolean();
    if (const auto* fit = row.Find("fitEffectToDuration"))
    {
        if (!fit->Is_Boolean()) throw std::runtime_error("fitEffectToDuration must be Boolean.");
        box.bFitEffectToDuration = fit->Get_Boolean();
        if (box.bFitEffectToDuration && (kind != KIND::EFFECT ||
            (Field(row, "resourceKind").Get_String() != "V1_EFFECT" && Field(row, "resourceKind").Get_String() != "V1_ELEMENT")))
            throw std::runtime_error("Fit Effect lifetime requires a V1 Effect resource.");
    }
    if (const auto* loop = row.Find("loopEffectToDuration"))
    {
        if (!loop->Is_Boolean()) throw std::runtime_error("loopEffectToDuration must be Boolean.");
        box.bLoopEffectToDuration = loop->Get_Boolean();
        if (box.bLoopEffectToDuration && (box.bFitEffectToDuration || kind != KIND::EFFECT ||
            (Field(row, "resourceKind").Get_String() != "V1_EFFECT" && Field(row, "resourceKind").Get_String() != "V1_ELEMENT")))
            throw std::runtime_error("Loop Effect lifetime requires a V1 Effect without time stretching.");
    }
    if (const auto* debug = row.Find("debugRender"))
    {
        if (!debug->Is_Boolean()) throw std::runtime_error("debugRender must be Boolean.");
        box.bDebugRender = debug->Get_Boolean();
    }
    box.strBone = Text(row, "bone", true);
    if (row.Find("boneTarget")) box.strBoneTarget = Text(row, "boneTarget");
    if (box.strBoneTarget != "BODY" && box.strBoneTarget != "WEAPON")
        throw std::runtime_error("Presentation boneTarget is unsupported.");
    if (box.strBone.size() > 128u) throw std::runtime_error("Presentation bone is too long.");
    if (row.Find("anchorKind")) box.strAnchorKind = Text(row, "anchorKind");
    if (row.Find("worldId")) box.strWorldId = Text(row, "worldId", true);
    if (row.Find("worldOccurrenceId")) box.strWorldOccurrenceId = Text(row, "worldOccurrenceId", true);
    if (row.Find("worldEmissionIndex")) box.iWorldEmissionIndex = UInt(row, "worldEmissionIndex", 0u, 127u);
    if (box.strAnchorKind != "BOSS" && box.strAnchorKind != "WORLD" &&
        !(kind == KIND::LIGHT && (box.strAnchorKind == "MAP" || box.strAnchorKind == "PLAYER")) &&
        !((kind == KIND::EFFECT || kind == KIND::COLLIDER || kind == KIND::SUBTITLE || kind == KIND::SOUND) && box.strAnchorKind == "MAP"))
        throw std::runtime_error("Presentation anchorKind is unsupported.");
    if (kind == KIND::SUBTITLE && (box.strAnchorKind != "MAP" || box.bFollowBoss || !box.strBone.empty() ||
        !box.strWorldId.empty() || !box.strWorldOccurrenceId.empty() || box.iWorldEmissionIndex != 0u))
        throw std::runtime_error("Subtitle requires a fixed MAP anchor without a bone or World occurrence.");
    if (kind == KIND::LIGHT && ((box.strAnchorKind != "WORLD" && !box.strWorldId.empty()) ||
        box.Scale != std::array<double, 3u>{1.0, 1.0, 1.0} ||
        (box.strAnchorKind != "BOSS" && !box.strBone.empty()) ||
        (box.strAnchorKind == "PLAYER" && !box.bFollowBoss)))
        throw std::runtime_error("Invalid Light anchor, scale or bone.");
    if ((kind == KIND::EFFECT || kind == KIND::COLLIDER || kind == KIND::SOUND) && box.strAnchorKind == "MAP" &&
        (box.bFollowBoss || !box.strBone.empty() || box.strBoneTarget != "BODY" ||
            !box.strWorldId.empty() || !box.strWorldOccurrenceId.empty() ||
            ((kind == KIND::COLLIDER || kind == KIND::SOUND) && box.iWorldEmissionIndex != 0u)))
        throw std::runtime_error("MAP Effect/Collider/Sound requires a fixed position without a bone or World occurrence.");
    if ((kind == KIND::EFFECT || kind == KIND::LIGHT || kind == KIND::COLLIDER) &&
        box.strAnchorKind == "WORLD" && box.strWorldId.empty())
        throw std::runtime_error("World Object anchor needs a worldId; fixed world coordinates use MAP: " + box.strOccurrenceId);
    if (box.strBoneTarget == "WEAPON" && ((kind != KIND::COLLIDER && kind != KIND::EFFECT) || box.strAnchorKind != "BOSS" || box.strBone.empty()))
        throw std::runtime_error("WEAPON bone target needs a Boss Collider/Effect and a named weapon bone.");
    if (row.Find("boneRotation")) box.strBoneRotation = Text(row, "boneRotation");
    if (box.strBoneRotation != "TARGET_YAW" && (box.strBoneRotation != "BONE" || kind != KIND::EFFECT ||
        box.strAnchorKind != "BOSS" || box.strBone.empty()))
        throw std::runtime_error("Bone rotation needs a Boss Effect and a named bone: " + box.strOccurrenceId);
    return box;
}

DATA_JSON_VALUE Read_ProductPresentationRoot(const std::string* supplied = nullptr)
{
    const auto admitted = CKoukuSaydonPresentationAssetService::Get_AdmittedDraftProduct();
    if (!supplied && admitted) supplied = &admitted->PresentationJson;
    std::string text;
    if (supplied)
    {
        if (supplied->empty() || supplied->size() > 16u * 1024u * 1024u)
            throw std::runtime_error("Draft Product presentation is empty or oversized.");
        text = *supplied;
    }
    else
    {
        const auto path = CProjectDataRoot::Resolve("Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json");
        std::error_code error;
        const auto bytes = std::filesystem::file_size(path, error);
        if (error || bytes > 16u * 1024u * 1024u)
            throw std::runtime_error("KoukuSaydon Product presentation is missing or oversized.");
        std::ifstream input(path, std::ios::binary);
        text.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
        if (!input || input.bad() || text.size() != bytes)
            throw std::runtime_error("KoukuSaydon Product presentation read failed.");
    }
        DATA_JSON_VALUE root;
        std::string parseStatus;
        if (!CDataJson::Parse(text, root, parseStatus) || !root.Is_Object())
            throw std::runtime_error("KoukuSaydon Product parse failed: " + parseStatus);
        if (Text(root, "schema") != "lostark.kouku-saydon-pattern-bindings" ||
            UInt(root, "formatVersion", 1u, 1u) != 1u ||
            Text(root, "bossArchetypeId") != "BOSS_KAKULSAYDON_G1_KOUKU")
            throw std::runtime_error("KoukuSaydon Product presentation header is incompatible.");
    return root;
}

struct PRESENTATION_WINDOW final
{
    KIND kind;
    std::int64_t startSubtick = 0, endSubtick = 0;
    std::string owner;
};
bool Admit_PresentationWindow(std::vector<PRESENTATION_WINDOW>& windows,
    KIND kind, double startMs, double endMs, const std::string& owner)
{
    const auto start = static_cast<std::int64_t>(std::llround(startMs * 30.0));
    const auto end = static_cast<std::int64_t>(std::llround(endMs * 30.0));
    for (const auto& window : windows)
        if (window.kind == kind && window.owner != owner &&
            start < window.endSubtick && window.startSubtick < end) return false;
    windows.push_back({ kind, start, end, owner });
    return true;
}

std::uint32_t Pattern_Duration(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern)
{
    std::uint64_t stageDuration = 0;
    for (const auto& stage : pattern.Stages) stageDuration += stage.iDurationMs;
    std::uint64_t duration = (std::max)(stageDuration, std::uint64_t(pattern.iDurationMs));
    for (const auto& row : pattern.LogicOccurrences)
        duration = (std::max)(duration, std::uint64_t(row.iStartMs) + row.iDurationMs);
    for (const auto& row : pattern.SummonOccurrences)
        duration = (std::max)(duration, std::uint64_t(row.iStartMs) + row.iDurationMs);
    for (const auto& row : pattern.PresentationOccurrences)
        duration = (std::max)(duration, std::uint64_t(row.iStartMs) + row.iDurationMs);
    for (const auto& row : pattern.SceneProfileOccurrences)
        duration = (std::max)(duration, std::uint64_t(row.iStartMs) + row.iDurationMs);
    for (const auto& row : pattern.WorldOccurrences)
        duration = (std::max)(duration, std::uint64_t(row.iStartMs) + row.iDurationMs);
    return static_cast<std::uint32_t>((std::min)(duration, std::uint64_t(MAX_TIMELINE_MS)));
}

bool Make_Pivot(const OCCURRENCE& box, const float4x4_t& root,
    const std::shared_ptr<Engine::CModel>& model, float4x4_t& result,
    const ANIMATION_MODEL_TARGET_VIEW* weaponView = nullptr, float4x4_t* sampledBasis = nullptr)
{
    float4x4_t anchor = root;
    if (!box.strBone.empty())
    {
        EFFECT_V2_TARGET_VIEW view;
        if (box.strBoneTarget == "WEAPON")
        {
            if (!weaponView) return false;
            view.pModel = weaponView->Model;
            view.BoneRoot = weaponView->BoneRoot;
            view.YawBasis = weaponView->TargetRoot;
        }
        else if (box.strBoneTarget == "BODY")
        {
            view.pModel = model;
            view.BoneRoot = root;
            view.YawBasis = root;
        }
        else return false;
        if (!view.pModel || !view.pModel->Has_Bone(box.strBone.c_str())) return false;
        if (!CEffectV2Object::Resolve_TargetPivot(view, box.strBone,
            box.strBoneRotation == "BONE" ? CEffectV2Object::PIVOT_ROTATION::BONE :
            CEffectV2Object::PIVOT_ROTATION::TARGET_YAW, anchor)) return false;
    }
    matrix_t basis = XMLoadFloat4x4(&anchor);
    for (size_t axis = 0; axis < 3u; ++axis)
    {
        if (XMVectorGetX(XMVector3LengthSq(basis.r[axis])) < 0.000001f) return false;
        basis.r[axis] = XMVector3Normalize(basis.r[axis]);
    }
    if (sampledBasis) XMStoreFloat4x4(sampledBasis, basis);
    XMStoreFloat4x4(&result,
        XMMatrixScaling(float(box.Scale[0]), float(box.Scale[1]), float(box.Scale[2])) *
        XMMatrixRotationRollPitchYaw(XMConvertToRadians(float(box.RotationDegrees[0])),
            XMConvertToRadians(float(box.RotationDegrees[1])), XMConvertToRadians(float(box.RotationDegrees[2]))) *
        XMMatrixTranslation(float(box.PositionOffset[0]), float(box.PositionOffset[1]), float(box.PositionOffset[2])) * basis);
    return true;
}

// Recorded resolved anchors contain WORLD scale but no editable occurrence geometry.
bool Make_ResolvedEffectPivot(const OCCURRENCE& box, const float4x4_t& anchor,
    float4x4_t& output)
{
    auto placed = box;
    placed.strBone.clear();
    const matrix_t basis = XMLoadFloat4x4(&anchor);
    for (size_t axis = 0u; axis < 3u; ++axis)
    {
        const float scale = XMVectorGetX(XMVector3Length(basis.r[axis]));
        placed.PositionOffset[axis] *= scale;
        placed.Scale[axis] *= scale;
    }
    return Make_Pivot(placed, anchor, {}, output);
}

CEffectV2Object::PIVOT_SAMPLER Effect_PivotSampler(const OCCURRENCE& box,
    const std::shared_ptr<EFFECT_V2_PIVOT_HISTORY>& rootHistory,
    const std::shared_ptr<EFFECT_V2_PIVOT_HISTORY>& anchorHistory,
    CEffectV2Object::PIVOT_SAMPLER exactRoot = {})
{
    if (box.strAnchorKind == "MAP" || !box.bFollowBoss) return {};
    if (exactRoot && box.strAnchorKind == "BOSS" && box.strBone.empty())
        return [box, exactRoot = std::move(exactRoot)](float seconds, float4x4_t& output, std::string& error) {
            float4x4_t root;
            return exactRoot(box.iStartMs / 1000.f + seconds, root, error) && Make_Pivot(box, root, {}, output);
        };
    const bool resolved = box.strAnchorKind == "WORLD" || !box.strBone.empty();
    return [box, history = resolved ? anchorHistory : rootHistory, resolved]
        (float seconds, float4x4_t& output, std::string& error)
    {
        if (!history) { error = "Effect anchor history is unavailable."; return false; }
        float4x4_t recorded;
        if (!history->Sample(box.iStartMs / 1000.f + seconds,
            recorded, error)) return false;
        if (resolved ? Make_ResolvedEffectPivot(box, recorded, output) :
            Make_Pivot(box, recorded, {}, output)) return true;
        error = "Recorded Effect anchor cannot form its authored pivot.";
        return false;
    };
}

using SOURCE_ATTACHMENTS = std::vector<EFFECT_ACTION_CUE_ATTACHMENT_DESC>;
using SOURCE_BONES = std::unordered_map<std::string, float4x4_t>;
using ANCHOR_ANIMATION = KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE;

SOURCE_ATTACHMENTS Source_Attachments(const EFFECT_DOCUMENT_DESC& document, const std::string& elementId = {})
{
    SOURCE_ATTACHMENTS result;
    for (const auto& element : document.Elements)
    {
        const auto& attachment = element.ActionCueAttachment;
        if (element.bVisible && (elementId.empty() || element.strElementId == elementId) &&
            attachment.bEnabled && attachment.bFollow && attachment.strModelCueId.empty())
            result.push_back(attachment);
    }
    return result;
}

bool Valid_SourceMatrix(const matrix_t& value)
{
    const float determinant = XMVectorGetX(XMMatrixDeterminant(value));
    return !XMMatrixIsNaN(value) && !XMMatrixIsInfinite(value) &&
        std::isfinite(determinant) && std::abs(determinant) > 1.e-12f;
}

bool Build_SourceAnchorWorlds(const SOURCE_ATTACHMENTS& attachments,
    const EFFECT_V2_TARGET_VIEW& view, const float4x4_t& root, const SOURCE_BONES& bones,
    SOURCE_BONES& anchors, std::string& error)
{
    SOURCE_BONES staged;
    float4x4_t ownerPivot;
    const bool needsBones = std::any_of(attachments.begin(), attachments.end(), [](const auto& value)
        { return value.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW; });
    matrix_t delta = XMMatrixIdentity();
    if (needsBones)
    {
        if (!CEffectV2Object::Resolve_TargetPivot(view, "", CEffectV2Object::PIVOT_ROTATION::TARGET_YAW, ownerPivot) ||
            !Valid_SourceMatrix(XMLoadFloat4x4(&ownerPivot)) || !Valid_SourceMatrix(XMLoadFloat4x4(&root)))
        { error = "Kouku source attachment owner root is unavailable or singular."; return false; }
        delta = XMMatrixInverse(nullptr, XMLoadFloat4x4(&ownerPivot)) * XMLoadFloat4x4(&root);
    }
    for (const auto& attachment : attachments)
    {
        if (attachment.strRuntimeAnchorSlotId.empty())
        { error = "Kouku source attachment has no runtime slot."; return false; }
        matrix_t anchor;
        if (attachment.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW)
        {
            const auto* camera = CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW);
            if (!camera) { error = "Kouku source camera anchor is unavailable."; return false; }
            anchor = XMLoadFloat4x4(camera);
        }
        else
        {
            const auto bone = bones.find(attachment.strRuntimeBoneName);
            if (bone == bones.end())
            { error = "Kouku source bone is unavailable: " + attachment.strRuntimeBoneName; return false; }
            matrix_t raw = XMLoadFloat4x4(&bone->second);
            // Both Kouku/Saydon cooked rigs already carry a 100x root basis.
            // CModel applies the actor pre-scale (G1 0.017 -> 1.7, G2 Kouku
            // 0.012053 -> 1.2053). Metre-based offsets consume that basis
            // directly; preserve animated scale/translation without another x100.
            if (attachment.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::BONE)
                anchor = raw * XMLoadFloat4x4(&view.BoneRoot);
            else if (attachment.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::OWNER_YAW)
            {
                float4x4_t normalizedBone, yawAnchor;
                XMStoreFloat4x4(&normalizedBone, raw);
                if (!CEffectPlayback::Build_OwnerYawBoneAnchorWorld(normalizedBone, view.BoneRoot, view.YawBasis, yawAnchor))
                { error = "Kouku source owner-yaw anchor is invalid."; return false; }
                anchor = XMLoadFloat4x4(&yawAnchor);
            }
            else { error = "Unsupported Kouku source attachment orientation."; return false; }
        }
        const auto& local = attachment.SocketLocalTransform;
        matrix_t world = XMMatrixScaling(local.vScale.x, local.vScale.y, local.vScale.z) *
            XMMatrixRotationRollPitchYaw(XMConvertToRadians(local.vRotationDegrees.x),
                XMConvertToRadians(local.vRotationDegrees.y), XMConvertToRadians(local.vRotationDegrees.z)) *
            XMMatrixTranslation(local.vPosition.x, local.vPosition.y, local.vPosition.z) * anchor;
        if (attachment.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW) world *= delta;
        if (!Valid_SourceMatrix(world))
        { error = "Kouku source attachment matrix is invalid: " + attachment.strRuntimeAnchorSlotId; return false; }
        float4x4_t value;
        XMStoreFloat4x4(&value, world);
        const auto [found, inserted] = staged.emplace(attachment.strRuntimeAnchorSlotId, value);
        if (!inserted)
            for (size_t r = 0u; r < 4u; ++r) for (size_t c = 0u; c < 4u; ++c)
                if (std::abs(found->second.m[r][c] - value.m[r][c]) > 0.0001f)
                { error = "Conflicting Kouku source attachment slot: " + attachment.strRuntimeAnchorSlotId; return false; }
    }
    anchors = std::move(staged);
    error.clear();
    return true;
}

bool Sample_SourceBones(const std::shared_ptr<CModel>& model,
    const std::vector<ANCHOR_ANIMATION>& animations,
    std::span<const KOUKU_SAYDON_ANIMATION_BLEND_WINDOW> blendWindows, float sampleMs,
    const std::vector<std::string>& names, SOURCE_BONES& bones, std::string& error)
{
    if (names.empty()) return true;
    if (!model) { error = "Kouku source animation model was released."; return false; }
    const ANCHOR_ANIMATION* animation = nullptr;
    const ANCHOR_ANIMATION* previous = nullptr;
    for (const auto& value : animations)
        if (sampleMs >= (value.iPoseStartMs == UINT32_MAX ? value.iStartOffsetMs : value.iPoseStartMs))
        { previous = animation; animation = &value; }
    // A leading Effect samples the scheduled clip's first pose before playback starts.
    if (!animation && !animations.empty()) animation = &animations.front();
    if (!animation) { error = "Kouku source attachment has no animation at its requested time."; return false; }
    const auto clipIndex = [&](const std::string& name) {
        uint32_t found = UINT32_MAX;
        for (uint32_t index = 0u; index < model->Get_NumAnimations(); ++index)
            if (const auto* value = model->Get_AnimationName(index); value && name == value)
            { if (found != UINT32_MAX) return UINT32_MAX; found = index; }
        return found;
    };
    const uint32_t index = clipIndex(animation->strRuntimeClip);
    float cursor = 0.f, duration = 0.f;
    if (index == UINT32_MAX || !model->Get_AnimationProgress(index, cursor, duration))
    { error = "Kouku source animation clip is unavailable: " + animation->strRuntimeClip; return false; }
    const float age = (std::max)(0.f, sampleMs - animation->iStartOffsetMs);
    const bool loop = animation->strEndPolicy == "LOOP_TO_WINDOW";
    const float elapsed = (std::min)(age, float(animation->iPlayMs));
    const float tps = model->Get_AnimationTickPerSecond(index);
    double sourceMs = 0.0;
    if (!CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(animation->iSourceStartMs,
        animation->iSourceEndMs, elapsed, animation->fPlayRate, duration * 1000.0 / tps, loop, sourceMs))
    { error = "Kouku source animation range is outside the native clip."; return false; }
    const float ticks = float(sourceMs * tps / 1000.0);
    std::vector<uint32_t> indices;
    for (const auto& name : names)
    {
        const auto bone = model->Find_BoneIndex(name.c_str());
        if (bone < 0) { error = "Kouku source bone is unavailable: " + name; return false; }
        indices.push_back(static_cast<uint32_t>(bone));
    }
    std::vector<float4x4_t> sampled(indices.size());
    bool sampledPose = false;
    CModel::ANIMATION_TRANSITION_POSE logicPose;
    bool logicActive = false;
    if (!CKoukuSaydonAnimationBlend::Sample_Pose(*model, blendWindows, sampleMs, logicPose, logicActive, error)) return false;
    if (logicActive) sampledPose = model->Sample_AnimationTransitionBoneCombinedMatrices(logicPose, indices, sampled);
    else if (previous && animation->iBlendInMs && age < animation->iBlendInMs)
    {
        CModel::ANIMATION_TRANSITION_POSE pose;
        pose.sourceIndex = clipIndex(previous->strRuntimeClip);
        pose.targetIndex = index; pose.targetTicks = ticks;
        pose.durationSeconds = animation->iBlendInMs * .001f;
        pose.elapsedSeconds = age * .001f; pose.playRate = animation->fPlayRate;
        float previousDuration = 0.f;
        if (pose.sourceIndex == UINT32_MAX || !model->Get_AnimationProgress(pose.sourceIndex, cursor, previousDuration))
        { error = "Kouku source animation blend clip is unavailable."; return false; }
        const float previousTps = model->Get_AnimationTickPerSecond(pose.sourceIndex);
        double previousMs = 0.0;
        if (!CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(previous->iSourceStartMs,
            previous->iSourceEndMs, previous->iPlayMs, previous->fPlayRate,
            previousDuration * 1000.0 / previousTps, previous->strEndPolicy == "LOOP_TO_WINDOW", previousMs))
        { error = "Kouku source animation blend range is outside the native clip."; return false; }
        pose.sourceTicks = float(previousMs * previousTps / 1000.0);
        sampledPose = model->Sample_AnimationTransitionBoneCombinedMatrices(pose, indices, sampled);
    }
    else sampledPose = model->Sample_AnimationBoneCombinedMatrices(animation->strRuntimeClip.c_str(), ticks, indices, sampled);
    if (!sampledPose) { error = "Kouku source animation pose sample failed."; return false; }
    for (size_t i = 0u; i < names.size(); ++i) bones.emplace(names[i], sampled[i]);
    return true;
}

CKoukuSaydonPresentationPlayer::V1_SOURCE_ANCHOR_SAMPLER Make_SourceAnchorSampler(
    const RESOURCE& resource, const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
    const std::shared_ptr<CModel>& model, const float4x4_t& ownerRoot, uint32_t startMs,
    CEffectV2Object::PIVOT_SAMPLER exactRoot = {},
    float sourceSecondsPerBoxSecond = 1.f, float sourceCycleStartSeconds = 0.f)
{
    const auto document = CEffectCatalog::Find_Loaded(resource.strAssetId);
    if (!document) return [](float, const float4x4_t&, SOURCE_BONES&, std::string& error)
        { error = "Prepared Kouku source Effect document is unavailable."; return false; };
    auto attachments = Source_Attachments(*document, resource.strElementId);
    if (attachments.empty()) return {};
    // This local FEAR overlay simulates wholly in camera-local coordinates.
    // Its living particles need the current view, never historical world camera motion.
    // Keep the general Product camera-history rejection for every other resource.
    const bool localFearCamera = resource.strAssetId == "effect.kouku.fear.screen.full.restore" &&
        document->ModelCues.empty() && !document->SourceModelPreview &&
        std::all_of(document->Elements.begin(), document->Elements.end(), [](const auto& element)
        {
            const auto& attachment = element.ActionCueAttachment;
            return !element.bVisible || (element.SourceRecipe.bEnabled && element.Detail.Particle.bLocalSpace &&
                !element.SourceTransformTrack && attachment.bEnabled && attachment.bFollow &&
                attachment.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW &&
                attachment.strRuntimeBoneName.empty() && attachment.strModelCueId.empty());
        });
    std::vector<ANCHOR_ANIMATION> animations;
    uint32_t stageStart = 0u;
    for (const auto& stage : pattern.Stages)
    {
        for (auto animation : stage.AnimationOccurrences)
        {
            if (animation.iPoseStartMs == UINT32_MAX)
                animation.iPoseStartMs = stageStart + (animation.strOccurrenceId == std::min_element(stage.AnimationOccurrences.begin(), stage.AnimationOccurrences.end(),
                [](const auto& a, const auto& b) { return a.iStartOffsetMs != b.iStartOffsetMs ?
                    a.iStartOffsetMs < b.iStartOffsetMs : a.strOccurrenceId < b.strOccurrenceId; })->strOccurrenceId ? 0u : animation.iStartOffsetMs);
            animation.iStartOffsetMs += stageStart; animations.push_back(std::move(animation));
        }
        stageStart += stage.iDurationMs;
    }
    std::stable_sort(animations.begin(), animations.end(), [](const auto& a, const auto& b)
        { return a.iStartOffsetMs != b.iStartOffsetMs ? a.iStartOffsetMs < b.iStartOffsetMs : a.strOccurrenceId < b.strOccurrenceId; });
    std::vector<std::string> names;
    for (const auto& attachment : attachments)
        if (attachment.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW &&
            std::find(names.begin(), names.end(), attachment.strRuntimeBoneName) == names.end())
            names.push_back(attachment.strRuntimeBoneName);
    // Independent Server combat visuals have no Pattern animation lane. Their
    // explicitly authored source model owns the Effect-local attachment clock.
    // A real Pattern lane remains authoritative when present.
    const auto sourceDocument = animations.empty() && document->SourceModelPreview ? document : nullptr;
    // A resource with no saved model animation previews the selected pose.
    // Product/Pattern timelines still require their authored animation history.
    SOURCE_BONES frozenBones;
    std::string frozenError;
    const bool freezeResourcePose = animations.empty() && (pattern.strPatternId == "preview.kouku.resource" ||
        pattern.strPatternId == "preview.kouku.resource.actor");
    if (freezeResourcePose)
        for (const auto& name : names)
        {
            if (!model || !model->Has_Bone(name.c_str()))
            { frozenError = "Selected Resource Preview model has no source bone: " + name; break; }
            float4x4_t value;
            XMStoreFloat4x4(&value, model->Get_BoneMatrix(name.c_str()));
            frozenBones.emplace(name, value);
        }
    return [attachments = std::move(attachments), animations = std::move(animations), names = std::move(names),
        frozenBones = std::move(frozenBones), frozenError = std::move(frozenError), freezeResourcePose,
        blendWindows = pattern.AnimationBlendWindows, weakModel = std::weak_ptr<CModel>(model), ownerRoot, startMs,
        sourceDocument, sourceSecondsPerBoxSecond, sourceCycleStartSeconds, localFearCamera, exactRoot = std::move(exactRoot)]
        (float seconds, const float4x4_t& root, SOURCE_BONES& anchors, std::string& error)
    {
        if (!std::isfinite(seconds) || seconds < 0.f) { error = "Invalid Kouku source anchor sample time."; return false; }
        if (localFearCamera)
        {
            EFFECT_V2_TARGET_VIEW cameraOnlyView;
            return Build_SourceAnchorWorlds(attachments, cameraOnlyView, root, {}, anchors, error);
        }
        if (std::any_of(attachments.begin(), attachments.end(), [](const auto& value)
            { return value.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW; }))
        { error = "Kouku Product camera-view source attachment requires recorded camera history."; return false; }
        EFFECT_V2_TARGET_VIEW view;
        view.pModel = weakModel.lock(); view.BoneRoot = ownerRoot; view.YawBasis = ownerRoot;
        if (exactRoot)
        {
            if (!exactRoot(startMs / 1000.f + seconds, view.BoneRoot, error)) return false;
            view.YawBasis = view.BoneRoot;
        }
        if (sourceDocument)
        {
            const float sourceSeconds = (std::max)(0.f, seconds - sourceCycleStartSeconds) * sourceSecondsPerBoxSecond;
            return CKoukuSaydonPresentationPlayer::Sample_SourceAnchorWorlds(
                *sourceDocument, view, root, sourceSeconds, anchors, error);
        }
        if (freezeResourcePose)
        {
            if (!frozenError.empty()) { error = frozenError; return false; }
            return Build_SourceAnchorWorlds(attachments, view, root, frozenBones, anchors, error);
        }
        SOURCE_BONES bones;
        return Sample_SourceBones(view.pModel, animations, blendWindows, startMs + seconds * 1000.f, names, bones, error) &&
            Build_SourceAnchorWorlds(attachments, view, root, bones, anchors, error);
    };
}

float Effect_SourceClockRate(const RESOURCE& resource, const OCCURRENCE& box)
{
    return box.bFitEffectToDuration ? float(resource.iDurationMs) / float(box.iDurationMs) : 1.f;
}

float Effect_SourceCycleStartSeconds(const float ageSeconds, const float finiteCycleSeconds)
{
    return finiteCycleSeconds > 0.f ? static_cast<float>(
        std::floor(static_cast<double>(ageSeconds) / finiteCycleSeconds) * finiteCycleSeconds) : 0.f;
}

EFFECT_FIXED_STEP_TRANSFORM_PROVIDER Effect_V1TransformProvider(const OCCURRENCE& box,
    const float4x4_t& frozenPivot, const std::shared_ptr<EFFECT_V2_PIVOT_HISTORY>& rootHistory,
    const std::shared_ptr<EFFECT_V2_PIVOT_HISTORY>& anchorHistory,
    CKoukuSaydonPresentationPlayer::V1_SOURCE_ANCHOR_SAMPLER sourceAnchors,
    float sourceSecondsPerBoxSecond = 1.f, CEffectV2Object::PIVOT_SAMPLER exactRoot = {},
    float sourceCycleStartSeconds = 0.f)
{
    return [sampler = Effect_PivotSampler(box, rootHistory, anchorHistory, std::move(exactRoot)), frozenPivot,
        sourceAnchors = std::move(sourceAnchors), sourceSecondsPerBoxSecond, sourceCycleStartSeconds]
        (float seconds, EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& output, std::string& error)
    {
        // A finite source cycle restarts particles only. Root, bone and source
        // attachment history continue at the full occurrence age.
        seconds = sourceCycleStartSeconds + seconds / sourceSecondsPerBoxSecond;
        output.RootWorld = frozenPivot;
        output.SourceAnchorWorlds.clear();
        if (sampler && !sampler(seconds, output.RootWorld, error)) return false;
        if (sourceAnchors && !sourceAnchors(seconds, output.RootWorld, output.SourceAnchorWorlds, error)) return false;
        error.clear();
        return true;
    };
}

// A centered WORLD circle is a ground-plane radius proxy. Uniform model
// scale changes its radius; mesh roll/pitch and self-spin do not tilt the proxy.
bool Is_CenteredWorldCircle(const RESOURCE& resource, const OCCURRENCE& box)
{
    return resource.eKind == KIND::COLLIDER && resource.strShape == "CIRCLE" &&
        box.strAnchorKind == "WORLD" && box.strBone.empty() &&
        std::all_of(box.PositionOffset.begin(), box.PositionOffset.end(), [](const double v) { return v == 0.0; }) &&
        std::abs(box.Scale[0] - box.Scale[1]) <= .0001 && std::abs(box.Scale[0] - box.Scale[2]) <= .0001;
}

void Flatten_CenteredWorldCircle(const RESOURCE& resource, const OCCURRENCE& box, float4x4_t& anchor)
{
    if (!Is_CenteredWorldCircle(resource, box)) return;
    const matrix_t world = XMLoadFloat4x4(&anchor);
    const float sx = XMVectorGetX(XMVector3Length(world.r[0]));
    const float sy = XMVectorGetX(XMVector3Length(world.r[1]));
    const float sz = XMVectorGetX(XMVector3Length(world.r[2]));
    // Keep legacy nonuniform upright colliders on their existing path. The
    // gameplay publisher rejects nonuniform spinning circle sources.
    if (!std::isfinite(sx) || sx <= 0.f || std::abs(sx - sy) > .0001f || std::abs(sx - sz) > .0001f) return;
    matrix_t upright = XMMatrixIdentity();
    upright.r[3] = world.r[3];
    XMStoreFloat4x4(&anchor, upright);
}

OCCURRENCE Sample_ColliderGeometry(const OCCURRENCE& box, const double clockMs)
{
    auto sampled = box;
    if (box.strColliderMotion == "LINEAR" && box.iDurationMs)
    {
        const double alpha = std::clamp((clockMs - box.iStartMs) / box.iDurationMs, 0.0, 1.0);
        for (size_t axis = 0u; axis < 3u; ++axis)
        {
            sampled.PositionOffset[axis] += (box.ColliderEndPositionOffset[axis] - box.PositionOffset[axis]) * alpha;
            sampled.Scale[axis] += (box.ColliderEndScale[axis] - box.Scale[axis]) * alpha;
        }
    }
    return sampled;
}

HIT_AREA_SHAPE Collider_Wire(const RESOURCE& resource, const OCCURRENCE& box)
{
    HIT_AREA_SHAPE shape;
    if (resource.strShape == "BOX")
    {
        const double halfWidth = resource.HalfExtents[0] * box.Scale[0];
        const double halfLength = resource.HalfExtents[2] * box.Scale[2];
        shape.fBoxHalfHeightM = static_cast<float>(resource.HalfExtents[1] * box.Scale[1]);
        shape.iAreaType = 2;
        shape.iAreaRange = static_cast<int32_t>((std::min)(halfLength * 200.0, 1000000000.0));
        shape.iAreaAngle = static_cast<int32_t>((std::min)(halfWidth * 200.0, 1000000000.0));
        shape.iAreaOffsetX = -shape.iAreaRange / 2;
    }
    else
    {
        shape.iAreaType = resource.strShape == "CYLINDER" ? 4 : resource.strShape == "CIRCLE" ? 1 : 3;
        if (resource.strShape == "CYLINDER")
            shape.fCylinderHalfHeightM = static_cast<float>(resource.HalfExtents[1] * box.Scale[1]);
        shape.iAreaRange = static_cast<int32_t>((std::min)(
            resource.fRadiusM * (std::max)(box.Scale[0], box.Scale[2]) * 100.0, 1000000000.0));
        shape.iAreaAngle = static_cast<int32_t>(resource.fHalfAngleDegrees * 2.0);
        shape.iAreaInner = static_cast<int32_t>(resource.fInnerRadiusM * box.Scale[0] * 100.0);
        if (shape.iAreaType == 3 && resource.fInnerRadiusM == 0.0)
        {
            shape.fSectorRadiusXM = static_cast<float>(resource.fRadiusM * box.Scale[0]);
            shape.fSectorRadiusZM = static_cast<float>(resource.fRadiusM * box.Scale[2]);
            shape.fSectorAngleDegrees = static_cast<float>(resource.fHalfAngleDegrees * 2.0);
            shape.bReverseSector = resource.strShape == "REVERSE_SECTOR";
        }
    }
    return shape;
}

#ifdef _DEBUG
void Draw_LightWire(const LIGHT_DESC& light)
{
    if (light.eType == LIGHT::DIRECTIONAL) return;
    auto& game = CGameInstance::Get();
    const matrix_t view = XMLoadFloat4x4(game.Get_Transform(D3DTS::VIEW));
    const matrix_t projection = XMLoadFloat4x4(game.Get_Transform(D3DTS::PROJ));
    auto* viewport = ImGui::GetMainViewport();
    auto* draw = ImGui::GetBackgroundDrawList(viewport);
    const auto project = [&](fvector_t world, ImVec2& out)
    {
        const vector_t v = XMVector3TransformCoord(world, view);
        if (XMVectorGetZ(v) <= .1f) return false;
        const vector_t p = XMVector3TransformCoord(v, projection);
        out = {viewport->Pos.x + (XMVectorGetX(p) * .5f + .5f) * viewport->Size.x,
            viewport->Pos.y + (.5f - XMVectorGetY(p) * .5f) * viewport->Size.y};
        return std::isfinite(out.x) && std::isfinite(out.y);
    };
    const auto line = [&](fvector_t a, fvector_t b)
    { ImVec2 pa{}, pb{}; if (project(a, pa) && project(b, pb)) draw->AddLine(pa, pb, IM_COL32(255, 224, 80, 220), 1.5f); };
    const vector_t origin = XMLoadFloat4(&light.vPosition);
    const bool spot = light.eType == LIGHT::SPOT;
    for (int ring = 0; ring < (spot ? 1 : 3); ++ring)
    {
        const vector_t direction = spot ? XMVector3Normalize(XMLoadFloat4(&light.vDirection)) :
            ring == 0 ? XMVectorSet(0.f, 1.f, 0.f, 0.f) : ring == 1 ? XMVectorSet(1.f, 0.f, 0.f, 0.f) : XMVectorSet(0.f, 0.f, 1.f, 0.f);
        const vector_t up = std::abs(XMVectorGetY(direction)) > .99f ? XMVectorSet(0.f, 0.f, 1.f, 0.f) : XMVectorSet(0.f, 1.f, 0.f, 0.f);
        const vector_t right = XMVector3Normalize(XMVector3Cross(direction, up));
        const vector_t forward = XMVector3Normalize(XMVector3Cross(right, direction));
        const float angle = spot ? std::acos(std::clamp(light.fSpotOuterCos, -1.f, 1.f)) : 0.f;
        const float radius = spot ? light.fRange * std::sin(angle) : light.fRange;
        const vector_t center = spot ? origin + direction * (light.fRange * std::cos(angle)) : origin;
        for (int i = 0; i < 48; ++i)
        {
            const float a = XM_2PI * float(i) / 48.f, b = XM_2PI * float(i + 1) / 48.f;
            const vector_t p = center + (right * std::cos(a) + forward * std::sin(a)) * radius;
            line(p, center + (right * std::cos(b) + forward * std::sin(b)) * radius);
            if (spot && i % 12 == 0) line(origin, p);
        }
    }
}
#endif

std::string Card_Asset(const LostArk::Shared::PLAYER_SNAPSHOT& snapshot)
{
    using namespace LostArk::Shared;
    // Roulette retains its overhead card; maze assignment now lives on the floor.
    const MECHANIC_CARD_SYMBOL cardSymbol = snapshot.eMechanicCardSymbol;
    const MECHANIC_CARD_COLOR cardColor = snapshot.eMechanicCardColor;
    const char* symbol = nullptr;
    switch (cardSymbol)
    {
    case MECHANIC_CARD_SYMBOL::HEART: symbol = "heart"; break;
    case MECHANIC_CARD_SYMBOL::SPADE: symbol = "spade"; break;
    case MECHANIC_CARD_SYMBOL::CLUB: symbol = "clober"; break;
    case MECHANIC_CARD_SYMBOL::DIAMOND: symbol = "dia"; break;
    default: return {};
    }
    const char* color = nullptr;
    switch (cardColor)
    {
    case MECHANIC_CARD_COLOR::RED: color = "red"; break;
    case MECHANIC_CARD_COLOR::BLACK: color = "black"; break;
    default: return {};
    }
    return std::string("boss.kouku.card.") + symbol + "." + color;
}
}

struct Client::CKoukuSaydonPresentationPlayer::FRAME_LIGHT_PROVIDER final : Engine::IPresentationProvider
{
    struct FRAME_LIGHT final { LIGHT_DESC desc; bool debugRender = false; };
    std::vector<FRAME_LIGHT> lights;
    std::size_t skippedByBudget = 0u;
    HRESULT Submit_Presentation() override
    {
        auto& presentation = CPresentation_Manager::Get();
        const auto count = lights.size();
        skippedByBudget = 0u;
        // Validation is finished before this provider joins the frame transaction.
        presentation.Register_ProviderSubmissionExpectation(lights.size(), count, 0u, 0u);
        for (std::size_t i = 0u; i < count; ++i)
            if (FAILED(presentation.Add_TransientLight(lights[i].desc))) return E_FAIL;
        return S_OK;
    }
};

std::size_t Client::CKoukuSaydonPresentationPlayer::Light_SkippedByBudget() const
{
    return m_LightProvider ? m_LightProvider->skippedByBudget : 0u;
}

void Client::CKoukuSaydonPresentationPlayer::Collect_FrameLights()
{
    if (!m_LightProvider) m_LightProvider = std::make_shared<FRAME_LIGHT_PROVIDER>();
    m_LightProvider->lights.clear();
    if (!m_pLightResources) return;
    const auto collect = [&](const SESSION& session, bool preview, std::uint32_t bossId = 0u)
    {
        for (const auto& [id, row] : session.rows)
        {
            if (row.kind != KIND::LIGHT || row.failed || row.waitingForAnchor || row.lightWeight <= 0.f) continue;
            if (!m_FearSession.rows.empty() && &session != &m_FearSession) continue;
            const auto* resource = preview ? m_pLightResources->Find_Resource(row.assetId) :
                m_pLightResources->Find_RuntimeResource(row.assetId);
            if (!resource)
            {
                m_strStatus = "Light resource unavailable: " + row.assetId + "; other rows preserved.";
                continue;
            }
            const auto append = [&](const float4x4_t& pivot, const float anchorHeight)
            {
                LIGHT_DESC desc{};
                std::string status;
                if (!CLightResourceCatalog::Try_BuildAnchoredLightDesc(*resource, row.lightBox.strAnchorKind,
                    pivot, anchorHeight, row.lightWeight, desc, status))
                { m_strStatus = "Light occurrence isolated: " + id + "; " + status; return; }
                m_LightProvider->lights.push_back({desc, row.debugRender});
            };
            if (row.lightBox.strAnchorKind == "PLAYER")
            {
                // No present character is a temporary empty target set, never a sticky row failure.
                for (const auto& root : m_LightPlayerPivots)
                {
                    float4x4_t pivot;
                    if (Make_Pivot(row.lightBox, root, nullptr, pivot)) append(pivot, root._42);
                }
            }
            else
            {
                append(row.pivot, row.placementAnchor._42);
                // A Server-owned same-body actor shares its owner's following
                // spotlight. Gaze clones retain their own transform and animation;
                // the original occurrence still owns timing, fade and brightness.
                if (preview || resource->eType != LIGHT::SPOT ||
                    row.lightBox.strAnchorKind != "BOSS" || !row.lightBox.bFollowBoss) continue;
                const auto followers = m_LightBossFollowers.find(bossId);
                if (followers == m_LightBossFollowers.end()) continue;
                for (const auto& follower : followers->second)
                {
                    const auto own = m_BossSessions.find(follower.entityId);
                    const bool hasOwnLight = own != m_BossSessions.end() &&
                        std::any_of(own->second.rows.begin(), own->second.rows.end(),
                            [&](const auto& entry)
                            {
                                const auto& candidate = entry.second;
                                return candidate.kind == KIND::LIGHT && candidate.assetId == row.assetId &&
                                    candidate.lightBox.strAnchorKind == "BOSS" && !candidate.failed &&
                                    !candidate.waitingForAnchor && candidate.lightWeight > 0.f;
                            });
                    if (hasOwnLight) continue;
                    const auto npc = follower.npc.lock();
                    float4x4_t pivot;
                    if (npc && npc->Get_Transform() && Make_Pivot(row.lightBox,
                        *npc->Get_Transform()->Get_WorldMatrixPtr(), npc->Get_Model(), pivot))
                        append(pivot, npc->Get_Transform()->Get_WorldMatrixPtr()->_42);
                }
            }
        }
    };
    for (const auto& tail : m_ProductTails) collect(tail.playback, false, tail.playback.bossEntityId);
    for (const auto& [id, session] : m_BossSessions) collect(session, false, id);
    for (const auto& [id, session] : m_ChildBossSessions) collect(session, false, id);
    for (const auto& [id, session] : m_MarioEntrySessions) collect(session, false, id);
    if (m_bPreviewPlaying) collect(m_PreviewSession, true);
    for (const auto& member : m_BundlePreviewMembers) collect(member.session, true);
    for (const auto& [id, stage] : m_ExternalStageEnvironments) collect(stage.session, stage.preview);
    collect(m_FearSession, false);
    if (!m_LightProvider->lights.empty())
        if (FAILED(CPresentation_Manager::Get().Add_FrameProvider(m_LightProvider)))
            m_strStatus = "Light frame provider registration failed.";
}

bool Client::CKoukuSaydonPresentationPlayer::Resolve_SourceAnchorWorlds(
    const EFFECT_DOCUMENT_DESC& document, const EFFECT_V2_TARGET_VIEW& view,
    const float4x4_t& root, std::unordered_map<std::string, float4x4_t>& anchors, std::string& error)
{
    const auto attachments = Source_Attachments(document);
    SOURCE_BONES bones;
    for (const auto& attachment : attachments)
    {
        if (attachment.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW ||
            bones.contains(attachment.strRuntimeBoneName)) continue;
        if (!view.pModel || !view.pModel->Has_Bone(attachment.strRuntimeBoneName.c_str()))
        { error = "Selected Kouku model has no source bone: " + attachment.strRuntimeBoneName; return false; }
        float4x4_t value;
        XMStoreFloat4x4(&value, view.pModel->Get_BoneMatrix(attachment.strRuntimeBoneName.c_str()));
        bones.emplace(attachment.strRuntimeBoneName, value);
    }
    return Build_SourceAnchorWorlds(attachments, view, root, bones, anchors, error);
}

bool Client::CKoukuSaydonPresentationPlayer::Sample_SourceAnchorWorlds(
    const EFFECT_DOCUMENT_DESC& document, const EFFECT_V2_TARGET_VIEW& view,
    const float4x4_t& root, const float seconds, SOURCE_BONES& anchors, std::string& error,
    const EFFECT_SOURCE_MODEL_PREVIEW* sourceOverride)
{
    const auto* sourceModel = sourceOverride ? sourceOverride :
        (document.SourceModelPreview ? &*document.SourceModelPreview : nullptr);
    if (!sourceModel) return Resolve_SourceAnchorWorlds(document, view, root, anchors, error);
    if (!std::isfinite(seconds) || seconds < 0.f)
    { error = "Source model animation time must be finite and nonnegative."; return false; }
    const auto attachments = Source_Attachments(document);
    std::vector<std::string> names;
    for (const auto& attachment : attachments)
        if (attachment.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW &&
            std::find(names.begin(), names.end(), attachment.strRuntimeBoneName) == names.end())
            names.push_back(attachment.strRuntimeBoneName);
    std::vector<ANCHOR_ANIMATION> animations;
    for (const auto& source : sourceModel->Animations)
    {
        ANCHOR_ANIMATION animation;
        animation.strRuntimeClip = source.strRuntimeClip;
        animation.iStartOffsetMs = source.iStartOffsetMs; animation.iSourceStartMs = source.iSourceStartMs;
        animation.iPoseStartMs = source.iStartOffsetMs;
        animation.iPlayMs = source.iPlayMs; animation.fPlayRate = source.fPlayRate; animation.strEndPolicy = source.strEndPolicy;
        animations.push_back(std::move(animation));
    }
    SOURCE_BONES bones;
    return Sample_SourceBones(view.pModel, animations, {}, seconds * 1000.f, names, bones, error) &&
        Build_SourceAnchorWorlds(attachments, view, root, bones, anchors, error);
}

Client::CKoukuSaydonPresentationPlayer::CKoukuSaydonPresentationPlayer(
    ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context,
    CRenderingProfileService& profiles)
    : m_Device(std::move(device)), m_Context(std::move(context)), m_Profiles(profiles)
{
    XMStoreFloat4x4(&m_PreviewPivot, XMMatrixIdentity());
}

Client::CKoukuSaydonPresentationPlayer::~CKoukuSaydonPresentationPlayer()
{
    Reset();
}

Client::CKoukuSaydonPresentationPlayer::WORLD_EMISSION_ANCHOR
Client::CKoukuSaydonPresentationPlayer::Make_WorldEmissionAnchor(
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
    const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION& world,
    const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE& occurrence)
{
    if (!pattern.BossMotion || occurrence.Placement || world.strAnchorKind != "BOSS_SPAWN") return {};
    KOUKU_SAYDON_COMPOSITION_PATTERN sampled;
    sampled.BossMotion = pattern.BossMotion;
    return [sampled = std::move(sampled), world, startMs = occurrence.iStartMs](
        const f32_t birthMs, float4x4_t& out)
    {
        std::array<double, 3u> position{};
        double yaw = 0.0;
        if (!Sample_KoukuSaydonBossMotion(sampled, double(startMs) + birthMs, position, yaw)) return false;
        for (std::size_t axis = 0; axis < position.size(); ++axis)
            position[axis] += world.PositionOffset[axis] - world.AnchorPosition[axis];
        XMStoreFloat4x4(&out, XMMatrixRotationY(XMConvertToRadians(float(yaw))) *
            XMMatrixTranslation(float(position[0]), float(position[1]), float(position[2])));
        return true;
    };
}

bool Client::CKoukuSaydonPresentationPlayer::Resolve_ProductWorldEmissionAnchor(
    const std::uint32_t sourceRevision, const std::string_view patternId,
    const std::string_view occurrenceId, WORLD_EMISSION_ANCHOR& out) const
{
    if (sourceRevision != m_iProductSourceRevision) return false;
    // An automatic RaidFlow continuation may no longer list the old member.
    // Its reliable cue still carries its original occurrence and pinned revision.
    if (!patternId.empty() && !m_Product.contains(std::string(patternId))) return false;
    // Reliable delayed births keep their globally stable occurrence ID even
    // after the member has advanced. Resolve against the pinned Product, never
    // silently replace an old motion sampler with the current Pattern's default.
    const WORLD_EMISSION_ANCHOR* resolved = nullptr;
    for (const auto& [id, owner] : m_Product)
        if (const auto anchor = owner.worldEmissionAnchors.find(std::string(occurrenceId));
            anchor != owner.worldEmissionAnchors.end())
        {
            if (resolved) return false;
            resolved = &anchor->second;
        }
    out = resolved ? *resolved : WORLD_EMISSION_ANCHOR{};
    return true;
}

bool Client::CKoukuSaydonPresentationPlayer::Is_TargetedCombatObjectArchetype(
    const std::string_view archetypeId) noexcept
{
    return archetypeId == "combatobject.kouku.showtime.fixed" ||
        archetypeId == "combatobject.kouku.showtime.tracking" ||
        archetypeId == "combatobject.kouku.pursuit";
}

Client::CKoukuSaydonPresentationPlayer::TARGETED_COMBAT_VISUALS
Client::CKoukuSaydonPresentationPlayer::Read_TargetedCombatVisuals(const DATA_JSON_VALUE& root)
{
    TARGETED_COMBAT_VISUALS staged;
    const auto* definitions = root.Find("targetedCombatVisuals");
    if (!definitions) return staged;
    if (!definitions->Is_Array() || definitions->Get_Array().size() > 8192u)
        throw std::runtime_error("Targeted combat visuals require a bounded array.");
    const auto stableId = [](const std::string& id) {
        return !id.empty() && id.size() <= 128u && id != "." && id != ".." &&
            std::all_of(id.begin(), id.end(), [](const unsigned char c) {
                return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') || c == '.' || c == '-' || c == '_'; });
    };
    for (const auto& row : definitions->Get_Array())
    {
        auto definition = std::make_shared<TARGETED_COMBAT_VISUAL>();
        const auto id = Text(row, "clientVisualId");
        definition->archetypeId = Text(row, "combatObjectArchetypeId");
        const auto& loop = Field(row, "loop");
        if (!stableId(id) || !Is_TargetedCombatObjectArchetype(definition->archetypeId) || !loop.Is_Boolean())
            throw std::runtime_error("Invalid targeted combat visual identity or loop.");
        definition->loop = loop.Get_Boolean();
        if (definition->loop != (definition->archetypeId != "combatobject.kouku.showtime.fixed"))
            throw std::runtime_error("Targeted visual loop must match its fixed/tracking archetype.");
        if (definition->archetypeId == "combatobject.kouku.pursuit")
        {
            definition->contactVisualId = Text(row, "contactVisualId");
            definition->contactEffectAssetId = Text(row, "contactEffectAssetId");
            if (!stableId(definition->contactVisualId) ||
                !CEffectV2Document::Is_ValidEffectId(definition->contactEffectAssetId))
                throw std::runtime_error("Pursuit visual requires its exact contact Effect identity.");
        }
        auto& presentation = definition->presentation;
        presentation.pattern.strPatternId = id;
        presentation.durationMs = UInt(row, "durationMs", 1u, MAX_TIMELINE_MS);
        auto& sourceBossPresentation = definition->sourceBossPresentation;
        sourceBossPresentation.pattern.strPatternId = id;
        sourceBossPresentation.durationMs = presentation.durationMs;
        const auto& resources = Field(row, "resources");
        const auto& occurrences = Field(row, "occurrences");
        if (!resources.Is_Array() || resources.Get_Array().empty() || resources.Get_Array().size() > 1024u ||
            !occurrences.Is_Array() || occurrences.Get_Array().empty() || occurrences.Get_Array().size() > 1024u)
            throw std::runtime_error("Targeted visual resources/occurrences require bounded nonempty arrays.");
        std::map<std::string, KIND> kinds;
        for (const auto& value : resources.Get_Array())
        {
            auto resource = Read_Resource(value);
            if ((resource.eKind != KIND::EFFECT && !(resource.eKind == KIND::SOUND && !definition->loop)) ||
                !stableId(resource.strResourceId) || !kinds.emplace(resource.strResourceId, resource.eKind).second)
                throw std::runtime_error("Targeted visual requires unique Effect resources or finite Sound cues.");
            presentation.document.PresentationResources.push_back(std::move(resource));
        }
        sourceBossPresentation.document = presentation.document;
        std::set<std::string> ids;
        for (const auto& value : occurrences.Get_Array())
        {
            const auto resource = kinds.find(Text(value, "resourceId"));
            if (resource == kinds.end()) throw std::runtime_error("Targeted occurrence has no resource.");
            auto box = Read_Occurrence(value, presentation.durationMs, resource->second);
            const bool sourceBoss = box.strAnchorKind == "BOSS" && box.bFollowBoss && !definition->loop;
            const bool map = box.strAnchorKind == "MAP" && !box.bFollowBoss;
            if (!stableId(box.strOccurrenceId) || !ids.insert(box.strOccurrenceId).second ||
                (resource->second == KIND::SOUND && !map) || (!map && !sourceBoss) || !box.strBone.empty() || box.strBoneTarget != "BODY" ||
                !box.strWorldId.empty() || !box.strWorldOccurrenceId.empty() ||
                !box.strLogicOccurrenceId.empty() || box.iWorldEmissionIndex != 0u)
                throw std::runtime_error("Targeted occurrence requires a relative MAP or following source BOSS placement.");
            if (sourceBoss)
            {
                sourceBossPresentation.pattern.PresentationOccurrences.push_back(std::move(box));
                continue;
            }
            // Private execution copy: the existing root/follow sampler consumes the
            // authoritative CombatObject translation, without a boss model or yaw.
            // Published MAP offsets and the ordinary fixed-MAP path are unchanged.
            box.strAnchorKind = "BOSS";
            box.bFollowBoss = definition->loop;
            presentation.pattern.PresentationOccurrences.push_back(std::move(box));
        }
        if (std::none_of(presentation.pattern.PresentationOccurrences.begin(), presentation.pattern.PresentationOccurrences.end(),
            [&](const auto& box) { return kinds.at(box.strResourceId) == KIND::EFFECT; }))
            throw std::runtime_error("Targeted visual requires at least one MAP Effect occurrence.");
        if (!staged.emplace(id, std::move(definition)).second)
            throw std::runtime_error("Duplicate targeted combat visual identity.");
    }
    return staged;
}

bool Client::CKoukuSaydonPresentationPlayer::Start_TargetedCombatVisual(
    const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawn, std::string& status)
{
    if (!Is_TargetedCombatObjectArchetype(spawn.strCombatObjectArchetypeId) ||
        !spawn.iCombatObjectId || !spawn.iSourceNetEntityId || !spawn.PinnedDefinitionRevision.Is_Valid() ||
        !std::isfinite(spawn.fPositionX) || !std::isfinite(spawn.fPositionY) || !std::isfinite(spawn.fPositionZ) || !std::isfinite(spawn.fYawDegrees) ||
        !std::isfinite(spawn.fUniformScale) || spawn.fUniformScale < .01f || spawn.fUniformScale > 10.f)
    { status = "Targeted combat visual spawn identity or position is invalid."; return false; }
    float age = 0.f;
    if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(spawn.iServerTick, spawn.iSpawnTick, 30.f, age))
    { status = "Targeted combat visual spawn clock is invalid."; return false; }
    if (m_TargetedCombatSessions.contains(spawn.iCombatObjectId))
    { status = "Targeted combat visual already has a live owner."; return false; }
    auto definition = m_TargetedCombatVisuals.find(spawn.strClientVisualId);
    if (definition == m_TargetedCombatVisuals.end())
    {
        try
        {
            // A content-addressed new visual can arrive before MainApp observes
            // the new Product run. Refresh this registry only; old sessions pin
            // their immutable definitions and keep their existing child clocks.
            auto staged = Read_TargetedCombatVisuals(Read_ProductPresentationRoot());
            m_TargetedCombatVisuals = std::move(staged);
        }
        catch (const std::exception& error) { status = error.what(); return false; }
        definition = m_TargetedCombatVisuals.find(spawn.strClientVisualId);
    }
    if (definition == m_TargetedCombatVisuals.end() ||
        definition->second->archetypeId != spawn.strCombatObjectArchetypeId)
    { status = "Targeted combat visual has no exact published ID/archetype join."; return false; }
    TARGETED_COMBAT_SESSION candidate;
    candidate.definition = definition->second;
    candidate.sourceId = spawn.iSourceNetEntityId;
    candidate.pinnedRevision = spawn.PinnedDefinitionRevision;
    candidate.spawnTick = spawn.iSpawnTick;
    candidate.serverTick = spawn.iServerTick;
    candidate.elapsedMs = double(age) * 1000.0;
    candidate.uniformScale = spawn.fUniformScale;
    candidate.playback.key = "targeted:" + std::to_string(spawn.iCombatObjectId) + ":" +
        std::to_string(spawn.iSpawnTick);
    candidate.sourceBossPlayback.key = candidate.playback.key + ":source-boss";
    XMStoreFloat4x4(&candidate.root,
        XMMatrixScaling(spawn.fUniformScale, spawn.fUniformScale, spawn.fUniformScale) *
        XMMatrixRotationY(spawn.strCombatObjectArchetypeId == "combatobject.kouku.pursuit" ? XMConvertToRadians(spawn.fYawDegrees) : 0.f) *
        XMMatrixTranslation(spawn.fPositionX, spawn.fPositionY, spawn.fPositionZ));
    const auto [entry, inserted] = m_TargetedCombatSessions.emplace(spawn.iCombatObjectId, std::move(candidate));
    if (!inserted) { status = "Targeted combat visual identity collided."; return false; }
    if (!Sample_TargetedCombatVisual(entry->second))
    {
        status = entry->second.failure;
        Stop_TargetedCombatVisual(spawn.iCombatObjectId);
        return false;
    }
    status = "Started targeted combat visual from the Server clock.";
    return true;
}

bool Client::CKoukuSaydonPresentationPlayer::Update_TargetedCombatVisual(
    const LostArk::Shared::COMBAT_OBJECT_SNAPSHOT& snapshot, const std::uint32_t serverTick,
    std::string& status)
{
    const auto found = m_TargetedCombatSessions.find(snapshot.iCombatObjectId);
    if (found == m_TargetedCombatSessions.end())
    { status = "Targeted combat visual has no live session."; return false; }
    auto& session = found->second;
    float age = 0.f;
    if (session.sourceId != snapshot.iSourceNetEntityId || session.pinnedRevision != snapshot.PinnedDefinitionRevision ||
        !std::isfinite(snapshot.fPositionX) || !std::isfinite(snapshot.fPositionY) || !std::isfinite(snapshot.fPositionZ) || !std::isfinite(snapshot.fYawDegrees) ||
        !CActionPresentationTimeline::Try_ResolveActionAgeSeconds(serverTick, session.spawnTick, 30.f, age))
    { status = "Targeted combat visual snapshot identity, pose or clock changed."; return false; }
    if (!session.failure.empty()) { status = session.failure; return false; }
    session.serverTick = serverTick;
    session.elapsedMs = (std::max)(session.elapsedMs, double(age) * 1000.0);
    if (session.definition->loop)
        XMStoreFloat4x4(&session.root,
            XMMatrixScaling(session.uniformScale, session.uniformScale, session.uniformScale) *
            XMMatrixRotationY(session.definition->archetypeId == "combatobject.kouku.pursuit" ? XMConvertToRadians(snapshot.fYawDegrees) : 0.f) *
            XMMatrixTranslation(snapshot.fPositionX, snapshot.fPositionY, snapshot.fPositionZ));
    // MainApp samples all child groups once per presentation frame after the
    // complete snapshot batch. No Client tracking velocity is synthesized.
    return true;
}

bool Client::CKoukuSaydonPresentationPlayer::Sample_TargetedCombatVisual(
    TARGETED_COMBAT_SESSION& session, const KOUKU_BOSS_PRESENTATION_VIEW* sourceBoss)
{
    if (session.finished) return true;
    if (!session.failure.empty()) return false;
    const auto& presentation = session.definition->presentation;
    if (!session.definition->loop && session.elapsedMs >= presentation.durationMs)
    {
        Stop_Session(session.playback);
        Stop_Session(session.sourceBossPlayback);
        session.finished = true;
        return true;
    }
    const auto cycle = session.definition->loop ?
        static_cast<std::uint64_t>(session.elapsedMs / presentation.durationMs) : 0u;
    if (cycle != session.cycle)
    {
        Stop_Session(session.playback);
        Stop_Session(session.sourceBossPlayback);
        session.cycle = cycle;
    }
    const float clock = static_cast<float>(session.definition->loop ?
        std::fmod(session.elapsedMs, double(presentation.durationMs)) : session.elapsedMs);
    if (session.definition->loop && !session.playback.rootHistory)
    {
        // A late join has no earlier tracking snapshots. Bind pre-join births
        // to the first authoritative pose, then record only observed movement.
        // This seeds the existing sampler without inventing a tracking path.
        session.playback.rootHistory = std::make_shared<EFFECT_V2_PIVOT_HISTORY>();
        std::string historyStatus;
        if (!session.playback.rootHistory->Record(0.f, session.root, false, historyStatus))
        { session.failure = std::move(historyStatus); return false; }
        session.playback.rootRecordedSeconds = 0.f;
        session.playback.rootRecordedPivot = session.root;
    }
    Sample(session.playback, presentation.document, presentation.pattern, clock, false, session.root, nullptr);
    const auto& sourcePresentation = session.definition->sourceBossPresentation;
    bool sampledSourceBoss = false;
    if (!sourcePresentation.pattern.PresentationOccurrences.empty() && sourceBoss &&
        sourceBoss->Snapshot.iNetEntityId == session.sourceId)
    {
        if (const auto npc = sourceBoss->pNpc.lock(); npc && npc->Get_Transform())
        {
            const auto& pivot = *npc->Get_Transform()->Get_WorldMatrixPtr();
            if (!session.sourceBossPlayback.rootHistory)
            {
                // Spawns can precede the first actor view. As with tracking late
                // joins, bind earlier particle births to the first observed pose.
                session.sourceBossPlayback.rootHistory = std::make_shared<EFFECT_V2_PIVOT_HISTORY>();
                std::string historyStatus;
                if (!session.sourceBossPlayback.rootHistory->Record(0.f, pivot, false, historyStatus))
                {
                    session.failure = std::move(historyStatus);
                    Stop_Session(session.playback);
                    Stop_Session(session.sourceBossPlayback);
                    return false;
                }
                session.sourceBossPlayback.rootRecordedSeconds = 0.f;
                session.sourceBossPlayback.rootRecordedPivot = pivot;
            }
            // Both placements use the same Server birth clock. The muzzle follows
            // its actual source actor; only ground children use the random root.
            Sample(session.sourceBossPlayback, sourcePresentation.document, sourcePresentation.pattern,
                clock, false, pivot, npc->Get_Model());
            sampledSourceBoss = true;
        }
    }
    if (!sampledSourceBoss && (session.sourceBossPlayback.rootHistory || !session.sourceBossPlayback.rows.empty()))
        Stop_Session(session.sourceBossPlayback);
    for (const auto* playback : { &session.playback, &session.sourceBossPlayback })
        for (const auto& [id, row] : playback->rows)
            if (row.failed)
            {
                session.failure = "Targeted combat visual " + id + ": " +
                    (row.failureStatus.empty() ? m_strStatus : row.failureStatus);
                break;
            }
    if (session.failure.empty()) return true;
    Stop_Session(session.playback);
    Stop_Session(session.sourceBossPlayback);
    return false;
}

void Client::CKoukuSaydonPresentationPlayer::Update_TargetedCombatVisuals(
    const float dt, const std::vector<KOUKU_BOSS_PRESENTATION_VIEW>& bosses)
{
    for (auto& [id, session] : m_TargetedCombatSessions)
    {
        (void)id;
        if (!session.finished && session.failure.empty()) session.elapsedMs += double(dt) * 1000.0;
        const auto source = std::find_if(bosses.begin(), bosses.end(), [&](const auto& boss) {
            return boss.Snapshot.iNetEntityId == session.sourceId;
        });
        if (!Sample_TargetedCombatVisual(session, source == bosses.end() ? nullptr : &*source))
            m_strStatus = session.failure;
    }
}

bool Client::CKoukuSaydonPresentationPlayer::Play_TargetedCombatContact(
    const LostArk::Shared::S2C_COMBAT_OBJECT_PRESENTATION_EVENT& event, std::string& status)
{
    const auto found = m_TargetedCombatSessions.find(event.iCombatObjectId);
    if (found == m_TargetedCombatSessions.end() || !event.iEventSequence || !event.iServerTick)
    { status = "Pursuit contact has no admitted visual occurrence."; return false; }
    auto& session = found->second;
    if (session.definition->archetypeId != "combatobject.kouku.pursuit" ||
        event.strCombatObjectArchetypeId != session.definition->archetypeId ||
        event.iSourceNetEntityId != session.sourceId || event.PinnedDefinitionRevision != session.pinnedRevision ||
        event.eKind != LostArk::Shared::COMBAT_OBJECT_PRESENTATION_EVENT_KIND::HIT_PULSE ||
        event.iRepeatIndex != 0u || event.strHitId != session.definition->contactVisualId ||
        !std::isfinite(event.fPositionX) || !std::isfinite(event.fPositionY) ||
        !std::isfinite(event.fPositionZ) || !std::isfinite(event.fYawDegrees))
    { status = "Pursuit contact does not match its pinned visual and Server pose."; return false; }
    if (session.contactReceived) return true;
    if (m_ContactCombatEffects.size() >= LostArk::Shared::MAX_COMBAT_OBJECTS_PER_SNAPSHOT)
    { status = "Pending contact Effect capacity reached."; return false; }
    CONTACT_COMBAT_EFFECT pending;
    pending.event = event;
    pending.definition = session.definition;
    m_ContactCombatEffects.emplace(event.iEventSequence, std::move(pending));
    session.contactReceived = true;
    // The reliable despawn may follow in this batch. The contact owns its own
    // immutable definition and pose while the shared preparation queue settles.
    return true;
}

void Client::CKoukuSaydonPresentationPlayer::Update_ContactCombatEffects(const float dt)
{
    for (auto it = m_ContactCombatEffects.begin(); it != m_ContactCombatEffects.end();)
    {
        auto& contact = it->second;
        const std::vector<std::string> targets{ contact.definition->contactEffectAssetId };
        if (!contact.queued)
        {
            std::vector<std::string> admitted;
            if (!CEffectPresentationService::Queue_ProductTargets_Priority(targets, admitted, m_strStatus))
            { it = m_ContactCombatEffects.erase(it); continue; }
            contact.queued = true;
        }
        const auto probe = CEffectPresentationService::Get_ProductCuePreparationProbe(targets);
        if (probe.iFailedCount || probe.iUnavailableCount)
        {
            m_strStatus = "Contact Effect preparation failed: " + targets.front() + "; " +
                CEffectPresentationService::Get_ProductCuePreparationFailure(targets.front());
            it = m_ContactCombatEffects.erase(it); continue;
        }
        if (!probe.bCatalogRevisionCurrent || !probe.bSettled)
        { contact.ageSeconds += (std::max)(dt, 0.f); ++it; continue; }
        const auto& event = contact.event;
        EFFECT_LEVEL_PLACEMENT_SPAWN_DESC spawn;
        spawn.iLevelIndex = CGameInstance::Get().Get_CurrentLevelID();
        spawn.strPlacementId = "kouku.contact:" + std::to_string(event.iEventSequence);
        spawn.strEffectAssetId = targets.front();
        spawn.iSpawnTick = event.iServerTick;
        spawn.fInitialSampleTimeSeconds = contact.ageSeconds;
        XMStoreFloat4x4(&spawn.RootWorld, XMMatrixRotationY(XMConvertToRadians(event.fYawDegrees)) *
            XMMatrixTranslation(event.fPositionX, event.fPositionY, event.fPositionZ));
        EFFECT_WORLD_ROOT_HANDLE handle;
        if (!CEffectPresentationService::Spawn_LevelPlacement(spawn, handle, m_strStatus))
            m_strStatus = "Contact Effect spawn failed: " + targets.front() + "; " + m_strStatus;
        it = m_ContactCombatEffects.erase(it);
    }
}

void Client::CKoukuSaydonPresentationPlayer::Stop_TargetedCombatVisual(
    const LostArk::Shared::COMBAT_OBJECT_ID objectId)
{
    const auto found = m_TargetedCombatSessions.find(objectId);
    if (found == m_TargetedCombatSessions.end()) return;
    Stop_Session(found->second.playback);
    Stop_Session(found->second.sourceBossPlayback);
    m_TargetedCombatSessions.erase(found);
}

Client::CKoukuSaydonPresentationPlayer::PRODUCT_REPLACEMENT
Client::CKoukuSaydonPresentationPlayer::Parse_ProductRoot(const DATA_JSON_VALUE& root)
{
    std::string parseStatus;
        const auto& patterns = Field(root, "patterns");
        if (!patterns.Is_Array() || patterns.Get_Array().size() > 4096u)
            throw std::runtime_error("Product patterns must be a bounded array.");
        std::map<std::string, PRODUCT_PATTERN> staged;
        std::size_t isolatedLights = 0u;
        std::size_t isolatedSceneProfiles = 0u;
        std::string isolatedSceneStatus;
        for (const auto& value : patterns.Get_Array())
        {
            PRODUCT_PATTERN item;
            item.pattern.strPatternId = Text(value, "patternId");
            if (value.Find("gateId")) item.pattern.strGateId = Text(value, "gateId");
            if (value.Find("targetBossPlacementId")) item.pattern.strTargetBossPlacementId = Text(value, "targetBossPlacementId");
            if (value.Find("actorProfileId")) item.pattern.strActorProfileId = Text(value, "actorProfileId");
            item.durationMs = UInt(value, "durationMs", 1u, MAX_TIMELINE_MS);
            item.stageDurationMs = value.Find("stageDurationMs") ?
                UInt(value, "stageDurationMs", 1u, item.durationMs) : item.durationMs;
            if (const auto* windows = value.Find("animationBlendWindows"))
                if (!CKoukuSaydonAnimationBlend::Read_ProductWindows(*windows, item.pattern.AnimationBlendWindows, parseStatus))
                    throw std::runtime_error(parseStatus);
            if (const auto* animations = value.Find("sourceAnchorAnimations"))
            {
                if (!animations->Is_Array() || animations->Get_Array().empty() || animations->Get_Array().size() > 4096u)
                    throw std::runtime_error("Product source anchor animations require a bounded nonempty array.");
                KOUKU_SAYDON_COMPOSITION_STAGE stage;
                stage.iDurationMs = item.durationMs;
                for (const auto& source : animations->Get_Array())
                {
                    ANCHOR_ANIMATION animation;
                    animation.strRuntimeClip = Text(source, "runtimeClip");
                    animation.iStartOffsetMs = UInt(source, "startOffsetMs", 0u, item.durationMs - 1u);
                    animation.iPoseStartMs = source.Find("stageStartMs") ? UInt(source, "stageStartMs", 0u, animation.iStartOffsetMs) : animation.iStartOffsetMs;
                    animation.iSourceStartMs = UInt(source, "sourceStartMs", 0u, MAX_TIMELINE_MS);
                    if (source.Find("sourceEndMs")) animation.iSourceEndMs = UInt(source, "sourceEndMs", 0u, MAX_TIMELINE_MS);
                    animation.iPlayMs = UInt(source, "playMs", 1u, MAX_TIMELINE_MS);
                    animation.iBlendInMs = UInt(source, "blendInMs", 0u, 1000u);
                    animation.fPlayRate = float(Number(source, "playRate", 0.01, 100.0));
                    animation.strEndPolicy = Text(source, "endPolicy");
                    if (animation.strEndPolicy != "EXACT" && animation.strEndPolicy != "HOLD_LAST_POSE" &&
                        animation.strEndPolicy != "LOOP_TO_WINDOW")
                        throw std::runtime_error("Unsupported source anchor animation end policy.");
                    if (!stage.AnimationOccurrences.empty() &&
                        animation.iStartOffsetMs <= stage.AnimationOccurrences.back().iStartOffsetMs)
                        throw std::runtime_error("Source anchor animations must have distinct increasing pattern times.");
                    stage.AnimationOccurrences.push_back(std::move(animation));
                }
                item.pattern.Stages.push_back(std::move(stage));
            }
            if (value.Find("animationRootVerticalScale"))
                item.pattern.fAnimationRootVerticalScale = Number(value, "animationRootVerticalScale", 0.0, 1.0);
            if (value.Find("animationRootHorizontalScale"))
                item.pattern.fAnimationRootHorizontalScale = Number(value, "animationRootHorizontalScale", 0.0, 1.0);
            if (const auto* source = value.Find("bossMotion"))
            {
                KOUKU_SAYDON_BOSS_MOTION motion;
                motion.iStartMs = UInt(*source, "startMs", 0u, item.durationMs - 1u);
                motion.iEndMs = UInt(*source, "endMs", motion.iStartMs + 1u, item.durationMs);
                motion.StartPosition = Vector(*source, "startPosition", -100000.0, 100000.0);
                motion.EndPosition = Vector(*source, "endPosition", -100000.0, 100000.0);
                motion.fYawDegrees = Number(*source, "yawDegrees", -360.0, 360.0);
                if (const auto* keys = source->Find("keys"))
                {
                    if (!keys->Is_Array() || keys->Get_Array().size() < 2u || keys->Get_Array().size() > 512u)
                        throw std::runtime_error("Product bossMotion keys require 2..512 samples.");
                    for (const auto& row : keys->Get_Array())
                    {
                        KOUKU_SAYDON_BOSS_MOTION_KEY key;
                        if (!row.Is_Object() || row.Get_Object().size() != 2u)
                            throw std::runtime_error("Product bossMotion key requires timeMs and position.");
                        key.iTimeMs = UInt(row, "timeMs", motion.iStartMs, motion.iEndMs);
                        key.Position = Vector(row, "position", -100000.0, 100000.0);
                        if (!motion.Keys.empty() && key.iTimeMs <= motion.Keys.back().iTimeMs)
                            throw std::runtime_error("Product bossMotion key times must increase strictly.");
                        motion.Keys.push_back(key);
                    }
                    if (motion.Keys.front().iTimeMs != motion.iStartMs || motion.Keys.back().iTimeMs != motion.iEndMs ||
                        motion.Keys.front().Position != motion.StartPosition || motion.Keys.back().Position != motion.EndPosition)
                        throw std::runtime_error("Product bossMotion keys must match its interval and endpoints.");
                }
                if (motion.Keys.empty() && std::abs(motion.StartPosition[1] - motion.EndPosition[1]) > 0.0001)
                    throw std::runtime_error("Product bossMotion must keep its ground height.");
                item.pattern.BossMotion = motion;
            }
            if (const auto* anchors = value.Find("worldEmissionAnchors"))
            {
                if (!item.pattern.BossMotion || !anchors->Is_Array() || anchors->Get_Array().size() > 4096u)
                    throw std::runtime_error("Product WORLD emission anchors require a bounded bossMotion owner.");
                for (const auto& anchor : anchors->Get_Array())
                {
                    KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION world;
                    world.strAnchorKind = "BOSS_SPAWN";
                    world.PositionOffset = Vector(anchor, "positionOffset", -100000.0, 100000.0);
                    world.AnchorPosition = Vector(anchor, "anchorPosition", -100000.0, 100000.0);
                    KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE occurrence;
                    occurrence.strOccurrenceId = Text(anchor, "occurrenceId");
                    occurrence.iStartMs = UInt(anchor, "startMs", 0u, item.durationMs - 1u);
                    if (!item.worldEmissionAnchors.emplace(occurrence.strOccurrenceId,
                        Make_WorldEmissionAnchor(item.pattern, world, occurrence)).second)
                        throw std::runtime_error("Duplicate Product WORLD emission anchor.");
                }
            }
            const auto& boxes = Field(value, "presentationOccurrences");
            if (!boxes.Is_Array() || boxes.Get_Array().size() > 4096u)
                throw std::runtime_error("Product presentationOccurrences must be a bounded array.");
            std::set<std::string> ids;
            for (const auto& box : boxes.Get_Array())
            {
                try
                {
                RESOURCE resource = Read_Resource(box);
                OCCURRENCE occurrence = Read_Occurrence(box, item.durationMs, resource.eKind);
                if (resource.fInnerRadiusM > 0.0 && std::abs(occurrence.Scale[0] - occurrence.Scale[2]) > .0001)
                    throw std::runtime_error("Hollow Collider requires equal X/Z scale.");
                // CAMERA/SOUND may use fixed WORLD coordinates without a World Object.
                // Named World anchors still require their published sequence identity.
                if (occurrence.strAnchorKind == "WORLD" && !occurrence.strWorldId.empty())
                {
                    // The Product pins the published sequence identity; it never
                    // reopens the editable source composition during gameplay.
                    const std::string sequenceId = Text(box, "worldSequenceInstanceId");
                    const auto world = std::find_if(item.document.Worlds.begin(), item.document.Worlds.end(),
                        [&occurrence](const auto& value) { return value.strWorldId == occurrence.strWorldId; });
                    if (world == item.document.Worlds.end())
                    {
                        KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION definition;
                        definition.strWorldId = occurrence.strWorldId;
                        definition.strSequenceInstanceId = sequenceId;
                        item.document.Worlds.push_back(std::move(definition));
                    }
                    else if (world->strSequenceInstanceId != sequenceId)
                        throw std::runtime_error("Conflicting WORLD presentation anchor identity.");
                }
                if (!ids.insert(occurrence.strOccurrenceId).second)
                    throw std::runtime_error("Duplicate Product presentation occurrence.");
                auto old = std::find_if(item.document.PresentationResources.begin(),
                    item.document.PresentationResources.end(), [&resource](const auto& row)
                    { return row.strResourceId == resource.strResourceId; });
                if (old == item.document.PresentationResources.end())
                    item.document.PresentationResources.push_back(std::move(resource));
                else
                {
                    // Scene resourceDurationMs is the individual projected box duration.
                    resource.iDurationMs = old->iDurationMs;
                    if (resource != *old) throw std::runtime_error("Conflicting Product resource identity.");
                }
                item.pattern.PresentationOccurrences.push_back(std::move(occurrence));
                }
                catch (const std::exception& error)
                {
                    const auto* kind = box.Find("kind");
                    if (!kind || !kind->Is_String()) throw;
                    if (kind->Get_String() == "LIGHT")
                    {
                        ++isolatedLights;
                        OutputDebugStringA((std::string("[KoukuSaydonPresentationPlayer] Isolated LIGHT row: ") + error.what() + "\n").c_str());
                    }
                    else if (kind->Get_String() == "SCENE_PROFILE")
                    {
                        ++isolatedSceneProfiles;
                        const auto* id = box.Find("occurrenceId");
                        isolatedSceneStatus = item.pattern.strPatternId + "/" +
                            (id && id->Is_String() ? id->Get_String() : "<missing occurrenceId>") +
                            ": " + error.what();
                        OutputDebugStringA((std::string("[KoukuSaydonPresentationPlayer] Isolated SCENE_PROFILE row: ") +
                            isolatedSceneStatus + "\n").c_str());
                    }
                    else throw;
                }
            }
            for (const auto& occurrence : item.pattern.PresentationOccurrences)
            {
                if (occurrence.strAnchorPresentationOccurrenceId.empty()) continue;
                const auto source = std::find_if(item.pattern.PresentationOccurrences.begin(), item.pattern.PresentationOccurrences.end(),
                    [&](const auto& other) { return other.strOccurrenceId == occurrence.strAnchorPresentationOccurrenceId; });
                const auto resource = source == item.pattern.PresentationOccurrences.end() ? item.document.PresentationResources.end() :
                    std::find_if(item.document.PresentationResources.begin(), item.document.PresentationResources.end(),
                        [&](const auto& value) { return value.strResourceId == source->strResourceId; });
                if (source == item.pattern.PresentationOccurrences.end() || resource == item.document.PresentationResources.end() ||
                    resource->eKind != KIND::EFFECT || source->strAnchorKind != "BOSS" || source->bFollowBoss ||
                    !source->strBone.empty() || source->strBoneTarget != "BODY" || source->iStartMs > occurrence.iStartMs ||
                    occurrence.strAnchorKind != "BOSS" || occurrence.bFollowBoss || !occurrence.strBone.empty() || occurrence.strBoneTarget != "BODY")
                    throw std::runtime_error("Product Collider has an invalid fixed Effect birth anchor.");
            }
            const std::string key = item.pattern.strPatternId;
            if (!staged.emplace(key, std::move(item)).second)
                throw std::runtime_error("Duplicate Product pattern identity.");
        }

        std::map<std::string, PRODUCT_BUNDLE> stagedBundles;
        if (const auto* bundles = root.Find("bundles"))
        {
            if (!bundles->Is_Array() || bundles->Get_Array().size() > 4096u)
                throw std::runtime_error("Product bundles must be a bounded array.");
            for (const auto& value : bundles->Get_Array())
            {
                PRODUCT_BUNDLE item;
                auto& common = item.common;
                common.pattern.strPatternId = Text(value, "bundleId");
                common.pattern.strGateId = Text(value, "gateId");
                common.durationMs = UInt(value, "durationMs", 1u, MAX_TIMELINE_MS);
                const auto& members = Field(value, "members");
                const auto& boxes = Field(value, "presentationOccurrences");
                if (!members.Is_Array() || members.Get_Array().empty() ||
                    members.Get_Array().size() > LostArk::Shared::MAX_KOUKUSAYDON_BUNDLE_MEMBERS ||
                    !boxes.Is_Array() || boxes.Get_Array().size() > 4096u)
                    throw std::runtime_error("Product bundle members/common rows are invalid.");
                std::set<std::string> memberIds, targets, boxIds;
                std::vector<PRESENTATION_WINDOW> globalWindows;
                for (const auto& box : boxes.Get_Array())
                {
                    auto resource = Read_Resource(box);
                    if (resource.eKind != KIND::CAMERA && resource.eKind != KIND::SCENE_PROFILE)
                        throw std::runtime_error("Product bundle common lane has a non-global resource.");
                    auto occurrence = Read_Occurrence(box, common.durationMs, resource.eKind);
                    if (!boxIds.insert(occurrence.strOccurrenceId).second)
                        throw std::runtime_error("Product bundle has duplicate common occurrences.");
                    (void)Admit_PresentationWindow(globalWindows, resource.eKind, occurrence.iStartMs,
                        double(occurrence.iStartMs) + occurrence.iDurationMs + Camera_ReturnMs(resource, false), "common");
                    const auto previous = std::find_if(common.document.PresentationResources.begin(),
                        common.document.PresentationResources.end(), [&](const auto& row)
                        { return row.strResourceId == resource.strResourceId; });
                    if (previous == common.document.PresentationResources.end())
                        common.document.PresentationResources.push_back(std::move(resource));
                    else
                    {
                        resource.iDurationMs = previous->iDurationMs;
                        if (resource != *previous) throw std::runtime_error("Conflicting bundle resource identity.");
                    }
                    common.pattern.PresentationOccurrences.push_back(std::move(occurrence));
                }
                for (const auto& member : members.Get_Array())
                {
                    const auto memberId = Text(member, "memberId");
                    const auto patternId = Text(member, "patternId");
                    const auto targetId = Text(member, "targetBossPlacementId");
                    const auto actorId = Text(member, "actorProfileId");
                    const auto offsetMs = UInt(member, "startOffsetMs", 0u, MAX_TIMELINE_MS);
                    const double offset = std::ceil(double(offsetMs) * 30.0 / 1000.0) * 1000.0 / 30.0;
                    const auto child = staged.find(patternId);
                    if (!memberIds.insert(memberId).second || !targets.insert(targetId).second ||
                        child == staged.end() || child->second.pattern.strGateId != common.pattern.strGateId ||
                        child->second.pattern.strTargetBossPlacementId != targetId || child->second.pattern.strActorProfileId != actorId)
                        throw std::runtime_error("Product bundle has duplicate or inconsistent child targets.");
                    for (const auto& row : child->second.pattern.PresentationOccurrences)
                        for (const auto& resource : child->second.document.PresentationResources)
                            if (resource.strResourceId == row.strResourceId &&
                                (resource.eKind == KIND::CAMERA || resource.eKind == KIND::SCENE_PROFILE) &&
                                !Admit_PresentationWindow(globalWindows, resource.eKind, offset + row.iStartMs,
                                    offset + row.iStartMs + row.iDurationMs + Camera_ReturnMs(resource, false), memberId))
                                throw std::runtime_error("Product bundle global presentation windows overlap.");
                    item.patternIds.push_back(patternId);
                }
                const auto key = common.pattern.strPatternId;
                if (!stagedBundles.emplace(key, std::move(item)).second)
                    throw std::runtime_error("Duplicate Product bundle identity.");
            }
        }

        std::map<std::string, PRODUCT_PATTERN> stagedFear;
        if (const auto* presentations = root.Find("fearPresentations"))
        {
            if (!presentations->Is_Array() || presentations->Get_Array().size() > 4096u)
                throw std::runtime_error("Fear presentations must be a bounded array.");
            for (const auto& value : presentations->Get_Array())
            {
                PRODUCT_PATTERN item;
                item.pattern.strPatternId = Text(value, "presentationId");
                item.durationMs = UInt(value, "durationMs", 1u, MAX_TIMELINE_MS);
                const auto append = [&](RESOURCE resource, std::uint32_t startMs, const std::string& anchor)
                {
                    OCCURRENCE box;
                    box.strOccurrenceId = item.pattern.strPatternId + ":" + resource.strResourceId;
                    box.strResourceId = resource.strResourceId;
                    box.iStartMs = startMs;
                    box.iDurationMs = item.durationMs - startMs;
                    box.strAnchorKind = anchor;
                    box.bDebugRender = false;
                    // Original ScreenPP is one finite native pulse. The requested repetition
                    // is scoped to this FEAR occurrence and ends with Server FEAR state.
                    box.bLoopEffectToDuration = resource.eKind == KIND::EFFECT &&
                        resource.strResourceKind == "V1_EFFECT" &&
                        resource.strAssetId == "effect.kouku.fear.screen.full.restore";
                    item.document.PresentationResources.push_back(std::move(resource));
                    item.pattern.PresentationOccurrences.push_back(std::move(box));
                };
                const auto sceneId = Text(value, "sceneProfileId", true);
                if (!sceneId.empty())
                {
                    RESOURCE scene;
                    scene.strResourceId = "fear.scene";
                    scene.eKind = KIND::SCENE_PROFILE;
                    scene.strResourceKind.clear();
                    scene.strAssetId = sceneId;
                    scene.iDurationMs = item.durationMs;
                    append(std::move(scene), 0u, "BOSS");
                }
                const auto delayMs = UInt(value, "effectDelayMs", 0u, item.durationMs - 1u);
                if (const auto* effect = value.Find("effectResource"); effect && !effect->Is_Null())
                {
                    auto resource = Read_Resource(*effect);
                    if (resource.eKind != KIND::EFFECT) throw std::runtime_error("Fear effectResource must be EFFECT.");
                    append(std::move(resource), delayMs, "BOSS");
                }
                else if (delayMs) throw std::runtime_error("Fear delay requires an Effect resource.");
                if (const auto* sound = value.Find("soundResource"); sound && !sound->Is_Null())
                {
                    auto resource = Read_Resource(*sound);
                    if (resource.eKind != KIND::SOUND) throw std::runtime_error("Fear soundResource must be SOUND.");
                    // One existing SOUND row shares the face onset and FEAR cleanup.
                    append(std::move(resource), delayMs, "PLAYER");
                }
                if (const auto* light = value.Find("lightResource"); light && !light->Is_Null())
                {
                    auto resource = Read_Resource(*light);
                    if (resource.eKind != KIND::LIGHT) throw std::runtime_error("Fear lightResource must be LIGHT.");
                    append(std::move(resource), 0u, "PLAYER");
                }
                const auto key = item.pattern.strPatternId;
                if (!stagedFear.emplace(key, std::move(item)).second)
                    throw std::runtime_error("Duplicate Fear presentation identity.");
            }
        }

        auto stagedTargeted = Read_TargetedCombatVisuals(root);

    return {std::move(staged), std::move(stagedFear), std::move(stagedBundles), std::move(stagedTargeted),
        isolatedLights, isolatedSceneProfiles, std::move(isolatedSceneStatus)};
}

bool Client::CKoukuSaydonPresentationPlayer::Validate_DraftProductJson(
    const std::string& text, const std::uint32_t sourceRevision, std::string& status)
{
    try
    {
        const auto root = Read_ProductPresentationRoot(&text);
        if (!sourceRevision || UInt(root, "sourceRevision", 1u, UINT32_MAX) != sourceRevision)
            throw std::runtime_error("Draft presentation source revision mismatch.");
        const auto staged = Parse_ProductRoot(root);
        if (staged.isolatedLights || staged.isolatedSceneProfiles)
            throw std::runtime_error("Draft presentation contains invalid isolated resource rows.");
        status.clear(); return true;
    }
    catch (const std::exception& error) { status = error.what(); return false; }
}

bool Client::CKoukuSaydonPresentationPlayer::Reload_Product(
    std::string& status, const std::uint32_t expectedSourceRevision)
{
    const auto admittedDraft = CKoukuSaydonPresentationAssetService::Get_AdmittedDraftProduct();
    const auto draftRevision = admittedDraft ? admittedDraft->RowsRevision : LostArk::Shared::GameplayDataRevision{};
    // Content identity distinguishes unsaved drafts with the same source revision.
    if (expectedSourceRevision && m_bProductLoaded && expectedSourceRevision == m_iProductSourceRevision && draftRevision == m_ProductDraftRowsRevision && !draftRevision.Is_Valid())
    { status = "KoukuSaydon presentation already matches the admitted source revision."; return true; }
    m_bProductAttempted = true;
    std::string soundStatus;
    CSoundCueCatalog::Load_ClassSnapshot("KoukuSaydon", m_SoundEventVariants, soundStatus);
    try
    {
        const auto root = Read_ProductPresentationRoot();
        std::string parseStatus;
        const auto sourceRevision = UInt(root, "sourceRevision", 1u, UINT32_MAX);
        if (expectedSourceRevision && sourceRevision != expectedSourceRevision)
            throw std::runtime_error("KoukuSaydon presentation source revision mismatch: requested " +
                std::to_string(expectedSourceRevision) + ", published " + std::to_string(sourceRevision) +
                ". Previous presentation is preserved; publish the matching Product on this Client.");
        auto replacement = Parse_ProductRoot(root);
        auto& staged = replacement.patterns;
        auto& stagedFear = replacement.fears;
        auto& stagedBundles = replacement.bundles;
        auto& stagedTargeted = replacement.targeted;
        const auto isolatedLights = replacement.isolatedLights;
        const auto isolatedSceneProfiles = replacement.isolatedSceneProfiles;
        const auto& isolatedSceneStatus = replacement.isolatedSceneStatus;

        // Only a fully staged replacement may stop the old running presentation.
        for (auto& [id, session] : m_BossSessions) Stop_Session(session);
        for (auto& [id, session] : m_ChildBossSessions) Stop_Session(session);
        m_BossSessions.clear();
        Clear_ProductTails();
        m_ChildBossSessions.clear();
        for (auto& [id, session] : m_MarioEntrySessions) Stop_Session(session);
        m_MarioEntrySessions.clear();
        m_Product = std::move(staged);
        m_TargetedCombatVisuals = std::move(stagedTargeted);
        Stop_Session(m_FearSession);
        m_FearSession.key.clear();
        m_strCompletedFearKey.clear();
        m_FearPresentations = std::move(stagedFear);
        Stop_Session(m_ProductBundleSession);
        m_ProductBundles = std::move(stagedBundles);
        m_iProductSourceRevision = sourceRevision;
        m_ProductDraftRowsRevision = draftRevision;
        m_MissingProductPatterns.clear();
        m_bProductLoaded = true;
        Refresh_SharedPresentation();
        status = "Loaded KoukuSaydon presentation for " + std::to_string(m_Product.size()) + " Product patterns.";
        std::set<std::string> pursuitTargets;
        for (const auto& [id, visual] : m_TargetedCombatVisuals)
            if (visual->archetypeId == "combatobject.kouku.pursuit")
            {
                pursuitTargets.insert(visual->contactEffectAssetId);
                for (const auto& resource : visual->presentation.document.PresentationResources)
                    pursuitTargets.insert(resource.strAssetId);
            }
        if (!pursuitTargets.empty())
        {
            const std::vector<std::string> targets(pursuitTargets.begin(), pursuitTargets.end());
            std::vector<std::string> admitted;
            std::string preparationStatus;
            if (!CEffectPresentationService::Queue_ProductTargets_Priority(targets, admitted, preparationStatus))
                status += " Pursuit Effect preparation: " + preparationStatus;
        }
        if (isolatedLights) status += " Isolated invalid LIGHT rows: " + std::to_string(isolatedLights);
        if (isolatedSceneProfiles) status += " Isolated invalid SCENE_PROFILE rows: " +
            std::to_string(isolatedSceneProfiles) + ". " + isolatedSceneStatus;
        m_strStatus = status;
        try { CNetworkManager::Get().Record_SessionEvent("kouku.product.loaded",
            "revision=" + std::to_string(m_iProductSourceRevision) + " patterns=" + std::to_string(m_Product.size())); }
        catch (...) { }
        return true;
    }
    catch (const std::exception& error)
    {
        status = error.what();
        m_strStatus = status;
        Write_EffectFailureDiagnostic("Kouku.product.load", status);
        OutputDebugStringA(("[KoukuSaydonPresentationPlayer] " + status + "\n").c_str());
        return false;
    }
}

bool Client::CKoukuSaydonPresentationPlayer::Collect_ProductEffectTargets(
    std::vector<std::string>& v1Targets,
    std::vector<std::pair<std::string, std::string>>& v2Targets, std::string& status)
{
    const auto started = GetTickCount64();
    try
    {
        std::set<std::string> v1{ "effect.kouku.card.match.bind.floor", "effect.kouku.card.match.bind.release" };
        std::set<std::pair<std::string, std::string>> v2;
        const auto add = [&](const std::string& kind, const std::string& id) {
            if (!CEffectV2Document::Is_ValidEffectId(id))
                throw std::runtime_error("Invalid raid Effect identity: " + id);
            if (kind == "V1_EFFECT" || kind == "V1_ELEMENT") v1.insert(id);
            else if (kind == "LEAF" || kind == "GROUP") v2.emplace(kind, id);
            else throw std::runtime_error("Unsupported raid Effect resource kind: " + kind);
        };
        const auto array = [](const DATA_JSON_VALUE& owner, const char* field) -> const auto& {
            const auto& value = Field(owner, field);
            if (!value.Is_Array() || value.Get_Array().size() > 16384u)
                throw std::runtime_error(std::string("Raid dependency array is invalid: ") + field);
            return value.Get_Array();
        };
        const auto collectResource = [&](const DATA_JSON_VALUE& row) {
            if (Text(row, "kind") == "EFFECT")
                add(Text(row, "resourceKind"), Text(row, "assetId"));
        };
        const auto product = Read_ProductPresentationRoot();
        for (const auto& pattern : array(product, "patterns"))
            for (const auto& row : array(pattern, "presentationOccurrences")) collectResource(row);
        if (product.Find("fearPresentations"))
            for (const auto& fear : array(product, "fearPresentations"))
                if (const auto* effect = fear.Find("effectResource"); effect && !effect->Is_Null()) collectResource(*effect);
        if (product.Find("targetedCombatVisuals"))
            for (const auto& visual : array(product, "targetedCombatVisuals"))
            {
                for (const auto& row : array(visual, "resources")) collectResource(row);
                if (const auto* contact = visual.Find("contactEffectAssetId"); contact && !contact->Is_Null())
                    add("V1_EFFECT", Text(visual, "contactEffectAssetId"));
            }

        const auto read = [](const std::filesystem::path& path, const char* schema) {
            std::error_code error;
            const auto size = std::filesystem::file_size(path, error);
            if (error || !size || size > 16u * 1024u * 1024u)
                throw std::runtime_error("Raid dependency document is missing/oversized: " + path.string());
            std::ifstream input(path, std::ios::binary);
            const std::string bytes{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
            DATA_JSON_VALUE root; std::string reason;
            if (!input || input.bad() || bytes.size() != size || !CDataJson::Parse(bytes, root, reason) ||
                !root.Is_Object() || Text(root, "schema") != schema)
                throw std::runtime_error("Raid dependency document is invalid: " + path.string() + "; " + reason);
            return root;
        };
        // The live raid cinematics consume this saved Sequence composition too.
        // Only used resources join the closure; the authoring library is larger.
        const auto sequence = read(CKoukuSaydonCompositionDocument::Resolve_SequencePath(),
            "lostark.kouku-saydon-composition");
        std::set<std::string> used;
        for (const auto& pattern : array(sequence, "patterns"))
            for (const auto& row : array(pattern, "presentationOccurrences")) used.insert(Text(row, "resourceId"));
        for (const auto& resource : array(sequence, "presentationResources"))
            if (used.erase(Text(resource, "resourceId"))) collectResource(resource);
        if (!used.empty()) throw std::runtime_error("Missing raid Sequence resource: " + *used.begin());

        const auto world = read(CMapAssetCatalog::Get_MapDataRoot() /
            "LV_LUT_MIDNIGHTC_ED.worldsequences.json", "lostark.world-sequences");
        std::set<std::string> templates;
        for (const auto& instance : array(world, "instances"))
        {
            const auto& enabled = Field(instance, "enabled");
            if (!enabled.Is_Boolean()) throw std::runtime_error("World enabled flag is invalid.");
            if (enabled.Get_Boolean()) templates.insert(Text(instance, "templateId"));
        }
        for (const auto& value : array(world, "templates"))
            if (templates.erase(Text(value, "sequenceId")) && value.Find("effectTracks"))
                for (const auto& effect : array(value, "effectTracks"))
                    add(Text(effect, "resourceKind"), Text(effect, "resourceId"));
        if (!templates.empty()) throw std::runtime_error("Missing raid World template: " + *templates.begin());
        // Server card/ball state selects these without a pattern occurrence.
        for (const char* symbol : {"heart", "spade", "clober", "dia"})
            for (const char* color : {"red", "black"})
                add("GROUP", std::string("boss.kouku.card.") + symbol + "." + color);
        for (const char* color : {"red", "blue", "yellow"})
            add("LEAF", std::string("boss.kouku.ball.smoke.") + color + "_1");
        v1Targets.assign(v1.begin(), v1.end());
        v2Targets.assign(v2.begin(), v2.end());
        g_RaidEffectSnapshots.clear();
        g_RaidEffectGeneration = CEffectV2Runtime::Cache_Generation();
        status = "Raid Effect dependencies: V1=" + std::to_string(v1Targets.size()) +
            ", V2=" + std::to_string(v2Targets.size());
        OutputDebugStringA(("[KoukuPrewarm] " + status + "\n").c_str());
        Write_EffectFailureDiagnostic("Kouku.Loading.Collect",
            "elapsed_ms=" + std::to_string(GetTickCount64() - started) +
            " v1_targets=" + std::to_string(v1Targets.size()) +
            " v2_targets=" + std::to_string(v2Targets.size()));
        return true;
    }
    catch (const std::exception& error) { status = error.what(); return false; }
}

bool Client::CKoukuSaydonPresentationPlayer::Prewarm_ProductEffectResources(
    const ComPtr<ID3D11Device>& device, const ComPtr<ID3D11DeviceContext>& context,
    const std::vector<std::pair<std::string, std::string>>& targets, std::string& status)
{
    const auto started = GetTickCount64();
    try
    {
        std::map<std::string, std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>> staged;
        for (const auto& [kind, id] : targets)
        {
            std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> snapshot;
            if (!CEffectV2Catalog::Get().Load_ResourceSnapshot(kind == "GROUP" ?
                EFFECT_V2_RESOURCE_KIND::GROUP : EFFECT_V2_RESOURCE_KIND::LEAF, id, snapshot, status)) return false;
            EFFECT_V2_GROUP group;
            if (kind == "GROUP")
            {
                const auto* found = snapshot ? snapshot->Find_Group(id) : nullptr;
                if (!found) { status = "Missing raid Effect group after load: " + id; return false; }
                group = *found;
            }
            else
            {
                group.strGroupId = id;
                EFFECT_V2_GROUP_CHILD child;
                child.strChildId = "raid.prewarm.leaf"; child.strResourceId = child.strEffectId = id;
                group.Children.push_back(std::move(child));
            }
            if (!CEffectV2Runtime::Prewarm_Group(group, snapshot, device, context))
            { status = "Raid Effect prewarm failed: " + id + "; " + CEffectV2Runtime::Last_Error(); return false; }
            staged.emplace(kind + ":" + id, std::move(snapshot));
        }
        // Selected Complete Play prepares one resource per frame. Keep immutable
        // snapshots already prepared for other selections under the same cache epoch.
        const auto generation = CEffectV2Runtime::Cache_Generation();
        if (g_RaidEffectGeneration != generation) g_RaidEffectSnapshots.clear();
        for (auto& [key, snapshot] : staged) g_RaidEffectSnapshots.insert_or_assign(key, std::move(snapshot));
        g_RaidEffectGeneration = generation;
        status = "Raid V2 Effect resources prepared: " + std::to_string(targets.size());
        Write_EffectFailureDiagnostic("Kouku.Loading.V2Prepared",
            "elapsed_ms=" + std::to_string(GetTickCount64() - started) +
            " targets=" + std::to_string(targets.size()));
        return true;
    }
    catch (const std::exception& error) { status = "Raid Effect prewarm failed: " + std::string(error.what()); return false; }
}

bool Client::CKoukuSaydonPresentationPlayer::Ensure_EffectResource(
    const std::string& kind, const std::string& asset,
    std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>& snapshot)
{
    const auto generation = CEffectV2Runtime::Cache_Generation();
    if (generation != m_iEffectCacheGeneration)
    {
        m_EffectResources.clear();
        m_EffectResourceFailures.clear();
        m_iEffectCacheGeneration = generation;
    }
    const std::string key = kind + ":" + asset;
    if (g_RaidEffectGeneration != generation) g_RaidEffectSnapshots.clear();
    if (const auto prepared = g_RaidEffectSnapshots.find(key); prepared != g_RaidEffectSnapshots.end())
    { snapshot = prepared->second; return true; }
    if (const auto found = m_EffectResources.find(key); found != m_EffectResources.end())
    { snapshot = found->second; return true; }
    if (const auto failed = m_EffectResourceFailures.find(key); failed != m_EffectResourceFailures.end())
    { m_strStatus = failed->second; return false; }
    EFFECT_V2_RESOURCE_KIND resourceKind;
    if (kind == "GROUP") resourceKind = EFFECT_V2_RESOURCE_KIND::GROUP;
    else if (kind == "LEAF") resourceKind = EFFECT_V2_RESOURCE_KIND::LEAF;
    else { m_strStatus = "Unsupported Effect owner: " + kind; return false; }
    std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> staged;
    if (!CEffectV2Catalog::Get().Load_ResourceSnapshot(resourceKind, asset, staged, m_strStatus))
    {
        m_strStatus = "Effect resource " + asset + ": " + m_strStatus;
        m_EffectResourceFailures.emplace(key, m_strStatus);
        return false;
    }
    m_EffectResources.emplace(key, staged);
    snapshot = std::move(staged);
    return true;
}

void Client::CKoukuSaydonPresentationPlayer::Clear_ProductTails()
{
    for (auto& tail : m_ProductTails) Stop_Session(tail.playback);
    m_ProductTails.clear();
}

void Client::CKoukuSaydonPresentationPlayer::Retire_ProductSession(SESSION& session,
    const std::vector<KOUKU_BOSS_PRESENTATION_VIEW>& bosses)
{
    const auto definition = m_Product.find(session.productPatternId);
    const auto view = std::find_if(bosses.begin(), bosses.end(), [&](const auto& row) {
        return row.Snapshot.iNetEntityId == session.bossEntityId && row.Snapshot.iCurrentHp && !row.pNpc.expired(); });
    const auto* arena = CLevel_KakulSaydonArena::Get_Active();
    const auto* run = arena ? &arena->Get_KoukuBundleState() : nullptr;
    using STATE = LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE;
    const bool cancelled = session.runEpoch && (!run || run->iRunEpoch != session.runEpoch || run->eState == STATE::ABORTED);
    if (cancelled && !session.key.empty())
    {
        auto& cancelledKeys = session.key.find(":child:") == std::string::npos ?
            m_CancelledBossSessionKeys : m_CancelledChildSessionKeys;
        cancelledKeys[session.bossEntityId] = session.key;
    }
    float seconds = 0.f;
    const bool hasClock = view != bosses.end() && CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
        view->iServerTick, session.patternStartTick, 30.f, seconds);
    const bool completedRun = run && session.runEpoch == run->iRunEpoch && run->eState == STATE::COMPLETED;
    const bool nextPattern = view != bosses.end() && !view->Snapshot.strPatternId.empty() &&
        (view->Snapshot.iPatternSequence != session.patternSequence || view->Snapshot.strPatternId != session.productPatternId);
    const bool completed = definition != m_Product.end() && hasClock &&
        (completedRun || nextPattern || seconds * 1000.f + 0.01f >= definition->second.stageDurationMs);
    if (!cancelled && completed && definition->second.durationMs > definition->second.stageDurationMs &&
        seconds * 1000.f < definition->second.durationMs && session.lastClockMs >= 0.f && m_ProductTails.size() < 1024u)
    {
        // Move the handles and observed anchor history; never respawn a sound or
        // particle just because the boss has advanced to the next Pattern.
        PRODUCT_TAIL tail;
        tail.definition = definition->second;
        tail.owner = view->pNpc;
        tail.playback = std::move(session);
        session = {};
        m_ProductTails.push_back(std::move(tail));
        return;
    }
    Stop_Session(session);
    session = {};
}

void Client::CKoukuSaydonPresentationPlayer::Retire_ProductBundleSession()
{
    auto& session = m_ProductBundleSession;
    const auto definition = m_ProductBundles.find(session.productPatternId);
    const auto* arena = CLevel_KakulSaydonArena::Get_Active();
    const auto* run = arena ? &arena->Get_KoukuBundleState() : nullptr;
    using STATE = LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE;
    float seconds = 0.f;
    if (run && session.runEpoch && run->iRunEpoch == session.runEpoch && run->eState != STATE::ABORTED &&
        definition != m_ProductBundles.end() && session.lastClockMs >= 0.f && m_ProductTails.size() < 1024u &&
        CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
            (std::max)(run->iServerTick, arena->Get_PresentationServerTick()), session.patternStartTick, 30.f, seconds) &&
        seconds * 1000.f < definition->second.common.durationMs)
    {
        PRODUCT_TAIL tail;
        tail.bundleCommon = true;
        tail.definition = definition->second.common;
        tail.playback = std::move(session);
        session = {};
        m_ProductTails.push_back(std::move(tail));
        return;
    }
    Stop_Session(session);
    session = {};
}

void Client::CKoukuSaydonPresentationPlayer::Update_ProductTails(const float dt,
    const std::vector<KOUKU_BOSS_PRESENTATION_VIEW>& bosses)
{
    const auto* arena = CLevel_KakulSaydonArena::Get_Active();
    const auto* run = arena ? &arena->Get_KoukuBundleState() : nullptr;
    using STATE = LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE;
    for (auto tail = m_ProductTails.begin(); tail != m_ProductTails.end();)
    {
        auto& session = tail->playback;
        const auto npc = tail->owner.lock();
        const auto view = std::find_if(bosses.begin(), bosses.end(), [&](const auto& row) {
            return row.Snapshot.iNetEntityId == session.bossEntityId && row.Snapshot.iCurrentHp && row.pNpc.lock() == npc; });
        float seconds = 0.f;
        const bool cancelled = (!tail->bundleCommon && (!npc || !npc->Get_Transform() || view == bosses.end())) ||
            (session.runEpoch && (!run || run->iRunEpoch != session.runEpoch || run->eState == STATE::ABORTED));
        const auto serverTick = tail->bundleCommon ? (arena && run ?
            (std::max)(run->iServerTick, arena->Get_PresentationServerTick()) : 0u) :
            (view != bosses.end() ? view->iServerTick : 0u);
        const bool timed = !cancelled && CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
            serverTick, session.patternStartTick, 30.f, seconds);
        const float clock = (std::max)(seconds * 1000.f, session.lastClockMs + dt * 1000.f);
        if (!timed || clock >= tail->definition.durationMs)
        {
            Stop_Session(session);
            tail = m_ProductTails.erase(tail);
            continue;
        }
        if (tail->bundleCommon)
        {
            float4x4_t pivot; XMStoreFloat4x4(&pivot, XMMatrixIdentity());
            Sample(session, tail->definition.document, tail->definition.pattern, clock, false, pivot, nullptr);
        }
        else
        {
            ANIMATION_MODEL_TARGET_VIEW weapon;
            const bool hasWeapon = npc->Try_GetAnimationModelTarget(ANIMATION_BONE_TARGET::WEAPON, weapon);
            Sample(session, tail->definition.document, tail->definition.pattern, clock, false,
                *npc->Get_Transform()->Get_WorldMatrixPtr(), npc->Get_Model(), hasWeapon ? &weapon : nullptr);
        }
        ++tail;
    }
}

void Client::CKoukuSaydonPresentationPlayer::Stop_Session(SESSION& session)
{
    for (auto& [id, row] : session.rows)
    {
        if (row.effectHandle) CEffectV2Runtime::Stop_Group(row.effectHandle);
        if (row.v1EffectHandle) CEffectPresentationService::Stop_WorldRoot({row.v1EffectHandle});
        if (row.soundHandle) CGameInstance::Get().Stop_SoundCue(row.soundHandle);
    }
    session.rows.clear();
    session.rootHistory.reset();
    session.fixedPresentationAnchors.clear();
    session.effectAnchorHistories.clear();
    session.rootRecordedSeconds = -1.f;
    session.lastClockMs = -1.f;
}

void Client::CKoukuSaydonPresentationPlayer::Update_DiceBindVisuals(float dt,
    const std::vector<KOUKU_CARD_PRESENTATION_VIEW>& players)
{
    if (!std::isfinite(dt) || dt < 0.f) return;
    std::set<std::uint32_t> live;
    for (const auto& view : players)
    {
        const auto character = view.pCharacter.lock();
        const auto& snapshot = view.Snapshot;
        if (!character || !character->Get_Transform() || !snapshot.iCurrentHp) continue;
        const auto id = snapshot.iNetEntityId;
        auto found = m_DiceBindVisuals.find(id);
        if (!snapshot.isPatternBound && found == m_DiceBindVisuals.end()) continue;
        live.insert(id);
        bool restart = found == m_DiceBindVisuals.end();
        if (restart) found = m_DiceBindVisuals.try_emplace(id).first;
        auto& visual = found->second;
        restart |= snapshot.isPatternBound && (visual.releasing || visual.endTick != snapshot.iPatternBindEndTick);
        restart |= !snapshot.isPatternBound && !visual.releasing;
        if (restart)
        {
            Stop_Session(visual.session);
            visual.document = {};
            visual.pattern = {};
            visual.clockMs = 0.f;
            visual.releasing = !snapshot.isPatternBound;
            visual.endTick = snapshot.iPatternBindEndTick;
            visual.session.key = "dice-bind:" + std::to_string(id) + ":" +
                std::to_string(visual.endTick) + (visual.releasing ? ":release" : ":hold");
            auto& resource = visual.document.PresentationResources.emplace_back();
            resource.strResourceId = "dice.bind.visual";
            resource.strResourceKind = "V1_EFFECT";
            resource.strAssetId = visual.releasing ? "effect.kouku.card.match.bind.release" :
                "effect.kouku.card.match.bind.floor";
            resource.iDurationMs = visual.releasing ? 6000u : 60000u;
            visual.pattern.strPatternId = "dice.bind.visual";
            visual.pattern.iDurationMs = resource.iDurationMs;
            auto& box = visual.pattern.PresentationOccurrences.emplace_back();
            box.strOccurrenceId = "dice.bind.visual";
            box.strResourceId = resource.strResourceId;
            box.iDurationMs = resource.iDurationMs;
            // The original floor lives at the actor origin; player yaw/scale
            // must not rotate or scale the circular ground projection.
            box.bFollowBoss = true;
        }
        else visual.clockMs += dt * 1000.f;
        if (visual.releasing && visual.clockMs >= visual.pattern.iDurationMs)
        {
            Stop_Session(visual.session);
            m_DiceBindVisuals.erase(found);
            continue;
        }
        const auto& world = *character->Get_Transform()->Get_WorldMatrixPtr();
        float4x4_t pivot;
        XMStoreFloat4x4(&pivot, XMMatrixTranslation(world._41, world._42, world._43));
        // The Server bound bit owns removal, including early card-match release.
        // No locally predicted deadline changes gameplay or restarts the cue.
        Sample(visual.session, visual.document, visual.pattern, visual.clockMs, false, pivot, nullptr);
    }
    for (auto it = m_DiceBindVisuals.begin(); it != m_DiceBindVisuals.end();)
    {
        if (live.contains(it->first)) { ++it; continue; }
        Stop_Session(it->second.session);
        it = m_DiceBindVisuals.erase(it);
    }
}

void Client::CKoukuSaydonPresentationPlayer::Sample(SESSION& session,
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, float clockMs, bool paused,
    const float4x4_t& pivot, const std::shared_ptr<Engine::CModel>& model,
    const ANIMATION_MODEL_TARGET_VIEW* weaponView)
{
    if (!std::isfinite(clockMs) || clockMs < 0.f) return;
    const bool previewSession = &session == &m_PreviewSession || std::any_of(
        m_BundlePreviewMembers.begin(), m_BundlePreviewMembers.end(),
        [&](const auto& member) { return &session == &member.session; }) || std::any_of(
        m_ExternalStageEnvironments.begin(), m_ExternalStageEnvironments.end(),
        [&](const auto& pair) { return pair.second.preview && &session == &pair.second.session; });
    CEffectV2Object::PIVOT_SAMPLER exactRoot;
    const auto previewMember = std::find_if(m_BundlePreviewMembers.begin(), m_BundlePreviewMembers.end(),
        [&](const auto& member) { return &session == &member.session && member.spatialLogicPreview; });
    if (previewMember != m_BundlePreviewMembers.end())
        exactRoot = [this, memberId = previewMember->memberId, generation = m_iPreviewGeneration]
            (float seconds, float4x4_t& output, std::string& error) {
            const auto member = std::find_if(m_BundlePreviewMembers.begin(), m_BundlePreviewMembers.end(),
                [&](const auto& candidate) { return candidate.memberId == memberId; });
            float3_t position;
            float yaw = 0.f;
            if (generation != m_iPreviewGeneration || member == m_BundlePreviewMembers.end() ||
                !Sample_BundlePreviewPose(*member, seconds * 1000.f, position, yaw, false))
            { error = "Spatial preview root has no recorded target input at this source clock."; return false; }
            const matrix_t root = XMLoadFloat4x4(member->actor->Get_Transform()->Get_WorldMatrixPtr());
            XMStoreFloat4x4(&output, XMMatrixScaling(XMVectorGetX(XMVector3Length(root.r[0])),
                XMVectorGetX(XMVector3Length(root.r[1])), XMVectorGetX(XMVector3Length(root.r[2]))) *
                XMMatrixRotationY(XMConvertToRadians(yaw)) * XMMatrixTranslation(position.x, position.y, position.z));
            error.clear(); return true;
        };
    // Preview's external clocks already handle forward/rewind samples. A slow
    // frame must not destroy a live occurrence and its frozen screen capture,
    // or the capture resolver repeatedly returns to the same boundary.
    if (!previewSession && session.lastClockMs >= 0.f &&
        (clockMs < session.lastClockMs - 0.5f || clockMs > session.lastClockMs + 150.f))
    {
        // Same occurrence seek retains observed actor history; a new Server run
        // calls Stop_Session separately and never borrows the previous run.
        auto history = session.rootHistory;
        auto anchorHistories = std::move(session.effectAnchorHistories);
        auto fixedAnchors = std::move(session.fixedPresentationAnchors);
        const auto recordedSeconds = session.rootRecordedSeconds;
        const auto recordedPivot = session.rootRecordedPivot;
        Stop_Session(session);
        session.rootHistory = std::move(history);
        session.effectAnchorHistories = std::move(anchorHistories);
        session.fixedPresentationAnchors = std::move(fixedAnchors);
        session.rootRecordedSeconds = recordedSeconds;
        session.rootRecordedPivot = recordedPivot;
    }
    if (!session.rootHistory)
    {
        session.rootHistory = std::make_shared<EFFECT_V2_PIVOT_HISTORY>();
        // The first replicated frame can arrive after pattern time zero. Bootstrap
        // births with that first authoritative pose until observed samples exist;
        // never borrow a preceding pattern or fabricate later moving history.
        if (!previewSession && clockMs > 0.f)
        {
            std::string historyStatus;
            if (!session.rootHistory->Record(0.f, pivot, false, historyStatus)) m_strStatus = historyStatus;
        }
    }
    const float rootSeconds = clockMs / 1000.f;
    if (rootSeconds >= session.rootRecordedSeconds)
    {
        const float dx = pivot._41 - session.rootRecordedPivot._41;
        const float dy = pivot._42 - session.rootRecordedPivot._42;
        const float dz = pivot._43 - session.rootRecordedPivot._43;
        const bool jump = session.rootRecordedSeconds >= 0.f && dx*dx + dy*dy + dz*dz > 2500.f;
        std::string historyStatus;
        if (session.rootHistory->Record(rootSeconds, pivot, jump, historyStatus))
        { session.rootRecordedSeconds = rootSeconds; session.rootRecordedPivot = pivot; }
        else m_strStatus = std::move(historyStatus);
    }
    // Shared births use the same ceiling-to-30-Hz boundary as Server capture.
    // Delay initial Effect placement until that frame exists, retaining authored age.
    std::set<std::string> sharedBirthSources;
    for (const auto& box : pattern.PresentationOccurrences)
        if (!box.strAnchorPresentationOccurrenceId.empty()) sharedBirthSources.insert(box.strAnchorPresentationOccurrenceId);
    // Keep the shared frame beyond the Effect lifetime for delayed Collider rows.
    for (const auto& box : pattern.PresentationOccurrences)
    {
        if (box.strAnchorPresentationOccurrenceId.empty() || session.fixedPresentationAnchors.contains(box.strAnchorPresentationOccurrenceId)) continue;
        const auto source = std::find_if(pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(),
            [&](const auto& other) { return other.strOccurrenceId == box.strAnchorPresentationOccurrenceId; });
        const auto resource = source == pattern.PresentationOccurrences.end() ? document.PresentationResources.end() :
            std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
                [&](const auto& value) { return value.strResourceId == source->strResourceId; });
        if (source == pattern.PresentationOccurrences.end() || resource == document.PresentationResources.end() || resource->eKind != KIND::EFFECT ||
            source->strAnchorKind != "BOSS" || source->bFollowBoss || !source->strBone.empty() || source->strBoneTarget != "BODY" ||
            box.strAnchorKind != "BOSS" || box.bFollowBoss || !box.strBone.empty() || source->iStartMs > box.iStartMs)
        { m_strStatus = "Collider Effect birth anchor is invalid: " + box.strOccurrenceId; return; }
        const auto captureTick = (std::uint64_t(source->iStartMs) * 30u + 999u) / 1000u;
        const float seconds = float(captureTick) / 30.f;
        if (clockMs + .001f < seconds * 1000.f) continue;
        float4x4_t birth;
        if (!(exactRoot ? exactRoot(seconds, birth, m_strStatus) : session.rootHistory->Sample(seconds, birth, m_strStatus))) return;
        session.fixedPresentationAnchors.emplace(source->strOccurrenceId, birth);
    }
    const auto resolveWorldPivot = [&](const OCCURRENCE& box, float4x4_t& anchor) {
        const auto world = std::find_if(document.Worlds.begin(), document.Worlds.end(),
            [&box](const auto& value) { return value.strWorldId == box.strWorldId; });
        if (world == document.Worlds.end()) return false;
        if (!session.previewWorlds.empty())
        {
            const CWorldSequencePlayer* selected = nullptr;
            for (const auto& [id, player] : session.previewWorlds)
                if ((box.strWorldOccurrenceId.empty() || id == box.strWorldOccurrenceId) &&
                    player->Is_Playing(world->strSequenceInstanceId))
                { if (selected) return false; selected = player.get(); }
            return selected && selected->Try_GetSequencePivot(world->strSequenceInstanceId, anchor, box.iWorldEmissionIndex);
        }
        const auto* level = CLevel_KakulSaydonArena::Get_Active();
        if (!level) return false;
        if (session.runEpoch)
            return level->Try_GetOwnedCompositionWorldPivot(session.runEpoch, session.memberId,
                world->strSequenceInstanceId, box.strWorldOccurrenceId, anchor, box.iWorldEmissionIndex, session.patternSequence);
        return level->Try_GetCompositionWorldPivot(world->strSequenceInstanceId, anchor, box.strWorldOccurrenceId, box.iWorldEmissionIndex);
    };
    // Observe future anchored cues as well as active ones. Their first emission
    // can then interpolate the real samples bracketing the box start even when
    // a render frame crosses that start by more than one fixed step.
    for (const auto& box : pattern.PresentationOccurrences)
    {
        if (!box.bFollowBoss || (box.strAnchorKind != "WORLD" && box.strBone.empty()) ||
            clockMs > double(box.iStartMs) + box.iDurationMs) continue;
        const auto resource = std::find_if(document.PresentationResources.begin(),
            document.PresentationResources.end(), [&box](const auto& row)
            { return row.strResourceId == box.strResourceId && row.eKind == KIND::EFFECT; });
        if (resource == document.PresentationResources.end()) continue;
        auto& history = session.effectAnchorHistories[box.strOccurrenceId];
        if (!history.samples) history.samples = std::make_shared<EFFECT_V2_PIVOT_HISTORY>();
        // Rewinding retains observed poses; it never appends fabricated history.
        if (rootSeconds < history.recordedSeconds) continue;
        auto sampledBox = box;
        float4x4_t anchor = pivot;
        auto anchorModel = model;
        float3_t anchorScale{1.f, 1.f, 1.f};
        if (box.strAnchorKind == "WORLD")
        {
            if (!resolveWorldPivot(box, anchor)) { history.missingSinceSample = true; continue; }
            const matrix_t matrix = XMLoadFloat4x4(&anchor);
            anchorScale = {XMVectorGetX(XMVector3Length(matrix.r[0])),
                XMVectorGetX(XMVector3Length(matrix.r[1])), XMVectorGetX(XMVector3Length(matrix.r[2]))};
            sampledBox.strBone.clear();
            anchorModel.reset();
        }
        float4x4_t unusedPivot, basis;
        if (!Make_Pivot(sampledBox, anchor, anchorModel, unusedPivot, weaponView, &basis))
        { history.missingSinceSample = true; continue; }
        float4x4_t recorded;
        XMStoreFloat4x4(&recorded, XMMatrixScaling(anchorScale.x, anchorScale.y, anchorScale.z) *
            XMLoadFloat4x4(&basis));
        const float dx = recorded._41 - history.recordedPivot._41;
        const float dy = recorded._42 - history.recordedPivot._42;
        const float dz = recorded._43 - history.recordedPivot._43;
        const bool discontinuity = history.recordedSeconds >= 0.f &&
            (history.missingSinceSample || dx*dx + dy*dy + dz*dz > 2500.f);
        std::string historyStatus;
        // A named World may become visible only after its resource preparation.
        // Seed just its initial birth interval from the first resolved pose. All
        // subsequent samples, gaps and teleports retain strict history semantics.
        if (!previewSession && history.recordedSeconds < 0.f)
            history.samples->Record((std::min)(rootSeconds, box.iStartMs / 1000.f),
                recorded, false, historyStatus);
        if (history.samples->Record(rootSeconds, recorded, discontinuity, historyStatus))
        {
            history.recordedSeconds = rootSeconds;
            history.recordedPivot = recorded;
            history.missingSinceSample = false;
        }
        else m_strStatus = "Effect anchor history " + box.strOccurrenceId + ": " + historyStatus;
    }
    std::set<std::string> active;
    const auto sampleOne = [&](const RESOURCE& resource, const OCCURRENCE& box)
    {
        if (clockMs < box.iStartMs || clockMs >= double(box.iStartMs) + box.iDurationMs) return;
        const auto& birthSource = box.strAnchorPresentationOccurrenceId.empty() ? box.strOccurrenceId : box.strAnchorPresentationOccurrenceId;
        if (sharedBirthSources.contains(birthSource) && !session.fixedPresentationAnchors.contains(birthSource)) return;
        active.insert(box.strOccurrenceId);
        auto [found, inserted] = session.rows.try_emplace(box.strOccurrenceId);
        PLAYING_ROW& row = found->second;
        if (row.failed) return;
        struct ROW_FAILURE_GUARD
        {
            PLAYING_ROW& row;
            const std::string& status;
            const std::string& occurrence;
            const std::string& asset;
            ~ROW_FAILURE_GUARD()
            {
                if (row.failed && row.failureStatus.empty())
                {
                    row.failureStatus = status;
                    try
                    {
                        Write_EffectFailureDiagnostic("Kouku.occurrence", "occurrence=" + occurrence +
                            " asset=" + asset + " reason=" + status);
                    }
                    catch (...) { }
                }
            }
        } failureGuard{row, m_strStatus, box.strOccurrenceId, resource.strAssetId};
        const float age = (clockMs - box.iStartMs) / 1000.f;
        const float effectRate = Effect_SourceClockRate(resource, box);
        float effectCycleStart = Effect_SourceCycleStartSeconds(age, row.v1FiniteLoopSeconds);
        float effectAge = (std::max)(0.f, age - effectCycleStart) * effectRate;
        if (row.v1EffectHandle && effectCycleStart != row.v1CycleStartSeconds)
        {
            CEffectPresentationService::Stop_WorldRoot({row.v1EffectHandle});
            row.v1EffectHandle = 0u;
        }
        row.kind = resource.eKind;
        row.debugRender = box.bDebugRender;
        row.startMs = float(box.iStartMs);
        row.cameraDurationMs = box.iDurationMs;
        row.assetId = resource.strAssetId;
        row.subtitleText = resource.strSubtitleText;
        row.subtitleUpper = resource.strSubtitlePosition == "UPPER";
        row.subtitleScreenOffset = { float(box.PositionOffset[0]), float(box.PositionOffset[1]) };
        row.subtitleTextScale = float(box.Scale[0]);
        row.cameraOffset = { float(box.PositionOffset[0]), float(box.PositionOffset[1]), float(box.PositionOffset[2]) };
        if (resource.eKind == KIND::LIGHT)
        {
            row.lightBox = box;
            row.lightWeight = float(box.fBrightnessMultiplier);
            if (box.iFadeInMs) row.lightWeight *= (std::min)(1.f, (clockMs - box.iStartMs) / box.iFadeInMs);
            if (box.iFadeOutMs) row.lightWeight *= (std::min)(1.f, (box.iStartMs + box.iDurationMs - clockMs) / box.iFadeOutMs);
        }
        if ((resource.eKind == KIND::EFFECT || resource.eKind == KIND::LIGHT || resource.eKind == KIND::COLLIDER) &&
            (inserted || box.bFollowBoss || row.waitingForAnchor ||
                (resource.eKind == KIND::COLLIDER && box.strColliderMotion == "LINEAR")) &&
            !(resource.eKind == KIND::LIGHT && box.strAnchorKind == "PLAYER"))
        {
            const auto sampledBox = resource.eKind == KIND::COLLIDER ? Sample_ColliderGeometry(box, clockMs) : box;
            OCCURRENCE placedBox = sampledBox;
            const bool frozenAnchor = resource.eKind == KIND::COLLIDER && !box.bFollowBoss && row.hasPlacementAnchor;
            float4x4_t anchor = frozenAnchor ? row.placementAnchor : pivot;
            const auto sharedBirth = session.fixedPresentationAnchors.find(box.strAnchorPresentationOccurrenceId.empty() ?
                box.strOccurrenceId : box.strAnchorPresentationOccurrenceId);
            if (!frozenAnchor && sharedBirth != session.fixedPresentationAnchors.end()) anchor = sharedBirth->second;
            else if (!frozenAnchor && exactRoot && !box.bFollowBoss && box.strAnchorKind == "BOSS" && box.strBone.empty() &&
                !exactRoot(box.iStartMs / 1000.f, anchor, m_strStatus))
            { row.failed = true; return; }
            auto anchorModel = model;
            if (frozenAnchor)
            {
                for (size_t axis = 0u; axis < 3u; ++axis)
                {
                    placedBox.Scale[axis] *= row.placementAnchorScale[axis];
                    placedBox.PositionOffset[axis] *= row.placementAnchorScale[axis];
                }
                placedBox.strBone.clear();
                anchorModel.reset();
            }
            if (!frozenAnchor && (resource.eKind == KIND::LIGHT || resource.eKind == KIND::EFFECT || resource.eKind == KIND::COLLIDER) && box.strAnchorKind == "MAP")
            { XMStoreFloat4x4(&anchor, XMMatrixIdentity()); anchorModel.reset(); }
            if (!frozenAnchor && box.strAnchorKind == "WORLD")
            {
                if (!resolveWorldPivot(box, anchor))
                {
                    // A WORLD may not have its first sampled pose yet. Retry
                    // presentation anchors next frame instead of hiding this box forever.
                    row.waitingForAnchor = resource.eKind == KIND::COLLIDER || resource.eKind == KIND::EFFECT || resource.eKind == KIND::LIGHT;
                    row.failed = !row.waitingForAnchor;
                    m_strStatus = "WORLD presentation anchor unavailable: " + box.strWorldId;
                    return;
                }
                const matrix_t worldMatrix = XMLoadFloat4x4(&anchor);
                // Lights use metre offsets and retain their source shape, independent of model scale.
                for (size_t axis = 0; resource.eKind != KIND::LIGHT && axis < 3u; ++axis)
                {
                    const float scale = XMVectorGetX(XMVector3Length(worldMatrix.r[axis]));
                    placedBox.Scale[axis] *= scale;
                    placedBox.PositionOffset[axis] *= scale;
                }
                Flatten_CenteredWorldCircle(resource, box, anchor);
                placedBox.strBone.clear();
                anchorModel.reset();
            }
            if (!Make_Pivot(placedBox, anchor, anchorModel, row.pivot, weaponView,
                (resource.eKind == KIND::COLLIDER || resource.eKind == KIND::EFFECT || resource.eKind == KIND::LIGHT) ? &row.placementAnchor : nullptr))
            {
                row.waitingForAnchor = resource.eKind == KIND::LIGHT;
                row.failed = !row.waitingForAnchor;
                m_strStatus = "Presentation bone/pivot is unavailable: " + box.strOccurrenceId;
                return;
            }
            if (row.waitingForAnchor)
                m_strStatus = "Presentation WORLD anchor ready: " + box.strWorldId;
            row.waitingForAnchor = false;
            if (resource.eKind == KIND::COLLIDER)
                row.wire = Collider_Wire(resource, placedBox);
            if (resource.eKind == KIND::COLLIDER || resource.eKind == KIND::EFFECT)
            {
                for (size_t axis = 0u; axis < 3u; ++axis)
                    row.placementAnchorScale[axis] = placedBox.Scale[axis] / sampledBox.Scale[axis];
                row.hasPlacementAnchor = true;
            }
        }
        if (resource.eKind == KIND::EFFECT && box.bFollowBoss &&
            (box.strAnchorKind == "WORLD" || !box.strBone.empty()))
        {
            const auto history = session.effectAnchorHistories.find(box.strOccurrenceId);
            if (history != session.effectAnchorHistories.end()) row.effectPivotHistory = history->second.samples;
        }
        if (inserted || (resource.eKind == KIND::EFFECT && !row.effectHandle && !row.v1EffectHandle))
        {
            switch (resource.eKind)
            {
            case KIND::EFFECT:
            {
                if (resource.strResourceKind == "V1_EFFECT" || resource.strResourceKind == "V1_ELEMENT")
                {
                    const auto catalogRevision = CEffectCatalog::Get_RuntimeRevision();
                    if (catalogRevision != m_iV1CatalogRevision)
                    { m_QueuedV1Effects.clear(); m_iV1CatalogRevision = catalogRevision; }
                    std::vector<std::string> targets{resource.strAssetId};
                    if (m_QueuedV1Effects.insert(resource.strAssetId).second)
                    {
                        std::vector<std::string> admitted;
                        if (!CEffectPresentationService::Queue_ProductTargets_Priority(targets, admitted, m_strStatus))
                        { m_QueuedV1Effects.erase(resource.strAssetId); row.failed = true; break; }
                    }
                    const auto preparation = CEffectPresentationService::Get_ProductCuePreparationProbe(targets);
                    if (preparation.iFailedCount || preparation.iUnavailableCount)
                    {
                        row.failed = true;
                        m_strStatus = "V1 Effect preparation failed: " + resource.strAssetId + "; " +
                            CEffectPresentationService::Get_ProductCuePreparationFailure(resource.strAssetId);
                        break;
                    }
                    if (!preparation.bCatalogRevisionCurrent || !preparation.bSettled) break;
                    bool nativeInfiniteLoop = false;
                    if (box.bLoopEffectToDuration)
                    {
                        const auto sourceDocument = CEffectCatalog::Find_Loaded(resource.strAssetId);
                        if (!sourceDocument)
                        { row.failed = true; m_strStatus = "Prepared looping V1 source is unavailable: " + resource.strAssetId; break; }
                        nativeInfiniteLoop = std::any_of(sourceDocument->Elements.begin(), sourceDocument->Elements.end(),
                            [&](const auto& element) { return
                                (resource.strElementId.empty() || element.strElementId == resource.strElementId) &&
                                element.SourceRecipe.bEnabled && element.SourceRecipe.iEmitterLoopCount == 0u; });
                        if (!nativeInfiniteLoop &&
                            (!CEffectPresentationService::Try_Get_PreparedProductDurationSeconds(resource.strAssetId, row.v1FiniteLoopSeconds) ||
                             !std::isfinite(row.v1FiniteLoopSeconds) || row.v1FiniteLoopSeconds <= 0.f))
                        { row.failed = true; m_strStatus = "Finite V1 loop needs a prepared source duration: " + resource.strAssetId; break; }
                    }
                    if (!box.bLoopEffectToDuration || nativeInfiniteLoop) row.v1FiniteLoopSeconds = 0.f;
                    effectCycleStart = Effect_SourceCycleStartSeconds(age, row.v1FiniteLoopSeconds);
                    effectAge = (std::max)(0.f, age - effectCycleStart) * effectRate;
                    row.sourceAnchorSampler = Make_SourceAnchorSampler(resource, pattern, model, pivot, box.iStartMs,
                        exactRoot, effectRate, effectCycleStart);
                    EFFECT_LEVEL_PLACEMENT_SPAWN_DESC spawn;
                    spawn.iLevelIndex = CGameInstance::Get().Get_CurrentLevelID();
                    spawn.strPlacementId = "kouku:" + session.key + ":" + box.strOccurrenceId;
                    spawn.strEffectAssetId = resource.strAssetId;
                    spawn.strElementId = resource.strElementId;
                    spawn.RootWorld = row.pivot;
                    spawn.fInitialSampleTimeSeconds = effectAge;
                    spawn.bExternallySampled = true;
                    // EmitterLoops=0 uses bounded source emission. Finite sources
                    // repeat their prepared lifetime without mutating the shared asset.
                    spawn.fSourceLoopEndSeconds = nativeInfiniteLoop ? box.iDurationMs / 1000.f : 0.f;
                    EFFECT_WORLD_ROOT_HANDLE handle;
                    if (!CEffectPresentationService::Spawn_LevelPlacement(spawn, handle, m_strStatus))
                    {
                        m_strStatus = "V1 Effect spawn rejected: " + resource.strAssetId + " | " + box.strOccurrenceId + "; " + m_strStatus;
                        row.failed = true;
                    }
                    else { row.v1EffectHandle = handle.iValue; row.v1CycleStartSeconds = effectCycleStart; }
                    break;
                }
                std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> effects;
                if (!Ensure_EffectResource(resource.strResourceKind, resource.strAssetId, effects))
                { row.failed = true; break; }
                EFFECT_V2_GROUP_PLAYBACK_DESC playback;
                playback.PivotWorld = row.pivot;
                playback.fInitialAgeSeconds = age;
                playback.fDurationSeconds = box.iDurationMs / 1000.f;
                // Workbench Fade 0 retains the leaf envelope via the runtime's -1 sentinel.
                playback.fFadeInSeconds = box.iFadeInMs > 0u ? box.iFadeInMs / 1000.f : -1.f;
                playback.fFadeOutSeconds = box.iFadeOutMs > 0u ? box.iFadeOutMs / 1000.f : -1.f;
                playback.fDissolveOutStart = box.iFadeOutMs > 0u ? float(box.fDissolveStart) : -1.f;
                playback.fDissolveOutEnd = box.iFadeOutMs > 0u ? float(box.fDissolveEnd) : -1.f;
                // Server/preview occurrence age owns this lane. Layer Update and
                // MainApp's general Effect clock cannot advance it a second time.
                playback.bProductOwned = true;
                playback.bExternalClock = true;
                playback.PivotSampler = Effect_PivotSampler(box, session.rootHistory, row.effectPivotHistory, exactRoot);
                if (resource.strResourceKind == "GROUP")
                {
                    const auto* group = effects->Find_Group(resource.strAssetId);
                    if (group)
                    {
                        // A particle composition owns emission intervals and tail;
                        // stretching it to the box would manufacture extra births.
                        if (std::any_of(effects->Get_Documents().begin(), effects->Get_Documents().end(),
                            [](const auto& leaf) { return leaf.eType == EFFECT_V2_TYPE::PARTICLE; }))
                            playback.fDurationSeconds = -1.f;
                        row.effectHandle = CEffectV2Runtime::Play_Group(*group,
                            effects, playback, m_Device, m_Context);
                    }
                }
                else if (resource.strResourceKind == "LEAF")
                {
                    const auto* leaf = effects->Find_Document(resource.strAssetId);
                    // Particle boxes include the living particles after emission ends.
                    // Keep the source emitter lifetime/loop; the box still owns final cleanup.
                    if (leaf && leaf->Desc.eShape == CEffectV2Object::SHAPE::PARTICLE)
                        playback.fDurationSeconds = -1.f;
                    row.effectHandle = CEffectV2Runtime::Play_Leaf(resource.strAssetId,
                        effects, playback, m_Device, m_Context);
                }
                if (!row.effectHandle)
                {
                    row.failed = true;
                    m_strStatus = "Effect occurrence unavailable: " + resource.strAssetId + "; " + CEffectV2Runtime::Last_Error();
                }
                break;
            }
            case KIND::SOUND:
            {
                std::string asset = resource.strAssetId;
                if (!resource.strSoundEvent.empty())
                {
                    const auto found = m_SoundEventVariants.find(resource.strSoundEvent);
                    if (found == m_SoundEventVariants.end() || found->second.empty())
                    {
                        row.failed = true;
                        m_strStatus = "Sound event has no admitted assets: " + resource.strSoundEvent;
                        break;
                    }
                    const auto& variants = found->second;
                    // One choice per occurrence remains stable when scrubbing the same cue.
                    // Source Layer/Sequence trees are already rendered into each event variant.
                    std::uint64_t selection = 14695981039346656037ull;
                    // Each authoritative volley keeps a stable choice across snapshots
                    // while distinct CombatObject births may choose different variants.
                    if (session.key.starts_with("targeted:"))
                        for (const unsigned char c : session.key)
                            selection = (selection ^ c) * 1099511628211ull;
                    for (const unsigned char c : box.strOccurrenceId)
                        selection = (selection ^ c) * 1099511628211ull;
                    asset = variants[selection % variants.size()];
                }
                const auto path = CRuntimeAssetRoot::Resolve(asset);
                const auto ageMs = box.iSoundSourceStartMs + static_cast<std::uint32_t>((std::max)(0.f, age) * 1000.f);
                std::uint32_t mediaDurationMs = 0u;
                // A shorter variant may already have completed inside the common cue window.
                // It is a completed one-shot, not a failed cue and never restarts at zero.
                if (!path.empty() && CGameInstance::Get().Get_SoundDurationMs(path.wstring(), mediaDurationMs) &&
                    ageMs >= mediaDurationMs) break;
                if (!path.empty()) row.soundHandle = CGameInstance::Get().Play_SoundCue(
                    path.wstring(), float(box.fVolume), ageMs, paused);
                if (!row.soundHandle)
                {
                    row.failed = true;
                    m_strStatus = "Sound occurrence unavailable: " + resource.strAssetId;
                }
                break;
            }
            case KIND::SUBTITLE: break;
            case KIND::LIGHT: break;
            case KIND::COLLIDER: break;
            case KIND::CAMERA: break;
            case KIND::SCENE_PROFILE:
                if (!m_Profiles.Has_Profile(resource.strAssetId))
                {
                    row.failed = true;
                    m_strStatus = "Scene profile unavailable: " + resource.strAssetId;
                }
                break;
            default:
                row.failed = true;
                m_strStatus = "Unsupported occurrence resource: " + resource.strResourceId;
                break;
            }
        }
        if (row.v1EffectHandle)
        {
            const bool captureSample = previewSession && m_bPreviewCaptureClockHeld;
            CEffectPresentationService::Set_ScreenPostCaptureAllowed({row.v1EffectHandle},
                !captureSample || m_bPreviewCaptureAllowed);
            // A capture boundary continues the same fixed-step history. Commit
            // it before render without replaying every active particle from birth.
            if (!CEffectPresentationService::Update_WorldRoot({row.v1EffectHandle}, row.pivot) ||
                !CEffectPresentationService::Seek_WorldRoot({row.v1EffectHandle}, effectAge,
                    Effect_V1TransformProvider(box, row.pivot, session.rootHistory, row.effectPivotHistory,
                        row.sourceAnchorSampler, effectRate, exactRoot, effectCycleStart),
                    false, ((std::min)(box.iDurationMs / 1000.f - effectCycleStart,
                        row.v1FiniteLoopSeconds > 0.f ? row.v1FiniteLoopSeconds : box.iDurationMs / 1000.f)) * effectRate) || (captureSample && FAILED(
                        CEffectPresentationService::Commit_WorldRootCaptureSample({row.v1EffectHandle}))))
            {
                CEffectPresentationService::Stop_WorldRoot({row.v1EffectHandle});
                row.v1EffectHandle = 0u;
                row.failed = true;
                m_strStatus = "V1 Effect occurrence lost its admitted handle after spawn/attach or rendering failure: " +
                    box.strOccurrenceId + ". " + CEffectPresentationService::Get_Status() +
                    " Correct the resource and restart or seek Preview to retry.";
                return;
            }
        }
        if (row.effectHandle)
        {
            CEffectV2Runtime::Set_GroupPivot(row.effectHandle, row.pivot);
            (void)CEffectV2Runtime::Sample_Group(row.effectHandle, age, paused, m_Device, m_Context);
            std::string failure;
            if (CEffectV2Runtime::Consume_GroupFailure(row.effectHandle, failure))
            {
                CEffectV2Runtime::Stop_Group(row.effectHandle);
                row.effectHandle = 0;
                row.failed = true;
                m_strStatus = std::move(failure);
            }
        }
        if (row.soundHandle) CGameInstance::Get().Pause_SoundCue(row.soundHandle, paused);
        row.lastAge = age;
    };
    for (const auto& box : pattern.PresentationOccurrences)
    {
        // SELECT-owned groups replay once at the captured point through LogicPreview
        // (or the Server combat-object visual), never through this authored MAP lane.
        if (Is_SelectedAirborneGroupMember(document, pattern, box)) continue;
        const auto resource = std::find_if(document.PresentationResources.begin(),
            document.PresentationResources.end(), [&box](const auto& row)
            { return row.strResourceId == box.strResourceId; });
        if (resource != document.PresentationResources.end()) sampleOne(*resource, box);
    }
    for (const auto& sceneBox : pattern.SceneProfileOccurrences)
    {
        const auto profile = std::find_if(document.SceneProfiles.begin(), document.SceneProfiles.end(),
            [&sceneBox](const auto& row) { return row.strSceneProfileId == sceneBox.strSceneProfileId; });
        if (profile == document.SceneProfiles.end()) continue;
        RESOURCE resource;
        resource.eKind = KIND::SCENE_PROFILE;
        resource.strResourceId = profile->strSceneProfileId;
        resource.strAssetId = profile->strRenderingProfileId;
        OCCURRENCE box;
        box.strOccurrenceId = sceneBox.strOccurrenceId;
        box.strResourceId = sceneBox.strSceneProfileId;
        box.iStartMs = sceneBox.iStartMs;
        box.iDurationMs = sceneBox.iDurationMs;
        box.iFadeInMs = sceneBox.iBlendMs;
        sampleOne(resource, box);
    }
    for (auto row = session.rows.begin(); row != session.rows.end();)
    {
        if (active.contains(row->first)) { ++row; continue; }
        if (row->second.effectHandle) CEffectV2Runtime::Stop_Group(row->second.effectHandle);
        if (row->second.v1EffectHandle) CEffectPresentationService::Stop_WorldRoot({row->second.v1EffectHandle});
        if (row->second.soundHandle) CGameInstance::Get().Stop_SoundCue(row->second.soundHandle);
        row = session.rows.erase(row);
    }
    session.lastClockMs = clockMs;
    Sample_LogicPreview(session, document, pattern, clockMs, paused);
}

std::vector<Client::KOUKU_SUBTITLE_VIEW> Client::CKoukuSaydonPresentationPlayer::Collect_Subtitles() const
{
    std::vector<KOUKU_SUBTITLE_VIEW> result;
    const auto collect = [&](const SESSION& session) {
        for (const auto& [id, row] : session.rows)
        {
            if (row.kind != KIND::SUBTITLE || row.failed || row.subtitleText.empty()) continue;
            if (std::none_of(result.begin(), result.end(), [&](const auto& value) {
                return value.strText == row.subtitleText && value.bUpper == row.subtitleUpper &&
                    value.vScreenOffset.x == row.subtitleScreenOffset.x &&
                    value.vScreenOffset.y == row.subtitleScreenOffset.y &&
                    value.fTextScale == row.subtitleTextScale;
            })) result.push_back({row.subtitleText, row.subtitleUpper,
                row.subtitleScreenOffset, row.subtitleTextScale});
        }
    };
    if (m_bPreviewPlaying)
    {
        collect(m_PreviewSession);
        for (const auto& member : m_BundlePreviewMembers) collect(member.session);
    }
    else
    {
        collect(m_ProductBundleSession);
        for (const auto& [id, session] : m_BossSessions) collect(session);
        for (const auto& [id, session] : m_ChildBossSessions) collect(session);
        for (const auto& [id, session] : m_MarioEntrySessions) collect(session);
        for (const auto& tail : m_ProductTails) collect(tail.playback);
    }
    return result;
}

void Client::CKoukuSaydonPresentationPlayer::Restore_Scene()
{
    if (m_bSceneUsed && !m_strScenePrevious.empty() &&
        m_Profiles.Get_ActiveProfileId() == m_strSceneApplied)
    {
        std::string status;
        if (!m_Profiles.Activate_Profile(m_strScenePrevious, status))
        {
            // Keep the owned previous profile until restoration succeeds or an
            // external profile supersedes it; a transient failure is not a release.
            m_strStatus = status;
            return;
        }
    }
    m_bSceneUsed = false;
    m_strScenePrevious.clear();
    m_strSceneOwner.clear();
    m_strSceneApplied.clear();
}

void Client::CKoukuSaydonPresentationPlayer::Refresh_SharedPresentation()
{
    Collect_FrameLights();
    const PLAYING_ROW* scene = nullptr;
    const PLAYING_ROW* camera = nullptr;
    std::string sceneOwner;
    std::string cameraOwner;
    bool cameraPreview = false;
    std::set<KIND> conflictingGlobals;
    std::map<KIND, std::string> globalOwners;
    const auto inspectGlobals = [&](const SESSION& session)
    {
        if (!m_ProductBundleSession.runEpoch || session.runEpoch != m_ProductBundleSession.runEpoch) return;
        for (const auto& [id, row] : session.rows)
            if (!row.failed && (row.kind == KIND::CAMERA || row.kind == KIND::SCENE_PROFILE))
            {
                const auto [owner, inserted] = globalOwners.emplace(row.kind, session.key);
                if (!inserted && owner->second != session.key) conflictingGlobals.insert(row.kind);
            }
    };
    inspectGlobals(m_ProductBundleSession);
    for (const auto& [id, session] : m_BossSessions) inspectGlobals(session);
    for (const auto& [id, session] : m_ChildBossSessions) inspectGlobals(session);
    if (!conflictingGlobals.empty())
        m_strStatus = "Bundle global presentation owner conflict; overlapping Camera/Scene rows isolated.";
    const auto choose = [&](const SESSION& session, bool product)
    {
        const PLAYING_ROW* localScene = nullptr;
        const PLAYING_ROW* localCamera = nullptr;
        std::string localOwner;
        std::string localCameraOwner;
        for (const auto& [id, row] : session.rows)
        {
            if (row.failed || (product && conflictingGlobals.contains(row.kind))) continue;
            if (row.kind == KIND::SCENE_PROFILE && (!localScene || row.startMs >= localScene->startMs))
            {
                localScene = &row;
                localOwner = session.key + ":" + id;
            }
            if (row.kind == KIND::CAMERA && (!localCamera || row.startMs >= localCamera->startMs))
            {
                localCamera = &row;
                localCameraOwner = session.key + ":" + id + ":" +
                    std::to_string(product ? session.runEpoch : m_iPreviewGeneration);
            }
        }
        if (localScene) { scene = localScene; sceneOwner = std::move(localOwner); }
        if (localCamera) { camera = localCamera; cameraOwner = std::move(localCameraOwner); cameraPreview = !product; }
    };
    // A new Pattern takes precedence over an older Camera/Scene tail.
    for (const auto& tail : m_ProductTails) choose(tail.playback, true);
    // Stable entity/id order resolves overlapping presentation; preview owns the final choice.
    for (const auto& [id, session] : m_BossSessions) choose(session, true);
    for (const auto& [id, session] : m_ChildBossSessions) choose(session, true);
    choose(m_ProductBundleSession, true);
    for (const auto& member : m_BundlePreviewMembers) choose(member.session, false);
    if (m_bPreviewPlaying) choose(m_PreviewSession, false);
    // A local player's temporary fear owns the scene until its replicated end.
    for (const auto& [id, stage] : m_ExternalStageEnvironments) choose(stage.session, !stage.preview);
    choose(m_FearSession, false);
    if (!scene) Restore_Scene();
    else if (m_strSceneOwner != sceneOwner || m_strSceneApplied != scene->assetId)
    {
        const std::string previous = m_Profiles.Get_ActiveProfileId();
        std::string status;
        if (m_Profiles.Activate_Profile(scene->assetId, status))
        {
            if (!m_bSceneUsed) m_strScenePrevious = previous;
            m_bSceneUsed = true;
            m_strSceneOwner = std::move(sceneOwner);
            m_strSceneApplied = scene->assetId;
        }
        else m_strStatus = status;
    }
    if (auto* level = CLevel_KakulSaydonArena::Get_Active())
    {
        const bool cameraEnabled = level->Is_CompositionCameraEnabled();
        if (camera && cameraEnabled)
        {
            if (level->Sample_CompositionCamera(camera->assetId, camera->lastAge, camera->cameraOffset,
                cameraOwner, camera->cameraDurationMs, cameraPreview))
                m_bCameraUsed = true;
            else
            {
                m_strStatus = "Camera shot unavailable or interrupted: " + camera->assetId;
                if (m_bCameraUsed) level->Stop_CompositionCamera();
                m_bCameraUsed = false;
            }
        }
        else if (m_bCameraUsed)
        {
            level->Stop_CompositionCamera(!cameraEnabled);
            m_bCameraUsed = false;
        }
    }
    else m_bCameraUsed = false;
}

void Client::CKoukuSaydonPresentationPlayer::Update_FearPresentation(float dt,
    const std::vector<KOUKU_CARD_PRESENTATION_VIEW>& players)
{
    const auto stopFear = [this]() { Stop_Session(m_FearSession); m_FearSession.key.clear(); };
    const auto localId = CNetworkManager::Get().Get_LocalEntityId();
    const auto player = std::find_if(players.begin(), players.end(), [&](const auto& view)
        { return view.Snapshot.iNetEntityId == localId; });
    const auto* arena = CLevel_KakulSaydonArena::Get_Active();
    if (!arena || !localId || player == players.end() || !player->Snapshot.iCurrentHp ||
        player->Snapshot.eAction != LostArk::Shared::PLAYER_ACTION_STATE::FEAR)
    { stopFear(); m_strCompletedFearKey.clear(); return; }
    const auto& snapshot = player->Snapshot;
    const auto key = "fear:" + snapshot.strFearPresentationId + ":" + std::to_string(snapshot.iActionStartTick);
    // A locally completed clock must not restart from the last, slightly older
    // Server snapshot while the expiry snapshot is still in flight.
    if (m_strCompletedFearKey == key) { stopFear(); return; }
    const auto rejectFear = [&](const std::string& reason)
    { stopFear(); m_strCompletedFearKey = key; m_strStatus = reason; };
    const auto presentation = m_FearPresentations.find(snapshot.strFearPresentationId);
    if (presentation == m_FearPresentations.end())
    { rejectFear("Fear presentation unavailable: " + snapshot.strFearPresentationId); return; }
    float seconds = 0.f;
    if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(arena->Get_PresentationServerTick(),
        snapshot.iActionStartTick, 30.f, seconds))
    { stopFear(); return; }
    if (m_FearSession.key != key) { stopFear(); m_FearSession.key = key; }
    const float clockMs = m_FearSession.lastClockMs < 0.f ? seconds * 1000.f :
        (std::max)(seconds * 1000.f, m_FearSession.lastClockMs + dt * 1000.f);
    const auto durationTicks = snapshot.iFearEndTick - snapshot.iActionStartTick;
    const float durationMs = (std::min)(float(presentation->second.durationMs), durationTicks * (1000.f / 30.f));
    if (clockMs >= durationMs) { stopFear(); m_strCompletedFearKey = key; return; }
    // Do not darken the scene until its declared character lights are usable.
    // Gameplay FEAR remains Server-owned even if this presentation is isolated.
    for (const auto& resource : presentation->second.document.PresentationResources)
    {
        if (resource.eKind == KIND::SCENE_PROFILE && !m_Profiles.Has_Profile(resource.strAssetId))
        { rejectFear("Fear scene profile unavailable: " + resource.strAssetId); return; }
        if (resource.eKind != KIND::LIGHT) continue;
        const auto* light = m_pLightResources ? m_pLightResources->Find_RuntimeResource(resource.strAssetId) : nullptr;
        if (!light) { rejectFear("Fear character light unavailable: " + resource.strAssetId); return; }
        if (m_LightPlayerPivots.empty()) { stopFear(); return; }
        for (const auto& pivot : m_LightPlayerPivots)
        {
            LIGHT_DESC desc{}; std::string reason;
            if (!CLightResourceCatalog::Try_BuildLightDesc(*light, pivot, 1.f, desc, reason))
            { rejectFear("Fear character light rejected: " + resource.strAssetId + "; " + reason); return; }
        }
    }
    float4x4_t pivot; XMStoreFloat4x4(&pivot, XMMatrixIdentity());
    Sample(m_FearSession, presentation->second.document, presentation->second.pattern,
        clockMs, false, pivot, nullptr);
}

void Client::CKoukuSaydonPresentationPlayer::Update(float dt,
    const std::vector<KOUKU_BOSS_PRESENTATION_VIEW>& bosses,
    const std::vector<KOUKU_CARD_PRESENTATION_VIEW>& players)
{
    m_LogicPreviewPlayers = players;
    if (!std::isfinite(dt) || dt < 0.f) return;
    Sync_PreviewSourceVisibility();
    m_LightPlayerPivots.clear();
    m_LightBossFollowers.clear();
    for (const auto& view : bosses)
    {
        if (!view.iOwnerBossNetEntityId || !view.Snapshot.iCurrentHp || view.pNpc.expired() || view.strArchetypeId.empty()) continue;
        const auto owner = std::find_if(bosses.begin(), bosses.end(), [&](const auto& candidate)
        {
            return candidate.Snapshot.iNetEntityId == view.iOwnerBossNetEntityId &&
                !candidate.iOwnerBossNetEntityId && candidate.Snapshot.iCurrentHp && !candidate.pNpc.expired() &&
                candidate.strArchetypeId == view.strArchetypeId;
        });
        if (owner != bosses.end()) m_LightBossFollowers[view.iOwnerBossNetEntityId].push_back(
            {view.Snapshot.iNetEntityId, view.pNpc});
    }
    for (const auto& view : players)
    {
        const auto character = view.pCharacter.lock();
        if (character && character->Get_Transform())
            m_LightPlayerPivots.push_back(*character->Get_Transform()->Get_WorldMatrixPtr());
    }
    if (!m_bProductAttempted) { std::string status; (void)Reload_Product(status); }
    for (auto owner = m_CounterAfterimageOwners.begin(); owner != m_CounterAfterimageOwners.end();)
    {
        const auto npc = owner->second.lock();
        const bool live = npc && std::any_of(bosses.begin(), bosses.end(), [&](const auto& view) {
            return view.Snapshot.iNetEntityId == owner->first && view.pNpc.lock() == npc; });
        if (live) { ++owner; continue; }
        if (npc) npc->Set_CounterAfterimageEnabled(false);
        owner = m_CounterAfterimageOwners.erase(owner);
    }
    for (const auto& view : bosses)
        if (const auto npc = view.pNpc.lock())
        {
            npc->Set_ChargeAfterimageEnabled(false);
            using FLAG = LostArk::Shared::BOSS_COMBAT_STATE_FLAG;
            const auto flags = view.Snapshot.BossCombat.iFlags;
            npc->Set_CounterAfterimageEnabled(view.Snapshot.iCurrentHp && view.Snapshot.hasBossCombatState &&
                LostArk::Shared::Has_BossCombatFlag(flags, FLAG::COUNTERABLE) &&
                !LostArk::Shared::Has_BossCombatFlag(flags, FLAG::GROGGY) &&
                !LostArk::Shared::Has_BossCombatFlag(flags, FLAG::GHOST_HIDDEN));
            m_CounterAfterimageOwners[view.Snapshot.iNetEntityId] = npc;
        }
    Update_TargetedCombatVisuals(dt, bosses);
    Update_ContactCombatEffects(dt);
    const auto* arena = CLevel_KakulSaydonArena::Get_Active();
    const auto* run = arena ? &arena->Get_KoukuBundleState() : nullptr;
    using RUN_STATE = LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE;
    const bool runLive = run && run->iRunEpoch &&
        (run->eState == RUN_STATE::PENDING || run->eState == RUN_STATE::ACTIVE || run->eState == RUN_STATE::PATTERN_COMPLETED);
    const bool runPresentationLive = run && run->iRunEpoch &&
        (runLive || run->eState == RUN_STATE::COMPLETED);
    if (runLive && m_iProductReloadRunEpoch != run->iRunEpoch)
    {
        if (auto* level = CLevel_KakulSaydonArena::Get_Active())
        { std::string cameraStatus; if (!level->Reload_PublishedCameraShots(cameraStatus)) m_strStatus = cameraStatus; }
        if (!CKoukuSaydonPresentationAssetService::Matches_AdmittedRun(run->iPinnedSourceRevision, run->DraftRowsRevision, run->iRunEpoch))
        { m_strStatus = "Presentation draft hash/epoch is unavailable for the admitted run."; return; }
        if (run->DraftRowsRevision.Is_Valid() || run->iPinnedSourceRevision != m_iProductSourceRevision || run->DraftRowsRevision != m_ProductDraftRowsRevision)
        { std::string status; if (!Reload_Product(status, run->iPinnedSourceRevision)) return; }
        m_iProductReloadRunEpoch = run->iRunEpoch;
    }
    if (runPresentationLive && (!CKoukuSaydonPresentationAssetService::Matches_AdmittedRun(
        run->iPinnedSourceRevision, run->DraftRowsRevision, run->iRunEpoch) || run->DraftRowsRevision != m_ProductDraftRowsRevision)) return;
    if (runPresentationLive && !run->strBundleId.empty())
    {
        const auto product = m_ProductBundles.find(run->strBundleId);
        float seconds = 0.f;
        if (product == m_ProductBundles.end() || run->iPinnedSourceRevision != m_iProductSourceRevision)
        {
            Stop_Session(m_ProductBundleSession);
            m_strStatus = "Replicated bundle Product/source revision is unavailable: " + run->strBundleId;
        }
        else if (CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
            (std::max)(run->iServerTick, arena->Get_PresentationServerTick()), run->iCommonStartTick, 30.f, seconds))
        {
            const auto key = "bundle-product:" + std::to_string(run->iRunEpoch) + ":" + run->strBundleId +
                ":" + std::to_string(run->iCommonStartTick);
            if (m_ProductBundleSession.key != key)
            {
                Retire_ProductBundleSession();
                m_ProductBundleSession.key = key;
                m_ProductBundleSession.runEpoch = run->iRunEpoch;
                m_ProductBundleSession.productPatternId = run->strBundleId;
                m_ProductBundleSession.patternStartTick = run->iCommonStartTick;
            }
            const float clock = m_ProductBundleSession.lastClockMs < 0.f ? seconds * 1000.f :
                (std::max)(seconds * 1000.f, m_ProductBundleSession.lastClockMs + dt * 1000.f);
            float4x4_t pivot; XMStoreFloat4x4(&pivot, XMMatrixIdentity());
            if (clock < product->second.common.durationMs)
                Sample(m_ProductBundleSession, product->second.common.document, product->second.common.pattern,
                    clock, false, pivot, nullptr);
            else
            {
                Stop_Session(m_ProductBundleSession);
                // Retain the terminal clock for this key. The interpolated
                // clock can finish before the next authoritative tick arrives.
                m_ProductBundleSession.lastClockMs = clock;
            }
        }
        else Stop_Session(m_ProductBundleSession);
    }
    else Retire_ProductBundleSession();
    std::set<std::uint32_t> liveMarioEntries;
    if (runLive && run->iPinnedSourceRevision == m_iProductSourceRevision)
        for (const auto& member : run->Members)
        {
            if (member.strMarioEntryPatternId.empty()) continue;
            const auto root = m_Product.find(member.strMarioEntryPatternId);
            if (root == m_Product.end()) continue;
            float seconds = 0.f;
            if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
                (std::max)(run->iServerTick, arena->Get_PresentationServerTick()), member.iMarioEntryStartTick, 30.f, seconds)) continue;
            liveMarioEntries.insert(member.iBossNetEntityId);
            auto& session = m_MarioEntrySessions[member.iBossNetEntityId];
            const auto key = std::to_string(run->iRunEpoch) + ":" + member.strMemberId + ":" + std::to_string(member.iMarioEntryStartTick);
            if (session.key != key) { Stop_Session(session); session.key = key; }
            session.runEpoch = run->iRunEpoch; session.memberId = member.strMemberId;
            const auto pivot = XMMatrixRotationY(XMConvertToRadians(member.fMarioEntryYawDegrees)) *
                XMMatrixTranslation(member.fMarioEntryX, member.fMarioEntryY, member.fMarioEntryZ);
            float4x4_t storedPivot; XMStoreFloat4x4(&storedPivot, pivot);
            // Keep the authored entry frame while child animations run. Its
            // lifetime and terminal stop come only from persistent Server state.
            const float clock = (std::min)(seconds * 1000.f, float(member.iMarioEntryHoldMs));
            Sample(session, root->second.document, root->second.pattern, clock,
                seconds * 1000.f >= member.iMarioEntryHoldMs, storedPivot, nullptr);
        }
    for (auto it = m_MarioEntrySessions.begin(); it != m_MarioEntrySessions.end();)
        if (liveMarioEntries.contains(it->first)) ++it;
        else { Stop_Session(it->second); it = m_MarioEntrySessions.erase(it); }
    std::set<std::uint32_t> liveBosses;
    for (const auto& view : bosses)
    {
        if (run && run->iRunEpoch && run->eState == RUN_STATE::ABORTED) continue;
        if (runLive && run->iPinnedSourceRevision != m_iProductSourceRevision) continue;
        const auto npc = view.pNpc.lock();
        const auto product = m_Product.find(view.Snapshot.strPatternId);
        if (m_bProductLoaded && !view.Snapshot.strPatternId.empty() && product == m_Product.end() &&
            m_MissingProductPatterns.insert(view.Snapshot.strPatternId).second)
        {
            m_strStatus = "Snapshot pattern has no Product presentation: " + view.Snapshot.strPatternId;
            OutputDebugStringA(("[KoukuSaydonPresentationPlayer] " + m_strStatus + "\n").c_str());
        }
        if (!npc || !npc->Get_Transform() || product == m_Product.end() || !view.Snapshot.iCurrentHp) continue;
        float seconds = 0.f;
        if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(view.iServerTick,
            view.Snapshot.iPatternStartTick, 30.f, seconds)) continue;
        const auto id = view.Snapshot.iNetEntityId;
        if (liveMarioEntries.contains(id) && std::any_of(run->Members.begin(), run->Members.end(), [&](const auto& member) {
            return member.iBossNetEntityId == id && member.strMarioEntryPatternId == view.Snapshot.strPatternId;
        })) continue;
        const LostArk::Shared::KOUKUSAYDON_BUNDLE_MEMBER_STATE* owner = nullptr;
        if (run && run->iRunEpoch)
            for (const auto& member : run->Members)
                if (member.iBossNetEntityId == id) { owner = &member; break; }
        // Run receipts can precede the boss snapshot. Do not adopt the old
        // Pattern's handles into a new member/epoch while waiting for that snapshot.
        float bornInRunSeconds = 0.f;
        if (owner && (owner->iPatternSequence != view.Snapshot.iPatternSequence ||
            owner->eState == RUN_STATE::PENDING || owner->strPatternId != view.Snapshot.strPatternId ||
            !CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
                view.Snapshot.iPatternStartTick, run->iCommonStartTick, 30.f, bornInRunSeconds)))
            continue;
        liveBosses.insert(id);
        SESSION& session = m_BossSessions[id];
        const std::string key = std::to_string(id) + ":" + view.Snapshot.strPatternId + ":" +
            std::to_string(view.Snapshot.iPatternSequence) + ":" + std::to_string(view.Snapshot.iPatternStartTick);
        if (session.runEpoch && (!run || session.runEpoch != run->iRunEpoch))
            Retire_ProductSession(session, bosses);
        if (const auto cancelled = m_CancelledBossSessionKeys.find(id);
            cancelled != m_CancelledBossSessionKeys.end() && cancelled->second == key) continue;
        if (session.key != key)
        {
            Retire_ProductSession(session, bosses);
            session.key = key;
            session.productPatternId = view.Snapshot.strPatternId;
            session.bossEntityId = id; session.patternSequence = view.Snapshot.iPatternSequence;
            session.patternStartTick = view.Snapshot.iPatternStartTick;
            if (owner) { session.runEpoch = run->iRunEpoch; session.memberId = owner->strMemberId; }
        }
        // Interpolate between snapshots without rewinding/restarting effects every network tick.
        const float clock = session.lastClockMs < 0.f ? seconds * 1000.f :
            (std::max)(seconds * 1000.f, session.lastClockMs + dt * 1000.f);
        ANIMATION_MODEL_TARGET_VIEW weaponView;
        const bool hasWeapon = npc->Try_GetAnimationModelTarget(ANIMATION_BONE_TARGET::WEAPON, weaponView);
        bool backstepAfterimage = false;
        const bool chargeAfterimage = Charge_AfterimageActive(product->second.pattern, clock, backstepAfterimage);
        npc->Set_ChargeAfterimageEnabled(chargeAfterimage, backstepAfterimage);
        Sample(session, product->second.document, product->second.pattern,
            (std::min)(clock, float(product->second.durationMs)), false,
            *npc->Get_Transform()->Get_WorldMatrixPtr(), npc->Get_Model(), hasWeapon ? &weaponView : nullptr);
    }
    for (auto session = m_BossSessions.begin(); session != m_BossSessions.end();)
    {
        if (liveBosses.contains(session->first)) { ++session; continue; }
        Retire_ProductSession(session->second, bosses);
        session = m_BossSessions.erase(session);
    }
    // Child effects use their own Server clock and the same live body. Keeping
    // a separate session preserves every common row on the parent timeline.
    std::set<std::uint32_t> liveChildren;
    for (const auto& view : bosses)
    {
        if (run && run->iRunEpoch && run->eState == RUN_STATE::ABORTED) continue;
        if (runLive && run->iPinnedSourceRevision != m_iProductSourceRevision) continue;
        if (view.Snapshot.strPresentationPatternId.empty() || !view.Snapshot.iCurrentHp) continue;
        const auto npc = view.pNpc.lock();
        const auto child = m_Product.find(view.Snapshot.strPresentationPatternId);
        if (m_bProductLoaded && child == m_Product.end() &&
            m_MissingProductPatterns.insert(view.Snapshot.strPresentationPatternId).second)
        {
            m_strStatus = "Snapshot child pattern has no Product presentation: " + view.Snapshot.strPresentationPatternId;
            OutputDebugStringA(("[KoukuSaydonPresentationPlayer] " + m_strStatus + "\n").c_str());
        }
        if (!npc || !npc->Get_Transform() || child == m_Product.end()) continue;
        float seconds = 0.f;
        if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(view.iServerTick,
            view.Snapshot.iPresentationPatternStartTick, 30.f, seconds)) continue;
        const auto id = view.Snapshot.iNetEntityId;
        const LostArk::Shared::KOUKUSAYDON_BUNDLE_MEMBER_STATE* owner = nullptr;
        if (run && run->iRunEpoch)
            for (const auto& member : run->Members)
                if (member.iBossNetEntityId == id) { owner = &member; break; }
        // Run receipts can precede the boss snapshot. Do not adopt the old
        // Pattern's handles into a new member/epoch while waiting for that snapshot.
        float bornInRunSeconds = 0.f;
        if (owner && (owner->iPatternSequence != view.Snapshot.iPatternSequence ||
            owner->eState == RUN_STATE::PENDING || owner->strPatternId != view.Snapshot.strPatternId ||
            !CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
                view.Snapshot.iPatternStartTick, run->iCommonStartTick, 30.f, bornInRunSeconds)))
            continue;
        liveChildren.insert(id);
        SESSION& session = m_ChildBossSessions[id];
        const auto key = std::to_string(id) + ":child:" + view.Snapshot.strPresentationPatternId + ":" +
            std::to_string(view.Snapshot.iPatternSequence) + ":" + std::to_string(view.Snapshot.iPresentationPatternStartTick);
        if (session.runEpoch && (!run || session.runEpoch != run->iRunEpoch))
            Retire_ProductSession(session, bosses);
        if (const auto cancelled = m_CancelledChildSessionKeys.find(id);
            cancelled != m_CancelledChildSessionKeys.end() && cancelled->second == key) continue;
        if (session.key != key)
        {
            Retire_ProductSession(session, bosses);
            session.key = key;
            session.productPatternId = view.Snapshot.strPresentationPatternId;
            session.bossEntityId = id; session.patternSequence = view.Snapshot.iPatternSequence;
            session.patternStartTick = view.Snapshot.iPresentationPatternStartTick;
            if (owner) { session.runEpoch = run->iRunEpoch; session.memberId = owner->strMemberId; }
        }
        const float clock = session.lastClockMs < 0.f ? seconds * 1000.f :
            (std::max)(seconds * 1000.f, session.lastClockMs + dt * 1000.f);
        ANIMATION_MODEL_TARGET_VIEW weaponView;
        const bool hasWeapon = npc->Try_GetAnimationModelTarget(ANIMATION_BONE_TARGET::WEAPON, weaponView);
        bool backstepAfterimage = false;
        if (Charge_AfterimageActive(child->second.pattern, clock, backstepAfterimage))
            npc->Set_ChargeAfterimageEnabled(true, backstepAfterimage);
        Sample(session, child->second.document, child->second.pattern,
            (std::min)(clock, float(child->second.durationMs)), false,
            *npc->Get_Transform()->Get_WorldMatrixPtr(), npc->Get_Model(), hasWeapon ? &weaponView : nullptr);
    }
    for (auto session = m_ChildBossSessions.begin(); session != m_ChildBossSessions.end();)
    {
        if (liveChildren.contains(session->first)) { ++session; continue; }
        Retire_ProductSession(session->second, bosses);
        session = m_ChildBossSessions.erase(session);
    }
    Update_ProductTails(dt, bosses);
    const auto ownerGone = [&](const auto& item) {
        return std::none_of(bosses.begin(), bosses.end(), [&](const auto& view) {
            return view.Snapshot.iNetEntityId == item.first; });
    };
    std::erase_if(m_CancelledBossSessionKeys, ownerGone);
    std::erase_if(m_CancelledChildSessionKeys, ownerGone);
    std::set<std::uint32_t> liveCards;
    for (const auto& view : players)
    {
        const auto character = view.pCharacter.lock();
        const std::string asset = Card_Asset(view.Snapshot);
        if (!character || !character->Get_Transform() || asset.empty()) continue;
        const auto id = view.Snapshot.iNetEntityId;
        liveCards.insert(id);
        CARD& card = m_Cards[id];
        if (card.assetId != asset)
        {
            if (card.handle) CEffectV2Runtime::Stop_Group(card.handle);
            card = {};
            card.assetId = asset;
            std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> effects;
            if (Ensure_EffectResource("GROUP", asset, effects))
            {
                if (const auto* source = effects->Find_Group(asset))
                {
                    EFFECT_V2_GROUP group = *source;
                    group.iDurationMs = 0u;
                    for (auto& child : group.Children)
                    {
                        child.vOffset.z = 0.f;
                        child.LocalTransform.vTranslation.z = 0.f;
                    }
                    EFFECT_V2_GROUP_PLAYBACK_DESC playback;
                    XMStoreFloat4x4(&playback.PivotWorld, XMMatrixIdentity());
                    playback.fDurationSeconds = 0.f;
                    playback.bProductOwned = true;
                    card.handle = CEffectV2Runtime::Play_Group(group, effects,
                        playback, m_Device, m_Context);
                }
                if (!card.handle) m_strStatus = "Assigned card effect unavailable: " + asset;
            }
        }
        if (card.handle)
        {
            const matrix_t world = XMLoadFloat4x4(character->Get_Transform()->Get_WorldMatrixPtr());
            vector_t position = world.r[3];
            const auto body = character->Get_BodyModel();
            if (body && body->Has_Bone("bip001-head"))
            {
                position = (body->Get_BoneMatrix("bip001-head") * world).r[3];
                position = XMVectorSetY(position, XMVectorGetY(position) + 0.65f - 2.f);
            }
            // Both card leaves already contribute +2m Y. Keep their billboard vertical.
            float4x4_t pivot;
            XMStoreFloat4x4(&pivot, XMMatrixTranslationFromVector(position));
            CEffectV2Runtime::Set_GroupPivot(card.handle, pivot);
            std::string failure;
            if (CEffectV2Runtime::Consume_GroupFailure(card.handle, failure))
            {
                CEffectV2Runtime::Stop_Group(card.handle);
                card.handle = 0;
                m_strStatus = std::move(failure);
            }
        }
    }
    for (auto card = m_Cards.begin(); card != m_Cards.end();)
    {
        if (liveCards.contains(card->first)) { ++card; continue; }
        if (card->second.handle) CEffectV2Runtime::Stop_Group(card->second.handle);
        card = m_Cards.erase(card);
    }
    Update_DiceBindVisuals(dt, players);
    Update_MazeMarks(players);
    Update_BingoMarks(dt);
    Update_FearPresentation(dt, players);
    if (m_bPreviewPlaying && m_bOwnPreviewClock && m_bPreviewPivotReady)
    {
        // Begin runs after this Update; the next delta includes synchronous WORLD
        // preparation. Arm the new clock once without charging that setup time.
        if (!Prepare_PreviewEffects())
        { m_bPreviewClockAwaitingFirstUpdate = true; Refresh_SharedPresentation(); return; }
        const bool advanceClock = !m_bPreviewClockAwaitingFirstUpdate;
        m_bPreviewClockAwaitingFirstUpdate = false;
        if (advanceClock && !m_bServerSequenceClock && !m_bPreviewPaused && !m_bModelReferencePreview && !m_bPreviewCaptureClockHeld)
            m_fPreviewClockMs += double(dt) * 1000.0;
        if (!m_bServerSequenceClock && !m_bPreviewPaused && !m_bModelReferencePreview && m_fPreviewClockMs >= m_iPreviewDurationMs)
        {
            if (m_bColliderResourcePreview)
            {
                m_fPreviewClockMs = m_iPreviewDurationMs - 1u;
                Pause_Preview(true);
            }
            else
            {
                // MainApp applies this frame's Pause/Seek/Stop before consuming completion.
                // Keep the real clock and borrowed actors alive until that decision.
                m_fPreviewClockMs = m_iPreviewDurationMs;
                m_strCompletedPreviewPatternId = Preview_IsBundle() ? m_PreviewBundleId : m_PreviewPattern.strPatternId;
            }
        }
        // Bundle members own their independent WORLD players, sampled before effects.
        if (Preview_IsBundle() && !m_bServerSequenceClock && m_strCompletedPreviewPatternId.empty()) Sample_BundlePreview();
        // MainApp samples single-pattern WORLD first, then its presentation.
    }
    Refresh_SharedPresentation();
}

bool Client::CKoukuSaydonPresentationPlayer::Begin_Preview(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, bool ownClock,
    std::uint32_t clockMs, bool paused, std::string& status)
{
    std::string soundStatus;
    CSoundCueCatalog::Load_ClassSnapshot("KoukuSaydon", m_SoundEventVariants, soundStatus);
    const auto duration = Pattern_Duration(pattern);
    if (!pattern.strLoadError.empty() || !duration)
    {
        status = "Presentation preview needs a valid finite pattern.";
        return false;
    }
    for (const auto& box : pattern.PresentationOccurrences)
    {
        const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
            [&box](const auto& value) { return value.strResourceId == box.strResourceId; });
        if (resource == document.PresentationResources.end())
        {
            status = "Preview names an unknown presentation resource: " + box.strResourceId;
            return false;
        }
        if (resource->eKind == KIND::EFFECT && !Validate_EffectAnchor(document, pattern, box, status)) return false;
    }
    if (pattern.strPatternId == "preview.kouku.resource" && pattern.WorldOccurrences.empty() &&
        pattern.PresentationOccurrences.size() == 1u)
    {
        const auto& box = pattern.PresentationOccurrences.front();
        const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
            [&](const auto& value) { return value.strResourceId == box.strResourceId; });
        if (resource->eKind == KIND::EFFECT &&
            (resource->strResourceKind == "V1_EFFECT" || resource->strResourceKind == "V1_ELEMENT"))
        {
            // A Resource has its own animation origin. Prepare its saved actor
            // through the existing single-member model/presentation owner.
            const auto effect = CEffectCatalog::Find(resource->strAssetId);
            if (!effect) { status = CEffectCatalog::Get_Status(); return false; }
            if (effect->SourceModelPreview)
            {
                CEffectCompositionModelPreview source;
                if (!source.Select_SourceEffect(*effect)) { status = source.Status(); return false; }
                auto stagedDocument = document;
                auto sourcePattern = source.Get_Document().Patterns.front();
                sourcePattern.strPatternId = pattern.strPatternId + ".actor";
                sourcePattern.PresentationOccurrences = pattern.PresentationOccurrences;
                // The Effect window owns lifetime; the native final pose is held
                // across particle tails by the same CModel source sampler.
                sourcePattern.iDurationMs = duration;
                sourcePattern.Stages.front().iDurationMs = duration;
                auto& animations = sourcePattern.Stages.front().AnimationOccurrences;
                std::erase_if(animations, [&](const auto& animation) { return animation.iStartOffsetMs >= duration; });
                for (auto& animation : animations)
                {
                    animation.iPoseStartMs = animation.iStartOffsetMs;
                    animation.iPlayMs = (std::min)(animation.iPlayMs, duration - animation.iStartOffsetMs);
                }
                const auto sourceId = sourcePattern.strPatternId;
                const auto gateId = sourcePattern.strGateId;
                stagedDocument.Patterns.push_back(std::move(sourcePattern));
                KOUKU_SAYDON_COMPOSITION_BUNDLE bundle;
                bundle.strBundleId = pattern.strPatternId; bundle.strGateId = gateId;
                bundle.Members.push_back({pattern.strPatternId + ".member", sourceId, 0u});
                stagedDocument.Bundles.push_back(std::move(bundle));
                if (!Begin_BundlePreview(stagedDocument, pattern.strPatternId, clockMs, paused, status)) return false;
                status = m_strStatus = "Effect Resource source model and animation ready: " + resource->strAssetId;
                return true;
            }
        }
        const bool bossLight = resource->eKind == KIND::LIGHT && box.strAnchorKind == "BOSS";
        if (bossLight && (pattern.strActorProfileId.empty() || pattern.strGateId.empty() || pattern.strTargetBossPlacementId.empty()))
        { status = "Boss Light Preview requires a selected Pattern with an exact Gate and boss target."; return false; }
        if ((resource->eKind == KIND::EFFECT || bossLight) && !pattern.strActorProfileId.empty() &&
            !pattern.strGateId.empty() && !pattern.strTargetBossPlacementId.empty())
        {
            // Reuse the selected boss actor owner for a Resource without source
            // animation metadata. Its existing idle pose needs no invented clip.
            auto stagedDocument = document;
            auto sourcePattern = pattern;
            sourcePattern.strPatternId = pattern.strPatternId + ".actor";
            sourcePattern.iDurationMs = duration;
            const auto sourceId = sourcePattern.strPatternId;
            stagedDocument.Patterns.push_back(std::move(sourcePattern));
            KOUKU_SAYDON_COMPOSITION_BUNDLE bundle;
            bundle.strBundleId = pattern.strPatternId; bundle.strGateId = pattern.strGateId;
            bundle.Members.push_back({pattern.strPatternId + ".member", sourceId, 0u});
            stagedDocument.Bundles.push_back(std::move(bundle));
            if (!Begin_BundlePreview(stagedDocument, pattern.strPatternId, clockMs, paused, status)) return false;
            status = m_strStatus = "Resource selected boss ready: " + resource->strAssetId +
                " | " + pattern.strGateId + " | " + pattern.strTargetBossPlacementId;
            return true;
        }
    }
    // Stage first. A rejected preview request preserves the active session.
    auto stagedDocument = document;
    auto stagedPattern = pattern;
    Stop_Preview();
    m_PreviewDocument = std::move(stagedDocument);
    m_PreviewPattern = std::move(stagedPattern);
    m_PreviewSession.key = "preview:" + pattern.strPatternId;
    m_iPreviewDurationMs = duration;
    m_fPreviewClockMs = (std::min)(clockMs, duration);
    m_bOwnPreviewClock = ownClock;
    m_bPreviewClockAwaitingFirstUpdate = ownClock;
    m_bPreviewPlaying = true;
    m_bPreviewPaused = paused;
    m_bColliderResourcePreview = pattern.strPatternId == "preview.kouku.resource" &&
        pattern.PresentationOccurrences.size() == 1u &&
        std::any_of(document.PresentationResources.begin(), document.PresentationResources.end(),
            [&pattern](const auto& resource) { return resource.eKind == KIND::COLLIDER &&
                resource.strResourceId == pattern.PresentationOccurrences.front().strResourceId; });
    m_EffectResources.clear();
    m_EffectResourceFailures.clear();
    status = "Presentation preview ready: " + pattern.strPatternId;
    m_strStatus = status;
    return true;
}

void Client::CKoukuSaydonPresentationPlayer::Release_BundlePreviewMembers(
    std::vector<BUNDLE_PREVIEW_MEMBER>& members)
{
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    for (auto& member : members)
    {
        if (const auto source = member.suppressedSourceActor.lock())
            source->Release_CompositionPreviewSuppression();
        member.suppressedSourceActor.reset();
        if (member.actor)
        {
            member.actor->Set_CounterAfterimageEnabled(false);
            member.actor->Set_ChargeAfterimageEnabled(false);
            member.actor->Reset_AfterimageHistory();
        }
        if (member.rootMotion)
        {
            member.rootMotion.reset();
            if (member.actor) (void)member.actor->Apply_NetworkState(member.initialPosition, member.initialYawDegrees);
        }
        else if (member.actor && member.actor->Get_Model())
            (void)member.actor->Get_Model()->Set_RootMotionVerticalScale(1.f);
        Stop_Session(member.session);
        if (level)
        {
            const auto targets = level->Get_CompositionWorldTargets();
            for (auto& [id, player] : member.session.previewWorlds) player->Stop_All(targets, true);
            level->Release_CompositionPreviewActor(member.actor);
        }
        else if (member.actor)
            CGameInstance::Get().Remove_GameObject_from_Layer(ETOUI(LEVEL::KAKULSAYDON_ARENA),
                L"Layer_KoukuCompositionPreview", member.actor);
    }
    members.clear();
}

bool Client::CKoukuSaydonPresentationPlayer::Prepare_CloneSplitPreview(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    std::vector<BUNDLE_PREVIEW_MEMBER>& members, std::string& status)
{
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    std::vector<BUNDLE_PREVIEW_MEMBER> clones;
    const auto fail = [&](const std::string& reason) {
        Release_BundlePreviewMembers(clones); status = reason; return false;
    };
    const auto flatten = [](const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern) {
        std::vector<KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE> rows;
        uint32_t start = 0u;
        for (const auto& stage : pattern.Stages)
        {
            for (auto row : stage.AnimationOccurrences)
            {
                const auto first = std::min_element(stage.AnimationOccurrences.begin(), stage.AnimationOccurrences.end(),
                    [](const auto& a, const auto& b) { return a.iStartOffsetMs < b.iStartOffsetMs; });
                row.iPoseStartMs = start + (row.strOccurrenceId == first->strOccurrenceId ? 0u : row.iStartOffsetMs);
                row.iStartOffsetMs += start;
                rows.push_back(std::move(row));
            }
            start += stage.iDurationMs;
        }
        std::stable_sort(rows.begin(), rows.end(), [](const auto& a, const auto& b) {
            return a.iStartOffsetMs != b.iStartOffsetMs ? a.iStartOffsetMs < b.iStartOffsetMs : a.strOccurrenceId < b.strOccurrenceId;
        });
        return rows;
    };
    const auto findPattern = [&](const std::string& id) -> const KOUKU_SAYDON_COMPOSITION_PATTERN* {
        const auto found = std::find_if(document.Patterns.begin(), document.Patterns.end(),
            [&](const auto& pattern) { return pattern.strPatternId == id; });
        return found == document.Patterns.end() ? nullptr : &*found;
    };
    const auto prepareClone = [&](const KOUKU_SAYDON_COMPOSITION_PATTERN& source, const std::string& id,
        uint32_t startTicks, uint32_t duration, const float3_t& position, float yaw) {
        clones.emplace_back();
        auto& clone = clones.back();
        clone.memberId = id; clone.offsetTicks = startTicks; clone.durationMs = duration;
        clone.pattern = source; clone.facingStages = source.Stages; clone.finiteActorLifetime = true;
        clone.session.key = "summon-preview:" + id;
        clone.spatialLogicPreview = std::any_of(source.LogicOccurrences.begin(), source.LogicOccurrences.end(),
            [](const auto& box) { return box.bEnabled; });
        clone.animations = flatten(source);
        if (!level || !duration || clone.animations.empty() ||
            !level->Create_CompositionPreviewActor(source, clone.actor, status)) return false;
        clone.actor->Set_PresentationVisible(false);
        if (!clone.actor->Apply_NetworkState(position, yaw)) return false;
        clone.initialPosition = position; clone.initialYawDegrees = yaw;
        clone.arenaCenterPosition = position;
        const auto model = clone.actor->Get_Model();
        clone.initialAnimation = model->Get_CurrentAnimIndex();
        float ignored = 0.f;
        model->Get_AnimationProgress(clone.initialAnimation, clone.initialTicks, ignored);
        clone.rootMotion = std::make_unique<CKoukuSaydonPreviewRootMotion>();
        return clone.rootMotion->Prepare(model, clone.animations, float(source.fAnimationRootVerticalScale), status, float(source.fAnimationRootHorizontalScale)) &&
            clone.rootMotion->Prepare_Airborne(document, source, status) && clone.rootMotion->Begin_Suppression();
    };
    for (auto& member : members)
    {
        const auto authoredPattern = member.pattern;
        std::vector<KOUKU_SAYDON_COMPOSITION_CROSS_DIRECTION_WINDOW> splitBoxes;
        if (!CKoukuSaydonCompositionDocument::Try_ResolveCrossDirectionWindows(
            document, authoredPattern, splitBoxes, status)) return fail(status);
        for (const auto& box : splitBoxes)
        {
            if (box.DirectionPatternIds.size() != 4u || !box.iDurationMs)
                return fail("Cross-direction preview needs four Patterns and a finite duration.");
            CWorldGameplayDocument world;
            if (!world.Load(CProjectDataRoot::Resolve(std::filesystem::path("Worlds") / document.strAreaId / "Gameplay.world.json"),
                document.strAreaId, status)) return fail(status);
            const auto* spawn = world.Find(member.pattern.strTargetBossPlacementId);
            if (!spawn || !Sample_BundlePreviewFacing(member, box.iStartMs))
                return fail("Cross-direction preview cannot resolve its boss spawn or starting transform.");
            const auto root = *member.actor->Get_Transform()->Get_WorldMatrixPtr();
            const float3_t position{root._41, root._42, root._43};
            const float yaw = XMConvertToDegrees(std::atan2(root._31, root._33));
            std::array<const KOUKU_SAYDON_COMPOSITION_PATTERN*, 4u> candidates{};
            std::array<uint32_t, 4u> cutoffs{};
            size_t selected = 0u;
            double bestDistance = (std::numeric_limits<double>::max)();
            for (size_t i = 0u; i < candidates.size(); ++i)
            {
                const auto* candidate = findPattern(box.DirectionPatternIds[i]);
                if (!candidate || candidate->strTargetBossPlacementId != member.pattern.strTargetBossPlacementId)
                    return fail("Cross-direction preview names an unavailable boss Pattern.");
                bool cutoffFound = false;
                for (const auto& stage : candidate->Stages)
                {
                    cutoffs[i] += stage.iDurationMs;
                    if (stage.strStageId == box.strCloneEndStageId) { cutoffFound = true; break; }
                }
                if (!cutoffFound || Pattern_Duration(*candidate) > box.iDurationMs)
                    return fail("Cross-direction preview has an invalid cutoff or child duration.");
                const auto rows = flatten(*candidate);
                CKoukuSaydonPreviewRootMotion motion;
                float3_t endpoint;
                const std::vector<float> yaws(rows.size(), yaw);
                if (!motion.Prepare(member.actor->Get_Model(), rows, float(candidate->fAnimationRootVerticalScale), status, float(candidate->fAnimationRootHorizontalScale)) ||
                    !motion.Sample_Displacement(cutoffs[i], yaws, endpoint)) return fail(status);
                const double dx = position.x + endpoint.x - spawn->position.x;
                const double dz = position.z + endpoint.z - spawn->position.z;
                const double distance = dx * dx + dz * dz;
                if (distance < bestDistance - 0.00001) { selected = i; bestDistance = distance; }
                candidates[i] = candidate;
            }
            const uint32_t startTicks = (uint64_t(box.iStartMs) * 30u + 999u) / 1000u;
            for (size_t i = 0u; i < candidates.size(); ++i)
            {
                if (i == selected) continue;
                auto fake = *candidates[i];
                fake.PresentationOccurrences.clear();
                if (!prepareClone(fake, member.memberId + ":" + box.strOccurrenceId + ":" + std::to_string(i),
                    member.offsetTicks + startTicks, cutoffs[i], position, yaw)) return fail(status);
            }
            const auto prefix = box.strOccurrenceId + ".selected.";
            for (auto row : flatten(*candidates[selected]))
            {
                row.strOccurrenceId = prefix + row.strOccurrenceId;
                row.iStartOffsetMs += box.iStartMs; row.iPoseStartMs += box.iStartMs;
                member.cloneAnimationWindows.emplace(row.strOccurrenceId,
                    std::pair{box.iStartMs, box.iStartMs + box.iDurationMs});
                member.animations.push_back(std::move(row));
            }
            for (auto effect : candidates[selected]->PresentationOccurrences)
            {
                effect.strOccurrenceId = prefix + effect.strOccurrenceId;
                effect.iStartMs += box.iStartMs;
                if (!effect.strSelectionGroupId.empty()) effect.strSelectionGroupId = prefix + effect.strSelectionGroupId;
                if (!effect.strLogicOccurrenceId.empty()) effect.strLogicOccurrenceId = prefix + effect.strLogicOccurrenceId;
                member.pattern.PresentationOccurrences.push_back(std::move(effect));
            }
            std::stable_sort(member.animations.begin(), member.animations.end(), [](const auto& a, const auto& b) {
                return a.iStartOffsetMs != b.iStartOffsetMs ? a.iStartOffsetMs < b.iStartOffsetMs : a.strOccurrenceId < b.strOccurrenceId;
            });
            KOUKU_SAYDON_COMPOSITION_STAGE previewStage;
            previewStage.strStageId = "preview.clone-split";
            previewStage.iDurationMs = member.durationMs;
            previewStage.AnimationOccurrences = member.animations;
            member.pattern.Stages = {std::move(previewStage)};
            member.rootMotion.reset();
            member.rootMotion = std::make_unique<CKoukuSaydonPreviewRootMotion>();
            if (!member.rootMotion->Prepare(member.actor->Get_Model(), member.animations,
                float(member.pattern.fAnimationRootVerticalScale), status, float(member.pattern.fAnimationRootHorizontalScale)) ||
                !member.rootMotion->Begin_Suppression()) return fail(status);
        }
        for (const auto& summon : authoredPattern.SummonOccurrences)
        {
            if (summon.PatternSpawns.empty()) continue;
            if (!Sample_BundlePreviewFacing(member, summon.iStartMs)) return fail("Summon preview cannot sample its owner.");
            const auto root = *member.actor->Get_Transform()->Get_WorldMatrixPtr();
            const float yaw = XMConvertToDegrees(std::atan2(root._31, root._33));
            const auto rotation = XMMatrixRotationY(XMConvertToRadians(yaw));
            for (const auto& spawn : summon.PatternSpawns)
            {
                const auto* candidate = findPattern(spawn.strPatternId);
                if (!candidate) return fail("Summon preview child Pattern is unavailable: " + spawn.strPatternId);
                float3_t offset;
                XMStoreFloat3(&offset, XMVector3TransformNormal(XMVectorSet(float(spawn.PositionOffset[0]),
                    float(spawn.PositionOffset[1]), float(spawn.PositionOffset[2]), 0.f), rotation));
                const bool fixed = spawn.strAnchorKind == "MAP";
                const float3_t position = fixed ? float3_t{float(spawn.PositionOffset[0]), float(spawn.PositionOffset[1]), float(spawn.PositionOffset[2])} :
                    float3_t{root._41 + offset.x, root._42 + offset.y, root._43 + offset.z};
                const uint32_t startTicks = (uint64_t(summon.iStartMs) * 30u + 999u) / 1000u;
                if (!prepareClone(*candidate, member.memberId + ":" + summon.strOccurrenceId + ":" + spawn.strSpawnId,
                    member.offsetTicks + startTicks, summon.iDurationMs,
                    position, (fixed ? 0.f : yaw) + float(spawn.fYawOffsetDegrees))) return fail(status);
            }
        }
        (void)member.actor->Apply_NetworkState(member.initialPosition, member.initialYawDegrees);
    }
    for (auto& clone : clones) members.push_back(std::move(clone));
    return true;
}
bool Client::CKoukuSaydonPresentationPlayer::Begin_BundlePreview(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document, const std::string& bundleId,
    std::uint32_t clockMs, bool paused, std::string& status, const CWorldSequenceDocument* sourceDocument,
    const bool automaticRootMotion, bool externalWorldPreview)
{
    std::string soundStatus;
    CSoundCueCatalog::Load_ClassSnapshot("KoukuSaydon", m_SoundEventVariants, soundStatus);
    const auto bundle = std::find_if(document.Bundles.begin(), document.Bundles.end(),
        [&](const auto& value) { return value.strBundleId == bundleId; });
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level || bundle == document.Bundles.end() || !bundle->strLoadError.empty() ||
        bundle->Members.empty() || bundle->Members.size() > LostArk::Shared::MAX_KOUKUSAYDON_BUNDLE_MEMBERS)
    { status = "Bundle preview requires an active arena and a valid nonempty bundle."; return false; }
    if (externalWorldPreview && (bundle->Members.size() != 1u || bundle->Members.front().iStartOffsetMs))
    { status = "Level WORLD preview requires one Pattern on the common clock."; return false; }
    CWorldGameplayDocument previewWorld;
    if (!previewWorld.Load(CProjectDataRoot::Resolve(std::filesystem::path("Worlds") /
        document.strAreaId / "Gameplay.world.json"), document.strAreaId, status)) return false;
    std::vector<BUNDLE_PREVIEW_MEMBER> staged;
    std::vector<CWorldSequencePlayer*> stagedWorldPlayers;
    KOUKU_SAYDON_COMPOSITION_PATTERN common;
    common.strPatternId = bundleId;
    common.PresentationOccurrences = bundle->PresentationOccurrences;
    common.SceneProfileOccurrences = bundle->SceneProfileOccurrences;
    std::uint64_t duration = Pattern_Duration(common);
    std::set<std::string> targetsUsed, memberIds, placementBindings;
    const auto targets = level->Get_CompositionWorldTargets();
    const auto& sequences = sourceDocument ? *sourceDocument : level->Get_WorldSequenceDocument();
    const auto fail = [&](const std::string& reason)
    { Release_BundlePreviewMembers(staged); status = reason; return false; };
    std::vector<PRESENTATION_WINDOW> globalWindows;
    for (const auto& row : common.SceneProfileOccurrences)
        (void)Admit_PresentationWindow(globalWindows, KIND::SCENE_PROFILE, row.iStartMs,
            double(row.iStartMs) + row.iDurationMs, "common");
    for (const auto& box : common.PresentationOccurrences)
    {
        const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
            [&](const auto& value) { return value.strResourceId == box.strResourceId; });
        if (resource == document.PresentationResources.end() ||
            (resource->eKind != KIND::CAMERA && resource->eKind != KIND::SCENE_PROFILE))
            return fail("Bundle common lane only accepts Camera or Scene Profile resources.");
        (void)Admit_PresentationWindow(globalWindows, resource->eKind, box.iStartMs,
            double(box.iStartMs) + box.iDurationMs + Camera_ReturnMs(*resource, true), "common");
    }
    for (const auto& sourceMember : bundle->Members)
    {
        const auto source = std::find_if(document.Patterns.begin(), document.Patterns.end(),
            [&](const auto& value) { return value.strPatternId == sourceMember.strPatternId; });
        if (source == document.Patterns.end() || !source->strLoadError.empty() ||
            source->strGateId != bundle->strGateId || source->strTargetBossPlacementId.empty() ||
            !targetsUsed.insert(source->strTargetBossPlacementId).second ||
            !memberIds.insert(sourceMember.strMemberId).second || sourceMember.strMemberId.empty())
            return fail("Bundle has an invalid, duplicate, or cross-Gate target/member.");
        const double offset = std::ceil(double(sourceMember.iStartOffsetMs) * 30.0 / 1000.0) * 1000.0 / 30.0;
        for (const auto& row : source->SceneProfileOccurrences)
            if (!Admit_PresentationWindow(globalWindows, KIND::SCENE_PROFILE, offset + row.iStartMs,
                offset + row.iStartMs + row.iDurationMs, sourceMember.strMemberId))
                return fail("Bundle global Scene Profile windows overlap.");
        for (const auto& box : source->PresentationOccurrences)
        {
            const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
                [&](const auto& value) { return value.strResourceId == box.strResourceId; });
            if (resource == document.PresentationResources.end()) return fail("Unknown child presentation resource.");
            if (resource->eKind == KIND::EFFECT && !Validate_EffectAnchor(document, *source, box, status))
                return fail(status);
            if ((resource->eKind == KIND::CAMERA || resource->eKind == KIND::SCENE_PROFILE) &&
                !Admit_PresentationWindow(globalWindows, resource->eKind, offset + box.iStartMs,
                    offset + box.iStartMs + box.iDurationMs + Camera_ReturnMs(*resource, true), sourceMember.strMemberId))
                return fail("Bundle global presentation windows overlap.");
        }
        staged.emplace_back();
        auto& member = staged.back();
        member.memberId = sourceMember.strMemberId;
        member.offsetTicks = static_cast<std::uint32_t>((std::uint64_t(sourceMember.iStartOffsetMs) * 30u + 999u) / 1000u);
        member.pattern = *source;
        const auto* sourcePlacement = previewWorld.Find(source->strTargetBossPlacementId);
        if (!sourcePlacement || sourcePlacement->eKind != WORLD_PLACEMENT_KIND::BOSS)
            return fail("Preview source boss placement is unavailable: " + source->strTargetBossPlacementId);
        member.sourceArchetypeId = sourcePlacement->archetypeId;
        member.arenaCenterPosition = sourcePlacement->position;
        member.facingStages = source->Stages;
        if (!CKoukuSaydonCompositionDocument::Try_ResolveAnimationBlendWindows(document, *source,
            member.pattern.AnimationBlendWindows, status)) return fail(status);
        member.durationMs = Pattern_Duration(*source);
        if (!member.durationMs) return fail("Empty child pattern cannot be previewed.");
        duration = (std::max)(duration, (std::uint64_t(member.offsetTicks) * 1000u + 29u) / 30u + member.durationMs);
        if (duration > MAX_TIMELINE_MS) return fail("Bundle preview exceeds the timeline duration limit.");
        if (!level->Create_CompositionPreviewActor(*source, member.actor, status)) return fail(status);
        const auto model = member.actor->Get_Model();
        if (!CKoukuSaydonAnimationBlend::Validate_ModelWindows(*model, member.pattern.AnimationBlendWindows, status))
            return fail(status);
        member.initialAnimation = model->Get_CurrentAnimIndex();
        const auto& initialRoot = *member.actor->Get_Transform()->Get_WorldMatrixPtr();
        member.initialYawDegrees = XMConvertToDegrees(std::atan2(initialRoot._31, initialRoot._33));
        member.initialPosition = {initialRoot._41, initialRoot._42, initialRoot._43};
        float ignored = 0.f;
        model->Get_AnimationProgress(member.initialAnimation, member.initialTicks, ignored);
        std::uint32_t stageStart = 0u;
        for (const auto& stage : source->Stages)
        {
            for (auto box : stage.AnimationOccurrences)
            {
                bool found = false;
                for (std::uint32_t i = 0; i < model->Get_NumAnimations(); ++i)
                    if (const auto* name = model->Get_AnimationName(i); name && box.strRuntimeClip == name)
                    { found = true; break; }
                if (!found || !box.iPlayMs || !std::isfinite(box.fPlayRate) || box.fPlayRate <= 0.f)
                    return fail("Bundle child animation is unavailable: " + box.strRuntimeClip);
                box.iPoseStartMs = stageStart + (box.strOccurrenceId == std::min_element(stage.AnimationOccurrences.begin(), stage.AnimationOccurrences.end(),
                [](const auto& a, const auto& b) { return a.iStartOffsetMs != b.iStartOffsetMs ?
                    a.iStartOffsetMs < b.iStartOffsetMs : a.strOccurrenceId < b.strOccurrenceId; })->strOccurrenceId ? 0u : box.iStartOffsetMs);
                box.iStartOffsetMs += stageStart;
                member.animations.push_back(std::move(box));
            }
            stageStart += stage.iDurationMs;
        }
        std::sort(member.animations.begin(), member.animations.end(), [](const auto& a, const auto& b)
            { return a.iStartOffsetMs != b.iStartOffsetMs ? a.iStartOffsetMs < b.iStartOffsetMs : a.strOccurrenceId < b.strOccurrenceId; });
        // World-only cinematics already own their visible actors. Keep this
        // boss as an anchor donor without drawing an additional idle body.
        if (document.strCompositionId == "boss.composition.kakulsaydon.sequencer" && member.animations.empty())
            member.actor->Set_PresentationVisible(false);
        bool airborne = false, spatialLogic = false;
        for (const auto& box : source->LogicOccurrences)
        {
            if (!box.bEnabled) continue;
            const auto definition = std::find_if(document.Logics.begin(), document.Logics.end(),
                [&](const auto& logic) { return logic.strLogicId == box.strLogicId; });
            if (definition == document.Logics.end()) continue;
            airborne |= definition->strTriggerKind == "ALBION_AIRBORNE";
            spatialLogic |= (definition->strTriggerKind == "BOSS_TELEPORT_XZ" || definition->strTriggerKind == "BOSS_TELEPORT_GROUNDED" ||
                definition->strTriggerKind == "BOSS_TELEPORT_FACE_CENTER");
            if (definition->strJudgementKind == "BOSS_TRACK_TARGET" ||
                (definition->strJudgementKind == "SHOWTIME_PLAYER_TARGETS" &&
                 (definition->strFixedSelectionGroupId.empty() || !definition->strTrackingPresentationOccurrenceId.empty())))
            {
                auto& tracking = member.targetTracking.emplace_back();
                tracking.occurrenceId = box.strOccurrenceId;
                tracking.startMs = box.iStartMs; tracking.durationMs = box.iDurationMs;
                tracking.immediate = definition->strJudgementKind == "SHOWTIME_PLAYER_TARGETS";
                if (definition->strJudgementKind == "BOSS_TRACK_TARGET")
                    tracking.followSpeedScale = definition->fFollowSpeedScale;
                spatialLogic = true;
            }
        }
        member.spatialLogicPreview = airborne || spatialLogic;
        if ((airborne || spatialLogic) && !CKoukuSaydonPreviewRootMotion::Allows_AutomaticMotion(document, *source))
            return fail("Spatial Logic preview requires no competing boss motion owner.");
        if (airborne && member.animations.empty())
            return fail("Albion airborne preview requires its original source animations.");
        if (((automaticRootMotion && !member.animations.empty()) || airborne || spatialLogic) &&
            CKoukuSaydonPreviewRootMotion::Allows_AutomaticMotion(document, *source))
        {
            member.rootMotion = std::make_unique<CKoukuSaydonPreviewRootMotion>();
            if (!member.rootMotion->Prepare(model, member.animations,
                    float(source->fAnimationRootVerticalScale), status, float(source->fAnimationRootHorizontalScale)) ||
                !member.rootMotion->Prepare_Airborne(document, *source, status) ||
                !member.rootMotion->Begin_Suppression())
                return fail("Bundle child root motion: " + status);
            if (airborne) member.airborneSelectionSeed = std::random_device{}();
        }
        for (const auto& box : source->WorldOccurrences)
        {
            if (externalWorldPreview) continue;
            const auto world = std::find_if(document.Worlds.begin(), document.Worlds.end(),
                [&](const auto& value) { return value.strWorldId == box.strWorldId; });
            if (world == document.Worlds.end()) return fail("Bundle child WORLD resource is missing.");
            const auto* instance = sequences.Find_Instance(world->strSequenceInstanceId);
            if (!instance) return fail("Bundle WORLD sequence is missing: " + world->strSequenceInstanceId);
            if (!level->Can_StartCompositionWorld(world->strSequenceInstanceId, status, &sequences)) return fail(status);
            for (const auto& binding : instance->bindings)
                if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE &&
                    !placementBindings.insert(std::to_string(static_cast<int>(binding.targetKind)) + ":" + binding.targetId).second)
                    return fail("Bundle WORLD members share a mutable map/deploy target.");
            auto player = std::make_shared<CWorldSequencePlayer>();
            const auto [entry, inserted] = member.session.previewWorlds.emplace(box.strOccurrenceId, player);
            if (!inserted) return fail("Bundle WORLD occurrence is duplicated: " + box.strOccurrenceId);
            stagedWorldPlayers.push_back(entry->second.get());
            float3_t offset(float(world->PositionOffset[0]), float(world->PositionOffset[1]), float(world->PositionOffset[2]));
            if (!box.Placement && world->strAnchorKind == "BOSS_SPAWN")
            {
                const auto& pivot = *member.actor->Get_Transform()->Get_WorldMatrixPtr();
                offset.x += pivot._41 - float(world->AnchorPosition[0]);
                offset.y += pivot._42 - float(world->AnchorPosition[1]);
                offset.z += pivot._43 - float(world->AnchorPosition[2]);
            }
            member.worldOffsets.emplace(box.strOccurrenceId, offset);
        }
        member.session.key = "bundle-preview:" + bundleId + ":" + member.memberId;
    }
    if (!stagedWorldPlayers.empty() &&
        !CWorldSequencePlayer::Set_DocumentBatch(sequences, targets, stagedWorldPlayers, status)) return fail(status);
    for (auto& member : staged)
        for (const auto& box : member.pattern.WorldOccurrences)
        {
            if (externalWorldPreview) continue;
            const auto world = std::find_if(document.Worlds.begin(), document.Worlds.end(),
                [&](const auto& value) { return value.strWorldId == box.strWorldId; });
            auto& player = *member.session.previewWorlds.at(box.strOccurrenceId);
            if (!player.Prepare_InstanceResources(world->strSequenceInstanceId, targets))
                return fail("Bundle WORLD " + box.strOccurrenceId + ": " + player.Get_Status());
            if (!player.Validate_ObjectPlacement(world->strSequenceInstanceId, WorldPlacementFromOccurrence(box), status))
                return fail(status);
            const auto worldSpan = player.Get_InstanceElapsedSpanMs(
                world->strSequenceInstanceId, box.fPlaybackSpeed, box.iDurationMs);
            if (worldSpan <= 0.f) return fail("Bundle WORLD has no finite presentation span.");
            duration = (std::max)(duration, (std::uint64_t(member.offsetTicks) * 1000u + 29u) / 30u +
                box.iStartMs + static_cast<std::uint64_t>(std::ceil(worldSpan)));
            if (duration > MAX_TIMELINE_MS) return fail("Bundle WORLD tail exceeds the timeline duration limit.");
        }
    if (!Prepare_CloneSplitPreview(document, staged, status)) return fail(status);
    for (auto& member : staged)
    {
        if (member.offsetTicks || !member.rootMotion || std::none_of(
            member.rootMotion->Airborne_Events().begin(), member.rootMotion->Airborne_Events().end(),
            [](const auto& event) { return event.clockMs == 0u && event.phase == "SELECT_PLAYER" &&
                event.targetPositionPolicy == "SELECT"; })) continue;
        // Pin Play-zero ground before asynchronous Effect preparation can hold the clock.
        // Staging failure leaves the previous preview and its captured positions intact.
        float3_t position;
        float yaw = 0.f;
        if (!Sample_BundlePreviewPose(member, 0.0, position, yaw, true))
            return fail("Play-zero selected ground is unavailable.");
    }
    // All models, clips and WORLD inputs are prepared before replacing the live preview.
    auto stagedDocument = document;
    Stop_Preview();
    m_PreviewDocument = std::move(stagedDocument);
    m_PreviewPattern = std::move(common);
    m_PreviewBundleId = bundleId;
    m_bBundleWorldExternal = externalWorldPreview;
    m_BundlePreviewMembers = std::move(staged);
    m_PreviewSession.key = "bundle-preview:" + bundleId + ":common";
    m_iPreviewDurationMs = static_cast<std::uint32_t>(duration);
    m_fPreviewClockMs = (std::min)(clockMs, m_iPreviewDurationMs);
    m_bOwnPreviewClock = m_bPreviewPlaying = m_bPreviewPivotReady = true;
    m_bPreviewClockAwaitingFirstUpdate = true;
    m_bPreviewPaused = paused;
    XMStoreFloat4x4(&m_PreviewPivot, XMMatrixIdentity());
    Sync_PreviewSourceVisibility();
    if (!externalWorldPreview) Sample_BundlePreview();
    if (!m_bPreviewPlaying) { status = m_strStatus; return false; }
    Refresh_SharedPresentation();
    status = m_strStatus = "Bundle preview ready: " + bundleId;
    return true;
}

bool Client::CKoukuSaydonPresentationPlayer::Begin_ModelReferencePreview(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document, const std::string& selectionId,
    const bool isBundle, const std::uint32_t clockMs, const bool paused, std::string& status,
    const CWorldSequenceDocument* propSequences)
{
    KOUKU_SAYDON_COMPOSITION_DOCUMENT reference;
    reference.iRevision = document.iRevision;
    reference.strAreaId = document.strAreaId;
    // Counter appearance is a read-only actor cue; model reference never executes outcomes.
    for (const auto& logic : document.Logics)
        if (logic.strLogicType == "DURATION" && logic.strJudgementKind == "COUNTER_WINDOW")
            reference.Logics.push_back(logic);
    KOUKU_SAYDON_COMPOSITION_BUNDLE bundle;
    if (isBundle)
    {
        const auto source = std::find_if(document.Bundles.begin(), document.Bundles.end(),
            [&](const auto& value) { return value.strBundleId == selectionId; });
        if (source == document.Bundles.end() || !source->strLoadError.empty())
        { status = "Saved model-reference Bundle is missing or invalid."; return false; }
        bundle = *source;
    }
    else
    {
        const auto source = std::find_if(document.Patterns.begin(), document.Patterns.end(),
            [&](const auto& value) { return value.strPatternId == selectionId; });
        if (source == document.Patterns.end())
        { status = "Saved model-reference Pattern is missing."; return false; }
        // Runtime-only grouping lets one Pattern use the same staged actor path.
        // It is neither a second saved Pattern nor a writable authoring document.
        bundle.strBundleId = "effect.model.reference:" + selectionId;
        bundle.strGateId = source->strGateId;
        bundle.Members.push_back({selectionId, selectionId, 0u});
    }
    bundle.PresentationOccurrences.clear();
    bundle.SceneProfileOccurrences.clear();
    for (const auto& member : bundle.Members)
    {
        const auto source = std::find_if(document.Patterns.begin(), document.Patterns.end(),
            [&](const auto& value) { return value.strPatternId == member.strPatternId; });
        if (source == document.Patterns.end() || !source->strLoadError.empty())
        {
            status = "Model-reference child is missing or invalid: " + member.strPatternId;
            if (source != document.Patterns.end()) status += ". " + source->strLoadError;
            return false;
        }
        auto& pattern = reference.Patterns.emplace_back(*source);
        pattern.BossMotion.reset();
        pattern.fAnimationRootVerticalScale = 1.0;
        pattern.fAnimationRootHorizontalScale = 1.0;
        for (auto& stage : pattern.Stages)
        { stage.bRetargetOnEnter = false; stage.RetargetTarget.reset(); }
        std::erase_if(pattern.LogicOccurrences, [&](const auto& box) {
            return std::none_of(reference.Logics.begin(), reference.Logics.end(),
                [&](const auto& logic) { return logic.strLogicId == box.strLogicId; }); });
        for (auto& box : pattern.LogicOccurrences)
        {
            box.OnSuccessLogicIds.clear(); box.OnFailLogicIds.clear(); box.OnTimeoutLogicIds.clear();
        }
        pattern.SummonOccurrences.clear();
        if (!propSequences) pattern.WorldOccurrences.clear();
        else
        {
            // A model reference may borrow actor props only. It never starts
            // map/deploy choreography, projectile transitions or nested Effects.
            std::vector<KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE> props;
            if (!CEffectCompositionModelPreview::Stage_ActorWorldProps(document, pattern, *propSequences, props, status))
                return false;
            for (const auto& box : props)
            {
                const auto world = std::find_if(document.Worlds.begin(), document.Worlds.end(),
                    [&](const auto& value) { return value.strWorldId == box.strWorldId; });
                if (std::none_of(reference.Worlds.begin(), reference.Worlds.end(),
                    [&](const auto& value) { return value.strWorldId == world->strWorldId; }))
                    reference.Worlds.push_back(*world);
            }
            pattern.WorldOccurrences = std::move(props);
        }
        pattern.SceneProfileOccurrences.clear();
        pattern.PresentationOccurrences.clear();
    }
    const std::string bundleId = bundle.strBundleId;
    reference.Bundles.push_back(std::move(bundle));
    if (!Begin_BundlePreview(reference, bundleId, clockMs, paused, status, propSequences, false)) return false;
    m_bModelReferencePreview = true;
    status = m_strStatus = "Model reference ready. Master cursor owns time; actors stay at authored spawn (no Server movement replay).";
    if (propSequences) status = m_strStatus += " Saved actor-bound WORLD props are enabled.";
    return true;
}

void Client::CKoukuSaydonPresentationPlayer::Sample_ModelReferencePreview(
    const std::uint32_t clockMs, const bool paused)
{
    if (!m_bModelReferencePreview || !m_bPreviewPlaying) return;
    m_fPreviewClockMs = (std::min)(clockMs, m_iPreviewDurationMs);
    m_bPreviewPaused = paused;
    Sample_BundlePreview();
}

bool Client::CKoukuSaydonPresentationPlayer::Place_ModelReferenceRoot(const float4x4_t& root)
{
    if (!m_bModelReferencePreview || !m_bPreviewPlaying || m_BundlePreviewMembers.size() != 1u) return false;
    for (const auto& row : root.m) for (const float value : row) if (!std::isfinite(value)) return false;
    auto& member = m_BundlePreviewMembers.front();
    const float3_t position{root._41, root._42, root._43};
    const float yaw = XMConvertToDegrees(std::atan2(root._31, root._33));
    if (!member.actor || !member.actor->Apply_NetworkState(position, yaw)) return false;
    // Model-reference stages never retarget. Their facing seed must be the
    // explicit preview placement, so resampling props cannot restore spawn yaw.
    member.initialPosition = position;
    member.initialYawDegrees = yaw;
    member.facingCheckpoints.clear();
    member.spatialPoseSamples.clear();
    if (!member.session.previewWorlds.empty()) Sample_BundlePreview();
    return m_bPreviewPlaying;
}

bool Client::CKoukuSaydonPresentationPlayer::Resolve_ModelReferenceWorldPivot(
    const std::string& memberId, const std::string& occurrenceId, float4x4_t& out) const
{
    if (!m_bModelReferencePreview || !m_bPreviewPlaying || occurrenceId.empty()) return false;
    const auto member = std::find_if(m_BundlePreviewMembers.begin(), m_BundlePreviewMembers.end(),
        [&](const auto& value) { return value.memberId == memberId; });
    if (member == m_BundlePreviewMembers.end()) return false;
    const auto box = std::find_if(member->pattern.WorldOccurrences.begin(), member->pattern.WorldOccurrences.end(),
        [&](const auto& value) { return value.strOccurrenceId == occurrenceId; });
    if (box == member->pattern.WorldOccurrences.end()) return false;
    const auto world = std::find_if(m_PreviewDocument.Worlds.begin(), m_PreviewDocument.Worlds.end(),
        [&](const auto& value) { return value.strWorldId == box->strWorldId; });
    const auto player = member->session.previewWorlds.find(occurrenceId);
    return world != m_PreviewDocument.Worlds.end() && player != member->session.previewWorlds.end() &&
        player->second->Try_GetSequencePivot(world->strSequenceInstanceId, out);
}

bool Client::CKoukuSaydonPresentationPlayer::Resolve_ModelReferenceTarget(
    const std::string& memberId, EFFECT_V2_TARGET& target, EFFECT_V2_TARGET_VIEW& view) const
{
    if (!m_bModelReferencePreview || !m_bPreviewPlaying) return false;
    const BUNDLE_PREVIEW_MEMBER* selected = nullptr;
    if (memberId.empty() && m_BundlePreviewMembers.size() == 1u)
        selected = &m_BundlePreviewMembers.front();
    else
        for (const auto& member : m_BundlePreviewMembers)
            if (member.memberId == memberId) { selected = &member; break; }
    if (!selected || !selected->actor) return false;
    auto stagedTarget = EFFECT_V2_TARGET::From_Npc(selected->actor);
    stagedTarget.strArchetypeId = std::string(CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(
        selected->pattern.strTargetBossPlacementId));
    EFFECT_V2_TARGET_VIEW stagedView;
    if (!CEffectV2Object::Resolve_TargetView(stagedTarget, stagedView)) return false;
    target = std::move(stagedTarget);
    view = std::move(stagedView);
    return true;
}

bool Client::CKoukuSaydonPresentationPlayer::Resolve_PatternModelTarget(
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, const ANIMATION_BONE_TARGET target,
    ANIMATION_MODEL_TARGET_VIEW& view) const
{
    // The preview owns complete CNpcs and never binds the global Animation Tool target.
    if (m_bPreviewPlaying && !m_bModelReferencePreview)
        for (const auto& member : m_BundlePreviewMembers)
            if (!member.finiteActorLifetime && member.pattern.strPatternId == pattern.strPatternId &&
                member.pattern.strActorProfileId == pattern.strActorProfileId && member.actor &&
                member.actor->Try_GetAnimationModelTarget(target, view)) return true;
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    const auto archetype = CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(pattern.strTargetBossPlacementId);
    if (!level || archetype.empty() || CKoukuSaydonCompositionDocument::Resolve_ActorProfileForPlacement(
        pattern.strTargetBossPlacementId) != pattern.strActorProfileId) return false;
    std::vector<KOUKU_BOSS_PRESENTATION_VIEW> bosses;
    std::vector<KOUKU_CARD_PRESENTATION_VIEW> players;
    level->Collect_KoukuPresentationViews(bosses, players);
    for (const auto& boss : bosses)
        if (boss.strArchetypeId == archetype && boss.iOwnerBossNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID)
            if (const auto actor = boss.pNpc.lock(); actor && actor->Try_GetAnimationModelTarget(target, view)) return true;
    return false;
}

bool Client::CKoukuSaydonPresentationPlayer::Sample_BundlePreviewFacing(
    BUNDLE_PREVIEW_MEMBER& member, const double localMs)
{
    float3_t position;
    float yaw = 0.f;
    return Sample_BundlePreviewPose(member, localMs, position, yaw, true) &&
        member.actor->Apply_NetworkState(position, yaw);
}

bool Client::CKoukuSaydonPresentationPlayer::Sample_BundlePreviewPose(
    BUNDLE_PREVIEW_MEMBER& member, const double localMs, float3_t& position, float& yaw,
    const bool recordTargets)
{
    if (!std::isfinite(localMs)) return false;
    const auto cached = member.spatialPoseSamples.find(localMs);
    if (cached != member.spatialPoseSamples.end())
    {
        position = cached->second.first;
        yaw = cached->second.second;
        return true;
    }
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    const auto player = level ? level->Get_LocalCharacter() : nullptr;
    std::vector<KOUKU_BOSS_PRESENTATION_VIEW> bosses;
    std::vector<KOUKU_CARD_PRESENTATION_VIEW> players;
    if (recordTargets && level && (!member.targetTracking.empty() ||
        std::any_of(member.pattern.Stages.begin(), member.pattern.Stages.end(), [](const auto& stage) {
            return stage.bRetargetOnEnter && stage.RetargetTarget == "NEAREST_ALIVE"; }) ||
        (member.rootMotion && !member.rootMotion->Airborne_Events().empty())))
        level->Collect_KoukuPresentationViews(bosses, players);
    players.erase(std::remove_if(players.begin(), players.end(), [](const auto& view) {
        const auto& s = view.Snapshot;
        using LostArk::Shared::PLAYER_ACTION_STATE;
        return !s.iCurrentHp || s.eAction == PLAYER_ACTION_STATE::DEAD ||
            s.eAction == PLAYER_ACTION_STATE::FALLING || s.eAction == PLAYER_ACTION_STATE::GRABBED ||
            !std::isfinite(s.fPositionX) || !std::isfinite(s.fPositionY) || !std::isfinite(s.fPositionZ);
    }), players.end());
    std::sort(players.begin(), players.end(), [](const auto& a, const auto& b) { return a.Snapshot.iNetEntityId < b.Snapshot.iNetEntityId; });
    std::vector<float3_t> selectionPositions;
    if (member.rootMotion && !member.rootMotion->Airborne_Events().empty())
    {
        const auto& events = member.rootMotion->Airborne_Events();
        selectionPositions.resize(events.size(), member.initialPosition);
        const auto pin = [&](const std::string& id, const bool captureGround) {
            if (!recordTargets || players.empty()) return false;
            uint32_t seed = member.airborneSelectionSeed;
            for (const unsigned char c : id) seed = (seed ^ c) * 16777619u;
            std::mt19937 generator(seed);
            const auto& view = players[std::uniform_int_distribution<size_t>(0u, players.size() - 1u)(generator)];
            const auto& target = view.Snapshot;
            float3_t selected{target.fPositionX, target.fPositionY, target.fPositionZ};
            if (captureGround)
            {
                const auto character = view.pCharacter.lock();
                float3_t ground;
                if (!character || !character->Try_SampleTargetGround(selected.x, selected.z, ground) ||
                    !std::isfinite(ground.x) || !std::isfinite(ground.y) || !std::isfinite(ground.z)) return false;
                selected = ground;
            }
            member.airborneSelections[id] = {target.iNetEntityId, selected};
            return true;
        };
        size_t selection = SIZE_MAX;
        for (size_t i = 0u; i < events.size(); ++i)
        {
            const auto& event = events[i];
            if (event.clockMs > localMs) break;
            if (event.phase == "SELECT_PLAYER")
            {
                selection = i;
                if (!member.airborneSelections.contains(event.occurrenceId) &&
                    !pin(event.occurrenceId, event.targetPositionPolicy == "SELECT")) return false;
            }
            else if (event.phase == "APPEAR_PLAYER")
            {
                // Like the Server, select at appearance when no explicit selection preceded it.
                // Retain that identity for later appearances, including after a backward seek.
                if (selection == SIZE_MAX) selection = i;
                if (member.airborneAppearancePositions.contains(event.occurrenceId)) continue;
                if (!recordTargets) return false;
                const auto& id = events[selection].occurrenceId;
                if (!member.airborneSelections.contains(id) && !pin(id, false)) return false;
                if (events[selection].phase == "SELECT_PLAYER" && events[selection].targetPositionPolicy == "SELECT")
                {
                    // Selection owns this ground point even if the player moves or leaves.
                    member.airborneAppearancePositions[event.occurrenceId] = member.airborneSelections.at(id).second;
                    continue;
                }
                auto chosen = member.airborneSelections.at(id).first;
                if (std::none_of(players.begin(), players.end(), [&](const auto& view) { return view.Snapshot.iNetEntityId == chosen; }))
                { if (!pin(id, false)) return false; chosen = member.airborneSelections.at(id).first; }
                const auto target = std::find_if(players.begin(), players.end(), [&](const auto& view) { return view.Snapshot.iNetEntityId == chosen; });
                if (target == players.end()) return false;
                const auto& s = target->Snapshot;
                member.airborneAppearancePositions[event.occurrenceId] = {s.fPositionX, s.fPositionY, s.fPositionZ};
            }
        }
        for (size_t i = 0u; i < events.size(); ++i)
        {
            if (const auto pin = member.airborneSelections.find(events[i].occurrenceId); pin != member.airborneSelections.end())
                selectionPositions[i] = pin->second.second;
            if (const auto pin = member.airborneAppearancePositions.find(events[i].occurrenceId); pin != member.airborneAppearancePositions.end())
                selectionPositions[i] = pin->second;
        }
    }
    yaw = member.initialYawDegrees;
    std::vector<float> rowYaws(member.animations.size(), yaw);
    /* Tracked follow is a walk the Server owns, not source root motion. The
       accumulated XZ carried here is added after the authored clock is sampled. */
    float3_t followOffset{};
    double completedClock = -1.0;
    const auto upper = member.facingCheckpoints.upper_bound(localMs);
    if (upper != member.facingCheckpoints.begin())
    {
        const auto& [clock, checkpoint] = *std::prev(upper);
        completedClock = clock;
        yaw = checkpoint.yawDegrees;
        rowYaws = checkpoint.animationYaws;
        followOffset = checkpoint.followOffset;
    }
    const auto positionAt = [&](double clock, float3_t& position) {
        const auto carryFollow = [&](const bool sampled) {
            if (sampled) { position.x += followOffset.x; position.z += followOffset.z; }
            return sampled;
        };
        position = member.initialPosition;
        if (!member.rootMotion) return carryFollow(true);
        if (!selectionPositions.empty())
            return carryFollow(member.rootMotion->Sample_AirbornePosition(clock, rowYaws,
                member.initialPosition, selectionPositions, position));
        float3_t displacement;
        if (!member.rootMotion->Sample_Displacement(clock, rowYaws, displacement)) return false;
        position = {member.initialPosition.x + displacement.x,
            member.initialPosition.y + displacement.y, member.initialPosition.z + displacement.z};
        return carryFollow(true);
    };
    const auto targetYawAt = [&](const float3_t& origin, const float3_t& target, float& targetYaw) {
        const float dx = target.x - origin.x, dz = target.z - origin.z;
        if (!std::isfinite(dx) || !std::isfinite(dz) || dx * dx + dz * dz <= .000001f) return false;
        targetYaw = XMConvertToDegrees(std::atan2(dx, dz));
        if (CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(
            member.pattern.strTargetBossPlacementId) == "BOSS_KAKULSAYDON_G2_BIG_SAYDON") targetYaw -= 90.f;
        return true;
    };
    struct FACING_EVENT final
    {
        double clock = 0.0;
        const KOUKU_SAYDON_COMPOSITION_STAGE* stage = nullptr;
        BUNDLE_PREVIEW_MEMBER::TARGET_TRACKING_WINDOW* tracking = nullptr;
        uint32_t tick = 0u, remainingTicks = 0u;
        const CKoukuSaydonPreviewRootMotion::AIRBORNE_EVENT* teleport = nullptr;
    };
    std::vector<FACING_EVENT> facingEvents;
    uint32_t stageStartMs = 0u;
    for (const auto& stage : member.facingStages)
    {
        if (stageStartMs > completedClock && stageStartMs <= localMs) facingEvents.push_back({double(stageStartMs), &stage});
        stageStartMs += stage.iDurationMs;
    }
    if (member.rootMotion)
        for (const auto& teleport : member.rootMotion->Airborne_Events())
            if (teleport.phase == "TELEPORT_FACE_CENTER" && teleport.clockMs > completedClock && teleport.clockMs <= localMs)
                facingEvents.push_back({double(teleport.clockMs), nullptr, nullptr, 0u, 0u, &teleport});
    for (auto& window : member.targetTracking)
    {
        const auto begin = (uint64_t(window.startMs) * 30u + 999u) / 1000u;
        const auto end = ((uint64_t(window.startMs) + window.durationMs) * 30u + 999u) / 1000u;
        const auto first = (std::max)(begin, completedClock < 0.0 ? uint64_t(0u) :
            static_cast<uint64_t>(std::floor(completedClock * 30.0 / 1000.0)));
        for (auto tick = first; tick < end && double(tick) * 1000.0 / 30.0 <= localMs; ++tick)
            if (double(tick) * 1000.0 / 30.0 > completedClock) facingEvents.push_back({double(tick) * 1000.0 / 30.0, nullptr, &window,
                uint32_t(tick), uint32_t(end - tick)});
    }
    std::stable_sort(facingEvents.begin(), facingEvents.end(), [](const auto& a, const auto& b) {
        if (a.clock != b.clock) return a.clock < b.clock;
        if ((a.stage != nullptr) != (b.stage != nullptr)) return a.stage != nullptr;
        return a.teleport != nullptr && b.teleport == nullptr;
    });
    double pendingClock = -1.0;
    const auto commitCheckpoint = [&] {
        if (pendingClock >= 0.0)
            member.facingCheckpoints.insert_or_assign(pendingClock,
                BUNDLE_PREVIEW_MEMBER::FACING_CHECKPOINT{yaw, rowYaws, followOffset});
    };
    for (const auto& event : facingEvents)
    {
        if (pendingClock != event.clock)
        {
            commitCheckpoint();
            pendingClock = event.clock;
        }
        // An absolute relocation supersedes pursuit accumulated before this event.
        if (event.teleport) followOffset = {};
        float3_t origin;
        if (!positionAt(event.clock, origin)) return false;
        if (event.teleport)
        {
            const float dx = member.arenaCenterPosition.x - event.teleport->destination.x;
            const float dz = member.arenaCenterPosition.z - event.teleport->destination.z;
            if (dx * dx + dz * dz > .000001f)
            {
                const auto& archetype = member.sourceArchetypeId;
                const bool saydon = archetype == "BOSS_KAKULSAYDON_G1_SAYDON" ||
                    archetype == "BOSS_KAKULSAYDON_G3_SAYDON" ||
                    archetype == "BOSS_KAKULSAYDON_BINGO_SAYDON" ||
                    archetype == "BOSS_KAKULSAYDON_G2_BIG_SAYDON";
                yaw = XMConvertToDegrees(std::atan2(dx, dz)) - (saydon ? 90.f : 0.f);
            }
            size_t activeRow = 0u;
            for (size_t i = 0u; i < member.animations.size(); ++i)
                if (member.animations[i].iPoseStartMs <= event.clock) activeRow = i;
            for (size_t i = activeRow; i < rowYaws.size(); ++i) rowYaws[i] = yaw;
            continue;
        }
        if (event.stage)
        {
            const auto& stage = *event.stage;
            if (stage.bRetargetOnEnter)
            {
                auto sample = member.stageFacingYawDegrees.find(stage.strStageId);
                const bool inserted = sample == member.stageFacingYawDegrees.end();
                if (inserted)
                {
                    if (!recordTargets) return false;
                    sample = member.stageFacingYawDegrees.emplace(stage.strStageId, yaw).first;
                }
                std::optional<float3_t> targetPosition;
                if (inserted && stage.RetargetTarget == "NEAREST_ALIVE")
                {
                    // Match Server XZ nearest selection; replicated IDs break distance ties.
                    const auto closest = std::min_element(players.begin(), players.end(), [&](const auto& left, const auto& right) {
                        const auto distance = [&](const auto& view) {
                            const auto& s = view.Snapshot;
                            const float dx = s.fPositionX - origin.x, dz = s.fPositionZ - origin.z;
                            return dx * dx + dz * dz;
                        };
                        const auto a = distance(left), b = distance(right);
                        return a != b ? a < b : left.Snapshot.iNetEntityId < right.Snapshot.iNetEntityId;
                    });
                    if (closest != players.end())
                    {
                        const auto& s = closest->Snapshot;
                        targetPosition = float3_t{s.fPositionX, s.fPositionY, s.fPositionZ};
                    }
                }
                else if (inserted && player && player->Get_Transform())
                {
                    const auto& target = *player->Get_Transform()->Get_WorldMatrixPtr();
                    targetPosition = float3_t{target._41, target._42, target._43};
                }
                if (targetPosition)
                {
                    if (targetYawAt(origin, *targetPosition, sample->second) &&
                        CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(
                            member.pattern.strTargetBossPlacementId) == "BOSS_KAKULSAYDON_G2_KOUKU")
                        sample->second -= 90.f; // Match the Server stage retarget for the laser's model +X front.
                }
                yaw = sample->second;
            }
            for (size_t i = 0u; i < member.animations.size(); ++i)
                if (member.animations[i].iPoseStartMs >= event.clock &&
                    member.animations[i].iPoseStartMs < event.clock + stage.iDurationMs)
                    rowYaws[i] = yaw;
            continue;
        }
        auto& window = *event.tracking;
        auto sample = window.targetSamples.find(event.tick);
        const bool inserted = sample == window.targetSamples.end();
        if (inserted)
        {
            if (!recordTargets) return false;
            sample = window.targetSamples.emplace(event.tick, std::nullopt).first;
        }
        if (inserted && !players.empty())
        {
            auto target = std::find_if(players.begin(), players.end(), [&](const auto& view) {
                return view.Snapshot.iNetEntityId == window.targetEntityId;
            });
            if (target == players.end())
            {
                uint32_t seed = member.airborneSelectionSeed;
                for (const unsigned char c : window.occurrenceId) seed = (seed ^ c) * 16777619u;
                target = players.begin() + seed % players.size();
                window.targetEntityId = target->Snapshot.iNetEntityId;
            }
            const auto& s = target->Snapshot;
            sample->second = float3_t{s.fPositionX, s.fPositionY, s.fPositionZ};
            window.targetMoveSpeeds[event.tick] = std::isfinite(s.fMoveSpeed) ? s.fMoveSpeed : 0.f;
        }
        float targetYaw = yaw;
        if (!sample->second || !targetYawAt(origin, *sample->second, targetYaw)) continue;
        const auto movingArchetype = CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(
            member.pattern.strTargetBossPlacementId);
        const bool saydon = movingArchetype == "BOSS_KAKULSAYDON_G1_SAYDON" ||
            movingArchetype == "BOSS_KAKULSAYDON_G3_SAYDON" ||
            movingArchetype == "BOSS_KAKULSAYDON_BINGO_SAYDON" ||
            movingArchetype == "BOSS_KAKULSAYDON_G2_BIG_SAYDON";
        // Stationary Showtime and moving pursuit use the same model +X front.
        // targetYawAt already applies BIG_SAYDON's existing basis correction.
        if (saydon && movingArchetype != "BOSS_KAKULSAYDON_G2_BIG_SAYDON")
            targetYaw = float(std::remainder(double(targetYaw) - 90.0, 360.0));
        if (window.immediate) yaw = targetYaw;
        else
        {
            const double turn = std::remainder(double(targetYaw) - yaw, 360.0);
            // Movement must keep steering when its lifetime is extended. Match the
            // Server's 180 degrees/second turn and shared rotate-only response.
            const double step = window.followSpeedScale > 0.0 ?
                (std::clamp)(turn, -6.0, 6.0) :
                turn * LostArk::Shared::KoukuTargetTracking::RotateOnlyFraction(1u, event.remainingTicks);
            yaw = float(std::remainder(double(yaw) + step, 360.0));
        }
        if (window.followSpeedScale > 0.0)
        {
            /* Mirrors the Server step: rotate first, then walk one fixed tick along the
               new facing. The preview keeps the authored height; the Server resolves
               terrain, navigation and body collision for the committed pose. */
            const auto pinned = window.targetMoveSpeeds.find(event.tick);
            const float step = pinned == window.targetMoveSpeeds.end() ? 0.f :
                pinned->second * float(window.followSpeedScale) / 30.f;
            if (std::isfinite(step) && step > 0.f)
            {
                const float radians = XMConvertToRadians(yaw + (saydon ? 90.f : 0.f));
                followOffset.x += std::sin(radians) * step;
                followOffset.z += std::cos(radians) * step;
            }
        }
    }
    if (!positionAt((std::clamp)(localMs, 0.0, double(member.durationMs)), position)) return false;
    commitCheckpoint();
    // A long edit session may visit arbitrary fractional clocks. Bound memoized
    // final poses; evicted clocks can still resume from fixed-event checkpoints.
    if (member.spatialPoseSamples.size() >= 16384u)
        member.spatialPoseSamples.erase(member.spatialPoseSamples.begin());
    member.spatialPoseSamples.emplace(localMs, std::make_pair(position, yaw));
    return true;
}

bool Client::CKoukuSaydonPresentationPlayer::Prepare_PreviewEffects()
{
    if (!m_bPreviewPreparationQueued)
    {
        std::set<std::string> targets;
        const auto collect = [&](const auto& pattern) {
            for (const auto& box : pattern.PresentationOccurrences)
                for (const auto& resource : m_PreviewDocument.PresentationResources)
                    if (resource.strResourceId == box.strResourceId && resource.eKind == KIND::EFFECT &&
                        (resource.strResourceKind == "V1_EFFECT" || resource.strResourceKind == "V1_ELEMENT"))
                        targets.insert(resource.strAssetId);
            return Collect_LogicPreviewEffects(m_PreviewDocument, pattern, targets);
        };
        if (!collect(m_PreviewPattern)) { const auto status = m_strStatus; Fail_Preview(status); return false; }
        for (const auto& member : m_BundlePreviewMembers)
            if (!collect(member.pattern)) { const auto status = m_strStatus; Fail_Preview(status); return false; }
        m_PreviewPreparationTargets.assign(targets.begin(), targets.end());
        if (!m_PreviewPreparationTargets.empty())
        {
            std::vector<std::string> admitted;
            std::string status;
            if (!CEffectPresentationService::Queue_ProductTargets_Priority(m_PreviewPreparationTargets, admitted, status))
            { Fail_Preview("Preview Effect preparation could not queue: " + status); return false; }
        }
        m_bPreviewPreparationQueued = true;
    }
    if (m_PreviewPreparationTargets.empty()) return true;
    const auto probe = CEffectPresentationService::Get_ProductCuePreparationProbe(m_PreviewPreparationTargets);
    // Failed targets are reported by their own occurrence. Preparation time
    // never consumes a short Effect window on the authoring clock.
    if (probe.bCatalogRevisionCurrent && probe.bSettled)
    {
        // Do not leave a held-clock message visible after this gate has opened,
        // or overwrite a later occurrence-specific failure on every sample.
        if (m_strStatus.starts_with("Preparing preview Effects;"))
            m_strStatus = "Preview Effects prepared: " + std::to_string(probe.iPreparedCount) + " ready, " +
                std::to_string(probe.iFailedCount) + " failed, " + std::to_string(probe.iUnavailableCount) + " unavailable.";
        return true;
    }
    if (!probe.strBlockingFailure.empty())
    {
        Fail_Preview("Preview Effect preparation stopped: " + probe.strBlockingFailure);
        return false;
    }
    m_strStatus = "Preparing preview Effects; " + std::to_string(probe.iTargetCount - probe.iPendingCount) +
        "/" + std::to_string(probe.iTargetCount) + " settled (" + std::to_string(probe.iFailedCount) +
        " failed, " + std::to_string(probe.iUnavailableCount) + " unavailable); timeline is held at " +
        std::to_string(Preview_ClockMs()) + " ms.";
    return false;
}

const Client::KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE*
Client::CKoukuSaydonPresentationPlayer::Resolve_PreviewAnimation(
    const BUNDLE_PREVIEW_MEMBER& member, const double localMs,
    const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE*& previous)
{
    const auto eligible = [&](const auto& row) {
        const auto window = member.cloneAnimationWindows.find(row.strOccurrenceId);
        return window == member.cloneAnimationWindows.end() ||
            (localMs >= window->second.first && localMs < window->second.second);
    };
    const auto sampleMs = (std::clamp)(localMs, 0.0, double(member.durationMs));
    const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE* selected = nullptr;
    previous = nullptr;
    for (const auto& row : member.animations)
        if (eligible(row) && sampleMs >= row.iPoseStartMs && localMs >= 0.0)
        { previous = selected; selected = &row; }
    if (!selected)
        for (const auto& row : member.animations)
            if (eligible(row)) { selected = &row; break; }
    return selected;
}

void Client::CKoukuSaydonPresentationPlayer::Sync_PreviewSourceVisibility()
{
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    std::vector<KOUKU_BOSS_PRESENTATION_VIEW> bosses;
    std::vector<KOUKU_CARD_PRESENTATION_VIEW> players;
    if (level) level->Collect_KoukuPresentationViews(bosses, players);
    for (auto& member : m_BundlePreviewMembers)
    {
        std::shared_ptr<CNpc> replacement;
        // Synthetic split/summon clones do not replace an authoritative boss.
        if (m_bPreviewPlaying && !member.finiteActorLifetime && !member.sourceArchetypeId.empty())
            for (const auto& boss : bosses)
                if (!boss.iOwnerBossNetEntityId && boss.strArchetypeId == member.sourceArchetypeId)
                { replacement = boss.pNpc.lock(); break; }
        const auto previous = member.suppressedSourceActor.lock();
        if (previous == replacement) continue;
        if (previous) previous->Release_CompositionPreviewSuppression();
        member.suppressedSourceActor = replacement;
        if (replacement) replacement->Acquire_CompositionPreviewSuppression();
    }
}

void Client::CKoukuSaydonPresentationPlayer::Sample_BundlePreview()
{
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    Sync_PreviewSourceVisibility();
    if (!level || !m_bPreviewPlaying || !Prepare_PreviewEffects()) return;
    std::uint32_t effectiveMs = Preview_ClockMs();
    if (!Resolve_PreviewCaptureClock(effectiveMs, effectiveMs))
    { const auto error = m_strStatus; Fail_Preview(error); return; }
    // The displayed cursor is integer milliseconds; keep the owned clock
    // remainder unless capture actually holds or moves that cursor.
    if (m_bPreviewCaptureClockHeld || effectiveMs != Preview_ClockMs())
        m_fPreviewClockMs = effectiveMs;
    std::string worldPreviewStatus;
    const auto targets = level->Get_CompositionWorldTargets();
    for (auto& member : m_BundlePreviewMembers)
    {
        const double localMs = m_fPreviewClockMs - double(member.offsetTicks) * 1000.0 / 30.0;
        if (member.finiteActorLifetime)
            member.actor->Set_PresentationVisible(localMs >= 0.0 && localMs < member.durationMs);
        const auto model = member.actor->Get_Model();
        (void)model->Set_RootMotionVerticalScale(member.rootMotion ? 0.f : float(member.pattern.fAnimationRootVerticalScale));
        const auto sampleMs = static_cast<float>((std::clamp)(localMs, 0.0, double(member.durationMs)));
        if (!Sample_BundlePreviewFacing(member, localMs))
        { Fail_Preview("Bundle root-motion sample or actor target is unavailable."); return; }
        std::array<double, 3u> bossPosition{};
        double bossYaw = 0.0;
        if (Sample_KoukuSaydonBossMotion(member.pattern, sampleMs, bossPosition, bossYaw))
            (void)member.actor->Apply_NetworkState(
                {float(bossPosition[0]), float(bossPosition[1]), float(bossPosition[2])}, float(bossYaw));
        const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE* previousAnimation = nullptr;
        const auto* animation = Resolve_PreviewAnimation(member, localMs, previousAnimation);
        std::uint32_t animationIndex = member.initialAnimation;
        float ticks = member.initialTicks;
        if (animation)
        {
            for (std::uint32_t i = 0; i < model->Get_NumAnimations(); ++i)
                if (const auto* name = model->Get_AnimationName(i); name && animation->strRuntimeClip == name)
                { animationIndex = i; break; }
            float oldTicks = 0.f, clipTicks = 0.f;
            model->Get_AnimationProgress(animationIndex, oldTicks, clipTicks);
            const float tps = model->Get_AnimationTickPerSecond(animationIndex);
            const float elapsed = (std::max)(0.f, sampleMs - animation->iStartOffsetMs);
            const float age = (std::min)(elapsed, float(animation->iPlayMs));
            double sourceMs = 0.0;
            if (!CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(animation->iSourceStartMs,
                animation->iSourceEndMs, age, animation->fPlayRate, clipTicks * 1000.0 / tps,
                animation->strEndPolicy == "LOOP_TO_WINDOW", sourceMs))
            { Fail_Preview("Animation source range is outside the native clip."); return; }
            ticks = float(sourceMs * tps / 1000.0);
        }
        model->Set_Animation(animationIndex, false, 0.f);
        model->Set_AnimPaused(true);
        model->Set_AnimTrackPosition(animationIndex, ticks);
        model->Update_Animation(0.f);
        CModel::ANIMATION_TRANSITION_POSE logicPose;
        bool logicActive = false;
        std::string blendStatus;
        if (!CKoukuSaydonAnimationBlend::Sample_Pose(*model, member.pattern.AnimationBlendWindows,
            sampleMs, logicPose, logicActive, blendStatus) ||
            (logicActive && !model->Set_AnimationTransitionPose(logicPose)))
        { Fail_Preview("Logic animation blend failed: " + blendStatus); return; }
        if (!logicActive && animation && animation->iBlendInMs && previousAnimation)
        {
            CModel::ANIMATION_TRANSITION_POSE pose;
            pose.targetIndex = animationIndex; pose.targetTicks = ticks;
            pose.durationSeconds = animation->iBlendInMs * .001f;
            pose.elapsedSeconds = (sampleMs - animation->iStartOffsetMs) * .001f;
            pose.playRate = animation->fPlayRate;
            for (uint32_t i = 0; i < model->Get_NumAnimations(); ++i)
                if (previousAnimation->strRuntimeClip == model->Get_AnimationName(i)) { pose.sourceIndex = i; break; }
            float cursor = 0.f, sourceEnd = 0.f;
            if (pose.sourceIndex == UINT32_MAX || !model->Get_AnimationProgress(pose.sourceIndex, cursor, sourceEnd))
                m_strStatus = "Animation blend source clip is unavailable.";
            else
            {
                const float previousTps = model->Get_AnimationTickPerSecond(pose.sourceIndex);
                double previousMs = 0.0;
                if (!CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(previousAnimation->iSourceStartMs,
                    previousAnimation->iSourceEndMs, previousAnimation->iPlayMs, previousAnimation->fPlayRate,
                    sourceEnd * 1000.0 / previousTps, previousAnimation->strEndPolicy == "LOOP_TO_WINDOW", previousMs))
                { Fail_Preview("Animation blend source range is outside the native clip."); return; }
                pose.sourceTicks = float(previousMs * previousTps / 1000.0);
                if (!model->Set_AnimationTransitionPose(pose)) m_strStatus = "Animation blend pose admission failed.";
            }
        }
        for (const auto& box : member.pattern.WorldOccurrences)
        {
            if (m_bBundleWorldExternal && !member.finiteActorLifetime) continue;
            const auto world = std::find_if(m_PreviewDocument.Worlds.begin(), m_PreviewDocument.Worlds.end(),
                [&](const auto& value) { return value.strWorldId == box.strWorldId; });
            auto& player = member.session.previewWorlds.at(box.strOccurrenceId);
            auto worldTargets = targets;
            worldTargets.bossAnchor = [&member](const std::string& archetype, const std::string& bone,
                CWorldSequencePlayer::PLAYER_ANCHOR& out, std::string& status)
            {
                bool found = false;
                for (const auto& [id, world] : member.session.previewWorlds)
                {
                    CWorldSequencePlayer::PLAYER_ANCHOR candidate;
                    std::string failure;
                    if (!world->Try_GetPresentationBossAnchor(archetype, bone, candidate, failure))
                    { if (!failure.empty()) { status = std::move(failure); return false; } continue; }
                    if (found && candidate.bodyModel != out.bodyModel)
                    { status = "Bundle cinematic World boss anchor is ambiguous: " + archetype; return false; }
                    out = candidate; found = true;
                }
                if (found) return true;
                if (archetype != CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(member.pattern.strTargetBossPlacementId) ||
                    !member.actor || !member.actor->Get_Transform())
                { status = "World Object Boss anchor does not match this preview actor: " + archetype; return false; }
                return CWorldSequencePlayer::Resolve_BossBoneAnchor(member.actor->Get_Model(),
                    *member.actor->Get_Transform()->Get_WorldMatrixPtr(), bone, out, status);
            };
            worldTargets.objectEmissionAnchor = Make_WorldEmissionAnchor(member.pattern, *world, box);
            const auto span = player->Get_InstanceElapsedSpanMs(world->strSequenceInstanceId,
                box.fPlaybackSpeed, box.iDurationMs);
            if (localMs < box.iStartMs || localMs >= double(box.iStartMs) + span)
            { player->Stop_All(worldTargets, true); continue; }
            if (!player->Is_Playing(world->strSequenceInstanceId) &&
                !player->Play(world->strSequenceInstanceId, worldTargets, box.fPlaybackSpeed,
                    member.worldOffsets.at(box.strOccurrenceId), box.iDurationMs, WorldPlacementFromOccurrence(box)))
            {
                Fail_Preview("Bundle WORLD failed: " + box.strOccurrenceId + ": " + player->Get_Status());
                return;
            }
            if (!player->Seek_InstanceToMs(world->strSequenceInstanceId, float(localMs - box.iStartMs), worldTargets))
            {
                Fail_Preview("Bundle WORLD failed: " + box.strOccurrenceId + ": " + player->Get_Status());
                return;
            }
        }
        member.actor->Synchronize_WeaponPose();
        const float counterAge = localMs >= 0.0 && localMs < member.durationMs ?
            Counter_AfterimageAgeSeconds(m_PreviewDocument, member.pattern, localMs) : -1.f;
        member.actor->Set_CounterAfterimageEnabled(counterAge >= 0.f, counterAge);
        bool backstepAfterimage = false;
        const bool chargeAfterimage = localMs >= 0.0 && localMs < member.durationMs &&
            Charge_AfterimageActive(member.pattern, float(localMs), backstepAfterimage);
        member.actor->Set_ChargeAfterimageEnabled(chargeAfterimage, backstepAfterimage,
            float((std::max)(0.0, localMs) * .001));
#ifdef _DEBUG
        if (m_bBundleWorldExternal && !member.finiteActorLifetime)
        {
            // External WORLD is admitted only for one zero-offset member. Its
            // freshly sampled BODY, WORLD props and Effects share this clock.
            const auto actor = member.actor;
            const std::string actorArchetype(CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(
                member.pattern.strTargetBossPlacementId));
            const auto bossAnchor = [actor, actorArchetype](const std::string& archetype,
                const std::string& bone, CWorldSequencePlayer::PLAYER_ANCHOR& out, std::string& status)
            {
                if (archetype != actorArchetype || !actor || !actor->Get_Transform())
                { status = "World Object Boss anchor does not match this preview actor: " + archetype; return false; }
                return CWorldSequencePlayer::Resolve_BossBoneAnchor(actor->Get_Model(),
                    *actor->Get_Transform()->Get_WorldMatrixPtr(), bone, out, status);
            };
            if (!level->Debug_SampleCompositionWorldPreview(m_PreviewBundleId, true,
                effectiveMs, worldPreviewStatus, bossAnchor))
            { Fail_Preview(worldPreviewStatus); return; }
        }
#endif
        ANIMATION_MODEL_TARGET_VIEW weaponView;
        const bool hasWeapon = member.actor->Try_GetAnimationModelTarget(ANIMATION_BONE_TARGET::WEAPON, weaponView);
        if (localMs < 0.0 || localMs >= member.durationMs) Stop_Session(member.session);
        else Sample(member.session, m_PreviewDocument, member.pattern, sampleMs, m_bPreviewPaused,
            *member.actor->Get_Transform()->Get_WorldMatrixPtr(), model, hasWeapon ? &weaponView : nullptr);
    }
    Sample(m_PreviewSession, m_PreviewDocument, m_PreviewPattern,
        float(m_fPreviewClockMs), m_bPreviewPaused, m_PreviewPivot, nullptr);
    // A queued/successful Effect must not hide a recoverable WORLD anchor wait.
    if (!worldPreviewStatus.empty()) m_strStatus = std::move(worldPreviewStatus);
    if (m_bPreviewCaptureClockHeld && Preview_ClockMs() == m_iPreviewCaptureBoundaryMs)
        m_bPreviewCaptureBoundarySampled = true;
}

void Client::CKoukuSaydonPresentationPlayer::Set_PreviewPivot(
    const float4x4_t& pivot, const std::shared_ptr<Engine::CModel>& model)
{
    m_PreviewPivot = pivot;
    m_PreviewModel = model;
    m_bPreviewPivotReady = true;
}

bool Client::CKoukuSaydonPresentationPlayer::Resolve_PreviewCaptureClock(
    std::uint32_t requestedMs, std::uint32_t& effectiveMs)
{
    effectiveMs = requestedMs;
    if (!m_bPreviewPlaying) return true;
    if (!Prepare_PreviewEffects()) { effectiveMs = Preview_ClockMs(); return m_bPreviewPlaying; }
    const bool wasHeld = m_bPreviewCaptureClockHeld;
    if (wasHeld) requestedMs = m_iPreviewCaptureResumeMs;
    if (!CPresentation_Manager::Get().Are_ScreenPostsEnabled())
    {
        m_bPreviewCaptureClockHeld = false;
        m_bPreviewCaptureBoundarySampled = false;
        m_bPreviewCaptureAllowed = true;
        m_iPreviewCaptureWaitFrames = 0u;
        effectiveMs = requestedMs;
        if (wasHeld) m_PreviewSession.lastClockMs = -1.f;
        return true;
    }
    std::optional<std::uint32_t> firstCaptureMs;
    struct CAPTURE_OWNER { const KOUKU_SAYDON_COMPOSITION_PATTERN* pattern; SESSION* session; std::uint32_t offsetMs; };
    std::vector<CAPTURE_OWNER> owners{{&m_PreviewPattern, &m_PreviewSession, 0u}};
    for (auto& member : m_BundlePreviewMembers)
        owners.push_back({&member.pattern, &member.session,
            static_cast<std::uint32_t>((std::uint64_t(member.offsetTicks) * 1000u + 29u) / 30u)});
    for (const auto& owner : owners)
    for (const auto& box : owner.pattern->PresentationOccurrences)
    {
        const auto boxStartMs = owner.offsetMs + box.iStartMs;
        if (requestedMs < boxStartMs ||
            double(requestedMs) >= double(boxStartMs) + box.iDurationMs) continue;
        const auto resource = std::find_if(m_PreviewDocument.PresentationResources.begin(),
            m_PreviewDocument.PresentationResources.end(), [&](const auto& value) {
                return value.strResourceId == box.strResourceId; });
        if (resource == m_PreviewDocument.PresentationResources.end() || resource->eKind != KIND::EFFECT ||
            (resource->strResourceKind != "V1_EFFECT" && resource->strResourceKind != "V1_ELEMENT")) continue;
        const auto document = CEffectCatalog::Find(resource->strAssetId);
        if (!document) continue; // Existing admission owns missing-document failures.
        for (const auto& element : document->Elements)
        {
            if (!element.bVisible || element.eKind != EFFECT_ELEMENT_KIND::SCREEN_POST ||
                !element.Detail.ScreenPost.bEnabled || element.Detail.ScreenPost.eStatus !=
                    EFFECT_PRESENTATION_RUNTIME_STATUS::RECONSTRUCTED_PROFILE || element.Detail.ScreenPost.eProfile !=
                    EFFECT_SCREEN_POST_PROFILE::SCENE_COLLAPSE_CAPTURE_V1 ||
                (!resource->strElementId.empty() && resource->strElementId != element.strElementId)) continue;
            const double effectRate = Effect_SourceClockRate(*resource, box);
            const double delayMs = double(element.Detail.Timing.fStartDelaySeconds) * 1000.0 / effectRate;
            if (!std::isfinite(delayMs) || delayMs < 0.0 || delayMs >= box.iDurationMs) continue;
            // Float seconds can put an authored integer millisecond a fraction
            // of a microsecond above itself; retain that cursor before rounding up.
            const auto captureMs = static_cast<std::uint32_t>(boxStartMs + std::ceil(delayMs - 0.001));
            const double captureEndMs = double(boxStartMs) + delayMs +
                double(element.Detail.Timing.fLifeTimeSeconds) * 1000.0 / effectRate;
            if (captureMs > requestedMs || double(requestedMs) >= captureEndMs) continue;
            const auto row = owner.session->rows.find(box.strOccurrenceId);
            if (row != owner.session->rows.end())
            {
                if (row->second.failed)
                {
                    m_strStatus = row->second.failureStatus;
                    return false;
                }
                const HRESULT captureResult = CEffectPresentationService::Get_ScreenPostCaptureResult(
                    {row->second.v1EffectHandle}, element.strElementId);
                if (FAILED(captureResult))
                {
                    m_strStatus = "Scene capture failed: " + box.strOccurrenceId + " / " +
                        element.strElementId + "; HRESULT=" + std::to_string(captureResult);
                    return false;
                }
                if (CEffectPresentationService::Has_CapturedScreenPost(
                    {row->second.v1EffectHandle}, element.strElementId)) continue;
            }
            if (!firstCaptureMs || captureMs < *firstCaptureMs) firstCaptureMs = captureMs;
        }
    }
    m_bPreviewCaptureClockHeld = firstCaptureMs.has_value();
    if (firstCaptureMs)
    {
        if (!wasHeld) m_iPreviewCaptureResumeMs = requestedMs;
        const bool sameBoundary = wasHeld && m_iPreviewCaptureBoundaryMs == *firstCaptureMs;
        m_bPreviewCaptureAllowed = sameBoundary && m_bPreviewCaptureBoundarySampled;
        if (!sameBoundary)
        {
            m_bPreviewCaptureBoundarySampled = false;
            m_iPreviewCaptureWaitFrames = 0u;
        }
        m_iPreviewCaptureBoundaryMs = *firstCaptureMs;
        effectiveMs = *firstCaptureMs;
        if (++m_iPreviewCaptureWaitFrames > 120u)
        {
            m_strStatus = "Scene capture did not receive a renderable frame at " +
                std::to_string(effectiveMs) + " ms within 120 preview updates. "
                "Check the active WORLD and Screen Presentation Post, then restart Preview.";
            return false;
        }
    }
    else
    {
        effectiveMs = requestedMs;
        m_bPreviewCaptureBoundarySampled = false;
        m_bPreviewCaptureAllowed = true;
        m_iPreviewCaptureWaitFrames = 0u;
    }
    if (wasHeld || m_bPreviewCaptureClockHeld)
    {
        // A render boundary is one continuing occurrence, not a new playback.
        // Keep its capture and observed anchors across the deferred cursor jump.
        m_PreviewSession.lastClockMs = -1.f;
        for (auto& member : m_BundlePreviewMembers) member.session.lastClockMs = -1.f;
        m_strCompletedPreviewPatternId.clear();
    }
    return true;
}

void Client::CKoukuSaydonPresentationPlayer::Sample_Preview(std::uint32_t clockMs,
    bool playing, bool paused, const float4x4_t& pivot, const std::shared_ptr<Engine::CModel>& model)
{
    if (!playing) { Stop_Preview(); return; }
    if (!m_bPreviewPlaying) return;
    Set_PreviewPivot(pivot, model);
    // MainApp returns the displayed integer cursor every frame. Writing it
    // into our accumulated clock would discard sub-millisecond time forever.
    if (!m_bOwnPreviewClock || m_bPreviewCaptureClockHeld || clockMs != Preview_ClockMs())
        m_fPreviewClockMs = (std::min)(clockMs, m_iPreviewDurationMs);
    m_bPreviewPaused = paused;
    ANIMATION_MODEL_TARGET_VIEW weaponView;
    const bool hasWeapon = CAnimationTargetService::Resolve_Model() == model &&
        CAnimationTargetService::Resolve_ModelTarget(ANIMATION_BONE_TARGET::WEAPON, weaponView);
    Sample(m_PreviewSession, m_PreviewDocument, m_PreviewPattern, float(m_fPreviewClockMs),
        paused, pivot, model, hasWeapon ? &weaponView : nullptr);
    // Resolve_PreviewCaptureClock checks the actual active capture occurrence.
    // Unrelated failed rows remain isolated, just as they do during ordinary
    // playback; a later scene capture must not turn them into a sequence stop.
    Refresh_SharedPresentation();
    // The next Engine Late_Update can now cull and submit this WORLD/camera.
    // The first boundary render is pass-through; only the next may latch it.
    if (m_bPreviewCaptureClockHeld && clockMs == m_iPreviewCaptureBoundaryMs)
        m_bPreviewCaptureBoundarySampled = true;
}

void Client::CKoukuSaydonPresentationPlayer::Pause_Preview(bool paused)
{
    m_strCompletedPreviewPatternId.clear();
    m_bPreviewPaused = paused;
    if (!paused && m_bPreviewPlaying && m_iPreviewDurationMs && m_fPreviewClockMs >= m_iPreviewDurationMs)
        Seek_Preview(0u);
    for (const auto& member : m_BundlePreviewMembers)
        for (const auto& [id, row] : member.session.rows)
        {
            if (row.effectHandle) CEffectV2Runtime::Set_GroupPaused(row.effectHandle, paused);
            if (row.soundHandle) CGameInstance::Get().Pause_SoundCue(row.soundHandle, paused);
        }
    for (const auto& [id, row] : m_PreviewSession.rows)
    {
        if (row.effectHandle) CEffectV2Runtime::Set_GroupPaused(row.effectHandle, paused);
        if (row.soundHandle) CGameInstance::Get().Pause_SoundCue(row.soundHandle, paused);
    }
}

void Client::CKoukuSaydonPresentationPlayer::Seek_Preview(std::uint32_t clockMs)
{
    m_bPreviewCaptureClockHeld = false;
    m_bPreviewCaptureBoundarySampled = false;
    m_bPreviewCaptureAllowed = true;
    m_iPreviewCaptureWaitFrames = 0u;
    for (const auto& [id, row] : m_PreviewSession.rows)
        CEffectPresentationService::Set_ScreenPostCaptureAllowed({row.v1EffectHandle}, true);
    m_strCompletedPreviewPatternId.clear();
    m_fPreviewClockMs = (std::min)(clockMs, m_iPreviewDurationMs);
    if (m_fPreviewClockMs >= m_iPreviewDurationMs) Pause_Preview(true);
    // Stop/paused scrubbing must retain the observed birth history and a
    // frozen Effect's original anchor. The V2 external clock handles rewind.
    const auto prepare = [&](SESSION& session) {
        session.lastClockMs = -1.f;
        for (auto row = session.rows.begin(); row != session.rows.end();)
        {
            if (row->second.kind == KIND::EFFECT && !row->second.failed)
            { ++row; continue; }
            if (row->second.effectHandle) CEffectV2Runtime::Stop_Group(row->second.effectHandle);
        if (row->second.v1EffectHandle) CEffectPresentationService::Stop_WorldRoot({row->second.v1EffectHandle});
            if (row->second.soundHandle) CGameInstance::Get().Stop_SoundCue(row->second.soundHandle);
            row = session.rows.erase(row);
        }
    };
    prepare(m_PreviewSession);
    for (auto& member : m_BundlePreviewMembers)
    {
        prepare(member.session);
        if (member.actor) member.actor->Reset_AfterimageHistory();
    }
    if (Preview_IsBundle()) Sample_BundlePreview();
    // MainApp samples single-pattern WORLD at the new clock before recreating these cue handles.
    Refresh_SharedPresentation();
}

void Client::CKoukuSaydonPresentationPlayer::Fail_Preview(std::string status)
{
    const std::string patternId = Preview_IsBundle() ? m_PreviewBundleId : m_PreviewPattern.strPatternId;
    Stop_Preview();
    m_strFailedPreviewPatternId = patternId;
    m_strFailedPreviewStatus = std::move(status);
    m_strStatus = m_strFailedPreviewStatus;
}

bool Client::CKoukuSaydonPresentationPlayer::Consume_FailedPreview(std::string& patternId, std::string& status)
{
    if (m_strFailedPreviewPatternId.empty()) return false;
    patternId = std::move(m_strFailedPreviewPatternId);
    status = std::move(m_strFailedPreviewStatus);
    m_strFailedPreviewPatternId.clear();
    m_strFailedPreviewStatus.clear();
    return true;
}

bool Client::CKoukuSaydonPresentationPlayer::Consume_CompletedPreview(std::string& patternId)
{
    if (m_strCompletedPreviewPatternId.empty()) return false;
    patternId = std::move(m_strCompletedPreviewPatternId);
    Stop_Preview();
    return true;
}

void Client::CKoukuSaydonPresentationPlayer::Stop_Preview()
{
    m_bServerSequenceClock = false;
    Clear_LogicPreview();
#ifdef _DEBUG
    if (m_bBundleWorldExternal)
        if (auto* level = CLevel_KakulSaydonArena::Get_Active()) level->Debug_StopCompositionWorldPreview();
#endif
    m_bBundleWorldExternal = false;
    m_strCompletedPreviewPatternId.clear();
    m_strFailedPreviewPatternId.clear();
    m_strFailedPreviewStatus.clear();
    ++m_iPreviewGeneration;
    m_bModelReferencePreview = false;
    Release_BundlePreviewMembers(m_BundlePreviewMembers);
    m_PreviewBundleId.clear();
    Stop_Session(m_PreviewSession);
    m_bPreviewPlaying = false;
    m_bOwnPreviewClock = false;
    m_bPreviewClockAwaitingFirstUpdate = false;
    m_bPreviewPreparationQueued = false;
    m_PreviewPreparationTargets.clear();
    m_bPreviewCaptureClockHeld = false;
    m_bPreviewCaptureBoundarySampled = false;
    m_bPreviewCaptureAllowed = true;
    m_iPreviewCaptureResumeMs = m_iPreviewCaptureBoundaryMs = m_iPreviewCaptureWaitFrames = 0u;
    m_bPreviewPaused = false;
    m_bPreviewPivotReady = false;
    m_bColliderResourcePreview = false;
    m_PreviewModel.reset();
    Refresh_SharedPresentation();
}

void Client::CKoukuSaydonPresentationPlayer::Sync_MazeMark(
    CARD& mark, const std::string& asset, const float4x4_t& pivot)
{
    if (!IsCardMazeMarkGroup(asset))
    {
        // Bingo and other floor marks keep the original single-attempt path.
        if (mark.assetId != asset)
        {
            if (mark.handle) CEffectV2Runtime::Stop_Group(mark.handle);
            mark = {}; mark.assetId = asset;
            std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> effects;
            if (Ensure_EffectResource("GROUP", asset, effects))
                if (const auto* group = effects->Find_Group(asset))
                {
                    EFFECT_V2_GROUP_PLAYBACK_DESC playback;
                    playback.PivotWorld = pivot;
                    playback.fDurationSeconds = 0.f;
                    playback.bProductOwned = true;
                    mark.handle = CEffectV2Runtime::Play_Group(*group, effects, playback, m_Device, m_Context);
                }
            if (!mark.handle) m_strStatus = "Maze floor mark unavailable: " + asset;
        }
        if (mark.handle)
        {
            CEffectV2Runtime::Set_GroupPivot(mark.handle, pivot);
            std::string failure;
            if (CEffectV2Runtime::Consume_GroupFailure(mark.handle, failure))
            {
                CEffectV2Runtime::Stop_Group(mark.handle); mark.handle = 0u;
                m_strStatus = "Maze floor mark failed: " + asset + ": " + failure;
            }
        }
        return;
    }
    if (mark.assetId != asset)
    {
        if (mark.handle) CEffectV2Runtime::Stop_Group(mark.handle);
        mark = {};
        mark.assetId = asset;
    }
    const auto generation = CEffectV2Runtime::Cache_Generation();
    const auto nowMs = static_cast<std::uint64_t>(std::chrono::duration_cast<
        std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
    mark.mazeRetry.ObserveGeneration(generation);
    if (mark.handle)
    {
        CEffectV2Runtime::Set_GroupPivot(mark.handle, pivot);
        std::string failure;
        if (!CEffectV2Runtime::Consume_GroupFailure(mark.handle, failure)) return;
        CEffectV2Runtime::Stop_Group(mark.handle);
        mark.handle = 0u;
        mark.mazeRetry.Defer(nowMs);
        m_strStatus = "Maze floor mark failed: " + asset + ": " + failure;
        return;
    }
    if (!mark.mazeRetry.TryBegin(nowMs)) return;
    const std::string resourceKey = "GROUP:" + asset;
    if (mark.mazeRetry.attempts > 1u)
    {
        // Only this resource is read again, at most twice per mark occurrence/generation.
        m_EffectResourceFailures.erase(resourceKey);
        m_EffectResources.erase(resourceKey);
    }
    std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> effects;
    if (!Ensure_EffectResource("GROUP", asset, effects))
    {
        const std::string reason = m_strStatus;
        m_strStatus = "Maze floor mark unavailable: " + asset + ": " + reason;
        return;
    }
    const auto* group = effects ? effects->Find_Group(asset) : nullptr;
    if (!group)
    {
        m_strStatus = "Maze floor mark group missing from snapshot: " + asset;
        return;
    }
    EFFECT_V2_GROUP_PLAYBACK_DESC playback;
    playback.PivotWorld = pivot;
    playback.fDurationSeconds = 0.f;
    playback.bProductOwned = true;
    mark.handle = CEffectV2Runtime::Play_Group(*group, effects, playback, m_Device, m_Context);
    // A handle may fail on the next frame; do not reset the attempt budget here.
    if (!mark.handle)
        m_strStatus = "Maze floor mark spawn failed: " + asset + ": " + CEffectV2Runtime::Last_Error();
}

void Client::CKoukuSaydonPresentationPlayer::Update_MazeMarks(
    const std::vector<KOUKU_CARD_PRESENTATION_VIEW>& players)
{
    using namespace LostArk::Shared;
    const auto suitName = [](MECHANIC_CARD_SYMBOL suit) -> const char*
    {
        switch (suit)
        {
        case MECHANIC_CARD_SYMBOL::HEART: return "heart";
        case MECHANIC_CARD_SYMBOL::SPADE: return "spade";
        case MECHANIC_CARD_SYMBOL::CLUB: return "club";
        case MECHANIC_CARD_SYMBOL::DIAMOND: return "diamond";
        default: return nullptr;
        }
    };
    const auto groundPivot = [](const float4x4_t& world)
    {
        // Translation only: character scale/yaw must not resize or rotate the symbol.
        float4x4_t pivot;
        XMStoreFloat4x4(&pivot, XMMatrixTranslation(world._41, world._42 + .02f, world._43));
        return pivot;
    };
    std::set<std::uint32_t> livePlayers, liveTargets, liveExits;
    bool mazeActive = false;
    for (const auto& view : players)
    {
        const auto& s = view.Snapshot;
        if (!s.iCurrentHp || s.eCardMazeRole == CARD_MAZE_ROLE::NONE || (s.CardMaze.flags & 8u)) continue;
        mazeActive = true;
        // The telescope owner never carries a suit marker, even with an old Debug snapshot.
        if (s.eCardMazeRole != CARD_MAZE_ROLE::HUNTER) continue;
        const char* suit = suitName(s.eCardMazeSuit);
        if (!suit) continue;
        if (!(s.CardMaze.flags & 2u) && !s.CardMaze.transferStartTick)
            if (auto character = view.pCharacter.lock(); character && character->Get_Transform())
            {
                livePlayers.insert(s.iNetEntityId);
                Sync_MazeMark(m_MazePlayerMarks[s.iNetEntityId], std::string("cardmaze.mark.") + suit,
                    groundPivot(*character->Get_Transform()->Get_WorldMatrixPtr()));
            }
        if ((s.CardMaze.flags & 4u) && !s.CardMaze.transferStartTick)
        {
            liveExits.insert(s.iNetEntityId);
            float4x4_t pivot;
            XMStoreFloat4x4(&pivot, XMMatrixTranslation(s.CardMaze.exitX, s.CardMaze.exitY + .02f, s.CardMaze.exitZ));
            Sync_MazeMark(m_MazeExits[s.iNetEntityId], std::string("cardmaze.exit.") + suit, pivot);
        }
    }
    if (const auto* arena = CLevel_KakulSaydonArena::Get_Active(); mazeActive && arena)
    {
        std::vector<KOUKU_MAZE_TARGET_VIEW> targets;
        arena->Collect_KoukuMazeTargets(targets);
        for (const auto& target : targets)
        {
            const char* suit = nullptr;
            if (target.archetypeId == "MONSTER_KOUKU_CARD_HEART") suit = "heart";
            else if (target.archetypeId == "MONSTER_KOUKU_CARD_SPADE") suit = "spade";
            else if (target.archetypeId == "MONSTER_KOUKU_CARD_CLUB") suit = "club";
            else if (target.archetypeId == "MONSTER_KOUKU_CARD_DIAMOND") suit = "diamond";
            const auto npc = target.npc.lock();
            if (!suit || !npc || !npc->Get_Transform()) continue;
            liveTargets.insert(target.entityId);
            Sync_MazeMark(m_MazeTargetMarks[target.entityId], std::string("cardmaze.mark.") + suit,
                groundPivot(*npc->Get_Transform()->Get_WorldMatrixPtr()));
        }
    }
    const auto removeStale = [](auto& marks, const auto& live)
    {
        for (auto i = marks.begin(); i != marks.end();)
        {
            if (live.contains(i->first)) { ++i; continue; }
            if (i->second.handle) CEffectV2Runtime::Stop_Group(i->second.handle);
            i = marks.erase(i);
        }
    };
    /* The bingo bomb's mark. It rides one named carrier and the Server can
       cancel it mid-flight, so it stays a followed state rather than a
       timeline. Height and size live in the authored document, so the pivot
       here is only the ground point under its carrier. The planted half is a
       World Sequence the Server names the moment it plants. */
    {
        const auto& bombBoard = CCombatHUDViewModel::Get().Get_BingoBoard();
        std::set<std::int32_t> liveBombs;
        for (std::uint8_t index = 0u; index < bombBoard.iBombCount; ++index)
        {
            const auto& bomb = bombBoard.Bombs[index];
            const char* bombAsset = nullptr;
            float4x4_t bombPivot;
            if (LostArk::Shared::BINGO_BOMB_PHASE::MARKED == bomb.ePhase)
            {
                std::shared_ptr<CCharacter> carrier;
                for (const auto& view : players)
                    if (view.Snapshot.iNetEntityId == bomb.iCarrierNetEntityId)
                    { carrier = view.pCharacter.lock(); break; }
                if (!carrier || !carrier->Get_Transform()) continue;
                const float4x4_t& carrierWorld =
                    *carrier->Get_Transform()->Get_WorldMatrixPtr();
                XMStoreFloat4x4(&bombPivot, XMMatrixTranslation(
                    carrierWorld._41, carrierWorld._42, carrierWorld._43));
                bombAsset = "bingo.bomb.mark";
            }
            /* PLANTED falls through: dropping the slot out of liveBombs is
               what takes the mark off the carrier, and the sequence has already
               started where it stood. */
            else continue;
            const std::int32_t slot = static_cast<std::int32_t>(index);
            liveBombs.insert(slot);
            Sync_MazeMark(m_BingoBombs[slot], bombAsset, bombPivot);
        }
        for (auto i = m_BingoBombs.begin(); i != m_BingoBombs.end();)
        {
            if (liveBombs.contains(i->first)) { ++i; continue; }
            if (i->second.handle) CEffectV2Runtime::Stop_Group(i->second.handle);
            i = m_BingoBombs.erase(i);
        }
    }
    /* The bingo hammer is authored now: four World Sequence templates, one
       per sweep direction, and one instance per anchor. The Server names the
       instance when it rolls the anchor, so there is nothing to pose here. */
    removeStale(m_MazePlayerMarks, livePlayers);
    removeStale(m_MazeTargetMarks, liveTargets);
    removeStale(m_MazeExits, liveExits);
}

void Client::CKoukuSaydonPresentationPlayer::Update_BingoMarks(float dt)
{
    const auto& board = CCombatHUDViewModel::Get().Get_BingoBoard();
    auto* arena = CLevel_KakulSaydonArena::Get_Active();
    if (!arena || board.iWhiteMask == 0u)
    {
        m_BingoMarks.clear();
        return;
    }
    auto targets = arena->Get_CompositionWorldTargets();
    const auto& source = arena->Get_WorldSequenceDocument();
    static constexpr const char* objectId = "world.object.kouku.bingo_skull";
    static constexpr const char* flipId = "world.object.instance.kouku.bingo_skull.flip";
    static constexpr const char* whiteId = "world.object.instance.kouku.bingo_skull.white";
    static constexpr const char* redId = "world.object.instance.kouku.bingo_skull.red";
    static constexpr const char* redFlipId = "world.object.instance.kouku.bingo_skull.red_flip";
    if (!m_BingoWorldDocument || m_BingoWorldDocument->Get_Revision() != source.Get_Revision())
    {
        // Preserve source revision so the Level's admitted CModel pool remains shared.
        // A small subset avoids copying unrelated cinematic documents for 25 cells.
        auto staged = std::make_shared<CWorldSequenceDocument>(source);
        std::erase_if(staged->Get_ObjectResources(), [](const auto& row) { return row.objectId != objectId; });
        std::erase_if(staged->Get_Instances(), [](const auto& row)
            { return row.instanceId != flipId && row.instanceId != whiteId && row.instanceId != redId && row.instanceId != redFlipId; });
        std::set<std::string> templates;
        for (const auto& row : staged->Get_Instances()) templates.insert(row.templateId);
        std::erase_if(staged->Get_Templates(), [&](const auto& row) { return !templates.contains(row.sequenceId); });
        WORLD_SEQUENCE_PLACEMENT_MAP placements;
        WORLD_SEQUENCE_DEPLOY_MAP deploy;
        std::string error;
        if (!staged->Find_ObjectResource(objectId) || !staged->Find_Instance(flipId) ||
            !staged->Find_Instance(whiteId) || !staged->Find_Instance(redId) || !staged->Find_Instance(redFlipId) ||
            !staged->Validate(placements, deploy, error))
        {
            m_strStatus = "Bingo skull Object motions unavailable: " + error;
            return;
        }
        m_BingoWorldDocument = std::move(staged);
    }
    const auto generation = CEffectV2Runtime::Cache_Generation() ^
        (static_cast<std::uint64_t>(source.Get_Revision()) << 32u);
    const auto nowMs = static_cast<std::uint64_t>(std::chrono::duration_cast<
        std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
    for (std::int32_t cell = 0; cell < LostArk::Shared::KOUKU_BINGO_CELL_COUNT; ++cell)
    {
        const auto bit = 1u << cell;
        if (!(board.iWhiteMask & bit)) continue;
        const bool red = (board.iRedMask & bit) != 0u;
        auto& mark = m_BingoMarks[cell];
        mark.retry.ObserveGeneration(generation);
        if (mark.player)
        {
            // A failed colour change keeps the previous cell animated while retry waits.
            mark.player->Update((std::max)(0.f, dt), targets);
            if (!mark.player->Is_Playing(mark.instanceId))
            {
                m_strStatus = "Bingo skull Object playback failed: " + mark.player->Get_Status();
                mark.player.reset(); mark.instanceId.clear(); mark.retry.Defer(nowMs);
                continue;
            }
            if (mark.red == red) continue;
        }
        if (!mark.retry.TryBegin(nowMs)) continue;
        // A newly painted white cell plays the native rotating tile, then its
        // saved NEXT maintenance; an ordinary-to-red transition replays that native flip.
        const std::string motion = red ? (mark.player && !mark.red ? redFlipId : redId) : (mark.player ? whiteId : flipId);
        CWorldSequencePlayer::OBJECT_PLACEMENT placement;
        placement.position = { LostArk::Shared::Kouku_BingoCellCenterX(cell), .02f,
            LostArk::Shared::Kouku_BingoCellCenterZ(cell) };
        if (mark.player && mark.player->Get_Document().Get_Revision() == m_BingoWorldDocument->Get_Revision())
        {
            // Reuse the admitted document/models. Stage a distinct instance so a
            // failed first sample cannot erase the previous colour. Applying a
            // motion in place would retain its 300-second maintenance effect tail.
            if (!mark.player->Play(motion, targets, 1.f, {}, 0u, placement) ||
                !mark.player->Seek_InstanceToMs(motion, 0.f, targets))
            {
                m_strStatus = "Bingo skull Object change failed: " + mark.player->Get_Status();
                mark.player->Stop_Instance(motion, targets, true);
                mark.retry.Defer(nowMs);
                continue;
            }
            mark.player->Stop_Instance(mark.instanceId, targets, true);
            mark.instanceId = motion; mark.red = red; mark.retry = {};
            continue;
        }
        auto staged = std::make_unique<CWorldSequencePlayer>();
        std::string error;
        if (!staged->Set_Document(*m_BingoWorldDocument, targets, error) ||
            !staged->Play(motion, targets, 1.f, {}, 0u, placement) ||
            !staged->Seek_InstanceToMs(motion, 0.f, targets))
        {
            m_strStatus = "Bingo skull Object start failed: " + (error.empty() ? staged->Get_Status() : error);
            mark.retry.Defer(nowMs);
            continue;
        }
        // Commit the replacement only after the new native effect starts.
        mark.player = std::move(staged); mark.instanceId = motion; mark.red = red;
        mark.retry = {};
    }
    for (auto it = m_BingoMarks.begin(); it != m_BingoMarks.end();)
    {
        if (board.iWhiteMask & (1u << it->first)) { ++it; continue; }
        it = m_BingoMarks.erase(it); // Existing WorldSequence destructor releases all tails/clones.
    }
}

void Client::CKoukuSaydonPresentationPlayer::Reset()
{
    for (auto& [id, stage] : m_ExternalStageEnvironments) Stop_Session(stage.session);
    m_ExternalStageEnvironments.clear();
    for (const auto& [id, owner] : m_CounterAfterimageOwners)
        if (const auto npc = owner.lock()) npc->Set_CounterAfterimageEnabled(false);
    m_CounterAfterimageOwners.clear();
    m_LightPlayerPivots.clear();
    m_LightBossFollowers.clear();
    m_iProductReloadRunEpoch = 0u;
    for (auto& [id, session] : m_TargetedCombatSessions)
    { Stop_Session(session.playback); Stop_Session(session.sourceBossPlayback); }
    m_TargetedCombatSessions.clear();
    m_ContactCombatEffects.clear();
    m_TargetedCombatVisuals.clear();
    Stop_Session(m_FearSession);
    m_FearSession.key.clear();
    m_strCompletedFearKey.clear();
    m_QueuedV1Effects.clear();
    for (auto& [id, session] : m_BossSessions) Stop_Session(session);
    for (auto& [id, session] : m_ChildBossSessions) Stop_Session(session);
    m_BossSessions.clear();
    Clear_ProductTails();
    m_CancelledBossSessionKeys.clear();
    m_CancelledChildSessionKeys.clear();
    m_ChildBossSessions.clear();
    for (auto& [id, session] : m_MarioEntrySessions) Stop_Session(session);
    m_MarioEntrySessions.clear();
    Stop_Session(m_ProductBundleSession);
    for (const auto& [id, card] : m_Cards)
        if (card.handle) CEffectV2Runtime::Stop_Group(card.handle);
    m_Cards.clear();
    for (auto& [id, visual] : m_DiceBindVisuals) Stop_Session(visual.session);
    m_DiceBindVisuals.clear();
    m_BingoMarks.clear();
    m_BingoWorldDocument.reset();
    for (const auto& [slot, bomb] : m_BingoBombs)
        if (bomb.handle) CEffectV2Runtime::Stop_Group(bomb.handle);
    m_BingoBombs.clear();
    for (const auto& [id, exit] : m_MazeExits)
        if (exit.handle) CEffectV2Runtime::Stop_Group(exit.handle);
    m_MazeExits.clear();
    for (auto* marks : { &m_MazePlayerMarks, &m_MazeTargetMarks })
    {
        for (const auto& [id, mark] : *marks)
            if (mark.handle) CEffectV2Runtime::Stop_Group(mark.handle);
        marks->clear();
    }
    m_ColliderDebugOverrides.clear();
    Stop_Preview();
    Restore_Scene();
}

bool Client::CKoukuSaydonPresentationPlayer::Preview_HasActiveWorldBox(const std::string_view occurrenceId) const
{
#ifdef _DEBUG
    if (occurrenceId.empty() || !m_bPreviewPlaying || m_bModelReferencePreview) return false;
    const auto contains = [&](const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, const SESSION& session,
        const double clockMs, const bool bundle)
    {
        const auto box = std::find_if(pattern.WorldOccurrences.begin(), pattern.WorldOccurrences.end(),
            [&](const auto& value) {
                return value.strOccurrenceId == occurrenceId && clockMs >= value.iStartMs &&
                    clockMs < double(value.iStartMs) + value.iDurationMs;
            });
        if (box == pattern.WorldOccurrences.end()) return false;
        if (!bundle)
        {
            const auto* level = CLevel_KakulSaydonArena::Get_Active();
            return level && level->Debug_HasVisibleCompositionWorldBox(occurrenceId);
        }
        const auto world = std::find_if(m_PreviewDocument.Worlds.begin(), m_PreviewDocument.Worlds.end(),
            [&](const auto& value) { return value.strWorldId == box->strWorldId; });
        const auto player = session.previewWorlds.find(box->strOccurrenceId);
        float4x4_t pivot;
        return world != m_PreviewDocument.Worlds.end() && player != session.previewWorlds.end() &&
            player->second->Try_GetObjectPivot(world->strSequenceInstanceId, pivot);
    };
    if (contains(m_PreviewPattern, m_PreviewSession, m_fPreviewClockMs, false)) return true;
    for (const auto& member : m_BundlePreviewMembers)
        if (contains(member.pattern, member.session,
            m_fPreviewClockMs - double(member.offsetTicks) * 1000.0 / 30.0, true)) return true;
#endif
    return false;
}

void Client::CKoukuSaydonPresentationPlayer::Refresh_WorldPlacementAuthoring(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document)
{
#ifdef _DEBUG
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level || !m_bPreviewPlaying || m_bModelReferencePreview) return;
    const auto targets = level->Get_CompositionWorldTargets();
    const auto refresh = [&](KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, SESSION& session, const bool bundle)
    {
        const auto source = std::find_if(document.Patterns.begin(), document.Patterns.end(),
            [&](const auto& value) { return value.strPatternId == pattern.strPatternId; });
        const bool placementPreview = pattern.strPatternId == "preview.kouku.resource";
        if (!placementPreview && (source == document.Patterns.end() || !source->strLoadError.empty())) return false;
        bool changed = false;
        for (auto& box : pattern.WorldOccurrences)
        {
            // Synthetic placement previews still own the authored stable occurrence IDs.
            const auto owner = placementPreview ? std::find_if(document.Patterns.begin(), document.Patterns.end(),
                [&](const auto& value) {
                    return value.strLoadError.empty() && std::any_of(value.WorldOccurrences.begin(), value.WorldOccurrences.end(),
                        [&](const auto& row) { return row.strOccurrenceId == box.strOccurrenceId; });
                }) : source;
            if (owner == document.Patterns.end()) continue;
            const auto edited = std::find_if(owner->WorldOccurrences.begin(), owner->WorldOccurrences.end(),
                [&](const auto& value) { return value.strOccurrenceId == box.strOccurrenceId && value.strWorldId == box.strWorldId; });
            if (edited == owner->WorldOccurrences.end() || box.Placement == edited->Placement) continue;
            const auto placement = WorldPlacementFromOccurrence(*edited);
            std::string status;
            bool applied = false;
            if (bundle)
            {
                const auto world = std::find_if(m_PreviewDocument.Worlds.begin(), m_PreviewDocument.Worlds.end(),
                    [&](const auto& value) { return value.strWorldId == box.strWorldId; });
                const auto player = session.previewWorlds.find(box.strOccurrenceId);
                if (world != m_PreviewDocument.Worlds.end() && player != session.previewWorlds.end())
                {
                    applied = player->second->Validate_ObjectPlacement(world->strSequenceInstanceId, placement, status);
                    if (applied && player->second->Is_Playing(world->strSequenceInstanceId))
                    {
                        applied = player->second->Set_ObjectPlacement(world->strSequenceInstanceId, placement, targets);
                        if (!applied) status = player->second->Get_Status();
                    }
                }
            }
            else applied = level->Debug_SetCompositionWorldPlacement(box.strOccurrenceId, placement, status);
            if (!applied)
            {
                m_strStatus = "WORLD placement preview kept its previous pose: " + box.strOccurrenceId + "; " + status;
                continue;
            }
            box.Placement = edited->Placement;
            changed = true;
        }
        return changed;
    };
    (void)refresh(m_PreviewPattern, m_PreviewSession, false);
    bool bundleChanged = false;
    for (auto& member : m_BundlePreviewMembers)
        bundleChanged |= refresh(member.pattern, member.session, true);
    if (bundleChanged) Sample_BundlePreview();
#endif
}

void Client::CKoukuSaydonPresentationPlayer::Refresh_ColliderAuthoring(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document, std::uint64_t generation)
{
    if (m_iColliderAuthoringGeneration == generation) return;
    m_iColliderAuthoringGeneration = generation;
    m_ColliderDebugOverrides.clear();
    for (const auto& pattern : document.Patterns)
        for (const auto& box : pattern.PresentationOccurrences)
            m_ColliderDebugOverrides.emplace(box.strOccurrenceId, box.bDebugRender);
    if (!m_bPreviewPlaying) return;
    Refresh_WorldPlacementAuthoring(document);
    bool changed = false;
    for (auto& box : m_PreviewPattern.PresentationOccurrences)
    {
        const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
            [&box](const auto& value) { return value.strResourceId == box.strResourceId && value.eKind == KIND::COLLIDER; });
        if (resource == document.PresentationResources.end()) continue;
        const auto old = std::find_if(m_PreviewDocument.PresentationResources.begin(), m_PreviewDocument.PresentationResources.end(),
            [&box](const auto& value) { return value.strResourceId == box.strResourceId; });
        if (old != m_PreviewDocument.PresentationResources.end() && *old != *resource)
        { *old = *resource; changed = true; }
        for (const auto& pattern : document.Patterns)
        {
            const auto source = std::find_if(pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(),
                [&box](const auto& value) { return value.strOccurrenceId == box.strOccurrenceId; });
            if (source == pattern.PresentationOccurrences.end()) continue;
            auto staged = *source;
            if (m_bColliderResourcePreview) staged.iStartMs = 0u;
            if (box != staged) { box = std::move(staged); changed = true; }
            break;
        }
    }
    if (changed)
    {
        m_PreviewDocument.Worlds = document.Worlds;
        if (m_bColliderResourcePreview && !m_PreviewPattern.Stages.empty())
            m_PreviewPattern.Stages.front().iDurationMs = m_PreviewPattern.PresentationOccurrences.front().iDurationMs;
        m_iPreviewDurationMs = Pattern_Duration(m_PreviewPattern);
        m_fPreviewClockMs = (std::min)(m_fPreviewClockMs, double(m_iPreviewDurationMs - 1u));
        Stop_Session(m_PreviewSession);
    }
}

bool Client::CKoukuSaydonPresentationPlayer::Preview_SummonPlacement(
    const std::string& patternId, const KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE& occurrence)
{
    if (!m_bPreviewPlaying || m_bModelReferencePreview || occurrence.PatternSpawns.empty()) return false;
    struct PLACEMENT final
    {
        BUNDLE_PREVIEW_MEMBER* member = nullptr;
        float3_t position{};
        float yaw = 0.f;
    };
    std::vector<PLACEMENT> staged;
    std::vector<KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE*> owners;
    for (auto& owner : m_BundlePreviewMembers)
    {
        if (owner.finiteActorLifetime || owner.pattern.strPatternId != patternId) continue;
        const auto box = std::find_if(owner.pattern.SummonOccurrences.begin(), owner.pattern.SummonOccurrences.end(),
            [&](const auto& row) { return row.strOccurrenceId == occurrence.strOccurrenceId; });
        if (box == owner.pattern.SummonOccurrences.end() || box->iStartMs != occurrence.iStartMs ||
            box->iDurationMs != occurrence.iDurationMs || box->PatternSpawns.size() != occurrence.PatternSpawns.size()) return false;
        float3_t ownerPosition{};
        float ownerYaw = 0.f;
        const bool needsOwner = std::any_of(occurrence.PatternSpawns.begin(), occurrence.PatternSpawns.end(),
            [](const auto& row) { return row.strAnchorKind == "BOSS"; });
        if (needsOwner && !Sample_BundlePreviewPose(owner, occurrence.iStartMs, ownerPosition, ownerYaw, false)) return false;
        for (const auto& spawn : occurrence.PatternSpawns)
        {
            const auto original = std::find_if(box->PatternSpawns.begin(), box->PatternSpawns.end(),
                [&](const auto& row) { return row.strSpawnId == spawn.strSpawnId && row.strPatternId == spawn.strPatternId; });
            if (original == box->PatternSpawns.end() || (spawn.strAnchorKind != "MAP" && spawn.strAnchorKind != "BOSS") ||
                !std::isfinite(spawn.fYawOffsetDegrees) || std::abs(spawn.fYawOffsetDegrees) > 360.0 ||
                !std::all_of(spawn.PositionOffset.begin(), spawn.PositionOffset.end(), [](const double value) {
                    return std::isfinite(value) && std::abs(value) <= 100000.0;
                })) return false;
            if (original->strAnchorKind == spawn.strAnchorKind && original->PositionOffset == spawn.PositionOffset &&
                original->fYawOffsetDegrees == spawn.fYawOffsetDegrees) continue;
            const auto memberId = owner.memberId + ":" + occurrence.strOccurrenceId + ":" + spawn.strSpawnId;
            const auto clone = std::find_if(m_BundlePreviewMembers.begin(), m_BundlePreviewMembers.end(),
                [&](const auto& row) { return row.finiteActorLifetime && row.memberId == memberId && row.actor; });
            if (clone == m_BundlePreviewMembers.end()) return false;
            float3_t position{float(spawn.PositionOffset[0]), float(spawn.PositionOffset[1]), float(spawn.PositionOffset[2])};
            float yaw = float(spawn.fYawOffsetDegrees);
            if (spawn.strAnchorKind == "BOSS")
            {
                const auto rotated = DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&position),
                    DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(ownerYaw)));
                DirectX::XMStoreFloat3(&position, rotated);
                position.x += ownerPosition.x; position.y += ownerPosition.y; position.z += ownerPosition.z;
                yaw += ownerYaw;
            }
            staged.push_back({&*clone, position, yaw});
        }
        owners.push_back(&*box);
    }
    if (owners.empty()) return false;
    // Validate all identities first. A placement edit retains the actor/model and
    // clock, invalidating only this actor's old pivot history and captured yaw.
    for (auto& change : staged)
    {
        auto& member = *change.member;
        member.initialPosition = change.position; member.initialYawDegrees = change.yaw;
        member.stageFacingYawDegrees.clear();
        member.airborneSelections.clear(); member.airborneAppearancePositions.clear();
        member.facingCheckpoints.clear();
        member.spatialPoseSamples.clear();
        Stop_Session(member.session);
    }
    for (auto* box : owners) box->PatternSpawns = occurrence.PatternSpawns;
    for (auto& pattern : m_PreviewDocument.Patterns)
        if (pattern.strPatternId == patternId)
            for (auto& box : pattern.SummonOccurrences)
                if (box.strOccurrenceId == occurrence.strOccurrenceId) box.PatternSpawns = occurrence.PatternSpawns;
    return true;
}

bool Client::CKoukuSaydonPresentationPlayer::Preview_PresentationGeometry(
    const std::string& patternId, const OCCURRENCE& occurrence)
{
    if (!m_bPreviewPlaying || m_bModelReferencePreview) return false;
    if (m_bColliderResourcePreview)
    {
        const auto owner = std::find_if(m_PreviewDocument.Patterns.begin(), m_PreviewDocument.Patterns.end(),
            [&](const auto& value) { return value.strPatternId == patternId; });
        if (owner == m_PreviewDocument.Patterns.end() ||
            std::none_of(owner->PresentationOccurrences.begin(), owner->PresentationOccurrences.end(),
                [&](const auto& value) { return value.strOccurrenceId == occurrence.strOccurrenceId &&
                    value.strResourceId == occurrence.strResourceId; })) return false;
    }
    const auto finite = [](const auto& values, const double minimum, const double maximum) {
        return std::all_of(values.begin(), values.end(), [=](const double value) {
            return std::isfinite(value) && value >= minimum && value <= maximum;
        });
    };
    if (!finite(occurrence.PositionOffset, -100000.0, 100000.0) ||
        !finite(occurrence.RotationDegrees, -36000.0, 36000.0) ||
        !finite(occurrence.Scale, 0.001, 10000.0)) return false;
    const auto update = [&](KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, SESSION& session) {
        if (pattern.strPatternId != patternId && !m_bColliderResourcePreview) return false;
        const auto box = std::find_if(pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(),
            [&](const auto& value) { return value.strOccurrenceId == occurrence.strOccurrenceId &&
                value.strResourceId == occurrence.strResourceId; });
        if (box == pattern.PresentationOccurrences.end()) return false;
        const auto resource = std::find_if(m_PreviewDocument.PresentationResources.begin(), m_PreviewDocument.PresentationResources.end(),
            [&](const auto& value) { return value.strResourceId == box->strResourceId &&
                (value.eKind == KIND::COLLIDER || value.eKind == KIND::EFFECT || value.eKind == KIND::SUBTITLE); });
        if (resource == m_PreviewDocument.PresentationResources.end()) return false;
        auto edited = *box;
        edited.PositionOffset = occurrence.PositionOffset;
        edited.RotationDegrees = occurrence.RotationDegrees;
        edited.Scale = occurrence.Scale;
        if (resource->eKind == KIND::COLLIDER)
        {
            edited.strAnchorPresentationOccurrenceId = occurrence.strAnchorPresentationOccurrenceId;
            edited.strColliderMotion = occurrence.strColliderMotion;
            edited.ColliderEndPositionOffset = occurrence.ColliderEndPositionOffset;
            edited.ColliderEndScale = occurrence.ColliderEndScale;
        }
        if (resource->eKind == KIND::SUBTITLE)
        {
            // Screen layout edits do not restart audio, animation or the subtitle clock.
            if (const auto active = session.rows.find(box->strOccurrenceId); active != session.rows.end())
            {
                active->second.subtitleScreenOffset = { float(edited.PositionOffset[0]), float(edited.PositionOffset[1]) };
                active->second.subtitleTextScale = float(edited.Scale[0]);
            }
            *box = std::move(edited);
            return true;
        }
        bool anchorChanged = edited.strAnchorPresentationOccurrenceId != box->strAnchorPresentationOccurrenceId ||
            Is_CenteredWorldCircle(*resource, edited) != Is_CenteredWorldCircle(*resource, *box);
        if (resource->eKind == KIND::EFFECT)
        {
            edited.strAnchorKind = occurrence.strAnchorKind;
            edited.bFollowBoss = occurrence.bFollowBoss;
            edited.strBone = occurrence.strBone;
            edited.strBoneTarget = occurrence.strBoneTarget;
            edited.strBoneRotation = occurrence.strBoneRotation;
            edited.strWorldId = occurrence.strWorldId;
            edited.strWorldOccurrenceId = occurrence.strWorldOccurrenceId;
            edited.iWorldEmissionIndex = occurrence.iWorldEmissionIndex;
            if (!Validate_EffectAnchor(m_PreviewDocument, pattern, edited, m_strStatus)) return false;
            anchorChanged = edited.strAnchorKind != box->strAnchorKind || edited.bFollowBoss != box->bFollowBoss ||
                edited.strBone != box->strBone || edited.strBoneTarget != box->strBoneTarget ||
                edited.strBoneRotation != box->strBoneRotation ||
                edited.strWorldId != box->strWorldId || edited.strWorldOccurrenceId != box->strWorldOccurrenceId ||
                edited.iWorldEmissionIndex != box->iWorldEmissionIndex;
        }
        const auto active = session.rows.find(box->strOccurrenceId);
        if (anchorChanged)
        {
            // A new anchor owns a new occurrence playback. Other rows and the
            // session's observed boss history retain their current clock/state.
            if (active != session.rows.end())
            {
                if (active->second.effectHandle) CEffectV2Runtime::Stop_Group(active->second.effectHandle);
                if (active->second.v1EffectHandle) CEffectPresentationService::Stop_WorldRoot({active->second.v1EffectHandle});
                session.rows.erase(active);
            }
            session.effectAnchorHistories.erase(box->strOccurrenceId);
            *box = std::move(edited);
            return true;
        }
        if (active != session.rows.end() && active->second.hasPlacementAnchor &&
            !active->second.failed && !active->second.waitingForAnchor)
        {
            // Frozen rows retain their first anchor; following rows retain the
            // current sampled anchor and all prior particle birth transforms.
            auto placed = resource->eKind == KIND::COLLIDER ? Sample_ColliderGeometry(edited, session.lastClockMs) : edited;
            placed.strBone.clear();
            for (size_t axis = 0u; axis < 3u; ++axis)
            {
                placed.PositionOffset[axis] *= active->second.placementAnchorScale[axis];
                placed.Scale[axis] *= active->second.placementAnchorScale[axis];
            }
            float4x4_t pivot{};
            if (!Make_Pivot(placed, active->second.placementAnchor, nullptr, pivot)) return false;
            if (resource->eKind == KIND::EFFECT && active->second.effectHandle &&
                !CEffectV2Runtime::Rebuild_GroupPlacement(active->second.effectHandle, pivot,
                    Effect_PivotSampler(edited, session.rootHistory, active->second.effectPivotHistory),
                    m_Device, m_Context))
            {
                m_strStatus = "Effect geometry preview failed: " + CEffectV2Runtime::Last_Error();
                return false;
            }
            if (resource->eKind == KIND::EFFECT && active->second.v1EffectHandle &&
                (!CEffectPresentationService::Update_WorldRoot({active->second.v1EffectHandle}, pivot) ||
                 !CEffectPresentationService::Seek_WorldRoot({active->second.v1EffectHandle},
                    (std::max)(0.f, active->second.lastAge - active->second.v1CycleStartSeconds) * Effect_SourceClockRate(*resource, edited),
                    Effect_V1TransformProvider(edited, pivot, session.rootHistory, active->second.effectPivotHistory,
                        active->second.sourceAnchorSampler, Effect_SourceClockRate(*resource, edited), {}, active->second.v1CycleStartSeconds), true,
                    ((std::min)(edited.iDurationMs / 1000.f - active->second.v1CycleStartSeconds,
                        active->second.v1FiniteLoopSeconds > 0.f ? active->second.v1FiniteLoopSeconds : edited.iDurationMs / 1000.f)) *
                        Effect_SourceClockRate(*resource, edited))))
            {
                m_strStatus = "V1 Effect geometry preview lost its active handle: " +
                    CEffectPresentationService::Get_Status();
                return false;
            }
            active->second.pivot = pivot;
            if (resource->eKind == KIND::COLLIDER)
                active->second.wire = Collider_Wire(*resource, placed);
        }
        else if (active != session.rows.end())
        {
            if (active->second.effectHandle) CEffectV2Runtime::Stop_Group(active->second.effectHandle);
            if (active->second.v1EffectHandle) CEffectPresentationService::Stop_WorldRoot({active->second.v1EffectHandle});
            session.rows.erase(active);
        }
        *box = std::move(edited);
        return true;
    };
    for (auto& member : m_BundlePreviewMembers)
    {
        if (!update(member.pattern, member.session)) continue;
        const double localMs = m_fPreviewClockMs - double(member.offsetTicks) * 1000.0 / 30.0;
        if (member.actor && localMs >= 0.0 && localMs < member.durationMs)
        {
            ANIMATION_MODEL_TARGET_VIEW weaponView;
            const bool hasWeapon = member.actor->Try_GetAnimationModelTarget(ANIMATION_BONE_TARGET::WEAPON, weaponView);
            Sample(member.session, m_PreviewDocument, member.pattern, float(localMs), m_bPreviewPaused,
                *member.actor->Get_Transform()->Get_WorldMatrixPtr(), member.actor->Get_Model(),
                hasWeapon ? &weaponView : nullptr);
        }
        return true;
    }
    if (!update(m_PreviewPattern, m_PreviewSession)) return false;
    // MainApp samples the single-pattern owner later in this same frame with its
    // actual animation target and weapon view. Its displayed clock is unchanged.
    return true;
}

void Client::CKoukuSaydonPresentationPlayer::Render_Debug() const
{
#ifdef _DEBUG
    if (m_LightProvider)
        for (const auto& light : m_LightProvider->lights)
            if (light.debugRender) Draw_LightWire(light.desc);
    const auto draw = [this](const SESSION& session, bool preview)
    {
        for (const auto& [id, row] : session.rows)
            if (!row.failed && !row.waitingForAnchor && row.kind == KIND::COLLIDER)
            {
                const auto override = m_ColliderDebugOverrides.find(id);
                const bool visible = !preview && override != m_ColliderDebugOverrides.end() ? override->second : row.debugRender;
                if (visible) CHitAreaWire::Draw(row.pivot, row.wire, 0xff40dfff);
            }
    };
    for (const auto& [id, session] : m_BossSessions) draw(session, false);
    for (const auto& [id, session] : m_ChildBossSessions) draw(session, false);
    for (const auto& [id, session] : m_MarioEntrySessions) draw(session, false);
    if (m_bPreviewPlaying) draw(m_PreviewSession, true);
    for (const auto& member : m_BundlePreviewMembers) draw(member.session, true);
#endif
}

void Client::CKoukuSaydonPresentationPlayer::Update_BossStageEnvironments(
    const std::vector<BOSS_STAGE_ENVIRONMENT_SAMPLE>& samples)
{
    std::set<std::string> live;
    for (const auto& sample : samples)
    {
        if (sample.strOwnerKey.empty() || sample.strActionId.empty() ||
            !std::isfinite(sample.fClockMs) || sample.fClockMs < 0.f) continue;
        live.insert(sample.strOwnerKey);
        auto& external = m_ExternalStageEnvironments[sample.strOwnerKey];
        const auto signature = Serialize_BossStageEnvironment(sample.SceneProfileOccurrences, sample.LightOccurrences);
        if (external.actionId != sample.strActionId || external.signature != signature ||
            external.preview != sample.bPreview || sample.fClockMs + .5f < external.session.lastClockMs)
        {
            Stop_Session(external.session);
            external.actionId = sample.strActionId;
            external.signature = signature;
            external.preview = sample.bPreview;
            external.document = {};
            external.pattern = {};
            external.pattern.strPatternId = sample.strActionId;
            external.session.key = "boss-stage:" + sample.strOwnerKey + ":" + sample.strActionId;
            for (const auto& source : sample.SceneProfileOccurrences)
            {
                KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION definition;
                definition.strSceneProfileId = source.strProfileId;
                definition.strRenderingProfileId = source.strProfileId;
                external.document.SceneProfiles.push_back(std::move(definition));
                KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE box;
                box.strOccurrenceId = source.strOccurrenceId;
                box.strSceneProfileId = source.strProfileId;
                box.iStartMs = source.iStartMs;
                box.iDurationMs = source.iDurationMs;
                external.pattern.SceneProfileOccurrences.push_back(std::move(box));
            }
            for (const auto& source : sample.LightOccurrences)
            {
                RESOURCE definition;
                definition.eKind = KIND::LIGHT;
                definition.strResourceId = source.strOccurrenceId;
                definition.strAssetId = source.strLightResourceId;
                external.document.PresentationResources.push_back(std::move(definition));
                OCCURRENCE box;
                box.strOccurrenceId = source.strOccurrenceId;
                box.strResourceId = source.strOccurrenceId;
                box.iStartMs = source.iStartMs;
                box.iDurationMs = source.iDurationMs;
                box.strAnchorKind = source.strAnchorKind;
                box.bFollowBoss = source.bFollowBoss;
                box.bDebugRender = sample.bPreview;
                box.fBrightnessMultiplier = source.fBrightnessMultiplier;
                box.iFadeInMs = source.iFadeInMs;
                box.iFadeOutMs = source.iFadeOutMs;
                for (size_t axis = 0; axis < 3u; ++axis)
                {
                    box.PositionOffset[axis] = source.Position[axis];
                    box.RotationDegrees[axis] = source.RotationDegrees[axis];
                }
                external.pattern.PresentationOccurrences.push_back(std::move(box));
            }
        }
        Sample(external.session, external.document, external.pattern, sample.fClockMs,
            sample.bPreview, sample.Root, nullptr);
    }
    for (auto it = m_ExternalStageEnvironments.begin(); it != m_ExternalStageEnvironments.end();)
    {
        if (live.contains(it->first)) { ++it; continue; }
        Stop_Session(it->second.session);
        it = m_ExternalStageEnvironments.erase(it);
    }
    Refresh_SharedPresentation();
}

void Client::CKoukuSaydonPresentationPlayer::Sample_ServerSequence(const std::uint32_t clockMs)
{
    if (!m_bPreviewPlaying || !Preview_IsBundle()) return;
    m_bServerSequenceClock = true; m_bPreviewPaused = false;
    m_fPreviewClockMs = (std::min)(clockMs, m_iPreviewDurationMs);
    m_strCompletedPreviewPatternId.clear();
    Sample_BundlePreview();
}

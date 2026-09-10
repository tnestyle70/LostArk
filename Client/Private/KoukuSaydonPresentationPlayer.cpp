#include "imgui.h"
#include "KoukuSaydonPresentationPlayer.h"

#include "ActionPresentationTimeline.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "DataJson.h"
#include "EffectV2_Catalog.h"
#include "Effect_PresentationService.h"
#include "Effect_Catalog.h"
#include "NetworkManager.h"
#include "EffectV2_Object.h"
#include "EffectV2_Runtime.h"
#include "GameInstance.h"
#include "CombatHUDViewModel.h"
#include "Level_KakulSaydonArena.h"
#include "LightResourceCatalog.h"
#include "Presentation_Manager.h"
#include "Model.h"
#include "Npc.h"
#include "WorldSequencePlayer.h"
#include "ProjectDataRoot.h"
#include "RenderingProfileService.h"
#include "RuntimeAssetRoot.h"
#include "Transform.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>
#include <set>
#include <stdexcept>

namespace
{
using namespace Client;
using RESOURCE = KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE;
using OCCURRENCE = KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE;
using KIND = KOUKU_SAYDON_PRESENTATION_KIND;
constexpr std::uint32_t MAX_TIMELINE_MS = 600000u;

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
    resource.strResourceKind = Text(row, "resourceKind", resource.eKind != KIND::EFFECT);
    if (row.Find("elementId")) resource.strElementId = Text(row, "elementId", true);
    resource.iDurationMs = UInt(row, "resourceDurationMs", 1u, MAX_TIMELINE_MS);
    resource.strShape = Text(row, "shape");
    resource.HalfExtents = Vector(row, "halfExtents", 0.001, 100000.0);
    resource.fRadiusM = Number(row, "radiusM", 0.001, 100000.0);
    resource.fHalfAngleDegrees = Number(row, "halfAngleDegrees", 0.001, 180.0);
    const bool v1 = resource.strResourceKind == "V1_EFFECT" || resource.strResourceKind == "V1_ELEMENT";
    if ((resource.eKind == KIND::EFFECT && resource.strResourceKind != "GROUP" && resource.strResourceKind != "LEAF" && !v1) ||
        (resource.strShape != "BOX" && resource.strShape != "SECTOR" && resource.strShape != "CIRCLE"))
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
    if (box.fDissolveStart >= box.fDissolveEnd)
        throw std::runtime_error("Presentation dissolve interval is empty or reversed.");
    box.fVolume = Number(row, "volume", 0.0, 1.0);
    if (row.Find("brightnessMultiplier")) box.fBrightnessMultiplier = Number(row, "brightnessMultiplier", 0.0, 16.0);
    const auto& follow = Field(row, "followBoss");
    if (!follow.Is_Boolean()) throw std::runtime_error("followBoss must be Boolean.");
    box.bFollowBoss = follow.Get_Boolean();
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
    if (box.strAnchorKind != "BOSS" && box.strAnchorKind != "WORLD" &&
        !(kind == KIND::LIGHT && (box.strAnchorKind == "MAP" || box.strAnchorKind == "PLAYER")))
        throw std::runtime_error("Presentation anchorKind is unsupported.");
    if (kind == KIND::LIGHT && (box.strAnchorKind == "WORLD" || !box.strWorldId.empty() ||
        box.Scale != std::array<double, 3u>{1.0, 1.0, 1.0} ||
        (box.strAnchorKind != "BOSS" && !box.strBone.empty()) ||
        (box.strAnchorKind == "PLAYER" && !box.bFollowBoss)))
        throw std::runtime_error("Invalid Light anchor, scale or bone.");
    if (box.strAnchorKind == "WORLD" && box.strWorldId.empty())
        throw std::runtime_error("WORLD presentation anchor needs a worldId.");
    if (box.strBoneTarget == "WEAPON" && ((kind != KIND::COLLIDER && kind != KIND::EFFECT) || box.strAnchorKind != "BOSS" || box.strBone.empty()))
        throw std::runtime_error("WEAPON bone target needs a Boss Collider/Effect and a named weapon bone.");
    return box;
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
    std::uint64_t duration = 0;
    for (const auto& stage : pattern.Stages) duration += stage.iDurationMs;
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
    const std::shared_ptr<EFFECT_V2_PIVOT_HISTORY>& anchorHistory)
{
    if (!box.bFollowBoss) return {};
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

EFFECT_FIXED_STEP_TRANSFORM_PROVIDER Effect_V1TransformProvider(const OCCURRENCE& box,
    const float4x4_t& frozenPivot, const std::shared_ptr<EFFECT_V2_PIVOT_HISTORY>& rootHistory,
    const std::shared_ptr<EFFECT_V2_PIVOT_HISTORY>& anchorHistory)
{
    return [sampler = Effect_PivotSampler(box, rootHistory, anchorHistory), frozenPivot]
        (float seconds, EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& output, std::string& error)
    {
        // V1 and V2 consume the same recorded owner/bone poses and authored
        // placement. The V1 playback adds its existing model-cue anchors.
        if (sampler) return sampler(seconds, output.RootWorld, error);
        output.RootWorld = frozenPivot;
        error.clear();
        return true;
    };
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
        shape.iAreaType = resource.strShape == "CIRCLE" ? 1 : 3;
        shape.iAreaRange = static_cast<int32_t>((std::min)(
            resource.fRadiusM * (std::max)(box.Scale[0], box.Scale[2]) * 100.0, 1000000000.0));
        shape.iAreaAngle = static_cast<int32_t>(resource.fHalfAngleDegrees * 2.0);
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
        const auto used = presentation.Get_TransientLights().size();
        const std::size_t remaining = used < 64u ? 64u - used : 0u;
        const auto count = (std::min)(remaining, lights.size());
        skippedByBudget = lights.size() - count;
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
            const auto append = [&](const float4x4_t& pivot)
            {
                LIGHT_DESC desc{};
                std::string status;
                if (!CLightResourceCatalog::Try_BuildLightDesc(*resource, pivot, row.lightWeight, desc, status))
                { m_strStatus = "Light occurrence isolated: " + id + "; " + status; return; }
                m_LightProvider->lights.push_back({desc, row.debugRender});
            };
            if (row.lightBox.strAnchorKind == "PLAYER")
            {
                // No present character is a temporary empty target set, never a sticky row failure.
                for (const auto& root : m_LightPlayerPivots)
                {
                    float4x4_t pivot;
                    if (Make_Pivot(row.lightBox, root, nullptr, pivot)) append(pivot);
                }
            }
            else
            {
                append(row.pivot);
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
                        *npc->Get_Transform()->Get_WorldMatrixPtr(), npc->Get_Model(), pivot)) append(pivot);
                }
            }
        }
    };
    for (const auto& [id, session] : m_BossSessions) collect(session, false, id);
    if (m_bPreviewPlaying) collect(m_PreviewSession, true);
    for (const auto& member : m_BundlePreviewMembers) collect(member.session, true);
    collect(m_FearSession, false);
    if (!m_LightProvider->lights.empty())
        if (FAILED(CPresentation_Manager::Get().Add_FrameProvider(m_LightProvider)))
            m_strStatus = "Light frame provider registration failed.";
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
    const auto pattern = m_Product.find(std::string(patternId));
    if (pattern == m_Product.end()) return false;
    const auto anchor = pattern->second.worldEmissionAnchors.find(std::string(occurrenceId));
    out = anchor == pattern->second.worldEmissionAnchors.end() ? WORLD_EMISSION_ANCHOR{} : anchor->second;
    return true;
}

bool Client::CKoukuSaydonPresentationPlayer::Reload_Product(
    std::string& status, const std::uint32_t expectedSourceRevision)
{
    // Restart keeps its admitted in-memory Product even after another publish.
    if (expectedSourceRevision && m_bProductLoaded && expectedSourceRevision == m_iProductSourceRevision)
    { status = "KoukuSaydon presentation already matches the admitted source revision."; return true; }
    m_bProductAttempted = true;
    try
    {
        const auto path = CProjectDataRoot::Resolve(
            "Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json");
        std::error_code error;
        const auto bytes = std::filesystem::file_size(path, error);
        if (error || bytes > 16u * 1024u * 1024u)
            throw std::runtime_error("KoukuSaydon Product presentation is missing or oversized.");
        std::ifstream input(path, std::ios::binary);
        const std::string text{ std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
        if (!input || input.bad() || text.size() != bytes)
            throw std::runtime_error("KoukuSaydon Product presentation read failed.");
        DATA_JSON_VALUE root;
        std::string parseStatus;
        if (!CDataJson::Parse(text, root, parseStatus) || !root.Is_Object())
            throw std::runtime_error("KoukuSaydon Product parse failed: " + parseStatus);
        if (Text(root, "schema") != "lostark.kouku-saydon-pattern-bindings" ||
            UInt(root, "formatVersion", 1u, 1u) != 1u ||
            Text(root, "bossArchetypeId") != "BOSS_KAKULSAYDON_G1_KOUKU")
            throw std::runtime_error("KoukuSaydon Product presentation header is incompatible.");
        const auto sourceRevision = UInt(root, "sourceRevision", 1u, UINT32_MAX);
        if (expectedSourceRevision && sourceRevision != expectedSourceRevision)
            throw std::runtime_error("KoukuSaydon presentation source revision mismatch: requested " +
                std::to_string(expectedSourceRevision) + ", published " + std::to_string(sourceRevision) +
                ". Previous presentation is preserved; publish the matching Product on this Client.");
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
            if (value.Find("animationRootVerticalScale"))
                item.pattern.fAnimationRootVerticalScale = Number(value, "animationRootVerticalScale", 0.0, 1.0);
            if (const auto* source = value.Find("bossMotion"))
            {
                KOUKU_SAYDON_BOSS_MOTION motion;
                motion.iStartMs = UInt(*source, "startMs", 0u, item.durationMs - 1u);
                motion.iEndMs = UInt(*source, "endMs", motion.iStartMs + 1u, item.durationMs);
                motion.StartPosition = Vector(*source, "startPosition", -100000.0, 100000.0);
                motion.EndPosition = Vector(*source, "endPosition", -100000.0, 100000.0);
                motion.fYawDegrees = Number(*source, "yawDegrees", -360.0, 360.0);
                if (std::abs(motion.StartPosition[1] - motion.EndPosition[1]) > 0.0001)
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
                if (occurrence.strAnchorKind == "WORLD")
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

        // Only a fully staged replacement may stop the old running presentation.
        for (auto& [id, session] : m_BossSessions) Stop_Session(session);
        m_BossSessions.clear();
        m_Product = std::move(staged);
        Stop_Session(m_FearSession);
        m_FearSession.key.clear();
        m_strCompletedFearKey.clear();
        m_FearPresentations = std::move(stagedFear);
        Stop_Session(m_ProductBundleSession);
        m_ProductBundles = std::move(stagedBundles);
        m_iProductSourceRevision = sourceRevision;
        m_MissingProductPatterns.clear();
        m_bProductLoaded = true;
        Refresh_SharedPresentation();
        status = "Loaded KoukuSaydon presentation for " + std::to_string(m_Product.size()) + " Product patterns.";
        if (isolatedLights) status += " Isolated invalid LIGHT rows: " + std::to_string(isolatedLights);
        if (isolatedSceneProfiles) status += " Isolated invalid SCENE_PROFILE rows: " +
            std::to_string(isolatedSceneProfiles) + ". " + isolatedSceneStatus;
        m_strStatus = status;
        return true;
    }
    catch (const std::exception& error)
    {
        status = error.what();
        m_strStatus = status;
        OutputDebugStringA(("[KoukuSaydonPresentationPlayer] " + status + "\n").c_str());
        return false;
    }
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
    session.effectAnchorHistories.clear();
    session.rootRecordedSeconds = -1.f;
    session.lastClockMs = -1.f;
}

void Client::CKoukuSaydonPresentationPlayer::Sample(SESSION& session,
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, float clockMs, bool paused,
    const float4x4_t& pivot, const std::shared_ptr<Engine::CModel>& model,
    const ANIMATION_MODEL_TARGET_VIEW* weaponView)
{
    if (!std::isfinite(clockMs) || clockMs < 0.f) return;
    if (session.lastClockMs >= 0.f &&
        (clockMs < session.lastClockMs - 0.5f || clockMs > session.lastClockMs + 150.f))
    {
        // Same occurrence seek retains observed actor history; a new Server run
        // calls Stop_Session separately and never borrows the previous run.
        auto history = session.rootHistory;
        auto anchorHistories = std::move(session.effectAnchorHistories);
        const auto recordedSeconds = session.rootRecordedSeconds;
        const auto recordedPivot = session.rootRecordedPivot;
        Stop_Session(session);
        session.rootHistory = std::move(history);
        session.effectAnchorHistories = std::move(anchorHistories);
        session.rootRecordedSeconds = recordedSeconds;
        session.rootRecordedPivot = recordedPivot;
    }
    if (!session.rootHistory) session.rootHistory = std::make_shared<EFFECT_V2_PIVOT_HISTORY>();
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
            return selected && selected->Try_GetSequencePivot(world->strSequenceInstanceId, anchor);
        }
        const auto* level = CLevel_KakulSaydonArena::Get_Active();
        if (!level) return false;
        if (session.runEpoch)
            return level->Try_GetOwnedCompositionWorldPivot(session.runEpoch, session.memberId,
                world->strSequenceInstanceId, box.strWorldOccurrenceId, anchor);
        return level->Try_GetCompositionWorldPivot(world->strSequenceInstanceId, anchor, box.strWorldOccurrenceId);
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
        active.insert(box.strOccurrenceId);
        auto [found, inserted] = session.rows.try_emplace(box.strOccurrenceId);
        PLAYING_ROW& row = found->second;
        if (row.failed) return;
        const float age = (clockMs - box.iStartMs) / 1000.f;
        row.kind = resource.eKind;
        row.debugRender = box.bDebugRender;
        row.startMs = float(box.iStartMs);
        row.cameraDurationMs = box.iDurationMs;
        row.assetId = resource.strAssetId;
        row.cameraOffset = { float(box.PositionOffset[0]), float(box.PositionOffset[1]), float(box.PositionOffset[2]) };
        if (resource.eKind == KIND::LIGHT)
        {
            row.lightBox = box;
            row.lightWeight = float(box.fBrightnessMultiplier);
            if (box.iFadeInMs) row.lightWeight *= (std::min)(1.f, (clockMs - box.iStartMs) / box.iFadeInMs);
            if (box.iFadeOutMs) row.lightWeight *= (std::min)(1.f, (box.iStartMs + box.iDurationMs - clockMs) / box.iFadeOutMs);
        }
        if (resource.eKind != KIND::CAMERA && (inserted || box.bFollowBoss || row.waitingForAnchor) &&
            !(resource.eKind == KIND::LIGHT && box.strAnchorKind == "PLAYER"))
        {
            OCCURRENCE placedBox = box;
            float4x4_t anchor = pivot;
            auto anchorModel = model;
            if (resource.eKind == KIND::LIGHT && box.strAnchorKind == "MAP")
            { XMStoreFloat4x4(&anchor, XMMatrixIdentity()); anchorModel.reset(); }
            if (box.strAnchorKind == "WORLD")
            {
                if (!resolveWorldPivot(box, anchor))
                {
                    // A WORLD may not have its first sampled pose yet. Retry
                    // Collider anchors next frame instead of hiding this box forever.
                    row.waitingForAnchor = resource.eKind == KIND::COLLIDER || resource.eKind == KIND::EFFECT;
                    row.failed = !row.waitingForAnchor;
                    m_strStatus = "WORLD presentation anchor unavailable: " + box.strWorldId;
                    return;
                }
                const matrix_t worldMatrix = XMLoadFloat4x4(&anchor);
                for (size_t axis = 0; axis < 3u; ++axis)
                {
                    const float scale = XMVectorGetX(XMVector3Length(worldMatrix.r[axis]));
                    placedBox.Scale[axis] *= scale;
                    placedBox.PositionOffset[axis] *= scale;
                }
                placedBox.strBone.clear();
                anchorModel.reset();
            }
            if (!Make_Pivot(placedBox, anchor, anchorModel, row.pivot, weaponView,
                (resource.eKind == KIND::COLLIDER || resource.eKind == KIND::EFFECT) ? &row.placementAnchor : nullptr))
            {
                row.waitingForAnchor = resource.eKind == KIND::LIGHT;
                row.failed = !row.waitingForAnchor;
                m_strStatus = "Presentation bone/pivot is unavailable: " + box.strOccurrenceId;
                return;
            }
            if (row.waitingForAnchor)
                m_strStatus = "Collider WORLD anchor ready: " + box.strWorldId;
            row.waitingForAnchor = false;
            if (resource.eKind == KIND::COLLIDER)
                row.wire = Collider_Wire(resource, placedBox);
            if (resource.eKind == KIND::COLLIDER || resource.eKind == KIND::EFFECT)
            {
                for (size_t axis = 0u; axis < 3u; ++axis)
                    row.placementAnchorScale[axis] = placedBox.Scale[axis] / box.Scale[axis];
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
                    { row.failed = true; m_strStatus = "V1 Effect preparation failed: " + resource.strAssetId; break; }
                    if (!preparation.bCatalogRevisionCurrent || !preparation.bSettled) break;
                    EFFECT_LEVEL_PLACEMENT_SPAWN_DESC spawn;
                    spawn.iLevelIndex = CGameInstance::Get().Get_CurrentLevelID();
                    spawn.strPlacementId = "kouku:" + session.key + ":" + box.strOccurrenceId;
                    spawn.strEffectAssetId = resource.strAssetId;
                    spawn.strElementId = resource.strElementId;
                    spawn.RootWorld = row.pivot;
                    spawn.fInitialSampleTimeSeconds = age;
                    spawn.bExternallySampled = true;
                    EFFECT_WORLD_ROOT_HANDLE handle;
                    if (!CEffectPresentationService::Spawn_LevelPlacement(spawn, handle, m_strStatus)) row.failed = true;
                    else row.v1EffectHandle = handle.iValue;
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
                playback.PivotSampler = Effect_PivotSampler(box, session.rootHistory, row.effectPivotHistory);
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
                const auto path = CRuntimeAssetRoot::Resolve(resource.strAssetId);
                if (!path.empty()) row.soundHandle = CGameInstance::Get().Play_SoundCue(
                    path.wstring(), float(box.fVolume), static_cast<std::uint32_t>(age * 1000.f));
                if (!row.soundHandle)
                {
                    row.failed = true;
                    m_strStatus = "Sound occurrence unavailable: " + resource.strAssetId;
                }
                break;
            }
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
            if (!CEffectPresentationService::Update_WorldRoot({row.v1EffectHandle}, row.pivot) ||
                !CEffectPresentationService::Seek_WorldRoot({row.v1EffectHandle}, age,
                    Effect_V1TransformProvider(box, row.pivot, session.rootHistory, row.effectPivotHistory)))
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
    // Stable entity/id order resolves overlapping presentation; preview owns the final choice.
    for (const auto& [id, session] : m_BossSessions) choose(session, true);
    choose(m_ProductBundleSession, true);
    for (const auto& member : m_BundlePreviewMembers) choose(member.session, false);
    if (m_bPreviewPlaying) choose(m_PreviewSession, false);
    // A local player's temporary fear owns the scene until its replicated end.
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
        if (camera)
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
            level->Stop_CompositionCamera();
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
    if (!std::isfinite(dt) || dt < 0.f) return;
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
    const auto* arena = CLevel_KakulSaydonArena::Get_Active();
    const auto* run = arena ? &arena->Get_KoukuBundleState() : nullptr;
    using RUN_STATE = LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE;
    const bool runLive = run && run->iRunEpoch &&
        (run->eState == RUN_STATE::PENDING || run->eState == RUN_STATE::ACTIVE || run->eState == RUN_STATE::PATTERN_COMPLETED);
    m_ProductBundleSession.runEpoch = runLive && !run->strBundleId.empty() ? run->iRunEpoch : 0u;
    if (runLive && m_iProductReloadRunEpoch != run->iRunEpoch)
    {
        m_iProductReloadRunEpoch = run->iRunEpoch;
        if (auto* level = CLevel_KakulSaydonArena::Get_Active())
        { std::string cameraStatus; if (!level->Reload_PublishedCameraShots(cameraStatus)) m_strStatus = cameraStatus; }
        if (run->iPinnedSourceRevision != m_iProductSourceRevision)
        { std::string status; (void)Reload_Product(status, run->iPinnedSourceRevision); }
    }
    if (runLive && !run->strBundleId.empty())
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
            const auto key = "bundle-product:" + std::to_string(run->iRunEpoch) + ":" + run->strBundleId;
            if (m_ProductBundleSession.key != key)
            { Stop_Session(m_ProductBundleSession); m_ProductBundleSession.key = key; }
            const float clock = m_ProductBundleSession.lastClockMs < 0.f ? seconds * 1000.f :
                (std::max)(seconds * 1000.f, m_ProductBundleSession.lastClockMs + dt * 1000.f);
            float4x4_t pivot; XMStoreFloat4x4(&pivot, XMMatrixIdentity());
            Sample(m_ProductBundleSession, product->second.common.document, product->second.common.pattern,
                clock, false, pivot, nullptr);
        }
        else Stop_Session(m_ProductBundleSession);
    }
    else Stop_Session(m_ProductBundleSession);
    std::set<std::uint32_t> liveBosses;
    for (const auto& view : bosses)
    {
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
        liveBosses.insert(id);
        SESSION& session = m_BossSessions[id];
        session.runEpoch = 0u; session.memberId.clear();
        if (runLive)
            for (const auto& member : run->Members)
                if (member.iBossNetEntityId == id)
                { session.runEpoch = run->iRunEpoch; session.memberId = member.strMemberId; break; }
        const std::string key = std::to_string(id) + ":" + view.Snapshot.strPatternId + ":" +
            std::to_string(view.Snapshot.iPatternSequence) + ":" + std::to_string(view.Snapshot.iPatternStartTick);
        if (session.key != key) { Stop_Session(session); session.key = key; }
        // Interpolate between snapshots without rewinding/restarting effects every network tick.
        const float clock = session.lastClockMs < 0.f ? seconds * 1000.f :
            (std::max)(seconds * 1000.f, session.lastClockMs + dt * 1000.f);
        ANIMATION_MODEL_TARGET_VIEW weaponView;
        const bool hasWeapon = npc->Try_GetAnimationModelTarget(ANIMATION_BONE_TARGET::WEAPON, weaponView);
        Sample(session, product->second.document, product->second.pattern,
            (std::min)(clock, float(product->second.durationMs)), false,
            *npc->Get_Transform()->Get_WorldMatrixPtr(), npc->Get_Model(), hasWeapon ? &weaponView : nullptr);
    }
    for (auto session = m_BossSessions.begin(); session != m_BossSessions.end();)
    {
        if (liveBosses.contains(session->first)) { ++session; continue; }
        Stop_Session(session->second);
        session = m_BossSessions.erase(session);
    }
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
    Update_MazeMarks(players);
    Update_FearPresentation(dt, players);
    if (m_bPreviewPlaying && m_bOwnPreviewClock && m_bPreviewPivotReady)
    {
        if (!m_bPreviewPaused && !m_bModelReferencePreview) m_fPreviewClockMs += double(dt) * 1000.0;
        if (!m_bPreviewPaused && !m_bModelReferencePreview && m_fPreviewClockMs >= m_iPreviewDurationMs)
        {
            if (m_bColliderResourcePreview)
            {
                m_fPreviewClockMs = m_iPreviewDurationMs - 1u;
                Pause_Preview(true);
            }
            else Stop_Preview();
        }
        // Bundle members own their independent WORLD players, sampled before effects.
        if (Preview_IsBundle()) Sample_BundlePreview();
        // MainApp samples single-pattern WORLD first, then its presentation.
    }
    Refresh_SharedPresentation();
}

bool Client::CKoukuSaydonPresentationPlayer::Begin_Preview(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, bool ownClock,
    std::uint32_t clockMs, bool paused, std::string& status)
{
    const auto duration = Pattern_Duration(pattern);
    if (!pattern.strLoadError.empty() || !duration)
    {
        status = "Presentation preview needs a valid finite pattern.";
        return false;
    }
    for (const auto& box : pattern.PresentationOccurrences)
        if (std::none_of(document.PresentationResources.begin(), document.PresentationResources.end(),
            [&box](const auto& resource) { return resource.strResourceId == box.strResourceId; }))
        {
            status = "Preview names an unknown presentation resource: " + box.strResourceId;
            return false;
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
        if (member.actor && member.actor->Get_Model()) (void)member.actor->Get_Model()->Set_RootMotionVerticalScale(1.f);
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

bool Client::CKoukuSaydonPresentationPlayer::Begin_BundlePreview(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document, const std::string& bundleId,
    std::uint32_t clockMs, bool paused, std::string& status, const CWorldSequenceDocument* sourceDocument)
{
    const auto bundle = std::find_if(document.Bundles.begin(), document.Bundles.end(),
        [&](const auto& value) { return value.strBundleId == bundleId; });
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level || bundle == document.Bundles.end() || !bundle->strLoadError.empty() ||
        bundle->Members.empty() || bundle->Members.size() > LostArk::Shared::MAX_KOUKUSAYDON_BUNDLE_MEMBERS)
    { status = "Bundle preview requires an active arena and a valid nonempty bundle."; return false; }
    std::vector<BUNDLE_PREVIEW_MEMBER> staged;
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
        member.durationMs = Pattern_Duration(*source);
        if (!member.durationMs) return fail("Empty child pattern cannot be previewed.");
        duration = (std::max)(duration, (std::uint64_t(member.offsetTicks) * 1000u + 29u) / 30u + member.durationMs);
        if (duration > MAX_TIMELINE_MS) return fail("Bundle preview exceeds the timeline duration limit.");
        if (!level->Create_CompositionPreviewActor(*source, member.actor, status)) return fail(status);
        const auto model = member.actor->Get_Model();
        member.initialAnimation = model->Get_CurrentAnimIndex();
        const auto& initialRoot = *member.actor->Get_Transform()->Get_WorldMatrixPtr();
        member.initialYawDegrees = XMConvertToDegrees(std::atan2(initialRoot._31, initialRoot._33));
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
                box.iStartOffsetMs += stageStart;
                member.animations.push_back(std::move(box));
            }
            stageStart += stage.iDurationMs;
        }
        std::sort(member.animations.begin(), member.animations.end(), [](const auto& a, const auto& b)
            { return a.iStartOffsetMs < b.iStartOffsetMs; });
        for (const auto& box : source->WorldOccurrences)
        {
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
            if (!player->Set_Document(sequences, targets, status))
                return fail(status);
            if (!player->Prepare_InstanceResources(world->strSequenceInstanceId, targets))
                return fail(player->Get_Status());
            if (!player->Validate_ObjectPlacement(world->strSequenceInstanceId, WorldPlacementFromOccurrence(box), status))
                return fail(status);
            const auto worldSpan = player->Get_InstanceElapsedSpanMs(
                world->strSequenceInstanceId, box.fPlaybackSpeed, box.iDurationMs);
            if (worldSpan <= 0.f) return fail("Bundle WORLD has no finite presentation span.");
            duration = (std::max)(duration, (std::uint64_t(member.offsetTicks) * 1000u + 29u) / 30u +
                box.iStartMs + static_cast<std::uint64_t>(std::ceil(worldSpan)));
            if (duration > MAX_TIMELINE_MS) return fail("Bundle WORLD tail exceeds the timeline duration limit.");
            member.session.previewWorlds.emplace(box.strOccurrenceId, player);
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
    // All models, clips and WORLD inputs are prepared before replacing the live preview.
    auto stagedDocument = document;
    Stop_Preview();
    m_PreviewDocument = std::move(stagedDocument);
    m_PreviewPattern = std::move(common);
    m_PreviewBundleId = bundleId;
    m_BundlePreviewMembers = std::move(staged);
    m_PreviewSession.key = "bundle-preview:" + bundleId + ":common";
    m_iPreviewDurationMs = static_cast<std::uint32_t>(duration);
    m_fPreviewClockMs = (std::min)(clockMs, m_iPreviewDurationMs);
    m_bOwnPreviewClock = m_bPreviewPlaying = m_bPreviewPivotReady = true;
    m_bPreviewPaused = paused;
    XMStoreFloat4x4(&m_PreviewPivot, XMMatrixIdentity());
    Sample_BundlePreview();
    Refresh_SharedPresentation();
    status = m_strStatus = "Bundle preview ready: " + bundleId;
    return true;
}

bool Client::CKoukuSaydonPresentationPlayer::Begin_ModelReferencePreview(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document, const std::string& selectionId,
    const bool isBundle, const std::uint32_t clockMs, const bool paused, std::string& status)
{
    KOUKU_SAYDON_COMPOSITION_DOCUMENT reference;
    reference.iRevision = document.iRevision;
    reference.strAreaId = document.strAreaId;
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
        for (auto& stage : pattern.Stages) stage.bRetargetOnEnter = false;
        pattern.LogicOccurrences.clear();
        pattern.SummonOccurrences.clear();
        pattern.WorldOccurrences.clear();
        pattern.SceneProfileOccurrences.clear();
        pattern.PresentationOccurrences.clear();
    }
    const std::string bundleId = bundle.strBundleId;
    reference.Bundles.push_back(std::move(bundle));
    if (!Begin_BundlePreview(reference, bundleId, clockMs, paused, status)) return false;
    m_bModelReferencePreview = true;
    status = m_strStatus = "Model reference ready. Master cursor owns time; actors stay at authored spawn (no Server movement replay).";
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

void Client::CKoukuSaydonPresentationPlayer::Sample_BundlePreviewFacing(
    BUNDLE_PREVIEW_MEMBER& member, const double localMs)
{
    if (std::none_of(member.pattern.Stages.begin(), member.pattern.Stages.end(),
        [](const auto& stage) { return stage.bRetargetOnEnter; })) return;
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    const auto player = level ? level->Get_LocalCharacter() : nullptr;
    const auto& root = *member.actor->Get_Transform()->Get_WorldMatrixPtr();
    float yaw = member.initialYawDegrees;
    std::uint32_t stageStartMs = 0u;
    for (const auto& stage : member.pattern.Stages)
    {
        if (localMs < stageStartMs) break;
        if (stage.bRetargetOnEnter)
        {
            auto [sample, inserted] = member.stageFacingYawDegrees.try_emplace(stage.strStageId, yaw);
            if (inserted && player && player->Get_Transform())
            {
                const auto& target = *player->Get_Transform()->Get_WorldMatrixPtr();
                const float dx = target._41 - root._41, dz = target._43 - root._43;
                if (std::isfinite(dx) && std::isfinite(dz) && dx * dx + dz * dz > .000001f)
                {
                    sample->second = XMConvertToDegrees(std::atan2(dx, dz));
                    // Match the Server's measured catalog +X face/hammer forward for Big Saydon.
                    if (CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(
                        member.pattern.strTargetBossPlacementId) == "BOSS_KAKULSAYDON_G2_BIG_SAYDON")
                        sample->second -= 90.f;
                }
            }
            // A seek reuses this stage's first sample, including its held yaw when no target existed.
            yaw = sample->second;
        }
        stageStartMs += stage.iDurationMs;
    }
    (void)member.actor->Apply_NetworkState({root._41, root._42, root._43}, yaw);
}

void Client::CKoukuSaydonPresentationPlayer::Sample_BundlePreview()
{
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level || !m_bPreviewPlaying) return;
    const auto targets = level->Get_CompositionWorldTargets();
    for (auto& member : m_BundlePreviewMembers)
    {
        const double localMs = m_fPreviewClockMs - double(member.offsetTicks) * 1000.0 / 30.0;
        const auto model = member.actor->Get_Model();
        (void)model->Set_RootMotionVerticalScale(float(member.pattern.fAnimationRootVerticalScale));
        const auto sampleMs = static_cast<float>((std::clamp)(localMs, 0.0, double(member.durationMs)));
        Sample_BundlePreviewFacing(member, localMs);
        std::array<double, 3u> bossPosition{};
        double bossYaw = 0.0;
        if (Sample_KoukuSaydonBossMotion(member.pattern, sampleMs, bossPosition, bossYaw))
            (void)member.actor->Apply_NetworkState(
                {float(bossPosition[0]), float(bossPosition[1]), float(bossPosition[2])}, float(bossYaw));
        const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE* animation = nullptr;
        const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE* previousAnimation = nullptr;
        for (const auto& box : member.animations)
            if (sampleMs >= box.iStartOffsetMs && localMs >= 0.0)
            { previousAnimation = animation; animation = &box; }
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
            const float age = (std::min)(sampleMs - animation->iStartOffsetMs, float(animation->iPlayMs));
            ticks = (animation->iSourceStartMs + age * animation->fPlayRate) * tps / 1000.f;
            if (animation->strEndPolicy == "LOOP_TO_WINDOW" && clipTicks > 0.f) ticks = std::fmod(ticks, clipTicks);
            else ticks = (std::min)(ticks, clipTicks);
        }
        model->Set_Animation(animationIndex, false, 0.f);
        model->Set_AnimPaused(true);
        model->Set_AnimTrackPosition(animationIndex, ticks);
        model->Update_Animation(0.f);
        if (animation && animation->iBlendInMs && previousAnimation)
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
                pose.sourceTicks = (std::min)(sourceEnd,
                    (previousAnimation->iSourceStartMs + previousAnimation->iPlayMs * previousAnimation->fPlayRate) *
                    model->Get_AnimationTickPerSecond(pose.sourceIndex) / 1000.f);
                if (!model->Set_AnimationTransitionPose(pose)) m_strStatus = "Animation blend pose admission failed.";
            }
        }
        for (const auto& box : member.pattern.WorldOccurrences)
        {
            const auto world = std::find_if(m_PreviewDocument.Worlds.begin(), m_PreviewDocument.Worlds.end(),
                [&](const auto& value) { return value.strWorldId == box.strWorldId; });
            auto& player = member.session.previewWorlds.at(box.strOccurrenceId);
            auto worldTargets = targets;
            worldTargets.bossAnchor = [&member](const std::string& archetype, const std::string& bone,
                CWorldSequencePlayer::PLAYER_ANCHOR& out, std::string& status)
            {
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
            { m_strStatus = player->Get_Status(); continue; }
            if (!player->Seek_InstanceToMs(world->strSequenceInstanceId, float(localMs - box.iStartMs), worldTargets))
                m_strStatus = player->Get_Status();
        }
        member.actor->Synchronize_WeaponPose();
        ANIMATION_MODEL_TARGET_VIEW weaponView;
        const bool hasWeapon = member.actor->Try_GetAnimationModelTarget(ANIMATION_BONE_TARGET::WEAPON, weaponView);
        if (localMs < 0.0 || localMs >= member.durationMs) Stop_Session(member.session);
        else Sample(member.session, m_PreviewDocument, member.pattern, sampleMs, m_bPreviewPaused,
            *member.actor->Get_Transform()->Get_WorldMatrixPtr(), model, hasWeapon ? &weaponView : nullptr);
    }
    Sample(m_PreviewSession, m_PreviewDocument, m_PreviewPattern,
        float(m_fPreviewClockMs), m_bPreviewPaused, m_PreviewPivot, nullptr);
}

void Client::CKoukuSaydonPresentationPlayer::Set_PreviewPivot(
    const float4x4_t& pivot, const std::shared_ptr<Engine::CModel>& model)
{
    m_PreviewPivot = pivot;
    m_PreviewModel = model;
    m_bPreviewPivotReady = true;
}

void Client::CKoukuSaydonPresentationPlayer::Sample_Preview(std::uint32_t clockMs,
    bool playing, bool paused, const float4x4_t& pivot, const std::shared_ptr<Engine::CModel>& model)
{
    if (!playing) { Stop_Preview(); return; }
    if (!m_bPreviewPlaying) return;
    Set_PreviewPivot(pivot, model);
    m_fPreviewClockMs = (std::min)(clockMs, m_iPreviewDurationMs);
    m_bPreviewPaused = paused;
    ANIMATION_MODEL_TARGET_VIEW weaponView;
    const bool hasWeapon = CAnimationTargetService::Resolve_Model() == model &&
        CAnimationTargetService::Resolve_ModelTarget(ANIMATION_BONE_TARGET::WEAPON, weaponView);
    Sample(m_PreviewSession, m_PreviewDocument, m_PreviewPattern, float(m_fPreviewClockMs),
        paused, pivot, model, hasWeapon ? &weaponView : nullptr);
    Refresh_SharedPresentation();
}

void Client::CKoukuSaydonPresentationPlayer::Pause_Preview(bool paused)
{
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
    for (auto& member : m_BundlePreviewMembers) prepare(member.session);
    if (Preview_IsBundle()) Sample_BundlePreview();
    // MainApp samples single-pattern WORLD at the new clock before recreating these cue handles.
    Refresh_SharedPresentation();
}

void Client::CKoukuSaydonPresentationPlayer::Stop_Preview()
{
    ++m_iPreviewGeneration;
    m_bModelReferencePreview = false;
    Release_BundlePreviewMembers(m_BundlePreviewMembers);
    m_PreviewBundleId.clear();
    Stop_Session(m_PreviewSession);
    m_bPreviewPlaying = false;
    m_bOwnPreviewClock = false;
    m_bPreviewPaused = false;
    m_bPreviewPivotReady = false;
    m_bColliderResourcePreview = false;
    m_PreviewModel.reset();
    Refresh_SharedPresentation();
}

void Client::CKoukuSaydonPresentationPlayer::Sync_MazeMark(
    CARD& mark, const std::string& asset, const float4x4_t& pivot)
{
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
    /* The bingo board. Both masks are room state the Server owns, so this
       only chooses which of the two authored decals sits on each painted
       cell and drops the ones the Server has cleared. */
    {
        const auto& board = CCombatHUDViewModel::Get().Get_BingoBoard();
        std::set<std::int32_t> liveBingo;
        for (std::int32_t cell = 0; cell < LostArk::Shared::KOUKU_BINGO_CELL_COUNT; ++cell)
        {
            const std::uint32_t bit = 1u << cell;
            if (0u == (board.iWhiteMask & bit)) continue;
            liveBingo.insert(cell);
            float4x4_t pivot;
            XMStoreFloat4x4(&pivot, XMMatrixTranslation(
                LostArk::Shared::Kouku_BingoCellCenterX(cell), .02f,
                LostArk::Shared::Kouku_BingoCellCenterZ(cell)));
            Sync_MazeMark(m_BingoMarks[cell],
                (0u != (board.iRedMask & bit)) ? "bingo.skull.red" : "bingo.skull.white",
                pivot);
        }
        for (auto i = m_BingoMarks.begin(); i != m_BingoMarks.end();)
        {
            if (liveBingo.contains(i->first)) { ++i; continue; }
            if (i->second.handle) CEffectV2Runtime::Stop_Group(i->second.handle);
            i = m_BingoMarks.erase(i);
        }
    }
    /* Bingo bombs. The Server owns both phases and their clock: a mark
       rides its carrier, and a planted bomb stays where that carrier was
       standing when the mark expired. Height and size live in the two
       authored documents, so the pivot here is only the ground point. */
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
            else if (LostArk::Shared::BINGO_BOMB_PHASE::PLANTED == bomb.ePhase)
            {
                XMStoreFloat4x4(&bombPivot, XMMatrixTranslation(
                    bomb.fPositionX, 0.f, bomb.fPositionZ));
                bombAsset = "bingo.bomb";
            }
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
    /* The bingo hammer. The Server owns the anchor and the phase window;
       this only interpolates that window against the replicated tick, so
       the sweep stays smooth between snapshots without the Client ever
       deciding where the hammer is. */
    {
        using namespace LostArk::Shared;
        const auto& hammer = CCombatHUDViewModel::Get().Get_BingoBoard().Hammer;
        const CLevel_KakulSaydonArena* hammerArena = CLevel_KakulSaydonArena::Get_Active();
        if (BINGO_HAMMER_PHASE::NONE == hammer.ePhase ||
            !Is_KoukuBingoHammerAnchor(hammer.iAnchor) ||
            hammer.iPhaseEndTick <= hammer.iPhaseStartTick ||
            nullptr == hammerArena)
        {
            if (m_BingoHammer.handle) CEffectV2Runtime::Stop_Group(m_BingoHammer.handle);
            m_BingoHammer = {};
        }
        else
        {
            const BINGO_HAMMER_PATH path = Kouku_BingoHammerPath(hammer.iAnchor);
            const std::uint32_t tick = hammerArena->Get_PresentationServerTick();
            const float window = float(hammer.iPhaseEndTick - hammer.iPhaseStartTick);
            const float elapsed = tick > hammer.iPhaseStartTick ?
                float(tick - hammer.iPhaseStartTick) : 0.f;
            const float ratio = (std::min)(1.f, elapsed / window);
            /* The model is authored hanging: head at the origin, chain up
               its own +Y. So the pivot is simply where the head goes. */
            float x = path.fStartX, z = path.fStartZ, y = KOUKU_BINGO_HAMMER_SKY_Y;
            if (BINGO_HAMMER_PHASE::DESCENDING == hammer.ePhase)
            {
                /* Smoothstep so the drop eases in and settles instead of
                   dropping at a constant rate. */
                const float fall = ratio * ratio * (3.f - 2.f * ratio);
                y = KOUKU_BINGO_HAMMER_SKY_Y * (1.f - fall);
            }
            float lean = 0.f;
            if (BINGO_HAMMER_PHASE::SWEEPING == hammer.ePhase)
            {
                y = 0.f;
                x = path.fStartX + (path.fEndX - path.fStartX) * ratio;
                z = path.fStartZ + (path.fEndZ - path.fStartZ) * ratio;
                /* The head stays down; only the lean changes, so the
                   hammer is flung rather than swung up. */
                const float t = (std::min)(1.f,
                    ratio / (std::max)(KOUKU_BINGO_HAMMER_LEAN_RATIO, 0.01f));
                lean = XMConvertToRadians(KOUKU_BINGO_HAMMER_LEAN_DEGREES) *
                    t * t * (3.f - 2.f * t);
            }
            /* The drum is a cylinder about the model's own X, so its two
               flat faces are what it strikes with. Yaw puts that X down the
               line so a face leads instead of the barrel's side, and the
               lean then tips the chain toward travel by rotating about the
               model's Z, which is the axis left across the line. */
            const float dx = path.fEndX - path.fStartX;
            const float dz = path.fEndZ - path.fStartZ;
            float4x4_t hammerPivot;
            XMStoreFloat4x4(&hammerPivot,
                XMMatrixRotationZ(-lean) *
                XMMatrixRotationY(std::atan2(-dz, dx)) *
                XMMatrixTranslation(x, y, z));
            Sync_MazeMark(m_BingoHammer, "bingo.hammer", hammerPivot);
        }
    }
    removeStale(m_MazePlayerMarks, livePlayers);
    removeStale(m_MazeTargetMarks, liveTargets);
    removeStale(m_MazeExits, liveExits);
}

void Client::CKoukuSaydonPresentationPlayer::Reset()
{
    m_LightPlayerPivots.clear();
    m_LightBossFollowers.clear();
    m_iProductReloadRunEpoch = 0u;
    Stop_Session(m_FearSession);
    m_FearSession.key.clear();
    m_strCompletedFearKey.clear();
    m_QueuedV1Effects.clear();
    for (auto& [id, session] : m_BossSessions) Stop_Session(session);
    m_BossSessions.clear();
    Stop_Session(m_ProductBundleSession);
    for (const auto& [id, card] : m_Cards)
        if (card.handle) CEffectV2Runtime::Stop_Group(card.handle);
    m_Cards.clear();
    for (const auto& [cell, mark] : m_BingoMarks)
        if (mark.handle) CEffectV2Runtime::Stop_Group(mark.handle);
    m_BingoMarks.clear();
    for (const auto& [slot, bomb] : m_BingoBombs)
        if (bomb.handle) CEffectV2Runtime::Stop_Group(bomb.handle);
    m_BingoBombs.clear();
    if (m_BingoHammer.handle) CEffectV2Runtime::Stop_Group(m_BingoHammer.handle);
    m_BingoHammer = {};
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
                (value.eKind == KIND::COLLIDER || value.eKind == KIND::EFFECT); });
        if (resource == m_PreviewDocument.PresentationResources.end()) return false;
        auto edited = *box;
        edited.PositionOffset = occurrence.PositionOffset;
        edited.RotationDegrees = occurrence.RotationDegrees;
        edited.Scale = occurrence.Scale;
        const auto active = session.rows.find(box->strOccurrenceId);
        if (active != session.rows.end() && active->second.hasPlacementAnchor &&
            !active->second.failed && !active->second.waitingForAnchor)
        {
            // Frozen rows retain their first anchor; following rows retain the
            // current sampled anchor and all prior particle birth transforms.
            auto placed = edited;
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
                    (std::max)(0.f, active->second.lastAge), Effect_V1TransformProvider(edited, pivot,
                        session.rootHistory, active->second.effectPivotHistory), true)))
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
        box->PositionOffset = occurrence.PositionOffset;
        box->RotationDegrees = occurrence.RotationDegrees;
        box->Scale = occurrence.Scale;
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
    if (m_bPreviewPlaying) draw(m_PreviewSession, true);
    for (const auto& member : m_BundlePreviewMembers) draw(member.session, true);
#endif
}

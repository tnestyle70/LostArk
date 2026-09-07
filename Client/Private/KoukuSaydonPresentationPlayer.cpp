#include "imgui.h"
#include "KoukuSaydonPresentationPlayer.h"

#include "ActionPresentationTimeline.h"
#include "Character.h"
#include "DataJson.h"
#include "EffectV2_Catalog.h"
#include "EffectV2_Object.h"
#include "EffectV2_Runtime.h"
#include "GameInstance.h"
#include "Level_KakulSaydonArena.h"
#include "LightResourceCatalog.h"
#include "Presentation_Manager.h"
#include "Model.h"
#include "Npc.h"
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
    resource.iDurationMs = UInt(row, "resourceDurationMs", 1u, MAX_TIMELINE_MS);
    resource.strShape = Text(row, "shape");
    resource.HalfExtents = Vector(row, "halfExtents", 0.001, 100000.0);
    resource.fRadiusM = Number(row, "radiusM", 0.001, 100000.0);
    resource.fHalfAngleDegrees = Number(row, "halfAngleDegrees", 0.001, 180.0);
    if ((resource.eKind == KIND::EFFECT && resource.strResourceKind != "GROUP" && resource.strResourceKind != "LEAF") ||
        (resource.strShape != "BOX" && resource.strShape != "SECTOR" && resource.strShape != "CIRCLE"))
        throw std::runtime_error("Invalid presentation resource type: " + resource.strResourceId);
    if (resource.eKind == KIND::EFFECT &&
        !CEffectV2Document::Is_ValidEffectId(resource.strAssetId))
        throw std::runtime_error("Invalid Effect V2 identity: " + resource.strAssetId);
    const auto stableLightId = [](const std::string& id)
    {
        return !id.empty() && id.size() <= 128u && id != "." && id != ".." &&
            std::all_of(id.begin(), id.end(), [](unsigned char c) {
                return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.'; });
    };
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
    box.iFadeInMs = UInt(row, "fadeInMs", 0u, box.iDurationMs);
    box.iFadeOutMs = UInt(row, "fadeOutMs", 0u, box.iDurationMs);
    if (box.iFadeInMs + box.iFadeOutMs > box.iDurationMs)
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
    if (box.strBone.size() > 128u) throw std::runtime_error("Presentation bone is too long.");
    if (row.Find("anchorKind")) box.strAnchorKind = Text(row, "anchorKind");
    if (row.Find("worldId")) box.strWorldId = Text(row, "worldId", true);
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
    return box;
}

std::uint32_t Pattern_Duration(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern)
{
    std::uint64_t duration = 0;
    for (const auto& stage : pattern.Stages) duration += stage.iDurationMs;
    for (const auto& row : pattern.PresentationOccurrences)
        duration = (std::max)(duration, std::uint64_t(row.iStartMs) + row.iDurationMs);
    for (const auto& row : pattern.SceneProfileOccurrences)
        duration = (std::max)(duration, std::uint64_t(row.iStartMs) + row.iDurationMs);
    return static_cast<std::uint32_t>((std::min)(duration, std::uint64_t(MAX_TIMELINE_MS)));
}

bool Make_Pivot(const OCCURRENCE& box, const float4x4_t& root,
    const std::shared_ptr<Engine::CModel>& model, float4x4_t& result)
{
    float4x4_t anchor = root;
    if (!box.strBone.empty())
    {
        if (!model || !model->Has_Bone(box.strBone.c_str())) return false;
        EFFECT_V2_TARGET_VIEW view;
        view.pModel = model;
        view.BoneRoot = root;
        view.YawBasis = root;
        if (!CEffectV2Object::Resolve_TargetPivot(view, box.strBone,
            CEffectV2Object::PIVOT_ROTATION::TARGET_YAW, anchor)) return false;
    }
    matrix_t basis = XMLoadFloat4x4(&anchor);
    for (size_t axis = 0; axis < 3u; ++axis)
    {
        if (XMVectorGetX(XMVector3LengthSq(basis.r[axis])) < 0.000001f) return false;
        basis.r[axis] = XMVector3Normalize(basis.r[axis]);
    }
    XMStoreFloat4x4(&result,
        XMMatrixScaling(float(box.Scale[0]), float(box.Scale[1]), float(box.Scale[2])) *
        XMMatrixRotationRollPitchYaw(XMConvertToRadians(float(box.RotationDegrees[0])),
            XMConvertToRadians(float(box.RotationDegrees[1])), XMConvertToRadians(float(box.RotationDegrees[2]))) *
        XMMatrixTranslation(float(box.PositionOffset[0]), float(box.PositionOffset[1]), float(box.PositionOffset[2])) * basis);
    return true;
}

HIT_AREA_SHAPE Collider_Wire(const RESOURCE& resource, const OCCURRENCE& box)
{
    HIT_AREA_SHAPE shape;
    if (resource.strShape == "BOX")
    {
        const double halfWidth = resource.HalfExtents[0] * box.Scale[0];
        const double halfLength = resource.HalfExtents[2] * box.Scale[2];
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
    const char* symbol = nullptr;
    switch (snapshot.eMechanicCardSymbol)
    {
    case MECHANIC_CARD_SYMBOL::HEART: symbol = "heart"; break;
    case MECHANIC_CARD_SYMBOL::SPADE: symbol = "spade"; break;
    case MECHANIC_CARD_SYMBOL::CLUB: symbol = "clober"; break;
    case MECHANIC_CARD_SYMBOL::DIAMOND: symbol = "dia"; break;
    default: return {};
    }
    const char* color = nullptr;
    switch (snapshot.eMechanicCardColor)
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
    const auto collect = [&](const SESSION& session, bool preview)
    {
        for (const auto& [id, row] : session.rows)
        {
            if (row.kind != KIND::LIGHT || row.failed || row.waitingForAnchor || row.lightWeight <= 0.f) continue;
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
                // No alive character is a temporary empty target set, never a sticky row failure.
                for (const auto& root : m_LightPlayerPivots)
                {
                    float4x4_t pivot;
                    if (Make_Pivot(row.lightBox, root, nullptr, pivot)) append(pivot);
                }
            }
            else append(row.pivot);
        }
    };
    for (const auto& [id, session] : m_BossSessions) collect(session, false);
    if (m_bPreviewPlaying) collect(m_PreviewSession, true);
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

bool Client::CKoukuSaydonPresentationPlayer::Reload_Product(std::string& status)
{
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
        (void)UInt(root, "sourceRevision", 1u, UINT32_MAX);
        const auto& patterns = Field(root, "patterns");
        if (!patterns.Is_Array() || patterns.Get_Array().size() > 4096u)
            throw std::runtime_error("Product patterns must be a bounded array.");
        std::map<std::string, PRODUCT_PATTERN> staged;
        std::size_t isolatedLights = 0u;
        for (const auto& value : patterns.Get_Array())
        {
            PRODUCT_PATTERN item;
            item.pattern.strPatternId = Text(value, "patternId");
            item.durationMs = UInt(value, "durationMs", 1u, MAX_TIMELINE_MS);
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
                    if (!kind || !kind->Is_String() || kind->Get_String() != "LIGHT") throw;
                    ++isolatedLights;
                    OutputDebugStringA((std::string("[KoukuSaydonPresentationPlayer] Isolated LIGHT row: ") + error.what() + "\n").c_str());
                }
            }
            const std::string key = item.pattern.strPatternId;
            if (!staged.emplace(key, std::move(item)).second)
                throw std::runtime_error("Duplicate Product pattern identity.");
        }
        // Only a fully staged replacement may stop the old running presentation.
        for (auto& [id, session] : m_BossSessions) Stop_Session(session);
        m_BossSessions.clear();
        m_Product = std::move(staged);
        m_MissingProductPatterns.clear();
        m_bProductLoaded = true;
        Refresh_SharedPresentation();
        status = "Loaded KoukuSaydon presentation for " + std::to_string(m_Product.size()) + " Product patterns.";
        if (isolatedLights) status += " Isolated invalid LIGHT rows: " + std::to_string(isolatedLights);
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

bool Client::CKoukuSaydonPresentationPlayer::Ensure_Effects()
{
    if (m_Effects && m_Effects->Is_Ready()) return true;
    if (m_bEffectsAttempted) return false;
    m_bEffectsAttempted = true;
    auto& catalog = CEffectV2Catalog::Get();
    auto snapshot = catalog.Get_Snapshot();
    if ((!snapshot || !snapshot->Is_Ready()) && !catalog.Reload_BossValtan(m_strStatus)) return false;
    snapshot = catalog.Get_Snapshot();
    if (!snapshot || !snapshot->Is_Ready()) return false;
    m_Effects = std::move(snapshot);
    return true;
}

void Client::CKoukuSaydonPresentationPlayer::Stop_Session(SESSION& session)
{
    for (auto& [id, row] : session.rows)
    {
        if (row.effectHandle) CEffectV2Runtime::Stop_Group(row.effectHandle);
        if (row.soundHandle) CGameInstance::Get().Stop_SoundCue(row.soundHandle);
    }
    session.rows.clear();
    session.lastClockMs = -1.f;
}

void Client::CKoukuSaydonPresentationPlayer::Sample(SESSION& session,
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, float clockMs, bool paused,
    const float4x4_t& pivot, const std::shared_ptr<Engine::CModel>& model)
{
    if (!std::isfinite(clockMs) || clockMs < 0.f) return;
    if (session.lastClockMs >= 0.f &&
        (clockMs < session.lastClockMs - 0.5f || clockMs > session.lastClockMs + 150.f))
        Stop_Session(session);
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
        row.assetId = resource.strAssetId;
        row.cameraOffset = { float(box.PositionOffset[0]), float(box.PositionOffset[1]), float(box.PositionOffset[2]) };
        if (resource.eKind == KIND::LIGHT)
        {
            row.lightBox = box;
            row.lightWeight = float(box.fBrightnessMultiplier);
            if (box.iFadeInMs) row.lightWeight *= (std::min)(1.f, (clockMs - box.iStartMs) / box.iFadeInMs);
            if (box.iFadeOutMs) row.lightWeight *= (std::min)(1.f, (box.iStartMs + box.iDurationMs - clockMs) / box.iFadeOutMs);
        }
        if ((inserted || box.bFollowBoss || row.waitingForAnchor) &&
            !(resource.eKind == KIND::LIGHT && box.strAnchorKind == "PLAYER"))
        {
            OCCURRENCE placedBox = box;
            float4x4_t anchor = pivot;
            auto anchorModel = model;
            if (resource.eKind == KIND::LIGHT && box.strAnchorKind == "MAP")
            { XMStoreFloat4x4(&anchor, XMMatrixIdentity()); anchorModel.reset(); }
            if (box.strAnchorKind == "WORLD")
            {
                const auto world = std::find_if(document.Worlds.begin(), document.Worlds.end(),
                    [&box](const auto& value) { return value.strWorldId == box.strWorldId; });
                const auto* level = CLevel_KakulSaydonArena::Get_Active();
                if (world == document.Worlds.end() || !level ||
                    !level->Try_GetCompositionWorldPivot(world->strSequenceInstanceId, anchor))
                {
                    // A WORLD may not have its first sampled pose yet. Retry
                    // Collider anchors next frame instead of hiding this box forever.
                    row.waitingForAnchor = resource.eKind == KIND::COLLIDER;
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
            if (!Make_Pivot(placedBox, anchor, anchorModel, row.pivot))
            {
                row.waitingForAnchor = resource.eKind == KIND::LIGHT;
                row.failed = !row.waitingForAnchor;
                m_strStatus = "Presentation bone/pivot is unavailable: " + box.strOccurrenceId;
                return;
            }
            if (row.waitingForAnchor)
                m_strStatus = "Collider WORLD anchor ready: " + box.strWorldId;
            row.waitingForAnchor = false;
            if (resource.eKind == KIND::COLLIDER) row.wire = Collider_Wire(resource, placedBox);
        }
        if (inserted)
        {
            switch (resource.eKind)
            {
            case KIND::EFFECT:
            {
                if (!Ensure_Effects()) { row.failed = true; break; }
                EFFECT_V2_GROUP_PLAYBACK_DESC playback;
                playback.PivotWorld = row.pivot;
                playback.fInitialAgeSeconds = age;
                playback.fDurationSeconds = box.iDurationMs / 1000.f;
                playback.fFadeInSeconds = box.iFadeInMs / 1000.f;
                playback.fFadeOutSeconds = box.iFadeOutMs / 1000.f;
                playback.fDissolveOutStart = float(box.fDissolveStart);
                playback.fDissolveOutEnd = float(box.fDissolveEnd);
                // This player is clocked by MainApp, including local previews.
                playback.bProductOwned = true;
                if (resource.strResourceKind == "GROUP")
                {
                    const auto* group = m_Effects->Find_Group(resource.strAssetId);
                    if (group) row.effectHandle = CEffectV2Runtime::Play_Group(*group,
                        m_Effects, playback, m_Device, m_Context);
                }
                else if (resource.strResourceKind == "LEAF")
                    row.effectHandle = CEffectV2Runtime::Play_Leaf(resource.strAssetId,
                        m_Effects, playback, m_Device, m_Context);
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
        if (row.effectHandle)
        {
            CEffectV2Runtime::Set_GroupPivot(row.effectHandle, row.pivot);
            CEffectV2Runtime::Set_GroupPaused(row.effectHandle, paused);
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
        if (!m_Profiles.Activate_Profile(m_strScenePrevious, status)) m_strStatus = status;
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
    const auto choose = [&](const SESSION& session)
    {
        const PLAYING_ROW* localScene = nullptr;
        const PLAYING_ROW* localCamera = nullptr;
        std::string localOwner;
        for (const auto& [id, row] : session.rows)
        {
            if (row.failed) continue;
            if (row.kind == KIND::SCENE_PROFILE && (!localScene || row.startMs >= localScene->startMs))
            {
                localScene = &row;
                localOwner = session.key + ":" + id;
            }
            if (row.kind == KIND::CAMERA && (!localCamera || row.startMs >= localCamera->startMs))
                localCamera = &row;
        }
        if (localScene) { scene = localScene; sceneOwner = std::move(localOwner); }
        if (localCamera) camera = localCamera;
    };
    // Stable entity/id order resolves overlapping presentation; preview owns the final choice.
    for (const auto& [id, session] : m_BossSessions) choose(session);
    if (m_bPreviewPlaying) choose(m_PreviewSession);
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
            if (level->Sample_CompositionCamera(camera->assetId, camera->lastAge, camera->cameraOffset))
                m_bCameraUsed = true;
            else m_strStatus = "Camera shot unavailable: " + camera->assetId;
        }
        else if (m_bCameraUsed)
        {
            level->Stop_CompositionCamera();
            m_bCameraUsed = false;
        }
    }
    else m_bCameraUsed = false;
}

void Client::CKoukuSaydonPresentationPlayer::Update(float dt,
    const std::vector<KOUKU_BOSS_PRESENTATION_VIEW>& bosses,
    const std::vector<KOUKU_CARD_PRESENTATION_VIEW>& players)
{
    if (!std::isfinite(dt) || dt < 0.f) return;
    m_LightPlayerPivots.clear();
    for (const auto& view : players)
    {
        const auto character = view.pCharacter.lock();
        if (view.Snapshot.iCurrentHp && character && character->Get_Transform())
            m_LightPlayerPivots.push_back(*character->Get_Transform()->Get_WorldMatrixPtr());
    }
    if (!m_bProductAttempted) { std::string status; (void)Reload_Product(status); }
    std::set<std::uint32_t> liveBosses;
    for (const auto& view : bosses)
    {
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
        const std::string key = std::to_string(id) + ":" + view.Snapshot.strPatternId + ":" +
            std::to_string(view.Snapshot.iPatternSequence) + ":" + std::to_string(view.Snapshot.iPatternStartTick);
        if (session.key != key) { Stop_Session(session); session.key = key; }
        // Interpolate between snapshots without rewinding/restarting effects every network tick.
        const float clock = session.lastClockMs < 0.f ? seconds * 1000.f :
            (std::max)(seconds * 1000.f, session.lastClockMs + dt * 1000.f);
        Sample(session, product->second.document, product->second.pattern,
            (std::min)(clock, float(product->second.durationMs)), false,
            *npc->Get_Transform()->Get_WorldMatrixPtr(), npc->Get_Model());
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
            if (Ensure_Effects())
            {
                if (const auto* source = m_Effects->Find_Group(asset))
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
                    card.handle = CEffectV2Runtime::Play_Group(group, m_Effects,
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
    if (m_bPreviewPlaying && m_bOwnPreviewClock && m_bPreviewPivotReady)
    {
        if (!m_bPreviewPaused) m_fPreviewClockMs += double(dt) * 1000.0;
        if (m_fPreviewClockMs >= m_iPreviewDurationMs)
        {
            if (m_bColliderResourcePreview)
            {
                m_fPreviewClockMs = m_iPreviewDurationMs - 1u;
                Pause_Preview(true);
            }
            else Stop_Preview();
        }
        // MainApp samples WORLD first, then this preview at the same clock.
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
    m_Effects.reset();
    m_bEffectsAttempted = false;
    status = "Presentation preview ready: " + pattern.strPatternId;
    m_strStatus = status;
    return true;
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
    Sample(m_PreviewSession, m_PreviewDocument, m_PreviewPattern, float(m_fPreviewClockMs),
        paused, pivot, model);
    Refresh_SharedPresentation();
}

void Client::CKoukuSaydonPresentationPlayer::Pause_Preview(bool paused)
{
    m_bPreviewPaused = paused;
    for (const auto& [id, row] : m_PreviewSession.rows)
    {
        if (row.effectHandle) CEffectV2Runtime::Set_GroupPaused(row.effectHandle, paused);
        if (row.soundHandle) CGameInstance::Get().Pause_SoundCue(row.soundHandle, paused);
    }
}

void Client::CKoukuSaydonPresentationPlayer::Seek_Preview(std::uint32_t clockMs)
{
    m_fPreviewClockMs = (std::min)(clockMs, m_iPreviewDurationMs);
    Stop_Session(m_PreviewSession);
    // MainApp samples WORLD at the new clock before recreating these cue handles.
    Refresh_SharedPresentation();
}

void Client::CKoukuSaydonPresentationPlayer::Stop_Preview()
{
    Stop_Session(m_PreviewSession);
    m_bPreviewPlaying = false;
    m_bOwnPreviewClock = false;
    m_bPreviewPaused = false;
    m_bPreviewPivotReady = false;
    m_bColliderResourcePreview = false;
    m_PreviewModel.reset();
    Refresh_SharedPresentation();
}

void Client::CKoukuSaydonPresentationPlayer::Reset()
{
    m_LightPlayerPivots.clear();
    for (auto& [id, session] : m_BossSessions) Stop_Session(session);
    m_BossSessions.clear();
    for (const auto& [id, card] : m_Cards)
        if (card.handle) CEffectV2Runtime::Stop_Group(card.handle);
    m_Cards.clear();
    m_ColliderDebugOverrides.clear();
    Stop_Preview();
    Restore_Scene();
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
#endif
}

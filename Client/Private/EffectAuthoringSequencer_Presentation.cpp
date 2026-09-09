#include "EffectAuthoringSequencer.h"
#include "GameInstance.h"
#include "HitAreaWire.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <set>

namespace Client
{
namespace
{
constexpr std::uint32_t PRESENTATION_MAX_MS = 600000u;
constexpr std::size_t PRESENTATION_MAX_ROWS = 256u;

bool Presentation_Text(const std::string& text, std::size_t maximum, bool empty = false)
{
    return (empty || !text.empty()) && text.size() <= maximum &&
        text.find('\0') == std::string::npos;
}

bool Presentation_Time(std::uint32_t start, std::uint32_t duration)
{
    return duration > 0u && duration <= PRESENTATION_MAX_MS &&
        start <= PRESENTATION_MAX_MS - duration;
}

bool Presentation_Vector(const float3_t& value, float minimum, float maximum)
{
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z) &&
        value.x >= minimum && value.x <= maximum && value.y >= minimum && value.y <= maximum &&
        value.z >= minimum && value.z <= maximum;
}

std::filesystem::path Sound_Path(const std::string& assetId)
{
    if (!Presentation_Text(assetId, 1024u) || !assetId.starts_with("Sound/") ||
        assetId.find(':') != std::string::npos || assetId.find('\\') != std::string::npos)
        return {};
    const auto relative = std::filesystem::path(std::u8string(assetId.begin(), assetId.end()));
    if (relative.is_absolute() || relative.has_root_path() ||
        std::any_of(relative.begin(), relative.end(), [](const auto& part) { return part == ".."; }))
        return {};
    const auto dot = assetId.find_last_of('.');
    if (dot == std::string::npos) return {};
    auto extension = assetId.substr(dot);
    std::transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char value) { return static_cast<char>(std::tolower(value)); });
    if (extension != ".wav" && extension != ".ogg" && extension != ".mp3") return {};
    const auto path = CRuntimeAssetRoot::Resolve(relative);
    std::error_code error;
    return !path.empty() && std::filesystem::is_regular_file(path, error) && !error ? path :
        std::filesystem::path{};
}

bool Collider_Resource(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& resource)
{
    if (resource.eKind != KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER ||
        resource.strColliderKind != "GEOMETRY" || !resource.strAssetId.empty() ||
        !CEffectV2Document::Is_ValidEffectId(resource.strResourceId) ||
        !Presentation_Text(resource.strDisplayName, 512u) ||
        !Presentation_Text(resource.strResourceKind, 128u, true) ||
        !Presentation_Text(resource.strDefaultAnchorKind, 128u, true) ||
        (resource.strShape != "BOX" && resource.strShape != "CIRCLE" && resource.strShape != "SECTOR") ||
        !Presentation_Time(0u, resource.iDurationMs) ||
        !std::isfinite(resource.fRadiusM) || resource.fRadiusM < .001 || resource.fRadiusM > 10000.0 ||
        !std::isfinite(resource.fHalfAngleDegrees) || resource.fHalfAngleDegrees < .001 ||
        resource.fHalfAngleDegrees > 180.0)
        return false;
    return std::all_of(resource.HalfExtents.begin(), resource.HalfExtents.end(), [](double value)
        { return std::isfinite(value) && value >= .001 && value <= 10000.0; });
}

// The composition player's geometry mapping is retained: meters become the
// existing hit-area wire's centimetres; BOX height stays a presentation volume.
HIT_AREA_SHAPE Collider_Wire(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& resource,
    const float3_t& scale)
{
    HIT_AREA_SHAPE shape;
    if (resource.strShape == "BOX")
    {
        const double halfWidth = resource.HalfExtents[0] * scale.x;
        const double halfLength = resource.HalfExtents[2] * scale.z;
        shape.fBoxHalfHeightM = static_cast<float>(resource.HalfExtents[1] * scale.y);
        shape.iAreaType = 2;
        shape.iAreaRange = static_cast<int32_t>((std::min)(halfLength * 200.0, 1000000000.0));
        shape.iAreaAngle = static_cast<int32_t>((std::min)(halfWidth * 200.0, 1000000000.0));
        shape.iAreaOffsetX = -shape.iAreaRange / 2;
    }
    else
    {
        shape.iAreaType = resource.strShape == "CIRCLE" ? 1 : 3;
        shape.iAreaRange = static_cast<int32_t>((std::min)(
            resource.fRadiusM * (std::max)(scale.x, scale.z) * 100.0, 1000000000.0));
        shape.iAreaAngle = static_cast<int32_t>(resource.fHalfAngleDegrees * 2.0);
    }
    return shape;
}
}

bool CEffectAuthoringSequencer::Validate_SoundRows(const std::vector<SOUND_ROW>& rows)
{
    if (rows.size() > PRESENTATION_MAX_ROWS)
    { m_Status = "A sequence supports at most 256 Sound occurrences."; return false; }
    std::set<std::string> ids;
    for (const auto& row : rows)
    {
        if (!CEffectV2Document::Is_ValidEffectId(row.id) || !ids.insert(row.id).second ||
            !Presentation_Text(row.label, 512u, true) || !Presentation_Time(row.startMs, row.durationMs) ||
            row.sourceStartMs > PRESENTATION_MAX_MS - row.durationMs ||
            !std::isfinite(row.volume) || row.volume < 0.f || row.volume > 4.f)
        { m_Status = "Invalid Sound identity, timing, source trim or volume: " + row.id; return false; }
        const auto path = Sound_Path(row.assetId);
        std::uint32_t sourceDuration = 0u;
        if (path.empty() || !CGameInstance::Get().Get_SoundDurationMs(path.wstring(), sourceDuration))
        { m_Status = "Sound asset is missing, unsafe or cannot be decoded: " + row.assetId; return false; }
        if (row.sourceStartMs >= sourceDuration)
        { m_Status = "Sound source trim starts at or beyond its natural end: " + row.id; return false; }
    }
    return true;
}

bool CEffectAuthoringSequencer::Validate_ColliderRows(const std::vector<COLLIDER_ROW>& rows)
{
    if (rows.size() > PRESENTATION_MAX_ROWS)
    { m_Status = "A sequence supports at most 256 Collider occurrences."; return false; }
    std::set<std::string> ids;
    for (const auto& row : rows)
        if (!CEffectV2Document::Is_ValidEffectId(row.id) || !ids.insert(row.id).second ||
            !Presentation_Text(row.label, 512u, true) || !Collider_Resource(row.resource) ||
            !Presentation_Time(row.startMs, row.durationMs) ||
            !Presentation_Vector(row.offset, -100000.f, 100000.f) ||
            !Presentation_Vector(row.rotation, -100000.f, 100000.f) ||
            !Presentation_Vector(row.scale, .001f, 10000.f) ||
            !Presentation_Text(row.anchorSlotId, 128u))
        { m_Status = "Invalid geometry Collider resource, transform, timing or anchor: " + row.id; return false; }
    // Load validates named anchors against its staged model selection. This
    // structural check must not accidentally use the previously selected model.
    return true;
}

bool CEffectAuthoringSequencer::Append_Sound(const std::string& assetId, std::uint32_t durationMs)
{
    SOUND_ROW row;
    std::uint32_t ordinal = 1u;
    do { row.id = "sound.occurrence." + std::to_string(ordinal++); }
    while (std::any_of(m_Sounds.begin(), m_Sounds.end(), [&](const auto& value) { return value.id == row.id; }));
    row.assetId = assetId;
    row.label = assetId.substr(assetId.find_last_of('/') + 1u);
    row.startMs = ClockMs();
    if (!durationMs)
    {
        const auto path = Sound_Path(assetId);
        if (path.empty() || !CGameInstance::Get().Get_SoundDurationMs(path.wstring(), durationMs))
        { m_Status = "Sound source duration is unavailable: " + assetId; return false; }
    }
    row.durationMs = durationMs;
    auto staged = m_Sounds; staged.push_back(row);
    if (!Validate_SoundRows(staged)) return false;
    auto& sound = CGameInstance::Get();
    if (m_Active)
    {
        const auto path = Sound_Path(assetId);
        if (path.empty())
        { m_Status = "Sound source preparation failed; the current timeline is preserved."; return false; }
        row.handle = sound.Play_SoundCue(path.wstring(), row.volume, row.sourceStartMs, true);
        if (!row.handle)
        { m_Status = "Sound channel preparation failed; the current timeline is preserved."; return false; }
        row.sampledAge = row.sourceStartMs;
    }
    if (!Commit_TransientPreview())
    {
        if (row.handle) sound.Stop_SoundCue(row.handle);
        return false;
    }
    m_Sounds.push_back(std::move(row));
    if (m_Sounds.back().handle) sound.Pause_SoundCue(m_Sounds.back().handle, m_Paused);
    Select_TimelineRow(TRACK_KIND::SOUND, m_Sounds.back().id);
    m_Dirty = true;
    Preserve_ClockDuringAuthoring();
    m_Status = "Appended Sound at " + std::to_string(ClockMs()) + " ms.";
    return true;
}

bool CEffectAuthoringSequencer::Append_Collider(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& resource)
{
    COLLIDER_ROW row;
    std::uint32_t ordinal = 1u;
    do { row.id = "collider.occurrence." + std::to_string(ordinal++); }
    while (std::any_of(m_Colliders.begin(), m_Colliders.end(), [&](const auto& value) { return value.id == row.id; }));
    row.resource = resource; row.label = resource.strDisplayName;
    row.startMs = ClockMs(); row.durationMs = resource.iDurationMs;
    row.anchorSlotId = m_DefaultAnchorSlotId;
    auto staged = m_Colliders; staged.push_back(row);
    if (!Validate_ColliderRows(staged) || !Validate_Anchor(row.anchorSlotId, m_ModelRoot, m_UseKouku)) return false;
    if (!Commit_TransientPreview()) return false;
    m_Colliders.push_back(std::move(row));
    Select_TimelineRow(TRACK_KIND::COLLIDER, m_Colliders.back().id);
    m_Dirty = true;
    Preserve_ClockDuringAuthoring();
    m_Status = "Appended Collider at " + std::to_string(ClockMs()) + " ms.";
    return true;
}

void CEffectAuthoringSequencer::Stop_Sounds()
{
    for (auto& row : m_Sounds)
    {
        if (row.handle) CGameInstance::Get().Stop_SoundCue(row.handle);
        row.handle = 0u; row.sampledAge = -1;
    }
}

bool CEffectAuthoringSequencer::Sample_Sounds(bool forceSeek)
{
    if (!m_Active || m_Transient) { Stop_Sounds(); return true; }
    const auto clock = ClockMs();
    auto& sound = CGameInstance::Get();
    for (auto& row : m_Sounds)
    {
        if (row.muted || clock < row.startMs || std::uint64_t(clock) >= std::uint64_t(row.startMs) + row.durationMs)
        {
            if (row.handle) sound.Stop_SoundCue(row.handle);
            row.handle = 0u; row.sampledAge = -1;
            continue;
        }
        const auto sourceAge = row.sourceStartMs + (clock - row.startMs);
        const bool reposition = forceSeek || (row.sampledAge >= 0 && sourceAge < row.sampledAge);
        if (row.sampledAge >= 0 && !reposition)
        {
            // A finished FMOD channel is deliberately not reborn each tick.
            // Only a new interval, Restart/Loop or explicit Seek can recreate it.
            if (row.handle) sound.Pause_SoundCue(row.handle, m_Paused);
            row.sampledAge = sourceAge;
            continue;
        }
        const auto path = Sound_Path(row.assetId);
        std::uint32_t sourceDuration = 0u;
        if (path.empty() || !sound.Get_SoundDurationMs(path.wstring(), sourceDuration))
        { m_Status = "Sound occurrence cannot resolve its source: " + row.id; return false; }
        if (sourceAge >= sourceDuration)
        {
            if (row.handle) sound.Stop_SoundCue(row.handle);
            row.handle = 0u; row.sampledAge = sourceAge;
            continue;
        }
        if (row.handle && sound.Is_SoundCueActive(row.handle))
        {
            sound.Pause_SoundCue(row.handle, true);
            sound.Seek_SoundCue(row.handle, sourceAge);
            sound.Pause_SoundCue(row.handle, m_Paused);
        }
        else
        {
            if (row.handle) sound.Stop_SoundCue(row.handle);
            row.handle = sound.Play_SoundCue(path.wstring(), row.volume, sourceAge, m_Paused);
            if (!row.handle)
            { m_Status = "Sound occurrence could not start: " + row.id; return false; }
        }
        row.sampledAge = sourceAge;
    }
    return true;
}

void CEffectAuthoringSequencer::Render_Colliders()
{
    if (!m_Active || m_Transient) return;
    float4x4_t root;
    if (!Resolve_Root(root)) return;
    const auto clock = ClockMs();
    for (const auto& row : m_Colliders)
    {
        if (row.muted || !row.debugRender || clock < row.startMs ||
            std::uint64_t(clock) >= std::uint64_t(row.startMs) + row.durationMs) continue;
        EFFECT_ROW anchorRow; anchorRow.anchorSlotId = row.anchorSlotId;
        float4x4_t anchor;
        if (!Resolve_RowPivot(anchorRow, root, anchor)) continue;
        matrix_t basis = XMLoadFloat4x4(&anchor);
        bool valid = !XMMatrixIsNaN(basis) && !XMMatrixIsInfinite(basis);
        for (std::size_t axis = 0; valid && axis < 3u; ++axis)
        {
            const auto length = XMVectorGetX(XMVector3LengthSq(basis.r[axis]));
            valid = std::isfinite(length) && length >= .000001f;
            if (valid) basis.r[axis] = XMVectorSetW(XMVector3Normalize(basis.r[axis]), 0.f);
        }
        if (!valid) { m_Status = "Collider anchor has a singular transform: " + row.id; continue; }
        // Scale is already baked into Collider_Wire. Keep the occurrence's
        // rotation/offset and sampled anchor without scaling dimensions twice.
        const matrix_t placed = XMMatrixRotationRollPitchYaw(XMConvertToRadians(row.rotation.x),
            XMConvertToRadians(row.rotation.y), XMConvertToRadians(row.rotation.z)) *
            XMMatrixTranslation(row.offset.x, row.offset.y, row.offset.z) * basis;
        if (row.resource.strShape != "BOX" &&
            XMVectorGetX(XMVector3LengthSq(XMVectorSetY(placed.r[2], 0.f))) < 1e-12f)
        { m_Status = "Collider footprint needs a nonvertical forward axis: " + row.id; continue; }
        float4x4_t world; XMStoreFloat4x4(&world, placed);
        // Preview geometry only; no collision object, damage or Server state is created.
        CHitAreaWire::Draw(world, Collider_Wire(row.resource, row.scale), 0xff40dfff);
    }
}
}

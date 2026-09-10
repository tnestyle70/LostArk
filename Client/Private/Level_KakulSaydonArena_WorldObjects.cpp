#include "Level_KakulSaydonArena.h"
#include "Character.h"
#include "Npc.h"
#include "CombatHUDViewModel.h"
#include "DeployPropObject.h"
#include "KoukuSaydonPresentationPlayer.h"
#include "Transform.h"
#include "WorldGameplayDocument.h"
#include <cmath>
#include <algorithm>
#include <set>

using namespace Client;

namespace
{
    constexpr std::uint32_t MARIO_BOMB_INTERVAL_MS = 4000u;
    constexpr float MARIO_BOMB_SPEED_MPS = 3.f;
    constexpr float MARIO_BOMB_LOW_BOTTOM_M = .05f;
    constexpr float MARIO_BOMB_HIGH_BOTTOM_M = .90f;
}

bool_t CLevel_KakulSaydonArena::Ready_MarioBombPresentation(std::string& status)
{
    struct BINDING { std::uint8_t stage; const char* marker; const char* arrival; const char* exit; };
    static constexpr BINDING bindings[] = {
        {2u, "Mario2_Boom", "Mario2_go", "Mario2_Trigger_2"},
        {2u, "Mario2_Boom_1", "Mario2_Trigger_2", "Mario2_Trigger_4"},
        {2u, "Mario2_Boom_2", "Mario2_Trigger_4", "Mario2_Trigger_7"},
        {3u, "Mario3_Boom", "Mario3_Trigger_4", "Mario3_Trigger_5"},
        {3u, "Mario3_Boom_1", "Mario3_Trigger_4", "Mario3_Trigger_5"},
        {3u, "Mario3_Boom_2", "Mario3_Trigger_8", "Mario3_Trigger_10"},
        {4u, "Mario4_Boom", "Mario4_Tigger_6", "Mario4_Tigger_7"}
    };
    CWorldGameplayDocument world;
    const auto path = CMapAssetCatalog::Get_MapDataRoot().parent_path() / "World" /
        "LV_LUT_MIDNIGHTC_ED.viewer.world.json";
    if (!world.Load(path, "LV_LUT_MIDNIGHTC_ED", status)) return false;
    const auto& source = m_SequencePlayer.Get_Document();
    const auto* base = source.Find_Instance("world.object.instance.mario.clown_face_ball.show");
    const auto* motion = base ? source.Find_Template(base->templateId) : nullptr;
    if (!base || !motion || base->bindings.size() != 1u ||
        base->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ||
        motion->tracks.size() != 1u || motion->tracks.front().keys.empty() ||
        !motion->animationTracks.empty() || !motion->effectTracks.empty())
    { status = "Mario clown face ball show motion is missing or incompatible."; return false; }
    const auto* resource = source.Find_ObjectResource(base->bindings.front().targetId);
    if (!resource || resource->animated || !resource->sequenceInstanceId.empty())
    { status = "Mario clown face ball model resource is missing or incompatible."; return false; }

    CWorldSequenceDocument stagedDocument;
    stagedDocument.Reset_Empty("LV_LUT_MIDNIGHTC_ED");
    auto copiedResource = *resource;
    copiedResource.defaultMotionInstanceId.clear();
    stagedDocument.Get_ObjectResources().push_back(std::move(copiedResource));
    std::vector<MARIO_BOMB_EMITTER> stagedEmitters;
    for (const auto& binding : bindings)
    {
        const auto* marker = world.Find(binding.marker);
        const auto* arrival = world.Find(binding.arrival);
        const auto* exit = world.Find(binding.exit);
        if (!marker || !arrival || !exit || marker->eKind != WORLD_PLACEMENT_KIND::TRIGGER_BOX ||
            arrival->eKind != WORLD_PLACEMENT_KIND::TRIGGER_BOX || exit->eKind != WORLD_PLACEMENT_KIND::TRIGGER_BOX ||
            arrival->triggerEvents.size() != 1u ||
            arrival->triggerEvents.front().eKind != WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER)
        { status = std::string("Mario bomb published marker/lane is missing: ") + binding.marker; return false; }
        const float3_t start = marker->position;
        const auto& laneStart = arrival->triggerEvents.front().targetPosition;
        const auto distance = [&start](const float3_t& p)
        { return std::hypot(double(p.x) - start.x, double(p.z) - start.z); };
        const auto& finish = distance(laneStart) > distance(exit->position) ? laneStart : exit->position;
        const double length = distance(finish);
        if (!std::isfinite(length) || length < .1 || length > 200.)
        { status = std::string("Mario bomb lane length is invalid: ") + binding.marker; return false; }

        MARIO_BOMB_EMITTER emitter;
        emitter.stage = binding.stage;
        // Stable marker identity, never vector order, supplies the random sequence and launch phase.
        emitter.seed = 2166136261u;
        for (const char* c = binding.marker; *c; ++c)
            emitter.seed = (emitter.seed ^ static_cast<unsigned char>(*c)) * 16777619u;
        emitter.phaseMs = emitter.seed % MARIO_BOMB_INTERVAL_MS;
        emitter.durationMs = static_cast<std::uint32_t>(std::ceil(length / MARIO_BOMB_SPEED_MPS * 1000.));
        const auto slotCount = (emitter.durationMs + MARIO_BOMB_INTERVAL_MS - 1u) / MARIO_BOMB_INTERVAL_MS;
        if (!slotCount || slotCount > 32u)
        { status = std::string("Mario bomb slot budget exceeded: ") + binding.marker; return false; }
        WORLD_SEQUENCE_TEMPLATE flight;
        flight.sequenceId = std::string("sequence.mario.bomb.") + binding.marker;
        flight.displayName = std::string(binding.marker) + " / flying clown bomb";
        flight.category = "WorldObject";
        flight.durationMs = emitter.durationMs;
        flight.interpolation = WORLD_SEQUENCE_INTERPOLATION::LINEAR;
        WORLD_SEQUENCE_TRACK track;
        track.slotId = base->bindings.front().slotId;
        auto first = motion->tracks.front().keys.front();
        first.timeMs = 0u;
        first.visible = true;
        // ClownFaceBall's nose points along cooked-model +X, not +Z.
        // Keep world-up and turn that face into the actual flight direction;
        // the Mario side camera then sees a profile on either travel side.
        const float flightYaw = static_cast<float>(std::atan2(
            -(double(finish.z) - start.z), double(finish.x) - start.x));
        const vector_t flightRotation = XMQuaternionRotationAxis(
            XMVectorSet(0.f, 1.f, 0.f, 0.f), flightYaw);
        XMStoreFloat4(&first.rotationQuaternion, flightRotation);
        // The show key recenters this asymmetric mesh at its bottom center.
        // Rotate that correction too so all headings share the same launch line.
        XMStoreFloat3(&first.positionOffset, XMVector3Rotate(
            XMLoadFloat3(&first.positionOffset), flightRotation));
        auto last = first;
        last.timeMs = emitter.durationMs;
        last.positionOffset.x += finish.x - start.x;
        last.positionOffset.z += finish.z - start.z;
        track.keys = {first, last};
        flight.tracks.push_back(std::move(track));
        stagedDocument.Get_Templates().push_back(flight);
        for (std::uint32_t slot = 0; slot < slotCount; ++slot)
        {
            WORLD_SEQUENCE_INSTANCE instance;
            instance.instanceId = std::string("world.object.instance.mario.bomb.") + binding.marker + ".slot" + std::to_string(slot);
            instance.templateId = flight.sequenceId;
            instance.position = start;
            instance.bindings = base->bindings;
            emitter.slots.push_back(instance.instanceId);
            emitter.births.push_back(-1);
            stagedDocument.Get_Instances().push_back(std::move(instance));
        }
        stagedEmitters.push_back(std::move(emitter));
    }
    auto stagedPlayer = std::make_unique<CWorldSequencePlayer>();
    const auto targets = Make_WorldSequenceTargets();
    if (!stagedPlayer->Set_Document(stagedDocument, targets, status)) return false;
    for (const auto& emitter : stagedEmitters)
        if (!stagedPlayer->Prepare_InstanceResources(emitter.slots.front(), targets))
        { status = stagedPlayer->Get_Status(); return false; }
    if (m_pMarioBombPlayer) m_pMarioBombPlayer->Stop_All(targets, true);
    m_pMarioBombPlayer = std::move(stagedPlayer);
    m_MarioBombEmitters = std::move(stagedEmitters);
    status = "7 Mario bomb markers ready; 4000ms interval, 3m/s, face follows flight, no damage.";
    OutputDebugStringA(("[MarioBomb] " + status + "\n").c_str());
    return true;
}

void CLevel_KakulSaydonArena::Update_MarioBombPresentation(const f32_t timeDelta)
{
    if (!std::isfinite(timeDelta) || timeDelta < 0.f) return;
    const auto& player = CCombatHUDViewModel::Get().Get_Player();
    const std::uint8_t stage = player.iCurrentHp && player.iMarioStage >= 2u && player.iMarioStage <= 4u ?
        player.iMarioStage : 0u;
    const auto tick = m_Replication.Get_LastServerTick();
    if (tick != m_iMarioBombSnapshotTick)
    { m_iMarioBombSnapshotTick = tick; m_fMarioBombSnapshotSeconds = 0.f; }
    else m_fMarioBombSnapshotSeconds = (std::min)(.1f, m_fMarioBombSnapshotSeconds + timeDelta);
    const double clockMs = double(tick) * (1000. / 30.) + double(m_fMarioBombSnapshotSeconds) * 1000.;
    const auto targets = Make_WorldSequenceTargets();
    if (stage != m_iMarioBombStage || clockMs < m_fMarioBombStageStartMs)
    {
        if (m_pMarioBombPlayer) m_pMarioBombPlayer->Stop_All(targets, true);
        for (auto& emitter : m_MarioBombEmitters)
        { std::fill(emitter.births.begin(), emitter.births.end(), -1); emitter.failed = false; }
        m_iMarioBombStage = stage;
        m_fMarioBombStageStartMs = clockMs;
        m_bMarioBombLoadAttempted = false;
    }
    if (!stage) return;
    if (!m_pMarioBombPlayer)
    {
        if (m_bMarioBombLoadAttempted) return;
        m_bMarioBombLoadAttempted = true;
        std::string status;
        if (!Ready_MarioBombPresentation(status))
        { OutputDebugStringA(("[MarioBomb] " + status + "\n").c_str()); return; }
    }
    for (auto& emitter : m_MarioBombEmitters)
    {
        if (emitter.stage != stage || emitter.failed) continue;
        const auto latest = static_cast<std::int64_t>(std::floor((clockMs - emitter.phaseMs) / MARIO_BOMB_INTERVAL_MS));
        const auto count = static_cast<std::int64_t>(emitter.slots.size());
        for (std::int64_t slot = 0; slot < count; ++slot)
        {
            const auto& id = emitter.slots[static_cast<size_t>(slot)];
            const auto birth = latest < slot ? -1 : latest - (latest - slot) % count;
            const double birthMs = double(birth) * MARIO_BOMB_INTERVAL_MS + emitter.phaseMs;
            const double age = clockMs - birthMs;
            if (birth < 0 || birthMs < m_fMarioBombStageStartMs || age < 0. || age >= emitter.durationMs)
            {
                if (m_pMarioBombPlayer->Is_Playing(id)) m_pMarioBombPlayer->Stop_Instance(id, targets, true);
                emitter.births[static_cast<size_t>(slot)] = -1;
                continue;
            }
            if (emitter.births[static_cast<size_t>(slot)] != birth)
            {
                m_pMarioBombPlayer->Stop_Instance(id, targets, true);
                std::uint32_t random = emitter.seed ^ (static_cast<std::uint32_t>(birth) * 0x9e3779b9u);
                random ^= random << 13; random ^= random >> 17; random ^= random << 5;
                const float3_t heightOffset{0.f, (random & 1u) ? MARIO_BOMB_HIGH_BOTTOM_M : MARIO_BOMB_LOW_BOTTOM_M, 0.f};
                if (!m_pMarioBombPlayer->Play(id, targets, 1.f, heightOffset))
                { emitter.failed = true; break; }
                emitter.births[static_cast<size_t>(slot)] = birth;
            }
            if (!m_pMarioBombPlayer->Seek_InstanceToMs(id, static_cast<float>(age), targets))
            { emitter.failed = true; break; }
        }
        if (emitter.failed)
        {
            const auto status = m_pMarioBombPlayer->Get_Status();
            for (const auto& id : emitter.slots) m_pMarioBombPlayer->Stop_Instance(id, targets, true);
            OutputDebugStringA(("[MarioBomb] " + emitter.slots.front() + ": " + status + "\n").c_str());
        }
    }
}

void CLevel_KakulSaydonArena::Update_MarioLayoutPresentation()
{
    const auto& player = CCombatHUDViewModel::Get().Get_Player();
    std::string desired;
    if (player.iCurrentHp && player.iMarioStage >= 1u && player.iMarioStage <= 4u &&
        player.iMarioLayoutVariant >= 1u && player.iMarioLayoutVariant <= 3u)
        desired = "world.sequence.instance.mario" + std::to_string(player.iMarioStage) +
            ".source.layout" + std::to_string(player.iMarioLayoutVariant);
    const auto targets = Make_WorldSequenceTargets();
    if (desired != m_strMarioLayoutInstance)
    {
        if (!m_strMarioLayoutInstance.empty())
            m_SequencePlayer.Stop_Instance(m_strMarioLayoutInstance, targets, true);
        m_strMarioLayoutInstance = desired;
        m_bMarioLayoutFailed = false;
        m_bMarioLayoutStarted = false;
    }
    if (desired.empty() || m_bMarioLayoutFailed || m_bMarioLayoutStarted) return;
    if (!m_SequencePlayer.Play(desired, targets))
    {
        m_SequencePlayer.Stop_Instance(desired, targets, true);
        m_bMarioLayoutFailed = true;
        OutputDebugStringA(("[MarioLayout] " + desired + ": " + m_SequencePlayer.Get_Status() + "\n").c_str());
    }
    else m_bMarioLayoutStarted = true;
}

void CLevel_KakulSaydonArena::Update_MarioBallBouncePresentation(const f32_t timeDelta)
{
    Update_MarioLayoutPresentation();
    static const std::string instanceId = "world.sequence.instance.mario.striped_ball.bounce";
    const auto stage = CCombatHUDViewModel::Get().Get_Player().iMarioStage;
    if (stage < 1u || stage > 4u)
    {
        if (m_bMarioBallBounceRunning)
            m_SequencePlayer.Stop_Instance(instanceId, Make_WorldSequenceTargets(), true);
        m_bMarioBallBounceRunning = false;
        m_bMarioBallBounceFailed = false;
        m_iMarioBallBounceSnapshotTick = 0u;
        m_fMarioBallBounceSnapshotSeconds = 0.f;
        return;
    }
    if (m_bMarioBallBounceFailed || !std::isfinite(timeDelta) || timeDelta < 0.f) return;
    const auto* instance = m_SequencePlayer.Get_Document().Find_Instance(instanceId);
    const auto* sequence = instance ? m_SequencePlayer.Get_Document().Find_Template(instance->templateId) : nullptr;
    if (!instance || !sequence || !instance->enabled)
    {
        if (m_bMarioBallBounceRunning)
            m_SequencePlayer.Stop_Instance(instanceId, Make_WorldSequenceTargets(), true);
        m_bMarioBallBounceRunning = false;
        return;
    }
    const auto tick = m_Replication.Get_LastServerTick();
    if (tick != m_iMarioBallBounceSnapshotTick)
    {
        m_iMarioBallBounceSnapshotTick = tick;
        m_fMarioBallBounceSnapshotSeconds = 0.f;
    }
    else m_fMarioBallBounceSnapshotSeconds = (std::min)(.1f, m_fMarioBallBounceSnapshotSeconds + timeDelta);
    const double periodMs = static_cast<double>(sequence->durationMs) / instance->playbackSpeed;
    if (!std::isfinite(periodMs) || periodMs <= 0.) return;
    // Sample authored placement-relative tracks; never accumulate displacement.
    // Per-track phase keys share one clock, so neighbours rise and fall out of phase.
    const double clockMs = static_cast<double>(tick) * (1000. / 30.) +
        static_cast<double>(m_fMarioBallBounceSnapshotSeconds) * 1000.;
    const f32_t elapsedMs = static_cast<f32_t>(instance->startDelayMs + std::fmod(clockMs, periodMs));
    const auto targets = Make_WorldSequenceTargets();
    if ((!m_SequencePlayer.Is_Playing(instanceId) && !m_SequencePlayer.Play(instanceId, targets)) ||
        !m_SequencePlayer.Seek_InstanceToMs(instanceId, elapsedMs, targets))
    {
        const auto failure = m_SequencePlayer.Get_Status();
        m_SequencePlayer.Stop_Instance(instanceId, targets, true);
        m_bMarioBallBounceRunning = false;
        m_bMarioBallBounceFailed = true;
        OutputDebugStringA(("[MarioBallBounce] " + failure + "\n").c_str());
        return;
    }
    m_bMarioBallBounceRunning = true;
}

CWorldSequencePlayer::TARGET_SET CLevel_KakulSaydonArena::Make_WorldSequenceTargets()
{
    CWorldSequencePlayer::TARGET_SET targets;
    targets.levelIndex = ETOUI(LEVEL::KAKULSAYDON_ARENA);
    targets.pCatalog = &m_MapRuntime.Get_Catalog();
    targets.pPlacements = &m_MapRuntime.Get_MutablePlacements();
    targets.pDeployRuntime = &m_DeployRuntime;
    targets.device = m_pDevice;
    targets.context = m_pContext;
    targets.playerAnchors = [this]()
    {
        std::vector<CWorldSequencePlayer::PLAYER_ANCHOR> anchors;
        std::vector<KOUKU_BOSS_PRESENTATION_VIEW> bosses;
        std::vector<KOUKU_CARD_PRESENTATION_VIEW> players;
        m_Replication.Collect_KoukuPresentationViews(bosses, players);
        for (const auto& player : players)
        {
            const auto character = player.pCharacter.lock();
            if (!player.Snapshot.iCurrentHp || !character || !character->Get_Transform()) continue;
            anchors.push_back({player.Snapshot.iNetEntityId, *character->Get_Transform()->Get_WorldMatrixPtr()});
        }
        return anchors;
    };
    targets.bossAnchor = [this](const std::string& archetype, const std::string& bone,
        CWorldSequencePlayer::PLAYER_ANCHOR& out, std::string& status)
    {
        std::vector<KOUKU_BOSS_PRESENTATION_VIEW> bosses;
        std::vector<KOUKU_CARD_PRESENTATION_VIEW> players;
        m_Replication.Collect_KoukuPresentationViews(bosses, players);
        const KOUKU_BOSS_PRESENTATION_VIEW* selected = nullptr;
        for (const auto& boss : bosses)
        {
            if (boss.strArchetypeId != archetype || boss.iOwnerBossNetEntityId || !boss.Snapshot.iCurrentHp) continue;
            if (selected) { status = "World Object Boss anchor is ambiguous: " + archetype; return false; }
            selected = &boss;
        }
        const auto actor = selected ? selected->pNpc.lock() : nullptr;
        if (!actor || !actor->Get_Transform())
        { status = "World Object Boss anchor is waiting for its living replicated actor: " + archetype; return false; }
        out.entityId = selected->Snapshot.iNetEntityId;
        return CWorldSequencePlayer::Resolve_BossBoneAnchor(actor->Get_Model(),
            *actor->Get_Transform()->Get_WorldMatrixPtr(), bone, out, status);
    };
    return targets;
}

void CLevel_KakulSaydonArena::Get_WorldObjectValidationTargets(
    WORLD_SEQUENCE_PLACEMENT_MAP& placements, WORLD_SEQUENCE_DEPLOY_MAP& deploy) const
{
    CWorldSequencePlayer::Collect_ValidationTargets(
        const_cast<CLevel_KakulSaydonArena*>(this)->Make_WorldSequenceTargets(), placements, deploy);
}

bool_t CLevel_KakulSaydonArena::Try_GetWorldSequencePlacementBaseline(
    const WORLD_SEQUENCE_INSTANCE& instance, float3_t& outPosition) const
{
    if (instance.anchorKind != "WORLD" || instance.bindings.empty()) return false;
    std::set<std::pair<WORLD_SEQUENCE_TARGET_KIND, std::string>> seen;
    double sumX = 0.0, sumY = 0.0, sumZ = 0.0;
    size_t count = 0u;
    for (const auto& binding : instance.bindings)
    {
        if (!seen.emplace(binding.targetKind, binding.targetId).second) continue;
        float3_t position{};
        if (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
            position = instance.position;
        else
        {
            uint64_t id = 0u;
            if (!CWorldSequencePlayer::Try_ParseTargetId(binding, id)) return false;
            if (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT)
            {
                const auto& placements = m_MapRuntime.Get_Placements();
                const auto entry = std::find_if(placements.begin(), placements.end(),
                    [id](const auto& value) { return value.record.placementId == id; });
                if (entry == placements.end()) return false;
                position = entry->record.position;
            }
            else if (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT)
            {
                const auto object = m_DeployRuntime.Find(id);
                float4_t rotation;
                if (!object || !object->Get_PlacedRootPose(position, rotation)) return false;
            }
            else return false;
        }
        if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z)) return false;
        sumX += position.x; sumY += position.y; sumZ += position.z;
        ++count;
    }
    if (!count) return false;
    // One common translation preserves the relative placement of a curtain group.
    outPosition = {float(sumX / count), float(sumY / count), float(sumZ / count)};
    return true;
}

bool_t CLevel_KakulSaydonArena::Reload_WorldObjectRuntime(std::string& status)
{
    if (m_SequencePlayer.Has_ActiveInstances())
    {
        m_bWorldObjectReloadPending = true;
        status = "Saved World Objects are ready for the next play; the active Server sequence keeps its current revision.";
        return true;
    }
    m_bWorldObjectReloadPending = false;
    // Every owned cue and authoring preview has its own admitted document copy.
    // Replacing the idle base does not stop or rewrite any of those active poses.
    const bool_t result = m_SequencePlayer.Load_Area("LV_LUT_MIDNIGHTC_ED", Make_WorldSequenceTargets());
    status = m_SequencePlayer.Get_Status();
    return result;
}

#ifdef _DEBUG
bool_t CLevel_KakulSaydonArena::Debug_BeginWorldObjectPreview(
    const CWorldSequenceDocument& document, const std::string& instanceId, std::string& status,
    const bool_t previewAtCharacter)
{
    if (m_SequencePlayer.Has_ActiveInstances() || !m_CompositionWorldPreviewCues.empty())
    { status = "Stop the active pattern/world preview before previewing this object."; return false; }
    if (!Can_StartCompositionWorld(instanceId, status, &document)) return false;
    const auto targets = Make_WorldSequenceTargets();
    auto staged = std::make_unique<CWorldSequencePlayer>();
    if (!staged->Set_Document(document, targets, status) || !staged->Prepare_InstanceResources(instanceId, targets))
    { status = staged->Get_Status(); return false; }
    float3_t previewOffset{};
    const auto* instance = document.Find_Instance(instanceId);
    if (previewAtCharacter && instance && instance->anchorKind == "WORLD")
    {
        float3_t previewPosition{}, baseline{};
        if (!Try_GetWorldSequencePlacementBaseline(*instance, baseline))
        { status = "World preview cannot resolve every saved placement in this object group."; return false; }
        if (!Try_Get_AuthoringForwardPlacement(previewPosition, status)) return false;
        previewOffset = {previewPosition.x - baseline.x,
            previewPosition.y - baseline.y, previewPosition.z - baseline.z};
    }
    // The preview offset never edits the saved map anchor or placed curtain/roulette.
    // All resources and the preview placement are admitted before releasing the old presentation.
    Debug_StopWorldObjectPreview();
    if (!staged->Play(instanceId, targets, 1.f, previewOffset)) { status = staged->Get_Status(); return false; }
    m_pWorldObjectPreview = std::move(staged);
    m_WorldObjectPreviewInstance = instanceId;
    return Debug_SampleWorldObjectPreview(0.f, status);
}

bool_t CLevel_KakulSaydonArena::Debug_SampleWorldObjectPreview(const f32_t clockMs, std::string& status)
{
    if (!m_pWorldObjectPreview || !std::isfinite(clockMs) || clockMs < 0.f)
    { status = "World Object preview is not active."; return false; }
    if (!m_pWorldObjectPreview->Seek_InstanceToMs(m_WorldObjectPreviewInstance, clockMs, Make_WorldSequenceTargets()))
    {
        status = m_pWorldObjectPreview->Get_Status();
        Debug_StopWorldObjectPreview();
        return false;
    }
    status = m_pWorldObjectPreview->Get_ObjectSampleStatus(m_WorldObjectPreviewInstance);
    return true;
}

void CLevel_KakulSaydonArena::Debug_StopWorldObjectPreview()
{
    if (m_pWorldObjectPreview) m_pWorldObjectPreview->Stop_All(Make_WorldSequenceTargets(), true);
    m_pWorldObjectPreview.reset();
    m_WorldObjectPreviewInstance.clear();
}
#endif

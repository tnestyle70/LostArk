#include "Level_KakulSaydonArena.h"
#include "Character.h"
#include "Npc.h"
#include "CombatHUDViewModel.h"
#include "DeployPropObject.h"
#include "EffectV2_Catalog.h"
#include "EffectV2_Runtime.h"
#include "KoukuSaydonPresentationPlayer.h"
#include "KoukuSaydonCompositionDocument.h"
#include "KakulArenaHiddenPlacements.h"
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

void CLevel_KakulSaydonArena::Update_MarioBallPresentation(const f32_t timeDelta)
{
    constexpr f32_t CURSE_NOTICE_SECONDS = 3.f;
    constexpr float BALL_CENTRE_HEIGHT_M = .47f;
    static constexpr const char* SMOKE_LEAVES[3] = {
        "boss.kouku.ball.smoke.red_1", "boss.kouku.ball.smoke.blue_1", "boss.kouku.ball.smoke.yellow_1" };
    const auto& player = CCombatHUDViewModel::Get().Get_Player();
    const auto& document = m_SequencePlayer.Get_Document();
    // Follows the layout the bounce update chose this frame; empty outside Mario.
    const bool_t layoutChanged = m_strMarioLayoutInstance != m_strMarioBallLayoutInstance;
    if (layoutChanged)
    {
        /* Hand the previous layout's slots back. Its stop already restored
           the authored visibility, so nothing is shown here. */
        if (const auto* previous = document.Find_Instance(m_strMarioBallLayoutInstance))
            for (const auto& binding : previous->bindings)
                if (uint64_t placementId = 0u; CWorldSequencePlayer::Try_ParseTargetId(binding, placementId))
                    m_SequencePlayer.Set_PlacementSuppressed(placementId, false);
        m_strMarioBallLayoutInstance = m_strMarioLayoutInstance;
        m_iMarioPoppedBallsSeen = 0u;
        m_iMarioCurseSeen = 0u;
        m_iMarioCurseNoticeQueue = 0u;
        m_iMarioCurseNoticeColor = -1;
        m_fMarioCurseNoticeSeconds = 0.f;
    }
    const auto* layout = m_strMarioBallLayoutInstance.empty() ?
        nullptr : document.Find_Instance(m_strMarioBallLayoutInstance);
    const std::uint16_t popped = layout ? player.iMarioPoppedBallMask : std::uint16_t{};
    const std::uint8_t curse = layout ? player.iMarioCurseReleasedMask : std::uint8_t{};
    /* Balls popped before this player arrived are hidden silently: the smoke
       and the notice belong to the pop itself. */
    const std::uint16_t changed = static_cast<std::uint16_t>(popped ^ m_iMarioPoppedBallsSeen);
    auto& placements = m_MapRuntime.Get_MutablePlacements();
    for (size_t slot = 0u; layout && slot < layout->bindings.size() && slot < 16u; ++slot)
    {
        const std::uint16_t bit = static_cast<std::uint16_t>(1u << slot);
        uint64_t placementId = 0u;
        if (!(changed & bit) || !CWorldSequencePlayer::Try_ParseTargetId(layout->bindings[slot], placementId))
            continue;
        const bool_t hidden = 0u != (popped & bit);
        m_SequencePlayer.Set_PlacementSuppressed(placementId, hidden);
        auto* entry = CWorldSequencePlayer::Find_Placement(placements, placementId);
        if (nullptr == entry) continue;
        (void)CMapPlacementRuntime::Set_RuntimeVisible(*entry, !hidden);
        if (!hidden || layoutChanged) continue;
        const std::string& asset = entry->record.assetId;
        const int color = asset == "MAP_MARIO_RED_STAR_BALL" ? 0 :
            asset == "MAP_MARIO_BLUE_BALL" ? 1 : asset == "MAP_MARIO_YELLOW_BALL" ? 2 : -1;
        if (color < 0 || m_bMarioBallSmokeFailed[color]) continue;
        const auto smoke = m_SequencePlayer.Find_PreparedLeafSnapshot(SMOKE_LEAVES[color]);
        if (!smoke)
        {
            m_bMarioBallSmokeFailed[color] = true;
            OutputDebugStringA(("[MarioBall] Loader snapshot unavailable: " + std::string(SMOKE_LEAVES[color]) + "\n").c_str());
            continue;
        }
        /* The authored cloud sits at its own local offset (2.3 m up, 1.35 m
           forward) because it was written for a taller anchor. Cancel that
           offset so the puff lands on the ball; reading it back keeps this
           correct if the effect is re-authored. */
        const auto* document = smoke->Find_Document(SMOKE_LEAVES[color]);
        const float3_t authored = nullptr == document ? float3_t{} :
            document->Desc.Params.Position.vStart;
        EFFECT_V2_GROUP_PLAYBACK_DESC playback;
        XMStoreFloat4x4(&playback.PivotWorld, XMMatrixTranslation(
            entry->record.position.x - authored.x,
            entry->record.position.y + BALL_CENTRE_HEIGHT_M - authored.y,
            entry->record.position.z - authored.z));
        playback.bProductOwned = true;
        if (0u == CEffectV2Runtime::Play_Leaf(SMOKE_LEAVES[color], smoke, playback, m_pDevice, m_pContext))
            OutputDebugStringA(("[MarioBall] smoke: " + CEffectV2Runtime::Last_Error() + "\n").c_str());
    }
    m_iMarioPoppedBallsSeen = popped;
    if (!layoutChanged)
        m_iMarioCurseNoticeQueue |= static_cast<std::uint8_t>(curse & ~m_iMarioCurseSeen);
    m_iMarioCurseSeen = curse;
    if (m_iMarioCurseNoticeColor >= 0)
    {
        m_fMarioCurseNoticeSeconds -= std::isfinite(timeDelta) && timeDelta > 0.f ? timeDelta : 0.f;
        if (m_fMarioCurseNoticeSeconds <= 0.f) m_iMarioCurseNoticeColor = -1;
    }
    for (int color = 0; m_iMarioCurseNoticeColor < 0 && color < 3; ++color)
        if (m_iMarioCurseNoticeQueue & (1u << color))
        {
            m_iMarioCurseNoticeQueue &= static_cast<std::uint8_t>(~(1u << color));
            m_iMarioCurseNoticeColor = color;
            m_fMarioCurseNoticeSeconds = CURSE_NOTICE_SECONDS;
        }
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
    targets.objectPreparationOwner = &m_SequencePlayer;
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
    const auto targets = Make_WorldSequenceTargets();
    if (!m_SequencePlayer.Load_Area("LV_LUT_MIDNIGHTC_ED", targets))
    { status = m_SequencePlayer.Get_Status(); return false; }
    const auto loadedStatus = m_SequencePlayer.Get_Status();
    std::string preparationErrors;
    for (const auto& [instanceId, copies] : {
        std::pair{"world.object.instance.kouku.card", 6u},
        std::pair{"world.object.instance.kouku.joker_card", 1u}})
        if (!m_SequencePlayer.Prewarm_ObjectInstances(instanceId, copies, targets))
        {
            if (!preparationErrors.empty()) preparationErrors += " / ";
            preparationErrors += m_SequencePlayer.Get_Status();
        }
    status = loadedStatus + (preparationErrors.empty() ? "; card clones prepared (6+1)." :
        "; card prewarm incomplete: " + preparationErrors);
    if (!preparationErrors.empty()) OutputDebugStringA(("[WorldObjectReload][JokerPrewarm] " + status + "\n").c_str());
    // The document committed successfully. A preparation warning does not turn
    // that completed load into a false failure or stop an older child revision.
    return true;
}

#ifdef _DEBUG
namespace
{
    struct WORLD_ALIAS_SCREEN_COMPANION final
    {
        std::string instanceId;
        KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE resource;
        std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> snapshot;
        float startDelayMs = 0.f, playbackSpeed = 1.f, durationMs = 0.f;
    };

    bool Stage_WorldAliasScreenCompanions(const CWorldSequenceDocument& document,
        const std::vector<std::string>& ids, std::vector<WORLD_ALIAS_SCREEN_COMPANION>& companions,
        std::string& status)
    {
        for (const auto& id : ids)
        {
            const bool alias = std::any_of(document.Get_ObjectResources().begin(), document.Get_ObjectResources().end(),
                [&](const auto& row) { return row.modelAssetId.empty() && row.sequenceInstanceId == id; });
            if (alias) { WORLD_ALIAS_SCREEN_COMPANION row; row.instanceId = id; companions.push_back(std::move(row)); }
        }
        if (companions.empty()) return true;
        // Read the actual saved companion references. Object preview never saves or reloads an open draft.
        for (const auto& path : {CKoukuSaydonCompositionDocument::Resolve_Path(), CKoukuSaydonCompositionDocument::Resolve_SequencePath()})
        {
            CKoukuSaydonCompositionDocument composition(path);
            if (!composition.Reload(status)) return false;
            const auto& saved = composition.Get_LastGood();
            for (auto& companion : companions)
                for (const auto& world : saved.Worlds)
                {
                    if (world.strSequenceInstanceId != companion.instanceId || world.strCompanionEffectResourceId.empty()) continue;
                    const auto resource = std::find_if(saved.PresentationResources.begin(), saved.PresentationResources.end(),
                        [&](const auto& row) { return row.strResourceId == world.strCompanionEffectResourceId; });
                    if (resource == saved.PresentationResources.end() || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT)
                    { status = "Object alias companion Effect resource is unavailable: " + world.strCompanionEffectResourceId; return false; }
                    if (!companion.resource.strAssetId.empty() &&
                        (companion.resource.strAssetId != resource->strAssetId || companion.resource.strResourceKind != resource->strResourceKind))
                    { status = "Object alias has conflicting saved companion Effects: " + companion.instanceId; return false; }
                    companion.resource = *resource;
                }
        }
        std::erase_if(companions, [](const auto& row) { return row.resource.strAssetId.empty(); });
        for (auto& companion : companions)
        {
            const auto* instance = document.Find_Instance(companion.instanceId);
            const auto* sequence = instance ? document.Find_Template(instance->templateId) : nullptr;
            if (!sequence || !instance->enabled || !sequence->durationMs || !std::isfinite(instance->playbackSpeed) || instance->playbackSpeed <= 0.f)
            { status = "Object alias companion has no valid motion clock: " + companion.instanceId; return false; }
            const auto& kind = companion.resource.strResourceKind;
            if ((kind != "GROUP" && kind != "LEAF") ||
                !CEffectV2Catalog::Get().Load_ResourceSnapshot(kind == "GROUP" ? EFFECT_V2_RESOURCE_KIND::GROUP : EFFECT_V2_RESOURCE_KIND::LEAF,
                    companion.resource.strAssetId, companion.snapshot, status))
            { if (kind != "GROUP" && kind != "LEAF") status = "Object alias screen preview needs a saved V2 Leaf or Group."; return false; }
            if (companion.snapshot->Get_Documents().empty() ||
                !std::all_of(companion.snapshot->Get_Documents().begin(), companion.snapshot->Get_Documents().end(),
                    [](const auto& leaf) { return leaf.eType == EFFECT_V2_TYPE::SCREEN_POST; }))
            { status = "Object alias companion includes a world Effect; play its complete Composition to preserve that anchor."; return false; }
            companion.startDelayMs = float(instance->startDelayMs);
            companion.playbackSpeed = instance->playbackSpeed;
            companion.durationMs = float(sequence->durationMs);
        }
        return true;
    }
}

bool_t CLevel_KakulSaydonArena::Debug_PrepareGateObjects(const size_t gateIndex, std::string& status)
{
    auto staged = std::make_unique<GATE_OBJECT_PRESENTATION>();
    staged->gateIndex = gateIndex;
    if (gateIndex != 0u && gateIndex != 2u)
    { m_pPendingGateObjects = std::move(staged); status.clear(); return true; }
    const auto& document = m_SequencePlayer.Get_Document();
    staged->player = std::make_unique<CWorldSequencePlayer>();
    auto targets = Make_WorldSequenceTargets();
    targets.objectPreparationOwner = staged->player.get();
    if (!staged->player->Set_Document(document, targets, status)) return false;
    if (gateIndex == 0u)
    {
        const std::string bookId = "world.sequence.instance.kouku.gate1.authored.book";
        const auto* book = document.Find_Instance(bookId);
        if (!book || !book->enabled || book->motionEnd != WORLD_SEQUENCE_MOTION_END::HOLD ||
            book->bindings.size() != 1u || book->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ||
            book->bindings.front().targetId != "world.object.kouku.popup.book")
        { status = "Gate 1 requires its saved popup book HOLD motion."; return false; }
        staged->instances.emplace_back(bookId, staged->player->Get_InstanceElapsedSpanMs(bookId));
        // Match the existing Sequence end: the standing copy replaces the
        // unfolding copy. Holding both copies would draw overlapping floors.
        for (uint64_t id = 41u; id <= 176u; ++id)
        {
            auto* entry = CWorldSequencePlayer::Find_Placement(*targets.pPlacements, id);
            bool_t visible = false;
            if (!entry || !CMapPlacementRuntime::Try_GetRuntimeVisible(*entry, visible))
            { status = "Gate 1 unfolding placement is unavailable: " + std::to_string(id); return false; }
            staged->visibility.push_back({id, visible, false});
        }
        for (const uint64_t id : KAKUL_ARENA_HIDDEN_PLACEMENT_IDS)
        {
            auto* entry = CWorldSequencePlayer::Find_Placement(*targets.pPlacements, id);
            if (!entry) continue; // The Loader's declared scope owns membership.
            bool_t visible = false;
            if (!CMapPlacementRuntime::Try_GetRuntimeVisible(*entry, visible))
            { status = "Gate 1 standing placement visibility is unavailable: " + std::to_string(id); return false; }
            staged->visibility.push_back({id, visible, true});
        }
        if (staged->visibility.size() == 136u)
        { status = "Gate 1 standing arena is outside the loaded map scope."; return false; }
        const auto legacy = m_DeployRuntime.Find(7u);
        if (legacy && legacy->Is_AnimationAuthoringPreviewActive())
        { status = "Gate 1 legacy book is already owned by an animation preview."; return false; }
    }
    else
    {
        const auto* group = document.Find_ObjectResource("world.object.group.kouku.g3.outer_fire");
        if (!group || group->motionInstanceIds.size() != 6u)
        { status = "Gate 3 requires the saved 3-gate outer fire Object group with six motions."; return false; }
        for (const auto& id : group->motionInstanceIds)
        {
            const auto* instance = document.Find_Instance(id);
            if (!instance || !instance->enabled || instance->motionEnd != WORLD_SEQUENCE_MOTION_END::LOOP)
            { status = "Gate 3 outer fire requires six enabled LOOP motions: " + id; return false; }
            staged->instances.emplace_back(id, 0.f);
        }
    }
    std::map<std::string, std::pair<std::string, uint32_t>> cloneCounts;
    for (const auto& [id, clock] : staged->instances)
    {
        const auto* instance = document.Find_Instance(id);
        const auto* sequence = instance ? document.Find_Template(instance->templateId) : nullptr;
        if (!sequence || !std::isfinite(clock) || clock < 0.f || instance->bindings.size() != 1u ||
            instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
        { status = "Gate Object motion must have one model resource and a valid clock: " + id; return false; }
        if (!Can_StartCompositionWorld(id, status, &document)) return false;
        if (!staged->player->Prepare_InstanceResources(id, targets))
        { status = "Gate Object preparation failed: " + staged->player->Get_Status(); return false; }
        auto& count = cloneCounts[instance->bindings.front().targetId];
        count.first = id;
        count.second += sequence->objectMotion.EmissionCount();
    }
    // CW and CCW share one resource pool; reserve their simultaneous emissions.
    for (const auto& [objectId, count] : cloneCounts)
        if (!staged->player->Prewarm_ObjectInstances(count.first, count.second, targets))
        { status = "Gate Object clone preparation failed: " + staged->player->Get_Status(); return false; }
    m_pPendingGateObjects = std::move(staged);
    status.clear();
    return true;
}

bool_t CLevel_KakulSaydonArena::Debug_ReleaseGateObjectPresentation(
    GATE_OBJECT_PRESENTATION& state, std::string& status)
{
    if (state.suspended) return true;
    auto targets = Make_WorldSequenceTargets();
    targets.objectPreparationOwner = state.player.get();
    if (state.player)
    {
        for (auto& [id, clock] : state.instances)
            (void)state.player->Try_GetElapsedMs(id, clock);
        state.player->Stop_All(targets, true);
    }
    bool restored = true;
    for (const auto& row : state.visibility)
    {
        auto* entry = CWorldSequencePlayer::Find_Placement(*targets.pPlacements, row.placementId);
        bool_t visible = false;
        if (entry && (!CMapPlacementRuntime::Try_GetRuntimeVisible(*entry, visible) ||
            (visible == row.applied && visible != row.previous && !CMapPlacementRuntime::Set_RuntimeVisible(*entry, row.previous))))
            restored = false;
    }
    if (state.previousLegacyBook)
    {
        const auto book = m_DeployRuntime.Find(7u);
        if (book && book->Get_State() == DEPLOY_PROP_STATE::DESPAWNED &&
            book->Get_State() != *state.previousLegacyBook && !m_DeployRuntime.Set_State(7u, *state.previousLegacyBook))
            restored = false;
    }
    if (!restored) { status = "Gate Object previous visibility/state restore is pending."; return false; }
    state.suspended = true;
    return true;
}

bool_t CLevel_KakulSaydonArena::Debug_StartGateObjectPresentation(
    GATE_OBJECT_PRESENTATION& state, std::string& status)
{
    if (!state.suspended) return true;
    auto targets = Make_WorldSequenceTargets();
    targets.objectPreparationOwner = state.player.get();
    state.suspended = false;
    const auto fail = [&]() {
        const auto failure = status;
        std::string restore;
        if (!Debug_ReleaseGateObjectPresentation(state, restore)) status = failure + " / " + restore;
        return false;
    };
    for (const auto& [id, clock] : state.instances)
        if (!state.player->Play(id, targets) || !state.player->Seek_InstanceToMs(id, clock, targets))
        { status = "Gate Object start failed: " + state.player->Get_Status(); return fail(); }
    for (const auto& row : state.visibility)
    {
        auto* entry = CWorldSequencePlayer::Find_Placement(*targets.pPlacements, row.placementId);
        if (!entry || !CMapPlacementRuntime::Set_RuntimeVisible(*entry, row.applied))
        { status = "Gate Object visibility apply failed: " + std::to_string(row.placementId); return fail(); }
    }
    if (state.previousLegacyBook && !m_DeployRuntime.Set_State(7u, DEPLOY_PROP_STATE::DESPAWNED))
    { status = "Gate Object legacy book hide failed: " + m_DeployRuntime.Get_Status(); return fail(); }
    return true;
}

bool_t CLevel_KakulSaydonArena::Debug_CommitGateObjects(const size_t gateIndex, std::string& status)
{
    if (!m_pPendingGateObjects || m_pPendingGateObjects->gateIndex != gateIndex)
    { status = "Gate Object activation has no matching prepared stage."; return false; }
    Debug_StopCompositionWorldPreview();
    Debug_StopWorldObjectPreview();
    if (m_pGateObjects && !Debug_ReleaseGateObjectPresentation(*m_pGateObjects, status)) return false;
    auto staged = std::move(m_pPendingGateObjects);
    const auto rollback = [&]() {
        if (m_pGateObjects)
        {
            std::string restore;
            if (!Debug_StartGateObjectPresentation(*m_pGateObjects, restore)) status += " / Previous gate restore: " + restore;
        }
        return false;
    };
    // Capture after releasing the previous gate; a later stop restores the
    // original scene, not the previous gate's borrowed presentation.
    for (auto& row : staged->visibility)
    {
        auto* entry = CWorldSequencePlayer::Find_Placement(m_MapRuntime.Get_MutablePlacements(), row.placementId);
        if (!entry || !CMapPlacementRuntime::Try_GetRuntimeVisible(*entry, row.previous))
        { status = "Gate Object baseline capture failed: " + std::to_string(row.placementId); return rollback(); }
    }
    if (gateIndex == 0u)
        if (const auto book = m_DeployRuntime.Find(7u)) staged->previousLegacyBook = book->Get_State();
    if (!Debug_StartGateObjectPresentation(*staged, status)) return rollback();
    m_pGateObjects = std::move(staged);
    status.clear();
    return true;
}

void CLevel_KakulSaydonArena::Debug_CancelGateObjects()
{
    m_pPendingGateObjects.reset();
}

void CLevel_KakulSaydonArena::Debug_StopGateObjects()
{
    Debug_CancelGateObjects();
    m_bCompositionWorldPreviewBorrowsGateObjects = false;
    std::string status;
    if (m_pGateObjects && !Debug_ReleaseGateObjectPresentation(*m_pGateObjects, status))
    { OutputDebugStringA(("[KoukuGateObjects] " + status + "\n").c_str()); return; }
    m_pGateObjects.reset();
}

bool_t CLevel_KakulSaydonArena::Debug_SetGateObjectsSuspended(const bool_t suspended, std::string& status)
{
    if (!m_pGateObjects || m_pGateObjects->gateIndex != 0u) return true;
    return suspended ? Debug_ReleaseGateObjectPresentation(*m_pGateObjects, status) :
        Debug_StartGateObjectPresentation(*m_pGateObjects, status);
}

void CLevel_KakulSaydonArena::Debug_UpdateGateObjects(const f32_t delta)
{
    if (!m_pGateObjects || m_pGateObjects->suspended || m_pGateObjects->gateIndex != 2u) return;
    auto targets = Make_WorldSequenceTargets();
    targets.objectPreparationOwner = m_pGateObjects->player.get();
    m_pGateObjects->player->Update(delta, targets);
    for (const auto& [id, clock] : m_pGateObjects->instances)
        if (!m_pGateObjects->player->Is_Playing(id))
        {
            m_strDebugGateStatus = "Gate 3 outer fire playback failed: " + m_pGateObjects->player->Get_Status();
            Debug_StopGateObjects();
            return;
        }
}

bool_t CLevel_KakulSaydonArena::Debug_BeginWorldObjectPreview(
    const CWorldSequenceDocument& document, const std::string& instanceId, std::string& status,
    const bool_t previewAtCharacter)
{
    if (m_SequencePlayer.Has_ActiveInstances() || !m_CompositionWorldPreviewCues.empty())
    { status = "Stop the active pattern/world preview before previewing this object."; return false; }
    const auto* group = document.Find_ObjectResource(instanceId);
    const bool isGroup = group && !group->motionInstanceIds.empty();
    std::vector<std::string> ids = isGroup ? group->motionInstanceIds : std::vector<std::string>{instanceId};
    if (isGroup)
        std::erase_if(ids, [&document](const auto& id) {
            const auto* motion = document.Find_Instance(id); return motion && !motion->enabled;
        });
    if (ids.empty()) { status = "Object group has no enabled motions."; return false; }
    for (const auto& id : ids)
        if (!Can_StartCompositionWorld(id, status, &document)) return false;
    std::vector<WORLD_ALIAS_SCREEN_COMPANION> companions;
    if (!Stage_WorldAliasScreenCompanions(document, ids, companions, status)) return false;
    std::string previewNotice;
    // A screen companion is independent of camera/map placement. Audition it even
    // when this single alias's physical scenery lies outside the active map scope.
    if (ids.size() == 1u && companions.size() == 1u)
    {
        const auto* instance = document.Find_Instance(ids.front());
        const bool mapAlias = instance && instance->anchorKind == "WORLD" && !instance->bindings.empty() &&
            std::all_of(instance->bindings.begin(), instance->bindings.end(), [](const auto& binding) {
                uint64_t placementId = 0u;
                return binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT &&
                    CWorldSequencePlayer::Try_ParseTargetId(binding, placementId);
            });
        float3_t baseline;
        if (mapAlias && !Try_GetWorldSequencePlacementBaseline(*instance, baseline))
        {
            previewNotice = "Screen companion only: the alias map placements are outside the current loaded scope.";
            ids.clear();
        }
    }
    const auto targets = Make_WorldSequenceTargets();
    auto staged = std::make_unique<CWorldSequencePlayer>();
    if (!ids.empty() && !staged->Set_Document(document, targets, status))
    { status = staged->Get_Status(); return false; }
    for (const auto& id : ids)
        if (!staged->Prepare_InstanceResources(id, targets))
        { status = staged->Get_Status(); return false; }
    float3_t previewOffset{};
    const auto* instance = ids.empty() ? nullptr : document.Find_Instance(ids.front());
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
    // Model-only groups stage independent instances before replacing the old
    // preview. A shared offset preserves every member's relative placement.
    const bool isolatedObjects = std::all_of(ids.begin(), ids.end(), [&](const auto& id) {
        const auto* motion = document.Find_Instance(id);
        return motion && !motion->bindings.empty() && std::all_of(motion->bindings.begin(), motion->bindings.end(),
            [](const auto& binding) { return binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE; });
    });
    std::vector<WORLD_OBJECT_PREVIEW_SCREEN_EFFECT> stagedEffects;
    const auto rollback = [&]() {
        staged->Stop_All(targets, true);
        for (const auto& effect : stagedEffects) CEffectV2Runtime::Stop_Group(effect.handle);
    };
    for (const auto& companion : companions)
    {
        EFFECT_V2_GROUP_PLAYBACK_DESC playback;
        XMStoreFloat4x4(&playback.PivotWorld, XMMatrixIdentity());
        playback.fDurationSeconds = companion.durationMs * .001f;
        playback.fInitialAgeSeconds = companion.startDelayMs > 0.f ? playback.fDurationSeconds : 0.f;
        playback.bProductOwned = true;
        playback.bExternalClock = true;
        uint32_t handle = 0u;
        if (companion.resource.strResourceKind == "GROUP")
        {
            const auto* groupResource = companion.snapshot->Find_Group(companion.resource.strAssetId);
            if (groupResource) handle = CEffectV2Runtime::Play_Group(*groupResource, companion.snapshot, playback, m_pDevice, m_pContext);
        }
        else handle = CEffectV2Runtime::Play_Leaf(companion.resource.strAssetId, companion.snapshot, playback, m_pDevice, m_pContext);
        if (!handle) { status = CEffectV2Runtime::Last_Error(); rollback(); return false; }
        stagedEffects.push_back({handle, companion.startDelayMs, companion.playbackSpeed, companion.durationMs});
        if (!CEffectV2Runtime::Sample_Group(handle, playback.fInitialAgeSeconds, true, m_pDevice, m_pContext))
        { status = CEffectV2Runtime::Last_Error(); rollback(); return false; }
    }
    if (!isGroup && !isolatedObjects) Debug_StopWorldObjectPreview();
    for (const auto& id : ids)
        if (!staged->Play(id, targets, 1.f, previewOffset))
        { status = staged->Get_Status(); rollback(); return false; }
    if (isolatedObjects)
        for (const auto& id : ids)
            if (!staged->Seek_InstanceToMs(id, 0.f, targets))
            { status = staged->Get_Status(); rollback(); return false; }
    if (isGroup || isolatedObjects) Debug_StopWorldObjectPreview();
    m_pWorldObjectPreview = std::move(staged);
    m_WorldObjectPreviewInstances = std::move(ids);
    m_WorldObjectPreviewScreenEffects = std::move(stagedEffects);
    m_strWorldObjectPreviewNotice = std::move(previewNotice);
    return Debug_SampleWorldObjectPreview(0.f, status);
}

bool_t CLevel_KakulSaydonArena::Debug_SampleWorldObjectPreview(const f32_t clockMs, std::string& status)
{
    if (!m_pWorldObjectPreview || !std::isfinite(clockMs) || clockMs < 0.f)
    { status = "World Object preview is not active."; return false; }
    for (const auto& id : m_WorldObjectPreviewInstances)
    {
        if (!m_pWorldObjectPreview->Seek_InstanceToMs(id, clockMs, Make_WorldSequenceTargets()))
        {
            status = m_pWorldObjectPreview->Get_Status();
            Debug_StopWorldObjectPreview();
            return false;
        }
    }
    for (const auto& effect : m_WorldObjectPreviewScreenEffects)
    {
        const float sourceMs = clockMs < effect.startDelayMs ? effect.durationMs :
            (clockMs - effect.startDelayMs) * effect.playbackSpeed;
        if (!CEffectV2Runtime::Sample_Group(effect.handle, sourceMs * .001f, true, m_pDevice, m_pContext))
        {
            status = "Object screen companion sampling failed: " + CEffectV2Runtime::Last_Error();
            Debug_StopWorldObjectPreview();
            return false;
        }
    }
    status = std::to_string(m_WorldObjectPreviewInstances.size()) + " motion(s), " +
        std::to_string(m_WorldObjectPreviewScreenEffects.size()) + " screen companion(s). ";
    if (!m_WorldObjectPreviewInstances.empty())
        status += m_pWorldObjectPreview->Get_ObjectSampleStatus(m_WorldObjectPreviewInstances.front());
    if (!m_strWorldObjectPreviewNotice.empty()) status += m_strWorldObjectPreviewNotice;
    return true;
}

void CLevel_KakulSaydonArena::Debug_StopWorldObjectPreview()
{
    if (m_pWorldObjectPreview) m_pWorldObjectPreview->Stop_All(Make_WorldSequenceTargets(), true);
    m_pWorldObjectPreview.reset();
    m_WorldObjectPreviewInstances.clear();
    for (const auto& effect : m_WorldObjectPreviewScreenEffects) CEffectV2Runtime::Stop_Group(effect.handle);
    m_WorldObjectPreviewScreenEffects.clear();
    m_strWorldObjectPreviewNotice.clear();
}
#endif

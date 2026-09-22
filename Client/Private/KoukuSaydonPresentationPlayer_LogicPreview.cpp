#include <WinSock2.h>
#include "KoukuSaydonPresentationPlayer.h"
#include "ActorCatalog.h"
#include "Character.h"
#include "EffectV2_Runtime.h"
#include "Gameplay/CombatCollisionContract.h"
#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
#include <cmath>

namespace
{
using namespace Client;
using LOGIC = KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION;

const LOGIC* Find_BlueCircleLogic(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE& occurrence)
{
    if (!occurrence.bEnabled) return nullptr;
    const auto found = std::find_if(document.Logics.begin(), document.Logics.end(),
        [&](const auto& logic) { return logic.strLogicId == occurrence.strLogicId; });
    return found != document.Logics.end() && found->strLogicType == "TRIGGER" &&
        found->strTriggerKind == "ALBION_BLUE_CIRCLE" ? &*found : nullptr;
}

const LOGIC* Find_SelectedAirborneLogic(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE& occurrence)
{
    if (!occurrence.bEnabled) return nullptr;
    const auto found = std::find_if(document.Logics.begin(), document.Logics.end(),
        [&](const auto& logic) { return logic.strLogicId == occurrence.strLogicId; });
    return found != document.Logics.end() && found->strLogicType == "TRIGGER" &&
        found->strTriggerKind == "ALBION_AIRBORNE" && found->strAirbornePhase == "SELECT_PLAYER" &&
        found->strAirborneTargetPositionPolicy == "SELECT" && !found->strSelectedEffectGroupId.empty() ? &*found : nullptr;
}

bool Supports_PlayerCenteredCircle(const LOGIC& logic)
{
    return logic.iCountPerPlayer == 1u && logic.fPlayerEffectRadiusM == 0.0 &&
        !logic.bRandomPlayerOnly && logic.iArenaRandomCount == 0u &&
        logic.iEffectLifetimeMs > 0u && logic.iEffectLifetimeMs <= 600000u;
}

const BOSS_COMBAT_OBJECT_VISUAL_ENTRY* Find_BlueCircleVisual(
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern)
{
    return CActorCatalog::Find_BossCombatObjectVisual(
        CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(pattern.strTargetBossPlacementId),
        "combatobject.kouku.albion.bluecircle", "combatvisual.kouku.albion.bluecircle");
}

bool Is_EligiblePlayer(const KOUKU_CARD_PRESENTATION_VIEW& player)
{
    using namespace LostArk::Shared;
    const auto& snapshot = player.Snapshot;
    return snapshot.iNetEntityId != INVALID_NET_ENTITY_ID && snapshot.isCombatReady &&
        snapshot.iCurrentHp != 0u && snapshot.iMarioStage == 0u &&
        snapshot.eAction != PLAYER_ACTION_STATE::DEAD && snapshot.eAction != PLAYER_ACTION_STATE::FALLING &&
        std::isfinite(snapshot.fPositionX) && std::isfinite(snapshot.fPositionY) && std::isfinite(snapshot.fPositionZ);
}
}

bool Client::CKoukuSaydonPresentationPlayer::Is_SelectedAirborneGroupMember(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
    const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& occurrence)
{
    if (occurrence.strSelectionGroupId.empty()) return false;
    return std::any_of(pattern.LogicOccurrences.begin(), pattern.LogicOccurrences.end(), [&](const auto& box) {
        const auto* logic = Find_SelectedAirborneLogic(document, box);
        return logic && logic->strSelectedEffectGroupId == occurrence.strSelectionGroupId;
    });
}

bool Client::CKoukuSaydonPresentationPlayer::Build_SelectedAirbornePresentation(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, const LOGIC& logic,
    const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE& occurrence,
    PRODUCT_PATTERN& output, std::string& error)
{
    PRODUCT_PATTERN staged;
    staged.pattern.strPatternId = pattern.strPatternId + ".selected-effect";
    const auto first = std::find_if(pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(),
        [&](const auto& row) { return row.strSelectionGroupId == logic.strSelectedEffectGroupId; });
    if (first == pattern.PresentationOccurrences.end())
    { error = "Selected airborne Effect group is absent: " + logic.strSelectedEffectGroupId; return false; }
    const auto origin = first->PositionOffset;
    std::set<std::string> resources;
    for (const auto& source : pattern.PresentationOccurrences)
    {
        if (source.strSelectionGroupId != logic.strSelectedEffectGroupId) continue;
        const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
            [&](const auto& row) { return row.strResourceId == source.strResourceId; });
        const auto endMs = std::uint64_t(source.iStartMs) + source.iDurationMs;
        if (resource == document.PresentationResources.end() || resource->eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
            source.strAnchorKind != "MAP" || source.bFollowBoss || !source.strBone.empty() ||
            !source.strWorldId.empty() || source.iStartMs < occurrence.iStartMs || endMs > 600000u)
        { error = "Selected airborne group needs finite MAP Effects after SELECT: " + source.strOccurrenceId; return false; }
        if (resources.insert(resource->strResourceId).second) staged.document.PresentationResources.push_back(*resource);
        auto row = source;
        row.iStartMs -= occurrence.iStartMs;
        for (size_t axis = 0u; axis < row.PositionOffset.size(); ++axis) row.PositionOffset[axis] -= origin[axis];
        row.strAnchorKind = "BOSS";
        row.strSelectionGroupId.clear();
        staged.durationMs = (std::max)(staged.durationMs, uint32_t(endMs - occurrence.iStartMs));
        staged.pattern.PresentationOccurrences.push_back(std::move(row));
    }
    if (staged.pattern.PresentationOccurrences.size() < 2u)
    { error = "Selected airborne Effect group requires at least two members."; return false; }
    staged.pattern.iDurationMs = staged.durationMs;
    output = std::move(staged);
    error.clear();
    return true;
}

bool Client::CKoukuSaydonPresentationPlayer::Collect_LogicPreviewEffects(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, std::set<std::string>& targets)
{
    for (const auto& occurrence : pattern.LogicOccurrences)
    {
        if (const auto* selected = Find_SelectedAirborneLogic(document, occurrence))
        {
            PRODUCT_PATTERN group;
            if (!Build_SelectedAirbornePresentation(document, pattern, *selected, occurrence, group, m_strStatus)) return false;
            for (const auto& resource : group.document.PresentationResources)
                if (resource.strResourceKind == "V1_EFFECT" || resource.strResourceKind == "V1_ELEMENT")
                    targets.insert(resource.strAssetId);
            continue;
        }
        if (occurrence.bEnabled)
        {
            const auto pursuit = std::find_if(document.Logics.begin(), document.Logics.end(),
                [&](const auto& row) { return row.strLogicId == occurrence.strLogicId &&
                    (row.strJudgementKind == "PURSUIT_PROJECTILES" || row.strTriggerKind == "PURSUIT_PROJECTILES"); });
            if (pursuit != document.Logics.end())
            {
                auto ids = pursuit->PursuitVisualIds;
                ids.push_back(pursuit->strContactVisualId);
                for (const auto& id : ids)
                {
                    const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
                        [&](const auto& row) { return row.strResourceId == id && row.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT; });
                    if (resource == document.PresentationResources.end())
                    { m_strStatus = "Pursuit preparation requires its saved Effect resource: " + id; return false; }
                    targets.insert(resource->strAssetId);
                }
                continue;
            }
        }
        const auto* logic = Find_BlueCircleLogic(document, occurrence);
        if (!logic || !Supports_PlayerCenteredCircle(*logic)) continue;
        const auto* visual = Find_BlueCircleVisual(pattern);
        if (!visual || visual->activeEffectKind != BOSS_COMBAT_OBJECT_ACTIVE_EFFECT_KIND::EFFECT_V1 ||
            visual->effectAssetId.empty())
        {
            m_strStatus = "Player-centered Albion preview has no supported BossCatalog Effect visual.";
            return false;
        }
        targets.insert(visual->effectAssetId);
    }
    return true;
}

void Client::CKoukuSaydonPresentationPlayer::Sample_LogicPreview(SESSION& owner,
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, const float clockMs, const bool paused)
{
    if (!m_bPreviewPlaying || m_bModelReferencePreview || !std::isfinite(clockMs) || clockMs < 0.f) return;
    const bool previewOwner = &owner == &m_PreviewSession || std::any_of(
        m_BundlePreviewMembers.begin(), m_BundlePreviewMembers.end(),
        [&](const auto& member) { return &owner == &member.session; });
    // Child Effect sessions re-enter Sample through the existing resource path.
    // They never own gameplay Logic or recursively create another volley.
    if (!previewOwner) return;
    for (const auto& occurrence : pattern.LogicOccurrences)
    {
        if (occurrence.bEnabled)
        {
            const auto pursuit = std::find_if(document.Logics.begin(), document.Logics.end(),
                [&](const auto& row) { return row.strLogicId == occurrence.strLogicId &&
                    (row.strJudgementKind == "PURSUIT_PROJECTILES" || row.strTriggerKind == "PURSUIT_PROJECTILES"); });
            if (pursuit != document.Logics.end())
            {
                Sample_PursuitPreview(owner, document, pattern, *pursuit, occurrence, clockMs, paused);
                continue;
            }
        }
        if (const auto* selected = Find_SelectedAirborneLogic(document, occurrence))
        {
            auto& trigger = m_LogicPreviewTriggers[owner.key][occurrence.strOccurrenceId];
            const double startMs = std::ceil(double(occurrence.iStartMs) * .03) * (1000.0 / 30.0);
            const double ageMs = double(clockMs) - startMs;
            if (ageMs < 0.0)
            { for (auto& spawn : trigger.spawns) Stop_Session(spawn.session); continue; }
            if (!trigger.captured)
            {
                const auto member = std::find_if(m_BundlePreviewMembers.begin(), m_BundlePreviewMembers.end(),
                    [&](const auto& row) { return &row.session == &owner; });
                if (member == m_BundlePreviewMembers.end())
                { m_strStatus = "Selected airborne group requires the same actor preview selection owner."; continue; }
                const auto capture = member->airborneSelections.find(occurrence.strOccurrenceId);
                if (capture == member->airborneSelections.end())
                { m_strStatus = "Selected airborne group has no captured ground point."; continue; }
                PRODUCT_PATTERN presentation;
                if (!Build_SelectedAirbornePresentation(document, pattern, *selected, occurrence, presentation, m_strStatus)) continue;
                LOGIC_PREVIEW_SPAWN spawn;
                spawn.session.key = owner.key + ":selected:" + occurrence.strOccurrenceId;
                const auto& position = capture->second.second;
                DirectX::XMStoreFloat4x4(&spawn.pivot, DirectX::XMMatrixTranslation(position.x, position.y, position.z));
                trigger.presentation = std::move(presentation);
                trigger.spawns.push_back(std::move(spawn));
                trigger.captured = true;
            }
            for (auto& spawn : trigger.spawns)
                if (ageMs >= trigger.presentation.durationMs) Stop_Session(spawn.session);
                else Sample(spawn.session, trigger.presentation.document, trigger.presentation.pattern,
                    float(ageMs), paused, spawn.pivot, nullptr);
            continue;
        }
        const auto* logic = Find_BlueCircleLogic(document, occurrence);
        if (!logic || !Supports_PlayerCenteredCircle(*logic)) continue;
        auto& trigger = m_LogicPreviewTriggers[owner.key][occurrence.strOccurrenceId];
        // Server mechanic cues begin on the first 30 Hz tick at/after their box.
        const double startMs = std::ceil(double(occurrence.iStartMs) * .03) * (1000.0 / 30.0);
        const double ageMs = double(clockMs) - startMs;
        if (ageMs < 0.0 || ageMs >= logic->iEffectLifetimeMs)
        {
            for (auto& spawn : trigger.spawns) Stop_Session(spawn.session);
            continue;
        }
        if (!trigger.captured)
        {
            const auto* visual = Find_BlueCircleVisual(pattern);
            if (!visual || visual->activeEffectKind != BOSS_COMBAT_OBJECT_ACTIVE_EFFECT_KIND::EFFECT_V1 ||
                visual->effectAssetId.empty())
            { m_strStatus = "Albion preview cannot resolve its BossCatalog Effect visual."; continue; }
            PRODUCT_PATTERN presentation;
            presentation.pattern.strPatternId = pattern.strPatternId + ".logic-preview";
            presentation.durationMs = logic->iEffectLifetimeMs;
            presentation.pattern.iDurationMs = presentation.durationMs;
            KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE resource;
            resource.strResourceId = "preview.albion.bluecircle";
            resource.strResourceKind = "V1_EFFECT";
            resource.strAssetId = visual->effectAssetId;
            resource.iDurationMs = presentation.durationMs;
            presentation.document.PresentationResources.push_back(resource);
            KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE effect;
            effect.strOccurrenceId = occurrence.strOccurrenceId + ".effect";
            effect.strResourceId = resource.strResourceId;
            effect.iDurationMs = presentation.durationMs;
            effect.bFollowBoss = false;
            // Make_Pivot normalizes its anchor basis; retain the catalog's
            // authoritative visual scale on this private execution occurrence.
            effect.Scale = {visual->worldScale.x, visual->worldScale.y, visual->worldScale.z};
            presentation.pattern.PresentationOccurrences.push_back(effect);
            std::vector<LOGIC_PREVIEW_SPAWN> spawns;
            std::set<LostArk::Shared::NET_ENTITY_ID> seen;
            bool admitted = true;
            for (const auto& player : m_LogicPreviewPlayers)
            {
                if (!Is_EligiblePlayer(player) || !seen.insert(player.Snapshot.iNetEntityId).second) continue;
                const auto& snapshot = player.Snapshot;
                float3_t position{snapshot.fPositionX, snapshot.fPositionY, snapshot.fPositionZ};
                if (const auto character = player.pCharacter.lock())
                {
                    float3_t ground;
                    if (!character->Try_SampleTargetGround(position.x, position.z, ground) ||
                        !std::isfinite(ground.y))
                    { admitted = false; break; }
                    position.y = ground.y;
                }
                LOGIC_PREVIEW_SPAWN spawn;
                spawn.session.key = owner.key + ":logic:" + occurrence.strOccurrenceId + ":" +
                    std::to_string(snapshot.iNetEntityId);
                DirectX::XMStoreFloat4x4(&spawn.pivot,
                    DirectX::XMMatrixTranslation(position.x, position.y, position.z));
                spawns.push_back(std::move(spawn));
            }
            // Empty rooms consume the one-shot trigger too. Never emit late
            // when a player becomes eligible after the authored boundary.
            trigger.captured = true;
            trigger.presentation = std::move(presentation);
            if (admitted) trigger.spawns = std::move(spawns);
            else m_strStatus = "Albion preview preserved existing Effects: player ground is unavailable.";
        }
        for (auto& spawn : trigger.spawns)
            Sample(spawn.session, trigger.presentation.document, trigger.presentation.pattern,
                float(ageMs), paused, spawn.pivot, nullptr);
    }
}

void Client::CKoukuSaydonPresentationPlayer::Sample_PursuitPreview(SESSION& owner,
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, const LOGIC& logic,
    const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE& occurrence, const float clockMs, const bool paused)
{
    using namespace DirectX;
    using namespace LostArk::Shared;
    // This is the existing ToolPreview visual sampler, never a combat object or
    // a command producer. Random (non-homing) volleys still require Server Play.
    if (!logic.bPursuitHoming || logic.PursuitVisualIds.empty() ||
        logic.PursuitVisualIds.size() > 4u || !logic.iCountPerWave || logic.iCountPerWave > 16u)
    {
        m_strStatus = "Pursuit Preview supports homing waves; use Server Play for random volleys.";
        return;
    }
    constexpr double tickMs = 1000.0 / 30.0;
    constexpr std::size_t maxProjectiles = 64u, maxPoses = 65536u;
    constexpr std::uint32_t maxTicks = 18000u;
    // Server cadence, GameRoom_BossSimulation.cpp:857 and :915: interval zero fires
    // one wave and never reschedules; otherwise a wave fires every interval while
    // the Duration window is open. Ticks_FromMs rounds up, as the room clock does.
    const auto ticksFromMs = [](const std::uint32_t ms) {
        return static_cast<std::uint32_t>((std::uint64_t(ms) * 30u + 999u) / 1000u);
    };
    const std::uint32_t intervalTicks = logic.iSpawnIntervalMs ?
        (std::max)(1u, ticksFromMs(logic.iSpawnIntervalMs)) : 0u;
    const std::uint32_t windowTicks = intervalTicks ?
        ticksFromMs(occurrence.iStartMs + occurrence.iDurationMs) - ticksFromMs(occurrence.iStartMs) : 0u;
    const std::uint32_t waveCount = intervalTicks ?
        (std::max)(1u, (windowTicks + intervalTicks - 1u) / intervalTicks) : 1u;
    const double authoredStart = std::ceil(double(occurrence.iStartMs) / tickMs) * tickMs;
    auto& trigger = m_LogicPreviewTriggers[owner.key][occurrence.strOccurrenceId];
    const auto stop = [&]() {
        for (auto& projectile : trigger.projectiles)
        { Stop_Session(projectile.flight); Stop_Session(projectile.contact); }
    };
    if (double(clockMs) < authoredStart) { stop(); return; }
    std::size_t projectileCount = 0u, poseCount = 0u;
    for (const auto& [key, triggers] : m_LogicPreviewTriggers)
        for (const auto& [id, row] : triggers)
            for (const auto& projectile : row.projectiles)
            { ++projectileCount; poseCount += projectile.poses.size(); }
    const auto eligible = [](const KOUKU_CARD_PRESENTATION_VIEW& player) {
        return Is_EligiblePlayer(player) && player.Snapshot.eAction != PLAYER_ACTION_STATE::GRABBED;
    };
    const auto findPlayer = [&](NET_ENTITY_ID id) -> const KOUKU_CARD_PRESENTATION_VIEW* {
        const auto found = std::find_if(m_LogicPreviewPlayers.begin(), m_LogicPreviewPlayers.end(),
            [&](const auto& player) { return player.Snapshot.iNetEntityId == id && eligible(player); });
        return found == m_LogicPreviewPlayers.end() ? nullptr : &*found;
    };
    if (!trigger.captured)
    {
        if (logic.strLogicType != "TRIGGER" && double(clockMs) >= authoredStart + occurrence.iDurationMs) return;
        const KOUKU_CARD_PRESENTATION_VIEW* selected = nullptr;
        for (const auto& player : m_LogicPreviewPlayers)
            if (eligible(player) && (!selected || player.Snapshot.iNetEntityId < selected->Snapshot.iNetEntityId))
                selected = &player;
        if (!selected || (trigger.awaitingPlayer && paused))
        {
            trigger.awaitingPlayer = true;
            m_strStatus = "Pursuit Preview has no eligible player at its observed birth; waiting for an eligible target.";
            return;
        }
        const std::size_t plannedProjectiles = std::size_t(waveCount) * logic.iCountPerWave;
        if (projectileCount + plannedProjectiles > maxProjectiles || poseCount + plannedProjectiles > maxPoses)
        { m_strStatus = "Pursuit Preview reached its 64-projectile/65536-pose bound."; return; }
        const auto makePresentation = [&](const std::string& resourceId, const std::string& identity,
                                          const bool flight, PRODUCT_PATTERN& output) {
            const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
                [&](const auto& row) { return row.strResourceId == resourceId && row.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT; });
            if (resource == document.PresentationResources.end() || !resource->iDurationMs) return false;
            PRODUCT_PATTERN staged;
            staged.durationMs = flight ? (logic.iPursuitLifetimeMs ? logic.iPursuitLifetimeMs : 600000u) : resource->iDurationMs;
            staged.pattern.strPatternId = pattern.strPatternId + ".pursuit-preview";
            staged.pattern.iDurationMs = staged.durationMs;
            staged.document.PresentationResources.push_back(*resource);
            KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE box;
            box.strOccurrenceId = identity;
            box.strResourceId = resourceId;
            box.iDurationMs = staged.durationMs;
            box.bFollowBoss = flight;
            box.bLoopEffectToDuration = flight;
            staged.pattern.PresentationOccurrences.push_back(std::move(box));
            output = std::move(staged);
            return true;
        };
        PRODUCT_PATTERN contact;
        if (!makePresentation(logic.strContactVisualId, occurrence.strOccurrenceId + ".contact", false, contact))
        { m_strStatus = "Pursuit Preview cannot resolve the saved contact Effect."; return; }
        const double birthMs = trigger.awaitingPlayer ?
            std::ceil(double(clockMs) / tickMs) * tickMs : authoredStart;
        auto pivot = owner.rootRecordedPivot;
        std::string historyStatus;
        // A seek beyond observed history starts a visual epoch from the actual
        // current root and player snapshot. It never invents historical input.
        const bool knownRoot = owner.rootHistory && owner.rootHistory->Sample(float(birthMs / 1000.0), pivot, historyStatus);
        if (!knownRoot) pivot = owner.rootRecordedPivot;
        const float bossYaw = std::atan2(pivot._31, pivot._33);
        if (!std::isfinite(pivot._41) || !std::isfinite(pivot._42) || !std::isfinite(pivot._43) || !std::isfinite(bossYaw))
        { m_strStatus = "Pursuit Preview requires a finite observed boss root."; return; }
        std::vector<PURSUIT_PREVIEW_PROJECTILE> staged;
        for (std::uint32_t wave = 0u; wave < waveCount; ++wave)
        for (std::uint32_t ordinal = 0u; ordinal < logic.iCountPerWave; ++ordinal)
        {
            PURSUIT_PREVIEW_PROJECTILE projectile;
            // Same visual and bearing the room derives, GameRoom_BossSimulation.cpp:884 and :901.
            const std::uint32_t sequence = wave * logic.iCountPerWave + ordinal;
            const auto suffix = std::to_string(wave) + "." + std::to_string(ordinal);
            const auto identity = owner.key + ":pursuit:" + occurrence.strOccurrenceId + ":" + suffix;
            if (!makePresentation(logic.PursuitVisualIds[sequence % logic.PursuitVisualIds.size()],
                                  occurrence.strOccurrenceId + ".flight." + suffix, true, projectile.presentation))
            { m_strStatus = "Pursuit Preview cannot resolve a saved flight Effect."; return; }
            projectile.flight.key = identity + ":flight";
            projectile.contact.key = identity + ":contact";
            projectile.targetId = selected->Snapshot.iNetEntityId;
            projectile.birthTickOffset = wave * intervalTicks;
            // Same Saydon +X body-front bearing as the Server (180 degrees from the old back-facing origin).
            constexpr float bodyFrontYaw = XM_PIDIV2;
            const float yaw = bossYaw + bodyFrontYaw + XM_2PI * float(ordinal) / float(logic.iCountPerWave);
            float4x4_t pose;
            XMStoreFloat4x4(&pose, XMMatrixRotationY(yaw) * XMMatrixTranslation(
                pivot._41 + std::sin(yaw) * float(logic.fSpawnRadiusM), pivot._42,
                pivot._43 + std::cos(yaw) * float(logic.fSpawnRadiusM)));
            projectile.poses.push_back(pose);
            staged.push_back(std::move(projectile));
        }
        trigger.contactPresentation = std::move(contact);
        trigger.projectiles = std::move(staged);
        trigger.pursuitBirthMs = birthMs;
        trigger.captured = true;
        poseCount += trigger.projectiles.size();
        m_strStatus = knownRoot && double(clockMs) - birthMs < tickMs * 2.0 ?
            "Pursuit visual Preview follows the lowest eligible NetEntityId; Server target selection remains authoritative." :
            "Pursuit visual seek rebuilt from current observed anchors; this is not recorded Server history.";
    }
    const double ageMs = double(clockMs) - trigger.pursuitBirthMs;
    if (ageMs < 0.0) { stop(); return; }
    const auto requestedTick = static_cast<std::uint32_t>((std::min)(double(maxTicks), std::floor(ageMs / tickMs + 1.e-5)));
    const auto lifetimeTick = logic.iPursuitLifetimeMs ?
        static_cast<std::uint32_t>(std::ceil(double(logic.iPursuitLifetimeMs) / tickMs)) : maxTicks;
    for (auto& projectile : trigger.projectiles)
    {
        // Each wave runs on its own clock offset from the trigger birth, so a
        // later wave stays unborn until the authored interval has elapsed.
        if (requestedTick < projectile.birthTickOffset)
        { Stop_Session(projectile.flight); Stop_Session(projectile.contact); continue; }
        const std::uint32_t localTick = requestedTick - projectile.birthTickOffset;
        const double localAgeMs = ageMs - double(projectile.birthTickOffset) * tickMs;
        // The snapshot is fixed throughout an unobserved seek rebuild. A later
        // live tick observes a new input; paused/rewound stored ticks do not.
        const auto* target = findPlayer(projectile.targetId);
        while (projectile.terminalTick == UINT32_MAX && projectile.poses.size() <= localTick)
        {
            if (poseCount >= maxPoses)
            { stop(); m_strStatus = "Pursuit Preview reached its 65536-pose history bound."; return; }
            const auto tick = static_cast<std::uint32_t>(projectile.poses.size());
            auto next = projectile.poses.back();
            const auto previous = next;
            if (!target)
            {
                projectile.terminalTick = tick;
                projectile.contactBurst = false;
                m_strStatus = "Pursuit visual target became ineligible; this preview projectile was released.";
            }
            else
            {
                const auto& snapshot = target->Snapshot;
                const float dx = snapshot.fPositionX - previous._41, dz = snapshot.fPositionZ - previous._43;
                const float distance = std::hypot(dx, dz);
                const float yaw = distance > .000001f ? std::atan2(dx, dz) : std::atan2(previous._31, previous._33);
                float step = (std::min)(float(logic.fPursuitSpeedMps) / 30.f, distance);
                if (logic.fPursuitMaxDistanceM > 0.0)
                    step = (std::min)(step, (std::max)(0.f, float(logic.fPursuitMaxDistanceM) - projectile.traveledM));
                projectile.traveledM += step;
                XMStoreFloat4x4(&next, XMMatrixRotationY(yaw) * XMMatrixTranslation(
                    previous._41 + std::sin(yaw) * step, snapshot.fPositionY, previous._43 + std::cos(yaw) * step));
                const CombatCollision::CIRCLE_XZ circle{snapshot.fPositionX, snapshot.fPositionZ,
                    float(logic.fContactRadiusM) + WorldCollision::PLAYER_HALF_EXTENT_X};
                const bool hit = CombatCollision::Segment_IntersectsCircle(previous._41, previous._43, next._41, next._43, circle);
                const bool expired = (logic.iPursuitLifetimeMs && tick >= lifetimeTick) ||
                    (logic.fPursuitMaxDistanceM > 0.0 && projectile.traveledM >= float(logic.fPursuitMaxDistanceM));
                // Match product ordering: last-tick movement, contact, expiry.
                if (hit || expired)
                {
                    if (hit) { next._41 = snapshot.fPositionX; next._42 = snapshot.fPositionY; next._43 = snapshot.fPositionZ; }
                    projectile.terminalTick = tick;
                    projectile.contactBurst = true;
                }
            }
            projectile.poses.push_back(next);
            ++poseCount;
        }
        if (localTick >= projectile.terminalTick)
        {
            Stop_Session(projectile.flight);
            const double contactAgeMs = localAgeMs - double(projectile.terminalTick) * tickMs;
            if (!projectile.contactBurst || contactAgeMs >= trigger.contactPresentation.durationMs)
                Stop_Session(projectile.contact);
            else
                Sample(projectile.contact, trigger.contactPresentation.document, trigger.contactPresentation.pattern,
                    float(contactAgeMs), paused, projectile.poses.back(), nullptr);
        }
        else
        {
            Stop_Session(projectile.contact);
            if (!projectile.flight.rootHistory)
                projectile.flight.rootHistory = std::make_shared<EFFECT_V2_PIVOT_HISTORY>();
            std::string historyStatus;
            for (std::size_t tick = 0u; tick < projectile.poses.size(); ++tick)
            {
                const float seconds = float(double(tick) / 30.0);
                if (seconds <= projectile.flight.rootRecordedSeconds) continue;
                if (!projectile.flight.rootHistory->Record(seconds, projectile.poses[tick], false, historyStatus))
                { m_strStatus = historyStatus; stop(); return; }
                projectile.flight.rootRecordedSeconds = seconds;
                projectile.flight.rootRecordedPivot = projectile.poses[tick];
            }
            Sample(projectile.flight, projectile.presentation.document, projectile.presentation.pattern,
                float(localAgeMs), paused, projectile.poses[localTick], nullptr);
        }
    }
}

void Client::CKoukuSaydonPresentationPlayer::Clear_LogicPreview()
{
    for (auto& [owner, triggers] : m_LogicPreviewTriggers)
        for (auto& [id, trigger] : triggers)
        {
            for (auto& spawn : trigger.spawns) Stop_Session(spawn.session);
            for (auto& projectile : trigger.projectiles)
            { Stop_Session(projectile.flight); Stop_Session(projectile.contact); }
        }
    m_LogicPreviewTriggers.clear();
}

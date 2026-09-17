#include <WinSock2.h>
#include "KoukuSaydonPresentationPlayer.h"
#include "ActorCatalog.h"
#include "Character.h"

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
                    row.strJudgementKind == "PURSUIT_PROJECTILES"; });
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

void Client::CKoukuSaydonPresentationPlayer::Clear_LogicPreview()
{
    for (auto& [owner, triggers] : m_LogicPreviewTriggers)
        for (auto& [id, trigger] : triggers)
            for (auto& spawn : trigger.spawns) Stop_Session(spawn.session);
    m_LogicPreviewTriggers.clear();
}

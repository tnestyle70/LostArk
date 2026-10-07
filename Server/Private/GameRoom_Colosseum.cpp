#include "GameRoom.h"
#include "ClientSession.h"
#include "ColosseumCombatPolicy.h"
#include "ServerCombatGeometry.h"
#include "Network/PacketWriter.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <set>

using namespace LostArk::Server;
using namespace LostArk::Shared;

namespace
{
    constexpr float PI = 3.14159265358979323846f;
    constexpr std::uint32_t MERCENARY_ALT_V_INTERVAL_TICKS = 30u * 30u;
    constexpr std::array<CHARACTER_CLASS_ID, 5> MERCENARY_CLASSES = {
        CHARACTER_CLASS_ID::DIMENSIONMASTER, CHARACTER_CLASS_ID::LANCE_MASTER,
        CHARACTER_CLASS_ID::WARLORD, CHARACTER_CLASS_ID::GUARDIANKNIGHT,
        CHARACTER_CLASS_ID::ARTIST };
    constexpr std::array<const char*, 5> MERCENARY_NAMES = {
        "차원술사 용병", "창술사 용병", "워로드 용병", "가디언나이트 용병", "도화가 용병" };

    bool DeadlineReached(std::uint32_t now, std::uint32_t deadline)
    { return deadline == 0u || static_cast<std::int32_t>(now - deadline) >= 0; }

    // Stable per-match streams make decisions reproducible without synchronizing bots.
    float RandomUnit(std::uint64_t& state)
    {
        if (!state) state = 0x9e3779b97f4a7c15ULL;
        state ^= state >> 12; state ^= state << 25; state ^= state >> 27;
        return static_cast<float>((state * 2685821657736338717ULL) >> 40) / 16777216.f;
    }

    void BuildAvailableMercenarySkills(const CGameplayCatalog& catalog,
        const SERVER_PLAYER& merc, std::vector<SKILL_ID>& skills)
    {
        skills.clear();
        for (const auto& [id, skill] : catalog.Get_Skills())
            if (skill.eCharacterClass == merc.eCharacterClass &&
                (skill.eRequiredStance == PLAYER_STANCE_ID::NONE || skill.eRequiredStance == merc.eStance))
                skills.push_back(id);
        // Catalog iteration order must not change a seeded weighted draw.
        std::sort(skills.begin(), skills.end());
    }

    bool SkillResourcesReady(const SERVER_PLAYER& merc, const PLAYER_SKILL_DEFINITION& skill,
        std::uint32_t tick)
    {
        const auto cooldown = merc.CooldownEndTickBySkillId.find(skill.iSkillId);
        return skill.eCharacterClass == merc.eCharacterClass &&
            (skill.eRequiredStance == PLAYER_STANCE_ID::NONE || skill.eRequiredStance == merc.eStance) &&
            (cooldown == merc.CooldownEndTickBySkillId.end() || DeadlineReached(tick, cooldown->second)) &&
            merc.iCurrentResource >= skill.iResourceCost && merc.iCurrentIdentity >= skill.iIdentityCost &&
            (skill.eSetsStance == PLAYER_STANCE_ID::NONE || DeadlineReached(tick, merc.iStanceSwitchCooldownEndTick));
    }

    float HealthFraction(const SERVER_PLAYER& player)
    { return player.iMaximumHp ? static_cast<float>(player.iCurrentHp) / player.iMaximumHp : 0.f; }

    float SkillReach(const PLAYER_SKILL_DEFINITION& skill)
    { return (std::max)(2.f, skill.fMaximumRange); }

    struct DODGE_MOTION_POINT { float time = 0.f, forward = 0.f, lateral = 0.f; };
    struct DODGE_MOTION
    {
        std::array<DODGE_MOTION_POINT, 181> points{};
        std::size_t count = 1u;
        bool releaseHold = false;
    };

    // Read the same cumulative curve as PlayerSkillSystem::Update. These samples
    // are observations only; the normal skill executor still commits every move.
    std::pair<float, float> SampleDodgeMotion(const std::vector<ROOT_MOTION_SAMPLE>& samples, float seconds)
    {
        if (samples.empty()) return {};
        const float milliseconds = seconds * 1000.f;
        if (milliseconds <= float(samples.front().iTimeMs))
            return {samples.front().fForward, samples.front().fLateral};
        for (std::size_t index = 1u; index < samples.size(); ++index)
        {
            const auto& previous = samples[index - 1u]; const auto& current = samples[index];
            if (milliseconds > float(current.iTimeMs)) continue;
            const float span = float(current.iTimeMs) - float(previous.iTimeMs);
            const float alpha = span <= 0.f ? 0.f : (milliseconds - float(previous.iTimeMs)) / span;
            return {previous.fForward + (current.fForward - previous.fForward) * alpha,
                previous.fLateral + (current.fLateral - previous.fLateral) * alpha};
        }
        return {samples.back().fForward, samples.back().fLateral};
    }

    bool ReadDodgeMotion(const PLAYER_SKILL_DEFINITION& skill, float fixedSeconds, DODGE_MOTION& out)
    {
        const auto* samples = &skill.RootMotion;
        std::uint32_t durationMs = skill.iActionDurationMs;
        out = {};
        if (skill.eSkillKind == PLAYER_SKILL_KIND::HOLD && !skill.ComboStages.empty() &&
            skill.ComboStages.front().iComboAdvanceMs < skill.ComboStages.front().iActionDurationMs)
        {
            // An immediately released branching HOLD completes its start landing,
            // without the duration/route changing with a later tactical release.
            samples = &skill.ComboStages.front().RootMotion;
            durationMs = skill.ComboStages.front().iActionDurationMs;
            out.releaseHold = true;
        }
        else if (skill.eSkillKind != PLAYER_SKILL_KIND::ACTIVE && skill.eSkillKind != PLAYER_SKILL_KIND::STANDUP)
            return false;
        const float duration = float(durationMs) * .001f;
        if (!(fixedSeconds > 0.f) || !(duration > 0.f) || !std::isfinite(skill.fRootMotionScale)) return false;
        float elapsed = 0.f, forward = 0.f, lateral = 0.f;
        while (elapsed < duration)
        {
            if (out.count == out.points.size()) return false;
            elapsed += fixedSeconds;
            float deltaForward = 0.f, deltaLateral = 0.f;
            if (!samples->empty())
            {
                const auto previous = SampleDodgeMotion(*samples, (std::max)(0.f, elapsed - fixedSeconds));
                const auto current = SampleDodgeMotion(*samples, elapsed);
                deltaForward = current.first - previous.first;
                deltaLateral = current.second - previous.second;
            }
            else if (skill.fMovementDistance > 0.f)
                deltaForward = skill.fMovementDistance / duration * fixedSeconds;
            forward += deltaForward * skill.fRootMotionScale;
            lateral += deltaLateral * skill.fRootMotionScale;
            if (!std::isfinite(forward) || !std::isfinite(lateral)) return false;
            out.points[out.count++] = {elapsed, forward, lateral};
        }
        return true;
    }

    S2C_PLAYER_SPAWNED MakeSpawn(const SERVER_PLAYER& player)
    {
        S2C_PLAYER_SPAWNED message;
        message.iPlayerId = player.iPlayerId;
        message.iNetEntityId = player.iNetEntityId;
        message.eCharacterClass = player.eCharacterClass;
        message.eControlKind = player.eControlKind;
        message.strNickName = player.strNickName;
        message.iVoiceType = player.iVoiceType;
        message.strAppearanceJson = player.strAppearanceJson;
        message.fPositionX = player.fPositionX;
        message.fPositionY = player.fPositionY;
        message.fPositionZ = player.fPositionZ;
        message.fYawDegrees = player.fYawDegrees;
        return message;
    }


}

bool CGameRoom::Transfer_ColosseumMatchTo(CGameRoom& target,
    const std::vector<SESSION_ID>& orderedSeats, std::uint64_t matchId, std::string& status)
{
    const auto reject = [&status](const char* reason) { status = reason; return false; };
    if (m_eWorldId != WORLD_ID::BERN || target.m_eWorldId != WORLD_ID::COLOSSEUM ||
        !m_isReady || !target.m_isReady || matchId == 0u || (orderedSeats.empty() || orderedSeats.size() > MAX_COLOSSEUM_MATCH_PLAYERS) ||
        !target.m_Players.empty() || target.m_iColosseumMatchId != 0u ||
        std::set<SESSION_ID>(orderedSeats.begin(), orderedSeats.end()).size() != orderedSeats.size())
        return reject("invalid Colosseum match batch");
    const auto* hpProfile = target.m_GameplayCatalog.Find_Boss("BOSS_VALTAN");
    if (!hpProfile || hpProfile->iMaximumHp == 0u || hpProfile->iMaximumHealthBars != 160u)
        return reject("Colosseum requires the active Valtan 160-bar health profile");
    const auto referenceHp = hpProfile->Get_DamageReferenceHp();
    const auto matchHp = referenceHp / 8u + (referenceHp % 8u != 0u ? 1u : 0u);

    std::vector<STAGED_PLAYER_ENTRY> entries;
    std::vector<PLAYER_ID> departingIds;
    std::vector<NET_ENTITY_ID> departingEntities;
    std::array<std::uint32_t, 2> partyIds{ target.m_iNextPartyId, target.m_iNextPartyId + 1u };
    if (partyIds[0] == 0u || partyIds[1] == 0u ||
        target.m_PartyMembersByPartyId.contains(partyIds[0]) || target.m_PartyMembersByPartyId.contains(partyIds[1]))
        return reject("Colosseum party identity exhausted");
    entries.reserve(4);
    for (std::size_t index = 0; index < orderedSeats.size(); ++index)
    {
        const SESSION_ID sessionId = orderedSeats[index];
        const auto identity = m_PlayerIdBySessionId.find(sessionId);
        if (identity == m_PlayerIdBySessionId.end()) return reject("queued player left Bern");
        const auto& source = m_Players.at(identity->second);
        const auto session = Find_Session(sessionId);
        if (!source.Is_Human() || source.iCurrentHp == 0u || source.eAction != PLAYER_ACTION_STATE::NONE ||
            !session || session->Is_Closing() ||
            std::none_of(m_ColosseumQueue.begin(), m_ColosseumQueue.end(),
                [sessionId](const auto& entry) { return entry.iSessionId == sessionId; }))
            return reject("queued player is no longer available");
        if (const auto party = m_PartyIdByPlayerId.find(source.iPlayerId); party != m_PartyIdByPlayerId.end())
        {
            const auto members = m_PartyMembersByPartyId.find(party->second);
            if (members == m_PartyMembersByPartyId.end() || members->second.size() != 1u)
                return reject("queued player joined another party");
        }
        if (std::any_of(m_RaidEntryProposals.begin(), m_RaidEntryProposals.end(),
            [&source](const auto& vote) { return std::find(vote.Voters.begin(), vote.Voters.end(), source.iPlayerId) != vote.Voters.end(); }))
            return reject("queued player owns an open world-entry vote");
        C2S_ENTER_WORLD enter;
        enter.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
        enter.eWorldId = WORLD_ID::COLOSSEUM;
        enter.eCharacterClass = source.eCharacterClass;
        enter.strNickName = source.strNickName;
        enter.iVoiceType = source.iVoiceType;
        enter.strAppearanceJson = source.strAppearanceJson;
        STAGED_PLAYER_ENTRY entry;
        SESSION_DIAGNOSTIC_REASON reason;
        if (!target.Stage_PlayerEntry(session, enter, entries, entry, reason, status,
            {}, source.Inventory, source.iHonorTitleId, {}, source.Purse, true)) return false;
        // Team spawn is an authored player standing point, not the NPC-approach override.
        const std::string spawnId = std::string("player.spawn.colosseum.team") +
            (index % 2u ? "b.0" : "a.0") + std::to_string(index / 2u + 1u);
        const auto* spawn = target.Find_Placement(spawnId);
        SERVER_NAV_POINT position;
        if (!spawn || !target.m_ServerNavigation.Sample_Position(spawn->fPositionX, spawn->fPositionZ,
            position, spawn->fPositionY) ||
            !target.m_ServerCollisionSystem.Is_PlayerPositionClear(position.x, position.y, position.z, entry.Player.iNetEntityId))
            return reject("Colosseum team spawn failed navigation/collision admission");
        for (const auto& preceding : entries)
            if (std::abs(preceding.Player.fPositionY - position.y) < 1.5f &&
                std::hypot(preceding.Player.fPositionX - position.x, preceding.Player.fPositionZ - position.z) < .75f)
                return reject("Colosseum team spawn overlaps an admitted human");
        auto& player = entry.Player;
        player.Inventory = source.Inventory;
        player.Purse = source.Purse;
        player.bRestoreAvailable = false;
        player.strSpawnPlacementId = spawnId;
        player.fPositionX = position.x; player.fPositionY = position.y; player.fPositionZ = position.z;
        player.fYawDegrees = spawn->fYawDegrees;
        player.iColosseumMatchId = matchId;
        player.iColosseumTeam = static_cast<std::uint8_t>(index % 2u);
        player.iColosseumArrivalIndex = static_cast<std::uint8_t>(index);
        player.fColosseumSpawnX = position.x; player.fColosseumSpawnY = position.y;
        player.fColosseumSpawnZ = position.z; player.fColosseumSpawnYaw = player.fYawDegrees;
        player.bColosseumParticipant = true;
        player.isCombatReady = false;
        player.iColosseumDamageReferenceHp = referenceHp;
        player.iCurrentHp = player.iMaximumHp = matchHp;
        departingIds.push_back(source.iPlayerId);
        departingEntities.push_back(source.iNetEntityId);
        entries.push_back(std::move(entry));
    }

    auto players = target.m_Players;
    auto sessionPlayers = target.m_PlayerIdBySessionId;
    auto entityPlayers = target.m_PlayerIdByEntityId;
    auto sessions = target.m_Sessions;
    auto playerParties = target.m_PartyIdByPlayerId;
    auto parties = target.m_PartyMembersByPartyId;
    auto mercenaries = target.m_ColosseumMercenaries;
    std::vector<SERVER_PLAYER> candidates;
    candidates.reserve(10);
    for (const auto& entry : entries)
    {
        const auto& player = entry.Player;
        players.emplace(player.iPlayerId, player);
        sessionPlayers.emplace(player.iSessionId, player.iPlayerId);
        entityPlayers.emplace(player.iNetEntityId, player.iPlayerId);
        sessions.emplace(player.iSessionId, entry.pSession);
        playerParties.emplace(player.iPlayerId, partyIds[player.iColosseumTeam]);
        parties[partyIds[player.iColosseumTeam]].push_back(player.iPlayerId);
    }
    for (std::uint8_t team = 0u; team < 2u; ++team)
    {
        SERVER_PLAYER anchor;
        const auto human = std::find_if(entries.begin(), entries.end(), [team](const auto& entry) { return entry.Player.iColosseumTeam == team; });
        if (human != entries.end()) anchor = human->Player;
        else
        {
            const auto* spawn = target.Find_Placement(team ? "player.spawn.colosseum.teamb.01" : "player.spawn.colosseum.teama.01");
            if (!spawn) return reject("Colosseum empty-team spawn missing");
            anchor.fPositionX = spawn->fPositionX; anchor.fPositionY = spawn->fPositionY;
            anchor.fPositionZ = spawn->fPositionZ; anchor.fYawDegrees = spawn->fYawDegrees;
        }
        // A solo entrant owns only their own team's recruitment. The opposite
        // team receives four real sessionless mercenaries in this same transaction.
        const bool automaticSoloOpponent = orderedSeats.size() == 1u && human == entries.end();
        auto& teamMembers = parties[partyIds[team]];
        const float yaw = anchor.fYawDegrees * PI / 180.f;
        for (std::size_t slot = 0u; slot < MERCENARY_CLASSES.size(); ++slot)
        {
            const auto* profile = target.m_GameplayCatalog.Find_Player(MERCENARY_CLASSES[slot]);
            if (!profile) return reject("Colosseum mercenary class profile missing");
            SERVER_PLAYER merc;
            merc.eControlKind = PLAYER_CONTROL_KIND::COLOSSEUM_MERCENARY_AI;
            merc.iPlayerId = target.m_iNextPlayerId + static_cast<PLAYER_ID>(orderedSeats.size() + candidates.size());
            merc.iNetEntityId = target.m_iNextNetEntityId + static_cast<NET_ENTITY_ID>(orderedSeats.size() + candidates.size());
            merc.eCharacterClass = MERCENARY_CLASSES[slot];
            merc.strNickName = MERCENARY_NAMES[slot];
            merc.eStance = profile->eDefaultStance;
            merc.iColosseumDamageReferenceHp = referenceHp;
            merc.iCurrentHp = merc.iMaximumHp = matchHp;
            merc.iCurrentResource = merc.iMaximumResource = profile->iMaximumResource;
            merc.iMaximumIdentity = profile->iMaximumIdentity;
            merc.fMoveSpeed = profile->fMoveSpeed;
            CPlayerSkillSystem::Reset_Gauges(merc, target.m_GameplayCatalog);
            merc.iColosseumMatchId = matchId; merc.iColosseumTeam = team;
            // Human-owned teams, including the solo player's team, still recruit manually.
            merc.bColosseumParticipant = automaticSoloOpponent && slot < 4u;
            merc.isCombatReady = false;
            merc.bColosseumReady = true;
            if (merc.bColosseumParticipant)
            {
                merc.iColosseumArrivalIndex = static_cast<std::uint8_t>(team + teamMembers.size() * 2u);
                teamMembers.push_back(merc.iPlayerId);
                playerParties.emplace(merc.iPlayerId, partyIds[team]);
            }
            merc.fYawDegrees = anchor.fYawDegrees;
            const float lateral = (static_cast<float>(slot) - 2.f) * 1.5f;
            const float x = anchor.fPositionX + std::sin(yaw) * 4.f + std::cos(yaw) * lateral;
            const float z = anchor.fPositionZ + std::cos(yaw) * 4.f - std::sin(yaw) * lateral;
            SERVER_NAV_POINT landing;
            bool admitted = false;
            unsigned navRejected = 0u, heightRejected = 0u, collisionRejected = 0u, overlapRejected = 0u, pathRejected = 0u;
            // Same bounded rings and exact navigation/collision admission as
            // Find_GuideLanding, with the complete uncommitted batch included.
            for (unsigned sample = 0u; sample < 49u && !admitted; ++sample)
            {
                const float radius = sample ? .75f + static_cast<float>((sample - 1u) / 12u) * .75f : 0.f;
                const float angle = static_cast<float>(sample % 12u) * PI / 6.f;
                const float candidateX = x + std::sin(angle) * radius;
                const float candidateZ = z + std::cos(angle) * radius;
                const float forward = (candidateX - anchor.fPositionX) * std::sin(yaw) +
                    (candidateZ - anchor.fPositionZ) * std::cos(yaw);
                if (forward < 1.f || std::hypot(candidateX - anchor.fPositionX, candidateZ - anchor.fPositionZ) > 7.5f) continue;
                SERVER_NAV_POINT point;
                if (!target.m_ServerNavigation.Sample_Position(candidateX, candidateZ, point, anchor.fPositionY))
                { ++navRejected; continue; }
                if (std::abs(point.y - anchor.fPositionY) > 1.f) { ++heightRejected; continue; }
                if (!target.m_ServerCollisionSystem.Is_PlayerPositionClear(point.x, point.y, point.z, merc.iNetEntityId))
                { ++collisionRejected; continue; }
                const bool overlap = std::any_of(players.begin(), players.end(), [&point](const auto& value)
                {
                    return std::abs(value.second.fPositionY - point.y) < 1.5f &&
                        std::hypot(value.second.fPositionX - point.x, value.second.fPositionZ - point.z) < .75f;
                });
                if (overlap) { ++overlapRejected; continue; }
                if (!target.m_ServerNavigation.Has_LineOfSight(anchor.fPositionX, anchor.fPositionZ,
                    point.x, point.z, anchor.fPositionY)) { ++pathRejected; continue; }
                landing = point; admitted = true;
            }
            if (!admitted)
            {
                status = "Colosseum mercenary admission team=" + std::to_string(team) + " slot=" + std::to_string(slot) +
                    " desired=" + std::to_string(x) + "," + std::to_string(z) + " nav=" + std::to_string(navRejected) +
                    " height=" + std::to_string(heightRejected) + " collision=" + std::to_string(collisionRejected) +
                    " overlap=" + std::to_string(overlapRejected) + " path=" + std::to_string(pathRejected);
                return false;
            }
            merc.fPositionX = landing.x; merc.fPositionY = landing.y; merc.fPositionZ = landing.z;
            merc.fColosseumSpawnX = landing.x; merc.fColosseumSpawnY = landing.y;
            merc.fColosseumSpawnZ = landing.z; merc.fColosseumSpawnYaw = merc.fYawDegrees;
            players.emplace(merc.iPlayerId, merc);
            entityPlayers.emplace(merc.iNetEntityId, merc.iPlayerId);
            COLOSSEUM_MERCENARY_RUNTIME ai;
            BuildAvailableMercenarySkills(target.m_GameplayCatalog.Active(), merc, ai.AvailableSkills);
            if (ai.AvailableSkills.empty())
            { status = "Colosseum mercenary has no published class skills"; return false; }
            ai.iRandomState = matchId ^ (static_cast<std::uint64_t>(merc.iNetEntityId) * 0x9e3779b97f4a7c15ULL);
            ai.iObservedHp = merc.iCurrentHp;
            mercenaries.emplace(merc.iPlayerId, std::move(ai));
            candidates.push_back(std::move(merc));
        }
    }

    const auto append = [&status](std::vector<PACKET_FRAME>& frames, PACKET_TYPE type, const auto& message)
    {
        CPacketWriter writer;
        if (!Write_Message(writer, message)) { status = "Colosseum admission message encoding failed"; return false; }
        frames.push_back({ type, writer.Get_Buffer() }); return true;
    };
    S2C_COLOSSEUM_MATCH_STATE state;
    state.iMatchId = matchId; state.iRevision = 1u;
    state.ePhase = COLOSSEUM_MATCH_PHASE::LOADING;
    state.iServerTick = target.m_iServerTick; state.iPhaseStartTick = target.m_iServerTick;
    state.iPhaseEndTick = target.m_iServerTick + 3600u;
    for (const auto& [id, player] : players)
    {
        COLOSSEUM_MATCH_PLAYER_STATE row;
        row.iPlayerId = id; row.iNetEntityId = player.iNetEntityId; row.iTeam = player.iColosseumTeam;
        row.bParticipant = player.bColosseumParticipant; row.iArrivalIndex = player.iColosseumArrivalIndex;
        row.bReady = player.bColosseumReady; row.iKills = player.iColosseumKills;
        state.Players.push_back(row);
        if (row.bParticipant) state.Participants.push_back(row);
    }
    state.iExpectedPlayers = static_cast<std::uint8_t>(state.Participants.size());
    S2C_COLOSSEUM_MATCH_FOUND found;
    found.iMatchId = matchId;
    for (const auto& entry : entries)
        found.Participants.push_back({ entry.Player.strNickName, entry.Player.eCharacterClass, entry.Player.iColosseumTeam });
    std::vector<CLIENT_SESSION_RELIABLE_BATCH> outboundBatches;
    for (std::size_t index = 0; index < entries.size(); ++index)
    {
        auto& entry = entries[index];
        if (!target.Build_PlayerEntryFrames(entry, entries, status)) return false;
        std::vector<PACKET_FRAME> frames;
        found.iLocalIndex = static_cast<std::uint8_t>(index);
        if (!append(frames, PACKET_TYPE::S2C_COLOSSEUM_MATCH_FOUND, found)) return false;
        frames.insert(frames.end(), entry.Frames.begin(), entry.Frames.end());
        for (const auto& candidate : candidates)
            if (!append(frames, PACKET_TYPE::S2C_PLAYER_SPAWNED, MakeSpawn(candidate))) return false;
        S2C_PARTY_ROSTER roster;
        for (const auto id : parties.at(partyIds[entry.Player.iColosseumTeam]))
        {
            const auto& member = players.at(id);
            roster.Members.push_back({ member.iNetEntityId, member.strNickName, member.eCharacterClass, member.eControlKind });
        }
        if (!append(frames, PACKET_TYPE::S2C_PARTY_ROSTER, roster) ||
            !append(frames, PACKET_TYPE::S2C_COLOSSEUM_MATCH_STATE, state)) return false;
        outboundBatches.push_back({ entry.pSession, std::move(frames) });
    }
    for (const auto& [id, player] : m_Players)
    {
        if (!player.Is_Human() || std::find(orderedSeats.begin(), orderedSeats.end(), player.iSessionId) != orderedSeats.end()) continue;
        CLIENT_SESSION_RELIABLE_BATCH observer{ Find_Session(player.iSessionId), {} };
        for (const auto entityId : departingEntities)
        {
            S2C_PLAYER_DESPAWNED despawn;
            despawn.iNetEntityId = entityId; despawn.eReason = PLAYER_DESPAWN_REASON::LEVEL_CHANGED;
            if (!append(observer.Frames, PACKET_TYPE::S2C_PLAYER_DESPAWNED, despawn)) return false;
        }
        outboundBatches.push_back(std::move(observer));
    }
    auto expectedSessions = orderedSeats;
    CClientSession::RELIABLE_BATCH_TRANSACTION outbound;
    if (!outbound.Prepare(outboundBatches, status)) return false;
    // All allocations and FIFO capacity checks precede the first source mutation.
    for (const auto playerId : departingIds)
    {
        const auto party = m_PartyIdByPlayerId.find(playerId);
        if (party != m_PartyIdByPlayerId.end())
        {
            m_PartyMembersByPartyId.erase(party->second);
            m_PartyIdByPlayerId.erase(party);
        }
    }
    for (const auto sessionId : orderedSeats) Leave(sessionId, PLAYER_DESPAWN_REASON::LEVEL_CHANGED, false);
    target.m_Players.swap(players); target.m_PlayerIdBySessionId.swap(sessionPlayers);
    target.m_PlayerIdByEntityId.swap(entityPlayers); target.m_Sessions.swap(sessions);
    target.m_PartyIdByPlayerId.swap(playerParties); target.m_PartyMembersByPartyId.swap(parties);
    target.m_ColosseumMercenaries.swap(mercenaries);
    target.m_iColosseumMatchId = matchId; target.m_ColosseumTeamPartyIds = partyIds;
    target.m_ColosseumSessions.swap(expectedSessions);
    target.m_eColosseumPhase = COLOSSEUM_MATCH_PHASE::LOADING;
    target.m_iColosseumPhaseStart = target.m_iServerTick; target.m_iColosseumPhaseEnd = target.m_iServerTick + 3600u;
    for (const auto& entry : entries) ++target.m_ColosseumInitialHumans[entry.Player.iColosseumTeam];
    target.m_iNextPlayerId += static_cast<PLAYER_ID>(target.m_Players.size());
    target.m_iNextNetEntityId += static_cast<NET_ENTITY_ID>(target.m_Players.size()); target.m_iNextPartyId += 2u;
    for (const auto& entry : entries) entry.pSession->Bind_PlayerId(entry.Player.iPlayerId);
    outbound.Commit();
    status = "Colosseum match committed for " + std::to_string(orderedSeats.size()) + " human players";
    return true;
}

void CGameRoom::Notify_ColosseumTransferResult(bool committed)
{
    m_bColosseumTransferPending = false;
    m_iColosseumRetryTick = committed ? 0u : m_iServerTick + 30u;
    if (committed) m_iColosseumQueueDeadline = m_ColosseumQueue.empty() ? 0u : m_iServerTick + 300u;
    for (const auto& entry : m_ColosseumQueue)
        Send_ColosseumQueueState(entry.iSessionId, COLOSSEUM_QUEUE_STATE::WAITING);
}

S2C_COLOSSEUM_MATCH_STATE CGameRoom::Build_ColosseumState() const
{
    S2C_COLOSSEUM_MATCH_STATE state;
    state.iMatchId = m_iColosseumMatchId; state.ePhase = m_eColosseumPhase;
    state.iWinnerTeam = state.iWinningTeam = m_iColosseumWinnerTeam; state.iRevision = m_iColosseumRevision;
    state.iServerTick = m_iServerTick; state.iPhaseStartTick = m_iColosseumPhaseStart; state.iPhaseEndTick = m_iColosseumPhaseEnd;
    state.iLeftScore = m_iColosseumScores[0]; state.iRightScore = m_iColosseumScores[1]; state.RecentKills = m_ColosseumRecentKills;
    for (const auto& [id, player] : m_Players)
        if (player.iColosseumMatchId == m_iColosseumMatchId && player.iColosseumTeam < 2u)
        {
            COLOSSEUM_MATCH_PLAYER_STATE row;
            row.iPlayerId = id; row.iNetEntityId = player.iNetEntityId; row.iTeam = player.iColosseumTeam;
            row.bParticipant = player.bColosseumParticipant; row.iArrivalIndex = player.iColosseumArrivalIndex;
            row.bReady = player.bColosseumReady; row.iKills = player.iColosseumKills;
            state.Players.push_back(row);
            if (row.bParticipant) state.Participants.push_back(row);
        }
    state.iExpectedPlayers = static_cast<std::uint8_t>(state.Participants.size());
    return state;
}

void CGameRoom::Broadcast_ColosseumState()
{
    if (m_iColosseumMatchId == 0u) return;
    CPacketWriter writer;
    if (!Write_Message(writer, Build_ColosseumState())) return;
    for (const auto& [id, player] : m_Players)
        if (player.Is_Human())
            if (const auto session = Find_Session(player.iSessionId); session &&
                !session->Send_Frame(PACKET_TYPE::S2C_COLOSSEUM_MATCH_STATE, writer.Get_Buffer())) session->Request_Close();
}

void CGameRoom::Handle_ColosseumRecruit(SESSION_ID sessionId, const C2S_COLOSSEUM_RECRUIT& request)
{
    if (m_eWorldId != WORLD_ID::COLOSSEUM || m_iColosseumMatchId == 0u || request.iMatchId != m_iColosseumMatchId) return;
    const auto actorId = m_PlayerIdBySessionId.find(sessionId);
    const auto candidateId = m_PlayerIdByEntityId.find(request.iMercenaryNetEntityId);
    if (actorId == m_PlayerIdBySessionId.end() || candidateId == m_PlayerIdByEntityId.end()) return;
    auto& actor = m_Players.at(actorId->second);
    auto& candidate = m_Players.at(candidateId->second);
    auto& sequence = m_ColosseumRecruitSequences[sessionId];
    if (request.iRequestSequence == 0u || (sequence != 0u && static_cast<std::int32_t>(request.iRequestSequence - sequence) <= 0))
    { Broadcast_ColosseumState(); return; }
    sequence = request.iRequestSequence;
    if (m_eColosseumPhase != COLOSSEUM_MATCH_PHASE::RECRUITING || !actor.Is_Human() || actor.iCurrentHp == 0u ||
        !candidate.Is_ColosseumMercenary() || candidate.iColosseumMatchId != m_iColosseumMatchId ||
        actor.iColosseumTeam >= 2u || candidate.iColosseumTeam != actor.iColosseumTeam ||
        candidate.bColosseumParticipant || std::abs(actor.fPositionY - candidate.fPositionY) > 2.f ||
        std::hypot(actor.fPositionX - candidate.fPositionX, actor.fPositionZ - candidate.fPositionZ) > 8.f)
    { Broadcast_ColosseumState(); return; }
    auto& members = m_PartyMembersByPartyId.at(m_ColosseumTeamPartyIds[actor.iColosseumTeam]);
    if (members.size() >= 4u) { Broadcast_ColosseumState(); return; }
    candidate.iColosseumArrivalIndex = static_cast<std::uint8_t>(actor.iColosseumTeam + members.size() * 2u);
    members.push_back(candidate.iPlayerId);
    m_PartyIdByPlayerId.emplace(candidate.iPlayerId, m_ColosseumTeamPartyIds[actor.iColosseumTeam]);
    candidate.bColosseumParticipant = true;
    ++m_iColosseumRevision;
    Try_StartColosseumEntry();
    Broadcast_PartyRoster(m_ColosseumTeamPartyIds[actor.iColosseumTeam]);
    Broadcast_ColosseumState();
}

void CGameRoom::Try_StartColosseumEntry()
{
    if (m_eColosseumPhase != COLOSSEUM_MATCH_PHASE::RECRUITING) return;
    for (const auto partyId : m_ColosseumTeamPartyIds)
    {
        const auto found = m_PartyMembersByPartyId.find(partyId);
        if (found == m_PartyMembersByPartyId.end() || found->second.size() != 4u) return;
    }
    m_eColosseumPhase = COLOSSEUM_MATCH_PHASE::ENTRY_COUNTDOWN;
    m_iColosseumPhaseStart = m_iServerTick; m_iColosseumPhaseEnd = m_iServerTick + 300u;
    ++m_iColosseumRevision;
    for (auto& [id, player] : m_Players)
    {
        player.hasMoveGoal = false; player.MovePath.clear(); player.PendingCommand.Clear();
        player.isCombatReady = player.bColosseumCombatActive = false;
    }
}

bool CGameRoom::Try_SealColosseumForRetirement()
{
    if (m_iColosseumMatchId == 0u || m_eWorldId != WORLD_ID::COLOSSEUM) return false;
    std::scoped_lock lock{ m_CommandMutex };
    if (!m_acceptsCommands) return true;
    if (!m_InboundCommands.empty() || !m_CleanupCommands.empty() || !m_QueuedCleanupSessionIds.empty() ||
        !m_PendingWorldTransfers.empty() || !m_Sessions.empty() || !m_PlayerIdBySessionId.empty() || Count_HumanPlayers() != 0u) return false;
    m_acceptsCommands = false;
    return true;
}

void CGameRoom::Update_Colosseum(float seconds)
{
    if (m_eWorldId == WORLD_ID::BERN)
    {
        if (!m_bColosseumTransferPending && (m_iColosseumRetryTick == 0u ||
            static_cast<std::int32_t>(m_iServerTick - m_iColosseumRetryTick) >= 0)) Try_FormColosseumMatch();
        return;
    }
    if (m_eWorldId != WORLD_ID::COLOSSEUM || m_iColosseumMatchId == 0u || m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::FINISHED) return;
    std::array<unsigned, 2> humans{};
    for (const auto& [id, player] : m_Players)
        if (player.Is_Human() && player.iColosseumTeam < 2u) ++humans[player.iColosseumTeam];
    // A team intentionally created without humans is supported. Losing an admitted
    // human while recruiting aborts the match instead of stranding its allies.
    if ((m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::RECRUITING || m_eColosseumPhase == COLOSSEUM_MATCH_PHASE::LOADING) &&
        (humans[0] < m_ColosseumInitialHumans[0] || humans[1] < m_ColosseumInitialHumans[1]))
    {
        m_eColosseumPhase = COLOSSEUM_MATCH_PHASE::FINISHED;
        m_iColosseumPhaseStart = m_iServerTick; m_iColosseumPhaseEnd = 0u; m_iColosseumWinnerTeam = COLOSSEUM_NO_TEAM;
        for (auto& [id, player] : m_Players)
        {
            player.isCombatReady = player.bColosseumCombatActive = false;
            player.hasMoveGoal = false; player.MovePath.clear(); player.PendingCommand.Clear();
        }
        ++m_iColosseumRevision; Broadcast_ColosseumState(); return;
    }
    if (m_eColosseumPhase != COLOSSEUM_MATCH_PHASE::ACTIVE) return;
    for (auto& [playerId, runtime] : m_ColosseumMercenaries)
    {
        auto actor = m_Players.find(playerId);
        if (actor == m_Players.end()) continue;
        auto& merc = actor->second;
        if (!merc.bColosseumParticipant || merc.iColosseumMatchId != m_iColosseumMatchId) continue;
        if (!merc.iCurrentHp)
        {
            runtime.iTargetEntityId = INVALID_NET_ENTITY_ID;
            runtime.eTactic = COLOSSEUM_TACTIC::WAIT;
            runtime.iObservedHp = 0u; runtime.iNextAttackTick = runtime.iNextEvadeTick = 0u;
            runtime.fSenseElapsed = 1.f; runtime.fThreatAgeSeconds = 0.f;
            runtime.pReason = "Waiting for the server respawn";
            continue;
        }
        if (!runtime.iRandomState)
            runtime.iRandomState = m_iColosseumMatchId ^ (std::uint64_t(merc.iNetEntityId) * 0x9e3779b97f4a7c15ULL);
        if (runtime.iObservedHp && merc.iCurrentHp < runtime.iObservedHp) runtime.iLastDamageTick = m_iServerTick;
        runtime.iObservedHp = merc.iCurrentHp;
        runtime.fThinkElapsed += seconds; runtime.fSenseElapsed += seconds;
        const bool thinking = runtime.fThinkElapsed >= runtime.fDecisionInterval || runtime.AvailableSkills.empty();
        if (thinking)
        {
            runtime.fThinkElapsed = 0.f;
            runtime.fDecisionInterval = .16f + RandomUnit(runtime.iRandomState) * .12f;
            BuildAvailableMercenarySkills(m_GameplayCatalog.Active(), merc, runtime.AvailableSkills);
        }
        // Observations are refreshed at 10 Hz; the cached moving shapes are sampled
        // every fixed tick. Tactical lotteries never run at the simulation frequency.
        if (runtime.fSenseElapsed >= .1f || thinking)
        {
            runtime.Threats.Observe(merc, m_Players, m_CombatObjectRuntime.Get_LiveObjects(),
                m_GameplayCatalog.Active(), m_iServerTick);
            runtime.fSenseElapsed = 0.f;
        }
        const auto immediate = runtime.Threats.At(merc.fPositionX, merc.fPositionY, merc.fPositionZ, runtime.fSenseElapsed);
        const auto approaching = runtime.Threats.At(merc.fPositionX, merc.fPositionY, merc.fPositionZ, runtime.fSenseElapsed + .3f);
        const float risk = (std::max)((std::max)(immediate.risk, approaching.risk),
            runtime.Threats.Along(merc.fPositionX, merc.fPositionY, merc.fPositionZ,
                merc.fPositionX, merc.fPositionY, merc.fPositionZ, .65f, runtime.fSenseElapsed));
        runtime.fThreatAgeSeconds = risk > 0.f ? runtime.fThreatAgeSeconds + seconds : 0.f;
        const auto eligibleEnemy = [&](const SERVER_PLAYER& other)
        {
            return other.bColosseumParticipant && other.iColosseumMatchId == m_iColosseumMatchId &&
                other.iColosseumTeam < 2u && other.iColosseumTeam != merc.iColosseumTeam &&
                other.iCurrentHp && other.isCombatReady && std::abs(other.fPositionY - merc.fPositionY) < 3.f;
        };
        const SERVER_PLAYER* enemy = nullptr;
        for (const auto& [id, other] : m_Players)
            if (other.iNetEntityId == runtime.iTargetEntityId && eligibleEnemy(other)) enemy = &other;
        if (thinking || !enemy)
        {
            const SERVER_PLAYER* best = enemy; float bestScore = -1.f, retainedScore = -1.f;
            for (const auto& [id, other] : m_Players)
            {
                if (!eligibleEnemy(other)) continue;
                const float distance = std::hypot(other.fPositionX - merc.fPositionX, other.fPositionZ - merc.fPositionZ);
                float score = 6.f / (2.f + distance) + (1.f - HealthFraction(other)) * 1.5f;
                if (other.Has_TimeStop(m_iServerTick) || !DeadlineReached(m_iServerTick, other.iInvulnerableEndTick)) score *= .15f;
                if (other.eAction == PLAYER_ACTION_STATE::KNOCKDOWN) score += .7f;
                for (const auto& [allyId, ally] : m_Players)
                {
                    if (!ally.bColosseumParticipant || ally.iColosseumMatchId != m_iColosseumMatchId ||
                        ally.iColosseumTeam != merc.iColosseumTeam || !ally.iCurrentHp || allyId == playerId) continue;
                    // Help a pressured ally and mildly prefer the team's current focus.
                    if (HealthFraction(ally) < .45f &&
                        std::hypot(other.fPositionX - ally.fPositionX, other.fPositionZ - ally.fPositionZ) < 4.f) score += .6f;
                    const auto allyBrain = m_ColosseumMercenaries.find(allyId);
                    if (allyBrain != m_ColosseumMercenaries.end() && allyBrain->second.iTargetEntityId == other.iNetEntityId) score += .15f;
                }
                if (m_ServerNavigation.Is_Loaded() && !m_ServerNavigation.Has_LineOfSight(merc.fPositionX, merc.fPositionZ,
                    other.fPositionX, other.fPositionZ, merc.fPositionY)) score *= .5f;
                if (&other == enemy) retainedScore = score;
                if (score > bestScore) { best = &other; bestScore = score; }
            }
            if (enemy && best != enemy && (bestScore < retainedScore * 1.25f ||
                (m_iServerTick - runtime.iTargetSelectedTick < 30u && bestScore < retainedScore * 1.7f))) best = enemy;
            if (best != enemy || !enemy)
            {
                runtime.iTargetEntityId = best ? best->iNetEntityId : INVALID_NET_ENTITY_ID;
                runtime.iTargetSelectedTick = m_iServerTick;
            }
            enemy = best;
        }
        if (merc.bPatternBound || merc.fKnockbackRemainingSeconds > 0.f || merc.TriggerMove.isActive ||
            merc.Has_TimeStop(m_iServerTick) || merc.eAction == PLAYER_ACTION_STATE::DEAD || merc.eAction == PLAYER_ACTION_STATE::FALLING)
        { runtime.pReason = "Waiting for crowd control to end"; continue; }
        const auto altVReady = [&](const PLAYER_SKILL_DEFINITION& skill)
        {
            return skill.strInputSlot != "ALT_V" || !runtime.iLastAltVAdmissionTick ||
                m_iServerTick - *runtime.iLastAltVAdmissionTick >= MERCENARY_ALT_V_INTERVAL_TICKS;
        };
        const auto trySkill = [&](const PLAYER_SKILL_DEFINITION& skill, float x, float z)
        {
            if (!altVReady(skill) || !SkillResourcesReady(merc, skill, m_iServerTick + 1u)) return false;
            C2S_USE_SKILL command;
            command.iClientSequence = ++runtime.iSequence; command.iSkillId = skill.iSkillId;
            command.eTargetIntent = skill.eTargetIntent; command.fAimX = x; command.fAimZ = z;
            if (!Execute_PlayerSkill(merc, command) || merc.eAction != PLAYER_ACTION_STATE::SKILL ||
                merc.iCurrentSkillId != skill.iSkillId || merc.PendingCommand.eKind != PLAYER_PENDING_COMMAND_KIND::NONE) return false;
            // Only an actual admission changes memory, never a pending COMBO command.
            if (skill.strInputSlot == "ALT_V") runtime.iLastAltVAdmissionTick = m_iServerTick;
            else if (!Is_DodgeSkill(skill) && skill.eSkillKind != PLAYER_SKILL_KIND::STANDUP)
            {
                runtime.iLastSkillId = skill.iSkillId;
                runtime.RecentSkills[runtime.iRecentSkillCursor++ % runtime.RecentSkills.size()] = skill.iSkillId;
            }
            return true;
        };
        const auto moveTo = [&](const SERVER_NAV_POINT& point)
        {
            if (merc.eAction != PLAYER_ACTION_STATE::NONE && !Is_MoveCancellableAction(merc)) return false;
            if (merc.hasMoveGoal && std::hypot(merc.fMoveGoalX - point.x, merc.fMoveGoalZ - point.z) < .05f) return true;
            C2S_MOVE move; move.iClientSequence = ++runtime.iSequence; move.fGoalX = point.x; move.fGoalZ = point.z;
            Execute_PlayerMove(merc, move);
            return merc.hasMoveGoal && merc.PendingCommand.eKind == PLAYER_PENDING_COMMAND_KIND::NONE &&
                std::hypot(merc.fMoveGoalX - point.x, merc.fMoveGoalZ - point.z) < .05f;
        };
        const float hp = HealthFraction(merc);
        unsigned nearbyEnemies = 0u, nearbyAllies = 0u;
        for (const auto& [id, other] : m_Players)
        {
            if (id == playerId || !other.bColosseumParticipant || other.iColosseumMatchId != m_iColosseumMatchId || !other.iCurrentHp) continue;
            if (std::hypot(other.fPositionX - merc.fPositionX, other.fPositionZ - merc.fPositionZ) > 6.f) continue;
            if (other.iColosseumTeam == merc.iColosseumTeam) ++nearbyAllies; else ++nearbyEnemies;
        }
        const float nearest = enemy ? std::hypot(enemy->fPositionX - merc.fPositionX, enemy->fPositionZ - merc.fPositionZ) : 0.f;
        // Range preference follows the currently available attacks, not a class-name script.
        float preferredRange = 2.f, reachSum = 0.f; unsigned reachCount = 0u;
        for (const auto id : runtime.AvailableSkills)
            if (const auto* skill = m_GameplayCatalog.Find_Skill(id); skill && skill->strInputSlot != "ALT_V" &&
                !Is_DodgeSkill(*skill) && skill->eSkillKind != PLAYER_SKILL_KIND::STANDUP &&
                SkillResourcesReady(merc, *skill, m_iServerTick + 1u))
            { reachSum += std::clamp(SkillReach(*skill) * .65f, 1.5f, 7.f); ++reachCount; }
        if (reachCount) preferredRange = reachSum / reachCount;
        const auto choosePosition = [&](bool escape, bool retreat, float radius, float speed, SERVER_NAV_POINT& out)
        {
            bool found = false; float bestScore = (std::numeric_limits<float>::max)();
            const auto scorePoint = [&](const SERVER_NAV_POINT& point)
            {
                const float distance = std::hypot(point.x - merc.fPositionX, point.z - merc.fPositionZ);
                const float travel = distance / (std::max)(speed, 1.f);
                const float endRisk = runtime.Threats.At(point.x, point.y, point.z, runtime.fSenseElapsed + travel).risk;
                const float pathRisk = runtime.Threats.Along(merc.fPositionX, merc.fPositionY, merc.fPositionZ,
                    point.x, point.y, point.z, travel, runtime.fSenseElapsed);
                float score = endRisk * 10.f + pathRisk * 4.f;
                float closestEnemy = 100.f, closestAlly = 100.f;
                for (const auto& [id, other] : m_Players)
                {
                    if (id == playerId || !other.bColosseumParticipant || other.iColosseumMatchId != m_iColosseumMatchId || !other.iCurrentHp) continue;
                    const float gap = std::hypot(point.x - other.fPositionX, point.z - other.fPositionZ);
                    if (other.iColosseumTeam == merc.iColosseumTeam) closestAlly = (std::min)(closestAlly, gap);
                    else closestEnemy = (std::min)(closestEnemy, gap);
                    if (gap < 1.4f) score += (1.4f - gap) * 2.f;
                }
                if (retreat) score -= (std::min)(closestEnemy, 8.f) * .8f;
                else if (enemy) score += std::abs(std::hypot(point.x - enemy->fPositionX, point.z - enemy->fPositionZ) - preferredRange) * (escape ? .08f : .4f);
                if (retreat && closestAlly < 100.f) score += (std::max)(0.f, closestAlly - 4.f) * .12f;
                if (merc.hasMoveGoal && std::hypot(point.x - merc.fMoveGoalX, point.z - merc.fMoveGoalZ) < .6f) score -= .35f;
                return std::pair{score, endRisk};
            };
            const float stay = scorePoint({merc.fPositionX, merc.fPositionY, merc.fPositionZ}).first;
            for (unsigned i = 0u; i < 25u; ++i)
            {
                if (i == 24u && !merc.hasMoveGoal) continue;
                const float angle = float(i % 12u) * PI / 6.f;
                const float length = i < 12u ? radius : radius * .5f;
                const float x = i == 24u ? merc.fMoveGoalX : merc.fPositionX + std::sin(angle) * length;
                const float z = i == 24u ? merc.fMoveGoalZ : merc.fPositionZ + std::cos(angle) * length;
                SERVER_NAV_POINT point;
                if (!Find_GuideLanding(merc, x, merc.fPositionY, z, point) ||
                    !m_ServerNavigation.Has_LineOfSight(merc.fPositionX, merc.fPositionZ, point.x, point.z, merc.fPositionY)) continue;
                const auto [score, endRisk] = scorePoint(point);
                if (escape && endRisk >= risk) continue;
                if (score >= bestScore || (!escape && score >= stay - .15f)) continue;
                // Leaving the current hit may cross its boundary, but a second
                // dangerous zone must not be preferred just for a safe endpoint.
                if (escape && score > stay + 1.f) continue;
                out = point; bestScore = score; found = true;
            }
            return found;
        };
        const auto chooseDodge = [&](const PLAYER_SKILL_DEFINITION& skill, SERVER_NAV_POINT& aim, bool& releaseHold)
        {
            DODGE_MOTION motion;
            if (!ReadDodgeMotion(skill, seconds, motion)) return false;
            bool found = false; float bestScore = (std::numeric_limits<float>::max)();
            const float stayRisk = runtime.Threats.Along(merc.fPositionX, merc.fPositionY, merc.fPositionZ,
                merc.fPositionX, merc.fPositionY, merc.fPositionZ, .65f, runtime.fSenseElapsed);
            for (unsigned direction = 0u; direction < 12u; ++direction)
            {
                const float angle = float(direction) * PI / 6.f;
                const float forwardX = std::sin(angle), forwardZ = std::cos(angle);
                SERVER_PLAYER pose;
                pose.iMarioStage = merc.iMarioStage;
                pose.fPositionX = merc.fPositionX; pose.fPositionY = merc.fPositionY; pose.fPositionZ = merc.fPositionZ;
                bool valid = true; float pathRisk = 0.f;
                for (std::size_t step = 1u; step < motion.count; ++step)
                {
                    const auto& previous = motion.points[step - 1u]; const auto& current = motion.points[step];
                    const float deltaForward = current.forward - previous.forward;
                    const float deltaLateral = current.lateral - previous.lateral;
                    SERVER_NAV_POINT next{pose.fPositionX + forwardX * deltaForward + forwardZ * deltaLateral,
                        pose.fPositionY, pose.fPositionZ + forwardZ * deltaForward - forwardX * deltaLateral};
                    if (deltaForward != 0.f || deltaLateral != 0.f)
                    {
                        bool clamped = false;
                        if (m_ServerNavigation.Is_Loaded())
                            CPlayerSkillSystem::Clamp_StepToWalkable(m_ServerNavigation,
                                pose.fPositionX, pose.fPositionZ, next.x, next.z, next, clamped, pose.fPositionY);
                        // Reject an altered route, instead of inventing where a
                        // native wall clamp or body slide would eventually land.
                        if (clamped) { valid = false; break; }
                        float x = next.x, y = next.y, z = next.z; bool blocked = false;
                        if (!m_ServerCollisionSystem.Resolve_PlayerMove(pose, next.x, next.y, next.z, x, y, z, blocked) ||
                            blocked || std::abs(x-next.x) > .001f || std::abs(y-next.y) > .001f || std::abs(z-next.z) > .001f)
                        { valid = false; break; }
                    }
                    pathRisk = (std::max)(pathRisk, runtime.Threats.Along(pose.fPositionX, pose.fPositionY, pose.fPositionZ,
                        next.x, next.y, next.z, current.time-previous.time, runtime.fSenseElapsed+previous.time, 0.f));
                    pose.fPositionX = next.x; pose.fPositionY = next.y; pose.fPositionZ = next.z;
                }
                if (!valid || std::hypot(pose.fPositionX-merc.fPositionX, pose.fPositionZ-merc.fPositionZ) < .25f) continue;
                const float endRisk = runtime.Threats.At(pose.fPositionX, pose.fPositionY, pose.fPositionZ,
                    runtime.fSenseElapsed+motion.points[motion.count-1u].time).risk;
                if (risk > 0.f && (endRisk >= risk || pathRisk > stayRisk)) continue;
                float score = endRisk * 10.f + pathRisk * 4.f;
                if (enemy) score += std::abs(std::hypot(pose.fPositionX-enemy->fPositionX,
                    pose.fPositionZ-enemy->fPositionZ)-preferredRange) * .08f;
                for (const auto& [id, other] : m_Players)
                {
                    if (id == playerId || !other.bColosseumParticipant || other.iColosseumMatchId != m_iColosseumMatchId || !other.iCurrentHp) continue;
                    const float gap = std::hypot(pose.fPositionX-other.fPositionX, pose.fPositionZ-other.fPositionZ);
                    if (gap < 1.4f) score += (1.4f-gap) * 2.f;
                }
                if (score >= bestScore) continue;
                // Aim selects a direction only. It is deliberately distinct from
                // the predicted landing; native motion never caps at aim distance.
                aim = {merc.fPositionX+forwardX, merc.fPositionY, merc.fPositionZ+forwardZ};
                releaseHold = motion.releaseHold; bestScore = score; found = true;
            }
            return found;
        };
        if (merc.eAction == PLAYER_ACTION_STATE::KNOCKDOWN)
        {
            if (thinking)
                for (const auto id : runtime.AvailableSkills)
                    if (const auto* skill = m_GameplayCatalog.Find_Skill(id); skill && skill->eSkillKind == PLAYER_SKILL_KIND::STANDUP)
                    {
                        SERVER_NAV_POINT goal{merc.fPositionX, merc.fPositionY, merc.fPositionZ};
                        bool releaseHold = false;
                        (void)chooseDodge(*skill, goal, releaseHold);
                        if (trySkill(*skill, goal.x, goal.z))
                        { runtime.eTactic = COLOSSEUM_TACTIC::RECOVER; runtime.pReason = "Stand-up toward a safer position"; break; }
                    }
            continue;
        }
        const auto* running = merc.eAction == PLAYER_ACTION_STATE::SKILL ? m_GameplayCatalog.Find_Skill(merc.iCurrentSkillId) : nullptr;
        if (risk > 0.f && runtime.fThreatAgeSeconds >= .065f && DeadlineReached(m_iServerTick, runtime.iNextEvadeTick))
        {
            bool escaped = false;
            // The COMBO executor buffers different skills before checking cancel
            // admission. Preserve that boundary; a queued SPACE is not a dodge.
            if (!running || running->eSkillKind != PLAYER_SKILL_KIND::COMBO)
                for (const auto id : runtime.AvailableSkills)
                    if (const auto* skill = m_GameplayCatalog.Find_Skill(id); skill && Is_DodgeSkill(*skill) &&
                        SkillResourcesReady(merc, *skill, m_iServerTick + 1u))
                    {
                        SERVER_NAV_POINT aim; bool releaseHold = false;
                        if (chooseDodge(*skill, aim, releaseHold) && trySkill(*skill, aim.x, aim.z))
                        {
                            if (releaseHold)
                            {
                                C2S_RELEASE_SKILL release; release.iClientSequence = ++runtime.iSequence; release.iSkillId = skill->iSkillId;
                                m_PlayerSkillSystem.Release(merc, release, m_GameplayCatalog);
                            }
                            escaped = true; runtime.pReason = "Dodge admitted along its authored motion route"; break;
                        }
                    }
            if (!escaped)
            {
                SERVER_NAV_POINT safe;
                if (choosePosition(true, false, 3.f, merc.fMoveSpeed, safe) && moveTo(safe))
                { escaped = true; runtime.pReason = "Walking out of a predicted attack"; }
            }
            runtime.iNextEvadeTick = m_iServerTick + (escaped ? 6u : 3u);
            if (escaped)
            { runtime.eTactic = COLOSSEUM_TACTIC::EVADE; runtime.iTacticUntilTick = m_iServerTick + 9u; continue; }
        }
        // Native staged inputs stay tick-accurate even while the tactical clock sleeps.
        if (running && running->eSkillKind == PLAYER_SKILL_KIND::COMBO && enemy &&
            !merc.hasBufferedComboInput && merc.iComboStage > 0u && merc.iComboStage < running->ComboStages.size())
        {
            const auto& stage = running->ComboStages[merc.iComboStage - 1u];
            const float nowMs = merc.fActionElapsedSeconds * 1000.f;
            if (stage.iInputCloseMs && nowMs >= stage.iInputOpenMs && nowMs <= stage.iInputCloseMs)
            {
                C2S_USE_SKILL continuation;
                continuation.iClientSequence = ++runtime.iSequence; continuation.iSkillId = running->iSkillId;
                continuation.eTargetIntent = running->eTargetIntent;
                continuation.fAimX = enemy->fPositionX; continuation.fAimZ = enemy->fPositionZ;
                (void)Execute_PlayerSkill(merc, continuation);
                if (merc.hasBufferedComboInput) runtime.pReason = "Buffered the current skill continuation";
            }
        }
        if (running && running->eSkillKind == PLAYER_SKILL_KIND::HOLD && !merc.hasReleasedHold &&
            merc.fActionElapsedSeconds >= (risk > 0.f ? .25f : 1.f))
        {
            C2S_RELEASE_SKILL release; release.iClientSequence = ++runtime.iSequence; release.iSkillId = running->iSkillId;
            m_PlayerSkillSystem.Release(merc, release, m_GameplayCatalog);
        }
        if (!thinking) continue;
        if (!enemy) { runtime.eTactic = COLOSSEUM_TACTIC::WAIT; runtime.pReason = "No living opponent in this match"; continue; }
        if (runtime.eTactic == COLOSSEUM_TACTIC::EVADE && !DeadlineReached(m_iServerTick, runtime.iTacticUntilTick)) continue;
        const bool hurtRecently = runtime.iLastDamageTick && m_iServerTick - runtime.iLastDamageTick < 45u;
        if (runtime.eTactic == COLOSSEUM_TACTIC::RETREAT && DeadlineReached(m_iServerTick, runtime.iTacticUntilTick))
        {
            // PvP offers no passive full heal: retreat buys space, then counterattack.
            runtime.eTactic = COLOSSEUM_TACTIC::ENGAGE;
            runtime.iRetreatAllowedTick = m_iServerTick + 60u;
        }
        else if (runtime.eTactic != COLOSSEUM_TACTIC::RETREAT && DeadlineReached(m_iServerTick, runtime.iRetreatAllowedTick) &&
            ((hp < .3f && (nearbyEnemies || hurtRecently)) || (hp < .55f && nearbyEnemies > nearbyAllies + 1u)))
        { runtime.eTactic = COLOSSEUM_TACTIC::RETREAT; runtime.iTacticUntilTick = m_iServerTick + 45u; }
        if (runtime.eTactic == COLOSSEUM_TACTIC::RETREAT)
        {
            SERVER_NAV_POINT safe;
            if (choosePosition(false, true, 4.f, merc.fMoveSpeed, safe) && moveTo(safe))
            { runtime.pReason = "Regrouping away from pressure before re-engaging"; continue; }
        }
        if (merc.eAction != PLAYER_ACTION_STATE::NONE)
        { runtime.pReason = "Preserving the admitted skill and its native input windows"; continue; }
        if (risk > 0.f)
        { runtime.pReason = "Threat remains; withholding a new attack commitment"; continue; }
        if (!DeadlineReached(m_iServerTick, runtime.iNextAttackTick)) continue;
        struct CHOICE { const PLAYER_SKILL_DEFINITION* skill; float weight, x, z; };
        std::vector<CHOICE> choices;
        const PLAYER_SKILL_DEFINITION* awakening = nullptr;
        for (const auto id : runtime.AvailableSkills)
        {
            const auto* skill = m_GameplayCatalog.Find_Skill(id);
            if (!skill || Is_DodgeSkill(*skill) || skill->eSkillKind == PLAYER_SKILL_KIND::STANDUP ||
                !SkillResourcesReady(merc, *skill, m_iServerTick + 1u) || nearest > SkillReach(*skill)) continue;
            if (skill->strInputSlot == "ALT_V") { if (altVReady(*skill)) awakening = skill; continue; }
            float weight = skill->strInputSlot == "LMB" ? .45f : 1.f;
            const float castSeconds = (std::max)(.15f, skill->iActionDurationMs * .001f);
            weight *= 1.f / (1.f + castSeconds * static_cast<float>(nearbyEnemies) * .25f);
            if (enemy->eAction == PLAYER_ACTION_STATE::KNOCKDOWN) weight *= 1.f + (std::min)(castSeconds, 2.f);
            if (HealthFraction(*enemy) < .3f) weight *= 1.f + 1.f / castSeconds;
            if (skill->eSetsStance != PLAYER_STANCE_ID::NONE) weight *= reachCount <= 2u ? 2.f : .35f;
            if (const auto* buffs = m_GameplayCatalog.Active().Find_SkillBuffs(id))
                for (const auto& buff : *buffs)
                    if (buff.eTarget != CGameplayCatalog::SKILL_BUFF_TARGET::ENEMY)
                    {
                        const bool active = std::any_of(merc.ActiveBuffs.begin(), merc.ActiveBuffs.end(),
                            [&](const auto& row) { return row.iBuffId == buff.iBuffId; });
                        weight *= active ? .25f : (buff.iShieldPercentOfMaxHp && hp < .6f ? 2.5f : 1.2f);
                    }
            if (!DeadlineReached(m_iServerTick, enemy->iInvulnerableEndTick) || enemy->Has_TimeStop(m_iServerTick)) weight *= .05f;
            if (id == runtime.iLastSkillId) weight *= .12f;
            else if (std::find(runtime.RecentSkills.begin(), runtime.RecentSkills.end(), id) != runtime.RecentSkills.end()) weight *= .5f;
            float x = enemy->fPositionX, z = enemy->fPositionZ;
            if (enemy->hasMoveGoal)
            {
                const float dx = enemy->fMoveGoalX - x, dz = enemy->fMoveGoalZ - z;
                const float length = std::hypot(dx, dz);
                const float lead = (std::min)(length, enemy->fMoveSpeed * std::clamp(skill->iHitTimeMs * .001f, .1f, .35f));
                if (length > .01f) { x += dx * lead / length; z += dz * lead / length; }
            }
            choices.push_back({skill, (std::max)(.01f, weight), x, z});
        }
        bool started = false;
        // Awakening has its own tactical opportunity and 30-second admission gate.
        if (awakening && (choices.empty() || nearbyEnemies >= 2u || HealthFraction(*enemy) < .4f) &&
            trySkill(*awakening, enemy->fPositionX, enemy->fPositionZ))
        { runtime.pReason = "Awakening admitted at a tactical opportunity"; started = true; }
        while (!started && !choices.empty())
        {
            float total = 0.f; for (const auto& choice : choices) total += choice.weight;
            float draw = RandomUnit(runtime.iRandomState) * total;
            std::size_t index = choices.size() - 1u;
            for (std::size_t i = 0u; i < choices.size(); ++i)
                if ((draw -= choices[i].weight) <= 0.f) { index = i; break; }
            const auto selected = choices[index];
            started = trySkill(*selected.skill, selected.x, selected.z);
            choices.erase(choices.begin() + index);
            if (started) runtime.pReason = "Weighted skill choice matched range, resources and pressure";
        }
        if (started)
        {
            runtime.eTactic = COLOSSEUM_TACTIC::ENGAGE;
            // The admitted action owns its real duration, including early HOLD
            // release and interruption. Never wait for a nominal duration afterward.
            runtime.iNextAttackTick = m_iServerTick + 3u +
                static_cast<std::uint32_t>(RandomUnit(runtime.iRandomState) * 6.f);
            continue;
        }
        SERVER_NAV_POINT position;
        if (choosePosition(false, false, 3.f, merc.fMoveSpeed, position) && moveTo(position))
        { runtime.eTactic = COLOSSEUM_TACTIC::REPOSITION; runtime.pReason = "Repositioning for a useful attack range"; }
        else { runtime.eTactic = COLOSSEUM_TACTIC::WAIT; runtime.pReason = "Holding position while useful skills recover"; }
    }
}

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
    constexpr std::array<CHARACTER_CLASS_ID, 5> MERCENARY_CLASSES = {
        CHARACTER_CLASS_ID::DIMENSIONMASTER, CHARACTER_CLASS_ID::LANCE_MASTER,
        CHARACTER_CLASS_ID::WARLORD, CHARACTER_CLASS_ID::GUARDIANKNIGHT,
        CHARACTER_CLASS_ID::ARTIST };
    constexpr std::array<const char*, 5> MERCENARY_NAMES = {
        "차원술사 용병", "창술사 용병", "워로드 용병", "가디언나이트 용병", "도화가 용병" };

    std::size_t MercenarySlotRank(const std::string& slot)
    {
        constexpr std::array<const char*, 15> order = {
            "LMB", "Q", "W", "E", "R", "A", "S", "D", "F", "T", "V", "ALT_V", "Z", "X", "SPACE" };
        const auto found = std::find(order.begin(), order.end(), slot);
        return static_cast<std::size_t>(found - order.begin());
    }

    // Reuse storage and resolve every currently available class/stance binding.
    // Preserve the next slot, rather than a stale index, when stance changes the list.
    void BuildAvailableMercenarySkills(const CGameplayCatalog& catalog,
        const SERVER_PLAYER& merc, std::vector<SKILL_ID>& skills, std::size_t* cursor = nullptr)
    {
        std::size_t nextRank = 0u;
        if (cursor && *cursor < skills.size())
            if (const auto* next = catalog.Find_Skill(skills[*cursor]))
                nextRank = MercenarySlotRank(next->strInputSlot);
        skills.clear();
        for (const auto& [id, skill] : catalog.Get_Skills())
            if (skill.eCharacterClass == merc.eCharacterClass &&
                (skill.eRequiredStance == PLAYER_STANCE_ID::NONE || skill.eRequiredStance == merc.eStance))
                skills.push_back(id);
        std::sort(skills.begin(), skills.end(), [&](const auto left, const auto right)
        {
            const auto& leftSlot = catalog.Find_Skill(left)->strInputSlot;
            const auto& rightSlot = catalog.Find_Skill(right)->strInputSlot;
            const auto leftRank = MercenarySlotRank(leftSlot), rightRank = MercenarySlotRank(rightSlot);
            if (leftRank != rightRank) return leftRank < rightRank;
            if (leftSlot != rightSlot) return leftSlot < rightSlot;
            return left < right;
        });
        if (cursor)
        {
            const auto next = std::find_if(skills.begin(), skills.end(), [&](const auto id) {
                return MercenarySlotRank(catalog.Find_Skill(id)->strInputSlot) >= nextRank;
            });
            *cursor = next == skills.end() ? 0u : static_cast<std::size_t>(next - skills.begin());
        }
    }

    // Only DimensionMaster follows the authored fixed rotation without LMB.
    bool BuildMercenaryCombo(const CGameplayCatalog& catalog, const CGuideCatalog& guide,
        const SERVER_PLAYER& merc, std::vector<SKILL_ID>& skills, float& timeout, float& stepWait)
    {
        if (merc.eCharacterClass != CHARACTER_CLASS_ID::DIMENSIONMASTER)
        {
            BuildAvailableMercenarySkills(catalog, merc, skills);
            return !skills.empty();
        }
        if (!guide.Loaded || guide.Combos.empty()) return false;
        const auto& authored = guide.Combos.front();
        timeout = static_cast<float>(authored.TimeoutMs) * .001f;
        stepWait = static_cast<float>(authored.StepWaitMs) * .001f;
        for (const auto& slot : authored.Slots)
        {
            if (slot == "LMB" || slot == "SPACE") continue;
            const PLAYER_SKILL_DEFINITION* resolved = nullptr;
            for (const auto& [id, skill] : catalog.Get_Skills())
                if (skill.eCharacterClass == merc.eCharacterClass && skill.strInputSlot == slot &&
                    (skill.eRequiredStance == PLAYER_STANCE_ID::NONE || skill.eRequiredStance == merc.eStance))
                {
                    if (resolved) return false;
                    resolved = &skill;
                }
            if (resolved) skills.push_back(resolved->iSkillId);
        }
        return !skills.empty() && timeout > 0.f && stepWait > 0.f;
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

    bool HitOverlaps(const PLAYER_SKILL_HIT& hit, const SERVER_PLAYER& source,
        float x, float z)
    {
        SERVER_COMBAT_SHAPE_XZ shape;
        shape.eKind = hit.iAreaType == 1u ? (hit.fInner > 0.f ?
            SERVER_COMBAT_SHAPE_KIND::RING : SERVER_COMBAT_SHAPE_KIND::CIRCLE) :
            hit.iAreaType == 2u ? SERVER_COMBAT_SHAPE_KIND::FORWARD_BOX :
            hit.iAreaType == 3u ? SERVER_COMBAT_SHAPE_KIND::CONE : SERVER_COMBAT_SHAPE_KIND::END;
        shape.fOffset = hit.fOffset;
        // Geometry validates unused fields as zero; each primitive owns only its extent.
        if (hit.iAreaType == 1u)
        {
            shape.fOuterRadius = hit.fRange; shape.fInnerRadius = hit.fInner;
        }
        else if (hit.iAreaType == 2u)
        {
            shape.fLength = hit.fRange; shape.fHalfWidth = hit.fWidth * .5f;
        }
        else if (hit.iAreaType == 3u)
        {
            shape.fLength = hit.fRange; shape.fInnerRadius = hit.fInner;
            shape.fAngleDegrees = hit.fAngleDegrees <= 0.f ? 360.f : (std::min)(360.f, hit.fAngleDegrees);
        }
        const float angle = source.fYawDegrees * PI / 180.f;
        return CServerCombatGeometry::Is_Valid(shape) &&
            CServerCombatGeometry::Overlaps_Pose(shape, source.fPositionX, source.fPositionZ,
                std::sin(angle), std::cos(angle), { x, z, .5f });
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
    const auto matchHp = hpProfile->iMaximumHp / 4u + (hpProfile->iMaximumHp % 4u != 0u ? 1u : 0u);

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
            {}, source.Inventory, source.iHonorTitleId, {}, source.Purse)) return false;
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
        player.iColosseumDamageReferenceHp = hpProfile->iMaximumHp;
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
        const bool automaticTeam = human == entries.end();
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
            merc.iColosseumDamageReferenceHp = hpProfile->iMaximumHp;
            merc.iCurrentHp = merc.iMaximumHp = matchHp;
            merc.iCurrentResource = merc.iMaximumResource = profile->iMaximumResource;
            merc.iMaximumIdentity = profile->iMaximumIdentity;
            merc.fMoveSpeed = profile->fMoveSpeed;
            CPlayerSkillSystem::Reset_Gauges(merc, target.m_GameplayCatalog);
            merc.iColosseumMatchId = matchId; merc.iColosseumTeam = team;
            merc.bColosseumParticipant = automaticTeam && slot < 4u; merc.isCombatReady = false;
            merc.bColosseumReady = true;
            if (merc.bColosseumParticipant)
            {
                merc.iColosseumArrivalIndex = static_cast<std::uint8_t>(team + teamMembers.size() * 2u);
                teamMembers.push_back(merc.iPlayerId); playerParties.emplace(merc.iPlayerId, partyIds[team]);
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
            if (!BuildMercenaryCombo(target.m_GameplayCatalog.Active(), target.m_GuideCatalog,
                merc, ai.ComboSkills, ai.fComboTimeout, ai.fStepWaitTimeout))
                return reject("Colosseum mercenary combo has no valid published class/stance rotation");
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
    m_iColosseumPhaseStart = m_iServerTick; m_iColosseumPhaseEnd = m_iServerTick + 90u;
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
        auto& merc = m_Players.at(playerId);
        if (!merc.bColosseumParticipant || merc.iCurrentHp == 0u) continue;
        const bool fixedRotation = merc.eCharacterClass == CHARACTER_CLASS_ID::DIMENSIONMASTER;
        runtime.fThinkElapsed += seconds;
        if (fixedRotation) runtime.fComboElapsed += seconds;
        // Manual COMBO input is sampled on every fixed Server tick, independently
        // of the 5 Hz tactical think. A narrow authored input window must not be lost.
        const auto* running = merc.eAction == PLAYER_ACTION_STATE::SKILL ?
            m_GameplayCatalog.Find_Skill(merc.iCurrentSkillId) : nullptr;
        if (!merc.bPatternBound && merc.fKnockbackRemainingSeconds <= 0.f && !merc.TriggerMove.isActive &&
            running && running->eSkillKind == PLAYER_SKILL_KIND::COMBO && running->eCharacterClass == merc.eCharacterClass &&
            (!fixedRotation || (running->strInputSlot != "LMB" &&
             std::find(runtime.ComboSkills.begin(), runtime.ComboSkills.end(), running->iSkillId) != runtime.ComboSkills.end())) &&
            !merc.hasBufferedComboInput && merc.iComboStage > 0u && merc.iComboStage < running->ComboStages.size())
        {
            const auto& stage = running->ComboStages[merc.iComboStage - 1u];
            const float nowMs = merc.fActionElapsedSeconds * 1000.f;
            if (stage.iInputCloseMs > 0u && nowMs >= stage.iInputOpenMs && nowMs <= stage.iInputCloseMs)
            {
                const SERVER_PLAYER* target = nullptr;
                float closest = (std::numeric_limits<float>::max)();
                for (const auto& [id, other] : m_Players)
                    if (other.bColosseumParticipant && other.iColosseumTeam != merc.iColosseumTeam && other.iCurrentHp && other.isCombatReady)
                    {
                        const float distance = std::hypot(other.fPositionX - merc.fPositionX, other.fPositionZ - merc.fPositionZ);
                        if (distance < closest) { closest = distance; target = &other; }
                    }
                if (target)
                {
                    C2S_USE_SKILL continuation;
                    continuation.iClientSequence = ++runtime.iSequence; continuation.iSkillId = running->iSkillId;
                    continuation.eTargetIntent = running->eTargetIntent;
                    continuation.fAimX = target->fPositionX; continuation.fAimZ = target->fPositionZ;
                    // A buffered continuation intentionally returns false. Only the
                    // shared executor owns the stage advance; the rotation stays put.
                    (void)Execute_PlayerSkill(merc, continuation);
                    if (merc.hasBufferedComboInput) runtime.pReason = "Buffered the current skill continuation";
                }
            }
        }
        if (runtime.fThinkElapsed < .2f) continue;
        const float elapsed = runtime.fThinkElapsed;
        runtime.fThinkElapsed = 0.f;
        if (fixedRotation && runtime.fComboElapsed >= runtime.fComboTimeout)
        {
            runtime.iSkillCursor = 0u; runtime.fComboElapsed = runtime.fStepWaitElapsed = 0.f;
            runtime.pReason = "Combo total deadline reached";
            continue;
        }
        if (merc.bPatternBound || merc.fKnockbackRemainingSeconds > 0.f || merc.TriggerMove.isActive ||
            merc.eAction == PLAYER_ACTION_STATE::DEAD || merc.eAction == PLAYER_ACTION_STATE::FALLING)
        { runtime.pReason = "Waiting for crowd control to end"; continue; }
        if (!fixedRotation)
        {
            BuildAvailableMercenarySkills(m_GameplayCatalog.Active(), merc, runtime.ComboSkills, &runtime.iSkillCursor);
            if (merc.eAction == PLAYER_ACTION_STATE::KNOCKDOWN)
            {
                for (const auto id : runtime.ComboSkills)
                    if (const auto* skill = m_GameplayCatalog.Find_Skill(id); skill && skill->eSkillKind == PLAYER_SKILL_KIND::STANDUP)
                    {
                        C2S_USE_SKILL standup;
                        standup.iClientSequence = ++runtime.iSequence; standup.iSkillId = id;
                        standup.eTargetIntent = skill->eTargetIntent;
                        standup.fAimX = merc.fPositionX; standup.fAimZ = merc.fPositionZ;
                        if (Execute_PlayerSkill(merc, standup))
                        { runtime.pReason = "Stand-up skill admitted"; break; }
                    }
                continue;
            }
        }
        if (merc.eAction == PLAYER_ACTION_STATE::SKILL)
            if (const auto* skill = m_GameplayCatalog.Find_Skill(merc.iCurrentSkillId);
                skill && skill->eSkillKind == PLAYER_SKILL_KIND::HOLD && merc.fActionElapsedSeconds >= 1.f)
            {
                C2S_RELEASE_SKILL release; release.iClientSequence = ++runtime.iSequence; release.iSkillId = merc.iCurrentSkillId;
                m_PlayerSkillSystem.Release(merc, release, m_GameplayCatalog);
            }
        const SERVER_PLAYER* enemy = nullptr; float nearest = (std::numeric_limits<float>::max)();
        for (const auto& [id, other] : m_Players)
            if (other.bColosseumParticipant && other.iColosseumTeam != merc.iColosseumTeam && other.iCurrentHp && other.isCombatReady)
            {
                const float distance = std::hypot(other.fPositionX - merc.fPositionX, other.fPositionZ - merc.fPositionZ);
                if (distance < nearest) { nearest = distance; enemy = &other; }
            }
        if (!enemy) continue;
        const auto risk = [this, &merc](float x, float z)
        {
            unsigned result = 0;
            for (const auto& [id, other] : m_Players)
            {
                if (!other.bColosseumParticipant || other.iColosseumTeam == merc.iColosseumTeam || other.iCurrentHp == 0u || other.eAction != PLAYER_ACTION_STATE::SKILL) continue;
                const auto* skill = m_GameplayCatalog.Find_Skill(other.iCurrentSkillId);
                if (!skill) continue;
                const auto* hits = &skill->Hits;
                if (other.iComboStage > 0u && other.iComboStage <= skill->ComboStages.size()) hits = &skill->ComboStages[other.iComboStage - 1u].Hits;
                for (const auto& hit : *hits)
                {
                    const float now = other.fActionElapsedSeconds * 1000.f;
                    const float end = static_cast<float>(hit.iTimeMs + (hit.iRepeatCount ? hit.iRepeatCount - 1u : 0u) * hit.iRepeatMs + hit.iDurationMs);
                    if (end + 100.f >= now && hit.iTimeMs <= now + 400.f && HitOverlaps(hit, other, x, z)) ++result;
                }
            }
            return result;
        };
        const auto currentRisk = risk(merc.fPositionX, merc.fPositionZ);
        if (currentRisk > 0u)
        {
            unsigned bestRisk = currentRisk; SERVER_NAV_POINT best; bool found = false;
            for (unsigned i = 0; i < 12u; ++i)
            {
                const float angle = i * PI / 6.f; SERVER_NAV_POINT point;
                if (!Find_GuideLanding(merc, merc.fPositionX + std::sin(angle) * 3.f, merc.fPositionY,
                    merc.fPositionZ + std::cos(angle) * 3.f, point)) continue;
                const auto candidateRisk = risk(point.x, point.z);
                if (candidateRisk >= bestRisk || !m_ServerNavigation.Has_LineOfSight(merc.fPositionX, merc.fPositionZ, point.x, point.z, merc.fPositionY)) continue;
                bestRisk = candidateRisk; best = point; found = true;
            }
            if (found)
            {
                bool dodged = false;
                const auto* running = merc.eAction == PLAYER_ACTION_STATE::SKILL ?
                    m_GameplayCatalog.Find_Skill(merc.iCurrentSkillId) : nullptr;
                // The common command boundary queues a different skill during
                // COMBO before testing availability. Keep that as navigation
                // avoidance, rather than mistaking a pending dodge for approval.
                if (!fixedRotation && (!running || running->eSkillKind != PLAYER_SKILL_KIND::COMBO))
                    for (const auto id : runtime.ComboSkills)
                        if (const auto* skill = m_GameplayCatalog.Find_Skill(id); skill && Is_DodgeSkill(*skill))
                        {
                            C2S_USE_SKILL dodge;
                            dodge.iClientSequence = ++runtime.iSequence; dodge.iSkillId = id;
                            dodge.eTargetIntent = skill->eTargetIntent; dodge.fAimX = best.x; dodge.fAimZ = best.z;
                            if (!Execute_PlayerSkill(merc, dodge)) continue;
                            runtime.pReason = "Dodge skill admitted toward safer navigation";
                            dodged = true;
                            break;
                        }
                if (dodged) continue;
                C2S_MOVE move; move.iClientSequence = ++runtime.iSequence; move.fGoalX = best.x; move.fGoalZ = best.z;
                Execute_PlayerMove(merc, move);
                runtime.pReason = "Evading a pending enemy hit";
                continue;
            }
        }
        if (merc.eAction != PLAYER_ACTION_STATE::NONE)
        { runtime.pReason = "Waiting for the admitted skill to finish"; continue; }
        if (runtime.ComboSkills.empty())
        { runtime.pReason = "No admitted skills"; continue; }
        if (!fixedRotation)
        {
            bool started = false;
            for (std::size_t attempt = 0u; attempt < runtime.ComboSkills.size(); ++attempt)
            {
                const auto index = (runtime.iSkillCursor + attempt) % runtime.ComboSkills.size();
                const auto* skill = m_GameplayCatalog.Find_Skill(runtime.ComboSkills[index]);
                if (!skill || skill->eSkillKind == PLAYER_SKILL_KIND::STANDUP || Is_DodgeSkill(*skill) ||
                    nearest > (std::max)(2.f, skill->fMaximumRange)) continue;
                C2S_USE_SKILL command;
                command.iClientSequence = ++runtime.iSequence; command.iSkillId = skill->iSkillId;
                command.eTargetIntent = skill->eTargetIntent; command.fAimX = enemy->fPositionX; command.fAimZ = enemy->fPositionZ;
                if (!Execute_PlayerSkill(merc, command)) continue;
                runtime.iSkillCursor = (index + 1u) % runtime.ComboSkills.size();
                runtime.pReason = "Available class skill admitted";
                started = true;
                break;
            }
            if (!started && nearest > 1.5f)
            {
                C2S_MOVE move; move.iClientSequence = ++runtime.iSequence;
                move.fGoalX = enemy->fPositionX + (merc.fPositionX - enemy->fPositionX) * 1.5f / nearest;
                move.fGoalZ = enemy->fPositionZ + (merc.fPositionZ - enemy->fPositionZ) * 1.5f / nearest;
                Execute_PlayerMove(merc, move);
                runtime.pReason = "Approaching available class skill range";
            }
            else if (!started) runtime.pReason = "No class skill currently available";
            continue;
        }
        if (runtime.iSkillCursor >= runtime.ComboSkills.size())
        {
            runtime.iSkillCursor = 0u; runtime.fComboElapsed = runtime.fStepWaitElapsed = 0.f;
        }
        const auto* skill = m_GameplayCatalog.Find_Skill(runtime.ComboSkills[runtime.iSkillCursor]);
        if (!skill || skill->strInputSlot == "LMB" || skill->eCharacterClass != merc.eCharacterClass)
        { runtime.pReason = "Published combo binding is unavailable"; continue; }
        const float range = (std::max)(2.f, skill->fMaximumRange);
        if (nearest > range)
        {
            C2S_MOVE move; move.iClientSequence = ++runtime.iSequence;
            const float stop = (std::max)(1.f, range * .75f);
            move.fGoalX = enemy->fPositionX + (merc.fPositionX - enemy->fPositionX) * stop / nearest;
            move.fGoalZ = enemy->fPositionZ + (merc.fPositionZ - enemy->fPositionZ) * stop / nearest;
            Execute_PlayerMove(merc, move);
            runtime.pReason = "Approaching the next combo skill range";
            continue;
        }
        // Exactly one ordered skill is attempted. A rejected cooldown/resource
        // check never falls through to another slot or to a basic attack.
        C2S_USE_SKILL command;
        command.iClientSequence = ++runtime.iSequence; command.iSkillId = skill->iSkillId;
        command.eTargetIntent = skill->eTargetIntent; command.fAimX = enemy->fPositionX; command.fAimZ = enemy->fPositionZ;
        if (Execute_PlayerSkill(merc, command))
        {
            ++runtime.iSkillCursor; runtime.fStepWaitElapsed = 0.f;
            runtime.pReason = "Ordered skill admitted";
        }
        else
        {
            runtime.fStepWaitElapsed += elapsed;
            const auto cooldown = merc.CooldownEndTickBySkillId.find(skill->iSkillId);
            runtime.pReason = cooldown != merc.CooldownEndTickBySkillId.end() &&
                static_cast<std::int32_t>(cooldown->second - m_iServerTick) > 0 ?
                "Waiting for the ordered skill cooldown" : "Waiting for ordered skill resources or status";
            if (runtime.fStepWaitElapsed >= runtime.fStepWaitTimeout)
            {
                runtime.iSkillCursor = 0u; runtime.fComboElapsed = runtime.fStepWaitElapsed = 0.f;
                runtime.pReason = "Unavailable skill exceeded its wait deadline";
            }
        }
    }
}

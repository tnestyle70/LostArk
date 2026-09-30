#include "GameRoom.h"
#include "ClientSession.h"
#include "Network/PacketWriter.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <regex>
#include <sstream>

using namespace LostArk::Server;
using namespace LostArk::Shared;
namespace
{
    constexpr float PI = 3.14159265359f;
    bool reached(std::uint32_t now, std::uint32_t end) { return !end || static_cast<std::int32_t>(now - end) >= 0; }
    std::uint32_t nextTick(std::uint32_t now, std::uint32_t delta) { const auto value = now + delta; return value ? value : 1u; }
    float roll(std::uint32_t seed) { seed ^= seed >> 16; seed *= 0x7feb352du; seed ^= seed >> 15; seed *= 0x846ca68bu; seed ^= seed >> 16; return (seed & 0xffffu) / 65535.f; }
    std::filesystem::path tuningPath()
    {
        wchar_t executable[32768]{};
        const auto count = GetModuleFileNameW(nullptr, executable, 32768);
        for (auto base : { std::filesystem::current_path(), count && count < 32768 ? std::filesystem::path(executable).parent_path() : std::filesystem::path{} })
            for (unsigned depth = 0; !base.empty() && depth < 8; ++depth, base = base.parent_path())
                if (std::filesystem::is_regular_file(base / L"AGENTS.md") && std::filesystem::is_directory(base / L"Data"))
                    return base / L"Data/AI/MaharakaWaterpangAI.json";
        return {};
    }
    std::string readFile(const std::filesystem::path& path)
    {
        std::ifstream stream(path, std::ios::binary);
        if (!stream) throw std::runtime_error("AI source JSON is unavailable");
        stream.seekg(0, std::ios::end);
        if (stream.tellg() > 16384) throw std::runtime_error("AI source JSON exceeds 16 KiB");
        stream.seekg(0); return { std::istreambuf_iterator<char>(stream), {} };
    }
    MAHARAKA_AI_TUNING parseTuning(const std::string& text)
    {
        // This contract is a flat JSON object of ASCII schema keys and finite numbers.
        // Match the whole input; duplicate/unknown keys and trailing data are rejected.
        const std::regex row(R"json(\s*"([A-Za-z][A-Za-z0-9]*)"\s*:\s*("[A-Za-z0-9._-]+"|-?(?:0|[1-9][0-9]*)(?:\.[0-9]+)?(?:[eE][+-]?[0-9]+)?)\s*)json");
        std::map<std::string, std::string> values;
        std::size_t at = text.find_first_not_of(" \r\n\t");
        if (at == std::string::npos || text[at++] != '{') throw std::runtime_error("AI JSON object expected");
        for (;;)
        {
            std::smatch match; const auto remainder = text.substr(at);
            if (!std::regex_search(remainder, match, row, std::regex_constants::match_continuous) ||
                !values.emplace(match[1].str(), match[2].str()).second) throw std::runtime_error("Invalid or duplicate AI JSON field");
            at += match.length();
            if (at == text.size()) throw std::runtime_error("Unclosed AI JSON object");
            const char separator = text[at++];
            if (separator == '}') break;
            if (separator != ',') throw std::runtime_error("Invalid AI JSON separator");
        }
        if (text.find_first_not_of(" \r\n\t", at) != std::string::npos || values.size() != 12u ||
            values.at("schema") != "\"lostark.maharaka-waterpang-ai\"" || values.at("formatVersion") != "1")
            throw std::runtime_error("Unsupported AI JSON schema");
        auto number = [&](const char* key) { std::size_t used = 0; const auto& s = values.at(key); const double v = std::stod(s, &used); if (used != s.size() || !std::isfinite(v)) throw std::runtime_error("Invalid AI number"); return v; };
        auto integer = [&](const char* key) { const double v = number(key); if (v < 0 || v > UINT32_MAX || v != std::floor(v)) throw std::runtime_error("Invalid AI integer"); return static_cast<std::uint32_t>(v); };
        MAHARAKA_AI_TUNING t;
        t.iRevision = integer("revision"); t.iBotCount = integer("botCount"); t.iDecisionTicks = integer("decisionTicks");
        t.iMoveRetargetTicks = integer("moveRetargetTicks"); t.iSkillIntervalTicks = integer("skillIntervalTicks");
        t.fTargetRangeM = static_cast<float>(number("targetRangeM")); t.fMoveProbability = static_cast<float>(number("moveProbability"));
        t.fAggression = static_cast<float>(number("aggression")); t.fKnockbackRangeM = static_cast<float>(number("knockbackRangeM"));
        t.iKnockbackMs = integer("knockbackMs");
        if (!Is_Valid_MaharakaAITuning(t)) throw std::runtime_error("AI tuning value outside bounds");
        return t;
    }
    std::string serializeTuning(const MAHARAKA_AI_TUNING& t)
    {
        std::ostringstream s; s << std::setprecision(9) << "{\n  \"schema\": \"lostark.maharaka-waterpang-ai\",\n  \"formatVersion\": 1,\n  \"revision\": " << t.iRevision
            << ",\n  \"botCount\": " << t.iBotCount << ",\n  \"decisionTicks\": " << t.iDecisionTicks << ",\n  \"moveRetargetTicks\": " << t.iMoveRetargetTicks
            << ",\n  \"skillIntervalTicks\": " << t.iSkillIntervalTicks << ",\n  \"targetRangeM\": " << t.fTargetRangeM
            << ",\n  \"moveProbability\": " << t.fMoveProbability << ",\n  \"aggression\": " << t.fAggression
            << ",\n  \"knockbackRangeM\": " << t.fKnockbackRangeM << ",\n  \"knockbackMs\": " << t.iKnockbackMs << "\n}\n";
        return s.str();
    }
    void loadTuning(bool& loaded, MAHARAKA_AI_TUNING& tuning, std::string& original)
    {
        if (loaded) return;
        loaded = true;
        try { auto bytes = readFile(tuningPath()); const auto candidate = parseTuning(bytes); tuning = candidate; original = std::move(bytes); }
        catch (const std::exception& e) { std::cout << "[WaterpangAI] keeping defaults: " << e.what() << '\n'; }
    }
}

void CGameRoom::Handle_MaharakaAITuning(SESSION_ID sessionId, const C2S_MAHARAKA_AI_TUNING& request)
{
    const auto session = Find_Session(sessionId);
    if (!session || !m_PlayerIdBySessionId.contains(sessionId)) return;
    loadTuning(m_bMaharakaAITuningLoaded, m_MaharakaAITuning, m_strMaharakaAISourceBytes);
    S2C_MAHARAKA_AI_TUNING result; result.iRequestSequence = request.iRequestSequence;
    result.strStatus = "Current Server AI tuning";
    if (m_eWorldId != WORLD_ID::MAHARAKA) { result.eResult = MAHARAKA_AI_RESULT::WRONG_WORLD; result.strStatus = "Enter Maharaka first"; }
    else if (request.eOperation != MAHARAKA_AI_OPERATION::GET)
    {
        auto staged = request.Tuning;
        if (request.iExpectedRevision != m_MaharakaAITuning.iRevision) { result.eResult = MAHARAKA_AI_RESULT::REVISION_CONFLICT; result.strStatus = "Revision changed; Get current values before applying"; }
        else if (!Is_Valid_MaharakaAITuning(staged) || m_MaharakaAITuning.iRevision == UINT32_MAX) { result.eResult = MAHARAKA_AI_RESULT::INVALID_VALUE; result.strStatus = "AI tuning is outside supported bounds"; }
        else
        {
            staged.iRevision = m_MaharakaAITuning.iRevision + 1u;
            try
            {
                if (request.eOperation == MAHARAKA_AI_OPERATION::SAVE)
                {
                    const auto path = tuningPath();
                    if (path.empty() || m_strMaharakaAISourceBytes.empty() || readFile(path) != m_strMaharakaAISourceBytes)
                        throw std::runtime_error("Source JSON changed or is unavailable; current Server values preserved");
                    const auto payload = serializeTuning(staged);
                    if (!Is_Valid_MaharakaAITuning(parseTuning(payload))) throw std::runtime_error("AI save validation failed");
                    auto temporary = path; temporary += L".tmp." + std::to_wstring(GetCurrentProcessId());
                    { std::ofstream output(temporary, std::ios::binary | std::ios::trunc); output.write(payload.data(), payload.size()); output.flush(); if (!output) throw std::runtime_error("AI temporary write failed"); }
                    if (readFile(path) != m_strMaharakaAISourceBytes) throw std::runtime_error("Concurrent AI source save; current values preserved");
                    auto backup = path; backup += L".backup";
                    if (!ReplaceFileW(path.c_str(), temporary.c_str(), backup.c_str(), REPLACEFILE_WRITE_THROUGH, nullptr, nullptr)) throw std::runtime_error("Atomic AI source replacement failed");
                    m_strMaharakaAISourceBytes = payload;
                }
                m_MaharakaAITuning = staged;
                for (auto& [id, state] : m_MaharakaWaterpangAI) { (void)id; state.iNextThinkTick = state.iNextMoveTick = state.iNextShotTick = 0; }
                result.strStatus = request.eOperation == MAHARAKA_AI_OPERATION::SAVE ? "Saved source and applied on Server" : "Applied on Server; source file unchanged";
            }
            catch (const std::exception& e) { result.eResult = MAHARAKA_AI_RESULT::SAVE_FAILED; result.strStatus = e.what(); }
        }
    }
    result.Tuning = m_MaharakaAITuning;
    CPacketWriter writer;
    if (!Write_Message(writer, result) || !session->Send_Frame(PACKET_TYPE::S2C_MAHARAKA_AI_TUNING, writer.Get_Buffer())) session->Request_Close();
}

bool CGameRoom::Spawn_MaharakaWaterpangAI()
{
    const auto firstSlot = static_cast<std::uint32_t>(m_MaharakaWaterpangAI.size());
    const auto count = m_MaharakaAITuning.iBotCount - firstSlot;
    if (m_Players.size() + count > MAX_WORLD_SNAPSHOT_PLAYERS || !m_ServerNavigation.Is_Loaded()) return false;
    std::vector<SERVER_PLAYER> staged;
    for (std::uint32_t slot = firstSlot; slot < m_MaharakaAITuning.iBotCount; ++slot)
    {
        SERVER_PLAYER ai; ai.eControlKind = PLAYER_CONTROL_KIND::WATERPANG_AI;
        ai.iPlayerId = m_iNextWaterpangAIPlayerId + slot - firstSlot; ai.iNetEntityId = m_iNextNetEntityId + slot - firstSlot;
        if (!ai.iPlayerId || !ai.iNetEntityId || m_Players.contains(ai.iPlayerId)) return false;
        ai.eCharacterClass = slot % 2u ? CHARACTER_CLASS_ID::LANCE_MASTER : CHARACTER_CLASS_ID::GUARDIANKNIGHT;
        const auto* profile = m_GameplayCatalog.Find_Player(ai.eCharacterClass);
        if (!profile) return false;
        ai.strNickName = "Waterpang AI " + std::to_string(slot + 1u);
        ai.strSpawnPlacementId = "waterpang.ai." + std::to_string(slot);
        ai.iCurrentHp = ai.iMaximumHp = profile->iMaximumHp;
        ai.iCurrentResource = ai.iMaximumResource = profile->iMaximumResource;
        ai.iMaximumIdentity = profile->iMaximumIdentity; ai.eStance = profile->eDefaultStance;
        ai.fMoveSpeed = profile->fMoveSpeed; ai.isCombatReady = true;
        CPlayerSkillSystem::Reset_Gauges(ai, m_GameplayCatalog);
        if (slot < 12u)
        {
            const std::string prefix = slot % 2u ? "AVATAR_LANCEMASTER_MOKOKO_036" : "AVATAR_GUARDIANKNIGHT_MOKOKO_036";
            const auto variant = slot / 2u; const auto set = prefix + (variant ? "-" + std::to_string(variant) : "");
            for (const auto& [suffix, equip] : { std::pair{"_HEAD", EQUIPMENT_SLOT::AVATAR_HEAD}, std::pair{"_OUTFIT", EQUIPMENT_SLOT::AVATAR_OUTFIT} })
            {
                const auto item = set + suffix;
                if (!m_ItemCatalog.Find_Item(item)) return false;
                ai.Inventory.push_back({item, 1u, equip});
            }
        }
        else ai.strWaterpangNpcArchetypeId = MAHARAKA_WATERPANG_AI_NPCS[slot - 12u];
        const float angle = (slot % 10u) * PI / 5.f + (slot / 10u) * PI / 10.f;
        const float radius = slot < 10u ? 3.4f : 6.f;
        SERVER_NAV_POINT point;
        if (!Find_GuideLanding(ai, MAHARAKA_WATERPANG_CANNON_X + std::sin(angle) * radius, 22.4f,
            MAHARAKA_WATERPANG_CANNON_Z + std::cos(angle) * radius, point) || point.y < MAHARAKA_WATERPANG_DECK_MIN_Y_M ||
            std::hypot(point.x - MAHARAKA_WATERPANG_CANNON_X, point.z - MAHARAKA_WATERPANG_CANNON_Z) > 7.f) return false;
        if (std::any_of(staged.begin(), staged.end(), [&](const auto& other) { return std::hypot(point.x - other.fPositionX, point.z - other.fPositionZ) < .75f; })) return false;
        ai.fPositionX = point.x; ai.fPositionY = point.y; ai.fPositionZ = point.z;
        ai.fYawDegrees = angle * 180.f / PI + 180.f;
        staged.push_back(std::move(ai));
    }
    for (auto& ai : staged)
    {
        MAHARAKA_WATERPANG_AI state; state.iSlot = static_cast<std::uint32_t>(m_MaharakaWaterpangAI.size()); state.iSkillSlot = state.iSlot % 4u;
        const auto id = ai.iPlayerId;
        m_PlayerIdByEntityId.emplace(ai.iNetEntityId, id); m_MaharakaWaterpangAI.emplace(id, state);
        m_Players.emplace(id, std::move(ai)); Broadcast_Spawned(m_Players.at(id), INVALID_SESSION_ID);
    }
    m_iNextNetEntityId += count; m_iNextWaterpangAIPlayerId += count;
    std::cout << "[WaterpangAI] spawned " << count << " contestants\n";
    return true;
}

void CGameRoom::Clear_MaharakaWaterpangAI(const std::uint32_t keepCount)
{
    for (auto it = m_MaharakaWaterpangAI.begin(); it != m_MaharakaWaterpangAI.end();)
    {
        if (it->second.iSlot < keepCount) { ++it; continue; }
        const auto id = it->first; it = m_MaharakaWaterpangAI.erase(it);
        auto actor = m_Players.find(id); if (actor == m_Players.end()) continue;
        m_CombatObjectRuntime.Cancel_Source(actor->second.iNetEntityId); m_ServerTriggerSystem.Remove_Player(id);
        Broadcast_Despawned(actor->second.iNetEntityId, PLAYER_DESPAWN_REASON::LEVEL_CHANGED);
        m_PlayerIdByEntityId.erase(actor->second.iNetEntityId); m_Players.erase(actor);
    }
}

bool CGameRoom::Finish_MaharakaWaterpangMatch()
{
    if (!m_MaharakaWaterpangIntro) return false;
    struct RETURN_DESTINATION { PLAYER_ID id; SERVER_NAV_POINT point; float yaw; };
    std::vector<RETURN_DESTINATION> destinations;
    for (const auto& [id, player] : m_Players)
    {
        if (!player.Is_Human() || !player.bWaterpangParticipant) continue;
        const auto* spawn = Find_Placement(player.strSpawnPlacementId);
        SERVER_NAV_POINT point;
        // Stage every participant's own admission spawn before sending STOP or mutating the room.
        if (!spawn || !spawn->isEnabled || !player.iMaximumHp ||
            !Find_GuideLanding(player, spawn->fPositionX, spawn->fPositionY, spawn->fPositionZ, point) ||
            Is_MaharakaWaterpangArenaFootprint(point.x, point.z) ||
            std::any_of(destinations.begin(), destinations.end(), [&](const auto& other)
            { return std::abs(point.y - other.point.y) < 1.5f && std::hypot(point.x - other.point.x, point.z - other.point.z) < .75f; }))
        {
            m_strStatus = "Waterpang return pending: admission spawn landing unavailable for " + player.strSpawnPlacementId;
            return false;
        }
        destinations.push_back({id, point, spawn->fYawDegrees});
    }
    S2C_WORLD_SEQUENCE_PLAY stopped = *m_MaharakaWaterpangIntro;
    stopped.eOperation = WORLD_SEQUENCE_OPERATION::STOP; stopped.iServerTick = m_iServerTick;
    CPacketWriter writer; if (!Write_Message(writer, stopped)) return false;
    for (const auto& [id, player] : m_Players)
    {
        (void)id; if (auto session = Find_Session(player.iSessionId); session && !session->Send_Frame(PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY, writer.Get_Buffer())) session->Request_Close();
    }
    m_MaharakaWaterGunShots.clear(); Clear_MaharakaWaterpangAI();
    for (const auto& destination : destinations)
    {
        auto& player = m_Players.at(destination.id);
        Reset_PlayerForDebugTeleport(player);
        player.bWaterpangParticipant = player.bWaterpangFall = player.bWaterpangLaunch = false;
        // The replicated latest-cast pair must become zero together, including after its clip ended.
        player.iWaterGunSkillId = player.iWaterGunCastTick = player.iWaterGunCastEndTick = player.iWaterGunSpeedEndTick = 0u;
        player.fWaterGunSpeedScale = 1.f; player.iWaterpangCannonHitTick = 0u;
        for (const auto& skill : MAHARAKA_WATERGUN_SKILLS) player.CooldownEndTickBySkillId.erase(skill.iSkillId);
        player.bKnockbackBallistic = player.bKnockbackCanLeaveArena = player.bArenaEjectionActive = false;
        player.iEjectionOwnerNetEntityId = INVALID_NET_ENTITY_ID;
        player.bPushOnlyHitReaction = false;
        player.fKnockbackVelocityY = player.fKnockbackLaunchY = player.fKnockbackSupportY = 0.f;
        player.fKnockbackGravityMps2 = SERVER_PLAYER::KNOCKBACK_GRAVITY_MPS2;
        player.iCurrentHp = player.iMaximumHp;
        player.fPositionX = destination.point.x; player.fPositionY = destination.point.y; player.fPositionZ = destination.point.z;
        player.fYawDegrees = destination.yaw;
    }
    m_MaharakaWaterpangIntro.reset(); m_MaharakaWaterpangDebugEvent.reset(); m_iWaterpangAIRetryTick = 0;
    m_ServerTriggerSystem.Reset_SequenceActivation(MAHARAKA_WATERPANG_INTRO_INSTANCE);
    std::cout << "[WaterpangAI] three-minute match finished; returned " << destinations.size() << " humans to island\n";
    return true;
}

void CGameRoom::Update_MaharakaWaterpangMatch(const std::uint32_t updateTick)
{
    if (m_eWorldId != WORLD_ID::MAHARAKA) return;
    loadTuning(m_bMaharakaAITuningLoaded, m_MaharakaAITuning, m_strMaharakaAISourceBytes);
    if (!m_MaharakaWaterpangIntro) return;
    for (auto& [id, player] : m_Players)
    {
        (void)id;
        if (player.Is_Human() && (Is_MaharakaWaterpangArenaFootprint(player.fPositionX, player.fPositionZ) ||
            player.bWaterpangFall || player.bWaterpangLaunch ||
            (player.TriggerMove.isActive && Is_MaharakaWaterpangArenaFootprint(player.TriggerMove.fTargetX, player.TriggerMove.fTargetZ))))
            player.bWaterpangParticipant = true;
    }
    const auto elapsed = static_cast<std::int32_t>(updateTick - m_MaharakaWaterpangIntro->iStartTick);
    if (elapsed >= static_cast<std::int32_t>(MAHARAKA_WATERPANG_MATCH_END_TICKS)) { (void)Finish_MaharakaWaterpangMatch(); return; }
    if (m_MaharakaWaterpangAI.size() != m_MaharakaAITuning.iBotCount && reached(updateTick, m_iWaterpangAIRetryTick))
    {
        // Admit additions atomically and preserve existing contestants if navigation fails.
        Clear_MaharakaWaterpangAI(m_MaharakaAITuning.iBotCount);
        if (m_MaharakaWaterpangAI.size() < m_MaharakaAITuning.iBotCount && !Spawn_MaharakaWaterpangAI()) std::cout << "[WaterpangAI] spawn admission pending: navigation, profile, avatar or room capacity\n";
        m_iWaterpangAIRetryTick = nextTick(updateTick, 30u);
    }
    if (elapsed < static_cast<std::int32_t>(MAHARAKA_WATERPANG_FIRST_EVENT_SECONDS * MAHARAKA_WATERPANG_TICK_HZ)) return;
    const auto& tuning = m_MaharakaAITuning;
    for (auto& [id, state] : m_MaharakaWaterpangAI)
    {
        auto& ai = m_Players.at(id);
        if (!reached(updateTick, state.iNextThinkTick)) continue;
        state.iNextThinkTick = nextTick(updateTick, tuning.iDecisionTicks);
        if (!ai.iCurrentHp || ai.eAction != PLAYER_ACTION_STATE::NONE || ai.TriggerMove.isActive || ai.fKnockbackRemainingSeconds > 0.f || ai.bWaterpangFall) continue;
        const float radius = std::hypot(ai.fPositionX - MAHARAKA_WATERPANG_CANNON_X, ai.fPositionZ - MAHARAKA_WATERPANG_CANNON_Z);
        if (radius > 7.25f || ai.fPositionY < MAHARAKA_WATERPANG_DECK_MIN_Y_M)
        {
            SERVER_NAV_POINT landing; const float angle = state.iSlot * PI * .618f;
            if (Find_GuideLanding(ai, MAHARAKA_WATERPANG_CANNON_X + std::sin(angle) * 5.5f, 22.4f, MAHARAKA_WATERPANG_CANNON_Z + std::cos(angle) * 5.5f, landing) && landing.y >= MAHARAKA_WATERPANG_DECK_MIN_Y_M)
            {
                ai.TriggerMove = {}; ai.TriggerMove.isActive = true; ai.TriggerMove.strSourcePlacementId = "waterpang.ai.rejoin";
                ai.TriggerMove.fStartX = ai.fPositionX; ai.TriggerMove.fStartY = ai.fPositionY; ai.TriggerMove.fStartZ = ai.fPositionZ;
                ai.TriggerMove.fTargetX = landing.x; ai.TriggerMove.fTargetY = landing.y; ai.TriggerMove.fTargetZ = landing.z;
                ai.TriggerMove.fDurationSeconds = 1.2f; ai.TriggerMove.fArcHeight = 2.5f; ai.eAction = PLAYER_ACTION_STATE::TRIGGER_MOVE;
                ai.iActionStartTick = updateTick; ai.hasMoveGoal = false; ai.MovePath.clear();
            }
            continue;
        }
        const SERVER_PLAYER* target = nullptr; float distance = tuning.fTargetRangeM;
        for (const auto& [otherId, other] : m_Players)
        {
            if (id == otherId || !other.iCurrentHp || other.Is_Guide() || other.eAction == PLAYER_ACTION_STATE::FALLING ||
                !Is_MaharakaWaterpangArenaFootprint(other.fPositionX, other.fPositionZ) || std::abs(ai.fPositionY - other.fPositionY) > 1.5f) continue;
            const float d = std::hypot(ai.fPositionX - other.fPositionX, ai.fPositionZ - other.fPositionZ);
            if (d < distance) { distance = d; target = &other; }
        }
        if (reached(updateTick, state.iNextMoveTick))
        {
            state.iNextMoveTick = nextTick(updateTick, tuning.iMoveRetargetTicks);
            if (roll(updateTick ^ id) < tuning.fMoveProbability)
            {
                const float angle = roll(updateTick + id * 37u) * 2.f * PI;
                const float range = 2.8f + roll(updateTick ^ (id * 77u)) * 3.6f;
                C2S_MOVE command; command.iClientSequence = ++state.iSequence;
                command.fGoalX = MAHARAKA_WATERPANG_CANNON_X + std::sin(angle) * range;
                command.fGoalZ = MAHARAKA_WATERPANG_CANNON_Z + std::cos(angle) * range;
                Execute_PlayerMove(ai, command);
            }
        }
        if (target && reached(updateTick, state.iNextShotTick))
        {
            state.iNextShotTick = nextTick(updateTick, tuning.iSkillIntervalTicks + state.iSlot % 7u);
            if (roll(updateTick ^ (id * 313u)) < tuning.fAggression)
                for (unsigned attempt = 0; attempt < 4; ++attempt)
                {
                    const auto& skill = MAHARAKA_WATERGUN_SKILLS[state.iSkillSlot++ % MAHARAKA_WATERGUN_SKILLS.size()];
                    C2S_USE_SKILL command; command.iClientSequence = ++state.iSequence; command.iSkillId = skill.iSkillId;
                    command.fAimX = target->fPositionX; command.fAimZ = target->fPositionZ;
                    if (Try_StartMaharakaWaterGunSkill(ai, command)) break;
                }
        }
    }
}

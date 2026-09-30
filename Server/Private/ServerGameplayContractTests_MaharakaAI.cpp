#include "ServerGameplayContractTests_Runner.h"
#include "GameRoom.h"
#include "ClientSession.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>

using namespace LostArk::Server;
using namespace LostArk::Shared;

int CServerGameplayContractRunner::Run_MaharakaAI()
{
    TESTS tests;
    Run_WorldPlayback(tests);
    auto room = std::make_unique<CGameRoom>(WORLD_ID::MAHARAKA);
    tests.Require(room->Is_Ready(), "Waterpang AI fixture loads the published Maharaka room");
    if (!room->Is_Ready()) return 1;
    std::vector<std::shared_ptr<CClientSession>> sessions;
    const auto drain = [&]() { for (auto& session : sessions) { session->m_OutboundFrames.clear(); session->m_iQueuedOutboundBytes = 0; session->m_OutboundMetrics.iCurrentQueuedByteCount = 0; session->m_OutboundMetrics.iCurrentQueuedFrameCount = 0; } };
    for (unsigned i = 0; i < 4; ++i)
    {
        auto session = std::make_shared<CClientSession>(99501u + i, INVALID_SOCKET, CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
        session->m_isSendRunning.store(true); room->Handle_Register(session);
        C2S_ENTER_WORLD enter; enter.eWorldId = WORLD_ID::MAHARAKA; enter.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER; enter.strNickName = "Waterpang contract";
        tests.Require(room->Join(session->Get_SessionId(), enter), "Human admission succeeds beside optional Waterpang contestants");
        sessions.push_back(session); drain();
    }
    if (room->Count_HumanPlayers() != 4) return 1;
    C2S_MAHARAKA_AI_TUNING request; request.iRequestSequence = 1;
    CPacketWriter requestWriter; C2S_MAHARAKA_AI_TUNING decoded;
    const bool written = Write_Message(requestWriter, request); CPacketReader requestReader(requestWriter.Get_Buffer());
    tests.Require(written && Read_Message(requestReader, decoded) && !requestReader.Get_RemainingSize(), "AI typed GET request round trips");
    room->Handle_MaharakaAITuning(sessions[0]->Get_SessionId(), request); drain();
    const auto revision = room->m_MaharakaAITuning.iRevision;
    request.eOperation = MAHARAKA_AI_OPERATION::APPLY; request.iExpectedRevision = revision;
    request.Tuning = room->m_MaharakaAITuning; request.Tuning.fMoveProbability = 1.f; request.Tuning.fAggression = 1.f;
    room->Handle_MaharakaAITuning(sessions[0]->Get_SessionId(), request); drain();
    tests.Require(room->m_MaharakaAITuning.iRevision == revision + 1 && room->m_MaharakaAITuning.fAggression == 1.f, "AI APPLY commits the new revision and live decision values");
    request.Tuning.iBotCount = 0;
    room->Handle_MaharakaAITuning(sessions[0]->Get_SessionId(), request); drain();
    tests.Require(room->m_MaharakaAITuning.iBotCount == 20 && room->m_MaharakaAITuning.iRevision == revision + 1, "Stale tuning CAS preserves the live roster and revision");
    request.iExpectedRevision = revision + 1; request.Tuning.fAggression = std::numeric_limits<float>::quiet_NaN();
    room->Handle_MaharakaAITuning(sessions[0]->Get_SessionId(), request); drain();
    tests.Require(room->m_MaharakaAITuning.iRevision == revision + 1, "Nonfinite AI tuning is rejected before mutation");
    // Exercise the real SAVE path in an isolated authoring root, without touching team data.
    const auto originalCwd = std::filesystem::current_path();
    const auto isolated = originalCwd / "out/WaterpangEffects20260930/ai-save-fixture";
    std::filesystem::create_directories(isolated / "Data/AI");
    { std::ofstream marker(isolated / "AGENTS.md"); marker << "isolated test root\n"; }
    const auto source = isolated / "Data/AI/MaharakaWaterpangAI.json";
    { std::ofstream stream(source, std::ios::binary); stream << room->m_strMaharakaAISourceBytes; }
    std::filesystem::current_path(isolated);
    request.eOperation = MAHARAKA_AI_OPERATION::SAVE; request.Tuning = room->m_MaharakaAITuning;
    room->Handle_MaharakaAITuning(sessions[0]->Get_SessionId(), request); drain();
    tests.Require(room->m_MaharakaAITuning.iRevision == revision + 2 && std::filesystem::is_regular_file(source.string() + ".backup"), "AI SAVE validates and atomically replaces source with a recoverable backup");
    { std::ofstream stream(source, std::ios::app); stream << " \n"; }
    request.iExpectedRevision = revision + 2; request.Tuning.iBotCount = 0;
    room->Handle_MaharakaAITuning(sessions[0]->Get_SessionId(), request); drain();
    tests.Require(room->m_MaharakaAITuning.iRevision == revision + 2 && room->m_MaharakaAITuning.iBotCount == 20, "External source edit rejects SAVE and preserves active tuning");
    std::filesystem::current_path(originalCwd);

    S2C_WORLD_SEQUENCE_PLAY intro; intro.eOperation = WORLD_SEQUENCE_OPERATION::PLAY;
    intro.strSequenceInstanceId = MAHARAKA_WATERPANG_INTRO_INSTANCE; intro.iStartTick = 100;
    room->m_MaharakaWaterpangIntro = intro; room->m_iServerTick = 100;
    room->Update_MaharakaWaterpangMatch(100); drain();
    tests.Require(room->m_MaharakaWaterpangAI.size() == 20 && room->m_Players.size() == 24 && room->Count_HumanPlayers() == 4, "Twenty AI fit the real navigation and retain all four human slots");
    if (room->m_MaharakaWaterpangAI.size() != 20) return 1;
    unsigned avatars = 0, npcs = 0;
    for (const auto& [id, state] : room->m_MaharakaWaterpangAI)
    {
        const auto& player = room->m_Players.at(id);
        tests.Require(player.Is_WaterpangAI() && !player.Is_Human() && player.iSessionId == INVALID_SESSION_ID && player.fPositionY >= MAHARAKA_WATERPANG_DECK_MIN_Y_M, "AI uses native deck support without a fake session");
        if (player.strWaterpangNpcArchetypeId.empty()) avatars += player.Inventory.size() == 2;
        else ++npcs;
    }
    tests.Require(avatars == 12 && npcs == 8, "Roster carries twelve real avatar loadouts and eight immutable NPC identities");
    room->m_iServerTick = 100 + 20 * 30;
    room->Update_MaharakaWaterpangMatch(room->m_iServerTick); drain();
    bool moved = false, usedSkill = false;
    for (const auto& [id, state] : room->m_MaharakaWaterpangAI)
    {
        const auto& player = room->m_Players.at(id);
        moved = moved || player.iLastMoveSequence != 0;
        usedSkill = usedSkill || player.iWaterGunSkillId != 0;
    }
    tests.Require(moved && usedSkill, "Actual AI decision loop submits admitted movement and Waterpang skills");
    for (unsigned i = 0; i < 15; ++i) { room->Tick(1.f / 30.f); drain(); }
    tests.Require(room->m_MaharakaWaterpangAI.size() == 20 && room->Count_HumanPlayers() == 4, "Actual room ticks preserve AI ownership and human roster");
    auto& human = room->m_Players.at(sessions[0]->Get_PlayerId());
    human.fPositionX = MAHARAKA_WATERPANG_CANNON_X + 4.f; human.fPositionY = 22.4f; human.fPositionZ = MAHARAKA_WATERPANG_CANNON_Z;
    room->m_iServerTick = intro.iStartTick + MAHARAKA_WATERPANG_MATCH_END_TICKS;
    room->Update_MaharakaWaterpangMatch(room->m_iServerTick);
    tests.Require(!room->m_MaharakaWaterpangIntro && room->m_MaharakaWaterpangAI.empty() && room->Count_HumanPlayers() == 4, "Three-minute expiry clears contestants and keeps four humans connected");
    tests.Require(!Is_MaharakaWaterpangArenaFootprint(human.fPositionX, human.fPositionZ) && human.eAction == PLAYER_ACTION_STATE::NONE, "Match completion moves arena humans onto real island exploration navigation");
    bool stopped = false; for (const auto& frame : sessions[0]->m_OutboundFrames) stopped = stopped || frame.ePacketType == PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY;
    tests.Require(stopped, "Match completion emits reliable sequence stop to clients"); drain();
    for (const auto& session : sessions) room->Leave(session->Get_SessionId(), PLAYER_DESPAWN_REASON::LEVEL_CHANGED);
    return tests.failures ? 1 : 0;
}

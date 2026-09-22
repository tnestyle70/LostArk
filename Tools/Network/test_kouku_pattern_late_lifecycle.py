"""Exercise actual bounded receive FIFO, wire codecs and Pattern Audition without UI/socket.

Only the send transport, connected socket state and service clock are substituted.
The real service translation unit and existing receive regression extraction run together.
Requires an x64 MSVC developer environment.
"""
from pathlib import Path
import argparse
import subprocess
import test_client_receive_dispatch as receive

ROOT = receive.ROOT

TESTS = r'''
#include "KoukuSaydonPatternAuditionService.h"
using namespace Client;
using AUDITION_STATE = KOUKU_SAYDON_PATTERN_AUDITION_STATE;
using LIFE = KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE;
std::uint64_t FakeNow = 100u;
bool Connected = true;
std::vector<C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST> Sent;
CNetworkManager& CNetworkManager::Get() { static CNetworkManager network; return network; }
bool CNetworkManager::Is_Connected() const { return Connected; }
bool CNetworkManager::Send_KoukuSaydonPatternAudition(const C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request) {
    CPacketWriter writer; if (!Write_Message(writer, request)) return false;
    Sent.push_back(request); return true;
}
void Check(bool value, const char* why) { if (!value) throw std::runtime_error(why); }
GameplayDataRevision Revision() { GameplayDataRevision r; r.Bytes[0] = 1u; return r; }
template<class T> PACKET_FRAME Frame(PACKET_TYPE type, const T& message) {
    CPacketWriter writer; Check(Write_Message(writer, message), "wire encoding");
    PACKET_FRAME frame; frame.ePacketType = type; frame.Payload = writer.Get_Buffer(); return frame;
}
auto& net = CNetworkManager::Get();
auto& service = CKoukuSaydonPatternAuditionService::Get();
std::string status;
void Begin() {
    net.Close_ServerConnection(); Connected = true; FakeNow = 100u; Sent.clear(); service.Reset();
    net.m_eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
    service.Set_TargetBoss("boss.kakulsaydon.g2.kouku", "BOSS_KAKULSAYDON_G2_KOUKU");
    Check(service.Play_Selected("KAKULSAYDON_G1_PATTERN_25", Revision(), 2166u, status), "submit pizza");
}
auto Result() {
    const auto& request = Sent.front(); S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT r;
    r.iRequestSequence = request.iRequestSequence; r.eOperation = request.eOperation; r.Scope = request.Scope;
    r.strRequestedPatternId = request.strPatternId; r.eResult = KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED;
    r.iRoomAuditionEpoch = 8u; r.iBossNetEntityId = 23u; r.PinnedGameplayRevision = Revision();
    r.iPinnedSourceRevision = 2166u; r.strResolvedPatternId = request.strPatternId; return r;
}
auto Lifecycle(LIFE state) {
    const auto r = Result(); S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE m;
    m.iRequestSequence = r.iRequestSequence; m.eOperation = r.eOperation; m.Scope = r.Scope;
    m.iRoomAuditionEpoch = r.iRoomAuditionEpoch; m.iBossNetEntityId = r.iBossNetEntityId;
    m.PinnedGameplayRevision = r.PinnedGameplayRevision; m.iPinnedSourceRevision = r.iPinnedSourceRevision;
    m.strPatternId = r.strRequestedPatternId; m.eState = state;
    m.iPatternSequence = state == LIFE::PENDING ? 0u : 1u; return m;
}
void QueueResult(auto result) { Check(net.Enqueue_InboundFrame(Frame(PACKET_TYPE::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT, result)), "enqueue result"); }
void QueueLife(auto message) { Check(net.Enqueue_InboundFrame(Frame(PACKET_TYPE::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE, message)), "enqueue lifecycle"); }
void Pump() { net.Update(); service.Update(); Check(!net.m_hasProtocolFailure, "protocol stays valid"); }
void DelayWithRawPrefix() {
    for (int i = 0; i < 64; ++i) { auto stale = Lifecycle(LIFE::PENDING); stale.iRequestSequence += 100u; QueueLife(stale); }
    QueueResult(Result()); QueueLife(Lifecycle(LIFE::PENDING)); QueueLife(Lifecycle(LIFE::ACTIVE));
    FakeNow += 25656u; Pump();
    Check(net.m_InboundFrames.size() == 3u, "real 64-frame FIFO leaves exact response pending");
}
void LateActive() {
    Begin(); DelayWithRawPrefix();
    Check(service.Get_Snapshot().eState == AUDITION_STATE::REQUEST_PENDING, "deadline must preserve exact pending request");
    Check(!service.Play_Selected("other", Revision(), 2166u, status), "unresolved request cannot be replaced");
    Pump(); Check(service.Get_Snapshot().Is_Live("KAKULSAYDON_G1_PATTERN_25", 2166u), "late exact ACTIVE is consumed");
}
void QueuedDelay() {
    Begin(); QueueResult(Result()); QueueLife(Lifecycle(LIFE::PENDING)); Pump();
    FakeNow += 15001u; Pump(); Check(service.Get_Snapshot().eState == AUDITION_STATE::QUEUED, "queued deadline preserves owner");
    QueueLife(Lifecycle(LIFE::ACTIVE)); Pump(); Check(service.Get_Snapshot().eState == AUDITION_STATE::ACTIVE, "late queued ACTIVE");
}
void TerminalBatch() {
    Begin(); DelayWithRawPrefix(); QueueLife(Lifecycle(LIFE::PATTERN_COMPLETED)); QueueLife(Lifecycle(LIFE::COMPLETED));
    Pump(); Check(service.Get_Snapshot().eState == AUDITION_STATE::COMPLETED && service.Get_Snapshot().strLivePatternId.empty(), "late full batch finishes without restarting presentation");
    QueueLife(Lifecycle(LIFE::ACTIVE)); Pump(); Check(service.Get_Snapshot().eState == AUDITION_STATE::COMPLETED, "terminal run ignores late ACTIVE");
}
void LateRejection() {
    Begin(); FakeNow += 5001u; Pump(); auto rejected = Result();
    rejected.eResult = KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_SOURCE_REVISION_MISMATCH;
    rejected.iRoomAuditionEpoch = 0u; rejected.iBossNetEntityId = INVALID_NET_ENTITY_ID; rejected.strResolvedPatternId.clear();
    rejected.strReason = "actual stale Product source"; QueueResult(rejected); Pump();
    Check(service.Get_Snapshot().eState == AUDITION_STATE::REJECTED && service.Get_Snapshot().strStatus == rejected.strReason, "late real rejection is visible");
}
void ExactIdentity() {
    Begin(); FakeNow += 5001u; Pump(); auto r = Result(); ++r.iRequestSequence; QueueResult(r);
    auto l = Lifecycle(LIFE::ACTIVE); l.Scope.strGateId = "GATE3"; QueueLife(l);
    l = Lifecycle(LIFE::ACTIVE); ++l.iPinnedSourceRevision; ++l.Scope.iExpectedSourceRevision; QueueLife(l);
    l = Lifecycle(LIFE::ACTIVE); l.PinnedGameplayRevision.Bytes[0] = 9u; l.Scope.ExpectedGameplayRevision = l.PinnedGameplayRevision; QueueLife(l); Pump();
    Check(service.Get_Snapshot().eState == AUDITION_STATE::REQUEST_PENDING, "wrong request Gate and revisions never admit");
    QueueResult(Result()); Pump(); l = Lifecycle(LIFE::ACTIVE); ++l.iRoomAuditionEpoch; QueueLife(l);
    l = Lifecycle(LIFE::ACTIVE); ++l.iBossNetEntityId; QueueLife(l); Pump();
    Check(service.Get_Snapshot().eState == AUDITION_STATE::QUEUED, "wrong epoch or boss never admit");
    QueueLife(Lifecycle(LIFE::COMPLETED)); Pump(); const auto previous = Lifecycle(LIFE::ACTIVE);
    Check(service.Play_Selected("next", Revision(), 2166u, status), "new request after exact completion");
    QueueLife(previous); Pump(); Check(service.Get_Snapshot().eState == AUDITION_STATE::REQUEST_PENDING, "superseded request cannot start new playback");
}
void StopBeforeAdmission(bool completed) {
    Begin(); FakeNow += 5001u; Pump(); Check(service.Get_Snapshot().Can_Stop(), "pending run exposes Stop");
    Check(service.Stop(status) && Sent.size() == 1u, "Stop awaits authoritative epoch");
    QueueResult(Result()); QueueLife(Lifecycle(LIFE::ACTIVE));
    if (completed) { QueueLife(Lifecycle(LIFE::PATTERN_COMPLETED)); QueueLife(Lifecycle(LIFE::COMPLETED)); }
    Pump(); Check(Sent.size() == 2u && Sent.back().eOperation == KOUKUSAYDON_PATTERN_AUDITION_OPERATION::STOP &&
        Sent.back().iExpectedRunEpoch == 8u && Sent.back().Scope.strBossPlacementId == Sent.front().Scope.strBossPlacementId,
        "late admitted or completed epoch receives exact Stop");
}
void Disconnect() {
    Begin(); FakeNow += 5001u; Pump(); Connected = false; service.Update();
    Check(service.Get_Snapshot().eState == AUDITION_STATE::ABORTED, "disconnect remains terminal");
    Begin(); FakeNow += 5001u; Pump(); ++net.m_iWorldInboundGeneration; service.Update();
    Check(service.Get_Snapshot().eState == AUDITION_STATE::ABORTED, "world change remains terminal");
}
int main() {
    int failures = 0;
    const auto run = [&](const char* name, auto test) {
        try { test(); std::cout << "PASS " << name << '\n'; }
        catch (const std::exception& e) { ++failures; std::cout << "FAIL " << name << ": " << e.what() << '\n'; }
    };
    run("raw_fifo_late_active", LateActive); run("queued_late_active", QueuedDelay);
    run("active_completed_one_batch", TerminalBatch); run("late_rejection", LateRejection);
    run("exact_identity_and_superseded", ExactIdentity);
    run("pending_stop", []{StopBeforeAdmission(false);}); run("pending_stop_completed_tail", []{StopBeforeAdmission(true);});
    run("disconnect_world_change", Disconnect); net.Close_ServerConnection();
    return failures ? 1 : 0;
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--baseline-service', type=Path)
    args = parser.parse_args()
    output = args.output.resolve()
    cpp = receive.generate(output, None)
    source = cpp.read_text(encoding='utf-8').removesuffix(receive.TESTS)
    network = receive.read_source(ROOT / 'Client/Private/NetworkManager.cpp')
    source += receive.cpp_function_definition(network, 'bool CNetworkManager::Try_Consume_KoukuSaydonPatternAuditionResult(')
    handle = receive.cpp_function_body(network, 'void CNetworkManager::Handle_Frame(')
    start = handle.index('case PACKET_TYPE::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT:')
    end = handle.find('\n\tcase PACKET_TYPE::', start + 1)
    source = source.replace('default: throw std::runtime_error("unexpected fixture packet");', handle[start:end] + '\ndefault: throw std::runtime_error("unexpected fixture packet");')
    service = receive.read_source(args.baseline_service or ROOT / 'Client/Private/KoukuSaydonPatternAuditionService.cpp')
    source += TESTS + '\n' + service.replace('static_cast<std::uint64_t>(::GetTickCount64())', 'FakeNow')
    cpp.write_text(source, encoding='utf-8')
    header = args.baseline_service.parent.parent / 'Public/KoukuSaydonPatternAuditionService.h' if args.baseline_service else ROOT / 'Client/Public/KoukuSaydonPatternAuditionService.h'
    (output / 'KoukuSaydonPatternAuditionService.h').write_bytes(header.read_bytes())
    (output / 'Client_Defines.h').write_text('#pragma once\n', encoding='utf-8')
    options = ['cl.exe', '/nologo', '/std:c++20', '/EHsc', '/D_UNICODE', '/DUNICODE', '/D_WINDOWS', '/wd4828', '/wd4819', '/MDd', '/D_DEBUG', '/Od', '/Zi', '/FS', '/fp:precise']
    options += ['/I' + str(ROOT / folder) for folder in ('Client/Public','Shared/Public','Engine/Public','Engine/External/imgui','Engine/ThirdPartyLib/PhysX/Inc','Engine/ThirdPartyLib/FMOD/Inc')]
    options += ['/Fo' + str(output) + '/', '/Fd' + str(output / 'compile.pdb')]
    sources = [str(cpp), str(ROOT / 'Client/Private/ClientSessionDiagnostic.cpp')]
    sources += [str(ROOT / 'Shared/Private' / name) for name in ('GameplayDataRevision.cpp','Network/PacketMessages.cpp','Network/PacketFrame.cpp','Network/PacketReader.cpp','Network/PacketWriter.cpp','Network/PacketStreamParser.cpp')]
    exe = output / 'pattern_lifecycle.exe'
    subprocess.run(options + sources + ['/Fe' + str(exe), '/link','Ws2_32.lib','Psapi.lib','/INCREMENTAL:NO'], check=True)
    return subprocess.run([str(exe)], cwd=output).returncode


if __name__ == '__main__':
    raise SystemExit(main())

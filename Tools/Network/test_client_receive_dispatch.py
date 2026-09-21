"""Compile the real receive/dispatch/queue functions into a socket-free regression.

The actual NetworkManager header, packet codecs, diagnostics, selected Handle_Frame
cases, reset, close, FIFO admission and snapshot coalescing run unchanged. Only the
unrelated filesystem presentation-baseline acquisition is isolated. No Client or
Server executable is launched. Requires an x64 MSVC developer environment.
"""
from __future__ import annotations

import argparse
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/Build"))
from cpp_source_domains import cpp_function_definition, cpp_function_body


PREAMBLE = r'''
#include "NetworkManager.h"
#include "ValtanPatternTree.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <utility>
using namespace LostArk::Shared;
std::vector<PACKET_FRAME> Observed;
constexpr std::uint64_t ENTRY_PRESENTATION_BASELINE_RETRY_MILLISECONDS = 250u;
// No presentation reader is created in this network-only fixture.
struct Client::CValtanPresentationGenerationReadAdmission::STATE {};
Client::CValtanPresentationGenerationReadAdmission::~CValtanPresentationGenerationReadAdmission() = default;
bool CapturePresentationArtifactBaseline(
    std::vector<CNetworkManager::PRESENTATION_ARTIFACT_BASELINE>&,
    Client::VALTAN_PRESENTATION_GENERATION_RECEIPT&, std::string& status,
    Client::VALTAN_CANONICAL_READ_DIAGNOSTIC*)
{
    status = "Presentation filesystem admission is outside this receive-queue regression.";
    return false;
}
'''

TESTS = r'''
void Check(bool value, const char* why) { if (!value) throw std::runtime_error(why); }
GameplayDataRevision Revision() { GameplayDataRevision r; r.Bytes[0] = 1u; return r; }
template<class T> PACKET_FRAME Frame(PACKET_TYPE type, const T& message) {
    CPacketWriter writer; Check(Write_Message(writer, message), "fixture wire encoding");
    PACKET_FRAME frame; frame.ePacketType = type; frame.Payload = writer.Get_Buffer(); return frame;
}
PACKET_FRAME Lifecycle(std::uint32_t sequence) {
    S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE m;
    m.iRequestSequence = sequence; m.iRoomAuditionEpoch = 1u; m.iBossNetEntityId = 2u;
    m.Scope.eWorldId = WORLD_ID::BERN; m.Scope.strEncounterId = "encounter.kouku";
    m.Scope.strGateId = "GATE2"; m.Scope.strBossPlacementId = "boss.kouku";
    m.Scope.strBossArchetypeId = "KOUKU"; m.Scope.ExpectedGameplayRevision = Revision();
    m.Scope.iExpectedSourceRevision = 1u; m.strPatternId = "pattern.fixture";
    m.iPatternSequence = sequence; m.PinnedGameplayRevision = Revision(); m.iPinnedSourceRevision = 1u;
    m.eState = KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ACTIVE;
    return Frame(PACKET_TYPE::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE, m);
}
PACKET_FRAME Snapshot(std::uint32_t tick) {
    S2C_WORLD_SNAPSHOT m; m.eWorldId = WORLD_ID::BERN; m.ActiveGameplayRevision = Revision(); m.iServerTick = tick;
    PLAYER_SNAPSHOT player; player.iNetEntityId = 9; player.eCharacterClass = CHARACTER_CLASS_ID::SLAYER;
    m.Players.push_back(player);
    return Frame(PACKET_TYPE::S2C_WORLD_SNAPSHOT, m);
}
PACKET_FRAME Teleport(std::uint32_t sequence) {
    S2C_DEBUG_TELEPORT_TO_POSITION_RESULT m; m.iRequestSequence = sequence; m.eWorldId = WORLD_ID::BERN;
    return Frame(PACKET_TYPE::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT, m);
}
void SeedWorld(CNetworkManager& n) {
    n.m_eWorldId = WORLD_ID::BERN; n.m_GameplayRevisionState.ServerActiveRevision = Revision();
}
void Queue(CNetworkManager& n, PACKET_FRAME frame) { Check(n.Enqueue_InboundFrame(std::move(frame)), "raw enqueue"); }
void Drain(CNetworkManager& n, std::vector<std::uint32_t>& ids) {
    S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE m;
    while (n.Try_Consume_KoukuSaydonPatternAuditionLifecycle(m)) ids.push_back(m.iRequestSequence);
    Client::CLIENT_REPLICATION_EVENT e;
    while (n.Try_Consume_ReplicationEvent(e)) {}
}
void Burst() {
    CNetworkManager n; SeedWorld(n); Observed.clear(); std::vector<PACKET_FRAME> expected;
    for (std::uint32_t i = 1; i <= 130; ++i) {
        expected.push_back(Lifecycle(i)); if (i % 3 == 0) expected.push_back(Snapshot(i));
    }
    for (const auto& frame : expected) Queue(n, frame);
    std::vector<std::uint32_t> ids; std::size_t pumps = 0;
    while (!n.m_InboundFrames.empty() && pumps++ < 20) {
        const auto previous = Observed.size(); n.Update();
        Check(!n.m_hasProtocolFailure.load(), "130 lifecycle burst must not fail before consumer runs");
        Check(Observed.size() - previous <= CNetworkManager::MAX_INBOUND_DISPATCH_PER_UPDATE, "dispatch budget");
        Drain(n, ids);
    }
    Check(n.m_InboundFrames.empty() && ids.size() == 130, "all reliable lifecycle events delivered");
    for (std::size_t i = 0; i < ids.size(); ++i) Check(ids[i] == i + 1, "lifecycle FIFO");
    Check(Observed.size() == expected.size(), "mixed frame delivery count");
    for (std::size_t i = 0; i < expected.size(); ++i)
        Check(Observed[i].ePacketType == expected[i].ePacketType && Observed[i].Payload == expected[i].Payload, "mixed packet FIFO");
}
void Backpressure() {
    CNetworkManager n; SeedWorld(n);
    for (std::uint32_t i = 1; i <= 64; ++i) n.Handle_Frame(Lifecycle(i));
    Observed.clear(); Queue(n, Lifecycle(65)); Queue(n, Teleport(99)); Queue(n, Lifecycle(66));
    n.Update(); Check(!n.m_hasProtocolFailure && n.m_InboundFrames.size() == 3 && Observed.empty(), "full destination retains FIFO head");
    S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE first;
    Check(n.Try_Consume_KoukuSaydonPatternAuditionLifecycle(first) && first.iRequestSequence == 1, "consumer frees one slot");
    n.Update(); Check(!n.m_hasProtocolFailure && n.m_InboundFrames.size() == 1 && Observed.size() == 2, "partial capacity resumes exact FIFO prefix");
    Check(Observed[0].ePacketType == PACKET_TYPE::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE &&
          Observed[1].ePacketType == PACKET_TYPE::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT, "unrelated destination can follow delivered head");
    std::vector<std::uint32_t> ids; Drain(n, ids); n.Update(); Drain(n, ids);
    Check(ids.size() == 65 && ids.front() == 2 && ids.back() == 66, "backpressured reliable suffix retained");
    S2C_DEBUG_TELEPORT_TO_POSITION_RESULT teleport;
    Check(n.Try_Consume_DebugTeleportResult(teleport) && teleport.iRequestSequence == 99, "mixed control reply preserved");
}
void OtherDestination() {
    CNetworkManager n; SeedWorld(n); n.m_DebugTeleportResults.resize(64);
    Queue(n, Teleport(99)); Queue(n, Lifecycle(1)); Observed.clear(); n.Update();
    Check(Observed.empty() && n.m_InboundFrames.size() == 2 && !n.m_hasProtocolFailure, "other full typed queue retains head and later lifecycle");
    n.m_DebugTeleportResults.clear(); n.Update();
    Check(n.m_InboundFrames.empty() && Observed.size() == 2 && !n.m_hasProtocolFailure, "other typed queue resumes after drain");
}
void EntryBoundary(bool split) {
    CNetworkManager n; SeedWorld(n); const auto generation = n.m_iWorldInboundGeneration;
    for (std::uint32_t i = 1; i <= (split ? 63u : 1u); ++i) Queue(n, Lifecycle(i));
    S2C_ENTER_ACCEPTED entry; entry.eWorldId = WORLD_ID::BERN; entry.iPlayerId = 7; entry.iNetEntityId = 9; entry.ActiveGameplayRevision = Revision();
    Queue(n, Frame(PACKET_TYPE::S2C_ENTER_ACCEPTED, entry));
    S2C_PLAYER_SPAWNED spawn; spawn.iPlayerId = 7; spawn.iNetEntityId = 9; spawn.eCharacterClass = CHARACTER_CLASS_ID::SLAYER; spawn.strNickName = "Fixture";
    Queue(n, Frame(PACKET_TYPE::S2C_PLAYER_SPAWNED, spawn)); Queue(n, Snapshot(44));
    n.Update();
    Check(!n.m_hasProtocolFailure && n.m_iWorldInboundGeneration == generation + 1, "entry advances generation");
    Check(n.m_KoukuSaydonPatternAuditionLifecycleEvents.empty(), "entry discards old-world typed lifecycle");
    if (split) { Check(n.m_InboundFrames.size() == 2, "entry retains next-batch initial-world frames"); n.Update(); }
    Check(n.m_InboundFrames.empty() && n.m_hasLocalSpawn && n.m_LocalSpawn.iPlayerId == 7, "entry retains same-batch local spawn");
    Client::CLIENT_REPLICATION_EVENT e;
    Check(n.Try_Consume_ReplicationEvent(e) && e.eType == Client::CLIENT_REPLICATION_EVENT_TYPE::PLAYER_SPAWNED, "new-world spawn ordered first");
    Check(n.Try_Consume_ReplicationEvent(e) && e.eType == Client::CLIENT_REPLICATION_EVENT_TYPE::WORLD_SNAPSHOT && e.WorldSnapshot.iServerTick == 44, "new-world snapshot follows spawn");
}
void ResetAndTerminal() {
    CNetworkManager n; SeedWorld(n); for (std::uint32_t i = 1; i <= 70; ++i) Queue(n, Lifecycle(i));
    n.Update(); Check(n.m_InboundFrames.size() == 6, "reset fixture has pending suffix");
    const auto generation = n.m_iWorldInboundGeneration; n.Close_ServerConnection();
    Check(n.m_InboundFrames.empty() && n.m_KoukuSaydonPatternAuditionLifecycleEvents.empty() && n.m_iWorldInboundGeneration == generation + 1, "connection close clears suffix and typed queues");
    SeedWorld(n); Queue(n, Lifecycle(200)); n.Update(); std::vector<std::uint32_t> ids; Drain(n, ids);
    Check(ids.size() == 1 && ids[0] == 200, "next generation has no stale suffix");
    PACKET_FRAME invalid; invalid.ePacketType = PACKET_TYPE::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE;
    Queue(n, invalid); Queue(n, Lifecycle(201)); n.Update();
    Check(n.m_hasProtocolFailure && n.m_InboundFrames.empty() && n.m_KoukuSaydonPatternAuditionLifecycleEvents.empty(), "terminal failure clears undelivered suffix");
}
void ReplicationCapacity() {
    CNetworkManager n; SeedWorld(n); n.m_ReplicationEvents.resize(CNetworkManager::MAX_REPLICATION_EVENT_QUEUE);
    Queue(n, Snapshot(88)); n.Update(); Check(n.m_InboundFrames.size() == 1 && !n.m_hasProtocolFailure, "full reliable replication queue pauses snapshot");
    n.m_ReplicationEvents.back().eType = Client::CLIENT_REPLICATION_EVENT_TYPE::WORLD_SNAPSHOT;
    n.m_ReplicationEvents.back().WorldSnapshot.DamageEvents.resize(1);
    n.Update(); Check(n.m_InboundFrames.empty() && n.m_ReplicationEvents.size() == CNetworkManager::MAX_REPLICATION_EVENT_QUEUE, "adjacent snapshot can replace at capacity");
    Check(n.m_ReplicationEvents.back().WorldSnapshot.iServerTick == 88 && n.m_ReplicationEvents.back().WorldSnapshot.DamageEvents.size() == 1, "snapshot state and carried tick event preserved");
}
void RawQueue() {
    CNetworkManager n; SeedWorld(n); for (std::uint32_t i = 1; i <= 30; ++i) Queue(n, Lifecycle(i));
    Queue(n, Snapshot(1)); Queue(n, Snapshot(2)); Queue(n, Lifecycle(31)); Queue(n, Snapshot(3));
    Check(n.m_InboundFrames.size() == 33 && n.m_SessionDiagnostic.Get_Snapshot().iRawSnapshotsCoalesced == 1, "raw coalescing retains lifecycle barrier");
    n.Close_ServerConnection();
    for (std::size_t i = 0; i < CNetworkManager::MAX_INBOUND_FRAME_QUEUE; ++i) Queue(n, Teleport(1));
    Check(!n.Enqueue_InboundFrame(Teleport(2)) && n.m_hasProtocolFailure && n.m_InboundFrames.size() == 4096, "raw 4096 guard unchanged");
}
int main() {
    int failures = 0;
    const auto run = [&](const char* name, auto test) {
        try { test(); std::cout << "PASS " << name << '\n'; }
        catch (const std::exception& e) { ++failures; std::cout << "FAIL " << name << ": " << e.what() << '\n'; }
    };
    run("burst130_mixed_fifo", Burst); run("partial_queue_backpressure", Backpressure);
    run("other_destination_full", OtherDestination); run("entry_same_batch", []{EntryBoundary(false);});
    run("entry_dispatch_boundary", []{EntryBoundary(true);}); run("connection_reset_terminal", ResetAndTerminal);
    run("replication_full_snapshot_coalesce", ReplicationCapacity); run("raw_coalesce_and_4096_guard", RawQueue);
    return failures ? 1 : 0;
}
'''


def read_source(path: Path) -> str:
    raw = path.read_bytes()
    for encoding in ("utf-8-sig", "cp949"):
        try:
            return raw.decode(encoding).replace("\r\n", "\n")
        except UnicodeDecodeError:
            pass
    raise UnicodeError(path)


def generate(output: Path, baseline_update: Path | None) -> Path:
    source = read_source(ROOT / "Client/Private/NetworkManager.cpp")
    header = read_source(ROOT / "Client/Public/NetworkManager.h")
    output.mkdir(parents=True, exist_ok=True)
    # Access only; production members and their types remain the real declaration.
    (output / "NetworkManager.h").write_text(header.replace("private:", "public:"), encoding="utf-8")
    methods = ["void CNetworkManager::Update()", "bool CNetworkManager::Has_DispatchCapacity(",
        "bool CNetworkManager::Enqueue_InboundFrame(", "bool CNetworkManager::Enqueue_ReplicationEvent(",
        "void CNetworkManager::Fail_Protocol(", "void CNetworkManager::Reset_WorldInboundState()",
        "void CNetworkManager::Close_ServerConnection()", "void CNetworkManager::Record_WorldRevisionSet(",
        "void CNetworkManager::Prune_PresentationAliases()", "bool CNetworkManager::Is_AnnouncedWorldRevision(",
        "bool CNetworkManager::Is_PresentationRevisionAvailable(", "void CNetworkManager::Record_PresentationIsolation(",
        "bool CNetworkManager::Try_Consume_KoukuSaydonPatternAuditionLifecycle(",
        "bool CNetworkManager::Try_Consume_ReplicationEvent(", "bool CNetworkManager::Try_Consume_DebugTeleportResult("]
    definitions = []
    for signature in methods:
        text = read_source(baseline_update) if baseline_update and signature == methods[0] else source
        definitions.append(cpp_function_definition(text, signature))
    handle = cpp_function_body(source, "void CNetworkManager::Handle_Frame(")
    packets = ["S2C_ENTER_ACCEPTED", "S2C_PLAYER_SPAWNED", "S2C_WORLD_SNAPSHOT",
        "S2C_DEBUG_TELEPORT_TO_POSITION_RESULT", "S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE"]
    cases = []
    for packet in packets:
        start = handle.index("case PACKET_TYPE::" + packet + ":")
        end = handle.find("\n\tcase PACKET_TYPE::", start + 1)
        cases.append(handle[start:end])
    body = '''void CNetworkManager::Handle_Frame(const PACKET_FRAME& frame) {
        if (m_hasProtocolFailure.load()) return;
        Observed.push_back(frame);
        CPacketReader reader{frame.Payload};
        switch(frame.ePacketType) {\n''' + "\n".join(cases) + "\ndefault: throw std::runtime_error(\"unexpected fixture packet\");\n}}\n"
    generated = output / "receive_dispatch.cpp"
    generated.write_text(PREAMBLE + "\n".join(definitions) + body + TESTS, encoding="utf-8")
    return generated


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--configuration", choices=("Debug", "Release"), default="Debug")
    parser.add_argument("--baseline-update", type=Path)
    parser.add_argument("--compile-product", action="store_true")
    args = parser.parse_args()
    output = args.output.resolve()
    cpp = generate(output, args.baseline_update)
    debug = args.configuration == "Debug"
    options = ["cl.exe", "/nologo", "/std:c++20", "/EHsc", "/D_UNICODE", "/DUNICODE", "/D_WINDOWS",
        "/wd4828", "/wd4819", "/MDd" if debug else "/MD", "/D_DEBUG" if debug else "/DNDEBUG",
        "/Od" if debug else "/O2", "/Zi", "/FS", "/fp:precise"]
    includes = ["Client/Public", "Shared/Public", "Engine/Public", "Engine/External/imgui", "Engine/ThirdPartyLib/PhysX/Inc", "Engine/ThirdPartyLib/FMOD/Inc"]
    options += ["/I" + str(ROOT / folder) for folder in includes]
    options += ["/Fo" + str(output) + "/", "/Fd" + str(output / "compile.pdb")]
    if args.compile_product:
        subprocess.run(options + ["/c", str(ROOT / "Client/Private/NetworkManager.cpp")], check=True)
    exe = output / "receive_dispatch.exe"
    sources = [str(cpp), str(ROOT / "Client/Private/ClientSessionDiagnostic.cpp")]
    # Build current wire codecs too: an installed SDK archive can lag the source ABI.
    sources += [str(ROOT / "Shared/Private" / source) for source in (
        "GameplayDataRevision.cpp", "Network/PacketMessages.cpp", "Network/PacketFrame.cpp",
        "Network/PacketReader.cpp", "Network/PacketWriter.cpp", "Network/PacketStreamParser.cpp")]
    subprocess.run(options + sources + ["/Fe" + str(exe), "/link", "Ws2_32.lib", "Psapi.lib", "/INCREMENTAL:NO"], check=True)
    return subprocess.run([str(exe)], cwd=output).returncode


if __name__ == "__main__":
    raise SystemExit(main())

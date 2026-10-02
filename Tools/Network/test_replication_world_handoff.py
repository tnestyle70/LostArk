"""Run production replication admission/Update against the real receive queue.

Initialize and Update are extracted unchanged from ClientReplication.cpp. Only
their graphics/navigation/presentation dependencies are inert fixture sinks; the
NetworkManager declaration, entry generation, wire codecs, dispatch, FIFO, reset
and close are the existing receive regression's production implementation. Socket
connectivity uses the real receive-running flag without a socket; balance polling
is inert. This does not launch Client/Server. Requires x64 MSVC.
"""
from __future__ import annotations

import argparse
from pathlib import Path
import re
import subprocess

import test_client_receive_dispatch as receive

ROOT = receive.ROOT

SUPPORT = r'''
#include <map>
CNetworkManager& CNetworkManager::Get() { static CNetworkManager network; return network; }
bool CNetworkManager::Is_Connected() const { return m_isReceiveRunning.load(); }
void CNetworkManager::Pump_BalanceSnapshot() {}
namespace Engine { struct CProfilerScope { template<class... T> CProfilerScope(T&&...) {} }; }
void Client::CPlayerSkillCatalog::Apply_ServerNumericSnapshot(const std::vector<BALANCE_NUMERIC_ENTRY>&) {}
namespace Client {
using bool_t = bool;
struct CGameInstance {
    static CGameInstance& Get() { static CGameInstance instance; return instance; }
    int Get_Profiler() { return 0; }
    unsigned Get_CurrentLevelID() { return Level; }
    void Stop_LoopingSound() { ++StoppedSounds; }
    unsigned Level = ETOUI(LEVEL::BERN), StoppedSounds = 0;
};
struct CCharacterSelectionState {
    static inline unsigned Captures = 0;
    static void Capture_ActiveWorldState() { ++Captures; }
    static bool Apply_RestoreResult(const S2C_RESTORE_CHARACTER_RESULT&) { return true; }
    static void Apply_CaptureResult(const S2C_CAPTURE_CHARACTER_RESULT& result, std::uint64_t generation) { CaptureResult = result; CaptureGeneration = generation; }
    static inline S2C_CAPTURE_CHARACTER_RESULT CaptureResult;
    static inline std::uint64_t CaptureGeneration = 0;
};
struct CCombatHUDViewModel {
    static CCombatHUDViewModel& Get() { static CCombatHUDViewModel instance; return instance; }
    bool Initialize_Definitions() { return true; }
    void Apply_RestoredHonorTitle(HONOR_TITLE_ID) {}
    void Apply_UpgradeEquipmentResult(const S2C_UPGRADE_EQUIPMENT_RESULT& result) { UpgradeResult = result; }
    const S2C_UPGRADE_EQUIPMENT_RESULT& Get_UpgradeEquipmentResult() const { return UpgradeResult; }
    const S2C_INVENTORY_SNAPSHOT& Get_Inventory() const { return Inventory; }
    struct PLAYER { bool isValid = true; } Player;
    const PLAYER& Get_Player() const { return Player; }
    S2C_UPGRADE_EQUIPMENT_RESULT UpgradeResult;
    S2C_INVENTORY_SNAPSHOT Inventory;
    void Set_InteractPromptTriggerId(const std::string&) {}
    void Apply_ServerNumericSnapshot(const std::vector<BALANCE_NUMERIC_ENTRY>&) {}
};
struct CLevel_Bern { static CLevel_Bern* Get_Active() { static CLevel_Bern bern; return &bern; } };
struct UpgradeView { void Set_SlotVisible(const char*, bool) {} };
double FixtureNow = 0.0;
double Product_Now_Seconds() { return FixtureNow; }
struct CMainApp {
    void Update_ItemUpgradeServerResult();
    std::uint32_t m_iPendingItemUpgradeRequest = 41;
    std::string m_strItemUpgradeAttemptItemId = "EQUIP_SLAYER_HONORWHISPER_WEAPON";
    EQUIPMENT_SLOT m_eItemUpgradeAttemptSlot = EQUIPMENT_SLOT::WEAPON;
    bool m_bItemUpgradeResultUnavailable = false, m_bItemUpgradePendingAttemptSuccess = false;
    std::uint16_t m_iItemUpgradeConfirmedLevel = 10;
    double m_dItemUpgradeRequestDeadline = 5.0;
    UpgradeView View;
    UpgradeView* m_pItemUpgradeView = &View;
};
struct CLevelTransitionService {
    template<class... T> static void Report_Recovery(T&&...) {}
    template<class... T> static bool Request_Load(T&&...) { return true; }
};
struct CKoukuSaydonPresentationAssetService {
    template<class... T> static bool Matches_AdmittedRun(T&&...) { return true; }
    template<class... T> static bool Admit_RunProduct(T&&...) { return true; }
};
bool Is_KoukuSaydonArenaBoss(const std::string&, const std::string&, NET_ENTITY_ID) { return false; }
struct MAP_NAVIGATION_CONTRACT { bool runtimeGridAvailable = true; std::string prototypeTag = "fixture.navigation"; };
struct CMapNavigationContract {
    static bool Resolve_Area(const std::string&, MAP_NAVIGATION_CONTRACT&, std::string&) { return true; }
};
struct Projection { bool Is_Ready() const { return true; } void Reset() {} };
// State/sinks are deliberately small. No ownership predicate is reimplemented here.
class CClientReplication {
public:
    struct DESC {
        void* pDevice = this;
        void* pContext = this;
        std::string strMapAreaId = "fixture.area", strPlayerLayerTag = "Layer_Player", strWorldEntityLayerTag = "Layer_WorldEntity";
        Projection* pDeployPropRuntime = nullptr;
        Projection* pWorldDestructionProjection = nullptr;
        unsigned iLayerLevelIndex = ETOUI(LEVEL::BERN);
    };
    bool Initialize(const DESC&);
    bool Update();
    void Reset();
    void Reset_World();
    void Sync_GlobalCombatDebugVisibility() {}
    void Clear_DeferredLocalCharacterClassReplacement() {}
    bool Advance_PlayerAssetPreparation() { ++Advances; return true; }
    void Advance_GuideBubbles() {}
    void Update_DeathPresentations() {}
    void Update_WaterGunSpeedAnchors() {}
    void Draw_CombatObjectHitAreaDebug() {}
    bool Apply_Spawn(const S2C_PLAYER_SPAWNED& spawn) {
        Spawns.push_back(spawn.iNetEntityId); Order.push_back(CLIENT_REPLICATION_EVENT_TYPE::PLAYER_SPAWNED); return true;
    }
    bool Apply_WorldSnapshot(const S2C_WORLD_SNAPSHOT& snapshot) {
        Ticks.push_back(snapshot.iServerTick); Order.push_back(CLIENT_REPLICATION_EVENT_TYPE::WORLD_SNAPSHOT); return true;
    }
#define INERT_SINK(name) template<class T> bool name(const T&) { return true; }
    INERT_SINK(Apply_WorldEntitySpawn) INERT_SINK(Apply_CombatObjectSpawn)
    INERT_SINK(Apply_CombatObjectPresentationEvent) INERT_SINK(Apply_WorldEntityDespawn)
    INERT_SINK(Apply_CombatObjectDespawn) INERT_SINK(Apply_Despawn)
    INERT_SINK(Apply_WorldDestructionFullSync) INERT_SINK(Apply_WorldDestructionDelta)
    INERT_SINK(Apply_EncounterPropSync) INERT_SINK(Apply_InventorySnapshot)
    INERT_SINK(Apply_PartyInviteReceived) INERT_SINK(Apply_PartyRoster)
    INERT_SINK(Apply_GuidePrompt) INERT_SINK(Apply_GuideState) INERT_SINK(Apply_ChatReceived)
#undef INERT_SINK
    DESC m_Desc;
    std::uint64_t m_iOwnedWorldInboundGeneration = 0u;
    bool m_isInitialized = false, m_wasConnected = false, m_hasPendingConnectionLoss = false;
    bool m_hasFatalWorldDestructionFailure = false;
    std::string m_strLocalPlayerNavigationPrototypeTag;
    Projection m_WorldDestructionProjectionRuntime;
    std::uint64_t m_iNextDeferredLocalCharacterClassReplacementGeneration = 0;
    std::vector<int> m_PendingWorldCombatHits;
    S2C_PARTY_TRANSFER_RESULT m_PendingPartyTransferResult;
    S2C_RAID_ENTRY_PROMPT m_PendingRaidEntryPrompt;
    S2C_RAID_ENTRY_VOTE m_PendingRaidEntryVote;
    bool m_hasPendingPartyTransferResult = false, m_hasPendingRaidEntryPrompt = false, m_hasPendingRaidEntryVote = false;
    std::string m_strInteractPromptTriggerId, m_strPendingPresentationFailure;
    S2C_KOUKUSAYDON_RAID_STATE m_KoukuRaidReply, m_KoukuRaidState;
    S2C_KOUKUSAYDON_BUNDLE_STATE m_KoukuBundleState;
    std::uint32_t m_iKoukuRaidReplyRequestSequence = 0;
    std::uint64_t m_iKoukuRaidReplyWorldGeneration = 0;
    struct Entity { std::string strArchetypeId, strEncounterId, strActiveActionId; NET_ENTITY_ID iOwnerBossNetEntityId = 0; };
    std::map<NET_ENTITY_ID, Entity> m_WorldEntities;
    std::vector<S2C_WORLD_SEQUENCE_PLAY> m_PendingWorldSequencePlays;
    struct { bool bCombatObjectHit = false; } m_CombatDebugVisibility;
    unsigned Resets = 0, Advances = 0;
    std::vector<NET_ENTITY_ID> Spawns;
    std::vector<std::uint32_t> Ticks;
    std::vector<CLIENT_REPLICATION_EVENT_TYPE> Order;
};
}
'''

TESTS = r'''
using Client::CClientReplication;
void Check(bool value, const char* why) { if (!value) throw std::runtime_error(why); }
GameplayDataRevision Revision() { GameplayDataRevision r; r.Bytes[0] = 1u; return r; }
template<class T> PACKET_FRAME Frame(PACKET_TYPE type, const T& message) {
    CPacketWriter writer; Check(Write_Message(writer, message), "fixture wire encoding");
    PACKET_FRAME frame; frame.ePacketType = type; frame.Payload = writer.Get_Buffer(); return frame;
}
auto& net = CNetworkManager::Get();
void Queue(PACKET_FRAME frame) { Check(net.Enqueue_InboundFrame(std::move(frame)), "raw enqueue"); }
void Begin() {
    net.Close_ServerConnection(); net.m_isReceiveRunning.store(true);
    net.m_eWorldId = WORLD_ID::BERN; net.m_GameplayRevisionState.ServerActiveRevision = Revision();
    Client::CCharacterSelectionState::Captures = 0;
}
void Init(CClientReplication& consumer) {
    Check(consumer.Initialize({}), "production replication Initialize");
}
void QueueSpawnSnapshot(WORLD_ID world, std::uint32_t tick) {
    S2C_PLAYER_SPAWNED spawn; spawn.iPlayerId = 7; spawn.iNetEntityId = 9;
    spawn.eCharacterClass = CHARACTER_CLASS_ID::SLAYER; spawn.strNickName = "Fixture";
    Queue(Frame(PACKET_TYPE::S2C_PLAYER_SPAWNED, spawn));
    S2C_WORLD_SNAPSHOT snapshot; snapshot.eWorldId = world; snapshot.ActiveGameplayRevision = Revision(); snapshot.iServerTick = tick;
    PLAYER_SNAPSHOT player; player.iNetEntityId = 9; player.eCharacterClass = CHARACTER_CLASS_ID::SLAYER;
    snapshot.Players.push_back(player); Queue(Frame(PACKET_TYPE::S2C_WORLD_SNAPSHOT, snapshot));
}
void Enter(WORLD_ID world) {
    S2C_ENTER_ACCEPTED entry; entry.eWorldId = world; entry.iPlayerId = 7; entry.iNetEntityId = 9; entry.ActiveGameplayRevision = Revision();
    Queue(Frame(PACKET_TYPE::S2C_ENTER_ACCEPTED, entry)); QueueSpawnSnapshot(world, 44); net.Update();
    Check(!net.m_hasProtocolFailure && net.m_InboundFrames.empty(), "entry dispatch succeeds");
    S2C_ENTER_ACCEPTED consumed;
    Check(net.Try_Consume_EnterAccepted(consumed) && consumed.eWorldId == world, "old level consumes one-shot approval before delayed transition");
}
void CheckDelivery(const CClientReplication& consumer) {
    Check(consumer.Spawns == std::vector<NET_ENTITY_ID>{9} && consumer.Ticks == std::vector<std::uint32_t>{44}, "new consumer receives exact initial spawn and snapshot");
    Check(consumer.Order == std::vector<Client::CLIENT_REPLICATION_EVENT_TYPE>{
        Client::CLIENT_REPLICATION_EVENT_TYPE::PLAYER_SPAWNED, Client::CLIENT_REPLICATION_EVENT_TYPE::WORLD_SNAPSHOT}, "spawn precedes snapshot");
}
void DelayedTransition(WORLD_ID target) {
    Begin(); CClientReplication previous; Init(previous); const auto oldGeneration = net.Get_WorldInboundGeneration();
    Enter(target); Check(net.Get_WorldInboundGeneration() != oldGeneration, "acceptance advances world generation");
    for (int frame = 0; frame < 4; ++frame) {
        Check(previous.Update(), "retiring level remains harmless while async resources drain");
        Check(previous.Spawns.empty() && previous.Ticks.empty() && previous.Advances == 0, "retiring consumer cannot drain next-world spawn/snapshot or prepare its assets");
        Check(net.m_ReplicationEvents.size() == 2, "next-world reliable events retained through delayed transition");
    }
    previous.Reset(); CClientReplication next; Init(next); Check(next.Update(), "new level consumes its generation");
    CheckDelivery(next); Check(net.m_ReplicationEvents.empty() && next.Advances == 1, "new level drains queue once");
    Check(next.Update(), "ordinary subsequent update"); CheckDelivery(next);
}
void SameGeneration() {
    Begin(); CClientReplication current; Init(current); QueueSpawnSnapshot(WORLD_ID::BERN, 44); net.Update();
    Check(current.Update(), "current world Update"); CheckDelivery(current);
}
void ResetInvalidates() {
    Begin(); CClientReplication retired; Init(retired); retired.Reset();
    QueueSpawnSnapshot(WORLD_ID::BERN, 44); net.Update();
    Check(retired.Update() && retired.Spawns.empty() && retired.Advances == 0 && net.m_ReplicationEvents.size() == 2,
        "explicit reset revokes queue ownership even without another acceptance");
    CClientReplication next; Init(next); Check(next.Update(), "replacement after reset"); CheckDelivery(next);
}
void DisconnectedCleanup() {
    Begin(); CClientReplication current; Init(current); const auto owned = net.Get_WorldInboundGeneration();
    net.Close_ServerConnection(); Check(net.Get_WorldInboundGeneration() != owned, "close changes generation");
    Client::CLIENT_REPLICATION_EVENT leftover; leftover.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::PLAYER_SPAWNED;
    Check(net.Enqueue_ReplicationEvent(std::move(leftover)), "seed disconnected cleanup suffix");
    Check(current.Update() && current.m_hasPendingConnectionLoss && !current.m_wasConnected && current.Resets == 1,
        "generation guard must not bypass disconnect cleanup");
    Check(Client::CCharacterSelectionState::Captures == 1 && net.m_ReplicationEvents.empty() && current.Spawns.empty(), "disconnect captures state and drains stale events");
    Check(current.Update() && current.Resets == 1, "disconnect cleanup runs once");
}
void HandlerClose() {
    Begin(); CClientReplication current; Init(current);
    Client::CLIENT_REPLICATION_EVENT rejected; rejected.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::RESTORE_CHARACTER_RESULT;
    rejected.RestoreCharacterResult.eResult = CHARACTER_RESTORE_RESULT::REJECTED_SESSION;
    Check(net.Enqueue_ReplicationEvent(std::move(rejected)), "seed restore rejection");
    Check(current.Update() && !net.Is_Connected() && current.Advances == 0, "handler close stops remaining generation work in this Update");
    Check(current.Update() && current.m_hasPendingConnectionLoss && current.Resets == 1, "handler close still reports disconnect next Update");
}
void EconomyConsumer() {
    Begin(); CClientReplication current; Init(current);
    S2C_UPGRADE_EQUIPMENT_RESULT upgrade; upgrade.iRequestSequence = 41; upgrade.iUpgradeLevel = 11;
    upgrade.eResult = EQUIPMENT_UPGRADE_RESULT::SUCCEEDED;
    S2C_CAPTURE_CHARACTER_RESULT capture; capture.iRequestSequence = 42;
    capture.eResult = CHARACTER_CAPTURE_RESULT::CAPTURED; capture.iPlayerId = 7; capture.iNetEntityId = 9;
    capture.eCharacterClass = CHARACTER_CLASS_ID::SLAYER; capture.iGold = 900;
    Queue(Frame(PACKET_TYPE::S2C_UPGRADE_EQUIPMENT_RESULT, upgrade));
    Queue(Frame(PACKET_TYPE::S2C_CAPTURE_CHARACTER_RESULT, capture)); net.Update();
    Check(current.Update(), "production replication handles economy replies");
    Check(Client::CCombatHUDViewModel::Get().UpgradeResult.iRequestSequence == 41, "upgrade result reaches HUD");
    Check(Client::CCharacterSelectionState::CaptureResult.iGold == 900 &&
        Client::CCharacterSelectionState::CaptureGeneration == net.Get_WorldInboundGeneration(), "capture reaches active generation consumer");
}
void UpgradeResultMatching() {
    using namespace Client;
    CGameInstance::Get().Level = ETOUI(LEVEL::BERN); FixtureNow = 0;
    auto& hud = CCombatHUDViewModel::Get(); hud.Player.isValid = true; hud.Inventory = {}; hud.UpgradeResult = {};
    INVENTORY_ITEM_SNAPSHOT item; item.strItemId = "EQUIP_SLAYER_HONORWHISPER_WEAPON";
    item.iQuantity = 1; item.eEquippedSlot = EQUIPMENT_SLOT::WEAPON; item.iUpgradeLevel = 11;
    hud.Inventory.Items.push_back(item);
    hud.UpgradeResult.iRequestSequence = 40; hud.UpgradeResult.eWorldId = WORLD_ID::BERN;
    hud.UpgradeResult.eResult = EQUIPMENT_UPGRADE_RESULT::SUCCEEDED; hud.UpgradeResult.iUpgradeLevel = 11;
    CMainApp stale; stale.Update_ItemUpgradeServerResult();
    Check(stale.m_iPendingItemUpgradeRequest == 41 && !stale.m_bItemUpgradePendingAttemptSuccess, "stale sequence cannot reveal success");
    hud.UpgradeResult.iRequestSequence = 41; hud.UpgradeResult.eWorldId = WORLD_ID::VALTAN_ARENA;
    stale.Update_ItemUpgradeServerResult();
    Check(stale.m_iPendingItemUpgradeRequest == 41, "wrong world cannot reveal success");
    hud.UpgradeResult.eWorldId = WORLD_ID::BERN; stale.Update_ItemUpgradeServerResult();
    Check(!stale.m_iPendingItemUpgradeRequest && stale.m_bItemUpgradePendingAttemptSuccess &&
        stale.m_iItemUpgradeConfirmedLevel == 11, "matching reply confirms replicated level without incrementing it");
    Check(hud.Inventory.Items.at(0).iUpgradeLevel == 11, "presentation does not mutate inventory level");
    hud.UpgradeResult.iUpgradeLevel = 12; CMainApp inconsistent; inconsistent.Update_ItemUpgradeServerResult();
    Check(inconsistent.m_bItemUpgradeResultUnavailable && !inconsistent.m_bItemUpgradePendingAttemptSuccess,
        "reply without matching inventory cannot show success");
    hud.UpgradeResult.eResult = EQUIPMENT_UPGRADE_RESULT::REJECTED; CMainApp rejected; rejected.Update_ItemUpgradeServerResult();
    Check(rejected.m_bItemUpgradeResultUnavailable && hud.Inventory.Items.at(0).iUpgradeLevel == 11, "rejection preserves inventory");
    hud.UpgradeResult = {}; FixtureNow = 5.0; CMainApp timeout; timeout.Update_ItemUpgradeServerResult();
    Check(timeout.m_bItemUpgradeResultUnavailable && !timeout.m_iPendingItemUpgradeRequest &&
        hud.Inventory.Items.at(0).iUpgradeLevel == 11, "timeout preserves inventory and releases wait");
}
int main() {
    int failures = 0;
    const auto run = [&](const char* name, auto test) {
        try { test(); std::cout << "PASS " << name << '\n'; }
        catch (const std::exception& e) { ++failures; std::cout << "FAIL " << name << ": " << e.what() << '\n'; }
    };
    run("economy_result_consumers", EconomyConsumer); run("upgrade_result_sequence_world_snapshot_timeout", UpgradeResultMatching);
    run("delayed_bern_to_valtan", [] { DelayedTransition(WORLD_ID::VALTAN_ARENA); });
    run("delayed_bern_to_kouku", [] { DelayedTransition(WORLD_ID::KAKULSAYDON_ARENA); });
    run("same_world_new_generation", [] { DelayedTransition(WORLD_ID::BERN); });
    run("same_generation_normal_delivery", SameGeneration); run("reset_revokes_ownership", ResetInvalidates);
    run("disconnect_cleanup_precedes_guard", DisconnectedCleanup); run("handler_close_stops_generation_work", HandlerClose);
    net.Close_ServerConnection(); return failures ? 1 : 0;
}
'''


def generate(output: Path, baseline: Path | None) -> Path:
    cpp = receive.generate(output, None)
    source = cpp.read_text(encoding="utf-8").removesuffix(receive.TESTS)
    source = "#define REPLICATION_HANDOFF_FIXTURE\n" + receive.PREAMBLE + SUPPORT + source.removeprefix(receive.PREAMBLE)
    network = receive.read_source(ROOT / "Client/Private/NetworkManager.cpp")
    for signature in ("LostArk::Shared::PLAYER_ID CNetworkManager::Get_LocalPlayerId() const",
                      "bool CNetworkManager::Try_Consume_EnterAccepted("):
        source += "\n" + receive.cpp_function_definition(network, signature)
    replication = receive.read_source(baseline or ROOT / "Client/Private/ClientReplication.cpp")
    # The whole methods remain unmodified, including ordering around disconnected
    # cleanup, dispatch, and per-event/post-dispatch generation changes.
    for signature in ("bool Client::CClientReplication::Initialize(",
                      "bool Client::CClientReplication::Update()",
                      "void Client::CClientReplication::Reset()"):
        source += "\n" + receive.cpp_function_definition(replication, signature)
    main_app = receive.read_source(ROOT / "Client/Private/MainApp.cpp")
    source += "\nnamespace Client {\n" + receive.cpp_function_definition(main_app, "void CMainApp::Update_ItemUpgradeServerResult()") + "\n}\n"
    reset = receive.cpp_function_body(replication, "void Client::CClientReplication::Reset_World()")
    revoke = re.search(r"\bm_iOwnedWorldInboundGeneration\s*=\s*0u;", reset)
    # Old source has no ownership to revoke. It must compile and fail runtime
    # assertions, rather than being rejected by the fixture generator.
    source += "\nvoid Client::CClientReplication::Reset_World() { ++Resets; "
    source += revoke.group(0) if revoke else ""
    source += " }\n" + TESTS
    cpp.write_text(source, encoding="utf-8")
    return cpp


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--baseline-replication", type=Path)
    parser.add_argument("--configuration", choices=("Debug", "Release"), default="Debug")
    args = parser.parse_args()
    output = args.output.resolve()
    cpp = generate(output, args.baseline_replication)
    debug = args.configuration == "Debug"
    options = ["cl.exe", "/nologo", "/std:c++20", "/EHsc", "/utf-8", "/D_UNICODE", "/DUNICODE", "/D_WINDOWS",
               "/wd4828", "/wd4819", "/MDd" if debug else "/MD", "/D_DEBUG" if debug else "/DNDEBUG",
               "/Od" if debug else "/O2", "/Zi", "/FS", "/fp:precise"]
    options += ["/I" + str(ROOT / folder) for folder in ("Client/Public", "Shared/Public", "Engine/Public",
                "Engine/External/imgui", "Engine/ThirdPartyLib/PhysX/Inc", "Engine/ThirdPartyLib/FMOD/Inc")]
    options += ["/Fo" + str(output) + "/", "/Fd" + str(output / "compile.pdb")]
    sources = [str(cpp), str(ROOT / "Client/Private/ClientSessionDiagnostic.cpp")]
    sources += [str(ROOT / "Shared/Private" / name) for name in (
        "GameplayDataRevision.cpp", "Network/PacketMessages.cpp", "Network/PacketFrame.cpp",
        "Network/PacketReader.cpp", "Network/PacketWriter.cpp", "Network/PacketStreamParser.cpp")]
    exe = output / "replication_world_handoff.exe"
    subprocess.run(options + sources + ["/Fe" + str(exe), "/link", "Ws2_32.lib", "Psapi.lib", "/INCREMENTAL:NO"], check=True)
    return subprocess.run([str(exe)], cwd=output).returncode


if __name__ == "__main__":
    raise SystemExit(main())

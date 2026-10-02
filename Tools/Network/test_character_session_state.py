"""Compile and run the production process-session roster and selection state.

CharacterRoster.cpp and CharacterSelectionState.cpp are compiled in full with
their actual headers and current Shared packet codecs. Only the graphics-owned
level/HUD observations and appearance voice reader are inert fixture inputs.
Each case runs in a fresh process, so no test-only reset of production globals is
needed. No Client, Server, socket, graphics device or personal roster is opened.
Run from an x64 MSVC developer environment with --output <ignored directory>.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[2]

STUBS = {
    "Engine_Defines.h": r'''#pragma once
#include <cstddef>
#include <cstdint>
#define NS_BEGIN(name) namespace name {
#define NS_END }
using bool_t = bool;
''',
    "Client_Defines.h": r'''#pragma once
#include "Engine_Defines.h"
namespace Client { enum class LEVEL { STATIC, LOADING, LOBBY, CHARACTER_SELECT,
    BERN, VALTAN_ARENA, KAKULSAYDON_ARENA, DEVELOPMENT, MAHARAKA, COLOSSEUM }; }
''',
    "GameInstance.h": r'''#pragma once
#include "Client_Defines.h"
namespace Client { struct CGameInstance {
    static CGameInstance& Get() { static CGameInstance instance; return instance; }
    LEVEL level = LEVEL::BERN;
    unsigned Get_CurrentLevelID() const { return static_cast<unsigned>(level); }
}; }
''',
    "CombatHUDViewModel.h": r'''#pragma once
#include "Network/PacketMessages.h"
namespace Client { struct CCombatHUDViewModel {
    struct PLAYER { bool isValid = false, isPreview = false; unsigned iHonorTitleId = 0; } player;
    LostArk::Shared::S2C_INVENTORY_SNAPSHOT inventory;
    bool hasInventory = false;
    static CCombatHUDViewModel& Get() { static CCombatHUDViewModel instance; return instance; }
    const auto& Get_Inventory() const { return inventory; }
    const auto& Get_Player() const { return player; }
    bool Has_Inventory() const { return hasInventory; }
}; }
''',
    "CustomizingView.h": r'''#pragma once
#include "Network/PacketMessages.h"
namespace Client { struct CCustomizingView {
    static std::uint8_t Read_SavedVoiceType(const std::string&) {
        return LostArk::Shared::MIN_VOICE_TYPE;
    }
}; }
''',
}

FIXTURE = r'''
#include "CharacterRoster.h"
#include "CharacterSelectionState.h"
#include "GameInstance.h"
#include "CombatHUDViewModel.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
#include <array>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
using namespace Client;
using namespace LostArk::Shared;
using Selection = CCharacterSelectionState;
using Roster = CCharacterRoster;
void Check(bool value, const char* why) { if (!value) throw std::runtime_error(why); }
CHARACTER_WORLD_STATE State(unsigned value) {
    CHARACTER_WORLD_STATE state; state.bValid = true;
    state.iSilver = 6000u + value; state.iGold = 200u + value; state.iHonorTitleId = 30000u + value;
    INVENTORY_ITEM_SNAPSHOT gear; gear.strItemId = "FIXTURE_WEAPON"; gear.iQuantity = 1;
    gear.eEquippedSlot = EQUIPMENT_SLOT::WEAPON; gear.iDurabilityPercent = 37;
    gear.iUpgradeLevel = static_cast<std::uint16_t>(10u + value % 10u);
    state.Items.push_back(gear);
    INVENTORY_ITEM_SNAPSHOT avatar; avatar.strItemId = "FIXTURE_AVATAR"; avatar.iQuantity = 1;
    avatar.eEquippedSlot = EQUIPMENT_SLOT::AVATAR_OUTFIT; state.Items.push_back(avatar);
    INVENTORY_ITEM_SNAPSHOT bag; bag.strItemId = "POTION_HP_SMALL"; bag.iQuantity = value + 3u;
    state.Items.push_back(bag); return state;
}
bool Same(const CHARACTER_WORLD_STATE& a, const CHARACTER_WORLD_STATE& b) {
    if (a.bValid != b.bValid || a.iSilver != b.iSilver || a.iGold != b.iGold ||
        a.iHonorTitleId != b.iHonorTitleId || a.Items.size() != b.Items.size()) return false;
    for (size_t i = 0; i < a.Items.size(); ++i) {
        const auto& x = a.Items[i]; const auto& y = b.Items[i];
        if (x.strItemId != y.strItemId || x.iQuantity != y.iQuantity || x.eEquippedSlot != y.eEquippedSlot ||
            x.iDurabilityPercent != y.iDurabilityPercent || x.iUpgradeLevel != y.iUpgradeLevel) return false;
    }
    return true;
}
std::string Create(size_t slot, const std::string& name = "SameName", const std::string& look = "look-a",
    CHARACTER_CLASS_ID type = CHARACTER_CLASS_ID::ARTIST) {
    Check(Selection::Select_CreationSlot(slot), "select empty creation slot");
    Check(Selection::Stage_Creation(type, name, look), "stage creation");
    Check(!Roster::Is_Occupied(slot), "staging must not create a roster entry");
    Check(Selection::Commit_PendingCreation(), "commit after Bern activation");
    return Roster::Get_Entries()[slot].strCharacterId;
}
void Save(const std::string& id, const CHARACTER_WORLD_STATE& state) {
    std::string status; Check(Roster::Update_WorldState(id, state, status), "save fixture authoritative state");
}
void Preserved(const std::string& id, const CHARACTER_WORLD_STATE& expected) {
    CHARACTER_WORLD_STATE actual;
    Check(Roster::Try_Get_WorldState(id, actual) && Same(actual, expected), "complete character state preserved");
}
void Reenter(size_t slot) {
    const auto& entry = Roster::Get_Entries()[slot];
    Check(Selection::Stage_ExistingEntry(entry.eCharacterClass, entry.strNickname,
        entry.strAppearanceJson, entry.strCharacterId), "stage selected existing ID");
    Check(Selection::Commit_PendingCreation(), "commit selected existing ID");
}
void SeedHud(const CHARACTER_WORLD_STATE& state) {
    auto& hud = CCombatHUDViewModel::Get(); hud.player.isValid = true; hud.player.isPreview = false;
    hud.player.iHonorTitleId = state.iHonorTitleId; hud.hasInventory = true;
    hud.inventory.Items = state.Items; hud.inventory.iSilver = state.iSilver; hud.inventory.iGold = state.iGold;
}
S2C_CAPTURE_CHARACTER_RESULT Capture(unsigned seq, const CHARACTER_WORLD_STATE& state) {
    S2C_CAPTURE_CHARACTER_RESULT result;
    result.iRequestSequence = seq; result.eResult = CHARACTER_CAPTURE_RESULT::CAPTURED;
    result.Items = state.Items; result.iSilver = state.iSilver; result.iGold = state.iGold;
    result.iHonorTitleId = state.iHonorTitleId; result.eWorldId = WORLD_ID::BERN;
    result.iPlayerId = 41u; result.iNetEntityId = 91u; result.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
    return result;
}
unsigned Begin() {
    const auto seq = Selection::Next_StateRequestSequence();
    Check(Selection::Begin_WorldStateCapture(seq, WORLD_ID::BERN, 41u, 91u, 7u), "begin capture barrier");
    return seq;
}
void EmptyIdentity() {
    Check(Roster::Get_Entries().size() == 6 && Roster::Get_CharacterCount() == 0, "fresh process has six empty slots");
    std::string status; auto state = State(1); CHARACTER_WORLD_STATE output = State(2);
    Check(!Roster::Update_WorldState("", state, status), "empty identity rejected");
    Check(!Roster::Try_Get_WorldState("", output) && Same(output, State(2)), "empty lookup preserves output");
    Check(!Selection::Begin_WorldStateCapture(1, WORLD_ID::BERN, 41, 91, 7), "capture without active character rejected");
    const auto id = Create(4); Save(id, state); state.bValid = false;
    Check(!Roster::Update_WorldState(id, state, status), "invalid state rejected"); Preserved(id, State(1));
    Check(!Roster::Update_WorldState("unknown-id", State(3), status), "unknown identity rejected");
    Check(Roster::Get_CharacterCount() == 1, "failed save never occupies an empty slot");
}
void SixSlots() {
    const std::array<size_t, 6> order{4, 1, 5, 0, 3, 2}; std::set<std::string> identities;
    for (const auto slot : order) {
        auto id = Create(slot, "SameName", "look-" + std::to_string(slot));
        Check(identities.insert(id).second, "same class and nickname retain independent IDs");
        Save(id, State(static_cast<unsigned>(slot)));
    }
    Check(Roster::Get_CharacterCount() == 6, "all six selected slots occupied");
    for (size_t slot = 0; slot != 6; ++slot) {
        const auto& entry = Roster::Get_Entries()[slot];
        Check(entry.strAppearanceJson == "look-" + std::to_string(slot), "creation slot order never sorted");
        Preserved(entry.strCharacterId, State(static_cast<unsigned>(slot)));
        Reenter(slot); CHARACTER_WORLD_STATE restore;
        Check(Selection::Try_Get_ActiveWorldState(restore) && Same(restore, State(static_cast<unsigned>(slot))),
            "reselect yields only this slot inventory/purse/avatar/upgrade/title");
        const auto seq = Selection::Next_StateRequestSequence(); Selection::Mark_RestoreRequested(seq);
        Check(Selection::Apply_RestoreResult({seq, CHARACTER_RESTORE_RESULT::APPLIED, restore.iHonorTitleId}), "restore selected slot");
    }
    Check(!Selection::Select_CreationSlot(6) && !Selection::Select_CreationSlot(4), "seventh and occupied slots rejected");
}
void CreationRollback() {
    const auto active = Create(4, "Previous", "previous-look"); Save(active, State(1));
    Check(Selection::Select_CreationSlot(1) && Selection::Stage_Creation(CHARACTER_CLASS_ID::SLAYER,
        "Candidate", "candidate-look"), "stage new candidate");
    size_t index = 99; std::string status;
    Check(Roster::Add(1, CHARACTER_CLASS_ID::WARLORD, "Concurrent", "concurrent-look", index, status), "occupy candidate slot before commit");
    Check(!Selection::Commit_PendingCreation(), "failed roster Add must fail commit");
    Selection::Cancel_PendingCreation(); CHARACTER_ENTRY_IDENTITY identity;
    Check(Selection::Try_Resolve_ForWorld(WORLD_ID::BERN, identity) && identity.strNickname == "Previous" &&
        identity.eCharacterClass == CHARACTER_CLASS_ID::ARTIST, "failed creation retains active identity");
    Check(Selection::Get_ActiveAppearanceJson() == "previous-look", "failed creation retains active appearance");
    SeedHud(State(3)); Selection::Capture_ActiveWorldState(); Preserved(active, State(3));
    Check(!Roster::Get_Entries()[1].World.bValid, "old identity remains save target after failed commit");
}
void ExistingValidation() {
    const auto id = Create(3); Save(id, State(1));
    Check(!Selection::Stage_ExistingEntry(CHARACTER_CLASS_ID::ARTIST, "SameName", "tampered", id), "reject altered appearance");
    Check(!Selection::Stage_ExistingEntry(CHARACTER_CLASS_ID::ARTIST, "OtherName", "look-a", id), "reject altered nickname");
    Check(!Selection::Stage_ExistingEntry(CHARACTER_CLASS_ID::SLAYER, "SameName", "look-a", id), "reject altered class");
    Check(!Selection::Stage_ExistingEntry(CHARACTER_CLASS_ID::ARTIST, "SameName", "look-a", ""), "reject empty ID");
    Check(!Selection::Has_PendingCreation(), "invalid selections do not stage an identity"); Preserved(id, State(1));
}
void CaptureSuccess() {
    const auto id = Create(4); Save(id, State(1)); SeedHud(State(0)); const auto seq = Begin();
    Check(Selection::Is_WorldStateSyncPending(), "capture waits before world leave");
    Check(!Selection::Begin_WorldStateCapture(seq + 1, WORLD_ID::BERN, 41, 91, 7), "one capture at a time");
    Selection::Capture_ActiveWorldState(); Preserved(id, State(1));
    const auto result = Capture(seq, State(8)); CPacketWriter writer;
    Check(Write_Message(writer, result), "serialize capture reply using production codec");
    CPacketReader reader(writer.Get_Buffer()); S2C_CAPTURE_CHARACTER_RESULT decoded;
    Check(Read_Message(reader, decoded) && reader.Get_RemainingSize() == 0, "deserialize complete capture reply");
    Check(Selection::Apply_CaptureResult(decoded, 7), "consume matching capture reply");
    Check(Selection::Get_WorldStateCaptureStatus() == CHARACTER_CAPTURE_STATUS::CAPTURED, "authoritative capture committed");
    Preserved(id, State(8)); Selection::Capture_ActiveWorldState(); Preserved(id, State(8));
    Selection::Finish_WorldStateCapture(); Selection::Capture_ActiveWorldState(); Preserved(id, State(8));
    Check(!Selection::Is_WorldStateSyncPending(), "finished barrier releases pending status");
}
void StaleCapture() {
    const auto id = Create(4); Save(id, State(1)); const auto seq = Begin();
    Check(!Selection::Apply_CaptureResult(Capture(seq + 1, State(8)), 7), "wrong capture sequence ignored");
    Check(!Selection::Apply_CaptureResult(Capture(seq, State(8)), 8), "wrong connection/world generation ignored");
    Check(Selection::Get_WorldStateCaptureStatus() == CHARACTER_CAPTURE_STATUS::WAITING, "stale reply does not consume live barrier");
    Preserved(id, State(1)); Check(Selection::Apply_CaptureResult(Capture(seq, State(8)), 7), "matching reply still accepted");
    Preserved(id, State(8));
}
void RejectedCaptureScope() {
    const auto id = Create(4); Save(id, State(1)); SeedHud(State(0));
    for (unsigned bad = 0; bad != 5; ++bad) {
        const auto seq = Begin(); auto result = Capture(seq, State(8));
        if (bad == 0) result.eWorldId = WORLD_ID::VALTAN_ARENA;
        if (bad == 1) ++result.iPlayerId;
        if (bad == 2) ++result.iNetEntityId;
        if (bad == 3) result.eCharacterClass = CHARACTER_CLASS_ID::SLAYER;
        if (bad == 4) result.eResult = CHARACTER_CAPTURE_RESULT::REJECTED;
        Check(Selection::Apply_CaptureResult(result, 7), "same request with rejected scope consumed");
        Check(Selection::Get_WorldStateCaptureStatus() == CHARACTER_CAPTURE_STATUS::FAILED, "invalid scope blocks exit success");
        Selection::Capture_ActiveWorldState(); Preserved(id, State(1)); Selection::Finish_WorldStateCapture();
    }
}
void ReplacedActiveIdentity() {
    const auto id = Create(4); Save(id, State(1)); const auto seq = Begin();
    const auto other = Create(1); Save(other, State(2));
    Check(!Selection::Apply_CaptureResult(Capture(seq, State(8)), 7), "old identity capture cannot enter new character");
    Preserved(id, State(1)); Preserved(other, State(2));
}
void RestoreSuccess() {
    const auto id = Create(4); Save(id, State(8)); SeedHud(State(0)); Reenter(4);
    Check(Selection::Is_RestorePending() && Selection::Is_WorldStateSyncPending(), "existing entry blocks until restore");
    Check(!Selection::Begin_WorldStateCapture(5, WORLD_ID::BERN, 41, 91, 7), "capture prohibited while restore pending");
    Selection::Capture_ActiveWorldState(); Preserved(id, State(8));
    CHARACTER_WORLD_STATE state; Check(Selection::Try_Get_ActiveWorldState(state) && Same(state, State(8)), "restore complete saved fields");
    const auto seq = Selection::Next_StateRequestSequence(); Selection::Mark_RestoreRequested(seq);
    Check(!Selection::Try_Get_ActiveWorldState(state), "restore sends once");
    Check(!Selection::Apply_RestoreResult({seq + 1, CHARACTER_RESTORE_RESULT::APPLIED, 0}), "stale restore acknowledgement ignored");
    Check(Selection::Is_RestorePending(), "stale acknowledgement cannot release protection");
    Check(Selection::Apply_RestoreResult({seq, CHARACTER_RESTORE_RESULT::APPLIED, state.iHonorTitleId}), "matching restore acknowledgement");
    Check(!Selection::Is_RestorePending(), "applied acknowledgement releases protection");
    SeedHud(State(9)); Selection::Capture_ActiveWorldState(); Preserved(id, State(9));
}
void RestoreRejected() {
    const auto id = Create(4); Save(id, State(8)); SeedHud(State(0)); Reenter(4);
    const auto seq = Selection::Next_StateRequestSequence(); Selection::Mark_RestoreRequested(seq);
    Check(Selection::Apply_RestoreResult({seq, CHARACTER_RESTORE_RESULT::REJECTED_CATALOG, 0}), "consume restore rejection");
    Check(Selection::Is_RestorePending(), "rejection leaves old save protected");
    CHARACTER_WORLD_STATE state; Check(!Selection::Try_Get_ActiveWorldState(state), "rejection cannot resend in same admission");
    Selection::Capture_ActiveWorldState(); Preserved(id, State(8));
    Reenter(4); Check(Selection::Try_Get_ActiveWorldState(state) && Same(state, State(8)), "explicit new admission can restore saved data again");
}
void RestoreGuardWorldScope() {
    const auto id = Create(4); Save(id, State(8)); Reenter(4);
    const auto seq = Selection::Next_StateRequestSequence(); Selection::Mark_RestoreRequested(seq);
    Check(Selection::Apply_RestoreResult({seq, CHARACTER_RESTORE_RESULT::REJECTED_CATALOG, 0}), "reject old Bern restore");
    CGameInstance::Get().level = LEVEL::LOBBY;
    Check(Selection::Is_RestorePending() && !Selection::Is_WorldStateSyncPending(), "Lobby allows recovery while rejected save stays protected");
    CGameInstance::Get().level = LEVEL::CHARACTER_SELECT;
    Check(!Selection::Is_WorldStateSyncPending(), "new character arena can send class commands after old restore rejection");
    CGameInstance::Get().level = LEVEL::BERN; Reenter(4);
    Check(Selection::Is_WorldStateSyncPending(), "saved character Bern readmission blocks until its own restore");
    CHARACTER_WORLD_STATE state;
    Check(Selection::Try_Get_ActiveWorldState(state) && Same(state, State(8)), "recovery preserves original character data");
    Preserved(id, State(8));
}
void FallbackGuards() {
    const auto id = Create(4); Save(id, State(8)); SeedHud(State(0)); auto& hud = CCombatHUDViewModel::Get();
    hud.player.isValid = false; Selection::Capture_ActiveWorldState(); Preserved(id, State(8));
    hud.player.isValid = true; hud.player.isPreview = true; Selection::Capture_ActiveWorldState(); Preserved(id, State(8));
    hud.player.isPreview = false; hud.hasInventory = false; Selection::Capture_ActiveWorldState(); Preserved(id, State(8));
    hud.hasInventory = true; CGameInstance::Get().level = LEVEL::LOBBY;
    Selection::Capture_ActiveWorldState(); Preserved(id, State(8));
    CGameInstance::Get().level = LEVEL::BERN; Selection::Capture_ActiveWorldState(); Preserved(id, State(0));
}
int main(int argc, char** argv) {
    if (argc != 2) return 2;
    const std::string name = argv[1];
    try {
        if (name == "empty_identity") EmptyIdentity();
        else if (name == "six_slots_independent") SixSlots();
        else if (name == "creation_commit_rollback") CreationRollback();
        else if (name == "existing_identity_validation") ExistingValidation();
        else if (name == "capture_success_final_latch") CaptureSuccess();
        else if (name == "capture_stale_sequence_generation") StaleCapture();
        else if (name == "capture_scope_rejection") RejectedCaptureScope();
        else if (name == "capture_active_identity_replacement") ReplacedActiveIdentity();
        else if (name == "restore_success") RestoreSuccess();
        else if (name == "restore_rejection_latch") RestoreRejected();
        else if (name == "restore_guard_world_scope") RestoreGuardWorldScope();
        else if (name == "fallback_capture_guards") FallbackGuards();
        else throw std::runtime_error("unknown test case");
        std::cout << "PASS " << name << '\n'; return 0;
    } catch (const std::exception& error) {
        std::cout << "FAIL " << name << ": " << error.what() << '\n'; return 1;
    }
}
'''

CASES = (
    "empty_identity", "six_slots_independent", "creation_commit_rollback",
    "existing_identity_validation", "capture_success_final_latch",
    "capture_stale_sequence_generation", "capture_scope_rejection",
    "capture_active_identity_replacement", "restore_success",
    "restore_rejection_latch", "fallback_capture_guards",
    "restore_guard_world_scope",
)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--configuration", choices=("Debug", "Release"), default="Debug")
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    for name, source in STUBS.items():
        (output / name).write_text(source, encoding="utf-8")
    cpp = output / "character_session_state.cpp"
    cpp.write_text(FIXTURE, encoding="utf-8")
    debug = args.configuration == "Debug"
    options = ["cl.exe", "/nologo", "/std:c++20", "/EHsc", "/utf-8", "/DNOMINMAX",
               "/MDd" if debug else "/MD", "/Od" if debug else "/O2", "/Zi", "/FS",
               "/I" + str(output), "/I" + str(ROOT / "Client/Public"),
               "/I" + str(ROOT / "Shared/Public"), "/Fo" + str(output) + "/",
               "/Fd" + str(output / "compile.pdb")]
    sources = [str(cpp), str(ROOT / "Client/Private/CharacterRoster.cpp"),
               str(ROOT / "Client/Private/CharacterSelectionState.cpp")]
    sources += [str(ROOT / "Shared/Private" / path) for path in (
        "GameplayDataRevision.cpp", "Network/PacketMessages.cpp", "Network/PacketReader.cpp",
        "Network/PacketWriter.cpp")]
    exe = output / "character_session_state.exe"
    built = subprocess.run(options + sources + ["/Fe" + str(exe), "/link", "/INCREMENTAL:NO"],
                           cwd=output, text=True, encoding="utf-8", errors="replace", capture_output=True)
    (output / "compile.log").write_text(built.stdout + built.stderr, encoding="utf-8")
    if built.returncode:
        print(built.stdout + built.stderr)
        return built.returncode
    results = []
    for case in CASES:
        run = subprocess.run([str(exe), case], cwd=output, text=True, capture_output=True)
        print(run.stdout.strip())
        results.append({"case": case, "exitCode": run.returncode,
                        "stdout": run.stdout, "stderr": run.stderr})
    (output / "results.json").write_text(json.dumps({
        "configuration": args.configuration, "passed": sum(r["exitCode"] == 0 for r in results),
        "total": len(results), "cases": results,
        "scope": "Full production roster/selection cpp and Shared codecs; graphics/HUD inputs stubbed; no Client or Server"},
        ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return int(any(result["exitCode"] != 0 for result in results))


if __name__ == "__main__":
    raise SystemExit(main())

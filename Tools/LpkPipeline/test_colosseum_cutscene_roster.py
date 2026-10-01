#!/usr/bin/env python3
"""Run the production Colosseum actor reconciliation methods without Client/UI.

Build_Actors, Clear_Actors, Has_PresentWinner and both ACTOR declarations are
extracted unchanged from this checkout and compiled as C++20. Only Character,
replication, display-name conversion and document storage are test doubles.
The fixture supplies observations and independent roster expectations; it does
not reimplement actor selection. This is not a GPU, animation or network test.

Run: python -B Tools/LpkPipeline/test_colosseum_cutscene_roster.py
MSVC is located through the developer environment or vswhere (including preview
installations). --out retains generated source, compiler output and a receipt.
"""
from pathlib import Path
import argparse
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def extract(text, signature, source_path, declarations=False):
    """Balance braces after masking comments/literals; never rewrite the body."""
    if text.count(signature) != 1:
        raise ValueError(f"Expected one {signature!r} in {source_path}")
    start = text.index(signature)
    masked = re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
                    lambda match: " " * len(match[0]), text)
    opening = masked.index("{", start)
    depth = 0
    for end in range(opening, len(masked)):
        depth += (masked[end] == "{") - (masked[end] == "}")
        if depth == 0:
            end += 1
            if declarations:
                if masked[end:end + 1] != ";":
                    raise ValueError(f"Missing declaration terminator in {source_path}")
                end += 1
            body = text[start:end]
            line = text.count("\n", 0, start) + 1
            return (f'#line {line} "{source_path}"\n{body}\n'
                    '#line 1 "colosseum_roster_fixture"\n',
                    {"file": source_path, "signature": signature, "line": line,
                     "sha256": hashlib.sha256(body.encode("utf-8")).hexdigest()})
    raise ValueError(f"Unclosed body in {source_path}: {signature}")


def msvc_environment(out):
    env = {key.upper(): value for key, value in os.environ.items()}
    compiler = shutil.which("cl.exe")
    if compiler:
        return Path(compiler), env
    vswhere = Path(os.environ.get("ProgramFiles(x86)", "C:/Program Files (x86)")) / "Microsoft Visual Studio/Installer/vswhere.exe"
    if not vswhere.is_file():
        raise RuntimeError("MSVC is required; use an x64 C++ developer shell")
    installation = subprocess.run(
        [str(vswhere), "-latest", "-prerelease", "-products", "*", "-requires",
         "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"],
        capture_output=True, check=True, text=True, timeout=20).stdout.strip()
    if not installation:
        raise RuntimeError("No installed MSVC x64 toolchain found")
    script = out / "environment.cmd"
    script.write_text('@echo off\ncall "' + str(Path(installation) / "Common7/Tools/VsDevCmd.bat") +
                      '" -arch=x64 -host_arch=x64 >nul\nif errorlevel 1 exit /b 1\nset\n', encoding="utf-8")
    setup = subprocess.run(["cmd.exe", "/d", "/c", str(script)], capture_output=True, check=True, timeout=30)
    for line in setup.stdout.decode(errors="replace").splitlines():
        if "=" in line and not line.startswith("="):
            key, value = line.split("=", 1)
            env[key.upper()] = value
    compiler = shutil.which("cl.exe", path=env.get("PATH", ""))
    if not compiler:
        raise RuntimeError("MSVC environment did not expose cl.exe")
    return Path(compiler), env


PREAMBLE = r'''
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
using f32_t = float;
struct float3_t {
    float x = 0, y = 0, z = 0;
    float3_t() = default;
    float3_t(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}
};
namespace LostArk::Shared {
using NET_ENTITY_ID = uint32_t;
constexpr NET_ENTITY_ID INVALID_NET_ENTITY_ID = 0;
// @COMBAT_LIMIT@
enum class PLAYER_CONTROL_KIND { HUMAN, COLOSSEUM_MERCENARY_AI };
struct COLOSSEUM_MATCH_PLAYER_STATE {
    uint32_t iPlayerId = 0, iNetEntityId = 0;
    uint8_t iTeam = 0, iArrivalIndex = 0;
    bool bParticipant = true;
};
struct S2C_COLOSSEUM_MATCH_STATE {
    uint64_t iMatchId = 1;
    uint8_t iWinningTeam = 0;
    std::vector<COLOSSEUM_MATCH_PLAYER_STATE> Participants;
};
}
using namespace LostArk::Shared;

// Test doubles record presentation mutations; they contain no selection logic.
struct CCharacter {
    bool transformReady = true, ship = false, pose = false, animation = false, suppressed = false;
    int clears = 0, animationClears = 0;
    CCharacter* Get_Transform() { return transformReady ? this : nullptr; }
    bool Is_ShipPresentation() const { return ship; }
    bool Is_CinematicPresentationSuppressed() const { return suppressed; }
    void Set_CinematicPresentationSuppressed(bool value) { suppressed = value; }
    void Clear_CutscenePoseOverride() { pose = false; ++clears; }
    void Clear_CutsceneAnimation() { animation = false; ++animationClears; }
};
struct REPLICATED_PLAYER_VIEW {
    uint32_t iPlayerId = 0, iNetEntityId = 0;
    int eCharacterClass = 0;
    PLAYER_CONTROL_KIND eControlKind = PLAYER_CONTROL_KIND::HUMAN;
    std::string strNickname;
    std::weak_ptr<CCharacter> pCharacter;
    bool isLocal = false;
};
struct CClientReplication {
    S2C_COLOSSEUM_MATCH_STATE match;
    std::vector<REPLICATED_PLAYER_VIEW> players;
    void Collect_PlayerViews(std::vector<REPLICATED_PLAYER_VIEW>& out) const { out = players; }
    const auto& Get_ColosseumMatchState() const { return match; }
};
struct CWorldPlayerNameplateView {
    static bool Try_ConvertUtf8(const std::string& in, std::wstring& out) {
        out.assign(in.begin(), in.end()); return true; // ASCII fixture names only.
    }
};
struct Intro {
// @INTRO_SLOT@
// @INTRO_ACTOR@
    struct DOCUMENT { float fFloorY = 0; std::vector<LINEUP_SLOT> Teams[2]; } m_Doc;
    std::vector<ACTOR> m_Actors;
    static std::wstring Job_Name(int) { return L"fixture class"; }
// @INTRO_BUILD@
// @INTRO_CLEAR@
};
struct Victory {
// @VICTORY_ACTOR@
    struct DOCUMENT { std::vector<int> actors; } document;
    std::vector<ACTOR> actors;
    S2C_COLOSSEUM_MATCH_STATE state;
    bool showingActors = false;
// @VICTORY_BUILD@
// @VICTORY_CLEAR@
// @VICTORY_PRESENT@
};
'''


FIXTURE = r'''
int checks = 0, cases = 0;
void Check(bool ok, const std::string& description) {
    ++checks;
    if (!ok) throw std::runtime_error(description);
}
void Pass(const char* name) { ++cases; std::cout << "PASS " << name << '\n'; }
bool Near(float a, float b) { return std::abs(a - b) < .00001f; }
Intro NewIntro() {
    Intro intro;
// @DOCUMENT_SETUP@
    return intro;
}
Victory NewVictory() { Victory view; view.document.actors.resize(4); return view; }
struct Fixture {
    CClientReplication replication;
    std::array<std::shared_ptr<CCharacter>, 8> bodies;
    explicit Fixture(unsigned count = 8) {
        for (unsigned arrival = 0; arrival < count; ++arrival) {
            bodies[arrival] = std::make_shared<CCharacter>();
            COLOSSEUM_MATCH_PLAYER_STATE row;
            row.iPlayerId = 100 + arrival; row.iNetEntityId = 1000 + arrival;
            row.iTeam = static_cast<uint8_t>(arrival % 2);
            row.iArrivalIndex = static_cast<uint8_t>(arrival);
            replication.match.Participants.push_back(row);
            REPLICATED_PLAYER_VIEW player;
            player.iPlayerId = row.iPlayerId; player.iNetEntityId = row.iNetEntityId;
            player.pCharacter = bodies[arrival]; player.strNickname = "player" + std::to_string(arrival);
            player.eControlKind = arrival < 4 ? PLAYER_CONTROL_KIND::HUMAN : PLAYER_CONTROL_KIND::COLOSSEUM_MERCENARY_AI;
            player.isLocal = arrival == 0;
            replication.players.push_back(player);
        }
        // Arrival order, packet order and entity enumeration order are independent.
        std::reverse(replication.players.begin(), replication.players.end());
        if (count > 1) std::rotate(replication.match.Participants.begin(),
            replication.match.Participants.begin() + 1, replication.match.Participants.end());
    }
    REPLICATED_PLAYER_VIEW& Player(unsigned arrival) {
        const auto it = std::find_if(replication.players.begin(), replication.players.end(), [&](const auto& p) {
            return p.iNetEntityId == 1000 + arrival;
        });
        if (it == replication.players.end()) throw std::runtime_error("fixture player missing");
        return *it;
    }
};
void Reconcile(Intro& intro, const Fixture& fixture) {
    // Same call order as Intro's live frame consumer; both method bodies are production.
    intro.Clear_Actors(); intro.Build_Actors(fixture.replication);
}
void Reconcile(Victory& view, const Fixture& fixture) {
    view.state = fixture.replication.match; view.Build_Actors(fixture.replication);
}
const Intro::ACTOR* IntroActor(const Intro& intro, const std::shared_ptr<CCharacter>& body) {
    const auto found = std::find_if(intro.m_Actors.begin(), intro.m_Actors.end(), [&](const auto& actor) {
        return actor.pCharacter.lock() == body;
    });
    return found == intro.m_Actors.end() ? nullptr : &*found;
}
const Victory::ACTOR* VictoryActor(const Victory& view, unsigned arrival) {
    const auto found = std::find_if(view.actors.begin(), view.actors.end(), [&](const auto& actor) {
        return actor.id == 1000 + arrival;
    });
    return found == view.actors.end() ? nullptr : &*found;
}
void CheckIntroSlot(const Intro& intro, const Fixture& fixture, unsigned arrival) {
    const auto* actor = IntroActor(intro, fixture.bodies[arrival]);
    Check(actor != nullptr, "Intro exact body missing for arrival " + std::to_string(arrival));
    const auto& slot = intro.m_Doc.Teams[arrival % 2][arrival / 2];
    Check(actor->iRow == arrival / 2 && Near(actor->vPosition.x, slot.fX) &&
        Near(actor->vPosition.y, intro.m_Doc.fFloorY) && Near(actor->vPosition.z, slot.fZ) &&
        Near(actor->fYawDegrees, slot.fYawDegrees), "Intro stable authored slot mismatch");
}
void CheckVictorySlot(const Victory& view, unsigned arrival, unsigned winner) {
    const auto* actor = VictoryActor(view, arrival);
    Check(actor != nullptr, "Victory exact entity missing");
    Check(actor->winner == (arrival % 2 == winner), "Victory team mismatch");
    if (actor->winner) Check(actor->slot == arrival / 2, "Victory stable slot mismatch");
}
void Cardinalities() {
    for (unsigned count = 0; count <= 8; ++count) {
        Fixture fixture(count); auto intro = NewIntro(); Reconcile(intro, fixture);
        Check(intro.m_Actors.size() == count, "Intro 0..8 actor count");
        for (unsigned arrival = 0; arrival < count; ++arrival) CheckIntroSlot(intro, fixture, arrival);
        for (unsigned winner : {0u, 1u}) {
            fixture.replication.match.iWinningTeam = static_cast<uint8_t>(winner);
            auto victory = NewVictory(); Reconcile(victory, fixture);
            Check(victory.actors.size() == count, "Victory 0..8 actor count");
            std::set<size_t> winnerSlots;
            for (unsigned arrival = 0; arrival < count; ++arrival) {
                CheckVictorySlot(victory, arrival, winner);
                if (arrival % 2 == winner) winnerSlots.insert(VictoryActor(victory, arrival)->slot);
            }
            Check(victory.Has_PresentWinner() == !winnerSlots.empty(), "Present winner vs actual roster");
            if (count == 8) Check(winnerSlots == std::set<size_t>({0, 1, 2, 3}), "All four winner slots");
        }
    }
    Pass("0..8 participants, shuffled order, both winning teams, humans and mercenaries, slots 0..3");
}
void SelectionGuards() {
    Fixture fixture; auto candidate = std::make_shared<CCharacter>();
    auto copy = fixture.Player(0); copy.iPlayerId = 900; copy.iNetEntityId = 9000;
    copy.pCharacter = candidate; copy.isLocal = false; fixture.replication.players.push_back(copy);
    auto intro = NewIntro(); auto victory = NewVictory(); Reconcile(intro, fixture); Reconcile(victory, fixture);
    Check(intro.m_Actors.size() == 8 && victory.actors.size() == 8, "Unselected candidate leaked into ceremony");
    // Defensive row filtering also rejects a nonparticipant accidentally present in the list.
    COLOSSEUM_MATCH_PLAYER_STATE row; row.iPlayerId = 900; row.iNetEntityId = 9000; row.bParticipant = false;
    fixture.replication.match.Participants.push_back(row);
    Reconcile(intro, fixture); Reconcile(victory, fixture);
    Check(intro.m_Actors.size() == 8 && victory.actors.size() == 8, "False participant accepted");
    fixture.Player(0).iPlayerId = 777; // Entity alone must not join.
    fixture.Player(1).iNetEntityId = 8888; // Player alone must not join.
    Reconcile(intro, fixture); Reconcile(victory, fixture);
    Check(intro.m_Actors.size() == 6 && victory.actors.size() == 6, "Partial identity matched");
    Check(!IntroActor(intro, fixture.bodies[0]) && !IntroActor(intro, fixture.bodies[1]) &&
        !VictoryActor(victory, 0) && !VictoryActor(victory, 1), "Wrong exact identity selected");
    for (unsigned invalid : {8u, 255u}) {
        Fixture bad(1); bad.replication.match.Participants[0].iArrivalIndex = static_cast<uint8_t>(invalid);
        Reconcile(intro, bad); Reconcile(victory, bad);
        Check(intro.m_Actors.empty() && victory.actors.empty(), "Out-of-range arrival accepted");
    }
    Fixture badTeam(1); badTeam.replication.match.Participants[0].iTeam = 2;
    Reconcile(intro, badTeam); Reconcile(victory, badTeam);
    Check(intro.m_Actors.empty() && victory.actors.empty(), "Invalid team accepted");
    Pass("unselected/false participants excluded; exact PlayerId+NetEntityId; arrival/team bounds");
}
void Reconciliation() {
    Fixture fixture; auto intro = NewIntro(); auto victory = NewVictory();
    auto& late = fixture.Player(6); late.pCharacter.reset();
    Reconcile(intro, fixture); Reconcile(victory, fixture);
    Check(intro.m_Actors.size() == 7 && victory.actors.size() == 7, "Absent body count");
    late.pCharacter = fixture.bodies[6];
    Reconcile(intro, fixture); Reconcile(victory, fixture);
    Check(intro.m_Actors.size() == 8 && victory.actors.size() == 8, "Late body was frozen out");
    CheckIntroSlot(intro, fixture, 6); CheckVictorySlot(victory, 6, 0);
    // Repeated unchanged observations must not reset an ongoing victory animation.
    fixture.bodies[6]->pose = true; fixture.bodies[6]->animation = true;
    const int clearCount = fixture.bodies[6]->clears;
    Reconcile(victory, fixture);
    Check(fixture.bodies[6]->pose && fixture.bodies[6]->animation && fixture.bodies[6]->clears == clearCount,
        "Stable victory roster unnecessarily rebuilt");
    // New character instance with the same network identity must replace the old weak pointer.
    auto oldBody = fixture.bodies[6];
    fixture.bodies[6] = std::make_shared<CCharacter>(); late.pCharacter = fixture.bodies[6];
    Reconcile(intro, fixture); Reconcile(victory, fixture);
    Check(!oldBody->pose && !oldBody->animation, "Replaced body presentation not restored");
    Check(IntroActor(intro, fixture.bodies[6]) && VictoryActor(victory, 6)->character.lock() == fixture.bodies[6],
        "Same identity body replacement missed");
    // Server roster departure alone is authoritative even if the old view is still present.
    fixture.bodies[0]->pose = true; fixture.bodies[0]->animation = true;
    std::erase_if(fixture.replication.match.Participants, [](const auto& p) { return p.iArrivalIndex == 0; });
    Reconcile(intro, fixture); Reconcile(victory, fixture);
    Check(intro.m_Actors.size() == 7 && victory.actors.size() == 7, "Departed actor remained");
    Check(!fixture.bodies[0]->pose && !fixture.bodies[0]->animation, "Departed body not restored");
    for (unsigned arrival = 1; arrival < 8; ++arrival) {
        CheckIntroSlot(intro, fixture, arrival); CheckVictorySlot(victory, arrival, 0);
    }
    fixture.Player(4).pCharacter.reset(); fixture.bodies[4].reset();
    Reconcile(intro, fixture); Reconcile(victory, fixture);
    Check(intro.m_Actors.size() == 6 && victory.actors.size() == 6, "Expired body remained");
    fixture.replication.match.Participants.clear();
    Reconcile(intro, fixture); Reconcile(victory, fixture);
    Check(intro.m_Actors.empty() && victory.actors.empty() && !victory.Has_PresentWinner(), "Empty roster not cleared");
    Pass("late bodies, unchanged frames, replaced bodies, departures, expired weak pointers; stable gaps");
}
void CleanupAndReadiness() {
    Fixture fixture;
    for (unsigned i = 0; i < 8; ++i) fixture.bodies[i]->suppressed = i % 2 != 0;
    auto intro = NewIntro(); auto victory = NewVictory(); Reconcile(intro, fixture); Reconcile(victory, fixture);
    for (const auto& body : fixture.bodies) { body->pose = true; body->animation = true; body->suppressed = !body->suppressed; }
    intro.Clear_Actors();
    for (const auto& body : fixture.bodies) Check(!body->pose && body->animation, "Intro must clear only its owned pose override");
    for (const auto& body : fixture.bodies) body->pose = true;
    victory.Clear_Actors();
    Check(victory.actors.empty() && !victory.showingActors, "Victory cleanup state");
    for (unsigned i = 0; i < 8; ++i) Check(!fixture.bodies[i]->pose && !fixture.bodies[i]->animation &&
        fixture.bodies[i]->suppressed == (i % 2 != 0), "Victory pose/animation/prior visibility not restored");
    auto empty = NewVictory(); Check(!empty.Has_PresentWinner(), "Empty winners should not start camera");
    Fixture one(1); one.replication.match.iWinningTeam = 1;
    Reconcile(empty, one); Check(!empty.Has_PresentWinner(), "Losers-only roster should not start camera");
    one.replication.match.iWinningTeam = 0; empty.Clear_Actors(); Reconcile(empty, one);
    Check(empty.Has_PresentWinner(), "Valid winner not ready");
    one.bodies[0]->transformReady = false;
    Check(!empty.Has_PresentWinner(), "Winner without transform reported ready");
    one.bodies[0]->transformReady = true;
    Check(empty.Has_PresentWinner(), "Late transform was frozen out");
    empty.actors[0].slot = empty.document.actors.size();
    Check(!empty.Has_PresentWinner(), "Out-of-range winner slot reported ready");
    empty.actors[0].slot = 0;
    one.bodies[0].reset();
    Check(!empty.Has_PresentWinner(), "Expired winner reported ready");
    one.bodies[0] = std::make_shared<CCharacter>(); one.Player(0).pCharacter = one.bodies[0];
    Reconcile(empty, one); Check(empty.Has_PresentWinner(), "Late winner cannot recover empty camera guard");
    one.bodies[0]->transformReady = false; Reconcile(intro, one);
    Check(intro.m_Actors.empty(), "Intro accepted null transform");
    one.bodies[0]->transformReady = true; one.bodies[0]->ship = true; Reconcile(intro, one);
    Check(intro.m_Actors.empty(), "Intro accepted ship presentation");
    Pass("clear restores owned state; empty/loser/null-transform/invalid-slot/expired/late winner readiness");
}
void PreviewBoundary() {
    Fixture fixture(2); fixture.replication.match.Participants.clear();
    auto intro = NewIntro(); Reconcile(intro, fixture);
    Check(intro.m_Actors.empty(), "Matched roster must not fall back to local player");
    fixture.replication.match.iMatchId = 0; Reconcile(intro, fixture);
    Check(intro.m_Actors.size() == 1 && IntroActor(intro, fixture.bodies[0]), "Explicit no-match local preview missing");
    intro.m_Doc.Teams[0].clear(); Reconcile(intro, fixture);
    Check(intro.m_Actors.empty(), "Missing document slot must be bounded");
    Pass("explicit unmatched local preview only; missing authored slot bounded");
}
int main() {
    try {
        Cardinalities(); SelectionGuards(); Reconciliation(); CleanupAndReadiness(); PreviewBoundary();
        std::cout << "SUMMARY cases=" << cases << " checks=" << checks << " failures=0\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL after " << checks << " checks: " << error.what() << '\n'; return 1;
    }
}
'''


def run(out):
    out.mkdir(parents=True, exist_ok=True)
    production = {
        "INTRO": "Client/Public/ColosseumIntroCutscene.h",
        "VICTORY": "Client/Private/ColosseumMatchView.cpp",
    }
    extracted, manifest = {}, []
    for prefix, path in production.items():
        text = (ROOT / path).read_text(encoding="utf-8-sig")
        signatures = [("ACTOR", "struct ACTOR\n", True),
                      ("BUILD", "void Build_Actors(const CClientReplication& replication)", False),
                      ("CLEAR", "void Clear_Actors()", False)]
        if prefix == "INTRO":
            signatures.append(("SLOT", "struct LINEUP_SLOT", True))
        else:
            signatures.append(("PRESENT", "bool Has_PresentWinner() const", False))
        for key, signature, declaration in signatures:
            block, evidence = extract(text, signature, path, declaration)
            extracted[f"// @{prefix}_{key}@"] = block
            manifest.append(evidence)
    header = (ROOT / "Shared/Public/Network/PacketMessages.h").read_text(encoding="utf-8-sig")
    limit = re.search(r"inline constexpr std::size_t MAX_COLOSSEUM_COMBAT_PLAYERS\s*=\s*\d+;", header)
    if not limit:
        raise ValueError("Missing production combat participant limit")
    extracted["// @COMBAT_LIMIT@"] = limit[0]
    doc_path = ROOT / "Data/Camera/ColosseumIntro.cutscene.json"
    doc = json.loads(doc_path.read_text(encoding="utf-8-sig"))
    if len(doc["rows"]) != 4 or any([row["row"] for row in doc["teams"][team]] != [0, 1, 2, 3] for team in ("A", "B")):
        raise ValueError("Expected four stable authored intro slots per team")
    setup = [f"intro.m_Doc.fFloorY = {float(doc['floorY'])}f;"]
    for team_index, team in enumerate(("A", "B")):
        for row in doc["teams"][team]:
            setup.append(f"intro.m_Doc.Teams[{team_index}].push_back({{" + ", ".join(
                [f"{float(row[key])}f" for key in ("x", "z", "yawDegrees")] + [f"{row['row']}u"]) + "});")
    extracted["// @DOCUMENT_SETUP@"] = "\n".join(setup)
    code = PREAMBLE + FIXTURE
    for marker, value in extracted.items():
        if code.count(marker) != 1:
            raise ValueError(f"Fixture marker count changed: {marker}")
        code = code.replace(marker, value)
    cpp = out / "colosseum_roster.cpp"
    cpp.write_text(code, encoding="utf-8")
    compiler, env = msvc_environment(out)
    executable = out / "colosseum_roster.exe"
    command = [str(compiler), "/nologo", "/std:c++20", "/EHsc", "/utf-8", "/W4", "/WX",
               "/Od", "/RTC1", "/MDd", "/D_ITERATOR_DEBUG_LEVEL=2", str(cpp), "/Fe:" + str(executable)]
    compiled = subprocess.run(command, cwd=out, env=env, capture_output=True, text=True, errors="replace", timeout=60)
    (out / "compile.log").write_text(compiled.stdout + compiled.stderr, encoding="utf-8")
    if compiled.returncode:
        raise RuntimeError("Native fixture compilation failed:\n" + compiled.stdout + compiled.stderr)
    result = subprocess.run([str(executable)], cwd=out, env=env, capture_output=True, text=True, errors="replace", timeout=20)
    (out / "run.log").write_text(result.stdout + result.stderr, encoding="utf-8")
    print(result.stdout, end="")
    receipt = {"compiler": str(compiler), "command": command, "productionMethods": manifest,
               "introDocumentSha256": hashlib.sha256(doc_path.read_bytes()).hexdigest(),
               "generatedSourceSha256": hashlib.sha256(cpp.read_bytes()).hexdigest(),
               "compileExitCode": compiled.returncode, "runExitCode": result.returncode,
               "scope": "Production roster/cleanup/readiness bodies with test doubles; no Client, GPU or network"}
    summary = re.search(r"SUMMARY cases=(\d+) checks=(\d+) failures=(\d+)", result.stdout)
    if summary:
        receipt.update(zip(("cases", "checks", "failures"), map(int, summary.groups())))
    (out / "receipt.json").write_text(json.dumps(receipt, indent=2) + "\n", encoding="utf-8")
    if result.returncode or not summary:
        raise RuntimeError("Native fixture failed:\n" + result.stdout + result.stderr)
    print("Compiler:", compiler)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", type=Path, help="Keep generated native source, logs and receipt in this directory")
    args = parser.parse_args()
    try:
        if args.out:
            run(args.out.resolve())
            print("Evidence:", args.out.resolve())
        else:
            with tempfile.TemporaryDirectory(prefix="colosseum-roster-") as temporary:
                run(Path(temporary))
    except (OSError, RuntimeError, ValueError, subprocess.SubprocessError) as error:
        print(error, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

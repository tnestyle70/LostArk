"""Small metadata tests; no publisher or live data installation is executed."""
import copy
import json
from pathlib import Path
import subprocess
import tempfile
import unittest

import raid_flow_projection as subject
import prepare_raid_integration as prepare
import project_kouku_saydon_composition as composition


ROOT = Path(__file__).resolve().parents[2]
PREFIX = "KAKULSAYDON_G1_PATTERN_"


def documents():
    action = {"patterns": [], "bundles": [], "patternFlows": []}
    sequence = {"compositionId": "sequence.test", "revision": 7, "patterns": [],
                "logics": [{"logicId": "arrival", "triggerKind": "ROOM_PLAYER_ARRIVAL"}]}
    for gate, number in (("GATE1", 4), ("GATE2", 3), ("GATE3", 5)):
        pattern_id = f"pattern.{gate}"
        action["patterns"].append({"patternId": pattern_id, "gateId": gate, "authoringStatus": "PRODUCT"})
        action["patternFlows"].append({"gateId": gate, "flowId": f"flow.{gate}", "entries": [
            {"entryId": f"entry.{gate}", "kind": "PATTERN", "targetId": pattern_id, "waitAfterMs": 19}]})
        sequence["patterns"].append({"patternId": PREFIX + str(number), "gateId": "GATE2" if gate == "GATE3" else gate,
            "enterCombatOnFinish": True, "durationMs": 0, "stages": [{"durationMs": 1000}],
            "logicOccurrences": [{"occurrenceId": f"arrival.{gate}.{slot}", "logicId": "arrival",
                "startMs": 999, "roomPlayerArrival": {"playerSlot": slot, "position": [slot, 1, 2]}}
                for slot in range(4)]})
    return action, sequence


class RaidProjectionTests(unittest.TestCase):
    def test_authoring_validator_rejects_a_loop_boundary_outside_its_flow(self):
        action, _ = documents()
        for flow in action["patternFlows"]:
            flow["displayName"] = flow["gateId"]
        flow = action["patternFlows"][1]
        flow["loopStartEntryId"] = flow["entries"][0]["entryId"]
        composition.validate_pattern_flows(action)
        for value in (None, 2, "entry.GATE1", "not present", "missing.entry"):
            with self.subTest(value=value):
                flow["loopStartEntryId"] = value
                with self.assertRaises(composition.CompositionError):
                    composition.validate_pattern_flows(action)

    def test_saved_loop_boundary_uses_entry_identity_and_survives_reordering(self):
        action, sequence = documents()
        flow = action["patternFlows"][1]
        second = dict(flow["entries"][0], entryId="entry.gate2.repeat")
        flow["entries"].append(second)
        flow["loopStartEntryId"] = second["entryId"]
        before = copy.deepcopy(action)
        projected = subject.project_raid_gates(action, sequence)[1]
        self.assertEqual(second["entryId"], projected["loopStartEntryId"])
        self.assertEqual(before, action)
        flow["entries"].reverse()
        self.assertEqual(second["entryId"], subject.project_raid_gates(action, sequence)[1]["loopStartEntryId"])
        flow["entries"].remove(second)
        with self.assertRaisesRegex(ValueError, "loop start"):
            subject.project_raid_gates(action, sequence)

    def test_absent_loop_boundary_preserves_legacy_and_invalid_types_are_rejected(self):
        action, sequence = documents()
        self.assertNotIn("loopStartEntryId", subject.project_raid_gates(action, sequence)[1])
        for value in (None, 1, ["entry.GATE2"], "missing.entry"):
            with self.subTest(value=value):
                action["patternFlows"][1]["loopStartEntryId"] = value
                with self.assertRaisesRegex(ValueError, "loop start"):
                    subject.project_raid_gates(action, sequence)

    def test_bingo_reuses_saved_encore_intro_then_combat_and_ending(self):
        action, sequence = documents()
        sequence["patterns"].append({"patternId": PREFIX + "9", "gateId": "BINGO",
            "durationMs": 51285, "stages": [{"durationMs": 49083}], "logicOccurrences": []})
        sequence["patterns"].append({"patternId": PREFIX + "10", "gateId": "BINGO",
            "enterCombatOnFinish": True, "stages": [{"durationMs": 23333}], "logicOccurrences": []})
        action["patterns"].append({"patternId": "parent.bingo", "gateId": "BINGO", "authoringStatus": "PRODUCT"})
        flow = {"gateId": "BINGO", "flowId": "flow.bingo", "entries": [
            {"entryId": "bingo.parent", "kind": "PATTERN", "targetId": "parent.bingo", "waitAfterMs": 0}]}
        action["patternFlows"].append(flow)
        bingo = subject.project_raid_gates(action, sequence)[-1]
        self.assertEqual("BINGO", bingo["gateId"])
        self.assertEqual("boss.kakulsaydon.bingo.saydon", bingo["primaryBossPlacementId"])
        self.assertEqual((PREFIX + "10", 23333, []), (bingo["introPatternId"], bingo["introDurationMs"], bingo["arrivals"]))
        self.assertEqual(flow["entries"], bingo["entries"])
        self.assertEqual((PREFIX + "9", 51285), (bingo["clearPatternId"], bingo["clearDurationMs"]))
        self.assertEqual((sequence["compositionId"], sequence["revision"]),
                         (bingo["sequenceCompositionId"], bingo["sequenceRevision"]))
        action["patterns"][-1]["authoringStatus"] = "DRAFT"
        with self.assertRaisesRegex(ValueError, "unavailable parent.bingo"):
            subject.project_raid_gates(action, sequence)

    def test_release_entry_uses_the_authored_time_zero_world_identity(self):
        action, sequence = documents()
        sequence["worlds"] = [{"worldId": "world.entry", "sequenceInstanceId": "sequence.entry"},
                              {"worldId": "world.later", "sequenceInstanceId": "sequence.later"}]
        sequence["patterns"][0]["worldOccurrences"] = [
            {"worldId": "world.entry", "startMs": 0}, {"worldId": "world.later", "startMs": 1}]
        gates = subject.project_raid_gates(action, sequence)
        self.assertEqual(["sequence.entry", "", ""], [g["entrySequenceInstanceId"] for g in gates])
        sequence["patterns"][0]["worldOccurrences"][1]["startMs"] = 0
        with self.assertRaisesRegex(ValueError, "unambiguous"):
            subject.project_raid_gates(action, sequence)

    def test_zero_duration_uses_stage_sum_and_saved_entries_are_unchanged(self):
        action, sequence = documents()
        before = copy.deepcopy((action, sequence))
        gates = subject.project_raid_gates(action, sequence)
        self.assertEqual([4, 3, 5], [int(g["introPatternId"].removeprefix(PREFIX)) for g in gates])
        self.assertEqual([1000] * 3, [g["introDurationMs"] for g in gates])
        self.assertEqual("", gates[1]["clearPatternId"])
        self.assertEqual(0, gates[1]["clearDurationMs"])
        self.assertEqual([f["entries"] for f in action["patternFlows"]], [g["entries"] for g in gates])
        self.assertEqual(before, (action, sequence))

    def test_unfinished_arrival_or_absent_flow_does_not_advertise_ready_gate(self):
        for missing in ("arrival", "flow"):
            with self.subTest(missing=missing):
                action, sequence = documents()
                if missing == "arrival": sequence["patterns"][1]["logicOccurrences"].pop()
                else: action["patternFlows"].pop(1)
                self.assertEqual(["GATE1", "GATE3"], [g["gateId"] for g in subject.project_raid_gates(action, sequence)])

    def test_gate2_complete_movie_owns_gate3_handoff_without_changing_source_gate(self):
        action, sequence = documents()
        gates = subject.project_raid_gates(action, sequence)
        self.assertEqual("", gates[1]["clearPatternId"])
        self.assertEqual(PREFIX + "5", gates[2]["introPatternId"])
        sequence["patterns"][2]["gateId"] = "GATE3"
        with self.assertRaisesRegex(ValueError, "authored combat handoff"):
            subject.project_raid_gates(action, sequence)

    def test_duplicate_arrival_slot_is_rejected(self):
        action, sequence = documents()
        sequence["patterns"][0]["logicOccurrences"][1]["roomPlayerArrival"]["playerSlot"] = 0
        with self.assertRaisesRegex(ValueError, "duplicate"): subject.project_raid_gates(action, sequence)

    def test_prepare_preserves_old_ids_when_new_entry_ordinal_collides(self):
        original = [json.loads((ROOT / path).read_text("utf-8-sig"))
                    for path in (prepare.ACTION, prepare.SEQUENCE, prepare.WORLD)]
        action, sequence, world = prepare.prepare(*original)
        flow = next(f for f in action["patternFlows"] if f["gateId"] == "GATE3")
        flow["entries"] = [e for e in flow["entries"] if e["targetId"] != PREFIX + "52"]
        old = flow["entries"][0]
        old["entryId"] = flow["flowId"] + ".raid.9"
        old["waitAfterMs"] = 1234
        result, _, _ = prepare.prepare(action, sequence, world)
        entries = next(f["entries"] for f in result["patternFlows"] if f["gateId"] == "GATE3")
        self.assertEqual(len(entries), len({e["entryId"] for e in entries}))
        self.assertEqual(old, next(e for e in entries if e["targetId"] == old["targetId"]))

    def test_powershell_publisher_matches_native_arrival_contract_and_dense_order(self):
        action, sequence = documents()
        gates = subject.project_raid_gates(action, sequence)
        valid = {"encounterId": "encounter.test", "raidGates": gates,
                 "patterns": action["patterns"], "bundles": []}
        # Include index 10 so the actual publisher sort's numeric padding is covered.
        first = valid["raidGates"][0]
        first["entries"] = [dict(first["entries"][0], entryId=f"entry.{i}") for i in range(12)]
        cases = [{"name": "valid", "accepted": True, "document": valid}]
        repeating = copy.deepcopy(valid)
        repeating["raidGates"][1]["loopStartEntryId"] = repeating["raidGates"][1]["entries"][0]["entryId"]
        cases.append({"name": "repeat_boundary", "accepted": True, "document": repeating})
        for boundary in ("missing.entry", 3, None):
            invalid = copy.deepcopy(repeating)
            invalid["raidGates"][1]["loopStartEntryId"] = boundary
            cases.append({"name": "invalid_repeat_boundary", "accepted": False, "document": invalid})
        for name in ("missing_slot", "duplicate_id", "orphan_clear"):
            doc = copy.deepcopy(valid)
            arrivals = doc["raidGates"][0]["arrivals"]
            if name == "missing_slot": arrivals.pop()
            elif name == "duplicate_id": arrivals[1]["occurrenceId"] = arrivals[0]["occurrenceId"]
            else: arrivals.append(dict(arrivals[0], phase="CLEAR", startMs=0, occurrenceId="orphan"))
            cases.append({"name": name, "accepted": False, "document": doc})
        output = ROOT / "out/KoukuRaidMetadataTests"
        output.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(dir=output) as temporary:
            fixture = Path(temporary) / "cases.json"
            fixture.write_text(json.dumps(cases), encoding="utf-8")
            script = Path(temporary) / "probe.ps1"
            script.write_text(r'''
param([string]$Root, [string]$Cases)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$stableIdPattern = '^[A-Za-z0-9_.-]{1,128}$'
$tokens = $null; $errors = $null
$ast = [Management.Automation.Language.Parser]::ParseFile((Join-Path $Root 'Tools/GameplayPipeline/Publish-GameplayBalance.ps1'), [ref]$tokens, [ref]$errors)
$names = @('Assert-ExactProperties','Assert-StableId','Assert-JsonInteger','Assert-JsonNumber','Get-BootstrapRowSortKey')
foreach ($node in $ast.FindAll({param($n) $n -is [Management.Automation.Language.FunctionDefinitionAst]}, $true)) {
    if ($node.Name -cin $names) { Invoke-Expression $node.Extent.Text }
}
. (Join-Path $Root 'Tools/KoukuSaydonPipeline/Publish-KoukuRaidRows.ps1')
foreach ($case in (Get-Content -LiteralPath $Cases -Raw | ConvertFrom-Json)) {
    $rows = [Collections.Generic.List[string]]::new()
    $accepted = $true
    try { Add-KoukuRaidRows $case.document $rows } catch { $accepted = $false }
    if ($accepted -ne $case.accepted) { throw "Unexpected acceptance: $($case.name)" }
    if ($accepted) {
        $sorted = @($rows | Sort-Object { Get-BootstrapRowSortKey $_ })
        foreach ($gate in $case.document.raidGates) {
            $gateRows = @($sorted | Where-Object { $f=$_.Split("`t"); ($f[0] -ceq 'RAIDGATE' -and $f[2] -ceq $gate.gateId) -or ($f[0] -cne 'RAIDGATE' -and $f[1] -ceq $gate.gateId) })
            if (-not $gateRows[0].StartsWith("RAIDGATE`t")) { throw 'Gate did not precede child rows' }
            if ($null -ne $gate.PSObject.Properties['loopStartEntryId'] -and $gate.loopStartEntryId) {
                if ($gateRows[0].Split("`t").Count -ne 14 -or $gateRows[0].Split("`t")[13] -cne $gate.loopStartEntryId) {
                    throw 'Loop boundary was not serialized as the authored entry identity'
                }
            }
            $index = 0
            foreach ($row in $gateRows) { if ($row.StartsWith("RAIDFLOWSTEP`t")) { if ([int]$row.Split("`t")[2] -ne $index) { throw 'Flow order was not dense' }; ++$index } }
        }
    }
}
Write-Output 'PASS metadata acceptance and row order'
''', encoding="utf-8")
            completed = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass",
                "-File", str(script), "-Root", str(ROOT), "-Cases", str(fixture)], capture_output=True, text=True)
            self.assertEqual(0, completed.returncode, completed.stdout + completed.stderr)


if __name__ == "__main__":
    unittest.main()

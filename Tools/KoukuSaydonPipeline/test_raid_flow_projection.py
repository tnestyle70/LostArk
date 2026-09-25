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


def bingo_documents():
    action, sequence = documents()
    for number, duration, combat in ((9, 51285, False), (10, 23333, True)):
        sequence["patterns"].append({"patternId": PREFIX + str(number), "gateId": "BINGO",
            "enterCombatOnFinish": combat, "durationMs": duration, "stages": [{"durationMs": duration}], "logicOccurrences": []})
    common = {"gateId": "BINGO", "authoringStatus": "PRODUCT", "actorProfileId": "MN_RPCT_07",
              "targetBossPlacementId": "boss.kakulsaydon.bingo.saydon"}
    action["patterns"].extend([dict(common, patternId="parent.bingo"),
        dict(common, patternId="special.bingo", patternOccurrences=[{"patternId": "child.bingo", "repeat": False}]),
        dict(common, patternId="child.bingo", logicOccurrences=[{"logicId": "detonate", "enabled": True}])])
    action["logics"] = [{"logicId": "detonate", "logicType": "TRIGGER", "triggerKind": "BINGO_DETONATION"}]
    action["patternFlows"].append({"gateId": "BINGO", "flowId": "flow.bingo", "bingoSpecialPatternId": "special.bingo",
        "entries": [{"entryId": "bingo.parent", "kind": "PATTERN", "targetId": "parent.bingo", "waitAfterMs": 0}]})
    for flow in action["patternFlows"]: flow["displayName"] = flow["gateId"]
    return action, sequence


class RaidProjectionTests(unittest.TestCase):
    def test_hp_groups_preserve_saved_identity_and_completion_boundary(self):
        action, sequence = documents()
        flow = action["patternFlows"][0]
        flow["entryGroups"] = [{"groupId": "group.normal", "displayName": "Normal attacks",
            "startEntryId": "entry.GATE1", "endEntryId": "entry.GATE1",
            "repeatUntilHealthBars": 130, "transitionAt": "PATTERN_END"}]
        before = copy.deepcopy(action)
        projected = subject.project_raid_gates(action, sequence)[0]
        self.assertEqual(flow["entryGroups"], projected["entryGroups"])
        self.assertEqual(before, action)
        projected["entryGroups"][0]["repeatUntilHealthBars"] = 60
        self.assertEqual(130, flow["entryGroups"][0]["repeatUntilHealthBars"])
        for threshold in (0, 1000):
            flow["entryGroups"][0]["repeatUntilHealthBars"] = threshold
            flow["entryGroups"][0]["transitionAt"] = "GROUP_END"
            composition.validate_flow_groups(flow)

    def test_hp_groups_reject_missing_overlap_reversed_or_ambiguous_ranges(self):
        action, _ = documents()
        flow = action["patternFlows"][0]
        flow["entries"].append(dict(flow["entries"][0], entryId="entry.second"))
        group = {"groupId": "group.normal", "displayName": "Normal attacks",
            "startEntryId": "entry.GATE1", "endEntryId": "entry.second",
            "repeatUntilHealthBars": 130, "transitionAt": "PATTERN_END"}
        mutations = [dict(startEntryId="missing"), dict(endEntryId="missing"),
            dict(startEntryId="entry.second", endEntryId="entry.GATE1"),
            *[dict(repeatUntilHealthBars=v) for v in (-1, 1001, True, 3.5, None)],
            dict(transitionAt="IMMEDIATE"), dict(transitionAt=None)]
        for mutation in mutations:
            with self.subTest(mutation=mutation):
                flow["entryGroups"] = [dict(group, **mutation)]
                with self.assertRaises(composition.CompositionError): composition.validate_flow_groups(flow)
        flow["entryGroups"] = [group, dict(group, groupId="group.overlap")]
        with self.assertRaises(composition.CompositionError): composition.validate_flow_groups(flow)
        flow["entryGroups"] = [group]
        flow["loopStartEntryId"] = "entry.GATE1"
        with self.assertRaises(composition.CompositionError): composition.validate_flow_groups(flow)
        del flow["loopStartEntryId"]
        del flow["entryGroups"][0]["transitionAt"]
        with self.assertRaises(composition.CompositionError): composition.validate_flow_groups(flow)

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
        action, sequence = bingo_documents()
        flow = action["patternFlows"][-1]
        bingo = subject.project_raid_gates(action, sequence)[-1]
        self.assertEqual("BINGO", bingo["gateId"])
        self.assertEqual("boss.kakulsaydon.bingo.saydon", bingo["primaryBossPlacementId"])
        self.assertEqual((PREFIX + "10", 23333, []), (bingo["introPatternId"], bingo["introDurationMs"], bingo["arrivals"]))
        self.assertEqual(flow["entries"], bingo["entries"])
        self.assertEqual((PREFIX + "9", 51285), (bingo["clearPatternId"], bingo["clearDurationMs"]))
        self.assertEqual((sequence["compositionId"], sequence["revision"]),
                         (bingo["sequenceCompositionId"], bingo["sequenceRevision"]))
        next(p for p in action["patterns"] if p["patternId"] == "parent.bingo")["authoringStatus"] = "DRAFT"
        with self.assertRaisesRegex(ValueError, "unavailable parent.bingo"):
            subject.project_raid_gates(action, sequence)

    def test_bingo_special_draft_compatibility_projection_and_dependency_closure(self):
        action, sequence = bingo_documents()
        flow = action["patternFlows"][-1]
        before = copy.deepcopy(action)
        composition.validate_pattern_flows(action)
        self.assertEqual("special.bingo", subject.project_raid_gates(action, sequence)[-1]["bingoSpecialPatternId"])
        self.assertEqual(before, action)
        flow["entries"].append(dict(flow["entries"][0], entryId="bingo.second"))
        flow["entries"].reverse()
        self.assertEqual("special.bingo", subject.project_raid_gates(action, sequence)[-1]["bingoSpecialPatternId"])
        del flow["bingoSpecialPatternId"]
        composition.validate_pattern_flows(action)  # Old unassigned drafts remain readable.
        with self.assertRaisesRegex(ValueError, "bingoSpecialPatternId"):
            subject.project_raid_gates(action, sequence)
        actual = json.loads((ROOT / prepare.ACTION).read_text("utf-8-sig"))
        bingo = next(f for f in actual["patternFlows"] if f["gateId"] == "BINGO")
        bingo["bingoSpecialPatternId"] = PREFIX + "107"
        # This source-only metadata check does not admit draft Patterns or write live files.
        for pattern in actual["patterns"]: pattern["authoringStatus"] = "PRODUCT"
        self.assertEqual(PREFIX + "107", subject.validate_bingo_special(actual, bingo))
        parent = next(p for p in actual["patterns"] if p["patternId"] == PREFIX + "107")
        self.assertEqual({PREFIX + str(i) for i in (127, 94, 128)}, composition._pattern_dependencies(actual, parent))

    def test_bingo_special_rejects_ambiguous_or_unavailable_parent(self):
        mutations = [
            lambda a,f: f.update(bingoSpecialPatternId="missing.special"),
            lambda a,f: f.update(bingoSpecialPatternId=3),
            lambda a,f: a["patterns"][-2].update(gateId="GATE3"),
            lambda a,f: a["patterns"][-2].update(targetBossPlacementId="wrong.boss"),
            lambda a,f: a["patterns"][-2].update(patternOccurrences=[]),
            lambda a,f: a["patterns"][-2].update(loopStartPatternOccurrenceId="loop"),
            lambda a,f: a["patterns"][-2]["patternOccurrences"][0].update(repeat=True),
            lambda a,f: a["patterns"][-1].update(patternOccurrences=[{"patternId": "parent.bingo"}]),
            lambda a,f: a["patterns"][-1].update(actorProfileId="wrong.actor"),
            lambda a,f: a["patterns"][-1].update(authoringStatus="DRAFT"),
            lambda a,f: a["patterns"][-1]["logicOccurrences"][0].update(enabled=False),
            lambda a,f: a["patterns"][-2].update(logicOccurrences=[{"logicId": "detonate"}]),
            lambda a,f: f["entries"][0].update(targetId="special.bingo"),
            lambda a,f: (a["bundles"].append({"bundleId": "bundle.special", "members": [{"patternId": "special.bingo"}]}),
                         f["entries"][0].update(kind="BUNDLE", targetId="bundle.special")),
        ]
        for index, mutation in enumerate(mutations):
            with self.subTest(index=index):
                action, sequence = bingo_documents(); mutation(action, action["patternFlows"][-1])
                with self.assertRaises(ValueError): subject.project_raid_gates(action, sequence)
        for value in (None, 4, [], "missing.special"):
            action, _ = bingo_documents(); action["patternFlows"][-1]["bingoSpecialPatternId"] = value
            with self.assertRaises(composition.CompositionError): composition.validate_pattern_flows(action)
        action, _ = bingo_documents(); action["patternFlows"][0]["bingoSpecialPatternId"] = "special.bingo"
        with self.assertRaises(composition.CompositionError): composition.validate_pattern_flows(action)

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
        bingo_action, bingo_sequence = bingo_documents()
        valid["raidGates"].append(subject.project_raid_gates(bingo_action, bingo_sequence)[-1])
        valid["patterns"].extend([{"patternId": "parent.bingo", "gateId": "BINGO"},
            {"patternId": "special.bingo", "gateId": "BINGO", "fixedTimeline": True,
             "targetBossPlacementId": "boss.kakulsaydon.bingo.saydon", "stages": [{"durationMs": 32628}],
             "mechanicTriggers": [{"kind": "BINGO_DETONATION"}]}])
        cases = [{"name": "valid", "accepted": True, "document": valid}]
        for name in ("absent_special", "missing_special", "wrong_gate", "no_detonation", "double_detonation", "ordinary_special"):
            invalid = copy.deepcopy(valid)
            if name == "absent_special": del invalid["raidGates"][-1]["bingoSpecialPatternId"]
            elif name == "missing_special": invalid["raidGates"][-1]["bingoSpecialPatternId"] = "missing.special"
            elif name == "wrong_gate": invalid["patterns"][-1]["gateId"] = "GATE3"
            elif name == "no_detonation": invalid["patterns"][-1]["mechanicTriggers"] = []
            elif name == "double_detonation": invalid["patterns"][-1]["mechanicTriggers"] *= 2
            else: invalid["raidGates"][-1]["entries"][0]["targetId"] = "special.bingo"
            cases.append({"name": name, "accepted": False, "document": invalid})
        grouped = copy.deepcopy(valid)
        grouped["raidGates"][0]["entryGroups"] = [{"groupId": "normal.first", "displayName": "First normal group",
            "startEntryId": "entry.0", "endEntryId": "entry.10", "repeatUntilHealthBars": 130, "transitionAt": "PATTERN_END"},
            {"groupId": "mechanic.first", "displayName": "First mechanic", "startEntryId": "entry.11", "endEntryId": "entry.11"}]
        cases.append({"name": "hp_groups", "accepted": True, "document": grouped})
        for mutation in ({"startEntryId": "missing"}, {"endEntryId": "missing"}, {"repeatUntilHealthBars": -1},
                         {"repeatUntilHealthBars": True}, {"transitionAt": "IMMEDIATE"}):
            invalid = copy.deepcopy(grouped)
            invalid["raidGates"][0]["entryGroups"][0].update(mutation)
            cases.append({"name": "invalid_hp_group", "accepted": False, "document": invalid})
        invalid = copy.deepcopy(grouped)
        invalid["raidGates"][0]["entryGroups"][1]["startEntryId"] = "entry.10"
        cases.append({"name": "overlapping_hp_group", "accepted": False, "document": invalid})
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
. (Join-Path $Root 'Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1')
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
            $specials = @($gateRows | Where-Object { $_.StartsWith("RAIDBINGOSPECIAL`t") })
            if ($gate.gateId -ceq 'BINGO') {
                if ($specials.Count -ne 1 -or $specials[0] -cne "RAIDBINGOSPECIAL`tBINGO`t$($gate.bingoSpecialPatternId)") { throw 'Bingo special identity was lost' }
                if ([Array]::IndexOf($gateRows, $specials[0]) -le $index) { throw 'Special preceded normal Flow rows' }
            } elseif ($specials.Count -ne 0) { throw 'Other Gate acquired a Bingo special' }
            $groups = @($gateRows | Where-Object { $_.StartsWith("RAIDFLOWGROUP`t") })
            if ($null -ne $gate.PSObject.Properties['entryGroups']) {
                if ($groups.Count -ne @($gate.entryGroups).Count) { throw 'Flow groups were lost' }
                for ($i = 0; $i -lt $groups.Count; ++$i) {
                    $fields = $groups[$i].Split("`t")
                    if ([int]$fields[2] -ne $i -or $fields[3] -cne $gate.entryGroups[$i].groupId -or
                        $fields[4] -cne $gate.entryGroups[$i].startEntryId -or $fields[5] -cne $gate.entryGroups[$i].endEntryId) { throw 'Flow group identity or order changed' }
                    if ([Array]::IndexOf($gateRows, $groups[$i]) -le $index) { throw 'Group preceded its entry rows' }
                }
            }
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

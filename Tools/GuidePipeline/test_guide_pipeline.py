"""Guide source/save/publish regressions; all writes stay in disposable repo copies.

Run: python Tools/GuidePipeline/test_guide_pipeline.py
Requires Windows PowerShell, no Client/Server process or product build.
"""
from __future__ import annotations

import copy
import ctypes
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


REPO = Path(__file__).resolve().parents[2]
FILES = {
    "catalog": "GuideCatalog.json",
    "placement": "DimensionMaster/Placement.json",
    "prompts": "DimensionMaster/Prompts.json",
    "triggers": "DimensionMaster/Triggers.json",
    "combat": "DimensionMaster/Combat.json",
}
RUNTIMES = [Path(side) / "Bin/DataFiles/Guide/Guide.runtime.json" for side in ("Server", "Client")]


def read(path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def write(path, value, *, sorted_keys=False):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2, sort_keys=sorted_keys) + "\n", encoding="utf-8")


class GuidePipelineTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if os.name != "nt":
            raise unittest.SkipTest("Windows PowerShell and Windows file sharing are required")
        cls.temp = Path(tempfile.mkdtemp(prefix="lostark-guide-tests-" )).resolve()
        cls.base = cls.temp / "baseline"
        dependencies = [Path("Tools/GuidePipeline/Publish-Guide.ps1"),
                        Path("Tools/GameplayPipeline/Publish-FileTransaction.ps1"),
                        Path("Data/Maps/MapCatalog.json"), Path("Data/Balance/PlayerSkills.json"),
                        Path("Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap")]
        dependencies += [Path("Data/Guide") / p for p in FILES.values()]
        catalog = read(REPO / "Data/Guide/GuideCatalog.json")
        dependencies += [Path("Data/Worlds") / c["areaId"] / "Gameplay.world.json" for c in catalog["categories"]]
        for relative in dependencies:
            target = cls.base / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(REPO / relative, target)
        result = cls.invoke(cls.base, "Publish")
        if result.returncode:
            raise RuntimeError("Seed publish failed: " + result.stdout)

    @classmethod
    def tearDownClass(cls):
        # Verify the exact recursive deletion target belongs to our explicit temp root.
        target = cls.temp.resolve()
        assert target.parent == Path(tempfile.gettempdir()).resolve() and target.name.startswith("lostark-guide-tests-")
        shutil.rmtree(target)

    @staticmethod
    def invoke(root, mode, *args):
        return subprocess.run(["powershell.exe", "-NoLogo", "-NoProfile", "-ExecutionPolicy", "Bypass",
                               "-File", str(root / "Tools/GuidePipeline/Publish-Guide.ps1"),
                               "-RepositoryRoot", str(root), "-Mode", mode, *map(str, args)],
                              stdout=subprocess.PIPE, stderr=subprocess.STDOUT, encoding="utf-8", errors="replace", timeout=90)

    def setUp(self):
        self.root = self.temp / self._testMethodName
        shutil.copytree(self.base, self.root)

    def path(self, key):
        return self.root / "Data/Guide" / FILES[key]

    def bundle(self):
        return {key: read(self.path(key)) for key in FILES}

    def snapshot(self):
        return {str(p.relative_to(self.root)): p.read_bytes()
                for p in [*[self.path(k) for k in FILES], *[self.root / p for p in RUNTIMES]]}

    def assert_preserved(self, before):
        self.assertEqual(before, self.snapshot())
        self.assertFalse(list(self.root.rglob("*.staging.*")))
        self.assertFalse(list(self.root.rglob("*.rollback.*")))

    def run_mode(self, mode, *, succeeds=True, contains=None, args=()):
        result = self.invoke(self.root, mode, *args)
        self.assertEqual(result.returncode == 0, succeeds, result.stdout)
        if contains:
            self.assertIn(contains, result.stdout)
        return result

    def save(self, baseline, draft, **kwargs):
        # CDataJson uses ordered object keys; mimic the actual tool serialization.
        before = self.root / "editor.baseline.json"
        candidate = self.root / "editor.draft.json"
        write(before, baseline, sorted_keys=True)
        write(candidate, draft, sorted_keys=True)
        return self.run_mode("Save", args=("-BaselinePath", before, "-DraftPath", candidate), **kwargs)

    def mutate(self, key, callback):
        doc = read(self.path(key))
        callback(doc)
        write(self.path(key), doc)

    def invalid_publish(self, key, callback, contains):
        self.mutate(key, callback)
        before = self.snapshot()
        self.run_mode("Publish", succeeds=False, contains=contains)
        self.assert_preserved(before)

    def test_publish_check_and_runtime_skill_resolution(self):
        self.run_mode("CheckPublished")
        self.assertEqual((self.root / RUNTIMES[0]).read_bytes(), (self.root / RUNTIMES[1]).read_bytes())
        runtime = read(self.root / RUNTIMES[0])
        skills = read(self.root / "Data/Balance/PlayerSkills.json")["skills"]
        expected = {s["inputSlot"]: s["skillId"] for s in skills if s["characterClass"] == "DIMENSIONMASTER" and s["skillKind"] == "ACTIVE"}
        for combo in runtime["combat"]["combos"]:
            self.assertEqual(combo["resolvedSkillIds"], [expected[slot] for slot in combo["inputSlots"]])
        self.assertGreater(runtime["revision"], 0)

    def test_personal_start_and_committed_return_events_are_published(self):
        runtime = read(self.root / RUNTIMES[0])
        events = [t["event"] for t in runtime["triggers"]]
        self.assertIn({"type": "GUIDE_STARTED"}, events)
        for world in ("VALTAN_ARENA", "KAKULSAYDON_ARENA"):
            self.assertIn({"type": "RAID_RETURNED", "raidWorldId": world}, events)
        for world in ("MAHARAKA", "COLOSSEUM"):
            self.assertIn({"type": "WORLD_RETURNED", "sourceWorldId": world}, events)
        boxes = [t for t in runtime["triggers"] if t["event"]["type"] == "SPACE_ENTER"]
        self.assertEqual(len({t["event"]["boxId"] for t in boxes}), len(boxes))
        for npc in ("npc.bern.src.31", "npc.bern.src.48", "npc.bern.ship.harbormaster.1"):
            self.assertTrue(any(t["event"]["anchorPlacementId"] == npc for t in boxes))

    def test_raid_return_rejects_nonraid_source_and_preserves_outputs(self):
        def mutate(doc):
            next(t for t in doc["triggers"] if t["event"]["type"] == "RAID_RETURNED")["event"]["raidWorldId"] = "BERN"
        self.invalid_publish("triggers", mutate, "supported source raid")

    def test_world_return_rejects_raid_source_and_preserves_outputs(self):
        def mutate(doc):
            next(t for t in doc["triggers"] if t["event"]["type"] == "WORLD_RETURNED")["event"]["sourceWorldId"] = "VALTAN_ARENA"
        self.invalid_publish("triggers", mutate, "supported source world")

    def test_personal_return_rejects_nonbern_destination_category(self):
        def mutate(doc):
            next(t for t in doc["triggers"] if t["event"]["type"] == "RAID_RETURNED")["categoryId"] = "valtan"
        self.invalid_publish("triggers", mutate, "Raid return requires Bern")

    def test_malformed_json_preserves_both_runtime_files(self):
        self.path("prompts").write_bytes(b'{"prompts": [')
        before = self.snapshot()
        self.run_mode("Publish", succeeds=False)
        self.assert_preserved(before)

    def test_duplicate_json_property_rejected(self):
        text = self.path("placement").read_text(encoding="utf-8-sig")
        self.path("placement").write_text(text.replace('"revision":', '"revision":123,"revision":', 1), encoding="utf-8")
        before = self.snapshot()
        self.run_mode("Publish", succeeds=False, contains="Duplicate JSON property")
        self.assert_preserved(before)

    def test_duplicate_prompt_id_rejected(self):
        self.invalid_publish("prompts", lambda d: d["prompts"].append(copy.deepcopy(d["prompts"][0])), "Duplicate prompt")

    def test_invalid_stable_id_rejected(self):
        self.invalid_publish("prompts", lambda d: d["prompts"][0].update(promptId="../bad prompt"), "ASCII stable ID")

    def test_invalid_skill_slot_rejected(self):
        self.invalid_publish("combat", lambda d: d["combos"][0]["inputSlots"].append("NOT_A_SLOT"), "Unresolvable active skill")

    def test_duplicate_enabled_alias_rejected(self):
        self.invalid_publish("combat", lambda d: d["commands"][1]["aliases"].append(d["commands"][0]["aliases"][0]), "Duplicate enabled command alias")

    def test_guide_name_wire_limit_rejected(self):
        self.invalid_publish("catalog", lambda d: d["guides"][0].update(displayName="X" * 33), "Guide name")

    def test_non_help_trigger_cannot_start_combat(self):
        combo = read(self.path("combat"))["combos"][0]["comboId"]
        self.invalid_publish("triggers", lambda d: next(t for t in d["triggers"] if t["event"]["type"] != "HELP_COMMAND").update(comboId=combo), "Only HELP_COMMAND")

    def test_prompt_wire_text_limit_rejected(self):
        self.invalid_publish("prompts", lambda d: d["prompts"][0]["segments"][0].update(text="X" * 513), "Segment")

    def test_prompt_control_character_rejected(self):
        self.invalid_publish("prompts", lambda d: d["prompts"][0]["segments"][0].update(text="bad\x01text"), "Segment")

    def test_fractional_duration_rejected_before_server_load(self):
        self.invalid_publish("prompts", lambda d: d["prompts"][0]["segments"][0].update(durationMs=2000.5), "whole number")

    def test_property_order_does_not_change_runtime_revision(self):
        expected = read(self.root / RUNTIMES[0])
        for key in FILES:
            write(self.path(key), read(self.path(key)), sorted_keys=True)
        self.run_mode("Publish")
        actual = read(self.root / RUNTIMES[0])
        self.assertEqual(actual, expected)

    def test_save_noop_preserves_source_bytes_and_revision(self):
        baseline = self.bundle()
        before = self.snapshot()
        self.save(baseline, copy.deepcopy(baseline))
        self.assert_preserved(before)

    def test_save_independent_fields_merge_and_publish_is_explicit(self):
        baseline = self.bundle()
        draft = copy.deepcopy(baseline)
        draft["prompts"]["prompts"][0]["title"] = "editor changed title"
        self.mutate("prompts", lambda d: d["prompts"][0]["segments"][0].update(text="another editor changed text"))
        prior_runtime = [(self.root / p).read_bytes() for p in RUNTIMES]
        self.save(baseline, draft)
        actual = read(self.path("prompts"))
        self.assertEqual(actual["prompts"][0]["title"], "editor changed title")
        self.assertEqual(actual["prompts"][0]["segments"][0]["text"], "another editor changed text")
        self.assertEqual(actual["revision"], baseline["prompts"]["revision"] + 1)
        self.assertEqual(prior_runtime, [(self.root / p).read_bytes() for p in RUNTIMES])
        self.run_mode("CheckPublished", succeeds=False, contains="runtime is stale")
        self.run_mode("Publish")
        self.run_mode("CheckPublished")

    def test_save_same_field_conflict_preserves_disk_and_draft(self):
        baseline = self.bundle()
        draft = copy.deepcopy(baseline)
        draft["prompts"]["prompts"][0]["title"] = "editor title"
        self.mutate("prompts", lambda d: d["prompts"][0].update(title="external title"))
        before = self.snapshot()
        self.save(baseline, draft, succeeds=False, contains="Save conflict")
        self.assert_preserved(before)
        self.assertEqual(read(self.root / "editor.draft.json"), draft)

    def test_save_invalid_candidate_preserves_all_sources(self):
        baseline = self.bundle()
        draft = copy.deepcopy(baseline)
        draft["placement"]["yawDegrees"] += 15
        draft["combat"]["combos"][0]["inputSlots"] = ["NO_SKILL"]
        before = self.snapshot()
        self.save(baseline, draft, succeeds=False, contains="Unresolvable active skill")
        self.assert_preserved(before)

    def test_concurrent_row_addition_survives_save(self):
        baseline = self.bundle()
        draft = copy.deepcopy(baseline)
        draft["prompts"]["prompts"][0]["title"] = "editor title"
        new_row = copy.deepcopy(baseline["prompts"]["prompts"][0])
        new_row.update(promptId="guide.test.external", title="external row")
        self.mutate("prompts", lambda d: d["prompts"].append(new_row))
        self.save(baseline, draft)
        rows = {r["promptId"]: r for r in read(self.path("prompts"))["prompts"]}
        self.assertEqual(rows[new_row["promptId"]], new_row)
        self.assertEqual(rows[baseline["prompts"]["prompts"][0]["promptId"]]["title"], "editor title")

    def test_source_changed_after_validation_aborts_publish(self):
        self.mutate("prompts", lambda d: d["prompts"][0].update(title="changed candidate"))
        runtimes = [(self.root / p).read_bytes() for p in RUNTIMES]
        helper = self.root / "Tools/GameplayPipeline/Publish-FileTransaction.ps1"
        text = helper.read_text(encoding="utf-8-sig").replace("function Assert-PublishSourceSnapshots(", "function Original-AssertPublishSourceSnapshots(", 1)
        text += '''\nfunction Assert-PublishSourceSnapshots([hashtable]$Sources) {
    Original-AssertPublishSourceSnapshots $Sources
    if(-not $script:InjectedGuideSourceEdit){
        $script:InjectedGuideSourceEdit=$true
        $path=Join-Path $RepositoryRoot 'Data/Guide/DimensionMaster/Placement.json'
        [IO.File]::AppendAllText($path," ")
    }
}
$script:InjectedGuideSourceEdit=$false
'''
        helper.write_text(text, encoding="utf-8")
        source = self.path("placement").read_bytes()
        self.run_mode("Publish", succeeds=False, contains="input changed during validation")
        self.assertEqual(self.path("placement").read_bytes(), source + b" ")
        self.assertEqual(runtimes, [(self.root / p).read_bytes() for p in RUNTIMES])
        self.assertFalse(list(self.root.rglob("*.staging.*")))

    def test_second_runtime_replace_failure_rolls_back_first(self):
        self.mutate("prompts", lambda d: d["prompts"][0].update(title="changed candidate"))
        before = self.snapshot()
        kernel = ctypes.WinDLL("kernel32", use_last_error=True)
        kernel.CreateFileW.argtypes = [ctypes.c_wchar_p, ctypes.c_uint32, ctypes.c_uint32, ctypes.c_void_p, ctypes.c_uint32, ctypes.c_uint32, ctypes.c_void_p]
        kernel.CreateFileW.restype = ctypes.c_void_p
        kernel.CloseHandle.argtypes = [ctypes.c_void_p]
        # Permit reads/hashes but deny deletion/replacement of only the second destination.
        handle = kernel.CreateFileW(str(self.root / RUNTIMES[1]), 0x80000000, 3, None, 3, 128, None)
        self.assertNotEqual(handle, ctypes.c_void_p(-1).value)
        try:
            self.run_mode("Publish", succeeds=False)
        finally:
            kernel.CloseHandle(handle)
        self.assert_preserved(before)

    def test_post_replace_verification_failure_rolls_back_current_file(self):
        self.mutate("prompts", lambda d: d["prompts"][0].update(title="changed candidate"))
        before = self.snapshot()
        helper = self.root / "Tools/GameplayPipeline/Publish-FileTransaction.ps1"
        text = helper.read_text(encoding="utf-8-sig").replace("function Get-PublishFileSha256(", "function Original-GetPublishFileSha256(", 1)
        text += '''\nfunction Get-PublishFileSha256([string]$Path) {
    if($Path.Contains('.rollback.') -and -not $script:InjectedGuideFailure){
        $script:InjectedGuideFailure=$true; throw 'Injected post-replace verification failure'
    }
    Original-GetPublishFileSha256 $Path
}
$script:InjectedGuideFailure=$false
'''
        helper.write_text(text, encoding="utf-8")
        self.run_mode("Publish", succeeds=False, contains="Injected post-replace")
        self.assert_preserved(before)


if __name__ == "__main__":
    unittest.main(verbosity=2)

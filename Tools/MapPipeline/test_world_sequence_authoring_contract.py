from __future__ import annotations

import re
import copy
import json
import subprocess
import tempfile
import unittest
import xml.etree.ElementTree as ET
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


class WorldSequenceAuthoringContractTests(unittest.TestCase):
    """Fast source/project integration guards; the Product build compiles behavior."""

    def setUp(self) -> None:
        self.document_h = read("Client/Public/WorldSequenceDocument.h")
        self.document_cpp = read("Client/Private/WorldSequenceDocument.cpp")
        self.panel_h = read("Client/Public/WorldSequenceToolPanel.h")
        self.panel_cpp = read("Client/Private/WorldSequenceToolPanel.cpp")
        self.map_tool_h = read("Client/Public/MapTool.h")
        self.map_tool_cpp = read("Client/Private/MapTool.cpp")

    def test_walkable_surface_accepts_fixed_roulette_and_rejects_unsupported_geometry(self) -> None:
        source = json.loads(read("Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json"))
        instance = next(row for row in source["instances"] if row["instanceId"] == "world.sequence.instance.8")
        instance["walkableSurface"] = {"radiusM": 2.5, "localHeightM": .026313}
        source["instances"] = [instance]
        source["templates"] = [row for row in source["templates"] if row["sequenceId"] == instance["templateId"]]
        source["objectResources"] = [row for row in source.get("objectResources", []) if row.get("sequenceInstanceId") == instance["instanceId"]]
        keys = source["templates"][0]["tracks"][0]["keys"]
        source["templates"][0]["tracks"][0]["keys"] = [keys[0], keys[-1]]
        cases = [("valid", source, True)]
        for name, mutate in (
            ("negative_radius", lambda d: d["instances"][0]["walkableSurface"].update(radiusM=-1)),
            ("boolean_height", lambda d: d["instances"][0]["walkableSurface"].update(localHeightM=True)),
            ("unknown_field", lambda d: d["instances"][0]["walkableSurface"].update(height=0)),
            ("moving_plane", lambda d: d["templates"][0]["tracks"][0]["keys"][-1].update(positionOffset=[0, 1, 0])),
            ("tilted_plane", lambda d: d["templates"][0]["tracks"][0]["keys"][-1].update(rotationQuaternion=[1, 0, 0, 0])),
            ("nonuniform_scale", lambda d: d["templates"][0]["tracks"][0]["keys"][0].update(scaleMultiplier=[1, 1, 2])),
        ):
            document = copy.deepcopy(source); mutate(document); cases.append((name, document, False))
        publisher = read("Tools/MapPipeline/Publish-MapAuthoring.ps1")
        definitions = [re.search(r"(?ms)^function " + name + r" \{.*?^\}", publisher).group(0)
                       for name in ("Test-JsonNumber", "Assert-ExactJsonProperties", "Read-WorldSequenceDocument")]
        with tempfile.TemporaryDirectory() as temporary:
            folder = Path(temporary)
            for name, document, _ in cases:
                (folder / (name + ".json")).write_text(json.dumps(document), encoding="utf-8")
            script = "$ErrorActionPreference='Stop'\n$AreaId='LV_LUT_MIDNIGHTC_ED'\n" + "\n".join(definitions)
            script += "\n$results=@(); foreach($file in Get-ChildItem -LiteralPath $PSScriptRoot -Filter '*.json') { try { [void](Read-WorldSequenceDocument $file.FullName); $ok=$true } catch { $ok=$false }; $results += [pscustomobject]@{name=$file.BaseName;valid=$ok} }; ConvertTo-Json -InputObject @($results) -Compress"
            script_path = folder / "validate.ps1"; script_path.write_text(script, encoding="utf-8-sig")
            result = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(script_path)], capture_output=True, text=True)
            self.assertEqual(0, result.returncode, result.stderr)
            actual = {row["name"]: row["valid"] for row in json.loads(result.stdout)}
            self.assertEqual({name: valid for name, _, valid in cases}, actual)

    def test_v3_object_publisher_accepts_source_and_rejects_invalid_resources(self) -> None:
        """Exercise the actual publisher function on a preserved source plus one model state."""
        source = json.loads(read("Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json"))
        # Keep the real placed curtain state to cover compatibility without scanning every roulette key per mutant.
        curtain = next(row for row in source["instances"] if row["instanceId"] == "world.sequence.instance.curtain_drop")
        source["instances"] = [curtain]
        source["templates"] = [row for row in source["templates"] if row["sequenceId"] == curtain["templateId"]]
        source["objectResources"] = [row for row in source.get("objectResources", [])
                                     if row.get("sequenceInstanceId") == curtain["instanceId"]]
        source["formatVersion"] = 3
        source.setdefault("objectResources", []).append({
            "objectId": "test.world.object", "displayName": "Test object", "modelAssetId": "Map/Test/test.wmodel",
            "diffuseTextureAssetId": "", "modelPreScale": 0.01, "animated": False,
            "scale": [1, 1, 1], "sequenceInstanceId": "",
        })
        source["templates"].append({
            "sequenceId": "test.world.template", "displayName": "Test state", "category": "Object",
            "durationMs": 1000, "interpolation": "LINEAR", "animationTracks": [],
            "objectMotion": {"velocity": [0, 1, 0], "acceleration": [0, -2, 0], "angularVelocityDegrees": [0, 180, 0],
                             "revolutionDegreesPerSecond": [0, 0, 0], "revolutionOffset": [0, 0, 0],
                             "count": 2, "intervalMs": 100, "spreadDegrees": 30, "seed": 7},
            "tracks": [{"slotId": "object", "keys": [
                {"timeMs": time, "positionOffset": [0, 0, 0], "rotationQuaternion": [0, 0, 0, 1],
                 "scaleMultiplier": [1, 1, 1], "visible": True} for time in (0, 1000)]}],
        })
        source["instances"].append({
            "instanceId": "test.world.instance", "templateId": "test.world.template", "enabled": True,
            "startDelayMs": 0, "playbackSpeed": 1, "anchorKind": "PLAYER", "position": [1, 2, 3],
            "bindings": [{"slotId": "object", "targetKind": "OBJECT_RESOURCE", "targetId": "test.world.object"}],
        })
        cases = [("valid", source, True)]
        with_default = copy.deepcopy(source)
        with_default["objectResources"][-1]["defaultMotionInstanceId"] = "test.world.instance"
        cases.append(("default_motion_valid", with_default, True))
        with_effect = copy.deepcopy(source)
        with_effect["templates"][-1]["effectTracks"] = [{
            "effectTrackId": "effect.smoke", "slotId": "object", "resourceKind": "GROUP",
            "resourceId": "boss.kouku.ball.smoke", "timing": "MOTION_END", "startMs": 0,
            "durationMs": 2000, "positionOffset": [0, -2.3, -1.35],
            "rotationDegrees": [0, 0, 0], "scale": [1, 1, 1],
        }]
        cases.append(("effect_motion_end", with_effect, True))
        ten_balls = copy.deepcopy(with_effect)
        ten_balls["templates"][-1]["objectMotion"].update(count=10, intervalMs=300, spreadDegrees=360)
        cases.append(("effect_ten_complete_emissions", ten_balls, True))
        at_time = copy.deepcopy(with_effect)
        at_time["templates"][-1]["effectTracks"][0].update(timing="TIME", startMs=1000)
        cases.append(("effect_time_includes_model_end", at_time, True))
        for name, fields in (
            ("unknown_timing", {"timing": "FINISH"}), ("end_offset", {"startMs": 1}),
            ("missing_slot", {"slotId": "absent"}), ("unknown_kind", {"resourceKind": "WORLD"}),
            ("path_identity", {"resourceId": "../smoke"}), ("zero_window", {"durationMs": 0}),
            ("boolean_time", {"startMs": True}), ("time_past_end", {"timing": "TIME", "startMs": 1001}),
            ("negative_scale", {"scale": [-1, 1, 1]}), ("excessive_tail", {"durationMs": 600000}),
        ):
            invalid = copy.deepcopy(with_effect)
            invalid["templates"][-1]["effectTracks"][0].update(fields)
            cases.append(("effect_invalid_" + name, invalid, False))
        duplicate_effect = copy.deepcopy(with_effect)
        duplicate_effect["templates"][-1]["effectTracks"] *= 2
        cases.append(("effect_duplicate_track_id", duplicate_effect, False))
        for name, default_id in (("missing", "missing.instance"), ("foreign", curtain["instanceId"]),
                                 ("number", 4), ("null", None), ("empty", "")):
            candidate = copy.deepcopy(source)
            candidate["objectResources"][-1]["defaultMotionInstanceId"] = default_id
            cases.append(("default_motion_" + name, candidate, name == "empty"))
        disabled_default = copy.deepcopy(with_default)
        disabled_default["instances"][-1]["enabled"] = False
        cases.append(("default_motion_disabled", disabled_default, False))
        alias_default = copy.deepcopy(source)
        alias_default["objectResources"][0]["defaultMotionInstanceId"] = curtain["instanceId"]
        cases.append(("default_motion_alias_valid", alias_default, True))
        alias_default_other = copy.deepcopy(alias_default)
        alias_default_other["objectResources"][0]["defaultMotionInstanceId"] = "test.world.instance"
        cases.append(("default_motion_alias_foreign", alias_default_other, False))
        for end in ("STOP", "HOLD", "LOOP"):
            completed = copy.deepcopy(source)
            completed["instances"][-1].update(motionEnd=end, nextMotionId="")
            cases.append(("motion_end_" + end, completed, True))
        for name, fields in (
            ("unknown", {"motionEnd": "FINISH"}), ("lowercase", {"motionEnd": "hold"}),
            ("number", {"motionEnd": 1}), ("null", {"motionEnd": None}),
            ("boolean", {"motionEnd": True}), ("next_missing", {"motionEnd": "NEXT"}),
            ("next_empty", {"motionEnd": "NEXT", "nextMotionId": ""}),
            ("stop_target", {"motionEnd": "STOP", "nextMotionId": "test.world.instance"}),
            ("implicit_stop_target", {"nextMotionId": "test.world.instance"}),
            ("target_number", {"nextMotionId": 7}), ("target_null", {"nextMotionId": None}),
        ):
            invalid = copy.deepcopy(source)
            invalid["instances"][-1].update(fields)
            cases.append(("motion_end_invalid_" + name, invalid, False))
        placed_completion = copy.deepcopy(source)
        placed_completion["instances"][0]["motionEnd"] = "HOLD"
        cases.append(("motion_end_placed_target", placed_completion, False))
        next_source = copy.deepcopy(source)
        next_source["templates"][-1]["objectMotion"]["count"] = 1
        next_state = copy.deepcopy(next_source["instances"][-1])
        next_state.update(instanceId="test.world.next", motionEnd="HOLD")
        next_source["instances"][-1].update(motionEnd="NEXT", nextMotionId=next_state["instanceId"])
        next_source["instances"].append(next_state)
        cases.append(("motion_next_valid", next_source, True))
        for name, mutate in (
            ("unknown", lambda d: d["instances"][-2].update(nextMotionId="missing.motion")),
            ("case_mismatch", lambda d: d["instances"][-2].update(nextMotionId="TEST.WORLD.NEXT")),
            ("disabled", lambda d: d["instances"][-1].update(enabled=False)),
            ("self", lambda d: d["instances"][-2].update(nextMotionId="test.world.instance")),
            ("cycle", lambda d: d["instances"][-1].update(motionEnd="NEXT", nextMotionId="test.world.instance")),
            ("multiple_emissions", lambda d: d["templates"][-1]["objectMotion"].update(count=2)),
        ):
            invalid = copy.deepcopy(next_source)
            mutate(invalid)
            cases.append(("motion_next_invalid_" + name, invalid, False))
        other_resource = copy.deepcopy(next_source)
        resource_copy = copy.deepcopy(other_resource["objectResources"][-1])
        resource_copy["objectId"] = "test.other.object"
        other_resource["objectResources"].append(resource_copy)
        other_resource["instances"][-1]["bindings"][0]["targetId"] = resource_copy["objectId"]
        cases.append(("motion_next_different_resource", other_resource, False))
        other_slot = copy.deepcopy(next_source)
        template_copy = copy.deepcopy(other_slot["templates"][-1])
        template_copy["sequenceId"] = "test.other.slot.template"
        template_copy["tracks"][0]["slotId"] = "another.slot"
        other_slot["templates"].append(template_copy)
        other_slot["instances"][-1].update(templateId=template_copy["sequenceId"])
        other_slot["instances"][-1]["bindings"][0]["slotId"] = "another.slot"
        cases.append(("motion_next_different_slot", other_slot, False))
        for depth in (32, 33):
            chain = copy.deepcopy(next_source)
            prototype = chain["instances"][-1]
            chain["instances"] = [chain["instances"][0]]
            for index in range(depth + 1):
                state = copy.deepcopy(prototype)
                state.update(instanceId=f"test.motion.{index}", motionEnd="NEXT" if index < depth else "STOP")
                if index < depth:
                    state["nextMotionId"] = f"test.motion.{index + 1}"
                chain["instances"].append(state)
            cases.append((f"motion_next_depth_{depth}", chain, depth == 32))
        animation_source = copy.deepcopy(source)
        animation_source["objectResources"][-1]["animated"] = True
        animation_source["templates"][-1]["animationTracks"] = [{
            "slotId": "object", "clipName": "mn_rhoc_00_sk.ao_idle_normal_1",
            "playbackRate": 1, "loop": False, "holdLastFrame": True,
        }]
        cases.append(("animation_label_absent", animation_source, True))
        for name, label, valid in (
            ("empty", "", True), ("korean", "카드_등장", True),
            ("maximum", "a" * 128, True), ("bytes_over", "한" * 43, False),
            ("long", "a" * 129, False), ("number", 7, False),
            ("boolean", True, False), ("null", None, False),
            ("control", "card\nlabel", False),
        ):
            labeled = copy.deepcopy(animation_source)
            labeled["templates"][-1]["animationTracks"][0]["displayName"] = label
            cases.append(("animation_label_" + name, labeled, valid))
        for anchor in ("WORLD", "PLAYER"):
            anchored = copy.deepcopy(source)
            anchored["objectResources"][-1]["anchorKind"] = anchor
            cases.append(("resource_anchor_" + anchor, anchored, True))
        boss_source = copy.deepcopy(source)
        boss_source["objectResources"][-1].update(
            anchorKind="BOSS", anchorBossArchetypeId="BOSS_KOUKU", anchorBone="bip001_R_Hand")
        boss_source["instances"][-1]["anchorKind"] = "BOSS"
        cases.append(("boss_hand_anchor", boss_source, True))
        for name, bone, valid in (
            ("root", "", True), ("maximum", "a" * 128, True), ("korean", "오른손", True),
            ("bytes_over", "한" * 43, False), ("too_long", "a" * 129, False),
            ("control", "hand\n", False), ("number", 7, False),
            ("boolean", True, False), ("null", None, False),
        ):
            candidate = copy.deepcopy(boss_source)
            candidate["objectResources"][-1]["anchorBone"] = bone
            cases.append(("boss_bone_" + name, candidate, valid))
        boss_root = copy.deepcopy(boss_source)
        del boss_root["objectResources"][-1]["anchorBone"]
        cases.append(("boss_bone_absent_root", boss_root, True))
        for name, boss_id, valid in (
            ("maximum", "a" * 128, True), ("empty", "", False),
            ("space", "boss kouku", False), ("path", "../kouku", False),
            ("too_long", "a" * 129, False), ("number", 7, False),
            ("boolean", True, False), ("null", None, False),
        ):
            candidate = copy.deepcopy(boss_source)
            candidate["objectResources"][-1]["anchorBossArchetypeId"] = boss_id
            cases.append(("boss_archetype_" + name, candidate, valid))
        missing_boss = copy.deepcopy(boss_source)
        del missing_boss["objectResources"][-1]["anchorBossArchetypeId"]
        cases.append(("boss_archetype_absent", missing_boss, False))
        for anchor in ("WORLD", "PLAYER"):
            empty_fields = copy.deepcopy(source)
            empty_fields["objectResources"][-1].update(
                anchorKind=anchor, anchorBossArchetypeId="", anchorBone="")
            cases.append(("nonboss_empty_fields_" + anchor, empty_fields, True))
            for field, value in (("anchorBossArchetypeId", "BOSS_KOUKU"), ("anchorBone", "hand"),
                                 ("anchorBossArchetypeId", None), ("anchorBone", None)):
                candidate = copy.deepcopy(empty_fields)
                candidate["objectResources"][-1][field] = value
                cases.append((f"nonboss_{anchor}_{field}_{value}", candidate, False))
            candidate = copy.deepcopy(boss_source)
            candidate["instances"][-1]["anchorKind"] = anchor
            cases.append(("boss_resource_instance_mismatch_" + anchor, candidate, False))
            candidate = copy.deepcopy(source)
            candidate["objectResources"][-1]["anchorKind"] = anchor
            candidate["instances"][-1]["anchorKind"] = "BOSS"
            cases.append(("boss_instance_resource_mismatch_" + anchor, candidate, False))
        implicit_instance = copy.deepcopy(boss_source)
        del implicit_instance["instances"][-1]["anchorKind"]
        cases.append(("boss_resource_implicit_world_instance", implicit_instance, False))
        multiple_boss_bindings = copy.deepcopy(boss_source)
        second_object = copy.deepcopy(multiple_boss_bindings["objectResources"][-1])
        second_object["objectId"] = "test.second.boss.object"
        multiple_boss_bindings["objectResources"].append(second_object)
        second_track = copy.deepcopy(multiple_boss_bindings["templates"][-1]["tracks"][0])
        second_track["slotId"] = "second.object"
        multiple_boss_bindings["templates"][-1]["tracks"].append(second_track)
        multiple_boss_bindings["instances"][-1]["bindings"].append({
            "slotId": "second.object", "targetKind": "OBJECT_RESOURCE", "targetId": second_object["objectId"]})
        cases.append(("boss_anchor_multiple_resources", multiple_boss_bindings, False))
        placed_boss = copy.deepcopy(source)
        placed_boss["instances"][0]["anchorKind"] = "BOSS"
        cases.append(("boss_anchor_placed_instance", placed_boss, False))
        alias_boss = copy.deepcopy(source)
        alias_boss["objectResources"][0].update(
            anchorKind="BOSS", anchorBossArchetypeId="BOSS_KOUKU", anchorBone="")
        cases.append(("boss_anchor_placed_alias", alias_boss, False))
        for name, mutate in (
            ("bad_count", lambda d: d["templates"][-1]["objectMotion"].update(count=0)),
            ("spawn_after_lifetime", lambda d: d["templates"][-1]["objectMotion"].update(intervalMs=1000)),
            ("unknown_resource", lambda d: d["instances"][-1]["bindings"][0].update(targetId="missing.object")),
            ("path_escape", lambda d: d["objectResources"][-1].update(modelAssetId="Map/../test.wmodel")),
            ("unknown_property", lambda d: d["objectResources"][-1].update(velocty=1)),
            ("unknown_alias", lambda d: d["objectResources"][-1].update(modelAssetId="", sequenceInstanceId="missing.instance")),
            ("invalid_anchor", lambda d: d["instances"][-1].update(anchorKind="UNKNOWN")),
            ("invalid_resource_anchor", lambda d: d["objectResources"][-1].update(anchorKind="UNKNOWN")),
            ("invalid_resource_anchor_type", lambda d: d["objectResources"][-1].update(anchorKind=0)),
            ("placed_alias_character_anchor", lambda d: d["objectResources"][0].update(anchorKind="PLAYER")),
        ):
            invalid = copy.deepcopy(source)
            mutate(invalid)
            cases.append((name, invalid, False))
        # Authored emission rows replace the seeded emitter: count follows the rows, interval/spread stay 0,
        # every delay stays inside the lifetime, and NEXT keeps requiring a single emission.
        emitted = copy.deepcopy(source)
        emitted["templates"][-1]["objectMotion"].update(count=3, intervalMs=0, spreadDegrees=0, emissions=[
            {"positionOffset": [0, 0, 0], "yawDegrees": 0, "startDelayMs": 0},
            {"positionOffset": [4, 0, 0], "yawDegrees": 90, "startDelayMs": 200},
            {"positionOffset": [-4, 0, 0], "yawDegrees": -90, "startDelayMs": 400}])
        cases.append(("authored_emissions_valid", emitted, True))
        for name, mutate in (
            ("count_mismatch", lambda d: d["templates"][-1]["objectMotion"].update(count=2)),
            ("interval_with_rows", lambda d: d["templates"][-1]["objectMotion"].update(intervalMs=100)),
            ("spread_with_rows", lambda d: d["templates"][-1]["objectMotion"].update(spreadDegrees=30)),
            ("delay_after_lifetime", lambda d: d["templates"][-1]["objectMotion"]["emissions"][-1].update(startDelayMs=1000)),
            ("unknown_row_field", lambda d: d["templates"][-1]["objectMotion"]["emissions"][0].update(phase=1)),
            ("missing_row_field", lambda d: d["templates"][-1]["objectMotion"]["emissions"][0].pop("yawDegrees")),
            ("fractional_delay", lambda d: d["templates"][-1]["objectMotion"]["emissions"][1].update(startDelayMs=200.5)),
            ("empty_rows", lambda d: d["templates"][-1]["objectMotion"].update(count=1, emissions=[])),
        ):
            invalid = copy.deepcopy(emitted)
            mutate(invalid)
            cases.append(("authored_emissions_invalid_" + name, invalid, False))
        publisher = read("Tools/MapPipeline/Publish-MapAuthoring.ps1")
        definitions = []
        for name in ("Test-JsonNumber", "Assert-ExactJsonProperties", "Read-WorldSequenceDocument"):
            match = re.search(r"(?m)^function " + re.escape(name) + r" \{.*?^\}", publisher, re.DOTALL)
            self.assertIsNotNone(match)
            definitions.append(match.group(0))
        with tempfile.TemporaryDirectory() as temporary:
            folder = Path(temporary)
            for name, document, _ in cases:
                (folder / (name + ".json")).write_text(json.dumps(document), encoding="utf-8")
            malformed_utf8 = json.dumps(animation_source).replace(
                '"clipName": "mn_rhoc_00_sk.ao_idle_normal_1"',
                '"clipName": "mn_rhoc_00_sk.ao_idle_normal_1", "displayName": "LABEL_BYTE"',
            ).encode("utf-8").replace(b"LABEL_BYTE", b"\xc0\x80")
            (folder / "animation_label_invalid_utf8.json").write_bytes(malformed_utf8)
            cases.append(("animation_label_invalid_utf8", animation_source, False))
            script = "$ErrorActionPreference='Stop'\n$AreaId='LV_LUT_MIDNIGHTC_ED'\n" + "\n".join(definitions)
            script += "\n$results=@(); foreach($file in Get-ChildItem -LiteralPath $PSScriptRoot -Filter '*.json') { try { [void](Read-WorldSequenceDocument $file.FullName); $ok=$true } catch { $ok=$false }; $results += [pscustomobject]@{name=$file.BaseName;valid=$ok} }; ConvertTo-Json -InputObject @($results) -Compress"
            script_path = folder / "validate.ps1"
            script_path.write_text(script, encoding="utf-8-sig")
            result = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(script_path)],
                                    capture_output=True, text=True, timeout=45)
            self.assertEqual(0, result.returncode, result.stderr)
            actual = {row["name"]: row["valid"] for row in json.loads(result.stdout)}
            self.assertEqual({name: valid for name, _, valid in cases}, actual)

    def test_world_sequences_only_scope_preserves_other_layers_and_rolls_back(self):
        area = "LV_LUT_MIDNIGHTC_ED"
        source = json.loads(read(f"Data/Maps/Authoring/{area}/{area}.worldsequences.json"))
        card = next(row for row in source["instances"] if row["instanceId"] == "world.object.instance.kouku.card")
        source["instances"] = [card]
        source["templates"] = [row for row in source["templates"] if row["sequenceId"] == card["templateId"]]
        source["objectResources"] = [row for row in source["objectResources"] if row["objectId"] == "world.object.kouku.card"]
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            catalog = root / "Data/Maps/MapCatalog.json"
            catalog.parent.mkdir(parents=True)
            # Deliberately incomplete unrelated declarations must not block this scope.
            catalog.write_text(json.dumps({"areas": [{"id": area, "sourceSequences": f"Data/Maps/Authoring/{area}/{area}.worldsequences.json",
                "sequences": f"Client/Bin/DataFiles/Map/{area}.worldsequences.json", "sourceLights": "unrelated.invalid"}]}), encoding="utf-8")
            authoring = root / f"Data/Maps/Authoring/{area}/{area}.worldsequences.json"
            authoring.parent.mkdir(parents=True)
            authoring.write_text(json.dumps(source), encoding="utf-8")
            runtime = root / "Client/Bin/DataFiles/Map"
            runtime.mkdir(parents=True)
            preserved = {}
            for suffix in ("maplights.json", "mapplacements", "mapmaterials.json", "camerashots.json"):
                path = runtime / f"{area}.{suffix}"; path.write_bytes(b"preserved unrelated runtime\r\n"); preserved[path] = path.read_bytes()
            output = runtime / f"{area}.worldsequences.json"
            output.write_bytes(b"previous sequence runtime")
            def run(mode, fail=0):
                return subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                    str(ROOT / "Tools/MapPipeline/Publish-MapAuthoring.ps1"), "-ProjectRoot", str(root), "-AreaId", area,
                    "-Scope", "WorldSequences", "-Mode", mode, "-FailureAfterPromote", str(fail)], capture_output=True, text=True, timeout=30)
            self.assertEqual(0, run("Validate").returncode)
            self.assertEqual(b"previous sequence runtime", output.read_bytes())
            failed = run("Publish", 1)
            self.assertNotEqual(0, failed.returncode)
            self.assertEqual(b"previous sequence runtime", output.read_bytes())
            applied = run("Publish")
            self.assertEqual(0, applied.returncode, applied.stderr)
            self.assertEqual(source, json.loads(output.read_bytes()))
            self.assertEqual(0, run("Check").returncode)
            self.assertTrue(all(path.read_bytes() == before for path, before in preserved.items()))
            prior = output.read_bytes()
            source["objectResources"][0]["defaultMotionInstanceId"] = "missing.motion"
            authoring.write_text(json.dumps(source), encoding="utf-8")
            self.assertNotEqual(0, run("Publish").returncode)
            self.assertEqual(prior, output.read_bytes())

    def test_sequence_publish_joins_installed_or_staged_map_and_deploy_targets(self):
        area = "TEST_SEQUENCE_JOIN"
        source = json.loads(read("Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json"))
        instance = next(row for row in source["instances"] if len(row["bindings"]) == 1 and row["bindings"][0]["targetKind"] == "MAP_PLACEMENT")
        template = next(row for row in source["templates"] if row["sequenceId"] == instance["templateId"])
        instance.pop("walkableSurface", None)
        instance["bindings"][0]["targetId"] = "2"
        source.update(areaId=area, instances=[instance], templates=[template], objectResources=[])
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            authoring = root / f"Data/Maps/Authoring/{area}"
            imported = root / f"Data/Maps/Imported/{area}"
            runtime = root / "Client/Bin/DataFiles/Map"
            for directory in (authoring, imported, runtime):
                directory.mkdir(parents=True)
            catalog = root / "Data/Maps/MapCatalog.json"
            entry = {"id": area, "catalogType": "single", "sourceSequences": f"Data/Maps/Authoring/{area}/{area}.worldsequences.json",
                     "sequences": f"Client/Bin/DataFiles/Map/{area}.worldsequences.json"}
            catalog.write_text(json.dumps({"areas": [entry]}), encoding="utf-8")
            sequence = authoring / f"{area}.worldsequences.json"
            sequence.write_text(json.dumps(source), encoding="utf-8")
            output = runtime / sequence.name
            output.write_bytes(b"previous sequence runtime")
            asset = lambda name: f'"{name}" "Fixture" "Map/{name}.wmodel" "Prototype_{name}" 1 1 1 Origin'
            placement = lambda number, name: f'{number} "editor.{number}" "fixture" "editor" "{name}" 0 0 0 0 0 0 1 1 1 1 1'
            def write_pair(folder, names, ids):
                (folder / f"{area}.mapassets").write_text(f'LOSTARK_MAP_ASSET_CATALOG 1 "{area}" {len(names)}\n' + "\n".join(map(asset, names)) + "\n", encoding="utf-8")
                (folder / f"{area}.mapplacements").write_text(f'LOSTARK_MAP_PLACEMENTS 2 "{area}" {len(ids)}\n' + "\n".join(placement(number, name) for number, name in ids) + "\n", encoding="utf-8")
            write_pair(runtime, ["OLD"], [(1, "OLD")])
            write_pair(imported, ["OLD", "NEW"], [(1, "OLD"), (2, "NEW")])
            (authoring / f"{area}.mapplacements").write_bytes((imported / f"{area}.mapplacements").read_bytes())
            def run(scope, mode="Publish"):
                return subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                    str(ROOT / "Tools/MapPipeline/Publish-MapAuthoring.ps1"), "-ProjectRoot", str(root), "-AreaId", area,
                    "-Scope", scope, "-Mode", mode], capture_output=True, text=True, timeout=30)
            snapshot = lambda: {path.name: path.read_bytes() for path in runtime.iterdir() if path.is_file()}
            def reject(expected):
                before = snapshot()
                result = run("WorldSequences")
                self.assertNotEqual(0, result.returncode)
                self.assertIn(expected, result.stderr)
                self.assertEqual(before, snapshot(), "A rejected join must preserve every installed file")
            reject("Invalid Map binding")
            before = snapshot()
            result = run("Area", "Validate")
            self.assertEqual(0, result.returncode, result.stderr)
            self.assertEqual(before, snapshot())
            result = run("Area")
            self.assertEqual(0, result.returncode, result.stderr)
            self.assertEqual(source, json.loads(output.read_bytes()))
            self.assertEqual(0, run("WorldSequences", "Check").returncode)
            write_pair(runtime, ["OLD"], [(1, "OLD"), (2, "NEW")])
            reject("Invalid world sequence Map target placement/asset")
            # Sky is specifically excluded by native Collect_Placements.
            sky = asset("NEW") + ' "group" "Group" "evidence" Sky Back 1 1 0 0 1 0 1 50 1 1 1 1'
            (runtime / f"{area}.mapassets").write_text(f'LOSTARK_MAP_ASSET_CATALOG 3 "{area}" 1\n{sky}\n', encoding="utf-8")
            (runtime / f"{area}.mapplacements").write_text(f'LOSTARK_MAP_PLACEMENTS 2 "{area}" 1\n{placement(2, "NEW")}\n', encoding="utf-8")
            reject("Invalid Map binding")
            self.assertEqual(0, run("Area").returncode)
            # Single-scope joins must also consume the installed shard set.
            (runtime / f"{area}.mapset").write_text(f'LOSTARK_MAP_SHARD_SET 1 "{area}" 1\n"one" "{area}.mapassets" "{area}.mapplacements" 2 2\n', encoding="utf-8")
            entry["catalogType"] = "shard-set"
            catalog.write_text(json.dumps({"areas": [entry]}), encoding="utf-8")
            self.assertEqual(0, run("WorldSequences", "Validate").returncode)
            entry["catalogType"] = "single"
            catalog.write_text(json.dumps({"areas": [entry]}), encoding="utf-8")
            # Deploy roles are not the complete native clip list: an arbitrary
            # named animation remains valid here and is checked by CModel later.
            instance["bindings"][0].update(targetKind="DEPLOY_PLACEMENT", targetId="11")
            template["tracks"] = []
            template["animationTracks"] = [{"slotId": instance["bindings"][0]["slotId"], "clipName": "additional.native.clip",
                "playbackRate": 1, "loop": False, "holdLastFrame": True}]
            sequence.write_text(json.dumps(source), encoding="utf-8")
            deploy_asset = '"ANIMATED" ANIM "Fixture" "Map/animated.wmodel" "Prototype_Animated" "" "" 1 0 "fixture" "on" "off"'
            deploy_catalog = f'LOSTARK_DEPLOY_PROP_CATALOG 3 "{area}" 1\n{deploy_asset}\n'
            (imported / f"{area}.deployassets").write_text(deploy_catalog, encoding="utf-8")
            (runtime / f"{area}.deployassets").write_text(deploy_catalog, encoding="utf-8")
            deploy_row = '11 0 0 "editor.11" "ANIMATED" 0 0 0 0 0 0 1 1 0 0 0 PROJECT_AUTHORED'
            deploy_source = f'LOSTARK_DEPLOY_PROP_PLACEMENTS 2 "{area}" 1\n{deploy_row}\n'
            (authoring / f"{area}.deployplacements").write_text(deploy_source, encoding="utf-8")
            (runtime / f"{area}.deployplacements").write_text(f'LOSTARK_DEPLOY_PROP_PLACEMENTS 2 "{area}" 0\n', encoding="utf-8")
            reject("Invalid animated Deploy binding")
            result = run("Area")
            self.assertEqual(0, result.returncode, result.stderr)
            self.assertEqual(0, run("WorldSequences", "Check").returncode)
            (runtime / f"{area}.deployplacements").write_text(deploy_source.replace('"ANIMATED"', '"ABSENT"'), encoding="utf-8")
            reject("Deploy placement identity is invalid")
            (runtime / f"{area}.deployplacements").write_text(deploy_source, encoding="utf-8")
            (runtime / f"{area}.deployassets").write_text(deploy_catalog.replace(' ANIM ', ' STATIC ').replace('"on" "off"', '"" ""'), encoding="utf-8")
            reject("Invalid animated Deploy binding")

    def test_camera_pattern_authoring_fields_validate_without_changing_legacy_shots(self):
        source = json.loads(read("Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json"))
        source["shots"] = [source["shots"][0]]
        cases = [("legacy", copy.deepcopy(source), True)]
        shot = source["shots"][0]
        shot.update(displayName="패턴 카메라", defaultHoldMs=3000, transitionEasing="LINEAR", activation="PATTERN_ONLY")
        cases.append(("pattern", copy.deepcopy(source), True))
        for name, fields in (
            ("name_number", {"displayName": 7}), ("name_empty", {"displayName": ""}),
            ("name_too_long", {"displayName": "가" * 43}), ("name_null", {"displayName": None}),
            ("bad_hold", {"defaultHoldMs": -1}), ("fraction_hold", {"defaultHoldMs": 1.5}),
            ("long_entry_hold", {"defaultHoldMs": 600000, "blendInMs": 1}),
            ("zero_entry_hold", {"defaultHoldMs": 0, "blendInMs": 0}),
            ("bad_easing", {"transitionEasing": "HOLD"}), ("easing_type", {"transitionEasing": True}),
            ("bad_activation", {"activation": "pattern_only"}), ("activation_null", {"activation": None}),
        ):
            invalid = copy.deepcopy(source); invalid["shots"][0].update(fields)
            cases.append((name, invalid, False))
        publisher = read("Tools/MapPipeline/Publish-MapAuthoring.ps1")
        definitions = []
        for name in ("Test-JsonNumber", "Assert-ExactJsonProperties", "Read-CameraShotDocument"):
            match = re.search(r"(?m)^function " + re.escape(name) + r" \{.*?^\}", publisher, re.DOTALL)
            self.assertIsNotNone(match); definitions.append(match.group(0))
        with tempfile.TemporaryDirectory() as temporary:
            folder = Path(temporary)
            for name, document, _ in cases:
                (folder / (name + ".json")).write_text(json.dumps(document), encoding="utf-8")
            script = "$ErrorActionPreference='Stop'\n$AreaId='LV_LUT_MIDNIGHTC_ED'\n" + "\n".join(definitions)
            script += "\n$results=@(); foreach($file in Get-ChildItem -LiteralPath $PSScriptRoot -Filter '*.json') { try { [void](Read-CameraShotDocument $file.FullName); $ok=$true } catch { $ok=$false }; $results += [pscustomobject]@{name=$file.BaseName;valid=$ok} }; ConvertTo-Json -InputObject @($results) -Compress"
            script_path = folder / "validate.ps1"; script_path.write_text(script, encoding="utf-8-sig")
            result = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(script_path)], capture_output=True, text=True, timeout=30)
            self.assertEqual(0, result.returncode, result.stderr)
            self.assertEqual({name: valid for name, _, valid in cases}, {r["name"]: r["valid"] for r in json.loads(result.stdout)})

    def test_client_project_registers_each_source_once(self) -> None:
        project = ET.parse(ROOT / "Client/Default/Client.vcxproj")
        namespace = {"msb": "http://schemas.microsoft.com/developer/msbuild/2003"}
        includes = [
            item.attrib.get("Include", "")
            for kind in ("ClInclude", "ClCompile")
            for item in project.findall(f".//msb:{kind}", namespace)
        ]
        expected = (
            r"..\Public\WorldSequenceDocument.h",
            r"..\Public\WorldSequenceToolPanel.h",
            r"..\Private\WorldSequenceDocument.cpp",
            r"..\Private\WorldSequenceToolPanel.cpp",
        )
        for path in expected:
            self.assertEqual(1, includes.count(path), path)
        panel_compile = project.find(
            ".//msb:ClCompile[@Include='..\\Private\\WorldSequenceToolPanel.cpp']",
            namespace,
        )
        self.assertIsNotNone(panel_compile)
        options = " ".join(
            value.text or ""
            for value in panel_compile.findall("msb:AdditionalOptions", namespace)
        )
        self.assertIn("/utf-8", options)

    def test_document_is_strict_versioned_and_transactional(self) -> None:
        self.assertIn('SCHEMA = "lostark.world-sequences"', self.document_cpp)
        self.assertIn("FORMAT_VERSION = 1", self.document_cpp)
        self.assertIn("Is_ExactObject", self.document_cpp)
        self.assertIn("parse -> exact schema validation", read(
            ".md/GB/08-31/2026-08-31_WORLD_SEQUENCE_MAP_TOOL_PLAN.md"
        ))
        self.assertIn("ReplaceFileW", self.document_cpp)
        self.assertIn("MoveFileExW", self.document_cpp)
        self.assertIn("writeSucceeded = writeSucceeded && !output.fail()", self.document_cpp)

    def test_document_reads_are_bounded_and_display_text_is_safe_utf8(self) -> None:
        self.assertIn("MAX_DOCUMENT_BYTES = 16u * 1024u * 1024u", self.document_cpp)
        self.assertIn("std::filesystem::file_size", self.document_cpp)
        self.assertIn("World sequence document exceeds the 16 MiB parse limit", self.document_cpp)
        self.assertIn("catch (const std::bad_alloc&)", self.document_cpp)
        self.assertIn("input.gcount()", self.document_cpp)
        self.assertIn("input.peek()", self.document_cpp)
        self.assertIn("Is_ValidUtf8DisplayText(value.displayName)", self.document_cpp)
        self.assertIn("Is_ValidUtf8DisplayText(value.category)", self.document_cpp)

    def test_document_rejects_invalid_refs_enums_and_runtime_scales(self) -> None:
        self.assertIn("availablePlacements.find(targetId)", self.document_cpp)
        self.assertIn("availableDeployPlacements.find(targetId)", self.document_cpp)
        self.assertIn(
            "WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT != binding.targetKind",
            self.document_cpp,
        )
        self.assertIn(
            "WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT == binding.targetKind",
            self.document_cpp,
        )
        self.assertIn("animationTargetSupported", self.document_cpp)
        self.assertIn("WORLD_SEQUENCE_INTERPOLATION::LINEAR != value.interpolation", self.document_cpp)
        self.assertIn("MIN_RUNTIME_SCALE_DETERMINANT", self.document_cpp)
        self.assertIn("std::isfinite(composedX)", self.document_cpp)
        self.assertIn("Sequence scale would create a singular map transform", self.document_cpp)
        self.assertIn("boundTargets.insert(uniqueTarget).second", self.document_cpp)
        self.assertIn("sequenceTargetSupported", self.document_cpp)
        self.assertIn("MAP_ASSET_RENDER_MODE::BACKGROUND", self.panel_cpp)

    def test_preview_uses_runtime_baseline_and_never_edits_map_records(self) -> None:
        self.assertIn("Try_GetRuntimeVisible", self.panel_cpp)
        self.assertIn("baseline->runtimeVisible || !baseline->baseline.visible", self.panel_cpp)
        self.assertIn("restoredRecord.visible = target.runtimeVisible", self.panel_cpp)
        self.assertIn("Stop_AndRestore", self.panel_cpp)
        self.assertNotRegex(self.panel_cpp, r"entry\.record\s*=")
        runtime_h = read("Client/Public/MapPlacementRuntime.h")
        batch_h = read("Client/Public/MapStaticBatchObject.h")
        self.assertIn("Try_GetRuntimeVisible", runtime_h)
        self.assertIn("Try_GetInstanceVisible", batch_h)

    def test_paused_live_edit_is_validated_then_resampled(self) -> None:
        update = re.search(
            r"void Client::CWorldSequenceToolPanel::Update\(.*?\n\}",
            self.panel_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(update)
        body = update.group(0)
        self.assertIn("m_bPreviewNeedsRefresh", body)
        self.assertIn("Validate(catalog, placements, deployRuntime, validation)", body)
        self.assertIn("Apply_Preview", body)
        self.assertLess(
            body.index("Validate(catalog, placements, deployRuntime, validation)"),
            body.index("Apply_Preview"),
        )
        self.assertIn("if (m_bPreviewActive)\n\t\tm_bPreviewNeedsRefresh = true", self.panel_cpp)

    def test_selection_cannot_orphan_an_active_preview(self) -> None:
        template_list = re.search(
            r"void Client::CWorldSequenceToolPanel::Render_TemplateList\(\).*?\n\}",
            self.panel_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(template_list)
        self.assertIn("ImGui::BeginDisabled(m_bPreviewActive)", template_list.group(0))
        self.assertIn("Is_PreviewActive", self.panel_h)
        self.assertIn("sequenceOwnsPreviewTargets", self.map_tool_cpp)
        self.assertIn("연출 미리보기를 Stop / Restore한 뒤", self.map_tool_cpp)

    def test_map_and_sequence_save_share_validation_and_rollback(self) -> None:
        self.assertIn("Save_PlacementsAndWorldSequences", self.map_tool_h)
        self.assertIn("PrepareAuthoringBackup", self.map_tool_cpp)
        self.assertIn("RestoreAuthoringBackup", self.map_tool_cpp)
        self.assertIn("WriteAuthoringTransactionMarker", self.map_tool_cpp)
        self.assertIn("RecoverAuthoringTransaction", self.map_tool_cpp)
        self.assertIn("Linked save verification failed", self.map_tool_cpp)
        save_all = re.search(
            r"bool_t Client::CMapTool::Save_AllAuthoring\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(save_all)
        body = save_all.group(0)
        self.assertIn("m_pWorldSequenceToolPanel->Validate", body)
        self.assertIn("Save_PlacementsAndWorldSequences", body)
        toolbar = re.search(
            r"void Client::CMapTool::Render_Toolbar\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(toolbar)
        self.assertIn('ImGui::Button("Save")', toolbar.group(0))
        self.assertIn("Save_AllAuthoring()", toolbar.group(0))

    def test_linked_save_uses_area_lock_and_stale_source_cas(self) -> None:
        self.assertIn("SCOPED_AUTHORING_SAVE_LOCK", self.map_tool_cpp)
        self.assertIn(".linked-save.lock", self.map_tool_cpp)
        self.assertIn("FILE_FLAG_DELETE_ON_CLOSE", self.map_tool_cpp)
        self.assertIn("GetLastError()", self.map_tool_cpp)
        self.assertIn("ERROR_SHARING_VIOLATION", self.map_tool_cpp)
        self.assertIn("Could not open Area authoring lock", self.map_tool_cpp)
        self.assertIn("Matches_LinkedSourceBaseline", self.panel_h)
        self.assertIn("m_PlacementBaselineBytes", self.panel_h)
        self.assertIn("m_SequenceBaselineBytes", self.panel_h)
        self.assertIn("Save conflict: linked map/sequence source changed after Reload", self.panel_cpp)
        linked_save = re.search(
            r"bool_t Client::CMapTool::Save_PlacementsAndWorldSequences\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(linked_save)
        body = linked_save.group(0)
        self.assertLess(body.index("authoringLock.Acquire"),
                        body.index("Matches_LinkedSourceBaseline"))
        self.assertLess(body.index("Matches_LinkedSourceBaseline"),
                        body.index("PrepareAuthoringBackup"))

        map_save = re.search(
            r"bool_t Client::CMapTool::Save_Placements\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(map_save)
        map_body = map_save.group(0)
        self.assertIn("return Save_PlacementsAndWorldSequences()", map_body)
        self.assertLess(map_body.index("return Save_PlacementsAndWorldSequences()"),
                        map_body.index("authoringLock.Acquire"))
        self.assertNotIn("Refresh_LinkedSourceBaseline", map_body)

    def test_linked_save_verifies_exact_intended_content(self) -> None:
        self.assertIn("AreExactlySamePlacementRecords", self.map_tool_cpp)
        self.assertIn("Has_SameDocument", self.panel_h)
        self.assertIn("Is_Equivalent", self.document_h)
        self.assertIn("Linked save verification found different Map Placement content", self.map_tool_cpp)
        self.assertIn("Linked save verification found different World Sequence content", self.map_tool_cpp)
        self.assertIn("stableVerifiedPlacements", self.map_tool_cpp)
        self.assertIn("return left == right", self.map_tool_cpp)
        self.assertIn("return left == right", self.document_cpp)
        self.assertIn("outStoredRecords", read("Client/Public/MapPlacementDocument.h"))
        placement_document = read("Client/Private/MapPlacementDocument.cpp")
        self.assertIn("UNIT_QUATERNION_TOLERANCE", placement_document)
        self.assertIn("std::abs(length - 1.f) <=", placement_document)
        self.assertIn("Linked save verification source changed during final read", self.map_tool_cpp)
        self.assertIn("RestoreAuthoringBackup", self.map_tool_cpp)
        self.assertIn("MAX_TRANSACTION_MARKER_BYTES = 512u", self.map_tool_cpp)
        self.assertIn("transaction marker exceeds its bounded read limit", self.map_tool_cpp)
        linked_save = re.search(
            r"bool_t Client::CMapTool::Save_PlacementsAndWorldSequences\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(linked_save)
        body = linked_save.group(0)
        self.assertIn("Adopt_VerifiedLinkedSourceBaseline", body)
        self.assertLess(body.rindex("ClearAuthoringTransactionMarker"),
                        body.index("Adopt_VerifiedLinkedSourceBaseline"))
        self.assertIn("editor baseline stayed unchanged", body)

    def test_reload_stages_placements_and_sequences_before_commit(self) -> None:
        reload_body = re.search(
            r"bool_t Client::CMapTool::Load_Placements\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(reload_body)
        body = reload_body.group(0)
        self.assertLess(body.index("authoringLock.Acquire"),
                        body.index("RecoverAuthoringTransactionUnderLock"))
        self.assertLess(body.index("RecoverAuthoringTransactionUnderLock"),
                        body.index("CMapPlacementDocument::Read"))
        self.assertLess(body.index("CMapPlacementDocument::Read"),
                        body.index("stagedWorldSequencePanel->Load_Area"))
        self.assertIn("stableLinkedDocument", body)
        self.assertIn("Matches_LinkedSourceBaseline", body)
        self.assertLess(body.index("stagedWorldSequencePanel->Load_Area"),
                        body.index("Remove_PlacementRuntime(m_Placements"))
        self.assertLess(body.index("Stage_PlacementRuntime"),
                        body.index("Remove_PlacementRuntime(m_Placements"))
        self.assertIn("Is_PreviewActive", body)
        render = re.search(
            r"void Client::CMapTool::Render_WorldSequencePanel\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(render)
        self.assertIn("Consume_ReloadAllRequest", render.group(0))
        self.assertIn("Load_Placements()", render.group(0))
        self.assertLess(render.group(0).index("Load_Placements()"),
                        render.group(0).index("return;", render.group(0).index("Load_Placements()")))

    def test_initial_area_load_recovers_an_interrupted_linked_save(self) -> None:
        switch = re.search(
            r"bool_t Client::CMapTool::Switch_EditorArea\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(switch)
        body = switch.group(0)
        self.assertIn("worldSequencePath", body)
        self.assertLess(body.index("authoringLock.Acquire"),
                        body.index("RecoverAuthoringTransactionUnderLock"))
        self.assertLess(body.index("RecoverAuthoringTransactionUnderLock"),
                        body.index("CMapPlacementDocument::Read"))
        self.assertLess(body.index("CMapPlacementDocument::Read"),
                        body.index("stagedWorldSequencePanel->Load_Area"))
        self.assertIn("stableLinkedRecords", body)
        self.assertIn("Matches_LinkedSourceBaseline", body)

    def test_ui_rejects_duplicate_and_background_bindings(self) -> None:
        self.assertIn("A map object can be bound to only one target slot", self.panel_cpp)
        self.assertIn("IsSequenceTargetSupported", self.panel_cpp)
        self.assertIn("[Background - unavailable]", self.panel_cpp)
        self.assertIn("sequenceTargetSupported", self.document_h)

    def test_loaded_selection_keeps_template_and_instance_coherent(self) -> None:
        load = re.search(
            r"bool_t Client::CWorldSequenceToolPanel::Load_Area\(.*?\n\}",
            self.panel_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(load)
        body = load.group(0)
        self.assertIn("m_SelectedInstanceId", body)
        self.assertIn("m_Document.Get_Instances().front().templateId", body)
        self.assertIn("placementBefore", body)
        self.assertIn("placementAfter", body)
        self.assertIn("Linked map/sequence source changed while the Area was loading", body)

    def test_visible_labels_are_english_and_help_is_korean(self) -> None:
        for label in (
            "New Sequence",
            "Sequence List",
            "Placed Instances",
            "Map Objects",
            "Add Target Track",
            "Add Key at Preview Time",
            "Stop / Restore",
        ):
            self.assertIn(label, self.panel_cpp)
        self.assertIn("ShowKoreanHelp", self.panel_cpp)
        self.assertIn("새 재사용 연출 템플릿을 만듭니다", self.panel_cpp)
        self.assertIn("미리보기는 원본 맵 배치를 수정하지 않으며", self.panel_cpp)

    def test_product_runtime_boundary_is_explicit(self) -> None:
        self.assertIn("Authoring preview only; product runtime publish is separate", self.panel_cpp)
        self.assertIn("서버 상호작용·제품 재생·길 개방은 아직 연결되지 않았습니다", self.panel_cpp)


class AnimatedPropAuthoringContractTests(unittest.TestCase):
    """MapTool owns creating the Deploy ANIM placements a sequence binds to."""

    def setUp(self) -> None:
        self.map_tool_h = read("Client/Public/MapTool.h")
        self.map_tool_cpp = read("Client/Private/MapTool.cpp")
        self.catalog_h = read("Client/Public/DeployPropCatalog.h")
        self.catalog_cpp = read("Client/Private/DeployPropCatalog.cpp")

    def test_every_map_tool_declaration_has_a_definition(self) -> None:
        declared = set()
        for match in re.finditer(
            r"\b([A-Za-z_]\w*)\s*\([^;{}]*\)\s*(?:const\s*)?(?:noexcept\s*)?;",
            self.map_tool_h,
            flags=re.DOTALL,
        ):
            declared.add(match.group(1))
        defined = set(re.findall(r"CMapTool::([A-Za-z_]\w*)", self.map_tool_cpp))
        ignored = {"ETOI", "ETOUI", "float2_t", "float3_t", "float4_t"}
        missing = sorted(declared - defined - ignored)
        self.assertEqual([], missing)

    def test_animated_prop_authoring_is_reachable_from_the_sequence_mode(self) -> None:
        panel = re.search(
            r"void Client::CMapTool::Render_WorldSequencePanel\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(panel)
        self.assertIn("Render_AnimatedPropsAuthoring();", panel.group(0))
        for label in (
            "Animated Props (Deploy ANIM)",
            "Place In Viewport",
            "Apply Transform",
            "Remove Placement",
            "Save Animated Props",
        ):
            self.assertIn(label, self.map_tool_cpp)

    def test_armed_viewport_click_places_only_in_sequence_mode(self) -> None:
        interaction = re.search(
            r"void Client::CMapTool::Update_WorldInteraction\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(interaction)
        body = interaction.group(0)
        self.assertIn(
            "if (m_bAnimatedPropPlacementArmed && mousePressed)\n"
            "\t\t\t(void)Try_PlaceSelectedDeploy();",
            body,
        )
        self.assertIn("m_bAnimatedPropPlacementArmed = false;", body)
        consumes = re.search(
            r"bool_t Client::CMapTool::ConsumesWorldLeftMouse\(\) const.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(consumes)
        self.assertIn("m_bAnimatedPropPlacementArmed", consumes.group(0))

    def test_project_authored_placement_ids_stay_in_the_editor_domain(self) -> None:
        allocate = re.search(
            r"uint64_t Client::CMapTool::Allocate_AnimatedPropPlacementId\(\) const.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(allocate)
        body = allocate.group(0)
        self.assertEqual(
            2, body.count("CMapPlacementDocument::MAX_EDITOR_PLACEMENT_ID")
        )
        self.assertIn("0u : candidate", body)
        # A project row may never carry extractor-only evidence fields.
        self.assertIn("0u == row.deployActorId && 0u == row.propDefinitionId", self.catalog_cpp)
        self.assertIn("PROJECT_AUTHORED", self.catalog_h)

    def test_deploy_placement_save_is_atomic_and_read_back_verified(self) -> None:
        self.assertIn("MoveFileExW", self.catalog_cpp)
        self.assertIn("MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH", self.catalog_cpp)
        save = re.search(
            r"bool_t Client::CMapTool::Save_DeployPlacements\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(save)
        body = save.group(0)
        self.assertIn("CDeployPropCatalog verification;", body)
        self.assertIn("DeployProp placement save verification found different content", body)
        self.assertLess(body.index("verification.Load"), body.index("m_bDeployDirty = false"))

    def test_unsaved_deploy_authoring_blocks_and_saves_before_sequences(self) -> None:
        unsaved = re.search(
            r"bool_t Client::CMapTool::Has_UnsavedAuthoring\(\) const.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(unsaved)
        self.assertIn("m_bDeployDirty", unsaved.group(0))
        save_all = re.search(
            r"bool_t Client::CMapTool::Save_AllAuthoring\(\).*?\n\treturn true;\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(save_all)
        body = save_all.group(0)
        self.assertIn("if (m_bDeployDirty && !Save_DeployPlacements())", body)
        self.assertLess(
            body.index("Save_DeployPlacements()"),
            body.index("Save_PlacementsAndWorldSequences()"),
        )

    def test_removing_a_bound_animated_prop_rolls_the_runtime_back(self) -> None:
        remove = re.search(
            r"bool_t Client::CMapTool::Remove_SelectedAnimatedProp\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(remove)
        body = remove.group(0)
        self.assertIn("CDeployPropCatalog restore = m_DeployRuntime.Get_Catalog();", body)
        self.assertIn("m_pWorldSequenceToolPanel->Validate(", body)
        self.assertIn("Commit_DeployCatalog(std::move(restore)", body)
        self.assertLess(body.index("Validate("), body.index("std::move(restore)"))
        self.assertIn("Source-extracted Deploy placements cannot be removed", body)

    def test_commit_releases_preview_seams_before_rebuilding_the_runtime(self) -> None:
        commit = re.search(
            r"bool_t Client::CMapTool::Commit_DeployCatalog\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(commit)
        body = commit.group(0)
        self.assertIn("Stop_AndRestore(", body)
        self.assertIn("Restore_DestructionPreview();", body)
        self.assertIn("m_pDestructionSimulationController->Clear();", body)
        self.assertLess(body.index("Stop_AndRestore("), body.index("stagedRuntime.Load("))
        self.assertLess(
            body.index("stagedRuntime.Load("),
            body.index("m_DeployRuntime = std::move(stagedRuntime);"),
        )

    def test_korean_help_remains_valid_escaped_utf8(self) -> None:
        project = ET.parse(ROOT / "Client/Default/Client.vcxproj")
        namespace = {"msb": "http://schemas.microsoft.com/developer/msbuild/2003"}
        map_tool = project.find(
            ".//msb:ClCompile[@Include='..\\Private\\MapTool.cpp']", namespace
        )
        self.assertIsNotNone(map_tool)
        # MapTool now explicitly compiles as UTF-8; escaped help remains valid.

        blocks = re.findall(
            r"static const char_t\* const (ANIMATED_PROP_HELP_\w+)\s*=\s*"
            r"((?:\s*\"(?:[^\"\\]|\\.)*\")+)\s*;",
            self.map_tool_cpp,
        )
        self.assertEqual(5, len(blocks))
        for name, body in blocks:
            data = bytearray()
            for part in re.findall(r"\"((?:[^\"\\]|\\.)*)\"", body):
                index = 0
                while index < len(part):
                    if part[index] == "\\" and part[index + 1] == "x":
                        data.append(int(part[index + 2:index + 4], 16))
                        index += 4
                    elif part[index] == "\\":
                        data.append(ord(part[index + 1]))
                        index += 2
                    else:
                        data.append(ord(part[index]))
                        index += 1
                # An \xHH escape followed by another hex digit would swallow it.
                self.assertIsNone(
                    re.search(r"\\x[0-9A-Fa-f]{2}[0-9A-Fa-f]", part), name
                )
            text = data.decode("utf-8")
            self.assertTrue(any("\uac00" <= ch <= "\ud7a3" for ch in text), name)



if __name__ == "__main__":
    unittest.main(verbosity=2)

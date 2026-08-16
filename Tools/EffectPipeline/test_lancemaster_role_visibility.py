import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]
MANIFEST_PATH = (
    ROOT
    / "Data/Effects/AuthoredCorrections/LanceMaster/"
    / "LanceMaster.q-w-e-r-a-s.user-role-composition.json"
)
PROJECTOR_PATH = ROOT / "Tools/EffectPipeline/project_manifest_role_visibility.py"
RESOURCES_ROOT = ROOT / "Client/Bin/Resources"
EXPECTED_DOCUMENTS = {
    "Data/Effects/Authored/effect.lancemaster.skill.34040.clip1.unified.effect.json",
    "Data/Effects/Authored/effect.lancemaster.skill.34040.clip2.unified.effect.json",
    "Data/Effects/Authored/effect.lancemaster.skill.34090.unified.effect.json",
    "Data/Effects/Authored/effect.lancemaster.skill.34100.clip1.unified.effect.json",
    "Data/Effects/Authored/effect.lancemaster.skill.34100.clip2.unified.effect.json",
    "Data/Effects/Authored/effect.lancemaster.skill.34100.clip3.unified.effect.json",
    "Data/Effects/Authored/effect.lancemaster.skill.34120.clip1.unified.effect.json",
    "Data/Effects/Authored/effect.lancemaster.skill.34120.clip2.unified.effect.json",
    "Data/Effects/Authored/effect.lancemaster.skill.34120.clip3.unified.effect.json",
    "Data/Effects/Authored/effect.lancemaster.skill.34140.ba1.clip1.unified.effect.json",
    "Data/Effects/Authored/effect.lancemaster.skill.34140.ba1.clip2.unified.effect.json",
    "Data/Effects/Authored/effect.lancemaster.skill.34140.ba2.clip2.unified.effect.json",
    "Data/Effects/Authored/effect.lancemaster.skill.34160.ba1.unified.effect.json",
    "Data/Effects/Authored/effect.lancemaster.skill.34160.ba2.unified.effect.json",
}


def load_json(path):
    return json.loads(Path(path).read_text(encoding="utf-8-sig"))


def load_projector():
    spec = importlib.util.spec_from_file_location("role_visibility_projector", PROJECTOR_PATH)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


projector = load_projector()


def selected_ids(skill):
    return {
        stable_id
        for role in skill["roles"]
        for stable_id in role["stableIds"]
    }


def clip_name(value):
    return value["clip"] if isinstance(value, dict) else value


def source_event_number(element):
    value = element.get("sourcePresentation", {}).get("sourceEventId", "")
    if isinstance(value, str) and value.startswith("source-event-"):
        return int(value.removeprefix("source-event-"))
    return 1_000_000


def physical_asset_ids(element):
    assets = [
        row.get("assetId", "")
        for row in element.get("resources", [])
        if row.get("assetId")
    ]
    assets.extend(
        row.get("assetId", "")
        for row in element.get("material", {})
        .get("sourceProfile", {})
        .get("textures", [])
        if row.get("assetId")
    )
    return assets


class LanceMasterRoleVisibilityTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.manifest = load_json(MANIFEST_PATH)
        cls.skills = cls.manifest["skills"]

    def test_exact_long_spear_scope_and_authority_mapping(self):
        self.assertEqual("lostark.effect-role-composition", self.manifest["schema"])
        self.assertEqual(1, self.manifest["formatVersion"])
        self.assertEqual("LANCE_MASTER", self.manifest["characterClass"])
        self.assertEqual(
            EXPECTED_DOCUMENTS,
            {skill["effectDocument"] for skill in self.skills},
        )
        self.assertEqual(14, len(self.skills))
        for path in EXPECTED_DOCUMENTS:
            self.assertNotIn(".skill.34110.", path)
            self.assertNotIn(".skill.34150.", path)
            self.assertNotIn(".skill.34010.", path)

        player_skills = load_json(ROOT / "Data/Balance/PlayerSkills.json")["skills"]
        expected_by_slot = {
            "Q": 34040,
            "W": 34090,
            "E": 34100,
            "R": 34160,
            "A": 34140,
            "S": 34120,
        }
        for slot, skill_id in expected_by_slot.items():
            matches = [
                row
                for row in player_skills
                if row.get("characterClass") == "LANCE_MASTER"
                and row.get("inputSlot") == slot
                and row.get("requiredStance") == "LANCE_MASTER_LONG_SPEAR"
            ]
            self.assertEqual([skill_id], [int(row["skillId"]) for row in matches])

        bindings = load_json(
            ROOT / "Data/Animation/Authored/LanceMaster/LanceMaster.skillbindings.json"
        )["bindings"]
        clips_by_skill = {int(row["skillId"]): row["clips"] for row in bindings}
        for skill in self.skills:
            clips = clips_by_skill[skill["skillId"]]
            if "comboStage" in skill:
                clips = clips[skill["comboStage"] - 1]
            if "clipIndex" in skill:
                clips = [clips[skill["clipIndex"] - 1]]
            self.assertEqual(skill["clips"], [clip_name(value) for value in clips])

        animevents = (
            ROOT / "Data/Animation/Authored/LanceMaster/LanceMaster.animevents"
        ).read_text(encoding="utf-8-sig")
        for skill in self.skills:
            effect_asset_id = Path(skill["effectDocument"]).name.removesuffix(
                ".effect.json"
            )
            expected_row = (
                f'"{skill["clips"][0]}" EFFECT startms=0 '
                f'payload="{effect_asset_id}" effectref=asset'
            )
            self.assertEqual(1, animevents.count(expected_row), expected_row)

        locked = self.manifest["lockedUnboundDefinitions"]
        self.assertEqual(1, len(locked))
        self.assertEqual(
            {
                "inputSlot": "A",
                "skillId": 34140,
                "comboStage": 2,
                "clipIndex": 1,
                "clips": ["flm_sk_chestdestruction_03"],
                "effectDocument": None,
                "status": "UNBOUND_LOCK",
                "typedReason": "UNBOUND_NO_EXACT_CLIP_LOCAL_SOURCE_EFFECT",
            },
            locked[0],
        )
        self.assertNotIn(
            '"flm_sk_chestdestruction_03" EFFECT', animevents
        )

    def test_projected_visibility_classification_resources_and_totals(self):
        payloads, summary = projector.build_projection(
            ROOT, MANIFEST_PATH, require_projected=True
        )
        self.assertEqual({}, payloads)
        self.assertEqual(14, summary["targetDocumentCount"])
        self.assertEqual(444, summary["targetElementCount"])
        self.assertEqual(308, summary["selectedVisibleCount"])
        self.assertEqual(243, summary["selectedFullCount"])
        self.assertEqual(65, summary["selectedApproximateCount"])
        self.assertEqual(102, summary["hardLockedCount"])
        self.assertEqual(34, summary["hiddenUnpositionedCount"])
        self.assertEqual(0, summary["explicitRoleExcludedCount"])

        for skill in self.skills:
            document = load_json(ROOT / skill["effectDocument"])
            expected = selected_ids(skill)
            visible = {
                element["id"]
                for element in document["elements"]
                if element.get("visible") is True
            }
            self.assertEqual(expected, visible, skill["effectDocument"])
            for element in document["elements"]:
                classification = projector.execution_class(element)
                if element["id"] in expected:
                    self.assertIn(
                        classification, ("FULL", "AUTHORING_APPROXIMATE")
                    )
                    for asset_id in physical_asset_ids(element):
                        if asset_id.lower().endswith((".dds", ".wmodel")):
                            self.assertTrue(
                                (RESOURCES_ROOT / Path(asset_id)).is_file(), asset_id
                            )
                if classification == "HARD_FAIL_CLOSED":
                    self.assertFalse(element.get("visible"), element["id"])

    def test_source_system_timings_and_visual_evidence_are_pinned(self):
        receipts = {}
        for skill in self.skills:
            receipt = receipts.setdefault(
                skill["sourceReceipt"], load_json(ROOT / skill["sourceReceipt"])
            )
            events = {
                row["eventId"]: row for row in receipt["timeline"]["events"]
            }
            document = load_json(ROOT / skill["effectDocument"])
            by_id = {element["id"]: element for element in document["elements"]}
            for role in skill["roles"]:
                for stable_id in role["stableIds"]:
                    self.assertEqual(
                        role["sourceSystem"], projector.source_system(by_id[stable_id])
                    )
                for occurrence in role["sourceOccurrences"]:
                    event_id = occurrence["sourceEventId"]
                    if event_id is None:
                        self.assertEqual(
                            "SOURCE_SYSTEM_DERIVED_DECAL_NO_EVENT_ID",
                            occurrence["timingStatus"],
                        )
                        continue
                    source = events[event_id]
                    self.assertEqual(role["sourceSystem"], source["sourceSystemId"])
                    self.assertEqual(occurrence["clip"], source["clip"])
                    self.assertEqual(
                        occurrence["localTimeSeconds"], source["localTimeSeconds"]
                    )
                    self.assertEqual(
                        occurrence["globalTimeSeconds"], source["globalTimeSeconds"]
                    )

        evidence_root = ROOT / Path(
            self.manifest["visualEvidence"]["manifest"]
        ).parent
        for slot in self.manifest["visualEvidence"]["scope"]:
            for row in slot["files"]:
                matches = list(evidence_root.rglob(row["filename"]))
                self.assertEqual(1, len(matches), row["filename"])
                payload = matches[0].read_bytes()
                self.assertEqual(row["sha256"], hashlib.sha256(payload).hexdigest())

    def test_duplicate_and_explicit_hidden_provenance(self):
        hidden_count = 0
        duplicate_full = 0
        duplicate_approximate = 0
        explicit_count = 0
        for skill in self.skills:
            document = load_json(ROOT / skill["effectDocument"])
            by_id = {element["id"]: element for element in document["elements"]}
            for group in skill["duplicateGroups"]:
                self.assertEqual("HIDDEN_UNPOSITIONED", group["typedReason"])
                keep = by_id[group["keepStableId"]]
                self.assertTrue(keep["visible"])
                for stable_id in group["hiddenStableIds"]:
                    hidden = by_id[stable_id]
                    self.assertFalse(hidden["visible"])
                    self.assertEqual(
                        projector.normalized_duplicate_payload(keep),
                        projector.normalized_duplicate_payload(hidden),
                    )
                    self.assertLessEqual(
                        source_event_number(keep), source_event_number(hidden)
                    )
                    duplicate_full += (
                        projector.execution_class(hidden) == "FULL"
                    )
                    duplicate_approximate += (
                        projector.execution_class(hidden)
                        == "AUTHORING_APPROXIMATE"
                    )
                    hidden_count += 1
            for row in skill["explicitlyHiddenRoleRows"]:
                self.assertEqual(
                    "OUTSIDE_USER_ROLE_COMCUSTOMIZING", row["typedReason"]
                )
                self.assertFalse(by_id[row["stableId"]]["visible"])
                explicit_count += 1
        self.assertEqual(34, hidden_count)
        self.assertEqual(23, duplicate_full)
        self.assertEqual(11, duplicate_approximate)
        self.assertEqual(0, explicit_count)

    def test_generic_projection_is_visible_only_idempotent_and_transactional(self):
        with tempfile.TemporaryDirectory() as directory:
            temp_root = Path(directory)
            temp_manifest = temp_root / MANIFEST_PATH.relative_to(ROOT)
            temp_manifest.parent.mkdir(parents=True)
            shutil.copy2(MANIFEST_PATH, temp_manifest)
            for relative in EXPECTED_DOCUMENTS:
                destination = temp_root / relative
                destination.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(ROOT / relative, destination)

            first_skill = self.skills[0]
            first_path = temp_root / first_skill["effectDocument"]
            before = load_json(first_path)
            mutated = copy.deepcopy(before)
            selected = selected_ids(first_skill)
            selected_id = next(iter(selected))
            by_id = {element["id"]: element for element in mutated["elements"]}
            hard_id = next(
                stable_id
                for stable_id, element in by_id.items()
                if projector.execution_class(element) == "HARD_FAIL_CLOSED"
            )
            by_id[selected_id]["visible"] = False
            by_id[hard_id]["visible"] = True
            first_path.write_bytes(
                projector.encode_json_like(first_path.read_bytes(), mutated)
            )
            pre_projection = load_json(first_path)

            payloads, summary = projector.build_projection(
                temp_root, temp_manifest, require_projected=False
            )
            self.assertEqual(1, summary["changedDocumentCount"])
            self.assertEqual(2, summary["changedVisibilityCount"])
            self.assertEqual({first_path}, set(payloads))
            staged = json.loads(payloads[first_path].decode("utf-8-sig"))
            self.assertEqual(
                projector.document_without_visibility(pre_projection),
                projector.document_without_visibility(staged),
            )
            projector.write_payloads_transactionally(payloads)
            second_payloads, second_summary = projector.build_projection(
                temp_root, temp_manifest, require_projected=False
            )
            self.assertEqual({}, second_payloads)
            self.assertEqual(0, second_summary["changedVisibilityCount"])
            projector.build_projection(
                temp_root, temp_manifest, require_projected=True
            )

            first = temp_root / "rollback-first.json"
            second = temp_root / "rollback-second.json"
            first.write_bytes(b"first-before")
            second.write_bytes(b"second-before")
            original = {path: path.read_bytes() for path in (first, second)}
            calls = 0

            def fail_second(source, target):
                nonlocal calls
                calls += 1
                if calls == 2:
                    raise OSError("injected second commit failure")
                return Path(source).replace(target)

            with self.assertRaisesRegex(OSError, "injected second commit failure"):
                projector.write_payloads_transactionally(
                    {first: b"first-after", second: b"second-after"},
                    replace_func=fail_second,
                )
            self.assertEqual(original, {path: path.read_bytes() for path in original})


if __name__ == "__main__":
    unittest.main(verbosity=2)

import copy
import hashlib
import json
import shutil
import tempfile
import unittest
from pathlib import Path

from Tools.EffectPipeline import project_manifest_authoring_approximations as projector


ROOT = Path(__file__).resolve().parents[2]
MANIFEST_PATH = ROOT / (
    "Data/Effects/AuthoredCorrections/Warlord/"
    "Warlord.user-role-composition.json"
)


def load_json(path: Path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def file_sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def module_identities(element):
    return [
        (module["stableId"], module["className"])
        for module in element["sourceRecipe"]["modules"]
    ]


class ManifestAuthoringApproximationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.manifest = load_json(MANIFEST_PATH)
        cls.projection = cls.manifest[projector.PROJECTION_KEY]
        cls.rows = cls.manifest["provisionalApproximatePromotions"]
        cls.rows_by_id = {row["stableId"]: row for row in cls.rows}
        cls.rules_by_id = {
            row["stableId"]: row for row in cls.projection["rules"]
        }
        cls.evidence_by_id = {
            row["stableId"]: row
            for row in cls.projection["sourceEvidence"]["rows"]
        }
        cls.document_cache = {}
        cls.imported_cache = {}

    @classmethod
    def authored_element(cls, stable_id):
        row = cls.rows_by_id[stable_id]
        path = ROOT / row["effectDocument"]
        document = cls.document_cache.setdefault(path, load_json(path))
        return next(element for element in document["elements"] if element["id"] == stable_id)

    @classmethod
    def imported_element(cls, stable_id):
        evidence = cls.evidence_by_id[stable_id]
        path = ROOT / evidence["importedDocument"]
        document = cls.imported_cache.setdefault(path, load_json(path))
        return next(
            element
            for element in document["elements"]
            if element["id"] == evidence["importedElementId"]
        )

    @classmethod
    def reconstruct_before(cls, stable_id):
        row = cls.rows_by_id[stable_id]
        evidence = cls.evidence_by_id[stable_id]
        imported = cls.imported_element(stable_id)
        before = copy.deepcopy(cls.authored_element(stable_id))
        before["visible"] = False
        before["resources"] = copy.deepcopy(row["resources"])
        before["material"]["execution"] = {
            "enabled": False,
            "failClosed": True,
        }
        profile = before["material"]["sourceProfile"]
        profile["runtimeShaderProfileId"] = row["runtimeShaderProfileId"]
        profile.pop("productAdmissionStatus", None)
        profile.update(
            copy.deepcopy(
                cls.rules_by_id[stable_id].get("sourceProfileBefore", {})
            )
        )
        before["sourceRecipe"]["modules"] = copy.deepcopy(
            imported["sourceRecipe"]["modules"]
        )
        cls.assert_hashes(before, evidence)
        return before

    @staticmethod
    def assert_hashes(before, evidence):
        if (
            projector.canonical_json_sha256(before)
            != evidence["authoredBeforeElementCanonicalSha256"]
        ):
            raise AssertionError(f"cannot reconstruct before element: {evidence['stableId']}")
        if (
            projector.canonical_json_sha256(before["sourceRecipe"])
            != evidence["authoredBeforeSourceRecipeCanonicalSha256"]
        ):
            raise AssertionError(f"cannot reconstruct before recipe: {evidence['stableId']}")
        if (
            projector.canonical_json_sha256(before["resources"])
            != evidence["authoredBeforeResourcesCanonicalSha256"]
        ):
            raise AssertionError(f"cannot reconstruct before resources: {evidence['stableId']}")
        if (
            projector.canonical_json_sha256(before["sourceRecipe"]["modules"])
            != evidence["authoredBeforeModulesCanonicalSha256"]
        ):
            raise AssertionError(f"cannot reconstruct before modules: {evidence['stableId']}")

    def test_check_mode_and_exact_denominators(self):
        payloads, summary = projector.build_projection(
            ROOT, MANIFEST_PATH, require_projected=True
        )
        self.assertEqual({}, payloads)
        self.assertEqual(
            {
                "projectedTargetCount": 16,
                "changedTargetCount": 0,
                "targetDocumentCount": 5,
            },
            summary,
        )
        self.assertEqual("PROJECTED", self.projection["status"])
        self.assertEqual(16, self.projection["projectedApproximateCount"])
        self.assertEqual(16, len(self.rows_by_id))
        self.assertEqual(16, len(self.rules_by_id))
        self.assertEqual(16, len(self.evidence_by_id))

    def test_imported_full_evidence_and_before_hashes_are_recoverable(self):
        imported_paths = set()
        for stable_id, evidence in self.evidence_by_id.items():
            row = self.rows_by_id[stable_id]
            imported = self.imported_element(stable_id)
            imported_paths.add(evidence["importedDocument"])
            self.assertEqual(
                evidence["importedElementCanonicalSha256"],
                projector.canonical_json_sha256(imported),
            )
            self.assertEqual(
                evidence["importedSourceRecipeCanonicalSha256"],
                projector.canonical_json_sha256(imported["sourceRecipe"]),
            )
            self.assertEqual(
                evidence["importedResourcesCanonicalSha256"],
                projector.canonical_json_sha256(imported["resources"]),
            )
            self.assertEqual(
                evidence["importedModulesCanonicalSha256"],
                projector.canonical_json_sha256(imported["sourceRecipe"]["modules"]),
            )
            self.assertEqual(
                evidence["authoredBeforeResourcesCanonicalSha256"],
                projector.canonical_json_sha256(row["resources"]),
            )
            before = self.reconstruct_before(stable_id)
            self.assertEqual(
                evidence["authoredBeforeModulesCanonicalSha256"],
                evidence["importedModulesCanonicalSha256"],
            )
            self.assertEqual(
                evidence["projectedElementCanonicalSha256"],
                projector.canonical_json_sha256(
                    projector.project_element(
                        before, row, self.rules_by_id[stable_id]
                    )
                ),
            )
        self.assertGreaterEqual(len(imported_paths), 5)

    def test_only_manifest_modules_are_omitted_and_reasons_are_explicit(self):
        omitted_classes = []
        for stable_id, rule in self.rules_by_id.items():
            current = self.authored_element(stable_id)
            imported = self.imported_element(stable_id)
            omitted = [
                (row["stableId"], row["className"])
                for row in rule["omittedModules"]
            ]
            omitted_classes.extend(class_name for _, class_name in omitted)
            self.assertEqual(
                [identity for identity in module_identities(imported) if identity not in omitted],
                module_identities(current),
            )
            for omission in rule["omittedModules"]:
                self.assertIn(
                    omission["typedReason"], rule["approximationReasons"]
                )
            self.assertTrue(current["visible"])
            self.assertEqual(
                projector.EXPECTED_EXECUTION,
                current["material"]["execution"],
            )
            self.assertEqual(
                "effect.ue3.grouped-translucent.v1",
                current["material"]["sourceProfile"]["runtimeShaderProfileId"],
            )
            self.assertEqual(
                "AUTHORING_APPROXIMATE",
                current["material"]["sourceProfile"]["productAdmissionStatus"],
            )
        self.assertEqual(4, omitted_classes.count("particlemodulecollision"))
        self.assertEqual(2, omitted_classes.count("particlemodulesubuv"))
        self.assertEqual(3, omitted_classes.count("efparticlemodulelocationemitter"))
        self.assertEqual(2, omitted_classes.count("particlemodulelocationemitter"))

    def test_fallback_reclassification_and_same_occurrence_s_ring_base(self):
        fallback_we = [
            row
            for row in self.rows
            if row["inputSlot"] in {"W", "E"}
            and row["currentTypedReason"] == "SOURCE_PROFILE_FALLBACK_BLOCKED"
        ]
        self.assertEqual(6, len(fallback_we))
        for row in fallback_we:
            rule = self.rules_by_id[row["stableId"]]
            self.assertEqual([], rule["omittedModules"])
            self.assertEqual(
                ["SOURCE_PROFILE_FALLBACK_APPROXIMATED_WITH_GROUPED_TRANSLUCENT"],
                rule["approximationReasons"],
            )

        ring = next(
            row
            for row in self.rows
            if row["desiredRole"] == "SHIELD_DISAPPEAR_RING_SPRITE"
        )
        rule = self.rules_by_id[ring["stableId"]]
        override = rule["resourceOverrides"][0]
        self.assertEqual("SAME_OCCURRENCE_RESOURCE", override["assetEvidence"])
        source_assets = {row["assetId"] for row in ring["resources"]}
        self.assertIn(override["assetId"], source_assets)
        current_resources = {
            row["slotId"]: row["assetId"]
            for row in self.authored_element(ring["stableId"])["resources"]
        }
        self.assertEqual(override["assetId"], current_resources["base"])
        self.assertEqual(override["assetId"], current_resources["noise"])

    def test_stale_check_is_read_only_and_transaction_keeps_unrelated_files(self):
        with tempfile.TemporaryDirectory() as directory:
            temp_root = Path(directory)
            temp_manifest = temp_root / MANIFEST_PATH.relative_to(ROOT)
            temp_manifest.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(MANIFEST_PATH, temp_manifest)
            for row in self.rows:
                source = ROOT / row["effectDocument"]
                target = temp_root / row["effectDocument"]
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(source, target)
            for evidence in self.evidence_by_id.values():
                source = ROOT / evidence["importedDocument"]
                target = temp_root / evidence["importedDocument"]
                target.parent.mkdir(parents=True, exist_ok=True)
                if not target.exists():
                    shutil.copy2(source, target)

            stale_id = self.rows[0]["stableId"]
            stale_row = self.rows_by_id[stale_id]
            stale_path = temp_root / stale_row["effectDocument"]
            stale_document = load_json(stale_path)
            current = next(
                element for element in stale_document["elements"] if element["id"] == stale_id
            )
            restored = self.reconstruct_before(stale_id)
            stale_document["elements"][stale_document["elements"].index(current)] = restored
            stale_path.write_bytes(projector.encode_json_like(stale_path.read_bytes(), stale_document))
            unrelated = temp_root / "Data/Effects/Authored/unrelated.user-owned.effect.json"
            unrelated.write_bytes(b"user-owned\r\n")
            stale_before = file_sha(stale_path)
            unrelated_before = file_sha(unrelated)

            with self.assertRaisesRegex(projector.ProjectionError, "projection is stale"):
                projector.build_projection(
                    temp_root, temp_manifest, require_projected=True
                )
            self.assertEqual(stale_before, file_sha(stale_path))
            self.assertEqual(unrelated_before, file_sha(unrelated))

            payloads, summary = projector.build_projection(
                temp_root, temp_manifest, require_projected=False
            )
            self.assertEqual(1, summary["changedTargetCount"])
            projector.write_payloads_transactionally(payloads)
            projector.build_projection(temp_root, temp_manifest, require_projected=True)
            self.assertEqual(unrelated_before, file_sha(unrelated))

    def test_mid_commit_failure_rolls_back_every_file(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            first = root / "first.json"
            second = root / "second.json"
            unrelated = root / "unrelated.json"
            first.write_bytes(b"first-before")
            second.write_bytes(b"second-before")
            unrelated.write_bytes(b"unrelated")
            original = {path: path.read_bytes() for path in (first, second, unrelated)}
            call_count = 0

            def fail_once_on_second(source, target):
                nonlocal call_count
                call_count += 1
                if call_count == 2:
                    raise OSError("injected second commit failure")
                return Path(source).replace(target)

            with self.assertRaisesRegex(OSError, "injected second commit failure"):
                projector.write_payloads_transactionally(
                    {first: b"first-after", second: b"second-after"},
                    replace_func=fail_once_on_second,
                )
            for path, payload in original.items():
                self.assertEqual(payload, path.read_bytes())
            self.assertEqual([], list(root.glob(".*.tmp")))


if __name__ == "__main__":
    unittest.main(verbosity=2)

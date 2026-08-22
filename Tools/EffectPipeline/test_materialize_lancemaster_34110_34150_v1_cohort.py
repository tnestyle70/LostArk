#!/usr/bin/env python3
"""Focused contracts for the Lance D/F bounded RT0 Base cohort."""

from __future__ import annotations

import copy
import hashlib
import json
from pathlib import Path
import shutil
import tempfile
import unittest

from Tools.EffectPipeline import (
    materialize_lancemaster_34110_34150_v1_cohort as subject,
)


ROOT = Path(__file__).resolve().parents[2]


def load_json(root: Path, relative: Path) -> dict:
    return json.loads((root / relative).read_text(encoding="utf-8-sig"))


def exact_element(document: dict, element_id: str) -> dict:
    rows = [row for row in document["elements"] if row["id"] == element_id]
    if len(rows) != 1:
        raise AssertionError(f"element is not singular: {element_id}")
    return rows[0]


def copy_fixture(destination: Path) -> None:
    relative_files = [
        subject.D_DOCUMENT,
        subject.F_DOCUMENT,
        subject.FAMILY_MANIFEST,
        subject.SOURCE_RECEIPT,
        subject.EFFECT_CATALOG,
        subject.SKILL_BINDINGS,
        subject.ANIMATION_EVENTS,
        subject.OUTPUT_RECEIPT,
        subject.RUNTIME_SHADER,
        subject.MATERIAL_TEMPLATE,
        subject.DOCUMENT_RENDERER,
    ]
    relative_files.extend(
        Path("Client/Bin/Resources") / asset_id
        for asset_id in subject.RESOURCE_SHA256
    )
    for relative in relative_files:
        source = ROOT / relative
        target = destination / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)


class LanceMasterDFV1CohortTests(unittest.TestCase):
    def test_checked_in_materialization_converges(self) -> None:
        changed, receipt = subject.run(ROOT, "check")
        self.assertFalse(changed)
        self.assertEqual(
            [row["disposition"] for row in receipt["candidateDispositions"]],
            ["KEEP", "KEEP"],
        )
        self.assertEqual(
            receipt["materialActions"][0]["action"],
            "PROMOTE_EFFECTIVE_PARENT_PROFILE36",
        )

    def test_d_is_an_unchanged_profile15_mesh_control(self) -> None:
        document, element = subject.validate_d_control(ROOT)
        self.assertEqual(subject.canonical_sha256(document), subject.D_DOCUMENT_SHA256)
        self.assertEqual(subject.canonical_sha256(element), subject.D_ROW_SHA256)
        self.assertEqual(element["sourceRecipe"]["rendererShape"], "mesh")
        self.assertEqual(
            element["material"]["sourceProfile"]["runtimeShaderProfileId"],
            "effect.ue3.missiletrail-two-emissive.v1",
        )

    def test_f_keeps_carrier_and_promotes_five_lane_profile36_source(self) -> None:
        document = load_json(ROOT, subject.F_DOCUMENT)
        element = exact_element(document, subject.F_ELEMENT_ID)
        profile = element["material"]["sourceProfile"]
        self.assertEqual(element["sourceRecipe"]["rendererShape"], "mesh")
        self.assertEqual(
            next(row["assetId"] for row in element["resources"]
                 if row["slotId"] == "meshModel"),
            "Effect/LanceMaster/Meshes/fm_o_swing_02.wmodel",
        )
        self.assertEqual(element["detail"]["timing"]["startDelaySeconds"], 1.4053)
        self.assertEqual(profile["profileId"], subject.CANONICAL_PROFILE)
        self.assertEqual(profile["parentMaterialPath"], subject.CANONICAL_PARENT)
        self.assertEqual(
            [row["name"] for row in profile["textures"]],
            list(subject.REQUIRED_TEXTURE_NAMES),
        )
        self.assertEqual(len(profile["scalars"]), 28)
        self.assertEqual(profile["dynamicParameterSemantics"], ["unbound"] * 4)

    def test_receipt_separates_carrier_keep_from_material_promotion(self) -> None:
        receipt = load_json(ROOT, subject.OUTPUT_RECEIPT)
        self.assertEqual(
            receipt["candidateDenominator"],
            {"keep": 2, "replace": 0, "add": 0, "retire": 0,
             "bulkRestore": False},
        )
        self.assertEqual(
            receipt["fCanary"]["effectiveSourceMaterialProfileIndex"], 36
        )
        dynamic = receipt["fCanary"]["dynamicParameter"]
        self.assertEqual(dynamic["sourceNames"], ["none"] * 4)
        self.assertEqual(dynamic["runtimeSemantics"], ["unbound"] * 4)
        self.assertEqual(dynamic["profile36Defaults"], [1.0, 0.0, 0.0, 0.0])
        self.assertEqual(dynamic["channel3Policy"], "SUPPRESSED_UNUSED_DEFAULT_ZERO")
        self.assertEqual(receipt["fCanary"]["fidelity"],
                         "PROJECT_RECONSTRUCTED_BOUNDED")
        self.assertEqual(receipt["runtime"]["visualState"],
                         "USER_REVIEW_PENDING")
        self.assertFalse(receipt["runtime"]["newCpp"])
        self.assertFalse(receipt["runtime"]["newHlsl"])
        self.assertEqual(
            set(receipt["runtime"]["evidence"]),
            {"shaderBranchSha256", "strictAdmissionSha256",
             "rendererBranchSha256"},
        )
        self.assertEqual(
            receipt["runtime"]["evidence"],
            subject.validate_profile36_runtime(ROOT),
        )
        artifact = receipt.pop("artifactSha256")
        self.assertEqual(artifact, subject.canonical_sha256(receipt))

    def test_idempotent_write_preserves_product_and_receipt_bytes(self) -> None:
        paths = [ROOT / subject.D_DOCUMENT, ROOT / subject.F_DOCUMENT,
                 ROOT / subject.OUTPUT_RECEIPT]
        before = {path: hashlib.sha256(path.read_bytes()).hexdigest()
                  for path in paths}
        changed, _ = subject.run(ROOT, "write")
        after = {path: hashlib.sha256(path.read_bytes()).hexdigest()
                 for path in paths}
        self.assertFalse(changed)
        self.assertEqual(before, after)

    def test_legacy_fixture_changes_only_f_target_source_profile(self) -> None:
        with tempfile.TemporaryDirectory(prefix="lance-d-f-v1-cohort-") as raw:
            fixture = Path(raw)
            copy_fixture(fixture)
            f_path = fixture / subject.F_DOCUMENT
            before_d = (fixture / subject.D_DOCUMENT).read_bytes()
            document = load_json(fixture, subject.F_DOCUMENT)
            binding = subject.source_material_binding(fixture)
            target = exact_element(document, subject.F_ELEMENT_ID)
            target["material"]["sourceProfile"] = (
                subject.build_legacy_source_profile(binding)
            )
            f_path.write_text(
                json.dumps(document, ensure_ascii=False, indent=2) + "\n",
                encoding="utf-8",
            )
            (fixture / subject.OUTPUT_RECEIPT).unlink()
            before_document = copy.deepcopy(document)

            changed, _ = subject.run(fixture, "write")
            self.assertTrue(changed)
            promoted = load_json(fixture, subject.F_DOCUMENT)
            self.assertEqual((fixture / subject.D_DOCUMENT).read_bytes(), before_d)
            self.assertEqual(
                [row for row in promoted["elements"]
                 if row["id"] != subject.F_ELEMENT_ID],
                [row for row in before_document["elements"]
                 if row["id"] != subject.F_ELEMENT_ID],
            )
            before_target = exact_element(before_document, subject.F_ELEMENT_ID)
            after_target = exact_element(promoted, subject.F_ELEMENT_ID)
            before_target["material"].pop("sourceProfile")
            after_target["material"].pop("sourceProfile")
            self.assertEqual(before_target, after_target)
            subject.run(fixture, "check")

    def test_source_recipe_drift_fails_before_commit(self) -> None:
        with tempfile.TemporaryDirectory(prefix="lance-d-f-drift-") as raw:
            fixture = Path(raw)
            copy_fixture(fixture)
            f_path = fixture / subject.F_DOCUMENT
            document = load_json(fixture, subject.F_DOCUMENT)
            target = exact_element(document, subject.F_ELEMENT_ID)
            module = next(
                row for row in target["sourceRecipe"]["modules"]
                if row["className"] == "particlemoduleparameterdynamic"
            )
            dynamic_name = next(
                row for row in module["literals"]
                if row["propertyPath"] == "dynamicparams[3].paramname"
            )
            dynamic_name["value"] = "guessed_lane"
            f_path.write_text(
                json.dumps(document, ensure_ascii=False, indent=2) + "\n",
                encoding="utf-8",
            )
            before = f_path.read_bytes()
            with self.assertRaisesRegex(
                subject.LanceV1CohortError,
                "carrier/transform/timing/source recipe changed",
            ):
                subject.run(fixture, "write")
            self.assertEqual(f_path.read_bytes(), before)


if __name__ == "__main__":
    unittest.main()

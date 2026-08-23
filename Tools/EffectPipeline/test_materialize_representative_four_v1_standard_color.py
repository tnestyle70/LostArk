#!/usr/bin/env python3
"""Focused contracts for the representative-four StandardColorV1 slice."""

from __future__ import annotations

import copy
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
import unittest

from Tools.EffectPipeline import (
    build_effect_material_program_registry as registry_builder,
    materialize_representative_four_v1_standard_color as subject,
)


ROOT = Path(__file__).resolve().parents[2]


def load_json(relative: Path) -> dict:
    return json.loads((ROOT / relative).read_text(encoding="utf-8-sig"))


class RepresentativeFourV1StandardColorTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.receipt = load_json(subject.RECEIPT_PATH)
        cls.fragment = load_json(subject.FRAGMENT_PATH)
        cls.v0_by_id = {
            spec["v0"]: load_json(subject.source_document_path(spec["v0"]))
            for spec in subject.SPECS
        }
        cls.v1_by_id = {
            spec["v1"]: load_json(subject.source_document_path(spec["v1"]))
            for spec in subject.SPECS
        }

    def test_checked_in_materialization_converges(self) -> None:
        changed, receipt = subject.run(ROOT, "check")
        self.assertFalse(changed)
        self.assertEqual(
            receipt["denominator"],
            {
                "documents": 5,
                "occurrences": 131,
                "sprite": 98,
                "mesh": 25,
                "decal": 8,
                "fallback": 11,
            },
        )

    def test_clean_checkout_preserves_sealed_source_and_generated_bytes(self) -> None:
        source_eol = {
            "effect.dimensionmaster.skill.2050180.unified": "lf",
            "effect.artist.skill.31460.unified": "crlf",
            "effect.lancemaster.skill.34110.unified": "crlf",
            "effect.warlord.skill.17110.clip2.unified": "lf",
            "effect.warlord.skill.17110.clip3.unified": "lf",
        }
        source_rows = [
            (
                subject.source_document_path(spec["v0"]),
                source_eol[spec["v0"]],
                spec["rawSha256"],
            )
            for spec in subject.SPECS
        ]
        generated_paths = [
            subject.source_document_path(spec["v1"])
            for spec in subject.SPECS
        ] + [subject.RECEIPT_PATH]
        rows = source_rows + [
            (path, "lf", None) for path in generated_paths
        ]
        relatives = [path.as_posix() for path, _, _ in rows]
        attributes = subprocess.run(
            ["git", "check-attr", "text", "eol", "--", *relatives],
            cwd=ROOT,
            check=True,
            capture_output=True,
            text=True,
        ).stdout.splitlines()
        self.assertEqual(
            attributes,
            [
                line
                for relative, (_, eol, _) in zip(relatives, rows)
                for line in (
                    f"{relative}: text: set",
                    f"{relative}: eol: {eol}",
                )
            ],
        )

        for relative, eol, expected_sha256 in rows:
            with self.subTest(path=relative, eol=eol):
                repository_bytes = subprocess.run(
                    ["git", "cat-file", "blob", f"HEAD:{relative.as_posix()}"],
                    cwd=ROOT,
                    check=True,
                    capture_output=True,
                ).stdout
                self.assertNotIn(b"\r", repository_bytes)
                checkout_bytes = (
                    repository_bytes
                    if eol == "lf"
                    else repository_bytes.replace(b"\n", b"\r\n")
                )
                self.assertEqual((ROOT / relative).read_bytes(), checkout_bytes)
                if expected_sha256 is not None:
                    self.assertEqual(
                        hashlib.sha256(checkout_bytes).hexdigest(),
                        expected_sha256,
                    )

    def test_all_rows_are_truthfully_project_tuned_approx(self) -> None:
        rows = self.receipt["occurrences"]
        self.assertEqual(len(rows), 131)
        self.assertEqual(
            {row["fidelityClass"] for row in rows},
            {subject.PROJECT_TUNED_APPROX},
        )
        self.assertFalse(any(row["sourceExact"] for row in rows))
        fallback = [
            row
            for row in rows
            if "PROJECT_TUNED_DOMAIN_GLOW_FALLBACK" in row["reasonCodes"]
        ]
        self.assertEqual(len(fallback), 11)
        self.assertEqual(
            {row["v1EffectAssetId"] for row in fallback},
            {
                "effect.lancemaster.skill.34110.v1.unified",
                "effect.warlord.skill.17110.clip2.v1.unified",
                "effect.warlord.skill.17110.clip3.v1.unified",
            },
        )

    def test_v1_preserves_non_material_occurrence_state_and_evidence(self) -> None:
        for spec in subject.SPECS:
            with self.subTest(effect=spec["v1"]):
                source = self.v0_by_id[spec["v0"]]
                target = self.v1_by_id[spec["v1"]]
                self.assertEqual(target["effectAssetId"], spec["v1"])
                self.assertEqual(target["displayName"], spec["v1"])
                self.assertEqual(
                    subject.canonical_sha256(
                        subject._non_material_projection(source)
                    ),
                    subject.canonical_sha256(
                        subject._non_material_projection(target)
                    ),
                )
                self.assertEqual(
                    [row["id"] for row in source["elements"]],
                    [row["id"] for row in target["elements"]],
                )
                for v0_element, v1_element in zip(
                    source["elements"], target["elements"]
                ):
                    v0_material = v0_element["material"]
                    v1_material = v1_element["material"]
                    self.assertEqual(
                        v1_material["sourceMaterialPath"],
                        v0_material["sourceMaterialPath"],
                    )
                    self.assertEqual(
                        v1_material["renderProfile"],
                        v0_material["renderProfile"],
                    )
                    expected_profile = copy.deepcopy(v0_material["sourceProfile"])
                    expected_profile["enabled"] = False
                    self.assertEqual(v1_material["sourceProfile"], expected_profile)
                    self.assertNotIn("authoringOverrides", v1_element)

    def test_every_target_uses_typed_standard_color_and_carrier_resources(self) -> None:
        counts = {"sprite": 0, "mesh": 0, "decal": 0}
        lane_counts = {2: 0, 3: 0}
        for document in self.v1_by_id.values():
            for element in document["elements"]:
                carrier = subject._carrier(element)
                counts[carrier] += 1
                resources = element["resources"]
                if carrier == "mesh":
                    self.assertEqual(len(resources), 1)
                    self.assertEqual(resources[0]["slotId"], "meshModel")
                else:
                    self.assertEqual(resources, [])
                material = element["material"]
                self.assertEqual(material["templateId"], "effect.standard_color_v1")
                self.assertFalse(material["sourceProfile"]["enabled"])
                execution = material["execution"]
                self.assertTrue(execution["enabled"])
                self.assertEqual((execution["backend"], execution["opcode"]),
                                 ("standardColorV1", 1))
                lane_counts[execution["textureLaneCount"]] += 1
                expected_roles = ["base_radiance", "coverage"]
                if execution["textureLaneCount"] == 3:
                    expected_roles.append("dissolve")
                self.assertEqual(
                    [row["role"] for row in execution["textureLanes"]],
                    expected_roles,
                )
                for index, lane in enumerate(execution["textureLanes"]):
                    self.assertEqual(lane["laneId"], f"lane.{index}")
                    self.assertEqual(lane["textureRegister"], index)
                    self.assertEqual(lane["samplerRegister"], 5 + index)
                    if index == 0:
                        self.assertIn(lane["sourceChannel"], ("R", "RGB"))
                        self.assertIn(lane["colorSpace"], ("linear", "srgb"))
                    else:
                        self.assertIn(lane["sourceChannel"], ("R", "A"))
                        self.assertEqual(lane["colorSpace"], "linear")
                    self.assertTrue(
                        (ROOT / "Client/Bin/Resources" / lane["assetId"]).is_file()
                    )
                standard = execution["standardColor"]
                self.assertEqual(
                    standard["baseRadianceChannel"],
                    execution["textureLanes"][0]["sourceChannel"],
                )
                self.assertEqual(
                    standard["coverageChannel"],
                    execution["textureLanes"][1]["sourceChannel"],
                )
                if execution["textureLaneCount"] == 3:
                    self.assertEqual(
                        standard["dissolveChannel"],
                        execution["textureLanes"][2]["sourceChannel"],
                    )
                for field in (
                    "dynamicConsumedMask", "dynamicSuppressedMask",
                    "particleColorPolicy", "particleColorConsumedMask",
                    "particleColorSuppressedMask", "scalarCount", "vectorCount",
                    "inputCount", "staticInputCount", "staticSelectedMask",
                    "staticConsumedMask", "staticSuppressedMask",
                    "renderInputCount", "renderConsumedMask", "renderSuppressedMask",
                ):
                    self.assertEqual(execution[field], 0)
                for field in (
                    "scalars", "vectors", "artistParameters", "colors"
                ):
                    self.assertEqual(execution[field], [])
        self.assertEqual(counts, {"sprite": 98, "mesh": 25, "decal": 8})
        self.assertEqual(lane_counts, {2: 104, 3: 27})
        policy = self.receipt["laneAdmissionPolicy"]
        self.assertEqual(policy["baseRadianceChannelCounts"], {"RGB": 126, "R": 5})
        self.assertEqual(policy["coverageChannelCounts"], {"A": 31, "R": 100})
        self.assertEqual(policy["dissolveChannelCounts"], {"A": 0, "R": 27})
        self.assertEqual(
            policy["baseRadianceColorSpaceCounts"],
            {"srgb": 6, "linear": 125},
        )

    def test_decal_alpha_two_uses_actual_projector_depth_state(self) -> None:
        executions = [
            element["material"]["execution"]
            for document in self.v1_by_id.values()
            for element in document["elements"]
            if subject._carrier(element) == "decal"
        ]
        self.assertEqual(len(executions), 8)
        self.assertTrue(
            all(
                execution["passIndex"] == 1
                and execution["renderState"] == {
                    "rasterizer": "RS_Cull_None",
                    "depthStencil": "DSS_ZNone",
                    "blend": "BS_EffectAlpha",
                    "stencilReference": 0,
                }
                for execution in executions
            )
        )

    def test_fragment_has_one_descriptor_and_binding_per_occurrence(self) -> None:
        self.assertEqual(
            self.fragment["programs"],
            [{
                "programId": subject.PROGRAM_ID,
                "backend": "standardColorV1",
                "opcode": 1,
            }],
        )
        self.assertEqual(
            len(self.fragment["layouts"]),
            self.receipt["registryFragment"]["layoutCount"],
        )
        self.assertEqual(len(self.fragment["layouts"]), 6)
        self.assertEqual(len(self.fragment["descriptors"]), 131)
        self.assertEqual(len(self.fragment["bindings"]), 131)
        descriptors = {
            row["descriptorId"]: row for row in self.fragment["descriptors"]
        }
        self.assertEqual(len(descriptors), 131)
        documents = self.v1_by_id
        for binding in self.fragment["bindings"]:
            document = documents[binding["effectAssetId"]]
            matches = [
                row for row in document["elements"]
                if row["id"] == binding["elementId"]
            ]
            self.assertEqual(len(matches), 1)
            execution = matches[0]["material"]["execution"]
            descriptor = descriptors[binding["descriptorId"]]
            self.assertEqual(descriptor["layoutId"], binding["layoutId"])
            self.assertEqual(
                [row["assetId"] for row in descriptor["textureLanes"]],
                [row["assetId"] for row in execution["textureLanes"]],
            )
            self.assertEqual(
                binding["inlineMirrorPolicy"], subject.INLINE_MIRROR_REQUIRED
            )

    def test_registry_materializes_every_inline_packet_bit_exact(self) -> None:
        fragment_root = ROOT / "Data/Effects/MaterialPrograms/Fragments"
        with tempfile.TemporaryDirectory(
            prefix="representative-four-v1-registry-"
        ) as raw:
            normalized_root = Path(raw)
            for source in fragment_root.glob("*.json"):
                (normalized_root / source.name).write_bytes(
                    source.read_bytes().replace(b"\r\n", b"\n")
                )
            registry = registry_builder.build_registry(
                ROOT / "Data/Effects/MaterialPrograms/"
                "effect-material-program-registry.v1.json",
                ROOT / subject.EFFECT_CATALOG,
                ROOT / "Data",
                normalized_root,
            )
        representative = [
            row for row in registry["bindings"]
            if row["effectAssetId"].endswith(".v1.unified")
            and row["effectAssetId"] in self.v1_by_id
        ]
        self.assertEqual(len(representative), 131)

    def test_catalog_rows_are_explicit_unique_and_sorted(self) -> None:
        catalog = load_json(subject.EFFECT_CATALOG)
        ids = [row["effectAssetId"] for row in catalog["effects"]]
        self.assertEqual(ids, sorted(ids))
        self.assertEqual(len(ids), len(set(ids)))
        indexed = {row["effectAssetId"]: row for row in catalog["effects"]}
        for spec in subject.SPECS:
            self.assertEqual(
                indexed[spec["v1"]],
                {
                    "effectAssetId": spec["v1"],
                    "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
                    "authoringPath": f"Effects/Authored/{spec['v1']}.effect.json",
                    "runtimeAdmission": subject.REGISTRY_BOUND_AUDITION_ONLY,
                    "fidelityClass": subject.PROJECT_TUNED_APPROX,
                    "sourceEffectAssetId": spec["v0"],
                    "sourceDocumentRawSha256": spec["rawSha256"],
                },
            )

    def test_lane_evidence_is_decoded_and_sampling_is_explicit(self) -> None:
        resource_rows = self.receipt["resourceEvidence"]
        self.assertGreater(len(resource_rows), 0)
        for row in resource_rows:
            statistics = row["textureStatistics"]
            self.assertGreater(statistics["width"], 0)
            self.assertGreater(statistics["height"], 0)
            self.assertEqual(
                [channel["channel"] for channel in statistics["channels"]],
                list("RGBA"),
            )
            self.assertEqual(
                row["textureStatisticsCanonicalSha256"],
                subject.canonical_sha256(statistics),
            )
        for row in self.receipt["occurrences"]:
            for lane in (row["baseRadiance"], row["coverage"], row["dissolve"]):
                if lane is None:
                    continue
                self.assertIn(
                    lane["samplingEvidence"]["evidence"],
                    (
                        "SOURCE_PROFILE_TEXTURE_MATCH",
                        "PROJECT_TUNED_LINEAR_WRAP_POLICY",
                    ),
                )
                self.assertEqual(len(lane["ddsEvidenceCanonicalSha256"]), 64)

    def test_receipt_hashes_v0_v1_resources_and_its_own_payload(self) -> None:
        document_rows = self.receipt["documents"]
        self.assertEqual(len(document_rows), 5)
        for row in document_rows:
            self.assertEqual(
                row["v0"]["nonMaterialInvariantSha256"],
                row["v1"]["nonMaterialInvariantSha256"],
            )
            for side in ("v0", "v1"):
                self.assertEqual(len(row[side]["rawSha256"]), 64)
                self.assertEqual(len(row[side]["canonicalSha256"]), 64)
                self.assertEqual(len(row[side]["resourcesCanonicalSha256"]), 64)
        receipt = copy.deepcopy(self.receipt)
        artifact = receipt.pop("artifactSha256")
        self.assertEqual(artifact, subject.canonical_sha256(receipt))

    def test_sealed_v0_drift_fails_before_materialization(self) -> None:
        spec = subject.SPECS[0]
        source_path = ROOT / subject.source_document_path(spec["v0"])
        payload, document = subject.load_json_bytes(source_path)
        modified = copy.deepcopy(document)
        modified["particleSystem"]["yawOffsetDegrees"] = 0.5
        with self.assertRaisesRegex(
            subject.RepresentativeFourV1Error, "sealed V0 raw identity changed"
        ):
            subject._validate_source_identity(
                spec, subject.pretty_bytes(modified), modified
            )
        subject._validate_source_identity(spec, payload, document)


if __name__ == "__main__":
    unittest.main()

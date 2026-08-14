#!/usr/bin/env python3
"""Regression tests for the generic V6 Effect visual-program corpus."""

from __future__ import annotations

import copy
import importlib.util
import tempfile
import unittest
from collections import Counter
from pathlib import Path


SCRIPT_PATH = Path(__file__).resolve().with_name(
    "build_effect_visual_program_corpus.py"
)
SPEC = importlib.util.spec_from_file_location(
    "build_effect_visual_program_corpus",
    SCRIPT_PATH,
)
assert SPEC is not None and SPEC.loader is not None
builder = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(builder)


def reseal(value: dict, field: str) -> None:
    value.pop(field, None)
    value[field] = builder.canonical_json_sha256(value)


def reseal_visual_row(corpus: dict, row: dict) -> None:
    reseal(row, "rowSha256")
    reseal(corpus, "artifactSha256")


class EffectVisualProgramCorpusTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.repository_root = builder.REPOSITORY_ROOT
        cls.corpus = builder.build_corpus(cls.repository_root)

    def test_schema_declares_closed_phase1_shape(self) -> None:
        schema = builder.load_json(
            self.repository_root / builder.SCHEMA_RELATIVE_PATH
        )
        self.assertEqual(
            schema["properties"]["contractRole"]["const"],
            builder.CONTRACT_ROLE,
        )
        self.assertEqual(
            schema["properties"]["visualRows"]["minItems"],
            135,
        )
        self.assertEqual(
            schema["properties"]["visualRows"]["maxItems"],
            135,
        )
        self.assertIn("extensionCanaries", schema["required"])
        self.assertIn("supplementalElements", schema["required"])
        self.assertIn(
            "sourceRowSha256",
            schema["$defs"]["sourceRecipe"]["required"],
        )
        self.assertEqual(
            schema["$defs"]["selector"]["required"],
            ["effectAssetId", "occurrenceId"],
        )

    def test_exact_denominators_and_truthful_execution_projection(self) -> None:
        corpus = self.corpus
        self.assertEqual(len(corpus["presentationSchedules"]), 35)
        self.assertEqual(len(corpus["visualRows"]), 135)
        ba = [row for row in corpus["visualRows"] if row["provenance"]["scope"] == "COMBAT_BA"]
        local = [row for row in corpus["visualRows"] if row["provenance"]["scope"] == "ARTIST_F_LOCAL_DECAL"]
        legacy = [row for row in ba if row["executionProjection"]["fidelity"] == "LEGACY_APPROXIMATION"]
        fail_closed = [row for row in ba if row["executionProjection"]["disposition"] == "FAIL_CLOSED"]
        self.assertEqual((len(ba), len(local), len(legacy), len(fail_closed)), (133, 2, 66, 63))
        self.assertEqual(
            Counter(
                row["executionProjection"]["family"]
                for row in corpus["visualRows"]
            ),
            Counter({
                "MESH_PARTICLE": 41,
                "SPRITE_PARTICLE": 69,
                "CASCADE_RIBBON": 4,
                "DECAL_PARTICLE": 4,
                "LIGHT_PARTICLE": 15,
                "SCREEN_POST": 2,
            }),
        )
        self.assertTrue(
            all(
                row["executionProjection"]["disposition"] == "ADMITTED_BOUNDED"
                and row["sourcePayload"] is not None
                and row["targetPayload"] is not None
                and row["executionProjection"]["sourceExact"] is False
                for row in legacy
            )
        )
        self.assertEqual(
            {row["selector"]["occurrenceId"] for row in local},
            {"source-active-020", "source-active-021"},
        )
        self.assertTrue(
            all(
                row["selector"]["effectAssetId"] == "effect.artist.skill.31470"
                and row["executionProjection"]["adapterId"] == "local-decal-rt0-bounded-v1"
                and row["executionProjection"]["nativeExecution"] is False
                for row in local
            )
        )

        cascade_rows = [
            row for row in ba
            if row["executionProjection"]["family"] == "CASCADE_RIBBON"
        ]
        self.assertEqual(len(cascade_rows), 4)
        for row in cascade_rows:
            resolution = row["sourceRecipe"]["typedFamilyResolution"]
            self.assertEqual(
                (
                    row["sourceRecipe"]["rendererShape"],
                    row["sourceRecipe"]["resolvedRendererShape"],
                    resolution["declaredRendererFamily"],
                    resolution["resolvedFamily"],
                    resolution["typeDataStableId"],
                    resolution["typeDataClassName"],
                ),
                (
                    "sprite",
                    "ribbon",
                    "SPRITE_PARTICLE",
                    "CASCADE_RIBBON",
                    "FX_PC_FLM_01:export:5909@ref:8",
                    "particlemoduletypedataribbon",
                ),
            )
            self.assertEqual(
                row["executionProjection"]["disposition"],
                "ADMITTED_BOUNDED",
            )

        supplemental = corpus["supplementalElements"]
        self.assertEqual(len(supplemental), 5)
        self.assertEqual(
            Counter(item["family"] for item in supplemental),
            Counter({"CASCADE_RIBBON": 1, "ANIMATION_TRAIL": 4}),
        )
        self.assertTrue(
            all(
                set(item["selector"]) == {"effectAssetId", "occurrenceId"}
                and item["disposition"] == "ADMITTED_BOUNDED"
                and item["tuningEligibleTransform"] is True
                for item in supplemental
            )
        )
        self.assertTrue(
            all(
                [resource["shaderRegister"] for resource in row["executionProjection"]["resourceRoles"]]
                == ["t0", "t1", "t2", "t3", "t4", "t5"]
                for row in local
            )
        )
        self.assertTrue(
            all(
                [(resource["role"], resource["sourceChannel"])
                 for resource in row["executionProjection"]["resourceRoles"]]
                == [
                    ("HEIGHT", "B"),
                    ("DIFFUSE", "RGBA"),
                    ("DISSOLVE", "G"),
                    ("NORMAL", "RG"),
                    ("SPECULAR", "RGB"),
                    ("EMISSIVE", "R"),
                ]
                for row in local
            )
        )

    def test_duplicate_selector_is_rejected(self) -> None:
        corpus = copy.deepcopy(self.corpus)
        corpus["visualRows"][1]["selector"] = copy.deepcopy(corpus["visualRows"][0]["selector"])
        corpus["visualRows"][1]["selectorSha256"] = builder.canonical_json_sha256(
            corpus["visualRows"][1]["selector"]
        )
        reseal(corpus["visualRows"][1], "rowSha256")
        corpus["visualRows"].sort(
            key=lambda row: (
                row["selector"]["effectAssetId"],
                row["selector"]["occurrenceId"],
            )
        )
        reseal(corpus, "artifactSha256")
        with self.assertRaisesRegex(builder.ContractError, "duplicate visual selector"):
            builder.validate_corpus(corpus, self.repository_root)

    def test_stale_source_and_target_payload_hashes_are_rejected(self) -> None:
        for payload_name in ("sourcePayload", "targetPayload"):
            with self.subTest(payload_name=payload_name):
                corpus = copy.deepcopy(self.corpus)
                row = next(
                    item
                    for item in corpus["visualRows"]
                    if item["executionProjection"]["disposition"] == "ADMITTED_BOUNDED"
                    and item[payload_name] is not None
                )
                row[payload_name]["rawSha256"] = "0" * 64
                reseal_visual_row(corpus, row)
                with self.assertRaisesRegex(builder.ContractError, "raw hash is stale"):
                    builder.validate_corpus(corpus, self.repository_root)

        corpus = copy.deepcopy(self.corpus)
        row = next(
            item
            for item in corpus["visualRows"]
            if item["provenance"]["scope"] == "COMBAT_BA"
        )
        row["sourceRecipe"]["sourceRowSha256"] = "0" * 64
        reseal_visual_row(corpus, row)
        with self.assertRaisesRegex(builder.ContractError, "source-row reference is stale"):
            builder.validate_corpus(corpus, self.repository_root)

    def test_unknown_adapter_and_family_mismatch_are_rejected(self) -> None:
        corpus = copy.deepcopy(self.corpus)
        row = corpus["visualRows"][0]
        row["executionProjection"]["adapterId"] = "unknown-adapter-v1"
        reseal_visual_row(corpus, row)
        with self.assertRaisesRegex(builder.ContractError, "unknown visual family adapter"):
            builder.validate_corpus(corpus, self.repository_root)

        corpus = copy.deepcopy(self.corpus)
        row = next(
            item
            for item in corpus["visualRows"]
            if item["executionProjection"]["family"] == "MESH_PARTICLE"
        )
        row["executionProjection"]["adapterId"] = "sprite-particle-document-v12"
        reseal_visual_row(corpus, row)
        with self.assertRaisesRegex(builder.ContractError, "adapter/family mismatch"):
            builder.validate_corpus(corpus, self.repository_root)

    def test_admitted_unresolved_resource_or_basis_is_rejected(self) -> None:
        corpus = copy.deepcopy(self.corpus)
        row = next(
            item
            for item in corpus["visualRows"]
            if item["executionProjection"]["disposition"] == "ADMITTED_BOUNDED"
        )
        row["executionProjection"]["executionBasis"]["status"] = "UNRESOLVED_FAIL_CLOSED"
        reseal_visual_row(corpus, row)
        with self.assertRaisesRegex(builder.ContractError, "unresolved execution basis"):
            builder.validate_corpus(corpus, self.repository_root)

        corpus = copy.deepcopy(self.corpus)
        row = next(
            item
            for item in corpus["visualRows"]
            if item["executionProjection"]["disposition"] == "ADMITTED_BOUNDED"
        )
        row["executionProjection"]["resourceRoles"][0]["resolutionStatus"] = "SOURCE_EVIDENCE_ONLY"
        reseal_visual_row(corpus, row)
        with self.assertRaisesRegex(builder.ContractError, "unresolved for an admitted row"):
            builder.validate_corpus(corpus, self.repository_root)

    def test_class_skill_order_selector_leakage_is_rejected(self) -> None:
        for prohibited in ("characterClass", "skillId", "sourceOrder"):
            with self.subTest(prohibited=prohibited):
                corpus = copy.deepcopy(self.corpus)
                row = corpus["visualRows"][0]
                row["selector"][prohibited] = 1 if prohibited != "characterClass" else "ARTIST"
                row["selectorSha256"] = builder.canonical_json_sha256(row["selector"])
                reseal_visual_row(corpus, row)
                with self.assertRaisesRegex(builder.ContractError, "selector keys mismatch"):
                    builder.validate_corpus(corpus, self.repository_root)

    def test_warlord_and_valtan_class_free_canary_selector_contract(self) -> None:
        adapters = self.corpus["adapterContracts"]
        fixtures = (
            (
                {"effectAssetId": "effect.warlord.skill.17000.ba1", "occurrenceId": "canary-warlord-001"},
                "MESH_PARTICLE",
                "mesh-particle-document-v12",
            ),
            (
                {"effectAssetId": "effect.valtan.schema-canary", "occurrenceId": "canary-valtan-001"},
                "SPRITE_PARTICLE",
                "sprite-particle-document-v12",
            ),
        )
        for selector, family, adapter_id in fixtures:
            with self.subTest(selector=selector):
                self.assertEqual(set(selector), {"effectAssetId", "occurrenceId"})
                builder.validate_selector_for_adapter(
                    selector,
                    family,
                    adapter_id,
                    "NONE",
                    adapters,
                )
        with self.assertRaisesRegex(builder.ContractError, "unknown visual family adapter"):
            builder.validate_selector_for_adapter(
                fixtures[0][0],
                "MESH_PARTICLE",
                "warlord-special-case-adapter",
                "NONE",
                adapters,
            )
        self.assertEqual(
            {(row["domain"], row["fidelity"], row["disposition"]) for row in self.corpus["extensionCanaries"]},
            {
                ("WARLORD", "EVIDENCE_ONLY", "FAIL_CLOSED"),
                ("VALTAN", "EVIDENCE_ONLY", "FAIL_CLOSED"),
            },
        )
        self.assertTrue(
            all(row["productCountContribution"] is False for row in self.corpus["extensionCanaries"])
        )

    def test_builder_is_deterministic_and_check_detects_stale_output(self) -> None:
        first = builder.build_corpus(self.repository_root)
        second = builder.build_corpus(self.repository_root)
        self.assertEqual(builder.pretty_json_bytes(first), builder.pretty_json_bytes(second))
        with tempfile.TemporaryDirectory() as temporary:
            output_path = Path(temporary) / "effect-visual-program-corpus.v1.json"
            builder.write_corpus_transactionally(first, output_path, self.repository_root)
            builder.build_and_write(self.repository_root, output_path, check=True)
            output_path.write_bytes(output_path.read_bytes() + b" ")
            with self.assertRaisesRegex(builder.ContractError, "is stale"):
                builder.build_and_write(self.repository_root, output_path, check=True)

    def test_invalid_stage_preserves_previous_output(self) -> None:
        invalid = copy.deepcopy(self.corpus)
        row = invalid["visualRows"][0]
        row["selector"]["skillId"] = row["provenance"]["skillId"]
        row["selectorSha256"] = builder.canonical_json_sha256(row["selector"])
        reseal_visual_row(invalid, row)
        with tempfile.TemporaryDirectory() as temporary:
            output_path = Path(temporary) / "previous.json"
            previous = b"previous-valid-output\n"
            output_path.write_bytes(previous)
            with self.assertRaises(builder.ContractError):
                builder.write_corpus_transactionally(
                    invalid,
                    output_path,
                    self.repository_root,
                )
            self.assertEqual(output_path.read_bytes(), previous)


if __name__ == "__main__":
    unittest.main()

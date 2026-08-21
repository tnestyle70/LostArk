#!/usr/bin/env python3

from __future__ import annotations

import copy
from collections import Counter
import importlib.util
import json
import sys
import tempfile
import unittest
from pathlib import Path


SCRIPT_PATH = Path(__file__).resolve()
REPOSITORY_ROOT = SCRIPT_PATH.parent.parent.parent


def load_module(name: str, path: Path):
    specification = importlib.util.spec_from_file_location(name, path)
    if specification is None or specification.loader is None:
        raise RuntimeError(f"could not load {path}")
    module = importlib.util.module_from_spec(specification)
    sys.modules[name] = module
    specification.loader.exec_module(module)
    return module


validator = load_module(
    "validate_boss_pattern_effects",
    SCRIPT_PATH.parent / "validate_boss_pattern_effects.py",
)
builder = load_module(
    "build_valtan_whirlwind_effect_canary",
    SCRIPT_PATH.parent / "build_valtan_whirlwind_effect_canary.py",
)


class ValtanWhirlwindEffectCanaryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.mapping_path = REPOSITORY_ROOT / builder.MAPPING_RELATIVE_PATH
        cls.schema_path = REPOSITORY_ROOT / builder.SCHEMA_RELATIVE_PATH
        cls.mapping = validator.load_json(cls.mapping_path)
        cls.schema = validator.load_json(cls.schema_path)
        cls.binding, _catalog, cls.source_carriers = builder.validate_source_contract(
            REPOSITORY_ROOT,
            cls.mapping,
            cls.schema,
        )
        cls.document = builder.build_canary(cls.binding, cls.source_carriers)
        builder.validate_canary(
            cls.document, cls.binding, cls.source_carriers
        )

    def test_repository_mapping_and_checked_in_canary_are_current(self) -> None:
        validator.validate_mapping(self.mapping, self.schema)
        actual = validator.load_json(
            REPOSITORY_ROOT / self.binding["effectDocument"]
        )
        builder.validate_canary(actual, self.binding, self.source_carriers)
        self.assertEqual(
            builder.pretty_json_bytes(actual),
            builder.pretty_json_bytes(self.document),
        )
        self.assertEqual(
            actual["displayName"],
            "레이드 발탄_휠윈드 | 420633 Active | Portable Canary",
        )
        self.assertEqual(
            builder.EXPECTED_SOURCE_ACTION_DISPLAY_NAME,
            "레이드 발탄_휠윈드",
        )

    def test_exact_stage_002_occurrences_and_fail_closed_channels_are_pinned(self) -> None:
        self.assertEqual(
            tuple(row["notifyId"] for row in self.binding["sourceOccurrences"]),
            builder.EXPECTED_SOURCE_OCCURRENCE_IDS,
        )
        self.assertEqual(
            {row["notifyId"] for row in self.binding["failClosedOccurrences"]},
            set(builder.EXPECTED_FAIL_CLOSED),
        )
        self.assertTrue(
            all(
                row["disposition"] == "FAIL_CLOSED"
                for row in self.binding["failClosedOccurrences"]
            )
        )
        self.assertEqual(
            Counter(
                row["sourcePresentation"]["sourceEventId"]
                for row in self.document["elements"]
            ),
            Counter(
                {
                    "action-420633/stage-002/notify-004": 3,
                    "action-420633/stage-002/notify-005": 2,
                    "action-420633/stage-002/notify-006": 3,
                    "action-420633/stage-002/notify-009": 1,
                }
            ),
        )
        visible = [
            row for row in self.document["elements"] if row["visible"]
        ]
        self.assertEqual(len(self.document["elements"]), 9)
        self.assertEqual(len(visible), 5)
        self.assertEqual(
            Counter(
                row["sourcePresentation"]["sourceEventId"]
                for row in visible
            ),
            Counter(
                {
                    "action-420633/stage-002/notify-005": 2,
                    "action-420633/stage-002/notify-006": 3,
                }
            ),
        )

    def test_first_lod_denominator_and_portable_admission_are_quantified(self) -> None:
        carriers = [
            carrier
            for occurrence in self.binding["sourceOccurrences"]
            for carrier in occurrence["admission"]["carriers"]
        ]
        self.assertEqual(len(carriers), 9)
        self.assertEqual(
            sum(
                carrier["sourceRecipe"]["portableStatus"].startswith(
                    "PORTABLE_"
                )
                for carrier in carriers
            ),
            5,
        )
        self.assertEqual(
            sum(
                carrier["disposition"] == "VISIBLE_EXECUTABLE"
                for carrier in carriers
            ),
            9,
        )
        self.assertEqual(
            sum(carrier["sourceRecipe"]["moduleCount"] for carrier in carriers),
            89,
        )
        self.assertEqual(
            sum(
                carrier["sourceRecipe"]["distributionCount"]
                for carrier in carriers
            ),
            136,
        )

    def test_only_evidence_closed_wwind_carriers_are_executable(self) -> None:
        visible = [
            row for row in self.document["elements"] if row["visible"]
        ]
        self.assertEqual(
            [row["sourceRecipe"]["rendererShape"] for row in visible],
            ["sprite", "sprite", "sprite", "mesh", "mesh"],
        )
        self.assertEqual(
            [
                row["material"]["sourceProfile"]["runtimeShaderProfileId"]
                for row in visible
            ],
            [
                "effect.ue3.grouped-translucent.v1",
                "effect.ue3.aura.v1",
                "effect.ue3.grouped-translucent.v1",
                "effect.ue3.missiletrail-01.v1",
                "effect.ue3.missiletrail-01.v1",
            ],
        )
        self.assertTrue(
            all(row["sourceRecipe"]["enabled"] for row in visible)
        )
        hidden = [
            row for row in self.document["elements"] if not row["visible"]
        ]
        self.assertEqual(len(hidden), 4)
        self.assertTrue(
            all(row["sourceRecipe"]["enabled"] is False for row in hidden)
        )
        self.assertTrue(
            all(
                "execution" not in row["material"]
                for row in hidden
            )
        )
        self.assertTrue(
            all(
                row["detail"]["screenPost"]["enabled"] is False
                for row in self.document["elements"]
            )
        )
        self.assertNotIn(
            builder.CASCADE_RIBBON_MODULE_CLASS,
            {
                module["className"].casefold()
                for row in self.document["elements"]
                for module in row["sourceRecipe"]["modules"]
            },
        )
        typed_light = [
            row
            for row in hidden
            if row["sourcePresentation"]["sourceEventId"]
            != builder.ANIMATION_TRAIL_NOTIFY_ID
        ]
        self.assertEqual(len(typed_light), 1)
        self.assertTrue(all(row["resources"] == [] for row in typed_light))
        self.assertTrue(
            all(row["sourceRecipe"]["modules"] == [] for row in typed_light)
        )

    def test_animation_trail_is_data_preserved_but_execution_quarantined(self) -> None:
        deferred = [
            row
            for row in self.document["elements"]
            if row["sourcePresentation"]["sourceEventId"]
            == builder.ANIMATION_TRAIL_NOTIFY_ID
        ]
        mapped = {
            row["carrierId"]: row
            for row in self.binding["sourceOccurrences"][0]["admission"][
                "carriers"
            ]
        }
        self.assertEqual(len(deferred), 3)
        self.assertEqual(
            {
                row["id"]: [
                    (resource["slotId"], resource["assetId"])
                    for resource in row["resources"]
                ]
                for row in deferred
            },
            {
                "valtan.420633.notify004.emitter5259": [
                    (
                        "base",
                        "Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_006.dds",
                    ),
                    (
                        "noise",
                        "Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_067.dds",
                    ),
                    (
                        "mask",
                        "Effect/Valtan/Textures/FX_TEX_02/fx_d_decal_023.dds",
                    ),
                ],
                "valtan.420633.notify004.emitter5260": [
                    (
                        "mask",
                        "Effect/Valtan/Textures/FX_TEX_03/fx_e_fluid_003.dds",
                    ),
                    (
                        "base",
                        "Effect/Valtan/Textures/FX_TEX_03/fx_a_trail_003_ycl.dds",
                    ),
                    (
                        "noise",
                        "Effect/Valtan/Textures/FX_TEX_02/fx_d_noise_009.dds",
                    ),
                ],
                "valtan.420633.notify004.emitter5258": [
                    (
                        "noise",
                        "Effect/Valtan/Textures/FX_TEX_01/fx_c_noise_009.dds",
                    ),
                    (
                        "mask",
                        "Effect/Valtan/Textures/FX_TEX_00/fx_a_trail_011.dds",
                    ),
                    (
                        "dissolve",
                        "Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_009.dds",
                    ),
                    (
                        "base",
                        "Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_028.dds",
                    ),
                ],
            },
        )
        for row in deferred:
            recipe = row["sourceRecipe"]
            classes = [
                module["className"].casefold()
                for module in recipe["modules"]
            ]
            expected = mapped[row["id"]]["sourceRecipe"]
            self.assertFalse(row["visible"])
            self.assertEqual(row["kind"], "trail")
            self.assertFalse(recipe["enabled"])
            self.assertEqual(
                row["sourcePresentation"]["profileId"],
                builder.TYPED_ANIMATION_TRAIL_PROFILE_ID,
            )
            self.assertEqual(
                classes.count(builder.ANIMATION_TRAIL_MODULE_CLASS), 1
            )
            self.assertNotIn(builder.CASCADE_RIBBON_MODULE_CLASS, classes)
            self.assertEqual(len(recipe["modules"]), expected["moduleCount"])
            self.assertEqual(
                sum(
                    len(module["distributions"])
                    for module in recipe["modules"]
                ),
                expected["distributionCount"],
            )
            self.assertTrue(row["material"]["sourceProfile"]["enabled"])
            self.assertTrue(
                all(
                    (
                        REPOSITORY_ROOT
                        / builder.RUNTIME_RESOURCE_ROOT
                        / resource["assetId"]
                    ).is_file()
                    for resource in row["resources"]
                )
            )

    def test_mapping_validator_rejects_count_or_fail_closed_resource_leaks(self) -> None:
        mutated = copy.deepcopy(self.mapping)
        mutated["bindings"][0]["sourceOccurrences"][2]["admission"][
            "visibleExecutableCarrierCount"
        ] = 2
        with self.assertRaisesRegex(
            validator.ContractError, "carrier denominator/counts disagree"
        ):
            validator.validate_mapping(mutated, self.schema)

        mutated = copy.deepcopy(self.mapping)
        admission = mutated["bindings"][0]["sourceOccurrences"][1][
            "admission"
        ]
        hidden = admission["carriers"][0]
        hidden["disposition"] = "FAIL_CLOSED"
        hidden["blockers"] = ["TEST_BLOCKER"]
        hidden["materialAdmission"] = {"status": "FAIL_CLOSED"}
        admission["visibleExecutableCarrierCount"] = 1
        admission["failClosedCarrierCount"] = 1
        admission["executableProjection"] = "PARTIAL_PORTABLE_AUTHORED_V13"
        admission["blockers"] = ["TEST_BLOCKER"]
        with self.assertRaisesRegex(
            validator.ContractError, "fail-closed carrier leaked executable data"
        ):
            validator.validate_mapping(mutated, self.schema)

    def test_visible_runtime_resources_and_mesh_geometry_are_hash_pinned(self) -> None:
        for derived in self.source_carriers:
            carrier = derived["carrier"]
            if carrier["disposition"] != "VISIBLE_EXECUTABLE":
                continue
            for resource in carrier["resources"]:
                runtime_path = (
                    REPOSITORY_ROOT
                    / builder.RUNTIME_RESOURCE_ROOT
                    / resource["assetId"]
                )
                self.assertEqual(
                    builder.raw_sha256(runtime_path), resource["sha256"]
                )
            geometry = carrier.get("geometryEvidence")
            if geometry is not None:
                self.assertEqual(
                    geometry["runtimeSha256"],
                    next(
                        resource["sha256"]
                        for resource in carrier["resources"]
                        if resource["slotId"] == "meshModel"
                    ),
                )

    def test_gameplay_owned_fields_cannot_be_copied_into_mapping(self) -> None:
        mutated = copy.deepcopy(self.mapping)
        mutated["gameplayAuthority"]["durationMs"] = 1500
        with self.assertRaisesRegex(
            validator.ContractError,
            "unknown fields|duplicated gameplay-owned fields",
        ):
            validator.validate_mapping(mutated, self.schema)

        mutated = copy.deepcopy(self.mapping)
        mutated["gameplayAuthority"]["duplicatedGameplayFields"] = [
            "serverDamageProfileId"
        ]
        with self.assertRaisesRegex(
            validator.ContractError,
            "too many items|duplicatedGameplayFields",
        ):
            validator.validate_mapping(mutated, self.schema)

    def test_unresolved_effect_material_and_viewshake_must_remain_fail_closed(self) -> None:
        mutated = copy.deepcopy(self.mapping)
        mutated["bindings"][0]["failClosedOccurrences"] = mutated["bindings"][0][
            "failClosedOccurrences"
        ][:-1]
        validator.validate_mapping(mutated, self.schema)
        with self.assertRaisesRegex(builder.ContractError, "fail-closed occurrence set"):
            builder._validate_fixed_identity(mutated)

        mutated = copy.deepcopy(self.mapping)
        row = mutated["bindings"][0]["failClosedOccurrences"][-2]
        row["reason"] = "GENERIC_FALLBACK"
        with self.assertRaisesRegex(builder.ContractError, "fail-closed admission changed"):
            builder.validate_source_contract(REPOSITORY_ROOT, mutated, self.schema)

    def test_graph_root_and_action_qualified_clip_drift_are_rejected(self) -> None:
        mutated = copy.deepcopy(self.mapping)
        mutated["bindings"][0]["sourceOccurrences"][2]["sourceSystem"][
            "graphRootNodeId"
        ] = "FX_MN_RPBF_00_N:export:1"
        with self.assertRaisesRegex(builder.ContractError, "source graph root changed"):
            builder.validate_source_contract(REPOSITORY_ROOT, mutated, self.schema)

        mutated = copy.deepcopy(self.mapping)
        mutated["bindings"][0]["sourceBranch"]["runtimeClipName"] = (
            "mesh_att_battle_20_02"
        )
        with self.assertRaisesRegex(builder.ContractError, "source branch identity changed"):
            builder.validate_source_contract(REPOSITORY_ROOT, mutated, self.schema)

        mutated = copy.deepcopy(self.mapping)
        mutated["bindings"][0]["sourceOccurrences"][1]["attachment"][
            "sourceAnchorSlotId"
        ] = ""
        with self.assertRaisesRegex(builder.ContractError, "dropped serialized B_EffectRoot"):
            builder.validate_source_contract(REPOSITORY_ROOT, mutated, self.schema)

    def test_animation_binding_accepts_v1_and_exactly_joins_v2_occurrence(self) -> None:
        binding = copy.deepcopy(self.binding)
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            relative = Path("Valtan.patternbindings.json")
            binding["animationBindingDocument"] = relative.as_posix()

            v1 = {
                "schema": "lostark.valtan-pattern-bindings",
                "formatVersion": 1,
                "bindings": [
                    {
                        "actionId": builder.EXPECTED_ACTION_ID,
                        "clip": builder.EXPECTED_RUNTIME_CLIP,
                    }
                ],
            }
            (root / relative).write_text(
                json.dumps(v1), encoding="utf-8"
            )
            builder._validate_animation_binding(root, binding)

            v2 = {
                "schema": "lostark.valtan-pattern-bindings",
                "formatVersion": 2,
                "bindings": [
                    {
                        "actionId": builder.EXPECTED_ACTION_ID,
                        "clips": [
                            {
                                "clipOccurrenceId": (
                                    f"{builder.EXPECTED_ACTION_ID}.clip.99"
                                ),
                                "clip": "mesh_decoy",
                            },
                            {
                                "clipOccurrenceId": (
                                    builder.EXPECTED_CLIP_OCCURRENCE_ID
                                ),
                                "clip": builder.EXPECTED_RUNTIME_CLIP,
                            },
                        ],
                    }
                ],
            }
            (root / relative).write_text(
                json.dumps(v2), encoding="utf-8"
            )
            builder._validate_animation_binding(root, binding)

            v2["bindings"][0]["clips"][1]["clipOccurrenceId"] = (
                f"{builder.EXPECTED_ACTION_ID}.clip.98"
            )
            (root / relative).write_text(
                json.dumps(v2), encoding="utf-8"
            )
            with self.assertRaisesRegex(
                builder.ContractError,
                "canonical Whirlwind v2 clip occurrence",
            ):
                builder._validate_animation_binding(root, binding)

    def test_physical_b_effectroot_is_unique_casefold_evidence(self) -> None:
        bone = self.binding["modelBoneEvidence"]
        model_path = (
            REPOSITORY_ROOT
            / builder.RUNTIME_RESOURCE_ROOT
            / bone["runtimeModelAssetId"]
        )
        rows = builder.read_wmodel_bones(model_path)
        derived = builder.derive_bone_evidence(
            rows,
            "B_EffectRoot",
            bone["runtimeModelAssetId"],
            builder.raw_sha256(model_path),
        )
        self.assertEqual(derived, bone)
        self.assertEqual(derived["runtimeBoneName"], "b_effectroot")
        self.assertEqual(derived["boneIndex"], 83)
        self.assertEqual(derived["matchPolicy"], "UNIQUE_ASCII_CASEFOLD")

    def test_missing_or_ambiguous_bone_never_falls_back_to_root(self) -> None:
        with self.assertRaisesRegex(builder.ContractError, "root fallback is forbidden"):
            builder.derive_bone_evidence(
                [builder.BoneRow(0, "RootNode", -1, 1)],
                "B_EffectRoot",
                "Character/Valtan/MN_RPBF_01.wmodel",
                "0" * 64,
            )
        with self.assertRaisesRegex(builder.ContractError, "root fallback is forbidden"):
            builder.derive_bone_evidence(
                [
                    builder.BoneRow(0, "b_effectroot", -1, 1),
                    builder.BoneRow(1, "B_EFFECTROOT", 0, 2),
                ],
                "B_EffectRoot",
                "Character/Valtan/MN_RPBF_01.wmodel",
                "0" * 64,
            )

    def test_builder_is_deterministic_and_check_detects_stale_output(self) -> None:
        self.assertEqual(
            builder.pretty_json_bytes(
                builder.build_canary(self.binding, self.source_carriers)
            ),
            builder.pretty_json_bytes(
                builder.build_canary(self.binding, self.source_carriers)
            ),
        )
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / "effect.json"
            builder.write_transactionally(self.document, output)
            builder.build_and_write(
                REPOSITORY_ROOT,
                output_path=output,
                check=True,
            )
            output.write_bytes(output.read_bytes() + b" ")
            with self.assertRaisesRegex(builder.ContractError, "output is stale"):
                builder.build_and_write(
                    REPOSITORY_ROOT,
                    output_path=output,
                    check=True,
                )

    def test_rebuild_preserves_legacy_crlf_canary_bytes(self) -> None:
        source = REPOSITORY_ROOT / self.binding["effectDocument"]
        expected = source.read_bytes()
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / "effect.json"
            output.write_bytes(expected)
            builder.build_and_write(REPOSITORY_ROOT, output_path=output)
            self.assertEqual(expected, output.read_bytes())

    def test_invalid_mapping_preserves_previous_output(self) -> None:
        mutated = copy.deepcopy(self.mapping)
        mutated["bindings"][0]["sourceOccurrences"][0]["notifyId"] = (
            "action-420633/stage-002/notify-999"
        )
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            mapping_path = temporary_root / "invalid.json"
            mapping_path.write_text(
                json.dumps(mutated, ensure_ascii=False, indent=2) + "\n",
                encoding="utf-8",
            )
            output = temporary_root / "previous.effect.json"
            previous = b"previous-valid-output\n"
            output.write_bytes(previous)
            with self.assertRaises(builder.ContractError):
                builder.build_and_write(
                    REPOSITORY_ROOT,
                    mapping_path=mapping_path,
                    output_path=output,
                )
            self.assertEqual(output.read_bytes(), previous)

    def test_recipe_material_resource_and_geometry_drift_fail_before_write(self) -> None:
        mutations = []

        source_graph = copy.deepcopy(self.mapping)
        source_graph["bindings"][0]["sourceEvidence"][
            "sourceGraphDocuments"
        ][2]["sha256"] = "0" * 64
        mutations.append((source_graph, "source graph SHA-256 changed"))

        runtime_cook = copy.deepcopy(self.mapping)
        runtime_cook["bindings"][0]["sourceEvidence"][
            "runtimeCookReceiptSha256"
        ] = "0" * 64
        mutations.append((runtime_cook, "runtime cook receipt SHA-256 changed"))

        recipe = copy.deepcopy(self.mapping)
        recipe["bindings"][0]["sourceOccurrences"][2]["admission"][
            "carriers"
        ][0]["sourceRecipe"]["portableRecipeSha256"] = "0" * 64
        mutations.append((recipe, "portable SourceRecipe admission changed"))

        material = copy.deepcopy(self.mapping)
        material["bindings"][0]["sourceOccurrences"][2]["admission"][
            "carriers"
        ][0]["materialAdmission"]["evidenceSha256"] = "0" * 64
        mutations.append((material, "source Material contract SHA-256 changed"))

        resource = copy.deepcopy(self.mapping)
        resource["bindings"][0]["sourceOccurrences"][2]["admission"][
            "carriers"
        ][0]["resources"][0]["sha256"] = "0" * 64
        mutations.append((resource, "runtime resource evidence changed"))

        geometry = copy.deepcopy(self.mapping)
        geometry["bindings"][0]["sourceOccurrences"][2]["admission"][
            "carriers"
        ][1]["geometryEvidence"]["sourceGltfSha256"] = "0" * 64
        mutations.append((geometry, "source Mesh geometry evidence changed"))

        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            for index, (mutated, message) in enumerate(mutations):
                mapping_path = temporary_root / f"invalid-{index}.json"
                mapping_path.write_text(
                    json.dumps(mutated, ensure_ascii=False, indent=2) + "\n",
                    encoding="utf-8",
                )
                output = temporary_root / f"previous-{index}.effect.json"
                previous = b"previous-valid-output\n"
                output.write_bytes(previous)
                with self.assertRaisesRegex(builder.ContractError, message):
                    builder.build_and_write(
                        REPOSITORY_ROOT,
                        mapping_path=mapping_path,
                        output_path=output,
                    )
                self.assertEqual(output.read_bytes(), previous)


if __name__ == "__main__":
    unittest.main()

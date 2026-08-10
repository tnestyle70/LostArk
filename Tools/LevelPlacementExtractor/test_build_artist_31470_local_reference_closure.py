from __future__ import annotations

import copy
import tempfile
import unittest
from contextlib import ExitStack
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

import build_artist_31470_local_reference_closure as closure
import build_artist_31470_source_evidence as source_evidence
import extract_ue3_particle_graph as particle_graph
import extract_ue3_particle_module_closure as module_closure
from effect_source_contract_io import (
    generated_text_matches,
    raw_file_sha256,
    tracked_text_sha256,
)


def tagged(value: object, value_type: str = "FloatProperty") -> dict:
    return closure.tagged_value(value, value_type)


def raw_distribution_reference(package_index: int) -> dict:
    return {
        "type": "structproperty",
        "structType": "rawdistributionvector",
        "value": {
            "properties": {
                "distribution": {
                    "type": "objectproperty",
                    "structType": None,
                    "value": package_index,
                }
            }
        },
    }


def particle_target(
    class_name: str,
    properties: dict,
    *,
    object_path: str = "test.module.distribution_0",
) -> dict:
    return {
        "className": class_name,
        "objectPath": object_path,
        "properties": copy.deepcopy(properties),
    }


def current_target(
    class_name: str,
    *,
    archetype_path: str = "",
) -> dict:
    return {
        "className": class_name,
        "classPath": f"Engine.{class_name}",
        "objectPath": "test.module.distribution_0",
        "archetypeIndex": -1 if archetype_path else 0,
        "archetypePath": archetype_path,
        "properties": {},
    }


def current_defaults() -> dict:
    empty_record = {
        "className": "",
        "classPath": "",
        "objectPath": "",
        "properties": {},
    }
    return {
        "classDefaults": {
            "efVectorMultiply": {
                **empty_record,
                "objectPath": (
                    "EFGame.Default__EFDistributionVectorMultiplyParticleParameter"
                ),
                "properties": {},
            },
            "locationSkipArchetype": {
                **empty_record,
                "objectPath": (
                    "EFGame.Default__EFParticleModuleLocationOnGround."
                    "DistributionLocationSkip"
                ),
                "properties": {"Constant": tagged(1.0)},
            },
            "floatParticleParameter": {
                "status": "CURRENT_NATIVE_CLASS_CDO_NOT_SERIALIZED",
                "classPath": "Engine.DistributionFloatParticleParameter",
                "properties": {},
            },
            "vectorParticleParameter": {
                "status": "CURRENT_NATIVE_CLASS_CDO_NOT_SERIALIZED",
                "classPath": "Engine.DistributionVectorParticleParameter",
                "properties": {},
            },
            "floatParameterBase": {
                **empty_record,
                "objectPath": "Engine.Default__DistributionFloatParameterBase",
                "properties": {},
            },
            "vectorParameterBase": {
                **empty_record,
                "objectPath": "Engine.Default__DistributionVectorParameterBase",
                "properties": {},
            },
        },
        "evaluatorDefaults": {
            "floatParticleParameter": {
                "status": "CURRENT_NATIVE_EVALUATOR_SHAPE_ONLY",
                "properties": {},
            },
            "vectorParticleParameter": {
                "status": "CURRENT_NATIVE_EVALUATOR_SHAPE_ONLY",
                "properties": {},
            },
        },
    }


def direct_vector_properties(*, include_constant: bool) -> dict:
    result = {
        "ParameterName": tagged("Color", "NameProperty"),
        "ParamModes": tagged("dpm_direct", "ByteProperty"),
        "ParamModes[1]": tagged("dpm_direct", "ByteProperty"),
        "ParamModes[2]": tagged("dpm_direct", "ByteProperty"),
    }
    if include_constant:
        result["Constant"] = tagged(
            {"x": 0.25, "y": 0.5, "z": 0.75}, "StructProperty"
        )
    return result


def range_float_properties(mode: str) -> dict:
    return {
        "ParameterName": tagged("Rate", "NameProperty"),
        "ParamMode": tagged(mode, "ByteProperty"),
        "MinInput": tagged(-1.0),
        "MaxInput": tagged(1.0),
        "MinOutput": tagged(2.0),
        "MaxOutput": tagged(4.0),
        "Constant": tagged(3.0),
    }


def mocked_export() -> SimpleNamespace:
    return SimpleNamespace(
        index=0,
        class_index=-11,
        object_name="distributionfloatparticleparameter_0",
        archetype_index=-22,
        serial_offset=0,
        serial_size=4,
    )


def mocked_package_ref_path(value: int, _imports: list, _exports: list) -> str:
    return {
        -11: "Engine.DistributionFloatParticleParameter",
        -22: (
            "EFGame.Default__EFParticleModuleLocationOnGround."
            "DistributionLocationSkip"
        ),
        1: "module.distributionfloatparticleparameter_0",
    }[value]


def patch_package_decoder(stack: ExitStack, module: object) -> None:
    stack.enter_context(
        patch.object(module, "parse_summary", return_value=SimpleNamespace(version=1))
    )
    stack.enter_context(
        patch.object(module, "decompress_package", return_value=b"\0" * 8)
    )
    stack.enter_context(patch.object(module, "parse_name_table", return_value=[]))
    stack.enter_context(patch.object(module, "parse_import_table", return_value=[]))
    stack.enter_context(
        patch.object(module, "parse_export_table", return_value=[mocked_export()])
    )
    stack.enter_context(
        patch.object(
            module,
            "parse_tagged_properties",
            return_value=({}, 4),
        )
    )
    stack.enter_context(
        patch.object(
            module,
            "package_ref_name",
            return_value="distributionfloatparticleparameter",
        )
    )
    stack.enter_context(
        patch.object(module, "package_ref_path", side_effect=mocked_package_ref_path)
    )


class Artist31470LocalReferenceClosureTests(unittest.TestCase):
    def semantics(
        self,
        class_name: str,
        properties: dict,
        *,
        defaults: dict | None = None,
        archetype_path: str = "",
    ) -> dict:
        return closure.particle_parameter_semantics(
            particle_target(class_name, properties),
            current_target(class_name, archetype_path=archetype_path),
            defaults or current_defaults(),
            "SOURCE_EXACT_PHYSICAL_PACKAGE",
        )

    def test_parent_quorum_does_not_prove_mutated_child(self) -> None:
        pinned_parent = {
            "className": "particlemodulemeshrotation",
            "objectName": "particlemodulemeshrotation_12",
            "objectPath": "parent.module",
            "properties": {"StartRotation": raw_distribution_reference(12)},
            "references": [
                {
                    "property": "StartRotation.Distribution",
                    "packageIndex": 12,
                    "objectPath": "parent.module.child_0",
                }
            ],
        }
        recovery_parent = copy.deepcopy(pinned_parent)
        recovery_child_before = {
            "Constant": tagged({"x": 0.0, "y": 0.0, "z": 0.0})
        }
        recovery_child_after = {
            "Constant": tagged({"x": 1.0, "y": 1.0, "z": 0.6})
        }

        before = closure.cross_revision_module_quorum(
            pinned_parent, recovery_parent, recovery_child_before
        )
        after = closure.cross_revision_module_quorum(
            pinned_parent, recovery_parent, recovery_child_after
        )

        self.assertNotEqual(
            before["recoveryChildRecordSha256"],
            after["recoveryChildRecordSha256"],
        )
        self.assertTrue(after["semanticEqual"])
        self.assertFalse(after["provesChildTargetPayloadEqual"])
        self.assertFalse(after["sourceExact"])
        self.assertIn(
            "CROSS_REVISION_TARGET_PAYLOAD_NOT_SOURCE_EXACT",
            after["blockers"],
        )

    def test_parent_mutation_breaks_quorum(self) -> None:
        pinned = {
            "className": "particlemodulemeshrotation",
            "objectName": "particlemodulemeshrotation_12",
            "objectPath": "parent.module",
            "properties": {"StartRotation": raw_distribution_reference(12)},
            "references": [
                {
                    "property": "StartRotation.Distribution",
                    "packageIndex": 12,
                    "objectPath": "parent.module.child_0",
                }
            ],
        }
        recovery = copy.deepcopy(pinned)
        recovery["references"][0]["objectPath"] = "parent.module.other_child_0"

        result = closure.cross_revision_module_quorum(pinned, recovery)

        self.assertFalse(result["semanticEqual"])
        self.assertFalse(result["sourceExact"])
        self.assertIn(
            "CROSS_REVISION_SOURCE_MODULE_QUORUM_FAILED", result["blockers"]
        )

    def test_occurrence_level_reference_rows_are_canonicalized_for_quorum(self) -> None:
        reference = {
            "sourceNodeId": "FX_PC_SDM_07:export:812",
            "property": "StartRotation.Distribution",
            "packageIndex": 12,
            "objectPath": "parent.module.child_0",
        }
        graph = {
            "sourceSystems": [
                {"unresolvedExternalReferences": [copy.deepcopy(reference)]},
                {"unresolvedExternalReferences": [copy.deepcopy(reference)]},
            ]
        }

        result = closure.canonical_source_node_references(
            graph, "FX_PC_SDM_07:export:812"
        )

        self.assertEqual(
            result,
            [
                {
                    "property": "startrotation.distribution",
                    "packageIndex": 12,
                    "objectPath": "parent.module.child_0",
                }
            ],
        )

    def test_standard_constant_curve_is_not_custom_particle_parameter(self) -> None:
        semantics = closure.particle_parameter_semantics(
            particle_target("distributionfloatconstantcurve", {}),
            current_target("distributionfloatconstantcurve"),
            current_defaults(),
            "SOURCE_EXACT_PHYSICAL_PACKAGE",
        )

        result = closure.evaluate_particle_parameter_occurrence(semantics, [])

        self.assertEqual(semantics["evaluatorKind"], "STANDARD_CONSTANT_CURVE")
        self.assertEqual(result["branch"], "STANDARD_CONSTANT_CURVE_SOURCE_PAYLOAD")
        self.assertNotIn(
            "CUSTOM_EF_DISTRIBUTION_EVALUATOR_UNPROVEN", result["blockers"]
        )
        self.assertIn("CONSTANT_CURVE_COMPILER_NOT_IMPLEMENTED", result["blockers"])

    def test_nested_archetype_precedes_generic_cdo(self) -> None:
        defaults = current_defaults()
        defaults["classDefaults"]["floatParticleParameter"]["properties"] = {
            "Constant": tagged(7.0)
        }
        defaults["classDefaults"]["floatParameterBase"]["properties"] = {
            "Constant": tagged(9.0)
        }
        archetype_path = (
            "EFGame.Default__EFParticleModuleLocationOnGround."
            "DistributionLocationSkip"
        )

        semantics = self.semantics(
            "distributionfloatparticleparameter",
            {},
            defaults=defaults,
            archetype_path=archetype_path,
        )
        selected = semantics["resolvedFields"]["constant"]["selected"]

        self.assertEqual(selected["tier"], "NESTED_ARCHETYPE_TEMPLATE")
        self.assertEqual(selected["evidenceStatus"], "CURRENT_REVISION_ARCHETYPE_EVIDENCE")
        self.assertEqual(selected["value"]["value"], 1.0)
        self.assertEqual(semantics["status"], "SEMANTIC_BLOCKED")
        self.assertIn("CLASS_DEFAULT_ARCHETYPE_UNPROVEN", semantics["blockers"])

    def test_nested_archetype_constant_mutation_changes_evidence_and_stays_blocked(
        self,
    ) -> None:
        defaults = current_defaults()
        archetype_path = (
            "EFGame.Default__EFParticleModuleLocationOnGround."
            "DistributionLocationSkip"
        )
        before = self.semantics(
            "distributionfloatparticleparameter",
            {},
            defaults=defaults,
            archetype_path=archetype_path,
        )
        before_record = closure.record_sha256(
            defaults["classDefaults"]["locationSkipArchetype"]
        )

        changed = copy.deepcopy(defaults)
        changed["classDefaults"]["locationSkipArchetype"]["properties"][
            "Constant"
        ] = tagged(2.0)
        after = self.semantics(
            "distributionfloatparticleparameter",
            {},
            defaults=changed,
            archetype_path=archetype_path,
        )
        after_record = closure.record_sha256(
            changed["classDefaults"]["locationSkipArchetype"]
        )

        self.assertEqual(
            before["resolvedFields"]["constant"]["selected"]["value"]["value"],
            1.0,
        )
        self.assertEqual(
            after["resolvedFields"]["constant"]["selected"]["value"]["value"],
            2.0,
        )
        self.assertNotEqual(before_record, after_record)
        self.assertEqual(after["status"], "SEMANTIC_BLOCKED")
        self.assertIn("CURRENT_REVISION_ARCHETYPE_EVIDENCE", after["blockers"])

    def test_direct_input_present_ignores_ranges(self) -> None:
        semantics = self.semantics(
            "distributionvectorparticleparameter",
            direct_vector_properties(include_constant=False),
        )

        result = closure.evaluate_particle_parameter_occurrence(
            semantics,
            [
                {
                    "name": "color",
                    "type": "vector",
                    "vectorValue": [1.0, 0.5, 0.25],
                }
            ],
        )

        self.assertEqual(result["branch"], "DIRECT_INPUT")
        self.assertTrue(result["allowed"])
        self.assertEqual(
            result["ignoredRangeFields"],
            ["mininput", "maxinput", "minoutput", "maxoutput"],
        )

    def test_direct_input_miss_uses_resolved_constant(self) -> None:
        semantics = self.semantics(
            "distributionvectorparticleparameter",
            direct_vector_properties(include_constant=True),
        )

        result = closure.evaluate_particle_parameter_occurrence(semantics, [])

        self.assertEqual(result["branch"], "CONSTANT_FALLBACK")
        self.assertTrue(result["allowed"])
        self.assertEqual(result["blockers"], [])

    def test_direct_input_miss_without_constant_is_blocked(self) -> None:
        semantics = self.semantics(
            "distributionvectorparticleparameter",
            direct_vector_properties(include_constant=False),
        )

        result = closure.evaluate_particle_parameter_occurrence(semantics, [])

        self.assertEqual(result["branch"], "UNRESOLVED_FALLBACK")
        self.assertFalse(result["allowed"])
        self.assertIn("CUE_PARAMETER_BINDING_MISSING", result["blockers"])
        self.assertIn(
            "PARAMETER_FALLBACK_CONSTANT_UNRESOLVED", result["blockers"]
        )

    def test_normal_and_abs_require_all_four_ranges(self) -> None:
        for mode in ("dpm_normal", "dpm_abs"):
            with self.subTest(mode=mode, case="complete"):
                complete = self.semantics(
                    "distributionfloatparticleparameter",
                    range_float_properties(mode),
                )
                self.assertEqual(complete["status"], "SEMANTIC_SOURCE_READY")
                self.assertNotIn(
                    "PARAMETER_RANGE_FOUR_FIELDS_INCOMPLETE",
                    complete["blockers"],
                )

            for field in ("MinInput", "MaxInput", "MinOutput", "MaxOutput"):
                with self.subTest(mode=mode, missing=field):
                    properties = range_float_properties(mode)
                    properties.pop(field)
                    incomplete = self.semantics(
                        "distributionfloatparticleparameter", properties
                    )
                    self.assertEqual(incomplete["status"], "SEMANTIC_BLOCKED")
                    self.assertIn(
                        "PARAMETER_RANGE_FOUR_FIELDS_INCOMPLETE",
                        incomplete["blockers"],
                    )

    def test_custom_particleparameter_never_uses_standard_evaluator(self) -> None:
        class_name = "EFDistributionVectorMultiplyParticleParameter"
        properties = direct_vector_properties(include_constant=True)
        properties.update(
            {
                "MinInput": tagged(
                    {"x": 0.0, "y": 0.0, "z": 0.0}, "StructProperty"
                ),
                "MaxInput": tagged(
                    {"x": 1.0, "y": 1.0, "z": 1.0}, "StructProperty"
                ),
                "MinOutput": tagged(
                    {"x": 0.0, "y": 0.0, "z": 0.0}, "StructProperty"
                ),
                "MaxOutput": tagged(
                    {"x": 1.0, "y": 1.0, "z": 1.0}, "StructProperty"
                ),
            }
        )

        semantics = self.semantics(class_name, properties)
        evaluation = closure.evaluate_particle_parameter_occurrence(
            semantics,
            [{"name": "Color", "type": "vector", "vectorValue": [1, 1, 1]}],
        )

        self.assertFalse(closure.standard_particle_parameter_class(class_name))
        self.assertTrue(closure.custom_particle_parameter_class(class_name))
        self.assertEqual(
            semantics["evaluatorKind"], "CUSTOM_EF_DISTRIBUTION_UNPROVEN"
        )
        self.assertEqual(semantics["status"], "SEMANTIC_BLOCKED")
        self.assertIn(
            "CUSTOM_EF_DISTRIBUTION_EVALUATOR_UNPROVEN", semantics["blockers"]
        )
        self.assertEqual(evaluation["branch"], "UNRESOLVED_CUSTOM_EVALUATOR")
        self.assertFalse(evaluation["allowed"])

    def test_target012_missing_or_wrong_type_cue_blocks_fallback(self) -> None:
        semantics = self.semantics(
            "distributionvectorparticleparameter",
            direct_vector_properties(include_constant=False),
        )

        missing = closure.evaluate_particle_parameter_occurrence(semantics, [])
        mismatch = closure.evaluate_particle_parameter_occurrence(
            semantics,
            [{"name": "Color", "type": "scalar", "scalarValue": 1.0}],
        )

        self.assertFalse(missing["allowed"])
        self.assertIn("CUE_PARAMETER_BINDING_MISSING", missing["blockers"])
        self.assertIn(
            "PARAMETER_FALLBACK_CONSTANT_UNRESOLVED", missing["blockers"]
        )
        self.assertFalse(mismatch["allowed"])
        self.assertIn("CUE_PARAMETER_TYPE_MISMATCH", mismatch["blockers"])
        self.assertIn(
            "PARAMETER_FALLBACK_CONSTANT_UNRESOLVED", mismatch["blockers"]
        )

    def test_qualified_distribution_join_uses_logical_package_tuple(self) -> None:
        left = closure.qualified_object_identity(
            "FX_A", "FX_A.Shared.Module.Distribution_0"
        )
        same = closure.qualified_object_identity(
            "fx_a", "shared.module.distribution_0"
        )
        collision = closure.qualified_object_identity(
            "FX_B", "Shared.Module.Distribution_0"
        )

        self.assertEqual(left, same)
        self.assertNotEqual(left, collision)
        self.assertEqual(left, ("fx_a", "shared.module.distribution_0"))

    def test_point_light_instance_and_cdo_mutations_follow_precedence(self) -> None:
        instance = {
            "Brightness": tagged(10.0),
            "bCastCompositeShadow": tagged(False, "BoolProperty"),
            "bAffectCompositeShadowDirection": tagged(False, "BoolProperty"),
        }
        archetype = {"Radius": tagged(200.0)}
        class_cdo = {"FalloffExponent": tagged(2.0)}
        parent_cdo = {
            "LightColor": tagged(
                {"r": 255, "g": 255, "b": 255, "a": 0}, "StructProperty"
            )
        }

        baseline = closure.resolve_point_light_fields(
            instance, archetype, class_cdo, parent_cdo
        )
        self.assertEqual(
            baseline["radius"]["selected"]["tier"], "NESTED_ARCHETYPE_TEMPLATE"
        )
        self.assertEqual(
            baseline["radius"]["selected"]["value"]["value"], 200.0
        )
        self.assertEqual(
            baseline["falloffexponent"]["selected"]["tier"], "CLASS_CDO"
        )
        self.assertEqual(
            baseline["lightcolor"]["selected"]["tier"], "PARENT_CDO_HIERARCHY"
        )

        changed_instance = copy.deepcopy(instance)
        changed_instance["Radius"] = tagged(350.0)
        instance_override = closure.resolve_point_light_fields(
            changed_instance, archetype, class_cdo, parent_cdo
        )
        self.assertEqual(
            instance_override["radius"]["selected"]["tier"], "INSTANCE_EXPLICIT"
        )
        self.assertEqual(
            instance_override["radius"]["selected"]["value"]["value"], 350.0
        )

        changed_cdo = copy.deepcopy(class_cdo)
        changed_cdo["FalloffExponent"] = tagged(3.0)
        cdo_mutation = closure.resolve_point_light_fields(
            instance, archetype, changed_cdo, parent_cdo
        )
        self.assertEqual(
            cdo_mutation["falloffexponent"]["selected"]["value"]["value"],
            3.0,
        )

    def test_point_light_reference_removal_type_and_path_mutations_fail(self) -> None:
        source = {
            "properties": {
                "PointLightComponent": {
                    "type": "ObjectProperty",
                    "value": 7055,
                }
            }
        }
        candidates = [
            {
                "property": "pointlightcomponent",
                "packageIndex": 7055,
                "objectPath": closure.POINT_LIGHT_PATH,
            }
        ]

        resolved = closure.resolve_typed_component_reference(
            source,
            candidates,
            "PointLightComponent",
            7055,
            closure.POINT_LIGHT_PATH,
        )
        self.assertEqual(resolved["sourcePackageIndex"], 7055)

        with self.assertRaisesRegex(ValueError, "missing or ambiguous"):
            closure.resolve_typed_component_reference(
                source,
                [],
                "PointLightComponent",
                7055,
                closure.POINT_LIGHT_PATH,
            )

        wrong_type = copy.deepcopy(source)
        wrong_type["properties"]["PointLightComponent"]["type"] = "IntProperty"
        with self.assertRaisesRegex(ValueError, "non-zero object reference"):
            closure.resolve_typed_component_reference(
                wrong_type,
                candidates,
                "PointLightComponent",
                7055,
                closure.POINT_LIGHT_PATH,
            )

        wrong_path = copy.deepcopy(candidates)
        wrong_path[0]["objectPath"] = "FX_CM_02.PointLightComponent_Changed"
        with self.assertRaisesRegex(ValueError, "identity changed"):
            closure.resolve_typed_component_reference(
                source,
                wrong_path,
                "PointLightComponent",
                7055,
                closure.POINT_LIGHT_PATH,
            )

    def test_raw_and_tracked_hash_apis_preserve_distinct_eol_contracts(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            lf = root / "derived.json"
            crlf = root / "external.particle-graph.json"
            lf.write_bytes(b'{\n  "value": 1\n}\n')
            crlf.write_bytes(b'{\r\n  "value": 1\r\n}\r\n')

            self.assertEqual(tracked_text_sha256(lf), tracked_text_sha256(crlf))
            self.assertNotEqual(raw_file_sha256(lf), raw_file_sha256(crlf))
            self.assertTrue(generated_text_matches(crlf, lf.read_bytes()))

            binary_lf = root / "source-a.upk"
            binary_crlf = root / "source-b.upk"
            binary_lf.write_bytes(b"payload\n")
            binary_crlf.write_bytes(b"payload\r\n")
            self.assertNotEqual(
                raw_file_sha256(binary_lf), raw_file_sha256(binary_crlf)
            )

    def test_pinned_source_graph_crlf_mutation_fails_raw_graph_sha(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            graph_path = root / "TEST.particle-graph.json"
            lf_payload = b'{\n  "objects": []\n}\n'
            graph_path.write_bytes(lf_payload)
            receipt = {
                "sourcePackages": [
                    {
                        "logicalPackage": "TEST",
                        "graphSha256": raw_file_sha256(graph_path),
                    }
                ]
            }

            objects, pins = source_evidence.load_pinned_source_graphs(
                receipt, root
            )
            self.assertEqual(objects, {})
            self.assertEqual(pins[0]["sha256"], receipt["sourcePackages"][0]["graphSha256"])

            graph_path.write_bytes(lf_payload.replace(b"\n", b"\r\n"))
            with self.assertRaisesRegex(ValueError, "pinned source graph hash changed"):
                source_evidence.load_pinned_source_graphs(receipt, root)

    def test_particle_graph_extractor_preserves_class_and_archetype_identity(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as directory:
            package_path = Path(directory) / "mock.upk"
            package_path.write_bytes(b"package")
            with ExitStack() as stack:
                patch_package_decoder(stack, particle_graph)
                result = particle_graph.extract_package(
                    package_path, "TEST", "unused-key"
                )

        self.assertEqual(len(result["objects"]), 1)
        row = result["objects"][0]
        self.assertEqual(
            row["classPath"], "Engine.DistributionFloatParticleParameter"
        )
        self.assertEqual(row["archetypeIndex"], -22)
        self.assertEqual(
            row["archetypePath"],
            (
                "EFGame.Default__EFParticleModuleLocationOnGround."
                "DistributionLocationSkip"
            ),
        )

    def test_module_closure_extractor_preserves_class_and_archetype_identity(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as directory:
            package_root = Path(directory)
            package_path = package_root / "mock.upk"
            package_path.write_bytes(b"package")
            requests = [
                {
                    "sourceNodeId": "TEST:export:7",
                    "referenceIndex": 1,
                    "property": "StartSize.Distribution",
                    "sourceSystemId": "test.system",
                    "objectPath": "TEST.module.distributionfloatparticleparameter_0",
                    "relativeObjectPath": (
                        "module.distributionfloatparticleparameter_0"
                    ),
                }
            ]
            with ExitStack() as stack:
                patch_package_decoder(stack, module_closure)
                stack.enter_context(
                    patch.object(
                        module_closure,
                        "obfuscate_package_name",
                        return_value="mock",
                    )
                )
                result = module_closure.extract_requested_package(
                    package_root, "TEST", requests, "unused-key"
                )

        self.assertEqual(len(result["objects"]), 1)
        row = result["objects"][0]
        self.assertEqual(
            row["classPath"], "Engine.DistributionFloatParticleParameter"
        )
        self.assertEqual(row["archetypeIndex"], -22)
        self.assertEqual(
            row["archetypePath"],
            (
                "EFGame.Default__EFParticleModuleLocationOnGround."
                "DistributionLocationSkip"
            ),
        )


if __name__ == "__main__":
    unittest.main()

from __future__ import annotations

import copy
import hashlib
import json
import struct
import sys
import tempfile
import unittest
from pathlib import Path

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
if str(REPOSITORY_ROOT) not in sys.path:
    sys.path.insert(0, str(REPOSITORY_ROOT))

from Tools.EffectPipeline import evaluate_ue3_material_uniform_expressions as subject


def _fname(names: list[str], name: str, number: int = 0) -> bytes:
    return struct.pack("<ii", names.index(name), number)


def _tag(
    names: list[str],
    name: str,
    property_type: str,
    payload: bytes,
    *,
    struct_type: str | None = None,
) -> bytes:
    result = bytearray()
    result += _fname(names, name)
    result += _fname(names, property_type)
    result += struct.pack("<ii", len(payload), 0)
    if struct_type is not None:
        result += _fname(names, struct_type)
    result += payload
    return bytes(result)


def _parameter_item(
    names: list[str],
    name: str,
    number: int,
    value: float | tuple[float, float, float, float],
) -> bytes:
    result = bytearray()
    result += _tag(names, "ParameterName", "NameProperty", _fname(names, name, number))
    if isinstance(value, tuple):
        result += _tag(
            names,
            "ParameterValue",
            "StructProperty",
            struct.pack("<4f", *value),
            struct_type="LinearColor",
        )
    else:
        result += _tag(
            names,
            "ParameterValue",
            "FloatProperty",
            struct.pack("<f", value),
        )
    result += _fname(names, "None")
    return bytes(result)


def _glasshole_scalar_values() -> list[float]:
    values = [0.0] * subject.GLASSHOLE_SCALAR_EXPRESSION_COUNT
    packed_rows = {
        1: [2.5, -15.0, -7.5, 1.0],
        7: [0.0, 0.0, -7.0, 0.2],
        8: [0.0, 2.0, 1.0, 0.3],
        9: [0.0, 0.0, 1.0, 0.1],
        10: [0.0, 2.0, -0.8, 1.0],
        11: [1.0, 0.0, 0.0, 0.0],
        12: [0.0, 0.0, 0.0, 2.0],
        13: [5.0, -0.0, 0.0, 0.8],
    }
    for group, row in packed_rows.items():
        values[group * 4 : group * 4 + 4] = row
    return values


def _glasshole_native_binding() -> dict[str, object]:
    return {
        "constantBufferClosure": {
            "declaredConstantBuffer0Float4Count": 22,
        },
        "scalarGroups": [
            {
                "expressionIndexOrGroup": group,
                "baseIndex": slot * 16,
            }
            for group, slot in subject.GLASSHOLE_SCALAR_GROUP_WIRES
        ],
    }


def _glasshole_evaluation(evidence: dict[str, object]) -> dict[str, object]:
    values = _glasshole_scalar_values()
    material_rows = []
    for group in evidence["groupsInAscendingCb0SlotOrder"]:
        indices = group["projectedExpressionIndices"]
        material_rows.append(
            {
                "slot": group["slot"],
                "source": "PIXEL_SCALAR_EXPRESSION_GROUP",
                "expressionIndices": indices,
                "value": [subject._float32(values[index]) for index in indices],
            }
        )
    return {
        "pixelScalarExpressionValues": values,
        "nativeCb0": {
            "materialRows": material_rows,
            "nativeScalarGroupPackingEvidence": evidence,
        },
    }


class SourceValueUniformEvaluatorTests(unittest.TestCase):
    def test_raw_mic_parameter_decode_preserves_fname_number(self) -> None:
        names = [
            "None",
            "ScalarParameterValues",
            "VectorParameterValues",
            "Parent",
            "ArrayProperty",
            "ObjectProperty",
            "ParameterName",
            "ParameterValue",
            "NameProperty",
            "FloatProperty",
            "StructProperty",
            "LinearColor",
            "Rate",
            "Tint",
        ]
        scalar_payload = struct.pack("<i", 1) + _parameter_item(
            names, "Rate", 3, 2.5
        )
        vector_payload = struct.pack("<i", 1) + _parameter_item(
            names, "Tint", 1, (1.0, 2.0, 3.0, 4.0)
        )
        serial = bytearray(struct.pack("<i", 0))
        serial += _tag(
            names, "ScalarParameterValues", "ArrayProperty", scalar_payload
        )
        serial += _tag(
            names, "VectorParameterValues", "ArrayProperty", vector_payload
        )
        serial += _tag(
            names, "Parent", "ObjectProperty", struct.pack("<i", -1)
        )
        serial += _fname(names, "None")

        decoded = subject.decode_mic_numeric_overrides(bytes(serial), names, 868)
        self.assertEqual(
            decoded["scalarOverrides"][0]["parameterKey"],
            {"nameCasefold": "rate", "number": 3},
        )
        self.assertEqual(decoded["scalarOverrides"][0]["value"], 2.5)
        self.assertEqual(
            decoded["vectorOverrides"][0]["parameterKey"],
            {"nameCasefold": "tint", "number": 1},
        )
        self.assertEqual(
            decoded["vectorOverrides"][0]["value"], [1.0, 2.0, 3.0, 4.0]
        )

    def test_parent_to_leaf_merge_is_casefolded_but_number_distinct(self) -> None:
        def row(name: str, number: int, value: float) -> dict[str, object]:
            return {
                "parameterName": name,
                "parameterNameNumber": number,
                "parameterKey": {"nameCasefold": name.casefold(), "number": number},
                "value": value,
            }

        layers = [
            {
                "objectPath": "Parent.MI",
                "scalarOverrides": [row("Rate", 0, 1.0), row("Rate", 2, 2.0)],
                "vectorOverrides": [],
            },
            {
                "objectPath": "Child.MI",
                "scalarOverrides": [row("rATE", 0, 3.0)],
                "vectorOverrides": [],
            },
        ]
        effective = subject.merge_effective_overrides(layers, "scalarOverrides")
        by_key = {
            (item["parameterKey"]["nameCasefold"], item["parameterKey"]["number"]): item
            for item in effective
        }
        self.assertEqual(by_key[("rate", 0)]["value"], 3.0)
        self.assertEqual(by_key[("rate", 0)]["sourceObjectPath"], "Child.MI")
        self.assertEqual(by_key[("rate", 2)]["value"], 2.0)
        self.assertEqual(by_key[("rate", 2)]["sourceObjectPath"], "Parent.MI")

    def test_folded_math_zero_is_add_and_two_is_multiply(self) -> None:
        constant_a = {
            "typeName": "fmaterialuniformexpressionconstant",
            "value": [1.0, 2.0, 3.0, 4.0],
        }
        constant_b = {
            "typeName": "fmaterialuniformexpressionconstant",
            "value": [5.0, 6.0, 7.0, 8.0],
        }

        def folded(ordinal: int) -> dict[str, object]:
            return {
                "typeName": "fmaterialuniformexpressionfoldedmath",
                "a": constant_a,
                "b": constant_b,
                "operationOrdinal": ordinal,
            }

        kwargs = {
            "game_time_seconds": 0.0,
            "real_time_seconds": 0.0,
        }
        self.assertEqual(
            subject.evaluate_expression(folded(0), {}, {}, **kwargs),
            [6.0, 8.0, 10.0, 12.0],
        )
        self.assertEqual(
            subject.evaluate_expression(folded(2), {}, {}, **kwargs),
            [5.0, 12.0, 21.0, 32.0],
        )
        with self.assertRaisesRegex(ValueError, "not source-proven"):
            subject.evaluate_expression(folded(1), {}, {}, **kwargs)

    def test_periodic_sine_and_append_use_source_proven_opcode_meaning(self) -> None:
        time = {"typeName": "fmaterialuniformexpressiontime"}
        periodic = {
            "typeName": "fmaterialuniformexpressionperiodic",
            "input": time,
        }
        sine = {
            "typeName": "fmaterialuniformexpressionsine",
            "input": {
                "typeName": "fmaterialuniformexpressionconstant",
                "value": [0.0, 0.0, 0.0, 0.0],
            },
            "isCosine": True,
        }
        append = {
            "typeName": "fmaterialuniformexpressionappendvector",
            "a": periodic,
            "b": sine,
            "componentsFromA": 2,
        }
        result = subject.evaluate_expression(
            append,
            {},
            {},
            game_time_seconds=2.25,
            real_time_seconds=0.0,
        )
        self.assertEqual(result, [0.25, 0.25, 1.0, 1.0])

    def test_periodic_uses_signed_fractional_for_negative_input(self) -> None:
        periodic = {
            "typeName": "fmaterialuniformexpressionperiodic",
            "input": {
                "typeName": "fmaterialuniformexpressionconstant",
                "value": [-1.25, -0.25, 1.25, -0.0],
            },
        }
        result = subject.evaluate_expression(
            periodic,
            {},
            {},
            game_time_seconds=0.0,
            real_time_seconds=0.0,
        )
        self.assertEqual(result[:3], [-0.25, -0.25, 0.25])
        self.assertEqual(struct.pack("<f", result[3]), struct.pack("<f", 0.0))

    def test_native_cb0_pack_preserves_engine_holes(self) -> None:
        uniform_set = {
            "pixelVectorExpressions": [
                {
                    "typeName": "fmaterialuniformexpressionvectorparameter",
                    "parameterName": "Tint",
                    "parameterNameNumber": 0,
                    "defaultValue": [1.0, 2.0, 3.0, 4.0],
                }
            ],
            "pixelScalarExpressions": [
                {
                    "typeName": "fmaterialuniformexpressionconstant",
                    "value": [float(index + 1)] * 4,
                }
                for index in range(5)
            ],
        }
        native = {
            "constantBufferClosure": {
                "declaredConstantBuffer0Float4Count": 4,
                "boundConstantBuffer0Slots": [1, 2, 3],
            },
            "scalarGroups": [
                {
                    "expressionIndexOrGroup": 0,
                    "baseIndex": 16,
                },
                {
                    "expressionIndexOrGroup": 1,
                    "baseIndex": 32,
                },
            ],
            "vectors": [
                {
                    "expressionIndexOrGroup": 0,
                    "baseIndex": 48,
                }
            ],
        }
        result = subject.evaluate_uniform_set_into_cb0(
            uniform_set,
            native,
            [],
            [],
            {"gameTimeSeconds": 0.0, "realTimeSeconds": 0.0},
        )
        rows = result["nativeCb0"]["allRows"]
        self.assertIsNone(rows[0]["value"])
        self.assertEqual(rows[1]["value"], [1.0, 2.0, 3.0, 4.0])
        self.assertEqual(rows[2]["value"], [5.0, 0.0, 0.0, 0.0])
        self.assertEqual(rows[3]["value"], [1.0, 2.0, 3.0, 4.0])
        evidence = result["nativeCb0"]["nativeScalarGroupPackingEvidence"]
        self.assertEqual(evidence["partialGroupCount"], 1)
        self.assertFalse(evidence["targetPaddingFree"])
        self.assertFalse(evidence["targetPaddingFreeCorroborationAdmission"])
        self.assertEqual(
            evidence["fidelity"], subject.PACKED_SCALAR_PADDING_UNRESOLVED
        )
        self.assertIsNone(evidence["packedLittleEndianByteCount"])
        self.assertIsNone(evidence["packedLittleEndianSha256"])
        partial = evidence["groupsInAscendingCb0SlotOrder"][1]
        self.assertFalse(partial["completeFourLaneProjection"])
        self.assertEqual(partial["projectedExpressionIndices"], [4, 5, 6, 7])
        self.assertEqual(partial["presentExpressionIndices"], [4])

    def test_glasshole_native_scalar_groups_are_padding_free_and_hash_sealed(self) -> None:
        values = _glasshole_scalar_values()
        evidence = subject.project_native_scalar_group_packing(
            values, _glasshole_native_binding()
        )
        subject.validate_target_native_scalar_group_packing(
            subject.GLASSHOLE_TARGET_ID, _glasshole_evaluation(evidence)
        )

        self.assertEqual(evidence["fidelity"], subject.PACKED_SCALAR_FIDELITY)
        self.assertEqual(evidence["selectedGroupCount"], 8)
        self.assertEqual(evidence["completeGroupCount"], 8)
        self.assertEqual(evidence["partialGroupCount"], 0)
        self.assertEqual(
            evidence["packedLittleEndianByteCount"],
            subject.GLASSHOLE_PACKED_SCALAR_BYTE_COUNT,
        )
        self.assertEqual(
            evidence["packedLittleEndianSha256"],
            subject.GLASSHOLE_PACKED_SCALAR_SHA256,
        )
        self.assertEqual(
            [
                [
                    subject._float32(values[index])
                    for index in row["projectedExpressionIndices"]
                ]
                for row in evidence["groupsInAscendingCb0SlotOrder"]
            ],
            [
                [2.5, -15.0, -7.5, 1.0],
                [0.0, 0.0, -7.0, subject._float32(0.2)],
                [0.0, 2.0, 1.0, subject._float32(0.3)],
                [0.0, 0.0, 1.0, subject._float32(0.1)],
                [0.0, 2.0, subject._float32(-0.8), 1.0],
                [1.0, 0.0, 0.0, 0.0],
                [0.0, 0.0, 0.0, 2.0],
                [5.0, -0.0, 0.0, subject._float32(0.8)],
            ],
        )

    def test_glasshole_lane_swap_and_group_mutation_fail_closed(self) -> None:
        values = _glasshole_scalar_values()
        evidence = subject.project_native_scalar_group_packing(
            values, _glasshole_native_binding()
        )
        lane_swapped = copy.deepcopy(evidence)
        indices = lane_swapped["groupsInAscendingCb0SlotOrder"][0][
            "projectedExpressionIndices"
        ]
        indices[0], indices[1] = indices[1], indices[0]
        with self.assertRaisesRegex(ValueError, "lane projection changed"):
            subject.validate_native_scalar_group_packing_evidence(
                lane_swapped, values
            )

        mutated_native = _glasshole_native_binding()
        mutated_native["scalarGroups"][0]["expressionIndexOrGroup"] = 2
        group_mutated = subject.project_native_scalar_group_packing(
            values, mutated_native
        )
        with self.assertRaisesRegex(ValueError, "Glasshole scalar group wire changed"):
            subject.validate_target_native_scalar_group_packing(
                subject.GLASSHOLE_TARGET_ID,
                _glasshole_evaluation(group_mutated),
            )

    def test_tracked_receipt_seals_only_glasshole_target_padding_free(self) -> None:
        receipt_path = REPOSITORY_ROOT / (
            "Data/Effects/Imported/DimensionMaster/Materials/"
            "skill.2050120.clip3.source-value-uniform-evaluation.receipt.json"
        )
        receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
        subject.validate_output_receipt(receipt)
        admitted = [
            target
            for target in receipt["targets"]
            if target["sourceValueUniformEvaluation"]["nativeCb0"]
            ["nativeScalarGroupPackingEvidence"]
            ["targetPaddingFreeCorroborationAdmission"]
        ]
        self.assertEqual(
            [target["targetId"] for target in admitted],
            [subject.GLASSHOLE_TARGET_ID],
        )
        for target in receipt["targets"]:
            self.assertFalse(target["sourceValueReplayAdmission"])
            self.assertFalse(target["actualVfPassAdmission"])
            self.assertFalse(target["runtimeAdmission"])
            self.assertFalse(target["visualAdmission"])
        self.assertFalse(receipt["scope"]["nativeScalarGroupPackingSourceClosed"])

    def test_official_source_semantics_are_verified_not_assumed(self) -> None:
        text = """
enum EFoldedMathOperation
{
    FMO_Add, FMO_Sub, FMO_Mul, FMO_Div, FMO_Dot, FMO_Cross
};
case FMO_Add: OutData.WriteOpcode(UE::Shader::EPreshaderOpcode::Add); break;
case FMO_Mul: OutData.WriteOpcode(UE::Shader::EPreshaderOpcode::Mul); break;
class FMaterialUniformExpressionPeriodic {};
EPreshaderOpcode::Fractional;
class FMaterialUniformExpressionSine {};
bIsCosine ? UE::Shader::EPreshaderOpcode::Cos : UE::Shader::EPreshaderOpcode::Sin;
""".lstrip()
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "MaterialUniformExpressions.h"
            path.write_text(text, encoding="utf-8")
            expected = {
                "repository": "EpicGames/UnrealEngine",
                "tag": "test",
                "commit": "test",
                "rawSha256": hashlib.sha256(path.read_bytes()).hexdigest(),
                "evidence": {
                    "foldedMathOperationOrder": [
                        "FMO_Add",
                        "FMO_Sub",
                        "FMO_Mul",
                        "FMO_Div",
                        "FMO_Dot",
                        "FMO_Cross",
                    ]
                },
            }
            evidence = subject.validate_official_source(path, expected)
        self.assertEqual(evidence["verifiedSemantics"]["foldedMathOrdinal0"], "ADD")
        self.assertEqual(evidence["verifiedSemantics"]["foldedMathOrdinal2"], "MUL")


if __name__ == "__main__":
    unittest.main()

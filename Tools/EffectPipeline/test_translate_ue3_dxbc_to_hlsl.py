#!/usr/bin/env python3
"""Unit tests for the cooked DXBC to HLSL translator.

The cases here are the ones that produced silently wrong HLSL during
development, so each is pinned rather than described: simultaneous vector
writes, positional swizzle indexing, the integer domain of comparisons and
bitwise selects, and hard failure on an opcode with no known spelling.
"""

from __future__ import annotations

import hashlib
import io
import json
import tempfile
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))

import translate_ue3_dxbc_to_hlsl as translator  # noqa: E402
from translate_ue3_dxbc_to_hlsl import (  # noqa: E402
    DEFAULT_D3DCOMPILER,
    Operand,
    TranslationError,
    _SAMPLE_SEQUENCE,
    emit_compile_probe,
    emit_module,
    parse_declarations,
    translate_body,
    translate_instruction,
    validate_module_compiles,
)


def translate(text: str) -> list[str]:
    _SAMPLE_SEQUENCE["value"] = 0
    return translate_instruction(text)


class OperandTest(unittest.TestCase):
    def test_source_swizzle_is_indexed_by_absolute_component(self) -> None:
        """`v2.yxyy` supplies y to component x and x to component y."""
        operand = Operand("v2.yxyy")
        self.assertEqual(operand.component(0), "v2.y")
        self.assertEqual(operand.component(1), "v2.x")
        self.assertEqual(operand.component(2), "v2.y")

    def test_single_component_swizzle_replicates(self) -> None:
        operand = Operand("cb0[17].z")
        self.assertEqual(operand.component(0), "cb0[17].z")
        self.assertEqual(operand.component(3), "cb0[17].z")

    def test_absolute_then_negate_order_is_preserved(self) -> None:
        self.assertEqual(Operand("-|r0.x|").component(0), "-(abs(r0.x))")

    def test_hex_immediate_is_a_raw_float_bit_pattern(self) -> None:
        operand = Operand("l(0xc0490fdb)")
        self.assertTrue(operand.component(0).startswith("-3.14159"))

    def test_literal_lane_selection_follows_the_write_mask(self) -> None:
        operand = Operand("l(0,0,1.000000,0)")
        self.assertEqual(operand.component(2), "1.0f")
        self.assertEqual(operand.component(3), "asfloat(0u)")

    def test_integer_immediate_is_a_bit_pattern_not_a_value(self) -> None:
        """`l(5)` in `bfi` is the integer 5, never the float 5.0."""
        self.assertEqual(Operand("l(5)").component(0), "asfloat(5u)")
        self.assertEqual(Operand("l(5.000000)").component(0), "5.0f")

    def test_reduction_takes_the_leading_components(self) -> None:
        self.assertEqual(Operand("r0.xyzx").vector(3), "r0.xyz")
        self.assertEqual(Operand("|r0.xyzx|").vector(3), "abs(r0.xyz)")
        self.assertEqual(Operand("cb0[17].wwww").vector(2), "cb0[17].ww")


class SimultaneousWriteTest(unittest.TestCase):
    def test_multi_component_write_stages_every_lane_first(self) -> None:
        """`mul r0.xyz, r0.xxxx, r1.xyzx` must not feed the new r0.x forward.

        This is the defect that made three of five recovered programs differ
        from their cooked originals: assigning r0.x before evaluating r0.y
        multiplies the second lane by the already-updated value.
        """
        lines = translate("mul r0.xyz, r0.xxxx, r1.xyzx")
        writes = [line for line in lines if line.startswith("r0.")]
        reads = [line for line in lines if not line.startswith("r0.")]
        self.assertEqual(len(reads), 3)
        self.assertEqual(len(writes), 3)
        for read in reads:
            self.assertIn("r0.x * r1.", read)
        self.assertTrue(
            all(lines.index(read) < lines.index(write)
                for read in reads for write in writes))

    def test_single_component_write_needs_no_staging(self) -> None:
        self.assertEqual(
            translate("mul r0.y, r0.y, r1.y"), ["r0.y = r0.y * r1.y;"])

    def test_sincos_stages_the_angle_once(self) -> None:
        lines = translate("sincos r0.x, r0.y, r0.x")
        self.assertTrue(lines[0].startswith("float stage"))
        self.assertTrue(any("sin(stage" in line for line in lines))
        self.assertTrue(any("cos(stage" in line for line in lines))


class IntegerDomainTest(unittest.TestCase):
    def test_comparison_writes_a_lane_mask(self) -> None:
        self.assertEqual(
            translate("lt r1.y, |r0.y|, |r0.x|"),
            ["r1.y = asfloat((abs(r0.y) < abs(r0.x)) ? 0xffffffffu : 0u);"])

    def test_bitwise_and_stays_in_the_integer_domain(self) -> None:
        line = translate("and r1.x, r1.y, r1.x")[0]
        self.assertIn("asuint(r1.y) & asuint(r1.x)", line)
        self.assertTrue(line.startswith("r1.x = asfloat("))

    def test_movc_tests_the_mask_not_the_float(self) -> None:
        line = translate("movc r0.y, r0.y, -r0.z, r0.z")[0]
        self.assertIn("asuint(r0.y) != 0u ? -r0.z : r0.z", line)


class OpcodeTest(unittest.TestCase):
    def test_dot_product_reduces_before_broadcast(self) -> None:
        self.assertEqual(
            translate("dp2 r0.x, r0.xyxx, r0.xyxx"),
            ["r0.x = dot(r0.xy, r0.xy);"])

    def test_exponentials_are_base_two(self) -> None:
        self.assertEqual(translate("exp r0.y, r0.y"), ["r0.y = exp2(r0.y);"])
        self.assertEqual(translate("log r0.y, r0.y"), ["r0.y = log2(r0.y);"])

    def test_saturate_suffix_wraps_the_result(self) -> None:
        self.assertEqual(
            translate("mul_sat r0.x, r0.y, r0.z"),
            ["r0.x = saturate(r0.y * r0.z);"])

    def test_sample_swizzle_is_indexed_by_destination_component(self) -> None:
        lines = translate(
            "sample_l_indexable(texture2d)(float,float,float,float) "
            "r0.yzw, r0.yzyy, t4.wxyz, s4, l(-1.000000)")
        self.assertIn("t4.SampleLevel(s4, r0.yz, -1.0f)", lines[0])
        self.assertTrue(lines[1].endswith(".x;"))
        self.assertTrue(lines[2].endswith(".y;"))
        self.assertTrue(lines[3].endswith(".z;"))

    def test_sample_bias_uses_the_bias_overload(self) -> None:
        lines = translate(
            "sample_b_indexable(texture2d)(float,float,float,float) "
            "r1.w, r3.xyxx, t0.yzwx, s1, l(0.000000)")
        self.assertIn("t0.SampleBias(s1, r3.xy, 0.0f)", lines[0])

    def test_return_emits_nothing(self) -> None:
        self.assertEqual(translate("ret"), [])

    def test_unknown_opcode_is_a_hard_failure(self) -> None:
        """An approximation here would compile, run, and be wrong."""
        with self.assertRaises(TranslationError):
            translate("atomic_iadd u0, r0.x, r0.y")


class ControlFlowTest(unittest.TestCase):
    def test_masked_materials_discard_on_the_lane_mask(self) -> None:
        self.assertEqual(
            translate("discard_nz r0.x"),
            ["if (asuint(r0.x) != 0u) discard;"])

    def test_loop_and_break_keep_their_structure(self) -> None:
        self.assertEqual(translate("loop"), ["while (true) {"])
        self.assertEqual(translate("endloop"), ["}"])
        self.assertEqual(
            translate("breakc_nz r1.w"),
            ["if (asuint(r1.w) != 0u) break;"])

    def test_screen_derivatives_map_to_their_hlsl_names(self) -> None:
        self.assertEqual(
            translate("deriv_rtx_coarse r0.xy, v2.xyxx"),
            ["float stage1_x = ddx_coarse(v2.x);",
             "float stage1_y = ddx_coarse(v2.y);",
             "r0.x = stage1_x;",
             "r0.y = stage1_y;"])

    def test_coverage_mask_write_stays_in_the_integer_domain(self) -> None:
        self.assertEqual(
            translate("mov oMask, r0.x"), ["oMask = asuint(r0.x);"])


class ProgramBodyTest(unittest.TestCase):
    def test_a_trailing_return_is_dropped(self) -> None:
        body = translate_body(["mov r0.x, l(1.0)", "ret"], "")
        self.assertNotIn("ret", "".join(body))

    def test_an_early_return_is_refused(self) -> None:
        """Skipping it would run instructions the cooked program never does."""
        with self.assertRaises(TranslationError):
            translate_body(["ret", "mov r0.x, l(1.0)"], "")


class DeclarationTest(unittest.TestCase):
    def test_declarations_are_recovered(self) -> None:
        parsed = parse_declarations([
            "dcl_constantbuffer CB0[22], immediateIndexed",
            "dcl_constantbuffer CB2[4], immediateIndexed",
            "dcl_sampler s0, mode_default",
            "dcl_resource_texture2d (float,float,float,float) t3",
            "dcl_input_ps linear centroid v7.xyw",
            "dcl_output o2.xyzw",
            "dcl_output oMask",
            "dcl_temps 4",
        ])
        self.assertEqual(parsed["constantBuffers"], {"cb0": 22, "cb2": 4})
        self.assertEqual(parsed["samplers"], ["s0"])
        self.assertEqual(parsed["textures"], ["t3"])
        self.assertEqual(parsed["outputs"], ["o2"])
        self.assertTrue(parsed["coverageMask"])
        self.assertEqual(parsed["temporaryCount"], 4)
        self.assertEqual(parsed["inputs"][0]["register"], "v7")
        self.assertTrue(parsed["inputs"][0]["centroid"])


class StoredModuleCompileTest(unittest.TestCase):
    @staticmethod
    def coverage_disassembly() -> dict[str, object]:
        return {
            "declarations": [
                "dcl_constantbuffer CB0[2], immediateIndexed",
                "dcl_constantbuffer CB2[1], immediateIndexed",
                "dcl_input_ps linear v0.xy",
                "dcl_input vCoverage",
                "dcl_output o0.xyzw",
                "dcl_output oMask",
                "dcl_temps 1",
            ],
            "instructions": [
                "mov o0.xyzw, cb0[0].xyzw",
                "mov oMask, vCoverage.x",
                "ret",
            ],
        }

    def test_module_preserves_registers_and_declares_coverage_input(self) -> None:
        disassembly = self.coverage_disassembly()
        source = emit_module(disassembly, "Shade_TestCoverage")

        self.assertIn(
            "cbuffer Shade_TestCoverage_Constants0 : register(b0)", source)
        self.assertIn(
            "cbuffer Shade_TestCoverage_Constants2 : register(b2)", source)
        self.assertIn("uint4 vCoverage;", source)
        self.assertIn("uint4 vCoverage = stage.vCoverage;", source)

        declarations = parse_declarations(disassembly["declarations"])
        probe = emit_compile_probe(declarations, "Shade_TestCoverage")
        self.assertIn("uint coverage : SV_Coverage", probe)
        self.assertIn("stage.vCoverage = coverage;", probe)

    def test_coverage_module_compiles_as_the_stored_source(self) -> None:
        disassembly = self.coverage_disassembly()
        declarations = parse_declarations(disassembly["declarations"])
        source = emit_module(disassembly, "Shade_TestCoverage")

        validate_module_compiles(
            source,
            declarations,
            "Shade_TestCoverage",
            DEFAULT_D3DCOMPILER,
        )

    def test_compile_gate_rejects_an_undeclared_coverage_register(self) -> None:
        disassembly = self.coverage_disassembly()
        declarations = parse_declarations(disassembly["declarations"])
        source = emit_module(disassembly, "Shade_TestCoverage").replace(
            "    uint4 vCoverage = stage.vCoverage;\n", "")

        with self.assertRaisesRegex(
                TranslationError, "stored HLSLI module failed to compile"):
            validate_module_compiles(
                source,
                declarations,
                "Shade_TestCoverage",
                DEFAULT_D3DCOMPILER,
            )


class ArtifactWriterTest(unittest.TestCase):
    def test_same_family_leaf_with_different_programs_gets_unique_names(self) -> None:
        first_digest = "1" * 64
        second_digest = "2" * 64
        receipt = {
            "families": [
                {
                    "parentMaterialPath": "package_a.group.fx_same_tr",
                    "dxbcSha256": first_digest,
                },
                {
                    "parentMaterialPath": "package_b.group.fx_same_tr",
                    "dxbcSha256": second_digest,
                },
            ]
        }

        with tempfile.TemporaryDirectory(prefix="dxbc-names-") as temporary:
            receipt_path = Path(temporary) / "receipt.json"
            receipt_path.write_text(json.dumps(receipt), encoding="utf-8")
            names = translator.family_function_names(receipt_path)

        self.assertEqual(
            names[first_digest], "Shade_Ue3_fx_same_tr_" + first_digest
        )
        self.assertEqual(
            names[second_digest], "Shade_Ue3_fx_same_tr_" + second_digest
        )
        self.assertEqual(len(set(names.values())), 2)

    def test_duplicate_explicit_output_name_fails_before_writing(self) -> None:
        with tempfile.TemporaryDirectory(prefix="dxbc-duplicate-name-") as temporary:
            root = Path(temporary)
            first = root / "first.dxbc"
            second = root / "second.dxbc"
            output = root / "TranslatedShaders"
            first.write_bytes(b"DXBC-1")
            second.write_bytes(b"DXBC-2")
            with mock.patch.object(translator, "translate_blob") as translate:
                with redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()):
                    status = translator.main(
                        [
                            str(first),
                            str(second),
                            "--function-name",
                            "Shade_Duplicate",
                            "--output-directory",
                            str(output),
                        ]
                    )

            self.assertEqual(status, 1)
            translate.assert_not_called()
            self.assertFalse(output.exists())

    def test_hlsli_and_report_are_lf_and_hash_the_written_bytes(self) -> None:
        source = "// generated\nfloat4 Shade_Test()\n{\n    return 0;\n}\n"
        source_digest = hashlib.sha256(source.encode("utf-8")).hexdigest()
        result = {
            "functionName": "Shade_Test",
            "profile": "ps_5_0",
            "instructionCount": 1,
            "declarations": {},
            "dxbcSha256": "a" * 64,
            "hlslSha256": source_digest,
            "hlsl": source,
        }

        with tempfile.TemporaryDirectory(prefix="dxbc-translation-") as temporary:
            root = Path(temporary)
            dxbc = root / "input.dxbc"
            output = root / "TranslatedShaders"
            report = root / "effect-family-hlsl-translations.v1.json"
            dxbc.write_bytes(b"DXBC")
            with mock.patch.object(translator, "translate_blob", return_value=result):
                with redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()):
                    status = translator.main(
                        [
                            str(dxbc),
                            "--function-name",
                            "Shade_Test",
                            "--output-directory",
                            str(output),
                            "--report",
                            str(report),
                        ]
                    )

            self.assertEqual(status, 0)
            hlsli_bytes = (output / "Shade_Test.hlsli").read_bytes()
            report_bytes = report.read_bytes()
            self.assertEqual(hlsli_bytes, source.encode("utf-8"))
            self.assertNotIn(b"\r\n", hlsli_bytes)
            self.assertNotIn(b"\r\n", report_bytes)
            report_row = json.loads(report_bytes.decode("utf-8"))[0]
            self.assertEqual(report_row["hlslSha256"], source_digest)
            self.assertEqual(
                hashlib.sha256(hlsli_bytes).hexdigest(),
                report_row["hlslSha256"],
            )

    def test_hash_bearing_checkout_paths_are_forced_to_lf(self) -> None:
        attributes = (Path(__file__).resolve().parents[2] / ".gitattributes").read_text(
            encoding="utf-8"
        )
        self.assertIn(
            "/Data/Effects/TranslatedShaders/*.hlsli text eol=lf", attributes
        )
        self.assertIn(
            "/Data/Effects/Contracts/effect-family-hlsl-translations.v1.json "
            "text eol=lf",
            attributes,
        )
        self.assertIn(
            "/Data/Effects/Contracts/effect-family-shader-inventory.v1.json "
            "text eol=lf",
            attributes,
        )


if __name__ == "__main__":
    unittest.main()

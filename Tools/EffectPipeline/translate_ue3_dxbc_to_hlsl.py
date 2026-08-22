#!/usr/bin/env python3
"""Translate a cooked UE3 pixel-shader DXBC program into readable HLSL.

Hand-translating one family took a session each, which is why 33 families are
implemented and the rest are not.  The programs themselves do not justify that
cost: across the recovered cooked pixel shaders the whole instruction set is a
few dozen arithmetic, comparison and sample opcodes with no control flow, and
every one of them has a direct HLSL spelling.

This translator emits that spelling.  It is deliberately literal - one DXBC
instruction becomes one HLSL statement against the same registers, so the
result can be diffed against the disassembly and replayed against the original
blob with identical inputs.  It does not name material parameters, choose a
vertex factory, or decide a render state; those are separate evidence, and a
translation that guessed them would be a reconstruction wearing a translation's
name.

An instruction this translator does not know is a hard failure.  Emitting an
approximation for an unrecognised opcode would produce HLSL that compiles,
runs, and is quietly wrong.
"""

from __future__ import annotations

import argparse
import ctypes
import hashlib
import json
import re
import struct
import sys
from pathlib import Path
from typing import Any

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
LEVEL_TOOLS = REPOSITORY_ROOT / "Tools" / "LevelPlacementExtractor"
if str(LEVEL_TOOLS) not in sys.path:
    sys.path.insert(0, str(LEVEL_TOOLS))

from extract_artist_31470_main_ref_shader_cache import (  # noqa: E402
    DEFAULT_D3DCOMPILER,
    D3DDisassembler,
)

COMPONENTS = "xyzw"

# Component-wise opcodes: for every enabled destination component c the
# operand's component c is read.  The expression template receives the already
# swizzled operands.
COMPONENTWISE = {
    "mov": "{0}",
    "add": "{0} + {1}",
    "mul": "{0} * {1}",
    "mad": "{0} * {1} + {2}",
    "div": "{0} / {1}",
    "min": "min({0}, {1})",
    "max": "max({0}, {1})",
    "exp": "exp2({0})",
    "log": "log2({0})",
    "sqrt": "sqrt({0})",
    "rsq": "rsqrt({0})",
    "frc": "frac({0})",
    "round_ni": "floor({0})",
    "round_pi": "ceil({0})",
    "round_z": "trunc({0})",
    "round_ne": "round({0})",
    "rcp": "rcp({0})",
    "deriv_rtx": "ddx({0})",
    "deriv_rty": "ddy({0})",
    "deriv_rtx_coarse": "ddx_coarse({0})",
    "deriv_rty_coarse": "ddy_coarse({0})",
    "deriv_rtx_fine": "ddx_fine({0})",
    "deriv_rty_fine": "ddy_fine({0})",
    "sincos": None,
}

# Structural opcodes with no destination.  Loops in these programs are the
# fixed-trip kind UE3 emits for layered samples, so the HLSL shape is a
# `while (true)` closed by the program's own `break`.
CONTROL_FLOW = {
    "loop": "while (true) {",
    "endloop": "}",
    "else": "} else {",
    "endif": "}",
    "break": "break;",
    "continue": "continue;",
}

CONTROL_FLOW_WITH_CONDITION = {
    "if_nz": "if (asuint({0}) != 0u) {{",
    "if_z": "if (asuint({0}) == 0u) {{",
    "breakc_nz": "if (asuint({0}) != 0u) break;",
    "breakc_z": "if (asuint({0}) == 0u) break;",
    "continuec_nz": "if (asuint({0}) != 0u) continue;",
    "continuec_z": "if (asuint({0}) == 0u) continue;",
    "discard_nz": "if (asuint({0}) != 0u) discard;",
    "discard_z": "if (asuint({0}) == 0u) discard;",
}

COVERAGE_OUTPUT = "oMask"
COVERAGE_INPUT = "vCoverage"

# Conversions and integer arithmetic keep the register's bit pattern honest:
# a DXBC register is typeless, so the domain is a property of the opcode, not
# of the storage.  `integer` marks the ones whose result is a bit pattern.
CONVERSIONS = {
    "ftou": ("(uint)({0})", True),
    "ftoi": ("(int)({0})", True),
    "utof": ("(float)asuint({0})", False),
    "itof": ("(float)asint({0})", False),
}

INTEGER_ARITHMETIC = {
    "iadd": "asint({0}) + asint({1})",
    "ineg": "-asint({0})",
    "imin": "min(asint({0}), asint({1}))",
    "imax": "max(asint({0}), asint({1}))",
    "umin": "min(asuint({0}), asuint({1}))",
    "umax": "max(asuint({0}), asuint({1}))",
    "ishl": "asint({0}) << asuint({1})",
    "ishr": "asint({0}) >> asuint({1})",
    "ushr": "asuint({0}) >> asuint({1})",
}

INTEGER_COMPARISONS = {
    "ieq": "asint({0}) == asint({1})",
    "ine": "asint({0}) != asint({1})",
    "ilt": "asint({0}) < asint({1})",
    "ige": "asint({0}) >= asint({1})",
    "ult": "asuint({0}) < asuint({1})",
    "uge": "asuint({0}) >= asuint({1})",
}

# Comparisons write an integer lane mask, not a float.  Keeping them in the
# integer domain is what makes the `and`/`movc` idioms below exact.
COMPARISONS = {
    "lt": "{0} < {1}",
    "ge": "{0} >= {1}",
    "eq": "{0} == {1}",
    "ne": "{0} != {1}",
}

BITWISE = {
    "and": "&",
    "or": "|",
    "xor": "^",
}

DOT_PRODUCTS = {"dp2": 2, "dp3": 3, "dp4": 4}

# The disassembler names the resource dimension in the first parenthesised
# group of a sample instruction, which is also the coordinate width.
RESOURCE_DIMENSIONS = {
    "texture1d": ("Texture1D", 1),
    "texture2d": ("Texture2D", 2),
    "texture3d": ("Texture3D", 3),
    "texturecube": ("TextureCube", 3),
    "texture2darray": ("Texture2DArray", 3),
    "texturecubearray": ("TextureCubeArray", 4),
}

SAMPLE_FORMS = {
    "sample": ("Sample", 0),
    "sample_l": ("SampleLevel", 1),
    "sample_b": ("SampleBias", 1),
    "sample_d": ("SampleGrad", 2),
    "sample_c": ("SampleCmp", 1),
    "sample_c_lz": ("SampleCmpLevelZero", 1),
}

PREAMBLE = """\
// Generated by Tools/EffectPipeline/translate_ue3_dxbc_to_hlsl.py.
// One statement per cooked DXBC instruction.  Do not hand-edit: regenerate.
"""


class TranslationError(RuntimeError):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise TranslationError(message)


def format_float(value: float) -> str:
    if value != value or value in (float("inf"), float("-inf")):
        raise TranslationError(f"non-finite immediate: {value}")
    text = repr(float(value))
    if "e" in text or "E" in text:
        return f"{value:.9g}f"
    if "." not in text:
        text += ".0"
    return text + "f"


def parse_literal(text: str) -> list[str]:
    """Spell an `l(...)` immediate as the float its register bit pattern holds.

    The disassembler prints a float immediate with a decimal point, an integer
    immediate without one, and a raw pattern in hex.  A DXBC register is
    typeless, so an integer immediate is the bit pattern, not the numeric
    value: reading `l(5)` as 5.0f made `bfi` mask against 0x40A00000 instead
    of 5.  Spelling it `asfloat(5u)` keeps both domains right, because the
    integer paths read it straight back with `asuint`.
    """
    values = []
    for token in text.split(","):
        token = token.strip()
        if token.lower().startswith("0x"):
            raw = struct.unpack("<f", struct.pack("<I", int(token, 16)))[0]
            values.append(format_float(raw))
        elif "." in token or "e" in token.lower():
            values.append(format_float(float(token)))
        else:
            number = int(token)
            values.append(
                f"asfloat({number}u)" if number >= 0
                else f"asfloat({number})")
    return values


class Operand:
    """One decoded DXBC source or destination operand."""

    def __init__(self, text: str) -> None:
        self.raw = text.strip()
        body = self.raw
        self.negate = False
        self.absolute = False
        if body.startswith("-"):
            self.negate = True
            body = body[1:]
        if body.startswith("|") and body.endswith("|"):
            self.absolute = True
            body = body[1:-1]
        self.literal: list[str] | None = None
        if body.startswith("l(") and body.endswith(")"):
            self.literal = parse_literal(body[2:-1])
            self.register = None
            self.swizzle = ""
            return
        match = re.fullmatch(r"([A-Za-z_]+)(\d*)(\[\d+\])?(?:\.([xyzw]+))?", body)
        require(match is not None, f"unsupported operand: {self.raw}")
        prefix, number, index, swizzle = match.groups()
        self.register = f"{prefix}{number}{index or ''}"
        self.swizzle = swizzle or ""

    @property
    def is_null(self) -> bool:
        return self.register == "null"

    def component(self, lane: int) -> str:
        """The scalar this operand supplies for destination component `lane`."""
        if self.literal is not None:
            values = self.literal
            value = values[lane] if len(values) > 1 else values[0]
            return self._decorate(value, scalar_literal=True)
        if not self.swizzle:
            return self._decorate(self.register)
        if len(self.swizzle) == 1:
            return self._decorate(f"{self.register}.{self.swizzle}")
        require(
            lane < len(self.swizzle),
            f"operand {self.raw} has no component {COMPONENTS[lane]}",
        )
        return self._decorate(f"{self.register}.{self.swizzle[lane]}")

    def vector(self, width: int) -> str:
        """The first `width` components, for reductions such as dp3."""
        if self.literal is not None:
            values = self.literal
            if len(values) == 1:
                values = values * width
            joined = ", ".join(values[:width])
            return self._decorate(
                f"float{width}({joined})", scalar_literal=True)
        if not self.swizzle:
            return self._decorate(f"{self.register}.{COMPONENTS[:width]}")
        if len(self.swizzle) == 1:
            return self._decorate(
                f"{self.register}.{self.swizzle * width}")
        require(
            len(self.swizzle) >= width,
            f"operand {self.raw} has fewer than {width} components",
        )
        return self._decorate(f"{self.register}.{self.swizzle[:width]}")

    def _decorate(self, text: str, scalar_literal: bool = False) -> str:
        if self.absolute:
            text = f"abs({text})"
        if self.negate:
            text = f"-({text})" if scalar_literal or self.absolute else f"-{text}"
        return text


def destination_lanes(operand: Operand) -> list[int]:
    require(bool(operand.swizzle), f"destination has no write mask: {operand.raw}")
    return [COMPONENTS.index(component) for component in operand.swizzle]


def assign(destination: Operand, lane_expressions: list[tuple[int, str]],
           saturate: bool, integer: bool) -> list[str]:
    """Write a destination the way the hardware does: read all, then write.

    A DXBC vector instruction samples every source before it writes any
    destination component, so `mul r0.xyz, r0.xxxx, r1.xyzx` multiplies by the
    *old* r0.x three times.  Assigning component by component would feed the
    new r0.x into the next lane.  Staging through per-lane temporaries keeps
    the simultaneous-write semantics for every multi-component destination.
    """
    prepared = []
    for lane, expression in lane_expressions:
        if saturate:
            expression = f"saturate({expression})"
        if integer:
            expression = f"asfloat({expression})"
        prepared.append((lane, expression))

    if len(prepared) == 1:
        lane, expression = prepared[0]
        return [f"{destination.register}.{COMPONENTS[lane]} = {expression};"]

    stage = next_stage_name(destination)
    lines = [
        f"float {stage}_{COMPONENTS[lane]} = {expression};"
        for lane, expression in prepared
    ]
    lines += [
        f"{destination.register}.{COMPONENTS[lane]} = "
        f"{stage}_{COMPONENTS[lane]};"
        for lane, _ in prepared
    ]
    return lines


def translate_instruction(text: str) -> list[str]:
    """One DXBC instruction -> the HLSL statements that reproduce it."""
    stripped = text.strip()
    if not stripped or stripped.startswith("//"):
        return []
    if stripped == "ret":
        # Only a trailing `ret` is a no-op; the emitters reject an early one,
        # because dropping it would run code the cooked program skips.
        return []
    if stripped in CONTROL_FLOW:
        return [CONTROL_FLOW[stripped]]

    match = re.match(r"([a-z0-9_]+)((?:\([^)]*\))*)\s*(.*)", stripped)
    require(match is not None, f"unparsed instruction: {stripped}")
    mnemonic = match.group(1)
    groups = re.findall(r"\(([^)]*)\)", match.group(2) or "")
    operand_text = match.group(3)

    saturate = mnemonic.endswith("_sat")
    if saturate:
        mnemonic = mnemonic[: -len("_sat")]

    operands = [Operand(part) for part in split_operands(operand_text)] \
        if operand_text else []

    base = mnemonic.replace("_indexable", "")
    if base.startswith("sample"):
        dimension = groups[0].strip().lower() if groups else "texture2d"
        require(dimension in RESOURCE_DIMENSIONS,
                f"unsupported sample resource dimension: {dimension}")
        return translate_sample(
            base, operands, saturate, RESOURCE_DIMENSIONS[dimension][1])

    if mnemonic in CONTROL_FLOW_WITH_CONDITION:
        require(len(operands) == 1,
                f"{mnemonic} operand count changed: {stripped}")
        return [CONTROL_FLOW_WITH_CONDITION[mnemonic].format(
            operands[0].component(0))]

    require(bool(operands), f"instruction has no operands: {stripped}")
    destination = operands[0]
    sources = operands[1:]

    if destination.register == COVERAGE_OUTPUT:
        require(mnemonic == "mov",
                f"unsupported coverage-mask write: {stripped}")
        return [f"{COVERAGE_OUTPUT} = asuint({sources[0].component(0)});"]

    if mnemonic in DOT_PRODUCTS:
        width = DOT_PRODUCTS[mnemonic]
        expression = (f"dot({sources[0].vector(width)}, "
                      f"{sources[1].vector(width)})")
        return assign(
            destination,
            [(lane, expression) for lane in destination_lanes(destination)],
            saturate, integer=False)

    if mnemonic in CONVERSIONS:
        template, integer = CONVERSIONS[mnemonic]
        rows = [(lane, template.format(sources[0].component(lane)))
                for lane in destination_lanes(destination)]
        return assign(destination, rows, saturate, integer=integer)

    if mnemonic in INTEGER_ARITHMETIC:
        template = INTEGER_ARITHMETIC[mnemonic]
        rows = [(lane, f"asuint({template.format(*[source.component(lane) for source in sources])})")
                for lane in destination_lanes(destination)]
        return assign(destination, rows, saturate=False, integer=True)

    if mnemonic in INTEGER_COMPARISONS or mnemonic in COMPARISONS:
        template = INTEGER_COMPARISONS.get(mnemonic) or COMPARISONS[mnemonic]
        rows = []
        for lane in destination_lanes(destination):
            condition = template.format(
                *[source.component(lane) for source in sources])
            rows.append((lane, f"({condition}) ? 0xffffffffu : 0u"))
        return assign(destination, rows, saturate=False, integer=True)

    if mnemonic in BITWISE:
        operator = BITWISE[mnemonic]
        rows = []
        for lane in destination_lanes(destination):
            left, right = (source.component(lane) for source in sources)
            rows.append((lane, f"asuint({left}) {operator} asuint({right})"))
        return assign(destination, rows, saturate=False, integer=True)

    if mnemonic == "bfi":
        require(len(sources) == 4, f"bfi operand count changed: {stripped}")
        rows = []
        for lane in destination_lanes(destination):
            width, offset, insert, base = (
                source.component(lane) for source in sources)
            mask = (f"((((1u << (asuint({width}) & 31u)) - 1u) "
                    f"<< (asuint({offset}) & 31u)))")
            rows.append((
                lane,
                f"(((asuint({insert}) << (asuint({offset}) & 31u)) & {mask}) "
                f"| (asuint({base}) & ~{mask}))"))
        return assign(destination, rows, saturate=False, integer=True)

    if mnemonic == "not":
        rows = [(lane, f"~asuint({sources[0].component(lane)})")
                for lane in destination_lanes(destination)]
        return assign(destination, rows, saturate=False, integer=True)

    if mnemonic == "movc":
        rows = []
        for lane in destination_lanes(destination):
            condition, true_value, false_value = (
                source.component(lane) for source in sources)
            rows.append((
                lane,
                f"asuint({condition}) != 0u ? {true_value} : {false_value}"))
        return assign(destination, rows, saturate, integer=False)

    if mnemonic == "sincos":
        require(len(operands) == 3, "sincos operand count changed")
        sine, cosine, source = operands
        # Both destinations read the same source, so the angle is staged once
        # before either write can disturb it.
        lanes = sorted({
            lane
            for operand in (sine, cosine)
            if not operand.is_null
            for lane in destination_lanes(operand)
        })
        stage = next_stage_name(source)
        lines = [
            f"float {stage}_{COMPONENTS[lane]} = {source.component(lane)};"
            for lane in lanes
        ]
        for operand, function in ((sine, "sin"), (cosine, "cos")):
            if operand.is_null:
                continue
            lines += assign(
                operand,
                [(lane, f"{function}({stage}_{COMPONENTS[lane]})")
                 for lane in destination_lanes(operand)],
                saturate, integer=False)
        return lines

    template = COMPONENTWISE.get(mnemonic)
    require(template is not None, f"unsupported DXBC opcode: {mnemonic}")
    rows = []
    for lane in destination_lanes(destination):
        rows.append((lane, template.format(
            *[source.component(lane) for source in sources])))
    return assign(destination, rows, saturate, integer=False)


def translate_sample(base: str, operands: list[Operand],
                     saturate: bool, coordinate_width: int) -> list[str]:
    form = SAMPLE_FORMS.get(base)
    require(form is not None, f"unsupported sample form: {base}")
    method, extra = form
    require(len(operands) >= 4 + extra,
            f"sample operand count changed: {base}")
    destination, coordinate, resource, sampler = operands[:4]
    trailing = [operand.component(0) for operand in operands[4:4 + extra]]

    lanes = destination_lanes(destination)
    arguments = [sampler.register, coordinate.vector(coordinate_width)]
    arguments += trailing
    call = f"{resource.register}.{method}({', '.join(arguments)})"

    fetched = next_sample_name(destination)
    lines = [f"float4 {fetched} = {call};"]
    swizzle = resource.swizzle or COMPONENTS
    for lane in lanes:
        require(lane < len(swizzle),
                f"sample resource swizzle has no component {COMPONENTS[lane]}")
        expression = f"{fetched}.{swizzle[lane]}"
        if saturate:
            expression = f"saturate({expression})"
        lines.append(
            f"{destination.register}.{COMPONENTS[lane]} = {expression};")
    return lines


_SAMPLE_SEQUENCE = {"value": 0}


def next_sample_name(destination: Operand) -> str:
    """A fresh name per fetch: the destination register is also a source."""
    _SAMPLE_SEQUENCE["value"] += 1
    return f"fetch{_SAMPLE_SEQUENCE['value']}"


def next_stage_name(destination: Operand) -> str:
    _SAMPLE_SEQUENCE["value"] += 1
    return f"stage{_SAMPLE_SEQUENCE['value']}"


def split_operands(text: str) -> list[str]:
    """Split on commas that are not inside `l(...)` or a resource form."""
    parts = []
    depth = 0
    current = []
    for character in text:
        if character == "(":
            depth += 1
        elif character == ")":
            depth -= 1
        if character == "," and depth == 0:
            parts.append("".join(current))
            current = []
            continue
        current.append(character)
    if current:
        parts.append("".join(current))
    return [part for part in (item.strip() for item in parts) if part]


def parse_declarations(declarations: list[str]) -> dict[str, Any]:
    constant_buffers: dict[str, int] = {}
    samplers: list[str] = []
    textures: list[str] = []
    texture_types: dict[str, str] = {}
    inputs: list[dict[str, Any]] = []
    outputs: list[str] = []
    temporaries = 0
    coverage = False
    coverage_input = False
    for line in declarations:
        stripped = line.strip()
        match = re.match(r"dcl_constantbuffer\s+(CB\d+)\[(\d+)\]", stripped,
                         re.I)
        if match:
            constant_buffers[match.group(1).lower()] = int(match.group(2))
            continue
        match = re.match(r"dcl_sampler\s+(s\d+)", stripped)
        if match:
            samplers.append(match.group(1))
            continue
        match = re.match(
            r"dcl_resource_(\w+)\s*\([^)]*\)\s*(t\d+)", stripped)
        if match:
            dimension = match.group(1).lower()
            require(dimension in RESOURCE_DIMENSIONS,
                    f"unsupported resource dimension: {dimension}")
            textures.append(match.group(2))
            texture_types[match.group(2)] = RESOURCE_DIMENSIONS[dimension][0]
            continue
        if re.match(r"dcl_input\s+vCoverage", stripped):
            coverage_input = True
            continue
        match = re.match(
            r"dcl_input_ps(?:_sgv|_siv)?\s+(?:(\w+)\s+)?(?:(centroid)\s+)?"
            r"(v\d+)\.([xyzw]+)",
            stripped)
        if match:
            interpolation, centroid, register, mask = match.groups()
            inputs.append({
                "register": register,
                "mask": mask,
                "interpolation": interpolation or "linear",
                "centroid": bool(centroid),
            })
            continue
        match = re.match(r"dcl_output\s+(o\d+)", stripped)
        if match:
            outputs.append(match.group(1))
            continue
        if re.match(r"dcl_output\s+oMask\b", stripped):
            coverage = True
            continue
        match = re.match(r"dcl_temps\s+(\d+)", stripped)
        if match:
            temporaries = int(match.group(1))
            continue
    return {
        "constantBuffers": constant_buffers,
        "samplers": samplers,
        "textures": textures,
        "textureTypes": texture_types,
        "inputs": inputs,
        "outputs": sorted(set(outputs)),
        "coverageMask": coverage,
        "coverageInput": coverage_input,
        "temporaryCount": temporaries,
    }


def translate_body(instructions: list[str], indent: str) -> list[str]:
    """Translate a whole program, rejecting a `ret` that is not the last one.

    A `ret` in the middle of the program ends it early.  Dropping it silently
    would run the remaining instructions, so it is refused rather than guessed
    at; none of the recovered Effect programs use one.
    """
    lines: list[str] = []
    for index, instruction in enumerate(instructions):
        if instruction.strip() == "ret":
            require(index == len(instructions) - 1,
                    "early `ret` is unsupported: the program returns before "
                    f"instruction {index + 1} of {len(instructions)}")
            continue
        statements = translate_instruction(instruction)
        if not statements:
            continue
        lines.append(f"{indent}// {instruction.strip()}")
        for statement in statements:
            lines.append(f"{indent}{statement}")
    return lines


def emit_module(disassembly: dict[str, Any], function_name: str) -> str:
    declarations = parse_declarations(disassembly["declarations"])
    lines: list[str] = [PREAMBLE]

    for name, size in sorted(declarations["constantBuffers"].items()):
        register = int(name[2:])
        lines.append(
            f"cbuffer {function_name}_Constants{register} : register(b{register})")
        lines.append("{")
        lines.append(f"    float4 {name}[{size}];")
        lines.append("};")
    for sampler in declarations["samplers"]:
        lines.append(
            f"SamplerState {sampler} : register(s{sampler[1:]});")
    for texture in declarations["textures"]:
        lines.append(
            f"{declarations['textureTypes'][texture]} {texture} "
            f": register(t{texture[1:]});")
    lines.append("")

    lines.append("struct " + function_name + "_INPUT")
    lines.append("{")
    for row in declarations["inputs"]:
        lines.append(f"    float4 {row['register']};")
    if declarations["coverageInput"]:
        lines.append(f"    uint4 {COVERAGE_INPUT};")
    lines.append("};")
    lines.append("")
    lines.append("struct " + function_name + "_OUTPUT")
    lines.append("{")
    for output in declarations["outputs"]:
        lines.append(f"    float4 {output};")
    if declarations["coverageMask"]:
        lines.append(f"    uint {COVERAGE_OUTPUT};")
    lines.append("};")
    lines.append("")

    lines.append(f"{function_name}_OUTPUT {function_name}("
                 f"{function_name}_INPUT stage)")
    lines.append("{")
    for row in declarations["inputs"]:
        lines.append(f"    float4 {row['register']} = stage.{row['register']};")
    if declarations["coverageInput"]:
        lines.append(
            f"    uint4 {COVERAGE_INPUT} = stage.{COVERAGE_INPUT};")
    for index in range(declarations["temporaryCount"]):
        lines.append(f"    float4 r{index} = (float4)0;")
    for output in declarations["outputs"]:
        lines.append(f"    float4 {output} = (float4)0;")
    if declarations["coverageMask"]:
        lines.append(f"    uint {COVERAGE_OUTPUT} = 0xffffffffu;")
    lines.append("")

    lines.extend(translate_body(disassembly["instructions"], "    "))
    lines.append("")
    lines.append(f"    {function_name}_OUTPUT result;")
    for output in declarations["outputs"]:
        lines.append(f"    result.{output} = {output};")
    if declarations["coverageMask"]:
        lines.append(f"    result.{COVERAGE_OUTPUT} = {COVERAGE_OUTPUT};")
    lines.append("    return result;")
    lines.append("}")
    return "\n".join(lines) + "\n"


def emit_compile_probe(declarations: dict[str, Any],
                       function_name: str) -> str:
    """Build a pixel entry point that type-checks the stored HLSLI module.

    Numeric replay intentionally uses the cooked shader's exact linkage.  It
    therefore cannot prove that the separately stored, callable HLSLI module
    has all of its own inputs declared or preserves its explicit resource
    registers.  This small wrapper calls the stored function so D3DCompile
    validates that artifact as written before it is published.
    """

    require(bool(declarations["outputs"]),
            "translated pixel shader declares no color output")
    arguments = ["float4 probe : TEXCOORD0"]
    if declarations["coverageInput"]:
        arguments.append("uint coverage : SV_Coverage")
    lines = [
        "",
        f"float4 main({', '.join(arguments)}) : SV_Target0",
        "{",
        f"    {function_name}_INPUT stage = ({function_name}_INPUT)0;",
    ]
    for row in declarations["inputs"]:
        lines.append(f"    stage.{row['register']} = probe;")
    if declarations["coverageInput"]:
        lines.append(f"    stage.{COVERAGE_INPUT} = coverage;")
    lines += [
        f"    {function_name}_OUTPUT result = {function_name}(stage);",
        f"    return result.{declarations['outputs'][0]};",
        "}",
    ]
    return "\n".join(lines) + "\n"


def validate_module_compiles(source: str, declarations: dict[str, Any],
                             function_name: str,
                             d3dcompiler: Path) -> None:
    """Compile the exact stored module plus a probe entry point, fail closed."""

    # Reuse the repository's pinned D3DCompile ABI wrapper while keeping the
    # emitted artifact independent of the verification harness.
    from replay_ue3_material_pixel_shaders import (  # noqa: WPS433
        CarrierCompiler,
        blob_bytes,
        release,
    )

    compiler = CarrierCompiler(d3dcompiler)
    probe = (source + emit_compile_probe(
        declarations, function_name)).encode("utf-8")
    backing = ctypes.create_string_buffer(probe)
    code = ctypes.c_void_p()
    errors = ctypes.c_void_p()
    try:
        result = compiler.function(
            backing,
            len(probe),
            function_name.encode("ascii"),
            None,
            None,
            b"main",
            b"ps_5_0",
            0,
            0,
            ctypes.byref(code),
            ctypes.byref(errors),
        )
        error_text = (
            blob_bytes(errors).decode("utf-8", "replace")
            if errors.value else "")
        require(
            result >= 0 and bool(code.value),
            "stored HLSLI module failed to compile: "
            f"0x{result & 0xFFFFFFFF:08X} {error_text}",
        )
    finally:
        release(errors)
        release(code)


def translate_blob(bytecode: bytes, function_name: str,
                   d3dcompiler: Path) -> dict[str, Any]:
    disassembler = D3DDisassembler(d3dcompiler)
    disassembly = disassembler.disassemble(bytecode)
    require(disassembly["profile"].startswith("ps_"),
            f"only pixel shaders are translated: {disassembly['profile']}")
    _SAMPLE_SEQUENCE["value"] = 0
    source = emit_module(disassembly, function_name)
    declarations = parse_declarations(disassembly["declarations"])
    validate_module_compiles(
        source, declarations, function_name, d3dcompiler)
    return {
        "functionName": function_name,
        "profile": disassembly["profile"],
        "instructionCount": disassembly["instructionCount"],
        "declarations": declarations,
        "dxbcSha256": hashlib.sha256(bytecode).hexdigest(),
        "hlslSha256": hashlib.sha256(source.encode("utf-8")).hexdigest(),
        "hlsl": source,
    }


COOKED_RECEIPT = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/effect-family-cooked-pixel-shaders.v1.json")


def family_function_names(receipt_path: Path) -> dict[str, str]:
    """DXBC digest -> the function name the family's own leaf gives it.

    Naming the output after the parent material rather than the blob hash is
    what makes a translated program findable from the family inventory; two
    families that cooked to the same program share one function, which is the
    same sharing the renderer would want.
    """
    if not receipt_path.is_file():
        return {}
    receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
    names: dict[str, list[str]] = {}
    for row in receipt.get("families", []):
        digest = row.get("dxbcSha256")
        if not digest:
            continue
        leaf = row["parentMaterialPath"].rsplit(".", 1)[-1]
        names.setdefault(digest, []).append(leaf)
    candidates = {
        digest: "Shade_Ue3_" + sorted(leaves)[0]
        for digest, leaves in names.items()
    }
    candidate_digests: dict[str, list[str]] = {}
    for digest, candidate in candidates.items():
        candidate_digests.setdefault(candidate, []).append(digest)
    # The same material leaf can exist in different packages and cook to
    # different programs.  Giving those digests one filename used to overwrite
    # the first HLSLI while leaving both report rows behind.  Suffix every member
    # of a collision cohort so the digest -> source mapping stays one-to-one.
    return {
        digest: (
            candidate
            if len(candidate_digests[candidate]) == 1
            else candidate + "_" + digest
        )
        for digest, candidate in candidates.items()
    }


def write_utf8_lf(path: Path, content: str) -> None:
    """Write a hash-bearing text artifact without platform newline conversion."""

    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="\n") as handle:
        handle.write(content)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("dxbc", type=Path, nargs="+")
    parser.add_argument("--function-name", default=None)
    parser.add_argument(
        "--name-from-receipt",
        type=Path,
        nargs="?",
        const=COOKED_RECEIPT,
        default=None,
        help="Name each output after the family that cooked it.",
    )
    parser.add_argument("--output-directory", type=Path, default=None)
    parser.add_argument("--d3dcompiler", type=Path,
                        default=DEFAULT_D3DCOMPILER)
    parser.add_argument("--report", type=Path, default=None)
    arguments = parser.parse_args(argv)

    receipt_names = (
        family_function_names(arguments.name_from_receipt)
        if arguments.name_from_receipt else {})
    planned_names = [
        (
            arguments.function_name
            or receipt_names.get(path.stem)
            or ("Shade_Ue3_" + path.stem[:12])
        )
        for path in arguments.dxbc
    ]
    if arguments.output_directory is not None:
        duplicate_names = sorted(
            name for name in set(planned_names) if planned_names.count(name) > 1
        )
        if duplicate_names:
            print(
                "FAIL output function names are not unique: "
                + ", ".join(duplicate_names),
                file=sys.stderr,
            )
            return 1
    rows = []
    failures = 0
    for path, name in zip(arguments.dxbc, planned_names):
        try:
            result = translate_blob(
                path.read_bytes(), name, arguments.d3dcompiler)
        except TranslationError as error:
            failures += 1
            print(f"FAIL {path.name}: {error}", file=sys.stderr)
            rows.append({"dxbc": path.name, "status": "FAILED",
                         "error": str(error)})
            continue
        if arguments.output_directory is not None:
            arguments.output_directory.mkdir(parents=True, exist_ok=True)
            target = arguments.output_directory / f"{name}.hlsli"
            write_utf8_lf(target, result["hlsl"])
            print(f"WROTE: {target}")
        else:
            print(result["hlsl"])
        rows.append({
            "dxbc": path.name,
            "status": "TRANSLATED",
            "functionName": result["functionName"],
            "instructionCount": result["instructionCount"],
            "dxbcSha256": result["dxbcSha256"],
            "hlslSha256": result["hlslSha256"],
            "declarations": result["declarations"],
        })

    if arguments.report is not None:
        write_utf8_lf(
            arguments.report,
            json.dumps(rows, indent=2, ensure_ascii=False) + "\n",
        )
    print(f"RESULT: translated={len(rows) - failures} failed={failures}")
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())

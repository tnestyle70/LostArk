#!/usr/bin/env python3
"""Focused Product-join and RT0 first-pixel proof for DimensionMaster F Fluid01."""

from __future__ import annotations

import json
import math
from pathlib import Path
import struct
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
EFFECT_ID = "effect.dimensionmaster.skill.2050230.unified"
ELEMENT_IDS = (
    "authored.source-particle.1ae3416ac205fee634b746a9",
    "authored.source-particle.ed33fb10661afb8854e76957",
)
SOURCE_MATERIAL = "fx_m_mi_w_00.mi.fx_w_pa_fd_01_3_tr"
AUTHORED = ROOT / "Data/Effects/Authored/effect.dimensionmaster.skill.2050230.unified.effect.json"
IMPORTED = ROOT / "Data/Effects/Imported/DimensionMaster/Converted/effect.dimensionmaster.skill.2050230.imported.effect.json"
CATALOG = ROOT / "Data/Effects/EffectCatalog.json"
RUNTIME_CATALOG = ROOT / "Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json"
SHADER = ROOT / "Client/Bin/ShaderFiles/Shader_EffectUe3MaterialFamilies.hlsli"
PARTICLE_SHADER = ROOT / "Client/Bin/ShaderFiles/Shader_VtxEffectParticle.hlsl"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def rgb565(value: int) -> tuple[float, float, float, float]:
    return (
        ((value >> 11) & 31) / 31.0,
        ((value >> 5) & 63) / 63.0,
        (value & 31) / 31.0,
        1.0,
    )


def decode_dxt1(block: bytes) -> list[tuple[float, float, float, float]]:
    c0, c1, indices = struct.unpack("<HHI", block)
    p0, p1 = rgb565(c0), rgb565(c1)
    if c0 > c1:
        palette = [p0, p1,
            tuple((2 * p0[i] + p1[i]) / 3 for i in range(4)),
            tuple((p0[i] + 2 * p1[i]) / 3 for i in range(4))]
    else:
        palette = [p0, p1,
            tuple((p0[i] + p1[i]) / 2 for i in range(4)),
            (0.0, 0.0, 0.0, 0.0)]
    return [palette[(indices >> (2 * i)) & 3] for i in range(16)]


def decode_bc4(block: bytes) -> list[float]:
    a0, a1 = block[0], block[1]
    bits = int.from_bytes(block[2:8], "little")
    if a0 > a1:
        palette = [a0, a1] + [
            ((7 - i) * a0 + i * a1) / 7 for i in range(1, 7)
        ]
    else:
        palette = [a0, a1] + [
            ((5 - i) * a0 + i * a1) / 5 for i in range(1, 5)
        ] + [0.0, 255.0]
    return [palette[(bits >> (3 * i)) & 7] / 255.0 for i in range(16)]


def load_dds(path: Path) -> tuple[int, int, list[tuple[float, float, float, float]]]:
    data = path.read_bytes()
    require(data[:4] == b"DDS " and len(data) >= 128, f"invalid DDS: {path}")
    height, width = struct.unpack_from("<II", data, 12)
    fourcc = data[84:88]
    block_bytes = 8 if fourcc == b"DXT1" else 16
    require(fourcc in (b"DXT1", b"ATI2"), f"unsupported DDS family: {fourcc!r}")
    pixels = [(0.0, 0.0, 0.0, 1.0)] * (width * height)
    offset = 128
    for by in range((height + 3) // 4):
        for bx in range((width + 3) // 4):
            block = data[offset:offset + block_bytes]
            offset += block_bytes
            if fourcc == b"DXT1":
                decoded = decode_dxt1(block)
            else:
                red, green = decode_bc4(block[:8]), decode_bc4(block[8:16])
                decoded = [(red[i], green[i], 0.0, 1.0) for i in range(16)]
            for py in range(4):
                for px in range(4):
                    x, y = bx * 4 + px, by * 4 + py
                    if x < width and y < height:
                        pixels[y * width + x] = decoded[py * 4 + px]
    return width, height, pixels


def sample(texture: tuple[int, int, list[tuple[float, float, float, float]]],
           u: float, v: float) -> tuple[float, float, float, float]:
    width, height, pixels = texture
    x, y = (u % 1.0) * width - 0.5, (v % 1.0) * height - 0.5
    x0, y0 = math.floor(x), math.floor(y)
    tx, ty = x - x0, y - y0
    def texel(ix: int, iy: int) -> tuple[float, float, float, float]:
        return pixels[(iy % height) * width + (ix % width)]
    rows = []
    for iy in (y0, y0 + 1):
        a, b = texel(x0, iy), texel(x0 + 1, iy)
        rows.append(tuple(a[i] * (1 - tx) + b[i] * tx for i in range(4)))
    return tuple(rows[0][i] * (1 - ty) + rows[1][i] * ty for i in range(4))


def smoothstep(a: float, b: float, value: float) -> float:
    t = max(0.0, min(1.0, (value - a) / (b - a)))
    return t * t * (3.0 - 2.0 * t)


def first_pixel_witness(document: dict[str, Any]) -> dict[str, int]:
    rows = [row for row in document["elements"] if row["id"] in ELEMENT_IDS]
    require([row["id"] for row in rows] == list(ELEMENT_IDS), "Product Fluid01 row order changed")
    execution = rows[0]["material"]["execution"]
    textures = [load_dds(ROOT / "Client/Bin/Resources" / lane["assetId"])
                for lane in execution["textureLanes"]]
    scalars = [row["value"] for row in execution["scalars"]]
    p = [scalars[i:i + 4] + [0.0] * (4 - len(scalars[i:i + 4]))
         for i in range(0, 24, 4)]
    dynamic = (1.3, 3.0, 1.0, 0.0)
    particle = (10.0, 1.2, 2.4, 0.5)
    results: dict[str, int] = {}
    for row in rows:
        local_time = 0.8 - float(row["detail"]["timing"]["startDelaySeconds"])
        nonzero = 0
        for iy in range(32):
            for ix in range(32):
                uv = ((ix + 0.5) / 32, (iy + 0.5) / 32)
                n1uv = (uv[0] * abs(p[1][3]) + p[2][1] * local_time,
                        uv[1] * abs(p[1][3]) + p[2][0] * local_time)
                n2uv = (uv[0] * abs(p[2][3]) + p[3][1] * local_time,
                        uv[1] * abs(p[2][3]) + p[3][0] * local_time)
                n1, n2 = sample(textures[2], *n1uv), sample(textures[3], *n2uv)
                noise = tuple(max(-0.25, min(0.25,
                    (n1[i] * 2 - 1) * p[2][2] + (n2[i] * 2 - 1) * p[3][2]))
                    for i in range(2))
                tuv = (uv[0] * abs(p[0][2]) + p[1][0] * local_time + noise[0],
                        uv[1] * abs(p[0][2]) + p[0][3] * local_time + noise[1])
                transition = sample(textures[0], *tuv)
                carrier = sum(transition[i] * (0.299, 0.587, 0.114)[i] for i in range(3))
                threshold = min(1.0, abs(dynamic[0]) / 1.5 * 0.8 + p[0][1] * 0.1)
                softness = max(abs(p[0][0]), 1 / 255)
                fill = smoothstep(threshold - softness, threshold + softness, carrier)
                line = max(0.0, min(1.0, 1 - abs(carrier - threshold) /
                    max(abs(p[1][2]) * softness, 1 / 255)))
                radius = math.hypot(uv[0] - 0.5, uv[1] - 0.5)
                radial = max(0.0, min(1.0, 1 - smoothstep(0.38, 0.5, radius)))
                shape = max(fill, line) ** dynamic[1] * radial
                euv = (uv[0] * abs(p[4][1]) + noise[0],
                        uv[1] * abs(p[4][2]) + noise[1])
                emissive = sample(textures[1], *euv)
                radiance = sum((emissive[i] * p[3][3] + transition[i] * line * p[1][1])
                    * p[5][1] * particle[i] for i in range(3))
                if particle[3] * shape > 1e-4 and radiance > 1e-4:
                    nonzero += 1
        require(nonzero > 0, f"RT0 first-pixel witness is zero: {row['id']}")
        results[row["id"]] = nonzero
    return results


def run() -> dict[str, Any]:
    authored, imported = load_json(AUTHORED), load_json(IMPORTED)
    raw = [row for row in imported["elements"]
           if row.get("material", {}).get("sourceMaterialPath") == SOURCE_MATERIAL]
    require(len(raw) == 2 and all(row["sourceRecipe"]["rendererShape"] == "sprite" for row in raw),
            "raw Fluid01 family census changed")
    catalog = load_json(CATALOG)
    row = next((item for item in catalog["effects"] if item["effectAssetId"] == EFFECT_ID), None)
    require(row is not None and row["payloadKind"] == "DIRECT_AUTHORED_DOCUMENT_V13",
            "authoring catalog join changed")
    runtime_catalog = load_json(RUNTIME_CATALOG)
    runtime_row = next((item for item in runtime_catalog["effects"]
                        if item["effectAssetId"] == EFFECT_ID), None)
    require(runtime_row is not None, "sealed runtime catalog join is missing")
    runtime = load_json(RUNTIME_CATALOG.parent / runtime_row["authoredDocumentPath"])
    for document in (authored, runtime):
        targets = [item for item in document["elements"] if item["id"] in ELEMENT_IDS]
        require(len(targets) == 2 and all(
            item["kind"] == "particle" and item["sourceRecipe"]["rendererShape"] == "sprite" and
            item["material"]["execution"]["opcode"] == 17 for item in targets),
            "typed Product Sprite carrier join changed")
    shader, dispatch = SHADER.read_text(encoding="utf-8"), PARTICLE_SHADER.read_text(encoding="utf-8")
    require("Shade_EffectUe3Fluid01SpriteWFd013Particle" in shader and
            "g_SourceTextureMask == 0xfu" in shader and
            "Shade_EffectUe3Fluid01SpriteWFd013Particle(" in dispatch,
            "opcode 17 HLSL include/dispatch changed")
    return {"status": "PASS", "rawFamilyRows": 2, "productRows": 2,
            "carrierDispositions": ["KEEP", "KEEP"],
            "cohortRoles": ["CANARY", "DATA_ONLY_EXPANSION"],
            "rt0NonzeroGridPixels": first_pixel_witness(authored),
            "visualStatus": "PENDING_USER_REVIEW"}


if __name__ == "__main__":
    try:
        print(json.dumps(run(), ensure_ascii=False, indent=2))
    except (OSError, ValueError, KeyError, RuntimeError, struct.error) as error:
        print(f"FAIL: {error}")
        raise SystemExit(1)

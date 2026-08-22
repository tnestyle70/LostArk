#!/usr/bin/env python3
"""Materialize the five Valtan canary pixel programs from the pinned cache.

The material-map receipt selects exact source Material/MIC, static set, VF,
pass, shader ID and DXBC identity.  This tool repeats the cache extraction and
writes only content-addressed DXBC files.  It deliberately does not admit a
vertex shader, carrier input ABI, sampler state, runtime renderer or Product
occurrence.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Any

import extract_ue3_material_shader_maps as material_maps
from extract_artist_31470_main_ref_shader_cache import package_tables, require
from extract_artist_31470_shader_cache_oracle import canonical_json_sha256


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_RECEIPT = REPOSITORY_ROOT / (
    "Data/Effects/Imported/Valtan/FrontBackFrontFamilyRestoration/"
    "Valtan.front-back-front-source-exact-family-receipt.v1.json"
)
DEFAULT_CACHE = Path(
    "C:/Users/user/Desktop/Resource_LostArk/01_Extracted/Effect/ARTIST/"
    "31470_TrackA_20260812/OfficialRefShaderCacheV974/"
    "EV2LG3OVEH3HGV7THTFFTM7TOKMCC.v974.upk"
)
DEFAULT_OUTPUT_DIRECTORY = REPOSITORY_ROOT / (
    "Data/Effects/Imported/Valtan/FrontBackFrontFamilyRestoration/CookedShaders"
)
DEFAULT_OUTPUT = REPOSITORY_ROOT / (
    "Data/Effects/Imported/Valtan/FrontBackFrontFamilyRestoration/"
    "Valtan.front-back-front-exact-pixel-programs.v1.json"
)

SCHEMA = "lostark.valtan-front-back-front-exact-pixel-programs"
FORMAT_VERSION = 1


def digest_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def digest_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        while payload := stream.read(1024 * 1024):
            digest.update(payload)
    return digest.hexdigest()


def read_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8-sig"))
    require(isinstance(value, dict), f"expected JSON object: {path}")
    return value


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(
        json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    temporary.replace(path)


def write_bytes_atomic(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_bytes(payload)
    temporary.replace(path)


def validate_source_receipt(receipt: dict[str, Any]) -> list[dict[str, Any]]:
    require(
        receipt.get("schema") == material_maps.RECEIPT_SCHEMA
        and receipt.get("formatVersion") == material_maps.RECEIPT_FORMAT_VERSION,
        "source material-map receipt identity changed",
    )
    summary = receipt.get("summary", {})
    targets = receipt.get("targets")
    require(
        isinstance(targets, list)
        and len(targets) == 5
        and summary.get("targetCount") == 5
        and summary.get("exactMaterialShaderMapCount") == 5
        and summary.get("exactPixelShaderDxbcCount") == 5
        and summary.get("exactNativeShaderObjectBindingCount") == 5
        and summary.get("runtimeAdmissionCount") == 0,
        "five-family source receipt denominator or admission changed",
    )
    for target in targets:
        require(
            target.get("status") == material_maps.STATUS_EXACT
            and target["nativeShaderObjectBinding"]["status"]
            == "EXACT_NATIVE_SHADER_OBJECT_BINDING"
            and target["structuralVfPassCandidate"]["actualVfPassAdmission"]
            is False,
            f"source target is not exact evidence-only: {target.get('targetId')}",
        )
    return targets


def build_program_rows(
    targets: list[dict[str, Any]],
    extracted: dict[str, dict[str, Any]],
    output_directory: Path,
) -> tuple[list[dict[str, Any]], dict[Path, bytes]]:
    rows = []
    payloads: dict[Path, bytes] = {}
    seen_targets: set[str] = set()
    for target in targets:
        target_id = target["targetId"]
        require(target_id not in seen_targets, "source target ID is duplicated")
        seen_targets.add(target_id)
        selection = target["structuralVfPassCandidate"]
        reference = selection["selectedPixelPassReference"]
        shader_id = reference["shaderIdHex"]
        result = extracted[shader_id]
        bytecode = result["_bytecode"]
        expected = target["cookedPixelShader"]["dxbc"]
        require(
            result["shaderType"] == reference["shaderType"]
            and result["dxbc"]["byteSize"] == expected["byteSize"]
            and result["dxbc"]["sha256"] == expected["sha256"]
            and digest_bytes(bytecode) == expected["sha256"],
            f"re-extracted DXBC differs from source receipt: {target_id}",
        )
        output_path = output_directory / f"{expected['sha256']}.dxbc"
        payloads[output_path] = bytecode
        rows.append(
            {
                "targetId": target_id,
                "familyId": target["familyId"],
                "sourceMaterialPath": target["sourceMaterialPath"],
                "rendererType": target["rendererType"],
                "selectedVertexFactoryTypes": reference["vertexFactoryTypes"],
                "shaderType": result["shaderType"],
                "shaderIdHex": shader_id,
                "dxbc": expected,
                "repoRelativePath": output_path.relative_to(
                    REPOSITORY_ROOT
                ).as_posix(),
                "pixelEquationEvidence": "EXACT_PACKED_DXBC",
                "actualVfPassAdmission": False,
                "runtimeAdmission": False,
                "productAdmission": False,
            }
        )
    rows.sort(key=lambda row: row["targetId"])
    return rows, payloads


def build_manifest(
    receipt_path: Path,
    cache_path: Path,
    output_directory: Path,
) -> tuple[dict[str, Any], dict[Path, bytes]]:
    receipt = read_json(receipt_path)
    targets = validate_source_receipt(receipt)
    cache = package_tables(cache_path)
    layout = material_maps.parse_shader_code_layout(cache)
    expected_cache = receipt["officialRefShaderCache"]["package"]
    for field in (
        "fileName",
        "physicalByteSize",
        "rawSha256",
        "logicalByteSize",
        "packageVersion",
        "licenseeVersion",
        "packageGuidHex",
    ):
        require(
            cache["identity"][field] == expected_cache[field],
            f"pinned cache {field} changed",
        )
    references = [
        target["structuralVfPassCandidate"]["selectedPixelPassReference"]
        for target in targets
    ]
    extracted = material_maps.extract_selected_packed_dxbc(
        cache, layout, references
    )
    rows, payloads = build_program_rows(targets, extracted, output_directory)
    manifest = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "identity": {
            "encounter": "VALTAN",
            "pattern": "FRONT_BACK_FRONT",
            "scope": "FIVE_FAMILY_EXACT_PIXEL_PROGRAM_EVIDENCE",
        },
        "inputs": {
            "sourceReceipt": {
                "repoRelativePath": receipt_path.relative_to(
                    REPOSITORY_ROOT
                ).as_posix(),
                "rawSha256": digest_file(receipt_path),
                "receiptSha256": receipt["receiptSha256"],
            },
            "officialRefShaderCache": cache["identity"],
        },
        "programs": rows,
        "admission": {
            "pixelEquationEvidence": "EXACT_PACKED_DXBC",
            "actualVertexShaderAndCarrierAbi": False,
            "sourceExactSampler": False,
            "runtimeAdmission": False,
            "productAdmission": False,
        },
        "summary": {
            "programCount": len(rows),
            "uniqueDxbcCount": len(payloads),
            "runtimeAdmissionCount": 0,
            "productAdmissionCount": 0,
        },
    }
    manifest["receiptSha256"] = canonical_json_sha256(manifest)
    return manifest, payloads


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--receipt", type=Path, default=DEFAULT_RECEIPT)
    parser.add_argument("--cache", type=Path, default=DEFAULT_CACHE)
    parser.add_argument(
        "--output-directory", type=Path, default=DEFAULT_OUTPUT_DIRECTORY
    )
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    arguments = parser.parse_args(argv)
    manifest, payloads = build_manifest(
        arguments.receipt.resolve(),
        arguments.cache.resolve(),
        arguments.output_directory.resolve(),
    )
    if arguments.check:
        require(arguments.output.is_file(), f"manifest is missing: {arguments.output}")
        require(
            read_json(arguments.output) == manifest,
            "exact pixel-program manifest differs from checked-in output",
        )
        for path, payload in payloads.items():
            require(path.is_file(), f"materialized DXBC is missing: {path}")
            require(path.read_bytes() == payload, f"materialized DXBC differs: {path}")
        print(f"PASS: {arguments.output}")
    else:
        for path, payload in payloads.items():
            write_bytes_atomic(path, payload)
        write_json_atomic(arguments.output, manifest)
        print(f"WROTE: {arguments.output}")
    print(
        "RESULT: "
        f"programs={manifest['summary']['programCount']} "
        f"unique={manifest['summary']['uniqueDxbcCount']} "
        "runtime=0 product=0"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

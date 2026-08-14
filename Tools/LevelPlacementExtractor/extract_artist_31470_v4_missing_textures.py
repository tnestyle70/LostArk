#!/usr/bin/env python3
"""Seal and deploy exact source-package textures needed by Artist 31470 V4.

This tool deliberately owns only payload acquisition.  A deployed DDS does not
admit a material program, native VF/pass, sampler, or Product rendering.  Those
remain explicit renderer-registry decisions.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import struct
import subprocess
import tempfile
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = Path(__file__).resolve()
DEFAULT_RESOURCE_ROOT = Path(r"C:\Users\user\Desktop\Resource_LostArk")
DEFAULT_SOURCE_ROOT = Path(
    r"C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages"
)
DEFAULT_OUTPUT = (
    REPO_ROOT
    / "Data/Effects/Imported/Artist/Materials/skill.31470.v4-missing-texture-acquisition.receipt.json"
)
DEFAULT_RUNTIME_ROOT = REPO_ROOT / "Client/Bin/Resources/Effect/Artist/Textures"
UMODEL_RELATIVE_PATH = Path("06_Tools/UEViewerLostArk_runtime/umodel_lostark_v7.exe")

SCHEMA = "lostark.artist-31470-v4-missing-texture-acquisition-receipt"
FORMAT_VERSION = 1

EXPECTED_UMODEL = {
    "byteCount": 1_766_400,
    "rawSha256": "b9573cdcbb7e9d26dbf60a0e3af47fb5af8543140873da8483c26d58cf40b249",
}

EXPECTED_PACKAGES = {
    "FX_TEX_02": {
        "fileName": "YGI3SORGM3I1FGHA5BMJ8Y5CZ.upk",
        "byteCount": 17_288_410,
        "md5": "bdbab159628a909ea9039573268d4b09",
        "rawSha256": "5f49a9481b02e5a174bc1ab1cb3bc621d6ed73e3b386d07ef0c5df3ead65f0a5",
    },
    "FX_TEX_03": {
        "fileName": "YGI3SORGM3I1MGHA5BMJ8B5CZ.upk",
        "byteCount": 6_344_494,
        "md5": "b4d8cc629f0f7d92aea5096e39594394",
        "rawSha256": "f47383ab63e08875d6a98929eba42843cad1be059c299c59aa03c5eb3ae5b314",
    },
}

EXPECTED_ASSETS = (
    {
        "logicalTexturePath": "fx_tex_02.fx_d_fluid_032_1_cl",
        "logicalPackage": "FX_TEX_02",
        "objectName": "fx_d_fluid_032_1_cl",
        "sourceOccurrenceIds": ["source-active-013", "source-active-014"],
        "sourceShaderRegister": "t3",
        "sourceShaderChannel": "G",
        "sourceSemantic": "head_emission_mask",
        "runtimeAssetId": "Effect/Artist/Textures/fx_d_fluid_032_1_cl.dds",
        "dds": {
            "byteCount": 65_664,
            "rawSha256": "6f99596c477d4eb5fb19f48075700dc6c8d64445baebd319ddc49c746c76e18f",
            "width": 512,
            "height": 256,
            "fourCC": "DXT1",
        },
    },
    {
        "logicalTexturePath": "fx_tex_03.fx_e_atypical_024_1_xcl",
        "logicalPackage": "FX_TEX_03",
        "objectName": "fx_e_atypical_024_1_xcl",
        "sourceOccurrenceIds": ["source-active-027"],
        "sourceShaderRegister": "t2",
        "sourceShaderChannel": "G",
        "sourceSemantic": "alpha_shape_mask_fixed_reference",
        "runtimeAssetId": "Effect/Artist/Textures/fx_e_atypical_024_1_xcl.dds",
        "dds": {
            "byteCount": 8_320,
            "rawSha256": "8181d6ba0970e2193cd216c5e60b00d563eb38608ba5b664a51571bdbc93126f",
            "width": 128,
            "height": 128,
            "fourCC": "DXT1",
        },
    },
    {
        "logicalTexturePath": "fx_tex_03.fx_e_electric_002_cl",
        "logicalPackage": "FX_TEX_03",
        "objectName": "fx_e_electric_002_cl",
        "sourceOccurrenceIds": ["source-active-027"],
        "sourceShaderRegister": "t3",
        "sourceShaderChannel": "A",
        "sourceSemantic": "secondary_alpha",
        "runtimeAssetId": "Effect/Artist/Textures/fx_e_electric_002_cl.dds",
        "dds": {
            "byteCount": 65_664,
            "rawSha256": "880b9628bd861d807ea5990773ccbfc2f6f900ee890e370e7ab8574ba9340ca8",
            "width": 256,
            "height": 256,
            "fourCC": "DXT5",
        },
    },
    {
        "logicalTexturePath": "fx_tex_03.fx_e_fluid_003",
        "logicalPackage": "FX_TEX_03",
        "objectName": "fx_e_fluid_003",
        "sourceOccurrenceIds": ["source-active-027"],
        "sourceShaderRegister": "t4",
        "sourceShaderChannel": "G",
        "sourceSemantic": "secondary_alpha_mask",
        "runtimeAssetId": "Effect/Artist/Textures/fx_e_fluid_003.dds",
        "dds": {
            "byteCount": 65_664,
            "rawSha256": "3339f6fd17e588fed6c11082a8a93a9b6dc55c49cb20f2a956dd6258f675a83f",
            "width": 256,
            "height": 256,
            "fourCC": "DXT5",
        },
    },
)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def file_hash(path: Path, algorithm: str = "sha256") -> str:
    digest = hashlib.new(algorithm)
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def canonical_json_bytes(value: Any) -> bytes:
    return (json.dumps(value, ensure_ascii=False, sort_keys=True,
                       separators=(",", ":")) + "\n").encode("utf-8")


def canonical_sha(value: Any) -> str:
    return hashlib.sha256(canonical_json_bytes(value)).hexdigest()


def parse_dds(path: Path) -> dict[str, Any]:
    raw = path.read_bytes()
    require(len(raw) >= 128 and raw[:4] == b"DDS ", f"invalid DDS: {path}")
    require(struct.unpack_from("<I", raw, 4)[0] == 124, f"invalid DDS header: {path}")
    height = struct.unpack_from("<I", raw, 12)[0]
    width = struct.unpack_from("<I", raw, 16)[0]
    mip_count = struct.unpack_from("<I", raw, 28)[0]
    fourcc = raw[84:88].decode("ascii")
    require(mip_count in (0, 1), f"fresh export unexpectedly has mip chain: {path}")
    require(fourcc in {"DXT1", "DXT5", "ATI2"}, f"unsupported DDS format: {fourcc}")
    block_size = 8 if fourcc == "DXT1" else 16
    payload = max(1, (width + 3) // 4) * max(1, (height + 3) // 4) * block_size
    require(len(raw) == 128 + payload, f"DDS payload size mismatch: {path}")
    return {
        "byteCount": len(raw),
        "rawSha256": hashlib.sha256(raw).hexdigest(),
        "width": width,
        "height": height,
        "fourCC": fourcc,
        "rawMipMapCount": mip_count,
        "payloadByteCount": payload,
    }


def verify_tool_and_packages(resource_root: Path, source_root: Path) -> tuple[Path, list[dict[str, Any]]]:
    umodel = resource_root / UMODEL_RELATIVE_PATH
    require(umodel.is_file(), f"pinned UModel is missing: {umodel}")
    require(umodel.stat().st_size == EXPECTED_UMODEL["byteCount"], "UModel byte count changed")
    require(file_hash(umodel) == EXPECTED_UMODEL["rawSha256"], "UModel SHA changed")
    packages = []
    for logical, expected in EXPECTED_PACKAGES.items():
        path = source_root / expected["fileName"]
        require(path.is_file(), f"source package is missing: {path}")
        require(path.stat().st_size == expected["byteCount"], f"package size changed: {logical}")
        require(file_hash(path, "md5") == expected["md5"], f"package MD5 changed: {logical}")
        require(file_hash(path) == expected["rawSha256"], f"package SHA changed: {logical}")
        packages.append({"logicalPackage": logical, **expected})
    return umodel, packages


def extract_asset(umodel: Path, source_root: Path, stage: Path,
                  expected: dict[str, Any]) -> tuple[Path, dict[str, Any]]:
    command = [
        str(umodel), "-export", "-game=lostark", "-kr", "-nameresolve",
        f"-path={source_root}", f"-out={stage}", "-dds", "-nooverwrite",
        f"-obj={expected['objectName']}", expected["logicalPackage"],
    ]
    completed = subprocess.run(command, cwd=umodel.parent, text=True,
                               capture_output=True, timeout=180, check=False)
    require(completed.returncode == 0,
            f"UModel export failed ({completed.returncode}):\n{completed.stdout}\n{completed.stderr}")
    require(f'Export "{expected["objectName"]}" was found' in completed.stdout,
            f"UModel did not resolve exact object: {expected['logicalTexturePath']}")
    candidates = list(stage.rglob(f"{expected['objectName']}.dds"))
    require(len(candidates) == 1, f"expected one exported DDS, found {len(candidates)}")
    invocation = {
        "logicalPackage": expected["logicalPackage"],
        "objectName": expected["objectName"],
        "argvTemplate": [
            "{umodel}", "-export", "-game=lostark", "-kr", "-nameresolve",
            "-path={sourcePackageRoot}", "-out={temporaryStage}", "-dds",
            "-nooverwrite", f"-obj={expected['objectName']}", expected["logicalPackage"],
        ],
        "exitCode": completed.returncode,
        "exactObjectResolved": True,
    }
    return candidates[0], invocation


def write_atomic(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_bytes(payload)
    temporary.replace(path)


def generate(resource_root: Path, source_root: Path, runtime_root: Path,
             output: Path, deploy: bool) -> dict[str, Any]:
    umodel, packages = verify_tool_and_packages(resource_root, source_root)
    rows = []
    invocations = []
    with tempfile.TemporaryDirectory(prefix="artist31470-v4-texture-") as temporary:
        stage = Path(temporary)
        for expected in EXPECTED_ASSETS:
            exported, invocation = extract_asset(umodel, source_root, stage, expected)
            actual = parse_dds(exported)
            for key, value in expected["dds"].items():
                require(actual[key] == value,
                        f"extracted DDS identity changed: {expected['logicalTexturePath']}:{key}")
            destination = runtime_root / Path(expected["runtimeAssetId"]).name
            if deploy:
                destination.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(exported, destination)
            require(not deploy or (destination.is_file() and file_hash(destination) == actual["rawSha256"]),
                    f"runtime DDS deployment failed: {destination}")
            rows.append({
                **{key: value for key, value in expected.items() if key != "dds"},
                "dds": actual,
                "runtimeRelativePath": destination.relative_to(REPO_ROOT).as_posix(),
                "runtimeDeploymentAdmission": deploy,
                "materialProgramAdmission": False,
                "nativeVfPassAdmission": False,
                "productAdmission": False,
            })
            invocations.append(invocation)

    receipt = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "scope": "EXACT_TEXTURE_PAYLOAD_ACQUISITION_ONLY",
        "toolIdentity": {
            "path": SCRIPT_PATH.relative_to(REPO_ROOT).as_posix(),
            "canonicalTextSha256": hashlib.sha256(
                SCRIPT_PATH.read_text(encoding="utf-8").replace("\r\n", "\n").encode("utf-8")
            ).hexdigest(),
            "umodel": {"relativePath": UMODEL_RELATIVE_PATH.as_posix(), **EXPECTED_UMODEL},
        },
        "sourcePackages": packages,
        "extractionInvocations": invocations,
        "assets": rows,
        "decision": {
            "exactDdsPayloadAdmission": True,
            "runtimeDeploymentAdmission": deploy,
            "materialProgramAdmission": False,
            "nativeVfPassAdmission": False,
            "productAdmission": False,
        },
        "summary": {
            "packageCount": len(packages),
            "assetCount": len(rows),
            "deployedAssetCount": len(rows) if deploy else 0,
            "materialProgramAdmittedCount": 0,
            "productAdmittedCount": 0,
        },
    }
    receipt["receiptSha256"] = canonical_sha(receipt)
    return receipt


def validate(receipt: dict[str, Any], output: Path) -> None:
    require(receipt.get("schema") == SCHEMA, "receipt schema changed")
    require(receipt.get("formatVersion") == FORMAT_VERSION, "receipt version changed")
    sealed = dict(receipt)
    claimed = sealed.pop("receiptSha256", None)
    require(claimed == canonical_sha(sealed), "receipt seal changed")
    require(receipt.get("summary", {}).get("assetCount") == len(EXPECTED_ASSETS),
            "asset denominator changed")
    for row, expected in zip(receipt["assets"], EXPECTED_ASSETS, strict=True):
        require(row["logicalTexturePath"] == expected["logicalTexturePath"], "asset order changed")
        require(row["dds"]["rawSha256"] == expected["dds"]["rawSha256"], "DDS SHA changed")
        runtime_path = REPO_ROOT / row["runtimeRelativePath"]
        if row["runtimeDeploymentAdmission"]:
            require(runtime_path.is_file(), f"deployed runtime DDS is missing: {runtime_path}")
            require(file_hash(runtime_path) == row["dds"]["rawSha256"], "runtime DDS changed")
    require(output.parent == DEFAULT_OUTPUT.parent or output.is_absolute(), "output path is invalid")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--resource-root", type=Path, default=DEFAULT_RESOURCE_ROOT)
    parser.add_argument("--source-root", type=Path, default=DEFAULT_SOURCE_ROOT)
    parser.add_argument("--runtime-root", type=Path, default=DEFAULT_RUNTIME_ROOT)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--validate-only", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--no-deploy", action="store_true")
    args = parser.parse_args()

    if args.validate_only:
        receipt = json.loads(args.output.read_text(encoding="utf-8"))
        validate(receipt, args.output)
        return 0

    receipt = generate(args.resource_root.resolve(), args.source_root.resolve(),
                       args.runtime_root.resolve(), args.output.resolve(), not args.no_deploy)
    validate(receipt, args.output.resolve())
    payload = canonical_json_bytes(receipt)
    if args.check:
        require(args.output.is_file() and args.output.read_bytes() == payload,
                "receipt differs from exact rebuild")
    else:
        write_atomic(args.output, payload)
    print(f"Artist 31470 V4 missing textures: {len(receipt['assets'])} exact DDS payload(s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

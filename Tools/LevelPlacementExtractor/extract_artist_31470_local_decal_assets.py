#!/usr/bin/env python3
"""Seal source-era LocalDecal defaults and deploy exact Artist-owned DDS lanes.

This acquisition receipt proves payload/default identities only.  It does not
admit the native FLocalDecal vertex factory, native MRT layout, render state,
or sampler descriptors.  Those remain explicit renderer-registry decisions.
"""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
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
DEFAULT_SOURCE_PACKAGE_ROOT = Path(
    r"C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages"
)
DEFAULT_SOURCE_ERA_EFGAME = (
    DEFAULT_RESOURCE_ROOT
    / "01_Extracted/Effect/ARTIST/31470_TrackA_20260812/"
      "OfficialSourceV975/NU1V7NCQ4YAE9ZPJVNOQS.u"
)
DEFAULT_RUNTIME_ROOT = REPO_ROOT / "Client/Bin/Resources/Effect/Artist/Textures"
DEFAULT_OUTPUT = (
    REPO_ROOT
    / "Data/Effects/Imported/Artist/Materials/"
      "skill.31470.local-decal-acquisition.receipt.json"
)
UMODEL_RELATIVE_PATH = Path(
    "06_Tools/UEViewerLostArk_runtime/umodel_lostark_v7.exe"
)
SCHEMA = "lostark.artist-31470-local-decal-acquisition-receipt"
FORMAT_VERSION = 1

EXPECTED_UMODEL = {
    "byteCount": 1_766_400,
    "rawSha256": "b9573cdcbb7e9d26dbf60a0e3af47fb5af8543140873da8483c26d58cf40b249",
}
EXPECTED_TEXTURE_PACKAGE = {
    "logicalPackage": "FX_TEX_01",
    "fileName": "YGI3SORGM3I18GHA5BMJ8L5CZ.upk",
    "byteCount": 3_432_777,
    "md5": "09a0b006d29ef00ce80a0440629a8626",
    "rawSha256": "afc7d3cbb63907a353926c1a58649cc2145da23ada15d4509ae9691f6b80c793",
}
EXPECTED_SOURCE_ERA_EFGAME = {
    "logicalPackage": "EFGame",
    "manifestFileVersion": 975,
    "manifestSequence": 15,
    "fileName": "NU1V7NCQ4YAE9ZPJVNOQS.u",
    "byteCount": 892_168,
    "packedByteCount": 712_501,
    "md5": "79c030dc4b00320b1d078b50d675bed3",
    "packedMd5": "d1ebd11fa1c1949406299aa887c5cc33",
    "rawSha256": "441c095aa79c84940bac3069f3568d5311dc15033f84b45b5e9cab5cd60a4c30",
}
EXPECTED_CDO = {
    "objectPath": "Default__EFParticleModuleTypeDataDecal",
    "className": "efparticlemoduletypedatadecal",
    "exportIndex": 9471,
    "recordSha256": "9393b946f4f13795b57b1c67692d921a5ca3091779f835565aabc0b9b03e3c3f",
    "properties": {
        "defaultsize": {
            "type": "structproperty",
            "structtype": "vector2d",
            "value": {"x": 50.0, "y": 50.0},
        },
        "farplane": {
            "type": "floatproperty",
            "structtype": None,
            "value": 300.0,
        },
        "blendrange": {
            "type": "structproperty",
            "structtype": "vector2d",
            "value": {"x": 100.0, "y": 100.0},
        },
        "bonlycalcrotationyaw": {
            "type": "boolproperty",
            "structtype": None,
            "value": True,
        },
        "bsupported3ddrawmode": {
            "type": "boolproperty",
            "structtype": None,
            "value": True,
        },
    },
}
EXPECTED_ASSETS = (
    {
        "logicalTexturePath": "fx_tex_01.fx_c_decal_002_2",
        "objectName": "fx_c_decal_002_2",
        "sourceShaderRegister": "t0",
        "sourceShaderSampler": "s0",
        "sourceShaderChannel": "B",
        "sourceSemantic": "height_parallax",
        "runtimeAssetId": "Effect/Artist/Textures/fx_c_decal_002_2.dds",
        "dds": {
            "byteCount": 32_896,
            "rawSha256": "0ac0be83ed4e5e0d9c2637ba6daecaec90bca48629aa90533a65923544c8ba83",
            "width": 256,
            "height": 256,
            "fourCC": "DXT1",
        },
    },
    {
        "logicalTexturePath": "fx_tex_01.fx_c_decal_002_1",
        "objectName": "fx_c_decal_002_1",
        "sourceShaderRegister": "t2",
        "sourceShaderSampler": "s5",
        "sourceShaderChannel": "G",
        "sourceSemantic": "dissolve_mask",
        "runtimeAssetId": "Effect/Artist/Textures/fx_c_decal_002_1.dds",
        "dds": {
            "byteCount": 32_896,
            "rawSha256": "03ee85efabffc4eff947077095e781ab115bc78efd6440aba4ce2e1138b25ad9",
            "width": 256,
            "height": 256,
            "fourCC": "DXT1",
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
    return (
        json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":"))
        + "\n"
    ).encode("utf-8")


def canonical_sha(value: Any) -> str:
    return hashlib.sha256(canonical_json_bytes(value)).hexdigest()


def tracked_text_sha(path: Path) -> str:
    text = path.read_text(encoding="utf-8")
    return hashlib.sha256(
        text.replace("\r\n", "\n").replace("\r", "\n").encode("utf-8")
    ).hexdigest()


def parse_dds(path: Path) -> dict[str, Any]:
    raw = path.read_bytes()
    require(len(raw) >= 128 and raw[:4] == b"DDS ", f"invalid DDS: {path}")
    require(struct.unpack_from("<I", raw, 4)[0] == 124, f"invalid DDS header: {path}")
    height = struct.unpack_from("<I", raw, 12)[0]
    width = struct.unpack_from("<I", raw, 16)[0]
    mip_count = struct.unpack_from("<I", raw, 28)[0]
    fourcc = raw[84:88].decode("ascii")
    require(mip_count in (0, 1), f"unexpected DDS mip chain: {path}")
    require(fourcc == "DXT1", f"unexpected LocalDecal DDS format: {path}")
    payload = max(1, (width + 3) // 4) * max(1, (height + 3) // 4) * 8
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


def load_source_era_cdo(path: Path) -> dict[str, Any]:
    placements_path = SCRIPT_PATH.with_name("extract_ue3_placements.py")
    closure_path = SCRIPT_PATH.with_name("extract_ue3_effect_material_closure.py")
    semantics_path = SCRIPT_PATH.with_name(
        "build_artist_31470_source_execution_semantics.py"
    )
    for module_path, module_name in (
        (placements_path, "local_decal_ue3_placements"),
        (closure_path, "local_decal_material_closure"),
        (semantics_path, "local_decal_source_semantics"),
    ):
        spec = importlib.util.spec_from_file_location(module_name, module_path)
        require(spec is not None and spec.loader is not None,
                f"cannot load parser dependency: {module_path}")
        module = importlib.util.module_from_spec(spec)
        import sys
        sys.modules[module_name] = module
        spec.loader.exec_module(module)
        if module_name == "local_decal_ue3_placements":
            placements = module
        elif module_name == "local_decal_material_closure":
            closure = module
        else:
            semantics = module
    package = closure.load_package(path, placements.LOSTARK_KR_AES_KEY)
    return semantics.decode_cdo(package, EXPECTED_CDO["objectPath"])


def verify_inputs(resource_root: Path, source_package_root: Path,
                  source_era_efgame: Path) -> tuple[Path, Path, dict[str, Any]]:
    umodel = resource_root / UMODEL_RELATIVE_PATH
    require(umodel.is_file(), f"pinned UModel is missing: {umodel}")
    require(umodel.stat().st_size == EXPECTED_UMODEL["byteCount"],
            "UModel byte count changed")
    require(file_hash(umodel) == EXPECTED_UMODEL["rawSha256"],
            "UModel SHA changed")
    package = source_package_root / EXPECTED_TEXTURE_PACKAGE["fileName"]
    require(package.is_file(), f"source texture package is missing: {package}")
    require(package.stat().st_size == EXPECTED_TEXTURE_PACKAGE["byteCount"],
            "source texture package byte count changed")
    require(file_hash(package, "md5") == EXPECTED_TEXTURE_PACKAGE["md5"],
            "source texture package MD5 changed")
    require(file_hash(package) == EXPECTED_TEXTURE_PACKAGE["rawSha256"],
            "source texture package SHA changed")
    require(source_era_efgame.is_file(),
            f"source-era EFGame package is missing: {source_era_efgame}")
    require(source_era_efgame.stat().st_size ==
            EXPECTED_SOURCE_ERA_EFGAME["byteCount"],
            "source-era EFGame byte count changed")
    require(file_hash(source_era_efgame, "md5") ==
            EXPECTED_SOURCE_ERA_EFGAME["md5"],
            "source-era EFGame MD5 changed")
    require(file_hash(source_era_efgame) ==
            EXPECTED_SOURCE_ERA_EFGAME["rawSha256"],
            "source-era EFGame SHA changed")
    cdo = load_source_era_cdo(source_era_efgame)
    require(cdo == EXPECTED_CDO, "source-era LocalDecal CDO changed")
    return umodel, package, cdo


def extract_asset(umodel: Path, source_package_root: Path, stage: Path,
                  expected: dict[str, Any]) -> tuple[Path, dict[str, Any]]:
    command = [
        str(umodel), "-export", "-game=lostark", "-kr", "-nameresolve",
        f"-path={source_package_root}", f"-out={stage}", "-dds",
        "-nooverwrite", f"-obj={expected['objectName']}", "FX_TEX_01",
    ]
    completed = subprocess.run(
        command, cwd=umodel.parent, text=True, capture_output=True,
        timeout=180, check=False
    )
    require(completed.returncode == 0,
            f"UModel export failed ({completed.returncode}):\n"
            f"{completed.stdout}\n{completed.stderr}")
    require(f'Export "{expected["objectName"]}" was found' in completed.stdout,
            f"UModel did not resolve {expected['logicalTexturePath']}")
    candidates = list(stage.rglob(f"{expected['objectName']}.dds"))
    require(len(candidates) == 1,
            f"expected one exported DDS, found {len(candidates)}")
    return candidates[0], {
        "logicalPackage": "FX_TEX_01",
        "objectName": expected["objectName"],
        "argvTemplate": [
            "{umodel}", "-export", "-game=lostark", "-kr", "-nameresolve",
            "-path={sourcePackageRoot}", "-out={temporaryStage}", "-dds",
            "-nooverwrite", f"-obj={expected['objectName']}", "FX_TEX_01",
        ],
        "exitCode": completed.returncode,
        "exactObjectResolved": True,
    }


def write_atomic(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_bytes(payload)
    temporary.replace(path)


def generate(resource_root: Path, source_package_root: Path,
             source_era_efgame: Path, runtime_root: Path,
             deploy: bool) -> dict[str, Any]:
    umodel, package, cdo = verify_inputs(
        resource_root, source_package_root, source_era_efgame
    )
    rows: list[dict[str, Any]] = []
    invocations: list[dict[str, Any]] = []
    with tempfile.TemporaryDirectory(prefix="artist31470-local-decal-") as temp:
        stage = Path(temp)
        for expected in EXPECTED_ASSETS:
            exported, invocation = extract_asset(
                umodel, source_package_root, stage, expected
            )
            actual = parse_dds(exported)
            for key, value in expected["dds"].items():
                require(actual[key] == value,
                        f"LocalDecal DDS changed: "
                        f"{expected['logicalTexturePath']}:{key}")
            destination = runtime_root / Path(expected["runtimeAssetId"]).name
            if deploy:
                destination.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(exported, destination)
            require(not deploy or (
                destination.is_file()
                and file_hash(destination) == actual["rawSha256"]
            ), f"LocalDecal DDS deployment failed: {destination}")
            rows.append({
                **{key: value for key, value in expected.items() if key != "dds"},
                "sourceOccurrenceIds": ["source-active-020", "source-active-021"],
                "dds": actual,
                "runtimeRelativePath": destination.relative_to(REPO_ROOT).as_posix(),
                "runtimeDeploymentAdmission": deploy,
                "materialProgramAdmission": False,
                "nativeVfPassAdmission": False,
                "productAdmission": False,
            })
            invocations.append(invocation)
    receipt: dict[str, Any] = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "scope": "LOCAL_DECAL_SOURCE_DEFAULT_AND_DDS_ACQUISITION_ONLY",
        "toolIdentity": {
            "path": SCRIPT_PATH.relative_to(REPO_ROOT).as_posix(),
            "canonicalTextSha256": tracked_text_sha(SCRIPT_PATH),
            "umodel": {
                "relativePath": UMODEL_RELATIVE_PATH.as_posix(),
                **EXPECTED_UMODEL,
            },
        },
        "sourceEraEfgame": {
            **EXPECTED_SOURCE_ERA_EFGAME,
            "sourceRelativePath": source_era_efgame.as_posix(),
            "classDefaultObject": cdo,
        },
        "sourceTexturePackage": {
            **EXPECTED_TEXTURE_PACKAGE,
            "sourceRelativePath": package.as_posix(),
        },
        "extractionInvocations": invocations,
        "assets": rows,
        "nativeShaderEvidence": {
            "vertexFactoryCandidate": "flocaldecalvertexfactory",
            "vertexShaderId": "5d79421dc8571c45aa49790f50274f51",
            "vertexDxbcSha256":
                "94072a22ef44ce0319bc9bd7915bded5f7ce94d9a5badde32def4fc01a4bff4d",
            "pixelShaderId": "ef68ae7aec8f94458ef2cbb3c6bafd2d",
            "pixelDxbcSha256":
                "d5a1d55021ff7e2a06e4de978e6850da56b9ff3ba6c7f68321f04852bd28ff1c",
            "nativeOutputs": ["SV_Target0", "SV_Target2", "SV_Target3",
                              "SV_Target4", "SV_Target5"],
        },
        "decision": {
            "sourceEraClassDefaultAdmission": True,
            "exactDdsPayloadAdmission": True,
            "runtimeDeploymentAdmission": deploy,
            "boundedSemanticReplayEligible": deploy,
            "nativeVfPassAdmission": False,
            "nativeMrtAdmission": False,
            "productAdmission": False,
        },
        "summary": {
            "sourceOccurrenceCount": 2,
            "assetCount": len(rows),
            "deployedAssetCount": len(rows) if deploy else 0,
            "nativeVfPassAdmittedCount": 0,
        },
    }
    receipt["receiptSha256"] = canonical_sha(receipt)
    return receipt


def validate(receipt: dict[str, Any]) -> None:
    require(receipt.get("schema") == SCHEMA, "receipt schema changed")
    require(receipt.get("formatVersion") == FORMAT_VERSION,
            "receipt version changed")
    sealed = dict(receipt)
    claimed = sealed.pop("receiptSha256", None)
    require(claimed == canonical_sha(sealed), "receipt seal changed")
    require(
        receipt.get("sourceEraEfgame", {}).get("classDefaultObject")
        == EXPECTED_CDO,
        "source-era CDO receipt changed",
    )
    require(receipt.get("summary", {}).get("assetCount") ==
            len(EXPECTED_ASSETS), "LocalDecal asset denominator changed")
    for row, expected in zip(
        receipt.get("assets", []), EXPECTED_ASSETS, strict=True
    ):
        require(row["logicalTexturePath"] == expected["logicalTexturePath"],
                "LocalDecal asset order changed")
        require(row["dds"]["rawSha256"] == expected["dds"]["rawSha256"],
                "LocalDecal DDS SHA changed")
        if row["runtimeDeploymentAdmission"]:
            runtime_path = REPO_ROOT / row["runtimeRelativePath"]
            require(runtime_path.is_file(),
                    f"deployed LocalDecal DDS is missing: {runtime_path}")
            require(file_hash(runtime_path) == row["dds"]["rawSha256"],
                    "deployed LocalDecal DDS changed")
    decision = receipt.get("decision") or {}
    require(decision.get("sourceEraClassDefaultAdmission") is True
            and decision.get("exactDdsPayloadAdmission") is True
            and decision.get("nativeVfPassAdmission") is False
            and decision.get("nativeMrtAdmission") is False
            and decision.get("productAdmission") is False,
            "LocalDecal acquisition/admission boundary changed")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--resource-root", type=Path,
                        default=DEFAULT_RESOURCE_ROOT)
    parser.add_argument("--source-package-root", type=Path,
                        default=DEFAULT_SOURCE_PACKAGE_ROOT)
    parser.add_argument("--source-era-efgame", type=Path,
                        default=DEFAULT_SOURCE_ERA_EFGAME)
    parser.add_argument("--runtime-root", type=Path,
                        default=DEFAULT_RUNTIME_ROOT)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--validate-only", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--no-deploy", action="store_true")
    args = parser.parse_args()
    if args.validate_only:
        receipt = json.loads(args.output.read_text(encoding="utf-8"))
        validate(receipt)
        return 0
    receipt = generate(
        args.resource_root.resolve(), args.source_package_root.resolve(),
        args.source_era_efgame.resolve(), args.runtime_root.resolve(),
        not args.no_deploy
    )
    validate(receipt)
    payload = canonical_json_bytes(receipt)
    if args.check:
        require(args.output.is_file() and args.output.read_bytes() == payload,
                "LocalDecal receipt differs from exact rebuild")
    else:
        write_atomic(args.output, payload)
    print(
        f"Artist 31470 LocalDecal: {len(receipt['assets'])} exact DDS payload(s), "
        "source-era CDO admitted, native VF/pass remains false"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

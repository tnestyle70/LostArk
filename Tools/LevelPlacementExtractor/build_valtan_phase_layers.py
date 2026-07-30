#!/usr/bin/env python3
"""Bake video-matched Valtan sky-phase proxy layers from exact UE3 textures."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import shutil
import subprocess
import tempfile
from pathlib import Path
from typing import Any


MATERIAL = "bg_lut_zamount_cloudplane_inst_02_old"
SPECS: tuple[dict[str, str], ...] = (
    {
        "assetId": "VALTAN_PHASE_SPACEHOLE_CLOUD",
        "texture": "spacehole_materials/space_m1/FX_TEX_02/Texture2D/fx_d_cloud_031.png",
        "evidence": "par_d_spacehole_03 exact texture; proxy plane topology",
    },
    {
        "assetId": "VALTAN_PHASE_SPACEHOLE_CORE",
        "texture": "spacehole_materials/space_m1/FX_TEX_02/Texture2D/fx_d_atypical_019.png",
        "evidence": "par_d_spacehole_03 exact texture; proxy plane topology",
    },
    {
        "assetId": "VALTAN_PHASE_SPACEHOLE_STREAK",
        "texture": "spacehole_materials/space_m1/FX_TEX_00/Texture2D/fx_a_cloud_017.png",
        "evidence": "par_d_spacehole_03 exact texture; proxy plane topology",
    },
    {
        "assetId": "VALTAN_PHASE_CHAOS_RING",
        "texture": "chaosgate_materials/m04/FX_TEX_02/Texture2D/fx_d_shockwave_001_ycl.png",
        "evidence": "par_d_hugechaosgate_01 exact ring texture; proxy plane topology",
    },
    {
        "assetId": "VALTAN_PHASE_CHAOS_CLOUD",
        "texture": "chaosgate_materials/m07/FX_TEX_05/Texture2D/fx_k_cloudtilie_01.png",
        "evidence": "par_d_hugechaosgate_01 exact cloud texture; proxy plane topology",
    },
    {
        "assetId": "VALTAN_PHASE_CHAOS_ELECTRIC",
        "texture": "chaosgate_materials/m06/FX_TEX_05/Texture2D/fx_k_electile_02.png",
        "evidence": "par_d_hugechaosgate_01 exact electric texture; proxy plane topology",
    },
)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def atomic_copy(source: Path, destination: Path) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)
    temporary = destination.with_name(destination.name + ".installing")
    shutil.copy2(source, temporary)
    os.replace(temporary, destination)


def install_tree(source: Path, destination: Path) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for path in sorted(source.rglob("*")):
        if not path.is_file():
            continue
        relative = path.relative_to(source)
        target = destination / relative
        atomic_copy(path, target)
        rows.append(
            {
                "path": relative.as_posix(),
                "bytes": target.stat().st_size,
                "sha256": sha256(target),
            }
        )
    return rows


def atomic_write(path: Path, value: object) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle, name = tempfile.mkstemp(prefix=path.name + ".", suffix=".tmp", dir=path.parent)
    temporary = Path(name)
    try:
        with os.fdopen(handle, "w", encoding="utf-8", newline="\n") as output:
            json.dump(value, output, ensure_ascii=False, indent=2)
            output.write("\n")
            output.flush()
            os.fsync(output.fileno())
        os.replace(temporary, path)
    except BaseException:
        temporary.unlink(missing_ok=True)
        raise


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--converter", required=True, type=Path)
    parser.add_argument("--audit-root", required=True, type=Path)
    parser.add_argument("--source-plane", required=True, type=Path)
    parser.add_argument("--pack-root", required=True, type=Path)
    parser.add_argument("--runtime-root", required=True, type=Path)
    parser.add_argument("--receipt", required=True, type=Path)
    args = parser.parse_args()

    for path in (args.converter, args.audit_root, args.source_plane):
        if not path.exists():
            parser.error(f"required path is missing: {path}")

    receipt_rows: list[dict[str, Any]] = []
    with tempfile.TemporaryDirectory(prefix="valtan-phase-") as temporary_name:
        stage = Path(temporary_name)
        for spec in SPECS:
            asset_id = spec["assetId"]
            texture = args.audit_root / spec["texture"]
            if not texture.is_file():
                raise FileNotFoundError(texture)
            asset_stage = stage / asset_id
            output = asset_stage / f"{asset_id}.wmodel"
            output.parent.mkdir(parents=True)
            command = [
                str(args.converter),
                str(args.source_plane),
                "-o",
                str(output),
                "--pretransform",
                "--scale",
                "100",
                "--no-auto-textures",
                "--material-remap",
                f"{MATERIAL}={texture}",
                "--opacity-remap",
                f"{MATERIAL}={texture}",
            ]
            converted = subprocess.run(
                command, check=True, capture_output=True, text=True
            )
            info = subprocess.run(
                [str(args.converter), "info", str(output)],
                check=True,
                capture_output=True,
                text=True,
            ).stdout
            expected = f"base=textures/{texture.name}"
            if "material-version=2" not in info or expected not in info:
                raise RuntimeError(f"phase layer verification failed: {asset_id}\n{info}")
            pack_files = install_tree(asset_stage, args.pack_root / asset_id)
            runtime_files = install_tree(
                asset_stage, args.runtime_root / "Map" / "ValtanPhase" / asset_id
            )
            if pack_files != runtime_files:
                raise RuntimeError(f"phase install mismatch: {asset_id}")
            receipt_rows.append(
                {
                    "assetId": asset_id,
                    "evidence": spec["evidence"],
                    "sourceTexture": str(texture),
                    "sourceTextureSha256": sha256(texture),
                    "converterOutput": converted.stdout.strip(),
                    "files": pack_files,
                }
            )

    receipt = {
        "schemaVersion": 1,
        "areaId": "LV_LUT_HEARTRB_ED",
        "topology": "video-matched proxy plane; original UE3 ParticleSystem topology not claimed",
        "timing": "manual phase selector; exact TriggerMapData/Matinee timing unresolved",
        "assetCount": len(receipt_rows),
        "assets": receipt_rows,
    }
    atomic_write(args.receipt, receipt)
    print(json.dumps({"assetCount": len(receipt_rows), "status": "installed"}))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

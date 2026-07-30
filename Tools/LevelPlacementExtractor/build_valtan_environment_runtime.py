#!/usr/bin/env python3
"""Recook exact HeartRB chain/cloud/sky materials and install atomically."""

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


AREA_ID = "LV_LUT_HEARTRB_ED"

SPECS: tuple[dict[str, Any], ...] = (
    {
        "assetId": "MAP_2F7659F7259C_BG_FAT_KARLAJAVIL_CHAIN01A_SM",
        "source": "chainA/BG_FAT_KARLAJAVIL_C/StaticMesh3/bg_fat_karlajavil_chain01a_sm.gltf",
        "material": "bg_fat_karlajavil_elevator01a_mi_khb",
        "textures": {
            "material": "chainA/BG_FAT_KARLAJAVIL_C/Texture2D/bg_fat_karlajavil_elevator01a_d_khb.png",
            "normal": "chainA/BG_FAT_KARLAJAVIL_C/Texture2D/bg_fat_karlajavil_elevator01a_n_khb.png",
            "specular": "chainA/BG_FAT_KARLAJAVIL_C/Texture2D/bg_fat_karlajavil_elevator01a_s_khb.png",
        },
        "provenance": "MATERIAL_EXACT",
    },
    {
        "assetId": "MAP_9ACE87B2E07E_BG_FAT_KARLAJAVIL_CHAIN01B_SM",
        "source": "chainB/BG_FAT_KARLAJAVIL_C/StaticMesh3/bg_fat_karlajavil_chain01b_sm.gltf",
        "material": "bg_fat_karlajavil_elevator01a_mi_khb",
        "textures": {
            "material": "chainB/BG_FAT_KARLAJAVIL_C/Texture2D/bg_fat_karlajavil_elevator01a_d_khb.png",
            "normal": "chainB/BG_FAT_KARLAJAVIL_C/Texture2D/bg_fat_karlajavil_elevator01a_n_khb.png",
            "specular": "chainB/BG_FAT_KARLAJAVIL_C/Texture2D/bg_fat_karlajavil_elevator01a_s_khb.png",
        },
        "provenance": "MATERIAL_EXACT",
    },
    {
        "assetId": "MAP_3CC7E67937A0_BG_LUT_ZAMOUNT_CLOUDPLANE_SM_OLD",
        "source": "cloud/BG_LUT_ZAMOUNT_A/StaticMesh3/bg_lut_zamount_cloudplane_sm_old.gltf",
        "material": "bg_lut_zamount_cloudplane_inst_02_old",
        "textures": {
            "material": "cloud/BG_LUT_ZAMOUNT_D/Texture2D/bg_lut_zamount_c_d3_old.png",
            "normal": "cloud/BG_LUT_ZAMOUNT_D/Texture2D/bg_lut_zamount_c_n3_old.png",
            "opacity": "cloud/EFMASTER_MATERIAL_SKYMATTE/Texture2D/plane_cloudtex_01_d.png",
        },
        "provenance": "MATERIAL_EXACT",
    },
    {
        "assetId": "MAP_EDDEDF2CF6A1_SKY_MIRROR_SM",
        "source": "skyMirror/LV_MATTE/StaticMesh3/sky_mirror_sm.gltf",
        "material": "sky_base_opa",
        "textures": {
            "material": "skyMirror/LV_MATTE/Texture2D/lv_sky_0161_d.png",
        },
        "provenance": "MATERIAL_EXACT",
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
    installed: list[dict[str, Any]] = []
    for path in sorted(source.rglob("*")):
        if not path.is_file():
            continue
        relative = path.relative_to(source)
        target = destination / relative
        atomic_copy(path, target)
        installed.append(
            {
                "path": relative.as_posix(),
                "bytes": target.stat().st_size,
                "sha256": sha256(target),
            }
        )
    return installed


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--converter", required=True, type=Path)
    parser.add_argument("--audit-root", required=True, type=Path)
    parser.add_argument("--pack-root", required=True, type=Path)
    parser.add_argument("--runtime-root", required=True, type=Path)
    parser.add_argument("--receipt", required=True, type=Path)
    args = parser.parse_args()

    for path in (args.converter, args.audit_root, args.pack_root, args.runtime_root):
        if not path.exists():
            parser.error(f"required path is missing: {path}")

    results: list[dict[str, Any]] = []
    with tempfile.TemporaryDirectory(prefix="valtan-environment-") as raw_stage:
        stage_root = Path(raw_stage)
        for spec in SPECS:
            asset_id = spec["assetId"]
            source = args.audit_root / spec["source"]
            if not source.is_file():
                raise FileNotFoundError(source)
            output_root = stage_root / asset_id
            output = output_root / f"{asset_id}.wmodel"
            output_root.mkdir(parents=True)
            command = [
                str(args.converter),
                str(source),
                "-o",
                str(output),
                "--pretransform",
                "--no-auto-textures",
                "--scale",
                "100",
            ]
            for slot, relative in spec["textures"].items():
                texture = args.audit_root / relative
                if not texture.is_file():
                    raise FileNotFoundError(texture)
                command.extend(
                    [f"--{slot}-remap", f"{spec['material']}={texture}"]
                )
            cooked = subprocess.run(command, check=True, capture_output=True, text=True)
            info = subprocess.run(
                [str(args.converter), "info", str(output)],
                check=True,
                capture_output=True,
                text=True,
            ).stdout
            if "material-version=2" not in info or "base=textures/" not in info:
                raise RuntimeError(f"invalid material receipt for {asset_id}:\n{info}")
            # `info` currently omits the WMA2 opacity slot.  Validate the slots it
            # does expose by exact relative path and verify every remapped texture
            # was copied into the cooked asset folder.  Runtime loading provides
            # the final opacity-slot contract check.
            reported_slots = {
                "material": "base",
                "normal": "normal",
                "specular": "specular",
                "emissive": "emissive",
                "orm": "orm",
            }
            for slot, relative in spec["textures"].items():
                texture_name = Path(relative).name
                copied_texture = output_root / "textures" / texture_name
                if not copied_texture.is_file():
                    raise RuntimeError(
                        f"missing cooked texture {copied_texture} for {asset_id}"
                    )
                if slot in reported_slots:
                    expected = f"{reported_slots[slot]}=textures/{texture_name}"
                    if expected not in info:
                        raise RuntimeError(
                            f"missing {slot} slot {expected!r} for {asset_id}:\n{info}"
                        )

            pack_files = install_tree(output_root, args.pack_root / asset_id)
            runtime_files = install_tree(
                output_root,
                args.runtime_root / "Map" / AREA_ID / asset_id,
            )
            if pack_files != runtime_files:
                raise RuntimeError(f"pack/runtime install mismatch: {asset_id}")
            results.append(
                {
                    "assetId": asset_id,
                    "provenance": spec["provenance"],
                    "source": str(source),
                    "sourceSha256": sha256(source),
                    "converterOutput": cooked.stdout.strip(),
                    "info": info.strip().splitlines(),
                    "files": pack_files,
                }
            )

    receipt = {
        "schemaVersion": 1,
        "areaId": AREA_ID,
        "assetCount": len(results),
        "assets": results,
    }
    args.receipt.parent.mkdir(parents=True, exist_ok=True)
    temporary = args.receipt.with_name(args.receipt.name + ".tmp")
    temporary.write_text(
        json.dumps(receipt, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    os.replace(temporary, args.receipt)
    print(json.dumps({"assetCount": len(results), "status": "installed"}))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

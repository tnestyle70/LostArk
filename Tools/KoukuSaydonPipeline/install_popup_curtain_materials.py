"""Verify or install the exact embedded texture closure of the popup curtain.

The effect material overrides shading, but CModel still loads the WModel's
embedded material slots. Copying only the mesh therefore cannot prepare it.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parents[2]
SOURCE = Path("Map/LV_LUT_MIDNIGHTC_ED/MAP_50A8C687EF41_BG_RAD_KOUKUSATON_CURTAIN01C_SM_OVR_231D0AB2E6CF")
DESTINATION = Path("Effect/KoukuSaydon/FullRestore/Meshes")
MESH_HASH = "eeed76e65b41f09adbc89b08d22b6ffcce83fefdad74e86d68d7638619b38bde"
TEXTURES = {
    "7adc5a2a9247_bg_rad_koukusaton_curtain01_d.dds": "7adc5a2a92475e74979628996ebe06ddb0f5486bd7634afc631f4e83df2b7d54",
    "9934688136d4_spec.dds": "9934688136d4cf587aec869cbf58b948b98d11e8d133dc9d82c6202688abca90",
    "8f7cda17b246_fx_tex_a_02710.dds": "8f7cda17b2462abb53b1db065263b954aa9602c849306a595cb86e02f93e68f9",
}


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(root: Path, install: bool) -> dict:
    resources = root / "Client/Bin/Resources"
    mesh = resources / DESTINATION / "bg_rad_koukusaton_curtain01c_sm.wmodel"
    if digest(mesh) != MESH_HASH:
        raise ValueError("Curtain geometry changed; remeasure its embedded texture references first.")
    # Validate the whole source set before writing any dependency.
    for name, expected in TEXTURES.items():
        if digest(resources / SOURCE / "textures" / name) != expected:
            raise ValueError("Original curtain texture differs: " + name)
    installed = []
    for name, expected in TEXTURES.items():
        target = resources / DESTINATION / "textures" / name
        if target.exists():
            if digest(target) != expected:
                raise ValueError("Existing destination differs; keep it for review: " + str(target))
        elif install:
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(resources / SOURCE / "textures" / name, target)
        else:
            raise ValueError("Missing embedded material dependency: " + str(target))
        if digest(target) != expected:
            raise ValueError("Copied curtain texture verification failed: " + name)
        installed.append({"assetId": target.relative_to(resources).as_posix(), "sha256": expected})
    return {"meshSha256": MESH_HASH, "textures": installed}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repository-root", type=Path, default=ROOT)
    parser.add_argument("--install", action="store_true")
    args = parser.parse_args()
    print(json.dumps(run(args.repository_root.resolve(), args.install), sort_keys=True))

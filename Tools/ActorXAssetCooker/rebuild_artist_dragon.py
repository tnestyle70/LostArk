"""Stage Artist's original DRA geometry with corrected ActorX animations.

The source geometry and cue transform are unchanged. Output is staged in the
requested directory, never installed automatically.
"""
from __future__ import annotations
import argparse
import json
from pathlib import Path
import subprocess

import build_umodel_gltf_psa as intake
import retime_wmodel_ticks as ticks


def rebuild(args: argparse.Namespace) -> dict:
    source = intake.bounded_file(args.gltf, "DRA glTF")
    if source.stem != "sk_sdm_dra_00_sk":
        raise ValueError("This authoring adjustment is only for SK_SDM_DRA_00")
    document = json.loads(source.read_text(encoding="utf-8"))
    psa = intake.read_psa(intake.bounded_file(args.psa, "DRA PSA"))
    source_buffer, joints = intake.validate_gltf(document, source, psa)
    payload = bytearray(source_buffer.read_bytes())
    changed = set()
    for mesh in document["meshes"]:
        for primitive in mesh["primitives"]:
            attributes = primitive["attributes"]
            index = attributes["WEIGHTS_0"]
            if index in changed:
                continue
            changed.add(index)
            accessor = document["accessors"][index]
            if accessor["componentType"] != 5121 or not accessor.get("normalized"):
                raise ValueError("Expected UModel's normalized byte skin weights")
            view = document["bufferViews"][accessor["bufferView"]]
            offset = view.get("byteOffset", 0) + accessor.get("byteOffset", 0)
            stride = view.get("byteStride", 4)
            weights = [tuple(x / 255 for x in payload[offset+i*stride:offset+i*stride+4]) for i in range(accessor["count"])]
            attributes["WEIGHTS_0"] = intake.append_accessor(document, payload, weights, "VEC4", 4)
    clips = intake.build_animations(document, payload, psa, joints)
    intake.align4(payload)
    out = args.output_dir.resolve()
    out.mkdir(parents=True, exist_ok=True)
    gltf = out / source.name
    binary = gltf.with_suffix(".bin")
    wmodel = gltf.with_suffix(".wmodel")
    if any(p.exists() for p in (gltf, binary, wmodel)):
        raise ValueError("Staged output already exists")
    document["buffers"][0].update(uri=binary.name, byteLength=len(payload))
    binary.write_bytes(payload)
    gltf.write_text(json.dumps(document, indent=2) + "\n", encoding="utf-8")
    material = document["materials"][0]["name"]
    command = [str(args.converter.resolve()), str(gltf), "-o", str(wmodel), "--no-auto-textures", "--texture-root", str(args.texture_root.resolve()), "--material-remap", material + "=" + args.material_asset]
    process = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    (out / "cook.log").write_bytes(process.stdout)
    if process.returncode:
        raise RuntimeError(f"ModelAssetConverter exited {process.returncode}; see cook.log")
    cooked = ticks.read_bounded(wmodel)
    retiming = ticks.retime(cooked, 30.0, 1000.0)
    wmodel.write_bytes(cooked)
    report = {"sourceGltf": str(source), "sourceGltfSha256": intake.sha256(source), "sourceBufferSha256": intake.sha256(source_buffer), "sourcePsaSha256": intake.sha256(args.psa), "sourceQuaternionConvention": "Undo ActorX Y/W mirror, swap Y/Z, conjugate root only", "geometryUnchanged": True, "clips": clips, "retiming": retiming, "command": command, "outputWmodel": str(wmodel), "outputSha256": intake.sha256(wmodel), "installed": False}
    (out / "receipt.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    return report


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--gltf", type=Path, required=True)
    parser.add_argument("--psa", type=Path, required=True)
    parser.add_argument("--converter", type=Path, required=True)
    parser.add_argument("--texture-root", type=Path, required=True)
    parser.add_argument("--material-asset", required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    result = rebuild(parser.parse_args())
    print(json.dumps({key: result[key] for key in ("outputWmodel", "outputSha256", "installed")}))

"""Stage Artist's DRA geometry with corrected ActorX animations.

Source width is the default. An explicit body-width scale adjusts bind-space
cross sections without changing the skeleton, animation or cue transform.
Output is staged in the requested directory, never installed automatically.
"""
from __future__ import annotations
import argparse
import json
import math
from pathlib import Path
import struct
import subprocess

import build_umodel_gltf_psa as intake
import retime_wmodel_ticks as ticks


def adjust_body_width(document: dict, payload: bytearray, factor: float) -> dict:
    """Author DRA's local X width before skinning; retain its animated path."""
    if not math.isfinite(factor) or factor <= 0.0:
        raise ValueError("DRA body width must be a finite positive scale")
    report = {"axis": "bind-space X", "factor": factor, "sourceValue": 1.0,
              "classification": "SOURCE_UNCHANGED" if factor == 1.0 else "PROJECT_AUTHORED",
              "skeletonAndAnimationTranslationUnchanged": True}
    if factor == 1.0:
        return report
    adjusted = set()
    for mesh in document["meshes"]:
        for primitive in mesh["primitives"]:
            for semantic, width in (("POSITION", 3), ("NORMAL", 3), ("TANGENT", 4)):
                index = primitive["attributes"][semantic]
                if index in adjusted:
                    continue
                adjusted.add(index)
                offset, count = intake.read_accessor_floats(document, payload, index, width)
                for row in range(count):
                    at = offset + row * width * 4
                    values = list(struct.unpack_from("<" + "f" * width, payload, at))
                    values[0] *= 1.0 / factor if semantic == "NORMAL" else factor
                    if semantic != "POSITION":
                        length = math.sqrt(sum(value * value for value in values[:3]))
                        if not math.isfinite(length) or length <= 0.0:
                            raise ValueError("DRA width adjustment produced an invalid normal/tangent")
                        values[:3] = [value / length for value in values[:3]]
                    struct.pack_into("<" + "f" * width, payload, at, *values)
                if semantic == "POSITION":
                    for bound in ("min", "max"):
                        document["accessors"][index][bound][0] *= factor
    return report


def rebuild(args: argparse.Namespace) -> dict:
    source = intake.bounded_file(args.gltf, "DRA glTF")
    if source.stem != "sk_sdm_dra_00_sk":
        raise ValueError("This authoring adjustment is only for SK_SDM_DRA_00")
    document = json.loads(source.read_text(encoding="utf-8"))
    psa = intake.read_psa(intake.bounded_file(args.psa, "DRA PSA"))
    source_buffer, joints = intake.validate_gltf(document, source, psa)
    payload = bytearray(source_buffer.read_bytes())
    width_adjustment = adjust_body_width(document, payload, args.body_width_scale)
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
    report = {"sourceGltf": str(source), "sourceGltfSha256": intake.sha256(source), "sourceBufferSha256": intake.sha256(source_buffer), "sourcePsaSha256": intake.sha256(args.psa), "sourceQuaternionConvention": "Undo ActorX Y/W mirror, swap Y/Z, conjugate root only", "geometryUnchanged": args.body_width_scale == 1.0, "bodyWidthAuthoring": width_adjustment, "clips": clips, "retiming": retiming, "command": command, "outputWmodel": str(wmodel), "outputSha256": intake.sha256(wmodel), "installed": False}
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
    parser.add_argument("--body-width-scale", type=float, default=1.0,
                        help="Project-authored bind-space X width; source value is 1")
    result = rebuild(parser.parse_args())
    print(json.dumps({key: result[key] for key in ("outputWmodel", "outputSha256", "installed")}))

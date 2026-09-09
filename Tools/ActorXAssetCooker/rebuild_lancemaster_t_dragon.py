"""Stage T 34650's two original skeletal sections and its 1.3-rate action clip.

Geometry, inverse binds and joint hierarchy come from the original SkeletalMesh;
the standalone AnimSet contributes named local keys. Nothing is installed here.
"""
from __future__ import annotations

import argparse
import copy
import json
from pathlib import Path
import subprocess

import build_umodel_gltf_psa as intake
import retime_wmodel_ticks as ticks


def rebuild(args: argparse.Namespace) -> dict:
    source = intake.bounded_file(args.gltf, "T dragon glTF")
    if source.stem != "sk_flm_pmshb_00_sk":
        raise ValueError("Expected SK_FLM_GDR_01.Mesh.SK_FLM_PMSHB_00_SK")
    document = json.loads(source.read_bytes())
    psa = intake.read_psa(intake.bounded_file(args.psa, "T dragon PSA"))
    names = [document["nodes"][j]["name"] for j in document["skins"][0]["joints"]]
    if len(names) != 32 or len(set(names)) != 32 or set(names) != set(psa["bones"]):
        raise ValueError("Original 32-joint SkeletalMesh/AnimSet name bijection changed")
    if psa["parents"] != [-1] + [0] * 31:
        raise ValueError("Expected standalone AnimSet's flat placeholder hierarchy")
    order = [psa["bones"].index(name) for name in names]
    for key in ("animation_keys", "scale_keys"):
        if psa[key] is not None:
            psa[key] = [psa[key][frame * 32 + old]
                        for frame in range(len(psa[key]) // 32) for old in order]
    psa["parents"] = [-1 if psa["parents"][old] < 0
                      else order.index(psa["parents"][old]) for old in order]
    psa["bones"] = names
    clip = next(s for s in psa["sequences"] if s["name"] == "sk_dragoncleave_03")
    if clip["rate"] != 30.0 or clip["frame_count"] != 52:
        raise ValueError("Original T clip's 52 frames at 30 Hz changed")
    # LOA base stage 1 notify 14, EFSkelMeshActorAnimSeq.AnimPlayRate, byte 454.
    # Bake that action-specific clock into this isolated resource; the runtime's
    # cooked 30-Hz contract and every other source clip remain unchanged.
    clip["rate"] *= 1.2999999523162842
    source_buffer, joints = intake.validate_gltf(document, source, psa)
    payload = bytearray(source_buffer.read_bytes())
    converted = set()
    for mesh in document["meshes"]:
        for primitive in mesh["primitives"]:
            index = primitive["attributes"]["WEIGHTS_0"]
            if index in converted:
                continue
            converted.add(index)
            accessor = document["accessors"][index]
            if accessor["componentType"] != 5121 or not accessor.get("normalized"):
                raise ValueError("Expected UModel normalized byte skin weights")
            view = document["bufferViews"][accessor["bufferView"]]
            offset = view.get("byteOffset", 0) + accessor.get("byteOffset", 0)
            stride = view.get("byteStride", 4)
            weights = [tuple(x / 255 for x in payload[offset+i*stride:offset+i*stride+4])
                       for i in range(accessor["count"])]
            primitive["attributes"]["WEIGHTS_0"] = intake.append_accessor(
                document, payload, weights, "VEC4", 4)
    clips = intake.build_animations(document, payload, psa, joints)
    document["animations"] = [a for a in document["animations"]
                              if a["name"] == "sk_dragoncleave_03"]
    if len(document["animations"]) != 1 or len(document["meshes"]) != 1 or len(document["meshes"][0]["primitives"]) != 2:
        raise ValueError("Expected one action animation and two original mesh sections")
    intake.align4(payload)
    out = args.output_dir.resolve()
    out.mkdir(parents=True, exist_ok=True)
    binary = out / (source.stem + ".bin")
    if binary.exists():
        raise ValueError("Staged output already exists")
    binary.write_bytes(payload)
    document["buffers"][0].update(uri=binary.name, byteLength=len(payload))
    sections = []
    for index, primitive in enumerate(document["meshes"][0]["primitives"]):
        section = copy.deepcopy(document)
        section["meshes"][0]["primitives"] = [primitive]
        gltf = out / f"{source.stem}.section{index}.gltf"
        wmodel = gltf.with_suffix(".wmodel")
        material = section["materials"][primitive["material"]]["name"]
        gltf.write_text(json.dumps(section, indent=2) + "\n", encoding="utf8")
        command = [str(args.converter.resolve()), str(gltf), "-o", str(wmodel),
                   "--no-auto-textures", "--texture-root", str(args.texture_root.resolve()),
                   "--material-remap", material + "=" + args.material_asset]
        process = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        (out / f"section{index}.cook.log").write_bytes(process.stdout)
        if process.returncode:
            raise RuntimeError(f"ModelAssetConverter exited {process.returncode}")
        cooked = ticks.read_bounded(wmodel)
        retiming = ticks.retime(cooked, 30.0, 1000.0)
        ticks.verify(cooked, retiming, 30.0)
        wmodel.write_bytes(cooked)
        sections.append(dict(section=index, output=str(wmodel), sha256=intake.sha256(wmodel),
                             command=command, retiming=retiming))
    report = dict(sourceGltf=str(source), sourceGltfSha256=intake.sha256(source),
                  sourceBufferSha256=intake.sha256(source_buffer), sourcePsaSha256=intake.sha256(args.psa),
                  jointNameOrder=order, meshHierarchyPreserved=True, geometryUnchanged=True,
                  quaternionConvention="Undo ActorX Y/W mirror, swap Y/Z, conjugate root only",
                  actionPlayRate=1.2999999523162842, sourceClips=clips, sections=sections, installed=False)
    (out / "receipt.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf8")
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
    print(json.dumps(result["sections"], indent=2))

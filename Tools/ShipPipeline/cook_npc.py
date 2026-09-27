#!/usr/bin/env python3
"""Cook a retail composite NPC (body + head SkeletalMesh sharing one skeleton) into one WModel.

Retail NPCs are LookInfo composites: a body mesh, a head mesh and sometimes a weapon. When the head
mesh carries exactly the body's skin joint list (checked here, never assumed) the head primitives are
appended to the body mesh, so one skin and one AnimSet drive both. The rest is the repository chain:

  merged glTF + PSA (selected clips) -> build_umodel_gltf_psa.py --scale 100
  -> ModelAssetConverter -> retime_wmodel_ticks.py (30 ticks/s) -> read_wmodel checks

The weapon is not attached (it is a one-joint rigid mesh that needs a hand socket). Nothing is written
into Resources unless --install is given, and --install never overwrites an existing file.

Usage:
  python Tools/ShipPipeline/cook_npc.py --asset Npc_MN_RHKP_02-2 --body <body.gltf> --head <head.gltf>
      --psa <animset.psa> --clips idle_normal_1_1,idle_normal_1_2,walk_normal_1 [--install]
"""

from __future__ import annotations

import argparse
import copy
import json
import shutil
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import cook_ships as cs  # noqa: E402


def align4(buffer: bytearray) -> None:
    while len(buffer) % 4:
        buffer.append(0)


def merge_head_into_body(body_gltf: Path, head_gltf: Path, out_gltf: Path, out_bin: Path) -> dict:
    body = json.loads(body_gltf.read_text(encoding="utf-8"))
    head = json.loads(head_gltf.read_text(encoding="utf-8"))
    body_joints = [body["nodes"][j]["name"] for j in body["skins"][0]["joints"]]
    head_joints = [head["nodes"][j]["name"] for j in head["skins"][0]["joints"]]
    if sorted(body_joints) != sorted(head_joints):
        raise RuntimeError("head and body skeletons carry different joints; a re-skin would be needed (not done)")
    body_bytes = (body_gltf.parent / body["buffers"][0]["uri"]).read_bytes()
    head_bytes = bytearray((head_gltf.parent / head["buffers"][0]["uri"]).read_bytes())

    def inverse_binds(document, blob):
        accessor = document["accessors"][document["skins"][0]["inverseBindMatrices"]]
        view = document["bufferViews"][accessor["bufferView"]]
        start = view.get("byteOffset", 0) + accessor.get("byteOffset", 0)
        return [struct.unpack_from("<16f", blob, start + i * 64) for i in range(accessor["count"])]

    body_ibm = dict(zip(body_joints, inverse_binds(body, body_bytes)))
    head_ibm = dict(zip(head_joints, inverse_binds(head, head_bytes)))
    # Only the joints the head vertices actually name must share the body's bind pose (facial
    # expression bones may differ between the two skeletons without affecting this mesh).
    used = set()
    for primitive in head["meshes"][0]["primitives"]:
        accessor = head["accessors"][primitive["attributes"]["JOINTS_0"]]
        view = head["bufferViews"][accessor["bufferView"]]
        start = view.get("byteOffset", 0) + accessor.get("byteOffset", 0)
        kind = {5121: ("B", 1), 5123: ("H", 2)}[accessor["componentType"]]
        stride = view.get("byteStride", 4 * kind[1])
        for vertex in range(accessor["count"]):
            used.update(head_joints[i] for i in struct.unpack_from("<4" + kind[0], head_bytes, start + vertex * stride))
    worst = max(abs(a - b) for name in used for a, b in zip(body_ibm[name], head_ibm[name]))
    if worst > 1e-2:  # 1 cm in metre units; the delta is reported so a larger one is never hidden
        raise RuntimeError(f"head and body bind poses differ on the joints the head uses (delta {worst}); not merging")
    # Head vertices name their joints by the head skin's order: rewrite them into the body's order.
    remap = [body_joints.index(name) for name in head_joints]
    if remap != list(range(len(remap))):
        for primitive in head["meshes"][0]["primitives"]:
            accessor = head["accessors"][primitive["attributes"]["JOINTS_0"]]
            view = head["bufferViews"][accessor["bufferView"]]
            start = view.get("byteOffset", 0) + accessor.get("byteOffset", 0)
            kind = {5121: ("B", 1), 5123: ("H", 2)}[accessor["componentType"]]
            stride = view.get("byteStride", 4 * kind[1])
            for vertex in range(accessor["count"]):
                offset = start + vertex * stride
                values = struct.unpack_from("<4" + kind[0], head_bytes, offset)
                struct.pack_into("<4" + kind[0], head_bytes, offset, *[remap[v] for v in values])
    payload = bytearray(body_bytes)
    align4(payload)
    base = len(payload)
    payload += head_bytes
    view_base, accessor_base, material_base = len(body["bufferViews"]), len(body["accessors"]), len(body["materials"])
    for view in head["bufferViews"]:
        view = copy.deepcopy(view)
        view["buffer"] = 0
        view["byteOffset"] = view.get("byteOffset", 0) + base
        body["bufferViews"].append(view)
    for accessor in head["accessors"]:
        accessor = copy.deepcopy(accessor)
        if "bufferView" in accessor:
            accessor["bufferView"] += view_base
        body["accessors"].append(accessor)
    for material in head["materials"]:
        body["materials"].append({"name": material.get("name", "head")})
    added = []
    for primitive in head["meshes"][0]["primitives"]:
        primitive = copy.deepcopy(primitive)
        primitive["attributes"] = {k: v + accessor_base for k, v in primitive["attributes"].items()}
        if "indices" in primitive:
            primitive["indices"] += accessor_base
        primitive["material"] = primitive.get("material", 0) + material_base
        body["meshes"][0]["primitives"].append(primitive)
        added.append(body["materials"][primitive["material"]]["name"])
    body["buffers"][0]["uri"] = out_bin.name
    body["buffers"][0]["byteLength"] = len(payload)
    out_bin.write_bytes(payload)
    out_gltf.write_text(json.dumps(body, ensure_ascii=False, indent=1), encoding="utf-8")
    return {"joints": len(body_joints), "headBindDeltaM": round(worst, 5), "headMaterials": added,
            "bodyMaterials": [m.get("name") for m in body["materials"][:material_base]]}


def select_psa_clips(psa: Path, names: list[str], out: Path) -> Path:
    """A PSA holding only the named clips, frames re-packed back to back."""
    data = psa.read_bytes()
    chunks = cs.read_psa_chunks(data)
    info, keys, bones = chunks["ANIMINFO"], chunks["ANIMKEYS"], chunks["BONENAMES"]
    bone_count = bones["count"]
    kept, frames = [], bytearray()
    frame_size = keys["size"] * bone_count
    first = 0
    for i in range(info["count"]):
        record = bytearray(data[info["offset"] + i * info["size"]: info["offset"] + (i + 1) * info["size"]])
        name = record[:64].split(b"\0")[0].decode("latin1")
        if name not in names:
            continue
        old_first, count = struct.unpack_from("<i", record, 160)[0], struct.unpack_from("<i", record, 164)[0]
        start = keys["offset"] + old_first * frame_size
        frames += data[start:start + count * frame_size]
        struct.pack_into("<i", record, 160, first)
        first += count
        kept.append(bytes(record))
    missing = [n for n in names if n not in [r[:64].split(b"\0")[0].decode("latin1") for r in kept]]
    if missing:
        raise RuntimeError(f"clips not in the PSA: {missing}")

    def header(chunk, count):
        return chunk["raw_name"] + struct.pack("<iii", chunk["flag"], chunk["size"], count)

    out_data = bytearray()
    for name, chunk in chunks.items():
        if name == "ANIMINFO":
            out_data += header(chunk, len(kept)) + b"".join(kept)
        elif name == "ANIMKEYS":
            out_data += header(chunk, first * bone_count) + bytes(frames)
        elif name == "SCALEKEYS":
            continue
        else:
            out_data += header(chunk, chunk["count"]) + data[chunk["offset"]:chunk["offset"] + chunk["size"] * chunk["count"]]
    out.write_bytes(bytes(out_data))
    return out


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--asset", required=True)
    parser.add_argument("--body", type=Path, required=True)
    parser.add_argument("--head", type=Path, required=True)
    parser.add_argument("--psa", type=Path, required=True)
    parser.add_argument("--clips", required=True)
    parser.add_argument("--raw", type=Path, required=True, help="export root holding the MIC props and textures")
    parser.add_argument("--work", type=Path, default=Path("C:/LostArkExtract/Ship20260925"))
    parser.add_argument("--allow-bone-order-remap", action="store_true",
                        help="PSA tracks are the skin joints in another order (same set)")
    parser.add_argument("--install", action="store_true")
    args = parser.parse_args()

    target = args.work / "cook" / args.asset
    if target.exists():
        shutil.rmtree(target)
    (target / "src").mkdir(parents=True)
    merged = merge_head_into_body(args.body, args.head, target / "src" / "merged.gltf", target / "src" / "merged.bin")
    clips = [c for c in args.clips.split(",") if c]
    psa = select_psa_clips(args.psa, clips, target / "src" / "clips.psa")
    document = json.loads((target / "src" / "merged.gltf").read_text(encoding="utf-8"))
    joints = [document["nodes"][j]["name"] for j in document["skins"][0]["joints"]]
    matched = psa if args.allow_bone_order_remap else cs.match_psa_to_joints(psa, joints, target / "src" / "matched.psa")
    extra = ["--allow-bone-order-remap"] if args.allow_bone_order_remap else []
    cs.run([sys.executable, cs.STAGER, "--gltf", target / "src" / "merged.gltf", "--psa", matched,
            "--output-gltf", target / "staged.gltf", "--output-bin", target / "staged.bin",
            "--report", target / "stage.json", "--scale", "100"] + extra, cs.REPO, "stage")
    converter_args = ["staged.gltf", "-o", f"{args.asset}.wmodel"]
    for material in sorted({m for m in merged["bodyMaterials"] + merged["headMaterials"]}):
        parameters = cs.parse_texture_parameters(cs.find_props(args.raw, material))
        for slot, flag in (("texture_diffuse", "--material-remap"), ("texture_normal", "--normal-remap")):
            if slot in parameters:
                source = cs.find_texture(args.raw, parameters[slot])
                copy_path = target / "src" / source.name
                shutil.copyfile(source, copy_path)
                converter_args += [flag, f"{material}={copy_path}"]
    converter_args.append("--no-auto-textures")
    cs.run([cs.CONVERTER] + converter_args, target, "convert")
    cs.run([sys.executable, cs.RETIMER, "--wmodel", target / f"{args.asset}.wmodel", "--ticks-per-second", "30",
            "--expect-ticks-per-second", "1000"], cs.REPO, "retime")
    analysis = cs.analyse(target / f"{args.asset}.wmodel", cs.psa_clip_frames(matched))
    print("OK", args.asset, "bones", analysis["bones"], "clips", sorted(analysis["clips"]), "bbox", analysis["bboxCm"],
          "materials", merged)
    if args.install:
        ship = {"asset": args.asset}
        print("   installed:", ", ".join(_install(ship, args.work)))


def _install(ship: dict, work: Path) -> list[str]:
    asset = ship["asset"]
    source = work / "cook" / asset
    destination = cs.REPO / "Client" / "Bin" / "Resources" / "Character" / "NPC" / asset
    files = [(source / f"{asset}.wmodel", destination / f"{asset}.wmodel")]
    for texture in sorted((source / "textures").glob("*")):
        files.append((texture, destination / "textures" / texture.name))
    for _, dst in files:
        if dst.exists():
            raise RuntimeError(f"refusing to overwrite {dst}")
    written = []
    for src, dst in files:
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(src, dst)
        written.append(str(dst.relative_to(cs.REPO / "Client" / "Bin" / "Resources")).replace("\\", "/"))
    return written


if __name__ == "__main__":
    main()

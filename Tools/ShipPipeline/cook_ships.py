#!/usr/bin/env python3
"""Cook the retail Voyage ships (EFTable_VoyageShip) into runtime WModels.

Chain (the repository's proven UModel glTF + PSA path, no Blender):

  UModel export (SkeletalMesh glTF + textures, AnimSet PSA)          -- done beforehand
  Tools/ActorXAssetCooker/build_umodel_gltf_psa.py --scale 100        -- glTF + PSA clips
  Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe               -- staged glTF -> .wmodel
  Tools/ActorXAssetCooker/retime_wmodel_ticks.py                      -- 1000 -> 30 ticks/s
  read_wmodel()                                                       -- bones, clips, weights, size

The converter cannot read non-ASCII absolute paths, so the whole work tree lives under an
ASCII root (default C:/LostArkExtract/Ship20260925). Nothing is written into Resources
unless --install is given, and --install never overwrites an existing file.

Usage:
  python Tools/ShipPipeline/cook_ships.py [--work C:/LostArkExtract/Ship20260925]
         [--only Ship_Estoc,Ship_Windship] [--install]
"""

from __future__ import annotations

import argparse
import glob
import hashlib
import json
import math
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / "Tools" / "ModelAssetConverter"))
import verify_dimensionmaster_summon_bind_pose as wmodel_reader  # noqa: E402

CONVERTER = REPO / "Tools" / "ModelAssetConverter" / "Bin" / "ModelAssetConverter.exe"
STAGER = REPO / "Tools" / "ActorXAssetCooker" / "build_umodel_gltf_psa.py"
RETIMER = REPO / "Tools" / "ActorXAssetCooker" / "retime_wmodel_ticks.py"

# id: EFTable_VoyageShip.PrimaryKey. gltf/psa are relative to <work>/raw.
SHIPS = [
    dict(id=8200, asset="Ship_Estoc", gltf="SH_Estoc01/SH_ESTOC01/SkeletalMesh3/sh_estoc01_sk.gltf",
         psa="anim_SH_Estoc01/SH_ESTOC01/AnimSet/sh_estoc01_ani.psa"),
    dict(id=8201, asset="Ship_Whitewind", gltf="SH_WindShip01/SH_WINDSHIP01/SkeletalMesh3/sh_windship01_sk.gltf",
         psa="anim_SH_WindShip01/SH_WINDSHIP01/AnimSet/sh_windship01_ani.psa"),
    dict(id=8202, asset="Ship_Astray", gltf="SH_FastShip_01/SH_FASTSHIP_01/SkeletalMesh3/sh_fastship01_sk_loc_int.gltf",
         psa="anim_SH_FastShip_01/SH_FASTSHIP_01/AnimSet/sh_fastship01_ani.psa"),
    dict(id=8203, asset="Ship_Barkstorm", gltf="SH_Icebreaker_01/SH_ICEBREAKER_01/SkeletalMesh3/sh_icebreaker_01_sk.gltf",
         psa="anim_SH_Icebreaker_01/SH_ICEBREAKER_01/AnimSet/sh_icebreaker_01_ani.psa"),
    dict(id=8204, asset="Ship_Ghost", gltf="SH_GhostShip_01/SH_GHOSTSHIP_01/SkeletalMesh3/sh_ghostship01_sk.gltf",
         psa="anim_SH_GhostShip_01/SH_GHOSTSHIP_01/AnimSet/sh_ghostship01_ani.psa"),
    dict(id=8205, asset="Ship_Brahms", gltf="SH_Brahms_01/SH_BRAHMS_01/SkeletalMesh3/sh_brahms01_sk.gltf",
         psa="anim_SH_Brahms_01/SH_BRAHMS_01/AnimSet/sh_brahms01_ani.psa"),
    dict(id=8206, asset="Ship_Tragon", gltf="SH_Tragon_01/SH_TRAGON_01/SkeletalMesh3/sh_tragon01_sk.gltf",
         psa="anim_SH_Tragon_01/SH_TRAGON_01/AnimSet/sh_tragon01_ani.psa"),
    dict(id=8207, asset="Ship_Pneuma", gltf="SH_FastShip_02/SH_FASTSHIP_02/SkeletalMesh3/sh_fastship02_sk_loc_int.gltf",
         psa="anim_SH_FastShip_01/SH_FASTSHIP_01/AnimSet/sh_fastship01_ani.psa"),
    dict(id=8208, asset="Ship_Luminous", gltf="SH_MagicShip01/SH_MAGICSHIP01/SkeletalMesh3/sh_magicship01_sk.gltf",
         psa="anim_SH_MagicShip01/SH_MAGICSHIP01/AnimSet/sh_magicship01_ani.psa"),
]


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def run(args, cwd: Path, label: str) -> str:
    result = subprocess.run([str(a) for a in args], cwd=str(cwd), capture_output=True, text=True,
                            encoding="utf-8", errors="replace")
    if result.returncode != 0:
        raise RuntimeError(f"{label} failed ({result.returncode}): {result.stdout[-600:]} {result.stderr[-600:]}")
    return result.stdout


def find_props(raw: Path, material: str) -> Path:
    hits = glob.glob(str(raw / "*" / "**" / "MaterialInstanceConstant" / (material + ".props.txt")), recursive=True)
    if not hits:
        raise RuntimeError(f"material {material} has no exported props")
    return Path(hits[0])


def parse_texture_parameters(props: Path) -> dict:
    """ParameterValue = Texture2D'tex.NAME' is written on the line before ParameterName."""
    found, pending = {}, None
    for line in props.read_text(encoding="utf-8", errors="replace").splitlines():
        value = re.search(r"ParameterValue = Texture2D'([^']+)'", line)
        if value:
            pending = value.group(1)
            continue
        name = re.search(r"ParameterName = (\S+)", line)
        if name and pending:
            found[name.group(1)] = pending
        pending = None
    return found


def find_texture(raw: Path, texture: str) -> Path:
    base = texture.split(".")[-1]
    for extension in ("dds", "tga", "png"):
        hits = glob.glob(str(raw / "*" / "**" / "Texture2D" / f"{base}.{extension}"), recursive=True)
        if hits:
            return Path(hits[0])
    raise RuntimeError(f"texture {texture} was not exported")


def read_psa_chunks(data: bytes) -> dict:
    import struct
    position, chunks = 0, {}
    while position + 32 <= len(data):
        name = data[position:position + 20].split(b"\0")[0].decode("ascii", "ignore")
        flag, size, count = struct.unpack_from("<iii", data, position + 20)
        if size < 0 or count < 0:
            break
        chunks[name] = dict(flag=flag, size=size, count=count, offset=position + 32,
                            raw_name=data[position:position + 20])
        position += 32 + size * count
    return chunks


def match_psa_to_joints(psa: Path, joints: list[str], out: Path) -> Path:
    """Return a PSA whose bone list equals the glTF skin joints, in the same order.

    Some ships (Pneuma) share another ship's AnimSet, which carries extra trailing bones.
    When the glTF joints are an order-preserving subset of the PSA bones the extra tracks are
    dropped; any other mismatch is an error, never a guess.
    """
    import struct
    data = psa.read_bytes()
    chunks = read_psa_chunks(data)
    bones = chunks["BONENAMES"]
    names = [data[bones["offset"] + i * bones["size"]: bones["offset"] + i * bones["size"] + 64]
             .split(b"\0")[0].decode("latin1") for i in range(bones["count"])]
    if names == joints:
        return psa
    if not set(joints) <= set(names) or [n for n in names if n in set(joints)] != joints:
        raise RuntimeError("PSA bones and glTF joints differ in more than trailing extra bones")
    keep = [names.index(n) for n in joints]
    old_total = len(names)
    new_records = []
    for new_index, old_index in enumerate(keep):
        record = bytearray(data[bones["offset"] + old_index * bones["size"]:
                                bones["offset"] + (old_index + 1) * bones["size"]])
        parent = struct.unpack_from("<i", record, 72)[0]
        remapped = keep.index(parent) if parent in keep else (-1 if new_index == 0 else 0)
        struct.pack_into("<i", record, 72, remapped)
        children = sum(1 for other in keep if other != old_index and
                       struct.unpack_from("<i", data, bones["offset"] + other * bones["size"] + 72)[0] == old_index)
        struct.pack_into("<i", record, 68, children)
        new_records.append(bytes(record))

    def header(chunk, count):
        return chunk["raw_name"] + struct.pack("<iii", chunk["flag"], chunk["size"], count)

    info = chunks["ANIMINFO"]
    infos = []
    for i in range(info["count"]):
        record = bytearray(data[info["offset"] + i * info["size"]: info["offset"] + (i + 1) * info["size"]])
        if struct.unpack_from("<i", record, 128)[0] != old_total:
            raise RuntimeError("PSA sequence TotalBones does not match its BONENAMES count")
        struct.pack_into("<i", record, 128, len(keep))
        infos.append(bytes(record))
    keys = chunks["ANIMKEYS"]
    if keys["count"] % old_total:
        raise RuntimeError("PSA key count is not a multiple of the bone count")
    frames = keys["count"] // old_total
    key_bytes = bytearray()
    for frame in range(frames):
        for old_index in keep:
            start = keys["offset"] + (frame * old_total + old_index) * keys["size"]
            key_bytes += data[start:start + keys["size"]]
    out_data = bytearray()
    for name, chunk in chunks.items():
        if name == "BONENAMES":
            out_data += header(chunk, len(keep)) + b"".join(new_records)
        elif name == "ANIMINFO":
            out_data += header(chunk, len(infos)) + b"".join(infos)
        elif name == "ANIMKEYS":
            out_data += header(chunk, frames * len(keep)) + bytes(key_bytes)
        elif name == "SCALEKEYS":
            scale = bytearray()
            for frame in range(frames):
                for old_index in keep:
                    start = chunk["offset"] + (frame * old_total + old_index) * chunk["size"]
                    scale += data[start:start + chunk["size"]]
            out_data += header(chunk, frames * len(keep)) + bytes(scale)
        else:
            out_data += header(chunk, chunk["count"]) + data[chunk["offset"]:chunk["offset"] + chunk["size"] * chunk["count"]]
    out.write_bytes(bytes(out_data))
    return out


def analyse(wmodel: Path, expected_clips: list[tuple[str, int]]) -> dict:
    model = wmodel_reader.read_wmodel(wmodel)
    model = {"vertices": model.vertices, "skeleton_bones": model.skeleton_bones, "animations": model.animations}
    vertices = model["vertices"]
    positions = [v.position for v in vertices]
    xs, ys, zs = zip(*positions)
    bad_weights = sum(1 for v in vertices if not all(math.isfinite(w) for w in v.weights)
                      or abs(sum(v.weights) - 1.0) > 0.02)
    animations = {a.name: (a.duration_ticks, a.ticks_per_second) for a in model["animations"]}
    for name, frames in expected_clips:
        if name not in animations:
            raise RuntimeError(f"clip {name} is missing from the WModel")
        ticks, rate = animations[name]
        if abs(rate - 30.0) > 0.001 or abs(ticks - (frames - 1)) > 0.5:
            raise RuntimeError(f"clip {name} has {ticks} ticks @ {rate}, expected {frames - 1} @ 30")
    if bad_weights:
        raise RuntimeError(f"{bad_weights} vertices have non-normalised skin weights")
    # deck height estimate: the most populated height band among hull vertices near the centre line.
    span = max(zs) - min(zs)
    centre = [p for p in positions if abs(p[2]) < span * 0.25 and 30.0 <= p[1] <= 200.0]
    bands = {}
    for p in centre:
        bands[round(p[1] / 5.0) * 5] = bands.get(round(p[1] / 5.0) * 5, 0) + 1
    deck = max(bands.items(), key=lambda kv: kv[1])[0] if bands else None
    return {
        "vertices": len(vertices),
        "bboxCm": {"x": [round(min(xs)), round(max(xs))], "y": [round(min(ys)), round(max(ys))],
                   "z": [round(min(zs)), round(max(zs))]},
        "bones": len(model["skeleton_bones"]),
        "boneNames": [b.name for b in model["skeleton_bones"]],
        "clips": {n: {"ticks": t, "tps": r, "seconds": round(t / r, 4)} for n, (t, r) in animations.items()},
        "deckHeightCmEstimate": deck,
    }


def psa_clip_frames(psa: Path) -> list[tuple[str, int]]:
    data = psa.read_bytes()
    position, chunks = 0, {}
    import struct
    while position + 32 <= len(data):
        name = data[position:position + 20].split(b"\0")[0].decode("ascii", "ignore")
        _flag, size, count = struct.unpack_from("<iii", data, position + 20)
        if size < 0 or count < 0:
            break
        chunks[name] = (position + 32, size, count)
        position += 32 + size * count
    offset, size, count = chunks["ANIMINFO"]
    clips = []
    for index in range(count):
        record = data[offset + index * size: offset + (index + 1) * size]
        clips.append((record[:64].split(b"\0")[0].decode("latin1"), struct.unpack_from("<i", record, 164)[0]))
    return clips


def cook(ship: dict, work: Path) -> dict:
    raw = work / "raw"
    asset = ship["asset"]
    target = work / "cook" / asset
    if target.exists():
        shutil.rmtree(target)
    (target / "src").mkdir(parents=True)
    gltf = raw / ship["gltf"]
    psa = raw / ship["psa"]
    if not gltf.is_file() or not psa.is_file():
        raise RuntimeError(f"{asset}: missing export {gltf} / {psa}")
    document = json.loads(gltf.read_text(encoding="utf-8"))
    materials = sorted({m.get("name") for m in document.get("materials", []) if m.get("name")})
    joints = [document["nodes"][j]["name"] for j in document["skins"][0]["joints"]]
    matched = match_psa_to_joints(psa, joints, target / "src" / "matched.psa")
    stage_out = run([sys.executable, STAGER, "--gltf", gltf, "--psa", matched,
                     "--output-gltf", target / "staged.gltf", "--output-bin", target / "staged.bin",
                     "--report", target / "stage.json", "--scale", "100"], REPO, f"{asset} stage")
    converter_args = ["staged.gltf", "-o", f"{asset}.wmodel"]
    textures = {}
    for material in materials:
        parameters = parse_texture_parameters(find_props(raw, material))
        for slot, flag in (("texture_diffuse", "--material-remap"), ("texture_normal", "--normal-remap")):
            if slot not in parameters:
                continue
            source = find_texture(raw, parameters[slot])
            copy = target / "src" / source.name
            shutil.copyfile(source, copy)
            converter_args += [flag, f"{material}={copy}"]
            textures[f"{material}|{slot}"] = source.name
    converter_args.append("--no-auto-textures")
    run([CONVERTER] + converter_args, target, f"{asset} convert")
    run([sys.executable, RETIMER, "--wmodel", target / f"{asset}.wmodel", "--ticks-per-second", "30",
         "--expect-ticks-per-second", "1000"], REPO, f"{asset} retime")
    info = analyse(target / f"{asset}.wmodel", psa_clip_frames(matched))
    receipt = {
        "schema": "lostark.ship-cook", "formatVersion": 1, "assetId": asset, "vehicleId": ship["id"],
        "source": {"gltf": ship["gltf"], "gltfSha256": sha256(gltf), "psa": ship["psa"], "psaSha256": sha256(psa)},
        "textures": textures, "wmodel": f"{asset}.wmodel", "wmodelSha256": sha256(target / f"{asset}.wmodel"),
        "stageOutput": stage_out.strip(), "analysis": info,
    }
    (target / f"{asset}.cook.json").write_text(json.dumps(receipt, indent=1, ensure_ascii=False), encoding="utf-8")
    return receipt


def install(ship: dict, work: Path) -> list[str]:
    asset = ship["asset"]
    source = work / "cook" / asset
    destination = REPO / "Client" / "Bin" / "Resources" / "Character" / "Vehicle" / asset
    files = [(source / f"{asset}.wmodel", destination / f"{asset}.wmodel")]
    for texture in sorted((source / "textures").glob("*")):
        files.append((texture, destination / "textures" / texture.name))
    written = []
    for src, dst in files:
        if dst.exists():
            raise RuntimeError(f"refusing to overwrite {dst}")
    for src, dst in files:
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(src, dst)
        written.append(str(dst.relative_to(REPO / "Client" / "Bin" / "Resources")).replace(os.sep, "/"))
    return written


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--work", type=Path, default=Path("C:/LostArkExtract/Ship20260925"))
    parser.add_argument("--only", default="")
    parser.add_argument("--install", action="store_true")
    args = parser.parse_args()
    only = {name for name in args.only.split(",") if name}
    failures = []
    for ship in SHIPS:
        if only and ship["asset"] not in only:
            continue
        try:
            receipt = cook(ship, args.work)
            analysis = receipt["analysis"]
            print(f"OK {ship['asset']} id={ship['id']} bones={analysis['bones']} clips={sorted(analysis['clips'])} "
                  f"bbox={analysis['bboxCm']} deck~{analysis['deckHeightCmEstimate']}")
            if args.install:
                print("   installed:", ", ".join(install(ship, args.work)))
        except Exception as error:  # report every ship, do not stop at the first failure
            failures.append(ship["asset"])
            print(f"FAIL {ship['asset']}: {error}")
    if failures:
        sys.exit(1)


if __name__ == "__main__":
    main()

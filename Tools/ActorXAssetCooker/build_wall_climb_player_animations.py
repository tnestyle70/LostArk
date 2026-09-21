"""Cook the two native Valtan TrackMove climb clips for every playable class.

The source PSA stays outside the repository.  This tool only emits the small,
two-clip WModel that is registered by CharacterCatalog.json; it never rewrites
the class body or its skeleton section.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/ModelAssetConverter"))
import append_psa_clip_to_wmodel as codec
from build_card_maze_player_animations import carrier

SOURCES = {
    "LanceMaster": "PC_FT_00", "GunSlinger": "PC_GN_F_00",
    "Slayer": "PC_WR_F_00", "Artist": "PC_SP_00",
    "DimensionMaster": "PC_SP_M_00", "Warlord": "PC_WR_00",
}
CLIPS = {"wall_climb_loop": "act_creep_up_1",
         "wall_climb_end": "act_creep_up_end_1"}


def sections(data):
    model = list(codec.MODEL_HEADER.unpack_from(data, 16))
    return model, [codec.SECTION_DESC.unpack_from(data, 48 + i * 64)
                   for i in range(model[1])]


def only_skeleton_and_clips(data):
    model, rows = sections(data)
    selected = [row for row in rows if row[0] in (1, 2, 3) or
                row[0] == 4 and row[4].split(b"\0")[0].decode() in CLIPS]
    model[1] = len(selected)
    model[2] = sum(row[0] == 4 for row in selected)
    result = bytearray(data[:16]) + codec.MODEL_HEADER.pack(*model)
    offset = codec.MODEL_HEADER.size + len(selected) * codec.SECTION_DESC.size
    bodies = bytearray()
    animation_index = 0
    for kind, index, source_offset, size, name in selected:
        if kind == 4:
            index = animation_index
            animation_index += 1
        result += codec.SECTION_DESC.pack(kind, index, offset, size, name)
        bodies += data[16 + source_offset:16 + source_offset + size]
        offset += size
    result += bodies
    struct.pack_into("<I", result, 12, len(result) - 16)
    return bytes(result)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True,
                        help="ActorX root containing the five non-Warlord classes")
    parser.add_argument("--warlord-source", type=Path, required=True,
                        help="ActorX root containing PC_WR_00")
    parser.add_argument("--resources", type=Path, required=True,
                        help="Client/Bin/Resources root")
    parser.add_argument("--receipt", type=Path, required=True)
    args = parser.parse_args()
    catalog = json.loads((ROOT / "Data/Actors/CharacterCatalog.json").read_text(
        encoding="utf-8"))
    receipt = []
    with tempfile.TemporaryDirectory(prefix="lostark-wall-climb-") as directory:
        scratch = Path(directory)
        for actor in catalog["characters"]:
            name = actor["assetId"]
            if name not in SOURCES:
                continue
            package = SOURCES[name]
            source = args.warlord_source if name == "Warlord" else args.source
            psa = source / package / "AnimSet" / (package.lower() + "_ani.psa")
            if not psa.is_file():
                raise SystemExit("Missing base AnimSet export: %s" % psa)
            body = args.resources / actor["bodyModel"]
            body_data = body.read_bytes()
            _, body_rows = sections(body_data)
            skeleton = next(row for row in body_rows if row[0] == 3)
            donor = next(row for row in body_rows if row[0] == 4)
            staged = scratch / (name + "_donor.wmodel")
            staged.write_bytes(carrier(body_data, skeleton, donor))
            for cooked_name, native_name in CLIPS.items():
                bones, info, _ = codec.load_clip(psa, native_name)
                if len(set(bones)) != len(bones) or info["rate"] != 30:
                    raise ValueError("Invalid native climb clip: " + native_name)
                destination = scratch / (name + "_" + cooked_name + ".wmodel")
                subprocess.run([sys.executable, str(Path(codec.__file__)),
                                "--wmodel", str(staged), "--psa", str(psa),
                                "--clip", native_name, "--name", cooked_name,
                                "--out", str(destination)], check=True)
                staged = destination
            final_data = only_skeleton_and_clips(staged.read_bytes())
            output = args.resources / "Character" / name / "AnimSets" / \
                (name + "_WallClimbAnimSet.wmodel")
            output.parent.mkdir(parents=True, exist_ok=True)
            temporary = output.with_suffix(".wmodel.tmp")
            temporary.write_bytes(final_data)
            temporary.replace(output)
            _, rows = sections(final_data)
            final_skeleton = next(row for row in rows if row[0] == 3)
            if final_data[16 + final_skeleton[2]:16 + final_skeleton[2] + final_skeleton[3]] != \
                    body_data[16 + skeleton[2]:16 + skeleton[2] + skeleton[3]]:
                raise ValueError("Cook changed body skeleton: " + name)
            clip_names = [row[4].split(b"\0")[0].decode() for row in rows if row[0] == 4]
            if sorted(clip_names) != sorted(CLIPS):
                raise ValueError("Cooked clip set is incomplete: " + name)
            receipt.append({"class": name, "sourcePsa": str(psa),
                            "sourceSha256": hashlib.sha256(psa.read_bytes()).hexdigest(),
                            "output": str(output),
                            "outputSha256": hashlib.sha256(final_data).hexdigest(),
                            "clipNames": clip_names, "skeletonPreserved": True})
    args.receipt.parent.mkdir(parents=True, exist_ok=True)
    args.receipt.write_text(json.dumps(receipt, indent=2), encoding="utf-8")
    for row in receipt:
        print("%s -> %s" % (row["class"], row["output"]))


if __name__ == "__main__":
    main()

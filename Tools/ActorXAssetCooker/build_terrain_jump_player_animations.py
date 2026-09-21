"""Stage the native terrain-crossing jump clips against each installed WModel skeleton.

Same recipe as build_card_maze_player_animations.py: the source PSA files are
UModel exports of each class's own base AnimSet, the installed skeleton and
donor armature scale are kept byte-for-byte, and body models are never
rewritten. --classes cooks a subset, so a class whose base AnimSet has not been
exported yet can be added later without redoing the others.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/ModelAssetConverter"))
import append_psa_clip_to_wmodel as codec

SOURCES = {
    "LanceMaster": "PC_FT_00", "GunSlinger": "PC_GN_F_00",
    "Slayer": "PC_WR_F_00", "Artist": "PC_SP_00",
    "DimensionMaster": "PC_SP_M_00", "Warlord": "PC_WR_00",
    "GuardianKnight": "PC_DL_00",
}
CLIPS = {"terrain_jump_short": "act_jump_s_1",
         "terrain_jump_medium": "act_jump_m_1"}
SUFFIX = "_TerrainJumpAnimSet.wmodel"


def sections(data):
    model = list(codec.MODEL_HEADER.unpack_from(data, 16))
    return model, [codec.SECTION_DESC.unpack_from(data, 48 + i * 64)
                   for i in range(model[1])]


def subset(data, selected):
    model, _ = sections(data)
    model[1] = len(selected)
    model[2] = sum(s[0] == 4 for s in selected)
    result = bytearray(data[:16]) + codec.MODEL_HEADER.pack(*model)
    offset = codec.MODEL_HEADER.size + len(selected) * codec.SECTION_DESC.size
    bodies = bytearray()
    animation_index = 0
    for section in selected:
        kind, index, source_offset, size, name = section
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
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--classes", nargs="*", default=[])
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    catalog = json.loads(
        (ROOT / "Data/Actors/CharacterCatalog.json").read_text(encoding="utf-8"))
    wanted = set(args.classes) if args.classes else set(SOURCES)
    unknown = wanted - set(SOURCES)
    if unknown:
        raise SystemExit("Unknown class: %s" % ", ".join(sorted(unknown)))
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    from build_card_maze_player_animations import carrier

    receipt = []
    for actor in catalog["characters"]:
        name = actor["assetId"]
        if name not in wanted:
            continue
        package = SOURCES[name]
        psa = args.source / package / "AnimSet" / (package.lower() + "_ani.psa")
        if not psa.is_file():
            raise SystemExit("Missing base AnimSet export: %s" % psa)
        body = ROOT / "Client/Bin/Resources" / actor["bodyModel"]
        body_data = body.read_bytes()
        _, rows = sections(body_data)
        skeleton = next(s for s in rows if s[0] == 3)
        donor = next(s for s in rows if s[0] == 4)
        staged = args.out / (name + "_donor.wmodel")
        staged.write_bytes(carrier(body_data, skeleton, donor))
        for output_name, native_name in CLIPS.items():
            bones, info, keys = codec.load_clip(psa, native_name)
            if len(set(bones)) != len(bones) or info["rate"] != 30:
                raise ValueError("Invalid native clip skeleton or rate: " + native_name)
            destination = args.out / (name + "_" + output_name + ".wmodel")
            subprocess.run([sys.executable, str(Path(codec.__file__)),
                            "--wmodel", str(staged), "--psa", str(psa),
                            "--clip", native_name, "--name", output_name,
                            "--out", str(destination)], check=True)
            staged = destination
        data = staged.read_bytes()
        _, rows = sections(data)
        selected = [s for s in rows if s[0] in (1, 2, 3) or
                    s[0] == 4 and s[4].split(b"\0")[0].decode() in CLIPS]
        final = args.out / (name + SUFFIX)
        final.write_bytes(subset(data, selected))
        # The complete source skeleton, including its hash, is unchanged.
        actual = final.read_bytes()
        _, result_rows = sections(actual)
        target_skeleton = next(s for s in result_rows if s[0] == 3)
        assert actual[16 + target_skeleton[2]:16 + target_skeleton[2] + target_skeleton[3]] == \
            body_data[16 + skeleton[2]:16 + skeleton[2] + skeleton[3]]
        receipt.append({"class": name, "sourcePsa": str(psa),
                        "sourceSha256": hashlib.sha256(psa.read_bytes()).hexdigest(),
                        "bodySha256": hashlib.sha256(body_data).hexdigest(),
                        "output": str(final),
                        "outputSha256": hashlib.sha256(actual).hexdigest(),
                        "clipNames": list(CLIPS), "skeletonPreserved": True})
    receipt_path = args.out / "receipt.json"
    existing = json.loads(receipt_path.read_text(encoding="utf-8")) \
        if receipt_path.is_file() else []
    merged = [row for row in existing
              if row["class"] not in {r["class"] for r in receipt}] + receipt
    merged.sort(key=lambda row: row["class"])
    receipt_path.write_text(json.dumps(merged, indent=2), encoding="utf-8")
    for row in receipt:
        print("%s -> %s" % (row["class"], row["output"]))


if __name__ == "__main__":
    main()

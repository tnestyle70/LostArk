"""Stage class-native card-maze clips against each installed WModel skeleton.

The source PSA files are UModel exports of each class's own base AnimSet. The
installed skeleton and donor armature scale are kept byte-for-byte; body models
are never rewritten. Installation is a separate hash-checked caller operation.
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
}
CLIPS = {"maze_hammer_lmb": "pr_it_gstfp_00_att_1_01",
         "maze_hammer_q": "pr_it_gstfp_00_att_2_01"}


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


def carrier(body_data, skeleton, donor):
    """Use the existing animation-set carrier triangle, with this body's palette.

    CModel requires mesh/material sections even for an animation donor. These
    carrier sections are never attached to the player's rendered body.
    """
    template = (ROOT / "Client/Bin/Resources/Character/LanceMaster/AnimSets/LanceMaster_EstherAnimSet.wmodel").read_bytes()
    _, template_rows = sections(template)
    _, body_rows = sections(body_data)
    mesh_desc = next(s for s in template_rows if s[0] == 1)
    material_desc = next(s for s in template_rows if s[0] == 2)
    source_mesh = next(s for s in body_rows if s[0] == 1)
    mesh = bytearray(template[16 + mesh_desc[2]:16 + mesh_desc[2] + mesh_desc[3]])
    old_submeshes, old_bones = struct.unpack_from("<II", mesh, 20)
    source_at = 32 + source_mesh[2]
    submeshes, bones = struct.unpack_from("<II", body_data, source_at + 4)
    source_stride, source_vertices, source_indices, source_index_stride = struct.unpack_from("<4I", body_data, source_at + 16)
    target_stride, target_vertices, target_indices, target_index_stride = struct.unpack_from("<4I", mesh, 32)
    source_palette_at = source_at + 36 + submeshes * 48 + source_stride * source_vertices + source_index_stride * source_indices
    target_palette_at = 16 + 36 + old_submeshes * 48 + target_stride * target_vertices + target_index_stride * target_indices
    mesh[target_palette_at:target_palette_at + old_bones * 128] = body_data[source_palette_at:source_palette_at + bones * 128]
    struct.pack_into("<I", mesh, 24, bones)
    struct.pack_into("<I", mesh, 12, len(mesh) - 16)
    payloads = [bytes(mesh), template[16 + material_desc[2]:16 + material_desc[2] + material_desc[3]],
                body_data[16 + skeleton[2]:16 + skeleton[2] + skeleton[3]],
                body_data[16 + donor[2]:16 + donor[2] + donor[3]]]
    result = bytearray(template[:48])
    offset = 32 + 4 * 64
    for index, (row, payload) in enumerate(zip([mesh_desc, material_desc, skeleton, donor], payloads)):
        result += codec.SECTION_DESC.pack(index + 1, 0, offset, len(payload), row[4])
        offset += len(payload)
    result += b"".join(payloads)
    struct.pack_into("<I", result, 12, len(result) - 16)
    return bytes(result)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    catalog = json.loads((ROOT / "Data/Actors/CharacterCatalog.json").read_text(encoding="utf-8"))
    receipt = []
    for actor in catalog["characters"]:
        name = actor["assetId"]
        package = SOURCES[name]
        psa = args.source / package / "AnimSet" / (package.lower() + "_ani.psa")
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
                raise ValueError("Invalid native clip skeleton or rate")
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
        final = args.out / (name + "_MazeHammerAnimSet.wmodel")
        final.write_bytes(subset(data, selected))
        # The complete source skeleton, including its hash, is unchanged.
        actual = final.read_bytes()
        _, result_rows = sections(actual)
        target_skeleton = next(s for s in result_rows if s[0] == 3)
        assert actual[16 + target_skeleton[2]:16 + target_skeleton[2] + target_skeleton[3]] == body_data[16 + skeleton[2]:16 + skeleton[2] + skeleton[3]]
        receipt.append({"class": name, "sourcePsa": str(psa),
                        "sourceSha256": hashlib.sha256(psa.read_bytes()).hexdigest(),
                        "bodySha256": hashlib.sha256(body_data).hexdigest(),
                        "output": str(final), "outputSha256": hashlib.sha256(actual).hexdigest(),
                        "clipNames": list(CLIPS), "skeletonPreserved": True})
    (args.out / "receipt.json").write_text(json.dumps(receipt, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()

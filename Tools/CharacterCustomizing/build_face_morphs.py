"""Cook the four playable classes' face MorphTargets into runtime .facemorphs sidecars.

The character-creation base tab drives the face with two different retail
mechanisms:

  * ``ADD_<slider>_UI``  - additive three-frame face AnimSequences.  Those are
    already cooked by ``build_face_sliders.py`` into
    ``Data/Customizing/FaceSliders/<race>.facesliders.json``.
  * ``MM_<group>_MeshType_<NN>_UI`` - real MorphTargets that live in the class
    face ``.upk`` next to ``pc_<race>_00_face_sk`` and are collected by a single
    ``MorphTargetSet``.  This tool cooks those.

It reads the retail package directly (the LostArk cooker encrypts and LZ4s the
package body; see ``decompress_package``), walks the export table, decodes every
``morphtarget`` export and writes one ``.facemorphs`` binary per class.  The
container is documented in ``FACEMORPH_FORMAT.md`` next to this file.

It also reads the per-preset customizing documents unpacked from ``data3.lpk``
(``XmlData/CharacterCustomizing/EFDLChar_PC_<TAG>.PC_<TAG>_<NN>.loa``) and writes
``Data/UI/Customizing/CustomizingFacePresets.json``, which pairs each preset icon
position with the morph weights that preset drives.

usage:
  python build_face_morphs.py --packages <retail Packages dir>
                              --customizing <dir with the unpacked .loa docs>
                              [--resources <Client/Bin/Resources>] [--data <Data>]
"""
from __future__ import annotations

import argparse
import json
import re
import struct
from pathlib import Path

from Crypto.Cipher import AES
import lz4.block

PACKAGE_KEY = b"V1ZEG1PL34V77SQW39A9I4VUW34T6L15"
COMPRESSED_CHUNK_MAGIC = 0x9E2A83C1
COMPRESSION_FLAG_ENCRYPTED = 0x44
EXPORT_ENTRY_SIZE = 68
IMPORT_ENTRY_SIZE = 28
MORPH_VERTEX_SIZE = 20

FACEMORPH_MAGIC = b"LAFMORPH"
FACEMORPH_VERSION = 1

# class asset id -> (retail face package, retail race tag)
CLASSES = {
    "LanceMaster":     ("VC2NXN2N002NXYCQF8HEH00E", "FT"),
    "Warlord":         ("VC2N892N002NXYCQF8REH00E", "WR"),
    "Artist":          ("VC2NGV2N002NXYCQF81EH00E", "SP"),
    "DimensionMaster": ("WD3OHW3OB3I113IYZDRG1MP6M", "SP_M"),
}

MORPH_NAME = re.compile(r"^MM_[A-Za-z]+_MeshType_\d+_UI$")
PRESET_DOC = re.compile(r"^EFDLCHAR_PC_(?P<tag>.+)\.PC_(?P=tag)_(?P<index>\d+)\.loa$", re.I)


# ---------------------------------------------------------------- package read

def decompress_package(path):
    """Undo the retail cooker's per-chunk AES-256-ECB + raw-LZ4 body compression."""
    data = path.read_bytes()
    at = 4                                    # package tag
    version, licensee = struct.unpack_from("<HH", data, at)
    at += 4
    header_size, = struct.unpack_from("<i", data, at)
    at += 4
    folder_len, = struct.unpack_from("<i", data, at)
    at += 4
    at += -folder_len * 2 if folder_len < 0 else folder_len
    at += 4                                   # package flags
    (name_count, name_offset, export_count, export_offset,
     import_count, import_offset, depends_offset) = struct.unpack_from("<7I", data, at)
    at += 28
    at += 16 + 16                             # export/import guards + package guid
    generation_count, = struct.unpack_from("<I", data, at)
    at += 4
    at += generation_count * 12
    _engine, _cooker, compression_flags, chunk_count = struct.unpack_from("<4I", data, at)
    at += 16

    chunks = []
    for _ in range(chunk_count):
        chunks.append(struct.unpack_from("<5I", data, at))   # FCompressedChunk is 20 bytes here
        at += 20

    total = max(chunk[0] + chunk[1] for chunk in chunks)
    out = bytearray(max(total, header_size))
    out[0:chunks[0][0]] = data[0:chunks[0][0]]
    for uncompressed_offset, uncompressed_size, compressed_offset, compressed_size, _ in chunks:
        magic, = struct.unpack_from("<I", data, compressed_offset)
        if magic != COMPRESSED_CHUNK_MAGIC:
            out[uncompressed_offset:uncompressed_offset + uncompressed_size] = \
                data[compressed_offset:compressed_offset + compressed_size]
            continue
        _magic, block_size, _c, uncompressed_total = struct.unpack_from(
            "<4I", data, compressed_offset)
        block_count = (uncompressed_total + block_size - 1) // block_size
        read_at = compressed_offset + 16 + 8 * block_count
        write_at = uncompressed_offset
        for block in range(block_count):
            block_compressed, block_uncompressed = struct.unpack_from(
                "<II", data, compressed_offset + 16 + 8 * block)
            payload = bytearray(data[read_at:read_at + block_compressed])
            read_at += block_compressed
            if compression_flags == COMPRESSION_FLAG_ENCRYPTED:
                encrypted = min(block_compressed & ~15, 4096)
                if encrypted:
                    payload[0:encrypted] = AES.new(PACKAGE_KEY, AES.MODE_ECB).decrypt(
                        bytes(payload[0:encrypted]))
            out[write_at:write_at + block_uncompressed] = lz4.block.decompress(
                bytes(payload), uncompressed_size=block_uncompressed)
            write_at += block_uncompressed
    return bytes(out), {
        "version": version, "licensee": licensee,
        "nameCount": name_count, "nameOffset": name_offset,
        "exportCount": export_count, "exportOffset": export_offset,
        "importCount": import_count, "importOffset": import_offset,
        "dependsOffset": depends_offset,
    }


def read_names(buffer, info):
    names = []
    at = info["nameOffset"]
    for _ in range(info["nameCount"]):
        length, = struct.unpack_from("<i", buffer, at)
        at += 4
        if length < 0:
            names.append(buffer[at:at - length * 2].decode("utf-16-le").rstrip("\0"))
            at += -length * 2
        else:
            names.append(buffer[at:at + length].decode("cp949", "replace").rstrip("\0"))
            at += length
        at += 8                                # FName flags
    if at != info["importOffset"]:
        raise SystemExit("name table ended at %d, expected %d" % (at, info["importOffset"]))
    return names


def read_exports(buffer, info, names):
    if (info["exportOffset"] - info["importOffset"]) != info["importCount"] * IMPORT_ENTRY_SIZE:
        raise SystemExit("unexpected import table stride")
    imports = []
    at = info["importOffset"]
    for _ in range(info["importCount"]):
        fields = struct.unpack_from("<7i", buffer, at)
        at += IMPORT_ENTRY_SIZE
        imports.append((names[fields[2]], names[fields[5]]))

    span = info["dependsOffset"] - info["exportOffset"]
    if span != info["exportCount"] * EXPORT_ENTRY_SIZE:
        raise SystemExit("unexpected export table stride")
    exports = []
    at = info["exportOffset"]
    for index in range(info["exportCount"]):
        class_index, = struct.unpack_from("<i", buffer, at)
        name_index, name_number = struct.unpack_from("<2i", buffer, at + 12)
        serial_size, serial_offset = struct.unpack_from("<2i", buffer, at + 32)
        component_count, = struct.unpack_from("<i", buffer, at + 40)
        if component_count:
            raise SystemExit("export component map is not empty; stride assumption broken")
        net_object_count, = struct.unpack_from("<i", buffer, at + 48)
        if net_object_count:
            raise SystemExit("export net object array is not empty; stride assumption broken")
        at += EXPORT_ENTRY_SIZE
        name = names[name_index]
        if name_number != 0:
            name = "%s_%d" % (name, name_number - 1)
        if class_index > 0:
            class_name = exports[class_index - 1]["name"]
        elif class_index < 0:
            class_name = imports[-class_index - 1][1]
        else:
            class_name = "Class"
        exports.append({"index": index, "name": name, "className": class_name,
                        "offset": serial_offset, "size": serial_size})
    return exports


def read_morph_target(buffer, export, names):
    """[4B export prefix][8B 'None' terminator][LOD count][vertex count][20B * n][base vertex count]"""
    at = export["offset"] + 4
    terminator_index, terminator_number = struct.unpack_from("<2i", buffer, at)
    if names[terminator_index].lower() != "none" or terminator_number != 0:
        raise SystemExit("%s: expected an empty property list" % export["name"])
    at += 8
    lod_count, = struct.unpack_from("<I", buffer, at)
    at += 4
    if lod_count != 1:
        raise SystemExit("%s: %d LOD models, only 1 is handled" % (export["name"], lod_count))
    vertex_count, = struct.unpack_from("<I", buffer, at)
    at += 4
    vertices = []
    for _ in range(vertex_count):
        dx, dy, dz = struct.unpack_from("<3f", buffer, at)
        packed = buffer[at + 12:at + 16]
        source_index, = struct.unpack_from("<I", buffer, at + 16)
        at += MORPH_VERTEX_SIZE
        vertices.append((source_index, (dx, dy, dz),
                         tuple(byte / 127.5 - 1.0 for byte in packed[:3])))
    base_vertex_count, = struct.unpack_from("<I", buffer, at)
    at += 4
    if at - export["offset"] != export["size"]:
        raise SystemExit("%s: consumed %d of %d bytes"
                         % (export["name"], at - export["offset"], export["size"]))
    return vertices, base_vertex_count


# ------------------------------------------------------------------ preset doc

def read_preset_morphs(path):
    """Return the document's ordered (MorphTarget name, weight) array."""
    data = path.read_bytes()
    anchor = data.find(b"MM_")
    if anchor < 12:
        return []
    count, = struct.unpack_from("<i", data, anchor - 8)
    if not 0 < count <= 512:
        raise SystemExit("%s: implausible morph array count %d" % (path.name, count))
    at = anchor - 4
    entries = []
    for _ in range(count):
        length, = struct.unpack_from("<i", data, at)
        at += 4
        name = data[at:at + length - 1].decode("ascii")
        at += length
        weight, = struct.unpack_from("<f", data, at)
        at += 4
        if not MORPH_NAME.match(name):
            raise SystemExit("%s: unexpected morph entry %r" % (path.name, name))
        entries.append((name, weight))
    return entries


# --------------------------------------------------------------------- writing

def write_facemorphs(path, base_vertex_count, vertex_index_bound, morphs):
    body = bytearray()
    for name, vertices in morphs:
        encoded = name.encode("ascii")
        body += struct.pack("<I", len(encoded))
        body += encoded
        body += b"\0" * (-len(encoded) % 4)
        body += struct.pack("<I", len(vertices))
        for source_index, position, normal in vertices:
            body += struct.pack("<I6f", source_index, position[0], position[1], position[2],
                                normal[0], normal[1], normal[2])
    header = struct.pack("<8s5I", FACEMORPH_MAGIC, FACEMORPH_VERSION, base_vertex_count,
                         vertex_index_bound, len(morphs), sum(len(v) for _, v in morphs))
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(header + bytes(body))


def main():
    repo = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser()
    parser.add_argument("--packages", type=Path, required=True,
                        help="retail EFGame/ReleasePC/Packages directory")
    parser.add_argument("--customizing", type=Path, required=True,
                        help="directory holding the unpacked XmlData/CharacterCustomizing/*.loa")
    parser.add_argument("--resources", type=Path, default=repo / "Client" / "Bin" / "Resources")
    parser.add_argument("--data", type=Path, default=repo / "Data")
    args = parser.parse_args()

    presets_document = {
        "schema": "lostark.customizing-face-presets",
        "formatVersion": 1,
        "classes": {},
    }

    for class_id, (package, tag) in CLASSES.items():
        buffer, info = decompress_package(args.packages / (package + ".upk"))
        names = read_names(buffer, info)
        exports = read_exports(buffer, info, names)

        morph_exports = [e for e in exports if e["className"] == "morphtarget"]
        morphs = []
        base_vertex_counts = set()
        for export in morph_exports:
            vertices, base_vertex_count = read_morph_target(buffer, export, names)
            base_vertex_counts.add(base_vertex_count)
            morphs.append((export["name"], vertices))
        if len(base_vertex_counts) != 1:
            raise SystemExit("%s: morphs disagree on base vertex count %s"
                             % (class_id, base_vertex_counts))
        base_vertex_count = base_vertex_counts.pop()
        vertex_index_bound = 1 + max(source_index
                                     for _name, vertices in morphs
                                     for source_index, _p, _n in vertices)
        for name, vertices in morphs:
            indices = [source_index for source_index, _p, _n in vertices]
            if indices != sorted(set(indices)):
                raise SystemExit("%s/%s: vertex indices are not strictly ascending"
                                 % (class_id, name))

        sidecar = args.resources / "Character" / class_id / "FaceMorphs" / (class_id + ".facemorphs")
        write_facemorphs(sidecar, base_vertex_count, vertex_index_bound, morphs)

        available = set(name.lower() for name, _ in morphs)
        documents = []
        for entry in sorted(args.customizing.iterdir()):
            match = PRESET_DOC.match(entry.name)
            if match and match.group("tag").upper() == tag:
                documents.append((int(match.group("index")), entry))
        documents.sort()
        if [index for index, _ in documents] != list(range(len(documents))):
            raise SystemExit("%s: preset indices are not contiguous from 0" % class_id)

        presets = []
        unresolved = []
        for index, entry in documents:
            weights = read_preset_morphs(entry)
            missing = [name for name, _ in weights if name.lower() not in available]
            if not weights:
                unresolved.append((index, "no morph array in document"))
            elif missing:
                unresolved.append((index, missing))
                weights = []
            presets.append({
                "index": index,
                "sourcePresetId": "PC_%s_%02d" % (tag, index),
                "morphs": [{"name": name, "weight": round(weight, 6)} for name, weight in weights],
            })

        presets_document["classes"][class_id] = {
            "faceMorphAssetId": "Character/%s/FaceMorphs/%s.facemorphs" % (class_id, class_id),
            "preset": presets,
        }

        lengths = [(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]) ** 0.5
                   for _name, vertices in morphs for _i, p, _n in vertices]
        components = [abs(c) for _name, vertices in morphs for _i, p, _n in vertices for c in p]
        print("%s: %d morphs, declared base mesh %d verts, vertex index bound %d, "
              "%d delta verts, %d bytes"
              % (class_id, len(morphs), base_vertex_count, vertex_index_bound,
                 sum(len(v) for _, v in morphs), sidecar.stat().st_size))
        if vertex_index_bound > base_vertex_count:
            print("  WARNING: morphs reference vertex %d but NumBaseMeshVerts is %d"
                  % (vertex_index_bound - 1, base_vertex_count))
        print("  |delta| min %.6f max %.6f mean %.6f (max abs component %.6f)"
              % (min(lengths), max(lengths), sum(lengths) / len(lengths), max(components)))
        print("  presets %d, resolved %d, unresolved %s"
              % (len(presets), len(presets) - len(unresolved), unresolved if unresolved else "none"))

    output = args.data / "UI" / "Customizing" / "CustomizingFacePresets.json"
    output.write_text(json.dumps(presets_document, ensure_ascii=False, indent=1) + "\n",
                      encoding="utf-8")
    print(str(output))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

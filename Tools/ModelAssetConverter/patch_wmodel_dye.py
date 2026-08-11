"""Patch a .wmodel's material section from WMA2 to WMA3 with dye data.

The source game paints characters through a colour-region mask (`_cm`) and
per-region tints instead of baking colour into `_d`; see
.md/JS/08-07/2026-08-07_LOSTARK_CHARACTER_DYE_COLOR_MASK_RESULT.md. The
converter that cooks .wmodel writes WMA2 and has no source here, so this tool
rewrites just the MATERIAL section in place -- geometry, skeleton and
animations are byte-identical. Applying is idempotent: a WMA3 input has its
dye values replaced.

usage:
  python patch_wmodel_dye.py list    <model.wmodel>
  python patch_wmodel_dye.py extract <props.txt...> -o tints.json
  python patch_wmodel_dye.py apply   <model.wmodel> <tints.json>

tints.json maps material name -> dye record:
  { "pc_sdm_00_upper_mi": {
      "colorMask": "textures/pc_sdm_00_upper_cm.tga",
      "diffusecolor":   [1, 1, 1, 1],
      "diffusecolor_a": [0.75, 0.14, 0.21, 0.76],
      "diffusecolor_b": [0.17, 0.17, 0.17, 0.36],
      "diffusecolor_c": [0.70, 0.53, 0.50, 0.53] } }

Materials absent from the JSON receive an empty mask path and identity tints,
which the runtime reads as no dye. A named colorMask file must exist next to
the wmodel or the patch refuses to write.
"""
import json
import re
import struct
import sys
from pathlib import Path

FILE_HEADER = struct.Struct("<4sHHII")
MODEL_META = struct.Struct("<4sIII16x")
SECTION_DESC = struct.Struct("<IIQQ40s")
MATERIAL_META = struct.Struct("<4sI")
ENTRY_V2 = struct.Struct("<IQ64s" + "520s" * 9)
ENTRY_V3 = struct.Struct("<IQ64s" + "520s" * 10 + "16f")
SECTION_MATERIAL = 2

IDENTITY = [1.0, 1.0, 1.0, 1.0]
TINT_KEYS = ("diffusecolor", "diffusecolor_a", "diffusecolor_b",
             "diffusecolor_c")

assert ENTRY_V2.size == 4756
assert ENTRY_V3.size == 5340


def cstr(raw):
    return raw.split(b"\0", 1)[0].decode("ascii", "replace")


def wstr(raw):
    text = raw.decode("utf-16-le", "replace")
    end = text.find("\0")
    return text if end < 0 else text[:end]


def wbytes(text, capacity_chars):
    encoded = text.encode("utf-16-le")
    limit = (capacity_chars - 1) * 2
    if len(encoded) > limit:
        raise SystemExit(f"path does not fit the entry: {text}")
    return encoded.ljust(capacity_chars * 2, b"\0")


class Container:
    def __init__(self, path):
        self.path = Path(path)
        data = self.path.read_bytes()
        magic, self.major, self.minor, self.flags, content_size = \
            FILE_HEADER.unpack_from(data, 0)
        if magic != b"WINT":
            raise SystemExit(f"not a WINT file: {path}")
        self.content = data[FILE_HEADER.size:FILE_HEADER.size + content_size]
        mmagic, count, self.anim_count, self.model_flags = \
            MODEL_META.unpack_from(self.content, 0)
        if mmagic != b"WMOD":
            raise SystemExit(f"not a WMOD container: {path}")
        self.descs = []
        offset = MODEL_META.size
        for _ in range(count):
            self.descs.append(list(SECTION_DESC.unpack_from(
                self.content, offset)))
            offset += SECTION_DESC.size
        first = self.descs[0]
        if self.content[first[2]:first[2] + 4] == b"WINT":
            self.base = 0
        elif self.content[first[2] + MODEL_META.size:
                          first[2] + MODEL_META.size + 4] == b"WINT":
            self.base = MODEL_META.size
        else:
            raise SystemExit("cannot determine the section offset base")
        self.sections = [
            self.content[self.base + d[2]:self.base + d[2] + d[3]]
            for d in self.descs]

    def rebuild(self):
        table_size = MODEL_META.size + SECTION_DESC.size * len(self.descs)
        cursor = table_size - self.base
        blobs = []
        for desc, blob in zip(self.descs, self.sections):
            desc[2] = cursor
            desc[3] = len(blob)
            cursor += len(blob)
            blobs.append(blob)
        content = bytearray()
        content += MODEL_META.pack(b"WMOD", len(self.descs),
                                   self.anim_count, self.model_flags)
        for desc in self.descs:
            content += SECTION_DESC.pack(desc[0], desc[1], desc[2],
                                         desc[3], desc[4])
        for blob in blobs:
            content += blob
        header = FILE_HEADER.pack(b"WINT", self.major, self.minor,
                                  self.flags, len(content))
        return header + bytes(content)


def parse_material_section(blob):
    magic, major, minor, flags, content_size = FILE_HEADER.unpack_from(blob, 0)
    payload = blob[FILE_HEADER.size:FILE_HEADER.size + content_size]
    mat_magic, count = MATERIAL_META.unpack_from(payload, 0)
    if mat_magic not in (b"WMA2", b"WMA3"):
        raise SystemExit(f"unsupported material version: {mat_magic}")
    entry = ENTRY_V3 if mat_magic == b"WMA3" else ENTRY_V2
    entries = []
    offset = MATERIAL_META.size
    for _ in range(count):
        entries.append(entry.unpack_from(payload, offset))
        offset += entry.size
    return (major, minor, flags), mat_magic, entries


def cmd_list(model_path):
    container = Container(model_path)
    for index, desc in enumerate(container.descs):
        if desc[0] != SECTION_MATERIAL:
            continue
        _, magic, entries = parse_material_section(container.sections[index])
        print(f"{model_path}: {magic.decode()} materials={len(entries)}")
        for fields in entries:
            name = cstr(fields[2])
            mask = wstr(fields[12]) if len(fields) > 13 else ""
            print(f"  [{fields[0]}] {name}"
                  + (f"  mask={mask}" if mask else ""))


def cmd_extract(prop_paths, out_path):
    tints = {}
    vector_pattern = re.compile(
        r"ParameterValue = \{ R=([-\d.eE]+), G=([-\d.eE]+), "
        r"B=([-\d.eE]+), A=([-\d.eE]+) \}\s*ParameterName = (\S+)")
    texture_pattern = re.compile(
        r"ParameterValue = Texture2D'tex\.(\S+)'\s*"
        r"ParameterName = texture_color_fx_skin")
    for prop_path in prop_paths:
        text = Path(prop_path).read_text(errors="replace")
        name = Path(prop_path).name.replace(".props.txt", "")
        record = {}
        mask = texture_pattern.search(text)
        if not mask:
            print(f"skip (no colour mask): {name}")
            continue
        record["colorMask"] = f"textures/{mask.group(1)}.tga"
        for match in vector_pattern.finditer(text):
            key = match.group(5)
            # The realpbr generation renamed the whole-material multiplier;
            # the region tints kept their names.
            if key == "basecolor_color":
                key = "diffusecolor"
            if key in TINT_KEYS:
                record[key] = [float(match.group(i)) for i in range(1, 5)]
        tints[name] = record
        print(f"{name}: mask={record['colorMask']} "
              f"tints={[k for k in TINT_KEYS if k in record]}")
    Path(out_path).write_text(json.dumps(tints, indent=2) + "\n",
                              encoding="utf-8")
    print(f"wrote {out_path} ({len(tints)} materials)")


def cmd_apply(model_path, tints_path):
    container = Container(model_path)
    tints = json.loads(Path(tints_path).read_text(encoding="utf-8"))
    model_dir = Path(model_path).parent
    patched = []
    for index, desc in enumerate(container.descs):
        if desc[0] != SECTION_MATERIAL:
            continue
        (major, minor, flags), _, entries = parse_material_section(
            container.sections[index])
        out_entries = []
        for fields in entries:
            name = cstr(fields[2])
            paths = list(fields[3:12])
            record = tints.get(name)
            mask_path = ""
            colors = [IDENTITY] * 4
            if record is not None:
                mask_path = record["colorMask"]
                if not (model_dir / mask_path).exists():
                    raise SystemExit(
                        f"colour mask missing beside the model: "
                        f"{model_dir / mask_path}")
                colors = [record.get(key, IDENTITY) for key in TINT_KEYS]
                patched.append(name)
            floats = [component for color in colors for component in color]
            out_entries.append(ENTRY_V3.pack(
                fields[0], fields[1], fields[2], *paths,
                wbytes(mask_path, 260), *floats))
        payload = MATERIAL_META.pack(b"WMA3", len(out_entries)) + \
            b"".join(out_entries)
        container.sections[index] = FILE_HEADER.pack(
            b"WINT", major, minor, flags, len(payload)) + payload
    backup = Path(str(model_path) + ".bak")
    if not backup.exists():
        backup.write_bytes(Path(model_path).read_bytes())
    Path(model_path).write_bytes(container.rebuild())
    reparsed = Container(model_path)
    for index, desc in enumerate(reparsed.descs):
        if desc[0] == SECTION_MATERIAL:
            parse_material_section(reparsed.sections[index])
    print(f"{model_path}: dyed {len(patched)} materials "
          f"({', '.join(patched) if patched else 'none'})")


def main():
    if len(sys.argv) < 3:
        raise SystemExit(__doc__)
    mode = sys.argv[1]
    if mode == "list":
        cmd_list(sys.argv[2])
    elif mode == "extract":
        arguments = sys.argv[2:]
        out = arguments[arguments.index("-o") + 1]
        cmd_extract(arguments[:arguments.index("-o")], out)
    elif mode == "apply":
        cmd_apply(sys.argv[2], sys.argv[3])
    else:
        raise SystemExit(__doc__)


if __name__ == "__main__":
    main()

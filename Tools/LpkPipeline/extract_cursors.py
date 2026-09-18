#!/usr/bin/env python3
"""Extract every retail mouse cursor from EFUI_CURSOR as a Windows animated cursor.

The retail cursors are not sprite sheets: each `efcursordata` export stores finished
Windows cursor files as byte arrays, one per (size, preset) -- an animated `.ani`
(`RIFF....ACON`) where the cursor moves, a static `.cur` otherwise, and a handful of `.ico`
(type 1) whose hotspot the export carries separately. All of them go straight into
`LoadCursorFromFileW`, animation included, once the `.ico` ones are re-tagged as `.cur` with
their authored hotspot.

Property layout of one export (`normal` has all 35 arrays):
  cursordata[_<size>][preset<N>]   the file bytes; <size> in full/large/xlarge/xxlarge or
                                   absent for the base size; <N> in 1..6
  <size>fileformat                 enum: which of the three formats the base array is
  <size>hotspotx / <size>hotspoty  the click point, needed to turn an .ico into a .cur
  <size>size, lastmodified*        bookkeeping strings

How the objects map onto the retail 접근성 tab:
  preset1..6                         마우스 커서 (7 choices: 기본 + 6 presets; 모코코 = 6)
  <size>                             마우스 커서 사이즈 (normal 48px, large 64, full 96,
                                     xlarge 128, xxlarge 192 -- note large < full)
  `<name>_blue` / `_black` / `_white` exports
                                     마우스 커서 외곽선 (파란색 / 검은색 / 흰색)

Why this tool exists: `umodel_lostark_cursordata.exe` writes only the unsuffixed arrays,
which is 26 of the 51 cursors and none of the presets the dropdown offers. This reads the
package itself.

Output
------
  Client/Bin/Resources/UI/Cursors/<preset>/<size>/<name>.<ani|cur>
  Client/Bin/Resources/UI/Cursors/manifest.json
      every written file with its format, pixel size and hotspot, plus the audit: exports
      without cursor arrays and payloads that were none of the three formats.

Reading a LostArk .upk (umodel cannot decode this class)
-------------------------------------------------------
Bodies are AES-then-LZ4 with an extended chunk table; see
`.md/GB/`-adjacent notes and `UEViewer_source_backup/Unreal/UnCoreCompression.cpp`:
  * `FCompressedChunk` is 20 bytes here, not the standard 16.
  * `CompressionFlags == 0x44`: per block AES-256-ECB the first
    `min(CompressedSize & ~15, 4096)` bytes with the ASCII key below, then LZ4 raw block.
  * `FObjectExport` is 68 bytes in this build, with SerialSize/SerialOffset at +32/+36
    (verified against umodel's own listing for this package).
  * Export data starts 4 bytes after SerialOffset, then ordinary UE3 tagged properties.
  * `IntProperty` serialises as three int32 (marker, 0, value) and advances 8 extra bytes.

Requires `pycryptodome` and `lz4`.

Usage:
  python extract_cursors.py [--repo <LostArk root>] [--packages <dir>] [--list]
"""

from __future__ import annotations

import argparse
import json
import re
import struct
import sys
from pathlib import Path

from Crypto.Cipher import AES
import lz4.block

PACKAGE_NAME = "OVSG0AAS7EM7D62YWWEBO8.upk"          # EFUI_CURSOR
DEFAULT_PACKAGES = Path(r"D:/Games/LOSTARK/EFGame/ReleasePC/Packages")
AES_KEY = b"V1ZEG1PL34V77SQW39A9I4VUW34T6L15"
COMPRESSED_MAGIC = 0x9E2A83C1
EXPORT_STRIDE = 68
EXPORT_SERIAL_AT = 32
ANI_MAGIC = (b"RIFF", b"ACON")
# cursordata / cursordata_full / cursordata_largepreset3 / ...
PROPERTY_RE = re.compile(r"^cursordata(?:_(full|large|xlarge|xxlarge))?(?:_?preset(\d+))?$")
SIZES = ("normal", "full", "large", "xlarge", "xxlarge")


def decompress_package(path: Path):
    """Whole package as one flat buffer, plus the summary offsets."""
    data = path.read_bytes()
    o = 0
    o += 4                                              # tag
    version, licensee = struct.unpack_from("<HH", data, o); o += 4
    headerSize, = struct.unpack_from("<i", data, o); o += 4
    folderLength, = struct.unpack_from("<i", data, o); o += 4
    o += (-folderLength * 2) if folderLength < 0 else folderLength
    o += 4                                              # package flags
    (nameCount, nameOffset, exportCount, exportOffset,
     importCount, importOffset, dependsOffset) = struct.unpack_from("<7I", data, o); o += 28
    o += 16                                             # extra header offsets
    o += 16                                             # guid
    generations, = struct.unpack_from("<I", data, o); o += 4
    o += generations * 12
    engineVersion, cookerVersion, compressionFlags, chunkCount = \
        struct.unpack_from("<4I", data, o); o += 16

    chunks = []
    for _ in range(chunkCount):
        chunks.append(struct.unpack_from("<5I", data, o)); o += 20
    if not chunks:
        return data, dict(nameCount=nameCount, nameOffset=nameOffset,
                          exportCount=exportCount, exportOffset=exportOffset,
                          importCount=importCount, importOffset=importOffset)

    total = max(uo + us for uo, us, _, _, _ in chunks)
    out = bytearray(max(total, headerSize))
    out[0:chunks[0][0]] = data[0:chunks[0][0]]
    for uncompressedOffset, uncompressedSize, compressedOffset, compressedSize, _ in chunks:
        magic, = struct.unpack_from("<I", data, compressedOffset)
        if magic != COMPRESSED_MAGIC:
            out[uncompressedOffset:uncompressedOffset + uncompressedSize] = \
                data[compressedOffset:compressedOffset + compressedSize]
            continue
        _, blockSize, _, uncompressedTotal = struct.unpack_from("<4I", data, compressedOffset)
        blockCount = (uncompressedTotal + blockSize - 1) // blockSize
        cursor = compressedOffset + 16 + 8 * blockCount
        destination = uncompressedOffset
        for b in range(blockCount):
            blockCompressed, blockUncompressed = \
                struct.unpack_from("<II", data, compressedOffset + 16 + 8 * b)
            block = bytearray(data[cursor:cursor + blockCompressed]); cursor += blockCompressed
            if compressionFlags == 0x44:
                encrypted = min(blockCompressed & ~15, 4096)
                if encrypted > 0:
                    block[0:encrypted] = \
                        AES.new(AES_KEY, AES.MODE_ECB).decrypt(bytes(block[0:encrypted]))
            out[destination:destination + blockUncompressed] = \
                lz4.block.decompress(bytes(block), uncompressed_size=blockUncompressed)
            destination += blockUncompressed
    return bytes(out), dict(nameCount=nameCount, nameOffset=nameOffset,
                            exportCount=exportCount, exportOffset=exportOffset,
                            importCount=importCount, importOffset=importOffset)


def read_names(buf: bytes, info: dict) -> list[str]:
    o = info["nameOffset"]
    names = []
    for _ in range(info["nameCount"]):
        length, = struct.unpack_from("<i", buf, o); o += 4
        if length < 0:
            names.append(buf[o:o - length * 2].decode("utf-16-le").rstrip("\0")); o += -length * 2
        else:
            names.append(buf[o:o + length].decode("cp949", "replace").rstrip("\0")); o += length
        o += 8                                          # name flags
    return names


def read_exports(buf: bytes, info: dict, names: list[str]) -> list[dict]:
    exports = []
    for i in range(info["exportCount"]):
        base = info["exportOffset"] + i * EXPORT_STRIDE
        classIndex, = struct.unpack_from("<i", buf, base)
        nameIndex, = struct.unpack_from("<i", buf, base + 12)
        serialSize, serialOffset = struct.unpack_from("<ii", buf, base + EXPORT_SERIAL_AT)
        exports.append({
            "index": i,
            "classIndex": classIndex,
            "name": names[nameIndex] if 0 <= nameIndex < len(names) else "?%d" % nameIndex,
            "serialSize": serialSize,
            "serialOffset": serialOffset,
        })
    return exports


class Reader:
    def __init__(self, buf: bytes, names: list[str], offset: int):
        self.buf = buf
        self.names = names
        self.o = offset

    def i32(self) -> int:
        v, = struct.unpack_from("<i", self.buf, self.o); self.o += 4
        return v

    def name(self) -> str:
        index = self.i32()
        self.i32()                                      # name number
        return self.names[index] if 0 <= index < len(self.names) else "?%d" % index


def read_properties(buf: bytes, names: list[str], export: dict) -> dict[str, object]:
    """The tagged properties of one export, keyed by name.

    Only the shapes this class uses: byte arrays (the cursor files), one-byte and enum-name
    ByteProperty (hotspots, file formats), StrProperty. Anything else is skipped by its
    declared size.
    """
    start = export["serialOffset"] + 4
    end = export["serialOffset"] + export["serialSize"]
    r = Reader(buf, names, start)
    out: dict[str, object] = {}
    while r.o < end:
        try:
            propertyName = r.name()
        except struct.error:
            break
        if propertyName.lower() == "none":
            break
        propertyType = r.name().lower()
        size = r.i32()
        r.i32()                                         # array index
        if propertyType == "structproperty":
            r.name()
        elif propertyType == "boolproperty":
            r.o += 1
        elif propertyType == "byteproperty":
            r.name()                                    # enum type
        if size < 0 or r.o + size > end:
            break
        valueStart = r.o
        if propertyType == "arrayproperty":
            count, = struct.unpack_from("<i", buf, valueStart)
            if 0 < count <= size - 4:
                out[propertyName] = buf[valueStart + 4:valueStart + 4 + count]
        elif propertyType == "byteproperty":
            if size == 1:
                out[propertyName] = buf[valueStart]
            elif size == 8:
                index, = struct.unpack_from("<i", buf, valueStart)
                out[propertyName] = names[index] if 0 <= index < len(names) else "?%d" % index
        elif propertyType == "strproperty":
            length, = struct.unpack_from("<i", buf, valueStart)
            if length < 0:
                out[propertyName] = buf[valueStart + 4:valueStart + 4 - length * 2] \
                    .decode("utf-16-le", "replace").rstrip("\0")
            elif length > 0:
                out[propertyName] = buf[valueStart + 4:valueStart + 4 + length] \
                    .decode("cp949", "replace").rstrip("\0")
        r.o = valueStart + size + (8 if propertyType == "intproperty" else 0)
    return out


def describe_payload(payload: bytes):
    """("ani"|"cur"|"ico", width, height, hotspotX, hotspotY) or None."""
    if payload[0:4] == b"RIFF" and payload[8:12] == b"ACON":
        return "ani", 0, 0, None, None
    if len(payload) >= 22 and payload[0:2] == b"\0\0" and payload[4:6] == b"\1\0":
        kind = {1: "ico", 2: "cur"}.get(struct.unpack_from("<H", payload, 2)[0])
        if kind is None:
            return None
        width, height = payload[6], payload[7]
        hotspotX, hotspotY = struct.unpack_from("<HH", payload, 10)
        return kind, width or 256, height or 256, hotspotX, hotspotY
    return None


def as_cur(payload: bytes, hotspotX: int, hotspotY: int) -> bytes:
    """Re-tag an .ico entry as a .cur carrying the export's authored hotspot."""
    fixed = bytearray(payload)
    struct.pack_into("<H", fixed, 2, 2)
    struct.pack_into("<HH", fixed, 10, hotspotX, hotspotY)
    return bytes(fixed)


def classify(propertyName: str):
    """`cursordata_xlargepreset2` -> ("preset2", "xlarge"); unsuffixed -> ("default","normal")."""
    m = PROPERTY_RE.match(propertyName)
    if not m:
        return None
    size = m.group(1) or "normal"
    preset = ("preset" + m.group(2)) if m.group(2) else "default"
    return preset, size


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[2])
    ap.add_argument("--packages", type=Path, default=DEFAULT_PACKAGES)
    ap.add_argument("--list", action="store_true",
                    help="report the package's property inventory and write nothing")
    args = ap.parse_args()

    package = args.packages / PACKAGE_NAME
    if not package.exists():
        print("missing package:", package)
        return 1

    buf, info = decompress_package(package)
    names = read_names(buf, info)
    exports = read_exports(buf, info, names)
    cursorClass = [-(i + 1) for i in range(info["importCount"])]
    # every export whose properties actually hold cursor arrays
    manifest: dict[str, dict[str, dict[str, str]]] = {}
    inventory: dict[str, list[str]] = {}
    unparsed: list[str] = []
    notAni: list[str] = []

    out_root = args.repo / "Client/Bin/Resources/UI/Cursors"
    written = 0
    for export in exports:
        if export["serialSize"] <= 0:
            continue
        props = read_properties(buf, names, export)
        arrays = {k: v for k, v in props.items() if isinstance(v, bytes)}
        if not arrays:
            unparsed.append(export["name"])
            continue
        inventory[export["name"]] = sorted(arrays)
        for propertyName, payload in sorted(arrays.items()):
            slot = classify(propertyName)
            if slot is None:
                continue
            preset, size = slot
            described = describe_payload(payload)
            if described is None:
                notAni.append("%s.%s" % (export["name"], propertyName))
                continue
            kind, width, height, hotspotX, hotspotY = described
            prefix = "" if size == "normal" else size
            authoredX = props.get(prefix + "hotspotx")
            authoredY = props.get(prefix + "hotspoty")
            if kind == "ico":
                # the export carries the click point the .ico header cannot
                payload = as_cur(payload, int(authoredX or 0), int(authoredY or 0))
                hotspotX, hotspotY = int(authoredX or 0), int(authoredY or 0)
                kind = "cur"
            extension = "ani" if kind == "ani" else "cur"
            relative = "UI/Cursors/%s/%s/%s.%s" % (preset, size, export["name"], extension)
            manifest.setdefault(export["name"], {}).setdefault(preset, {})[size] = {
                "path": relative,
                "format": kind,
                "width": width,
                "height": height,
                "hotspotX": hotspotX if hotspotX is not None else authoredX,
                "hotspotY": hotspotY if hotspotY is not None else authoredY,
            }
            if args.list:
                continue
            target = args.repo / "Client/Bin/Resources" / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(payload)
            written += 1

    presets = sorted({p for byName in manifest.values() for p in byName})
    print("exports %d, cursors with data %d, presets %s" %
          (len(exports), len(manifest), presets))
    for name in sorted(manifest):
        have = manifest[name]
        formats = sorted({entry["format"] for bySize in have.values() for entry in bySize.values()})
        print("  %-24s %-8s %s" % (name, "/".join(formats), " ".join(
            "%s[%s]" % (p, ",".join(s for s in SIZES if s in have[p])) for p in sorted(have))))
    if unparsed:
        print("no cursor arrays (%d): %s" % (len(unparsed), ", ".join(sorted(unparsed))))
    if notAni:
        print("!! payload was none of ani/cur/ico (%d): %s" % (len(notAni), ", ".join(notAni[:8])))

    if args.list:
        return 0

    out_root.mkdir(parents=True, exist_ok=True)
    (out_root / "manifest.json").write_text(json.dumps({
        "schema": "lostark.cursor-manifest",
        "formatVersion": 1,
        "source": {"package": PACKAGE_NAME, "friendlyName": "EFUI_CURSOR"},
        "presets": presets,
        "sizes": list(SIZES),
        "cursors": manifest,
        "audit": {"exportsWithoutCursorArrays": sorted(unparsed), "payloadUnknownFormat": notAni},
    }, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
    print("wrote %d cursor files + manifest.json to %s" % (written, out_root))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

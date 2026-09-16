#!/usr/bin/env python3
"""Recover an exact cooked BC mip chain through a Lost Ark UModel scratch copy.

The original packages and the expected DDS are read-only. No resampling, color
conversion, recompression, normal adjustment, or runtime installation occurs.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
import sys
import tempfile
from typing import Any

import extract_ue3_placements as up
from source_extraction_io import write_pair


class MipExtractionError(ValueError):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise MipExtractionError(message)


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


@dataclass(frozen=True)
class BlockDDS:
    header: tuple[int, ...]
    fourcc: bytes
    width: int
    height: int
    mip_count: int
    payloads: tuple[bytes, ...]


def block_bytes(width: int, height: int, fourcc: bytes) -> int:
    require(fourcc in (b"DXT1", b"DXT5", b"ATI2"),
            f"unsupported DDS format {fourcc!r}; expected BC1/BC3/BC5 legacy DDS")
    return max(1, (width + 3) // 4) * max(1, (height + 3) // 4) * (8 if fourcc == b"DXT1" else 16)


def parse_dds(data: bytes) -> BlockDDS:
    require(len(data) >= 128 and data[:4] == b"DDS ", "expected a legacy BC DDS")
    header = struct.unpack_from("<31I", data, 4)
    require(header[0] == 124 and header[18] == 32, "invalid DDS header size")
    require(header[19] & 4 != 0, "DDS has no FourCC")
    require(header[27] == 0 and header[5] in (0, 1), "cube/volume DDS is unsupported")
    fourcc = data[84:88]
    width, height, count = header[3], header[2], max(1, header[6])
    require(1 <= width <= 32768 and 1 <= height <= 32768, "invalid DDS dimensions")
    require(1 <= count <= max(width, height).bit_length(), "invalid DDS mip count")
    payloads, offset = [], 128
    for level in range(count):
        size = block_bytes(max(1, width >> level), max(1, height >> level), fourcc)
        require(offset + size <= len(data), f"truncated DDS mip {level}")
        payloads.append(data[offset:offset + size])
        offset += size
    require(offset == len(data), "unexpected DDS trailing payload")
    return BlockDDS(header, fourcc, width, height, count, tuple(payloads))


class SourcePackage:
    def __init__(self, path: Path):
        self.path = path.resolve(strict=True)
        self.physical = self.path.read_bytes()
        self.digest = sha256(self.physical)
        self.summary = up.parse_summary(self.physical)
        self.reader = up.LostArkPackageRangeReader(self.path, self.summary)
        header = self.reader.read_logical_range(0, self.summary.header_size)
        self.names = up.parse_name_table(header, self.summary)
        self.imports = up.parse_import_table(header, self.summary, self.names)
        self.exports = up.parse_export_table(header, self.summary, self.names)

    def ref(self, index: int) -> str:
        return up.package_ref_path(index, self.imports, self.exports)

    def raw(self, index: int) -> bytes:
        entry = self.exports[index]
        return self.reader.read_logical_range(entry.serial_offset, entry.serial_size)

    def cls(self, index: int) -> str:
        return up.package_ref_name(self.exports[index].class_index, self.imports, self.exports).casefold()

    def find(self, object_path: str) -> int:
        matches = [i for i in range(len(self.exports))
                   if self.ref(i + 1).casefold() == object_path.casefold()]
        require(len(matches) == 1, f"missing/ambiguous source object {object_path}: {matches}")
        return matches[0]


def split_object(source: str) -> tuple[str, str]:
    require(bool(re.fullmatch(r"[A-Za-z0-9_]+(?:\.[A-Za-z0-9_]+)+", source)),
            f"invalid source object path: {source}")
    logical, object_path = source.split(".", 1)
    return logical, object_path


def resolve_texture(source: str, source_package: Path, package_root: Path,
                    umodel: Path, region: str) -> tuple[SourcePackage, int, str, list[dict], dict[str, SourcePackage]]:
    logical, object_path = split_object(source)
    packages = {logical.casefold(): SourcePackage(source_package)}
    redirects: list[dict] = []
    seen = set()
    while True:
        identity = f"{logical}.{object_path}"
        require(identity.casefold() not in seen and len(seen) < 8, "redirect cycle or depth exceeds 8")
        seen.add(identity.casefold())
        key = logical.casefold()
        if key not in packages:
            resolved = up.resolve_physical_package(umodel, package_root, logical, region).resolve(strict=True)
            require(resolved.is_relative_to(package_root), "redirect package escaped package root")
            packages[key] = SourcePackage(resolved)
        package = packages[key]
        index = package.find(object_path)
        if package.cls(index) != "objectredirector":
            require(package.cls(index) == "texture2d", f"unsupported source class {package.cls(index)}")
            return package, index, identity, redirects, packages
        serial = package.raw(index)
        # These cooked redirectors contain an object prefix, an empty property
        # list (None FName), then the destination reference. The generic tagged
        # property locator intentionally rejects an empty list.
        require(len(serial) == 16, "unsupported ObjectRedirector native layout")
        redirect_reader = up.Reader(serial, 4)
        terminator, number = up.parse_fname(redirect_reader, package.names)
        require(terminator.casefold() == "none" and number == 0,
                "unsupported ObjectRedirector property list")
        destination = package.ref(redirect_reader.i32())
        redirects.append({"sourceObject": identity, "destination": destination})
        logical, object_path = split_object(destination)


@dataclass(frozen=True)
class NativeMip:
    flags: int
    elements: int
    packed: bytes
    width: int
    height: int


def parse_native_mips(serial: bytes, property_end: int, serial_offset: int) -> tuple[list[NativeMip], bytes]:
    reader = up.Reader(serial, property_end)
    _, elements, size, _ = reader.unpack("<4i")
    require(elements == size == 0, "nonempty Texture2D source-art bulk is unsupported")
    count = reader.i32()
    require(1 <= count <= 16, f"unsupported native mip count {count}")
    records = []
    for level in range(count):
        flags, elements, size, offset = reader.unpack("<4i")
        require(size > 0 and elements > 0 and offset == serial_offset + reader.offset,
                f"mip {level} is missing or uses external/non-inline bulk")
        packed = reader.read(size)
        width, height = reader.unpack("<2i")
        require(1 <= width <= 32768 and 1 <= height <= 32768, f"invalid mip {level} dimensions")
        records.append(NativeMip(flags, elements, packed, width, height))
    return records, serial[reader.offset:]


def uncompressed_scratch(package: SourcePackage) -> tuple[bytearray, int]:
    """Preserve the physical summary prefix even when its chunk table grew."""
    first = min(chunk.uncompressed_offset for chunk in package.summary.chunks)
    physical_first = min(chunk.compressed_offset for chunk in package.summary.chunks)
    shift = physical_first - first
    require(shift >= 0, "unsupported physical/logical package prefix layout")
    blob = bytearray(package.physical[:physical_first])
    blob.extend(package.reader.read_logical_range(first, package.reader.logical_size - first))
    # The summary parser records the GUID position, so optional version fields
    # before it need not be duplicated here.
    reader = up.Reader(blob, package.summary.package_guid_offset + 16)
    generations = reader.i32()
    reader.read(generations * 12)
    if package.summary.version >= 245:
        reader.read(4)
    if package.summary.version >= 277:
        reader.read(4)
    require(package.summary.version >= 334, "unsupported package summary version")
    reader.read(4)
    require(reader.i32() == len(package.summary.chunks), "scratch chunk table mismatch")
    require(reader.offset + 20 * len(package.summary.chunks) <= physical_first,
            "scratch chunk table overlaps logical data")
    for chunk in package.summary.chunks:
        struct.pack_into("<5i", blob, reader.offset, chunk.uncompressed_offset,
                         chunk.uncompressed_size, chunk.uncompressed_offset + shift,
                         chunk.uncompressed_size, 0)
        reader.read(20)
    return blob, shift


def rotate_mips(serial: bytes, property_end: int, serial_offset: int,
                records: list[NativeMip], suffix: bytes, level: int) -> bytes:
    rotated = bytearray(serial[:property_end + 20])
    for mip in records[level:] + records[:level]:
        rotated.extend(struct.pack("<4i", mip.flags, mip.elements, len(mip.packed),
                                   serial_offset + len(rotated) + 16))
        rotated.extend(mip.packed)
        rotated.extend(struct.pack("<2i", mip.width, mip.height))
    rotated.extend(suffix)
    require(len(rotated) == len(serial), "scratch mip rotation changed export size")
    return bytes(rotated)


def validate_paths(source_package: Path, package_root: Path, umodel: Path,
                   expected: Path, output: Path, receipt: Path, scratch_root: Path) -> None:
    allowed_scratch = Path(__file__).resolve().parents[2] / "out"
    require(scratch_root.is_relative_to(allowed_scratch.resolve()) and scratch_root != allowed_scratch.resolve(),
            "scratch-root must be a dedicated subdirectory of repository out")
    require(source_package.is_file() and source_package.is_relative_to(package_root),
            "source-package must be an existing file beneath package-root")
    require(umodel.is_file() and expected.is_file(), "umodel and expected-mip0 must exist")
    require(len({source_package, umodel, expected, output, receipt}) == 5,
            "source, decoder, expected input, output and receipt must be distinct paths")
    require(output.suffix.casefold() == ".dds", "output must have .dds extension")
    for target in (scratch_root, output, receipt):
        require(not target.is_relative_to(package_root) and not target.is_relative_to(umodel.parent),
                "output/scratch must not be inside source packages or decoder directory")


def extract_texture_mips(*, source_object: str, source_package: Path, package_root: Path,
                         umodel: Path, expected_mip0: Path, output: Path,
                         scratch_root: Path, receipt: Path | None = None,
                         region: str = "kr", timeout_seconds: int = 120) -> dict[str, Any]:
    source_package, package_root, umodel, expected_mip0, output, scratch_root = (
        path.resolve() for path in (source_package, package_root, umodel, expected_mip0, output, scratch_root))
    receipt = (receipt or output.with_suffix(".dds.receipt.json")).resolve()
    require(region == "kr" and timeout_seconds > 0, "only verified KR packages and a positive decoder timeout are supported")
    validate_paths(source_package, package_root, umodel, expected_mip0, output, receipt, scratch_root)
    expected_bytes = expected_mip0.read_bytes()
    expected = parse_dds(expected_bytes)
    decoder_digest = sha256(umodel.read_bytes())
    package, index, resolved, redirects, packages = resolve_texture(
        source_object, source_package, package_root, umodel, region)
    serial = package.raw(index)
    properties, end = up.parse_tagged_properties(serial, package.names, package.summary.version)
    serial_offset = package.exports[index].serial_offset
    records, suffix = parse_native_mips(serial, end, serial_offset)
    require(len(records) == max(expected.width, expected.height).bit_length(),
            "source does not contain the complete mip chain through 1x1")
    for level, mip in enumerate(records):
        dimensions = max(1, expected.width >> level), max(1, expected.height >> level)
        require((mip.width, mip.height) in (dimensions, tuple(max(4, value) for value in dimensions)),
                f"source mip {level} dimensions disagree with expected DDS")
    scratch_root.mkdir(parents=True, exist_ok=True)
    run_dir = Path(tempfile.mkdtemp(prefix="source-mips-", dir=scratch_root))
    scratch, shift = uncompressed_scratch(package)
    physical_start = serial_offset + shift
    require(physical_start + len(serial) <= len(scratch), "scratch export outside package")
    payloads, mip_receipts = [], []
    scratch_path = run_dir / "source_mip.upk"
    for level, mip in enumerate(records):
        scratch[physical_start:physical_start + len(serial)] = rotate_mips(
            serial, end, serial_offset, records, suffix, level)
        scratch_path.write_bytes(scratch)
        export_dir = run_dir / f"export{level:02}"
        completed = subprocess.run(
            [str(umodel), "-export", "-game=lostark", f"-{region}", f"-path={run_dir}",
             f"-out={export_dir}", "-dds", str(scratch_path), package.exports[index].object_name],
            capture_output=True, text=True, encoding="utf-8", errors="replace", timeout=timeout_seconds,
            creationflags=subprocess.CREATE_NO_WINDOW if sys.platform == "win32" else 0)
        log = completed.stdout + "\n" + completed.stderr
        (run_dir / f"mip{level:02}.umodel.log").write_text(log, encoding="utf-8")
        require(completed.returncode == 0, f"UModel mip {level} failed ({completed.returncode}): {log[-2000:]}")
        exported = list(export_dir.rglob("*.dds"))
        require(len(exported) == 1, f"expected one DDS for mip {level}, found {len(exported)}")
        decoded = parse_dds(exported[0].read_bytes())
        require(decoded.fourcc == expected.fourcc and decoded.mip_count == 1 and
                (decoded.width, decoded.height) == (mip.width, mip.height),
                f"decoder mip {level} format/dimensions/count mismatch")
        blocks = decoded.payloads[0]
        if level == 0:
            require(blocks == expected.payloads[0], "mip0 compressed bytes differ from expected input")
        payloads.append(blocks)
        mip_receipts.append({"level": level, "width": max(1, expected.width >> level),
                             "height": max(1, expected.height >> level),
                             "sourceStorageWidth": mip.width, "sourceStorageHeight": mip.height,
                             "packedBytes": len(mip.packed), "packedSHA256": sha256(mip.packed),
                             "blockBytes": len(blocks), "blocksSHA256": sha256(blocks)})
    header = list(expected.header)
    header[1] |= 0x20000
    header[6] = len(records)
    header[26] |= 0x400008
    staged = b"DDS " + struct.pack("<31I", *header) + b"".join(payloads)
    parsed = parse_dds(staged)
    require(parsed.payloads == tuple(payloads), "final DDS validation failed")
    for original in packages.values():
        require(sha256(original.path.read_bytes()) == original.digest, "source package changed during extraction")
    require(expected_mip0.read_bytes() == expected_bytes and sha256(umodel.read_bytes()) == decoder_digest,
            "expected input or decoder changed during extraction")
    result = {"schema": "lostark.ue3-texture-mips", "formatVersion": 1,
              "status": "SOURCE_MIP_CHAIN_VALIDATED", "sourceObject": source_object,
              "resolvedSourceObject": resolved, "redirects": redirects,
              "sourceExportIndex0": index, "sourceSerialSHA256": sha256(serial), "properties": properties,
              "sourcePackages": [{"logicalName": name, "file": str(item.path), "sha256": item.digest}
                                 for name, item in packages.items()],
              "decoder": {"file": str(umodel), "sha256": decoder_digest},
              "expectedMip0": {"file": str(expected_mip0), "sha256": sha256(expected_bytes)},
              "mip0CompressedBytesIdentical": True, "sourcePackagesUnchanged": True,
              "fourCC": expected.fourcc.decode("ascii"), "mipCount": len(records), "mips": mip_receipts,
              "output": str(output), "outputSHA256": sha256(staged), "outputBytes": len(staged),
              "scratchDirectory": str(run_dir), "sourceTrailingBytes": len(suffix),
              "conversion": "original cooked blocks; no filtering, recompression, color or normal conversion"}
    receipt_bytes = (json.dumps(result, ensure_ascii=False, indent=2) + "\n").encode("utf-8")
    # All decoding and validation precedes either destination replacement.
    write_pair(output, staged, receipt, receipt_bytes)
    return result


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-object", required=True)
    for name in ("source-package", "package-root", "umodel", "expected-mip0", "output", "scratch-root"):
        parser.add_argument(f"--{name}", type=Path, required=True)
    parser.add_argument("--receipt", type=Path)
    parser.add_argument("--region", choices=("kr",), default="kr")
    parser.add_argument("--timeout-seconds", type=int, default=120)
    args = parser.parse_args(argv)
    try:
        result = extract_texture_mips(**vars(args))
    except (MipExtractionError, up.ExtractionError, OSError, subprocess.SubprocessError) as error:
        print(f"texture mip extraction failed: {error}", file=sys.stderr)
        return 1
    print(f"{result['resolvedSourceObject']}: {result['mipCount']} original mips; mip0 exact; {result['output']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

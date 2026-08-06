#!/usr/bin/env python3
"""Recover UE3 StaticMesh placements from Lost Ark level packages.

The tool deliberately stays read-only.  It resolves a logical package with the
Lost Ark UModel build, decrypts/decompresses the original UPK in memory, parses
the UE3 name/import/export tables, then joins StaticMeshActor exports to their
StaticMeshComponent exports and imported StaticMesh references.
"""

from __future__ import annotations

import argparse
import ctypes
import json
import math
import re
import struct
import subprocess
import sys
from contextlib import AbstractContextManager
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any, Iterable


PACKAGE_FILE_TAG = 0x9E2A83C1
LOSTARK_KR_AES_KEY = "V1ZEG1PL34V77SQW39A9I4VUW34T6L15"
ROTATOR_UNITS_PER_TURN = 65536.0
PLACEMENT_ACTOR_CLASSES = frozenset(
    (
        "staticmeshactor",
        "interpactor",
        "staticmeshcollectionactor",
        "efmotionstaticmeshactor",
    )
)


class ExtractionError(RuntimeError):
    pass


class Reader:
    def __init__(self, data: bytes | bytearray | memoryview, offset: int = 0):
        self.data = memoryview(data)
        self.offset = offset

    def require(self, size: int) -> None:
        if size < 0 or self.offset + size > len(self.data):
            raise ExtractionError(
                f"read outside buffer: offset=0x{self.offset:X} size=0x{size:X} "
                f"buffer=0x{len(self.data):X}"
            )

    def read(self, size: int) -> bytes:
        self.require(size)
        result = self.data[self.offset : self.offset + size].tobytes()
        self.offset += size
        return result

    def unpack(self, fmt: str) -> tuple[Any, ...]:
        size = struct.calcsize(fmt)
        self.require(size)
        result = struct.unpack_from(fmt, self.data, self.offset)
        self.offset += size
        return result

    def i32(self) -> int:
        return self.unpack("<i")[0]

    def u32(self) -> int:
        return self.unpack("<I")[0]

    def u64(self) -> int:
        return self.unpack("<Q")[0]

    def f32(self) -> float:
        return self.unpack("<f")[0]

    def fstring(self) -> str:
        length = self.i32()
        if length == 0:
            return ""
        if length > 0:
            raw = self.read(length)
            return raw.rstrip(b"\0").decode("latin-1")
        raw = self.read((-length) * 2)
        if raw.endswith(b"\0\0"):
            raw = raw[:-2]
        return raw.decode("utf-16-le")


class WindowsAesEcbDecryptor(AbstractContextManager["WindowsAesEcbDecryptor"]):
    """Small dependency-free AES-256 ECB wrapper around Windows CNG."""

    def __init__(self, key: bytes):
        if sys.platform != "win32":
            raise ExtractionError("Lost Ark AES decryption currently requires Windows")
        if len(key) != 32:
            raise ExtractionError(f"AES-256 key must be 32 bytes, got {len(key)}")

        self._bcrypt = ctypes.WinDLL("bcrypt", use_last_error=True)
        self._alg = ctypes.c_void_p()
        self._key = ctypes.c_void_p()
        self._key_object: ctypes.Array[ctypes.c_char] | None = None

        self._configure_signatures()
        self._check(
            self._bcrypt.BCryptOpenAlgorithmProvider(
                ctypes.byref(self._alg), "AES", None, 0
            ),
            "BCryptOpenAlgorithmProvider",
        )

        chaining_mode = ctypes.create_unicode_buffer("ChainingModeECB")
        self._check(
            self._bcrypt.BCryptSetProperty(
                self._alg,
                "ChainingMode",
                ctypes.cast(chaining_mode, ctypes.c_void_p),
                ctypes.sizeof(chaining_mode),
                0,
            ),
            "BCryptSetProperty(ChainingModeECB)",
        )

        object_length = ctypes.c_uint32()
        result_length = ctypes.c_uint32()
        self._check(
            self._bcrypt.BCryptGetProperty(
                self._alg,
                "ObjectLength",
                ctypes.byref(object_length),
                ctypes.sizeof(object_length),
                ctypes.byref(result_length),
                0,
            ),
            "BCryptGetProperty(ObjectLength)",
        )

        self._key_object = ctypes.create_string_buffer(object_length.value)
        secret = ctypes.create_string_buffer(key, len(key))
        self._check(
            self._bcrypt.BCryptGenerateSymmetricKey(
                self._alg,
                ctypes.byref(self._key),
                self._key_object,
                object_length.value,
                secret,
                len(key),
                0,
            ),
            "BCryptGenerateSymmetricKey",
        )

    def _configure_signatures(self) -> None:
        bcrypt = self._bcrypt
        bcrypt.BCryptOpenAlgorithmProvider.restype = ctypes.c_long
        bcrypt.BCryptSetProperty.restype = ctypes.c_long
        bcrypt.BCryptGetProperty.restype = ctypes.c_long
        bcrypt.BCryptGenerateSymmetricKey.restype = ctypes.c_long
        bcrypt.BCryptDecrypt.restype = ctypes.c_long
        bcrypt.BCryptDestroyKey.restype = ctypes.c_long
        bcrypt.BCryptCloseAlgorithmProvider.restype = ctypes.c_long

    @staticmethod
    def _check(status: int, operation: str) -> None:
        if status != 0:
            raise ExtractionError(f"{operation} failed with NTSTATUS 0x{status & 0xFFFFFFFF:08X}")

    def decrypt(self, data: bytes) -> bytes:
        if len(data) % 16:
            raise ExtractionError("AES ECB input length must be a multiple of 16")
        source = ctypes.create_string_buffer(data, len(data))
        output = ctypes.create_string_buffer(len(data))
        written = ctypes.c_uint32()
        self._check(
            self._bcrypt.BCryptDecrypt(
                self._key,
                source,
                len(data),
                None,
                None,
                0,
                output,
                len(data),
                ctypes.byref(written),
                0,
            ),
            "BCryptDecrypt",
        )
        if written.value != len(data):
            raise ExtractionError(
                f"BCryptDecrypt returned {written.value} bytes for {len(data)} bytes"
            )
        return output.raw[: written.value]

    def close(self) -> None:
        if self._key:
            self._bcrypt.BCryptDestroyKey(self._key)
            self._key = ctypes.c_void_p()
        if self._alg:
            self._bcrypt.BCryptCloseAlgorithmProvider(self._alg, 0)
            self._alg = ctypes.c_void_p()

    def __exit__(self, exc_type: Any, exc_value: Any, traceback: Any) -> None:
        self.close()


def decompress_lz4_block(source: bytes, expected_size: int) -> bytes:
    input_offset = 0
    output = bytearray()
    source_size = len(source)

    while input_offset < source_size:
        token = source[input_offset]
        input_offset += 1

        literal_length = token >> 4
        if literal_length == 15:
            while True:
                if input_offset >= source_size:
                    raise ExtractionError("truncated LZ4 literal length")
                value = source[input_offset]
                input_offset += 1
                literal_length += value
                if value != 255:
                    break

        if input_offset + literal_length > source_size:
            raise ExtractionError("LZ4 literal runs beyond compressed block")
        output.extend(source[input_offset : input_offset + literal_length])
        input_offset += literal_length

        if input_offset == source_size:
            break
        if input_offset + 2 > source_size:
            raise ExtractionError("truncated LZ4 match offset")

        match_offset = source[input_offset] | (source[input_offset + 1] << 8)
        input_offset += 2
        if match_offset == 0 or match_offset > len(output):
            raise ExtractionError(
                f"invalid LZ4 match offset {match_offset} at output {len(output)}"
            )

        match_length = (token & 0x0F) + 4
        if (token & 0x0F) == 15:
            while True:
                if input_offset >= source_size:
                    raise ExtractionError("truncated LZ4 match length")
                value = source[input_offset]
                input_offset += 1
                match_length += value
                if value != 255:
                    break

        for _ in range(match_length):
            output.append(output[-match_offset])
        if len(output) > expected_size:
            raise ExtractionError("LZ4 block expanded beyond its declared size")

    if len(output) != expected_size:
        raise ExtractionError(
            f"LZ4 size mismatch: got {len(output)}, expected {expected_size}"
        )
    return bytes(output)


@dataclass(frozen=True)
class CompressedChunk:
    uncompressed_offset: int
    uncompressed_size: int
    compressed_offset: int
    compressed_size: int
    encrypted_lz4: int


@dataclass(frozen=True)
class PackageSummary:
    version: int
    licensee_version: int
    header_size: int
    package_group: str
    package_flags: int
    name_count: int
    name_offset: int
    export_count: int
    export_offset: int
    import_count: int
    import_offset: int
    depends_offset: int
    engine_version: int
    cooker_version: int
    compression_flags: int
    chunks: tuple[CompressedChunk, ...]


@dataclass(frozen=True)
class ImportEntry:
    index: int
    class_package: str
    class_name: str
    package_index: int
    object_name: str


@dataclass(frozen=True)
class ExportEntry:
    index: int
    class_index: int
    super_index: int
    package_index: int
    object_name: str
    archetype_index: int
    serial_size: int
    serial_offset: int
    export_flags: int


def parse_fname(reader: Reader, names: list[str]) -> tuple[str, int]:
    name_index = reader.i32()
    number = reader.i32()
    if not 0 <= name_index < len(names):
        raise ExtractionError(f"FName index {name_index} is outside NameTable")
    name = names[name_index]
    if number > 0:
        return f"{name}_{number - 1}", number
    return name, number


def parse_summary(physical: bytes) -> PackageSummary:
    reader = Reader(physical)
    tag = reader.u32()
    if tag != PACKAGE_FILE_TAG:
        raise ExtractionError(f"wrong package tag 0x{tag:08X}")

    version, licensee_version = reader.unpack("<HH")
    header_size = reader.i32()
    package_group = reader.fstring()
    package_flags = reader.u32()
    name_count = reader.i32()
    name_offset = reader.i32()
    export_count = reader.i32()
    export_offset = reader.i32()
    import_count = reader.i32()
    import_offset = reader.i32()
    depends_offset = reader.i32() if version >= 415 else 0

    if version >= 623:
        reader.read(12)
    if version >= 584:
        reader.read(4)
    reader.read(16)  # package GUID

    generation_count = reader.i32()
    if generation_count < 0 or generation_count > 4096:
        raise ExtractionError(f"implausible generation count {generation_count}")
    reader.read(generation_count * 12)

    engine_version = reader.i32() if version >= 245 else 0
    cooker_version = reader.i32() if version >= 277 else 0
    compression_flags = reader.i32() if version >= 334 else 0
    chunk_count = reader.i32() if version >= 334 else 0
    if chunk_count <= 0 or chunk_count > 100000:
        raise ExtractionError(f"implausible Lost Ark compressed chunk count {chunk_count}")

    chunks: list[CompressedChunk] = []
    for _ in range(chunk_count):
        values = reader.unpack("<5i")
        chunk = CompressedChunk(*values)
        if (
            chunk.uncompressed_offset < 0
            or chunk.uncompressed_size <= 0
            or chunk.compressed_offset < 0
            or chunk.compressed_size <= 0
            or chunk.compressed_offset + chunk.compressed_size > len(physical)
            or chunk.encrypted_lz4 not in (0, 1)
        ):
            raise ExtractionError(f"invalid Lost Ark compressed chunk {chunk}")
        chunks.append(chunk)

    return PackageSummary(
        version=version,
        licensee_version=licensee_version,
        header_size=header_size,
        package_group=package_group,
        package_flags=package_flags,
        name_count=name_count,
        name_offset=name_offset,
        export_count=export_count,
        export_offset=export_offset,
        import_count=import_count,
        import_offset=import_offset,
        depends_offset=depends_offset,
        engine_version=engine_version,
        cooker_version=cooker_version,
        compression_flags=compression_flags,
        chunks=tuple(chunks),
    )


def decompress_chunk(
    physical: bytes,
    chunk: CompressedChunk,
    aes: WindowsAesEcbDecryptor,
) -> bytes:
    source = physical[
        chunk.compressed_offset : chunk.compressed_offset + chunk.compressed_size
    ]
    if chunk.encrypted_lz4 == 0:
        if len(source) != chunk.uncompressed_size:
            raise ExtractionError(f"uncompressed chunk has mismatched sizes: {chunk}")
        return source

    reader = Reader(source)
    tag = reader.u32()
    _stored_block_size = reader.i32()
    sum_compressed = reader.i32()
    sum_uncompressed = reader.i32()
    if tag != PACKAGE_FILE_TAG:
        raise ExtractionError(f"wrong compressed chunk tag 0x{tag:08X}")
    if sum_uncompressed != chunk.uncompressed_size:
        raise ExtractionError(
            f"compressed chunk size mismatch {sum_uncompressed} != {chunk.uncompressed_size}"
        )

    block_count = math.ceil(sum_uncompressed / 0x20000)
    block_sizes = [reader.unpack("<II") for _ in range(block_count)]
    if sum(item[0] for item in block_sizes) != sum_compressed:
        raise ExtractionError("compressed chunk block table does not match its summary")

    output = bytearray()
    for compressed_size, uncompressed_size in block_sizes:
        block = bytearray(reader.read(compressed_size))
        encrypted_size = min(4096, len(block)) & ~15
        if encrypted_size:
            block[:encrypted_size] = aes.decrypt(bytes(block[:encrypted_size]))
        output.extend(decompress_lz4_block(bytes(block), uncompressed_size))

    if len(output) != chunk.uncompressed_size:
        raise ExtractionError(
            f"chunk output mismatch {len(output)} != {chunk.uncompressed_size}"
        )
    return bytes(output)


def decompress_package(physical: bytes, summary: PackageSummary, aes_key: str) -> bytes:
    logical_size = max(
        chunk.uncompressed_offset + chunk.uncompressed_size for chunk in summary.chunks
    )
    logical = bytearray(logical_size)
    first_offset = min(chunk.uncompressed_offset for chunk in summary.chunks)
    logical[:first_offset] = physical[:first_offset]

    with WindowsAesEcbDecryptor(aes_key.encode("ascii")) as aes:
        for chunk in summary.chunks:
            start = chunk.uncompressed_offset
            end = start + chunk.uncompressed_size
            logical[start:end] = decompress_chunk(physical, chunk, aes)
    return bytes(logical)


def parse_name_table(logical: bytes, summary: PackageSummary) -> list[str]:
    reader = Reader(logical, summary.name_offset)
    names: list[str] = []
    for _ in range(summary.name_count):
        names.append(reader.fstring())
        reader.u64()
    if reader.offset != summary.import_offset:
        raise ExtractionError(
            f"NameTable ended at 0x{reader.offset:X}, expected ImportTable at "
            f"0x{summary.import_offset:X}"
        )
    return names


def parse_import_table(
    logical: bytes, summary: PackageSummary, names: list[str]
) -> list[ImportEntry]:
    reader = Reader(logical, summary.import_offset)
    imports: list[ImportEntry] = []
    for index in range(summary.import_count):
        class_package, _ = parse_fname(reader, names)
        class_name, _ = parse_fname(reader, names)
        package_index = reader.i32()
        object_name, _ = parse_fname(reader, names)
        imports.append(
            ImportEntry(index, class_package, class_name, package_index, object_name)
        )
    if reader.offset != summary.export_offset:
        raise ExtractionError(
            f"ImportTable ended at 0x{reader.offset:X}, expected ExportTable at "
            f"0x{summary.export_offset:X}"
        )
    return imports


def parse_export_table(
    logical: bytes, summary: PackageSummary, names: list[str]
) -> list[ExportEntry]:
    reader = Reader(logical, summary.export_offset)
    exports: list[ExportEntry] = []
    for index in range(summary.export_count):
        class_index = reader.i32()
        super_index = reader.i32()
        package_index = reader.i32()
        object_name, _ = parse_fname(reader, names)
        archetype_index = reader.i32()
        reader.u32()  # object flags low
        reader.u32()  # object flags high
        serial_size = reader.i32()
        serial_offset = reader.i32()
        export_flags = reader.u32()
        net_object_count = reader.i32()
        if net_object_count < 0 or net_object_count > 100000:
            raise ExtractionError(
                f"export {index} has implausible net object count {net_object_count}"
            )
        reader.read(net_object_count * 4)
        reader.read(16)  # GUID
        reader.i32()  # UE3 unknown field
        exports.append(
            ExportEntry(
                index=index,
                class_index=class_index,
                super_index=super_index,
                package_index=package_index,
                object_name=object_name,
                archetype_index=archetype_index,
                serial_size=serial_size,
                serial_offset=serial_offset,
                export_flags=export_flags,
            )
        )

    table_end = summary.depends_offset or summary.name_offset
    if reader.offset != table_end:
        raise ExtractionError(
            f"ExportTable ended at 0x{reader.offset:X}, expected 0x{table_end:X}"
        )
    return exports


def package_ref_name(
    package_index: int,
    imports: list[ImportEntry],
    exports: list[ExportEntry],
) -> str:
    if package_index < 0:
        index = -package_index - 1
        return imports[index].object_name if 0 <= index < len(imports) else "<bad-import>"
    if package_index > 0:
        index = package_index - 1
        return exports[index].object_name if 0 <= index < len(exports) else "<bad-export>"
    return "Class"


def package_ref_path(
    package_index: int,
    imports: list[ImportEntry],
    exports: list[ExportEntry],
) -> str | None:
    if package_index == 0:
        return None
    pieces: list[str] = []
    seen: set[int] = set()
    cursor = package_index
    while cursor != 0 and cursor not in seen:
        seen.add(cursor)
        if cursor < 0:
            index = -cursor - 1
            if not 0 <= index < len(imports):
                break
            entry = imports[index]
            pieces.append(entry.object_name)
            cursor = entry.package_index
        else:
            index = cursor - 1
            if not 0 <= index < len(exports):
                break
            entry = exports[index]
            pieces.append(entry.object_name)
            cursor = entry.package_index
    return ".".join(reversed(pieces))


def resolve_component_actor(
    component: ExportEntry,
    imports: list[ImportEntry],
    exports: list[ExportEntry],
) -> ExportEntry | None:
    if component.package_index <= 0:
        return None
    outer_index = component.package_index - 1
    if not 0 <= outer_index < len(exports):
        return None
    candidate = exports[outer_index]
    candidate_class = package_ref_name(
        candidate.class_index, imports, exports
    ).casefold()
    return candidate if candidate_class in PLACEMENT_ACTOR_CLASSES else None


def decode_property_value(
    property_type: str,
    struct_type: str | None,
    payload: bytes,
    names: list[str],
    bool_value: bool | None,
    property_name: str | None = None,
    owner_struct_type: str | None = None,
) -> Any:
    reader = Reader(payload)
    kind = property_type.casefold()
    structure = (struct_type or "").casefold()
    owner_structure = (owner_struct_type or "").casefold()
    property_key = (property_name or "").casefold()

    if kind == "boolproperty":
        return bool_value
    if kind in ("objectproperty", "componentproperty", "interfaceproperty") and len(payload) >= 4:
        return reader.i32()
    if kind == "floatproperty" and len(payload) >= 4:
        return reader.f32()
    if kind == "intproperty" and len(payload) >= 4:
        # Lost Ark stores an additional 8-byte numeric descriptor before the
        # declared IntProperty payload.  The actual cooked value is the final
        # signed 32-bit word.
        return struct.unpack_from("<i", payload, len(payload) - 4)[0]
    if kind == "nameproperty" and len(payload) >= 8:
        return parse_fname(reader, names)[0]
    if kind == "strproperty" and len(payload) >= 4:
        return reader.fstring()
    if kind == "byteproperty":
        if len(payload) == 1:
            return payload[0]
        if len(payload) >= 8:
            return parse_fname(reader, names)[0]
    if kind == "structproperty":
        if structure in ("vector", "vector3d") and len(payload) >= 12:
            x, y, z = reader.unpack("<3f")
            return {"x": x, "y": y, "z": z}
        if structure == "vector2d" and len(payload) >= 8:
            x, y = reader.unpack("<2f")
            return {"x": x, "y": y}
        if structure == "rotator" and len(payload) >= 12:
            pitch, yaw, roll = reader.unpack("<3i")
            return {
                "pitch": pitch,
                "yaw": yaw,
                "roll": roll,
                "degrees": {
                    "pitch": pitch * 360.0 / ROTATOR_UNITS_PER_TURN,
                    "yaw": yaw * 360.0 / ROTATOR_UNITS_PER_TURN,
                    "roll": roll * 360.0 / ROTATOR_UNITS_PER_TURN,
                },
            }
        if structure == "color" and len(payload) >= 4:
            b, g, r, a = reader.unpack("<4B")
            return {"r": r, "g": g, "b": b, "a": a}
        if structure in {
            "rawdistributionfloat",
            "rawdistributionvector",
            "interpcurvefloat",
            "interpcurvevector",
            "particlerandomseedinfo",
            "expressioninput",
            "materialinput",
            "colormaterialinput",
            "scalarmaterialinput",
            "vectormaterialinput",
            "vector2materialinput",
        }:
            try:
                nested, nested_end = parse_tagged_properties_at(
                    payload, names, 0, structure
                )
                if nested_end != len(payload):
                    raise ExtractionError(
                        f"{struct_type} left {len(payload) - nested_end} trailing bytes"
                    )
                return {"size": len(payload), "properties": nested}
            except (ExtractionError, ValueError, struct.error) as error:
                return {
                    "size": len(payload),
                    "hex": payload[:32].hex(),
                    "decodeError": str(error),
                }
    if kind == "arrayproperty" and len(payload) >= 4:
        count = reader.i32()
        if (
            owner_structure in {"rawdistributionfloat", "rawdistributionvector"}
            and property_key == "lookuptable"
            and count >= 0
            and len(payload) == 4 + count * 4
        ):
            return [reader.f32() for _ in range(count)]
        if (
            owner_structure in {"interpcurvefloat", "interpcurvevector"}
            and property_key == "points"
            and count >= 0
        ):
            points = []
            offset = reader.offset
            try:
                for _ in range(count):
                    point, point_end = parse_tagged_properties_at(
                        payload, names, offset
                    )
                    if point_end <= offset:
                        raise ExtractionError("curve point parser did not advance")
                    points.append(point)
                    offset = point_end
                if offset != len(payload):
                    raise ExtractionError(
                        f"curve points left {len(payload) - offset} trailing bytes"
                    )
                return points
            except (ExtractionError, ValueError, struct.error):
                pass
        # UE3 arrays of script structs (for example ParticleBurst and
        # EmitterDynamicParameter) store every entry as a tagged-property
        # stream. Decode that common shape before falling back to a raw array.
        if count > 0:
            items = []
            offset = reader.offset
            try:
                for _ in range(count):
                    item, item_end = parse_tagged_properties_at(
                        payload, names, offset, f"{property_key}item"
                    )
                    if item_end <= offset:
                        raise ExtractionError(
                            "tagged struct array parser did not advance"
                        )
                    items.append(item)
                    offset = item_end
                if offset != len(payload):
                    raise ExtractionError(
                        f"tagged struct array left {len(payload) - offset} "
                        "trailing bytes"
                    )
                return items
            except (ExtractionError, ValueError, struct.error):
                pass
        if count >= 0 and len(payload) == 4 + count * 4:
            return [reader.i32() for _ in range(count)]
        return {"count": count, "size": len(payload)}
    return {"size": len(payload), "hex": payload[:32].hex()}


def parse_tagged_properties_at(
    serial_data: bytes,
    names: list[str],
    start_offset: int,
    owner_struct_type: str | None = None,
) -> tuple[dict[str, Any], int]:
    reader = Reader(serial_data, start_offset)
    properties: dict[str, Any] = {}

    while reader.offset < len(serial_data):
        property_name, _ = parse_fname(reader, names)
        if property_name.casefold() == "none":
            return properties, reader.offset

        property_type, _ = parse_fname(reader, names)
        data_size = reader.i32()
        array_index = reader.i32()
        if data_size < 0 or reader.offset + data_size > len(serial_data) + 32:
            raise ExtractionError(
                f"invalid property {property_name} size {data_size} at 0x{reader.offset:X}"
            )

        struct_type: str | None = None
        bool_value: bool | None = None
        type_key = property_type.casefold()
        if type_key == "structproperty":
            struct_type, _ = parse_fname(reader, names)
        elif type_key == "boolproperty":
            bool_value = bool(reader.read(1)[0])
        elif type_key == "byteproperty":
            parse_fname(reader, names)  # enum name

        serialized_size = data_size + 8 if type_key == "intproperty" else data_size
        payload = reader.read(serialized_size)
        key = property_name if array_index == 0 else f"{property_name}[{array_index}]"
        properties[key] = {
            "type": property_type,
            "structType": struct_type,
            "value": decode_property_value(
                property_type,
                struct_type,
                payload,
                names,
                bool_value,
                property_name,
                owner_struct_type,
            ),
        }

    raise ExtractionError("tagged property stream has no None terminator")


def parse_tagged_properties(
    serial_data: bytes,
    names: list[str],
    package_version: int,
) -> tuple[dict[str, Any], int]:
    """Locate and parse the tagged-property stream after the UE3 object preamble.

    Lost Ark exports may carry an UnrealScript stack frame before the usual UE3
    NetIndex and property tags, so the preamble is not a single fixed size.  A
    valid start is identified by two consecutive valid FNames where the second
    one is a Property class, then accepted only if the complete stream reaches
    its None terminator without leaving the export buffer.
    """

    minimum_offset = 4 if package_version >= 322 else 0
    search_end = min(len(serial_data) - 20, 256)
    failures: list[str] = []
    for start_offset in range(minimum_offset, max(minimum_offset, search_end) + 1):
        try:
            name_index, name_number, type_index, type_number = struct.unpack_from(
                "<4i", serial_data, start_offset
            )
        except struct.error:
            break
        if (
            not 0 <= name_index < len(names)
            or name_number < 0
            or not 0 <= type_index < len(names)
            or type_number < 0
            or not names[type_index].casefold().endswith("property")
        ):
            continue
        try:
            return parse_tagged_properties_at(serial_data, names, start_offset)
        except ExtractionError as error:
            failures.append(f"0x{start_offset:X}: {error}")

    detail = failures[0] if failures else "no plausible first property tag"
    raise ExtractionError(f"could not locate tagged properties ({detail})")


def property_value(properties: dict[str, Any], name: str, default: Any) -> Any:
    wanted = name.casefold()
    for key, item in properties.items():
        if key.casefold() == wanted:
            return item["value"]
    return default


def vector(value: Any, default: tuple[float, float, float]) -> dict[str, float]:
    if isinstance(value, dict) and all(axis in value for axis in ("x", "y", "z")):
        return {axis: float(value[axis]) for axis in ("x", "y", "z")}
    return {"x": default[0], "y": default[1], "z": default[2]}


def rotator(value: Any) -> dict[str, Any]:
    if isinstance(value, dict) and all(axis in value for axis in ("pitch", "yaw", "roll")):
        return value
    return {
        "pitch": 0,
        "yaw": 0,
        "roll": 0,
        "degrees": {"pitch": 0.0, "yaw": 0.0, "roll": 0.0},
    }


def extract_package(
    package_path: Path,
    logical_name: str,
    aes_key: str,
) -> dict[str, Any]:
    physical = package_path.read_bytes()
    summary = parse_summary(physical)
    logical = decompress_package(physical, summary, aes_key)
    names = parse_name_table(logical, summary)
    imports = parse_import_table(logical, summary, names)
    exports = parse_export_table(logical, summary, names)

    properties_by_export: dict[int, dict[str, Any]] = {}
    property_errors: list[dict[str, Any]] = []
    relevant_classes = set(PLACEMENT_ACTOR_CLASSES) | {"staticmeshcomponent"}

    for entry in exports:
        class_name = package_ref_name(entry.class_index, imports, exports).casefold()
        if class_name not in relevant_classes or entry.serial_size <= 0:
            continue
        serial = logical[entry.serial_offset : entry.serial_offset + entry.serial_size]
        try:
            properties, _ = parse_tagged_properties(serial, names, summary.version)
            properties_by_export[entry.index] = properties
        except ExtractionError as error:
            property_errors.append(
                {
                    "exportIndex": entry.index,
                    "class": class_name,
                    "object": entry.object_name,
                    "error": str(error),
                }
            )

    placements: list[dict[str, Any]] = []
    unresolved_placements: list[dict[str, Any]] = []
    for component in exports:
        class_name = package_ref_name(component.class_index, imports, exports).casefold()
        if class_name != "staticmeshcomponent":
            continue
        component_properties = properties_by_export.get(component.index)
        if component_properties is None:
            continue
        static_mesh_ref = property_value(component_properties, "StaticMesh", 0)
        if not isinstance(static_mesh_ref, int) or static_mesh_ref == 0:
            continue

        actor = resolve_component_actor(component, imports, exports)
        if actor is None or actor.index not in properties_by_export:
            outer: ExportEntry | None = None
            if component.package_index > 0:
                outer_index = component.package_index - 1
                if 0 <= outer_index < len(exports):
                    outer = exports[outer_index]
            unresolved_placements.append(
                {
                    "placementId": f"{logical_name}:export:{component.index}",
                    "levelPackage": logical_name,
                    "reason": (
                        "unsupported-component-owner"
                        if actor is None
                        else "actor-properties-unavailable"
                    ),
                    "component": {
                        "exportIndex": component.index,
                        "objectName": component.object_name,
                        "outerReference": component.package_index,
                        "archetypePath": package_ref_path(
                            component.archetype_index, imports, exports
                        ),
                    },
                    "owner": {
                        "exportIndex": outer.index if outer else None,
                        "objectName": outer.object_name if outer else None,
                        "class": (
                            package_ref_name(outer.class_index, imports, exports)
                            if outer
                            else None
                        ),
                        "objectPath": package_ref_path(
                            component.package_index, imports, exports
                        ),
                    },
                    "asset": {
                        "packageIndex": static_mesh_ref,
                        "objectName": package_ref_name(
                            static_mesh_ref, imports, exports
                        ),
                        "objectPath": package_ref_path(
                            static_mesh_ref, imports, exports
                        ),
                    },
                }
            )
            continue

        actor_properties = properties_by_export[actor.index]
        actor_location = vector(
            property_value(actor_properties, "Location", None), (0.0, 0.0, 0.0)
        )
        actor_rotation = rotator(property_value(actor_properties, "Rotation", None))
        draw_scale = float(property_value(actor_properties, "DrawScale", 1.0))
        draw_scale3d = vector(
            property_value(actor_properties, "DrawScale3D", None), (1.0, 1.0, 1.0)
        )
        component_translation = vector(
            property_value(component_properties, "Translation", None), (0.0, 0.0, 0.0)
        )
        component_rotation = rotator(
            property_value(component_properties, "Rotation", None)
        )
        component_scale = float(property_value(component_properties, "Scale", 1.0))
        component_scale3d = vector(
            property_value(component_properties, "Scale3D", None), (1.0, 1.0, 1.0)
        )

        if actor and package_ref_name(actor.class_index, imports, exports).casefold() == "staticmeshcollectionactor":
            transform_source = "component"
            resolved_position = component_translation
            resolved_rotation = component_rotation
            resolved_scale = {
                axis: component_scale * component_scale3d[axis]
                for axis in ("x", "y", "z")
            }
        else:
            transform_source = "actor"
            resolved_position = actor_location
            resolved_rotation = actor_rotation
            resolved_scale = {
                axis: draw_scale * draw_scale3d[axis]
                for axis in ("x", "y", "z")
            }

        placements.append(
            {
                "placementId": f"{logical_name}:export:{component.index}",
                "levelPackage": logical_name,
                "actor": {
                    "exportIndex": actor.index if actor else None,
                    "objectName": actor.object_name if actor else None,
                    "class": package_ref_name(actor.class_index, imports, exports)
                    if actor
                    else None,
                    "location": actor_location,
                    "rotation": actor_rotation,
                    "drawScale": draw_scale,
                    "drawScale3D": draw_scale3d,
                },
                "component": {
                    "exportIndex": component.index,
                    "objectName": component.object_name,
                    "translation": component_translation,
                    "rotation": component_rotation,
                    "scale": component_scale,
                    "scale3D": component_scale3d,
                },
                "asset": {
                    "packageIndex": static_mesh_ref,
                    "objectName": package_ref_name(static_mesh_ref, imports, exports),
                    "objectPath": package_ref_path(static_mesh_ref, imports, exports),
                },
                "transform": {
                    "coordinateSystem": "UE3-native",
                    "source": transform_source,
                    "position": resolved_position,
                    "rotation": resolved_rotation,
                    "scale3D": resolved_scale,
                },
            }
        )

    return {
        "schemaVersion": 1,
        "source": {
            "logicalPackage": logical_name,
            "physicalPackage": str(package_path),
            "packageVersion": summary.version,
            "licenseeVersion": summary.licensee_version,
            "engineVersion": summary.engine_version,
            "nameCount": summary.name_count,
            "importCount": summary.import_count,
            "exportCount": summary.export_count,
            "compressedChunkCount": len(summary.chunks),
        },
        "summary": {
            "placementCount": len(placements),
            "uniqueStaticMeshCount": len(
                {item["asset"]["objectPath"] for item in placements}
            ),
            "propertyErrorCount": len(property_errors),
            "unresolvedPlacementCount": len(unresolved_placements),
        },
        "placements": placements,
        "propertyErrors": property_errors,
        "unresolvedPlacements": unresolved_placements,
    }


def resolve_physical_package(
    umodel: Path,
    package_root: Path,
    logical_name: str,
    region: str,
) -> Path:
    completed = subprocess.run(
        [
            str(umodel),
            "-list",
            "-game=lostark",
            f"-{region}",
            "-nameresolve",
            f"-path={package_root}",
            logical_name,
        ],
        cwd=umodel.parent,
        text=True,
        encoding="utf-8",
        errors="replace",
        capture_output=True,
        check=False,
        creationflags=subprocess.CREATE_NO_WINDOW if sys.platform == "win32" else 0,
    )
    output = completed.stdout + "\n" + completed.stderr
    match = re.search(r"Loading package:\s+([^\r\n]+?\.upk)\s+Ver:", output)
    if completed.returncode != 0 or match is None:
        raise ExtractionError(
            f"UModel could not resolve {logical_name} (exit {completed.returncode})\n"
            f"{output[-2000:]}"
        )
    path = package_root / match.group(1).strip()
    if not path.is_file():
        raise ExtractionError(f"resolved package does not exist: {path}")
    return path


def parse_args(argv: Iterable[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--umodel", type=Path, required=True)
    parser.add_argument("--package-root", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--region", default="kr", choices=("kr", "na", "ru", "jp", "tw", "cn"))
    parser.add_argument("--aes-key", default=LOSTARK_KR_AES_KEY)
    parser.add_argument("packages", nargs="+")
    return parser.parse_args(argv)


def main(argv: Iterable[str] | None = None) -> int:
    args = parse_args(argv)
    args.output.mkdir(parents=True, exist_ok=True)

    manifest_packages: list[dict[str, Any]] = []
    all_asset_paths: set[str] = set()
    total_placements = 0
    total_property_errors = 0
    total_unresolved_placements = 0
    for logical_name in args.packages:
        physical_path = resolve_physical_package(
            args.umodel, args.package_root, logical_name, args.region
        )
        result = extract_package(physical_path, logical_name, args.aes_key)
        output_path = args.output / f"{logical_name}.placements.json"
        output_path.write_text(
            json.dumps(result, ensure_ascii=False, indent=2), encoding="utf-8"
        )
        total_placements += result["summary"]["placementCount"]
        total_property_errors += result["summary"]["propertyErrorCount"]
        total_unresolved_placements += result["summary"][
            "unresolvedPlacementCount"
        ]
        all_asset_paths.update(
            item["asset"]["objectPath"] for item in result["placements"]
        )
        manifest_packages.append(
            {
                "logicalPackage": logical_name,
                "physicalPackage": physical_path.name,
                "placementFile": output_path.name,
                **result["summary"],
            }
        )
        print(
            json.dumps(
                {
                    "package": logical_name,
                    "physical": physical_path.name,
                    "placements": result["summary"]["placementCount"],
                    "uniqueStaticMeshes": result["summary"]["uniqueStaticMeshCount"],
                    "propertyErrors": result["summary"]["propertyErrorCount"],
                    "unresolvedPlacements": result["summary"][
                        "unresolvedPlacementCount"
                    ],
                    "output": str(output_path),
                },
                ensure_ascii=False,
            )
        )

    manifest = {
        "schemaVersion": 1,
        "coordinateSystem": "UE3-native",
        "packages": manifest_packages,
        "summary": {
            "packageCount": len(manifest_packages),
            "placementCount": total_placements,
            "uniqueStaticMeshCount": len(all_asset_paths),
            "propertyErrorCount": total_property_errors,
            "unresolvedPlacementCount": total_unresolved_placements,
        },
    }
    manifest_path = args.output / "placement_manifest.json"
    manifest_path.write_text(
        json.dumps(manifest, ensure_ascii=False, indent=2), encoding="utf-8"
    )
    print(json.dumps({"manifest": str(manifest_path), **manifest["summary"]}, ensure_ascii=False))
    return 1 if total_property_errors or total_unresolved_placements else 0


if __name__ == "__main__":
    raise SystemExit(main())

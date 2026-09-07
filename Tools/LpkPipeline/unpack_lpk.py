#!/usr/bin/env python3
"""Unpack a Lost Ark EFGame/*.lpk archive.

An .lpk holds the client data the .upk packages do not: the EFTable databases
(item, icon, string and customizing tables), Binaries config and the fonts.  It
is not readable by UModel, which is why every earlier table lookup in this repo
came up empty.

Layout
------
    uint32            file count
    entry[count]      512 + 16 = 528 bytes each, Blowfish-ECB encrypted
        int32         path length
        char[len]     UTF-8 path, backslash separated, e.g. "\\Binaries\\Fonts\\x.ttf"
        ...           zero padding
        int32         unpacked size      (entry end - 12)
        int32         padded size        (entry end - 8)
        int32         compressed size    (entry end - 4)
    payload           entries in index order, first at 8 + count * 528

Both the index and each packed payload use Blowfish-ECB with a per-region key,
run in the "compat" byte order the client uses: every 32-bit word is byte
swapped going in and coming back out.  A packed entry's plaintext is a zlib
stream of `compressed size` bytes.

An entry whose compressed size is zero is a database instead: no Blowfish and no
zlib, but AES-128-CBC with a zero IV over each 1024-byte block independently,
under a key derived from the entry's own file name.  That derivation is what ties
a table to its key, so `EFTable_GameMsg.db` cannot be decrypted with another
table's key.

Region keys are the community-recovered constants (leanleon93/LpkTool,
Poyoanon/lostark-explorer).  KR is the default because that is what this project
extracts from.

Usage:
  python unpack_lpk.py <archive.lpk> --out <dir> [--region KR|NAEU|CN]
                       [--filter <substring>] [--list]
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import struct
import sys
import zlib

from Crypto.Cipher import AES, Blowfish

ENTRY_SIZE = 528
DB_BLOCK_SIZE = 1024

REGIONS = {
    "NAEU": ("83657ea6ffa1e671375c689a2e99a598", "1069d88738c5c75f82b44a1f0a382768"),
    "KR": ("e3235b518eb04c8ef81b938839f2fea7", "b5c3862e3e51b8863090694c528cb396"),
    "CN": ("a2e587d2fac8d6836cad3fd144f0cf2b", "c7db380e7d8d9e4771c7eaf5b1f3a78c"),
}


class LpkError(RuntimeError):
    pass


def swap32(data: bytes) -> bytes:
    """The client's Blowfish runs over byte-swapped 32-bit words."""
    if len(data) % 4:
        raise LpkError(f"swap32 needs a multiple of 4 bytes, got {len(data)}")
    out = bytearray(len(data))
    for offset in range(0, len(data), 4):
        out[offset : offset + 4] = data[offset : offset + 4][::-1]
    return bytes(out)


def blowfish_decrypt(data: bytes, key: bytes) -> bytes:
    if len(data) % 8:
        raise LpkError(f"Blowfish needs a multiple of 8 bytes, got {len(data)}")
    return swap32(Blowfish.new(key, Blowfish.MODE_ECB).decrypt(swap32(data)))


def database_key(entry_path: str, base_key: bytes) -> bytes:
    """A table's AES key comes from its own name, so keys are not interchangeable."""
    stem = Path(entry_path.replace("\\", "/")).stem
    name = stem[len("EFTable_") :] if stem.startswith("EFTable_") else stem
    digest = hashlib.md5(name.encode("utf-16-le")).digest()
    mixed = bytes(base_key[i] ^ digest[15 - i] for i in range(16))
    return hashlib.sha256(mixed.hex().encode("ascii")).digest()


def decrypt_database(payload: bytes, entry_path: str, base_key: bytes) -> bytes:
    key = database_key(entry_path, base_key)
    out = bytearray()
    for offset in range(0, len(payload), DB_BLOCK_SIZE):
        chunk = payload[offset : offset + DB_BLOCK_SIZE]
        if len(chunk) % 16:
            raise LpkError(f"database block at 0x{offset:x} is not AES sized: {len(chunk)}")
        # Each block restarts from the zero IV; they are not chained together.
        out += AES.new(key, AES.MODE_CBC, b"\0" * 16).decrypt(chunk)
    return bytes(out)


def read_index(raw: bytes, key: bytes) -> list[dict]:
    if len(raw) < 4:
        raise LpkError("archive is too small to hold a file count")
    count = struct.unpack_from("<I", raw, 0)[0]
    index_size = count * ENTRY_SIZE
    if count <= 0 or 4 + index_size > len(raw):
        raise LpkError(f"implausible file count {count}")
    index = blowfish_decrypt(raw[4 : 4 + index_size], key)

    entries: list[dict] = []
    # The payload starts one 4-byte field past the index; the client reads it as
    # count * ENTRY_SIZE + 8 from the start of the file.
    offset = index_size + 8
    for i in range(count):
        record = index[i * ENTRY_SIZE : (i + 1) * ENTRY_SIZE]
        length = struct.unpack_from("<i", record, 0)[0]
        if not 0 < length <= ENTRY_SIZE - 16:
            raise LpkError(
                f"entry {i} has an invalid path length {length}; wrong region key?"
            )
        path = record[4 : 4 + length].decode("utf-8", "replace")
        unpacked, padded, compressed = struct.unpack_from("<3i", record, ENTRY_SIZE - 12)
        entries.append(
            {
                "index": i,
                "path": path,
                "offset": offset,
                "unpacked": unpacked,
                "padded": padded,
                "compressed": compressed,
            }
        )
        offset += padded
    return entries


def extract(raw: bytes, entry: dict, key: bytes, base_key: bytes) -> bytes:
    stored = raw[entry["offset"] : entry["offset"] + entry["padded"]]
    if len(stored) != entry["padded"]:
        raise LpkError(f"{entry['path']}: payload runs past the end of the archive")
    if entry["compressed"] == 0:
        return decrypt_database(stored, entry["path"], base_key)
    packed = blowfish_decrypt(stored, key)[: entry["compressed"]]
    data = zlib.decompress(packed)
    if entry["unpacked"] and len(data) != entry["unpacked"]:
        raise LpkError(
            f"{entry['path']}: unpacked {len(data)} bytes, index says {entry['unpacked']}"
        )
    return data


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("archive", type=Path)
    parser.add_argument("--out", type=Path)
    parser.add_argument("--region", default="KR", choices=sorted(REGIONS))
    parser.add_argument("--filter", default="", help="only entries whose path contains this")
    parser.add_argument("--list", action="store_true", help="list without extracting")
    args = parser.parse_args()

    key_hex, base_hex = REGIONS[args.region]
    key = key_hex.encode("latin-1")
    base_key = bytes.fromhex(base_hex)

    raw = args.archive.read_bytes()
    entries = read_index(raw, key)
    wanted = [e for e in entries if args.filter.lower() in e["path"].lower()]
    print(f"{args.archive.name}: {len(entries)} entries, {len(wanted)} selected")

    if args.list or args.out is None:
        for e in wanted:
            kind = "db" if e["compressed"] == 0 else "packed"
            print(f"  {e['index']:5d} {kind:6s} {e['unpacked']:>10d}  {e['path']}")
        return 0

    failures = 0
    for e in wanted:
        relative = e["path"].replace("\\", "/").lstrip("./")
        target = args.out / relative
        try:
            data = extract(raw, e, key, base_key)
        except (LpkError, zlib.error) as error:
            # One bad entry must not lose the rest of the archive.
            print(f"  FAIL {e['path']}: {error}", file=sys.stderr)
            failures += 1
            continue
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
    print(f"extracted {len(wanted) - failures} files to {args.out}"
          + (f", {failures} failed" if failures else ""))
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())

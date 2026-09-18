#!/usr/bin/env python3
"""Read the Lost Ark Wwise audio packages under
EFGame/ReleasePC/WwiseAudioPackage (*.pck and the loose streamed *.wem).

Container
---------
Every file starts with the 4-byte marker ``3E CE A6 74``. Everything after it
is XORed with one keystream that is identical for every file and repeats every
435,540 bytes; the marker itself is not part of the stream. The client's own
decryption routine is unreachable (LOSTARK.exe and EFEngine.dll are wrapped by
WinLicense, sections ``.winlice``/``.vm_sec``), so the table in
``wwise_keystream.dat`` was recovered from the shipped data instead --
``recover_wwise_keystream.py`` regenerates and re-verifies it.

Once decrypted a .pck is a stock Audiokinetic AKPK (header, language map,
soundbank LUT, streamed-file LUT, then the payloads) and each payload is a
stock .bnk (BKHD/HIRC) or Wwise RIFF .wem. Event names are not stored; an event
object's ID is the FNV-1 32-bit hash of its lowercased name, which is what lets
a known name such as ``S_Vehicle_TrisionHorse_Dash1`` be resolved to the .wem
files it plays.

The .wem payloads are Wwise Vorbis with stripped setup packets; this module only
decrypts, indexes and extracts. ``wwise_vorbis_to_ogg`` turns one into playable
Ogg Vorbis, and ``render_events`` chains the two.
"""
from __future__ import annotations

import argparse
import os
import struct
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO_ROOT / "Tools" / "MoviePipeline"))
from deobfuscate_names import decode as decode_package_name  # noqa: E402

CONTAINER_MAGIC = b"\x3e\xce\xa6\x74"
KEYSTREAM_PATH = Path(__file__).resolve().parent / "wwise_keystream.dat"
KEYSTREAM_PERIOD = 435540

# The launcher installs wherever the user pointed it, so the install is searched
# for rather than assumed. LOSTARK_PACKAGE_ROOT wins when it is set.
PACKAGE_SUFFIX = Path("EFGame/ReleasePC/WwiseAudioPackage")
KNOWN_INSTALL_ROOTS = (
    Path("C:/ProgramData/Smilegate/Games/LOSTARK"),
    Path("D:/Games/LOSTARK"),
    Path("C:/Games/LOSTARK"),
    Path("D:/Smilegate/Games/LOSTARK"),
)


def _default_package_root() -> Path:
    override = os.environ.get("LOSTARK_PACKAGE_ROOT")
    if override:
        return Path(override)
    for root in KNOWN_INSTALL_ROOTS:
        candidate = root / PACKAGE_SUFFIX
        if candidate.is_dir():
            return candidate
    return KNOWN_INSTALL_ROOTS[0] / PACKAGE_SUFFIX


DEFAULT_PACKAGE_ROOT = _default_package_root()

# AkActionType high byte; only Play reaches the audio an event actually starts.
ACTION_PLAY = 0x04

HIRC_SOUND = 2
HIRC_ACTION = 3
HIRC_EVENT = 4
HIRC_RANDOM_SEQUENCE = 5
HIRC_SWITCH = 6
HIRC_ACTOR_MIXER = 7
HIRC_LAYER = 9
HIRC_CONTAINERS = (HIRC_RANDOM_SEQUENCE, HIRC_SWITCH, HIRC_ACTOR_MIXER, HIRC_LAYER)

_keystream: bytes | None = None


class WwiseError(RuntimeError):
    pass


def keystream() -> bytes:
    global _keystream
    if _keystream is None:
        if not KEYSTREAM_PATH.is_file():
            raise WwiseError(
                f"{KEYSTREAM_PATH} is missing; run recover_wwise_keystream.py"
            )
        data = KEYSTREAM_PATH.read_bytes()
        if len(data) != KEYSTREAM_PERIOD:
            raise WwiseError(
                f"{KEYSTREAM_PATH} is {len(data)} bytes, expected {KEYSTREAM_PERIOD}"
            )
        _keystream = data
    return _keystream


def decrypt(body: bytes, stream_offset: int = 0) -> bytes:
    """Decrypt ``body``, which starts ``stream_offset`` bytes into the payload
    (payload offset 0 is the byte right after the 4-byte marker)."""
    ks = keystream()
    start = stream_offset % KEYSTREAM_PERIOD
    pad = ks[start:] + ks * (len(body) // KEYSTREAM_PERIOD + 2)
    return bytes(a ^ b for a, b in zip(body, pad))


def decrypt_file(path: Path) -> bytes:
    raw = path.read_bytes()
    if raw[:4] != CONTAINER_MAGIC:
        raise WwiseError(f"{path.name}: not a Lost Ark Wwise container")
    return decrypt(raw[4:])


def fnv1_32(name: str) -> int:
    """Wwise hashes object names as FNV-1 over the lowercased ASCII name."""
    h = 2166136261
    for c in name.lower().encode("utf-8"):
        h = (h * 16777619) & 0xFFFFFFFF
        h ^= c
    return h


class AudioPackage:
    """One decrypted AKPK package."""

    def __init__(self, path: Path):
        self.path = Path(path)
        self.name = decode_package_name(self.path.stem.split(".")[0])
        self.data = decrypt_file(self.path)
        if self.data[:4] != b"AKPK":
            raise WwiseError(f"{self.path.name}: decrypted header is not AKPK")
        header_size, self.version, language_map_size, banks_size, streams_size, _ = (
            struct.unpack_from("<6I", self.data, 4)
        )
        self.header_size = header_size
        self.languages = self._read_language_map(0x1C)
        self.banks = self._read_lut(0x1C + language_map_size)
        self.streams = self._read_lut(0x1C + language_map_size + banks_size)

    def _read_language_map(self, base: int) -> dict[int, str]:
        out = {}
        count = struct.unpack_from("<I", self.data, base)[0]
        for i in range(count):
            offset, language_id = struct.unpack_from("<II", self.data, base + 4 + 8 * i)
            raw = self.data[base + offset : base + offset + 64]
            text = raw.split(b"\x00\x00")[0].decode("utf-16-le", "ignore")
            out[language_id] = text.rstrip("\x00")
        return out

    def _read_lut(self, base: int) -> list[dict]:
        out = []
        count = struct.unpack_from("<I", self.data, base)[0]
        for i in range(count):
            file_id, block_size, size, start_block, language_id = struct.unpack_from(
                "<5I", self.data, base + 4 + 20 * i
            )
            out.append(
                {
                    "id": file_id,
                    "size": size,
                    "offset": start_block * (block_size or 1),
                    "language": language_id,
                }
            )
        return out

    def payload(self, entry: dict) -> bytes:
        return self.data[entry["offset"] : entry["offset"] + entry["size"]]

    def stream_by_id(self, file_id: int) -> dict | None:
        for entry in self.streams:
            if entry["id"] == file_id:
                return entry
        return None


def hirc_objects(bank: bytes) -> dict[int, tuple[int, bytes]]:
    """id -> (hierarchy type, payload after the id) for one .bnk."""
    objects: dict[int, tuple[int, bytes]] = {}
    offset = 0
    while offset + 8 <= len(bank):
        chunk_id = bank[offset : offset + 4]
        chunk_size = struct.unpack_from("<I", bank, offset + 4)[0]
        if chunk_id == b"HIRC":
            cursor = offset + 8
            count = struct.unpack_from("<I", bank, cursor)[0]
            cursor += 4
            for _ in range(count):
                hirc_type = bank[cursor]
                section_size = struct.unpack_from("<I", bank, cursor + 1)[0]
                object_id = struct.unpack_from("<I", bank, cursor + 5)[0]
                objects[object_id] = (
                    hirc_type,
                    bank[cursor + 9 : cursor + 5 + section_size],
                )
                cursor += 5 + section_size
        offset += 8 + chunk_size
    return objects


def event_action_ids(payload: bytes) -> list[int]:
    count = payload[0]
    if 1 + 4 * count == len(payload):
        return list(struct.unpack_from("<%dI" % count, payload, 1))
    count = struct.unpack_from("<I", payload, 0)[0]
    return list(struct.unpack_from("<%dI" % count, payload, 4))


def action_fields(payload: bytes) -> tuple[int, int]:
    """(AkActionType high byte, target object id)."""
    action_type = struct.unpack_from("<H", payload, 0)[0]
    return action_type >> 8, struct.unpack_from("<I", payload, 2)[0]


def sound_source_id(payload: bytes) -> int:
    """AkBankSourceData: plugin id, stream type, then the media (wem) id."""
    return struct.unpack_from("<I", payload, 5)[0]


def container_children(payload: bytes, objects: dict[int, tuple[int, bytes]]) -> list[int]:
    """Locate a container's child-id list.

    The list sits after NodeBaseParams, whose length depends on how many
    property and RTPC entries the object carries, so the position is searched
    for rather than computed: a count followed by that many distinct ids that
    all resolve to real objects. The longest such run wins, which keeps the
    child list from being mistaken for the shorter playlist that follows it.
    """
    best: list[int] = []
    for position in range(0, max(len(payload) - 4, 0)):
        count = struct.unpack_from("<I", payload, position)[0]
        if not 1 <= count <= 512 or position + 4 + 4 * count > len(payload):
            continue
        if count <= len(best):
            continue
        ids = struct.unpack_from("<%dI" % count, payload, position + 4)
        if len(set(ids)) == count and all(i in objects for i in ids):
            best = list(ids)
    return best


def switch_branches(payload: bytes, children: list[int]) -> list[tuple[int, list[int]]]:
    """A Switch container's (switch value id, node ids) map.

    It follows the child list, so the child list is located first and the map
    read from just past it: a group count, then per group a switch id, an item
    count and that many node ids.
    """
    count = len(children)
    if count == 0:
        return []
    for position in range(0, max(len(payload) - 4, 0)):
        if struct.unpack_from("<I", payload, position)[0] != count:
            continue
        if struct.unpack_from("<%dI" % count, payload, position + 4) != tuple(children):
            continue
        cursor = position + 4 + 4 * count
        if cursor + 4 > len(payload):
            continue
        groups = struct.unpack_from("<I", payload, cursor)[0]
        if not 1 <= groups <= 64:
            continue
        cursor += 4
        out: list[tuple[int, list[int]]] = []
        for _ in range(groups):
            if cursor + 8 > len(payload):
                return []
            switch_id, items = struct.unpack_from("<II", payload, cursor)
            if items > 64 or cursor + 8 + 4 * items > len(payload):
                return []
            out.append((switch_id, list(struct.unpack_from("<%dI" % items, payload, cursor + 8))))
            cursor += 8 + 4 * items
        return out
    return []


def collect_sound_sources(
    object_id: int,
    objects: dict[int, tuple[int, bytes]],
    seen: set[int] | None = None,
    switch_value: str | None = None,
) -> list[int]:
    """Every .wem id reachable from one hierarchy object.

    ``switch_value`` names the branch to take through Switch containers, e.g.
    the floor material a footstep is standing on. Without it every branch is
    collected, which for a footstep means every surface at once. A Switch that
    does not declare the requested value falls back to all of its branches.
    """
    seen = set() if seen is None else seen
    if object_id in seen or object_id not in objects:
        return []
    seen.add(object_id)
    hirc_type, payload = objects[object_id]
    if hirc_type == HIRC_SOUND:
        return [sound_source_id(payload)]
    if hirc_type not in HIRC_CONTAINERS:
        return []
    children = container_children(payload, objects)
    if hirc_type == HIRC_SWITCH and switch_value is not None:
        wanted = fnv1_32(switch_value)
        for switch_id, nodes in switch_branches(payload, children):
            if switch_id == wanted and nodes:
                children = nodes
                break
    out: list[int] = []
    for child in children:
        out += collect_sound_sources(child, objects, seen, switch_value)
    return out


def resolve_event(
    event_name: str,
    objects: dict[int, tuple[int, bytes]],
    switch_value: str | None = None,
) -> tuple[list[int], list[int]]:
    """(.wem ids the event plays, target ids that are not in ``objects``).

    Only Play actions are followed. A Stop-only event such as
    ``S_Vehicle_AncientDragon1_Stop1`` legitimately resolves to nothing.
    ``switch_value`` is passed down to Switch containers.
    """
    event_id = fnv1_32(event_name)
    entry = objects.get(event_id)
    if entry is None or entry[0] != HIRC_EVENT:
        raise KeyError(event_name)
    sources: list[int] = []
    unresolved: list[int] = []
    for action_id in event_action_ids(entry[1]):
        action = objects.get(action_id)
        if action is None or action[0] != HIRC_ACTION:
            continue
        action_type, target = action_fields(action[1])
        if action_type != ACTION_PLAY:
            continue
        if target not in objects:
            unresolved.append(target)
            continue
        for source in collect_sound_sources(target, objects, None, switch_value):
            if source not in sources:
                sources.append(source)
    return sources, unresolved


def load_packages(paths) -> list[AudioPackage]:
    return [AudioPackage(Path(p)) for p in paths]


def merged_objects(packages) -> dict[int, tuple[int, bytes]]:
    objects: dict[int, tuple[int, bytes]] = {}
    for package in packages:
        for bank in package.banks:
            objects.update(hirc_objects(package.payload(bank)))
    return objects


def find_packages(root: Path, name_filter: str = "") -> list[Path]:
    """Package paths whose deobfuscated name contains ``name_filter``."""
    out = []
    for path in sorted(root.rglob("*.pck")):
        name = decode_package_name(path.stem.split(".")[0])
        if name_filter.upper() in name.upper():
            out.append(path)
    return out


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--package-root", type=Path, default=DEFAULT_PACKAGE_ROOT)
    parser.add_argument("--filter", default="", help="deobfuscated package name substring")
    parser.add_argument("--list", action="store_true", help="list matching packages")
    parser.add_argument("--event", action="append", default=[], help="resolve an event name")
    parser.add_argument("--extract", type=Path, help="write resolved .wem files here")
    parser.add_argument("--switch", help="Switch container branch to take, e.g. stone")
    args = parser.parse_args(argv)

    paths = find_packages(args.package_root, args.filter)
    if not paths:
        print(f"no package matched {args.filter!r} under {args.package_root}")
        return 1
    if args.list:
        for path in paths:
            package = AudioPackage(path)
            print(
                "%-40s banks=%-4d streams=%-6d %s"
                % (package.name, len(package.banks), len(package.streams), path.name)
            )
        return 0

    packages = load_packages(paths)
    objects = merged_objects(packages)
    print("%d package(s), %d hierarchy objects" % (len(packages), len(objects)))
    for name in args.event:
        try:
            sources, unresolved = resolve_event(name, objects, args.switch)
        except KeyError:
            print("%-40s EVENT NOT FOUND" % name)
            continue
        print("%-40s sources=%d unresolved=%d" % (name, len(sources), len(unresolved)))
        if args.extract:
            args.extract.mkdir(parents=True, exist_ok=True)
            for source in sources:
                for package in packages:
                    entry = package.stream_by_id(source)
                    if entry is None:
                        continue
                    out = args.extract / ("%d.wem" % source)
                    out.write_bytes(package.payload(entry))
                    break
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

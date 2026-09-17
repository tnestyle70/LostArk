#!/usr/bin/env python3
"""Parse a retail MinimapData.loa (leveldata1.lpk, Common_Extra/MapData/<zone>/) into JSON.

Layout (little endian, all strings are int32 length-prefixed and NUL terminated):
  float32 version (1.0), int32 unknown (5), int32 zoneId, int32 zoneId, int32 volumeCount
  per volume:
    string  className ("CEFMinimapVolume")
    int32   volumeIndex, columns, rows, tileSizeCm
    string  package        (EFMinimap_<continent>, the Texture2D package)
    string  textureBase    (LV_..._PS)
    string  fullImage      (LV_..._PS_<vol>_Full, the whole-volume overview texture)
    int32   tileCount
    per tile:
      int32 x3    tile index (0-based), 0, 0
      string name (<textureBase>_<vol>_<col>x<row>)
      float32 x6  world box in retail cm: minX minY minZ maxX maxY maxZ
    float32 x6  volume box (union of the tiles)
    tail        int32 -1, flag ints, optional floor-name string (tip.name.floor_...), up to the next
                "CEFMinimapVolume" record (kept raw as tailInts / floorName)
Tile "AxB": A runs along retail X (descending: 0 = max X), B along retail Y (ascending).
Retail cm -> client metres: X = x / 100, Z = -y / 100.

Usage: python parse_minimap_data.py <MinimapData.loa> [--out zone.json]
"""
from __future__ import annotations

import argparse
import json
import struct
from pathlib import Path


class Reader:
    def __init__(self, data: bytes):
        self.b = data
        self.o = 0

    def i32(self) -> int:
        v = struct.unpack_from("<i", self.b, self.o)[0]
        self.o += 4
        return v

    def f32(self) -> float:
        v = struct.unpack_from("<f", self.b, self.o)[0]
        self.o += 4
        return v

    def s(self) -> str:
        n = self.i32()
        v = self.b[self.o:self.o + n].rstrip(b"\0").decode("cp949", "replace")
        self.o += n
        return v


def parse(path: Path) -> dict:
    data = path.read_bytes()
    r = Reader(data)
    doc = {"version": r.f32(), "unknown": r.i32(), "zoneId": r.i32(), "zoneId2": r.i32(),
           "volumeCountField": r.i32(), "volumes": []}
    marker = b"CEFMinimapVolume"
    while True:
        start = data.find(marker, r.o)
        if start < 0:
            break
        r.o = start - 4
        vol = {"className": r.s(), "volumeIndex": r.i32(), "columns": r.i32(), "rows": r.i32(),
               "tileSizeCm": r.i32(), "package": r.s(), "textureBase": r.s(), "fullImage": r.s()}
        tile_count = r.i32()
        tiles = []
        for _ in range(tile_count):
            head = [r.i32() for _ in range(3)]
            name = r.s()
            box = [r.f32() for _ in range(6)]
            tiles.append({"index": head[0], "head": head[1:], "name": name, "minCm": box[:3], "maxCm": box[3:]})
        vol["tiles"] = tiles
        vbox = [r.f32() for _ in range(6)]
        vol["boundsCm"] = {"min": vbox[:3], "max": vbox[3:]}
        nxt = data.find(marker, r.o)
        tail_end = (nxt - 4) if nxt >= 0 else len(data)
        tail = data[r.o:tail_end]
        ints = [struct.unpack_from("<i", tail, k)[0] for k in range(0, len(tail) - len(tail) % 4, 4)]
        vol["tailInts"] = ints[:12]
        floor = None
        k = tail.find(b"tip.name.floor")
        if k >= 0:
            floor = tail[k:tail.find(bytes([0]), k)].decode("ascii", "replace")
        vol["floorName"] = floor
        r.o = tail_end
        doc["volumes"].append(vol)
    return doc


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("loa", type=Path)
    ap.add_argument("--out", type=Path)
    a = ap.parse_args()
    doc = parse(a.loa)
    text = json.dumps(doc, ensure_ascii=False, indent=1)
    if a.out:
        a.out.parent.mkdir(parents=True, exist_ok=True)
        a.out.write_text(text + "\n", encoding="utf-8")
    for v in doc["volumes"]:
        b = v.get("boundsCm", {})
        mn, mx = b["min"], b["max"]
        print(f"zone {doc['zoneId']} vol {v['volumeIndex']}: {v['columns']}x{v['rows']} tiles of {v['tileSizeCm']}cm "
              f"({len(v['tiles'])} tiles), full {v['fullImage']}, x {mn[0]:.0f}..{mx[0]:.0f} y {mn[1]:.0f}..{mx[1]:.0f} "
              f"z {mn[2]:.0f}..{mx[2]:.0f}, floor {v['floorName']}, tail {v['tailInts'][:9]}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

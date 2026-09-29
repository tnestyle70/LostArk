#!/usr/bin/env python3
"""Add the retail ship supply capacity to Data/Actors/VehicleCatalog.json.

The ocean HUD's dome (oceanhud.gfx OceanSupplieGauge) is the ship *supply* gauge: current /
EFTable_VoyageShip.MaxSupply. The project has no supply consumption, so the HUD shows the level 1
capacity as a full gauge. This inserts an integer "maxSupply" line under every `"ship": true` row of
the catalog (text insertion, other bytes untouched, idempotent).

  python add_ship_max_supply.py [--tables <TableData dir>] [--apply]
"""
from __future__ import annotations

import argparse
import re
import sqlite3
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
CATALOG = REPO / "Data/Actors/VehicleCatalog.json"
DEFAULT_TABLES = REPO / "out/Bern3Ship20260925/lpk/EFGame_Extra/ClientData/TableData"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--tables", type=Path, default=DEFAULT_TABLES)
    parser.add_argument("--apply", action="store_true")
    args = parser.parse_args()
    con = sqlite3.connect(str(args.tables / "EFTable_VoyageShip.db"))
    supply = {}
    for primary, secondary, max_supply in con.execute("select PrimaryKey, SecondaryKey, MaxSupply from VoyageShip order by PrimaryKey, SecondaryKey"):
        supply.setdefault(primary, (secondary, max_supply))  # the lowest SecondaryKey is level 1
    raw = CATALOG.read_bytes()
    text = raw.decode("utf-8")
    pattern = re.compile(r'(?P<head>"vehicleId": (?P<id>\d+),(?P<nl>\r?\n)(?P<indent>[ \t]+)"archetypeId": "[^"]*",\r?\n[ \t]+"ship": true,)(?P<tail>\r?\n)')
    count = 0
    def repl(match):
        nonlocal count
        vehicle = int(match.group("id"))
        if vehicle not in supply:
            raise SystemExit("EFTable_VoyageShip has no ship %d" % vehicle)
        block_end = text.find('"runtimeStatus"', match.end())
        if '"maxSupply"' in text[match.end():block_end]:
            return match.group(0)
        count += 1
        secondary, value = supply[vehicle]
        print("ship %d level %d MaxSupply %d" % (vehicle, secondary, value))
        return match.group("head") + match.group("tail") + match.group("indent") + '"maxSupply": %d,' % value + match.group("tail")
    patched = pattern.sub(repl, text)
    print("rows to patch:", count)
    if args.apply and count:
        CATALOG.write_bytes(patched.encode("utf-8"))
        import json
        json.loads(CATALOG.read_text(encoding="utf-8"))
        print("written", CATALOG.name)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

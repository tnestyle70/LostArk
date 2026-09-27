#!/usr/bin/env python3
"""Add the Bern harbor ship NPCs: NpcCatalog archetypes + Gameplay.world.json placements.

Placements stand on walkable Bern3 navigation cells 3-5 m from the two ESTOCSHIP01 harbor ship
meshes of LV_BER_BERNCASTLE_SL02 (278.6, -201.1) and (225.4, -204.7); the cells and heights come from
the published Bern3 navgrid. The NPC that opens the ship list is picked in CLevel_Bern by the archetype
prefix NPC_SHIP_, so a new ship NPC only needs a catalog row and a placement.

The two archetypes use retail models cooked by Tools/ShipPipeline/cook_npc.py: the Astray
shipwright (EFTable_Npc 19991, MN_RHKP_02-2: body MN_RHKP_00 + head MN_Head_MA04_012) and the voyage
liner NPC (NP_LRKK_01: body NP_LRKK_00 + head Head_MA02_001). Weapons are not attached.

Both destinations are merged in place and skipped when the ids already exist:
  Data/Actors/NpcCatalog.json                       (parsed, appended, written back: round trip is exact)
  Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json (text insertion, revision + 1)
"""

from __future__ import annotations

import json
import os
import re
import shutil
import tempfile
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
BACKUP = REPO / "out" / "Bern3Ship20260925" / "backup"

ARCHETYPES = [
    {"archetypeId": "NPC_SHIP_SHIPWRIGHT", "clientPresentationId": "npc.bern.ship.shipwright.client.v1",
     "modelAssetId": "Character/NPC/Npc_MN_RHKP_02_2/Npc_MN_RHKP_02_2.wmodel", "animationSetId": None,
     "idleClip": "idle_normal_1_1", "runtimeStatus": "supported"},
    {"archetypeId": "NPC_SHIP_HARBORMASTER", "clientPresentationId": "npc.bern.ship.harbormaster.client.v1",
     "modelAssetId": "Character/NPC/Npc_NP_LRKK_01/Npc_NP_LRKK_01.wmodel", "animationSetId": None,
     "idleClip": "idle_normal_1", "runtimeStatus": "supported"},
]

# placementId, archetype, position (x, y, z metres), yaw degrees (faces the ship)
PLACEMENTS = [
    ("npc.bern.ship.shipwright", "NPC_SHIP_SHIPWRIGHT", (283.25, 12.89, -200.25), 259.6),
    ("npc.bern.ship.harbormaster", "NPC_SHIP_HARBORMASTER", (225.25, 13.0, -208.25), 2.4),
]


def atomic_write(path: Path, data: bytes) -> None:
    handle, temp = tempfile.mkstemp(dir=str(path.parent), prefix=path.name + ".", suffix=".tmp")
    with os.fdopen(handle, "wb") as stream:
        stream.write(data)
    os.replace(temp, path)


def backup(path: Path) -> None:
    destination = BACKUP / path.relative_to(REPO)
    if not destination.exists():
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(path, destination)


def main() -> None:
    catalog_path = REPO / "Data" / "Actors" / "NpcCatalog.json"
    catalog = json.loads(catalog_path.read_text(encoding="utf-8"))
    by_id = {row["archetypeId"]: index for index, row in enumerate(catalog["npcs"])}
    fresh, changed = [], False
    for row in ARCHETYPES:
        if row["archetypeId"] not in by_id:
            fresh.append(row)
        elif catalog["npcs"][by_id[row["archetypeId"]]] != row:
            catalog["npcs"][by_id[row["archetypeId"]]] = row  # the model of an existing ship NPC row was swapped
            changed = True
    for row in ARCHETYPES:
        if not (REPO / "Client/Bin/Resources" / row["modelAssetId"]).is_file():
            raise SystemExit(f"model missing: {row['modelAssetId']}")
    if fresh or changed:
        backup(catalog_path)
        catalog["npcs"].extend(fresh)
        text = json.dumps(catalog, indent=2, ensure_ascii=True).replace("\n", "\r\n") + "\r\n"
        atomic_write(catalog_path, text.encode("utf-8"))
    print("NpcCatalog.json: added", len(fresh), "updated", int(changed))

    world_path = REPO / "Data" / "Worlds" / "LV_BER_BERNCASTLE" / "Gameplay.world.json"
    raw = world_path.read_bytes().decode("utf-8")
    existing = set(re.findall(r'"placementId": "([^"]+)"', raw))
    blocks = []
    for placement_id, archetype, (x, y, z), yaw in PLACEMENTS:
        if placement_id in existing:
            continue
        blocks.append(
            "    {\r\n"
            f'      "placementId": "{placement_id}",\r\n'
            '      "kind": "npc",\r\n'
            f'      "archetypeId": "{archetype}",\r\n'
            '      "encounterId": null,\r\n'
            '      "idleClip": null,\r\n'
            '      "behavior": null,\r\n'
            f'      "position": [{x}, {y}, {z}],\r\n'
            f'      "yawDegrees": {yaw},\r\n'
            '      "enabled": true\r\n'
            "    }")
    if blocks:
        tail = "\r\n  ]\r\n}\r\n"
        if not raw.endswith(tail):
            raise SystemExit("Gameplay.world.json: unexpected ending")
        match = re.search(r'"revision": (\d+),', raw)
        if not match:
            raise SystemExit("Gameplay.world.json: revision not found")
        backup(world_path)
        body = raw[:-len(tail)] + ",\r\n" + ",\r\n".join(blocks) + tail
        body = body.replace(match.group(0), f'"revision": {int(match.group(1)) + 1},', 1)
        atomic_write(world_path, body.encode("utf-8"))
    print("Gameplay.world.json: added", len(blocks))


if __name__ == "__main__":
    main()

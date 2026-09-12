#!/usr/bin/env python3
"""Build Data/UI/MVP/MvpContentNames.json -- the award page's headline line.

The line above the MVP reads "[노말] 한밤중의 서커스 3관문" and is three separate
pieces, not one string:

  difficulty  sys.mvp.difficulty_<id>.  Most carry their own <FONT COLOR>;
              [노말] is the only one with none, so it takes the field colour.
  raid name   keyed by EFTable_ZoneEpicGate.GroupId, so Valtan (101) and
              KoukuSaydon (103) resolve from the same lookup.
  gate        sys.mvp.gate_num_<n>, always #A9D0F5.

contentNameTF itself is #ffffff (its __setProp overrides the authored #f2d694),
so a piece with no colour of its own is drawn white -- which is why the raid
name is white on the reference capture while the gate is blue.

The raid names come from sys.preset.combined_list_commander_<GroupId>, the only
string keyed by GroupId.  That key wraps its text in #84aecb for the preset
list; the colour is dropped here because this page does not use it.

Usage:
  python build_mvp_content_names.py --gamemsg <EFTable_GameMsg.db>
                                    --out Data/UI/MVP/MvpContentNames.json
"""

from __future__ import annotations

import argparse
import json
import re
import sqlite3
import sys
from pathlib import Path

COLOR = re.compile(r"<FONT\s+COLOR\s*=\s*'([^']+)'", re.IGNORECASE)
TAG = re.compile(r"<[^>]+>")


def split_colour(markup: str) -> tuple[str, str | None]:
    """The text with its markup removed, and the colour the markup asked for."""
    match = COLOR.search(markup or "")
    return TAG.sub("", markup or "").strip(), match.group(1) if match else None


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--gamemsg", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()

    db = sqlite3.connect(f"file:{args.gamemsg.as_posix()}?mode=ro", uri=True)

    difficulties = []
    for key, markup in db.execute(
            "select KEY, MSG from GameMsg "
            "where KEY like 'sys.mvp.difficulty_%' order by KEY"):
        text, colour = split_colour(markup)
        difficulties.append({
            "id": key[len("sys.mvp.difficulty_"):],
            "text": text,
            "color": colour,
        })

    gates = []
    for key, markup in db.execute(
            "select KEY, MSG from GameMsg "
            "where KEY like 'sys.mvp.gate_num_%' order by KEY"):
        text, colour = split_colour(markup)
        gates.append({
            "gate": int(key[len("sys.mvp.gate_num_"):]),
            "text": text,
            "color": colour,
        })
    gates.sort(key=lambda g: g["gate"])

    raids = []
    for key, markup in db.execute(
            "select KEY, MSG from GameMsg "
            "where KEY like 'sys.preset.combined_list_commander_%' order by KEY"):
        text, _colour = split_colour(markup)
        raids.append({
            "groupId": int(key[len("sys.preset.combined_list_commander_"):]),
            "name": text,
        })
    raids.sort(key=lambda r: r["groupId"])

    if not difficulties or not gates or not raids:
        raise SystemExit("one of the three lookups came back empty")

    document = {
        "formatVersion": 1,
        "source": "EFTable_GameMsg sys.mvp.difficulty_* / sys.mvp.gate_num_* / "
                  "sys.preset.combined_list_commander_<GroupId>; the GroupId is "
                  "EFTable_ZoneEpicGate.GroupId",
        "fieldColor": "#ffffff",
        "difficulties": difficulties,
        "gates": gates,
        "raids": raids,
    }

    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(
        json.dumps(document, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8")
    print("%s: %d difficulties, %d gates, %d raids"
          % (args.out, len(difficulties), len(gates), len(raids)))
    return 0


if __name__ == "__main__":
    sys.exit(main())

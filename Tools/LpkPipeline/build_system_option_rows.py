#!/usr/bin/env python3
"""Build the 환경설정 window's row document from the retail option table.

The whole retail window is data driven: `EFTable_SystemOption` (787 rows) owns every tab,
group, row order, control type, default, range and combo choice, and `EFTable_GameMsg` owns
the text. Nothing here is read off a screenshot.

Output
------
  Data/UI/SystemOption/SystemOptionRows.json
      schema `lostark.system-option-rows`. One entry per retail tab in the retail tab-column
      order, each with its rows in `DisplayOrder`. Tabs this project does not implement stay
      in the document with `"included": false` so the excluded set is visible rather than
      silently dropped.

How the retail table lays a screen out
--------------------------------------
  Tab / Category   which screen, and which tab-column parent it hangs under
  DisplayOrder     order inside the screen -- and, on the `TAB_TITLE` row, the order of the
                   tab column itself (0 video, 100 audio, 200 accessibility, 300..600 the
                   four game-play children, 700.. community, 1000.. hotkeys, 1700.. gamepad)
  ComponentType    the control (see COMPONENT_TYPES)
  AttachTargetIndex + Direction + MarginX/Y
                   relative layout: a row attaches to another row's PrimaryKey (or to a
                   container anchor >= 20000) and sits either below it (Direction 3) or to
                   its right (Direction 2), offset by the margins
  Enabled          0 = retail does not show the row at all (e.g. the DirectX setting rows)
  WidgetEnabled    1 on a GROUP_TITLE = it carries the favourite star
  ExportName       the save tag, identical to UserOption.xml's `Tag="..."`
  ChildComp01..11  combo box / radio choices, as GameMsg keys

Inputs (extract first if missing)
  python unpack_lpk.py <EFGame>/data2.lpk --region KR --filter EFTable_SystemOption --out <dir>
  D:/ClaudeWork/Extracted/LpkTables/EFGame_Extra/ClientData/TableData/EFTable_SystemOption.db
  D:/ClaudeWork/Extracted/LpkTables/EFGame_Extra/ClientData/TableData/EFTable_GameMsg.db

Usage:
  python build_system_option_rows.py [--repo <LostArk root>]
"""

from __future__ import annotations

import argparse
import json
import re
import sqlite3
import sys
from pathlib import Path

EXTRACTED = Path(r"D:/ClaudeWork/Extracted")
TABLES = EXTRACTED / "LpkTables/EFGame_Extra/ClientData/TableData"
SYSTEM_OPTION_DB = TABLES / "EFTable_SystemOption.db"
GAMEMSG_DB = TABLES / "EFTable_GameMsg.db"

# ComponentType -> what the retail window instantiates. Derived from the ExportName /
# CompName of every row of each type (combobox_*, slider_*, checkbox_*, color_*, ...) plus
# the decoration types, which carry no CompName at all.
COMPONENT_TYPES = {
    0: "COMBOBOX",
    1: "SLIDER",
    2: "CHECKBOX",
    3: "HOTKEY",
    4: "COLOR",
    5: "SCALE_LIST",      # HUD / buff size radio row
    6: "BUTTON",          # 초기화, 자동 설정
    7: "GROUP_TITLE",     # "| <name>" heading, optional favourite star
    8: "SUBTITLE",        # small caption under a slider (느리게 / 빠르게)
    9: "SEPARATOR",       # horizontal rule, no text
    10: "TEXT_INPUT",
    11: "UNKNOWN_11",
    12: "UNKNOWN_12",
    13: "TAB_TITLE",      # the screen's own heading, and the tab column's order
    14: "COUNT_LIST",     # buff count radio row
    15: "RADIO_PAIR",     # 기본 연출 / 부드러운 연출
    16: "SPINNER",        # -/+ numeric with a slider (FPS limit)
}

# Category -> the tab-column parent. A category with a single tab has no parent row in the
# column; the four game-play screens and the hotkey / gamepad / community sets do.
CATEGORY_PARENTS = {
    0: (None, None),
    1: (None, None),
    6: (None, "sys.preferences.window_tab_accessibility"),
    2: ("gameplay", "sys.preferences.window_tap_gameplay"),
    3: ("community", "sys.preferences.window_tap_cummunity"),
    5: ("hotkey", "sys.preferences.window_tap_hotkey"),
    4: ("controller", "sys.preferences.window_tap_controller"),
}

# What this project implements. The rest stays in the document, flagged out.
INCLUDED_TABS = {0, 1, 2, 21, 22, 8, 9}

CHILD_COMP_KEYS = ["ChildComp%02d" % i for i in range(1, 12)]
MARKUP = re.compile(r"<[^>]*>")


def read_table(path: Path, table: str) -> list[dict]:
    """Every row as a dict. Text columns in this table are CP949, unlike GameMsg's UTF-8."""
    conn = sqlite3.connect(str(path))
    conn.text_factory = bytes
    cur = conn.cursor()
    cur.execute("PRAGMA table_info(%s)" % table)
    columns = [c[1].decode() if isinstance(c[1], bytes) else c[1] for c in cur.fetchall()]

    def decode(value):
        if isinstance(value, bytes):
            return value.decode("cp949", "replace")
        return value

    cur.execute("select * from %s" % table)
    rows = [dict(zip(columns, [decode(v) for v in row])) for row in cur.fetchall()]
    conn.close()
    return rows


def read_strings() -> dict[str, str]:
    """GameMsg MSG is UTF-8 bytes; Scaleform <FONT>/<br> markup is stripped."""
    conn = sqlite3.connect(str(GAMEMSG_DB))
    conn.text_factory = bytes
    cur = conn.cursor()
    cur.execute("select KEY, MSG from GameMsg where KEY like 'sys.preferences.%' "
                "or KEY like 'sys.systemoption.%'")
    out = {}
    for key, msg in cur.fetchall():
        name = key.decode("ascii", "replace")
        text = MARKUP.sub("", msg.decode("utf-8", "replace")).strip()
        out[name] = text
        # EFTable_SystemOption spells a handful of keys in mixed case
        # (`combobox_NarrationAssistant`, `keyboard_language_Korean`) while GameMsg stores
        # them lowercase. Keep a folded alias so those rows resolve instead of going blank.
        out.setdefault(name.lower(), text)
    conn.close()
    return out


def build(rows: list[dict], strings: dict[str, str]) -> dict:
    missing_strings: set[str] = set()

    def text(key):
        if not key:
            return None
        value = strings.get(key)
        return value if value is not None else strings.get(key.lower())

    def resolve(key):
        if not key:
            return None
        value = text(key)
        if value is None:
            missing_strings.add(key)
        return value

    tab_titles = {r["Tab"]: r for r in rows if r["ComponentType"] == 13}
    tabs = []
    for tabId, titleRow in sorted(tab_titles.items(), key=lambda kv: kv[1]["DisplayOrder"]):
        category = titleRow["Category"]
        parentId, parentKey = CATEGORY_PARENTS.get(category, (None, None))
        tabRows = [r for r in rows if r["Tab"] == tabId and r["ComponentType"] != 13]
        tabRows.sort(key=lambda r: r["DisplayOrder"])

        emitted = []
        for r in tabRows:
            choices = []
            for column in CHILD_COMP_KEYS:
                key = r[column]
                if not key:
                    continue
                choices.append({"key": key, "text": resolve(key)})
            emitted.append({
                "id": r["CompName"] or r["ExportName"] or ("row%d" % r["PrimaryKey"]),
                "primaryKey": r["PrimaryKey"],
                "order": r["DisplayOrder"],
                "type": COMPONENT_TYPES.get(r["ComponentType"], "UNKNOWN_%d" % r["ComponentType"]),
                "group": r["GroupIndex"],
                "saveTag": r["ExportName"] or None,
                "titleKey": r["Title"] or None,
                "title": resolve(r["Title"]),
                "tooltipKey": r["ToolTip"] or None,
                "tooltip": resolve(r["ToolTip"]),
                "default": r["DefaultValue"],
                "defaultString": r["DefaultStringvalue"] or None,
                "minimum": r["Min"],
                "maximum": r["Max"],
                "unit": r["Unit"] or None,
                "sliderIcon": r["SliderIcon"],
                "showSliderValue": bool(r["ShowSliderValue"]),
                "snapInterval": r["SliderSnapInterval"],
                "attach": {
                    "target": r["AttachTargetIndex"],
                    "direction": r["Direction"],
                    "marginX": r["MarginX"],
                    "marginY": r["MarginY"],
                    "marginLength": r["MarginLength"],
                },
                "width": r["Width"],
                "height": r["Height"],
                "enabled": bool(r["Enabled"]),
                "favourite": bool(r["WidgetEnabled"]),
                "choices": choices,
            })

        tabs.append({
            "tabId": tabId,
            "category": category,
            "order": titleRow["DisplayOrder"],
            "included": tabId in INCLUDED_TABS,
            "titleKey": titleRow["Title"],
            "title": resolve(titleRow["Title"]),
            "parentId": parentId,
            "parentLabelKey": parentKey,
            "parentLabel": text(parentKey),
            "rows": emitted,
        })

    document = {
        "schema": "lostark.system-option-rows",
        "formatVersion": 1,
        "source": {
            "table": "EFTable_SystemOption",
            "tableRows": len(rows),
            "strings": "EFTable_GameMsg",
        },
        "componentTypes": {str(k): v for k, v in sorted(COMPONENT_TYPES.items())},
        "tabs": tabs,
    }
    return document, missing_strings


def audit(document: dict, missing_strings: set[str]) -> None:
    """Report what the extract covers and what it does not, per tab."""
    included = [t for t in document["tabs"] if t["included"]]
    excluded = [t for t in document["tabs"] if not t["included"]]
    print("tabs: %d total, %d included, %d excluded" %
          (len(document["tabs"]), len(included), len(excluded)))
    for tab in document["tabs"]:
        hidden = sum(1 for r in tab["rows"] if not r["enabled"])
        unknown = sorted({r["type"] for r in tab["rows"] if r["type"].startswith("UNKNOWN")})
        mark = "+" if tab["included"] else "-"
        print("  %s tab %-2d order %-4d %-24s %3d rows (%d retail-hidden)%s" % (
            mark, tab["tabId"], tab["order"], tab["title"], len(tab["rows"]), hidden,
            ("  unknown types: " + ",".join(unknown)) if unknown else ""))
    if excluded:
        print("excluded (project scope): " +
              ", ".join("%s" % t["title"] for t in excluded))
    untitled = sum(1 for t in document["tabs"] for r in t["rows"]
                   if r["titleKey"] and r["title"] is None)
    if missing_strings:
        print("!! %d GameMsg keys did not resolve (%d rows affected); first few: %s" % (
            len(missing_strings), untitled, ", ".join(sorted(missing_strings)[:5])))
    else:
        print("every referenced GameMsg key resolved")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[2])
    args = ap.parse_args()

    for path in (SYSTEM_OPTION_DB, GAMEMSG_DB):
        if not path.exists():
            print("missing input:", path)
            return 1

    rows = read_table(SYSTEM_OPTION_DB, "SystemOption")
    strings = read_strings()
    document, missing_strings = build(rows, strings)

    out_dir = args.repo / "Data/UI/SystemOption"
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / "SystemOptionRows.json"
    out_path.write_text(json.dumps(document, ensure_ascii=False, indent=1) + "\n",
                        encoding="utf-8")
    audit(document, missing_strings)
    print("wrote", out_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

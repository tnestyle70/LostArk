#!/usr/bin/env python3
"""환경설정 DB의 탭·행·기본값과 GameMsg 문구를 SystemOptionRows.json으로 옮긴다.
원본 DisplayOrder·배치 연결을 유지하고 미지원 탭은 included=false로 남긴다.
입력은 EFTable_SystemOption.db/EFTable_GameMsg.db, 출력은 Data/UI/SystemOption이다.
"""

from __future__ import annotations

# 출력할 저장소 경로 --repo를 받는다.
import argparse
# 옵션 행과 원본 출처를 JSON으로 저장한다.
import json
# GameMsg의 표시 마크업을 제거한다.
import re
# 추출한 SystemOption·GameMsg DB를 조회한다.
import sqlite3
# 현재 본문에서는 사용하지 않는 기존 import다.
import sys
# 추출 DB와 저작 JSON 경로를 결합한다.
from pathlib import Path

EXTRACTED = Path(r"D:/ClaudeWork/Extracted")
TABLES = EXTRACTED / "LpkTables/EFGame_Extra/ClientData/TableData"
SYSTEM_OPTION_DB = TABLES / "EFTable_SystemOption.db"
GAMEMSG_DB = TABLES / "EFTable_GameMsg.db"

# 원본 ComponentType을 JSON의 컨트롤 이름으로 연결한다. 장식 행도 별도 종류로 남긴다.
COMPONENT_TYPES = {
    0: "COMBOBOX",
    1: "SLIDER",
    2: "CHECKBOX",
    3: "HOTKEY",
    4: "COLOR",
    5: "SCALE_LIST",      # HUD·버프 크기 선택
    6: "BUTTON",          # 초기화, 자동 설정
    7: "GROUP_TITLE",     # 그룹 제목과 즐겨찾기 표시
    8: "SUBTITLE",        # 슬라이더 설명(느리게 / 빠르게)
    9: "SEPARATOR",       # 글자 없는 구분선
    10: "TEXT_INPUT",
    11: "UNKNOWN_11",
    12: "UNKNOWN_12",
    13: "TAB_TITLE",      # 화면 제목과 탭 순서
    14: "COUNT_LIST",     # 버프 개수 선택
    15: "RADIO_PAIR",     # 기본 연출 / 부드러운 연출
    16: "SPINNER",        # 숫자 증감과 슬라이더(FPS 제한)
}

# 여러 탭을 묶는 category는 부모 행 ID와 문구 키를 가진다. 단독 탭은 부모를 두지 않는다.
CATEGORY_PARENTS = {
    0: (None, None),
    1: (None, None),
    6: (None, "sys.preferences.window_tab_accessibility"),
    2: ("gameplay", "sys.preferences.window_tap_gameplay"),
    3: ("community", "sys.preferences.window_tap_cummunity"),
    5: ("hotkey", "sys.preferences.window_tap_hotkey"),
    4: ("controller", "sys.preferences.window_tap_controller"),
}

# 지원 탭만 included=true로 표시하며 나머지 원본 행도 문서에 보존한다.
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
        # 두 원본 테이블의 대소문자 차이로 문구가 비지 않도록 소문자 조회 키도 보관한다.
        out.setdefault(name.lower(), text)
    conn.close()
    return out


# 탭/행은 DisplayOrder로 정렬하고 텍스트 키·기본값·범위·상대 배치를 한 문서로 묶는다.
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

    # 두 DB의 문자 인코딩을 각각 해석한 뒤 ID로 문구를 연결한다.
    rows = read_table(SYSTEM_OPTION_DB, "SystemOption")
    strings = read_strings()
    document, missing_strings = build(rows, strings)

    # JSON 저장 후 누락 문구를 보고한다. 문구 누락 자체가 저장을 취소하지는 않는다.
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

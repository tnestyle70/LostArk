#!/usr/bin/env python3
"""Build Data/UI/MVP/MvpAwards.json from the retail MVP tables.

The award page needs two different kinds of fact and only one of them ships in
the client:

  medals   EFTable_MvpMedalGroup carries the real conditions as Param1..3, so a
           medal row is extracted end to end -- which medals a raid can award,
           their icon index, their name and their threshold values.

  titles   EFTable_MvpTitle carries three tiers per contribution and nothing
           else.  Every numeric column on it (ClassifyType, ClassifyIndex,
           Comment, Milestone) is a constant across all 1413 rows, and no other
           table in any of the eight .lpk archives holds a cutoff.  Retail's
           server decides the tier from the player's share and sends an index.
           The percentages therefore come from OUTSIDE the client data and are
           declared in EXTERNAL_TIER_THRESHOLDS below, marked as such in the
           output so nobody later mistakes them for an extraction.

Inputs are the decrypted tables produced by unpack_lpk.py.

Usage:
  python build_mvp_awards.py --tables <dir> --gamemsg <EFTable_GameMsg.db>
                             --group 220000 --out Data/UI/MVP/MvpAwards.json
"""

from __future__ import annotations

import argparse
import json
import re
import sqlite3
import sys
from pathlib import Path

# Contribution share cutoffs, as the lower bound of tier 1 / 2 / 3, per party
# size.  Keyed by EFTable_Mvp.StatType.
#
# NOT EXTRACTED.  Supplied by the project owner from an external reference.  A
# stat absent from this table ships without thresholds and the runtime resolves
# no title for it rather than inventing one.
EXTERNAL_TIER_THRESHOLDS = {
    # 준 피해 -- 4인 투사 29% 이하 / 강투 30~39 / 잔혈 40+
    #            8인 투사 14% 이하 / 강투 15~19 / 잔혈 20+
    # Tier 1 has no stated lower bound, so any share earns 투사.
    1: {"party4": [0.0, 30.0, 40.0], "party8": [0.0, 15.0, 20.0]},
    # 준 무력화 -- 섬멸자 / 비정한 / 진격의
    3: {"party4": [10.0, 30.0, 40.0], "party8": [5.0, 15.0, 20.0]},
    # 파티 회복 -- 치유사 / 온화한 / 고결한
    4: {"party4": [20.0, 30.0, 40.0], "party8": [10.0, 15.0, 20.0]},
    # 배틀아이템 사용 횟수 -- 지략가 / 냉철한 / 탁월한
    9: {"party4": [20.0, 30.0, 40.0], "party8": [10.0, 15.0, 20.0]},
    # 카운터 성공 -- 봉쇄자 / 찰나의 / 섬광의
    11: {"party4": [20.0, 30.0, 40.0], "party8": [10.0, 15.0, 20.0]},
    # 공격 지원 -- 조력자 / 숭고한 / 찬란한
    13: {"party4": [15.0, 20.0, 25.0], "party8": [7.5, 10.0, 12.5]},
    # 파티 방어 -- 수호자 / 달의 / 태양의
    14: {"party4": [20.0, 30.0, 40.0], "party8": [10.0, 15.0, 20.0]},
}

# Stats whose title is not a share ranking at all.  EFTable_MvpTitle still
# carries three rows for them; retail awards the one title on the condition
# named here instead of on a percentage.
TITLE_RULE_OVERRIDES = {
    # 생존 시간(초) -- 최후의 1인, awarded for being alive at the clear.
    12: "survive",
}

# How much a contribution moves the total that picks the MVP.
#
# NOT EXTRACTED.  Supplied by the project owner from an external reference,
# which states an ordering ("최상위 / 상위 / 중위 / 하위") and not coefficients.
# `weightTier` therefore carries that ordering only -- 1 is the strongest -- and
# the runtime never multiplies by it.  Scoring belongs to the server; the
# runtime is handed a score per contribution and uses the tier only to break a
# tie between two equal scores.
#
# `scoreRole` is the same reference's split between the contributions that carry
# the total ("핵심 점수 반영") and the two that only nudge it ("보조 점수 반영").
SCORE_WEIGHTS = {
    1:  {"weightTier": 1, "scoreRole": "core"},     # 준 피해
    13: {"weightTier": 1, "scoreRole": "core"},     # 공격 지원
    3:  {"weightTier": 2, "scoreRole": "core"},     # 준 무력화
    14: {"weightTier": 2, "scoreRole": "core"},     # 파티 방어
    11: {"weightTier": 3, "scoreRole": "support"},  # 카운터 성공
    4:  {"weightTier": 3, "scoreRole": "core"},     # 파티 회복
    9:  {"weightTier": 4, "scoreRole": "support"},  # 배틀아이템 사용 횟수
    # StatType 12 생존 is not part of the total at all.
}

# Contributions whose title at most one of the three party columns may show.
# The main MVP is exempt.  From the same reference: a second dealer whose best
# contribution is 준 피해 is pushed onto their next contribution instead.
COLUMN_EXCLUSIVE_STAT_TYPES = [1]

TAG = re.compile(r"<[^>]+>")


def strip_markup(text: str) -> str:
    return TAG.sub("", text or "")


def first_line(text: str) -> str:
    for line in strip_markup(text).splitlines():
        line = line.strip()
        if line:
            return line
    return ""


class Strings:
    """EFTable_GameMsg lookup.  A missing key is an error, not a blank."""

    def __init__(self, path: Path):
        self._db = sqlite3.connect(f"file:{path.as_posix()}?mode=ro", uri=True)

    def get(self, key: str) -> str:
        row = self._db.execute(
            "select MSG from GameMsg where KEY=?", (key,)
        ).fetchone()
        if row is None:
            raise SystemExit(f"GameMsg has no key {key!r}")
        return row[0] or ""


def open_table(tables: Path, name: str) -> sqlite3.Connection:
    path = tables / f"EFTable_{name}.db"
    if not path.is_file():
        raise SystemExit(f"missing table {path}")
    db = sqlite3.connect(f"file:{path.as_posix()}?mode=ro", uri=True)
    db.row_factory = sqlite3.Row
    return db


def build_stats(tables: Path, msg: Strings, group: int) -> list[dict]:
    mvp = open_table(tables, "Mvp")
    title = open_table(tables, "MvpTitle")

    tiers: dict[int, dict[int, sqlite3.Row]] = {}
    for row in title.execute("select * from MvpTitle"):
        tiers.setdefault(row["PrimaryKey"], {})[int(row["SecondaryKey"])] = row

    stats = []
    for row in mvp.execute(
        "select * from Mvp where GroupIndex=? order by PrimaryKey", (group,)
    ):
        pk = row["PrimaryKey"]
        by_tier = tiers.get(pk)
        if not by_tier or sorted(by_tier) != [1, 2, 3]:
            raise SystemExit(f"EFTable_MvpTitle PrimaryKey {pk} is not 3 tiers")

        # Desc and TitleTooltip are identical across the three tiers; tier 1
        # stands in for all of them.
        base = by_tier[1]
        entry = {
            "primaryKey": pk,
            "statType": row["StatType"],
            # 0 = absolute amount, 1 = percentage; the value formatter retail
            # picks (sys.mvp.calctype_abs / _percent).
            "calcType": row["CalcType"],
            "targetNpcGrade": row["TargetNpcGrade"],
            "name": strip_markup(msg.get(base["Desc"])).strip(),
            "tooltip": strip_markup(msg.get(base["TitleTooltip"])).strip(),
            "titles": [strip_markup(msg.get(by_tier[t]["Title"])).strip()
                       for t in (1, 2, 3)],
        }

        stat_type = row["StatType"]
        entry["titleRule"] = TITLE_RULE_OVERRIDES.get(stat_type, "share")
        entry.update(SCORE_WEIGHTS.get(
            stat_type, {"weightTier": 0, "scoreRole": "none"}))

        cutoffs = EXTERNAL_TIER_THRESHOLDS.get(stat_type)
        if entry["titleRule"] == "share" and cutoffs is not None:
            for size, values in cutoffs.items():
                if len(values) != 3 or sorted(values) != values:
                    raise SystemExit(f"thresholds for StatType {stat_type} "
                                     f"{size} must be three ascending values")
            entry["sharePercentThresholds"] = cutoffs
            entry["thresholdSource"] = "external"
        stats.append(entry)

    if not stats:
        raise SystemExit(f"EFTable_Mvp has no rows for GroupIndex {group}")
    return stats


def build_medals(tables: Path, msg: Strings, group: int) -> list[dict]:
    medal_group = open_table(tables, "MvpMedalGroup")
    description = open_table(tables, "MvpMedalDescription")

    described: dict[int, sqlite3.Row] = {}
    for row in description.execute("select * from MvpMedalDescription"):
        described[int(row["IconIndex"])] = row

    medals = []
    for row in medal_group.execute(
        "select * from MvpMedalGroup where PrimaryKey=?", (group,)
    ):
        index = int(re.sub(r"\D", "", str(row["SecondaryKey"])) or 0)
        described_row = described.get(index)
        if described_row is None:
            raise SystemExit(f"medal index {index} has no MvpMedalDescription row")
        tooltip = msg.get(described_row["Title"])
        medals.append({
            "medalIndex": index,
            "icon": described_row["Icon"],
            "iconIndex": int(described_row["IconIndex"]),
            # EFTable_MvpMedalDescription has no separate name column; retail's
            # own tooltip opens with the medal name on its first line.
            "name": first_line(tooltip),
            "tooltip": strip_markup(tooltip).strip(),
            # The columns are literally Param1..3 -- the table does not say
            # whether a value is milliseconds, a percentage or a count.
            "param1": row["Param1"],
            "param2": row["Param2"],
            "param3": row["Param3"],
        })

    medals.sort(key=lambda m: m["medalIndex"])
    if not medals:
        raise SystemExit(f"EFTable_MvpMedalGroup has no rows for {group}")
    return medals


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--tables", type=Path, required=True)
    parser.add_argument("--gamemsg", type=Path, required=True)
    parser.add_argument("--group", type=int, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()

    msg = Strings(args.gamemsg)
    document = {
        "formatVersion": 1,
        "source": "EFTable_Mvp / EFTable_MvpTitle / EFTable_MvpMedalGroup / "
                  "EFTable_MvpMedalDescription (data2.lpk); strings from "
                  "EFTable_GameMsg",
        "thresholdNote": "sharePercentThresholds is not in the client data. "
                         "Retail's server picks the tier and sends an index; "
                         "these cutoffs are supplied by the project owner.",
        "mvpGroupId": args.group,
        "columnExclusiveStatTypes": COLUMN_EXCLUSIVE_STAT_TYPES,
        "stats": build_stats(args.tables, msg, args.group),
        "medals": build_medals(args.tables, msg, args.group),
    }

    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(
        json.dumps(document, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8")
    print(f"{args.out}: {len(document['stats'])} stats, "
          f"{len(document['medals'])} medals")
    return 0


if __name__ == "__main__":
    sys.exit(main())

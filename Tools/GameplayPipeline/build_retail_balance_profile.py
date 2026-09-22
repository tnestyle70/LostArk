#!/usr/bin/env python3
"""Build Data/Balance/Profiles/Retail.balanceprofile.json from the official tables.

The authored documents under Data/Balance keep the team's fast-combat tuning.  This
script writes a sparse override document that Publish-GameplayBalance.ps1 applies with
-BalanceProfile Retail, so the published runtime carries the original values instead.

Every field is either an official table cell or one of the four project policies listed
in PROJECT_POLICY below; nothing else is invented.  The tables are the SQLite databases
unpacked from the retail client:

    python Tools/LpkPipeline/unpack_lpk.py <EFGame>/data2.lpk --out <table-root> \
        --filter EFTable_Skill.db      (and Npc, NpcBalance, NpcStat, PC)

Usage:
    python Tools/GameplayPipeline/build_retail_balance_profile.py --table-root <dir>
"""

from __future__ import annotations

import argparse
import json
import re
import sqlite3
from pathlib import Path
from typing import Any

SCHEMA = "lostark.balance-profile"
FORMAT_VERSION = 1
PROFILE_ID = "Retail"

REFERENCE_SKILL_LEVEL = 10

# EFTable_PC.PrimaryKey of the original class each playable class is built from.
# Gunslinger and Slayer were never built out in this project, so their rows keep the
# authored values and only these five take the original damage rates.
DAMAGE_RATE_CLASSES = (
    "LANCE_MASTER", "ARTIST", "DIMENSIONMASTER", "WARLORD", "GUARDIANKNIGHT",
)

CLASS_PC_KEY = {
    "LANCE_MASTER": 305,        # LanceMaster
    "GUNSLINGER": 512,          # Devilhunter_Female
    "SLAYER": 112,              # Berserker_Female
    "ARTIST": 602,              # YinYangShi
    "DIMENSIONMASTER": 612,     # DimensionMaster
    "WARLORD": 104,             # Gunlancer
    "GUARDIANKNIGHT": 702,      # DragonKnight
}

# EFTable_Npc.PrimaryKey of the original boss each archetype plays.
# BOSS_KAKULSAYDON_G1_KOUKU is left out on purpose: the MN_RPCZ_00 candidates carry no
# NpcBalance row, matching the project's own one-bar presentation-only boss.
BOSS_NPC_KEY = {
    "BOSS_VALTAN": 480007,
    "BOSS_VALTAN_GHOST": 480008,
    "BOSS_KAKULSAYDON_G1_SAYDON": 480601,
    "BOSS_KAKULSAYDON_G2_KOUKU": 480611,
    "BOSS_KAKULSAYDON_G2_BIG_SAYDON": 480621,
    "BOSS_KAKULSAYDON_G3_SAYDON": 480631,
    "BOSS_KAKULSAYDON_BINGO_SAYDON": 480635,
}

# Monsters whose model path carries no NPC id.  The clown box is named in CLAUDE.md.
EXTRA_MONSTER_NPC_KEY = {
    "MONSTER_KOUKU_CLOWN_BOX": 480720,
}

# A tooltip names the exact damage rows it shows and how many times each repeats, e.g.
#   "창을 <FONT>2</FONT>회 휘둘러 <$MACRO physic_ch305 @1:340400/>, <$MACRO ... @1:340401/>"
# so it is the oracle for a skill's total rate, not the raw row list (which also holds
# tripod variants).  Only physic/magic macros are damage.
TOOLTIP_MACRO = re.compile(r"<\$MACRO\s+(\w+)\s+@1:(\d+)")
TOOLTIP_REPEAT = re.compile(r"<FONT[^>]*>(\d+)</FONT>\s*회")
TOOLTIP_DAMAGE_MACRO = re.compile(r"^(physic|magic)", re.IGNORECASE)

# The buffs the Server applies, and what each one modifies.  Duration, percent and the
# target come from the original tables: EFTable_SkillBuff.Duration and
# PassiveOptionValue (hundredths of a percent), and EFTable_SkillEffect.Target on the
# add_status_effect row -- 0 self, 1 ally, 2 enemy.  The field a buff drives is named
# here because PassiveOptionType is 2 for both directions; the sign alone would not say
# whether it is damage dealt or damage taken.
SKILL_BUFFS = [
    # skillId, buffId, icon asset name
    (17170, 171702, "warlord_guardian"),
    (17250, 172500, "warlord_oath"),
    (31050, 310501, "artist_setting_moon"),
    (31950, 319503, "artist_mir"),
    (34510, 345003, "lancemaster_short_spear"),
    (49040, 490407, "guardianknight_dragon_mark"),
]
# EFTable_SkillBuff.PassiveOptionKeyStat names the stat a buff drives, resolved through
# the stattype enum: 144 physical_inc_sub_rate_2, 146 magical_inc_sub_rate_2 (both the
# damage the holder takes) and 148 skill_damage_sub_rate_2 (the damage it deals).
BUFF_STAT_FIELD = {
    141: "damageDealtPercent", 142: "damageDealtPercent",   # attack_power_sub_rate
    143: "damageTakenPercent", 144: "damageTakenPercent",   # physical_inc_sub_rate
    145: "damageTakenPercent", 146: "damageTakenPercent",   # magical_inc_sub_rate
    147: "damageDealtPercent", 148: "damageDealtPercent",   # skill_damage_sub_rate
    78: "attackSpeedPercent",                               # attack_speed_rate
}
BUFF_TARGET = {0: "SELF", 1: "ALLY", 2: "ENEMY"}
# EFTable_SkillBuff.Duration is -1 for a buff the original ends on a condition rather
# than a clock (the Gunlancer guardians).  The Server needs a window, so these hold for
# ten seconds, which is this project's choice and not an official cell.
CONDITION_BUFF_DURATION_MS = 10000

PROJECT_POLICY = {
    # The client tables ship no final player attack power: EFTable_PCStat is published
    # with its payload columns removed, so this is a project number, not an official
    # cell.  It is the one dial the original rates are balanced against.
    "attackPower": 23000,
    # Player HP is MaxHpCon x the constitution stat.  The stat is server-side, so the
    # baseline is chosen to put a 250% Valtan pattern at ~43% of a Lance Master's bar.
    "constitutionBaseline": 60000,
    # criticalChance = critical stat x PCLevel.CriticalHitCoefficient(0.2794) / 10.
    # The stat itself is not in the tables; 1500 is the reference spec's value.
    "criticalStat": 1500,
    "criticalHitCoefficient": 0.2794,
    # Retail's base critical multiplier.  No table cell carries it.
    "criticalDamagePercent": 200,
    # Skill.CostMp is 185..938, so the pool moves from 1000 to 10000 and the regen with
    # it.  The ratio is unchanged, which keeps resource pacing identical.
    "resourcePool": 10000,
    "resourceRegenPerSecond": 5000,
    # Skill.StiffnessTooltipType is a display grade, not a number, and the amount is
    # computed server-side.  The step between grades is official though: every EFTable_
    # SkillFeature Type 48 tripod that raises a skill one grade gives +40% stagger
    # damage (하->중 x16, 중->중상 x35, 중상->상 x26, 상->최상 x12 rows) and every
    # two-grade tripod gives +96% = 1.4 x 1.4, so one grade is x1.4.  Only the base of
    # the series is a project choice: 1800 puts four players' three best stagger skills
    # at 38,790 against the original 40,000 gauge.
    "staggerLowBase": 1800,
    "staggerGradeStep": 1.4,
    # Authored SET_STAGGER_GAUGE values are 30 and 100; x400 makes the large window the
    # original 40000 of NpcBalance.ParalyzationPointMax.
    "staggerGaugeScale": 400,
    # ZoneContentsGauge 3708100 has no passive fill, so the original Kouku madness gauge
    # only rises through a skill effect or accumulate_damage_ratio.  The Server now charges
    # it from the share of maximum HP a hit took, so the authored fixed step retires.
    "madnessGaugeAddPercent": 0,
    # Skill.PartsAttackLevelTooltip is the original part-break level 1..3 and 0 for a
    # skill that cannot break a part at all.  The Server subtracts partDamage straight
    # from a plate's durability, so the level is the amount and the authored plate
    # durability stays the project's own threshold.
    "partDamageByLevel": {0: 0, 1: 1, 2: 2, 3: 3},
}


def read_json(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as stream:
        return json.load(stream)


def connect(table_root: Path, name: str) -> sqlite3.Connection:
    path = table_root / f"EFTable_{name}.db"
    if not path.is_file():
        raise FileNotFoundError(f"Missing official table: {path}")
    connection = sqlite3.connect(f"file:{path}?mode=ro", uri=True)
    connection.row_factory = sqlite3.Row
    return connection


def skill_row(skills: sqlite3.Connection, skill_id: int) -> sqlite3.Row:
    """Level 10, or the level 1 definition when 1 is the only level there is."""
    rows = skills.execute(
        "SELECT * FROM Skill WHERE PrimaryKey = ? ORDER BY SecondaryKey", (skill_id,)
    ).fetchall()
    if not rows:
        raise ValueError(f"EFTable_Skill has no row for skill {skill_id}")
    for row in rows:
        if REFERENCE_SKILL_LEVEL == int(row["SecondaryKey"]):
            return row
    levels = {int(row["SecondaryKey"]) for row in rows}
    if levels != {1}:
        raise ValueError(
            f"Skill {skill_id} has no level {REFERENCE_SKILL_LEVEL} row and is not a "
            f"fixed level 1 definition: secondaryKeys={sorted(levels)}"
        )
    return rows[0]


def resolve_npc(
    balances: sqlite3.Connection,
    stats: sqlite3.Connection,
    npc_id: int,
) -> dict[str, Any]:
    balance = balances.execute(
        "SELECT * FROM NpcBalance WHERE PrimaryKey = ?", (npc_id,)
    ).fetchone()
    if balance is None:
        raise ValueError(f"EFTable_NpcBalance has no row for NPC {npc_id}")
    stat = stats.execute(
        "SELECT Hp, SkillDamage, Def FROM NpcStat "
        "WHERE BalanceLevel = ? AND StatScaleKey = ?",
        (balance["BalanceLevel"], balance["StatScaleKey"]),
    ).fetchone()
    if stat is None:
        raise ValueError(
            f"EFTable_NpcStat has no row for NPC {npc_id} at BalanceLevel "
            f"{balance['BalanceLevel']} / {balance['StatScaleKey']}"
        )
    return {
        "npcId": npc_id,
        "balanceLevel": int(balance["BalanceLevel"]),
        "statScaleKey": str(balance["StatScaleKey"]),
        "maximumHp": int(stat["Hp"] * balance["Hp"] / 100),
        "attackPower": int(stat["SkillDamage"] * balance["AttackPower"] / 100),
        "maximumHealthBars": int(balance["Hp_Count"]),
        "staggerGaugeMaximum": int(balance["ParalyzationPointMax"]),
        "staggerDurationMinMs": int(balance["ParalyzationTimeMin"]),
        "staggerDurationMaxMs": int(balance["ParalyzationTimeMax"]),
        "counterFreezeMs": int(balance["CounterFreezeTime"]),
    }


def build_players(pcs: sqlite3.Connection, authored: list[dict]) -> list[dict]:
    policy = PROJECT_POLICY
    critical_chance = int(
        policy["criticalStat"] * policy["criticalHitCoefficient"] / 10.0
    )
    players = []
    for entry in authored:
        character_class = entry["characterClass"]
        key = CLASS_PC_KEY[character_class]
        row = pcs.execute(
            "SELECT MaxHpCon, DefCoefficient FROM PC WHERE PrimaryKey = ?", (key,)
        ).fetchone()
        if row is None:
            raise ValueError(f"EFTable_PC has no row {key} for {character_class}")
        players.append({
            "characterClass": character_class,
            "maximumHp": int(round(row["MaxHpCon"] * policy["constitutionBaseline"])),
            "maximumResource": policy["resourcePool"],
            "resourceRegenPerSecond": policy["resourceRegenPerSecond"],
            "attackPower": policy["attackPower"],
            "defense": int(round(row["DefCoefficient"] * 100)),
            "criticalChancePercent": critical_chance,
            "criticalDamagePercent": policy["criticalDamagePercent"],
        })
    return players


def stagger_by_grade() -> dict[int, int]:
    """Grade 0 stages nothing; 1..5 are the official x1.4 series off the chosen base."""
    base = PROJECT_POLICY["staggerLowBase"]
    step = PROJECT_POLICY["staggerGradeStep"]
    return {0: 0} | {
        grade: int(round(base * step ** (grade - 1))) for grade in range(1, 6)
    }


def tooltip_damage(
    messages: sqlite3.Connection,
    effects: sqlite3.Connection,
    skill_id: int,
    level: int,
) -> tuple[int, int]:
    """(attack coefficient in 1/10000, flat addend) summed over the tooltip's hits."""
    row = messages.execute(
        "SELECT MSG FROM GameMsg WHERE KEY = ?", (f"tip.desc.skill_{skill_id}",)
    ).fetchone()
    if row is None:
        return 0, 0
    text = str(row["MSG"])
    coefficient = addend = 0
    for match in TOOLTIP_MACRO.finditer(text):
        if not TOOLTIP_DAMAGE_MACRO.match(match.group(1)):
            continue
        hit = damage_terms(effects, int(match.group(2)), level)
        if hit is None:
            continue
        tail = text[match.end(): match.end() + 140]
        repeat = TOOLTIP_REPEAT.search(tail)
        times = int(repeat.group(1)) if (
            repeat is not None and "<$MACRO" not in tail[:repeat.start()]) else 1
        coefficient += hit[0] * times
        addend += hit[1] * times
    return coefficient, addend


def damage_terms(
    effects: sqlite3.Connection,
    effect_pk: int,
    level: int,
) -> tuple[int, int] | None:
    """One hit's (ValueF, (ValueA + ValueB) / 2), only for a real damage row."""
    row = effects.execute(
        "SELECT ValueA, ValueB, ValueF FROM SkillEffect WHERE PrimaryKey = ? "
        "AND SecondaryKey = ? AND Key IN (1,2,3)", (effect_pk, level)).fetchone()
    if row is None:
        row = effects.execute(
            "SELECT ValueA, ValueB, ValueF FROM SkillEffect WHERE PrimaryKey = ? "
            "AND Key IN (1,2,3) ORDER BY SecondaryKey DESC LIMIT 1", (effect_pk,)).fetchone()
    if row is None:
        return None
    return int(row["ValueF"]), (int(row["ValueA"]) + int(row["ValueB"])) // 2


def tooltip_damage_rate(
    messages: sqlite3.Connection,
    effects: sqlite3.Connection,
    skill_id: int,
    level: int,
) -> int:
    """The skill's total rate as its own tooltip adds it up, or 0 when it shows none."""
    row = messages.execute(
        "SELECT MSG FROM GameMsg WHERE KEY = ?", (f"tip.desc.skill_{skill_id}",)
    ).fetchone()
    if row is None:
        return 0
    text = str(row["MSG"])
    total = 0
    for match in TOOLTIP_MACRO.finditer(text):
        if not TOOLTIP_DAMAGE_MACRO.match(match.group(1)):
            continue
        value = damage_value(effects, int(match.group(2)), level)
        if value is None:
            continue
        tail = text[match.end(): match.end() + 140]
        repeat = TOOLTIP_REPEAT.search(tail)
        # A repeat count belongs to the macro right before it, with no macro in between.
        times = int(repeat.group(1)) if (
            repeat is not None and "<$MACRO" not in tail[:repeat.start()]) else 1
        total += value * times
    return total


def damage_value(
    effects: sqlite3.Connection,
    effect_pk: int,
    level: int,
) -> int | None:
    """ValueA of one effect row, only when that row really is a damage effect."""
    row = effects.execute(
        "SELECT ValueA FROM SkillEffect WHERE PrimaryKey = ? AND SecondaryKey = ? "
        "AND Key IN (1,2,3)", (effect_pk, level)).fetchone()
    if row is None:
        row = effects.execute(
            "SELECT ValueA FROM SkillEffect WHERE PrimaryKey = ? AND Key IN (1,2,3) "
            "ORDER BY SecondaryKey DESC LIMIT 1", (effect_pk,)).fetchone()
    return None if row is None else int(row["ValueA"])


def build_skills(
    skills: sqlite3.Connection,
    effects: sqlite3.Connection,
    messages: sqlite3.Connection,
    authored: list[dict],
) -> list[dict]:
    stagger = stagger_by_grade()
    part_damage_by_level = PROJECT_POLICY["partDamageByLevel"]
    rows = []
    for entry in authored:
        skill_id = int(entry["skillId"])
        official = skill_row(skills, skill_id)
        grade = int(official["StiffnessTooltipType"])
        if grade not in stagger:
            raise ValueError(f"Skill {skill_id} has unknown stagger grade {grade}")
        part_level = int(official["PartsAttackLevelTooltip"])
        if part_level not in part_damage_by_level:
            raise ValueError(f"Skill {skill_id} has unknown part level {part_level}")
        row = {
            "skillId": skill_id,
            "cooldownMs": int(official["Cooltime"]),
            "resourceCost": int(official["CostMp"]),
            "staggerDamage": stagger[grade],
            "partDamage": part_damage_by_level[part_level],
        }
        # The basic attack is a combo whose stages divide their own damage, so it is not
        # a quick-slot skill and keeps the authored rate.  A skill whose tooltip shows no
        # damage keeps its authored rate too rather than dropping to zero.
        if (entry["characterClass"] in DAMAGE_RATE_CLASSES
                and entry["inputSlot"] != "LMB"
                and entry["serverDamageProfileId"]):
            level = REFERENCE_SKILL_LEVEL
            levels = {int(r["SecondaryKey"]) for r in skills.execute(
                "SELECT SecondaryKey FROM Skill WHERE PrimaryKey = ?", (skill_id,))}
            if REFERENCE_SKILL_LEVEL not in levels and levels:
                level = max(levels)
            coefficient, addend = tooltip_damage(messages, effects, skill_id, level)
            if coefficient > 0 or addend > 0:
                row["damageProfileId"] = entry["serverDamageProfileId"]
                row["attackCoefficientBp"] = coefficient
                row["damageAddend"] = addend
        rows.append(row)
    return rows


def build_skill_buffs(
    effects: sqlite3.Connection,
    buffs: sqlite3.Connection,
    messages: sqlite3.Connection,
) -> list[dict]:
    rows = []
    for skill_id, buff_id, icon in SKILL_BUFFS:
        buff = buffs.execute("SELECT * FROM SkillBuff WHERE PrimaryKey = ?",
                             (buff_id,)).fetchone()
        if buff is None:
            raise ValueError(f"EFTable_SkillBuff has no row {buff_id}")
        effect = effects.execute(
            "SELECT Target FROM SkillEffect WHERE PrimaryKey BETWEEN ? AND ? "
            "AND Key = 7 AND ValueA = ? LIMIT 1",
            (skill_id * 10, skill_id * 10 + 9, buff_id)).fetchone()
        if effect is None:
            raise ValueError(f"Skill {skill_id} does not apply buff {buff_id}")
        target = BUFF_TARGET.get(int(effect["Target"]))
        if target is None:
            raise ValueError(f"Buff {buff_id} has unknown target {effect['Target']}")
        # A passive-option buff names its stat and carries hundredths of a percent in
        # up to four slots; the damage-amplify archetype names no stat and puts the same
        # hundredths in ValueH, because it only raises the damage its holder takes.
        # Two slots of the same stat family (physical and magical) are one field.
        fields: dict[str, int] = {}
        for slot in range(4):
            if int(buff[f"PassiveOptionType{slot}"] or 0) == 0:
                continue
            key_stat = int(buff[f"PassiveOptionKeyStat{slot}"] or 0)
            field = BUFF_STAT_FIELD.get(key_stat)
            if field is None:
                raise ValueError(f"Buff {buff_id} drives unknown stat {key_stat}")
            percent = round(int(buff[f"PassiveOptionValue{slot}"] or 0) / 100)
            if field in fields and fields[field] != percent:
                raise ValueError(f"Buff {buff_id} sets {field} twice with different values")
            fields[field] = percent
        if not fields:
            fields["damageTakenPercent"] = round(int(buff["ValueH"] or 0) / 100)
        if not any(fields.values()):
            raise ValueError(f"Buff {buff_id} has no percent to apply")
        duration = int(buff["Duration"])
        name = messages.execute(
            "SELECT MSG FROM GameMsg WHERE KEY = ?", (str(buff["Name"]),)).fetchone()
        rows.append({
            "skillId": skill_id,
            "buffId": buff_id,
            "displayName": "" if name is None else str(name["MSG"]),
            "target": target,
            "kind": "DEBUFF" if target == "ENEMY" else "BUFF",
            "durationMs": CONDITION_BUFF_DURATION_MS if duration < 0 else duration,
            "iconAsset": f"UI/HUD/Buff/buff_{icon}.png",
            **fields,
        })
    return rows


def build_bosses(
    balances: sqlite3.Connection,
    stats: sqlite3.Connection,
    authored: list[dict],
) -> list[dict]:
    rows = []
    for entry in authored:
        archetype = entry["archetypeId"]
        npc_id = BOSS_NPC_KEY.get(archetype)
        if npc_id is None:
            continue
        resolved = resolve_npc(balances, stats, npc_id)
        rows.append({
            "archetypeId": archetype,
            "sourceNpcId": resolved["npcId"],
            "maximumHp": resolved["maximumHp"],
            "attackPower": resolved["attackPower"],
            "maximumHealthBars": resolved["maximumHealthBars"],
            "staggerGaugeMaximum": resolved["staggerGaugeMaximum"],
        })
    return rows


def build_monsters(
    balances: sqlite3.Connection,
    stats: sqlite3.Connection,
    authored: list[dict],
    catalog: list[dict],
) -> list[dict]:
    npc_by_archetype: dict[str, int] = dict(EXTRA_MONSTER_NPC_KEY)
    for entry in catalog:
        asset = str(entry.get("modelAssetId", ""))
        marker = "/NPC_"
        if marker not in asset:
            continue
        digits = asset.split(marker, 1)[1].split("_", 1)[0]
        if digits.isdigit():
            npc_by_archetype.setdefault(entry["archetypeId"], int(digits))
    rows = []
    for entry in authored:
        archetype = entry["archetypeId"]
        npc_id = npc_by_archetype.get(archetype)
        if npc_id is None:
            continue
        try:
            resolved = resolve_npc(balances, stats, npc_id)
        except ValueError:
            continue
        rows.append({
            "archetypeId": archetype,
            "sourceNpcId": resolved["npcId"],
            "maxHp": resolved["maximumHp"],
            "attackPower": resolved["attackPower"],
        })
    return rows


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--table-root", type=Path, required=True,
                        help="Directory holding the unpacked EFTable_*.db files")
    parser.add_argument("--message-root", type=Path, default=None,
                        help="Directory holding EFTable_GameMsg.db (defaults to --table-root)")
    parser.add_argument("--project-root", type=Path,
                        default=Path(__file__).resolve().parents[2])
    parser.add_argument("--output", type=Path, default=None)
    args = parser.parse_args()

    if args.message_root is None:
        args.message_root = args.table_root
    balance_dir = args.project_root / "Data/Balance"
    output = args.output or balance_dir / "Profiles/Retail.balanceprofile.json"

    players_document = read_json(balance_dir / "PlayerProfiles.json")
    skills_document = read_json(balance_dir / "PlayerSkills.json")
    bosses_document = read_json(balance_dir / "BossProfiles.json")
    monsters_document = read_json(balance_dir / "MonsterProfiles.json")
    monster_catalog = read_json(args.project_root / "Data/Actors/MonsterCatalog.json")

    pcs = connect(args.table_root, "PC")
    skills = connect(args.table_root, "Skill")
    effects = connect(args.table_root, "SkillEffect")
    buffs = connect(args.table_root, "SkillBuff")
    messages = connect(args.message_root, "GameMsg")
    balances = connect(args.table_root, "NpcBalance")
    stats = connect(args.table_root, "NpcStat")

    profile = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "profileId": PROFILE_ID,
        "displayName": "원작 수치 (발탄 1415 / 쿠크세이튼 1475, 기준 스펙 아이템 레벨 1500)",
        "staggerGaugeScale": PROJECT_POLICY["staggerGaugeScale"],
        "madnessGaugeAddPercent": PROJECT_POLICY["madnessGaugeAddPercent"],
        "players": build_players(pcs, players_document["players"]),
        "skills": build_skills(skills, effects, messages, skills_document["skills"]),
        "damageProfiles": [],
        "skillBuffs": build_skill_buffs(effects, buffs, messages),
        "bosses": build_bosses(balances, stats, bosses_document["bosses"]),
        "monsters": build_monsters(
            balances, stats, monsters_document["profiles"],
            monster_catalog[next(
                key for key, value in monster_catalog.items()
                if isinstance(value, list)
            )],
        ),
    }

    # The Server reads the formula off the damage profile the skill points at.
    profile["damageProfiles"] = [
        {"damageProfileId": row["damageProfileId"],
         "attackCoefficientBp": row["attackCoefficientBp"],
         "damageAddend": row["damageAddend"]}
        for row in profile["skills"] if "damageProfileId" in row
    ]
    for row in profile["skills"]:
        row.pop("damageProfileId", None)
        row.pop("attackCoefficientBp", None)
        row.pop("damageAddend", None)
    output.parent.mkdir(parents=True, exist_ok=True)
    # core.autocrlf is true in this repository, so write what Git checks out and a
    # regenerated profile stays byte-identical instead of showing up as a change.
    with output.open("w", encoding="utf-8", newline="\r\n") as stream:
        json.dump(profile, stream, ensure_ascii=False, indent=2)
        stream.write("\n")
    print(
        f"{output}: {len(profile['players'])} players, {len(profile['skills'])} skills, "
        f"{len(profile['bosses'])} bosses, {len(profile['monsters'])} monsters"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

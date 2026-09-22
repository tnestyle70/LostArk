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
import sqlite3
from pathlib import Path
from typing import Any

SCHEMA = "lostark.balance-profile"
FORMAT_VERSION = 1
PROFILE_ID = "Retail"

REFERENCE_SKILL_LEVEL = 10

# EFTable_PC.PrimaryKey of the original class each playable class is built from.
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

PROJECT_POLICY = {
    # The client tables ship no final player attack power: EFTable_PCStat is published
    # with its payload columns removed.  The reference spec is the item level 1500
    # weapon attack power cell, EFTable_ItemLevelOption.MaxDam at SecondaryKey 1500
    # (LevelOptionId 23110000 / 24110000), which also lands mid-range of the attack
    # power each class needs to clear Valtan gate 1 with four players in eight minutes.
    "attackPower": 57048,
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
    # Skill.StiffnessTooltipType is a display grade, not a number: the amount is
    # computed server-side from the grade and the player's stagger stat.  These map the
    # grade onto the original 40000 gauge so four players clear a window with two
    # stagger skills each.
    "staggerByGrade": {0: 0, 1: 1500, 2: 3000, 3: 4500, 4: 7000, 5: 12000},
    # Authored SET_STAGGER_GAUGE values are 30 and 100; x400 makes the large window the
    # original 40000 of NpcBalance.ParalyzationPointMax.
    "staggerGaugeScale": 400,
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


def build_skills(skills: sqlite3.Connection, authored: list[dict]) -> list[dict]:
    stagger_by_grade = PROJECT_POLICY["staggerByGrade"]
    part_damage_by_level = PROJECT_POLICY["partDamageByLevel"]
    rows = []
    for entry in authored:
        skill_id = int(entry["skillId"])
        official = skill_row(skills, skill_id)
        grade = int(official["StiffnessTooltipType"])
        if grade not in stagger_by_grade:
            raise ValueError(f"Skill {skill_id} has unknown stagger grade {grade}")
        part_level = int(official["PartsAttackLevelTooltip"])
        if part_level not in part_damage_by_level:
            raise ValueError(f"Skill {skill_id} has unknown part level {part_level}")
        rows.append({
            "skillId": skill_id,
            "cooldownMs": int(official["Cooltime"]),
            "resourceCost": int(official["CostMp"]),
            "staggerDamage": stagger_by_grade[grade],
            "partDamage": part_damage_by_level[part_level],
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
    parser.add_argument("--project-root", type=Path,
                        default=Path(__file__).resolve().parents[2])
    parser.add_argument("--output", type=Path, default=None)
    args = parser.parse_args()

    balance_dir = args.project_root / "Data/Balance"
    output = args.output or balance_dir / "Profiles/Retail.balanceprofile.json"

    players_document = read_json(balance_dir / "PlayerProfiles.json")
    skills_document = read_json(balance_dir / "PlayerSkills.json")
    bosses_document = read_json(balance_dir / "BossProfiles.json")
    monsters_document = read_json(balance_dir / "MonsterProfiles.json")
    monster_catalog = read_json(args.project_root / "Data/Actors/MonsterCatalog.json")

    pcs = connect(args.table_root, "PC")
    skills = connect(args.table_root, "Skill")
    balances = connect(args.table_root, "NpcBalance")
    stats = connect(args.table_root, "NpcStat")

    profile = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "profileId": PROFILE_ID,
        "displayName": "원작 수치 (발탄 1415 / 쿠크세이튼 1475, 기준 스펙 아이템 레벨 1500)",
        "staggerGaugeScale": PROJECT_POLICY["staggerGaugeScale"],
        "players": build_players(pcs, players_document["players"]),
        "skills": build_skills(skills, skills_document["skills"]),
        "bosses": build_bosses(balances, stats, bosses_document["bosses"]),
        "monsters": build_monsters(
            balances, stats, monsters_document["profiles"],
            monster_catalog[next(
                key for key, value in monster_catalog.items()
                if isinstance(value, list)
            )],
        ),
    }

    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("w", encoding="utf-8", newline="\n") as stream:
        json.dump(profile, stream, ensure_ascii=False, indent=2)
        stream.write("\n")
    print(
        f"{output}: {len(profile['players'])} players, {len(profile['skills'])} skills, "
        f"{len(profile['bosses'])} bosses, {len(profile['monsters'])} monsters"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Build the reviewable field-level provenance receipt for gameplay balance.

The Smilegate LPK/SQLite/action payload stays outside Git.  This script records
only hashes, the exact source cells needed for review, project transforms, and
the authored values that the Server publisher consumes.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import sqlite3
from pathlib import Path
from typing import Any


SCHEMA = "lostark.balance-provenance-receipt"
FORMAT_VERSION = 1
REFERENCE_SKILL_LEVEL = 10
ALLOWED_BASES = {
    "OFFICIAL_EXTRACTED",
    "OFFICIAL_DERIVED",
    "OFFICIAL_SCALED",
    "PROJECT_TUNED",
    "REFERENCE_ONLY",
}
CLASS_PC_KEYS = {
    "LANCE_MASTER": 305,
    "GUNSLINGER": 512,
    "SLAYER": 112,
    "ARTIST": 602,
    "DIMENSIONMASTER": 612,
}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def read_json(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as stream:
        value = json.load(stream)
    if not isinstance(value, dict):
        raise ValueError(f"JSON root is not an object: {path}")
    return value


def query_one(database: Path, sql: str, values: tuple[Any, ...]) -> dict[str, Any] | None:
    with sqlite3.connect(database) as connection:
        connection.row_factory = sqlite3.Row
        row = connection.execute(sql, values).fetchone()
        return None if row is None else dict(row)


def query_reference_row(
    database: Path,
    sql: str,
    values: tuple[Any, ...],
    context: str,
) -> dict[str, Any] | None:
    """Select level 10, or an explicitly single-level level-1 definition.

    Awakening/basic skills in the extracted tables are fixed definitions whose
    only SecondaryKey is 1.  Any other missing-level-10 shape is ambiguous and
    must stop receipt generation instead of silently selecting an arbitrary row.
    """
    with sqlite3.connect(database) as connection:
        connection.row_factory = sqlite3.Row
        rows = [dict(row) for row in connection.execute(sql, values).fetchall()]
    if not rows:
        return None
    level_ten = [row for row in rows if int(row["SecondaryKey"]) == REFERENCE_SKILL_LEVEL]
    if level_ten:
        return level_ten[0]
    levels = {int(row["SecondaryKey"]) for row in rows}
    if levels == {1}:
        return rows[0]
    raise ValueError(
        f"{context} has no level {REFERENCE_SKILL_LEVEL} row and is not a fixed level-1 definition: "
        f"secondaryKeys={sorted(levels)}"
    )


def source_cell(
    table: str,
    row: dict[str, Any],
    column: str,
    database_sha256: str,
) -> dict[str, Any]:
    return {
        "type": "official-table-cell",
        "database": f"EFTable_{table}.db",
        "databaseSha256": database_sha256,
        "table": table,
        "primaryKey": row["PrimaryKey"],
        "secondaryKey": row["SecondaryKey"],
        "column": column,
    }


def project_source(policy_id: str) -> dict[str, str]:
    return {"type": "project-policy", "policyId": policy_id}


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--project-root", type=Path, required=True)
    parser.add_argument("--table-root", type=Path, required=True)
    parser.add_argument("--data2-lpk", type=Path, required=True)
    parser.add_argument("--data3-lpk", type=Path, required=True)
    parser.add_argument("--valtan-action", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    project_root = args.project_root.resolve()
    table_root = args.table_root.resolve()
    output = args.output.resolve()
    documents = {
        "Data/Balance/PlayerProfiles.json": read_json(project_root / "Data/Balance/PlayerProfiles.json"),
        "Data/Balance/PlayerSkills.json": read_json(project_root / "Data/Balance/PlayerSkills.json"),
        "Data/Balance/DamageProfiles.json": read_json(project_root / "Data/Balance/DamageProfiles.json"),
        "Data/Balance/BossProfiles.json": read_json(project_root / "Data/Balance/BossProfiles.json"),
        "Data/Balance/ValtanBossParts.json": read_json(
            project_root / "Data/Balance/ValtanBossParts.json"
        ),
        "Data/Encounters/Valtan/ValtanCombatObjects.json": read_json(
            project_root / "Data/Encounters/Valtan/ValtanCombatObjects.json"
        ),
        "Data/Encounters/Valtan/ValtanEncounter.json": read_json(
            project_root / "Data/Encounters/Valtan/ValtanEncounter.json"
        ),
    }
    database_paths = {
        name: table_root / f"EFTable_{name}.db"
        for name in ("PC", "Skill", "SkillEffect", "Npc", "NpcBalance", "NpcStat")
    }
    required_paths = [args.data2_lpk, args.data3_lpk, args.valtan_action, *database_paths.values()]
    missing = [str(path) for path in required_paths if not path.is_file()]
    if missing:
        raise FileNotFoundError("Missing provenance source(s): " + ", ".join(missing))

    database_hashes = {name: sha256(path) for name, path in database_paths.items()}
    data2_hash = sha256(args.data2_lpk)
    data3_hash = sha256(args.data3_lpk)
    entries: list[dict[str, Any]] = []

    def add(
        target_document: str,
        target_id: str,
        target_field: str,
        result_value: Any,
        basis: str,
        source: dict[str, Any],
        source_value: Any,
        transform: str,
        note: str = "",
    ) -> None:
        if basis not in ALLOWED_BASES:
            raise ValueError(f"Unknown basis {basis}")
        entry: dict[str, Any] = {
            "targetDocument": target_document,
            "targetId": target_id,
            "targetField": target_field,
            "basis": basis,
            "source": source,
            "sourceValue": source_value,
            "transform": transform,
            "resultValue": result_value,
        }
        if note:
            entry["note"] = note
        entries.append(entry)

    profiles_path = "Data/Balance/PlayerProfiles.json"
    for profile in documents[profiles_path]["players"]:
        character_class = profile["characterClass"]
        target_id = f"player:{character_class}"
        pc = query_one(
            database_paths["PC"],
            "SELECT * FROM PC WHERE PrimaryKey=? AND SecondaryKey=0",
            (CLASS_PC_KEYS[character_class],),
        )
        if pc is None:
            raise ValueError(f"Missing EFTable_PC row for {character_class}")
        add(profiles_path, target_id, "characterClass", character_class, "OFFICIAL_DERIVED",
            source_cell("PC", pc, "PrimaryKey", database_hashes["PC"]), pc["PrimaryKey"],
            f"class map {pc['PrimaryKey']} -> {character_class}")
        add(profiles_path, target_id, "maximumHp", profile["maximumHp"], "OFFICIAL_SCALED",
            source_cell("PC", pc, "MaxHpCon", database_hashes["PC"]), pc["MaxHpCon"],
            "round(MaxHpCon * projectBaselineCon[2500])")
        for field, policy in (
            ("maximumResource", "player-resource-pool-v1"),
            ("resourceRegenPerSecond", "player-resource-regen-v1"),
            ("attackPower", "deterministic-rate-display-baseline-v1"),
        ):
            add(profiles_path, target_id, field, profile[field], "PROJECT_TUNED",
                project_source(policy), profile[field], "identity")
        add(profiles_path, target_id, "defense", profile["defense"], "OFFICIAL_SCALED",
            source_cell("PC", pc, "DefCoefficient", database_hashes["PC"]), pc["DefCoefficient"],
            "round(DefCoefficient * projectDefenseBaseline[100])")
        add(profiles_path, target_id, "moveSpeed", profile["moveSpeed"], "OFFICIAL_DERIVED",
            source_cell("PC", pc, "MoveSpeed", database_hashes["PC"]), pc["MoveSpeed"],
            "MoveSpeed / 100")

    skills_path = "Data/Balance/PlayerSkills.json"
    skill_rows: dict[int, dict[str, Any] | None] = {}
    effect_rows: dict[int, dict[str, Any] | None] = {}
    for skill in documents[skills_path]["skills"]:
        skill_id = int(skill["skillId"])
        target_id = f"skill:{skill_id}"
        row = query_reference_row(
            database_paths["Skill"],
            "SELECT * FROM Skill WHERE PrimaryKey=? ORDER BY SecondaryKey, PrimaryKey",
            (skill_id,),
            f"EFTable_Skill skill {skill_id}",
        )
        effect = query_reference_row(
            database_paths["SkillEffect"],
            "SELECT * FROM SkillEffect WHERE PrimaryKey BETWEEN ? AND ? AND Key IN (1,2) "
            "ORDER BY SecondaryKey, PrimaryKey",
            (skill_id * 10, skill_id * 10 + 9),
            f"EFTable_SkillEffect skill {skill_id}",
        )
        skill_rows[skill_id] = row
        effect_rows[skill_id] = effect
        if row is None:
            raise ValueError(f"Missing EFTable_Skill row for admitted skill {skill_id}")
        add(skills_path, target_id, "skillId", skill_id, "OFFICIAL_EXTRACTED",
            source_cell("Skill", row, "PrimaryKey", database_hashes["Skill"]), row["PrimaryKey"], "identity")
        for field, policy in (
            ("characterClass", "project-class-roster-v1"),
            ("inputSlot", "project-quick-slot-layout-v1"),
            ("displayName", "project-localized-skill-label-v1"),
            ("actionId", "project-animation-action-id-v1"),
            ("skillKind", "project-skill-kind-v1"),
            ("actionDurationMs", "animation-reference-timing-v1"),
            ("hitTimeMs", "animation-reference-hit-timing-v1"),
            ("movementDistance", "server-movement-admission-v1"),
            ("serverDamageProfileId", "project-damage-profile-link-v1"),
            ("staggerDamage", "boss-landed-hit-stagger-v1"),
            ("partDamage", "boss-landed-hit-part-damage-v1"),
            ("counterPower", "boss-counter-power-v1"),
            ("effectId", "project-effect-binding-v1"),
        ):
            add(skills_path, target_id, field, skill[field], "PROJECT_TUNED",
                project_source(policy), skill[field], "identity")
        for field, column in (("cooldownMs", "Cooltime"), ("resourceCost", "CostMp")):
            official = row[column]
            if skill[field] == official:
                add(skills_path, target_id, field, skill[field], "OFFICIAL_EXTRACTED",
                    source_cell("Skill", row, column, database_hashes["Skill"]), official, "identity")
            else:
                add(skills_path, target_id, field, skill[field], "PROJECT_TUNED",
                    source_cell("Skill", row, column, database_hashes["Skill"]), official,
                    "project override", "Authored value differs from the selected official skill row.")
        official_range = float(row["MaxRange"]) / 100.0
        if abs(float(skill["maximumRange"]) - official_range) < 1e-6:
            add(skills_path, target_id, "maximumRange", skill["maximumRange"], "OFFICIAL_DERIVED",
                source_cell("Skill", row, "MaxRange", database_hashes["Skill"]), row["MaxRange"],
                "MaxRange / 100")
        else:
            add(skills_path, target_id, "maximumRange", skill["maximumRange"], "PROJECT_TUNED",
                source_cell("Skill", row, "MaxRange", database_hashes["Skill"]), row["MaxRange"],
                "project server hit range override")
        combo_stages = skill["comboStages"]
        add(skills_path, target_id, "comboStages.length", len(combo_stages), "PROJECT_TUNED",
            project_source("server-combo-stage-count-v1"), len(combo_stages), "identity")
        for index, stage in enumerate(combo_stages):
            for field, value in stage.items():
                add(skills_path, target_id, f"comboStages[{index}].{field}", value, "PROJECT_TUNED",
                    project_source("server-combo-stage-timing-v1"), value, "identity")

    damage_path = "Data/Balance/DamageProfiles.json"
    for profile in documents[damage_path]["profiles"]:
        damage_id = profile["damageProfileId"]
        target_id = f"damage:{damage_id}"
        add(damage_path, target_id, "damageProfileId", damage_id, "PROJECT_TUNED",
            project_source("stable-damage-profile-id-v1"), damage_id, "identity")
        rate = int(profile["damageRatePercent"])
        if damage_id.startswith("damage.player."):
            skill_id = int(damage_id.rsplit(".", 1)[1])
            effect = effect_rows.get(skill_id)
            if effect is not None and rate == int(effect["ValueA"]):
                add(damage_path, target_id, "damageRatePercent", rate, "OFFICIAL_EXTRACTED",
                    source_cell("SkillEffect", effect, "ValueA", database_hashes["SkillEffect"]),
                    effect["ValueA"], "identity")
            elif skill_id == 34620 and rate == 15102:
                anchor = effect_rows[34600]
                add(damage_path, target_id, "damageRatePercent", rate, "OFFICIAL_SCALED",
                    source_cell("SkillEffect", anchor, "ValueA", database_hashes["SkillEffect"]),
                    anchor["ValueA"], "ValueA(skill 34600) * project awakening cap[3]")
            else:
                source = project_source("project-deterministic-skill-rate-v1") if effect is None else source_cell(
                    "SkillEffect", effect, "ValueA", database_hashes["SkillEffect"]
                )
                source_value = rate if effect is None else effect["ValueA"]
                add(damage_path, target_id, "damageRatePercent", rate, "PROJECT_TUNED",
                    source, source_value, "project override")
        else:
            add(damage_path, target_id, "damageRatePercent", rate, "PROJECT_TUNED",
                project_source("valtan-pattern-rate-v2"), rate, "identity",
                "No official server pattern damage coefficient is present in the client payload.")

    boss_path = "Data/Balance/BossProfiles.json"
    npc = query_one(database_paths["Npc"], "SELECT * FROM Npc WHERE PrimaryKey=? AND SecondaryKey=0", (480007,))
    npc_balance = query_one(database_paths["NpcBalance"], "SELECT * FROM NpcBalance WHERE PrimaryKey=? AND SecondaryKey=0", (480007,))
    npc_stat = query_one(
        database_paths["NpcStat"],
        "SELECT * FROM NpcStat WHERE BalanceLevel=? AND StatScaleKey=? LIMIT 1",
        (npc_balance["BalanceLevel"], npc_balance["StatScaleKey"]),
    )
    if npc is None or npc_balance is None or npc_stat is None:
        raise ValueError("Valtan source join failed")
    for boss in documents[boss_path]["bosses"]:
        target_id = f"boss:{boss['archetypeId']}"
        for field, policy in (
            ("archetypeId", "project-boss-archetype-id-v1"),
            ("encounterId", "project-encounter-id-v1"),
            ("displayName", "project-localized-boss-label-v1"),
            ("maximumHealthBars", "project-valtan-health-bars-v1"),
            ("attackPower", "project-boss-attack-baseline-v1"),
            ("collisionRadius", "project-boss-collision-radius-v1"),
            ("phaseTwoHpPercent", "project-valtan-phase-threshold-v1"),
        ):
            add(boss_path, target_id, field, boss[field], "PROJECT_TUNED",
                project_source(policy), boss[field], "identity")
        add(boss_path, target_id, "maximumHp", boss["maximumHp"], "OFFICIAL_SCALED",
            source_cell("NpcBalance", npc_balance, "Hp_Count", database_hashes["NpcBalance"]),
            npc_balance["Hp_Count"], "Hp_Count * projectHpPerBar[375]")
        add(boss_path, target_id, "engageDistance", boss["engageDistance"], "OFFICIAL_DERIVED",
            source_cell("Npc", npc, "PursuitRange", database_hashes["Npc"]),
            npc["PursuitRange"], "PursuitRange / 100")
        add(boss_path, target_id, "moveSpeed", boss["moveSpeed"], "OFFICIAL_DERIVED",
            source_cell("Npc", npc, "MoveSpeed", database_hashes["Npc"]),
            npc["MoveSpeed"], "MoveSpeed / 100")

    boss_part_path = "Data/Balance/ValtanBossParts.json"
    boss_part_document = documents[boss_part_path]
    boss_part_root = f"boss-parts:{boss_part_document['bossArchetypeId']}"
    add(boss_part_path, boss_part_root, "bossArchetypeId",
        boss_part_document["bossArchetypeId"], "PROJECT_TUNED",
        project_source("valtan-boss-part-owner-v1"),
        boss_part_document["bossArchetypeId"], "identity")
    add(boss_part_path, boss_part_root, "parts.length",
        len(boss_part_document["parts"]), "PROJECT_TUNED",
        project_source("valtan-boss-part-count-v1"),
        len(boss_part_document["parts"]), "identity")
    for part in boss_part_document["parts"]:
        target_id = f"boss-part:{part['partId']}"
        for field, value in part.items():
            add(boss_part_path, target_id, field, value, "PROJECT_TUNED",
                project_source("valtan-boss-part-runtime-v1"), value, "identity")

    combat_object_path = "Data/Encounters/Valtan/ValtanCombatObjects.json"
    combat_object_document = documents[combat_object_path]
    combat_object_root = f"combat-objects:{combat_object_document['encounterId']}"
    add(combat_object_path, combat_object_root, "encounterId",
        combat_object_document["encounterId"], "PROJECT_TUNED",
        project_source("valtan-combat-object-owner-v1"),
        combat_object_document["encounterId"], "identity")
    add(combat_object_path, combat_object_root, "objects.length",
        len(combat_object_document["objects"]), "PROJECT_TUNED",
        project_source("valtan-combat-object-count-v1"),
        len(combat_object_document["objects"]), "identity")
    for combat_object in combat_object_document["objects"]:
        target_id = f"combat-object:{combat_object['combatObjectArchetypeId']}"
        for field, value in combat_object.items():
            add(combat_object_path, target_id, field, value, "PROJECT_TUNED",
                project_source("valtan-combat-object-runtime-v1"), value,
                "identity")

    encounter_path = "Data/Encounters/Valtan/ValtanEncounter.json"
    encounter = documents[encounter_path]
    root_id = f"encounter:{encounter['encounterId']}"
    for field in ("encounterId", "bossArchetypeId", "authority", "fixedTickHz"):
        add(encounter_path, root_id, field, encounter[field], "PROJECT_TUNED",
            project_source("server-encounter-state-machine-v1"), encounter[field], "identity")
    add(encounter_path, root_id, "states.length", len(encounter["states"]), "PROJECT_TUNED",
        project_source("server-encounter-state-machine-v1"), len(encounter["states"]), "identity")
    for index, state in enumerate(encounter["states"]):
        for field, value in state.items():
            add(encounter_path, root_id, f"states[{index}].{field}", value, "PROJECT_TUNED",
                project_source("server-encounter-state-machine-v1"), value, "identity")
    add(encounter_path, root_id, "patterns.length", len(encounter["patterns"]), "PROJECT_TUNED",
        project_source("server-valtan-pattern-v1"), len(encounter["patterns"]), "identity")
    for index, pattern in enumerate(encounter["patterns"]):
        for field, value in pattern.items():
            add(encounter_path, f"pattern:{pattern['patternId']}", f"patterns[{index}].{field}", value,
                "PROJECT_TUNED", project_source("server-valtan-pattern-v2"), value, "identity",
                "Client action payload is reference evidence only; it does not prove server timing/damage.")

    keys = [f"{entry['targetDocument']}#{entry['targetId']}.{entry['targetField']}" for entry in entries]
    if len(keys) != len(set(keys)):
        raise ValueError("Duplicate target field receipt key")
    entries.sort(key=lambda item: (item["targetDocument"], item["targetId"], item["targetField"]))
    source_files = [
        {"id": "data2.lpk", "sha256": data2_hash},
        {"id": "data3.lpk", "sha256": data3_hash},
        {"id": "MN_RPBF_01-1.loa", "sha256": sha256(args.valtan_action)},
    ] + [
        {"id": f"EFTable_{name}.db", "sha256": database_hashes[name]}
        for name in sorted(database_paths)
    ]
    result = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "sourceBuildId": f"data2-{data2_hash[:16]}_data3-{data3_hash[:16]}",
        "referenceSkillLevel": REFERENCE_SKILL_LEVEL,
        "extractorSha256": sha256(Path(__file__).resolve()),
        "sourceFiles": source_files,
        "coverage": {
            "playerProfileCount": len(documents[profiles_path]["players"]),
            "skillDefinitionCount": len(documents[skills_path]["skills"]),
            "damageProfileCount": len(documents[damage_path]["profiles"]),
            "bossProfileCount": len(documents[boss_path]["bosses"]),
            "bossPartCount": len(boss_part_document["parts"]),
            "bossCombatObjectCount": len(combat_object_document["objects"]),
            "encounterPatternCount": len(encounter["patterns"]),
            "fieldEntryCount": len(entries),
        },
        "entries": entries,
    }
    output.parent.mkdir(parents=True, exist_ok=True)
    temporary = output.with_suffix(output.suffix + ".tmp")
    temporary.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    temporary.replace(output)
    print(json.dumps(result["coverage"], ensure_ascii=False))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

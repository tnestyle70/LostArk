"""Explicit one-time import of existing Server hits into independent result colliders.

The original authored geometry is retained for every channel. Missing source hits
are imported from the existing maximumRange single-target Server behavior, not
reconstructed from an animation or Effect. Existing v4 edits are never rewritten.
"""
import copy
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
KINDS = ("DAMAGE", "COUNTER", "STAGGER")


def split_hits(hits, prefix):
    result = []
    for index, hit in enumerate(hits):
        for kind in KINDS:
            row = copy.deepcopy(hit)
            identity = f"{prefix}.hit{index + 1}.{kind.lower()}"
            row["colliderId"] = identity + ".collider"
            row["logic"] = {
                "logicId": identity + ".logic", "logicType": "DURATION",
                "judgementKind": "AREA_OVERLAP", "colliderId": row["colliderId"],
                "resultId": identity + ".result",
            }
            row["result"] = {
                "resultId": identity + ".result", "logicType": "RESULT",
                "resultKind": kind,
            }
            result.append(row)
    return result


def migrate(document, skills):
    if document["formatVersion"] == 4:
        return document
    if document["formatVersion"] != 3:
        raise ValueError("Only v3 hit shapes can be imported")
    document = copy.deepcopy(document)
    document["formatVersion"] = 4
    covered = {s["skillId"] for s in document["skills"]}
    for skill in skills:
        if (skill["characterClass"] != document["characterClass"] or
                not skill["serverDamageProfileId"] or skill["skillId"] in covered):
            continue
        def fallback(timing):
            return {"timeMs": timing["hitTimeMs"], "repeatCount": 1, "repeatMs": 0,
                    "areaType": 1, "range": skill["maximumRange"], "angle": 0,
                    "width": 0.0, "height": 1.5, "offset": 0.0, "inner": 0.0,
                    "maxTargets": 1, "pushMs": 0, "pushRange": 0.0}
        entry = {"skillId": skill["skillId"], "sourceBasis": "EXISTING_SERVER_MAXIMUM_RANGE"}
        stages = skill["comboStages"]
        if stages:
            entry["stages"] = [{"stageIndex": i, "hits": [fallback(stage)]}
                               for i, stage in enumerate(stages)
                               if skill["skillKind"] == "COMBO" or
                               (skill["skillKind"] == "HOLD" and i == 2) or
                               (skill["skillKind"] == "COUNTER" and i == 1)]
        else:
            entry["hits"] = [fallback(skill)]
        document["skills"].append(entry)
    for skill in document["skills"]:
        for stage in skill.get("stages", [skill]):
            prefix = f"skill{skill['skillId']}.stage{stage.get('stageIndex', 0)}"
            stage["hits"] = split_hits(stage["hits"], prefix + ".caster")
            for index, projectile in enumerate(stage.get("projectiles", [])):
                projectile["hits"] = split_hits(projectile["hits"], prefix + f".projectile{index + 1}")
    return document


def main():
    skills = json.loads((ROOT / "Data/Balance/PlayerSkills.json").read_text(encoding="utf-8"))["skills"]
    for path in sorted((ROOT / "Data/Animation/HitShapes").glob("*.hitshapes.json")):
        before = json.loads(path.read_text(encoding="utf-8"))
        after = migrate(before, skills)
        if after != before:
            path.write_text(json.dumps(after, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        print(f"{path.name}: {len(after['skills'])} skills, v{after['formatVersion']}")


if __name__ == "__main__":
    main()

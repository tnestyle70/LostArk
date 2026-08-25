"""Seed the two Authored Effect documents for Valtan's PART_BREAK stages.

`build_valtan_stage_effects.py` joins a stage to its source systems through the
model clip, and no source system in the Valtan extraction names
`Dmg_Parts_End_1` - the clip the PART_BREAK stages bind to. Every part
destruction system fires on `Dmg_Parts_Start_1` instead, at t=0 of the original
three-clip chain (Start 1.5s -> Loop 1.333s -> End 3.0s). So the general seeder
produces no document for PART_BREAK and never will without inventing a join.

This names the join instead of guessing it. The PART_BREAK stage is the moment a
plate comes off, and `Par_D_RPBF_PartsDestruction_01/02` is what the original
plays at that moment, so those two systems are declared as the stage's source by
hand. Everything downstream - emitter rows, slot assignment, render profile,
mesh pre-scale, seeded particle defaults - is the sibling seeder's own code, so
the elements this writes are the ones it would have written had the clip matched.

Nothing here touches `EffectCatalog.json`. A document on disk with no direct
catalog row is the contracted authoring-only state: Product consumes only the
identity-derived `Data/Effects/Authored` rows accepted by
`Validate-EffectSources.ps1`. The Effect Tool still reaches this reference
through the Valtan pattern tree by the stage naming rule. The catalog row and
the cue row are one later change, made together.

`_01` carries `mn_rpbf_01_1_mi` and `_02` carries `mn_rpbf_01_2_mi`: the same two
armour materials as `MN_RPBF_01_Parts1/Parts2.wmodel`, which are plate 0 and
plate 1. The stage cue has no plate axis, so both plates' emitters go into one
document under separate group IDs rather than being silently merged or dropped.
"""

import argparse
import copy
import os
import sys

import build_valtan_stage_effects as seeder

# (source system object name, the group ID its emitters land in)
PART_BREAK_SYSTEMS = [
    ("par_d_rpbf_partsdestruction_01", "part_break_plate_1"),
    ("par_d_rpbf_partsdestruction_02", "part_break_plate_2"),
]

PART_BREAK_DOCUMENTS = [
    ("effect.valtan.armor-break-opening.part-break",
     "VALTAN_ARMOR_BREAK_OPENING / PART_BREAK"),
    ("effect.valtan.dash-charge.part-break",
     "VALTAN_DASH_CHARGE / PART_BREAK"),
]


def read_part_break_durations(encounter):
    """Each pattern's PART_BREAK duration, keyed by the effect asset id."""
    durations = {}
    for pattern in encounter["patterns"]:
        for stage in pattern["stages"]:
            if stage["stageId"] != "PART_BREAK":
                continue
            asset_id = "effect.valtan.{}.{}".format(
                seeder.pattern_slug(pattern["actionId"]),
                seeder.stage_slug(pattern["actionId"], stage))
            durations[asset_id] = stage.get("durationMs", 0)
    return durations


def build_elements(systems, materials, resources, template, duration_ms):
    elements = []
    for name, group_id in PART_BREAK_SYSTEMS:
        system = systems.get(name)
        if system is None:
            raise SystemExit("Part destruction source system is missing: " + name)
        for _node, ordinal, textures, mesh, path in seeder.emitter_rows(
                system, materials, resources):
            if not textures and mesh is None:
                continue
            elements.append(seeder.build_element(
                template, "{}.em{:02d}".format(name, ordinal),
                "{} / emitter {:02d}".format(name, ordinal),
                group_id, textures, mesh, path, duration_ms))
    return elements


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true",
                        help="write documents; otherwise report only")
    args = parser.parse_args()

    encounter = seeder.read_json(os.path.join(
        seeder.DATA, "Encounters", "Valtan", "ValtanEncounter.json"))
    catalog = seeder.read_json(os.path.join(
        seeder.DATA, "Effects", "Imported", "Valtan",
        "Valtan.effect-resource-catalog.json"))
    evidence = seeder.read_json(os.path.join(
        seeder.DATA, "Effects", "Imported", "Valtan",
        "Valtan.source-material-evidence.json"))
    skeleton = seeder.read_json(seeder.SKELETON)

    systems = {row["objectName"]: row for row in catalog["sourceSystems"]}
    materials = {row["materialId"]: row for row in evidence["materials"]}
    resources = seeder.index_resources()
    durations = read_part_break_durations(encounter)
    template = skeleton["elements"][0]

    written = []
    for asset_id, display_name in PART_BREAK_DOCUMENTS:
        duration_ms = durations.get(asset_id)
        if duration_ms is None:
            raise SystemExit(
                "The encounter authors no PART_BREAK stage for " + asset_id)
        elements = build_elements(
            systems, materials, resources, template, duration_ms)
        document = copy.deepcopy(skeleton)
        document["effectAssetId"] = asset_id
        document["displayName"] = display_name
        document["elements"] = elements
        print("{:52} elements={:3}  life={}s".format(
            asset_id, len(elements), round(max(duration_ms, 1) / 1000.0, 3)))
        if args.write:
            seeder.write_json_atomic(
                os.path.join(seeder.OUT_DIR, asset_id + ".effect.json"),
                document)
        written.append(asset_id)

    if args.write:
        print("wrote {} documents to {}".format(len(written), seeder.OUT_DIR))
    else:
        print("(dry run; pass --write to emit)")
    return 0


if __name__ == "__main__":
    sys.exit(main())

"""Seed one Authored Effect document per Valtan pattern stage.

The join is the stage actionId. ValtanEncounter.json owns the stage list and
its hit shape, Valtan.patternbindings.json maps the stage actionId to a model
clip, and Valtan.effect-resource-catalog.json records which original Cascade
systems fired on that clip and which DDS/WModel each of them referenced.

Only systems that reference at most MAX_SLOT_TEXTURES textures are turned into
Elements: the authoring document has six slots, so a system carrying 21 or 42
textures cannot be expressed as one Element and is left for hand authoring.

Nothing here restores numbers. Every Element gets the neutral Detail block
from the skeleton document; only family, mesh and texture slots are filled,
because those are the parts the original data actually tells us.
"""

import argparse
import collections
import copy
import json
import os
import re
import sys

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
DATA = os.path.join(REPO, "Data")
RESOURCES = os.path.join(REPO, "Client", "Bin", "Resources")
EFFECT_ROOT = os.path.join(RESOURCES, "Effect", "Valtan")
OUT_DIR = os.path.join(DATA, "Effects", "Authored")
SKELETON = os.path.join(OUT_DIR, "valtan.test.effect.json")

MAX_SLOT_TEXTURES = 5
TEXTURE_SLOTS = ["base", "noise", "mask", "emissive", "dissolve"]


def read_json(path):
    with open(path, encoding="utf-8") as handle:
        return json.load(handle)


def normalize_clip(name):
    lowered = name.lower()
    return lowered[5:] if lowered.startswith("mesh_") else lowered


def index_resources():
    """Map a bare asset file name to its Resources-relative id."""
    index = {}
    for root, _dirs, files in os.walk(EFFECT_ROOT):
        for name in files:
            if not name.lower().endswith((".dds", ".wmodel")):
                continue
            relative = os.path.relpath(os.path.join(root, name), RESOURCES)
            index.setdefault(name.lower(), relative.replace("\\", "/"))
    return index


def resolve_slot(name):
    """Which slot a texture belongs in, by the kind token in its file name."""
    lowered = name.lower()
    if "normal" in lowered or "_n." in lowered or "_n_" in lowered:
        return "noise"
    if "noise" in lowered or "flow" in lowered or "turbulence" in lowered:
        return "noise"
    if "decal" in lowered:
        return "base"
    if any(token in lowered for token in
           ("trail", "atypical", "glow", "shine", "aura", "line")):
        return "base"
    return None


def assign_slots(textures):
    """Place textures into slots, preferred slot first then the fallback order."""
    assigned = {}
    leftovers = []
    for name in textures:
        slot = resolve_slot(name)
        if slot is not None and slot not in assigned:
            assigned[slot] = name
        else:
            leftovers.append(name)
    for name in leftovers:
        for slot in TEXTURE_SLOTS:
            if slot not in assigned:
                assigned[slot] = name
                break
    return assigned


def stage_slug(pattern_action, stage):
    action = stage["actionId"]
    prefix = pattern_action + "."
    if action.startswith(prefix) and len(action) > len(prefix):
        return action[len(prefix):]
    return stage["stageId"].lower().replace("_", "-")


def pattern_slug(pattern_action):
    parts = pattern_action.split(".")
    # valtan.attack.whirlwind -> whirlwind ; the category is shown by the tree
    return ".".join(parts[2:]) if len(parts) > 2 else parts[-1]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true",
                        help="write documents; otherwise report only")
    args = parser.parse_args()

    encounter = read_json(os.path.join(
        DATA, "Encounters", "Valtan", "ValtanEncounter.json"))
    bindings = read_json(os.path.join(
        DATA, "Animation", "Authored", "Valtan", "Valtan.patternbindings.json"))
    catalog = read_json(os.path.join(
        DATA, "Effects", "Imported", "Valtan",
        "Valtan.effect-resource-catalog.json"))
    skeleton = read_json(SKELETON)
    element_template = skeleton["elements"][0]

    clip_by_action = {b["actionId"]: b["clip"] for b in bindings["bindings"]}

    systems_by_clip = collections.defaultdict(list)
    for system in catalog["sourceSystems"]:
        for clip in (system.get("clipNames") or []):
            systems_by_clip[normalize_clip(clip)].append(system)

    assets_by_system = collections.defaultdict(list)
    for asset in catalog.get("assets", []):
        for owner in (asset.get("sourceSystems") or []):
            assets_by_system[owner.rsplit(".", 1)[-1]].append(asset)

    resources = index_resources()
    is_texture_package = re.compile(r"^fx_tex", re.IGNORECASE)

    written = skipped_wide = skipped_empty = 0
    documents = []
    for pattern in encounter["patterns"]:
        pattern_action = pattern["actionId"]
        for stage in pattern["stages"]:
            clip = clip_by_action.get(stage["actionId"])
            if not clip:
                continue
            elements = []
            for system in systems_by_clip.get(normalize_clip(clip), []):
                rows = assets_by_system.get(system["objectName"], [])
                paths = {row["sourceAssetPath"] for row in rows}
                textures, meshes = [], []
                for path in sorted(paths):
                    leaf = path.rsplit(".", 1)[-1]
                    if is_texture_package.match(path.split(".")[0]):
                        candidate = resources.get(leaf.lower() + ".dds")
                        if candidate:
                            textures.append((leaf, candidate))
                    candidate = resources.get(leaf.lower() + ".wmodel")
                    if candidate:
                        meshes.append((leaf, candidate))
                if not textures and not meshes:
                    skipped_empty += 1
                    continue
                if len(textures) > MAX_SLOT_TEXTURES:
                    skipped_wide += 1
                    continue

                element = copy.deepcopy(element_template)
                # Two packages can export the same system name, and Element IDs
                # are the document's stable save key, so disambiguate here.
                element_id = system["objectName"]
                if any(existing["id"] == element_id for existing in elements):
                    element_id = "{}.{}".format(
                        system["logicalPackage"].lower(), element_id)
                element["id"] = element_id
                element["displayName"] = system["objectName"]
                element["groupId"] = stage["stageId"].lower()
                element["kind"] = "particle"
                element["visible"] = True
                slots = []
                if meshes:
                    slots.append({"slotId": "meshModel",
                                  "assetId": meshes[0][1]})
                for slot, leaf in assign_slots(
                        [name for name, _ in textures]).items():
                    assetId = dict(textures)[leaf]
                    slots.append({"slotId": slot, "assetId": assetId})
                element["resources"] = slots
                elements.append(element)

            if not elements:
                continue
            asset_id = "effect.valtan.{}.{}".format(
                pattern_slug(pattern_action), stage_slug(pattern_action, stage))
            document = copy.deepcopy(skeleton)
            document["effectAssetId"] = asset_id
            document["displayName"] = "{} / {}".format(
                pattern["patternId"], stage["stageId"])
            document["elements"] = elements
            documents.append((asset_id, pattern["patternId"],
                              stage["stageId"], clip, document))
            written += 1

    print(f"documents        {written}")
    print(f"elements         {sum(len(d[4]['elements']) for d in documents)}")
    print(f"skipped >5 tex   {skipped_wide}")
    print(f"skipped no asset {skipped_empty}")
    if not args.write:
        for asset_id, pat, st, clip, doc in documents[:10]:
            print(f"  {asset_id:52} {pat:30} {st:16} "
                  f"elements={len(doc['elements'])}")
        print("\n(dry run; pass --write to emit)")
        return 0

    for asset_id, _pat, _st, _clip, document in documents:
        path = os.path.join(OUT_DIR, asset_id + ".effect.json")
        with open(path, "w", encoding="utf-8", newline="\n") as handle:
            json.dump(document, handle, ensure_ascii=False, indent=2)
            handle.write("\n")
    print(f"\nwrote {written} documents to {OUT_DIR}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

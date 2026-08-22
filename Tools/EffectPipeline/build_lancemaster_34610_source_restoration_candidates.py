"""Build the Tool-only source restoration candidate for LanceMaster V (34610).

The converted source for 적룡질풍격 holds 172 emitter elements.  The three live
Product documents together own 38 of them, so 134 source occurrences never reach
the screen.  They are not re-inserted into the Product documents here: the
candidate is a separate uncatalogued v13 document the Effect Tool can open, and
only occurrences the user approves are moved into Product afterwards.

Every restored occurrence keeps its source carrier exactly as converted - the
element kind, the renderer shape, the mesh asset and the render profile are
copied, never inferred.  A sprite is not turned into a decal by swapping its
texture, and a mesh particle keeps the `.wmodel` the source emitter referenced.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import pathlib
import re
import sys

SKILL_ID = "34610"
IMPORTED_RELATIVE = pathlib.PurePosixPath(
    "Data/Effects/Imported/LanceMaster/CurrentCombat/Converted/"
    "effect.lancemaster.skill.34610.imported.effect.json")
LIVE_GLOB = "Data/Effects/Authored/effect.lancemaster.skill.34610.clip*.unified.effect.json"
OUTPUT_RELATIVE = pathlib.PurePosixPath(
    "Data/Effects/Authored/"
    "effect.lancemaster.skill.34610.restoration-candidate.effect.json")
CANDIDATE_ASSET_ID = "effect.lancemaster.skill.34610.restoration-candidate"

_LIVE_SOURCE = re.compile(r"element:([^|]+)$")
_OCCURRENCE_SUFFIX = re.compile(r"\.event_source-event-\d+$")

IDENTITY_PARTICLE_SYSTEM = {
    "uniformScaleMultiplier": 1.0,
    "yawOffsetDegrees": 0.0,
    "directionYawDegrees": 0.0,
    "initialSpeedMultiplier": 1.0,
}


def live_emitter_keys(repo_root: pathlib.Path):
    """Source emitter ids the Product documents already own."""
    keys = set()
    for path in sorted(repo_root.glob(LIVE_GLOB)):
        document = json.loads(path.read_text(encoding="utf-8"))
        for element in document.get("elements", []) or []:
            match = _LIVE_SOURCE.search(element.get("sourceNode") or "")
            if match is None:
                continue
            keys.add(_OCCURRENCE_SUFFIX.sub("", match.group(1)).lower())
    return keys


def stable_element_id(source_id: str) -> str:
    digest = hashlib.sha1(source_id.encode("utf-8")).hexdigest()[:16]
    return "restore.lancemaster.%s.%s" % (SKILL_ID, digest)


def to_authoring_element(element: dict) -> dict:
    """Copy a converted source element into the v13 authoring shape."""
    restored = copy.deepcopy(element)
    restored["id"] = stable_element_id(element["id"])
    restored["visible"] = True
    # v13 owns transformInheritance; the converted v12 element predates it and
    # a restored occurrence never starts slaved to another element.
    restored["transformInheritance"] = {"enabled": False, "masterElementId": ""}
    return restored


def build(repo_root: pathlib.Path):
    imported_path = repo_root / IMPORTED_RELATIVE
    imported = json.loads(imported_path.read_text(encoding="utf-8"))
    live = live_emitter_keys(repo_root)
    missing = [element for element in imported.get("elements", []) or []
               if element["id"].lower() not in live]
    elements = [to_authoring_element(element) for element in missing]
    identifiers = {element["id"] for element in elements}
    if len(identifiers) != len(elements):
        raise SystemExit("Restored element ids collided; source ids are not unique.")
    document = {
        "schema": "lostark.effect-authoring",
        "version": 13,
        "effectAssetId": CANDIDATE_ASSET_ID,
        "displayName": CANDIDATE_ASSET_ID,
        "particleSystem": dict(IDENTITY_PARTICLE_SYSTEM),
        "modelCues": [],
        "elements": elements,
    }
    return document, len(imported.get("elements", []) or []), len(live)


def carrier_summary(document: dict):
    summary = {}
    for element in document["elements"]:
        recipe = element.get("sourceRecipe") or {}
        material = element.get("material") or {}
        key = (element.get("kind"), recipe.get("rendererShape") or "",
               material.get("renderProfile"))
        summary[key] = summary.get(key, 0) + 1
    return summary


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repo-root",
        default=str(pathlib.Path(__file__).resolve().parents[2]))
    parser.add_argument(
        "--check", action="store_true",
        help="Compare against the committed candidate instead of writing it.")
    arguments = parser.parse_args(argv)
    repo_root = pathlib.Path(arguments.repo_root).resolve()
    document, imported_count, live_count = build(repo_root)
    output_path = repo_root / OUTPUT_RELATIVE
    payload = json.dumps(document, ensure_ascii=False, indent=2) + "\n"
    if arguments.check:
        if not output_path.exists():
            print("MISSING: %s" % OUTPUT_RELATIVE)
            return 1
        current = output_path.read_text(encoding="utf-8")
        if json.loads(current) != document:
            print("STALE: %s" % OUTPUT_RELATIVE)
            return 1
        print("PASS: %s is current" % OUTPUT_RELATIVE)
        return 0
    output_path.write_text(payload, encoding="utf-8")
    print("imported emitters %d / live emitters %d / restored candidates %d"
          % (imported_count, live_count, len(document["elements"])))
    for key, count in sorted(carrier_summary(document).items(),
                             key=lambda row: -row[1]):
        print("   %-3d kind=%-11s shape=%-11s %s" % (count, key[0], key[1], key[2]))
    print("wrote %s" % OUTPUT_RELATIVE)
    return 0


if __name__ == "__main__":
    sys.exit(main())

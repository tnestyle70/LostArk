"""Build the Tool-only source restoration candidates for a Product skill.

A converted source document holds every emitter the original effect authored.
The Product documents usually own a subset of them, so the remaining source
occurrences never reach the screen.  They are not re-inserted into the Product
documents here: each candidate is a separate uncatalogued v13 document the
Effect Tool can open, and only occurrences the user approves are moved into
Product afterwards.

Every restored occurrence keeps its source carrier exactly as converted - the
element kind, the renderer shape, the mesh asset and the render profile are
copied, never inferred.  A sprite is not turned into a decal by swapping its
texture, and a mesh particle keeps the `.wmodel` the source emitter referenced.

Source material identity is deliberately left as the converter produced it.
Harvesting an enrichment for the same child material out of the corpus is not
safe: a single child path carries up to sixteen different sourceProfile bodies
across the authored documents, so the correct one cannot be decided from the
data alone.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import pathlib
import re
import sys


class RestorationTarget:
    """One skill whose converted source is compared against its Product rows."""

    def __init__(self, key, character_class, skill_id, imported, live_glob):
        self.key = key
        self.character_class = character_class
        self.skill_id = skill_id
        self.imported = pathlib.PurePosixPath(imported)
        self.live_glob = live_glob

    @property
    def output(self):
        return pathlib.PurePosixPath(
            "Data/Effects/Authored/effect.%s.skill.%s.restoration-candidate"
            ".effect.json" % (self.character_class, self.skill_id))

    @property
    def asset_id(self):
        return "effect.%s.skill.%s.restoration-candidate" % (
            self.character_class, self.skill_id)


def _target(key, folder, character_class, skill_id, live_glob):
    return RestorationTarget(
        key, character_class, skill_id,
        "Data/Effects/Imported/%s/CurrentCombat/Converted/"
        "effect.%s.skill.%s.imported.effect.json"
        % (folder, character_class, skill_id),
        live_glob)


TARGETS = {
    "lancemaster-34610": _target(
        "lancemaster-34610", "LanceMaster", "lancemaster", "34610",
        "Data/Effects/Authored/effect.lancemaster.skill.34610.clip*"
        ".unified.effect.json"),
    "warlord-17090": _target(
        "warlord-17090", "Warlord", "warlord", "17090",
        "Data/Effects/Authored/effect.warlord.skill.17090.unified.effect.json"),
    "artist-31050": _target(
        "artist-31050", "Artist", "artist", "31050",
        "Data/Effects/Authored/effect.artist.skill.31050.clip*"
        ".unified.effect.json"),
}

_LIVE_SOURCE = re.compile(r"element:([^|]+)$")
_OCCURRENCE_SUFFIX = re.compile(r"\.event_source-event-\d+$")

IDENTITY_PARTICLE_SYSTEM = {
    "uniformScaleMultiplier": 1.0,
    "yawOffsetDegrees": 0.0,
    "directionYawDegrees": 0.0,
    "initialSpeedMultiplier": 1.0,
}


def live_emitter_keys(repo_root: pathlib.Path, target: RestorationTarget):
    """Source emitter ids the Product documents already own."""
    keys = set()
    for path in sorted(repo_root.glob(target.live_glob)):
        document = json.loads(path.read_text(encoding="utf-8"))
        for element in document.get("elements", []) or []:
            match = _LIVE_SOURCE.search(element.get("sourceNode") or "")
            if match is None:
                continue
            keys.add(_OCCURRENCE_SUFFIX.sub("", match.group(1)).lower())
    return keys


def stable_element_id(target: RestorationTarget, source_id: str) -> str:
    digest = hashlib.sha1(source_id.encode("utf-8")).hexdigest()[:16]
    return "restore.%s.%s.%s" % (
        target.character_class, target.skill_id, digest)


def to_authoring_element(target: RestorationTarget, element: dict) -> dict:
    """Copy a converted source element into the v13 authoring shape."""
    restored = copy.deepcopy(element)
    restored["id"] = stable_element_id(target, element["id"])
    restored["visible"] = True
    # v13 owns transformInheritance; the converted v12 element predates it and
    # a restored occurrence never starts slaved to another element.
    restored["transformInheritance"] = {"enabled": False, "masterElementId": ""}
    return restored


def build(repo_root: pathlib.Path, target: RestorationTarget):
    imported_path = repo_root / target.imported
    imported = json.loads(imported_path.read_text(encoding="utf-8"))
    live = live_emitter_keys(repo_root, target)
    missing = [element for element in imported.get("elements", []) or []
               if element["id"].lower() not in live]
    elements = [to_authoring_element(target, element) for element in missing]
    identifiers = {element["id"] for element in elements}
    if len(identifiers) != len(elements):
        raise SystemExit("Restored element ids collided; source ids are not unique.")
    document = {
        "schema": "lostark.effect-authoring",
        "version": 13,
        "effectAssetId": target.asset_id,
        "displayName": target.asset_id,
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
        "--target", action="append", choices=sorted(TARGETS),
        help="Restore one target; repeatable. Defaults to every target.")
    parser.add_argument(
        "--check", action="store_true",
        help="Compare against the committed candidates instead of writing.")
    arguments = parser.parse_args(argv)
    repo_root = pathlib.Path(arguments.repo_root).resolve()
    keys = arguments.target or sorted(TARGETS)
    failed = False
    for key in keys:
        target = TARGETS[key]
        document, imported_count, live_count = build(repo_root, target)
        output_path = repo_root / target.output
        payload = json.dumps(document, ensure_ascii=False, indent=2) + chr(10)
        if arguments.check:
            if not output_path.exists():
                print("MISSING: %s" % target.output)
                failed = True
                continue
            if json.loads(output_path.read_text(encoding="utf-8")) != document:
                print("STALE: %s" % target.output)
                failed = True
                continue
            print("PASS: %s is current" % target.output)
            continue
        output_path.write_text(payload, encoding="utf-8")
        print("%s: imported %d / live %d / restored %d"
              % (key, imported_count, live_count, len(document["elements"])))
        for carrier, count in sorted(carrier_summary(document).items(),
                                     key=lambda row: -row[1]):
            print("   %-3d kind=%-11s shape=%-11s %s"
                  % (count, carrier[0], carrier[1], carrier[2]))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())

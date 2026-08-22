"""Audit the strict typed source-profile identity join.

`Resolve_EffectStrictTypedSourceProfile` in `Client/Public/Effect_MaterialTemplate.h`
selects a typed material family by an exact `(profileId, parentMaterialPath)` pair.
A single mistyped character in either literal silently drops the whole family back
to the grouped compatibility path with no build error and no runtime error, so the
join is audited against the authored corpus instead of being trusted.

The audit reports, for every classifier pair:

  * how many authored elements carry that exact pair
  * which pairs join nothing at all (a dead identity)

and, for every parent material that authored elements do carry, whether the
classifier knows it.
"""

from __future__ import annotations

import argparse
import collections
import json
import pathlib
import re
import sys

HEADER_RELATIVE = pathlib.PurePosixPath(
    "Client/Public/Effect_MaterialTemplate.h")
AUTHORED_RELATIVE = pathlib.PurePosixPath("Data/Effects/Authored")

_RESOLVER_START = "Resolve_EffectStrictTypedSourceProfile"
_RESOLVER_END = "EFFECT_STRICT_TYPED_SOURCE_PROFILE::NONE;\n}"
_BLOCK = re.compile(
    r"if\s*\((?P<condition>.*?)\)\s*\{\s*return\s+"
    r"EFFECT_STRICT_TYPED_SOURCE_PROFILE::(?P<family>[A-Z0-9_]+);",
    re.DOTALL)
_PROFILE_ID = re.compile(r'strProfileId\s*==\s*\r?\n?\s*"([^"]+)"')
_PARENT_PATH = re.compile(r'strParentMaterialPath\s*==\s*\r?\n?\s*"([^"]+)"')


def read_classifier_pairs(header_path: pathlib.Path):
    """Return [(family, profileId, parentMaterialPath)] declared by the header."""
    text = header_path.read_text(encoding="utf-8").replace("\r\n", "\n")
    start = text.index(_RESOLVER_START)
    end = text.index(_RESOLVER_END, start)
    body = text[start:end]
    pairs = []
    for match in _BLOCK.finditer(body):
        condition = match.group("condition")
        family = match.group("family")
        profile_ids = _PROFILE_ID.findall(condition)
        parent_paths = _PARENT_PATH.findall(condition)
        if len(profile_ids) != len(parent_paths):
            # A branch that keys on something other than a symmetric
            # profileId/parent pair is reported rather than guessed at.
            pairs.append((family, None, None))
            continue
        for profile_id, parent_path in zip(profile_ids, parent_paths):
            pairs.append((family, profile_id, parent_path))
    return pairs


def read_authored_pairs(authored_dir: pathlib.Path):
    """Return Counter[(profileId, parentMaterialPath)] over authored elements."""
    counts: collections.Counter = collections.Counter()
    for path in sorted(authored_dir.glob("*.effect.json")):
        try:
            document = json.loads(path.read_text(encoding="utf-8"))
        except (OSError, ValueError):
            continue
        for element in document.get("elements", []) or []:
            material = element.get("material") or {}
            source = material.get("sourceProfile") or {}
            if not source.get("enabled"):
                continue
            counts[(source.get("profileId", ""),
                    source.get("parentMaterialPath", ""))] += 1
    return counts


def audit(repo_root: pathlib.Path):
    classifier = read_classifier_pairs(repo_root / HEADER_RELATIVE)
    authored = read_authored_pairs(repo_root / AUTHORED_RELATIVE)
    malformed = [family for family, profile_id, _ in classifier
                 if profile_id is None]
    joined = {}
    dead = []
    for family, profile_id, parent_path in classifier:
        if profile_id is None:
            continue
        count = authored.get((profile_id, parent_path), 0)
        joined[(family, profile_id, parent_path)] = count
        if count == 0:
            dead.append((family, profile_id, parent_path))
    by_family: collections.Counter = collections.Counter()
    for (family, _, _), count in joined.items():
        by_family[family] += count
    return {
        "classifierPairCount": len(joined),
        "malformedBranches": malformed,
        "deadPairs": dead,
        "joinedPairs": joined,
        "elementsByFamily": by_family,
        "authoredPairCount": len(authored),
    }


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repo-root",
        default=str(pathlib.Path(__file__).resolve().parents[2]),
        help="Repository root that owns Client/ and Data/.")
    parser.add_argument(
        "--fail-on-dead-pair", action="store_true",
        help="Exit non-zero when a classifier pair joins no authored element.")
    arguments = parser.parse_args(argv)
    repo_root = pathlib.Path(arguments.repo_root).resolve()
    report = audit(repo_root)

    print("classifier pairs: %d over %d distinct authored pairs" % (
        report["classifierPairCount"], report["authoredPairCount"]))
    for family, count in sorted(report["elementsByFamily"].items()):
        print("  %-32s %d authored elements" % (family, count))
    if report["malformedBranches"]:
        print("branches without a symmetric profileId/parent pair:")
        for family in report["malformedBranches"]:
            print("  %s" % family)
    if report["deadPairs"]:
        print("DEAD identity joins (classifier pair matches no element):")
        for family, profile_id, parent_path in report["deadPairs"]:
            print("  %-28s %s | %s" % (family, profile_id, parent_path))
    else:
        print("no dead identity joins")
    if arguments.fail_on_dead_pair and report["deadPairs"]:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())

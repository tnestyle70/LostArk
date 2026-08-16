"""Classify every Authored Effect Element so pruning is a decision, not a guess.

"It looks wrong" collapses several different causes: the source recipe owning
playback so no authored value applies, a fail-closed material, a zero size or
scale, an asset id that does not resolve on disk, or the Element simply being
the same source system bound twice in one document. Each needs a different
answer, so each is reported separately.

Read-only.
"""

import argparse
import collections
import csv
import glob
import json
import os
import sys

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
AUTHORED = os.path.join(REPO, "Data", "Effects", "Authored")
RESOURCES = os.path.join(REPO, "Client", "Bin", "Resources")


def resolves(asset_id):
    if not asset_id:
        return True
    return os.path.exists(os.path.join(RESOURCES, asset_id.replace("/", os.sep)))


def zero_vector(values):
    return bool(values) and all(abs(float(v)) < 1e-6 for v in values)


def classify(element):
    """Every reason this Element may not appear as authored."""
    reasons = []
    material = element.get("material") or {}
    execution = material.get("execution") or {}
    recipe = element.get("sourceRecipe") or {}
    detail = element.get("detail") or {}
    transform = detail.get("transform") or {}
    particle = detail.get("particle") or {}
    timing = detail.get("timing") or {}

    if recipe.get("enabled"):
        reasons.append("SOURCE_OWNS_PLAYBACK")
    if execution.get("failClosed"):
        reasons.append("MATERIAL_FAIL_CLOSED")
    if not element.get("visible", True):
        reasons.append("HIDDEN")
    if zero_vector(transform.get("scale") or []):
        reasons.append("ZERO_SCALE")
    if element.get("kind") == "particle":
        size = particle.get("startSize") or particle.get("vStartSize") or []
        if zero_vector(size):
            reasons.append("ZERO_START_SIZE")
        if not particle.get("maxParticles") and not particle.get("burstCount"):
            reasons.append("NO_PARTICLES")
    life = timing.get("lifeTimeSeconds")
    if life is not None and float(life) <= 0.0:
        reasons.append("ZERO_LIFETIME")

    missing = [r.get("assetId") for r in (element.get("resources") or [])
               if r.get("assetId") and not resolves(r["assetId"])]
    if missing:
        reasons.append("UNRESOLVED_ASSET")

    bound = [r for r in (element.get("resources") or []) if r.get("assetId")]
    if not bound:
        reasons.append("NO_RESOURCES")
    return reasons, missing


def binding_signature(element):
    """Two Elements with the same slots and the same assets are duplicates."""
    return tuple(sorted(
        (r.get("slotId", ""), r.get("assetId", ""))
        for r in (element.get("resources") or []) if r.get("assetId")))


def owner(name):
    stem = name[len("effect."):] if name.startswith("effect.") else name
    return stem.split(".", 1)[0]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--csv")
    args = parser.parse_args()

    rows = []
    dup_groups = 0
    dup_elements = 0
    per_owner = collections.defaultdict(lambda: collections.Counter())
    reason_totals = collections.Counter()
    missing_assets = collections.Counter()

    for path in sorted(glob.glob(os.path.join(AUTHORED, "*.effect.json"))):
        name = os.path.basename(path)
        try:
            with open(path, encoding="utf-8") as handle:
                document = json.load(handle)
        except Exception:  # noqa: BLE001
            continue
        elements = document.get("elements") or []
        by_signature = collections.defaultdict(list)
        for element in elements:
            signature = binding_signature(element)
            if signature:
                by_signature[signature].append(element.get("id"))

        duplicate_ids = set()
        for signature, ids in by_signature.items():
            if len(ids) > 1:
                dup_groups += 1
                dup_elements += len(ids) - 1
                duplicate_ids.update(ids[1:])

        who = owner(name)
        per_owner[who]["documents"] += 0
        for element in elements:
            reasons, missing = classify(element)
            for reason in reasons:
                reason_totals[reason] += 1
                per_owner[who][reason] += 1
            for asset in missing:
                missing_assets[asset] += 1
            per_owner[who]["elements"] += 1
            rows.append({
                "document": name,
                "owner": who,
                "elementId": element.get("id") or "",
                "kind": element.get("kind") or "",
                "templateId": (element.get("material") or {}).get("templateId") or "",
                "duplicate": element.get("id") in duplicate_ids,
                "reasons": "|".join(reasons),
            })
        per_owner[who]["documents"] += 1

    print("elements %d   documents %d" % (len(rows), len(set(r["document"] for r in rows))))
    print()
    print("why an Element may not appear as authored")
    for reason, count in reason_totals.most_common():
        print("   %-24s %5d   %4.1f%%" % (reason, count, 100.0 * count / max(1, len(rows))))
    print()
    print("duplicate bindings (same slots, same assets, same document)")
    print("   groups %d   redundant elements %d" % (dup_groups, dup_elements))
    print()
    print("per owner")
    header = ["elements", "SOURCE_OWNS_PLAYBACK", "MATERIAL_FAIL_CLOSED",
              "UNRESOLVED_ASSET", "NO_RESOURCES", "ZERO_START_SIZE"]
    print("   %-20s %s" % ("owner", "  ".join("%-8s" % h[:8] for h in header)))
    for who in sorted(per_owner, key=lambda k: -per_owner[k]["elements"])[:14]:
        counts = per_owner[who]
        print("   %-20s %s" % (who, "  ".join(
            "%-8d" % counts[h] for h in header)))
    if missing_assets:
        print()
        print("unresolved assets (top 10)")
        for asset, count in missing_assets.most_common(10):
            print("   %-70s %d" % (asset, count))

    if args.csv:
        with open(args.csv, "w", encoding="utf-8", newline="") as handle:
            writer = csv.DictWriter(handle, fieldnames=list(rows[0].keys()))
            writer.writeheader()
            writer.writerows(rows)
        print("\nwrote %s" % args.csv)
    return 0


if __name__ == "__main__":
    sys.exit(main())

"""Drop Elements that carry nothing new: exact duplicate bindings and empties.

A duplicate here is strict - the same slot ids bound to the same asset ids
inside the same document - so the first one is kept and the rest carry no
information the survivor does not already have. Elements with no bound
resource at all cannot draw and are dropped too.

Two things are never dropped, because they would change or break the
document rather than shrink it:
  an Element another surviving Element inherits its transform from,
  and the last Element of a document.

Read Data/Effects/Authored, rewrite in place. Everything is tracked by git,
so a bad run is one checkout away.
"""

import argparse
import collections
import glob
import json
import os
import sys

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
AUTHORED = os.path.join(REPO, "Data", "Effects", "Authored")


def binding_signature(element):
    return tuple(sorted(
        (r.get("slotId", ""), r.get("assetId", ""))
        for r in (element.get("resources") or []) if r.get("assetId")))


def transform_master(element):
    inheritance = element.get("transformInheritance") or {}
    if not inheritance.get("enabled"):
        return ""
    return inheritance.get("masterElementId") or ""


def prune(document):
    """Return (kept elements, dropped ids, reason counter)."""
    elements = document.get("elements") or []
    reasons = collections.Counter()
    if len(elements) <= 1:
        return elements, [], reasons

    # Anything still referenced as a transform master has to survive.
    masters = {transform_master(e) for e in elements}
    masters.discard("")

    seen = {}
    kept, dropped = [], []
    for element in elements:
        element_id = element.get("id") or ""
        signature = binding_signature(element)
        if element_id in masters:
            kept.append(element)
            continue
        if not signature:
            reasons["NO_RESOURCES"] += 1
            dropped.append(element_id)
            continue
        if signature in seen:
            reasons["DUPLICATE"] += 1
            dropped.append(element_id)
            continue
        seen[signature] = element_id
        kept.append(element)

    if not kept:
        # Never empty a document; keep the first Element as authored.
        kept = [elements[0]]
        dropped = [e.get("id") or "" for e in elements[1:]]
        reasons["FORCED_KEEP_FIRST"] += 1
    return kept, dropped, reasons


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true")
    args = parser.parse_args()

    totals = collections.Counter()
    changed = []
    for path in sorted(glob.glob(os.path.join(AUTHORED, "*.effect.json"))):
        name = os.path.basename(path)
        with open(path, encoding="utf-8") as handle:
            document = json.load(handle)
        before = len(document.get("elements") or [])
        kept, dropped, reasons = prune(document)
        if not dropped:
            continue
        totals.update(reasons)
        totals["elementsBefore"] += before
        totals["elementsAfter"] += len(kept)
        totals["documents"] += 1
        changed.append((name, before, len(kept)))
        if args.write:
            document["elements"] = kept
            with open(path, "w", encoding="utf-8", newline="\n") as handle:
                json.dump(document, handle, ensure_ascii=False, indent=2)
                handle.write("\n")

    print("documents touched   %d" % totals["documents"])
    print("elements  %d -> %d   (-%d, %.0f%%)" % (
        totals["elementsBefore"], totals["elementsAfter"],
        totals["elementsBefore"] - totals["elementsAfter"],
        100.0 * (totals["elementsBefore"] - totals["elementsAfter"]) /
        max(1, totals["elementsBefore"])))
    print("  duplicate bindings  %d" % totals["DUPLICATE"])
    print("  no bound resource   %d" % totals["NO_RESOURCES"])
    if totals["FORCED_KEEP_FIRST"]:
        print("  kept first to avoid an empty document  %d"
              % totals["FORCED_KEEP_FIRST"])
    print()
    for name, before, after in sorted(
            changed, key=lambda r: r[2] - r[1])[:10]:
        print("   %-58s %3d -> %3d" % (name[:58], before, after))
    if not args.write:
        print("\n(dry run; pass --write to rewrite)")
    return 0


if __name__ == "__main__":
    sys.exit(main())

"""Drop Elements that carry nothing new: byte-identical twins.

An earlier version of this pass judged two Elements identical on their
(slotId, assetId) set alone. That is not what makes an Element: placing one
texture at twelve positions, scales, particle counts and lifetimes is how an
effect is built, so the rule collapsed the spatial structure of everything it
touched. Measured over the restored corpus it would drop 4,471 Elements of
which only 6 are actually redundant - the other 4,465 differ in particle
counts, timing, transform, decal projection, material profile, or even the
Element kind itself.

So a duplicate here is now the whole Element minus its own identity. Two
Elements are the same only when every authored field agrees, at which point
the survivor really does carry everything the dropped one did.

Elements with no bound resource are reported but not dropped. Lights and
screen-post Elements draw without a texture by design - 28 of the 44 lights in
the corpus have no resource at all - so the earlier blanket drop was removing
working lighting.

Two things are never dropped: an Element another surviving Element inherits
its transform from, and the last Element of a document.

Read Data/Effects/Authored, rewrite in place. Everything is tracked by git,
so a bad run is one checkout away.
"""

import argparse
import collections
import copy
import glob
import json
import os
import sys

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
AUTHORED = os.path.join(REPO, "Data", "Effects", "Authored")

# Kinds that put pixels on screen through a bound texture or model. A light or
# a screenPost has nothing to bind, so an empty resource list is authored
# intent rather than an incomplete Element.
KINDS_NEEDING_A_RESOURCE = ("particle", "sprite", "mesh", "decal", "trail")


def element_signature(element):
    """Everything the Element says, minus who it is.

    Identity is excluded because that is exactly what a duplicate differs in;
    every other field is included because every other field changes the
    picture.
    """
    clone = copy.deepcopy(element)
    clone.pop("id", None)
    clone.pop("displayName", None)
    return json.dumps(clone, sort_keys=True, ensure_ascii=False)


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
        if not (element.get("resources") or []):
            # Counted so the report can say how many there are, then kept.
            if element.get("kind") in KINDS_NEEDING_A_RESOURCE:
                reasons["NO_RESOURCES_KEPT"] += 1
            else:
                reasons["NO_RESOURCES_BY_DESIGN"] += 1
        if element_id in masters:
            kept.append(element)
            continue
        signature = element_signature(element)
        if signature in seen:
            reasons["IDENTICAL"] += 1
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
        totals.update(reasons)
        totals["elementsSeen"] += before
        if not dropped:
            continue
        totals["elementsBefore"] += before
        totals["elementsAfter"] += len(kept)
        totals["documents"] += 1
        changed.append((name, before, len(kept)))
        if args.write:
            document["elements"] = kept
            with open(path, "w", encoding="utf-8", newline="\n") as handle:
                json.dump(document, handle, ensure_ascii=False, indent=2)
                handle.write("\n")

    print("elements read       %d" % totals["elementsSeen"])
    print("documents touched   %d" % totals["documents"])
    if totals["documents"]:
        print("elements  %d -> %d   (-%d)" % (
            totals["elementsBefore"], totals["elementsAfter"],
            totals["elementsBefore"] - totals["elementsAfter"]))
    print("  identical apart from id   %d" % totals["IDENTICAL"])
    if totals["FORCED_KEEP_FIRST"]:
        print("  kept first to avoid an empty document  %d"
              % totals["FORCED_KEEP_FIRST"])
    print()
    print("kept, not dropped:")
    print("  no resource on a drawing kind   %d   (reported, needs a look)"
          % totals["NO_RESOURCES_KEPT"])
    print("  no resource by design           %d   (light / screenPost)"
          % totals["NO_RESOURCES_BY_DESIGN"])
    if changed:
        print()
        for name, before, after in sorted(
                changed, key=lambda r: r[2] - r[1])[:10]:
            print("   %-58s %3d -> %3d" % (name[:58], before, after))
    if not args.write:
        print("\n(dry run; pass --write to rewrite)")
    return 0


if __name__ == "__main__":
    sys.exit(main())

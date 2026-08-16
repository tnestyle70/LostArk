"""Report what every Authored Effect Element actually uses.

Three different arrays surface as "slots" in the tool: the five standard
textures the ordinary shader samples, the seven Track A source lanes only
Shader_Artist31470* reads, and the material execution lane metadata. Deciding
what is duplication and what is load-bearing needs those counted separately,
per element, with the material template that decides which shader runs.

Read-only. Prints a summary and, with --csv, a row per element.
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

STANDARD_SLOTS = {"base", "noise", "mask", "emissive", "dissolve"}
MESH_SLOT = "meshModel"


def classify(document):
    rows = []
    for element in document.get("elements", []) or []:
        material = element.get("material") or {}
        template = material.get("templateId") or ""
        execution = material.get("execution") or {}
        source_material = material.get("sourceMaterial") or {}
        recipe = element.get("sourceRecipe") or {}

        bound = [r for r in (element.get("resources") or [])
                 if r.get("assetId")]
        standard = [r for r in bound if r.get("slotId") in STANDARD_SLOTS]
        mesh = [r for r in bound if r.get("slotId") == MESH_SLOT]
        other = [r for r in bound
                 if r.get("slotId") not in STANDARD_SLOTS
                 and r.get("slotId") != MESH_SLOT]

        lanes = [l for l in (execution.get("textureLanes") or [])
                 if l.get("assetId")]
        named = [t for t in (source_material.get("textures") or [])
                 if t.get("assetId")]

        rows.append({
            "elementId": element.get("id") or "",
            "kind": element.get("kind") or "",
            "visible": bool(element.get("visible")),
            "templateId": template,
            "recipeEnabled": bool(recipe.get("enabled")),
            "recipeModules": len(recipe.get("modules") or []),
            "standardSlots": len(standard),
            "meshSlot": len(mesh),
            "otherSlots": len(other),
            "executionLanes": len(lanes),
            "sourceTextures": len(named),
        })
    return rows


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--csv", help="write a per-element csv here")
    parser.add_argument("--filter", default="",
                        help="only documents whose name contains this")
    args = parser.parse_args()

    all_rows = []
    per_document = []
    for path in sorted(glob.glob(os.path.join(AUTHORED, "*.effect.json"))):
        name = os.path.basename(path)
        if args.filter and args.filter not in name:
            continue
        try:
            with open(path, encoding="utf-8") as handle:
                document = json.load(handle)
        except Exception as error:  # noqa: BLE001
            print("unreadable: %s (%s)" % (name, error))
            continue
        rows = classify(document)
        for row in rows:
            row["document"] = name
        all_rows.extend(rows)
        per_document.append((name, rows))

    templates = collections.Counter(r["templateId"] for r in all_rows)
    blocked = [r for r in all_rows if r["recipeEnabled"]]
    extra = [r for r in all_rows
             if r["executionLanes"] or r["sourceTextures"] or r["otherSlots"]]
    invisible = [r for r in all_rows if not r["visible"]]
    reconstructed = [r for r in all_rows
                     if r["templateId"] and "standard" not in r["templateId"]]

    print("documents %d   elements %d" % (len(per_document), len(all_rows)))
    print()
    print("materialTemplate")
    for template, count in templates.most_common():
        print("   %-40s %d" % (template or "(none)", count))
    print()
    print("sourceRecipe.enabled=true  %d   (authored Detail is ignored)"
          % len(blocked))
    print("invisible elements         %d" % len(invisible))
    print("elements carrying lanes or non-standard slots  %d" % len(extra))
    print("elements on a non-standard template            %d"
          % len(reconstructed))
    print()
    print("the overlap that decides what is safe to drop")
    droppable = [r for r in extra if "standard" in (r["templateId"] or "")]
    keep = [r for r in extra if "standard" not in (r["templateId"] or "")]
    print("   extra data on effect.standard  %d  <- shader never reads these"
          % len(droppable))
    print("   extra data on reconstructed    %d  <- Artist31470 reads these"
          % len(keep))

    if args.csv:
        fields = ["document", "elementId", "kind", "visible", "templateId",
                  "recipeEnabled", "recipeModules", "standardSlots",
                  "meshSlot", "otherSlots", "executionLanes", "sourceTextures"]
        with open(args.csv, "w", encoding="utf-8", newline="") as handle:
            writer = csv.DictWriter(handle, fieldnames=fields)
            writer.writeheader()
            writer.writerows(all_rows)
        print("\nwrote %s" % args.csv)
    return 0


if __name__ == "__main__":
    sys.exit(main())

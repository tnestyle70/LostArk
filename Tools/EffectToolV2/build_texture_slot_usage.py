"""Build Data/Effects/V2/TextureSlotUsage.v1.json for Effect Tool v2.

Scans every imported/authored Effect document and records, per texture file name,
how many times it was bound to each material slot (base/noise/mask/emissive/dissolve)
and which original material parameter names referenced it.

Usage (Blender bundled python is fine):
    python Tools/EffectToolV2/build_texture_slot_usage.py
"""
import collections
import glob
import json
import os
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUTPUT = os.path.join(ROOT, "Data", "Effects", "V2", "TextureSlotUsage.v1.json")
SOURCES = [
    "Data/Effects/Imported/*/Converted/*.imported.effect.json",
    "Data/Effects/Authored/*.json",
]
SLOT_ALIASES = {
    "base": "base", "base2": "base",
    "noise": "noise", "noise2": "noise",
    "mask": "mask", "mask2": "mask",
    "emissive": "emissive",
    "dissolve": "dissolve",
}


def walk(node, slots, params):
    if isinstance(node, dict):
        asset = node.get("assetId")
        if isinstance(asset, str) and asset.lower().endswith(".dds"):
            name = os.path.basename(asset).lower()
            slot = SLOT_ALIASES.get(str(node.get("slotId", "")).lower())
            if slot:
                slots[name][slot] += 1
            param = node.get("name")
            if isinstance(param, str) and "sourceObjectPath" in node:
                params[name][param.lower()] += 1
        for value in node.values():
            walk(value, slots, params)
    elif isinstance(node, list):
        for value in node:
            walk(value, slots, params)


def main():
    slots = collections.defaultdict(collections.Counter)
    params = collections.defaultdict(collections.Counter)
    files = []
    for pattern in SOURCES:
        files.extend(sorted(glob.glob(os.path.join(ROOT, pattern))))
    parsed = 0
    for path in files:
        try:
            with open(path, encoding="utf-8") as handle:
                walk(json.load(handle), slots, params)
            parsed += 1
        except (OSError, ValueError) as error:
            print(f"skip {path}: {error}", file=sys.stderr)

    names = sorted(set(slots) | set(params))
    textures = {}
    for name in names:
        entry = {slot: count for slot, count in sorted(slots[name].items())}
        if params[name]:
            entry["params"] = [p for p, _ in params[name].most_common()]
        textures[name] = entry

    document = {
        "schema": "lostark.effect-tool-v2.texture-slot-usage",
        "formatVersion": 1,
        "sourcePatterns": SOURCES,
        "sourceDocumentCount": parsed,
        "slots": ["base", "noise", "mask", "emissive", "dissolve"],
        "textures": textures,
    }
    os.makedirs(os.path.dirname(OUTPUT), exist_ok=True)
    with open(OUTPUT, "w", encoding="utf-8", newline="\n") as handle:
        json.dump(document, handle, ensure_ascii=False, indent=1)
        handle.write("\n")
    print(f"wrote {OUTPUT}: {len(textures)} textures from {parsed} documents")


if __name__ == "__main__":
    main()

"""Prepare a Movie-only lookup binding candidate; never modify source or Resources."""

import argparse
import copy
import hashlib
import json
from pathlib import Path
import struct

from PIL import Image


PREFIX = "world.object.classselect."
TARGETS = {
    "guardianknight": {"a12265.p0": (9, 11), "a12253.p4": (4, 6)},
    "artist": {
        "a12220.p0": (5, 8), "a12220.p1": (5, 8), "a12220.p2": (4, 6),
        "a12220.p3": (5, 8), "a12222.p0": (9, 11), "a12238.p0": (4, 6),
        "a12239.p0": (4, 6), "a12240.p0": (9, 11), "a12243.p0": (5, 8),
        "a12243.p1": (4, 6), "a12244.p0": (5, 8), "a12244.p1": (5, 8),
        "a12244.p2": (4, 6), "a12245.p0": (4, 6), "a12247.p0": (9, 11),
    },
    "dimensionmaster": {
        "a12269.p0": (9, 11), "a12267.p4": (4, 6), "a754.p4": (4, 6),
    },
    "lancemaster": {
        "a12241.p0": (8,), "a12241.p1": (8,), "a12241.p2": (8,),
        "a12241.p3": (8,), "a748.p0": (8,), "a749.p0": (8,),
    },
    "warlord": {
        "a12206.p5": (9, 11), "a12207.p1": (4, 6), "a12207.p2": (4, 7),
        "a12207.p3": (4, 6), "a12207.p4": (4, 7), "a726.p0": (3, 7),
    },
}
LOOKUPS = {
    "hdr07_1.dds": ("Character/SourceMaterials/efmaster_material_prologue/hdr07_1.tga", "srgb"),
    "brdf_beckmann_spec.dds": ("Character/SourceMaterials/efmaster_material_prologue/brdf_beckmann_spec.tga", "linear"),
}


def sha(payload):
    return hashlib.sha256(payload).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def compare_texture(resources, before, after, color_space):
    old_path, new_path = resources / before, resources / after
    old_bytes, new_bytes = old_path.read_bytes(), new_path.read_bytes()
    require(old_bytes[:4] == b"DDS ", f"Not DDS: {before}")
    mip_header = struct.unpack_from("<I", old_bytes, 28)[0]
    require(max(1, mip_header) == 1, f"Not a single-mip DDS: {before}")
    with Image.open(old_path) as image:
        old_size, old_rgba = image.size, image.convert("RGBA").tobytes()
    with Image.open(new_path) as image:
        new_size, new_rgba = image.size, image.convert("RGBA").tobytes()
    require(old_size == new_size == (512, 512), f"Unexpected lookup dimensions: {before}")
    require(old_rgba == new_rgba, f"Lookup mip0 RGBA differs: {before}")
    return {
        "beforeAssetId": before, "afterAssetId": after, "colorSpace": color_space,
        "beforeSha256": sha(old_bytes), "afterSha256": sha(new_bytes),
        "dimensions": list(old_size), "decodedRgbaSha256": sha(old_rgba),
        "mip0RgbaExactEqual": True, "ddsHeaderMipCount": mip_header,
        "ddsEffectiveMipCount": 1, "existingTgaLoaderMipCount": 10,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--resources", type=Path, required=True)
    parser.add_argument("--expected-source-sha256", required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    require(not args.output.exists(), "Output must be a new directory")
    source_bytes = args.source.read_bytes()
    require(sha(source_bytes) == args.expected_source_sha256, "Source freshness check failed")
    original = json.loads(source_bytes.decode("utf-8-sig"))
    candidate = copy.deepcopy(original)
    objects = {row["objectId"]: row for row in candidate["objectResources"]}
    require(len(objects) == len(candidate["objectResources"]), "Duplicate objectId")
    patches, textures = [], {}
    for class_name, actors in TARGETS.items():
        for actor, slots in actors.items():
            object_id = PREFIX + class_name + "." + actor
            row = objects[object_id]
            require("/Cinematics/ClassSelect/" in row["modelAssetId"], object_id)
            require("/Props/" not in row["modelAssetId"], object_id)
            profile = row["materialProfile"]
            require(profile["family"].startswith("source.character."), object_id)
            require("monster-" not in profile["family"], object_id)
            for slot in slots:
                matches = [t for t in profile["textures"] if t["expressionIndex"] == slot]
                require(len(matches) == 1, f"Missing/duplicate expression: {object_id}/{slot}")
                texture = matches[0]
                before = texture["assetId"]
                after, color_space = LOOKUPS[Path(before).name]
                require(texture["colorSpace"] == color_space, f"Color space: {object_id}/{slot}")
                if before not in textures:
                    textures[before] = compare_texture(args.resources, before, after, color_space)
                patches.append({
                    "objectId": object_id, "expressionIndex": slot,
                    "field": "assetId", "beforeAssetId": before, "afterAssetId": after,
                    "colorSpace": color_space, "modelAssetId": row["modelAssetId"],
                    "sourceMaterial": profile["sourceMaterial"], "family": profile["family"],
                })
                texture["assetId"] = after
    restored = copy.deepcopy(candidate)
    restored_objects = {row["objectId"]: row for row in restored["objectResources"]}
    for patch in patches:
        ts = restored_objects[patch["objectId"]]["materialProfile"]["textures"]
        next(t for t in ts if t["expressionIndex"] == patch["expressionIndex"])["assetId"] = patch["beforeAssetId"]
    require(restored == original, "Unexpected changes outside permitted assetId fields")
    require(args.source.read_bytes() == source_bytes, "Source changed while preparing candidate")
    for record in textures.values():
        for key in ("before", "after"):
            require(sha((args.resources / record[key + "AssetId"]).read_bytes()) == record[key + "Sha256"], "Resource changed")
    payload = (json.dumps(candidate, ensure_ascii=False, separators=(",", ":")) + "\n").encode("utf-8")
    receipt = {
        "schema": "lostark.movie-lookup-mip-candidate", "formatVersion": 1,
        "sourcePath": str(args.source.resolve()), "sourceSha256": sha(source_bytes),
        "candidateSha256": sha(payload), "objectCount": len({p["objectId"] for p in patches}),
        "fieldCount": len(patches), "texturePairCount": len(textures),
        "newResources": [], "onlyTextureAssetIdFieldsChanged": True,
        "textures": list(textures.values()), "patches": patches,
    }
    args.output.mkdir(parents=True)
    candidate_path = args.output / "Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00" / args.source.name
    candidate_path.parent.mkdir(parents=True)
    candidate_path.write_bytes(payload)
    (args.output / "source-baseline.json").write_bytes(source_bytes)
    (args.output / "lookup-mip-receipt.json").write_text(json.dumps(receipt, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({k: v for k, v in receipt.items() if k not in ("textures", "patches")}, indent=2))


if __name__ == "__main__":
    main()

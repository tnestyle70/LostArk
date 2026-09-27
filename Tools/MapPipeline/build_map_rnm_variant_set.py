#!/usr/bin/env python3
"""Build the catalog, placement, material and placementLighting set for RNM lightmap binding.

A map material row's bakedLighting carries only the atlas pair, while every per-placement value
(coordinateScale, coordinateBias, averageScale, directionalScale) belongs to a placementLighting
instance. So an asset whose placements reference more than one atlas pair needs one material row
variant per pair, and the placements of each pair must point at the variant that carries it.

A variant catalog row reuses the base asset's installed .wmodel, so no geometry is re-cooked and no
new Resources folder is created. The base assetId keeps the pair with the most placements.

Placements whose source component reports no lightmap are never given a pair. Assets with no material
row cannot carry bakedLighting, so their placements are reported as excluded instead of being
emitted; Publish-MapAuthoring.ps1 rejects a placementLighting row whose asset has no bakedLighting.

This module only writes the files named on the command line. It never writes Data/** in place and
never runs a publisher.
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import json
import os
from pathlib import Path
import shlex
import sys
from typing import Any

LIGHTMAP_FIELDS = ("averageTexture", "directionalTexture", "colorSpace")
PLACEMENT_LIGHTING_FIELDS = ("sourcePlacementId", "assetId", "coordinateScale",
                             "coordinateBias", "averageScale", "directionalScale")
QUOTED_CATALOG_TOKENS = (0, 1, 2, 3, 8, 9, 10)
QUOTED_PLACEMENT_TOKENS = (1, 2, 3, 4)


class VariantSetError(ValueError):
    pass


def lightmap_asset_id(source_object: str, directory: str) -> str:
    """Resources-relative id for an installed source lightmap, matching the published convention."""
    return directory.rstrip("/") + "/" + source_object.replace(".", "_").lower() + ".dds"


def variant_asset_id(base_asset_id: str, average: str, directional: str) -> str:
    """Stable id from the base asset identity and that slot's atlas pair identity."""
    token = (base_asset_id + "|" + average + "|" + directional).encode("utf-8")
    return base_asset_id + "_LM_" + hashlib.sha256(token).hexdigest()[:12].upper()


def plan_variants(rnm: list[dict[str, Any]],
                  rows_by_asset: dict[str, list[dict[str, Any]]]) -> dict[str, Any]:
    """Decide, per asset, which atlas pair the base id keeps and which pairs become variants."""
    by_asset: dict[str, list[dict[str, Any]]] = collections.defaultdict(list)
    for item in rnm:
        by_asset[item["assetId"]].append(item)
    base_pair: dict[str, tuple[str, str]] = {}
    pair_to_asset: dict[tuple[str, tuple[str, str]], str] = {}
    variant_of: dict[str, tuple[str, tuple[str, str]]] = {}
    excluded: list[str] = []
    for asset_id in sorted(by_asset):
        items = by_asset[asset_id]
        if asset_id not in rows_by_asset:
            excluded.append(asset_id)
            continue
        counts = collections.Counter((item["average"], item["directional"]) for item in items)
        ordered = sorted(counts.items(), key=lambda entry: (-entry[1], entry[0]))
        base = ordered[0][0]
        base_pair[asset_id] = base
        pair_to_asset[(asset_id, base)] = asset_id
        for pair, _ in ordered[1:]:
            identity = variant_asset_id(asset_id, pair[0], pair[1])
            pair_to_asset[(asset_id, pair)] = identity
            variant_of[identity] = (asset_id, pair)
    return dict(byAsset=by_asset, basePair=base_pair, pairToAsset=pair_to_asset,
                variantOf=variant_of, excludedNoMaterialRow=excluded)


def build_variant_set(*, catalog_rows: list[list[str]], placement_rows: list[list[str]],
                      materials: list[dict[str, Any]], rnm: list[dict[str, Any]],
                      lightmap_directory: str) -> dict[str, Any]:
    """Return the new catalog rows, placement rows, material rows and placementLighting array."""
    rows_by_asset: dict[str, list[dict[str, Any]]] = collections.defaultdict(list)
    for row in materials:
        rows_by_asset[row["assetId"]].append(row)
    catalog_by_id = {row[0]: row for row in catalog_rows}
    plan = plan_variants(rnm, rows_by_asset)
    base_pair = plan["basePair"]
    pair_to_asset = plan["pairToAsset"]
    variant_of = plan["variantOf"]

    new_catalog: list[list[str]] = []
    for identity, (base_id, _pair) in sorted(variant_of.items()):
        if base_id not in catalog_by_id:
            raise VariantSetError("variant base asset is absent from the catalog: " + base_id)
        row = list(catalog_by_id[base_id])
        row[0] = identity
        row[3] = "Prototype_Component_Model_" + identity
        new_catalog.append(row)

    repoint: dict[str, str] = {}
    for item in rnm:
        asset_id = item["assetId"]
        if asset_id not in base_pair:
            continue
        target = pair_to_asset[(asset_id, (item["average"], item["directional"]))]
        if target != asset_id:
            repoint[item["sourcePlacementId"]] = target
    new_placements: list[list[str]] = []
    for row in placement_rows:
        row = list(row)
        target = repoint.get(row[1])
        if target is not None:
            row[4] = target
        new_placements.append(row)

    def baked(pair: tuple[str, str]) -> dict[str, str]:
        return dict(averageTexture=lightmap_asset_id(pair[0], lightmap_directory),
                    directionalTexture=lightmap_asset_id(pair[1], lightmap_directory),
                    colorSpace="linear")

    new_materials: list[dict[str, Any]] = []
    baked_assets: set[str] = set()
    for row in materials:
        updated = dict(row)
        pair = base_pair.get(row["assetId"])
        if pair is not None:
            updated["bakedLighting"] = baked(pair)
            baked_assets.add(row["assetId"])
        new_materials.append(updated)
    for identity, (base_id, pair) in sorted(variant_of.items()):
        for row in rows_by_asset[base_id]:
            updated = dict(row)
            updated["assetId"] = identity
            updated["bakedLighting"] = baked(pair)
            new_materials.append(updated)
        baked_assets.add(identity)

    placement_lighting: list[dict[str, Any]] = []
    for item in sorted(rnm, key=lambda entry: entry["sourcePlacementId"]):
        asset_id = item["assetId"]
        if asset_id not in base_pair:
            continue
        placement_lighting.append(dict(
            sourcePlacementId=item["sourcePlacementId"],
            assetId=pair_to_asset[(asset_id, (item["average"], item["directional"]))],
            coordinateScale=item["coordinateScale"], coordinateBias=item["coordinateBias"],
            averageScale=item["averageScale"], directionalScale=item["directionalScale"]))

    return dict(catalogAdditions=new_catalog, placements=new_placements,
                materials=new_materials, placementLighting=placement_lighting,
                bakedAssets=baked_assets, repointed=repoint, plan=plan)


def validate_set(*, catalog_rows: list[list[str]], catalog_additions: list[list[str]],
                 placement_rows: list[list[str]], materials: list[dict[str, Any]],
                 placement_lighting: list[dict[str, Any]], baked_assets: set[str],
                 source_absent_ids: set[str], resources_root: Path | None) -> list[str]:
    """Check the emitted set against the rules Publish-MapAuthoring.ps1 enforces."""
    problems: list[str] = []
    all_catalog = catalog_rows + catalog_additions
    identities = [row[0] for row in all_catalog]
    for identity, count in collections.Counter(identities).items():
        if count > 1:
            problems.append("duplicate catalog assetId: " + identity)
    models: dict[str, set[str]] = collections.defaultdict(set)
    for row in all_catalog:
        models[row[0]].add(row[2])
    for identity, paths in models.items():
        if len(paths) > 1:
            problems.append("conflicting model paths for asset: " + identity)
    catalog_ids = set(identities)

    seen_rows: set[tuple[str, str]] = set()
    for row in materials:
        key = (row["assetId"], row["materialName"])
        if key in seen_rows:
            problems.append("duplicate material row: %s|%s" % key)
        seen_rows.add(key)
        if row["assetId"] not in catalog_ids:
            problems.append("material asset is absent from the catalog: " + row["assetId"])
        lighting = row.get("bakedLighting")
        if lighting is None:
            continue
        if tuple(sorted(lighting)) != tuple(sorted(LIGHTMAP_FIELDS)):
            problems.append("bakedLighting field set violation: " + row["assetId"])
            continue
        if lighting["colorSpace"] not in ("linear", "srgb"):
            problems.append("invalid lightmap colour space: " + row["assetId"])
        for field in ("averageTexture", "directionalTexture"):
            value = lighting[field]
            if (not isinstance(value, str) or not value.strip() or os.path.isabs(value)
                    or ":" in value or ".." in value.replace("\\", "/").split("/")
                    or os.path.splitext(value)[1].lower() != ".dds"):
                problems.append("lightmap must be a Resources-relative DDS: %r" % (value,))
                continue
            if resources_root is not None:
                target = resources_root / value.replace("/", os.sep)
                if not target.is_file():
                    problems.append("lightmap is missing: " + value)

    published = {row[1]: row[4] for row in placement_rows}
    seen_sources: set[str] = set()
    for item in placement_lighting:
        if tuple(sorted(item)) != tuple(sorted(PLACEMENT_LIGHTING_FIELDS)):
            problems.append("placementLighting field set violation: "
                            + str(item.get("sourcePlacementId")))
            continue
        source_id = item["sourcePlacementId"]
        if item["assetId"] not in baked_assets:
            problems.append("placementLighting asset has no bakedLighting: " + item["assetId"])
        if source_id in seen_sources:
            problems.append("duplicate placementLighting source: " + source_id)
        seen_sources.add(source_id)
        if source_id not in published:
            problems.append("placementLighting source is not published: " + source_id)
        elif published[source_id] != item["assetId"]:
            problems.append("placementLighting identity mismatch: " + source_id)
        if source_id in source_absent_ids:
            problems.append("RNM attached to a placement with no source lighting: " + source_id)
        for field, count in (("coordinateScale", 2), ("coordinateBias", 2),
                            ("averageScale", 3), ("directionalScale", 3)):
            values = item[field]
            if not isinstance(values, list) or len(values) != count:
                problems.append("%s component count violation: %s" % (field, source_id))
                continue
            for value in values:
                if not isinstance(value, (int, float)) or isinstance(value, bool) or value < 0:
                    problems.append("%s value violation: %s" % (field, source_id))
        scale, bias = item["coordinateScale"], item["coordinateBias"]
        if isinstance(scale, list) and isinstance(bias, list) and len(scale) == len(bias) == 2:
            for axis in range(2):
                if not all(isinstance(v, (int, float)) for v in (scale[axis], bias[axis])):
                    continue
                if scale[axis] + bias[axis] > 1.00001:
                    problems.append("atlas bounds exceeded: %s axis %d" % (source_id, axis))
    return problems


def read_json(path: Path) -> Any:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def read_rows(path: Path) -> tuple[str, list[list[str]]]:
    lines = path.read_text(encoding="utf-8").splitlines()
    return lines[0], [shlex.split(line) for line in lines[1:] if line.strip()]


def write_rows(path: Path, header: str, rows: list[list[str]], quoted: tuple[int, ...]) -> None:
    parts = shlex.split(header)
    body = ['%s %s "%s" %d' % (parts[0], parts[1], parts[2], len(rows))]
    for row in rows:
        body.append(" ".join('"%s"' % row[i] if i in quoted else row[i]
                             for i in range(len(row))))
    path.write_text("\n".join(body) + "\n", encoding="utf-8", newline="\n")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--area-id", required=True)
    parser.add_argument("--catalog", type=Path, required=True)
    parser.add_argument("--placements", type=Path, required=True)
    parser.add_argument("--materials", type=Path, required=True,
                        help="candidate map materials document to add lighting to")
    parser.add_argument("--rnm", type=Path, required=True,
                        help="recalculated RNM placements with average/directional and scales")
    parser.add_argument("--lightmap-directory", default="Map/Lighting/Maharaka")
    parser.add_argument("--resources-root", type=Path, default=None)
    parser.add_argument("--output-directory", type=Path, required=True)
    args = parser.parse_args(argv)

    catalog_header, catalog_rows = read_rows(args.catalog)
    placement_header, placement_rows = read_rows(args.placements)
    document = read_json(args.materials)
    recalculated = read_json(args.rnm)
    rnm = recalculated["rnm"] if isinstance(recalculated, dict) else recalculated
    absent = recalculated.get("sourceAbsent", []) if isinstance(recalculated, dict) else []
    absent_ids = {item["sourcePlacementId"] for item in absent}

    try:
        built = build_variant_set(catalog_rows=catalog_rows, placement_rows=placement_rows,
                                  materials=document["materials"], rnm=rnm,
                                  lightmap_directory=args.lightmap_directory)
    except VariantSetError as error:
        print("variant set build failed: %s" % error, file=sys.stderr)
        return 1

    problems = validate_set(catalog_rows=catalog_rows,
                            catalog_additions=built["catalogAdditions"],
                            placement_rows=built["placements"],
                            materials=built["materials"],
                            placement_lighting=built["placementLighting"],
                            baked_assets=built["bakedAssets"],
                            source_absent_ids=absent_ids,
                            resources_root=args.resources_root)

    args.output_directory.mkdir(parents=True, exist_ok=True)
    write_rows(args.output_directory / args.catalog.name, catalog_header,
               catalog_rows + built["catalogAdditions"], QUOTED_CATALOG_TOKENS)
    write_rows(args.output_directory / args.placements.name, placement_header,
               built["placements"], QUOTED_PLACEMENT_TOKENS)
    emitted = dict(schema="lostark.map-materials", formatVersion=2, areaId=args.area_id,
                   materials=built["materials"], placementLighting=built["placementLighting"])
    (args.output_directory / args.materials.name).write_text(
        json.dumps(emitted, ensure_ascii=False, indent=1), encoding="utf-8")

    plan = built["plan"]
    receipt = dict(
        format="lostark-map-rnm-variant-set", formatVersion=1, areaId=args.area_id,
        catalog=dict(before=len(catalog_rows), added=len(built["catalogAdditions"])),
        placements=dict(total=len(built["placements"]), repointed=len(built["repointed"])),
        materials=dict(before=len(document["materials"]), after=len(built["materials"]),
                       withBakedLighting=len([r for r in built["materials"]
                                              if r.get("bakedLighting")])),
        placementLighting=len(built["placementLighting"]),
        baseAssets=len(plan["basePair"]), variantAssets=len(plan["variantOf"]),
        excludedNoMaterialRow=len(plan["excludedNoMaterialRow"]),
        sourceAbsentPlacements=len(absent_ids),
        validation=dict(problemCount=len(problems), problems=problems[:64]))
    (args.output_directory / "rnm-variant-set.receipt.json").write_text(
        json.dumps(receipt, ensure_ascii=False, indent=1), encoding="utf-8")

    print("%s: catalog +%d, placements repointed %d, materials %d -> %d, placementLighting %d"
          % (args.area_id, len(built["catalogAdditions"]), len(built["repointed"]),
             len(document["materials"]), len(built["materials"]),
             len(built["placementLighting"])))
    if problems:
        print("validation problems: %d" % len(problems), file=sys.stderr)
        for message in problems[:10]:
            print("  " + message, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

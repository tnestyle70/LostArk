#!/usr/bin/env python3
"""Split existing RNM material variants by their source static-shadow binding.

Geometry is reused. A placement without a source shadow gets a separate material
when necessary, so it never inherits another placement's shadow atlas. Inputs
are staged in memory; unknown placements or missing RNM are errors.
"""

from __future__ import annotations

import argparse
import collections
import copy
import hashlib
import json
from pathlib import Path

import build_map_rnm_variant_set as rnm


def build_shadow_variants(*, catalog_rows, placement_rows, materials,
                          placement_lighting, shadows):
    catalogs = copy.deepcopy(catalog_rows)
    placements = copy.deepcopy(placement_rows)
    output_materials = copy.deepcopy(materials)
    lighting = copy.deepcopy(placement_lighting)
    catalog_by_id = {row[0]: row for row in catalogs}
    placement_by_id = {row[1]: row for row in placements}
    lighting_by_id = {row['sourcePlacementId']: row for row in lighting}
    if len(catalog_by_id) != len(catalogs) or len(placement_by_id) != len(placements):
        raise rnm.VariantSetError('Duplicate catalog or placement identity')
    if len(lighting_by_id) != len(lighting):
        raise rnm.VariantSetError('Duplicate placementLighting identity')
    material_by_id = collections.defaultdict(list)
    for row in output_materials:
        material_by_id[row['assetId']].append(row)
        if 'staticShadow' in row.get('bakedLighting', {}):
            raise rnm.VariantSetError('Input already contains staticShadow bindings')
    shadow_by_id = {}
    for row in shadows:
        identity = row['sourcePlacementId']
        if identity in shadow_by_id:
            raise rnm.VariantSetError('Duplicate source shadow: ' + identity)
        if identity not in placement_by_id or identity not in lighting_by_id:
            raise rnm.VariantSetError('Source shadow requires a published RNM placement: ' + identity)
        asset = placement_by_id[identity][4]
        if lighting_by_id[identity]['assetId'] != asset:
            raise rnm.VariantSetError('Shadow/RNM asset mismatch: ' + identity)
        if not material_by_id[asset] or any('bakedLighting' not in item for item in material_by_id[asset]):
            raise rnm.VariantSetError('Source shadow requires every named material to carry RNM: ' + asset)
        shadow_by_id[identity] = row
    groups = collections.defaultdict(lambda: collections.defaultdict(list))
    for row in placements:
        shadow = shadow_by_id.get(row[1])
        signature = json.dumps(shadow['staticShadow'], sort_keys=True) if shadow else ''
        groups[row[4]][signature].append(row[1])
    repointed = {}
    for asset, signatures in sorted(groups.items()):
        if asset not in catalog_by_id:
            raise rnm.VariantSetError('Placement asset missing from catalog: ' + asset)
        ordered = sorted(signatures, key=lambda key: (-len(signatures[key]), key))
        # Clone all rows before adding shadow fields to any base row.
        pristine = copy.deepcopy(material_by_id[asset])
        for index, signature in enumerate(ordered):
            target = asset
            target_materials = material_by_id[asset]
            if index:
                target += '_SH_' + hashlib.sha256((asset + '|' + signature).encode()).hexdigest()[:12].upper()
                if target in catalog_by_id:
                    raise rnm.VariantSetError('Static-shadow variant identity collision: ' + target)
                row = list(catalog_by_id[asset])
                row[0], row[3] = target, 'Prototype_Component_Model_' + target
                catalogs.append(row)
                catalog_by_id[target] = row
                target_materials = copy.deepcopy(pristine)
                for item in target_materials:
                    item['assetId'] = target
                output_materials.extend(target_materials)
            if signature:
                for item in target_materials:
                    item['bakedLighting']['staticShadow'] = json.loads(signature)
            for identity in signatures[signature]:
                placement_by_id[identity][4] = target
                if target != asset:
                    repointed[identity] = target
                light = lighting_by_id.get(identity)
                if light:
                    light['assetId'] = target
                    shadow = shadow_by_id.get(identity)
                    if shadow:
                        for field in ('shadowCoordinateScale', 'shadowCoordinateBias'):
                            light[field] = copy.deepcopy(shadow[field])
    return dict(catalogs=catalogs, placements=placements, materials=output_materials,
                placementLighting=lighting, repointed=repointed,
                addedVariants=len(catalogs) - len(catalog_rows),
                boundShadowPlacements=len(shadow_by_id))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('catalog', 'placements', 'materials', 'shadows', 'output-directory'):
        parser.add_argument('--' + name, required=True, type=Path)
    args = parser.parse_args()
    catalog_header, catalogs = rnm.read_rows(args.catalog)
    placement_header, placements = rnm.read_rows(args.placements)
    document = rnm.read_json(args.materials)
    result = build_shadow_variants(catalog_rows=catalogs, placement_rows=placements,
        materials=document['materials'], placement_lighting=document['placementLighting'],
        shadows=rnm.read_json(args.shadows))
    args.output_directory.mkdir(parents=True, exist_ok=True)
    rnm.write_rows(args.output_directory / args.catalog.name, catalog_header,
                   result['catalogs'], rnm.QUOTED_CATALOG_TOKENS)
    rnm.write_rows(args.output_directory / args.placements.name, placement_header,
                   result['placements'], rnm.QUOTED_PLACEMENT_TOKENS)
    document['materials'] = result['materials']
    document['placementLighting'] = result['placementLighting']
    (args.output_directory / args.materials.name).write_text(
        json.dumps(document, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(json.dumps({key: result[key] for key in ('addedVariants', 'boundShadowPlacements')}))


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Build the build_source_map_materials.py input manifest from a staged Area cook.

The compiler (build_source_map_materials.py) needs a manifest that binds each
material slot to effective MIC parameters and to exact installed DDS files. This
adapter reads the staged extraction of one Area and writes that manifest:

  * effective parameters   extract_source_map_material_parameters.py output
  * runtime slot identity  the cook's map_material_runtime_assets.json (runtimeName <-> MIC path)
  * installed textures     the recooked mip catalog + runtime manifest file hashes
  * texture color space    the source Texture2D SRGB property in the mip-recovery receipts
                           (absent means the UE3 default, true)

Only slots the deferred surface builders express and whose textures are valid DDS are
emitted, and only for assets where every slot is emitted (the scene admission binds whole
assets). Anything else stays on the legacy material path and is listed in slot_report.json
with its reason. Nothing is inferred from a file name except the documented engine-default
alias (<package>.tex.X -> catalog tex.X).

Lighting: this adapter does not bind ShadowMap2D / RNM lightmaps, but it does not claim they
are missing either. --component-lighting is required and lightingEvidence is derived per asset
from those receipts: any RNM component makes the asset `source-bound`, components that all
report no lighting record make it `source-absent`, and anything else leaves the asset
unresolved and drops its slots with a reason in slot_report.json. Binding the lightmaps needs
auxiliaryTextures plus a bakedLighting pair on every row of the asset and is out of scope here.
"""
from __future__ import annotations

import argparse
import collections
import hashlib
import json
import shlex
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import build_maptool_scene as scene  # noqa: E402
import build_source_map_materials as compiler  # noqa: E402
import source_map_surface  # noqa: E402
import source_map_surface_extra  # noqa: E402

# Uniform 2x2 PF_A8R8G8B8 engine textures (mip chain 2x2 -> 1x1). Values and SRGB flags were read
# from the source package (EFMaster_Material_Prologue.tex.*) with UModel and the UE3 property parser.
ENGINE_DEFAULTS = {
    'normal': ((127, 127, 254, 255), 'linear'),
    'flat_gray': ((128, 128, 128, 255), 'srgb'),
    'flat_normalmap': ((127, 127, 254, 229), 'linear'),
    'flat_white': ((255, 255, 255, 255), 'srgb'),
    'null': ((0, 0, 0, 255), 'srgb'),
}
ENGINE_PACKAGE_PREFIX = 'efmaster_material_prologue.tex.'
NORMAL_PARAMETERS = ('texture_normal', 'texture_detail_normal', 'texture_overlay_normal')
EXTRA_BUILDER_TERMINALS = ('.bg_base_pbr_opa', '.bg_base_pbr_seamless_opa', '.bg_base_pbr_msk',
                           '.bg_foliage_msk', '.bg_grass_msk', '.preset_overlay_snowice_opa')


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with open(path, 'rb') as stream:
        for block in iter(lambda: stream.read(1 << 20), b''):
            digest.update(block)
    return digest.hexdigest()


def read_json(path: Path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def build_dds_a8r8g8b8(rgba, size=2) -> bytes:
    """Legacy DDS, A8R8G8B8, one colour, complete mip chain down to 1x1."""
    r, g, b, a = rgba
    levels, dim = [], size
    while True:
        levels.append(bytes((b, g, r, a)) * (dim * dim))
        if dim == 1:
            break
        dim //= 2
    header = bytearray(128)
    header[0:4] = b'DDS '
    struct.pack_into('<IIIIII', header, 4, 124, 0x1 | 0x2 | 0x4 | 0x8 | 0x1000 | 0x20000, size, size, size * 4, 0)
    struct.pack_into('<I', header, 28, len(levels))
    struct.pack_into('<II', header, 76, 32, 0x1 | 0x40)
    struct.pack_into('<IIIII', header, 88, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000)
    struct.pack_into('<I', header, 108, 0x8 | 0x1000 | 0x400000)
    return bytes(header) + b''.join(levels)


class Adapter:
    def __init__(self, args):
        self.area = args.area_id
        self.resources = args.resources_root.resolve()
        self.prefix = args.runtime_area_root.rstrip('/') + '/'
        self.new_resources = args.new_resources_root
        self.engine_dir = args.engine_default_dir.strip('/') + '/'
        self.parameters_path = args.parameters
        self.runtime_path = args.runtime_manifest
        self.mip_catalog_path = args.mip_catalog
        self.inventory_path = args.asset_inventory
        params = read_json(args.parameters)
        self.materials = {k.casefold(): v for k, v in params['materials'].items()}
        self.runtime = read_json(args.runtime_manifest)
        self.catalog = {t['objectPath'].casefold(): t for t in read_json(args.mip_catalog)['textures']}
        self.recovered = {x['objectPath'].casefold() for x in read_json(args.mip_results) if x['status'] == 'recovered'}
        self.receipts = {}
        for path in args.mip_chains.glob('*.receipt.json'):
            doc = read_json(path)
            self.receipts[doc['sourceObject'].casefold()] = doc
        self.installed = {}
        for asset in self.runtime['assets']:
            for entry in asset['files']:
                if entry['path'].lower().endswith('.dds'):
                    self.installed.setdefault(entry['sha256'], self.prefix + entry['path'])
        self.cull = {}
        for line in args.imported_catalog.read_text(encoding='utf-8').splitlines()[1:]:
            tokens = shlex.split(line)
            self.cull[tokens[0]] = tokens[12].lower()
        self.model_of = {a['assetId']: a['model'] for a in self.runtime['assets']}
        self.cache = {}

    def engine_default(self, source_object):
        key = source_object.casefold()
        if not key.startswith(ENGINE_PACKAGE_PREFIX):
            return None
        leaf = key[len(ENGINE_PACKAGE_PREFIX):]
        if leaf not in ENGINE_DEFAULTS:
            return None
        rgba, color = ENGINE_DEFAULTS[leaf]
        payload = build_dds_a8r8g8b8(rgba)
        digest = hashlib.sha256(payload).hexdigest()
        relative = self.engine_dir + digest[:12] + '_' + leaf + '.dds'
        target = self.new_resources / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(payload)
        return dict(sourceObject=key, assetId=relative, sha256=digest, colorSpace=color,
                    mipCount=max(1, int.from_bytes(payload[28:32], 'little')), mipEvidence='source-chain')

    def texture(self, source_object):
        key = source_object.casefold()
        if key in self.cache:
            if isinstance(self.cache[key], Exception):
                raise self.cache[key]
            return self.cache[key]
        try:
            item = self.engine_default(source_object) or self._installed_texture(source_object)
        except ValueError as error:
            self.cache[key] = error
            raise
        self.cache[key] = item
        return item

    def _installed_texture(self, source_object):
        key = source_object.casefold()
        entry = self.catalog.get(key)
        if entry is None and '.tex.' in key:
            entry = self.catalog.get('tex.' + key.split('.tex.', 1)[1])
        if entry is None:
            raise ValueError('texture not in the recooked catalog: ' + source_object)
        relative = self.installed.get(entry['sha256'])
        if relative is None:
            raise ValueError('installed texture is not a DDS in the runtime manifest: ' + source_object)
        full = self.resources / relative
        if not full.is_file() or sha256_file(full) != entry['sha256']:
            raise ValueError('installed texture missing or hash differs: ' + relative)
        data = full.read_bytes()
        receipt = self.receipts.get(entry['objectPath'].casefold())
        color = 'srgb'
        if receipt is not None:
            flag = receipt.get('properties', {}).get('srgb', {}).get('value', True)
            color = 'srgb' if flag in (True, 'True', 'true') else 'linear'
        chain = entry['objectPath'].casefold() in self.recovered
        return dict(sourceObject=key, assetId=relative, sha256=entry['sha256'], colorSpace=color,
                    mipCount=max(1, int.from_bytes(data[28:32], 'little')),
                    mipEvidence='source-chain' if chain else 'project-generated')

    def build_slot(self, asset_id, runtime_name, object_path):
        resolved = self.materials.get(object_path.casefold())
        if resolved is None:
            raise ValueError('no extracted parameters')
        terminal = resolved['terminal'].casefold()
        builder = source_map_surface_extra if terminal.endswith(EXTRA_BUILDER_TERMINALS) else source_map_surface
        requested = []

        def texture(parameter):
            source = resolved['textures'].get(parameter)
            if source is None:
                raise ValueError('active texture %s is NULL or unresolved' % parameter)
            item = self.texture(source)
            requested.append(item['sourceObject'])
            if parameter in NORMAL_PARAMETERS and item['colorSpace'] != 'linear':
                raise ValueError('normal lane %s is not linear in the source: %s' % (parameter, source))
            return item['assetId'], item['colorSpace']

        if not all(type(v) is bool for v in resolved['switches'].values()):
            raise ValueError('static switches are not booleans')
        if resolved.get('unresolvedSwitches'):
            raise ValueError('unresolved static switches')
        if any('switch' in str(x).casefold() for x in resolved.get('unresolvedDefaults', [])):
            raise ValueError('unresolved switch defaults')
        if any(v for k, v in resolved['switches'].items() if 'dynamicfoliage' in k):
            raise ValueError('foliage vertex wind needs a verified binding')
        tracked = compiler.TrackedValues(resolved['values'])
        row = builder.build_surface(object_path.casefold(), terminal, tracked, resolved['switches'], texture,
                                    asset_id=asset_id, material_name=runtime_name)
        row.update(castsShadow=True, renderMode='deferred', cullMode=self.cull.get(asset_id, 'back'))
        compiler.validate_surface(row, {})
        return row, requested, terminal.split('.')[-1]


def derive_lighting_evidence(inventory_path, component_lighting_paths):
    """Per-asset lightingEvidence measured from component lighting receipts.

    The compiler takes lightingEvidence as a statement about the original component, so it is
    derived here instead of assumed. Components carry sourceMesh.objectPath and the asset
    inventory carries fullPath, which is the join; one mesh can back several _OVR_ assets.
    """
    inventory = read_json(inventory_path)
    full_path_of = {}
    for asset in inventory.get('assets', []):
        full_path = asset.get('fullPath')
        if full_path:
            full_path_of[asset['assetId']] = str(full_path).casefold()
    statuses = collections.defaultdict(collections.Counter)
    packages = []
    for path in component_lighting_paths:
        document = read_json(path)
        packages.append(str(document.get('logicalPackage') or path.name))
        for component in document.get('components', {}).values():
            mesh = (component.get('sourceMesh') or {}).get('objectPath')
            if mesh:
                statuses[str(mesh).casefold()][str(component.get('status'))] += 1
    evidence, unresolved = {}, {}
    for asset_id, full_path in full_path_of.items():
        observed = statuses.get(full_path)
        if not observed:
            unresolved[asset_id] = 'no component lighting record for ' + full_path
        elif observed.get('RNM_TEXTURE_LIGHTMAP'):
            evidence[asset_id] = 'source-bound'
        elif set(observed) == {'NO_LOD_LIGHTING_DATA'}:
            evidence[asset_id] = 'source-absent'
        else:
            unresolved[asset_id] = 'component lighting is undecided: ' + '/'.join(sorted(observed))
    summary = dict(
        componentLightingPackages=sorted(packages),
        inventoryAssets=len(full_path_of), distinctSourceMeshes=len(statuses),
        sourceBound=sum(1 for v in evidence.values() if v == 'source-bound'),
        sourceAbsent=sum(1 for v in evidence.values() if v == 'source-absent'),
        unresolved=len(unresolved))
    return evidence, unresolved, summary


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--area-id', required=True)
    parser.add_argument('--parameters', type=Path, required=True)
    parser.add_argument('--runtime-manifest', type=Path, required=True)
    parser.add_argument('--mip-catalog', type=Path, required=True)
    parser.add_argument('--mip-results', type=Path, required=True)
    parser.add_argument('--mip-chains', type=Path, required=True)
    parser.add_argument('--asset-inventory', type=Path, required=True)
    parser.add_argument('--component-lighting', type=Path, nargs='+', required=True,
                        help='extract_source_map_component_lighting.py *.component-lighting.json for every '
                             'logical package of this Area; lightingEvidence is derived from them')
    parser.add_argument('--imported-catalog', type=Path, required=True)
    parser.add_argument('--resources-root', type=Path, required=True)
    parser.add_argument('--runtime-area-root', default=None,
                        help='Resources-relative folder that holds the cooked variants (default Map/<AreaId>)')
    parser.add_argument('--new-resources-root', type=Path, required=True,
                        help='Where the generated engine-default DDS files are written (then installed under --resources-root)')
    parser.add_argument('--engine-default-dir', default=None,
                        help='Resources-relative folder for engine defaults (default Map/<AreaId>_SOURCE_MATERIALS/EngineDefaults)')
    parser.add_argument('--output-dir', type=Path, required=True)
    args = parser.parse_args(argv)
    args.runtime_area_root = args.runtime_area_root or 'Map/' + args.area_id
    args.engine_default_dir = args.engine_default_dir or 'Map/' + args.area_id + '_SOURCE_MATERIALS/EngineDefaults'
    args.output_dir.mkdir(parents=True, exist_ok=True)

    adapter = Adapter(args)
    lighting_evidence, lighting_unresolved, lighting_summary = derive_lighting_evidence(
        args.asset_inventory, args.component_lighting)
    report, built_rows, slots = [], {}, []
    counts, reasons = collections.Counter(), collections.Counter()
    for asset in adapter.runtime['assets']:
        asset_id = asset['assetId']
        for material in asset['materials']:
            object_path = material.get('objectPath')
            record = dict(assetId=asset_id, materialName=material['runtimeName'], sourceMaterial=object_path)
            if object_path is None:
                record.update(status='skip', reason='null material slot')
                counts['skip:null-slot'] += 1
                report.append(record)
                continue
            try:
                row, requested, terminal = adapter.build_slot(asset_id, material['runtimeName'], object_path)
            except (ValueError, KeyError) as error:
                message = str(error)
                terminal = adapter.materials.get(object_path.casefold(), {}).get('terminal', '?').split('.')[-1]
                record.update(status='skip', reason=message[:200], terminal=terminal)
                counts['skip:' + terminal] += 1
                reasons[terminal + ' | ' + message.split(':', 1)[-1].strip()[:100]] += 1
            else:
                evidence = lighting_evidence.get(asset_id)
                if evidence is None:
                    reason = lighting_unresolved.get(asset_id, 'asset is not in the asset inventory')
                    record.update(status='skip', reason='lighting evidence unresolved: ' + reason, terminal=terminal)
                    counts['skip:lighting-evidence'] += 1
                    reasons['lighting-evidence | ' + reason[:100]] += 1
                    report.append(record)
                    continue
                built_rows[(asset_id, material['runtimeName'])] = row
                record.update(status='ok', family=row['family'], textures=requested, terminal=terminal,
                              lightingEvidence=evidence)
                counts['ok'] += 1
                slots.append(dict(assetId=asset_id, materialName=material['runtimeName'], sourceMaterial=object_path,
                                  component=dict(rendering=dict(castsShadow=True, renderMode='deferred',
                                                                 cullMode=row['cullMode']),
                                                 lightingEvidence=evidence)))
            report.append(record)

    # Whole assets only: the scene admission raises on a partial or null slot list.
    ok_by_asset = collections.defaultdict(list)
    for record in report:
        ok_by_asset[record['assetId']].append(record['status'] == 'ok')
    complete = {a for a, v in ok_by_asset.items() if v and all(v)}
    partial = {a for a, v in ok_by_asset.items() if any(v) and not all(v)}
    geometry = {}
    for asset_id in sorted(complete):
        rows = [row for (a, _), row in built_rows.items() if a == asset_id]
        try:
            scene.validate_source_material_geometry(args.resources_root / adapter.prefix / adapter.model_of[asset_id], rows)
        except ValueError as error:
            geometry[asset_id] = str(error).split(';')[-1].strip()[:80]
    complete -= set(geometry)
    for record in report:
        if record['status'] == 'ok' and record['assetId'] in partial:
            record.update(status='skip', reason='another slot of the same asset is not compiled')
        elif record['status'] == 'ok' and record['assetId'] in geometry:
            record.update(status='skip', reason='geometry channel missing in the cooked WModel: ' + geometry[record['assetId']])
    slots = [s for s in slots if s['assetId'] in complete]
    textures = {}
    for record in report:
        if record['status'] == 'ok':
            for source in record['textures']:
                textures[source] = dict(adapter.cache[source])
    approximations = [
        dict(note='lightingEvidence is measured per asset from the component lighting receipts: source-bound when any component reports an RNM lightmap, source-absent only when every component reports no lighting record. Assets that resolve to neither are dropped, not assumed.'),
        dict(note='source-bound here states what the original carries, not what this build binds. This adapter still emits no bakedLighting pair and no placementLighting instances, so the RNM lightmaps remain unbound; binding them needs auxiliaryTextures and a texture pair on every row.'),
        dict(note='castsShadow=true is the project default: the placement extractor does not record the source component CastShadow.'),
        dict(note='mipEvidence=project-generated marks textures whose source mip chain could not be recovered (mip0 only).'),
        dict(note='Engine default textures (efmaster_material_prologue.tex.X) come from the source package EFMaster_Material_Prologue; the generated DDS carries the same uniform 2x2 texels, SRGB flag and mip chain.'),
    ]
    manifest = dict(
        format='lostark-source-map-material-input', formatVersion=1, areaId=args.area_id,
        parameters=dict(path=str(args.parameters), sha256=sha256_file(args.parameters)),
        evidence=[dict(path=str(p), sha256=sha256_file(p)) for p in (args.asset_inventory, args.runtime_manifest, args.mip_catalog)],
        textures=[textures[k] for k in sorted(textures)], slots=slots, placementLighting=[],
        lightingEvidenceSummary=lighting_summary,
        projectApproximations=approximations)
    (args.output_dir / 'inputs.json').write_text(json.dumps(manifest, ensure_ascii=False, indent=1), encoding='utf-8')
    (args.output_dir / 'slot_report.json').write_text(json.dumps(report, ensure_ascii=False, indent=1), encoding='utf-8')
    print('slots ok before whole-asset filter:', counts['ok'])
    for key, value in sorted(counts.items(), key=lambda kv: -kv[1]):
        if key != 'ok':
            print('  %5d  %s' % (value, key))
    for key, value in reasons.most_common(15):
        print('  %4d  %s' % (value, key))
    print('assets bound: %d of %d (partial %d left on the legacy path, geometry-dropped %d); slots %d; textures %d'
          % (len(complete), len(adapter.runtime['assets']), len(partial), len(geometry), len(slots), len(textures)))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())

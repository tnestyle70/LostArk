"""Read-only source-to-installed Maharaka inventory; never auto-admit missing rows.

Start at the PS WorldInfo streaming references, not filename/name similarity.
Static presence, transforms, non-static exports and product visibility are separate.
This writes audit artifacts only, not authoring, Resources, or published data.
"""
from __future__ import annotations
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re

import extract_ue3_placements as ue
import build_maptool_scene as scene

ROOT = Path(__file__).resolve().parents[2]
AREA = 'LV_OCN_EVENTIS_MHP'


def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def rows(path):
    return [[a or b for a, b in re.findall(r'"([^"]*)"|(\S+)', line)]
            for line in path.read_text(encoding='utf-8-sig').splitlines()[1:] if line.strip()]


def decode(document):
    path = Path(document['source']['physicalPackage'])
    raw = path.read_bytes()
    digest = hashlib.sha256(raw).hexdigest()
    if digest != document['source']['sha256']:
        raise ValueError(f'Source changed since placement extraction: {path}')
    summary = ue.parse_summary(raw)
    data = ue.decompress_package(raw, summary, ue.LOSTARK_KR_AES_KEY)
    names = ue.parse_name_table(data, summary)
    imports = ue.parse_import_table(data, summary, names)
    exports = ue.parse_export_table(data, summary, names)
    return path, digest, summary, data, names, imports, exports


def referenced_streaming_levels(objects):
    """Only WorldInfo-owned streaming references define the closure."""
    owners = [o for o in objects if o['className'] == 'worldinfo']
    if len(owners) != 1:
        raise ValueError('Expected exactly one persistent WorldInfo')
    refs = owners[0].get('properties', {}).get('streaminglevels', {}).get('value')
    if not isinstance(refs, list) or any(type(i) is not int or i <= 0 for i in refs):
        raise ValueError('WorldInfo streaming reference array was not decoded')
    if len(set(refs)) != len(refs):
        raise ValueError('Duplicate WorldInfo streaming reference')
    by_index = {o['packageIndex']: o for o in objects}
    result = []
    for index in refs:
        obj = by_index.get(index)
        if obj is None or obj['className'] not in ('eflevelstreamingalwaysloaded', 'levelstreamingalwaysloaded', 'levelstreamingkismet'):
            raise ValueError(f'Invalid streaming reference {index}')
        name = obj.get('properties', {}).get('packagename', {}).get('value')
        if not isinstance(name, str) or not name:
            raise ValueError(f'Unresolved streaming package {index}')
        result.append(dict(packageIndex=index, className=obj['className'], logicalPackage=name.upper()))
    return result


def attachment(properties):
    """Preserve actual references; a similar mesh name is not an attachment."""
    return {key: properties[key]['value'] for key in
            ('base', 'baseskelcomponent', 'basebonename', 'relativelocation', 'relativerotation')
            if key in properties}


def compare_material_overrides(overrides, installed_materials):
    result = []
    for slot in (overrides or {}).get('slots', []):
        source = slot.get('objectPath')
        prefix = f"SLOT_{slot['slot']:03d}_"
        found = [m for m in installed_materials if m.get('materialName', '').startswith(prefix)]
        if slot.get('packageIndex') == 0:
            status = 'NULL_OVERRIDE_REQUIRES_PARENT_MATERIAL_RESOLUTION'
        elif len(found) != 1:
            status = 'SLOT_IDENTITY_UNRESOLVED'
        elif str(found[0].get('sourceMaterial', '')).casefold() != str(source).casefold():
            status = 'SOURCE_MATERIAL_DIFFERENCE'
        else:
            status = 'SOURCE_MATERIAL_REFERENCE_MATCH'
        result.append(dict(slot=slot['slot'], sourceMaterial=source, status=status,
                           installedSourceMaterials=[m.get('sourceMaterial') for m in found]))
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-root', type=Path, required=True)
    parser.add_argument('--sublevels', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    inputs = {}
    for directory in (args.source_root, args.sublevels):
        for path in sorted(directory.rglob('*.placements.json')):
            doc = read(path)
            name = doc['source']['logicalPackage'].upper()
            if name in inputs:
                raise ValueError(f'Duplicate source document: {name}')
            inputs[name] = (path, doc)
    if AREA+'_PS' not in inputs:
        raise ValueError('Persistent level source is missing')
    installed_path = ROOT/f'Data/Maps/Authoring/{AREA}/{AREA}.mapplacements'
    installed = rows(installed_path)
    by_source = {}
    for row in installed:
        by_source.setdefault(row[1], []).append(row)
    catalog = {r[0]: r for r in rows(ROOT/f'Data/Maps/Imported/{AREA}/{AREA}.mapassets')}
    materials = read(ROOT/f'Data/Maps/Authoring/{AREA}/{AREA}.mapmaterials.json')['materials']
    materials_by_asset = {}
    for material in materials:
        materials_by_asset.setdefault(material['assetId'], []).append(material)
    summary_rows = []
    all_source_ids = set()
    streaming = []
    graph_errors = []
    for logical, (path, doc) in sorted(inputs.items()):
        physical, digest, summary, data, names, imports, exports = decode(doc)
        classes = Counter()
        objects = []
        for entry in exports:
            cls = ue.package_ref_name(entry.class_index, imports, exports).lower()
            classes[cls] += 1
            # Include all actors/components/tracks and script references, not only meshes.
            if cls in ('texture2d', 'lightmaptexture2d', 'shadowmap2d', 'model', 'polys'):
                continue
            item = dict(packageIndex=entry.index+1, objectName=entry.object_name, className=cls)
            if entry.serial_size:
                try:
                    props, end = ue.parse_tagged_properties(data[entry.serial_offset:entry.serial_offset+entry.serial_size], names, summary.version)
                    item['properties'] = props
                    item['unparsedNativeBytes'] = entry.serial_size-end
                except Exception as error:
                    item['propertyError'] = str(error)
                    graph_errors.append(dict(level=logical, packageIndex=entry.index+1, className=cls, error=str(error)))
            objects.append(item)
        if logical == AREA+'_PS':
            streaming = referenced_streaming_levels(objects)
        objects_by_index = {o['packageIndex']: o for o in objects}
        (args.output/(logical+'.objects.json')).write_text(json.dumps(dict(source=doc['source'],
            imports=[dict(packageIndex=-(i+1), objectName=e.object_name,
                          resolvedName=ue.package_ref_path(-(i+1), imports, exports),
                          className=e.class_name) for i,e in enumerate(imports)],
            exports=objects), ensure_ascii=False, indent=2), encoding='utf-8')
        comparison = []
        for p in doc['placements']:
            identity = p['placementId']
            all_source_ids.add(identity)
            actual = by_source.get(identity, [])
            t = p['transform']
            position = scene.convert_position(t['position'])
            rotation = scene.convert_rotation(t['rotation'])
            scale = scene.convert_scale(t['scale3D'])
            status = 'MISSING'
            diffs = []
            for row in actual:
                got = list(map(float, row[5:15]))
                # q and -q encode the same orientation.
                qerror = min(max(abs(got[3+i]-sign*rotation[i]) for i in range(4)) for sign in (-1, 1))
                diffs.append(dict(placementId=row[0], assetId=row[4], enabled=row[15]=='1',
                    positionErrorMeters=max(abs(got[i]-position[i]) for i in range(3)),
                    quaternionError=qerror, scaleError=max(abs(got[7+i]-scale[i]) for i in range(3)),
                    catalogSourceDescription=catalog[row[4]][10],
                    materialOverrides=compare_material_overrides(p.get('materialOverrides'), materials_by_asset.get(row[4], []))))
            if actual:
                status = 'PRESENT_TRANSFORM_MATCH' if all(max(d['positionErrorMeters'], d['quaternionError'], d['scaleError'])<1e-4 for d in diffs) else 'PRESENT_TRANSFORM_DIFFERENCE'
            actor = objects_by_index[p['actor']['exportIndex']+1]
            comparison.append(dict(sourcePlacementId=identity, actorClass=p['actor']['class'],
                attachment=attachment(actor.get('properties', {})), actorPropertyError=actor.get('propertyError'),
                sourceMesh=p['asset']['objectPath'], position=position, quaternion=rotation, scale=scale,
                sourceVisible=p.get('sourceVisibility',{}).get('visible'),
                materialOverrides=p.get('materialOverrides'), status=status, installed=diffs))
        (args.output/(logical+'.static-comparison.json')).write_text(json.dumps(comparison,ensure_ascii=False,indent=2),encoding='utf-8')
        counts = dict(Counter(r['status'] for r in comparison))
        record=dict(level=logical, physicalPackage=str(physical), sourceSha256=digest,
                    staticSourceCount=len(comparison), staticStatusCounts=counts, exportClassCounts=dict(classes))
        summary_rows.append(record)
        print(logical, counts, flush=True)
    missing_levels = [s for s in streaming if s['logicalPackage'] not in inputs]
    unexpected_levels = sorted(set(inputs)-{AREA+'_PS'}-{s['logicalPackage'] for s in streaming})
    report = dict(schema='lostark.maharaka-source-coverage-audit', formatVersion=2,
        scope='PS-referenced streaming levels plus PS; no visual or material fidelity admission',
        streamingLevels=streaming, missingSourceLevels=missing_levels, unexpectedInputLevels=unexpected_levels, propertyErrors=graph_errors,
        installedPlacementCount=len(installed), sourceStaticPlacementCount=len(all_source_ids),
        installedWithoutStaticSource=[dict(sourcePlacementId=r[1],assetId=r[4]) for r in installed if r[1] not in all_source_ids],
        levels=summary_rows, automaticAdmission=False, runtimeChanged=False,
        limits=['Native payload bytes require class-specific decoding, not absence inference',
                'Static presence/transform match does not prove exact materials, lighting, motion or visibility',
                'Missing runtime rows may be scope-filtered or conditional; inspect consumers before installing'])
    (args.output/'coverage.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
    if missing_levels:
        raise ValueError(f'{len(missing_levels)} PS-referenced source levels missing; see coverage.json')
    if unexpected_levels:
        raise ValueError(f'Inputs outside WorldInfo closure: {unexpected_levels}')


if __name__ == '__main__':
    main()

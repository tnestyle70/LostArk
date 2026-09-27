"""Restore missing explicit MIC bindings using an identical admitted MIC surface.

Never infer identity from leaf names, copy placement lighting, replace existing
overrides, or promote water/wind/native programs through an opaque carrier.
WorldInfo audit must have been generated from the current source packages first.
The official publisher remains the only runtime writer.
"""
from __future__ import annotations
import argparse
from collections import defaultdict
import copy
import hashlib
import json
import os
from pathlib import Path
import re
import sys
import tempfile

import audit_maharaka_source_coverage as audit

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT/'Tools/BernCastlePipeline'))
from audit_bern_runtime_materials import inspect_model


def digest(value):
    return hashlib.sha256(value).hexdigest()


def surface_only(row):
    return {k: copy.deepcopy(v) for k, v in row.items()
            if k not in ('assetId', 'materialName', 'bakedLighting')}


def resolve_surface(source, donors):
    matches = [r for r in donors if r.get('sourceMaterial', '').casefold() == source.casefold()]
    if not matches:
        return None, 'NO_EXISTING_EXACT_SOURCE_MATERIAL'
    values = [surface_only(r) for r in matches]
    if any(v != values[0] for v in values):
        return None, 'CONFLICTING_SURFACE_VALUES_FOR_SAME_SOURCE'
    value = values[0]
    if (value.get('family') != 'bg-source-opaque-masked' or
            value.get('renderMode') != 'deferred' or value.get('sourceFoliageWind')):
        return None, 'REQUIRES_SEPARATE_RENDER_OR_VERTEX_CONTRACT'
    return value, None


def resource(root, relative):
    if not isinstance(relative, str) or ':' in relative or '\\' in relative or '..' in relative.split('/'):
        raise ValueError(f'Invalid Resources identity: {relative}')
    path = (root/relative).resolve()
    if not path.is_relative_to(root.resolve()) or not path.is_file():
        raise ValueError(f'Missing Resources input: {relative}')
    return path


def append_preserving_text(before, added):
    """Insert at the top-level materials array without reformatting user data."""
    if not added:
        return before
    text = before.decode('utf-8-sig')
    matches = list(re.finditer(r'(?m)^([ \t]*)"materials"\s*:\s*\[', text))
    if len(matches) != 1:
        raise ValueError('Expected one materials array anchor')
    match = matches[0]
    start = match.end()-1
    existing, end = json.JSONDecoder().raw_decode(text, start)
    if not isinstance(existing, list) or text[end-1] != ']':
        raise ValueError('Invalid materials array')
    insertion = end-1
    while insertion > start+1 and text[insertion-1].isspace():
        insertion -= 1
    newline = '\r\n' if '\r\n' in text else '\n'
    unit = match.group(1) or '  '
    rendered = []
    for row in added:
        lines = json.dumps(row, ensure_ascii=False, indent=unit, allow_nan=False).splitlines()
        rendered.append(newline.join(unit+unit+line for line in lines))
    patch = (',' if existing else '')+newline+(','+newline).join(rendered)
    result = (text[:insertion]+patch+text[insertion:]).encode('utf-8')
    return (b'\xef\xbb\xbf' if before.startswith(b'\xef\xbb\xbf') else b'')+result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--audit', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--apply', action='store_true')
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    coverage = audit.read(args.audit/'coverage.json')
    if coverage['formatVersion'] != 2 or coverage['missingSourceLevels'] or coverage['unexpectedInputLevels']:
        raise ValueError('Complete WorldInfo reference audit is required')
    for level in coverage['levels']:
        if digest(Path(level['physicalPackage']).read_bytes()) != level['sourceSha256']:
            raise ValueError('Source package changed: '+level['level'])
    area = audit.AREA
    path = ROOT/f'Data/Maps/Authoring/{area}/{area}.mapmaterials.json'
    before = path.read_bytes()
    document = json.loads(before.decode('utf-8-sig'))
    catalog_path = ROOT/f'Data/Maps/Imported/{area}/{area}.mapassets'
    placement_path = ROOT/f'Data/Maps/Authoring/{area}/{area}.mapplacements'
    dependencies = {catalog_path: catalog_path.read_bytes(), placement_path: placement_path.read_bytes()}
    catalog = {r[0]: r for r in audit.rows(catalog_path)}
    live = {r[1]: r for r in audit.rows(placement_path)}
    originals = document['materials']
    wanted = defaultdict(set)
    occurrences = defaultdict(set)
    for comparison in sorted(args.audit.glob('*.static-comparison.json')):
        for p in audit.read(comparison):
            for installed in p['installed']:
                current = live.get(p['sourcePlacementId'])
                if current is None or current[4] != installed['assetId']:
                    raise ValueError('Placement identity changed since audit')
                for slot in installed['materialOverrides']:
                    if slot['status'] != 'SLOT_IDENTITY_UNRESOLVED' or not slot['sourceMaterial']:
                        continue
                    key = installed['assetId'], slot['slot']
                    wanted[key].add(slot['sourceMaterial'].casefold())
                    occurrences[key].add(p['sourcePlacementId'])
    added, evidence, deferred = [], [], []
    model_slots = {}
    resources = ROOT/'Client/Bin/Resources'
    for key, source_set in sorted(wanted.items()):
        asset, slot = key
        if len(source_set) != 1:
            raise ValueError(f'One asset/slot has conflicting source identities: {key}')
        source, = source_set
        surface, reason = resolve_surface(source, originals)
        if reason:
            deferred.append(dict(assetId=asset, slot=slot, sourceMaterial=source, reason=reason))
            continue
        if asset not in model_slots:
            model = resource(resources, catalog[asset][2])
            dependencies[model] = model.read_bytes()
            model_slots[asset] = inspect_model(model)[2]
        slots = model_slots[asset]
        if slot >= len(slots) or not slots[slot].startswith(f'SLOT_{slot:03d}_'):
            raise ValueError(f'Cooked slot index/name mismatch: {key}')
        name = slots[slot]
        if any(r['assetId'] == asset and r['materialName'] == name for r in originals):
            # Re-running after admission is intentionally a no-op.
            continue
        for k, v in surface.items():
            if k.endswith('Texture') and isinstance(v, str) and v:
                texture = resource(resources, v)
                dependencies[texture] = texture.read_bytes()
        added.append(dict(assetId=asset, materialName=name, **surface))
        evidence.append(dict(assetId=asset, materialName=name, sourceMaterial=source,
                             sourcePlacementIds=sorted(occurrences[key]),
                             donorAssetIds=sorted({m['assetId'] for m in originals if m.get('sourceMaterial', '').casefold() == source}),
                             copiedPlacementLighting=False))
    candidate = copy.deepcopy(document)
    candidate['materials'].extend(added)
    after = append_preserving_text(before, added)
    if json.loads(after.decode('utf-8-sig')) != candidate:
        raise ValueError('Candidate text does not match the staged transaction')
    report = dict(schema='lostark.maharaka-missing-material-restoration', formatVersion=1,
                  beforeSha256=digest(before), candidateSha256=digest(after),
                  addedSlots=len(added), affectedAssets=len({r['assetId'] for r in added}),
                  affectedPlacements=len({p for r in evidence for p in r['sourcePlacementIds']}),
                  bindings=evidence, unresolved=deferred, runtimePublished=False,
                  placementLightingUnchanged=candidate['placementLighting']==document['placementLighting'],
                  caveat='Exact existing MIC binding reuse, not independent full native shader/geometry fidelity proof')
    (args.out/'before.mapmaterials.json').write_bytes(before)
    (args.out/'candidate.mapmaterials.json').write_bytes(after)
    (args.out/'report.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    if args.apply and added:
        if path.read_bytes() != before or any(p.read_bytes()!=b for p,b in dependencies.items()):
            raise ValueError('Inputs changed; refusing to overwrite current edits')
        fd, temp = tempfile.mkstemp(prefix=path.name+'.', suffix='.tmp', dir=path.parent)
        try:
            with os.fdopen(fd, 'wb') as stream:
                stream.write(after)
            os.replace(temp, path)
        finally:
            if os.path.exists(temp):
                os.unlink(temp)
    print(json.dumps({k:report[k] for k in ('addedSlots','affectedAssets','affectedPlacements','placementLightingUnchanged')}))


if __name__ == '__main__':
    main()

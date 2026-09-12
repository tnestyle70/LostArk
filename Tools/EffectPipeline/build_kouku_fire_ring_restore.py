"""Expose the original fire-ring projectile and impact as independent V1 groups."""
import argparse
import copy
import hashlib
import math
import struct
from pathlib import Path

import build_kouku_gate1_full_restore as source
from extract_action_effect_notifies import scan_length_prefixed_strings

ROOT = source.ROOT
TARGETS = {
    'fx_mn_rpct_05_l.par_l_rpct_05_sk_04_10_loc_int': ('loop', '화염링 / 지속'),
    'fx_mn_rpct_05_l.par_l_rpct_05_sk_04_11_loc_int': ('impact', '화염링 / 폭발'),
}


def project(library_root, evidence, install):
    manifest = source.read(library_root / 'installation.json')
    projectile = library_root / 'source/Projectile/421980401.loa'
    raw = projectile.read_bytes()
    lifetime = struct.unpack_from('<f', raw, 1014)[0]
    assert lifetime == 5
    nodes = scan_length_prefixed_strings(raw, 0, len(raw))
    transforms = {}
    for token in nodes:
        if not token['value'].startswith("ParticleSystem'"):
            continue
        key = token['value'].split("'")[1].lower()
        if key not in TARGETS:
            continue
        at = token['sourceOffset'] + len(token['value']) + 5
        pos = list(struct.unpack_from('<3f', raw, at + 76))
        rot = list(struct.unpack_from('<3i', raw, at + 100))
        scale = list(struct.unpack_from('<3f', raw, at + 136))
        assert struct.unpack_from('<i', raw, at + 148)[0] == 0
        transforms[key] = dict(sourceByteOffset=token['sourceOffset'], position=[pos[0]*.01,pos[2]*.01,-pos[1]*.01],
            rotationDegrees=[rot[0]*360/65536,rot[1]*360/65536,-rot[2]*360/65536], scale=scale)
    assert set(transforms) == set(TARGETS)
    rows = []
    for system, (suffix, title) in TARGETS.items():
        entry = next(d for d in manifest['documents'] if d['sourceParticleSystem'] == system)
        doc = source.read(library_root / 'candidate' / Path(entry['path']).name)
        asset = 'effect.kouku.gate3.fire_ring.' + suffix
        doc.update(effectAssetId=asset, displayName=title)
        identifiers = {e['id']: 'kouku.fire-ring.' + hashlib.sha256((suffix + e['id']).encode()).hexdigest()[:24] for e in doc['elements']}
        for element in doc['elements']:
            element['groupId'] = asset
            element['id'] = identifiers[element['id']]
            if 'particleSystemOccurrenceId' in element['sourceRecipe']:
                element['sourceRecipe']['particleSystemOccurrenceId'] = asset
            for module in element['sourceRecipe']['modules']:
                for literal in module['literals']:
                    if literal['propertyPath'] == 'runtime.providerelementid':
                        literal['value'] = identifiers[literal['value']]
            element['actionCueAttachment']['enabled'] = False
            for field in ('position','rotationDegrees','scale'):
                element['detail']['transform'][field] = transforms[system][field]
            if suffix == 'loop':
                element['detail']['timing']['lifeTimeSeconds'] = lifetime
        path = ROOT / 'Data/Effects/Authored' / (asset + '.effect.json')
        source.write(evidence / 'candidate' / path.name, doc)
        if install:
            assert manifest['installed'], 'Original native library must be installed first'
            assert all(e['material']['sourceProfile'].get('enabled') for e in doc['elements']), 'Original native material is missing'
            assert not path.exists() or source.read(path) == doc, 'Preserve authored edits: ' + str(path)
            source.write(path, doc)
        duration = math.ceil(max(e['detail']['timing']['startDelaySeconds'] + e['detail']['timing']['lifeTimeSeconds'] + max(e['detail']['particle']['lifeTimeSeconds']) for e in doc['elements']) * 1000)
        rows.append(dict(effectAssetId=asset, displayName=title, path=path.relative_to(ROOT).as_posix(), elementCount=len(doc['elements']),
            durationMs=duration, sourceParticleSystem=system, sourceProjectileId=421980401, categoryPath=['KoukuSaydon','3관문','패턴','화염링'],
            sourceTransform=transforms[system], referenceImageIdentity='USER_VISUAL_COMPARISON_PENDING'))
    source.write(evidence / 'installation.json', dict(installed=install, documents=rows,
        sourceEvidence=dict(projectilePath=str(projectile), sha256=hashlib.sha256(raw).hexdigest(), lifetimeSeconds=lifetime,
            sourceActions=[dict(profileId='MN_RPCT_05', actionId=4219869), dict(profileId='MN_RPCT_07', actionId=4219956)],
            decodedSourceStrings=nodes), manualVisualValidation='USER_PENDING'))
    print('Fire-ring groups', len(rows), 'elements', sum(r['elementCount'] for r in rows))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--library-root', type=Path, default=ROOT/'out/KoukuAllEffects20260912')
    parser.add_argument('--evidence-root', type=Path, default=ROOT/'out/KoukuFireRing20260912')
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    project(args.library_root, args.evidence_root, args.install)

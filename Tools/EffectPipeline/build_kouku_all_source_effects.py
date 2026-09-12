"""Restore source ParticleSystems used by Kouku actions and sequence parents.

The output consists of independent effect groups. Original action notification
timing and poses belong to their pattern occurrences, not this shared library.
Only source-qualified, projected documents enter the installed catalog.
"""
import argparse
import collections
import copy
import csv
import hashlib
import json
import math
from pathlib import Path

import build_kouku_gate1_full_restore as source
import build_kouku_showtime_restore as library

ROOT = source.ROOT
INVENTORY = ROOT / '.md/GB/09-11/2026-09-11_KOUKU_SOURCE_PATTERN_EFFECT_INVENTORY_PARTICLES.csv'


def source_targets(extra=None):
    rows = list(csv.DictReader(INVENTORY.open(encoding='utf-8-sig')))
    selected = {r['particleSystem'].lower(): r for r in rows if int(r['directSourceActionUseCount'])}
    if extra:
        for system in source.read(extra):
            key = system.lower()
            selected[key] = next((r for r in rows if r['particleSystem'].lower() == key),
                dict(particleSystem=key, actionNames='', emitterCount=None, sourceActions=''))
    targets = {}
    for key, row in sorted(selected.items()):
        identity = int(hashlib.sha256(key.encode()).hexdigest()[:7], 16)
        assert identity not in targets
        targets[identity] = dict(asset='effect.kouku.source.' + key, name=(row['actionNames'].split(' ; ')[0] + '_' + key.rsplit('.', 1)[-1]).strip('_'),
            system=key, count=int(row['emitterCount']) if row['emitterCount'] is not None else None,
            countIncludesEmpty=True, sourceActions=[s for s in row['sourceActions'].split(' ; ') if s])
    return targets


def acquire(evidence, targets):
    # Keep one source system per stable library entry; the shared extractor
    # resolves real first LOD modules, archetypes and class default subobjects.
    library.TARGETS = targets
    from extract_ue3_placements import resolve_physical_package
    import extract_ue3_placements
    original_resolve = resolve_physical_package
    def package_path(umodel, release, logical, region):
        if logical in ('engine', 'efgame'):
            return source.source_package('Shared', logical)
        return original_resolve(umodel, release, logical, region)
    extract_ue3_placements.resolve_physical_package = package_path
    original_record = source.record_from_export
    def record(package, logical, entry):
        try:
            return original_record(package, logical, entry)
        except Exception as error:
            raise RuntimeError(f'Source properties: {logical}.{entry.object_name}: {error}') from error
    source.record_from_export = record
    result = library.acquire(evidence)
    source.write(evidence / 'library_targets.json', targets)
    summary = dict(systems=len(targets), emitters=len(result[2]),
        materials=len({o['sourceMaterial'] for o in result[2]}),
        shapes=dict(collections.Counter(o['rendererShape'] for o in result[2])),
        modules=dict(collections.Counter(result[0].objects[k].class_name for o in result[2] for k in o['moduleOrder'])))
    source.write(evidence / 'source_summary.json', summary)
    print(json.dumps(summary, ensure_ascii=False))
    return result


def project(evidence, targets, material_patch, install, cached_source=None):
    from build_kouku_action_effect_groups import restored_index, project_light_occurrences
    if cached_source:
        index = restored_index(cached_source)
        notifies = source.read(cached_source / 'source_notifies.json')
        occurrences = source.read(cached_source / 'source_occurrences.json')
        records = {}
        original_decode = source.decode_typed_payload
        def decode_library(kind, payload, *args):
            if payload.get('sourceKind') == 'ORIGINAL_PARTICLE_SYSTEM_LIBRARY_PREVIEW':
                return copy.deepcopy(payload)
            return original_decode(kind, payload, *args)
        source.decode_typed_payload = decode_library
    else:
        index, notifies, occurrences, records = acquire(evidence, targets)
    material_patch = library.patch_simulation_providers(evidence, material_patch, index, occurrences)
    destination = evidence / 'projected'
    successes, failures = [], []
    for identity, target in targets.items():
        own = [o for o in occurrences if o['actionId'] == identity]
        if not own:
            failures.append(dict(sourceParticleSystem=target['system'], reason='SOURCE_HAS_NO_FIRST_LOD_EMITTERS'))
            continue
        source.SELECTED = {identity: ([0], target['name'])}
        try:
            if any(o['rendererShape'] == 'animationTrail' for o in own):
                raise ValueError('AnimationTrail requires the owning source notify baked-edge history')
            own_notifies = [n for n in notifies if n['actionId'] == identity]
            light_occurrences = [o for o in own if o['rendererShape'] == 'light']
            particle_occurrences = [o for o in own if o['rendererShape'] != 'light']
            document = None
            if particle_occurrences:
                source.project(evidence, index, own_notifies, particle_occurrences, records, destination, material_patch)
                document = source.read(destination / f'effect.kouku.gate1.{identity}.full.restore.effect.json')
            if light_occurrences:
                assert len(own_notifies) == 1, 'Neutral library owns one original PS per entry'
                notify = own_notifies[0]
                light = project_light_occurrences(index, light_occurrences, notify['cue'], notify, evidence)
                if document is None:
                    document = light
                else:
                    document['elements'] += light['elements']
            assert document is not None
            document.update(effectAssetId=target['asset'], displayName=target['name'])
            # Typed light particles have separate birth IDs but keep the exact
            # original emitter identity for provider/name lookup.
            expanded = []
            for element in document['elements']:
                occurrence = next(o for o in own if o['sourceEmitter'] == element['sourcePresentation']['sourceObjectPath'])
                expanded.append(dict(occurrence, elementId=element['id']))
            library.bind_source_providers(document, index, expanded)
            for element in document['elements']:
                element['groupId'] = target['asset']
                element['actionCueAttachment']['enabled'] = False
                element['actionCueAttachment'].pop('snapshotRootSourceBasisYawDegrees', None)
                element['detail']['transform'].update(position=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1])
            duration = math.ceil(max(e['detail']['timing']['startDelaySeconds'] + e['detail']['timing']['lifeTimeSeconds']
                + (0 if e['kind'] == 'light' else max(e['detail']['particle']['lifeTimeSeconds'])) for e in document['elements']) * 1000)
            path = ROOT / 'Data/Effects/Authored' / (target['asset'] + '.effect.json')
            source.write(evidence / 'candidate' / path.name, document)
            if install:
                assert material_patch, 'Native source materials are required for installation'
                assert not path.exists() or source.read(path) == document, f'Preserve authored edits: {path}'
                source.write(path, document)
            successes.append(dict(effectAssetId=target['asset'], displayName=target['name'], path=path.relative_to(ROOT).as_posix(),
                sourceParticleSystem=target['system'], sourceActions=target['sourceActions'], elementCount=len(document['elements']), durationMs=duration))
        except Exception as error:
            failures.append(dict(sourceParticleSystem=target['system'], errorType=type(error).__name__, reason=str(error)))
    source.write(evidence / 'installation.json', dict(installed=install, documents=successes, sourceFailures=failures,
        manualVisualValidation='USER_PENDING'))
    print(json.dumps(dict(projected=len(successes), sourceFailures=len(failures), installed=install)))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/KoukuAllEffects20260912')
    parser.add_argument('--extra-source-systems', type=Path)
    parser.add_argument('--acquire-only', action='store_true')
    parser.add_argument('--native-material-patch', type=Path)
    parser.add_argument('--install', action='store_true')
    parser.add_argument('--cached-source-root', type=Path,
        help='Reuse an existing source closure while writing projection receipts to --evidence-root')
    args = parser.parse_args()
    targets = ({int(k): v for k,v in source.read(args.cached_source_root / 'library_targets.json').items()}
               if args.cached_source_root else source_targets(args.extra_source_systems))
    if args.acquire_only:
        acquire(args.evidence_root, targets)
    else:
        project(args.evidence_root, targets, args.native_material_patch, args.install, args.cached_source_root)

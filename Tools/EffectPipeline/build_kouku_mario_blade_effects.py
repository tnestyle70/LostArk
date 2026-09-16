"""Reproject the two original Mario blade systems to out-only V1 candidates.

The current installed library is read-only. Native descriptors are reused per
exact source emitter; fresh source modules/CDO defaults own particle behavior.
No catalog, Composition, Resources or gameplay document is installed here.
"""
import argparse
import copy
import hashlib
from pathlib import Path

import build_kouku_all_source_effects as library
import build_kouku_gate1_full_restore as source

ROWS = (
    ('par_v_rpct_cutting_pjt_01', 'normal', '일반칼날이펙트', 9),
    ('par_v_rpct_cutting_pjt_02_loc_int', 'instant-death', '즉사칼날이펙트', 10),
)


def build(evidence):
    targets, originals, programs, inputs = {}, {}, [], []
    for leaf, slug, title, count in ROWS:
        system = 'fx_mn_rpct_07_v.' + leaf
        identity = int(hashlib.sha256(system.encode()).hexdigest()[:7], 16)
        original_path = source.ROOT / 'Data/Effects/Authored' / ('effect.kouku.source.' + system + '.effect.json')
        original = source.read(original_path)
        assert len(original['elements']) == count
        originals[identity] = original
        inputs.append(dict(path=original_path.relative_to(source.ROOT).as_posix(),
                           sha256=hashlib.sha256(original_path.read_bytes()).hexdigest()))
        targets[identity] = dict(asset='effect.kouku.mario.blade.' + slug + '.full.restore',
            name=title, system=system, count=count, countIncludesEmpty=False, sourceActions=[])
        for element in original['elements']:
            assert element['material']['sourceProfile']['enabled']
            programs.append(dict(sourceMaterial=element['material']['sourceMaterialPath'],
                rendererShape=element['sourceRecipe']['rendererShape'],
                occurrences=[element['id']], material=copy.deepcopy(element['material'])))
    patch = evidence / 'installed_native_material_patch.json'
    source.write(patch, dict(programs=programs))
    library.acquire(evidence / 'source', targets)
    library.project(evidence, targets, patch, False, evidence / 'source')
    installation = source.read(evidence / 'installation.json')
    assert not installation['sourceFailures'], installation['sourceFailures']
    reports = []
    for identity, original in originals.items():
        target = targets[identity]
        path = evidence / 'candidate' / (target['asset'] + '.effect.json')
        document = source.read(path)
        assert len(document['elements']) == len(original['elements'])
        old_by_id = {element['id']: element for element in original['elements']}
        changes, body_ids = [], []
        for element in document['elements']:
            old = old_by_id[element['id']]
            assert element['material'] == old['material'], 'Keep the exact native material descriptor'
            assert element['resources'] == old['resources'], 'Keep the exact installed mesh'
            assert element['detail']['transform'] == old['detail']['transform']
            assert element['detail']['particle']['localSpace'] == old['detail']['particle']['localSpace']
            assert not element['actionCueAttachment']['enabled']
            if any(r['assetId'].endswith('/fm_o_cngn_01.wmodel') for r in element['resources']):
                body_ids.append(element['id'])
            required = next(m for m in element['sourceRecipe']['modules'] if m['className'] == 'particlemodulerequired')
            loops = next((v['value'] for v in required['literals'] if v['propertyPath'] == 'emitterloops'), 0)
            assert element['sourceRecipe']['emitterLoopCount'] == loops
            if old['sourceRecipe']['emitterLoopCount'] != loops:
                changes.append(dict(elementId=element['id'], field='sourceRecipe.emitterLoopCount',
                    before=old['sourceRecipe']['emitterLoopCount'], after=loops,
                    evidence='Fresh source Required/archetype/CDO closure; absent integer is native zero'))
        assert len(body_ids) == 1
        reports.append(dict(effectAssetId=target['asset'], displayName=target['name'],
            sourceSystem=target['system'], elementCount=len(document['elements']), bodyElementIds=body_ids,
            nativeProfiles=sorted({e['material']['sourceProfile']['runtimeShaderProfileId'] for e in document['elements']}),
            defaultRepairs=changes, worldMotion='External occurrence owns movement and finite activation; particle lifetimes are unchanged'))
    for item in inputs:
        assert hashlib.sha256((source.ROOT / item['path']).read_bytes()).hexdigest() == item['sha256']
    source.write(evidence / 'blade_restore_receipt.json', dict(installed=False, originalDocuments=inputs,
        documents=reports, visualValidation='USER_PENDING'))
    for item in installation['documents']:
        item['parentPath'] = ['KoukuSaydon', '마리오 패턴']
        item['defaultAnchorKind'] = 'MAP'
    source.write(evidence / 'registration.json', installation)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, default=source.ROOT / 'out/KoukuMarioBladeEffects20260916')
    build(parser.parse_args().evidence_root)

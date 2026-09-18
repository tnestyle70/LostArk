"""Compose the Saydon stagger ("무력화") original effects into four V1 group documents.

Source action 4219945 (MN_RPCT_07, stage 5 Att_Battle_6_02 with an 8.1 s particle
window, stage 6 explosion) and its per-system authored library documents
(effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_*_loc_int) are the inputs.

* star:      vertex cards (sk_12_2) + the five drawn pentagram edges (sk_12_9 followers
             driven by the source LocationDirect tracer curves) + the completed star
             (sk_12_4 at 4.1 s) + the double flash (sk_12_3 at 6.116 s) + the source
             explosion (sk_12_5 at 8.1 s) + PROJECT_AUTHORED vertex bursts along the star
* boundary:  outer ring / centre charge (sk_12_6) + r7-9 m glitter ring (sk_12_7) +
             floor streak decals and 14 m light streaks (sk_12_8), all at 0 s
* combined:  star + boundary on one clock
* laser:     the per-slash laser strike (sk_12 at 0.609 s), ground impact (sk_12_1 at
             0.624 s) and the later residual change (sk_01_2 at 2.037 s) of Att_Battle_6_04

Source emitter delays and burst times are retained inside each element's sourceRecipe,
so the library documents keep their original ordering; only system-level offsets are
added here. Candidates go to out/; --install appends the four documents and their
catalog / resource-tree / project rows to the live data without touching other rows.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import re
import uuid
from pathlib import Path

from build_saydon_card_pattern_groups import AUTHORED, ROOT, leaf, read, renamed, append_group, current_preview, duration_ms
from build_kouku_dove_pizza_candidates import registration, native_references
from sync_kouku_effect_tree import stage_project_metadata

EVIDENCE = ROOT / 'out/StaggerBallValtan20260917/stagger'
CANDIDATE = EVIDENCE / 'candidate'
SK12_9_CANDIDATE = ROOT / 'out/KoukuAllEffects20260912/candidate/effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_9_loc_int.effect.json'
STAGGER_CATEGORY = 'kouku.category.cdef6f5c47c6e9a619ed'   # KoukuSaydon > 1관문 > 패턴 > 세이튼 > 세이튼_무력화 시작

STAR = 'effect.kouku.gate1.stagger.star.group'
BOUNDARY = 'effect.kouku.gate1.stagger.boundary.group'
COMBINED = 'effect.kouku.gate1.stagger.star.boundary.group'
LASER = 'effect.kouku.gate1.stagger.laser.group'
NAMES = {
    STAR: '세이튼 / 무력화 | 별 그리기·별 폭발',
    BOUNDARY: '세이튼 / 무력화 | 외곽 경계',
    COMBINED: '세이튼 / 무력화 | 무력화 별과 외곽 경계',
    LASER: '세이튼 / 무력화 | 레이저 생성·변화',
}

# Source timing of stage 5 (Att_Battle_6_02) relative to the stage start.
STAR_FULL_START = 4.1      # Par_L_RPCT_05_Sk_12_4 "Star02"
STAR_FLASH_START = 6.116   # Par_L_RPCT_05_Sk_12_3 "Star01"
EXPLOSION_START = 8.1      # stage 6 Par_L_RPCT_05_Sk_12_5 after the 8.1 s window
LASER_STRIKE, LASER_IMPACT, LASER_CHANGE = 0.609, 0.624, 2.037

# Par_L_RPCT_05_Sk_12_9 LocationDirect curves (UE3 cm, X/Y/Z-up), decoded from
# FX_MN_RPCT_05_L.particle-graph.json particlemodulelocationdirect_{0,1,2,5,7}.
# Each tracer lives 1.25 s and crosses its edge during relative time 0..0.5 (11 samples).
TRACER_TRAVERSE_SECONDS = 0.625
TRACER_LINES = {
    1: [[-700.0, 500.0, 40.0], [-660.800048828125, 486.55999755859375, 40.0], [-554.4000244140625, 450.0799865722656, 40.0],
        [-397.5999755859375, 396.32000732421875, 40.0], [-207.20001220703125, 331.0400085449219, 40.0], [0.0, 260.0, 40.0],
        [207.20001220703125, 188.9600067138672, 40.0], [397.599853515625, 123.68003845214844, 40.0],
        [554.4000244140625, 69.91998291015625, 40.0], [660.8001708984375, 33.43994140625, 40.0], [700.0, 20.0, 40.0]],
    2: [[700.0, 20.0, 40.0], [659.4000244140625, 6.0, 40.0], [549.2000122070312, -32.0, 40.0], [386.79998779296875, -88.0, 40.0],
        [189.60000610351562, -155.99998474121094, 40.0], [-25.0, -230.0, 40.0], [-239.60000610351562, -304.0, 40.0],
        [-436.79986572265625, -371.99993896484375, 40.0], [-599.2000122070312, -428.0000305175781, 40.0],
        [-709.4002075195312, -466.00006103515625, 40.0], [-750.0, -480.0, 40.0]],
    3: [[-700.0, -480.0, 40.0], [-674.2400512695312, -444.7200012207031, 40.0], [-604.3200073242188, -348.96002197265625, 40.0],
        [-501.2799987792969, -207.8399658203125, 40.0], [-376.1600036621094, -36.480010986328125, 40.0], [-240.0, 150.0, 40.0],
        [-103.83999633789062, 336.4800109863281, 40.0], [21.279922485351562, 507.83990478515625, 40.0],
        [124.32003021240234, 648.9600219726562, 40.0], [194.2401123046875, 744.7201538085938, 40.0], [220.0, 780.0, 40.0]],
    4: [[240.0, 780.0, 40.0], [240.0, 735.7599487304688, 40.0], [240.00001525878906, 615.6799926757812, 40.0],
        [239.99998474121094, 438.719970703125, 40.0], [240.0, 223.83999633789062, 40.0], [240.0, -10.0, 40.0],
        [240.0, -243.84002685546875, 40.0], [240.0, -458.71990966796875, 40.0], [240.0, -635.6800537109375, 40.0],
        [240.0, -755.7601928710938, 40.0], [240.0, -800.0, 40.0]],
    5: [[240.0, -800.0, 40.0], [212.27999877929688, -760.7999877929688, 40.0], [137.04000854492188, -654.4000244140625, 40.0],
        [26.159988403320312, -497.6000061035156, 40.0], [-108.47999572753906, -307.20001220703125, 40.0], [-255.0, -100.0, 40.0],
        [-401.52001953125, 107.19998168945312, 40.0], [-536.159912109375, 297.59991455078125, 40.0],
        [-647.0399780273438, 454.4000549316406, 40.0], [-722.2801513671875, 560.8001708984375, 40.0], [-750.0, 600.0, 40.0]],
}
TRACER_FOLLOWERS = {1: (21, 16), 2: (22, 17), 3: (23, 18), 4: (24, 19), 5: (25, 20)}   # smoke_tail, ninjaflow
FOLLOWER_EMISSION_SECONDS = 0.65
# The sk_12_9 library candidate carries no native material for its followers. The
# ninjaflow material is admitted as a sprite (native 2560). The source smoke_tail
# material fx_k_pa_turbpa_06_tr (native 3008) is registered ribbon-shaped, so a sprite
# element cannot admit it; the strike's admitted sprite smoke (native 2992) stands in
# as a PROJECT_AUTHORED substitute until 3008 gets a sprite table row.
MATERIAL_DONORS = {
    'fx_m_mi_02.fx_mi.fx_k_pa_turbpa_06_tr': ('effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_loc_int',
                                              'kouku.214170223.92de16cb6bbdae927a67', 'fx_m_mi_03.fx_mi.fx_m_pa_smoke_01_8_tr'),
    'fx_m_mi_k_00.fx_mi.fx_k_pa_ninjaflow_01_03_tr': ('effect.kouku.common.flame.wave.decal',
                                                      'effect.kouku.common.flame.wave.decal.09801db50a044080dd4d',
                                                      'fx_m_mi_k_00.fx_mi.fx_k_pa_ninjaflow_01_03_tr'),
}

# Installed authored basis: UE3 (x, y, z-up) cm -> [x/100, z/100, -y/100] m. Vertices in drawing order.
STAR_VERTICES_M = [('V5', [-6.75, 0.0, 4.75]), ('V1', [8.25, 0.0, 0.0]), ('V4', [-6.75, 0.0, -4.75]),
                   ('V2', [2.5, 0.0, 7.75]), ('V3', [2.5, 0.0, -7.75])]
VERTEX_BURST_EMITTERS = ('particlespriteemitter_53', 'particlespriteemitter_51', 'particlespriteemitter_49',
                         'particlespriteemitter_52', 'particlespriteemitter_54', 'particlespriteemitter_55',
                         'particlespriteemitter_60')
VERTEX_BURST_STAGGER_SECONDS = 0.12
VERTEX_BURST_SIZE_SCALE = 0.35
VERTEX_BURST_COUNT_SCALE = 0.6


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes((json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + '\n').encode('utf-8'))


def sha(data):
    return hashlib.sha256(data).hexdigest()


def stable(prefix, *parts):
    return prefix + hashlib.sha256('|'.join(parts).encode('utf8')).hexdigest()[:24]


def preview(stage_indices):
    """Boss preview clock: the selected KAKULSAYDON_G1_PATTERN_1 stages re-based to 0."""
    source = current_preview(1)
    composition = read(ROOT / 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json')
    pattern = next(p for p in composition['patterns'] if p['patternId'] == 'KAKULSAYDON_G1_PATTERN_1')
    offsets, offset = [], 0
    for stage in pattern['stages']:
        offsets.append(offset)
        offset += stage['durationMs']
    keep = [offsets[i] for i in stage_indices]
    base = min(keep)
    animations = [dict(a, startOffsetMs=a['startOffsetMs'] - base) for a in source['animations'] if a['startOffsetMs'] in keep]
    assert len(animations) == len(stage_indices)
    return dict(source, animations=animations)


def base_document(asset, stage_indices):
    document = renamed(leaf('12_2_loc_int'), asset, NAMES[asset])
    document['elements'] = []
    document.pop('sourceAnchorAnimations', None)
    document['sourceModelPreview'] = preview(stage_indices)
    return document


def linear_key(time, value):
    return dict(timeSeconds=time, value=[float(v) for v in value], arriveTangent=[0, 0, 0], leaveTangent=[0, 0, 0],
                interpolation='linear')


def line_track(element_id, start, samples):
    step = TRACER_TRAVERSE_SECONDS / (len(samples) - 1)
    keys = [linear_key(start + i * step, sample) for i, sample in enumerate(samples)]
    return dict(sourceOccurrenceId=element_id, sourceTimeOriginSeconds=0, previewOriginUE3Cm=[0, 0, 0],
                nodes=[dict(sourceObjectPath=element_id + '/edge', frame='WORLD', initialPositionUE3Cm=list(samples[0]),
                            initialEulerDegrees=[0, 0, 0], scaleUE3=[1, 1, 1], positionKeys=keys, eulerKeys=[])],
                alphaScaleKeys=[])


def donor_material(source_material_path):
    asset, element_id, donor_path = MATERIAL_DONORS[source_material_path]
    document = read(AUTHORED / f'{asset}.effect.json')
    element = next(e for e in document['elements'] if e['id'] == element_id)
    assert element['material']['sourceMaterialPath'] == donor_path
    assert element['material']['sourceProfile']['enabled']
    assert element['kind'] == 'particle' and element['sourceRecipe']['rendererShape'] == 'sprite'
    return copy.deepcopy(element['material']), donor_path


def star_line_elements(asset):
    """The five drawn pentagram edges: smoke_tail + ninjaflow followers carried along the tracer curve."""
    candidate = read(SK12_9_CANDIDATE)
    by_emitter = {e['displayName'].rsplit(' | ', 1)[-1]: e for e in candidate['elements']}
    group = asset + '.star.lines'
    elements = []
    for tracer, samples in TRACER_LINES.items():
        start = float(tracer - 1)
        for role, emitter_index in zip(('smoke', 'ninja'), TRACER_FOLLOWERS[tracer]):
            emitter = f'particlespriteemitter_{emitter_index}'
            element = copy.deepcopy(by_emitter[emitter])
            element['id'] = stable('kouku.stagger.line.', asset, emitter)
            element['displayName'] = f'star line {tracer} | {role} {emitter}'
            element['groupId'] = group
            element['visible'] = True
            element['resources'] = []
            source_path = element['material']['sourceMaterialPath']
            element['material'], donor_path = donor_material(source_path)
            if donor_path != source_path:
                element['displayName'] = f'star line {tracer} | {role}* {emitter}'
            particle = element['detail']['particle']
            particle['localSpace'] = False
            timing = element['detail']['timing']
            timing['startDelaySeconds'] = start
            timing['lifeTimeSeconds'] = FOLLOWER_EMISSION_SECONDS + max(particle['lifeTimeSeconds'])
            recipe = element['sourceRecipe']
            recipe['emitterDelaySeconds'] = 0.0
            recipe['emitterDurationSeconds'] = FOLLOWER_EMISSION_SECONDS
            recipe['emitterLoopCount'] = 1
            # The source followed the invisible tracer particle through
            # efparticlemodulelocationemitter; the transform track carries the emitter instead.
            recipe['modules'] = [m for m in recipe['modules'] if m['className'] != 'efparticlemodulelocationemitter']
            for module in recipe['modules']:
                if module['className'] == 'particlemodulerequired':
                    for literal in module['literals']:
                        if literal['propertyPath'] == 'emitterdelay':
                            literal['value'] = 0.0
                        if literal['propertyPath'] == 'material.objectpath':
                            literal['value'] = donor_path
            presentation = element.get('sourcePresentation')
            if isinstance(presentation, dict) and 'sourceTimeSeconds' in presentation:
                presentation['sourceTimeSeconds'] = start
            element['sourceTransformTrack'] = line_track(element['id'], start, samples)
            elements.append(element)
    return elements


def vertex_bursts(document):
    """PROJECT_AUTHORED: the explosion core repeated at the five star vertices in drawing order."""
    source = leaf('12_5_loc_int')
    core = copy.deepcopy(source)
    core['elements'] = [e for e in source['elements'] if e['sourceNode'].rsplit('.', 1)[-1] in VERTEX_BURST_EMITTERS]
    assert len(core['elements']) == len(VERTEX_BURST_EMITTERS), len(core['elements'])
    for index, (label, position) in enumerate(STAR_VERTICES_M):
        before = len(document['elements'])
        append_group(document, core, f'explosion.vertex.{label.lower()}', EXPLOSION_START + VERTEX_BURST_STAGGER_SECONDS * index)
        for element in document['elements'][before:]:
            element['displayName'] = f'vertex burst {label} | ' + element['displayName'].rsplit(' | ', 1)[-1]
            transform = element['detail']['transform']
            transform['position'] = [transform['position'][0] + position[0], transform['position'][1] + position[1],
                                     transform['position'][2] + position[2]]
            scale = element['detail']['particle']['sourceScale']
            scale['size'] = scale['size'] * VERTEX_BURST_SIZE_SCALE
            scale['count'] = scale['count'] * VERTEX_BURST_COUNT_SCALE


def add_star(document):
    append_group(document, leaf('12_2_loc_int'), 'vertex.cards', 0)
    document['elements'].extend(star_line_elements(document['effectAssetId']))
    append_group(document, leaf('12_4_loc_int'), 'star.full', STAR_FULL_START)
    append_group(document, leaf('12_3_loc_int'), 'star.flash', STAR_FLASH_START)
    append_group(document, leaf('12_5_loc_int'), 'explosion', EXPLOSION_START)
    vertex_bursts(document)


def add_boundary(document):
    append_group(document, leaf('12_6_loc_int'), 'ring.charge', 0)
    append_group(document, leaf('12_7_loc_int'), 'ring.glitter', 0)
    append_group(document, leaf('12_8_loc_int'), 'floor.streaks', 0)


def add_laser(document):
    append_group(document, leaf('12_loc_int'), 'strike', LASER_STRIKE)
    append_group(document, leaf('12_1_loc_int'), 'impact', LASER_IMPACT)
    append_group(document, leaf('01_2_loc_int'), 'change', LASER_CHANGE)


def finish(document):
    ids = [e['id'] for e in document['elements']]
    assert len(ids) == len(set(ids)), 'duplicate element ids'
    for element in document['elements']:
        assert re.fullmatch(r'[a-z0-9][a-z0-9._-]{0,127}', element['id']), element['id']
        assert 0 < len(element['displayName'].encode('utf8')) <= 64, element['displayName']
        assert element['material']['sourceProfile']['enabled'] or element['kind'] in ('light', 'screenPost'), element['id']
    assert len(document['displayName'].encode('utf8')) <= 64
    native_references(document)
    return document


def build_all():
    star = base_document(STAR, [2, 3, 4, 5, 6])
    add_star(star)
    boundary = base_document(BOUNDARY, [2, 3, 4, 5, 6])
    add_boundary(boundary)
    combined = base_document(COMBINED, [2, 3, 4, 5, 6])
    add_star(combined)
    add_boundary(combined)
    laser = base_document(LASER, [1])
    add_laser(laser)
    return [finish(d) for d in (star, boundary, combined, laser)]


def append_catalog_row(text, row):
    start = text.index('\n  "effects": [')
    end = text.index('\n  ]\n}', start)
    block = json.dumps(row, ensure_ascii=False, indent=2)
    block = '\n'.join('    ' + line for line in block.split('\n'))
    return text[:end] + ',\n' + block + text[end:]


def append_tree_row(text, row):
    newline = '\r\n' if '\r\n' in text else '\n'
    marker = newline + '  ]' + newline + '}'
    end = text.rindex(marker)
    return text[:end] + ',' + newline + '    ' + json.dumps(row, ensure_ascii=False) + text[end:]


def install(documents, entries):
    catalog_path = ROOT / 'Data/Effects/EffectCatalog.json'
    tree_path = ROOT / 'Data/Effects/EffectResourceTree.json'
    catalog_text = catalog_path.read_bytes().decode('utf-8')
    tree_text = tree_path.read_bytes().decode('utf-8')
    catalog = json.loads(catalog_text)
    tree = json.loads(tree_text)
    assert not catalog_text.startswith('﻿') and not tree_text.startswith('﻿')
    known = {r['effectAssetId']: r for r in catalog['effects']}
    references = {(r['kind'], r['assetId']): r for r in tree['references']}
    node_ids = {n['id'] for n in tree['nodes']}
    staged_docs = []
    for document, entry in zip(documents, entries):
        asset = document['effectAssetId']
        target = AUTHORED / f'{asset}.effect.json'
        payload = (CANDIDATE / f'{asset}.effect.json').read_bytes()
        if target.exists() and target.read_bytes() != payload:
            raise SystemExit(f'{asset}: a different live document exists; refusing to overwrite')
        staged_docs.append((target, payload))
        row = entry['catalogEntry']
        assert known.get(asset) in (None, row), f'{asset}: catalog row conflict'
        if asset not in known:
            catalog_text = append_catalog_row(catalog_text, row)
            known[asset] = row
        reference = entry['treeReference']
        assert reference['parentId'] in node_ids
        assert references.get(('V1', asset)) in (None, reference), f'{asset}: tree reference conflict'
        if ('V1', asset) not in references:
            tree_text = append_tree_row(tree_text, reference)
            references[('V1', asset)] = reference
    updated_catalog, updated_tree = json.loads(catalog_text), json.loads(tree_text)
    assert [r['effectAssetId'] for r in updated_catalog['effects']][:len(catalog['effects'])] == [r['effectAssetId'] for r in catalog['effects']]
    assert updated_tree['nodes'] == tree['nodes'] and updated_tree['references'][:len(tree['references'])] == tree['references']
    project = stage_project_metadata([target for target, _ in staged_docs])
    writes = [(target, payload) for target, payload in staged_docs]
    writes += [(catalog_path, catalog_text.encode('utf-8')), (tree_path, tree_text.encode('utf-8'))]
    writes += [(path, after) for path, before, after in project if before != after]
    backup_root = EVIDENCE / 'before'
    for path, payload in writes:
        if path.exists():
            backup = backup_root / path.relative_to(ROOT)
            backup.parent.mkdir(parents=True, exist_ok=True)
            backup.write_bytes(path.read_bytes())
        temp = path.with_name(path.name + f'.claude-{os.getpid()}-{uuid.uuid4().hex[:8]}.tmp')
        with open(temp, 'xb') as handle:
            handle.write(payload)
        os.replace(temp, path)
    return [str(path.relative_to(ROOT)) for path, _ in writes]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    documents = build_all()
    entries, fixtures, summary = [], [], {}
    for document in documents:
        asset = document['effectAssetId']
        path = CANDIDATE / f'{asset}.effect.json'
        write(path, document)
        entry = registration(document, 'effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_3_loc_int')
        assert entry['treeReference']['parentId'] == STAGGER_CATEGORY
        entry['compositionResourceProposal'] = dict(resourceId='kakulsaydon.effect.' + hashlib.sha256(asset.encode()).hexdigest()[:20],
                                                    displayName=document['displayName'], defaultAnchorKind='BOSS', kind='EFFECT',
                                                    assetId=asset, resourceKind='V1_EFFECT', durationMs=duration_ms(document))
        entries.append(entry)
        fixtures.append(dict(name=asset, passExpected=True, path=str(path), values={}))
        groups = {}
        for element in document['elements']:
            groups[element['groupId']] = groups.get(element['groupId'], 0) + 1
        summary[asset] = dict(displayName=document['displayName'], elements=len(document['elements']),
                              durationMs=duration_ms(document), groups=groups, sha256=sha(path.read_bytes()))
    write(EVIDENCE / 'fixtures.json', fixtures)
    write(EVIDENCE / 'pending-registration.json', dict(installed=False, entries=entries))
    receipt = dict(installed=False, summary=summary)
    if args.install:
        receipt['written'] = install(documents, entries)
        receipt['installed'] = True
    write(EVIDENCE / 'receipt.json', receipt)
    print(json.dumps(summary, ensure_ascii=False, indent=1))


if __name__ == '__main__':
    main()

"""Restore the source Matinee movement of Kouku's golden stage trails.

Preserve existing sequence timing and authoring edits. The standalone document
reuses the same four stage occurrences, translated to the original stage floor.
"""
import argparse
import copy
import hashlib
import json
import math
import re
from pathlib import Path

import build_kouku_sequence_effect_groups as sequence

ROOT = sequence.ROOT
SOURCE_SYSTEM = 'fx_q_w_01.fx_par_02.par_q_trail_01'
ASSET = 'effect.kouku.gate1.intro.gold-trails.full.restore'
LABEL = '1관문_금빛 이동 축포_무대 4경로'
PAIRS = [
    ('effect.kouku.sequence.lv_lut_midnightc_ed_scene03a.efseqact_matinee_0.' + str(i),
     'effect.kouku.gate1.authored.portal-arrival.' + str(i)) for i in (1, 2)]


def read(path):
    return json.loads(path.read_bytes())


def payload(value):
    return (json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + '\n').encode('utf8')


def replace_nodes(original, recovered):
    """Replace only serialized node arrays, keeping all other bytes unchanged."""
    text = original.decode('utf-8-sig')
    newline = '\r\n' if '\r\n' in text else '\n'
    edits = []
    for match in re.finditer(r'^        "nodes": ', text, re.MULTILINE):
        nodes, length = json.JSONDecoder().raw_decode(text[match.end():])
        key = nodes[-1]['sourceObjectPath'] if nodes else ''
        if key not in recovered or nodes == recovered[key]:
            continue
        expected = copy.deepcopy(recovered[key])
        assert len(nodes) == len(expected) == 1, 'Selected trails have no parent chain'
        assert not nodes[0]['positionKeys'] and not nodes[0]['eulerKeys'], 'Preserve authored motion edits'
        expected[0]['positionKeys'] = []
        expected[0]['eulerKeys'] = []
        assert nodes == expected, 'Preserve authored frame, placement, rotation and scale'
        encoded = json.dumps(recovered[key], ensure_ascii=False, indent=2, allow_nan=False)
        encoded = encoded.replace('\n', newline + '        ')
        edits.append((match.end(), match.end() + length, encoded))
    for start, end, encoded in reversed(edits):
        text = text[:start] + encoded + text[end:]
    return (b'\xef\xbb\xbf' if original.startswith(b'\xef\xbb\xbf') else b'') + text.encode('utf8'), len(edits)


def verify_required_default(defaults_path):
    records = {row['fullPath']: row for row in read(defaults_path)['records']}
    key = 'engine.default__particlemodulerequired'
    chain = []
    while key:
        assert key not in chain, 'Source CDO cycle'
        chain.append(key)
        row = records[key]
        props = {name.lower(): value for name, value in row['properties'].items()}
        assert 'emitterloops' not in props, 'Source loop default changed'
        if key == chain[0]:
            assert props['emitterduration']['value'] == 1.0
        key = row['archetypeFullPath']
    assert chain == ['engine.default__particlemodulerequired',
                     'engine.default__particlemodule', 'core.default__object']
    return chain


def replace_provisional_loops(original):
    """The complete CDO chain uses zero loops; source Toggle owns the stop."""
    text = original.decode('utf-8-sig')
    edits = []
    for match in re.finditer(r'^      "sourceRecipe": ', text, re.MULTILINE):
        recipe, length = json.JSONDecoder().raw_decode(text[match.end():])
        required = next((m for m in recipe.get('modules', []) if m['className'] == 'particlemodulerequired'), None)
        if not required or not required['stableId'].startswith(SOURCE_SYSTEM + '.'):
            continue
        assert not any(v['propertyPath'] == 'emitterloops' for v in required['literals'])
        assert recipe['emitterDurationSeconds'] == 1.0 and recipe['emitterLoopCount'] in (0, 1)
        if recipe['emitterLoopCount'] == 0:
            continue
        block = text[match.end():match.end() + length]
        count = re.search(r'"emitterLoopCount": (1)(?=\s*[,}])', block)
        assert count, 'Unexpected loop field serialization'
        edits.append(match.end() + count.start(1))
    for position in reversed(edits):
        text = text[:position] + '0' + text[position + 1:]
    return (b'\xef\xbb\xbf' if original.startswith(b'\xef\xbb\xbf') else b'') + text.encode('utf8'), len(edits)


def build(organization_path, placements_path, defaults_path, output, install):
    default_chain = verify_required_default(defaults_path)
    organization = read(organization_path)
    occurrences = [row for row in organization['sequences']['occurrences']
                   if row['sourceScene'].lower() == 'lv_lut_midnightc_ed_scene03a'
                   and row['matineeId'] == 'efseqact_matinee_0'
                   and row['sourceSystem'] == SOURCE_SYSTEM and row['liveReference'] and not row['disabled']]
    assert len(occurrences) == 8
    recovered = {}
    for occurrence in occurrences:
        nodes = sequence.moving_transform(occurrence, read(Path(occurrence['sourceCache'])))['sourceTransformNodes']
        assert len(nodes) == 1 and nodes[0]['positionKeys'] and nodes[0]['eulerKeys']
        recovered[nodes[0]['sourceObjectPath']] = nodes

    floor = next(row for row in read(placements_path)['placements']
                 if row['placementId'] == 'LV_LUT_MIDNIGHTC_ED_SL04:export:206')
    assert floor['asset']['objectName'] == 'bg_rad_koukusaton_floor08_sm'
    assert floor['transform']['coordinateSystem'] == 'UE3-native'
    origin = sequence.vector(floor['transform']['position'])
    snapshots, staged, reports = {}, {}, []
    output.mkdir(parents=True, exist_ok=True)
    (output / 'before').mkdir(exist_ok=True)
    (output / 'candidate').mkdir(exist_ok=True)
    for pair in PAIRS:
        for asset in pair:
            path = ROOT / 'Data/Effects/Authored' / (asset + '.effect.json')
            before = path.read_bytes()
            value, changed = replace_nodes(before, recovered)
            value, loop_changes = replace_provisional_loops(value)
            snapshots[path] = before
            staged[path] = value
            original = json.loads(before)
            repaired = json.loads(value)
            check = copy.deepcopy(repaired)
            for old, new in zip(original['elements'], check['elements']):
                new['sourceTransformTrack']['nodes'] = old['sourceTransformTrack']['nodes']
                new['sourceRecipe']['emitterLoopCount'] = old['sourceRecipe']['emitterLoopCount']
            assert check == original, 'Only source node curves and provisional loops may change'
            (output / 'before' / path.name).write_bytes(before)
            (output / 'candidate' / path.name).write_bytes(value)
            reports.append(dict(path=path.relative_to(ROOT).as_posix(), changedElements=changed,
                                restoredLoopElements=loop_changes,
                                beforeSha256=hashlib.sha256(before).hexdigest(),
                                afterSha256=hashlib.sha256(value).hexdigest()))

    stage_path = ROOT / 'Data/Effects/Authored' / (PAIRS[1][0] + '.effect.json')
    standalone = json.loads(staged[stage_path])
    assert len(standalone['elements']) == 24
    first = min(e['detail']['timing']['startDelaySeconds'] for e in standalone['elements'])
    for element in standalone['elements']:
        track = element['sourceTransformTrack']
        assert len(track['nodes']) == 1 and track['nodes'][0]['sourceObjectPath'] in recovered
        assert element['sourcePresentation']['sourceObjectPath'].startswith(SOURCE_SYSTEM + '.')
        element['detail']['timing']['startDelaySeconds'] -= first
        track['sourceTimeOriginSeconds'] += first
        track['previewOriginUE3Cm'] = origin
    standalone['effectAssetId'] = ASSET
    standalone['displayName'] = LABEL
    target = ROOT / 'Data/Effects/Authored' / (ASSET + '.effect.json')
    value = payload(standalone)
    assert not target.exists() or target.read_bytes() == value, 'Preserve standalone authoring edits'
    snapshots[target] = target.read_bytes() if target.exists() else None
    staged[target] = value
    (output / 'candidate' / target.name).write_bytes(value)
    duration = math.ceil(max(e['detail']['timing']['startDelaySeconds'] +
                            e['detail']['timing']['lifeTimeSeconds'] +
                            max(e['detail']['particle']['lifeTimeSeconds']) for e in standalone['elements']) * 1000)
    for path, before in snapshots.items():
        assert (path.read_bytes() if path.exists() else None) == before, 'Concurrent authoring edit: ' + str(path)
    if install:
        committed = []
        try:
            for path, value in staged.items():
                if value != snapshots[path]:
                    path.write_bytes(value)
                    committed.append(path)
        except Exception:
            for path in reversed(committed):
                if snapshots[path] is None:
                    path.unlink()
                else:
                    path.write_bytes(snapshots[path])
            raise
    manifest = dict(installed=install, documents=[dict(effectAssetId=ASSET,
        path=target.relative_to(ROOT).as_posix(), displayName=LABEL, elementCount=24,
        durationMs=duration, defaultAnchorKind='MAP',
        categoryPath=['KoukuSaydon', '1관문', '연출', '금빛 이동 축포'])],
        repairedDocuments=reports, sourceParticleSystem=SOURCE_SYSTEM, sourceRequiredDefaultChain=default_chain,
        sourceEmitterLoopCount=0, emissionStopOwner='ORIGINAL_MATINEE_TOGGLE_INTERVAL',
        previewOriginUE3Cm=origin, sourceFloorPlacementId=floor['placementId'],
        originalFirstActivationSeconds=standalone['elements'][0]['sourceTransformTrack']['sourceTimeOriginSeconds'],
        manualVisualValidation='USER_PENDING')
    (output / 'installation.json').write_bytes(payload(manifest))
    print(json.dumps(dict(installed=install, repairedElements=sum(r['changedElements'] for r in reports),
                         standaloneElements=24, durationMs=duration)))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--organization', type=Path, default=ROOT / 'out/KoukuAllEffects20260912/organization.json')
    parser.add_argument('--stage-placements', type=Path, required=True)
    parser.add_argument('--source-defaults', type=Path, default=ROOT / 'out/KoukuAllEffects20260912/source_class_defaults.json')
    parser.add_argument('--output', type=Path, default=ROOT / 'out/KoukuGoldTrails20260913')
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    build(args.organization, args.stage_placements, args.source_defaults, args.output, args.install)

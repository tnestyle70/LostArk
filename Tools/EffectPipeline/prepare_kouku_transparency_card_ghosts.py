"""Stage narrow Saydon appearance edits against the latest saved effects.

Never installs candidates. The deleted card ghost is recovered from the admitted
eight-element source document, not reconstructed from the user's six survivors.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import subprocess
from pathlib import Path

from build_saydon_card_pattern_groups import (
    ROOT, AUTHORED, SUITS, renamed, fixed_subimage, scale_spinning_card_geometry,
)

SOURCE_REVISION = '084c80822'
SOURCE_ASSET = 'effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_13_1_loc_int'
GHOST_SOURCE = 'fx_mn_rpct_05_l.par_l_rpct_05_sk_13_1_loc_int.particlespriteemitter_0'


def digest(raw):
    return hashlib.sha256(raw).hexdigest()


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes((json.dumps(value, ensure_ascii=False, indent=2,
                               allow_nan=False) + '\n').encode('utf-8'))


def prepare(output):
    output = output.resolve()
    assert output.is_relative_to(ROOT / 'out') and output != ROOT / 'out'
    source_path = 'Data/Effects/Authored/' + SOURCE_ASSET + '.effect.json'
    source_raw = subprocess.check_output(['git', 'show', SOURCE_REVISION + ':' + source_path], cwd=ROOT)
    source = json.loads(source_raw)
    ghost, = [e for e in source['elements'] if e['sourceNode'].endswith(GHOST_SOURCE)]
    assert len(source['elements']) == 8
    assert ghost['material']['sourceProfile']['runtimeShaderProfileId'] == 'effect.ue3.kouku-2999-native.v1'
    assert not ghost['detail']['particle']['localSpace']
    assert ghost['detail']['particle']['spawnRatePerSecond'] == 0
    per_unit, = [m for m in ghost['sourceRecipe']['modules'] if m['className'] == 'particlemodulespawnperunit']
    assert any(v['propertyPath'] == 'unitscalar' and v['value'] == 30 for v in per_unit['literals'])
    entries, resources, captured = [], set(), []

    def stage(asset, edit):
        live = AUTHORED / (asset + '.effect.json')
        raw = live.read_bytes()
        original = json.loads(raw)
        candidate = copy.deepcopy(original)
        changed = edit(candidate)
        assert candidate['effectAssetId'] == original['effectAssetId']
        assert len({e['id'] for e in candidate['elements']}) == len(candidate['elements'])
        for e in candidate['elements']:
            resources.update(r['assetId'] for r in e.get('resources', []))
            resources.update(r['assetId'] for r in e['material']['sourceProfile'].get('textures', []))
        resources.update(c['modelAssetId'] for c in candidate.get('modelCues', []))
        before = output / 'before' / live.name
        before.parent.mkdir(parents=True, exist_ok=True)
        before.write_bytes(raw)
        target = output / 'candidate' / live.name
        write(target, candidate)
        captured.append((live, raw))
        entries.append(dict(path=live.relative_to(ROOT).as_posix(), effectAssetId=asset,
                            beforePath=before.relative_to(ROOT).as_posix(), beforeSha256=digest(raw),
                            candidatePath=target.relative_to(ROOT).as_posix(),
                            candidateSha256=digest(target.read_bytes()), changes=changed))

    for suit, _, _, tile in SUITS:
        asset = 'effect.kouku.card.spinning.' + suit

        def restore(doc, tile=tile):
            assert not any(e['sourceNode'].endswith(GHOST_SOURCE) for e in doc['elements'])
            # Reuse the original stable-ID derivation and native world-space
            # SpawnPerUnit module. Only the existing suit and 1.5x size apply.
            donor = copy.deepcopy(source)
            donor['elements'] = [copy.deepcopy(ghost)]
            donor = renamed(donor, doc['effectAssetId'], doc['displayName'])
            added = donor['elements'][0]
            fixed_subimage(added, tile)
            scale_spinning_card_geometry(donor)
            doc['elements'].append(added)
            return [dict(elementId=added['id'], operation='restore-source-element',
                         sourceObjectPath=GHOST_SOURCE, sourceRevision=SOURCE_REVISION,
                         suitTile=tile, sizeMultiplier=1.5)]

        stage(asset, restore)

    def blue(doc):
        changes = []
        for element in doc['elements']:
            if element['material']['renderProfile'].startswith('alpha_'):
                color = element['detail']['color']
                before = color['multiply'][3]
                color['multiply'][3] *= .5
                changes.append(dict(elementId=element['id'], field='detail.color.multiply[3]',
                                    before=before, after=color['multiply'][3]))
        assert len(changes) == 7
        return changes

    stage('effect.kouku.gate2.safezone.blue.full.restore', blue)

    def counter(doc):
        element, = doc['elements']
        color = element['detail']['color']
        before = copy.deepcopy(color['multiply'])
        peak = max(before[:3])
        assert peak > 1 and before[3] > 1
        color['multiply'] = [v / peak for v in before[:3]] + [.5]
        return [dict(elementId=element['id'], field='detail.color.multiply',
                     before=before, after=color['multiply'])]

    stage('effect.kouku.common.counter.ring', counter)

    for suffix in ('charge_counter.afterimage', 'charge_counter.group', 'backstep.afterimage'):
        def afterimage(doc, backstep=suffix.startswith('backstep')):
            changes = []
            for cue in doc['modelCues']:
                before = copy.deepcopy(cue)
                white = .5 if backstep else .8
                cue['colorMultiply'] = [white, white, white, .19]
                cue['afterimage']['endColor'] = [white, white, white, 0]
                if backstep:
                    cue['afterimage']['sourceColorIntensity'] = .5
                changes.append(dict(cueId=cue['cueId'], before=before, after=copy.deepcopy(cue)))
            return changes
        stage('effect.kouku.gate1.' + suffix, afterimage)

    def beam_axes(doc):
        changes = []
        for element in doc['elements']:
            if element['sourceNode'].split('|')[-1] not in {
                'fx_mn_rpct_07_v.par_v_rpct_eye_atk_01_loc_int.particlespriteemitter_5',
                'fx_mn_rpct_07_v.par_v_rpct_eye_atk_01_loc_int.particlespriteemitter_28',
            }:
                continue
            sprite = element['detail']['sprite']
            assert not sprite.get('followEmitterAxisRotation', False)
            sprite['followEmitterAxisRotation'] = True
            changes.append(dict(elementId=element['id'],
                                field='detail.sprite.followEmitterAxisRotation',
                                before=False, after=True))
        assert len(changes) == 4
        return changes

    stage('effect.kouku.bingo.encore.blackhole.beam.full.restore', beam_axes)

    missing = sorted(r for r in resources if not (ROOT / 'Client/Bin/Resources' / r).is_file())
    assert not missing, missing
    assert all(p.read_bytes() == raw for p, raw in captured), 'Saved effect changed while staging.'
    write(output / 'manifest.json', dict(installed=False, documents=entries,
          source=dict(revision=SOURCE_REVISION, path=source_path, sha256=digest(source_raw)),
          resourcesChecked=len(resources), missingResources=missing,
          liveCompositionChanged=False, nativeShaderChanged=False,
          appearanceBasis='PROJECT_AUTHORED opacity/exposure; source material equations preserved',
          manualVisualValidation='USER_PENDING'))
    print(json.dumps(dict(candidates=len(entries), resources=len(resources),
                         manifest=str(output / 'manifest.json'))))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--output', type=Path,
                        default=ROOT / 'out/KoukuRaidPolish20260924/effects')
    prepare(parser.parse_args().output)

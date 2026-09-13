"""Build the G12 cutscene targets (2026-09-13) from source Matinees.

Targets:
  P3 KAKULSAYDON_G1_PATTERN_3  2관문_진입      camera shots regenerated from SCENE04A Matinee2
  P7 KAKULSAYDON_G1_PATTERN_7  3관문_진입      full SCENE02A Matinee10 window (0..35368ms)
  P8 KAKULSAYDON_G1_PATTERN_8  빙고_최종엔딩씬  new pattern from SCENE01B Matinee0 (0..49083ms)

Cameras and source props use the same generator functions as build_source_sequences
(make_cameras/static_worlds). P7 actors reuse the 2관문_클리어 baked worlds, which
were sampled from the same Matinee at the same 0ms origin. Rows that belong to
other owners (fade, effects, lights, scene profiles, world definitions) are moved
in time where the plan says so and never edited or deleted.

Without --install the three candidate documents are written under OUT only.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import shutil
from pathlib import Path

import build_source_sequences as bss
import build_gate2_intro_composition as base

ROOT = base.ROOT
OUT = ROOT / 'out/KoukuGateCutscenes20260913'
PHYSICAL = {  # logical scene -> obfuscated package in the installed game
    'SCENE04A': 'B9AVB2VAZIQRPQCJVKAVYRAVOKYPY806.upk',
    'SCENE02A': 'B9AVB2VAZIQRPQCJVKAVYRAVOKYPY8M6.upk',
    'SCENE01B': 'B9AVB2VAZIQRPQCJVKAVYRAVOKYPY8FD.upk',
}
P7_TIME_SHIFT_MS = 16710  # former source window start; existing P7 boxes keep their source clock
P7_ACTOR_DONORS = ['kakulsaydon.g1.world.28', 'kakulsaydon.g1.world.29', 'kakulsaydon.g1.world.30']
CONFIGS = [
    dict(id=3, name='2관문_진입', prefix='kouku.gate2.intro', scene='SCENE04A', matinee=329, data=394,
         duration=27000, start=0, gate='GATE2', combat=True, mode='cameras'),
    dict(id=7, name='3관문_진입', prefix='kouku.gate3.intro', scene='SCENE02A', matinee=63, data=117,
         duration=35368, start=0, gate='GATE3', combat=True, mode='full-window'),
    dict(id=8, name='빙고_최종엔딩씬', prefix='kouku.bingo.ending', scene='SCENE01B', matinee=32, data=45,
         duration=49083, start=0, gate='GATE3', combat=False, mode='new'),
]


def scene_rows(config):
    cache = bss.OUT / ('LV_LUT_MIDNIGHTC_ED_' + config['scene'] + '.json')
    if not cache.exists():
        rows, imports = base.extract_scene(base.PACKAGES / PHYSICAL[config['scene']])
        base.write(cache, dict(rows=rows, imports=imports))
    return bss.scene_rows(config)


def replace_cameras(docs, pattern, config, rows):
    """Replace this pattern's camera shots/resources/occurrences; keep every other box."""
    shots = base.make_cameras(rows, config['matinee'], config['data'], config['duration'],
                              config['prefix'], config['name'], config['start'])
    shot_prefix = config['prefix'] + '.camera.'
    resource_prefix = 'presentation.' + shot_prefix
    docs['cameras']['shots'] = [s for s in docs['cameras']['shots'] if not s['shotId'].startswith(shot_prefix)]
    docs['cameras']['shots'] += [s for _, _, s in shots]
    docs['composition']['presentationResources'] = [
        r for r in docs['composition']['presentationResources'] if not r['resourceId'].startswith(resource_prefix)]
    kept = [o for o in pattern['presentationOccurrences'] if not o['resourceId'].startswith(resource_prefix)]
    ordinal = pattern.get('nextPresentationOccurrenceOrdinal', len(pattern['presentationOccurrences']) + 1)
    added = []
    for start, end, shot in shots:
        r = bss.resource('presentation.' + shot['shotId'], shot['displayName'], 'CAMERA', shot['shotId'], end - start)
        docs['composition']['presentationResources'].append(r)
        added.append(bss.occurrence(pattern['patternId'], ordinal, r['resourceId'], start, end))
        ordinal += 1
    pattern['presentationOccurrences'] = added + kept
    pattern['nextPresentationOccurrenceOrdinal'] = ordinal
    return shots


def new_pattern(config):
    pid = 'KAKULSAYDON_G1_PATTERN_' + str(config['id'])
    return dict(patternId=pid, actorProfileId='MN_RPCT_05' if config['gate'] != 'GATE2' else 'MN_RPCZ_00',
                gateId=config['gate'],
                targetBossPlacementId='boss.kakulsaydon.' + dict(GATE1='g1.saydon', GATE2='g2.kouku', GATE3='g3.saydon')[config['gate']],
                displayName=config['name'], authoringStatus='DRAFT', category='MECHANIC', enterCombatOnFinish=config['combat'],
                nextStageOrdinal=2, nextAnimationOrdinal=1, nextLogicOccurrenceOrdinal=1, nextSummonOccurrenceOrdinal=1,
                nextWorldOccurrenceOrdinal=1, nextSceneProfileOccurrenceOrdinal=1, nextPresentationOccurrenceOrdinal=1,
                stages=[dict(stageId='STAGE_1', actionId=pid + '.stage.1', stageKind='ACTIVE',
                             durationMs=config['duration'] - config['start'], animationOccurrences=[])],
                logicOccurrences=[], summonOccurrences=[], worldOccurrences=[], sceneProfileOccurrences=[],
                resetBossToSpawn=False, presentationOccurrences=[])


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    paths = dict(composition=ROOT / 'Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json',
                 cameras=bss.AREA / (base.AREA + '.camerashots.json'),
                 worlds=bss.AREA / (base.AREA + '.worldsequences.json'))
    baseline = {k: p.read_bytes() for k, p in paths.items()}
    docs = {k: json.loads(v.decode('utf-8-sig')) for k, v in baseline.items()}
    composition = docs['composition']
    patterns = {p['patternId']: p for p in composition['patterns']}
    world_ids = {w['worldId'] for w in composition['worlds']}
    summaries = []
    for config in CONFIGS:
        rows = scene_rows(config)
        pid = 'KAKULSAYDON_G1_PATTERN_' + str(config['id'])
        duration = config['duration'] - config['start']
        summary = dict(patternId=pid, name=config['name'], sourceScene=config['scene'], sourceMatinee=config['matinee'],
                       durationMs=duration, mode=config['mode'])
        if config['mode'] == 'new' and pid not in patterns:
            assert composition['nextPatternOrdinal'] == config['id'], ('nextPatternOrdinal', composition['nextPatternOrdinal'])
            pattern = new_pattern(config)
            composition['patterns'].append(pattern)
            patterns[pid] = pattern
            composition['nextPatternOrdinal'] += 1
        else:
            pattern = patterns[pid]
        if config['mode'] in ('cameras', 'new'):
            pattern['displayName'] = config['name']
            assert pattern['stages'][0]['durationMs'] == duration
        # A rerun finds the window already widened; only the first run moves boxes.
        if config['mode'] == 'full-window' and pattern['stages'][0]['durationMs'] == duration:
            summary['movedBoxes'] = 'already full window'
        elif config['mode'] == 'full-window':
            old = pattern['stages'][0]['durationMs']
            assert old == config['duration'] - P7_TIME_SHIFT_MS, ('unexpected P7 window', old)
            pattern['displayName'] = config['name']
            pattern['stages'][0]['durationMs'] = duration
            # Existing fade/effect/scene-profile boxes keep their source clock: same
            # source instant, now measured from the full-window origin.
            moved = []
            for box in pattern['presentationOccurrences']:
                if not box['resourceId'].startswith('presentation.' + config['prefix'] + '.camera.'):
                    box['startMs'] += P7_TIME_SHIFT_MS
                    assert box['startMs'] + box['durationMs'] <= duration, box['occurrenceId']
                    moved.append((box['occurrenceId'], box['startMs'], box['durationMs']))
            for box in pattern['sceneProfileOccurrences']:
                box['startMs'] += P7_TIME_SHIFT_MS
                assert box['startMs'] + box['durationMs'] <= duration, box['occurrenceId']
                moved.append((box['occurrenceId'], box['startMs'], box['durationMs']))
            summary['movedBoxes'] = moved
            # Actors: the 2관문_클리어 worlds were baked from this Matinee at 0ms.
            assert all(w in world_ids for w in P7_ACTOR_DONORS)
            ordinal = pattern['nextWorldOccurrenceOrdinal']
            pattern['worldOccurrences'] = []
            for world in P7_ACTOR_DONORS:
                pattern['worldOccurrences'].append(dict(occurrenceId=f'{pid}.world.{ordinal}', worldId=world,
                                                        startMs=0, durationMs=duration, playbackSpeed=1))
                ordinal += 1
            pattern['nextWorldOccurrenceOrdinal'] = ordinal
            summary['worldOccurrences'] = [o['worldId'] for o in pattern['worldOccurrences']]
        shots = replace_cameras(docs, pattern, config, rows)
        summary['cameraShots'] = [dict(shotId=s['shotId'], startMs=a, endMs=b, keys=len(s['cameraTrack']['keyframes']))
                                  for a, b, s in shots]
        if config['mode'] != 'cameras':
            # Source map props: none of these two Matinees animate a resolvable static mesh;
            # record the count so the report proves it rather than assuming it.
            props, sets, failures, count = bss.static_worlds(config, rows)
            assert count == 0 and not sets, ('unexpected source props', count, len(sets))
            summary['sourceStaticProps'] = count
        summaries.append(summary)
    for key in ('composition', 'cameras'):
        docs[key]['revision'] += 1
    camera_bytes = json.dumps(docs['cameras'], ensure_ascii=False, indent=2).encode('utf-8')
    assert len(camera_bytes) < 2 * 1024 * 1024, 'Camera exceeds runtime byte bound'
    assert len(docs['cameras']['shots']) <= 128, len(docs['cameras']['shots'])
    for shot in docs['cameras']['shots']:
        assert len(shot['cameraTrack']['keyframes']) <= 64 if 'cameraTrack' in shot else True, shot['shotId']
    outputs = {paths['composition']: docs['composition'], paths['cameras']: docs['cameras']}
    if OUT.exists():
        shutil.rmtree(OUT)
    for path, document in outputs.items():
        base.write(OUT / 'candidate' / path.relative_to(ROOT), document)
    base.write(OUT / 'report.json', dict(installed=args.install, patterns=summaries,
               cameraBytes=len(camera_bytes), cameraShots=len(docs['cameras']['shots']),
               baseline={k: hashlib.sha256(v).hexdigest() for k, v in baseline.items()},
               manualVisualValidation='USER_PENDING'))
    if args.install:
        for key, path in paths.items():
            if path.read_bytes() != baseline[key]:
                raise ValueError('Concurrent authoring changed: ' + str(path))
        for path, document in outputs.items():
            base.write(path, document)
    print(json.dumps(dict(installed=args.install, cameraShots=len(docs['cameras']['shots']), cameraBytes=len(camera_bytes),
                          patterns=[dict(p, cameraShots=len(p['cameraShots'])) for p in summaries]), ensure_ascii=False), flush=True)


if __name__ == '__main__':
    main()

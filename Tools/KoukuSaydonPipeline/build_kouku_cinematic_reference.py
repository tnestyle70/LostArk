#!/usr/bin/env python3
"""Index native Matinee keys for the existing boss animation editor.

Only the new reference catalog is written. Actor binding, source keys and weight
curves remain evidence; sequential Append is explicitly an editing operation.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import math
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/ModelAssetConverter'))
from retime_wmodel_from_psa import read_wmodel_animation_sections

CONFIGS = [
    ('gate1.intro', '1관문 연출', 'SCENE03A', 365, 0, None),
    ('gate2.intro', '2관문 시작', 'SCENE04A', 329, 0, None),
    ('gate2.maze', '2관문 카드미로', 'SCENE04A', 328, 0, None),
    ('gate2.clear', '2관문 종료', 'SCENE02A', 63, 0, 16710),
    ('gate3.intro', '3관문 진입', 'SCENE02A', 63, 16710, None),
    ('encore', '앵콜 · 가짜 클리어', 'SCENE07A', 21, 0, None),
    ('bingo.end', '빙고 종료', 'SCENE01B', 32, 0, None),
]

def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))

def build(root=ROOT):
    resource_root = root / 'Client/Bin/Resources'
    references = {p.stem.split('.')[0]: read(p) for p in
                  (root / 'Data/Animation/Reference/KoukuSaydon').glob('*.actionreference.json')}
    models = {}
    groups = []
    sources = {}
    for identity, label, scene, matinee, begin, end in CONFIGS:
        filename = 'LV_LUT_MIDNIGHTC_ED_' + scene + '.json'
        path = next((root / 'out' / folder / filename for folder in
                     ('KoukuSourceSequenceRestore20260912', 'KoukuFireworks20260911')
                     if (root / 'out' / folder / filename).is_file()), None)
        if path is None:
            raise ValueError('Missing native scene cache: ' + filename)
        source = read(path)
        rows = source['rows']
        links = rows[str(matinee)]['p']['variablelinks']
        data_id = next(v['linkedvariables'][0] for v in links if v['linkdesc'] == 'Data')
        data = rows[str(data_id)]['p']
        finish = float(end) if end is not None else data['interplength'] * 1000.0
        sources[scene] = dict(path=path.relative_to(root).as_posix(),
                              sha256=hashlib.sha256(path.read_bytes()).hexdigest())
        for group_id in data['interpgroups']:
            group = rows[str(group_id)]['p']
            animsets = [source['imports'].get(str(i), '') for i in group.get('groupanimsets', [])]
            if not any(s.startswith(('mn_rpct', 'mn_rpcz')) for s in animsets):
                continue
            actor_ids = [rows[str(v)]['p']['objvalue'] for link in links
                         if link['linkdesc'].casefold() == str(group.get('groupname')).casefold()
                         for v in link.get('linkedvariables', [])
                         if rows.get(str(v), {}).get('p', {}).get('objvalue')]
            look = [rows.get(str(a), {}).get('p', {}).get('lookinfokey', '') for a in actor_ids]
            profiles = {s.rsplit('.', 1)[-1] for s in look if s}
            profile = next(iter(profiles)) if len(profiles) == 1 else ''
            reference = references.get(profile)
            model_id = reference['modelAssetId'] + '.wmodel' if reference else ''
            binding_error = '' if reference else 'No unique bound actor with an admitted boss profile.'
            clips = {}
            if model_id:
                if model_id not in models:
                    model_bytes = (resource_root / model_id).read_bytes()
                    models[model_id] = {a['name']: a for a in read_wmodel_animation_sections(model_bytes)}
                clips = models[model_id]
            result = dict(id='cinematic.' + identity + '.group.' + str(group_id),
                          sceneId=identity, displayName=label, sourceScene=scene,
                          matineeExport=matinee, matineeName=rows[str(matinee)]['name'],
                          interpDataExport=data_id, groupExport=group_id,
                          groupName=group.get('groupname', ''), actorExports=actor_ids,
                          actorLookInfo=look, animSets=animsets, profileId=profile,
                          modelAssetId=model_id, runtimeProfileId=Path(model_id).stem if model_id else "", sourceWindowStartMs=begin,
                          sourceWindowEndMs=finish, bindingError=binding_error,
                          appendMode='CHRONOLOGICAL_EDITING_STAGES', tracks=[], keys=[])
            for track_id in group.get('interptracks', []):
                track_row = rows.get(str(track_id), {})
                if track_row.get('cls') != 'interptrackanimcontrol':
                    continue
                track = track_row['p']
                keys = track.get('animseqs', [])
                if not keys:
                    continue
                slot = track.get('slotname', '')
                result['tracks'].append(dict(export=track_id, slot=slot, source=track))
                for ordinal, key in enumerate(keys):
                    key_start = key['starttime'] * 1000.0
                    key_end = keys[ordinal + 1]['starttime'] * 1000.0 if ordinal + 1 < len(keys) else finish
                    start, stop = max(float(begin), key_start), min(finish, key_end)
                    if stop <= start:
                        continue
                    name = key['animseqname']
                    candidates = [n for n in clips if n == name or n.endswith('_' + name)]
                    runtime = candidates[0] if len(candidates) == 1 else ''
                    rate = key.get('animplayrate', 1.0)
                    error = binding_error
                    notice = ''
                    if not runtime and not error:
                        error = 'Missing or ambiguous installed clip: ' + name
                    if key.get('breverse', False):
                        error = 'Reverse source key is reference-only; boss occurrence playback is forward.'
                    if track.get('bdisabletrack', False):
                        error = 'The source animation track is disabled.'
                    source_in = key.get('animstartoffset', 0.0) * 1000.0
                    source_out = 0.0
                    loop = key.get('blooping', False)
                    if runtime:
                        meta = clips[runtime]
                        native = meta['durationTicks'] / meta['ticksPerSecond'] * 1000.0
                        source_out = native - key.get('animendoffset', 0.0) * 1000.0
                        if not math.isfinite(rate) or not .01 <= rate <= 16 or source_out <= source_in:
                            error = 'Invalid native source window or rate.'
                        elif start > key_start:
                            if loop:
                                notice = 'Preview/Append begins at native Source In; the original scene-boundary loop phase is reference-only.'
                            else:
                                source_in += (start - key_start) * rate
                                source_in = min(source_in, max(0.0, source_out - 1.0))
                    digest = hashlib.sha256(json.dumps([scene, matinee, group_id, track_id, key],
                                                       sort_keys=True).encode()).hexdigest()[:20]
                    result['keys'].append(dict(id='cinekey.' + digest, trackExport=track_id,
                        slot=slot, sourceKey=key, nativeStartMs=key_start, nativeEndMs=key_end,
                        localStartMs=start - begin, localEndMs=stop - begin,
                        runtimeClip=runtime, sourceStartMs=round(source_in), sourceEndMs=round(source_out),
                        playMs=max(1, round(stop - start)), playRate=rate,
                        endPolicy='LOOP_TO_WINDOW' if loop else 'HOLD_LAST_POSE', error=error, previewNotice=notice))
            if result['keys']:
                result['keys'].sort(key=lambda k: (k['localStartMs'], k['trackExport'], k['id']))
                groups.append(result)
    return dict(schema='lostark.kouku-cinematic-animation-reference', formatVersion=1,
                authority='REFERENCE_ONLY', sources=sources, groups=groups)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / 'Data/Animation/Reference/KoukuSaydon/KoukuSaydon.cinematicreference.json')
    args = parser.parse_args()
    document = build()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(document, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    keys = [k for g in document['groups'] for k in g['keys']]
    print(json.dumps(dict(groups=len(document['groups']), keys=len(keys), admitted=sum(not k['error'] for k in keys)), ensure_ascii=False))

if __name__ == '__main__':
    main()

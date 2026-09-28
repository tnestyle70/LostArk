"""Attach two source-confirmed one-shot media to Waterpang editor previews.

This is an explicit upgrade of the unedited G01 candidate, never a blanket
replacement of editor changes. Intro preview owns the 18 unchanged stage poses
and a source-timed audio lane. MusicSwitch/BGM and particle actors stay separate.
"""
import argparse
import copy
import hashlib
import io
import json
import wave
from pathlib import Path

import build_maharaka_waterpang_sequences as build


def prepare():
    audio_root = build.OUT/'audio'
    evidence = build.load_json(audio_root/'source-event-evidence.json')
    events = {e['event']: e for e in evidence['events']}
    source_audit = build.load_json(build.AUDIT)['source']
    package = Path(source_audit['physicalPackage'])
    build.require(hashlib.sha256(package.read_bytes()).hexdigest() == source_audit['sha256'], 'Original scene changed')
    rows, imports = build.source.extract_scene(package)

    def event_time(matinee, data, event_name):
        times = [event['time'] for group in build.groups_for(rows, matinee, data)
                 for track in build.source.active_tracks(rows, group) if track['cls'] == 'interptrackakevent'
                 for event in track['p'].get('akevents', [])
                 if imports.get(str(event['event']), '').rsplit('.', 1)[-1] == event_name]
        build.require(len(times) == 1, 'Requires one exact source event time: '+event_name)
        return round(times[0]*1000)
    staged, media = {}, {}
    for name in ('scene_maharakap_waterpangstart', 'scene_maharakap_fallout_foley'):
        event = events[name]
        build.require(len(event['actions']) == 1 and event['actions'][0]['actionType'] == 4
                      and event['actions'][0]['targetType'] == 2 and len(event['mediaIds']) == 1
                      and not event['unresolved'], 'Requires one direct original Sound Play: '+name)
        wav = (audio_root/(name+'.wav')).read_bytes()
        build.require(hashlib.sha256(wav).hexdigest() == event['wavSha256'], 'Decoded audio changed')
        with wave.open(io.BytesIO(wav)) as stream:
            duration = round(stream.getnframes()*1000/stream.getframerate())
            build.require(stream.getsampwidth() == 2 and stream.getnchannels() in (1, 2), 'Unsupported PCM')
        asset = 'Sound/Maharaka/WaterpangSource/'+name+'.wav'
        destination = build.ROOT/'Client/Bin/Resources'/asset
        before = destination.read_bytes() if destination.exists() else None
        build.require(before is None or before == wav, 'Preserve existing resource: '+asset)
        staged[destination] = before, wav
        media[name] = dict(assetId=asset, durationMs=duration, eventId=event['eventId'], mediaId=event['mediaIds'][0])
    paths = {name: build.AUTHORING/(build.AREA+'.'+name+'.json') for name in ('worldsequences', 'camerashots')}
    # Frozen G01 candidate proves ownership; a modified same-ID user row is not migrated.
    originals = {name: build.load_json(build.OUT/'candidate'/path.relative_to(build.ROOT)) for name,path in paths.items()}
    world = copy.deepcopy(originals['worldsequences'])
    cameras = copy.deepcopy(originals['camerashots'])
    collapse_id = 'sequence.'+build.PREFIX+'.collapse'
    collapse = next(t for t in world['templates'] if t['sequenceId'] == collapse_id)
    bindings = next(i['bindings'] for i in world['instances'] if i['templateId'] == collapse_id)
    collapse['soundTracks'] = [dict(soundTrackId=build.PREFIX+'.collapse.foley',
        assetId=media['scene_maharakap_fallout_foley']['assetId'],
        startMs=event_time(42, 157, 'scene_maharakap_fallout_foley'),
        durationMs=media['scene_maharakap_fallout_foley']['durationMs'], volume=1.)]
    for tag, matinee, data in (('intro15', 43, 158), ('intro20', 45, 160)):
        start = event_time(matinee, data, 'scene_maharakap_waterpangstart')
        identity = build.PREFIX+'.'+tag
        cutscene = next(c for c in cameras['cutscenes'] if c['cutsceneId'] == 'cutscene.'+identity)
        duration = cutscene['durationMs']
        # The intro does not move these actors: keep original placed stage poses,
        # with the preview's normal capture/Stop rollback ownership.
        tracks = [dict(slotId=b['slotId'], keys=[dict(timeMs=ms, positionOffset=[0.,0.,0.],
                   rotationQuaternion=[0.,0.,0.,1.], scaleMultiplier=[1.,1.,1.], visible=True)
                   for ms in (0,duration)]) for b in bindings]
        template = dict(sequenceId='sequence.'+identity+'.stage', displayName='워터팡 / 도입 무대 기본 자세·효과음 '+tag,
            category='World', durationMs=duration, interpolation='LINEAR', tracks=tracks, animationTracks=[],
            soundTracks=[dict(soundTrackId=identity+'.foley', assetId=media['scene_maharakap_waterpangstart']['assetId'],
                             startMs=start, durationMs=media['scene_maharakap_waterpangstart']['durationMs'], volume=1.)])
        instance = dict(instanceId='world.sequence.instance.'+identity+'.stage', templateId=template['sequenceId'],
                        enabled=True, startDelayMs=0, playbackSpeed=1, bindings=copy.deepcopy(bindings))
        build.append_owned(world, 'templates', 'sequenceId', [template])
        build.append_owned(world, 'instances', 'instanceId', [instance])
        cutscene['worldInstanceIds'] = [instance['instanceId']]
        cutscene['displayName'] = cutscene['displayName'].replace('(카메라만)', '(카메라·효과음)')
    world['revision'] += 1
    cameras['revision'] += 1
    previous_upgrade = {'worldsequences': copy.deepcopy(world), 'camerashots': copy.deepcopy(cameras)}
    # A missing serialized InterpLength inherits Engine's CDO (5s), not the last
    # movement key (~3.644s). Upgrade only our exact earlier generated candidate.
    collapse_duration = round(build.interp_defaults()['interpLength']*1000)
    collapse_scene = next(c for c in cameras['cutscenes'] if c['cutsceneId'] == 'cutscene.'+build.PREFIX+'.collapse')
    if collapse['durationMs'] != collapse_duration:
        build.require(collapse['durationMs'] < collapse_duration, 'Unexpected collapse window shortening')
        for track in collapse['tracks']:
            last = copy.deepcopy(track['keys'][-1])
            last['timeMs'] = collapse_duration
            track['keys'].append(last)
        collapse['durationMs'] = collapse_duration
        collapse_scene['durationMs'] = collapse_duration
        world['revision'] += 1
        cameras['revision'] += 1
    for name, document in (('worldsequences', world), ('camerashots', cameras)):
        path = paths[name]
        before = path.read_bytes()
        actual = json.loads(before.decode('utf-8-sig'))
        build.require(actual == originals[name] or actual == previous_upgrade[name] or actual == document,
                      'Authoring changed after G01: preserve editor changes and merge explicitly: '+str(path))
        after = before if actual == document else build.encoded(document)
        build.require(len(after) <= (16777216 if name == 'worldsequences' else 2097152), 'Document exceeds runtime cap')
        staged[path] = before, after
    return staged, media


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--apply', action='store_true')
    args = parser.parse_args()
    staged, media = prepare()
    folder = build.OUT/'foley-upgrade'
    for path, (before, after) in staged.items():
        candidate = folder/'candidate'/path.relative_to(build.ROOT)
        candidate.parent.mkdir(parents=True, exist_ok=True)
        candidate.write_bytes(after)
        if before is not None:
            backup = folder/'before'/path.relative_to(build.ROOT)
            backup.parent.mkdir(parents=True, exist_ok=True)
            if not backup.exists():
                backup.write_bytes(before)
    if args.apply:
        build.commit_staged_files(staged)
    print(json.dumps(dict(applied=args.apply, files=sum(a != b for a,b in staged.values()), media=media), ensure_ascii=False))


if __name__ == '__main__':
    main()

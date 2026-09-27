"""Identify Waterpang AkEvent actions/media without flattening music containers.

Writes only out/ evidence. Source MusicSwitch (type13) is NOT missing audio,
and Stop/Resume actions must not become fresh one-shot sounds.
"""
import hashlib
import json
from pathlib import Path
import wwise_audio_package as w

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT/'out/MaharakaWaterpang20260927/audio'
EVENTS = ('scene_maharakap_waterpangstart', 'scene_maharakap_fallout_foley',
          'bgm_eventis_mhp_m05_scene_waterpangstart',
          'bgm_eventis_mhp_m05_scene_waterpangstart_skipend', 'bgm_eventis_mhp_m06_survival')


def require(condition, message):
    if not condition:
        raise ValueError(message)


def fast_decrypt(body, stream_offset=0):
    key = w.keystream()
    start = stream_offset % len(key)
    size = len(body)
    pad = (key[start:] + key*(size//len(key)+2))[:size]
    return (int.from_bytes(body, 'big') ^ int.from_bytes(pad, 'big')).to_bytes(size, 'big')


def main():
    # Exact original banks: scene event media and music event control tree.
    filters = ('SOUND_SCENE_OCEAN1', 'SOUND_SCENE_OCEAN3', 'SOUND_BGM_OCEAN2')
    paths = sorted({p for f in filters for p in w.find_packages(w.DEFAULT_PACKAGE_ROOT, f)})
    probe = bytes(range(256))*1800
    for offset in (0, 1, w.KEYSTREAM_PERIOD-3):
        require(w.decrypt(probe, offset) == fast_decrypt(probe, offset), 'XOR equivalence failed')
    previous = w.decrypt
    try:
        w.decrypt = fast_decrypt
        packages = w.load_packages(paths)
        objects = w.merged_objects(packages)
        report = {'packages': [p.name for p in packages], 'events': []}
        for name in EVENTS:
            entry = objects.get(w.fnv1_32(name))
            require(entry is not None and entry[0] == w.HIRC_EVENT, 'Event not found: '+name)
            actions = []
            for identity in w.event_action_ids(entry[1]):
                require(identity in objects and objects[identity][0] == w.HIRC_ACTION, 'Missing action')
                payload = objects[identity][1]
                kind, target = w.action_fields(payload)
                actions.append(dict(id=identity, actionType=kind, target=target,
                                    targetType=objects.get(target, (-1,))[0], payload=payload.hex()))
            sources, unresolved = w.resolve_event(name, objects)
            record = dict(event=name, eventId=w.fnv1_32(name), actions=actions,
                          mediaIds=sources, unresolved=unresolved)
            if len(sources) == 1:
                matches = [(p, p.stream_by_id(sources[0])) for p in packages]
                payloads = [p.payload(e) for p,e in matches if e is not None]
                require(payloads and all(p == payloads[0] for p in payloads), 'Conflicting/missing source media')
                wem = OUT/(name+'.wem')
                require(wem.read_bytes() == payloads[0], 'Rendered candidate belongs to a different media')
                record['mediaSha256'] = hashlib.sha256(payloads[0]).hexdigest()
                record['wavSha256'] = hashlib.sha256(wem.with_suffix('.wav').read_bytes()).hexdigest()
            report['events'].append(record)
        OUT.mkdir(parents=True, exist_ok=True)
        (OUT/'source-event-evidence.json').write_text(json.dumps(report, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
        print(json.dumps(report, ensure_ascii=False))
    finally:
        w.decrypt = previous


if __name__ == '__main__':
    main()

"""Decode bounded enclosing skeletal actors and nested source material events.

The Action extractor splits at nested AnimEvent classes. The actor transform
follows the last child; CEFAN_Particle carries only its own particle transform.
Supported tail layout is guarded against the original PlaySkeletalMeshActor
reflection contract, including the no-subparts/no-material-look variant.
"""
import base64, collections, math, struct

def fstring(raw, at):
    count = struct.unpack_from('<i', raw, at)[0]
    assert 0 <= count <= 4096 and at + 4 + count <= len(raw)
    data = raw[at + 4:at + 4 + count]
    assert not data or data[-1] == 0
    return data[:-1].decode('utf8') if data else '', at + 4 + count

def material_event(notify):
    kind = notify['sourceType']
    if kind not in ('AnimEvent_MaterialParamterScalar', 'AnimEvent_MaterialParamterLinearColor'):
        return None
    raw = base64.b64decode(notify['serializedPayload']['data'], validate=True)
    signature = ('CEFActionNotify_' + kind + '\0').encode('ascii')
    assert raw.startswith(signature)
    flag, start, duration = struct.unpack_from('<iff', raw, len(signature))
    name, cursor = fstring(raw, len(signature) + 12)
    count = 1 if kind.endswith('Scalar') else 4
    values = list(struct.unpack_from('<' + 'f' * (count * 2), raw, cursor))
    assert flag == 1 and start >= 0 and duration > 0
    assert all(math.isfinite(v) for v in [start, duration, *values])
    return dict(name=name.casefold(), start=start, duration=duration,
                begin=values[:count], end=values[count:], endOffset=cursor + count * 8)

def source_actor(notifies, index):
    notify = notifies[index]
    raw = base64.b64decode(notify['serializedPayload']['data'], validate=True)
    clip = [label for label in notify['serializedLabels'] if label.startswith('SK_') and "'" not in label]
    assert len(clip) == 1
    clip_end = raw.index(clip[0].encode('ascii') + b'\0') + len(clip[0]) + 1
    rate = struct.unpack_from('<f', raw, clip_end)[0]
    assert math.isfinite(rate) and 0 < rate <= 16
    children = []
    for child in notifies[index + 1:]:
        if not child['sourceType'].startswith('AnimEvent_'): break
        children.append(child)
    assert children
    # Action extraction splits at each child class. The enclosing actor's tail
    # therefore follows the last material event, not its CEFAN_Particle block.
    last = children[-1]; event = material_event(last); assert event
    tail = base64.b64decode(last['serializedPayload']['data'], validate=True)
    at = event['endOffset']
    assert struct.unpack_from('<4i', tail, at) == (1, 0, 0, 1), notify['notifyId']
    location = list(struct.unpack_from('<3f', tail, at + 16))
    rotator = list(struct.unpack_from('<3i', tail, at + 28))
    scale = list(struct.unpack_from('<3f', tail, at + 40))
    # EFGame.PlaySkeletalMeshActor reflection: bApplyLocalRotation,
    # RelativeLocation/RelativeRotation/RelativeScale, SubParts, guarantee flag.
    assert struct.unpack_from('<2i', tail, at + 52) == (0, 0)
    attach, end = fstring(tail, at + 64)
    detach = struct.unpack_from('<f', tail, end)[0]
    assert all(math.isfinite(v) for v in location + scale + [detach]) and min(scale) > 0
    return dict(clip=clip[0].casefold(), playRate=rate,
                sourcePositionUeCm=location, sourceRotator=rotator, sourceScale=scale,
                attachName=attach, detachSeconds=detach, tailNotify=last['notifyId'], tailOffset=at,
                localTransform=dict(position=[location[1] * .01, location[2] * .01, location[0] * .01],
                    rotationDegrees=[rotator[0] * 360 / 65536, rotator[1] * 360 / 65536, -rotator[2] * 360 / 65536],
                    scale=[scale[0], scale[2], scale[1]], revolutionDegreesPerSecond=[0,0,0], velocityPerSecond=[0,0,0]))

def material_tracks(notifies, index):
    owner = notifies[index]
    tag = owner['serializedLabels'][0]
    parent = None
    segments = collections.defaultdict(list)
    for current, notify in enumerate(notifies):
        kind = notify['sourceType']
        if not kind.startswith('AnimEvent_'):
            parent = notify if (current == index or kind == 'PlaySkeletalMeshMaterialParam' and tag in notify['serializedLabels']) else None
            continue
        if parent is None: continue
        event = material_event(notify)
        if event is None: continue
        start = parent['localTimeSeconds'] - owner['localTimeSeconds'] + event['start']
        assert start >= 0
        segments[event['name']].append((start, start + event['duration'], event['begin'], event['end']))
    tracks = []
    for name, rows in sorted(segments.items()):
        keys = []
        for start, end, begin, finish in sorted(rows):
            assert not keys or start >= keys[-1]['timeSeconds']
            for t, value in [(start, begin), (end, finish)]:
                key = dict(timeSeconds=t, value=value, arriveTangent=[0.] * len(value), leaveTangent=[0.] * len(value), interpolation='linear')
                if keys and keys[-1]['timeSeconds'] == t:
                    assert keys[-1]['value'] == value
                else: keys.append(key)
        tracks.append(dict(name=name, kind='COLOR' if len(keys[0]['value']) == 4 else 'SCALAR', keys=keys))
    return tracks

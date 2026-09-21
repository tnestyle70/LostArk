"""Project EFActionNotify_TrailGhostEffect into the existing live pose carrier.

Field order is recovered from EFGame's reflected class/enum. Timing, part/local
flags, source colors and diffuse intensity are preserved. The rim/fade shader
remains PROJECT_AUTHORED; this is not a claim of the native material ABI.
"""
from __future__ import annotations
import base64
import hashlib
import math
import re
import struct
from collections.abc import Mapping
from typing import Any


def decode_trail_ghost(row: Mapping[str, Any]) -> dict[str, Any]:
    payload = row['serializedPayload']
    raw = base64.b64decode(payload['data'], validate=True)
    if (len(raw) != payload['byteSize'] or hashlib.sha256(raw).hexdigest() != payload['sha256'] or
            not raw.startswith(b'CEFActionNotify_TrailGhostEffect\0')):
        raise ValueError('TrailGhost source header/receipt mismatch')
    cursor = 73  # Common CEFActionNotify fixed header, followed by two counted strings.
    def string() -> str:
        nonlocal cursor
        count, = struct.unpack_from('<I', raw, cursor); cursor += 4
        if count > 4096 or cursor+count > len(raw): raise ValueError('TrailGhost string bounds')
        value = raw[cursor:cursor+count]; cursor += count
        if count and value[-1] != 0: raise ValueError('TrailGhost string terminator')
        return value.rstrip(b'\0').decode('utf8', 'strict')
    label, category = string(), string()
    priority, = struct.unpack_from('<I', raw, cursor); cursor += 4
    start = cursor
    fields = struct.unpack_from('<4I3fI3f16I4f', raw, cursor); cursor += struct.calcsize('<4I3fI3f16I4f')
    if any(v not in (0, 1) for v in fields[:4]) or fields[7] > 2:
        raise ValueError('TrailGhost flag/enum domain')
    numbers = (*fields[4:7], *fields[8:11], *fields[27:31])
    if any(not math.isfinite(v) for v in numbers): raise ValueError('TrailGhost nonfinite input')
    if any(not 0 <= v <= 255 for v in fields[11:27]): raise ValueError('TrailGhost RGBA domain')
    pivot = string()
    material, replacement, ordinal = struct.unpack_from('<III', raw, cursor); cursor += 12
    if cursor != len(raw): raise ValueError('TrailGhost trailing ABI mismatch')
    if material or replacement or pivot.lower() not in ('none', ''):
        raise ValueError('TrailGhost custom material/pivot needs an explicit source resolver')
    result = dict(forceRemovePrevious=bool(fields[0]), stopWhenNotifyEnd=bool(fields[1]),
        forceChildAllRemove=bool(fields[2]), onlyLocalPlayer=bool(fields[3]),
        durationSeconds=fields[4], sampleIntervalSeconds=fields[5], sampleLifetimeSeconds=fields[6],
        sourcePartType=fields[7], initialAlpha=fields[8], initialAlphaDuration=fields[9],
        sourceColorIntensity=fields[10], ambientStart=list(fields[11:15]), ambientEnd=list(fields[15:19]),
        rimStart=list(fields[19:23]), rimEnd=list(fields[23:27]),
        scaleStart=fields[27], scaleEnd=fields[28], viewOffsetStart=fields[29], viewOffsetEnd=fields[30],
        pivotBone=pivot, materialReference=material, replaceParameterReference=replacement,
        fieldByteOffset=start, ordinal=ordinal, label=label, category=category, priority=priority,
        sourceSha256=payload['sha256'])
    if not (0 < fields[4] <= 30 and .005 <= fields[5] <= 2 and .005 <= fields[6] <= 2 and 0 <= fields[8] <= 1):
        raise ValueError('TrailGhost time/alpha bounds')
    return result


def project_trail_ghost_rows(rows: list[dict[str, Any]], clip_durations: Mapping[str, float],
        model_asset_id: str = 'Character/GuardianKnight/GuardianKnight.wmodel') -> tuple[dict[tuple[int, int], list[dict]], list[dict]]:
    by_stage: dict[tuple[int, int], list[dict]] = {}; receipts = []
    for row in rows:
        if row.get('sourceType') != 'TrailGhostEffect': continue
        decoded = decode_trail_ghost(row)
        # These values are all identity/zero in this source set. Do not silently
        # approximate a future nonidentity source or destructive cross-cue flag.
        if (decoded['forceRemovePrevious'] or decoded['forceChildAllRemove'] or
                decoded['initialAlphaDuration'] != 0 or decoded['scaleStart'] != 1 or decoded['scaleEnd'] != 1 or
                decoded['viewOffsetStart'] != 0 or decoded['viewOffsetEnd'] != 0 or
                any(decoded[c][i] for c in ('ambientStart', 'ambientEnd') for i in range(3))):
            raise ValueError('TrailGhost requires additional source semantics: '+row['notifyId'])
        match = re.fullmatch(r'action-(\d+)/stage-(\d+)/notify-(\d+)', row['notifyId'])
        if not match or int(match[1]) != row['skillId']: raise ValueError('TrailGhost stable source identity')
        clip = row['clipName']; clip_duration = clip_durations[clip]
        start = float(row['localTimeSeconds']); end = start + decoded['durationSeconds']
        if not math.isfinite(start) or start < 0: raise ValueError('TrailGhost notify clock')
        if decoded['stopWhenNotifyEnd']:
            notify_end = float(row.get('sourceEndSeconds', 0))
            end = min(end, notify_end if notify_end > start else clip_duration)
        if end <= start: raise ValueError('TrailGhost empty emission range')
        cue = dict(cueId=row['notifyId'].replace('/', '.')+'.live-afterimage', modelAssetId=model_asset_id,
            clipName=clip, startDelaySeconds=0, durationSeconds=end, clipPlayRate=1,
            alphaMode='TRANSLUCENT', opacity=1,
            colorMultiply=[*(v/255 for v in decoded['rimStart'][:3]), decoded['initialAlpha']],
            holdLastFrame=True, loop=False, visible=True,
            localTransform=dict(position=[0,0,0], rotationDegrees=[0,0,0], scale=[1,1,1],
                velocityPerSecond=[0,0,0], revolutionDegreesPerSecond=[0,0,0]),
            assetPreTransform=dict(scale=[.0001]*3, rotationDegrees=[0,0,0]),
            afterimage=dict(emissionStartSeconds=start, emissionEndSeconds=end,
                sampleIntervalSeconds=decoded['sampleIntervalSeconds'], sampleLifetimeSeconds=decoded['sampleLifetimeSeconds'],
                maxSamples=min(64, math.ceil(decoded['sampleLifetimeSeconds']/decoded['sampleIntervalSeconds'])+1),
                appearanceBasis='PROJECT_AUTHORED', liveOwnerPose=True, onlyLocalPlayer=decoded['onlyLocalPlayer'],
                captureInitialPose=True, sourcePartType=decoded['sourcePartType'],
                sourceColorIntensity=decoded['sourceColorIntensity'],
                endColor=[*(v/255 for v in decoded['rimEnd'][:3]), decoded['initialAlpha']]))
        key=(int(match[1]), int(match[2])); by_stage.setdefault(key,[]).append(cue)
        receipts.append(dict(notifyId=row['notifyId'], decoded=decoded, cueId=cue['cueId'],
            consumer='CCharacter current visible parts -> CSkeletalAfterimage -> existing model shaders',
            sourceEnum={'0':'EFTG_NONE','1':'EFTG_WP','2':'EFTG_ALL'},
            projectedPartPolicy='0=current base outfit; 1=socketed weapon; 2=all; NONE-to-outfit is project interpretation',
            appearanceBasis='PROJECT_AUTHORED: source colors/intensity are consumed; original rim exponent/fade material ABI is unrecovered'))
    return by_stage,receipts

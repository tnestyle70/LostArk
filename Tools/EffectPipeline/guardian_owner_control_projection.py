"""Decode original Guardian visibility/camera controls, retaining source receipts.

Runtime visibility is a PROJECT_ADAPTER overlay over approved stance and user
equipment visibility. This module never changes combat or the network stance.
"""
from __future__ import annotations
import base64, hashlib, math, struct

def decode_control(notify):
    kind=notify['sourceType'];payload=notify['serializedPayload']
    if kind not in ('IdentityParts','HidePawn','UltimateSkillCameraControl'):raise ValueError('Unsupported owner control')
    raw=base64.b64decode(payload['data'],validate=True);signature=('CEFActionNotify_'+kind+'\0').encode()
    if len(raw)!=payload['byteSize'] or hashlib.sha256(raw).hexdigest()!=payload['sha256'] or not raw.startswith(signature):raise ValueError('Owner control source receipt mismatch')
    cursor=len(signature)+40
    def uint():
        nonlocal cursor
        value,=struct.unpack_from('<I',raw,cursor);cursor+=4;return value
    def scalar():
        nonlocal cursor
        value,=struct.unpack_from('<f',raw,cursor);cursor+=4
        if not math.isfinite(value):raise ValueError('Nonfinite control scalar')
        return value
    def flag():
        value=uint()
        if value not in (0,1):raise ValueError('Invalid control bool')
        return bool(value)
    def string():
        nonlocal cursor
        count=uint()
        if count>4096 or cursor+count>len(raw):raise ValueError('Control string bounds')
        value=raw[cursor:cursor+count];cursor+=count
        if count and value[-1]!=0:raise ValueError('Control string terminator')
        return value.rstrip(b'\0').decode('utf8','strict')
    label,category=string(),string();priority=uint();begin=cursor
    enabled=struct.unpack_from('<I',raw,len(signature)+12)[0]
    if enabled not in (0,1):raise ValueError('Invalid common notify enabled flag')
    result={'kind':kind,'fieldByteOffset':begin,'label':label,'category':category,'priority':priority,
            'sourceSha256':payload['sha256'],'enabled':bool(enabled)}
    if kind=='IdentityParts':
        result.update(makeParts=flag(),failCompleteCancel=flag(),executeNotifyEnd=flag())
        # This retail action stream serializes three flags, then the next
        # notify type-string length. Reflected changeStance is not serialized.
    elif kind=='HidePawn':
        names=('hidePawn','weaponTypeIgnoresGadget','hideBaseMeshWithFX','executeNotifyEnd',
               'hideStatusEffectFX','hideHeadStatusUI','excludeHideWeapon','hidePawnByRideVehicle')
        result.update((name,flag()) for name in names);count=uint()
        if count>32:raise ValueError('HidePawn part count')
        result['parts']=[{'type':uint(),'subtype':struct.unpack('<i',struct.pack('<I',uint()))[0]} for _ in range(count)]
    else:
        result.update(controlType=uint(),eventName=string(),jumpTime=scalar(),npcId=uint(),hideIdentityBuffParticle=flag())
        count=uint()
        if count>64:raise ValueError('Camera allowed status count')
        result['allowedStatusEffectIds']=[uint() for _ in range(count)]
        result['totalDamageShowDelayTime']=scalar()
    result['typedBlockEnd']=cursor;result['unconsumedBytes']=len(raw)-cursor
    # extract_action_effect_notifies includes the next notify-name length;
    # a final notify can additionally contain the source stage footer. Never
    # interpret those motion samples as HidePawn fields.
    if cursor>len(raw)-4:raise ValueError('Control source boundary mismatch')
    return result

def project_visibility_controls(rows):
    by_stage={};receipt=[]
    for row in rows:
        notify=row['notify'];kind=notify['sourceType']
        if kind not in ('IdentityParts','HidePawn'):continue
        value=decode_control(notify)
        if not value['enabled']:continue
        if kind=='IdentityParts':
            if value['makeParts'] or not value['failCompleteCancel'] or not value['executeNotifyEnd']:raise ValueError('New identity source policy requires review')
            target=12;runtime_kind='IDENTITY_VISIBILITY';parameter='identity'
        else:
            if not value['hidePawn'] or any(value[n] for n in ('weaponTypeIgnoresGadget','hideBaseMeshWithFX','hideHeadStatusUI','excludeHideWeapon','hidePawnByRideVehicle')):raise ValueError('New pawn visibility policy requires review')
            if value['parts'] not in ([],[{'type':9,'subtype':-1}]):raise ValueError('Unmapped source visibility parts')
            target=9 if value['parts'] else 0;runtime_kind='PAWN_VISIBILITY';parameter='visibility'
        # EurosLoof destroys its temporary identity parts at notify end. The
        # interval therefore owns the existing Wing presentation; it does not
        # switch gameplay stance. Other source policies retain their mapping.
        temporary_parts=(kind=='IdentityParts' and row['skillId']==49260 and
                         row['sourceStageIndex'] in (0,1,2))
        visible=1 if temporary_parts else 0
        start=float(notify['localTimeSeconds']);duration=float(notify['durationSeconds'])
        if not math.isfinite(start) or not math.isfinite(duration) or start<0 or not 0<duration<=60:raise ValueError('Visibility clock bounds')
        control=dict(controlId=notify['notifyId'].replace('/','.')+'.visibility',kind=runtime_kind,parameter=parameter,
            mappingBasis='PROJECT_ADAPTER',sourceTargetType=target,onlyLocalPlayer=False,startSeconds=start,
            keys=[dict(seconds=0,value=[visible,0,0,0]),dict(seconds=duration,value=[visible,0,0,0])])
        if temporary_parts:control['sourceValues']=[int(value[k]) for k in ('makeParts','failCompleteCancel','executeNotifyEnd')]
        by_stage.setdefault((row['skillId'],row['sourceStageIndex']),[]).append(control)
        receipt.append(dict(notifyId=notify['notifyId'],decoded=value,control=control,
            mapping=('Temporary identity visibility until source end-removal; removal restores current approved stance and authored/user visibility' if temporary_parts else
                     'Temporary presentation suppression; removal restores current approved stance and authored/user visibility'),
            statusFxBoundary='No independently rendered status/buff FX owner exists in the current Guardian runtime; source flag retained, skill FX are not suppressed' if value.get('hideStatusEffectFX') else 'not requested'))
    return by_stage,receipt

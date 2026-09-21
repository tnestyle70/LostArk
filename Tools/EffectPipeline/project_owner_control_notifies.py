"""Project decoded PawnMaterial/Directional notify data to explicit owner adapters.
Raw flags/values stay in the emitted row; retail scheduling is not claimed.
"""
from __future__ import annotations
import base64, hashlib, json, math, struct
from collections import defaultdict

def decode_owner_control_rows(rows):
    """Decode the selected original CEF wire blocks with receipt/bounds checks.

    The common +12 boolean follows the existing PlayParticleEffect parser's
    enabled contract. Unknown header fields remain in the evidence unchanged.
    """
    decoded=[]
    for row in rows:
        notify=row['notify'];kind=notify['sourceType']
        if kind not in ('PawnMaterialParam','DominantDirectionalLight_Control'):continue
        receipt=notify['serializedPayload'];raw=base64.b64decode(receipt['data'],validate=True)
        signature=('CEFActionNotify_'+kind+'\0').encode()
        if not raw.startswith(signature) or len(raw)!=receipt['byteSize'] or hashlib.sha256(raw).hexdigest()!=receipt['sha256']:
            raise ValueError('Owner control payload receipt mismatch')
        cursor=len(signature)+40
        def uint():
            nonlocal cursor
            value,=struct.unpack_from('<I',raw,cursor);cursor+=4;return value
        def scalar():
            nonlocal cursor
            value,=struct.unpack_from('<f',raw,cursor);cursor+=4
            if not math.isfinite(value):raise ValueError('Nonfinite source control')
            return value
        def string():
            nonlocal cursor
            count=uint()
            if count>4096 or cursor+count>len(raw):raise ValueError('Owner control string bounds')
            value=raw[cursor:cursor+count];cursor+=count
            if count and value[-1]!=0:raise ValueError('Owner control string terminator')
            return value.rstrip(b'\0').decode('utf8','strict')
        header=list(struct.unpack_from('<4I',raw,len(signature)))
        if header[3] not in (0,1):raise ValueError('Owner control enabled flag is not boolean')
        label,category,priority=string(),string(),uint()
        if kind=='DominantDirectionalLight_Control':
            source=dict(flags=[uint() for _ in range(5)],values=[scalar() for _ in range(4)],color=[uint() for _ in range(4)])
            if any(v not in (0,1) for v in source['flags']) or any(v>255 for v in source['color']):raise ValueError('Directional source range')
        else:
            def curve(channels):
                count=uint()
                if count>64:raise ValueError('Source curve bounds')
                keys=[]
                for _ in range(count):
                    key=dict(seconds=scalar(),value=[scalar() for _ in range(channels)],arrive=[scalar() for _ in range(channels)],leave=[scalar() for _ in range(channels)],interp=uint())
                    if key['interp']!=0:raise ValueError('Unsupported nonlinear source curve')
                    keys.append(key)
                method=uint()
                if method not in range(6):raise ValueError('Source curve method')
                return keys
            def unit():
                merge,target,count=uint(),uint(),uint()
                if count>64:raise ValueError('Source mesh index bounds')
                indices=[uint() for _ in range(count)];name=string();life=scalar();random=scalar()
                return dict(merge=merge,target=target,indices=indices,name=name,life=life,randomStart=random,curves=[curve(4 if k<4 else 1) for k in range(8)])
            source=dict(actionType=uint(),default=unit());count=uint()
            if count>64:raise ValueError('Source look override bounds')
            source['overrides']=[dict(look=string(),unit=unit()) for _ in range(count)]
        if len(raw)-cursor!=4:raise ValueError('Owner control source boundary mismatch')
        decoded.append(dict(skillId=row['skillId'],stage=row['sourceStageIndex'],bound=row['bound'],notify=notify['notifyId'],type=kind,
            start=notify['localTimeSeconds'],duration=notify['durationSeconds'],decoded=source,enabled=bool(header[3]),
            commonHeader=header,category=category,label=label,priority=priority,sourceSha256=receipt['sha256']))
    return decoded

def project_decoded_owner_controls(rows):
    stages=defaultdict(list); receipts=[]
    for row in rows:
        if not row.get('enabled',True):
            receipts.append(dict(notifyId=row['notify'],status='SOURCE_DISABLED',sourceSha256=row.get('sourceSha256'),commonHeader=row.get('commonHeader')));continue
        source=row["decoded"]; identity=row["notify"].replace("/", ".")
        target=stages[(row["skillId"],row["stage"])]
        def cue(kind, parameter, recipient, local, keys, raw_values):
            result=dict(controlId=identity+"."+kind.lower(),kind=kind,parameter=parameter,
                mappingBasis="PROJECT_ADAPTER",sourceTargetType=recipient,onlyLocalPlayer=local,
                startSeconds=row["start"],keys=keys,sourceValues=raw_values)
            assert len(keys)>=2 and keys[0]["seconds"]==0
            assert all(b["seconds"]>a["seconds"] for a,b in zip(keys,keys[1:]))
            target.append(result)
        if row["type"]=="PawnMaterialParam":
            unit=source["default"]
            assert source["actionType"]==1 and not source["overrides"]
            if not unit["name"] and not any(unit["curves"]):
                receipts.append(dict(notifyId=row["notify"],status="SOURCE_EMPTY_NO_OP"));continue
            assert unit["merge"]==1 and unit["target"] in (1,4) and not unit["indices"] and unit["randomStart"]==0
            assert not any(unit["curves"][2:])
            start,end=unit["curves"][:2]; assert start and end
            hold_end=start[-1]["seconds"]+unit["life"]
            keys=[dict(seconds=k["seconds"],value=k["value"]) for k in start]
            keys.extend(dict(seconds=hold_end+k["seconds"],value=k["value"]) for k in end)
            # Exact equal-time endpoints are a step at the start of the next phase.
            keys=list({k["seconds"]:k for k in keys}.values())
            cue("MATERIAL_VECTOR",unit["name"].lower(),unit["target"],False,keys,
                row.get('commonHeader',[])+[source["actionType"],unit["merge"],unit["target"],unit["life"],unit["randomStart"]])
        else:
            flags=source["flags"];fade,hold,release,brightness=source["values"]
            assert flags[0]==1 and flags[1]==0 and fade>=0 and hold>=0 and release>=0
            raw=row.get('commonHeader',[])+list(flags)+list(source["values"])+list(source["color"])
            def envelope(value, origin):
                points=[(0,origin),(fade,value),(fade+hold,value),(fade+hold+release,origin)]
                return [dict(seconds=t,value=v) for t,v in dict(points).items()]
            if flags[3]:
                # Explicit project adapter: the signed authored adjustment is a
                # delta from the current scene's unit brightness baseline.
                value=1+brightness; assert 0<=value<=16
                cue("DIRECTIONAL_BRIGHTNESS","",4,True,envelope([value,0,0,0],[1,0,0,0]),raw)
            if flags[4]:
                color=[x/255 for x in source["color"][:3]]+[1]
                cue("DIRECTIONAL_COLOR","",4,True,envelope(color,[0,0,0,0]),raw)
        receipts.append(dict(notifyId=row["notify"],status="PROJECT_ADAPTER",controlIds=[c["controlId"] for c in target if c["controlId"].startswith(identity)]))
    return [dict(skillId=s,sourceStageIndex=i,ownerControls=c) for (s,i),c in stages.items() if c],receipts

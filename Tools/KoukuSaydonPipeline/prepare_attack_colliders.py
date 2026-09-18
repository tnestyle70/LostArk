"""Prepare source-backed attack candidates without replacing an open authoring draft.

Use prepare(document, source_root) to merge into another staged candidate. Source
Effect damage is converted only when its caster origin and primitive are explicit;
projectile graphs and special control notifies remain an inventory, never bounds.
"""
from __future__ import annotations
import argparse, base64, collections, copy, hashlib, json, math, re, sqlite3, struct
from pathlib import Path
try:
    from .combat_hit_templates import validate_hits
    from . import project_kouku_saydon_composition as projection
except ImportError:
    from combat_hit_templates import validate_hits
    import project_kouku_saydon_composition as projection

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_SOURCE = Path('C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829')
SOURCE_PROFILES = ('MN_RPCT_05','MN_RPCT_06','MN_RPCT_07','MN_RPCZ_00')
TAG = '[source attack] '

def read(path): return json.loads(Path(path).read_text(encoding='utf-8-sig'))
def digest(path): return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def write(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data,ensure_ascii=False,indent=2,allow_nan=False)+'\n',encoding='utf-8')

def source_hits(source_root):
    database=source_root/'WorldObjectExtraction-20260907/EFTable_SkillEffect.db'
    con=sqlite3.connect(database.resolve().as_uri()+'?mode=ro',uri=True);con.row_factory=sqlite3.Row
    rows=collections.defaultdict(list)
    for row in con.execute('select * from SkillEffect'): rows[int(row['PrimaryKey'])].append(dict(row))
    con.close();result={};inputs=[dict(path=str(database),sha256=digest(database))]
    for profile in SOURCE_PROFILES:
        path=source_root/'RemainingCharacterExtraction-20260829/ActionNameSources'/f'{profile}.action-effects.json'
        document=read(path);inputs.append(dict(path=str(path),sha256=digest(path)))
        for action in document['actions']:
            for stage in action['stages']:
                hits=[]
                for notify in stage.get('notifies',[]):
                    if notify.get('sourceType')!='Effect':continue
                    raw=base64.b64decode(notify['serializedPayload']['data'])
                    if not raw.startswith(b'CEFActionNotify_Effect\0') or len(raw)<71:continue
                    # Qualified common Notify header: enabled uint32 follows time fields.
                    if struct.unpack_from('<I',raw,35)[0]!=1:continue
                    length=struct.unpack_from('<i',raw,67)[0];width=-length*2 if length<0 else length
                    field=71+width+12
                    if not 0<width<=4096 or field+4>len(raw):continue
                    effect=struct.unpack_from('<I',raw,field)[0]
                    for row in rows.get(effect,[]):
                        if row['Key'] not in (1,12):continue
                        hits.append(dict(timeMs=round(notify['localTimeSeconds']*1000),notifyId=notify['notifyId'],
                            sourceEffectId=effect,row=row,payloadSha256=notify['serializedPayload']['sha256'],
                            notifyYawDegrees=struct.unpack_from('<i',raw,field+4)[0],notifyPositionCm=list(struct.unpack_from('<3f',raw,field+40))))
                result[profile,action['actionId'],f"stage-{stage['stageIndex']:03}"]=hits
    return result,rows,inputs

def source_shape(row):
    if row['Key']!=1:return None,'PROJECTILE_CALLBACK_REQUIRES_AUTHORITATIVE_INSTANCE'
    if row['AreaOrigin']!=0:return None,'TARGET_OR_WORLD_ORIGIN_REQUIRES_EXACT_ANCHOR'
    if row.get('AreaOffsetAngleRandom',0) or row.get('AreaOriginOption',0):return None,'CONDITIONAL_OR_RANDOM_SOURCE_ANCHOR'
    radius=row['AreaRange']*.01;inner=row['AreaRemoveRange']*.01;angle=row['AreaAngle']
    if not 0<=inner<radius:return None,'INVALID_SOURCE_INNER_RADIUS'
    if not 0<radius<=100:return None,'UNQUALIFIED_SOURCE_RANGE'
    if inner>0 and row['AreaType'] not in (1,3):return None,'UNSUPPORTED_SOURCE_INNER_PRIMITIVE'
    if row['AreaType']==1 or (row['AreaType']==3 and angle==360):shape='CIRCLE'
    elif row['AreaType']==2 and angle>0:shape='BOX'
    elif row['AreaType']==3 and 0<angle<360:shape='SECTOR'
    else:return None,'UNSUPPORTED_SOURCE_PRIMITIVE'
    return dict(shape=shape,halfExtents=[max(.001,angle*.005) if shape=='BOX' else 1, max(.001,row['AreaHeight']*.005),radius*.5 if shape=='BOX' else 1],
                radiusM=radius,innerRadiusM=inner,halfAngleDegrees=angle*.5 if shape=='SECTOR' else 45),''

def prepare(document, source_root=DEFAULT_SOURCE, *, damage_percent=10, facing_yaw_degrees=None, include_normal=True):
    """Return a detached candidate and exact additions/exclusions receipt; input untouched."""
    if not 1<=damage_percent<=100 or (facing_yaw_degrees is not None and not math.isfinite(facing_yaw_degrees)):raise ValueError('Invalid explicit tuning')
    out=copy.deepcopy(document); source,skill_rows,inputs=source_hits(Path(source_root)); receipt=dict(
        sourceInputs=inputs,sourceRevision=document['revision'],changes=[],excluded=[],
        tuning=dict(normalDamage=dict(basis='PROJECT_TUNED',maxHpPercent=damage_percent),
                    directHitWindowMs=dict(basis='PROJECT_TUNED_34MS_FIXED_TICK_WINDOW',value=34),
                    napalmRepeatIntervalMs=dict(basis='PROJECT_TUNED_NOT_DECODED_SOURCE_TIMER',value=1000),
                    casterFacingYawDegrees=dict(basis='INSTALLED_WMODEL_EYE_MOUTH_COMBINED_PLUS_X_SCALE_ONLY_ADMISSION',override=facing_yaw_degrees,profileDefaults={'MN_RPCT_05':90,'MN_RPCT_06':90,'MN_RPCT_07':90,'MN_RPCZ_00':90}),
                    trackingRadiusM=dict(basis='PROJECT_TUNED_CONSERVATIVE_CENTRAL_TARGET_DISC',value=.5),
                    trackingBirthAnchor='AUTHORITATIVE_BOSS_SPAWN_POSITION',trackingActivationMs=0))
    resources={r['resourceId']:r for r in out['presentationResources']}; definitions={l['logicId']:l for l in out['logics']}
    by_asset=lambda row:resources[row['resourceId']].get('assetId','')
    used_logic_ids={b['logicId'] for p in out['patterns'] for b in p.get('logicOccurrences',[]) if b.get('enabled',True)}
    for logic in out['logics']:
        if logic['logicId'] in used_logic_ids and logic.get('triggerKind')=='ALBION_BLUE_CIRCLE' and not logic.get('fixedHits'):
            logic['fixedHits']=validate_hits([dict(hitId='skill.421990318.impact',atMs=2000,radiusM=1.6,damagePercent=damage_percent)],logic['effectLifetimeMs'])
            receipt['changes'].append(dict(logicId=logic['logicId'],field='fixedHits',basis='ORIGINAL_421990319_WARNING_2S_RADIUS_1P6_ON_EXISTING_AUTHORITATIVE_INSTANCE'))
        if logic.get('judgementKind')=='PURSUIT_PROJECTILES' and not logic.get('projectileHits'):
            life=logic.get('lifetimeMs',0) or 600000
            logic['projectileHits']=validate_hits([dict(hitId='projectile.contact',trigger='CONTACT',endMs=life,
                radiusM=logic['contactRadiusM'],damagePercent=damage_percent)],life)
            receipt['changes'].append(dict(logicId=logic['logicId'],field='projectileHits',basis='EXISTING_AUTHORITATIVE_CONTACT_RADIUS_AND_INDIVIDUAL_COMBAT_OBJECT'))
    def visual_hits(members):
        if not members:return []
        start=min(r['startMs'] for r in members)
        maps=[r for r in members if r.get('anchorKind','BOSS')=='MAP']
        if not maps:return []
        anchor=min(maps,key=lambda r:(r['startMs'],r['occurrenceId']))['positionOffset']
        hits=[];end=max(r['startMs']+r['durationMs'] for r in members)-start
        for index,row in enumerate(members):
            asset=by_asset(row);eid=421991204 if asset.endswith('.ball.impact') else 421991206 if asset.endswith('.napalm') else 0
            if not eid:continue
            source_row=next(r for r in skill_rows[eid] if r['Key'] in (1,2))
            scale=row.get('scale',[1,1,1]); radius=source_row['AreaRange']*.01*max(scale[0],scale[2]);at=row['startMs']-start
            hit=dict(hitId=f'skill.{eid}.{index}',radiusM=radius,atMs=at,offsetRightM=row['positionOffset'][0]-anchor[0],
                offsetForwardM=row['positionOffset'][2]-anchor[2],damagePercent=damage_percent)
            if eid==421991206:hit.update(trigger='CONTACT',endMs=at+row['durationMs'],repeatCount=max(1,min(64,math.ceil(row['durationMs']/1000))),repeatIntervalMs=1000)
            hits.append(hit)
        return validate_hits(hits,end)
    for pattern in out['patterns']:
        presentations=pattern.get('presentationOccurrences',[]);by_id={r['occurrenceId']:r for r in presentations}
        for box in pattern.get('logicOccurrences',[]):
            logic=definitions[box['logicId']]
            if logic.get('judgementKind')!='SHOWTIME_PLAYER_TARGETS':continue
            for field,members in [('fixedHits',[r for r in presentations if r.get('selectionGroupId')==logic.get('fixedSelectionGroupId')])]:
                if logic.get('fixedSelectionGroupId') and not logic.get(field):
                    hits=visual_hits(members)
                    if hits:logic[field]=hits;receipt['changes'].append(dict(logicId=logic['logicId'],field=field,basis='ORIGINAL_421991204_421991206_GEOMETRY_AUTHORED_IMPACT_TIMES',hits=hits))
            if logic.get('trackingPresentationOccurrenceId') and not logic.get('trackingHits'):
                logic['trackingHits']=validate_hits([dict(hitId='tracking.instant-death',trigger='CONTACT',endMs=box['durationMs'],radiusM=.5,damageKind='INSTANT_DEATH',damagePercent=0)],box['durationMs'])
                receipt['changes'].append(dict(logicId=logic['logicId'],field='trackingHits',basis='USER_REQUEST_INSTANT_DEATH_FROM_CENTRAL_BIRTH',hits=logic['trackingHits']))
            if logic.get('randomVolleyOccurrenceSets') and not logic.get('randomVolleyHits'):
                logic['randomVolleyHits']=[visual_hits([by_id[i] for i in ids]) for ids in logic['randomVolleyOccurrenceSets']]
                receipt['changes'].append(dict(logicId=logic['logicId'],field='randomVolleyHits',basis='SAME_ATTACK_TEMPLATES_ON_EVERY_AUTHORITATIVE_RANDOM_INSTANCE'))
    if include_normal:
        existing_names={r['displayName'] for r in out['presentationResources']}
        def allocate_logic(values):
            ordinal=out.get('nextLogicOrdinal',1);out['nextLogicOrdinal']=ordinal+1
            item=dict(logicId=f'kakulsaydon.g1.logic.{ordinal}',**values);out['logics'].append(item);definitions[item['logicId']]=item;return item['logicId']
        trigger_id=result_id=None
        for pattern in out['patterns']:
            facing = facing_yaw_degrees if facing_yaw_degrees is not None else 90.0
            elapsed=0;events={};duration=pattern.get('durationMs') or sum(s['durationMs'] for s in pattern['stages'])
            for stage in pattern['stages']:
                for anim in stage.get('animationOccurrences',[]):
                    key=anim.get('profileId'),anim.get('sourceActionId'),anim.get('sourceStageId')
                    for item in source.get(key,[]):
                        rate=anim.get('playRate',1);local=(item['timeMs']-anim.get('sourceStartMs',0))/rate
                        if not 0<=local<anim['playMs']:continue
                        at=round(elapsed+anim.get('startOffsetMs',0)+local)
                        if at+34>duration:continue
                        shape,reason=source_shape(item['row']);identifier=f"{pattern['patternId']}.{anim['occurrenceId'].rsplit('.',1)[-1]}.{item['sourceEffectId']}.{item['row']['SecondaryKey']}.{item['notifyId'].rsplit('-',1)[-1]}.{at}"
                        if reason:
                            receipt['excluded'].append(dict(patternId=pattern['patternId'],sourceProfile=key[0],sourceActionId=key[1],sourceStageId=key[2],
                                sourceEffectId=item['sourceEffectId'],sourceProjectileId=item['row']['ValueA'] if item['row']['Key']==12 else None,
                                sourceNotifyId=item['notifyId'],sourceNotifyYawDegrees=item['notifyYawDegrees'],sourceNotifyPositionCm=item['notifyPositionCm'],
                                sourceArea={k:v for k,v in item['row'].items() if k.startswith('Area')},atMs=at,reason=reason));continue
                        if TAG+identifier in existing_names:continue
                        events.setdefault(at,[]).append((identifier,shape,item))
                elapsed+=stage['durationMs']
            # Mechanic triggers and tracking do not consume the 128 LogicWindow slots.
            used_windows=sum(1 for box in pattern.get('logicOccurrences',[]) if box.get('enabled',True)
                and definitions[box['logicId']].get('judgementKind',definitions[box['logicId']].get('triggerKind')) not in
                    {'SHOWTIME_PLAYER_TARGETS','BOSS_TRACK_TARGET','CROSS_DIRECTION_CLONES','PURSUIT_PROJECTILES'}
                and (definitions[box['logicId']]['logicType']=='DURATION' or definitions[box['logicId']].get('triggerKind') in {'ENTER_AREA','OBJECT_CONTACT'}))
            available=max(0,128-used_windows)
            for ordinal,(at,items) in enumerate(sorted(events.items())):
                if ordinal>=available:
                    receipt['excluded'].append(dict(patternId=pattern['patternId'],atMs=at,sourceEffectIds=[item[2]['sourceEffectId'] for item in items],reason='128_LOGIC_WINDOWS_LIMIT'));continue
                if trigger_id is None:
                    trigger_id=allocate_logic(dict(displayName=TAG+'source damage contact',logicType='TRIGGER',triggerKind='ENTER_AREA'))
                    result_id=allocate_logic(dict(displayName=TAG+f'max HP {damage_percent} percent (tuning)',logicType='RESULT',outcomeKind='MAX_HP_PERCENT_DAMAGE',percent=damage_percent,durationMs=0,followupPatternId=''))
                n=pattern.get('nextLogicOccurrenceOrdinal',1);pattern['nextLogicOccurrenceOrdinal']=n+1;lid=f"{pattern['patternId']}.logic.{n}"
                logicbox=dict(occurrenceId=lid,logicId=trigger_id,startMs=at,durationMs=34,enabled=True,onSuccessLogicIds=[result_id],onFailLogicIds=[],onTimeoutLogicIds=[])
                pattern.setdefault('logicOccurrences',[]).append(logicbox)
                for identifier,shape,item in items[:64]:
                    row=item['row'];n=out['nextPresentationResourceOrdinal'];out['nextPresentationResourceOrdinal']=n+1;rid=f'kakulsaydon.g1.presentation.{n}'
                    resource=dict(resourceId=rid,displayName=TAG+identifier,defaultAnchorKind='BOSS',kind='COLLIDER',assetId='',resourceKind='GROUP',elementId='',durationMs=34,colliderKind='GEOMETRY',**shape)
                    out['presentationResources'].append(resource)
                    forward=(row['AreaOffsetX']+item['notifyPositionCm'][0])*.01+(row['AreaRange']*.005 if shape['shape']=='BOX' else 0)
                    right=(row['AreaOffsetY']+item['notifyPositionCm'][1])*.01;theta=math.radians(facing+item['notifyYawDegrees'])
                    pos=[forward*math.sin(theta)+right*math.cos(theta),(row['AreaOffsetZ']+item['notifyPositionCm'][2])*.01,forward*math.cos(theta)-right*math.sin(theta)]
                    n=pattern.get('nextPresentationOccurrenceOrdinal',1);pattern['nextPresentationOccurrenceOrdinal']=n+1
                    collider=dict(occurrenceId=f"{pattern['patternId']}.presentation.{n}",resourceId=rid,startMs=at,durationMs=34,
                        anchorKind='BOSS',followBoss=True,positionOffset=pos,rotationDegrees=[0,facing+item['notifyYawDegrees']+row['AreaOffsetAngle'],0],scale=[1,1,1],logicOccurrenceId=lid,debugRender=True)
                    pattern.setdefault('presentationOccurrences',[]).append(collider)
                    receipt['changes'].append(dict(patternId=pattern['patternId'],logicOccurrenceId=lid,presentationOccurrenceId=collider['occurrenceId'],resourceId=rid,
                        sourceEffectId=item['sourceEffectId'],sourceNotifyId=item['notifyId'],sourceNotifyYawDegrees=item['notifyYawDegrees'],sourceNotifyPositionCm=item['notifyPositionCm'],payloadSha256=item['payloadSha256'],sourceArea={k:v for k,v in row.items() if k.startswith('Area')},sourceDamageValues=[row['ValueA'],row['ValueB']],sourceDamageImported=False))
        # These are edited MAP impacts, not a replay of the ten original callbacks.
        impact_assets={f'effect.kouku.gate3.showtime.{shape}.impact':eid for shape,eid in
                       (('circle',421991218),('innerdonut',421991220),('outerdonut',421991222))}
        for pattern in out['patterns']:
            if pattern['patternId']!='KAKULSAYDON_G1_PATTERN_35':continue
            for effect in list(pattern.get('presentationOccurrences',[])):
                eid=impact_assets.get(resources.get(effect['resourceId'],{}).get('assetId',''))
                if not eid:continue
                name='[authored impact] '+effect['occurrenceId']
                if name in existing_names:continue
                if effect.get('anchorKind','BOSS')!='MAP' or effect.get('bone') or effect.get('followBoss',False):
                    raise ValueError('Showtime impact requires its existing absolute MAP anchor')
                scale=effect.get('scale',[1,1,1]);rotation=effect.get('rotationDegrees',[0,0,0]);at=effect['startMs']
                if abs(scale[0]-scale[2])>.0001 or rotation[0]!=0 or rotation[2]!=0:
                    raise ValueError('Showtime ring impact requires uniform X/Z and yaw-only geometry')
                row=next(r for r in skill_rows[eid] if r['Key']==1)
                if trigger_id is None:
                    trigger_id=allocate_logic(dict(displayName=TAG+'source damage contact',logicType='TRIGGER',triggerKind='ENTER_AREA'))
                    result_id=allocate_logic(dict(displayName=TAG+f'max HP {damage_percent} percent (tuning)',logicType='RESULT',outcomeKind='MAX_HP_PERCENT_DAMAGE',percent=damage_percent,durationMs=0,followupPatternId=''))
                n=pattern.get('nextLogicOccurrenceOrdinal',1);pattern['nextLogicOccurrenceOrdinal']=n+1;lid=f"{pattern['patternId']}.logic.{n}"
                pattern.setdefault('logicOccurrences',[]).append(dict(occurrenceId=lid,logicId=trigger_id,startMs=at,durationMs=34,enabled=True,onSuccessLogicIds=[result_id],onFailLogicIds=[],onTimeoutLogicIds=[]))
                n=out['nextPresentationResourceOrdinal'];out['nextPresentationResourceOrdinal']=n+1;rid=f'kakulsaydon.g1.presentation.{n}'
                out['presentationResources'].append(dict(resourceId=rid,displayName=name,defaultAnchorKind='MAP',kind='COLLIDER',assetId='',durationMs=34,
                    colliderKind='GEOMETRY',shape='CIRCLE',radiusM=row['AreaRange']*.01,innerRadiusM=row['AreaRemoveRange']*.01,halfExtents=[1,1,1],halfAngleDegrees=180))
                n=pattern.get('nextPresentationOccurrenceOrdinal',1);pattern['nextPresentationOccurrenceOrdinal']=n+1;cid=f"{pattern['patternId']}.presentation.{n}"
                pattern['presentationOccurrences'].append(dict(occurrenceId=cid,resourceId=rid,startMs=at,durationMs=34,anchorKind='MAP',followBoss=False,
                    positionOffset=list(effect.get('positionOffset',[0,0,0])),rotationDegrees=list(rotation),scale=list(scale),logicOccurrenceId=lid,debugRender=True))
                receipt['changes'].append(dict(patternId=pattern['patternId'],logicOccurrenceId=lid,presentationOccurrenceId=cid,resourceId=rid,
                    sourceEffectId=eid,sourcePresentationOccurrenceId=effect['occurrenceId'],sourceArea={k:v for k,v in row.items() if k.startswith('Area')},
                    timingBasis='CURRENT_AUTHORED_IMPACT_START_MS',placementBasis='CURRENT_AUTHORED_MAP_POSITION_YAW_SCALE',sourceDamageImported=False))
        circus_assets={f'effect.kouku.common.circus.{shape}.warning':eid for shape,eid in
                       (('circle',421980629),('innerdonut',421980630),('outerdonut',421980631))}
        for pattern in out['patterns']:
            if pattern['patternId']!='KAKULSAYDON_G1_PATTERN_83':continue
            for impact in list(pattern.get('presentationOccurrences',[])):
                if resources.get(impact['resourceId'],{}).get('assetId')!='effect.kouku.gate1.circus.rainbow.impact':continue
                group=impact.get('selectionGroupId')
                warnings=[x for x in pattern['presentationOccurrences'] if group and x.get('selectionGroupId')==group
                          and resources.get(x['resourceId'],{}).get('assetId')in circus_assets]
                if len(warnings)!=3:raise ValueError('Circus impact requires exactly its three authored warning rings')
                names=['[authored impact] '+impact['occurrenceId']+'.'+x['occurrenceId'] for x in warnings]
                if all(name in existing_names for name in names):continue
                if any(name in existing_names for name in names):raise ValueError('Partially generated Circus impact needs stable-field conflict review')
                at=impact['startMs'];n=pattern.get('nextLogicOccurrenceOrdinal',1);pattern['nextLogicOccurrenceOrdinal']=n+1;lid=f"{pattern['patternId']}.logic.{n}"
                pattern.setdefault('logicOccurrences',[]).append(dict(occurrenceId=lid,logicId=trigger_id,startMs=at,durationMs=34,enabled=True,onSuccessLogicIds=[result_id],onFailLogicIds=[],onTimeoutLogicIds=[]))
                for warning,name in zip(warnings,names):
                    if warning.get('anchorKind')!='MAP' or impact.get('anchorKind')!='MAP' or warning.get('positionOffset')!=impact.get('positionOffset'):
                        raise ValueError('Circus impact and warning must share one exact authored MAP origin')
                    scale=warning.get('scale',[1,1,1]);rotation=warning.get('rotationDegrees',[0,0,0])
                    if abs(scale[0]-scale[2])>.0001 or rotation[0]!=0 or rotation[2]!=0:raise ValueError('Circus hollow rings require uniform X/Z and yaw-only geometry')
                    eid=circus_assets[resources[warning['resourceId']]['assetId']];row=next(r for r in skill_rows[eid] if r['Key']==2)
                    n=out['nextPresentationResourceOrdinal'];out['nextPresentationResourceOrdinal']=n+1;rid=f'kakulsaydon.g1.presentation.{n}'
                    out['presentationResources'].append(dict(resourceId=rid,displayName=name,defaultAnchorKind='MAP',kind='COLLIDER',assetId='',durationMs=34,
                        colliderKind='GEOMETRY',shape='CIRCLE',radiusM=row['AreaRange']*.01,innerRadiusM=row['AreaRemoveRange']*.01,halfExtents=[1,1,1],halfAngleDegrees=180))
                    n=pattern.get('nextPresentationOccurrenceOrdinal',1);pattern['nextPresentationOccurrenceOrdinal']=n+1;cid=f"{pattern['patternId']}.presentation.{n}"
                    pattern['presentationOccurrences'].append(dict(occurrenceId=cid,resourceId=rid,startMs=at,durationMs=34,anchorKind='MAP',followBoss=False,
                        positionOffset=list(warning['positionOffset']),rotationDegrees=list(rotation),scale=list(scale),logicOccurrenceId=lid,debugRender=True))
                    receipt['changes'].append(dict(patternId=pattern['patternId'],logicOccurrenceId=lid,presentationOccurrenceId=cid,resourceId=rid,
                        sourceEffectId=eid,sourcePresentationOccurrenceId=impact['occurrenceId'],sourceWarningOccurrenceId=warning['occurrenceId'],
                        sourceArea={k:v for k,v in row.items() if k.startswith('Area')},timingBasis='CURRENT_AUTHORED_IMPACT_START_MS',placementBasis='CURRENT_AUTHORED_MAP_WARNING_POSITION_YAW_SCALE',sourceDamageImported=False))
    callbacks=[r for r in receipt['excluded'] if r['reason']=='PROJECTILE_CALLBACK_REQUIRES_AUTHORITATIVE_INSTANCE']
    covered=[]
    for row in callbacks:
        pattern=next(p for p in out['patterns'] if p['patternId']==row['patternId'])
        # 421981901 is the already authored spinning-card pursuit action; this
        # preserves that accepted schedule without claiming it decodes the source graph.
        owners=[b['logicId'] for b in pattern.get('logicOccurrences',[]) if b.get('enabled',True)
                and definitions.get(b['logicId'],{}).get('judgementKind')=='PURSUIT_PROJECTILES']
        if row['sourceEffectId']==421981901 and owners:
            row['coverage']='EXISTING_AUTHORED_PER_PROJECTILE_PURSUIT';row['authoritativeLogicIds']=owners;covered.append(row)
        elif row['sourceProjectileId']==421990319 and any(definitions[b['logicId']].get('triggerKind')=='ALBION_BLUE_CIRCLE' and definitions[b['logicId']].get('fixedHits') for b in pattern.get('logicOccurrences',[]) if b.get('enabled',True)):
            row['coverage']='EXISTING_AUTHORITATIVE_ALBION_CIRCLES_SOURCE_GEOMETRY_AND_2S_IMPACT'
        elif row['sourceProjectileId'] in (421991207,421991208,421991209,421980613) and any(c.get('patternId')==pattern['patternId'] and c.get('sourcePresentationOccurrenceId') for c in receipt['changes']):
            row['coverage']='CURRENT_AUTHORED_MAP_IMPACTS_REPLACE_SOURCE_SCHEDULE_NOT_ONE_TO_ONE'
        elif row['sourceProjectileId'] in (421971800,421990606):
            row['coverage']='NO_DIRECT_DAMAGE_CALLBACK_NPC_SUMMON_OR_PRESENTATION'
        else:row['coverage']='HOLDOUT_SOURCE_ORIGIN_CONDITION_TIMER_UNCONFIRMED'
    receipt['projectileCoverage']=dict(sourceOccurrences=len(callbacks),existingAuthoredPursuitOccurrences=len(covered),
        existingAuthoritativeAlbionOccurrences=sum(r['coverage'].startswith('EXISTING_AUTHORITATIVE_ALBION') for r in callbacks),
        currentAuthoredMapReplacementSourceOccurrences=sum(r['coverage'].startswith('CURRENT_AUTHORED_MAP') for r in callbacks),
        noDirectDamageCallbackOccurrences=sum(r['coverage'].startswith('NO_DIRECT_DAMAGE') for r in callbacks),
        unresolvedOccurrences=sum(r['coverage'].startswith('HOLDOUT') for r in callbacks),originalScheduleDecoded=False)
    out['revision']=document['revision']+1
    receipt['summary']=dict(changes=len(receipt['changes']),normalShapes=sum('resourceId'in r for r in receipt['changes']),
        authoredImpactShapes=sum('sourcePresentationOccurrenceId'in r for r in receipt['changes']),
        sourceDirectShapes=sum('resourceId'in r and 'sourcePresentationOccurrenceId'not in r for r in receipt['changes']),
        affectedPatterns=len({r['patternId'] for r in receipt['changes'] if 'patternId'in r}),exclusions=dict(collections.Counter(r['reason'] for r in receipt['excluded'])))
    return out,receipt

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--composition',type=Path,default=ROOT/'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json');parser.add_argument('--source',type=Path,default=DEFAULT_SOURCE);parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();args.output.mkdir(parents=True,exist_ok=True)
    candidate,receipt=prepare(read(args.composition),args.source);receipt['base']=dict(path=str(args.composition),sha256=digest(args.composition))
    write(args.output/'KoukuSaydonComposition.json',candidate);write(args.output/'attack-colliders.receipt.json',receipt)
    print(json.dumps(receipt['summary']))
if __name__=='__main__':main()

"""Group exact, static Matinee particle occurrences without flattening motion.

Unconsumed original movement, skeletal attachment and parameter curves remain
occurrence-level evidence. They never enter a static candidate as frozen FX.
"""
import argparse
import collections
import copy
import hashlib
import math
import functools
from pathlib import Path

import build_kouku_action_effect_groups as action
import build_maptool_scene as transforms

source = action.source
ROOT = source.ROOT


def vector(value, fallback=(0,0,0)):
    return [float(value.get(k, fallback[i])) for i,k in enumerate(('x','y','z'))]


def constant_curve(curve):
    points = curve.get('points', [])
    if not points:
        return None
    first = points[0]['outval']
    if any(p['outval'] != first for p in points):
        raise ValueError('SOURCE_NONCONSTANT_HERMITE_CURVE')
    for point in points:
        if point.get('interpmode', 'cim_linear') not in ('cim_linear', 'cim_constant'):
            for field in ('arrivetangent','leavetangent'):
                value = point.get(field, 0)
                if any(v != 0 for v in value.values()) if isinstance(value, dict) else value != 0:
                    raise ValueError('SOURCE_NONZERO_CONSTANT_ENDPOINT_TANGENT')
    return first


def source_tracks(occurrence, cache):
    group = cache['rows'][str(occurrence['groupExport'])]
    return [dict(exportId=k, **cache['rows'][str(k)]) for k in group['p'].get('interptracks', [])]


def static_transform(occurrence, tracks):
    props = occurrence['actorProperties']
    if props.get('basebonename'):
        raise ValueError('SOURCE_SKELETAL_PARENT_REQUIRES_ANIMATED_MODEL_CUE')
    if props.get('base'):
        raise ValueError('SOURCE_ATTACHED_PARENT_REQUIRES_PARENT_TRACK_RESOLUTION')
    position = vector(props['location'])
    rotation = {k:props.get('rotation', {}).get(k, 0) for k in ('pitch','yaw','roll')}
    for track in tracks:
        if track['cls'] != 'interptrackmove' or track['p'].get('bdisabletrack', False):
            continue
        values = track['p']
        if values.get('moveframe', 'imf_world') != 'imf_world':
            raise ValueError('SOURCE_RELATIVE_FIRST_KEY_MOVE_FRAME_REQUIRES_OWNER_INITIAL_TRANSFORM')
        # Lookup tracks can dynamically refer to another group instead of the
        # serialized OutVal. An empty/None lookup keeps the exact keyed value.
        if any(p.get('groupname', 'none') not in ('none', '') for p in values.get('lookuptrack',{}).get('points', [])):
            raise ValueError('SOURCE_MOVE_LOOKUP_GROUP_REQUIRES_DYNAMIC_OWNER')
        pos = constant_curve(values.get('postrack', {}))
        rot = constant_curve(values.get('eulertrack', {}))
        if pos is not None:
            position = vector(pos)
        if rot is not None:
            # UE3 InterpTrackMove EulerTrack owns degrees (not Rotator ints).
            rotation = {k:vector(rot)[i] * 65536 / 360 for i,k in enumerate(('roll','pitch','yaw'))}
    matrix = transforms.directx_row_matrix_from_quaternion(transforms.convert_rotation(rotation))
    pitch = math.asin(max(-1,min(1,-matrix[2][1])))
    yaw = math.atan2(matrix[2][0], matrix[2][2]) if abs(math.cos(pitch)) > 1e-5 else math.atan2(-matrix[0][2],matrix[0][0])
    roll = math.atan2(matrix[0][1],matrix[1][1]) if abs(math.cos(pitch)) > 1e-5 else 0
    scale = props.get('drawscale', 1)
    scale3 = vector(props.get('drawscale3d',{}), (1,1,1))
    return dict(sourcePositionUE3Cm=position,
        rotationDegrees=[math.degrees(pitch),math.degrees(yaw),math.degrees(roll)],
        scale=[scale*scale3[i] for i in (0,2,1)])


def parameters(occurrence, tracks):
    values = {}
    for parameter in occurrence['componentProperties'].get('instanceparameters', []):
        name, kind = parameter['name'], parameter['paramtype']
        if kind == 'pspt_scalar':
            values[name.lower()] = dict(name=name, type='scalar', scalarValue=parameter['scalar'])
        elif kind == 'pspt_vector':
            values[name.lower()] = dict(name=name, type='vector', vectorValue=vector(parameter['vector']))
        elif kind != 'pspt_none':
            raise ValueError('Unsupported original component parameter: ' + kind)
    for track in tracks:
        props = track['p']
        if props.get('bdisabletrack',False):
            continue
        if track['cls'] not in ('interptrackfloatparticleparam','efinterptrackvectorparticleparam'):
            continue
        name = props['paramname']
        if track['cls'] == 'interptrackfloatparticleparam':
            value = constant_curve(props['floattrack'])
            if value is not None:
                values[name.lower()] = dict(name=name, type='scalar', scalarValue=value)
        else:
            value = constant_curve(props['vectortrack'])
            if value is not None:
                values[name.lower()] = dict(name=name, type='vector', vectorValue=vector(value))
    return list(values.values())


def curve_keys(curve):
    result = []
    for p in curve.get('points', []):
        mode = p.get('interpmode','cim_linear')
        interpolation = {'cim_linear':'linear','cim_constant':'constant',
            'cim_curveauto':'cubic','cim_curveautoclamped':'cubic',
            'cim_curveuser':'cubic','cim_curvebreak':'cubic'}[mode]
        result.append(dict(timeSeconds=p['inval'],value=vector(p['outval']),
            arriveTangent=vector(p.get('arrivetangent',{})),
            leaveTangent=vector(p.get('leavetangent',{})),interpolation=interpolation))
    return result


def alpha_parameter_track(occurrence, tracks, template, index):
    active=[t for t in tracks if t['cls']=='interptrackfloatparticleparam' and not t['p'].get('bdisabletrack',False)]
    animated=[]
    for t in active:
        try: constant_curve(t['p'].get('floattrack',{}))
        except ValueError: animated.append(t)
    if not animated:
        return parameters(occurrence,tracks),None
    assert len(animated)==1,'SOURCE_MULTIPLE_DYNAMIC_SCALAR_PARAMETERS'
    track=animated[0]
    name=track['p']['paramname'].lower()
    affected=[]
    for element in template['elements']:
        for module in element['sourceRecipe']['modules']:
            for dist in module['distributions']:
                obj=index.get_path(dist.get('sourceObjectPath'))
                if obj and action.IMPORTED.prop(obj.properties,'parametername','').lower()==name:
                    assert (module['className']=='particlemodulecolorscaleoverlife' and
                        dist['propertyPath']=='alphascaleoverlife' and
                        action.IMPORTED.prop(obj.properties,'parammode','')=='dpm_direct'), 'SOURCE_DYNAMIC_PARAMETER_REQUIRES_NON_ALPHA_MODULE_CLOCK'
                    affected.append(element['id'])
    assert affected,'SOURCE_ANIMATED_PARAMETER_HAS_NO_CONSUMER'
    changed=copy.deepcopy(occurrence)
    changed['componentProperties']['instanceparameters']=[p for p in changed['componentProperties'].get('instanceparameters',[])if p['name'].lower()!=name]
    bindings=parameters(changed,[t for t in tracks if t is not track])
    bindings.append(dict(name=name,type='scalar',scalarValue=1))
    raw=copy.deepcopy(track['p']['floattrack'])
    for key in raw['points']:
        for field in ('outval','arrivetangent','leavetangent'):
            key[field]=dict(x=key.get(field,0),y=0,z=0)
    return bindings,dict(keys=curve_keys(raw),affectedElements=affected,sourceParameterName=name)


def actor_groups(occurrence, cache, actor):
    rows=cache['rows']
    matinee=rows[str(occurrence['matineeExport'])]['p']
    names={link['linkdesc'] for link in matinee.get('variablelinks',[])
        if any(rows[str(v)]['p'].get('objvalue')==actor for v in link.get('linkedvariables',[]) if v>0)}
    return [rows[str(g)] for g in rows[str(occurrence['dataExport'])]['p'].get('interpgroups',[])
        if rows[str(g)]['p'].get('groupname') in names]


def actor_tracks(occurrence, cache, actor):
    return [dict(exportId=t,**cache['rows'][str(t)])
        for g in actor_groups(occurrence,cache,actor) for t in g['p'].get('interptracks',[])]


@functools.lru_cache(None)
def decoded_lookup(source_path):
    import build_kouku_gate3_rainbow_native as native
    ue3=source.ue3
    package_name,relative=source_path.split('.',1)
    package=native.pkg(package_name)
    entry=native.find_export(package,relative)
    raw=package.logical[entry.serial_offset:entry.serial_offset+entry.serial_size]
    original=ue3.decode_property_value
    def decode(pt,st,payload,names,bool_value,pn=None,owner=None):
        if pt.lower()=='structproperty' and str(st).lower()=='interplookuptrack':
            properties,end=ue3.parse_tagged_properties_at(payload,names,0,st)
            assert end==len(payload),'SOURCE_LOOKUP_TRAILING_BYTES'
            return dict(properties=properties)
        return original(pt,st,payload,names,bool_value,pn,owner)
    ue3.decode_property_value=decode
    try:
        properties,_=ue3.parse_tagged_properties(raw,package.names,package.summary.version)
    finally:
        ue3.decode_property_value=original
    def unwrap(v):
        if isinstance(v,dict):
            if 'type'in v and 'value'in v:return unwrap(v['value'])
            if 'properties'in v:return unwrap(v['properties'])
            return {k:unwrap(x)for k,x in v.items()}
        if isinstance(v,list):return [unwrap(x)for x in v]
        return v
    result=unwrap(properties['lookuptrack'])
    assert isinstance(result.get('points',[]),list),'SOURCE_LOOKUP_POINTS_NOT_DECODED'
    return result


def moving_transform(occurrence, cache):
    nodes=[]
    visiting=set()
    def append(actor):
        assert actor not in visiting, 'SOURCE_ACTOR_ATTACHMENT_CYCLE'
        visiting.add(actor)
        row=cache['rows'][str(actor)]
        props=row['p']
        if props.get('basebonename'):
            raise ValueError('SOURCE_SKELETAL_PARENT_REQUIRES_ANIMATED_MODEL_CUE')
        base=props.get('base',0)
        if base:
            assert base>0, 'SOURCE_EXTERNAL_ATTACHMENT_PARENT'
            append(base)
        tracks=actor_tracks(occurrence,cache,actor)
        moves=[t for t in tracks if t['cls']=='interptrackmove' and not t['p'].get('bdisabletrack',False)]
        assert len(moves)<=1, 'SOURCE_MULTIPLE_ACTOR_MOVE_TRACKS'
        frame='PARENT' if base else 'WORLD'
        position=vector(props.get('relativelocation' if base else 'location',{}))
        rot=props.get('relativerotation' if base else 'rotation',{})
        euler=[rot.get(k,0)*360/65536 for k in ('roll','pitch','yaw')]
        position_keys,euler_keys=[],[]
        if moves:
            move=moves[0]['p']
            lookup=move.get('lookuptrack',{})
            if 'hex'in lookup:
                lookup=decoded_lookup(occurrence['sourceScene'].lower()+'.'+moves[0]['name'])
            assert not any(p.get('groupname','none') not in ('none','') for p in lookup.get('points',[])), 'SOURCE_MOVE_LOOKUP_GROUP_REQUIRES_DYNAMIC_OWNER'
            frame={'imf_world':'WORLD','imf_relativetoinitial':'RELATIVE_TO_INITIAL'}[move.get('moveframe','imf_world')]
            if base:
                assert frame=='WORLD','SOURCE_PARENT_RELATIVE_TO_INITIAL_MOVE'
                # Source attached actor keys are serialized in its base frame.
                # SCENE07A's first placed key equals RelativeLocation, while its
                # absolute actor location is in the remote sequence set.
                frame='PARENT'
            position_keys=curve_keys(move.get('postrack',{}))
            euler_keys=curve_keys(move.get('eulertrack',{}))
        scale=[v*props.get('drawscale',1) for v in vector(props.get('drawscale3d',{}),(1,1,1))]
        nodes.append(dict(sourceObjectPath=occurrence['sourceScene'].lower()+'.'+row['name'],frame=frame,
            initialPositionUE3Cm=position,initialEulerDegrees=euler,scaleUE3=scale,
            positionKeys=position_keys,eulerKeys=euler_keys))
    append(occurrence['actorExport'])
    return dict(sourcePositionUE3Cm=vector(occurrence['actorProperties']['location']),
        rotationDegrees=[0,0,0],scale=[1,1,1],sourceTransformNodes=nodes)


def project(organization, library_root, source_root, evidence, install, source_motion=False,
            sampled_transforms=None):
    organization = source.read(organization)
    library = source.read(library_root / 'installation.json')
    index = action.restored_index(source_root)
    templates = {r['sourceParticleSystem']:source.read(library_root / 'candidate' / Path(r['path']).name) for r in library['documents']}
    scenes, grouped = {}, collections.defaultdict(list)
    for occurrence in organization['sequences']['occurrences']:
        grouped[(occurrence['sourceScene'],occurrence['matineeId'])].append(occurrence)
        scenes.setdefault(occurrence['sourceCache'], source.read(Path(occurrence['sourceCache'])))
    documents, failures, observations = [], [], []
    for (scene, matinee), occurrences in sorted(grouped.items()):
        asset = 'effect.kouku.sequence.' + scene.lower() + '.' + matinee
        retained, rejected = [], []
        for occurrence in occurrences:
            tracks = source_tracks(occurrence, scenes[occurrence['sourceCache']])
            observation = dict(occurrence=occurrence, originalTracks=tracks)
            try:
                if not occurrence['liveReference'] or occurrence['disabled']:
                    raise ValueError('SOURCE_DISABLED_OR_UNREACHABLE_SEQUENCE_OCCURRENCE')
                sampled = (sampled_transforms or {}).get(occurrence['sourceOccurrenceId'])
                if sampled is not None:
                    assert source_motion and occurrence['actorProperties'].get('basebonename'), 'SAMPLED_TRANSFORM_REQUIRES_SOURCE_BONE_PARENT'
                    transform = copy.deepcopy(sampled)
                else:
                    transform = moving_transform(occurrence,scenes[occurrence['sourceCache']]) if source_motion else static_transform(occurrence, tracks)
                assert occurrence['sourceSystem'] in templates, 'SOURCE_PARTICLE_SYSTEM_LIBRARY_NOT_PROJECTED'
                if source_motion:
                    bindings,alpha=alpha_parameter_track(occurrence,tracks,templates[occurrence['sourceSystem']],index)
                    if alpha:transform['sourceAlphaScale']=alpha
                else:
                    bindings=parameters(occurrence,tracks)
                assert occurrence['activationIntervals'], 'SOURCE_HAS_NO_ACTIVE_INTERVAL'
                if any(i['stopSeconds'] is None for i in occurrence['activationIntervals']):
                    # ETTA_Trigger owns a one-shot activation, not an invented
                    # Matinee stop. Its original finite emitter loops finish it.
                    assert all(e['sourceRecipe']['emitterLoopCount'] > 0
                        and e['sourceRecipe']['emitterDurationSeconds'] > 0
                        for e in templates[occurrence['sourceSystem']]['elements']), \
                        'SOURCE_UNBOUNDED_TRIGGER_REQUIRES_OWNER_STOP'
                retained.append((occurrence, transform, bindings))
                observation.update(status='EXACT_SOURCE_TRANSFORM_OCCURRENCE' if source_motion else 'EXACT_CONSTANT_OCCURRENCE', transform=transform, parameters=bindings)
            except Exception as error:
                failure = dict(sourceOccurrenceId=occurrence['sourceOccurrenceId'],sourceScene=scene, matineeId=matinee,
                    groupName=occurrence['groupName'], sourceParticleSystem=occurrence['sourceSystem'], reason=str(error))
                failures.append(failure)
                rejected.append(failure)
                observation.update(status='SOURCE_OWNER_REQUIRED', reason=str(error))
            observations.append(observation)
        if not retained:
            continue
        origin = retained[0][1]['sourcePositionUE3Cm']
        first_time = min(i['startSeconds'] for r,_,_ in retained for i in r['activationIntervals'])
        elements = []
        for occurrence, transform, bindings in retained:
            position = transform['sourcePositionUE3Cm']
            local = dict(position=[(position[0]-origin[0])*.01,(position[2]-origin[2])*.01,-(position[1]-origin[1])*.01],
                rotationDegrees=transform['rotationDegrees'],scale=transform['scale'])
            if source_motion:
                local['position']=[0,0,0]
            cue = dict(enabled=True,particleDataDecoded=True,parameterOverridesDecoded=True,parameterOverrides=bindings,
                attachment=dict(mode='SNAPSHOT_ROOT',sourceAnchorNames=[],runtimeAnchors=[],runtimeAnchorSlotId='root',runtimeBoneName=''),
                localTransform=local)
            for ordinal, interval in enumerate(occurrence['activationIntervals']):
                notify = dict(notifyId=occurrence['sourceOccurrenceId'] + f'/activation-{ordinal}',
                    localTimeSeconds=interval['startSeconds']-first_time,
                    durationSeconds=0 if interval['stopSeconds'] is None else interval['stopSeconds']-interval['startSeconds'])
                stream,_ = action.instantiate(templates[occurrence['sourceSystem']],cue,notify,asset,scene,index)
                for element in stream:
                    if element['detail']['particle']['lifeTimeSeconds'] == [0, 0]:
                        # Cascade's zero lifetime remains in SourceRecipe. The
                        # current particle runtime consumes a positive Detail
                        # fallback for that case; bound it by this source ON/OFF
                        # interval, rather than an invented global lifetime.
                        active = element['detail']['timing']['lifeTimeSeconds']
                        assert 0 < active <= 30, 'SOURCE_ZERO_LIFETIME_REQUIRES_BOUNDED_OWNER'
                        element['detail']['particle']['lifeTimeSeconds'] = [active, active]
                    element['actionCueAttachment']['enabled'] = False
                    element['actionCueAttachment'].pop('snapshotRootSourceBasisYawDegrees',None)
                    if source_motion:
                        element['sourceTransformTrack']=dict(sourceOccurrenceId=occurrence['sourceOccurrenceId'],
                            sourceTimeOriginSeconds=first_time,previewOriginUE3Cm=origin,nodes=transform['sourceTransformNodes'])
                        if transform.get('sourceAlphaScale'):
                            original_id=next(e['id'] for e in templates[occurrence['sourceSystem']]['elements']
                                if e['sourcePresentation']['sourceObjectPath']==element['sourcePresentation']['sourceObjectPath'])
                            if original_id in transform['sourceAlphaScale']['affectedElements']:
                                element['sourceTransformTrack']['alphaScaleKeys']=transform['sourceAlphaScale']['keys']
                elements += stream
        # Cascade ribbon runtimeCarrier entries require authored-v15 even when
        # this Matinee has no baked animation-edge histories of its own.
        histories={}
        for occurrence,_,_ in retained:
            for history in templates[occurrence['sourceSystem']].get('runtimeExtensions',{}).get('bakedEdgeHistories',[]):
                assert history['historyId'] not in histories or histories[history['historyId']]==history
                histories[history['historyId']]=copy.deepcopy(history)
        document = dict(schema='lostark.effect-authoring',version=15,effectAssetId=asset,
            displayName=occurrences[0]['classification']['displayName'],
            particleSystem=dict(uniformScaleMultiplier=1,yawOffsetDegrees=0,directionYawDegrees=0,initialSpeedMultiplier=1),
            modelCues=[],runtimeExtensions=dict(formatVersion=1,bakedEdgeHistories=list(histories.values())),elements=elements)
        path = ROOT / 'Data/Effects/Authored' / (asset + '.effect.json')
        source.write(evidence / 'candidate' / path.name, document)
        complete = not rejected
        if install and complete:
            assert library['installed'], 'Native source library must be installed first'
            assert not path.exists() or source.read(path)==document, 'Preserve authored edits: '+str(path)
            source.write(path,document)
        duration = math.ceil(max(e['detail']['timing']['startDelaySeconds']+e['detail']['timing']['lifeTimeSeconds']+
            (0 if e['kind']=='light' else max(e['detail']['particle']['lifeTimeSeconds'])) for e in elements)*1000)
        gate = occurrences[0]['classification']['gateId']
        documents.append(dict(effectAssetId=asset,path=path.relative_to(ROOT).as_posix(),displayName=document['displayName'],
            sourceScene=scene,matineeId=matinee,categoryPath=['KoukuSaydon',gate or '원본 시퀀스','연출',document['displayName']],
            complete=complete,elementCount=len(elements),durationMs=duration,
            originalOccurrenceCount=len(occurrences),projectedOccurrenceCount=len(retained),sourceFailures=rejected,
            previewOriginUE3Cm=origin,previewTimeOriginSeconds=first_time,
            sourceOccurrenceIds=[r['sourceOccurrenceId'] for r,_,_ in retained],sourceParticleSystems=sorted({r['sourceSystem'] for r,_,_ in retained})))
    source.write(evidence / 'source_occurrences.json',observations)
    source.write(evidence / 'installation.json',dict(installed=install,documents=documents,sourceFailures=failures,
        sourceMatineeCount=len(grouped),sourceOccurrenceCount=len(observations),manualVisualValidation='USER_PENDING'))
    print(dict(documents=len(documents),complete=sum(d['complete'] for d in documents),
        projectedOccurrences=sum(d['projectedOccurrenceCount'] for d in documents),elements=sum(d['elementCount'] for d in documents),sourceFailures=len(failures)))


if __name__ == '__main__':
    parser=argparse.ArgumentParser(__doc__)
    parser.add_argument('--organization',type=Path,default=ROOT/'out/KoukuAllEffects20260912/organization.json')
    parser.add_argument('--library-root',type=Path,default=ROOT/'out/KoukuAllEffectsLightClosure20260912')
    parser.add_argument('--source-root',type=Path,default=ROOT/'out/KoukuAllEffects20260912')
    parser.add_argument('--evidence-root',type=Path,default=ROOT/'out/KoukuSequenceEffects20260912')
    parser.add_argument('--install',action='store_true')
    parser.add_argument('--source-motion',action='store_true',help='Consume original Hermite movement and attachment parents')
    args=parser.parse_args()
    project(args.organization,args.library_root,args.source_root,args.evidence_root,args.install,args.source_motion)

"""Build an isolated V1 circus-ball drop/split authoring preview.

The installed native mesh/material and enabled Cascade fall/bounce curves stay
unchanged. Original Missile death callbacks provide the two-child graph and
generation scales. Source data does not establish the original engine's
distance-vs-lifetime termination precedence: this preview deliberately chooses
the serialized lifetime and clamps horizontal travel to the serialized range.
No gameplay authority or live Composition document is changed by this builder.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
from pathlib import Path
import re
import sqlite3
import struct

import build_kouku_gate1_full_restore as source
from build_kouku_backstep_flame_groups import project_mesh_rotation, remap

ROOT = source.ROOT
ASSET = 'effect.kouku.common.circus.ball.drop.split'
NAME = '세이튼_서커스공_낙하분열(편집용)'
AUTHORED = ROOT / 'Data/Effects/Authored'
PROJECTILES = ROOT / 'out/KoukuAllEffects20260912/source/Projectile'
DATABASE = source.SOURCE / 'WorldObjectExtraction-20260907/EFTable_SkillEffect.db'
SOURCE_IDS = tuple(range(421980602, 421980608))
SYSTEMS = ('fx_mn_rpct_05_g.par_g_rpct_05_circusball_prj_01',
           'fx_mn_rpct_05_g.par_g_rpct_05_circusball_prj_02')


def digest(path):
    return dict(path=path.as_posix(), sha256=hashlib.sha256(path.read_bytes()).hexdigest())


def module_enabled(module):
    return next((v['value'] for v in module['literals'] if v['propertyPath'] == 'benabled'), True)


def source_vector_defaults(contract, libraries):
    """Resolve original unit mesh size and direct-location scale.

    Both source instances serialize only Distribution=None inside these two
    raw-distribution structs. Their class CDOs supply the unchanged cooked
    lookup fields. The installed leaves lost those nested defaults: StartSize
    falls back to the generic .2 extent and ScaleFactor suppresses the location
    curve with zero. Restore only the missing fields in the derived group.
    Collision radius does not determine visual size.
    """
    defaults_path=ROOT/'out/KoukuAllEffects20260912/source_class_defaults.json'
    instances_path=ROOT/'out/KoukuAllEffects20260912/source_module_inputs.json'
    defaults=source.read(defaults_path)['records']
    instances=source.read(instances_path)['records']
    repairs=[]
    for class_name,property_name,evidence_name in (
            ('particlemodulesize','StartSize','meshStartSizeDefault'),
            ('particlemodulelocationdirect','ScaleFactor','directLocationScaleDefault')):
        cdo=next(r for r in defaults if r['fullPath']=='engine.default__'+class_name)
        cdo_property=cdo['properties'][property_name]
        assert cdo_property['structType']=='RawDistributionVector'
        values=cdo_property['value']['properties']
        table=values['LookupTable']['value']
        assert table==[1.0]*8
        assert [values[k]['value'] for k in ('Op','LookupTableNumElements','LookupTableChunkSize')]==[1,1,3]
        observations=[]
        for library in libraries.values():
            element=library['elements'][0]
            module=next(m for m in element['sourceRecipe']['modules'] if m['className']==class_name and module_enabled(m))
            instance=instances[module['objectPath']]
            raw_property=instance['properties'][property_name.lower()]
            assert raw_property['structType']=='rawdistributionvector'
            delta=raw_property['value']['properties']
            assert list(delta)==['distribution'] and delta['distribution']['value']==0
            distribution=next(d for d in module['distributions'] if d['propertyPath']==property_name.lower())
            assert distribution['componentCount']==3
            assert not distribution['lookupTable'] and not distribution['keys']
            observations.append(dict(sourceModule=module['objectPath'],sourceExportIndex=instance['exportIndex'],
                serializedPropertyDelta=delta,installedDistribution=copy.deepcopy(distribution)))
        inherited=dict(operation=values['Op']['value'],
            lookupTableNumElements=values['LookupTableNumElements']['value'],
            lookupTableChunkSize=values['LookupTableChunkSize']['value'],
            lookupTableTimeScale=values['LookupTableTimeScale']['value'],
            lookupTableStartTime=values['LookupTableStartTime']['value'],lookupTable=table)
        contract[evidence_name]=dict(sourceClassDefault=cdo,sourceInstances=observations,
            dimensionlessScale=table[2:5],inheritedDistributionFields=inherited,
            projection='SOURCE_CDO_NESTED_RAW_DISTRIBUTION_INHERITANCE',collisionRadiusUsed=False)
        repairs.append(dict(className=class_name,propertyPath=property_name.lower(),inheritedFields=inherited))
    contract['sourceInputs'].extend([digest(defaults_path),digest(instances_path)])
    return repairs


def inherit_source_vectors(recipe, repairs):
    for repair in repairs:
        module=next(m for m in recipe['modules'] if m['className']==repair['className'] and module_enabled(m))
        distribution=next(d for d in module['distributions'] if d['propertyPath']==repair['propertyPath'])
        distribution.update(copy.deepcopy(repair['inheritedFields']))


def project_active_recipe(recipe, repairs):
    # Source execution skips disabled modules. The portable carrier validates
    # module cardinality before execution, so its recipe contains active ones.
    # The untouched source recipes and omitted modules remain in the evidence.
    recipe['modules']=[module for module in recipe['modules'] if module_enabled(module)]
    inherit_source_vectors(recipe,repairs)


def read_contract():
    rows, inputs = [], [digest(DATABASE)]
    with sqlite3.connect('file:' + DATABASE.as_posix() + '?mode=ro', uri=True) as database:
        database.row_factory = sqlite3.Row
        for index, identifier in enumerate(SOURCE_IDS):
            path = PROJECTILES / (str(identifier) + '.loa')
            raw = path.read_bytes()
            inputs.append(digest(path))
            assert raw[20:56] == b'CEFSequenceSummonsProjectileMissile\0'
            # A parameter payload adds 82 bytes to the first ParticleData for
            # generations 2..5. Assert all concrete fields, not guessed types.
            offset = 1058 + (82 if index >= 2 else 0)
            scale, radius, height, minimum, maximum, life, speed = struct.unpack_from('<3f2ifi', raw, offset)
            expected = ((2,112,250), (1.7,95,275), (1.4,78,300),
                        (1.1,62,325), (.8,45,350), (.5,28,400))[index]
            assert math.isclose(scale, expected[0], abs_tol=1e-6)
            assert (radius, minimum, maximum, height, life, speed) == (expected[1], expected[2], expected[2], 50, 1.5, 1500)
            # The motion block is followed by contact-action count; contact
            # actions are distinct from the following end-action array.
            contact_count = struct.unpack_from('<I', raw, offset + 44)[0]
            assert contact_count == (1 if index == 5 else 2)
            marker = b'CEFSequenceSummonsActionSkillEffect\0'
            callbacks, cursor = [], 0
            while (cursor := raw.find(marker, cursor)) >= 0:
                payload = cursor + len(marker)
                assert struct.unpack_from('<7I', raw, payload) == (0,0,1,1,1,1,0)
                ordinal, effect_id, contact = struct.unpack_from('<3I', raw, payload + 28)
                callback = dict(sourceOffset=cursor, sourceFieldOffset=payload+32,
                                ordinal=ordinal, skillEffectId=effect_id, contact=bool(contact))
                callbacks.append(callback)
                cursor = payload + 40
            children = [c for c in callbacks if not c['contact']]
            assert len(children) == (0 if index == 5 else 2)
            for child in children:
                table = dict(database.execute('select * from SkillEffect where PrimaryKey=?',
                                               (child['skillEffectId'],)).fetchone())
                assert table['Key'] == 12 and table['ValueA'] == identifier + 1
                assert [table[k] for k in ('AreaOffsetX','AreaOffsetY','AreaOffsetZ','AreaOffsetAngleRandom')] == [50,0,0,360]
                child['sourceSkillEffect'] = table
            rows.append(dict(projectileId=identifier, sourceMotionOffset=offset,
                scale=scale, radiusCm=radius, heightCm=height, minDistanceCm=minimum,
                maxDistanceCm=maximum, maxLifeSeconds=life, speedCmPerSecond=speed,
                sourceSystem=SYSTEMS[0 if index == 0 else 1],
                contactCallbacks=[c for c in callbacks if c['contact']], endChildCallbacks=children))
        warning_rows = [dict(database.execute('select * from SkillEffect where PrimaryKey=?', (i,)).fetchone())
                        for i in (421980629,421980630,421980631)]
    return dict(sourceActionId=4219806, projectiles=rows, sourceInputs=inputs,
        originalBoundary='Missile hit actions and two end-child callbacks are serialized; no timed callback trailer is present.',
        previewPolicy=dict(termination='SERIALIZED_MAX_LIFETIME', horizontalMotion='SOURCE_SPEED_CLAMPED_TO_SOURCE_MAX_DISTANCE',
            directions='DETERMINISTIC_SAMPLE_OF_SOURCE_360_DEGREE_RANDOM_RANGE', sourceSeedAvailable=False,
            firstOriginUE3Cm=[0,0,0], gameplayAuthority=False,
            unresolved='Original engine distance-arrival versus max-lifetime termination precedence and live target positions.'),
        groundWarningSource=dict(projectileId=421980613, damageRows=warning_rows,
            radiiMeters=[dict(skillEffectId=r['PrimaryKey'], inner=r['AreaRemoveRange']*.01,
                             outer=r['AreaRange']*.01) for r in warning_rows]))


def key(time, value, interpolation='linear'):
    return dict(timeSeconds=time, value=value, arriveTangent=[0]*len(value),
                leaveTangent=[0]*len(value), interpolation=interpolation)


def preview_direction(identity):
    # This seed is explicitly a repeatable editing choice. It does not claim
    # to reproduce the source encounter's random state or player targets.
    unit = int(hashlib.sha256(identity.encode()).hexdigest()[:8], 16) / 0x100000000
    return unit * 360


def append_ball(document, library, projectile, identity, parent, start, origin, angle, source_defaults):
    group_id = 'kouku.ball.' + hashlib.sha256(identity.encode()).hexdigest()[:20]
    ids = {e['id']: group_id+'.'+str(i) for i,e in enumerate(library['elements'])}
    elements = remap(copy.deepcopy(library['elements']), ids)
    end = start + projectile['maxLifeSeconds']
    travel = projectile['maxDistanceCm'] / projectile['speedCmPerSecond']
    radians = math.radians(angle)
    finish = [origin[0]+projectile['maxDistanceCm']*math.cos(radians),
              origin[1]+projectile['maxDistanceCm']*math.sin(radians), origin[2]]
    for element in elements:
        element['groupId'] = group_id
        element['displayName'] = f'Ball {identity} | Projectile {projectile["projectileId"]}'
        element['sourceNode'] = identity+'|'+element['sourcePresentation']['sourceObjectPath']
        element['detail']['timing']['startDelaySeconds'] += start
        element['sourcePresentation']['sourceTimeSeconds'] += start
        element['sourcePresentation']['sourceEventId'] = identity
        element['detail']['transform'].update(position=[0,0,0], rotationDegrees=[0,0,0], scale=[1,1,1])
        project_active_recipe(element['sourceRecipe'],source_defaults)
        element['actionCueAttachment'].update(enabled=True, follow=False,
            sourceAnchorSlotId='root',runtimeAnchorSlotId='root',runtimeBoneName='',
            socketLocalTransform=dict(position=[0,0,0],rotationDegrees=[0,0,0],scale=[1,1,1]),
            snapshotRootSourceBasisYawDegrees=-90)
        element['sourceTransformTrack'] = dict(sourceOccurrenceId=identity,
            sourceTimeOriginSeconds=0, previewOriginUE3Cm=[0,0,0],
            nodes=[dict(sourceObjectPath=f'projectile/{projectile["projectileId"]}/{identity}',
                frame='WORLD', initialPositionUE3Cm=origin, initialEulerDegrees=[0,0,angle],
                scaleUE3=[projectile['scale']]*3,
                positionKeys=[key(start,origin),key(start+travel,finish),key(end,finish)], eulerKeys=[])],
            # Keep the source Cascade life and its normalized bounce curve.
            # The chosen parent expiry hides the carrier without rewriting it.
            alphaScaleKeys=[key(start,[1,1,1],'constant'),key(end,[0,0,0],'constant')])
        project_mesh_rotation(element)
        document['elements'].append(element)
    return dict(occurrenceId=identity, parentOccurrenceId=parent, projectileId=projectile['projectileId'],
        startSeconds=start,endSeconds=end,originUE3Cm=origin,finishUE3Cm=finish,
        directionDegrees=angle,sourceScale=projectile['scale'],elementIds=list(ids.values()))


def build(evidence):
    evidence = evidence.resolve()
    assert evidence == (ROOT/'out/KoukuCircusBalls20260913').resolve()
    contract = read_contract()
    libraries = {}
    for system in SYSTEMS:
        path = AUTHORED/('effect.kouku.source.'+system+'.effect.json')
        contract['sourceInputs'].append(digest(path))
        library = source.read(path)
        assert len(library['elements']) == 1
        element = library['elements'][0]
        assert element['material']['sourceProfile']['enabled']
        active = [m for m in element['sourceRecipe']['modules'] if m['className']=='particlemodulelocationdirect' and module_enabled(m)]
        assert len(active)==1
        contract.setdefault('preservedBounceModules', []).append(copy.deepcopy(active[0]))
        disabled=[m for m in element['sourceRecipe']['modules'] if not module_enabled(m)]
        assert [m['className'] for m in disabled]==(
            ['particlemodulelocationdirect']*3 if system==SYSTEMS[0] else ['particlemodulelifetime'])
        contract.setdefault('activeRecipeProjection',[]).append(dict(sourceSystem=system,
            originalRecipe=copy.deepcopy(element['sourceRecipe']),excludedDisabledModules=copy.deepcopy(disabled),
            activeModulePaths=[m['objectPath'] for m in element['sourceRecipe']['modules'] if module_enabled(m)]))
        libraries[system] = library
    source_defaults=source_vector_defaults(contract,libraries)
    document = copy.deepcopy(libraries[SYSTEMS[0]])
    document.update(version=15,effectAssetId=ASSET,displayName=NAME,elements=[],modelCues=[])
    document['particleSystem'].update(uniformScaleMultiplier=1,yawOffsetDegrees=0,directionYawDegrees=0,initialSpeedMultiplier=1)
    document.pop('sourceAnchorAnimations',None)
    document['sourceModelPreview'] = dict(gateId='GATE1',actorProfileId='MN_RPCT_05',
        targetBossPlacementId='boss.kakulsaydon.g1.saydon',animations=[dict(runtimeClip='rpct00_att_battle_18_03',
            startOffsetMs=0,sourceStartMs=0,playMs=4167,playRate=1,endPolicy='HOLD_LAST_POSE')])
    document['runtimeExtensions']=dict(formatVersion=1,bakedEdgeHistories=[])
    queue = [(0,'circus.ball.0','',0.0,[0,0,0],0.0)]
    occurrences=[]
    while queue:
        generation,identity,parent,start,origin,angle=queue.pop(0)
        projectile=contract['projectiles'][generation]
        ball=append_ball(document,libraries[projectile['sourceSystem']],projectile,identity,parent,start,origin,angle,source_defaults)
        occurrences.append(ball)
        for i,callback in enumerate(projectile['endChildCallbacks']):
            child=identity+str(i)
            direction=preview_direction(child)
            radians=math.radians(direction)
            finish=ball['finishUE3Cm']
            offset=callback['sourceSkillEffect']['AreaOffsetX']
            child_origin=[finish[0]+offset*math.cos(radians),finish[1]+offset*math.sin(radians),finish[2]]
            queue.append((generation+1,child,identity,ball['endSeconds'],child_origin,direction))
    assert len(occurrences)==63 and len(document['elements'])==63
    assert len({e['id'] for e in document['elements']})==63
    paths=set()
    def visit(value):
        if isinstance(value,dict):
            for v in value.values():visit(v)
        elif isinstance(value,list):
            for v in value:visit(v)
        elif isinstance(value,str) and value.startswith('Effect/'):
            assert ':' not in value and '..' not in Path(value).parts
            paths.add(value)
    visit(document)
    assert all((ROOT/'Client/Bin/Resources'/p).is_file() for p in paths)
    assert all(re.fullmatch(r'[A-Za-z0-9_.-]{1,128}',e['id']) and len(e['sourceNode'].encode())<=256 for e in document['elements'])
    # Invariant: source-active projection and measured nested CDO fields are
    # the only recipe changes. Fall/bounce/spin/alpha curves remain unchanged.
    for occurrence,element in zip(occurrences,document['elements']):
        system=contract['projectiles'][occurrence['projectileId']-SOURCE_IDS[0]]['sourceSystem']
        original=libraries[system]['elements'][0]
        expected_recipe=copy.deepcopy(original['sourceRecipe'])
        project_active_recipe(expected_recipe,source_defaults)
        assert element['sourceRecipe']==expected_recipe
        assert element['material']==original['material'] and element['resources']==original['resources']
    json.dumps(document,allow_nan=False)
    validation=dict(elements=63,ballCountsByGeneration=[1,2,4,8,16,32],resources=len(paths),
        nativeMaterials=63,originalMotionRecipesPreserved=True,
        sourceActiveRecipeProjection=True,excludedDisabledModulesByLeaf=[3,1],
        sourceCdoMeshScale=contract['meshStartSizeDefault']['dimensionlessScale'],
        sourceCdoDirectLocationScale=contract['directLocationScaleDefault']['dimensionlessScale'],visiblePreviewSeconds=9,
        originalGameplayTimingRestored=False,visualValidation='USER_PENDING')
    candidate=evidence/'candidate'/(ASSET+'.effect.json')
    source.write(candidate,document)
    source.write(evidence/'source_contract.json',contract)
    source.write(evidence/'occurrences.json',occurrences)
    source.write(evidence/'validation.json',validation)
    source.write(evidence/'installation.json',dict(installed=False,documents=[dict(effectAssetId=ASSET,
        displayName=NAME,path='Data/Effects/Authored/'+candidate.name,durationMs=9000,
        elementCount=63,defaultAnchorKind='MAP',followBoss=False,
        categoryPath=['KoukuSaydon','1관문','패턴','세이튼','세이튼_서커스공 날리기'])],
        sourceActionId=4219806,sourceProjectiles=list(SOURCE_IDS),liveCompositionChanged=False,
        sourceContract='source_contract.json',validation=validation))
    print(json.dumps(dict(candidate=candidate.as_posix(),**validation),ensure_ascii=False))


if __name__=='__main__':
    parser=argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root',type=Path,default=ROOT/'out/KoukuCircusBalls20260913')
    args=parser.parse_args()
    build(args.evidence_root)

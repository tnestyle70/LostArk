"""Stage Encore's original forward eye beam; never install or edit live authoring.

Action4219983 stages 0/1 are one original cast/shot. Source particle, material,
notify and actor identities remain explicit. The second identical action pass
is not concatenated into this reusable Effect resource.
"""
import argparse
import collections
import copy
import hashlib
import math
import sys
from pathlib import Path

import build_kouku_gate1_full_restore as source
from extract_ue3_skeletal_mesh_sockets import parse_socket_contract

ROOT = source.ROOT
ASSET = 'effect.kouku.bingo.encore.blackhole.beam.full.restore'
NAME = '빙고 | 앵콜세이튼 | 블랙홀빔'
ACTION = source.SOURCE / 'RemainingCharacterExtraction-20260829/ActionNameSources/MN_RPCT_07.action-effects.json'
SOCKETS = source.SOURCE / 'CanonicalSource/Character/UModelExports/MN_RPCT_05/Export/MN_RPCT_05/mesh/mn_rpct_05_sk.props.txt'
MODEL = ROOT / 'Client/Bin/Resources/Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel'
BASIS = ROOT / 'out/KoukuGate3BossAssembly20260912/body_socket_basis_evidence.json'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def restore_nested_raw_defaults(doc, evidence):
    """Inherit only cooked fields omitted by an explicit Distribution=None delta."""
    records = source.read(evidence/'source_module_inputs.json')['records']
    defaults = {r['fullPath']: r for r in source.read(evidence/'source_class_defaults.json')['records']}
    repairs = []
    fields = {'op':'operation', 'lookuptablenumelements':'lookupTableNumElements',
              'lookuptablechunksize':'lookupTableChunkSize', 'lookuptabletimescale':'lookupTableTimeScale',
              'lookuptablestarttime':'lookupTableStartTime', 'lookuptable':'lookupTable'}
    for element in doc['elements']:
        for module in element['sourceRecipe']['modules']:
            instance = records[module['objectPath']]
            cdo = defaults.get('engine.default__'+module['className'])
            if not cdo:
                continue
            cdo_props = {k.lower():v for k,v in cdo['properties'].items()}
            own_props = {k.lower():v for k,v in instance['properties'].items()}
            for dist in module['distributions']:
                if dist['lookupTable'] or dist['keys']:
                    continue
                # Required.SpawnRate is the legacy null CDO descriptor. The
                # admitted Spawn module owns emission; keep the null contract.
                if module['className']=='particlemodulerequired' and dist['propertyPath']=='spawnrate':
                    continue
                key=dist['propertyPath'];raw=own_props.get(key);base=cdo_props.get(key)
                if not raw or not base or not str(raw.get('structType','')).lower().startswith('rawdistribution'):
                    continue
                delta={k.lower():v for k,v in raw['value']['properties'].items()}
                inherited={k.lower():v for k,v in base['value']['properties'].items()}
                if set(delta)!={'distribution'} or delta['distribution']['value']!=0 or 'lookuptable' not in inherited:
                    continue
                assert not instance.get('archetypeFullPath'), 'A template override needs its own exact inheritance chain'
                table=inherited['lookuptable']['value']
                if not table:
                    continue
                before=copy.deepcopy(dist)
                dist.update({target:copy.deepcopy(inherited[src]['value']) for src,target in fields.items()})
                repairs.append(dict(elementId=element['id'],sourceModule=module['objectPath'],
                    propertyPath=key,sourceInstanceExport=instance['exportIndex'],
                    sourceDelta=delta,sourceClassDefault=cdo['fullPath'],sourceClassDefaultSerialSha256=cdo['serialSha256'],
                    before=before,inheritedFields={target:dist[target] for target in fields.values()}))
    assert repairs, 'Expected source AlphaScaleOverLife CDO inheritance is absent'
    source.write(evidence/'nested-cdo-projections.json',repairs)


def model_sockets(evidence):
    sys.path[:0] = [str(ROOT / 'Tools/CharacterCustomizing'), str(ROOT / 'Tools/ModelAssetConverter')]
    from align_rig_gltf import read_body_skeleton
    from retime_wmodel_from_psa import read_wmodel_animation_sections
    bones = dict(read_body_skeleton(MODEL))
    clips = {c['name'] for c in read_wmodel_animation_sections(MODEL.read_bytes())}
    assert {'bip001-l_eye', 'bip001-r_eye', 'bip001-spine2'} <= bones.keys()
    assert {'rpct00_att_battle_35_01', 'rpct00_att_battle_35_04'} <= clips
    measured = source.read(BASIS)
    assert next(s['sha256'] for s in measured['sources'] if s['path'].endswith('MN_RPCT_05.wmodel')) == sha(MODEL)
    maps = {r['bone']: r for r in measured['boneBindMaps']}
    for bone in ('bip001-l_eye', 'bip001-r_eye', 'bip001-spine2'):
        assert maps[bone]['maxErrorFromPoint01ReflectZ'] < 1e-6
    sockets = parse_socket_contract(SOCKETS)
    sockets['sockets'] += [dict(socketName='', boneName=bone) for bone in bones]
    source.write(evidence / 'source_socket_contract.raw.json', sockets)
    state = next(s for s in sockets['sockets'] if s.get('socketName', '').lower() == 'fx_state_01')
    assert state['boneName'] == 'bip001-spine2'
    assert state['sourceTransform'] == dict(positionUeUnits=[10.9597, -18.7649, 0.0], rotationUnrealUnits=[0.0]*3, scale=[1.0]*3)
    # Particle UE cm -> Client (X,Z,-Y) metres, followed by this cooked rig's
    # measured .01*ReflectZ PSK bind map and UModel's UE->PSK ReflectY.
    # Canonical Rx(-90) is this composition, not a global direction tweak.
    state['runtimeLocalTransform'] = dict(position=[.109597, .187649, 0], rotationDegrees=[-90, 0, 0], scale=[1]*3)
    source.write(evidence / 'source_socket_contract.json', sockets)
    return dict(modelAssetId=MODEL.relative_to(ROOT/'Client/Bin/Resources').as_posix(),
                modelSha256=sha(MODEL), sourceProfile='MN_RPCT_07', actorProfile='MN_RPCT_05',
                installedClips=sorted(clips), installedBones=sorted(bones),
                basisEvidence=str(BASIS), basisEvidenceSha256=sha(BASIS))


def build(evidence):
    evidence.mkdir(parents=True, exist_ok=True)
    model = model_sockets(evidence)
    source.write(evidence/'model-contract.json', model)
    original_action, original_selected = source.ACTION, source.SELECTED
    source.ACTION, source.SELECTED = ACTION, {4219983: ([0, 1], NAME)}
    try:
        index, notifies, occurrences, records = source.acquire(evidence)
        assert len(notifies) == 5 and len(occurrences) == 39
        material_inputs, programs = {}, []
        for system in sorted({o['sourceSystem'] for o in occurrences}):
            path = ROOT/'Data/Effects/Authored'/('effect.kouku.source.'+system+'.effect.json')
            template = source.read(path)
            material_inputs[path] = sha(path)
            materials = {(e['sourcePresentation']['sourceObjectPath'], e['material']['sourceMaterialPath']): e['material'] for e in template['elements']}
            for occurrence in (o for o in occurrences if o['sourceSystem'] == system and o['kind'] != 'light'):
                material = materials[(occurrence['sourceEmitter'], occurrence['sourceMaterial'])]
                assert material['sourceProfile']['enabled']
                programs.append(dict(occurrences=[occurrence['elementId']], material=material))
        patch = evidence/'native_material_patch.json'
        source.write(patch, dict(programs=programs))
        preview = source.source_model_preview(ACTION, 4219983, [0, 1], 'GATE3', 'MN_RPCT_05', 'boss.kakulsaydon.g3.saydon')
        source.project(evidence, index, notifies, occurrences, records, evidence/'projected',
                       material_patch=patch, source_model_previews={4219983: preview})
    finally:
        source.ACTION, source.SELECTED = original_action, original_selected
    doc = source.read(evidence/'projected/effect.kouku.gate1.4219983.full.restore.effect.json')
    doc.update(effectAssetId=ASSET, displayName=NAME)
    restore_nested_raw_defaults(doc,evidence)
    groups = collections.Counter()
    notify_windows = {n['notifyId']: n['durationSeconds'] for n in notifies}
    window_projections = []
    for e in doc['elements']:
        anchor = e['actionCueAttachment']
        if anchor['runtimeBoneName'] in ('bip001-l_eye', 'bip001-r_eye'):
            assert anchor['socketLocalTransform'] == dict(position=[0.0]*3, rotationDegrees=[0.0]*3, scale=[1.0]*3)
            anchor['socketLocalTransform']['rotationDegrees'] = [-90, 0, 0]
            # Notify's original UE scale is (1,1,1.2). Runtime local axes are
            # (UE.X,UE.Z,-UE.Y), so only this anisotropic notify permutes Y/Z.
            scale = e['detail']['transform']['scale']
            assert scale == [1.0, 1.0, 1.2000000476837158]
            e['detail']['transform']['scale'] = [scale[0], scale[2], scale[1]]
        recipe = e['sourceRecipe']
        window = notify_windows[e['sourcePresentation']['sourceEventId']]
        if window > 0 and recipe['emitterDurationSeconds'] > window:
            # Existing loop0 admission consumes the explicit Timing window.
            # The original period is longer than that window, so this never
            # creates an additional loop and leaves emitter-time curves intact.
            assert recipe['emitterLoopCount'] == 1 and recipe['emitterDelaySeconds'] == 0
            recipe['emitterLoopCount'] = 0
            window_projections.append(dict(elementId=e['id'], sourceEmitterLoopCount=1,
                sourceEmitterDurationSeconds=recipe['emitterDurationSeconds'],
                sourceNotifyDurationSeconds=window, policy='ORIGINAL_NOTIFY_STOPS_EMISSION_BEFORE_FIRST_LOOP_END'))
        if e['kind'] == 'light':
            # One typed light owns the one original burst particle. Its 10s PS
            # activation period is not the 1.2s light-particle lifetime.
            assert recipe['emitterLoopCount'] == 1 and len(recipe['bursts']) == 1
            assert recipe['bursts'][0]['countMinimum'] == recipe['bursts'][0]['countMaximum'] == 1
            recipe['emitterDurationSeconds'] = e['detail']['timing']['lifeTimeSeconds']
        if e['kind'] == 'screenPost':
            assert e['material']['sourceProfile']['runtimeShaderProfileId']=='effect.ue3.kouku-3328-native.v1'
            assert e['detail']['particle']['lifeTimeSeconds']==[0.30000001192092896]*2
            # The native post material still needs the ordinary evaluated-post
            # carrier. The raw library had left that typed carrier disabled.
            e['detail']['screenPost'].update(enabled=True,profileId='screen.zoom-blur.reconstructed.v1',
                intensity=1,secondaryIntensity=0,frequency=1,tint=[1,1,1,1])
            e['detail']['timing']['lifeTimeSeconds']=e['detail']['particle']['lifeTimeSeconds'][0]
            recipe['emitterDurationSeconds']=e['detail']['timing']['lifeTimeSeconds']
        cue = e['sourcePresentation']['sourceEventId'].rsplit('/', 1)[-1]
        e['groupId'] = ASSET+'.'+cue+'.'+anchor['runtimeAnchorSlotId'].lower()
        groups[e['groupId']] += 1
    assert len(doc['elements']) == 60 and len({e['id'] for e in doc['elements']}) == 60
    assert len(window_projections) == 10
    source.write(evidence/'notify-window-projections.json', window_projections)
    by_bone = collections.Counter(e['actionCueAttachment']['runtimeBoneName'] for e in doc['elements'])
    assert by_bone == {'bip001-spine2': 16, 'bip001-l_eye': 21, 'bip001-r_eye': 21, '': 2}, by_bone
    assert len(NAME.encode('utf-8')) < 64
    target = ROOT/'Data/Effects/Authored'/(ASSET+'.effect.json')
    candidate = evidence/'candidate'/target.name
    source.write(candidate, doc)
    def end_seconds(e):
        r,t=e['sourceRecipe'],e['detail']['timing']
        active=r['emitterDurationSeconds']*r['emitterLoopCount'] if r['emitterLoopCount'] else t['lifeTimeSeconds']
        tail=max(e['detail']['particle']['lifeTimeSeconds']) if e['kind']=='particle' else 0
        return t['startDelaySeconds']+r['emitterDelaySeconds']+active+t['afterImageSeconds']+tail
    duration_ms = math.ceil(1000*max(map(end_seconds,doc['elements'])))
    before = {p.relative_to(ROOT).as_posix(): sha(p) for p in material_inputs}
    for path in ('Data/Effects/EffectCatalog.json', 'Data/Effects/EffectResourceTree.json',
                 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'):
        p = ROOT/path
        if p.is_file(): before[path] = sha(p)
    row = dict(effectAssetId=ASSET, candidatePath=str(candidate.resolve()), path=target.relative_to(ROOT).as_posix(),
               beforeSha256=sha(target) if target.is_file() else None, candidateSha256=sha(candidate), displayName=NAME,
               durationMs=duration_ms, defaultAnchorKind='BOSS', followBoss=True,
               categoryPath=['KoukuSaydon','빙고','앵콜세이튼'], elementCount=60)
    source.write(evidence/'installation.json', dict(installed=False, documents=[row], inputHashes=before,
        sourceInputs=[dict(path=str(p), sha256=sha(p)) for p in (ACTION, SOCKETS, MODEL, BASIS)],
        sourceActionId=4219983, sourceStageIndices=[0,1], groups=dict(groups),
        selectedPassPolicy='ONE_ORIGINAL_CAST_AND_SHOT; identical stages2/3 are excluded',
        manualVisualValidation='USER_PENDING'))
    # Parent assigns the final stable pattern/resource/occurrence identities.
    source.write(evidence/'pattern-template.json', dict(displayName=NAME, gateId='GATE3', actorProfileId='MN_RPCT_05',
        sourceProfileId='MN_RPCT_07', sourceActionId=4219983, sourceStageIndices=[0,1],
        durationMs=max(3500,duration_ms), animationDurationMs=3500,
        animations=copy.deepcopy(preview['animations']), effects=[dict(effectAssetId=ASSET,startMs=0,durationMs=duration_ms,
            anchorKind='BOSS',followBoss=True,positionOffset=[0,0,0],rotationOffsetDegrees=[0,0,0],scale=[1,1,1])]))
    print(dict(asset=ASSET,elements=60,durationMs=duration_ms,installed=False))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, default=ROOT/'out/KoukuEncoreBlackhole20260914')
    build(parser.parse_args().evidence_root)

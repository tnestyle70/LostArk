"""Project original RPCT07 Showtime ParticleSystems as independent V1 groups.

Each leaf preserves the source first LOD and native material. It is an isolated
library item: a Workbench occurrence owns placement, anchor and playback. Action
and Projectile references are evidence, not invented sequential gameplay logic.
"""
from pathlib import Path
import argparse
import copy
import json
import math
import sys
import struct

import build_kouku_gate1_full_restore as source
from build_action_cue_recipe import ue3_axis_scale_to_client

ROOT = source.ROOT
PREFIX = 'fx_mn_rpct_07_v.'
ROWS = [
    ('gun.create', 'Gun Create', 'par_v_rpct_07_bazooka', 2),
    ('gun.loop', 'Gun Loop', 'par_v_rpct_07_bazooka_loop', 1),
    ('gun.end', 'Gun End', 'par_v_rpct_07_bazooka_end', 1),
    ('gun.shell', 'Gun Shell', 'par_v_rpct_bazooka_mesh_01', 1),
    ('gun.muzzle', 'Gun Muzzle', 'par_v_rpct_signalshot_01_loc_int', 11),
    ('gun.signature', 'Gun Signature', 'par_v_rpct_signature_01_1_loc_int', 11),
    ('gun.ground', 'Gun Ground Mark', 'par_v_rpct_gun_shot_decal_01_loc_int', 3),
    ('airstrike.impact', 'Airstrike Impact', 'par_v_rpct_airstrike_exp_01_loc_int', 16),
    ('airstrike.missile', 'Airstrike Missile', 'par_v_rpct_airstrike_missile_02', 1),
    ('ball.impact', 'Ball Impact', 'par_v_rpct_atk_exp_02_loc_int', 15),
    ('ball.red', 'Red Ball Burst', 'par_v_rpct_ballread_exp_01_loc_int', 11),
    ('circle.impact01', 'Circle Impact 01', 'par_v_rpct_magiccircle_prj_exp_01_loc_int', 13),
    ('circle.impact03', 'Circle Impact 03', 'par_v_rpct_magiccircle_prj_exp_03_1_loc_int', 14),
    ('ball.drop', 'Ball Drop', 'par_v_rpct_missiledrop_01_loc_int', 5),
    ('napalm', 'Napalm Ground', 'par_v_rpct_napalm_area_01_loc_int', 7),
    ('target.fixed', 'Fixed Target', 'par_v_rpct_realtarget_01_loc_int', 4),
    ('target.tracking', 'Tracking Target', 'par_v_rpct_realtarget_02_loc_int', 3),
    ('target.end', 'Target End', 'par_v_rpct_realtarget_end_01_loc_int', 3),
    ('fire.impact', 'Round Fire Impact', 'par_v_rpct_round_fire_exp_01_loc_int', 11),
    ('bullet', 'Target Bullet', 'par_v_target_shot_01_loc_int', 7),
]
TARGETS = {i: dict(asset='effect.kouku.gate3.showtime.' + slug,
                  name='Kouku G3 Showtime ' + name, system=PREFIX + system,
                  count=count) for i, (slug, name, system, count) in enumerate(ROWS, 1)}


def source_model_calls(evidence):
    from extract_ue3_skeletal_mesh_sockets import parse_socket_contract
    from build_action_cue_recipe import decode_typed_payload
    sockets_path = source.SOURCE / 'CanonicalSource/Character/UModelExports/MN_RPCT_05/Export/MN_RPCT_05/mesh/mn_rpct_05_sk.props.txt'
    sockets = parse_socket_contract(sockets_path)
    sys.path[:0] = [str(ROOT / 'Tools/CharacterCustomizing'), str(ROOT / 'Tools/ModelAssetConverter')]
    from align_rig_gltf import read_body_skeleton
    from retime_wmodel_from_psa import read_wmodel_animation_sections
    model_path = ROOT / 'Client/Bin/Resources/Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel'
    bones = dict(read_body_skeleton(model_path))
    clips = {r['name'] for r in read_wmodel_animation_sections(model_path.read_bytes())}
    document = source.read(source.SOURCE / 'RemainingCharacterExtraction-20260829/ActionNameSources/MN_RPCT_07.action-effects.json')
    calls = {}
    for ordinal in range(1, 8):
        target = TARGETS[ordinal]
        for action_id in (4219939, 4219912):
            action = next(a for a in document['actions'] if a['actionId'] == action_id)
            for stage in action['stages']:
                selected = []
                if not stage['animationClips']:
                    continue
                for notify in stage['notifies']:
                    if notify['sourceType'] != 'PlayParticleEffect' or not any(
                        r['objectPath'].lower() == target['system'] for r in notify['assetReferences']):
                        continue
                    cue = decode_typed_payload(notify['sourceType'], notify['serializedPayload'], sockets,
                                               notify['assetReferences'], notify['serializedLabels'])
                    attachment = cue['attachment']
                    for anchor in attachment.get('runtimeAnchors', []):
                        name = anchor['sourceAnchorName'].lower()
                        if anchor['resolutionStatus'] == 'MISSING_SOURCE_SOCKET' and name in bones:
                            anchor.update(runtimeBoneName=name, resolutionStatus='EXACT_SOURCE_BONE')
                    anchors = attachment.get('runtimeAnchors', [])
                    if anchors:
                        attachment['runtimeBoneName'] = anchors[0]['runtimeBoneName']
                        attachment['runtimeResolutionStatus'] = anchors[0]['resolutionStatus']
                    if cue['enabled']:
                        selected.append(dict(notify=notify, cue=cue))
                if selected:
                    assert 'rpct00_' + stage['animationClips'][0]['clipName'].lower() in clips
                    calls[ordinal] = dict(sourceActionId=action_id, sourceStage=stage['stageIndex'],
                        clip=stage['animationClips'][0], calls=selected)
                    break
            if ordinal in calls:
                break
    source.write(evidence / 'source_model_calls.json', calls)
    used = sorted({a['runtimeBoneName'] for m in calls.values() for c in m['calls']
                   for a in c['cue']['attachment'].get('runtimeAnchors', []) if a['runtimeBoneName']})
    source.write(evidence / 'installed_model_attachment_checks.json', dict(modelAssetId=model_path.relative_to(ROOT / 'Client/Bin/Resources').as_posix(),
        bones=len(bones), clips=len(clips), usedBones=[dict(name=name, parentIndex=bones[name]) for name in used],
        status='INSTALLED_WMODEL_SKELETON_AND_EMBEDDED_CLIP_CHECK', clientRun=False, manualVisualValidation='USER_PENDING'))
    return calls


def bind_source_providers(document, index, occurrences):
    by_id = {o['elementId']: o for o in occurrences}
    providers = {}
    for element in document['elements']:
        occurrence = by_id[element['id']]
        props = index.objects[occurrence['sourceEmitter']].properties
        name = source.imported.prop(props, 'emittername', occurrence['sourceEmitter'].rsplit('.', 1)[-1])
        providers.setdefault(name, []).append(element['id'])
    requested_names = {v['value'] for e in document['elements'] for m in e['sourceRecipe']['modules']
        if 'locationemitter' in m['className'] for v in m['literals'] if v['propertyPath'] == 'emittername'}
    for element in document['elements']:
        occurrence = by_id[element['id']]
        recipe = element['sourceRecipe']
        props = index.objects[occurrence['sourceEmitter']].properties
        mode = source.imported.prop(props, 'emitterrendermode', 'erm_normal')
        locations = [m for m in recipe['modules'] if m['className'] in
            ('particlemodulelocationemitter', 'efparticlemodulelocationemitter',
             'particlemodulelocationemitterdirect', 'efparticlemodulelocationemitterdirect')]
        emitter_name = source.imported.prop(props, 'emittername', occurrence['sourceEmitter'].rsplit('.', 1)[-1])
        if mode in ('erm_point', 'erm_none') or locations or emitter_name in requested_names:
            recipe['particleSystemOccurrenceId'] = occurrence['sourceNotify']
            recipe['emitterName'] = emitter_name
        if mode in ('erm_point', 'erm_none'):
            recipe['simulationOnly'] = True
            required = next(m for m in recipe['modules'] if m['className'] == 'particlemodulerequired')
            required['literals'].append(dict(propertyPath='source.emitterrendermode', kind='string', value=mode))
        for location in locations:
            name = next(v['value'] for v in location['literals'] if v['propertyPath'] == 'emittername')
            if name in providers:
                assert len(providers[name]) == 1, (document['effectAssetId'], location['objectPath'], name, providers[name])
                location['literals'].append(dict(propertyPath='runtime.providerelementid', kind='string', value=providers[name][0]))
            else:
                location['literals'].append(dict(propertyPath='runtime.sourceprovidermissing', kind='boolean', value=True))
            if not location['className'].endswith('direct') and not any(v['propertyPath'] == 'selectionmethod' for v in location['literals']):
                location['literals'].append(dict(propertyPath='selectionmethod', kind='string', value='elesm_random'))


def patch_simulation_providers(evidence, material_patch, index, occurrences):
    if not material_patch:
        return material_patch
    patch = source.read(material_patch)
    bound = {identity for program in patch['programs'] for identity in program['occurrences']}
    hidden = []
    for occurrence in occurrences:
        if occurrence['elementId'] in bound or occurrence['sourceMaterial'] != 'enginematerials.defaultparticle':
            continue
        props = index.objects[occurrence['sourceEmitter']].properties
        mode = source.imported.prop(props, 'emitterrendermode', 'erm_normal')
        if mode in ('erm_none', 'erm_point'):
            hidden.append(occurrence['elementId'])
    if hidden:
        originals = [p for p in patch['programs']
            if p.get('sourceMaterial') == 'enginematerials.defaultparticle' and p.get('rendererShape') == 'sprite']
        if originals:
            assert len(originals) == 1, 'Original default-particle native material is ambiguous'
            originals[0]['occurrences'] += hidden
        else:
            # A no-draw simulation provider has no GPU material consumer. This
            # branch only represents actual source ERM_None/Point emitters.
            patch['programs'].append(dict(program=None, occurrences=hidden,
                material=dict(templateId='effect.standard', sourceMaterialPath='enginematerials.defaultparticle',
                    renderProfile='alpha_two_sided_depth_read', sourceProfile=dict(enabled=False))))
    destination = evidence / 'native_material_with_providers.json'
    source.write(destination, patch)
    return destination


def projectile_leaf_parameters(evidence):
    """Read the same CEFParticleData layout qualified by the fire-grid decoder.

    A leaf has no projectile route. Only its original ParticleSystem parameter
    values and local scale are imported; occurrence placement stays editable.
    """
    from extract_action_effect_notifies import scan_length_prefixed_strings
    rows = {}
    for identifier in (421991201, 421991203, 421991205, 421991206, 421991207, 421991208, 421991209, 421991210):
        path = ROOT / 'out/KoukuShowtimeInventory20260911/source' / (str(identifier) + '.loa')
        raw = path.read_bytes()
        for token in scan_length_prefixed_strings(raw, 0, len(raw)):
            if not token['value'].startswith("ParticleSystem'"):
                continue
            system = token['value'].split("'")[1].lower()
            if not any(t['system'] == system for t in TARGETS.values()):
                continue
            end = token['sourceOffset'] + 4 + len(token['value']) + 1
            scale = list(struct.unpack_from('<3f', raw, end + 136))
            assert all(math.isfinite(v) and v > 0 for v in scale)
            cursor = end + 148
            count = struct.unpack_from('<i', raw, cursor)[0]
            cursor += 4
            assert 0 <= count <= 6
            parameters = []
            for _ in range(count):
                length = struct.unpack_from('<i', raw, cursor)[0]
                assert 0 < length < 256 and raw[cursor + 3 + length] == 0
                name = raw[cursor + 4:cursor + 3 + length].decode('ascii')
                cursor += 4 + length
                kind = struct.unpack_from('<i', raw, cursor)[0]
                assert kind in (0, 1, 3)
                if kind == 1:
                    parameters.append(dict(name=name, type='scalar', scalarValue=struct.unpack_from('<f', raw, cursor + 4)[0]))
                elif kind == 3:
                    parameters.append(dict(name=name, type='vector', vectorValue=list(struct.unpack_from('<3f', raw, cursor + 12))))
                sentinel = cursor + 56
                length = struct.unpack_from('<i', raw, sentinel)[0]
                assert raw[sentinel + 4:sentinel + 4 + length] == b'None\0'
                cursor = sentinel + 4 + length + 4
            value = dict(sourceProjectileId=identifier, sourceParticleByteOffset=token['sourceOffset'],
                         parameterOverrides=parameters, sourceScale=scale)
            if system in rows:
                assert rows[system]['parameterOverrides'] == parameters and rows[system]['sourceScale'] == scale
            else:
                rows[system] = value
    source.write(evidence / 'source_projectile_leaf_parameters.json', rows)
    return rows


def acquire(evidence):
    actions = []
    for ordinal, target in TARGETS.items():
        cue = dict(enabled=True, particleDataDecoded=True, parameterOverridesDecoded=True,
                   sourceKind='ORIGINAL_PARTICLE_SYSTEM_LIBRARY_PREVIEW', parameterOverrides=[],
                   attachment=dict(mode='SNAPSHOT_ROOT', sourceAnchorNames=[], runtimeAnchors=[],
                       runtimeAnchorSlotId='root', runtimeBoneName='',
                       runtimeResolutionStatus='EXACT_ROOT_SNAPSHOT',
                       socketLocalTransform=dict(position=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1])),
                   localTransform=dict(position=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1]))
        notify = dict(notifyId='source-system/' + target['system'] + '/library-root',
                      sourceType='PlayParticleEffect', localTimeSeconds=0, durationSeconds=0,
                      serializedPayload=cue, serializedLabels=[],
                      assetReferences=[dict(className='ParticleSystem', objectPath=target['system'])])
        actions.append(dict(actionId=ordinal, sourceKind=cue['sourceKind'],
                            stages=[dict(stageIndex=0, animationClips=[], notifies=[notify])]))
    selected = evidence / 'selected_source_actions.json'
    source.write(selected, dict(sourceKind='ORIGINAL_PARTICLE_SYSTEM_LIBRARY_PREVIEW', actions=actions))
    source.ACTION = selected
    source.SELECTED = {i: ([0], target['name']) for i, target in TARGETS.items()}
    original = source.decode_typed_payload
    def decode(kind, payload, *args):
        if isinstance(payload, dict) and payload.get('sourceKind') == 'ORIGINAL_PARTICLE_SYSTEM_LIBRARY_PREVIEW':
            return copy.deepcopy(payload)
        return original(kind, payload, *args)
    source.decode_typed_payload = decode
    from build_kouku_gate3_rainbow_native import UMODEL, RELEASE
    from extract_ue3_placements import resolve_physical_package
    result = source.acquire(evidence, lambda name: resolve_physical_package(UMODEL, RELEASE, name, 'kr'))
    for ordinal, target in TARGETS.items():
        actual = sum(o['actionId'] == ordinal for o in result[2])
        expected = target['count']
        if target.get('countIncludesEmpty', False):
            notify_id = 'source-system/' + target['system'] + '/library-root'
            actual += sum(o['sourceNotify'] == notify_id for o in source.read(evidence / 'source_empty_emitters.json'))
            if expected is None:
                expected = sum(p == 'emitters' for p, _ in result[0].objects[target['system']].references)
        assert actual == expected, (target['system'], actual, expected)
    return result


def restore_ball_drop_source_defaults(document):
    """Restore only the two source CDO vector fields lost by the leaf projection."""
    import hashlib
    assert document['effectAssetId'] == 'effect.kouku.gate3.showtime.ball.drop'
    assert len(document['elements']) == 5
    base = ROOT / 'out/KoukuAllEffects20260912'
    defaults_path, instances_path = base / 'source_class_defaults.json', base / 'source_module_inputs.json'
    defaults = source.read(defaults_path)['records']
    instances = source.read(instances_path)['records']
    changes = []
    for emitter, module_name, class_name, property_name in (
        ('particlespriteemitter_38', 'particlemodulelocationdirect_0', 'particlemodulelocationdirect', 'ScaleFactor'),
        ('particlespriteemitter_4', 'particlemodulesize_3', 'particlemodulesize', 'StartSize')):
        prefix = 'fx_mn_rpct_07_v.par_v_rpct_missiledrop_01_loc_int.'
        elements = [e for e in document['elements'] if e['sourceNode'].endswith('|' + prefix + emitter)]
        assert len(elements) == 1
        element = elements[0]
        module = next(m for m in element['sourceRecipe']['modules'] if m['objectPath'] == prefix + module_name)
        assert module['className'] == class_name
        assert next((v['value'] for v in module['literals'] if v['propertyPath'] == 'benabled'), True)
        instance = instances[module['objectPath']]['properties'][property_name.lower()]
        assert instance['structType'] == 'rawdistributionvector'
        delta = instance['value']['properties']
        assert list(delta) == ['distribution'] and delta['distribution']['value'] == 0
        cdo = next(r for r in defaults if r['fullPath'] == 'engine.default__' + class_name)
        inherited = cdo['properties'][property_name]['value']['properties']
        assert inherited['LookupTable']['value'] == [1.] * 8
        expected = dict(operation=inherited['Op']['value'],
            lookupTableNumElements=inherited['LookupTableNumElements']['value'],
            lookupTableChunkSize=inherited['LookupTableChunkSize']['value'],
            lookupTableTimeScale=inherited['LookupTableTimeScale']['value'],
            lookupTableStartTime=inherited['LookupTableStartTime']['value'],
            lookupTable=inherited['LookupTable']['value'])
        distribution = next(d for d in module['distributions'] if d['propertyPath'] == property_name.lower())
        assert distribution['componentCount'] == 3 and not distribution['keys']
        assert not distribution['lookupTable'] or all(distribution[k] == v for k, v in expected.items()), 'Preserve a changed authored distribution'
        before = copy.deepcopy(distribution)
        distribution.update(copy.deepcopy(expected))
        changes.append(dict(elementId=element['id'], sourceModule=module['objectPath'], property=property_name,
            sourceInstanceDelta=delta, sourceCDO=cdo['fullPath'], before=before, after=copy.deepcopy(distribution)))
    return dict(changes=changes, inputHashes={p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
        for p in (defaults_path, instances_path)}, sourceKind='SOURCE_CDO_NESTED_RAW_DISTRIBUTION_INHERITANCE')


def stage_ball_drop_source_defaults(evidence):
    """Stage the current leaf without replacing user transform or provider edits."""
    import hashlib
    evidence = evidence.resolve()
    assert evidence.is_relative_to((ROOT / 'out').resolve())
    path = ROOT / 'Data/Effects/Authored/effect.kouku.gate3.showtime.ball.drop.effect.json'
    before = path.read_bytes()
    document = json.loads(before.decode('utf-8-sig'))
    contract = restore_ball_drop_source_defaults(document)
    relative = path.relative_to(ROOT).as_posix()
    baseline = evidence / 'baseline' / relative
    baseline.parent.mkdir(parents=True, exist_ok=True)
    baseline.write_bytes(before)
    candidate = evidence / 'candidate' / relative
    source.write(candidate, document)
    contract['inputHashes'][relative] = hashlib.sha256(before).hexdigest()
    source.write(evidence / 'installation.json', dict(sourceWritten=False, stageOnly=True,
        documents=[dict(effectAssetId=document['effectAssetId'], displayName=document['displayName'],
            path=relative, candidatePath=candidate.relative_to(ROOT).as_posix(), durationMs=2000,
            defaultAnchorKind='MAP', candidateSha256=hashlib.sha256(candidate.read_bytes()).hexdigest())],
        preserved='Every field outside the two inherited distribution payloads, including user TRS and provider links',
        manualVisualValidation='USER_PENDING', **contract))
    print('Staged original ball size and provider path defaults; no Data writes')


def stage_bomb_fuse(evidence):
    """Stage the original skull-bomb fuse in its installed FBX bone frame."""
    from extract_ue3_skeletal_mesh_sockets import parse_socket_contract
    import hashlib

    evidence = evidence.resolve()
    assert evidence.is_relative_to((ROOT / 'out').resolve()), 'Evidence must remain under out'
    leaf_path = ROOT / 'Data/Effects/Authored/effect.kouku.source.fx_mn_rhcn_01.par_x_rhcn_saprkloop_01.effect.json'
    world_path = ROOT / 'Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json'
    socket_path = source.SOURCE / 'WorldObjectExtraction-20260907/WorldProps/MN_RHCN_01/mesh/mn_rhcn_01_sk.props.txt'
    socket = next(s for s in parse_socket_contract(socket_path)['sockets'] if s['socketName'] == 'fx_01')
    assert socket['boneName'] == 'b_body'
    position = socket['runtimeLocalTransform']['position']
    assert abs(position[0] - .2) < 1.e-8 and position[1] == 0 and abs(position[2] - .521496) < 1.e-8
    assert socket['runtimeLocalTransform']['rotationDegrees'] == [0, 0, 0]
    assert socket['runtimeLocalTransform']['scale'] == [1, 1, 1]
    leaf_raw, world_raw = leaf_path.read_bytes(), world_path.read_bytes()
    leaf, world = json.loads(leaf_raw), json.loads(world_raw)
    asset = 'effect.kouku.gate3.showtime.bomb.fuse'
    candidate = copy.deepcopy(leaf)
    assert len(candidate['elements']) == 3 and not candidate['modelCues']
    candidate.update(effectAssetId=asset, displayName='쇼타임_해골 폭탄_심지 불꽃')
    cdo_path = ROOT / 'out/KoukuAllEffects20260912/source_class_defaults.json'
    raw_path = ROOT / 'out/KoukuAllEffects20260912/source_module_inputs.json'
    cdo = next(r for r in source.read(cdo_path)['records'] if r['fullPath'] == 'engine.default__particlemodulecolor')
    original = source.read(raw_path)['records']['fx_mn_rhcn_01.par_x_rhcn_saprkloop_01.particlemodulecolor_0']
    for element in candidate['elements']:
        element['id'] = asset + '.' + element['id'].rsplit('.', 1)[-1]
        element['groupId'] = asset
        assert not element['actionCueAttachment']['enabled'] and not element['transformInheritance']['enabled']
        # This alias sustains the original emitters within the source notify's
        # two-second window. It does not change the shared one-second library leaf.
        element['detail']['timing']['lifeTimeSeconds'] = 2.0
        element['sourceRecipe']['emitterLoopCount'] = 0
        for module in element['sourceRecipe']['modules']:
            if module['objectPath'] != original['fullPath']:
                continue
            for distribution in module['distributions']:
                name = distribution['propertyPath']
                assert name in ('startcolor', 'startalpha')
                assert not distribution['lookupTable'] and not distribution['keys']
                delta = original['properties'][name]['value']['properties']
                assert set(delta) == {'distribution'} and delta['distribution']['value'] == 0
                values = cdo['properties']['StartColor' if name == 'startcolor' else 'StartAlpha']['value']['properties']
                for key, src in [('operation', 'Op'), ('lookupTableNumElements', 'LookupTableNumElements'),
                                 ('lookupTableChunkSize', 'LookupTableChunkSize'), ('lookupTableTimeScale', 'LookupTableTimeScale'),
                                 ('lookupTableStartTime', 'LookupTableStartTime'), ('lookupTable', 'LookupTable')]:
                    distribution[key] = copy.deepcopy(values[src]['value'])
    object_id = 'world.object.kouku.bingo_bomb'
    resource = next(r for r in world['objectResources'] if r['objectId'] == object_id)
    assert resource['modelAssetId'] == 'Character/KoukuSaton/MN_RHCN_01/MN_RHCN_01.wmodel' and resource['animated']
    assert abs(resource['modelPreScale'] - .01) < 1.e-8
    model_path = ROOT / 'Client/Bin/Resources' / resource['modelAssetId']
    assert hashlib.sha256(model_path.read_bytes()).hexdigest() == '59c0e0c0ccfa6d05e413b52514badbb7a603f178f96935c9eb92a970168b8cde', 'Re-audit the installed bomb socket basis after model replacement'
    selected = ['sequence.LV_LUT_MIDNIGHTC_ED.world_object.bingo_bomb',
                'sequence.LV_LUT_MIDNIGHTC_ED.world_object.bingo_bomb.native.bomb_respawn_1']
    modified = copy.deepcopy(world)
    for template in modified['templates']:
        if template['sequenceId'] not in selected:
            continue
        assert template['durationMs'] == 2000 and not template.get('effectTracks')
        # Source particle XYZ is already converted by Playback. The installed
        # Blender/FBX bone local Z points down; -90 X restores its effect frame.
        template['effectTracks'] = [dict(effectTrackId='effect.showtime.bomb.fuse', slotId='object',
            resourceKind='V1_EFFECT', resourceId=asset, timing='TIME', followObject=True, bone='b_body',
            startMs=0, durationMs=2000, positionOffset=[position[0], 0, -position[2]],
            rotationDegrees=[-90, 0, 0], scale=[1, 1, 1])]
    assert sum(bool(t.get('effectTracks')) for t in modified['templates'] if t['sequenceId'] in selected) == 2
    modified['revision'] += 1
    target = ROOT / 'Data/Effects/Authored' / (asset + '.effect.json')
    assert not target.exists(), 'Existing authored fuse must be merged explicitly'
    source.write(evidence / 'candidate' / target.relative_to(ROOT), candidate)
    source.write(evidence / 'candidate' / world_path.relative_to(ROOT), modified)
    for path, raw in [(leaf_path, leaf_raw), (world_path, world_raw)]:
        destination = evidence / 'baseline' / path.relative_to(ROOT)
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(raw)
        assert path.read_bytes() == raw, 'Concurrent authoring edit: ' + str(path)
    hash_file = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
    relative = lambda path: path.relative_to(ROOT).as_posix()
    source.write(evidence / 'installation.json', dict(stageOnly=True, sourceWritten=False,
        inputHashes={relative(p): hash_file(p) for p in [leaf_path, world_path, cdo_path, raw_path, model_path, ROOT / 'Data/Actors/BossCatalog.json']},
        externalSourceInputs={str(socket_path): hash_file(socket_path)},
        documents=[dict(effectAssetId=asset, displayName=candidate['displayName'], path=relative(target),
            candidatePath=relative(evidence / 'candidate' / target.relative_to(ROOT)),
            beforeSha256=None, candidateSha256=hash_file(evidence / 'candidate' / target.relative_to(ROOT)),
            durationMs=2000, defaultAnchorKind='MAP', categoryPath=['Kouku', '3관문', '쇼타임', '폭탄'])],
        worldSequence=dict(path=relative(world_path), candidatePath=relative(evidence / 'candidate' / world_path.relative_to(ROOT)),
            beforeSha256=hashlib.sha256(world_raw).hexdigest(), afterSha256=hash_file(evidence / 'candidate' / world_path.relative_to(ROOT)),
            templateIds=selected, resourceId=object_id, beforeRevision=world['revision'], afterRevision=modified['revision']),
        sourceSocket=socket, installedSocketFrame=dict(position=[.2, 0, -.521496], rotationDegrees=[-90, 0, 0]),
        preserved='Existing object resources, user TRS, all instances, other templates, source leaf and native materials',
        lifetimePolicy='Authored continuous source emitters bounded by the original two-second Spark notify',
        manualVisualValidation='USER_PENDING'))
    print(relative(evidence / 'installation.json'))


def project(evidence, material_patch, install):
    index, notifies, occurrences, records = acquire(evidence)
    calls = source_model_calls(evidence)
    projectiles = projectile_leaf_parameters(evidence)
    material_patch = patch_simulation_providers(evidence, material_patch, index, occurrences)
    destination = evidence / 'projected'
    source.project(evidence, index, notifies, occurrences, records, destination, material_patch)
    writes, documents = [], []
    for ordinal, target in TARGETS.items():
        document = source.read(destination / f'effect.kouku.gate1.{ordinal}.full.restore.effect.json')
        document.update(effectAssetId=target['asset'], displayName=target['name'])
        bind_source_providers(document, index, occurrences)
        for element in document['elements']:
            element['groupId'] = target['asset']
            element['actionCueAttachment']['enabled'] = False
            element['actionCueAttachment'].pop('snapshotRootSourceBasisYawDegrees', None)
            element['detail']['transform'].update(position=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1])
            if target['system'] in projectiles:
                original = projectiles[target['system']]
                element['detail']['transform']['scale'] = ue3_axis_scale_to_client(original['sourceScale'])
                source.project_parameters(index, element['sourceRecipe'], original)
        if ordinal in calls:
            model = calls[ordinal]
            originals = document['elements']
            document['elements'] = []
            # A single selected original stage supplies both independent hands.
            # The group starts at that stage's first call; source call offsets
            # and source pose offsets remain synchronized within the group.
            first_time = min(c['notify']['localTimeSeconds'] for c in model['calls'])
            for call_index, call in enumerate(model['calls']):
                cue, notify = call['cue'], call['notify']
                for original in originals:
                    element = copy.deepcopy(original)
                    if call_index:
                        element['id'] += '.call' + str(call_index)
                    element['actionCueAttachment'] = source.imported.action_cue_attachment(dict(
                        actionCuePayload=cue, eventId=notify['notifyId'], actionCueId=notify['notifyId'],
                        globalTimeSeconds=notify['localTimeSeconds'] - first_time))
                    element['detail']['transform'].update(copy.deepcopy({k: cue['localTransform'][k]
                        for k in ('position', 'rotationDegrees', 'scale')}))
                    element['detail']['timing']['startDelaySeconds'] += notify['localTimeSeconds'] - first_time
                    source.project_parameters(index, element['sourceRecipe'], cue)
                    element['sourcePresentation'].update(sourceActionCueId=notify['notifyId'], sourceEventId=notify['notifyId'],
                        sourceTimeSeconds=notify['localTimeSeconds'])
                    document['elements'].append(element)
            clip = model['clip']
            document['sourceModelPreview'] = dict(gateId='GATE3', actorProfileId='MN_RPCT_05',
                targetBossPlacementId='boss.kakulsaydon.g3.saydon', animations=[dict(
                    runtimeClip='rpct00_' + clip['clipName'].lower(), startOffsetMs=0,
                    sourceStartMs=round(first_time * 1000), playMs=max(1, round((clip['lengthSeconds'] - first_time) * 1000)),
                    playRate=1, endPolicy='HOLD_LAST_POSE')])
        if ordinal == 14:
            restore_ball_drop_source_defaults(document)
        duration = math.ceil(max(e['detail']['timing']['startDelaySeconds'] + e['detail']['timing']['lifeTimeSeconds'] +
            max(e['detail']['particle']['lifeTimeSeconds']) for e in document['elements']) * 1000)
        source.write(evidence / 'candidate' / (target['asset'] + '.effect.json'), document)
        path = ROOT / 'Data/Effects/Authored' / (target['asset'] + '.effect.json')
        before = path.read_bytes() if path.exists() else None
        assert before is None or json.loads(before) == document, f'Preserve authored edits: {path}'
        writes.append((path, before, (json.dumps(document, ensure_ascii=False, indent=2, allow_nan=False) + '\n').encode('utf8')))
        category = '기관총' if ordinal <= 7 or ordinal == 20 else '공 낙하·폭발' if ordinal in (8, 9, 10, 11, 14) else '장판·조준·폭발'
        documents.append(dict(effectAssetId=target['asset'], path=path.relative_to(ROOT).as_posix(), displayName=target['name'],
            sourceParticleSystem=target['system'], elementCount=len(document['elements']), durationMs=duration,
            category='쇼타임/' + category, sourceModelPreview=document.get('sourceModelPreview'),
            nativePrograms=sorted({e['material']['sourceProfile']['runtimeShaderProfileId'] for e in document['elements']
                if e['material'].get('sourceProfile', {}).get('enabled')})))
    assert material_patch is not None or not install
    for path, before, _ in writes:
        assert (path.read_bytes() if path.exists() else None) == before, f'Concurrent edit: {path}'
    changed = []
    for path, before, payload in writes:
        if payload != before:
            changed.append(path.relative_to(ROOT).as_posix())
            if install:
                path.write_bytes(payload)
    source.write(evidence / 'installation.json', dict(installed=install, changedPaths=changed,
        documents=documents, sourceKind='ORIGINAL_PARTICLE_SYSTEM_LIBRARY_PREVIEW',
        ownership='Occurrence owns anchor and placement; source modules own internal particle timing',
        manualVisualValidation='USER_PENDING'))
    print(json.dumps(dict(installed=install, changedPaths=changed, documents=documents), ensure_ascii=False))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/KoukuShowtimeRestore20260912')
    parser.add_argument('--acquire-only', action='store_true')
    parser.add_argument('--native-material-patch', type=Path)
    parser.add_argument('--install', action='store_true')
    parser.add_argument('--repair-ball-drop-defaults', action='store_true',
                        help='Stage the current ball leaf with inherited source size/path defaults; never installs')
    parser.add_argument('--stage-bomb-fuse', action='store_true', help='Stage the skull-bomb source fuse and two WORLD tracks; never installs')
    args = parser.parse_args()
    if args.stage_bomb_fuse:
        assert not args.install and not args.acquire_only and args.native_material_patch is None and not args.repair_ball_drop_defaults
        stage_bomb_fuse(args.evidence_root)
    elif args.repair_ball_drop_defaults:
        assert not args.install and not args.acquire_only and args.native_material_patch is None
        stage_ball_drop_source_defaults(args.evidence_root)
    elif args.acquire_only:
        acquire(args.evidence_root)
    else:
        project(args.evidence_root, args.native_material_patch, args.install)

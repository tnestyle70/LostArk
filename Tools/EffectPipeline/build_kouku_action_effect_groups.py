"""Instantiate original enabled Action notifies from the recovered PS library.

Source stages are independent groups: condition/probability branches are not
invented as one linear action. All poses resolve against installed WModel clips
and bones; native material and Cascade data remain the shared library's source.
"""
import argparse
import base64
import collections
import contextlib
import copy
import hashlib
import json
import io
import math
import re
import struct
import sys
from pathlib import Path

import build_kouku_gate1_full_restore as source
import build_kouku_gate3_slam_mario_restore as exact

ROOT = source.ROOT
IMPORTED = source.imported
PROFILES = {
    'MN_RPCT_05': ('GATE1', 'MN_RPCT_05', 'boss.kakulsaydon.g1.saydon', 'mn_rpct_05.mesh.mn_rpct_05_sk'),
    'MN_RPCT_07': ('GATE3', 'MN_RPCT_05', 'boss.kakulsaydon.g3.saydon', 'mn_rpct_05.mesh.mn_rpct_05_sk'),
    'MN_RPCZ_00': ('GATE2', 'MN_RPCZ_00', 'boss.kakulsaydon.g2.kouku', 'mn_rpcz_00.mesh.mn_rpcz_00_sk'),
    'MN_RPCT_06': ('GATE2', 'MN_RPCT_06', 'boss.kakulsaydon.g2.big-saydon', 'mn_rpct_06.mesh.mn_rpct_06_sk'),
}


def restored_index(evidence):
    """Rehydrate the already extracted CDO/archetype closure without rereading UPKs."""
    records = source.read(evidence / 'source_module_inputs.json')['records']
    records.update({r['fullPath']: r for r in source.read(evidence / 'source_class_defaults.json')['records']})
    effective, refs = {}, {}
    def resolve(key, stack=()):
        if key in effective:
            return effective[key]
        assert key not in stack, ('archetype cycle', key)
        row = records[key]
        parent = row.get('archetypeFullPath')
        cls = row['classPath']
        default = cls.split('.')[0] + '.default__' + cls.split('.')[-1]
        if not parent and key != default:
            parent = default
        value = resolve(parent, stack + (key,)) if parent else {}
        effective[key] = source.merge(value, source.norm(row.get('properties', {})))
        own_roots = {p.lower() for p in row.get('properties', {})}
        refs[key] = [copy.deepcopy(r) for r in refs.get(parent, [])
                     if r['property'].split('.')[0].split('[')[0].lower() not in own_roots] + copy.deepcopy(row['references'])
        return effective[key]
    index = IMPORTED.SourceIndex({}, {})
    for key, row in records.items():
        obj = IMPORTED.SourceObject(key, key, row['className'], key, resolve(key))
        for ref in refs[key]:
            pair = (ref['property'].lower(), ref['objectPath'])
            obj.reference_paths.append(pair)
            if ref['objectPath'] in records:
                obj.references.append(pair)
        index.objects[key] = obj
        index.by_source_id[key] = obj
    return index


def model_contract(profile, evidence):
    gate, actor, placement, mesh = PROFILES[profile]
    sys.path[:0] = [str(ROOT / 'Tools/CharacterCustomizing'), str(ROOT / 'Tools/ModelAssetConverter')]
    from align_rig_gltf import read_body_skeleton
    from retime_wmodel_from_psa import read_wmodel_animation_sections
    model = ROOT / 'Client/Bin/Resources/Character/KoukuSaton' / actor / (actor + '.wmodel')
    bones = dict(read_body_skeleton(model))
    clips = {c['name'] for c in read_wmodel_animation_sections(model.read_bytes())}
    sockets = exact.socket_contract(mesh)
    # The shared decoder uses socket bone names as its exact bare-bone index.
    # Add installed bones without a socket alias; never accept name prefixes as
    # skeleton evidence, and preserve actual original socket offsets.
    sockets['sockets'] += [dict(socketName='', boneName=bone) for bone in bones]
    source.write(evidence / (profile + '.model_contract.json'), dict(
        modelAssetId=model.relative_to(ROOT / 'Client/Bin/Resources').as_posix(),
        modelSha256=hashlib.sha256(model.read_bytes()).hexdigest(),
        installedBones=bones, installedClips=sorted(clips), sourceSockets=sockets))
    return dict(gateId=gate, actorProfileId=actor, targetBossPlacementId=placement), bones, clips, sockets


def decode_notify(notify, sockets, bones):
    payload = notify['serializedPayload']
    raw = base64.b64decode(payload['data'])
    adapted = bytearray(raw)
    extra = {}
    # Engine.u EParticleSysParamType export 12792: Actor=6, VectorRand=4.
    # The common scalar/vector decoder still owns all field boundaries. Adapt
    # only exact named records, then require its decoded source byte offset to
    # match before restoring the original kind and random endpoints below.
    for name in ['None'] + [str(v) for v in notify['serializedLabels'] if re.fullmatch(r'[A-Za-z_][A-Za-z_0-9]*', str(v))]:
        encoded = name.encode('ascii') + b'\0'
        marker = struct.pack('<i', len(encoded)) + encoded
        for match in re.finditer(re.escape(marker), raw):
            at = match.end()
            if at + 69 > len(raw) or raw[at + 56:at + 65] != b'\x05\0\0\0None\0':
                continue
            kind = struct.unpack_from('<i', raw, at)[0]
            if kind == 6 and name == 'None' and raw[at + 4:at + 56] == bytes(52):
                struct.pack_into('<i', adapted, at, 0)
                extra[match.start()] = dict(type='none', enabled=False, sourceTypeCode=6, sourceNullActor=True)
            elif kind == 4:
                high, low = list(struct.unpack_from('<3f', raw, at + 12)), list(struct.unpack_from('<3f', raw, at + 24))
                assert all(math.isfinite(v) for v in high + low)
                struct.pack_into('<i', adapted, at, 3)
                extra[match.start()] = dict(type='vectorRandom', sourceTypeCode=4, vectorMaximum=high, vectorMinimum=low)
    if extra:
        payload = dict(payload, data=base64.b64encode(adapted).decode('ascii'))
    cue = source.decode_typed_payload(notify['sourceType'], payload, sockets, notify['assetReferences'], notify['serializedLabels'])
    seen = set()
    for parameter in cue.get('parameterOverrides', []):
        at = parameter['sourceRecordByteOffset']
        if at in extra:
            parameter.update(extra[at])
            seen.add(at)
    assert seen == set(extra), 'Adapted source parameter is outside the decoded parameter table'
    if not cue.get('enabled'):
        return cue
    if not cue.get('particleDataDecoded') or not cue.get('parameterOverridesDecoded'):
        raise ValueError('Source CEFParticleData or parameter table is not decoded')
    attachment = cue['attachment']
    for anchor in attachment['runtimeAnchors']:
        runtime = anchor['runtimeBoneName'].lower()
        if runtime not in bones:
            raise ValueError('Source anchor has no installed bone: ' + anchor['sourceAnchorName'])
        anchor['runtimeBoneName'] = runtime
    return cue


def project_parameters(index, recipe, cue):
    random = [p for p in cue['parameterOverrides'] if p['type'] == 'vectorRandom']
    if not random:
        return source.project_parameters(index, recipe, cue)
    endpoints = []
    for endpoint in ('vectorMinimum', 'vectorMaximum'):
        adjusted = copy.deepcopy(cue)
        for parameter in adjusted['parameterOverrides']:
            if parameter['type'] == 'vectorRandom':
                parameter.update(type='vector', vectorValue=parameter[endpoint])
        endpoint_recipe = copy.deepcopy(recipe)
        evidence = source.project_parameters(index, endpoint_recipe, adjusted)
        endpoints.append((endpoint_recipe, evidence))
    source.project_parameters(index, recipe, cue)
    random_names = {p['name'].lower() for p in random}
    for module, low, high in zip(recipe['modules'], endpoints[0][0]['modules'], endpoints[1][0]['modules']):
        for dist, ld, hd in zip(module['distributions'], low['distributions'], high['distributions']):
            obj = index.get_path(dist.get('sourceObjectPath'))
            if not obj or IMPORTED.prop(obj.properties, 'parametername', '').lower() not in random_names:
                continue
            modes = [IMPORTED.prop(obj.properties, 'parammodes' if i == 0 else f'parammodes[{i}]', 'dpm_normal') for i in range(3)]
            assert all(m in ('dpm_direct', 'dpm_normal') for m in modes), 'Random ABS parameters require a zero-crossing projection'
            dist.update(operation=2,
                defaultMinimum=[min(a,b) for a,b in zip(ld['defaultMinimum'],hd['defaultMinimum'])],
                defaultMaximum=[max(a,b) for a,b in zip(ld['defaultMaximum'],hd['defaultMaximum'])])
    return [dict(type='ORIGINAL_VECTOR_RANDOM_PARAMETER', parameter=p) for p in random]


def stage_animations(stage, original, clips):
    """Preserve each Anim notify's source start, then hold to a fixed stage exit."""
    mapped = {a['extractedClip'].lower(): a for a in stage['runtimeAnimations']}
    fixed = [n['localTimeSeconds'] for n in original['notifies'] if n['sourceType'] == 'MonsterMoveNextStage']
    stop = min(fixed) if fixed else None
    animations, missing = [], []
    for clip in original['animationClips']:
        mapping = mapped.get(clip['clipName'].lower())
        if not mapping or mapping['runtimeClip'] not in clips:
            missing.append(clip['clipName'])
            continue
        notify = next(n for n in original['notifies'] if n['notifyId'] == clip['notifyId'])
        start = max(0, round(notify['localTimeSeconds'] * 1000))
        length = int(mapping['playMs'])
        if stop is not None and stop * 1000 > start:
            length = max(1, round(stop * 1000) - start)
        animations.append(dict(runtimeClip=mapping['runtimeClip'], startOffsetMs=start,
            sourceStartMs=int(mapping['sourceStartMs']), playMs=length,
            playRate=mapping['playRate'], endPolicy='HOLD_LAST_POSE'))
    return animations, missing


def project_light_occurrences(index, source_occurrences, cue, notify, evidence):
    """Light birth/lifetime depends on original Spawn/Lifetime cue overrides."""
    identity = 999991
    own = copy.deepcopy(source_occurrences)
    for row in own:
        row.update(actionId=identity, sourceNotify=notify['notifyId'], sourceTimeSeconds=0,
                   sourceDurationSeconds=notify['durationSeconds'])
    original = copy.deepcopy(notify)
    first_anchor = copy.deepcopy(cue)
    first_anchor['attachment']['runtimeAnchors'] = first_anchor['attachment']['runtimeAnchors'][:1]
    schedules = {}
    for row in own:
        modules = [index.objects[k] for k in row['moduleOrder']]
        _, _, bursts = IMPORTED.emitter_detail(index, index.objects[row['sourceLOD']], modules, 0, notify['durationSeconds'], 1)
        recipe = IMPORTED.build_source_recipe(index, modules, 'light', bursts)
        project_parameters(index, recipe, cue)
        spawn = next(m for m in recipe['modules'] if m['className'] == 'particlemodulespawn')
        rate = next(d for d in spawn['distributions'] if d['propertyPath'] == 'rate')
        low, high = source.distribution_bounds(rate)
        assert low == high and low >= 0, 'Light birth rate requires a source constant'
        required = next(m for m in modules if m.class_name == 'particlemodulerequired')
        duration = IMPORTED.prop(required.properties, 'emitterduration', recipe['emitterDurationSeconds'])
        delay = IMPORTED.prop(required.properties, 'emitterdelay', 0)
        loops = IMPORTED.prop(required.properties, 'emitterloops', 0)
        assert loops == 1, 'Light loop birth scheduling requires an owning bounded cue window'
        active = min(duration, notify['durationSeconds']) if notify['durationSeconds'] > 0 else duration
        births = [delay + b['timeSeconds'] for b in recipe['bursts'] for _ in range(b['countMinimum'])]
        assert all(b['countMinimum'] == b['countMaximum'] for b in recipe['bursts'])
        if low:
            births += [delay + i / low for i in range(1, math.floor(active * low) + 1)]
        schedules[row['elementId']] = sorted(births)
    # Each existing typed-light element owns one source particle. The emitter's
    # constant-rate births are expanded below; all original distributions are
    # restored after using the common one-particle light projector.
    first_anchor['parameterOverrides'] = [p for p in first_anchor['parameterOverrides'] if p['name'].lower() != 'spawn']
    first_anchor['parameterOverrides'].append(dict(name='Spawn', type='scalar', scalarValue=0))
    original.update(actionId=identity, cue=first_anchor, sourceType='EXACT_DECODED_LIGHT_NOTIFY', globalTimeSeconds=0)
    old_selected = source.SELECTED
    source.SELECTED = {identity: ([0], 'Source light occurrence')}
    try:
        with contextlib.redirect_stdout(io.StringIO()):
            source.project(evidence / 'light_projection', index, [original], own, {}, evidence / 'light_projection')
    finally:
        source.SELECTED = old_selected
    document = source.read(evidence / 'light_projection' / f'effect.kouku.gate1.{identity}.full.restore.effect.json')
    output = []
    for element in document['elements']:
        project_parameters(index, element['sourceRecipe'], cue)
        for birth_index, birth in enumerate(schedules[element['id']]):
            extra = copy.deepcopy(element)
            extra['id'] += f'.birth{birth_index}'
            extra['detail']['timing']['startDelaySeconds'] = birth
            output.append(extra)
    document['elements'] = output
    return document


def project_trail(index, source_occurrences, notify, profile, asset, evidence, native_materials):
    raw = base64.b64decode(notify['serializedPayload']['data'])
    signature = b'CEFActionNotify_Trails\0'
    assert raw.startswith(signature) and raw[len(signature) + 12] in (0, 1)
    if not raw[len(signature) + 12]:
        return [], []
    paths = [r['objectPath'].lower() for r in notify['assetReferences'] if r['className'] == 'EFData_AnimNotify_Trails']
    assert len(paths) == 1
    outer = exact.native.obj(paths[0])
    package = exact.native.pkg(outer['package'])
    child = exact.native.fullref(outer['package'], package, exact.native.tagged_value(outer['properties'], 'trail_default'))
    row = source.record_from_export(package, outer['package'], source.find_export(package, child.split('.', 1)[1]))
    samples = []
    for raw_sample in IMPORTED.prop(row['properties'], 'trailsampleddata'):
        sample = dict(relativeTimeSeconds=IMPORTED.prop(raw_sample, 'relativetime'))
        for field, target in [('firstedgesample', 'firstEdgeUE3Cm'), ('controlpointsample', 'controlPointUE3Cm'), ('secondedgesample', 'secondEdgeUE3Cm')]:
            v = IMPORTED.prop(raw_sample, field)
            sample[target] = [v[k] for k in ('x','y','z')]
        samples.append(sample)
    assert len(samples) >= 2 and all(b['relativeTimeSeconds'] > a['relativeTimeSeconds'] for a,b in zip(samples,samples[1:]))
    history = dict(historyId='kouku.trail.' + hashlib.sha256((profile + notify['notifyId']).encode()).hexdigest()[:24],
        coordinateBasis='UE3_CM_X_Z_NEG_Y_TO_RUNTIME_METERS', sourceEndTimeSeconds=samples[-1]['relativeTimeSeconds'],
        playbackClampSeconds=notify['durationSeconds'], samples=samples)
    source.write(evidence / 'trails' / (history['historyId'] + '.source.json'), dict(sourcePath=paths[0], sourceRecord=row, projection=history))
    # First project the same Cascade modules through the common detail builder,
    # then select the existing authored baked-edge carrier with actual history.
    own = copy.deepcopy(source_occurrences)
    identity = 999992
    for occurrence in own:
        occurrence.update(actionId=identity, sourceNotify=notify['notifyId'], sourceTimeSeconds=notify['localTimeSeconds'],
            sourceDurationSeconds=notify['durationSeconds'], rendererShape='sprite')
    original = copy.deepcopy(notify)
    original.update(actionId=identity, cue=dict(enabled=True), globalTimeSeconds=notify['localTimeSeconds'])
    previous = source.SELECTED
    source.SELECTED = {identity: ([0], 'Source baked trail')}
    try:
        with contextlib.redirect_stdout(io.StringIO()):
            source.project(evidence / 'trail_projection', index, [original], own, {}, evidence / 'trail_projection')
    finally:
        source.SELECTED = previous
    document = source.read(evidence / 'trail_projection' / f'effect.kouku.gate1.{identity}.full.restore.effect.json')
    actor = PROFILES[profile][1]
    body = f'Character/KoukuSaton/{actor}/{actor}.wmodel'
    catalog = source.read(ROOT / 'Data/Actors/BossCatalog.json')
    scale = next(a['bodyModelPreScale'] for a in catalog['bosses'] if a.get('bodyModel') == body) / .01
    for element, occurrence in zip(document['elements'], source_occurrences):
        if occurrence['elementId'] in native_materials:
            element['material'] = copy.deepcopy(native_materials[occurrence['elementId']])
        element.update(id='kouku.trail.' + hashlib.sha256((profile + notify['notifyId'] + element['id']).encode()).hexdigest()[:24], groupId=asset)
        element['runtimeCarrier'] = dict(formatVersion=1, kind='animationTrailBakedEdgeV1', admission='bounded', historyId=history['historyId'])
        element['sourceRecipe'].update(enabled=False, rendererShape='animationTrail')
        element['detail']['transform']['scale'] = [scale] * 3
        element['detail']['trail'].update(maxPoints=len(samples), pointLifeTimeSeconds=max(element['detail']['particle']['lifeTimeSeconds']),
            sampleIntervalSeconds=min(b['relativeTimeSeconds'] - a['relativeTimeSeconds'] for a,b in zip(samples,samples[1:])), minimumDistance=0, faceCamera=False)
        element['sourcePresentation'].update(enabled=True, profileId='kouku.animation-trail-baked-edge-history.v1', status='reconstructed')
    return document['elements'], [history]


def instantiate(template, cue, notify, asset, profile, index):
    start = notify['localTimeSeconds']
    attachment = IMPORTED.action_cue_attachment(dict(actionCuePayload=cue))
    if attachment['sourceAnchorSlotId'] == 'root' and not attachment['follow']:
        attachment['snapshotRootSourceBasisYawDegrees'] = -90
    anchors = cue['attachment'].get('runtimeAnchors') or [None]
    output, parameter_evidence = [], []
    for anchor in anchors:
        suffix = '.' + anchor['sourceAnchorName'].lower() if anchor else ''
        ids = {e['id']: 'kouku.action.' + hashlib.sha256((profile + '|' + notify['notifyId'] + '|' + e['id'] + suffix).encode()).hexdigest()[:24]
               for e in template['elements']}
        for original in template['elements']:
            element = copy.deepcopy(original)
            element.update(id=ids[original['id']], groupId=asset,
                displayName=notify['notifyId'].rsplit('/', 1)[-1] + ' | ' + original['displayName'],
                sourceNode=notify['notifyId'] + '|' + original['sourcePresentation']['sourceObjectPath'])
            element['actionCueAttachment'] = copy.deepcopy(attachment)
            if anchor:
                element['actionCueAttachment'].update(sourceAnchorSlotId=anchor['sourceAnchorName'],
                    runtimeAnchorSlotId=anchor['runtimeAnchorSlotId'], runtimeBoneName=anchor['runtimeBoneName'],
                    socketLocalTransform=copy.deepcopy(anchor['socketLocalTransform']))
            element['sourcePresentation'].update(sourceActionCueId=notify['notifyId'], sourceEventId=notify['notifyId'], sourceTimeSeconds=start)
            detail, recipe = element['detail'], element['sourceRecipe']
            parameter_evidence += project_parameters(index, recipe, cue)
            for field in ('position', 'rotationDegrees', 'scale'):
                detail['transform'][field] = copy.deepcopy(cue['localTransform'][field])
            life = [d for m in recipe['modules'] if m['className'] == 'particlemodulelifetime'
                    for d in m['distributions'] if d['propertyPath'] == 'lifetime']
            if life:
                detail['particle']['lifeTimeSeconds'] = list(source.distribution_bounds(life[0]))
            active = notify['durationSeconds'] or recipe['emitterDurationSeconds'] * max(1, recipe['emitterLoopCount']) + recipe['emitterDelaySeconds']
            if active <= 0:
                raise ValueError('Source notify and emitter have no positive active window')
            if element['kind'] == 'light':
                detail['timing']['startDelaySeconds'] += start
            else:
                detail['timing'].update(startDelaySeconds=start, lifeTimeSeconds=active)
            if 'particleSystemOccurrenceId' in recipe:
                recipe['particleSystemOccurrenceId'] = profile + '/' + notify['notifyId'] + suffix
            for module in recipe['modules']:
                for literal in module['literals']:
                    if literal['propertyPath'] == 'runtime.providerelementid':
                        literal['value'] = ids[literal['value']]
            output.append(element)
    return output, parameter_evidence


def project(organization_path, library_root, evidence, install=False, native_patch=None):
    organization = source.read(organization_path)
    library = source.read(library_root / 'installation.json')
    index = restored_index(library_root)
    native_materials = {key: p['material'] for p in source.read(native_patch)['programs'] for key in p['occurrences']} if native_patch else {}
    templates = {r['sourceParticleSystem']: source.read(library_root / 'candidate' / Path(r['path']).name)
                 for r in library['documents']}
    source_occurrences = collections.defaultdict(list)
    for row in source.read(library_root / 'source_occurrences.json'):
        source_occurrences[row['sourceSystem']].append(row)
    contracts = {p: model_contract(p, evidence) for p in PROFILES}
    actions = {p: {a['actionId']: a for a in source.read(next(Path(a['sourceActionPath']) for a in organization['actions'] if a['profileId'] == p))['actions']}
               for p in PROFILES}
    documents, failures, disabled, occurrences = [], [], [], []
    for action in organization['actions']:
        profile = action['profileId']
        if profile not in contracts:
            continue
        preview, bones, clips, sockets = contracts[profile]
        original_action = actions[profile][action['actionId']]
        for stage in action['stages']:
            original = next(s for s in original_action['stages'] if s['stageIndex'] == stage['stageIndex'])
            source_notifies = [n for n in original['notifies'] if n['sourceType'] in ('PlayParticleEffect', 'Trails')]
            if not source_notifies:
                continue
            asset = f"effect.kouku.action.{profile.lower()}.{action['actionId']}.stage{stage['stageIndex']:03d}"
            animations, missing_clips = stage_animations(stage, original, clips)
            elements, own_evidence, histories = [], [], []
            for notify in source_notifies:
                context = dict(profileId=profile, actionId=action['actionId'], stageIndex=stage['stageIndex'], notifyId=notify['notifyId'])
                try:
                    if notify['sourceType'] == 'Trails':
                        systems = [r['objectPath'].lower() for r in notify['assetReferences'] if r['className'].lower() == 'particlesystem']
                        assert len(systems) == 1 and source_occurrences[systems[0]], 'Missing original Trail PS'
                        new, extra_histories = project_trail(index, source_occurrences[systems[0]], notify, profile, asset, evidence, native_materials)
                        elements += new
                        histories += extra_histories
                        own_evidence.append(dict(**context, sourceParticleSystem=systems[0], elementCount=len(new), sourceType='Trails'))
                        continue
                    if not any(r['className'].lower() == 'particlesystem' for r in notify['assetReferences']):
                        disabled.append(dict(**context, reason='SOURCE_NULL_PARTICLE_SYSTEM'))
                        continue
                    cue = decode_notify(notify, sockets, bones)
                    if not cue.get('enabled'):
                        disabled.append(context)
                        continue
                    systems = [r['objectPath'].lower() for r in notify['assetReferences'] if r['className'].lower() == 'particlesystem']
                    for system in systems:
                        context['sourceParticleSystem'] = system
                        template = templates.get(system)
                        light_rows = [o for o in source_occurrences[system] if o['rendererShape'] == 'light']
                        if light_rows:
                            light = project_light_occurrences(index, light_rows, cue, notify, evidence)
                            if template:
                                template = copy.deepcopy(template)
                                template['elements'] = [e for e in template['elements'] if e['kind'] != 'light'] + light['elements']
                            else:
                                template = light
                        if template is None:
                            raise ValueError('Source ParticleSystem library not projected: ' + system)
                        new, parameters = instantiate(template, cue, notify, asset, profile, index)
                        elements += new
                        own_evidence.append(dict(**context, elementCount=len(new), sourceTimeSeconds=notify['localTimeSeconds'],
                            sourceDurationSeconds=notify['durationSeconds'], sourceParameters=cue['parameterOverrides'], projectedParameters=parameters,
                            sourceAttachment=cue['attachment'], sourceTransform=cue['localTransform']))
                except Exception as error:
                    failures.append(dict(**context, reason=str(error), errorType=type(error).__name__))
            if not elements:
                continue
            if not animations:
                failures.append(dict(profileId=profile, actionId=action['actionId'], stageIndex=stage['stageIndex'],
                    reason='No installed source clip for effect stage', missingClips=missing_clips))
                continue
            document = dict(schema='lostark.effect-authoring', version=15, effectAssetId=asset,
                displayName=f"{action['displayName']} / {stage['stageName']} [{stage['stageIndex']}]",
                particleSystem=dict(uniformScaleMultiplier=1, yawOffsetDegrees=0, directionYawDegrees=0, initialSpeedMultiplier=1),
                modelCues=[], sourceModelPreview=dict(**preview, animations=animations), elements=elements)
            if histories:
                document['runtimeExtensions'] = dict(formatVersion=1, bakedEdgeHistories=histories)
            path = ROOT / 'Data/Effects/Authored' / (asset + '.effect.json')
            source.write(evidence / 'candidate' / path.name, document)
            own_failures = [f for f in failures if f['profileId'] == profile and f['actionId'] == action['actionId'] and f['stageIndex'] == stage['stageIndex']]
            duration = math.ceil(max(e['detail']['timing']['startDelaySeconds'] + e['detail']['timing']['lifeTimeSeconds']
                                    + max(e['detail']['particle']['lifeTimeSeconds']) for e in elements) * 1000)
            complete = not own_failures and not missing_clips
            if install and complete:
                assert library['installed'], 'Original native library must be installed first'
                assert all(e['kind'] == 'light' or e['material']['sourceProfile'].get('enabled') for e in elements), 'Native source material missing in action group'
                assert not path.exists() or source.read(path) == document, 'Preserve authored edits: ' + str(path)
                source.write(path, document)
            documents.append(dict(effectAssetId=asset, displayName=document['displayName'], path=path.relative_to(ROOT).as_posix(),
                profileId=profile, actionId=action['actionId'], stageIndex=stage['stageIndex'], stageName=stage['stageName'],
                categoryPath=action['categoryPath'], elementCount=len(elements), durationMs=duration, complete=complete,
                sourceParticleSystems=sorted({o['sourceParticleSystem'] for o in own_evidence}), sourceModelPreview=document['sourceModelPreview']))
            occurrences += own_evidence
    source.write(evidence / 'source_occurrences.json', occurrences)
    source.write(evidence / 'installation.json', dict(installed=install, documents=documents, sourceFailures=failures,
        disabledSourceNotifies=disabled, manualVisualValidation='USER_PENDING'))
    print(json.dumps(dict(documents=len(documents), complete=sum(d['complete'] for d in documents),
        elements=sum(d['elementCount'] for d in documents), sourceFailures=len(failures), disabledNotifies=len(disabled))))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--library-root', type=Path, default=ROOT / 'out/KoukuAllEffects20260912')
    parser.add_argument('--organization', type=Path, default=ROOT / 'out/KoukuAllEffects20260912/organization.json')
    parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/KoukuActionEffects20260912')
    parser.add_argument('--install', action='store_true')
    parser.add_argument('--native-material-patch', type=Path)
    args = parser.parse_args()
    project(args.organization, args.library_root, args.evidence_root, args.install, args.native_material_patch)

"""Build out-only Saydon circus World motions and source-backed V1 groups.

The source callback graph owns five splits; the user owns the +90 +/-45 degree
directions and one-bounce presentation. Existing WorldSequence group/emissions
own every object and effect clock. This adds no gameplay or network authority.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
from pathlib import Path
import sys

import build_kouku_circus_ball_groups as circus
from build_kouku_backstep_flame_groups import project_mesh_rotation, remap

ROOT = circus.ROOT
sys.path.insert(0, str(ROOT / 'Tools/KoukuSaydonPipeline'))
from apply_saydon_ball_world_motions import geometry

OUT = ROOT / 'out/SaydonCircusWorld20260917'
WORLD = ROOT / 'Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json'
AUTHORED = ROOT / 'Data/Effects/Authored'
PREFIX = 'effect.kouku.gate1.circus.'
WORLD_PREFIX = 'world.object.kouku.saydon.circus.'
MI_PREFIX = 'world.object.instance.kouku.saydon.circus.'
SQ_PREFIX = 'sequence.kouku.saydon.circus.'


def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def write(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def leaf(suffix):
    return read(AUTHORED / ('effect.kouku.source.fx_mn_ppct_00.par_k_ppct_vividfracture_' + suffix + '.effect.json'))


def group(asset, name, rows):
    result = copy.deepcopy(rows[0][0])
    result.update(effectAssetId=asset, displayName=name, elements=[], modelCues=[])
    result.pop('sourceModelPreview', None)
    for ordinal, (document, delay) in enumerate(rows):
        stem = 'circus.' + hashlib.sha256((asset + str(ordinal)).encode()).hexdigest()[:20]
        ids = {element['id']: stem + '.' + str(i) for i, element in enumerate(document['elements'])}
        elements = remap(copy.deepcopy(document['elements']), ids)
        for element in elements:
            element['groupId'] = asset
            element['detail']['timing']['startDelaySeconds'] += delay
            element['sourcePresentation']['sourceTimeSeconds'] += delay
            # The native library is root-relative; WorldSequence supplies its root.
            element['actionCueAttachment']['enabled'] = False
            project_mesh_rotation(element)
            result['elements'].append(element)
    return result


def inherit_missing_defaults(document, evidence):
    """Only restore missing nested CDO fields, never author replacement artwork."""
    defaults = read(ROOT / 'out/KoukuAllEffects20260912/source_class_defaults.json')['records']
    classes = {row['fullPath']: row for row in defaults}
    for element in document['elements']:
        for module in element['sourceRecipe']['modules']:
            pair = {'particlemodulelocationdirect': ('scalefactor', 'ScaleFactor'),
                    'particlemodulesize': ('startsize', 'StartSize')}.get(module['className'])
            if not pair:
                continue
            distribution = next((d for d in module['distributions'] if d['propertyPath'] == pair[0]), None)
            if not distribution or distribution['lookupTable'] or distribution['keys']:
                continue
            values = classes['engine.default__' + module['className']]['properties'][pair[1]]['value']['properties']
            assert values['LookupTable']['value'] == [1.0] * 8
            for target, source in [('operation', 'Op'), ('lookupTableNumElements', 'LookupTableNumElements'),
                    ('lookupTableChunkSize', 'LookupTableChunkSize'), ('lookupTableTimeScale', 'LookupTableTimeScale'),
                    ('lookupTableStartTime', 'LookupTableStartTime'), ('lookupTable', 'LookupTable')]:
                distribution[target] = copy.deepcopy(values[source]['value'])
            evidence.append(dict(module=module['objectPath'], property=pair[0], basis='SOURCE_CDO_NESTED_DEFAULT'))


def effect_track(name, asset, start, duration, offset, scale=1, follow=False, fit=False):
    return dict(effectTrackId=name, slotId='object', resourceKind='V1_EFFECT', resourceId=asset,
        followObject=follow, fitEffectToDuration=fit, bone='', timing='TIME', startMs=start,
        durationMs=duration, positionOffset=offset, rotationDegrees=[0, 0, 0], scale=[scale] * 3)


def curve_from_source(system):
    document = read(AUTHORED / ('effect.kouku.source.' + system + '.effect.json'))
    module = next(m for m in document['elements'][0]['sourceRecipe']['modules']
        if m['className'] == 'particlemodulelocationdirect' and circus.module_enabled(m))
    distribution = next(d for d in module['distributions'] if d['propertyPath'] == 'location')
    assert distribution['operation'] == 1 and distribution['lookupTableChunkSize'] == 3
    table = distribution['lookupTable'][2:]
    return [table[i:i + 3] for i in range(0, len(table), 3)]


def motion_pair(name, object_id, rows, keys, duration, effects):
    template = dict(sequenceId=SQ_PREFIX + name, displayName=name, category='WorldObject',
        durationMs=duration, interpolation='LINEAR',
        objectMotion=dict(velocity=[0, 0, 0], acceleration=[0, 0, 0], angularVelocityDegrees=[0, 0, 0],
            revolutionDegreesPerSecond=[0, 0, 0], revolutionOffset=[0, 0, 0], spawnHalfExtents=[0, 0, 0],
            count=len(rows), intervalMs=0, spreadDegrees=0, seed=4219806, emissions=rows),
        tracks=[dict(slotId='object', keys=keys)], animationTracks=[], effectTracks=effects)
    instance = dict(instanceId=MI_PREFIX + name, templateId=template['sequenceId'], enabled=True,
        startDelayMs=0, playbackSpeed=1, anchorKind='WORLD', position=[0, 0, 0],
        motionEnd='STOP', nextMotionId='',
        bindings=[dict(slotId='object', targetKind='OBJECT_RESOURCE', targetId=object_id)])
    return template, instance


def set_enabled(module, enabled):
    row = next((r for r in module['literals'] if r['propertyPath'] == 'benabled'), None)
    if row: row['value'] = enabled
    else: module['literals'].append(dict(propertyPath='benabled', kind='boolean', value=enabled))


def align_upper_sprite_facing(element):
    """Keep source velocity facing; author the derived quad above its anchor."""
    for module in element['sourceRecipe']['modules']:
        if module['className'] == 'particlemodulevelocity':
            # DirectLoc owns final position; this is also PSA_Velocity's axis.
            set_enabled(module, True)
        if module['className'] == 'particlemodulerequired':
            pivot = next(row for row in module['literals'] if row['propertyPath'] == 'offsetcentery')
            # Explicit requested upper attachment, not a native packing claim.
            pivot['value'] = 1


def suppress_split_child_upper(template):
    if template['sequenceId'] in {SQ_PREFIX + 'split.g' + str(i) for i in range(1, 6)}:
        template['effectTracks'] = [track for track in template['effectTracks']
                                   if track['effectTrackId'] != 'upper']


def align_drop(document, measured):
    """Share the existing live mesh particle's position; only its top is offset."""
    from build_kouku_dove_pizza_candidates import module, vector
    already_aligned = any('.ballfollow.' in m['objectPath'] for e in document['elements'] for m in e['sourceRecipe']['modules'])
    mesh = next(e for e in document['elements'] if e['sourceRecipe'].get('emitterName') == 'a'
                and e['sourceRecipe']['rendererShape'] == 'mesh')
    size_module = next(m for m in mesh['sourceRecipe']['modules'] if m['className'] == 'particlemodulesize')
    scale = next(d for d in size_module['distributions'] if d['propertyPath'] == 'startsize')['lookupTable'][2]
    center, half = measured['centerM'], measured['halfExtentsM']
    # Native LocationDirect ends at -.5m plus the mesh's +.1m start location.
    # The native centered FX mesh and the Mario World's bottom pivot differ.
    mesh['detail']['transform']['position'][1] = .4 - (center[1] - half[1]) * scale
    for element in document['elements']:
        if not element['displayName'].endswith(('particlespriteemitter_30', 'particlespriteemitter_35')):
            continue
        if not any('vividfracture_ball_04' in m['objectPath'] for m in element['sourceRecipe']['modules']):
            continue
        element['detail']['transform']['position'] = [0, 0, 0]
        for m in element['sourceRecipe']['modules']:
            if m['className'] == 'particlemodulelocationdirect':
                set_enabled(m, False)
        align_upper_sprite_facing(element)
        element['sourceRecipe']['particleSystemOccurrenceId'] = mesh['sourceRecipe']['particleSystemOccurrenceId']
        element['sourceRecipe']['emitterName'] = 'attached.upper.' + element['id']
        element['sourceRecipe']['modules'] = [m for m in element['sourceRecipe']['modules'] if '.ballfollow.' not in m['objectPath']]
        name = element['id'] + '.ballfollow'
        element['sourceRecipe']['modules'] += [
            module(name, 'direct', 'particlemodulelocationemitterdirect',
                {'bspawnmodule': True, 'bupdatemodule': True, 'emittername': 'a',
                 'runtime.providerelementid': mesh['id']}),
            module(name, 'top', 'particlemoduleorbit',
                {'bspawnmodule': True, 'bupdatemodule': True, 'chainmode': 'eochainmode_add',
                 'offsetoptions.bprocessduringspawn': True},
                [vector('offsetamount', [0, 0, (center[1] + half[1]) * scale * 100]),
                 vector('rotationamount', [0, 0, 0]), vector('rotationrateamount', [0, 0, 0])])]
    # Burst-only followers must evaluate after their live source on the first tick.
    followers = [e for e in document['elements'] if any(m['className'] == 'particlemodulelocationemitterdirect'
                 and '.ballfollow.' in m['objectPath'] for m in e['sourceRecipe']['modules'])]
    document['elements'] = [e for e in document['elements'] if e not in followers]
    position = document['elements'].index(mesh) + 1
    document['elements'][position:position] = followers
    for element in document['elements']:
        if not already_aligned and any('vividfracture_exp_' in m['objectPath'] for m in element['sourceRecipe']['modules']):
            element['detail']['timing']['startDelaySeconds'] -= .2 - 1 / 60
            element['detail']['transform']['position'][2] -= .2
            if element.get('sourcePresentation', {}).get('enabled'):
                element['sourcePresentation']['sourceTimeSeconds'] -= .2 - 1 / 60


def align_upper(document):
    # World already supplies the whole trajectory. Asset duration includes its
    # emitter tail, so fitting that duration shortens its only live particle.
    for element in document['elements']:
        for m in element['sourceRecipe']['modules']:
            if m['className'] in ('particlemodulelocationdirect', 'particlemodulevelocity'):
                set_enabled(m, False)
        particle = element['detail']['particle']
        particle['initialPositionMin'] = particle['initialPositionMax'] = [0, 0, 0]
        particle['initialVelocityMin'] = particle['initialVelocityMax'] = [0, 0, 0]
        life_module = next(m for m in element['sourceRecipe']['modules'] if m['className'] == 'particlemodulelifetime')
        source_life = next(d for d in life_module['distributions'] if d['propertyPath'] == 'lifetime')['lookupTable'][2]
        particle.setdefault('sourceScale', {})['lifeTime'] = 1.5 / source_life


def repair_follow():
    """Guarded, out-only correction of the user's latest saved documents."""
    destination = OUT / 'follow/candidate'
    composition_path = ROOT / 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'
    paths = [WORLD, composition_path] + [AUTHORED / (PREFIX + suffix + '.effect.json')
                                        for suffix in ('rainbow.drop', 'ball.upper')]
    inputs = {str(p.relative_to(ROOT)): p.read_bytes() for p in paths}
    baseline = read(WORLD); composition = read(composition_path)
    measured = geometry('Effect/KoukuSaydon/FullRestore/Meshes/fm_k_ppct_ball_01.wmodel')
    donor = next(o for o in baseline['objectResources'] if o['objectId'] == WORLD_PREFIX + 'split.model')
    world_geometry = geometry(donor['modelAssetId'])
    effects = []
    for suffix, operation in [('rainbow.drop', lambda d: align_drop(d, measured)), ('ball.upper', align_upper)]:
        path = AUTHORED / (PREFIX + suffix + '.effect.json')
        document = read(path); operation(document)
        target = destination / path.name; write(target, document)
        effects.append(dict(path=path.relative_to(ROOT).as_posix(), expectedSha256=hashlib.sha256(path.read_bytes()).hexdigest(),
                            candidate=target.relative_to(ROOT).as_posix()))
    world_ops = []
    for original in baseline['templates']:
        if not original['sequenceId'].startswith(SQ_PREFIX): continue
        row = copy.deepcopy(original)
        track = next(t for t in row['effectTracks'] if t['effectTrackId'] == 'upper')
        track['fitEffectToDuration'] = False
        track['positionOffset'] = [world_geometry['centerM'][0], world_geometry['centerM'][1] + world_geometry['halfExtentsM'][1], world_geometry['centerM'][2]]
        world_ops.append(dict(collection='templates', stableId=row['sequenceId'], expected=original, proposed=row))
    world_candidate = copy.deepcopy(baseline)
    by_id = {r['sequenceId']: r for r in world_candidate['templates']}
    for op in world_ops: by_id[op['stableId']].update(op['proposed'])
    world_candidate['revision'] += 1
    destination.mkdir(parents=True, exist_ok=True)
    # Keep the existing 16 MiB whole-document admission; indented key arrays exceed it.
    (destination / 'WorldSequences.candidate.json').write_text(json.dumps(world_candidate, ensure_ascii=False, separators=(',', ':')) + '\n', encoding='utf-8')
    pattern = next(p for p in composition['patterns'] if p['patternId'] == 'KAKULSAYDON_G1_PATTERN_83')
    definitions = {w['worldId']: w for w in composition['worlds']}
    group = next(w for w in composition['worlds'] if w['sequenceInstanceId'] == WORLD_PREFIX + 'split')
    changed = copy.deepcopy(pattern['worldOccurrences'])
    found = 0
    for row in changed:
        if definitions[row['worldId']]['sequenceInstanceId'] == MI_PREFIX + 'split.g0':
            row['worldId'] = group['worldId']; row['durationMs'] = math.ceil(11500 / row['playbackSpeed']); found += 1
    assert found == 1, 'Expected one saved P83 g0 occurrence; preserve new edits and inspect again.'
    ops = [dict(collection='patterns', stableId=pattern['patternId'], field='worldOccurrences',
                expected=pattern['worldOccurrences'], proposed=changed)]
    duration = max(pattern.get('durationMs', 0), sum(s['durationMs'] for s in pattern['stages']),
                   max(r['startMs'] + r['durationMs'] for r in changed))
    if duration > max(pattern.get('durationMs', 0), sum(s['durationMs'] for s in pattern['stages'])):
        ops.append(dict(collection='patterns', stableId=pattern['patternId'], field='durationMs', expected=pattern.get('durationMs', 0), proposed=duration))
    for resource in composition['presentationResources']:
        if resource['assetId'] == PREFIX + 'ball.upper' and resource['durationMs'] < 2500:
            ops.append(dict(collection='presentationResources', stableId=resource['resourceId'], field='durationMs',
                            expected=resource['durationMs'], proposed=2500))
    write(destination / 'repair.patch.json', dict(sourceRevision=composition['revision'], worldSourceRevision=baseline['revision'],
          sourceSha256=hashlib.sha256(inputs[str(composition_path.relative_to(ROOT))]).hexdigest(),
          worldSourceSha256=hashlib.sha256(inputs[str(WORLD.relative_to(ROOT))]).hexdigest(),
          operations=ops, worldOperations=world_ops, effects=effects, geometry=measured, worldGeometry=world_geometry,
          actualVisualApproval=False, liveWrites=False))
    for path, data in inputs.items(): assert (ROOT / path).read_bytes() == data, 'Input changed during candidate preparation'
    print(json.dumps(dict(output=str(destination), sourceRevision=composition['revision'], worldSourceRevision=baseline['revision'],
                         effects=len(effects), worldOperations=len(world_ops), compositionOperations=len(ops))))


def build_launch(output):
    """Derive an upward ball with a lower rainbow/star wake; never install it."""
    from build_kouku_dove_pizza_candidates import module, vector, native_references
    output = output.resolve()
    assert output.is_relative_to((ROOT / 'out').resolve()), 'Candidates must stay under out'
    source_path = AUTHORED / (PREFIX + 'rainbow.drop.effect.json')
    baseline_bytes = source_path.read_bytes()
    baseline = json.loads(baseline_bytes.decode('utf-8-sig'))
    asset = PREFIX + 'ball.launch'
    document = copy.deepcopy(baseline)
    selected = [e for e in document['elements']
                if '.par_k_ppct_vividfracture_ball_04.' in e['sourceNode']]
    by_emitter = {e['sourceNode'].rsplit('_', 1)[-1]: e for e in selected}
    # Keep the saved four-element ball, including the user's removal of sprite35.
    assert set(by_emitter) == {'5', '30', '33', '34'}, 'Inspect new saved ball elements before deriving launch'
    identities = {e['id']: 'circus.launch.' + hashlib.sha256(e['id'].encode()).hexdigest()[:20]
                  for e in selected}
    source_system = by_emitter['5']['sourceRecipe']['particleSystemOccurrenceId']
    identities[source_system] = asset + '.project-authored-upward'
    document['elements'] = remap(selected, identities)
    document.update(effectAssetId=asset, displayName='세이튼 / 쓰리투원투하 | 공 발사', modelCues=[])
    document.pop('sourceModelPreview', None)
    document.pop('sourceAnchorAnimations', None)
    for element in document['elements']:
        element['groupId'] = asset
        element['actionCueAttachment']['enabled'] = False
    by_emitter = {e['sourceNode'].rsplit('_', 1)[-1]: e for e in document['elements']}
    mesh, rainbow = by_emitter['5'], by_emitter['30']
    mesh_asset = next(r['assetId'] for r in mesh['resources'] if r['slotId'] == 'meshModel')
    measured = geometry(mesh_asset)
    size_module = next(m for m in mesh['sourceRecipe']['modules'] if m['className'] == 'particlemodulesize')
    size = next(d for d in size_module['distributions'] if d['propertyPath'] == 'startsize')
    assert size['operation'] == 1 and size['lookupTableNumElements'] == 1
    size_xyz = size['lookupTable'][2:5]
    assert max(size_xyz) - min(size_xyz) < 1.e-6
    assert mesh['detail']['transform']['scale'] == [1, 1, 1]
    size_scale = size_xyz[0] * mesh['detail']['particle'].get('sourceScale', {}).get('size', 1)
    bottom = (measured['centerM'][1] - measured['halfExtentsM'][1]) * size_scale
    direct = next(m for m in mesh['sourceRecipe']['modules'] if m['className'] == 'particlemodulelocationdirect')
    location = next(d for d in direct['distributions'] if d['propertyPath'] == 'location')
    assert location['operation'] == 1 and location['lookupTableChunkSize'] == 3 and not location['keys']
    samples = [location['lookupTable'][i:i + 3] for i in range(2, len(location['lookupTable']), 3)]
    assert samples == [[0, 0, 1000], [0, 0, -50]], 'Expected current 10.5m / 0.8s source drop'
    reversed_samples = list(reversed(samples))
    location['lookupTable'] = location['lookupTable'][:2] + [v for row in reversed_samples for v in row]
    # EmitterDirect owns the same live particle. Velocity only supplies the
    # source PSA_Velocity facing axis; offsetcentery0 extends the quad below it.
    rainbow['detail']['transform']['position'] = [0, 0, 0]
    for item in rainbow['sourceRecipe']['modules']:
        if item['className'] == 'particlemodulevelocity':
            set_enabled(item, True)
        elif item['className'] == 'particlemodulelocationdirect':
            set_enabled(item, False)
        elif item['className'] == 'particlemodulerequired':
            next(v for v in item['literals'] if v['propertyPath'] == 'offsetcentery')['value'] = 0
    followers = [m for m in rainbow['sourceRecipe']['modules']
                 if m['className'] == 'particlemodulelocationemitterdirect']
    assert len(followers) == 1 and any(v['propertyPath'] == 'runtime.providerelementid'
        and v['value'] == mesh['id'] for v in followers[0]['literals'])
    orbit = next(m for m in rainbow['sourceRecipe']['modules']
                 if m['className'] == 'particlemoduleorbit' and '.ballfollow.' in m['objectPath'])
    offset = next(d for d in orbit['distributions'] if d['propertyPath'] == 'offsetamount')
    offset.update(vector('offsetamount', [0, 0, bottom * 100]))
    # Stars are born at the ball underside, then retain their original sphere,
    # velocity/gravity, color and fade. They are a wake, not a second ball owner.
    for suffix in ('33', '34'):
        star = by_emitter[suffix]
        modules = star['sourceRecipe']['modules']
        provider = next(m for m in modules if m['className'] == 'efparticlemodulelocationemitter')
        literal = next((v for v in provider['literals'] if v['propertyPath'] == 'runtime.providerelementid'), None)
        if literal:
            literal['value'] = mesh['id']
        else:
            provider['literals'].append(dict(propertyPath='runtime.providerelementid', kind='string', value=mesh['id']))
        modules.insert(modules.index(provider) + 1, module(asset + '.' + suffix, 'underside',
            'particlemodulelocation', {'bspawnmodule': True}, [vector('startlocation', [0, 0, bottom * 100])]))
    # Provider first is required for same-tick burst and first star births.
    document['elements'] = [by_emitter[suffix] for suffix in ('5', '30', '33', '34')]
    refs = native_references(document)
    particle_life = mesh['detail']['particle']['lifeTimeSeconds']
    assert max(abs(v - .8) for v in particle_life) < 1.e-6
    assert all(not any('vividfracture_exp_' in m['objectPath'] for m in e['sourceRecipe']['modules'])
               for e in document['elements'])
    duration_ms = math.ceil(max(e['detail']['timing']['startDelaySeconds'] +
        e['detail']['timing']['lifeTimeSeconds'] + e['detail']['timing']['afterImageSeconds'] +
        max(e['detail']['particle']['lifeTimeSeconds']) for e in document['elements']) * 1000)
    tree = read(ROOT / 'Data/Effects/EffectResourceTree.json')
    parent = next(r['parentId'] for r in tree['references'] if r.get('assetId') == baseline['effectAssetId'])
    registration = dict(installed=False, catalogEntry=dict(effectAssetId=asset,
        payloadKind='DIRECT_AUTHORED_DOCUMENT', authoringPath=f'Effects/Authored/{asset}.effect.json'),
        treeReference=dict(kind='V1', assetId=asset, displayName=document['displayName'], parentId=parent),
        compositionResourceProposal=dict(assetId=asset, displayName=document['displayName'], durationMs=duration_ms),
        projectNoneInclude='..\\..\\Data\\Effects\\Authored\\' + asset + '.effect.json',
        projectFilter='96.DataFiles', patternOccurrenceAdded=False)
    # Analytic positions use the retained StartLocation and mesh pivot. Native
    # Codec/Playback/geometry validation is a separate caller-owned checkpoint.
    start_location = next(d for m in mesh['sourceRecipe']['modules'] if m['className'] == 'particlemodulelocation'
                          for d in m['distributions'] if d['propertyPath'] == 'startlocation')['lookupTable'][2:5]
    ys = [mesh['detail']['transform']['position'][1] + (row[2] + start_location[2]) * .01
          for row in reversed_samples]
    receipt = dict(status='OUT_CANDIDATE_PYTHON_CHECKED_NATIVE_PROBE_PENDING',
        basis='USER_REQUESTED_PROJECT_AUTHORED_REVERSE_SOURCE_DROP', sourcePath=source_path.relative_to(ROOT).as_posix(),
        sourceSha256=hashlib.sha256(baseline_bytes).hexdigest(), effectAssetId=asset,
        sourceCurveUE3Cm=samples, authoredCurveUE3Cm=reversed_samples, flightSeconds=particle_life[0],
        riseMeters=ys[1] - ys[0], centerStartYM=ys[0], centerEndYM=ys[1],
        startBottomYM=ys[0] + bottom, lowerAttachmentOffsetM=bottom,
        meshGeometry=measured, sourceSizeMultiplier=size_scale,
        ballWorldDiameterM=[2 * v * size_scale for v in measured['halfExtentsM']],
        elementIds={k: e['id'] for k, e in by_emitter.items()}, elementCount=len(document['elements']),
        durationMs=duration_ms, resources=refs, preserved='Saved ball model/scale and all four native materials, colors, particle lifetimes; source leaves/drop/World/Composition unchanged.',
        placement='Local +Y rise; the user authors the effect root angle and owner height. Rainbow uses ball underside, retained source +Y velocity facing and explicit lower pivot. Stars retain their native wake motion.',
        impactIncluded=False, clientRun=False, visualApproval=False, liveWrites=False)
    output.mkdir(parents=True, exist_ok=True)
    (output / 'source-drop.before.json').write_bytes(baseline_bytes)
    write(output / (asset + '.effect.json'), document)
    write(output / 'registration.entries.json', registration)
    write(output / 'candidate.receipt.json', receipt)
    assert source_path.read_bytes() == baseline_bytes, 'Saved drop changed during candidate generation'
    print(json.dumps(dict(output=str(output), elements=len(document['elements']), riseMeters=ys[1] - ys[0],
                         durationMs=duration_ms, installed=False), ensure_ascii=False))


def build(split_count=5):
    assert 1 <= split_count <= 5
    before = WORLD.read_bytes()
    baseline = json.loads(before.decode('utf-8-sig'))
    contract = circus.read_contract()
    contract.pop('previewPolicy', None)  # The older 63-element preview is not this World motion policy.
    assert len(contract['projectiles']) == 6
    source_ball = next(o for o in baseline['objectResources'] if o['objectId'] == 'world.object.mario.striped_ball')
    measured = geometry(source_ball['modelAssetId'])
    native_measured = geometry('Effect/KoukuSaydon/FullRestore/Meshes/fm_k_ppct_ball_01.wmodel')
    assert measured['vertexCount'] == native_measured['vertexCount'] == 290
    assert max(abs(a - b) for a, b in zip(measured['halfExtentsM'], native_measured['halfExtentsM'])) < 1e-6
    center, half = measured['centerM'], measured['halfExtentsM']
    defaults = []
    ball = leaf('ball_04')
    inherit_missing_defaults(ball, defaults)
    explosions = [leaf('exp_02'), leaf('exp_03'), leaf('exp_04')]
    impact = group(PREFIX + 'rainbow.impact', '세이튼 / 쓰리투원투하 | 무지개 도넛 폭발', [(d, 0) for d in explosions])
    drop = group(PREFIX + 'rainbow.drop', '세이튼 / 쓰리투원투하 | 공 낙하·상단 무지개·도넛 폭발',
        [(ball, 0)] + [(d, 1) for d in explosions])
    muzzle_leaf = read(AUTHORED / 'effect.kouku.gate3.showtime.gun.muzzle.effect.json')
    muzzle = group(PREFIX + 'gun.muzzle', '세이튼 / 쓰리투원투하 | 공 발사·쇼타임 총구', [(muzzle_leaf, 0)])
    # These are the two independent sprites around the original ball mesh.
    # Subtract that mesh's DirectLocation curve so the World object owns motion
    # exactly once. Emitter-location children remain only in the full V1 group.
    upper_source = copy.deepcopy(ball)
    upper_source['elements'] = [e for e in upper_source['elements']
        if e['sourcePresentation']['sourceObjectPath'].rsplit('_', 1)[-1] in ('30', '35')]
    for element in upper_source['elements']:
        module = next(m for m in element['sourceRecipe']['modules'] if m['className'] == 'particlemodulelocationdirect')
        location = next(d for d in module['distributions'] if d['propertyPath'] == 'location')
        assert location['lookupTableChunkSize'] == 3 and len(location['lookupTable']) == 8
        values = location['lookupTable']
        start = [values[i + 2] - [0, 0, 1000][i] for i in range(3)]
        finish = [values[i + 5] - [0, 0, -50][i] for i in range(3)]
        location['lookupTable'] = [min(start + finish), max(start + finish)] + start + finish
    upper = group(PREFIX + 'ball.upper', '세이튼 / 쓰리투원투하 | 공 상단 무지개', [(upper_source, 0)])
    align_drop(drop, native_measured)
    align_upper(upper)
    documents = [drop, impact, upper, muzzle]
    contract['sourceInputs'].extend(circus.digest(AUTHORED / (d['effectAssetId'] + '.effect.json'))
        for d in [leaf('ball_04'), *explosions, muzzle_leaf])
    candidate = dict(schema=baseline['schema'], formatVersion=3, areaId=baseline['areaId'], revision=1,
        objectResources=[], templates=[], instances=[])
    expected = []
    first_curve = curve_from_source(circus.SYSTEMS[0])[:9]
    child_curve = curve_from_source(circus.SYSTEMS[1])[:7]
    assert first_curve[3][2] == 20 and abs(first_curve[-1][2] - 20) < 1e-3
    assert child_curve[0][2] == child_curve[-1][2] == 20
    for mode, label in [('split', '세이튼 / 쓰리투원투하 | 공 1회 튕김·5회 분열'),
                        ('shot', '세이튼 / 쓰리투원투하 | 공 발사·5회 분열')]:
        object_id = WORLD_PREFIX + mode + '.model'
        obj = copy.deepcopy(source_ball)
        obj.update(objectId=object_id, displayName=label, scale=[1, 1, 1], anchorKind='WORLD',
            sequenceInstanceId='', defaultMotionInstanceId=MI_PREFIX + mode + '.g0')
        candidate['objectResources'].append(obj)
        candidate['objectResources'].append(dict(objectId=WORLD_PREFIX + mode, displayName=label,
            modelAssetId='', diffuseTextureAssetId='', modelPreScale=.01, animated=False,
            scale=[1, 1, 1], anchorKind='WORLD', sequenceInstanceId='', defaultMotionInstanceId='',
            motionInstanceIds=[MI_PREFIX + mode + '.g' + str(g) for g in range(split_count + 1)]))
        origins = [dict(positionOffset=[0, 0, 0], yawDegrees=0, startDelayMs=0)]
        for generation, projectile in enumerate(contract['projectiles'][:split_count + 1]):
            duration = round(projectile['maxLifeSeconds'] * 1000)
            scale = projectile['scale']
            curve = first_curve if generation == 0 and mode == 'split' else child_curve
            distance = projectile['maxDistanceCm'] * .01
            keys = []
            for index, sample in enumerate(curve):
                fraction = index / (len(curve) - 1)
                height = (sample[2] - 20) * .01
                desired = [0, half[1] * scale + height, distance * fraction]
                position = [desired[i] - center[i] * scale for i in range(3)]
                keys.append(dict(timeMs=round(fraction * duration), positionOffset=position,
                    rotationQuaternion=[0, 0, 0, 1], scaleMultiplier=[scale] * 3,
                    visible=index != len(curve) - 1))
            effects = [effect_track('upper', upper['effectAssetId'], 0, duration, [center[0], center[1] + half[1], center[2]], follow=True, fit=False),
                effect_track('impact', impact['effectAssetId'], duration, 2500,
                    [center[0], center[1] - half[1], center[2]], scale=1 / scale)]
            if generation == 0 and mode == 'split':
                effects.append(effect_track('first.bounce', impact['effectAssetId'], keys[3]['timeMs'], 2500,
                    [center[0], center[1] - half[1], center[2]], scale=1 / scale))
            if generation == 0 and mode == 'shot':
                effects.append(effect_track('muzzle', muzzle['effectAssetId'], 0, 11000, center, scale=1 / scale))
            name = mode + '.g' + str(generation)
            rows = [dict(row, startDelayMs=generation * duration) for row in origins]
            template, instance = motion_pair(name, object_id, rows, keys, duration, effects)
            suppress_split_child_upper(template)
            candidate['templates'].append(template)
            candidate['instances'].append(instance)
            following = []
            for emitter, row in enumerate(origins):
                radians = math.radians(row['yawDegrees'])
                end = [row['positionOffset'][0] + distance * math.sin(radians), 0,
                    row['positionOffset'][2] + distance * math.cos(radians)]
                expected.append(dict(mode=mode, generation=generation, emitter=emitter,
                    origin=row['positionOffset'], finish=end, yawDegrees=row['yawDegrees'], scale=scale,
                    startMs=generation * duration, endMs=(generation + 1) * duration))
                for delta in (45, 135):
                    following.append(dict(positionOffset=end, yawDegrees=row['yawDegrees'] + delta, startDelayMs=0))
            origins = following
    # Registration entries are semantic append inputs, never replacements for Live documents.
    for document in documents:
        write(OUT / 'candidates' / (document['effectAssetId'] + '.effect.json'), document)
    write(OUT / 'candidates/WorldSequence.entries.json', candidate)
    write(OUT / 'candidates/EffectCatalog.entries.json', dict(effects=[dict(effectAssetId=d['effectAssetId'],
        payloadKind='DIRECT_AUTHORED_DOCUMENT', authoringPath='Effects/Authored/' + d['effectAssetId'] + '.effect.json') for d in documents]))
    write(OUT / 'candidates/EffectResourceTree.entries.json', dict(nodes=[],
        references=[dict(kind='V1', assetId=d['effectAssetId'], displayName=d['displayName'],
            parentId='kouku.category.867ede1a948dc387a93d') for d in documents]))
    write(OUT / 'candidates/Composition.worlds.proposal.json', dict(registerOnly=True, patternMutation=None,
        definitions=[dict(stableProposalId='saydon.circus.' + mode, displayName=obj['displayName'],
            objectResourceId=obj['objectId'], sequenceInstanceId=obj['objectId'],
            positionOffset=[0, 0, 0], anchorKind='NONE', anchorPosition=[0, 0, 0], companionEffectResourceId='')
            for mode, obj in zip(('split', 'shot'), candidate['objectResources'][1::2])],
        existingGunResources=['world.object.kouku.saydon_showtime_gun_left', 'world.object.kouku.saydon_showtime_gun_right'],
        timelinePlacement='World group owns six STOP motions with emission delays; one Composition occurrence targets the group ID. P83 remains untouched.',
        presentationDurationMs=(split_count + 1) * 1500 + 2500))
    contract.update(requestedSplitCount=split_count, generatedBallCountPerWorld=2 ** (split_count + 1) - 1,
        nativeSplitCount=5, directions=dict(basis='USER_AUTHORED', parentAxisTurn=90, childOffsets=[-45, 45]),
        motionProjection=dict(basis='PROJECT_AUTHORED', policy='FIRST_BOUNCE_CURVE_RETIMED_TO_SOURCE_MISSILE_LIFETIME',
            firstSourceSamples=first_curve, childSourceSamples=child_curve, geometry=measured,
            material='Existing Mario StripedBall CMaterial; intentionally reused per user request.',
            centerPolicy='Cancel measured CModel pivot; rest bottom touches ground.',
            horizontalPolicy='SOURCE_MAX_DISTANCE_SPREAD_OVER_SOURCE_MAX_LIFETIME; source speed is retained as evidence, not claimed as reproduced.',
            scheduling='Six STOP motions in one model-less World group; per-emission birth delays scale with playback speed.',
            groupOrigin='First successful cue-birth provider matrix shared by every generation; pending origin retries.',
            damageAuthority=False, gameplayCollisionAdded=False), sourceDefaultRepairs=defaults,
        rainbowSource=dict(projectileId=421980613, systems=['VividFracture_Ball_04', 'VividFracture_Exp_02',
            'VividFracture_Exp_03', 'VividFracture_Exp_04'],
            upperPolicy='World g0 and shot sprites follow only the model top for 1.5s without duration fitting; split g1..g5 omit upper emission. Standalone drop retains source velocity for facing and uses an explicit upper-attachment pivot override with emitter-direct position.',
            dropImpactDelaySeconds=.8 + 1 / 60, delayBasis='PROJECT_AUTHORED matching source Ball_04 0.8s particle lifetime plus existing fixed-step birth tick; mesh ground pivot and live emitter-direct upper attachment'),
        deployment=dict(installed=False, worldBaselineSha256=hashlib.sha256(before).hexdigest(),
            untouchedP83=True, clientRun=False, visualApproval=False), expectedObjects=expected)
    write(OUT / 'source-contract.json', contract)
    write(OUT / 'pending-registration.json', dict(installed=False,
        worldEntries='candidates/WorldSequence.entries.json', effects='candidates/EffectCatalog.entries.json',
        effectTree='candidates/EffectResourceTree.entries.json', composition='candidates/Composition.worlds.proposal.json',
        worldGroups=[WORLD_PREFIX + 'split', WORLD_PREFIX + 'shot'],
        rules=['Semantic append by stable ID after preserving the user draft; never replace the whole Live document.',
            'World group objectId is both Composition objectResourceId and sequenceInstanceId.',
            'Do not add or alter P83 animation stages or occurrences.',
            'Keep existing Showtime BOSS hand-bound WORLD guns as separately placeable resources.'],
        durations=dict(rainbowDropMs=3500, rainbowImpactMs=2500, upperMs=2500, muzzleMs=11000,
            worldSplitMs=(split_count + 1) * 1500 + 2500, worldShotMs=max(11000, (split_count + 1) * 1500 + 2500))))
    assert WORLD.read_bytes() == before, 'Authoring document changed while reading; rebuild candidates.'
    print(json.dumps(dict(output=str(OUT), installed=False, effects=len(documents),
        worldResources=len(candidate['objectResources']), splits=split_count,
        ballsPerWorld=2 ** (split_count + 1) - 1), ensure_ascii=False))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--split-count', type=int, default=5, choices=range(1, 6))
    parser.add_argument('--repair-follow', action='store_true', help='Prepare guarded corrections from current saved data; no live writes.')
    parser.add_argument('--launch-only', action='store_true', help='Derive an upward V1 ball from the saved drop; no live writes.')
    parser.add_argument('--output', type=Path, default=ROOT / 'out/CardMatchFix20260917/ball-launch')
    args = parser.parse_args()
    assert not (args.launch_only and args.repair_follow), 'Choose one candidate mode'
    if args.launch_only:
        build_launch(args.output)
    elif args.repair_follow:
        repair_follow()
    else:
        build(args.split_count)

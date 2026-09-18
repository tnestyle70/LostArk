"""Assemble Saydon card/trumpet groups from installed source occurrences.

Writes candidates only. The live Composition and the user's edited leaves are
never rewritten by this builder. Source timings remain separate from the
explicit 6114/4184/6637 ms sequencing requested for the current patterns.
"""
from __future__ import annotations

import copy
import argparse
import hashlib
import json
import math
import sys
from pathlib import Path

import build_kouku_gate1_full_restore as source
from build_kouku_backstep_flame_groups import project_mesh_rotation, remap

ROOT = source.ROOT
AUTHORED = ROOT / 'Data/Effects/Authored'
OUTPUT = ROOT / 'out/sayton-pattern-20260917/effects'
STAGES = ROOT / 'out/KoukuActionEffects20260912/candidate'
PREFIX = 'effect.kouku.card.match.'
SUITS = (('heart', '하트', 5, 2), ('clover', '클로버', 4, 0),
         ('diamond', '다이아', 6, 1), ('spade', '스페이드', 3, 3))


def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2,
                               allow_nan=False) + '\n', encoding='utf-8')


def leaf(suffix):
    return read(AUTHORED / ('effect.kouku.source.fx_mn_rpct_05_l.'
                           'par_l_rpct_05_sk_' + suffix + '.effect.json'))


def renamed(document, asset, name):
    result = copy.deepcopy(document)
    result.update(effectAssetId=asset, displayName=name)
    if result['version'] == 15:
        result.setdefault('runtimeExtensions', dict(formatVersion=1, bakedEdgeHistories=[]))
    mapping = {e['id']: 'saydon.' + hashlib.sha256(
        (asset + '/' + e['id']).encode()).hexdigest()[:24]
        for e in result['elements']}
    result['elements'] = remap(result['elements'], mapping)
    for e in result['elements']:
        e['groupId'] = asset
    return result


def native_stage(action, ordinal):
    document = read(STAGES / f'effect.kouku.action.mn_rpct_05.{action}.stage{ordinal:03}.effect.json')
    libraries = {}
    for element in document['elements']:
        emitter = element['sourcePresentation']['sourceObjectPath']
        system = emitter.rsplit('.', 1)[0]
        if system not in libraries:
            library = read(AUTHORED / ('effect.kouku.source.' + system + '.effect.json'))
            libraries[system] = {e['sourcePresentation'].get('sourceObjectPath',
                e.get('sourceNode', '').split('|')[-1]): e for e in library['elements']}
        original = libraries[system][emitter]
        assert original['material']['sourceProfile']['enabled']
        assert original['material']['sourceMaterialPath'] == element['material']['sourceMaterialPath']
        element['material'] = copy.deepcopy(original['material'])
        element['resources'] = copy.deepcopy(original['resources'])
        if 'runtimeCarrier' in original:
            element['runtimeCarrier'] = copy.deepcopy(original['runtimeCarrier'])
        project_mesh_rotation(element)
    return document


def key(time, alpha):
    return dict(timeSeconds=time, value=[alpha] * 3, arriveTangent=[0] * 3,
                leaveTangent=[0] * 3, interpolation='constant')


def append_group(document, original, name, delay=0, cut=None):
    copied = renamed(original, document['effectAssetId'] + '.' + name, name)
    for element in copied['elements']:
        element['detail']['timing']['startDelaySeconds'] += delay
        presentation = element.get('sourcePresentation', {})
        if 'sourceTimeSeconds' in presentation:
            presentation['sourceTimeSeconds'] += delay
        if cut is not None:
            element['detail']['timing']['lifeTimeSeconds'] = min(
                element['detail']['timing']['lifeTimeSeconds'], cut - delay)
            assert 'sourceTransformTrack' not in element
            element['sourceTransformTrack'] = dict(sourceOccurrenceId=element['id'],
                sourceTimeOriginSeconds=0, previewOriginUE3Cm=[0, 0, 0],
                nodes=[dict(sourceObjectPath=element['id'], frame='WORLD',
                    initialPositionUE3Cm=[0, 0, 0], initialEulerDegrees=[0, 0, 0],
                    scaleUE3=[1, 1, 1], positionKeys=[], eulerKeys=[])],
                alphaScaleKeys=[key(delay, 1), key(cut, 0)])
        document['elements'].append(element)


def current_preview(pattern_ordinal):
    composition = read(ROOT / 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json')
    pattern = next(p for p in composition['patterns']
                   if p['patternId'] == f'KAKULSAYDON_G1_PATTERN_{pattern_ordinal}')
    animations, offset = [], 0
    for stage in pattern['stages']:
        for occurrence in stage['animationOccurrences']:
            animation = {k: occurrence[k] for k in ('runtimeClip', 'startOffsetMs',
                'sourceStartMs', 'playMs', 'playRate', 'endPolicy')}
            animation['startOffsetMs'] += offset
            animations.append(animation)
        offset += stage['durationMs']
    return dict(gateId=pattern['gateId'], actorProfileId=pattern['actorProfileId'],
                targetBossPlacementId=pattern['targetBossPlacementId'], animations=animations)


def fixed_subimage(element, tile):
    recipe = element['sourceRecipe']
    required = next(m for m in recipe['modules'] if m['className'] == 'particlemodulerequired')
    mode = next(v for v in required['literals'] if v['propertyPath'] == 'interpolationmethod')
    assert mode['value'] == 'psuvim_random'
    mode['value'] = 'psuvim_linear'
    module = next(m for m in recipe['modules'] if m['className'] == 'particlemodulesubuv')
    distribution = next(d for d in module['distributions'] if d['propertyPath'] == 'subimageindex')
    distribution.update(lookupTable=[tile] * 4, lookupTableTimeScale=0,
                        lookupTableStartTime=0, keys=[])


def duration_ms(document):
    return math.ceil(max(e['detail']['timing']['startDelaySeconds'] +
        e['detail']['timing']['lifeTimeSeconds'] +
        e['detail']['timing']['afterImageSeconds'] +
        max(e['detail']['particle']['lifeTimeSeconds'])
        for e in document['elements']) * 1000)


def repair_dice_continuity(document):
    """Project the original dice curves onto one measured owner-space flight.

    This is an authored continuity correction, not a decoded proprietary notify
    rotation flag. Native materials, spin rates and source sample order remain.
    The receipt owns the exact original fields and the installed-rig measurement.
    """
    sys.path.insert(0, str(ROOT))
    from Tools.KoukuSaydonPipeline import project_kouku_saydon_composition as composition
    pose = composition.wmodel_pose
    dice = [e for e in document['elements'] if any(
        r['assetId'].endswith('/fm_l_dice_01_sm.wmodel') for r in e['resources'])]
    assert len(dice) == 4
    by_notify = {e['sourcePresentation']['sourceEventId'].rsplit('-', 1)[-1]: e for e in dice}
    hand, rise, fall, landed = [by_notify[n] for n in ('005', '010', '011', '015')]
    assert document['sourceModelPreview']['animations'][0]['runtimeClip'] == 'rpct00_att_battle_11_02'
    before = copy.deepcopy(document)
    def birth_time(element):
        return math.ceil(element['detail']['timing']['startDelaySeconds'] * 60 - 1e-4) / 60
    def cut_at(element, seconds):
        assert 'sourceTransformTrack' not in element
        element['sourceTransformTrack'] = dict(sourceOccurrenceId=element['id'], sourceTimeOriginSeconds=0,
            previewOriginUE3Cm=[0, 0, 0], nodes=[dict(sourceObjectPath=element['id'], frame='WORLD',
            initialPositionUE3Cm=[0, 0, 0], initialEulerDegrees=[0, 0, 0], scaleUE3=[1, 1, 1],
            positionKeys=[], eulerKeys=[])], alphaScaleKeys=[key(0, 1), key(seconds, 0)])
    release_time = birth_time(rise)
    model_path = ROOT / 'Client/Bin/Resources/Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel'
    boss = next(b for b in read(ROOT / 'Data/Actors/BossCatalog.json')['bosses']
                if b['archetypeId'] == 'BOSS_KAKULSAYDON_G1_SAYDON')
    model = pose.read_wmodel(model_path, animation_names={'rpct00_att_battle_11_02'})
    actor = dict(body=model, bodyPre=pose.affine_matrix((boss['bodyModelPreScale'],) * 3,
        (0, 0, 0, 1), (0, 0, 0)), poses={}, validatedClips=set())
    bone = composition._sample_bone_bake_pose(actor, 'body', 'rpct00_att_battle_11_02',
        release_time, root_vertical_scale=0)[[b.name for b in model.skeleton_bones].index('bip001-l-hand')]
    assert hand['detail']['transform']['rotationDegrees'] == [0, 0, 0]
    assert hand['actionCueAttachment']['socketLocalTransform'] == dict(
        position=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1])
    release = pose.matrix_multiply(pose.affine_matrix((1, 1, 1), (0, 0, 0, 1),
        hand['detail']['transform']['position']), bone)[12:15]
    basis = [math.sqrt(sum(bone[r * 4 + c] ** 2 for c in range(3))) for r in range(3)]
    assert max(basis) - min(basis) < 1e-5
    # Snapshot source yaw -90 maps UE centimeters (x,y,z) to runtime (y,z,x).
    release_ue = [release[2] * 100, release[0] * 100, release[1] * 100]
    hand['detail']['particle']['sourceScale']['size'] /= basis[0]
    defaults_path = ROOT / 'out/KoukuAllEffects20260912/source_class_defaults.json'
    instances_path = ROOT / 'out/KoukuAllEffects20260912/source_module_inputs.json'
    cdo = next(r for r in read(defaults_path)['records']
               if r['fullPath'] == 'engine.default__particlemodulelocationdirect')
    fields = cdo['properties']['ScaleFactor']['value']['properties']
    assert fields['LookupTable']['value'] == [1.] * 8
    inherited = {target: fields[source_name]['value'] for target, source_name in (
        ('operation', 'Op'), ('lookupTableNumElements', 'LookupTableNumElements'),
        ('lookupTableChunkSize', 'LookupTableChunkSize'), ('lookupTableTimeScale', 'LookupTableTimeScale'),
        ('lookupTableStartTime', 'LookupTableStartTime'), ('lookupTable', 'LookupTable'))}
    instances = read(instances_path)['records']
    def distribution(element, class_name, property_name):
        module = next(m for m in element['sourceRecipe']['modules'] if m['className'] == class_name)
        return module, next(d for d in module['distributions'] if d['propertyPath'] == property_name)
    end = distribution(landed, 'particlemodulelocation', 'startlocation')[1]['lookupTable'][2:5]
    apex = distribution(fall, 'particlemodulelocationdirect', 'location')[1]['lookupTable'][2:5]
    for element, next_element in ((rise, fall), (fall, landed)):
        module, curve = distribution(element, 'particlemodulelocationdirect', 'location')
        scale = next(d for d in module['distributions'] if d['propertyPath'] == 'scalefactor')
        raw = instances[module['objectPath']]['properties']['scalefactor']['value']['properties']
        assert list(raw) == ['distribution'] and raw['distribution']['value'] == 0
        assert not scale['lookupTable'] and not scale['keys']
        scale.update(copy.deepcopy(inherited))
        points = [curve['lookupTable'][i:i + 3] for i in range(2, len(curve['lookupTable']), 3)]
        assert len(points) in (5, 7) and curve['componentCount'] == 3
        if element is rise:
            assert points[0] == [0, 0, 0] and points[-1] == apex
            points = [[v + (1 - point[2] / apex[2]) * release_ue[axis]
                       for axis, v in enumerate(point)] for point in points]
        else:
            original_end = points[-1]
            points = [[v + (index / (len(points) - 1)) * (end[axis] - original_end[axis])
                       for axis, v in enumerate(point)] for index, point in enumerate(points)]
        curve['lookupTable'] = [min(v for p in points for v in p), max(v for p in points for v in p)] + [v for p in points for v in p]
        window = birth_time(next_element) - birth_time(element)
        # Keep the last pre-handoff sample alive despite floating fixed-step
        # rounding. An absolute cutoff owns visibility, not particle expiry.
        lifetime = window + 1 / 60
        curve['lookupTableTimeScale'] = (len(points) - 1) * lifetime / window
        life = distribution(element, 'particlemodulelifetime', 'lifetime')[1]
        life['lookupTable'] = [lifetime] * 4
        element['detail']['particle']['lifeTimeSeconds'] = [lifetime, lifetime]
        # One solid carrier until the next carrier takes over; native spin stays.
        alpha = distribution(element, 'particlemodulecolorscaleoverlife', 'alphascaleoverlife')[1]
        alpha.update(lookupTable=[1.] * 4, lookupTableTimeScale=0.)
        cut_at(element, next_element['detail']['timing']['startDelaySeconds'])
    # All three ascent emitters now share the snapshot owner basis. Their original
    # emitter-location links continue to sample the same moving dice particle.
    for element in document['elements']:
        if 'stage-000/notify-010|' in element['sourceNode']:
            element['actionCueAttachment'] = copy.deepcopy(fall['actionCueAttachment'])
            element['detail']['transform']['position'] = [0, 0, 0]
    # The held mesh should not remain behind the released one for its old tail.
    cut_at(hand, rise['detail']['timing']['startDelaySeconds'])
    return dict(basis='SOURCE_CURVE_WITH_PROJECT_AUTHORED_CONTINUITY', sourceAction=4219840,
        modelAssetId=model_path.relative_to(ROOT / 'Client/Bin/Resources').as_posix(),
        modelSha256=hashlib.sha256(model_path.read_bytes()).hexdigest(), runtimeClip='rpct00_att_battle_11_02',
        releaseTimeSeconds=release_time, releasePosition=release, handBoneBasisScale=basis,
        sourceApexUE3Cm=apex, sourceLandingUE3Cm=end,
        cdoSourcePath=str(defaults_path), cdoScaleFactor=cdo['properties']['ScaleFactor'],
        originalDocument=before, manualVisualValidation='USER_PENDING')


def scale_dice_and_diamond_geometry(document, factor=1.5):
    """Apply the requested visual scale without changing the repaired flight."""
    changed = []
    for element in document['elements']:
        dice = any(r['assetId'].endswith('/fm_l_dice_01_sm.wmodel') for r in element['resources'])
        diamond = '.par_l_rpct_05_sk_09_15_loc_int.' in element.get('sourceNode', '') and any(
            t['assetId'].endswith('/fx_d_symbol_109.dds')
            for t in element['material'].get('sourceProfile', {}).get('textures', []))
        if dice or diamond:
            scale = element['detail']['particle'].setdefault('sourceScale', {})
            scale['size'] = scale.get('size', 1) * factor
            changed.append(element['id'])
    return changed


def scale_spinning_card_geometry(document, factor=1.5):
    """Apply the requested size trim to the card and its symbol afterimages only."""
    changed = []
    for element in document['elements']:
        if '.par_l_rpct_05_sk_13_1_loc_int.' not in element.get('sourceNode', ''):
            continue
        symbol = element['material'].get('sourceMaterialPath', '').endswith('.fx_l_pa_sy_09_1_tr')
        card_mesh = any(r['assetId'].endswith('/fm_l_rpct_rcard_01_sm.wmodel') for r in element['resources'])
        if symbol or card_mesh:
            scale = element['detail']['particle'].setdefault('sourceScale', {})
            scale['size'] = scale.get('size', 1) * factor
            changed.append(element['id'])
    return changed


def follow_local_card_axes(document):
    """Keep a local card overlay in the same owner frame as its mesh."""
    for element in document['elements']:
        if not any('.par_l_rpct_05_sk_' + suffix + '_loc_int.' in
                   element.get('sourceNode', '')
                   for suffix in ('08_3', '08_4', '08_5', '08_6', '13_1')):
            continue
        recipe = element.get('sourceRecipe', {})
        if (not recipe.get('enabled') or recipe.get('rendererShape') != 'sprite'
                or not element['detail']['particle']['localSpace']
                or element.get('sourceTransformTrack')):
            continue
        if any(m['className'] == 'particlemoduleorientationaxislock' and
               any(v['propertyPath'] == 'lockaxisflags' and v['value'] == 'epal_z'
                   for v in m['literals']) for m in recipe['modules']):
            element['detail']['sprite']['followEmitterAxisRotation'] = True


def repair_match_card(document):
    """Restore source loops/owner axes and apply the requested white front."""
    assert document['effectAssetId'] in {PREFIX + suit for suit, _, _, _ in SUITS}
    meshes = [e for e in document['elements'] if any(
        r['assetId'].endswith('/fm_l_rpct_rcard_01_sm.wmodel') for r in e['resources'])]
    fronts = [e for e in document['elements'] if any(
        t['assetId'].endswith('/fx_d_symbol_050.dds')
        for t in e['material']['sourceProfile'].get('textures', []))]
    assert len(meshes) == len(fronts) == 1, 'Expected one card mesh and front mask'
    # The original StartLocation is shared. Preserve each suit's edited front
    # placement; the separated diamond back must use that same owner origin.
    meshes[0]['detail']['transform']['position'] = copy.deepcopy(
        fronts[0]['detail']['transform']['position'])
    for element in document['elements']:
        required = next(m for m in element['sourceRecipe']['modules']
                        if m['className'] == 'particlemodulerequired')
        values = {v['propertyPath']: v['value'] for v in required['literals']}
        # Required's omitted native integer defaults to zero (repeat), not one.
        # Preserve explicit finite counts and the original 1s/2s particle life.
        element['sourceRecipe']['emitterLoopCount'] = int(values.get('emitterloops', 0))
    follow_local_card_axes(document)
    # PROJECT_AUTHORED exposure: native2967 and its gray DDS remain unchanged.
    # This changes RGB only; the original masked alpha threshold is preserved.
    fronts[0]['detail']['color']['emissiveIntensity'] = 4.0


def restore_spinning_card_afterimages(document):
    """Restore the original purple alpha afterimages requested after G26 suppression."""
    for element in document['elements']:
        if element.get('sourceNode', '').endswith(
                'fx_mn_rpct_05_l.par_l_rpct_05_sk_13_1_loc_int.particlespriteemitter_0'):
            assert element['material']['sourceProfile']['runtimeShaderProfileId'] == 'effect.ue3.kouku-2999-native.v1'
            element['visible'] = True


def stage_current_card_repairs(output, afterimages_only=False):
    """Patch only current saved documents; never rebuild edited source leaves."""
    output = output.resolve()
    assert output.is_relative_to((ROOT / 'out').resolve()), 'Candidates must stay under out'
    assets = [PREFIX + suit for suit, _, _, _ in SUITS] + [
        'effect.kouku.card.spinning.' + suit for suit, _, _, _ in SUITS] + [
        'effect.kouku.card.spinning.emitter', 'effect.kouku.common.spinning.card.throw']
    if afterimages_only:
        assets = ['effect.kouku.common.spinning.card.throw']
    rows = []
    for asset in assets:
        live = AUTHORED / (asset + '.effect.json')
        raw = live.read_bytes()
        original = json.loads(raw.decode('utf-8-sig'))
        candidate = copy.deepcopy(original)
        if afterimages_only:
            restore_spinning_card_afterimages(candidate)
            assert sum(e.get('sourceNode', '').endswith(
                'fx_mn_rpct_05_l.par_l_rpct_05_sk_13_1_loc_int.particlespriteemitter_0')
                for e in candidate['elements']) == 24
        elif asset.startswith(PREFIX):
            repair_match_card(candidate)
        else:
            follow_local_card_axes(candidate)
            restore_spinning_card_afterimages(candidate)
        if candidate == original:
            continue
        before_path, candidate_path = output / 'before' / live.name, output / 'candidate' / live.name
        before_path.parent.mkdir(parents=True, exist_ok=True)
        before_path.write_bytes(raw)
        write(candidate_path, candidate)
        assert live.read_bytes() == raw, 'Saved document changed during staging'
        rows.append(dict(path=live.relative_to(ROOT).as_posix(),
            beforeSha256=hashlib.sha256(raw).hexdigest(),
            beforePath=before_path.relative_to(ROOT).as_posix(),
            candidatePath=candidate_path.relative_to(ROOT).as_posix(),
            candidateSha256=hashlib.sha256(candidate_path.read_bytes()).hexdigest()))
    write(output / 'stage.json', dict(installed=False, documents=rows,
        whiteFrontBasis='PROJECT_AUTHORED_RGB_EXPOSURE_4',
        sourceLifetimePolicy='Original per-emitter lifetime; native Required loop count restored',
        afterimagePolicy='SOURCE_EMITTER_0_REENABLED_AFTER_USER_REQUEST_SUPERSEDED_G26',
        manualVisualValidation='USER_PENDING'))
    print(json.dumps(dict(candidates=len(rows), manifest=str(output / 'stage.json'))))


def build():
    documents, rows = [], []
    dice = renamed(native_stage(4219840, 0), PREFIX + 'dice.diamond.explosion', '주사위 다이아 폭발')
    repair_dice_continuity(dice)
    scale_dice_and_diamond_geometry(dice)
    documents.append((dice, '카드 짝 맞추기'))

    # Each source Trace projectile already supplies the matching card mesh,
    # symbol, local-space particles and exact per-suit native material inputs.
    # One repeatable second is the visual clock, not a gameplay expiration.
    for suit, label, suffix, tile in SUITS:
        card = renamed(leaf(f'08_{suffix}_loc_int'), PREFIX + suit, label)
        card.pop('sourceModelPreview', None)
        card.pop('sourceAnchorAnimations', None)
        for element in card['elements']:
            project_mesh_rotation(element)
        repair_match_card(card)
        documents.append((card, '카드 짝 맞추기'))

    explosion = renamed(leaf('08_7_loc_int'), PREFIX + 'explosion', '카드폭발')
    documents.append((explosion, '카드 짝 맞추기'))

    mouth = renamed(native_stage(4219840, 2), PREFIX + 'emit', '카드출력 이펙트')
    mouth['elements'] = [e for e in mouth['elements']
                         if '.par_l_rpct_05_sk_08_loc_int.' in e['sourceNode']]
    assert len(mouth['elements']) == 6
    for element in mouth['elements']:
        element['detail']['particle'].update(billboard=True, localSpace=True)
        axes = [v['value'] for m in element['sourceRecipe']['modules']
                if m['className'] == 'particlemoduleorientationaxislock'
                and not any(p['propertyPath'] == 'benabled' and p['value'] is False
                            for p in m['literals'])
                for v in m['literals'] if v['propertyPath'] == 'lockaxisflags']
        if any(axis.startswith('epal_rotate_') for axis in axes):
            element['detail']['sprite']['followEmitterAxisRotation'] = True
    documents.append((mouth, '카드 짝 맞추기'))

    # The six remaining elements are the user's saved edit. Do not restore
    # removed source mesh/emitters or change the saved transforms.
    spin_leaf = leaf('13_1_loc_int')
    for suit, label, suffix, tile in SUITS:
        spin = renamed(spin_leaf, 'effect.kouku.card.spinning.' + suit, '회전 카드_' + label)
        spin.pop('sourceModelPreview', None)
        spin.pop('sourceAnchorAnimations', None)
        card = next(e for e in spin['elements'] if any(
            t['assetId'].endswith('/fx_l_symbol_45.dds')
            for t in e['material']['sourceProfile'].get('textures', [])))
        fixed_subimage(card, tile)
        scale_spinning_card_geometry(spin)
        follow_local_card_axes(spin)
        restore_spinning_card_afterimages(spin)
        documents.append((spin, '세이튼_회전하며 카드 날리기'))

    trumpet = renamed(read(AUTHORED / 'effect.kouku.common.trumpet.radial.lasers.effect.json'),
                      'effect.kouku.common.trumpet.card.floor', '트럼펫_카드장판')
    trumpet['elements'] = []
    trumpet['sourceModelPreview'] = current_preview(47)
    append_group(trumpet, read(AUTHORED / 'effect.kouku.common.trumpet.radial.lasers.effect.json'),
                 'lasers', 4.184, 6.637)
    append_group(trumpet, read(AUTHORED / 'effect.kouku.common.trumpet.suit.floor.effect.json'),
                 'floor', 4.184, 6.637)
    # The original one-second suit particles disappear before the requested
    # 2.453-second floor interval. Preserve their recipe/material curves and
    # apply the existing authored lifetime multiplier to this floor group only.
    # The absolute alpha cutoff still retires every floor layer at impact.
    for element in trumpet['elements']:
        if element['groupId'].endswith('.floor'):
            element['detail']['particle']['sourceScale']['lifeTime'] = 2.453
    append_group(trumpet, leaf('06_2_loc_int'), 'impact', 6.637)
    documents.append((trumpet, '트럼펫장판소환'))

    for document, category in documents:
        assert len({e['id'] for e in document['elements']}) == len(document['elements'])
        assert len(document['displayName'].encode('utf8')) <= 64
        resources = set()
        for element in document['elements']:
            assert element['material']['sourceProfile']['enabled']
            for row in element['resources'] + element['material']['sourceProfile'].get('textures', []):
                resources.add(row['assetId'])
        missing = [p for p in resources if not (ROOT / 'Client/Bin/Resources' / p).is_file()]
        assert not missing, (document['effectAssetId'], missing)
        path = 'Data/Effects/Authored/' + document['effectAssetId'] + '.effect.json'
        write(OUTPUT / 'candidate' / Path(path).name, document)
        looping_card = document['effectAssetId'] in {
            prefix + suit for prefix in (PREFIX, 'effect.kouku.card.spinning.')
            for suit, _, _, _ in SUITS}
        rows.append(dict(effectAssetId=document['effectAssetId'], displayName=document['displayName'],
            path=path, durationMs=1000 if looping_card else duration_ms(document), elementCount=len(document['elements']),
            defaultAnchorKind='BOSS', followBoss=bool(document.get('sourceModelPreview')),
            categoryPath=['KoukuSaydon', '1관문', '패턴', '세이튼', category]))
    write(OUTPUT / 'installation.json', dict(installed=False, documents=rows,
        sourceActionDice=4219840, sourceDiceClip='rpct00_att_battle_11_02',
        sourceTraceProjectiles=[421980901, 421980902, 421980903, 421980904],
        sourceAtlas=dict(assetId='Effect/KoukuSaydon/Textures/FX_TEX_HIGH_03/fx_l_symbol_45.dds',
                         tileOrder=['clover', 'diamond', 'heart', 'spade'], turnsPerSecond=3),
        requestedTimingMs=dict(cardEmit=6114, trumpetStart=4184, trumpetImpact=6637),
        requestedTrumpetFloorLifetimeMultiplier=2.453,
        cardLifetimePolicy='Server-owned lifetime; Client repeats source presentation while alive.',
        sourceMouthSocket='FX_Prj_01 / b_wp_1 (original action attachment)',
        manualVisualValidation='USER_PENDING'))
    print(json.dumps(dict(candidates=len(rows), elements=sum(r['elementCount'] for r in rows),
                          manifest=str(OUTPUT / 'installation.json'))))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--dice-only', action='store_true')
    parser.add_argument('--cards-only', action='store_true')
    parser.add_argument('--restore-card-afterimages-only', action='store_true')
    parser.add_argument('--output', type=Path, default=ROOT / 'out/EffectV1Dice20260917/candidate')
    args = parser.parse_args()
    if args.restore_card_afterimages_only:
        assert not args.cards_only and not args.dice_only, 'Select one repair scope'
        stage_current_card_repairs(args.output, afterimages_only=True)
    elif args.cards_only:
        assert not args.dice_only, 'Select one repair scope'
        stage_current_card_repairs(args.output)
    elif args.dice_only:
        live = AUTHORED / (PREFIX + 'dice.diamond.explosion.effect.json')
        data = read(live)
        receipt = repair_dice_continuity(data)
        write(args.output / live.name, data)
        receipt.update(sourcePath=str(live), sourceSha256=hashlib.sha256(live.read_bytes()).hexdigest())
        write(args.output / 'dice-continuity.receipt.json', receipt)
    else:
        build()

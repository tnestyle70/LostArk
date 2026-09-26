"""Bake SCENE01B ending actors into candidate Character models, never live data.

The source AnimTrees explicitly route C over A over B. Read their single-bone
controls and the Matinee vector/strength curves before sampling; retain source
visibility, actor scale and bone attachments in normal World Sequence rows.
"""
from __future__ import annotations
import argparse
import copy
import hashlib
import math
from pathlib import Path
import sys

import numpy as np
from scipy.spatial.transform import Rotation

import build_gate2_intro_composition as base
from bake_character_cinematic_clips import animation_payloads, append_idempotent

ROOT = base.ROOT
DEFAULT_OUT = ROOT / 'out/KoukuEncoreFlow20260921'
SPECS = ((49, 37, 'MN_RPCT_05', 'saydon1', 2039),
         (63, 34, 'MN_RPCT_05', 'saydon2', 2039),
         (58, 35, 'MN_RPCZ_00', 'kouku1', 2041),
         (64, 36, 'MN_RPCZ_00', 'kouku2', 2041))
DURATION = 49083


def read_source(out):
    old = base.ue3.decode_property_value
    def decode(pt, st, payload, names, bv, pn=None, owner=None):
        if pt.lower() == 'arrayproperty' and pn and pn.lower().startswith('skelcontrolnamelist_'):
            reader = base.ue3.Reader(payload)
            return [base.ue3.parse_fname(reader, names)[0] for _ in range(reader.i32())]
        return old(pt, st, payload, names, bv, pn, owner)
    base.ue3.decode_property_value = decode
    try:
        rows, imports = base.extract_scene(base.PACKAGES / 'B9AVB2VAZIQRPQCJVKAVYRAVOKYPY8FD.upk')
    finally:
        base.ue3.decode_property_value = old
    base.write(out / 'SCENE01B.json', dict(rows=rows, imports=imports))
    tree_path = out / 'scene_a.json'
    if not tree_path.exists():
        sys.path.insert(0, str(ROOT / 'Tools/EffectPipeline'))
        import build_kouku_gate3_rainbow_native as native
        native.UMODEL = Path('C:/LostArkUModelP1C/runs/20260728T035441252Z-025d51a550a74a2b9e9729a1ad6313a6/input/tool/umodel_lostark_v7.exe')
        tree_rows, tree_imports = base.extract_scene(native.pkg('scene_a').path)
        base.write(tree_path, dict(rows=tree_rows, imports=tree_imports))
    trees = {int(k): v for k, v in base.read(tree_path)['rows'].items()}
    return rows, trees


def blend(lower, upper, alpha):
    alpha = float(np.clip(alpha, 0., 1.))
    if alpha == 0.: return lower
    if alpha == 1.: return upper
    return [(p*(1-alpha)+ap*alpha, base.slerp(q, aq, alpha), s*(1-alpha)+asc*alpha)
            for (p, q, s), (ap, aq, asc) in zip(lower, upper)]


def controls_for(model, tree, trees, tracks):
    names = set()
    for track in tracks:
        p = track['p']
        if track['cls'] == 'interptrackskelcontrolstrength': names.add(p['skelcontrolname'])
        if track['cls'] == 'efinterptrackskelcontrolvector':
            for k, v in p.items():
                if k.startswith('skelcontrolnamelist_'): names.update(v)
    indices = {b.name.casefold(): i for i, b in enumerate(model.skeleton_bones)}
    controls = []
    for chain in tree['p']['skelcontrollists']:
        idx = indices.get(chain['bonename'].casefold())
        ref = chain['controlhead']
        while ref:
            node = trees[ref]; p = node['p']; ref = p.get('nextcontrol', 0)
            if p.get('controlname') not in names: continue
            if idx is None: continue  # Alternative old/new facial bones in the source tree.
            assert node['cls'] == 'skelcontrolsinglebone', node
            assert p.get('bonetranslationspace', 'bcs_bonespace') in ('bcs_parentbonespace', 'bcs_bonespace')
            assert p.get('bonerotationspace', 'bcs_bonespace') in ('bcs_parentbonespace', 'bcs_bonespace')
            controls.append((idx, p))
    missing = names - {p['controlname'] for _, p in controls}
    assert not missing, ('unresolved source controls', missing)
    return controls


def strength_at(tracks, seconds):
    strength = {}
    for track in tracks:
        p = track['p']
        if track['cls'] == 'interptrackskelcontrolstrength':
            strength[p['skelcontrolname']] = float(base.curve(p.get('floattrack', {}).get('points', []), seconds, 0.))
        elif track['cls'] == 'efinterptrackskelcontrolvector':
            vector = base.curve(p.get('vectortrack', {}).get('points', []), seconds, [0, 0, 0])
            for axis, value in zip('xyz', vector):
                for sign, signed in (('positive', value), ('negative', -value)):
                    for name in p.get('skelcontrolnamelist_' + sign + axis, []):
                        strength[name] = max(0., float(signed))
    return strength


def control_translation_scale(model, body):
    # The imported Character armature expands metre-sized local bone translations
    # by 100. UE SkelControl offsets are centimetres and need the inverse once.
    armature = {'MN_RPCT_05': 'rpct00', 'MN_RPCZ_00': 'rpcz00'}[body]
    matches = [bone for bone in model.skeleton_bones if bone.name == armature]
    assert len(matches) == 1, ('missing cinematic armature', body)
    scale = np.linalg.norm(np.array(matches[0].transform).reshape(4, 4)[:3, :3], axis=1)
    assert np.allclose(scale, [100., 100., 100.], rtol=0., atol=.001), ('unexpected cinematic units', body, scale)
    return .01


def apply_controls(pose, controls, strengths, translation_scale):
    pose = list(pose)
    for index, control in controls:
        alpha = float(np.clip(strengths.get(control['controlname'], 0.), 0., 1.))
        if not alpha: continue
        p, q, s = pose[index]
        basis = Rotation.from_quat(q).as_matrix()
        if control.get('bapplytranslation'):
            delta = (base.BASIS @ base.vec(control.get('bonetranslation'))) * translation_scale
            if control.get('bonetranslationspace', 'bcs_bonespace') == 'bcs_bonespace': delta = basis @ delta
            p = p + delta*alpha if control.get('baddtranslation') else p*(1-alpha)+delta*alpha
        if control.get('bapplyrotation'):
            delta = base.BASIS @ base.actor_rotation({'rotation': control.get('bonerotation', {})}) @ base.BASIS.T
            if control.get('baddrotation'):
                delta = delta @ basis if control.get('bonerotationspace') == 'bcs_parentbonespace' else basis @ delta
            q = base.slerp(q, Rotation.from_matrix(delta).as_quat(), alpha)
        s = s*(1-alpha+alpha*control.get('bonescale', 1.))
        pose[index] = p, q, s
    return pose


def bake_one(rows, trees, spec, out):
    group, actor, body, label, tree_id = spec
    asset = f'Character/KoukuSaton/{body}/{body}.wmodel'
    source = base.RESOURCES / asset
    clip = 'kouku.bingo.ending.' + label
    metadata = base.wm.read_wmodel(source, include_geometry=False, animation_names=())
    needed = {'idle_normal_1'} | {key['animseqname'] for tr in base.active_tracks(rows, group) if tr['cls'] == 'interptrackanimcontrol' for key in tr['p'].get('animseqs', [])}
    names = [a.name for a in metadata.animations if base.clip_name(a.name) in needed]
    model = base.wm.read_wmodel(source, include_geometry=False, animation_names=names)
    clips = {base.clip_name(a.name): a for a in model.animations}
    tracks = base.active_tracks(rows, group)
    animation = {t['p']['slotname']: t['p'] for t in tracks if t['cls'] == 'interptrackanimcontrol'}
    assert set(animation) <= {'a', 'b', 'c'}
    controls = controls_for(model, trees[tree_id], trees, tracks)
    translation_scale = control_translation_scale(model, body)
    samples = []
    for frame in range(math.ceil(DURATION*.03)+1):
        seconds = min(DURATION/1000., frame/30.)
        pose = base.pose_sample(model, clips['idle_normal_1'], seconds % (clips['idle_normal_1'].duration_ticks/clips['idle_normal_1'].ticks_per_second))
        for slot in ('b', 'a', 'c'):
            track = animation.get(slot)
            if not track or seconds + 1e-6 < track['animseqs'][0]['starttime']: continue
            anim, time = base.anim_at(track, clips, seconds)
            weight = float(base.curve(track.get('floattrack', {}).get('points', []), seconds, 1.))
            pose = blend(pose, base.pose_sample(model, anim, time), weight)
        pose = apply_controls(pose, controls, strength_at(tracks, seconds), translation_scale)
        assert np.isfinite(np.array([np.concatenate(p) for p in pose])).all()
        samples.append(pose)
    donor = out / 'ending-donors' / (label + '.wmodel')
    baked = base.write_clip(source, donor, model, samples, clip, label)
    base.bind_bone_model(actor, baked, group, clip)
    print('baked', clip, len(samples), 'frames', flush=True)
    return asset, donor, clip, len(controls)


def world_rows(rows, spec, asset, clip):
    group, actor, _, label, _ = spec
    identity = 'kouku.bingo.ending.' + label
    p = rows[actor]['p']; tracks = base.active_tracks(rows, group)
    visibility = [key for t in tracks if t['cls'] == 'interptrackvisibility' for key in t['p'].get('visibilitytrack', [])]
    times = {round(frame*1000/30) for frame in range(math.ceil(DURATION*.03))} | {DURATION}
    for track in tracks:
        if track['cls'] != 'interptrackmove': continue
        for field in ('postrack', 'eulertrack'):
            for k in track['p'].get(field, {}).get('points', []):
                if 0 <= k['inval']*1000 <= DURATION:
                    ms = round(k['inval']*1000); times.update((max(0, ms-1), ms))
    for key in visibility:
        ms = round(key['time']*1000); times.update((max(0, ms-1), ms))
    keys = []
    for ms in sorted(times):
        pos, rotation = base.world_pose(rows, group, actor, ms/1000., 32, 45)
        visible = not p.get('bhidden', False)
        for key in visibility:
            if round(key['time']*1000) <= ms: visible = key['action'] == 'evta_show'
        scale = p.get('drawscale', 1.)
        keys.append(dict(timeMs=ms, positionOffset=pos.tolist(), rotationQuaternion=Rotation.from_matrix(rotation).as_quat().tolist(), scaleMultiplier=[scale]*3, visible=visible))
    # Hidden actor transforms cannot affect the image; retain every show/hide
    # boundary but do not spend the runtime key budget on a hidden attachment.
    keys = [key for i, key in enumerate(keys) if key['visible'] or i in (0, len(keys)-1) or
            (i and keys[i-1]['visible']) or (i+1 < len(keys) and keys[i+1]['visible'])]
    from build_gate2_intro_backdrops import reduced_indices
    keep = set(reduced_indices(np.array([k['timeMs'] for k in keys]), np.array([k['positionOffset'] for k in keys]), np.array([k['rotationQuaternion'] for k in keys])))
    for index, key in enumerate(keys):
        if index and key['visible'] != keys[index-1]['visible']: keep.update((index-1, index))
    keys = [keys[i] for i in sorted(keep)]
    assert len(keys) <= 256, (identity, len(keys))
    obj = dict(objectId='world.object.'+identity, displayName='빙고 최종엔딩 / '+label, modelAssetId=asset,
               animated=True, modelPreScale=.01, scale=[1, 1, 1], anchorKind='WORLD', diffuseTextureAssetId='',
               sequenceInstanceId='', defaultMotionInstanceId='world.sequence.instance.'+identity)
    template = dict(sequenceId='sequence.'+identity, displayName=obj['displayName'], category='World', durationMs=DURATION,
        interpolation='LINEAR', tracks=[dict(slotId='actor', keys=keys)],
        animationTracks=[dict(slotId='actor', clipName=clip, startMs=0, playbackRate=1, loop=False, holdLastFrame=True)])
    instance = dict(instanceId=obj['defaultMotionInstanceId'], templateId=template['sequenceId'], enabled=True, startDelayMs=0,
        playbackSpeed=1, anchorKind='WORLD', position=[0, 0, 0], motionEnd='STOP', nextMotionId='',
        bindings=[dict(slotId='actor', targetKind='OBJECT_RESOURCE', targetId=obj['objectId'])])
    return obj, template, instance


def build(out):
    out = out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    rows, trees = read_source(out)
    baked = [bake_one(rows, trees, spec, out) for spec in SPECS]
    patch = dict(objectResources=[], templates=[], instances=[], assets=[], source='SCENE01B Matinee 32 / data 45',
                 limitations=['Source material parameter dead is a separate material track, not a skeletal WANM channel.',
                              'Source weapon actor 33 references unavailable idle_normal_1 with no bound AnimSet; its source rest pose and movement/visibility are restored.'])
    for spec, (asset, donor, clip, controls) in zip(SPECS, baked):
        obj, template, instance = world_rows(rows, spec, asset, clip)
        for key, item in zip(('objectResources', 'templates', 'instances'), (obj, template, instance)): patch[key].append(item)
        patch['assets'].append(dict(assetId=asset, donor=str(donor.relative_to(ROOT)), clipName=clip, sourceGroup=spec[0], sourceActor=spec[1], appliedControls=controls))
    weapon = world_rows(rows, (59, 33, 'WP_MN_RPCT_05', 'weapon', 0),
                        'Character/KoukuSaton/WP_MN_RPCT_05/WP_MN_RPCT_05.wmodel', '')
    # The source component has no AnimSet and its package has no idle_normal_1.
    # Keep its real rest pose; never substitute the unrelated battle attack clip.
    weapon[1]['animationTracks'] = []
    for key, item in zip(('objectResources', 'templates', 'instances'), weapon): patch[key].append(item)
    for asset in {a[0] for a in baked}:
        raw = (base.RESOURCES/asset).read_bytes()
        additions = [(clip, animation_payloads(donor.read_bytes())[clip]) for target, donor, clip, _ in baked if target == asset]
        candidate, count = append_idempotent(raw, additions)
        assert append_idempotent(candidate, additions) == (candidate, 0)
        target = out / 'ending-resources' / asset; target.parent.mkdir(parents=True, exist_ok=True); target.write_bytes(candidate)
        patch.setdefault('candidates', []).append(dict(assetId=asset, path=str(target.relative_to(ROOT)), addedClips=count,
            baselineSha256=hashlib.sha256(raw).hexdigest(), sha256=hashlib.sha256(candidate).hexdigest()))
    base.write(out / 'ending-actor-field-patch.json', patch)
    return patch


def build_hand_reach(out):
    """Candidate only: repair the B-slot rewind without retiming the cinematic.

    The 30 Hz-aligned window keeps both installed endpoint poses. Only the
    native body clock changes; source layer weights and SkelControls retain
    their original cinematic time. Repetition is an authoring choice, not a
    default change made by this bake.
    """
    out = out.resolve()
    assert not out.is_relative_to(base.RESOURCES.resolve())
    out.mkdir(parents=True, exist_ok=True)
    rows, trees = read_source(out)
    start_ms, end_ms = 12900, 16400
    duration_ms = end_ms - start_ms
    name = 'kouku.bingo.ending.handreach'
    baked_name = 'kouku.bingo.ending.saydon1'
    asset = 'Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel'
    source = base.RESOURCES / asset
    raw = source.read_bytes()
    tracks = base.active_tracks(rows, 49)
    animation = {t['p']['slotname']: t['p'] for t in tracks
                 if t['cls'] == 'interptrackanimcontrol'}
    metadata = base.wm.read_wmodel(source, include_geometry=False, animation_names=())
    needed = {'idle_normal_1'} | {key['animseqname'] for track in animation.values()
                                for key in track['animseqs']}
    names = [a.name for a in metadata.animations
             if base.clip_name(a.name) in needed or a.name == baked_name]
    model = base.wm.read_wmodel(source, include_geometry=False, animation_names=names)
    clips = {base.clip_name(a.name): a for a in model.animations}
    old_baked = next(a for a in model.animations if a.name == baked_name)
    controls = controls_for(model, trees[2039], trees, tracks)
    control_scale = control_translation_scale(model, 'MN_RPCT_05')
    start, end = start_ms / 1000., end_ms / 1000.
    before_anim, source_start = base.anim_at(animation['b'], clips, start)
    after_anim, source_end = base.anim_at(animation['b'], clips, end)
    assert before_anim.name == after_anim.name == 'rpct00_evt2_atpain01'
    span = end - start
    source_delta = source_end - source_start
    minimum_source_rate = min(1., 1. + 1.5 * (source_delta / span - 1.))
    assert minimum_source_rate > 0.  # Quadratic derivative minimum over the entire interval.

    def source_clock(seconds):
        u = min(1., max(0., (seconds - start) / span))
        # Cubic Hermite, native seconds per cinematic second = 1 at both ends.
        clock = ((2*u**3 - 3*u**2 + 1)*source_start +
                 (u**3 - 2*u**2 + u)*span +
                 (-2*u**3 + 3*u**2)*source_end +
                 (u**3 - u**2)*span)
        rate = 1. + 6.*u*(1.-u)*(source_delta/span - 1.)
        return clock, rate

    samples, old_samples, source_samples = [], [], []
    for frame in range(duration_ms * 30 // 1000 + 1):
        seconds = start + frame / 30.
        clock, rate = source_clock(seconds)
        pose = base.pose_sample(model, clips['idle_normal_1'],
            seconds % (clips['idle_normal_1'].duration_ticks / clips['idle_normal_1'].ticks_per_second))
        for slot in ('b', 'a', 'c'):
            track = animation.get(slot)
            if not track or seconds + 1e-6 < track['animseqs'][0]['starttime']:
                continue
            anim, time = (before_anim, clock) if slot == 'b' else base.anim_at(track, clips, seconds)
            weight = float(base.curve(track.get('floattrack', {}).get('points', []), seconds, 1.))
            pose = blend(pose, base.pose_sample(model, anim, time), weight)
        pose = apply_controls(pose, controls, strength_at(tracks, seconds), control_scale)
        assert np.isfinite(np.array([np.concatenate(p) for p in pose])).all()
        samples.append(pose)
        old_samples.append(base.pose_sample(model, old_baked, seconds))
        source_samples.append(dict(timelineMs=seconds*1000., sourceMs=clock*1000., rate=rate))
    # Reuse the exact stored 30 Hz endpoint keys, including the old float32
    # quantization. Re-sampling a quaternion would unnecessarily normalize it.
    def endpoint_keys(ticks):
        channels = {channel.bone_index: channel for channel in old_baked.channels}
        result = []
        for bone in range(len(model.skeleton_bones)):
            channel = channels[bone]
            result.append(tuple(np.array(next(key[1:] for key in keys if key[0] == ticks))
                                for keys in (channel.position_keys, channel.rotation_keys, channel.scale_keys)))
        return result

    samples[0], samples[-1] = endpoint_keys(start_ms * 30 // 1000), endpoint_keys(end_ms * 30 // 1000)
    assert min(row['rate'] for row in source_samples) > 0.
    assert abs(source_samples[0]['rate'] - 1.) < 1e-12
    assert abs(source_samples[-1]['rate'] - 1.) < 1e-12
    assert np.all(np.diff([row['sourceMs'] for row in source_samples]) > 0.)
    donor = out / 'handreach-donor' / 'Saydon.wmodel'
    base.write_clip(source, donor, model, samples, name, 'saydon1')
    additions = [(name, animation_payloads(donor.read_bytes())[name])]
    candidate, count = append_idempotent(raw, additions)
    assert append_idempotent(candidate, additions) == (candidate, 0)
    target = out / 'candidate-resources' / asset
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_bytes(candidate)
    check = base.wm.read_wmodel(target, include_geometry=False, animation_names=(name,))
    fixed = next(a for a in check.animations if a.name == name)
    assert len(fixed.channels) == len(model.skeleton_bones) == 168
    assert fixed.duration_ticks == 105. and fixed.ticks_per_second == 30.
    for channel in fixed.channels:
        for coordinate, keys in enumerate((channel.position_keys, channel.rotation_keys, channel.scale_keys)):
            assert tuple(keys[0][1:]) == tuple(samples[0][channel.bone_index][coordinate])
            assert tuple(keys[-1][1:]) == tuple(samples[-1][channel.bone_index][coordinate])
    saved = [base.pose_sample(check, fixed, frame / 30.) for frame in range(106)]

    def pose_error(left, right):
        return float(max(np.max(np.abs(np.concatenate(a) - np.concatenate(b)))
                         for a, b in zip(left, right)))

    def rotation_delta(a, b):
        a, b = a / np.linalg.norm(a), b / np.linalg.norm(b)
        return math.degrees(2.*math.acos(min(1., abs(float(np.dot(a, b))))))

    def metrics(poses):
        result = []
        wanted = {'bip001-r-hand', 'bip001-r-forearm', 'bip001-r-upperarm',
                  'bip001-l-hand', 'bip001-l-forearm', 'bip001-l-upperarm'}
        combined = [base.wm.combined_transforms(model.skeleton_bones,
            [base.wm.affine_matrix(s, q, p) for p, q, s in pose]) for pose in poses]
        for index, bone in enumerate(model.skeleton_bones):
            angles = [rotation_delta(poses[i-1][index][1], poses[i][index][1])
                      for i in range(1, len(poses))]
            distances = [math.dist(combined[i-1][index][12:15], combined[i][index][12:15])*.01
                         for i in range(1, len(poses))]
            result.append(dict(bone=bone.name, maxLocalRotationDegrees=max(angles),
                               maxPivotStepMeters=max(distances), handWitness=bone.name in wanted))
        return result

    original_metrics, fixed_metrics = metrics(old_samples), metrics(saved)
    endpoints = [pose_error(saved[0], old_samples[0]), pose_error(saved[-1], old_samples[-1])]
    assert max(endpoints) < 1e-7, endpoints
    witnesses = []
    for old, new in zip(original_metrics, fixed_metrics):
        if not old['handWitness']:
            continue
        witnesses.append(dict(bone=old['bone'], before=old, after=new))
        assert new['maxLocalRotationDegrees'] < old['maxLocalRotationDegrees'], (old, new)
        assert new['maxPivotStepMeters'] < old['maxPivotStepMeters'], (old, new)
    world_path = ROOT / 'Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json'
    world_raw = world_path.read_bytes()
    world = base.read(world_path)
    template = next(r for r in world['templates'] if r['sequenceId'] == 'sequence.kouku.bingo.ending.saydon1')
    before = copy.deepcopy(template['animationTracks'])
    after = copy.deepcopy(before)
    selected = next(r for r in after if r['slotId'] == 'actor' and r['startMs'] == 12967)
    following = next(r for r in after if r['slotId'] == 'actor' and r['startMs'] == 16333)
    assert selected['clipName'] == following['clipName'] == baked_name
    assert selected.get('sourceStartMs') == 12967 and following.get('sourceStartMs') == 16333
    selected.update(startMs=start_ms, sourceStartMs=0, sourceEndMs=duration_ms,
                    clipName=name, displayName='Saydon hand reach (continuous)', loop=False)
    following.update(startMs=end_ms, sourceStartMs=end_ms)
    patch = dict(kind='animationTracks-field-patch', sequenceId=template['sequenceId'],
                 baselineRevision=world['revision'], baselineSha256=hashlib.sha256(world_raw).hexdigest(),
                 before=before, after=after,
                 changes=[dict(index=index, identity={key: old[key] for key in ('slotId', 'startMs', 'clipName')},
                               fields={key: dict(before=old.get(key), after=new.get(key))
                                       for key in sorted(old.keys() | new.keys()) if old.get(key) != new.get(key)})
                          for index, (old, new) in enumerate(zip(before, after)) if old != new],
                 unchanged=['template.durationMs=49083', 'WORLD transform/visibility/effects',
                            'all other actors', 'Sequence Composition', 'camera', 'sound', 'subtitles'])
    receipt = dict(clipName=name, assetId=asset, sourceNativeClip=before_anim.name,
                   sourceMatinee='SCENE01B group49 actor37 B track72',
                   appliedControls=len(controls), windowMs=[start_ms, end_ms],
                   durationTicks=fixed.duration_ticks, ticksPerSecond=fixed.ticks_per_second,
                   channelCount=len(fixed.channels), sourceClock=source_samples,
                   minimumSourceRate=minimum_source_rate,
                   endpointStoredKeyValuesIdentical=True,
                   endpointPoseMaximumComponentError=endpoints, handWitnesses=witnesses,
                   beforeAllBones=original_metrics, afterAllBones=fixed_metrics,
                   baselineSha256=hashlib.sha256(raw).hexdigest(), sha256=hashlib.sha256(candidate).hexdigest(),
                   candidate=str(target.relative_to(ROOT)), donor=str(donor.relative_to(ROOT)),
                   addedClips=count, existingSectionPayloadsPreserved=True,
                   repetition='Default non-loop. User chooses later repeat count/range; total P9 time is unchanged.',
                   manualVisualValidation='USER_PENDING')
    base.write(out / 'handreach-field-patch.json', patch)
    base.write(out / 'handreach-receipt.json', receipt)
    print('handreach candidate', target, 'endpoint errors', endpoints, flush=True)
    return receipt



def slice_baked_window(payload, first_tick, last_tick):
    """Trim a baked WANM, preserving interior values and interpolating cut keys.

    Matinee boxes use integer milliseconds, which are not always exact 30 Hz
    frames. Keep those boundaries instead of rounding and shifting their poses.
    """
    wm = base.wm
    nested = list(wm.FILE_HEADER.unpack_from(payload))
    header = list(wm.ANIMATION_HEADER.unpack_from(payload, wm.FILE_HEADER.size))
    assert nested[:3] == [b'WINT', 1, 0] and nested[-1] == len(payload) - wm.FILE_HEADER.size
    assert header[0] == b'WANM' and header[3] == 30. and header[5] == 0
    assert 0 <= first_tick < last_tick <= header[2]
    table_start = wm.FILE_HEADER.size + wm.ANIMATION_HEADER.size
    key_start = table_start + header[1] * wm.ANIMATION_CHANNEL.size
    channels, keys = bytearray(), bytearray()
    ticks = [first_tick] + list(range(math.floor(first_tick) + 1, math.ceil(last_tick))) + [last_tick]
    count = len(ticks)
    for index in range(header[1]):
        row = list(wm.ANIMATION_CHANNEL.unpack_from(payload, table_start + index * wm.ANIMATION_CHANNEL.size))
        for column, fmt in ((1, wm.VECTOR_KEY), (3, wm.QUATERNION_KEY), (5, wm.VECTOR_KEY)):
            source_count, source_offset = row[column:column + 2]
            assert source_count == int(header[2]) + 1 and last_tick < source_count
            assert key_start + source_offset + source_count * fmt.size <= len(payload) - 8
            row[column:column + 2] = count, len(keys)
            for tick in ticks:
                offset = key_start + source_offset + math.floor(tick) * fmt.size
                left = fmt.unpack_from(payload, offset)
                assert left[0] == math.floor(tick)
                if tick == math.floor(tick):
                    # Preserve every original float32 coordinate at stored keys.
                    keys += base.struct.pack('<f', float(tick - first_tick)) + payload[offset + 4:offset + fmt.size]
                else:
                    right = fmt.unpack_from(payload, offset + fmt.size)
                    assert right[0] == math.floor(tick) + 1
                    value = (wm.sample_quaternion([left, right], tick) if column == 3
                             else wm.sample_vector([left, right], tick, (0, 0, 0)))
                    keys += fmt.pack(float(tick - first_tick), *value)
        channels += wm.ANIMATION_CHANNEL.pack(*row)
    header[2], header[4] = float(last_tick - first_tick), header[1] * count * 3
    result = wm.ANIMATION_HEADER.pack(*header) + channels + keys + payload[-8:]
    nested[-1] = len(result)
    return wm.FILE_HEADER.pack(*nested) + result


def build_split_original_world(out, split_source, original_donors):
    """Prepare independent native clips and a narrow original WORLD field patch.

    This reads the latest world and models but never installs or publishes.
    Camera, sound, subtitles, object size, and all other patterns stay unchanged.
    """
    from bake_character_cinematic_clips import validate_keys
    out, split_source, original_donors = out.resolve(), split_source.resolve(), original_donors.resolve()
    assert not out.is_relative_to(base.RESOURCES.resolve())
    out.mkdir(parents=True, exist_ok=True)
    world_path = ROOT / 'Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json'
    world_raw = world_path.read_bytes()
    world, baseline = base.read(world_path), base.read(split_source)
    candidate_world = copy.deepcopy(world)
    before_templates = {row['sequenceId']: row for row in world['templates']}
    source_templates = {row['sequenceId']: row for row in baseline['templates']}
    target_templates = {row['sequenceId']: row for row in candidate_world['templates']}
    model_bytes, additions, windows, changes = {}, {}, [], []
    for _, _, body, label, _ in SPECS:
        baked_name = 'kouku.bingo.ending.' + label
        identity = 'sequence.' + baked_name
        asset = f'Character/KoukuSaton/{body}/{body}.wmodel'
        if asset not in model_bytes:
            model_bytes[asset] = (base.RESOURCES / asset).read_bytes()
        raw = model_bytes[asset]
        source_payload = animation_payloads(raw)[baked_name]
        donor = original_donors / (label + '.wmodel')
        # A fresh source bake proves the scene controls are still intact.
        assert animation_payloads(donor.read_bytes())[baked_name] == source_payload, ('original bake differs', label)
        old, target = source_templates[identity], target_templates[identity]
        assert old['durationMs'] == DURATION
        old_rows = old['animationTracks']
        assert len(old_rows) == {'saydon1': 11, 'saydon2': 4, 'kouku1': 4, 'kouku2': 6}[label]
        after = []
        for index, row in enumerate(old_rows):
            start = row['startMs']
            end = old_rows[index + 1]['startMs'] if index + 1 < len(old_rows) else DURATION
            assert row['clipName'] == baked_name and row.get('sourceStartMs', 0) == start
            assert row['playbackRate'] == 1 and not row['loop'] and end > start
            name = baked_name + f'.{index:02d}'
            payload = slice_baked_window(source_payload, start * .03, end * .03)
            additions.setdefault(asset, []).append((name, payload))
            track = copy.deepcopy(row)
            track.update(clipName=name, sourceStartMs=0, sourceEndMs=end-start, loop=False)
            after.append(track)
            windows.append(dict(actor=label, assetId=asset, clipName=name,
                                sourceClip=baked_name, startMs=start, endMs=end,
                                durationMs=end-start, originalSourcePayloadSha256=hashlib.sha256(source_payload).hexdigest()))
        target['animationTracks'] = after
        if label == 'saydon1':
            # Previous Duplicate appended only an invisible held transform key.
            # Refuse to discard any independent user edit inside the scene.
            assert len(target['tracks']) == len(old['tracks'])
            for current_track, old_track in zip(target['tracks'], old['tracks']):
                assert {k: v for k, v in current_track.items() if k != 'keys'} == {k: v for k, v in old_track.items() if k != 'keys'}
                assert current_track['keys'][:len(old_track['keys'])] == old_track['keys']
                assert all(k['timeMs'] > DURATION and not k['visible'] for k in current_track['keys'][len(old_track['keys']):])
            target['durationMs'], target['tracks'] = DURATION, copy.deepcopy(old['tracks'])
        else:
            assert target['durationMs'] == DURATION and target['tracks'] == old['tracks']
        before = before_templates[identity]
        fields = {field: dict(before=copy.deepcopy(before[field]), after=copy.deepcopy(target[field]))
                  for field in ('animationTracks', 'durationMs', 'tracks') if before[field] != target[field]}
        changes.append(dict(sequenceId=identity, fields=fields))
    candidates, max_error = [], 0.
    for asset, extra in additions.items():
        raw = model_bytes[asset]
        candidate, count = append_idempotent(raw, extra)
        assert append_idempotent(candidate, extra) == (candidate, 0)
        destination = out / 'candidate-resources' / asset
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(candidate)
        names = {name for name, _ in extra} | {r['sourceClip'] for r in windows if r['assetId'] == asset}
        model = base.wm.read_wmodel(destination, include_geometry=False, animation_names=names)
        animations = {a.name: a for a in model.animations}
        for window in (r for r in windows if r['assetId'] == asset):
            cropped, original = animations[window['clipName']], animations[window['sourceClip']]
            window['keyCount'] = validate_keys(cropped)
            assert len(cropped.channels) == len(original.channels) == len(model.skeleton_bones)
            assert abs(cropped.duration_ticks / cropped.ticks_per_second * 1000. - window['durationMs']) < .002
            # Check both cuts, every retained key, and interval midpoints.
            sample_ticks = sorted({k[0] for c in cropped.channels for k in c.position_keys})
            sample_ticks += [(a+b)*.5 for a, b in zip(sample_ticks, sample_ticks[1:])]
            error = 0.
            for old_channel, new_channel in zip(original.channels, cropped.channels):
                assert old_channel.bone_index == new_channel.bone_index
                for local_tick in sample_ticks:
                    original_tick = window['startMs'] * .03 + local_tick
                    for field in ('position_keys', 'scale_keys', 'rotation_keys'):
                        old_keys, new_keys = getattr(old_channel, field), getattr(new_channel, field)
                        if field == 'rotation_keys':
                            left, right = np.array(base.wm.sample_quaternion(old_keys, original_tick)), np.array(base.wm.sample_quaternion(new_keys, local_tick))
                            if float(np.dot(left, right)) < 0.: right = -right
                        else:
                            left, right = np.array(base.wm.sample_vector(old_keys, original_tick, (0, 0, 0))), np.array(base.wm.sample_vector(new_keys, local_tick, (0, 0, 0)))
                        error = max(error, float(np.max(np.abs(left-right))))
            # A float32 timestamp rebase can introduce sub-microsecond rounding.
            assert error < .0001, (window['clipName'], error)
            window.update(channelCount=len(cropped.channels), poseSamplesChecked=len(sample_ticks), maximumPoseComponentError=error)
            max_error = max(max_error, error)
        assert (base.RESOURCES / asset).read_bytes() == raw, 'Model changed during candidate preparation'
        candidates.append(dict(assetId=asset, baselineSha256=hashlib.sha256(raw).hexdigest(),
                               sha256=hashlib.sha256(candidate).hexdigest(), addedClips=count,
                               candidate=str(destination.relative_to(ROOT))))
    candidate_world['revision'] = world['revision'] + 1
    reverse = copy.deepcopy(candidate_world)
    reverse['revision'] = world['revision']
    reverse_templates = {r['sequenceId']: r for r in reverse['templates']}
    for change in changes:
        for field, values in change['fields'].items():
            reverse_templates[change['sequenceId']][field] = values['before']
    assert reverse == world, 'Unrelated world data changed'
    assert world_path.read_bytes() == world_raw, 'World changed during candidate preparation'
    world_candidate = out / 'worldsequences.original-split.candidate.json'
    base.write(world_candidate, candidate_world)
    patch = dict(baselineRevision=world['revision'], baselineSha256=hashlib.sha256(world_raw).hexdigest(),
                 revision=candidate_world['revision'], changes=changes,
                 candidate=str(world_candidate.relative_to(ROOT)))
    receipt = dict(clipCount=len(windows), windows=windows, models=candidates,
                   maximumPoseComponentError=max_error, worldPatch='world-field-patch.json',
                   allExistingModelSectionsIdentical=True, originalSourceBakeIdentical=True,
                   otherWorldFieldsIdentical=True, compositionModified=False, installed=False,
                   manualVisualValidation='USER_PENDING')
    base.write(out / 'world-field-patch.json', patch)
    base.write(out / 'original-split-receipt.json', receipt)
    print('original split candidate', len(windows), 'clips; maximum pose component error', max_error, flush=True)
    return receipt


def build_original_hand_reach(out):
    """Prepare one original-scene clip choice without changing live authoring."""
    from bake_character_cinematic_clips import validate_keys
    out = out.resolve()
    assert not out.is_relative_to(base.RESOURCES.resolve())
    out.mkdir(parents=True, exist_ok=True)
    asset = 'Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel'
    source = base.RESOURCES / asset
    raw = source.read_bytes()
    baked_name = 'kouku.bingo.ending.saydon1'
    name = 'kouku.bingo.ending.handreach.original'
    first, last = 387, 492
    payload = slice_baked_window(animation_payloads(raw)[baked_name], first, last)
    additions = [(name, payload)]
    candidate, count = append_idempotent(raw, additions)
    assert append_idempotent(candidate, additions) == (candidate, 0)
    target = out / 'candidate-resources' / asset
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_bytes(candidate)
    model = base.wm.read_wmodel(target, include_geometry=False, animation_names=(baked_name, name))
    original, cropped = (next(a for a in model.animations if a.name == clip) for clip in (baked_name, name))
    assert len(original.channels) == len(cropped.channels) == len(model.skeleton_bones) == 168
    assert cropped.duration_ticks == 105. and cropped.ticks_per_second == 30.
    key_count = validate_keys(cropped)
    for before, after in zip(original.channels, cropped.channels):
        assert before.bone_index == after.bone_index
        for left, right in zip((before.position_keys, before.rotation_keys, before.scale_keys),
                               (after.position_keys, after.rotation_keys, after.scale_keys)):
            assert len(right) == 106
            assert all(a[0] - first == b[0] and a[1:] == b[1:]
                       for a, b in zip(left[first:last + 1], right))
    max_error = 0.
    # Include every stored frame and every midpoint to check interpolation.
    for half_frame in range(211):
        seconds = half_frame / 60.
        old_pose = base.pose_sample(model, original, first / 30. + seconds)
        new_pose = base.pose_sample(model, cropped, seconds)
        error = max(float(np.max(np.abs(np.concatenate(a) - np.concatenate(b))))
                    for a, b in zip(old_pose, new_pose))
        max_error = max(max_error, error)
    assert max_error < 1e-7, max_error
    assert source.read_bytes() == raw, 'Source model changed while preparing candidate'
    receipt = dict(clipName=name, assetId=asset, sourceClip=baked_name,
                   sourceMatinee='SCENE01B group49 actor37; original slot blend and SkelControls',
                   windowMs=[12900, 16400], durationTicks=105, ticksPerSecond=30,
                   channelCount=168, keyCount=key_count, sampleCountPerChannel=106,
                   originalStoredKeyValuesIdentical=True, poseSamplesChecked=211,
                   maximumPoseComponentError=max_error, existingSectionPayloadsPreserved=True,
                   baselineSha256=hashlib.sha256(raw).hexdigest(),
                   sha256=hashlib.sha256(candidate).hexdigest(), addedClips=count,
                   nativeClipCount=len(model.animations),
                   candidate=str(target.relative_to(ROOT)),
                   authoringModified=False, installed=False, manualVisualValidation='USER_PENDING')
    base.write(out / 'original-handreach-receipt.json', receipt)
    print('original handreach candidate', target, 'pose error', max_error, flush=True)
    return receipt


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__); parser.add_argument('--output', type=Path, default=DEFAULT_OUT)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument('--original-hand-reach', action='store_true', help='Prepare the original 3.5-second hand-reach clip choice')
    mode.add_argument('--hand-reach', action='store_true', help='Prepare only the continuous hand-reach clip candidate')
    mode.add_argument('--split-original-world', action='store_true', help='Prepare original timeline boxes as independent native clips')
    parser.add_argument('--split-source', type=Path, help='Original WORLD document containing the 25 split boundaries')
    parser.add_argument('--original-donors', type=Path, help='Freshly source-baked ending-donors directory')
    args = parser.parse_args()
    if args.split_original_world:
        if args.split_source is None or args.original_donors is None:
            parser.error('--split-original-world requires --split-source and --original-donors')
        build_split_original_world(args.output, args.split_source, args.original_donors)
    else:
        (build_original_hand_reach if args.original_hand_reach else
         build_hand_reach if args.hand_reach else build)(args.output)

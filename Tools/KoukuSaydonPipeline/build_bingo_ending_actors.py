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


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__); parser.add_argument('--output', type=Path, default=DEFAULT_OUT)
    args = parser.parse_args(); build(args.output)

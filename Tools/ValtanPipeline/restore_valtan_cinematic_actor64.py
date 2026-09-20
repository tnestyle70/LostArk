"""Restore the original SCENE06A/53 supporting actor using existing model tracks.

Inputs are the measured UModel glTF/PSA cook and source Matinee dump. This
offline assembly adds no gameplay entity or alternate renderer.
"""
from pathlib import Path
import argparse
import copy
import json
import math
import shutil
import sys

import numpy as np
from scipy.spatial.transform import Rotation

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'Tools/EffectPipeline'), str(ROOT / 'Tools/KoukuSaydonPipeline')]
import build_valtan_full_restore as writer
import bake_valtan_original_cinematic_actors as bake
from restore_valtan_cinematic_static_effects import intervals

source = bake.source
PREFIX = 'Map/Valtan/Cinematics/Actor64/'
BODY = PREFIX + 'Body/MN_TSLC_00-1.wmodel'
WEAPON = PREFIX + 'Weapon/WP_MN_TSLC_00-1.wmodel'
CLIP = 'valtan.cinematic.actor64.entrance'
OBJECT = 'world.object.valtan.source-preview.actor64.'


def split_actor64_instances(document):
    """Keep the existing single-object FX/pool contract and one source clock."""
    entrance = next(i for i in document['instances'] if i['instanceId'] ==
                    'world.sequence.instance.valtan.source-preview.entrance')
    source_template = next(t for t in document['templates'] if t['sequenceId'] == entrance['templateId'])
    moved = [b for b in entrance['bindings'] if b['slotId'].startswith('actor64.')]
    for binding in moved:
        slot = binding['slotId']
        suffix = 'entrance.' + slot
        instance_id = 'world.sequence.instance.valtan.source-preview.' + suffix
        template_id = 'sequence.LV_LUT_HEARTRB_ED.valtan.source-preview.' + suffix
        template = copy.deepcopy(source_template)
        template.update(sequenceId=template_id, displayName='Valtan original supporting actor / ' + slot)
        for lane in ('tracks', 'animationTracks', 'effectTracks', 'colliderTracks'):
            if lane in template:
                template[lane] = [t for t in template[lane] if t['slotId'] == slot]
                source_template[lane] = [t for t in source_template[lane] if t['slotId'] != slot]
        instance = copy.deepcopy(entrance)
        instance.update(instanceId=instance_id, templateId=template_id, bindings=[copy.deepcopy(binding)])
        assert not any(i['instanceId'] == instance_id for i in document['instances'])
        document['templates'].append(template)
        document['instances'].append(instance)
    entrance['bindings'] = [b for b in entrance['bindings'] if not b['slotId'].startswith('actor64.')]
    return len(moved)


def prepare(cook, evidence):
    scene_path = bake.SCENES / 'LV_LUT_HEARTRB_ED_SCENE06A.json'
    scene = source.read(scene_path)
    rows = {int(k): dict(v, p=source.unwrap(v['p'])) for k, v in scene['rows'].items()}
    assert rows[64]['p']['lookinfokey'] == 'EFDLChar_MN_TSLC_00-1.MN_TSLC_00-1'
    assert scene['imports'][str(rows[815]['p']['materials'][0])].endswith('mn_tslc_00-1_mi_dead')
    assert scene['imports'][str(rows[816]['p']['materials'][0])].endswith('wp_mn_tslc_00-1_mi_dead')
    actor_scale = rows[64]['p']['drawscale']
    raw = (cook / 'Body/MN_TSLC_00-1.wmodel').read_bytes()
    model = bake.wm.read_wmodel(cook / 'Body/MN_TSLC_00-1.wmodel')
    tree, tree_source = bake.inherited_cinematic_tree()
    times, samples, animation_receipt = bake.bake(model, rows, 133, 24.708, tree)
    trailer = next(p[-8:] for kind, _, _, p in bake.sections(raw) if kind == 4)
    payload = bake.animation_payload(model, times, samples, trailer, bake.wm.FILE_HEADER.unpack_from(raw)[2])
    body = bake.append(raw, [(CLIP, payload)])
    evidence.mkdir(parents=True, exist_ok=True)
    body_candidate = evidence / BODY
    body_candidate.parent.mkdir(parents=True, exist_ok=True)
    body_candidate.write_bytes(body)
    for label in ['Body', 'Weapon']:
        shutil.copytree(cook / label / 'textures', evidence / PREFIX / label / 'textures', dirs_exist_ok=True)
    model = bake.wm.read_wmodel(body_candidate)
    clip = next(a for a in model.animations if a.name == CLIP)
    grip = next(i for i, b in enumerate(model.skeleton_bones) if b.name == 'b_wp_1')
    attachment = rows[815]['p']['attachments'][0]
    assert attachment['bonename'] == 'b_wp_1' and attachment['component'] == 816
    assert source.vec(attachment['relativelocation']).tolist() == [0., 0., 0.]
    assert source.vec(attachment['relativescale']).tolist() == [1., 1., 1.]

    path = ROOT / 'Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.worldsequences.json'
    before = path.read_bytes()
    document = json.loads(before)
    instance = next(i for i in document['instances'] if i['instanceId'] == 'world.sequence.instance.valtan.source-preview.entrance')
    template = next(t for t in document['templates'] if t['sequenceId'] == instance['templateId'])
    assert template['durationMs'] == 24708
    origin = np.array(instance['position'])
    discontinuities = set()
    for track in source.active_tracks(rows, 133):
        for field in ['postrack', 'eulertrack']:
            for point in track['p'].get(field, {}).get('points', []):
                exact = point['inval'] * 1000
                discontinuities.update([max(0, math.ceil(exact) - 1), math.ceil(exact)])
    sample_times = np.array(sorted(set(round(t * 1000) for t in times) |
        set(source.source_times(rows, 133, end=24708)) | discontinuities), dtype=int)
    positions, quaternions, grip_positions, grip_quaternions = [], [], [], []
    for ms in sample_times:
        position, rotation = source.world_pose(rows, 133, 64, ms / 1000., 53, 113)
        positions.append(position - origin)
        quaternions.append(Rotation.from_matrix(rotation).as_quat())
        pose = source.pose_sample(model, clip, ms / 1000.)
        combined = np.array(bake.wm.combined_transforms(model.skeleton_bones,
            [bake.wm.affine_matrix(s, q, p) for p, q, s in pose])[grip]).reshape(4, 4)
        basis = combined[:3, :3]
        assert np.allclose(np.linalg.norm(basis, axis=1), [1, 1, 1], atol=1e-5)
        grip_positions.append(position - origin + rotation @ (combined[3, :3] * .01 * actor_scale))
        grip_quaternions.append(Rotation.from_matrix(rotation @ basis.T).as_quat())
    generated_tracks, bindings = [], []
    counts = {}
    for label, ps, qs in [('body', positions, quaternions), ('weapon', grip_positions, grip_quaternions)]:
        ps, qs = np.array(ps), np.array(qs)
        keep = source.reduced_indices(sample_times, ps, qs)
        counts[label] = len(keep)
        # Each track stays below the existing 256-key limit. Non-overlapping
        # visibility windows permit dense original socket motion without a
        # new runtime attachment format or relaxing the document bound.
        for ordinal, offset in enumerate(range(0, len(keep) - 1, 250)):
            part = keep[offset:offset + 251]
            start, end = int(sample_times[part[0]]), int(sample_times[part[-1]])
            slot = 'actor64.' + label + '.' + str(ordinal)
            def key(index, ms=None, visible=True):
                q = qs[index]
                if q[3] < 0: q = -q
                return dict(timeMs=int(sample_times[index]) if ms is None else ms,
                    positionOffset=ps[index].tolist(), rotationQuaternion=q.tolist(),
                    scaleMultiplier=[actor_scale] * 3, visible=visible)
            keys = [key(i) for i in part]
            if start:
                keys = [key(part[0], 0, False), key(part[0], start - 1, False)] + keys
            if end < 24708:
                keys[-1]['visible'] = False
                keys.append(key(part[-1], 24708, False))
            assert len(keys) <= 256 and len({k['timeMs'] for k in keys}) == len(keys)
            generated_tracks.append(dict(slotId=slot, keys=keys))
            bindings.append(dict(slotId=slot, targetKind='OBJECT_RESOURCE', targetId=OBJECT + label))
    for label, asset, animated in [('body', BODY, True), ('weapon', WEAPON, False)]:
        assert not any(r['objectId'] == OBJECT + label for r in document['objectResources'])
        document['objectResources'].append(dict(objectId=OBJECT + label,
            displayName='발탄 진입 / 원본 일리아칸 ' + label, modelAssetId=asset,
            materialSourceModelAssetId=asset, modelPreScale=.01, animated=animated,
            anchorKind='WORLD', scale=[1, 1, 1], diffuseTextureAssetId='', sequenceInstanceId='', defaultMotionInstanceId=''))
    template['tracks'].extend(generated_tracks)
    instance['bindings'].extend(bindings)
    template['animationTracks'].extend(dict(slotId=t['slotId'], clipName=CLIP,
        startMs=0, sourceStartMs=0, playbackRate=1., loop=False, holdLastFrame=True)
        for t in generated_tracks if t['slotId'].startswith('actor64.body.'))
    effects = []
    for actor, group in [(112, 134), (72, 135)]:
        properties = rows[actor]['p']
        component = rows[properties['particlesystemcomponent']]['p']
        asset = 'effect.valtan.source.' + scene['imports'][str(component['template'])]
        assert (ROOT / 'Data/Effects/Authored' / (asset + '.effect.json')).is_file()
        tracks = source.active_tracks(rows, group)
        move = next(t['p'] for t in tracks if t['cls'] == 'interptrackmove')
        assert len(move['postrack']['points']) == len(move['eulertrack']['points']) == 1
        location = source.BASIS @ source.vec(move['postrack']['points'][0]['outval']) / actor_scale
        rotation = source.BASIS @ source.rotation(source.vec(move['eulertrack']['points'][0]['outval'])) @ source.BASIS.T
        yaw, pitch, roll = Rotation.from_matrix(rotation).as_euler('YXZ', degrees=True)
        bone = properties['basebonename']
        assert any(b.name == bone for b in model.skeleton_bones)
        toggles = next(t['p']['toggletrack'] for t in tracks if t['cls'] == 'interptracktoggle')
        for ordinal, start, end, loop in intervals(toggles, template['durationMs']):
            effects.append(dict(effectTrackId=f'fx.valtan.source.entrance.{actor}.{ordinal}',
                slotId='actor64.body.0', resourceKind='V1_EFFECT', resourceId=asset,
                timing='TIME', startMs=start, durationMs=end-start, followObject=True,
                inheritObjectRotation=True, loopEffectToDuration=loop, bone=bone,
                positionOffset=location.tolist(), rotationDegrees=[pitch, yaw, roll],
                scale=[1. / (.01 * actor_scale)] * 3))
    template.setdefault('effectTracks', []).extend(effects)
    assert split_actor64_instances(document) == len(generated_tracks)
    document['revision'] += 1
    writes = [(path, before, writer.encode_json(document))]
    for relative, contents in [(BODY, body), (WEAPON, (cook / 'Weapon/WP_MN_TSLC_00-1.wmodel').read_bytes())]:
        candidate = evidence / relative
        candidate.parent.mkdir(parents=True, exist_ok=True)
        candidate.write_bytes(contents)
        target = bake.RESOURCES / relative
        writes.append((target, target.read_bytes() if target.exists() else None, contents))
    for label in ['Body', 'Weapon']:
        for texture in (cook / label / 'textures').iterdir():
            target = bake.RESOURCES / PREFIX / label / 'textures' / texture.name
            writes.append((target, target.read_bytes() if target.exists() else None, texture.read_bytes()))
    (evidence / 'worldsequences.before.json').write_bytes(before)
    (evidence / 'worldsequences.candidate.json').write_bytes(writer.encode_json(document))
    receipt = dict(sourceScene=str(scene_path), sourceSceneSha256=scene['sha256'],
        actor=64, group=133, matinee=53, component=815, weaponComponent=816,
        bakedAnimation=animation_receipt, inheritedTree=tree_source, sourceCookSha256=bake.sha(raw),
        bodySha256=bake.sha(body), samples=len(sample_times), reducedKeys=counts,
        trackCount=len(generated_tracks), visualStatus='USER_PENDING',
        materialStatus='SCENE_DEAD_MIC_BINDING_REQUIRED', attachedEffects=effects)
    (evidence / 'receipt.json').write_bytes(writer.encode_json(receipt))
    return writes, receipt


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--cook-root', required=True, type=Path)
    parser.add_argument('--evidence-root', required=True, type=Path)
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    writes, receipt = prepare(args.cook_root, args.evidence_root)
    if args.install: writer.commit_writes(writes)
    print(json.dumps(dict(installed=args.install, tracks=receipt['trackCount'], keys=receipt['reducedKeys'])))


if __name__ == '__main__':
    main()

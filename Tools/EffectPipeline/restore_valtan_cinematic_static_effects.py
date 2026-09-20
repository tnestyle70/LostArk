"""Restore measured, stationary original Matinee FX into the saved Valtan cuts.

This is a bounded reuse pass. Moving/bone-attached emitters and unavailable
source systems remain explicit in the receipt, never frozen or substituted.
The existing WorldSequencePlayer owns both editor playback and V1 FX sampling.
"""
from pathlib import Path
import argparse
import copy
import json
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'Tools/KoukuSaydonPipeline'), str(ROOT / 'Tools/LevelPlacementExtractor')]
import build_gate2_intro_composition as original
import build_valtan_full_restore as writer

SCENES = (
    ('LV_LUT_HEARTRB_ED_SCENE06A', 53, 'entrance'),
    ('LV_LUT_HEARTRB_ED_SCENE06A', 54, 'trash'),
    ('LV_LUT_HEARTRB_ED_SCENE06A', 55, 'roar'),
    ('LV_LUT_HEARTRB_ED_SCENE04A', 24, 'finale'),
    ('LV_LUT_HEARTRB_ED_SCENE02A', 52, 'phase2'),
)

# Actual CEffectDocumentCodec admission, not a source-name heuristic. Preserve
# the original recipe for a future exact module consumer; do not let one
# unsupported source system abort prewarming every admitted cinematic lane.
ADMISSION_DEFERRED = {
    'effect.valtan.source.bfx_low_03.lightning.par_c_lightning_001':
        'UNSUPPORTED_SOURCE_MODULE_particlemodulesubuvdirect',
    'effect.valtan.cinematic.roar.actor98.at4889':
        'UNSUPPORTED_SOURCE_MODULE_particlemodulesubuvdirect',
    'effect.valtan.source.fx_npc_m_00.par_m_ghostmeteor_01':
        'SOURCE_LOCATIONEMITTER_FAMILY_CARDINALITY_NOT_ADMITTED',
    'effect.valtan.source.fx_npc_m_00.par_m_gravityarea_01_1':
        'UNSUPPORTED_SOURCE_MODULE_particlemodulemeshmaterial',
    'effect.valtan.source.fx_npc_m_00.par_m_ghostmeteor_loop_02':
        'SOURCE_DETAIL_ZERO_LIFETIME_BOUND_NOT_ADMITTED',
    'effect.valtan.source.scene_a.fx.par_j_arktrail_02_cine':
        'SOURCE_DETAIL_ZERO_LIFETIME_BOUND_NOT_ADMITTED',
}


def position_at(template, instance, milliseconds):
    keys = template['tracks'][0]['keys']
    assert template['interpolation'] == 'LINEAR'
    left = next((k for k in reversed(keys) if k['timeMs'] <= milliseconds), keys[0])
    right = next((k for k in keys if k['timeMs'] > milliseconds), left)
    alpha = ((milliseconds - left['timeMs']) / (right['timeMs'] - left['timeMs'])
             if right['timeMs'] != left['timeMs'] else 0)
    assert left['scaleMultiplier'] == right['scaleMultiplier'] == [1, 1, 1]
    return original.np.array(instance['position']) + original.np.array(left['positionOffset']) * (1-alpha) + original.np.array(right['positionOffset']) * alpha


def intervals(keys, duration):
    ordered = sorted(enumerate(keys), key=lambda pair: (pair[1]['time'], pair[0]))
    for ordinal, (_, key) in enumerate(ordered):
        if key['toggleaction'] not in ('etta_on', 'etta_trigger'):
            continue
        start = max(0, round(key['time'] * 1000))
        if start >= duration:
            continue
        end = next((min(duration, max(0, round(later['time'] * 1000)))
                    for _, later in ordered[ordinal+1:]
                    if later['toggleaction'] in ('etta_off', 'etta_trigger')), duration)
        if end > start:
            yield ordinal, start, end, key['toggleaction'] == 'etta_on'


def prepare(source_root):
    path = ROOT / 'Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.worldsequences.json'
    before = path.read_bytes(); document = json.loads(before)
    templates = {t['sequenceId']: t for t in document['templates']}
    instances = {i['instanceId']: i for i in document['instances']}
    resources = {r['objectId']: r for r in document['objectResources']}
    installed = {p.stem.removesuffix('.effect'): p for p in (ROOT/'Data/Effects/Authored').glob('*.effect.json')}
    receipt = dict(restored=[], deferred=[], visualStatus='USER_PENDING')
    infinite_sources = {}
    def source_is_infinite(asset):
        if asset not in infinite_sources:
            effect = json.loads(installed[asset].read_bytes())
            infinite_sources[asset] = any(e.get('sourceRecipe', {}).get('enabled') and
                e['sourceRecipe'].get('emitterLoopCount') == 0 for e in effect['elements'])
        return infinite_sources[asset]
    for scene, matinee, name in SCENES:
        source = json.loads((source_root / (scene + '.json')).read_bytes())
        rows = {int(k): v for k, v in source['rows'].items()}
        data_links = [link for link in rows[matinee]['p']['variablelinks'] if link['linkdesc'] == 'Data']
        assert len(data_links) == 1 and len(data_links[0]['linkedvariables']) == 1
        data = data_links[0]['linkedvariables'][0]
        instance = instances['world.sequence.instance.valtan.source-preview.' + name]
        template = templates[instance['templateId']]
        primary_binding = next(b for b in instance['bindings'] if b['slotId'] == 'actor')
        resource = resources[primary_binding['targetId']]
        owner_scale = original.np.array(resource['scale'])
        assert instance['playbackSpeed'] == 1
        additions = []
        for group in rows[data]['p']['interpgroups']:
            tracks = original.active_tracks(rows, group)
            toggle = [t for t in tracks if t['cls'] == 'interptracktoggle']
            if not toggle:
                continue
            for actor in original.group_actor(rows, group, matinee):
                properties = rows[actor]['p']; component = properties.get('particlesystemcomponent')
                if not component:
                    continue
                component_properties = rows[component]['p']
                system = source['imports'].get(str(component_properties.get('template', 0)), '')
                asset = 'effect.valtan.source.' + system.casefold()
                context = dict(scene=scene, matinee=matinee, group=group, actor=actor, sourceParticleSystem=system)
                fixed_eye = name == 'entrance' and actor in (108, 109) and \
                    'effect.valtan.cinematic.entrance.eyes.parameters' in installed
                if fixed_eye:
                    asset = 'effect.valtan.cinematic.entrance.eyes.parameters'
                # The separate original actor64 installer owns the exact body
                # socket basis. Preserve and acknowledge its already-verified lane.
                if name == 'entrance' and actor in (72, 112):
                    existing = [e for candidate in document['templates'] for e in candidate.get('effectTracks', [])
                                if e['effectTrackId'].startswith(f'fx.valtan.source.{name}.{actor}.')]
                    if existing:
                        assert all(e['resourceId'] == asset and e['bone'] == properties['basebonename']
                                   and e['slotId'].startswith('actor64.body.') for e in existing)
                        if source_is_infinite(asset):
                            for e in existing:
                                e['loopEffectToDuration'] = True
                        receipt['restored'] += [dict(**context, effectTrackId=e['effectTrackId'],
                            startMs=e['startMs'], endMs=e['startMs']+e['durationMs'],
                            owner='ORIGINAL_ACTOR64_INSTALLER', sourcePackageSha256=source['sha256']) for e in existing]
                        continue

                def curve_lane(variant, ordinal, start, end, loop):
                    projected = json.loads(installed[variant].read_bytes())
                    source_track = projected['elements'][0]['sourceTransformTrack']
                    assert source_track['sourceTimeOriginSeconds'] == start/1000
                    point = original.BASIS @ original.np.array(source_track['previewOriginUE3Cm']) * .01
                    if name == 'phase2':
                        source_origin = original.world_pose(rows, 135, 74, 0, matinee, data)[0]
                        point += original.np.array(instance['position']) - source_origin
                    local = (point - position_at(template, instance, start)) / owner_scale
                    # The original source node owns DrawScale/rotation once.
                    component_scale = component_properties.get('scale', 1.)
                    scale3 = original.vec(component_properties.get('scale3d'), (1., 1., 1.))[[0, 2, 1]]
                    identity = f'fx.valtan.source.{name}.{actor}.{ordinal}'
                    additions.append(dict(effectTrackId=identity, slotId='actor', resourceKind='V1_EFFECT',
                        resourceId=variant, timing='TIME', startMs=start, durationMs=end-start,
                        followObject=False, inheritObjectRotation=False, loopEffectToDuration=loop,
                        positionOffset=local.tolist(), rotationDegrees=[0, 0, 0],
                        scale=(scale3*component_scale/owner_scale).tolist()))
                    receipt['restored'].append(dict(**context, effectTrackId=identity, startMs=start,
                        endMs=end, loop=loop, exactSourceTransformAsset=variant,
                        sourcePackageSha256=source['sha256']))

                final_variant = 'effect.valtan.cinematic.finale.actor35.parameters'
                if name == 'finale' and actor == 35 and final_variant in installed:
                    for ordinal, start, end, loop in intervals(toggle[0]['p']['toggletrack'], template['durationMs']):
                        curve_lane(final_variant, ordinal, start, end, loop)
                    continue
                reason = ('SOURCE_SYSTEM_NOT_INSTALLED' if asset not in installed else
                          'SOURCE_ATTACHED_EMITTER' if properties.get('base') else
                          'SOURCE_PARAMETER_OVERRIDE' if component_properties.get('instanceparameters') else
                          'SOURCE_PARAMETER_TRACK' if any('particleparam' in t['cls'] for t in tracks) else '')
                if (reason == 'SOURCE_ATTACHED_EMITTER' and properties['base'] in (65, 68, 25)
                        and (fixed_eye or (not component_properties.get('instanceparameters')
                        and not any('particleparam' in t['cls'] for t in tracks)))):
                    # Preserve the actual source bone. A single Move key overrides
                    # the serialized relative transform; without a Move track the
                    # source relative transform is the attachment contract.
                    move = [t['p'] for t in tracks if t['cls'] == 'interptrackmove']
                    if move and (len(move) != 1 or len(move[0]['postrack']['points']) != 1
                                 or len(move[0]['eulertrack']['points']) != 1
                                 or move[0].get('moveframe') == 'imf_relativetoinitial'):
                        receipt['deferred'].append(dict(**context, reason='SOURCE_ANIMATED_BONE_OFFSET')); continue
                    animation_name = {'entrance': 'valtan.cinematic.entrance', 'trash': 'valtan.cinematic.trash',
                                      'finale': 'valtan.cinematic.finale', 'roar': 'mesh_att_battle_12_05'}[name]
                    model = original.wm.read_wmodel(ROOT/'Client/Bin/Resources'/resource['animationSetAssetId'],
                        include_geometry=False, animation_names=[animation_name])
                    clip = next(a for a in model.animations if a.name == animation_name)
                    bone_index = next(i for i, bone in enumerate(model.skeleton_bones) if bone.name == properties['basebonename'])
                    measured = []
                    for at in (0., .4, 1., 3.):
                        pose = original.pose_sample(model, clip, at)
                        combined = original.wm.combined_transforms(model.skeleton_bones,
                            [original.wm.affine_matrix(scale, quaternion, translation) for translation, quaternion, scale in pose])
                        basis = original.np.array(combined[bone_index]).reshape(4, 4)[:3, :3] * resource['modelPreScale']
                        measured.append(original.np.linalg.norm(basis, axis=1))
                    bone_basis = 1. if name == 'finale' else .01
                    assert all(original.np.allclose(value, [bone_basis]*3, atol=1e-6) for value in measured), (name, actor, measured)
                    location = original.vec(move[0]['postrack']['points'][0]['outval'] if move else properties.get('relativelocation'))
                    if move:
                        source_rotation = original.rotation(original.vec(move[0]['eulertrack']['points'][0]['outval']))
                    else:
                        source_rotation = original.actor_rotation(dict(rotation=properties.get('relativerotation', {})))
                    local = original.BASIS @ location * .01 / (bone_basis * owner_scale)
                    rotation = original.BASIS @ source_rotation @ original.BASIS.T
                    yaw, pitch, roll = original.Rotation.from_matrix(rotation).as_euler('YXZ', degrees=True)
                    component_scale = component_properties.get('scale', 1.)
                    scale3 = original.vec(component_properties.get('scale3d'), (1., 1., 1.))
                    scale = scale3 * component_scale * properties.get('drawscale', 1.) / (bone_basis * owner_scale)
                    for ordinal, start, end, loop in intervals(toggle[0]['p']['toggletrack'], template['durationMs']):
                        identity = f'fx.valtan.source.{name}.{actor}.{ordinal}'
                        additions.append(dict(effectTrackId=identity, slotId='actor', resourceKind='V1_EFFECT',
                            resourceId=asset, timing='TIME', startMs=start, durationMs=end-start,
                            followObject=True, inheritObjectRotation=True, loopEffectToDuration=loop,
                            bone=properties['basebonename'], positionOffset=local.tolist(),
                            rotationDegrees=[pitch, yaw, roll], scale=scale.tolist()))
                        receipt['restored'].append(dict(**context, effectTrackId=identity, startMs=start,
                            endMs=end, loop=loop, bone=properties['basebonename'], installedBoneBasisNorm=measured[0].tolist(),
                            sourcePackageSha256=source['sha256']))
                    continue
                if reason:
                    receipt['deferred'].append(dict(**context, reason=reason)); continue
                assert len(toggle) == 1
                for ordinal, start, end, loop in intervals(toggle[0]['p']['toggletrack'], template['durationMs']):
                    samples = [original.world_pose(rows, group, actor, ms / 1000, matinee, data)
                               for ms in (start, (start+end)/2, end)]
                    point, rotation = samples[0]
                    if any(not original.np.allclose(p, point, atol=1e-6) or
                           not original.np.allclose(r, rotation, atol=1e-6) for p, r in samples[1:]):
                        variant = f'effect.valtan.cinematic.{name}.actor{actor}.at{start}'
                        if variant in installed:
                            curve_lane(variant, ordinal, start, end, loop)
                            continue
                        receipt['deferred'].append(dict(**context, startMs=start, reason='SOURCE_MOVING_EMITTER')); continue
                    if name == 'phase2':
                        # Source Event_02 is rebased as one actor/camera/FX assembly
                        # onto the already-authoritative arena landing anchor.
                        source_origin = original.world_pose(rows, 135, 74, 0, matinee, data)[0]
                        point = point + original.np.array(instance['position']) - source_origin
                    # Rotation is excluded from the owner pivot. V1 still owns the
                    # resource scale, so divide once to preserve original FX metres.
                    local = (point - position_at(template, instance, start)) / owner_scale
                    yaw, pitch, roll = original.Rotation.from_matrix(rotation).as_euler('YXZ', degrees=True)
                    component_scale = component_properties.get('scale', 1.)
                    scale3 = original.vec(component_properties.get('scale3d'), (1., 1., 1.))
                    scale = scale3 * component_scale * properties.get('drawscale', 1.) / owner_scale
                    identity = f'fx.valtan.source.{name}.{actor}.{ordinal}'
                    addition = dict(effectTrackId=identity, slotId='actor', resourceKind='V1_EFFECT',
                        resourceId=asset, timing='TIME', startMs=start, durationMs=end-start,
                        followObject=False, inheritObjectRotation=False, loopEffectToDuration=loop,
                        positionOffset=local.tolist(), rotationDegrees=[pitch, yaw, roll], scale=scale.tolist())
                    additions.append(addition)
                    receipt['restored'].append(dict(**context, effectTrackId=identity, startMs=start,
                        endMs=end, loop=loop, sourceWorldPosition=point.tolist(), sourcePackageSha256=source['sha256']))
        current = template.setdefault('effectTracks', [])
        indexed = {e['effectTrackId']: e for e in current}
        for addition in additions:
            # ETTA_Trigger starts the original system once. A native infinite
            # emitter still runs until the source OFF/end; the existing runtime
            # uses this flag to bound EmitterLoops=0 without repeating finite FX.
            if source_is_infinite(addition['resourceId']):
                addition['loopEffectToDuration'] = True
                previous = indexed.get(addition['effectTrackId'])
                if previous and dict(previous, loopEffectToDuration=True) == addition:
                    previous['loopEffectToDuration'] = True
            assert addition['effectTrackId'] not in indexed or indexed[addition['effectTrackId']] == addition, 'Preserve edited cinematic effect lane'
            if addition['effectTrackId'] not in indexed:
                current.append(addition)
        for record in receipt['restored']:
            lane = next((e for e in current if e['effectTrackId'] == record['effectTrackId']), None)
            if lane:
                record['loop'] = lane['loopEffectToDuration']
    for template in document['templates']:
        if '.valtan.source-preview.' not in template['sequenceId']:
            continue
        rejected = {e['effectTrackId']: e for e in template.get('effectTracks', [])
                    if e['resourceId'] in ADMISSION_DEFERRED}
        if not rejected:
            continue
        template['effectTracks'] = [e for e in template['effectTracks']
                                   if e['effectTrackId'] not in rejected]
        retained = []
        for record in receipt['restored']:
            lane = rejected.get(record['effectTrackId'])
            if lane:
                receipt['deferred'].append(dict(record,
                    reason=ADMISSION_DEFERRED[lane['resourceId']],
                    sourceAssetId=lane['resourceId']))
            else:
                retained.append(record)
        receipt['restored'] = retained
    # The original finale actor explicitly names MN_RPBF_02. Reuse the
    # existing product body + weapon assembly; the old preview bound every
    # Valtan actor, including this ghost, to MN_RPBF_01.
    finale_source = json.loads((source_root/'LV_LUT_HEARTRB_ED_SCENE04A.json').read_bytes())
    assert finale_source['rows']['25']['p']['lookinfokey'] == 'EFDLChar_MN_RPBF_02.MN_RPBF_02'
    ghost_id = 'world.object.valtan.source-preview.ghost'
    ghost = next(r for r in document['objectResources'] if r['objectId'] == ghost_id)
    assert ghost.get('presentationBossArchetypeId', 'BOSS_VALTAN_GHOST') == 'BOSS_VALTAN_GHOST'
    ghost['presentationBossArchetypeId'] = 'BOSS_VALTAN_GHOST'
    finale = next(i for i in document['instances'] if i['instanceId'].endswith('.finale'))
    binding = next(b for b in finale['bindings'] if b['slotId'] == 'actor')
    assert binding['targetId'] in (ghost_id, 'world.object.valtan.source-preview.body')
    binding['targetId'] = ghost_id
    receipt['finalePresentation'] = dict(sourceActor=25, sourceModel='MN_RPBF_02',
        targetResource=ghost_id, presentationBossArchetypeId='BOSS_VALTAN_GHOST')
    if document != json.loads(before):
        document['revision'] += 1
    return path, before, document, receipt


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--source-root', type=Path, required=True)
    parser.add_argument('--evidence-root', type=Path, required=True)
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    path, before, document, receipt = prepare(args.source_root)
    args.evidence_root.mkdir(parents=True, exist_ok=True)
    (args.evidence_root/'worldsequences.before.json').write_bytes(before)
    after = writer.encode_json(document)
    (args.evidence_root/'worldsequences.candidate.json').write_bytes(after)
    (args.evidence_root/'cinematic_fx_receipt.json').write_bytes(writer.encode_json(receipt))
    if args.install:
        writer.commit_writes([(path, before, after)])
    print(json.dumps(dict(restored=len(receipt['restored']), deferred=len(receipt['deferred']), installed=args.install)))


if __name__ == '__main__':
    main()

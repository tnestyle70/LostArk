"""Restore the two original portal-rush GroundEffect notifies through LocalDecal.

The source action, SkillDecal/SkillEffect tables and GroundEffect payload own
timing, area, material and color. The existing LocalDecal carrier owns drawing;
this adapter adds neither gameplay authority nor a product presentation cue.
"""
from pathlib import Path
import argparse
import base64
import copy
import hashlib
import json
import sqlite3
import struct

import build_kouku_showtime_warning_groups as warning
import build_valtan_full_restore as restore

ROOT = restore.ROOT
ASSET = 'effect.valtan.action.420624.stage003.full.restore'
MATERIAL = 'fx_m_mi_o_00.fx_mi.fx_o_de_behitcircle_02_01_tr'


def append_verified_fan_elements(document, elements):
    """Append owned source warnings, preserving exact existing rows on replay."""
    owned = {element['id']: element for element in elements}
    installed = {element['id']: element for element in document['elements'] if element['id'] in owned}
    assert not installed or installed == owned, 'Modified or incomplete owned fan warnings must be preserved'
    if not installed:
        document['elements'].extend(elements)
    return document


def build(evidence, native_patch, install=False, fan=False, source_action=None,
          skill_decal_db=None, skill_effect_db=None):
    stage_index, decal_id, program_id = (7, 2001, 2615) if fan else (3, 2002, 2614)
    asset = f'effect.valtan.action.420624.stage{stage_index:03d}.full.restore'
    material_name = 'fx_m_mi_o_00.fx_mi.fx_o_de_behitmonfan_02_01_tr' if fan else MATERIAL
    archetype = 'GR_Mon_Fan_behit_02' if fan else 'GR_Mon_Circle_behit_02'
    expected_skills = (42062402, 42062404, 42062406, 42062408) if fan else (42065501, 42065502)
    action_path = source_action or ROOT / 'out/ValtanPriorityRestore20260915/MN_RPBF_00.all.action-effects.json'
    actions = warning.source.read(action_path)
    action = next(row for row in actions['actions'] if row['actionId'] == 420624)
    stage = next(row for row in action['stages'] if row['stageIndex'] == stage_index)
    notifies = [row for row in stage['notifies'] if row['sourceType'] == 'PlayDecalEffect']
    assert len(notifies) == len(expected_skills)
    native = warning.source.read(native_patch)
    programs = [row for row in native['programs'] if row['program'] == program_id]
    assert len(programs) == 1 and programs[0]['material']['sourceMaterialPath'] == material_name
    patch = programs[0]['material']
    assert patch['sourceProfile']['runtimeShaderProfileId'] == f'effect.ue3.kouku-{program_id}-native.v1'

    decal_path = skill_decal_db or ROOT / 'out/KoukuShowtimeWarnings20260912/EFTable_SkillDecal.db'
    skill_path = skill_effect_db or warning.source.SOURCE / 'WorldObjectExtraction-20260907/EFTable_SkillEffect.db'
    decal_db = sqlite3.connect('file:' + decal_path.as_posix() + '?mode=ro', uri=True)
    skill_db = sqlite3.connect('file:' + skill_path.as_posix() + '?mode=ro', uri=True)
    decal_db.row_factory = skill_db.row_factory = sqlite3.Row
    decal = dict(decal_db.execute('SELECT * FROM SkillDecal WHERE PrimaryKey=?', (decal_id,)).fetchone())
    assert decal['DecalArchetype'] == archetype and decal['DecalKey'] == ('FanShape' if fan else 'Circle')
    name = '10_EF_PARTICLE_SOUND_DATA_GROUND_EFFECT_' + archetype + '.loa'
    source_path, raw = warning.read_archive_entries('data3.lpk', [name])[name]
    material = next(row for row in warning.scan_length_prefixed_strings(raw, 0, len(raw))
                    if row['value'].startswith("MaterialInstanceConstant'"))
    assert material['value'].split("'")[1].lower() == material_name
    at = material['sourceOffset'] + 4 + len(material['value']) + 1
    projection = struct.unpack_from('<4f', raw, at)
    assert projection == (100, 100, -300, 300)
    color = list(struct.unpack_from('<4f', raw, at + 20))

    template = warning.source.read(ROOT / 'Data/Effects/Authored/effect.kouku.gate3.showtime.circle.warning.effect.json')
    template = template['elements'][0]
    elements, source_rows = [], []
    for notify, expected_skill in zip(notifies, expected_skills):
        payload = base64.b64decode(notify['serializedPayload']['data'], validate=True)
        assert hashlib.sha256(payload).hexdigest() == notify['serializedPayload']['sha256']
        assert len(payload) == 187 and payload.startswith(b'CEFActionNotify_PlayDecalEffect\0')
        assert struct.unpack_from('<I', payload, 44)[0] == 1
        assert struct.unpack_from('<I', payload, 91)[0] == decal_id
        assert struct.unpack_from('<I', payload, 151)[0] == expected_skill
        area = dict(skill_db.execute('SELECT * FROM SkillEffect WHERE PrimaryKey=?', (expected_skill,)).fetchone())
        assert area['AreaType'] == (3 if fan else 1) and area['AreaRemoveRange'] == 0
        assert not any(area[key] for key in ('AreaOffsetY', 'AreaOffsetZ', 'AreaOffsetAngleRandom'))
        assert (area['AreaRange'] == 1000 and area['AreaAngle'] == 80) if fan else area['AreaOffsetAngle'] == 0
        start, lifetime = notify['localTimeSeconds'], notify['durationSeconds']
        fade_in, fade_out = struct.unpack_from('<f', payload, 163)[0], struct.unpack_from('<f', payload, 175)[0]
        assert 0 < fade_in <= lifetime - fade_out < lifetime
        diameter = area['AreaRange'] * .02
        identity = notify['notifyId'] + '|groundeffect.' + str(expected_skill)
        element = copy.deepcopy(template)
        element.update(id='valtan.groundeffect.' + str(expected_skill),
                       displayName=('Source fan warning ' if fan else 'Portal rush source circle ') + str(expected_skill),
                       groupId=asset, sourceNode=identity, material=copy.deepcopy(patch), resources=[])
        detail = element['detail']
        # The original fan mask is centered on UV +V. The existing projector
        # maps +V to local -Z; this per-fan adapter maps that to source +X
        # before the ordinary snapshot source basis. UE positive yaw keeps
        # its sign in the existing client yaw carrier.
        fan_basis_yaw = -90 if fan else 0
        detail['transform'].update(position=[area['AreaOffsetX'] * .01, 0, 0],
                                   rotationDegrees=[0, fan_basis_yaw + area['AreaOffsetAngle'], 0], scale=[1, 1, 1])
        detail['color']['multiply'] = color
        detail['timing'].update(startDelaySeconds=start, lifeTimeSeconds=lifetime, afterImageSeconds=0)
        detail['decal'].update(size=[diameter] * 2, depth=(projection[3] - projection[2]) * .01)
        detail['particle'].update(lifeTimeSeconds=[lifetime] * 2, startSize=[diameter] * 2, endSize=[diameter] * 2)
        element['actionCueAttachment'].update(enabled=True, follow=False, snapshotRootSourceBasisYawDegrees=-90)
        recipe = element['sourceRecipe']
        recipe.update(emitterDelaySeconds=0, emitterDurationSeconds=lifetime, emitterLoopCount=1)
        for module in recipe['modules']:
            module.update(stableId=identity + '.' + module['className'], objectPath=identity + '.' + module['className'])
            for literal in module['literals']:
                if literal['propertyPath'] == 'emitterduration':
                    literal['value'] = lifetime
            for distribution in module['distributions']:
                kind = distribution['propertyPath']
                if kind in ('lifetime', 'startsize'):
                    distribution.update(warning.constant_distribution(kind, [lifetime] if kind == 'lifetime' else [diameter * 100] * 3))
        track = element['sourceTransformTrack']
        track.update(sourceOccurrenceId=identity, sourceTimeOriginSeconds=-start)
        track['nodes'][0]['sourceObjectPath'] = identity
        track.pop('materialParameterTracks', None)
        def alpha_key(time, value):
            return dict(timeSeconds=time, value=[value] * 3, arriveTangent=[0] * 3, leaveTangent=[0] * 3, interpolation='linear')
        track['alphaScaleKeys'] = [alpha_key(0, 0), alpha_key(fade_in, 1), alpha_key(lifetime - fade_out, 1), alpha_key(lifetime, 0)]
        for scalar in element['material']['sourceProfile']['scalars']:
            if scalar['name'] == 'decal_drawscale':
                scalar['value'] = diameter * 100 / projection[0]
            elif fan and scalar['name'] == 'angle':
                scalar['value'] = area['AreaAngle'] / 360
        elements.append(element)
        source_rows.append(dict(notifyId=notify['notifyId'], sourceSkillDecalId=decal_id,
            sourceSkillEffectId=expected_skill, sourceArea={key: value for key, value in area.items() if key.startswith('Area')},
            startSeconds=start, lifetimeSeconds=lifetime, fadeInSeconds=fade_in, fadeOutSeconds=fade_out,
            diameterM=diameter, sourceOffsetM=area['AreaOffsetX'] * .01,
            nativeProgram=program_id, sourceMaterial=material_name, radialFillCurveAdded=False,
            fanNativeUvForwardToSourceXYawDegrees=fan_basis_yaw))
    decal_db.close()
    skill_db.close()
    document = dict(schema='lostark.effect-authoring', version=13, effectAssetId=asset,
        displayName=f'Valtan 420624 / StageName [{stage_index}] full restore',
        particleSystem=dict(uniformScaleMultiplier=1, yawOffsetDegrees=0, directionYawDegrees=0, initialSpeedMultiplier=1),
        modelCues=[], elements=elements)
    if fan:
        existing_path = ROOT / 'Data/Effects/Authored' / (asset + '.effect.json')
        existing_bytes = existing_path.read_bytes()
        existing = json.loads(existing_bytes)
        document = append_verified_fan_elements(existing, elements)
    receipt = [dict(effectAssetId=asset, sourceActionId=420624, sourceStageIndex=stage_index,
        originalAnimationClips=copy.deepcopy(stage['animationClips']), elementCount=len(document['elements']), restored=source_rows,
        sourceFailures=[], completeSourceDirectDecalNotifies=True, visualStatus='USER_PENDING')]
    receipt = restore.animation_receipt_with_loops(receipt, actions)
    warning.source.write(evidence / 'candidate' / (asset + '.effect.json'), document)
    warning.source.write(evidence / 'stage_projection.json', receipt)
    warning.source.write(evidence / 'ground_effect_provenance.json', dict(sourceActionPath=action_path.as_posix(),
        sourceGroundEffectPath=source_path, sourceGroundEffectSha256=hashlib.sha256(raw).hexdigest(),
        sourceSkillDecalRow=decal, projectionCm=projection, activeColor=color,
        adapter='Existing one-burst LocalDecal carrier; original GroundEffect is not a Cascade emitter',
        rows=source_rows, productCueWrites=False))
    if install:
        if fan:
            path = ROOT / 'Data/Effects/Authored' / (asset + '.effect.json')
            before = path.read_bytes()
            assert before == existing_bytes, 'Concurrent stage007 edits must be preserved'
            original = warning.source.read(path)
            owned_ids = {element['id'] for element in elements}
            assert [element for element in document['elements'] if element['id'] not in owned_ids] == [
                element for element in original['elements'] if element['id'] not in owned_ids], 'Existing particle changes must be preserved'
            after = (json.dumps(document, ensure_ascii=False, indent=2) + '\n').encode('utf8')
            restore.commit_writes([(path, before, after)])
        else:
            restore.install_documents({asset: document}, evidence, receipt)
    return document


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, required=True)
    parser.add_argument('--native-material-patch', type=Path, required=True)
    parser.add_argument('--install', action='store_true')
    parser.add_argument('--fan', action='store_true', help='Append the four source fan warnings to stage007')
    parser.add_argument('--source-action', type=Path)
    parser.add_argument('--skill-decal-db', type=Path)
    parser.add_argument('--skill-effect-db', type=Path)
    args = parser.parse_args()
    result = build(args.evidence_root, args.native_material_patch, args.install, args.fan,
                   args.source_action, args.skill_decal_db, args.skill_effect_db)
    print(json.dumps(dict(effectAssetId=result['effectAssetId'], elements=len(result['elements']), installed=args.install)))

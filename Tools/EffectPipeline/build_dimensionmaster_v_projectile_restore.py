"""Restore the original V projectile CreateFX calls missing from its Action FX.

The exact Action -> SkillEffect -> fixed-area Projectile link is checked before
appending source emitter streams to the existing V document. Original direct
Action elements are preserved. Native extraction uses the shared material
carrier; it does not create another renderer or alter the F skill.
"""
from __future__ import annotations

import argparse
import collections
import copy
import hashlib
import math
import sqlite3
import struct
from pathlib import Path

import build_kouku_all_source_effects as library
import build_kouku_gate1_full_restore as source
from build_kouku_effect_organization import (ArchiveReader, effect_payload_id,
    extract_action_document, scan_length_prefixed_strings)

ROOT = source.ROOT
ASSET = 'effect.dimensionmaster.skill.2050520.full.restore'
PREFIX = 'dimensionmaster.2050520.projectile20505200.'
SYSTEMS = {
    205052001: ('fx_pc_swp_02.par_r_swp_timewave_00_01', '차원술사 V 파동'),
    205052004: ('fx_pc_swp_02.par_r_swp_timewave_00_04', '차원술사 V 방출'),
    205052005: ('fx_post.fx_par.par_c_zoomblur_02', '차원술사 V 파동 화면 확산'),
    205052006: ('fx_post.fx_par.par_j_rgbnoise_01', '차원술사 V 파동 화면 왜곡'),
}


def targets():
    return {i: dict(asset=ASSET + '.projectile.' + str(i), name=name, system=system,
        count=None, countIncludesEmpty=True, sourceActions=[])
        for i, (system, name) in SYSTEMS.items()}


def decode(evidence):
    archive_root = Path('C:/ProgramData/Smilegate/Games/LOSTARK/EFGame')
    receipts = []
    paths = {}
    for name, domain, filename in [('data3.lpk', 'Action', 'DimensionMaster.loa'),
                                   ('data1.lpk', 'Projectile', '20505200.loa')]:
        archive = ArchiveReader(archive_root / name, evidence / 'source')
        try:
            paths[domain] = archive.acquire(domain, filename)
            receipts.extend(archive.receipts)
        finally:
            archive.close()
        assert paths[domain] is not None
    source.write(evidence / 'source_acquisition.json', receipts)
    action = next(a for a in extract_action_document(paths['Action'], 'DIMENSIONMASTER')['actions']
                  if a['actionId'] == 2050520)
    assert len(action['stages']) == 1 and action['stages'][0]['stageName'] == 'Main'
    calls = [(n, effect_payload_id(n['serializedPayload'])[0]) for n in action['stages'][0]['notifies']
             if n['sourceType'] == 'Effect']
    assert len(calls) == 1 and calls[0][1] == 20505200
    notify = calls[0][0]
    assert math.isclose(notify['localTimeSeconds'], 1.55, abs_tol=1e-6)
    source.write(evidence / 'source_action_2050520.json', action)
    database = library.source.SOURCE / 'WorldObjectExtraction-20260907/EFTable_SkillEffect.db'
    with sqlite3.connect(database.as_uri() + '?mode=ro', uri=True) as conn:
        conn.row_factory = sqlite3.Row
        skill_rows = [dict(r) for r in conn.execute(
            'SELECT * FROM SkillEffect WHERE PrimaryKey BETWEEN 20505200 AND 20505207')]
    spawn = [r for r in skill_rows if r['PrimaryKey'] == 20505200]
    assert len(spawn) == 1
    spawn = spawn[0]
    assert (spawn['SecondaryKey'], spawn['Key'], spawn['ValueA']) == (1, 12, 20505200)
    assert [spawn[k] for k in ('AreaOffsetX', 'AreaOffsetY', 'AreaOffsetZ', 'AreaOffsetAngle')] == [25, 0, 0, 0]
    assert all(r['Key'] == 2 for r in skill_rows if r['PrimaryKey'] != 20505200)
    source.write(evidence / 'source_skill_effect_2050520.json', skill_rows)
    raw = paths['Projectile'].read_bytes()
    strings = scan_length_prefixed_strings(raw, 0, len(raw))
    assert strings[0]['value'] == 'CEFSequenceSummonsProjectileFixArea'
    assert struct.unpack_from('<I', raw, 1034)[0] == 13
    assert math.isclose(struct.unpack_from('<f', raw, 1014)[0], 3.1, abs_tol=1e-6)
    actions = [s for s in strings if s['value'].startswith('CEFSequenceSummonsAction')]
    assert collections.Counter(s['value'] for s in actions) == {
        'CEFSequenceSummonsActionTimer': 7, 'CEFSequenceSummonsActionSkillEffect': 7,
        'CEFSequenceSummonsActionCreateFX': 4, 'CEFSequenceSummonsActionCameraShake': 1,
        'CEFSequenceSummonsActionAkEvent': 1}
    # Seven fixed-size timer children precede the six immediate root actions.
    # Check their typed SkillEffect IDs and trailing timer data, not string order
    # alone, so a changed serialized topology cannot silently move the FX clock.
    for i in range(7):
        timer = 1038 + 158 * i
        child = timer + 38
        assert any(s['sourceOffset'] == timer and s['value'] == 'CEFSequenceSummonsActionTimer' for s in actions)
        assert struct.unpack_from('<I', raw, child + 72)[0] == 20505201 + i
        assert struct.unpack_from('<5i', raw, timer + 122) == (1, 1, 1, 1, 0)
        assert math.isclose(struct.unpack_from('<f', raw, timer + 142)[0], .3 * (i + 1), abs_tol=1e-6)
    expected_offsets = {205052004: 2303, 205052001: 2708, 205052005: 3316, 205052006: 3794}
    particles = {}
    for identity, (system, _) in SYSTEMS.items():
        token = next(s for s in strings if s['value'].lower() == "particlesystem'" + system + "'")
        at = token['sourceOffset']
        assert at == expected_offsets[identity]
        assert any(s['sourceOffset'] == at - 159 and s['value'] == 'CEFSequenceSummonsActionCreateFX' for s in actions)
        assert struct.unpack_from('<5i', raw, at - 114) == (1, 1, 1, 1, 0)
        end = at + 4 + len(token['value']) + 1
        position = list(struct.unpack_from('<3f', raw, end + 76))
        rotation = list(struct.unpack_from('<3i', raw, end + 100))
        scale = list(struct.unpack_from('<3f', raw, end + 136))
        assert rotation == [0, 0, 0] and all(math.isfinite(v) and v > 0 for v in scale)
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
        particles[system] = dict(sourceParticleByteOffset=at, sourcePositionCm=position,
            sourceScale=scale, sourceRotator=rotation, parameterOverrides=parameters)
    result = dict(skillId=2050520, inputSlot='V', runtimeClip='pc_sp_m_00_sk_sk_timewave',
        sourceNotifyId=notify['notifyId'], notifySeconds=notify['localTimeSeconds'],
        projectileId=20505200, sourceLifetimeSeconds=3.1, sourceSpawnOffsetCm=[25, 0, 0],
        sourceParticleSystems=particles, sourceSha256=hashlib.sha256(raw).hexdigest(),
        immediateCreateFxCount=4, damageOnlyTimerCount=7,
        excludedNonParticleCalls=['original camera shake', 'original AkEvent sound'])
    source.write(evidence / 'projectile_source_contract.json', result)
    return result


def project(evidence, native_patch, install):
    contract = decode(evidence)
    index, notifies, occurrences, records = library.acquire(evidence, targets())
    original_path = ROOT / 'Data/Effects/Authored' / (ASSET + '.effect.json')
    original = source.read(original_path)
    for notify in notifies:
        system = notify['assetReferences'][0]['objectPath'].lower()
        parameters = contract['sourceParticleSystems'][system]['parameterOverrides']
        notify['cue']['parameterOverrides'] = copy.deepcopy(parameters)
        notify['serializedPayload']['parameterOverrides'] = copy.deepcopy(parameters)
    projection_patch = native_patch
    if native_patch:
        patch = source.read(native_patch)
        for occurrence in occurrences:
            if occurrence['rendererShape'] != 'screenPost':
                continue
            material = next(e['material'] for e in original['elements'] if e['kind'] == 'screenPost'
                and e['material']['sourceMaterialPath'] == occurrence['sourceMaterial'])
            patch['programs'].append(dict(program=None, occurrences=[occurrence['elementId']], material=copy.deepcopy(material)))
        projection_patch = evidence / 'native/projectile_projection_material_patch.json'
        source.write(projection_patch, patch)
    source.project(evidence, index, notifies, occurrences, records, evidence / 'projected', projection_patch)
    direct = [e for e in original['elements'] if not e['id'].startswith(PREFIX)]
    assert len(direct) == 42, 'The original V Action document changed; inspect it before merging'
    document = copy.deepcopy(original)
    document['elements'] = copy.deepcopy(direct)
    native_contract = source.read(evidence / 'native/native_runtime_contract.json') if native_patch else None
    native_inputs = source.read(evidence / 'native/native_material_inputs.json')['materials'] if native_patch else []
    native_hlsl = (ROOT / 'Client/Bin/ShaderFiles/Shader_EffectDimensionMasterVNative.hlsli').read_text(encoding='utf-8-sig')
    reused_posts = []
    additions = []
    for identity, (system, _) in SYSTEMS.items():
        partial = source.read(evidence / 'projected' / f'effect.kouku.gate1.{identity}.full.restore.effect.json')
        data = contract['sourceParticleSystems'][system]
        for element in partial['elements']:
            source.project_parameters(index, element['sourceRecipe'], data)
            origin = element['sourcePresentation']['sourceObjectPath']
            element['id'] = PREFIX + hashlib.sha256(origin.encode()).hexdigest()[:20]
            element['displayName'] = 'V 파동 | ' + origin.rsplit('.', 1)[-1]
            element['groupId'] = 'dimensionmaster.2050520.projectile20505200'
            element['sourceNode'] = 'projectile-20505200/createfx-' + str(data['sourceParticleByteOffset']) + '|' + origin
            element['detail']['timing']['startDelaySeconds'] += contract['notifySeconds']
            element['actionCueAttachment'].update(enabled=True, follow=False, snapshotRootSourceBasisYawDegrees=-90)
            x, y, z = data['sourcePositionCm']
            element['detail']['transform'].update(position=[(x + 25) * .01, z * .01, -y * .01],
                rotationDegrees=[0, 0, 0], scale=[data['sourceScale'][i] for i in (0, 2, 1)])
            element['sourcePresentation'].update(sourceTimeSeconds=contract['notifySeconds'],
                sourceActionCueId=contract['sourceNotifyId'], sourceEventId='projectile-20505200/createfx-' + str(data['sourceParticleByteOffset']))
            if element['kind'] == 'screenPost':
                if native_contract:
                    program = next(p for p in native_contract['programs']
                        if p['sourceMaterial'] == element['material']['sourceMaterialPath'] and p['rendererShape'] == 'screenPost')
                    material_input = next(p for p in native_inputs if p['sourceMaterial'] == program['sourceMaterial'])
                    existing = next(e['material'] for e in direct
                        if e['kind'] == 'screenPost' and e['material']['sourceMaterialPath'] == program['sourceMaterial'])
                    # This existing full-screen carrier executes the same exact
                    # original PS and MaterialMap with identical effective inputs.
                    # Reuse V68/V76 instead of duplicating that runtime program.
                    assert program['sourcePS'] in native_hlsl and material_input['mapKey'] in native_hlsl
                    profile = existing['sourceProfile']
                    assert {p['name']: p['effective'] for p in program['parameters']} == {
                        p['name']: p['value'] for field in ('scalars', 'vectors') for p in profile[field]}
                    assert {p['parameterName']: p['value'] for p in program['staticSwitches']} == {
                        p['name']: p['value'] for p in profile['staticSwitches']}
                    assert not program['textures'] and not profile['textures']
                    assert existing['renderProfile'] == element['material']['renderProfile']
                    element['material'] = copy.deepcopy(existing)
                    reused_posts.append(dict(sourceMaterial=program['sourceMaterial'], sourcePS=program['sourcePS'],
                        materialMapKey=material_input['mapKey'], runtimeShaderProfileId=profile['runtimeShaderProfileId']))
                element['detail']['screenPost'].update(enabled=True,
                    profileId='screen.zoom-blur.reconstructed.v1' if 'zoomblur' in system else 'screen.film-noise.reconstructed.v1',
                    status='reconstructed_profile', intensity=1, secondaryIntensity=0, frequency=1,
                    tint=[1, 1, 1, 1], randomSeed=element['detail']['particle']['randomSeed'])
            additions.append(element)
    document['elements'].extend(additions)
    assert len({e['id'] for e in document['elements']}) == len(document['elements'])
    assert document['elements'][:42] == direct
    source.write(evidence / 'candidate' / original_path.name, document)
    if native_contract:
        active = copy.deepcopy(native_contract)
        active['programs'] = [p for p in active['programs'] if p['rendererShape'] != 'screenPost']
        source.write(evidence / 'native/active_native_runtime_contract.json', active)
        source.write(evidence / 'native/exact_reused_v_screen_posts.json', reused_posts)
    if install:
        assert native_patch is not None
        assert source.read(original_path) == original, 'V authoring changed while staging'
        source.write(original_path, document)
    source.write(evidence / 'installation.json', dict(installed=install, effectAssetId=ASSET,
        documentPath=original_path.relative_to(ROOT).as_posix(), previousDirectElements=42,
        addedProjectileElements=len(additions), totalElements=len(document['elements']),
        sourceParticleSystems=list(contract['sourceParticleSystems']),
        nativeMaterialPatch=str(native_patch), manualVisualValidation='USER_PENDING'))
    print('V direct preserved', len(direct), 'projectile added', len(additions), 'installed', install)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/DimensionMasterV20260912')
    parser.add_argument('--acquire-only', action='store_true')
    parser.add_argument('--native-material-patch', type=Path)
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    if args.acquire_only:
        decode(args.evidence_root)
        library.acquire(args.evidence_root, targets())
    else:
        project(args.evidence_root, args.native_material_patch, args.install)

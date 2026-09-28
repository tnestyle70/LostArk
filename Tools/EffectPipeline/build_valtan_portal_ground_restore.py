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
FOUR_ASSET = 'effect.valtan.action.420624.stage007.full.restore'
FOUR_SECTOR_IDS = tuple('valtan.groundeffect.' + str(skill) for skill in
                       (42062402, 42062404, 42062406, 42062408))


def split_four_candidates(document):
    """Derive independent warnings without changing the reviewed full source.

    V2 Groups only admit LEAF/GROUP. These native LocalDecals remain ordinary
    V1 documents; a V1 pattern cue can run beside the existing V2 impacts.
    """
    assert document['effectAssetId'] == FOUR_ASSET
    sectors = [copy.deepcopy(element) for element in document['elements']
               if element['id'] in FOUR_SECTOR_IDS]
    assert tuple(element['id'] for element in sectors) == FOUR_SECTOR_IDS
    body = copy.deepcopy(document)
    body['effectAssetId'] = 'effect.valtan.action.420624.stage007.body.full.restore'
    body['displayName'] = '2페이즈 4방향 공격 / Full Restore 본체 (Sector 제외)'
    body['elements'] = [element for element in body['elements'] if element['id'] not in FOUR_SECTOR_IDS]
    rows = []
    for element in sectors:
        assert element['material']['sourceProfile']['runtimeShaderProfileId'] == 'effect.ue3.kouku-2615-native.v1'
        rows.append(dict(elementId=element['id'],
            sourceStartSeconds=element['detail']['timing']['startDelaySeconds'],
            sourceYawDegrees=element['detail']['transform']['rotationDegrees'][1],
            fill=warning.animate_radial_fill(element)))
    sequence = {key: copy.deepcopy(value) for key, value in document.items()
                if key not in ('elements', 'runtimeExtensions')}
    sequence.update(effectAssetId='effect.valtan.action.420624.stage007.sectors.full.restore',
        displayName='2페이즈 4방향 공격 / Sector 4방향 (Inner 확장)', modelCues=[], elements=sectors)
    # All four share one material/shape/fade. Keep the existing native-UV to
    # source-X and source-X to target-forward adapters; callers own relative yaw.
    single = copy.deepcopy(sequence)
    single.update(effectAssetId='effect.valtan.sequence.four.sector',
        displayName='2페이즈 4방향 공격 / Sector 단일 (Inner 확장)', elements=[copy.deepcopy(sectors[0])])
    element = single['elements'][0]
    element['detail']['timing']['startDelaySeconds'] = 0
    element['sourceTransformTrack']['sourceTimeOriginSeconds'] = 0
    assert element['detail']['transform']['rotationDegrees'] == [0, -90, 0]
    assert element['actionCueAttachment']['snapshotRootSourceBasisYawDegrees'] == -90
    return (body, sequence, single), rows


def write_four_candidates(evidence):
    """Write reviewable candidates only; installation merges current saved data."""
    relative = Path('Data/Effects/Authored') / (FOUR_ASSET + '.effect.json')
    before = (ROOT / relative).read_bytes()
    document = json.loads(before)
    documents, rows = split_four_candidates(document)
    for candidate in documents:
        warning.source.write(evidence / 'candidate' / (candidate['effectAssetId'] + '.effect.json'), candidate)
    bindings_path = ROOT / 'Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json'
    binding_bytes = bindings_path.read_bytes()
    impacts = [binding for binding in json.loads(binding_bytes)['bindings']
               if binding['scope']['actionId'] == 'valtan.sequence.four.step-01']
    impacts.sort(key=lambda binding: binding['clock']['startMs'])
    assert len(impacts) == len(rows) and all(binding['resource'] ==
        dict(kind='GROUP', id='boss.valtan.impact') for binding in impacts)
    cues_path = ROOT / 'Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json'
    cues_bytes = cues_path.read_bytes()
    existing = [cue for cue in json.loads(cues_bytes)['cues']
                if cue['effectAssetId'] == 'effect.valtan.project-tuned.sequence.four']
    assert len(existing) == 1
    cues = []
    for index, (row, impact) in enumerate(zip(rows, impacts), 1):
        cue = copy.deepcopy(existing[0])
        cue.update(bindingId=f'cue.valtan.phase2.four.sector.{index:02d}',
            occurrenceId=f'cue.valtan.phase2.four.sector.{index:02d}.occurrence.01',
            effectAssetId=documents[2]['effectAssetId'], followPolicy='snapshot',
            sourceStartMs=round(row['sourceStartSeconds'] * 1000), sourceEndMs=None)
        cue['localTransform']['rotationDegrees'] = copy.deepcopy(impact['anchor']['localTransform']['rotation'])
        # Cancel the mandatory Valtan 1.5 world-footprint policy locally so the
        # new warning retains the full-source 10 m radius, without enlarging it.
        cue['localTransform']['scale'] = [1 / value for value in cue['scalePolicy']['worldScale']]
        cues.append(cue)
    warning.source.write(evidence / 'sector-cue-additions.json', dict(cues=cues))
    metadata = warning.source.read(ROOT / restore.ANIMATIONS)
    source_metadata = next(row for row in metadata['effects'] if row['effectAssetId'] == FOUR_ASSET)
    variants = [dict(copy.deepcopy(source_metadata), effectAssetId=doc['effectAssetId'], variantId=variant)
                for doc, variant in zip(documents[:2], ('body', 'sectors'))]
    warning.source.write(evidence / 'animation-metadata-additions.json', dict(effects=variants))
    receipt = dict(sourcePath=relative.as_posix(), sourceSha256=hashlib.sha256(before).hexdigest(),
        originalElementCount=len(document['elements']), bodyElementCount=len(documents[0]['elements']),
        derivedDocuments=[dict(effectAssetId=doc['effectAssetId'], elementCount=len(doc['elements'])) for doc in documents],
        nativeProgram=2615, nativeInnerRow=0, nativeInnerLane=2, rows=rows,
        v2BindingSha256=hashlib.sha256(binding_bytes).hexdigest(),
        v1CueSha256=hashlib.sha256(cues_bytes).hexdigest(), preservedV2Bindings=impacts,
        cuePolicy='Four independently editable V1 sector cues beside unchanged V2 impacts; current impact yaw is reused.',
        scalePolicy=copy.deepcopy(existing[0]['scalePolicy']),
        cueLocalScale=copy.deepcopy(cues[0]['localTransform']['scale']), effectiveWorldScale=[1, 1, 1],
        sourceAnimationMetadata=copy.deepcopy(source_metadata),
        sourceRadiusMeters=10, sourceAngleDegrees=80,
        canonicalFilesWritten=False, visualStatus='USER_PENDING')
    warning.source.write(evidence / 'four-sector-projection.json', receipt)
    assert (ROOT / relative).read_bytes() == before
    assert bindings_path.read_bytes() == binding_bytes and cues_path.read_bytes() == cues_bytes
    return receipt


def append_verified_fan_elements(document, elements):
    """Append owned source warnings, preserving exact existing rows on replay."""
    owned = {element['id']: element for element in elements}
    installed = {element['id']: element for element in document['elements'] if element['id'] in owned}
    assert not installed or installed == owned, 'Modified or incomplete owned fan warnings must be preserved'
    if not installed:
        document['elements'].extend(elements)
    return document


def write_tracking_axe_circle_candidate(evidence, source_root):
    """Reuse the source circle material for the eight measured large warnings.

    The source audit owns occurrence selection and SkillEffect table extraction.
    Recheck raw notify/ground bytes here; only the missing inner clock is authored.
    """
    audit = warning.source.read(source_root / 'source-and-installed.json')
    actions = warning.source.read(ROOT / audit['sourceActionPath'])
    action = next(row for row in actions['actions'] if row['actionId'] == 420610)
    selected = [row for row in audit['directDecals'] if row['skillEffectId'] in (42061011, 42061012)]
    assert len(selected) == 8
    stages = (6, 11, 14, 15, 33, 38, 41, 42)
    assert tuple(int(row['notifyId'].split('/')[1].split('-')[1]) for row in selected) == stages
    assert all((row['lifetimeSeconds'], row['fadeInSeconds'], row['fadeOutSeconds']) ==
               (selected[0]['lifetimeSeconds'], selected[0]['fadeInSeconds'], selected[0]['fadeOutSeconds'])
               for row in selected), 'One reusable warning requires identical source lifetime and fade'
    notifies = {notify['notifyId']: notify for stage in action['stages'] for notify in stage['notifies']}
    for row in selected:
        notify = notifies[row['notifyId']]
        raw = base64.b64decode(notify['serializedPayload']['data'], validate=True)
        assert len(raw) == 193 and raw.startswith(b'CEFActionNotify_PlayDecalEffect\0')
        assert hashlib.sha256(raw).hexdigest() == row['payloadSha256'] == notify['serializedPayload']['sha256']
        assert raw[72:82] == b'\x06\x00\x00\x00Decal\0' and raw[82:93] == b'\x07\x00\x00\x00notify\0'
        assert struct.unpack_from('<I', raw, 44)[0] == 1
        assert struct.unpack_from('<I', raw, 97)[0] == row['skillDecalId'] == 2002
        assert struct.unpack_from('<I', raw, 157)[0] == row['skillEffectId']
        assert struct.unpack_from('<f', raw, 169)[0] == row['fadeInSeconds']
        assert struct.unpack_from('<f', raw, 181)[0] == row['fadeOutSeconds']
        assert notify['localTimeSeconds'] == row['localStartSeconds'] == 3.5
        assert notify['durationSeconds'] == row['lifetimeSeconds']
        area = row['sourceArea']
        assert area['AreaType'] == 1 and area['AreaRange'] == 875 and area['AreaRemoveRange'] == 0
        assert not any(area[key] for key in ('AreaOffsetX', 'AreaOffsetY', 'AreaOffsetZ', 'AreaOffsetAngle', 'AreaOffsetAngleRandom'))
    name = '10_EF_PARTICLE_SOUND_DATA_GROUND_EFFECT_GR_Mon_Circle_behit_02.loa'
    raw = (source_root / name).read_bytes()
    ground = warning.source.read(source_root / (name + '.json'))
    assert hashlib.sha256(raw).hexdigest() == ground['sha256']
    material = next(row for row in warning.scan_length_prefixed_strings(raw, 0, len(raw))
                    if row['value'].startswith("MaterialInstanceConstant'"))
    assert material['value'].split("'")[1].lower() == MATERIAL
    at = material['sourceOffset'] + 4 + len(material['value']) + 1
    projection = struct.unpack_from('<4f', raw, at)
    color = list(struct.unpack_from('<4f', raw, at + 20))
    assert list(projection) == ground['projectionCm'] == [100, 100, -300, 300]
    assert color == ground['activeColor']
    template_path = ROOT / 'Data/Effects/Authored' / (ASSET + '.effect.json')
    template_bytes = template_path.read_bytes()
    document = json.loads(template_bytes)
    element = copy.deepcopy(document['elements'][0])
    assert element['material']['sourceMaterialPath'] == MATERIAL
    assert element['material']['sourceProfile']['runtimeShaderProfileId'] == 'effect.ue3.kouku-2614-native.v1'
    identity = selected[0]['notifyId'] + '|groundeffect.42061011'
    asset = 'effect.valtan.tracking-axe.large-circle.warning'
    element.update(id='valtan.groundeffect.42061011.independent', groupId=asset, sourceNode=identity,
                   displayName='Source tracking axe large circle / inner fill')
    row = selected[0]
    lifetime, fade_in, fade_out = row['lifetimeSeconds'], row['fadeInSeconds'], row['fadeOutSeconds']
    diameter = row['sourceArea']['AreaRange'] * .02
    detail = element['detail']
    detail['transform'].update(position=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1])
    detail['timing'].update(startDelaySeconds=0, lifeTimeSeconds=lifetime, afterImageSeconds=0)
    detail['color']['multiply'] = color
    detail['decal'].update(size=[diameter] * 2, depth=(projection[3] - projection[2]) * .01)
    detail['particle'].update(lifeTimeSeconds=[lifetime] * 2, startSize=[diameter] * 2, endSize=[diameter] * 2)
    for scalar in element['material']['sourceProfile']['scalars']:
        if scalar['name'] == 'decal_drawscale': scalar['value'] = diameter * 100 / projection[0]
    recipe = element['sourceRecipe']
    recipe.update(emitterDelaySeconds=0, emitterDurationSeconds=lifetime, emitterLoopCount=1)
    for module in recipe['modules']:
        module.update(stableId=identity + '.' + module['className'], objectPath=identity + '.' + module['className'])
        for literal in module['literals']:
            if literal['propertyPath'] == 'emitterduration': literal['value'] = lifetime
        for distribution in module['distributions']:
            kind = distribution['propertyPath']
            if kind in ('lifetime', 'startsize'):
                distribution.update(warning.constant_distribution(kind, [lifetime] if kind == 'lifetime' else [diameter * 100] * 3))
    track = element['sourceTransformTrack']
    track.update(sourceOccurrenceId=identity, sourceTimeOriginSeconds=0)
    track['nodes'][0]['sourceObjectPath'] = identity
    def key(time, value):
        return dict(timeSeconds=time, value=[value] * 3, arriveTangent=[0] * 3, leaveTangent=[0] * 3, interpolation='linear')
    track['alphaScaleKeys'] = [key(0, 0), key(fade_in, 1), key(lifetime - fade_out, 1), key(lifetime, 0)]
    fill = warning.animate_radial_fill(element)
    document.update(effectAssetId=asset, displayName='점프 후 플레이어 추적 도끼 / 큰 원형 장판 (Inner 확장)',
                    elements=[element], modelCues=[])
    warning.source.write(evidence / 'candidate' / (asset + '.effect.json'), document)
    receipt = dict(effectAssetId=asset, nativeProgram=2614, nativeInnerRow=0, nativeInnerLane=1,
        sourceTemplateSha256=hashlib.sha256(template_bytes).hexdigest(), sourceGroundEffect=ground,
        sourceOccurrences=selected, sourceRadiusMeters=diameter * .5, fill=fill,
        timingPolicy='Independent warning starts at zero; source stage notify remains at 3.5 seconds.',
        canonicalFilesWritten=False, productCueWrites=False, visualStatus='USER_PENDING')
    warning.source.write(evidence / 'tracking-circle-projection.json', receipt)
    assert template_path.read_bytes() == template_bytes
    return receipt


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
    parser.add_argument('--native-material-patch', type=Path)
    parser.add_argument('--split-four-candidates', action='store_true',
                        help='Derive body, growing-sector documents and cue additions into evidence only')
    parser.add_argument('--tracking-axe-circle-candidate', type=Path,
                        help='Source audit directory for the independent large-circle warning candidate')
    parser.add_argument('--install', action='store_true')
    parser.add_argument('--fan', action='store_true', help='Append the four source fan warnings to stage007')
    parser.add_argument('--source-action', type=Path)
    parser.add_argument('--skill-decal-db', type=Path)
    parser.add_argument('--skill-effect-db', type=Path)
    args = parser.parse_args()
    if args.tracking_axe_circle_candidate:
        if args.install or args.fan or args.split_four_candidates:
            parser.error('--tracking-axe-circle-candidate only writes its independent candidate')
        result = write_tracking_axe_circle_candidate(args.evidence_root, args.tracking_axe_circle_candidate)
        print(json.dumps(dict(effectAssetId=result['effectAssetId'], sourceOccurrences=len(result['sourceOccurrences']), canonicalFilesWritten=False)))
        raise SystemExit(0)
    if args.split_four_candidates:
        if args.install or args.fan:
            parser.error('--split-four-candidates cannot install or rebuild source fan warnings')
        result = write_four_candidates(args.evidence_root)
        print(json.dumps(dict(candidates=result['derivedDocuments'], canonicalFilesWritten=False)))
        raise SystemExit(0)
    if args.native_material_patch is None:
        parser.error('--native-material-patch is required for source GroundEffect restoration')
    result = build(args.evidence_root, args.native_material_patch, args.install, args.fan,
                   args.source_action, args.skill_decal_db, args.skill_effect_db)
    print(json.dumps(dict(effectAssetId=result['effectAssetId'], elements=len(result['elements']), installed=args.install)))

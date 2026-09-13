"""Recover NPC 4222003 fire-cross and Albion warning/impact library groups.

This builder owns its new authored documents and evidence only. Catalog,
Composition, gameplay and shared shader installation remain caller-owned.
"""
import argparse
import base64
import copy
import hashlib
import json
import math
import sqlite3
import struct
from pathlib import Path

import build_kouku_all_source_effects as library
import build_kouku_gate1_full_restore as source
import build_kouku_showtime_warning_groups as warning

ROOT = source.ROOT
EVIDENCE = ROOT / 'out/KoukuPatternGroups20260912/albion_cross'
AUTHORED = ROOT / 'Data/Effects/Authored'
NPC_TARGETS = {
    422200301: dict(asset='effect.kouku.firecross.warning.line', name='화염 십자_한 줄 예고',
                   system='fx_mn_istm_00-4.par_d_istm_00-4_sk02_01', count=6,
                   countIncludesEmpty=True, sourceActions=['4222003']),
    422200302: dict(asset='effect.kouku.firecross.impact.line', name='화염 십자_한 줄 폭발',
                   system='fx_mn_istm_00-4.par_d_istm_00-4_sk02_02', count=10,
                   countIncludesEmpty=True, sourceActions=['4222003']),
}


def acquire_npc(evidence):
    library.acquire(evidence / 'npc_source', NPC_TARGETS)


def prepare_native(evidence):
    """Use the MIC's own cooked texture table for its static permutation."""
    import build_kouku_pattern_native as native
    folder = evidence / 'npc_source/native'
    path = 'fx_m_mi_d_00.fx_mi.fx_d_pa_worldpospatten_01_05_tr'
    row = copy.deepcopy(native.material(path))
    own = native.obj(path)
    package = native.pkg(own['package'])
    tail = own['tail']
    assert struct.unpack_from('<I', tail)[0] == 3
    count = struct.unpack_from('<I', tail, 36)[0]
    assert count == 6
    refs = list(struct.unpack_from('<6i', tail, 40))
    cache = native.sm.package_tables(native.RELEASE / 'EV2LG3OVEH3HGV7THTFFTM7TOKMCC.upk')
    layout = native.sm.parse_shader_code_layout(cache)
    scans_path = folder / 'material_map_scan.json'
    scans = source.read(scans_path) if scans_path.exists() else {}
    if row['baseId'] not in scans:
        scans.update(native.sm.scan_base_material_contexts(cache, layout, [row['baseId']]))
        source.write(scans_path, scans)
    equality = row['mic']['engineEqualityStaticParameterSetSha256']
    context = native.sm.select_unique_map_context(scans[row['baseId']], equality)
    row['materialMap'] = native.sm.parse_material_map(cache, layout, context, equality)
    row['mapKey'] = equality
    row['effectiveTextures'] = []
    reference_paths = [native.fullref(own['package'], package, value) for value in refs]
    assert reference_paths[:4] == ['fx_tex_01.fx_d_normal_085', 'fx_tex_05.fx_k_electile_01',
                                   'fx_tex_02.fx_d_line_003_1_ycl', 'fx_tex_02.fx_d_line_004_1_ycl']
    for index, expression in enumerate(row['materialMap']['uniformExpressionSet']['pixelTexture2DExpressions']):
        name = expression.get('parameterName')
        texture_path = row['textureOverrides'].get(name) or reference_paths[expression['referencedTextureIndex']]
        texture = native.obj(texture_path)
        row['effectiveTextures'].append(dict(index=index, parameterName=name,
            sourceReferencePath=texture_path, sourceObjectPath=texture['path'], properties=texture['properties']))
    inputs = folder / 'native_material_inputs.json'
    old = source.read(inputs)['materials'] if inputs.exists() else []
    source.write(inputs, dict(materials=[r for r in old if r['sourceMaterial'] != path] + [row]))
    source.write(folder/'source_mic_texture_table.json', dict(sourceMaterial=path,
        sourceSerialSha256=own['serialSha256'], sourceExportIndex=own['exportIndex'],
        tableOffsetInNativeTail=36, references=refs, referencePaths=reference_paths,
        staticMapKey=equality, effectiveTextures=[{k:v for k,v in t.items() if k!='properties'} for t in row['effectiveTextures']],
        reason='The selected MIC static permutation owns its own cooked texture table; parent default permutation differs.'))
    native.prepare(evidence/'npc_source', 3603, 3615, [ROOT/'out'/name/'native' for name in
        ('KoukuAllEffects20260912','KoukuShowtimeRestore20260912','KoukuRainbowMatched20260911')])


def read_skill_rows(identities):
    path = source.SOURCE / 'WorldObjectExtraction-20260907/EFTable_SkillEffect.db'
    with sqlite3.connect('file:' + path.as_posix() + '?mode=ro', uri=True) as db:
        db.row_factory = sqlite3.Row
        return {identifier: dict(db.execute('SELECT * FROM SkillEffect WHERE PrimaryKey=?',
                                           (identifier,)).fetchone()) for identifier in identities}


def source_facts(evidence):
    from extract_action_effect_notifies import extract_action_document, scan_length_prefixed_strings
    folder = evidence / 'source'
    folder.mkdir(parents=True, exist_ok=True)
    archive, raw = warning.read_archive_entries('data3.lpk', ['Action/MN_ISTM_00-4.loa'])['MN_ISTM_00-4.loa']
    action_path = folder / 'MN_ISTM_00-4.loa'
    action_path.write_bytes(raw)
    npc = extract_action_document(action_path, 'MN_ISTM_00-4', action_ids={4222003}, source_logical_path=archive)
    source.write(folder / 'MN_ISTM_00-4.action-effects.json', npc)
    boss = source.read(source.SOURCE / 'RemainingCharacterExtraction-20260829/ActionNameSources/MN_RPCT_07.action-effects.json')
    boss = next(a for a in boss['actions'] if a['actionId'] == 4219903)
    source.write(folder / 'MN_RPCT_07.4219903.action.json', boss)
    area = read_skill_rows([422200301, 422200302, 421990318, 421990333, 421990334, 421990335, 421990336])
    assert [(area[k]['AreaRange'], area[k]['AreaAngle'], area[k]['AreaOffsetAngle'])
            for k in (421990333, 421990334, 421990335, 421990336)] == [(1100, 70, v) for v in (0, 90, 180, 270)]
    assert [(area[k]['AreaRange'], area[k]['AreaAngle'], area[k]['AreaOffsetX'])
            for k in (422200301, 422200302)] == [(1600, 200, -800)] * 2
    ground_name = '10_EF_PARTICLE_SOUND_DATA_GROUND_EFFECT_GR_Mon_Circle_cond_02.loa'
    ground_entry, ground = warning.read_archive_entries('data3.lpk', [ground_name])[ground_name]
    (folder / ground_name).write_bytes(ground)
    token = next(t for t in scan_length_prefixed_strings(ground, 0, len(ground)) if t['value'].startswith("MaterialInstanceConstant'"))
    end = token['sourceOffset'] + len(token['value']) + 5
    blue_color = list(struct.unpack_from('<4f', ground, end + 20))
    assert all(abs(a-b) < 1e-6 for a,b in zip(blue_color, [.03,.3,1.5,3]))
    raw = (ROOT / 'out/KoukuAllEffects20260912/source/Projectile/421990319.loa').read_bytes()
    assert struct.unpack_from('<I', raw, 1598)[0] == 2102 and struct.unpack_from('<f', raw, 1550)[0] == 2
    source.write(folder / 'source_chains.json', dict(skillEffects=area,
        blueCircle=dict(projectileId=421990319, skillDecalId=2102, radiusM=1.6, warningSeconds=2,
            sourceFixedAreaSeconds=3, activeColor=blue_color, sourceGround=ground_entry,
            sourceMaterial=token['value'].split("'")[1].lower(), projectileSha256=hashlib.sha256(raw).hexdigest()),
        fireCross=dict(actionId=4222003, particleImpactSeconds=3.1500000953674316, damageSeconds=3.3,
            fullLengthM=16, widthM=2, sourceAreaAnglesDegrees=[90,0])))
    return npc['actions'][0], boss, blue_color


def decode_particle(notify):
    """Read the separate serialized FRotator, including the named-anchor layout."""
    from build_action_cue_recipe import decode_typed_payload
    cue = decode_typed_payload(notify['sourceType'], notify['serializedPayload'], None,
                               notify['assetReferences'], notify['serializedLabels'])
    raw = base64.b64decode(notify['serializedPayload']['data'])
    if not cue['enabled']:
        return cue
    at = cue['sourceTransformByteOffset']
    variant = cue['sourceParameterCountByteOffset'] - at - 88
    assert variant in (0, 4)
    rotator = list(struct.unpack_from('<3i', raw, at + 40 + variant))
    cue['sourceFRotator'] = rotator
    cue['sourceFRotatorByteOffset'] = at + 40 + variant
    cue['localTransform']['rotationDegrees'] = [rotator[0]*360/65536, rotator[1]*360/65536, -rotator[2]*360/65536]
    return cue


def independent(template, asset, name):
    return warning.independent_document(template, asset, name)


def instance(template, asset, name, transform=None, delay=0, active=None):
    doc = independent(template, asset, name)
    for element in doc['elements']:
        element['actionCueAttachment']['enabled'] = False
        if transform:
            element['detail']['transform'].update({k: copy.deepcopy(transform[k]) for k in ('position','rotationDegrees','scale')})
        element['detail']['timing']['startDelaySeconds'] += delay
        if active is not None and element['kind'] != 'light':
            element['detail']['timing']['lifeTimeSeconds'] = active
        if 'particleSystemOccurrenceId' in element.get('sourceRecipe', {}):
            element['sourceRecipe']['particleSystemOccurrenceId'] = asset
    return doc


def merge_documents(parts, asset, name):
    doc = copy.deepcopy(parts[0])
    doc['elements'] = [e for part in parts for e in part['elements']]
    return independent(doc, asset, name)


def record(document, category, **evidence):
    # Mirror CEffectPlayback::Calculate_ElementEndSeconds for these admitted
    # source particles/decals/lights. Cascade's duration/loops and delay take
    # precedence over the notify's Timing lifetime; hidden emitters do not run.
    def f32(value):
        return struct.unpack('<f',struct.pack('<f',value))[0]
    ends=[]
    for element in document['elements']:
        if not element['visible']:continue
        timing=element['detail']['timing']
        particle=element['detail']['particle']
        recipe=element['sourceRecipe']
        assert recipe['enabled'] and element['kind'] in ('particle','decal','light')
        active=f32(timing['lifeTimeSeconds'])
        if recipe['emitterDurationSeconds']>0 and recipe['emitterLoopCount']:
            active=f32(f32(recipe['emitterDurationSeconds'])*f32(recipe['emitterLoopCount']))
        tail=0 if element['kind']=='light' else f32(f32(max(particle['lifeTimeSeconds']))*
            f32(particle['sourceScale']['lifeTime']))
        end=f32(timing['startDelaySeconds'])
        for value in (recipe['emitterDelaySeconds'],active,timing['afterImageSeconds'],tail):
            end=f32(end+f32(value))
        ends.append(end)
    playback_seconds=max(ends)
    duration=math.ceil(playback_seconds*1000)
    return dict(effectAssetId=document['effectAssetId'], displayName=document['displayName'],
        path='Data/Effects/Authored/' + document['effectAssetId'] + '.effect.json', categoryPath=category,
        durationMs=duration, playbackDurationSeconds=playback_seconds,
        durationBasis='SOURCE_RECIPE_ACTIVE_CLOCK_PLUS_TAIL_FLOAT32',
        defaultAnchorKind='MAP', elementCount=len(document['elements']), **evidence)


def compose_albion(evidence, boss, color):
    prefix = 'effect.kouku.albion.'
    category = ['KoukuSaydon','3관문','패턴','세이튼','알비온']
    circle = source.read(AUTHORED / 'effect.kouku.gate3.showtime.circle.warning.impact.effect.json')
    circle['elements'] = [next(e for e in circle['elements'] if e['id'] == 'kouku.showtime.warning.circle')]
    warning.resize_warning(circle, 1.6, 0, 2, color)
    circle = independent(circle, prefix + 'bluecircle.warning', '알비온_파란 원_예고')
    circle['elements'][0]['sourceNode'] = 'project.groundeffect.adapter.albion.2102|fx_m_mi_o_00.fx_mi.fx_o_de_condcircle_02_01_tr'
    burst = source.read(AUTHORED / 'effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_thunderstorm_cast_01_loc_int.effect.json')
    burst = independent(burst, prefix + 'thunderstorm.impact', '알비온_뇌격_폭발')
    runtime = merge_documents([circle, instance(burst, prefix+'thunderstorm.runtime.delayed', '뇌격', delay=2)],
        prefix+'bluecircle.warning.impact.runtime', '알비온_파란 원_예고·폭발_런타임')
    documents = [circle, burst, runtime]
    records = [record(doc, category+['원형 뇌격'], sourceProjectileId=421990319, sourceSkillDecalId=2102,
        sourceWarningSeconds=2, sourceFixedAreaSeconds=3, sourceRadiusM=1.6,
        sourceEvidencePath=str(evidence/'source/source_chains.json')) for doc in documents]
    records[0].update(durationMs=2000,durationBasis='SOURCE_GROUND_EFFECT_ALPHA_WINDOW',
        visibilityWindowSeconds=2, durationNote='The original ground warning alpha reaches zero at 2 seconds; the conservative decal playback drain is not an additional warning.')
    assert records[-1]['durationMs'] == 7000
    sector = source.read(AUTHORED / 'effect.kouku.gate3.showtime.sector.warning.shot.effect.json')
    sector['elements'] = [next(e for e in sector['elements'] if e['id'] == 'kouku.showtime.warning.sector')]
    warning.resize_warning(sector, 11, 0, 1.7)
    for scalar in sector['elements'][0]['material']['sourceProfile']['scalars']:
        if scalar['name'] == 'angle': scalar['value'] = 70/360
    sector = independent(sector, prefix+'fourfan.warning.single', '알비온_70도 부채꼴_한 방향 예고')
    fan_parts = [instance(sector, prefix+f'fourfan.warning.direction.{yaw}', str(yaw),
        dict(position=[0,0,0], rotationDegrees=[0,yaw,0], scale=[1,1,1])) for yaw in (0,90,180,270)]
    fan = merge_documents(fan_parts, prefix+'fourfan.warning', '알비온_네 방향 부채꼴_예고')
    stage = next(s for s in boss['stages'] if s['stageIndex'] == 15)
    calls = [n for n in stage['notifies'] if n['sourceType']=='PlayParticleEffect' and any(
        r['objectPath'].lower()=='fx_mn_rpct_07_v.par_v_rpct_crack_pjt_01_loc_int' for r in n['assetReferences'])]
    assert len(calls) == 4
    leaf = source.read(AUTHORED/'effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_crack_pjt_01_loc_int.effect.json')
    first = min(n['localTimeSeconds'] for n in calls)
    decoded = [dict(notifyId=n['notifyId'], timeSeconds=n['localTimeSeconds'], cue=decode_particle(n)) for n in calls]
    assert sorted(r['cue']['sourceFRotator'][1] for r in decoded) == [0,16384,32768,49152]
    impact = merge_documents([instance(leaf, prefix+f'fourfan.impact.direction.{i}', str(i),
        row['cue']['localTransform'], row['timeSeconds']-first) for i,row in enumerate(decoded)],
        prefix+'fourfan.impact', '알비온_네 방향 부채꼴_폭발')
    source.write(evidence/'source/albion_fourfan_particle_calls.json', decoded)
    for doc in (sector, fan, impact):
        documents.append(doc)
        row=record(doc, category+['네 방향 부채꼴'], sourceActionId=4219903, sourceStageIndex=15,
            sourceSkillDecalId=2111, sourceWarningStartSeconds=.1, sourceWarningSeconds=1.7,
            sourceDamageSeconds=1.8, sourceFullAngleDegrees=70, sourceRadiusM=11,
            sourceParticleStartSeconds=first, sourceRelativeYawDegrees=[0,90,180,270])
        if doc is sector or doc is fan:
            row.update(durationMs=1700,durationBasis='SOURCE_GROUND_EFFECT_ALPHA_WINDOW',
                visibilityWindowSeconds=1.7,durationNote='The original 0.1..1.8-second warning becomes a standalone 1.7-second alpha window.')
        records.append(row)
    return documents, records


def compose_npc(evidence, action):
    folder = evidence/'npc_source'
    patch=source.read(folder/'native/native_material_patch.json')
    own_programs={r['program']:r for r in patch['programs']}
    prior_programs={}
    for name in ('KoukuAllEffects20260912','KoukuShowtimeRestore20260912','KoukuRainbowMatched20260911'):
        for row in source.read(ROOT/'out'/name/'native/native_material_patch.json')['programs']:
            prior_programs.setdefault(row['program'],row)
    for reuse in source.read(folder/'native/reused_native_programs.json')['programs']:
        if reuse['program'] not in own_programs:
            row=copy.deepcopy(prior_programs[reuse['program']])
            row['occurrences']=reuse['occurrences']
            patch['programs'].append(row)
    projection_patch=folder/'projection_native_material_patch.json'
    source.write(projection_patch,patch)
    library.project(folder, NPC_TARGETS, projection_patch, False, folder)
    installation = source.read(folder/'installation.json')
    assert not installation['sourceFailures'], installation['sourceFailures']
    leaves = {key: source.read(folder/'candidate'/(target['asset']+'.effect.json')) for key,target in NPC_TARGETS.items()}
    documents = list(leaves.values())
    category = ['KoukuSaydon','3관문','패턴','세이튼','화염 십자']
    records = [record(doc, category+['한 줄 원본'], sourceActionId=4222003,
        sourceParticleSystem=NPC_TARGETS[key]['system']) for key,doc in leaves.items()]
    evidence_calls = []
    for phase, key, start in (('warning',422200301,0), ('impact',422200302,3.1500000953674316)):
        system = NPC_TARGETS[key]['system']
        calls = [n for n in action['stages'][0]['notifies'] if n['sourceType']=='PlayParticleEffect'
            and any(r['objectPath'].lower()==system for r in n['assetReferences'])]
        assert len(calls) == 2
        parts = []
        for index, notify in enumerate(calls):
            cue = decode_particle(notify)
            assert cue['enabled']
            evidence_calls.append(dict(phase=phase, notifyId=notify['notifyId'], sourceTimeSeconds=notify['localTimeSeconds'],
                durationSeconds=notify['durationSeconds'], cue=cue))
            parts.append(instance(leaves[key], f'effect.kouku.firecross.{phase}.direction.{index}', str(index),
                cue['localTransform'], notify['localTimeSeconds']-start,
                notify['durationSeconds'] if notify['durationSeconds']>0 else None))
        yaws = [c['cue']['sourceFRotator'][1] for c in evidence_calls if c['phase']==phase]
        assert sorted(yaws) == ([16384,32768] if phase=='warning' else [0,16384]), yaws
        doc = merge_documents(parts, 'effect.kouku.firecross.'+phase,
            '화염 십자_'+('예고' if phase=='warning' else '폭발'))
        if phase=='warning':
            # Source bKillOnDeactivate ends this notify's particles at 3 s.
            # The existing alpha track expresses that visibility boundary
            # without changing the source 5 s particle lifetime/distributions.
            track_template=source.read(AUTHORED/'effect.kouku.gate3.showtime.circle.warning.impact.effect.json')['elements'][0]['sourceTransformTrack']
            for element in doc['elements']:
                required=next(m for m in element['sourceRecipe']['modules'] if m['className']=='particlemodulerequired')
                kill=next((v['value'] for v in required['literals'] if v['propertyPath']=='bkillondeactivate'),False)
                assert kill, 'The visibility cutoff must be supported by source kill-on-deactivate'
                track=copy.deepcopy(track_template)
                track['sourceOccurrenceId']=element['sourceNode']
                track['nodes'][0]['sourceObjectPath']=element['sourcePresentation']['sourceObjectPath']
                track['alphaScaleKeys']=[dict(timeSeconds=t,value=[alpha]*3,arriveTangent=[0]*3,
                    leaveTangent=[0]*3,interpolation='constant') for t,alpha in ((0,1),(3,0))]
                element['sourceTransformTrack']=track
        documents.append(doc)
        row=record(doc, category, sourceActionId=4222003, sourceParticleStartSeconds=start,
            sourceParticleSystem=system, sourceParticleYawDegrees=[v*360/65536 for v in yaws],
            sourceRelativeYawDegrees=[0,90], sourceFullLengthM=16, sourceWidthM=2,
            sourceDamageSeconds=3.3, sourceEvidencePath=str(evidence/'source/firecross_particle_calls.json'))
        if phase=='warning':
            row.update(durationMs=3000,sourceNotifyDurationSeconds=3,
                durationBasis='SOURCE_KILL_ON_DEACTIVATE_ALPHA_WINDOW',visibilityWindowSeconds=3,
                sourceDeactivationAdapter='Original bKillOnDeactivate uses existing constant alpha track at 3 seconds.')
        records.append(row)
    source.write(evidence/'source/firecross_particle_calls.json', evidence_calls)
    from build_kouku_action_effect_groups import restored_index, project_light_occurrences
    all_source=ROOT/'out/KoukuAllEffects20260912'
    index=restored_index(all_source)
    light_system='fx_cm_02.light.par_mp_light_01'
    light_occurrences=[o for o in source.read(all_source/'source_occurrences.json')
        if o['sourceEmitter'].startswith(light_system+'.') and o['rendererShape']=='light']
    light_calls=[n for n in action['stages'][0]['notifies'] if n['sourceType']=='PlayParticleEffect'
        and any(r['objectPath'].lower()==light_system for r in n['assetReferences'])]
    assert len(light_occurrences)==1 and len(light_calls)==5
    light_parts=[]
    light_evidence=[]
    for ordinal,notify in enumerate(light_calls):
        cue=decode_particle(notify)
        projected=project_light_occurrences(index,light_occurrences,cue,notify,evidence/'npc_source')
        assert len(projected['elements'])==1, 'Original Spawn=0 leaves only one burst light'
        light_parts.append(instance(projected,'effect.kouku.firecross.light.'+str(ordinal),'원본 조명',
            cue['localTransform']))
        light_evidence.append(dict(notifyId=notify['notifyId'],sourceTimeSeconds=notify['localTimeSeconds'],cue=cue))
    lights=merge_documents(light_parts,'effect.kouku.firecross.impact.light','화염 십자_폭발 조명')
    full=merge_documents([next(d for d in documents if d['effectAssetId']=='effect.kouku.firecross.impact'),lights],
        'effect.kouku.firecross.impact.full','화염 십자_폭발·조명 전체')
    source.write(evidence/'source/firecross_light_calls.json',light_evidence)
    for doc in (lights,full):
        documents.append(doc)
        records.append(record(doc,category,sourceActionId=4222003,sourceParticleStartSeconds=3.1500000953674316,
            sourceLightParticleSystem=light_system,sourceLightCount=5,
            sourceEvidencePath=str(evidence/'source/firecross_light_calls.json')))
    # The boss owns these effects; they are independent library leaves so the
    # pattern occurrence can keep its actual boss-root/weapon attachment.
    source_action_path=source.SOURCE/'RemainingCharacterExtraction-20260829/ActionNameSources/MN_RPCT_05.action-effects.json'
    boss_action=next(a for a in source.read(source_action_path)['actions'] if a['actionId']==4219820)
    stage=next(s for s in boss_action['stages'] if s['stageIndex']==1)
    boss_calls=[]
    for suffix, source_name, title in (
        ('wand.spin','par_g_rpct_05_wand_spin_01_loc_int','지팡이 회전'),
        ('wand.decal','par_g_rpct_05_wand_decal_loc_int','지팡이 바닥')):
        system='fx_mn_rpct_05_g.'+source_name
        template=source.read(AUTHORED/('effect.kouku.source.'+system+'.effect.json'))
        notify=next(n for n in stage['notifies'] if n['sourceType']=='PlayParticleEffect'
            and any(r['objectPath'].lower()==system for r in n['assetReferences']))
        cue=decode_particle(notify)
        assert cue['enabled'] and cue['attachment']['mode']=='SNAPSHOT_ROOT'
        assert not cue['parameterOverrides'] or cue['parameterOverrides'][0]['vectorValue']==[1,1,1]
        boss_calls.append(dict(system=system,notifyId=notify['notifyId'],sourceTimeSeconds=notify['localTimeSeconds'],
            durationSeconds=notify['durationSeconds'],cue=cue))
        doc=instance(template,'effect.kouku.firecross.'+suffix,'화염 파동_'+title,cue['localTransform'],
            active=notify['durationSeconds'] if notify['durationSeconds']>0 else None)
        documents.append(doc)
        row=record(doc, category+['본체 연출'], sourceBossActions=[4219820,4219948],
            sourceStageIndex=1,sourceStageTimeSeconds=notify['localTimeSeconds'],sourceParticleSystem=system,
            sourceAnchor='SNAPSHOT_ROOT',sourceLocalTransform=cue['localTransform'],placementOwner='BOSS_ACTION_OCCURRENCE')
        row['defaultAnchorKind']='BOSS'
        records.append(row)
    source.write(evidence/'source/firecross_boss_wand_calls.json',boss_calls)
    return documents, records


def compose(evidence):
    previous={p.name:source.read(p) for p in (evidence/'candidate').glob('*.effect.json')}
    npc,boss,color=source_facts(evidence)
    documents,records=compose_albion(evidence,boss,color)
    native_patch=evidence/'npc_source/native/native_material_patch.json'
    if native_patch.exists():
        npc_documents,npc_records=compose_npc(evidence,npc)
        documents+=npc_documents
        records+=npc_records
    for doc in documents:
        path=AUTHORED/(doc['effectAssetId']+'.effect.json')
        assert doc['elements'] and len({e['id'] for e in doc['elements']})==len(doc['elements'])
        assert all(not e['actionCueAttachment']['enabled'] for e in doc['elements'])
        if path.exists():
            old=source.read(path)
            assert old==doc or old==previous.get(path.name), 'Preserve authored tuning: '+str(path)
    for doc in documents:
        path=AUTHORED/(doc['effectAssetId']+'.effect.json')
        if not path.exists() or source.read(path)!=doc:source.write(path,doc)
        source.write(evidence/'candidate'/path.name,doc)
    source.write(evidence/'installation.json',dict(installed=True,documents=records,
        manualVisualValidation='USER_PENDING', clientRun=False))
    print(json.dumps(dict(installed=len(records),elements=sum(len(d['elements']) for d in documents)),ensure_ascii=False))


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, default=EVIDENCE)
    parser.add_argument('--acquire-npc', action='store_true')
    parser.add_argument('--prepare-native', action='store_true')
    parser.add_argument('--compose', action='store_true')
    args = parser.parse_args()
    if args.acquire_npc:
        acquire_npc(args.evidence_root)
    if args.prepare_native:
        prepare_native(args.evidence_root)
    if args.compose:
        compose(args.evidence_root)


if __name__ == '__main__':
    main()

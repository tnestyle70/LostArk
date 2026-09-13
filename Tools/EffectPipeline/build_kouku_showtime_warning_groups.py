"""Build independent source GroundEffect warnings and Showtime attack groups.

The source FixedArea/SkillDecal/ParticleSound chain owns circle and donut timing.
Legacy circle/donut merged documents remain unchanged; new groups start at zero.
The fan retains its explicit authored warning/shot combination. No Composition,
gameplay authority, or UI launch.
"""
from pathlib import Path
import argparse
import copy
import hashlib
import json
import math
import mmap
import re
import sqlite3
import struct
import sys

import build_kouku_gate1_full_restore as source
from extract_action_effect_notifies import scan_length_prefixed_strings

ROOT = source.ROOT
GAME = Path('C:/ProgramData/Smilegate/Games/LOSTARK/EFGame')
ROWS = [
    dict(shape='circle', archetype='Circle', material='condcircle', decalId=2112,
         skillEffectId=421991218, projectileId=421991207, attack='fire.impact',
         displayName='원형 예고·폭발'),
    dict(shape='donut', archetype='Donut', material='condmondonut', decalId=2116,
         skillEffectId=421991220, projectileId=421991208, attack='circle.impact01',
         displayName='도넛 예고·폭발'),
    dict(shape='sector', archetype='Fan', material='condmonfan', decalId=2111,
         skillEffectId=421991212, projectileId=None, attack='gun.ground',
         displayName='부채꼴 예고·사격 (저작 조합)'),
]


def read_archive_entries(name, wanted):
    sys.path.insert(0, str(ROOT / 'Tools/LpkPipeline'))
    import unpack_lpk as lpk
    archive = GAME / name
    key, base = lpk.REGIONS['KR'][0].encode('latin1'), bytes.fromhex(lpk.REGIONS['KR'][1])
    result = {}
    with archive.open('rb') as stream, mmap.mmap(stream.fileno(), 0, access=mmap.ACCESS_READ) as packed:
        for entry in lpk.read_index(packed, key):
            relative = entry['path'].replace('\\', '/')
            if any(relative.lower().endswith(suffix.lower()) for suffix in wanted):
                result[relative.rsplit('/', 1)[-1]] = (entry['path'], lpk.extract(packed, entry, key, base))
    assert len(result) == len(wanted), ('Source archive entry closure', name, wanted, list(result))
    return result


def acquire(evidence):
    evidence.mkdir(parents=True, exist_ok=True)
    table = read_archive_entries('data2.lpk', ['EFTable_SkillDecal.db'])['EFTable_SkillDecal.db']
    decal_path = evidence / 'EFTable_SkillDecal.db'
    decal_path.write_bytes(table[1])
    skill_path = source.SOURCE / 'WorldObjectExtraction-20260907/EFTable_SkillEffect.db'
    decal_db = sqlite3.connect('file:' + decal_path.resolve().as_posix() + '?mode=ro', uri=True)
    skill_db = sqlite3.connect('file:' + skill_path.as_posix() + '?mode=ro', uri=True)
    decal_db.row_factory = skill_db.row_factory = sqlite3.Row
    ground = read_archive_entries('data3.lpk', [
        '10_EF_PARTICLE_SOUND_DATA_GROUND_EFFECT_GR_Mon_' + row['archetype'] + '_cond_EX_01.loa'
        for row in ROWS])
    fixed = read_archive_entries('data1.lpk', [str(row['projectileId']) + '.loa'
        for row in ROWS if row['projectileId']])
    acquired = []
    for row in ROWS:
        decal = dict(decal_db.execute('SELECT * FROM SkillDecal WHERE PrimaryKey=?', (row['decalId'],)).fetchone())
        area = dict(skill_db.execute('SELECT * FROM SkillEffect WHERE PrimaryKey=?', (row['skillEffectId'],)).fetchone())
        expected = 'GR_Mon_' + row['archetype'] + '_cond_EX_01'
        assert decal['DecalArchetype'] == expected
        ground_name = '10_EF_PARTICLE_SOUND_DATA_GROUND_EFFECT_' + expected + '.loa'
        ground_path, raw = ground[ground_name]
        (evidence / ground_name).write_bytes(raw)
        material = next(token for token in scan_length_prefixed_strings(raw, 0, len(raw))
                        if token['value'].startswith("MaterialInstanceConstant'"))
        material_path = material['value'].split("'")[1].lower()
        assert material_path == 'fx_m_mi_o_00.fx_mi.fx_o_de_' + row['material'] + '_02_01_tr'
        after_material = material['sourceOffset'] + 4 + len(material['value']) + 1
        dimensions = struct.unpack_from('<4f', raw, after_material)
        assert dimensions == (100.0, 100.0, -300.0, 300.0)
        timing = dict(warningSeconds=1.5, fadeInSeconds=.2, fadeOutSeconds=.2)
        fixed_source = None
        if row['projectileId']:
            fixed_path, fixed_raw = fixed[str(row['projectileId']) + '.loa']
            (evidence / (str(row['projectileId']) + '.loa')).write_bytes(fixed_raw)
            assert struct.unpack_from('<I', fixed_raw, len(fixed_raw) - 116)[0] == row['decalId']
            assert struct.unpack_from('<I', fixed_raw, len(fixed_raw) - 112)[0] == row['skillEffectId']
            timing = dict(warningSeconds=struct.unpack_from('<f', fixed_raw, len(fixed_raw) - 164)[0],
                fadeInSeconds=struct.unpack_from('<f', fixed_raw, len(fixed_raw) - 104)[0],
                fadeOutSeconds=struct.unpack_from('<f', fixed_raw, len(fixed_raw) - 92)[0])
            assert abs(timing['warningSeconds'] - 1.5) < 1e-6
            fixed_source = dict(path=fixed_path, sha256=hashlib.sha256(fixed_raw).hexdigest())
        acquired.append(dict(row, sourceMaterial=material_path, sourceGroundEffect=ground_path,
            sourceGroundEffectSha256=hashlib.sha256(raw).hexdigest(), sourceMaterialByteOffset=material['sourceOffset'],
            projectionCm=list(dimensions), thicknessCm=struct.unpack_from('<f', raw, after_material + 16)[0],
            activeColor=list(struct.unpack_from('<4f', raw, after_material + 20)),
            deactiveColor=list(struct.unpack_from('<4f', raw, after_material + 36)), skillDecal=decal,
            sourceArea={key: value for key, value in area.items() if key.startswith('Area')},
            sourceFixedArea=fixed_source, timing=timing,
            pairing='ORIGINAL_FIXED_AREA_WARNING_AND_IMPACT' if fixed_source else 'PROJECT_AUTHORED_SOURCE_FAN_AND_SHOT_CONE'))
    source.write(evidence / 'source_warning_chains.json', dict(groups=acquired))
    return acquired


def prepare_native(evidence):
    rows = acquire(evidence)
    # These empty records select the existing LocalDecal ABI in the generic
    # material extractor. They are labelled adapters, never source emitters.
    required = 'engine.default__particlemodulerequired'
    records = {required: dict(fullPath=required, classPath='engine.particlemodulerequired',
                             archetypeFullPath=None, properties={})}
    occurrences = []
    for row in rows:
        adapter = 'project.groundeffect.adapter.' + row['shape']
        records[adapter] = dict(fullPath=adapter, classPath='project.GroundEffectLocalDecalAdapter', properties={})
        occurrences.append(dict(elementId='kouku.showtime.warning.' + row['shape'], sourceEmitter=adapter,
            moduleOrder=[required], sourceMaterial=row['sourceMaterial'], rendererShape='decal', sourceMesh='',
            sourceKind='ORIGINAL_GROUND_EFFECT_MATERIAL_WITH_PROJECT_DECAL_CARRIER_ADAPTER'))
    source.write(evidence / 'source_module_inputs.json', dict(records=records,
        provenance='LocalDecal ABI adapters only. GroundEffect is not a Cascade emitter.'))
    source.write(evidence / 'source_class_defaults.json', dict(records=[]))
    source.write(evidence / 'source_occurrences.json', occurrences)
    import build_kouku_pattern_native as native
    original_object = native.obj
    def resolve_ground_texture(path):
        if path == 'engineresources.defaulttexture':
            # This engine noise input has no source payload in the installed
            # archive. A constant source white texture cancels only its sampled
            # UV derivatives; the source caustic texture and shader stay intact.
            return original_object('fx_tex_00.fx_a_blankwhite_01')
        return original_object(path)
    native.obj = resolve_ground_texture
    native.prepare(evidence, 3600, 3602)


def lower_native(evidence):
    """Use the extracted original programs with one explicit neutral noise input."""
    import build_kouku_pattern_native as native
    folder = evidence / 'native'
    materials = source.read(folder / 'native_material_inputs.json')
    white = native.obj('fx_tex_00.fx_a_blankwhite_01')
    replaced = []
    for material in materials['materials']:
        for texture in material['effectiveTextures']:
            if texture.get('sourceReferencePath', texture['sourceObjectPath']) == 'engineresources.defaulttexture':
                texture.update(sourceReferencePath='engineresources.defaulttexture', sourceObjectPath=white['path'],
                               properties=copy.deepcopy(white['properties']))
                replaced.append(dict(sourceMaterial=material['sourceMaterial'], expressionIndex=texture['index']))
    assert len(replaced) == 3
    source.write(folder / 'native_material_inputs.json', materials)
    source.write(folder / 'required_native_textures.json', {t['sourceObjectPath']: t['properties']
        for material in materials['materials'] for t in material['effectiveTextures']})
    source.write(evidence / 'engine_noise_adapter.json', dict(
        status='PROJECT_RECONSTRUCTED_NEUTRAL_ENGINE_NOISE', sourceInput='engineresources.defaulttexture',
        boundSourceTexture=white['path'], consumers=replaced,
        behavior='Constant texture cancels finite-difference UV warp; original caustic animation, geometry and color remain.',
        limitation='Original engine noise UV warping is not restored.', manualVisualValidation='USER_PENDING'))
    native.generate_native(evidence, 3600, 3602)


def install_native(evidence):
    """Append only the three GroundEffect programs to the installed corpus."""
    import install_kouku_gate1_native_materials as materials
    import install_kouku_gate1_native_shaders as shaders
    folder = evidence / 'native'
    contract = source.read(folder / 'native_runtime_contract.json')
    assert not contract['deferredPrograms']
    assert {row['program'] for row in contract['programs']} == {3600, 3601, 3602}
    materials.install(folder / 'native_runtime_contract.json', folder, ROOT / 'Client/Public/Effect_ArtistMaterial.h')
    shader_root = ROOT / 'Client/Bin/ShaderFiles'
    existing = '\n'.join(path.read_text(encoding='utf8') for path in sorted(shader_root.glob('Shader_EffectKoukuNativeGroup*.hlsli')))
    cases = shaders.installed_kouku_cases(shader_root)
    owned = {3600, 3601, 3602}
    existing_blocks = [block for block in shaders.conditional_blocks(existing)
                       if int(re.search(r'float4 ArtistNative(\d+)', block)[1]) not in owned]
    case_blocks = [block for block in shaders.conditional_blocks(cases)
                   if int(re.search(r'case (\d+)u:', block)[1]) not in owned]
    generated = (folder / 'Shader_EffectArtistNative.hlsli').read_text(encoding='utf8')
    additions = re.findall(r'#ifndef ARTIST_NATIVE_MODEL_ONLY\n(// [^\n]+\nfloat4 ArtistNative(360[012])\(ARTIST_NATIVE_INPUT input\)\n\{.*?\n\})\n#endif', generated, re.S)
    assert len(additions) == 3
    guard = ' && '.join('!defined(EFFECT_NATIVE_' + kind + '_CARRIER)' for kind in ('MESH', 'PARTICLE', 'TRAIL', 'SCREEN_POST'))
    for block, identifier in additions:
        existing_blocks.append('#if ' + guard + '\n' + block + '\n#endif\n')
        case_blocks.append('#if ' + guard + '\n#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3584 || defined(EFFECT_NATIVE_DECAL_CARRIER)\n'
            + f'    case {identifier}u: nativeColor=ArtistNative{identifier}(input); opaqueCoverage=false; break;\n#endif\n#endif\n')
    shaders.install_partitioned_groups('\n'.join(existing_blocks), ''.join(case_blocks))


def constant_distribution(name, values):
    count = len(values)
    defaults = values + [0] * (4 - count)
    return dict(propertyPath=name, sourceClass='', sourceObjectPath='', componentCount=count,
        operation=1, randomLockAxes=0, lookupTableChunkSize=0, lookupTableNumElements=0,
        lookupTableTimeScale=0, lookupTableStartTime=0, defaultMinimum=defaults,
        defaultMaximum=defaults, lookupTable=[], keys=[])


def independent_document(template, asset_id, display_name):
    document = copy.deepcopy(template)
    document.update(effectAssetId=asset_id, displayName=display_name)
    document.pop('sourceModelPreview', None)
    assert not document.get('modelCues'), 'Independent particle group cannot discard model cues'
    identities = {element['id']: asset_id + '.' + hashlib.sha256(element['id'].encode()).hexdigest()[:20]
                  for element in document['elements']}
    def remap(value):
        if isinstance(value, dict):
            return {key: remap(item) for key, item in value.items()}
        if isinstance(value, list):
            return [remap(item) for item in value]
        return identities.get(value, value) if isinstance(value, str) else value
    document = remap(document)
    for element in document['elements']:
        element['groupId'] = asset_id
    return document


def resize_warning(document, radius, inner_radius, lifetime, color=None):
    assert len(document['elements']) == 1
    element = document['elements'][0]
    detail = element['detail']
    old_lifetime = detail['timing']['lifeTimeSeconds']
    detail['timing'].update(startDelaySeconds=0, lifeTimeSeconds=lifetime)
    detail['decal']['size'] = [radius * 2] * 2
    detail['particle'].update(lifeTimeSeconds=[lifetime] * 2,
        startSize=[radius * 2] * 2, endSize=[radius * 2] * 2)
    if color is not None:
        detail['color']['multiply'] = color
    recipe = element['sourceRecipe']
    recipe.update(emitterDelaySeconds=0, emitterDurationSeconds=lifetime, emitterLoopCount=1)
    for module in recipe['modules']:
        for literal in module['literals']:
            if literal['propertyPath'] == 'emitterduration':
                literal['value'] = lifetime
        for distribution in module['distributions']:
            name = distribution['propertyPath']
            if name in ('lifetime', 'startsize'):
                replacement = constant_distribution(name, [lifetime] if name == 'lifetime' else [radius * 200] * 3)
                distribution.update(replacement)
    track = element.get('sourceTransformTrack', {})
    for key in track.get('alphaScaleKeys', []):
        key['timeSeconds'] *= lifetime / old_lifetime
    for scalar in element['material']['sourceProfile']['scalars']:
        if scalar['name'] == 'decal_drawscale':
            scalar['value'] = radius * 2
        elif scalar['name'] == 'thickness' and inner_radius:
            scalar['value'] = inner_radius / radius
    return document


def document_duration_ms(document, warning=False):
    return math.ceil(max(element['detail']['timing']['startDelaySeconds']
        + element['detail']['timing']['lifeTimeSeconds']
        + element['detail']['timing']['afterImageSeconds']
        + (0 if warning else max(element['detail']['particle']['lifeTimeSeconds']))
        for element in document['elements']) * 1000)


def install_independent_documents(documents):
    """Preflight every document before writing; authored tuning is never replaced."""
    targets = []
    for document in documents:
        assert document['elements'] and len({e['id'] for e in document['elements']}) == len(document['elements'])
        assert all(not e['actionCueAttachment']['enabled'] for e in document['elements'])
        path = ROOT / 'Data/Effects/Authored' / (document['effectAssetId'] + '.effect.json')
        if path.exists():
            assert source.read(path) == document, 'Preserve authored tuning: ' + str(path)
        targets.append((path, document))
    for path, document in targets:
        if not path.exists():
            source.write(path, document)


def compose_independent(evidence):
    """Split the three original fixed areas while retaining legacy merged assets."""
    authored = ROOT / 'Data/Effects/Authored'
    rows = source.read(evidence / 'source_warning_chains.json')['groups']
    row_by_shape = {row['shape']: row for row in rows}
    documents, records = [], []
    for shape, label, radius, inner, projectile, damage, attack in (
        ('circle', '원형', 4, 0, 421991207, 421991218, 'fire.impact'),
        ('innerdonut', '도넛1', 8, 4, 421991208, 421991220, 'circle.impact01'),
        ('outerdonut', '도넛2', 12, 8, 421991209, 421991222, 'circle.impact03')):
        source_shape = 'circle' if shape == 'circle' else 'donut'
        chain = row_by_shape[source_shape]
        raw = (ROOT / 'out/KoukuShowtimeInventory20260911/source' / (str(projectile) + '.loa')).read_bytes()
        assert struct.unpack_from('<I', raw, len(raw) - 116)[0] == chain['decalId']
        assert struct.unpack_from('<I', raw, len(raw) - 112)[0] == damage
        lead = struct.unpack_from('<f', raw, len(raw) - 164)[0]
        warning = source.read(authored / ('effect.kouku.gate3.showtime.' + source_shape + '.warning.impact.effect.json'))
        warning['elements'] = [copy.deepcopy(next(e for e in warning['elements']
            if e['id'] == 'kouku.showtime.warning.' + source_shape))]
        resize_warning(warning, radius, inner, lead)
        for phase, template in (('warning', warning), ('impact', source.read(authored / ('effect.kouku.gate3.showtime.' + attack + '.effect.json')))):
            asset_id = 'effect.kouku.gate3.showtime.' + shape + '.' + phase
            name = '쇼타임_' + label + ('_예고' if phase == 'warning' else '_폭발')
            document = independent_document(template, asset_id, name)
            documents.append(document)
            records.append(dict(effectAssetId=asset_id, displayName=name,
                authoringPath='Effects/Authored/' + asset_id + '.effect.json',
                parentPath=['KoukuSaydon', '3관문', '패턴', '세이튼', '쇼타임', '원형·도넛'],
                durationMs=document_duration_ms(document, phase == 'warning'), elementCount=len(document['elements']),
                phase=phase, radiusM=radius, innerRadiusM=inner, warningSeconds=lead,
                sourceProjectileId=projectile, sourceDamageSkillEffectId=damage,
                sourceSkillDecalId=chain['decalId'], sourceMaterial=chain['sourceMaterial'],
                sourceAttackEffectAssetId='effect.kouku.gate3.showtime.' + attack,
                pairing='ORIGINAL_FIXED_AREA_WARNING_AND_IMPACT',
                resourceId='kakulsaydon.effect.' + hashlib.sha256(asset_id.encode()).hexdigest()[:20]))
    install_independent_documents(documents)
    return records


def compose(evidence):
    """GroundEffect values use an explicit one-burst LocalDecal adapter."""
    rows = source.read(evidence / 'source_warning_chains.json')['groups']
    patches = {row['program']: row['material'] for row in source.read(evidence / 'native/native_material_patch.json')['programs']}
    authored = ROOT / 'Data/Effects/Authored'
    template = source.read(authored / 'effect.kouku.gate3.mario.boss.pentagram.full.restore.effect.json')['elements'][-1]
    records = compose_independent(evidence)
    for index, row in enumerate(rows):
        if row['shape'] != 'sector':
            continue
        asset_id = 'effect.kouku.gate3.showtime.' + row['shape'] + '.warning.' + ('shot' if row['shape'] == 'sector' else 'impact')
        attack_asset = 'effect.kouku.gate3.showtime.' + row['attack']
        attack_path = authored / (attack_asset + '.effect.json')
        if row['shape'] == 'sector':
            attack_path = authored / 'effect.kouku.source.fx_mn_rpct_07_v.par_v_rpct_gun_shot_decal_01_loc_int.effect.json'
        document = source.read(attack_path)
        document.update(effectAssetId=asset_id, displayName=row['displayName'])
        document.pop('sourceModelPreview', None)
        assert not document['modelCues']
        lead = row['timing']['warningSeconds']
        for element in document['elements']:
            assert not element['actionCueAttachment']['enabled']
            element['groupId'] = asset_id
            element['detail']['timing']['startDelaySeconds'] += lead
        warning = copy.deepcopy(template)
        identity = 'project.groundeffect.adapter.' + row['shape']
        warning.update(id='kouku.showtime.warning.' + row['shape'], displayName=row['displayName'],
            groupId=asset_id, sourceNode=identity + '|' + row['sourceMaterial'], resources=[], material=copy.deepcopy(patches[3600 + index]))
        radius = row['sourceArea']['AreaRange'] * .01
        diameter_cm = row['sourceArea']['AreaRange'] * 2
        warning['detail']['transform'].update(position=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1])
        warning['detail']['color']['multiply'] = row['activeColor']
        warning['detail']['timing'].update(startDelaySeconds=0, lifeTimeSeconds=lead)
        warning['detail']['decal'].update(size=[radius * 2] * 2, depth=6, receiverMode='upwardSurfaces', normalCutoff=.5)
        particle = warning['detail']['particle']
        particle.update(maxParticles=1, spawnRatePerSecond=0, burstCount=0,
            lifeTimeSeconds=[lead] * 2, initialPositionMin=[0, 0, 0], initialPositionMax=[0, 0, 0],
            initialVelocityMin=[0, 0, 0], initialVelocityMax=[0, 0, 0], startSize=[radius * 2] * 2,
            endSize=[radius * 2] * 2, localSpace=True, billboard=False)
        track = warning['sourceTransformTrack']
        track['sourceOccurrenceId'] = identity
        track['nodes'][0]['sourceObjectPath'] = identity
        def key(t, alpha):
            return dict(timeSeconds=t, value=[alpha] * 3, arriveTangent=[0] * 3, leaveTangent=[0] * 3, interpolation='linear')
        track['alphaScaleKeys'] = [key(0, 0), key(row['timing']['fadeInSeconds'], 1),
            key(lead - row['timing']['fadeOutSeconds'], 1), key(lead, 0)]
        def module(kind, distributions=(), literals=()):
            path = identity + '.' + kind
            return dict(stableId=path, className=kind, objectPath=path,
                literals=([] if kind == 'efparticlemoduletypedatadecal' else
                    [dict(propertyPath='benabled', kind='boolean', value=True)]) + list(literals), distributions=list(distributions))
        def number(name, value):
            return dict(propertyPath=name, kind='number', value=value)
        warning['sourceRecipe'] = dict(enabled=True, rendererShape='decal', emitterDelaySeconds=0,
            emitterDurationSeconds=lead, emitterLoopCount=1,
            bursts=[dict(timeSeconds=0, countMinimum=1, countMaximum=1)], modules=[
                module('particlemodulerequired', [constant_distribution('spawnrate', [0])],
                       [number('emitterduration', lead), number('emitterloops', 1), dict(propertyPath='buselocalspace', kind='boolean', value=True)]),
                module('particlemodulelifetime', [constant_distribution('lifetime', [lead])]),
                module('particlemodulesize', [constant_distribution('startsize', [diameter_cm] * 3)]),
                module('particlemodulecolor', [constant_distribution('startcolor', [1] * 3), constant_distribution('startalpha', [1])]),
                module('efparticlemoduletypedatadecal', [], [number('nearplane', -300), number('farplane', 300),
                       number('rotation.degrees.roll', 180 if row['shape'] == 'sector' else 0)])])
        # Original shader: thickness is hole/outer radius; angle is full turns.
        overrides = dict(decal_drawscale=diameter_cm / row['projectionCm'][0])
        if row['shape'] == 'donut':
            overrides.update(thickness=row['sourceArea']['AreaRemoveRange'] / row['sourceArea']['AreaRange'], angle=1)
        if row['shape'] == 'sector':
            overrides.update(angle=row['sourceArea']['AreaAngle'] / 360)
        for parameter in warning['material']['sourceProfile']['scalars']:
            if parameter['name'] in overrides:
                parameter['value'] = overrides[parameter['name']]
        document['elements'].insert(0, warning)
        install_independent_documents([document])
        record = dict(effectAssetId=asset_id, displayName=row['displayName'], sourceAttackEffectAssetId=attack_asset,
            sourceAttackAuthoringPath=attack_path.relative_to(ROOT).as_posix(), warningSeconds=lead,
            radiusM=radius, innerRadiusM=row['sourceArea']['AreaRemoveRange'] * .01,
            fullAngleDegrees=45 if row['shape'] == 'sector' else 360,
            materialOverrides=overrides, pairing=row['pairing'], elementCount=len(document['elements']),
            resourceId='kakulsaydon.effect.' + hashlib.sha256(asset_id.encode()).hexdigest()[:20])
        records.append(record)
    source.write(evidence / 'authored_groups.json', dict(groups=records, manualVisualValidation='USER_PENDING'))
    return records


def register_groups(evidence):
    records = source.read(evidence / 'authored_groups.json')['groups']
    path = ROOT / 'Data/Effects/EffectCatalog.json'
    catalog = source.read(path)
    for row in records:
        entry = dict(effectAssetId=row['effectAssetId'], payloadKind='DIRECT_AUTHORED_DOCUMENT',
            authoringPath='Effects/Authored/' + row['effectAssetId'] + '.effect.json')
        found = [i for i, old in enumerate(catalog['effects']) if old['effectAssetId'] == row['effectAssetId']]
        assert len(found) <= 1
        if found: catalog['effects'][found[0]] = entry
        else: catalog['effects'].append(entry)
    source.write(path, catalog)
    path = ROOT / 'Data/Effects/EffectResourceTree.json'
    original = path.read_text(encoding='utf8')
    tree = json.loads(original)
    references = tree['references']
    parent = next(row['parentId'] for row in references if row['assetId'] == 'effect.kouku.gate3.showtime.circle.impact01')
    for row in records:
        entry = dict(kind='V1', assetId=row['effectAssetId'], displayName=row['displayName'], parentId=parent)
        if any(old['assetId'] == row['effectAssetId'] for old in references):
            original = re.sub(r'^    \{"kind": "V1", "assetId": "' + re.escape(row['effectAssetId']) + r'"[^\n]*',
                              '    ' + json.dumps(entry, ensure_ascii=False) + ',', original, flags=re.M)
        else:
            marker = '  "references": [\n'
            assert original.count(marker) == 1
            original = original.replace(marker, marker + '    ' + json.dumps(entry, ensure_ascii=False) + ',\n', 1)
    json.loads(original)
    path.write_text(original, encoding='utf8')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/KoukuShowtimeWarnings20260912')
    parser.add_argument('--prepare-native', action='store_true')
    parser.add_argument('--lower-native', action='store_true')
    parser.add_argument('--install-native', action='store_true')
    parser.add_argument('--build-groups', action='store_true')
    parser.add_argument('--register-groups', action='store_true')
    args = parser.parse_args()
    if args.prepare_native:
        prepare_native(args.evidence_root.resolve())
    elif args.lower_native:
        lower_native(args.evidence_root.resolve())
    elif args.install_native:
        install_native(args.evidence_root.resolve())
    elif args.build_groups:
        compose(args.evidence_root.resolve())
    elif args.register_groups:
        register_groups(args.evidence_root.resolve())
    else:
        acquire(args.evidence_root.resolve())

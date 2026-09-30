"""Stage Waterpang projectile and speed-buff FX from the installed game.

The original Action -> SkillEffect -> Projectile/ParticleSoundNew references,
Cascade/CDO, native materials and source geometry use the existing importers.
This tool writes candidates only. It never changes the live catalog, shaders,
Resources, World authoring, or a running editor's state.
"""
from pathlib import Path
import argparse
import base64
import collections
import copy
import hashlib
import json
import re
import sqlite3
import struct

import build_gate3_world_auras as world
import build_kouku_pattern_native as native
import build_esther_inanna_source_effects as driver
from build_kouku_effect_organization import (
    ArchiveReader, effect_payload_id, extract_action_document, scan_length_prefixed_strings)
from build_action_cue_recipe import decode_typed_payload

ROOT = world.ROOT
OUT = ROOT / 'out/WaterpangEffects20260930/watergun'
PREFIX = 'effect.maharaka.watergun.'
BUFF_FILE = '9_EF_PARTICLE_SOUND_DATA_BUFF_FX_COMMON_BUFF&DEBUFF_SpdUp.loa'
TARGETS = {
    PREFIX + 'q.flight': ('fx_bs_07.gadget.par_l_watergun02_sk_01_1', '워터팡 | Q 세 갈래 물총 날아가기'),
    PREFIX + 'q.hit': ('fx_bs_07.gadget.par_l_watergun02_sk_01_2', '워터팡 | Q 세 갈래 물총 맞기'),
    PREFIX + 'q.start': ('fx_bs_07.gadget.par_l_watergun02_sk_01_3', '워터팡 | Q 세 갈래 물방울 발사'),
    PREFIX + 'w.flight': ('fx_bs_07.gadget.par_l_watergun01_sk_02', '워터팡 | W 물 폭탄 · 비행'),
    PREFIX + 'w.hit': ('fx_bs_07.gadget.par_l_watergun01_sk_02_1', '워터팡 | W 물 폭탄 · 폭발'),
    PREFIX + 'r.flight': ('fx_bs_07.gadget.par_l_watergun01_sk_03_1', '워터팡 | R 기본 사격 · 비행'),
    PREFIX + 'r.hit': ('fx_bs_07.gadget.par_l_watergun01_sk_03_2', '워터팡 | R 기본 사격 · 피격'),
    PREFIX + 'shot.start': ('fx_bs_07.gadget.par_l_watergun01_sk_01_3', '워터팡 | 물총 발사 물보라'),
    PREFIX + 'e.speed': ('fx_bs_04.state.par_d_spdfast_004', '워터팡 | E 이동속도 증가'),
}
FIRST, LAST = 5273, 5311


def configure(args):
    global OUT
    OUT = args.evidence_root.resolve()
    world.OUT, world.TOOL, world.GAME = OUT, args.umodel.resolve(), args.game_release.resolve()
    world.TARGETS = TARGETS
    native.UMODEL = native.source_native.UMODEL = world.TOOL
    native.RELEASE = native.source_native.RELEASE = world.GAME
    native.source_native.packages = world.PACKAGES


def original_contract():
    source = OUT / 'source'
    receipts = []
    for archive_name, domain, names in (
        ('data1.lpk', 'Projectile', ['569020.loa', '570020.loa', '569120.loa', '569320.loa']),
        ('data3.lpk', 'Action', ['GADGET.loa']),
        ('data3.lpk', 'ParticleSoundNew', [BUFF_FILE]),
        ('data2.lpk', 'TableData', ['EFTable_SkillEffect.db', 'EFTable_SkillBuff.db'])):
        archive = ArchiveReader(world.GAME.parent / archive_name, source)
        try:
            for name in names:
                assert archive.acquire(domain, name), (domain, name)
            receipts.extend(archive.receipts)
        finally:
            archive.close()
    actions = extract_action_document(source / 'Action/GADGET.loa', 'GADGET')
    launch = []
    with sqlite3.connect((source / 'TableData/EFTable_SkillEffect.db').as_uri() + '?mode=ro', uri=True) as database:
        database.row_factory = sqlite3.Row
        for action in actions['actions']:
            if action['actionId'] not in (56902, 57002, 56912, 56932):
                continue
            for stage in action['stages']:
                for notify in stage['notifies']:
                    if notify['sourceType'] != 'Effect':
                        continue
                    identifier, offset = effect_payload_id(notify['serializedPayload'])
                    raw = base64.b64decode(notify['serializedPayload']['data'])
                    # EFSkillEffectNotifyInfo rows are 60 bytes. MK2 Q contains
                    # three simultaneous rows; the first ID alone loses its fan.
                    count = struct.unpack_from('<i', raw, offset - 8)[0]
                    assert 1 <= count <= 3 and offset + count * 60 <= len(raw)
                    for ordinal in range(count):
                        at = offset + ordinal * 60
                        identifier = struct.unpack_from('<i', raw, at)[0]
                        row = dict(database.execute('SELECT * FROM SkillEffect WHERE PrimaryKey=?', (identifier,)).fetchone())
                        assert row['Key'] == 12 and row['ValueA'] == identifier
                        launch.append(dict(actionId=action['actionId'], projectileId=identifier,
                            projectileOrdinal=ordinal, projectileCount=count,
                            timeSeconds=notify['localTimeSeconds'], sourcePositionCm=list(struct.unpack_from('<3f', raw, at + 40)),
                            sourceAngleDegrees=struct.unpack_from('<i', raw, at + 4)[0], skillEffect=row))
    particles = []
    for name in ('569020', '570020', '569120', '569320'):
        raw = (source / 'Projectile' / (name + '.loa')).read_bytes()
        refs = [t for t in scan_length_prefixed_strings(raw, 0, len(raw)) if t['value'].startswith("ParticleSystem'")]
        for i, token in enumerate(refs):
            # Only the action wrapper is supplied to reuse the bounded common
            # CEFParticleData decoder. Original bytes after the reference stay
            # unchanged and their true archive offset is retained separately.
            header = b'CEFActionNotify_PlayParticleEffect\0'.ljust(47, b'\0') + b'\1'
            cue = decode_typed_payload('PlayParticleEffect', dict(data=base64.b64encode(header + raw[token['sourceOffset']:]).decode()))
            slot = ('TrailParticleData', 'ExplodeParticleData', 'StartFXParticleData')[i]
            particles.append(dict(projectileId=int(name), slot=slot,
                system=token['value'].split("'")[1].lower(), sourceByteOffset=token['sourceOffset'],
                localTransform=cue['localTransform'], sourceAnchors=cue['attachment']['sourceAnchorNames'],
                parameterOverrides=cue['parameterOverrides']))
    with sqlite3.connect((source / 'TableData/EFTable_SkillBuff.db').as_uri() + '?mode=ro', uri=True) as database:
        database.row_factory = sqlite3.Row
        buff = dict(database.execute('SELECT * FROM SkillBuff WHERE PrimaryKey=569200').fetchone())
    assert buff['Archetype'] == 'SpdUp' and buff['Duration'] == 5000 and buff['BuffFXApply'] == 1
    raw = (source / 'ParticleSoundNew' / BUFF_FILE).read_bytes()
    strings = scan_length_prefixed_strings(raw, 0, len(raw))
    assert next(t['value'] for t in strings if t['value'].startswith("ParticleSystem'")) == "ParticleSystem'FX_BS_04.State.Par_D_SpdFast_004'"
    assert any(t['value'] == 'FX_Buff_01' for t in strings)
    assert struct.unpack_from('<3f', raw, 575) == (0, 0, 0)
    assert struct.unpack_from('<3f', raw, 635) == (1, 1, 1)
    raw = (source / 'Projectile/569120.loa').read_bytes()
    grenade = dict(minHeightCm=struct.unpack_from('<i', raw, len(raw)-40)[0],
        maxHeightCm=struct.unpack_from('<i', raw, len(raw)-36)[0],
        maxHeightRatio=struct.unpack_from('<f', raw, len(raw)-32)[0],
        standardDistanceCm=struct.unpack_from('<f', raw, len(raw)-24)[0])
    contract = dict(launches=launch, particles=particles, speedBuff=buff, grenade=grenade,
        speedBuffSource=dict(system=TARGETS[PREFIX+'e.speed'][0], sourceAnchor='FX_Buff_01',
            position=[0, 0, 0], scale=[1, 1, 1], sourceParticleByteOffset=427),
        sourceInputs=receipts, basis='Source +X forward; document yaw -90 maps to gameplay +Z. Launch offset belongs to the authoritative projectile root.',
        startFxScales={'569020': 1, '570020': 1, '569320': .7})
    world.source.write(OUT / 'source_contract.json', contract)
    return contract


def acquire():
    original_contract()
    index, occurrences, records = world.acquire()
    for row in occurrences:
        row['sourceType'] = 'WaterpangProjectileOrBuff'
        row['sourceDurationSeconds'] = 5 if row['effectAssetId'].endswith('.e.speed') else 1.5 if row['effectAssetId'].endswith('.flight') else 0
    world.source.write(OUT / 'source_occurrences.json', occurrences)
    world.source.write(OUT / 'targets.json', TARGETS)
    return index, occurrences, records


def installed_materials():
    # Reuse exact material/renderer permutations, preferring this Waterpang
    # cohort. Generic grouped approximations cannot replace native programs.
    installed = {}
    paths = sorted((ROOT / 'Data/Effects/Authored').glob('*.effect.json'),
        key=lambda path: (not path.name.startswith(PREFIX), path.name))
    for path in paths:
        for element in world.source.read(path).get('elements', []):
            material = element.get('material', {})
            profile = material.get('sourceProfile', {})
            if not profile.get('enabled') or not re.fullmatch(
                    r'effect\.ue3\.kouku-\d+-native\.v1', profile.get('runtimeShaderProfileId', '')):
                continue
            key = (material.get('sourceMaterialPath'), element.get('sourceRecipe', {}).get('rendererShape'))
            installed.setdefault(key, material)
    return installed


def materials():
    installed = installed_materials()
    occurrences = world.source.read(OUT / 'source_occurrences.json')
    if all((row['sourceMaterial'], row['rendererShape']) in installed for row in occurrences):
        world.source.write(OUT / 'native/native_material_patch.json', dict(programs=[
            dict(occurrences=[row['elementId']], material=installed[row['sourceMaterial'], row['rendererShape']])
            for row in occurrences]))
        return
    existing = (ROOT / 'Client/Private/Effect_ArtistMaterial_Tables.inl').read_text(encoding='utf8')
    used = {int(v) for v in re.findall(r'ARTIST_PARAMETERS_(\d+)', existing)}
    assert not used.intersection(range(FIRST, LAST+1)), 'Native program range is already installed'
    reuse = ROOT / 'out/BattleItemEffects20260929/native'
    native.prepare(OUT, FIRST, LAST, [reuse] if reuse.exists() else [], resource_root=OUT / 'Resources')


def project():
    index, occurrences, records = acquire()
    source = world.source
    contract = source.read(OUT / 'source_contract.json')
    source.SELECTED = {asset: ([], name) for asset, (_, name) in TARGETS.items()}
    notifies = []
    for asset, (system, _) in TARGETS.items():
        parameters = next((p['parameterOverrides'] for p in contract['particles'] if p['system'] == system), [])
        notifies.append(dict(notifyId=asset, sourceType='WaterpangProjectileOrBuff',
            cue=dict(parameterOverrides=parameters), actionId=asset))
    source.project(OUT, index, notifies, occurrences, records, OUT / 'projected')
    patch_path = OUT / 'native/native_material_patch.json'
    patches = source.read(patch_path)['programs'] if patch_path.is_file() else []
    patched = {key.replace('.shot.end.', '.shot.start.'): row['material'] for row in patches for key in row['occurrences']}
    installed = installed_materials()
    by_element = {row['elementId']: row for row in occurrences}
    resource_root = ROOT / 'Client/Bin/Resources'
    summary = []
    for asset, (_, name) in TARGETS.items():
        document = source.read(OUT / 'projected' / ('effect.kouku.gate1.' + asset + '.full.restore.effect.json'))
        document.update(effectAssetId=asset, displayName=name)
        document['particleSystem']['yawOffsetDegrees'] = -90
        for element in document['elements']:
            element['groupId'] = asset
            element['displayName'] = element['id'].rsplit('.', 1)[-1]
            element['actionCueAttachment']['enabled'] = False
            occurrence = by_element[element['id']]
            key = (occurrence['sourceMaterial'], occurrence['rendererShape'])
            material = installed.get(key) or patched.get(element['id'])
            assert material and material['sourceMaterialPath'] == occurrence['sourceMaterial'], key
            element['material'] = copy.deepcopy(material)
            element['detail']['uv'].update(start=[0, 0], speed=[0, 0], wave=False, sequence=False)
            element['detail']['color']['emissiveIntensity'] = 1
            if element['sourceRecipe']['rendererShape'] == 'mesh':
                element['detail']['mesh']['useModelMaterial'] = False
                typed = next(index.objects[key] for key in by_element[element['id']]['moduleOrder']
                    if index.objects[key].class_name == 'particlemoduletypedatamesh')
                assert source.imported.prop(typed.properties, 'boverridematerial', False)
                element['detail']['mesh']['sourceTypeDataRotationDegrees'] = [
                    float(source.imported.prop(typed.properties, key, 0)) for key in ('roll', 'pitch', 'yaw')]
                mesh_name = by_element[element['id']]['sourceMesh'].rsplit('.', 1)[-1]
                preferred = resource_root / 'Effect/KoukuSaydon/FullRestore/Meshes' / (mesh_name + '.wmodel')
                package = by_element[element['id']]['sourceMesh'].split('.')[0]
                paths = [preferred] if preferred.is_file() else sorted((resource_root / 'Effect').rglob(mesh_name + '.wmodel'),
                    key=lambda p: (package not in [part.lower() for part in p.parts], p.as_posix()))
                assert paths, ('Source geometry missing', mesh_name)
                for resource in element['resources']:
                    if resource['slotId'] == 'meshModel':
                        resource['assetId'] = paths[0].relative_to(resource_root).as_posix()
            for resource in element['resources'] + element['material']['sourceProfile'].get('textures', []):
                relative = resource['assetId']
                assert (resource_root / relative).is_file() or (OUT / 'Resources' / relative).is_file(), relative
        driver.bind_providers(document, index, by_element)
        for element in document['elements']:
            if element['sourceRecipe'].get('simulationOnly'):
                # ERM_None/Point remains a simulation provider even when its
                # source names a real material. It has no surface consumer.
                element['resources'] = []
                element['material'] = dict(templateId='effect.standard',
                    sourceMaterialPath=by_element[element['id']]['sourceMaterial'],
                    renderProfile='alpha_two_sided_depth_read', sourceProfile=dict(enabled=False))
                element['detail']['mesh'].pop('sourceMaterialSlots', None)
        driver.clamp_trail_budget(document)
        audit(document)
        source.write(OUT / 'candidate' / (asset + '.effect.json'), document)
        summary.append(dict(effectAssetId=asset, elementCount=len(document['elements']),
            shapes=dict(collections.Counter(e['sourceRecipe']['rendererShape'] for e in document['elements']))))
    source.write(OUT / 'candidate_summary.json', summary)
    print(json.dumps(summary, ensure_ascii=False))


def audit(document):
    # The native runtime normalizes UE seeded module subclasses to their base
    # class; the older generic structural audit predates this contract.
    normalized = copy.deepcopy(document)
    for element in normalized['elements']:
        if element['sourceRecipe'].get('simulationOnly'):
            material = element['material']
            assert element['sourceRecipe']['rendererShape'] == 'sprite' and not element['resources']
            assert material['templateId'] == 'effect.standard' and not material.get('sourceProfile', {}).get('enabled')
            assert not material.get('execution', {}).get('enabled') and not element['detail']['mesh'].get('sourceMaterialSlots')
            required = next(m for m in element['sourceRecipe']['modules'] if m['className'] == 'particlemodulerequired')
            assert any(v['propertyPath'] == 'source.emitterrendermode' and v['value'] in ('erm_none', 'erm_point')
                       for v in required['literals'])
        for module in element['sourceRecipe']['modules']:
            if module['className'].endswith('_seeded'):
                module['className'] = module['className'][:-7]
    driver.audit_document(normalized)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--stage', choices=('source', 'materials', 'project'), default='source')
    parser.add_argument('--evidence-root', type=Path, default=OUT)
    parser.add_argument('--umodel', type=Path, required=True)
    parser.add_argument('--game-release', type=Path, default=world.GAME)
    args = parser.parse_args()
    configure(args)
    {'source': acquire, 'materials': materials, 'project': project}[args.stage]()

"""Recover four retail battle-item effects through the existing World carriers.

Item/SkillEffect/Projectile and ParticleSoundNew identities are recorded in the
receipt; visual inputs come from the original Cascade/CDO/material closure.
Candidates remain in out until explicit installation. Existing authored edits
are never overwritten by a repeat installation.
"""
from pathlib import Path
import argparse
import copy
import hashlib
import json
import os
import shutil
import sys
import tempfile

import build_gate3_world_auras as world
import build_kouku_pattern_native as native

ROOT = world.ROOT
OUT = ROOT / 'out/BattleItemEffects20260929'
TARGETS = {
    'effect.world.item.destruction_bomb':
        ('fx_cm_01.explosion.par_l_steelbomb_exp_01_loc_int', '파괴 폭탄'),
    'effect.world.item.whirlwind_grenade':
        ('fx_cm_01.explosion.par_l_battle_wind_bomb_01', '회오리 수류탄'),
    'effect.world.item.holy_charm':
        ('fx_bs_02.item.par_k_bi_holyamul_01', '성스러운 부적'),
    'effect.world.item.time_stop':
        ('fx_bs_04.state.par_k_timepause_02_l', '시간 정지 물약'),
    'effect.world.item.destruction_bomb.flight':
        ('fx_bs_05.prj.par_l_steelbomb_prj_01', '파괴 폭탄 · 비행'),
    'effect.world.item.whirlwind_grenade.flight':
        ('fx_bs_05.prj.par_l_battle_wind_prj_01', '회오리 수류탄 · 비행'),
}

# Original Projectile 32140/32310 Grenade resource ResScale. This belongs to
# the flight particle-system resource, independently of its StartSize curves
# and the mesh cm-to-m pre-scale; the replicated projectile root stays at 1.
FLIGHT_RESOURCE_SCALES = {
    'effect.world.item.destruction_bomb.flight': 1.1,
    'effect.world.item.whirlwind_grenade.flight': 1.2,
}


def configure(args):
    global OUT
    OUT = args.evidence_root.resolve()
    world.OUT, world.TOOL, world.GAME = OUT, args.umodel.resolve(), args.game_release.resolve()
    world.TARGETS = TARGETS
    native.UMODEL = native.source_native.UMODEL = world.TOOL
    native.RELEASE = native.source_native.RELEASE = world.GAME
    native.source_native.packages = world.PACKAGES


def acquire():
    index, occurrences, records = world.acquire()
    for row in occurrences:
        row['sourceType'] = 'BattleItem'
        mesh_materials = [index.objects[key] for key in row['moduleOrder']
                          if index.objects[key].class_name == 'particlemodulemeshmaterial']
        if mesh_materials:
            assert len(mesh_materials) == 1
            overrides = [path for prop, path in mesh_materials[0].reference_paths if prop == 'meshmaterials']
            assert len(overrides) == 1
            row['sourceRequiredMaterial'] = row['sourceMaterial']
            row['sourceMaterial'] = overrides[0]
        # Explosions preserve their source emitter windows and tails. TimeStop
        # is an owner-sustained loop whose authoritative buff lasts 3 seconds.
        row['sourceDurationSeconds'] = (3 if row['effectAssetId'].endswith('.time_stop') else
                                        10 if row['effectAssetId'].endswith('.flight') else 0)
    world.source.write(OUT / 'source_occurrences.json', occurrences)
    return index, occurrences, records


def project():
    index, occurrences, records = acquire()
    source = world.source
    source.SELECTED = {asset: ([], label) for asset, (_, label) in TARGETS.items()}
    notifies = [dict(notifyId=asset, sourceType='BattleItem', cue={}, actionId=asset) for asset in TARGETS]
    source.project(OUT, index, notifies, occurrences, records, OUT / 'projected')
    materials = {key: row['material'] for row in source.read(OUT / 'native/native_material_patch.json')['programs']
                 for key in row['occurrences']}
    by_element = {row['elementId']: row for row in occurrences}
    for asset, (_, label) in TARGETS.items():
        document = source.read(OUT / 'projected' / ('effect.kouku.gate1.' + asset + '.full.restore.effect.json'))
        document.update(effectAssetId=asset, displayName=label)
        if asset in FLIGHT_RESOURCE_SCALES:
            document['particleSystem']['uniformScaleMultiplier'] = FLIGHT_RESOURCE_SCALES[asset]
        if asset == 'effect.world.item.time_stop':
            document['ownerControls'] = [dict(controlId='battle.item.time_stop.buffcolor',
                kind='MATERIAL_VECTOR', parameter='buffcolor', mappingBasis='PROJECT_ADAPTER',
                sourceTargetType=4, onlyLocalPlayer=False, startSeconds=0,
                keys=[dict(seconds=seconds, value=value) for seconds, value in (
                    (0, [0, 0, 0, 0]), (.5, [.8, .8, .8, 0]),
                    (2.5, [.8, .8, .8, 0]), (3, [0, 0, 0, 0]))],
                sourceValues=[1, 0, 0, .8, .8, .8, 0, 0, 0, 0, 0, .5, .5, -1, 0, -1,
                              0, 0, 0, 1, 2, 0, 0, 0, .6, 0])]
        for element in document['elements']:
            element['groupId'] = asset
            element['actionCueAttachment']['enabled'] = False
            element['material'] = copy.deepcopy(materials[element['id']])
            element['detail']['uv'].update(start=[0, 0], speed=[0, 0], wave=False, sequence=False)
            element['detail']['color']['emissiveIntensity'] = 1
            if element['sourceRecipe']['rendererShape'] == 'mesh':
                element['detail']['mesh']['useModelMaterial'] = False
                typed = next(index.objects[key] for key in by_element[element['id']]['moduleOrder']
                    if index.objects[key].class_name == 'particlemoduletypedatamesh')
                element['detail']['mesh']['sourceTypeDataRotationDegrees'] = [
                    float(source.imported.prop(typed.properties, field, 0)) for field in ('roll', 'pitch', 'yaw')]
                if not source.imported.prop(typed.properties, 'boverridematerial', False):
                    original = by_element[element['id']]
                    assert original.get('sourceRequiredMaterial')
                    element['detail']['mesh']['sourceMaterialSlots'] = [
                        dict(sourceMaterialIndex=0, material=copy.deepcopy(element['material']))]
                    element['material'] = dict(templateId='effect.standard',
                        sourceMaterialPath=original['sourceRequiredMaterial'],
                        renderProfile='alpha_two_sided_depth_read', sourceProfile=dict(enabled=False),
                        execution=dict(enabled=False, failClosed=True))
                    module = next(m for m in element['sourceRecipe']['modules'] if m['className'] == 'particlemodulemeshmaterial')
                    literal = next(value for value in module['literals'] if value['propertyPath'] == 'meshmaterials.objectpath')
                    literal['propertyPath'] = 'meshmaterials[0].objectpath'
                for resource in element['resources']:
                    resource['assetId'] = resource['assetId'].replace('Effect/KoukuSaydon/FullRestore/Meshes/', 'Effect/World/Meshes/')
            for resource in element['resources'] + element['material']['sourceProfile'].get('textures', []):
                relative = resource['assetId']
                assert (ROOT / 'Client/Bin/Resources' / relative).is_file() or (OUT / 'Resources' / relative).is_file(), relative
        source.write(OUT / 'candidate' / (asset + '.effect.json'), document)


def replace_checked(path, previous, content):
    if path.exists():
        assert path.read_bytes() == previous, ('Concurrent edit', path)
    else:
        assert previous is None
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, staged = tempfile.mkstemp(prefix=path.name + '.', suffix='.tmp', dir=path.parent)
    try:
        with os.fdopen(fd, 'wb') as stream:
            stream.write(content)
        assert (path.read_bytes() if path.exists() else None) == previous, ('Concurrent edit', path)
        os.replace(staged, path)
    finally:
        if os.path.exists(staged):
            os.unlink(staged)


def install():
    source = world.source
    contract = source.read(OUT / 'native/merged_native_runtime_contract.json')
    assert not contract.get('deferredPrograms')
    pending = []
    for asset in TARGETS:
        path = ROOT / 'Data/Effects/Authored' / (asset + '.effect.json')
        payload = (OUT / 'candidate' / path.name).read_bytes()
        previous = path.read_bytes() if path.exists() else None
        assert previous is None or previous == payload, ('Preserve existing authored edit', path)
        pending.append((path, previous, payload))
    for path in (OUT / 'Resources').rglob('*'):
        if not path.is_file():
            continue
        destination = ROOT / 'Client/Bin/Resources' / path.relative_to(OUT / 'Resources')
        assert not destination.exists() or destination.read_bytes() == path.read_bytes(), destination
        destination.parent.mkdir(parents=True, exist_ok=True)
        if not destination.exists():
            shutil.copyfile(path, destination)
    import install_kouku_gate1_native_shaders as shaders
    shaders.append_reviewed(OUT / 'native')
    for path, previous, payload in pending:
        replace_checked(path, previous, payload)
    catalog_path = ROOT / 'Data/Effects/EffectCatalog.json'
    previous = catalog_path.read_bytes()
    catalog = json.loads(previous)
    for asset in TARGETS:
        row = dict(effectAssetId=asset, payloadKind='DIRECT_AUTHORED_DOCUMENT',
                   authoringPath='Effects/Authored/' + asset + '.effect.json')
        existing = next((entry for entry in catalog['effects'] if entry['effectAssetId'] == asset), None)
        assert existing is None or existing == row
        if existing is None:
            catalog['effects'].append(row)
    replace_checked(catalog_path, previous, (json.dumps(catalog, ensure_ascii=False, indent=2) + '\n').encode())
    tree_path = ROOT / 'Data/Effects/EffectResourceTree.json'
    previous = tree_path.read_bytes()
    text = previous.decode('utf8')
    anchor = next(line for line in text.splitlines(keepends=True) if '"assetId": "effect.world.move_destination"' in line)
    additions = ''.join('    ' + json.dumps(dict(kind='V1', assetId=asset, displayName=label, parentId='world'),
                                         ensure_ascii=False) + ',\n'
                        for asset, (_, label) in TARGETS.items() if '"assetId": "' + asset + '"' not in text)
    replace_checked(tree_path, previous, text.replace(anchor, anchor + additions).encode())
    for suffix in ('', '.filters'):
        path = ROOT / ('Client/Default/Client.vcxproj' + suffix)
        previous = path.read_bytes()
        text = previous.decode('utf8')
        newline = '\r\n' if '\r\n' in text else '\n'
        anchor = next(line for line in text.splitlines(keepends=True)
                      if 'effect.world.move_destination.effect.json' in line)
        if suffix:
            start = text.index(anchor)
            end = text.index('    </None>', start) + len('    </None>' + newline)
            anchor = text[start:end]
        additions = ''
        for asset in TARGETS:
            relative = '..\\..\\Data\\Effects\\Authored\\' + asset + '.effect.json'
            if relative not in text:
                additions += ('    <None Include="' + relative + '">' + newline +
                              '      <Filter>96.DataFiles\\Effects</Filter>' + newline +
                              '    </None>' + newline) if suffix else '    <None Include="' + relative + '" />' + newline
        replace_checked(path, previous, text.replace(anchor, anchor + additions).encode())


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--stage', choices=('source', 'materials', 'geometry', 'project', 'install'), default='source')
    parser.add_argument('--evidence-root', type=Path, default=OUT)
    parser.add_argument('--game-release', type=Path, default=world.GAME)
    parser.add_argument('--umodel', type=Path, required=True)
    args = parser.parse_args()
    configure(args)
    if args.stage == 'materials':
        native.prepare(OUT, 5312, 5375, resource_root=OUT / 'Resources')
    elif args.stage == 'geometry':
        world.prepare_geometry()
    else:
        {'source': acquire, 'project': project, 'install': install}[args.stage]()

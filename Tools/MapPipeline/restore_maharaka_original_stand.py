"""Replace only the approved 35-piece stand with its resolved SCENE04A mesh.

Authoring/resource transaction only; Publish-MapAuthoring/WorldGameplay remain
the exclusive runtime-document writers. Requires the preserved source audit.
"""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import re
import sys
from types import SimpleNamespace

ROOT = Path(__file__).resolve().parents[2]
for directory in ('LevelPlacementExtractor', 'VehiclePipeline', 'EffectPipeline', 'WorldPipeline'):
    sys.path.insert(0, str(ROOT/'Tools'/directory))
from restore_maharaka_missing_material_slots import append_preserving_text, inspect_model
from source_character_registration import commit_staged_files
from test_maharaka_npc_population import mesh_triangles, transform, rows
import build_vehicle_source_material as native

AREA = 'LV_OCN_EVENTIS_MHP'
ASSET = 'MAP_65096D72C5C9_ITR_02453_SK'
SOURCE = AREA+'_SCENE04A:export:612'
PREFIX = 'reconstruction.maharaka.mokomoko.'
MODEL = f'Map/{AREA}/{ASSET}/{ASSET}.wmodel'


def top_at(triangles, placement, x, z):
    heights = []
    for triangle in triangles:
        a, b, c = [transform(v, placement) for v in triangle]
        den = (b[2]-c[2])*(a[0]-c[0])+(c[0]-b[0])*(a[2]-c[2])
        if abs(den) < 1e-10:
            continue
        u = ((b[2]-c[2])*(x-c[0])+(c[0]-b[0])*(z-c[2]))/den
        v = ((c[2]-a[2])*(x-c[0])+(a[0]-c[0])*(z-c[2]))/den
        if min(u, v, 1-u-v) >= -1e-7:
            heights.append(u*a[1]+v*b[1]+(1-u-v)*c[1])
    if not heights:
        raise ValueError('No original support triangle beneath the current head')
    return max(heights)


def replace_table(before, remove, addition):
    text = before.decode('utf-8-sig')
    newline = '\r\n' if '\r\n' in text else '\n'
    lines = text.splitlines()
    kept = [line for line in lines[1:] if line.strip() and not remove(line)]
    kept.append(addition)
    header = re.sub(r'\d+$', str(len(kept)), lines[0])
    return (header+newline+newline.join(kept)+newline).encode('utf-8')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--evidence', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--apply', action='store_true')
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    closure = args.evidence/'reference-closure'
    audit = json.loads((closure/'audit.json').read_text(encoding='utf-8'))
    if audit['sourcePlacementId'] != SOURCE or audit['sourceMesh'] != 'itr_02453.mesh.itr_02453_sk':
        raise ValueError('Wrong original actor/component identity')
    scene = json.loads((closure/'scene04.json').read_text(encoding='utf-8'))
    package = Path(scene['package']['physicalPackage'])
    if hashlib.sha256(package.read_bytes()).hexdigest() != scene['package']['sha256']:
        raise ValueError('Source level changed after reference audit')
    model = args.evidence/f'original-stand/runtime/{ASSET}/{ASSET}.wmodel'
    names = inspect_model(model)[2]
    expected_names = [f'itr_02453_{n:02d}_mi' for n in (4, 1, 2, 3)]
    if names[:4] != expected_names or any(names[4:]):
        raise ValueError('Cooked material slot identity changed')
    source_receipt = json.loads((args.evidence/f'original-stand/source/{ASSET}/source.receipt.json').read_text())
    resources = ROOT/'Client/Bin/Resources'
    staged, dependencies = {}, {}

    def stage(path, after):
        before = path.read_bytes() if path.exists() else None
        if before != after:
            staged[path] = before, after

    texture_map = {}
    mip_count = 0
    for path in sorted((closure/'mips').glob('*.dds')):
        receipt = json.loads(path.with_suffix('.receipt.json').read_text())
        if receipt['status'] != 'SOURCE_MIP_CHAIN_VALIDATED' or receipt['sourceObject'] != 'itr_02453.tex.'+path.stem:
            raise ValueError('Texture source mismatch')
        dependencies[path] = path.read_bytes()
        target = f'Map/{AREA}/{ASSET}/textures/{path.name}'
        stage(resources/target, dependencies[path])
        texture_map[path.stem] = target
        mip_count += int.from_bytes(dependencies[path][28:32], 'little')
    if len(texture_map) != 10:
        raise ValueError('The original ten source textures are required')
    # The same parent shader references the original engine white mask.
    white = list((args.evidence.parent/'psk').rglob('flat_white.png'))
    if len(white) != 1:
        raise ValueError('Expected one exported EFMASTER_MATERIAL_PROLOGUE flat_white')
    from PIL import Image
    white_asset = f'Map/{AREA}_SOURCE_MATERIALS/EngineDefaults/726b99cf1d98_flat_white.dds'
    expected_white = Image.open(white[0]).convert('RGBA')
    installed_white = Image.open(resources/white_asset).convert('RGBA')
    if expected_white.size != installed_white.size or expected_white.tobytes() != installed_white.tobytes():
        raise ValueError('Installed engine-default DDS differs from original white mask')
    texture_map['flat_white'] = white_asset
    dependencies[resources/white_asset] = (resources/white_asset).read_bytes()
    stage(resources/MODEL, model.read_bytes())
    texture_file = args.out/'textures.json'
    texture_file.write_text(json.dumps(texture_map, indent=2), encoding='utf-8')
    native_file = args.out/'native-rows.json'
    entries = [str(closure/f'native-static/itr_02453.mat.{name}.json')+
               f'=source.character.maharaka-itr02453-{int(name[10:12]):02d}.v1@{name}' for name in expected_names]
    native.command_rows(SimpleNamespace(texture_map=texture_file, model=MODEL, out=native_file, entry=entries))
    added = json.loads(native_file.read_text())
    for row, material in zip(added, source_receipt['materials']):
        del row['modelAssetId']
        row['assetId'] = ASSET
        row['renderMode'] = 'deferred'
        row['cullMode'] = 'none' if material['renderFlags']['twoSided'] else 'back'

    catalog = ROOT/f'Data/Maps/Imported/{AREA}/{AREA}.mapassets'
    placements = ROOT/f'Data/Maps/Authoring/{AREA}/{AREA}.mapplacements'
    materials = placements.with_name(AREA+'.mapmaterials.json')
    world = ROOT/f'Data/Worlds/{AREA}/Gameplay.world.json'
    for p in (catalog, placements, materials, world, model):
        dependencies[p] = p.read_bytes()
    old_rows = rows(placements)
    previous = [r for r in old_rows if r[1].startswith(PREFIX)]
    original = [r for r in old_rows if r[1] == SOURCE]
    if original and not previous:
        installed = json.loads(dependencies[materials].decode('utf-8-sig'))
        bindings = [r for r in installed['materials'] if r['assetId'] == ASSET]
        if len(original) != 1 or original[0][4] != ASSET or bindings != added or staged:
            raise ValueError('Existing original stand resources or bindings changed; no overwrite')
        print('Original stand already installed; resource bytes and four bindings match; no authoring changes.')
        return
    if len(previous) != 35 or original or any(r[0] == ASSET for r in rows(catalog)):
        raise ValueError('Expected exactly 35 old stand pieces and no original stand')
    stable = int.from_bytes(hashlib.sha256(SOURCE.encode()).digest()[:8], 'little') | (1 << 63)
    if any(int(r[0]) == stable for r in old_rows):
        raise ValueError('Stable placement ID collision')
    values = audit['runtimePosition']+audit['runtimeQuaternion']+audit['runtimeScale']+[1]
    # Product island shard consumes this explicitly promoted cinematic actor.
    line = f'{stable} "{SOURCE}" "{AREA}_SL01" "actor" "{ASSET}" '+ ' '.join(format(x,'.17g') for x in values)
    placement = [str(stable), SOURCE, AREA+'_SL01', 'actor', ASSET]+[str(x) for x in values]
    stage(placements, replace_table(dependencies[placements], lambda s: '"'+PREFIX in s, line))
    catalog_row = f'"{ASSET}" "itr_02453_sk" "{MODEL}" "Prototype_Component_Model_{ASSET}" 1 1 1 Origin "staticmesh" "itr_02453" "UE3 ImportTable exact: itr_02453.mesh.itr_02453_sk; SCENE04A actor55 component613" Opaque Back 1 1 0 0 1 1 1 50 1 1 1 1 1'
    stage(catalog, replace_table(dependencies[catalog], lambda s: False, catalog_row))
    stage(materials, append_preserving_text(dependencies[materials], added))
    world_before = json.loads(dependencies[world].decode('utf-8-sig'))
    current = copy.deepcopy(world_before)
    head, = [p for p in current['placements'] if p['placementId'] == 'npc.maharaka.source57009.actor100']
    top = top_at(mesh_triangles(model), placement, head['position'][0], head['position'][2])
    old_y = head['position'][1]
    head['position'][1] = round(top + .06260592, 8)
    # This is current idle-pose contact, not a claim to reproduce cinematic root motion.
    current['revision'] += 1
    world_text = dependencies[world].decode('utf-8-sig')
    world_text, n1 = re.subn(r'("revision"\s*:\s*)'+str(world_before['revision'])+r'\b', lambda m: m[1]+str(current['revision']), world_text, count=1)
    world_text, n2 = re.subn(r'(?<![\d.])'+re.escape(str(old_y))+r'(?![\d.])', str(head['position'][1]), world_text)
    if n1 != 1 or n2 != 1 or json.loads(world_text) != current:
        raise ValueError('Refusing broad or ambiguous World edit')
    stage(world, world_text.encode('utf-8'))
    report = dict(assetId=ASSET, sourcePlacementId=SOURCE, removedPieces=len(previous),
                  originalStandCount=1, materials=expected_names, sourceTextureMips=mip_count,
                  standTopAtHead=top, previousHeadY=old_y, currentIdleHeadY=head['position'][1],
                  npcCount=sum(p['kind']=='npc' for p in current['placements']),
                  headContactBasis='PROJECT_IDLE_POSE_ADAPTER_NOT_SOURCE_CINEMATIC_ROOT',
                  visualApproval=False, runtimePublished=False)
    for p, (before, after) in staged.items():
        relative = p.relative_to(ROOT)
        backup = args.out/'before'/relative
        candidate = args.out/'candidate'/relative
        if before is not None:
            backup.parent.mkdir(parents=True, exist_ok=True)
            if not backup.exists():
                backup.write_bytes(before)
        candidate.parent.mkdir(parents=True, exist_ok=True)
        candidate.write_bytes(after)
    (args.out/'report.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
    if args.apply:
        commit_staged_files(staged, expected=dependencies)
    print(json.dumps(report))


if __name__ == '__main__':
    main()

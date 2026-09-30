"""Installs the original far Maharaka island (EFDLProp_ISL_00072) into the Bern Area sources.

Replaces the v2/v3 shrunken-playable-island (install_bern_island.py) with the retail ocean island prop.
Run `python Tools/ShipPipeline/install_bern_island.py --remove --apply` first (removes the old shard, rows,
materials and lighting); this script refuses to run while the ISLAND00 shard exists.

Writes (dry run by default, --apply to write; compare-and-swap on the current bytes, backups first):
  Client/Bin/Resources/Map/LV_BER_BERNCASTLE/<ASSET>/<ASSET>.wmodel      (Git-ignored runtime Resources)
  Client/Bin/Resources/Map/LV_BER_BERNCASTLE/SourceMaterials/<hash>_<name>.dds
  Data/Maps/Imported/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE_ISLAND00.mapassets|.mapplacements + .mapset row
  Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.mapplacements   one editor row
  Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.mapmaterials.json  four material rows (append only)
Then publish once: Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_BER_BERNCASTLE -Mode Publish -Scope Area
"""
import argparse
import hashlib
import json
import os
import shutil
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import bern_isl72 as isl  # noqa: E402
import install_bern_island as legacy  # noqa: E402  (read_lines / atomic_write / paths)

ROOT = isl.ROOT
IMPORTED = os.path.join(ROOT, 'Data', 'Maps', 'Imported', isl.BERN_AREA)
AUTH_DIR = os.path.join(ROOT, 'Data', 'Maps', 'Authoring', isl.BERN_AREA)
AUTHORING = os.path.join(AUTH_DIR, isl.BERN_AREA + '.mapplacements')
MATERIALS = os.path.join(AUTH_DIR, isl.BERN_AREA + '.mapmaterials.json')
MAPSET = os.path.join(IMPORTED, isl.BERN_AREA + '.mapset')
BACKUP = r'C:\Users\USER\.claude\jobs\46aea322\tmp\bern_backup'

SL12 = 'Map/LV_LOBBY_CLASSSELECT_SL12/SourceMaterials/'
SL12_REFLECTION = SL12 + '8675f5bc7bb9_ambientreflection_02.dds'
SL12_DETAIL_NORMAL = SL12 + '8b35b0d8e8e3_normal.dds'

# (gltf material, slot name, family kind, diffuse texture export path, normal texture export path)
SLOTS = [
    ('isl_00037_01_mi', 'opaque', 'ISL_00037/Texture2D/isl_00037_01_d.dds', 'ISL_00037/Texture2D/isl_00037_01_n.dds'),
    ('isl_00037_02_mi', 'masked', 'ISL_00037/Texture2D/isl_00037_02_d.dds', 'ISL_00037/Texture2D/isl_00037_02_n.dds'),
    ('isl_00037_03_mi', 'opaque', 'ISL_00037/Texture2D/isl_00037_03_d.dds', 'ISL_00037/Texture2D/isl_00037_03_n.dds'),
    ('isl_00072_mi', 'pbr', 'ISL_00072/Texture2D/isl_00072_d.dds', 'ISL_00072/Texture2D/isl_00072_n.dds'),
]
ORM_EXPORT = 'EFMASTER_MATERIAL_PROLOGUE/Texture2D/orm.dds'


def sha(path):
    return hashlib.sha256(open(path, 'rb').read()).hexdigest()


def texture_target(export_relative):
    source = os.path.join(isl.UMODEL_EXPORT, export_relative)
    name = os.path.basename(export_relative)
    relative = '%s/%s_%s' % (isl.TEXTURE_DIR_REL, sha(source)[:12], name)
    return source, relative


def opaque_row(index, name, kind, diffuse, normal):
    slot = 'SLOT_%03d_%s' % (index, name)
    flags = 261 + (64 if kind == 'masked' else 0)     # simple graph (256+4) + normal map (1) [+ alpha mask 64]
    return {
        'assetId': isl.ASSET_ID, 'materialName': slot, 'sourceMaterial': 'isl_00037.mat.' + name,
        'castsShadow': True, 'renderMode': 'deferred', 'cullMode': 'back',
        'textureColorSpace': {'diffuse': 'srgb', 'normal': 'linear', 'specular': 'linear', 'reflection': 'linear'},
        'diffuseBrightness': 1.0, 'normalIntensity': 1.0, 'diffuseColor': [1.0, 1.0, 1.0, 1.0],
        'specularIntensity': 0.0, 'specularPower': 10.0, 'specularColor': [1.0, 1.0, 1.0, 1.0],
        'diffuseSaturation': 1, 'uvTiling': [1, 1], 'sourceUV': [0.0, 1.0, 0, 0],
        'diffuseTexture': diffuse, 'family': 'bg-source-opaque-masked', 'sourceFlags': flags,
        'sourceBump': [0.0, 0.0, 1.0, 0], 'flickerMode': 0, 'addressU': 'WRAP', 'normalTexture': normal,
        'reflectionContrast': 0.5, 'reflectionTiling': 1.0, 'reflectionColor': [1.0, 1.0, 1.0, 1.0],
        'reflectionIntensity': 0, 'reflectionOriginOffset': [0, 0], 'sourceSpecularSaturation': 1,
    }


def pbr_row(index, name, diffuse, normal, orm):
    return {
        'assetId': isl.ASSET_ID, 'materialName': 'SLOT_%03d_%s' % (index, name),
        'sourceMaterial': 'isl_00072.mat.' + name, 'castsShadow': True, 'renderMode': 'deferred', 'cullMode': 'back',
        'textureColorSpace': {'diffuse': 'srgb', 'normal': 'linear', 'detailNormal': 'linear', 'orm': 'srgb',
                              'reflection': 'srgb'},
        'family': 'bg_base_pbr_opa', 'diffuseBrightness': 1.0, 'normalIntensity': 1.0, 'reflectionContrast': 0.5,
        'diffuseColor': [1.0, 1.0, 1.0, 1.0], 'reflectionColor': [1.0, 1.0, 1.0, 1.0],
        # bg_base_pbr_opa parent MIC (EFMASTER_MATERIAL_PROLOGUE) overrides, ISL_00072 MIC overrides no scalar
        'metallicIntensity': 1.0, 'metallicPower': 1.0, 'roughnessIntensity': 1.0, 'roughnessPower': 1.0,
        'aoIntensity': 1.0, 'aoPower': 1.0, 'specularPBRIntensity': 0.5, 'nonmetallicBrightness': 1.0,
        'metallicBrightness': 1.0, 'minimumRoughness': 0.04, 'vertexAlpha': 1, 'reflectionIntensity': 0,
        'reflectionTiling': 1, 'diffuseSaturation': 1.0, 'detailNormalIntensity': 0, 'detailNormalTiling': 1,
        'uvFixedNormal': False, 'useWorldReflection': False, 'reflectionOriginOffset': [0, 0], 'uvTiling': [1, 1],
        'diffuseTexture': diffuse, 'normalTexture': normal, 'detailNormalTexture': SL12_DETAIL_NORMAL,
        'ormTexture': orm, 'reflectionTexture': SL12_REFLECTION,
    }


def build_plan():
    if not os.path.exists(isl.COOKED_WMODEL):
        raise SystemExit('cooked wmodel missing: ' + isl.COOKED_WMODEL)
    copies = [(isl.COOKED_WMODEL, isl.MODEL_REL)]
    rows = []
    textures = {}
    for index, (name, kind, diffuse_export, normal_export) in enumerate(SLOTS):
        diffuse_src, diffuse_rel = texture_target(diffuse_export)
        normal_src, normal_rel = texture_target(normal_export)
        textures[diffuse_rel] = diffuse_src
        textures[normal_rel] = normal_src
        if kind == 'pbr':
            orm_src, orm_rel = texture_target(ORM_EXPORT)
            textures[orm_rel] = orm_src
            rows.append(pbr_row(index, name, diffuse_rel, normal_rel, orm_rel))
        else:
            rows.append(opaque_row(index, name, kind, diffuse_rel, normal_rel))
    copies.extend((source, relative) for relative, source in textures.items())
    return copies, rows


def catalog_text():
    row = ('"%s" "Maharaka Summer Camp ocean island (ISL_00072)" "%s" "Prototype_Component_Model_%s" 1 1 1 Origin '
           '"ocean-island" "Ocean Island" "Ocean DeployData 30703 prop 1041048 -> EFDLProp_ISL_00072 StaticMesh ISL_00072_SK" '
           'Opaque Back 1 1 0 0 1 1 1 50 1 1 1 1 1') % (isl.ASSET_ID, isl.MODEL_REL, isl.ASSET_ID)
    return 'LOSTARK_MAP_ASSET_CATALOG 4 "%s" 1\n%s\n' % (isl.BERN_AREA, row)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--apply', action='store_true')
    parser.add_argument('--dock-json', default='')
    args = parser.parse_args()

    copies, rows = build_plan()
    source, placement_row = isl.island_row()
    print('asset', isl.ASSET_ID)
    print('resources to install: %d files' % len(copies))
    print('placement', placement_row)

    set_raw, set_lines = legacy.read_lines(MAPSET)
    if set_lines and set_lines[-1] == '':
        set_lines = set_lines[:-1]
    shard_prefix = '"%s" ' % isl.SHARD_ID
    if any(line.startswith(shard_prefix) for line in set_lines[1:]):
        raise SystemExit('island shard already present; run install_bern_island.py --remove --apply first')
    new_set = list(set_lines)
    new_set[0] = 'LOSTARK_MAP_SHARD_SET 1 "%s" %d' % (isl.BERN_AREA, len(set_lines))
    new_set.append('"%s" "%s" "%s" 1 0' % (isl.SHARD_ID, isl.SHARD_CATALOG, isl.SHARD_PLACEMENTS))
    new_set_text = '\n'.join(new_set) + '\n'

    auth_raw, auth_lines = legacy.read_lines(AUTHORING)
    trailing = auth_lines and auth_lines[-1] == ''
    body = auth_lines[:-1] if trailing else auth_lines
    if int(body[0].split()[-1]) != len(body) - 1:
        raise SystemExit('authoring header count mismatch')
    if any(line.split('"')[1:2] == [source] for line in body[1:]):
        raise SystemExit('island placement already present')
    kept = list(body) + [placement_row]
    kept[0] = 'LOSTARK_MAP_PLACEMENTS 2 "%s" %d' % (isl.BERN_AREA, len(kept) - 1)
    new_auth_text = '\n'.join(kept) + '\n'

    mat_raw = open(MATERIALS, 'rb').read()
    mat_doc = json.loads(mat_raw.decode('utf-8'))
    if any(m['assetId'] == isl.ASSET_ID for m in mat_doc['materials']):
        raise SystemExit('island materials already present')
    mat_doc['materials'].extend(rows)
    new_mat_bytes = (json.dumps(mat_doc, indent=2, ensure_ascii=True).replace('\n', '\r\n') + '\r\n').encode('utf-8')

    catalog_path = os.path.join(IMPORTED, isl.SHARD_CATALOG)
    placements_path = os.path.join(IMPORTED, isl.SHARD_PLACEMENTS)
    placements_text = 'LOSTARK_MAP_PLACEMENTS 2 "%s" 0\n' % isl.BERN_AREA

    if args.dock_json:
        payload = {
            'status': 'FINAL_V3_ISL72',
            'coordinateSystem': 'Bern runtime metres (same as Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json placement positions)',
            'islandOrigin': {'x': isl.ISLAND_ORIGIN[0], 'y': isl.SEA_SURFACE_Y, 'z': isl.ISLAND_ORIGIN[1]},
            'islandScale': isl.ISLAND_SCALE,
            'retailAnchor': {'x': round(isl.ANCHOR_BERN[0], 2), 'z': round(isl.ANCHOR_BERN[1], 2)},
        }
        with open(args.dock_json, 'w', encoding='utf-8') as handle:
            json.dump(payload, handle, indent=2)
        print('wrote', args.dock_json)
    if not args.apply:
        print('dry run only; pass --apply to write')
        return 0

    os.makedirs(BACKUP, exist_ok=True)
    stamp = 'isl72'
    shutil.copyfile(MAPSET, os.path.join(BACKUP, 'LV_BER_BERNCASTLE.mapset.before_' + stamp))
    shutil.copyfile(AUTHORING, os.path.join(BACKUP, 'LV_BER_BERNCASTLE.mapplacements.before_' + stamp))
    shutil.copyfile(MATERIALS, os.path.join(BACKUP, 'LV_BER_BERNCASTLE.mapmaterials.json.before_' + stamp))
    for source_path, relative in copies:
        target = os.path.join(isl.RESOURCES, *relative.split('/'))
        os.makedirs(os.path.dirname(target), exist_ok=True)
        if os.path.exists(target) and sha(target) != sha(source_path):
            raise SystemExit('refusing to overwrite a different file: ' + target)
        if not os.path.exists(target):
            shutil.copyfile(source_path, target)
    for path, text in ((catalog_path, catalog_text()), (placements_path, placements_text)):
        with open(path, 'w', encoding='utf-8', newline='\n') as handle:
            handle.write(text)
    legacy.atomic_write(MAPSET, new_set_text.encode('utf-8'), hashlib.sha256(set_raw).hexdigest())
    legacy.atomic_write(AUTHORING, new_auth_text.encode('utf-8'), hashlib.sha256(auth_raw).hexdigest())
    legacy.atomic_write(MATERIALS, new_mat_bytes, hashlib.sha256(mat_raw).hexdigest())
    print('applied')
    return 0


if __name__ == '__main__':
    sys.exit(main())

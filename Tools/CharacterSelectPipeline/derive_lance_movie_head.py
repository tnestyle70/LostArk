"""Stage the normal Lance customization head on the existing Movie costume/rig.

Only the three existing face/eye/eyelash actor resources change. The normal
geometry and catalog material inputs replace the Movie's face02 appearance;
Movie animation, actor transforms, costume and cameras remain owned by Movie.
The receipt also carries the matching Intro/Loop face material-track patches.
Outputs stay under out. Installation and publication are separate operations.
"""
from __future__ import annotations

import argparse
import copy
from dataclasses import replace
import hashlib
import json
from pathlib import Path
import struct
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/ModelAssetConverter'))
import cook_wmodel_geometry_contract as geom
import verify_dimensionmaster_summon_bind_pose as wm
from bake_guardian_selection import rebase_donor_material_paths
from split_source_movie_materials import split_material
from derive_lance_movie_hair import BASIS, derive as derive_hair

DONOR = 'Character/LanceMaster/LanceMaster.wmodel'
PARTS = (
    (3, 'face', 1, 'DefaultFace.wmodel'),
    (4, 'eyelashes', 2, 'DefaultEyelashes.wmodel'),
    (5, 'eyes', 0, 'DefaultEyes.wmodel'),
)
PREFIX = 'Character/LanceMaster/Cinematics/ClassSelect/'


def digest(data):
    return hashlib.sha256(data).hexdigest()


def build_face_material_track_patches(cinematics_path: Path, catalog_path: Path):
    """Describe the two Movie face bindings without editing either source file."""
    cinematics_bytes, catalog_bytes = cinematics_path.read_bytes(), catalog_path.read_bytes()
    cinematics = json.loads(cinematics_bytes.decode('utf-8-sig'))
    catalog = json.loads(catalog_bytes.decode('utf-8-sig'))
    characters = [row for row in catalog['characters']
                  if row.get('assetId') == 'LanceMaster']
    if len(characters) != 1:
        raise ValueError('Expected one LanceMaster donor catalog entry')
    donors = [row for row in characters[0]['modelMaterialOverrides']
              if row.get('modelAssetId') == DONOR and row.get('materialName') == 'pc_ft_face_mi']
    if len(donors) != 1 or donors[0].get('family') != 'source.character.classic-head.v1':
        raise ValueError('Expected one normal Lance face material contract')
    donor = donors[0]
    scenes = [row for row in cinematics['scenes'] if row.get('classId') == 'LANCE_MASTER']
    if len(scenes) != 1:
        raise ValueError('Expected one Lance Movie scene')
    fields = ('materialName', 'family', 'parameters')
    after = {name: copy.deepcopy(donor[name]) for name in fields}
    patches = []
    for phase in ('intro', 'loop'):
        instance_id = f'world.sequence.instance.classselect.lancemaster.{phase}.a12205.p1'
        tracks = [row for row in scenes[0][phase]['materialTracks']
                  if row.get('instanceId') == instance_id and row.get('slotId') == 'actor']
        if len(tracks) != 1:
            raise ValueError(f'Expected one Lance Movie face material track: {phase}')
        track = tracks[0]
        identity = (track.get('materialName'), track.get('family'))
        if identity not in (('pc_ft_face_mi_high', 'source.character.equipment-native-200.v1'),
                            (after['materialName'], after['family'])):
            raise ValueError(f'Unexpected Lance Movie face material contract: {phase}')
        curves = track['curves']
        if any(curve['parameter'] not in after['parameters'] for curve in curves):
            raise ValueError(f'Lance Movie face curve is unsupported by the donor: {phase}')
        before = {name: copy.deepcopy(track[name]) for name in fields}
        patches.append(dict(classId='LANCE_MASTER', phase=phase, instanceId=instance_id,
            slotId='actor', before=before, after=copy.deepcopy(after), changed=before != after,
            curvesSha256=digest(json.dumps(curves, sort_keys=True, separators=(',', ':')).encode('utf-8'))))
    return dict(cinematicsSourcePath=str(cinematics_path.resolve()),
        cinematicsSourceSha256=digest(cinematics_bytes),
        catalogSourcePath=str(catalog_path.resolve()), catalogSourceSha256=digest(catalog_bytes),
        patches=patches)


def derive_part(resources: Path, output: Path, slot: int, movie_slot: int):
    donor_path = resources / DONOR
    movie_id = PREFIX + f'Original/Parts/fighter_face.slot{movie_slot}.wmodel'
    movie_path = resources / movie_id
    source_bytes, movie_bytes = donor_path.read_bytes(), movie_path.read_bytes()
    donor = wm.read_wmodel(donor_path, animation_names=())
    movie = wm.read_wmodel(movie_path, animation_names=())
    subset, subset_receipt = split_material(
        rebase_donor_material_paths(source_bytes, donor_path, resources), slot)
    dp = geom.parse_skinned_uv_wmodel(subset)
    if dp['meshHeader'][4] == geom.STRIDE_SKINNED:
        subset, _ = geom.cook_skinned_basis_uv_contract(subset, {},
            {i: [1.] * row[1] for i, row in enumerate(dp['submeshes'])})
        dp = geom.parse_skinned_uv_wmodel(subset)
    mp = geom.parse_skinned_uv_wmodel(movie_bytes)
    movie_indices = {bone.name: i for i, bone in enumerate(movie.skeleton_bones)}
    mesh = bytearray(dp['mesh'])
    header = list(dp['meshHeader'])
    weighted_names = set()
    for row in dp['submeshes']:
        for i in range(row[1]):
            at = dp['vertexStart'] + row[0] + i * header[4]
            for offset in (0, 12, 32):
                x, y, z = struct.unpack_from('<3f', mesh, at + offset)
                struct.pack_into('<3f', mesh, at + offset, x, -z, -y)
            sign = struct.unpack_from('<f', mesh, at + 76)[0]
            struct.pack_into('<f', mesh, at + 76, -sign)
            indices = struct.unpack_from('<4I', mesh, at + 44)
            weights = struct.unpack_from('<4f', mesh, at + 60)
            mapped = []
            for index, weight in zip(indices, weights):
                if weight > 0:
                    name = donor.skeleton_bones[index].name
                    assert name in movie_indices, ('Weighted bone missing from Movie', name)
                    weighted_names.add(name)
                    mapped.append(movie_indices[name])
                else:
                    mapped.append(0)
            struct.pack_into('<4I', mesh, at + 44, *mapped)
            assert struct.unpack_from('<4f', mesh, at + 60) == weights
    code = '<H' if header[7] == 2 else '<I'
    for row in dp['submeshes']:
        for i in range(0, row[3], 3):
            first = dp['indexStart'] + row[2] + i * header[7]
            last = first + 2 * header[7]
            a, b = struct.unpack_from(code, mesh, first)[0], struct.unpack_from(code, mesh, last)[0]
            struct.pack_into(code, mesh, first, b)
            struct.pack_into(code, mesh, last, a)
    old_bones = dp['indexStart'] + header[6] * header[7]
    old_bounds = old_bones + header[2] * wm.MESH_BONE.size
    old_tail = old_bounds + (header[1] * geom.BOUNDS_V1.size if header[8] else 0)
    movie_bones = mp['indexStart'] + mp['meshHeader'][6] * mp['meshHeader'][7]
    bone_payload = bytearray(mp['mesh'][movie_bones:movie_bones + mp['meshHeader'][2] * wm.MESH_BONE.size])
    donor_bones = {bone.name: bone for bone in donor.mesh_bones}
    # Movie's original face does not weight every normal-head facial bone; its
    # unused inverse binds are not valid offsets for the newly weighted mesh.
    # Preserve the Movie skeleton/clip, but carry the donor mesh bind by name.
    for name in weighted_names:
        inverse_bind = BASIS @ np.array(donor_bones[name].transform).reshape(4, 4) @ BASIS
        struct.pack_into('<16f', bone_payload,
            movie_indices[name] * wm.MESH_BONE.size + 44, *inverse_bind.ravel())
    bounds = bytearray()
    if header[8]:
        for row in dp['submeshes']:
            positions = np.array([struct.unpack_from('<3f', mesh,
                dp['vertexStart'] + row[0] + i * header[4]) for i in range(row[1])])
            low, high = positions.min(axis=0), positions.max(axis=0)
            center = (low + high) * .5
            radius = np.linalg.norm(positions - center, axis=1).max()
            bounds.extend(geom.BOUNDS_V1.pack(*low, *high, *center, radius))
    mesh = mesh[:old_bones] + bone_payload + bounds + mesh[old_tail:]
    header[2] = mp['meshHeader'][2]
    wm.MESH_HEADER.pack_into(mesh, 16, *header)
    file_header = list(wm.FILE_HEADER.unpack_from(mesh))
    file_header[-1] = len(mesh) - wm.FILE_HEADER.size
    wm.FILE_HEADER.pack_into(mesh, 0, *file_header)
    material = next(s.payload for s in dp['sections'] if s.type_id == 2)
    sections = [replace(s, payload=bytes(mesh) if s.type_id == 1 else material if s.type_id == 2 else s.payload)
                for s in mp['sections']]
    result = geom.rebuild_wmodel(mp['modelHeader'], sections, bytes(mesh))
    checked = geom.parse_skinned_uv_wmodel(result)
    assert checked['uvRows'] == dp['uvRows']
    assert all(old.payload == new.payload for old, new in zip(mp['sections'], checked['sections'])
               if old.type_id not in (1, 2))
    output.parent.mkdir(parents=True, exist_ok=True)
    assert not output.exists(), 'Use a fresh output directory'
    output.write_bytes(result)
    decoded = wm.read_wmodel(output, animation_names=())
    combined = np.array(wm.combined_transforms(decoded.skeleton_bones,
        [bone.transform for bone in decoded.skeleton_bones])).reshape(-1, 4, 4)
    inverse = np.array([bone.transform for bone in decoded.mesh_bones]).reshape(-1, 4, 4)
    positions = np.array([(*vertex.position, 1.) for vertex in decoded.vertices])
    indices = np.array([vertex.indices for vertex in decoded.vertices])
    weights = np.array([vertex.weights for vertex in decoded.vertices])
    weights /= weights.sum(axis=1)[:, None]
    neutral = np.einsum('vi,vkij,vk->vj', positions, (inverse @ combined)[indices], weights)
    neutral_error = float(np.linalg.norm(neutral[:, :3] - positions[:, :3], axis=1).max())
    assert np.isfinite(neutral).all() and neutral_error < .01, (
        'Normal head does not retain its neutral geometry on the Movie rig', neutral_error)
    return dict(candidatePath=str(output), sha256=digest(result), bytes=len(result),
        donorAssetId=DONOR, donorSha256=digest(source_bytes), sourceMaterialSlot=slot,
        sourceSubmeshes=subset_receipt['originalSubmeshIndices'],
        movieAssetId=movie_id, movieSha256=digest(movie_bytes),
        vertexCount=len(decoded.vertices), weightedBoneCount=len(weighted_names), missingWeightedBones=[],
        skeletonBones=len(decoded.skeleton_bones), movieSkeletonAndClipSectionsByteIdentical=True,
        weightedDonorInverseBindsTransformedByName=True,
        neutralPoseMaxErrorCm=neutral_error,
        donorUVChannelsPreserved=True, tangentBasisReflectionPreserved=True,
        clips=[dict(name=a.name, durationTicks=a.duration_ticks, ticksPerSecond=a.ticks_per_second)
               for a in decoded.animations])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--resources', type=Path, default=ROOT / 'Client/Bin/Resources')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    output, resources = args.output.resolve(), args.resources.resolve()
    assert output.is_relative_to((ROOT / 'out').resolve()), 'Candidates must remain under out'
    assert not output.exists(), 'Use a fresh output directory'
    receipt = dict(intent='User-selected normal customization head with existing Movie costume',
        installed=False, actorTransformsCameraClockCostumeUnchanged=True, resources=[], objectPatches=[])
    receipt['cinematicMaterialTrackPatches'] = build_face_material_track_patches(
        ROOT / 'Data/Camera/ClassSelection.cinematics.json', ROOT / 'Data/Actors/CharacterCatalog.json')
    for material, name, movie_slot, filename in PARTS:
        asset = PREFIX + 'Appearance/' + filename
        detail = derive_part(resources, output / 'Resources' / asset, material, movie_slot)
        detail.update(part=name, targetAssetId=asset)
        receipt['resources'].append(detail)
        receipt['objectPatches'].append(dict(objectId=f'world.object.classselect.lancemaster.a12205.p{movie_slot}',
            setFields=dict(modelAssetId=asset, materialSourceModelAssetId=DONOR), removeFields=['materialProfile']))
    hair = derive_hair(resources, output / 'Resources' / (PREFIX + 'Appearance/FT43_Hair.wmodel'))
    receipt['resources'].append(hair)
    output.joinpath('head-replacement-receipt.json').write_text(json.dumps(receipt, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(receipt, indent=2))


if __name__ == '__main__':
    main()

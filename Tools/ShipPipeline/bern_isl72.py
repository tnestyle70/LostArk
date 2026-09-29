"""The original far Maharaka island (EFDLProp_ISL_00072) placed in the Bern sea.

Evidence chain (2026-09-29, installed retail client, EFTable/DeployData/UPK read-only):
  EFTable_ZoneFallback 57009/57017/57025/57037 (Maharaka Paradise 2021/22/23, Maharaka Returns base camp)
    all point at the ocean map 30703 position (-36086, 50366).
  leveldata1.lpk MapData/30703/DeployData.loa holds a CEFDeployActor_Prop there:
    prop 1041048 -> EFTable_Prop Model EFDLProp_ISL_00072 (Scale 100) at UE (-36070, 50710) yaw 2496 (13.7 deg)
    prop 1040158 -> EFDLProp_ITR_10066 at UE (-36565, 50542)  (the anchor / entry point)
  data4.lpk LookInfo/Prop/EFDLProp_ISL_00072 -> StaticMesh ISL_00072.Mesh.ISL_00072_SK (package ISL_00072).
  The mesh atlas carries the MAHARAKA lettering, pool, slides and the wrecked ship of the retail screenshots.

Everything the sea navigation builder and the installer share lives here so the drawn island, the
navigation keep-out and the dock cannot drift apart.
"""
import hashlib
import math
import os
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from bern_terrain import MODEL_SCALE, read_static_mesh  # noqa: E402

ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'Tools', 'LevelPlacementExtractor'))
from build_maptool_scene import convert_rotation  # noqa: E402
from placement_transform import quaternion_matrix  # noqa: E402

BERN_AREA = 'LV_BER_BERNCASTLE'
SHARD_ID = 'ISLAND00'
SHARD_CATALOG = BERN_AREA + '_' + SHARD_ID + '.mapassets'
SHARD_PLACEMENTS = BERN_AREA + '_' + SHARD_ID + '.mapplacements'
PLACEMENT_LEVEL = BERN_AREA + '_ISLAND'
RESOURCES = os.path.join(ROOT, 'Client', 'Bin', 'Resources')

ASSET_ID = 'MAP_' + hashlib.sha256(b'EFDLProp_ISL_00072.ISL_00072_SK').hexdigest()[:12].upper() + '_ISL_00072_SK'
MODEL_REL = 'Map/%s/%s/%s.wmodel' % (BERN_AREA, ASSET_ID, ASSET_ID)
TEXTURE_DIR_REL = 'Map/%s/SourceMaterials' % BERN_AREA
WORK = r'C:\LostArkExtract\SeaIslandISL72_20260929'
COOKED_WMODEL = os.path.join(WORK, 'ISL_00072.wmodel')
UMODEL_EXPORT = r'C:\Users\USER\.claude\jobs\46aea322\tmp\sea_island\isl72'

SEA_SURFACE_Y = 10.8

# Retail deploy data (UE centimetres -> metres). Prop origin and anchor prop.
RETAIL_PROP_ORIGIN_UE_M = (-360.70, 507.10)
RETAIL_ANCHOR_UE_M = (-365.65, 505.42)
RETAIL_YAW_UNITS = 2496            # FRotator units, 65536 = 360 degrees
# Scale: the retail prop is 100 % (mesh 11.6 x 12.7 m, sand hull 8.5 x 6.7 m). The same retail screenshots
# show the island 3.2 - 4.0 ship lengths wide next to the ship drawn at 4.43 m in this project; hull
# extent gives x1.9 - x3.2 and hull area x2.5, so 2.0 is used (measurement error about +-25 %).
ISLAND_SCALE = 2.0
ISLAND_ORIGIN = (440.0, -480.0)    # Bern x, z of the mesh origin

# Anchor offset from the prop origin in UE metres, converted to Bern axes (x, z = -y) and scaled.
ANCHOR_OFFSET_BERN = ((RETAIL_ANCHOR_UE_M[0] - RETAIL_PROP_ORIGIN_UE_M[0]) * ISLAND_SCALE,
                      -(RETAIL_ANCHOR_UE_M[1] - RETAIL_PROP_ORIGIN_UE_M[1]) * ISLAND_SCALE)
ANCHOR_BERN = (ISLAND_ORIGIN[0] + ANCHOR_OFFSET_BERN[0], ISLAND_ORIGIN[1] + ANCHOR_OFFSET_BERN[1])


def quaternion():
    return convert_rotation({'pitch': 0, 'yaw': RETAIL_YAW_UNITS, 'roll': 0})


def placement_position():
    return (ISLAND_ORIGIN[0], SEA_SURFACE_Y, ISLAND_ORIGIN[1])


def placement_scale():
    return (ISLAND_SCALE, ISLAND_SCALE, ISLAND_SCALE)


def placement_id(source):
    digest = hashlib.sha256(source.encode('utf-8')).digest()
    return int.from_bytes(digest[:8], 'little') & 0x7FFFFFFFFFFFFFFF or 1


def format_row(source, asset, position, quat, scale):
    parts = [str(placement_id(source)), '"%s"' % source, '"%s"' % PLACEMENT_LEVEL, '"editor"', '"%s"' % asset]
    parts += [repr(round(v, 5)) for v in position]
    parts += [repr(round(v, 9)) for v in quat]
    parts += [repr(round(v, 6)) for v in scale]
    parts.append('1')
    return ' '.join(parts)


def island_row():
    source = PLACEMENT_LEVEL + ':isl72'
    return source, format_row(source, ASSET_ID, placement_position(), quaternion(), placement_scale())


def world_triangles(wmodel_path=None):
    """Island triangles (M,3,3) in Bern world metres: T * R * S applied to the cooked mesh."""
    vertices, triangles = read_static_mesh(wmodel_path or COOKED_WMODEL)
    local = vertices.astype(np.float64) * MODEL_SCALE
    matrix = np.eye(4)
    matrix[:3, :3] = quaternion_matrix(quaternion())[:3, :3] * np.array(placement_scale())[np.newaxis, :]
    matrix[:3, 3] = placement_position()
    homogeneous = np.concatenate([local, np.ones((len(local), 1))], axis=1)
    world = (matrix @ homogeneous.T).T[:, :3]
    return world[triangles]

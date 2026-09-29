"""Cook the Square Hole song instruments (retail CommonAction 60001 "Song of Return").

The retail action equips one EFDLItem_IT_<CLASS>_<NAME>_00 item per class family while the
act_music_loop_1 clip plays. Each item is a single-bone static SkeletalMesh in its own package.
This script exports that package's glTF with UModel and cooks it to a static .wmodel the same
way the WP_* weapon models were made (--pretransform, cm units), beside its textures:

    Client/Bin/Resources/Character/<PACKAGE>/<PACKAGE>.wmodel
    Client/Bin/Resources/Character/<PACKAGE>/textures/*.dds

Attachment is not in the item: the retail socket is `sc_prop3_01` on the class body mesh, which
names the bone `bip001-prop3` and carries no offset (read from the body package's
skeletalmeshsocket exports). Data/Actors/SquareHoleInstruments.json records that bone.

Converter and UModel both misread non-ASCII absolute paths, so everything is cooked in an ASCII
staging folder and copied into the repository afterwards.

    python Tools/CharacterEquipmentPipeline/cook_squarehole_instruments.py [--staging D:/some/ascii/dir]
"""
import argparse
import math
import shutil
import struct
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
CONVERTER = REPO / "Tools" / "ModelAssetConverter" / "Bin" / "ModelAssetConverter.exe"
UMODEL = Path("C:/Users/엄태준/OneDrive/Desktop/UModel/umodel_lostark_v7.exe")
PACKAGES = "D:/Games/LOSTARK/EFGame/ReleasePC/Packages"
DEFAULT_STAGING = Path("D:/ClaudeWork/Work/InstrumentResearch/cook")

# package (friendly name), material, then which extra texture the retail material carries.
INSTRUMENTS = [
    # war horn: Warlord / Slayer / Destroyer (WR family)
    ("IT_WR_HORN_00", "it_wr_horn_00_mi", "s"),
    # pipa: LanceMaster (FT family)
    ("IT_FT_PIPA_00", "it_ft_pipa_00_mi", "orm"),
    # harp: Artist (SP family)
    ("IT_SP_HARP_00", "it_sp_harp_00_mi", "s"),
    # DK family instrument (Guardian Knight); the retail item is named CART
    ("IT_DK_CART_00", "it_dk_cart_00_mi", "orm"),
]


def run(command, cwd=None):
    result = subprocess.run([str(part) for part in command], capture_output=True, text=True, cwd=cwd)
    if result.returncode != 0:
        sys.exit(f"command failed ({result.returncode}): {' '.join(str(p) for p in command)}\n"
                 f"{result.stdout[-1500:]}\n{result.stderr[-1500:]}")
    return result.stdout


FILE_HEADER = struct.Struct("<4sHHII")
MESH_HEADER = struct.Struct("<4sIIIIIIIB3s")
SUBMESH_DESC = struct.Struct("<IIIIIQ20s")
BOUNDS_V1 = struct.Struct("<10f")
VERTEX = struct.Struct("<3f3f2f3ff")


def to_legacy_basis(wmodel: Path):
    """Put a static v1.0 WMSH in the basis the installed class bodies and WP_* weapons use.

    The converter keeps UModel's glTF axes (x, z, y of the UE mesh). The installed bodies, their
    animations and weapons are the UE space with z negated. From the glTF axes that is
    (x, y, z) -> (x, z, -y), a pure rotation, so neither the triangle winding nor the tangent
    handedness changes. An earlier version used (x, -z, -y) here, which was fitted on a weapon
    that is mirror-symmetric in y: it agreed with the weapons but mirrored every instrument
    (the harp's faces swapped and the pipa was reversed left to right). A part that rides a body
    bone has to be in the bone's basis, so positions, normals and tangents are mapped. Sizes do
    not change, so the file is patched in place.
    """
    data = bytearray(wmodel.read_bytes())
    wint = bytes(data).index(b"WMSH") - FILE_HEADER.size
    _, _, minor, _, _ = FILE_HEADER.unpack_from(data, wint)
    mesh = wint + FILE_HEADER.size
    (_, submeshes, bone_count, _, stride, total_vertices, total_indices, index_stride, has_bounds,
     _) = MESH_HEADER.unpack_from(data, mesh)
    if minor != 0 or bone_count != 0 or stride != VERTEX.size or index_stride not in (2, 4):
        sys.exit(f"{wmodel} is not a static v1.0 mesh")
    table = mesh + MESH_HEADER.size
    descriptors = [SUBMESH_DESC.unpack_from(data, table + row * SUBMESH_DESC.size)
                   for row in range(submeshes)]
    vertex_base = table + submeshes * SUBMESH_DESC.size
    index_base = vertex_base + total_vertices * stride
    bounds_base = index_base + total_indices * index_stride

    for row in range(total_vertices):
        at = vertex_base + row * stride
        px, py, pz, nx, ny, nz, u, v, tx, ty, tz, w = VERTEX.unpack_from(data, at)
        VERTEX.pack_into(data, at, px, pz, -py, nx, nz, -ny, u, v, tx, tz, -ty, w)

    for row, (vertex_offset, vertex_count, index_offset, index_count, *_rest) in enumerate(descriptors):
        if has_bounds:
            points = [VERTEX.unpack_from(data, vertex_base + vertex_offset + k * stride)[:3]
                      for k in range(vertex_count)]
            low = [min(p[axis] for p in points) for axis in range(3)]
            high = [max(p[axis] for p in points) for axis in range(3)]
            centre = [(low[axis] + high[axis]) * 0.5 for axis in range(3)]
            radius = max(math.dist(p, centre) for p in points)
            BOUNDS_V1.pack_into(data, bounds_base + row * BOUNDS_V1.size, *low, *high, *centre, radius)
    wmodel.write_bytes(bytes(data))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--staging", type=Path, default=DEFAULT_STAGING)
    args = parser.parse_args()
    staging = args.staging
    if not str(staging).isascii():
        sys.exit("staging folder must be an ASCII path")
    for tool in (CONVERTER, UMODEL):
        if not tool.exists():
            sys.exit(f"missing tool: {tool}")

    for package, material, extra in INSTRUMENTS:
        export = staging / "export"
        export.mkdir(parents=True, exist_ok=True)
        # UModel resolves its game profile relative to its own folder.
        run([UMODEL, "-export", "-gltf", "-game=lostark", "-kr", f"-path={PACKAGES}",
             f"-out={export.as_posix()}", f"{package}.upk", "-3rdparty"], cwd=UMODEL.parent)
        stem = package.lower()
        gltf = export / package / "mesh" / f"{stem}_sk.gltf"
        tex = export / package / "tex"
        if not gltf.exists():
            sys.exit(f"UModel did not write {gltf}")

        out_dir = staging / "cooked" / package
        out_dir.mkdir(parents=True, exist_ok=True)
        wmodel = out_dir / f"{package}.wmodel"
        command = [CONVERTER, gltf, "-o", wmodel, "--pretransform", "--no-auto-textures",
                   "--scale", "100",
                   "--material-remap", f"{material}={tex / (stem + '_d.dds')}",
                   "--normal-remap", f"{material}={tex / (stem + '_n.dds')}"]
        if extra == "orm":
            command += ["--orm-remap", f"{material}={tex / (stem + '_orm.dds')}"]
        else:
            command += ["--specular-remap", f"{material}={tex / (stem + '_s.dds')}"]
        print(run(command).strip())
        to_legacy_basis(wmodel)

        target = REPO / "Client" / "Bin" / "Resources" / "Character" / package
        if target.exists():
            shutil.rmtree(target)
        shutil.copytree(out_dir, target)
        print(f"installed {target.relative_to(REPO)}")


if __name__ == "__main__":
    main()

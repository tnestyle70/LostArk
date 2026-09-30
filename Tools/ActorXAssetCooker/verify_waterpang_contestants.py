"""Read-only audit of the installed Waterpang roster, weapon and hand attachment.

Uses the same WModel skeleton/animation reader as existing asset diagnostics.
Does not run Client, change Resources, or claim a visual-fidelity result.
"""
from pathlib import Path
import argparse
import hashlib
import json
import math
import re
import struct
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/ModelAssetConverter"))
from verify_dimensionmaster_summon_bind_pose import (read_wmodel, combined_transforms,
    affine_matrix, sample_vector, sample_quaternion, matrix_multiply, matrix_inverse,
    FILE_HEADER, MODEL_HEADER, SECTION_DESC, MESH_HEADER, SUBMESH_DESC)


def audit(resources):
    read = lambda path: json.loads((ROOT / path).read_text(encoding="utf-8-sig"))
    contract = (ROOT / "Shared/Public/Gameplay/MaharakaWaterpangContract.h").read_text()
    roster_text = contract.split("MAHARAKA_WATERPANG_AI_NPCS", 1)[1].split(";", 1)[0]
    ids = re.findall(r'"(NPC_[^"]+)"', roster_text)
    assert len(ids) == len(set(ids)) == 8
    catalog = {x["archetypeId"]: x for x in read("Data/Actors/NpcCatalog.json")["npcs"]}
    rows = []
    for identity in ids:
        actor = catalog[identity]
        path = resources / actor["modelAssetId"]
        model = read_wmodel(path, include_geometry=False, animation_names=())
        names = {bone.name: i for i, bone in enumerate(model.skeleton_bones)}
        assert "bip001-r-hand" in names
        scale = .0001 if "RootNode" in names else .01
        pose = combined_transforms(model.skeleton_bones, [bone.transform for bone in model.skeleton_bones])
        hand = pose[names["bip001-r-hand"]]
        basis = [math.sqrt(sum(hand[axis * 4 + j] ** 2 for j in range(3))) * scale for axis in range(3)]
        assert all(abs(value - .01) < .00001 for value in basis), (identity, basis)
        clip_source = model
        if actor.get("animationSetId"):
            clip_source = read_wmodel(resources / actor["animationSetId"], include_geometry=False, animation_names=())
        assert actor["idleClip"] in {clip.name for clip in clip_source.animations}
        rows.append(dict(archetype=identity, model=actor["modelAssetId"], handBasis=basis,
            sourceSha256=hashlib.sha256(path.read_bytes()).hexdigest()))

    items = {x["itemId"]: x for x in read("Data/Items/ItemCatalog.json")["items"]}
    sets = {x["visualSetId"]: x for x in read("Data/Actors/EquipmentPresentationCatalog.json")["visualSets"]}
    equipment = set()
    for slot in range(12):
        prefix = "AVATAR_LANCEMASTER_MOKOKO_036" if slot % 2 else "AVATAR_GUARDIANKNIGHT_MOKOKO_036"
        if slot // 2:
            prefix += "-" + str(slot // 2)
        for suffix in ("_HEAD", "_OUTFIT"):
            visual = sets[items[prefix + suffix]["visualSetId"]]
            for part in visual["parts"]:
                assert (resources / part["modelAssetId"]).is_file()
                equipment.add(part["modelAssetId"])

    # Reconstruct the measured adapter from actual source animation at time zero.
    rig = read_wmodel(resources / "Character/GuardianKnight/AnimSets/GuardianKnight_WaterGunAnimSet.wmodel",
        include_geometry=False, animation_names=("watergun_idle",))
    clip = next(clip for clip in rig.animations if clip.name == "watergun_idle")
    local = [bone.transform for bone in rig.skeleton_bones]
    for channel in clip.channels:
        local[channel.bone_index] = affine_matrix(sample_vector(channel.scale_keys, 0, (1, 1, 1)),
            sample_quaternion(channel.rotation_keys, 0), sample_vector(channel.position_keys, 0, (0, 0, 0)))
    pose = dict(zip((bone.name for bone in rig.skeleton_bones), combined_transforms(rig.skeleton_bones, local)))
    adapter = matrix_multiply(pose["bip001-prop3"], matrix_inverse(pose["bip001-r-hand"]))
    offset = [component * .01 for component in adapter[12:15]]
    angles = [math.degrees(math.asin(-adapter[9])), math.degrees(math.atan2(adapter[8], adapter[10])),
        math.degrees(math.atan2(adapter[1], adapter[5]))]
    npc_source = (ROOT / "Client/Private/Npc.cpp").read_text(encoding="utf-8-sig")
    values = re.search(r"part->Set_SocketTransform\(\{([^}]+)\},\s*\{([^}]+)\}", npc_source)
    installed = [[float(value.strip().removesuffix("f")) for value in group.split(",")] for group in values.groups()]
    assert max(abs(a - b) for a, b in zip(installed[0], offset)) < .000001
    assert max(abs(a - b) for a, b in zip(installed[1], angles)) < .00001
    for name in ("LanceMaster", "GuardianKnight"):
        model = read_wmodel(resources / f"Character/{name}/AnimSets/{name}_WaterGunAnimSet.wmodel",
            include_geometry=False, animation_names=())
        assert {"watergun_idle", "watergun_run"}.issubset({clip.name for clip in model.animations})

    gun = resources / "Character/Maharaka/WaterGun/ITR_02164/ITR_02164.wmodel"
    data = gun.read_bytes()
    header = MODEL_HEADER.unpack_from(data, FILE_HEADER.size)
    sections = [SECTION_DESC.unpack_from(data, FILE_HEADER.size + MODEL_HEADER.size + i * SECTION_DESC.size)
        for i in range(header[1])]
    section = next(row for row in sections if row[0] == 1)
    cursor = section[2] + 2 * FILE_HEADER.size
    mesh = MESH_HEADER.unpack_from(data, cursor)
    assert mesh[0] == b"WMSH" and mesh[1] == 2 and mesh[2] == 0
    cursor += MESH_HEADER.size + mesh[1] * SUBMESH_DESC.size
    vertices = [struct.unpack_from("<3f", data, cursor + i * mesh[4]) for i in range(mesh[5])]
    dimensions = [(max(row[axis] for row in vertices) - min(row[axis] for row in vertices)) * .01 for axis in range(3)]
    assert .6 < max(dimensions) < 1.0
    return dict(npcRigs=rows, avatarContestants=12, avatarModelFiles=len(equipment),
        gunVertices=len(vertices), gunMetreDimensions=dimensions, gripOffsetMetres=offset,
        gripPitchYawRollDegrees=angles, passAll=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--resources", type=Path, default=ROOT / "Client/Bin/Resources")
    parser.add_argument("--out", type=Path, default=ROOT / "out/WaterpangPreloadReview/result.json")
    args = parser.parse_args()
    result = audit(args.resources)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({key: value for key, value in result.items() if key != "npcRigs"}))


if __name__ == "__main__":
    main()

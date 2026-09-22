"""Bake source Matinee slots and facial controls into ordinary candidate WANMs."""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import struct
from pathlib import Path
import sys

import numpy as np
from scipy.spatial.transform import Rotation

from project_guardian_selection import ROOT, read, rows_from, write, phase_info, base

sys.path.insert(0, str(ROOT / "Tools/ValtanPipeline"))
sys.path.insert(0, str(ROOT / "Tools/KoukuSaydonPipeline"))
sys.path.insert(0, str(ROOT / "Tools/ModelAssetConverter"))
from bake_valtan_original_cinematic_actors import animation_payload, sections
from bake_character_cinematic_clips import append_idempotent, validate_keys
from build_bingo_ending_actors import blend, strength_at
import patch_wmodel_dye as material_format

BODY = ROOT / "Client/Bin/Resources/Character/GuardianKnight/GuardianKnight.wmodel"
LOCAL_BASIS = np.diag([1., 1., -1.])
SPECS = (("intro", 702, 2563), ("loop", 701, 2526))


def compact_animation_payload(data):
    """Remove only byte-identical constant-run interiors, preserving every pose.

    Runtime WANM admission caps a clip at 1,000,000 TRS keys. Dense 60 Hz
    Guardian clips exceed it even though most scale/translation channels and
    the stopped-time hold are constant. Keep both endpoints of each run so
    linear position/scale and quaternion interpolation retain the same curve.
    """
    wm = base.wm
    file_header = list(wm.FILE_HEADER.unpack_from(data))
    metadata = list(wm.ANIMATION_HEADER.unpack_from(data, wm.FILE_HEADER.size))
    assert metadata[0] == b"WANM" and metadata[5] == 0, "Expected an event-free baked clip"
    channel_start = wm.FILE_HEADER.size + wm.ANIMATION_HEADER.size
    key_start = channel_start + metadata[1] * wm.ANIMATION_CHANNEL.size
    channels, keys = bytearray(), bytearray()
    original_count = kept_count = 0
    for index in range(metadata[1]):
        channel = list(wm.ANIMATION_CHANNEL.unpack_from(data, channel_start + index * wm.ANIMATION_CHANNEL.size))
        for count_field, offset_field, layout in ((1, 2, wm.VECTOR_KEY), (3, 4, wm.QUATERNION_KEY), (5, 6, wm.VECTOR_KEY)):
            count, offset = channel[count_field], key_start + channel[offset_field]
            values = [data[offset + i * layout.size:offset + (i + 1) * layout.size] for i in range(count)]
            assert all(len(value) == layout.size for value in values)
            keep = [i for i in range(count) if i == 0 or i == count - 1 or
                    values[i - 1][4:] != values[i][4:] or values[i][4:] != values[i + 1][4:]]
            channel[count_field], channel[offset_field] = len(keep), len(keys)
            keys.extend(b"".join(values[i] for i in keep))
            original_count += count
            kept_count += len(keep)
        channels.extend(wm.ANIMATION_CHANNEL.pack(*channel))
    assert original_count == metadata[4], "Invalid source key count"
    assert kept_count <= 1_000_000, f"Baked clip still exceeds runtime WANM limit: {kept_count}"
    metadata[4] = kept_count
    payload = wm.ANIMATION_HEADER.pack(*metadata) + channels + keys + data[-8:]
    file_header[-1] = len(payload)
    return wm.FILE_HEADER.pack(*file_header) + payload


def rebase_donor_material_paths(data, source_path):
    """Keep installed texture identity when copying a donor into ClassSelect."""
    resources = (ROOT / "Client/Bin/Resources").resolve()
    source_path = source_path.resolve()
    if not source_path.is_relative_to(resources):
        return data  # Candidate actor materials already use Resources asset IDs.
    output = bytearray(data)
    header = base.wm.MODEL_HEADER.unpack_from(data, base.wm.FILE_HEADER.size)
    for index in range(header[1]):
        kind, _, offset, size, _ = base.wm.SECTION_DESC.unpack_from(data,
            base.wm.FILE_HEADER.size + base.wm.MODEL_HEADER.size + index * base.wm.SECTION_DESC.size)
        if kind != material_format.SECTION_MATERIAL:
            continue
        start = base.wm.FILE_HEADER.size + offset
        blob = data[start:start + size]
        _, magic, entries = material_format.parse_material_section(blob)
        layout = material_format.ENTRY_V3 if magic == b"WMA3" else material_format.ENTRY_V2
        payload = bytearray(material_format.MATERIAL_META.pack(magic, len(entries)))
        for fields in entries:
            fields = list(fields)
            for field in range(3, 13 if magic == b"WMA3" else 12):
                stored = material_format.wstr(fields[field])
                if not stored:
                    continue
                texture = (source_path.parent / stored).resolve()
                if not texture.is_file():
                    texture = (resources / stored).resolve()
                assert texture.is_relative_to(resources) and texture.is_file(), f"Missing donor texture: {stored}"
                fields[field] = material_format.wbytes(texture.relative_to(resources).as_posix(), 260)
            payload.extend(layout.pack(*fields))
        replacement = blob[:material_format.FILE_HEADER.size] + payload
        assert len(replacement) == size
        output[start:start + size] = replacement
    return bytes(output)


def controls_for(model, tree_rows, tracks):
    names = {t["p"]["skelcontrolname"] for t in tracks if t["cls"] == "interptrackskelcontrolstrength"}
    tree = next(r for r in tree_rows.values() if r["cls"] == "animtree")
    indices = {b.name.casefold(): i for i, b in enumerate(model.skeleton_bones)}
    controls = []
    for chain in tree["p"]["skelcontrollists"]:
        index = indices.get(chain["bonename"].casefold())
        ref, seen = chain["controlhead"], set()
        while ref:
            assert ref not in seen, "Cyclic native control chain"
            seen.add(ref)
            row = tree_rows[ref]; p = row["p"]; ref = p.get("nextcontrol", 0)
            if index is None or p.get("controlname") not in names:
                continue
            assert row["cls"] == "skelcontrolsinglebone"
            assert p.get("bonerotationspace", "bcs_bonespace") in ("bcs_parentbonespace", "bcs_bonespace")
            assert p.get("bonetranslationspace", "bcs_bonespace") in ("bcs_parentbonespace", "bcs_bonespace", "bcs_otherbonespace")
            if p.get("bonetranslationspace") == "bcs_otherbonespace":
                # Scene_D and original Engine CDO both omit the name. UE3's
                # CalcComponentToFrameMatrix returns identity for an absent bone.
                assert p.get("translationspacebonename", "none") == "none"
            controls.append((index, p))
    assert names == {p["controlname"] for _, p in controls}
    # Native controller composition follows skeleton hierarchy, then each chain.
    return sorted(controls, key=lambda pair: pair[0])


def apply_controls(model, pose, controls, strengths):
    pose = list(pose)
    for index, p in controls:
        alpha = float(np.clip(strengths[p["controlname"]], 0., 1.))
        if not alpha:
            continue
        position, quaternion, scale = pose[index]
        rotation = Rotation.from_quat(quaternion).as_matrix()
        if p.get("bapplytranslation"):
            delta = LOCAL_BASIS @ base.vec(p.get("bonetranslation"))
            space = p.get("bonetranslationspace", "bcs_bonespace")
            if space == "bcs_bonespace":
                delta = rotation @ (scale * delta)
            elif space == "bcs_otherbonespace":
                # WModel component units include the inherited armature x100.
                # Only the component-frame fallback uses that conversion; local
                # Parent/BoneSpace source centimetres already match local units.
                parent = model.skeleton_bones[index].parent
                combined = np.eye(3)
                lineage = []
                while parent >= 0:
                    lineage.append(parent); parent = model.skeleton_bones[parent].parent
                for ancestor in reversed(lineage):
                    _, aq, asc = pose[ancestor]
                    combined = combined @ Rotation.from_quat(aq).as_matrix() @ np.diag(asc)
                delta = np.linalg.solve(combined, base.BASIS @ base.vec(p.get("bonetranslation")) * 100.)
            assert p.get("baddtranslation"), "Replacement translation needs an explicit frame origin"
            position = position + delta * alpha
        if p.get("bapplyrotation"):
            delta = LOCAL_BASIS @ base.actor_rotation({"rotation": p.get("bonerotation", {})}) @ LOCAL_BASIS
            assert p.get("baddrotation")
            target = delta @ rotation if p.get("bonerotationspace") == "bcs_parentbonespace" else rotation @ delta
            quaternion = base.slerp(quaternion, Rotation.from_matrix(target).as_quat(), alpha)
        scale = scale * (1.-alpha+alpha*p.get("bonescale", 1.))
        pose[index] = position, quaternion, scale
    return pose


def bake_body(model, rows, tree_rows, group, duration):
    tracks = base.active_tracks(rows, group)
    animation = {t["p"]["slotname"]: t["p"] for t in tracks if t["cls"] == "interptrackanimcontrol"}
    assert set(animation) == {"a", "b", "c"}
    assert all(p["outval"] == p.get("arrivetangent", 0.) == p.get("leavetangent", 0.) == 0.
               for p in animation["c"]["floattrack"]["points"]), "C is only discardable when identically zero"
    clips = {a.name.removeprefix("mesh_").removeprefix("ddk_"): a for a in model.animations}
    controls = controls_for(model, tree_rows, tracks)
    samples, probes = [], []
    times = [i/60. for i in range(math.ceil(duration*.06))] + [duration/1000.]
    for seconds in times:
        # B's weight is one. Before its first key, A already has full weight.
        bclip, bage = base.anim_at(animation["b"], clips, seconds)
        pose = base.pose_sample(model, bclip, bage)
        alpha = float(base.curve(animation["a"]["floattrack"]["points"], seconds, 1.))
        if alpha:
            aclip, aage = base.anim_at(animation["a"], clips, seconds)
            pose = blend(pose, base.pose_sample(model, aclip, aage), alpha)
        pose = apply_controls(model, pose, controls, strength_at(tracks, seconds))
        assert np.isfinite(np.asarray([np.concatenate(bone) for bone in pose])).all()
        samples.append(pose)
        if len(probes) < 10 and seconds >= len(probes)*.6:
            probes.append(dict(sourceSeconds=seconds, bClip=bclip.name, bClipSeconds=bage, aWeight=alpha))
    return times, samples, dict(controlApplications=len(controls), namedControls=sorted({p['controlname'] for _, p in controls}),
        zeroWeightSlot="c", sampleRate=60, probes=probes, frames=len(times),
        localControlBasis="UE [X,Y,-Z] in source centimetres; component fallback [X,Z,-Y] x100")


def rest_pose(model):
    result = []
    for bone in model.skeleton_bones:
        matrix = np.asarray(bone.transform).reshape(4, 4)
        scale = np.linalg.norm(matrix[:3, :3], axis=1)
        result.append((matrix[3, :3].copy(), Rotation.from_matrix((matrix[:3, :3]/scale[:, None]).T).as_quat(), scale))
    return result


def target_samples(target, body, samples, preserve_rest):
    indices = {bone.name.casefold(): i for i, bone in enumerate(body.skeleton_bones)}
    rest = rest_pose(target)
    output = []
    for sample in samples:
        pose = [rest[i] if b.name.casefold() in preserve_rest or b.name.casefold() not in indices
                else sample[indices[b.name.casefold()]] for i, b in enumerate(target.skeleton_bones)]
        output.append(pose)
    return output


def save_model(original_path, target_path, model, phases, drop_existing_animations=False):
    original = rebase_donor_material_paths(original_path.read_bytes(), original_path)
    if drop_existing_animations:
        header = list(base.wm.MODEL_HEADER.unpack_from(original, base.wm.FILE_HEADER.size))
        rows = [row for row in sections(original) if row[0] != 4]
        header[1], header[2] = len(rows), 0
        table, payload = bytearray(), bytearray()
        offset = base.wm.MODEL_HEADER.size + len(rows)*base.wm.SECTION_DESC.size
        for kind, index, name, data in rows:
            table += base.wm.SECTION_DESC.pack(kind, index, offset, len(data), name)
            payload += data; offset += len(data)
        body = base.wm.MODEL_HEADER.pack(*header) + table + payload
        file_header = list(base.wm.FILE_HEADER.unpack_from(original)); file_header[-1] = len(body)
        original = base.wm.FILE_HEADER.pack(*file_header) + body
    # WANM's trailing skeleton hash binds the animation to this exact palette.
    skeleton_hash = 0xcbf29ce484222325
    for bone in model.skeleton_bones:
        skeleton_hash = ((skeleton_hash ^ bone.name_hash) * 0x100000001b3) & ((1 << 64)-1)
    trailer = struct.pack("<Q", skeleton_hash)
    additions = [(name, compact_animation_payload(animation_payload(model, times, samples, trailer, base.wm.FILE_HEADER.unpack_from(original)[2])))
                 for name, times, samples in phases]
    candidate, count = append_idempotent(original, additions)
    target_path.parent.mkdir(parents=True, exist_ok=True); target_path.write_bytes(candidate)
    checked = base.wm.read_wmodel(target_path, include_geometry=False, animation_names=[p[0] for p in phases])
    # Check every added pose key through the same ordinary WModel reader used by
    # the existing bake tools; geometry/skeleton remain byte-for-byte identical.
    for animation in checked.animations:
        if animation.name in {p[0] for p in phases}:
            validate_keys(animation)
    return dict(candidatePath=str(target_path), sha256=hashlib.sha256(candidate).hexdigest(), addedClips=count,
                boneCount=len(model.skeleton_bones), preservedSections=len(sections(original)))


def bake_dragon(rows, output):
    path = ROOT / 'Client/Bin/Resources/Effect/GuardianKnight/FullRestore/Models/SK_DDK_DRG_00/sk_ddk_drg_00_head_mi.wmodel'
    clip_name = 'evt1_sk_super_sanctumofembereth'
    model = base.wm.read_wmodel(path, include_geometry=False, animation_names=[clip_name])
    clips = {a.name: a for a in model.animations}
    phases, details = [], []
    for name, matinee, group in (("intro", 702, 2567), ("loop", 701, 2529)):
        _, _, duration = phase_info(rows, matinee)
        tracks = [t['p'] for t in base.active_tracks(rows, group+1) if t['cls'] == 'interptrackanimcontrol']
        assert len(tracks) == 1 and len(tracks[0]['animseqs']) == 1
        times = [i/60. for i in range(math.ceil(duration*.06))] + [duration/1000.]
        samples = []
        for seconds in times:
            clip, age = base.anim_at(tracks[0], clips, seconds)
            samples.append(base.pose_sample(model, clip, age))
        phases.append(('guardian.select.'+name, times, samples))
        details.append(dict(phase=name, frames=len(times), sourceTrack=tracks[0]))
    asset = 'Character/GuardianKnight/Cinematics/ClassSelect/dragon/ClassSelectAnimSet.wmodel'
    detail = save_model(path, output / 'Resources' / asset, model, phases, drop_existing_animations=True)
    write(output / 'dragon-bake-receipt.json', dict(animationSetAssetId=asset, clips=details, **detail))
    print('baked dragon', len(model.skeleton_bones), 'bones; original embedded clips excluded from donor', flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--tree", type=Path, required=True)
    parser.add_argument("--geometry-manifest", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--dragon-only", action="store_true")
    args = parser.parse_args()
    assert not args.output.resolve().is_relative_to(ROOT / "Client/Bin/Resources")
    rows, trees = rows_from(read(args.source)), rows_from(read(args.tree))
    if args.dragon_only:
        bake_dragon(rows, args.output)
        return
    wanted = {key["animseqname"] for _, _, g in SPECS for t in base.active_tracks(rows, g+1)
              if t["cls"] == "interptrackanimcontrol" and t["p"]["slotname"] != "c" for key in t["p"]["animseqs"]}
    metadata = base.wm.read_wmodel(BODY, include_geometry=False, animation_names=())
    body = base.wm.read_wmodel(BODY, include_geometry=False,
        animation_names={a.name for a in metadata.animations if a.name.removeprefix("ddk_") in wanted})
    phases, details = [], []
    for name, matinee, group in SPECS:
        _, _, duration = phase_info(rows, matinee)
        times, samples, detail = bake_body(body, rows, trees, group+1, duration)
        phases.append(("guardian.select."+name, times, samples))
        details.append(dict(phase=name, **detail))
        print("baked", name, len(times), "frames", flush=True)
    body_out = args.output / "Resources/Character/GuardianKnight/Cinematics/ClassSelect/GuardianClassSelectAnimSet.wmodel"
    geometry = read(args.geometry_manifest)
    container = Path(next(e for e in geometry if e["part"] == "shadow")["resources"][0]["candidatePath"])
    receipt = dict(clips=details, body=save_model(container, body_out, body, phases), parts=[], installed=False)
    for entry in geometry:
        if entry["part"] not in ("hair", "wing"):
            continue
        first = entry["resources"][0]
        path = Path(first["candidatePath"])
        target = base.wm.read_wmodel(path, include_geometry=False, animation_names=())
        preserve = set(entry.get("preserveRestBones", [])) if entry["part"] == "hair" else set()
        remapped = [(name, times, target_samples(target, body, samples, preserve)) for name, times, samples in phases]
        asset = f"Character/GuardianKnight/Cinematics/ClassSelect/{entry['part']}/ClassSelectAnimSet.wmodel"
        detail = save_model(path, args.output / "Resources" / asset, target, remapped)
        receipt["parts"].append(dict(part=entry["part"], animationSetAssetId=asset, preserveRestBones=sorted(preserve), **detail))
        print("remapped", entry["part"], len(target.skeleton_bones), "bones", flush=True)
    write(args.output / "bake-receipt.json", receipt)


if __name__ == "__main__":
    main()

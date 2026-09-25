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


def rebase_donor_material_paths(data, source_path, resource_root=None):
    """Keep installed texture identity when copying a donor into ClassSelect."""
    resources = Path(resource_root or ROOT / "Client/Bin/Resources").resolve()
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


def save_model(original_path, target_path, model, phases, drop_existing_animations=False, resource_root=None):
    original = rebase_donor_material_paths(original_path.read_bytes(), original_path, resource_root)
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


def apply_profile_controls(model, pose, controls, strengths, actor):
    """Apply source controls in the basis proven by this actor's converter."""
    basis = np.asarray(actor['controlBasis'], dtype=float)
    component_basis = np.asarray(actor.get('componentBasis', base.BASIS), dtype=float)
    local_scale = float(actor['controlTranslationScale'])
    component_scale = float(actor.get('componentTranslationScale', 100.))
    for matrix in (basis, component_basis):
        assert matrix.shape == (3, 3) and np.isfinite(matrix).all()
        assert np.allclose(matrix.T @ matrix, np.eye(3), atol=1e-6)
    assert math.isfinite(local_scale) and local_scale > 0
    assert math.isfinite(component_scale) and component_scale > 0
    pose = list(pose)
    for index, p in controls:
        alpha = float(np.clip(strengths[p['controlname']], 0., 1.))
        if not alpha:
            continue
        position, quaternion, scale = pose[index]
        rotation = Rotation.from_quat(quaternion).as_matrix()
        if p.get('bapplytranslation'):
            delta = basis @ base.vec(p.get('bonetranslation')) * local_scale
            space = p.get('bonetranslationspace', 'bcs_bonespace')
            if space == 'bcs_bonespace':
                delta = rotation @ (scale * delta)
            elif space == 'bcs_otherbonespace':
                combined, lineage = np.eye(3), []
                parent = model.skeleton_bones[index].parent
                while parent >= 0:
                    lineage.append(parent); parent = model.skeleton_bones[parent].parent
                for ancestor in reversed(lineage):
                    _, aq, asc = pose[ancestor]
                    combined = combined @ Rotation.from_quat(aq).as_matrix() @ np.diag(asc)
                delta = np.linalg.solve(combined, component_basis @ base.vec(p.get('bonetranslation')) * component_scale)
            position = position + delta * alpha
        if p.get('bapplyrotation'):
            delta = basis @ base.actor_rotation({'rotation': p.get('bonerotation', {})}) @ basis.T
            target = delta @ rotation if p.get('bonerotationspace') == 'bcs_parentbonespace' else rotation @ delta
            quaternion = base.slerp(quaternion, Rotation.from_matrix(target).as_quat(), alpha)
        scale = scale * (1.-alpha+alpha*p.get('bonescale', 1.))
        pose[index] = position, quaternion, scale
    return pose


def bake_profile(profile_path, output):
    from class_selection_animation_tree import SourceAnimTree, sample_times, source_controls, source_strengths

    profile_path, output = profile_path.resolve(), output.resolve()
    assert output.is_relative_to((ROOT / 'out').resolve()), 'Candidate output must stay under repository out'
    profile = read(profile_path)

    def path(value):
        candidate = Path(value)
        return (candidate if candidate.is_absolute() else profile_path.parent / candidate).resolve()

    inputs = {}
    documents = {}

    def source_rows(source_path):
        if source_path not in documents:
            payload = source_path.read_bytes()
            inputs[str(source_path)] = hashlib.sha256(payload).hexdigest()
            raw = read(source_path)
            records = rows_from(raw)
            for row in raw['rows']:
                if 'parseError' in row:
                    records[row['exportIndex0']+1]['parseError'] = row['parseError']
            documents[source_path] = records
        return documents[source_path]

    rows = source_rows(path(profile['sourcePath']))
    actors = profile['actors']
    assert 0 < len(actors) <= 128
    identities, assets = set(), set()
    receipt = dict(classId=profile['classId'], profilePath=str(profile_path),
                   profileSha256=hashlib.sha256(profile_path.read_bytes()).hexdigest(),
                   actors=[], boneBindings=[], resources=[], installed=False)
    for actor in actors:
        identity, asset = actor['actorId'], actor['outputAssetId']
        assert identity and identity not in identities and asset not in assets
        identities.add(identity); assets.add(asset)
        relative = Path(asset)
        assert not relative.is_absolute() and not relative.drive and '..' not in relative.parts
        assert relative.parts[0] in ('Character', 'Effect') and relative.suffix.lower() == '.wmodel'
        donor_path = path(actor['donorModelPath'])
        geometry_path = path(actor.get('geometryModelPath', actor['donorModelPath']))
        resource_root = path(actor['resourceRoot'])
        assert geometry_path.is_relative_to(resource_root), 'Geometry must belong to its candidate Resources root'
        donor_bytes, geometry_bytes = donor_path.read_bytes(), geometry_path.read_bytes()
        inputs[str(donor_path)] = hashlib.sha256(donor_bytes).hexdigest()
        inputs[str(geometry_path)] = hashlib.sha256(geometry_bytes).hexdigest()
        skeleton = lambda data: [blob for kind, _, _, blob in sections(data) if kind == 3]
        assert len(skeleton(donor_bytes)) == 1 and skeleton(donor_bytes) == skeleton(geometry_bytes), (
            'Donor/geometry skeleton or rest basis mismatch', identity)
        clip_map = actor['clipMap']
        assert clip_map and len(set(clip_map.values())) == len(clip_map), 'Clip aliases must be explicit and unambiguous'
        model = base.wm.read_wmodel(donor_path, include_geometry=False, animation_names=set(clip_map.values()))
        by_name = {clip.name: clip for clip in model.animations}
        assert set(clip_map.values()) <= set(by_name), ('Missing donor clips', set(clip_map.values())-set(by_name))
        clips = {native: by_name[name] for native, name in clip_map.items()}
        trees = source_rows(path(actor['treePath']))
        root = int(actor['treeExportIndex0'])+1
        phases, details, phase_clips = [], [], {}
        for phase in actor['phases']:
            name, clip_name = phase['name'], phase['clipName']
            assert name in ('intro', 'loop') and name not in phase_clips
            phase_clips[name] = clip_name
            matinee, data, duration = phase_info(rows, int(phase['matineeExportIndex0']))
            if 'sourceHoldFrom' in phase:
                source_phase = phase['sourceHoldFrom']
                assert source_phase == 'intro' and name == 'loop'
                prior = next((p for p in phases if p[0] == phase_clips.get(source_phase)), None)
                assert prior is not None, 'Source hold requires a successfully baked intro'
                assert 'groupExportIndex0' not in phase, 'Source hold cannot replace a live authored group'
                final_pose = prior[2][-1]
                times, samples = [0., duration/1000.], [final_pose, final_pose]
                phases.append((clip_name, times, samples))
                details.append(dict(name=name, clipName=clip_name, durationMs=duration, sampleCount=2,
                                    sourceMatineeExportIndex0=matinee-1, sourceHoldFrom=source_phase,
                                    heldSourceSeconds=prior[1][-1]))
                print('held', profile['classId'], identity, name, 'from intro endpoint', flush=True)
                continue
            group = int(phase['groupExportIndex0'])+1
            assert group in rows[data]['p']['interpgroups'], ('Group not owned by phase', identity, name)
            source_actors = actor.get('sourceActorExportIndices0', [])
            assert source_actors and set(ref+1 for ref in source_actors) <= set(base.group_actor(rows, group, matinee)), (
                'Source actor/group binding mismatch', identity, name)
            tracks = base.active_tracks(rows, group)
            tree = SourceAnimTree(model, trees, root, tracks, clips)
            controls, absent = source_controls(model, trees, root, tracks)
            times, samples, probes = sample_times(tracks, duration), [], []
            last_control_signature, last_control_pose = None, None
            for index, seconds in enumerate(times):
                pose = tree.sample(seconds)
                strengths = source_strengths(tracks, seconds)
                signature = (id(pose), tuple(sorted(strengths.items())))
                if signature == last_control_signature:
                    pose = last_control_pose
                else:
                    pose = apply_profile_controls(model, pose, controls, strengths, actor)
                    last_control_signature, last_control_pose = signature, pose
                assert np.isfinite(np.asarray([np.concatenate(bone) for bone in pose])).all(), (identity, name, seconds)
                samples.append(pose)
                if index in (0, len(times)//2, len(times)-1):
                    probes.append(dict(sourceSeconds=seconds, rootPosition=pose[0][0].tolist(), rootRotation=pose[0][1].tolist()))
            phases.append((clip_name, times, samples))
            details.append(dict(name=name, clipName=clip_name, durationMs=duration, sampleCount=len(times),
                                sourceMatineeExportIndex0=matinee-1, sourceGroupExportIndex0=group-1,
                                namedControls=len(controls), absentPaletteControls=absent, probes=probes, **tree.receipt()))
            print('baked', profile['classId'], identity, name, len(times), 'frames', flush=True)
        destination = output / 'Resources' / asset
        saved = save_model(geometry_path, destination, model, phases, drop_existing_animations=True, resource_root=resource_root)
        detail = dict(actorId=identity, animationSetAssetId=asset, donorModel=str(destination),
                      sourceActorExportIndices0=actor['sourceActorExportIndices0'], clips=phase_clips,
                      phases=details, controlBasis=actor['controlBasis'], controlTranslationScale=actor['controlTranslationScale'], **saved)
        receipt['actors'].append(detail)
        receipt['resources'].append(dict(animationSetAssetId=asset, candidatePath=str(destination),
                                         clips=phase_clips, sha256=saved['sha256']))
        for source_actor in actor['sourceActorExportIndices0']:
            receipt['boneBindings'].append(dict(sourceActorIndex0=source_actor, donorModel=str(destination),
                modelPreScale=float(actor.get('modelPreScale', .01)), animationSetAssetId=asset, clips=phase_clips))
        write(output / 'bake-receipt.json', receipt)
    for source_path, expected in inputs.items():
        assert hashlib.sha256(Path(source_path).read_bytes()).hexdigest() == expected, ('Source changed during bake', source_path)
    receipt['inputs'] = inputs
    receipt['complete'] = True
    write(output / 'bake-receipt.json', receipt)
    return receipt


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
    parser.add_argument("--profile", type=Path, help="Exact offline source/actor/palette profile; preserves the Guardian default CLI")
    parser.add_argument("--source", type=Path)
    parser.add_argument("--tree", type=Path)
    parser.add_argument("--geometry-manifest", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--dragon-only", action="store_true")
    args = parser.parse_args()
    if args.profile:
        assert not args.dragon_only
        bake_profile(args.profile, args.output)
        return
    assert args.source and args.tree and args.geometry_manifest, 'Guardian mode requires source, tree and geometry-manifest'
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

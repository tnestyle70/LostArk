"""Restore SCENE04A camera-attached set pieces through existing World Sequence tracks.

The source parent/bone sampler is supplied by build_gate2_intro_composition.
No installed resource or authoring document is changed by this helper itself.
"""
from __future__ import annotations

from collections import defaultdict
import json
import math
from pathlib import Path
import shlex

import numpy as np
from scipy.spatial.transform import Rotation

ROOT = Path(__file__).resolve().parents[2]
AREA = "LV_LUT_MIDNIGHTC_ED"
PREFIX = "kouku.gate2.intro.backdrop"
BASIS = np.array([[1., 0., 0.], [0., 0., 1.], [0., -1., 0.]])
DURATION = 27000
POSITION_TOLERANCE = .001
ROTATION_TOLERANCE_DEGREES = .05


def vec(value, default=(0., 0., 0.)):
    return np.array([value.get(k, default[i]) for i, k in enumerate("xyz")]) if isinstance(value, dict) else np.array(default)


def source_rotation(value):
    d = value.get("degrees", {})
    return Rotation.from_euler("ZYX", [d.get("yaw", 0), -d.get("pitch", 0), -d.get("roll", 0)], degrees=True).as_matrix()


def reduced_indices(times, positions, quaternions):
    """Keep a linear-position/slerp path within measured errors at all source samples."""
    keep = {0, len(times) - 1}
    pending = [(0, len(times) - 1)]
    radians = math.radians(ROTATION_TOLERANCE_DEGREES)
    while pending:
        a, b = pending.pop()
        if b <= a + 1:
            continue
        ratio = (times[a + 1:b] - times[a]) / (times[b] - times[a])
        pos = positions[a][None, :] * (1 - ratio[:, None]) + positions[b][None, :] * ratio[:, None]
        qa, qb = quaternions[a], quaternions[b]
        dot = float(np.dot(qa, qb))
        if dot < 0:
            qb = -qb
            dot = -dot
        angle = math.acos(min(1., max(-1., dot)))
        if angle < 1e-7:
            q = qa[None, :] * (1 - ratio[:, None]) + qb[None, :] * ratio[:, None]
        else:
            q = (np.sin((1 - ratio) * angle)[:, None] * qa + np.sin(ratio * angle)[:, None] * qb) / math.sin(angle)
        q /= np.linalg.norm(q, axis=1)[:, None]
        rotation_error = 2 * np.arccos(np.clip(np.abs(np.sum(q * quaternions[a + 1:b], axis=1)), 0., 1.))
        errors = np.maximum(np.linalg.norm(pos - positions[a + 1:b], axis=1) / POSITION_TOLERANCE,
                            rotation_error / radians)
        worst = int(np.argmax(errors))
        if errors[worst] > 1:
            index = a + 1 + worst
            keep.add(index)
            pending += [(a, index), (index, b)]
    return sorted(keep)


def build_backdrops(rows, imports, pose_sampler):
    """Return resources/templates/instances/worlds/windows and a source receipt.

    pose_sampler(actor_index, seconds) returns the source hierarchy's client-space
    position and 3x3 rotation. It must include the existing baked skeletal parents.
    """
    groups = rows[394]["p"]["interpgroups"]
    links = rows[329]["p"]["variablelinks"]
    directly_bound = set()
    for group in groups:
        name = rows[group]["p"].get("groupname", "")
        link = next((r for r in links if r["linkdesc"].casefold() == str(name).casefold()), {})
        directly_bound.update(rows[v]["p"]["objvalue"] for v in link.get("linkedvariables", []) if rows[v]["p"].get("objvalue"))
    children = [r for r in rows.values() if "staticmeshcomponent" in r.get("p", {}) and r["index"] not in directly_bound]
    assert len(children) == 165 and all(r["p"].get("base") in rows for r in children)
    catalog_path = ROOT / "Client/Bin/DataFiles/Map" / f"{AREA}.mapassets"
    catalog = [shlex.split(line) for line in catalog_path.read_text(encoding="utf-8-sig").splitlines()[1:]]
    materials = json.loads((ROOT / "Data/Maps/Authoring" / AREA / f"{AREA}.mapmaterials.json").read_text(encoding="utf-8-sig"))["materials"]
    resources = {}
    source_receipt = []
    by_parent = defaultdict(list)
    times = set(round(i * 1000 / 30) for i in range(811))
    for row in rows.values():
        if row["cls"] != "interptrackmove":
            continue
        for field in ("postrack", "eulertrack"):
            points = row["p"].get(field, {}).get("points", [])
            for index, point in enumerate(points):
                ms = round(point["inval"] * 1000)
                if 0 <= ms <= DURATION:
                    times.add(ms)
                    if index and points[index - 1].get("interpmode") == "cim_constant" and ms:
                        times.add(ms - 1)
    times = np.array(sorted(times), dtype=int)
    parent_samples = {}
    sampled = {}
    for row in children:
        actor, p = row["index"], row["p"]
        parent = p["base"]
        by_parent[parent].append(actor)
        component = rows[p["staticmeshcomponent"]]["p"]
        source_mesh = imports[str(component["staticmesh"])]
        matches = [r for r in catalog if r[0].endswith("_" + source_mesh.rsplit(".", 1)[-1].upper())]
        assert len(matches) == 1, (actor, source_mesh, matches)
        asset, model = matches[0][0], matches[0][2]
        assert (ROOT / "Client/Bin/Resources" / model).is_file(), model
        map_rows = [r for r in materials if r["assetId"] == asset]
        assert map_rows and all(r["family"] == "bg-source-opaque-masked" for r in map_rows), asset
        for slot_index, material_index in enumerate(component.get("materials", [])):
            if not material_index:
                continue
            slot_prefix = f"SLOT_{slot_index:03}_"
            source_material = imports[str(material_index)]
            assert any(m["materialName"].startswith(slot_prefix) and m.get("sourceMaterial") == source_material
                       for m in map_rows), (actor, "unresolved component material override", source_material)
        object_id = f"world.object.{PREFIX}.{asset.lower()}"
        resources.setdefault(asset, dict(objectId=object_id, displayName=source_mesh.rsplit(".", 1)[-1],
            modelAssetId=model, anchorKind="WORLD", diffuseTextureAssetId="", modelPreScale=.01,
            animated=False, scale=[1, 1, 1], sequenceInstanceId="", defaultMotionInstanceId="",
            mapMaterialBindings=[dict(materialName=m["materialName"], sourceAssetId=asset,
                                     sourceMaterialName=m["materialName"]) for m in map_rows]))
        if parent not in parent_samples:
            parent_samples[parent] = [pose_sampler(parent, int(ms) / 1000.) for ms in times]
        relative = BASIS @ vec(p.get("relativelocation")) * .01
        relative_rotation = BASIS @ source_rotation(p.get("relativerotation", {})) @ BASIS.T
        rotations = []
        positions = []
        motions = [rows[m] for m in p.get("motionarr", [])]
        for ms, (parent_position, parent_rotation) in zip(times, parent_samples[parent]):
            local_rotation = relative_rotation.copy()
            for motion in motions:
                m = motion["p"]
                assert motion["cls"] == "efactormotionrotationcyclic" and m.get("fmotioncycle", 0) > 0
                axis = {"axis_x": "x", "axis_y": "y", "axis_z": "z"}[m["emotionaxis"]]
                # The cooked asset records range/cycle/axis, but not native code.
                # A zero-phase sinusoid is an explicit reconstructed waveform.
                angle = m["fmotionrange"] * math.sin(2 * math.pi * int(ms) / (1000 * m["fmotioncycle"]))
                delta = Rotation.from_euler(axis, -angle if axis in ("x", "y") else angle, degrees=True).as_matrix()
                local_rotation = local_rotation @ (BASIS @ delta @ BASIS.T)
            rotation = parent_rotation @ local_rotation
            if p.get("bignorebaserotation"):
                rotation = BASIS @ source_rotation(p.get("rotation", {})) @ BASIS.T
            positions.append(parent_position + parent_rotation @ relative)
            quaternion = Rotation.from_matrix(rotation).as_quat()
            rotations.append(-quaternion if quaternion[3] < 0 else quaternion)
        positions = np.asarray(positions)
        quaternions = np.asarray(rotations)
        assert np.isfinite(positions).all() and np.isfinite(quaternions).all(), actor
        keep = reduced_indices(times, positions, quaternions)
        scale = vec(p.get("drawscale3d"), (1., 1., 1.))[[0, 2, 1]] * p.get("drawscale", 1.)
        sampled[actor] = (object_id, positions, quaternions, scale, keep)
        source_receipt.append(dict(actorIndex=actor, sourceName=row["name"], parentIndex=parent,
            sourceMesh=source_mesh, modelAssetId=model, sourceAssetId=asset, sampleCount=len(times),
            reducedKeyCount=len(keep), motionPrograms=motions,
            waveform="RECONSTRUCTED_ZERO_PHASE_SINE" if motions else "SOURCE_PARENT_TRANSFORM"))
    templates, instances, worlds, windows = [], [], [], []
    for parent, actors in sorted(by_parent.items()):
        # One parent set uses at most 32 slots per runtime instance. Shared segment
        # boundaries preserve continuity when a trajectory needs over 256 keys.
        for chunk_start in range(0, len(actors), 32):
            chunk = actors[chunk_start:chunk_start + 32]
            indices = sorted(set(i for actor in chunk for i in sampled[actor][4]))
            for segment, start in enumerate(range(0, len(indices) - 1, 255)):
                segment_indices = indices[start:start + 256]
                if segment_indices[-1] != indices[-1] and len(segment_indices) < 256:
                    raise AssertionError("incomplete backdrop segment")
                first, last = segment_indices[0], segment_indices[-1]
                begin, end = int(times[first]), int(times[last])
                tag = f"parent{parent}.part{chunk_start // 32 + 1}.segment{segment + 1}"
                sequence_id = f"sequence.{PREFIX}.{tag}"
                instance_id = f"world.sequence.instance.{PREFIX}.{tag}"
                bindings, tracks = [], []
                for actor in chunk:
                    object_id, positions, quaternions, scale, keep = sampled[actor]
                    selected = sorted({first, last, *(i for i in keep if first < i < last)})
                    slot = f"source.actor.{actor}"
                    bindings.append(dict(slotId=slot, targetKind="OBJECT_RESOURCE", targetId=object_id))
                    keys = [dict(timeMs=int(times[i]) - begin, positionOffset=np.round(positions[i], 7).tolist(),
                        rotationQuaternion=np.round(quaternions[i], 9).tolist(), scaleMultiplier=np.round(scale, 9).tolist(),
                        visible=not rows[actor]["p"].get("bhidden", False)) for i in selected]
                    assert len(keys) <= 256
                    tracks.append(dict(slotId=slot, keys=keys))
                label = "2관문 진입 촬영 세트 / " + rows[parent]["name"].rsplit(".", 1)[-1]
                templates.append(dict(sequenceId=sequence_id, displayName=label, category="World",
                    durationMs=end - begin, interpolation="LINEAR", tracks=tracks, animationTracks=[]))
                instances.append(dict(instanceId=instance_id, templateId=sequence_id, enabled=True, startDelayMs=0,
                    playbackSpeed=1, anchorKind="WORLD", position=[0, 0, 0], motionEnd="STOP", nextMotionId="", bindings=bindings))
                worlds.append(dict(worldId=f"world.{PREFIX}.{tag}", displayName=label, sequenceInstanceId=instance_id,
                    positionOffset=[0, 0, 0], anchorKind="NONE", anchorPosition=[0, 0, 0], companionEffectResourceId=""))
                windows.append((begin, end))
    return dict(resources=list(resources.values()), templates=templates, instances=instances, worlds=worlds,
                windows=windows, receipt=dict(sourceStaticActorCount=172, alreadyDirectlyBoundCount=7,
                    restoredAttachedCount=len(children), parentCount=len(by_parent), modelCount=len(resources),
                    positionToleranceM=POSITION_TOLERANCE, rotationToleranceDegrees=ROTATION_TOLERANCE_DEGREES,
                    reconstructedCyclicCount=sum(bool(r["motionPrograms"]) for r in source_receipt), actors=source_receipt))

"""Append source Matinee poses to ordinary Valtan AnimSets without changing base clips.

No runtime skeleton, mesh, material, gameplay, or animation path is introduced.
The candidate files use the same CModel Attach_AnimationSet contract as the base.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import sys
from pathlib import Path

import numpy as np
from scipy.spatial.transform import Rotation

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/KoukuSaydonPipeline"))
import build_gate2_intro_composition as source

wm = source.wm
RESOURCES = ROOT / "Client/Bin/Resources"
SCENES = ROOT / "out/FullMapRestoration20260915/ValtanSequences"
SPECS = (
    ("entrance", "SCENE06A", 53, 136, "normal"),
    ("trash", "SCENE06A", 54, 159, "normal"),
    ("finale", "SCENE04A", 24, 45, "ghost"),
)
BASE = {
    "normal": "Character/Valtan/AnimSets/MN_RPBF_01_AnimSet.wmodel",
    "ghost": "Character/Valtan/Ghost/MN_RPBF_02_AnimSet.wmodel",
}
OUTPUT = {
    "normal": "Character/Valtan/Cinematics/MN_RPBF_01_CinematicAnimSet.wmodel",
    "ghost": "Character/Valtan/Cinematics/MN_RPBF_02_CinematicAnimSet.wmodel",
}


def sha(data):
    return hashlib.sha256(data).hexdigest()


def inherited_cinematic_tree():
    records = source.read(ROOT / "out/KoukuAllEffects20260912/source_class_defaults.json")["records"]
    script = next(r["sourcePackage"] for r in records if r["fullPath"].startswith("efgame."))
    path = source.PACKAGES.parent / Path(script).name
    package = source.load_package(path, source.ue3.LOSTARK_KR_AES_KEY)
    def normalized(value):
        if isinstance(value, dict):
            return {key.lower(): normalized(item) for key, item in value.items()}
        if isinstance(value, list):
            return [normalized(item) for item in value]
        return value.lower() if isinstance(value, str) else value
    def props(index):
        entry = package.exports[index - 1]
        raw = package.logical[entry.serial_offset:entry.serial_offset + entry.serial_size]
        return normalized(source.unwrap(source.ue3.parse_tagged_properties(raw, package.names, package.summary.version)[0]))
    cdo = next(e.index + 1 for e in package.exports if source.ue3.package_ref_path(
        e.index + 1, package.imports, package.exports).lower() == "default__efskeletalmeshactorlookinfomat")
    component = props(cdo)["skeletalmeshcomponent"]
    tree_id = props(component)["animtreetemplate"]
    tree_name = source.ue3.package_ref_path(tree_id, package.imports, package.exports).lower()
    assert tree_name == "animblendingmat.animblending_mix"
    rows = {}
    for entry in package.exports:
        name = source.ue3.package_ref_path(entry.index + 1, package.imports, package.exports).lower()
        if not name.startswith("animblendingmat."):
            continue
        try:
            properties = props(entry.index + 1)
        except source.ue3.ExtractionError:
            continue
        rows[entry.index + 1] = dict(index=entry.index + 1, name=name.removeprefix("animblendingmat."),
            cls=source.ue3.package_ref_name(entry.class_index, package.imports, package.exports).lower(), p=properties)
    return rows, dict(package=str(path), packageSha256=sha(path.read_bytes()), cdoExport=cdo,
        componentExport=component, animTreeExport=tree_id, animTree=tree_name)


def sections(data):
    header = wm.MODEL_HEADER.unpack_from(data, wm.FILE_HEADER.size)
    result = []
    for i in range(header[1]):
        kind, index, offset, size, name = wm.SECTION_DESC.unpack_from(
            data, wm.FILE_HEADER.size + wm.MODEL_HEADER.size + i * wm.SECTION_DESC.size)
        result.append((kind, index, name, data[wm.FILE_HEADER.size + offset:wm.FILE_HEADER.size + offset + size]))
    return result


def upper_mask(model, tree_rows):
    tree = next(r for r in tree_rows.values() if r["name"] == "animblending_mix")
    # Follow the actual tree: root -> facial wrappers -> C -> masked UP_A -> A/B.
    pending, seen, matching = [tree["index"]], set(), []
    while pending:
        index = pending.pop()
        if not index or index in seen:
            continue
        seen.add(index)
        row = tree_rows[index]
        children = row["p"].get("children", [])
        pending.extend(x["anim"] for x in children)
        if row["cls"] == "animnode_multiblendperbone" and len(children) == 2:
            target = tree_rows[children[1]["anim"]]["p"]
            if target.get("nodename") == "up_a":
                matching.append(row["p"]["masklist"][0])
    assert len(matching) == 1
    mask = matching[0]
    assert mask["desiredweight"] == 1 and not mask["weightrulelist"]
    weights = np.zeros(len(model.skeleton_bones))
    names = {bone.name.lower(): i for i, bone in enumerate(model.skeleton_bones)}
    for branch in mask["branchlist"]:
        root = names[branch["bonename"].lower()]
        step = branch["perboneweightincrease"]
        assert step == 1, "This source uses a full subtree, not a gradual mask"
        for i, bone in enumerate(model.skeleton_bones):
            parent = i
            while parent >= 0:
                if parent == root:
                    weights[i] = 1
                    break
                parent = model.skeleton_bones[parent].parent
    assert 0 < sum(weights) < len(weights)
    return weights, mask


def blend(base, overlay, weights):
    return [(bp * (1 - a) + p * a, source.slerp(bq, q, a), bs * (1 - a) + s * a)
            for (bp, bq, bs), (p, q, s), a in zip(base, overlay, weights)]


def bake(model, rows, group, length, tree_rows):
    tracks = {r["p"]["slotname"]: r["p"] for r in source.active_tracks(rows, group)
              if r["cls"] == "interptrackanimcontrol" and r["p"].get("animseqs")}
    assert "a" in tracks and set(tracks) <= {"a", "b", "up_a", "up_b"}
    animations = {a.name.removeprefix("mesh_"): a for a in model.animations}
    for track in tracks.values():
        for key in track["animseqs"]:
            assert key["animseqname"] in animations, key
    mask, mask_source = upper_mask(model, tree_rows)
    strength = {r["p"]["skelcontrolname"]: r["p"] for r in source.active_tracks(rows, group)
                if r["cls"] == "interptrackskelcontrolstrength"}
    controls = []
    tree = next(r for r in tree_rows.values() if r["name"] == "animblending_mix")
    names = {bone.name.lower(): i for i, bone in enumerate(model.skeleton_bones)}
    for chain in tree["p"].get("skelcontrollists", []):
        index, ref = names.get(chain["bonename"].lower()), chain["controlhead"]
        while ref:
            control = tree_rows[ref]["p"]
            ref = control.get("nextcontrol", 0)
            if index is not None and control.get("controlname") in strength:
                assert control.get("bapplyrotation") and control.get("baddrotation")
                assert control["bonerotationspace"] == "bcs_parentbonespace"
                assert not control.get("bapplytranslation") and "bonescale" not in control
                controls.append((index, control, strength[control["controlname"]]))

    def pose(slot, seconds):
        anim, age = source.anim_at(tracks[slot], animations, seconds)
        return source.pose_sample(model, anim, age)

    def alpha(slot, seconds):
        track = tracks[slot]
        if seconds < track["animseqs"][0]["starttime"]:
            return 0.
        return float(np.clip(source.curve(track.get("floattrack", {}).get("points", []), seconds, 1.), 0, 1))

    times = [i / 30. for i in range(math.ceil(length * 30.))] + [length]
    samples, probes = [], []
    for seconds in times:
        base = pose("a", seconds)
        if "b" in tracks:
            base = blend(pose("b", seconds), base, [alpha("a", seconds)] * len(base))
        upper = base
        for slot in ("up_b", "up_a"):
            if slot in tracks:
                upper = blend(upper, pose(slot, seconds), mask * alpha(slot, seconds))
        for index, control, track in controls:
            weight = float(source.curve(track.get("floattrack", {}).get("points", []), seconds, control.get("controlstrength", 0.)))
            p, q, s = upper[index]
            delta = source.BASIS @ source.actor_rotation({"rotation": control["bonerotation"]}) @ source.BASIS.T
            target = Rotation.from_matrix(delta @ Rotation.from_quat(q).as_matrix()).as_quat()
            upper[index] = (p, source.slerp(q, target, weight), s)
        assert all(np.isfinite(value).all() for bone in upper for value in bone)
        samples.append(upper)
        if "up_a" in tracks and alpha("up_a", seconds) == 1:
            # No lower-body local pose changes are allowed from an upper mask.
            assert all(all(np.array_equal(x, y) for x, y in zip(base[i], upper[i]))
                       for i in range(len(base)) if mask[i] == 0)
        if 18.7 < seconds < 19.4:
            _, age = source.anim_at(tracks["a"], animations, seconds)
            probes.append([seconds, age])
    if any(key.get("breverse") for key in tracks["a"]["animseqs"]):
        assert len(probes) > 2 and probes[0][1] > probes[-1][1]
    return times, samples, dict(slots=list(tracks), upperMask=mask_source,
        maskedBoneCount=int(sum(mask)), controls=[c[1]["controlname"] for c in controls],
        reverseSamples=probes, finitePoseSamples=len(samples) * len(model.skeleton_bones))


def animation_payload(model, times, samples, trailer, endian):
    keys, channels = bytearray(), bytearray()
    count = len(times)
    for index, bone in enumerate(model.skeleton_bones):
        offsets = []
        for component, layout in ((0, wm.VECTOR_KEY), (1, wm.QUATERNION_KEY), (2, wm.VECTOR_KEY)):
            offsets.append(len(keys))
            for seconds, pose in zip(times, samples):
                keys += layout.pack(seconds * 30., *pose[index][component])
        channels += wm.ANIMATION_CHANNEL.pack(bone.name_hash, count, offsets[0], count,
            offsets[1], count, offsets[2], index, 0)
    payload = wm.ANIMATION_HEADER.pack(b"WANM", len(model.skeleton_bones), times[-1] * 30.,
        30., count * 3 * len(model.skeleton_bones), 0, 0, b"\0" * 7) + channels + keys + trailer
    return wm.FILE_HEADER.pack(b"WINT", 1, endian, 0, len(payload)) + payload


def append(source_bytes, additions):
    file_header = list(wm.FILE_HEADER.unpack_from(source_bytes))
    header = list(wm.MODEL_HEADER.unpack_from(source_bytes, wm.FILE_HEADER.size))
    original = sections(source_bytes)
    rows = original + [(4, header[2] + i, name.encode().ljust(40, b"\0"), payload)
                       for i, (name, payload) in enumerate(additions)]
    header[1], header[2] = len(rows), header[2] + len(additions)
    table, body = bytearray(), bytearray()
    offset = wm.MODEL_HEADER.size + len(rows) * wm.SECTION_DESC.size
    for kind, index, name, payload in rows:
        table += wm.SECTION_DESC.pack(kind, index, offset, len(payload), name)
        body += payload
        offset += len(payload)
    payload = wm.MODEL_HEADER.pack(*header) + table + body
    file_header[-1] = len(payload)
    result = wm.FILE_HEADER.pack(*file_header) + payload
    assert sections(result)[:len(original)] == original
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    tree_rows, tree_evidence = inherited_cinematic_tree()
    receipt = dict(tool=Path(__file__).relative_to(ROOT).as_posix(), inheritedTree=tree_evidence, models=[])
    for kind, asset in BASE.items():
        path = RESOURCES / asset
        original = path.read_bytes()
        model = wm.read_wmodel(path)
        trailer = next(payload[-8:] for section_kind, _, _, payload in sections(original) if section_kind == 4)
        additions, details = [], []
        for name, scene, matinee, group, model_kind in SPECS:
            scene_path = SCENES / f"LV_LUT_HEARTRB_ED_{scene}.json"
            document = source.read(scene_path)
            rows = {int(k): dict(r, p=source.unwrap(r["p"])) for k, r in document["rows"].items()}
            data_id = next(v["linkedvariables"][0] for v in rows[matinee]["p"]["variablelinks"] if v["linkdesc"] == "Data")
            length = rows[data_id]["p"]["interplength"]
            actor = source.group_actor(rows, group, matinee)[0]
            component = rows[actor]["p"]["skeletalmeshcomponent"]
            tree_ref = rows[component]["p"].get("animtreetemplate")
            if tree_ref is not None:
                assert document["imports"][str(tree_ref)] == "animblendingmat.animblending_mix"
            if name == "entrance":
                def animation_inputs(group_id):
                    return {r["p"]["slotname"]: {key: r["p"].get(key) for key in ("animseqs", "floattrack")}
                            for r in source.active_tracks(rows, group_id) if r["cls"] == "interptrackanimcontrol"}
                assert animation_inputs(136) == animation_inputs(137), "Colorless actor needs an independent clip"
            times, samples, detail = bake(model, rows, group, length, tree_rows)
            clip = f"valtan.cinematic.{name}"
            additions.append((clip, animation_payload(model, times, samples, trailer, wm.FILE_HEADER.unpack_from(original)[2])))
            details.append(dict(clipName=clip, durationSeconds=length, frameCount=len(times), sceneSha256=sha(scene_path.read_bytes()),
                sourceMatinee=matinee, sourceGroup=group, sourceActor=actor, **detail))
        candidate = args.output / OUTPUT[kind]
        candidate.parent.mkdir(parents=True, exist_ok=True)
        candidate.write_bytes(append(original, additions))
        checked = wm.read_wmodel(candidate)
        assert len(checked.animations) == len(model.animations) + len(additions)
        assert [a.name for a in checked.animations[:len(model.animations)]] == [a.name for a in model.animations]
        receipt["models"].append(dict(sourceAssetId=asset, targetAssetId=OUTPUT[kind], sourceSha256=sha(original),
            candidateSha256=sha(candidate.read_bytes()), preservedSectionCount=len(sections(original)),
            preservedAnimationCount=len(model.animations), animationCount=len(checked.animations), clips=details))
        print(kind, len(model.animations), "->", len(checked.animations), candidate, flush=True)
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output / "receipt.json").write_text(json.dumps(receipt, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()

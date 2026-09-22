"""Project original Guardian class-select Matinees into staged runtime documents.

The input is a lossless tagged-property extraction, not an editor placement copy.
This tool never writes authoring, published data, or installed Resources. Unhandled
source tracks remain in the timeline receipt and prevent a claim of full parity.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
from pathlib import Path
import sys

import numpy as np
from scipy.integrate import quad
from scipy.spatial.transform import Rotation

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/KoukuSaydonPipeline"))
import build_gate2_intro_composition as base

AREA = "LV_LOBBY_CLASSSELECT_SL00"
PREFIX = "classselect.guardianknight"
PHASES = (("intro", 702), ("loop", 701))  # Source export indices are zero-based.
BONE_BINDINGS = {}


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n", encoding="utf-8")


def read(path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def rows_from(document):
    # Existing cinematic projection helpers use UE object refs (one-based).
    return {r["exportIndex0"] + 1: dict(index=r["exportIndex0"] + 1,
        name=r["objectPath"], cls=r["className"], p=copy.deepcopy(r["properties"]))
        for r in document["rows"]}


def phase_info(rows, matinee0):
    matinee = matinee0 + 1
    data = next(link["linkedvariables"][0] for link in rows[matinee]["p"]["variablelinks"]
                if link["linkdesc"] == "Data")
    seconds = rows[data]["p"]["interplength"]
    return matinee, data, round(seconds * 1000)


def source_key_times(tracks, duration):
    result = {0, duration}
    for track in tracks:
        for value in track["p"].values():
            points = value.get("points", []) if isinstance(value, dict) else value if isinstance(value, list) else []
            for point in points:
                if not isinstance(point, dict):
                    continue
                seconds = point.get("inval", point.get("time", point.get("starttime")))
                if not isinstance(seconds, (int, float)):
                    continue
                ms = round(seconds * 1000)
                if 0 <= ms <= duration:
                    result.add(ms)
                    if ms and point.get("interpmode") == "cim_constant":
                        result.add(ms - 1)
    return result


def cameras(rows, matinee, data, duration, phase):
    groups = rows[data]["p"]["interpgroups"]
    names = {rows[g]["p"].get("groupname"): g for g in groups}
    tracks = [track for group in groups for track in base.active_tracks(rows, group)]
    directors = [t for t in tracks if t["cls"] == "interptrackdirector"]
    assert len(directors) == 1
    cuts = directors[0]["p"]["cuttrack"]
    result = []
    for index, cut in enumerate(cuts):
        start = max(0, round(cut["time"] * 1000))
        end = min(duration, round(cuts[index + 1]["time"] * 1000)) if index + 1 < len(cuts) else duration
        if end <= start:
            continue
        group = names[cut["targetcamgroup"]]
        actor = base.group_actor(rows, group, matinee)[0]
        camera_tracks = base.active_tracks(rows, group)
        fovs = [t for t in camera_tracks if t["cls"] == "interptrackfloatprop" and t["p"].get("propertyname") == "fovangle"]
        assert len(fovs) == 1
        times = {start, end} | set(range(start, end, 8))
        times |= {t for t in source_key_times(tracks, duration) if start <= t <= end}
        keys = []
        identity = f"{PREFIX}.{phase}.camera.{index + 1}"
        for ms in sorted(times):
            eye, rotation = base.world_pose(rows, group, actor, ms / 1000., matinee, data)
            forward = rotation @ np.array([1., 0., 0.])
            up = rotation @ np.array([0., 1., 0.])
            fov = float(base.curve(fovs[0]["p"]["floattrack"]["points"], ms / 1000., 90.))
            assert np.isfinite([*eye, *forward, *up, fov]).all()
            assert 1. < fov < 179. and abs(float(forward @ up)) < 1e-6
            keys.append(dict(keyId=f"{identity}.k{len(keys)}", timeMs=ms-start,
                eye=eye.tolist(), lookAt=(eye+forward).tolist(), up=up.tolist(), fovDegrees=fov))
        assert len(keys) <= 4096
        result.append(dict(cameraId=identity, displayName=f"Guardian {phase} {cut['targetcamgroup']}",
            source=f"LV_LOBBY_CLASSSELECT_SCENE01 Matinee{rows[matinee]['p']['matineeindex']} Director export {directors[0]['index']-1}; camera export {actor-1}",
            startMs=start, durationMs=end-start, space="WORLD", fovAxis="HORIZONTAL",
            interpolation="LINEAR", easing="LINEAR", muted=False, keys=keys))
    assert result[0]["startMs"] == 0 and result[-1]["startMs"] + result[-1]["durationMs"] == duration
    return result


def clock_keys(rows, data, duration):
    tracks = [t for g in rows[data]["p"]["interpgroups"] for t in base.active_tracks(rows, g)
              if t["cls"] == "interptrackslomo"]
    if not tracks:
        return [], dict(sourceDurationMs=duration, wallDurationMs=duration, rate=1.)
    assert len(tracks) == 1
    points = tracks[0]["p"]["floattrack"]["points"]
    points = sorted(points, key=lambda p: p["inval"])
    # d(source)/d(wall) = TimeDilation. Integrate its reciprocal in source time.
    # Dense only inside changed dilation; constant regions need two endpoints.
    times = {0., duration / 1000.}
    for left, right in zip(points, points[1:]):
        a, b = max(0., left["inval"]), min(duration / 1000., right["inval"])
        if b < a:
            continue
        times.update([a, b])
        if left["outval"] != right["outval"] or left.get("leavetangent", 0.) or right.get("arrivetangent", 0.):
            times.update(a+(b-a)*i/math.ceil((b-a)*240) for i in range(math.ceil((b-a)*240)+1))
    wall, previous, result = 0., 0., [dict(timeMs=0., sourceMs=0.)]
    def reciprocal(t):
        dilation = float(base.curve(points, t, 1.))
        assert math.isfinite(dilation) and dilation > 0.
        return 1. / dilation
    for seconds in sorted(times - {0.}):
        wall += quad(reciprocal, previous, seconds, epsabs=1e-10)[0]
        result.append(dict(timeMs=wall*1000., sourceMs=seconds*1000.))
        previous = seconds
    result[-1]["sourceMs"] = duration
    return result, dict(sourceDurationMs=duration, wallDurationMs=wall*1000.,
        slomoTrackExportIndex0=tracks[0]["index"]-1, integration="integral ds / source TimeDilation(s)",
        maxSourceStepInsideChangingDilationMs=1000./240.)


def timeline(rows, matinee, data, duration):
    groups = []
    for group in rows[data]["p"]["interpgroups"]:
        tracks = []
        for ref in rows[group]["p"].get("interptracks", []):
            row = rows[ref]
            tracks.append(dict(exportIndex0=ref-1, sourcePath=row["name"], className=row["cls"],
                disabled=row["p"].get("bdisabletrack", False), properties=row["p"]))
        groups.append(dict(groupExportIndex0=group-1, name=rows[group]["p"].get("groupname", "director"),
            actorExportIndices0=[a-1 for a in base.group_actor(rows, group, matinee)], tracks=tracks))
    return dict(matineeExportIndex0=matinee-1, interpDataExportIndex0=data-1, durationMs=duration, groups=groups)


def visible_at(rows, group, actor, seconds):
    visible = not rows[actor]["p"].get("bhidden", False)
    for tr in base.active_tracks(rows, group):
        if tr["cls"] != "interptrackvisibility":
            continue
        for key in tr["p"].get("visibilitytrack", []):
            if key["time"] > seconds:
                break
            assert key.get("activecondition", "evtc_always") == "evtc_always"
            action = key["action"]
            visible = True if action == "evta_show" else False if action == "evta_hide" else not visible
    return visible


def reduce_vectors(times, values, tolerance):
    """Keep linear samples within a per-component absolute error budget."""
    keep = {0, len(times)-1}
    values = np.asarray(values)
    def visit(left, right):
        if right-left < 2:
            return
        u = (np.asarray(times[left+1:right])-times[left])/(times[right]-times[left])
        line = values[left] + u[:, None]*(values[right]-values[left])
        error = np.max(np.abs(values[left+1:right]-line)/np.asarray(tolerance), axis=1)
        local = int(np.argmax(error))
        if error[local] > 1.:
            index = left+1+local; keep.add(index); visit(left, index); visit(index, right)
    visit(0, len(times)-1)
    return sorted(keep)


def bind_body(rows, matinee, data, phase, baked_body):
    if phase in BONE_BINDINGS:
        base.BONE_MODELS.update(BONE_BINDINGS[phase])
        return
    model = base.wm.read_wmodel(baked_body, include_geometry=False, animation_names=[f"guardian.select.{phase}"])
    for actor in (12253, 12254, 12255, 12256, 12257, 12264, 12265):
        group = next(g for g in rows[data]["p"]["interpgroups"] if actor+1 in base.group_actor(rows, g, matinee))
        base.bind_bone_model(actor+1, model, group, f"guardian.select.{phase}", model_pre_scale=.0001)
    BONE_BINDINGS[phase] = dict(base.BONE_MODELS)


def lights(rows, matinee, data, duration):
    result = []
    for actor, row in rows.items():
        if row["cls"] != "pointlightmovable":
            continue
        p = row["p"]
        if p.get("basebonename") and p.get("base") not in base.BONE_MODELS:
            raise ValueError((actor-1, "Point light needs its actual baked bone donor"))
        component = rows[p["lightcomponent"]]["p"]
        group = next((g for g in rows[data]["p"]["interpgroups"] if actor in base.group_actor(rows, g, matinee)), None)
        tracks = base.active_tracks(rows, group)
        brightness = [t for t in tracks if t["cls"] == "interptrackfloatprop" and
                      t["p"].get("propertyname") == "pointlightcomponent0.brightness"]
        assert len(brightness) <= 1
        color = component.get("lightcolor", dict(r=255, g=255, b=255))
        # Follow extract_ue3_map_lights.colour: RGB bytes /255, alpha is only an
        # editor swatch. Brightness stays a separate intensity multiplier.
        rgb = [color[channel]/255. for channel in "rgb"]
        times = sorted(set(range(0, duration, 8)) | source_key_times(tracks, duration))
        values = []
        for ms in times:
            position, _ = base.world_pose(rows, group, actor, ms/1000., matinee, data)
            intensity = float(base.curve(brightness[0]["p"]["floattrack"]["points"], ms/1000., component.get("brightness", 1.))) if brightness else component.get("brightness", 1.)
            values.append([*position, *rgb, intensity])
        keep = reduce_vectors(times, values, [.001]*3+[.0001]*4)
        keys = [dict(timeMs=times[i], position=values[i][:3], color=values[i][3:6], brightness=values[i][6],
                     enabled=component.get("benabled", True) and visible_at(rows, group, actor, times[i]/1000.)) for i in keep]
        result.append(dict(lightId=f"{PREFIX}.light.{actor-1}", falloffExponent=component.get("falloffexponent", 2.),
            radiusMeters=component.get("radius", 1024.)*.01, keys=keys))
    return result


def material_tracks(rows, matinee, data, duration, manifest):
    components = {r["p"].get("skeletalmeshcomponent", r["p"].get("staticmeshcomponent")): i-1
                  for i, r in rows.items() if r["cls"] in ("skeletalmeshactormat", "efskeletalmeshactor", "interpactor")}
    entries = manifest.get("actors", manifest.get("entries", []))
    resources = {e["sourceActorIndex0"]: e["resources"] for e in entries}
    imports = manifest.get("sourceImports", {})
    del imports  # Material source is resolved by affected component+slot, not a name heuristic.
    projected = {}
    gaps = []
    for group in rows[data]["p"]["interpgroups"]:
        for track in base.active_tracks(rows, group):
            vector = track["cls"] == "interptrackvectormaterialparam"
            if not vector and track["cls"] != "interptrackfloatmaterialparam":
                continue
            p = track["p"]
            for material in p["materials"]:
                for affected in material.get("affectedmaterialrefs", []):
                    actor0 = components.get(affected["primitive"])
                    if actor0 is None:
                        # A shared MIC remembers other classes' primitive refs.
                        # They are outside this Matinee's source actor closure.
                        continue
                    specs = resources.get(actor0, [])
                    matching = [(i, spec) for i, spec in enumerate(specs) if spec.get("sourceMaterialSlot", spec.get("materialIndex")) == affected["materialindex"]]
                    if not matching:
                        gaps.append(dict(trackExportIndex0=track["index"]-1, actorExportIndex0=actor0,
                                         sourceMaterialSlot=affected["materialindex"], reason="No mapped component/material slot"))
                        continue
                    points = p["vectortrack" if vector else "floattrack"]["points"]
                    times = sorted(set(range(0, duration, 8)) | source_key_times([track], duration))
                    values = []
                    for ms in times:
                        value = base.curve(points, ms/1000., [0., 0., 0.] if vector else 0.)
                        values.append([*value.tolist(), 1.] if vector else [float(value), 0., 0., 0.])
                    keep = reduce_vectors(times, values, [1e-4]*4)
                    curve = dict(parameter=p["paramname"], keys=[dict(timeMs=times[i], value=values[i]) for i in keep])
                    for ordinal, spec in matching:
                        profile = spec["materialProfile"]
                        key = actor0, ordinal
                        if key not in projected:
                            projected[key] = dict(instanceId=f"world.sequence.instance.{PREFIX}.{'intro' if matinee==703 else 'loop'}.a{actor0}.p{ordinal}",
                                slotId="actor", materialName=profile["materialName"], family=profile["family"],
                                parameters=copy.deepcopy(profile["parameters"]), curves=[])
                        projected[key]["curves"].append(curve)
    return list(projected.values()), gaps


def transform_keys(rows, group, actor, matinee, data, duration):
    tracks = base.active_tracks(rows, group)
    times = sorted(set(range(0, duration, 8)) | source_key_times(tracks, duration))
    scale = base.vec(rows[actor]["p"].get("drawscale3d"), (1., 1., 1.))[[0, 2, 1]] * rows[actor]["p"].get("drawscale", 1.)
    samples = []
    for ms in times:
        pos, rot = base.world_pose(rows, group, actor, ms/1000., matinee, data)
        q = Rotation.from_matrix(rot).as_quat()
        samples.append((pos, q, visible_at(rows, group, actor, ms/1000.)))
    keep = {0, len(times)-1}
    # Adaptive chord reduction verifies every 8 ms source sample, with millimetre
    # translation and 0.05 degree rotation tolerances. Visibility is stepped.
    def reduce(a, b):
        pa, qa, va = samples[a]; pb, qb, vb = samples[b]
        worst, at = 1., None
        for i in range(a+1, b):
            p, q, visible = samples[i]
            u = (times[i]-times[a])/(times[b]-times[a])
            iq = base.slerp(qa, qb, u)
            angle = math.degrees(2*math.acos(min(1., abs(float(q@iq)))))
            error = max(float(np.linalg.norm(p-(pa*(1-u)+pb*u)))/.001, angle/.05,
                        2. if visible != va else 0.)
            if error > worst:
                worst, at = error, i
        if at is not None:
            keep.add(at); reduce(a, at); reduce(at, b)
    reduce(0, len(times)-1)
    assert len(keep) <= 4096, (actor-1, "Transform track exceeds WorldSequence contract", len(keep))
    return [dict(timeMs=times[i], positionOffset=samples[i][0].tolist(),
        rotationQuaternion=samples[i][1].tolist(), scaleMultiplier=scale.tolist(), visible=samples[i][2]) for i in sorted(keep)]


def project_resources(rows, manifest, infos, baked_body=None):
    world = dict(schema="lostark.world-sequences", formatVersion=3, areaId=AREA, revision=1,
                 objectResources=[], templates=[], instances=[])
    instance_ids = {name: [] for name, _ in PHASES}
    entries = manifest.get("actors", manifest.get("entries", []))
    for entry in entries:
        actor = entry["sourceActorIndex0"] + 1
        if rows[actor]["p"].get("basebonename") and rows[actor]["p"].get("base") not in base.BONE_MODELS:
            raise ValueError((actor-1, "Bone attachment must bind an admitted baked donor before projection"))
        for ordinal, spec in enumerate(entry["resources"]):
            object_id = f"world.object.{PREFIX}.a{actor-1}.p{ordinal}"
            resource = dict(objectId=object_id, displayName=f"Guardian actor {actor-1} part {ordinal}",
                modelAssetId=spec["modelAssetId"], anchorKind="WORLD", diffuseTextureAssetId="",
                modelPreScale=spec["modelPreScale"], animated=spec.get("animated", True), scale=[1, 1, 1],
                sequenceInstanceId="", defaultMotionInstanceId="")
            for field in ("animationSetAssetId", "materialProfile", "materialSourceModelAssetId", "mapMaterialBindings"):
                if field in spec and spec[field] not in ('', None):
                    resource[field] = spec[field]
            world["objectResources"].append(resource)
            for phase, _ in PHASES:
                matinee, data, duration = infos[phase]
                if baked_body:
                    bind_body(rows, matinee, data, phase, baked_body)
                group = next((g for g in rows[data]["p"]["interpgroups"] if actor in base.group_actor(rows, g, matinee)), None)
                identity = f"{PREFIX}.{phase}.a{actor-1}.p{ordinal}"
                instance_id, sequence_id = "world.sequence.instance."+identity, "sequence."+identity
                keys = transform_keys(rows, group, actor, matinee, data, duration)
                template = dict(sequenceId=sequence_id, displayName=identity, category="World", durationMs=duration,
                    interpolation="LINEAR", tracks=[dict(slotId="actor", keys=keys)], animationTracks=[])
                # A skinned mesh can retain its rest palette without playing a
                # clip: the weapon follows its baked hand-bone WORLD transform.
                # Only an admitted donor or explicit embedded clips authorize
                # an animation track; model type alone does not imply clips.
                if resource["animated"] and (resource.get("animationSetAssetId") or spec.get("clips")):
                    clips = spec.get("clips", {"intro": "guardian.select.intro", "loop": "guardian.select.loop"})
                    template["animationTracks"] = [dict(slotId="actor", startMs=0, clipName=clips[phase],
                        playbackRate=1., loop=False, holdLastFrame=True)]
                instance = dict(instanceId=instance_id, templateId=sequence_id, enabled=True, startDelayMs=0,
                    playbackSpeed=1., anchorKind="WORLD", position=[0, 0, 0], motionEnd="HOLD", nextMotionId="",
                    bindings=[dict(slotId="actor", targetKind="OBJECT_RESOURCE", targetId=object_id)])
                world["templates"].append(template); world["instances"].append(instance)
                instance_ids[phase].append(instance_id)
                if phase == "intro":
                    resource["defaultMotionInstanceId"] = instance_id
    return world, instance_ids


def effect_clocks(rows, document, library, phase):
    """Original static PSC roots and particle-age clocks; no action anchors."""
    metadata = {row['assetId']: row for row in library['assets']}
    imports = {row['reference']: row['path'] for row in document['imports']}
    intro_m, intro_d, intro_duration = phase_info(rows, PHASES[0][1])
    matinee, data, duration = phase_info(rows, dict(PHASES)[phase])
    result = []
    for actor0 in (1716, 1718, 1719, 1720, 1721):
        actor = actor0 + 1
        p = rows[actor]['p']; component = rows[p['particlesystemcomponent']]['p']
        asset = 'effect.classselect.guardianknight.' + imports[component['template']]
        assert asset in metadata and not p.get('base') and not p.get('basebonename')
        group = next(g for g in rows[intro_d]['p']['interpgroups'] if actor in base.group_actor(rows, g, intro_m))
        tracks = base.active_tracks(rows, group)
        assert not any(t['cls'] == 'interptrackmove' for t in tracks)
        toggles = [key for t in tracks if t['cls'] == 'interptracktoggle' for key in t['p'].get('toggletrack', [])]
        on = [key['time'] for key in toggles if key['toggleaction'] == 'etta_on']
        activated = 0. if component.get('bautoactivate', True) else on[0]
        off = [key['time'] for key in toggles if key['toggleaction'] == 'etta_off' and key['time'] > activated]
        # Zoom blur is one 1-second burst; it has expired before Matinee74.
        if phase == 'loop' and off:
            continue
        shown = [key['time'] for t in tracks if t['cls'] == 'interptrackvisibility'
                 for key in t['p'].get('visibilitytrack', []) if key['action'] == 'evta_show']
        visible_start = max(activated, shown[0] if shown else 0.)
        slomo = [t for t in tracks if t['cls'] == 'efinterptrackparticleslomo']
        points = slomo[0]['p']['floattrack']['points'] if slomo else []
        def age(seconds):
            seconds = max(activated, seconds)
            boundaries = sorted({activated, seconds} | {k['inval'] for k in points if activated < k['inval'] < seconds})
            return sum(quad(lambda t: float(base.curve(points, t, 1.)), a, b, epsabs=1e-10)[0]
                       for a, b in zip(boundaries, boundaries[1:])) * 1000.
        if phase == 'intro':
            times = sorted({0., float(duration), activated*1000., visible_start*1000.} |
                           {k['inval']*1000. for k in points if 0 <= k['inval']*1000. <= duration} |
                           {float(t) for t in range(round(activated*1000.), min(duration, round(points[-1]['inval']*1000.))+1, 4)}) if points else sorted({0., float(duration), activated*1000.})
            ages = [age(ms/1000.) for ms in times]
            keep = reduce_vectors(times, [[v] for v in ages], [.01])
            clocks = [dict(timeMs=times[i], sourceMs=ages[i]) for i in keep]
            start = visible_start*1000.; end = min(duration, off[0]*1000.) if off else duration
        else:
            rate = float(base.curve(points, intro_duration/1000., 1.))
            first = age(intro_duration/1000.)
            clocks = [dict(timeMs=0, sourceMs=first), dict(timeMs=duration, sourceMs=first+duration*rate)]
            start, end = 0, duration
        position, rotation = base.world_pose(rows, group, actor, 0., intro_m, intro_d)
        scale = base.vec(p.get('drawscale3d'), (1., 1., 1.))[[0, 2, 1]] * p.get('drawscale', 1.)
        matrix = np.eye(4); matrix[:3, :3] = np.diag(scale) @ rotation.T; matrix[3, :3] = position
        row = dict(effectId=f'{PREFIX}.psc.{p["particlesystemcomponent"]-1}', assetId=asset,
            rootWorld=matrix.reshape(-1).tolist(), clockKeys=clocks, startMs=start, endMs=end,
            sourceLoopEndMs=clocks[-1]['sourceMs'] if metadata[asset]['nativeInfinite'] or metadata[asset].get('sourceZeroLifetime') else 0.,
            loopAgeDeltaMs=clocks[-1]['sourceMs']-clocks[0]['sourceMs'] if phase == 'loop' else 0.)
        result.append(row)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--actor-manifest", type=Path)
    parser.add_argument("--baked-body", type=Path)
    parser.add_argument("--effect-library", type=Path)
    parser.add_argument("--effects-only", action="store_true")
    args = parser.parse_args()
    assert ROOT / "Data" not in args.output.resolve().parents and ROOT / "Client/Bin" not in args.output.resolve().parents
    document = read(args.source); rows = rows_from(document)
    if args.effects_only:
        assert args.effect_library
        target = args.output / 'ClassSelection.cinematics.json'
        candidate = read(target)
        for phase, _ in PHASES:
            candidate['scenes'][0][phase]['effects'] = effect_clocks(rows, document, read(args.effect_library), phase)
        write(target, candidate)
        print('Updated original PSC clocks', [len(candidate['scenes'][0][phase]['effects']) for phase, _ in PHASES])
        return
    infos, phases, timelines, clocks, material_gaps = {}, {}, {}, {}, {}
    for phase, matinee0 in PHASES:
        infos[phase] = phase_info(rows, matinee0)
        matinee, data, duration = infos[phase]
        if args.baked_body:
            bind_body(rows, matinee, data, phase, args.baked_body)
        clock, clocks[phase] = clock_keys(rows, data, duration)
        phases[phase] = dict(durationMs=duration, instanceIds=[], cameras=cameras(rows, matinee, data, duration, phase))
        if clock:
            phases[phase]["clockKeys"] = clock
        if args.baked_body:
            phases[phase]["lights"] = lights(rows, matinee, data, duration)
        if args.actor_manifest:
            phases[phase]["materialTracks"], material_gaps[phase] = material_tracks(rows, matinee, data, duration, read(args.actor_manifest))
        if args.effect_library:
            phases[phase]['effects'] = effect_clocks(rows, document, read(args.effect_library), phase)
        timelines[phase] = timeline(rows, matinee, data, duration)
    if args.actor_manifest:
        world, ids = project_resources(rows, read(args.actor_manifest), infos, args.baked_body)
        for phase in phases:
            phases[phase]["instanceIds"] = ids[phase]
        write(args.output / f"{AREA}.worldsequences.json", world)
    scene = dict(schema="lostark.class-selection-cinematics", formatVersion=1, areaId=AREA,
        scenes=[dict(classId="GUARDIANKNIGHT", sceneId=PREFIX, **phases)])
    write(args.output / "ClassSelection.cinematics.json", scene)
    write(args.output / "guardian-native-timeline.json", dict(schema="lostark.source-matinee-timeline", formatVersion=1,
        source=document["source"], imports=document["imports"], phases=timelines))
    receipt = dict(sourceSha256=hashlib.sha256(args.source.read_bytes()).hexdigest(), source=document["source"],
        coordinateConvention="UE world X,Z,-Y in metres; existing cinematic world_pose attachment evaluation",
        timeDomain="durationMs/cameras/WorldSequence keys are source time; clockKeys map wall time to source time",
        clocks=clocks, cameraKeyCounts={k: sum(len(c["keys"]) for c in v["cameras"]) for k, v in phases.items()},
        installed=False, runtimeReady=False, materialProjectionGaps=material_gaps,
        unconsumedSourceFeatures=["CameraShake", "DOF and bloom float properties", "18 cinematic static prop geometry/materials",
            "AkEvent audio", "Particle shader/carrier admission requires effect-library nativeReady and runtime validation"])
    write(args.output / "projection-receipt.json", receipt)
    print(json.dumps({k: receipt[k] for k in ("clocks", "cameraKeyCounts", "installed", "runtimeReady")}))


if __name__ == "__main__":
    main()

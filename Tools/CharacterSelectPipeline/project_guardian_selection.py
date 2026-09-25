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
CLASS_ID = "GUARDIANKNIGHT"
DISPLAY_NAME = "Guardian"
PROFILE = {}
ACTOR_SCOPE = None
BONE_MANIFEST = None
BONE_MANIFEST_ROOT = None
PHASE_ROWS = {}


def configure_profile(profile):
    """Select a source-verified category without adding another runtime format."""
    global PROFILE, CLASS_ID, DISPLAY_NAME, PREFIX, PHASES, ACTOR_SCOPE
    PROFILE = profile
    CLASS_ID = profile["classId"]
    DISPLAY_NAME = profile.get("displayName", CLASS_ID)
    PREFIX = profile["prefix"]
    PHASES = tuple((name, int(profile["phases"][name])) for name in ("intro", "loop"))
    assert PREFIX.startswith("classselect.") and len({v for _, v in PHASES}) == 2
    ACTOR_SCOPE = None
    BONE_BINDINGS.clear()
    PHASE_ROWS.clear()
    base.BONE_MODELS.clear()
    base.BONE_MODEL_PRE_SCALES.clear()


def actor_scope(rows):
    actors = set()
    for _, index in PHASES:
        matinee, data, _ = phase_info(rows, index)
        actors.update(a for g in rows[data]["p"]["interpgroups"] for a in base.group_actor(rows, g, matinee))
    # Untracked attached children (not every light has an InterpGroup) belong to
    # their actual source parent, not to every category in the shared package.
    changed = True
    while changed:
        previous = len(actors)
        actors.update(i for i, r in rows.items() if r["p"].get("base") in actors)
        actors.update(rows[a]["p"]["base"] for a in tuple(actors) if rows[a]["p"].get("base") in rows)
        changed = len(actors) != previous
    return actors


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


def source_identity(document):
    return document.get("source", {key: document.get(key) for key in
        ("logicalPackage", "physicalPackage", "packageSha256", "byteSize")})


def source_imports(document):
    values = document["imports"]
    return {int(k): v for k, v in values.items()} if isinstance(values, dict) else {r["reference"]: r["path"] for r in values}


def rotation_property(client_rotation):
    yaw, pitch, roll = Rotation.from_matrix(base.BASIS.T @ client_rotation @ base.BASIS).as_euler("ZYX", degrees=True)
    return dict(degrees=dict(roll=-float(roll), pitch=-float(pitch), yaw=float(yaw)))


def loop_initial_rows(rows, infos):
    """Carry source actor state across the two Matinees, including omitted props."""
    intro_m, intro_d, duration = infos["intro"]
    loop_m, loop_d, _ = infos["loop"]
    result = copy.deepcopy(rows)
    groups = lambda m, d: {a: g for g in rows[d]["p"]["interpgroups"] for a in base.group_actor(rows, g, m)}
    before, after = groups(intro_m, intro_d), groups(loop_m, loop_d)
    seconds = duration / 1000.
    for actor in ACTOR_SCOPE:
        p = rows[actor]["p"]
        target = result[actor]["p"]
        group = before.get(actor)
        tracks = base.active_tracks(rows, group)
        following = base.active_tracks(rows, after.get(actor))
        target["bhidden"] = not visible_at(rows, group, actor, seconds)
        for track in tracks:
            prop = track["p"]
            name = prop.get("propertyname", "").rsplit(".", 1)[-1]
            if name == "drawscale" and prop.get("floattrack", {}).get("points"):
                target["drawscale"] = float(base.curve(prop["floattrack"]["points"], seconds, p.get("drawscale", 1.)))
            component_ref = p.get("lightcomponent")
            if component_ref:
                component = result[component_ref]["p"]
                if name in ("brightness", "radius") and prop.get("floattrack", {}).get("points"):
                    component[name] = float(base.curve(prop["floattrack"]["points"], seconds, component.get(name, 1.)))
                elif name == "lightcolor" and prop.get("vectortrack", {}).get("points"):
                    color = base.curve(prop["vectortrack"]["points"], seconds, [1., 1., 1.])
                    component["lightcolor"] = dict(zip("rgb", color*255.))
        if not any(t["cls"] == "interptrackmove" for t in tracks) or any(t["cls"] == "interptrackmove" for t in following):
            continue
        # Source-only camera probes do not yet own skeleton donors. They remain
        # explicitly unready; final projection requires the actual bone manifest.
        if p.get("base") and p.get("basebonename") and p.get("base") not in base.BONE_MODELS:
            continue
        position, rotation = base.world_pose(rows, group, actor, seconds, intro_m, intro_d)
        target["location"] = dict(zip("xyz", base.BASIS.T @ position * 100.))
        target["rotation"] = rotation_property(rotation)
        if p.get("base") not in rows:
            continue
        parent = p["base"]
        pp, pr = base.world_pose(rows, before.get(parent), parent, seconds, intro_m, intro_d)
        if p.get("basebonename"):
            model, _, animation, _ = base.BONE_MODELS[parent]
            pose = base.pose_sample(model, animation, min(seconds, animation.duration_ticks/animation.ticks_per_second))
            local = [base.wm.affine_matrix(s, q, t) for t, q, s in pose]
            index = next(i for i, b in enumerate(model.skeleton_bones) if b.name.casefold() == p["basebonename"].casefold())
            bone = np.asarray(base.wm.combined_transforms(model.skeleton_bones, local)[index]).reshape(4, 4)
            bone_rotation = bone[:3, :3].T.copy()
            bone_rotation /= np.linalg.norm(bone_rotation, axis=0)
            pp += pr @ (bone[3, :3] * base.BONE_MODEL_PRE_SCALES[parent] * rows[parent]["p"].get("drawscale", 1.))
            pr = pr @ bone_rotation
        target["relativelocation"] = dict(zip("xyz", base.BASIS.T @ (pr.T @ (position-pp)) * 100.))
        target["relativerotation"] = rotation_property(pr.T @ rotation)
    return result


def carry_materials(phases):
    """Unmentioned MIC fields retain the last value when the loop starts."""
    following = phases["loop"].setdefault("materialTracks", [])
    for source in phases["intro"].get("materialTracks", []):
        identity = source["instanceId"].replace(f"{PREFIX}.intro.", f"{PREFIX}.loop.", 1)
        target = next((t for t in following if t["instanceId"] == identity and t["slotId"] == source["slotId"]), None)
        if target is None:
            target = copy.deepcopy(source)
            target.update(instanceId=identity, curves=[])
            following.append(target)
        present = {c["parameter"] for c in target["curves"]}
        for curve in source["curves"]:
            if curve["parameter"] not in present:
                value = curve["keys"][-1]["value"]
                target["curves"].append(dict(parameter=curve["parameter"], keys=[
                    dict(timeMs=0, value=value), dict(timeMs=phases["loop"]["durationMs"], value=value)]))


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
                    if "time" in point or point.get("interpmode") == "cim_constant":
                        result.update(t for t in (math.floor(seconds*1000), math.ceil(seconds*1000)) if 0 <= t <= duration)
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
        # Warrior/Fighter contain an empty FOV placeholder followed by the
        # populated property track. Empty UE float tracks do not set a value.
        fovs = [t for t in camera_tracks if t["cls"] == "interptrackfloatprop" and
                t["p"].get("propertyname") == "fovangle" and t["p"].get("floattrack", {}).get("points")]
        # A group applies property tracks in its serialized order. If two
        # populated setters address the same field, the final setter wins.
        placed_fov = rows[actor]["p"].get("fovangle", 90.)
        times = {start, end} | set(range(start, end, 8))
        times |= {t for t in source_key_times(tracks, duration) if start <= t <= end}
        keys = []
        identity = f"{PREFIX}.{phase}.camera.{index + 1}"
        for ms in sorted(times):
            eye, rotation = base.world_pose(rows, group, actor, ms / 1000., matinee, data)
            forward = rotation @ np.array([1., 0., 0.])
            up = rotation @ np.array([0., 1., 0.])
            fov = float(base.curve(fovs[-1]["p"]["floattrack"]["points"], ms / 1000., placed_fov)) if fovs else float(placed_fov)
            assert np.isfinite([*eye, *forward, *up, fov]).all()
            assert 1. < fov < 179. and abs(float(forward @ up)) < 1e-6
            keys.append(dict(keyId=f"{identity}.k{len(keys)}", timeMs=ms-start,
                eye=eye.tolist(), lookAt=(eye+forward).tolist(), up=up.tolist(), fovDegrees=fov))
        assert len(keys) <= 4096
        if PROFILE.get("compactCameraKeys", True):
            values = [k["eye"] + k["lookAt"] + k["up"] + [k["fovDegrees"]] for k in keys]
            keep = reduce_vectors([k["timeMs"] for k in keys], values, [.0001]*6 + [.00001]*3 + [.0001])
            keys = [dict(keys[i], keyId=f"{identity}.k{ordinal}") for ordinal, i in enumerate(keep)]
        result.append(dict(cameraId=identity, displayName=f"{DISPLAY_NAME} {phase} {cut['targetcamgroup']}",
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
        if b <= a:
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
    input_count = len(result)
    result = admit_clock_keys(result)
    return result, dict(sourceDurationMs=duration, wallDurationMs=wall*1000.,
        slomoTrackExportIndex0=tracks[0]["index"]-1, integration="integral ds / source TimeDilation(s)",
        integratedSamples=input_count, admittedSamples=len(result), maxReductionSourceErrorMs=.001,
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


def admit_clock_keys(keys):
    """Respect native phase admission without discarding slow-motion timing."""
    if len(keys) <= 4096:
        return keys
    keep = reduce_vectors([k["timeMs"] for k in keys], [[k["sourceMs"]] for k in keys], [.001])
    result = [keys[i] for i in keep]
    if len(result) > 4096:
        raise ValueError("Movie clock exceeds native 4096 samples after bounded 0.001 ms reduction")
    return result


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
    if BONE_MANIFEST is not None:
        bind_actors(rows, matinee, data, phase)
        return
    if phase in BONE_BINDINGS:
        base.BONE_MODELS.update(BONE_BINDINGS[phase])
        return
    model = base.wm.read_wmodel(baked_body, include_geometry=False, animation_names=[f"guardian.select.{phase}"])
    for actor in (12253, 12254, 12255, 12256, 12257, 12264, 12265):
        group = next(g for g in rows[data]["p"]["interpgroups"] if actor+1 in base.group_actor(rows, g, matinee))
        base.bind_bone_model(actor+1, model, group, f"guardian.select.{phase}", model_pre_scale=.0001)
    BONE_BINDINGS[phase] = dict(base.BONE_MODELS)


def bind_actors(rows, matinee, data, phase):
    base.BONE_MODELS.clear()
    base.BONE_MODEL_PRE_SCALES.clear()
    if phase not in BONE_BINDINGS:
        for binding in BONE_MANIFEST["boneBindings"]:
            actor = binding["sourceActorIndex0"] + 1
            clip = binding["clips"].get(phase)
            if clip is None:
                continue
            path = Path(binding["donorModel"])
            if not path.is_absolute():
                path = BONE_MANIFEST_ROOT / path
            model = base.wm.read_wmodel(path, include_geometry=False, animation_names=[clip])
            group = next((g for g in rows[data]["p"]["interpgroups"] if actor in base.group_actor(rows, g, matinee)), None)
            base.bind_bone_model(actor, model, group, clip, model_pre_scale=binding["modelPreScale"])
        BONE_BINDINGS[phase] = (dict(base.BONE_MODELS), dict(base.BONE_MODEL_PRE_SCALES))
    models, scales = BONE_BINDINGS[phase]
    base.BONE_MODELS.update(models)
    base.BONE_MODEL_PRE_SCALES.update(scales)
    for actor in ACTOR_SCOPE or ():
        p = rows[actor]["p"]
        if p.get("base") and p.get("basebonename") and p.get("base") not in base.BONE_MODELS:
            raise ValueError((actor-1, "Source bone attachment has no admitted donor", p.get("base"), p["basebonename"]))


def lights(rows, matinee, data, duration):
    result = []
    for actor, row in rows.items():
        if ACTOR_SCOPE is not None and actor not in ACTOR_SCOPE:
            continue
        if row["cls"] not in ("pointlightmovable", "dominantpointlight"):
            continue
        p = row["p"]
        if p.get("base") and p.get("basebonename") and p.get("base") not in base.BONE_MODELS:
            raise ValueError((actor-1, "Point light needs its actual baked bone donor"))
        component = rows[p["lightcomponent"]]["p"]
        group = next((g for g in rows[data]["p"]["interpgroups"] if actor in base.group_actor(rows, g, matinee)), None)
        tracks = base.active_tracks(rows, group)
        def property_track(name, channel):
            matches = [t for t in tracks if t["p"].get("propertyname", "").rsplit(".", 1)[-1] == name and
                       t["p"].get(channel, {}).get("points")]
            return matches[-1]["p"][channel]["points"] if matches else []
        brightness = property_track("brightness", "floattrack")
        radius = property_track("radius", "floattrack")
        color_curve = property_track("lightcolor", "vectortrack")
        color = component.get("lightcolor", dict(r=255, g=255, b=255))
        # Follow extract_ue3_map_lights.colour: RGB bytes /255, alpha is only an
        # editor swatch. Brightness stays a separate intensity multiplier.
        rgb = [color[channel]/255. for channel in "rgb"]
        times = sorted(set(range(0, duration, 8)) | source_key_times(tracks, duration))
        values = []
        for ms in times:
            position, _ = base.world_pose(rows, group, actor, ms/1000., matinee, data)
            intensity = float(base.curve(brightness, ms/1000., component.get("brightness", 1.)))
            # LightColor is the source byte RGB property, not an unbounded MIC vector.
            color_value = np.clip(base.curve(color_curve, ms/1000., rgb), 0., 1.)
            range_value = float(base.curve(radius, ms/1000., component.get("radius", 1024.))) * .01
            values.append([*position, *color_value, intensity, range_value])
        keep = set(reduce_vectors(times, values, [.001]*3+[.0001]*4+[.001]))
        visibility = [visible_at(rows, group, actor, t/1000.) and values[i][6] > 0. and values[i][7] > 0.
                      for i, t in enumerate(times)]
        for i in range(1, len(times)):
            if visibility[i] != visibility[i-1]:
                keep.update((i-1, i))
        # A source light with nonpositive energy/radius contributes no light.
        # Keep it disabled; the positive radius sentinel satisfies the runtime
        # descriptor without turning that disabled source sample into a light.
        keys = [dict(timeMs=times[i], position=values[i][:3], color=values[i][3:6], brightness=max(0., values[i][6]),
                     enabled=component.get("benabled", True) and visibility[i],
                     **(dict(radiusMeters=max(1e-6, values[i][7])) if PROFILE and radius else {})) for i in sorted(keep)]
        result.append(dict(lightId=f"{PREFIX}.light.{actor-1}", falloffExponent=component.get("falloffexponent", 2.),
            radiusMeters=max(1e-6, component.get("radius", 1024.)*.01), keys=keys))
    return result


def material_tracks(rows, matinee, data, duration, manifest):
    components = {r["p"].get("skeletalmeshcomponent", r["p"].get("staticmeshcomponent")): i-1
                  for i, r in rows.items() if (r["p"].get("skeletalmeshcomponent") or r["p"].get("staticmeshcomponent"))
                  and (ACTOR_SCOPE is None or i in ACTOR_SCOPE)}
    entries = manifest.get("actors", manifest.get("entries", []))
    resources = {e["sourceActorIndex0"]: e["resources"] for e in entries}
    component_resources = {}
    for entry in entries:
        actor0 = entry["sourceActorIndex0"]
        actor = rows[actor0+1]["p"]
        primary = actor.get("skeletalmeshcomponent", actor.get("staticmeshcomponent"))
        for ordinal, spec in enumerate(entry["resources"]):
            component = spec.get("sourceComponentIndex0")
            component = component+1 if component is not None else primary
            if component is not None:
                component_resources.setdefault(component, []).append((actor0, ordinal, spec))
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
            points = p.get("vectortrack" if vector else "floattrack", {}).get("points", [])
            if not points:
                continue  # An empty serialized setter does not change the MIC.
            for material in p["materials"]:
                for affected in material.get("affectedmaterialrefs", []):
                    exact = component_resources.get(affected["primitive"], [])
                    actor0 = exact[0][0] if exact else components.get(affected["primitive"])
                    if actor0 is None:
                        # A shared MIC remembers other classes' primitive refs.
                        # They are outside this Matinee's source actor closure.
                        continue
                    matching = [(owner, ordinal, spec) for owner, ordinal, spec in exact
                        if spec.get("sourceMaterialSlot", spec.get("materialIndex")) == affected["materialindex"]]
                    if not matching:
                        gaps.append(dict(trackExportIndex0=track["index"]-1, actorExportIndex0=actor0,
                                         sourceMaterialSlot=affected["materialindex"], reason="No mapped component/material slot"))
                        continue
                    times = sorted(set(range(0, duration, 8)) | source_key_times([track], duration))
                    values = []
                    for ms in times:
                        value = base.curve(points, ms/1000., [0., 0., 0.] if vector else 0.)
                        values.append([*value.tolist(), 1.] if vector else [float(value), 0., 0., 0.])
                    keep = reduce_vectors(times, values, [1e-4]*4)
                    curve = dict(parameter=p["paramname"], keys=[dict(timeMs=times[i], value=values[i]) for i in keep])
                    for actor0, ordinal, spec in matching:
                        if "materialProfile" not in spec:
                            gaps.append(dict(trackExportIndex0=track["index"]-1, actorExportIndex0=actor0,
                                sourceMaterialSlot=affected["materialindex"], reason="Material curve needs an admitted animated material profile"))
                            continue
                        profile = spec["materialProfile"]
                        key = actor0, ordinal
                        if key not in projected:
                            phase = next(name for name, index in PHASES if index+1 == matinee)
                            projected[key] = dict(instanceId=f"world.sequence.instance.{PREFIX}.{phase}.a{actor0}.p{ordinal}",
                                slotId="actor", materialName=profile["materialName"], family=profile["family"],
                                parameters=copy.deepcopy(profile["parameters"]), curves=[])
                        # Matinee evaluates setters in serialized order. Keep
                        # the last nonempty setter for the same component slot.
                        projected[key]["curves"] = [c for c in projected[key]["curves"] if c["parameter"] != curve["parameter"]] + [curve]
    return list(projected.values()), gaps


def transform_keys(rows, group, actor, matinee, data, duration):
    tracks = base.active_tracks(rows, group)
    times = sorted(set(range(0, duration, 8)) | source_key_times(tracks, duration))
    scale3 = base.vec(rows[actor]["p"].get("drawscale3d"), (1., 1., 1.))[[0, 2, 1]]
    scale_tracks = [t for t in tracks if t["cls"] == "interptrackfloatprop" and t["p"].get("propertyname") == "drawscale"
                    and t["p"].get("floattrack", {}).get("points")]
    assert len(scale_tracks) <= 1
    samples = []
    for ms in times:
        pos, rot = base.world_pose(rows, group, actor, ms/1000., matinee, data)
        q = Rotation.from_matrix(rot).as_quat()
        if q[3] < 0.:
            q = -q
        scalar = float(base.curve(scale_tracks[0]["p"]["floattrack"]["points"], ms/1000., 1.)) if scale_tracks else rows[actor]["p"].get("drawscale", 1.)
        samples.append((pos, q, visible_at(rows, group, actor, ms/1000.), scale3 * scalar))
    keep = {0, len(times)-1}
    # Adaptive chord reduction verifies every 8 ms source sample, with millimetre
    # translation and 0.05 degree rotation tolerances. Visibility is stepped.
    def reduce(a, b):
        pa, qa, va, sa = samples[a]; pb, qb, vb, sb = samples[b]
        worst, at = 1., None
        for i in range(a+1, b):
            p, q, visible, scale = samples[i]
            u = (times[i]-times[a])/(times[b]-times[a])
            iq = base.slerp(qa, qb, u)
            angle = math.degrees(2*math.acos(min(1., abs(float(q@iq)))))
            error = max(float(np.linalg.norm(p-(pa*(1-u)+pb*u)))/.001, angle/.05,
                        float(np.max(np.abs(scale-(sa*(1-u)+sb*u))))/.0001, 2. if visible != va else 0.)
            if error > worst:
                worst, at = error, i
        if at is not None:
            keep.add(at); reduce(a, at); reduce(at, b)
    reduce(0, len(times)-1)
    assert len(keep) <= 4096, (actor-1, "Transform track exceeds WorldSequence contract", len(keep))
    return [dict(timeMs=times[i], positionOffset=samples[i][0].tolist(),
        rotationQuaternion=samples[i][1].tolist(), scaleMultiplier=samples[i][3].tolist(), visible=samples[i][2]) for i in sorted(keep)]


def project_resources(rows, manifest, infos, baked_body=None):
    world = dict(schema="lostark.world-sequences", formatVersion=3, areaId=AREA, revision=1,
                 objectResources=[], templates=[], instances=[])
    instance_ids = {name: [] for name, _ in PHASES}
    entries = manifest.get("actors", manifest.get("entries", []))
    for entry in entries:
        actor = entry["sourceActorIndex0"] + 1
        if rows[actor]["p"].get("base") and rows[actor]["p"].get("basebonename") and rows[actor]["p"].get("base") not in base.BONE_MODELS:
            raise ValueError((actor-1, "Bone attachment must bind an admitted baked donor before projection"))
        for ordinal, spec in enumerate(entry["resources"]):
            object_id = f"world.object.{PREFIX}.a{actor-1}.p{ordinal}"
            resource = dict(objectId=object_id, displayName=f"{DISPLAY_NAME} actor {actor-1} part {ordinal}",
                modelAssetId=spec["modelAssetId"], anchorKind="WORLD", diffuseTextureAssetId="",
                modelPreScale=spec["modelPreScale"], animated=spec.get("animated", True), scale=[1, 1, 1],
                sequenceInstanceId="", defaultMotionInstanceId="")
            for field in ("animationSetAssetId", "materialProfile", "materialSourceModelAssetId", "mapMaterialBindings"):
                if field in spec and spec[field] not in ('', None):
                    resource[field] = spec[field]
            if resource.get("animationSetAssetId") == resource["modelAssetId"]:
                if not spec.get("clips"):
                    raise ValueError((actor-1, "Embedded animation requires explicit admitted clip names"))
                # This WModel already carries the baked clips. Attaching the same
                # model again would correctly fail CModel's duplicate-name check.
                del resource["animationSetAssetId"]
            world["objectResources"].append(resource)
            for phase, _ in PHASES:
                matinee, data, duration = infos[phase]
                phase_rows = PHASE_ROWS.get(phase, rows)
                if baked_body or BONE_MANIFEST is not None:
                    bind_body(phase_rows, matinee, data, phase, baked_body)
                group = next((g for g in rows[data]["p"]["interpgroups"] if actor in base.group_actor(rows, g, matinee)), None)
                identity = f"{PREFIX}.{phase}.a{actor-1}.p{ordinal}"
                instance_id, sequence_id = "world.sequence.instance."+identity, "sequence."+identity
                keys = transform_keys(phase_rows, group, actor, matinee, data, duration)
                template = dict(sequenceId=sequence_id, displayName=identity, category="World", durationMs=duration,
                    interpolation="LINEAR", tracks=[dict(slotId="actor", keys=keys)], animationTracks=[])
                # A scene's authored cue has exactly one owner, even when its
                # source avatar is split into many geometry/material resources.
                if actor == entries[0]["sourceActorIndex0"]+1 and ordinal == 0:
                    sounds = [s for s in manifest.get("sounds", []) if s["phase"] == phase]
                    if sounds:
                        fields = ("soundTrackId", "assetId", "startMs", "durationMs", "volume")
                        template["soundTracks"] = [{k: sound[k] for k in fields} for sound in sounds]
                # A skinned mesh can retain its rest palette without playing a
                # clip: the weapon follows its baked hand-bone WORLD transform.
                # Only an admitted donor or explicit embedded clips authorize
                # an animation track; model type alone does not imply clips.
                if resource["animated"] and (resource.get("animationSetAssetId") or spec.get("clips")):
                    clips = spec.get("clips", {"intro": "guardian.select.intro", "loop": "guardian.select.loop"})
                    if PROFILE and not spec.get("clips"):
                        raise ValueError((actor-1, "New category requires explicit admitted clip names"))
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
    imports = source_imports(document)
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
    global ACTOR_SCOPE, BONE_MANIFEST, BONE_MANIFEST_ROOT
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--actor-manifest", type=Path)
    parser.add_argument("--baked-body", type=Path)
    parser.add_argument("--effect-library", type=Path)
    parser.add_argument("--effects-only", action="store_true")
    parser.add_argument("--profile", type=Path, help="Verified category, phase export indices and background Area")
    parser.add_argument("--bone-manifest", type=Path, help="Actual baked actor donors for source bone attachments")
    args = parser.parse_args()
    assert ROOT / "Data" not in args.output.resolve().parents and ROOT / "Client/Bin" not in args.output.resolve().parents
    document = read(args.source); rows = rows_from(document)
    if args.profile:
        configure_profile(read(args.profile))
        expected = PROFILE.get("sourcePackageSha256")
        if expected and document.get("packageSha256", source_identity(document).get("packageSha256")) != expected:
            raise ValueError("Source package differs from reviewed category profile")
        ACTOR_SCOPE = actor_scope(rows)
    if args.bone_manifest:
        BONE_MANIFEST = read(args.bone_manifest)
        BONE_MANIFEST_ROOT = args.bone_manifest.resolve().parent
    if args.effects_only:
        assert args.effect_library
        target = args.output / 'ClassSelection.cinematics.json'
        candidate = read(target)
        if PROFILE:
            raise ValueError("Reproject the full category with its actual bone manifest to update moving PSCs")
        for phase, _ in PHASES:
            candidate['scenes'][0][phase]['effects'] = effect_clocks(rows, document, read(args.effect_library), phase)
        write(target, candidate)
        print('Updated original PSC clocks', [len(candidate['scenes'][0][phase]['effects']) for phase, _ in PHASES])
        return
    infos = {name: phase_info(rows, index) for name, index in PHASES}
    phases, timelines, clocks, material_gaps = {}, {}, {}, {}
    if PROFILE:
        if BONE_MANIFEST is not None:
            bind_actors(rows, *infos["intro"][:2], "intro")
        PHASE_ROWS.update(intro=rows, loop=loop_initial_rows(rows, infos))
    projected_effects, effect_evidence = None, []
    if PROFILE and args.effect_library:
        import project_class_selection_effects
        projected_effects, effect_evidence = project_class_selection_effects.project(
            sys.modules[__name__], rows, document, read(args.effect_library), infos)
    for phase, matinee0 in PHASES:
        rows = PHASE_ROWS.get(phase, rows)
        matinee, data, duration = infos[phase]
        if args.baked_body or BONE_MANIFEST is not None:
            bind_body(rows, matinee, data, phase, args.baked_body)
        clock, clocks[phase] = clock_keys(rows, data, duration)
        phases[phase] = dict(durationMs=duration, instanceIds=[], cameras=cameras(rows, matinee, data, duration, phase))
        if clock:
            phases[phase]["clockKeys"] = clock
        if args.baked_body or BONE_MANIFEST is not None:
            phases[phase]["lights"] = lights(rows, matinee, data, duration)
        if args.actor_manifest:
            phases[phase]["materialTracks"], material_gaps[phase] = material_tracks(rows, matinee, data, duration, read(args.actor_manifest))
        if args.effect_library:
            phases[phase]['effects'] = projected_effects[phase] if projected_effects is not None else effect_clocks(rows, document, read(args.effect_library), phase)
        timelines[phase] = timeline(rows, matinee, data, duration)
    if PROFILE and args.actor_manifest:
        carry_materials(phases)
    if args.actor_manifest:
        world, ids = project_resources(rows, read(args.actor_manifest), infos, args.baked_body)
        for phase in phases:
            phases[phase]["instanceIds"] = ids[phase]
        write(args.output / f"{AREA}.worldsequences.json", world)
    entry = dict(classId=CLASS_ID, sceneId=PREFIX, **phases)
    if PROFILE.get("backgroundAreaId"):
        entry["backgroundAreaId"] = PROFILE["backgroundAreaId"]
    scene = dict(schema="lostark.class-selection-cinematics", formatVersion=1, areaId=AREA, scenes=[entry])
    write(args.output / "ClassSelection.cinematics.json", scene)
    write(args.output / ("source-native-timeline.json" if PROFILE else "guardian-native-timeline.json"), dict(schema="lostark.source-matinee-timeline", formatVersion=1,
        source=source_identity(document), imports=document["imports"], phases=timelines))
    actor_manifest = read(args.actor_manifest) if args.actor_manifest else None
    effect_library = read(args.effect_library) if args.effect_library else None
    source_audio = {(phase, track["exportIndex0"])
        for phase, phase_timeline in timelines.items()
        for group in phase_timeline["groups"] for track in group["tracks"]
        if not track["disabled"] and "akevent" in track["className"]
        and track["properties"].get("akevents")}
    projected_audio = {(sound["phase"], sound.get("sourceTrackExportIndex0"))
        for sound in (actor_manifest or {}).get("sounds", [])}
    missing_audio = sorted(source_audio - projected_audio)
    unconsumed = ["CameraShake", "DOF and bloom float properties"]
    if missing_audio:
        unconsumed.append("AkEvent audio tracks without an admitted source cue")
    if not PROFILE:
        unconsumed.append("18 cinematic static prop geometry/materials")
    receipt = dict(sourceSha256=hashlib.sha256(args.source.read_bytes()).hexdigest(), source=source_identity(document),
        coordinateConvention="UE world X,Z,-Y in metres; existing cinematic world_pose attachment evaluation",
        timeDomain="durationMs/cameras/WorldSequence keys are source time; clockKeys map wall time to source time",
        clocks=clocks, cameraKeyCounts={k: sum(len(c["keys"]) for c in v["cameras"]) for k, v in phases.items()},
        installed=False, runtimeReady=False, materialProjectionGaps=material_gaps,
        candidateInputsReady=bool(actor_manifest and actor_manifest.get("nativeReady")
            and effect_library and effect_library.get("nativeReady")
            and not any(material_gaps.values()) and not missing_audio),
        sourceAudioTrackCount=len(source_audio), projectedAudioTrackCount=len(projected_audio),
        missingSourceAudioTracks=missing_audio,
        requiredRuntimeValidation="Installed CModel, WorldSequence and Effect admission, then Play/Stop/replay",
        unconsumedSourceFeatures=unconsumed)
    write(args.output / "projection-receipt.json", receipt)
    if effect_evidence:
        write(args.output / "source-psc-occurrences.json", effect_evidence)
    print(json.dumps({k: receipt[k] for k in ("clocks", "cameraKeyCounts", "installed", "runtimeReady")}))


if __name__ == "__main__":
    main()

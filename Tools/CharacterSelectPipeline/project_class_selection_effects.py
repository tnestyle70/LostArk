"""Project source PSC occurrences into the existing cinematic effect contract.

This module is an offline companion to project_guardian_selection. It does not
spawn effects, edit live documents, or introduce a second playback path.
"""
from __future__ import annotations

import math
import numpy as np
from scipy.integrate import quad


def initial_parameters(component):
    result = []
    for parameter in component.get("instanceparameters", []):
        kind = parameter.get("paramtype", "pspt_none")
        if kind == "pspt_scalar":
            value = [float(parameter.get("scalar", 0.))]
        elif kind == "pspt_vector":
            value = [float(parameter.get("vector", {}).get(axis, 0.)) for axis in "xyz"]
        else:
            raise ValueError((parameter.get("name"), "PSC initial parameter type needs source closure", kind))
        result.append(dict(parameterName=parameter["name"], componentCount=len(value),
            keys=[dict(timeMs=0., value=value, arriveTangent=[0.]*len(value),
                leaveTangent=[0.]*len(value), interpolation="CONSTANT")]))
    return result


def parameter_tracks(api, tracks, duration, inherited=None):
    """Clip original Hermite segments without baking a PSC curve in particle age."""
    result = {}
    for previous in inherited or []:
        value = previous["keys"][-1]["value"]
        result[previous["parameterName"]] = dict(parameterName=previous["parameterName"],
            componentCount=len(value), keys=[dict(timeMs=0., value=value,
                arriveTangent=[0.]*len(value), leaveTangent=[0.]*len(value), interpolation="CONSTANT")])
    for track in tracks:
        cls, prop = track["cls"], track["p"]
        if cls not in ("interptrackfloatparticleparam", "efinterptrackvectorparticleparam"):
            continue
        count = 3 if cls == "efinterptrackvectorparticleparam" else 1
        points = sorted(prop.get("vectortrack" if count == 3 else "floattrack", {}).get("points", []), key=lambda p: p["inval"])
        if not points:
            continue

        def value(point, field):
            raw = point.get(field, {}) if count == 3 else point.get(field, 0.)
            return api.base.vec(raw) if count == 3 else np.asarray([raw], dtype=float)

        def sample(seconds):
            if seconds < points[0]["inval"]:
                return value(points[0], "outval"), np.zeros(count), "CONSTANT"
            if seconds >= points[-1]["inval"]:
                return value(points[-1], "outval"), np.zeros(count), "CONSTANT"
            left = max(i for i, point in enumerate(points[:-1]) if point["inval"] <= seconds)
            a, b = points[left:left+2]
            dt = b["inval"]-a["inval"]
            if dt <= 0.:
                raise ValueError((track["index"]-1, "Duplicate particle parameter key time"))
            u = (seconds-a["inval"])/dt
            av, bv = value(a, "outval"), value(b, "outval")
            mode = a.get("interpmode", "cim_linear")
            if mode == "cim_constant":
                return av, np.zeros(count), "CONSTANT"
            if mode == "cim_linear":
                return av*(1-u)+bv*u, (bv-av)/dt, "LINEAR"
            at, bt = value(a, "leavetangent"), value(b, "arrivetangent")
            v = (2*u**3-3*u*u+1)*av + (u**3-2*u*u+u)*dt*at + (-2*u**3+3*u*u)*bv + (u**3-u*u)*dt*bt
            derivative = ((6*u*u-6*u)*av + (3*u*u-4*u+1)*dt*at + (-6*u*u+6*u)*bv + (3*u*u-2*u)*dt*bt)/dt
            return v, derivative, "CUBIC"

        times = sorted({0., duration/1000.} | {p["inval"] for p in points if 0 <= p["inval"] <= duration/1000.})
        keys = []
        for seconds in times:
            original = next((p for p in points if p["inval"] == seconds), None)
            v, derivative, mode = sample(seconds)
            arrive = value(original, "arrivetangent") if original is not None else derivative
            leave = value(original, "leavetangent") if original is not None else derivative
            keys.append(dict(timeMs=seconds*1000., value=v.tolist(), arriveTangent=arrive.tolist(),
                             leaveTangent=leave.tolist(), interpolation=mode))
        result[prop["paramname"]] = dict(parameterName=prop["paramname"], componentCount=count, keys=keys)
    return list(result.values())


def project(api, rows, document, library, infos):
    metadata = {entry["assetId"]: entry for entry in library["assets"]}
    imports = api.source_imports(document)
    result = {phase: [] for phase, _ in api.PHASES}
    evidence = []
    phase_groups = {phase: {a: g for g in rows[data]["p"]["interpgroups"]
                           for a in api.base.group_actor(rows, g, matinee)}
                    for phase, (matinee, data, _) in infos.items()}
    for actor in sorted(api.ACTOR_SCOPE):
        properties = rows[actor]["p"]
        component_ref = properties.get("particlesystemcomponent")
        if component_ref is None:
            continue
        component = rows[component_ref]["p"]
        template = component.get("template")
        if not template:
            continue
        system = imports[template] if template < 0 else rows[template]["name"]
        base_asset = "effect." + api.PREFIX + "." + system
        asset = library.get("actorAssets", {}).get(str(actor-1), base_asset)
        active = bool(component.get("bautoactivate", True))
        age = 0.
        inherited_rate = 1.
        occurrence_id = f"{api.PREFIX}.psc.{component_ref-1}.initial"
        carried_window = None
        inherited_parameters = initial_parameters(component)
        for phase, _ in api.PHASES:
            matinee, data, duration = infos[phase]
            phase_rows = api.PHASE_ROWS.get(phase, rows)
            if api.BONE_MANIFEST is not None:
                api.bind_actors(phase_rows, matinee, data, phase)
            group = phase_groups[phase].get(actor)
            tracks = api.base.active_tracks(phase_rows, group)
            parameters = parameter_tracks(api, tracks, duration, inherited_parameters)
            inherited_parameters = parameters
            toggles = sorted((key for t in tracks if t["cls"] == "interptracktoggle"
                              for key in t["p"].get("toggletrack", [])), key=lambda k: k["time"])
            # Negative loop keys document events before the cropped loop starts;
            # their effects already occurred in the intro and must not retrigger.
            events = [key for key in toggles if 0 <= key["time"]*1000 < duration]
            slomo = [t for t in tracks if t["cls"] == "efinterptrackparticleslomo"
                     and t["p"].get("floattrack", {}).get("points")]
            if len(slomo) > 1:
                raise ValueError((actor-1, phase, "Multiple particle clock tracks"))
            points = slomo[0]["p"]["floattrack"]["points"] if slomo else []

            def rate(seconds):
                value = float(api.base.curve(points, seconds, inherited_rate))
                if not math.isfinite(value) or value < -1e-6:
                    raise ValueError((actor-1, phase, "Invalid particle time dilation", value))
                return max(0., value)

            def advance(start, end):
                boundaries = sorted({start, end} | {p["inval"]*1000 for p in points if start < p["inval"]*1000 < end})
                return sum(quad(rate, a/1000., b/1000., epsabs=1e-9)[0]*1000. for a, b in zip(boundaries, boundaries[1:]))

            spans = []
            previous_window, carried_window = carried_window, None
            start, first_age = 0., age
            for ordinal, event in enumerate(events):
                time = event["time"]*1000.
                action = event["toggleaction"]
                if action not in ("etta_on", "etta_off", "etta_trigger"):
                    raise ValueError((actor-1, phase, "Unsupported PSC toggle", action))
                if active and action in ("etta_off", "etta_trigger"):
                    if time > start:
                        spans.append((start, time, first_age, occurrence_id))
                    age = first_age + advance(start, time)
                    active = False
                if action == "etta_trigger" or (action == "etta_on" and not active):
                    active, start, first_age = True, time, 0.
                    occurrence_id = f"{api.PREFIX}.psc.{component_ref-1}.{phase}.{ordinal}"
            if active:
                spans.append((start, float(duration), first_age, occurrence_id))
                age = first_age + advance(start, duration)
            if spans and asset not in metadata:
                raise ValueError((actor-1, phase, "Active original PSC has no admitted effect asset", asset))
            if not spans:
                inherited_rate = rate(duration/1000.)
                continue
            root_keys = api.transform_keys(phase_rows, group, actor, matinee, data, duration)
            trs = [dict(timeMs=k["timeMs"], position=k["positionOffset"],
                        rotationQuaternion=k["rotationQuaternion"], scale=k["scaleMultiplier"]) for k in root_keys]
            first = trs[0]
            rotation = api.Rotation.from_quat(first["rotationQuaternion"]).as_matrix()
            matrix = np.eye(4)
            matrix[:3, :3] = np.diag(first["scale"]) @ rotation.T
            matrix[3, :3] = first["position"]
            visible_edges = {0., float(duration)} | {key["time"]*1000 for t in tracks
                if t["cls"] == "interptrackvisibility" for key in t["p"].get("visibilitytrack", [])
                if 0 < key["time"]*1000 < duration}
            for start, end, first_age, identity in spans:
                times = {0., float(duration), start, end}
                for left, right in zip(points, points[1:]):
                    a, b = max(start, left["inval"]*1000), min(end, right["inval"]*1000)
                    if b <= a:
                        continue
                    times.update((a, b))
                    if left["outval"] != right["outval"] or left.get("leavetangent", 0) or right.get("arrivetangent", 0):
                        count = math.ceil((b-a)/4.)
                        times.update(a+(b-a)*i/count for i in range(count+1))
                times = sorted(times)
                values, previous, current = [], start, first_age
                for time in times:
                    sample = min(end, max(start, time))
                    if sample > previous:
                        current += advance(previous, sample)
                    values.append([current])
                    previous = sample
                keep = api.reduce_vectors(times, values, [.01])
                clocks = [dict(timeMs=times[i], sourceMs=values[i][0]) for i in keep]
                if len(clocks) > 4096:
                    raise ValueError((actor-1, phase, "Particle clock exceeds runtime contract"))
                edges = sorted({start, end} | {t for t in visible_edges if start < t < end})
                windows = []
                for a, b in zip(edges, edges[1:]):
                    if api.visible_at(phase_rows, group, actor, (a+b)/2000.):
                        if windows and windows[-1][1] == a:
                            windows[-1] = windows[-1][0], b
                        else:
                            windows.append((a, b))
                for ordinal, (a, b) in enumerate(windows):
                    continuous = phase == "loop" and ".loop." not in identity
                    window_id = (previous_window[1] if previous_window and previous_window[0] == identity and a == 0.
                                 else identity+f".{phase}.visible{ordinal}")
                    if b == duration:
                        carried_window = identity, window_id
                    entry = dict(effectId=window_id, assetId=asset,
                        rootWorld=matrix.reshape(-1).tolist(), rootKeys=trs, clockKeys=clocks,
                        startMs=a, endMs=b,
                        sourceLoopEndMs=clocks[-1]["sourceMs"] if metadata[asset].get("nativeInfinite") or metadata[asset].get("sourceZeroLifetime") else 0.,
                        loopAgeDeltaMs=clocks[-1]["sourceMs"]-clocks[0]["sourceMs"] if continuous else 0.)
                    if parameters:
                        entry["parameterTracks"] = parameters
                    result[phase].append(entry)
                evidence.append(dict(actorExportIndex0=actor-1, componentExportIndex0=component_ref-1,
                    phase=phase, assetId=asset, activationId=identity, startMs=start, endMs=end,
                    firstParticleAgeMs=first_age, lastParticleAgeMs=values[-1][0], visibleWindows=windows))
            inherited_rate = rate(duration/1000.)
    return result, evidence

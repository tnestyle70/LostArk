"""Bake Object Tool collider rows into the existing server WorldTrack contract.

The source clock and matrix order match WorldSequencePlayer_Objects.cpp. This
module runs only during publication; the Server still uses its one region runtime.
"""
from __future__ import annotations

import hashlib
import math
import re
from pathlib import Path
from Tools.ModelAssetConverter import verify_dimensionmaster_summon_bind_pose as wm


class ColliderBakeError(ValueError):
    pass


def canonicalize_baked_position(position):
    """Stabilize validated generated metres, well below the 0.5 mm bake tolerance.

    Keep raw samples for curve refinement and bounds checks. Only serialized
    positions use this precision; authored values and positive scales keep theirs.
    """
    return [round(value, 9) or 0.0 for value in position]


def reduce_keys(keys, tolerance=.0005):
    """Simplify a baked curve without crossing a visibility discontinuity."""
    fields=("positionOffset","scaleMultiplier") + (("gripPosition",) if "gripPosition" in keys[0] else ())
    keep={0,len(keys)-1}
    for i in range(1,len(keys)):
        if keys[i]["visible"] != keys[i-1]["visible"]: keep.update((i-1,i))
    def visit(a,b):
        if b<=a+1:return
        worst=-1.;split=a
        for i in range(a+1,b):
            alpha=(keys[i]["timeMs"]-keys[a]["timeMs"])/(keys[b]["timeMs"]-keys[a]["timeMs"])
            error=max(abs(x+(y-x)*alpha-z) for field in fields for x,y,z in zip(keys[a][field],keys[b][field],keys[i][field]))
            if error>worst:worst=error;split=i
        if worst>tolerance:
            keep.add(split);visit(a,split);visit(split,b)
    protected=sorted(keep)
    for a,b in zip(protected,protected[1:]):visit(a,b)
    return [keys[i] for i in sorted(keep)]


def validate_tracks(sequence):
    if "colliderTracks" not in sequence:
        return
    rows = sequence.get("colliderTracks", [])
    if not isinstance(rows, list) or sum(len(sequence.get(k, [])) for k in
            ("tracks", "animationTracks", "effectTracks", "colliderTracks")) > 64:
        raise ColliderBakeError("Object tracks exceed the combined 64-track limit")
    motion=sequence.get("objectMotion",{})
    if rows and (motion.get("spreadDegrees",0) or any(motion.get("spawnHalfExtents",[0,0,0]))):
        raise ColliderBakeError("Collider publication requires zero spread and zero random spawn extents")
    slots = {r["slotId"] for r in sequence.get("tracks", [])}
    ids = set()
    required = {"colliderTrackId", "slotId", "startMs", "durationMs", "positionOffset",
                "halfExtents", "yawDegrees", "behavior", "damagePercent", "gripLocalOffset"}
    for row in rows:
        if set(row) - required - {"attachmentBone", "shape"} or required - set(row):
            raise ColliderBakeError("Object collider has missing or unknown fields")
        identity = row["colliderTrackId"]
        if not isinstance(identity, str) or not re.fullmatch(r"[A-Za-z0-9_.-]{1,128}",identity) or identity in ids:
            raise ColliderBakeError("Object collider ID must be unique and stable")
        ids.add(identity)
        if row["slotId"] not in slots:
            raise ColliderBakeError("Object collider slot lacks a Transform track")
        for key in ("startMs", "durationMs"):
            if type(row[key]) is not int or row[key] < (key == "durationMs"):
                raise ColliderBakeError("Object collider timing must be nonnegative integer milliseconds")
        if row["startMs"] + row["durationMs"] > sequence["durationMs"]:
            raise ColliderBakeError("Object collider exceeds its Motion")
        for key in ("positionOffset", "halfExtents", "gripLocalOffset"):
            values = row[key]
            if not isinstance(values, list) or len(values) != 3 or any(type(x) not in (int, float) or not math.isfinite(x) or abs(x) > (1000 if key == "halfExtents" else 100000) or (key == "halfExtents" and x <= .001) for x in values):
                raise ColliderBakeError("Object collider vector is outside its finite bounds")
        if type(row["yawDegrees"]) not in (int, float) or not math.isfinite(row["yawDegrees"]) or abs(row["yawDegrees"]) > 36000:
            raise ColliderBakeError("Object collider yaw is invalid")
        shape = row.get("shape", "BOX")
        if shape not in ("BOX", "CYLINDER"):
            raise ColliderBakeError("Object collider shape must be BOX or CYLINDER")
        if shape == "CYLINDER" and abs(row["halfExtents"][0] - row["halfExtents"][2]) > .0001:
            raise ColliderBakeError("Object CYLINDER requires equal X/Z radii")
        behavior = row["behavior"]
        if behavior not in ("DAMAGE", "INSTANT_DEATH", "HOOK_CAPTURE") or type(row["damagePercent"]) not in (int, float):
            raise ColliderBakeError("Object collider behavior is invalid")
        if (behavior == "DAMAGE" and not (1 <= row["damagePercent"] <= 100 and int(row["damagePercent"]) == row["damagePercent"])) or (behavior != "DAMAGE" and row["damagePercent"] != 0):
            raise ColliderBakeError("Only DAMAGE carries an integer HP percentage")
        if behavior == "HOOK_CAPTURE" and shape != "BOX":
            raise ColliderBakeError("HOOK_CAPTURE requires a BOX collider")
        bone = row.get("attachmentBone", "")
        if not isinstance(bone, str) or len(bone.encode()) > 256 or any(ord(c) < 32 or ord(c) == 127 for c in bone):
            raise ColliderBakeError("Object collider bone is invalid")
        if behavior != "HOOK_CAPTURE" and (bone or any(row["gripLocalOffset"])):
            raise ColliderBakeError("Only HOOK_CAPTURE carries a bone or grip")


def rotation(degrees):
    p, y, r = [math.radians(v) / 2 for v in degrees]
    cp, sp, cy, sy, cr, sr = math.cos(p), math.sin(p), math.cos(y), math.sin(y), math.cos(r), math.sin(r)
    return (cr*sp*cy+sr*cp*sy, cr*cp*sy-sr*sp*cy, sr*cp*cy-cr*sp*sy, cr*cp*cy+sr*sp*sy)


def matrix(scale=(1, 1, 1), quat=(0, 0, 0, 1), position=(0, 0, 0)):
    return wm.affine_matrix(scale, quat, position)


def sample_key(sequence, slot, age):
    keys = next(r["keys"] for r in sequence["tracks"] if r["slotId"] == slot)
    left = keys[0]
    for right in keys[1:]:
        if age < right["timeMs"]:
            alpha = max(0., (age-left["timeMs"])/(right["timeMs"]-left["timeMs"]))
            if sequence.get("interpolation", "SMOOTH_STEP") == "SMOOTH_STEP":
                alpha = alpha*alpha*(3-2*alpha)
            return {"positionOffset": [a+(b-a)*alpha for a,b in zip(left["positionOffset"],right["positionOffset"])],
                    "scaleMultiplier": [a+(b-a)*alpha for a,b in zip(left["scaleMultiplier"],right["scaleMultiplier"])],
                    "rotationQuaternion": wm.sample_quaternion([(0,*left["rotationQuaternion"]),(1,*right["rotationQuaternion"])],alpha),
                    "visible": left.get("visible",True)}
        left = right
    return left


def sample_bone(sequence, resource, slot, age, bone, load_model):
    tracks = sorted((r for r in sequence.get("animationTracks", []) if r["slotId"] == slot), key=lambda r:r.get("startMs",0))
    clips = tuple(r["clipName"] for r in tracks)
    models = getattr(load_model, "_collider_models", None)
    model_key = (resource["modelAssetId"], clips)
    model = models.get(model_key) if models is not None else None
    if model is None:
        model = load_model(resource["modelAssetId"], clips)
        if models is not None:
            models[model_key] = model
    cache = getattr(load_model, "_collider_pose_cache", None)
    cache_key = (id(model),id(sequence),slot,age,bone,resource.get("modelPreScale",.01))
    if cache is not None and cache_key in cache: return cache[cache_key]
    indices = [i for i,b in enumerate(model.skeleton_bones) if b.name == bone]
    if len(indices) != 1:
        raise ColliderBakeError("Object collider attachment bone is absent or ambiguous: " + bone)
    local = [list(b.transform) for b in model.skeleton_bones]
    selected = [r for r in tracks if r.get("startMs",0) <= age]
    if selected:
        track = selected[-1]
        animation = next((a for a in model.animations if a.name == track["clipName"]), None)
        if animation is None or animation.channels is None or animation.ticks_per_second <= 0:
            raise ColliderBakeError("Object collider animation is unavailable")
        source = track.get("sourceStartMs",0)*.001*animation.ticks_per_second
        elapsed = max(0,age-track.get("startMs",0))*.001*track["playbackRate"]*animation.ticks_per_second
        end = min([r.get("startMs",0) for r in tracks if r.get("startMs",0)>track.get("startMs",0)] + [sequence["durationMs"]])
        if age >= end and track.get("holdLastFrame",True):
            ticks = animation.duration_ticks
        elif track.get("loop",False):
            span = animation.duration_ticks-source
            if span <= 0: raise ColliderBakeError("Object animation loop has no remaining source duration")
            ticks = source + elapsed % span
        elif source+elapsed > animation.duration_ticks:
            ticks = animation.duration_ticks if track.get("holdLastFrame",True) else source
        else: ticks = source+elapsed
        for channel in animation.channels:
            local[channel.bone_index] = matrix(wm.sample_vector(channel.scale_keys,ticks,(1,1,1)),
                wm.sample_quaternion(channel.rotation_keys,ticks),wm.sample_vector(channel.position_keys,ticks,(0,0,0)))
    combined = wm.combined_transforms(model.skeleton_bones,local)[indices[0]]
    result = list(combined)
    for base in (0,4,8):
        length = math.sqrt(sum(result[base+i]**2 for i in range(3)))
        if length <= 1e-10: raise ColliderBakeError("Object bone basis is singular")
        for i in range(3): result[base+i] /= length
    for i in range(12,15): result[i] *= resource.get("modelPreScale",.01)
    if cache is not None:
        if len(cache)>=65536: cache.clear()
        cache[cache_key]=result
    return result


def sample_object(sequence, instance, resource, box, world, emitter, age, row, load_model):
    motion = sequence.get("objectMotion", {})
    # Authored emissions have zero spread. Random spread's argument evaluation
    # order is compiler-dependent; reject that unproven gameplay case explicitly.
    if motion.get("spreadDegrees",0): raise ColliderBakeError("Collider publication needs authored emissions or zero spread")
    key = sample_key(sequence,row["slotId"],min(age,sequence["durationMs"]))
    seconds = age*.001
    state = (motion.get("seed",1) ^ ((emitter+1)*0x9e3779b9)) & 0xffffffff
    def random_unit():
        nonlocal state
        state ^= (state << 13) & 0xffffffff; state ^= state >> 17; state ^= (state << 5) & 0xffffffff
        state &= 0xffffffff
        return (state & 0xffffff)/16777215.
    # The zero-spread direction still consumes one/two RNG draws in C++.
    for _ in range(1 if sequence.get("effectTracks") else 2): random_unit()
    spawn = [(random_unit()*2-1)*v for v in motion.get("spawnHalfExtents",[0,0,0])]
    revolution = matrix(quat=rotation([v*seconds for v in motion.get("revolutionDegreesPerSecond",[0,0,0])]))
    orbit_offset = motion.get("revolutionOffset",[0,0,0])
    orbit = [a-b for a,b in zip(wm.transform_point(tuple(orbit_offset),revolution),orbit_offset)]
    placement = box.get("placement")
    emissions = motion.get("emissions",[])
    instance_offset = [0,0,0] if placement else instance.get("position",[0,0,0])
    pos = [(0 if emissions else instance_offset[i])+key["positionOffset"][i]+spawn[i]+motion.get("velocity",[0,0,0])[i]*seconds+
           motion.get("acceleration",[0,0,0])[i]*.5*seconds**2+orbit[i] for i in range(3)]
    scale = [a*b for a,b in zip(resource.get("scale",[1,1,1]),key["scaleMultiplier"])]
    local_scale = list(scale)
    visual = wm.matrix_multiply(matrix(scale,key["rotationQuaternion"]),matrix(quat=rotation([v*seconds for v in motion.get("angularVelocityDegrees",[0,0,0])])))
    visual = wm.matrix_multiply(visual,matrix(position=pos))
    yaw = 0.
    if emissions:
        emission = emissions[emitter]; yaw = emission.get("yawDegrees",0)
        visual = wm.matrix_multiply(visual,matrix(quat=rotation([0,yaw,0]),position=emission["positionOffset"]))
        visual = wm.matrix_multiply(visual,matrix(position=instance_offset))
    if placement:
        angles = placement.get("rotationDegrees",[0,0,0])
        if abs(angles[0])+abs(angles[2]) > 1e-6: raise ColliderBakeError("Ground collider placement must remain upright")
        visual = wm.matrix_multiply(visual,matrix(placement["scale"],rotation(angles),placement["position"]))
        scale = [a*b for a,b in zip(scale,placement["scale"])]; yaw += angles[1]
    else:
        offset = world.get("positionOffset",[0,0,0])
        for i in range(3): visual[12+i] += offset[i]
    if any(v<=0 or not math.isfinite(v) for v in scale): raise ColliderBakeError("Object collider scale must remain positive")
    if row.get("attachmentBone"):
        pivot = wm.matrix_multiply(sample_bone(sequence,resource,row["slotId"],age,row["attachmentBone"],load_model),visual)
    elif row["behavior"] == "HOOK_CAPTURE":
        pivot = visual
    else:
        # Mesh upright correction and self-spin must not rotate the floor collider.
        emission_yaw = emissions[emitter].get("yawDegrees",0) if emissions else 0.
        pivot = matrix(local_scale,rotation([0,emission_yaw,0]))
        if placement:
            pivot = wm.matrix_multiply(pivot,matrix(placement["scale"],rotation(placement["rotationDegrees"])))
        pivot[12:15] = visual[12:15]
    center = wm.transform_point(tuple(row["positionOffset"]),pivot)
    grip = wm.transform_point(tuple(row["gripLocalOffset"]),pivot)
    return center,grip,scale,yaw+row["yawDegrees"],key.get("visible",True)


def bake_windows(sequences, worlds, boxes, load_model, *, pattern_end_ms=None, sampling_cache=None):
    """Return generated ENTER_AREA descriptors, preserving finite Motion cycles."""
    sampling_cache = {} if sampling_cache is None else sampling_cache
    # A shared cache belongs to one publication's immutable input snapshot.
    # Keep sequence references alive for the identity-based pose keys.
    sampling_cache.setdefault("sequences", {})[id(sequences)] = sequences
    load_model._collider_pose_cache = sampling_cache.setdefault("poses", {})
    # A bake uses pinned publication inputs. Resolve/check/decode a model+clip
    # selection once, not at every pose sample (including pose-cache hits).
    # The publisher still rechecks every observed input before/after promotion.
    load_model._collider_models = sampling_cache.setdefault("models", {})
    templates = {r["sequenceId"]:r for r in sequences["templates"]}
    instances = {r["instanceId"]:r for r in sequences["instances"]}
    resources = {r["objectId"]:r for r in sequences.get("objectResources",[])}
    for template in templates.values(): validate_tracks(template)
    # Model-less groups share one placed origin. Expand only publication input;
    # each child still bakes through the existing emission/motion sampler.
    worlds = dict(worlds)
    expanded_boxes = []
    for box in boxes:
        world = worlds[box["worldId"]]
        group = resources.get(world["sequenceInstanceId"], {})
        members = group.get("motionInstanceIds")
        if members is None:
            expanded_boxes.append(box)
            continue
        for index, identity in enumerate(members):
            instance = instances.get(identity)
            if instance is None:
                raise ColliderBakeError("Collider group motion is missing")
            if not instance.get("enabled", True):
                continue
            bindings = instance.get("bindings", [])
            if len(bindings) != 1 or bindings[0].get("targetKind") != "OBJECT_RESOURCE":
                raise ColliderBakeError("Collider group motion needs one real model binding")
            world_id = box["worldId"] + ".collider-member." + str(index)
            worlds[world_id] = {**world, "sequenceInstanceId": identity,
                                "objectResourceId": bindings[0]["targetId"]}
            expanded_boxes.append({**box, "worldId": world_id,
                                   "occurrenceId": box["occurrenceId"] + ".member." + str(index)})
    result = []
    for box in expanded_boxes:
        world = worlds[box["worldId"]]
        instance = instances[world["sequenceInstanceId"]]
        initial_instance = instance
        cursor = 0.; depth = 0
        while cursor < box["durationMs"]:
            sequence = templates[instance["templateId"]]
            rows = sequence.get("colliderTracks",[])
            if rows and (initial_instance.get("anchorKind","WORLD") != "WORLD" or world.get("anchorKind","NONE") not in ("NONE","WORLD")):
                raise ColliderBakeError("Object collider bake requires a fixed WORLD anchor")
            rate = instance.get("playbackSpeed",1)*box["playbackSpeed"]
            if rate <= 0: raise ColliderBakeError("Object Motion playback speed must be positive")
            begin = cursor+instance.get("startDelayMs",0)
            motion = sequence.get("objectMotion",{})
            emissions = motion.get("emissions",[])
            delays = [e["startDelayMs"] for e in emissions] if emissions else [i*motion.get("intervalMs",0) for i in range(motion.get("count",1))]
            span = sequence["durationMs"] + (max(delays,default=0) if sequence.get("effectTracks") else 0)
            if instance.get("loopFullPresentation", False):
                effect_end = max((sequence["durationMs"] if row.get("timing", "MOTION_END") == "MOTION_END" else row.get("startMs", 0)) + row["durationMs"] for row in sequence.get("effectTracks", [])) if sequence.get("effectTracks") else sequence["durationMs"]
                span = max(span, effect_end + max(delays, default=0))
            end_policy = instance.get("motionEnd","STOP")
            repeats = math.ceil(max(0,box["durationMs"]-begin)/(span/rate)) if end_policy == "LOOP" else 1
            for cycle in range(repeats):
                origin = begin+cycle*span/rate
                cycle_end = (origin+span/rate if sequence.get("effectTracks") else min(box["durationMs"],origin+span/rate)) if end_policy != "HOLD" else (max(box["durationMs"],origin+span/rate) if sequence.get("effectTracks") else box["durationMs"])
                # Birth deadlines allow existing visual tails; gameplay ownership ends with the Pattern.
                if pattern_end_ms is not None:
                    cycle_end = min(cycle_end, pattern_end_ms - box["startMs"])
                for emitter,delay in enumerate(delays):
                    if origin+delay/rate >= box["durationMs"]: continue
                    for row in rows:
                        transform_keys = next(r["keys"] for r in sequence["tracks"] if r["slotId"] == row["slotId"])
                        spans=[]; visible_start=None
                        for transform_key in transform_keys:
                            if transform_key.get("visible",True) and visible_start is None:
                                visible_start=transform_key["timeMs"]
                            elif not transform_key.get("visible",True) and visible_start is not None:
                                spans.append((visible_start,transform_key["timeMs"]));visible_start=None
                        if visible_start is not None: spans.append((visible_start,sequence["durationMs"]))
                        for visible_start,visible_end in spans:
                            if max(row["startMs"],visible_start) >= min(row["startMs"]+row["durationMs"],visible_end): continue
                            binding = [r for r in initial_instance["bindings"] if r["slotId"] == row["slotId"] and r["targetKind"] == "OBJECT_RESOURCE"]
                            if len(binding)!=1 or binding[0]["targetId"] not in resources: raise ColliderBakeError("Collider slot requires one model object binding")
                            resource = resources[binding[0]["targetId"]]
                            start = origin+(delay+max(row["startMs"],visible_start))/rate
                            stop = min(cycle_end,origin+(delay+min(row["startMs"]+row["durationMs"],visible_end))/rate)
                            transform_keys = next(r["keys"] for r in sequence["tracks"] if r["slotId"] == row["slotId"])
                            first_hidden = visible_end
                            carry_end = min(cycle_end,origin+(delay+first_hidden)/rate) if row["behavior"] == "HOOK_CAPTURE" else stop
                            stop = min(stop,carry_end)
                            # Integer keys retain exact samples at every Server 30 Hz
                            # boundary (within the existing millisecond wire format).
                            start_ms = math.ceil(box["startMs"]+start-1e-8)
                            stop_ms = math.ceil(box["startMs"]+stop-1e-8)
                            end_ms = math.ceil(box["startMs"]+carry_end-1e-8)
                            if stop_ms<=start_ms: continue
                            duration = end_ms-start_ms
                            # The Server starts a WORLD cue and a Logic window on
                            # their respective ceil-to-fixed-tick clocks. Bake the
                            # source pose at that actual first tick, not at the raw
                            # authored millisecond (which can differ by 33 ms).
                            source_origin = math.ceil(start_ms*.03-1e-10)/.03 - math.ceil(box["startMs"]*.03-1e-10)/.03 - origin
                            times = {0,duration}
                            times.update(range(1,duration,16))
                            boundaries = [k["timeMs"] for k in transform_keys] + [a.get("startMs",0) for a in sequence.get("animationTracks",[]) if a["slotId"] == row["slotId"]]
                            for boundary in boundaries:
                                offset = math.ceil((delay+boundary)/rate-source_origin-1e-9)
                                times.update(t for t in (offset-1,offset,offset+1) if 0<t<duration)
                            if len(times)>4096: raise ColliderBakeError("Object collider track exceeds 4096 keys; shorten its finite window")
                            sample_cache = {}
                            def sample(time):
                                if time in sample_cache: return sample_cache[time]
                                age = max(0,(source_origin+time)*rate-delay)
                                center,grip,scale,current_yaw,visible = sample_object(sequence,initial_instance,resource,box,world,emitter,age,row,load_model)
                                if row.get("shape", "BOX") == "CYLINDER":
                                    # Circular WORLD tracks require equal X/Z keys.
                                    # Preserve the existing radius * max(X,Z) hit
                                    # shape after the full source center is sampled.
                                    radius_scale = max(scale[0], scale[2])
                                    scale = [radius_scale, scale[1], radius_scale]
                                key={"timeMs":time,"positionOffset":list(center),"rotationY":0.,"rotationW":1.,"scaleMultiplier":scale,"visible":bool(visible and time<duration and age<first_hidden)}
                                if row["behavior"] == "HOOK_CAPTURE": key["gripPosition"]=list(grip)
                                if any(not math.isfinite(x) or abs(x)>100000 for x in [*center,*grip]): raise ColliderBakeError("Object collider bake exceeds world bounds")
                                sample_cache[time] = (key,current_yaw)
                                return sample_cache[time]
                            # Refine nonlinear translation/bone arcs to 1 mm at
                            # integer midpoints; no extra transform runtime exists.
                            def refine(a,b):
                                if b-a<=1: return
                                mid=(a+b)//2
                                ka,kb,km=sample(a)[0],sample(b)[0],sample(mid)[0]
                                alpha=(mid-a)/(b-a)
                                fields=("positionOffset","scaleMultiplier") + (("gripPosition",) if row["behavior"] == "HOOK_CAPTURE" else ())
                                if max(abs(x+(y-x)*alpha-z) for field in fields for x,y,z in zip(ka[field],kb[field],km[field])) > .0005:
                                    times.add(mid)
                                    if len(times)>4096: raise ColliderBakeError("Object collider nonlinear motion exceeds 4096 keys")
                                    refine(a,mid);refine(mid,b)
                            initial=sorted(times)
                            for a,b in zip(initial,initial[1:]): refine(a,b)
                            keys=reduce_keys([sample(time)[0] for time in sorted(times)])
                            keys=[{**key,
                                   "positionOffset":canonicalize_baked_position(key["positionOffset"]),
                                   **({"gripPosition":canonicalize_baked_position(key["gripPosition"])}
                                      if "gripPosition" in key else {})} for key in keys]
                            yaw=sample(0)[1]
                            identity = hashlib.sha256((box["occurrenceId"]+'|'+instance["instanceId"]+'|'+row["colliderTrackId"]+f'|{cycle}|{emitter}|{visible_start}').encode()).hexdigest()[:32]
                            shape = row.get("shape", "BOX")
                            region={"regionId":"object.collider.region."+identity,"shape":shape,"anchorKind":"WORLD","center":[0,0,0],
                                    "yawDegrees":yaw,"halfExtents":row["halfExtents"],"radiusM":row["halfExtents"][0] if shape == "CYLINDER" else 1.,"halfAngleDegrees":45.,"cardSymbol":"NONE","cardColor":"NONE",
                                    "worldTrack":{"startMs":start_ms,"startDelayMs":0,"durationMs":duration,"playbackSpeed":1.,"interpolation":"LINEAR",
                                                  "baselinePosition":[0,0,0],"baselineYawDegrees":0.,"baselineScale":[1,1,1],"keys":keys}}
                            result.append({"occurrenceId":"object.collider.window."+identity,"startMs":start_ms,"durationMs":stop_ms-start_ms,
                                           "region":region,"behavior":row["behavior"],"damagePercent":int(row["damagePercent"])})
                            if len(result)>128: raise ColliderBakeError("Object colliders exceed the 128-window pattern budget")
            if end_policy != "NEXT": break
            cursor=begin+span/rate; depth+=1
            if depth>=32 or instance.get("nextMotionId") not in instances: raise ColliderBakeError("Object Motion NEXT chain is unresolved or exceeds 32")
            instance=instances[instance["nextMotionId"]]
    return result

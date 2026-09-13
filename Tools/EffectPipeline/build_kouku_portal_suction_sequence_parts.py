"""Split the authored portal cue from its unrelated mice and golden trails.

The source document stays untouched. Both outputs retain element IDs, native
materials, source modules and motion; the portal gets its own center and clock.
The caller stages catalog/Composition registration after preserving tool drafts.
"""
import argparse
import copy
import hashlib
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SOURCE = "effect.kouku.gate1.authored.portal-arrival.1"
PORTAL = "effect.kouku.gate1.intro.portal-suction.centered"
CONTEXT = "effect.kouku.gate1.authored.portal-arrival.context"
ACTOR_PREFIX = "lv_lut_midnightc_ed_scene03a.theworld.persistentlevel."


def payload(value):
    return (json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n").encode("utf8")


def build(source_path, output):
    original_bytes = source_path.read_bytes()
    source = json.loads(original_bytes)
    if source["effectAssetId"] != SOURCE:
        raise ValueError("Expected the saved portal-arrival.1 source document")
    portal_rows, other_rows = [], []
    for element in source["elements"]:
        track = element.get("sourceTransformTrack", {})
        nodes = track.get("nodes", [])
        actor = nodes[-1]["sourceObjectPath"] if nodes else ""
        (portal_rows if actor in (ACTOR_PREFIX + "emitter_8", ACTOR_PREFIX + "emitter_9")
         else other_rows).append(element)
    if len(portal_rows) != 30 or len(other_rows) != 120:
        raise ValueError("Source portal/remaining occurrence set changed; inspect the saved edit first")
    center = next(e for e in portal_rows if e["sourceTransformTrack"]["nodes"][-1]["sourceObjectPath"] ==
                  ACTOR_PREFIX + "emitter_8")["sourceTransformTrack"]["nodes"][-1]
    if center["frame"] != "WORLD" or center["positionKeys"] or center["eulerKeys"]:
        raise ValueError("Portal center requires its original fixed WORLD actor")
    origin = center["initialPositionUE3Cm"]
    first = min(e["detail"]["timing"]["startDelaySeconds"] for e in portal_rows)
    portal = copy.deepcopy(source)
    portal.update(effectAssetId=PORTAL, displayName="1관문_포탈 생성·흡입_중심 앵커", elements=copy.deepcopy(portal_rows))
    for element in portal["elements"]:
        element["detail"]["timing"]["startDelaySeconds"] -= first
        element["sourceTransformTrack"]["sourceTimeOriginSeconds"] += first
        element["sourceTransformTrack"]["previewOriginUE3Cm"] = copy.deepcopy(origin)
    context = copy.deepcopy(source)
    context.update(effectAssetId=CONTEXT, displayName="1관문_전후 쥐·금빛 연결", elements=copy.deepcopy(other_rows))
    # This derivative is independently scheduled by its Composition box. Keep
    # the original 150-element reference intact, but do not bake its scene
    # pre-roll into the reusable context asset again on regeneration.
    if (context.get("modelCues") or context.get("sourceModelPreview") or
            context.get("runtimeExtensions", {}).get("bakedEdgeHistories")):
        raise ValueError("Context pre-roll removal requires independent map source clocks")
    for element in context["elements"]:
        if (not element.get("sourceTransformTrack", {}).get("nodes") or
                element.get("actionCueAttachment", {}).get("enabled") or
                element.get("transformInheritance", {}).get("enabled") or
                element.get("sourcePresentation", {}).get("enabled") or
                element["kind"] in ("LIGHT", "SCREEN_POST")):
            raise ValueError("Context has an unsupported independent timeline: " + element["id"])
        delay = element["detail"]["timing"]["startDelaySeconds"]
        source_origin = element["sourceTransformTrack"]["sourceTimeOriginSeconds"]
        if not math.isfinite(delay) or not 0 <= delay <= 600 or not math.isfinite(source_origin):
            raise ValueError("Context source clock is invalid: " + element["id"])
    context_first = min(e["detail"]["timing"]["startDelaySeconds"] for e in context["elements"])
    for element in context["elements"]:
        element["detail"]["timing"]["startDelaySeconds"] -= context_first
        element["sourceTransformTrack"]["sourceTimeOriginSeconds"] += context_first
    # Match the existing Playback endpoint: finite source loops can exceed the
    # authored emission window, and already living particles retain their tail.
    endpoints = []
    for element in portal["elements"]:
        timing, recipe = element["detail"]["timing"], element["sourceRecipe"]
        particle = element["detail"]["particle"]
        emission = (recipe["emitterDurationSeconds"] * recipe["emitterLoopCount"]
                    if recipe["emitterDurationSeconds"] > 0 and recipe["emitterLoopCount"]
                    else timing["lifeTimeSeconds"])
        endpoints.append(timing["startDelaySeconds"] + recipe["emitterDelaySeconds"] + emission +
                         timing["afterImageSeconds"] + particle["lifeTimeSeconds"][1] *
                         particle["sourceScale"]["lifeTime"])
    duration = max(endpoints)
    context_origin = other_rows[0]["sourceTransformTrack"]["previewOriginUE3Cm"]
    if any(e["sourceTransformTrack"]["previewOriginUE3Cm"] != context_origin for e in other_rows):
        raise ValueError("Context no longer shares one source placement origin")
    output.mkdir(parents=True, exist_ok=True)
    documents = []
    for document in (portal, context):
        relative = Path("Data/Effects/Authored") / (document["effectAssetId"] + ".effect.json")
        target = output / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        encoded = payload(document)
        if target.exists() and target.read_bytes() != encoded:
            raise ValueError("Candidate already contains different edits: " + str(target))
        target.write_bytes(encoded)
        documents.append(dict(effectAssetId=document["effectAssetId"], path=relative.as_posix(),
                              displayName=document["displayName"], elementCount=len(document["elements"]),
                              sha256=hashlib.sha256(encoded).hexdigest()))
    if source_path.read_bytes() != original_bytes:
        raise ValueError("Source changed while preparing candidates; regenerate from the saved document")
    manifest = dict(sourcePath=str(source_path), sourceSha256=hashlib.sha256(original_bytes).hexdigest(),
                    installed=False, documents=documents, portalElementIds=[e["id"] for e in portal_rows],
                    originalStartSeconds=first, portalDurationMs=math.ceil(duration * 1000),
                    contextOriginalStartSeconds=context_first,
                    contextRemovedLeadingDelaySeconds=context_first,
                    contextClockBasis="Original scene reference retained; local Effect zero is the first context cue",
                    contextTimingPolicy="Composition box owns scene start; relative element and source track clocks are preserved",
                    portalMapPosition=[origin[0] * .01, origin[2] * .01, -origin[1] * .01],
                    portalMapRotationDegrees=[0, 0, 0],
                    contextMapPosition=[context_origin[0] * .01, context_origin[2] * .01,
                                        -context_origin[1] * .01],
                    contextMapRotationDegrees=[0, 0, 0],
                    manualVisualValidation="USER_PENDING")
    (output / "portal-parts.json").write_bytes(payload(manifest))
    return manifest


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=ROOT / "Data/Effects/Authored" / (SOURCE + ".effect.json"))
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    print(json.dumps(build(args.source, args.output), ensure_ascii=False))

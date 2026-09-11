"""Combine the current four ALT V authored clips without rewriting their inputs.

This preserves author edits, stable element/provider/anchor IDs, source modules,
and resources. Only the single output receives action-clock offsets and a common
horse assembly rotation. Registration and the one Product cue are separate edits.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
from pathlib import Path
import struct
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/ActorXAssetCooker"))
import retime_wmodel_ticks as wmodel

ASSET = "effect.lancemaster.skill.34630.full.restore"
CLIPS = [f"flm_sk_super_squalllance_0{i}" for i in range(1, 5)]


def read(path):
    return json.loads(path.read_bytes())


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def f32(value):
    return struct.unpack("<f", struct.pack("<f", value))[0]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-root", type=Path, required=True)
    args = parser.parse_args()
    binding = next(row for row in read(ROOT / "Data/Animation/Authored/LanceMaster/LanceMaster.skillbindings.json")["bindings"]
                   if row["skillId"] == 34630)
    if binding["clips"] != CLIPS:
        raise ValueError("ALT V no longer uses the measured four native clips")
    model_path = ROOT / "Client/Bin/Resources/Character/LanceMaster/LanceMaster.wmodel"
    data = wmodel.read_bounded(model_path)
    native = {name: wmodel.ANIMATION_HEADER.unpack_from(data, offset)[2]
              for offset, name in wmodel.find_animation_sections(data)}
    # CAnimation uses 30 ticks/second regardless of the stored rate (24 here).
    # Match CEffectAuthoringSequencer's float arithmetic and per-row ceil.
    durations = [math.ceil(f32(f32(native[name] / 30.0) * 1000.0)) for name in CLIPS]
    offsets = [sum(durations[:index]) for index in range(4)]
    documents, sequences, receipts = [], [], []
    for index in range(4):
        source_id = f"effect.lancemaster.skill.34630.clip{index + 1}.full.restore"
        path = ROOT / "Data/Effects/Authored" / (source_id + ".effect.json")
        document = read(path)
        if document["effectAssetId"] != source_id or document["version"] != 13:
            raise ValueError("Unexpected source identity/schema: " + str(path))
        documents.append(document)
        sidecar = ROOT / "Data/Effects/Sequences" / (source_id + ".effectsequence.json")
        sequences.append(read(sidecar) if sidecar.exists() else None)
        receipts.append(dict(path=str(path.relative_to(ROOT)), sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
                             elements=len(document["elements"]), modelCues=len(document["modelCues"]),
                             clip=CLIPS[index], offsetMs=offsets[index], durationMs=durations[index]))
    if any(document["particleSystem"] != documents[0]["particleSystem"] for document in documents):
        raise ValueError("Per-clip particle-system edits differ; cannot flatten them silently")
    full = copy.deepcopy(documents[0])
    full.update(effectAssetId=ASSET, displayName="창술사 Alt V 전체 · 이펙트 / 말 / 카메라", elements=[], modelCues=[])
    rotation_changes = []
    for index, document in enumerate(documents):
        seconds = offsets[index] / 1000.0
        for source in document["elements"]:
            element = copy.deepcopy(source)
            element["detail"]["timing"]["startDelaySeconds"] += seconds
            full["elements"].append(element)
        for source in document["modelCues"]:
            cue = copy.deepcopy(source)
            cue["startDelaySeconds"] += seconds
            # All four material sections are one source skeleton. A per-section
            # inspection rotation cannot remain in the assembled new Effect.
            rotation = cue["localTransform"]["rotationDegrees"]
            if rotation != [0, -90, 0]:
                rotation_changes.append(dict(cueId=cue["cueId"], previous=rotation, assembled=[0, -90, 0]))
            cue["localTransform"]["rotationDegrees"] = [0, -90, 0]
            full["modelCues"].append(cue)
    element_ids = {element["id"] for element in full["elements"]}
    cue_ids = {cue["cueId"] for cue in full["modelCues"]}
    if len(element_ids) != len(full["elements"]) or len(cue_ids) != len(full["modelCues"]):
        raise ValueError("Cross-clip stable ID collision")
    for element in full["elements"]:
        owner = element.get("actionCueAttachment", {}).get("modelCueId")
        if owner and owner not in cue_ids:
            raise ValueError("Missing ModelCue anchor owner: " + owner)
        for module in element.get("sourceRecipe", {}).get("modules", []):
            for literal in module.get("literals", []):
                if literal["propertyPath"] == "runtime.providerelementid" and literal.get("value") not in element_ids:
                    raise ValueError("Missing particle provider")
    cameras, local_only = [], set()
    duration = sum(durations)
    for index, sequence in enumerate(sequences):
        if not sequence:
            continue
        if sequence["model"]["sequenceId"] != "skill.34630" or len(sequence["effects"]) != 1:
            raise ValueError("Incompatible source camera sequence")
        duration = max(duration, offsets[index] + sequence["effects"][0]["durationMs"])
        for source in sequence["cameras"]:
            camera = copy.deepcopy(source)
            camera["startMs"] += offsets[index]
            cameras.append(camera)
        local_only.update(set(sequence.get("localOnlyElementIds", [])) & element_ids)
    # These ordinary source documents use the existing finite emitter duration
    # plus particle/trail/afterimage tails. The Tool recomputes actual runtime
    # duration when playing; this sidecar also bounds its camera/animation rows.
    for element in full["elements"]:
        timing = element["detail"]["timing"]
        recipe = element.get("sourceRecipe", {})
        span = timing["lifeTimeSeconds"]
        delay = 0
        tail = 0
        if recipe.get("enabled"):
            delay = recipe["emitterDelaySeconds"]
            if recipe["emitterDurationSeconds"] > 0 and recipe["emitterLoopCount"]:
                span = recipe["emitterDurationSeconds"] * recipe["emitterLoopCount"]
        if element["kind"] == "particle":
            particle = element["detail"]["particle"]
            tail = particle["lifeTimeSeconds"][1] * particle.get("sourceScale", {}).get("lifeTime", 1)
        if element["kind"] == "trail":
            tail = element["detail"]["trail"]["pointLifeTimeSeconds"]
        duration = max(duration, math.ceil((timing["startDelaySeconds"] + delay + span + tail + timing["afterImageSeconds"]) * 1000))
    sequence = dict(schema="lostark.effect-authoring-sequence", formatVersion=4, sequenceId=ASSET,
                    model=dict(kind="MODEL_SEQUENCE", assetName="LanceMaster", sequenceId="skill.34630", anchorMemberId=""),
                    anchorMode="MODEL_ROOT", worldPosition=[0, 0, 0], effects=[dict(occurrenceId="lance34630.full.effect",
                    owner="V1_DOCUMENT", effectId=ASSET, anchorSlotId="root", startMs=0, durationMs=duration,
                    offset=[0, 0, 0], muted=False, screenPost=False)], cameras=cameras, customAnimation=True,
                    animationRows=[dict(occurrenceId=f"animation.lance34630.full.clip{index+1}", displayName=name,
                    memberId="", clipName=name, startMs=offsets[index], durationMs=durations[index], sourceStartMs=0,
                    sourcePlayMs=0, playRate=1.0, loop=False, muted=False) for index, name in enumerate(CLIPS)],
                    soundRows=[], colliderRows=[], localOnlyElementIds=sorted(local_only))
    write(args.output_root / "Authored" / (ASSET + ".effect.json"), full)
    write(args.output_root / "Sequences" / (ASSET + ".effectsequence.json"), sequence)
    write(args.output_root / "combine_receipt.json", dict(inputs=receipts, assetId=ASSET,
          elements=len(element_ids), modelCues=len(cue_ids), animationDurationMs=sum(durations),
          sequenceDurationMs=duration, cameraRows=len(cameras), localOnlyElements=len(local_only),
          assemblyRotationChanges=rotation_changes, originalDocumentsRewritten=False))
    print(json.dumps(dict(assetId=ASSET, elements=len(element_ids), modelCues=len(cue_ids),
                         offsetsMs=offsets, animationDurationMs=sum(durations), sequenceDurationMs=duration)))


if __name__ == "__main__":
    main()

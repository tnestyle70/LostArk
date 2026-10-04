"""Build, but never install, the exact Alt V six-notify radial restoration.

The source rotator belongs to the complete ParticleSystem, not its first
sprite. Preserve authored world-space birth, native modules, and all other
elements. Re-run this narrow adapter after a full source regeneration.
"""
from pathlib import Path
import argparse
import base64
import collections
import copy
import hashlib
import json
import struct
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/LevelPlacementExtractor"))
from extract_action_effect_notifies import extract_action_document
from build_action_cue_recipe import decode_typed_payload

SYSTEM = "fx_pc_wgl_08.par_w_wgl_supergprotection_explo_de"
ASSET = "effect.warlord.skill.17250.clip2.full.restore"
SLOT = "authored.warlord.altv.guardian-ground"
YAWS = (0, -10922, -21845, 32768, 21845, 10922)
PROFILES = {7: 1140, 6: 1141, 5: 1142, 13: 1143, 15: 1143,
            17: 401, 21: 1144, 19: 1145, 20: 1146, 30: 1034}
AXIS_EMITTERS = {7, 6, 5}


def sha(value):
    return hashlib.sha256(value).hexdigest()


def encoded(value):
    return (json.dumps(value, ensure_ascii=False, indent=2) + "\n").encode("utf-8")


def require(condition, message):
    if not condition:
        raise ValueError(message)


def source_rotators(action_path):
    raw_action = action_path.read_bytes()
    actions = extract_action_document(action_path, "WARLORD")["actions"]
    action = next(row for row in actions if row["actionId"] == 17250)
    result = {}
    baseline = None
    for notify in action["stages"][1]["notifies"]:
        if not any(r.get("objectPath", "").lower() == SYSTEM
                   for r in notify.get("assetReferences", [])):
            continue
        ordinal = len(result)
        require(ordinal < 6, "Unexpected source notify count")
        expected_id = f"action-17250/stage-001/notify-{24 + ordinal:03}"
        require(notify["notifyId"] == expected_id, "Source notify identity changed")
        raw = base64.b64decode(notify["serializedPayload"]["data"])
        cue = decode_typed_payload(notify["sourceType"], notify["serializedPayload"],
                                   None, notify["assetReferences"], notify["serializedLabels"])
        start = cue["sourceTransformByteOffset"]
        variant = cue["sourceParameterCountByteOffset"] - start - 88
        require(start == 416 and variant == 4 and len(raw) == 690,
                "Source named-anchor layout changed")
        require(cue["enabled"] and cue["sourceParticleSystem"].lower() == f"particlesystem'{SYSTEM}'",
                "Source system disabled or changed")
        # Same field rule as build_kouku_albion_cross_groups.decode_particle.
        rotator = list(struct.unpack_from("<3i", raw, start + 40 + variant))
        require(rotator == [0, YAWS[ordinal], 0], "Source signed rotator changed")
        baseline = raw if baseline is None else baseline
        different = [i for i, (a, b) in enumerate(zip(raw, baseline)) if a != b]
        require(set(different) <= set(range(464, 468)), "Source notify differs beyond yaw")
        result[expected_id] = dict(sourceFRotator=rotator,
            rotationDegrees=[0.0, YAWS[ordinal] * 360 / 65536, 0.0],
            sourceFRotatorByteOffset=start + 40 + variant,
            sourcePayloadSHA256=sha(raw), differentBytesFromFirst=different,
            sourceTimeSeconds=notify["localTimeSeconds"])
    require(len(result) == 6, "Expected all six source notifications")
    return result, sha(raw_action)


def set_field(element, path, value, changes):
    owner = element
    for key in path[:-1]:
        owner = owner[key]
    exists = path[-1] in owner
    before = copy.deepcopy(owner.get(path[-1]))
    if exists and before == value:
        return
    changes.append(dict(path=".".join(path), beforeExists=exists,
                        before=before, after=copy.deepcopy(value)))
    owner[path[-1]] = copy.deepcopy(value)


def build(document, rotators):
    require(document["effectAssetId"] == ASSET, "Not the installed Alt V clip2 asset")
    result = copy.deepcopy(document)
    ids = [e["id"] for e in result["elements"]]
    require(len(ids) == len(set(ids)), "Duplicate element stable ID")
    patches, denominator = [], []
    pairs = set()
    for element in result["elements"]:
        marker = "|element:" + SYSTEM + ".particlespriteemitter_"
        if marker not in element.get("sourceNode", "").lower():
            continue
        emitter = int(element["sourceNode"].lower().split(marker)[1].split(".")[0])
        require(emitter in PROFILES or emitter == 16, "Unreviewed source emitter")
        presentation = element["sourcePresentation"]
        cue_id = presentation["sourceActionCueId"]
        require(cue_id in rotators, "Unexpected radial source cue")
        pair = (emitter, cue_id)
        require(pair not in pairs, "Duplicate source occurrence")
        pairs.add(pair)
        expected_profile = ("effect.ue3.kouku-5111-native.v1" if emitter == 16 else
                            f"effect.ue3.warlord-{PROFILES[emitter]}-native.v1")
        require(element["material"]["sourceProfile"]["runtimeShaderProfileId"] == expected_profile,
                "Source material identity changed")
        if emitter == 16:
            require(element["material"]["sourceMaterialPath"] == "fx_m_mi_05.fx_mi.fx_e_me_ht_03_4_ma",
                    "Restored debris MIC changed")
        require(element["detail"]["particle"]["localSpace"] is False,
                "Authored world-space birth policy changed")
        source = rotators[cue_id]
        rotation = source["rotationDegrees"]
        attachment = element["actionCueAttachment"]
        require(attachment["enabled"] and attachment["follow"] and
                attachment["sourceAnchorSlotId"] == "b_effectroot" and
                attachment["runtimeBoneName"] == "b_effectroot",
                "Source attachment owner changed")
        require(attachment["runtimeAnchorSlotId"] in ("b_effectroot", SLOT),
                "User-edited runtime slot")
        socket = attachment["socketLocalTransform"]
        require(socket["position"] == [0, 0, 0] and socket["scale"] == [1, 1, 1]
                and socket["rotationDegrees"] in ([0, 0, 0], [90, 180, 0]),
                "User-edited socket transform")
        require(element["detail"]["transform"]["rotationDegrees"] in ([0, 0, 0], rotation),
                "User-edited notify rotation")
        before = copy.deepcopy(element)
        changes = []
        set_field(element, ["detail", "transform", "rotationDegrees"], rotation, changes)
        set_field(element, ["actionCueAttachment", "runtimeAnchorSlotId"], SLOT, changes)
        set_field(element, ["actionCueAttachment", "socketLocalTransform", "rotationDegrees"],
                  [90, 180, 0], changes)
        if emitter in AXIS_EMITTERS:
            flags = [literal["value"] for module in element["sourceRecipe"]["modules"]
                     for literal in module["literals"]
                     if literal["propertyPath"] == "lockaxisflags"]
            require(flags == ["epal_z"], "Reviewed ground sprite axis changed")
            sprite = element["detail"]["sprite"]
            roll = sprite.get("billboardRollDegrees", 0)
            require(roll in (0, -rotation[1]) if emitter == 7 else roll == 0,
                    "User-edited billboard roll")
            # Replace G18's fixed notify-only compensation, not source StartRotation.
            if emitter == 7:
                set_field(element, ["detail", "sprite", "billboardRollDegrees"], 0.0, changes)
            set_field(element, ["detail", "sprite", "followEmitterAxisRotation"], True, changes)
        restored = copy.deepcopy(element)
        for change in reversed(changes):
            path = change["path"].split(".")
            owner = restored
            for key in path[:-1]:
                owner = owner[key]
            if change["beforeExists"]:
                owner[path[-1]] = change["before"]
            else:
                del owner[path[-1]]
        require(restored == before, "Unlisted field changed")
        row = dict(elementId=element["id"], emitter=emitter, sourceActionCueId=cue_id,
                   nativeProfile=expected_profile, source=source,
                   beforeSHA256=sha(encoded(before)), afterSHA256=sha(encoded(element)),
                   fields=changes)
        denominator.append(row)
        if changes:
            patches.append(row)
    required_pairs = {(emitter, cue) for emitter in PROFILES for cue in rotators}
    optional_pairs = {(16, cue) for cue in rotators}
    require(pairs in (required_pairs, required_pairs | optional_pairs),
            "Expected ten source emitters and either no debris or all six restored debris occurrences")
    return result, patches, denominator


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--action", required=True, type=Path)
    parser.add_argument("--input", type=Path, default=ROOT / "Data/Effects/Authored" / (ASSET + ".effect.json"))
    parser.add_argument("--out", type=Path, default=ROOT / "out/WarlordAltVRockDirections20261004")
    args = parser.parse_args()
    output = args.out.resolve()
    require(output.is_relative_to((ROOT / "out").resolve()), "Candidate output must remain under out")
    before = args.input.read_bytes()
    document = json.loads(before.decode("utf-8-sig"))
    rotators, action_sha = source_rotators(args.action)
    candidate, patches, denominator = build(document, rotators)
    repeated, repeat_patches, _ = build(candidate, rotators)
    require(repeated == candidate and not repeat_patches, "Candidate is not idempotent")
    data = encoded(candidate)
    require(args.input.read_bytes() == before, "Source document changed while preparing candidate")
    output.mkdir(parents=True, exist_ok=True)
    candidate_path = output / args.input.name
    (output / (args.input.name + ".before")).write_bytes(before)
    candidate_path.write_bytes(data)
    manifest = dict(schemaVersion=1, sourcePath=str(args.input.resolve()),
        beforeSHA256=sha(before), candidatePath=str(candidate_path), candidateSHA256=sha(data),
        sourceActionPath=str(args.action.resolve()), sourceActionSHA256=action_sha,
        effectAssetId=ASSET, elementCount=len(document["elements"]),
        radialOccurrenceCount=len(denominator), changedElementCount=len(patches),
        unchangedElementCount=len(document["elements"]) - len(patches),
        fieldPatchCount=sum(len(row["fields"]) for row in patches),
        emitterCounts=dict(collections.Counter(row["emitter"] for row in denominator)),
        sourceNotifies=rotators, patches=patches, denominator=denominator,
        validation=dict(idempotent=True, sourceRecipeMaterialResourcesAndUnlistedFieldsPreserved=True,
                        productInstallationPerformed=False,
                        actualPlaybackAndUserScreenValidation="Separate receipts required"))
    (output / "candidate-manifest.json").write_bytes(encoded(manifest))
    print(json.dumps({key: manifest[key] for key in (
        "candidatePath", "beforeSHA256", "candidateSHA256", "radialOccurrenceCount",
        "changedElementCount", "fieldPatchCount")}))


if __name__ == "__main__":
    main()

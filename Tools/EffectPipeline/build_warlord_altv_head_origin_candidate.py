"""Prepare the exact Alt V SDenergy head-origin correction; never install it.

The installed b_effectroot maps local -Z to character +Y. Re-express only
the notify translation, preserving particle axes, velocity and world-space
birth. This is a scoped placement adapter, not a change to the UE decoder.
"""
from pathlib import Path
import argparse
import copy
import hashlib
import json
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/LevelPlacementExtractor"))
from extract_action_effect_notifies import extract_action_document
from build_action_cue_recipe import decode_typed_payload

ASSET = "effect.warlord.skill.17250.clip1.full.restore"
CUE = "action-17250/stage-000/notify-025"
SYSTEM = "fx_pc_wgl_08.par_w_wgl_supergprotection_sdenergy"
PROFILES = {28: 1176, 7: 1177, 38: 665, 3: 666, 1: 667, 11: 664,
            12: 437, 0: 671, 5: 671, 6: 671, 19: 1178, 71: 458,
            74: 1179, 33: 1149, 35: 1020, 36: 1085, 18: 1180, 16: 1022}
BEFORE = [0, 1.55, 0]
AFTER = [0, 0, -1.55]


def require(condition, message):
    if not condition:
        raise ValueError(message)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def encoded(document):
    return (json.dumps(document, ensure_ascii=False, indent=2) + "\n").encode("utf-8")


def read_source(action_path):
    action = next(a for a in extract_action_document(action_path, "WARLORD")["actions"]
                  if a["actionId"] == 17250)
    notify = next(n for n in action["stages"][0]["notifies"] if n["notifyId"] == CUE)
    cue = decode_typed_payload(notify["sourceType"], notify["serializedPayload"],
                              None, notify["assetReferences"], notify["serializedLabels"])
    require(cue["enabled"] and cue["sourceParticleSystem"].lower() == f"particlesystem'{SYSTEM}'",
            "Source SDenergy notify changed")
    require(abs(notify["localTimeSeconds"] - 0.3457449973) < 1e-8,
            "Source notify time changed")
    # Keep the original decoded payload as provenance; do not rewrite its fields.
    require(cue["localTransform"]["sourcePositionUeUnits"] == [0, 0, 155],
            "Source notify translation changed")
    return dict(notifyId=CUE, localTimeSeconds=notify["localTimeSeconds"], typedPayload=cue)


def build(document):
    require(document["effectAssetId"] == ASSET, "Unexpected effect document")
    result = copy.deepcopy(document)
    ids = [e["id"] for e in result["elements"]]
    require(len(ids) == len(set(ids)), "Duplicate stable ID")
    patches, seen = [], set()
    for element in result["elements"]:
        if element.get("sourcePresentation", {}).get("sourceActionCueId") != CUE:
            continue
        marker = "|element:" + SYSTEM + ".particlespriteemitter_"
        node = element.get("sourceNode", "").lower()
        require(marker in node, "Unexpected ParticleSystem in head-origin notify")
        emitter = int(node.split(marker)[1].split(".")[0])
        require(emitter in PROFILES and emitter not in seen, "Unexpected or duplicate emitter")
        seen.add(emitter)
        profile = f"effect.ue3.warlord-{PROFILES[emitter]}-native.v1"
        require(element["material"]["sourceProfile"]["runtimeShaderProfileId"] == profile,
                "Source material identity changed")
        attachment = element["actionCueAttachment"]
        require(attachment["enabled"] and attachment["follow"] and
                all(attachment[key] == "b_effectroot" for key in
                    ("sourceAnchorSlotId", "runtimeAnchorSlotId", "runtimeBoneName")),
                "Source attachment changed")
        socket = attachment["socketLocalTransform"]
        require(socket["position"] == [0, 0, 0] and socket["rotationDegrees"] == [0, 0, 0]
                and socket["scale"] == [1, 1, 1], "Socket basis changed")
        require(element["detail"]["particle"]["localSpace"] is False,
                "World-space birth policy changed")
        transform = element["detail"]["transform"]
        require(transform["position"] in (BEFORE, AFTER), "User-edited notify position")
        if transform["position"] != AFTER:
            patches.append(dict(elementId=element["id"], emitter=emitter, profile=profile,
                                path="detail.transform.position",
                                before=transform["position"], after=AFTER.copy()))
            transform["position"] = AFTER.copy()
    require(seen == set(PROFILES), "Expected all 18 SDenergy emitters")
    return result, patches


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--action", type=Path, required=True)
    parser.add_argument("--input", type=Path,
                        default=ROOT / "Data/Effects/Authored" / (ASSET + ".effect.json"))
    parser.add_argument("--out", type=Path, default=ROOT / "out/WarlordAltVHeadOrigin20261005")
    args = parser.parse_args()
    output = args.out.resolve()
    require(output.is_relative_to((ROOT / "out").resolve()), "Candidate must remain under out")
    before = args.input.read_bytes()
    action_bytes = args.action.read_bytes()
    source = read_source(args.action)
    candidate, patches = build(json.loads(before))
    repeated, repeat_patches = build(candidate)
    require(repeated == candidate and not repeat_patches, "Candidate must be idempotent")
    require(args.input.read_bytes() == before, "Source document changed during preparation")
    require(args.action.read_bytes() == action_bytes, "Source Action changed during preparation")
    output.mkdir(parents=True, exist_ok=True)
    destination = output / args.input.name
    data = encoded(candidate)
    (output / (args.input.name + ".before")).write_bytes(before)
    destination.write_bytes(data)
    manifest = dict(sourcePath=str(args.input.resolve()), beforeSHA256=sha(before),
                    candidatePath=str(destination), candidateSHA256=sha(data),
                    sourceActionPath=str(args.action.resolve()),
                    sourceActionSHA256=sha(action_bytes), sourceNotify=source,
                    effectAssetId=ASSET, elementCount=len(candidate["elements"]),
                    changedElementCount=len(patches), patches=patches,
                    installed=False, userScreenVerified=False)
    (output / "candidate-manifest.json").write_bytes(encoded(manifest))
    print(json.dumps({k: manifest[k] for k in
                      ("candidatePath", "candidateSHA256", "changedElementCount")}))


if __name__ == "__main__":
    main()

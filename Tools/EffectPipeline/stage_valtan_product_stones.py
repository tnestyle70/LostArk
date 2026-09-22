"""Stage native Valtan stones on the existing Product cross-wave birth lattice.

This command writes candidates and a baseline receipt only. The caller owns
saved-document approval, stable-ID merge and domain publishing.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
CROSS = Path("Data/Effects/Authored/effect.valtan.sequence.cross.effect.json")
DONOR = Path("Data/Effects/Authored/effect.valtan.ground-roar.rock.active.effect.json")
STONE_GROUP = "valtan.cross.rock-wave"
PROGRAM = "effect.ue3.kouku-2391-native.v1"


def sha256(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def set_constant(distribution: dict, value: float) -> None:
    distribution.update(
        operation=1, lookupTableChunkSize=1, lookupTableNumElements=1,
        lookupTableTimeScale=0.0, lookupTableStartTime=0.0,
        lookupTable=[value, value, value, value], keys=[],
    )


def set_gap_override(element: dict) -> None:
    scalars = element["material"]["sourceProfile"]["scalars"]
    gap = next(row for row in scalars if row["name"] == "09.gap_offset")
    gap["value"] = 0.25
    overrides = element.setdefault("authoringOverrides", {
        "resources": [], "colors": [], "scalars": [],
    })
    own = next((row for row in overrides["scalars"] if row["name"] == "09.gap_offset"), None)
    if own is None:
        overrides["scalars"].append({"name": "09.gap_offset", "value": 0.25,
                                    "compilerValue": 0.17499999701976776})
    else:
        own["value"] = 0.25


def project_cross(document: dict, donor_document: dict, scale: float) -> dict:
    candidate = copy.deepcopy(document)
    donor = next(row for row in donor_document["elements"]
                 if row["id"] == "ground-roar.rock.active.mesh")
    if donor["material"]["sourceProfile"]["runtimeShaderProfileId"] != PROGRAM:
        raise ValueError("The persistent stone donor no longer uses native 2391")
    targets = [row for row in candidate["elements"] if row.get("groupId") == STONE_GROUP]
    if len(targets) != 4:
        raise ValueError("Cross must retain its four existing stone emitters")
    for element in targets:
        detail = element["detail"]
        life = detail["particle"]["lifeTimeSeconds"]
        if life[0] != life[1] or not 0 < detail["particle"]["fixedCenterSpacingWorldUnits"]:
            raise ValueError("Cross stone requires its existing fixed spacing and deterministic life")
        element["material"] = copy.deepcopy(donor["material"])
        element["resources"] = copy.deepcopy(donor["resources"])
        # Generic base/mask edits targeted slots that the native material no
        # longer owns. Replace those along with the requested material itself.
        element["authoringOverrides"] = {
            "resources": [], "colors": [], "scalars": [],
        }
        set_gap_override(element)
        detail["color"] = copy.deepcopy(donor["detail"]["color"])
        detail["transform"]["scale"] = [value * scale for value in detail["transform"]["scale"]]
        recipe = copy.deepcopy(donor["sourceRecipe"])
        recipe.update(emitterDelaySeconds=0.0,
                      emitterDurationSeconds=detail["timing"]["lifeTimeSeconds"],
                      emitterLoopCount=1, bursts=[])
        for module in recipe["modules"]:
            cls = module["className"]
            module["stableId"] = f"authored.{element['id']}.{cls}"
            module["objectPath"] = module["stableId"]
            for literal in module["literals"]:
                if cls == "particlemodulerequired":
                    if literal["propertyPath"] == "emitterduration":
                        literal["value"] = recipe["emitterDurationSeconds"]
                    if literal["propertyPath"] == "buselocalspace":
                        literal["value"] = False
                if cls == "particlemodulespawn" and literal["propertyPath"].startswith("burstlist["):
                    if literal["propertyPath"].endswith((".count", ".countlow")):
                        literal["value"] = 0.0
            for distribution in module["distributions"]:
                prop = distribution["propertyPath"]
                if cls == "particlemodulelifetime" and prop == "lifetime":
                    set_constant(distribution, life[0])
                elif cls == "particlemodulespawn" and prop == "rate":
                    set_constant(distribution, 0.0)
                elif cls == "particlemoduleparameterdynamic" and prop == "dynamicparams[0].paramvalue":
                    # Keep the corrected donor's reveal endpoints; the cross
                    # owns when its existing per-particle terminal dissolve starts.
                    start = detail["timing"]["dissolveStartNormalized"]
                    distribution["lookupTableStartTime"] = start
                    distribution["lookupTableTimeScale"] = 1.0 / (1.0 - start)
        element["sourceRecipe"] = recipe
    return candidate


def replace_element_bytes(raw: bytes, replacements_by_id: dict[str, dict]) -> bytes:
    """Replace named element objects and preserve every untouched source byte."""
    document = json.loads(raw.decode("utf-8-sig"))
    text = raw.decode("utf-8-sig")
    decoder = json.JSONDecoder()
    cursor = text.index("[", text.index('"elements"')) + 1
    replacements = []
    for _ in document["elements"]:
        while text[cursor] in " \r\n\t,":
            cursor += 1
        old, length = decoder.raw_decode(text[cursor:])
        if old["id"] in replacements_by_id:
            replacement = json.dumps(replacements_by_id[old["id"]], ensure_ascii=False, indent=2)
            replacements.append((cursor, cursor + length, replacement.replace("\n", "\n    ")))
        cursor += length
    for begin, end, replacement in reversed(replacements):
        text = text[:begin] + replacement + text[end:]
    staged = text.encode("utf-8")
    if raw.startswith(b"\xef\xbb\xbf"):
        staged = b"\xef\xbb\xbf" + staged
    return staged


def write_staged(output: Path, relative: Path, raw: bytes, staged: bytes, ids: list[str]) -> dict:
    target = output / "candidates" / relative
    backup = output / "baseline" / relative
    target.parent.mkdir(parents=True, exist_ok=True)
    backup.parent.mkdir(parents=True, exist_ok=True)
    if backup.exists() and backup.read_bytes() != raw:
        raise ValueError("Baseline changed; stage in a fresh output directory")
    backup.write_bytes(raw)
    target.write_bytes(staged)
    return {
        "source": relative.as_posix(), "sourceSha256": sha256(raw),
        "baseline": str(backup.resolve()), "candidate": str(target.resolve()),
        "candidateSha256": sha256(staged), "elementIds": ids,
    }


def stage(root: Path, output: Path, donor_path: Path, scale: float) -> dict:
    raw = (root / CROSS).read_bytes()
    donor_raw = donor_path.read_bytes()
    document = json.loads(raw.decode("utf-8-sig"))
    candidate = project_cross(document, json.loads(donor_raw.decode("utf-8-sig")), scale)
    cross_rows = {row["id"]: row for row in candidate["elements"] if row.get("groupId") == STONE_GROUP}
    staged = replace_element_bytes(raw, cross_rows)
    files = [write_staged(output, CROSS, raw, staged, list(cross_rows))]
    for pattern in ("ground-roar", "six-pizza", "struggling"):
        relative = Path(f"Data/Effects/Authored/effect.valtan.{pattern}.rock.active.effect.json")
        active_raw = (root / relative).read_bytes()
        active = json.loads(active_raw.decode("utf-8-sig"))
        stone = next(row for row in active["elements"] if row["id"] == pattern + ".rock.active.mesh")
        if stone["material"]["sourceProfile"]["runtimeShaderProfileId"] != PROGRAM:
            raise ValueError("Persistent stone's native material identity changed")
        set_gap_override(stone)
        transform = stone["detail"]["transform"]
        transform["scale"] = [value * scale for value in transform["scale"]]
        active_staged = replace_element_bytes(active_raw, {stone["id"]: stone})
        files.append(write_staged(output, relative, active_raw, active_staged, [stone["id"]]))
    receipt = {
        **files[0], "files": files,
        "donor": str(donor_path.resolve()), "donorSha256": sha256(donor_raw),
        "elementIds": [row["id"] for row in candidate["elements"] if row.get("groupId") == STONE_GROUP],
        "visualScaleMultiplier": scale, "serverGameplayChanged": False,
        "preserved": ["four birth paths", "world spacing", "emission duration", "particle lifetime", "smoke lane", "cue ID and timing"],
        "installed": False,
    }
    (output / "cross-staging-receipt.json").write_text(
        json.dumps(receipt, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    (output / "manifest.json").write_text(
        json.dumps(receipt, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return receipt


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repository-root", type=Path, default=ROOT)
    parser.add_argument("--output", type=Path, default=ROOT / "out/ValtanStoneProduct20260922")
    parser.add_argument("--donor", type=Path)
    parser.add_argument("--scale", type=float, default=1.2)
    args = parser.parse_args()
    if not 0.0 < args.scale <= 10.0:
        parser.error("scale must be finite, positive and no greater than 10")
    receipt = stage(args.repository_root, args.output,
                    args.donor or args.repository_root / DONOR, args.scale)
    print(json.dumps(receipt, ensure_ascii=True))

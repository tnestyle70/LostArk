"""Stage source-texture Ctrl reticle/ping on the existing native sprite carriers.

Writes candidates only. Timings, placement, white reticle and selected atlas cell
are project authoring; copied material programs and original DDS remain unchanged.
"""
from __future__ import annotations

import copy
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "out/KoukuMarkers20260922"


def constant(distribution: dict, values: list[float]) -> None:
    n = len(values)
    distribution.update(componentCount=n, operation=1, lookupTableChunkSize=n,
                        lookupTableNumElements=1, lookupTableTimeScale=0,
                        lookupTableStartTime=0, defaultMinimum=values + [0] * (4 - n),
                        defaultMaximum=values + [0] * (4 - n),
                        lookupTable=[0, 0] + values + values, keys=[])


def stage_element(donor: dict, asset: str, seconds: float, size_cm: float,
                  atlas_index: int | None = None) -> dict:
    e = copy.deepcopy(donor)
    e.update(id=asset + ".symbol", displayName=asset + " / source texture",
             groupId="manual." + asset, visible=True)
    e["actionCueAttachment"]["enabled"] = False
    e["transformInheritance"] = {"enabled": False, "masterElementId": ""}
    detail = e["detail"]
    detail["transform"]["position"] = [0, 0, 0]
    detail["timing"].update(startDelaySeconds=0, lifeTimeSeconds=seconds)
    detail["particle"].update(maxParticles=2, lifeTimeSeconds=[seconds, seconds],
                              startSize=[size_cm * .01, size_cm * .01],
                              endSize=[size_cm * .01, size_cm * .01],
                              initialVelocityMin=[0, 0, 0], initialVelocityMax=[0, 0, 0],
                              initialPositionMin=[0, 0, 0], initialPositionMax=[0, 0, 0],
                              localSpace=True)
    recipe = e["sourceRecipe"]
    recipe.update(emitterDelaySeconds=0, emitterDurationSeconds=seconds,
                  emitterLoopCount=0 if atlas_index is None else 1,
                  bursts=[{"timeSeconds": 0, "countMinimum": 1, "countMaximum": 1}])
    recipe["modules"] = [m for m in recipe["modules"] if m["className"] not in {
        "particlemodulevelocity", "particlemodulevelocityoverlifetime",
        "particlemodulecolorscaleoverlife"}]
    for module in recipe["modules"]:
        for literal in module["literals"]:
            if literal["propertyPath"] == "emitterduration": literal["value"] = seconds
        for d in module["distributions"]:
            key = d["propertyPath"]
            if key == "lifetime": constant(d, [seconds])
            elif key == "startsize": constant(d, [size_cm, size_cm, size_cm])
            elif key == "startlocation": constant(d, [0, 0, 0])
            elif key == "alphaoverlife": constant(d, [1])
            elif key == "coloroverlife": constant(d, [1, 1, 1])
            elif key == "subimageindex" and atlas_index is not None: constant(d, [atlas_index])
    return e


def main() -> None:
    load = lambda name: json.loads((ROOT / "Data/Effects/Authored" / (name + ".effect.json")).read_text("utf-8-sig"))
    sight = load("effect.kouku.source.fx_mn_cdebz_00.par_v_cdebz_sight_01")
    guide = load("effect.kouku.source.fx_cm_04.sys.par_t_singlemode_guide_01")
    target = "effect.world.target_reticle"
    ping = "effect.world.ping"
    reticle = stage_element(sight["elements"][1], target, 1, 55)
    for vector in reticle["material"]["sourceProfile"]["vectors"]:
        if vector["name"] in {"center_color", "red_ch_color"}:
            vector["value"] = [1, 1, 1, 1]
    for scalar in reticle["material"]["sourceProfile"]["scalars"]:
        if scalar["name"] == "line strength": scalar["value"] = 0
    marker = stage_element(guide["elements"][-1], ping, 3, 95, 4)
    files = []
    for asset, title, element in [(target, "Target Reticle / Ctrl Pending", reticle),
                                  (ping, "Ping / Ctrl + Left Click", marker)]:
        doc = {"schema": "lostark.effect-authoring", "version": 13,
               "effectAssetId": asset, "displayName": title,
               "particleSystem": {"uniformScaleMultiplier": 1, "yawOffsetDegrees": 0,
                                  "directionYawDegrees": 0, "initialSpeedMultiplier": 1},
               "modelCues": [], "elements": [element]}
        relative = f"Data/Effects/Authored/{asset}.effect.json"
        destination = OUT / "candidate" / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        payload = (json.dumps(doc, ensure_ascii=False, indent=2) + "\n").encode()
        destination.write_bytes(payload)
        files.append({"path": relative, "beforeSha256": None,
                      "candidateSha256": hashlib.sha256(payload).hexdigest()})
        for texture in element["material"]["sourceProfile"]["textures"]:
            assert (ROOT / "Client/Bin/Resources" / texture["assetId"]).is_file(), texture
    manifest = {"files": files, "newAssetIds": [target, ping],
                "resourceTreeParent": "world", "newResources": [],
                "fidelity": "PROJECT_AUTHORED timings/size/white reticle/atlas-cell; existing source textures and native programs",
                "runtime": "local Ctrl pending reticle and navigation-ground ping; no move or combat command"}
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(manifest, indent=2))


if __name__ == "__main__":
    main()

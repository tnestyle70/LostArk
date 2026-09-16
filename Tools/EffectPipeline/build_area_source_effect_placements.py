"""Project source-qualified, autonomous UE3 ParticleSystem actors into MapEffects.

Input components are the decoded source PSC/Emitter records. The installation
receipt selects the exact source systems whose Effect documents were restored.
Scripted activation, actor bases and per-instance parameters require their own
typed consumers and are reported instead of being replaced by defaults.
"""
import argparse
import hashlib
import json
import math
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/LevelPlacementExtractor"))
from build_maptool_scene import convert_position, convert_rotation, convert_scale


def parameter_bindings(parameters):
    bindings = []
    names = set()
    for parameter in parameters:
        name = parameter["name"].lower()
        if name in names:
            raise ValueError(f"Duplicate source instance parameter: {name}")
        names.add(name)
        kind = parameter["paramtype"].lower()
        if kind == "pspt_scalar":
            binding = dict(name=name, type="scalar", scalarValue=float(parameter["scalar"]))
            numbers = [binding["scalarValue"]]
        elif kind == "pspt_vector":
            binding = dict(name=name, type="vector", vectorValue=[
                float(parameter["vector"][axis]) for axis in ("x", "y", "z")])
            numbers = binding["vectorValue"]
        else:
            raise ValueError(f"Unsupported source instance parameter: {name}/{kind}")
        if not all(math.isfinite(value) for value in numbers):
            raise ValueError(f"Non-finite source instance parameter: {name}")
        bindings.append(binding)
    return sorted(bindings, key=lambda binding: binding["name"])


def parameter_signature(parameters):
    if not parameters:
        return ""
    encoded = json.dumps(parameter_bindings(parameters), sort_keys=True, separators=(",", ":"))
    return hashlib.sha256(encoded.encode()).hexdigest()[:20]


def build(area_id, components, installations, controls=()):
    restored = {}
    for installation in installations:
        for row in installation["documents"]:
            system = row["sourceParticleSystem"].lower()
            key = (system, row.get("sourceInstanceParameterSignature", ""))
            if key in restored and restored[key]["asset"] != row["effectAssetId"]:
                raise ValueError(f"Source system has conflicting restored assets: {system}")
            document = json.loads((ROOT / row["path"]).read_text(encoding="utf-8-sig"))
            if document.get("modelCues") or not document["elements"] or any(
                    not element.get("sourceRecipe", {}).get("enabled") or
                    element["sourceRecipe"]["rendererShape"] not in ("sprite", "mesh")
                    for element in document["elements"]):
                raise ValueError(f"Ambient placement requires native sprite/mesh emitter closure: {system}")
            policy = "SOURCE_LOOP" if any(element["sourceRecipe"]["emitterLoopCount"] == 0
                                          for element in document["elements"]) else "SOURCE_ONCE"
            restored[key] = dict(asset=row["effectAssetId"], policy=policy)
    restored_systems = {key[0] for key in restored}
    controlled = {(row["sourceLevel"], ref["packageIndex"])
                  for row in controls for ref in row["targetReferences"]}
    presentations, excluded, sources = [], [], []
    for row in components:
        system = row["template"].lower()
        if system not in restored_systems:
            continue
        component, actor = row["componentProperties"], row["actorProperties"]
        source_id = f"{row['sourceLevel']}:export:{row['componentExport']}"
        restored_key = (system, parameter_signature(component.get("instanceparameters", [])))
        reason = None
        if not component.get("bautoactivate", True):
            reason = "SOURCE_AUTO_ACTIVATE_FALSE"
        elif restored_key not in restored:
            reason = "SOURCE_INSTANCE_PARAMETERS_REQUIRE_CONSUMER"
        elif actor.get("base"):
            reason = "SOURCE_ACTOR_BASE_REQUIRES_CONSUMER"
        elif any((row["sourceLevel"], row[key]) in controlled
                 for key in ("componentExport", "actorExport")):
            reason = "SOURCE_SEQUENCE_CONTROL_REQUIRES_CONSUMER"
        elif any(key in component for key in ("translation", "rotation", "scale", "scale3d")):
            reason = "SOURCE_COMPONENT_LOCAL_TRANSFORM_REQUIRES_COMPOSITION"
        if reason:
            excluded.append(dict(sourcePlacementId=source_id, reason=reason))
            continue
        position = convert_position(actor["location"])
        rotation = convert_rotation(actor.get("rotation", dict(pitch=0, yaw=0, roll=0)))
        uniform = actor.get("drawscale", 1.0)
        scale = convert_scale({axis: uniform * actor.get("drawscale3d", {}).get(axis, 1.0)
                               for axis in ("x", "y", "z")})
        if any(value < 0.001 for value in scale):
            excluded.append(dict(sourcePlacementId=source_id, reason="SOURCE_SIGNED_SCALE_REQUIRES_CONSUMER"))
            continue
        distance = float(component.get("cachedmaxdrawdistance", 0.0)) * 0.01
        if not math.isfinite(distance) or distance < 0:
            raise ValueError(f"Invalid native draw distance: {source_id}")
        identity = "source-particle." + hashlib.sha256(source_id.encode()).hexdigest()[:24]
        presentations.append(dict(
            independentEffectId=identity, displayName=system.rsplit(".", 1)[-1],
            presentationKind="EFFECT_DOCUMENT", placementId=identity,
            effectAssetId=restored[restored_key]["asset"], position=list(position),
            rotationQuaternion=list(rotation), scale=list(scale),
            orientationPolicy="WORLD", activationPolicy="LEVEL_ACTIVE", activationSetId="",
            activationWindows=[], playbackPolicy=restored[restored_key]["policy"], maxDrawDistanceMeters=distance))
        sources.append(dict(placementId=identity, sourcePlacementId=source_id,
                            sourceParticleSystem=system, componentSha256=row.get("componentSha256"),
                            actorSha256=row.get("actorSha256")))
    if len({p["placementId"] for p in presentations}) != len(presentations):
        raise ValueError("Duplicate source placement identity")
    return (dict(schema="lostark.map-effect-presentation", formatVersion=1,
                 areaId=area_id, presentations=presentations),
            dict(areaId=area_id, placementCount=len(presentations), sources=sources,
                 excluded=excluded, visualValidation="USER_PENDING"))


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument("--area-id", required=True)
    parser.add_argument("--components", type=Path, required=True)
    parser.add_argument("--installation", type=Path, action="append", required=True)
    parser.add_argument("--controls", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--receipt", type=Path, required=True)
    args = parser.parse_args()
    read = lambda path: json.loads(path.read_text(encoding="utf-8-sig"))
    document, receipt = build(args.area_id, read(args.components),
                              [read(p) for p in args.installation],
                              read(args.controls) if args.controls else ())
    for path, data in ((args.output, document), (args.receipt, receipt)):
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(dict(placements=receipt["placementCount"], excluded=len(receipt["excluded"]))))


if __name__ == "__main__":
    main()

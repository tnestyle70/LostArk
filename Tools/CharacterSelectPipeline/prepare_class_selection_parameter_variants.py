"""Create PSC-specific immutable source-distribution bindings in staged Effects."""
from __future__ import annotations

import argparse
import copy
import hashlib
from pathlib import Path

from project_guardian_selection import read, write, rows_from


def normalized_mapping(binding, count):
    props = binding["effectiveDistribution"]
    modes = [mode.upper().removeprefix("DPM_") for mode in binding["effectiveParameterModes"]]
    if len(modes) != count or any(mode not in ("DIRECT", "NORMAL") for mode in modes):
        raise ValueError((binding["distribution"], "Unsupported reviewed parameter modes", modes))

    def vector(field, default):
        value = props.get(field, default)
        return [float(value.get(axis, default)) for axis in "xyz"[:count]] if isinstance(value, dict) else [float(value)]*count

    return dict(modes=modes, minInput=vector("mininput", 0.), maxInput=vector("maxinput", 1.),
                minOutput=vector("minoutput", 0.), maxOutput=vector("maxoutput", 1.)), vector("constant", 0.)+[0.]*(4-count)


def initial_binding_rows(source, instances):
    components = {r["p"].get("particlesystemcomponent")-1: index-1
        for index, r in rows_from(read(source)).items() if r["p"].get("particlesystemcomponent")}
    output, ignored = [], []
    for row in read(instances):
        bindings = []
        for dist in row["bindings"]:
            if not dist["declaredTypeMatchesDistribution"]:
                ignored.append(dict(componentIndex0=row["sourceComponentIndex0"], parameter=row["parameterName"],
                    distribution=dist["distribution"], reason="Typed PSC lookup fails; original distribution constant remains"))
                continue
            count = 1 if dist["expectedParameterType"] == "PSPT_SCALAR" else 3
            props = dist["effectiveDistribution"]
            # UE's ParticleParameter mode enum starts at NORMAL (zero). Its
            # recovered scalar/vector CDO contains no overriding mode fields.
            modes = ([props.get("parammode", "dpm_normal")] if count == 1 else
                [props.get("parammodes" if i == 0 else f"parammodes[{i}]", "dpm_normal") for i in range(3)])
            for module in dist["modules"]:
                if not module["moduleUsers"]:
                    continue
                bindings.append(dict(module=module["module"], propertyPath=module["propertyPath"],
                    distribution=dist["distribution"], distributionClass=dist["distributionClass"],
                    effectiveDistribution=props, effectiveParameterModes=modes))
        if bindings:
            output.append(dict(sourceSystem=row["sourceSystem"], parameterName=row["parameterName"],
                actorExportIndex0=components[row["sourceComponentIndex0"]], bindings=bindings))
    return output, ignored


def prepare(base_effects, library_path, bindings_path, output, instance_bindings=None, source=None):
    library = read(library_path)
    rows = read(bindings_path) if bindings_path else []
    original_hashes = {str(p): hashlib.sha256(p.read_bytes()).hexdigest()
                       for p in (library_path, bindings_path, instance_bindings, source) if p is not None}
    ignored = []
    if instance_bindings:
        if not source:
            raise ValueError("Initial PSC bindings require their source actor/component identities")
        initials, ignored = initial_binding_rows(source, instance_bindings)
        rows += initials
    assets = {row["assetId"]: row for row in library["assets"]}
    grouped = {}
    for row in rows:
        grouped.setdefault(row["actorExportIndex0"], []).append(row)
    receipts = []
    library.setdefault("actorAssets", {})
    for actor, actor_rows in sorted(grouped.items()):
        systems = {row["sourceSystem"] for row in actor_rows}
        if len(systems) != 1:
            raise ValueError((actor, "A PSC changed its source template across phases"))
        system = next(iter(systems))
        matches = [asset for asset in assets if asset.endswith("."+system)]
        if len(matches) != 1:
            raise ValueError((actor, "Source effect metadata is not unique", system, matches))
        base_asset = matches[0]
        source = base_effects / (base_asset+".effect.json")
        original_hashes[str(source)] = hashlib.sha256(source.read_bytes()).hexdigest()
        document = read(source)
        variant = base_asset+f".psc{actor}"
        document["effectAssetId"] = variant
        document["displayName"] = document.get("displayName", base_asset)+f" | source PSC {actor}"
        mappings = {}
        for row in actor_rows:
            for binding in row["bindings"]:
                # Source reference paths include the FRawDistribution's object
                # member; the runtime recipe identifies its containing field.
                key = (binding["module"], binding["propertyPath"].removesuffix(".distribution"), binding["distribution"])
                incoming = (row["parameterName"], binding)
                if key in mappings:
                    previous_name, previous = mappings[key]
                    count = 1 if "float" in binding["distributionClass"].lower() else 3
                    if previous_name != incoming[0] or normalized_mapping(previous, count) != normalized_mapping(binding, count):
                        raise ValueError((actor, key, "Conflicting phase parameter mapping"))
                mappings[key] = incoming
        consumed = set()
        for element in document["elements"]:
            old_id = element["id"]
            if old_id.startswith(base_asset+"."):
                element["id"] = variant+old_id[len(base_asset):]
            element["groupId"] = variant
            for module in element["sourceRecipe"]["modules"]:
                for dist in module["distributions"]:
                    key = (module["objectPath"], dist["propertyPath"], dist.get("sourceObjectPath"))
                    if key not in mappings:
                        continue
                    name, binding = mappings[key]
                    count = dist["componentCount"]
                    mapping, constant = normalized_mapping(binding, count)
                    if dist.get("keys") or dist.get("lookupTable"):
                        raise ValueError((actor, key, "ParticleParameter has a competing baked curve/table"))
                    dist.update(parameterBinding="worldSample", parameterName=name,
                        parameterMapping=mapping, operation=1, randomLockAxes=0,
                        lookupTableChunkSize=0, lookupTableNumElements=0,
                        lookupTableTimeScale=0., lookupTableStartTime=0.)
                    dist["defaultMinimum"] = constant
                    dist["defaultMaximum"] = constant
                    consumed.add(key)
        missing = set(mappings)-consumed
        if missing:
            raise ValueError((actor, "Original parameter targets absent from the admitted Effect", sorted(missing)))
        write(output / (variant+".effect.json"), document)
        library["assets"].append(dict(assets[base_asset], assetId=variant))
        library["actorAssets"][str(actor)] = variant
        receipts.append(dict(actorExportIndex0=actor, sourceEffect=base_asset, effectAssetId=variant,
            distributionBindings=len(consumed), parameterNames=sorted({name for name, _ in mappings.values()})))
    for path, expected in original_hashes.items():
        if hashlib.sha256(Path(path).read_bytes()).hexdigest() != expected:
            raise ValueError((path, "Source changed during candidate preparation"))
    write(output / "effect-library.json", library)
    write(output / "parameter-variant-receipt.json", dict(installed=False, sourceHashes=original_hashes,
        variants=receipts, originalTypedFallbacks=ignored))
    return receipts


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base-effects", type=Path, required=True)
    parser.add_argument("--library", type=Path, required=True)
    parser.add_argument("--bindings", type=Path)
    parser.add_argument("--instance-bindings", type=Path)
    parser.add_argument("--source", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    print("Prepared PSC variants:", len(prepare(args.base_effects, args.library, args.bindings, args.output,
        args.instance_bindings, args.source)))

"""Fold fixed source PSC scalar/vector overrides into immutable Effect variants.

Uses the same acquired CDO/module closure, DPM parameter transfer, native
material patch and Effect document projector as character full restoration.
Each asset ID contains its stable source parameter signature; existing edited
documents retain the common projector's preserve-on-conflict behavior.
"""
import argparse
import copy
import json
from pathlib import Path

import build_kouku_all_source_effects as library
from build_area_source_effect_placements import parameter_bindings, parameter_signature


def project(components, source_root, native_patch, evidence_root, install):
    read = library.source.read
    targets = {int(key): value for key, value in read(source_root / "library_targets.json").items()}
    target_by_system = {value["system"]: (key, value) for key, value in targets.items()}
    notifies = read(source_root / "source_notifies.json")
    groups = {}
    for component in components:
        properties = component["componentProperties"]
        parameters = properties.get("instanceparameters", [])
        system = component["template"].lower()
        if (not parameters or system not in target_by_system or
                not properties.get("bautoactivate", True) or component["actorProperties"].get("base")):
            continue
        signature = parameter_signature(parameters)
        group = groups.setdefault((system, signature), dict(parameters=parameters, components=[]))
        group["components"].append(component)
    documents, failures = [], []
    for (system, signature), group in sorted(groups.items()):
        identity, base_target = target_by_system[system]
        target = copy.deepcopy(base_target)
        target["asset"] += ".instance." + signature
        target["name"] += " [source instance " + signature[:8] + "]"
        bindings = parameter_bindings(group["parameters"])
        own_notifies = [copy.deepcopy(row) for row in notifies if row["actionId"] == identity]
        for notify in own_notifies:
            for field in ("serializedPayload", "cue"):
                if field in notify:
                    notify[field]["parameterOverrides"] = copy.deepcopy(bindings)
                    notify[field]["parameterOverridesDecoded"] = True
        destination = evidence_root / signature / str(identity)
        destination.mkdir(parents=True, exist_ok=True)
        library.source.write(destination / "source_instance_parameter_input.json", dict(
            sourceParticleSystem=system, signature=signature, parameterOverrides=bindings,
            components=[dict(sourceLevel=row["sourceLevel"], componentExport=row["componentExport"],
                             componentSha256=row.get("componentSha256")) for row in group["components"]]))

        def read_with_instance(path):
            if Path(path).resolve() == (source_root / "source_notifies.json").resolve():
                return copy.deepcopy(own_notifies)
            return read(path)

        library.source.read = read_with_instance
        try:
            library.project(destination, {identity: target}, native_patch, install, cached_source=source_root)
        finally:
            library.source.read = read
        receipt = read(destination / "installation.json")
        for row in receipt["documents"]:
            row["sourceInstanceParameterSignature"] = signature
            row["sourceParameterOverrides"] = bindings
            documents.append(row)
        failures.extend(receipt["sourceFailures"])
    result = dict(installed=install, sourceVariantCount=len(groups), documents=documents,
                  sourceFailures=failures, manualVisualValidation="USER_PENDING")
    library.source.write(evidence_root / "installation.json", result)
    print(json.dumps(dict(variants=len(documents), sourceFailures=len(failures), installed=install)))
    return result


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument("--components", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--native-material-patch", type=Path, required=True)
    parser.add_argument("--evidence-root", type=Path, required=True)
    parser.add_argument("--install", action="store_true")
    args = parser.parse_args()
    project(library.source.read(args.components), args.source_root.resolve(),
            args.native_material_patch.resolve(), args.evidence_root.resolve(), args.install)


if __name__ == "__main__":
    main()

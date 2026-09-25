"""Join reviewed movie candidates without writing live authoring or Resources.

The resulting manifest is consumed by install_guardian_selection.py. Native
runtime admission remains a separate required check after publication.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

from install_guardian_selection import ROOT, AREA, read, write, merge_rows


def unique(rows, key):
    result = {row[key]: row for row in rows}
    if len(result) != len(rows):
        raise ValueError(f"Duplicate delivery identity: {key}")
    return result


def json_values(value):
    if isinstance(value, dict):
        return 1 + sum(json_values(v) for v in value.values())
    if isinstance(value, list):
        return 1 + sum(map(json_values, value))
    return 1


def assemble(config_path, output):
    config_path, output = config_path.resolve(), output.resolve()
    if output.is_relative_to(ROOT / "Data") or output.is_relative_to(ROOT / "Client/Bin"):
        raise ValueError("Delivery assembly must stay outside live data and resources")
    config = read(config_path)
    inputs = {str(config_path): hashlib.sha256(config_path.read_bytes()).hexdigest()}

    def path(value, parent=config_path.parent):
        p = Path(value)
        return p.resolve() if p.is_absolute() else (parent / p).resolve()

    def pin(p):
        data = p.read_bytes()
        inputs[str(p)] = hashlib.sha256(data).hexdigest()
        return data

    def pinned(p):
        return json.loads(pin(p))

    cinema = None
    world = None
    effect_paths = {}
    classes, boundaries = [], {}
    roots = {str(path(p)) for p in config["resourceRoots"]}
    for entry in config["movies"]:
        directory = path(entry["directory"])
        receipt = pinned(directory / "projection-receipt.json")
        if not receipt.get("candidateInputsReady"):
            raise ValueError((directory, "Movie source inputs are not admitted", receipt.get("materialProjectionGaps")))
        scene = pinned(directory / "ClassSelection.cinematics.json")
        sequence = pinned(directory / f"{AREA}.worldsequences.json")
        if len(scene["scenes"]) != 1:
            raise ValueError((directory, "Expected one reviewed category"))
        class_id = scene["scenes"][0]["classId"]
        if class_id in classes:
            raise ValueError((class_id, "Repeated class candidate"))
        classes.append(class_id)
        boundaries[class_id] = receipt["unconsumedSourceFeatures"]
        if cinema is None:
            cinema, world = scene, sequence
        else:
            for incoming, combined in ((scene, cinema), (sequence, world)):
                for key in ("schema", "formatVersion", "areaId"):
                    if incoming[key] != combined[key]:
                        raise ValueError((directory, "Document header differs", key))
            cinema["scenes"] += scene["scenes"]
            for key in ("objectResources", "templates", "instances"):
                world[key] += sequence[key]
            world["revision"] = max(world["revision"], sequence["revision"])
        library = pinned(path(entry["effectLibrary"]))
        if not library.get("nativeReady"):
            raise ValueError((class_id, "Native Effect library is not admitted"))
        supplied = {}
        for effect_dir in entry["effectDirectories"]:
            for effect_path in sorted(path(effect_dir).glob("*.effect.json")):
                effect = pinned(effect_path)
                identity = effect["effectAssetId"]
                if identity in supplied and pinned(supplied[identity]) != effect:
                    raise ValueError((identity, "Conflicting candidate Effects"))
                supplied[identity] = effect_path
        for asset in library["assets"]:
            identity = asset["assetId"]
            if identity not in supplied:
                raise ValueError((class_id, "Missing admitted Effect document", identity))
            if identity in effect_paths and pinned(effect_paths[identity]) != pinned(supplied[identity]):
                raise ValueError((identity, "Effect identity differs across classes"))
            effect_paths[identity] = supplied[identity]

    if not classes:
        raise ValueError("No movie candidates supplied")
    objects = unique(world["objectResources"], "objectId")
    templates = unique(world["templates"], "sequenceId")
    instances = unique(world["instances"], "instanceId")
    for instance in instances.values():
        if instance["templateId"] not in templates:
            raise ValueError((instance["instanceId"], "Missing template"))
        for binding in instance["bindings"]:
            if binding["targetKind"] == "OBJECT_RESOURCE" and binding["targetId"] not in objects:
                raise ValueError((instance["instanceId"], "Missing bound object"))
    for scene in cinema["scenes"]:
        for name in ("intro", "loop"):
            phase = scene[name]
            if not phase["instanceIds"] or len(phase["instanceIds"]) > 128:
                raise ValueError((scene["classId"], name, "Instance count is outside product limit"))
            for identity in phase["instanceIds"]:
                if identity not in instances:
                    raise ValueError((scene["classId"], name, "Missing movie instance", identity))
            for effect in phase.get("effects", []):
                if effect["assetId"] not in effect_paths:
                    raise ValueError((scene["classId"], name, "Missing movie Effect", effect["assetId"]))

    areas = []
    for value in config["areaManifests"]:
        manifest_path = path(value)
        manifest = pinned(manifest_path)
        rows = manifest.get("areas", [manifest])
        for row in rows:
            for document in row["documents"]:
                source = path(document["source"], manifest_path.parent)
                pin(source)
                document["source"] = str(source)
            areas.append(row)
        roots.update(str(path(p, manifest_path.parent)) for p in manifest.get("resourceRoots", []))
    area_ids = unique([a["catalogEntry"] for a in areas], "id")
    for scene in cinema["scenes"]:
        if scene.get("backgroundAreaId") not in area_ids:
            raise ValueError((scene["classId"], "Missing source background Area"))

    # Account for the preserved Guardian and any unrelated latest saved rows.
    sizes = {}
    for relative, candidate, fields in (
        ("Data/Camera/ClassSelection.cinematics.json", cinema, (("scenes", "classId"),)),
        (f"Data/Maps/Authoring/{AREA}/{AREA}.worldsequences.json", world,
         (("objectResources", "objectId"), ("templates", "sequenceId"), ("instances", "instanceId"))),
    ):
        saved = pinned(ROOT / relative)
        for field, key in fields:
            saved[field] = merge_rows(saved[field], candidate[field], key)
        encoded = json.dumps(saved, ensure_ascii=False, separators=(",", ":"), allow_nan=False).encode("utf-8")
        count = json_values(saved)
        if len(encoded) > 16 * 1024 * 1024 or count > 1000000:
            raise ValueError((relative, "Combined document exceeds runtime limit", len(encoded), count))
        sizes[relative] = dict(compactBytes=len(encoded), jsonValues=count)
    for filename, candidate in (("ClassSelection.cinematics.json", cinema), (f"{AREA}.worldsequences.json", world)):
        write(output / filename, candidate)
    manifest = dict(resourceRoots=sorted(roots), effectDocuments=[str(p) for p in effect_paths.values()],
        worldSequencesDocument=str(output / f"{AREA}.worldsequences.json"),
        cameraDocument=str(output / "ClassSelection.cinematics.json"), classIds=classes, areas=areas)
    for name, expected in inputs.items():
        if hashlib.sha256(Path(name).read_bytes()).hexdigest() != expected:
            raise RuntimeError(f"Source changed while assembling: {name}")
    write(output / "manifest.json", manifest)
    result = dict(installed=False, runtimeValidated=False, classes=classes, effects=len(effect_paths),
        objectResources=len(objects), instances=len(instances), inputHashes=inputs,
        combinedDocumentSizes=sizes, unconsumedSourceFeatures=boundaries)
    write(output / "assembly-receipt.json", result)
    return {k: result[k] for k in ("classes", "effects", "objectResources", "instances", "combinedDocumentSizes")}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--config", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    print(json.dumps(assemble(args.config, args.output), indent=2))

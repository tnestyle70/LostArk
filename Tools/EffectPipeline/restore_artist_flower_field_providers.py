#!/usr/bin/env python3
"""Append the recovered Field01 emitter dependency closure without retuning saved rows."""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import tempfile

from materialize_artist_31470_portable_particle_carriers import (
    SOURCE_ONLY_DISTRIBUTION_FIELDS, SOURCE_ONLY_RECIPE_FIELDS,
)

ROOT = Path(__file__).resolve().parents[2]


def read(path: Path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-dir", type=Path, default=ROOT / "out/ArtistCoreRestore20260909")
    parser.add_argument("--evidence-dir", type=Path,
                        default=ROOT / "out/ArtistWarlordVisualFollowup20260910/Field01")
    args = parser.parse_args()
    source, evidence = args.source_dir.resolve(), args.evidence_dir.resolve()
    path = ROOT / "Data/Effects/Authored/effect.artist.skill.31930.clip2.full.restore.effect.json"
    original_bytes = path.read_bytes()
    original = read(path)
    source_rows = read(evidence / "source-emitter-provider.json")["rows"]
    assert len(source_rows) == 32
    assert [row["order"] for row in source_rows] == list(range(32))
    candidate = read(source / "candidate" / path.name)
    candidates = {e["id"]: e for e in candidate["elements"]}
    projections = {e["target"]: e for e in read(source / "source_projection.json")["projected"]}
    materials = {key: p["material"] for p in read(source / "native_material_patch.json")["programs"]
                 for key in p["occurrences"]}
    geometry = {r["source"]: r for r in read(source / "mesh_cook_receipt.json")
                if r["status"] == "COOKED_NATIVE_GEOMETRY"}
    providers = {row["properties"]["emittername"]["value"]: row["elementId"]
                 for row in source_rows
                 if row["properties"].get("emitterrendermode", {}).get("value") == "erm_none"}
    assert set(providers) == {"1", "2"}
    additions = []
    for row in source_rows:
        element = copy.deepcopy(candidates[row["elementId"]])
        recipe = element["sourceRecipe"]
        assert recipe == row["sourceRecipe"]
        recipe["particleSystemOccurrenceId"] = projections[element["id"]]["cueId"]
        recipe["emitterName"] = row["properties"].get("emittername", {}).get("value", "")
        element.pop("renderer", None)
        element["sourcePresentation"] = {"enabled": False}
        element["resources"] = []
        for field in SOURCE_ONLY_RECIPE_FIELDS:
            recipe.pop(field, None)
        for module in recipe["modules"]:
            for dist in module["distributions"]:
                assert dist.get("parameterBinding", "none") == "none" and not dist.get("parameterName")
                for field in SOURCE_ONLY_DISTRIBUTION_FIELDS:
                    dist.pop(field, None)
        if element["id"] in providers.values():
            recipe["simulationOnly"] = True
            element["displayName"] = f"Field01 provider {recipe['emitterName']} (ERM_None; no draw)"
            element["material"] = {"templateId": "effect.standard",
                                   "sourceMaterialPath": "enginematerials.defaultparticle",
                                   "renderProfile": "alpha_two_sided_depth_read",
                                   "sourceProfile": {"enabled": False}}
            required = next(m for m in recipe["modules"] if m["className"] == "particlemodulerequired")
            required["literals"].append({"propertyPath": "source.emitterrendermode",
                                         "kind": "string", "value": "erm_none"})
        else:
            element["material"] = copy.deepcopy(materials["skill.31930:" + element["id"]])
            location = next(m for m in recipe["modules"] if m["className"] == "efparticlemodulelocationemitter")
            name = next(l["value"] for l in location["literals"] if l["propertyPath"] == "emittername")
            location["literals"].append({"propertyPath": "runtime.providerelementid",
                                         "kind": "string", "value": providers[name]})
            if recipe["rendererShape"] == "mesh":
                td = next(m for m in recipe["modules"] if m["className"] == "particlemoduletypedatamesh")
                mesh = next(l["value"] for l in td["literals"] if l["propertyPath"] == "mesh.objectpath")
                entry = geometry[mesh]
                assert (ROOT / "Client/Bin/Resources" / entry["assetId"]).is_file()
                element["resources"] = [{"slotId": "meshModel", "assetId": entry["assetId"]}]
                element["detail"]["mesh"].update(useModelMaterial=False, modelPreScale=entry["modelPreScale"])
        additions.append(element)
    added_ids = {e["id"] for e in additions}
    existing_ids = {e["id"] for e in original["elements"]}
    if added_ids & existing_ids:
        raise ValueError("Field rows already exist; do not overwrite user-saved provider or petal tuning")
    updated = copy.deepcopy(original)
    updated["elements"].extend(additions)
    assert updated["elements"][:-32] == original["elements"]
    assert {k: v for k, v in updated.items() if k != "elements"} == {
        k: v for k, v in original.items() if k != "elements"}
    backup = evidence / "before" / path.relative_to(ROOT)
    backup.parent.mkdir(parents=True, exist_ok=True)
    if backup.exists() and backup.read_bytes() != original_bytes:
        raise ValueError("Existing Field baseline differs; refusing to replace it")
    backup.write_bytes(original_bytes)
    output = json.dumps(updated, ensure_ascii=False, indent=2).encode("utf-8")
    descriptor, temporary = tempfile.mkstemp(prefix=path.name + ".", suffix=".tmp", dir=path.parent)
    try:
        with os.fdopen(descriptor, "wb") as stream:
            stream.write(output)
        if path.read_bytes() != original_bytes:
            raise ValueError("Saved document changed during materialization; refusing overwrite")
        os.replace(temporary, path)
    finally:
        if os.path.exists(temporary):
            os.unlink(temporary)
    receipt = {"document": path.relative_to(ROOT).as_posix(),
               "beforeSha256": hashlib.sha256(original_bytes).hexdigest(),
               "afterSha256": hashlib.sha256(output).hexdigest(),
               "preservedElements": len(original["elements"]), "addedElements": len(additions),
               "preservedTopLevelValues": True, "simulationProviders": providers,
               "visibleDependentCount": len(additions) - len(providers),
               "sourceOrder": [e["id"] for e in additions]}
    (evidence / "materialize-result.json").write_text(json.dumps(receipt, indent=2), encoding="utf-8")
    print(json.dumps(receipt))


if __name__ == "__main__":
    main()

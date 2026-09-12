"""Bind an explicitly identified original model's catalog materials to a World Object.

This tool changes only materialSourceModelAssetId in authoring JSON. The existing
area publisher remains the only writer of runtime WorldSequence documents.
"""
from __future__ import annotations

import argparse
from collections import Counter
import copy
import json
import os
from pathlib import Path, PurePosixPath
import shlex
import tempfile

from patch_wmodel_dye import Container, SECTION_MATERIAL, cstr, parse_material_section


def resource_path(root: Path, asset_id: str) -> Path:
    parts = PurePosixPath(asset_id)
    if (not asset_id or "\\" in asset_id or ":" in asset_id or parts.is_absolute()
            or any(p in ("", ".", "..") for p in asset_id.split("/"))
            or any(ord(c) < 32 or ord(c) == 127 for c in asset_id)):
        raise ValueError("Expected a Resources-relative model or texture ID: " + asset_id)
    base = (root / "Client/Bin/Resources").resolve()
    result = (base / parts).resolve()
    if not result.is_relative_to(base) or not result.is_file():
        raise ValueError("Missing Resources input: " + asset_id)
    return result


def catalog_materials(root: Path, model_asset_id: str) -> list[dict]:
    """Use the same exact character-owner-first lookup as CActorCatalog."""
    characters = json.loads((root / "Data/Actors/CharacterCatalog.json").read_text(encoding="utf-8-sig"))
    owners = [c for c in characters["characters"] if model_asset_id in
              [c["bodyModel"], *c.get("equipmentModels", []), *c.get("weaponModels", [])]]
    if len(owners) > 1 or any(c["runtimeStatus"] != "supported" for c in owners):
        raise ValueError("Original model ownership is ambiguous or unsupported: " + model_asset_id)
    if owners:
        rows = owners[0].get("modelMaterialOverrides", [])
    else:
        rows = json.loads((root / "Data/Actors/BossCatalog.json").read_text(encoding="utf-8-sig")).get("modelMaterialOverrides", [])
    return [r for r in rows if r["modelAssetId"] == model_asset_id]


def material_names(path: Path) -> Counter:
    model = Container(path)
    names = Counter()
    for desc, section in zip(model.descs, model.sections):
        if desc[0] == SECTION_MATERIAL:
            _, _, entries = parse_material_section(section)
            names.update(cstr(row[2]) for row in entries if cstr(row[2]))
    if not names:
        raise ValueError("Model has no named material slots: " + str(path))
    return names


def assign_material_source(resource: dict, source_model: str, root: Path) -> dict:
    """Return a validated resource copy; never modify the caller on failure."""
    if resource.get("sequenceInstanceId"):
        raise ValueError("A placed sequence alias cannot own model materials")
    target = resource_path(root, resource["modelAssetId"])
    source = resource_path(root, source_model)
    rows = catalog_materials(root, source_model)
    if not rows:
        raise ValueError("Original model has no catalog material overrides: " + source_model)
    source_slots, target_slots = material_names(source), material_names(target)
    seen = set()
    for row in rows:
        name = row["materialName"]
        if name in seen or source_slots[name] != 1 or target_slots[name] != 1:
            raise ValueError("Derived model must retain one original material slot: " + name)
        seen.add(name)
        # Preserve the entire native family/parameter/texture contract in its
        # catalog. Presence checks do not turn unknown families into defaults.
        if not row.get("family") or not row.get("sourceMaterial"):
            raise ValueError("Original material identity is incomplete: " + name)
        for texture in row.get("textures", []):
            resource_path(root, texture["assetId"])
    staged = copy.deepcopy(resource)
    staged["materialSourceModelAssetId"] = source_model
    return staged


def inherit_material_source(resource: dict, original_model: str, root: Path) -> dict:
    """Default for a derived model: carry its exact known owner, without guessing."""
    if resource.get("materialSourceModelAssetId"):
        return assign_material_source(resource, resource["materialSourceModelAssetId"], root)
    if catalog_materials(root, original_model):
        return assign_material_source(resource, original_model, root)
    return copy.deepcopy(resource)


def assign_map_material_source(resource: dict, source_asset: str, area_id: str, root: Path) -> dict:
    """Carry an exact map asset's admitted slots, excluding placement lighting."""
    if resource.get("sequenceInstanceId"):
        raise ValueError("A placed sequence alias cannot own model materials")
    slots = material_names(resource_path(root, resource["modelAssetId"]))
    catalog = root / "Data/Maps/Imported" / area_id / (area_id + ".mapassets")
    assets = [shlex.split(line) for line in catalog.read_text(encoding="utf-8-sig").splitlines()[1:] if line.strip()]
    matches = [row for row in assets if row[0] == source_asset]
    if len(matches) != 1 or matches[0][2] != resource["modelAssetId"]:
        raise ValueError("Map material source must identify this exact model in the imported asset catalog")
    path = root / "Data/Maps/Authoring" / area_id / (area_id + ".mapmaterials.json")
    rows = [r for r in json.loads(path.read_text(encoding="utf-8-sig"))["materials"] if r["assetId"] == source_asset]
    if not rows or len(rows) != len(slots):
        raise ValueError("Map source must cover every named target material slot")
    bindings = []
    for row in rows:
        name = row["materialName"]
        if slots[name] != 1 or row["family"] != "bg-source-opaque-masked":
            raise ValueError("World Object map binding has no admitted family/slot: " + name)
        bindings.append(dict(materialName=name, sourceAssetId=source_asset, sourceMaterialName=name))
    staged = copy.deepcopy(resource)
    if staged.get("mapMaterialBindings") and staged["mapMaterialBindings"] != bindings:
        raise ValueError("Existing explicit map bindings differ; preserve authored overrides")
    staged["mapMaterialBindings"] = bindings
    return staged


def apply_bindings(document: dict, bindings: dict[str, str], root: Path) -> dict:
    staged = copy.deepcopy(document)
    for object_id, source_model in bindings.items():
        matches = [r for r in staged["objectResources"] if r["objectId"] == object_id]
        if len(matches) != 1:
            raise ValueError("Expected one World Object: " + object_id)
        candidate = assign_material_source(matches[0], source_model, root)
        matches[0].clear()
        matches[0].update(candidate)
    if staged != document:
        staged["revision"] += 1
    return staged


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--project-root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--document", required=True, type=Path)
    parser.add_argument("--object-id", action="append", required=True)
    parser.add_argument("--source-model", required=True)
    parser.add_argument("--output", required=True, type=Path, help="Reviewable candidate JSON; never a runtime output")
    parser.add_argument("--apply", action="store_true", help="Replace authoring only after validation and baseline comparison")
    args = parser.parse_args()
    root = args.project_root.resolve()
    authoring = (root / "Data/Maps/Authoring").resolve()
    document_path = args.document.resolve()
    output_path = args.output.resolve()
    if not document_path.is_relative_to(authoring) or output_path == document_path:
        parser.error("Document must be authoring; output must be a separate candidate")
    if output_path.is_relative_to((root / "Client/Bin/DataFiles").resolve()):
        parser.error("Only the area publisher may write runtime data")
    before = document_path.read_bytes()
    document = json.loads(before.decode("utf-8-sig"))
    candidate = apply_bindings(document, dict.fromkeys(args.object_id, args.source_model), root)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(candidate, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    if args.apply and candidate != document:
        # Preserve the existing document's whitespace and every unrelated row.
        # Semantic validation above owns the edit; these replacements target its
        # complete resource records and are checked before replacing authoring.
        newline = "\r\n" if b"\r\n" in before else "\n"
        text = before.decode("utf-8-sig").replace("\r\n", "\n")
        for object_id in args.object_id:
            old = next(r for r in document["objectResources"] if r["objectId"] == object_id)
            new = next(r for r in candidate["objectResources"] if r["objectId"] == object_id)
            old_text = json.dumps(old, ensure_ascii=False, indent=2)
            new_text = json.dumps(new, ensure_ascii=False, indent=2)
            old_text = "\n".join("    " + line for line in old_text.splitlines())
            new_text = "\n".join("    " + line for line in new_text.splitlines())
            if text.count(old_text) != 1:
                raise ValueError("Authoring resource formatting differs; candidate preserved for review")
            text = text.replace(old_text, new_text, 1)
        revision = '"revision": ' + str(document["revision"])
        if text.count(revision + ",") != 1:
            raise ValueError("Authoring revision is not unique")
        text = text.replace(revision + ",", '"revision": ' + str(candidate["revision"]) + ",", 1)
        if json.loads(text) != candidate:
            raise ValueError("Staged source differs from validated candidate")
        payload = (b"\xef\xbb\xbf" if before.startswith(b"\xef\xbb\xbf") else b"") + text.replace("\n", newline).encode("utf-8")
        handle, temporary = tempfile.mkstemp(dir=document_path.parent, suffix=".material-staged")
        try:
            with os.fdopen(handle, "wb") as stream:
                stream.write(payload)
            if document_path.read_bytes() != before:
                raise ValueError("Authoring changed concurrently; existing file preserved")
            os.replace(temporary, document_path)
        finally:
            if os.path.exists(temporary):
                os.unlink(temporary)
    print(json.dumps({"candidate": str(output_path), "applied": args.apply and candidate != document,
                      "objectIds": args.object_id, "materialSourceModelAssetId": args.source_model}))


if __name__ == "__main__":
    main()

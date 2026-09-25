"""Install the reviewed Guardian cinematic through current authoring contracts.

Catalogs are merged by stable identity from the latest saved document. Every
replacement checks its original hash, uses an atomic rename, and records a
rollback copy. Published map data remains owned by Publish-MapAuthoring.ps1.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import sys
from contextlib import contextmanager

ROOT = Path(__file__).resolve().parents[2]
AREA = "LV_LOBBY_CLASSSELECT_SL00"
RESOURCE_FOLDERS = ("Fonts", "Character", "Deploy", "Effect", "Map", "Sound", "UI")


def asset_path(value):
    path = Path(value)
    if (path.is_absolute() or path.drive or "\\" in value or ".." in path.parts or
            not path.parts or path.parts[0] not in RESOURCE_FOLDERS):
        raise ValueError(f"Not a Resources-relative asset ID: {value}")
    return path


def validate_texture_container(source, asset):
    # D3D's DDS loader dispatches by extension. A renamed UModel TGA must not
    # pass installation merely because the reference and file hash agree.
    if Path(asset).suffix.lower() == ".dds":
        with Path(source).open("rb") as stream:
            if stream.read(4) != b"DDS ":
                raise ValueError(f"DDS asset contains a different image container: {asset}")


@contextmanager
def install_lock():
    # Concurrent cinematic installers share one admission gate. External editor
    # saves are additionally guarded by per-target hashes at atomic promotion.
    path = ROOT / "out/ClassSelection.install.lock"
    path.parent.mkdir(parents=True, exist_ok=True)
    descriptor = os.open(path, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(descriptor, str(os.getpid()).encode("ascii"))
        yield
    finally:
        os.close(descriptor)
        path.unlink()


def internal_textures(model, model_asset):
    sys.path.insert(0, str(ROOT / "Tools/ModelAssetConverter"))
    import patch_wmodel_dye as material
    container = material.Container(model)
    for index, descriptor in enumerate(container.descs):
        if descriptor[0] != material.SECTION_MATERIAL:
            continue
        _, version, entries = material.parse_material_section(container.sections[index])
        for entry in entries:
            for raw in entry[3:13 if version == b"WMA3" else 12]:
                value = material.wstr(raw).replace("\\", "/")
                if not value:
                    continue
                path = Path(value)
                if path.is_absolute() or path.drive:
                    raise ValueError((model_asset, "Absolute internal texture", value))
                if path.parts[0] not in RESOURCE_FOLDERS:
                    value = os.path.normpath(str(Path(model_asset).parent / path)).replace("\\", "/")
                asset_path(value)
                yield value


def sha(path):
    if not path.exists():
        return None
    value = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            value.update(block)
    return value.hexdigest()


def read(path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def write(path, document):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(document, ensure_ascii=False, indent=2, allow_nan=False) + "\n", encoding="utf-8")


def merge_rows(current, replacement, key):
    additions = {row[key]: row for row in replacement}
    if len(additions) != len(replacement) or len({row[key] for row in current}) != len(current):
        raise ValueError(f"Duplicate stable {key}")
    result = [additions.pop(row[key], row) for row in current]
    return result + list(additions.values())


def resource_ids(value):
    if isinstance(value, dict):
        for child in value.values():
            yield from resource_ids(child)
    elif isinstance(value, list):
        for child in value:
            yield from resource_ids(child)
    elif isinstance(value, str) and value.startswith(tuple(folder + "/" for folder in RESOURCE_FOLDERS)):
        if Path(value).suffix.lower() in (".wmodel", ".dds", ".png", ".tga", ".wav", ".ogg"):
            asset_path(value)
            yield value


def install(output, mirror, commit, manifest_path=None):
    with install_lock():
        return install_locked(output, mirror, commit, manifest_path)


def install_locked(output, mirror, commit, manifest_path=None):
    output = output.resolve()
    staged = output / "Installation/Staged"
    receipt_path = output / "Installation/receipt.json"
    candidate = output / "Candidate"
    resource_root = ROOT / "Client/Bin/Resources"
    planned = []
    manifest = read(manifest_path) if manifest_path else None
    manifest_root = Path(manifest_path).resolve().parent if manifest_path else output

    def candidate_path(value):
        path = Path(value)
        return path if path.is_absolute() else manifest_root / path

    def plan(source, target, expected_before=None):
        source, target = source.resolve(), target.resolve()
        before = sha(target)
        if expected_before is not None and before != expected_before:
            raise RuntimeError(f"Document changed while preparing: {target}")
        after = sha(source)
        if after is None:
            raise FileNotFoundError(source)
        planned.append(dict(source=str(source), target=str(target), before=before,
                            after=after, bytes=source.stat().st_size,
                            status="unchanged" if before == after else "planned"))

    if manifest:
        donors = {}
        for root in map(candidate_path, manifest["resourceRoots"]):
            for path in root.rglob("*"):
                if not path.is_file():
                    continue
                asset = path.relative_to(root).as_posix()
                asset_path(asset)
                if path.suffix.lower() not in (".wmodel", ".dds", ".png", ".tga", ".wav", ".ogg"):
                    raise ValueError(f"Unexpected candidate resource type: {path}")
                if asset in donors and sha(donors[asset]) != sha(path):
                    raise ValueError(f"Conflicting candidate resource: {asset}")
                donors[asset] = path
        effects = [read(candidate_path(p)) for p in manifest.get("effectDocuments", [])]
        sequences = read(candidate_path(manifest["worldSequencesDocument"]))
        cinema = read(candidate_path(manifest["cameraDocument"]))
        if {s["classId"] for s in cinema["scenes"]} != set(manifest["classIds"]):
            raise ValueError("Reviewed class list differs from candidate scenes")
    else:
        donor_root = output / "Baked/Resources"
        donors = {p.relative_to(donor_root).as_posix(): p for p in donor_root.rglob("*.wmodel")}
        if len(donors) != 4:
            raise ValueError(f"Expected four baked donors, found {len(donors)}")
        effects = [read(p) for p in sorted((output / "Effects/candidate").glob("*.effect.json"))]
        if len(effects) != 4 or sum(len(d["elements"]) for d in effects) != 25:
            raise ValueError("Expected four admitted effects / 25 emitters")
        sequences = read(candidate / f"{AREA}.worldsequences.json")
        cinema = read(candidate / "ClassSelection.cinematics.json")
    referenced = set(resource_ids(effects)) | set(resource_ids(sequences)) | set(resource_ids(cinema))
    for area in manifest.get("areas", []) if manifest else []:
        for entry in area["documents"]:
            source = candidate_path(entry["source"])
            if source.suffix.lower() == ".json":
                referenced.update(resource_ids(read(source)))
    for asset in sorted(referenced | set(donors)):
        if Path(asset).suffix.lower() == ".wmodel":
            referenced.update(internal_textures(donors.get(asset, resource_root / asset), asset))
    for asset in sorted(referenced | set(donors)):
        source = donors.get(asset, resource_root / asset)
        if not source.is_file():
            raise FileNotFoundError(f"Cinematic resource missing: {asset}")
        validate_texture_container(source, asset)
        for root in (resource_root, mirror):
            target = (root / asset).resolve()
            if not target.is_relative_to(root.resolve()):
                raise ValueError(f"Invalid Resources-relative asset ID: {asset}")
            # Existing source assets belong to other owners. Never replace them.
            if target.exists() and sha(target) != sha(source):
                raise RuntimeError(f"Existing resource differs; explicit reconciliation required: {target}")
            plan(source, target)

    def document(relative, transform):
        target = ROOT / relative
        baseline = sha(target)
        previous = read(target) if baseline is not None else None
        updated = transform(previous)
        stage = staged / relative
        write(stage, updated)
        if relative.endswith((".worldsequences.json", ".cinematics.json")):
            maximum = 16 * 1024 * 1024
            if stage.stat().st_size > maximum:
                # Several source movies share one document. Whitespace must not
                # consume the product reader's bounded document capacity.
                stage.write_text(json.dumps(updated, ensure_ascii=False,
                    separators=(",", ":"), allow_nan=False) + "\n", encoding="utf-8")
            if stage.stat().st_size > maximum:
                raise ValueError(f"Combined cinematic document exceeds runtime size: {relative}")
        if sha(target) != baseline:
            raise RuntimeError(f"Concurrent saved-document change: {target}")
        plan(stage, target)
        if planned[-1]["before"] != baseline:
            raise RuntimeError(f"Concurrent saved-document change during planning: {target}")

    for effect in effects:
        document(f"Data/Effects/Authored/{effect['effectAssetId']}.effect.json", lambda old, effect=effect: effect)
    effect_rows = [dict(effectAssetId=d["effectAssetId"], payloadKind="DIRECT_AUTHORED_DOCUMENT",
                        authoringPath=f"Effects/Authored/{d['effectAssetId']}.effect.json") for d in effects]
    def catalog(old):
        old["effects"] = merge_rows(old["effects"], effect_rows, "effectAssetId")
        return old
    document("Data/Effects/EffectCatalog.json", catalog)

    def merged_cinema(old):
        if old is None:
            return cinema
        if any(old[k] != cinema[k] for k in ("schema", "formatVersion", "areaId")):
            raise ValueError("Cinematic header conflict")
        old["scenes"] = merge_rows(old["scenes"], cinema["scenes"], "classId")
        return old
    document("Data/Camera/ClassSelection.cinematics.json", merged_cinema)

    sequence_relative = f"Data/Maps/Authoring/{AREA}/{AREA}.worldsequences.json"
    def merged_sequences(old):
        if old is None:
            return sequences
        if any(old[k] != sequences[k] for k in ("schema", "formatVersion", "areaId")):
            raise ValueError("WorldSequence header conflict")
        for field, key in (("objectResources", "objectId"), ("templates", "sequenceId"), ("instances", "instanceId")):
            old[field] = merge_rows(old[field], sequences[field], key)
        old["revision"] = max(old["revision"], sequences["revision"])
        return old
    document(sequence_relative, merged_sequences)

    areas = manifest.get("areas", []) if manifest else []
    for area in areas:
        for entry in area["documents"]:
            relative = Path(entry["target"])
            source = candidate_path(entry["source"])
            if relative.is_absolute() or ".." in relative.parts or relative.parts[:2] != ("Data", "Maps"):
                raise ValueError(f"Area document outside Data/Maps: {relative}")
            target = ROOT / relative
            # These are newly restored source Areas. An independently authored
            # Area with different bytes requires a stable-record reconciliation.
            if target.exists() and sha(target) != sha(source):
                raise ValueError(f"Existing Area document differs: {target}")
            plan(source, target)

    def maps(old):
        area = next(row for row in old["areas"] if row["id"] == AREA)
        area["sourceSequences"] = sequence_relative
        area["sequences"] = f"Client/Bin/DataFiles/Map/{AREA}.worldsequences.json"
        for addition in areas:
            row = addition["catalogEntry"]
            current = next((a for a in old["areas"] if a["id"] == row["id"]), None)
            if current is not None and current != row:
                raise ValueError(f"Existing Area catalog row differs: {row['id']}")
            if current is None:
                old["areas"].append(row)
        return old
    document("Data/Maps/MapCatalog.json", maps)

    by_target = {}
    for row in planned:
        previous = by_target.get(row["target"])
        if previous and (previous["before"], previous["after"]) != (row["before"], row["after"]):
            raise ValueError(f"Multiple writes disagree: {row['target']}")
        by_target[row["target"]] = row
    planned = list(by_target.values())

    write(output / "Installation/plan.json", planned)
    if not commit:
        return dict(planned=len(planned), changed=sum(r["status"] == "planned" for r in planned))
    installed = []
    try:
        for index, row in enumerate(planned):
            if row["status"] == "unchanged":
                continue
            source, target = Path(row["source"]), Path(row["target"])
            if sha(source) != row["after"] or sha(target) != row["before"]:
                raise RuntimeError(f"Freshness check failed: {target}")
            if row["before"] is not None:
                backup = output / "Installation/Backup" / f"{index:04d}-{target.name}"
                backup.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(target, backup)
                row["backup"] = str(backup)
            target.parent.mkdir(parents=True, exist_ok=True)
            temporary = target.with_name(target.name + ".guardian-selection-stage")
            if temporary.exists():
                raise RuntimeError(f"Unexpected staging file: {temporary}")
            shutil.copyfile(source, temporary)
            if sha(temporary) != row["after"] or sha(target) != row["before"]:
                temporary.unlink()
                raise RuntimeError(f"Freshness check failed before promotion: {target}")
            os.replace(temporary, target)
            row["status"] = "installed"
            installed.append(row)
        if any(sha(Path(r["target"])) != r["after"] for r in planned):
            raise RuntimeError("Post-install hash verification failed")
    except Exception:
        for row in reversed(installed):
            target = Path(row["target"])
            if sha(target) != row["after"]:
                row["status"] = "rollback-conflict"
                continue
            if row["before"] is None:
                target.unlink()
            else:
                temporary = target.with_name(target.name + ".guardian-selection-rollback")
                shutil.copyfile(row["backup"], temporary)
                os.replace(temporary, target)
            row["status"] = "rolled-back"
        write(receipt_path, planned)
        raise
    write(receipt_path, planned)
    return dict(installed=len(installed), unchanged=len(planned)-len(installed), receipt=str(receipt_path))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--mirror", type=Path, default=ROOT.parent / "GBResources")
    parser.add_argument("--commit", action="store_true")
    parser.add_argument("--manifest", type=Path, help="Reviewed multi-class candidates and Resource roots")
    args = parser.parse_args()
    print(json.dumps(install(args.output, args.mirror.resolve(), args.commit, args.manifest), indent=2))

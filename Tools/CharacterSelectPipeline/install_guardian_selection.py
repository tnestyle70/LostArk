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

ROOT = Path(__file__).resolve().parents[2]
AREA = "LV_LOBBY_CLASSSELECT_SL00"


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
    elif isinstance(value, str) and value.startswith(("Character/", "Effect/", "Map/")):
        if Path(value).suffix.lower() in (".wmodel", ".dds", ".png", ".tga"):
            yield value


def install(output, mirror, commit):
    output = output.resolve()
    staged = output / "Installation/Staged"
    receipt_path = output / "Installation/receipt.json"
    candidate = output / "Candidate"
    resource_root = ROOT / "Client/Bin/Resources"
    planned = []

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
    for asset in sorted(referenced | set(donors)):
        source = donors.get(asset, resource_root / asset)
        if not source.is_file():
            raise FileNotFoundError(f"Cinematic resource missing: {asset}")
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

    def maps(old):
        area = next(row for row in old["areas"] if row["id"] == AREA)
        area["sourceSequences"] = sequence_relative
        area["sequences"] = f"Client/Bin/DataFiles/Map/{AREA}.worldsequences.json"
        return old
    document("Data/Maps/MapCatalog.json", maps)

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
    args = parser.parse_args()
    print(json.dumps(install(args.output, args.mirror.resolve(), args.commit), indent=2))

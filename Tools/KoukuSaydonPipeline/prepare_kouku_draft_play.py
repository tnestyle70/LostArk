"""Project an immutable Debug audition snapshot into request-local artifacts."""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import sys

if __package__:
    from . import project_kouku_saydon_composition as composition
else:
    import project_kouku_saydon_composition as composition


MAX_SOURCE_BYTES = 64 * 1024 * 1024


def output_directory(root: Path, path: Path) -> Path:
    resolved = path.resolve()
    if not resolved.is_relative_to((root / "out").resolve()) or resolved == (root / "out").resolve():
        raise composition.CompositionError("Draft artifacts require a request directory under repository/out.")
    return resolved


def prepare(source: dict, root: Path, *, pattern_id: str = "", bundle_id: str = "") -> tuple[dict, dict[Path, bytes]]:
    if bool(pattern_id) == bool(bundle_id):
        raise composition.CompositionError("Select exactly one Pattern or Bundle for draft playback.")
    with composition._publication_session():
        # Reuse the publisher's global identity checks and exact dependency closure.
        inventory = composition._saved_pattern_inventory(source, root)
        patterns = {row["patternId"]: row for row in source["patterns"]}
        bundle = next((row for row in source.get("bundles", []) if row["bundleId"] == bundle_id), None)
        if bundle_id and bundle is None:
            raise composition.CompositionError("Draft Bundle is missing: " + bundle_id)
        pending = [pattern_id] if pattern_id else [row["patternId"] for row in bundle["members"]]
        if not pending:
            raise composition.CompositionError("Draft playback requires a nonempty execution target.")
        closure: set[str] = set()
        while pending:
            identity = pending.pop()
            if identity in closure:
                continue
            if identity not in patterns:
                raise composition.CompositionError("Required draft Pattern is missing: " + identity)
            closure.add(identity)
            pending.extend(composition._pattern_dependencies(source, patterns[identity]) - closure)
        candidate = composition._publication_candidate(source, closure, {bundle_id} if bundle_id else set())
        admitted, selected_inventory = composition.prepare_publication(candidate, root)
        key, identity = ("bundleId", bundle_id) if bundle_id else ("patternId", pattern_id)
        family = "bundles" if bundle_id else "patterns"
        selected = next((row for row in selected_inventory[family] if row[key] == identity), None)
        if selected is None or selected["unavailableReason"]:
            reason = selected["unavailableReason"] if selected else "target was not admitted"
            raise composition.CompositionError("Draft target is unavailable: " + identity + ": " + reason)
        projected = composition.projected_outputs(admitted, root, selected_inventory)
        encounter = json.loads(projected[composition.ENCOUNTER_PATH])
        presentation = json.loads(projected[composition.PRESENTATION_PATH])
        target = bundle if bundle_id else patterns[pattern_id]
        starts = [patterns[row["patternId"]] for row in bundle["members"]] if bundle_id else [target]
        metadata = {
            "schema": "lostark.kouku-draft-admission", "formatVersion": 1,
            "sourceRevision": admitted["revision"], "targetKind": "BUNDLE" if bundle_id else "PATTERN",
            "targetId": identity, "gateId": target["gateId"],
            "targetPlacementIds": list(dict.fromkeys(row["targetBossPlacementId"] for row in starts)),
            "patternIds": [row["patternId"] for row in encounter["patterns"]],
            "bundleIds": [row["bundleId"] for row in encounter["bundles"]],
            "patternInventory": selected_inventory,
            "savedPatternCount": len(inventory["patterns"]),
        }
        return metadata, {
            Path("encounter.json"): composition.serialize_json(encounter),
            Path("presentation.json"): composition.serialize_json(presentation),
        }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repository-root", type=Path, default=composition.REPOSITORY_ROOT)
    parser.add_argument("--source-path", type=Path, required=True)
    parser.add_argument("--output-directory", type=Path, required=True)
    target = parser.add_mutually_exclusive_group(required=True)
    target.add_argument("--pattern-id", default="")
    target.add_argument("--bundle-id", default="")
    args = parser.parse_args()
    try:
        root = args.repository_root.resolve()
        output = output_directory(root, args.output_directory)
        # The resolved output boundary is checked before removing old request metadata.
        (output / "admission.json").unlink(missing_ok=True)
        source_path = args.source_path.resolve()
        if not source_path.is_relative_to((root / "out").resolve()):
            raise composition.CompositionError("Draft input must be an explicit snapshot under repository/out.")
        if not 0 < source_path.stat().st_size <= MAX_SOURCE_BYTES:
            raise composition.CompositionError("Draft snapshot must contain 1..64 MiB of UTF-8 JSON.")
        source_bytes = source_path.read_bytes()
        source = json.loads(source_bytes.decode("utf-8-sig"))
        metadata, artifacts = prepare(source, root, pattern_id=args.pattern_id, bundle_id=args.bundle_id)
        if source_path.read_bytes() != source_bytes:
            raise composition.CompositionError("Draft snapshot changed during preparation; request a fresh snapshot.")
        metadata["sourceSha256"] = hashlib.sha256(source_bytes).hexdigest()
        artifacts[Path("projection.json")] = composition.serialize_json(metadata)
        output.mkdir(parents=True, exist_ok=True)
        for name, content in artifacts.items():
            destination = output / name
            if destination == source_path:
                raise composition.CompositionError("Draft snapshot and output paths must be distinct.")
            temporary = destination.with_suffix(destination.suffix + ".tmp")
            temporary.write_bytes(content)
            os.replace(temporary, destination)
        print(json.dumps({"targetId": metadata["targetId"], "sourceRevision": metadata["sourceRevision"]}))
        return 0
    except (composition.CompositionError, OSError, ValueError, KeyError, TypeError) as error:
        print("Draft playback preparation failed: " + str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())

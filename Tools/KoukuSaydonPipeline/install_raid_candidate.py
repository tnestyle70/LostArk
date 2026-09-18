"""Review or install one prepared Kouku raid candidate; dry-run is the default.

--candidate-root names the directory containing manifest.json, candidate/ and
baseline-current/. No projection, merge, publish, Reload or process control runs here.
"""
from __future__ import annotations

import argparse
import ctypes
import hashlib
import json
import os
import re
import shutil
import sys
import uuid
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
from typing import Callable

DATA_PATHS = frozenset({
    "Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json",
    "Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json",
    "Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json",
    "Data/Sound/CharacterSoundCatalog.json",
})
OPTIONAL_DATA_PATHS = frozenset({
    "Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json",
})
SOUND_PREFIX = "Client/Bin/Resources/Sound/"
IDENTITIES = ("occurrenceId", "resourceId", "patternId", "stageId", "worldId", "logicId",
              "sequenceId", "instanceId", "objectId", "spawnId", "bundleId", "memberId",
              "folderId", "summonId", "placementId", "entryId", "flowId")


class InstallError(RuntimeError):
    pass


def sha(path: Path) -> str | None:
    if not path.exists():
        return None
    if not path.is_file():
        raise InstallError(f"Expected a file: {path}")
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def checked_hash(value: object) -> str:
    if not isinstance(value, str) or not re.fullmatch(r"[0-9a-f]{64}", value):
        raise InstallError("Manifest SHA-256 must contain 64 lowercase hex digits")
    return value


def relative(value: object) -> str:
    if not isinstance(value, str) or not value or "\\" in value or ":" in value:
        raise InstallError(f"Invalid manifest relative path: {value!r}")
    path = PurePosixPath(value)
    if path.is_absolute() or any(part in {"", ".", ".."} for part in value.split("/")):
        raise InstallError(f"Invalid manifest relative path: {value!r}")
    return path.as_posix()


def bounded(root: Path, value: str) -> Path:
    path = root / relative(value)
    resolved = path.resolve()
    if not resolved.is_relative_to(root.resolve()):
        raise InstallError(f"Resolved path escapes its root: {path} -> {resolved}")
    if path.is_symlink():
        raise InstallError(f"A manifest file cannot be a symlink: {path}")
    return path


def read_json(path: Path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def leaf_fields(value, prefix="$") -> dict:
    """Stable IDs avoid confusing an appended row with edits to a different row."""
    if isinstance(value, dict):
        result = {}
        for key, item in value.items():
            result.update(leaf_fields(item, prefix + "/" + key))
        return result or {prefix: {}}
    if isinstance(value, list):
        key = next((key for key in IDENTITIES if value and all(
            isinstance(item, dict) and isinstance(item.get(key), str) for item in value) and
            len({item[key] for item in value}) == len(value)), None)
        result = {prefix + "/@order": [item[key] for item in value]} if key else {}
        for index, item in enumerate(value):
            label = key + "=" + item[key] if key else str(index)
            result.update(leaf_fields(item, prefix + "/" + label))
        return result or {prefix: []}
    return {prefix: value}


def changed_fields(before, after) -> set[str]:
    left, right = leaf_fields(before), leaf_fields(after)
    missing = object()
    return {key for key in left.keys() | right.keys() if left.get(key, missing) != right.get(key, missing)}


def additions(before, after) -> dict:
    result = {}
    for key, identity in (("presentationResources", "resourceId"), ("worlds", "worldId")):
        old = {row.get(identity) for row in before.get(key, [])}
        result[key] = [{identity: row[identity], "displayName": row.get("displayName", "")}
                       for row in after.get(key, []) if row.get(identity) not in old]
    old_patterns = {row["patternId"]: row for row in before.get("patterns", [])}
    cues = []
    for pattern in after.get("patterns", []):
        old = old_patterns.get(pattern["patternId"], {})
        for lane in ("presentationOccurrences", "worldOccurrences", "logicOccurrences", "summonOccurrences"):
            previous = {row.get("occurrenceId") for row in old.get(lane, [])}
            for row in pattern.get(lane, []):
                if row.get("occurrenceId") not in previous:
                    cues.append({"patternId": pattern["patternId"], "lane": lane, **row})
    result["occurrences"] = cues
    old_classes = before.get("classes", {})
    result["soundCatalogEntries"] = [
        {"class": actor, "event": event, "assets": assets}
        for actor, events in after.get("classes", {}).items()
        for event, assets in events.items() if event not in old_classes.get(actor, {})]
    return result


@dataclass
class Entry:
    relative: str
    source: Path
    destination: Path
    before_sha: str | None
    after_sha: str
    kind: str
    baseline: Path | None = None
    stage: Path | None = None
    displaced: Path | None = None
    backup: Path | None = None
    applied: bool = False


@dataclass
class Plan:
    root: Path
    candidate: Path
    manifest_sha: str
    entries: list[Entry]
    report: dict


def inspect(candidate_root: Path, repository_root: Path) -> Plan:
    root, candidate = repository_root.resolve(), candidate_root.resolve()
    manifest_path = bounded(candidate, "manifest.json")
    manifest_hash = sha(manifest_path)
    manifest = read_json(manifest_path)
    if not isinstance(manifest, dict) or not isinstance(manifest.get("files"), list) or not isinstance(manifest.get("media"), list):
        raise InstallError("Manifest requires files and media arrays")
    report = {"mode": "dry-run", "ready": True, "candidateRoot": str(candidate),
              "repositoryRoot": str(root), "files": [], "media": [], "conflicts": []}
    entries, seen = [], set()
    for row in manifest["files"]:
        path = relative(row["path"])
        if path not in DATA_PATHS | OPTIONAL_DATA_PATHS or path in seen:
            raise InstallError(f"Unexpected or duplicate Data path: {path}")
        seen.add(path)
        baseline_hash = checked_hash(row["baselineSha256"])
        source = bounded(candidate, "candidate/" + path)
        baseline = bounded(candidate, "baseline-current/" + path)
        destination = bounded(root, path)
        if sha(baseline) != baseline_hash:
            raise InstallError(f"Prepared baseline hash mismatch: {path}")
        before, after = read_json(baseline), read_json(source)
        if not isinstance(before, dict) or not isinstance(after, dict):
            raise InstallError(f"Data document must be an object: {path}")
        if before.get("revision") != row.get("baselineRevision") or after.get("revision") != row.get("candidateRevision"):
            raise InstallError(f"Prepared revision mismatch: {path}")
        installed, expected = sha(destination), sha(source)
        if expected is None:
            raise InstallError(f"Candidate file is missing: {path}")
        current = read_json(destination) if installed is not None else None
        changed = sorted(changed_fields(before, after))
        if installed not in {baseline_hash, expected}:
            edits = changed_fields(before, current) if isinstance(current, dict) else {"$"}
            overlap = sorted(set(changed) & edits)
            report["conflicts"].append({"path": path, "reason": "saved-file-changed; rebuild against latest disk save",
                "baselineSha256": baseline_hash, "currentSha256": installed,
                "baselineRevision": row.get("baselineRevision"),
                "currentRevision": current.get("revision") if isinstance(current, dict) else None,
                "sameFieldConflicts": overlap, "concurrentFields": sorted(edits)})
        state = "already-installed" if installed == expected else "replace"
        report["files"].append({"path": path, "state": state, "baselineSha256": baseline_hash,
            "candidateSha256": expected, "candidateRevision": after.get("revision"),
            "changedFields": changed, "additions": additions(before, after)})
        entries.append(Entry(path, source, destination, installed, expected, "data", baseline))
    if not DATA_PATHS.issubset(seen):
        raise InstallError("Manifest must contain the four required Data paths; only the named WorldSequence authoring file is optional")
    media = {}
    for row in manifest["media"]:
        path = relative(row["path"])
        if not path.startswith(SOUND_PREFIX) or PurePosixPath(path).suffix.lower() != ".wav":
            raise InstallError(f"Media path must be a Sound WAV: {path}")
        expected = checked_hash(row["sha256"])
        if path in media and media[path] != expected:
            raise InstallError(f"Duplicate media path has different hashes: {path}")
        source = Path(row["source"]).resolve()
        if not (source.is_relative_to(candidate) or source.is_relative_to((root / "out").resolve())):
            raise InstallError(f"Media source must remain in candidate or repository out: {source}")
        if sha(source) != expected:
            raise InstallError(f"Media source hash mismatch: {path}")
        if path in media:
            continue
        media[path] = expected
        destination = bounded(root, path)
        current = sha(destination)
        if current not in {None, expected}:
            report["conflicts"].append({"path": path, "reason": "existing media has different content", "currentSha256": current})
        report["media"].append({"path": path, "sha256": expected, "bytes": source.stat().st_size,
                                "state": "already-installed" if current == expected else "add"})
        entries.append(Entry(path, source, destination, current, expected, "media"))
    if sha(manifest_path) != manifest_hash:
        raise InstallError("Manifest changed during inspection")
    report["ready"] = not report["conflicts"]
    report["mediaCount"] = len(media)
    report["mediaDuplicateCount"] = len(manifest["media"]) - len(media)
    return Plan(root, candidate, manifest_hash, entries, report)


def durable_copy(source: Path, destination: Path) -> None:
    with source.open("rb") as src, destination.open("xb") as dst:
        shutil.copyfileobj(src, dst, 1024 * 1024)
        dst.flush()
        os.fsync(dst.fileno())


def atomic_replace_capture(source: Path, destination: Path, displaced: Path) -> None:
    """Windows atomically captures the file actually replaced, including a late save."""
    if os.name != "nt":
        raise InstallError("Install requires Windows ReplaceFileW; dry-run remains portable")
    replace = ctypes.WinDLL("kernel32", use_last_error=True).ReplaceFileW
    replace.argtypes = [ctypes.c_wchar_p, ctypes.c_wchar_p, ctypes.c_wchar_p,
                       ctypes.c_uint32, ctypes.c_void_p, ctypes.c_void_p]
    replace.restype = ctypes.c_int
    if not replace(str(destination), str(source), str(displaced), 0, None, None):
        raise ctypes.WinError(ctypes.get_last_error())


def write_receipt(path: Path, report: dict) -> None:
    stage = path.with_name(path.name + "." + uuid.uuid4().hex + ".tmp")
    try:
        with stage.open("x", encoding="utf-8", newline="\n") as stream:
            json.dump(report, stream, ensure_ascii=False, indent=2)
            stream.write("\n")
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(stage, path)
    finally:
        if stage.exists():
            stage.unlink()


def install(plan: Plan, before_commit: Callable | None = None, after_commit: Callable | None = None) -> dict:
    """Hooks are for out-only fault-injection tests; the CLI exposes no hook option."""
    if not plan.report["ready"]:
        raise InstallError("Latest saved bytes conflict; inspect the review report and rebuild the candidate")
    if os.name != "nt":
        raise InstallError("Installation requires Windows")
    if sha(bounded(plan.candidate, "manifest.json")) != plan.manifest_sha:
        raise InstallError("Manifest changed after review")
    identity = uuid.uuid4().hex
    transaction = bounded(plan.root, "out/transactions/kouku-raid-" + identity)
    transaction.mkdir(parents=True, exist_ok=False)
    report = {**plan.report, "mode": "install", "transactionId": identity,
              "transactionPath": str(transaction), "status": "preparing", "applied": [], "rollback": []}
    receipt = transaction / "receipt.json"
    # Install media and catalog before documents referring to those resources.
    ordered = sorted(plan.entries, key=lambda e: (e.kind != "media", e.relative not in {"Data/Sound/CharacterSoundCatalog.json"}, e.relative))
    changed = [entry for entry in ordered if entry.before_sha != entry.after_sha]
    owned_temporaries = []
    try:
        for entry in changed:
            destination = bounded(plan.root, entry.relative)
            if sha(destination) != entry.before_sha:
                raise InstallError(f"Saved bytes changed before staging: {entry.relative}")
            if sha(entry.source) != entry.after_sha:
                raise InstallError(f"Candidate source changed after review: {entry.relative}")
            destination.parent.mkdir(parents=True, exist_ok=True)
            bounded(plan.root, entry.relative)
            stem = "." + destination.name + ".raid-" + identity
            entry.stage = destination.with_name(stem + ".stage")
            entry.displaced = destination.with_name(stem + ".displaced")
            owned_temporaries.extend((entry.stage, entry.displaced))
            if entry.before_sha is not None:
                entry.backup = transaction / "backup" / entry.relative
                entry.backup.parent.mkdir(parents=True, exist_ok=True)
                durable_copy(destination, entry.backup)
                if sha(entry.backup) != entry.before_sha:
                    raise InstallError(f"Saved bytes changed while backing up: {entry.relative}")
            durable_copy(entry.source, entry.stage)
            if sha(entry.stage) != entry.after_sha:
                raise InstallError(f"Staged bytes differ from reviewed candidate: {entry.relative}")
        report["status"] = "staged"
        write_receipt(receipt, report)
        for index, entry in enumerate(changed):
            if before_commit:
                before_commit(index, entry)
            destination = bounded(plan.root, entry.relative)
            if sha(bounded(plan.candidate, "manifest.json")) != plan.manifest_sha:
                raise InstallError("Manifest changed before replace")
            if sha(entry.source) != entry.after_sha or sha(entry.stage) != entry.after_sha:
                raise InstallError(f"Candidate changed before replace: {entry.relative}")
            # Last disk read immediately precedes the filesystem commit.
            if sha(destination) != entry.before_sha:
                raise InstallError(f"Concurrent save preserved before replace: {entry.relative}")
            if entry.before_sha is None:
                # Atomic create-if-absent: never replace a concurrently created asset.
                os.link(entry.stage, destination)
                entry.applied = True
                entry.stage.unlink()
            else:
                atomic_replace_capture(entry.stage, destination, entry.displaced)
                entry.applied = True
                if sha(entry.displaced) != entry.before_sha:
                    raise InstallError(f"Concurrent save captured during replace: {entry.relative}")
            report["applied"].append(entry.relative)
            write_receipt(receipt, report)
            if after_commit:
                after_commit(index, entry)
        report["status"] = "installed"
    except BaseException as error:
        report["status"] = "failed"
        report["error"] = str(error)
        for entry in reversed(changed):
            # Handle an interrupted ReplaceFileW return without assuming it made no change.
            applied = entry.applied or (entry.displaced and entry.displaced.exists() and sha(entry.destination) == entry.after_sha)
            if not applied:
                continue
            outcome = {"path": entry.relative}
            try:
                destination = bounded(plan.root, entry.relative)
                if sha(destination) != entry.after_sha:
                    outcome["status"] = "preserved-concurrent-edit"
                elif entry.before_sha is None:
                    captured = destination.with_name("." + destination.name + ".raid-" + identity + ".new-capture")
                    owned_temporaries.append(captured)
                    os.rename(destination, captured)
                    if sha(captured) == entry.after_sha:
                        outcome["status"] = "removed-owned-new-file"
                    else:
                        # A save raced with the check. Recreate it only if the name is still absent.
                        try:
                            os.link(captured, destination)
                        except FileExistsError:
                            pass
                        outcome["status"] = "preserved-concurrent-edit"
                else:
                    restore = entry.displaced if entry.displaced.exists() else entry.backup
                    expected_current = entry.after_sha
                    for attempt in range(4):
                        if sha(destination) != expected_current:
                            outcome["status"] = "preserved-concurrent-edit"
                            break
                        restore_stage = destination.with_name("." + destination.name + ".raid-" + identity + f".restore{attempt}")
                        captured = destination.with_name("." + destination.name + ".raid-" + identity + f".rollback-capture{attempt}")
                        owned_temporaries.extend((restore_stage, captured))
                        durable_copy(restore, restore_stage)
                        restoring_hash = sha(restore_stage)
                        if sha(destination) != expected_current:
                            outcome["status"] = "preserved-concurrent-edit"
                            break
                        atomic_replace_capture(restore_stage, destination, captured)
                        if sha(captured) == expected_current:
                            outcome["status"] = "restored"
                            break
                        # A save raced with rollback; its captured bytes take precedence.
                        restore, expected_current = captured, restoring_hash
                    else:
                        outcome["status"] = "recovery-required"
                        outcome["recoverySource"] = str(restore)
            except BaseException as rollback_error:
                outcome.update(status="recovery-required", error=str(rollback_error))
            report["rollback"].append(outcome)
    finally:
        # Preserve captured bytes in the transaction before removing our temporary names.
        recovery = transaction / "captured"
        for temporary in owned_temporaries:
            if not temporary.exists():
                continue
            try:
                if "displaced" in temporary.name or "capture" in temporary.name:
                    recovery.mkdir(exist_ok=True)
                    saved = recovery / (uuid.uuid4().hex + ".bin")
                    durable_copy(temporary, saved)
                    report.setdefault("capturedBackups", []).append({"temporary": str(temporary), "backup": str(saved), "sha256": sha(saved)})
                    for outcome in report["rollback"]:
                        if outcome.get("recoverySource") == str(temporary):
                            outcome["recoverySource"] = str(saved)
                temporary.unlink()
            except OSError as cleanup_error:
                # Never erase the sole displaced copy when durable backup fails.
                report["status"] = "recovery-required"
                report.setdefault("cleanupErrors", []).append({"path": str(temporary), "error": str(cleanup_error)})
        write_receipt(receipt, report)
    return report


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--candidate-root", type=Path, required=True)
    parser.add_argument("--repository-root", type=Path, default=Path(__file__).resolve().parents[2])
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--dry-run", action="store_true", help="Default: review only")
    mode.add_argument("--install", action="store_true", help="Install the reviewed candidate with backup and CAS checks")
    parser.add_argument("--report", type=Path, help="Optional detailed JSON report under repository out/")
    args = parser.parse_args(argv)
    try:
        plan = inspect(args.candidate_root, args.repository_root)
        output = args.report.resolve() if args.report else None
        if output is not None:
            out_root = bounded(plan.root, "out").resolve()
            if not output.is_relative_to(out_root) or output.suffix.lower() != ".json":
                raise InstallError("Review report must be a JSON file under repository out/")
            protected = {bounded(plan.candidate, "manifest.json").resolve()}
            protected.update(entry.source.resolve() for entry in plan.entries)
            protected.update(entry.baseline.resolve() for entry in plan.entries if entry.baseline)
            protected.update(entry.destination.resolve() for entry in plan.entries)
            if output in protected:
                raise InstallError("Review report cannot overwrite a candidate, baseline, manifest or destination")
        report = install(plan) if args.install and plan.report["ready"] else plan.report
        if output is not None:
            output.parent.mkdir(parents=True, exist_ok=True)
            write_receipt(output, report)
        summary = {key: report[key] for key in ("mode", "ready", "status", "transactionPath", "mediaCount", "mediaDuplicateCount", "conflicts") if key in report}
        print(json.dumps(summary, ensure_ascii=False))
        return 0 if report["ready"] and report.get("status", "dry-run") in {"dry-run", "installed"} else 2
    except (InstallError, OSError, ValueError, KeyError, TypeError) as error:
        print(json.dumps({"status": "rejected", "error": str(error)}, ensure_ascii=False), file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())

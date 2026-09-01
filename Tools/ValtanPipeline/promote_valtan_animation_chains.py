#!/usr/bin/env python3
"""Promote reviewed Valtan animation chains into managed audition patterns.

The animator-owned debug document remains an intake library.  This projector
freezes its native clip lengths against the cooked Valtan model and produces
the same split gameplay/presentation contract used by phase one.  Every source
occurrence becomes one Server stage, so an explicitly extended clip can loop
for exactly the authored wall time without changing the next occurrence.

Promotion is intentionally non-destructive to live combat: every generated
pattern is owned by ``decisionModel.manualAuditions`` and compiles to
``AUDITION_ONLY``.  Pattern/effect authors can therefore play it through the
existing stable-ID Server audition command while normal rotation ignores it.
"""

from __future__ import annotations

import argparse
import contextlib
import copy
import hashlib
import json
import math
import os
import re
import shutil
import sys
import time
import uuid
from pathlib import Path
from typing import Any, Iterator, Mapping


MANIFEST_REL = "Data/Valtan/Valtan.animation-chain-promotions.json"
DEBUG_REL = "Data/Valtan/Valtan.presentation.debug.json"
GAMEPLAY_REL = "Data/Valtan/Valtan.gameplay.json"
PRESENTATION_REL = "Data/Valtan/Valtan.presentation.json"
PATTERN_SOUND_REL = (
    "Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json"
)
EFFECT_V2_BINDINGS_REL = (
    "Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json"
)
ROOT_MOTION_REL = "Data/Animation/RootMotion/Valtan.rootmotion.json"
RECEIPT_REL = "Data/Valtan/Valtan.animation-chain-promotion.receipt.json"
ANIM_NOTIFY_REL = "Data/Animation/Reference/Valtan/Valtan.animnotify"
CREATE_REQUEST_SCHEMA = "lostark.valtan-animation-pattern-create-request"
CREATE_REQUEST_FORMAT_VERSION = 1
CREATE_RESULT_SCHEMA = "lostark.valtan-animation-pattern-create-result"
CREATE_RESULT_FORMAT_VERSION = 1
PRODUCT_COMMIT_RESULT_SCHEMA = "lostark.valtan-product-transaction-commit-result"
PRODUCT_COMMIT_RESULT_FORMAT_VERSION = 1
TYPED_PATCH_COMMIT_RESULT_SCHEMA = "lostark.valtan-typed-authoring-commit-result"
TYPED_PATCH_COMMIT_RESULT_FORMAT_VERSION = 1
TRANSACTION_LOCK_REL = "out/ValtanPatternTransactions/create-pattern.lock"
TRANSACTION_GENERATION_ROOT_REL = "out/ValtanPatternTransactions/generations"
TRANSACTION_ACTIVE_REL = "out/ValtanPatternTransactions/active-generation.json"
TRANSACTION_GENERATION_SCHEMA = "lostark.valtan-animation-pattern-transaction-generation"
TRANSACTION_GENERATION_FORMAT_VERSION = 1
TRANSACTION_ACTIVE_SCHEMA = "lostark.valtan-animation-pattern-active-generation"
TRANSACTION_ACTIVE_FORMAT_VERSION = 1
EXTERNAL_LOCK_MARKER_PREFIX = "lostark.valtan-canonical-writer-owner-v1"
LOCK_DIAGNOSTIC_MARKER_PREFIX = (
    "lostark.valtan-canonical-writer-diagnostic-v1"
)
LOCK_DIAGNOSTIC_MAX_BYTES = 4096
LOCK_OPERATION = re.compile(r"^[A-Za-z0-9_.-]{1,96}$")

STABLE_ID = re.compile(r"^[A-Za-z0-9_.-]{1,160}$")
SHA256 = re.compile(r"^[0-9a-f]{64}$")
ANIMATION_INTAKE_ONLY = "ANIMATION_INTAKE_ONLY"


class PromotionError(RuntimeError):
    pass


class CanonicalTransactionBusyError(PromotionError):
    error_code = "CANONICAL_TRANSACTION_BUSY"

    def __init__(self, message: str, lock_owner: Mapping[str, Any]) -> None:
        super().__init__(message)
        self.lock_owner = dict(lock_owner)
        self.lock_owner_pid = self.lock_owner["pid"]
        self.lock_owner_operation = self.lock_owner["operation"]
        self.lock_acquisition_age_ms = self.lock_owner["acquisitionAgeMs"]
        self.lock_owner_relation = self.lock_owner["relation"]

    def as_error(self) -> dict[str, Any]:
        return {
            "errorCode": self.error_code,
            "message": str(self),
            "lockOwner": dict(self.lock_owner),
        }


def _process_is_ancestor(ancestor_pid: int) -> bool:
    if ancestor_pid <= 0:
        return False
    if os.name != "nt":
        return os.getppid() == ancestor_pid

    import ctypes
    from ctypes import wintypes

    class ProcessEntry32W(ctypes.Structure):
        _fields_ = (
            ("dwSize", wintypes.DWORD),
            ("cntUsage", wintypes.DWORD),
            ("th32ProcessID", wintypes.DWORD),
            ("th32DefaultHeapID", ctypes.c_size_t),
            ("th32ModuleID", wintypes.DWORD),
            ("cntThreads", wintypes.DWORD),
            ("th32ParentProcessID", wintypes.DWORD),
            ("pcPriClassBase", wintypes.LONG),
            ("dwFlags", wintypes.DWORD),
            ("szExeFile", wintypes.WCHAR * 260),
        )

    kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
    kernel32.CreateToolhelp32Snapshot.argtypes = (wintypes.DWORD, wintypes.DWORD)
    kernel32.CreateToolhelp32Snapshot.restype = wintypes.HANDLE
    kernel32.Process32FirstW.argtypes = (
        wintypes.HANDLE,
        ctypes.POINTER(ProcessEntry32W),
    )
    kernel32.Process32FirstW.restype = wintypes.BOOL
    kernel32.Process32NextW.argtypes = (
        wintypes.HANDLE,
        ctypes.POINTER(ProcessEntry32W),
    )
    kernel32.Process32NextW.restype = wintypes.BOOL
    kernel32.CloseHandle.argtypes = (wintypes.HANDLE,)
    kernel32.CloseHandle.restype = wintypes.BOOL
    snapshot = kernel32.CreateToolhelp32Snapshot(0x00000002, 0)
    if snapshot == ctypes.c_void_p(-1).value:
        return False
    parents: dict[int, int] = {}
    try:
        entry = ProcessEntry32W()
        entry.dwSize = ctypes.sizeof(ProcessEntry32W)
        has_entry = kernel32.Process32FirstW(snapshot, ctypes.byref(entry))
        while has_entry:
            parents[int(entry.th32ProcessID)] = int(entry.th32ParentProcessID)
            has_entry = kernel32.Process32NextW(snapshot, ctypes.byref(entry))
    finally:
        kernel32.CloseHandle(snapshot)

    current = os.getpid()
    seen: set[int] = set()
    while current not in seen and current > 0:
        if current == ancestor_pid:
            return True
        seen.add(current)
        current = parents.get(current, 0)
    return False


def _unknown_lock_owner() -> dict[str, Any]:
    return {
        "pid": None,
        "operation": "UNKNOWN",
        "acquisitionAgeMs": None,
        "relation": "UNKNOWN",
    }


def _parse_lock_diagnostic_marker(marker: bytes) -> dict[str, Any] | None:
    prefix = (LOCK_DIAGNOSTIC_MARKER_PREFIX + ":").encode("ascii")
    if not marker.startswith(prefix) or not marker.endswith(b"\n"):
        return None
    try:
        payload = json.loads(marker[len(prefix) : -1].decode("ascii"))
    except (UnicodeError, ValueError):
        return None
    if not isinstance(payload, dict) or set(payload) != {
        "acquiredAtUnixMs",
        "operation",
        "pid",
    }:
        return None
    pid = payload["pid"]
    operation = payload["operation"]
    acquired_at_ms = payload["acquiredAtUnixMs"]
    if (
        not isinstance(pid, int)
        or isinstance(pid, bool)
        or pid <= 0
        or not isinstance(operation, str)
        or LOCK_OPERATION.fullmatch(operation) is None
        or not isinstance(acquired_at_ms, int)
        or isinstance(acquired_at_ms, bool)
        or acquired_at_ms <= 0
    ):
        return None
    now_ms = time.time_ns() // 1_000_000
    if acquired_at_ms > now_ms:
        return None
    return {
        "pid": pid,
        "operation": operation,
        "acquisitionAgeMs": now_ms - acquired_at_ms,
    }


def _read_lock_owner(handle: Any) -> dict[str, Any]:
    owner = _unknown_lock_owner()
    try:
        handle.seek(1)
        raw = handle.read(LOCK_DIAGNOSTIC_MAX_BYTES + 1)
    except OSError:
        return owner
    if len(raw) > LOCK_DIAGNOSTIC_MAX_BYTES:
        return owner

    lines = raw.splitlines(keepends=True)
    external_prefix = (EXTERNAL_LOCK_MARKER_PREFIX + ":").encode("ascii")
    external_pid: int | None = None
    diagnostic: dict[str, Any] | None = None
    for line in lines:
        if line.startswith(external_prefix):
            try:
                pid_text, nonce = line[len(external_prefix) : -1].decode("ascii").split(
                    ":", 1
                )
                parsed_pid = int(pid_text)
                if parsed_pid > 0 and re.fullmatch(r"[0-9a-f]{32}", nonce):
                    external_pid = parsed_pid
            except (UnicodeError, ValueError):
                pass
        parsed_diagnostic = _parse_lock_diagnostic_marker(line)
        if parsed_diagnostic is not None:
            diagnostic = parsed_diagnostic

    if diagnostic is not None and (
        external_pid is None or diagnostic["pid"] == external_pid
    ):
        owner.update(diagnostic)
    elif external_pid is not None:
        owner["pid"] = external_pid

    owner_pid = owner["pid"]
    if owner_pid == os.getpid():
        owner["relation"] = "SELF"
    elif isinstance(owner_pid, int) and _process_is_ancestor(owner_pid):
        owner["relation"] = "ANCESTOR"
    elif isinstance(owner_pid, int):
        owner["relation"] = "OTHER_PROCESS"
    return owner


def _lock_diagnostic_marker(operation: str) -> bytes:
    payload = {
        "acquiredAtUnixMs": time.time_ns() // 1_000_000,
        "operation": operation,
        "pid": os.getpid(),
    }
    return (
        LOCK_DIAGNOSTIC_MARKER_PREFIX
        + ":"
        + json.dumps(payload, ensure_ascii=True, separators=(",", ":"), sort_keys=True)
        + "\n"
    ).encode("ascii")


def _canonical_busy_error(owner: Mapping[str, Any]) -> CanonicalTransactionBusyError:
    relation = owner["relation"]
    if relation == "SELF":
        summary = "Create Pattern transaction lock is held by this process (nested acquisition)"
    elif relation == "ANCESTOR":
        summary = "Create Pattern transaction lock is held by an ancestor process"
    else:
        summary = "Create Pattern transaction lock is held by another process"
    pid = owner["pid"] if owner["pid"] is not None else "UNKNOWN"
    age = (
        owner["acquisitionAgeMs"]
        if owner["acquisitionAgeMs"] is not None
        else "UNKNOWN"
    )
    message = (
        f"{summary}; lockOwnerPid={pid}; "
        f"lockOwnerOperation={owner['operation']}; "
        f"lockAcquisitionAgeMs={age}; lockOwnerRelation={relation}"
    )
    return CanonicalTransactionBusyError(message, owner)


@contextlib.contextmanager
def _exclusive_transaction_lock(
    repo_root: Path,
    *,
    timeout_seconds: float = 0.0,
    operation: str = "UNKNOWN",
) -> Iterator[Path]:
    """Serialize every Valtan Create/Project writer across processes.

    The lock lives under ignored ``out`` state rather than beside a canonical
    Data owner.  It is intentionally retained after release: deleting a lock
    path while another process is waiting on the old inode would create two
    independent writer locks.
    """

    if not math.isfinite(timeout_seconds) or timeout_seconds < 0.0:
        raise PromotionError("transaction lock timeout must be finite and non-negative")
    if not isinstance(operation, str) or LOCK_OPERATION.fullmatch(operation) is None:
        raise PromotionError("transaction lock operation identity is invalid")
    repo_root = repo_root.resolve()
    lock_path = repo_root / TRANSACTION_LOCK_REL
    lock_path.parent.mkdir(parents=True, exist_ok=True)
    descriptor: int | None = None
    try:
        descriptor = os.open(
            lock_path,
            os.O_RDWR | os.O_CREAT | getattr(os, "O_BINARY", 0),
            0o666,
        )
        handle = os.fdopen(descriptor, "r+b")
        descriptor = None
    except OSError as exc:
        if descriptor is not None:
            os.close(descriptor)
        raise PromotionError(f"cannot open Create Pattern transaction lock: {exc}") from exc

    acquired = False
    deadline = time.monotonic() + timeout_seconds
    try:
        while True:
            try:
                handle.seek(0)
                if os.name == "nt":
                    import msvcrt

                    if lock_path.stat().st_size < 1:
                        handle.write(b"\0")
                        handle.flush()
                        os.fsync(handle.fileno())
                        handle.seek(0)
                    msvcrt.locking(handle.fileno(), msvcrt.LK_NBLCK, 1)
                else:
                    import fcntl

                    fcntl.flock(handle.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                acquired = True
                break
            except (OSError, BlockingIOError):
                if time.monotonic() >= deadline:
                    raise _canonical_busy_error(_read_lock_owner(handle))
                time.sleep(min(0.025, max(0.0, deadline - time.monotonic())))
        marker = _lock_diagnostic_marker(operation)
        handle.seek(0)
        handle.truncate(1 + len(marker))
        handle.write(b"\0" + marker)
        handle.flush()
        os.fsync(handle.fileno())
        _recover_incomplete_product_transaction(repo_root)
        yield lock_path
    finally:
        if acquired:
            try:
                handle.seek(0)
                handle.truncate(1)
                handle.write(b"\0")
                handle.flush()
                os.fsync(handle.fileno())
                handle.seek(0)
                if os.name == "nt":
                    import msvcrt

                    msvcrt.locking(handle.fileno(), msvcrt.LK_UNLCK, 1)
                else:
                    import fcntl

                    fcntl.flock(handle.fileno(), fcntl.LOCK_UN)
            except OSError:
                # Closing the descriptor releases the OS lock even if an
                # explicit unlock reports a late shutdown error.
                pass
        handle.close()


def _assert_external_transaction_lock(
    repo_root: Path,
    owner_pid: int,
    owner_nonce: str,
) -> None:
    """Require this child to be running below the process holding the lock."""

    def is_process_ancestor(ancestor_pid: int) -> bool:
        if ancestor_pid <= 0:
            return False
        if os.name != "nt":
            return os.getppid() == ancestor_pid

        import ctypes
        from ctypes import wintypes

        class ProcessEntry32W(ctypes.Structure):
            _fields_ = (
                ("dwSize", wintypes.DWORD),
                ("cntUsage", wintypes.DWORD),
                ("th32ProcessID", wintypes.DWORD),
                ("th32DefaultHeapID", ctypes.c_size_t),
                ("th32ModuleID", wintypes.DWORD),
                ("cntThreads", wintypes.DWORD),
                ("th32ParentProcessID", wintypes.DWORD),
                ("pcPriClassBase", wintypes.LONG),
                ("dwFlags", wintypes.DWORD),
                ("szExeFile", wintypes.WCHAR * 260),
            )

        kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
        kernel32.CreateToolhelp32Snapshot.argtypes = (wintypes.DWORD, wintypes.DWORD)
        kernel32.CreateToolhelp32Snapshot.restype = wintypes.HANDLE
        kernel32.Process32FirstW.argtypes = (
            wintypes.HANDLE,
            ctypes.POINTER(ProcessEntry32W),
        )
        kernel32.Process32FirstW.restype = wintypes.BOOL
        kernel32.Process32NextW.argtypes = (
            wintypes.HANDLE,
            ctypes.POINTER(ProcessEntry32W),
        )
        kernel32.Process32NextW.restype = wintypes.BOOL
        kernel32.CloseHandle.argtypes = (wintypes.HANDLE,)
        kernel32.CloseHandle.restype = wintypes.BOOL
        snapshot = kernel32.CreateToolhelp32Snapshot(0x00000002, 0)
        invalid_handle = ctypes.c_void_p(-1).value
        if snapshot == invalid_handle:
            return False
        parents: dict[int, int] = {}
        try:
            entry = ProcessEntry32W()
            entry.dwSize = ctypes.sizeof(ProcessEntry32W)
            first = kernel32.Process32FirstW(snapshot, ctypes.byref(entry))
            while first:
                parents[int(entry.th32ProcessID)] = int(entry.th32ParentProcessID)
                first = kernel32.Process32NextW(snapshot, ctypes.byref(entry))
        finally:
            kernel32.CloseHandle(snapshot)

        current = os.getpid()
        seen: set[int] = set()
        while current not in seen and current > 0:
            if current == ancestor_pid:
                return True
            seen.add(current)
            current = parents.get(current, 0)
        return False

    if not is_process_ancestor(owner_pid):
        raise PromotionError(
            "external writer admission owner must be the Product projector parent"
        )
    if re.fullmatch(r"[0-9a-f]{32}", owner_nonce) is None:
        raise PromotionError("external writer admission nonce is invalid")
    try:
        with _exclusive_transaction_lock(
            repo_root,
            timeout_seconds=0.0,
            operation="ExternalAdmissionProbe",
        ):
            pass
    except CanonicalTransactionBusyError:
        pass
    else:
        raise PromotionError(
            "external writer admission was requested without a held create-pattern.lock"
        )

    lock_path = repo_root.resolve() / TRANSACTION_LOCK_REL
    expected_marker = f"{EXTERNAL_LOCK_MARKER_PREFIX}:{owner_pid}:{owner_nonce}\n".encode(
        "ascii"
    )
    try:
        with lock_path.open("rb") as marker_file:
            marker_file.seek(1)
            marker = marker_file.read()
    except OSError as exc:
        raise PromotionError(
            f"cannot read external writer admission marker: {exc}"
        ) from exc
    if marker != expected_marker:
        raise PromotionError(
            "external writer admission marker does not match its parent owner"
        )


def _reject_duplicate_pairs(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise PromotionError(f"duplicate JSON property: {key}")
        result[key] = value
    return result


def _read_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(
            path.read_text(encoding="utf-8"),
            object_pairs_hook=_reject_duplicate_pairs,
        )
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise PromotionError(f"cannot read strict JSON {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise PromotionError(f"JSON root must be an object: {path}")
    return value


def _exact(value: Mapping[str, Any], fields: tuple[str, ...], context: str) -> None:
    if not isinstance(value, dict) or set(value) != set(fields):
        actual = sorted(value) if isinstance(value, dict) else type(value).__name__
        raise PromotionError(
            f"{context} properties mismatch: expected={list(fields)} actual={actual}"
        )


def _required_with_optional(
    value: Mapping[str, Any],
    required: tuple[str, ...],
    optional: tuple[str, ...],
    context: str,
) -> None:
    if not isinstance(value, dict):
        raise PromotionError(
            f"{context} properties mismatch: expected object actual={type(value).__name__}"
        )
    actual = set(value)
    required_set = set(required)
    allowed = required_set | set(optional)
    if not required_set.issubset(actual) or not actual.issubset(allowed):
        raise PromotionError(
            f"{context} properties mismatch: required={list(required)} "
            f"optional={list(optional)} actual={sorted(actual)}"
        )


def _stable(value: Any, context: str) -> str:
    if not isinstance(value, str) or STABLE_ID.fullmatch(value) is None:
        raise PromotionError(f"{context} is not a stable ID: {value!r}")
    return value


def _integer(value: Any, context: str, minimum: int = 0) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value < minimum:
        raise PromotionError(f"{context} must be an integer >= {minimum}")
    return value


def _number(value: Any, context: str, minimum: float = 0.0) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise PromotionError(f"{context} must be a finite number")
    result = float(value)
    if not math.isfinite(result) or result < minimum:
        raise PromotionError(f"{context} must be a finite number >= {minimum}")
    return result


def _sha256(path: Path) -> str:
    digest = hashlib.sha256()
    try:
        with path.open("rb") as source:
            for block in iter(lambda: source.read(1024 * 1024), b""):
                digest.update(block)
    except OSError as exc:
        raise PromotionError(f"cannot hash {path}: {exc}") from exc
    return digest.hexdigest()


def _sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def _read_bytes_or_none(path: Path) -> bytes | None:
    try:
        return path.read_bytes() if path.exists() else None
    except OSError as exc:
        raise PromotionError(f"cannot read transaction baseline {path}: {exc}") from exc


def _read_json_bytes(payload: bytes | None, path: Path) -> dict[str, Any]:
    if payload is None:
        raise PromotionError(f"required JSON document is missing: {path}")
    try:
        value = json.loads(
            payload.decode("utf-8"),
            object_pairs_hook=_reject_duplicate_pairs,
        )
    except (UnicodeError, json.JSONDecodeError) as exc:
        raise PromotionError(f"cannot read strict JSON {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise PromotionError(f"JSON root must be an object: {path}")
    return value


def _lround_positive(value: float) -> int:
    if not math.isfinite(value) or value < 0.0:
        raise PromotionError(f"cannot round invalid duration: {value}")
    return int(math.floor(value + 0.5))


def _json_text(value: Any) -> str:
    return json.dumps(value, ensure_ascii=False, indent=2) + "\n"


def _reviewed_closure_counts(
    promotions: list[dict[str, Any]], chains: list[dict[str, Any]]
) -> tuple[int, int]:
    """Return the closure reviewed by the manifest/debug exact-order join."""

    return len(promotions), sum(
        len(chain["animation"]["occurrences"]) for chain in chains
    )


def _load_clip_skills(path: Path) -> dict[str, int]:
    rows: dict[str, int] = {}
    pattern = re.compile(r'^"([^"]+)" skill=([0-9]+) ')
    try:
        lines = path.read_text(encoding="utf-8").splitlines()
    except (OSError, UnicodeError) as exc:
        raise PromotionError(f"cannot read animation provenance {path}: {exc}") from exc
    for line in lines:
        match = pattern.match(line)
        if match is None:
            continue
        clip = match.group(1)
        skill = int(match.group(2))
        if clip in rows and rows[clip] != skill:
            raise PromotionError(f"clip has conflicting source skills: {clip}")
        rows[clip] = skill
    if not rows:
        raise PromotionError("Valtan.animnotify contains no clip skill rows")
    return rows


def _load_root_curves(repo_root: Path, model_path: Path) -> dict[str, tuple[float, float, list]]:
    # The model may live in an isolated transaction fixture.  Python tooling,
    # however, is owned by this checkout, not by the candidate data root.
    # Resolve the package root from this module so direct invocation and
    # temp-root tests do not depend on the caller's current working directory.
    tooling_root = str(Path(__file__).resolve().parents[2])
    if tooling_root not in sys.path:
        sys.path.insert(0, tooling_root)
    try:
        from Tools.ValtanActionExtractor.build_valtan_rootmotion import read_root_curves
    except ImportError as exc:
        raise PromotionError(f"cannot import WModel duration reader: {exc}") from exc
    try:
        curves = read_root_curves(model_path)
    except (OSError, SystemExit, ValueError) as exc:
        raise PromotionError(f"cannot read Valtan WModel animation table: {exc}") from exc
    if not curves:
        raise PromotionError("Valtan WModel animation table is empty")
    return curves


def _validate_header(manifest: dict[str, Any], debug: dict[str, Any]) -> None:
    _exact(
        manifest,
        (
            "schema",
            "formatVersion",
            "bossArchetypeId",
            "encounterId",
            "sourceDocument",
            "presentationProfile",
            "clipAliases",
            "animationIntakeOnly",
            "patterns",
        ),
        "promotion manifest",
    )
    if (
        manifest["schema"] != "lostark.valtan-animation-chain-promotions"
        or manifest["formatVersion"] != 2
        or manifest["bossArchetypeId"] != "BOSS_VALTAN"
        or manifest["encounterId"] != "ENCOUNTER_VALTAN"
    ):
        raise PromotionError("promotion manifest header mismatch")
    _exact(
        debug,
        ("schema", "formatVersion", "bossArchetypeId", "encounterId", "chains"),
        "debug source",
    )
    if (
        debug["schema"] != "lostark.valtan-pattern-presentation-debug"
        or debug["formatVersion"] != 1
        or debug["bossArchetypeId"] != manifest["bossArchetypeId"]
        or debug["encounterId"] != manifest["encounterId"]
    ):
        raise PromotionError("debug source header mismatch")


def _validate_manifest_paths(
    repo_root: Path,
    manifest: dict[str, Any],
    *,
    source_sha256: str | None = None,
) -> tuple[Path, Path]:
    source = manifest["sourceDocument"]
    _exact(source, ("path", "sha256"), "sourceDocument")
    if source["path"] != DEBUG_REL or not SHA256.fullmatch(source["sha256"]):
        raise PromotionError("sourceDocument identity is invalid")
    source_path = repo_root / source["path"]
    actual_source_sha256 = (
        _sha256(source_path) if source_sha256 is None else source_sha256
    )
    if actual_source_sha256 != source["sha256"]:
        raise PromotionError(
            "animation chain source changed; review its rows and refresh the manifest hash"
        )

    profile = manifest["presentationProfile"]
    _exact(
        profile,
        ("profileId", "modelAssetId", "modelPath", "modelSha256"),
        "presentationProfile",
    )
    _stable(profile["profileId"], "presentationProfile.profileId")
    model_asset = profile["modelAssetId"]
    if (
        not isinstance(model_asset, str)
        or "\\" in model_asset
        or model_asset.startswith("/")
        or ".." in model_asset.split("/")
        or not model_asset.startswith("Character/Valtan/")
        or not model_asset.endswith(".wmodel")
    ):
        raise PromotionError("presentationProfile.modelAssetId is outside Valtan Resources")
    model_relative = profile["modelPath"]
    if (
        not isinstance(model_relative, str)
        or "\\" in model_relative
        or model_relative.startswith("/")
        or ".." in model_relative.split("/")
        or not SHA256.fullmatch(profile["modelSha256"])
    ):
        raise PromotionError("presentationProfile model identity is invalid")
    model_path = repo_root / model_relative
    if _sha256(model_path) != profile["modelSha256"]:
        raise PromotionError("Valtan WModel changed; refresh durations and review before promotion")
    return source_path, model_path


def _manual_gameplay_pattern(
    promotion: dict[str, Any],
    source_action_ids: list[int],
    stages: list[dict[str, Any]],
) -> dict[str, Any]:
    chain_id = promotion["sourceChainId"]
    return {
        "patternId": promotion["patternId"],
        "displayName": promotion["displayName"],
        "category": "NORMAL",
        "compatibilitySelectionWeight": 0,
        "actionId": f"valtan.sequence.{chain_id}",
        "entryActionId": stages[0]["actionId"],
        "targetPolicy": promotion.get("targetPolicy", "NONE"),
        "aimPolicy": promotion.get("aimPolicy", "NONE"),
        "eligibility": {
            "armorRequirement": "ANY",
            "phaseRequirement": "ANY",
            "minimumGameplayPhase": 1,
            "maximumGameplayPhase": 3,
            "minimumHealthBarInclusive": 0,
            "maximumHealthBarInclusive": 0,
            "minimumRangeM": 0.0,
            "maximumRangeM": 1.0,
            "cooldownPolicy": "DERIVED_SOURCE_ACTION",
            "selectionCooldownMs": None,
            "cooldownGroupId": None,
            "repeatPolicy": {
                "kind": "SOFT_AVOID_UNLESS_ONLY_ELIGIBLE",
                "limit": 0,
            },
        },
        "invulnerableWhileRunning": False,
        "sourceActionIds": source_action_ids,
        "serverMotion": None,
        "reactions": [],
        "stages": [
            {
                "stageId": stage["stageId"],
                "actionId": stage["actionId"],
                "stageKind": "ACTIVE",
                "durationMs": stage["durationMs"],
                "defaultNextActionId": (
                    stages[index + 1]["actionId"]
                    if index + 1 < len(stages)
                    else None
                ),
                "hit": {"shape": {"kind": "NONE"}},
                "motion": None,
                "events": [],
                "branches": [],
            }
            for index, stage in enumerate(stages)
        ],
    }


def _manual_presentation_pattern(
    promotion: dict[str, Any],
    source_action_ids: list[int],
    stages: list[dict[str, Any]],
) -> dict[str, Any]:
    primary_action_id = promotion.get("sourceActionId", source_action_ids[0])
    if primary_action_id not in source_action_ids:
        raise PromotionError(
            "reviewed primary source action is absent from the promoted Animation chain: "
            f"{primary_action_id}"
        )
    primary_sequence_index = promotion.get("sourceSequenceIndex", 1)
    ordered_source_action_ids = [primary_action_id] + [
        source_action_id
        for source_action_id in source_action_ids
        if source_action_id != primary_action_id
    ]
    sources = [
        {
            "sourceActionId": source_action_id,
            "sequenceIndex": (
                primary_sequence_index if index == 0 else 1
            ),
            "role": "PRIMARY" if index == 0 else "REFERENCE",
        }
        for index, source_action_id in enumerate(ordered_source_action_ids)
    ]
    return {
        "patternId": promotion["patternId"],
        "sourceSequenceIndex": primary_sequence_index,
        "presentationSources": sources,
        "stages": [
            {
                "stageId": stage["stageId"],
                "actionId": stage["actionId"],
                "sequenceRole": "STEP",
                "animation": {
                    "endPolicy": stage["endPolicy"],
                    "repeatCount": 1,
                    "occurrences": [stage["occurrence"]],
                },
                "effectCues": [],
                "cameraInvocations": [],
            }
            for stage in stages
        ],
    }


def _expand_trash_capture_promotion(
    generated_gameplay: dict[str, Any],
    generated_presentation: dict[str, Any],
    existing_gameplay: dict[str, Any] | None,
    existing_presentation: dict[str, Any] | None,
) -> tuple[dict[str, Any], dict[str, Any]]:
    """Join the reviewed eight-clip intake to the authored capture graph.

    The six outcome stages belong to the phase-two author, not the animation
    intake. Reuse that author before the ordinary enrichment identity checks;
    do not accept arbitrary extra stages or quietly repair their fixed clocks.
    """
    # The author imports this module's readers, so defer the reverse dependency.
    from author_valtan_phase_two_mechanics import author_trash_capture_flow

    intake_ids = [f"STEP_{index:02d}" for index in range(1, 9)]
    for generated in (generated_gameplay, generated_presentation):
        if generated.get("patternId") != "VALTAN_TRASH" or [
            row.get("stageId") for row in generated.get("stages", [])
        ] != intake_ids:
            raise PromotionError("Trash capture promotion requires the reviewed eight-stage intake")
    if (existing_gameplay is None) != (existing_presentation is None):
        raise PromotionError("Trash capture promotion requires paired split source rows")

    # Keep the receipt's sourceActionIds/occurrences limited to the intake.
    gameplay = copy.deepcopy(generated_gameplay)
    presentation = copy.deepcopy(generated_presentation)
    author_trash_capture_flow({"patterns": [gameplay]}, {"patterns": [presentation]},
                              audition_pattern_ids=())
    expected_ids = [row["stageId"] for row in gameplay["stages"]]
    for domain, generated, existing in (
        ("gameplay", gameplay, existing_gameplay),
        ("presentation", presentation, existing_presentation),
    ):
        if existing is None:
            continue
        rows = existing.get("stages")
        if not isinstance(rows, list) or any(not isinstance(row, dict) for row in rows) or [
            row.get("stageId") for row in rows
        ] != expected_ids:
            raise PromotionError(f"Trash capture {domain} stage closure/order drift")
        for expected, current in zip(generated["stages"], rows):
            stage_id = expected["stageId"]
            if current.get("actionId") != expected["actionId"]:
                raise PromotionError(f"Trash capture {domain} action identity drift: {stage_id}")
            if domain == "gameplay" and "counterProxy" in expected and "counterProxy" not in current:
                raise PromotionError(f"Trash capture counter proxy is missing: {stage_id}")
            if stage_id in intake_ids:
                continue
            if domain == "gameplay":
                duration = _integer(current.get("durationMs"), f"Trash/{stage_id}.durationMs", 1)
                if duration != expected["durationMs"]:
                    raise PromotionError(f"Trash capture branch duration drift: {stage_id}")
                if "counterProxy" in expected and "counterProxy" not in current:
                    raise PromotionError(f"Trash capture counter proxy is missing: {stage_id}")
            elif current.get("animation") != expected["animation"]:
                raise PromotionError(f"Trash capture branch source slice drift: {stage_id}")
    return gameplay, presentation


def _preserve_manual_gameplay_enrichment(
    generated: dict[str, Any],
    existing: dict[str, Any] | None,
    *,
    preserve_sequence_append: bool = False,
) -> dict[str, Any]:
    """Keep reviewed gameplay semantics while rebuilding animation lineage.

    The promotion tool owns clip identity and stage walls. Pattern authors own
    target/aim, hit, motion, event and branch semantics after that promotion.
    Re-running the animation resolver must not silently turn a working Server
    pattern back into an animation-only audition.
    """

    if existing is None:
        return generated
    generated_stages = {
        stage["stageId"]: stage for stage in generated["stages"]
    }
    existing_stages = {
        stage.get("stageId"): stage
        for stage in existing.get("stages", [])
        if isinstance(stage, dict)
    }
    if set(generated_stages) != set(existing_stages):
        raise PromotionError(
            f"manual gameplay enrichment stage closure drift: {generated['patternId']}"
        )
    for field in (
        "targetPolicy",
        "aimPolicy",
        "eligibility",
        "invulnerableWhileRunning",
        "serverMotion",
        "reactions",
    ):
        if field in existing:
            generated[field] = copy.deepcopy(existing[field])
    if preserve_sequence_append:
        generated_sources = generated.get("sourceActionIds")
        existing_sources = existing.get("sourceActionIds")
        if (
            not isinstance(generated_sources, list)
            or not isinstance(existing_sources, list)
            or len(existing_sources) < len(generated_sources)
            or existing_sources[: len(generated_sources)] != generated_sources
            or len(existing_sources) != len(set(existing_sources))
            or any(
                not isinstance(source_action_id, int)
                or isinstance(source_action_id, bool)
                or source_action_id <= 0
                for source_action_id in existing_sources
            )
        ):
            raise PromotionError(
                "manual Sequence append gameplay provenance drift: "
                f"{generated['patternId']}"
            )
        generated["sourceActionIds"] = copy.deepcopy(existing_sources)
    generated_action_ids = {
        stage["actionId"] for stage in generated_stages.values()
    }
    for stage_id, generated_stage in generated_stages.items():
        existing_stage = existing_stages[stage_id]
        if existing_stage.get("actionId") != generated_stage["actionId"]:
            raise PromotionError(
                "manual gameplay enrichment action identity drift: "
                f"{generated['patternId']}/{stage_id}"
            )
        for field in (
            "stageKind",
            "defaultNextActionId",
            "hit",
            "motion",
            "events",
            "branches",
            "counterProxy",
        ):
            if field in existing_stage:
                generated_stage[field] = copy.deepcopy(existing_stage[field])
        # A gameplay motion can legitimately outlive the reviewed animation
        # slice (for example the 2.3 s Portal rush uses a looping 0.9 s clip).
        # Refreshing animation lineage must not shrink the Server stage below
        # the already-authored motion clock.
        existing_duration_ms = existing_stage.get("durationMs")
        generated_duration_ms = generated_stage.get("durationMs")
        if (
            (existing_stage.get("motion") is not None or preserve_sequence_append)
            and isinstance(existing_duration_ms, int)
            and not isinstance(existing_duration_ms, bool)
            and isinstance(generated_duration_ms, int)
            and not isinstance(generated_duration_ms, bool)
            and existing_duration_ms > generated_duration_ms
        ):
            generated_stage["durationMs"] = existing_duration_ms
        next_action = generated_stage["defaultNextActionId"]
        if next_action is not None and next_action not in generated_action_ids:
            raise PromotionError(
                "manual gameplay enrichment default edge leaves its pattern: "
                f"{generated['patternId']}/{stage_id}"
            )
        for branch in generated_stage["branches"]:
            next_action = branch.get("nextActionId")
            if next_action is not None and next_action not in generated_action_ids:
                raise PromotionError(
                    "manual gameplay enrichment branch leaves its pattern: "
                    f"{generated['patternId']}/{stage_id}"
                )
    return generated


def _typed_appended_sequence_sources(
    generated: dict[str, Any], existing: dict[str, Any] | None
) -> list[dict[str, Any]]:
    if existing is None:
        return []
    generated_sources = generated.get("presentationSources")
    existing_sources = existing.get("presentationSources")
    if generated_sources is None and existing_sources is None:
        return []
    if not isinstance(generated_sources, list) or not isinstance(existing_sources, list):
        raise PromotionError(
            f"manual presentation source list is invalid: {generated['patternId']}"
        )
    if len(existing_sources) <= len(generated_sources):
        return []
    if existing_sources[: len(generated_sources)] != generated_sources:
        raise PromotionError(
            "manual Sequence append rewrote immutable presentation lineage: "
            f"{generated['patternId']}"
        )
    extras = existing_sources[len(generated_sources) :]
    identities: set[tuple[int, int]] = set()
    roles: set[str] = set()
    for source in extras:
        if not isinstance(source, dict):
            raise PromotionError(
                f"manual Sequence append source is invalid: {generated['patternId']}"
            )
        source_action_id = source.get("sourceActionId")
        sequence_index = source.get("sequenceIndex")
        role = source.get("role")
        expected_role = f"REFERENCE_{source_action_id}_{sequence_index}"
        identity = (source_action_id, sequence_index)
        if (
            not isinstance(source_action_id, int)
            or isinstance(source_action_id, bool)
            or source_action_id <= 0
            or not isinstance(sequence_index, int)
            or isinstance(sequence_index, bool)
            or not 0 <= sequence_index <= 4096
            or role != expected_role
            or identity in identities
            or role in roles
        ):
            raise PromotionError(
                "manual Sequence append requires unique deterministic provenance: "
                f"{generated['patternId']}"
            )
        identities.add(identity)
        roles.add(role)
    return extras


def _preserve_manual_presentation_enrichment(
    generated: dict[str, Any], existing: dict[str, Any] | None
) -> dict[str, Any]:
    """Keep exact-occurrence Effect/Camera joins across animation refreshes."""

    if existing is None:
        return generated
    appended_sequence_sources = _typed_appended_sequence_sources(
        generated, existing
    )
    if appended_sequence_sources:
        generated["presentationSources"] = copy.deepcopy(
            existing["presentationSources"]
        )
    generated_stages = {
        stage["stageId"]: stage for stage in generated["stages"]
    }
    existing_stages = {
        stage.get("stageId"): stage
        for stage in existing.get("stages", [])
        if isinstance(stage, dict)
    }
    if set(generated_stages) != set(existing_stages):
        raise PromotionError(
            f"manual presentation enrichment stage closure drift: {generated['patternId']}"
        )
    for stage_id, generated_stage in generated_stages.items():
        existing_stage = existing_stages[stage_id]
        if existing_stage.get("actionId") != generated_stage["actionId"]:
            raise PromotionError(
                "manual presentation enrichment action identity drift: "
                f"{generated['patternId']}/{stage_id}"
            )
        for field in ("sequenceRole", "effectCues", "cameraInvocations"):
            if field in existing_stage:
                generated_stage[field] = copy.deepcopy(existing_stage[field])
        if appended_sequence_sources:
            generated_animation = generated_stage.get("animation")
            existing_animation = existing_stage.get("animation")
            if (
                not isinstance(generated_animation, dict)
                or not isinstance(existing_animation, dict)
                or not isinstance(generated_animation.get("occurrences"), list)
                or not isinstance(existing_animation.get("occurrences"), list)
            ):
                raise PromotionError(
                    "manual Sequence append animation is invalid: "
                    f"{generated['patternId']}/{stage_id}"
                )
            existing_occurrences = existing_animation["occurrences"]
            for occurrence in existing_occurrences:
                if (
                    not isinstance(occurrence, dict)
                    or occurrence.get("mappingBasis")
                    not in ("PROJECT_AUTHORED", "SOURCE_REVIEWED_DELTA")
                ):
                    raise PromotionError(
                        "manual Sequence append occurrence provenance is invalid: "
                        f"{generated['patternId']}/{stage_id}"
                    )
            generated_stage["animation"] = copy.deepcopy(existing_animation)
    return generated


def build_candidates(
    repo_root: Path,
    *,
    manifest_document: dict[str, Any] | None = None,
    debug_document: dict[str, Any] | None = None,
    gameplay_document: dict[str, Any] | None = None,
    presentation_document: dict[str, Any] | None = None,
) -> tuple[dict[str, Any], dict[str, Any], dict[str, Any]]:
    manifest_path = repo_root / MANIFEST_REL
    manifest = (
        _read_json(manifest_path)
        if manifest_document is None
        else copy.deepcopy(manifest_document)
    )
    source_path = repo_root / DEBUG_REL
    debug = (
        _read_json(source_path)
        if debug_document is None
        else copy.deepcopy(debug_document)
    )
    _validate_header(manifest, debug)
    source_payload = (
        None
        if debug_document is None
        else _json_text(debug).encode("utf-8")
    )
    checked_source_path, model_path = _validate_manifest_paths(
        repo_root,
        manifest,
        source_sha256=(
            None if source_payload is None else _sha256_bytes(source_payload)
        ),
    )
    if checked_source_path.resolve() != source_path.resolve():
        raise PromotionError("manifest source path does not resolve to the debug document")

    aliases = manifest["clipAliases"]
    if not isinstance(aliases, dict):
        raise PromotionError("clipAliases must be an object")
    for source_clip, target_clip in aliases.items():
        _stable(source_clip, "clipAliases source")
        _stable(target_clip, f"clipAliases[{source_clip}]")

    intake_only = manifest["animationIntakeOnly"]
    promotions = manifest["patterns"]
    chains = debug["chains"]
    if (
        not isinstance(intake_only, list)
        or not isinstance(promotions, list)
        or not isinstance(chains, list)
        or not promotions
        or not chains
    ):
        raise PromotionError(
            "manifest promotions, animation intake, and debug chains must be arrays"
        )
    promotion_ids: set[str] = set()
    pattern_ids: set[str] = set()
    for ordinal, promotion in enumerate(promotions):
        _required_with_optional(
            promotion,
            (
                "sourceChainId",
                "patternId",
                "displayName",
                "authoringPhase",
                "admissionState",
            ),
            (
                "targetPolicy",
                "aimPolicy",
                "sourceActionId",
                "sourceSequenceIndex",
            ),
            f"promotion[{ordinal}]",
        )
        chain_id = _stable(promotion["sourceChainId"], f"promotion[{ordinal}].sourceChainId")
        pattern_id = _stable(promotion["patternId"], f"promotion[{ordinal}].patternId")
        if chain_id in promotion_ids or pattern_id in pattern_ids:
            raise PromotionError(f"duplicate promotion identity: {chain_id}/{pattern_id}")
        promotion_ids.add(chain_id)
        pattern_ids.add(pattern_id)
        if (
            not isinstance(promotion["displayName"], str)
            or not promotion["displayName"].strip()
            or len(promotion["displayName"]) > 64
            or _integer(promotion["authoringPhase"], "authoringPhase", 1) > 3
            or promotion["admissionState"] != "MANUAL_SERVER_AUDITION"
        ):
            raise PromotionError(f"promotion metadata is invalid: {chain_id}")
        target_policy = promotion.get("targetPolicy", "NONE")
        aim_policy = promotion.get("aimPolicy", "NONE")
        if target_policy not in {
            "NONE",
            "LOCK_NEAREST_ON_START",
            "LOCK_RANDOM_ALIVE_ON_START",
        } or aim_policy not in {
            "NONE",
            "LOCK_FACING_ON_START",
        }:
            raise PromotionError(f"promotion targeting is invalid: {chain_id}")
        if (target_policy == "NONE") != (aim_policy == "NONE"):
            raise PromotionError(
                f"promotion target and aim policies must be authored together: {chain_id}"
            )
        if "sourceActionId" in promotion:
            _integer(
                promotion["sourceActionId"],
                f"promotion[{ordinal}].sourceActionId",
                1,
            )
        if "sourceSequenceIndex" in promotion:
            if "sourceActionId" not in promotion:
                raise PromotionError(
                    f"promotion sourceSequenceIndex requires sourceActionId: {chain_id}"
                )
            source_sequence_index = _integer(
                promotion["sourceSequenceIndex"],
                f"promotion[{ordinal}].sourceSequenceIndex",
                0,
            )
            if source_sequence_index > 4096:
                raise PromotionError(
                    f"promotion[{ordinal}].sourceSequenceIndex must be <= 4096"
                )

    intake_chain_ids: list[str] = []
    for ordinal, row in enumerate(intake_only):
        _exact(
            row,
            ("sourceChainId", "displayName", "authoringPhase", "admissionState"),
            f"animationIntakeOnly[{ordinal}]",
        )
        chain_id = _stable(
            row["sourceChainId"],
            f"animationIntakeOnly[{ordinal}].sourceChainId",
        )
        if chain_id in promotion_ids or chain_id in intake_chain_ids:
            raise PromotionError(f"duplicate animation intake identity: {chain_id}")
        if (
            not isinstance(row["displayName"], str)
            or not row["displayName"].strip()
            or len(row["displayName"]) > 64
            or _integer(row["authoringPhase"], "authoringPhase", 1) > 3
            or row["admissionState"] != ANIMATION_INTAKE_ONLY
        ):
            raise PromotionError(f"animation intake metadata is invalid: {chain_id}")
        intake_chain_ids.append(chain_id)

    chain_ids = [
        _stable(chain.get("chainId"), f"debug chain[{ordinal}].chainId")
        for ordinal, chain in enumerate(chains)
        if isinstance(chain, dict)
    ]
    if len(chain_ids) != len(chains) or len(chain_ids) != len(set(chain_ids)):
        raise PromotionError("debug chain IDs are missing or duplicated")
    promoted_chain_ids = [row["sourceChainId"] for row in promotions]
    declared_chain_ids = promoted_chain_ids + intake_chain_ids
    if (
        set(declared_chain_ids) != set(chain_ids)
        or promoted_chain_ids != [value for value in chain_ids if value in promotion_ids]
        or intake_chain_ids != [value for value in chain_ids if value in intake_chain_ids]
    ):
        raise PromotionError(
            "promotion plus intake-only manifest order/closure must exactly match debug chains"
        )
    # Retiring a promoted pattern must not reorder the user's animation intake.
    # Each admission partition keeps its original relative order instead.
    chains_by_id = {chain["chainId"]: chain for chain in chains}
    promoted_chains = [chains_by_id[value] for value in promoted_chain_ids]

    curves = _load_root_curves(repo_root, model_path)
    clip_skills = _load_clip_skills(repo_root / ANIM_NOTIFY_REL)

    intake_occurrence_ids: set[str] = set()
    used_aliases: set[str] = set()
    for declaration in intake_only:
        chain_id = declaration["sourceChainId"]
        chain = chains_by_id[chain_id]
        _exact(
            chain,
            ("chainId", "targetPatternId", "targetStageId", "animation"),
            f"intake-only chain {chain_id}",
        )
        if chain["chainId"] != chain_id or chain["targetPatternId"] or chain["targetStageId"]:
            raise PromotionError(
                f"intake-only debug chain target fields must remain empty: {chain_id}"
            )
        animation = chain["animation"]
        _exact(
            animation,
            ("endPolicy", "repeatCount", "occurrences"),
            f"intake-only chain {chain_id}.animation",
        )
        occurrences = animation["occurrences"]
        if (
            animation["endPolicy"] != "NATIVE_CLIP_LENGTHS"
            or not isinstance(occurrences, list)
            or not occurrences
            or _integer(animation["repeatCount"], f"chain {chain_id}.repeatCount", 1)
            != len(occurrences)
            or len(occurrences) > 64
        ):
            raise PromotionError(f"intake-only animation header is invalid: {chain_id}")
        for ordinal, source in enumerate(occurrences, start=1):
            _exact(
                source,
                (
                    "clipOccurrenceId",
                    "clip",
                    "mappingBasis",
                    "sourceStartMs",
                    "playMs",
                    "playRate",
                    "repeatUntilStageEnd",
                ),
                f"intake-only chain {chain_id} occurrence[{ordinal}]",
            )
            occurrence_id = _stable(
                source["clipOccurrenceId"],
                f"intake-only chain {chain_id} source occurrence",
            )
            if occurrence_id in intake_occurrence_ids:
                raise PromotionError(f"duplicate intake-only occurrence: {occurrence_id}")
            intake_occurrence_ids.add(occurrence_id)
            source_clip = _stable(
                source["clip"], f"intake-only chain {chain_id} source clip"
            )
            resolved_clip = aliases.get(source_clip, source_clip)
            if resolved_clip != source_clip:
                used_aliases.add(source_clip)
            curve = curves.get(resolved_clip)
            if curve is None:
                raise PromotionError(
                    f"intake-only clip is absent from the reviewed WModel: {source_clip}"
                )
            if source["mappingBasis"] != "PROJECT_AUTHORED":
                raise PromotionError(
                    f"intake-only clip must remain PROJECT_AUTHORED: {occurrence_id}"
                )
            source_start_ms = _integer(
                source["sourceStartMs"], f"{occurrence_id}.sourceStartMs"
            )
            _integer(source["playMs"], f"{occurrence_id}.playMs")
            _number(source["playRate"], f"{occurrence_id}.playRate", 0.000001)
            if not isinstance(source["repeatUntilStageEnd"], bool):
                raise PromotionError(
                    f"{occurrence_id}.repeatUntilStageEnd must be Boolean"
                )
            native_duration_ms, _ticks_per_second, _keys = curve
            if source_start_ms >= native_duration_ms:
                raise PromotionError(
                    f"intake-only sourceStartMs escapes native clip: {occurrence_id}"
                )

    gameplay = (
        _read_json(repo_root / GAMEPLAY_REL)
        if gameplay_document is None
        else copy.deepcopy(gameplay_document)
    )
    presentation = (
        _read_json(repo_root / PRESENTATION_REL)
        if presentation_document is None
        else copy.deepcopy(presentation_document)
    )
    retired_collisions = pattern_ids & set(gameplay.get("retiredPatternIds", []))
    if retired_collisions:
        raise PromotionError(
            f"retired patterns cannot be promoted: {sorted(retired_collisions)}"
        )

    current_gameplay_ids = {row.get("patternId") for row in gameplay.get("patterns", [])}
    current_presentation_ids = {
        row.get("patternId") for row in presentation.get("patterns", [])
    }
    live_collisions = pattern_ids & (
        current_gameplay_ids ^ current_presentation_ids
    )
    if live_collisions:
        raise PromotionError(f"split source contains partial promotion rows: {sorted(live_collisions)}")

    existing_manual = gameplay.get("decisionModel", {}).get("manualAuditions", [])
    existing_manual_by_id = {
        row.get("patternId"): row for row in existing_manual if isinstance(row, dict)
    }
    existing_gameplay_by_id = {
        row.get("patternId"): row
        for row in gameplay.get("patterns", [])
        if isinstance(row, dict) and row.get("patternId") in pattern_ids
    }
    existing_presentation_by_id = {
        row.get("patternId"): row
        for row in presentation.get("patterns", [])
        if isinstance(row, dict) and row.get("patternId") in pattern_ids
    }
    for promotion in promotions:
        pattern_id = promotion["patternId"]
        if pattern_id in current_gameplay_ids:
            owner = existing_manual_by_id.get(pattern_id)
            if owner is None or owner.get("sourceChainId") != promotion["sourceChainId"]:
                raise PromotionError(f"pattern ID collides with a non-matching owner: {pattern_id}")

    gameplay["patterns"] = [
        row for row in gameplay["patterns"] if row.get("patternId") not in pattern_ids
    ]
    presentation["patterns"] = [
        row for row in presentation["patterns"] if row.get("patternId") not in pattern_ids
    ]
    if "decisionModel" not in gameplay or not isinstance(gameplay["decisionModel"], dict):
        raise PromotionError("gameplay decisionModel is missing")
    gameplay["decisionModel"]["manualAuditions"] = [
        row for row in existing_manual if row.get("patternId") not in pattern_ids
    ]

    receipt_patterns: list[dict[str, Any]] = []
    seen_source_occurrences: set[str] = set()
    seen_target_actions: set[str] = set()
    seen_target_occurrences: set[str] = set()
    for promotion, chain in zip(promotions, promoted_chains):
        chain_id = promotion["sourceChainId"]
        _exact(
            chain,
            ("chainId", "targetPatternId", "targetStageId", "animation"),
            f"chain {chain_id}",
        )
        if chain["chainId"] != chain_id or chain["targetPatternId"] or chain["targetStageId"]:
            raise PromotionError(f"debug chain target fields must remain empty: {chain_id}")
        animation = chain["animation"]
        _exact(animation, ("endPolicy", "repeatCount", "occurrences"), f"chain {chain_id}.animation")
        occurrences = animation["occurrences"]
        if (
            animation["endPolicy"] != "NATIVE_CLIP_LENGTHS"
            or not isinstance(occurrences, list)
            or not occurrences
            or _integer(animation["repeatCount"], f"chain {chain_id}.repeatCount", 1)
            != len(occurrences)
            or len(occurrences) > 64
        ):
            raise PromotionError(f"debug chain animation header is invalid: {chain_id}")

        stages: list[dict[str, Any]] = []
        source_action_ids: list[int] = []
        receipt_occurrences: list[dict[str, Any]] = []
        for occurrence_ordinal, source in enumerate(occurrences, start=1):
            _exact(
                source,
                (
                    "clipOccurrenceId",
                    "clip",
                    "mappingBasis",
                    "sourceStartMs",
                    "playMs",
                    "playRate",
                    "repeatUntilStageEnd",
                ),
                f"chain {chain_id} occurrence[{occurrence_ordinal}]",
            )
            source_occurrence_id = _stable(
                source["clipOccurrenceId"],
                f"chain {chain_id} source occurrence",
            )
            if source_occurrence_id in seen_source_occurrences:
                raise PromotionError(f"duplicate source occurrence: {source_occurrence_id}")
            seen_source_occurrences.add(source_occurrence_id)
            source_clip = _stable(source["clip"], f"chain {chain_id} source clip")
            resolved_clip = aliases.get(source_clip, source_clip)
            if source_clip in aliases:
                used_aliases.add(source_clip)
                if source_clip in curves:
                    raise PromotionError(f"unnecessary clip alias shadows a real clip: {source_clip}")
            curve = curves.get(resolved_clip)
            if curve is None:
                raise PromotionError(f"clip is absent from the reviewed WModel: {source_clip}")
            notified_source_action_id = clip_skills.get(resolved_clip)
            reviewed_source_action_id = promotion.get("sourceActionId")
            # One model clip can be reused by several recovered .clipseq actions.
            # For a CURRENT_CHAIN created from the Composition browser, the exact
            # reviewed action/sequence tuple is the primary presentation owner;
            # Valtan.animnotify remains only the legacy saved-chain fallback.
            if reviewed_source_action_id is not None:
                source_action_id = reviewed_source_action_id
            elif notified_source_action_id is not None:
                source_action_id = notified_source_action_id
            else:
                raise PromotionError(
                    f"clip has no Valtan.animnotify source action: {resolved_clip}"
                )
            if source_action_id not in source_action_ids:
                source_action_ids.append(source_action_id)

            source_start_ms = _integer(
                source["sourceStartMs"], f"{source_occurrence_id}.sourceStartMs"
            )
            authored_wall_ms = _integer(source["playMs"], f"{source_occurrence_id}.playMs")
            play_rate = _number(source["playRate"], f"{source_occurrence_id}.playRate", 0.000001)
            if not isinstance(source["repeatUntilStageEnd"], bool):
                raise PromotionError(f"{source_occurrence_id}.repeatUntilStageEnd must be Boolean")
            native_duration_ms, ticks_per_second, _keys = curve
            available_source_ms = native_duration_ms - source_start_ms
            if available_source_ms <= 0.0:
                raise PromotionError(f"sourceStartMs escapes native clip: {source_occurrence_id}")
            native_source_ms = _lround_positive(available_source_ms)
            native_wall_ms = _lround_positive(native_source_ms / play_rate)
            if native_source_ms <= 0 or native_wall_ms <= 0:
                raise PromotionError(f"resolved native duration is empty: {source_occurrence_id}")

            if authored_wall_ms == 0:
                if source["repeatUntilStageEnd"]:
                    raise PromotionError(
                        f"native occurrence cannot request an unbounded loop: {source_occurrence_id}"
                    )
                duration_ms = native_wall_ms
                product_play_ms = native_source_ms
                end_policy = "EXACT"
                repeat_until_stage_end = False
                resolution = "NATIVE_WMODEL"
            elif source["repeatUntilStageEnd"] or authored_wall_ms > native_wall_ms:
                duration_ms = authored_wall_ms
                product_play_ms = 0
                end_policy = "LOOP_TO_STAGE_END"
                repeat_until_stage_end = True
                resolution = "EXPLICIT_WALL_LOOP"
            else:
                duration_ms = authored_wall_ms
                product_play_ms = _lround_positive(authored_wall_ms * play_rate)
                if product_play_ms <= 0 or product_play_ms > native_source_ms:
                    raise PromotionError(f"explicit source slice is invalid: {source_occurrence_id}")
                end_policy = "EXACT"
                repeat_until_stage_end = False
                resolution = "EXPLICIT_WALL_EXACT"

            stage_id = f"STEP_{occurrence_ordinal:02d}"
            action_id = f"valtan.sequence.{chain_id}.step-{occurrence_ordinal:02d}"
            target_occurrence_id = f"{action_id}.clip-01"
            _stable(action_id, f"chain {chain_id} actionId")
            _stable(target_occurrence_id, f"chain {chain_id} target occurrence")
            if action_id in seen_target_actions or target_occurrence_id in seen_target_occurrences:
                raise PromotionError(f"generated identity collided: {action_id}")
            seen_target_actions.add(action_id)
            seen_target_occurrences.add(target_occurrence_id)
            target_occurrence = {
                "clipOccurrenceId": target_occurrence_id,
                "clip": resolved_clip,
                "mappingBasis": "PROJECT_AUTHORED",
                "sourceStartMs": source_start_ms,
                "playMs": product_play_ms,
                "playRate": play_rate,
                "repeatUntilStageEnd": repeat_until_stage_end,
            }
            stages.append(
                {
                    "stageId": stage_id,
                    "actionId": action_id,
                    "durationMs": duration_ms,
                    "endPolicy": end_policy,
                    "occurrence": target_occurrence,
                }
            )
            receipt_occurrences.append(
                {
                    "sourceClipOccurrenceId": source_occurrence_id,
                    "targetStageId": stage_id,
                    "targetActionId": action_id,
                    "targetClipOccurrenceId": target_occurrence_id,
                    "sourceClip": source_clip,
                    "resolvedClip": resolved_clip,
                    "aliasApplied": source_clip != resolved_clip,
                    "sourceActionId": source_action_id,
                    "sourceStartMs": source_start_ms,
                    "authoredWallMs": authored_wall_ms,
                    "playRate": play_rate,
                    "nativeDurationTicks": round(
                        native_duration_ms * ticks_per_second / 1000.0, 6
                    ),
                    "ticksPerSecond": ticks_per_second,
                    "nativeSourceMs": native_source_ms,
                    "stageDurationMs": duration_ms,
                    "productPlayMs": product_play_ms,
                    "endPolicy": end_policy,
                    "resolution": resolution,
                }
            )

        gameplay_pattern = _manual_gameplay_pattern(
            promotion, source_action_ids, stages
        )
        presentation_pattern = _manual_presentation_pattern(
            promotion, source_action_ids, stages
        )
        if promotion["patternId"] == "VALTAN_TRASH":
            gameplay_pattern, presentation_pattern = _expand_trash_capture_promotion(
                gameplay_pattern,
                presentation_pattern,
                existing_gameplay_by_id.get(promotion["patternId"]),
                existing_presentation_by_id.get(promotion["patternId"]),
            )
        elif promotion["patternId"] in (
            "VALTAN_TRASH_CATCH_IF", "VALTAN_TRASH_CATCH_SUCCESS", "VALTAN_TRASH_CATCH_FAIL"
        ):
            from author_valtan_phase_two_mechanics import author_trash_capture_flow
            shared_gameplay = copy.deepcopy(next(
                row for row in gameplay["patterns"] if row["patternId"] == "VALTAN_TRASH"))
            shared_presentation = copy.deepcopy(next(
                row for row in presentation["patterns"] if row["patternId"] == "VALTAN_TRASH"))
            author_trash_capture_flow(
                {"patterns": [shared_gameplay, gameplay_pattern]},
                {"patterns": [shared_presentation, presentation_pattern]},
                audition_pattern_ids=(promotion["patternId"],),
            )
        existing_gameplay_pattern = existing_gameplay_by_id.get(
            promotion["patternId"]
        )
        existing_presentation_pattern = existing_presentation_by_id.get(
            promotion["patternId"]
        )
        if (
            promotion["patternId"] == "VALTAN_COUNTER"
            and existing_gameplay_pattern is not None
            and existing_presentation_pattern is not None
            and "VALTAN_COUNTER_GROGGY" in current_gameplay_ids
            and "VALTAN_COUNTER_GROGGY" in current_presentation_ids
        ):
            # STEP_04 was promoted into its own reusable outcome Pattern.  The
            # animation-chain receipt continues to prove the reviewed four-step
            # source closure, while canonical gameplay deliberately owns the
            # parent/result split and its cross-pattern branch identity.
            gameplay_pattern = copy.deepcopy(existing_gameplay_pattern)
            presentation_pattern = copy.deepcopy(existing_presentation_pattern)
        else:
            preserve_sequence_append = bool(_typed_appended_sequence_sources(
                presentation_pattern,
                existing_presentation_pattern,
            ))
            gameplay_pattern = _preserve_manual_gameplay_enrichment(
                gameplay_pattern,
                existing_gameplay_pattern,
                preserve_sequence_append=preserve_sequence_append,
            )
            presentation_pattern = _preserve_manual_presentation_enrichment(
                presentation_pattern,
                existing_presentation_pattern,
            )
        gameplay["patterns"].append(gameplay_pattern)
        presentation["patterns"].append(presentation_pattern)
        gameplay["decisionModel"]["manualAuditions"].append(
            {
                "patternId": promotion["patternId"],
                "sourceChainId": chain_id,
                "authoringPhase": promotion["authoringPhase"],
                "admissionState": promotion["admissionState"],
            }
        )
        receipt_patterns.append(
            {
                "sourceChainId": chain_id,
                "patternId": promotion["patternId"],
                "authoringPhase": promotion["authoringPhase"],
                "admissionState": promotion["admissionState"],
                "sourceActionIds": source_action_ids,
                "stageCount": len(stages),
                "occurrences": receipt_occurrences,
            }
        )

    if used_aliases != set(aliases):
        raise PromotionError(
            f"clip alias coverage drift: used={sorted(used_aliases)} declared={sorted(aliases)}"
        )
    reviewed_pattern_count, reviewed_stage_count = _reviewed_closure_counts(
        promotions, promoted_chains
    )
    generated_stage_count = sum(row["stageCount"] for row in receipt_patterns)
    if (
        len(receipt_patterns) != reviewed_pattern_count
        or generated_stage_count != reviewed_stage_count
    ):
        raise PromotionError(
            "generated promotion closure left the reviewed manifest/debug closure"
        )
    receipt = {
        "schema": "lostark.valtan-animation-chain-promotion-receipt",
        "formatVersion": 1,
        "bossArchetypeId": manifest["bossArchetypeId"],
        "encounterId": manifest["encounterId"],
        "resolverVersion": 1,
        "roundingPolicy": "POSITIVE_HALF_AWAY_FROM_ZERO",
        "sourceDocument": copy.deepcopy(manifest["sourceDocument"]),
        "presentationProfile": copy.deepcopy(manifest["presentationProfile"]),
        "patternCount": reviewed_pattern_count,
        "stageCount": reviewed_stage_count,
        "patterns": receipt_patterns,
    }
    return gameplay, presentation, receipt


def _load_v2_pipeline(repo_root: Path) -> Any:
    root_text = str(repo_root)
    if root_text not in sys.path:
        sys.path.insert(0, root_text)
    try:
        from Tools.ValtanPipeline import valtan_tuning_pipeline as pipeline
    except ImportError as exc:
        raise PromotionError(f"cannot import Valtan V2 pipeline: {exc}") from exc
    return pipeline


def _product_projection_relatives(repo_root: Path) -> tuple[str, ...]:
    pipeline = _load_v2_pipeline(repo_root)
    return (
        pipeline.ENCOUNTER_REL,
        pipeline.BINDINGS_REL,
        pipeline.CUES_REL,
        pipeline.ROTATIONS_REL,
        pipeline.COMBAT_PRODUCT_REL,
        pipeline.WORLD_PRODUCT_REL,
        pipeline.PROVENANCE_REL,
    )


def _transaction_projection_relatives(repo_root: Path) -> tuple[str, ...]:
    """Every derived document committed with a split authoring generation."""

    return (*_product_projection_relatives(repo_root), ROOT_MOTION_REL)


def _project_candidate_root_motion(
    repo_root: Path,
    encounter: dict[str, Any],
    bindings: dict[str, Any],
) -> str:
    root_text = str(repo_root)
    if root_text not in sys.path:
        sys.path.insert(0, root_text)
    try:
        from Tools.ValtanActionExtractor import build_valtan_rootmotion

        document, _notes = build_valtan_rootmotion.build(
            repo_root,
            encounter_document=encounter,
            bindings_document=bindings,
        )
    except (ImportError, OSError, KeyError, TypeError, ValueError) as exc:
        raise PromotionError(
            f"cannot project candidate Valtan root motion: {exc}"
        ) from exc
    return _json_text(document)


def validate_and_project(
    repo_root: Path,
    gameplay: dict[str, Any],
    presentation: dict[str, Any],
    *,
    debug_document: dict[str, Any] | None = None,
    promotion_manifest: dict[str, Any] | None = None,
) -> dict[str, str]:
    pipeline = _load_v2_pipeline(repo_root)

    try:
        docs = pipeline.load_pipeline_documents(repo_root)
        docs[pipeline.GAMEPLAY_AUTHORING_REL] = gameplay
        docs[pipeline.PRESENTATION_AUTHORING_REL] = presentation
        pipeline.validate_valtan_native_animation_source(
            repo_root, presentation
        )
        joined = pipeline.join_v2_authoring(
            gameplay,
            presentation,
            docs[pipeline.WORLD_SET_REL],
            docs[pipeline.COMBAT_AUTHORING_REL],
        )
        pipeline.validate_manual_audition_animation_lineage(
            joined,
            debug_document
            if debug_document is not None
            else pipeline.read_json(
                pipeline.repo_path(repo_root, pipeline.DEBUG_PRESENTATION_REL)
            ),
            promotion_manifest
            if promotion_manifest is not None
            else pipeline.read_json(
                pipeline.repo_path(
                    repo_root, pipeline.ANIMATION_PROMOTION_MANIFEST_REL
                )
            ),
            repository_root=repo_root,
        )
        pipeline.validate_legacy_manifest(
            docs[pipeline.LEGACY_REL],
            {row["patternId"] for row in joined["patterns"]},
        )
        outputs = pipeline.project_v2_products(repo_root, docs, joined)
        balance_outputs = pipeline.project_balance_products(
            repo_root,
            docs[pipeline.BOSS_PROFILES_REL],
            docs[pipeline.DAMAGE_REL],
        )
        outputs[pipeline.PROVENANCE_REL] = pipeline.project_provenance_receipt(
            repo_root, {**outputs, **balance_outputs}
        )
        outputs[ROOT_MOTION_REL] = _project_candidate_root_motion(
            repo_root,
            json.loads(outputs[pipeline.ENCOUNTER_REL]),
            json.loads(outputs[pipeline.BINDINGS_REL]),
        )
    except (KeyError, TypeError, ValueError, pipeline.PipelineError) as exc:
        raise PromotionError(f"promoted split/Product validation failed: {exc}") from exc
    for relative, text in outputs.items():
        try:
            json.loads(text, object_pairs_hook=_reject_duplicate_pairs)
        except (json.JSONDecodeError, PromotionError) as exc:
            raise PromotionError(f"projected Product is not strict JSON: {relative}: {exc}") from exc
    return outputs


def _validate_pattern_sound_dependencies_against_candidate_products(
    repo_root: Path,
    outputs: Mapping[str, str],
    *,
    sound_source_bytes: bytes | None = None,
) -> None:
    """Join the physical Sound owner to the exact candidate Product closure.

    The Workbench preflight is useful feedback, but it runs before the
    canonical writer admission is acquired. Pattern Sound is a separate typed
    source owner, so its physical document must be read again while the
    exclusive Valtan writer lock is held and before any split source/Product
    byte is replaced. This prevents a Sound save that completed after the UI
    preflight from becoming dangling when a Pattern edit removes or retargets
    an animation occurrence.

    Encounter and animation binding documents are generated Products here,
    but only as the candidate projection of their real owners. They remain
    read-only and are never accepted as authoring input.
    """

    pipeline = _load_v2_pipeline(repo_root)

    def candidate_product(relative: str, context: str) -> dict[str, Any]:
        text = outputs.get(relative)
        if not isinstance(text, str):
            raise PromotionError(
                f"Pattern Sound dependency admission is missing candidate {context}: "
                f"{relative}"
            )
        return _read_json_bytes(text.encode("utf-8"), repo_root / relative)

    sound_path = repo_root / PATTERN_SOUND_REL
    sound = (
        _read_json(sound_path)
        if sound_source_bytes is None
        else _read_json_bytes(sound_source_bytes, sound_path)
    )
    _exact(
        sound,
        ("schema", "formatVersion", "ownerArchetypeId", "cues"),
        "Valtan Pattern Sound source",
    )
    if (
        sound["schema"] != "lostark.valtan-pattern-sound-cues"
        or isinstance(sound["formatVersion"], bool)
        or not isinstance(sound["formatVersion"], int)
        or sound["formatVersion"] != 1
        or sound["ownerArchetypeId"] != "BOSS_VALTAN"
    ):
        raise PromotionError("Valtan Pattern Sound source header/version is invalid")
    cues = sound["cues"]
    if not isinstance(cues, list) or not cues or len(cues) > 1024:
        raise PromotionError(
            "Valtan Pattern Sound source cues must contain 1..1024 rows"
        )

    encounter = candidate_product(
        pipeline.ENCOUNTER_REL, "Encounter Product"
    )
    encounter_patterns = encounter.get("patterns")
    if not isinstance(encounter_patterns, list):
        raise PromotionError(
            "Pattern Sound dependency admission candidate Encounter has no patterns"
        )
    stages_by_pattern: dict[str, dict[str, dict[str, Any]]] = {}
    for pattern_ordinal, pattern in enumerate(encounter_patterns):
        if not isinstance(pattern, dict):
            raise PromotionError(
                "Pattern Sound dependency admission candidate Encounter pattern "
                f"{pattern_ordinal} is not an object"
            )
        pattern_id = _stable(
            pattern.get("patternId"),
            f"candidate Encounter patterns[{pattern_ordinal}].patternId",
        )
        stages = pattern.get("stages")
        if not isinstance(stages, list):
            raise PromotionError(
                f"candidate Encounter pattern {pattern_id} stages must be an array"
            )
        staged: dict[str, dict[str, Any]] = {}
        for stage_ordinal, stage in enumerate(stages):
            if not isinstance(stage, dict):
                raise PromotionError(
                    f"candidate Encounter pattern {pattern_id} stage "
                    f"{stage_ordinal} is not an object"
                )
            stage_id = _stable(
                stage.get("stageId"),
                f"candidate Encounter {pattern_id} stages[{stage_ordinal}].stageId",
            )
            _stable(
                stage.get("actionId"),
                f"candidate Encounter {pattern_id}/{stage_id}.actionId",
            )
            if stage_id in staged:
                raise PromotionError(
                    f"candidate Encounter has duplicate Stage ID: {pattern_id}/{stage_id}"
                )
            staged[stage_id] = stage
        if pattern_id in stages_by_pattern:
            raise PromotionError(
                f"candidate Encounter has duplicate Pattern ID: {pattern_id}"
            )
        stages_by_pattern[pattern_id] = staged

    animation_bindings = candidate_product(
        pipeline.BINDINGS_REL, "Animation Binding Product"
    )
    binding_rows = animation_bindings.get("bindings")
    if not isinstance(binding_rows, list):
        raise PromotionError(
            "Pattern Sound dependency admission candidate Animation Product has no bindings"
        )
    clips_by_action: dict[str, dict[str, dict[str, Any]]] = {}
    for binding_ordinal, binding in enumerate(binding_rows):
        if not isinstance(binding, dict):
            raise PromotionError(
                "Pattern Sound dependency admission candidate Animation binding "
                f"{binding_ordinal} is not an object"
            )
        action_id = _stable(
            binding.get("actionId"),
            f"candidate Animation bindings[{binding_ordinal}].actionId",
        )
        clips = binding.get("clips")
        if not isinstance(clips, list):
            raise PromotionError(
                f"candidate Animation binding {action_id} clips must be an array"
            )
        staged_clips: dict[str, dict[str, Any]] = {}
        for clip_ordinal, clip in enumerate(clips):
            if not isinstance(clip, dict):
                raise PromotionError(
                    f"candidate Animation binding {action_id} clip "
                    f"{clip_ordinal} is not an object"
                )
            clip_occurrence_id = _stable(
                clip.get("clipOccurrenceId"),
                f"candidate Animation {action_id} clips[{clip_ordinal}].clipOccurrenceId",
            )
            if clip_occurrence_id in staged_clips:
                raise PromotionError(
                    "candidate Animation binding has duplicate clip occurrence: "
                    f"{action_id}/{clip_occurrence_id}"
                )
            staged_clips[clip_occurrence_id] = clip
        if action_id in clips_by_action:
            raise PromotionError(
                f"candidate Animation Product has duplicate action binding: {action_id}"
            )
        clips_by_action[action_id] = staged_clips

    cue_fields = (
        "bindingId",
        "occurrenceId",
        "patternId",
        "stageId",
        "actionId",
        "clipOccurrenceId",
        "soundBank",
        "soundEvent",
        "repeatPolicy",
        "startMs",
    )
    binding_ids: set[str] = set()
    occurrence_ids: set[str] = set()
    action_clip_occurrence_tuples: set[tuple[str, str, str]] = set()
    for cue_ordinal, cue in enumerate(cues):
        context = f"Valtan Pattern Sound cues[{cue_ordinal}]"
        _exact(cue, cue_fields, context)
        binding_id = _stable(cue["bindingId"], f"{context}.bindingId")
        occurrence_id = _stable(cue["occurrenceId"], f"{context}.occurrenceId")
        pattern_id = _stable(cue["patternId"], f"{context}.patternId")
        stage_id = _stable(cue["stageId"], f"{context}.stageId")
        action_id = _stable(cue["actionId"], f"{context}.actionId")
        clip_occurrence_id = _stable(
            cue["clipOccurrenceId"], f"{context}.clipOccurrenceId"
        )
        _stable(cue["soundBank"], f"{context}.soundBank")
        _stable(cue["soundEvent"], f"{context}.soundEvent")
        repeat_policy = cue["repeatPolicy"]
        if not isinstance(repeat_policy, str) or repeat_policy not in {
            "once",
            "each_loop",
        }:
            raise PromotionError(
                f"{context}.repeatPolicy must be once or each_loop"
            )
        start_ms = _integer(cue["startMs"], f"{context}.startMs")
        if start_ms > 60000:
            raise PromotionError(f"{context}.startMs must be <= 60000")
        if binding_id in binding_ids:
            raise PromotionError(
                f"Valtan Pattern Sound source has duplicate bindingId: {binding_id}"
            )
        if occurrence_id in occurrence_ids:
            raise PromotionError(
                "Valtan Pattern Sound source has duplicate occurrenceId: "
                f"{occurrence_id}"
            )
        binding_ids.add(binding_id)
        occurrence_ids.add(occurrence_id)

        tuple_key = (action_id, clip_occurrence_id, occurrence_id)
        if tuple_key in action_clip_occurrence_tuples:
            raise PromotionError(
                "Valtan Pattern Sound source has duplicate action/clip/occurrence tuple: "
                f"{occurrence_id}"
            )
        action_clip_occurrence_tuples.add(tuple_key)

        pattern_stages = stages_by_pattern.get(pattern_id)
        if pattern_stages is None:
            raise PromotionError(
                "Valtan Pattern Sound dependency does not resolve candidate Pattern: "
                f"{occurrence_id} -> {pattern_id}"
            )
        stage = pattern_stages.get(stage_id)
        if stage is None:
            raise PromotionError(
                "Valtan Pattern Sound dependency does not resolve candidate Stage: "
                f"{occurrence_id} -> {pattern_id}/{stage_id}"
            )
        if stage.get("actionId") != action_id:
            raise PromotionError(
                "Valtan Pattern Sound dependency action does not match candidate Stage: "
                f"{occurrence_id} -> {pattern_id}/{stage_id}/{action_id}"
            )
        action_clips = clips_by_action.get(action_id)
        if action_clips is None:
            raise PromotionError(
                "Valtan Pattern Sound dependency does not resolve candidate action: "
                f"{occurrence_id} -> {action_id}"
            )
        clip = action_clips.get(clip_occurrence_id)
        if clip is None:
            raise PromotionError(
                "Valtan Pattern Sound dependency does not resolve candidate clip occurrence: "
                f"{occurrence_id} -> {action_id}/{clip_occurrence_id}"
            )
        if repeat_policy == "each_loop" and clip.get("loop") is not True:
            raise PromotionError(
                "Valtan Pattern Sound each_loop dependency targets a non-loop clip: "
                f"{occurrence_id} -> {action_id}/{clip_occurrence_id}"
            )

        source_start_ms = clip.get("sourceStartMs")
        play_ms = clip.get("playMs")
        if (
            isinstance(source_start_ms, bool)
            or not isinstance(source_start_ms, int)
            or source_start_ms < 0
            or isinstance(play_ms, bool)
            or not isinstance(play_ms, int)
            or play_ms < 0
        ):
            raise PromotionError(
                "candidate Animation clip has an invalid source window: "
                f"{action_id}/{clip_occurrence_id}"
            )
        if start_ms < source_start_ms or (
            play_ms != 0 and start_ms >= source_start_ms + play_ms
        ):
            raise PromotionError(
                "Valtan Pattern Sound dependency startMs is outside its candidate clip segment: "
                f"{occurrence_id} -> {action_id}/{clip_occurrence_id}"
            )


def _validate_effect_v2_bindings_v1_compatibility(
    repo_root: Path,
    outputs: Mapping[str, str],
    binding_source_bytes: bytes,
) -> None:
    """Validate one staged BOSS_VALTAN V2 binding owner without publishing it.

    The C++ editor validates its in-memory catalog before invoking Save.  The
    repository writer repeats the stable-ID and clock joins while holding the
    common writer lock so an external Effect/Pattern writer cannot turn that
    preflight into a dangling binding before the generation is committed.
    """

    pipeline = _load_v2_pipeline(repo_root)
    binding_path = repo_root / EFFECT_V2_BINDINGS_REL
    document = _read_json_bytes(binding_source_bytes, binding_path)
    _exact(
        document,
        ("schema", "formatVersion", "archetypeId", "bindings"),
        "BOSS_VALTAN Effect V2 bindings",
    )
    if (
        document["schema"] != "lostark.effect-v2-bindings"
        or document["formatVersion"] != 1
        or document["archetypeId"] != "BOSS_VALTAN"
    ):
        raise PromotionError("BOSS_VALTAN Effect V2 binding header is invalid")
    rows = document["bindings"]
    if not isinstance(rows, list) or len(rows) > 4096:
        raise PromotionError("BOSS_VALTAN Effect V2 bindings must be an array <= 4096")

    authored_ids: set[str] = set()
    authored_root = repo_root / "Data/Effects/V2/Authored"
    for path in sorted(authored_root.glob("*.effectv2.json")):
        authored = _read_json(path)
        effect_id = _stable(authored.get("effectId"), f"Effect V2 authored {path.name}")
        if effect_id in authored_ids:
            raise PromotionError(f"duplicate Effect V2 effectId: {effect_id}")
        authored_ids.add(effect_id)
    if not authored_ids:
        raise PromotionError("Effect V2 authored catalog is empty")

    group_children: dict[str, list[tuple[str, int]]] = {}
    group_root = repo_root / "Data/Effects/V2/Groups"
    for path in sorted(group_root.glob("*.effectv2group.json")):
        group = _read_json(path)
        group_id = _stable(group.get("groupId"), f"Effect V2 group {path.name}")
        if group_id in authored_ids or group_id in group_children:
            raise PromotionError(f"duplicate/colliding Effect V2 groupId: {group_id}")
        children = group.get("children")
        if not isinstance(children, list) or not children:
            raise PromotionError(f"Effect V2 group children are invalid: {group_id}")
        clocks: list[tuple[str, int]] = []
        for ordinal, child in enumerate(children):
            if not isinstance(child, dict):
                raise PromotionError(f"Effect V2 group child is invalid: {group_id}/{ordinal}")
            effect_id = _stable(
                child.get("effectId"), f"Effect V2 group {group_id} child {ordinal}"
            )
            if effect_id not in authored_ids:
                raise PromotionError(
                    f"Effect V2 group child has no authored effect: {group_id}/{effect_id}"
                )
            start_ms = _integer(
                child.get("startMs", 0),
                f"Effect V2 group {group_id} child {ordinal}.startMs",
            )
            if start_ms > 600000:
                raise PromotionError(f"Effect V2 group child clock exceeds 600000 ms: {group_id}")
            clocks.append((effect_id, start_ms))
        group_children[group_id] = clocks

    animation_text = outputs.get(pipeline.BINDINGS_REL)
    if not isinstance(animation_text, str):
        raise PromotionError("Effect V2 admission is missing the candidate Animation Product")
    animation = _read_json_bytes(
        animation_text.encode("utf-8"), repo_root / pipeline.BINDINGS_REL
    )
    action_ids = {
        binding.get("actionId")
        for binding in animation.get("bindings", [])
        if isinstance(binding, dict)
        and isinstance(binding.get("actionId"), str)
    }
    clip_ids = {
        clip.get("clip")
        for binding in animation.get("bindings", [])
        if isinstance(binding, dict)
        for clip in binding.get("clips", [])
        if isinstance(clip, dict) and isinstance(clip.get("clip"), str)
    }

    row_fields = {
        "startMs", "bone", "followBone", "rotation", "stopWithClip",
        "offset", "yawDegrees",
    }
    identities: set[tuple[Any, ...]] = set()
    validated: list[tuple[str | None, str | None, str | None, str | None, int]] = []
    for ordinal, row in enumerate(rows):
        context = f"BOSS_VALTAN Effect V2 bindings[{ordinal}]"
        if not isinstance(row, dict):
            raise PromotionError(f"{context} must be an object")
        effect_id = row.get("effectId")
        group_id = row.get("group")
        stage_id = row.get("stage")
        clip_id = row.get("clip")
        has_effect = isinstance(effect_id, str) and bool(effect_id)
        has_group = isinstance(group_id, str) and bool(group_id)
        has_stage = isinstance(stage_id, str) and bool(stage_id)
        has_clip = isinstance(clip_id, str) and bool(clip_id)
        if has_effect == has_group or has_stage == has_clip:
            raise PromotionError(
                f"{context} requires exactly one effectId/group and stage/clip"
            )
        expected_fields = set(row_fields)
        expected_fields.add("effectId" if has_effect else "group")
        expected_fields.add("stage" if has_stage else "clip")
        if set(row) != expected_fields:
            raise PromotionError(f"{context} fields are not exact")
        if has_effect:
            effect_id = _stable(effect_id, f"{context}.effectId")
            if effect_id not in authored_ids:
                raise PromotionError(f"{context} has no authored effect: {effect_id}")
        else:
            group_id = _stable(group_id, f"{context}.group")
            if group_id not in group_children:
                raise PromotionError(f"{context} has no group: {group_id}")
        if has_stage:
            stage_id = _stable(stage_id, f"{context}.stage")
            if stage_id not in action_ids:
                raise PromotionError(f"{context} has no candidate Stage action: {stage_id}")
        else:
            clip_id = _stable(clip_id, f"{context}.clip")
            if clip_id not in clip_ids:
                raise PromotionError(f"{context} has no candidate Animation clip: {clip_id}")
        start_ms = _integer(row.get("startMs"), f"{context}.startMs")
        if start_ms > 600000:
            raise PromotionError(f"{context}.startMs exceeds 600000")
        if not isinstance(row.get("bone"), str) or len(row["bone"]) > 160:
            raise PromotionError(f"{context}.bone is invalid")
        if not isinstance(row.get("followBone"), bool) or not isinstance(
            row.get("stopWithClip"), bool
        ):
            raise PromotionError(f"{context} boolean fields are invalid")
        if row.get("rotation") not in {"Bone", "TargetYaw", "World"}:
            raise PromotionError(f"{context}.rotation is invalid")
        offset = row.get("offset")
        if not isinstance(offset, list) or len(offset) != 3 or any(
            isinstance(value, bool)
            or not isinstance(value, (int, float))
            or not math.isfinite(float(value))
            for value in offset
        ):
            raise PromotionError(f"{context}.offset is invalid")
        yaw = row.get("yawDegrees")
        if (
            isinstance(yaw, bool)
            or not isinstance(yaw, (int, float))
            or not math.isfinite(float(yaw))
        ):
            raise PromotionError(f"{context}.yawDegrees is invalid")
        identity = (effect_id, group_id, stage_id, clip_id, start_ms, row["bone"])
        if identity in identities:
            raise PromotionError(f"duplicate Effect V2 binding row: {identity}")
        identities.add(identity)
        validated.append((effect_id, group_id, stage_id, clip_id, start_ms))

    for effect_id, _group_id, stage_id, clip_id, start_ms in validated:
        if effect_id is None:
            continue
        for _leaf_id, group_id, group_stage, group_clip, group_start_ms in validated:
            if group_id is None or stage_id != group_stage or clip_id != group_clip:
                continue
            if any(
                child_effect_id == effect_id
                and group_start_ms + child_start_ms == start_ms
                for child_effect_id, child_start_ms in group_children[group_id]
            ):
                raise PromotionError(
                    "Effect V2 leaf overlaps the same group child at the same clock: "
                    f"{effect_id}/{group_id}@{start_ms}"
                )


def _load_effect_v2_binding_pipeline(repo_root: Path) -> Any:
    root_text = str(repo_root)
    if root_text not in sys.path:
        sys.path.insert(0, root_text)
    try:
        from Tools.EffectToolV2 import effect_v2_binding_pipeline as binding_pipeline
    except ImportError as exc:
        raise PromotionError(
            f"cannot import Effect V2 binding pipeline: {exc}"
        ) from exc
    return binding_pipeline


def _validate_effect_v2_bindings_against_candidate_products(
    repo_root: Path,
    outputs: Mapping[str, str],
    binding_source_bytes: bytes,
    gameplay_document: Mapping[str, Any],
) -> dict[str, Any]:
    """Validate the v2 owner against one immutable candidate Product closure."""

    binding_pipeline = _load_effect_v2_binding_pipeline(repo_root)
    pipeline = _load_v2_pipeline(repo_root)
    binding_path = repo_root / EFFECT_V2_BINDINGS_REL
    document = _read_json_bytes(binding_source_bytes, binding_path)
    if document.get("formatVersion") != binding_pipeline.BINDING_FORMAT_VERSION:
        raise PromotionError(
            "BOSS_VALTAN Effect V2 Composition Save requires formatVersion 2; "
            "run and review the explicit legacy migration before Save"
        )
    animation_text = outputs.get(pipeline.BINDINGS_REL)
    if not isinstance(animation_text, str):
        raise PromotionError(
            "Effect V2 admission is missing the candidate Animation Product"
        )
    animation = _read_json_bytes(
        animation_text.encode("utf-8"), repo_root / pipeline.BINDINGS_REL
    )
    try:
        return binding_pipeline.validate_binding_document(
            repo_root,
            document,
            gameplay_document,
            animation,
            _read_json(repo_root / pipeline.LEGACY_REL),
        )
    except binding_pipeline.BindingContractError as exc:
        raise PromotionError(str(exc)) from exc


def _validate_create_request(request: dict[str, Any]) -> dict[str, Any]:
    _exact(
        request,
        (
            "schema",
            "formatVersion",
            "expectedSourceSha256",
            "patternId",
            "displayName",
            "authoringPhase",
            "targetPolicy",
            "aimPolicy",
            "intakeChain",
        ),
        "Create New Pattern request",
    )
    if (
        request["schema"] != CREATE_REQUEST_SCHEMA
        or isinstance(request["formatVersion"], bool)
        or not isinstance(request["formatVersion"], int)
        or request["formatVersion"] != CREATE_REQUEST_FORMAT_VERSION
    ):
        raise PromotionError("Create New Pattern request header/version mismatch")
    if not isinstance(request["expectedSourceSha256"], str) or not SHA256.fullmatch(
        request["expectedSourceSha256"]
    ):
        raise PromotionError("expectedSourceSha256 must be one lowercase SHA-256")
    pattern_id = _stable(request["patternId"], "Create New Pattern patternId")
    display_name = request["displayName"]
    if (
        not isinstance(display_name, str)
        or not display_name
        or display_name != display_name.strip()
        or len(display_name) > 64
        or any(ord(character) < 0x20 for character in display_name)
    ):
        raise PromotionError(
            "Create New Pattern displayName must be 1..64 trimmed Unicode characters"
        )
    authoring_phase = _integer(
        request["authoringPhase"], "Create New Pattern authoringPhase", 1
    )
    if authoring_phase > 3:
        raise PromotionError("Create New Pattern authoringPhase must be 1..3")
    target_policy = request["targetPolicy"]
    aim_policy = request["aimPolicy"]
    if target_policy not in {
        "NONE",
        "LOCK_NEAREST_ON_START",
        "LOCK_RANDOM_ALIVE_ON_START",
    } or aim_policy not in {"NONE", "LOCK_FACING_ON_START"}:
        raise PromotionError("Create New Pattern target/aim policy is invalid")
    if (target_policy == "NONE") != (aim_policy == "NONE"):
        raise PromotionError(
            "Create New Pattern target and aim policies must be authored together"
        )

    selection = request["intakeChain"]
    if not isinstance(selection, dict):
        raise PromotionError("Create New Pattern intakeChain must be an object")
    selection_kind = selection.get("selectionKind")
    if selection_kind == "SAVED_INTAKE_CHAIN":
        _exact(
            selection,
            ("selectionKind", "sourceChainId"),
            "Create New Pattern saved intakeChain",
        )
        _stable(selection["sourceChainId"], "saved intake sourceChainId")
    elif selection_kind == "CURRENT_CHAIN":
        _required_with_optional(
            selection,
            ("selectionKind", "chain"),
            ("sourceActionId", "sourceSequenceIndex"),
            "Create New Pattern current intakeChain",
        )
        has_source_action = "sourceActionId" in selection
        has_source_sequence = "sourceSequenceIndex" in selection
        if has_source_action != has_source_sequence:
            raise PromotionError(
                "Create New Pattern exact source action and sequence must be authored together"
            )
        if has_source_action:
            _integer(
                selection["sourceActionId"],
                "Create New Pattern sourceActionId",
                1,
            )
            source_sequence_index = _integer(
                selection["sourceSequenceIndex"],
                "Create New Pattern sourceSequenceIndex",
                0,
            )
            if source_sequence_index > 4096:
                raise PromotionError(
                    "Create New Pattern sourceSequenceIndex must be <= 4096"
                )
        chain = selection["chain"]
        if not isinstance(chain, dict):
            raise PromotionError("Create New Pattern current chain must be an object")
        _stable(chain.get("chainId"), "current chainId")
    else:
        raise PromotionError(
            "intakeChain.selectionKind must be SAVED_INTAKE_CHAIN or CURRENT_CHAIN"
        )
    normalized = copy.deepcopy(request)
    normalized["patternId"] = pattern_id
    normalized["displayName"] = display_name
    normalized["authoringPhase"] = authoring_phase
    return normalized


def _stage_create_pattern_documents(
    manifest: dict[str, Any],
    debug: dict[str, Any],
    gameplay: dict[str, Any],
    presentation: dict[str, Any],
    request: dict[str, Any],
) -> tuple[dict[str, Any], dict[str, Any], str]:
    manifest = copy.deepcopy(manifest)
    debug = copy.deepcopy(debug)
    pattern_id = request["patternId"]
    manifest_patterns = manifest.get("patterns")
    intake_only = manifest.get("animationIntakeOnly")
    debug_chains = debug.get("chains")
    if not all(isinstance(rows, list) for rows in (
        manifest_patterns, intake_only, debug_chains
    )):
        raise PromotionError("animation chain manifest/debug arrays are unavailable")

    existing_pattern_ids = {
        row.get("patternId")
        for rows in (
            manifest_patterns,
            gameplay.get("patterns", []),
            presentation.get("patterns", []),
        )
        for row in rows
        if isinstance(row, dict)
    }
    if pattern_id in existing_pattern_ids:
        raise PromotionError(f"Create New Pattern duplicate patternId: {pattern_id}")
    if pattern_id in set(gameplay.get("retiredPatternIds", [])):
        raise PromotionError(f"Create New Pattern patternId is retired: {pattern_id}")

    selection = request["intakeChain"]
    selection_kind = selection["selectionKind"]
    promoted_chain_ids = {
        row.get("sourceChainId")
        for row in manifest_patterns
        if isinstance(row, dict)
    }
    debug_chain_ids = [
        row.get("chainId") for row in debug_chains if isinstance(row, dict)
    ]
    if len(debug_chain_ids) != len(debug_chains) or len(set(debug_chain_ids)) != len(
        debug_chain_ids
    ):
        raise PromotionError("debug chain IDs are missing or duplicated")

    if selection_kind == "SAVED_INTAKE_CHAIN":
        source_chain_id = selection["sourceChainId"]
        matches = [
            (index, row)
            for index, row in enumerate(intake_only)
            if isinstance(row, dict) and row.get("sourceChainId") == source_chain_id
        ]
        if len(matches) != 1 or source_chain_id not in debug_chain_ids:
            raise PromotionError(
                f"saved intake chain is missing or ambiguous: {source_chain_id}"
            )
        if source_chain_id in promoted_chain_ids:
            raise PromotionError(f"intake chain is already promoted: {source_chain_id}")
        intake_index, _declaration = matches[0]
        del intake_only[intake_index]
        debug_index = debug_chain_ids.index(source_chain_id)
        insert_index = sum(
            1
            for chain_id in debug_chain_ids[:debug_index]
            if chain_id in promoted_chain_ids
        )
    else:
        chain = copy.deepcopy(selection["chain"])
        source_chain_id = chain["chainId"]
        declared_intake_ids = {
            row.get("sourceChainId")
            for row in intake_only
            if isinstance(row, dict)
        }
        if source_chain_id in promoted_chain_ids or source_chain_id in declared_intake_ids:
            raise PromotionError(
                f"Create New Pattern chain is already declared: {source_chain_id}"
            )
        if source_chain_id in debug_chain_ids:
            raise PromotionError(
                f"Create New Pattern current chain already exists: {source_chain_id}"
            )
        debug_chains.append(chain)
        debug_chain_ids.append(source_chain_id)
        insert_index = len(manifest_patterns)

    promotion = {
        "sourceChainId": source_chain_id,
        "patternId": pattern_id,
        "displayName": request["displayName"],
        "authoringPhase": request["authoringPhase"],
        "admissionState": "MANUAL_SERVER_AUDITION",
        "targetPolicy": request["targetPolicy"],
        "aimPolicy": request["aimPolicy"],
    }
    if selection_kind == "CURRENT_CHAIN" and "sourceActionId" in selection:
        promotion["sourceActionId"] = selection["sourceActionId"]
        promotion["sourceSequenceIndex"] = selection["sourceSequenceIndex"]
    manifest_patterns.insert(insert_index, promotion)
    debug_payload = _json_text(debug).encode("utf-8")
    manifest["sourceDocument"]["sha256"] = _sha256_bytes(debug_payload)
    return manifest, debug, source_chain_id


def prepare_create_pattern_transaction(
    repo_root: Path,
    request: dict[str, Any],
) -> tuple[dict[Path, bytes], dict[Path, bytes | None], dict[str, Any]]:
    repo_root = repo_root.resolve()
    request = _validate_create_request(request)
    core_paths = tuple(
        repo_root / relative
        for relative in (
            DEBUG_REL,
            MANIFEST_REL,
            GAMEPLAY_REL,
            PRESENTATION_REL,
            RECEIPT_REL,
        )
    )
    expected_baselines = {
        path: _read_bytes_or_none(path) for path in core_paths
    }
    debug = _read_json_bytes(expected_baselines[repo_root / DEBUG_REL], repo_root / DEBUG_REL)
    manifest = _read_json_bytes(
        expected_baselines[repo_root / MANIFEST_REL], repo_root / MANIFEST_REL
    )
    gameplay = _read_json_bytes(
        expected_baselines[repo_root / GAMEPLAY_REL], repo_root / GAMEPLAY_REL
    )
    presentation = _read_json_bytes(
        expected_baselines[repo_root / PRESENTATION_REL], repo_root / PRESENTATION_REL
    )
    current_source_payload = expected_baselines[repo_root / DEBUG_REL]
    if current_source_payload is None:
        raise PromotionError("Create New Pattern debug intake source is missing")
    current_source_sha256 = _sha256_bytes(current_source_payload)
    if request["expectedSourceSha256"] != current_source_sha256:
        raise PromotionError(
            "Create New Pattern source drift: expectedSourceSha256 no longer matches debug intake"
        )
    _validate_header(manifest, debug)
    _validate_manifest_paths(
        repo_root, manifest, source_sha256=current_source_sha256
    )

    staged_manifest, staged_debug, source_chain_id = _stage_create_pattern_documents(
        manifest, debug, gameplay, presentation, request
    )
    staged_gameplay, staged_presentation, receipt = build_candidates(
        repo_root,
        manifest_document=staged_manifest,
        debug_document=staged_debug,
        gameplay_document=gameplay,
        presentation_document=presentation,
    )
    product_relatives = _transaction_projection_relatives(repo_root)
    if len(product_relatives) != len(set(product_relatives)):
        raise PromotionError("Product projection target closure contains duplicates")
    for relative in product_relatives:
        path = repo_root / relative
        if path in expected_baselines:
            raise PromotionError(
                f"Product projection collides with an authoring target: {relative}"
            )
        expected_baselines[path] = _read_bytes_or_none(path)
    outputs = validate_and_project(
        repo_root,
        staged_gameplay,
        staged_presentation,
        debug_document=staged_debug,
        promotion_manifest=staged_manifest,
    )
    if set(outputs) != set(product_relatives):
        raise PromotionError(
            "Product projection target closure changed during Create New Pattern"
        )
    targets: dict[Path, bytes] = {
        repo_root / DEBUG_REL: _json_text(staged_debug).encode("utf-8"),
        repo_root / MANIFEST_REL: _json_text(staged_manifest).encode("utf-8"),
        repo_root / GAMEPLAY_REL: _json_text(staged_gameplay).encode("utf-8"),
        repo_root / PRESENTATION_REL: _json_text(staged_presentation).encode("utf-8"),
        repo_root / RECEIPT_REL: _json_text(receipt).encode("utf-8"),
    }
    for relative, text in outputs.items():
        path = repo_root / relative
        targets[path] = text.encode("utf-8")

    result = {
        "patternId": request["patternId"],
        "sourceChainId": source_chain_id,
        "admissionState": "MANUAL_SERVER_AUDITION",
        "selectionMode": "AUDITION_ONLY",
        "sourceSha256": staged_manifest["sourceDocument"]["sha256"],
        "patternCount": receipt["patternCount"],
        "stageCount": receipt["stageCount"],
        "projectedArtifactCount": len(outputs),
    }
    return targets, expected_baselines, result


def _infer_repository_root(targets: Mapping[Path, bytes]) -> Path:
    roots: set[Path] = set()
    for target in targets:
        resolved = target.resolve()
        cursor = resolved.parent
        while cursor != cursor.parent and cursor.name.casefold() != "data":
            cursor = cursor.parent
        if cursor.name.casefold() != "data":
            raise PromotionError(f"transaction target is outside the Data tree: {target}")
        roots.add(cursor.parent)
    if len(roots) != 1:
        raise PromotionError("transaction targets do not share one repository root")
    return next(iter(roots))


def _validate_transaction_targets(
    repository_root: Path,
    targets: Mapping[Path, bytes],
) -> list[Path]:
    if not targets:
        raise PromotionError("transaction target closure is empty")
    repository_root = repository_root.resolve()
    resolved: list[Path] = []
    for target, payload in targets.items():
        if not isinstance(target, Path) or not isinstance(payload, bytes):
            raise PromotionError("transaction targets must map Paths to bytes")
        candidate = target.resolve()
        try:
            relative = candidate.relative_to(repository_root)
        except ValueError as exc:
            raise PromotionError(
                f"transaction target escapes repository root: {target}"
            ) from exc
        if not relative.parts or relative.parts[0].casefold() != "data":
            raise PromotionError(f"transaction target is not a Data owner/Product: {target}")
        resolved.append(candidate)
    if len(resolved) != len(set(resolved)):
        raise PromotionError("transaction target closure aliases one physical path")
    return resolved


def _write_durable(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("xb") as output:
        output.write(payload)
        output.flush()
        os.fsync(output.fileno())


def _write_generation_state(path: Path, value: dict[str, Any]) -> None:
    payload = _json_text(value).encode("utf-8")
    temporary = path.with_suffix(".next")
    temporary.unlink(missing_ok=True)
    _write_durable(temporary, payload)
    os.replace(temporary, path)


def _transaction_artifact_path(
    root: Path,
    ordinal: int,
    target: Path,
    suffix: str,
) -> Path:
    identity = hashlib.sha256(str(target.resolve()).encode()).hexdigest()
    return root / f"{ordinal:03d}-{identity}.{suffix}"


def _active_generation_path(repository_root: Path) -> Path:
    return repository_root.resolve() / TRANSACTION_ACTIVE_REL


def _write_active_generation(repository_root: Path, generation_id: str) -> None:
    if re.fullmatch(r"[0-9a-f]{32}", generation_id) is None:
        raise PromotionError("transaction generation ID is invalid")
    _write_generation_state(
        _active_generation_path(repository_root),
        {
            "schema": TRANSACTION_ACTIVE_SCHEMA,
            "formatVersion": TRANSACTION_ACTIVE_FORMAT_VERSION,
            "generationId": generation_id,
            "generationState": (
                Path(TRANSACTION_GENERATION_ROOT_REL)
                / generation_id
                / "generation.json"
            ).as_posix(),
        },
    )


def _clear_active_generation(repository_root: Path) -> None:
    active = _active_generation_path(repository_root)
    active.unlink(missing_ok=True)
    active.with_suffix(".next").unlink(missing_ok=True)


def _transaction_target_rows(
    repository_root: Path,
    state: dict[str, Any],
) -> list[tuple[Path, bytes | None, Path | None, str]]:
    required = {
        "schema",
        "formatVersion",
        "generationId",
        "state",
        "targets",
    }
    if not isinstance(state, dict) or not required.issubset(state) or not set(state).issubset(
        required | {"failure"}
    ):
        raise PromotionError("active transaction generation properties are invalid")
    generation_id = state["generationId"]
    if (
        state["schema"] != TRANSACTION_GENERATION_SCHEMA
        or state["formatVersion"] != TRANSACTION_GENERATION_FORMAT_VERSION
        or not isinstance(generation_id, str)
        or re.fullmatch(r"[0-9a-f]{32}", generation_id) is None
        or state["state"] not in {"STAGING", "COMMITTING", "COMMITTED", "ROLLED_BACK"}
        or not isinstance(state["targets"], list)
        or not state["targets"]
    ):
        raise PromotionError("active transaction generation header/state is invalid")
    generation = repository_root / TRANSACTION_GENERATION_ROOT_REL / generation_id
    backups = generation / "baselines"
    rows: list[tuple[Path, bytes | None, Path | None, str]] = []
    seen: set[Path] = set()
    for ordinal, row in enumerate(state["targets"], start=1):
        if not isinstance(row, dict) or set(row) != {
            "path",
            "baselineSha256",
            "candidateSha256",
        }:
            raise PromotionError("active transaction target row is invalid")
        relative_text = row["path"]
        baseline_sha = row["baselineSha256"]
        candidate_sha = row["candidateSha256"]
        if (
            not isinstance(relative_text, str)
            or not relative_text
            or Path(relative_text).is_absolute()
            or (baseline_sha is not None and (
                not isinstance(baseline_sha, str) or SHA256.fullmatch(baseline_sha) is None
            ))
            or not isinstance(candidate_sha, str)
            or SHA256.fullmatch(candidate_sha) is None
        ):
            raise PromotionError("active transaction target identity/hash is invalid")
        target = (repository_root / relative_text).resolve()
        try:
            relative = target.relative_to(repository_root)
        except ValueError as exc:
            raise PromotionError("active transaction target escapes repository root") from exc
        if not relative.parts or relative.parts[0].casefold() != "data" or target in seen:
            raise PromotionError("active transaction target is outside/duplicated in Data")
        seen.add(target)
        backup: Path | None = None
        baseline: bytes | None = None
        if baseline_sha is not None:
            backup = _transaction_artifact_path(backups, ordinal, target, "baseline")
            baseline = _read_bytes_or_none(backup)
            if baseline is None or _sha256_bytes(baseline) != baseline_sha:
                raise PromotionError(f"active transaction baseline is missing/corrupt: {target}")
        rows.append((target, baseline, backup, candidate_sha))
    return rows


def _recover_incomplete_product_transaction(repository_root: Path) -> None:
    """Recover a writer that died between per-file replacements.

    The process-wide lock is already held by the caller. ``COMMITTING`` always
    rolls back from immutable generation baselines, while a fully sealed
    ``COMMITTED`` generation is retained after all target hashes are verified.
    This makes the next cooperating writer deterministic after process death.
    """

    repository_root = repository_root.resolve()
    active_path = _active_generation_path(repository_root)
    if not active_path.exists():
        return
    active = _read_json(active_path)
    if set(active) != {
        "schema",
        "formatVersion",
        "generationId",
        "generationState",
    }:
        raise PromotionError("active transaction pointer properties are invalid")
    generation_id = active["generationId"]
    expected_state = (
        Path(TRANSACTION_GENERATION_ROOT_REL) / generation_id / "generation.json"
    ).as_posix() if isinstance(generation_id, str) else ""
    if (
        active["schema"] != TRANSACTION_ACTIVE_SCHEMA
        or active["formatVersion"] != TRANSACTION_ACTIVE_FORMAT_VERSION
        or not isinstance(generation_id, str)
        or re.fullmatch(r"[0-9a-f]{32}", generation_id) is None
        or active["generationState"] != expected_state
    ):
        raise PromotionError("active transaction pointer header/path is invalid")
    generation = repository_root / TRANSACTION_GENERATION_ROOT_REL / generation_id
    state_path = generation / "generation.json"
    state = _read_json(state_path)
    if state.get("generationId") != generation_id:
        raise PromotionError("active transaction pointer/generation mismatch")
    rows = _transaction_target_rows(repository_root, state)
    transaction_state = state["state"]
    if transaction_state == "COMMITTING":
        failures: list[str] = []
        for target, baseline, backup, _candidate_sha in reversed(rows):
            try:
                _restore_exact_baseline(
                    target,
                    baseline,
                    backup,
                    inject_primary_replace_failure=False,
                )
            except BaseException as exc:
                failures.append(f"{target}: {exc}")
        if failures:
            raise PromotionError(
                f"incomplete Product transaction recovery failed: {failures}; "
                f"generation retained at {generation}"
            )
    elif transaction_state in {"STAGING", "ROLLED_BACK"}:
        for target, baseline, _backup, _candidate_sha in rows:
            if _read_bytes_or_none(target) != baseline:
                raise PromotionError(
                    f"inactive transaction baseline changed externally: {target}"
                )
    else:
        for target, _baseline, _backup, candidate_sha in rows:
            payload = _read_bytes_or_none(target)
            if payload is None or _sha256_bytes(payload) != candidate_sha:
                raise PromotionError(
                    f"committed Product generation target is missing/corrupt: {target}"
                )
    _clear_active_generation(repository_root)
    shutil.rmtree(generation)


def _restore_exact_baseline(
    path: Path,
    baseline: bytes | None,
    backup: Path | None,
    *,
    inject_primary_replace_failure: bool,
) -> None:
    """Restore and verify one byte-exact owner after a commit fault.

    A failed primary atomic restore falls back to a durable direct rewrite from
    the immutable generation backup.  This second path is intentionally only a
    failure recovery path under the process-wide writer lock; normal commits
    never truncate a destination in place.
    """

    if baseline is None:
        path.unlink(missing_ok=True)
    else:
        if backup is None or _read_bytes_or_none(backup) != baseline:
            raise PromotionError(f"transaction backup is unavailable for {path}")
        restore = path.parent / f".{path.name}.restore-{uuid.uuid4().hex}.tmp"
        _write_durable(restore, baseline)
        try:
            if inject_primary_replace_failure:
                raise OSError("injected rollback replace failure")
            os.replace(restore, path)
        except OSError:
            restore.unlink(missing_ok=True)
            with path.open("wb") as output:
                output.write(baseline)
                output.flush()
                os.fsync(output.fileno())
        finally:
            restore.unlink(missing_ok=True)
    if _read_bytes_or_none(path) != baseline:
        raise PromotionError(f"byte-exact rollback verification failed for {path}")


def _atomic_commit_locked(
    repository_root: Path,
    targets: Mapping[Path, bytes],
    *,
    inject_failure_after: int | None,
    inject_replace_failure_at: int | None,
    inject_rollback_failure_at: int | None,
    inject_hard_crash_after: int | None,
    expected_baselines: Mapping[Path, bytes | None] | None,
) -> None:
    repository_root = repository_root.resolve()
    _recover_incomplete_product_transaction(repository_root)
    _validate_transaction_targets(repository_root, targets)
    if expected_baselines is None:
        baselines: dict[Path, bytes | None] = {
            path: _read_bytes_or_none(path) for path in targets
        }
    else:
        if set(expected_baselines) != set(targets):
            raise PromotionError("transaction baseline/target closure mismatch")
        baselines = dict(expected_baselines)
    for injection, label in (
        (inject_failure_after, "commit failure ordinal"),
        (inject_replace_failure_at, "replace failure ordinal"),
        (inject_rollback_failure_at, "rollback failure ordinal"),
        (inject_hard_crash_after, "hard-crash ordinal"),
    ):
        if injection is not None and injection < 1:
            raise PromotionError(f"{label} must be >= 1")

    generation_id = uuid.uuid4().hex
    generation = repository_root / TRANSACTION_GENERATION_ROOT_REL / generation_id
    candidate_root = generation / "candidates"
    backup_root = generation / "baselines"
    state_path = generation / "generation.json"
    staged: dict[Path, Path] = {}
    backups: dict[Path, Path | None] = {}
    ordered = sorted(targets, key=lambda value: str(value).casefold())
    state: dict[str, Any] = {
        "schema": TRANSACTION_GENERATION_SCHEMA,
        "formatVersion": TRANSACTION_GENERATION_FORMAT_VERSION,
        "generationId": generation_id,
        "state": "STAGING",
        "targets": [],
    }
    possibly_mutated: list[Path] = []
    cleanup_generation = False
    try:
        for ordinal, path in enumerate(ordered, start=1):
            path.parent.mkdir(parents=True, exist_ok=True)
            candidate = _transaction_artifact_path(
                candidate_root, ordinal, path, "candidate"
            )
            _write_durable(candidate, targets[path])
            staged[path] = candidate
            baseline = baselines[path]
            backup: Path | None = None
            if baseline is not None:
                backup = _transaction_artifact_path(
                    backup_root, ordinal, path, "baseline"
                )
                _write_durable(backup, baseline)
            backups[path] = backup
            state["targets"].append(
                {
                    "path": path.resolve().relative_to(repository_root).as_posix(),
                    "baselineSha256": None if baseline is None else _sha256_bytes(baseline),
                    "candidateSha256": _sha256_bytes(targets[path]),
                }
            )
        _write_generation_state(state_path, state)
        _write_active_generation(repository_root, generation_id)

        # CAS is evaluated only after every candidate and immutable rollback
        # image is durable.  A stale writer therefore cannot mutate any owner.
        for path, baseline in baselines.items():
            current = _read_bytes_or_none(path)
            if current != baseline:
                raise PromotionError(f"target changed during promotion: {path}")

        state["state"] = "COMMITTING"
        _write_generation_state(state_path, state)
        for ordinal, path in enumerate(ordered, start=1):
            possibly_mutated.append(path)
            if inject_replace_failure_at == ordinal:
                raise PromotionError("injected promotion replace failure")
            os.replace(staged[path], path)
            if _read_bytes_or_none(path) != targets[path]:
                raise PromotionError(f"committed candidate verification failed: {path}")
            if inject_hard_crash_after is not None and ordinal >= inject_hard_crash_after:
                os._exit(97)
            if inject_failure_after is not None and ordinal >= inject_failure_after:
                raise PromotionError("injected promotion commit failure")

        state["state"] = "COMMITTED"
        _write_generation_state(state_path, state)
        cleanup_generation = True
    except BaseException as exc:
        rollback_failures: list[str] = []
        for restore_ordinal, path in enumerate(reversed(possibly_mutated), start=1):
            try:
                _restore_exact_baseline(
                    path,
                    baselines[path],
                    backups[path],
                    inject_primary_replace_failure=(
                        inject_rollback_failure_at == restore_ordinal
                    ),
                )
            except BaseException as rollback_exc:
                rollback_failures.append(f"{path}: {rollback_exc}")
        if not rollback_failures:
            state["state"] = "ROLLED_BACK"
            state["failure"] = str(exc)
            try:
                _write_generation_state(state_path, state)
            except BaseException:
                # The immutable baselines have already been verified at their
                # owner paths.  A diagnostic journal failure cannot downgrade
                # that rollback result.
                pass
            cleanup_generation = True
        if rollback_failures:
            raise PromotionError(
                f"promotion failed ({exc}); byte-exact rollback failed: {rollback_failures}; "
                f"recovery generation retained at {generation}"
            ) from exc
        raise
    finally:
        if cleanup_generation:
            _clear_active_generation(repository_root)
            shutil.rmtree(generation, ignore_errors=True)


def _atomic_commit(
    targets: Mapping[Path, bytes],
    *,
    inject_failure_after: int | None = None,
    inject_replace_failure_at: int | None = None,
    inject_rollback_failure_at: int | None = None,
    inject_hard_crash_after: int | None = None,
    expected_baselines: Mapping[Path, bytes | None] | None = None,
    repository_root: Path | None = None,
    lock_already_held: bool = False,
    lock_timeout_seconds: float = 0.0,
) -> None:
    repository_root = (
        _infer_repository_root(targets)
        if repository_root is None
        else repository_root.resolve()
    )
    if lock_already_held:
        _atomic_commit_locked(
            repository_root,
            targets,
            inject_failure_after=inject_failure_after,
            inject_replace_failure_at=inject_replace_failure_at,
            inject_rollback_failure_at=inject_rollback_failure_at,
            inject_hard_crash_after=inject_hard_crash_after,
            expected_baselines=expected_baselines,
        )
        return
    with _exclusive_transaction_lock(
        repository_root,
        timeout_seconds=lock_timeout_seconds,
        operation="AtomicCommit",
    ):
        _atomic_commit_locked(
            repository_root,
            targets,
            inject_failure_after=inject_failure_after,
            inject_replace_failure_at=inject_replace_failure_at,
            inject_rollback_failure_at=inject_rollback_failure_at,
            inject_hard_crash_after=inject_hard_crash_after,
            expected_baselines=expected_baselines,
        )


def commit_projected_products(
    repo_root: Path,
    projected_product_root: Path,
    expected_source_manifest_id: str,
    *,
    lock_timeout_seconds: float = 0.0,
    external_lock_owner_pid: int | None = None,
    external_lock_owner_nonce: str | None = None,
) -> dict[str, Any]:
    """Admit one validated Valtan Product projection under the shared writer lock.

    Projection intentionally happens outside this function.  The projection's
    source manifest is compared again after acquiring the same lock used by
    Create New Pattern, then every changed Product is committed through the
    durable generation journal.  A stale projector therefore cannot overwrite
    a Create transaction that won the lock first.
    """

    repo_root = repo_root.resolve()
    if (
        not isinstance(expected_source_manifest_id, str)
        or SHA256.fullmatch(expected_source_manifest_id) is None
    ):
        raise PromotionError(
            "expected Product source manifest ID must be one lowercase SHA-256"
        )

    projection_parent = (
        repo_root / "Intermediate" / "ValtanProductProjection"
    ).resolve()
    projected_product_root = projected_product_root.resolve()
    try:
        projection_relative = projected_product_root.relative_to(projection_parent)
    except ValueError as exc:
        raise PromotionError(
            "projected Product root must be below Intermediate/ValtanProductProjection"
        ) from exc
    if not projection_relative.parts or not projected_product_root.is_dir():
        raise PromotionError("projected Product root is not one staged generation")

    if (external_lock_owner_pid is None) != (external_lock_owner_nonce is None):
        raise PromotionError(
            "external writer admission requires both owner PID and nonce"
        )
    if external_lock_owner_pid is None:
        admission = _exclusive_transaction_lock(
            repo_root,
            timeout_seconds=lock_timeout_seconds,
            operation="CommitProjectedProducts",
        )
    else:
        if lock_timeout_seconds != 0.0:
            raise PromotionError(
                "external writer admission cannot also request a lock timeout"
            )
        _assert_external_transaction_lock(
            repo_root,
            external_lock_owner_pid,
            external_lock_owner_nonce,
        )
        admission = contextlib.nullcontext()

    with admission:
        pipeline = _load_v2_pipeline(repo_root)
        product_relatives = _product_projection_relatives(repo_root)
        if not product_relatives or len(product_relatives) != len(set(product_relatives)):
            raise PromotionError("Valtan Product projection closure is empty or duplicated")

        def current_source_manifest_id() -> str:
            try:
                manifest = pipeline.source_manifest(repo_root)
            except (OSError, KeyError, TypeError, ValueError, pipeline.PipelineError) as exc:
                raise PromotionError(
                    f"cannot verify Valtan Product source manifest: {exc}"
                ) from exc
            manifest_id = (
                manifest.get("sourceManifestId")
                if isinstance(manifest, dict)
                else None
            )
            if not isinstance(manifest_id, str) or SHA256.fullmatch(manifest_id) is None:
                raise PromotionError("Valtan Product source manifest result is invalid")
            return manifest_id

        if current_source_manifest_id() != expected_source_manifest_id:
            raise PromotionError("Valtan sources changed before Product commit")

        targets: dict[Path, bytes] = {}
        expected_baselines: dict[Path, bytes | None] = {}
        for relative_text in product_relatives:
            if not isinstance(relative_text, str) or not relative_text:
                raise PromotionError("Valtan Product projection path is invalid")
            relative = Path(relative_text)
            if relative.is_absolute() or ".." in relative.parts:
                raise PromotionError(
                    f"Valtan Product projection path escapes its root: {relative_text}"
                )
            projected = (projected_product_root / relative).resolve()
            target = (repo_root / relative).resolve()
            try:
                projected.relative_to(projected_product_root)
                target_relative = target.relative_to(repo_root)
            except ValueError as exc:
                raise PromotionError(
                    f"Valtan Product projection path escapes its root: {relative_text}"
                ) from exc
            if not target_relative.parts or target_relative.parts[0].casefold() != "data":
                raise PromotionError(
                    f"Valtan Product target is outside Data: {relative_text}"
                )
            projected_payload = _read_bytes_or_none(projected)
            baseline = _read_bytes_or_none(target)
            if projected_payload is None or baseline is None:
                raise PromotionError(
                    f"Valtan Product projection/target is missing: {relative_text}"
                )
            _read_json_bytes(projected_payload, projected)
            expected_baselines[target] = baseline
            if projected_payload != baseline:
                targets[target] = projected_payload

        # Keep the manifest CAS adjacent to the target CAS.  The shared lock
        # excludes every cooperating Create/Publisher writer during both.
        if current_source_manifest_id() != expected_source_manifest_id:
            raise PromotionError("Valtan sources changed while staging Product commit")
        if targets:
            _atomic_commit(
                targets,
                expected_baselines={
                    target: expected_baselines[target] for target in targets
                },
                repository_root=repo_root,
                lock_already_held=True,
            )

    return {
        "schema": PRODUCT_COMMIT_RESULT_SCHEMA,
        "formatVersion": PRODUCT_COMMIT_RESULT_FORMAT_VERSION,
        "mode": "CommitProjectedProducts",
        "sourceManifestId": expected_source_manifest_id,
        "artifactCount": len(product_relatives),
        "changedCount": len(targets),
    }


def commit_typed_authoring_patch(
    repo_root: Path,
    draft_patch_path: Path,
    *,
    authoring_root: Path | None = None,
    pattern_sound_baseline_path: Path | None = None,
    pattern_sound_candidate_path: Path | None = None,
    effect_v2_baseline_path: Path | None = None,
    effect_v2_candidate_path: Path | None = None,
    effect_v2_read_set_path: Path | None = None,
    lock_timeout_seconds: float = 0.0,
    inject_failure_after: int | None = None,
) -> dict[str, Any]:
    """Commit one joined Pattern draft to its real split source owners.

    The Balance live-candidate path intentionally rejects changed Client
    presentation bytes.  Action Composition therefore needs a distinct
    repository-authoring transaction: resolve the effective saved authoring
    head, apply the typed patch once, split it back into the gameplay and
    presentation owners, project every generated Product, and CAS the complete
    closure under the same writer lock used by Create New Pattern and PublishV2.

    Boss-profile and damage-profile edits are excluded here because those
    owners have their own balance publisher/bootstrap transaction.  Pattern,
    Stage, Animation, Effect invocation and gameplay hit/motion operations are
    accepted by the shared strict draft validator.
    """

    repo_root = repo_root.resolve()
    draft_patch_path = draft_patch_path.resolve()
    if not draft_patch_path.is_file():
        raise PromotionError("typed Pattern draft patch is missing")
    draft_patch = _read_json(draft_patch_path)
    operations = draft_patch.get("operations") if isinstance(draft_patch, dict) else None
    if not isinstance(operations, list):
        raise PromotionError("typed Pattern draft operations are missing")
    forbidden_balance_ops = {"SET_BOSS_BASE_FIELD", "SET_DAMAGE_RATE"}
    for ordinal, operation in enumerate(operations):
        if not isinstance(operation, dict) or operation.get("op") in forbidden_balance_ops:
            raise PromotionError(
                "typed Pattern source commit cannot mutate Balance-owned boss/damage fields "
                f"(operation {ordinal})"
            )

    pipeline = _load_v2_pipeline(repo_root)
    resolved_authoring_root = (
        (repo_root / "Intermediate" / "ValtanTuningAuthoring")
        if authoring_root is None
        else authoring_root
    ).resolve()
    try:
        resolved_authoring_root.relative_to(repo_root.resolve())
    except ValueError as exc:
        raise PromotionError("AuthoringRoot must remain below the repository") from exc
    resolved_authoring_root.mkdir(parents=True, exist_ok=True)

    def read_owner_pair(
        owner: str,
        baseline_path: Path | None,
        candidate_path: Path | None,
    ) -> tuple[bytes, bytes] | None:
        if (baseline_path is None) != (candidate_path is None):
            raise PromotionError(f"{owner} baseline/candidate paths must be paired")
        if baseline_path is None or candidate_path is None:
            return None
        baseline = _read_bytes_or_none(baseline_path.resolve())
        candidate = _read_bytes_or_none(candidate_path.resolve())
        if not baseline or not candidate:
            raise PromotionError(f"{owner} baseline/candidate staging files are empty or missing")
        return baseline, candidate

    with _exclusive_transaction_lock(
        repo_root,
        timeout_seconds=lock_timeout_seconds,
        operation="ApplyTypedPatch",
    ):
        pipeline._recover_durable_transaction(
            repo_root, resolved_authoring_root, "authoring"
        )
        authoring_lock_path = resolved_authoring_root / ".save.lock"
        authoring_lock_fd, _lock_document, _transaction_id = (
            pipeline._acquire_transaction_lock(
                resolved_authoring_root, "authoring"
            )
        )
        try:
            current_sources = pipeline.source_manifest(repo_root)
            docs = pipeline.load_pipeline_documents(repo_root)
            current_head = pipeline._validated_authoring_head(
                repo_root, resolved_authoring_root, current_sources, docs
            )
            base_revision = (
                draft_patch.get("sourceRevision")
                if isinstance(draft_patch, dict)
                else ""
            )
            if base_revision != current_head:
                raise PromotionError(
                    "typed Pattern draft source revision is not the current authoring head"
                )
            committed_master, committed_bosses, committed_damage = (
                pipeline.resolve_authoring_base(
                    repo_root,
                    resolved_authoring_root,
                    base_revision,
                    current_sources,
                    docs,
                )
            )
            committed_master, committed_bosses, committed_damage, operation_count = (
                pipeline.apply_draft_patch(
                    committed_master,
                    committed_bosses,
                    committed_damage,
                    draft_patch,
                    base_revision,
                    docs[pipeline.WORLD_SET_REL],
                    docs[pipeline.COMBAT_AUTHORING_REL],
                    repository_root=repo_root,
                    effect_catalog=docs[pipeline.EFFECT_CATALOG_REL],
                )
            )
            if (
                committed_bosses != docs[pipeline.BOSS_PROFILES_REL]
                or committed_damage != docs[pipeline.DAMAGE_REL]
            ):
                raise PromotionError(
                    "typed Pattern commit escaped into Balance-owned documents"
                )

            gameplay, presentation = pipeline.split_v2_authoring(
                committed_master,
                docs[pipeline.WORLD_SET_REL],
                docs[pipeline.COMBAT_AUTHORING_REL],
            )
            outputs = validate_and_project(repo_root, gameplay, presentation)
            pattern_sound_pair = read_owner_pair(
                "Pattern Sound",
                pattern_sound_baseline_path,
                pattern_sound_candidate_path,
            )
            effect_v2_pair = read_owner_pair(
                "Effect V2",
                effect_v2_baseline_path,
                effect_v2_candidate_path,
            )
            pattern_sound_target = repo_root / PATTERN_SOUND_REL
            effect_v2_target = repo_root / EFFECT_V2_BINDINGS_REL
            if effect_v2_pair is None and effect_v2_read_set_path is not None:
                raise PromotionError(
                    "Effect V2 resource read-set requires a baseline/candidate pair"
                )
            effect_v2_physical_baseline = _read_bytes_or_none(effect_v2_target)
            if not effect_v2_physical_baseline:
                raise PromotionError("BOSS_VALTAN Effect V2 binding owner is missing")
            effect_v2_effective_bytes = (
                effect_v2_physical_baseline
                if effect_v2_pair is None
                else effect_v2_pair[1]
            )
            _validate_pattern_sound_dependencies_against_candidate_products(
                repo_root,
                outputs,
                sound_source_bytes=(
                    None
                    if pattern_sound_pair is None
                    else pattern_sound_pair[1]
                ),
            )
            effect_v2_effective_header = _read_json_bytes(
                effect_v2_effective_bytes, effect_v2_target
            )
            if effect_v2_effective_header.get("formatVersion") == 1:
                if effect_v2_read_set_path is not None:
                    raise PromotionError(
                        "BOSS_VALTAN Effect V2 formatVersion 1 does not admit a "
                        "formatVersion 2 resource read-set snapshot"
                    )
                _validate_effect_v2_bindings_v1_compatibility(
                    repo_root, outputs, effect_v2_effective_bytes
                )
                effect_v2_document = None
            else:
                if effect_v2_pair is not None and effect_v2_read_set_path is None:
                    raise PromotionError(
                        "Effect V2 formatVersion 2 Composition Save requires its "
                        "resource read-set snapshot; Reload before Save"
                    )
                effect_v2_document = (
                    _validate_effect_v2_bindings_against_candidate_products(
                        repo_root, outputs, effect_v2_effective_bytes, gameplay
                    )
                )
            if effect_v2_pair is not None and effect_v2_document is not None:
                binding_pipeline = _load_effect_v2_binding_pipeline(repo_root)
                try:
                    binding_pipeline.assert_resource_read_set_current(
                        repo_root,
                        effect_v2_document,
                        _read_json(effect_v2_read_set_path.resolve()),
                    )
                except binding_pipeline.BindingContractError as exc:
                    raise PromotionError(str(exc)) from exc

            target_payloads: dict[Path, bytes] = {
                repo_root / GAMEPLAY_REL: _json_text(gameplay).encode("utf-8"),
                repo_root / PRESENTATION_REL: _json_text(presentation).encode("utf-8"),
            }
            for relative, text in outputs.items():
                target_payloads[repo_root / relative] = text.encode("utf-8")
            provided_baselines: dict[Path, bytes] = {}
            if pattern_sound_pair is not None:
                target_payloads[pattern_sound_target] = pattern_sound_pair[1]
                provided_baselines[pattern_sound_target] = pattern_sound_pair[0]
            if effect_v2_pair is not None:
                target_payloads[effect_v2_target] = effect_v2_pair[1]
                provided_baselines[effect_v2_target] = effect_v2_pair[0]
            expected_baselines = {
                path: _read_bytes_or_none(path) for path in target_payloads
            }
            if any(value is None for value in expected_baselines.values()):
                raise PromotionError(
                    "typed Pattern source/Product closure contains a missing owner"
                )
            for path, provided_baseline in provided_baselines.items():
                if expected_baselines[path] != provided_baseline:
                    raise PromotionError(
                        f"{path.relative_to(repo_root).as_posix()} changed after the Composition draft began"
                    )
                expected_baselines[path] = provided_baseline
            if _read_bytes_or_none(effect_v2_target) != effect_v2_physical_baseline:
                raise PromotionError(
                    "BOSS_VALTAN Effect V2 binding owner changed while the typed patch was staged"
                )
            if pipeline.source_manifest(repo_root) != current_sources:
                raise PromotionError(
                    "Valtan source/Product closure changed while the typed patch was staged"
                )
            changed_targets = {
                path: payload
                for path, payload in target_payloads.items()
                if expected_baselines[path] != payload
            }
            if changed_targets:
                _atomic_commit(
                    changed_targets,
                    inject_failure_after=inject_failure_after,
                    expected_baselines={
                        path: expected_baselines[path] for path in changed_targets
                    },
                    repository_root=repo_root,
                    lock_already_held=True,
                )
            admitted = pipeline.source_manifest(repo_root)
        finally:
            os.close(authoring_lock_fd)
            authoring_lock_path.unlink(missing_ok=True)

    return {
        "schema": TYPED_PATCH_COMMIT_RESULT_SCHEMA,
        "formatVersion": TYPED_PATCH_COMMIT_RESULT_FORMAT_VERSION,
        "mode": "ApplyTypedPatch",
        "previousSourceRevision": current_sources["sourceManifestId"],
        "sourceRevision": admitted["sourceManifestId"],
        "operationCount": operation_count,
        "artifactCount": len(target_payloads),
        "changedCount": len(changed_targets),
        "runtimeActivation": "NOT_ACTIVATED",
    }


def create_pattern_from_request(
    repo_root: Path,
    request_path: Path,
    mode: str,
    *,
    inject_failure_after: int | None = None,
    inject_replace_failure_at: int | None = None,
    inject_rollback_failure_at: int | None = None,
    lock_timeout_seconds: float = 0.0,
) -> dict[str, Any]:
    if mode not in {"Validate", "Apply"}:
        raise PromotionError(f"unsupported Create New Pattern mode: {mode}")
    repo_root = repo_root.resolve()
    request = _read_json(request_path.resolve())
    with _exclusive_transaction_lock(
        repo_root,
        timeout_seconds=lock_timeout_seconds,
        operation=f"CreatePattern.{mode}",
    ):
        targets, expected_baselines, result = prepare_create_pattern_transaction(
            repo_root, request
        )
        if mode == "Apply":
            _atomic_commit(
                targets,
                inject_failure_after=inject_failure_after,
                inject_replace_failure_at=inject_replace_failure_at,
                inject_rollback_failure_at=inject_rollback_failure_at,
                expected_baselines=expected_baselines,
                repository_root=repo_root,
                lock_already_held=True,
            )
    return {
        "schema": CREATE_RESULT_SCHEMA,
        "formatVersion": CREATE_RESULT_FORMAT_VERSION,
        "mode": mode,
        **result,
    }


def run(
    repo_root: Path,
    mode: str,
    *,
    inject_failure_after: int | None = None,
    inject_replace_failure_at: int | None = None,
    inject_rollback_failure_at: int | None = None,
    lock_timeout_seconds: float = 0.0,
) -> dict[str, Any]:
    repo_root = repo_root.resolve()
    if mode not in {"Validate", "Apply"}:
        raise PromotionError(f"unsupported mode: {mode}")
    with _exclusive_transaction_lock(
        repo_root,
        timeout_seconds=lock_timeout_seconds,
        operation=f"AnimationPromotion.{mode}",
    ):
        gameplay, presentation, receipt = build_candidates(repo_root)
        outputs = validate_and_project(repo_root, gameplay, presentation)
        if mode == "Apply":
            targets: dict[Path, bytes] = {
                repo_root / GAMEPLAY_REL: _json_text(gameplay).encode("utf-8"),
                repo_root / PRESENTATION_REL: _json_text(presentation).encode("utf-8"),
                repo_root / RECEIPT_REL: _json_text(receipt).encode("utf-8"),
            }
            for relative, text in outputs.items():
                targets[repo_root / relative] = text.encode("utf-8")
            _atomic_commit(
                targets,
                inject_failure_after=inject_failure_after,
                inject_replace_failure_at=inject_replace_failure_at,
                inject_rollback_failure_at=inject_rollback_failure_at,
                repository_root=repo_root,
                lock_already_held=True,
            )
    return {
        "mode": mode,
        "patternCount": receipt["patternCount"],
        "stageCount": receipt["stageCount"],
        "sourceOccurrenceCount": sum(
            len(pattern["occurrences"]) for pattern in receipt["patterns"]
        ),
        "projectedArtifactCount": len(outputs),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repo-root",
        type=Path,
        default=Path(__file__).resolve().parents[2],
    )
    parser.add_argument(
        "--mode",
        choices=("Validate", "Apply", "CommitProjectedProducts", "ApplyTypedPatch"),
        default="Validate",
    )
    parser.add_argument(
        "--request-file",
        type=Path,
        help=(
            "Strict lostark.valtan-animation-pattern-create-request JSON. "
            "When present, stage the selected current/saved intake chain and "
            "run the same promotion/Product transaction."
        ),
    )
    parser.add_argument("--projected-product-root", type=Path)
    parser.add_argument("--expected-source-manifest-id")
    parser.add_argument("--draft-patch-file", type=Path)
    parser.add_argument("--authoring-root", type=Path)
    parser.add_argument("--lock-timeout-seconds", type=float, default=0.0)
    parser.add_argument("--external-lock-owner-pid", type=int, help=argparse.SUPPRESS)
    parser.add_argument("--external-lock-owner-nonce", help=argparse.SUPPRESS)
    parser.add_argument("--inject-failure-after", type=int, default=None, help=argparse.SUPPRESS)
    arguments = parser.parse_args()
    try:
        if arguments.mode == "CommitProjectedProducts":
            if (
                arguments.request_file is not None
                or arguments.projected_product_root is None
                or arguments.expected_source_manifest_id is None
                or arguments.draft_patch_file is not None
                or arguments.authoring_root is not None
                or arguments.inject_failure_after is not None
            ):
                raise PromotionError(
                    "CommitProjectedProducts requires only projected root and expected source manifest"
                )
            result = commit_projected_products(
                arguments.repo_root,
                arguments.projected_product_root,
                arguments.expected_source_manifest_id,
                lock_timeout_seconds=arguments.lock_timeout_seconds,
                external_lock_owner_pid=arguments.external_lock_owner_pid,
                external_lock_owner_nonce=arguments.external_lock_owner_nonce,
            )
        elif arguments.mode == "ApplyTypedPatch":
            if (
                arguments.request_file is not None
                or arguments.projected_product_root is not None
                or arguments.expected_source_manifest_id is not None
                or arguments.draft_patch_file is None
                or arguments.external_lock_owner_pid is not None
                or arguments.external_lock_owner_nonce is not None
            ):
                raise PromotionError(
                    "ApplyTypedPatch requires only a draft patch and optional authoring root"
                )
            result = commit_typed_authoring_patch(
                arguments.repo_root,
                arguments.draft_patch_file,
                authoring_root=arguments.authoring_root,
                lock_timeout_seconds=arguments.lock_timeout_seconds,
                inject_failure_after=arguments.inject_failure_after,
            )
        else:
            if (
                arguments.projected_product_root is not None
                or arguments.expected_source_manifest_id is not None
                or arguments.draft_patch_file is not None
                or arguments.authoring_root is not None
                or arguments.lock_timeout_seconds != 0.0
                or arguments.external_lock_owner_pid is not None
                or arguments.external_lock_owner_nonce is not None
            ):
                raise PromotionError(
                    "Product commit arguments are invalid for Validate/Apply"
                )
            result = (
                create_pattern_from_request(
                    arguments.repo_root,
                    arguments.request_file,
                    arguments.mode,
                    inject_failure_after=arguments.inject_failure_after,
                )
                if arguments.request_file is not None
                else run(
                    arguments.repo_root,
                    arguments.mode,
                    inject_failure_after=arguments.inject_failure_after,
                )
            )
    except PromotionError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1
    print(json.dumps(result, ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Validate the Drive-owned runtime resource boundary.

The physical ``Client/Bin/Resources`` directory is a local runtime input.  Git
owns stable relative asset IDs in source documents, but it must not own files
inside the physical resource tree.  This validator deliberately does not hash
or version the resource pack; the team lead distributes that pack through the
shared Drive.
"""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path


RESOURCE_PREFIX = "Client/Bin/Resources/"
RESOURCE_IGNORE_RULE = "/Client/Bin/Resources/"


@dataclass(frozen=True)
class ValidationResult:
    repository_root: Path
    tracked_paths: tuple[str, ...]
    physical_file_count: int
    physical_bytes: int
    errors: tuple[str, ...]

    @property
    def ok(self) -> bool:
        return not self.errors

    def as_json(self) -> dict[str, object]:
        return {
            "schema": "lostark.resource-delivery-policy-validation",
            "formatVersion": 1,
            "ok": self.ok,
            "trackedResourceCount": len(self.tracked_paths),
            "physicalResourceFileCount": self.physical_file_count,
            "physicalResourceBytes": self.physical_bytes,
            "errors": list(self.errors),
        }


def _git_tracked_resource_paths(repository_root: Path) -> tuple[str, ...]:
    completed = subprocess.run(
        [
            "git",
            "-C",
            str(repository_root),
            "ls-files",
            "-z",
            "--",
            RESOURCE_PREFIX,
        ],
        check=False,
        capture_output=True,
    )
    if completed.returncode != 0:
        detail = completed.stderr.decode("utf-8", errors="replace").strip()
        raise RuntimeError(f"git ls-files failed: {detail or completed.returncode}")
    return tuple(
        path.decode("utf-8", errors="strict")
        for path in completed.stdout.split(b"\0")
        if path
    )


def validate(repository_root: Path, require_local: bool = False) -> ValidationResult:
    repository_root = repository_root.resolve()
    errors: list[str] = []

    ignore_path = repository_root / ".gitignore"
    if not ignore_path.is_file():
        errors.append(".gitignore is missing")
    else:
        ignore_lines = {
            line.strip()
            for line in ignore_path.read_text(encoding="utf-8-sig").splitlines()
        }
        if RESOURCE_IGNORE_RULE not in ignore_lines:
            errors.append(
                f".gitignore must contain the exact rule {RESOURCE_IGNORE_RULE}"
            )

    try:
        tracked_paths = _git_tracked_resource_paths(repository_root)
    except RuntimeError as error:
        tracked_paths = ()
        errors.append(str(error))
    if tracked_paths:
        errors.append(
            "Client/Bin/Resources is Drive-owned but Git still tracks "
            f"{len(tracked_paths)} file(s)"
        )

    resource_root = repository_root / "Client" / "Bin" / "Resources"
    physical_file_count = 0
    physical_bytes = 0
    if resource_root.is_dir():
        for path in resource_root.rglob("*"):
            if not path.is_file():
                continue
            physical_file_count += 1
            physical_bytes += path.stat().st_size
    elif require_local:
        errors.append(
            "Client/Bin/Resources is absent; hydrate the team Drive resource tree"
        )

    return ValidationResult(
        repository_root=repository_root,
        tracked_paths=tracked_paths,
        physical_file_count=physical_file_count,
        physical_bytes=physical_bytes,
        errors=tuple(errors),
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--repository-root",
        type=Path,
        default=Path(__file__).resolve().parents[2],
    )
    parser.add_argument("--require-local", action="store_true")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    result = validate(args.repository_root, args.require_local)
    if args.json:
        print(json.dumps(result.as_json(), ensure_ascii=False, sort_keys=True))
    else:
        verdict = "PASS" if result.ok else "FAIL"
        print(
            f"Resource delivery policy {verdict}: "
            f"tracked={len(result.tracked_paths)} "
            f"physicalFiles={result.physical_file_count} "
            f"physicalBytes={result.physical_bytes}"
        )
        for error in result.errors:
            print(f"ERROR: {error}", file=sys.stderr)
    return 0 if result.ok else 1


if __name__ == "__main__":
    raise SystemExit(main())

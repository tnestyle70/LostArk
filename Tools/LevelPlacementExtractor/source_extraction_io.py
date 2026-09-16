"""Stage source extraction outputs and preserve the previous pair on failure.

This is rollback for reported filesystem errors, not a crash-safe transaction
across two files. Callers finish source/content validation before committing.
"""

from __future__ import annotations

import os
from pathlib import Path
import tempfile


def _stage(target: Path, payload: bytes) -> Path:
    target.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary = tempfile.mkstemp(prefix=f".{target.name}.", suffix=".tmp", dir=target.parent)
    staged = Path(temporary)
    try:
        with os.fdopen(descriptor, "wb") as stream:
            stream.write(payload)
    except BaseException:
        staged.unlink(missing_ok=True)
        raise
    return staged


def write_atomic(path: Path, payload: bytes) -> None:
    staged = _stage(path, payload)
    try:
        os.replace(staged, path)
    finally:
        staged.unlink(missing_ok=True)


def write_pair(output: Path, output_bytes: bytes, receipt: Path, receipt_bytes: bytes) -> None:
    if output.resolve() == receipt.resolve():
        raise ValueError("output and receipt must differ")
    before = {path: path.read_bytes() if path.exists() else None for path in (output, receipt)}
    staged: list[tuple[Path, Path]] = []
    promoted: list[Path] = []
    try:
        for target, payload in ((output, output_bytes), (receipt, receipt_bytes)):
            staged.append((_stage(target, payload), target))
        for temporary, target in staged:
            os.replace(temporary, target)
            promoted.append(target)
    except BaseException:
        for target in reversed(promoted):
            if before[target] is None:
                target.unlink(missing_ok=True)
            else:
                target.write_bytes(before[target])
        raise
    finally:
        for temporary, _ in staged:
            temporary.unlink(missing_ok=True)

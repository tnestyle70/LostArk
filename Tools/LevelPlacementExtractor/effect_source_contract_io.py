from __future__ import annotations

import hashlib
from pathlib import Path


def normalize_utf8_eol(content: bytes) -> bytes:
    """Canonicalize repository-tracked UTF-8 text without inspecting a suffix."""
    text = content.decode("utf-8")
    return text.replace("\r\n", "\n").replace("\r", "\n").encode("utf-8")


def tracked_text_sha256(path: Path) -> str:
    """Hash repository-tracked UTF-8 text independent of checkout EOL."""
    return hashlib.sha256(normalize_utf8_eol(path.read_bytes())).hexdigest()


def raw_file_sha256(path: Path) -> str:
    """Hash immutable external source evidence and binary artifacts byte-for-byte."""
    return hashlib.sha256(path.read_bytes()).hexdigest()


def generated_text_matches(path: Path, expected: bytes) -> bool:
    """Compare a generated UTF-8 JSON/header artifact with EOL normalization."""
    return path.is_file() and normalize_utf8_eol(path.read_bytes()) == normalize_utf8_eol(expected)

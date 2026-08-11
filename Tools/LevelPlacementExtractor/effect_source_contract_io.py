from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any


def _object_without_duplicate_keys(
    pairs: list[tuple[str, Any]],
) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def load_strict_json_object(path: Path) -> dict[str, Any]:
    """Load one UTF-8 JSON object while rejecting duplicate keys at any depth."""
    payload = path.read_bytes()
    if payload.startswith(b"\xef\xbb\xbf"):
        raise ValueError(f"JSON must be UTF-8 without BOM: {path}")
    try:
        value = json.loads(
            payload.decode("utf-8"),
            object_pairs_hook=_object_without_duplicate_keys,
        )
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise ValueError(f"cannot parse JSON {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise ValueError(f"JSON root must be an object: {path}")
    return value


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

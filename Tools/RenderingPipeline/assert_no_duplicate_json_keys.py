#!/usr/bin/env python3
"""Reject duplicate JSON object keys before PowerShell collapses them."""

from __future__ import annotations

import json
import pathlib
import sys


class DuplicateJsonKeyError(ValueError):
    pass


def reject_duplicate_keys(pairs: list[tuple[str, object]]) -> dict[str, object]:
    result: dict[str, object] = {}
    for key, value in pairs:
        if key in result:
            raise DuplicateJsonKeyError(f"duplicate JSON object key: {key}")
        result[key] = value
    return result


def reject_nonstandard_constant(value: str) -> None:
    raise ValueError(f"non-standard JSON constant: {value}")


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: assert_no_duplicate_json_keys.py <json-path>", file=sys.stderr)
        return 2

    source_path = pathlib.Path(sys.argv[1])
    try:
        source_text = source_path.read_text(encoding="utf-8-sig")
        json.loads(
            source_text,
            object_pairs_hook=reject_duplicate_keys,
            parse_constant=reject_nonstandard_constant,
        )
    except (OSError, UnicodeError, json.JSONDecodeError, ValueError) as error:
        print(f"strict JSON validation failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

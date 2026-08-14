#!/usr/bin/env python3
"""Validate direct authored v13 Effect rows without changing derived authority.

The established derived-artifact validator remains the authority for compiled,
legacy, and reconstructed runtime rows.  This adapter validates the additive
DIRECT_AUTHORED_DOCUMENT_V13 payload, projects those rows out, and delegates
the unchanged remainder to that validator.
"""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
from pathlib import Path, PurePosixPath
import re
import sys
from typing import Any


DIRECT_PAYLOAD_KIND = "DIRECT_AUTHORED_DOCUMENT_V13"
RUNTIME_SCHEMA = "lostark.effect-runtime-catalog"
RUNTIME_VERSION = 3
AUTHORING_SCHEMA = "lostark.effect-authoring"
AUTHORING_VERSION = 13
DIRECT_ENTRY_KEYS = (
    "payloadKind",
    "effectAssetId",
    "authoringFormatVersion",
    "contentSha256",
    "dependencies",
    "authoredDocumentUtf8",
)
DEPENDENCY_KEYS = ("assetId", "sha256")
STABLE_ID_PATTERN = re.compile(r"^[a-z0-9]+(?:[._-][a-z0-9]+)*$")
SHA256_PATTERN = re.compile(r"^[0-9a-f]{64}$")


class ContractError(RuntimeError):
    """Raised when a runtime catalog violates the direct authored contract."""


def _object_no_duplicates(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ContractError(f"duplicate JSON property: {key}")
        result[key] = value
    return result


def _load_json(path: Path) -> dict[str, Any]:
    try:
        payload = path.read_bytes()
    except OSError as exc:
        raise ContractError(f"cannot read runtime catalog: {exc}") from exc
    if payload.startswith(b"\xef\xbb\xbf"):
        raise ContractError("runtime catalog must be UTF-8 without BOM")
    if b"\r" in payload:
        raise ContractError("runtime catalog must use LF line endings")
    try:
        value = json.loads(
            payload.decode("utf-8"), object_pairs_hook=_object_no_duplicates
        )
    except (UnicodeError, json.JSONDecodeError) as exc:
        raise ContractError(f"runtime catalog JSON is invalid: {exc}") from exc
    if not isinstance(value, dict):
        raise ContractError("runtime catalog root must be an object")
    return value


def _require_exact_order(value: Any, keys: tuple[str, ...], label: str) -> None:
    if not isinstance(value, dict) or tuple(value.keys()) != keys:
        raise ContractError(f"{label} fields or order are invalid")


def _require_sha(value: Any, label: str) -> str:
    if not isinstance(value, str) or SHA256_PATTERN.fullmatch(value) is None:
        raise ContractError(f"{label} must be a lowercase SHA-256")
    return value


def _require_stable_id(value: Any, label: str) -> str:
    if (
        not isinstance(value, str)
        or len(value) > 255
        or STABLE_ID_PATTERN.fullmatch(value) is None
    ):
        raise ContractError(f"{label} is not a stable ID")
    return value


def _validate_dependency_asset_id(value: Any, previous: str) -> str:
    if not isinstance(value, str):
        raise ContractError("direct authored dependency assetId must be a string")
    path = PurePosixPath(value)
    if (
        len(value) > 1024
        or not value.startswith(("Effect/", "Character/"))
        or "\\" in value
        or ":" in value
        or "//" in value
        or path.is_absolute()
        or any(part in ("", ".", "..") for part in path.parts)
        or any(ord(character) < 0x20 or ord(character) == 0x7F for character in value)
        or value <= previous
    ):
        raise ContractError(
            "direct authored dependency ID is unsafe, duplicate, or unsorted"
        )
    return value


def validate_direct_entry(entry: dict[str, Any], effect_id: str) -> None:
    _require_exact_order(entry, DIRECT_ENTRY_KEYS, "direct authored runtime entry")
    if entry["payloadKind"] != DIRECT_PAYLOAD_KIND:
        raise ContractError("direct authored runtime payloadKind mismatch")
    if entry["effectAssetId"] != effect_id:
        raise ContractError("direct authored outer effect identity mismatch")
    version = entry["authoringFormatVersion"]
    if type(version) is not int or version != AUTHORING_VERSION:
        raise ContractError("direct authored runtime authoring version is invalid")

    content_sha = _require_sha(
        entry["contentSha256"], "direct authored runtime contentSha256"
    )
    authored_text = entry["authoredDocumentUtf8"]
    if not isinstance(authored_text, str) or authored_text.startswith("\ufeff"):
        raise ContractError("direct authored runtime document text is invalid")
    try:
        authored_bytes = authored_text.encode("utf-8")
    except UnicodeEncodeError as exc:
        raise ContractError("direct authored runtime document is not UTF-8") from exc
    if hashlib.sha256(authored_bytes).hexdigest() != content_sha:
        raise ContractError("direct authored runtime document hash mismatch")
    try:
        authored = json.loads(
            authored_text, object_pairs_hook=_object_no_duplicates
        )
    except (UnicodeError, json.JSONDecodeError) as exc:
        raise ContractError(
            f"direct authored runtime document JSON is invalid: {exc}"
        ) from exc
    if not isinstance(authored, dict):
        raise ContractError("direct authored runtime document root is invalid")
    authored_version = authored.get("version")
    if (
        authored.get("schema") != AUTHORING_SCHEMA
        or type(authored_version) is not int
        or authored_version != AUTHORING_VERSION
        or authored.get("effectAssetId") != effect_id
        or not isinstance(authored.get("elements"), list)
    ):
        raise ContractError("direct authored runtime document identity mismatch")

    dependencies = entry["dependencies"]
    if not isinstance(dependencies, list):
        raise ContractError("direct authored runtime dependencies are invalid")
    previous_asset_id = ""
    for index, dependency in enumerate(dependencies):
        _require_exact_order(
            dependency,
            DEPENDENCY_KEYS,
            f"direct authored dependency[{index}]",
        )
        previous_asset_id = _validate_dependency_asset_id(
            dependency["assetId"], previous_asset_id
        )
        _require_sha(
            dependency["sha256"],
            f"direct authored dependency[{index}].sha256",
        )


def _load_derived_validator() -> Any:
    path = Path(__file__).with_name("build_effect_derived_artifact.py")
    spec = importlib.util.spec_from_file_location(
        "lostark_effect_derived_artifact_validator", path
    )
    if spec is None or spec.loader is None:
        raise ContractError("cannot load the derived Effect validator")
    module = importlib.util.module_from_spec(spec)
    try:
        spec.loader.exec_module(module)
    except Exception as exc:
        raise ContractError(f"cannot initialize the derived Effect validator: {exc}") from exc
    return module


def validate_runtime_catalog(value: dict[str, Any]) -> None:
    _require_exact_order(
        value,
        ("schema", "formatVersion", "components", "effects"),
        "runtime catalog",
    )
    if value["schema"] != RUNTIME_SCHEMA:
        raise ContractError("runtime catalog schema mismatch")
    version = value["formatVersion"]
    if type(version) is not int or version != RUNTIME_VERSION:
        raise ContractError("runtime catalog formatVersion mismatch")
    if not isinstance(value["components"], list) or not isinstance(
        value["effects"], list
    ):
        raise ContractError("runtime catalog arrays are invalid")

    effect_ids: set[str] = set()
    non_direct: list[dict[str, Any]] = []
    for index, entry in enumerate(value["effects"]):
        if not isinstance(entry, dict):
            raise ContractError(f"runtime effect[{index}] must be an object")
        effect_id = _require_stable_id(
            entry.get("effectAssetId"), f"runtime effect[{index}] ID"
        )
        if effect_id in effect_ids:
            raise ContractError(f"duplicate runtime effect ID: {effect_id}")
        effect_ids.add(effect_id)
        if entry.get("payloadKind") == DIRECT_PAYLOAD_KIND:
            validate_direct_entry(entry, effect_id)
        else:
            non_direct.append(entry)

    projected = {
        "schema": value["schema"],
        "formatVersion": value["formatVersion"],
        "components": value["components"],
        "effects": non_direct,
    }
    derived = _load_derived_validator()
    try:
        derived.validate_runtime_catalog(projected)
    except Exception as exc:
        if exc.__class__.__name__ == "ContractError":
            raise ContractError(str(exc)) from exc
        raise


def make_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--catalog", type=Path, required=True)
    return parser


def main(argv: list[str] | None = None) -> int:
    args = make_parser().parse_args(argv)
    try:
        validate_runtime_catalog(_load_json(args.catalog))
    except ContractError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

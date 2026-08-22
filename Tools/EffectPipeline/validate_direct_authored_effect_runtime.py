#!/usr/bin/env python3
"""Validate minimal direct-authored v13 Effect rows and their documents.

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
import stat
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
    "authoredDocumentPath",
)
DIRECT_ENTRY_WITH_SCREEN_OVERLAY_KEYS = DIRECT_ENTRY_KEYS + (
    "screenOverlayPresentation",
)
SCREEN_OVERLAY_BINDING_KEYS = (
    "presentationId",
    "byteCount",
    "sha256",
    "utf8Json",
    "resources",
)
SCREEN_OVERLAY_RESOURCE_KEYS = ("assetId", "byteCount", "sha256")
STABLE_ID_PATTERN = re.compile(r"^[a-z0-9]+(?:[._-][a-z0-9]+)*$")


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


def _require_stable_id(value: Any, label: str) -> str:
    if (
        not isinstance(value, str)
        or len(value) > 255
        or STABLE_ID_PATTERN.fullmatch(value) is None
    ):
        raise ContractError(f"{label} is not a stable ID")
    return value


def _load_sealed_authored_document(
    runtime_catalog_path: Path,
    authored_document_path: Any,
    effect_id: str,
) -> dict[str, Any]:
    if not isinstance(authored_document_path, str):
        raise ContractError("direct authored runtime document path must be a string")
    relative_path = PurePosixPath(authored_document_path)
    sealed_name_pattern = re.compile(
        rf"^{re.escape(effect_id)}\.[0-9a-f]{{64}}\.effect\.json$"
    )
    if (
        len(authored_document_path) > 1024
        or "\\" in authored_document_path
        or ":" in authored_document_path
        or "//" in authored_document_path
        or relative_path.is_absolute()
        or tuple(relative_path.parts[:1]) != ("Authored",)
        or len(relative_path.parts) != 2
        or any(part in ("", ".", "..") for part in relative_path.parts)
        or sealed_name_pattern.fullmatch(relative_path.name) is None
        or any(
            ord(character) < 0x20 or ord(character) == 0x7F
            for character in authored_document_path
        )
    ):
        raise ContractError(
            "direct authored runtime document path is unsafe or does not match its ID"
        )

    catalog_root = runtime_catalog_path.resolve(strict=False).parent
    candidate = catalog_root.joinpath(*relative_path.parts)
    try:
        resolved = candidate.resolve(strict=True)
    except OSError as exc:
        raise ContractError(
            f"direct authored runtime document could not be resolved: {exc}"
        ) from exc
    try:
        resolved.relative_to(catalog_root)
    except ValueError as exc:
        raise ContractError(
            "direct authored runtime document escapes the runtime catalog directory"
        ) from exc
    try:
        candidate_stat = candidate.lstat()
    except OSError as exc:
        raise ContractError(
            f"direct authored runtime document could not be inspected: {exc}"
        ) from exc
    reparse_flag = getattr(stat, "FILE_ATTRIBUTE_REPARSE_POINT", 0)
    file_attributes = getattr(candidate_stat, "st_file_attributes", 0)
    if (
        not stat.S_ISREG(candidate_stat.st_mode)
        or (reparse_flag and file_attributes & reparse_flag)
        or not resolved.is_file()
    ):
        raise ContractError("direct authored runtime document is not a regular file")
    try:
        authored_bytes = resolved.read_bytes()
    except OSError as exc:
        raise ContractError(
            f"direct authored runtime document could not be read: {exc}"
        ) from exc
    if authored_bytes.startswith(b"\xef\xbb\xbf"):
        raise ContractError("direct authored runtime document must be UTF-8 without BOM")
    try:
        authored = json.loads(
            authored_bytes.decode("utf-8"), object_pairs_hook=_object_no_duplicates
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
    return authored


def validate_direct_entry(
    entry: dict[str, Any], effect_id: str, runtime_catalog_path: Path
) -> None:
    if tuple(entry.keys()) not in (
        DIRECT_ENTRY_KEYS,
        DIRECT_ENTRY_WITH_SCREEN_OVERLAY_KEYS,
    ):
        raise ContractError("direct authored runtime entry fields or order are invalid")
    if entry["payloadKind"] != DIRECT_PAYLOAD_KIND:
        raise ContractError("direct authored runtime payloadKind mismatch")
    if entry["effectAssetId"] != effect_id:
        raise ContractError("direct authored outer effect identity mismatch")
    version = entry["authoringFormatVersion"]
    if type(version) is not int or version != AUTHORING_VERSION:
        raise ContractError("direct authored runtime authoring version is invalid")

    _load_sealed_authored_document(
        runtime_catalog_path,
        entry["authoredDocumentPath"],
        effect_id,
    )
    binding = entry.get("screenOverlayPresentation")
    if binding is not None:
        validate_screen_overlay_binding(binding, effect_id)


def validate_screen_overlay_binding(value: Any, effect_id: str) -> None:
    _require_exact_order(
        value, SCREEN_OVERLAY_BINDING_KEYS, "screen-overlay Product binding"
    )
    presentation_id = _require_stable_id(
        value["presentationId"], "screen-overlay presentationId"
    )
    if presentation_id != f"{effect_id}.screen-overlay":
        raise ContractError("screen-overlay presentation/effect identity mismatch")
    byte_count = value["byteCount"]
    sha256 = value["sha256"]
    utf8_json = value["utf8Json"]
    resources = value["resources"]
    if (
        type(byte_count) is not int
        or byte_count < 1
        or byte_count > 4 * 1024 * 1024
        or not isinstance(sha256, str)
        or re.fullmatch(r"[0-9a-f]{64}", sha256) is None
        or not isinstance(utf8_json, str)
        or not isinstance(resources, list)
        or not 1 <= len(resources) <= 64
    ):
        raise ContractError("screen-overlay Product binding identity is invalid")
    payload = utf8_json.encode("utf-8")
    if len(payload) != byte_count or hashlib.sha256(payload).hexdigest() != sha256:
        raise ContractError("screen-overlay Product binding payload identity mismatch")

    resource_ids: set[str] = set()
    for resource in resources:
        _require_exact_order(
            resource, SCREEN_OVERLAY_RESOURCE_KEYS, "screen-overlay resource"
        )
        asset_id = resource["assetId"]
        resource_bytes = resource["byteCount"]
        resource_sha = resource["sha256"]
        if (
            not isinstance(asset_id, str)
            or not asset_id.startswith("Effect/")
            or not asset_id.endswith(".dds")
            or "\\" in asset_id
            or ":" in asset_id
            or any(part in ("", ".", "..") for part in asset_id.split("/"))
            or asset_id in resource_ids
            or type(resource_bytes) is not int
            or resource_bytes < 1
            or not isinstance(resource_sha, str)
            or re.fullmatch(r"[0-9a-f]{64}", resource_sha) is None
        ):
            raise ContractError("screen-overlay resource identity is invalid")
        resource_ids.add(asset_id)

    try:
        embedded = json.loads(utf8_json, object_pairs_hook=_object_no_duplicates)
    except (UnicodeError, json.JSONDecodeError) as exc:
        raise ContractError(f"embedded screen-overlay JSON is invalid: {exc}") from exc
    _require_exact_order(
        embedded,
        ("schema", "formatVersion", "provenance", "presentationId", "overlays"),
        "embedded screen-overlay root",
    )
    overlays = embedded["overlays"]
    if (
        embedded["schema"] != "lostark.effect-screen-overlay"
        or type(embedded["formatVersion"]) is not int
        or embedded["formatVersion"] != 1
        or embedded["provenance"] != "PROJECT_TUNED"
        or embedded["presentationId"] != presentation_id
        or not isinstance(overlays, list)
        or not 1 <= len(overlays) <= 64
    ):
        raise ContractError("embedded screen-overlay header is invalid")
    referenced_ids: set[str] = set()
    for overlay in overlays:
        if not isinstance(overlay, dict) or not isinstance(
            overlay.get("textureAssetId"), str
        ):
            raise ContractError("embedded screen-overlay texture row is invalid")
        referenced_ids.add(overlay["textureAssetId"])
    if referenced_ids != resource_ids:
        raise ContractError("embedded screen-overlay resource closure mismatch")


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


def validate_runtime_catalog(
    value: dict[str, Any], runtime_catalog_path: Path
) -> None:
    root_keys = tuple(value.keys())
    plain_keys = ("schema", "formatVersion", "components", "effects")
    sidecar_plain_keys = (
        "schema",
        "formatVersion",
        "visualProgramSidecarRequired",
        "components",
        "effects",
    )
    if root_keys not in (plain_keys, sidecar_plain_keys):
        raise ContractError("runtime catalog fields or order are invalid")
    if "visualProgramSidecarRequired" in value and type(
        value["visualProgramSidecarRequired"]
    ) is not bool:
        raise ContractError("runtime catalog visual-program sidecar marker is invalid")
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
            validate_direct_entry(entry, effect_id, runtime_catalog_path)
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
    runtime_catalog_path = args.catalog.resolve()
    try:
        validate_runtime_catalog(
            _load_json(runtime_catalog_path), runtime_catalog_path
        )
    except ContractError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Remove invisible Elements from canonical authored Effect documents.

Imported source graphs, receipts, DDS, and WModel resources are intentionally
outside this operation.  The authored JSON remains the manual restoration
surface; only rows whose explicit ``visible`` field is the JSON boolean false
are removed.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
from typing import Any


def _encode(document: dict[str, Any]) -> bytes:
    return (json.dumps(document, ensure_ascii=False, indent=2) + "\n").encode(
        "utf-8"
    )


def _load(path: Path) -> tuple[dict[str, Any], bytes]:
    raw = path.read_bytes()
    if raw.startswith(b"\xef\xbb\xbf"):
        raise ValueError(f"UTF-8 BOM is not allowed: {path}")
    document = json.loads(raw.decode("utf-8"))
    if not isinstance(document, dict):
        raise ValueError(f"Effect document root must be an object: {path}")
    if document.get("schema") != "lostark.effect-authoring":
        raise ValueError(f"Unexpected Effect document schema: {path}")
    if not isinstance(document.get("effectAssetId"), str):
        raise ValueError(f"Effect document identity is missing: {path}")
    elements = document.get("elements")
    if not isinstance(elements, list):
        raise ValueError(f"Effect document elements must be an array: {path}")
    ids: set[str] = set()
    for element in elements:
        if not isinstance(element, dict) or not isinstance(element.get("id"), str):
            raise ValueError(f"Effect Element identity is invalid: {path}")
        element_id = element["id"]
        if element_id in ids:
            raise ValueError(f"Duplicate Effect Element identity {element_id}: {path}")
        ids.add(element_id)
    return document, raw


def _is_reparse_point(path: Path) -> bool:
    attributes = getattr(path.stat(), "st_file_attributes", 0)
    reparse_flag = getattr(os, "FILE_ATTRIBUTE_REPARSE_POINT", 0x400)
    return bool(attributes & reparse_flag)


def _cleanup_stale_sealed_documents(
    repo_root: Path,
    expected_stale_count: int | None,
    expected_stale_bytes: int | None,
) -> None:
    if expected_stale_count is None or expected_stale_bytes is None:
        raise ValueError(
            "Sealed cleanup requires --expected-stale-count and "
            "--expected-stale-bytes"
        )

    catalog_path = (
        repo_root
        / "Client"
        / "Bin"
        / "DataFiles"
        / "Effect"
        / "EffectCatalog.runtime.json"
    ).resolve()
    sealed_root = (catalog_path.parent / "Authored").resolve()
    if not catalog_path.is_file() or not sealed_root.is_dir():
        raise ValueError("Runtime Effect catalog or sealed Authored directory is missing")
    if _is_reparse_point(sealed_root):
        raise ValueError(f"Sealed Authored directory must not be a reparse point: {sealed_root}")

    catalog = json.loads(catalog_path.read_text(encoding="utf-8"))
    effects = catalog.get("effects")
    if not isinstance(effects, list):
        raise ValueError("Runtime Effect catalog effects must be an array")

    referenced: set[Path] = set()
    for effect in effects:
        if not isinstance(effect, dict):
            raise ValueError("Runtime Effect catalog row must be an object")
        if effect.get("payloadKind") != "DIRECT_AUTHORED_DOCUMENT_V13":
            continue
        relative = effect.get("authoredDocumentPath")
        expected_sha = effect.get("contentSha256")
        if not isinstance(relative, str) or not isinstance(expected_sha, str):
            raise ValueError("Direct authored catalog identity is incomplete")
        relative_path = Path(relative)
        if (
            relative_path.is_absolute()
            or relative_path.parts[:1] != ("Authored",)
            or len(relative_path.parts) != 2
        ):
            raise ValueError(f"Unsafe sealed authored path: {relative}")
        resolved = (catalog_path.parent / relative_path).resolve()
        if resolved.parent != sealed_root or not resolved.is_file():
            raise ValueError(f"Sealed authored path escaped or is missing: {relative}")
        if _is_reparse_point(resolved):
            raise ValueError(f"Sealed authored file must not be a reparse point: {resolved}")
        actual_sha = hashlib.sha256(resolved.read_bytes()).hexdigest()
        if actual_sha != expected_sha:
            raise ValueError(
                f"Sealed authored hash mismatch: {relative} "
                f"expected={expected_sha} actual={actual_sha}"
            )
        referenced.add(resolved)

    if len(referenced) != 98:
        raise ValueError(f"Expected 98 referenced direct documents, found {len(referenced)}")

    all_sealed = sorted(sealed_root.glob("*.effect.json"))
    for path in all_sealed:
        resolved = path.resolve()
        if resolved.parent != sealed_root or not resolved.is_file():
            raise ValueError(f"Unsafe sealed cleanup candidate: {path}")
        if _is_reparse_point(resolved):
            raise ValueError(f"Sealed cleanup candidate is a reparse point: {resolved}")
    stale = [path for path in all_sealed if path.resolve() not in referenced]
    stale_bytes = sum(path.stat().st_size for path in stale)
    print(
        f"sealed_referenced={len(referenced)} sealed_stale={len(stale)} "
        f"stale_bytes={stale_bytes}"
    )
    if len(stale) != expected_stale_count or stale_bytes != expected_stale_bytes:
        raise ValueError(
            "Sealed cleanup expectation mismatch: "
            f"expected_count={expected_stale_count} actual_count={len(stale)} "
            f"expected_bytes={expected_stale_bytes} actual_bytes={stale_bytes}"
        )

    for path in stale:
        path.unlink()

    remaining = sorted(sealed_root.glob("*.effect.json"))
    remaining_resolved = {path.resolve() for path in remaining}
    if remaining_resolved != referenced:
        raise ValueError(
            "Sealed cleanup postcondition failed: remaining files do not exactly "
            "match the runtime catalog"
        )
    print(f"sealed_cleanup_complete={len(stale)} sealed_remaining={len(remaining)}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--repo-root",
        type=Path,
        default=Path(__file__).resolve().parents[2],
    )
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--require-clean", action="store_true")
    parser.add_argument("--cleanup-stale-sealed", action="store_true")
    parser.add_argument("--expected-stale-count", type=int)
    parser.add_argument("--expected-stale-bytes", type=int)
    args = parser.parse_args()

    repo_root = args.repo_root.resolve()
    if args.cleanup_stale_sealed:
        _cleanup_stale_sealed_documents(
            repo_root,
            args.expected_stale_count,
            args.expected_stale_bytes,
        )

    authored_root = (repo_root / "Data" / "Effects" / "Authored").resolve()
    if not authored_root.is_dir():
        raise SystemExit(f"Authored Effect directory is missing: {authored_root}")

    staged: list[tuple[Path, Path]] = []
    document_count = 0
    before_elements = 0
    removed_elements = 0
    before_bytes = 0
    after_bytes = 0
    try:
        for path in sorted(authored_root.glob("*.effect.json")):
            document, raw = _load(path)
            elements = document["elements"]
            retained = [
                element for element in elements if element.get("visible") is not False
            ]
            removed = len(elements) - len(retained)
            if removed == 0:
                continue

            document_count += 1
            before_elements += len(elements)
            removed_elements += removed
            before_bytes += len(raw)
            document["elements"] = retained
            encoded = _encode(document)
            after_bytes += len(encoded)

            if args.write:
                temporary = path.with_name(path.name + ".prune.tmp")
                if temporary.exists():
                    raise ValueError(f"Stale prune staging file exists: {temporary}")
                with temporary.open("xb") as output:
                    output.write(encoded)
                    output.flush()
                    os.fsync(output.fileno())
                staged_document, staged_raw = _load(temporary)
                if staged_raw != encoded or len(staged_document["elements"]) != len(retained):
                    raise ValueError(f"Staged Effect document verification failed: {path}")
                staged.append((path, temporary))

        print(
            "authored_documents_with_invisible="
            f"{document_count} before_elements={before_elements} "
            f"removed_elements={removed_elements} "
            f"retained_elements={before_elements - removed_elements} "
            f"before_bytes={before_bytes} after_bytes={after_bytes} "
            f"saved_bytes={before_bytes - after_bytes}"
        )

        if args.require_clean and removed_elements:
            return 1
        if args.write:
            for destination, temporary in staged:
                os.replace(temporary, destination)
            staged.clear()
        return 0
    finally:
        for _, temporary in staged:
            temporary.unlink(missing_ok=True)


if __name__ == "__main__":
    raise SystemExit(main())

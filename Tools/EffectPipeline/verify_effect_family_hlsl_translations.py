#!/usr/bin/env python3
"""Verify the checked Effect-family DXBC-to-HLSL translation artifacts.

This gate is deliberately cheap.  It does not disassemble or execute DXBC;
the expensive numeric parity harness owns that proof.  It instead closes the
checked-file contract from the cooked receipt through every translated HLSLI:

* the cooked receipt is a valid, self-hashed v1 artifact;
* its EXTRACTED rows name exactly the checked 169-program denominator;
* the translation report contains exactly one successful row per program;
* every named DXBC and HLSLI file has the recorded raw SHA-256; and
* TranslatedShaders contains no missing or unreported HLSLI file.

Any ambiguity or drift is a hard failure.  In particular, this verifier never
chooses one duplicate report row, tolerates a FAILED row, or silently ignores
an extra translated source file.
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import json
import re
import sys
from pathlib import Path
from typing import Any


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_RECEIPT = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/effect-family-cooked-pixel-shaders.v1.json"
)
DEFAULT_TRANSLATION_REPORT = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/effect-family-hlsl-translations.v1.json"
)
DEFAULT_COOKED_SHADER_DIRECTORY = (
    REPOSITORY_ROOT / "Data/Effects/CookedShaders"
)
DEFAULT_TRANSLATED_SHADER_DIRECTORY = (
    REPOSITORY_ROOT / "Data/Effects/TranslatedShaders"
)

RECEIPT_SCHEMA = "lostark.effect-family-cooked-pixel-shaders"
FORMAT_VERSION = 1
EXPECTED_UNIQUE_EXTRACTED_DIGEST_COUNT = 169
EXTRACTED = "EXTRACTED"
BLOCKED = "BLOCKED"
TRANSLATED = "TRANSLATED"
SHA256_PATTERN = re.compile(r"[0-9a-f]{64}")
IDENTIFIER_PATTERN = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")


class VerificationError(RuntimeError):
    """Raised when any checked translation artifact is not exact."""


def require(condition: bool, message: str) -> None:
    if not condition:
        raise VerificationError(message)


def canonical_sha256(value: Any) -> str:
    payload = json.dumps(
        value, sort_keys=True, separators=(",", ":"), ensure_ascii=False
    )
    return hashlib.sha256(payload.encode("utf-8")).hexdigest()


def raw_sha256(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def read_bytes(path: Path, description: str) -> bytes:
    require(path.is_file(), f"{description} is missing: {path}")
    try:
        return path.read_bytes()
    except OSError as error:
        raise VerificationError(
            f"{description} could not be read: {path}: {error}"
        ) from error


def read_json(path: Path, description: str) -> Any:
    payload = read_bytes(path, description)
    try:
        return json.loads(payload.decode("utf-8"))
    except (UnicodeError, json.JSONDecodeError) as error:
        raise VerificationError(
            f"{description} is not valid UTF-8 JSON: {error}"
        ) from error


def require_sha256(value: Any, description: str) -> str:
    require(
        isinstance(value, str) and SHA256_PATTERN.fullmatch(value) is not None,
        f"{description} is missing or malformed",
    )
    return value


def require_non_negative_integer(value: Any, description: str) -> int:
    require(
        isinstance(value, int) and not isinstance(value, bool) and value >= 0,
        f"{description} must be a non-negative integer",
    )
    return value


def verify_receipt(receipt: Any) -> dict[str, int]:
    require(isinstance(receipt, dict), "cooked receipt root must be an object")
    require(
        receipt.get("schema") == RECEIPT_SCHEMA,
        "cooked receipt schema is not supported",
    )
    require(
        receipt.get("formatVersion") == FORMAT_VERSION,
        "cooked receipt formatVersion is not supported",
    )
    artifact_sha = require_sha256(
        receipt.get("artifactSha256"), "cooked receipt artifactSha256"
    )
    artifact_payload = dict(receipt)
    artifact_payload.pop("artifactSha256", None)
    require(
        canonical_sha256(artifact_payload) == artifact_sha,
        "cooked receipt artifactSha256 drifted",
    )

    families = receipt.get("families")
    require(isinstance(families, list), "cooked receipt families must be an array")
    extracted_rows = 0
    digest_sizes: dict[str, int] = {}
    for offset, row in enumerate(families):
        require(
            isinstance(row, dict),
            f"cooked receipt family {offset} must be an object",
        )
        status = row.get("status")
        require(
            status in {EXTRACTED, BLOCKED},
            f"cooked receipt family {offset} has unknown status: {status}",
        )
        if status != EXTRACTED:
            continue
        extracted_rows += 1
        digest = require_sha256(
            row.get("dxbcSha256"),
            f"cooked receipt EXTRACTED family {offset} dxbcSha256",
        )
        byte_size = require_non_negative_integer(
            row.get("dxbcByteSize"),
            f"cooked receipt EXTRACTED family {offset} dxbcByteSize",
        )
        require(
            byte_size > 0,
            f"cooked receipt EXTRACTED family {offset} has an empty DXBC blob",
        )
        previous_size = digest_sizes.setdefault(digest, byte_size)
        require(
            previous_size == byte_size,
            f"cooked receipt shares DXBC {digest} with conflicting byte sizes",
        )

    require(
        len(digest_sizes) == EXPECTED_UNIQUE_EXTRACTED_DIGEST_COUNT,
        "cooked receipt unique EXTRACTED DXBC denominator must be "
        f"{EXPECTED_UNIQUE_EXTRACTED_DIGEST_COUNT}; got {len(digest_sizes)}",
    )
    summary = receipt.get("summary")
    require(isinstance(summary, dict), "cooked receipt summary must be an object")
    expected_summary = {
        "familyCount": len(families),
        "extractedCount": extracted_rows,
        "blockedCount": len(families) - extracted_rows,
    }
    for key, expected in expected_summary.items():
        require(
            summary.get(key) == expected,
            f"cooked receipt summary.{key} is inconsistent",
        )
    return digest_sizes


def verify_translation_report(
    report: Any,
    expected_digests: set[str],
) -> dict[str, dict[str, str]]:
    require(isinstance(report, list), "translation report root must be an array")
    rows_by_digest: dict[str, dict[str, str]] = {}
    digest_counts: collections.Counter[str] = collections.Counter()
    function_names: set[str] = set()
    casefolded_function_names: set[str] = set()

    for offset, row in enumerate(report):
        require(
            isinstance(row, dict),
            f"translation report row {offset} must be an object",
        )
        status = row.get("status")
        require(
            status == TRANSLATED,
            f"translation report row {offset} is not TRANSLATED: {status}",
        )
        digest = require_sha256(
            row.get("dxbcSha256"),
            f"translation report row {offset} dxbcSha256",
        )
        digest_counts[digest] += 1
        dxbc_name = row.get("dxbc")
        require(
            dxbc_name == f"{digest}.dxbc",
            f"translation report row {offset} DXBC filename does not match its digest",
        )
        function_name = row.get("functionName")
        require(
            isinstance(function_name, str)
            and IDENTIFIER_PATTERN.fullmatch(function_name) is not None,
            f"translation report row {offset} functionName is malformed",
        )
        require(
            function_name not in function_names,
            f"translation report duplicates functionName: {function_name}",
        )
        folded_name = function_name.casefold()
        require(
            folded_name not in casefolded_function_names,
            "translation report function names collide on a case-insensitive "
            f"filesystem: {function_name}",
        )
        function_names.add(function_name)
        casefolded_function_names.add(folded_name)
        hlsl_sha = require_sha256(
            row.get("hlslSha256"),
            f"translation report row {offset} hlslSha256",
        )
        rows_by_digest[digest] = {
            "dxbc": dxbc_name,
            "functionName": function_name,
            "hlslSha256": hlsl_sha,
        }

    duplicate_digests = sorted(
        digest for digest, count in digest_counts.items() if count != 1
    )
    require(
        not duplicate_digests,
        "translation report must contain exactly one row per DXBC digest; "
        f"duplicates: {', '.join(duplicate_digests)}",
    )
    actual_digests = set(rows_by_digest)
    missing = sorted(expected_digests - actual_digests)
    extra = sorted(actual_digests - expected_digests)
    require(
        not missing and not extra,
        "translation report denominator differs from the cooked receipt: "
        f"missing={len(missing)} extra={len(extra)}",
    )
    return rows_by_digest


def verify_dxbc_files(
    rows_by_digest: dict[str, dict[str, str]],
    digest_sizes: dict[str, int],
    cooked_shader_directory: Path,
) -> None:
    require(
        cooked_shader_directory.is_dir(),
        f"cooked shader directory is missing: {cooked_shader_directory}",
    )
    for digest, row in rows_by_digest.items():
        expected_name = f"{digest}.dxbc"
        require(
            row["dxbc"] == expected_name,
            f"DXBC filename differs from its digest: {row['dxbc']}",
        )
        path = cooked_shader_directory / expected_name
        payload = read_bytes(path, f"DXBC artifact {expected_name}")
        require(
            raw_sha256(payload) == digest,
            f"DXBC artifact raw SHA-256 drifted: {expected_name}",
        )
        require(
            len(payload) == digest_sizes[digest],
            f"DXBC artifact byte size drifted: {expected_name}",
        )


def strip_hlsl_comments(source: str) -> str:
    without_blocks = re.sub(r"/\*.*?\*/", "", source, flags=re.DOTALL)
    return re.sub(r"//[^\n]*", "", without_blocks)


def declares_function(source: str, function_name: str) -> bool:
    # Generated sources use either a scalar/vector return type or their
    # function-specific output struct.  Requiring a return type before the
    # symbol distinguishes a declaration/definition from a call site.
    uncommented = strip_hlsl_comments(source)
    pattern = re.compile(
        r"(?m)^[ \t]*(?!(?:return|if|for|while|switch)\b)"
        r"[A-Za-z_][A-Za-z0-9_]*(?:[ \t]*<[^\n>]+>)?[ \t]+"
        + re.escape(function_name)
        + r"[ \t]*\("
    )
    return pattern.search(uncommented) is not None


def hlsli_file_names(directory: Path) -> set[str]:
    require(directory.is_dir(), f"translated shader directory is missing: {directory}")
    try:
        return {
            path.name
            for path in directory.iterdir()
            if path.is_file() and path.suffix.lower() == ".hlsli"
        }
    except OSError as error:
        raise VerificationError(
            f"translated shader directory could not be read: {directory}: {error}"
        ) from error


def verify_hlsli_files(
    rows_by_digest: dict[str, dict[str, str]],
    translated_shader_directory: Path,
) -> None:
    rows_by_file = {
        f"{row['functionName']}.hlsli": row for row in rows_by_digest.values()
    }
    expected_files = set(rows_by_file)
    actual_files = hlsli_file_names(translated_shader_directory)
    missing = sorted(expected_files - actual_files)
    extra = sorted(actual_files - expected_files)
    require(
        not missing and not extra,
        "translated HLSLI file set differs from the report: "
        f"missing={len(missing)} extra={len(extra)}",
    )

    for filename, row in rows_by_file.items():
        path = translated_shader_directory / filename
        payload = read_bytes(path, f"translated HLSLI artifact {filename}")
        require(
            b"\r" not in payload,
            f"translated HLSLI artifact is not LF-only: {filename}",
        )
        require(
            raw_sha256(payload) == row["hlslSha256"],
            f"translated HLSLI artifact raw SHA-256 drifted: {filename}",
        )
        try:
            source = payload.decode("utf-8")
        except UnicodeError as error:
            raise VerificationError(
                f"translated HLSLI artifact is not UTF-8: {filename}: {error}"
            ) from error
        require(
            declares_function(source, row["functionName"]),
            "translated HLSLI artifact does not declare its reported function: "
            f"{filename}",
        )


def verify(
    receipt_path: Path,
    translation_report_path: Path,
    cooked_shader_directory: Path,
    translated_shader_directory: Path,
) -> dict[str, int]:
    receipt = read_json(receipt_path, "cooked receipt")
    digest_sizes = verify_receipt(receipt)
    report = read_json(translation_report_path, "translation report")
    rows_by_digest = verify_translation_report(report, set(digest_sizes))
    verify_dxbc_files(rows_by_digest, digest_sizes, cooked_shader_directory)
    verify_hlsli_files(rows_by_digest, translated_shader_directory)
    return {
        "uniqueExtractedProgramCount": len(digest_sizes),
        "translatedProgramCount": len(rows_by_digest),
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--receipt", type=Path, default=DEFAULT_RECEIPT)
    parser.add_argument(
        "--translation-report", type=Path, default=DEFAULT_TRANSLATION_REPORT
    )
    parser.add_argument(
        "--cooked-shader-directory",
        type=Path,
        default=DEFAULT_COOKED_SHADER_DIRECTORY,
    )
    parser.add_argument(
        "--translated-shader-directory",
        type=Path,
        default=DEFAULT_TRANSLATED_SHADER_DIRECTORY,
    )
    arguments = parser.parse_args(argv)
    try:
        result = verify(
            arguments.receipt.resolve(),
            arguments.translation_report.resolve(),
            arguments.cooked_shader_directory.resolve(),
            arguments.translated_shader_directory.resolve(),
        )
    except VerificationError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print(
        "PASS: Effect-family HLSL translations "
        f"programs={result['translatedProgramCount']} "
        f"uniqueExtracted={result['uniqueExtractedProgramCount']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

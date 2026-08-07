"""Atomically promote the staged DimensionMaster base-11 Effect documents.

The materializer intentionally skips the canonical catalog for selected runs.
This command is the narrow commit boundary for the agreed base variant: eleven
aggregate skills, four basic-attack stage documents, and their full source-
material evidence. ALT_V is never a promotion input and its canonical bytes are
verified before and after commit.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import shutil
import tempfile
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


SCHEMA = "lostark.dimensionmaster-base11-promotion-receipt"
SOURCE_RECEIPT_SCHEMA = (
    "lostark.dimensionmaster-base-effect-materialization-receipt"
)
VARIANT_CONTRACT = "BASE_NO_TIME_OR_SPACE_AXIS"
AGGREGATE_SKILL_IDS = (
    2050010,
    2050100,
    2050120,
    2050160,
    2050180,
    2050210,
    2050220,
    2050230,
    2050240,
    2050500,
    2050520,
)
AGGREGATE_EFFECT_IDS = tuple(
    f"effect.dimensionmaster.skill.{skill_id}"
    for skill_id in AGGREGATE_SKILL_IDS
)
COMBO_STAGE_EFFECT_IDS = tuple(
    f"effect.dimensionmaster.skill.2050010.ba{stage}"
    for stage in range(1, 5)
)
PROMOTED_EFFECT_IDS = AGGREGATE_EFFECT_IDS + COMBO_STAGE_EFFECT_IDS
EXCLUDED_EFFECT_IDS = ("effect.dimensionmaster.skill.2050540",)
SHA256_PATTERN = re.compile(r"^[0-9a-f]{64}$")
ABSOLUTE_PATH_PATTERN = re.compile(
    r"^(?:[A-Za-z]:[\\/]|[/\\]{2}|/|~[/\\])"
)


def sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def sha256_file(path: Path) -> str:
    return sha256_bytes(path.read_bytes())


def load_previous_promoted_hashes(
    receipt_path: Path | None,
) -> dict[str, str]:
    if receipt_path is None or not receipt_path.is_file():
        return {}
    receipt = read_json(receipt_path)
    if receipt.get("schema") != SCHEMA:
        return {}
    if (
        receipt.get("formatVersion") != 1
        or receipt.get("transactionStatus") != "COMMITTED"
    ):
        raise ValueError(
            f"previous promotion receipt is not committed: {receipt_path}"
        )
    rows = receipt.get("documents")
    if not isinstance(rows, list):
        raise ValueError("previous promotion receipt documents are invalid")
    hashes: dict[str, str] = {}
    for index, row in enumerate(rows):
        if not isinstance(row, dict):
            raise ValueError(
                f"previous promotion receipt documents[{index}] is invalid"
            )
        effect_id = row.get("effectAssetId")
        promoted_sha = require_sha256(
            row.get("promotedSha256"),
            f"previous promotion receipt documents[{index}].promotedSha256",
        )
        if effect_id not in PROMOTED_EFFECT_IDS or effect_id in hashes:
            raise ValueError(
                "previous promotion receipt Effect identity is invalid"
            )
        hashes[effect_id] = promoted_sha
    if set(hashes) != set(PROMOTED_EFFECT_IDS):
        raise ValueError("previous promotion receipt Effect set is incomplete")
    return hashes


def read_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as error:
        raise ValueError(f"invalid JSON document: {path}: {error}") from error
    if not isinstance(value, dict):
        raise ValueError(f"JSON root must be an object: {path}")
    return value


def json_bytes(value: dict[str, Any]) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2) + "\n"
    ).encode("utf-8")


def require_int(value: Any, label: str) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise ValueError(f"{label} must be an integer")
    return value


def validate_no_absolute_paths(value: Any, pointer: str = "$") -> None:
    if isinstance(value, dict):
        for key, child in value.items():
            if isinstance(key, str) and ABSOLUTE_PATH_PATTERN.match(key):
                raise ValueError(
                    f"absolute path is forbidden in source evidence: {pointer}"
                )
            validate_no_absolute_paths(child, f"{pointer}/{key}")
    elif isinstance(value, list):
        for index, child in enumerate(value):
            validate_no_absolute_paths(child, f"{pointer}/{index}")
    elif isinstance(value, str) and ABSOLUTE_PATH_PATTERN.match(value):
        raise ValueError(f"absolute path is forbidden in source evidence: {pointer}")


def require_sha256(value: Any, label: str) -> str:
    if not isinstance(value, str) or not SHA256_PATTERN.fullmatch(value):
        raise ValueError(f"{label} must be a lowercase SHA-256")
    return value


def validate_source_evidence(path: Path) -> tuple[bytes, dict[str, Any]]:
    payload = path.read_bytes()
    try:
        evidence = json.loads(payload.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise ValueError(f"invalid source-material evidence: {path}: {error}") from error
    if not isinstance(evidence, dict):
        raise ValueError("source-material evidence root must be an object")
    if evidence.get("schema") != "lostark.effect-source-material-evidence":
        raise ValueError("source-material evidence schema is invalid")
    if require_int(evidence.get("formatVersion"), "evidence formatVersion") != 1:
        raise ValueError("source-material evidence version is invalid")
    if evidence.get("characterClass") != "DIMENSIONMASTER":
        raise ValueError("source-material evidence class is invalid")
    checkpoints = evidence.get("checkpointSkillIds")
    if not isinstance(checkpoints, list) or checkpoints != list(
        AGGREGATE_SKILL_IDS
    ):
        raise ValueError("source-material evidence checkpointSkillIds are invalid")
    validate_no_absolute_paths(evidence)

    parents = evidence.get("parentMaterialEvidence")
    materials = evidence.get("materials")
    if not isinstance(parents, dict) or not parents:
        raise ValueError("source-material parent evidence is empty")
    if not isinstance(materials, dict) or not materials:
        raise ValueError("source-material material evidence is empty")

    for reference, parent in parents.items():
        if not isinstance(reference, str) or not isinstance(parent, dict):
            raise ValueError("source-material parent evidence row is invalid")
        parent_path = parent.get("parentMaterialPath")
        props_file = parent.get("propsFile")
        props_sha = require_sha256(
            parent.get("propsFileSha256"),
            f"parent evidence hash ({reference})",
        )
        if not isinstance(parent_path, str):
            raise ValueError(f"parent material path is invalid: {reference}")
        if not isinstance(props_file, str) or not props_file:
            raise ValueError(f"parent props file is invalid: {reference}")
        if reference != f"{parent_path}@sha256:{props_sha}":
            raise ValueError(f"parent evidence reference/hash mismatch: {reference}")
        if not isinstance(parent.get("materialEvidence"), dict):
            raise ValueError(f"parent Material props payload is invalid: {reference}")

    material_row_count = 0
    parent_linked_row_count = 0
    for material_path, rows in materials.items():
        if not isinstance(material_path, str) or not material_path:
            raise ValueError("source-material identity is invalid")
        if not isinstance(rows, list) or not rows:
            raise ValueError(f"source-material rows are empty: {material_path}")
        for row in rows:
            material_row_count += 1
            if not isinstance(row, dict) or row.get("material_path") != material_path:
                raise ValueError(f"source-material row identity mismatch: {material_path}")
            for hash_name in ("propsFileSha256", "materialEvidencePropsSha256"):
                value = row.get(hash_name)
                if value is not None:
                    require_sha256(value, f"{material_path}.{hash_name}")
            reference = row.get("materialEvidenceRef")
            if reference is None:
                continue
            if not isinstance(reference, str) or reference not in parents:
                raise ValueError(
                    f"source-material parent reference is invalid: {material_path}"
                )
            parent = parents[reference]
            if row.get("materialEvidencePropsSha256") != parent.get(
                "propsFileSha256"
            ) or row.get("materialEvidencePropsFile") != parent.get("propsFile"):
                raise ValueError(
                    f"source-material parent evidence hash mismatch: {material_path}"
                )
            parent_linked_row_count += 1
    if material_row_count <= 0 or parent_linked_row_count <= 0:
        raise ValueError("source-material evidence has no parent-linked material rows")
    return payload, {
        "checkpointSkillIds": list(checkpoints),
        "materialIdentityCount": len(materials),
        "materialRowCount": material_row_count,
        "parentMaterialEvidenceCount": len(parents),
        "parentLinkedMaterialRowCount": parent_linked_row_count,
    }


def validate_source_receipt(
    receipt_path: Path,
) -> tuple[dict[str, Any], dict[str, dict[str, Any]]]:
    receipt = read_json(receipt_path)
    if receipt.get("schema") != SOURCE_RECEIPT_SCHEMA:
        raise ValueError("source materialization receipt schema is invalid")
    if require_int(receipt.get("formatVersion"), "formatVersion") != 1:
        raise ValueError("source materialization receipt version is invalid")
    if receipt.get("variantContract") != VARIANT_CONTRACT:
        raise ValueError("source materialization variant contract is invalid")
    selected = receipt.get("selectedSkillIds")
    if not isinstance(selected, list) or sorted(selected) != list(
        AGGREGATE_SKILL_IDS
    ):
        raise ValueError("source receipt is not the exact base-11 selection")
    if require_int(
        receipt.get("aggregateSkillCount"), "aggregateSkillCount"
    ) != len(AGGREGATE_EFFECT_IDS):
        raise ValueError("source receipt aggregate count is invalid")
    if require_int(
        receipt.get("comboStageDocumentCount"),
        "comboStageDocumentCount",
    ) != len(COMBO_STAGE_EFFECT_IDS):
        raise ValueError("source receipt combo-stage count is invalid")
    if receipt.get("catalogSynchronizationSkippedForSelection") is not True:
        raise ValueError("selected source run unexpectedly synchronized catalog")

    rows = receipt.get("documents")
    if not isinstance(rows, list):
        raise ValueError("source receipt documents must be an array")
    by_id: dict[str, dict[str, Any]] = {}
    for row in rows:
        if not isinstance(row, dict):
            raise ValueError("source receipt document row must be an object")
        effect_id = row.get("effectAssetId")
        if not isinstance(effect_id, str) or not effect_id:
            raise ValueError("source receipt document ID is invalid")
        if effect_id in by_id:
            raise ValueError(f"duplicate source receipt document: {effect_id}")
        by_id[effect_id] = row
    if set(by_id) != set(PROMOTED_EFFECT_IDS):
        missing = sorted(set(PROMOTED_EFFECT_IDS) - set(by_id))
        extra = sorted(set(by_id) - set(PROMOTED_EFFECT_IDS))
        raise ValueError(
            f"source receipt document set mismatch; missing={missing}, extra={extra}"
        )
    for effect_id in AGGREGATE_EFFECT_IDS:
        if by_id[effect_id].get("role") != "AGGREGATE":
            raise ValueError(f"aggregate receipt role is invalid: {effect_id}")
    for effect_id in COMBO_STAGE_EFFECT_IDS:
        if by_id[effect_id].get("role") != "COMBO_STAGE":
            raise ValueError(f"combo-stage receipt role is invalid: {effect_id}")
    return receipt, by_id


def validate_effect_document(
    path: Path,
    expected_effect_id: str,
    receipt_row: dict[str, Any],
) -> tuple[bytes, dict[str, Any]]:
    payload = path.read_bytes()
    try:
        document = json.loads(payload.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise ValueError(f"invalid staged Effect JSON: {path}: {error}") from error
    if not isinstance(document, dict):
        raise ValueError(f"staged Effect root must be an object: {path}")
    if document.get("schema") != "lostark.effect-authoring":
        raise ValueError(f"staged Effect schema is invalid: {expected_effect_id}")
    if require_int(document.get("version"), "Effect version") != 12:
        raise ValueError(f"staged Effect version is invalid: {expected_effect_id}")
    if document.get("effectAssetId") != expected_effect_id:
        raise ValueError(f"staged Effect identity mismatch: {expected_effect_id}")
    elements = document.get("elements")
    model_cues = document.get("modelCues", [])
    if not isinstance(elements, list) or not isinstance(model_cues, list):
        raise ValueError(f"staged Effect arrays are invalid: {expected_effect_id}")
    if require_int(receipt_row.get("elementCount"), "elementCount") != len(
        elements
    ):
        raise ValueError(f"staged Effect element count mismatch: {expected_effect_id}")
    if require_int(receipt_row.get("modelCueCount"), "modelCueCount") != len(
        model_cues
    ):
        raise ValueError(f"staged Effect model-cue count mismatch: {expected_effect_id}")
    receipt_name = Path(str(receipt_row.get("path", ""))).name
    if receipt_name != path.name:
        raise ValueError(f"source receipt path identity mismatch: {expected_effect_id}")
    return payload, document


def collect_transaction(
    stage_root: Path,
    authored_root: Path,
    source_receipt_path: Path,
    canonical_source_receipt_path: Path,
    source_evidence_path: Path,
    canonical_evidence_path: Path,
    previous_promotion_receipt_path: Path | None = None,
) -> dict[str, Any]:
    stage_root = stage_root.resolve()
    authored_root = authored_root.resolve()
    if stage_root == authored_root:
        raise ValueError("stage and canonical Authored roots must be different")
    _, receipt_rows = validate_source_receipt(source_receipt_path)
    source_receipt_path = source_receipt_path.resolve()
    canonical_source_receipt_path = canonical_source_receipt_path.resolve()
    if source_receipt_path == canonical_source_receipt_path:
        raise ValueError(
            "staged and canonical materialization receipts must be different"
        )
    source_receipt_payload = source_receipt_path.read_bytes()
    canonical_source_receipt_existed = canonical_source_receipt_path.is_file()
    canonical_source_receipt_payload = (
        canonical_source_receipt_path.read_bytes()
        if canonical_source_receipt_existed else None
    )
    if canonical_source_receipt_existed:
        validate_source_receipt(canonical_source_receipt_path)
    source_evidence_path = source_evidence_path.resolve()
    canonical_evidence_path = canonical_evidence_path.resolve()
    if source_evidence_path == canonical_evidence_path:
        raise ValueError("staged and canonical source evidence must be different")
    source_evidence_payload, evidence_summary = validate_source_evidence(
        source_evidence_path
    )
    if not canonical_evidence_path.is_file():
        raise ValueError("canonical source-material evidence is missing")
    canonical_evidence_payload = canonical_evidence_path.read_bytes()
    canonical_evidence = read_json(canonical_evidence_path)
    if (
        canonical_evidence.get("schema")
        != "lostark.effect-source-material-evidence"
        or canonical_evidence.get("characterClass") != "DIMENSIONMASTER"
    ):
        raise ValueError("canonical source-material evidence identity is invalid")

    source_files = sorted(stage_root.glob("*.effect.json"))
    source_names = {path.name for path in source_files}
    expected_names = {
        f"{effect_id}.effect.json" for effect_id in PROMOTED_EFFECT_IDS
    }
    if source_names != expected_names:
        missing = sorted(expected_names - source_names)
        extra = sorted(source_names - expected_names)
        raise ValueError(
            f"staged Effect file set mismatch; missing={missing}, extra={extra}"
        )

    previous_promoted_hashes = load_previous_promoted_hashes(
        previous_promotion_receipt_path
    )
    documents: list[dict[str, Any]] = []
    for effect_id in PROMOTED_EFFECT_IDS:
        file_name = f"{effect_id}.effect.json"
        source_path = stage_root / file_name
        destination_path = authored_root / file_name
        if not destination_path.is_file():
            raise ValueError(f"canonical Effect is missing: {effect_id}")
        source_payload, source_document = validate_effect_document(
            source_path, effect_id, receipt_rows[effect_id]
        )
        destination_payload = destination_path.read_bytes()
        destination_document = read_json(destination_path)
        if destination_document.get("effectAssetId") != effect_id:
            raise ValueError(f"canonical Effect identity mismatch: {effect_id}")
        destination_sha = sha256_bytes(destination_payload)
        previous_promoted_sha = previous_promoted_hashes.get(effect_id)
        if (
            previous_promoted_sha is not None
            and destination_sha != previous_promoted_sha
        ):
            raise ValueError(
                "canonical Authored Effect changed after the previous automatic "
                f"promotion; preserve the manual override before promoting: {effect_id}"
            )
        documents.append(
            {
                "effectAssetId": effect_id,
                "role": receipt_rows[effect_id]["role"],
                "fileName": file_name,
                "sourcePath": source_path,
                "destinationPath": destination_path,
                "sourcePayload": source_payload,
                "previousPayload": destination_payload,
                "previousSha256": destination_sha,
                "promotedSha256": sha256_bytes(source_payload),
                "elementCount": len(source_document["elements"]),
                "modelCueCount": len(source_document.get("modelCues", [])),
            }
        )

    preserved: list[dict[str, Any]] = []
    for effect_id in EXCLUDED_EFFECT_IDS:
        path = authored_root / f"{effect_id}.effect.json"
        if not path.is_file():
            raise ValueError(f"excluded canonical Effect is missing: {effect_id}")
        document = read_json(path)
        if document.get("effectAssetId") != effect_id:
            raise ValueError(f"excluded canonical Effect identity mismatch: {effect_id}")
        preserved.append(
            {
                "effectAssetId": effect_id,
                "fileName": path.name,
                "path": path,
                "sha256": sha256_file(path),
            }
        )

    return {
        "stageRoot": stage_root,
        "authoredRoot": authored_root,
        "sourceReceiptPath": source_receipt_path,
        "sourceReceiptSha256": sha256_bytes(source_receipt_payload),
        "sourceReceipt": {
            "sourcePath": source_receipt_path,
            "destinationPath": canonical_source_receipt_path,
            "sourcePayload": source_receipt_payload,
            "previousPayload": canonical_source_receipt_payload,
            "previousSha256": (
                sha256_bytes(canonical_source_receipt_payload)
                if canonical_source_receipt_payload is not None else None
            ),
            "promotedSha256": sha256_bytes(source_receipt_payload),
            "previouslyExisted": canonical_source_receipt_existed,
        },
        "sourceEvidence": {
            "sourcePath": source_evidence_path,
            "destinationPath": canonical_evidence_path,
            "sourcePayload": source_evidence_payload,
            "previousPayload": canonical_evidence_payload,
            "previousSha256": sha256_bytes(canonical_evidence_payload),
            "promotedSha256": sha256_bytes(source_evidence_payload),
            **evidence_summary,
        },
        "documents": documents,
        "preserved": preserved,
    }


def build_receipt(transaction: dict[str, Any]) -> dict[str, Any]:
    evidence = transaction["sourceEvidence"]
    source_receipt = transaction["sourceReceipt"]
    return {
        "schema": SCHEMA,
        "formatVersion": 1,
        "generatedAtUtc": datetime.now(timezone.utc).isoformat(),
        "transactionStatus": "COMMITTED",
        "variantContract": VARIANT_CONTRACT,
        "sourceMaterializationReceiptSha256": transaction[
            "sourceReceiptSha256"
        ],
        "sourceMaterializationReceipt": {
            "fileName": source_receipt["destinationPath"].name,
            "previousSha256": source_receipt["previousSha256"],
            "promotedSha256": source_receipt["promotedSha256"],
            "changed": (
                source_receipt["previousSha256"]
                != source_receipt["promotedSha256"]
            ),
        },
        "aggregateSkillIds": list(AGGREGATE_SKILL_IDS),
        "aggregateEffectCount": len(AGGREGATE_EFFECT_IDS),
        "comboStageEffectCount": len(COMBO_STAGE_EFFECT_IDS),
        "promotedEffectCount": len(PROMOTED_EFFECT_IDS),
        "changedEffectCount": sum(
            row["previousSha256"] != row["promotedSha256"]
            for row in transaction["documents"]
        ),
        "sourceMaterialEvidence": {
            "fileName": evidence["destinationPath"].name,
            "checkpointSkillIds": evidence["checkpointSkillIds"],
            "materialIdentityCount": evidence["materialIdentityCount"],
            "materialRowCount": evidence["materialRowCount"],
            "parentMaterialEvidenceCount": evidence[
                "parentMaterialEvidenceCount"
            ],
            "parentLinkedMaterialRowCount": evidence[
                "parentLinkedMaterialRowCount"
            ],
            "previousSha256": evidence["previousSha256"],
            "promotedSha256": evidence["promotedSha256"],
            "changed": (
                evidence["previousSha256"] != evidence["promotedSha256"]
            ),
        },
        "excludedEffectIds": list(EXCLUDED_EFFECT_IDS),
        "documents": [
            {
                "effectAssetId": row["effectAssetId"],
                "role": row["role"],
                "fileName": row["fileName"],
                "elementCount": row["elementCount"],
                "modelCueCount": row["modelCueCount"],
                "previousSha256": row["previousSha256"],
                "promotedSha256": row["promotedSha256"],
                "changed": row["previousSha256"] != row["promotedSha256"],
            }
            for row in transaction["documents"]
        ],
        "preservedExcludedEffects": [
            {
                "effectAssetId": row["effectAssetId"],
                "fileName": row["fileName"],
                "sha256Before": row["sha256"],
                "sha256After": row["sha256"],
                "unchanged": True,
            }
            for row in transaction["preserved"]
        ],
    }


def validate_unchanged_inputs(transaction: dict[str, Any]) -> None:
    if sha256_file(transaction["sourceReceiptPath"]) != transaction[
        "sourceReceiptSha256"
    ]:
        raise RuntimeError("source materialization receipt changed before commit")
    source_receipt = transaction["sourceReceipt"]
    if source_receipt["previouslyExisted"]:
        if (
            not source_receipt["destinationPath"].is_file()
            or sha256_file(source_receipt["destinationPath"])
            != source_receipt["previousSha256"]
        ):
            raise RuntimeError(
                "canonical materialization receipt changed before commit"
            )
    elif source_receipt["destinationPath"].exists():
        raise RuntimeError(
            "canonical materialization receipt appeared before commit"
        )
    for row in transaction["documents"]:
        if sha256_file(row["sourcePath"]) != row["promotedSha256"]:
            raise RuntimeError(
                f"staged Effect changed before commit: {row['effectAssetId']}"
            )
        if sha256_file(row["destinationPath"]) != row["previousSha256"]:
            raise RuntimeError(
                f"canonical Effect changed before commit: {row['effectAssetId']}"
            )
    evidence = transaction["sourceEvidence"]
    if sha256_file(evidence["sourcePath"]) != evidence["promotedSha256"]:
        raise RuntimeError("staged source-material evidence changed before commit")
    if sha256_file(evidence["destinationPath"]) != evidence["previousSha256"]:
        raise RuntimeError("canonical source-material evidence changed before commit")
    for row in transaction["preserved"]:
        if sha256_file(row["path"]) != row["sha256"]:
            raise RuntimeError(
                f"excluded Effect changed before commit: {row['effectAssetId']}"
            )


def promote(
    transaction: dict[str, Any],
    receipt_path: Path,
    failure_after_promote: int = 0,
) -> dict[str, Any]:
    if failure_after_promote < 0:
        raise ValueError("failure_after_promote must be non-negative")
    validate_unchanged_inputs(transaction)
    receipt_path = receipt_path.resolve()
    protected_paths = {
        row["sourcePath"].resolve() for row in transaction["documents"]
    }
    protected_paths.update(
        row["destinationPath"].resolve()
        for row in transaction["documents"]
    )
    protected_paths.update(
        row["path"].resolve() for row in transaction["preserved"]
    )
    protected_paths.add(transaction["sourceReceiptPath"].resolve())
    source_receipt = transaction["sourceReceipt"]
    protected_paths.add(source_receipt["destinationPath"].resolve())
    evidence = transaction["sourceEvidence"]
    protected_paths.add(evidence["sourcePath"].resolve())
    protected_paths.add(evidence["destinationPath"].resolve())
    if receipt_path in protected_paths:
        raise ValueError("promotion receipt path overlaps a protected input")
    receipt_existed = receipt_path.is_file()
    receipt_previous = receipt_path.read_bytes() if receipt_existed else None
    receipt = build_receipt(transaction)
    authored_root: Path = transaction["authoredRoot"]
    authored_root.mkdir(parents=True, exist_ok=True)
    receipt_path.parent.mkdir(parents=True, exist_ok=True)
    source_receipt["destinationPath"].parent.mkdir(
        parents=True, exist_ok=True
    )
    transaction_root = Path(
        tempfile.mkdtemp(
            prefix=".dimensionmaster-base11-promotion-",
            dir=authored_root,
        )
    )
    staged_root = transaction_root / "staged"
    backup_root = transaction_root / "backup"
    staged_root.mkdir()
    backup_root.mkdir()
    committed: list[dict[str, Any]] = []
    source_receipt_committed = False
    evidence_committed = False
    receipt_committed = False

    try:
        for index, row in enumerate(transaction["documents"]):
            staged_path = staged_root / f"{index:02d}.json"
            backup_path = backup_root / f"{index:02d}.json"
            staged_path.write_bytes(row["sourcePayload"])
            backup_path.write_bytes(row["previousPayload"])
            if sha256_file(staged_path) != row["promotedSha256"]:
                raise RuntimeError(
                    f"staged promotion hash mismatch: {row['effectAssetId']}"
                )
            if sha256_file(backup_path) != row["previousSha256"]:
                raise RuntimeError(
                    f"promotion backup hash mismatch: {row['effectAssetId']}"
                )
        evidence_stage = staged_root / "source-material-evidence.json"
        evidence_backup = backup_root / "source-material-evidence.json"
        evidence_stage.write_bytes(evidence["sourcePayload"])
        evidence_backup.write_bytes(evidence["previousPayload"])
        if sha256_file(evidence_stage) != evidence["promotedSha256"]:
            raise RuntimeError("staged source-material evidence hash mismatch")
        if sha256_file(evidence_backup) != evidence["previousSha256"]:
            raise RuntimeError("source-material evidence backup hash mismatch")
        source_receipt_stage = staged_root / "base11-materialization-receipt.json"
        source_receipt_stage.write_bytes(source_receipt["sourcePayload"])
        source_receipt_backup = backup_root / "base11-materialization-receipt.json"
        if source_receipt["previousPayload"] is not None:
            source_receipt_backup.write_bytes(
                source_receipt["previousPayload"]
            )
        if sha256_file(source_receipt_stage) != source_receipt["promotedSha256"]:
            raise RuntimeError("staged materialization receipt hash mismatch")
        if (
            source_receipt["previousPayload"] is not None
            and sha256_file(source_receipt_backup)
            != source_receipt["previousSha256"]
        ):
            raise RuntimeError("materialization receipt backup hash mismatch")

        validate_unchanged_inputs(transaction)
        promoted_artifact_count = 0
        for index, row in enumerate(transaction["documents"]):
            staged_path = staged_root / f"{index:02d}.json"
            os.replace(staged_path, row["destinationPath"])
            committed.append(row)
            promoted_artifact_count += 1
            if failure_after_promote == promoted_artifact_count:
                raise RuntimeError("injected promotion failure")

        for row in transaction["documents"]:
            if sha256_file(row["destinationPath"]) != row["promotedSha256"]:
                raise RuntimeError(
                    f"promoted Effect hash mismatch: {row['effectAssetId']}"
                )
        os.replace(
            source_receipt_stage, source_receipt["destinationPath"]
        )
        source_receipt_committed = True
        promoted_artifact_count += 1
        if failure_after_promote == promoted_artifact_count:
            raise RuntimeError("injected promotion failure")
        if sha256_file(source_receipt["destinationPath"]) != source_receipt[
            "promotedSha256"
        ]:
            raise RuntimeError("promoted materialization receipt hash mismatch")
        os.replace(evidence_stage, evidence["destinationPath"])
        evidence_committed = True
        promoted_artifact_count += 1
        if failure_after_promote == promoted_artifact_count:
            raise RuntimeError("injected promotion failure")
        if sha256_file(evidence["destinationPath"]) != evidence[
            "promotedSha256"
        ]:
            raise RuntimeError("promoted source-material evidence hash mismatch")
        for row in transaction["preserved"]:
            if sha256_file(row["path"]) != row["sha256"]:
                raise RuntimeError(
                    f"excluded Effect changed during commit: {row['effectAssetId']}"
                )

        receipt_stage = transaction_root / "receipt.json"
        receipt_stage.write_bytes(json_bytes(receipt))
        os.replace(receipt_stage, receipt_path)
        receipt_committed = True
        promoted_artifact_count += 1
        if failure_after_promote == promoted_artifact_count:
            raise RuntimeError("injected promotion failure")
        if read_json(receipt_path).get("transactionStatus") != "COMMITTED":
            raise RuntimeError("promotion receipt verification failed")
        return receipt
    except Exception as commit_error:
        rollback_errors: list[str] = []
        if receipt_committed:
            try:
                if receipt_previous is None:
                    receipt_path.unlink(missing_ok=True)
                else:
                    receipt_restore = transaction_root / "receipt.restore.json"
                    receipt_restore.write_bytes(receipt_previous)
                    os.replace(receipt_restore, receipt_path)
            except Exception as rollback_error:  # pragma: no cover - fatal path
                rollback_errors.append(f"receipt: {rollback_error}")
        if evidence_committed:
            try:
                os.replace(evidence_backup, evidence["destinationPath"])
                if sha256_file(evidence["destinationPath"]) != evidence[
                    "previousSha256"
                ]:
                    raise RuntimeError("restored hash mismatch")
            except Exception as rollback_error:  # pragma: no cover - fatal path
                rollback_errors.append(f"source evidence: {rollback_error}")
        if source_receipt_committed:
            try:
                if source_receipt["previousPayload"] is None:
                    source_receipt["destinationPath"].unlink(missing_ok=True)
                else:
                    os.replace(
                        source_receipt_backup,
                        source_receipt["destinationPath"],
                    )
                    if sha256_file(source_receipt["destinationPath"]) != \
                            source_receipt["previousSha256"]:
                        raise RuntimeError("restored hash mismatch")
            except Exception as rollback_error:  # pragma: no cover - fatal path
                rollback_errors.append(
                    f"materialization receipt: {rollback_error}"
                )
        for row in reversed(committed):
            index = transaction["documents"].index(row)
            backup_path = backup_root / f"{index:02d}.json"
            try:
                os.replace(backup_path, row["destinationPath"])
                if sha256_file(row["destinationPath"]) != row["previousSha256"]:
                    raise RuntimeError("restored hash mismatch")
            except Exception as rollback_error:  # pragma: no cover - fatal path
                rollback_errors.append(
                    f"{row['effectAssetId']}: {rollback_error}"
                )
        if rollback_errors:
            raise RuntimeError(
                f"promotion failed ({commit_error}); rollback failed: "
                + "; ".join(rollback_errors)
            ) from commit_error
        raise
    finally:
        shutil.rmtree(transaction_root, ignore_errors=True)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--stage-root", type=Path, required=True)
    parser.add_argument("--source-receipt", type=Path, required=True)
    parser.add_argument("--source-evidence", type=Path, required=True)
    parser.add_argument(
        "--authored-root",
        type=Path,
        default=Path("Data/Effects/Authored"),
    )
    parser.add_argument(
        "--canonical-source-receipt",
        type=Path,
        default=Path(
            "Data/Effects/Imported/DimensionMaster/"
            "DimensionMaster.base11-materialization.receipt.json"
        ),
    )
    parser.add_argument(
        "--canonical-evidence",
        type=Path,
        default=Path(
            "Data/Effects/Imported/DimensionMaster/ActionSource/"
            "DimensionMaster.source-material-evidence.json"
        ),
    )
    parser.add_argument(
        "--receipt",
        type=Path,
        default=Path(
            "Data/Effects/Imported/DimensionMaster/"
            "DimensionMaster.base11-promotion.receipt.json"
        ),
    )
    parser.add_argument(
        "--mode", choices=("validate", "promote"), default="validate"
    )
    parser.add_argument(
        "--failure-after-promote",
        type=int,
        default=0,
        help=argparse.SUPPRESS,
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    transaction = collect_transaction(
        args.stage_root,
        args.authored_root,
        args.source_receipt,
        args.canonical_source_receipt,
        args.source_evidence,
        args.canonical_evidence,
        args.receipt,
    )
    if args.mode == "promote":
        receipt = promote(
            transaction, args.receipt, args.failure_after_promote
        )
        output = {
            "mode": "promote",
            "promotedEffectCount": receipt["promotedEffectCount"],
            "changedEffectCount": receipt["changedEffectCount"],
            "receipt": str(args.receipt),
        }
    else:
        output = {
            "mode": "validate",
            "promotableEffectCount": len(transaction["documents"]),
            "excludedEffectCount": len(transaction["preserved"]),
            "materialIdentityCount": transaction["sourceEvidence"][
                "materialIdentityCount"
            ],
            "parentMaterialEvidenceCount": transaction["sourceEvidence"][
                "parentMaterialEvidenceCount"
            ],
            "sourceMaterialEvidenceSha256": transaction["sourceEvidence"][
                "promotedSha256"
            ],
            "sourceMaterializationReceiptSha256": transaction[
                "sourceReceiptSha256"
            ],
        }
    print(json.dumps(output, ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

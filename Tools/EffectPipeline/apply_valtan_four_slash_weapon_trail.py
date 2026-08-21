#!/usr/bin/env python3
"""Apply the drawable-proven FourSlash weapon-bone Trail missing-only.

Only the exact canonical FourSlash Effect and this applicator's receipt are in
the write set.  Cue, animation binding, sequence, catalog, renderer, source
safe-gap row, and color-pipeline files are guarded by omission.  A prior
projection is preserved byte-for-byte, including hand-tuned detail fields.
"""

from __future__ import annotations

import argparse
import copy
from dataclasses import dataclass
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import sys
import tempfile
from typing import Any, Mapping


SCRIPT_PATH = Path(__file__).resolve()
DEFAULT_REPOSITORY_ROOT = SCRIPT_PATH.parent.parent.parent
if str(SCRIPT_PATH.parent) not in sys.path:
    sys.path.insert(0, str(SCRIPT_PATH.parent))

import build_valtan_four_slash_weapon_trail_candidate as candidate_builder
import build_valtan_four_slash_weapon_trail_drawable_proof as proof_builder


RECEIPT_SCHEMA = "lostark.valtan-four-slash-weapon-trail-application-receipt"
FORMAT_VERSION = 1
RECEIPT_RELATIVE_PATH = candidate_builder.OUTPUT_DIRECTORY_RELATIVE_PATH / (
    PurePosixPath("Valtan.four-slash-weapon-trail.application-receipt.v1.json")
)
RECEIPT_SCHEMA_RELATIVE_PATH = PurePosixPath(
    "Tools/EffectPipeline/Schemas/"
    "lostark.valtan-four-slash-weapon-trail-application-receipt.schema.json"
)


class ApplicationError(RuntimeError):
    """The missing-only transaction cannot be proven safe."""


class SourceRebaseRequired(ApplicationError):
    """A sealed input or protected projected contract changed."""


@dataclass(frozen=True)
class Projection:
    repository_root: Path
    outputs: Mapping[PurePosixPath, bytes]
    guards: Mapping[PurePosixPath, bytes | None]
    changed_paths: tuple[PurePosixPath, ...]
    already_applied: bool
    receipt: dict[str, Any]


def canonical_sha256(value: Any) -> str:
    return proof_builder.canonical_sha256(value)


def raw_sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def raw_sha256(path: Path) -> str:
    return raw_sha256_bytes(path.read_bytes())


def json_bytes_like(source: bytes, value: Any) -> bytes:
    text = json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
    if b"\r\n" in source:
        text = text.replace("\n", "\r\n")
    return text.encode("utf-8")


def pretty_bytes(value: Any) -> bytes:
    return (json.dumps(value, ensure_ascii=False, indent=2) + "\n").encode(
        "utf-8"
    )


def read_json_bytes(path: Path) -> tuple[dict[str, Any], bytes]:
    try:
        payload = path.read_bytes()
        if payload.startswith(b"\xef\xbb\xbf"):
            raise ApplicationError(f"JSON must be UTF-8 without BOM: {path}")

        def no_duplicates(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
            result: dict[str, Any] = {}
            for key, value in pairs:
                if key in result:
                    raise ApplicationError(
                        f"duplicate JSON property {key!r}: {path}"
                    )
                result[key] = value
            return result

        value = json.loads(
            payload.decode("utf-8"), object_pairs_hook=no_duplicates
        )
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise ApplicationError(f"cannot read JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise ApplicationError(f"JSON root must be an object: {path}")
    return value, payload


def repository_path(root: Path, relative: PurePosixPath) -> Path:
    resolved_root = root.resolve()
    result = root.joinpath(*relative.parts).resolve()
    if result != resolved_root and resolved_root not in result.parents:
        raise ApplicationError(f"repository path escaped root: {relative}")
    return result


def relative_from_text(value: Any, label: str) -> PurePosixPath:
    if not isinstance(value, str) or not value or "\\" in value:
        raise ApplicationError(f"{label} must be a repository-relative POSIX path")
    relative = PurePosixPath(value)
    if relative.is_absolute() or ".." in relative.parts or "." in relative.parts:
        raise ApplicationError(f"{label} escaped repository")
    return relative


def protected_contract(element: dict[str, Any]) -> dict[str, Any]:
    return {
        "id": element.get("id"),
        "sourceNode": element.get("sourceNode"),
        "kind": element.get("kind"),
        "resources": element.get("resources"),
        "material": element.get("material"),
        "actionCueAttachment": element.get("actionCueAttachment"),
        "transformInheritance": element.get("transformInheritance"),
        "sourceRecipe": element.get("sourceRecipe"),
        "sourcePresentation": element.get("sourcePresentation"),
    }


def _validate_receipt(
    receipt: dict[str, Any],
    *,
    manifest_relative: PurePosixPath,
    manifest_payload: bytes,
    manifest_artifact: str,
    proof_relative: PurePosixPath,
    proof_payload: bytes,
    proof_artifact: str,
    candidate_relative: PurePosixPath,
    candidate_payload: bytes,
    candidate_element: dict[str, Any],
    canonical_relative: PurePosixPath,
    source_element: dict[str, Any],
) -> None:
    if (
        receipt.get("schema") != RECEIPT_SCHEMA
        or receipt.get("formatVersion") != FORMAT_VERSION
        or receipt.get("bossArchetypeId") != "BOSS_VALTAN"
        or receipt.get("transactionStatus") != "COMMITTED"
        or receipt.get("reconcileMode") != "MISSING_ONLY"
    ):
        raise SourceRebaseRequired("FourSlash application receipt identity changed")
    try:
        proof_builder.verify_seal(
            receipt, "artifactSha256", "FourSlash application receipt"
        )
    except proof_builder.ProofError as error:
        raise SourceRebaseRequired(str(error)) from error
    manifest_identity = receipt.get("candidateManifest") or {}
    proof_identity = receipt.get("drawableProof") or {}
    candidate_identity = receipt.get("candidateDocument") or {}
    target = receipt.get("target") or {}
    source = receipt.get("sourcePreservation") or {}
    projection = receipt.get("projection") or {}
    boundary = receipt.get("writeBoundary") or {}
    if (
        manifest_identity
        != {
            "path": manifest_relative.as_posix(),
            "rawSha256": raw_sha256_bytes(manifest_payload),
            "artifactSha256": manifest_artifact,
        }
        or proof_identity
        != {
            "path": proof_relative.as_posix(),
            "rawSha256": raw_sha256_bytes(proof_payload),
            "artifactSha256": proof_artifact,
        }
        or candidate_identity
        != {
            "path": candidate_relative.as_posix(),
            "rawSha256": raw_sha256_bytes(candidate_payload),
            "elementId": candidate_builder.CANDIDATE_ELEMENT_ID,
            "elementSha256": canonical_sha256(candidate_element),
        }
        or target.get("canonicalPath") != canonical_relative.as_posix()
        or target.get("effectAssetId")
        != candidate_builder.SOURCE_EFFECT_ASSET_ID
        or target.get("patternId") != "VALTAN_FOUR_SLASH"
        or target.get("stageId") != "SLASHES"
        or source
        != {
            "sourceElementId": candidate_builder.SOURCE_ELEMENT_ID,
            "sourceElementSha256": canonical_sha256(source_element),
            "sourceElementAttachmentSha256": canonical_sha256(
                source_element["actionCueAttachment"]
            ),
            "preservedRowCount": 1,
        }
        or projection.get("candidateElementId")
        != candidate_builder.CANDIDATE_ELEMENT_ID
        or projection.get("candidateSourceNode")
        != candidate_builder.CANDIDATE_SOURCE_NODE
        or projection.get("initialElementSha256")
        != canonical_sha256(candidate_element)
        or projection.get("protectedContractSha256")
        != canonical_sha256(protected_contract(candidate_element))
        or boundary
        != {
            "canonicalAuthoringPath": canonical_relative.as_posix(),
            "receiptPath": RECEIPT_RELATIVE_PATH.as_posix(),
            "canonicalOutputCount": 1,
            "cueBindingMutationPerformed": False,
            "catalogMutationPerformed": False,
            "sequenceMutationPerformed": False,
            "rendererMutationPerformed": False,
            "colorPipelineMutationPerformed": False,
        }
    ):
        raise SourceRebaseRequired(
            "FourSlash application receipt requires source rebase"
        )


def _make_receipt(
    *,
    manifest_relative: PurePosixPath,
    manifest_payload: bytes,
    manifest: dict[str, Any],
    proof_relative: PurePosixPath,
    proof_payload: bytes,
    proof: dict[str, Any],
    candidate_relative: PurePosixPath,
    candidate_payload: bytes,
    candidate_element: dict[str, Any],
    canonical_relative: PurePosixPath,
    source_element: dict[str, Any],
    pre_apply_payload: bytes,
    post_apply_payload: bytes,
    inserted_index: int,
) -> dict[str, Any]:
    receipt = {
        "schema": RECEIPT_SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "bossArchetypeId": "BOSS_VALTAN",
        "transactionStatus": "COMMITTED",
        "reconcileMode": "MISSING_ONLY",
        "candidateManifest": {
            "path": manifest_relative.as_posix(),
            "rawSha256": raw_sha256_bytes(manifest_payload),
            "artifactSha256": manifest["artifactSha256"],
        },
        "drawableProof": {
            "path": proof_relative.as_posix(),
            "rawSha256": raw_sha256_bytes(proof_payload),
            "artifactSha256": proof["artifactSha256"],
        },
        "candidateDocument": {
            "path": candidate_relative.as_posix(),
            "rawSha256": raw_sha256_bytes(candidate_payload),
            "elementId": candidate_builder.CANDIDATE_ELEMENT_ID,
            "elementSha256": canonical_sha256(candidate_element),
        },
        "target": {
            "patternId": "VALTAN_FOUR_SLASH",
            "stageId": "SLASHES",
            "effectAssetId": candidate_builder.SOURCE_EFFECT_ASSET_ID,
            "canonicalPath": canonical_relative.as_posix(),
        },
        "sourcePreservation": {
            "sourceElementId": candidate_builder.SOURCE_ELEMENT_ID,
            "sourceElementSha256": canonical_sha256(source_element),
            "sourceElementAttachmentSha256": canonical_sha256(
                source_element["actionCueAttachment"]
            ),
            "preservedRowCount": 1,
        },
        "projection": {
            "candidateElementId": candidate_builder.CANDIDATE_ELEMENT_ID,
            "candidateSourceNode": candidate_builder.CANDIDATE_SOURCE_NODE,
            "initialElementSha256": canonical_sha256(candidate_element),
            "protectedContractSha256": canonical_sha256(
                protected_contract(candidate_element)
            ),
            "insertedElementIndex": inserted_index,
            "projectedElementCount": 1,
        },
        "canonicalApply": {
            "preApplyRawSha256": raw_sha256_bytes(pre_apply_payload),
            "postApplyRawSha256": raw_sha256_bytes(post_apply_payload),
        },
        "writeBoundary": {
            "canonicalAuthoringPath": canonical_relative.as_posix(),
            "receiptPath": RECEIPT_RELATIVE_PATH.as_posix(),
            "canonicalOutputCount": 1,
            "cueBindingMutationPerformed": False,
            "catalogMutationPerformed": False,
            "sequenceMutationPerformed": False,
            "rendererMutationPerformed": False,
            "colorPipelineMutationPerformed": False,
        },
    }
    proof_builder.seal(receipt, "artifactSha256")
    return receipt


def collect_projection(
    repository_root: Path,
    *,
    manifest_path: Path | None = None,
    drawable_proof_path: Path | None = None,
    receipt_path: Path | None = None,
) -> Projection:
    root = repository_root.resolve()
    default_manifest_relative = (
        candidate_builder.OUTPUT_DIRECTORY_RELATIVE_PATH
        / candidate_builder.MANIFEST_FILENAME
    )
    manifest_file = manifest_path or repository_path(
        root, default_manifest_relative
    )
    proof_file = drawable_proof_path or repository_path(
        root, proof_builder.PROOF_RELATIVE_PATH
    )
    receipt_file = receipt_path or repository_path(root, RECEIPT_RELATIVE_PATH)
    try:
        manifest_relative = PurePosixPath(
            manifest_file.resolve().relative_to(root).as_posix()
        )
        proof_relative = PurePosixPath(
            proof_file.resolve().relative_to(root).as_posix()
        )
        receipt_relative = PurePosixPath(
            receipt_file.resolve().relative_to(root).as_posix()
        )
    except ValueError as error:
        raise ApplicationError("projection inputs must stay inside repository") from error
    if receipt_relative != RECEIPT_RELATIVE_PATH:
        raise ApplicationError("FourSlash application receipt path is not canonical")

    manifest, manifest_payload = read_json_bytes(manifest_file)
    proof, proof_payload = read_json_bytes(proof_file)
    try:
        proof_builder.verify_seal(
            manifest, "artifactSha256", "FourSlash candidate manifest"
        )
        proof_builder.validate_proof(proof)
    except proof_builder.ProofError as error:
        raise SourceRebaseRequired(str(error)) from error
    if (
        proof.get("candidateManifest")
        != {
            "path": manifest_relative.as_posix(),
            "rawSha256": raw_sha256_bytes(manifest_payload),
            "artifactSha256": manifest.get("artifactSha256"),
        }
    ):
        raise SourceRebaseRequired("FourSlash proof/manifest join is stale")

    target = manifest.get("target") or {}
    candidate_relative = relative_from_text(
        target.get("candidatePath"), "candidatePath"
    )
    canonical_relative = relative_from_text(
        target.get("canonicalPath"), "canonicalPath"
    )
    candidate_file = repository_path(root, candidate_relative)
    canonical_file = repository_path(root, canonical_relative)
    candidate, candidate_payload = read_json_bytes(candidate_file)
    canonical, canonical_payload = read_json_bytes(canonical_file)
    candidate_rows = candidate.get("elements") or []
    if len(candidate_rows) != 1 or not isinstance(candidate_rows[0], dict):
        raise SourceRebaseRequired("FourSlash candidate denominator changed")
    candidate_element = candidate_rows[0]
    candidate_identity = manifest.get("candidateIdentity") or {}
    if (
        candidate.get("effectAssetId")
        != candidate_builder.SOURCE_EFFECT_ASSET_ID
        or raw_sha256_bytes(candidate_payload)
        != candidate_identity.get("rawSha256")
        or canonical_sha256(candidate)
        != candidate_identity.get("canonicalSha256")
        or canonical_sha256(candidate_element)
        != candidate_identity.get("elementSha256")
        or proof.get("candidateDocument")
        != {
            "path": candidate_relative.as_posix(),
            "rawSha256": raw_sha256_bytes(candidate_payload),
            "canonicalSha256": canonical_sha256(candidate),
            "elementId": candidate_builder.CANDIDATE_ELEMENT_ID,
            "elementSha256": canonical_sha256(candidate_element),
        }
    ):
        raise SourceRebaseRequired("FourSlash candidate/proof identity changed")
    if (
        canonical.get("schema") != "lostark.effect-authoring"
        or canonical.get("version") != 13
        or canonical.get("effectAssetId")
        != candidate_builder.SOURCE_EFFECT_ASSET_ID
        or not isinstance(canonical.get("elements"), list)
    ):
        raise SourceRebaseRequired("FourSlash canonical document identity changed")
    try:
        source_element = candidate_builder._source_element(canonical)
    except candidate_builder.CandidateError as error:
        raise SourceRebaseRequired(str(error)) from error
    source_proof = proof.get("sourcePreservation") or {}
    if (
        source_proof.get("canonicalPath") != canonical_relative.as_posix()
        or source_proof.get("sourceElementId")
        != candidate_builder.SOURCE_ELEMENT_ID
        or source_proof.get("sourceElementSha256")
        != canonical_sha256(source_element)
        or source_proof.get("sourceElementAttachmentSha256")
        != canonical_sha256(source_element["actionCueAttachment"])
        or source_proof.get("preservedRowCount") != 1
    ):
        raise SourceRebaseRequired("FourSlash safe-gap preservation proof changed")

    elements: list[dict[str, Any]] = canonical["elements"]
    id_matches = [
        row
        for row in elements
        if isinstance(row, dict)
        and row.get("id") == candidate_builder.CANDIDATE_ELEMENT_ID
    ]
    node_matches = [
        row
        for row in elements
        if isinstance(row, dict)
        and row.get("sourceNode") == candidate_builder.CANDIDATE_SOURCE_NODE
    ]
    slot_matches = [
        row
        for row in elements
        if isinstance(row, dict)
        and (row.get("actionCueAttachment") or {}).get("runtimeAnchorSlotId")
        == candidate_builder.RUNTIME_ANCHOR_SLOT_ID
    ]
    receipt_exists = receipt_file.is_file()
    if not id_matches and not node_matches and not slot_matches:
        if receipt_exists:
            raise SourceRebaseRequired(
                "FourSlash projected row was deleted after committed apply"
            )
        input_identity = manifest.get("inputIdentity") or {}
        if (
            raw_sha256_bytes(canonical_payload)
            != input_identity.get("canonicalDocumentRawSha256")
            or canonical_sha256(canonical)
            != input_identity.get("canonicalDocumentCanonicalSha256")
            or source_proof.get("canonicalRawSha256")
            != raw_sha256_bytes(canonical_payload)
        ):
            raise SourceRebaseRequired(
                "FourSlash canonical source changed before first apply"
            )
        staged = copy.deepcopy(canonical)
        inserted_index = len(staged["elements"])
        staged["elements"].append(copy.deepcopy(candidate_element))
        canonical_output = json_bytes_like(canonical_payload, staged)
        receipt = _make_receipt(
            manifest_relative=manifest_relative,
            manifest_payload=manifest_payload,
            manifest=manifest,
            proof_relative=proof_relative,
            proof_payload=proof_payload,
            proof=proof,
            candidate_relative=candidate_relative,
            candidate_payload=candidate_payload,
            candidate_element=candidate_element,
            canonical_relative=canonical_relative,
            source_element=source_element,
            pre_apply_payload=canonical_payload,
            post_apply_payload=canonical_output,
            inserted_index=inserted_index,
        )
        outputs = {
            canonical_relative: canonical_output,
            RECEIPT_RELATIVE_PATH: pretty_bytes(receipt),
        }
        guards = {
            canonical_relative: canonical_payload,
            RECEIPT_RELATIVE_PATH: None,
        }
        return Projection(
            root,
            outputs,
            guards,
            (canonical_relative, RECEIPT_RELATIVE_PATH),
            False,
            receipt,
        )

    if (
        len(id_matches) != 1
        or len(node_matches) != 1
        or len(slot_matches) != 1
        or id_matches[0] is not node_matches[0]
        or id_matches[0] is not slot_matches[0]
    ):
        raise SourceRebaseRequired(
            "FourSlash candidate stable identity collided in canonical data"
        )
    if not receipt_exists:
        raise SourceRebaseRequired(
            "FourSlash candidate row exists without its committed receipt"
        )
    receipt, receipt_payload = read_json_bytes(receipt_file)
    _validate_receipt(
        receipt,
        manifest_relative=manifest_relative,
        manifest_payload=manifest_payload,
        manifest_artifact=manifest["artifactSha256"],
        proof_relative=proof_relative,
        proof_payload=proof_payload,
        proof_artifact=proof["artifactSha256"],
        candidate_relative=candidate_relative,
        candidate_payload=candidate_payload,
        candidate_element=candidate_element,
        canonical_relative=canonical_relative,
        source_element=source_element,
    )
    projected = id_matches[0]
    if canonical_sha256(protected_contract(projected)) != canonical_sha256(
        protected_contract(candidate_element)
    ):
        raise SourceRebaseRequired(
            "FourSlash projected immutable resource/material/anchor contract drifted"
        )
    return Projection(
        root,
        {},
        {
            canonical_relative: canonical_payload,
            RECEIPT_RELATIVE_PATH: receipt_payload,
        },
        (),
        True,
        receipt,
    )


def commit_projection(
    projection: Projection, *, failure_after_promote: int | None = None
) -> None:
    if not projection.outputs:
        return
    root = projection.repository_root
    for relative, expected in projection.guards.items():
        path = repository_path(root, relative)
        actual = path.read_bytes() if path.is_file() else None
        if actual != expected:
            raise ApplicationError(
                f"FourSlash transaction guard changed before commit: {relative}"
            )

    staged: dict[PurePosixPath, Path] = {}
    backups: dict[PurePosixPath, bytes | None] = {}
    promoted: list[PurePosixPath] = []
    try:
        for relative, payload in projection.outputs.items():
            target = repository_path(root, relative)
            target.parent.mkdir(parents=True, exist_ok=True)
            descriptor, temporary_name = tempfile.mkstemp(
                prefix=target.name + ".staging.", dir=target.parent
            )
            temporary = Path(temporary_name)
            with os.fdopen(descriptor, "wb") as stream:
                stream.write(payload)
                stream.flush()
                os.fsync(stream.fileno())
            staged[relative] = temporary
            backups[relative] = target.read_bytes() if target.is_file() else None
        for index, relative in enumerate(projection.outputs, start=1):
            target = repository_path(root, relative)
            os.replace(staged[relative], target)
            promoted.append(relative)
            if failure_after_promote is not None and index >= failure_after_promote:
                raise OSError("injected transaction failure")
    except OSError as error:
        for relative in reversed(promoted):
            target = repository_path(root, relative)
            previous = backups[relative]
            if previous is None:
                target.unlink(missing_ok=True)
            else:
                target.write_bytes(previous)
        raise ApplicationError(
            f"FourSlash application transaction rolled back: {error}"
        ) from error
    finally:
        for temporary in staged.values():
            temporary.unlink(missing_ok=True)


def check_projection(projection: Projection) -> None:
    if not projection.already_applied or projection.changed_paths:
        raise ApplicationError("FourSlash weapon Trail projection is not applied")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--dry-run", action="store_true")
    mode.add_argument("--apply", action="store_true")
    mode.add_argument("--check", action="store_true")
    parser.add_argument(
        "--repository-root", type=Path, default=DEFAULT_REPOSITORY_ROOT
    )
    parser.add_argument("--manifest", type=Path)
    parser.add_argument("--drawable-proof", type=Path)
    parser.add_argument("--receipt", type=Path)
    arguments = parser.parse_args(argv)
    try:
        projection = collect_projection(
            arguments.repository_root,
            manifest_path=arguments.manifest,
            drawable_proof_path=arguments.drawable_proof,
            receipt_path=arguments.receipt,
        )
        if arguments.apply:
            commit_projection(projection)
            projection = collect_projection(
                arguments.repository_root,
                manifest_path=arguments.manifest,
                drawable_proof_path=arguments.drawable_proof,
                receipt_path=arguments.receipt,
            )
            check_projection(projection)
            label = "APPLY"
        elif arguments.check:
            check_projection(projection)
            label = "CHECK"
        else:
            label = "DRY_RUN"
        print(
            f"FourSlash weapon-bone Trail {label}: "
            f"changed={len(projection.changed_paths)} "
            f"alreadyApplied={str(projection.already_applied).lower()}"
        )
        return 0
    except (ApplicationError, OSError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())

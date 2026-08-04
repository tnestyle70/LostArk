#!/usr/bin/env python3
"""Summarize extracted effect resources and derived authoring admission gaps."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path


def read_diagnostic(path: Path | None) -> str:
    if path is None or not path.is_file():
        return ""
    payload = path.read_bytes()
    for encoding in ("utf-8-sig", "utf-16", "cp949"):
        try:
            return payload.decode(encoding)
        except UnicodeDecodeError:
            pass
    return payload.decode("utf-8", "replace")


def first_int(text: str, pattern: str) -> int | None:
    match = re.search(pattern, text, re.IGNORECASE | re.DOTALL)
    return int(match.group(1)) if match else None


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--intake-manifest", type=Path, required=True)
    parser.add_argument("--export-receipt", type=Path, required=True)
    parser.add_argument("--candidate-catalog", type=Path, required=True)
    parser.add_argument("--authoring-dir", type=Path, required=True)
    parser.add_argument("--converter-diagnostic", type=Path, default=None)
    parser.add_argument("--resource-root", required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    intake = json.loads(args.intake_manifest.read_text(encoding="utf-8"))
    receipt = json.loads(args.export_receipt.read_text(encoding="utf-8"))
    catalog = json.loads(args.candidate_catalog.read_text(encoding="utf-8"))
    diagnostic = read_diagnostic(args.converter_diagnostic)
    effect_files = sorted(args.authoring_dir.glob("*.effect"))
    emitter_count = enabled_emitters = required_count = textured_required = 0
    for path in effect_files:
        for line in path.read_text(encoding="utf-8").splitlines():
            if line.startswith("EMITTER "):
                emitter_count += 1
                enabled_emitters += " enabled=1 " in f" {line} "
            if " type=REQUIRED " in f" {line} ":
                required_count += 1
                textured_required += ' tex=""' not in line

    summary = intake.get("summary", {})
    blockers = []
    if summary.get("unresolvedModuleRefCount"):
        blockers.append("external Cascade module imports are recorded but not materialized")
    if summary.get("missingMaterialCount"):
        blockers.append("source material references remain unresolved")
    if enabled_emitters != emitter_count:
        blockers.append("derived authoring contains disabled emitters")
    if "skipped modules the tool cannot represent:" in diagnostic:
        blockers.append("Effect Tool schema does not represent every Cascade module class")

    result = {
        "schema": "lostark.effect-authoring-admission",
        "formatVersion": 1,
        "classId": "dimensionmaster",
        "status": "candidate_only" if blockers else "admitted",
        "sourcePackages": [f"FX_PC_SWP_{index:02d}" for index in range(6)],
        "resourceRoot": args.resource_root,
        "candidateCatalog": args.candidate_catalog.as_posix(),
        "authoringRoot": args.authoring_dir.as_posix(),
        "summary": {
            "particleSystemCandidateCount": catalog.get("count", len(catalog.get("rows", []))),
            "effectFileCount": len(effect_files),
            "emitterCount": emitter_count,
            "enabledEmitterCount": enabled_emitters,
            "disabledEmitterCount": emitter_count - enabled_emitters,
            "requiredModuleCount": required_count,
            "texturedRequiredModuleCount": textured_required,
            "runtimeTextureCount": receipt.get("summary", {}).get("textureCount"),
            "runtimeMeshCount": receipt.get("summary", {}).get("meshCount"),
            "runtimeExportFailureCount": receipt.get("summary", {}).get("failureCount"),
            "unresolvedModuleRefCount": summary.get("unresolvedModuleRefCount"),
            "unresolvedModuleRefAffectedParticleSystemCount": summary.get("unresolvedModuleRefAffectedParticleSystemCount"),
            "missingMaterialCount": summary.get("missingMaterialCount"),
            "missingMaterialAffectedParticleSystemCount": summary.get("missingMaterialAffectedParticleSystemCount"),
            "missingMaterialOccurrenceCount": summary.get("missingMaterialOccurrenceCount"),
            "emptyLodEmitterCount": summary.get("emptyLodEmitterCount"),
            "externalImportDisabledEmitterCount": first_int(
                diagnostic,
                r"disabled\s+(\d+)\s+emitter\(s\)\s+with\s+\d+\s+external",
            ),
            "untexturedMaterialCount": first_int(
                diagnostic, r"(\d+)\s+material\(s\)\s+had no texture"
            ),
        },
        "missingMaterials": intake.get("missingMaterials", []),
        "blockingReasons": blockers,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(json.dumps({"output": str(args.output), **result["summary"]}))
    return 0 if result["status"] == "admitted" else 1


if __name__ == "__main__":
    raise SystemExit(main())

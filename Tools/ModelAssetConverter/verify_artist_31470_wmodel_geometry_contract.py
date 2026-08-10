#!/usr/bin/env python3
"""Build and verify the seven Artist 31470 mesh carriers in a temp directory."""

from __future__ import annotations

import argparse
import json
import subprocess
import tempfile
from pathlib import Path
from typing import Any

from cook_wmodel_geometry_contract import (
    cook_wmodel_geometry_contract,
    load_json_object,
    load_geometry_provenance_evidence,
    verify_source_against_geometry_contract,
)


ASSETS = (
    ("FX_SM_01", "fm_v_wp_wsdm_base_01"),
    ("FX_SM_01", "fm_m_trail_002"),
    ("FX_SM_00", "fm_h_swing_03"),
    ("FX_SM_00", "fm_h_swing_05"),
    ("FX_SM_00", "fm_h_swing_01"),
    ("FX_SM_00", "fm_o_swing_02"),
    ("FX_SM_00", "fm_a_stone_001"),
)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def verify_artist_31470(
    source_root: Path,
    runtime_mesh_root: Path,
    source_manifest_path: Path,
    source_export_receipt_path: Path,
    legacy_cook_receipt_path: Path,
    source_package_root: Path,
    legacy_converter_path: Path,
    decoder_harness_path: Path,
) -> dict[str, Any]:
    manifest, _ = load_json_object(source_manifest_path, "source manifest")
    manifest_assets = {
        str(row.get("sourceAssetPath")): row
        for row in manifest.get("assets") or []
        if isinstance(row, dict)
    }
    rows: list[dict[str, Any]] = []
    with tempfile.TemporaryDirectory(prefix="artist-31470-wmodel-v11-") as directory:
        temporary_root = Path(directory)
        for package, name in ASSETS:
            source_object = f"{package.lower()}.{name}"
            manifest_row = manifest_assets.get(source_object)
            require(manifest_row is not None, f"source manifest misses {source_object}")
            source_gltf = source_root / package / "StaticMesh3" / f"{name}.gltf"
            legacy_wmodel = runtime_mesh_root / f"{name}.wmodel"
            source_package = source_package_root / str(
                manifest_row.get("physicalPackage", "")
            )
            provenance, expected_gltf, expected_wmodel = load_geometry_provenance_evidence(
                source_object,
                source_gltf,
                legacy_wmodel,
                source_manifest_path,
                source_export_receipt_path,
                legacy_cook_receipt_path,
                source_package,
                legacy_converter_path,
            )
            output, cook_receipt = cook_wmodel_geometry_contract(
                source_gltf,
                legacy_wmodel,
                provenance,
                100.0,
                0.01,
                expected_gltf,
                expected_wmodel,
            )
            candidate = temporary_root / f"{name}.wmodel"
            candidate.write_bytes(output)
            oracle = verify_source_against_geometry_contract(source_gltf, candidate)
            decoder_gate = subprocess.run(
                [str(decoder_harness_path.resolve()), "--candidate", str(candidate)],
                check=False,
                capture_output=True,
                text=True,
            )
            require(
                decoder_gate.returncode == 0,
                f"C++ WModelDecoder rejected {name}: "
                + (decoder_gate.stdout + decoder_gate.stderr).strip(),
            )
            require(
                not cook_receipt["runtimeProductAdmission"]
                and not oracle["productAdmission"],
                f"geometry-only cook may not admit {name}",
            )
            require(
                oracle["sourceToWModelScale"] == 100.0
                and abs(oracle["geometryPreScale"] - 0.01) <= 1e-9,
                f"Artist geometry scale contract differs for {name}",
            )
            rows.append(
                {
                    "sourceObject": source_object,
                    "runtimeAssetId": f"Effect/Artist/Meshes/{name}.wmodel",
                    "legacyWModelSha256": cook_receipt["legacyWModelSha256"],
                    "candidateWModelSha256": cook_receipt["outputSha256"],
                    "hasColor0": cook_receipt["geometry"]["hasColor0"],
                    "productBlockers": cook_receipt["runtimeProductBlockers"],
                    "cppDecoderGate": "SELF_CONSISTENT_UNAUTHENTICATED_PASS",
                    "oracle": oracle,
                }
            )

    require(len(rows) == 7, "Artist geometry verifier did not cover seven carriers")
    require(
        sum(bool(row["hasColor0"]) for row in rows) == 2,
        "Artist geometry verifier expected COLOR_0 on exactly two carriers",
    )
    require(
        sum(bool(row["oracle"]["asymmetricBoundsFixture"]) for row in rows) >= 1,
        "Artist geometry verifier has no asymmetric pivot/bounds fixture",
    )
    failed_numeric_rows = [
        row
        for row in rows
        if not (
            row["oracle"]["bitangentBasisStatus"] == "PROVEN_NUMERIC"
            and row["oracle"]["maximumBitangentBasisError"] <= 2e-6
            and row["oracle"]["maximumPositionErrorAfterGeometryPreScale"]
            <= row["oracle"][
                "maximumPositionFloat32ErrorBoundAfterGeometryPreScale"
            ]
            and row["oracle"]["maximumBoundsErrorAfterGeometryPreScale"]
            <= row["oracle"][
                "maximumBoundsFloat32ErrorBoundAfterGeometryPreScale"
            ]
            and not row["oracle"]["recenterApplied"]
            and row["oracle"]["pivotStatus"]
            == "UPK_TO_GLTF_PIVOT_UNRESOLVED"
        )
    ]
    require(
        not failed_numeric_rows,
        "Artist geometry numeric oracle failed: "
        + json.dumps(failed_numeric_rows, ensure_ascii=False, sort_keys=True),
    )
    negative_source_rows = sum(
        int(row["oracle"]["sourceTangentWCounts"].get("-1.0", 0)) > 0
        for row in rows
    )
    require(
        negative_source_rows == 4,
        "Artist geometry verifier expected four mixed/negative tangent-W carriers",
    )
    return {
        "schema": "lostark.artist-31470-wmodel-geometry-contract-verification",
        "formatVersion": 1,
        "characterClass": "Artist",
        "skillId": 31470,
        "carrierCount": len(rows),
        "color0CarrierCount": sum(bool(row["hasColor0"]) for row in rows),
        "negativeSourceTangentWCarrierCount": negative_source_rows,
        "asymmetricBoundsFixtureCount": sum(
            bool(row["oracle"]["asymmetricBoundsFixture"]) for row in rows
        ),
        "selfConsistencyVerifiedCount": len(rows),
        "externallyAuthenticatedPayloadIntegrityCount": 0,
        "cppDecoderCandidateGateCount": len(rows),
        "sourceFidelityClosedCount": 0,
        "runtimeGeometryPreScaleConsumedCount": 0,
        "color0ShaderConsumedCount": 0,
        "numericOracle": {
            "errorModel": (
                "float32 cook half-ULP plus stored geometryPreScale error "
                "from exact reciprocal"
            ),
            "maximumObservedPositionErrorAfterGeometryPreScale": max(
                row["oracle"]["maximumPositionErrorAfterGeometryPreScale"]
                for row in rows
            ),
            "maximumPositionFloat32ErrorBoundAfterGeometryPreScale": max(
                row["oracle"][
                    "maximumPositionFloat32ErrorBoundAfterGeometryPreScale"
                ]
                for row in rows
            ),
            "maximumObservedBoundsErrorAfterGeometryPreScale": max(
                row["oracle"]["maximumBoundsErrorAfterGeometryPreScale"]
                for row in rows
            ),
            "maximumBoundsFloat32ErrorBoundAfterGeometryPreScale": max(
                row["oracle"][
                    "maximumBoundsFloat32ErrorBoundAfterGeometryPreScale"
                ]
                for row in rows
            ),
        },
        "productAdmission": False,
        "rows": rows,
    }


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Verify temporary WModel 1.1 cooks for all Artist 31470 carriers"
    )
    parser.add_argument("--source-root", required=True, type=Path)
    parser.add_argument("--runtime-mesh-root", required=True, type=Path)
    parser.add_argument("--source-manifest", required=True, type=Path)
    parser.add_argument("--source-export-receipt", required=True, type=Path)
    parser.add_argument("--legacy-cook-receipt", required=True, type=Path)
    parser.add_argument("--source-package-root", required=True, type=Path)
    parser.add_argument("--legacy-converter", required=True, type=Path)
    parser.add_argument("--decoder-harness", required=True, type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    receipt = verify_artist_31470(
        args.source_root,
        args.runtime_mesh_root,
        args.source_manifest,
        args.source_export_receipt,
        args.legacy_cook_receipt,
        args.source_package_root,
        args.legacy_converter,
        args.decoder_harness,
    )
    content = json.dumps(receipt, ensure_ascii=False, indent=2) + "\n"
    if args.output is not None:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(content, encoding="utf-8", newline="\n")
    print(
        "Artist 31470 WModel geometry contract: "
        f"carriers={receipt['carrierCount']} color0={receipt['color0CarrierCount']} "
        f"negativeW={receipt['negativeSourceTangentWCarrierCount']} "
        "selfConsistency=7 externallyAuthenticated=0 cppDecoder=7 "
        "sourceFidelity=0 preScaleConsumed=0 product=false"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

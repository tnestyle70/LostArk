"""Contract tests for the material-family shader evidence inventory.

The inventory is intentionally stricter than a reporting script. It accepts
only a complete, self-hashed and transitively pinned evidence chain, then keeps
program identity, named-lane identity, runtime ABI closure and Product
admission as four different claims.
"""

from __future__ import annotations

import hashlib
import io
import json
import os
import shutil
import tempfile
import unittest
from contextlib import redirect_stderr, redirect_stdout
from unittest import mock

from Tools.EffectPipeline import build_effect_family_shader_inventory as inventory


DXBC_A = b"DXBC-program-a"
DXBC_B = b"DXBC-program-b"
DXBC_C = b"DXBC-program-c"
DIGEST_A = hashlib.sha256(DXBC_A).hexdigest()
DIGEST_B = hashlib.sha256(DXBC_B).hexdigest()
DIGEST_C = hashlib.sha256(DXBC_C).hexdigest()
DXBC_PAYLOADS = {DIGEST_A: DXBC_A, DIGEST_B: DXBC_B, DIGEST_C: DXBC_C}


def _write(root: str, relative_path: str, payload: object) -> dict:
    absolute_path = os.path.join(root, relative_path)
    os.makedirs(os.path.dirname(absolute_path), exist_ok=True)
    raw = (json.dumps(payload, ensure_ascii=False, indent=2) + "\n").encode("utf-8")
    with open(absolute_path, "wb") as handle:
        handle.write(raw)
    return {"rawSha256": hashlib.sha256(raw).hexdigest(), "byteSize": len(raw)}


def _artifact(schema: str, families: list, summary: dict, inputs: dict) -> dict:
    document = {
        "schema": schema,
        "formatVersion": inventory.FORMAT_VERSION,
        "inputs": inputs,
        "summary": summary,
        "families": families,
    }
    document["artifactSha256"] = inventory._canonical_sha256(document)
    return document


def _rewrite_artifact(root: str, relative_path: str, mutate) -> None:
    absolute_path = os.path.join(root, relative_path)
    with open(absolute_path, "r", encoding="utf-8") as handle:
        document = json.load(handle)
    mutate(document)
    document.pop("artifactSha256", None)
    document["artifactSha256"] = inventory._canonical_sha256(document)
    _write(root, relative_path, document)


def _element(
    parent_material_path: object,
    enabled: bool = True,
    *,
    source_material_path: str = "pkg.mi.child",
    carrier: str = "mesh",
) -> dict:
    profile: dict = {"enabled": enabled}
    if parent_material_path is not None:
        profile["parentMaterialPath"] = parent_material_path
    return {
        "id": "e",
        "kind": "particle",
        "material": {
            "sourceMaterialPath": source_material_path,
            "sourceProfile": profile,
        },
        "sourceRecipe": {"rendererShape": carrier},
    }


def _shader_map_row(path: str, resolved_by: str, cooked_evidence: str) -> dict:
    return {
        "parentMaterialPath": path,
        "occurrenceCount": 1,
        "resolution": {"resolvedBy": resolved_by},
        "cookedEvidence": cooked_evidence,
    }


def _cooked_extracted(path: str, digest: str) -> dict:
    return {
        "parentMaterialPath": path,
        "status": inventory.COOKED_EXTRACTED,
        "occurrenceCount": 1,
        "carrier": "mesh",
        "childMaterialPath": "pkg.mi.child",
        "permutationSelection": "CHILD_MIC_ENGINE_EQUALITY",
        "permutationCount": 2,
        "dxbcSha256": digest,
        "dxbcByteSize": len(DXBC_PAYLOADS[digest]),
    }


def _child_resolved(
    child: str,
    known_family: str | None,
    *,
    cooked_status: str = inventory.COOKED_EXTRACTED,
    named_closed: bool = True,
) -> dict:
    row = {
        "childMaterialPath": child,
        "elementCount": 1,
        "effectAssetIdCount": 1,
        "status": "RESOLVED",
        "resolvedBy": "DECLARED_PACKAGE_EXPORT",
        "parentChain": [known_family or "new.pkg.parent"],
        "parentMaterialPath": known_family or "new.pkg.parent",
        "canonicalParentMaterialPath": known_family or "new.pkg.parent",
        "parentDeclaringPackage": "pkg",
        "knownFamilyPath": known_family,
        "cookedPixelShaderStatus": cooked_status,
        "namedAbiClosed": named_closed,
        "familyAlreadyInDenominator": known_family is not None,
    }
    row["rowSha256"] = inventory._canonical_sha256(row)
    return row


def _child_blocked(child: str) -> dict:
    row = {
        "childMaterialPath": child,
        "elementCount": 1,
        "effectAssetIdCount": 1,
        "status": "BLOCKED",
        "blocker": "LEAF_ABSENT_IN_EVERY_STAGED_PACKAGE",
        "parentChain": [],
        "unresolvedPath": child,
    }
    row["rowSha256"] = inventory._canonical_sha256(row)
    return row


def _cooked_blocked(path: str, blocker: str) -> dict:
    return {
        "parentMaterialPath": path,
        "status": inventory.COOKED_BLOCKED,
        "occurrenceCount": 1,
        "blocker": blocker,
    }


def _named(path: str, digest: str) -> dict:
    return {
        "parentMaterialPath": path,
        "admits": inventory.NAMED_MAPPING_ONLY,
        "dxbcSha256": digest,
        "instructionCount": 78,
        "summary": {
            "scalarLaneCount": 18,
            "vectorLaneCount": 13,
            "timeDependentRegisters": ["cb0[7]"],
        },
        "textureSlots": [],
    }


def _named_blocked(path: str, blocker: str) -> dict:
    return {
        "parentMaterialPath": path,
        "status": inventory.COOKED_BLOCKED,
        "blocker": {"reasonCode": blocker, "candidateCount": 2},
    }


def _hlsl_source(function_name: str) -> bytes:
    return ("float4 " + function_name + "() { return 0.0.xxxx; }\n").encode("utf-8")


def _translation(digest: str, suffix: str) -> dict:
    function_name = "Shade_Ue3_" + suffix
    return {
        "dxbc": digest + ".dxbc",
        "status": inventory.TRANSLATED,
        "functionName": function_name,
        "instructionCount": 78,
        "dxbcSha256": digest,
        "hlslSha256": hashlib.sha256(_hlsl_source(function_name)).hexdigest(),
    }


class FamilyShaderInventoryTest(unittest.TestCase):
    def setUp(self) -> None:
        self.root = tempfile.mkdtemp(prefix="family-inventory-")
        self.addCleanup(shutil.rmtree, self.root, True)

    def _publish(
        self,
        elements: list,
        shader_map: list,
        cooked: list,
        abi: list,
        translations: list,
        child_rows: list | None = None,
    ) -> None:
        _write(
            self.root,
            inventory.AUTHORED_DIRECTORY + "/doc.effect.json",
            {"elements": elements},
        )

        shader_document = _artifact(
            inventory.SHADER_MAP_SCHEMA,
            shader_map,
            {"parentMaterialCount": len(shader_map)},
            {},
        )
        shader_identity = _write(
            self.root, inventory.SHADER_MAP_INDEX_PATH, shader_document
        )

        extracted = [
            row for row in cooked if row.get("status") == inventory.COOKED_EXTRACTED
        ]
        cooked_document = _artifact(
            inventory.COOKED_PIXEL_SHADERS_SCHEMA,
            cooked,
            {
                "familyCount": len(cooked),
                "extractedCount": len(extracted),
                "blockedCount": len(cooked) - len(extracted),
            },
            {
                "shaderMapArtifactSha256": shader_document["artifactSha256"],
                "shaderMapRawSha256": shader_identity["rawSha256"],
                "shaderMapByteSize": shader_identity["byteSize"],
            },
        )
        cooked_identity = _write(
            self.root, inventory.COOKED_PIXEL_SHADERS_PATH, cooked_document
        )

        _write(self.root, inventory.HLSL_TRANSLATIONS_PATH, translations)

        cooked_directory = os.path.join(self.root, inventory.COOKED_SHADER_DIRECTORY)
        translated_directory = os.path.join(
            self.root, inventory.TRANSLATED_SHADER_DIRECTORY
        )
        os.makedirs(cooked_directory, exist_ok=True)
        os.makedirs(translated_directory, exist_ok=True)
        for row in extracted:
            digest = row["dxbcSha256"]
            with open(os.path.join(cooked_directory, digest + ".dxbc"), "wb") as handle:
                handle.write(DXBC_PAYLOADS[digest])
        for row in translations:
            function_name = row["functionName"]
            with open(
                os.path.join(translated_directory, function_name + ".hlsli"), "wb"
            ) as handle:
                handle.write(_hlsl_source(function_name))

        digest_sizes = {
            row["dxbcSha256"]: row["dxbcByteSize"] for row in extracted
        }
        cooked_identities = {
            digest: {"rawSha256": digest, "byteSize": size}
            for digest, size in digest_sizes.items()
        }
        resolved = sum(
            row.get("admits") == inventory.NAMED_MAPPING_ONLY for row in abi
        )
        blocked = sum(row.get("status") == inventory.COOKED_BLOCKED for row in abi)
        blocked_parents = sorted(
            row["parentMaterialPath"]
            for row in abi
            if row.get("status") == inventory.COOKED_BLOCKED
        )
        abi_document = _artifact(
            inventory.NAMED_ABI_SCHEMA,
            abi,
            {
                "familyCount": len(abi),
                "resolvedNamedMappingCount": resolved,
                "blockedCount": blocked,
                "blockedParents": blocked_parents,
                "blockedParentSetSha256": inventory._canonical_sha256(
                    blocked_parents
                ),
            },
            {
                "shaderMapArtifactSha256": shader_document["artifactSha256"],
                "shaderMapRawSha256": shader_identity["rawSha256"],
                "shaderMapByteSize": shader_identity["byteSize"],
                "cookedPixelShadersArtifactSha256": cooked_document[
                    "artifactSha256"
                ],
                "cookedPixelShadersRawSha256": cooked_identity["rawSha256"],
                "cookedPixelShadersByteSize": cooked_identity["byteSize"],
                "cookedShaderProgramCount": len(digest_sizes),
                "cookedShaderSetSha256": inventory._canonical_sha256(
                    cooked_identities
                ),
            },
        )
        abi_identity = _write(self.root, inventory.NAMED_ABI_PATH, abi_document)

        children = list(child_rows or [])
        recovered_parent_counts: dict[str, int] = {}
        recovered_parent_evidence: dict[str, tuple] = {}
        resolved_by_counts: dict[str, int] = {}
        blocker_counts: dict[str, int] = {}
        for row in children:
            if row.get("status") == "RESOLVED":
                parent = row["canonicalParentMaterialPath"]
                recovered_parent_counts[parent] = (
                    recovered_parent_counts.get(parent, 0) + row["elementCount"]
                )
                recovered_parent_evidence.setdefault(
                    parent,
                    (
                        row.get("knownFamilyPath"),
                        row.get("cookedPixelShaderStatus"),
                        row.get("namedAbiClosed"),
                        row.get("familyAlreadyInDenominator"),
                    ),
                )
                resolved_by = row["resolvedBy"]
                resolved_by_counts[resolved_by] = (
                    resolved_by_counts.get(resolved_by, 0) + 1
                )
            else:
                blocker = row["blocker"]
                blocker_counts[blocker] = blocker_counts.get(blocker, 0) + 1
        child_families = []
        for parent in sorted(recovered_parent_counts):
            known, cooked_status, named_closed, already = recovered_parent_evidence[
                parent
            ]
            family = {
                "canonicalParentMaterialPath": parent,
                "knownFamilyPath": known,
                "recoveredElementCount": recovered_parent_counts[parent],
                "cookedPixelShaderStatus": cooked_status,
                "namedAbiClosed": named_closed,
                "alreadyInDenominator": already,
            }
            family["rowSha256"] = inventory._canonical_sha256(family)
            child_families.append(family)
        orphan_element_count = sum(row["elementCount"] for row in children)
        recovered_element_count = sum(recovered_parent_counts.values())
        child_document = {
            "schema": inventory.CHILD_PARENT_RESOLUTION_SCHEMA,
            "formatVersion": inventory.FORMAT_VERSION,
            "identity": {
                "scope": "AUTHORED_CORPUS_ORPHAN_CHILD_MATERIALS",
                "admits": "PARENT_MATERIAL_JOIN_KEY_ONLY",
            },
            "inputs": {
                "authoredDirectory": inventory.AUTHORED_DIRECTORY,
                "authoredDocumentCount": 1,
                "authoredDocumentSetSha256": "1" * 64,
                "sourcePackManifest": "fixture-manifest.json",
                "sourcePackManifestRawSha256": "2" * 64,
                "sourcePackManifestByteSize": 1,
                "sourcePackageCount": 1,
                "sourcePackageSetSha256": "3" * 64,
                "sourcePackageByteSize": 1,
                "cookedPixelShaders": inventory.COOKED_PIXEL_SHADERS_PATH,
                "cookedPixelShadersArtifactSha256": cooked_document[
                    "artifactSha256"
                ],
                "cookedPixelShadersRawSha256": cooked_identity["rawSha256"],
                "cookedPixelShadersByteSize": cooked_identity["byteSize"],
                "namedAbi": inventory.NAMED_ABI_PATH,
                "namedAbiArtifactSha256": abi_document["artifactSha256"],
                "namedAbiRawSha256": abi_identity["rawSha256"],
                "namedAbiByteSize": abi_identity["byteSize"],
                "packagesOpened": 0,
                "leafSearchSweeps": 0,
            },
            "summary": {
                "authoredElementCount": orphan_element_count,
                "parentRetainedElementCount": 0,
                "parentLostElementCount": orphan_element_count,
                "parentLostWithChildPathElementCount": orphan_element_count,
                "parentLostWithoutChildPathElementCount": 0,
                "distinctOrphanChildCount": len(children),
                "resolvedChildCount": sum(
                    row.get("status") == "RESOLVED" for row in children
                ),
                "resolvedByCounts": dict(sorted(resolved_by_counts.items())),
                "blockedChildCount": sum(
                    row.get("status") == "BLOCKED" for row in children
                ),
                "recoveredElementCount": recovered_element_count,
                "recoveredParentMaterialCount": len(child_families),
                "recoveredParentsAlreadyInDenominator": sum(
                    row["alreadyInDenominator"] for row in child_families
                ),
                "recoveredParentsNewToDenominator": sum(
                    not row["alreadyInDenominator"] for row in child_families
                ),
                "recoveredElementsWithExtractedDxbc": sum(
                    row["recoveredElementCount"]
                    for row in child_families
                    if row["cookedPixelShaderStatus"] == inventory.COOKED_EXTRACTED
                ),
                "blockerCounts": dict(sorted(blocker_counts.items())),
            },
            "families": child_families,
            "children": children,
        }
        child_document["artifactSha256"] = inventory._canonical_sha256(
            child_document
        )
        _write(
            self.root,
            inventory.CHILD_PARENT_RESOLUTION_PATH,
            child_document,
        )

    def _statuses(self) -> dict:
        artifact = inventory.build_inventory(self.root, ["doc.effect.json"])
        return {row["parentMaterialPath"]: row for row in artifact["families"]}

    def test_literal_program_and_named_mapping_do_not_claim_runtime_closure(self) -> None:
        parent = "pkg.grp.fx_a_tr"
        self._publish(
            [_element(parent)],
            [_shader_map_row(parent, "DECLARED_PACKAGE_EXPORT", inventory.COOKED_PRESENT)],
            [_cooked_extracted(parent, DIGEST_A)],
            [_named(parent, DIGEST_A)],
            [_translation(DIGEST_A, "fx_a_tr")],
        )
        row = self._statuses()[parent]
        self.assertEqual(row["status"], inventory.STATUS_PROGRAM_EXACT_NAMED_ONLY)
        self.assertEqual(
            row["programEvidence"], "OCCURRENCE_EXACT_LITERAL_TRANSLATION"
        )
        self.assertEqual(row["namedMappingEvidence"], "NAMED_LANE_IDENTITY_ONLY")
        self.assertEqual(row["runtimeAbiClosure"], "NOT_PROVEN")
        self.assertEqual(row["productAdmission"], "NOT_ADMITTED")
        self.assertIn("runtime ABI packet", row["blocker"])

    def test_blocked_named_mapping_keeps_exact_program_as_separate_evidence(self) -> None:
        parent = "pkg.grp.fx_b_ad"
        blocker = "NATIVE_BINDING_ARRAY_CANDIDATE_AMBIGUOUS"
        self._publish(
            [_element(parent)],
            [_shader_map_row(parent, "DECLARED_PACKAGE_EXPORT", inventory.COOKED_PRESENT)],
            [_cooked_extracted(parent, DIGEST_B)],
            [_named_blocked(parent, blocker)],
            [_translation(DIGEST_B, "fx_b_ad")],
        )
        row = self._statuses()[parent]
        self.assertEqual(row["status"], inventory.STATUS_PROGRAM_EXACT_NAMING_MISSING)
        self.assertEqual(
            row["programEvidence"], "OCCURRENCE_EXACT_LITERAL_TRANSLATION"
        )
        self.assertEqual(row["namedMappingEvidence"], "MISSING_OR_BLOCKED")
        self.assertEqual(row["blocker"], blocker + " candidateCount=2")

    def test_classify_cooked_program_without_translation_aborts(self) -> None:
        with self.assertRaisesRegex(
            inventory.InventoryError, "has no literal translation"
        ):
            inventory._classify(
                "pkg.grp.fx_c_tr",
                "pkg.mi.child",
                "mesh",
                "AUTHORED_PARENT_EXACT",
                _shader_map_row(
                    "pkg.grp.fx_c_tr",
                    "DECLARED_PACKAGE_EXPORT",
                    inventory.COOKED_PRESENT,
                ),
                _cooked_extracted("pkg.grp.fx_c_tr", DIGEST_C),
                _named("pkg.grp.fx_c_tr", DIGEST_C),
                {},
            )

    def test_blocked_cooked_extraction_is_dxbc_missing(self) -> None:
        parent = "pkg.grp.fx_d_ad"
        blocker = "structural pixel pass reference is ambiguous"
        self._publish(
            [_element(parent)],
            [_shader_map_row(parent, "DECLARED_PACKAGE_EXPORT", inventory.COOKED_PRESENT)],
            [_cooked_blocked(parent, blocker)],
            [],
            [],
        )
        row = self._statuses()[parent]
        self.assertEqual(row["status"], inventory.STATUS_DXBC_MISSING)
        self.assertEqual(row["blocker"], blocker)

    def test_unresolved_base_material_is_parent_only(self) -> None:
        parent = "pkg.grp.fx_e_tr"
        self._publish(
            [_element(parent)],
            [_shader_map_row(parent, "PARENT_MATERIAL_EXPORT_ABSENT", "BASE_MATERIAL_ID_UNRESOLVED")],
            [],
            [],
            [],
        )
        row = self._statuses()[parent]
        self.assertEqual(row["status"], inventory.STATUS_PARENT_ONLY)
        self.assertIn("PARENT_MATERIAL_EXPORT_ABSENT", row["blocker"])

    def test_path_absent_from_shader_map_is_unknown(self) -> None:
        self._publish([_element("pkg.grp.fx_f_tr")], [], [], [], [])
        row = self._statuses()["pkg.grp.fx_f_tr"]
        self.assertEqual(row["status"], inventory.STATUS_UNKNOWN)

    def test_missing_parent_path_is_kept_as_unknown_row(self) -> None:
        self._publish([_element(None)], [], [], [], [])
        row = self._statuses()[""]
        self.assertEqual(row["status"], inventory.STATUS_UNKNOWN)
        self.assertEqual(row["occurrenceCount"], 1)

    def test_shared_leaf_across_packages_stays_two_rows(self) -> None:
        first = "pkg_a.grp.fx_same_ad"
        second = "pkg_b.grp.fx_same_ad"
        self._publish(
            [_element(first), _element(second)],
            [
                _shader_map_row(first, "DECLARED_PACKAGE_EXPORT", inventory.COOKED_PRESENT),
                _shader_map_row(second, "LEAF_NAME_AMBIGUOUS", "BASE_MATERIAL_ID_UNRESOLVED"),
            ],
            [_cooked_extracted(first, DIGEST_A)],
            [_named(first, DIGEST_A)],
            [_translation(DIGEST_A, "fx_same_ad")],
        )
        rows = self._statuses()
        self.assertEqual(len(rows), 2)
        self.assertEqual(rows[first]["status"], inventory.STATUS_PROGRAM_EXACT_NAMED_ONLY)
        self.assertEqual(rows[second]["status"], inventory.STATUS_PARENT_ONLY)

    def test_execution_authored_rows_are_not_counted_as_source_rows(self) -> None:
        element = {
            "id": "e",
            "kind": "particle",
            "material": {
                "sourceProfile": {"enabled": False, "parentMaterialPath": "pkg.grp.fx_g_tr"},
                "execution": {"enabled": True, "backend": "runtimeMaterialV2", "opcode": 9},
            },
        }
        self._publish([element], [], [], [], [])
        artifact = inventory.build_inventory(self.root, ["doc.effect.json"])
        self.assertEqual(artifact["families"], [])
        summary = artifact["summary"]["documents"][0]
        self.assertEqual(summary["sourceRowCount"], 0)
        self.assertEqual(summary["executionRowCount"], 1)
        self.assertEqual(summary["runtimeAbiClosedRowCount"], 0)

    def test_source_and_execution_cannot_be_enabled_together(self) -> None:
        element = _element("pkg.grp.fx_conflict_tr")
        element["material"]["execution"] = {"enabled": True}
        self._publish([element], [], [], [], [])
        with self.assertRaisesRegex(inventory.InventoryError, "together"):
            inventory.build_inventory(self.root, ["doc.effect.json"])

    def test_child_and_carrier_mismatch_keep_representative_program_pending(self) -> None:
        parent = "pkg.grp.fx_pending_tr"
        self._publish(
            [
                _element(parent, source_material_path="pkg.mi.other"),
                _element(parent, source_material_path="pkg.mi.child", carrier="sprite"),
            ],
            [_shader_map_row(parent, "DECLARED_PACKAGE_EXPORT", inventory.COOKED_PRESENT)],
            [_cooked_extracted(parent, DIGEST_A)],
            [_named(parent, DIGEST_A)],
            [_translation(DIGEST_A, "pending")],
        )
        row = self._statuses()[parent]
        self.assertEqual(row["status"], inventory.STATUS_PROGRAM_PENDING_NAMED_ONLY)
        self.assertEqual(row["programExactOccurrenceCount"], 0)
        self.assertEqual(row["programPermutationPendingOccurrenceCount"], 2)
        self.assertIn("representative", row["programEvidence"].lower())

    def test_single_permutation_still_requires_matching_carrier(self) -> None:
        parent = "pkg.grp.fx_single_tr"
        cooked = _cooked_extracted(parent, DIGEST_A)
        cooked["permutationSelection"] = "SINGLE_PERMUTATION_FAMILY"
        cooked["permutationCount"] = 1
        cooked["childMaterialPath"] = None
        self._publish(
            [_element(parent, carrier="sprite")],
            [_shader_map_row(parent, "DECLARED_PACKAGE_EXPORT", inventory.COOKED_PRESENT)],
            [cooked],
            [_named(parent, DIGEST_A)],
            [_translation(DIGEST_A, "single")],
        )
        row = self._statuses()[parent]
        self.assertEqual(row["status"], inventory.STATUS_PROGRAM_PENDING_NAMED_ONLY)
        self.assertIn("carrier", row["blocker"])

    def test_exact_child_receipt_resolves_known_parent_without_leaf_fallback(self) -> None:
        parent = "pkg.grp.fx_known_tr"
        child = "pkg.mi.child"
        self._publish(
            [_element("wrong.authored.parent", source_material_path=child)],
            [_shader_map_row(parent, "DECLARED_PACKAGE_EXPORT", inventory.COOKED_PRESENT)],
            [_cooked_extracted(parent, DIGEST_A)],
            [_named(parent, DIGEST_A)],
            [_translation(DIGEST_A, "known")],
            [_child_resolved(child, parent)],
        )
        row = self._statuses()[parent]
        self.assertEqual(row["status"], inventory.STATUS_PROGRAM_EXACT_NAMED_ONLY)
        self.assertEqual(
            row["parentResolutions"][0]["resolvedBy"],
            "CHILD_PARENT_KNOWN_FAMILY_EXACT",
        )

    def test_two_exact_child_aliases_join_one_single_permutation_family(self) -> None:
        parent = "pkg.grp.fx_lens_ad"
        first_child = "pkg.mi.lens_03"
        second_child = "pkg.mi.lens_05"
        first = _element("short.fx_lens_ad", source_material_path=first_child)
        second = _element(parent, source_material_path=second_child)
        second["id"] = "e2"
        cooked = _cooked_extracted(parent, DIGEST_A)
        cooked["permutationSelection"] = "SINGLE_PERMUTATION_FAMILY"
        cooked["permutationCount"] = 1
        cooked["childMaterialPath"] = None
        self._publish(
            [first, second],
            [_shader_map_row(parent, "DECLARED_PACKAGE_EXPORT", inventory.COOKED_PRESENT)],
            [cooked],
            [_named(parent, DIGEST_A)],
            [_translation(DIGEST_A, "lens")],
            [
                _child_resolved(first_child, parent),
                _child_resolved(second_child, parent),
            ],
        )
        artifact = inventory.build_inventory(self.root, ["doc.effect.json"])
        self.assertEqual(len(artifact["families"]), 1)
        row = artifact["families"][0]
        self.assertEqual(row["occurrenceCount"], 2)
        self.assertEqual(row["programExactOccurrenceCount"], 2)

    def test_blocked_child_receipt_does_not_remap_authored_parent(self) -> None:
        parent = "pkg.mi.child"
        self._publish(
            [_element(parent, source_material_path=parent)],
            [_shader_map_row(parent, "PARENT_MATERIAL_EXPORT_ABSENT", "BASE_MATERIAL_ID_UNRESOLVED")],
            [],
            [],
            [],
            [_child_blocked(parent)],
        )
        row = self._statuses()[parent]
        self.assertEqual(row["status"], inventory.STATUS_PARENT_ONLY)
        self.assertEqual(row["parentResolutions"][0]["resolvedBy"], "AUTHORED_PARENT_EXACT")

    def test_resolved_new_family_is_not_silently_unknown(self) -> None:
        child = "new.pkg.mi"
        self._publish(
            [_element(None, source_material_path=child)],
            [],
            [],
            [],
            [],
            [
                _child_resolved(
                    child, None, cooked_status="ABSENT", named_closed=False
                )
            ],
        )
        row = self._statuses()["new.pkg.parent"]
        self.assertEqual(
            row["status"], inventory.STATUS_PARENT_RESOLVED_PROGRAM_MISSING
        )

    def test_mixed_family_preserves_exact_and_pending_occurrence_counts(self) -> None:
        parent = "pkg.grp.fx_mixed_tr"
        exact = _element(parent, source_material_path="pkg.mi.child")
        pending = _element(parent, source_material_path="pkg.mi.other")
        pending["id"] = "e2"
        self._publish(
            [exact, pending],
            [_shader_map_row(parent, "DECLARED_PACKAGE_EXPORT", inventory.COOKED_PRESENT)],
            [_cooked_extracted(parent, DIGEST_A)],
            [_named(parent, DIGEST_A)],
            [_translation(DIGEST_A, "mixed")],
        )
        row = self._statuses()[parent]
        self.assertEqual(row["status"], inventory.STATUS_MIXED)
        self.assertEqual(row["programExactOccurrenceCount"], 1)
        self.assertEqual(row["programPermutationPendingOccurrenceCount"], 1)
        self.assertEqual(len(row["evidenceClasses"]), 2)

    def test_absent_input_contract_aborts_before_writing(self) -> None:
        self._publish([_element("pkg.grp.fx_h_tr")], [], [], [], [])
        os.remove(os.path.join(self.root, inventory.NAMED_ABI_PATH))
        with self.assertRaises(inventory.InventoryError):
            inventory.build_inventory(self.root, ["doc.effect.json"])
        self.assertFalse(os.path.exists(os.path.join(self.root, inventory.OUTPUT_PATH)))

    def test_absent_child_parent_contract_is_rejected(self) -> None:
        self._publish([], [], [], [], [])
        os.remove(os.path.join(self.root, inventory.CHILD_PARENT_RESOLUTION_PATH))
        with self.assertRaisesRegex(inventory.InventoryError, "required input"):
            inventory.build_inventory(self.root, ["doc.effect.json"])

    def test_child_parent_row_hash_and_duplicate_child_are_rejected(self) -> None:
        child = _child_blocked("pkg.mi.blocked")
        self._publish([], [], [], [], [], [child])
        _rewrite_artifact(
            self.root,
            inventory.CHILD_PARENT_RESOLUTION_PATH,
            lambda document: document["children"][0].__setitem__(
                "blocker", "changed-without-row-hash"
            ),
        )
        with self.assertRaisesRegex(inventory.InventoryError, "rowSha256"):
            inventory.build_inventory(self.root, ["doc.effect.json"])

        self._publish([], [], [], [], [], [child, dict(child)])
        with self.assertRaisesRegex(inventory.InventoryError, "duplicate childMaterialPath"):
            inventory.build_inventory(self.root, ["doc.effect.json"])

    def test_child_parent_dependency_pin_drift_is_rejected(self) -> None:
        self._publish([], [], [], [], [])
        _rewrite_artifact(
            self.root,
            inventory.CHILD_PARENT_RESOLUTION_PATH,
            lambda document: document["inputs"].__setitem__(
                "namedAbiRawSha256", "0" * 64
            ),
        )
        with self.assertRaisesRegex(inventory.InventoryError, "different namedAbi bytes"):
            inventory.build_inventory(self.root, ["doc.effect.json"])

    def test_child_parent_known_family_swap_disagreeing_with_family_row_is_rejected(
        self,
    ) -> None:
        first = "pkg.grp.fx_first_tr"
        second = "pkg.grp.fx_second_tr"
        child = "pkg.mi.child"
        self._publish(
            [],
            [
                _shader_map_row(first, "DECLARED_PACKAGE_EXPORT", inventory.COOKED_PRESENT),
                _shader_map_row(second, "DECLARED_PACKAGE_EXPORT", inventory.COOKED_PRESENT),
            ],
            [_cooked_extracted(first, DIGEST_A), _cooked_extracted(second, DIGEST_B)],
            [_named(first, DIGEST_A), _named(second, DIGEST_B)],
            [_translation(DIGEST_A, "first"), _translation(DIGEST_B, "second")],
            [_child_resolved(child, first)],
        )

        def swap_known_family(document: dict) -> None:
            row = document["children"][0]
            row["knownFamilyPath"] = second
            row.pop("rowSha256", None)
            row["rowSha256"] = inventory._canonical_sha256(row)

        _rewrite_artifact(
            self.root,
            inventory.CHILD_PARENT_RESOLUTION_PATH,
            swap_known_family,
        )
        with self.assertRaisesRegex(inventory.InventoryError, "evidence differs"):
            inventory.build_inventory(self.root, ["doc.effect.json"])

    def test_new_child_parent_family_cannot_claim_cooked_or_named_closure(self) -> None:
        child = _child_resolved(
            "new.pkg.mi", None, cooked_status="EXTRACTED", named_closed=True
        )
        self._publish([], [], [], [], [], [child])
        with self.assertRaisesRegex(inventory.InventoryError, "non-ABSENT cooked"):
            inventory.build_inventory(self.root, ["doc.effect.json"])

    def test_duplicate_parent_row_in_input_is_rejected(self) -> None:
        parent = "pkg.grp.fx_i_tr"
        self._publish(
            [_element(parent)],
            [
                _shader_map_row(parent, "DECLARED_PACKAGE_EXPORT", "BASE_MATERIAL_ID_UNRESOLVED"),
                _shader_map_row(parent, "LEAF_NAME_SEARCH", "BASE_MATERIAL_ID_UNRESOLVED"),
            ],
            [],
            [],
            [],
        )
        with self.assertRaises(inventory.InventoryError):
            inventory.build_inventory(self.root, ["doc.effect.json"])

    def test_check_accepts_only_current_lf_bytes(self) -> None:
        self._publish([_element("pkg.grp.fx_current_tr")], [], [], [], [])
        artifact = inventory.build_inventory(self.root, ["doc.effect.json"])
        published = inventory.write_inventory(self.root, artifact)
        with mock.patch.object(inventory, "TARGET_DOCUMENTS", ("doc.effect.json",)):
            with redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()):
                result = inventory.main(["--repository-root", self.root, "--check"])
        self.assertEqual(result, 0)
        with open(published, "rb") as handle:
            raw = handle.read()
        self.assertEqual(raw, inventory.serialize_inventory(artifact))
        self.assertNotIn(b"\r", raw)

    def test_check_rejects_stale_contract_without_replacing_it(self) -> None:
        self._publish([_element("pkg.grp.fx_stale_tr")], [], [], [], [])
        artifact = inventory.build_inventory(self.root, ["doc.effect.json"])
        published = inventory.write_inventory(self.root, artifact)
        stale = inventory.serialize_inventory(artifact).replace(
            b'"occurrenceCount": 1', b'"occurrenceCount": 9', 1
        )
        with open(published, "wb") as handle:
            handle.write(stale)
        with mock.patch.object(inventory, "TARGET_DOCUMENTS", ("doc.effect.json",)):
            with redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()):
                result = inventory.main(["--repository-root", self.root, "--check"])
        self.assertEqual(result, 1)
        with open(published, "rb") as handle:
            self.assertEqual(handle.read(), stale)

    def test_check_rejects_authored_input_drift(self) -> None:
        self._publish([_element("pkg.grp.fx_authored_tr")], [], [], [], [])
        artifact = inventory.build_inventory(self.root, ["doc.effect.json"])
        published = inventory.write_inventory(self.root, artifact)
        with open(published, "rb") as handle:
            original = handle.read()
        _write(
            self.root,
            inventory.AUTHORED_DIRECTORY + "/doc.effect.json",
            {"elements": [_element("pkg.grp.fx_authored_tr"), {"id": "detail"}]},
        )
        with mock.patch.object(inventory, "TARGET_DOCUMENTS", ("doc.effect.json",)):
            with redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()):
                result = inventory.main(["--repository-root", self.root, "--check"])
        self.assertEqual(result, 1)
        with open(published, "rb") as handle:
            self.assertEqual(handle.read(), original)

    def test_check_rejects_absent_published_contract(self) -> None:
        self._publish([_element("pkg.grp.fx_absent_tr")], [], [], [], [])
        with mock.patch.object(inventory, "TARGET_DOCUMENTS", ("doc.effect.json",)):
            with redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()):
                result = inventory.main(["--repository-root", self.root, "--check"])
        self.assertEqual(result, 1)

    def test_unsupported_schema_is_rejected(self) -> None:
        self._publish([], [], [], [], [])
        _rewrite_artifact(
            self.root,
            inventory.SHADER_MAP_INDEX_PATH,
            lambda document: document.__setitem__("schema", "wrong.schema"),
        )
        with self.assertRaisesRegex(inventory.InventoryError, "schema"):
            inventory.build_inventory(self.root, ["doc.effect.json"])

    def test_artifact_hash_drift_is_rejected(self) -> None:
        self._publish([], [], [], [], [])
        path = os.path.join(self.root, inventory.SHADER_MAP_INDEX_PATH)
        with open(path, "r", encoding="utf-8") as handle:
            document = json.load(handle)
        document["summary"]["parentMaterialCount"] = 9
        _write(self.root, inventory.SHADER_MAP_INDEX_PATH, document)
        with self.assertRaisesRegex(inventory.InventoryError, "artifactSha256 drifted"):
            inventory.build_inventory(self.root, ["doc.effect.json"])

    def test_cooked_shader_map_raw_pin_drift_is_rejected(self) -> None:
        self._publish([], [], [], [], [])
        _rewrite_artifact(
            self.root,
            inventory.COOKED_PIXEL_SHADERS_PATH,
            lambda document: document["inputs"].__setitem__(
                "shaderMapRawSha256", "0" * 64
            ),
        )
        with self.assertRaisesRegex(inventory.InventoryError, "different shaderMap bytes"):
            inventory.build_inventory(self.root, ["doc.effect.json"])

    def test_named_cooked_raw_pin_drift_is_rejected(self) -> None:
        self._publish([], [], [], [], [])
        _rewrite_artifact(
            self.root,
            inventory.NAMED_ABI_PATH,
            lambda document: document["inputs"].__setitem__(
                "cookedPixelShadersRawSha256", "0" * 64
            ),
        )
        with self.assertRaisesRegex(
            inventory.InventoryError, "different cookedPixelShaders bytes"
        ):
            inventory.build_inventory(self.root, ["doc.effect.json"])

    def test_duplicate_translation_digest_is_rejected(self) -> None:
        parent = "pkg.grp.fx_dup_tr"
        row = _translation(DIGEST_A, "dup")
        self._publish(
            [_element(parent)],
            [_shader_map_row(parent, "DECLARED_PACKAGE_EXPORT", inventory.COOKED_PRESENT)],
            [_cooked_extracted(parent, DIGEST_A)],
            [_named(parent, DIGEST_A)],
            [row, dict(row)],
        )
        with self.assertRaisesRegex(inventory.InventoryError, "duplicate dxbcSha256"):
            inventory.build_inventory(self.root, ["doc.effect.json"])

    def test_missing_translation_digest_is_rejected(self) -> None:
        parent = "pkg.grp.fx_missing_tr"
        self._publish(
            [_element(parent)],
            [_shader_map_row(parent, "DECLARED_PACKAGE_EXPORT", inventory.COOKED_PRESENT)],
            [_cooked_extracted(parent, DIGEST_A)],
            [_named(parent, DIGEST_A)],
            [],
        )
        with self.assertRaisesRegex(inventory.InventoryError, "translation denominator"):
            inventory.build_inventory(self.root, ["doc.effect.json"])

    def test_extra_translation_digest_is_rejected(self) -> None:
        self._publish([], [], [], [], [_translation(DIGEST_A, "extra")])
        with self.assertRaisesRegex(inventory.InventoryError, "translation denominator"):
            inventory.build_inventory(self.root, ["doc.effect.json"])

    def test_crlf_translation_contract_is_rejected(self) -> None:
        self._publish([], [], [], [], [])
        path = os.path.join(self.root, inventory.HLSL_TRANSLATIONS_PATH)
        with open(path, "rb") as handle:
            raw = handle.read()
        with open(path, "wb") as handle:
            handle.write(raw.replace(b"\n", b"\r\n"))
        with self.assertRaisesRegex(inventory.InventoryError, "not LF-only"):
            inventory.build_inventory(self.root, ["doc.effect.json"])

    def test_named_family_denominator_mismatch_is_rejected(self) -> None:
        parent = "pkg.grp.fx_named_denominator_tr"
        self._publish(
            [_element(parent)],
            [_shader_map_row(parent, "DECLARED_PACKAGE_EXPORT", inventory.COOKED_PRESENT)],
            [_cooked_extracted(parent, DIGEST_A)],
            [],
            [_translation(DIGEST_A, "named_denominator")],
        )
        with self.assertRaisesRegex(inventory.InventoryError, "named ABI family denominator"):
            inventory.build_inventory(self.root, ["doc.effect.json"])

    def test_unstructured_named_blocker_is_rejected(self) -> None:
        parent = "pkg.grp.fx_named_blocker_tr"
        self._publish(
            [_element(parent)],
            [_shader_map_row(parent, "DECLARED_PACKAGE_EXPORT", inventory.COOKED_PRESENT)],
            [_cooked_extracted(parent, DIGEST_A)],
            [_named_blocked(parent, "NATIVE_BINDING_ARRAY_CANDIDATE_AMBIGUOUS")],
            [_translation(DIGEST_A, "named_blocker")],
        )
        _rewrite_artifact(
            self.root,
            inventory.NAMED_ABI_PATH,
            lambda document: document["families"][0].__setitem__(
                "blocker", "unstructured"
            ),
        )
        with self.assertRaisesRegex(inventory.InventoryError, "structured blocker"):
            inventory.build_inventory(self.root, ["doc.effect.json"])

    def test_named_blocked_parent_set_drift_is_rejected(self) -> None:
        parent = "pkg.grp.fx_named_set_tr"
        self._publish(
            [_element(parent)],
            [_shader_map_row(parent, "DECLARED_PACKAGE_EXPORT", inventory.COOKED_PRESENT)],
            [_cooked_extracted(parent, DIGEST_A)],
            [_named_blocked(parent, "NATIVE_BINDING_ARRAY_CANDIDATE_AMBIGUOUS")],
            [_translation(DIGEST_A, "named_set")],
        )
        _rewrite_artifact(
            self.root,
            inventory.NAMED_ABI_PATH,
            lambda document: document["summary"].__setitem__("blockedParents", []),
        )
        with self.assertRaisesRegex(inventory.InventoryError, "blockedParents"):
            inventory.build_inventory(self.root, ["doc.effect.json"])

    def test_cooked_dxbc_raw_hash_drift_is_rejected(self) -> None:
        parent = "pkg.grp.fx_dxbc_drift_tr"
        self._publish(
            [_element(parent)],
            [_shader_map_row(parent, "DECLARED_PACKAGE_EXPORT", inventory.COOKED_PRESENT)],
            [_cooked_extracted(parent, DIGEST_A)],
            [_named(parent, DIGEST_A)],
            [_translation(DIGEST_A, "dxbc_drift")],
        )
        path = os.path.join(
            self.root, inventory.COOKED_SHADER_DIRECTORY, DIGEST_A + ".dxbc"
        )
        with open(path, "wb") as handle:
            handle.write(DXBC_A[:-1] + b"x")
        with self.assertRaisesRegex(inventory.InventoryError, "raw SHA-256 drifted"):
            inventory.build_inventory(self.root, ["doc.effect.json"])

    def test_translated_hlsli_raw_hash_drift_is_rejected(self) -> None:
        parent = "pkg.grp.fx_hlsl_drift_tr"
        translation = _translation(DIGEST_A, "hlsl_drift")
        self._publish(
            [_element(parent)],
            [_shader_map_row(parent, "DECLARED_PACKAGE_EXPORT", inventory.COOKED_PRESENT)],
            [_cooked_extracted(parent, DIGEST_A)],
            [_named(parent, DIGEST_A)],
            [translation],
        )
        path = os.path.join(
            self.root,
            inventory.TRANSLATED_SHADER_DIRECTORY,
            translation["functionName"] + ".hlsli",
        )
        with open(path, "ab") as handle:
            handle.write(b"// drift\n")
        with self.assertRaisesRegex(inventory.InventoryError, "raw SHA-256 drifted"):
            inventory.build_inventory(self.root, ["doc.effect.json"])

    def test_extra_translated_hlsli_is_rejected(self) -> None:
        self._publish([], [], [], [], [])
        path = os.path.join(
            self.root, inventory.TRANSLATED_SHADER_DIRECTORY, "unreported.hlsli"
        )
        with open(path, "wb") as handle:
            handle.write(b"float4 Unreported() { return 0.0.xxxx; }\n")
        with self.assertRaisesRegex(inventory.InventoryError, "HLSLI file set"):
            inventory.build_inventory(self.root, ["doc.effect.json"])

    def test_output_artifact_hash_is_canonical(self) -> None:
        self._publish([], [], [], [], [])
        artifact = inventory.build_inventory(self.root, ["doc.effect.json"])
        digest = artifact.pop("artifactSha256")
        self.assertEqual(digest, inventory._canonical_sha256(artifact))


if __name__ == "__main__":
    unittest.main()

from __future__ import annotations

import copy
import hashlib
import json
import subprocess
import sys
import unittest
from collections import Counter
from pathlib import Path

import build_artist_31470_source_contract as source_contract
from effect_source_contract_io import tracked_text_sha256


ROOT = Path(__file__).resolve().parents[2]
CANDIDATE = ROOT / (
    "Data/Effects/Imported/Artist/Candidates/"
    "effect.artist.skill.31470.native-v14.source-contract-candidate.effect.json"
)
RECEIPT = ROOT / (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.native-v14.source-contract-candidate.receipt.json"
)
REGISTRY = ROOT / "Data/Effects/Contracts/ue3-cascade-source-v1.registry.json"
SOURCE_RECEIPT = ROOT / "Data/Effects/Imported/Artist/skill.31470.source-receipt.json"
SOURCE_EVIDENCE = ROOT / "Data/Effects/Imported/Artist/skill.31470.source-evidence-envelope.json"
LOCAL_REFERENCE_CLOSURE = ROOT / "Data/Effects/Imported/Artist/Graphs/skill.31470.local-reference-closure.json"
GEOMETRY_PARITY = ROOT / "Data/Effects/Imported/Artist/Geometry/skill.31470.wmodel-geometry-parity.receipt.json"
PUBLISHER = ROOT / "Tools/EffectPipeline/Publish-Effects.ps1"


class Artist31470SourceContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.candidate = json.loads(CANDIDATE.read_text(encoding="utf-8"))
        cls.receipt = json.loads(RECEIPT.read_text(encoding="utf-8"))
        cls.registry = json.loads(REGISTRY.read_text(encoding="utf-8"))
        cls.local_reference = json.loads(
            LOCAL_REFERENCE_CLOSURE.read_text(encoding="utf-8")
        )
        cls.elements = {row["id"]: row for row in cls.candidate["elements"]}

    def element(self, fragment: str) -> dict:
        matches = [row for key, row in self.elements.items() if fragment in key]
        self.assertEqual(len(matches), 1, fragment)
        return matches[0]

    @staticmethod
    def module(element: dict, class_name: str) -> dict:
        return next(
            row
            for row in element["sourceRecipe"]["modules"]
            if row["className"].casefold() == class_name.casefold()
        )

    @staticmethod
    def distribution(module: dict, property_path: str) -> dict:
        return next(
            row
            for row in module["distributions"]
            if row["propertyPath"] == property_path
        )

    def test_source_receipt_stays_immutable_and_f_mapping_is_derived(self) -> None:
        source = json.loads(SOURCE_RECEIPT.read_text(encoding="utf-8"))
        self.assertEqual(source["inputSlot"], "R")
        self.assertEqual(self.receipt["importedReceiptInputSlot"], "R")
        self.assertEqual(self.receipt["derivedInputSlot"], "F")

    def test_native_v14_is_a_source_contract_not_runtime_product(self) -> None:
        self.assertEqual(self.candidate["version"], 14)
        self.assertEqual(self.candidate["purpose"], "source_contract")
        self.assertFalse(self.registry["runtimeAdmission"])
        self.assertFalse(self.receipt["productAdmission"]["allowed"])
        self.assertEqual(
            self.receipt["runtimeAdmission"], "NOT_IMPLEMENTED_IN_THIS_SLICE"
        )
        self.assertEqual(
            self.receipt["visualApprovalStatus"], "NOT_VISUAL_APPROVED"
        )
        for element in self.candidate["elements"]:
            self.assertFalse(element["material"]["sourceProfile"]["enabled"])

    def test_active_renderer_inventory_is_exactly_35(self) -> None:
        self.assertEqual(len(self.candidate["elements"]), 35)
        counts = Counter(row["renderer"]["type"] for row in self.candidate["elements"])
        self.assertEqual(
            counts,
            Counter(
                {
                    "meshParticle": 13,
                    "spriteParticle": 16,
                    "decalParticle": 3,
                    "cascadeRibbon": 1,
                    "lightParticle": 1,
                    "screenPost": 1,
                }
            ),
        )
        self.assertEqual(
            self.receipt["summary"]["excludedExecutionDisabledElementCount"], 93
        )

    def test_weapon_mesh_particle_numeric_golden_is_source_backed(self) -> None:
        element = self.element(
            "par_v_smd_onestroke_weapon_01.particlespriteemitter_6"
        )
        self.assertEqual(element["renderer"]["type"], "meshParticle")
        self.assertEqual(element["sourceRecipe"]["sourcePeakActiveParticles"], 3)
        self.assertIn(
            {
                "slotId": "meshModel",
                "assetId": "Effect/Artist/Meshes/fm_v_wp_wsdm_base_01.wmodel",
            },
            element["resources"],
        )
        attachment = element["actionCueAttachment"]
        self.assertEqual(attachment["sourceAnchorSlotId"], "WP_SDM_R_Battle")
        self.assertEqual(attachment["runtimeBoneName"], "b_wp_1")
        self.assertEqual(
            attachment["socketLocalTransform"],
            {
                "position": [0.0, 0.0, 0.0],
                "rotationDegrees": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            },
        )
        lifetime = self.distribution(
            self.module(element, "particlemodulelifetime"), "lifetime"
        )
        size = self.distribution(
            self.module(element, "particlemodulesize"), "startsize"
        )
        self.assertEqual(lifetime["lookupTable"], [0.4000000059604645] * 4)
        self.assertTrue(all(value == 2.5 for value in size["lookupTable"]))

    def test_ribbon_and_decal_numbers_are_preserved_without_runtime_claim(self) -> None:
        ribbon = self.element("par_v_sdm_ink_spw_01.particlespriteemitter_0")
        self.assertEqual(ribbon["renderer"]["type"], "cascadeRibbon")
        self.assertEqual(ribbon["sourceRecipe"]["sourcePeakActiveParticles"], 594)
        typedata = self.module(ribbon, "particlemoduletypedataribbon")
        literals = {row["propertyPath"]: row["value"] for row in typedata["literals"]}
        self.assertEqual(literals["tilingdistance"], 600.0)
        self.assertEqual(literals["distancetessellationstepsize"], 5.0)
        rate = self.distribution(
            self.module(ribbon, "particlemodulespawn"), "rate"
        )
        self.assertEqual(rate["lookupTable"], [40.0] * 4)

        decal = self.element(
            "par_v_sdm_onestroke_hit_01.particlespriteemitter_43"
        )
        self.assertEqual(decal["renderer"]["type"], "decalParticle")
        self.assertEqual(decal["sourceRecipe"]["sourcePeakActiveParticles"], 77)
        decal_type = next(
            row
            for row in decal["sourceRecipe"]["modules"]
            if row["className"].casefold().endswith("typedatadecal")
        )
        literals = {
            row["propertyPath"]: row["value"] for row in decal_type["literals"]
        }
        self.assertEqual(literals["nearplane"], -300.0)

    def test_registry_is_machine_readable_and_candidate_is_hash_pinned(self) -> None:
        self.assertEqual(
            self.registry["schema"], "lostark.effect-source-contract-registry"
        )
        self.assertGreater(len(self.registry["moduleClasses"]), 20)
        self.assertEqual(
            self.receipt["candidate"]["contractSha256"],
            self.registry["contractSha256"],
        )
        for element in self.candidate["elements"]:
            recipe = element["sourceRecipe"]
            self.assertEqual(
                recipe["sourceContractSha256"], self.registry["contractSha256"]
            )
            self.assertEqual(len(recipe["modules"]), len(recipe["moduleCoverage"]))

    def test_source_evidence_is_typed_linked_and_fail_closed(self) -> None:
        self.assertEqual(
            self.receipt["aggregateSourceEvidenceStatus"],
            "SOURCE_EVIDENCE_PARTIAL",
        )
        self.assertEqual(
            self.receipt["summary"]["aggregateSourceEvidenceStatus"],
            "SOURCE_EVIDENCE_PARTIAL",
        )
        evidence_ids = set()
        cue_ids = set()
        module_reference_count = 0
        for element in self.candidate["elements"]:
            recipe = element["sourceRecipe"]
            compiler = recipe["compilerEvidence"]
            self.assertEqual(
                compiler["sourceEvidenceStatus"], "SOURCE_EVIDENCE_PARTIAL"
            )
            self.assertEqual(compiler["lodSelectionPolicy"], "FIRST_LOD_ONLY")
            self.assertEqual(compiler["selectedLodArrayIndex"], 0)
            self.assertEqual(
                compiler["compositionOrder"],
                [
                    "carrierGeometryPreScale",
                    "signedParticleScaleRotationLocation",
                    "emitterElementTransform",
                    "cueLocalTransform",
                    "attachmentSocketOrRoot",
                    "actorWorld",
                ],
            )
            evidence_ids.add(compiler["evidenceId"])
            cue_ids.add(compiler["sourceCueId"])
            module_reference_count += len(compiler["moduleReferenceOrder"])
            self.assertEqual(
                len(compiler["moduleReferenceOrder"]), len(recipe["modules"])
            )
            self.assertFalse(recipe["compiledExecutionAdmission"]["allowed"])
            self.assertIn(
                "SOURCE_EVIDENCE_PARTIAL",
                recipe["compiledExecutionAdmission"]["blockers"],
            )
        self.assertEqual(len(evidence_ids), 35)
        self.assertEqual(len(cue_ids), 7)
        self.assertEqual(module_reference_count, 399)

    def test_local_reference_denominators_and_source_identity_axes_are_exact(self) -> None:
        summary = self.local_reference["summary"]
        self.assertEqual(self.local_reference["formatVersion"], 5)
        self.assertEqual(
            {
                name: summary[name]
                for name in (
                    "distributionTargetUniqueCount",
                    "distributionTargetOccurrenceCount",
                    "receiptPackageIdentityPinnedUniqueCount",
                    "pinnedPayloadDecodedUniqueCount",
                    "exactPhysicalSourcePackagePresentUniqueCount",
                    "distributionTargetPayloadDecodedUniqueCount",
                    "distributionTargetPayloadUnresolvedUniqueCount",
                    "distributionTargetPayloadUnresolvedOccurrenceCount",
                    "distributionTargetSemanticReadyUniqueCount",
                    "distributionTargetSemanticReadyOccurrenceCount",
                    "distributionTargetSemanticBlockedUniqueCount",
                    "distributionTargetSemanticBlockedOccurrenceCount",
                    "compiledExecutionAllowedOccurrenceCount",
                )
            },
            {
                "distributionTargetUniqueCount": 15,
                "distributionTargetOccurrenceCount": 17,
                "receiptPackageIdentityPinnedUniqueCount": 8,
                "pinnedPayloadDecodedUniqueCount": 7,
                "exactPhysicalSourcePackagePresentUniqueCount": 3,
                "distributionTargetPayloadDecodedUniqueCount": 14,
                "distributionTargetPayloadUnresolvedUniqueCount": 1,
                "distributionTargetPayloadUnresolvedOccurrenceCount": 2,
                "distributionTargetSemanticReadyUniqueCount": 9,
                "distributionTargetSemanticReadyOccurrenceCount": 9,
                "distributionTargetSemanticBlockedUniqueCount": 6,
                "distributionTargetSemanticBlockedOccurrenceCount": 8,
                "compiledExecutionAllowedOccurrenceCount": 0,
            },
        )
        definitions = {
            row["referenceId"]: row
            for row in self.local_reference["distributionDefinitions"]
        }
        for reference_id in (
            "distribution-target-000",
            "distribution-target-001",
            "distribution-target-002",
            "distribution-target-003",
            "distribution-target-007",
            "distribution-target-008",
            "distribution-target-009",
        ):
            definition = definitions[reference_id]
            self.assertFalse(definition["receiptPackageIdentityPinned"])
            self.assertFalse(definition["pinnedPayloadDecoded"])
            self.assertFalse(definition["fidelity"].startswith("SOURCE_EXACT"))

    def test_v14_bindings_preserve_distribution_and_point_light_boundaries(self) -> None:
        bindings = [
            binding
            for element in self.candidate["elements"]
            for binding in element["sourceRecipe"]["localReferenceBindings"]
        ]
        self.assertEqual(len(bindings), 18)
        self.assertEqual(
            Counter(binding["referenceKind"] for binding in bindings),
            Counter({"DISTRIBUTION_TARGET": 17, "TYPEDATA_COMPONENT": 1}),
        )
        self.assertEqual(len({row["occurrenceId"] for row in bindings}), 18)
        required_distribution_fields = {
            "referenceId",
            "occurrenceId",
            "payloadStatus",
            "fidelity",
            "executionAdmission",
        }
        distributions = [
            distribution
            for element in self.candidate["elements"]
            for module in element["sourceRecipe"]["modules"]
            for distribution in module["distributions"]
        ]
        self.assertEqual(len(distributions), 629)
        for distribution in distributions:
            self.assertTrue(required_distribution_fields <= distribution.keys())
            admission = distribution["executionAdmission"]
            self.assertEqual(admission["allowed"], not admission["blockers"])

        light = next(
            row for row in bindings if row["referenceKind"] == "TYPEDATA_COMPONENT"
        )
        self.assertEqual(light["referenceId"], "typedata-point-light-component-000")
        self.assertEqual(
            {row["propertyPath"]: row["value"] for row in light["exactPayload"]},
            {
                "brightness": 10.0,
                "bcastcompositeshadow": False,
                "baffectcompositeshadowdirection": False,
            },
        )
        self.assertEqual(
            {
                row["propertyPath"]: row["value"]
                for row in light["currentDefaultEvidence"]
            },
            {
                "radius": 200.0,
                "falloffexponent": 2.0,
                "lightcolor": [255, 255, 255, 0],
            },
        )
        self.assertEqual(
            set(light["executionAdmission"]["blockers"]),
            {
                "LIGHT_RENDERER_NOT_COMPILED",
                "POINT_LIGHT_CLASS_DEFAULTS_UNRESOLVED",
                "SOURCE_ERA_SCRIPT_PACKAGE_IDENTITY_NOT_PINNED",
            },
        )

    def test_target008_and_target014_remain_typed_and_fail_closed(self) -> None:
        definitions = {
            row["referenceId"]: row
            for row in self.local_reference["distributionDefinitions"]
        }
        curve = definitions["distribution-target-008"]
        self.assertEqual(
            curve["semanticCoverage"]["evaluatorKind"], "STANDARD_CONSTANT_CURVE"
        )
        self.assertIn(
            "CONSTANT_CURVE_COMPILER_NOT_IMPLEMENTED",
            curve["executionAdmission"]["blockers"],
        )
        self.assertNotIn(
            "CUSTOM_EF_DISTRIBUTION_EVALUATOR_UNPROVEN",
            curve["executionAdmission"]["blockers"],
        )

        rotation = definitions["distribution-target-014"]
        self.assertEqual(rotation["sourcePayload"]["variant"], "UNRESOLVED")
        self.assertIsNone(rotation["sourcePayload"]["properties"])
        self.assertFalse(
            rotation["sourceRevisionEvidence"]["sourceModuleQuorum"][
                "provesChildTargetPayloadEqual"
            ]
        )
        self.assertTrue(
            {
                "CROSS_REVISION_TARGET_PAYLOAD_NOT_SOURCE_EXACT",
                "CUSTOM_EF_DISTRIBUTION_EVALUATOR_UNPROVEN",
                "OLD_SOURCE_ABSENT_BINDING_FALLBACK_UNRESOLVED",
                "SELECTED_LOD_CLASS_DEFAULTS_UNRESOLVED",
                "RUNTIME_PARAMETER_SOURCE_CLOSURE_UNPROVEN",
            }
            <= set(rotation["executionAdmission"]["blockers"])
        )
        self.assertEqual(len(rotation["occurrenceIds"]), 2)

    def test_blocked_binder_rejects_before_payload_and_scrubs_evaluator_fields(self) -> None:
        class PoisonPayload(dict):
            def get(self, *_args: object, **_kwargs: object) -> object:
                raise AssertionError("blocked payload was inspected")

            def __bool__(self) -> bool:
                raise AssertionError("blocked payload was inspected")

        definitions = {
            row["referenceId"]: row
            for row in self.local_reference["distributionDefinitions"]
        }
        occurrences = {
            row["referenceId"]: row
            for row in self.local_reference["distributionOccurrences"]
        }
        for reference_id in (
            "distribution-target-000",
            "distribution-target-001",
            "distribution-target-007",
            "distribution-target-009",
            "distribution-target-014",
        ):
            definition = copy.deepcopy(definitions[reference_id])
            occurrence = copy.deepcopy(occurrences[reference_id])
            definition["sourcePayload"] = PoisonPayload()
            identity = occurrence["propertyIdentity"]
            property_path = identity["propertyPath"]
            self.assertTrue(property_path.endswith(".distribution"))
            distribution_property = property_path[: -len(".distribution")]
            distribution = {
                "propertyPath": distribution_property,
                "sourceClass": "poison",
                "sourceObjectPath": "WRONG.Poison.Payload",
                "parameterBinding": "actionCue",
                "parameterName": "Poison",
                "operation": 7,
                "randomLockAxes": 7,
                "lookupTableChunkSize": 4,
                "lookupTableNumElements": 4,
                "lookupTableTimeScale": 2.0,
                "lookupTableStartTime": 3.0,
                "defaultMinimum": [1.0, 2.0, 3.0, 4.0],
                "defaultMaximum": [5.0, 6.0, 7.0, 8.0],
                "lookupTable": [9.0],
                "keys": [{"time": 0.0, "value": [1.0]}],
            }
            module_order = int(occurrence["referenceOrderIndex"])
            modules = [
                {
                    "objectPath": f"FILLER_{index}.module",
                    "stableId": f"filler:{index}",
                    "distributions": [],
                    "literals": [],
                }
                for index in range(module_order + 1)
            ]
            modules[module_order] = {
                "objectPath": (
                    f"{identity['logicalPackage']}.{identity['packageLocalPath']}"
                ),
                "stableId": f"source:{module_order}",
                "distributions": [distribution],
                "literals": [],
            }
            occurrence_key = source_contract.property_identity_key(
                occurrence["activeElementId"],
                identity["logicalPackage"],
                identity["packageLocalPath"],
                identity["propertyPath"],
            )
            bindings, _overlays, consumed = (
                source_contract.bind_source_recipe_local_references(
                    {"modules": modules},
                    {"evidenceId": occurrence["activeElementId"]},
                    {definition["definitionId"]: definition},
                    {occurrence_key: occurrence},
                    {},
                    {},
                )
            )
            self.assertEqual(consumed, {occurrence["occurrenceId"]})
            self.assertEqual(len(bindings), 1)
            self.assertEqual(distribution["payloadStatus"], "UNRESOLVED_SEMANTIC_CLOSURE")
            self.assertFalse(distribution["executionAdmission"]["allowed"])
            self.assertEqual(distribution["sourceObjectPath"], "")
            self.assertEqual(distribution["operation"], 0)
            self.assertEqual(distribution["randomLockAxes"], 0)
            self.assertEqual(distribution["lookupTableChunkSize"], 0)
            self.assertEqual(distribution["lookupTableNumElements"], 0)
            self.assertEqual(distribution["lookupTable"], [])
            self.assertEqual(distribution["keys"], [])
            self.assertEqual(distribution["defaultMinimum"], [0.0] * 4)
            self.assertEqual(distribution["defaultMaximum"], [0.0] * 4)
            if reference_id == "distribution-target-000":
                wrong_package_definition = copy.deepcopy(definition)
                wrong_package_definition["logicalPackage"] = "WRONG_PACKAGE"
                with self.assertRaisesRegex(
                    ValueError, "definition module identity changed"
                ):
                    source_contract.bind_source_recipe_local_references(
                        {"modules": copy.deepcopy(modules)},
                        {"evidenceId": occurrence["activeElementId"]},
                        {
                            wrong_package_definition["definitionId"]: (
                                wrong_package_definition
                            )
                        },
                        {occurrence_key: occurrence},
                        {},
                        {},
                    )

    def test_local_reference_blockers_reach_every_public_admission_gate(self) -> None:
        receipt_elements = {
            row["elementId"]: row for row in self.receipt["elements"]
        }
        receipt_blockers = set(self.receipt["blockers"])
        registry_blockers = set(self.registry["executionAdmission"]["blockers"])
        product_blockers = set(self.receipt["productAdmission"]["blockers"])
        for element in self.candidate["elements"]:
            recipe = element["sourceRecipe"]
            coverage = {
                row["moduleStableId"]: row for row in recipe["moduleCoverage"]
            }
            receipt_element = receipt_elements[element["id"]]
            for binding in recipe["localReferenceBindings"]:
                blockers = set(binding["executionAdmission"]["blockers"])
                module = coverage[binding["moduleStableId"]]
                prop = next(
                    row
                    for row in module["properties"]
                    if row["propertyPath"] == binding["propertyPath"]
                )
                self.assertTrue(blockers <= set(prop["blockers"]))
                self.assertTrue(blockers <= set(module["blockers"]))
                self.assertTrue(
                    blockers <= set(recipe["compiledExecutionAdmission"]["blockers"])
                )
                self.assertTrue(blockers <= set(receipt_element["blockers"]))
                self.assertTrue(blockers <= receipt_blockers)
                self.assertTrue(blockers <= registry_blockers)
                self.assertTrue(blockers <= product_blockers)

        product = self.receipt["productAdmission"]
        self.assertEqual(product["allowed"], product["blockerCount"] == 0)
        self.assertEqual(product["blockerCount"], len(product["blockers"]))
        self.assertEqual(
            product["blockerSummary"]["uniqueBlockerCount"],
            product["blockerCount"],
        )
        self.assertFalse(product["allowed"])

    def test_evidence_artifact_hash_chain_is_not_sidecar_only(self) -> None:
        links = {
            "sourceEvidence": (SOURCE_EVIDENCE, "evidenceSha256"),
            "localReferenceClosure": (
                LOCAL_REFERENCE_CLOSURE,
                "closureSha256",
            ),
            "wmodelGeometryParity": (GEOMETRY_PARITY, "receiptSha256"),
        }
        for name, (path, self_field) in links.items():
            artifact = json.loads(path.read_text(encoding="utf-8"))
            unsigned = dict(artifact)
            expected_self = unsigned.pop(self_field)
            actual_self = hashlib.sha256(
                json.dumps(
                    unsigned,
                    ensure_ascii=False,
                    sort_keys=True,
                    separators=(",", ":"),
                ).encode("utf-8")
            ).hexdigest()
            self.assertEqual(expected_self, actual_self)
            link = self.receipt["source"][name]
            self.assertEqual(
                link["fileSha256"], tracked_text_sha256(path)
            )
            self.assertEqual(link["selfSha256"], expected_self)
        registry_links = self.registry["evidenceLinks"]
        self.assertEqual(
            registry_links["localReferenceClosure"]["selfSha256"],
            self.receipt["source"]["localReferenceClosure"]["selfSha256"],
        )
        self.assertEqual(
            registry_links["wmodelGeometryParity"]["selfSha256"],
            self.receipt["source"]["wmodelGeometryParity"]["selfSha256"],
        )

    def test_unresolved_local_references_cannot_be_source_decoded(self) -> None:
        unresolved = []
        for element in self.candidate["elements"]:
            for module in element["sourceRecipe"]["moduleCoverage"]:
                for prop in module["properties"]:
                    if prop["status"] == "unresolved":
                        unresolved.append((module, prop))
                        self.assertNotEqual(prop["provenance"], "SOURCE_TAGGED_PRIMITIVE")
                        self.assertEqual(module["status"], "unresolved")
                        self.assertTrue(module["blockers"])
        paths = {prop["propertyPath"] for _, prop in unresolved}
        self.assertIn("startrotation", paths)
        self.assertIn("colorscaleoverlife", paths)

    def test_mesh_geometry_prescale_and_duplicate_denominator_are_explicit(self) -> None:
        geometry = json.loads(GEOMETRY_PARITY.read_text(encoding="utf-8"))
        weapon = next(
            row
            for row in geometry["assets"]
            if row["assetId"].endswith("fm_v_wp_wsdm_base_01.wmodel")
        )
        partition = weapon["parity"]["indexedFullPayloadPartitions"]
        self.assertEqual(partition["sourceVertexCount"], 11686)
        self.assertEqual(partition["sourceReferencedVertexCount"], 11686)
        self.assertEqual(partition["sourceUnreferencedVertexCount"], 0)
        self.assertEqual(partition["sourceDuplicateFullPayloadVertexCount"], 78)
        self.assertEqual(partition["runtimeVertexCount"], 11608)
        for element in self.candidate["elements"]:
            binding = element["sourceRecipe"]["geometryBinding"]
            if element["renderer"]["type"] == "meshParticle":
                self.assertTrue(binding["enabled"])
                self.assertEqual(binding["carrierGeometryPreScale"], 0.01)
                self.assertEqual(
                    binding["particleScaleSemantics"],
                    "DIMENSIONLESS_AXIS_REORDER_ONLY",
                )
            else:
                self.assertFalse(binding["enabled"])
                self.assertEqual(binding["carrierGeometryPreScale"], 1.0)

    def test_existing_publisher_does_not_admit_v14(self) -> None:
        source = PUBLISHER.read_text(encoding="utf-8")
        self.assertIn(
            "$documentVersion -notin @(5, 6, 7, 8, 9, 10, 11, 12)", source
        )
        self.assertNotIn("$documentVersion -notin @(5, 6, 7, 8, 9, 10, 11, 12, 14)", source)
        self.assertNotIn("source-contract-candidate", source)

    def test_checked_in_outputs_are_byte_exact(self) -> None:
        command = [
            sys.executable,
            str(ROOT / "Tools/LevelPlacementExtractor/build_artist_31470_source_contract.py"),
            "--source-receipt",
            str(SOURCE_RECEIPT),
            "--action-cue-recipe",
            str(ROOT / "Data/Effects/Imported/Artist/skill.31470.action-cue-recipe.json"),
            "--active-inventory",
            str(ROOT / "Data/Effects/Imported/Artist/skill.31470.source-active-effect-inventory.receipt.json"),
            "--normalized-graph",
            str(ROOT / "Data/Effects/Imported/Artist/Graphs/skill.31470.normalized-effect-graph.json"),
            "--module-closure",
            str(ROOT / "Data/Effects/Imported/Artist/Modules/skill.31470.external-module-closure.json"),
            "--material-closure",
            str(ROOT / "Data/Effects/Imported/Artist/Materials/skill.31470.active-material-closure.json"),
            "--source-evidence",
            str(SOURCE_EVIDENCE),
            "--local-reference-closure",
            str(LOCAL_REFERENCE_CLOSURE),
            "--geometry-parity",
            str(GEOMETRY_PARITY),
            "--output-candidate",
            str(CANDIDATE),
            "--output-receipt",
            str(RECEIPT),
            "--output-registry",
            str(REGISTRY),
            "--output-header",
            str(ROOT / "Client/Public/Generated/Effect_SourceContractRegistry.generated.h"),
            "--check",
        ]
        completed = subprocess.run(
            command, cwd=ROOT, capture_output=True, text=True, check=False
        )
        self.assertEqual(completed.returncode, 0, completed.stderr)


if __name__ == "__main__":
    unittest.main()

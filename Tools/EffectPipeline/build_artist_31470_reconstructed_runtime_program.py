#!/usr/bin/env python3
"""Build the immutable, typed Artist 31470 F reconstructed runtime program.

This is an offline evidence join.  It deliberately does not grant runtime or
Product admission.  Every input is read from an independently reviewed Git
object, every emitted row is sealed, and every executable choice is explicit.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import struct
import subprocess
import sys
from pathlib import Path
from typing import Any, Iterable


SCHEMA = "lostark.artist-31470-reconstructed-runtime-program"
FORMAT_VERSION = 1
PROGRAM_ID = "effect.artist.skill.31470.reconstructed-approved-v1"
PROGRAM_VERSION = 1
PROGRAM_ROLE = "OFFLINE_IMMUTABLE_TYPED_PROGRAM_NOT_RUNTIME_OR_PRODUCT_AUTHORITY"
MAX_PROGRAM_BYTES = 20 * 1024 * 1024
REQUIRED_GLOBAL_PROGRAM_BLOCKERS = frozenset({
    "CATALOG_PUBLISH_TRANSACTION_NOT_RUN",
    "EXECUTION_ADMISSION_FALSE",
    "MANUAL_HUMAN_EYE_VALIDATION_0_OF_35",
    "PRODUCT_ADMISSION_NOT_OPEN",
    "PRODUCT_ORACLE_35_OF_35_NOT_PASSED",
    "R3_TYPED_EXECUTOR_NOT_COMPLETE",
    "R4_GEOMETRY_AND_MATERIAL_RUNTIME_BINDING_NOT_COMPLETE",
    "R5_SIX_RENDERER_FAMILIES_NOT_COMPLETE",
    "R6_35_OCCURRENCE_RUNTIME_AND_MANUAL_VALIDATION_NOT_COMPLETE",
    "R7_FREEZE_BUILD_AND_TRANSACTION_REGRESSION_NOT_COMPLETE",
    "RENDERER_POLICY_CONSUMER_NOT_IMPLEMENTED",
    "SOURCE_FIDELITY_BLOCKERS_PRESERVED",
    "TYPED_RUNTIME_MATERIAL_POLICY_CONSUMER_NOT_IMPLEMENTED",
})
EXPECTED_D3D_DESCRIPTOR_PROJECTION_SHA256 = (
    "5b76e727418c588efd614524e5202b3aaf7040ba4064d0faafe7195ce994c49d"
)
EXPECTED_D3D_SRV_PROJECTION_SHA256 = (
    "47d4c6bea3fa30805ae5a085cfe2094c56766a04ae45d4c76120dbefecedb14f"
)
EXPECTED_APPROVAL_RECEIPT_SHA256 = (
    "da922add6d90287c88eed1d97efd8bd29ef35cd724c11f87ccfbb0bf9c3b7957"
)
EXPECTED_APPROVAL_CANONICAL_SHA256 = (
    "5838c227ddde3755a49ca4b69437b8aecfd65e09679b584ab3e6a998b9088473"
)
EXPECTED_APPROVAL_SOURCE_FAMILIES_SHA256 = (
    "faef8854a965934114c483dd70393efef91e439b578bb21ae59d38b025d15a36"
)
EXPECTED_APPROVAL_MATERIAL_FAMILIES_SHA256 = (
    "f6eef769a5057938276e5d57f163ee73840274a88a17c38cd98e0fb49a2ba20b"
)
EXPECTED_APPROVAL_SOURCE_ROWS_SHA256 = (
    "fd5b0a034ed882d909f867da1adb0edd5d0861c2deed3f373623fa7281185a6f"
)
EXPECTED_APPROVAL_MATERIAL_ROWS_SHA256 = (
    "24f85942185874577ec2e0f1df364f515dccc2ae01110ec984af976ee59e9c1a"
)
EXPECTED_APPROVAL_ARITHMETIC_ROWS_SHA256 = (
    "5eae02a316924651bf84e16dc93a2ddda7e6a97bf51b07156a11110aadbf6204"
)
EXPECTED_APPROVAL_GEOMETRY_ROWS_SHA256 = (
    "7ffc270ca3038f40913aae1d3c6c9effff79d98e7ebf443bd9342dad9c4fea11"
)
EXPECTED_SOURCE_CAPABILITY_RECEIPT_SHA256 = (
    "5d1b827cb3bbd9ac4ddbf3c3dd976a584d9251017e9979b5aae771e8e9a1ae1f"
)
EXPECTED_SOURCE_CAPABILITY_CANONICAL_SHA256 = (
    "62f3b5847262926ef5834ca9192dc9d5a3046045a3ad69cfa94bae0f59f2c55c"
)
EXPECTED_MATERIAL_POLICY_RECEIPT_SHA256 = (
    "10c7cacac0c54bf22060ab54a5596d48785631dd65b12c1c2810a87eb013d1c7"
)
EXPECTED_MATERIAL_POLICY_CANONICAL_SHA256 = (
    "412f8466ef50c67aa2c4a14a54a7f9f8df50ca7978b58a4daad0a69db93dc801"
)
EXPECTED_MATERIAL_TEXTURE_BINDING_RECEIPT_SHA256 = (
    "39c91577c09b853fa55a8fd5531c1cddc4fef928d77a6caa7f67c472a56159e0"
)
EXPECTED_MATERIAL_TEXTURE_BINDING_CANONICAL_SHA256 = (
    "bd0abbb81cc6daa46f83f9ca8850512f597280c8df42fe5ee23b8e140720bcaf"
)
EXPECTED_MATERIAL_TEXTURE_BINDING_TREE_ID = (
    "2f00f00851ee93f498dd6c13d6a3055209d4d8c3"
)
EXPECTED_MATERIAL_TEXTURE_BINDING_BLOB_ID = (
    "3105c22a3c8e9b73b47b721ffab72d1254fc1750"
)
# Filled from the exact normalized 72-row program projection below.  This is a
# fixture-specific offline trust pin, not a production-parser constant.
EXPECTED_MATERIAL_TEXTURE_BINDINGS_SHA256 = (
    "68cefbf719310b090c92e1475c01c46642854f7a050009b6cb2026418e07ce0e"
)
EXPECTED_MATERIAL_FAMILY_SAMPLES_SHA256 = (
    "5350e20004898130541c6dc147ce2c14fa8a7870bee36a9baeaf14dcda33b5d3"
)
EXPECTED_MATERIAL_RECIPE_SAMPLES_SHA256 = (
    "23977bf027c015086393b1f4b2a9495313a992025ffdc4ad3ac769e20595f69b"
)
EXPECTED_CAPABILITY_SAMPLES_SHA256 = (
    "d367b8c6f8134bf70fb16bbb7bf071f06bcf0bfcb5b459dad0c687effecbbc94"
)
EXPECTED_DISTRIBUTION_SAMPLES_SHA256 = (
    "7e4abfe5346568f053859fc717be94ff9a8b750cd64edad4128288a7adb6d546"
)
EXPECTED_RENDERER_TEXTURE_RESOURCES_SHA256 = (
    "6c4ff35e5323f81e1c7556167e3264fc83c57d8e9390cf89b10c3ac5ac01c023"
)
EXPECTED_GEOMETRY_CARRIERS_SHA256 = (
    "a602be873f49bb21954a0554c467c6945d26c328a5f46218af7ad6cf22254ba3"
)
EXPECTED_BLOCKER_OWNERSHIP_PROJECTION_SHA256 = (
    "c5ba50fd76fd0b14f849c5bb6adf7e0259a2f885d3dd28d4588c67eb5db576d0"
)
EXPECTED_ACTION_CUE_PROJECTION_SHA256 = {
    "skill-31470/clip-000/notify-000": "af843d4cc8b81bae1f0a47ba3cb40c2962daae3bcd800e18eb5c50dd62bdcf1e",
    "skill-31470/clip-000/notify-014": "47a93850834f949645c79cdbdba0583cb0c3efe3e12683ec63d17bfe6cb074a6",
    "skill-31470/clip-000/notify-018": "9f2fd20522ffd275667e83bf95066d754d48c9f1efdb4a6801afe8d6678da068",
    "skill-31470/clip-000/notify-022": "554a99bc413989c76498aa84909fb768502e8c2e7376b3229ef26caf0a1893b3",
    "skill-31470/clip-000/notify-026": "82d8f16ff1a4a7aaf188d98b608b4cba06cc763672588c8ab88eae3f80bbfc92",
    "skill-31470/clip-000/notify-028": "0fdf5a735c2b9b8509585a8050963104164f6a9787276f85b189696967e467a4",
    "skill-31470/clip-000/notify-029": "d4b2b817026969c468e475da3badb7ffed0a799a15cea622ec7c90a96749e395",
}

REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_OUTPUT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.reconstructed-runtime-program.candidate.json"
)

APPROVAL_COMMIT = "baee9f64039b455dd17666c2cd1289b8d1777d28"
SOURCE_EVIDENCE_COMMIT = "7da937aeaa34c088c694e8eb4f53ff1f7f848ef3"
SOURCE_CAPABILITY_COMMIT = "2fa2a2701e0ec9a8f990c65fef3fabb9e5a3e408"
MATERIAL_EVIDENCE_COMMIT = "cde8f3bddea2f9415f682b387d2705fd25794075"
MATERIAL_POLICY_COMMIT = "97597531215fa9c9873fe1be3ba8cd23db60031d"
MATERIAL_TEXTURE_BINDING_COMMIT = "fda3b5637847f9205915ad25ff02215424024b88"
GEOMETRY_COMMIT = "0aca792819fdda3f541bb7cec7451c5ed93c6467"

APPROVAL_PATH = (
    "Data/Effects/Policies/Artist/"
    "artist.31470.f.reconstructed-approved-v1.policy.receipt.json"
)
SOURCE_SEMANTICS_PATH = (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.source-execution-semantics.receipt.json"
)
SOURCE_CAPABILITY_PATH = (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.reconstructed-source-capability.receipt.json"
)
LOCAL_CLOSURE_PATH = (
    "Data/Effects/Imported/Artist/Graphs/"
    "skill.31470.local-reference-closure.json"
)
SOURCE_CANDIDATE_PATH = (
    "Data/Effects/Imported/Artist/Candidates/"
    "effect.artist.skill.31470.native-v14.source-contract-candidate.effect.json"
)
NORMALIZED_GRAPH_PATH = (
    "Data/Effects/Imported/Artist/Graphs/"
    "skill.31470.normalized-effect-graph.json"
)
ACTION_CUE_RECIPE_PATH = (
    "Data/Effects/Imported/Artist/skill.31470.action-cue-recipe.json"
)
MATERIAL_CONTRACT_PATH = (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.typed-material-evidence-contract.json"
)
MATERIAL_RUNTIME_PATH = (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.material-runtime-oracle.receipt.json"
)
MATERIAL_POLICY_PATH = (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.material-reconstructed-approved-v1.receipt.json"
)
MATERIAL_TEXTURE_BINDING_PATH = (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.material-texture-runtime-binding.receipt.json"
)
GEOMETRY_BINDING_PATH = (
    "Data/Effects/Imported/Artist/Geometry/"
    "skill.31470.geometry-binding.json"
)
GEOMETRY_RECEIPT_PATH = (
    "Data/Effects/Imported/Artist/Geometry/"
    "skill.31470.geometry-resource-binding.receipt.json"
)

AUTHORITIES = (
    ("approval", APPROVAL_PATH, APPROVAL_COMMIT, "receiptSha256"),
    ("sourceSemantics", SOURCE_SEMANTICS_PATH, SOURCE_EVIDENCE_COMMIT, "receiptSha256"),
    ("sourceCapability", SOURCE_CAPABILITY_PATH, SOURCE_CAPABILITY_COMMIT, "receiptSha256"),
    ("localReferenceClosure", LOCAL_CLOSURE_PATH, SOURCE_EVIDENCE_COMMIT, "closureSha256"),
    ("sourceCandidate", SOURCE_CANDIDATE_PATH, SOURCE_EVIDENCE_COMMIT, ""),
    ("normalizedEffectGraph", NORMALIZED_GRAPH_PATH, SOURCE_EVIDENCE_COMMIT, ""),
    ("actionCueRecipe", ACTION_CUE_RECIPE_PATH, SOURCE_EVIDENCE_COMMIT, ""),
    ("materialContract", MATERIAL_CONTRACT_PATH, MATERIAL_EVIDENCE_COMMIT, "contractSha256"),
    ("materialRuntime", MATERIAL_RUNTIME_PATH, MATERIAL_EVIDENCE_COMMIT, "receiptSha256"),
    ("materialPolicy", MATERIAL_POLICY_PATH, MATERIAL_POLICY_COMMIT, "receiptSha256"),
    ("geometryBinding", GEOMETRY_BINDING_PATH, GEOMETRY_COMMIT, "bindingSha256"),
    ("geometryReceipt", GEOMETRY_RECEIPT_PATH, GEOMETRY_COMMIT, "receiptSha256"),
    ("materialTextureBinding", MATERIAL_TEXTURE_BINDING_PATH,
     MATERIAL_TEXTURE_BINDING_COMMIT, "receiptSha256"),
)

ROOT_KEYS = (
    "schema", "formatVersion", "programId", "programVersion", "target",
    "policyRouteBinding", "particleSystemPolicy", "materialEvaluatorOracleContract",
    "programRole", "inputArtifacts", "handlerRegistry", "emitters", "actionSchedules", "modules",
    "properties", "primitiveLeaves", "literals", "distributions", "seedPolicies",
    "implicitDefaults", "pointLightFields", "materialFamilies", "materialRecipes",
    "materialInputs", "materialStaticBindings", "materialRenderBindings",
    "materialOccurrences", "materialPolicyRows", "materialTextureBindings",
    "rendererTextureResources",
    "geometryCarriers", "geometryUses",
    "sectionDigests", "blockerOwnership", "blockerUnion", "admission", "summary",
    "programSha256",
)

BLOCKER_OWNERSHIP_KEYS = (
    "version", "fieldCount", "tokenOccurrenceCount", "projectionSha256",
)

SECTION_NAMES = (
    "inputArtifacts", "handlerRegistry", "emitters", "actionSchedules", "modules", "properties",
    "primitiveLeaves", "literals", "distributions", "seedPolicies",
    "implicitDefaults", "pointLightFields", "materialFamilies", "materialRecipes",
    "materialInputs", "materialStaticBindings", "materialRenderBindings",
    "materialOccurrences", "materialPolicyRows", "materialTextureBindings",
    "rendererTextureResources",
    "geometryCarriers", "geometryUses",
)

ROW_KEYS: dict[str, tuple[str, ...]] = {
    "inputArtifacts": (
        "artifactId", "order", "path", "authorityCommitId", "authorityTreeId",
        "blobId", "schema", "versionField", "versionValue", "hashDomain",
        "trackedTextSha256", "canonicalJsonSha256", "selfHashField", "selfSha256",
        "rowSha256",
    ),
    "handlerRegistry": (
        "handlerRegistryId", "order", "handlerKind", "implementationId",
        "implementationVersion", "implementationSha256", "exactSourceClass", "variant",
        "consumerContract", "contractSha256", "sourceExact", "rowSha256",
    ),
    "emitters": (
        "emitterId", "order", "evidenceId", "sourceOccurrenceId", "sourceSystemId",
        "sourceEmitterPath", "sourceEmitterNodeId", "sourceCueId", "sourceElementId", "sourceNode",
        "sourceActionCueProjectionSha256",
        "visible", "rendererType", "rendererSourceSpace", "selectedLodPath",
        "selectedLodNodeId", "selectedLodRecordSha256", "selectedLodDecision",
        "selectedLodOracleId", "sourceRecipeEnabled", "localSpace", "sizeUnitPolicy",
        "operationalMaxParticles", "sourcePeakActiveParticles", "spawnRateFallbackPolicy",
        "lifetimeFallbackPolicy", "operationalCapPolicy", "operationalCapProjectionSha256",
        "operationalCapBlockers", "rendererRuntimeConfig", "screenPostAdapter",
        "lightAdapter", "decalAdapter", "ribbonAdapter", "timing", "random", "cueLocalTransform",
        "detailTransform", "actionCueAttachment", "transformInheritance",
        "transformCompositionOrder", "anchorRequests", "moduleIds",
        "actionCueParameterInputs", "scheduleId", "materialOccurrenceId", "textureResourceIds",
        "geometryUseId",
        "sourceExact", "rowSha256",
    ),
    "actionSchedules": (
        "scheduleId", "order", "sourceCueId", "sourceOccurrenceId", "sourceSystemId",
        "sourceReceiptEventIndex", "globalTimeSeconds", "durationSeconds",
        "sourceCueRowSha256", "rowSha256",
    ),
    "modules": (
        "moduleId", "emitterId", "order", "sourceObjectId", "sourceRecordSha256",
        "exactSourceClass", "selectionKind", "handlerRegistryId", "handlerVariant",
        "capabilityPolicyFamilyId", "capabilityImplementationId",
        "capabilityImplementationVersion", "capabilityImplementationSha256",
        "capabilityFamilySemanticSha256", "capabilityInputSchemaSha256",
        "capabilityOutputSchemaSha256", "capabilityDefaultPolicySha256",
        "capabilityNumericSamples", "capabilitySourceRowSha256",
        "capabilityLiteralBindingsSha256", "capabilityDistributionBindingsSha256",
        "capabilityPropertyConsumptionSha256", "capabilitySeedBindingSha256",
        "capabilityActionCueInputNames", "capabilityActionCueInputsSha256",
        "approvalPolicyRowId", "approvalUpstreamIdentitySha256",
        "approvalPolicyBindingSha256", "approvalRequiredMutatedOutput",
        "approvalSourceOrder", "approvalRequiredOracleIds", "approvalExecutionBlockers",
        "approvalSourceRow",
        "approvalSourceRowSha256",
        "propertyIds", "primitiveLeafIds", "literalIds", "distributionIds",
        "seedPolicyId", "implicitDefaultIds", "sourceDecision", "preservedBlockers",
        "sourceExact", "rowSha256",
    ),
    "properties": (
        "propertyId", "moduleId", "order", "propertyPath", "handlerRegistryId",
        "consumptionDecision", "irrelevanceOracleId", "payloadLiteralIds",
        "payloadDistributionIds", "semanticDistributionIds", "sourceFidelity",
        "capabilityConsumptionDecision", "semanticRole", "outputDependencyRequired",
        "capabilitySourceRowSha256", "preservedBlockers", "sourceExact", "rowSha256",
    ),
    "primitiveLeaves": (
        "leafId", "moduleId", "propertyId", "order", "propertyPath",
        "topLevelPropertyPath", "literalId", "valueVariant", "handlerRegistryId",
        "consumptionDecision", "preservedBlockers", "sourceExact", "rowSha256",
    ),
    "literals": (
        "literalId", "moduleId", "propertyId", "order", "propertyPath", "variant",
        "boolValue", "f64Value", "enumStringValue", "sourceExact", "rowSha256",
    ),
    "distributions": (
        "distributionId", "moduleId", "propertyId", "order", "variant",
        "evaluatorRegistryId", "payloadDistributionId", "propertyPath", "sourceClass",
        "sourceObjectPath", "referenceId", "occurrenceId", "payloadStatus", "fidelity",
        "componentCount", "operation", "randomLockAxes", "lookupTableChunkSize",
        "lookupTableNumElements", "lookupTableTimeScale", "lookupTableStartTime",
        "defaultMinimum", "defaultMaximum", "lookupTable", "curveKeys",
        "parameterName", "paramModes", "minimumInput", "maximumInput", "minimumOutput",
        "maximumOutput", "constantValues", "isDirty", "actionCueBindings",
        "fieldProvenance", "samples", "capabilityImplementationId",
        "capabilityImplementationVersion", "capabilityImplementationSha256",
        "preservedBlockers", "sourceExact", "rowSha256",
    ),
    "seedPolicies": (
        "seedPolicyId", "moduleId", "order", "evaluatorId", "randomSeeds",
        "parameterName", "getSeedFromInstance", "instanceSeedIsIndex",
        "resetSeedOnEmitterLooping", "randomlySelectSeedArray",
        "emptyArrayUsesOccurrenceRandomStream", "currentCdoEvidenceKey", "source",
        "sourceFidelity", "sourceExact", "rowSha256",
    ),
    "implicitDefaults": (
        "defaultId", "moduleId", "order", "family", "fieldPath", "variant",
        "boolValue", "ribbonValues", "decalValues", "decision", "provenance",
        "reason", "valuesOwnedBy", "sourceExact", "rowSha256",
    ),
    "pointLightFields": (
        "fieldId", "moduleId", "order", "fieldPath", "valueVariant", "boolValue",
        "f64Value", "colorRgba8Value", "guid128Value", "sourceTier", "sourceFidelity",
        "decision", "oracleId", "sourceExact", "rowSha256",
    ),
    "materialFamilies": (
        "familyId", "order", "familyIdentitySha256", "evaluatorRegistryId",
        "evaluatorId", "evaluatorVersion", "evaluatorSha256", "rendererShapes",
        "featureMask", "features", "graphProvenance", "cpuNumericOracleVerified",
        "hlslNumericOracleVerified", "numericSamples", "sampleProjectionSha256",
        "approvalPolicyRowId", "approvalArithmeticOrder", "approvalPolicyFamilyId",
        "approvalRequiredOracleIds",
        "approvalExecutionBlockers", "approvalArithmeticRow",
        "approvalArithmeticRowSha256", "preservedBlockers",
        "sourceExact", "rowSha256",
    ),
    "materialRecipes": (
        "recipeId", "order", "sourceMaterialPath", "sourceRecipeCompositionSha256",
        "familyId", "evaluatorRegistryId", "inputIds", "staticBindingIds",
        "renderBindingIds", "numericBindingSamples", "bindingSha256",
        "preservedBlockers", "sourceExact", "rowSha256",
    ),
    "materialInputs": (
        "fieldId", "recipeId", "order", "fieldKind", "bindingRole", "bindingOrigin",
        "sourceSection", "sourceSectionIndex", "parameterName", "normalizedParameterName",
        "valueVariant", "valueF64", "valueF64x4", "valueTextureId", "typedValueSha256",
        "sourceFieldValueSha256", "sourceLineageSha256", "sourceExact", "rowSha256",
    ),
    "materialStaticBindings": (
        "fieldId", "recipeId", "order", "parameterName", "normalizedParameterName",
        "bindingOrigin", "selectionRole", "sourceValue", "selectedValue", "policyRowId",
        "sourceFieldValueSha256", "sourceLineageSha256", "sourceExact", "rowSha256",
    ),
    "materialRenderBindings": (
        "renderBindingId", "recipeId", "order", "fieldName", "bindingOrigin",
        "sourceStatus", "sourceFidelity", "valueVariant", "boolValue", "enumValue",
        "f64Value", "sourceRecordSha256", "policyRowId", "sourceBlocker", "sourceExact",
        "rowSha256",
    ),
    "materialOccurrences": (
        "occurrenceId", "order", "emitterId", "cueId", "rendererType", "recipeId",
        "familyId", "evaluatorRegistryId", "sourceOccurrenceIdentitySha256",
        "bindingSha256", "sourceOccurrenceBindingSha256", "preservedBlockers",
        "sourceExact", "rowSha256",
    ),
    "materialPolicyRows": (
        "policyRowId", "order", "domain", "sourceMatrixRowId", "recipeId",
        "materialOccurrenceIds", "fieldId", "fieldKind", "bindingOrigin",
        "evidenceOwnerRecipeId", "policyFidelity", "sourceExact", "evidenceBlockers",
        "policySelectionAdmission", "implementationRegistryId", "implementationId",
        "implementationVersion", "consumerContract", "valueVariant", "boolValue",
        "f64Value", "enumType", "enumValue", "enumOrdinal", "samplerDescriptor",
        "providerBasisSha256", "numericOracleSha256", "d3dStateOracleId",
        "d3dDescriptorOracle", "d3dSrvOracle", "approvalPolicyRowId",
        "approvalUpstreamIdentitySha256", "approvalPolicyBindingSha256",
        "approvalPolicyFamilyId", "approvalMaterialOrder", "approvalRequiredOracleIds",
        "approvalExecutionBlockers", "approvalMaterialRow", "approvalMaterialRowSha256", "sourceRowSha256",
        "rowSha256",
    ),
    "materialTextureBindings": (
        "bindingId", "order", "recipeId", "materialInputFieldId",
        "logicalTexturePath", "samplerPolicyRowId", "materialOccurrenceIds",
        "sourceBindingId", "sourceBindingRowSha256", "sourceTextureResourceId",
        "sourceTextureResourceRowSha256", "sourceProvisioningProposalId",
        "sourceProvisioningProposalRowSha256", "sourceReceiptStatus",
        "runtimeAssetId", "resolutionStatus", "bindingBasis", "sourceExact",
        "blockers", "rowSha256",
    ),
    "rendererTextureResources": (
        "textureResourceId", "order", "emitterId", "sourceNode",
        "materialOccurrenceId", "sourceMaterialPath", "slotId", "assetId",
        "sourceResourceProjectionSha256", "projectionRole", "blockers", "sourceExact",
        "rowSha256",
    ),
    "geometryCarriers": (
        "carrierId", "order", "bindingId", "sourceObject", "assetId", "formatVersion",
        "candidateResourceByteSize", "candidateResourceSha256", "payloadSha256",
        "provenanceSha256", "provenanceRole", "metadataIdentitySha256",
        "cacheIdentitySha256", "geometryPreScale", "geometryPreScaleF32Hex",
        "channelMask", "evidenceFlags", "expectedTupleSha256", "submeshes",
        "approvalGeometryOrder", "approvalGeometryRow", "approvalGeometryRowSha256",
        "preparedCacheIdentitySha256", "sourceExact",
        "preScaleConsumed", "rowSha256",
    ),
    "geometryUses": (
        "geometryUseId", "order", "emitterId", "moduleId", "sourceEmitterPath",
        "sourceMeshPackageRef", "carrierId", "assetId", "sizeSemantics",
        "preScaleApplication", "preScaleConsumed", "sourceExact", "rowSha256",
    ),
}

TARGET_KEYS = (
    "characterClass", "skillId", "inputSlot", "policyTargetId", "sourceCandidateId",
    "geometryBindingAssetId", "runtimeCatalogAssetId", "identityRouteId",
    "identityRouteSha256",
)
ADMISSION_KEYS = (
    "artifactBinding", "policyRoute", "sourceHandlerSelection",
    "distributionEvaluatorSelection", "materialPolicySelection", "geometryBinding",
    "sourceExact", "runtimeExecution", "product", "blockers",
)
SUMMARY_KEYS = (
    "inputArtifactCount", "handlerRegistryCount", "emitterCount", "actionScheduleCount", "moduleCount",
    "moduleSelectionCounts", "propertyCount", "primitiveLeafCount", "literalCount", "distributionCount",
    "distributionVariantCounts", "seedPolicyCount", "implicitDefaultCount",
    "pointLightFieldCount", "materialFamilyCount", "materialRecipeCount",
    "materialInputCount", "materialStaticBindingCount", "materialRenderBindingCount",
    "materialOccurrenceCount", "materialPolicyCount", "materialPolicyDomainCounts",
    "materialD3dDescriptorCount", "materialD3dDescriptorProjectionSha256",
    "materialSrvDescriptorCount", "materialSrvDescriptorProjectionSha256",
    "materialTextureBindingCount", "resolvedMaterialTextureBindingCount",
    "rendererTextureResourceCount", "rendererTextureSlotCounts",
    "geometryCarrierCount", "geometryUseCount", "unknownHandlerCount",
    "ownerlessRowCount", "duplicateIdCount", "sourceExactRowCount",
    "runtimeExecution", "product",
)

HANDLER_KINDS = {
    "SOURCE_MODULE", "SOURCE_PROPERTY", "SOURCE_PRIMITIVE", "RECONSTRUCTED_MODULE",
    "DISTRIBUTION", "MATERIAL_EVALUATOR", "MATERIAL_POLICY",
}
RENDERER_TYPES = {
    "SpriteParticle", "MeshParticle", "DecalParticle", "CascadeRibbon",
    "ScreenPost", "LightParticle",
}
LITERAL_VARIANTS = {"BOOL", "F64", "ENUM_STRING"}
DISTRIBUTION_VARIANTS = {
    "INLINE", "FLOAT_PARAMETER", "VECTOR_PARAMETER", "FLOAT_CURVE", "EF_MULTIPLY",
}
MATERIAL_VALUE_VARIANTS = {"BOOL", "F64", "F64X4", "TEXTURE_ID", "ENUM_STRING"}
MATERIAL_POLICY_DOMAINS = {"RENDER_STATE", "STATIC_PERMUTATION", "SAMPLER_DESCRIPTOR"}
RENDERER_TEXTURE_SLOTS = {"base", "noise", "mask", "emissive", "dissolve"}
POINT_LIGHT_VALUE_VARIANTS = {"BOOL", "F64", "COLOR_RGBA8", "GUID128"}
IMPLICIT_DEFAULT_VARIANTS = {
    "BOOL", "RIBBON_DEFAULTS", "DECAL_DEFAULTS", "SCREEN_POST_IRRELEVANT",
    "POINT_LIGHT_OWNER_REFERENCE",
}

ACTION_CUE_KEYS = (
    "name", "kind", "scalarValue", "vectorValue", "sourceIndex",
    "sourceValueByteOffset",
)
EMITTER_TIMING_KEYS = (
    "requiredModuleId", "spawnModuleId", "lifetimeModuleId", "emitterDelaySeconds",
    "emitterDelayPolicy", "emitterDurationSeconds", "emitterDurationPolicy",
    "emitterLoopCount", "bursts", "sourceProjectionSha256", "fidelity", "sourceExact",
    "blockers", "timingSha256",
)
BURST_KEYS = (
    "spawnModuleId", "burstIndex", "timeSeconds", "countMinimum", "countMaximum",
    "sourceLiteralProjectionSha256",
)
EMITTER_RANDOM_KEYS = (
    "policyId", "seedDerivationInputSha256", "emitterRandomSeed", "sourceExact",
    "blockers", "policySha256",
)
TRANSFORM_KEYS = (
    "position", "rotationDegrees", "revolutionDegreesPerSecond", "scale",
    "velocityPerSecond",
)
DETAIL_TRANSFORM_KEYS = TRANSFORM_KEYS + (
    "decision", "consumptionPolicy", "projectionSha256",
)
CUE_TRANSFORM_KEYS = ("sourcePositionUeUnits", "position", "rotationDegrees", "scale")
SOCKET_TRANSFORM_KEYS = ("position", "rotationDegrees", "scale")
ATTACHMENT_KEYS = (
    "enabled", "follow", "sourceAnchorSlotId", "runtimeAnchorSlotId", "runtimeBoneName",
    "socketLocalTransform",
)
TRANSFORM_INHERITANCE_KEYS = (
    "enabled", "masterEmitterId", "decision", "consumptionPolicy", "projectionSha256",
)
ANCHOR_REQUEST_KEYS = (
    "anchorRequestId", "order", "sourceKind", "sourceModuleId", "sourceAnchorSlotId",
    "runtimeAnchorSlotId", "runtimeBoneName", "socketLocalTransform", "follow",
)
RENDERER_RUNTIME_CONFIG_KEYS = (
    "color", "uv", "linearLerp", "mesh", "sprite", "decal", "trail",
    "afterImage", "screenPost", "sourceProjectionSha256", "fidelity", "sourceExact",
    "consumptionPolicy", "blockers",
)
RENDERER_COLOR_KEYS = (
    "offset", "multiply", "clip", "emissiveIntensity", "distortionIntensity",
    "distortionOnBaseMaterial", "radialTime", "radialIntensity",
)
RENDERER_UV_KEYS = (
    "start", "speed", "wave", "waveAmplitude", "waveFrequency", "sequence", "loop",
    "sequenceTerm", "tileColumns", "tileRows", "tileIndex",
)
RENDERER_LINEAR_LERP_KEYS = (
    "position", "endPosition", "rotation", "endRotationDegrees", "revolution",
    "endRevolutionDegreesPerSecond", "scale", "endScale", "velocity",
    "endVelocityPerSecond", "colorOffset", "endColorOffset", "colorMultiply",
    "endColorMultiply", "emissiveIntensity", "endEmissiveIntensity",
)
RENDERER_MESH_KEYS = ("useModelMaterial",)
RENDERER_SPRITE_KEYS = ("billboard", "billboardRollDegrees")
RENDERER_DECAL_KEYS = ("size", "depth")
RENDERER_TRAIL_KEYS = (
    "maxPoints", "pointLifeTimeSeconds", "sampleIntervalSeconds", "minimumDistance",
    "startWidth", "endWidth", "faceCamera",
)
RENDERER_AFTER_IMAGE_KEYS = (
    "sampleIntervalSeconds", "maxCopies", "alphaExponent",
)
RENDERER_SCREEN_POST_KEYS = (
    "enabled", "profileId", "status", "intensity", "secondaryIntensity",
    "frequency", "tint", "randomSeed",
)
SCREEN_POST_ADAPTER_KEYS = (
    "adapterId", "enabled", "occurrenceId", "materialOccurrenceId", "recipeId",
    "familyId", "intensityDistributionId", "alphaDistributionId",
    "secondaryIntensity", "frequencyPolicy", "tint", "sourceSpace", "fidelity",
    "sourceExact", "blockers", "adapterSha256",
)
LIGHT_ADAPTER_KEYS = (
    "adapterId", "enabled", "moduleId", "fieldIds", "positionSourcePolicy",
    "ueUnitScale", "sourceExact", "blockers", "adapterSha256",
)
DECAL_ADAPTER_KEYS = (
    "adapterId", "enabled", "moduleId", "defaultId", "sizeDistributionId",
    "nearPlane", "farPlane", "defaultSize", "blendRange", "yawOnlyCdoDefault",
    "yawOnlyCapabilityOutput", "yawOnlyDecision", "executionAdmission",
    "supports3dDrawMode", "depthWorldUnits", "depthRuntimeUnits", "fidelity",
    "sourceExact", "blockers", "adapterSha256",
)
RIBBON_ADAPTER_KEYS = (
    "adapterId", "enabled", "typeDataModuleId", "defaultId", "spawnModuleId",
    "lifetimeModuleId", "sizeModuleId", "colorModuleIds", "dynamicParameterModuleId",
    "spawnDistributionIds", "lifetimeDistributionIds", "sizeDistributionIds",
    "colorDistributionIds", "dynamicDistributionIds", "tilingDistance",
    "distanceTessellationStepSize", "typedMaxParticleInTrailCount",
    "operationalMaxPoints", "widthPolicy", "geometryPolicy", "orientationPolicy", "fidelity",
    "sourceExact", "blockers", "adapterSha256",
)
PARTICLE_SYSTEM_POLICY_KEYS = (
    "uniformScaleMultiplier", "yawOffsetDegrees", "directionYawDegrees",
    "initialSpeedMultiplier", "policySha256",
)
POLICY_ROUTE_BINDING_KEYS = (
    "approvalPolicyId", "approvalPolicyVersion", "approvalReceiptSha256",
    "sourceExecutionFamilyCount", "sourceExecutionFamilies",
    "sourceExecutionFamilyProjectionSha256",
    "sourcePolicyRowCount", "sourcePolicyRowProjectionSha256",
    "materialExecutionFamilyCount", "materialExecutionFamilies",
    "materialExecutionFamilyProjectionSha256",
    "materialPolicyRowCount", "materialPolicyRowProjectionSha256",
    "materialArithmeticRowCount", "materialArithmeticRowProjectionSha256",
    "geometryPolicyRowCount", "geometryPolicyRowProjectionSha256",
    "sourceCapabilityReceiptSha256", "sourceCapabilityCanonicalSha256",
    "materialPolicyReceiptSha256", "materialPolicyCanonicalSha256", "bindingSha256",
)
MATERIAL_EVALUATOR_ORACLE_CONTRACT_KEYS = (
    "version", "operationOrder", "inputSampleCountPerFamily", "inputSamples",
    "numericTolerance", "fidelity", "sourceExact", "contractSha256",
)
MATERIAL_EVALUATOR_INPUT_KEYS = (
    "sampleId", "time", "uvScale", "panRotationAux", "texture0", "texture1",
    "color", "params0", "params1",
)
SECTION_DIGEST_KEYS = ("sectionName", "rowCount", "orderedSha256")
CURVE_KEY_KEYS = (
    "time", "minimum", "maximum", "arriveTangentMinimum", "leaveTangentMinimum",
    "arriveTangentMaximum", "leaveTangentMaximum", "interpolation",
)
FIELD_PROVENANCE_KEYS = (
    "fieldPath", "provenanceTier", "evidenceStatus", "valueSha256",
    "typedValueBindingSha256",
)
DIST_SAMPLE_KEYS = (
    "sampleId", "domain", "inputVariant", "time", "randomUnits", "sourceCueId", "branch",
    "parameterInput", "diagnosticStandardBaseValue", "blocked",
    "actionCueBindingsSha256", "inputSha256", "outputSha256", "outputValues",
    "absoluteTolerance", "relativeTolerance",
)
PARAMETER_SAMPLE_INPUT_KEYS = (
    "name", "kind", "value", "sourceIndex", "sourceValueByteOffset",
)
CAPABILITY_SAMPLE_KEYS = (
    "sampleId", "order", "ownerModuleId", "time", "fixedSeed", "fixedSeedSource",
    "randomUnits", "inputVariant", "inputLiteralProjectionSha256",
    "inputDistributionProjectionSha256", "inputValues", "outputVariant", "outputValues",
    "typedInputSha256", "outputSha256", "absoluteTolerance", "relativeTolerance",
)
D3D_DESCRIPTOR_ORACLE_KEYS = (
    "policyRowId", "descriptorKind", "expectedDescriptor", "actualDescriptor",
    "numericTolerance", "decision",
)
D3D_SRV_ORACLE_KEYS = (
    "policyRowId", "expectedSrv", "actualSrv", "numericTolerance", "decision",
)
D3D_STENCIL_FACE_KEYS = (
    "StencilFailOp", "StencilDepthFailOp", "StencilPassOp", "StencilFunc",
)
D3D_DEPTH_STENCIL_KEYS = (
    "DepthEnable", "DepthWriteMask", "DepthFunc", "StencilEnable",
    "StencilReadMask", "StencilWriteMask", "FrontFace", "BackFace",
)
D3D_RASTERIZER_KEYS = (
    "FillMode", "CullMode", "FrontCounterClockwise", "DepthBias", "DepthBiasClamp",
    "SlopeScaledDepthBias", "DepthClipEnable", "ScissorEnable",
    "MultisampleEnable", "AntialiasedLineEnable",
)
D3D_SAMPLER_KEYS = (
    "Filter", "AddressU", "AddressV", "AddressW", "MipLODBias", "MaxAnisotropy",
    "ComparisonFunc", "BorderColor", "MinLOD", "MaxLOD",
)
D3D_SRV_KEYS = (
    "Format", "ViewDimension", "MostDetailedMip", "MipLevels", "srvColorSpace",
)
D3D_DESCRIPTOR_KINDS = {
    "D3D11_DEPTH_STENCIL_DESC", "D3D11_RASTERIZER_DESC", "D3D11_SAMPLER_DESC",
}
APPROVAL_SOURCE_ROW_KEYS = (
    "policyRowId", "upstreamIdentitySha256", "policyBindingSha256",
    "exactSourceClass", "moduleOccurrenceId", "upstreamClusterId",
    "requiredMutatedOutput", "upstreamDecision", "policyFamilyId",
    "evidenceFidelity", "executionFidelity", "sourceExact",
    "preservedEvidenceBlockers", "policyFidelityGuards", "requiredOracleIds",
    "executionBlockers", "executionAdmission", "productAdmission",
)
APPROVAL_SOURCE_FAMILY_KEYS = (
    "upstreamClusterId", "upstreamNativeFamily", "policyFamilyId", "closureBasis",
    "moduleOccurrenceCount", "requiredMutatedOutputs", "requiredOracleIds",
    "sourceExact", "executionAdmission",
)
APPROVAL_MATERIAL_FAMILY_KEYS = (
    "policyFamilyId", "closureBasis", "requiredOracleIds",
)
APPROVAL_MATERIAL_ROW_KEYS = (
    "policyRowId", "upstreamIdentitySha256", "policyBindingSha256", "domain",
    "upstreamMatrixRowId", "materialRecipeId", "materialOccurrenceIds", "fieldId",
    "fieldKind", "bindingOrigin", "upstreamDecision", "policyFamilyId",
    "evidenceFidelity", "executionFidelity", "sourceValueAcquired",
    "partialSourceExactFields", "previousSamplerAdmission", "fullDescriptorSourceExact",
    "sourceExact", "preservedEvidenceBlockers", "policyFidelityGuards",
    "requiredOracleIds", "executionBlockers", "executionAdmission", "productAdmission",
)
APPROVAL_ARITHMETIC_ROW_KEYS = (
    "policyRowId", "upstreamFamilyId", "familyIdentitySha256", "evaluatorId",
    "evaluatorVersion", "evaluatorSha256", "policyFamilyId", "evidenceFidelity",
    "sourceExact", "cpuNumericOracleVerified", "hlslNumericOracleVerified",
    "preservedEvidenceBlockers", "requiredOracleIds", "executionBlockers",
    "executionAdmission", "productAdmission",
)
APPROVAL_GEOMETRY_ROW_KEYS = (
    "assetId", "sourceObject", "candidateResourceSha256", "payloadSha256",
    "metadataIdentitySha256", "geometryPreScale", "artifactBindingIntegrity",
    "sourceFidelity", "sourceExact", "executionBlockers", "executionAdmission",
    "productAdmission",
)
MATERIAL_FAMILY_SAMPLE_KEYS = (
    "sampleId", "order", "inputSha256", "expectedFloat4",
)
MATERIAL_RECIPE_SAMPLE_KEYS = (
    "sampleId", "order", "time", "uvScale", "panRotationAux", "texture0",
    "texture1", "color", "params0", "params1", "inputSha256", "expectedFloat4",
)
RIBBON_VALUE_KEYS = (
    "maxTessellationBetweenParticles", "sheetsPerTrail", "maxTrailCount",
    "maxParticleInTrailCount", "deadTrailsOnDeactivate", "deadTrailsOnSourceLoss",
    "clipSourceSegment", "enablePreviousTangentRecalculation", "renderGeometry",
    "distanceTessellationStepSize", "tangentTessellationScalar",
)
DECAL_VALUE_KEYS = (
    "defaultSize", "farPlane", "blendRange", "onlyCalculateRotationYaw",
    "supports3dDrawMode",
)
SAMPLER_DESCRIPTOR_KEYS = (
    "type", "filterUe3", "filterD3d11", "addressUUe3", "addressUD3d11",
    "addressVUe3", "addressVD3d11", "addressWUe3", "addressWD3d11", "mipLODBias",
    "maxAnisotropy", "comparisonFuncName", "comparisonFuncD3d11", "borderColor",
    "minLOD", "maxLOD", "sRgb", "srvColorSpace", "lodGroup",
)
GEOMETRY_SUBMESH_KEYS = (
    "name", "materialIndex", "vertexCount", "indexCount", "channelCounts",
    "channelSha256", "boundsF32Hex",
)
GEOMETRY_CHANNEL_COUNT_KEYS = (
    "position", "normal", "tangentXyz", "tangentW", "uv0", "color0",
)
GEOMETRY_CHANNEL_SHA_KEYS = (
    "position", "normal", "tangentXyz", "tangentW", "uv0", "color0", "indicesU32",
)

EXPECTED_ACTION_SCHEDULES = (
    (
        "skill-31470/clip-000/notify-000", "action-31470/stage-000/notify-000",
        "fx_pc_sdm_07.par_v_sdm_ink_spw_01", 0, 0.0, 1.501613974571228,
        "2d102a3068cb7590a006cc2730c627b5da31ced1b0d3bf26dbae410d056a6d9c",
    ),
    (
        "skill-31470/clip-000/notify-014", "action-31470/stage-000/notify-014",
        "fx_pc_sdm_07.par_v_smd_onestroke_weapon_01", 1,
        1.3377399444580078, 0.2844119966030121,
        "60b16532d2de74d993bbe6655a1cb00cbf2b4f90dd1f99db363bb5db6874fce3",
    ),
    (
        "skill-31470/clip-000/notify-018", "action-31470/stage-000/notify-018",
        "fx_pc_sdm_07.par_v_smd_onestroke_swing_01", 5,
        1.3803969621658325, 0.0,
        "75b13df35a1943b8a8781aaec33a4dda32dec7c7a1eff653652d1e2d1fafb1f0",
    ),
    (
        "skill-31470/clip-000/notify-022", "action-31470/stage-000/notify-022",
        "fx_pc_sdm_07.par_v_sdm_onestroke_hit_01", 9,
        1.4506419897079468, 0.0,
        "d1aca7538245ca5a28af6c8f99e5f27ced5a2cb4519cf065731ff027aee61b88",
    ),
    (
        "skill-31470/clip-000/notify-026", "action-31470/stage-000/notify-026",
        "fx_post.fx_par.par_c_zoomblur_03", 13,
        1.4511719942092896, 0.0,
        "c45c2dd6f5f18f00e46b0811de3896f6482118278e2e7f8f719d25179ca9fd5c",
    ),
    (
        "skill-31470/clip-000/notify-028", "action-31470/stage-000/notify-028",
        "fx_cm_01.distortion_onelayer.par_convatedisol_fsm_pushinghit_01", 15,
        1.4520330429077148, 0.0,
        "8875647b1a2c0debf3f6c66059c40d96698275f6e8e4a7a4204ac5e731b7bd9d",
    ),
    (
        "skill-31470/clip-000/notify-029", "action-31470/stage-000/notify-029",
        "fx_cm_02.light.par_mp_light_01", 16,
        1.4527740478515625, 0.0,
        "1da4a1e03bdc47637320c7d684031bf1a543be699e4a1e3bd9155161c6d32f15",
    ),
)

SECTION_MAX_COUNTS = {
    "inputArtifacts": 16,
    "handlerRegistry": 4096,
    "emitters": 35,
    "actionSchedules": 7,
    "modules": 399,
    "properties": 1434,
    "primitiveLeaves": 1572,
    "literals": 1590,
    "distributions": 629,
    "seedPolicies": 14,
    "implicitDefaults": 14,
    "pointLightFields": 8,
    "materialFamilies": 23,
    "materialRecipes": 27,
    "materialInputs": 729,
    "materialStaticBindings": 94,
    "materialRenderBindings": 162,
    "materialOccurrences": 34,
    "materialPolicyRows": 255,
    "materialTextureBindings": 72,
    "rendererTextureResources": 57,
    "geometryCarriers": 7,
    "geometryUses": 13,
}


class ContractError(RuntimeError):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ContractError(message)


def reject_duplicate_keys(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        require(key not in result, f"duplicate JSON object key: {key}")
        result[key] = value
    return result


def load_json_bytes(raw: bytes, label: str) -> dict[str, Any]:
    require(not raw.startswith(b"\xef\xbb\xbf"), f"{label}: UTF-8 BOM is forbidden")
    try:
        text = raw.decode("utf-8")
    except UnicodeDecodeError as exc:
        raise ContractError(f"{label}: invalid UTF-8") from exc
    try:
        value = json.loads(
            text,
            object_pairs_hook=reject_duplicate_keys,
            parse_constant=lambda token: (_ for _ in ()).throw(
                ContractError(f"{label}: non-finite number {token} is forbidden")
            ),
        )
    except json.JSONDecodeError as exc:
        raise ContractError(f"{label}: invalid JSON: {exc}") from exc
    require(type(value) is dict, f"{label}: root must be an object")
    assert_finite_tree(value, label)
    return value


def load_json(path: Path) -> dict[str, Any]:
    return load_json_bytes(path.read_bytes(), str(path))


def assert_finite_tree(value: Any, label: str) -> None:
    if type(value) is float:
        require(math.isfinite(value), f"{label}: non-finite float")
    elif type(value) is list:
        for item in value:
            assert_finite_tree(item, label)
    elif type(value) is dict:
        for item in value.values():
            assert_finite_tree(item, label)


def canonical_json_bytes(value: Any) -> bytes:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_json_bytes(value)).hexdigest()


def canonical_lf_text_sha256(raw: bytes) -> str:
    require(not raw.startswith(b"\xef\xbb\xbf"), "tracked input BOM is forbidden")
    text = raw.decode("utf-8").replace("\r\n", "\n").replace("\r", "\n")
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def run_git(*args: str, binary: bool = False) -> Any:
    completed = subprocess.run(
        ["git", *args], cwd=REPO_ROOT, check=False,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        text=not binary,
    )
    if completed.returncode != 0:
        stderr = completed.stderr.decode("utf-8", "replace") if binary else completed.stderr
        raise ContractError(f"git {' '.join(args)} failed: {stderr.strip()}")
    return completed.stdout if binary else completed.stdout.strip()


def git_blob(commit: str, path: str) -> bytes:
    return run_git("show", f"{commit}:{path}", binary=True)


def verify_self_hash(value: dict[str, Any], field: str, label: str) -> str:
    if not field:
        return ""
    require(field in value and type(value[field]) is str, f"{label}: missing {field}")
    unsigned = copy.deepcopy(value)
    expected = unsigned.pop(field)
    require(canonical_sha256(unsigned) == expected, f"{label}: {field} mismatch")
    return expected


def seal_row(row: dict[str, Any], section: str) -> dict[str, Any]:
    require("rowSha256" not in row, f"{section}: row already sealed")
    expected_keys = ROW_KEYS[section][:-1]
    require(tuple(row.keys()) == expected_keys, f"{section}: builder row key drift")
    row["rowSha256"] = canonical_sha256(row)
    return row


def require_keys(value: dict[str, Any], keys: Iterable[str], label: str) -> None:
    require(tuple(value.keys()) == tuple(keys), f"{label}: exact key set/order mismatch")


def flatten_blockers(value: Any) -> list[str]:
    result: set[str] = set()
    if type(value) is str:
        if value:
            result.add(value)
    elif type(value) is list:
        for item in value:
            result.update(flatten_blockers(item))
    elif type(value) is dict:
        for item in value.values():
            result.update(flatten_blockers(item))
    return sorted(result)


def is_blocker_owner_key(key: str) -> bool:
    return key != "blockerUnion" and "blocker" in key.casefold()


def collect_owned_blockers(value: Any, result: set[str]) -> None:
    if type(value) is dict:
        for key, item in value.items():
            if is_blocker_owner_key(key):
                result.update(flatten_blockers(item))
            collect_owned_blockers(item, result)
    elif type(value) is list:
        for item in value:
            collect_owned_blockers(item, result)


def derive_program_blockers_from_sections(
    sections: dict[str, list[dict[str, Any]]],
) -> list[str]:
    blockers = set(REQUIRED_GLOBAL_PROGRAM_BLOCKERS)
    for rows in sections.values():
        collect_owned_blockers(rows, blockers)
    blockers.discard("")
    return sorted(blockers)


def blocker_ownership_projection(
    sections: dict[str, list[dict[str, Any]]],
) -> list[dict[str, Any]]:
    projection: list[dict[str, Any]] = []

    def walk(
        value: Any, section: str, row_index: int, owner_id: str,
        path: list[str | int],
    ) -> None:
        if type(value) is dict:
            for key, item in value.items():
                child_path = [*path, key]
                if is_blocker_owner_key(key):
                    projection.append({
                        "section": section,
                        "rowIndex": row_index,
                        "ownerId": owner_id,
                        "path": child_path,
                        "value": copy.deepcopy(item),
                    })
                walk(item, section, row_index, owner_id, child_path)
        elif type(value) is list:
            for index, item in enumerate(value):
                walk(item, section, row_index, owner_id, [*path, index])

    for section in SECTION_NAMES:
        rows = sections[section]
        id_field = PRIMARY_ID_FIELDS[section]
        for row_index, row in enumerate(rows):
            walk(row, section, row_index, row[id_field], [])
    return projection


def build_blocker_ownership_contract(
    sections: dict[str, list[dict[str, Any]]],
) -> dict[str, Any]:
    projection = blocker_ownership_projection(sections)
    value = {
        "version": 1,
        "fieldCount": len(projection),
        "tokenOccurrenceCount": sum(
            len(flatten_blockers(row["value"])) for row in projection
        ),
        "projectionSha256": canonical_sha256(projection),
    }
    require_keys(value, BLOCKER_OWNERSHIP_KEYS, "blocker ownership contract")
    return value


def finite_json_number(value: Any, label: str) -> float:
    require(type(value) in (int, float) and type(value) is not bool,
            f"{label} must be a non-bool JSON number")
    result = float(value)
    require(math.isfinite(result), f"{label} must be finite")
    return result


def vector_values(value: Any, count: int) -> list[float]:
    if type(value) in (int, float) and type(value) is not bool:
        result = [finite_json_number(value, "typed vector scalar")]
    elif type(value) is list:
        result = [
            finite_json_number(item, f"typed vector component {index}")
            for index, item in enumerate(value)
        ]
    elif type(value) is dict:
        component_keys = ("x", "y", "z", "w")[:count]
        result = [
            finite_json_number(value[key], f"typed vector component {key}")
            for key in component_keys
        ]
    else:
        raise ContractError(f"cannot convert typed vector from {type(value).__name__}")
    require(len(result) == count, f"expected {count} vector components, got {len(result)}")
    assert_finite_tree(result, "typed vector")
    return result


def nullable_sha(value: Any) -> str:
    return value if type(value) is str else ""


def material_value_slots(typed: Any) -> tuple[str, Any, Any, str, str, str, Any]:
    """Return variant, bool, f64, enumType, enumValue, texture, float4."""
    if type(typed) is bool:
        return "BOOL", typed, None, "", "", "", []
    if type(typed) in (int, float) and type(typed) is not bool:
        return "F64", None, finite_json_number(typed, "material scalar"), "", "", "", []
    if type(typed) is str:
        return "TEXTURE_ID", None, None, "", "", typed, []
    if type(typed) is list:
        require(len(typed) == 4, "material vector must have four components")
        return "F64X4", None, None, "", "", "", [
            finite_json_number(value, f"material vector component {index}")
            for index, value in enumerate(typed)
        ]
    raise ContractError(f"unsupported material value type: {type(typed).__name__}")


def typed_policy_slots(selected: dict[str, Any] | None) -> tuple[str, Any, Any, str, str, Any]:
    if selected is None:
        return "SAMPLER_DESCRIPTOR", None, None, "", "", None
    require(type(selected) is dict and type(selected.get("type")) is str,
            "material policy selectedValue must be typed")
    kind = selected["type"]
    if kind == "BOOL":
        require(type(selected.get("value")) is bool, "BOOL policy requires bool")
        return "BOOL", selected["value"], None, "", "", None
    if kind == "FLOAT32":
        require(type(selected.get("value")) in (int, float) and type(selected["value"]) is not bool,
                "FLOAT32 policy requires number")
        return "F64", None, float(selected["value"]), "", "", None
    if kind == "ENUM":
        require(type(selected.get("enumType")) is str, "ENUM requires enumType")
        require(type(selected.get("value")) is str, "ENUM requires value")
        require(type(selected.get("ordinal")) is int, "ENUM requires integer ordinal")
        return "ENUM_STRING", None, None, selected["enumType"], selected["value"], selected["ordinal"]
    raise ContractError(f"unsupported material policy selectedValue type: {kind}")


def git_authority_bundle() -> tuple[dict[str, dict[str, Any]], list[dict[str, Any]]]:
    documents: dict[str, dict[str, Any]] = {}
    rows: list[dict[str, Any]] = []
    for order, (artifact_id, path, commit, self_field) in enumerate(AUTHORITIES):
        raw = git_blob(commit, path)
        value = load_json_bytes(raw, f"{commit}:{path}")
        self_sha = verify_self_hash(value, self_field, artifact_id)
        tree_id = run_git("rev-parse", f"{commit}^{{tree}}")
        blob_id = run_git("rev-parse", f"{commit}:{path}")
        version_field = ""
        version_value = 0
        for candidate in ("formatVersion", "schemaVersion", "version"):
            if candidate in value:
                require(type(value[candidate]) is int,
                        f"{artifact_id}: {candidate} must be an exact JSON integer")
                version_field = candidate
                version_value = value[candidate]
                break
        row = {
            "artifactId": artifact_id,
            "order": order,
            "path": path,
            "authorityCommitId": commit,
            "authorityTreeId": tree_id,
            "blobId": blob_id,
            "schema": value.get("schema", ""),
            "versionField": version_field,
            "versionValue": version_value,
            "hashDomain": "GIT_OBJECT_CANONICAL_LF_JSON",
            "trackedTextSha256": canonical_lf_text_sha256(raw),
            "canonicalJsonSha256": canonical_sha256(value),
            "selfHashField": self_field,
            "selfSha256": self_sha,
        }
        rows.append(seal_row(row, "inputArtifacts"))
        documents[artifact_id] = value
    return documents, rows


_EXPECTED_INPUT_ARTIFACTS_CANONICAL: bytes | None = None
_FROZEN_SOURCE_CAPABILITY_BY_MODULE: dict[str, dict[str, Any]] | None = None
_FROZEN_EMITTER_IDENTITY_CANONICAL: bytes | None = None


def expected_input_artifacts_canonical() -> bytes:
    global _EXPECTED_INPUT_ARTIFACTS_CANONICAL
    if _EXPECTED_INPUT_ARTIFACTS_CANONICAL is None:
        _, rows = git_authority_bundle()
        _EXPECTED_INPUT_ARTIFACTS_CANONICAL = canonical_json_bytes(rows)
    return _EXPECTED_INPUT_ARTIFACTS_CANONICAL


def frozen_source_capability_by_module() -> dict[str, dict[str, Any]]:
    global _FROZEN_SOURCE_CAPABILITY_BY_MODULE
    if _FROZEN_SOURCE_CAPABILITY_BY_MODULE is None:
        raw = git_blob(SOURCE_CAPABILITY_COMMIT, SOURCE_CAPABILITY_PATH)
        receipt = load_json_bytes(
            raw, f"{SOURCE_CAPABILITY_COMMIT}:{SOURCE_CAPABILITY_PATH}"
        )
        verify_self_hash(receipt, "receiptSha256", "sourceCapability")
        require(receipt.get("schema")
                == "lostark.effect-reconstructed-source-capability"
                and type(receipt.get("formatVersion")) is int
                and receipt["formatVersion"] == 1
                and receipt.get("characterClass") == "ARTIST"
                and receipt.get("skillId") == 31470
                and receipt.get("inputSlot") == "F",
                "frozen Source capability identity mismatch")
        rows = {
            row["moduleOccurrenceId"]: row for row in receipt["occurrences"]
        }
        require(len(rows) == len(receipt["occurrences"]) == 29,
                "frozen Source capability occurrence denominator mismatch")
        _FROZEN_SOURCE_CAPABILITY_BY_MODULE = rows
    return _FROZEN_SOURCE_CAPABILITY_BY_MODULE


def frozen_emitter_identity_canonical() -> bytes:
    global _FROZEN_EMITTER_IDENTITY_CANONICAL
    if _FROZEN_EMITTER_IDENTITY_CANONICAL is None:
        source = load_json_bytes(
            git_blob(SOURCE_EVIDENCE_COMMIT, SOURCE_SEMANTICS_PATH),
            f"{SOURCE_EVIDENCE_COMMIT}:{SOURCE_SEMANTICS_PATH}",
        )
        verify_self_hash(source, "receiptSha256", "sourceSemantics")
        candidate = load_json_bytes(
            git_blob(SOURCE_EVIDENCE_COMMIT, SOURCE_CANDIDATE_PATH),
            f"{SOURCE_EVIDENCE_COMMIT}:{SOURCE_CANDIDATE_PATH}",
        )
        candidate_by_source_node = {
            row["sourceNode"]: row for row in candidate["elements"]
        }
        require(len(candidate_by_source_node) == len(candidate["elements"]),
                "frozen Source candidate has duplicate sourceNode")
        projection: list[dict[str, Any]] = []
        for order, occurrence in enumerate(source["occurrences"]):
            source_node = (
                occurrence["sourceSystemId"] + "|" + occurrence["sourceEmitterPath"]
            )
            element = candidate_by_source_node.get(source_node)
            require(element is not None,
                    f"frozen Source candidate lacks {source_node}")
            compiler = element["sourceRecipe"]["compilerEvidence"]
            require(compiler["evidenceId"] == occurrence["evidenceId"]
                    and compiler["sourceEmitterPath"]
                    == occurrence["sourceEmitterPath"],
                    f"frozen Source emitter compiler join mismatch: {source_node}")
            projection.append({
                "emitterId": occurrence["occurrenceCompositeId"],
                "order": order,
                "evidenceId": occurrence["evidenceId"],
                "sourceOccurrenceId": occurrence["sourceOccurrenceId"],
                "sourceSystemId": occurrence["sourceSystemId"],
                "sourceEmitterPath": occurrence["sourceEmitterPath"],
                "sourceEmitterNodeId": compiler["sourceEmitterNodeId"],
                "sourceCueId": compiler["sourceCueId"],
                "sourceElementId": element["id"],
                "sourceNode": element["sourceNode"],
                "visible": element["visible"],
                "rendererType": occurrence["rendererType"],
                "rendererSourceSpace": element["renderer"]["sourceSpace"],
                "selectedLodPath": occurrence["selectedLod"]["sourceLodPath"],
                "selectedLodNodeId": occurrence["selectedLod"]["sourceLodNodeId"],
                "selectedLodRecordSha256": (
                    occurrence["selectedLod"]["sourceLodRecordSha256"]
                ),
            })
        require(len(projection) == 35,
                "frozen Source emitter identity denominator mismatch")
        _FROZEN_EMITTER_IDENTITY_CANONICAL = canonical_json_bytes(projection)
    return _FROZEN_EMITTER_IDENTITY_CANONICAL


class HandlerRegistry:
    def __init__(self) -> None:
        self.rows: list[dict[str, Any]] = []
        self.by_identity: dict[tuple[str, str, int, str, str, str, str], str] = {}

    def get(
        self,
        handler_kind: str,
        implementation_id: str,
        *,
        implementation_version: int = 1,
        implementation_sha256: str = "",
        exact_source_class: str = "",
        variant: str = "",
        consumer_contract: str = "",
    ) -> str:
        require(handler_kind in HANDLER_KINDS, f"unknown handler kind: {handler_kind}")
        require(type(implementation_id) is str and implementation_id,
                "handler implementationId must be non-empty")
        require(type(implementation_version) is int and implementation_version > 0,
                "handler implementationVersion must be a positive int")
        if not implementation_sha256:
            implementation_sha256 = canonical_sha256({
                "reviewedStandardHandlerId": implementation_id,
                "implementationVersion": implementation_version,
                "exactSourceClass": exact_source_class,
                "variant": variant,
                "consumerContract": consumer_contract,
            })
        require(type(implementation_sha256) is str and len(implementation_sha256) == 64,
                "handler implementationSha256 must be SHA-256 hex")
        identity = (
            handler_kind, implementation_id, implementation_version,
            implementation_sha256, exact_source_class, variant, consumer_contract,
        )
        existing = self.by_identity.get(identity)
        if existing:
            return existing
        contract = {
            "handlerKind": handler_kind,
            "implementationId": implementation_id,
            "implementationVersion": implementation_version,
            "implementationSha256": implementation_sha256,
            "exactSourceClass": exact_source_class,
            "variant": variant,
            "consumerContract": consumer_contract,
        }
        registry_id = "handler-" + canonical_sha256(contract)[:24]
        row = {
            "handlerRegistryId": registry_id,
            "order": len(self.rows),
            **contract,
            "contractSha256": canonical_sha256(contract),
            "sourceExact": False,
        }
        self.rows.append(seal_row(row, "handlerRegistry"))
        self.by_identity[identity] = registry_id
        return registry_id


def typed_action_cue_bindings(rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []
    for row in rows:
        require(type(row) is dict, "ActionCue binding must be an object")
        kind = row.get("kind")
        require(kind in ("scalar", "vector"), "unknown ActionCue parameter kind")
        value = row.get("value")
        scalar = None
        vector: list[float] = []
        if kind == "scalar":
            require(type(value) in (int, float) and type(value) is not bool,
                    "scalar ActionCue value must be numeric")
            scalar = float(value)
        else:
            require(type(value) is list and len(value) in (3, 4),
                    "vector ActionCue value must have three or four components")
            vector = [float(item) for item in value]
        item = {
            "name": row.get("name", ""),
            "kind": kind.upper(),
            "scalarValue": scalar,
            "vectorValue": vector,
            "sourceIndex": row.get("sourceIndex", -1),
            "sourceValueByteOffset": row.get("sourceValueByteOffset", -1),
        }
        require_keys(item, ACTION_CUE_KEYS, "ActionCue binding")
        result.append(item)
    return result


def typed_renderer_runtime_config(
    element: dict[str, Any], renderer_type: str,
) -> dict[str, Any]:
    detail = element["detail"]
    if renderer_type == "LightParticle":
        projection: dict[str, Any] = {
            "color": None, "uv": None, "linearLerp": None, "mesh": None,
            "sprite": None, "decal": None, "trail": None, "afterImage": None,
            "screenPost": None,
        }
    else:
        color_raw = detail["color"]
        color = {
            "offset": [float(v) for v in color_raw["offset"]],
            "multiply": [float(v) for v in color_raw["multiply"]],
            "clip": float(color_raw["clip"]),
            "emissiveIntensity": float(color_raw["emissiveIntensity"]),
            "distortionIntensity": float(color_raw["distortionIntensity"]),
            "distortionOnBaseMaterial": color_raw["distortionOnBaseMaterial"],
            "radialTime": float(color_raw["radialTime"]),
            "radialIntensity": float(color_raw["radialIntensity"]),
        }
        require_keys(color, RENDERER_COLOR_KEYS, "renderer color config")
        uv_raw = detail["uv"]
        uv = {
            "start": [float(v) for v in uv_raw["start"]],
            "speed": [float(v) for v in uv_raw["speed"]],
            "wave": uv_raw["wave"],
            "waveAmplitude": [float(v) for v in uv_raw["waveAmplitude"]],
            "waveFrequency": float(uv_raw["waveFrequency"]),
            "sequence": uv_raw["sequence"],
            "loop": uv_raw["loop"],
            "sequenceTerm": float(uv_raw["sequenceTerm"]),
            "tileColumns": uv_raw["tileColumns"],
            "tileRows": uv_raw["tileRows"],
            "tileIndex": uv_raw["tileIndex"],
        }
        require_keys(uv, RENDERER_UV_KEYS, "renderer UV config")
        lerp_raw = detail["linearLerp"]
        linear_lerp = {
            key: ([float(v) for v in value] if type(value) is list
                  else float(value) if type(value) in (int, float) and type(value) is not bool
                  else value)
            for key, value in lerp_raw.items()
        }
        require_keys(linear_lerp, RENDERER_LINEAR_LERP_KEYS, "renderer linear lerp config")
        mesh = None
        if renderer_type == "MeshParticle":
            mesh = {"useModelMaterial": detail["mesh"]["useModelMaterial"]}
            require_keys(mesh, RENDERER_MESH_KEYS, "renderer mesh config")
        sprite = None
        if renderer_type == "SpriteParticle":
            sprite = {
                "billboard": detail["sprite"]["billboard"],
                "billboardRollDegrees": 0.0,
            }
            require_keys(sprite, RENDERER_SPRITE_KEYS, "renderer sprite config")
        decal = None
        if renderer_type == "DecalParticle":
            decal = {
                "size": [float(v) for v in detail["decal"]["size"]],
                "depth": float(detail["decal"]["depth"]),
            }
            require_keys(decal, RENDERER_DECAL_KEYS, "renderer decal config")
        trail = None
        if renderer_type == "CascadeRibbon":
            trail_raw = detail["trail"]
            trail = {
                "maxPoints": trail_raw["maxPoints"],
                "pointLifeTimeSeconds": float(trail_raw["pointLifeTimeSeconds"]),
                "sampleIntervalSeconds": float(trail_raw["sampleIntervalSeconds"]),
                "minimumDistance": float(trail_raw["minimumDistance"]),
                "startWidth": float(trail_raw["startWidth"]),
                "endWidth": float(trail_raw["endWidth"]),
                "faceCamera": trail_raw["faceCamera"],
            }
            require_keys(trail, RENDERER_TRAIL_KEYS, "renderer trail config")
        after_raw = detail["afterImage"]
        after_image = {
            "sampleIntervalSeconds": float(after_raw["sampleIntervalSeconds"]),
            "maxCopies": after_raw["maxCopies"],
            "alphaExponent": float(after_raw["alphaExponent"]),
        }
        require_keys(after_image, RENDERER_AFTER_IMAGE_KEYS, "renderer after-image config")
        screen_post = None
        if renderer_type == "ScreenPost":
            screen_raw = detail["screenPost"]
            screen_post = {
                "enabled": screen_raw["enabled"],
                "profileId": screen_raw["profileId"],
                "status": screen_raw["status"],
                "intensity": float(screen_raw["intensity"]),
                "secondaryIntensity": float(screen_raw["secondaryIntensity"]),
                "frequency": float(screen_raw["frequency"]),
                "tint": [float(v) for v in screen_raw["tint"]],
                "randomSeed": screen_raw["randomSeed"],
            }
            require_keys(screen_post, RENDERER_SCREEN_POST_KEYS,
                         "renderer source candidate ScreenPost diagnostic")
        projection = {
            "color": color, "uv": uv, "linearLerp": linear_lerp, "mesh": mesh,
            "sprite": sprite, "decal": decal, "trail": trail,
            "afterImage": after_image, "screenPost": screen_post,
        }
    projection_sha = canonical_sha256({
        "sourceNode": element["sourceNode"],
        "rendererType": renderer_type,
        **projection,
    })
    result = {
        **projection,
        "sourceProjectionSha256": projection_sha,
        "fidelity": "RECONSTRUCTED_SOURCE_CANDIDATE_PROJECTION",
        "sourceExact": False,
        "consumptionPolicy": "DIAGNOSTIC_ONLY_FORBIDDEN_CONSUMPTION",
        "blockers": ["SOURCE_CANDIDATE_PRESENTATION_VALUES_NOT_APPROVAL_ROUTED"],
    }
    require_keys(result, RENDERER_RUNTIME_CONFIG_KEYS, "renderer runtime config")
    return result


def find_literal_owner(
    literal: dict[str, Any], properties: list[dict[str, Any]],
) -> str:
    literal_id = literal["literalId"]
    direct = [row["propertyId"] for row in properties if literal_id in row["payloadLiteralIds"]]
    if len(direct) == 1:
        return direct[0]
    require(not direct, f"literal {literal_id} has duplicate direct owners")
    path = literal["propertyPath"]
    candidates = [
        row for row in properties
        if path == row["propertyPath"] or path.startswith(row["propertyPath"] + ".")
    ]
    require(candidates, f"literal {literal_id} has no property owner")
    max_length = max(len(row["propertyPath"]) for row in candidates)
    owners = [row for row in candidates if len(row["propertyPath"]) == max_length]
    require(len(owners) == 1, f"literal {literal_id} has ambiguous property owner")
    return owners[0]["propertyId"]


def numeric_leaf_values(value: Any) -> list[float]:
    result: list[float] = []
    if type(value) is bool:
        result.append(1.0 if value else 0.0)
    elif type(value) in (int, float):
        result.append(float(value))
    elif type(value) is list:
        for item in value:
            result.extend(numeric_leaf_values(item))
    elif type(value) is dict:
        for key in sorted(value):
            if key != "variant":
                result.extend(numeric_leaf_values(value[key]))
    return result


def normalized_capability_samples(
    module_id: str, capability: dict[str, Any],
) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []
    for order, raw in enumerate(capability["numericSamples"]):
        typed_inputs = raw["typedInputs"]
        output = raw["output"]
        row = {
            "sampleId": raw["sampleId"],
            "order": order,
            "ownerModuleId": module_id,
            "time": float(raw["time"]),
            "fixedSeed": raw["fixedSeed"],
            "fixedSeedSource": raw["fixedSeedSource"],
            "randomUnits": [float(v) for v in raw["randomUnits"]],
            "inputVariant": capability["variant"],
            "inputLiteralProjectionSha256": canonical_sha256(
                typed_inputs.get("sourceLiterals", {})
            ),
            "inputDistributionProjectionSha256": canonical_sha256(
                typed_inputs.get("evaluatedDistributions", {})
            ),
            "inputValues": numeric_leaf_values(typed_inputs),
            "outputVariant": output["variant"],
            "outputValues": numeric_leaf_values(output),
            "typedInputSha256": raw["typedInputSha256"],
            "outputSha256": raw["outputSha256"],
            "absoluteTolerance": float(raw["absoluteTolerance"]),
            "relativeTolerance": float(raw["relativeTolerance"]),
        }
        require_keys(row, CAPABILITY_SAMPLE_KEYS, "module capability sample")
        result.append(row)
    require(len(result) == 3, f"capability module {module_id} must own three samples")
    return result


def capability_literal_projection_from_raw(rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    result = []
    for raw in rows:
        variant = {"boolean": "BOOL", "number": "F64", "string": "ENUM_STRING"}[
            raw["kind"]
        ]
        result.append({
            "literalId": raw["literalId"],
            "propertyPath": raw["propertyPath"],
            "variant": variant,
            "boolValue": raw["value"] if variant == "BOOL" else None,
            "f64Value": float(raw["value"]) if variant == "F64" else None,
            "enumStringValue": raw["value"] if variant == "ENUM_STRING" else "",
        })
    return sorted(result, key=lambda row: row["literalId"])


def capability_literal_projection_from_program(rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    result = [{key: row[key] for key in (
        "literalId", "propertyPath", "variant", "boolValue", "f64Value",
        "enumStringValue",
    )} for row in rows]
    return sorted(result, key=lambda row: row["literalId"])


def capability_distribution_projection_from_raw(
    rows: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    result = [{key: row[key] for key in (
        "distributionId", "payloadDistributionId", "propertyPath",
    )} for row in rows]
    return sorted(result, key=lambda row: row["distributionId"])


def capability_distribution_projection_from_program(
    rows: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    result = [{key: row[key] for key in (
        "distributionId", "payloadDistributionId", "propertyPath",
    )} for row in rows]
    return sorted(result, key=lambda row: row["distributionId"])


def capability_property_projection_from_raw(
    rows: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    result = [{
        "propertyId": row["propertyId"],
        "propertyPath": row["propertyPath"],
        "consumptionDecision": row["sourceDecision"],
        "sourceFidelity": row["sourceFidelity"],
        "capabilityConsumptionDecision": row["capabilityConsumptionDecision"],
        "semanticRole": row["semanticRole"],
        "outputDependencyRequired": row["outputDependencyRequired"],
        "irrelevanceOracleId": row["irrelevanceOracleId"],
        "payloadLiteralIds": row["payloadLiteralIds"],
        "payloadDistributionIds": row["payloadDistributionIds"],
        "semanticDistributionIds": row["semanticDistributionIds"],
        "preservedBlockers": row["preservedBlockers"],
    } for row in rows]
    return sorted(result, key=lambda row: row["propertyId"])


def capability_property_projection_from_program(
    rows: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    keys = (
        "propertyId", "propertyPath", "consumptionDecision", "sourceFidelity",
        "capabilityConsumptionDecision", "semanticRole", "outputDependencyRequired",
        "irrelevanceOracleId", "payloadLiteralIds", "payloadDistributionIds",
        "semanticDistributionIds", "preservedBlockers",
    )
    result = [{key: row[key] for key in keys} for row in rows]
    return sorted(result, key=lambda row: row["propertyId"])


def capability_seed_projection_from_raw(raw: dict[str, Any] | None) -> dict[str, Any] | None:
    if not raw:
        return None
    policy = raw["policy"]
    return {
        "evaluatorId": raw["evaluatorId"],
        "randomSeeds": raw["randomSeeds"],
        "parameterName": policy["parameterName"],
        "getSeedFromInstance": policy["getSeedFromInstance"],
        "instanceSeedIsIndex": policy["instanceSeedIsIndex"],
        "resetSeedOnEmitterLooping": policy["resetSeedOnEmitterLooping"],
        "randomlySelectSeedArray": policy["randomlySelectSeedArray"],
        "emptyArrayUsesOccurrenceRandomStream": policy["emptyArrayUsesOccurrenceRandomStream"],
        "currentCdoEvidenceKey": raw["currentCdoEvidenceKey"],
        "source": raw["source"],
        "sourceFidelity": raw["sourceFidelity"],
    }


def capability_seed_projection_from_program(raw: dict[str, Any] | None) -> dict[str, Any] | None:
    if raw is None:
        return None
    return {key: raw[key] for key in (
        "evaluatorId", "randomSeeds", "parameterName", "getSeedFromInstance",
        "instanceSeedIsIndex", "resetSeedOnEmitterLooping", "randomlySelectSeedArray",
        "emptyArrayUsesOccurrenceRandomStream", "currentCdoEvidenceKey", "source",
        "sourceFidelity",
    )}


def normalized_distribution_samples(
    adapter: dict[str, Any], action_cue_bindings: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []
    for index, raw in enumerate(adapter.get("numericOracleSamples", [])):
        if "time" in raw and "randomUnits" in raw:
            time = float(raw.get("time", 0.0))
            random_units = [float(v) for v in raw.get("randomUnits", [])]
            output_values = [float(v) for v in raw["value"]]
            domain = "DISTRIBUTION_EVALUATOR"
            input_variant = "TIME_RANDOM_UNITS"
            source_cue_id = None
            branch = None
            parameter_input = None
            diagnostic_base = None
            blocked = None
            input_payload = {
                "domain": domain,
                "distributionId": adapter["distributionId"],
                "inputVariant": input_variant,
                "time": time,
                "randomUnits": random_units,
            }
        else:
            time = 0.0
            random_units = []
            output_values = [float(v) for v in raw["value"]]
            domain = "PARTICLE_PARAMETER_BRANCH"
            input_variant = "PARTICLE_PARAMETER_INPUT"
            source_cue_id = raw["sourceCueId"]
            branch = raw["branch"]
            parameter_input = copy.deepcopy(raw["parameterInput"])
            if parameter_input is not None:
                require_keys(parameter_input, PARAMETER_SAMPLE_INPUT_KEYS,
                             "ParticleParameter sample input")
            diagnostic_base = copy.deepcopy(raw["diagnosticStandardBaseValue"])
            blocked = raw["blocked"]
            input_payload = {
                "domain": domain,
                "distributionId": adapter["distributionId"],
                "inputVariant": input_variant,
                "sourceCueId": source_cue_id,
                "branch": branch,
                "parameterInput": parameter_input,
                "actionCueBindings": action_cue_bindings,
                "diagnosticStandardBaseValue": diagnostic_base,
                "blocked": blocked,
            }
        item = {
            "sampleId": f"{adapter['distributionId']}::sample:{index:03d}",
            "domain": domain,
            "inputVariant": input_variant,
            "time": time,
            "randomUnits": random_units,
            "sourceCueId": source_cue_id,
            "branch": branch,
            "parameterInput": parameter_input,
            "diagnosticStandardBaseValue": diagnostic_base,
            "blocked": blocked,
            "actionCueBindingsSha256": canonical_sha256(action_cue_bindings),
            "inputSha256": canonical_sha256(input_payload),
            "outputSha256": canonical_sha256(output_values),
            "outputValues": output_values,
            "absoluteTolerance": 1.0e-6,
            "relativeTolerance": 1.0e-6,
        }
        require_keys(item, DIST_SAMPLE_KEYS, f"distribution sample {index}")
        result.append(item)
    return result


def distribution_field_provenance(
    adapter: dict[str, Any], module_adapter: dict[str, Any],
) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []
    for raw in adapter.get("currentRevisionFields", []):
        item = {
            "fieldPath": raw["fieldPath"],
            "provenanceTier": raw["provenanceTier"],
            "evidenceStatus": raw["evidenceStatus"],
            "valueSha256": canonical_sha256(raw["value"]),
            "typedValueBindingSha256": "",
        }
        require_keys(item, FIELD_PROVENANCE_KEYS, "distribution field provenance")
        result.append(item)
    raw_provenance = module_adapter.get("fieldProvenance", {})
    for field_name in raw_provenance.get("reconstructedFieldNames", []):
        item = {
            "fieldPath": field_name,
            "provenanceTier": raw_provenance.get("reconstructionBasis", ""),
            "evidenceStatus": raw_provenance.get("rawFieldSourceFidelity", ""),
            "valueSha256": "",
            "typedValueBindingSha256": "",
        }
        require_keys(item, FIELD_PROVENANCE_KEYS, "reconstructed field provenance")
        result.append(item)
    return result


def parameter_field_map(adapter: dict[str, Any]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for row in adapter.get("currentRevisionFields", []):
        result[row["fieldPath"]] = row["value"]
    return result


def build_source_sections(
    source: dict[str, Any], capability_receipt: dict[str, Any],
    local_closure: dict[str, Any], candidate: dict[str, Any], approval: dict[str, Any],
    registry: HandlerRegistry,
) -> dict[str, list[dict[str, Any]]]:
    require(source.get("characterClass") == "ARTIST" and source.get("skillId") == 31470,
            "source semantics target mismatch")
    capability_by_module = {
        row["moduleOccurrenceId"]: row for row in capability_receipt["occurrences"]
    }
    require(len(capability_by_module) == 29, "source capability must own 29 module rows")
    approval_by_module = {row["moduleOccurrenceId"]: row for row in approval["sourceRows"]}
    approval_source_order_by_module = {
        row["moduleOccurrenceId"]: order for order, row in enumerate(approval["sourceRows"])
    }
    require(len(approval_by_module) == 29, "approval must own 29 unique Source rows")
    require(set(approval_by_module) == set(capability_by_module),
            "approval Source rows must exactly cover reconstructed capability modules")
    approval_family_by_cluster = {
        row["upstreamClusterId"]: row for row in approval["sourceExecutionFamilies"]
    }
    require(len(approval_family_by_cluster) == 7,
            "approval must own seven unique Source execution families")
    require(sorted(row["moduleOccurrenceCount"] for row in approval_family_by_cluster.values())
            == [1, 2, 3, 3, 4, 5, 11],
            "approval Source family occurrence denominators mismatch")
    local_by_distribution = {
        row["distributionId"]: row for row in source["localDistributionAdapters"]
    }
    emitters: list[dict[str, Any]] = []
    modules: list[dict[str, Any]] = []
    properties: list[dict[str, Any]] = []
    leaves: list[dict[str, Any]] = []
    literals: list[dict[str, Any]] = []
    distributions: list[dict[str, Any]] = []
    seeds: list[dict[str, Any]] = []
    defaults: list[dict[str, Any]] = []
    point_fields: list[dict[str, Any]] = []
    candidate_by_source_node: dict[str, dict[str, Any]] = {}
    source_candidate_canonical_sha = canonical_sha256(candidate)
    for element in candidate["elements"]:
        source_node = element["sourceNode"]
        require(source_node not in candidate_by_source_node,
                f"duplicate source candidate sourceNode: {source_node}")
        candidate_by_source_node[source_node] = element

    for emitter_order, occurrence in enumerate(source["occurrences"]):
        selected_fields = occurrence["selectedLod"]["fields"]
        require(len(selected_fields) == 2, "selected LOD must expose exactly two fields")
        emitter_id = occurrence["occurrenceCompositeId"]
        emitter_module_ids = [row["moduleOccurrenceId"] for row in occurrence["modules"]]
        selected_oracles = {row["oracleId"] for row in selected_fields}
        require(len(selected_oracles) == 1, "selected LOD oracle mismatch")
        expected_source_node = (
            occurrence["sourceSystemId"] + "|" + occurrence["sourceEmitterPath"]
        )
        element = candidate_by_source_node.get(expected_source_node)
        require(element is not None,
                f"source candidate lacks exact sourceNode {expected_source_node}")
        compiler_evidence = element["sourceRecipe"]["compilerEvidence"]
        require(compiler_evidence["evidenceId"] == occurrence["evidenceId"],
                f"candidate evidence cross-check mismatch for {expected_source_node}")
        require(compiler_evidence["sourceEmitterPath"] == occurrence["sourceEmitterPath"],
                f"candidate emitter path mismatch for {occurrence['evidenceId']}")
        require(element["sourceRecipe"]["enabled"] is True,
                f"source recipe disabled for {occurrence['evidenceId']}")
        required_modules = [
            row for row in occurrence["modules"]
            if row["exactSourceClass"] == "particlemodulerequired"
        ]
        require(len(required_modules) == 1,
                f"emitter {occurrence['evidenceId']} must own one Required module")
        required_module = required_modules[0]
        explicit_local_space = [
            row["value"] for row in required_module["typedPayload"]["literals"]
            if row["propertyPath"] == "buselocalspace"
        ]
        inherited_local_space = [
            row["value"] for row in required_module["implicitDefaults"]
            if row["family"] == "RequiredLocalSpace" and row["fieldPath"] == "buselocalspace"
        ]
        require(len(explicit_local_space) + len(inherited_local_space) == 1,
                f"emitter {occurrence['evidenceId']} localSpace must have one typed source")
        local_space = (
            explicit_local_space[0] if explicit_local_space else inherited_local_space[0]
        )
        require(type(local_space) is bool, "localSpace must be bool")
        required_literals = {
            row["propertyPath"]: row for row in required_module["typedPayload"]["literals"]
        }
        require("emitterloops" in required_literals,
                f"emitter {occurrence['evidenceId']} lacks explicit Required emitterloops")
        loops_value = required_literals["emitterloops"]["value"]
        require(type(loops_value) is float and loops_value.is_integer()
                and loops_value >= 0.0,
                f"emitter {occurrence['evidenceId']} emitterloops invalid")
        delay_literal = required_literals.get("emitterdelay")
        duration_literal = required_literals.get("emitterduration")
        emitter_delay = float(delay_literal["value"]) if delay_literal else 0.0
        emitter_duration = float(duration_literal["value"]) if duration_literal else 1.0
        delay_policy = (
            "EXPLICIT_REQUIRED_LITERAL" if delay_literal
            else "RECONSTRUCTED_UE3_ZERO_DISTRIBUTION_DEFAULT_V1"
        )
        duration_policy = (
            "EXPLICIT_REQUIRED_LITERAL" if duration_literal
            else "CURRENT_REVISION_CDO_RECONSTRUCTED_DEFAULT_V1"
        )
        spawn_modules = [
            row for row in occurrence["modules"]
            if row["exactSourceClass"] == "particlemodulespawn"
        ]
        lifetime_modules = [
            row for row in occurrence["modules"]
            if row["exactSourceClass"] in {
                "particlemodulelifetime", "particlemodulelifetime_seeded",
            }
        ]
        require(len(spawn_modules) == len(lifetime_modules) == 1,
                f"emitter {occurrence['evidenceId']} Spawn/Lifetime denominator mismatch")
        spawn_module = spawn_modules[0]
        lifetime_module = lifetime_modules[0]
        burst_literals = {
            row["propertyPath"]: row for row in spawn_module["typedPayload"]["literals"]
            if row["propertyPath"].startswith("burstlist[")
        }
        burst_indices = sorted({
            int(path.split("[", 1)[1].split("]", 1)[0])
            for path in burst_literals
        })
        require(burst_indices == list(range(len(burst_indices))),
                f"emitter {occurrence['evidenceId']} burst indices are not contiguous")
        bursts: list[dict[str, Any]] = []
        for burst_index in burst_indices:
            prefix = f"burstlist[{burst_index}]"
            triplet = [
                burst_literals.get(prefix + suffix)
                for suffix in (".count", ".countlow", ".time")
            ]
            require(all(item is not None for item in triplet),
                    f"emitter {occurrence['evidenceId']} burst triplet incomplete")
            count, count_low, burst_time = [item["value"] for item in triplet]
            require(all(type(value) is float and math.isfinite(value)
                        for value in (count, count_low, burst_time))
                    and count.is_integer() and count_low.is_integer()
                    and burst_time >= 0.0,
                    f"emitter {occurrence['evidenceId']} burst literal type/range mismatch")
            count_maximum = int(count)
            count_minimum = count_maximum if count_low < 0.0 else int(count_low)
            require(0 <= count_minimum <= count_maximum,
                    f"emitter {occurrence['evidenceId']} burst count range invalid")
            if count_maximum == 0:
                continue
            literal_projection = [
                {
                    "propertyPath": item["propertyPath"],
                    "variant": "F64",
                    "f64Value": float(item["value"]),
                }
                for item in triplet
            ]
            burst = {
                "spawnModuleId": spawn_module["moduleOccurrenceId"],
                "burstIndex": burst_index,
                "timeSeconds": float(burst_time),
                "countMinimum": count_minimum,
                "countMaximum": count_maximum,
                "sourceLiteralProjectionSha256": canonical_sha256(literal_projection),
            }
            require_keys(burst, BURST_KEYS, "emitter burst")
            bursts.append(burst)
        timing_core = {
            "requiredModuleId": required_module["moduleOccurrenceId"],
            "spawnModuleId": spawn_module["moduleOccurrenceId"],
            "lifetimeModuleId": lifetime_module["moduleOccurrenceId"],
            "emitterDelaySeconds": emitter_delay,
            "emitterDelayPolicy": delay_policy,
            "emitterDurationSeconds": emitter_duration,
            "emitterDurationPolicy": duration_policy,
            "emitterLoopCount": int(loops_value),
            "bursts": bursts,
        }
        timing_blockers = ["R3_TYPED_TIMING_EXECUTOR_NOT_COMPLETE"]
        if delay_literal is None:
            timing_blockers.append("SOURCE_ERA_REQUIRED_DELAY_DEFAULT_UNPROVEN")
        if duration_literal is None:
            timing_blockers.append("SOURCE_ERA_REQUIRED_DURATION_DEFAULT_UNPROVEN")
        timing = {
            **timing_core,
            "sourceProjectionSha256": canonical_sha256(timing_core),
            "fidelity": "RECONSTRUCTED_TYPED_SOURCE_AND_CURRENT_CDO_DEFAULTS_V1",
            "sourceExact": False,
            "blockers": sorted(timing_blockers),
        }
        timing["timingSha256"] = canonical_sha256(timing)
        require_keys(timing, EMITTER_TIMING_KEYS, "emitter timing")
        random_seed_input = {
            "policyId": "DETERMINISTIC_OCCURRENCE_RNG_FROM_SOURCE_CANDIDATE_V1",
            "sourceCandidateCanonicalSha256": source_candidate_canonical_sha,
            "emitterId": emitter_id,
        }
        random_input_sha = canonical_sha256(random_seed_input)
        random = {
            "policyId": random_seed_input["policyId"],
            "seedDerivationInputSha256": random_input_sha,
            "emitterRandomSeed": int(random_input_sha[:8], 16),
            "sourceExact": False,
            "blockers": ["R3_OCCURRENCE_RANDOM_STREAM_NOT_EXECUTED"],
        }
        random["policySha256"] = canonical_sha256(random)
        require_keys(random, EMITTER_RANDOM_KEYS, "emitter random")
        cue_source = compiler_evidence["cueLocalTransform"]
        cue_transform = {
            "sourcePositionUeUnits": [float(v) for v in cue_source["sourcePositionUeUnits"]],
            "position": [float(v) for v in cue_source["position"]],
            "rotationDegrees": [float(v) for v in cue_source["rotationDegrees"]],
            "scale": [float(v) for v in cue_source["scale"]],
        }
        require_keys(cue_transform, CUE_TRANSFORM_KEYS, "cue transform")
        raw_detail = element["detail"]["transform"]
        detail_projection = {
            "position": [float(v) for v in raw_detail["position"]],
            "rotationDegrees": [float(v) for v in raw_detail["rotationDegrees"]],
            "revolutionDegreesPerSecond": [float(v) for v in raw_detail["revolutionDegreesPerSecond"]],
            "scale": [float(v) for v in raw_detail["scale"]],
            "velocityPerSecond": [float(v) for v in raw_detail["velocityPerSecond"]],
        }
        require(detail_projection == {
            "position": [0.0, 0.0, 0.0],
            "rotationDegrees": [0.0, 0.0, 0.0],
            "revolutionDegreesPerSecond": [0.0, 0.0, 0.0],
            "scale": [1.0, 1.0, 1.0],
            "velocityPerSecond": [0.0, 0.0, 0.0],
        }, f"emitter {occurrence['evidenceId']} detail transform is not identity")
        detail_transform = {
            **detail_projection,
            "decision": "VERIFIED_IDENTITY_IRRELEVANT",
            "consumptionPolicy": "IDENTITY_ONLY_NO_RUNTIME_OPERATION",
        }
        detail_transform["projectionSha256"] = canonical_sha256(detail_transform)
        require_keys(detail_transform, DETAIL_TRANSFORM_KEYS, "detail transform")
        raw_attachment = element["actionCueAttachment"]
        raw_socket = raw_attachment["socketLocalTransform"]
        socket_transform = {
            "position": [float(v) for v in raw_socket["position"]],
            "rotationDegrees": [float(v) for v in raw_socket["rotationDegrees"]],
            "scale": [float(v) for v in raw_socket["scale"]],
        }
        require_keys(socket_transform, SOCKET_TRANSFORM_KEYS, "socket transform")
        attachment = {
            "enabled": raw_attachment["enabled"],
            "follow": raw_attachment["follow"],
            "sourceAnchorSlotId": raw_attachment["sourceAnchorSlotId"],
            "runtimeAnchorSlotId": raw_attachment["runtimeAnchorSlotId"],
            "runtimeBoneName": raw_attachment["runtimeBoneName"],
            "socketLocalTransform": socket_transform,
        }
        require_keys(attachment, ATTACHMENT_KEYS, "ActionCue attachment")
        raw_inheritance = element["transformInheritance"]
        require(raw_inheritance == {"enabled": False, "masterElementId": ""},
                f"emitter {occurrence['evidenceId']} unexpected transform inheritance")
        transform_inheritance = {
            "enabled": False,
            "masterEmitterId": "",
            "decision": "RECONSTRUCTED_NO_INHERITANCE_V1",
            "consumptionPolicy": "NO_RUNTIME_PARENT_TRANSFORM_INHERITANCE",
        }
        transform_inheritance["projectionSha256"] = canonical_sha256(transform_inheritance)
        require_keys(transform_inheritance, TRANSFORM_INHERITANCE_KEYS,
                     "transform inheritance")
        anchor_requests: list[dict[str, Any]] = []
        if raw_attachment["enabled"]:
            anchor = {
                "anchorRequestId": emitter_id + "::anchor:action-cue",
                "order": 0,
                "sourceKind": "ACTION_CUE",
                "sourceModuleId": "",
                "sourceAnchorSlotId": raw_attachment["sourceAnchorSlotId"],
                "runtimeAnchorSlotId": raw_attachment["runtimeAnchorSlotId"],
                "runtimeBoneName": raw_attachment["runtimeBoneName"],
                "socketLocalTransform": socket_transform,
                "follow": raw_attachment["follow"],
            }
            require_keys(anchor, ANCHOR_REQUEST_KEYS, "anchor request")
            anchor_requests.append(anchor)
        size_unit_policy = {
            "MeshParticle": "DIMENSIONLESS_AXIS_REORDER_ONLY",
            "ScreenPost": "SCREEN_SPACE_UNITLESS",
        }.get(occurrence["rendererType"], "UE3_LENGTH_XZY_0P01")
        source_peak = element["sourceRecipe"]["sourcePeakActiveParticles"]
        require(type(source_peak) is int and source_peak > 0,
                f"emitter {occurrence['evidenceId']} source peak invalid")
        cap_projection = {
            "emitterId": emitter_id,
            "sourcePeakActiveParticles": source_peak,
            "operationalMaxParticles": source_peak,
            "policy": "SOURCE_LOD_PEAK_ACTIVE_PARTICLES_V1",
        }
        emitter = {
            "emitterId": emitter_id,
            "order": emitter_order,
            "evidenceId": occurrence["evidenceId"],
            "sourceOccurrenceId": occurrence["sourceOccurrenceId"],
            "sourceSystemId": occurrence["sourceSystemId"],
            "sourceEmitterPath": occurrence["sourceEmitterPath"],
            "sourceEmitterNodeId": compiler_evidence["sourceEmitterNodeId"],
            "sourceCueId": compiler_evidence["sourceCueId"],
            "sourceElementId": element["id"],
            "sourceNode": element["sourceNode"],
            "sourceActionCueProjectionSha256": "",
            "visible": element["visible"],
            "rendererType": occurrence["rendererType"],
            "rendererSourceSpace": element["renderer"]["sourceSpace"],
            "selectedLodPath": occurrence["selectedLod"]["sourceLodPath"],
            "selectedLodNodeId": occurrence["selectedLod"]["sourceLodNodeId"],
            "selectedLodRecordSha256": occurrence["selectedLod"]["sourceLodRecordSha256"],
            "selectedLodDecision": "VERIFIED_IRRELEVANT",
            "selectedLodOracleId": next(iter(selected_oracles)),
            "sourceRecipeEnabled": True,
            "localSpace": local_space,
            "sizeUnitPolicy": size_unit_policy,
            "operationalMaxParticles": source_peak,
            "sourcePeakActiveParticles": source_peak,
            "spawnRateFallbackPolicy": "FORBIDDEN_TYPED_OPCODE_REQUIRED",
            "lifetimeFallbackPolicy": "FORBIDDEN_TYPED_OPCODE_REQUIRED",
            "operationalCapPolicy": cap_projection["policy"],
            "operationalCapProjectionSha256": canonical_sha256(cap_projection),
            "operationalCapBlockers": ["R3_OPERATIONAL_PARTICLE_CAP_NOT_EXECUTED"],
            "rendererRuntimeConfig": typed_renderer_runtime_config(
                element, occurrence["rendererType"]
            ),
            "screenPostAdapter": None,
            "lightAdapter": None,
            "decalAdapter": None,
            "ribbonAdapter": None,
            "timing": timing,
            "random": random,
            "cueLocalTransform": cue_transform,
            "detailTransform": detail_transform,
            "actionCueAttachment": attachment,
            "transformInheritance": transform_inheritance,
            "transformCompositionOrder": compiler_evidence["compositionOrder"],
            "anchorRequests": anchor_requests,
            "moduleIds": emitter_module_ids,
            "actionCueParameterInputs": typed_action_cue_bindings(
                occurrence["actionCueParameterInputs"]
            ),
            "scheduleId": "",
            "materialOccurrenceId": None,
            "textureResourceIds": [],
            "geometryUseId": None,
            "sourceExact": False,
        }
        emitters.append(seal_row(emitter, "emitters"))

        for module in occurrence["modules"]:
            module_id = module["moduleOccurrenceId"]
            capability = capability_by_module.get(module_id)
            approval_source = approval_by_module.get(module_id)
            if capability is None:
                require(approval_source is None,
                        f"standard source module {module_id} unexpectedly has approval row")
                selection_kind = "SOURCE_HANDLER"
                module_handler_id = registry.get(
                    "SOURCE_MODULE", module["handlerCapabilityId"],
                    exact_source_class=module["exactSourceClass"],
                    variant="EXACT_SOURCE_HANDLER",
                )
                handler_variant = "EXACT_SOURCE_HANDLER"
                module_blockers = list(module["blockers"])
            else:
                require(approval_source is not None,
                        f"reconstructed module {module_id} lacks approval row")
                selection_kind = "RECONSTRUCTED_HANDLER"
                require(capability["exactSourceClass"] == module["exactSourceClass"],
                        f"capability class mismatch for {module_id}")
                require(approval_source["exactSourceClass"] == module["exactSourceClass"],
                        f"approval class mismatch for {module_id}")
                approval_family = approval_family_by_cluster.get(
                    approval_source["upstreamClusterId"]
                )
                require(approval_family is not None
                        and approval_family["policyFamilyId"]
                        == approval_source["policyFamilyId"],
                        f"approval Source family mismatch for {module_id}")
                require(approval_source["sourceExact"] is False
                        and approval_source["executionAdmission"] is False
                        and approval_source["productAdmission"] is False,
                        f"approval Source row admission mismatch for {module_id}")
                module_handler_id = registry.get(
                    "RECONSTRUCTED_MODULE", capability["implementationId"],
                    implementation_version=capability["implementationVersion"],
                    implementation_sha256=capability["implementationSha256"],
                    exact_source_class=module["exactSourceClass"],
                    variant=capability["variant"],
                )
                handler_variant = capability["variant"]
                module_blockers = sorted(set(
                    module["blockers"] + capability["preservedEvidenceBlockers"]
                ))

            module_properties = module["properties"]
            capability_properties = {
                row["propertyId"]: row
                for row in (capability["propertyConsumption"] if capability else [])
            }
            require(
                len(capability_properties)
                == len(capability["propertyConsumption"] if capability else []),
                f"capability module {module_id} has duplicate property consumption rows",
            )
            if capability is not None:
                require(
                    set(capability_properties)
                    == {row["propertyId"] for row in module_properties},
                    f"capability module {module_id} property consumption coverage mismatch",
                )
            literal_owner = {
                raw["literalId"]: find_literal_owner(raw, module_properties)
                for raw in module["typedPayload"]["literals"]
            }
            property_by_id = {row["propertyId"]: row for row in module_properties}
            property_by_path = {row["propertyPath"]: row for row in module_properties}
            for property_order, raw in enumerate(module_properties):
                capability_property = capability_properties.get(raw["propertyId"])
                if capability_property is not None:
                    require(capability_property["propertyPath"] == raw["propertyPath"],
                            f"capability property path mismatch for {raw['propertyId']}")
                    require(capability_property["sourceDecision"] == raw["decision"],
                            f"capability property decision mismatch for {raw['propertyId']}")
                    require(capability_property["sourceFidelity"] == raw["sourceFidelity"],
                            f"capability property fidelity mismatch for {raw['propertyId']}")
                    for key in (
                        "irrelevanceOracleId", "payloadLiteralIds",
                        "payloadDistributionIds", "semanticDistributionIds",
                    ):
                        require(capability_property[key] == raw[key],
                                f"capability property {raw['propertyId']} {key} mismatch")
                handler_id = registry.get(
                    "SOURCE_PROPERTY", raw["handlerCapabilityId"],
                    exact_source_class=module["exactSourceClass"],
                    variant=raw["propertyPath"],
                )
                row = {
                    "propertyId": raw["propertyId"],
                    "moduleId": module_id,
                    "order": property_order,
                    "propertyPath": raw["propertyPath"],
                    "handlerRegistryId": handler_id,
                    "consumptionDecision": raw["decision"],
                    "irrelevanceOracleId": raw["irrelevanceOracleId"],
                    "payloadLiteralIds": raw["payloadLiteralIds"],
                    "payloadDistributionIds": raw["payloadDistributionIds"],
                    "semanticDistributionIds": raw["semanticDistributionIds"],
                    "sourceFidelity": raw["sourceFidelity"],
                    "capabilityConsumptionDecision": (
                        capability_property["capabilityConsumptionDecision"]
                        if capability_property else ""
                    ),
                    "semanticRole": capability_property["semanticRole"] if capability_property else "",
                    "outputDependencyRequired": (
                        capability_property["outputDependencyRequired"]
                        if capability_property else False
                    ),
                    "capabilitySourceRowSha256": (
                        canonical_sha256(capability_property) if capability_property else ""
                    ),
                    "preservedBlockers": (
                        capability_property["preservedBlockers"]
                        if capability_property else raw["blockers"]
                    ),
                    "sourceExact": False,
                }
                properties.append(seal_row(row, "properties"))

            for leaf_order, raw in enumerate(module["primitiveLeaves"]):
                property_id = property_by_path[raw["topLevelPropertyPath"]]["propertyId"]
                handler_id = registry.get(
                    "SOURCE_PRIMITIVE", raw["handlerCapabilityId"],
                    exact_source_class=module["exactSourceClass"],
                    variant=raw["topLevelPropertyPath"],
                )
                row = {
                    "leafId": raw["leafId"],
                    "moduleId": module_id,
                    "propertyId": property_id,
                    "order": leaf_order,
                    "propertyPath": raw["propertyPath"],
                    "topLevelPropertyPath": raw["topLevelPropertyPath"],
                    "literalId": raw["payloadLiteralId"],
                    "valueVariant": {
                        "boolean": "BOOL", "number": "F64", "string": "ENUM_STRING",
                    }[raw["kind"]],
                    "handlerRegistryId": handler_id,
                    "consumptionDecision": raw["decision"],
                    "preservedBlockers": raw["blockers"],
                    "sourceExact": False,
                }
                leaves.append(seal_row(row, "primitiveLeaves"))

            for literal_order, raw in enumerate(module["typedPayload"]["literals"]):
                variant = {"boolean": "BOOL", "number": "F64", "string": "ENUM_STRING"}[
                    raw["kind"]
                ]
                row = {
                    "literalId": raw["literalId"],
                    "moduleId": module_id,
                    "propertyId": literal_owner[raw["literalId"]],
                    "order": literal_order,
                    "propertyPath": raw["propertyPath"],
                    "variant": variant,
                    "boolValue": raw["value"] if variant == "BOOL" else None,
                    "f64Value": float(raw["value"]) if variant == "F64" else None,
                    "enumStringValue": raw["value"] if variant == "ENUM_STRING" else "",
                    "sourceExact": False,
                }
                literals.append(seal_row(row, "literals"))

            payload_by_id = {
                row["payloadDistributionId"]: row["descriptor"]
                for row in module["typedPayload"]["distributions"]
            }
            distribution_ids: list[str] = []
            for distribution_order, module_adapter in enumerate(module["distributionAdapters"]):
                distribution_id = module_adapter["distributionId"]
                distribution_ids.append(distribution_id)
                payload_id = module_adapter["payloadDistributionId"]
                payload = payload_by_id[payload_id]
                local = local_by_distribution.get(distribution_id)
                cap_distribution = None
                if capability is not None:
                    matches = [
                        row for row in capability["distributionBindings"]
                        if row["distributionId"] == distribution_id
                    ]
                    require(len(matches) <= 1, f"duplicate capability distribution {distribution_id}")
                    cap_distribution = matches[0] if matches else None
                source_class = payload.get("sourceClass", "")
                descriptor = payload
                variant = "INLINE"
                if local is not None:
                    source_class = local["exactSourceClass"]
                    descriptor = local.get("reconstructedDescriptor") or payload
                    variant = {
                        "distributionfloatparticleparameter": "FLOAT_PARAMETER",
                        "distributionvectorparticleparameter": "VECTOR_PARAMETER",
                        "distributionfloatconstantcurve": "FLOAT_CURVE",
                        "efdistributionvectormultiplyparticleparameter": "EF_MULTIPLY",
                    }[source_class]
                property_matches = [
                    row for row in module_properties
                    if distribution_id in row["semanticDistributionIds"]
                ]
                require(len(property_matches) == 1,
                        f"distribution {distribution_id} must have one semantic property owner")
                property_row = property_matches[0]
                if variant == "EF_MULTIPLY":
                    require(capability is not None and cap_distribution is not None,
                            f"EF multiply {distribution_id} lacks reconstructed capability")
                    evaluator_id = registry.get(
                        "DISTRIBUTION", capability["implementationId"],
                        implementation_version=capability["implementationVersion"],
                        implementation_sha256=capability["implementationSha256"],
                        exact_source_class=source_class,
                        variant=capability["variant"],
                    )
                    cap_impl_id = capability["implementationId"]
                    cap_impl_version = capability["implementationVersion"]
                    cap_impl_sha = capability["implementationSha256"]
                else:
                    evaluator_implementation = (
                        local["evaluatorCapabilityId"] if local is not None
                        else module_adapter["evaluatorCapabilityId"]
                    )
                    evaluator_id = registry.get(
                        "DISTRIBUTION", evaluator_implementation,
                        exact_source_class=source_class,
                        variant=variant,
                    )
                    cap_impl_id = ""
                    cap_impl_version = 0
                    cap_impl_sha = ""
                field_map = parameter_field_map(local or {})
                component_count = int(descriptor.get("componentCount", 1))
                parameter_name = ""
                param_modes: list[str] = []
                minimum_input: list[float] = []
                maximum_input: list[float] = []
                minimum_output: list[float] = []
                maximum_output: list[float] = []
                constant_values: list[float] = []
                is_dirty = None
                if variant in ("FLOAT_PARAMETER", "VECTOR_PARAMETER", "EF_MULTIPLY"):
                    parameter_name = str(field_map.get("parametername", ""))
                    mode_root = "parammode" if component_count == 1 else "parammodes"
                    param_modes = [
                        str(field_map.get(mode_root if index == 0 else f"{mode_root}[{index}]", ""))
                        for index in range(component_count)
                    ]
                    minimum_input = vector_values(field_map["mininput"], component_count)
                    maximum_input = vector_values(field_map["maxinput"], component_count)
                    minimum_output = vector_values(field_map["minoutput"], component_count)
                    maximum_output = vector_values(field_map["maxoutput"], component_count)
                    constant_values = vector_values(field_map["constant"], component_count)
                    is_dirty = bool(field_map["bisdirty"])
                curve_keys = []
                for curve in descriptor.get("keys", []):
                    curve_row = {
                        "time": float(curve["time"]),
                        "minimum": [float(v) for v in curve["minimum"]],
                        "maximum": [float(v) for v in curve["maximum"]],
                        "arriveTangentMinimum": [float(v) for v in curve["arriveTangentMinimum"]],
                        "leaveTangentMinimum": [float(v) for v in curve["leaveTangentMinimum"]],
                        "arriveTangentMaximum": [float(v) for v in curve["arriveTangentMaximum"]],
                        "leaveTangentMaximum": [float(v) for v in curve["leaveTangentMaximum"]],
                        "interpolation": curve["interpolation"],
                    }
                    require_keys(curve_row, CURVE_KEY_KEYS, "distribution curve key")
                    curve_keys.append(curve_row)
                blockers = sorted(set(
                    module_adapter.get("blockers", [])
                    + (local.get("blockers", []) if local else [])
                    + (cap_distribution.get("blockers", []) if cap_distribution else [])
                ))
                action_cues = typed_action_cue_bindings(occurrence["actionCueParameterInputs"])
                if parameter_name:
                    action_cues = [
                        row for row in action_cues
                        if row["name"].casefold() == parameter_name.casefold()
                    ]
                else:
                    action_cues = []
                row = {
                    "distributionId": distribution_id,
                    "moduleId": module_id,
                    "propertyId": property_row["propertyId"],
                    "order": distribution_order,
                    "variant": variant,
                    "evaluatorRegistryId": evaluator_id,
                    "payloadDistributionId": payload_id,
                    "propertyPath": descriptor.get("propertyPath", property_row["propertyPath"]),
                    "sourceClass": source_class,
                    "sourceObjectPath": descriptor.get("sourceObjectPath", ""),
                    "referenceId": (local or {}).get("referenceId", descriptor.get("referenceId", "")),
                    "occurrenceId": (local or {}).get("legacyOccurrenceId", descriptor.get("occurrenceId", "")),
                    "payloadStatus": descriptor.get("payloadStatus", "INLINE_SOURCE_PAYLOAD"),
                    "fidelity": (local or {}).get("sourceFidelity", descriptor.get("fidelity", "")),
                    "componentCount": component_count,
                    "operation": descriptor.get("operation") if variant in ("INLINE", "FLOAT_CURVE") else None,
                    "randomLockAxes": descriptor.get("randomLockAxes") if variant in ("INLINE", "FLOAT_CURVE") else None,
                    "lookupTableChunkSize": descriptor.get("lookupTableChunkSize") if variant in ("INLINE", "FLOAT_CURVE") else None,
                    "lookupTableNumElements": descriptor.get("lookupTableNumElements") if variant in ("INLINE", "FLOAT_CURVE") else None,
                    "lookupTableTimeScale": descriptor.get("lookupTableTimeScale") if variant in ("INLINE", "FLOAT_CURVE") else None,
                    "lookupTableStartTime": descriptor.get("lookupTableStartTime") if variant in ("INLINE", "FLOAT_CURVE") else None,
                    "defaultMinimum": [float(v) for v in descriptor.get("defaultMinimum", [])],
                    "defaultMaximum": [float(v) for v in descriptor.get("defaultMaximum", [])],
                    "lookupTable": [float(v) for v in descriptor.get("lookupTable", [])],
                    "curveKeys": curve_keys,
                    "parameterName": parameter_name,
                    "paramModes": param_modes,
                    "minimumInput": minimum_input,
                    "maximumInput": maximum_input,
                    "minimumOutput": minimum_output,
                    "maximumOutput": maximum_output,
                    "constantValues": constant_values,
                    "isDirty": is_dirty,
                    "actionCueBindings": action_cues,
                    "fieldProvenance": distribution_field_provenance(local or {}, module_adapter),
                    "samples": normalized_distribution_samples(
                        local or module_adapter, action_cues
                    ),
                    "capabilityImplementationId": cap_impl_id,
                    "capabilityImplementationVersion": cap_impl_version,
                    "capabilityImplementationSha256": cap_impl_sha,
                    "preservedBlockers": blockers,
                    "sourceExact": False,
                }
                reconstructed_field_bindings = {
                    "operation": "operation",
                    "lookupTableChunkSize": "lookupTableChunkSize",
                    "lookupTableNumElements": "lookupTableNumElements",
                }
                for provenance in row["fieldProvenance"]:
                    if provenance["valueSha256"] != "":
                        continue
                    value_key = reconstructed_field_bindings.get(provenance["fieldPath"])
                    require(value_key is not None,
                            "unknown reconstructed distribution field binding")
                    provenance["typedValueBindingSha256"] = canonical_sha256({
                        "fieldPath": provenance["fieldPath"],
                        "typedValue": row[value_key],
                    })
                distributions.append(seal_row(row, "distributions"))

            seed_policy_id = ""
            if module["seed"] is not None:
                raw_seed = module["seed"]
                seed_policy_id = module_id + "::seed-policy"
                policy = raw_seed["policy"]
                row = {
                    "seedPolicyId": seed_policy_id,
                    "moduleId": module_id,
                    "order": len(seeds),
                    "evaluatorId": raw_seed["evaluatorId"],
                    "randomSeeds": raw_seed["randomSeeds"],
                    "parameterName": policy["parameterName"],
                    "getSeedFromInstance": policy["getSeedFromInstance"],
                    "instanceSeedIsIndex": policy["instanceSeedIsIndex"],
                    "resetSeedOnEmitterLooping": policy["resetSeedOnEmitterLooping"],
                    "randomlySelectSeedArray": policy["randomlySelectSeedArray"],
                    "emptyArrayUsesOccurrenceRandomStream": policy["emptyArrayUsesOccurrenceRandomStream"],
                    "currentCdoEvidenceKey": raw_seed["currentCdoEvidenceKey"],
                    "source": raw_seed["source"],
                    "sourceFidelity": raw_seed["sourceFidelity"],
                    "sourceExact": False,
                }
                seeds.append(seal_row(row, "seedPolicies"))

            implicit_ids: list[str] = []
            for raw_default in module["implicitDefaults"]:
                implicit_ids.append(raw_default["defaultId"])
                family = raw_default["family"]
                if family == "RequiredLocalSpace":
                    variant = "BOOL"
                    bool_value = raw_default["value"]
                    ribbon_values = None
                    decal_values = None
                    reason = ""
                    owned = []
                elif family == "Ribbon":
                    values = source["currentRevisionDefaultEvidence"]["ribbon"]["values"]
                    variant = "RIBBON_DEFAULTS"
                    bool_value = None
                    ribbon_values = {
                        "maxTessellationBetweenParticles": values["maxtessellationbetweenparticles"],
                        "sheetsPerTrail": values["sheetspertrail"],
                        "maxTrailCount": values["maxtrailcount"],
                        "maxParticleInTrailCount": values["maxparticleintrailcount"],
                        "deadTrailsOnDeactivate": values["bdeadtrailsondeactivate"],
                        "deadTrailsOnSourceLoss": values["bdeadtrailsonsourceloss"],
                        "clipSourceSegment": values["bclipsourcesegement"],
                        "enablePreviousTangentRecalculation": values["benableprevioustangentrecalculation"],
                        "renderGeometry": values["brendergeometry"],
                        "distanceTessellationStepSize": values["distancetessellationstepsize"],
                        "tangentTessellationScalar": values["tangenttessellationscalar"],
                    }
                    require_keys(ribbon_values, RIBBON_VALUE_KEYS, "ribbon defaults")
                    decal_values = None
                    reason = ""
                    owned = []
                elif family == "Decal":
                    values = source["currentRevisionDefaultEvidence"]["decal"]["values"]
                    variant = "DECAL_DEFAULTS"
                    bool_value = None
                    ribbon_values = None
                    decal_values = {
                        "defaultSize": [values["defaultsize"]["x"], values["defaultsize"]["y"]],
                        "farPlane": values["farplane"],
                        "blendRange": [values["blendrange"]["x"], values["blendrange"]["y"]],
                        "onlyCalculateRotationYaw": values["bonlycalcrotationyaw"],
                        "supports3dDrawMode": values["bsupported3ddrawmode"],
                    }
                    require_keys(decal_values, DECAL_VALUE_KEYS, "decal defaults")
                    reason = ""
                    owned = []
                elif family == "ScreenPost":
                    variant = "SCREEN_POST_IRRELEVANT"
                    bool_value = None
                    ribbon_values = None
                    decal_values = None
                    reason = "NO_IMPLICIT_SOURCE_FIELD"
                    owned = []
                elif family == "Light":
                    variant = "POINT_LIGHT_OWNER_REFERENCE"
                    bool_value = None
                    ribbon_values = None
                    decal_values = None
                    reason = ""
                    owned = [
                        "point-light::brightness", "point-light::bcastcompositeshadow",
                        "point-light::baffectcompositeshadowdirection", "point-light::radius",
                        "point-light::falloffexponent", "point-light::lightcolor",
                        "point-light::lightguid", "point-light::lightmapguid",
                    ]
                else:
                    raise ContractError(f"unknown implicit default family: {family}")
                row = {
                    "defaultId": raw_default["defaultId"],
                    "moduleId": module_id,
                    "order": len(defaults),
                    "family": family,
                    "fieldPath": raw_default["fieldPath"],
                    "variant": variant,
                    "boolValue": bool_value,
                    "ribbonValues": ribbon_values,
                    "decalValues": decal_values,
                    "decision": raw_default["decision"],
                    "provenance": raw_default["provenance"],
                    "reason": reason,
                    "valuesOwnedBy": owned,
                    "sourceExact": False,
                }
                defaults.append(seal_row(row, "implicitDefaults"))

            if capability is not None:
                owned_literal_rows = [row for row in literals if row["moduleId"] == module_id]
                owned_distribution_rows = [
                    row for row in distributions if row["moduleId"] == module_id
                ]
                owned_property_rows = [row for row in properties if row["moduleId"] == module_id]
                literal_projection = capability_literal_projection_from_program(
                    owned_literal_rows
                )
                require(
                    literal_projection
                    == capability_literal_projection_from_raw(capability["sourceLiteralBindings"]),
                    f"capability literal binding mismatch for {module_id}",
                )
                distribution_projection = capability_distribution_projection_from_program(
                    owned_distribution_rows
                )
                require(
                    distribution_projection
                    == capability_distribution_projection_from_raw(
                        capability["distributionBindings"]
                    ),
                    f"capability distribution binding mismatch for {module_id}",
                )
                property_projection = capability_property_projection_from_program(
                    owned_property_rows
                )
                require(
                    property_projection
                    == capability_property_projection_from_raw(
                        capability["propertyConsumption"]
                    ),
                    f"capability property consumption mismatch for {module_id}",
                )
                seed_program_row = next(
                    (row for row in seeds if row["moduleId"] == module_id), None
                )
                seed_projection = capability_seed_projection_from_program(seed_program_row)
                require(
                    seed_projection
                    == capability_seed_projection_from_raw(capability["seedBinding"]),
                    f"capability seed binding mismatch for {module_id}",
                )
                module_action_cues = typed_action_cue_bindings(
                    capability["actionCueParameterInputs"]
                )
                emitter_action_cues = typed_action_cue_bindings(
                    occurrence["actionCueParameterInputs"]
                )
                require(all(item in emitter_action_cues for item in module_action_cues),
                        f"capability ActionCue binding mismatch for {module_id}")
                capability_literal_sha = canonical_sha256(literal_projection)
                capability_distribution_sha = canonical_sha256(distribution_projection)
                capability_property_sha = canonical_sha256(property_projection)
                capability_seed_sha = canonical_sha256(seed_projection)
                capability_action_names = [item["name"] for item in module_action_cues]
                capability_action_sha = canonical_sha256(module_action_cues)
            else:
                capability_literal_sha = ""
                capability_distribution_sha = ""
                capability_property_sha = ""
                capability_seed_sha = ""
                capability_action_names = []
                capability_action_sha = ""

            module_row = {
                "moduleId": module_id,
                "emitterId": emitter_id,
                "order": module["order"],
                "sourceObjectId": module["sourceObjectId"],
                "sourceRecordSha256": module["sourceRecordSha256"],
                "exactSourceClass": module["exactSourceClass"],
                "selectionKind": selection_kind,
                "handlerRegistryId": module_handler_id,
                "handlerVariant": handler_variant,
                "capabilityPolicyFamilyId": capability["policyFamilyId"] if capability else "",
                "capabilityImplementationId": capability["implementationId"] if capability else "",
                "capabilityImplementationVersion": capability["implementationVersion"] if capability else 0,
                "capabilityImplementationSha256": capability["implementationSha256"] if capability else "",
                "capabilityFamilySemanticSha256": capability["familySemanticImplementationSha256"] if capability else "",
                "capabilityInputSchemaSha256": capability["variantInputSchemaSha256"] if capability else "",
                "capabilityOutputSchemaSha256": capability["variantOutputSchemaSha256"] if capability else "",
                "capabilityDefaultPolicySha256": capability["explicitDefaultsSha256"] if capability else "",
                "capabilityNumericSamples": normalized_capability_samples(module_id, capability) if capability else [],
                "capabilitySourceRowSha256": canonical_sha256(capability) if capability else "",
                "capabilityLiteralBindingsSha256": capability_literal_sha,
                "capabilityDistributionBindingsSha256": capability_distribution_sha,
                "capabilityPropertyConsumptionSha256": capability_property_sha,
                "capabilitySeedBindingSha256": capability_seed_sha,
                "capabilityActionCueInputNames": capability_action_names,
                "capabilityActionCueInputsSha256": capability_action_sha,
                "approvalPolicyRowId": approval_source["policyRowId"] if approval_source else "",
                "approvalUpstreamIdentitySha256": (
                    approval_source["upstreamIdentitySha256"] if approval_source else ""
                ),
                "approvalPolicyBindingSha256": (
                    approval_source["policyBindingSha256"] if approval_source else ""
                ),
                "approvalRequiredMutatedOutput": (
                    approval_source["requiredMutatedOutput"] if approval_source else ""
                ),
                "approvalSourceOrder": (
                    approval_source_order_by_module[module_id] if approval_source else -1
                ),
                "approvalRequiredOracleIds": (
                    approval_source["requiredOracleIds"] if approval_source else []
                ),
                "approvalExecutionBlockers": (
                    approval_source["executionBlockers"] if approval_source else []
                ),
                "approvalSourceRow": copy.deepcopy(approval_source) if approval_source else None,
                "approvalSourceRowSha256": (
                    canonical_sha256(approval_source) if approval_source else ""
                ),
                "propertyIds": [row["propertyId"] for row in module_properties],
                "primitiveLeafIds": [row["leafId"] for row in module["primitiveLeaves"]],
                "literalIds": [row["literalId"] for row in module["typedPayload"]["literals"]],
                "distributionIds": distribution_ids,
                "seedPolicyId": seed_policy_id,
                "implicitDefaultIds": implicit_ids,
                "sourceDecision": module["decision"],
                "preservedBlockers": module_blockers,
                "sourceExact": False,
            }
            modules.append(seal_row(module_row, "modules"))

    point_adapter = source["pointLightAdapter"]
    point_capabilities = [
        row for row in capability_receipt["occurrences"]
        if row["variant"] == "EF_POINT_LIGHT_DESCRIPTOR"
    ]
    require(len(point_capabilities) == 1, "PointLight capability occurrence missing")
    point_module_id = point_capabilities[0]["moduleOccurrenceId"]
    raw_fields = list(point_adapter["fields"])
    exact_payload = local_closure["componentDefinitions"][0]["exactPayload"]
    explicit = exact_payload["explicitProperties"]
    raw_fields.extend((
        {
            "fieldId": "point-light::lightguid", "fieldPath": "lightguid",
            "value": explicit["lightguid"]["value"]["hex"],
            "sourceTier": "INSTANCE_EXPLICIT", "sourceFidelity": "SOURCE_EXACT_PHYSICAL_PACKAGE",
            "decision": "VERIFIED_IRRELEVANT", "oracleId": "ue3.runtime-light-guid-irrelevance.v1",
        },
        {
            "fieldId": "point-light::lightmapguid", "fieldPath": "lightmapguid",
            "value": explicit["lightmapguid"]["value"]["hex"],
            "sourceTier": "INSTANCE_EXPLICIT", "sourceFidelity": "SOURCE_EXACT_PHYSICAL_PACKAGE",
            "decision": "VERIFIED_IRRELEVANT", "oracleId": "ue3.runtime-light-guid-irrelevance.v1",
        },
    ))
    for order, raw in enumerate(raw_fields):
        value = raw["value"]
        if type(value) is bool:
            variant, bool_value, f64_value, color_value, guid_value = "BOOL", value, None, None, ""
        elif type(value) in (int, float) and type(value) is not bool:
            variant, bool_value, f64_value, color_value, guid_value = "F64", None, float(value), None, ""
        elif type(value) is dict:
            variant, bool_value, f64_value, color_value, guid_value = (
                "COLOR_RGBA8", None, None,
                [value["r"], value["g"], value["b"], value["a"]], "",
            )
        else:
            variant, bool_value, f64_value, color_value, guid_value = (
                "GUID128", None, None, None, value,
            )
        row = {
            "fieldId": raw["fieldId"],
            "moduleId": point_module_id,
            "order": order,
            "fieldPath": raw["fieldPath"],
            "valueVariant": variant,
            "boolValue": bool_value,
            "f64Value": f64_value,
            "colorRgba8Value": color_value,
            "guid128Value": guid_value,
            "sourceTier": raw["sourceTier"],
            "sourceFidelity": raw["sourceFidelity"],
            "decision": raw["decision"],
            "oracleId": raw["oracleId"],
            "sourceExact": False,
        }
        point_fields.append(seal_row(row, "pointLightFields"))

    return {
        "emitters": emitters,
        "modules": modules,
        "properties": properties,
        "primitiveLeaves": leaves,
        "literals": literals,
        "distributions": distributions,
        "seedPolicies": seeds,
        "implicitDefaults": defaults,
        "pointLightFields": point_fields,
    }


def normalize_sampler_descriptor(raw: dict[str, Any]) -> dict[str, Any]:
    require(type(raw) is dict, "sampler descriptor must be an object")
    result = {
        "type": raw["type"],
        "filterUe3": raw["filter"]["ue3"],
        "filterD3d11": raw["filter"]["d3d11"],
        "addressUUe3": raw["addressU"]["ue3"],
        "addressUD3d11": raw["addressU"]["d3d11"],
        "addressVUe3": raw["addressV"]["ue3"],
        "addressVD3d11": raw["addressV"]["d3d11"],
        "addressWUe3": raw["addressW"]["ue3"],
        "addressWD3d11": raw["addressW"]["d3d11"],
        "mipLODBias": float(raw["mipLODBias"]),
        "maxAnisotropy": raw["maxAnisotropy"],
        "comparisonFuncName": raw["comparisonFunc"]["name"],
        "comparisonFuncD3d11": raw["comparisonFunc"]["d3d11"],
        "borderColor": [float(v) for v in raw["borderColor"]],
        "minLOD": float(raw["minLOD"]),
        "maxLOD": float(raw["maxLOD"]),
        "sRgb": raw["sRgb"],
        "srvColorSpace": raw["srvColorSpace"],
        "lodGroup": raw["lodGroup"],
    }
    require_keys(result, SAMPLER_DESCRIPTOR_KEYS, "sampler descriptor")
    require(len(result["borderColor"]) == 4, "sampler borderColor must be float4")
    return result


def require_json_scalar_type(value: Any, expected: type, label: str) -> None:
    if expected is float:
        require(type(value) is float, f"{label}: expected JSON float")
        require(math.isfinite(value), f"{label}: non-finite float")
    else:
        require(type(value) is expected, f"{label}: expected {expected.__name__}")


def validate_d3d_descriptor_shape(kind: str, value: Any, label: str) -> None:
    require(type(value) is dict, f"{label}: descriptor must be object")
    if kind == "D3D11_DEPTH_STENCIL_DESC":
        require_keys(value, D3D_DEPTH_STENCIL_KEYS, label)
        for key in ("DepthEnable", "StencilEnable"):
            require_json_scalar_type(value[key], bool, f"{label}.{key}")
        for key in ("DepthWriteMask", "DepthFunc", "StencilReadMask", "StencilWriteMask"):
            require_json_scalar_type(value[key], int, f"{label}.{key}")
        for face_name in ("FrontFace", "BackFace"):
            face = value[face_name]
            require(type(face) is dict, f"{label}.{face_name}: expected object")
            require_keys(face, D3D_STENCIL_FACE_KEYS, f"{label}.{face_name}")
            for key in D3D_STENCIL_FACE_KEYS:
                require_json_scalar_type(face[key], int, f"{label}.{face_name}.{key}")
    elif kind == "D3D11_RASTERIZER_DESC":
        require_keys(value, D3D_RASTERIZER_KEYS, label)
        for key in ("FillMode", "CullMode", "DepthBias"):
            require_json_scalar_type(value[key], int, f"{label}.{key}")
        for key in ("DepthBiasClamp", "SlopeScaledDepthBias"):
            require_json_scalar_type(value[key], float, f"{label}.{key}")
        for key in (
            "FrontCounterClockwise", "DepthClipEnable", "ScissorEnable",
            "MultisampleEnable", "AntialiasedLineEnable",
        ):
            require_json_scalar_type(value[key], bool, f"{label}.{key}")
    elif kind == "D3D11_SAMPLER_DESC":
        require_keys(value, D3D_SAMPLER_KEYS, label)
        for key in (
            "Filter", "AddressU", "AddressV", "AddressW", "MaxAnisotropy",
            "ComparisonFunc",
        ):
            require_json_scalar_type(value[key], int, f"{label}.{key}")
        for key in ("MipLODBias", "MinLOD", "MaxLOD"):
            require_json_scalar_type(value[key], float, f"{label}.{key}")
        border = value["BorderColor"]
        require(type(border) is list and len(border) == 4,
                f"{label}.BorderColor: expected float4")
        for index, component in enumerate(border):
            require_json_scalar_type(component, float, f"{label}.BorderColor[{index}]")
    else:
        raise ContractError(f"{label}: unknown descriptor kind {kind}")


def validate_d3d_srv_shape(value: Any, label: str) -> None:
    require(type(value) is dict, f"{label}: SRV descriptor must be object")
    require_keys(value, D3D_SRV_KEYS, label)
    for key in ("Format", "ViewDimension", "MostDetailedMip", "MipLevels"):
        require_json_scalar_type(value[key], int, f"{label}.{key}")
    require_json_scalar_type(value["srvColorSpace"], str, f"{label}.srvColorSpace")
    require(value["srvColorSpace"] in {"LINEAR", "SRGB"},
            f"{label}.srvColorSpace: unsupported token")


def normalize_d3d_descriptor_oracle(raw: dict[str, Any]) -> dict[str, Any]:
    require_keys(raw, D3D_DESCRIPTOR_ORACLE_KEYS, "D3D descriptor oracle")
    require(raw["descriptorKind"] in D3D_DESCRIPTOR_KINDS,
            "D3D descriptor oracle kind unsupported")
    validate_d3d_descriptor_shape(
        raw["descriptorKind"], raw["expectedDescriptor"], "D3D expected descriptor"
    )
    validate_d3d_descriptor_shape(
        raw["descriptorKind"], raw["actualDescriptor"], "D3D actual descriptor"
    )
    require(raw["expectedDescriptor"] == raw["actualDescriptor"],
            "D3D expected/actual descriptor mismatch")
    require(type(raw["numericTolerance"]) is float and raw["numericTolerance"] == 0.0,
            "D3D descriptor tolerance must be exact float zero")
    require(raw["decision"] == "PASS", "D3D descriptor oracle must pass")
    return copy.deepcopy(raw)


def normalize_d3d_srv_oracle(raw: dict[str, Any]) -> dict[str, Any]:
    require_keys(raw, D3D_SRV_ORACLE_KEYS, "D3D SRV oracle")
    validate_d3d_srv_shape(raw["expectedSrv"], "D3D expected SRV")
    validate_d3d_srv_shape(raw["actualSrv"], "D3D actual SRV")
    require(raw["expectedSrv"] == raw["actualSrv"],
            "D3D expected/actual SRV mismatch")
    require(type(raw["numericTolerance"]) is float and raw["numericTolerance"] == 0.0,
            "D3D SRV tolerance must be exact float zero")
    require(raw["decision"] == "PASS", "D3D SRV oracle must pass")
    return copy.deepcopy(raw)


def normalize_material_family_samples(rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []
    for order, raw in enumerate(rows):
        row = {
            "sampleId": raw["sampleId"],
            "order": order,
            "inputSha256": raw["inputSha256"],
            "expectedFloat4": [float(value) for value in raw["expectedFloat4"]],
        }
        require_keys(row, MATERIAL_FAMILY_SAMPLE_KEYS, "Material family numeric sample")
        require_numeric_vector(row["expectedFloat4"], 4,
                               "Material family sample expectedFloat4")
        result.append(row)
    require(len(result) == 4, "Material family must own four numeric samples")
    return result


def normalize_material_recipe_samples(rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []
    for order, raw in enumerate(rows):
        typed_input = raw["input"]
        require_keys(typed_input, (
            "sampleId", "time", "uvScale", "panRotationAux", "texture0", "texture1",
            "color", "params0", "params1",
        ), "Material recipe numeric sample input")
        require(typed_input["sampleId"] == raw["sampleId"],
                "Material recipe sample input ID mismatch")
        row = {
            "sampleId": raw["sampleId"],
            "order": order,
            "time": float(typed_input["time"]),
            "uvScale": [float(value) for value in typed_input["uvScale"]],
            "panRotationAux": [float(value) for value in typed_input["panRotationAux"]],
            "texture0": [float(value) for value in typed_input["texture0"]],
            "texture1": [float(value) for value in typed_input["texture1"]],
            "color": [float(value) for value in typed_input["color"]],
            "params0": [float(value) for value in typed_input["params0"]],
            "params1": [float(value) for value in typed_input["params1"]],
            "inputSha256": raw["inputSha256"],
            "expectedFloat4": [float(value) for value in raw["expectedFloat4"]],
        }
        require_keys(row, MATERIAL_RECIPE_SAMPLE_KEYS, "Material recipe numeric sample")
        require_numeric_vector(row["uvScale"], 2, "Material recipe sample uvScale")
        for key in (
            "panRotationAux", "texture0", "texture1", "color", "params0", "params1",
            "expectedFloat4",
        ):
            require_numeric_vector(row[key], 4, f"Material recipe sample {key}")
        result.append(row)
    require(len(result) == 4, "Material recipe must own four numeric samples")
    return result


def build_material_sections(
    runtime: dict[str, Any], policy: dict[str, Any],
    approval: dict[str, Any], source_emitters: list[dict[str, Any]],
    registry: HandlerRegistry,
    texture_binding_receipt: dict[str, Any] | None = None,
) -> dict[str, list[dict[str, Any]]]:
    require(runtime.get("characterClass") == "ARTIST" and runtime.get("skillId") == 31470,
            "material runtime target mismatch")
    require(policy.get("characterClass") == "ARTIST" and policy.get("skillId") == 31470,
            "material policy target mismatch")
    all_policy_source_rows = (
        policy["renderStatePolicies"]
        + policy["staticPermutationPolicies"]
        + policy["samplerPolicies"]
    )
    require([row["policyOrder"] for row in all_policy_source_rows] == list(range(255)),
            "material policy order must be the exact 0..254 sequence")
    policy_by_id = {row["policyRowId"]: row for row in all_policy_source_rows}
    require(len(policy_by_id) == 255, "material policy IDs must be unique")
    static_policy_by_field = {
        row["fieldId"]: row for row in policy["staticPermutationPolicies"]
    }
    render_policy_by_owner = {
        (row["materialRecipeId"], row["fieldName"]): row
        for row in policy["renderStatePolicies"]
    }
    require(len(static_policy_by_field) == 94, "static policy field IDs must be unique")
    require(len(render_policy_by_owner) == 89, "render policy owner keys must be unique")
    approval_material_by_matrix = {
        row["upstreamMatrixRowId"]: row for row in approval["materialRows"]
    }
    approval_material_order_by_matrix = {
        row["upstreamMatrixRowId"]: order
        for order, row in enumerate(approval["materialRows"])
    }
    require(len(approval_material_by_matrix) == 255,
            "approval must own 255 unique Material policy rows")
    require(
        set(approval_material_by_matrix)
        == {row["sourceMatrixRowId"] for row in all_policy_source_rows},
        "approval Material rows must exactly cover reconstructed policy rows",
    )
    approval_material_family_ids = {
        row["policyFamilyId"] for row in approval["materialExecutionFamilies"]
    }
    require(len(approval_material_family_ids) == 8,
            "approval must own eight unique Material execution families")
    warp = policy["warpDescriptorVerification"]
    require(warp["verified"] is True, "Material WARP descriptor verification not passed")
    require(type(warp["descriptorRowCount"]) is int and warp["descriptorRowCount"] == 107,
            "Material D3D descriptor denominator mismatch")
    require(type(warp["srvColorSpaceRowCount"]) is int
            and warp["srvColorSpaceRowCount"] == 72,
            "Material D3D SRV denominator mismatch")
    require(canonical_sha256(warp["rowResults"]) == warp["rowResultsSha256"]
            == EXPECTED_D3D_DESCRIPTOR_PROJECTION_SHA256,
            "Material D3D descriptor projection identity mismatch")
    require(canonical_sha256(warp["srvRowResults"]) == warp["srvRowResultsSha256"]
            == EXPECTED_D3D_SRV_PROJECTION_SHA256,
            "Material D3D SRV projection identity mismatch")
    d3d_descriptor_by_policy: dict[str, dict[str, Any]] = {}
    for raw_oracle in warp["rowResults"]:
        oracle = normalize_d3d_descriptor_oracle(raw_oracle)
        policy_id = oracle["policyRowId"]
        require(policy_id in policy_by_id, "D3D descriptor references unknown policy")
        require(policy_id not in d3d_descriptor_by_policy,
                "D3D descriptor policy owner must be unique")
        d3d_descriptor_by_policy[policy_id] = oracle
    d3d_srv_by_policy: dict[str, dict[str, Any]] = {}
    for raw_oracle in warp["srvRowResults"]:
        oracle = normalize_d3d_srv_oracle(raw_oracle)
        policy_id = oracle["policyRowId"]
        require(policy_id in policy_by_id, "D3D SRV references unknown policy")
        require(policy_id not in d3d_srv_by_policy,
                "D3D SRV policy owner must be unique")
        d3d_srv_by_policy[policy_id] = oracle
    require(
        {policy_id for policy_id, row in policy_by_id.items()
         if row["policyKind"] == "SAMPLER_DESCRIPTOR"}
        == set(d3d_srv_by_policy),
        "every sampler policy must own exactly one D3D SRV oracle",
    )

    family_rows: list[dict[str, Any]] = []
    family_registry: dict[str, str] = {}
    approval_arithmetic_by_family = {
        row["upstreamFamilyId"]: row for row in approval["materialArithmeticRows"]
    }
    approval_arithmetic_order_by_family = {
        row["upstreamFamilyId"]: order
        for order, row in enumerate(approval["materialArithmeticRows"])
    }
    require(len(approval_arithmetic_by_family) == 23,
            "approval must own 23 unique Material arithmetic rows")
    require(
        set(approval_arithmetic_by_family)
        == {row["familyId"] for row in runtime["familyEvaluators"]},
        "approval arithmetic rows must exactly cover Material evaluator families",
    )
    for order, raw in enumerate(runtime["familyEvaluators"]):
        approval_arithmetic = approval_arithmetic_by_family[raw["familyId"]]
        require(
            approval_arithmetic["familyIdentitySha256"] == raw["familyIdentitySha256"]
            and approval_arithmetic["evaluatorId"] == raw["evaluatorId"]
            and approval_arithmetic["evaluatorVersion"] == raw["evaluatorVersion"]
            and approval_arithmetic["evaluatorSha256"] == raw["evaluatorSha256"]
            and approval_arithmetic["sourceExact"] is False
            and approval_arithmetic["cpuNumericOracleVerified"] is True
            and approval_arithmetic["hlslNumericOracleVerified"] is True,
            f"approval arithmetic row mismatch for {raw['familyId']}",
        )
        registry_id = registry.get(
            "MATERIAL_EVALUATOR", raw["evaluatorId"],
            implementation_version=raw["evaluatorVersion"],
            implementation_sha256=raw["evaluatorSha256"],
            variant=raw["familyId"], consumer_contract="ARTIST_F_COMMON_MATERIAL_EVALUATOR_V1",
        )
        family_registry[raw["familyId"]] = registry_id
        row = {
            "familyId": raw["familyId"],
            "order": order,
            "familyIdentitySha256": raw["familyIdentitySha256"],
            "evaluatorRegistryId": registry_id,
            "evaluatorId": raw["evaluatorId"],
            "evaluatorVersion": raw["evaluatorVersion"],
            "evaluatorSha256": raw["evaluatorSha256"],
            "rendererShapes": raw["rendererShapes"],
            "featureMask": raw["featureMask"],
            "features": raw["features"],
            "graphProvenance": raw["graphProvenance"],
            "cpuNumericOracleVerified": raw["cpuNumericOracleVerified"],
            "hlslNumericOracleVerified": raw["hlslNumericOracleVerified"],
            "numericSamples": normalize_material_family_samples(raw["sampleRows"]),
            "sampleProjectionSha256": canonical_sha256(raw["sampleRows"]),
            "approvalPolicyRowId": approval_arithmetic["policyRowId"],
            "approvalArithmeticOrder": approval_arithmetic_order_by_family[raw["familyId"]],
            "approvalPolicyFamilyId": approval_arithmetic["policyFamilyId"],
            "approvalRequiredOracleIds": approval_arithmetic["requiredOracleIds"],
            "approvalExecutionBlockers": approval_arithmetic["executionBlockers"],
            "approvalArithmeticRow": copy.deepcopy(approval_arithmetic),
            "approvalArithmeticRowSha256": canonical_sha256(approval_arithmetic),
            "preservedBlockers": sorted(set(raw["evidenceBlockers"] + raw["runtimeBlockers"])),
            "sourceExact": False,
        }
        family_rows.append(seal_row(row, "materialFamilies"))

    input_rows: list[dict[str, Any]] = []
    static_rows: list[dict[str, Any]] = []
    render_rows: list[dict[str, Any]] = []
    recipe_input_ids: dict[str, list[str]] = {}
    recipe_static_ids: dict[str, list[str]] = {}
    recipe_render_ids: dict[str, list[str]] = {}
    for recipe in runtime["materialRecipeBindings"]:
        recipe_id = recipe["recipeId"]
        recipe_input_ids[recipe_id] = []
        for raw in recipe["orderedInputBindings"]:
            require(raw["sourceOwnerRecipeId"] == recipe_id,
                    f"material input {raw['fieldId']} owner mismatch")
            variant, _, f64_value, _, _, texture_id, vector_value = material_value_slots(
                raw["typedValue"]
            )
            require(variant in {"F64", "F64X4", "TEXTURE_ID"},
                    "material input variant must be numeric/vector/texture")
            row = {
                "fieldId": raw["fieldId"],
                "recipeId": recipe_id,
                "order": len(input_rows),
                "fieldKind": raw["fieldKind"],
                "bindingRole": raw["bindingRole"],
                "bindingOrigin": raw["bindingOrigin"],
                "sourceSection": raw["sourceSection"],
                "sourceSectionIndex": raw["sourceSectionIndex"],
                "parameterName": raw["parameterName"],
                "normalizedParameterName": raw["normalizedParameterName"],
                "valueVariant": variant,
                "valueF64": f64_value,
                "valueF64x4": vector_value,
                "valueTextureId": texture_id,
                "typedValueSha256": raw["typedValueSha256"],
                "sourceFieldValueSha256": raw["sourceFieldValueSha256"],
                "sourceLineageSha256": raw["sourceLineageSha256"],
                "sourceExact": False,
            }
            input_rows.append(seal_row(row, "materialInputs"))
            recipe_input_ids[recipe_id].append(raw["fieldId"])

        recipe_static_ids[recipe_id] = []
        for raw in recipe["orderedStaticSwitchBindings"]:
            require(raw["sourceOwnerRecipeId"] == recipe_id,
                    f"static field {raw['fieldId']} owner mismatch")
            selected = static_policy_by_field.get(raw["fieldId"])
            require(selected is not None and selected["materialRecipeId"] == recipe_id,
                    f"static field {raw['fieldId']} lacks exact reconstructed policy")
            selected_value = selected["selectedValue"]
            require(
                type(selected_value) is dict
                and selected_value.get("type") == "BOOL"
                and type(selected_value.get("value")) is bool,
                f"static policy {raw['fieldId']} must select a typed bool",
            )
            row = {
                "fieldId": raw["fieldId"],
                "recipeId": recipe_id,
                "order": len(static_rows),
                "parameterName": raw["parameterName"],
                "normalizedParameterName": raw["normalizedParameterName"],
                "bindingOrigin": raw["bindingOrigin"],
                "selectionRole": raw["selectionRole"],
                "sourceValue": raw["typedValue"],
                "selectedValue": selected_value["value"],
                "policyRowId": selected["policyRowId"],
                "sourceFieldValueSha256": raw["sourceFieldValueSha256"],
                "sourceLineageSha256": raw["sourceLineageSha256"],
                "sourceExact": False,
            }
            static_rows.append(seal_row(row, "materialStaticBindings"))
            recipe_static_ids[recipe_id].append(raw["fieldId"])

        recipe_render_ids[recipe_id] = []
        for local_order, raw in enumerate(recipe["renderStateBindings"]):
            policy_row = render_policy_by_owner.get((recipe_id, raw["fieldName"]))
            if raw["typedValue"] is None:
                require(policy_row is not None,
                        f"unresolved render field {recipe_id}/{raw['fieldName']} lacks policy")
                selected = policy_row["selectedValue"]
                variant, bool_value, f64_value, _, enum_value, _ = typed_policy_slots(selected)
                source_blocker = raw["blocker"] or ""
                policy_row_id = policy_row["policyRowId"]
            else:
                require(policy_row is None,
                        f"exact render field {recipe_id}/{raw['fieldName']} was overwritten by policy")
                if type(raw["typedValue"]) is bool:
                    variant, bool_value, f64_value, enum_value = "BOOL", raw["typedValue"], None, ""
                elif type(raw["typedValue"]) in (int, float):
                    variant, bool_value, f64_value, enum_value = "F64", None, float(raw["typedValue"]), ""
                elif type(raw["typedValue"]) is str:
                    variant, bool_value, f64_value, enum_value = "ENUM_STRING", None, None, raw["typedValue"]
                else:
                    raise ContractError("unsupported exact render value")
                source_blocker = ""
                policy_row_id = ""
            render_id = f"{recipe_id}::render:{raw['fieldName']}"
            row = {
                "renderBindingId": render_id,
                "recipeId": recipe_id,
                "order": len(render_rows),
                "fieldName": raw["fieldName"],
                "bindingOrigin": raw["bindingOrigin"],
                "sourceStatus": raw["status"],
                "sourceFidelity": raw["fidelity"],
                "valueVariant": variant,
                "boolValue": bool_value,
                "enumValue": enum_value,
                "f64Value": f64_value,
                "sourceRecordSha256": nullable_sha(raw["sourceRecordSha256"]),
                "policyRowId": policy_row_id,
                "sourceBlocker": source_blocker,
                "sourceExact": False,
            }
            render_rows.append(seal_row(row, "materialRenderBindings"))
            recipe_render_ids[recipe_id].append(render_id)
        require(len(recipe_render_ids[recipe_id]) == 6,
                f"material recipe {recipe_id} must own six render fields")

    recipe_rows: list[dict[str, Any]] = []
    recipe_by_id = {row["recipeId"]: row for row in runtime["materialRecipeBindings"]}
    require(len(recipe_by_id) == 27, "material recipe IDs must be unique")
    for order, raw in enumerate(runtime["materialRecipeBindings"]):
        family_id = raw["familyId"]
        row = {
            "recipeId": raw["recipeId"],
            "order": order,
            "sourceMaterialPath": raw["sourceMaterialPath"],
            "sourceRecipeCompositionSha256": raw["sourceRecipeCompositionSha256"],
            "familyId": family_id,
            "evaluatorRegistryId": family_registry[family_id],
            "inputIds": recipe_input_ids[raw["recipeId"]],
            "staticBindingIds": recipe_static_ids[raw["recipeId"]],
            "renderBindingIds": recipe_render_ids[raw["recipeId"]],
            "numericBindingSamples": normalize_material_recipe_samples(
                raw["numericBindingSamples"]
            ),
            "bindingSha256": raw["bindingSha256"],
            "preservedBlockers": raw["runtimeBlockers"],
            "sourceExact": False,
        }
        recipe_rows.append(seal_row(row, "materialRecipes"))

    emitter_by_evidence = {row["evidenceId"]: row for row in source_emitters}
    occurrence_rows: list[dict[str, Any]] = []
    for order, raw in enumerate(runtime["occurrenceBindings"]):
        recipe = recipe_by_id[raw["materialRecipeId"]]
        emitter = emitter_by_evidence.get(raw["occurrenceId"])
        require(emitter is not None, f"material occurrence {raw['occurrenceId']} lacks emitter")
        require(emitter["rendererType"] == raw["rendererType"],
                f"material occurrence {raw['occurrenceId']} renderer mismatch")
        row = {
            "occurrenceId": raw["occurrenceId"],
            "order": order,
            "emitterId": emitter["emitterId"],
            "cueId": raw["cueId"],
            "rendererType": raw["rendererType"],
            "recipeId": raw["materialRecipeId"],
            "familyId": recipe["familyId"],
            "evaluatorRegistryId": family_registry[recipe["familyId"]],
            "sourceOccurrenceIdentitySha256": raw["sourceOccurrenceIdentitySha256"],
            "bindingSha256": raw["materialBindingSha256"],
            "sourceOccurrenceBindingSha256": raw["bindingSha256"],
            "preservedBlockers": recipe["runtimeBlockers"],
            "sourceExact": False,
        }
        occurrence_rows.append(seal_row(row, "materialOccurrences"))

    policy_rows: list[dict[str, Any]] = []
    for raw in all_policy_source_rows:
        domain = raw["policyKind"]
        require(domain in MATERIAL_POLICY_DOMAINS, f"unknown material policy domain: {domain}")
        implementation = raw["implementation"]
        registry_id = registry.get(
            "MATERIAL_POLICY", implementation["implementationId"],
            variant=domain, consumer_contract=implementation["consumerContract"],
        )
        selected_value = raw.get("selectedValue")
        variant, bool_value, f64_value, enum_type, enum_value, ordinal = typed_policy_slots(
            selected_value
        )
        sampler = None
        if domain == "SAMPLER_DESCRIPTOR":
            require(selected_value is None, "sampler row must not carry selectedValue")
            sampler = normalize_sampler_descriptor(raw["selectedDescriptor"])
        owner = raw["bindingOriginAndOwner"]
        approval_material = approval_material_by_matrix[raw["sourceMatrixRowId"]]
        approval_domain = "SAMPLER" if domain == "SAMPLER_DESCRIPTOR" else domain
        require(approval_material["domain"] == approval_domain
                and approval_material["materialRecipeId"] == raw["materialRecipeId"]
                and approval_material["materialOccurrenceIds"] == raw["materialOccurrenceIds"]
                and approval_material["fieldId"] == raw["fieldId"]
                and approval_material["fieldKind"] == raw["fieldKind"]
                and approval_material["bindingOrigin"] == owner["bindingOrigin"],
                f"approval Material row mismatch for {raw['policyRowId']}")
        require(approval_material["policyFamilyId"] in approval_material_family_ids
                and approval_material["sourceExact"] is False
                and approval_material["executionAdmission"] is False
                and approval_material["productAdmission"] is False,
                f"approval Material row route mismatch for {raw['policyRowId']}")
        row = {
            "policyRowId": raw["policyRowId"],
            "order": raw["policyOrder"],
            "domain": domain,
            "sourceMatrixRowId": raw["sourceMatrixRowId"],
            "recipeId": raw["materialRecipeId"],
            "materialOccurrenceIds": raw["materialOccurrenceIds"],
            "fieldId": raw["fieldId"],
            "fieldKind": raw["fieldKind"],
            "bindingOrigin": owner["bindingOrigin"],
            "evidenceOwnerRecipeId": owner["evidenceOwnerRecipeId"],
            "policyFidelity": raw["policyFidelity"],
            "sourceExact": False,
            "evidenceBlockers": flatten_blockers(raw["evidenceBlockers"]),
            "policySelectionAdmission": raw["policySelectionAdmission"],
            "implementationRegistryId": registry_id,
            "implementationId": implementation["implementationId"],
            "implementationVersion": implementation["implementationVersion"],
            "consumerContract": implementation["consumerContract"],
            "valueVariant": variant,
            "boolValue": bool_value,
            "f64Value": f64_value,
            "enumType": enum_type,
            "enumValue": enum_value,
            "enumOrdinal": ordinal if domain != "SAMPLER_DESCRIPTOR" else None,
            "samplerDescriptor": sampler,
            "providerBasisSha256": canonical_sha256(raw["providerBasis"]),
            "numericOracleSha256": canonical_sha256(raw["numericOracle"]),
            "d3dStateOracleId": raw["d3dStateOracleId"] or "",
            "d3dDescriptorOracle": d3d_descriptor_by_policy.get(raw["policyRowId"]),
            "d3dSrvOracle": d3d_srv_by_policy.get(raw["policyRowId"]),
            "approvalPolicyRowId": approval_material["policyRowId"],
            "approvalUpstreamIdentitySha256": approval_material["upstreamIdentitySha256"],
            "approvalPolicyBindingSha256": approval_material["policyBindingSha256"],
            "approvalPolicyFamilyId": approval_material["policyFamilyId"],
            "approvalMaterialOrder": approval_material_order_by_matrix[
                raw["sourceMatrixRowId"]
            ],
            "approvalRequiredOracleIds": approval_material["requiredOracleIds"],
            "approvalExecutionBlockers": approval_material["executionBlockers"],
            "approvalMaterialRow": copy.deepcopy(approval_material),
            "approvalMaterialRowSha256": canonical_sha256(approval_material),
            "sourceRowSha256": raw["rowSha256"],
        }
        policy_rows.append(seal_row(row, "materialPolicyRows"))

    input_by_id = {row["fieldId"]: row for row in input_rows}
    texture_binding_by_policy: dict[str, dict[str, Any]] = {}
    texture_resource_by_id: dict[str, dict[str, Any]] = {}
    proposal_by_id: dict[str, dict[str, Any]] = {}
    if texture_binding_receipt is not None:
        require(texture_binding_receipt.get("schema")
                == "lostark.artist-31470-material-texture-runtime-binding-receipt"
                and type(texture_binding_receipt.get("formatVersion")) is int
                and texture_binding_receipt["formatVersion"] == 1
                and texture_binding_receipt.get("characterClass") == "ARTIST"
                and texture_binding_receipt.get("skillId") == 31470
                and texture_binding_receipt.get("inputSlot") == "F",
                "Material texture runtime binding receipt identity mismatch")
        texture_binding_by_policy = {
            row["samplerPolicyRowId"]: row
            for row in texture_binding_receipt["materialTextureBindings"]
        }
        texture_resource_by_id = {
            row["textureResourceId"]: row
            for row in texture_binding_receipt["textureResources"]
        }
        proposal_by_id = {
            row["proposalId"]: row
            for row in texture_binding_receipt["provisioningProposals"]
        }
        require(len(texture_binding_by_policy) == 72
                and len(texture_resource_by_id) == 48
                and len(proposal_by_id) == 4
                and texture_binding_receipt["summary"]["resolvedBindingRowCount"] == 68
                and texture_binding_receipt["summary"]["unresolvedBindingRowCount"] == 4
                and texture_binding_receipt["admission"]["product"] is False,
                "Material texture runtime binding receipt denominator mismatch")
    texture_binding_rows: list[dict[str, Any]] = []
    for raw in policy["samplerPolicies"]:
        input_row = input_by_id.get(raw["fieldId"])
        require(input_row is not None
                and input_row["recipeId"] == raw["materialRecipeId"]
                and input_row["valueVariant"] == "TEXTURE_ID",
                f"sampler policy {raw['policyRowId']} lacks exact logical texture input")
        source_binding = texture_binding_by_policy.get(raw["policyRowId"])
        runtime_asset_id: str | None = None
        resolution_status = "UNRESOLVED_EXACT_LOGICAL_TEXTURE_RESOURCE_BINDING"
        binding_basis = "NO_BASENAME_GUESS"
        binding_blockers = ["MATERIAL_TEXTURE_FIELD_RUNTIME_RESOURCE_BINDING_NOT_COMPLETE"]
        source_binding_id = ""
        source_binding_row_sha256 = ""
        source_texture_resource_id = ""
        source_texture_resource_row_sha256 = ""
        source_provisioning_proposal_id = ""
        source_provisioning_proposal_row_sha256 = ""
        source_receipt_status = ""
        if texture_binding_receipt is not None:
            require(source_binding is not None
                    and source_binding["bindingOrder"] == len(texture_binding_rows)
                    and source_binding["recipeId"] == raw["materialRecipeId"]
                    and source_binding["materialInputFieldId"] == raw["fieldId"]
                    and source_binding["materialOccurrenceIds"]
                    == raw["materialOccurrenceIds"]
                    and source_binding["logicalTexturePath"] == input_row["valueTextureId"]
                    and source_binding["samplerPolicyOrder"] == raw["policyOrder"]
                    and source_binding["policySourceRowSha256"] == raw["rowSha256"]
                    and normalize_sampler_descriptor(source_binding["samplerDescriptor"])
                    == next(row["samplerDescriptor"] for row in policy_rows
                            if row["policyRowId"] == raw["policyRowId"]),
                    f"Material texture receipt policy join mismatch: {raw['policyRowId']}")
            owner = source_binding["bindingOriginAndOwner"]
            require(owner == {
                "bindingOrigin": input_row["bindingOrigin"],
                "evidenceOwnerRecipeId": input_row["recipeId"],
                "sourceSection": input_row["sourceSection"],
                "sourceSectionIndex": input_row["sourceSectionIndex"],
            }, f"Material texture receipt owner mismatch: {raw['policyRowId']}")
            require(source_binding["samplerDescriptorSha256"]
                    == canonical_sha256(source_binding["samplerDescriptor"])
                    and source_binding["srvIdentitySha256"]
                    == canonical_sha256(source_binding["srvIdentity"])
                    and source_binding["srvIdentity"] == {
                        "sRgb": source_binding["samplerDescriptor"]["sRgb"],
                        "srvColorSpace": source_binding["samplerDescriptor"]["srvColorSpace"],
                        "lodGroup": source_binding["samplerDescriptor"]["lodGroup"],
                    }, f"Material texture receipt descriptor/SRV mismatch: {raw['policyRowId']}")
            resource = texture_resource_by_id.get(source_binding["textureResourceId"])
            require(resource is not None
                    and resource["logicalTexturePath"] == source_binding["logicalTexturePath"]
                    and resource["status"] == source_binding["status"]
                    and resource["runtimeAssetId"] == source_binding["runtimeAssetId"],
                    f"Material texture receipt resource join mismatch: {raw['policyRowId']}")
            source_binding_id = source_binding["bindingId"]
            source_binding_row_sha256 = source_binding["rowSha256"]
            source_texture_resource_id = resource["textureResourceId"]
            source_texture_resource_row_sha256 = resource["rowSha256"]
            source_receipt_status = source_binding["status"]
            if source_binding["status"] == "RESOLVED_EXACT_RUNTIME_COOK_RECEIPT":
                runtime_asset_id = source_binding["runtimeAssetId"]
                require_safe_renderer_texture_asset_id(runtime_asset_id)
                require(source_binding["runtimeAssetAdmission"] is True
                        and resource["runtimeAssetAdmission"] is True
                        and resource["runtimeCookEvidence"] is not None
                        and resource["resourceExportEvidence"] is not None
                        and resource["provisioningProposalId"] is None,
                        f"resolved Material texture evidence incomplete: {raw['policyRowId']}")
                resolution_status = "RESOLVED_EXACT_RUNTIME_ASSET"
                binding_basis = "EXACT_FULL_LOGICAL_PATH_RUNTIME_COOK_RECEIPT"
                binding_blockers = ["R4_TEXTURE_SRV_CONSUMER_NOT_COMPLETE"]
            else:
                require(source_binding["status"] == "UNRESOLVED_RUNTIME_ASSET"
                        and source_binding["runtimeAssetId"] is None
                        and source_binding["runtimeAssetAdmission"] is False
                        and resource["runtimeAssetAdmission"] is False
                        and resource["exactDdsEvidence"] is not None
                        and resource["provisioningProposalId"] in proposal_by_id,
                        f"unresolved Material texture evidence mismatch: {raw['policyRowId']}")
                proposal = proposal_by_id[resource["provisioningProposalId"]]
                require(proposal["textureResourceId"] == resource["textureResourceId"],
                        f"Material texture provisioning owner mismatch: {raw['policyRowId']}")
                source_provisioning_proposal_id = proposal["proposalId"]
                source_provisioning_proposal_row_sha256 = proposal["rowSha256"]
                resolution_status = "UNRESOLVED_RUNTIME_ASSET"
                binding_basis = "EXACT_DDS_PROVISIONING_PENDING_NO_RUNTIME_ASSET"
                binding_blockers = [
                    "MATERIAL_TEXTURE_RUNTIME_ASSET_UNRESOLVED",
                    "EXACT_DDS_TRANSACTIONAL_DEPLOYMENT_PENDING",
                    "R4_TEXTURE_SRV_CONSUMER_NOT_COMPLETE",
                ]
        row = {
            "bindingId": raw["fieldId"] + "::runtime-texture-binding",
            "order": len(texture_binding_rows),
            "recipeId": raw["materialRecipeId"],
            "materialInputFieldId": raw["fieldId"],
            "logicalTexturePath": input_row["valueTextureId"],
            "samplerPolicyRowId": raw["policyRowId"],
            "materialOccurrenceIds": raw["materialOccurrenceIds"],
            "sourceBindingId": source_binding_id,
            "sourceBindingRowSha256": source_binding_row_sha256,
            "sourceTextureResourceId": source_texture_resource_id,
            "sourceTextureResourceRowSha256": source_texture_resource_row_sha256,
            "sourceProvisioningProposalId": source_provisioning_proposal_id,
            "sourceProvisioningProposalRowSha256": source_provisioning_proposal_row_sha256,
            "sourceReceiptStatus": source_receipt_status,
            "runtimeAssetId": runtime_asset_id if texture_binding_receipt is not None else "",
            "resolutionStatus": resolution_status,
            "bindingBasis": binding_basis,
            "sourceExact": False,
            "blockers": binding_blockers,
        }
        texture_binding_rows.append(seal_row(row, "materialTextureBindings"))
    require(len(texture_binding_rows) == 72,
            "Material logical texture binding denominator must be 72")

    return {
        "materialFamilies": family_rows,
        "materialRecipes": recipe_rows,
        "materialInputs": input_rows,
        "materialStaticBindings": static_rows,
        "materialRenderBindings": render_rows,
        "materialOccurrences": occurrence_rows,
        "materialPolicyRows": policy_rows,
        "materialTextureBindings": texture_binding_rows,
    }


def normalize_geometry_submesh(raw: dict[str, Any]) -> dict[str, Any]:
    counts = raw["channelCounts"]
    digests = raw["channelSha256"]
    require_keys(counts, GEOMETRY_CHANNEL_COUNT_KEYS, "geometry channel counts")
    require_keys(digests, GEOMETRY_CHANNEL_SHA_KEYS, "geometry channel digests")
    row = {
        "name": raw["name"],
        "materialIndex": raw["materialIndex"],
        "vertexCount": raw["vertexCount"],
        "indexCount": raw["indexCount"],
        "channelCounts": counts,
        "channelSha256": digests,
        "boundsF32Hex": raw["boundsF32Hex"],
    }
    require_keys(row, GEOMETRY_SUBMESH_KEYS, "geometry submesh")
    require(len(row["boundsF32Hex"]) == 10, "geometry submesh bounds must contain 10 f32 words")
    return row


def build_geometry_sections(
    binding: dict[str, Any], receipt: dict[str, Any], graph: dict[str, Any],
    candidate: dict[str, Any], source_modules: list[dict[str, Any]],
    source_emitters: list[dict[str, Any]], approval: dict[str, Any],
    identity_route_sha256: str,
) -> dict[str, list[dict[str, Any]]]:
    require(str(binding.get("characterClass", "")).casefold() == "artist" and binding.get("skillId") == 31470,
            "geometry binding target mismatch")
    require(str(receipt.get("characterClass", "")).casefold() == "artist" and receipt.get("skillId") == 31470,
            "geometry receipt target mismatch")
    receipt_by_asset = {row["assetId"]: row for row in receipt["assets"]}
    binding_by_asset = {row["carrierAssetId"]: row for row in binding["bindings"]}
    require(len(receipt_by_asset) == len(binding_by_asset) == 7,
            "geometry carrier asset IDs must be unique")
    require(set(receipt_by_asset) == set(binding_by_asset),
            "geometry binding/receipt asset sets mismatch")
    approval_geometry_by_asset = {row["assetId"]: row for row in approval["geometryRows"]}
    approval_geometry_order_by_asset = {
        row["assetId"]: order for order, row in enumerate(approval["geometryRows"])
    }
    require(len(approval_geometry_by_asset) == 7
            and set(approval_geometry_by_asset) == set(binding_by_asset),
            "approval Geometry rows must exactly cover Geometry carriers")
    carriers: list[dict[str, Any]] = []
    for order, raw in enumerate(binding["bindings"]):
        asset_id = raw["carrierAssetId"]
        evidence = receipt_by_asset.get(asset_id)
        require(evidence is not None, f"geometry asset {asset_id} lacks resource receipt")
        expected = evidence["expectedTuple"]
        require(evidence["sourceObject"] == raw["sourceObject"]
                and evidence["assetId"] == asset_id,
                f"geometry source/asset identity mismatch for {asset_id}")
        binding_expected_tuple = {
            "formatVersion": raw["formatVersion"],
            "channelMask": raw["channelMask"],
            "evidenceFlags": raw["evidenceFlags"],
            "geometryPreScale": raw["geometryPreScale"],
            "geometryPreScaleF32Hex": raw["geometryPreScaleF32Hex"],
            "payloadSha256": raw["payloadSha256"],
            "provenanceSha256": raw["provenanceSha256"],
            "provenanceRole": raw["provenanceRole"],
            "metadataIdentitySha256": raw["provenanceSha256"],
            "submeshes": raw["submeshes"],
        }
        require(expected == binding_expected_tuple,
                f"geometry binding/receipt expected tuple mismatch for {asset_id}")
        approval_geometry = approval_geometry_by_asset[asset_id]
        require(
            approval_geometry["sourceObject"] == raw["sourceObject"]
            and approval_geometry["candidateResourceSha256"]
            == evidence["candidateResource"]["sha256"]
            and approval_geometry["payloadSha256"] == raw["payloadSha256"]
            and approval_geometry["metadataIdentitySha256"]
            == expected["metadataIdentitySha256"]
            and approval_geometry["geometryPreScale"] == raw["geometryPreScale"]
            and approval_geometry["sourceExact"] is False
            and approval_geometry["executionAdmission"] is False
            and approval_geometry["productAdmission"] is False,
            f"approval Geometry row mismatch for {asset_id}",
        )
        expected_tuple_sha = canonical_sha256(expected)
        row = {
            "carrierId": "geometry-carrier-" + canonical_sha256(
                {"bindingId": raw["bindingId"], "assetId": asset_id}
            )[:20],
            "order": order,
            "bindingId": raw["bindingId"],
            "sourceObject": raw["sourceObject"],
            "assetId": asset_id,
            "formatVersion": raw["formatVersion"],
            "candidateResourceByteSize": evidence["candidateResource"]["byteSize"],
            "candidateResourceSha256": evidence["candidateResource"]["sha256"],
            "payloadSha256": raw["payloadSha256"],
            "provenanceSha256": raw["provenanceSha256"],
            "provenanceRole": expected["provenanceRole"],
            "metadataIdentitySha256": expected["metadataIdentitySha256"],
            "cacheIdentitySha256": raw["cacheIdentitySha256"],
            "geometryPreScale": raw["geometryPreScale"],
            "geometryPreScaleF32Hex": raw["geometryPreScaleF32Hex"],
            "channelMask": raw["channelMask"],
            "evidenceFlags": raw["evidenceFlags"],
            "expectedTupleSha256": expected_tuple_sha,
            "submeshes": [normalize_geometry_submesh(item) for item in raw["submeshes"]],
            "approvalGeometryOrder": approval_geometry_order_by_asset[asset_id],
            "approvalGeometryRow": copy.deepcopy(approval_geometry),
            "approvalGeometryRowSha256": canonical_sha256(approval_geometry),
            "preparedCacheIdentitySha256": canonical_sha256({
                "assetId": asset_id,
                "candidateResourceSha256": evidence["candidateResource"]["sha256"],
                "expectedTupleSha256": expected_tuple_sha,
                "identityRouteSha256": identity_route_sha256,
            }),
            "sourceExact": False,
            "preScaleConsumed": False,
        }
        carriers.append(seal_row(row, "geometryCarriers"))
    carrier_by_asset = {row["assetId"]: row for row in carriers}

    module_by_emitter: dict[str, dict[str, Any]] = {}
    for module in source_modules:
        if module["exactSourceClass"] == "particlemoduletypedatamesh":
            require(module["emitterId"] not in module_by_emitter,
                    f"duplicate TypeDataMesh module for {module['emitterId']}")
            module_by_emitter[module["emitterId"]] = module
    emitter_by_evidence = {row["evidenceId"]: row for row in source_emitters}
    candidate_by_evidence: dict[str, dict[str, Any]] = {}
    for element in candidate["elements"]:
        evidence_id = element["sourceRecipe"]["compilerEvidence"]["evidenceId"]
        require(evidence_id not in candidate_by_evidence,
                f"duplicate candidate evidence ID: {evidence_id}")
        candidate_by_evidence[evidence_id] = element
    graph_system_by_id = {
        row["sourceSystemId"].casefold(): row for row in graph["sourceSystems"]
    }
    runtime_resource_by_object: dict[str, dict[str, Any]] = {}
    for resource in graph["runtimeResourceBindings"]:
        if resource["role"] != "mesh":
            continue
        key = resource["sourceObjectPath"].casefold()
        require(key not in runtime_resource_by_object,
                f"duplicate normalized mesh runtime binding: {resource['sourceObjectPath']}")
        runtime_resource_by_object[key] = resource
    source_literal_by_id: dict[str, dict[str, Any]] = {}
    for element in candidate["elements"]:
        for module in element["sourceRecipe"]["modules"]:
            for literal in module["literals"]:
                source_literal_by_id[literal.get("literalId", "")] = literal

    uses: list[dict[str, Any]] = []
    for emitter in source_emitters:
        if emitter["rendererType"] != "MeshParticle":
            continue
        module = module_by_emitter.get(emitter["emitterId"])
        require(module is not None, f"Mesh emitter {emitter['emitterId']} lacks TypeDataMesh")
        element = candidate_by_evidence[emitter["evidenceId"]]
        resources = [row for row in element["resources"] if row["slotId"] == "meshModel"]
        require(len(resources) == 1,
                f"Mesh emitter {emitter['emitterId']} must own one meshModel resource")
        asset_id = resources[0]["assetId"]
        carrier = carrier_by_asset.get(asset_id)
        require(carrier is not None, f"Mesh resource {asset_id} has no geometry carrier")
        raw_module = next(
            row for row in element["sourceRecipe"]["modules"]
            if row["stableId"].split("@ref:", 1)[0] == module["sourceObjectId"]
        )
        mesh_literals = [row for row in raw_module["literals"] if row["propertyPath"] == "mesh"]
        require(len(mesh_literals) == 1 and mesh_literals[0]["kind"] == "number",
                f"TypeDataMesh {module['moduleId']} must own one numeric mesh package reference")
        package_index_f64 = float(mesh_literals[0]["value"])
        require(package_index_f64.is_integer() and package_index_f64 < 0,
                f"TypeDataMesh {module['moduleId']} mesh reference must be a negative import index")
        package_index = int(package_index_f64)
        system = graph_system_by_id.get(emitter["sourceSystemId"].casefold())
        require(system is not None, f"normalized graph lacks {emitter['sourceSystemId']}")
        import_rows = [
            row for row in system["resourceBindings"]
            if row.get("sourceNodeId") == module["sourceObjectId"]
            and row.get("property") == "mesh"
            and row.get("packageIndex") == package_index
            and row.get("role") == "mesh"
        ]
        require(len(import_rows) == 1,
                f"mesh import {module['moduleId']}/{package_index} must resolve exactly once")
        import_row = import_rows[0]
        require(import_row["objectPath"].casefold() == carrier["sourceObject"].casefold(),
                f"mesh import object path mismatch for {module['moduleId']}")
        runtime_resource = runtime_resource_by_object.get(import_row["objectPath"].casefold())
        require(runtime_resource is not None,
                f"mesh import lacks normalized runtime binding for {module['moduleId']}")
        require(runtime_resource["resolutionStatus"] == "RESOLVED_RUNTIME_ASSET"
                and runtime_resource["candidateCount"] == 1,
                f"mesh runtime binding is not uniquely resolved for {module['moduleId']}")
        require(runtime_resource["assetId"] == asset_id,
                f"mesh import asset ID mismatch for {module['moduleId']}")
        row = {
            "geometryUseId": f"{emitter['emitterId']}::geometry-use",
            "order": len(uses),
            "emitterId": emitter["emitterId"],
            "moduleId": module["moduleId"],
            "sourceEmitterPath": emitter["sourceEmitterPath"],
            "sourceMeshPackageRef": package_index,
            "carrierId": carrier["carrierId"],
            "assetId": asset_id,
            "sizeSemantics": "DIMENSIONLESS_PARTICLE_SIZE",
            "preScaleApplication": "VERTEX_AND_BOUNDS_EXACTLY_ONCE_REQUIRED",
            "preScaleConsumed": False,
            "sourceExact": False,
        }
        uses.append(seal_row(row, "geometryUses"))
    require(len(uses) == 13, "geometry use join must produce exactly 13 Mesh uses")
    return {"geometryCarriers": carriers, "geometryUses": uses}


def cue_particle_system_id(cue: dict[str, Any]) -> str:
    value = cue["typedPayload"]["sourceParticleSystem"]
    require(type(value) is str and value.startswith("ParticleSystem'") and value.endswith("'"),
            "action cue ParticleSystem reference malformed")
    return value[len("ParticleSystem'"):-1].casefold()


def normalized_action_cue_program_projection(cue: dict[str, Any]) -> dict[str, Any]:
    payload = cue["typedPayload"]
    attachment_raw = payload["attachment"]
    local = copy.deepcopy(payload["localTransform"])
    require_keys(local, CUE_TRANSFORM_KEYS, "action cue local transform")
    socket = copy.deepcopy(attachment_raw["socketLocalTransform"])
    require_keys(socket, SOCKET_TRANSFORM_KEYS, "action cue socket transform")
    follows_named_anchor = attachment_raw["mode"] == "FOLLOW_NAMED_ANCHORS"
    require(attachment_raw["mode"] in {"FOLLOW_NAMED_ANCHORS", "SNAPSHOT_ROOT"},
            "action cue attachment mode unsupported")
    attachment = {
        "enabled": follows_named_anchor,
        "follow": follows_named_anchor,
        "sourceAnchorSlotId": (attachment_raw["sourceAnchorNames"][0]
                               if follows_named_anchor else ""),
        "runtimeAnchorSlotId": attachment_raw["runtimeAnchorSlotId"],
        "runtimeBoneName": attachment_raw["runtimeBoneName"],
        "socketLocalTransform": socket,
    }
    require_keys(attachment, ATTACHMENT_KEYS, "action cue attachment projection")
    parameter_rows = []
    for raw in payload["parameterOverrides"]:
        parameter_rows.append({
            "name": raw["name"],
            "kind": raw["type"].upper(),
            "scalarValue": (float(raw["scalarValue"])
                            if raw["type"] == "scalar" else None),
            "vectorValue": ([float(item) for item in raw["vectorValue"]]
                            if raw["type"] == "vector" else []),
            "sourceIndex": raw["sourceIndex"],
            "sourceValueByteOffset": raw["sourceValueByteOffset"],
        })
    parameter_rows.sort(key=lambda item: item["name"])
    for item in parameter_rows:
        validate_action_cue_row(item, "action cue parameter projection")
    projection = {
        "sourceCueId": cue["cueId"],
        "sourceOccurrenceId": cue["sourceOccurrence"]["notifyId"],
        "sourceSystemId": cue_particle_system_id(cue),
        "cueLocalTransform": local,
        "actionCueAttachment": attachment,
        "actionCueParameterInputs": parameter_rows,
    }
    return projection


def build_action_schedules(
    action_cue_recipe: dict[str, Any], source_semantics: dict[str, Any],
    emitters: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    require(action_cue_recipe["schema"] == "lostark.effect-action-cue-recipe"
            and type(action_cue_recipe["formatVersion"]) is int
            and action_cue_recipe["formatVersion"] == 2
            and action_cue_recipe["characterClass"] == "ARTIST"
            and action_cue_recipe["skillId"] == 31470
            and action_cue_recipe["inputSlot"] == "F",
            "action cue recipe root identity mismatch")
    action_input = source_semantics["inputs"]["actionCueRecipe"]
    require(action_input["path"] == ACTION_CUE_RECIPE_PATH
            and action_input["canonicalJsonSha256"]
            == canonical_sha256(action_cue_recipe),
            "Source semantics action cue recipe identity mismatch")
    cue_by_occurrence = {
        cue["sourceOccurrence"]["notifyId"]: cue
        for cue in action_cue_recipe["cues"]
        if cue["sourceType"] == "PlayParticleEffect"
    }
    required_occurrences = {row[1] for row in EXPECTED_ACTION_SCHEDULES}
    require(required_occurrences.issubset(cue_by_occurrence),
            "action cue recipe lacks a required particle occurrence")
    schedules: list[dict[str, Any]] = []
    for order, expected in enumerate(EXPECTED_ACTION_SCHEDULES):
        (cue_id, occurrence_id, system_id, event_index, global_time,
         duration, cue_sha) = expected
        cue = cue_by_occurrence[occurrence_id]
        require(canonical_sha256(cue) == cue_sha
                and cue["cueId"] == cue_id
                and cue["executionEnabled"] is True
                and cue["sourceOccurrence"]["enabled"] is True
                and cue["typedPayload"]["enabled"] is True
                and cue_particle_system_id(cue) == system_id
                and cue["sourceReceiptEventIndex"] == event_index
                and cue["globalTimeSeconds"] == global_time
                and cue["durationSeconds"] == duration,
                f"action cue recipe row {occurrence_id} mismatch")
        projection = {
            "order": order,
            "sourceCueId": cue_id,
            "sourceOccurrenceId": occurrence_id,
            "sourceSystemId": system_id,
            "sourceReceiptEventIndex": event_index,
            "globalTimeSeconds": float(global_time),
            "durationSeconds": float(duration),
            "sourceCueRowSha256": cue_sha,
        }
        row = {
            "scheduleId": "action-schedule-" + canonical_sha256(projection)[:24],
            **projection,
        }
        schedules.append(seal_row(row, "actionSchedules"))
    schedule_by_occurrence = {
        row["sourceOccurrenceId"]: row for row in schedules
    }
    coverage = {row["scheduleId"]: 0 for row in schedules}
    for emitter in emitters:
        schedule = schedule_by_occurrence.get(emitter["sourceOccurrenceId"])
        require(schedule is not None
                and schedule["sourceCueId"] == emitter["sourceCueId"]
                and schedule["sourceSystemId"] == emitter["sourceSystemId"].casefold(),
                f"emitter {emitter['emitterId']} action schedule join mismatch")
        coverage[schedule["scheduleId"]] += 1
    require(list(coverage.values()) == [4, 1, 15, 12, 1, 1, 1],
            "action schedule emitter coverage mismatch")
    return schedules


def bind_emitter_direct_ids(
    emitters: list[dict[str, Any]], material_occurrences: list[dict[str, Any]],
    material_recipes: list[dict[str, Any]], texture_resources: list[dict[str, Any]],
    geometry_uses: list[dict[str, Any]], modules: list[dict[str, Any]],
    distributions: list[dict[str, Any]], literals: list[dict[str, Any]],
    implicit_defaults: list[dict[str, Any]], point_light_fields: list[dict[str, Any]],
    action_schedules: list[dict[str, Any]], action_cue_recipe: dict[str, Any],
) -> list[dict[str, Any]]:
    materials_by_emitter: dict[str, list[str]] = {}
    for row in material_occurrences:
        materials_by_emitter.setdefault(row["emitterId"], []).append(row["occurrenceId"])
    geometry_by_emitter: dict[str, list[str]] = {}
    for row in geometry_uses:
        geometry_by_emitter.setdefault(row["emitterId"], []).append(row["geometryUseId"])
    textures_by_emitter: dict[str, list[str]] = {}
    for row in texture_resources:
        textures_by_emitter.setdefault(row["emitterId"], []).append(row["textureResourceId"])
    recipe_by_id = {row["recipeId"]: row for row in material_recipes}
    schedule_by_occurrence = {
        row["sourceOccurrenceId"]: row for row in action_schedules
    }
    cue_by_occurrence = {
        cue["sourceOccurrence"]["notifyId"]: cue
        for cue in action_cue_recipe["cues"]
        if cue["sourceType"] == "PlayParticleEffect"
    }
    result: list[dict[str, Any]] = []
    for raw in emitters:
        row = copy.deepcopy(raw)
        row.pop("rowSha256")
        material_ids = materials_by_emitter.get(row["emitterId"], [])
        geometry_ids = geometry_by_emitter.get(row["emitterId"], [])
        texture_ids = textures_by_emitter.get(row["emitterId"], [])
        schedule = schedule_by_occurrence.get(row["sourceOccurrenceId"])
        cue = cue_by_occurrence.get(row["sourceOccurrenceId"])
        require(schedule is not None and cue is not None
                and schedule["sourceCueId"] == row["sourceCueId"],
                f"emitter {row['emitterId']} action cue authority missing")
        cue_projection = normalized_action_cue_program_projection(cue)
        require(cue_projection["sourceSystemId"] == row["sourceSystemId"].casefold(),
                f"emitter {row['emitterId']} action cue system mismatch")
        row["scheduleId"] = schedule["scheduleId"]
        row["cueLocalTransform"] = cue_projection["cueLocalTransform"]
        row["actionCueAttachment"] = cue_projection["actionCueAttachment"]
        row["actionCueParameterInputs"] = cue_projection["actionCueParameterInputs"]
        row["sourceActionCueProjectionSha256"] = canonical_sha256(cue_projection)
        row["anchorRequests"] = []
        if row["actionCueAttachment"]["enabled"]:
            anchor = {
                "anchorRequestId": row["emitterId"] + "::anchor:action-cue",
                "order": 0,
                "sourceKind": "ACTION_CUE",
                "sourceModuleId": "",
                "sourceAnchorSlotId": row["actionCueAttachment"]["sourceAnchorSlotId"],
                "runtimeAnchorSlotId": row["actionCueAttachment"]["runtimeAnchorSlotId"],
                "runtimeBoneName": row["actionCueAttachment"]["runtimeBoneName"],
                "socketLocalTransform": copy.deepcopy(
                    row["actionCueAttachment"]["socketLocalTransform"]
                ),
                "follow": row["actionCueAttachment"]["follow"],
            }
            require_keys(anchor, ANCHOR_REQUEST_KEYS, "emitter ActionCue anchor")
            row["anchorRequests"].append(anchor)
        if row["rendererType"] == "LightParticle":
            require(material_ids == [], "Light emitter must not own a Material occurrence")
            row["materialOccurrenceId"] = None
            require(texture_ids == [], "Light emitter must not own renderer textures")
        else:
            require(len(material_ids) == 1,
                    f"non-Light emitter {row['emitterId']} must own one Material occurrence")
            row["materialOccurrenceId"] = material_ids[0]
        row["textureResourceIds"] = texture_ids
        row["screenPostAdapter"] = None
        row["lightAdapter"] = None
        row["decalAdapter"] = None
        row["ribbonAdapter"] = None
        if row["rendererType"] == "ScreenPost":
            occurrence = next(
                item for item in material_occurrences
                if item["occurrenceId"] == row["materialOccurrenceId"]
            )
            recipe = recipe_by_id[occurrence["recipeId"]]
            require(recipe["sourceMaterialPath"].casefold().endswith(
                ".fx_c_pa_zoomblur_01_tr"
            ), "ScreenPost Material recipe is not the frozen zoom-blur recipe")
            owned_distributions = [
                item for item in distributions if item["moduleId"] in {
                    module["moduleId"] for module in modules
                    if module["emitterId"] == row["emitterId"]
                }
            ]
            intensity = [
                item for item in owned_distributions
                if item["propertyPath"] == "dynamicparams[0].paramvalue"
            ]
            alpha = [
                item for item in owned_distributions
                if item["propertyPath"] == "alphaoverlife"
            ]
            require(len(intensity) == len(alpha) == 1,
                    "ScreenPost zoom adapter distribution binding mismatch")
            require(intensity[0]["lookupTable"][:3] == [0.0, 0.5, 0.0],
                    "ScreenPost blurstrength curve leading oracle mismatch")
            adapter = {
                "adapterId": "ZOOM_BLUR_RECONSTRUCTED_V1",
                "enabled": True,
                "occurrenceId": row["sourceOccurrenceId"],
                "materialOccurrenceId": occurrence["occurrenceId"],
                "recipeId": recipe["recipeId"],
                "familyId": recipe["familyId"],
                "intensityDistributionId": intensity[0]["distributionId"],
                "alphaDistributionId": alpha[0]["distributionId"],
                "secondaryIntensity": 0.0,
                "frequencyPolicy": "IRRELEVANT_ZOOM_PROFILE",
                "tint": [1.0, 1.0, 1.0, 1.0],
                "sourceSpace": "screenSpaceV1",
                "fidelity": "RECONSTRUCTED_APPROVED_V1",
                "sourceExact": False,
                "blockers": ["R5_SCREEN_POST_RUNTIME_PROBE_NOT_COMPLETE"],
            }
            adapter["adapterSha256"] = canonical_sha256(adapter)
            require_keys(adapter, SCREEN_POST_ADAPTER_KEYS, "ScreenPost adapter")
            row["screenPostAdapter"] = adapter
        if row["rendererType"] == "LightParticle":
            light_modules = [
                module for module in modules
                if module["emitterId"] == row["emitterId"]
                and module["exactSourceClass"] == "efparticlemoduletypedatalight"
            ]
            require(len(light_modules) == 1, "Light adapter module binding mismatch")
            field_ids = [
                field["fieldId"] for field in point_light_fields
                if field["moduleId"] == light_modules[0]["moduleId"]
            ]
            require(len(field_ids) == 8, "Light adapter field denominator mismatch")
            adapter = {
                "adapterId": "POINT_LIGHT_TYPED_COMPONENT_V1",
                "enabled": True,
                "moduleId": light_modules[0]["moduleId"],
                "fieldIds": field_ids,
                "positionSourcePolicy": "EMITTER_TRANSFORM_PLUS_PARTICLE_LOCATION",
                "ueUnitScale": 0.01,
                "sourceExact": False,
                "blockers": ["R5_POINT_LIGHT_RUNTIME_PROBE_NOT_COMPLETE"],
            }
            adapter["adapterSha256"] = canonical_sha256(adapter)
            require_keys(adapter, LIGHT_ADAPTER_KEYS, "Light adapter")
            row["lightAdapter"] = adapter
        if row["rendererType"] == "DecalParticle":
            owned_modules = [module for module in modules if module["emitterId"] == row["emitterId"]]
            decal_modules = [
                module for module in owned_modules
                if module["exactSourceClass"] == "efparticlemoduletypedatadecal"
                and module["handlerVariant"] == "EF_DECAL_DESCRIPTOR"
            ]
            size_modules = [
                module for module in owned_modules
                if module["exactSourceClass"] == "particlemodulesize"
            ]
            require(len(decal_modules) == len(size_modules) == 1,
                    "Decal adapter module binding mismatch")
            defaults = [
                item for item in implicit_defaults
                if item["moduleId"] == decal_modules[0]["moduleId"]
                and item["variant"] == "DECAL_DEFAULTS"
            ]
            size_distributions = [
                item for item in distributions
                if item["moduleId"] == size_modules[0]["moduleId"]
                and item["propertyPath"] == "startsize"
            ]
            require(len(defaults) == len(size_distributions) == 1,
                    "Decal adapter default/distribution binding mismatch")
            values = defaults[0]["decalValues"]
            near_literals = [
                item for item in literals
                if item["moduleId"] == decal_modules[0]["moduleId"]
                and item["propertyPath"] == "nearplane"
            ]
            require(len(near_literals) == 1
                    and near_literals[0]["variant"] == "F64",
                    "Decal near-plane literal binding mismatch")
            near_plane = near_literals[0]["f64Value"]
            far_plane = float(values["farPlane"])
            adapter = {
                "adapterId": "EF_DECAL_TYPED_DESCRIPTOR_V1",
                "enabled": True,
                "moduleId": decal_modules[0]["moduleId"],
                "defaultId": defaults[0]["defaultId"],
                "sizeDistributionId": size_distributions[0]["distributionId"],
                "nearPlane": near_plane,
                "farPlane": far_plane,
                "defaultSize": [float(value) for value in values["defaultSize"]],
                "blendRange": [float(value) for value in values["blendRange"]],
                "yawOnlyCdoDefault": values["onlyCalculateRotationYaw"],
                "yawOnlyCapabilityOutput": True,
                "yawOnlyDecision": "RECONSTRUCTED_CDO_AND_CAPABILITY_AGREE",
                "executionAdmission": True,
                "supports3dDrawMode": values["supports3dDrawMode"],
                "depthWorldUnits": far_plane - near_plane,
                "depthRuntimeUnits": (far_plane - near_plane) * 0.01,
                "fidelity": "RECONSTRUCTED_APPROVED_V1",
                "sourceExact": False,
                "blockers": ["R5_DECAL_RUNTIME_PROBE_NOT_COMPLETE"],
            }
            adapter["adapterSha256"] = canonical_sha256(adapter)
            require_keys(adapter, DECAL_ADAPTER_KEYS, "Decal adapter")
            row["decalAdapter"] = adapter
        if row["rendererType"] == "CascadeRibbon":
            owned_modules = sorted(
                (module for module in modules if module["emitterId"] == row["emitterId"]),
                key=lambda item: item["order"],
            )
            def exact_modules(*classes: str) -> list[dict[str, Any]]:
                return [module for module in owned_modules
                        if module["exactSourceClass"] in classes]
            type_data = exact_modules("particlemoduletypedataribbon")
            spawn = exact_modules("particlemodulespawn")
            lifetime = exact_modules("particlemodulelifetime")
            size = exact_modules("particlemodulesize")
            color = exact_modules("particlemodulecolor", "particlemodulecolorscaleoverlife")
            dynamic = exact_modules("particlemoduleparameterdynamic")
            require(len(type_data) == len(spawn) == len(lifetime) == len(size)
                    == len(dynamic) == 1 and len(color) == 2,
                    "Ribbon adapter ordered module binding mismatch")
            defaults = [
                item for item in implicit_defaults
                if item["moduleId"] == type_data[0]["moduleId"]
                and item["variant"] == "RIBBON_DEFAULTS"
            ]
            require(len(defaults) == 1, "Ribbon adapter default binding mismatch")
            ribbon_values = defaults[0]["ribbonValues"]
            type_literals = {
                item["propertyPath"]: item for item in literals
                if item["moduleId"] == type_data[0]["moduleId"]
            }
            require(type_literals["tilingdistance"]["f64Value"] == 600.0
                    and type_literals["distancetessellationstepsize"]["f64Value"] == 5.0,
                    "Ribbon source tiling/tessellation values mismatch")
            def distribution_ids_for(module_rows: list[dict[str, Any]]) -> list[str]:
                module_ids = {module["moduleId"] for module in module_rows}
                return [item["distributionId"] for item in distributions
                        if item["moduleId"] in module_ids]
            typed_max = ribbon_values["maxParticleInTrailCount"]
            adapter = {
                "adapterId": "CASCADE_RIBBON_TYPED_ADAPTER_V1",
                "enabled": True,
                "typeDataModuleId": type_data[0]["moduleId"],
                "defaultId": defaults[0]["defaultId"],
                "spawnModuleId": spawn[0]["moduleId"],
                "lifetimeModuleId": lifetime[0]["moduleId"],
                "sizeModuleId": size[0]["moduleId"],
                "colorModuleIds": [module["moduleId"] for module in color],
                "dynamicParameterModuleId": dynamic[0]["moduleId"],
                "spawnDistributionIds": distribution_ids_for(spawn),
                "lifetimeDistributionIds": distribution_ids_for(lifetime),
                "sizeDistributionIds": distribution_ids_for(size),
                "colorDistributionIds": distribution_ids_for(color),
                "dynamicDistributionIds": distribution_ids_for(dynamic),
                "tilingDistance": 600.0,
                "distanceTessellationStepSize": 5.0,
                "typedMaxParticleInTrailCount": typed_max,
                "operationalMaxPoints": typed_max,
                "widthPolicy": "TYPED_SIZE_DISTRIBUTION_X_AXIS",
                "geometryPolicy": "TYPED_RIBBON_DEFAULTS_RENDER_GEOMETRY",
                "orientationPolicy": "CAMERA_FACING_SINGLE_SHEET_RECONSTRUCTED_V1",
                "fidelity": "RECONSTRUCTED_TYPED_SOURCE_WITH_EXPLICIT_ORIENTATION_POLICY_V1",
                "sourceExact": False,
                "blockers": [
                    "R5_RIBBON_RUNTIME_PROBE_NOT_COMPLETE",
                    "RIBBON_ORIENTATION_POLICY_REQUIRES_RUNTIME_PROBE",
                ],
            }
            adapter["adapterSha256"] = canonical_sha256(adapter)
            require_keys(adapter, RIBBON_ADAPTER_KEYS, "Ribbon adapter")
            row["ribbonAdapter"] = adapter
        if row["rendererType"] == "MeshParticle":
            require(len(geometry_ids) == 1,
                    f"Mesh emitter {row['emitterId']} must own one Geometry use")
            row["geometryUseId"] = geometry_ids[0]
        else:
            require(geometry_ids == [], "non-Mesh emitter must not own Geometry use")
            row["geometryUseId"] = None
        result.append(seal_row(row, "emitters"))
    require(sum(row["materialOccurrenceId"] is not None for row in result) == 34,
            "emitter Material direct binding count must be 34")
    require(sum(row["geometryUseId"] is not None for row in result) == 13,
            "emitter Geometry direct binding count must be 13")
    require(
        {row["materialOccurrenceId"] for row in result if row["materialOccurrenceId"]}
        == {row["occurrenceId"] for row in material_occurrences},
        "emitter Material reverse coverage mismatch",
    )
    require(
        {row["geometryUseId"] for row in result if row["geometryUseId"]}
        == {row["geometryUseId"] for row in geometry_uses},
        "emitter Geometry reverse coverage mismatch",
    )
    require(
        {resource_id for row in result for resource_id in row["textureResourceIds"]}
        == {row["textureResourceId"] for row in texture_resources},
        "emitter renderer texture reverse coverage mismatch",
    )
    return result


def require_safe_renderer_texture_asset_id(asset_id: Any) -> None:
    require(type(asset_id) is str and asset_id != "",
            "renderer texture assetId must be non-empty string")
    require("\\" not in asset_id and ":" not in asset_id,
            "renderer texture assetId must use Resources-relative forward slashes")
    parts = asset_id.split("/")
    require(all(part not in ("", ".", "..") for part in parts),
            "renderer texture assetId contains unsafe path segment")
    require(asset_id.startswith("Effect/") and asset_id.casefold().endswith(".dds"),
            "renderer texture assetId must be an Effect Resources-relative DDS")


def build_renderer_texture_resources(
    candidate: dict[str, Any], source_emitters: list[dict[str, Any]],
    material_occurrences: list[dict[str, Any]], material_recipes: list[dict[str, Any]],
) -> dict[str, list[dict[str, Any]]]:
    element_by_source_node = {row["sourceNode"]: row for row in candidate["elements"]}
    require(len(element_by_source_node) == 35,
            "source candidate texture join requires 35 unique sourceNode rows")
    occurrence_by_emitter = {row["emitterId"]: row for row in material_occurrences}
    recipe_by_id = {row["recipeId"]: row for row in material_recipes}
    rows: list[dict[str, Any]] = []
    owner_slots: set[tuple[str, str]] = set()
    for emitter in source_emitters:
        element = element_by_source_node.get(emitter["sourceNode"])
        require(element is not None, "renderer texture emitter lacks source candidate element")
        resources = element["resources"]
        require(type(resources) is list, "candidate element resources must be array")
        if emitter["rendererType"] == "LightParticle":
            require(resources == [], "Light emitter candidate resources must be empty")
            continue
        occurrence = occurrence_by_emitter.get(emitter["emitterId"])
        require(occurrence is not None, "renderer texture emitter lacks Material occurrence")
        recipe = recipe_by_id[occurrence["recipeId"]]
        source_material_path = element["material"]["sourceMaterialPath"]
        require(source_material_path == recipe["sourceMaterialPath"],
                "renderer texture source material path mismatch")
        for resource in resources:
            require(type(resource) is dict, "renderer texture resource must be object")
            require_keys(resource, ("slotId", "assetId"), "renderer texture source resource")
            slot_id = resource["slotId"]
            asset_id = resource["assetId"]
            if slot_id == "meshModel":
                require(emitter["rendererType"] == "MeshParticle"
                        and type(asset_id) is str
                        and asset_id.casefold().endswith(".wmodel"),
                        "meshModel resource must be owned by Geometry Mesh path")
                continue
            require(slot_id in RENDERER_TEXTURE_SLOTS,
                    f"unsupported renderer texture slot: {slot_id}")
            require_safe_renderer_texture_asset_id(asset_id)
            owner_slot = (emitter["emitterId"], slot_id)
            require(owner_slot not in owner_slots, "duplicate emitter renderer texture slot")
            owner_slots.add(owner_slot)
            projection = {
                "sourceNode": emitter["sourceNode"],
                "sourceMaterialPath": source_material_path,
                "slotId": slot_id,
                "assetId": asset_id,
            }
            row = {
                "textureResourceId": (
                    emitter["emitterId"] + "::renderer-texture:" + slot_id
                ),
                "order": len(rows),
                "emitterId": emitter["emitterId"],
                "sourceNode": emitter["sourceNode"],
                "materialOccurrenceId": occurrence["occurrenceId"],
                "sourceMaterialPath": source_material_path,
                "slotId": slot_id,
                "assetId": asset_id,
                "sourceResourceProjectionSha256": canonical_sha256(projection),
                "projectionRole": "RECONSTRUCTED_RENDERER_SLOT_PROJECTION",
                "blockers": ["MATERIAL_TEXTURE_FIELD_RUNTIME_RESOURCE_BINDING_NOT_COMPLETE"],
                "sourceExact": False,
            }
            rows.append(seal_row(row, "rendererTextureResources"))
    require(len(rows) == 57, "renderer texture resource denominator must be 57")
    counts = {slot: sum(row["slotId"] == slot for row in rows)
              for slot in sorted(RENDERER_TEXTURE_SLOTS)}
    require(counts == {"base": 19, "dissolve": 5, "emissive": 3,
                       "mask": 15, "noise": 15},
            "renderer texture slot denominator mismatch")
    return {"rendererTextureResources": rows}


def build_particle_system_policy(candidate: dict[str, Any]) -> dict[str, Any]:
    raw = candidate["particleSystem"]
    policy = {
        "uniformScaleMultiplier": float(raw["uniformScaleMultiplier"]),
        "yawOffsetDegrees": float(raw["yawOffsetDegrees"]),
        "directionYawDegrees": float(raw["directionYawDegrees"]),
        "initialSpeedMultiplier": float(raw["initialSpeedMultiplier"]),
    }
    assert_finite_tree(policy, "particleSystemPolicy")
    require(policy["uniformScaleMultiplier"] > 0.0,
            "particleSystemPolicy uniform scale must be positive")
    require(policy["initialSpeedMultiplier"] > 0.0,
            "particleSystemPolicy initial speed must be positive")
    policy["policySha256"] = canonical_sha256(policy)
    require_keys(policy, PARTICLE_SYSTEM_POLICY_KEYS, "particleSystemPolicy")
    return policy


def build_target_identity(documents: dict[str, dict[str, Any]]) -> dict[str, Any]:
    approval_target = documents["approval"]["target"]
    require(approval_target == {
        "characterClass": "ARTIST", "skillId": 31470, "inputSlot": "F",
        "effectAssetId": "effect.artist.skill.31470.f",
    }, "approval target identity mismatch")
    require(
        documents["sourceCandidate"]["effectAssetId"]
        == "effect.artist.skill.31470.native-v14.source-contract-candidate",
        "source candidate identity mismatch",
    )
    require(documents["geometryBinding"]["assetId"] == "effect.artist.skill.31470",
            "Geometry binding asset identity mismatch")
    projection = {
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "policyTargetId": "effect.artist.skill.31470.f",
        "sourceCandidateId": (
            "effect.artist.skill.31470.native-v14.source-contract-candidate"
        ),
        "geometryBindingAssetId": "effect.artist.skill.31470",
        "runtimeCatalogAssetId": "effect.artist.skill.31470",
        "identityRouteId": "ARTIST_31470_F_RECONSTRUCTED_ROUTE_V1",
    }
    target = {**projection, "identityRouteSha256": canonical_sha256(projection)}
    require_keys(target, TARGET_KEYS, "runtime program target")
    return target


def build_policy_route_binding(documents: dict[str, dict[str, Any]]) -> dict[str, Any]:
    approval = documents["approval"]
    source_capability = documents["sourceCapability"]
    material_policy = documents["materialPolicy"]
    row = {
        "approvalPolicyId": approval["policyId"],
        "approvalPolicyVersion": approval["policyVersion"],
        "approvalReceiptSha256": approval["receiptSha256"],
        "sourceExecutionFamilyCount": len(approval["sourceExecutionFamilies"]),
        "sourceExecutionFamilies": copy.deepcopy(approval["sourceExecutionFamilies"]),
        "sourceExecutionFamilyProjectionSha256": canonical_sha256(
            approval["sourceExecutionFamilies"]
        ),
        "sourcePolicyRowCount": len(approval["sourceRows"]),
        "sourcePolicyRowProjectionSha256": canonical_sha256(approval["sourceRows"]),
        "materialExecutionFamilyCount": len(approval["materialExecutionFamilies"]),
        "materialExecutionFamilies": copy.deepcopy(approval["materialExecutionFamilies"]),
        "materialExecutionFamilyProjectionSha256": canonical_sha256(
            approval["materialExecutionFamilies"]
        ),
        "materialPolicyRowCount": len(approval["materialRows"]),
        "materialPolicyRowProjectionSha256": canonical_sha256(approval["materialRows"]),
        "materialArithmeticRowCount": len(approval["materialArithmeticRows"]),
        "materialArithmeticRowProjectionSha256": canonical_sha256(
            approval["materialArithmeticRows"]
        ),
        "geometryPolicyRowCount": len(approval["geometryRows"]),
        "geometryPolicyRowProjectionSha256": canonical_sha256(approval["geometryRows"]),
        "sourceCapabilityReceiptSha256": source_capability["receiptSha256"],
        "sourceCapabilityCanonicalSha256": canonical_sha256(source_capability),
        "materialPolicyReceiptSha256": material_policy["receiptSha256"],
        "materialPolicyCanonicalSha256": canonical_sha256(material_policy),
    }
    require(row["sourceExecutionFamilyCount"] == 7
            and row["sourcePolicyRowCount"] == 29,
            "approval Source policy route denominator mismatch")
    require(row["materialExecutionFamilyCount"] == 8
            and row["materialPolicyRowCount"] == 255
            and row["materialArithmeticRowCount"] == 23,
            "approval Material policy route denominator mismatch")
    require(row["geometryPolicyRowCount"] == 7,
            "approval Geometry policy route denominator mismatch")
    row["bindingSha256"] = canonical_sha256(row)
    require_keys(row, POLICY_ROUTE_BINDING_KEYS, "policyRouteBinding")
    return row


def build_material_evaluator_oracle_contract(runtime: dict[str, Any]) -> dict[str, Any]:
    raw = runtime["evaluatorContract"]
    expected_operations = [
        "SECOND_TEXTURE_MULTIPLY", "UV_TRANSFORM_PHASE", "PANNER_PHASE",
        "COLOR_MULTIPLY", "DESATURATION", "SIGNED_POWER", "FRESNEL_GAIN",
        "DISTORTION_OFFSET", "DISSOLVE_ALPHA", "ALPHA_MULTIPLY",
    ]
    require(raw["version"] == 1 and raw["operationOrder"] == expected_operations,
            "Material evaluator operation contract mismatch")
    inputs: list[dict[str, Any]] = []
    for sample in raw["inputSamples"]:
        require_keys(sample, MATERIAL_EVALUATOR_INPUT_KEYS,
                     "Material evaluator shared input")
        copied = copy.deepcopy(sample)
        require(type(copied["time"]) is float,
                "Material evaluator input time must be JSON float")
        require_numeric_vector(copied["uvScale"], 2,
                               "Material evaluator shared uvScale")
        for key in (
            "panRotationAux", "texture0", "texture1", "color", "params0", "params1",
        ):
            require_numeric_vector(copied[key], 4,
                                   f"Material evaluator shared {key}")
        inputs.append(copied)
    require(len(inputs) == raw["inputSampleCountPerFamily"] == 4,
            "Material evaluator shared input denominator mismatch")
    contract = {
        "version": 1,
        "operationOrder": expected_operations,
        "inputSampleCountPerFamily": 4,
        "inputSamples": inputs,
        "numericTolerance": float(raw["numericTolerance"]),
        "fidelity": raw["fidelity"],
        "sourceExact": False,
    }
    require(contract["numericTolerance"] == 2.0e-5,
            "Material evaluator numeric tolerance mismatch")
    contract["contractSha256"] = canonical_sha256(contract)
    require_keys(contract, MATERIAL_EVALUATOR_ORACLE_CONTRACT_KEYS,
                 "Material evaluator oracle contract")
    return contract


def derive_program_blockers(
    sections: dict[str, list[dict[str, Any]]], documents: dict[str, dict[str, Any]],
) -> list[str]:
    blockers = set(derive_program_blockers_from_sections(sections))
    external_blockers: set[str] = set()
    external_blockers.update(documents["sourceCapability"].get("blockerUnion", []))
    external_blockers.update(documents["materialPolicy"]["admission"]["blockers"])
    external_blockers.update(documents["geometryBinding"].get("productBlockers", []))
    external_blockers.update(documents["geometryReceipt"].get("productBlockers", []))
    approval_policy = documents["approval"]["admissionPolicy"]
    external_blockers.update(
        token for token in approval_policy["executionBlockers"]
        if token != "R2_TYPED_MATERIALIZATION_NOT_COMPLETE"
    )
    external_blockers.update(approval_policy["productBlockers"])
    external_blockers.discard("")
    require(external_blockers.issubset(blockers),
            "external blocker authority is not preserved by program-owned rows/gates: "
            + ", ".join(sorted(external_blockers - blockers)))
    return sorted(blockers)


def section_digest(section_name: str, rows: list[dict[str, Any]]) -> dict[str, Any]:
    value = {
        "sectionName": section_name,
        "rowCount": len(rows),
        "orderedSha256": canonical_sha256([row["rowSha256"] for row in rows]),
    }
    require_keys(value, SECTION_DIGEST_KEYS, "section digest")
    return value


def build_program() -> dict[str, Any]:
    documents, input_rows = git_authority_bundle()
    approval = documents["approval"]
    target = build_target_identity(documents)
    require(approval["policyId"] == "artist.31470.f.reconstructed-approved-v1",
            "approval policy identity mismatch")
    require(approval["approvalDecision"]["classification"] == "RECONSTRUCTED_APPROVED_V1",
            "approval classification mismatch")
    require(approval["admissionPolicy"]["policyRouteApproved"] is True,
            "approval route is not open")
    require(approval["admissionPolicy"]["executionAdmission"] is False,
            "approval input unexpectedly grants runtime execution")
    require(approval["admissionPolicy"]["productAdmission"] is False,
            "approval input unexpectedly grants Product admission")

    registry = HandlerRegistry()
    source_sections = build_source_sections(
        documents["sourceSemantics"], documents["sourceCapability"],
        documents["localReferenceClosure"], documents["sourceCandidate"],
        documents["approval"], registry,
    )
    action_schedules = build_action_schedules(
        documents["actionCueRecipe"], documents["sourceSemantics"],
        source_sections["emitters"],
    )
    material_sections = build_material_sections(
        documents["materialRuntime"], documents["materialPolicy"],
        documents["approval"], source_sections["emitters"], registry,
        documents["materialTextureBinding"],
    )
    renderer_resource_sections = build_renderer_texture_resources(
        documents["sourceCandidate"], source_sections["emitters"],
        material_sections["materialOccurrences"], material_sections["materialRecipes"],
    )
    geometry_sections = build_geometry_sections(
        documents["geometryBinding"], documents["geometryReceipt"],
        documents["normalizedEffectGraph"], documents["sourceCandidate"],
        source_sections["modules"], source_sections["emitters"], documents["approval"],
        target["identityRouteSha256"],
    )
    source_sections["emitters"] = bind_emitter_direct_ids(
        source_sections["emitters"], material_sections["materialOccurrences"],
        material_sections["materialRecipes"],
        renderer_resource_sections["rendererTextureResources"],
        geometry_sections["geometryUses"], source_sections["modules"],
        source_sections["distributions"], source_sections["literals"],
        source_sections["implicitDefaults"], source_sections["pointLightFields"],
        action_schedules, documents["actionCueRecipe"],
    )
    sections: dict[str, list[dict[str, Any]]] = {
        "inputArtifacts": input_rows,
        "handlerRegistry": registry.rows,
        "emitters": source_sections.pop("emitters"),
        "actionSchedules": action_schedules,
        **source_sections,
        **material_sections,
        **renderer_resource_sections,
        **geometry_sections,
    }
    require(tuple(sections.keys()) == SECTION_NAMES, "builder section order drift")
    blockers = derive_program_blockers(sections, documents)
    admission = {
        "artifactBinding": True,
        "policyRoute": True,
        "sourceHandlerSelection": True,
        "distributionEvaluatorSelection": True,
        "materialPolicySelection": True,
        "geometryBinding": True,
        "sourceExact": False,
        "runtimeExecution": False,
        "product": False,
        "blockers": blockers,
    }
    require_keys(admission, ADMISSION_KEYS, "admission")
    distribution_counts = {
        variant: sum(row["variant"] == variant for row in sections["distributions"])
        for variant in ("INLINE", "FLOAT_PARAMETER", "VECTOR_PARAMETER", "FLOAT_CURVE", "EF_MULTIPLY")
    }
    policy_counts = {
        domain: sum(row["domain"] == domain for row in sections["materialPolicyRows"])
        for domain in ("RENDER_STATE", "STATIC_PERMUTATION", "SAMPLER_DESCRIPTOR")
    }
    summary = {
        "inputArtifactCount": len(sections["inputArtifacts"]),
        "handlerRegistryCount": len(sections["handlerRegistry"]),
        "emitterCount": len(sections["emitters"]),
        "actionScheduleCount": len(sections["actionSchedules"]),
        "moduleCount": len(sections["modules"]),
        "moduleSelectionCounts": {
            variant: sum(row["selectionKind"] == variant for row in sections["modules"])
            for variant in ("SOURCE_HANDLER", "RECONSTRUCTED_HANDLER")
        },
        "propertyCount": len(sections["properties"]),
        "primitiveLeafCount": len(sections["primitiveLeaves"]),
        "literalCount": len(sections["literals"]),
        "distributionCount": len(sections["distributions"]),
        "distributionVariantCounts": distribution_counts,
        "seedPolicyCount": len(sections["seedPolicies"]),
        "implicitDefaultCount": len(sections["implicitDefaults"]),
        "pointLightFieldCount": len(sections["pointLightFields"]),
        "materialFamilyCount": len(sections["materialFamilies"]),
        "materialRecipeCount": len(sections["materialRecipes"]),
        "materialInputCount": len(sections["materialInputs"]),
        "materialStaticBindingCount": len(sections["materialStaticBindings"]),
        "materialRenderBindingCount": len(sections["materialRenderBindings"]),
        "materialOccurrenceCount": len(sections["materialOccurrences"]),
        "materialPolicyCount": len(sections["materialPolicyRows"]),
        "materialPolicyDomainCounts": policy_counts,
        "materialD3dDescriptorCount": sum(
            row["d3dDescriptorOracle"] is not None
            for row in sections["materialPolicyRows"]
        ),
        "materialD3dDescriptorProjectionSha256": canonical_sha256([
            row["d3dDescriptorOracle"] for row in sections["materialPolicyRows"]
            if row["d3dDescriptorOracle"] is not None
        ]),
        "materialSrvDescriptorCount": sum(
            row["d3dSrvOracle"] is not None for row in sections["materialPolicyRows"]
        ),
        "materialSrvDescriptorProjectionSha256": canonical_sha256([
            row["d3dSrvOracle"] for row in sections["materialPolicyRows"]
            if row["d3dSrvOracle"] is not None
        ]),
        "materialTextureBindingCount": len(sections["materialTextureBindings"]),
        "resolvedMaterialTextureBindingCount": sum(
            row["resolutionStatus"] == "RESOLVED_EXACT_RUNTIME_ASSET"
            for row in sections["materialTextureBindings"]
        ),
        "rendererTextureResourceCount": len(sections["rendererTextureResources"]),
        "rendererTextureSlotCounts": {
            slot: sum(row["slotId"] == slot for row in sections["rendererTextureResources"])
            for slot in sorted(RENDERER_TEXTURE_SLOTS)
        },
        "geometryCarrierCount": len(sections["geometryCarriers"]),
        "geometryUseCount": len(sections["geometryUses"]),
        "unknownHandlerCount": 0,
        "ownerlessRowCount": 0,
        "duplicateIdCount": 0,
        "sourceExactRowCount": 0,
        "runtimeExecution": False,
        "product": False,
    }
    require_keys(summary, SUMMARY_KEYS, "summary")
    program: dict[str, Any] = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "programId": PROGRAM_ID,
        "programVersion": PROGRAM_VERSION,
        "target": target,
        "policyRouteBinding": build_policy_route_binding(documents),
        "particleSystemPolicy": build_particle_system_policy(documents["sourceCandidate"]),
        "materialEvaluatorOracleContract": build_material_evaluator_oracle_contract(
            documents["materialRuntime"]
        ),
        "programRole": PROGRAM_ROLE,
        **sections,
        "sectionDigests": [section_digest(name, sections[name]) for name in SECTION_NAMES],
        "blockerOwnership": build_blocker_ownership_contract(sections),
        "blockerUnion": blockers,
        "admission": admission,
        "summary": summary,
    }
    require(tuple(program.keys()) == ROOT_KEYS[:-1], "program root construction order drift")
    program["programSha256"] = canonical_sha256(program)
    return program


PRIMARY_ID_FIELDS = {
    "inputArtifacts": "artifactId",
    "handlerRegistry": "handlerRegistryId",
    "emitters": "emitterId",
    "actionSchedules": "scheduleId",
    "modules": "moduleId",
    "properties": "propertyId",
    "primitiveLeaves": "leafId",
    "literals": "literalId",
    "distributions": "distributionId",
    "seedPolicies": "seedPolicyId",
    "implicitDefaults": "defaultId",
    "pointLightFields": "fieldId",
    "materialFamilies": "familyId",
    "materialRecipes": "recipeId",
    "materialInputs": "fieldId",
    "materialStaticBindings": "fieldId",
    "materialRenderBindings": "renderBindingId",
    "materialOccurrences": "occurrenceId",
    "materialPolicyRows": "policyRowId",
    "materialTextureBindings": "bindingId",
    "rendererTextureResources": "textureResourceId",
    "geometryCarriers": "carrierId",
    "geometryUses": "geometryUseId",
}


EXPECTED_COUNTS = {
    "inputArtifacts": len(AUTHORITIES),
    "emitters": 35,
    "actionSchedules": 7,
    "modules": 399,
    "properties": 1434,
    "primitiveLeaves": 1572,
    "literals": 1590,
    "distributions": 629,
    "seedPolicies": 14,
    "implicitDefaults": 14,
    "pointLightFields": 8,
    "materialFamilies": 23,
    "materialRecipes": 27,
    "materialInputs": 729,
    "materialStaticBindings": 94,
    "materialRenderBindings": 162,
    "materialOccurrences": 34,
    "materialPolicyRows": 255,
    "materialTextureBindings": 72,
    "rendererTextureResources": 57,
    "geometryCarriers": 7,
    "geometryUses": 13,
}


def require_numeric_vector(value: Any, length: int, label: str) -> None:
    require(type(value) is list and len(value) == length, f"{label}: expected float{length}")
    for item in value:
        require(type(item) in (int, float) and type(item) is not bool,
                f"{label}: vector component must be numeric")
        require(math.isfinite(float(item)), f"{label}: non-finite vector component")


def require_json_float(value: Any, label: str) -> None:
    require(type(value) is float and math.isfinite(value),
            f"{label}: expected finite JSON float")


def require_json_int(value: Any, label: str, minimum: int | None = None) -> None:
    require(type(value) is int, f"{label}: expected JSON integer")
    if minimum is not None:
        require(value >= minimum, f"{label}: integer is below minimum")


def require_sha256(value: Any, label: str, allow_empty: bool = False) -> None:
    if allow_empty and value == "":
        return
    require(type(value) is str and len(value) == 64
            and all(ch in "0123456789abcdef" for ch in value),
            f"{label}: expected lowercase SHA-256")


def require_string_list(value: Any, label: str, unique: bool = False) -> None:
    require(type(value) is list and all(type(item) is str for item in value),
            f"{label}: expected string array")
    if unique:
        require(len(value) == len(set(value)), f"{label}: duplicate string")


def require_self_digest(value: dict[str, Any], key: str, label: str) -> None:
    require_sha256(value.get(key), f"{label} {key}")
    unsigned = copy.deepcopy(value)
    expected = unsigned.pop(key)
    require(canonical_sha256(unsigned) == expected, f"{label}: self digest mismatch")


def validate_action_cue_row(value: dict[str, Any], label: str) -> None:
    require_keys(value, ACTION_CUE_KEYS, label)
    require(type(value["name"]) is str and value["name"], f"{label}: name missing")
    require(value["kind"] in {"SCALAR", "VECTOR"}, f"{label}: invalid kind")
    require_json_int(value["sourceIndex"], f"{label} sourceIndex")
    require_json_int(value["sourceValueByteOffset"], f"{label} sourceValueByteOffset")
    if value["kind"] == "SCALAR":
        require_json_float(value["scalarValue"], f"{label} scalarValue")
        require(value["vectorValue"] == [], f"{label}: scalar carries vector")
    else:
        require(value["scalarValue"] is None, f"{label}: vector carries scalar")
        require(type(value["vectorValue"]) is list
                and len(value["vectorValue"]) in (3, 4),
                f"{label}: vector arity mismatch")
        for item in value["vectorValue"]:
            require_json_float(item, f"{label} vectorValue")


def validate_sampler_descriptor(value: dict[str, Any], label: str) -> None:
    require_keys(value, SAMPLER_DESCRIPTOR_KEYS, label)
    require(value["type"] == "D3D11_SAMPLER_DESC_AND_SRV_COLOR_SPACE",
            f"{label}: descriptor type mismatch")
    for key in (
        "filterUe3", "addressUUe3", "addressVUe3", "addressWUe3",
        "comparisonFuncName", "srvColorSpace", "lodGroup",
    ):
        require(type(value[key]) is str and value[key], f"{label}: {key} missing")
    for key in (
        "filterD3d11", "addressUD3d11", "addressVD3d11", "addressWD3d11",
        "maxAnisotropy", "comparisonFuncD3d11",
    ):
        require_json_int(value[key], f"{label} {key}", 0)
    for key in ("mipLODBias", "minLOD", "maxLOD"):
        require_json_float(value[key], f"{label} {key}")
    require_numeric_vector(value["borderColor"], 4, f"{label} borderColor")
    require(all(type(item) is float for item in value["borderColor"]),
            f"{label}: borderColor must contain JSON floats")
    require(type(value["sRgb"]) is bool, f"{label}: sRgb must be bool")
    require(value["srvColorSpace"] in {"LINEAR", "SRGB"},
            f"{label}: invalid SRV color space")


def validate_material_input_sample(value: dict[str, Any], label: str) -> None:
    require_keys(value, MATERIAL_EVALUATOR_INPUT_KEYS, label)
    require(type(value["sampleId"]) is str and value["sampleId"],
            f"{label}: sampleId missing")
    require_json_float(value["time"], f"{label} time")
    require_numeric_vector(value["uvScale"], 2, f"{label} uvScale")
    for key in (
        "panRotationAux", "texture0", "texture1", "color", "params0", "params1",
    ):
        require_numeric_vector(value[key], 4, f"{label} {key}")
    for key in MATERIAL_EVALUATOR_INPUT_KEYS[2:]:
        require(all(type(item) is float for item in value[key]),
                f"{label}: {key} must contain JSON floats")


def validate_material_evaluator_oracle_contract(program: dict[str, Any]) -> None:
    contract = program["materialEvaluatorOracleContract"]
    require_keys(contract, MATERIAL_EVALUATOR_ORACLE_CONTRACT_KEYS,
                 "materialEvaluatorOracleContract")
    require_json_int(contract["version"], "Material evaluator version")
    require(contract["version"] == 1, "Material evaluator version mismatch")
    require(contract["operationOrder"] == [
        "SECOND_TEXTURE_MULTIPLY", "UV_TRANSFORM_PHASE", "PANNER_PHASE",
        "COLOR_MULTIPLY", "DESATURATION", "SIGNED_POWER", "FRESNEL_GAIN",
        "DISTORTION_OFFSET", "DISSOLVE_ALPHA", "ALPHA_MULTIPLY",
    ], "Material evaluator operation order mismatch")
    require_json_int(contract["inputSampleCountPerFamily"],
                     "Material evaluator input sample count")
    require(contract["inputSampleCountPerFamily"] == 4
            and type(contract["inputSamples"]) is list
            and len(contract["inputSamples"]) == 4,
            "Material evaluator input denominator mismatch")
    for index, sample in enumerate(contract["inputSamples"]):
        validate_material_input_sample(sample, f"Material evaluator input {index}")
        require(sample["sampleId"] == f"sample-{index}",
                "Material evaluator sample order/identity mismatch")
    require_json_float(contract["numericTolerance"], "Material evaluator tolerance")
    require(contract["numericTolerance"] == 2.0e-5
            and contract["fidelity"] == "RECONSTRUCTED_NUMERICALLY_VERIFIED"
            and contract["sourceExact"] is False,
            "Material evaluator fidelity/tolerance mismatch")
    require_self_digest(contract, "contractSha256", "Material evaluator contract")


def validate_material_numeric_samples(program: dict[str, Any]) -> None:
    shared_inputs = program["materialEvaluatorOracleContract"]["inputSamples"]
    family_projection: list[dict[str, Any]] = []
    for family in program["materialFamilies"]:
        samples = family["numericSamples"]
        require(type(samples) is list and len(samples) == 4,
                "Material family numeric sample denominator mismatch")
        for order, sample in enumerate(samples):
            require_keys(sample, MATERIAL_FAMILY_SAMPLE_KEYS,
                         "Material family numeric sample")
            require(sample["sampleId"] == f"sample-{order}"
                    and type(sample["order"]) is int and sample["order"] == order,
                    "Material family numeric sample identity/order mismatch")
            require(sample["inputSha256"] == canonical_sha256(shared_inputs[order]),
                    "Material family sample input binding mismatch")
            require(type(sample["expectedFloat4"]) is list
                    and len(sample["expectedFloat4"]) == 4
                    and all(type(value) is float and math.isfinite(value)
                            for value in sample["expectedFloat4"]),
                    "Material family expectedFloat4 must contain JSON floats")
        require_sha256(family["sampleProjectionSha256"],
                       "Material family upstream sample projection")
        family_projection.append({
            "familyId": family["familyId"],
            "numericSamples": samples,
        })
    require(len(family_projection) == 23
            and canonical_sha256(family_projection)
            == EXPECTED_MATERIAL_FAMILY_SAMPLES_SHA256,
            "Material family numeric oracle projection mismatch")

    recipe_projection: list[dict[str, Any]] = []
    for recipe in program["materialRecipes"]:
        samples = recipe["numericBindingSamples"]
        require(type(samples) is list and len(samples) == 4,
                "Material recipe numeric sample denominator mismatch")
        for order, sample in enumerate(samples):
            require_keys(sample, MATERIAL_RECIPE_SAMPLE_KEYS,
                         "Material recipe numeric sample")
            require(sample["sampleId"] == f"sample-{order}"
                    and type(sample["order"]) is int and sample["order"] == order,
                    "Material recipe numeric sample identity/order mismatch")
            typed_input = {key: sample[key] for key in MATERIAL_EVALUATOR_INPUT_KEYS}
            validate_material_input_sample(
                typed_input, f"Material recipe {recipe['recipeId']} sample {order}"
            )
            require(sample["inputSha256"] == canonical_sha256(typed_input),
                    "Material recipe sample input digest mismatch")
            require(type(sample["expectedFloat4"]) is list
                    and len(sample["expectedFloat4"]) == 4
                    and all(type(value) is float and math.isfinite(value)
                            for value in sample["expectedFloat4"]),
                    "Material recipe expectedFloat4 must contain JSON floats")
        recipe_projection.append({
            "recipeId": recipe["recipeId"],
            "numericBindingSamples": samples,
        })
    require(len(recipe_projection) == 27
            and canonical_sha256(recipe_projection)
            == EXPECTED_MATERIAL_RECIPE_SAMPLES_SHA256,
            "Material recipe numeric oracle projection mismatch")


def validate_policy_route_binding(program: dict[str, Any]) -> None:
    require(canonical_json_bytes(program["inputArtifacts"])
            == expected_input_artifacts_canonical(),
            "frozen 13-input artifact table mismatch")
    artifacts = {row["artifactId"]: row for row in program["inputArtifacts"]}
    require(len(artifacts) == len(program["inputArtifacts"]),
            "duplicate input artifact identity")
    exact_artifacts = {
        "approval": (
            APPROVAL_COMMIT, APPROVAL_PATH, EXPECTED_APPROVAL_CANONICAL_SHA256,
            EXPECTED_APPROVAL_RECEIPT_SHA256,
        ),
        "sourceCapability": (
            SOURCE_CAPABILITY_COMMIT, SOURCE_CAPABILITY_PATH,
            EXPECTED_SOURCE_CAPABILITY_CANONICAL_SHA256,
            EXPECTED_SOURCE_CAPABILITY_RECEIPT_SHA256,
        ),
        "materialPolicy": (
            MATERIAL_POLICY_COMMIT, MATERIAL_POLICY_PATH,
            EXPECTED_MATERIAL_POLICY_CANONICAL_SHA256,
            EXPECTED_MATERIAL_POLICY_RECEIPT_SHA256,
        ),
        "materialTextureBinding": (
            MATERIAL_TEXTURE_BINDING_COMMIT, MATERIAL_TEXTURE_BINDING_PATH,
            EXPECTED_MATERIAL_TEXTURE_BINDING_CANONICAL_SHA256,
            EXPECTED_MATERIAL_TEXTURE_BINDING_RECEIPT_SHA256,
        ),
    }
    for artifact_id, (commit, path, canonical_sha, self_sha) in exact_artifacts.items():
        artifact = artifacts.get(artifact_id)
        require(artifact is not None
                and artifact["authorityCommitId"] == commit
                and artifact["path"] == path
                and artifact["canonicalJsonSha256"] == canonical_sha
                and artifact["selfSha256"] == self_sha,
                f"{artifact_id} frozen input artifact identity mismatch")
    texture_artifact = artifacts["materialTextureBinding"]
    require(texture_artifact["authorityTreeId"]
            == EXPECTED_MATERIAL_TEXTURE_BINDING_TREE_ID
            and texture_artifact["blobId"]
            == EXPECTED_MATERIAL_TEXTURE_BINDING_BLOB_ID,
            "Material texture binding Git tree/blob identity mismatch")

    route = program["policyRouteBinding"]
    require(route["approvalReceiptSha256"] == EXPECTED_APPROVAL_RECEIPT_SHA256
            and route["sourceExecutionFamilyProjectionSha256"]
            == EXPECTED_APPROVAL_SOURCE_FAMILIES_SHA256
            and route["sourcePolicyRowProjectionSha256"]
            == EXPECTED_APPROVAL_SOURCE_ROWS_SHA256
            and route["materialExecutionFamilyProjectionSha256"]
            == EXPECTED_APPROVAL_MATERIAL_FAMILIES_SHA256
            and route["materialPolicyRowProjectionSha256"]
            == EXPECTED_APPROVAL_MATERIAL_ROWS_SHA256
            and route["materialArithmeticRowProjectionSha256"]
            == EXPECTED_APPROVAL_ARITHMETIC_ROWS_SHA256
            and route["geometryPolicyRowProjectionSha256"]
            == EXPECTED_APPROVAL_GEOMETRY_ROWS_SHA256
            and route["sourceCapabilityReceiptSha256"]
            == EXPECTED_SOURCE_CAPABILITY_RECEIPT_SHA256
            and route["sourceCapabilityCanonicalSha256"]
            == EXPECTED_SOURCE_CAPABILITY_CANONICAL_SHA256
            and route["materialPolicyReceiptSha256"]
            == EXPECTED_MATERIAL_POLICY_RECEIPT_SHA256
            and route["materialPolicyCanonicalSha256"]
            == EXPECTED_MATERIAL_POLICY_CANONICAL_SHA256,
            "policy route frozen identity mismatch")
    require(canonical_sha256(route["sourceExecutionFamilies"])
            == EXPECTED_APPROVAL_SOURCE_FAMILIES_SHA256
            and [row["moduleOccurrenceCount"]
                 for row in route["sourceExecutionFamilies"]]
            == [11, 3, 5, 3, 2, 1, 4],
            "approval Source family projection mismatch")
    require(canonical_sha256(route["materialExecutionFamilies"])
            == EXPECTED_APPROVAL_MATERIAL_FAMILIES_SHA256,
            "approval Material family projection mismatch")

    reconstructed = sorted(
        (row for row in program["modules"]
         if row["selectionKind"] == "RECONSTRUCTED_HANDLER"),
        key=lambda row: row["approvalSourceOrder"],
    )
    require([row["approvalSourceOrder"] for row in reconstructed] == list(range(29)),
            "approval Source row order mismatch")
    source_rows = [row["approvalSourceRow"] for row in reconstructed]
    require(canonical_sha256(source_rows) == EXPECTED_APPROVAL_SOURCE_ROWS_SHA256,
            "approval Source row frozen projection mismatch")

    material_rows = sorted(
        program["materialPolicyRows"], key=lambda row: row["approvalMaterialOrder"]
    )
    require([row["approvalMaterialOrder"] for row in material_rows] == list(range(255)),
            "approval Material row order mismatch")
    for row in material_rows:
        embedded = row["approvalMaterialRow"]
        require(type(embedded) is dict,
                "approval Material embedded row missing")
        require_keys(embedded, APPROVAL_MATERIAL_ROW_KEYS, "approval Material row")
        require(row["approvalMaterialRowSha256"] == canonical_sha256(embedded)
                and embedded["policyRowId"] == row["approvalPolicyRowId"]
                and embedded["upstreamIdentitySha256"]
                == row["approvalUpstreamIdentitySha256"]
                and embedded["policyBindingSha256"]
                == row["approvalPolicyBindingSha256"]
                and embedded["policyFamilyId"] == row["approvalPolicyFamilyId"]
                and embedded["requiredOracleIds"] == row["approvalRequiredOracleIds"]
                and embedded["executionBlockers"] == row["approvalExecutionBlockers"],
                f"approval Material row {row['policyRowId']} projection mismatch")
    require(canonical_sha256([row["approvalMaterialRow"] for row in material_rows])
            == EXPECTED_APPROVAL_MATERIAL_ROWS_SHA256,
            "approval Material row frozen projection mismatch")

    arithmetic_rows = sorted(
        program["materialFamilies"], key=lambda row: row["approvalArithmeticOrder"]
    )
    require([row["approvalArithmeticOrder"] for row in arithmetic_rows]
            == list(range(23)), "approval arithmetic row order mismatch")
    for row in arithmetic_rows:
        embedded = row["approvalArithmeticRow"]
        require(type(embedded) is dict,
                "approval arithmetic embedded row missing")
        require_keys(embedded, APPROVAL_ARITHMETIC_ROW_KEYS, "approval arithmetic row")
        require(row["approvalArithmeticRowSha256"] == canonical_sha256(embedded)
                and embedded["policyRowId"] == row["approvalPolicyRowId"]
                and embedded["upstreamFamilyId"] == row["familyId"]
                and embedded["familyIdentitySha256"] == row["familyIdentitySha256"]
                and embedded["evaluatorId"] == row["evaluatorId"]
                and embedded["evaluatorVersion"] == row["evaluatorVersion"]
                and embedded["evaluatorSha256"] == row["evaluatorSha256"]
                and embedded["policyFamilyId"] == row["approvalPolicyFamilyId"]
                and embedded["requiredOracleIds"] == row["approvalRequiredOracleIds"]
                and embedded["executionBlockers"] == row["approvalExecutionBlockers"],
                f"approval arithmetic row {row['familyId']} projection mismatch")
    require(canonical_sha256([row["approvalArithmeticRow"] for row in arithmetic_rows])
            == EXPECTED_APPROVAL_ARITHMETIC_ROWS_SHA256,
            "approval arithmetic row frozen projection mismatch")

    geometry_rows = sorted(
        program["geometryCarriers"], key=lambda row: row["approvalGeometryOrder"]
    )
    require([row["approvalGeometryOrder"] for row in geometry_rows] == list(range(7)),
            "approval Geometry row order mismatch")
    for row in geometry_rows:
        embedded = row["approvalGeometryRow"]
        require(type(embedded) is dict,
                "approval Geometry embedded row missing")
        require_keys(embedded, APPROVAL_GEOMETRY_ROW_KEYS, "approval Geometry row")
        require(row["approvalGeometryRowSha256"] == canonical_sha256(embedded)
                and embedded["assetId"] == row["assetId"]
                and embedded["sourceObject"] == row["sourceObject"]
                and embedded["candidateResourceSha256"]
                == row["candidateResourceSha256"]
                and embedded["payloadSha256"] == row["payloadSha256"]
                and embedded["metadataIdentitySha256"]
                == row["metadataIdentitySha256"]
                and embedded["geometryPreScale"] == row["geometryPreScale"],
                f"approval Geometry row {row['carrierId']} projection mismatch")
    require(canonical_sha256([row["approvalGeometryRow"] for row in geometry_rows])
            == EXPECTED_APPROVAL_GEOMETRY_ROWS_SHA256,
            "approval Geometry row frozen projection mismatch")


def require_lower_hex(value: Any, length: int, label: str) -> None:
    require(type(value) is str and len(value) == length
            and all(character in "0123456789abcdef" for character in value),
            f"{label}: expected {length}-character lowercase hex")


def validate_geometry_nested(program: dict[str, Any]) -> None:
    carrier_projection: list[dict[str, Any]] = []
    for carrier in program["geometryCarriers"]:
        require(carrier["formatVersion"] == "1.1",
                "Geometry carrier formatVersion mismatch")
        require_json_int(carrier["order"], "Geometry carrier order", 0)
        require_json_int(carrier["candidateResourceByteSize"],
                         "Geometry candidate byte size", 1)
        require_json_int(carrier["channelMask"], "Geometry channel mask", 0)
        require_json_int(carrier["evidenceFlags"], "Geometry evidence flags", 0)
        require_json_float(carrier["geometryPreScale"], "Geometry pre-scale")
        require(carrier["geometryPreScale"] == 0.01
                and carrier["preScaleConsumed"] is False
                and carrier["sourceExact"] is False,
                "Geometry pre-scale/source boundary mismatch")
        require_lower_hex(carrier["geometryPreScaleF32Hex"], 8,
                          "Geometry pre-scale f32 hex")
        require(carrier["geometryPreScaleF32Hex"] == "3c23d70a",
                "Geometry pre-scale f32 identity mismatch")
        for key in (
            "candidateResourceSha256", "payloadSha256", "provenanceSha256",
            "metadataIdentitySha256", "cacheIdentitySha256", "expectedTupleSha256",
            "approvalGeometryRowSha256", "preparedCacheIdentitySha256",
        ):
            require_sha256(carrier[key], f"Geometry carrier {key}")
        require(carrier["provenanceRole"] == "WMODEL_METADATA_IDENTITY_SHA256"
                and carrier["metadataIdentitySha256"] == carrier["provenanceSha256"],
                "Geometry provenance identity mismatch")
        require(type(carrier["submeshes"]) is list and carrier["submeshes"],
                "Geometry carrier must own submeshes")
        for submesh in carrier["submeshes"]:
            require_keys(submesh, GEOMETRY_SUBMESH_KEYS, "geometry submesh")
            require(type(submesh["name"]) is str and submesh["name"],
                    "Geometry submesh name missing")
            require_json_int(submesh["materialIndex"],
                             "Geometry submesh material index", 0)
            require_json_int(submesh["vertexCount"],
                             "Geometry submesh vertex count", 1)
            require_json_int(submesh["indexCount"],
                             "Geometry submesh index count", 1)
            require_keys(submesh["channelCounts"], GEOMETRY_CHANNEL_COUNT_KEYS,
                         "geometry channel counts")
            for key, count in submesh["channelCounts"].items():
                require_json_int(count, f"Geometry channel count {key}", 0)
            require(all(submesh["channelCounts"][key] == submesh["vertexCount"]
                        for key in GEOMETRY_CHANNEL_COUNT_KEYS if key != "color0")
                    and submesh["channelCounts"]["color0"]
                    in {0, submesh["vertexCount"]},
                    "Geometry vertex channel count mismatch")
            require_keys(submesh["channelSha256"], GEOMETRY_CHANNEL_SHA_KEYS,
                         "geometry channel SHA")
            for key, digest in submesh["channelSha256"].items():
                if key == "color0" and submesh["channelCounts"]["color0"] == 0:
                    require(digest is None, "absent Geometry COLOR0 carries digest")
                else:
                    require_sha256(digest, f"Geometry channel SHA {key}")
            bounds_words = submesh["boundsF32Hex"]
            require(type(bounds_words) is list and len(bounds_words) == 10,
                    "geometry bounds must contain 10 f32 words")
            decoded_bounds: list[float] = []
            for word in bounds_words:
                require_lower_hex(word, 8, "Geometry bounds f32 word")
                decoded = struct.unpack(">f", bytes.fromhex(word))[0]
                require(math.isfinite(decoded), "Geometry bounds contain non-finite f32")
                decoded_bounds.append(decoded)
            require(all(decoded_bounds[index] <= decoded_bounds[index + 3]
                        for index in range(3))
                    and decoded_bounds[9] >= 0.0,
                    "Geometry bounds min/max/radius invalid")
        carrier_projection.append({
            key: value for key, value in carrier.items() if key != "rowSha256"
        })
    require(canonical_sha256(carrier_projection) == EXPECTED_GEOMETRY_CARRIERS_SHA256,
            "Geometry frozen carrier projection mismatch")


def validate_renderer_runtime_config(emitter: dict[str, Any]) -> None:
    config = emitter["rendererRuntimeConfig"]
    require_keys(config, RENDERER_RUNTIME_CONFIG_KEYS, "renderer runtime diagnostic")
    require(config["fidelity"] == "RECONSTRUCTED_SOURCE_CANDIDATE_PROJECTION"
            and config["sourceExact"] is False
            and config["consumptionPolicy"]
            == "DIAGNOSTIC_ONLY_FORBIDDEN_CONSUMPTION"
            and config["blockers"]
            == ["SOURCE_CANDIDATE_PRESENTATION_VALUES_NOT_APPROVAL_ROUTED"],
            "renderer diagnostic consumption boundary mismatch")
    nested_keys = (
        "color", "uv", "linearLerp", "mesh", "sprite", "decal", "trail",
        "afterImage", "screenPost",
    )
    projection = {
        "sourceNode": emitter["sourceNode"],
        "rendererType": emitter["rendererType"],
        **{key: config[key] for key in nested_keys},
    }
    require(config["sourceProjectionSha256"] == canonical_sha256(projection),
            "renderer diagnostic source projection mismatch")
    if emitter["rendererType"] == "LightParticle":
        require(all(config[key] is None for key in nested_keys),
                "Light renderer diagnostic must be empty")
        return
    color = config["color"]
    require_keys(color, RENDERER_COLOR_KEYS, "renderer diagnostic color")
    for key in ("offset", "multiply"):
        require_numeric_vector(color[key], 4, f"renderer color {key}")
        require(all(type(item) is float for item in color[key]),
                f"renderer color {key} must contain JSON floats")
    for key in (
        "clip", "emissiveIntensity", "distortionIntensity", "radialTime",
        "radialIntensity",
    ):
        require_json_float(color[key], f"renderer color {key}")
    require(type(color["distortionOnBaseMaterial"]) is bool,
            "renderer color distortionOnBaseMaterial must be bool")
    uv = config["uv"]
    require_keys(uv, RENDERER_UV_KEYS, "renderer diagnostic UV")
    for key in ("start", "speed", "waveAmplitude"):
        require_numeric_vector(uv[key], 2, f"renderer UV {key}")
        require(all(type(item) is float for item in uv[key]),
                f"renderer UV {key} must contain JSON floats")
    for key in ("wave", "sequence", "loop"):
        require(type(uv[key]) is bool, f"renderer UV {key} must be bool")
    for key in ("waveFrequency", "sequenceTerm"):
        require_json_float(uv[key], f"renderer UV {key}")
    for key in ("tileColumns", "tileRows", "tileIndex"):
        require_json_int(uv[key], f"renderer UV {key}", 0)
    linear = config["linearLerp"]
    require_keys(linear, RENDERER_LINEAR_LERP_KEYS, "renderer diagnostic linear lerp")
    bool_keys = (
        "position", "rotation", "revolution", "scale", "velocity", "colorOffset",
        "colorMultiply", "emissiveIntensity",
    )
    for key in bool_keys:
        require(type(linear[key]) is bool, f"renderer linear {key} must be bool")
    for key, length in (
        ("endPosition", 3), ("endRotationDegrees", 3),
        ("endRevolutionDegreesPerSecond", 3), ("endScale", 3),
        ("endVelocityPerSecond", 3), ("endColorOffset", 4),
        ("endColorMultiply", 4),
    ):
        require_numeric_vector(linear[key], length, f"renderer linear {key}")
        require(all(type(item) is float for item in linear[key]),
                f"renderer linear {key} must contain JSON floats")
    require_json_float(linear["endEmissiveIntensity"],
                       "renderer linear endEmissiveIntensity")
    mesh = config["mesh"]
    if emitter["rendererType"] == "MeshParticle":
        require_keys(mesh, RENDERER_MESH_KEYS, "renderer diagnostic mesh")
        require(type(mesh["useModelMaterial"]) is bool,
                "renderer mesh useModelMaterial must be bool")
    else:
        require(mesh is None, "non-Mesh renderer carries mesh diagnostic")
    sprite = config["sprite"]
    if emitter["rendererType"] == "SpriteParticle":
        require_keys(sprite, RENDERER_SPRITE_KEYS, "renderer diagnostic sprite")
        require(type(sprite["billboard"]) is bool,
                "renderer sprite billboard must be bool")
        require_json_float(sprite["billboardRollDegrees"],
                           "renderer sprite billboard roll")
    else:
        require(sprite is None, "non-Sprite renderer carries sprite diagnostic")
    decal = config["decal"]
    if emitter["rendererType"] == "DecalParticle":
        require_keys(decal, RENDERER_DECAL_KEYS, "renderer diagnostic decal")
        require_numeric_vector(decal["size"], 2, "renderer decal size")
        require(all(type(item) is float for item in decal["size"]),
                "renderer decal size must contain JSON floats")
        require_json_float(decal["depth"], "renderer decal depth")
    else:
        require(decal is None, "non-Decal renderer carries decal diagnostic")
    trail = config["trail"]
    if emitter["rendererType"] == "CascadeRibbon":
        require_keys(trail, RENDERER_TRAIL_KEYS, "renderer diagnostic trail")
        require_json_int(trail["maxPoints"], "renderer trail maxPoints", 0)
        for key in (
            "pointLifeTimeSeconds", "sampleIntervalSeconds", "minimumDistance",
            "startWidth", "endWidth",
        ):
            require_json_float(trail[key], f"renderer trail {key}")
        require(type(trail["faceCamera"]) is bool,
                "renderer trail faceCamera must be bool")
    else:
        require(trail is None, "non-Ribbon renderer carries trail diagnostic")
    after = config["afterImage"]
    require_keys(after, RENDERER_AFTER_IMAGE_KEYS, "renderer diagnostic after-image")
    require_json_float(after["sampleIntervalSeconds"], "after-image sample interval")
    require_json_int(after["maxCopies"], "after-image max copies", 0)
    require_json_float(after["alphaExponent"], "after-image alpha exponent")
    screen = config["screenPost"]
    if emitter["rendererType"] == "ScreenPost":
        require_keys(screen, RENDERER_SCREEN_POST_KEYS, "renderer diagnostic ScreenPost")
        require(type(screen["enabled"]) is bool, "ScreenPost enabled must be bool")
        require(type(screen["profileId"]) is str and type(screen["status"]) is str,
                "ScreenPost diagnostic strings invalid")
        for key in ("intensity", "secondaryIntensity", "frequency"):
            require_json_float(screen[key], f"ScreenPost {key}")
        require_numeric_vector(screen["tint"], 4, "ScreenPost tint")
        require(all(type(item) is float for item in screen["tint"]),
                "ScreenPost tint must contain JSON floats")
        require_json_int(screen["randomSeed"], "ScreenPost randomSeed", 0)
    else:
        require(screen is None, "non-ScreenPost renderer carries ScreenPost diagnostic")


def validate_adapter_self(value: dict[str, Any], keys: tuple[str, ...], label: str) -> None:
    require_keys(value, keys, label)
    require_self_digest(value, "adapterSha256", label)
    require(type(value["enabled"]) is bool and value["sourceExact"] is False,
            f"{label}: enabled/sourceExact type mismatch")
    require_string_list(value["blockers"], f"{label} blockers", unique=True)


def validate_nested_shapes(program: dict[str, Any]) -> None:
    policy = program["particleSystemPolicy"]
    require_keys(policy, PARTICLE_SYSTEM_POLICY_KEYS, "particleSystemPolicy")
    unsigned_policy = dict(policy)
    policy_sha = unsigned_policy.pop("policySha256")
    require(canonical_sha256(unsigned_policy) == policy_sha,
            "particleSystemPolicy digest mismatch")
    for emitter in program["emitters"]:
        require_keys(emitter["timing"], EMITTER_TIMING_KEYS, "emitter timing")
        require_keys(emitter["random"], EMITTER_RANDOM_KEYS, "emitter random")
        require_keys(emitter["cueLocalTransform"], CUE_TRANSFORM_KEYS, "cue transform")
        require_keys(emitter["detailTransform"], DETAIL_TRANSFORM_KEYS, "detail transform")
        require_keys(emitter["actionCueAttachment"], ATTACHMENT_KEYS, "attachment")
        require_keys(
            emitter["actionCueAttachment"]["socketLocalTransform"],
            SOCKET_TRANSFORM_KEYS, "attachment socket transform",
        )
        require_keys(
            emitter["transformInheritance"], TRANSFORM_INHERITANCE_KEYS,
            "transform inheritance",
        )
        require(emitter["sourceRecipeEnabled"] is True, "source recipe must be enabled")
        require(type(emitter["localSpace"]) is bool, "emitter localSpace must be bool")
        require(emitter["spawnRateFallbackPolicy"] == "FORBIDDEN_TYPED_OPCODE_REQUIRED",
                "spawn fallback must be forbidden")
        require(emitter["lifetimeFallbackPolicy"] == "FORBIDDEN_TYPED_OPCODE_REQUIRED",
                "lifetime fallback must be forbidden")
        require(type(emitter["operationalMaxParticles"]) is int
                and emitter["operationalMaxParticles"] > 0,
                "operational max particles must be positive int")
        require(type(emitter["sourcePeakActiveParticles"]) is int
                and emitter["sourcePeakActiveParticles"] > 0,
                "source peak must be positive int")
        require(emitter["operationalMaxParticles"] == emitter["sourcePeakActiveParticles"]
                and emitter["operationalCapPolicy"] == "SOURCE_LOD_PEAK_ACTIVE_PARTICLES_V1",
                "emitter operational cap policy mismatch")
        cap_projection = {
            "emitterId": emitter["emitterId"],
            "sourcePeakActiveParticles": emitter["sourcePeakActiveParticles"],
            "operationalMaxParticles": emitter["operationalMaxParticles"],
            "policy": emitter["operationalCapPolicy"],
        }
        require(emitter["operationalCapProjectionSha256"]
                == canonical_sha256(cap_projection)
                and emitter["operationalCapBlockers"]
                == ["R3_OPERATIONAL_PARTICLE_CAP_NOT_EXECUTED"],
                "emitter operational cap projection mismatch")
        require(type(emitter["random"]["emitterRandomSeed"]) is int
                and 0 <= emitter["random"]["emitterRandomSeed"] <= 0xFFFFFFFF,
                "emitter random seed must be u32")
        require(emitter["random"]["policyId"]
                == "DETERMINISTIC_OCCURRENCE_RNG_FROM_SOURCE_CANDIDATE_V1"
                and emitter["random"]["sourceExact"] is False
                and emitter["random"]["blockers"]
                == ["R3_OCCURRENCE_RANDOM_STREAM_NOT_EXECUTED"],
                "emitter random policy mismatch")
        require_self_digest(emitter["random"], "policySha256", "emitter random")
        require_sha256(emitter["random"]["seedDerivationInputSha256"],
                       "emitter random derivation input")
        require(emitter["random"]["emitterRandomSeed"]
                == int(emitter["random"]["seedDerivationInputSha256"][:8], 16),
                "emitter random seed derivation mismatch")
        timing = emitter["timing"]
        for key in ("requiredModuleId", "spawnModuleId", "lifetimeModuleId"):
            require(type(timing[key]) is str and timing[key],
                    f"emitter timing {key} missing")
        for key in ("emitterDelaySeconds", "emitterDurationSeconds"):
            require_json_float(timing[key], f"emitter timing {key}")
            require(timing[key] >= 0.0, f"emitter timing {key} negative")
        require(timing["emitterDelayPolicy"] in {
            "EXPLICIT_REQUIRED_LITERAL", "RECONSTRUCTED_UE3_ZERO_DISTRIBUTION_DEFAULT_V1",
        } and timing["emitterDurationPolicy"] in {
            "EXPLICIT_REQUIRED_LITERAL", "CURRENT_REVISION_CDO_RECONSTRUCTED_DEFAULT_V1",
        }, "emitter timing policy invalid")
        require_json_int(timing["emitterLoopCount"], "emitter loop count", 0)
        require(timing["fidelity"]
                == "RECONSTRUCTED_TYPED_SOURCE_AND_CURRENT_CDO_DEFAULTS_V1"
                and timing["sourceExact"] is False,
                "emitter timing fidelity mismatch")
        timing_core = {key: timing[key] for key in EMITTER_TIMING_KEYS[:9]}
        require(timing["sourceProjectionSha256"] == canonical_sha256(timing_core),
                "emitter timing source projection mismatch")
        require_self_digest(timing, "timingSha256", "emitter timing")
        for burst in emitter["timing"]["bursts"]:
            require_keys(burst, BURST_KEYS, "emitter burst")
            require(type(burst["spawnModuleId"]) is str and burst["spawnModuleId"],
                    "emitter burst owner missing")
            require_json_int(burst["burstIndex"], "emitter burst index", 0)
            require_json_float(burst["timeSeconds"], "emitter burst time")
            require_json_int(burst["countMinimum"], "emitter burst countMinimum", 0)
            require_json_int(burst["countMaximum"], "emitter burst countMaximum", 1)
            require(0 <= burst["countMinimum"] <= burst["countMaximum"]
                    and burst["timeSeconds"] >= 0.0,
                    "emitter burst range invalid")
            require_sha256(burst["sourceLiteralProjectionSha256"],
                           "emitter burst literal projection")
        for binding in emitter["actionCueParameterInputs"]:
            validate_action_cue_row(binding, "ActionCue parameter")
        for anchor in emitter["anchorRequests"]:
            require_keys(anchor, ANCHOR_REQUEST_KEYS, "anchor request")
            require_keys(anchor["socketLocalTransform"], SOCKET_TRANSFORM_KEYS,
                         "anchor socket transform")
        for key in ("position", "rotationDegrees", "revolutionDegreesPerSecond",
                    "scale", "velocityPerSecond"):
            require_numeric_vector(emitter["detailTransform"][key], 3,
                                   f"emitter detail {key}")
            require(all(type(item) is float for item in emitter["detailTransform"][key]),
                    f"emitter detail {key} must contain JSON floats")
        for key in ("sourcePositionUeUnits", "position", "rotationDegrees", "scale"):
            require_numeric_vector(emitter["cueLocalTransform"][key], 3,
                                   f"emitter cue {key}")
        require(emitter["detailTransform"]["decision"] == "VERIFIED_IDENTITY_IRRELEVANT"
                and emitter["detailTransform"]["consumptionPolicy"]
                == "IDENTITY_ONLY_NO_RUNTIME_OPERATION"
                and {key: emitter["detailTransform"][key] for key in TRANSFORM_KEYS}
                == {
                    "position": [0.0, 0.0, 0.0],
                    "rotationDegrees": [0.0, 0.0, 0.0],
                    "revolutionDegreesPerSecond": [0.0, 0.0, 0.0],
                    "scale": [1.0, 1.0, 1.0],
                    "velocityPerSecond": [0.0, 0.0, 0.0],
                }, "emitter detail transform must remain identity/irrelevant")
        require_self_digest(emitter["detailTransform"], "projectionSha256",
                            "emitter detail transform")
        inheritance = emitter["transformInheritance"]
        require(inheritance["enabled"] is False and inheritance["masterEmitterId"] == ""
                and inheritance["decision"] == "RECONSTRUCTED_NO_INHERITANCE_V1"
                and inheritance["consumptionPolicy"]
                == "NO_RUNTIME_PARENT_TRANSFORM_INHERITANCE",
                "emitter transform inheritance policy mismatch")
        require_self_digest(inheritance, "projectionSha256",
                            "emitter transform inheritance")
        validate_renderer_runtime_config(emitter)
        screen_adapter = emitter["screenPostAdapter"]
        if screen_adapter is not None:
            validate_adapter_self(screen_adapter, SCREEN_POST_ADAPTER_KEYS,
                                  "ScreenPost adapter")
            require(screen_adapter["adapterId"] == "ZOOM_BLUR_RECONSTRUCTED_V1"
                    and screen_adapter["enabled"] is True
                    and screen_adapter["frequencyPolicy"] == "IRRELEVANT_ZOOM_PROFILE"
                    and screen_adapter["sourceSpace"] == "screenSpaceV1"
                    and screen_adapter["fidelity"] == "RECONSTRUCTED_APPROVED_V1",
                    "ScreenPost adapter policy mismatch")
            require_json_float(screen_adapter["secondaryIntensity"],
                               "ScreenPost adapter secondary intensity")
            require(screen_adapter["secondaryIntensity"] == 0.0,
                    "ScreenPost secondary intensity mismatch")
            require_numeric_vector(screen_adapter["tint"], 4,
                                   "ScreenPost adapter tint")
            require(all(type(item) is float for item in screen_adapter["tint"])
                    and screen_adapter["tint"] == [1.0, 1.0, 1.0, 1.0],
                    "ScreenPost adapter tint mismatch")
            for key in (
                "occurrenceId", "materialOccurrenceId", "recipeId", "familyId",
                "intensityDistributionId", "alphaDistributionId",
            ):
                require(type(screen_adapter[key]) is str and screen_adapter[key],
                        f"ScreenPost adapter {key} missing")
        light_adapter = emitter["lightAdapter"]
        if light_adapter is not None:
            validate_adapter_self(light_adapter, LIGHT_ADAPTER_KEYS, "Light adapter")
            require(light_adapter["adapterId"] == "POINT_LIGHT_TYPED_COMPONENT_V1"
                    and light_adapter["enabled"] is True
                    and light_adapter["positionSourcePolicy"]
                    == "EMITTER_TRANSFORM_PLUS_PARTICLE_LOCATION",
                    "Light adapter policy mismatch")
            require_json_float(light_adapter["ueUnitScale"], "Light adapter unit scale")
            require(light_adapter["ueUnitScale"] == 0.01,
                    "Light adapter unit scale mismatch")
            require_string_list(light_adapter["fieldIds"], "Light adapter field IDs", True)
            require(len(light_adapter["fieldIds"]) == 8,
                    "Light adapter field denominator mismatch")
        decal_adapter = emitter["decalAdapter"]
        if decal_adapter is not None:
            validate_adapter_self(decal_adapter, DECAL_ADAPTER_KEYS, "Decal adapter")
            require(decal_adapter["adapterId"] == "EF_DECAL_TYPED_DESCRIPTOR_V1"
                    and decal_adapter["enabled"] is True
                    and decal_adapter["yawOnlyCdoDefault"] is True
                    and decal_adapter["yawOnlyCapabilityOutput"] is True
                    and decal_adapter["yawOnlyDecision"]
                    == "RECONSTRUCTED_CDO_AND_CAPABILITY_AGREE"
                    and decal_adapter["executionAdmission"] is True
                    and decal_adapter["supports3dDrawMode"] is True
                    and decal_adapter["fidelity"] == "RECONSTRUCTED_APPROVED_V1",
                    "Decal adapter capability/default policy mismatch")
            for key in ("nearPlane", "farPlane", "depthWorldUnits", "depthRuntimeUnits"):
                require_json_float(decal_adapter[key], f"Decal adapter {key}")
            require_numeric_vector(decal_adapter["defaultSize"], 2,
                                   "Decal adapter default size")
            require_numeric_vector(decal_adapter["blendRange"], 2,
                                   "Decal adapter blend range")
            require(all(type(item) is float for key in ("defaultSize", "blendRange")
                        for item in decal_adapter[key]),
                    "Decal adapter vector fields must contain JSON floats")
            require(decal_adapter["nearPlane"] == -300.0
                    and decal_adapter["farPlane"] == 300.0
                    and decal_adapter["defaultSize"] == [50.0, 50.0]
                    and decal_adapter["blendRange"] == [100.0, 100.0]
                    and decal_adapter["depthWorldUnits"] == 600.0
                    and decal_adapter["depthRuntimeUnits"] == 6.0,
                    "Decal adapter numeric contract mismatch")
        ribbon_adapter = emitter["ribbonAdapter"]
        if ribbon_adapter is not None:
            validate_adapter_self(ribbon_adapter, RIBBON_ADAPTER_KEYS, "Ribbon adapter")
            require(ribbon_adapter["adapterId"] == "CASCADE_RIBBON_TYPED_ADAPTER_V1"
                    and ribbon_adapter["enabled"] is True
                    and ribbon_adapter["widthPolicy"]
                    == "TYPED_SIZE_DISTRIBUTION_X_AXIS"
                    and ribbon_adapter["geometryPolicy"]
                    == "TYPED_RIBBON_DEFAULTS_RENDER_GEOMETRY"
                    and ribbon_adapter["orientationPolicy"]
                    == "CAMERA_FACING_SINGLE_SHEET_RECONSTRUCTED_V1"
                    and ribbon_adapter["fidelity"]
                    == "RECONSTRUCTED_TYPED_SOURCE_WITH_EXPLICIT_ORIENTATION_POLICY_V1"
                    and ribbon_adapter["blockers"] == [
                        "R5_RIBBON_RUNTIME_PROBE_NOT_COMPLETE",
                        "RIBBON_ORIENTATION_POLICY_REQUIRES_RUNTIME_PROBE",
                    ],
                    "Ribbon adapter policy mismatch")
            for key in (
                "colorModuleIds", "spawnDistributionIds", "lifetimeDistributionIds",
                "sizeDistributionIds", "colorDistributionIds", "dynamicDistributionIds",
            ):
                require_string_list(ribbon_adapter[key], f"Ribbon adapter {key}", True)
            for key in (
                "typeDataModuleId", "defaultId", "spawnModuleId", "lifetimeModuleId",
                "sizeModuleId", "dynamicParameterModuleId",
            ):
                require(type(ribbon_adapter[key]) is str and ribbon_adapter[key],
                        f"Ribbon adapter {key} missing")
            require_json_float(ribbon_adapter["tilingDistance"], "Ribbon tiling distance")
            require_json_float(ribbon_adapter["distanceTessellationStepSize"],
                               "Ribbon tessellation distance")
            require_json_int(ribbon_adapter["typedMaxParticleInTrailCount"],
                             "Ribbon typed particle cap", 1)
            require_json_int(ribbon_adapter["operationalMaxPoints"],
                             "Ribbon operational point cap", 1)
            require(ribbon_adapter["tilingDistance"] == 600.0
                    and ribbon_adapter["distanceTessellationStepSize"] == 5.0
                    and ribbon_adapter["operationalMaxPoints"]
                    == ribbon_adapter["typedMaxParticleInTrailCount"],
                    "Ribbon adapter numeric contract mismatch")
    adapter_contracts = {
        "screenPostAdapter": ("ScreenPost", 1),
        "lightAdapter": ("LightParticle", 1),
        "decalAdapter": ("DecalParticle", 3),
        "ribbonAdapter": ("CascadeRibbon", 1),
    }
    for field, (renderer_type, expected_count) in adapter_contracts.items():
        require(sum(row[field] is not None for row in program["emitters"])
                == expected_count,
                f"{field} denominator mismatch")
        require(all((row[field] is not None) == (row["rendererType"] == renderer_type)
                    for row in program["emitters"]),
                f"{field} renderer ownership mismatch")
    ribbon_emitter = next(row for row in program["emitters"]
                          if row["ribbonAdapter"] is not None)
    ribbon = ribbon_emitter["ribbonAdapter"]
    ribbon_defaults = [
        row for row in program["implicitDefaults"]
        if row["defaultId"] == ribbon["defaultId"]
        and row["moduleId"] == ribbon["typeDataModuleId"]
        and row["variant"] == "RIBBON_DEFAULTS"
    ]
    require(len(ribbon_defaults) == 1, "Ribbon adapter typed default owner mismatch")
    ribbon_values = ribbon_defaults[0]["ribbonValues"]
    require(ribbon_values["sheetsPerTrail"] == 1
            and ribbon_values["renderGeometry"] is True
            and ribbon_values["maxParticleInTrailCount"] == 500
            and ribbon["typedMaxParticleInTrailCount"] == 500
            and ribbon["operationalMaxPoints"] == 500,
            "Ribbon adapter sheet/render/cap default projection mismatch")
    ribbon_literals = {
        row["propertyPath"]: row for row in program["literals"]
        if row["moduleId"] == ribbon["typeDataModuleId"]
    }
    require(ribbon_literals["btangentrecalculationeveryframe"]["boolValue"] is True
            and ribbon_literals["distancetessellationstepsize"]["f64Value"] == 5.0
            and ribbon_literals["tilingdistance"]["f64Value"] == 600.0
            and ribbon_literals["lodvalidity"]["f64Value"] == 1.0,
            "Ribbon adapter explicit typed literal projection mismatch")
    for row in program["distributions"]:
        require(row["variant"] in DISTRIBUTION_VARIANTS, "unknown distribution variant")
        require_json_int(row["order"], "distribution order", 0)
        require_json_int(row["componentCount"], "distribution component count", 1)
        table_variant = row["variant"] in {"INLINE", "FLOAT_CURVE"}
        for key in (
            "operation", "randomLockAxes", "lookupTableChunkSize",
            "lookupTableNumElements",
        ):
            if table_variant:
                require_json_int(row[key], f"distribution {key}", 0)
            else:
                require(row[key] is None,
                        f"parameter distribution carries {key}")
        for key in ("lookupTableTimeScale", "lookupTableStartTime"):
            if table_variant:
                require_json_float(row[key], f"distribution {row['distributionId']} {key}")
            else:
                require(row[key] is None,
                        f"parameter distribution carries {key}")
        for key in (
            "defaultMinimum", "defaultMaximum", "lookupTable", "minimumInput",
            "maximumInput", "minimumOutput", "maximumOutput", "constantValues",
        ):
            value = row[key]
            require(type(value) is list
                    and all(type(item) is float and math.isfinite(item) for item in value),
                    f"distribution {row['distributionId']} {key} must contain floats")
        for item in row["actionCueBindings"]:
            validate_action_cue_row(item, "distribution ActionCue binding")
        for item in row["fieldProvenance"]:
            require_keys(item, FIELD_PROVENANCE_KEYS, "distribution field provenance")
            require(all(type(item[key]) is str and item[key]
                        for key in ("fieldPath", "provenanceTier", "evidenceStatus")),
                    "distribution field provenance strings missing")
            if item["valueSha256"] != "":
                require_sha256(item["valueSha256"],
                               "distribution field provenance value")
                require(item["typedValueBindingSha256"] == "",
                        "raw distribution provenance carries reconstructed binding")
            else:
                reconstructed_field_bindings = {
                    "operation": "operation",
                    "lookupTableChunkSize": "lookupTableChunkSize",
                    "lookupTableNumElements": "lookupTableNumElements",
                }
                value_key = reconstructed_field_bindings.get(item["fieldPath"])
                require(value_key is not None
                        and item["provenanceTier"]
                        == "CURRENT_UE3_RAW_DISTRIBUTION_DEFAULT_AND_PAYLOAD_SHAPE"
                        and item["evidenceStatus"]
                        in {"SOURCE_EXACT", "CURRENT_REVISION_EVIDENCE"},
                        "unbound reconstructed distribution provenance")
                expected_binding = canonical_sha256({
                    "fieldPath": item["fieldPath"],
                    "typedValue": row[value_key],
                })
                require(item["typedValueBindingSha256"] == expected_binding,
                        "reconstructed distribution typed value binding mismatch")
        for item in row["samples"]:
            require_keys(item, DIST_SAMPLE_KEYS, "distribution sample")
            require(type(item["sampleId"]) is str and item["sampleId"],
                    "distribution sample ID missing")
            require_json_float(item["time"], "distribution sample time")
            require(type(item["randomUnits"]) is list
                    and all(type(value) is float and math.isfinite(value)
                            for value in item["randomUnits"]),
                    "distribution sample random units must be float array")
            require(type(item["outputValues"]) is list
                    and item["outputValues"]
                    and all(type(value) is float and math.isfinite(value)
                            for value in item["outputValues"]),
                    "distribution sample output must be non-empty float array")
            require(item["outputSha256"] == canonical_sha256(item["outputValues"]),
                    "distribution sample output digest mismatch")
            require(item["actionCueBindingsSha256"]
                    == canonical_sha256(row["actionCueBindings"]),
                    "distribution sample ActionCue digest mismatch")
            require_json_float(item["absoluteTolerance"],
                               "distribution sample absolute tolerance")
            require_json_float(item["relativeTolerance"],
                               "distribution sample relative tolerance")
            require(item["absoluteTolerance"] == item["relativeTolerance"] == 1.0e-6,
                    "distribution sample tolerance mismatch")
            if item["domain"] == "DISTRIBUTION_EVALUATOR":
                require(item["inputVariant"] == "TIME_RANDOM_UNITS"
                        and item["sourceCueId"] is None and item["branch"] is None
                        and item["parameterInput"] is None
                        and item["diagnosticStandardBaseValue"] is None
                        and item["blocked"] is None,
                        "distribution evaluator sample variant mismatch")
                input_payload = {
                    "domain": item["domain"],
                    "distributionId": row["distributionId"],
                    "inputVariant": item["inputVariant"],
                    "time": item["time"],
                    "randomUnits": item["randomUnits"],
                }
            else:
                require(item["domain"] == "PARTICLE_PARAMETER_BRANCH"
                        and item["inputVariant"] == "PARTICLE_PARAMETER_INPUT"
                        and item["time"] == 0.0 and item["randomUnits"] == []
                        and type(item["sourceCueId"]) is str and item["sourceCueId"]
                        and item["branch"] in {"PARAMETER_INPUT", "CONSTANT_FALLBACK"}
                        and item["diagnosticStandardBaseValue"] is None
                        and item["blocked"] is False,
                        "ParticleParameter sample variant mismatch")
                parameter_input = item["parameterInput"]
                if item["branch"] == "PARAMETER_INPUT":
                    require(type(parameter_input) is dict,
                            "parameter sample input missing")
                    require_keys(parameter_input, PARAMETER_SAMPLE_INPUT_KEYS,
                                 "ParticleParameter sample input")
                    require(type(parameter_input["name"]) is str
                            and parameter_input["name"],
                            "ParticleParameter sample name missing")
                    require(parameter_input["kind"] in {"scalar", "vector"},
                            "ParticleParameter sample kind invalid")
                    require_json_int(parameter_input["sourceIndex"],
                                     "ParticleParameter sample source index", 0)
                    require_json_int(parameter_input["sourceValueByteOffset"],
                                     "ParticleParameter sample byte offset", 0)
                    if parameter_input["kind"] == "scalar":
                        require_json_float(parameter_input["value"],
                                           "ParticleParameter scalar value")
                    else:
                        require(type(parameter_input["value"]) is list
                                and len(parameter_input["value"]) in (3, 4)
                                and all(type(value) is float and math.isfinite(value)
                                        for value in parameter_input["value"]),
                                "ParticleParameter vector value invalid")
                else:
                    require(parameter_input is None,
                            "constant fallback carries parameter input")
                input_payload = {
                    "domain": item["domain"],
                    "distributionId": row["distributionId"],
                    "inputVariant": item["inputVariant"],
                    "sourceCueId": item["sourceCueId"],
                    "branch": item["branch"],
                    "parameterInput": parameter_input,
                    "actionCueBindings": row["actionCueBindings"],
                    "diagnosticStandardBaseValue": item["diagnosticStandardBaseValue"],
                    "blocked": item["blocked"],
                }
            require(item["inputSha256"] == canonical_sha256(input_payload),
                    "distribution sample input digest mismatch")
        for item in row["curveKeys"]:
            require_keys(item, CURVE_KEY_KEYS, "distribution curve key")
            require_json_float(item["time"], "distribution curve key time")
            for key in CURVE_KEY_KEYS[1:-1]:
                require(type(item[key]) is list and len(item[key]) == 4
                        and all(type(value) is float and math.isfinite(value)
                                for value in item[key]),
                        f"distribution curve key {key} must be float4")
            require(item["interpolation"] in {"cubic", "linear"},
                    "distribution curve interpolation invalid")
        if row["variant"] in {"FLOAT_PARAMETER", "VECTOR_PARAMETER", "EF_MULTIPLY"}:
            count = row["componentCount"]
            for key in ("paramModes", "minimumInput", "maximumInput", "minimumOutput",
                        "maximumOutput", "constantValues"):
                require(type(row[key]) is list and len(row[key]) == count,
                        f"distribution {row['distributionId']} {key} arity mismatch")
            require(type(row["isDirty"]) is bool, "parameter distribution isDirty must be bool")
        else:
            require(row["parameterName"] == "" and row["paramModes"] == [],
                    "non-parameter distribution carries parameter slots")
            require(row["isDirty"] is None, "non-parameter distribution carries isDirty")
    require(sum(len(row["samples"]) for row in program["distributions"]) == 1852
            and sum(item["domain"] == "DISTRIBUTION_EVALUATOR"
                    for row in program["distributions"] for item in row["samples"]) == 1839
            and sum(item["domain"] == "PARTICLE_PARAMETER_BRANCH"
                    for row in program["distributions"] for item in row["samples"]) == 13
            and sum(item["parameterInput"] is not None
                    for row in program["distributions"] for item in row["samples"]) == 7,
            "distribution numeric sample denominator mismatch")
    require(canonical_sha256([
        {"distributionId": row["distributionId"], "samples": row["samples"]}
        for row in program["distributions"]
    ]) == EXPECTED_DISTRIBUTION_SAMPLES_SHA256,
            "distribution numeric sample frozen projection mismatch")
    for row in program["implicitDefaults"]:
        require(row["variant"] in IMPLICIT_DEFAULT_VARIANTS, "unknown implicit default variant")
        if row["ribbonValues"] is not None:
            require_keys(row["ribbonValues"], RIBBON_VALUE_KEYS, "ribbon defaults")
        if row["decalValues"] is not None:
            require_keys(row["decalValues"], DECAL_VALUE_KEYS, "decal defaults")
    for row in program["pointLightFields"]:
        require(row["valueVariant"] in POINT_LIGHT_VALUE_VARIANTS,
                "unknown PointLight value variant")
        populated = sum((
            row["boolValue"] is not None,
            row["f64Value"] is not None,
            row["colorRgba8Value"] is not None,
            row["guid128Value"] != "",
        ))
        require(populated == 1, "PointLight row must populate exactly one value slot")
    for row in program["materialInputs"]:
        require(row["valueVariant"] in {"F64", "F64X4", "TEXTURE_ID"},
                "unknown material input variant")
        populated = sum((
            row["valueF64"] is not None,
            row["valueF64x4"] != [],
            row["valueTextureId"] != "",
        ))
        require(populated == 1, "material input must populate exactly one value slot")
    for row in program["materialPolicyRows"]:
        require(row["domain"] in MATERIAL_POLICY_DOMAINS, "unknown Material policy domain")
        if row["domain"] == "SAMPLER_DESCRIPTOR":
            require(row["valueVariant"] == "SAMPLER_DESCRIPTOR",
                    "sampler policy variant mismatch")
            validate_sampler_descriptor(row["samplerDescriptor"], "sampler descriptor")
        else:
            require(row["samplerDescriptor"] is None,
                    "non-sampler Material policy carries sampler descriptor")
            require(row["valueVariant"] in {"BOOL", "F64", "ENUM_STRING"},
                    "unknown Material policy value variant")
        descriptor_oracle = row["d3dDescriptorOracle"]
        srv_oracle = row["d3dSrvOracle"]
        if descriptor_oracle is not None:
            require(type(descriptor_oracle) is dict,
                    "D3D descriptor oracle must be object or null")
            require_keys(descriptor_oracle, D3D_DESCRIPTOR_ORACLE_KEYS,
                         "D3D descriptor oracle")
            require(descriptor_oracle["policyRowId"] == row["policyRowId"],
                    "D3D descriptor owner mismatch")
            normalize_d3d_descriptor_oracle(descriptor_oracle)
        if srv_oracle is not None:
            require(type(srv_oracle) is dict, "D3D SRV oracle must be object or null")
            require_keys(srv_oracle, D3D_SRV_ORACLE_KEYS, "D3D SRV oracle")
            require(srv_oracle["policyRowId"] == row["policyRowId"],
                    "D3D SRV owner mismatch")
            normalize_d3d_srv_oracle(srv_oracle)
        if row["domain"] == "SAMPLER_DESCRIPTOR":
            require(descriptor_oracle is not None
                    and descriptor_oracle["descriptorKind"] == "D3D11_SAMPLER_DESC"
                    and srv_oracle is not None,
                    "sampler policy must own sampler and SRV exact oracles")
        else:
            require(srv_oracle is None, "non-sampler policy cannot own SRV oracle")
    validate_material_numeric_samples(program)
    has_texture_binding_receipt = any(
        row["artifactId"] == "materialTextureBinding"
        for row in program["inputArtifacts"]
    )
    source_binding_ids: set[str] = set()
    for row in program["materialTextureBindings"]:
        require_json_int(row["order"], "Material texture binding order", 0)
        require(type(row["logicalTexturePath"]) is str and row["logicalTexturePath"]
                and "\\" not in row["logicalTexturePath"]
                and "/" not in row["logicalTexturePath"],
                "Material logical texture path invalid")
        require_string_list(row["materialOccurrenceIds"],
                            "Material texture occurrence IDs", unique=True)
        require(row["sourceExact"] is False,
                "Material texture binding cannot claim Source exact")
        if has_texture_binding_receipt:
            require(type(row["sourceBindingId"]) is str and row["sourceBindingId"]
                    and row["sourceBindingId"] not in source_binding_ids
                    and type(row["sourceTextureResourceId"]) is str
                    and row["sourceTextureResourceId"],
                    "Material texture receipt identity/uniqueness mismatch")
            source_binding_ids.add(row["sourceBindingId"])
            require_sha256(row["sourceBindingRowSha256"],
                           "Material texture source binding row")
            require_sha256(row["sourceTextureResourceRowSha256"],
                           "Material texture source resource row")
        else:
            require(all(row[key] == "" for key in (
                "sourceBindingId", "sourceBindingRowSha256", "sourceTextureResourceId",
                "sourceTextureResourceRowSha256", "sourceProvisioningProposalId",
                "sourceProvisioningProposalRowSha256", "sourceReceiptStatus",
            )), "legacy Material texture binding carries receipt identity")
        if type(row["runtimeAssetId"]) is str and row["runtimeAssetId"] != "":
            require(has_texture_binding_receipt,
                    "resolved Material texture lacks frozen binding receipt")
            require_safe_renderer_texture_asset_id(row["runtimeAssetId"])
            require(row["resolutionStatus"] == "RESOLVED_EXACT_RUNTIME_ASSET"
                    and row["bindingBasis"]
                    == "EXACT_FULL_LOGICAL_PATH_RUNTIME_COOK_RECEIPT"
                    and row["sourceReceiptStatus"]
                    == "RESOLVED_EXACT_RUNTIME_COOK_RECEIPT"
                    and row["sourceProvisioningProposalId"] == ""
                    and row["sourceProvisioningProposalRowSha256"] == ""
                    and row["blockers"]
                    == ["R4_TEXTURE_SRV_CONSUMER_NOT_COMPLETE"],
                    "resolved Material texture runtime binding contract mismatch")
        elif row["runtimeAssetId"] is None:
            require_sha256(row["sourceProvisioningProposalRowSha256"],
                           "Material texture source provisioning proposal row")
            require(has_texture_binding_receipt
                    and row["resolutionStatus"] == "UNRESOLVED_RUNTIME_ASSET"
                    and row["bindingBasis"]
                    == "EXACT_DDS_PROVISIONING_PENDING_NO_RUNTIME_ASSET"
                    and row["sourceReceiptStatus"] == "UNRESOLVED_RUNTIME_ASSET"
                    and type(row["sourceProvisioningProposalId"]) is str
                    and row["sourceProvisioningProposalId"]
                    and row["blockers"] == [
                        "MATERIAL_TEXTURE_RUNTIME_ASSET_UNRESOLVED",
                        "EXACT_DDS_TRANSACTIONAL_DEPLOYMENT_PENDING",
                        "R4_TEXTURE_SRV_CONSUMER_NOT_COMPLETE",
                    ], "unresolved Material texture receipt contract mismatch")
        else:
            require(not has_texture_binding_receipt and row["runtimeAssetId"] == ""
                    and row["resolutionStatus"]
                    == "UNRESOLVED_EXACT_LOGICAL_TEXTURE_RESOURCE_BINDING"
                    and row["bindingBasis"] == "NO_BASENAME_GUESS"
                    and row["blockers"]
                    == ["MATERIAL_TEXTURE_FIELD_RUNTIME_RESOURCE_BINDING_NOT_COMPLETE"],
                    "legacy unresolved Material texture binding contract mismatch")
    require((not has_texture_binding_receipt and not source_binding_ids)
            or len(source_binding_ids) == 72,
            "Material texture source binding reverse denominator mismatch")
    if has_texture_binding_receipt:
        require(sum(
            row["resolutionStatus"] == "RESOLVED_EXACT_RUNTIME_ASSET"
            for row in program["materialTextureBindings"]
        ) == 68 and sum(
            row["resolutionStatus"] == "UNRESOLVED_RUNTIME_ASSET"
            for row in program["materialTextureBindings"]
        ) == 4, "Material texture resolved/unresolved denominator mismatch")
        require(canonical_sha256(program["materialTextureBindings"])
                == EXPECTED_MATERIAL_TEXTURE_BINDINGS_SHA256,
                "Material texture frozen 72-row projection mismatch")
    for row in program["rendererTextureResources"]:
        require_json_int(row["order"], "renderer texture resource order", 0)
        require(row["slotId"] in RENDERER_TEXTURE_SLOTS,
                "renderer texture resource slot invalid")
        require_safe_renderer_texture_asset_id(row["assetId"])
        require(row["projectionRole"] == "RECONSTRUCTED_RENDERER_SLOT_PROJECTION"
                and row["sourceExact"] is False
                and row["blockers"]
                == ["MATERIAL_TEXTURE_FIELD_RUNTIME_RESOURCE_BINDING_NOT_COMPLETE"],
                "renderer texture projection boundary mismatch")
        require_sha256(row["sourceResourceProjectionSha256"],
                       "renderer texture source projection")
    validate_geometry_nested(program)


def validate_owner_order(program: dict[str, Any]) -> None:
    ids_by_section: dict[str, set[str]] = {}
    global_ids: set[str] = set()
    for section, id_field in PRIMARY_ID_FIELDS.items():
        values = [row[id_field] for row in program[section]]
        require(all(type(value) is str and value for value in values),
                f"{section}: primary IDs must be non-empty strings")
        require(len(values) == len(set(values)), f"{section}: duplicate primary ID")
        overlap = global_ids.intersection(values)
        require(not overlap, f"global primary ID collision: {sorted(overlap)[:1]}")
        global_ids.update(values)
        ids_by_section[section] = set(values)
    handler_rows = {row["handlerRegistryId"]: row for row in program["handlerRegistry"]}
    handlers = set(handler_rows)
    emitters = {row["emitterId"]: row for row in program["emitters"]}
    modules = {row["moduleId"]: row for row in program["modules"]}
    properties = {row["propertyId"]: row for row in program["properties"]}
    leaves = {row["leafId"]: row for row in program["primitiveLeaves"]}
    literals = {row["literalId"]: row for row in program["literals"]}
    distributions = {row["distributionId"]: row for row in program["distributions"]}
    schedules = {row["scheduleId"]: row for row in program["actionSchedules"]}
    emitter_identity_projection = [{
        "emitterId": row["emitterId"],
        "order": row["order"],
        "evidenceId": row["evidenceId"],
        "sourceOccurrenceId": row["sourceOccurrenceId"],
        "sourceSystemId": row["sourceSystemId"],
        "sourceEmitterPath": row["sourceEmitterPath"],
        "sourceEmitterNodeId": row["sourceEmitterNodeId"],
        "sourceCueId": row["sourceCueId"],
        "sourceElementId": row["sourceElementId"],
        "sourceNode": row["sourceNode"],
        "visible": row["visible"],
        "rendererType": row["rendererType"],
        "rendererSourceSpace": row["rendererSourceSpace"],
        "selectedLodPath": row["selectedLodPath"],
        "selectedLodNodeId": row["selectedLodNodeId"],
        "selectedLodRecordSha256": row["selectedLodRecordSha256"],
    } for row in program["emitters"]]
    require(canonical_json_bytes(emitter_identity_projection)
            == frozen_emitter_identity_canonical(),
            "frozen Source/compiler emitter identity projection mismatch")
    require(len(schedules) == len(EXPECTED_ACTION_SCHEDULES),
            "action schedule denominator mismatch")
    for order, (row, expected) in enumerate(zip(
        program["actionSchedules"], EXPECTED_ACTION_SCHEDULES, strict=True,
    )):
        (cue_id, occurrence_id, system_id, event_index, global_time,
         duration, cue_sha) = expected
        expected_projection = {
            "order": order,
            "sourceCueId": cue_id,
            "sourceOccurrenceId": occurrence_id,
            "sourceSystemId": system_id,
            "sourceReceiptEventIndex": event_index,
            "globalTimeSeconds": global_time,
            "durationSeconds": duration,
            "sourceCueRowSha256": cue_sha,
        }
        require_json_float(row["globalTimeSeconds"], "action schedule global time")
        require_json_float(row["durationSeconds"], "action schedule duration")
        require_json_int(row["sourceReceiptEventIndex"],
                         "action schedule receipt event index", 0)
        require_sha256(row["sourceCueRowSha256"], "action schedule cue row")
        require({key: row[key] for key in expected_projection} == expected_projection
                and row["scheduleId"]
                == "action-schedule-" + canonical_sha256(expected_projection)[:24],
                f"action schedule {order} frozen projection mismatch")
    schedule_coverage = {schedule_id: 0 for schedule_id in schedules}
    for handler in handler_rows.values():
        contract = {key: handler[key] for key in (
            "handlerKind", "implementationId", "implementationVersion",
            "implementationSha256", "exactSourceClass", "variant", "consumerContract",
        )}
        require(handler["contractSha256"] == canonical_sha256(contract)
                and handler["handlerRegistryId"]
                == "handler-" + canonical_sha256(contract)[:24],
                f"handler registry identity mismatch for {handler['handlerRegistryId']}")
    for emitter in emitters.values():
        require(emitter["scheduleId"] in schedules,
                f"emitter {emitter['emitterId']} schedule missing")
        schedule = schedules[emitter["scheduleId"]]
        require(schedule["sourceCueId"] == emitter["sourceCueId"]
                and schedule["sourceOccurrenceId"] == emitter["sourceOccurrenceId"]
                and schedule["sourceSystemId"] == emitter["sourceSystemId"].casefold(),
                f"emitter {emitter['emitterId']} schedule identity mismatch")
        cue_projection = {
            "sourceCueId": emitter["sourceCueId"],
            "sourceOccurrenceId": emitter["sourceOccurrenceId"],
            "sourceSystemId": emitter["sourceSystemId"].casefold(),
            "cueLocalTransform": emitter["cueLocalTransform"],
            "actionCueAttachment": emitter["actionCueAttachment"],
            "actionCueParameterInputs": emitter["actionCueParameterInputs"],
        }
        require(emitter["sourceActionCueProjectionSha256"]
                == canonical_sha256(cue_projection)
                and emitter["sourceActionCueProjectionSha256"]
                == EXPECTED_ACTION_CUE_PROJECTION_SHA256[emitter["sourceCueId"]],
                f"emitter {emitter['emitterId']} action cue projection mismatch")
        schedule_coverage[emitter["scheduleId"]] += 1
        owned = [row for row in program["modules"] if row["emitterId"] == emitter["emitterId"]]
        require([row["order"] for row in owned] == list(range(len(owned))),
                f"emitter {emitter['emitterId']} module order mismatch")
        require(emitter["moduleIds"] == [row["moduleId"] for row in owned],
                f"emitter {emitter['emitterId']} module ID order mismatch")
    require(list(schedule_coverage.values()) == [4, 1, 15, 12, 1, 1, 1],
            "action schedule reverse coverage mismatch")
    frozen_capabilities = frozen_source_capability_by_module()
    reconstructed_capability_ids: set[str] = set()
    for module in modules.values():
        require(module["emitterId"] in emitters, "module has unknown emitter owner")
        require(module["handlerRegistryId"] in handlers, "module has unknown handler")
        require(module["sourceDecision"]
                == ("READY_FOR_HANDLER" if module["selectionKind"] == "SOURCE_HANDLER"
                    else "BLOCKED"),
                f"module {module['moduleId']} source decision mismatch")
        handler = handler_rows[module["handlerRegistryId"]]
        expected_kind = (
            "SOURCE_MODULE" if module["selectionKind"] == "SOURCE_HANDLER"
            else "RECONSTRUCTED_MODULE"
        )
        require(handler["handlerKind"] == expected_kind,
                f"module {module['moduleId']} handler kind mismatch")
        require(handler["exactSourceClass"] == module["exactSourceClass"],
                f"module {module['moduleId']} handler class mismatch")
        require(handler["variant"] == module["handlerVariant"],
                f"module {module['moduleId']} handler variant mismatch")
        owned_literal_rows = [row for row in program["literals"]
                              if row["moduleId"] == module["moduleId"]]
        owned_distribution_rows = [row for row in program["distributions"]
                                   if row["moduleId"] == module["moduleId"]]
        owned_property_rows = [row for row in program["properties"]
                               if row["moduleId"] == module["moduleId"]]
        if module["selectionKind"] == "SOURCE_HANDLER":
            for key in (
                "capabilityPolicyFamilyId", "capabilityImplementationId",
                "capabilityImplementationSha256", "capabilityFamilySemanticSha256",
                "capabilityInputSchemaSha256", "capabilityOutputSchemaSha256",
                "capabilityDefaultPolicySha256", "capabilitySourceRowSha256",
                "capabilityLiteralBindingsSha256", "capabilityDistributionBindingsSha256",
                "capabilityPropertyConsumptionSha256", "capabilitySeedBindingSha256",
                "capabilityActionCueInputsSha256", "approvalPolicyRowId",
                "approvalUpstreamIdentitySha256", "approvalPolicyBindingSha256",
                "approvalRequiredMutatedOutput", "approvalSourceRowSha256",
            ):
                require(module[key] == "", f"source module {module['moduleId']} {key} not empty")
            require(module["capabilityImplementationVersion"] == 0
                    and module["capabilityNumericSamples"] == []
                    and module["capabilityActionCueInputNames"] == []
                    and module["approvalRequiredOracleIds"] == []
                    and module["approvalExecutionBlockers"] == []
                    and module["approvalSourceRow"] is None,
                    f"source module {module['moduleId']} carries reconstructed capability")
            for prop in owned_property_rows:
                require(prop["capabilityConsumptionDecision"] == ""
                        and prop["semanticRole"] == ""
                        and prop["outputDependencyRequired"] is False
                        and prop["capabilitySourceRowSha256"] == "",
                        f"source property {prop['propertyId']} carries capability metadata")
        else:
            capability = frozen_capabilities.get(module["moduleId"])
            require(capability is not None,
                    f"reconstructed module {module['moduleId']} lacks frozen capability row")
            reconstructed_capability_ids.add(module["moduleId"])
            expected_literal_sha = canonical_sha256(
                capability_literal_projection_from_raw(capability["sourceLiteralBindings"])
            )
            expected_distribution_sha = canonical_sha256(
                capability_distribution_projection_from_raw(
                    capability["distributionBindings"]
                )
            )
            expected_property_sha = canonical_sha256(
                capability_property_projection_from_raw(capability["propertyConsumption"])
            )
            expected_seed_sha = canonical_sha256(
                capability_seed_projection_from_raw(capability["seedBinding"])
            )
            expected_action_cues = typed_action_cue_bindings(
                capability["actionCueParameterInputs"]
            )
            require(
                module["sourceObjectId"] == capability["sourceObjectId"]
                and module["sourceRecordSha256"] == capability["sourceRecordSha256"]
                and module["exactSourceClass"] == capability["exactSourceClass"]
                and module["handlerVariant"] == capability["variant"]
                and module["capabilityPolicyFamilyId"] == capability["policyFamilyId"]
                and module["capabilityImplementationId"]
                == capability["implementationId"]
                and module["capabilityImplementationVersion"]
                == capability["implementationVersion"]
                and module["capabilityImplementationSha256"]
                == capability["implementationSha256"]
                and module["capabilityFamilySemanticSha256"]
                == capability["familySemanticImplementationSha256"]
                and module["capabilityInputSchemaSha256"]
                == capability["variantInputSchemaSha256"]
                and module["capabilityOutputSchemaSha256"]
                == capability["variantOutputSchemaSha256"]
                and module["capabilityDefaultPolicySha256"]
                == capability["explicitDefaultsSha256"]
                and module["capabilitySourceRowSha256"]
                == canonical_sha256(capability)
                and module["capabilityLiteralBindingsSha256"]
                == expected_literal_sha
                and module["capabilityDistributionBindingsSha256"]
                == expected_distribution_sha
                and module["capabilityPropertyConsumptionSha256"]
                == expected_property_sha
                and module["capabilitySeedBindingSha256"] == expected_seed_sha
                and module["capabilityActionCueInputNames"]
                == [row["name"] for row in expected_action_cues]
                and module["capabilityActionCueInputsSha256"]
                == canonical_sha256(expected_action_cues)
                and module["capabilityNumericSamples"]
                == normalized_capability_samples(module["moduleId"], capability),
                f"reconstructed module {module['moduleId']} frozen capability mismatch",
            )
            raw_properties = {
                row["propertyId"]: row for row in capability["propertyConsumption"]
            }
            require(len(raw_properties) == len(capability["propertyConsumption"])
                    and set(raw_properties)
                    == {row["propertyId"] for row in owned_property_rows},
                    f"reconstructed module {module['moduleId']} capability property coverage mismatch")
            for prop in owned_property_rows:
                require(prop["capabilitySourceRowSha256"]
                        == canonical_sha256(raw_properties[prop["propertyId"]]),
                        f"property {prop['propertyId']} frozen capability row mismatch")
            require(module["capabilityImplementationVersion"] > 0,
                    f"reconstructed module {module['moduleId']} version missing")
            require(handler["implementationId"] == module["capabilityImplementationId"]
                    and handler["implementationVersion"]
                    == module["capabilityImplementationVersion"]
                    and handler["implementationSha256"]
                    == module["capabilityImplementationSha256"],
                    f"reconstructed module {module['moduleId']} handler contract mismatch")
            samples = module["capabilityNumericSamples"]
            require(len(samples) == 3,
                    f"reconstructed module {module['moduleId']} sample denominator mismatch")
            for index, sample in enumerate(samples):
                require_keys(sample, CAPABILITY_SAMPLE_KEYS, "module capability sample")
                require(sample["order"] == index
                        and sample["ownerModuleId"] == module["moduleId"],
                        f"module capability sample owner/order mismatch for {module['moduleId']}")
                require(type(sample["sampleId"]) is str and sample["sampleId"],
                        "module capability sample ID missing")
                require_json_float(sample["time"], "module capability sample time")
                require_json_int(sample["fixedSeed"], "module capability fixed seed")
                require(type(sample["fixedSeedSource"]) is str
                        and sample["fixedSeedSource"],
                        "module capability fixed seed source missing")
                require(type(sample["randomUnits"]) is list
                        and len(sample["randomUnits"]) == 4
                        and all(type(value) is float and math.isfinite(value)
                                for value in sample["randomUnits"]),
                        "module capability random units invalid")
                require(sample["inputVariant"] == module["handlerVariant"]
                        and sample["outputVariant"] == module["handlerVariant"],
                        "module capability sample variant mismatch")
                for key in (
                    "inputLiteralProjectionSha256", "inputDistributionProjectionSha256",
                    "typedInputSha256", "outputSha256",
                ):
                    require_sha256(sample[key], f"module capability sample {key}")
                for key in ("inputValues", "outputValues"):
                    require(type(sample[key]) is list and sample[key]
                            and all(type(value) is float and math.isfinite(value)
                                    for value in sample[key]),
                            f"module capability sample {key} invalid")
                require_json_float(sample["absoluteTolerance"],
                                   "module capability absolute tolerance")
                require_json_float(sample["relativeTolerance"],
                                   "module capability relative tolerance")
                require(sample["absoluteTolerance"] == sample["relativeTolerance"] == 1.0e-6,
                        "module capability sample tolerance mismatch")
            require(module["capabilityLiteralBindingsSha256"] == canonical_sha256(
                capability_literal_projection_from_program(owned_literal_rows)
            ), f"capability literal projection mismatch for {module['moduleId']}")
            require(module["capabilityDistributionBindingsSha256"] == canonical_sha256(
                capability_distribution_projection_from_program(owned_distribution_rows)
            ), f"capability distribution projection mismatch for {module['moduleId']}")
            require(module["capabilityPropertyConsumptionSha256"] == canonical_sha256(
                capability_property_projection_from_program(owned_property_rows)
            ), f"capability property projection mismatch for {module['moduleId']}")
            owned_seed = next((row for row in program["seedPolicies"]
                               if row["moduleId"] == module["moduleId"]), None)
            require(module["capabilitySeedBindingSha256"] == canonical_sha256(
                capability_seed_projection_from_program(owned_seed)
            ), f"capability seed projection mismatch for {module['moduleId']}")
            names = module["capabilityActionCueInputNames"]
            require(len(names) == len(set(names)),
                    f"capability ActionCue names duplicate for {module['moduleId']}")
            emitter_cues = emitters[module["emitterId"]]["actionCueParameterInputs"]
            scoped_cues = [row for row in emitter_cues if row["name"] in names]
            require([row["name"] for row in scoped_cues] == names
                    and module["capabilityActionCueInputsSha256"]
                    == canonical_sha256(scoped_cues),
                    f"capability ActionCue projection mismatch for {module['moduleId']}")
            approval_row = module["approvalSourceRow"]
            require(type(approval_row) is dict,
                    f"reconstructed module {module['moduleId']} approval row missing")
            require_keys(approval_row, APPROVAL_SOURCE_ROW_KEYS, "approval Source row")
            require(module["approvalSourceRowSha256"] == canonical_sha256(approval_row)
                    and approval_row["moduleOccurrenceId"] == module["moduleId"]
                    and approval_row["exactSourceClass"] == module["exactSourceClass"]
                    and approval_row["policyRowId"] == module["approvalPolicyRowId"]
                    and approval_row["upstreamIdentitySha256"]
                    == module["approvalUpstreamIdentitySha256"]
                    and approval_row["policyBindingSha256"]
                    == module["approvalPolicyBindingSha256"]
                    and approval_row["requiredMutatedOutput"]
                    == module["approvalRequiredMutatedOutput"]
                    and approval_row["requiredOracleIds"]
                    == module["approvalRequiredOracleIds"]
                    and approval_row["executionBlockers"]
                    == module["approvalExecutionBlockers"],
                    f"approval Source projection mismatch for {module['moduleId']}")
        for section, field, owner_field in (
            (program["properties"], "propertyIds", "moduleId"),
            (program["primitiveLeaves"], "primitiveLeafIds", "moduleId"),
            (program["literals"], "literalIds", "moduleId"),
            (program["distributions"], "distributionIds", "moduleId"),
        ):
            owned = [row for row in section if row[owner_field] == module["moduleId"]]
            require([row["order"] for row in owned] == list(range(len(owned))),
                    f"module {module['moduleId']} owned row order mismatch")
            primary = PRIMARY_ID_FIELDS[next(
                name for name in SECTION_NAMES if program[name] is section
            )]
            require(module[field] == [row[primary] for row in owned],
                    f"module {module['moduleId']} {field} mismatch")
    require(reconstructed_capability_ids == set(frozen_capabilities),
            "frozen reconstructed capability reverse coverage mismatch")
    source_candidate_artifacts = [
        row for row in program["inputArtifacts"] if row["artifactId"] == "sourceCandidate"
    ]
    require(len(source_candidate_artifacts) == 1,
            "source candidate input artifact missing")
    source_candidate_sha = source_candidate_artifacts[0]["canonicalJsonSha256"]
    total_bursts = 0
    follow_attachment_count = 0
    for emitter in emitters.values():
        timing = emitter["timing"]
        required = modules.get(timing["requiredModuleId"])
        spawn = modules.get(timing["spawnModuleId"])
        lifetime = modules.get(timing["lifetimeModuleId"])
        require(required is not None and required["emitterId"] == emitter["emitterId"]
                and required["exactSourceClass"] == "particlemodulerequired",
                f"emitter {emitter['emitterId']} Required timing owner mismatch")
        require(spawn is not None and spawn["emitterId"] == emitter["emitterId"]
                and spawn["exactSourceClass"] == "particlemodulespawn",
                f"emitter {emitter['emitterId']} Spawn timing owner mismatch")
        require(lifetime is not None and lifetime["emitterId"] == emitter["emitterId"]
                and lifetime["exactSourceClass"] in {
                    "particlemodulelifetime", "particlemodulelifetime_seeded",
                }, f"emitter {emitter['emitterId']} Lifetime timing owner mismatch")
        lifetime_distributions = [
            row for row in program["distributions"]
            if row["moduleId"] == lifetime["moduleId"]
            and row["propertyPath"] == "lifetime"
        ]
        require(len(lifetime_distributions) == 1,
                f"emitter {emitter['emitterId']} Lifetime distribution mismatch")
        required_literals = {
            row["propertyPath"]: row for row in program["literals"]
            if row["moduleId"] == required["moduleId"]
        }
        loops = required_literals.get("emitterloops")
        require(loops is not None and loops["variant"] == "F64"
                and type(loops["f64Value"]) is float
                and loops["f64Value"].is_integer()
                and timing["emitterLoopCount"] == int(loops["f64Value"]),
                f"emitter {emitter['emitterId']} loop literal mismatch")
        for path, value_key, policy_key, explicit_policy, default_policy, default_value in (
            (
                "emitterdelay", "emitterDelaySeconds", "emitterDelayPolicy",
                "EXPLICIT_REQUIRED_LITERAL", "RECONSTRUCTED_UE3_ZERO_DISTRIBUTION_DEFAULT_V1",
                0.0,
            ),
            (
                "emitterduration", "emitterDurationSeconds", "emitterDurationPolicy",
                "EXPLICIT_REQUIRED_LITERAL", "CURRENT_REVISION_CDO_RECONSTRUCTED_DEFAULT_V1",
                1.0,
            ),
        ):
            literal = required_literals.get(path)
            if literal is None:
                require(timing[value_key] == default_value
                        and timing[policy_key] == default_policy,
                        f"emitter {emitter['emitterId']} omitted {path} policy mismatch")
            else:
                require(literal["variant"] == "F64"
                        and type(literal["f64Value"]) is float
                        and timing[value_key] == literal["f64Value"]
                        and timing[policy_key] == explicit_policy,
                        f"emitter {emitter['emitterId']} explicit {path} mismatch")
        spawn_literals = {
            row["propertyPath"]: row for row in program["literals"]
            if row["moduleId"] == spawn["moduleId"]
            and row["propertyPath"].startswith("burstlist[")
        }
        indices = sorted({
            int(path.split("[", 1)[1].split("]", 1)[0]) for path in spawn_literals
        })
        require(indices == list(range(len(indices))),
                f"emitter {emitter['emitterId']} emitted burst indices mismatch")
        expected_bursts: list[dict[str, Any]] = []
        for burst_index in indices:
            prefix = f"burstlist[{burst_index}]"
            triplet = [
                spawn_literals.get(prefix + suffix)
                for suffix in (".count", ".countlow", ".time")
            ]
            require(all(item is not None and item["variant"] == "F64"
                        and type(item["f64Value"]) is float for item in triplet),
                    f"emitter {emitter['emitterId']} emitted burst triplet invalid")
            count, count_low, burst_time = [item["f64Value"] for item in triplet]
            require(count.is_integer() and count_low.is_integer() and burst_time >= 0.0,
                    f"emitter {emitter['emitterId']} emitted burst values invalid")
            maximum = int(count)
            minimum = maximum if count_low < 0.0 else int(count_low)
            require(0 <= minimum <= maximum,
                    f"emitter {emitter['emitterId']} emitted burst range invalid")
            if maximum == 0:
                continue
            projection = [
                {
                    "propertyPath": item["propertyPath"],
                    "variant": item["variant"],
                    "f64Value": item["f64Value"],
                }
                for item in triplet
            ]
            expected_bursts.append({
                "spawnModuleId": spawn["moduleId"],
                "burstIndex": burst_index,
                "timeSeconds": burst_time,
                "countMinimum": minimum,
                "countMaximum": maximum,
                "sourceLiteralProjectionSha256": canonical_sha256(projection),
            })
        require(timing["bursts"] == expected_bursts,
                f"emitter {emitter['emitterId']} typed burst projection mismatch")
        total_bursts += len(expected_bursts)
        random_input = {
            "policyId": emitter["random"]["policyId"],
            "sourceCandidateCanonicalSha256": source_candidate_sha,
            "emitterId": emitter["emitterId"],
        }
        require(emitter["random"]["seedDerivationInputSha256"]
                == canonical_sha256(random_input),
                f"emitter {emitter['emitterId']} random authority mismatch")
        attachment = emitter["actionCueAttachment"]
        if attachment["enabled"]:
            follow_attachment_count += 1
            require(attachment["follow"] is True and len(emitter["anchorRequests"]) == 1,
                    f"emitter {emitter['emitterId']} follow attachment mismatch")
            anchor = emitter["anchorRequests"][0]
            require(anchor["sourceAnchorSlotId"] == attachment["sourceAnchorSlotId"]
                    and anchor["runtimeAnchorSlotId"] == attachment["runtimeAnchorSlotId"]
                    and anchor["runtimeBoneName"] == attachment["runtimeBoneName"]
                    and anchor["socketLocalTransform"] == attachment["socketLocalTransform"]
                    and anchor["follow"] is True,
                    f"emitter {emitter['emitterId']} anchor projection mismatch")
        else:
            require(attachment["follow"] is False and emitter["anchorRequests"] == [],
                    f"emitter {emitter['emitterId']} root attachment mismatch")
    require(total_bursts == 31 and follow_attachment_count == 5
            and sum(row["operationalMaxParticles"] for row in emitters.values()) == 1291,
            "emitter operational denominator mismatch")
    reconstructed_modules = [
        module for module in modules.values()
        if module["selectionKind"] == "RECONSTRUCTED_HANDLER"
    ]
    require(len(reconstructed_modules) == 29
            and sum(len(module["capabilityNumericSamples"]) for module in reconstructed_modules)
            == 87
            and sum(bool(module["seedPolicyId"]) for module in reconstructed_modules) == 14
            and sum(bool(module["capabilityActionCueInputNames"])
                    for module in reconstructed_modules) == 5,
            "reconstructed capability denominator mismatch")
    require(canonical_sha256([
        {
            "moduleId": module["moduleId"],
            "capabilityNumericSamples": module["capabilityNumericSamples"],
        }
        for module in reconstructed_modules
    ]) == EXPECTED_CAPABILITY_SAMPLES_SHA256,
            "reconstructed capability numeric sample projection mismatch")
    require(len({module["approvalPolicyRowId"] for module in reconstructed_modules}) == 29,
            "approval Source row reverse coverage mismatch")
    for row in program["properties"]:
        require(row["consumptionDecision"]
                in {"BLOCKED", "READY_FOR_HANDLER", "VERIFIED_IRRELEVANT"}
                and row["sourceFidelity"]
                in {"SOURCE_EXACT", "CURRENT_REVISION_EVIDENCE"}
                and row["sourceExact"] is False,
                f"property {row['propertyId']} closed decision/fidelity mismatch")
        require(row["moduleId"] in modules, "property has unknown module owner")
        require(row["handlerRegistryId"] in handlers, "property has unknown handler")
        handler = handler_rows[row["handlerRegistryId"]]
        module = modules[row["moduleId"]]
        require(handler["handlerKind"] == "SOURCE_PROPERTY"
                and handler["exactSourceClass"] == module["exactSourceClass"]
                and handler["variant"] == row["propertyPath"],
                f"property {row['propertyId']} handler identity mismatch")
    for row in program["primitiveLeaves"]:
        require(row["consumptionDecision"]
                in {"READY_FOR_HANDLER", "VERIFIED_IRRELEVANT"}
                and row["sourceExact"] is False,
                f"primitive leaf {row['leafId']} closed decision mismatch")
        require(row["propertyId"] in properties and row["literalId"] in literals,
                "primitive leaf owner/reference mismatch")
        require(row["handlerRegistryId"] in handlers, "primitive leaf has unknown handler")
        prop = properties[row["propertyId"]]
        literal = literals[row["literalId"]]
        require(row["moduleId"] == prop["moduleId"] == literal["moduleId"],
                f"primitive leaf {row['leafId']} cross-module owner mismatch")
        require(literal["propertyId"] == row["propertyId"],
                f"primitive leaf {row['leafId']} literal property mismatch")
        require(row["propertyPath"] == literal["propertyPath"]
                and row["topLevelPropertyPath"] == prop["propertyPath"],
                f"primitive leaf {row['leafId']} path mismatch")
        handler = handler_rows[row["handlerRegistryId"]]
        require(handler["handlerKind"] == "SOURCE_PRIMITIVE"
                and handler["exactSourceClass"] == modules[row["moduleId"]]["exactSourceClass"]
                and handler["variant"] == row["topLevelPropertyPath"],
                f"primitive leaf {row['leafId']} handler identity mismatch")
    for row in program["literals"]:
        require(row["propertyId"] in properties, "literal has unknown property owner")
        require(row["moduleId"] == properties[row["propertyId"]]["moduleId"],
                f"literal {row['literalId']} cross-module property owner mismatch")
        variant = row["variant"]
        require(variant in LITERAL_VARIANTS, "unknown literal variant")
        populated = sum((
            row["boolValue"] is not None,
            row["f64Value"] is not None,
            row["enumStringValue"] != "",
        ))
        require(populated == 1, "literal must populate exactly one value slot")
    for row in program["distributions"]:
        require(row["propertyId"] in properties, "distribution has unknown property owner")
        require(row["evaluatorRegistryId"] in handlers,
                "distribution has unknown evaluator handler")
        prop = properties[row["propertyId"]]
        require(row["moduleId"] == prop["moduleId"],
                f"distribution {row['distributionId']} cross-module owner mismatch")
        require(row["distributionId"] in prop["semanticDistributionIds"]
                and row["payloadDistributionId"] in prop["payloadDistributionIds"],
                f"distribution {row['distributionId']} property coverage mismatch")
        handler = handler_rows[row["evaluatorRegistryId"]]
        require(handler["handlerKind"] == "DISTRIBUTION"
                and handler["exactSourceClass"] == row["sourceClass"],
                f"distribution {row['distributionId']} handler identity mismatch")
        if row["variant"] != "EF_MULTIPLY":
            require(handler["variant"] == row["variant"],
                    f"distribution {row['distributionId']} handler variant mismatch")
        else:
            require(handler["implementationId"] == row["capabilityImplementationId"]
                    and handler["variant"] != "",
                    f"EF distribution {row['distributionId']} capability handler mismatch")
    seed_rows = {row["seedPolicyId"]: row for row in program["seedPolicies"]}
    default_rows = {row["defaultId"]: row for row in program["implicitDefaults"]}
    for module in modules.values():
        if module["seedPolicyId"]:
            require(module["seedPolicyId"] in seed_rows
                    and seed_rows[module["seedPolicyId"]]["moduleId"] == module["moduleId"],
                    f"module {module['moduleId']} seed owner mismatch")
        owned_defaults = [
            row["defaultId"] for row in program["implicitDefaults"]
            if row["moduleId"] == module["moduleId"]
        ]
        require(module["implicitDefaultIds"] == owned_defaults,
                f"module {module['moduleId']} implicit default owner/order mismatch")
    require(
        {row["seedPolicyId"] for row in program["seedPolicies"]}
        == {row["seedPolicyId"] for row in modules.values() if row["seedPolicyId"]},
        "seed policy reverse coverage mismatch",
    )
    light_modules = [
        row for row in modules.values()
        if row["exactSourceClass"] == "efparticlemoduletypedatalight"
    ]
    require(len(light_modules) == 1, "expected one EF typed light module")
    require(all(row["moduleId"] == light_modules[0]["moduleId"]
                for row in program["pointLightFields"]),
            "PointLight field owner mismatch")
    families = {row["familyId"]: row for row in program["materialFamilies"]}
    recipes = {row["recipeId"]: row for row in program["materialRecipes"]}
    occurrences = {row["occurrenceId"]: row for row in program["materialOccurrences"]}
    for recipe in recipes.values():
        require(recipe["familyId"] in families, "material recipe has unknown family")
        require(recipe["evaluatorRegistryId"] in handlers,
                "material recipe has unknown evaluator")
        for section, field, primary in (
            (program["materialInputs"], "inputIds", "fieldId"),
            (program["materialStaticBindings"], "staticBindingIds", "fieldId"),
            (program["materialRenderBindings"], "renderBindingIds", "renderBindingId"),
        ):
            owned = [row for row in section if row["recipeId"] == recipe["recipeId"]]
            require(recipe[field] == [row[primary] for row in owned],
                    f"material recipe {recipe['recipeId']} {field} mismatch")
    for occurrence in occurrences.values():
        require(occurrence["emitterId"] in emitters, "material occurrence unknown emitter")
        require(occurrence["recipeId"] in recipes, "material occurrence unknown recipe")
        recipe = recipes[occurrence["recipeId"]]
        require(occurrence["familyId"] == recipe["familyId"]
                and occurrence["evaluatorRegistryId"] == recipe["evaluatorRegistryId"]
                and occurrence["bindingSha256"] == recipe["bindingSha256"],
                f"material occurrence {occurrence['occurrenceId']} recipe projection mismatch")
        require(occurrence["rendererType"] == emitters[occurrence["emitterId"]]["rendererType"],
                f"material occurrence {occurrence['occurrenceId']} renderer mismatch")
    policy_ids = ids_by_section["materialPolicyRows"]
    for row in program["materialStaticBindings"]:
        require(row["policyRowId"] in policy_ids, "static binding unknown policy")
    for row in program["materialRenderBindings"]:
        if row["policyRowId"]:
            require(row["policyRowId"] in policy_ids, "render binding unknown policy")
    for row in program["materialPolicyRows"]:
        require(row["recipeId"] in recipes, "Material policy unknown recipe")
        require(row["implementationRegistryId"] in handlers,
                "Material policy unknown implementation handler")
        require(set(row["materialOccurrenceIds"]).issubset(occurrences),
                "Material policy unknown occurrence")
        expected_occurrences = sorted(
            occurrence["occurrenceId"] for occurrence in occurrences.values()
            if occurrence["recipeId"] == row["recipeId"]
        )
        require(sorted(row["materialOccurrenceIds"]) == expected_occurrences,
                f"Material policy {row['policyRowId']} occurrence coverage mismatch")
        require(row["evidenceOwnerRecipeId"] == row["recipeId"],
                f"Material policy {row['policyRowId']} evidence owner mismatch")
        if row["domain"] == "RENDER_STATE":
            matches = [
                binding for binding in program["materialRenderBindings"]
                if binding["recipeId"] == row["recipeId"]
                and f"{row['recipeId']}:{binding['fieldName']}" == row["fieldId"]
            ]
            require(len(matches) == 1 and matches[0]["policyRowId"] == row["policyRowId"],
                    f"render policy {row['policyRowId']} binding reverse mismatch")
        elif row["domain"] == "STATIC_PERMUTATION":
            matches = [
                binding for binding in program["materialStaticBindings"]
                if binding["recipeId"] == row["recipeId"]
                and binding["fieldId"] == row["fieldId"]
            ]
            require(len(matches) == 1 and matches[0]["policyRowId"] == row["policyRowId"],
                    f"static policy {row['policyRowId']} binding reverse mismatch")
        else:
            matches = [
                binding for binding in program["materialInputs"]
                if binding["recipeId"] == row["recipeId"]
                and binding["fieldId"] == row["fieldId"]
                and binding["valueVariant"] == "TEXTURE_ID"
            ]
            require(len(matches) == 1,
                    f"sampler policy {row['policyRowId']} texture owner mismatch")
    material_inputs = {row["fieldId"]: row for row in program["materialInputs"]}
    material_policies = {row["policyRowId"]: row for row in program["materialPolicyRows"]}
    texture_binding_field_ids: set[str] = set()
    texture_binding_policy_ids: set[str] = set()
    for order, row in enumerate(program["materialTextureBindings"]):
        require(row["order"] == order,
                "Material texture binding order mismatch")
        field = material_inputs.get(row["materialInputFieldId"])
        policy_row = material_policies.get(row["samplerPolicyRowId"])
        require(field is not None and policy_row is not None,
                "Material texture binding owner missing")
        require(field["recipeId"] == row["recipeId"]
                and field["valueVariant"] == "TEXTURE_ID"
                and field["valueTextureId"] == row["logicalTexturePath"]
                and policy_row["domain"] == "SAMPLER_DESCRIPTOR"
                and policy_row["recipeId"] == row["recipeId"]
                and policy_row["fieldId"] == row["materialInputFieldId"]
                and policy_row["materialOccurrenceIds"] == row["materialOccurrenceIds"]
                and row["bindingId"]
                == row["materialInputFieldId"] + "::runtime-texture-binding",
                f"Material texture binding {row['bindingId']} compositional join mismatch")
        texture_binding_field_ids.add(row["materialInputFieldId"])
        texture_binding_policy_ids.add(row["samplerPolicyRowId"])
    expected_sampler_rows = [
        row for row in program["materialPolicyRows"]
        if row["domain"] == "SAMPLER_DESCRIPTOR"
    ]
    require(len(texture_binding_field_ids) == 72
            and texture_binding_field_ids == {row["fieldId"] for row in expected_sampler_rows}
            and len(texture_binding_policy_ids) == 72
            and texture_binding_policy_ids
            == {row["policyRowId"] for row in expected_sampler_rows},
            "Material texture binding reverse coverage mismatch")

    renderer_texture_projection: list[dict[str, Any]] = []
    renderer_resources_by_emitter: dict[str, list[dict[str, Any]]] = {
        emitter_id: [] for emitter_id in emitters
    }
    owner_slots: set[tuple[str, str]] = set()
    for order, row in enumerate(program["rendererTextureResources"]):
        require(row["order"] == order and row["emitterId"] in emitters,
                "renderer texture resource order/owner mismatch")
        emitter = emitters[row["emitterId"]]
        occurrence = occurrences.get(row["materialOccurrenceId"])
        require(emitter["rendererType"] != "LightParticle"
                and row["sourceNode"] == emitter["sourceNode"]
                and occurrence is not None
                and occurrence["emitterId"] == emitter["emitterId"],
                "renderer texture emitter/occurrence ownership mismatch")
        require(row["textureResourceId"]
                == row["emitterId"] + "::renderer-texture:" + row["slotId"],
                "renderer texture resource identity/owner mismatch")
        recipe = recipes[occurrence["recipeId"]]
        require(row["sourceMaterialPath"] == recipe["sourceMaterialPath"],
                "renderer texture Material recipe mismatch")
        projection = {
            "sourceNode": row["sourceNode"],
            "sourceMaterialPath": row["sourceMaterialPath"],
            "slotId": row["slotId"],
            "assetId": row["assetId"],
        }
        require(row["sourceResourceProjectionSha256"] == canonical_sha256(projection),
                "renderer texture source projection mismatch")
        owner_slot = (row["emitterId"], row["slotId"])
        require(owner_slot not in owner_slots,
                "duplicate renderer texture emitter/slot")
        owner_slots.add(owner_slot)
        renderer_resources_by_emitter[row["emitterId"]].append(row)
        renderer_texture_projection.append({
            key: value for key, value in row.items() if key != "rowSha256"
        })
    require(canonical_sha256(renderer_texture_projection)
            == EXPECTED_RENDERER_TEXTURE_RESOURCES_SHA256,
            "frozen renderer texture projection mismatch")
    for emitter in emitters.values():
        owned_ids = [row["textureResourceId"]
                     for row in renderer_resources_by_emitter[emitter["emitterId"]]]
        require(emitter["textureResourceIds"] == owned_ids,
                f"emitter {emitter['emitterId']} texture resource order mismatch")
        if emitter["rendererType"] == "LightParticle":
            require(owned_ids == [], "Light emitter carries renderer textures")
    require({slot: sum(row["slotId"] == slot
                       for row in program["rendererTextureResources"])
             for slot in sorted(RENDERER_TEXTURE_SLOTS)}
            == {"base": 19, "dissolve": 5, "emissive": 3, "mask": 15, "noise": 15},
            "renderer texture slot reverse denominator mismatch")
    carriers = {row["carrierId"]: row for row in program["geometryCarriers"]}
    uses = {row["geometryUseId"]: row for row in program["geometryUses"]}
    for row in uses.values():
        require(row["emitterId"] in emitters and row["moduleId"] in modules,
                "Geometry use owner mismatch")
        require(row["carrierId"] in carriers, "Geometry use unknown carrier")
        require(row["assetId"] == carriers[row["carrierId"]]["assetId"],
                "Geometry use asset mismatch")
        require(emitters[row["emitterId"]]["rendererType"] == "MeshParticle",
                "Geometry use owner is not MeshParticle")
        require(modules[row["moduleId"]]["emitterId"] == row["emitterId"]
                and modules[row["moduleId"]]["exactSourceClass"] == "particlemoduletypedatamesh",
                "Geometry use TypeDataMesh owner mismatch")
        require(type(row["sourceMeshPackageRef"]) is int
                and row["sourceMeshPackageRef"] < 0,
                "Geometry use source mesh package reference must be negative int")
        require(row["sizeSemantics"] == "DIMENSIONLESS_PARTICLE_SIZE"
                and row["preScaleApplication"] == "VERTEX_AND_BOUNDS_EXACTLY_ONCE_REQUIRED"
                and row["preScaleConsumed"] is False,
                "Geometry use pre-scale contract mismatch")
    require({row["carrierId"] for row in uses.values()} == set(carriers),
            "all seven Geometry carriers must be used")
    for carrier in carriers.values():
        expected_tuple = {
            "formatVersion": carrier["formatVersion"],
            "channelMask": carrier["channelMask"],
            "evidenceFlags": carrier["evidenceFlags"],
            "geometryPreScale": carrier["geometryPreScale"],
            "geometryPreScaleF32Hex": carrier["geometryPreScaleF32Hex"],
            "payloadSha256": carrier["payloadSha256"],
            "provenanceSha256": carrier["provenanceSha256"],
            "provenanceRole": carrier["provenanceRole"],
            "metadataIdentitySha256": carrier["metadataIdentitySha256"],
            "submeshes": carrier["submeshes"],
        }
        require(canonical_sha256(expected_tuple) == carrier["expectedTupleSha256"],
                f"Geometry carrier {carrier['carrierId']} expected tuple digest mismatch")
        require(carrier["metadataIdentitySha256"] == carrier["provenanceSha256"],
                f"Geometry carrier {carrier['carrierId']} metadata/provenance mismatch")
        cache_payload = {
            "assetId": carrier["assetId"],
            "payloadSha256": carrier["payloadSha256"],
            "provenanceSha256": carrier["provenanceSha256"],
            "geometryPreScaleF32Hex": carrier["geometryPreScaleF32Hex"],
            "channelMask": carrier["channelMask"],
            "evidenceFlags": carrier["evidenceFlags"],
            "submeshes": carrier["submeshes"],
        }
        require(canonical_sha256(cache_payload) == carrier["cacheIdentitySha256"],
                f"Geometry carrier {carrier['carrierId']} cache identity mismatch")
    for emitter in emitters.values():
        if emitter["rendererType"] == "LightParticle":
            require(emitter["materialOccurrenceId"] is None,
                    "Light emitter material binding must be null")
        else:
            require(emitter["materialOccurrenceId"] in occurrences,
                    "non-Light emitter lacks direct Material occurrence")
            require(occurrences[emitter["materialOccurrenceId"]]["emitterId"] == emitter["emitterId"],
                    "emitter Material direct binding reverse mismatch")
        if emitter["rendererType"] == "MeshParticle":
            require(emitter["geometryUseId"] in uses, "Mesh emitter lacks Geometry use")
            require(uses[emitter["geometryUseId"]]["emitterId"] == emitter["emitterId"],
                    "emitter Geometry direct binding reverse mismatch")
        else:
            require(emitter["geometryUseId"] is None,
                    "non-Mesh emitter Geometry binding must be null")


def recompute_summary(program: dict[str, Any]) -> dict[str, Any]:
    return {
        "inputArtifactCount": len(program["inputArtifacts"]),
        "handlerRegistryCount": len(program["handlerRegistry"]),
        "emitterCount": len(program["emitters"]),
        "actionScheduleCount": len(program["actionSchedules"]),
        "moduleCount": len(program["modules"]),
        "moduleSelectionCounts": {
            variant: sum(row["selectionKind"] == variant for row in program["modules"])
            for variant in ("SOURCE_HANDLER", "RECONSTRUCTED_HANDLER")
        },
        "propertyCount": len(program["properties"]),
        "primitiveLeafCount": len(program["primitiveLeaves"]),
        "literalCount": len(program["literals"]),
        "distributionCount": len(program["distributions"]),
        "distributionVariantCounts": {
            variant: sum(row["variant"] == variant for row in program["distributions"])
            for variant in ("INLINE", "FLOAT_PARAMETER", "VECTOR_PARAMETER", "FLOAT_CURVE", "EF_MULTIPLY")
        },
        "seedPolicyCount": len(program["seedPolicies"]),
        "implicitDefaultCount": len(program["implicitDefaults"]),
        "pointLightFieldCount": len(program["pointLightFields"]),
        "materialFamilyCount": len(program["materialFamilies"]),
        "materialRecipeCount": len(program["materialRecipes"]),
        "materialInputCount": len(program["materialInputs"]),
        "materialStaticBindingCount": len(program["materialStaticBindings"]),
        "materialRenderBindingCount": len(program["materialRenderBindings"]),
        "materialOccurrenceCount": len(program["materialOccurrences"]),
        "materialPolicyCount": len(program["materialPolicyRows"]),
        "materialPolicyDomainCounts": {
            domain: sum(row["domain"] == domain for row in program["materialPolicyRows"])
            for domain in ("RENDER_STATE", "STATIC_PERMUTATION", "SAMPLER_DESCRIPTOR")
        },
        "materialD3dDescriptorCount": sum(
            row["d3dDescriptorOracle"] is not None
            for row in program["materialPolicyRows"]
        ),
        "materialD3dDescriptorProjectionSha256": canonical_sha256([
            row["d3dDescriptorOracle"] for row in program["materialPolicyRows"]
            if row["d3dDescriptorOracle"] is not None
        ]),
        "materialSrvDescriptorCount": sum(
            row["d3dSrvOracle"] is not None for row in program["materialPolicyRows"]
        ),
        "materialSrvDescriptorProjectionSha256": canonical_sha256([
            row["d3dSrvOracle"] for row in program["materialPolicyRows"]
            if row["d3dSrvOracle"] is not None
        ]),
        "materialTextureBindingCount": len(program["materialTextureBindings"]),
        "resolvedMaterialTextureBindingCount": sum(
            row["resolutionStatus"] == "RESOLVED_EXACT_RUNTIME_ASSET"
            for row in program["materialTextureBindings"]
        ),
        "rendererTextureResourceCount": len(program["rendererTextureResources"]),
        "rendererTextureSlotCounts": {
            slot: sum(row["slotId"] == slot for row in program["rendererTextureResources"])
            for slot in sorted(RENDERER_TEXTURE_SLOTS)
        },
        "geometryCarrierCount": len(program["geometryCarriers"]),
        "geometryUseCount": len(program["geometryUses"]),
        "unknownHandlerCount": 0,
        "ownerlessRowCount": 0,
        "duplicateIdCount": 0,
        "sourceExactRowCount": sum(
            row.get("sourceExact") is True
            for section in SECTION_NAMES for row in program[section]
        ),
        "runtimeExecution": False,
        "product": False,
    }


def validate_program(program: dict[str, Any], expected: dict[str, Any] | None = None) -> None:
    assert_finite_tree(program, "runtime program")
    require_keys(program, ROOT_KEYS, "runtime program root")
    require(program["schema"] == SCHEMA, "runtime program schema mismatch")
    require(type(program["formatVersion"]) is int and program["formatVersion"] == FORMAT_VERSION,
            "runtime program formatVersion must be integer 1")
    require(program["programId"] == PROGRAM_ID and program["programVersion"] == PROGRAM_VERSION,
            "runtime program identity mismatch")
    require_keys(program["target"], TARGET_KEYS, "runtime program target")
    target_projection = dict(program["target"])
    target_sha = target_projection.pop("identityRouteSha256")
    require(target_projection == {
        "characterClass": "ARTIST", "skillId": 31470, "inputSlot": "F",
        "policyTargetId": "effect.artist.skill.31470.f",
        "sourceCandidateId": (
            "effect.artist.skill.31470.native-v14.source-contract-candidate"
        ),
        "geometryBindingAssetId": "effect.artist.skill.31470",
        "runtimeCatalogAssetId": "effect.artist.skill.31470",
        "identityRouteId": "ARTIST_31470_F_RECONSTRUCTED_ROUTE_V1",
    } and canonical_sha256(target_projection) == target_sha,
            "runtime program target value or route digest mismatch")
    route = program["policyRouteBinding"]
    require_keys(route, POLICY_ROUTE_BINDING_KEYS, "policyRouteBinding")
    unsigned_route = dict(route)
    route_sha = unsigned_route.pop("bindingSha256")
    require(canonical_sha256(unsigned_route) == route_sha,
            "policyRouteBinding digest mismatch")
    require(route["approvalPolicyId"] == "artist.31470.f.reconstructed-approved-v1"
            and type(route["approvalPolicyVersion"]) is int
            and route["approvalPolicyVersion"] == 1,
            "policyRouteBinding approval identity mismatch")
    require(route["sourceExecutionFamilyCount"] == 7
            and route["sourcePolicyRowCount"] == 29
            and route["materialExecutionFamilyCount"] == 8
            and route["materialPolicyRowCount"] == 255
            and route["materialArithmeticRowCount"] == 23
            and route["geometryPolicyRowCount"] == 7,
            "policyRouteBinding denominator mismatch")
    require(program["programRole"] == PROGRAM_ROLE, "runtime program role mismatch")
    for section in SECTION_NAMES:
        rows = program[section]
        require(type(rows) is list, f"{section}: section must be an array")
        require(len(rows) <= SECTION_MAX_COUNTS[section], f"{section}: section exceeds hard limit")
        if section in EXPECTED_COUNTS:
            require(len(rows) == EXPECTED_COUNTS[section],
                    f"{section}: expected {EXPECTED_COUNTS[section]} rows, got {len(rows)}")
        for index, row in enumerate(rows):
            require(type(row) is dict, f"{section}[{index}]: row must be object")
            require_keys(row, ROW_KEYS[section], f"{section}[{index}]")
            unsigned = dict(row)
            expected_row_sha = unsigned.pop("rowSha256")
            require(canonical_sha256(unsigned) == expected_row_sha,
                    f"{section}[{index}]: row digest mismatch")
    require(
        program["sectionDigests"]
        == [section_digest(name, program[name]) for name in SECTION_NAMES],
        "section digest table mismatch",
    )
    require_keys(program["admission"], ADMISSION_KEYS, "runtime program admission")
    program_sections = {
        section: program[section] for section in SECTION_NAMES
    }
    require_keys(program["blockerOwnership"], BLOCKER_OWNERSHIP_KEYS,
                 "blocker ownership contract")
    expected_blocker_ownership = build_blocker_ownership_contract(program_sections)
    require(program["blockerOwnership"] == expected_blocker_ownership
            and program["blockerOwnership"]["projectionSha256"]
            == EXPECTED_BLOCKER_OWNERSHIP_PROJECTION_SHA256,
            "blocker ownership does not match frozen owner/path/token projection")
    expected_blockers = derive_program_blockers_from_sections(program_sections)
    require(program["blockerUnion"] == expected_blockers,
            "blockerUnion does not equal exact owned-row/global-gate derivation")
    require(program["blockerUnion"] == sorted(set(program["blockerUnion"])),
            "blockerUnion must be sorted and unique")
    require(program["admission"]["blockers"] == program["blockerUnion"],
            "admission blockers must equal blockerUnion")
    require(all(program["admission"][key] is True for key in (
        "artifactBinding", "policyRoute", "sourceHandlerSelection",
        "distributionEvaluatorSelection", "materialPolicySelection", "geometryBinding",
    )), "offline join gates must be true")
    require(program["admission"]["sourceExact"] is False,
            "reconstructed program cannot claim source exact")
    require(program["admission"]["runtimeExecution"] is False,
            "R2 program cannot grant runtime execution")
    require(program["admission"]["product"] is False,
            "R2 program cannot grant Product admission")
    require("R3_TYPED_EXECUTOR_NOT_COMPLETE" in program["blockerUnion"],
            "R3 blocker must remain at R2")
    require("DECAL_YAW_ONLY_CDO_DEFAULT_CAPABILITY_OUTPUT_CONFLICT"
            not in program["blockerUnion"],
            "resolved Decal yaw conflict blocker must be discharged")
    require("R2_TYPED_MATERIALIZATION_NOT_COMPLETE" not in program["blockerUnion"],
            "completed R2 blocker must be discharged")
    validate_policy_route_binding(program)
    validate_material_evaluator_oracle_contract(program)
    validate_nested_shapes(program)
    validate_owner_order(program)
    require(program["summary"] == recompute_summary(program), "summary recomputation mismatch")
    require(program["summary"]["distributionVariantCounts"] == {
        "INLINE": 612, "FLOAT_PARAMETER": 8, "VECTOR_PARAMETER": 5,
        "FLOAT_CURVE": 1, "EF_MULTIPLY": 3,
    }, "distribution variant denominator mismatch")
    require(program["summary"]["moduleSelectionCounts"] == {
        "SOURCE_HANDLER": 370, "RECONSTRUCTED_HANDLER": 29,
    }, "module handler selection denominator mismatch")
    require(program["summary"]["materialPolicyDomainCounts"] == {
        "RENDER_STATE": 89, "STATIC_PERMUTATION": 94, "SAMPLER_DESCRIPTOR": 72,
    }, "Material policy denominator mismatch")
    require(program["summary"]["materialD3dDescriptorCount"] == 107
            and program["summary"]["materialD3dDescriptorProjectionSha256"]
            == EXPECTED_D3D_DESCRIPTOR_PROJECTION_SHA256,
            "Material D3D descriptor projection mismatch")
    require(program["summary"]["materialSrvDescriptorCount"] == 72
            and program["summary"]["materialSrvDescriptorProjectionSha256"]
            == EXPECTED_D3D_SRV_PROJECTION_SHA256,
            "Material D3D SRV projection mismatch")
    require(program["summary"]["sourceExactRowCount"] == 0,
            "reconstructed program must contain zero sourceExact rows")
    unsigned_program = copy.deepcopy(program)
    expected_program_sha = unsigned_program.pop("programSha256")
    require(canonical_sha256(unsigned_program) == expected_program_sha,
            "runtime program self digest mismatch")
    if expected is not None:
        require(canonical_json_bytes(program) == canonical_json_bytes(expected),
                "runtime program differs from frozen authority reconstruction")


def output_bytes(program: dict[str, Any]) -> bytes:
    raw = (json.dumps(program, ensure_ascii=False, indent=2, allow_nan=False) + "\n").encode("utf-8")
    require(len(raw) <= MAX_PROGRAM_BYTES,
            f"runtime program exceeds {MAX_PROGRAM_BYTES} byte hard limit")
    require(not raw.startswith(b"\xef\xbb\xbf") and b"\r" not in raw,
            "runtime program output must be UTF-8 without BOM and LF-only")
    return raw


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--print-schema", action="store_true")
    args = parser.parse_args(argv)
    if args.print_schema:
        print(json.dumps({
            "rootKeys": ROOT_KEYS,
            "rowKeys": ROW_KEYS,
            "sectionMaxCounts": SECTION_MAX_COUNTS,
            "maxProgramBytes": MAX_PROGRAM_BYTES,
        }, indent=2))
        return 0
    program = build_program()
    validate_program(program)
    raw = output_bytes(program)
    if args.check:
        require(args.output.is_file(), f"candidate missing: {args.output}")
        actual_raw = args.output.read_bytes()
        require(actual_raw == raw, "candidate is stale or not canonical LF UTF-8")
        actual = load_json_bytes(actual_raw, str(args.output))
        validate_program(actual, expected=program)
        print(
            "Artist 31470 reconstructed runtime program CHECK PASS "
            f"bytes={len(raw)} sha={program['programSha256']} "
            "runtime=false product=false"
        )
        return 0
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(raw)
    print(
        "Artist 31470 reconstructed runtime program BUILT "
        f"bytes={len(raw)} sha={program['programSha256']} "
        "runtime=false product=false"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except ContractError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        raise SystemExit(1)

#!/usr/bin/env python3

from __future__ import annotations

import copy
import hashlib
import json
import math
import re
import unittest
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
DATA_ROOT = REPOSITORY_ROOT / "Data"
AUTHORED_ROOT = DATA_ROOT / "Effects" / "Authored"
PRODUCT_CATALOG_PATH = DATA_ROOT / "Effects" / "EffectCatalog.json"
AUDITION_CATALOG_PATH = DATA_ROOT / "Effects" / "EffectAuditionCatalog.json"
CANDIDATE_ID = (
    "effect.dimensionmaster.skill.2050230.mirror-particle-canary.unified"
)
CANDIDATE_PATH = AUTHORED_ROOT / f"{CANDIDATE_ID}.effect.json"
SOURCE_ID = "effect.dimensionmaster.skill.2050230.unified"
SOURCE_PATH = AUTHORED_ROOT / f"{SOURCE_ID}.effect.json"
SOURCE_RAW_SHA256 = (
    "afab680bd36b4efcc4baf654c4848a1f3571a29cbc338b0ed58fec940de60e09"
)
MODEL_ASSET = "Effect/DimensionMaster/Meshes/fx_m_glass_01.wmodel"
PATTERN_ASSET = (
    "Effect/DimensionMaster/Textures/FX_TEX_HIGH_03/"
    "fx_h_brokenglass_02_1.dds"
)
SCALAR_BOUNDS = [
    ("CoverageGain", 0.0, 4.0),
    ("BodyOpacity", 0.0, 1.0),
    ("FresnelPower", 0.25, 16.0),
    ("EdgeGain", 0.0, 8.0),
    ("CrackGain", 0.0, 4.0),
    ("RefractionStrength", -0.025, 0.025),
    ("DistortionClamp", 0.0, 0.025),
    ("EmissionGain", 0.0, 8.0),
]
VECTOR_BOUNDS = [
    ("BodyTintLinear", 4.0),
    ("EdgeTintLinear", 8.0),
]


def load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def in_range(value: object, minimum: float, maximum: float) -> bool:
    return (
        isinstance(value, (int, float))
        and not isinstance(value, bool)
        and math.isfinite(float(value))
        and minimum <= float(value) <= maximum
    )


def validate_glass_document(document: dict) -> None:
    if (
        document.get("schema") != "lostark.effect-authoring"
        or document.get("version") != 13
        or document.get("effectAssetId") != CANDIDATE_ID
    ):
        raise ValueError("glass identity")
    elements = document.get("elements")
    if not isinstance(elements, list) or len(elements) != 1:
        raise ValueError("glass element count")
    element = elements[0]
    if (
        element.get("id") != "project-tuned.glass-mirror-shards.2050230.01"
        or element.get("sourceNode")
        != "authored-copy:geometry-oracle.fx_m_glass_01"
        or element.get("kind") != "particle"
        or element.get("visible") is not True
    ):
        raise ValueError("glass carrier identity")
    if element.get("resources") != [
        {"slotId": "meshModel", "assetId": MODEL_ASSET},
        {"slotId": "base", "assetId": PATTERN_ASSET},
    ]:
        raise ValueError("glass resources")

    material = element["material"]
    execution = material["execution"]
    if (
        material.get("templateId") != "effect.standard"
        or material.get("sourceMaterialPath") != ""
        or material.get("renderProfile") != "alpha_two_sided_depth_read"
        or material.get("sourceProfile") != {"enabled": False}
        or execution.get("enabled") is not True
        or execution.get("fidelity") != "PROJECT_TUNED_APPROX"
        or execution.get("version") != 1
        or execution.get("backend") != "runtimeMaterialV2"
        or execution.get("opcode") != 1004
        or execution.get("passIndex") != 1
        or execution.get("renderState")
        != {
            "rasterizer": "RS_Cull_None",
            "depthStencil": "DSS_ReadOnly",
            "blend": "BS_EffectAlpha",
            "stencilReference": 0,
        }
    ):
        raise ValueError("glass packet identity")
    expected_lane = {
        "laneId": "lane.0",
        "role": "broken_glass_pattern",
        "assetId": PATTERN_ASSET,
        "textureRegister": 0,
        "samplerRegister": 5,
        "sourceChannel": "RGBA",
        "colorSpace": "linear",
        "sampler": {
            "filter": "linear",
            "addressU": "wrap",
            "addressV": "wrap",
            "addressW": "wrap",
            "mipLodBias": 0,
            "maxAnisotropy": 1,
            "comparison": "never",
            "borderColor": [0, 0, 0, 0],
            "minLod": 0,
            "maxLod": 3.40282347e38,
        },
    }
    if execution.get("textureLanes") != [expected_lane]:
        raise ValueError("glass texture lane")
    expected_masks = {
        "textureLaneCount": 1,
        "textureMask": 1,
        "dynamicConsumedMask": 0,
        "dynamicSuppressedMask": 15,
        "particleColorPolicy": 2,
        "particleColorConsumedMask": 8,
        "particleColorSuppressedMask": 7,
        "scalarCount": 8,
        "vectorCount": 2,
        "inputCount": 8,
        "inputConsumedMask": [255, 0],
        "inputSuppressedMask": [0, 0],
        "vectorComponentConsumedMask": [15, 15, 0],
        "vectorComponentSuppressedMask": [0, 0, 0],
        "staticInputCount": 0,
        "staticSelectedMask": 0,
        "staticConsumedMask": 0,
        "staticSuppressedMask": 0,
        "renderInputCount": 6,
        "renderConsumedMask": 47,
        "renderSuppressedMask": 16,
    }
    if any(execution.get(key) != value for key, value in expected_masks.items()):
        raise ValueError("glass packet masks")

    scalars = execution.get("scalars")
    if not isinstance(scalars, list) or len(scalars) != len(SCALAR_BOUNDS):
        raise ValueError("glass scalars")
    for index, (name, minimum, maximum) in enumerate(SCALAR_BOUNDS):
        row = scalars[index]
        if (
            row.get("name") != name
            or row.get("packedIndex") != index
            or not in_range(row.get("value"), minimum, maximum)
        ):
            raise ValueError("glass scalars")

    vectors = execution.get("vectors")
    if not isinstance(vectors, list) or len(vectors) != len(VECTOR_BOUNDS):
        raise ValueError("glass vectors")
    for index, (name, rgb_maximum) in enumerate(VECTOR_BOUNDS):
        row = vectors[index]
        value = row.get("value")
        if (
            row.get("name") != name
            or row.get("packedIndex") != index
            or not isinstance(value, list)
            or len(value) != 4
            or not all(in_range(channel, 0.0, rgb_maximum) for channel in value[:3])
            or not in_range(value[3], 0.0, 1.0)
        ):
            raise ValueError("glass vectors")

    detail = element["detail"]
    particle = detail["particle"]
    linear_lerp = detail["linearLerp"]
    if (
        detail["mesh"].get("useModelMaterial") is not False
        or not in_range(detail["mesh"].get("modelPreScale"), 0.001, 0.1)
        or detail["color"].get("offset") != [0, 0, 0, 0]
        or linear_lerp.get("colorOffset") is not False
        or linear_lerp.get("endColorOffset") != [0, 0, 0, 0]
        or linear_lerp.get("colorMultiply") is not False
        or linear_lerp.get("endColorMultiply") != [1, 1, 1, 1]
        or particle.get("maxParticles") != 1
        or particle.get("spawnRatePerSecond") != 0
        or particle.get("burstCount") != 1
        or particle.get("randomSeed") != 2050231
        or particle.get("localSpace") is not True
        or particle.get("billboard") is not False
        or element.get("sourceRecipe") != {"enabled": False}
        or element.get("sourcePresentation") != {"enabled": False}
    ):
        raise ValueError("glass rigid-cluster topology")
    for key in ("startSize", "endSize"):
        values = particle.get(key)
        if (
            not isinstance(values, list)
            or len(values) != 2
            or not all(in_range(value, 0.01, 4.0) for value in values)
        ):
            raise ValueError("glass motion bounds")
    revolution = detail["transform"].get("revolutionDegreesPerSecond")
    if (
        not isinstance(revolution, list)
        or len(revolution) != 3
        or not all(in_range(value, -720.0, 720.0) for value in revolution)
    ):
        raise ValueError("glass motion bounds")


class DimensionMasterMirrorParticleToolCanaryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.product_catalog = load_json(PRODUCT_CATALOG_PATH)
        cls.audition_catalog = load_json(AUDITION_CATALOG_PATH)
        cls.candidate = load_json(CANDIDATE_PATH)

    def test_catalog_row_is_audition_only_and_source_pinned(self) -> None:
        rows = [
            row
            for row in self.audition_catalog["effects"]
            if row.get("effectAssetId") == CANDIDATE_ID
        ]
        self.assertEqual(
            [
                {
                    "effectAssetId": CANDIDATE_ID,
                    "payloadKind": "DIRECT_AUTHORED_DOCUMENT",
                    "authoringPath": f"Effects/Authored/{CANDIDATE_ID}.effect.json",
                    "runtimeAdmission": "REGISTRY_BOUND_AUDITION_ONLY",
                    "fidelityClass": "PROJECT_TUNED_APPROX",
                    "sourceEffectAssetId": SOURCE_ID,
                    "sourceDocumentRawSha256": SOURCE_RAW_SHA256,
                }
            ],
            rows,
        )
        self.assertNotIn(
            CANDIDATE_ID,
            {
                row.get("effectAssetId")
                for row in self.product_catalog["effects"]
                if isinstance(row, dict)
            },
        )
        self.assertEqual(
            SOURCE_RAW_SHA256, hashlib.sha256(SOURCE_PATH.read_bytes()).hexdigest()
        )

    def test_candidate_is_not_referenced_by_product_data(self) -> None:
        unexpected: list[str] = []
        excluded = {AUDITION_CATALOG_PATH.resolve(), CANDIDATE_PATH.resolve()}
        for path in DATA_ROOT.rglob("*"):
            if (
                not path.is_file()
                or path.resolve() in excluded
                or path.suffix.lower() not in {".json", ".animevents"}
            ):
                continue
            if CANDIDATE_ID in path.read_text(encoding="utf-8"):
                unexpected.append(path.relative_to(REPOSITORY_ROOT).as_posix())
        self.assertEqual([], unexpected)

    def test_document_is_one_tunable_local_mesh_cluster(self) -> None:
        validate_glass_document(self.candidate)

    def test_opcode_1004_is_occurrence_scoped_and_runtime_wired(self) -> None:
        opcode_pattern = re.compile(r'"opcode"\s*:\s*1004\b')
        occurrences = [
            path.name
            for path in AUTHORED_ROOT.glob("*.effect.json")
            if opcode_pattern.search(path.read_text(encoding="utf-8"))
        ]
        self.assertEqual([CANDIDATE_PATH.name], occurrences)

        codec = (
            REPOSITORY_ROOT / "Client" / "Private" / "Effect_DocumentCodec.cpp"
        ).read_text(encoding="utf-8-sig")
        renderer = (
            REPOSITORY_ROOT
            / "Client"
            / "Private"
            / "Effect_DocumentRenderer.cpp"
        ).read_text(encoding="utf-8-sig")
        families = (
            REPOSITORY_ROOT
            / "Client"
            / "Bin"
            / "ShaderFiles"
            / "Shader_EffectUe3MaterialFamilies.hlsli"
        ).read_text(encoding="utf-8")
        mesh_shader = (
            REPOSITORY_ROOT
            / "Client"
            / "Bin"
            / "ShaderFiles"
            / "Shader_VtxEffectMeshPreview.hlsl"
        ).read_text(encoding="utf-8")
        tool = (
            REPOSITORY_ROOT / "Client" / "Private" / "Effect_Tool.cpp"
        ).read_text(encoding="utf-8-sig")
        self.assertEqual(2, codec.count("iOpcode == 1004u"))
        self.assertIn("Validate_DimensionMasterGlassMirrorMeshExecution", renderer)
        self.assertIn(
            "Validate_DimensionMasterGlassMirrorDocumentOccurrence", renderer
        )
        self.assertIn(
            "Validate_DimensionMasterProjectTunedDocumentExecution", renderer
        )
        self.assertIn(CANDIDATE_ID, renderer)
        self.assertIn(
            "RUNTIME_MATERIAL_V2_PROJECT_TUNED_GLASS_MESH_V1 = 1004u", families
        )
        self.assertIn("EffectProjectTunedGlassMeshV1PacketIsValid", families)
        self.assertIn("unassociatedTint", families)
        self.assertIn("Shade_EffectProjectTunedGlassMeshV1", mesh_shader)
        self.assertIn("pixelViewNormal", mesh_shader)
        for label in (
            "Glass Tint",
            "Glass Coverage / Edge",
            "Glass Refraction",
            "Glass Emission",
        ):
            self.assertIn(label, tool)

    def test_cpp_product_admission_and_source_freshness_are_fail_closed(self) -> None:
        catalog_cpp = (
            REPOSITORY_ROOT / "Client" / "Private" / "Effect_Catalog.cpp"
        ).read_text(encoding="utf-8-sig")
        source_index_h = (
            REPOSITORY_ROOT
            / "Client"
            / "Public"
            / "Effect_DirectAuthoredSourceIndex.h"
        ).read_text(encoding="utf-8-sig")
        source_index_cpp = (
            REPOSITORY_ROOT
            / "Client"
            / "Private"
            / "Effect_DirectAuthoredSourceIndex.cpp"
        ).read_text(encoding="utf-8-sig")
        tool = (
            REPOSITORY_ROOT / "Client" / "Private" / "Effect_Tool.cpp"
        ).read_text(encoding="utf-8-sig")
        build_gate = (
            REPOSITORY_ROOT / "Tools" / "Build" / "Invoke-BuildAndRegression.ps1"
        ).read_text(encoding="utf-8-sig")
        valtan_admission = (
            REPOSITORY_ROOT
            / "Client"
            / "Private"
            / "ValtanPresentationGenerationAdmission.cpp"
        ).read_text(encoding="utf-8-sig")

        self.assertNotIn("AuditionSourcePins.push_back", catalog_cpp)
        self.assertIn(
            "EffectCatalog.json cannot contain audition admission; use EffectAuditionCatalog.json",
            catalog_cpp,
        )
        self.assertNotIn(CANDIDATE_ID, PRODUCT_CATALOG_PATH.read_text(encoding="utf-8"))
        self.assertIn(CANDIDATE_ID, AUDITION_CATALOG_PATH.read_text(encoding="utf-8"))
        self.assertNotIn("EffectAuditionCatalog.json", valtan_admission)
        self.assertIn("bRegistryBoundAuditionOnly", source_index_h)
        self.assertIn("bAuditionSourceFreshnessValid", source_index_h)
        self.assertIn("Compute_Sha256Hex(SourceBytes)", source_index_cpp)
        self.assertIn(
            "Validate_RegistryBoundAuditionCatalogProvenanceFresh",
            source_index_cpp,
        )
        self.assertIn("EffectAuditionCatalog.json", source_index_cpp)
        self.assertIn(
            "Validate_RegistryBoundAuditionSourceFreshness", tool
        )
        self.assertIn(
            "Validate_ActiveRegistryBoundAuditionFreshness", tool
        )
        self.assertIn(
            "metadata disappeared or was reclassified", tool
        )
        active_freshness = tool.split(
            "bool_t Client::CEffect_Tool::Validate_ActiveRegistryBoundAuditionFreshness",
            maxsplit=1,
        )[1]
        active_freshness = active_freshness.split(
            "bool_t Client::CEffect_Tool::Is_UnifiedEffectActive",
            maxsplit=1,
        )[0]
        self.assertIn(
            "Validate_RegistryBoundAuditionCatalogProvenanceFresh",
            active_freshness,
        )
        load_stage = tool.split(
            "bool_t Client::CEffect_Tool::Try_LoadDocumentPathStaged",
            maxsplit=1,
        )[1]
        load_stage = load_stage.split(
            "bool_t Client::CEffect_Tool::Execute_PendingDocumentLoad",
            maxsplit=1,
        )[0]
        self.assertIn(
            "Validate_RegistryBoundAuditionCatalogProvenanceFresh",
            load_stage,
        )
        self.assertIn(
            "Open rejected: registry-bound audition source freshness failed",
            load_stage,
        )
        self.assertIn(
            "Carrier Color is locked for opcode 1003/1004", tool
        )
        self.assertIn(
            "Save rejected: registry-bound audition source freshness failed", tool
        )
        stage_preview = tool.split(
            "bool_t Client::CEffect_Tool::Stage_WorldPreview(\n"
            "\tconst EFFECT_DOCUMENT_DESC& Document,\n"
            "\tconst bool_t bAllowReadOnlySourceProjection)",
            maxsplit=1,
        )[1]
        stage_preview = stage_preview.split(
            "EFFECT_DOCUMENT_DESC Client::CEffect_Tool::Build_PreviewDocument",
            maxsplit=1,
        )[0]
        self.assertIn(
            "Validate_ActiveRegistryBoundAuditionFreshness", stage_preview
        )
        self.assertIn(
            "Preview staging rejected: registry-bound audition source freshness failed",
            stage_preview,
        )
        self.assertIn("Release_WorldPreview(true)", stage_preview)
        self.assertIn(
            "Registry-bound audition candidates cannot be saved under another Effect ID",
            tool,
        )
        self.assertIn(
            "Save As rejected: registry-bound audition source freshness failed",
            tool,
        )
        self.assertIn(
            "Tools.EffectPipeline.test_dimensionmaster_2050230_mirror_particle_tool_canary",
            build_gate,
        )
        self.assertIn(
            "Tools.EffectPipeline.test_dimensionmaster_2050230_glass_water_visual_canary",
            build_gate,
        )

    def test_semantic_values_are_mutable_only_inside_bounds(self) -> None:
        in_range_document = copy.deepcopy(self.candidate)
        in_range_document["elements"][0]["material"]["execution"]["scalars"][0][
            "value"
        ] = 3.75
        in_range_document["elements"][0]["material"]["execution"]["vectors"][1][
            "value"
        ] = [7.5, 6.5, 5.5, 0.7]
        validate_glass_document(in_range_document)

        out_of_range = copy.deepcopy(self.candidate)
        out_of_range["elements"][0]["material"]["execution"]["scalars"][6][
            "value"
        ] = 0.026
        with self.assertRaisesRegex(ValueError, "glass scalars"):
            validate_glass_document(out_of_range)

        wrong_resource = copy.deepcopy(self.candidate)
        wrong_resource["elements"][0]["resources"][0]["assetId"] = (
            "Effect/DimensionMaster/Meshes/not-the-glass-oracle.wmodel"
        )
        with self.assertRaisesRegex(ValueError, "glass resources"):
            validate_glass_document(wrong_resource)


if __name__ == "__main__":
    unittest.main()

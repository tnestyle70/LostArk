#!/usr/bin/env python3

from __future__ import annotations

import copy
import json
import math
import re
import unittest
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
DATA_ROOT = REPOSITORY_ROOT / "Data"
AUTHORED_ROOT = DATA_ROOT / "Effects" / "Authored"
CATALOG_PATH = DATA_ROOT / "Effects" / "EffectCatalog.json"
AUDITION_CATALOG_PATH = DATA_ROOT / "Effects" / "EffectAuditionCatalog.json"
EVENT_PATH = (
    DATA_ROOT
    / "Animation"
    / "Authored"
    / "DimensionMaster"
    / "DimensionMaster.animevents"
)
PRODUCT_ID = "effect.dimensionmaster.skill.2050230.single-glass-canary"
PRODUCT_PATH = AUTHORED_ROOT / f"{PRODUCT_ID}.effect.json"
AUDITION_ID = (
    "effect.dimensionmaster.skill.2050230.mirror-particle-canary.unified"
)
ELEMENT_ID = "project-tuned.single-glass.2050230.01"
MODEL_ASSET = "Effect/DimensionMaster/Meshes/fm_a_broken_012.wmodel"
PATTERN_ASSET = (
    "Effect/DimensionMaster/Textures/FX_TEX_HIGH_03/"
    "fx_h_brokenglass_02_1.dds"
)
CHRONORECOIL = "pc_sp_m_00_sk_sk_chronorecoil"
SCALAR_NAMES = [
    "CoverageGain",
    "BodyOpacity",
    "FresnelPower",
    "EdgeGain",
    "CrackGain",
    "RefractionStrength",
    "DistortionClamp",
    "EmissionGain",
]


def load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def finite_number(value: object) -> bool:
    return (
        isinstance(value, (int, float))
        and not isinstance(value, bool)
        and math.isfinite(float(value))
    )


def validate_product_document(document: dict) -> None:
    if (
        document.get("schema") != "lostark.effect-authoring"
        or document.get("version") != 13
        or document.get("effectAssetId") != PRODUCT_ID
    ):
        raise ValueError("single glass identity")
    elements = document.get("elements")
    if not isinstance(elements, list) or len(elements) != 1:
        raise ValueError("single glass element count")
    element = elements[0]
    if (
        element.get("id") != ELEMENT_ID
        or element.get("sourceNode")
        != "authored-copy:single-carrier.fm_a_broken_012"
        or element.get("kind") != "particle"
        or element.get("visible") is not True
        or element.get("resources")
        != [
            {"slotId": "meshModel", "assetId": MODEL_ASSET},
            {"slotId": "base", "assetId": PATTERN_ASSET},
        ]
    ):
        raise ValueError("single glass carrier")

    material = element.get("material", {})
    execution = material.get("execution", {})
    if (
        material.get("templateId") != "effect.standard"
        or material.get("renderProfile") != "alpha_two_sided_depth_read"
        or material.get("sourceProfile") != {"enabled": False}
        or execution.get("enabled") is not True
        or execution.get("fidelity") != "PROJECT_TUNED_APPROX"
        or execution.get("backend") != "runtimeMaterialV2"
        or execution.get("opcode") != 1004
        or execution.get("passIndex") != 1
        or execution.get("textureLaneCount") != 1
        or execution.get("textureMask") != 1
    ):
        raise ValueError("single glass material packet")
    lanes = execution.get("textureLanes")
    if (
        not isinstance(lanes, list)
        or len(lanes) != 1
        or lanes[0].get("role") != "broken_glass_pattern"
        or lanes[0].get("assetId") != PATTERN_ASSET
        or lanes[0].get("textureRegister") != 0
        or lanes[0].get("samplerRegister") != 5
    ):
        raise ValueError("single glass texture lane")

    scalars = execution.get("scalars")
    if not isinstance(scalars, list) or len(scalars) != len(SCALAR_NAMES):
        raise ValueError("single glass scalar schema")
    for index, name in enumerate(SCALAR_NAMES):
        if (
            scalars[index].get("name") != name
            or scalars[index].get("packedIndex") != index
            or not finite_number(scalars[index].get("value"))
        ):
            raise ValueError("single glass scalar schema")
    vectors = execution.get("vectors")
    if not isinstance(vectors, list) or [row.get("name") for row in vectors] != [
        "BodyTintLinear",
        "EdgeTintLinear",
    ]:
        raise ValueError("single glass vector schema")

    detail = element.get("detail", {})
    particle = detail.get("particle", {})
    mesh = detail.get("mesh", {})
    if (
        particle.get("maxParticles") != 1
        or particle.get("burstCount") != 1
        or particle.get("spawnRatePerSecond") != 0
        or particle.get("randomSeed") != 2050231
        or particle.get("localSpace") is not True
        or particle.get("billboard") is not False
        or particle.get("lifeTimeSeconds") != [1.4, 1.4]
        or mesh.get("useModelMaterial") is not False
        or mesh.get("modelPreScale") != 0.05
        or element.get("sourceRecipe") != {"enabled": False}
        or element.get("sourcePresentation") != {"enabled": False}
    ):
        raise ValueError("single glass one-particle contract")


class DimensionMasterSingleGlassProductCanaryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.catalog = load_json(CATALOG_PATH)
        cls.audition_catalog = load_json(AUDITION_CATALOG_PATH)
        cls.document = load_json(PRODUCT_PATH)
        cls.events = EVENT_PATH.read_text(encoding="utf-8")

    def test_product_catalog_has_exact_direct_authored_row(self) -> None:
        rows = [
            row
            for row in self.catalog["effects"]
            if row.get("effectAssetId") == PRODUCT_ID
        ]
        self.assertEqual(
            [
                {
                    "effectAssetId": PRODUCT_ID,
                    "payloadKind": "DIRECT_AUTHORED_DOCUMENT",
                    "authoringPath": f"Effects/Authored/{PRODUCT_ID}.effect.json",
                }
            ],
            rows,
        )
        self.assertNotIn(
            PRODUCT_ID,
            {
                row.get("effectAssetId")
                for row in self.audition_catalog["effects"]
                if isinstance(row, dict)
            },
        )
        self.assertIn(
            AUDITION_ID,
            {
                row.get("effectAssetId")
                for row in self.audition_catalog["effects"]
                if isinstance(row, dict)
            },
        )

    def test_document_is_exactly_one_typed_glass_particle(self) -> None:
        validate_product_document(self.document)

    def test_f_has_exactly_one_700ms_product_cue(self) -> None:
        header, *rows = self.events.splitlines()
        match = re.fullmatch(r'LOSTARK_ANIM_EVENTS 6 "DimensionMaster" (\d+)', header)
        self.assertIsNotNone(match)
        assert match is not None
        self.assertEqual(int(match.group(1)), len(rows))
        cue_pattern = re.compile(
            rf'^"{re.escape(CHRONORECOIL)}" EFFECT startms=(\d+) '
            rf'payload="{re.escape(PRODUCT_ID)}" effectref=asset '
            r'anchor="root" follow=follow stop=natural '
            r'px=0 py=0 pz=0 rx=0 ry=0 rz=0 sx=1 sy=1 sz=1$'
        )
        payload_rows = [
            row for row in rows if f'payload="{PRODUCT_ID}"' in row
        ]
        self.assertEqual(1, len(payload_rows))
        admitted = cue_pattern.fullmatch(payload_rows[0])
        self.assertIsNotNone(admitted)
        assert admitted is not None
        self.assertEqual("700", admitted.group(1))

    def test_renderer_and_shader_keep_color_and_distortion_outputs_typed(self) -> None:
        renderer = (
            REPOSITORY_ROOT / "Client/Private/Effect_DocumentRenderer.cpp"
        ).read_text(encoding="utf-8-sig")
        shader = (
            REPOSITORY_ROOT
            / "Client/Bin/ShaderFiles/Shader_EffectUe3MaterialFamilies.hlsli"
        ).read_text(encoding="utf-8")
        tool = (
            REPOSITORY_ROOT / "Client/Private/Effect_Tool.cpp"
        ).read_text(encoding="utf-8-sig")
        for witness in (PRODUCT_ID, ELEMENT_ID, MODEL_ASSET):
            self.assertIn(witness, renderer)
        self.assertIn("Shade_EffectProjectTunedGlassMeshV1", shader)
        self.assertIn("output.SceneColor = float4(radiance, coverage);", shader)
        self.assertIn("output.Distortion = float4(distortion, 0.f, 0.f);", shader)
        for label in (
            "Glass Tint",
            "Glass Coverage / Edge",
            "Glass Refraction",
            "Glass Emission",
        ):
            self.assertIn(label, tool)

    def test_mutation_cannot_silently_become_a_multi_particle_effect(self) -> None:
        wrong_model = copy.deepcopy(self.document)
        wrong_model["elements"][0]["resources"][0]["assetId"] = (
            "Effect/DimensionMaster/Meshes/not-single-glass.wmodel"
        )
        with self.assertRaisesRegex(ValueError, "single glass carrier"):
            validate_product_document(wrong_model)

        multiple = copy.deepcopy(self.document)
        multiple["elements"][0]["detail"]["particle"]["maxParticles"] = 2
        with self.assertRaisesRegex(ValueError, "one-particle contract"):
            validate_product_document(multiple)


if __name__ == "__main__":
    unittest.main()

#!/usr/bin/env python3

from __future__ import annotations

import copy
import hashlib
import json
import re
import unittest
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
AUTHORED_ROOT = REPOSITORY_ROOT / "Data" / "Effects" / "Authored"
EVENT_PATH = (
    REPOSITORY_ROOT
    / "Data"
    / "Animation"
    / "Authored"
    / "DimensionMaster"
    / "DimensionMaster.animevents"
)
CATALOG_PATH = REPOSITORY_ROOT / "Data" / "Effects" / "EffectCatalog.json"
WATER_ID = "effect.dimensionmaster.skill.2050230.water-burst"
WATER_PATH = AUTHORED_ROOT / f"{WATER_ID}.effect.json"
CHRONORECOIL = "pc_sp_m_00_sk_sk_chronorecoil"
EXPECTED_CUES = [
    (0, "effect.dimensionmaster.skill.2050230.unified"),
    (450, "effect.dimensionmaster.skill.2050120.clip3.unified"),
    (590, "effect.dimensionmaster.skill.2050120.clip2.unified"),
    (700, WATER_ID),
]
ORIGINAL_RAW_SHA256 = {
    "effect.dimensionmaster.skill.2050230.unified.effect.json": (
        "afab680bd36b4efcc4baf654c4848a1f3571a29cbc338b0ed58fec940de60e09"
    ),
    "effect.dimensionmaster.skill.2050120.clip2.unified.effect.json": (
        "bf09a2f1b87789081a94458e71214ec71a9a433823f8ba8e0bb4d0f3f2aaaf7a"
    ),
    "effect.dimensionmaster.skill.2050120.clip3.unified.effect.json": (
        "f0492a1563f7d14ea43778bcc973c4ab373d6da764ef0eee966fa7de6dd2fc1d"
    ),
}
VALTAN_WATER_ELEMENT_SHA256 = (
    "9e37907097226da2276c78e73b01eb0780d18aa9f2383e59303fd633f77348c3"
)


def load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def canonical_sha256(value: object) -> str:
    payload = json.dumps(
        value, sort_keys=True, separators=(",", ":"), ensure_ascii=False
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def collect_chronorecoil_product_cues(event_text: str) -> list[tuple[int, str]]:
    pattern = re.compile(
        rf'^"{re.escape(CHRONORECOIL)}" EFFECT startms=(\d+) '
        r'payload="([^"]+)" effectref=asset anchor="root" follow=follow '
        r'stop=natural px=0 py=0 pz=0 rx=0 ry=0 rz=0 sx=1 sy=1 sz=1$'
    )
    cues: list[tuple[int, str]] = []
    for line in event_text.splitlines():
        match = pattern.fullmatch(line)
        if match is not None:
            cues.append((int(match.group(1)), match.group(2)))
    return cues


def validate_water_document(document: dict) -> None:
    if document.get("schema") != "lostark.effect-authoring":
        raise ValueError("water schema")
    if document.get("version") != 13 or document.get("effectAssetId") != WATER_ID:
        raise ValueError("water identity")
    elements = document.get("elements")
    if not isinstance(elements, list) or len(elements) != 1:
        raise ValueError("water element count")
    element = elements[0]
    if (
        element.get("id") != "project-tuned.water-burst.2050230.01"
        or element.get("sourceNode")
        != "authored-copy:source.68619daf746949ce5bee"
        or element.get("kind") != "particle"
        or not element.get("visible")
    ):
        raise ValueError("water carrier identity")
    if element.get("resources") != [
        {
            "slotId": "base",
            "assetId": "Effect/Valtan/Textures/FX_TEX_02/fx_d_noise_003.dds",
        },
        {
            "slotId": "mask",
            "assetId": "Effect/Valtan/Textures/FX_TEX_03/fx_e_fluid_006.dds",
        },
    ]:
        raise ValueError("water resources")

    material = element["material"]
    execution = material["execution"]
    packet_identity = (
        material["templateId"],
        material["sourceMaterialPath"],
        material["renderProfile"],
        material["sourceProfile"],
        execution["enabled"],
        execution["fidelity"],
        execution["backend"],
        execution["opcode"],
        execution["passIndex"],
        execution["renderState"],
    )
    if packet_identity != (
        "effect.standard",
        "fx_m_mi_01.fx_mi.fx_e_pa_fd_07_1_ad",
        "alpha_two_sided_depth_read",
        {"enabled": False},
        True,
        "PROJECT_TUNED_APPROX",
        "runtimeMaterialV2",
        1003,
        1,
        {
            "rasterizer": "RS_Cull_None",
            "depthStencil": "DSS_ReadOnly",
            "blend": "BS_EffectAlpha",
            "stencilReference": 0,
        },
    ):
        raise ValueError("water packet identity")
    if execution["textureLanes"] != [
        {
            "laneId": "lane.0",
            "role": "flow_noise",
            "assetId": "Effect/Valtan/Textures/FX_TEX_02/fx_d_noise_003.dds",
            "textureRegister": 0,
            "samplerRegister": 5,
            "sourceChannel": "RG",
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
        },
        {
            "laneId": "lane.1",
            "role": "droplet_mask",
            "assetId": "Effect/Valtan/Textures/FX_TEX_03/fx_e_fluid_006.dds",
            "textureRegister": 1,
            "samplerRegister": 6,
            "sourceChannel": "RGBA",
            "colorSpace": "linear",
            "sampler": {
                "filter": "linear",
                "addressU": "clamp",
                "addressV": "clamp",
                "addressW": "clamp",
                "mipLodBias": 0,
                "maxAnisotropy": 1,
                "comparison": "never",
                "borderColor": [0, 0, 0, 0],
                "minLod": 0,
                "maxLod": 3.40282347e38,
            },
        },
    ]:
        raise ValueError("water lanes")
    expected_masks = {
        "textureLaneCount": 2,
        "textureMask": 3,
        "dynamicConsumedMask": 0,
        "dynamicSuppressedMask": 15,
        "particleColorPolicy": 2,
        "particleColorConsumedMask": 8,
        "particleColorSuppressedMask": 7,
        "scalarCount": 16,
        "vectorCount": 2,
        "inputCount": 16,
        "inputConsumedMask": [65535, 0],
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
        raise ValueError("water packet masks")
    expected_scalars = [
        ("water.noise-tiling", 1.75),
        ("water.pan-x", 0.18),
        ("water.pan-y", -0.12),
        ("water.second-octave-scale", 3.0),
        ("water.flow-warp", 0.035),
        ("water.mask-threshold", 0.015),
        ("water.edge-softness", 1.5),
        ("water.rim-width", 0.12),
        ("water.coverage-power", 0.65),
        ("water.body-strength", 0.18),
        ("water.rim-strength", 1.1),
        ("water.distortion-strength", 0.012),
        ("water.alpha-gain", 0.9),
        ("water.fade-start-seconds", 0.3),
        ("water.fade-end-seconds", 0.62),
        ("water.card-feather", 0.12),
    ]
    if [
        (row["name"], row["value"]) for row in execution["scalars"]
    ] != expected_scalars or [
        row["packedIndex"] for row in execution["scalars"]
    ] != list(range(16)):
        raise ValueError("water scalars")
    if execution["vectors"] != [
        {
            "name": "water.body-color",
            "packedIndex": 0,
            "value": [0.05, 0.28, 0.75, 1.0],
        },
        {
            "name": "water.rim-color",
            "packedIndex": 1,
            "value": [0.55, 0.9, 1.0, 1.0],
        },
    ]:
        raise ValueError("water vectors")

    detail = element["detail"]
    particle = detail["particle"]
    if (
        detail["transform"]["position"] != [0, 0.2, 0.6]
        or detail["timing"]["startDelaySeconds"] != 0
        or detail["timing"]["lifeTimeSeconds"] != 0.75
        or particle["maxParticles"] != 6
        or particle["burstCount"] != 6
        or particle["randomSeed"] != 2050230
        or particle["lifeTimeSeconds"] != [0.38, 0.62]
        or particle["initialVelocityMin"] != [-3, 3, -3]
        or particle["initialVelocityMax"] != [3, 6, 3]
        or particle["acceleration"] != [0, -8, 0]
        or particle["startSize"] != [0.28, 0.36]
        or particle["endSize"] != [0.08, 0.12]
        or particle["localSpace"] is not False
        or particle["billboard"] is not True
        or element["sourceRecipe"] != {"enabled": False}
    ):
        raise ValueError("water burst motion")


class DimensionMasterGlassWaterVisualCanaryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.event_text = EVENT_PATH.read_text(encoding="utf-8")
        cls.catalog = load_json(CATALOG_PATH)
        cls.water = load_json(WATER_PATH)

    def test_original_f_and_w_documents_are_byte_immutable(self) -> None:
        for name, expected in ORIGINAL_RAW_SHA256.items():
            self.assertEqual(
                expected, hashlib.sha256((AUTHORED_ROOT / name).read_bytes()).hexdigest()
            )

    def test_valtan_water_resource_donor_remains_canonical(self) -> None:
        donor_document = load_json(
            AUTHORED_ROOT
            / "effect.valtan.carrier-v1.attack.down-smash.active.clip-01.effect.json"
        )
        donor = next(
            row
            for row in donor_document["elements"]
            if row["id"] == "source.68619daf746949ce5bee"
        )
        self.assertEqual(VALTAN_WATER_ELEMENT_SHA256, canonical_sha256(donor))

    def test_chronorecoil_has_exact_full_f_w_and_water_composition(self) -> None:
        header, *event_lines = self.event_text.splitlines()
        match = re.fullmatch(r'LOSTARK_ANIM_EVENTS 6 "DimensionMaster" (\d+)', header)
        self.assertIsNotNone(match)
        assert match is not None
        self.assertEqual(int(match.group(1)), len(event_lines))
        self.assertEqual(EXPECTED_CUES, collect_chronorecoil_product_cues(self.event_text))

    def test_w_internal_core_times_land_on_f_hit_at_700ms(self) -> None:
        clip2 = load_json(
            AUTHORED_ROOT
            / "effect.dimensionmaster.skill.2050120.clip2.unified.effect.json"
        )
        clip3 = load_json(
            AUTHORED_ROOT
            / "effect.dimensionmaster.skill.2050120.clip3.unified.effect.json"
        )
        self.assertTrue(
            any(
                abs(row["detail"]["timing"]["startDelaySeconds"] - 0.11)
                < 1.0e-6
                for row in clip2["elements"]
            )
        )
        glass = next(
            row
            for row in clip3["elements"]
            if row["id"] == "authored.source-particle.40e1b48e2f0f88dcfeff1549"
        )
        self.assertEqual(
            "effect.ue3.glasshole-02.v1",
            glass["material"]["sourceProfile"]["runtimeShaderProfileId"],
        )
        self.assertAlmostEqual(
            700,
            450
            + 1000 * glass["detail"]["timing"]["startDelaySeconds"],
            places=4,
        )

    def test_water_is_direct_authored_and_exactly_typed(self) -> None:
        rows = [
            row
            for row in self.catalog["effects"]
            if row["effectAssetId"] == WATER_ID
        ]
        self.assertEqual(
            [
                {
                    "effectAssetId": WATER_ID,
                    "payloadKind": "DIRECT_AUTHORED_DOCUMENT",
                    "authoringPath": f"Effects/Authored/{WATER_ID}.effect.json",
                }
            ],
            rows,
        )
        validate_water_document(self.water)

    def test_opcode_1003_is_occurrence_scoped_and_runtime_wired(self) -> None:
        opcode_documents = []
        opcode_pattern = re.compile(r'"opcode"\s*:\s*1003\b')
        for path in AUTHORED_ROOT.glob("*.effect.json"):
            if opcode_pattern.search(path.read_text(encoding="utf-8")):
                opcode_documents.append(path.name)
        self.assertEqual([WATER_PATH.name], opcode_documents)

        codec = (
            REPOSITORY_ROOT / "Client" / "Private" / "Effect_DocumentCodec.cpp"
        ).read_text(encoding="utf-8")
        renderer = (
            REPOSITORY_ROOT
            / "Client"
            / "Private"
            / "Effect_DocumentRenderer.cpp"
        ).read_text(encoding="utf-8")
        families = (
            REPOSITORY_ROOT
            / "Client"
            / "Bin"
            / "ShaderFiles"
            / "Shader_EffectUe3MaterialFamilies.hlsli"
        ).read_text(encoding="utf-8")
        particle = (
            REPOSITORY_ROOT
            / "Client"
            / "Bin"
            / "ShaderFiles"
            / "Shader_VtxEffectParticle.hlsl"
        ).read_text(encoding="utf-8")
        self.assertEqual(2, codec.count("iOpcode == 1003u"))
        self.assertIn(
            "Validate_DimensionMasterWaterDropletBurstExecution", renderer
        )
        self.assertIn("DIMENSIONMASTER_WATER_DROPLET_BURST_OPCODE = 1003u", renderer)
        self.assertIn(
            "RUNTIME_MATERIAL_V2_PROJECT_TUNED_WATER_DROPLET_BURST = 1003u",
            families,
        )
        self.assertIn("EffectProjectTunedWaterDropletBurstPacketIsValid", families)
        self.assertIn("Shade_EffectProjectTunedWaterDropletBurst", particle)

    def test_contract_rejects_mutated_water_packet(self) -> None:
        wrong_mask = copy.deepcopy(self.water)
        wrong_mask["elements"][0]["material"]["execution"][
            "particleColorConsumedMask"
        ] = 15
        with self.assertRaisesRegex(ValueError, "packet masks"):
            validate_water_document(wrong_mask)

        wrong_resource = copy.deepcopy(self.water)
        wrong_resource["elements"][0]["resources"][1]["assetId"] = (
            "Effect/Valtan/Textures/FX_TEX_03/not-water.dds"
        )
        with self.assertRaisesRegex(ValueError, "resources"):
            validate_water_document(wrong_resource)

    def test_contract_rejects_duplicate_or_retimed_cue(self) -> None:
        water_line = next(
            line
            for line in self.event_text.splitlines()
            if f'payload="{WATER_ID}" effectref=asset' in line
        )
        duplicated = self.event_text + "\n" + water_line
        self.assertNotEqual(
            EXPECTED_CUES, collect_chronorecoil_product_cues(duplicated)
        )
        retimed = self.event_text.replace(
            f'"{CHRONORECOIL}" EFFECT startms=590 '
            'payload="effect.dimensionmaster.skill.2050120.clip2.unified"',
            f'"{CHRONORECOIL}" EFFECT startms=591 '
            'payload="effect.dimensionmaster.skill.2050120.clip2.unified"',
            1,
        )
        self.assertNotEqual(EXPECTED_CUES, collect_chronorecoil_product_cues(retimed))


if __name__ == "__main__":
    unittest.main()

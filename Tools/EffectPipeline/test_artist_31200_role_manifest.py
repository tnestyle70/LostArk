from __future__ import annotations

import hashlib
import json
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[2]
DOCUMENT_PATH = ROOT / "Data/Effects/Authored/effect.artist.skill.31200.unified.effect.json"
MANIFEST_PATH = (
    ROOT
    / "Data/Effects/AuthoredCorrections/Artist/effect.artist.skill.31200.role-manifest.json"
)
SOURCE_RECEIPT_PATH = (
    ROOT / "Data/Effects/Imported/Artist/CurrentCombat/skill.31200.source-receipt.json"
)
NORMALIZED_GRAPH_PATH = (
    ROOT
    / "Data/Effects/Imported/Artist/CurrentCombat/Graphs/skill.31200.normalized-effect-graph.json"
)
FROZEN_ARTIST_F_PATH = (
    ROOT / "Data/Effects/Authored/effect.artist.skill.31470.unified.effect.json"
)
Q_MESH_EVIDENCE_PATH = (
    ROOT
    / "Data/Effects/Imported/Artist/CurrentCombat/skill.31200.q-mesh-source-evidence.receipt.json"
)
Q_GRAPH_PATH = NORMALIZED_GRAPH_PATH
F_GRAPH_PATH = (
    ROOT / "Data/Effects/Imported/Artist/Graphs/skill.31470.normalized-effect-graph.json"
)
F_TYPED_EVIDENCE_PATH = (
    ROOT
    / "Data/Effects/Imported/Artist/Materials/skill.31470.typed-material-evidence-contract.json"
)
RESOURCES_ROOT = ROOT / "Client/Bin/Resources"

Q_PARTICLE_MASTER_ID = "authored.source-particle.e705de9acdd96252b98fb6ff"
Q_SPRITEWAVE_ID = "authored.source-particle.8b8cad3aafdd36174d713698"
Q_DECAL_ID = "authored.source-decal.45800d0c0054acd91f11cfb4"
F_SPRITEWAVE_ID = "mesh.646163c341579b56"
VISIBLE_IDS = {Q_PARTICLE_MASTER_ID, Q_SPRITEWAVE_ID, Q_DECAL_ID}


def load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def sha256_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def source_emitter_path(element: dict) -> str:
    marker = "|element:"
    source_node = element["sourceNode"]
    if marker not in source_node:
        raise AssertionError(f"missing source occurrence identity: {element['id']}")
    return source_node.split(marker, 1)[1].casefold()


class Artist31200RoleManifestTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.document = load_json(DOCUMENT_PATH)
        cls.manifest = load_json(MANIFEST_PATH)
        cls.source_receipt = load_json(SOURCE_RECEIPT_PATH)
        cls.normalized_graph = load_json(NORMALIZED_GRAPH_PATH)
        cls.q_mesh_evidence = load_json(Q_MESH_EVIDENCE_PATH)
        cls.f_document = load_json(FROZEN_ARTIST_F_PATH)
        cls.f_graph = load_json(F_GRAPH_PATH)
        cls.f_typed_evidence = load_json(F_TYPED_EVIDENCE_PATH)
        cls.elements_by_id = {row["id"]: row for row in cls.document["elements"]}
        cls.entries_by_id = {row["elementId"]: row for row in cls.manifest["entries"]}
        cls.graph_nodes_by_path = {
            f"{row['package']}.{row['objectPath']}".casefold(): row
            for row in cls.normalized_graph["nodes"]
        }

    def test_manifest_covers_every_source_occurrence_without_deletion(self) -> None:
        self.assertEqual(self.manifest["schema"], "lostark.effect-role-manifest")
        self.assertEqual(self.manifest["version"], 1)
        self.assertEqual(self.manifest["effectAssetId"], self.document["effectAssetId"])
        self.assertEqual(len(self.document["elements"]), 31)
        self.assertEqual(len(self.manifest["entries"]), 31)
        self.assertEqual(set(self.entries_by_id), set(self.elements_by_id))
        self.assertEqual(len(self.entries_by_id), 31)

        evidence = self.manifest["sourceEvidence"]
        self.assertEqual(evidence["sourceReceiptSha256"], sha256_file(SOURCE_RECEIPT_PATH))
        self.assertEqual(
            evidence["normalizedGraphSha256"], sha256_file(NORMALIZED_GRAPH_PATH)
        )
        self.assertEqual(
            evidence["qMeshSourceEvidenceSha256"], sha256_file(Q_MESH_EVIDENCE_PATH)
        )
        source_package = next(
            row
            for row in self.source_receipt["sourcePackages"]
            if row["logicalPackage"] == evidence["logicalPackage"]
        )
        self.assertEqual(source_package["physicalPackage"], evidence["physicalPackage"])
        self.assertEqual(source_package["sourcePackageSha256"], evidence["sourcePackageSha256"])

        for element_id, entry in self.entries_by_id.items():
            element = self.elements_by_id[element_id]
            expected_path = entry["sourceEmitterPath"].casefold()
            self.assertEqual(source_emitter_path(element), expected_path)
            self.assertIn(expected_path, self.graph_nodes_by_path)

    def test_role_allowlist_is_the_only_visible_set(self) -> None:
        selected = [
            row for row in self.manifest["entries"] if row["decision"] == "SELECTED"
        ]
        suppressed = [
            row for row in self.manifest["entries"] if row["decision"] == "SUPPRESSED"
        ]
        self.assertEqual(len(selected), 3)
        self.assertEqual(len(suppressed), 28)
        self.assertEqual(
            sorted(row["role"] for row in selected),
            sorted(["GROUND_INK_DECAL"] + ["BLACK_CRESCENT_SLASH"] * 2),
        )
        expected_visible = {row["elementId"] for row in selected}
        self.assertEqual(expected_visible, VISIBLE_IDS)
        actual_visible = {
            row["id"] for row in self.document["elements"] if row["visible"] is True
        }
        self.assertEqual(actual_visible, expected_visible)
        for entry in suppressed:
            self.assertIs(self.elements_by_id[entry["elementId"]]["visible"], False)

    def test_selected_lifetimes_and_fidelity_ownership_are_source_bounded(self) -> None:
        for entry in self.manifest["entries"]:
            if entry["decision"] != "SELECTED":
                continue
            element = self.elements_by_id[entry["elementId"]]
            execution = element["material"]["execution"]
            if entry["elementId"] == Q_SPRITEWAVE_ID:
                self.assertIs(execution["enabled"], True)
                self.assertEqual(execution["backend"], "runtimeMaterialV2")
                self.assertNotIn("authoringApproximate", execution)
            else:
                self.assertIs(execution["authoringApproximate"], True)
            self.assertAlmostEqual(
                float(element["detail"]["timing"]["lifeTimeSeconds"]),
                float(entry["effectiveLifetimeSeconds"]),
                places=6,
            )
            particle_tail = max(
                float(value) for value in element["detail"]["particle"]["lifeTimeSeconds"]
            )
            emitter_duration = float(element["sourceRecipe"]["emitterDurationSeconds"])
            expected_bound = particle_tail + max(emitter_duration, 0.0)
            self.assertAlmostEqual(
                float(entry["effectiveLifetimeSeconds"]), expected_bound, places=6
            )

    def test_selected_mesh_and_decal_resources_are_physical_and_role_correct(self) -> None:
        for entry in self.manifest["entries"]:
            if entry["decision"] != "SELECTED":
                continue
            element = self.elements_by_id[entry["elementId"]]
            for binding in element.get("resources") or []:
                asset_id = binding["assetId"]
                self.assertTrue((RESOURCES_ROOT / asset_id).is_file(), asset_id)
            source_profile = element["material"].get("sourceProfile") or {}
            for texture in source_profile.get("textures") or []:
                asset_id = texture.get("assetId")
                if asset_id:
                    self.assertTrue((RESOURCES_ROOT / asset_id).is_file(), asset_id)

        slash_models = {
            entry["meshAssetId"]
            for entry in self.manifest["entries"]
            if entry["role"] == "BLACK_CRESCENT_SLASH"
        }
        self.assertEqual(
            slash_models,
            {
                "Effect/Artist/Meshes/fm_h_swing_03.wmodel",
                "Effect/Artist/Meshes/fm_h_swing_05.wmodel",
            },
        )

        decal = self.elements_by_id[Q_DECAL_ID]
        expected_bindings = self.entries_by_id[Q_DECAL_ID]["requiredResourceBindings"]
        self.assertEqual(decal["resources"], expected_bindings)
        self.assertEqual(
            sha256_file(RESOURCES_ROOT / "Effect/Artist/Textures/fx_w_inkpaddle.dds"),
            "7413e5fadb00547b991c78e7fa840f6c9ef8b9fa76289853b5c1cf5124d391ea",
        )
        for actual, expected in zip(
            decal["detail"]["color"]["multiply"], [0.06, 0.06, 0.07, 1.0]
        ):
            self.assertAlmostEqual(float(actual), expected, places=6)
        self.assertEqual(decal["detail"]["color"]["emissiveIntensity"], 0.0)

        binding = next(
            row
            for row in self.normalized_graph["materialParameterBindings"]
            if row["sourceMaterialPath"] == decal["material"]["sourceMaterialPath"]
        )
        source_textures = {row["name"]: row["texture"] for row in binding["textures"]}
        self.assertEqual(source_textures["01.alphamaskmap"], "fx_tex_06.fx_w_inkpaddle")
        self.assertEqual(source_textures["01.diffmap_a"], "fx_tex_02.fx_d_environ_004")
        self.assertEqual(source_textures["01.emismap"], "fx_tex_05.fx_k_blood_01")
        self.assertIsNone(binding["parentSourcePhysicalPackage"])
        self.assertEqual(
            self.entries_by_id[Q_DECAL_ID]["approximationReason"],
            "SOURCE_DECAL_PARENT_ARITHMETIC_UNAVAILABLE",
        )

    def test_q_mesh_source_lanes_and_artist_f_exact_execution_are_closed(self) -> None:
        particle_master = self.elements_by_id[Q_PARTICLE_MASTER_ID]
        self.assertEqual(
            self.entries_by_id[Q_PARTICLE_MASTER_ID]["materialContract"],
            "TYPED_PARTICLE_MASTER_FULL_NAMED_LANES_APPROXIMATE",
        )
        self.assertIs(
            particle_master["material"]["execution"]["authoringApproximate"], True
        )
        self.assertIs(particle_master["material"]["execution"]["enabled"], False)
        expected_particle_lanes = {
            "21.map_c": (
                "Effect/Artist/Textures/fx_bg_lightbeam_falloff_01_ycl.dds",
                "949d8b3e231af2de40b89152f53d8169dc61428a24f447b79145e81966d8295b",
            ),
            "01.map_a": (
                "Effect/Artist/Textures/fx_d_environ_035.dds",
                "67845b6501a645505f2ccb016d164a221d58f0f5bd3f23e58c83f7c4abaaacd8",
            ),
            "11.map_b": (
                "Effect/Artist/Textures/fx_d_atypical_055_1_cl.dds",
                "ed520b735702a85d4d72c346a3f6af2e0132188e0e6136c4d7dfec38be26e254",
            ),
            "06.map": (
                "Effect/Artist/Textures/fx_m_flow_03_n.dds",
                "3d02ae77275200075e64f286622ac918a50db5852a55593930841a1e0dc19c80",
            ),
            "02.map_e": (
                "Effect/Artist/Textures/fx_d_environ_067.dds",
                "4b1fd65d5584a4c00aa59a02a0abb69c294f3a9dfc280afa194344d8855f9705",
            ),
            "12.map_f": (
                "Effect/Artist/Textures/fx_k_liquid_02.dds",
                "05e4cf42291d549bb02ce28b0f24b916bbe7bc68eed58220a7ca5ef9a9bf79fa",
            ),
            "42.map_c": (
                "Effect/Artist/Textures/fx_d_fluid_026.dds",
                "8c1374da4a341417c11d0732fcac20c33755b8af407843eb0c6b25e4dad19b96",
            ),
        }
        actual_particle_lanes = {
            row["name"]: row["assetId"]
            for row in particle_master["material"]["sourceProfile"]["textures"]
            if row["name"] in expected_particle_lanes
        }
        self.assertEqual(
            actual_particle_lanes,
            {name: value[0] for name, value in expected_particle_lanes.items()},
        )
        for asset_id, expected_sha in expected_particle_lanes.values():
            self.assertEqual(sha256_file(RESOURCES_ROOT / asset_id), expected_sha)

        q_spritewave = self.elements_by_id[Q_SPRITEWAVE_ID]
        f_spritewave = next(
            row for row in self.f_document["elements"] if row["id"] == F_SPRITEWAVE_ID
        )
        self.assertEqual(
            self.entries_by_id[Q_SPRITEWAVE_ID]["materialContract"],
            "RUNTIME_MATERIAL_V2_SOURCE_EXACT_INPUTS",
        )
        self.assertEqual(q_spritewave["material"], f_spritewave["material"])
        execution = q_spritewave["material"]["execution"]
        self.assertEqual(
            {
                "backend": execution["backend"],
                "opcode": execution["opcode"],
                "passIndex": execution["passIndex"],
                "staticInputCount": execution["staticInputCount"],
                "staticSelectedMask": execution["staticSelectedMask"],
                "scalarCount": execution["scalarCount"],
                "vectorCount": execution["vectorCount"],
                "textureLaneCount": execution["textureLaneCount"],
                "renderState": execution["renderState"],
            },
            {
                "backend": "runtimeMaterialV2",
                "opcode": 8,
                "passIndex": 1,
                "staticInputCount": 9,
                "staticSelectedMask": 495,
                "scalarCount": 47,
                "vectorCount": 1,
                "textureLaneCount": 4,
                "renderState": {
                    "rasterizer": "RS_Cull_None",
                    "depthStencil": "DSS_ReadOnly",
                    "blend": "BS_EffectAlpha",
                    "stencilReference": 0,
                },
            },
        )
        expected_spritewave_sha = {
            "Effect/Artist/Textures/fx_m_trail_004_cl.dds": "5681360a77c21948e854a46cd2b6a547f40f676f3ff31c73902e777f112c30b0",
            "Effect/Artist/Textures/fx_d_noise_033.dds": "1505396d86385b176ced2a804001deda74ff20fa44578fb5ec23156c34cc48de",
            "Effect/Artist/Textures/fx_m_atypical_012.dds": "8a05d8f0ed9cb2a8d8b2e49139400b24d554f4c21a58f10ac5c334a55d573bab",
            "Effect/Artist/Textures/fx_m_noise_001.dds": "19843f9ee15e94e629926f45e1887ad6ca9815bfd785527ed8b4ce63692918b8",
        }
        self.assertEqual(
            {row["assetId"] for row in execution["textureLanes"]},
            set(expected_spritewave_sha),
        )
        for asset_id, expected_sha in expected_spritewave_sha.items():
            self.assertEqual(sha256_file(RESOURCES_ROOT / asset_id), expected_sha)

    def test_q_and_artist_f_spritewave_source_inputs_are_identical(self) -> None:
        source_path = "fx_m_mi_m_00.fx_m_pa_spritewave_01_19_tr"

        def binding(graph: dict) -> dict:
            matches = [
                row
                for row in graph["materialParameterBindings"]
                if row["sourceMaterialPath"] == source_path
            ]
            self.assertEqual(len(matches), 1)
            return matches[0]

        def normalized(row: dict) -> dict:
            return {
                "sourceMaterialPath": row["sourceMaterialPath"],
                "sourcePhysicalPackage": row["sourcePhysicalPackage"],
                "parent": row["parent"],
                "textures": sorted(
                    (value["name"], value["texture"])
                    for value in row["textures"]
                ),
                "scalars": sorted(
                    (value["name"], float(value["value"]))
                    for value in row["scalars"]
                ),
                "vectors": sorted(
                    (value["name"], value["value"]) for value in row["vectors"]
                ),
            }

        q_binding = binding(self.normalized_graph)
        f_binding = binding(self.f_graph)
        self.assertEqual(normalized(q_binding), normalized(f_binding))
        self.assertEqual(q_binding["parent"], "fx_m.fx_m_pa_spritewave_01_tr")
        self.assertEqual(q_binding["sourcePhysicalPackage"], "ZHJ4TC4PCK4PC4J22HIXEYUXEU.upk")

        recipes = [
            row
            for row in self.f_typed_evidence["materialRecipes"]
            if row["identity"]["canonicalSourceMaterialPath"] == source_path
        ]
        self.assertEqual(len(recipes), 1)
        identity = recipes[0]["identity"]
        self.assertEqual(
            identity["materialSerialSha256"],
            "f8ffba06e81c39387e168ebb2aace8f1ba84d648f4bb04f064b2d893dc7c74b2",
        )
        self.assertEqual(
            identity["physicalPackageSha256"],
            "6a46989680d244946e2c7910a444da7a403500c4e2f8af1665da196b05fadc3e",
        )

    def test_q_mesh_occurrence_numbers_and_decal_are_preserved(self) -> None:
        particle_master = self.elements_by_id[Q_PARTICLE_MASTER_ID]
        spritewave = self.elements_by_id[Q_SPRITEWAVE_ID]

        def dynamic_names(element: dict) -> list[str]:
            values = []
            for module in element["sourceRecipe"]["modules"]:
                if module["className"] != "particlemoduleparameterdynamic":
                    continue
                values.extend(
                    (literal["propertyPath"], literal["value"])
                    for literal in module["literals"]
                    if literal["propertyPath"].endswith(".paramname")
                )
            return [value for _, value in sorted(values)]

        self.assertEqual(
            dynamic_names(particle_master),
            ["alphadissolve[0-1]", "pan[0-2]", "edgestr[0-x]", "disrotion[0-x]"],
        )
        self.assertEqual(
            dynamic_names(spritewave),
            [
                "maintex_tile_pan",
                "dissolve",
                "uv_noisevelue",
                "uv_sphery_uv_noisepan",
            ],
        )
        self.assertEqual(
            particle_master["detail"]["particle"]["initialPositionMin"],
            [2.20000005, 1, 0],
        )
        self.assertEqual(
            particle_master["detail"]["particle"]["startSize"],
            [0.0139999995, 0.00999999978],
        )
        self.assertEqual(
            particle_master["detail"]["particle"]["endSize"], [0, 0]
        )
        self.assertEqual(
            spritewave["detail"]["particle"]["initialPositionMin"],
            [2.04999995, 1, 0.150000006],
        )
        self.assertEqual(
            spritewave["detail"]["particle"]["startSize"],
            [0.0129999993, 0.0129999993],
        )
        self.assertEqual(
            spritewave["detail"]["particle"]["endSize"],
            [0.0136499992, 0.0136499992],
        )
        for element in (particle_master, spritewave):
            self.assertEqual(element["detail"]["timing"]["startDelaySeconds"], 0.625)
            self.assertEqual(
                element["detail"]["timing"]["lifeTimeSeconds"], 0.400000006
            )
            self.assertEqual(element["detail"]["mesh"]["modelPreScale"], 0.00999999978)
            self.assertIs(element["detail"]["particle"]["billboard"], False)

        self.assertEqual(
            hashlib.sha256(
                json.dumps(
                    self.elements_by_id[Q_DECAL_ID],
                    ensure_ascii=False,
                    sort_keys=True,
                    separators=(",", ":"),
                ).encode("utf-8")
            ).hexdigest(),
            "db79d0f3b2301b4e3b5373aaaeec3f7ffe237ad726f184376e8e3f4929f6320f",
        )

    def test_unselected_source_meshes_remain_preserved_and_hidden(self) -> None:
        preserved_ids = {
            "authored.source-particle.9884e1b749a8dbff12c4a34c",
            "authored.source-particle.fb0e1be781f519a2ea2a02d6",
            "authored.source-particle.0f23b8105317d951c61a6cd9",
        }
        for element_id in preserved_ids:
            element = self.elements_by_id[element_id]
            self.assertEqual(element["sourceRecipe"]["rendererShape"], "mesh")
            self.assertIs(element["visible"], False)
            self.assertFalse(
                any(row["slotId"] == "meshModel" for row in element.get("resources") or [])
            )

    def test_frozen_artist_f_control_is_unchanged(self) -> None:
        self.assertEqual(
            sha256_file(FROZEN_ARTIST_F_PATH),
            "32676821df73c772bd313825c6968e2a79f9ada7af445b7734b07f0d40828799",
        )


if __name__ == "__main__":
    unittest.main()

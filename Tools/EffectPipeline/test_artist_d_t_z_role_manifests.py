from __future__ import annotations

import hashlib
import json
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[2]
RESOURCES_ROOT = ROOT / "Client/Bin/Resources"
FROZEN_ARTIST_F_PATH = (
    ROOT / "Data/Effects/Authored/effect.artist.skill.31470.unified.effect.json"
)


def load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def sha256_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def flatten_role_groups(groups: dict[str, list[str]]) -> list[str]:
    return [element_id for element_ids in groups.values() for element_id in element_ids]


def source_emitter_path(element: dict) -> str:
    marker = "|element:"
    source_node = element["sourceNode"]
    if marker not in source_node:
        raise AssertionError(f"missing source occurrence identity: {element['id']}")
    path = source_node.split(marker, 1)[1].casefold()
    return path.split(".event_source-event-", 1)[0]


class ArtistDTZRoleManifestTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.cases = [
            {
                "manifest_path": ROOT
                / "Data/Effects/AuthoredCorrections/Artist/effect.artist.skill.31490.role-manifest.json",
                "document_paths": [
                    ROOT
                    / "Data/Effects/Authored/effect.artist.skill.31490.unified.effect.json"
                ],
                "receipt_path": ROOT
                / "Data/Effects/Imported/Artist/CurrentCombat/skill.31490.source-receipt.json",
                "graph_path": ROOT
                / "Data/Effects/Imported/Artist/CurrentCombat/Graphs/skill.31490.normalized-effect-graph.json",
            },
            {
                "manifest_path": ROOT
                / "Data/Effects/AuthoredCorrections/Artist/effect.artist.skill.31950.role-manifest.json",
                "document_paths": [
                    ROOT
                    / "Data/Effects/Authored/effect.artist.skill.31950.unified.effect.json"
                ],
                "receipt_path": ROOT
                / "Data/Effects/Imported/Artist/CurrentCombat/skill.31950.source-receipt.json",
                "graph_path": ROOT
                / "Data/Effects/Imported/Artist/CurrentCombat/Graphs/skill.31950.normalized-effect-graph.json",
            },
            {
                "manifest_path": ROOT
                / "Data/Effects/AuthoredCorrections/Artist/effect.artist.skill.31050.role-manifest.json",
                "document_paths": [
                    ROOT
                    / "Data/Effects/Authored/effect.artist.skill.31050.clip1.unified.effect.json",
                    ROOT
                    / "Data/Effects/Authored/effect.artist.skill.31050.clip2.unified.effect.json",
                ],
                "receipt_path": ROOT
                / "Data/Effects/Imported/Artist/CurrentCombat/skill.31050.source-receipt.json",
                "graph_path": ROOT
                / "Data/Effects/Imported/Artist/CurrentCombat/Graphs/skill.31050.normalized-effect-graph.json",
            },
        ]
        for case in cls.cases:
            case["manifest"] = load_json(case["manifest_path"])
            case["documents"] = [load_json(path) for path in case["document_paths"]]
            case["receipt"] = load_json(case["receipt_path"])
            case["graph"] = load_json(case["graph_path"])

    def test_source_receipts_graphs_and_packages_are_pinned(self) -> None:
        for case in self.cases:
            with self.subTest(manifest=case["manifest_path"].name):
                manifest = case["manifest"]
                evidence = manifest["sourceEvidence"]
                self.assertEqual(
                    evidence["sourceReceiptSha256"], sha256_file(case["receipt_path"])
                )
                self.assertEqual(
                    evidence["normalizedGraphSha256"], sha256_file(case["graph_path"])
                )
                packages = {
                    row["logicalPackage"]: row
                    for row in case["receipt"]["sourcePackages"]
                }
                for expected in evidence["sourcePackages"]:
                    actual = packages[expected["logicalPackage"]]
                    self.assertEqual(actual["physicalPackage"], expected["physicalPackage"])
                    self.assertEqual(
                        actual["sourcePackageSha256"], expected["sourcePackageSha256"]
                    )

    def test_role_allowlists_preserve_rows_and_disable_every_other_occurrence(self) -> None:
        for case in self.cases:
            manifest = case["manifest"]
            document_manifests = manifest.get("documents")
            if document_manifests is None:
                document_manifests = [
                    {
                        "effectAssetId": manifest["effectAssetId"],
                        "elementCount": manifest["baseline"]["documentElementCount"],
                        "visibleRoleAllowlists": manifest["visibleRoleAllowlists"],
                        "roleLockedAllowlists": manifest["roleLockedAllowlists"],
                    }
                ]
            documents_by_asset = {
                document["effectAssetId"]: document for document in case["documents"]
            }
            total_visible = 0
            total_locked = 0
            total_elements = 0
            for expected in document_manifests:
                document = documents_by_asset[expected["effectAssetId"]]
                elements = document["elements"]
                elements_by_id = {row["id"]: row for row in elements}
                visible_ids = flatten_role_groups(expected["visibleRoleAllowlists"])
                locked_ids = flatten_role_groups(expected["roleLockedAllowlists"])
                self.assertEqual(len(elements), expected["elementCount"])
                self.assertEqual(len(elements_by_id), len(elements))
                self.assertEqual(len(set(visible_ids)), len(visible_ids))
                self.assertEqual(len(set(locked_ids)), len(locked_ids))
                self.assertTrue(set(visible_ids).isdisjoint(locked_ids))
                self.assertTrue(set(visible_ids) <= set(elements_by_id))
                self.assertTrue(set(locked_ids) <= set(elements_by_id))
                actual_visible = {
                    row["id"] for row in elements if row["visible"] is True
                }
                self.assertEqual(actual_visible, set(visible_ids))
                for element_id in locked_ids:
                    element = elements_by_id[element_id]
                    self.assertIs(element["visible"], False)
                    self.assertIs(element["material"]["execution"]["enabled"], False)
                    self.assertIs(element["material"]["execution"]["failClosed"], True)
                total_visible += len(visible_ids)
                total_locked += len(locked_ids)
                total_elements += len(elements)
            baseline = manifest["baseline"]
            self.assertEqual(total_elements, baseline["documentElementCount"])
            self.assertEqual(total_visible, baseline["visibleElementCount"])
            self.assertEqual(total_locked, baseline["roleLockedElementCount"])
            self.assertEqual(
                total_elements - total_visible - total_locked,
                baseline["suppressedByDefaultCount"],
            )

    def test_selected_occurrences_join_exact_normalized_source_nodes(self) -> None:
        for case in self.cases:
            graph_paths = {
                f"{row['package']}.{row['objectPath']}".casefold()
                for row in case["graph"]["nodes"]
            }
            manifest = case["manifest"]
            document_manifests = manifest.get("documents")
            if document_manifests is None:
                document_manifests = [manifest]
            documents_by_asset = {
                document["effectAssetId"]: document for document in case["documents"]
            }
            for expected in document_manifests:
                effect_asset_id = expected.get("effectAssetId", manifest["effectAssetId"])
                document = documents_by_asset[effect_asset_id]
                elements_by_id = {row["id"]: row for row in document["elements"]}
                selected_ids = flatten_role_groups(expected["visibleRoleAllowlists"])
                selected_ids += flatten_role_groups(expected["roleLockedAllowlists"])
                for element_id in selected_ids:
                    source_path = source_emitter_path(elements_by_id[element_id])
                    self.assertIn(source_path, graph_paths, element_id)

    def test_selected_resource_lanes_have_physical_closure(self) -> None:
        for case in self.cases:
            manifest = case["manifest"]
            document_manifests = manifest.get("documents")
            if document_manifests is None:
                document_manifests = [manifest]
            documents_by_asset = {
                document["effectAssetId"]: document for document in case["documents"]
            }
            for expected in document_manifests:
                effect_asset_id = expected.get("effectAssetId", manifest["effectAssetId"])
                document = documents_by_asset[effect_asset_id]
                elements_by_id = {row["id"]: row for row in document["elements"]}
                selected_ids = flatten_role_groups(expected["visibleRoleAllowlists"])
                selected_ids += flatten_role_groups(expected["roleLockedAllowlists"])
                for element_id in selected_ids:
                    element = elements_by_id[element_id]
                    for binding in element.get("resources") or []:
                        self.assertTrue(
                            (RESOURCES_ROOT / binding["assetId"]).is_file(),
                            binding["assetId"],
                        )
                    source_profile = element["material"].get("sourceProfile") or {}
                    for texture in source_profile.get("textures") or []:
                        asset_id = texture.get("assetId")
                        if asset_id:
                            self.assertTrue(
                                (RESOURCES_ROOT / asset_id).is_file(), asset_id
                            )
            for asset in manifest["exactAssets"]:
                asset_path = RESOURCES_ROOT / asset["assetId"]
                self.assertTrue(asset_path.is_file(), asset["assetId"])
                self.assertEqual(sha256_file(asset_path), asset["sha256"])

    def test_source_proven_mesh_and_decal_lanes_are_exact(self) -> None:
        t_document = load_json(
            ROOT / "Data/Effects/Authored/effect.artist.skill.31950.unified.effect.json"
        )
        t_elements = {row["id"]: row for row in t_document["elements"]}
        t_mesh_bindings = {
            "authored.source-particle.9d91ec9bc2f148340b39c280": "Effect/Artist/Meshes/fm_j_helixline_1.wmodel",
            "authored.source-particle.fd837ecc3c83ca3dedc2f7b6": "Effect/Artist/Meshes/fm_j_helixline_1.wmodel",
            "authored.source-particle.0756db16034b319f543b0d18": "Effect/Artist/Meshes/fm_j_helixline_1.wmodel",
            "authored.source-particle.22123b1b41884fb15ae5ce79": "Effect/Artist/Meshes/fm_c_square_001.wmodel",
            "authored.source-particle.d4e10cceb59b0601823c3106": "Effect/Artist/Meshes/fm_d_helix_031.wmodel",
        }
        for element_id, asset_id in t_mesh_bindings.items():
            resources = t_elements[element_id]["resources"]
            self.assertEqual(
                [row["assetId"] for row in resources if row["slotId"] == "meshModel"],
                [asset_id],
            )
        t_decal = t_elements["authored.source-decal.44c0558f5ae3eb34ceef93a0"]
        self.assertEqual(t_decal["material"]["sourceMaterialPath"], "fx_m_mi_04.fx_o_de_fi_07_2_tr")
        self.assertEqual(
            t_decal["resources"],
            [
                {
                    "slotId": "emissive",
                    "assetId": "Effect/Artist/Textures/fx_f_firedust001.dds",
                },
                {
                    "slotId": "noise",
                    "assetId": "Effect/Artist/Textures/fx_e_noise_002.dds",
                },
            ],
        )
        t_manifest = load_json(
            ROOT
            / "Data/Effects/AuthoredCorrections/Artist/effect.artist.skill.31950.role-manifest.json"
        )
        material_evidence = t_manifest["sourceMaterialEvidence"]
        self.assertEqual(
            material_evidence["sourcePhysicalPackage"],
            "YGI3SB3OBJ3O1TGUMP6QMP8O5.upk",
        )
        self.assertEqual(
            material_evidence["sourcePhysicalPackageSha256"],
            "aeb9a9b9efb7b8fa9f49fabf0ad19010390227f25a2e23b2044110a6012954f7",
        )
        self.assertEqual(
            material_evidence["emissiveTextureObjectPath"],
            "FX_TEX_04.fx_f_firedust001",
        )
        self.assertEqual(
            material_evidence["noiseTextureObjectPath"],
            "FX_TEX_03.fx_e_noise_002",
        )

        z_document = load_json(
            ROOT
            / "Data/Effects/Authored/effect.artist.skill.31050.clip2.unified.effect.json"
        )
        z_elements = {row["id"]: row for row in z_document["elements"]}
        z_mesh_bindings = {
            "authored.source-particle.3487c5ddd5a51e3a320e213d": "Effect/Artist/Meshes/fm_b_cylinder_002.wmodel",
            "authored.source-particle.932d589e59f7a845a05b5697": "Effect/Artist/Meshes/fm_d_cylinder_004.wmodel",
            "authored.source-particle.1c88f481cc45cfb03187c5a5": "Effect/Artist/Meshes/fm_d_cylinder_004.wmodel",
            "authored.source-particle.f89e87c920317092ed5420ef": "Effect/Artist/Meshes/fm_d_cylinder_004.wmodel",
        }
        for element_id, asset_id in z_mesh_bindings.items():
            resources = z_elements[element_id]["resources"]
            self.assertEqual(
                [row["assetId"] for row in resources if row["slotId"] == "meshModel"],
                [asset_id],
            )
        symbol = z_elements["authored.source-particle.7333c91fe90fce737bb443be"]
        self.assertIn(
            {
                "slotId": "base",
                "assetId": "Effect/Artist/Textures/fx_o_symbol_14.dds",
            },
            symbol["resources"],
        )

    def test_frozen_artist_f_control_is_unchanged(self) -> None:
        self.assertEqual(
            sha256_file(FROZEN_ARTIST_F_PATH),
            "32676821df73c772bd313825c6968e2a79f9ada7af445b7734b07f0d40828799",
        )


if __name__ == "__main__":
    unittest.main()

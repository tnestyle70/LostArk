from __future__ import annotations

import hashlib
import json
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import build_map_material_variants as tool
import build_maptool_scene as scene


class MapMaterialInventoryTests(unittest.TestCase):
    @staticmethod
    def write_json(path: Path, value: object) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(value), encoding="utf-8")

    @staticmethod
    def placement(source_id: str, level: str, mesh: str) -> dict:
        return {
            "placementId": source_id,
            "levelPackage": level,
            "asset": {"objectName": mesh, "objectPath": f"PKG.Mesh.{mesh}"},
            "transform": {
                "source": "actor",
                "position": {"x": 0.0, "y": 0.0, "z": 0.0},
                "rotation": {"pitch": 0, "yaw": 0, "roll": 0},
                "scale3D": {"x": 1.0, "y": 1.0, "z": 1.0},
            },
        }

    def test_v1_is_base_variant_and_v2_ordered_override_is_distinct(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            placements = root / "placements"
            base_placement = self.placement("base", "LV_TEST_A", "MeshA")
            self.write_json(
                placements / "LV_TEST_A.placements.json",
                {
                    "schemaVersion": 1,
                    "propertyErrors": [],
                    "unresolvedPlacements": [],
                    "placements": [base_placement],
                },
            )
            slots = [
                {
                    "slot": 0,
                    "packageIndex": -1,
                    "class": "MaterialInstanceConstant",
                    "objectPath": "PKG.Material.Override_MI",
                },
                {
                    "slot": 1,
                    "packageIndex": 0,
                    "class": None,
                    "objectPath": None,
                },
            ]
            signature = scene.material_signature_from_slots(slots)
            override = self.placement("override", "LV_TEST_B", "MeshA")
            override["materialOverrides"] = {
                "propertyPresent": True,
                "slots": slots,
                "signatureSha256": signature,
            }
            second_mesh = self.placement("second", "LV_TEST_B", "MeshB")
            second_mesh["materialOverrides"] = {
                "propertyPresent": False,
                "slots": [],
                "signatureSha256": scene.EMPTY_MATERIAL_SIGNATURE,
            }
            self.write_json(
                placements / "LV_TEST_B.placements.json",
                {
                    "schemaVersion": 2,
                    "propertyErrors": [],
                    "unresolvedPlacements": [],
                    "placements": [override, second_mesh],
                },
            )

            inventory = tool.build_inventory(
                [placements],
                area_id="LV_TEST",
                level_prefix="LV_TEST_",
                expect_packages=2,
                expect_source_meshes=2,
                expect_variants=3,
                expect_placements=3,
                expect_override_placements=1,
            )
            self.assertEqual(2, inventory["sourceMeshCount"])
            self.assertEqual(3, inventory["assetCount"])
            mesh_a = [
                row for row in inventory["assets"]
                if row["fullPath"].casefold() == "pkg.mesh.mesha"
            ]
            self.assertEqual(
                {scene.EMPTY_MATERIAL_SIGNATURE, signature},
                {row["materialSignatureSha256"] for row in mesh_a},
            )
            self.assertEqual(1, inventory["summary"]["overridePlacements"])
            self.assertEqual(2, inventory["summary"]["uniqueRawOverrideSignatures"])
            self.assertEqual(1, inventory["summary"]["nullMaterialSlots"])
            with self.assertRaisesRegex(tool.VariantError, "variants count gate"):
                tool.build_inventory(
                    [placements],
                    area_id="LV_TEST",
                    level_prefix="LV_TEST_",
                    expect_packages=2,
                    expect_source_meshes=2,
                    expect_variants=4,
                    expect_placements=3,
                    expect_override_placements=1,
                )

    def test_inventory_hash_and_parse_share_one_captured_placement_payload(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            placements = root / "placements"
            placement_path = placements / "LV_TEST_A.placements.json"
            original_document = {
                "schemaVersion": 1,
                "propertyErrors": [],
                "unresolvedPlacements": [],
                "placements": [
                    self.placement("original", "LV_TEST_A", "MeshA")
                ],
            }
            self.write_json(placement_path, original_document)
            original_payload = placement_path.read_bytes()
            real_snapshot = tool.load_json_snapshot

            def snapshot_then_mutate(path: Path):
                document, fingerprint = real_snapshot(path)
                mutated = dict(original_document)
                mutated["placements"] = [
                    self.placement("new-a", "LV_TEST_A", "MeshA"),
                    self.placement("new-b", "LV_TEST_A", "MeshB"),
                ]
                self.write_json(path, mutated)
                return document, fingerprint

            with patch.object(
                tool, "load_json_snapshot", side_effect=snapshot_then_mutate
            ):
                inventory = tool.build_inventory(
                    [placements],
                    area_id="LV_TEST",
                    level_prefix="LV_TEST_",
                    expect_packages=1,
                    expect_source_meshes=1,
                    expect_variants=1,
                    expect_placements=1,
                    expect_override_placements=0,
                )

            self.assertEqual(1, inventory["placementCount"])
            self.assertEqual(1, inventory["sources"][0]["selectedPlacementCount"])
            self.assertEqual(
                hashlib.sha256(original_payload).hexdigest(),
                inventory["sources"][0]["sha256"],
            )
            self.assertNotEqual(
                inventory["sources"][0]["sha256"],
                tool.sha256_file(placement_path),
            )


class ExactMaterialResolutionTests(unittest.TestCase):
    @staticmethod
    def props(path: Path, text: str) -> Path:
        path.write_text(text, encoding="utf-8")
        return path

    def test_full_object_path_selects_one_same_basename_and_resolves_parent(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            parent = self.props(root / "parent.props.txt", "BlendMode=BLEND_Opaque\n")
            child = self.props(
                root / "child.props.txt",
                "Parent=Material'PKG_A.Material.Shared'\n",
            )
            unrelated = self.props(root / "other.props.txt", "BlendMode=BLEND_Additive\n")
            candidates = [
                {
                    "objectPath": "PKG_A.Material.Shared",
                    "class": "Material",
                    "props": str(parent),
                },
                {
                    "objectPath": "PKG_B.Material.Shared",
                    "class": "Material",
                    "props": str(unrelated),
                },
                {
                    "objectPath": "PKG_A.Material.Child_MI",
                    "class": "MaterialInstanceConstant",
                    "props": str(child),
                },
            ]

            resolved = tool.resolve_material_contracts(
                [("PKG_A.Material.Child_MI", "MaterialInstanceConstant")],
                candidates,
                [],
            )
            self.assertEqual(
                ["PKG_A.Material.Shared", "PKG_A.Material.Child_MI"],
                resolved["pkg_a.material.child_mi"]["parentChain"],
            )
            self.assertNotIn("pkg_b.material.shared", resolved)

    def test_duplicate_full_identity_and_missing_parent_fail_closed(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            child = self.props(
                root / "child.props.txt",
                "Parent=Material'PKG.Material.Missing'\n",
            )
            row = {
                "objectPath": "PKG.Material.Child",
                "class": "MaterialInstanceConstant",
                "props": str(child),
            }
            with self.assertRaisesRegex(tool.VariantError, "found 2"):
                tool.resolve_material_contracts(
                    [("PKG.Material.Child", "MaterialInstanceConstant")],
                    [row, dict(row)],
                    [],
                )
            with self.assertRaisesRegex(tool.VariantError, "Missing, found 0"):
                tool.resolve_material_contracts(
                    [("PKG.Material.Child", "MaterialInstanceConstant")],
                    [row],
                    [],
                )

    def test_leaf_texture_override_is_applied_before_source_resolution(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            parent = self.props(
                root / "parent.props.txt",
                "Texture = Texture2D'tex.missing_parent_default'\n"
                "Name = texture_diffuse\n",
            )
            child = self.props(
                root / "child.props.txt",
                "Parent = Material'mastermaterial_bg.base.parent'\n"
                "ParameterValue = Texture2D'AREA.tex.child_diffuse'\n"
                "ParameterName = texture_diffuse\n",
            )
            texture = root / "child.dds"
            texture.write_bytes(b"child")
            resolved = tool.resolve_material_contracts(
                [("AREA.mat.child", "MaterialInstanceConstant")],
                [
                    {
                        "objectPath": "mastermaterial_bg.base.parent",
                        "class": "Material",
                        "props": str(parent),
                        "sourcePack": "parent-pack",
                    },
                    {
                        "objectPath": "AREA.mat.child",
                        "class": "MaterialInstanceConstant",
                        "props": str(child),
                        "sourcePack": "child-pack",
                    },
                ],
                [
                    {
                        "objectPath": "AREA.tex.child_diffuse",
                        "source": str(texture),
                        "sourcePacks": ["child-pack"],
                    }
                ],
            )
            contract = resolved["area.mat.child"]
            self.assertEqual(
                "AREA.tex.child_diffuse",
                contract["resolvedTextures"]["texture_diffuse"]["objectPath"],
            )

    def test_ambiguous_relative_texture_uses_material_source_pack(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            props = self.props(
                root / "material.props.txt",
                "Texture = Texture2D'tex.shared'\nName = texture_diffuse\n",
            )
            texture_a = root / "a.dds"
            texture_b = root / "b.dds"
            texture_a.write_bytes(b"a")
            texture_b.write_bytes(b"b")
            resolved = tool.resolve_material_contracts(
                [("mastermaterial_bg.base.parent", "Material")],
                [
                    {
                        "objectPath": "mastermaterial_bg.base.parent",
                        "class": "Material",
                        "props": str(props),
                        "sourcePack": "pack-b",
                    }
                ],
                [
                    {
                        "objectPath": "AREA_A.tex.shared",
                        "source": str(texture_a),
                        "sourcePacks": ["pack-a"],
                    },
                    {
                        "objectPath": "AREA_B.tex.shared",
                        "source": str(texture_b),
                        "sourcePacks": ["pack-b"],
                    },
                ],
            )
            self.assertEqual(
                "AREA_B.tex.shared",
                resolved["mastermaterial_bg.base.parent"]["resolvedTextures"][
                    "texture_diffuse"
                ]["objectPath"],
            )

    def test_material_graph_referenced_textures_resolve_without_runtime_role(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            parent = self.props(
                root / "parent.props.txt",
                "ReferencedTextures[2] = { Texture2D'tex.graph_default', "
                "Texture2D'PKG.tex.graph_mask' }\n",
            )
            child = self.props(
                root / "child.props.txt",
                "Parent = Material'PKG.mat.parent'\n",
            )
            default_texture = root / "default.dds"
            mask_texture = root / "mask.dds"
            default_texture.write_bytes(b"default")
            mask_texture.write_bytes(b"mask")
            resolved = tool.resolve_material_contracts(
                [("PKG.mat.child", "MaterialInstanceConstant")],
                [
                    {
                        "objectPath": "PKG.mat.parent",
                        "class": "Material",
                        "props": str(parent),
                        "sourcePack": "parent-pack",
                    },
                    {
                        "objectPath": "PKG.mat.child",
                        "class": "MaterialInstanceConstant",
                        "props": str(child),
                        "sourcePack": "child-pack",
                    },
                ],
                [
                    {
                        "objectPath": "PKG.tex.graph_default",
                        "source": str(default_texture),
                        "sourcePacks": ["parent-pack"],
                    },
                    {
                        "objectPath": "PKG.tex.graph_mask",
                        "source": str(mask_texture),
                        "sourcePacks": ["parent-pack"],
                    },
                ],
            )
            references = resolved["pkg.mat.child"]["resolvedReferencedTextures"]
            self.assertEqual(
                ["PKG.tex.graph_default", "PKG.tex.graph_mask"],
                [row["objectPath"] for row in references],
            )
            self.assertEqual(
                ["PKG.mat.parent", "PKG.mat.parent"],
                [row["declaredByMaterial"] for row in references],
            )

    def test_loaded_but_not_exported_texture_is_a_hydration_input(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            pack = Path(temporary) / "source" / "PACK"
            pack.mkdir(parents=True)
            (pack / "umodel.log.txt").write_text(
                "Loading Texture2D exported from package A.upk\n"
                "Loading Texture2D missing from package B.upk\n"
                "Exporting Texture2D exported to C:/work/umodel/PKG/tex/exported.dds\n",
                encoding="utf-8",
            )
            self.assertEqual(
                [{"physicalPackage": "B.upk", "objectName": "missing"}],
                tool.missing_loaded_texture_sources([pack.parent]),
            )

    def test_source_only_null_mesh_default_remains_explicit(self) -> None:
        defaults = tool.exact_mesh_defaults(
            [
                {
                    "meshObjectPath": "PKG.mesh.helper",
                    "slots": [
                        {
                            "slot": 0,
                            "class": None,
                            "objectPath": None,
                            "sourceOnlyReason": "umodel-empty-material3",
                            "sourceMaterialName": "helper_mi",
                        }
                    ],
                }
            ],
            "PKG.mesh.helper",
        )
        self.assertEqual("umodel-empty-material3", defaults[0]["sourceOnlyReason"])
        self.assertIsNone(defaults[0]["objectPath"])


class SlotUniqueCookTests(unittest.TestCase):
    @staticmethod
    def contract(object_path: str, texture: Path) -> dict:
        return {
            "objectPath": object_path,
            "class": "Material",
            "parentChain": [object_path],
            "resolvedTextures": {
                "texture_diffuse": {
                    "objectPath": f"PKG.Texture.{texture.stem}",
                    "source": str(texture),
                    "sha256": tool.sha256_file(texture),
                }
            },
            "resolvedReferencedTextures": [],
            "textures": {"texture_diffuse": f"PKG.Texture.{texture.stem}"},
            "scalars": {},
            "vectors": {},
            "flags": {},
        }

    def test_cook_remaps_materials_by_unique_slot_name(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            source_root = root / "source"
            full_path = "PKG.Mesh.Mesh"
            source_asset = tool.base.stable_asset_id(full_path, "Mesh")
            source_pack = source_root / source_asset
            source_pack.mkdir(parents=True)
            (source_pack / "mesh.bin").write_bytes(b"buffer")
            gltf = {
                "asset": {"version": "2.0"},
                "buffers": [{"uri": "mesh.bin", "byteLength": 6}],
                "materials": [{"name": "Shared"}, {"name": "Shared"}],
                "meshes": [
                    {"primitives": [{"material": 0}, {"material": 1}]}
                ],
            }
            (source_pack / "mesh.gltf").write_text(json.dumps(gltf), encoding="utf-8")
            (source_pack / "source.receipt.json").write_text(
                json.dumps({"fullPath": full_path, "gltf": "mesh.gltf"}),
                encoding="utf-8",
            )
            texture_a = root / "a.dds"
            texture_b = root / "b.dds"
            auxiliary_texture = root / "overlay.dds"
            referenced_texture = root / "graph.dds"
            texture_a.write_bytes(b"texture-a")
            texture_b.write_bytes(b"texture-b")
            auxiliary_texture.write_bytes(b"texture-overlay")
            referenced_texture.write_bytes(b"texture-graph")
            contracts = {
                "pkg.material.override": self.contract("PKG.Material.Override", texture_a),
                "pkg.material.defaultb": self.contract("PKG.Material.DefaultB", texture_b),
            }
            contracts["pkg.material.override"]["resolvedTextures"][
                "texture_overlay_diffuse"
            ] = {
                "objectPath": "PKG.Texture.overlay",
                "source": str(auxiliary_texture),
                "sha256": tool.sha256_file(auxiliary_texture),
            }
            contracts["pkg.material.override"]["resolvedReferencedTextures"] = [
                {
                    "declaredObjectPath": "graph",
                    "declaredByMaterial": "PKG.Material.Override",
                    "objectPath": "PKG.Texture.graph",
                    "source": str(referenced_texture),
                    "sha256": tool.sha256_file(referenced_texture),
                }
            ]
            signature = scene.material_signature_from_slots(
                [
                    {
                        "slot": 0,
                        "class": "Material",
                        "objectPath": "PKG.Material.Override",
                    }
                ]
            )
            asset = {
                "assetId": tool.variant_asset_id(source_asset, signature),
                "sourceAssetId": source_asset,
                "fullPath": full_path,
                "objectName": "Mesh",
                "materialSignatureSha256": signature,
                "materialSlots": [
                    {
                        "slot": 0,
                        "class": "Material",
                        "objectPath": "PKG.Material.Override",
                    }
                ],
            }
            converter = root / "ModelAssetConverter.exe"
            converter.write_bytes(b"tool")
            commands: list[list[str]] = []

            def fake_run(command, cwd, timeout, label):
                command = [str(item) for item in command]
                commands.append(command)
                if len(command) > 1 and command[1] == "info":
                    return subprocess.CompletedProcess(command, 0, "model info", "")
                output = Path(command[command.index("-o") + 1])
                output.write_bytes(b"WMOD-test")
                return subprocess.CompletedProcess(command, 0, "cooked", "")

            with patch.object(tool.base, "run", side_effect=fake_run):
                receipt = tool.cook_variant(
                    asset,
                    source_root=source_root,
                    output_root=root / "output",
                    converter=converter,
                    defaults_by_mesh={
                        "pkg.mesh.mesh": [
                            {"class": "Material", "objectPath": "PKG.Material.DefaultA"},
                            {"class": "Material", "objectPath": "PKG.Material.DefaultB"},
                        ]
                    },
                    contracts=contracts,
                    timeout=1.0,
                    force=False,
                )

            cook_command = commands[0]
            self.assertIn("SLOT_000_Shared=", " ".join(cook_command))
            self.assertIn("SLOT_001_Shared=", " ".join(cook_command))
            self.assertEqual(
                ["SLOT_000_Shared", "SLOT_001_Shared"],
                [row["runtimeName"] for row in receipt["materials"]],
            )
            self.assertNotIn(str(auxiliary_texture), " ".join(cook_command))
            self.assertNotIn(str(referenced_texture), " ".join(cook_command))
            dependencies = receipt["materials"][0]["textureDependencies"]
            self.assertEqual(3, len(dependencies))
            self.assertEqual(
                [None, None],
                [
                    row["runtimeBinding"]
                    for row in dependencies
                    if row["objectPath"]
                    in ("PKG.Texture.overlay", "PKG.Texture.graph")
                ],
            )
            runtime_pack = root / "output" / "runtime" / asset["assetId"]
            for dependency in dependencies:
                self.assertTrue((runtime_pack / dependency["path"]).is_file())
            self.assertTrue(
                receipt["runtimeCoverage"]["textureDependencyClosureComplete"]
            )


class CasInstallTests(unittest.TestCase):
    @staticmethod
    def write_manifest(path: Path, area_id: str, runtime: Path, payload: bytes) -> None:
        assets = []
        for asset_id in ("A", "B"):
            source = runtime / asset_id / f"{asset_id}.wmodel"
            source.parent.mkdir(parents=True, exist_ok=True)
            source.write_bytes(payload + asset_id.encode("ascii"))
            relative = f"{asset_id}/{asset_id}.wmodel"
            assets.append(
                {
                    "assetId": asset_id,
                    "runtimeCoverage": {
                        "textureDependencyClosureComplete": True,
                        "textureSlotsComplete": True,
                        "materialComplete": True,
                    },
                    "sourceOnlyUnsupported": [],
                    "files": [
                        {
                            "source": relative,
                            "path": relative,
                            "sha256": tool.sha256_file(source),
                        }
                    ],
                }
            )
        path.write_text(
            json.dumps(
                {
                    "schemaVersion": 1,
                    "areaId": area_id,
                    "assetCount": len(assets),
                    "assets": assets,
                }
            ),
            encoding="utf-8",
        )

    def test_unknown_file_blocks_replacement_and_failed_promotion_rolls_back(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            resources = root / "Resources"
            runtime = root / "runtime"
            manifest = root / "runtime.json"
            area_id = "LV_TEST"
            self.write_manifest(manifest, area_id, runtime, b"old-")
            tool.install_runtime_area(
                manifest,
                runtime,
                resources,
                area_id=area_id,
                expect_variants=2,
            )
            destination = resources / "Map" / area_id
            old_bytes = (destination / "A" / "A.wmodel").read_bytes()
            unknown = destination / "user-note.txt"
            unknown.write_text("preserve me", encoding="utf-8")
            with self.assertRaisesRegex(tool.VariantError, "unknown"):
                tool.install_runtime_area(
                    manifest,
                    runtime,
                    resources,
                    area_id=area_id,
                    expect_variants=2,
                )
            self.assertEqual("preserve me", unknown.read_text(encoding="utf-8"))
            unknown.unlink()

            self.write_manifest(manifest, area_id, runtime, b"new-")
            with self.assertRaisesRegex(tool.VariantError, "injected"):
                tool.install_runtime_area(
                    manifest,
                    runtime,
                    resources,
                    area_id=area_id,
                    expect_variants=2,
                    fail_after="promote",
                )
            self.assertEqual(old_bytes, (destination / "A" / "A.wmodel").read_bytes())
            tool.validate_owned_area(destination)
            self.assertEqual(
                [],
                list((resources / "Map").glob(".li-*"))
                + list((resources / "Map").glob(".lr-*")),
            )

    def test_partial_material_install_requires_explicit_preview_admission(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            resources = root / "Resources"
            runtime = root / "runtime"
            manifest = root / "runtime.json"
            self.write_manifest(manifest, "LV_TEST", runtime, b"partial-")
            document = json.loads(manifest.read_text(encoding="utf-8"))
            for asset in document["assets"]:
                asset["runtimeCoverage"]["materialComplete"] = False
                asset["sourceOnlyUnsupported"] = [
                    {"slot": 0, "fields": ["renderFlag:twoSided"]}
                ]
            manifest.write_text(json.dumps(document), encoding="utf-8")

            with self.assertRaisesRegex(tool.VariantError, "partial"):
                tool.install_runtime_area(
                    manifest,
                    runtime,
                    resources,
                    area_id="LV_TEST",
                    expect_variants=2,
                )

            receipt = tool.install_runtime_area(
                manifest,
                runtime,
                resources,
                area_id="LV_TEST",
                expect_variants=2,
                allow_partial_material_preview=True,
            )
            self.assertEqual(
                receipt["admissionState"], "geometry-preview-partial-material"
            )
            self.assertEqual(receipt["runtimeCoverage"]["materialIncomplete"], 2)

    def test_malicious_manifest_cannot_forge_material_completeness(self) -> None:
        cases = (
            (
                "material complete without texture closure",
                {
                    "textureDependencyClosureComplete": True,
                    "textureSlotsComplete": False,
                    "materialComplete": True,
                },
                [],
            ),
            (
                "material complete with unsupported source fields",
                {
                    "textureDependencyClosureComplete": True,
                    "textureSlotsComplete": True,
                    "materialComplete": True,
                },
                [{"slot": 0, "fields": ["renderFlag:twoSided"]}],
            ),
            (
                "material incomplete despite a complete supported closure",
                {
                    "textureDependencyClosureComplete": True,
                    "textureSlotsComplete": True,
                    "materialComplete": False,
                },
                [],
            ),
        )
        for label, coverage, unsupported in cases:
            with self.subTest(case=label), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                resources = root / "Resources"
                runtime = root / "runtime"
                manifest = root / "runtime.json"
                self.write_manifest(manifest, "LV_TEST", runtime, b"forged-")
                document = json.loads(manifest.read_text(encoding="utf-8"))
                document["assets"][0]["runtimeCoverage"] = coverage
                document["assets"][0]["sourceOnlyUnsupported"] = unsupported
                manifest.write_text(json.dumps(document), encoding="utf-8")

                with self.assertRaisesRegex(
                    tool.VariantError, "material coverage invariant failed"
                ):
                    tool.install_runtime_area(
                        manifest,
                        runtime,
                        resources,
                        area_id="LV_TEST",
                        expect_variants=2,
                        allow_partial_material_preview=True,
                    )
                self.assertFalse(resources.exists())


if __name__ == "__main__":
    unittest.main()

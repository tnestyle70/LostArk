from __future__ import annotations

import hashlib
import importlib.util
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).with_name("validate_effect_sources.py")
SPEC = importlib.util.spec_from_file_location("validate_effect_sources", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class EffectSourceValidatorTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        (self.root / "Data/Effects/Authored").mkdir(parents=True)
        (self.root / "Data/Effects/ScreenOverlays").mkdir(parents=True)
        (self.root / "Client/Private").mkdir(parents=True)
        (self.root / "Tools/EffectPipeline").mkdir(parents=True)
        (self.root / "Tools/EffectRenderContractHarness").mkdir(parents=True)
        self.effect_id = "effect.test.source"
        self.row = {
            "effectAssetId": self.effect_id,
            "payloadKind": MODULE.DIRECT_KIND,
            "authoringPath": f"Effects/Authored/{self.effect_id}.effect.json",
        }
        self._write_catalog([self.row])
        self._write_source(self.effect_id)
        self._write_consumers()

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def _write_json(self, path: Path, value: object) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(value, ensure_ascii=False), encoding="utf-8")

    def _write_catalog(self, rows: list[dict[str, object]]) -> None:
        self._write_json(
            self.root / "Data/Effects/EffectCatalog.json",
            {"formatVersion": 1, "effects": rows},
        )

    def _write_source(self, effect_id: str, *, document_id: str | None = None) -> None:
        self._write_json(
            self.root / f"Data/Effects/Authored/{effect_id}.effect.json",
            {
                "schema": "lostark.effect-authoring",
                "version": 13,
                "effectAssetId": document_id or effect_id,
                "displayName": effect_id,
                "particleSystem": {},
                "modelCues": [],
                "elements": [],
            },
        )

    def _set_source_resource(self, resource_id: str, *, model: bool = False) -> None:
        path = self.root / f"Data/Effects/Authored/{self.effect_id}.effect.json"
        source = json.loads(path.read_text(encoding="utf-8"))
        if model:
            source["modelCues"] = [{"modelAssetId": resource_id}]
        else:
            source["elements"] = [{"resources": [{"assetId": resource_id}]}]
        self._write_json(path, source)

    def _write_overlay(self, resource_id: str) -> None:
        self.row["screenOverlayPresentationPath"] = (
            f"Effects/ScreenOverlays/{self.effect_id}.screen-overlay.json"
        )
        self._write_catalog([self.row])
        self._write_json(
            self.root
            / f"Data/Effects/ScreenOverlays/{self.effect_id}.screen-overlay.json",
            {
                "schema": "lostark.effect-screen-overlay",
                "formatVersion": 1,
                "presentationId": f"{self.effect_id}.screen-overlay",
                "overlays": [{"textureAssetId": resource_id}],
            },
        )

    def _write_resource(self, resource_id: str, payload: bytes) -> Path:
        path = self.root / "Client/Bin/Resources" / Path(resource_id)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(payload)
        return path

    def _git(self, *args: str) -> None:
        completed = subprocess.run(
            ["git", "-C", str(self.root), *args],
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        self.assertEqual(completed.returncode, 0, completed.stderr)

    def _git_bytes(self, *args: str, input_bytes: bytes) -> bytes:
        completed = subprocess.run(
            ["git", "-C", str(self.root), *args],
            check=False,
            input=input_bytes,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        self.assertEqual(
            completed.returncode,
            0,
            completed.stderr.decode("utf-8", errors="replace"),
        )
        return completed.stdout

    def _initialize_git(self, attributes: str | None = None) -> None:
        self._git("init", "--quiet")
        if attributes is not None:
            path = self.root / ".gitattributes"
            path.write_text(attributes, encoding="utf-8", newline="\n")
            self._git("add", "--", ".gitattributes")

    def _stage_index_blob(self, path: Path, payload: bytes) -> None:
        object_id = self._git_bytes(
            "hash-object", "-w", "--stdin", input_bytes=payload
        ).decode("ascii").strip()
        relative = path.relative_to(self.root).as_posix()
        self._git(
            "update-index",
            "--add",
            "--cacheinfo",
            f"100644,{object_id},{relative}",
        )

    @staticmethod
    def _lfs_pointer(payload: bytes) -> bytes:
        return (
            b"version https://git-lfs.github.com/spec/v1\n"
            + b"oid sha256:"
            + hashlib.sha256(payload).hexdigest().encode("ascii")
            + b"\nsize "
            + str(len(payload)).encode("ascii")
            + b"\n"
        )

    def _stage_lfs_resource(self, path: Path, payload: bytes) -> None:
        self._stage_index_blob(path, self._lfs_pointer(payload))

    def _write_v15_source(self, effect_id: str) -> None:
        self._write_json(
            self.root / f"Data/Effects/Authored/{effect_id}.effect.json",
            {
                "schema": "lostark.effect-authoring",
                "version": 15,
                "effectAssetId": effect_id,
                "displayName": effect_id,
                "particleSystem": {},
                "modelCues": [],
                "runtimeExtensions": {
                    "formatVersion": 1,
                    "bakedEdgeHistories": [],
                },
                "elements": [
                    {
                        "id": "source.ribbon.1",
                        "visible": True,
                        "kind": "trail",
                        "runtimeCarrier": {
                            "formatVersion": 1,
                            "kind": "cascadeRibbonV1",
                            "admission": "bounded",
                            "typeDataModuleStableId": "Source:Ribbon@1",
                        },
                        "sourceRecipe": {
                            "enabled": True,
                            "modules": [
                                {
                                    "stableId": "Source:Ribbon@1",
                                    "className": "ParticleModuleTypeDataRibbon",
                                }
                            ],
                        },
                    }
                ],
            },
        )

    def _write_consumers(self, *, forbidden: str = "") -> None:
        (self.root / "Client/Private/Effect_Catalog.cpp").write_text(
            "bool_t Client::CEffectCatalog::Load(std::string& s) { "
            f"{forbidden} return true; }}\n"
            "bool_t Client::CEffectCatalog::Stage_DebugDirectAuthoredReplacement() { return true; }\n",
            encoding="utf-8",
        )
        (self.root / "Client/Private/Effect_Tool.cpp").write_text(
            "Data/Effects/Authored is canonical\n", encoding="utf-8"
        )
        (self.root / "Tools/EffectRenderContractHarness/Run-EffectRenderContractHarness.ps1").write_text(
            "param()\n", encoding="utf-8"
        )

    def test_positive_is_read_only_and_reports_unbound_references(self) -> None:
        self._write_source("effect.reference.baseline")
        report = MODULE.validate_repository(self.root)
        self.assertEqual(report.direct_source_count, 1)
        self.assertEqual(report.unbound_reference_count, 1)

    def test_duplicate_id_is_rejected(self) -> None:
        self._write_catalog([self.row, dict(self.row)])
        with self.assertRaisesRegex(MODULE.ContractError, "duplicate direct Effect ID"):
            MODULE.validate_repository(self.root)

    def test_identity_derived_path_is_required(self) -> None:
        bad = dict(self.row)
        bad["authoringPath"] = "Effects/Authored/other.effect.json"
        self._write_catalog([bad])
        with self.assertRaisesRegex(MODULE.ContractError, "identity-derived"):
            MODULE.validate_repository(self.root)

    def test_source_document_id_must_match(self) -> None:
        self._write_source(self.effect_id, document_id="effect.test.wrong")
        with self.assertRaisesRegex(MODULE.ContractError, "mismatches"):
            MODULE.validate_repository(self.root)

    def test_typed_v15_runtime_carrier_source_is_accepted(self) -> None:
        self._write_v15_source(self.effect_id)
        report = MODULE.validate_repository(self.root)
        self.assertEqual(report.direct_source_count, 1)

    def test_v15_runtime_carrier_requires_closed_history(self) -> None:
        self._write_v15_source(self.effect_id)
        path = self.root / f"Data/Effects/Authored/{self.effect_id}.effect.json"
        source = json.loads(path.read_text(encoding="utf-8"))
        carrier = source["elements"][0]["runtimeCarrier"]
        carrier.clear()
        carrier.update(
            {
                "formatVersion": 1,
                "kind": "animationTrailBakedEdgeV1",
                "admission": "bounded",
                "historyId": "missing.history",
            }
        )
        source["elements"][0]["sourceRecipe"]["enabled"] = False
        self._write_json(path, source)
        with self.assertRaisesRegex(MODULE.ContractError, "history is missing"):
            MODULE.validate_repository(self.root)

    def test_retired_payload_kind_is_rejected(self) -> None:
        bad = dict(self.row)
        bad["payloadKind"] = "DIRECT_AUTHORED_DOCUMENT_V13"
        self._write_catalog([bad])
        with self.assertRaisesRegex(MODULE.ContractError, "retired Effect payloadKind"):
            MODULE.validate_repository(self.root)

    def test_retired_runtime_artifact_directory_is_rejected(self) -> None:
        path = self.root / "Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json"
        path.parent.mkdir(parents=True)
        path.write_text("{}", encoding="utf-8")
        with self.assertRaisesRegex(MODULE.ContractError, "retired Effect publish artifact"):
            MODULE.validate_repository(self.root)

    def test_missing_catalog_source_is_rejected(self) -> None:
        (self.root / f"Data/Effects/Authored/{self.effect_id}.effect.json").unlink()
        with self.assertRaisesRegex(MODULE.ContractError, "missing or unreadable"):
            MODULE.validate_repository(self.root)

    def test_retired_runtime_catalog_read_is_rejected(self) -> None:
        self._write_consumers(forbidden='auto p = "EffectCatalog.runtime.json";')
        with self.assertRaisesRegex(MODULE.ContractError, "retired runtime input"):
            MODULE.validate_repository(self.root)

    def test_retired_sidecar_builder_is_rejected(self) -> None:
        (self.root / "Tools/EffectPipeline/stale_builder.py").write_text(
            'PATH = "Data/Effects/VisualPrograms/effect-visual-program-runtime.v1.json"\n',
            encoding="utf-8",
        )
        with self.assertRaisesRegex(MODULE.ContractError, "retired runtime input"):
            MODULE.validate_repository(self.root)

    def test_missing_runtime_resource_is_rejected(self) -> None:
        self._set_source_resource("Effect/Test/missing.dds")
        with self.assertRaisesRegex(MODULE.ContractError, "resource files are missing"):
            MODULE.validate_repository(self.root)

    def test_unsafe_runtime_resource_id_is_rejected(self) -> None:
        self._set_source_resource("../outside.dds")
        with self.assertRaisesRegex(MODULE.ContractError, "unsafe Resources-relative"):
            MODULE.validate_repository(self.root)

    def test_untracked_runtime_resource_is_rejected_in_git_worktree(self) -> None:
        resource_id = "Effect/Test/untracked.dds"
        self._set_source_resource(resource_id)
        self._write_resource(resource_id, b"DDS untracked")
        self._git("init", "--quiet")
        with self.assertRaisesRegex(MODULE.ContractError, "not tracked by Git"):
            MODULE.validate_repository(self.root)

    def test_tracked_source_and_overlay_resource_closure_is_reported(self) -> None:
        model_id = "Effect/Test/model.wmodel"
        overlay_id = "Effect/Test/overlay.dds"
        model_payload = b"WINTwmodel-payload"
        overlay_payload = b"DDS dds-payload"
        self._set_source_resource(model_id, model=True)
        self._write_overlay(overlay_id)
        model_path = self._write_resource(model_id, model_payload)
        overlay_path = self._write_resource(overlay_id, overlay_payload)
        self._initialize_git(
            "*.dds filter=lfs diff=lfs merge=lfs -text\n"
        )
        self._git("add", "--", model_path.relative_to(self.root).as_posix())
        self._stage_lfs_resource(overlay_path, overlay_payload)

        report = MODULE.validate_repository(self.root)

        self.assertEqual(report.resource_file_count, 2)
        self.assertEqual(
            report.resource_bytes, len(model_payload) + len(overlay_payload)
        )
        serialized = json.loads(report.as_json())
        self.assertEqual(serialized["resourceFileCount"], 2)
        self.assertEqual(
            serialized["resourceBytes"], len(model_payload) + len(overlay_payload)
        )

    def test_unhydrated_lfs_pointer_working_copy_is_rejected(self) -> None:
        resource_id = "Effect/Test/pointer.dds"
        hydrated_payload = b"DDS hydrated"
        pointer = self._lfs_pointer(hydrated_payload)
        self._set_source_resource(resource_id)
        resource_path = self._write_resource(resource_id, pointer)
        self._initialize_git(
            "*.dds filter=lfs diff=lfs merge=lfs -text\n"
        )
        self._stage_index_blob(resource_path, pointer)

        with self.assertRaisesRegex(MODULE.ContractError, "unhydrated Git LFS pointer"):
            MODULE.validate_repository(self.root)

    def test_dds_magic_is_required(self) -> None:
        resource_id = "Effect/Test/not-dds.dds"
        payload = b"not-a-dds"
        self._set_source_resource(resource_id)
        resource_path = self._write_resource(resource_id, payload)
        self._initialize_git(
            "*.dds filter=lfs diff=lfs merge=lfs -text\n"
        )
        self._stage_lfs_resource(resource_path, payload)

        with self.assertRaisesRegex(MODULE.ContractError, "DDS magic is invalid"):
            MODULE.validate_repository(self.root)

    def test_wmodel_magic_is_required(self) -> None:
        resource_id = "Effect/Test/not-wmodel.wmodel"
        payload = b"not-a-wmodel"
        self._set_source_resource(resource_id, model=True)
        resource_path = self._write_resource(resource_id, payload)
        self._initialize_git()
        self._stage_index_blob(resource_path, payload)

        with self.assertRaisesRegex(MODULE.ContractError, "WModel magic is invalid"):
            MODULE.validate_repository(self.root)

    def test_dds_lfs_pointer_must_match_working_bytes(self) -> None:
        resource_id = "Effect/Test/mismatch.dds"
        working_payload = b"DDS working"
        staged_payload = b"DDS staged"
        self._set_source_resource(resource_id)
        resource_path = self._write_resource(resource_id, working_payload)
        self._initialize_git(
            "*.dds filter=lfs diff=lfs merge=lfs -text\n"
        )
        self._stage_lfs_resource(resource_path, staged_payload)

        with self.assertRaisesRegex(MODULE.ContractError, "does not match working bytes"):
            MODULE.validate_repository(self.root)

    def test_dds_index_blob_must_be_an_lfs_pointer(self) -> None:
        resource_id = "Effect/Test/plain-index.dds"
        payload = b"DDS plain-index"
        self._set_source_resource(resource_id)
        resource_path = self._write_resource(resource_id, payload)
        self._initialize_git(
            "*.dds filter=lfs diff=lfs merge=lfs -text\n"
        )
        self._stage_index_blob(resource_path, payload)

        with self.assertRaisesRegex(MODULE.ContractError, "not a canonical LFS pointer"):
            MODULE.validate_repository(self.root)

    def test_wmodel_index_blob_must_match_working_bytes(self) -> None:
        resource_id = "Effect/Test/mismatch.wmodel"
        staged_payload = b"WINTstaged"
        working_payload = b"WINTworking"
        self._set_source_resource(resource_id, model=True)
        resource_path = self._write_resource(resource_id, staged_payload)
        self._initialize_git()
        self._stage_index_blob(resource_path, staged_payload)
        resource_path.write_bytes(working_payload)

        with self.assertRaisesRegex(MODULE.ContractError, "does not match working bytes"):
            MODULE.validate_repository(self.root)

    def test_intent_to_add_resource_is_rejected(self) -> None:
        resource_id = "Effect/Test/intent.wmodel"
        payload = b"WINTintent"
        self._set_source_resource(resource_id, model=True)
        resource_path = self._write_resource(resource_id, payload)
        self._initialize_git()
        self._git(
            "add",
            "--intent-to-add",
            "--",
            resource_path.relative_to(self.root).as_posix(),
        )

        with self.assertRaisesRegex(MODULE.ContractError, "does not match working bytes"):
            MODULE.validate_repository(self.root)

    def test_dds_filter_must_be_lfs(self) -> None:
        resource_id = "Effect/Test/filter.dds"
        payload = b"DDS filter"
        self._set_source_resource(resource_id)
        resource_path = self._write_resource(resource_id, payload)
        self._initialize_git()
        self._stage_lfs_resource(resource_path, payload)

        with self.assertRaisesRegex(MODULE.ContractError, "filter must be lfs"):
            MODULE.validate_repository(self.root)

    def test_wmodel_filter_must_be_unspecified(self) -> None:
        resource_id = "Effect/Test/filter.wmodel"
        payload = b"WINTfilter"
        self._set_source_resource(resource_id, model=True)
        resource_path = self._write_resource(resource_id, payload)
        self._initialize_git("*.wmodel filter=lfs\n")
        self._stage_index_blob(resource_path, payload)

        with self.assertRaisesRegex(MODULE.ContractError, "filter must be unspecified"):
            MODULE.validate_repository(self.root)


if __name__ == "__main__":
    unittest.main()

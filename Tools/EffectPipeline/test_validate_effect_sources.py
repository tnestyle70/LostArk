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
        self.effect_id = "effect.test.source"
        self.row = {
            "effectAssetId": self.effect_id,
            "payloadKind": MODULE.DIRECT_KIND,
            "authoringPath": f"Effects/Authored/{self.effect_id}.effect.json",
        }
        self._write_catalog([self.row])
        self._write_audition_catalog([])
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

    def _write_audition_catalog(self, rows: list[dict[str, object]]) -> None:
        self._write_json(
            self.root / "Data/Effects/EffectAuditionCatalog.json",
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

    def _audition_row(
        self,
        effect_id: str,
        *,
        source_effect_id: str | None = None,
        source_hash: str | None = None,
    ) -> dict[str, object]:
        source_effect_id = source_effect_id or self.effect_id
        source_path = (
            self.root
            / f"Data/Effects/Authored/{source_effect_id}.effect.json"
        )
        if source_hash is None:
            source_hash = hashlib.sha256(source_path.read_bytes()).hexdigest()
        return {
            "effectAssetId": effect_id,
            "payloadKind": MODULE.DIRECT_KIND,
            "authoringPath": f"Effects/Authored/{effect_id}.effect.json",
            "runtimeAdmission": "REGISTRY_BOUND_AUDITION_ONLY",
            "fidelityClass": "PROJECT_TUNED_APPROX",
            "sourceEffectAssetId": source_effect_id,
            "sourceDocumentRawSha256": source_hash,
        }

    def _set_source_resource(self, resource_id: str, *, model: bool = False) -> None:
        path = self.root / f"Data/Effects/Authored/{self.effect_id}.effect.json"
        source = json.loads(path.read_text(encoding="utf-8"))
        if model:
            source["modelCues"] = [{"modelAssetId": resource_id}]
        else:
            source["elements"] = [{"resources": [{"assetId": resource_id}]}]
        self._write_json(path, source)

    def test_native_particle_options_validate_without_source_fallback(self) -> None:
        element = {
            "id": "manual.hand.fire",
            "kind": "particle",
            "material": {"templateId": "effect.standard"},
            "resources": [],
            "detail": {
                "uv": {"tileColumns": 4, "tileRows": 4},
                "particle": {
                    "billboard": True,
                    "drag": 1,
                    "rotationRangeDegrees": [0, 0],
                    "spinRangeDegreesPerSecond": [55, 65],
                    "subUVOverLife": True,
                    "initialVelocity": {"mode": "cone", "speed": [0.2, 0.5], "uniformSolidAngle": True},
                },
            },
        }
        source = {"elements": [element]}
        MODULE._validate_native_sprite_particle_options(source, "fixture")
        cases = [
            ("negative-drag", lambda e: e["detail"]["particle"].update(drag=-1)),
            ("nonfinite-drag", lambda e: e["detail"]["particle"].update(drag=float("nan"))),
            ("inverted-spin", lambda e: e["detail"]["particle"].update(spinRangeDegreesPerSecond=[65, 55])),
            ("boolean-rotation", lambda e: e["detail"]["particle"].update(rotationRangeDegrees=[False, 1])),
            ("numeric-life-uv", lambda e: e["detail"]["particle"].update(subUVOverLife=1)),
            ("source-recipe", lambda e: e.update(sourceRecipe={"enabled": True})),
            ("typed-material", lambda e: e["material"].update(execution={"enabled": True})),
            ("non-billboard", lambda e: e["detail"]["particle"].update(billboard=False)),
            ("emitter-clock", lambda e: e["detail"]["uv"].update(sequence=True)),
            ("no-atlas", lambda e: e["detail"]["uv"].update(tileColumns=1, tileRows=1)),
            ("non-cone", lambda e: e["detail"]["particle"]["initialVelocity"].update(mode="outward")),
            ("invalid-cone-angle", lambda e: e["detail"]["particle"]["initialVelocity"].update(coneAngleDegrees=181)),
            ("malformed-material", lambda e: e.update(material=[])),
            ("malformed-resources", lambda e: e.update(resources={})),
        ]
        for label, mutate in cases:
            with self.subTest(label=label):
                candidate = json.loads(json.dumps(element))
                mutate(candidate)
                with self.assertRaises(MODULE.ContractError):
                    MODULE._validate_native_sprite_particle_options({"elements": [candidate]}, "fixture")

    def test_native_particle_defaults_do_not_reject_existing_source_rows(self) -> None:
        MODULE._validate_native_sprite_particle_options({"elements": [{
            "kind": "particle",
            "sourceRecipe": {"enabled": True},
            "detail": {"particle": {
                "drag": 0, "rotationRangeDegrees": [0, 0],
                "spinRangeDegreesPerSecond": [0, 0], "subUVOverLife": False,
                "initialVelocity": {"uniformSolidAngle": False},
            }},
        }]}, "legacy")

    def test_material_color_space_omission_false_and_true_are_compatible(self) -> None:
        path = self.root / f"Data/Effects/Authored/{self.effect_id}.effect.json"
        source = json.loads(path.read_text(encoding="utf-8"))
        for flag in (None, False, True):
            with self.subTest(flag=flag):
                material = {"templateId": "effect.standard"}
                if flag is not None:
                    material["colorTexturesSRGB"] = flag
                source["elements"] = [{"material": material}]
                self._write_json(path, source)
                self.assertEqual(MODULE.validate_repository(self.root).direct_source_count, 1)
        MODULE._validate_authored_material_color_space({"elements": [{
            "material": {
                "templateId": "effect.source_material",
                "colorTexturesSRGB": False,
                "sourceProfile": {"enabled": True},
                "execution": {"enabled": True, "failClosed": True},
            },
            "sourceRecipe": {"enabled": True},
        }]}, "legacy")

    def test_material_color_space_presence_requires_boolean(self) -> None:
        path = self.root / f"Data/Effects/Authored/{self.effect_id}.effect.json"
        source = json.loads(path.read_text(encoding="utf-8"))
        for invalid in (None, 0, 1, "true", [], {}):
            with self.subTest(value=invalid):
                source["elements"] = [{"material": {
                    "templateId": "effect.standard", "colorTexturesSRGB": invalid,
                }}]
                self._write_json(path, source)
                with self.assertRaisesRegex(MODULE.ContractError, "colorTexturesSRGB must be a boolean"):
                    MODULE.validate_repository(self.root)

    def test_material_color_space_true_rejects_source_owned_execution(self) -> None:
        element = {"material": {"templateId": "effect.standard", "colorTexturesSRGB": True}}
        cases = (
            ("source-template", lambda e: e["material"].update(templateId="effect.source_material")),
            ("typed-template", lambda e: e["material"].update(templateId="effect.standard_color_v1")),
            ("missing-template", lambda e: e["material"].pop("templateId")),
            ("source-profile", lambda e: e["material"].update(sourceProfile={"enabled": True})),
            ("source-recipe", lambda e: e.update(sourceRecipe={"enabled": True})),
            ("execution-enabled", lambda e: e["material"].update(execution={"enabled": True})),
            ("execution-fail-closed", lambda e: e["material"].update(execution={"failClosed": True})),
            ("malformed-profile", lambda e: e["material"].update(sourceProfile=None)),
            ("malformed-recipe", lambda e: e.update(sourceRecipe=[])),
            ("malformed-execution", lambda e: e["material"].update(execution="disabled")),
        )
        for label, mutate in cases:
            with self.subTest(label=label):
                candidate = json.loads(json.dumps(element))
                mutate(candidate)
                with self.assertRaisesRegex(MODULE.ContractError, "ordinary authored standard material"):
                    MODULE._validate_authored_material_color_space({"elements": [candidate]}, "fixture")

    def test_owner_yaw_attachment_allows_shared_pair_and_default_bone(self) -> None:
        attachment = {
            "enabled": True, "follow": True, "orientation": "owner_yaw",
            "sourceAnchorSlotId": "bip001-l-hand", "runtimeAnchorSlotId": "manual.hand",
            "runtimeBoneName": "bip001-l-hand",
            "socketLocalTransform": {"position": [0, 0, 0], "rotationDegrees": [0, 0, 0], "scale": [1, 1, 1]},
        }
        source = {"version": 13, "elements": [
            {"id": "manual.hand.fire", "actionCueAttachment": attachment},
            {"id": "manual.hand.smoke", "actionCueAttachment": json.loads(json.dumps(attachment))},
        ]}
        MODULE._validate_attachment_orientations(source, "fixture")
        for element in source["elements"]:
            element["actionCueAttachment"].pop("orientation")
        MODULE._validate_attachment_orientations(source, "legacy")

    def test_owner_yaw_attachment_rejects_wrong_mode_and_conflicting_shared_id(self) -> None:
        attachment = {
            "enabled": True, "follow": True, "orientation": "owner_yaw",
            "sourceAnchorSlotId": "bip001-l-hand", "runtimeAnchorSlotId": "manual.hand",
            "runtimeBoneName": "bip001-l-hand",
            "socketLocalTransform": {"position": [0, 0, 0], "rotationDegrees": [0, 0, 0], "scale": [1, 1, 1]},
        }
        element = {"id": "manual.hand.fire", "actionCueAttachment": attachment}
        cases = [
            ("unknown", lambda s: s["elements"][0]["actionCueAttachment"].update(orientation="camera")),
            ("non-string", lambda s: s["elements"][0]["actionCueAttachment"].update(orientation=1)),
            ("disabled", lambda s: s["elements"][0]["actionCueAttachment"].update(enabled=False)),
            ("snapshot", lambda s: s["elements"][0]["actionCueAttachment"].update(follow=False)),
            ("source-version", lambda s: s.update(version=14)),
            ("source-recipe", lambda s: s["elements"][0].update(sourceRecipe={"enabled": True})),
            ("runtime-carrier", lambda s: s["elements"][0].update(runtimeCarrier={"kind": "cascadeRibbonV1"})),
            ("conflicting-bone", lambda s: s["elements"][1]["actionCueAttachment"].update(runtimeBoneName="bip001-r-hand")),
            ("conflicting-orientation", lambda s: s["elements"][1]["actionCueAttachment"].update(orientation="bone")),
            ("conflicting-socket", lambda s: s["elements"][1]["actionCueAttachment"]["socketLocalTransform"].update(position=[0, 1, 0])),
        ]
        baseline = {"version": 13, "elements": [element, json.loads(json.dumps(element))]}
        baseline["elements"][1]["id"] = "manual.hand.smoke"
        for label, mutate in cases:
            with self.subTest(label=label):
                candidate = json.loads(json.dumps(baseline))
                mutate(candidate)
                with self.assertRaises(MODULE.ContractError):
                    MODULE._validate_attachment_orientations(candidate, "fixture")
                MODULE._validate_attachment_orientations(baseline, "unchanged baseline")

    def test_repository_validation_consumes_attachment_orientation(self) -> None:
        path = self.root / f"Data/Effects/Authored/{self.effect_id}.effect.json"
        source = json.loads(path.read_text(encoding="utf-8"))
        source["elements"] = [{"actionCueAttachment": {"orientation": "unknown"}}]
        self._write_json(path, source)
        with self.assertRaisesRegex(MODULE.ContractError, "attachment orientation"):
            MODULE.validate_repository(self.root)

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

    def _write_valtan_draft(self, bindings: list[dict[str, object]]) -> None:
        self._write_json(
            self.root / MODULE.VALTAN_DRAFT_PATH,
            {
                "schema": "lostark.valtan-pattern-authoring-effects",
                "formatVersion": 1,
                "bossArchetypeId": "BOSS_VALTAN",
                "bindings": bindings,
            },
        )

    def _write_runtime_reachability_contract(self, effect_id: str) -> None:
        for relative in MODULE.PLAYER_EFFECT_EVENT_PATHS:
            path = self.root / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(
                f'"clip" EFFECT startms=0 payload="{effect_id}" effectref=asset\n',
                encoding="utf-8",
            )
        self._write_json(
            self.root / MODULE.VALTAN_CUE_PATH,
            {
                "schema": "lostark.valtan-pattern-effect-cues",
                "formatVersion": 4,
                "ownerArchetypeId": "BOSS_VALTAN",
                "cues": [],
            },
        )
        self._write_json(
            self.root / MODULE.VALTAN_BOSS_CATALOG_PATH,
            {
                "schema": "lostark.boss-catalog",
                "formatVersion": 5,
                "bosses": [
                    {"archetypeId": "BOSS_VALTAN", "combatObjectVisuals": []}
                ],
            },
        )
        self._write_json(
            self.root / MODULE.VALTAN_V1_ALIAS_PATH,
            {
                "schema": "lostark.valtan-pattern-effect-v1-aliases",
                "formatVersion": 1,
                "ownerArchetypeId": "BOSS_VALTAN",
                "aliases": [],
            },
        )

    def test_positive_is_read_only_and_reports_unbound_references(self) -> None:
        self._write_source("effect.reference.baseline")
        report = MODULE.validate_repository(self.root)
        self.assertEqual(report.direct_source_count, 1)
        self.assertEqual(report.unbound_reference_count, 1)

    def test_empty_draft_contract_rejects_orphaned_authored_source(self) -> None:
        self._write_valtan_draft([])
        self._write_source("effect.test.orphan")
        with self.assertRaisesRegex(MODULE.ContractError, "no Product or draft owner"):
            MODULE.validate_repository(self.root)

    def test_declared_draft_source_is_accepted(self) -> None:
        draft_id = "effect.test.draft"
        self._write_source(draft_id)
        self._write_valtan_draft(
            [
                {
                    "patternId": "VALTAN_TEST_DRAFT",
                    "effectAssetId": draft_id,
                    "authoringPath": f"Effects/Authored/{draft_id}.effect.json",
                    "state": "DRAFT_ATTACHED",
                }
            ]
        )
        report = MODULE.validate_repository(self.root)
        self.assertEqual(report.unbound_reference_count, 1)

    def test_player_skill_effect_must_resolve_to_product_catalog(self) -> None:
        self._write_json(
            self.root / MODULE.PLAYER_SKILLS_PATH,
            {"skills": [{"effectId": "effect.test.retired"}]},
        )
        with self.assertRaisesRegex(MODULE.ContractError, "PlayerSkills Effect references"):
            MODULE.validate_repository(self.root)

    def test_product_catalog_must_equal_runtime_reachability_set(self) -> None:
        self._write_runtime_reachability_contract(self.effect_id)
        self._write_valtan_draft([])
        self.assertEqual(MODULE.validate_repository(self.root).direct_source_count, 1)

        unused_id = "effect.test.unused"
        unused_row = {
            "effectAssetId": unused_id,
            "payloadKind": MODULE.DIRECT_KIND,
            "authoringPath": f"Effects/Authored/{unused_id}.effect.json",
        }
        self._write_source(unused_id)
        self._write_catalog([self.row, unused_row])
        with self.assertRaisesRegex(MODULE.ContractError, "no runtime consumer"):
            MODULE.validate_repository(self.root)

    def test_unbound_audition_row_is_excluded_from_product_reachability(self) -> None:
        audition_id = "effect.test.audition.unified"
        self._write_source(audition_id)
        self._write_audition_catalog([self._audition_row(audition_id)])
        self._write_runtime_reachability_contract(self.effect_id)
        self._write_valtan_draft([])

        report = MODULE.validate_repository(self.root)

        self.assertEqual(report.direct_source_count, 2)

    def test_audition_source_must_be_an_ordinary_product_row(self) -> None:
        source_audition_id = "effect.test.source-audition.unified"
        target_audition_id = "effect.test.target-audition.unified"
        self._write_source(source_audition_id)
        self._write_source(target_audition_id)
        source_row = self._audition_row(source_audition_id)
        target_row = self._audition_row(
            target_audition_id,
            source_effect_id=source_audition_id,
        )
        self._write_audition_catalog([source_row, target_row])

        with self.assertRaisesRegex(
            MODULE.ContractError, "source is not an ordinary Product row"
        ):
            MODULE.validate_repository(self.root)

    def test_audition_source_hash_must_match_current_raw_document(self) -> None:
        audition_id = "effect.test.audition.unified"
        self._write_source(audition_id)
        row = self._audition_row(audition_id, source_hash="0" * 64)
        self._write_audition_catalog([row])

        with self.assertRaisesRegex(
            MODULE.ContractError, "source hash mismatches current source document"
        ):
            MODULE.validate_repository(self.root)

    def test_product_reference_cannot_target_an_audition_row(self) -> None:
        audition_id = "effect.test.audition.unified"
        self._write_source(audition_id)
        self._write_audition_catalog([self._audition_row(audition_id)])
        self._write_runtime_reachability_contract(audition_id)
        self._write_valtan_draft([])

        with self.assertRaisesRegex(
            MODULE.ContractError, "runtime Effect references are missing from catalog"
        ):
            MODULE.validate_repository(self.root)

    def test_runtime_effect_reference_must_resolve_to_product_catalog(self) -> None:
        self._write_runtime_reachability_contract("effect.test.missing")
        self._write_valtan_draft([])
        with self.assertRaisesRegex(MODULE.ContractError, "missing from catalog"):
            MODULE.validate_repository(self.root)

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

    def test_external_runtime_resource_root_supports_lightweight_worktree(self) -> None:
        resource_id = "Effect/Test/shared.dds"
        self._set_source_resource(resource_id)
        with tempfile.TemporaryDirectory() as external_directory:
            external_root = Path(external_directory)
            resource_path = external_root / Path(resource_id)
            resource_path.parent.mkdir(parents=True)
            resource_path.write_bytes(b"DDS shared-worktree-resource")
            report = MODULE.validate_repository(
                self.root,
                resource_root=external_root,
                allow_local_resources=True,
            )
        self.assertEqual(report.resource_file_count, 1)
        self.assertGreater(report.resource_bytes, 4)

    def test_unsafe_runtime_resource_id_is_rejected(self) -> None:
        self._set_source_resource("../outside.dds")
        with self.assertRaisesRegex(MODULE.ContractError, "unsafe Resources-relative"):
            MODULE.validate_repository(self.root)

    def test_drive_owned_runtime_resource_is_validated_in_git_worktree(self) -> None:
        resource_id = "Effect/Test/untracked.dds"
        self._set_source_resource(resource_id)
        self._write_resource(resource_id, b"DDS untracked")
        self._git("init", "--quiet")
        report = MODULE.validate_repository(self.root)
        self.assertEqual(report.resource_file_count, 1)
        self.assertEqual(report.local_untracked_resource_count, 1)
        self.assertTrue(json.loads(report.as_json())["allowLocalResources"])
        with self.assertRaisesRegex(MODULE.ContractError, "not tracked by Git"):
            MODULE.validate_repository(self.root, allow_local_resources=False)
        self._write_resource(resource_id, b"not-a-dds")
        with self.assertRaisesRegex(MODULE.ContractError, "DDS magic is invalid"):
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

        for allow_local in (False, True):
            with self.subTest(allow_local_resources=allow_local):
                with self.assertRaisesRegex(MODULE.ContractError, "does not match working bytes"):
                    MODULE.validate_repository(self.root, allow_local_resources=allow_local)

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

#!/usr/bin/env python3

from __future__ import annotations

import copy
import json
import tempfile
import unittest
from pathlib import Path

from build_effect_components import (
    action_cues_for_effect,
    admitted_effect_variant,
    admitted_skills_for_class,
    bounded_runtime_display_name,
    build_all,
    component_directory,
    compile_assembly,
    remove_relocated_generated_components,
    remove_stale_generated_components,
    remove_unadmitted_generated_artifacts,
    read_product_effect_cues,
    renderer_kind,
    split_document,
    validate_ba_stage_contract,
    verify_existing,
)


def element(element_id: str, group_id: str, start: float) -> dict:
    return {
        "id": element_id,
        "displayName": element_id,
        "groupId": group_id,
        "sourceNode": f"source|{element_id}",
        "visible": True,
        "kind": "particle",
        "resources": [{"slotId": "base", "assetId": "Effect/Test.dds"}],
        "material": {
            "templateId": "effect.standard",
            "sourceMaterialPath": "source.material",
            "renderProfile": "alpha_two_sided_depth_read",
        },
        "detail": {"timing": {"startDelaySeconds": start}},
        "sourceRecipe": {"rendererShape": "sprite", "modules": []},
    }


def write_animevents(
    path: Path,
    animation_asset_id: str,
    rows: list[str],
) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    text = (
        f'LOSTARK_ANIM_EVENTS 5 "{animation_asset_id}" {len(rows)}\n' +
        "\n".join(rows)
    )
    if rows:
        text += "\n"
    path.write_text(text, encoding="utf-8")


def write_empty_product_contract(root: Path, animation_asset_id: str) -> None:
    stable_class = {
        "LanceMaster": "LANCE_MASTER",
        "Artist": "ARTIST",
        "Warlord": "WARLORD",
    }[animation_asset_id]
    balance = root / "Balance"
    authored = root / "Animation" / "Authored" / animation_asset_id
    balance.mkdir(parents=True, exist_ok=True)
    authored.mkdir(parents=True, exist_ok=True)
    (balance / "PlayerSkills.json").write_text(
        json.dumps({"skills": []}), encoding="utf-8"
    )
    (authored / f"{animation_asset_id}.skillbindings.json").write_text(
        json.dumps({
            "animationAssetId": animation_asset_id,
            "characterClass": stable_class,
            "bindings": [],
        }),
        encoding="utf-8",
    )
    write_animevents(
        authored / f"{animation_asset_id}.animevents",
        animation_asset_id,
        [],
    )


class EffectComponentTests(unittest.TestCase):
    def test_generated_component_display_name_obeys_runtime_utf8_limit(self) -> None:
        effect_id = (
            "effect.dimensionmaster.skill.2050240."
            "authored-baseline.clip2"
        )
        document = {
            "schema": "lostark.effect-authoring",
            "version": 12,
            "effectAssetId": effect_id,
            "displayName": effect_id,
            "particleSystem": {},
            "modelCues": [],
            "elements": [element("slash", "hit01", 0.0)],
        }
        _, first_outputs = split_document(document, "DimensionMaster")
        _, second_outputs = split_document(document, "DimensionMaster")
        first = first_outputs[0][1]
        second = second_outputs[0][1]
        self.assertEqual(first["displayName"], first["document"]["displayName"])
        self.assertEqual(first["displayName"], second["displayName"])
        self.assertLessEqual(len(first["displayName"].encode("utf-8")), 64)
        self.assertNotIn(".wfx", first["displayName"])

        oversized = "차원술사" * 30
        bounded = bounded_runtime_display_name(oversized)
        self.assertLessEqual(len(bounded.encode("utf-8")), 64)
        self.assertEqual(bounded, bounded_runtime_display_name(oversized))
        self.assertRegex(bounded, r"~[0-9a-f]{10}$")

    def test_ba_stage_contract_accepts_all_action_stage_skill_kinds(self) -> None:
        for skill_kind in ("COMBO", "HOLD", "COUNTER"):
            with self.subTest(skillKind=skill_kind):
                validate_ba_stage_contract(
                    {"skillKind": skill_kind, "comboStages": [{}, {}]},
                    f"effect.test.{skill_kind.casefold()}.ba2",
                    2,
                )
        with self.assertRaisesRegex(ValueError, "without action-stage contract"):
            validate_ba_stage_contract(
                {"skillKind": "ACTIVE", "comboStages": []},
                "effect.test.active.ba1",
                1,
            )

    def test_product_effect_cue_parser_matches_runtime_fail_closed_fields(
        self,
    ) -> None:
        effect_id = "effect.lancemaster.skill.34010.ba1"
        base = (
            '"attack" EFFECT startms=0 '
            f'payload="{effect_id}" effectref=asset '
            'anchor="root" follow=follow stop=natural '
            'px=0 py=0 pz=0 rx=0 ry=0 rz=0 sx=1 sy=1 sz=1'
        )
        invalid_rows = {
            "missing-startms": base.replace("startms=0 ", ""),
            "startms": base.replace("startms=0", "startms=-1"),
            "end-before-start": base.replace("startms=0", "startms=2 endms=1"),
            "cue-end-without-duration": base.replace(
                "stop=natural", "stop=cue_end"
            ),
            "empty-anchor": base.replace('anchor="root"', 'anchor=""'),
            "follow": base.replace("follow=follow", "follow=owner"),
            "stop": base.replace("stop=natural", "stop=loop"),
            "scale": base.replace("sx=1", "sx=0"),
            "bare-token": base + " malformed",
        }
        for label, row in invalid_rows.items():
            with self.subTest(label=label), tempfile.TemporaryDirectory() as temporary:
                path = Path(temporary) / "LanceMaster.animevents"
                write_animevents(path, "LanceMaster", [row])
                with self.assertRaises(ValueError):
                    read_product_effect_cues(
                        path, "LanceMaster", {"attack"}, {effect_id}
                    )

    def test_product_effect_cue_parser_rejects_unowned_and_duplicate_cues(
        self,
    ) -> None:
        effect_id = "effect.artist.skill.31000.ba1"
        row = (
            '"attack" EFFECT startms=0 '
            f'payload="{effect_id}" effectref=asset anchor="root"'
        )
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "Artist.animevents"
            write_animevents(path, "Artist", [row])
            with self.assertRaisesRegex(ValueError, "not owned"):
                read_product_effect_cues(
                    path, "Artist", {"other"}, {effect_id}
                )

            write_animevents(path, "Artist", [row, row])
            with self.assertRaisesRegex(ValueError, "duplicate admitted"):
                read_product_effect_cues(
                    path, "Artist", {"attack"}, {effect_id}
                )

    def _write_product_verification_fixture(
        self,
        root: Path,
        receipt_effect_ids: list[str],
    ) -> tuple[Path, Path, Path, Path, Path]:
        data = root / "Data"
        effect_id = "effect.lancemaster.skill.34010.ba1"
        authored = data / "Effects" / "Authored"
        animation = data / "Animation" / "Authored" / "LanceMaster"
        balance = data / "Balance"
        imported = data / "Effects" / "Imported" / "LanceMaster"
        authored.mkdir(parents=True)
        animation.mkdir(parents=True)
        balance.mkdir(parents=True)
        imported.mkdir(parents=True)
        catalog_path = data / "Effects" / "EffectCatalog.json"
        catalog_path.write_text(json.dumps({
            "effects": [{
                "effectAssetId": effect_id,
                "authoringPath": f"Effects/Authored/{effect_id}.effect.json",
            }],
        }), encoding="utf-8")
        (balance / "PlayerSkills.json").write_text(json.dumps({
            "skills": [{
                "skillId": 34010,
                "characterClass": "LANCE_MASTER",
                "inputSlot": "LMB",
                "skillKind": "COMBO",
                "setsStance": "NONE",
                "comboStages": [{}],
                "effectId": "",
            }],
        }), encoding="utf-8")
        bindings_path = animation / "LanceMaster.skillbindings.json"
        bindings_path.write_text(json.dumps({
            "animationAssetId": "LanceMaster",
            "characterClass": "LANCE_MASTER",
            "bindings": [{"skillId": 34010, "clips": [["attack_01"]]}],
        }), encoding="utf-8")
        animevents_path = animation / "LanceMaster.animevents"
        write_animevents(animevents_path, "LanceMaster", [
            f'"attack_01" EFFECT startms=0 payload="{effect_id}" '
            'effectref=asset anchor="root" follow=follow stop=natural',
        ])
        receipt_path = imported / "LanceMaster.component-build.receipt.json"
        receipt_path.write_text(json.dumps({
            "schema": "lostark.effect-component-build-receipt",
            "version": 1,
            "characterClass": "LANCE_MASTER",
            "animationAssetId": "LanceMaster",
            "effectCount": len(receipt_effect_ids),
            "componentCount": 0,
            "emitterCount": 0,
            "sourceActionCueCount": 0,
            "compileIdentityComplete": True,
            "effects": [
                {"effectAssetId": value} for value in receipt_effect_ids
            ],
        }), encoding="utf-8")
        return (
            catalog_path,
            data,
            bindings_path,
            animevents_path,
            receipt_path,
        )

    def test_product_cue_without_catalog_entry_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            skills_path = root / "PlayerSkills.json"
            bindings_path = root / "LanceMaster.skillbindings.json"
            animevents_path = root / "LanceMaster.animevents"
            skills_path.write_text(json.dumps({
                "skills": [{
                    "skillId": 34010,
                    "characterClass": "LANCE_MASTER",
                    "inputSlot": "LMB",
                    "skillKind": "COMBO",
                    "setsStance": "NONE",
                    "comboStages": [{}],
                    "effectId": "",
                }],
            }), encoding="utf-8")
            bindings_path.write_text(json.dumps({
                "animationAssetId": "LanceMaster",
                "characterClass": "LANCE_MASTER",
                "bindings": [{"skillId": 34010, "clips": [["attack"]]}],
            }), encoding="utf-8")
            write_animevents(animevents_path, "LanceMaster", [
                '"attack" EFFECT startms=0 '
                'payload="effect.lancemaster.skill.34010.ba1" '
                'effectref=asset anchor="root" follow=follow stop=natural',
            ])

            with self.assertRaisesRegex(ValueError, "missing from Effect catalog"):
                admitted_skills_for_class(
                    skills_path,
                    bindings_path,
                    animevents_path,
                    "LanceMaster",
                    set(),
                )

    def test_non_dimensionmaster_catalog_entry_requires_product_cue(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            skills_path = root / "PlayerSkills.json"
            bindings_path = root / "Artist.skillbindings.json"
            animevents_path = root / "Artist.animevents"
            skills_path.write_text(json.dumps({
                "skills": [{
                    "skillId": 31000,
                    "characterClass": "ARTIST",
                    "inputSlot": "LMB",
                    "skillKind": "COMBO",
                    "setsStance": "NONE",
                    "comboStages": [{}],
                    "effectId": "",
                }],
            }), encoding="utf-8")
            bindings_path.write_text(json.dumps({
                "animationAssetId": "Artist",
                "characterClass": "ARTIST",
                "bindings": [{"skillId": 31000, "clips": [["attack"]]}],
            }), encoding="utf-8")
            write_animevents(animevents_path, "Artist", [])

            with self.assertRaisesRegex(ValueError, "require an exact animation"):
                admitted_skills_for_class(
                    skills_path,
                    bindings_path,
                    animevents_path,
                    "Artist",
                    {"effect.artist.skill.31000.ba1"},
                )

    def test_movement_only_space_skill_is_not_product_admitted(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            effect_id = "effect.lancemaster.skill.34020"
            skills_path = root / "PlayerSkills.json"
            bindings_path = root / "LanceMaster.skillbindings.json"
            animevents_path = root / "LanceMaster.animevents"
            skills_path.write_text(json.dumps({
                "skills": [{
                    "skillId": 34020,
                    "characterClass": "LANCE_MASTER",
                    "inputSlot": "SPACE",
                    "skillKind": "ACTIVE",
                    "setsStance": "NONE",
                    "comboStages": [],
                    "effectId": "",
                }],
            }), encoding="utf-8")
            bindings_path.write_text(json.dumps({
                "animationAssetId": "LanceMaster",
                "characterClass": "LANCE_MASTER",
                "bindings": [{"skillId": 34020, "clips": ["dash"]}],
            }), encoding="utf-8")
            write_animevents(animevents_path, "LanceMaster", [
                f'"dash" EFFECT startms=0 payload="{effect_id}" '
                'effectref=asset anchor="root" follow=follow stop=natural',
            ])

            with self.assertRaisesRegex(ValueError, "no gameplay skill contract"):
                admitted_skills_for_class(
                    skills_path,
                    bindings_path,
                    animevents_path,
                    "LanceMaster",
                    {effect_id},
                )

    def test_artist_identity_control_slot_is_not_product_admitted(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            effect_id = "effect.artist.skill.31110"
            skills_path = root / "PlayerSkills.json"
            bindings_path = root / "Artist.skillbindings.json"
            animevents_path = root / "Artist.animevents"
            skills_path.write_text(json.dumps({
                "skills": [{
                    "skillId": 31110,
                    "characterClass": "ARTIST",
                    "inputSlot": "X",
                    "skillKind": "ACTIVE",
                    "setsStance": "NONE",
                    "comboStages": [],
                    "effectId": "",
                }],
            }), encoding="utf-8")
            bindings_path.write_text(json.dumps({
                "animationAssetId": "Artist",
                "characterClass": "ARTIST",
                "bindings": [{"skillId": 31110, "clips": ["identity"]}],
            }), encoding="utf-8")
            write_animevents(animevents_path, "Artist", [
                f'"identity" EFFECT startms=0 payload="{effect_id}" '
                'effectref=asset anchor="root" follow=follow stop=natural',
            ])

            with self.assertRaisesRegex(ValueError, "no gameplay skill contract"):
                admitted_skills_for_class(
                    skills_path,
                    bindings_path,
                    animevents_path,
                    "Artist",
                    {effect_id},
                )

    def test_verify_rejects_omitted_catalog_effect(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            catalog, data, bindings, events, receipt = (
                self._write_product_verification_fixture(root, [])
            )
            with self.assertRaisesRegex(ValueError, "catalog coverage mismatch"):
                verify_existing(
                    catalog, data,
                    data / "Effects" / "Components" / "LanceMaster",
                    data / "Effects" / "Assemblies" / "LanceMaster",
                    receipt, "LanceMaster", bindings, events,
                )

    def test_verify_rejects_duplicate_receipt_effect(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            effect_id = "effect.lancemaster.skill.34010.ba1"
            catalog, data, bindings, events, receipt = (
                self._write_product_verification_fixture(
                    root, [effect_id, effect_id]
                )
            )
            with self.assertRaisesRegex(ValueError, "catalog coverage mismatch"):
                verify_existing(
                    catalog, data,
                    data / "Effects" / "Components" / "LanceMaster",
                    data / "Effects" / "Assemblies" / "LanceMaster",
                    receipt, "LanceMaster", bindings, events,
                )

    def test_verify_rejects_extra_receipt_effect(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            catalog_id = "effect.lancemaster.skill.34010.ba1"
            extra_id = "effect.lancemaster.skill.34010.ba2"
            catalog, data, bindings, events, receipt = (
                self._write_product_verification_fixture(
                    root, [catalog_id, extra_id]
                )
            )
            with self.assertRaisesRegex(ValueError, "catalog coverage mismatch"):
                verify_existing(
                    catalog, data,
                    data / "Effects" / "Components" / "LanceMaster",
                    data / "Effects" / "Assemblies" / "LanceMaster",
                    receipt, "LanceMaster", bindings, events,
                )

    def test_verify_zero_product_class_does_not_require_receipt(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            catalog_path = root / "EffectCatalog.json"
            catalog_path.write_text(
                json.dumps({"effects": []}), encoding="utf-8"
            )
            write_empty_product_contract(root, "LanceMaster")

            result = verify_existing(
                catalog_path,
                root,
                root / "Components" / "LanceMaster",
                root / "Assemblies" / "LanceMaster",
                root / "Imported" / "LanceMaster.component-build.receipt.json",
                "LanceMaster",
            )

            self.assertEqual(0, result["effectCount"])
            self.assertEqual(0, result["componentCount"])
            self.assertTrue(result["compileIdentityComplete"])

    def test_verify_zero_product_class_rejects_class_owned_artifact(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            catalog_path = root / "EffectCatalog.json"
            catalog_path.write_text(
                json.dumps({"effects": []}), encoding="utf-8"
            )
            write_empty_product_contract(root, "Artist")
            component_root = root / "Components" / "Artist"
            component_root.mkdir(parents=True)
            (component_root / "stale.wfx.json").write_text(json.dumps({
                "schema": "lostark.effect-component",
                "version": 1,
                "source": {
                    "effectAssetId": "effect.artist.skill.31000.ba1",
                },
            }), encoding="utf-8")

            with self.assertRaisesRegex(
                ValueError, "unadmitted generated Effect artifacts"
            ):
                verify_existing(
                    catalog_path,
                    root,
                    component_root,
                    root / "Assemblies" / "Artist",
                    root / "Imported" / "Artist.component-build.receipt.json",
                    "Artist",
                )

    def test_non_dimensionmaster_catalog_cue_admits_empty_gameplay_effect_id(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            skills_path = root / "PlayerSkills.json"
            bindings_path = root / "LanceMaster.skillbindings.json"
            skills_path.write_text(json.dumps({
                "skills": [{
                    "skillId": 34010,
                    "characterClass": "LANCE_MASTER",
                    "inputSlot": "LMB",
                    "displayName": "basic attack",
                    "skillKind": "COMBO",
                    "comboStages": [{}, {}, {}, {}],
                    "effectId": "",
                }],
            }), encoding="utf-8")
            bindings_path.write_text(json.dumps({
                "animationAssetId": "LanceMaster",
                "characterClass": "LANCE_MASTER",
                "bindings": [{
                    "skillId": 34010,
                    "clips": [["attack_01"], ["attack_02"]],
                }],
            }), encoding="utf-8")
            animevents_path = root / "LanceMaster.animevents"
            write_animevents(animevents_path, "LanceMaster", [
                '"attack_01" EFFECT startms=0 '
                'payload="effect.lancemaster.skill.34010.ba1" '
                'effectref=asset anchor="root" follow=follow stop=natural',
            ])

            admitted = admitted_skills_for_class(
                skills_path,
                bindings_path,
                animevents_path,
                "LanceMaster",
                {"effect.lancemaster.skill.34010.ba1"},
            )

            self.assertEqual(1, len(admitted))
            self.assertEqual("effect.lancemaster.skill.34010", admitted[0]["effectAssetId"])
            self.assertEqual("BA", admitted[0]["inputSlot"])
            self.assertEqual(["attack_01", "attack_02"], admitted[0]["clips"])

    def test_non_dimensionmaster_existing_gameplay_effect_id_must_match(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            skills_path = root / "PlayerSkills.json"
            bindings_path = root / "Artist.skillbindings.json"
            skills_path.write_text(json.dumps({
                "skills": [{
                    "skillId": 31000,
                    "characterClass": "ARTIST",
                    "inputSlot": "LMB",
                    "skillKind": "COMBO",
                    "comboStages": [{}],
                    "effectId": "effect.artist.skill.wrong",
                }],
            }), encoding="utf-8")
            bindings_path.write_text(json.dumps({
                "animationAssetId": "Artist",
                "characterClass": "ARTIST",
                "bindings": [{"skillId": 31000, "clips": ["attack"]}],
            }), encoding="utf-8")
            animevents_path = root / "Artist.animevents"
            write_animevents(animevents_path, "Artist", [
                '"attack" EFFECT startms=0 '
                'payload="effect.artist.skill.31000.ba1" '
                'effectref=asset anchor="root" follow=follow stop=natural',
            ])

            with self.assertRaisesRegex(ValueError, "identity mismatch"):
                admitted_skills_for_class(
                    skills_path,
                    bindings_path,
                    animevents_path,
                    "Artist",
                    {"effect.artist.skill.31000.ba1"},
                )

    def test_combo_authored_effect_builds_without_source_action_recipe(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            data = root / "Data"
            authored = data / "Effects" / "Authored"
            bindings = data / "Animation" / "Authored" / "Warlord"
            balance = data / "Balance"
            authored.mkdir(parents=True)
            bindings.mkdir(parents=True)
            balance.mkdir(parents=True)
            effect_id = "effect.warlord.skill.17000.ba1"
            document_path = authored / f"{effect_id}.effect.json"
            document_path.write_text(json.dumps({
                "schema": "lostark.effect-authoring",
                "version": 12,
                "effectAssetId": effect_id,
                "displayName": "Warlord BA1",
                "particleSystem": {},
                "modelCues": [],
                "elements": [element("slash", "hit01", 0.3)],
            }), encoding="utf-8")
            catalog_path = data / "Effects" / "EffectCatalog.json"
            catalog_path.write_text(json.dumps({
                "effects": [{
                    "effectAssetId": effect_id,
                    "authoringPath": (
                        f"Effects/Authored/{effect_id}.effect.json"
                    ),
                }],
            }), encoding="utf-8")
            (balance / "PlayerSkills.json").write_text(json.dumps({
                "skills": [{
                    "skillId": 17000,
                    "characterClass": "WARLORD",
                    "inputSlot": "LMB",
                    "skillKind": "COMBO",
                    "comboStages": [{}],
                    "effectId": "",
                }],
            }), encoding="utf-8")
            bindings_path = bindings / "Warlord.skillbindings.json"
            bindings_path.write_text(json.dumps({
                "animationAssetId": "Warlord",
                "characterClass": "WARLORD",
                "bindings": [{"skillId": 17000, "clips": [["attack_01"]]}],
            }), encoding="utf-8")
            write_animevents(
                bindings / "Warlord.animevents",
                "Warlord",
                [
                    f'"attack_01" EFFECT startms=0 payload="{effect_id}" '
                    'effectref=asset anchor="root" follow=follow stop=natural',
                ],
            )

            receipt = build_all(
                catalog_path,
                data,
                data / "Effects" / "Components" / "Warlord",
                data / "Effects" / "Assemblies" / "Warlord",
                bindings_path,
                "Warlord",
            )

            self.assertEqual(1, receipt["effectCount"])
            self.assertEqual(0, receipt["sourceActionCueCount"])
            self.assertTrue(receipt["compileIdentityComplete"])

    def test_cascade_renderer_shape_and_mesh_binding_must_agree(self) -> None:
        sprite = element("sprite", "group.a", 0.0)
        sprite["sourceRecipe"]["enabled"] = True
        self.assertEqual("sprite", renderer_kind(sprite))

        mesh = copy.deepcopy(sprite)
        mesh["id"] = "mesh"
        mesh["sourceRecipe"]["rendererShape"] = "mesh"
        mesh["resources"].append({
            "slotId": "meshModel",
            "assetId": "Effect/Test.wmodel",
        })
        self.assertEqual("mesh", renderer_kind(mesh))

        contradictory = copy.deepcopy(mesh)
        contradictory["sourceRecipe"]["rendererShape"] = "sprite"
        with self.assertRaisesRegex(ValueError, "shape/resource contradiction"):
            renderer_kind(contradictory)

    def test_invalid_source_action_cue_time_is_preserved_but_fail_closed(self) -> None:
        payload = {
            "encoding": "base64",
            "sha256": "a" * 64,
            "data": "AA==",
        }
        recipe = {"cues": [{
            "cueId": "skill-1/clip-000/notify-001",
            "localTimeSeconds": 1.0e22,
            "globalTimeSeconds": 1.0e22,
            "durationSeconds": -3.0,
            "runtimeChannel": "CHARACTER_MATERIAL",
            "serializedPayload": payload,
            "executionEnabled": True,
            "sourceExecutionStatus": "SEMANTIC_EXECUTION_AUDIT_REQUIRED",
        }]}

        cues = action_cues_for_effect(recipe, "effect.dimensionmaster.skill.1")

        self.assertEqual(1.0e22, cues[0]["localTimeSeconds"])
        self.assertEqual(payload, cues[0]["serializedPayload"])
        self.assertFalse(cues[0]["executionEnabled"])
        self.assertEqual(
            "INVALID_SOURCE_TIME_FAIL_CLOSED",
            cues[0]["sourceExecutionStatus"],
        )
        self.assertTrue(recipe["cues"][0]["executionEnabled"])

    def test_finite_source_action_cue_time_remains_executable(self) -> None:
        recipe = {"cues": [{
            "cueId": "skill-1/clip-000/notify-001",
            "localTimeSeconds": 1.25,
            "globalTimeSeconds": 1.25,
            "durationSeconds": 0.5,
            "executionEnabled": True,
            "sourceExecutionStatus": "RUNTIME_TYPED",
        }]}
        cues = action_cues_for_effect(recipe, "effect.dimensionmaster.skill.1")
        self.assertTrue(cues[0]["executionEnabled"])
        self.assertEqual("RUNTIME_TYPED", cues[0]["sourceExecutionStatus"])

    def test_component_identity_never_depends_on_input_slot(self) -> None:
        document = {
            "schema": "lostark.effect-authoring",
            "version": 10,
            "effectAssetId": "effect.dimensionmaster.skill.42",
            "displayName": "test",
            "particleSystem": {},
            "modelCues": [],
            "elements": [element("a", "group.a", 0.0)],
        }
        _assembly_q, files_q = split_document(
            document, "DimensionMaster", "Q"
        )
        _assembly_s, files_s = split_document(
            document, "DimensionMaster", "S"
        )

        self.assertEqual(files_q[0][0], files_s[0][0])
        self.assertEqual(
            files_q[0][1]["componentAssetId"],
            files_s[0][1]["componentAssetId"],
        )
        self.assertEqual(
            "effect.component.dimensionmaster.skill.42.00",
            files_q[0][1]["componentAssetId"],
        )

    def test_ba_stage_identity_has_distinct_component_directory(self) -> None:
        self.assertEqual(
            "skill.2050010.ba1",
            component_directory(
                "effect.dimensionmaster.skill.2050010.ba1",
                "DimensionMaster",
            ),
        )

    def test_authored_baseline_variant_maps_to_canonical_skill(self) -> None:
        self.assertEqual(
            ("effect.dimensionmaster.skill.2050210", "authored-baseline"),
            admitted_effect_variant(
                "effect.dimensionmaster.skill.2050210.authored-baseline"
            ),
        )
        self.assertEqual(
            ("effect.dimensionmaster.skill.2050010", None),
            admitted_effect_variant(
                "effect.dimensionmaster.skill.2050010.ba4"
            ),
        )
        self.assertEqual(
            (
                "effect.dimensionmaster.skill.2050240",
                "authored-baseline.clip2",
            ),
            admitted_effect_variant(
                "effect.dimensionmaster.skill.2050240.authored-baseline.clip2"
            ),
        )
        self.assertEqual(
            ("effect.lancemaster.skill.34140", "ba2.clip2"),
            admitted_effect_variant(
                "effect.lancemaster.skill.34140.ba2.clip2"
            ),
        )
        with self.assertRaisesRegex(ValueError, "invalid Authored Baseline"):
            admitted_effect_variant(
                "effect.dimensionmaster.skill.2050240.authored-baseline.clip0"
            )
        self.assertNotEqual(
            component_directory(
                "effect.dimensionmaster.skill.2050010.ba1",
                "DimensionMaster",
            ),
            component_directory(
                "effect.dimensionmaster.skill.2050010.ba2",
                "DimensionMaster",
            ),
        )

    def test_split_compile_identity(self) -> None:
        document = {
            "schema": "lostark.effect-authoring",
            "version": 10,
            "effectAssetId": "effect.dimensionmaster.skill.1",
            "displayName": "test",
            "particleSystem": {},
            "modelCues": [],
            "elements": [
                element("a", "group.a", 1.25),
                element("b", "group.a", 1.5),
                element("c", "group.b", 0.25),
            ],
        }
        assembly, files = split_document(document, "DimensionMaster", "S")
        components = {row["componentAssetId"]: row for _, row in files}
        compiled = compile_assembly(assembly, components)
        self.assertEqual(document, compiled)
        self.assertEqual(2, len(files))
        self.assertEqual(
            0.0,
            files[0][1]["document"]["elements"][0]
            ["detail"]["timing"]["startDelaySeconds"],
        )
        self.assertEqual(
            files[0][1]["emitters"][0]["emitterId"],
            files[0][1]["emitters"][0]["elementId"],
        )

    def test_missing_component_rolls_back_output_value(self) -> None:
        assembly = {
            "schema": "lostark.effect-assembly",
            "version": 1,
            "effectAssetId": "effect.test",
            "displayName": "test",
            "sourceAuthoringVersion": 10,
            "componentCues": [{
                "componentAssetId": "effect.component.missing",
                "startDelaySeconds": 0.0,
            }],
        }
        before = copy.deepcopy(assembly)
        with self.assertRaises(ValueError):
            compile_assembly(assembly, {})
        self.assertEqual(before, assembly)

    def test_stale_generated_component_cleanup_is_source_scoped(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)

            def write(name: str, effect_id: str) -> None:
                (root / name).write_text(json.dumps({
                    "schema": "lostark.effect-component",
                    "version": 1,
                    "source": {"effectAssetId": effect_id},
                }), encoding="utf-8")

            write("current.wfx.json", "effect.test")
            write("stale.wfx.json", "effect.test")
            write("foreign.wfx.json", "effect.foreign")
            removed = remove_stale_generated_components(
                root, {"current.wfx.json"}, "effect.test"
            )
            self.assertEqual(["stale.wfx.json"], removed)
            self.assertTrue((root / "current.wfx.json").exists())
            self.assertTrue((root / "foreign.wfx.json").exists())
            self.assertFalse((root / "stale.wfx.json").exists())

    def test_relocated_component_cleanup_preserves_foreign_sources(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            old = root / "Q"
            expected = root / "skill.10"
            old.mkdir()
            expected.mkdir()

            def write(path: Path, effect_id: str) -> None:
                path.write_text(json.dumps({
                    "schema": "lostark.effect-component",
                    "version": 1,
                    "source": {"effectAssetId": effect_id},
                }), encoding="utf-8")

            write(old / "relocated.wfx.json", "effect.dimensionmaster.skill.10")
            write(old / "foreign.wfx.json", "effect.dimensionmaster.skill.20")
            write(expected / "current.wfx.json", "effect.dimensionmaster.skill.10")

            removed = remove_relocated_generated_components(
                root, expected, "effect.dimensionmaster.skill.10"
            )

            self.assertEqual([str(old / "relocated.wfx.json")], removed)
            self.assertFalse((old / "relocated.wfx.json").exists())
            self.assertTrue((old / "foreign.wfx.json").exists())
            self.assertTrue((expected / "current.wfx.json").exists())

    def test_unadmitted_cleanup_removes_only_generated_dimensionmaster_files(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            components = root / "Components"
            assemblies = root / "Assemblies"
            components.mkdir()
            assemblies.mkdir()

            def write_component(name: str, effect_id: str) -> None:
                (components / name).write_text(json.dumps({
                    "schema": "lostark.effect-component",
                    "version": 1,
                    "source": {"effectAssetId": effect_id},
                }), encoding="utf-8")

            def write_assembly(name: str, effect_id: str) -> None:
                (assemblies / name).write_text(json.dumps({
                    "schema": "lostark.effect-assembly",
                    "version": 1,
                    "effectAssetId": effect_id,
                }), encoding="utf-8")

            admitted = "effect.dimensionmaster.skill.10"
            candidate = "effect.dimensionmaster.skill.99"
            write_component("admitted.wfx.json", admitted)
            write_component("candidate.wfx.json", candidate)
            write_component("foreign.wfx.json", "effect.other.skill.99")
            write_assembly("admitted.assembly.json", admitted)
            write_assembly("candidate.assembly.json", candidate)
            write_assembly("foreign.assembly.json", "effect.other.skill.99")

            removed_components, removed_assemblies = (
                remove_unadmitted_generated_artifacts(
                    components, assemblies, {admitted}
                )
            )

            self.assertEqual(
                [str(components / "candidate.wfx.json")],
                removed_components,
            )
            self.assertEqual(
                [str(assemblies / "candidate.assembly.json")],
                removed_assemblies,
            )
            self.assertTrue((components / "admitted.wfx.json").exists())
            self.assertTrue((components / "foreign.wfx.json").exists())
            self.assertTrue((assemblies / "admitted.assembly.json").exists())
            self.assertTrue((assemblies / "foreign.assembly.json").exists())

    def test_unadmitted_cleanup_is_scoped_to_requested_product_class(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            components = root / "Components"
            assemblies = root / "Assemblies"
            components.mkdir()
            assemblies.mkdir()
            artist_id = "effect.artist.skill.31000.ba1"
            warlord_id = "effect.warlord.skill.17000.ba1"
            (components / "artist.wfx.json").write_text(json.dumps({
                "schema": "lostark.effect-component",
                "version": 1,
                "source": {"effectAssetId": artist_id},
            }), encoding="utf-8")
            (components / "warlord.wfx.json").write_text(json.dumps({
                "schema": "lostark.effect-component",
                "version": 1,
                "source": {"effectAssetId": warlord_id},
            }), encoding="utf-8")
            (assemblies / "artist.assembly.json").write_text(json.dumps({
                "schema": "lostark.effect-assembly",
                "version": 1,
                "effectAssetId": artist_id,
            }), encoding="utf-8")

            removed_components, removed_assemblies = (
                remove_unadmitted_generated_artifacts(
                    components,
                    assemblies,
                    set(),
                    "Artist",
                )
            )

            self.assertEqual([str(components / "artist.wfx.json")], removed_components)
            self.assertEqual([str(assemblies / "artist.assembly.json")], removed_assemblies)
            self.assertTrue((components / "warlord.wfx.json").exists())


if __name__ == "__main__":
    unittest.main()

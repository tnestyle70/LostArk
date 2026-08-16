import copy
import hashlib
import json
import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
MANIFEST_PATH = (
    ROOT
    / "Data/Effects/AuthoredCorrections/Warlord/Warlord.user-role-composition.json"
)
RESOURCES_ROOT = ROOT / "Client/Bin/Resources"
EVIDENCE_ROOT = (
    ROOT
    / ".md/GB/08-15/assets/2026-08-15_FOUR_CLASS_VALTAN_VISUAL_REVIEW/워로드원본"
)
DESKTOP_EVIDENCE_ROOT = Path(
    r"C:\Users\user\Desktop\로스트아크이펙트이미지\워로드"
)


def load_json(path: Path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def selected_ids(skill):
    return [stable_id for role in skill.get("roles", []) for stable_id in role["stableIds"]]


def execution_class(element):
    execution = element.get("material", {}).get("execution", {})
    if execution.get("authoringApproximate"):
        return "AUTHORING_APPROXIMATE"
    if execution.get("failClosed"):
        return "HARD_FAIL_CLOSED"
    return "FULL"


def normalized_duplicate_payload(element):
    value = copy.deepcopy(element)
    for key in ("id", "displayName", "sourceNode", "visible"):
        value.pop(key, None)
    presentation = value.get("sourcePresentation", {})
    for key in ("sourceEventId", "sourceOccurrenceIndex", "sourceTimeSeconds"):
        presentation.pop(key, None)
    return value


def source_event_number(element):
    presentation = element.get("sourcePresentation", {})
    haystack = " ".join(
        (
            str(presentation.get("sourceEventId", "")),
            element.get("displayName", ""),
            element.get("sourceNode", ""),
        )
    )
    match = re.search(r"source-event-(\d+)", haystack)
    return int(match.group(1)) if match else 1_000_000


def resource_signature(elements):
    rows = []
    for element in elements:
        for resource in element.get("resources", []):
            if resource.get("assetId"):
                rows.append(
                    f"{element['id']}|R|{resource['slotId']}|{resource['assetId']}"
                )
        source_profile = element.get("material", {}).get("sourceProfile", {})
        for texture in source_profile.get("textures", []):
            if texture.get("assetId"):
                rows.append(
                    f"{element['id']}|T|{texture['name']}|{texture['assetId']}"
                )
    return hashlib.sha256("\n".join(sorted(rows)).encode("utf-8")).hexdigest()


def selected_id_sha(elements):
    return hashlib.sha256(
        "\n".join(sorted(element["id"] for element in elements)).encode("utf-8")
    ).hexdigest()


def physical_asset_ids(element):
    assets = [
        row.get("assetId", "")
        for row in element.get("resources", [])
        if row.get("assetId")
    ]
    assets.extend(
        row.get("assetId", "")
        for row in element.get("material", {})
        .get("sourceProfile", {})
        .get("textures", [])
        if row.get("assetId")
    )
    return assets


class WarlordRoleCompositionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.manifest = load_json(MANIFEST_PATH)
        cls.skills = cls.manifest["skills"]
        cls.materialized = [row for row in cls.skills if row.get("effectDocument")]
        cls.projected = cls.manifest["provisionalApproximatePromotions"]
        cls.projected_by_document = {}
        for row in cls.projected:
            cls.projected_by_document.setdefault(row["effectDocument"], []).append(
                row["stableId"]
            )

    def test_authority_skill_and_clip_mapping(self):
        self.assertEqual("lostark.effect-role-composition", self.manifest["schema"])
        self.assertEqual(1, self.manifest["formatVersion"])
        self.assertEqual("WARLORD", self.manifest["characterClass"])

        player_skills = load_json(ROOT / "Data/Balance/PlayerSkills.json")["skills"]
        authoritative_by_slot = {}
        for row in player_skills:
            if row.get("characterClass") == "WARLORD":
                authoritative_by_slot.setdefault(row["inputSlot"], []).append(row)
        bindings = load_json(
            ROOT / "Data/Animation/Authored/Warlord/Warlord.skillbindings.json"
        )["bindings"]
        clips_by_skill = {int(row["skillId"]): row["clips"] for row in bindings}

        for skill in self.skills:
            candidates = authoritative_by_slot[skill["inputSlot"]]
            if "requiredStance" in skill:
                candidates = [
                    row
                    for row in candidates
                    if row.get("requiredStance") == skill["requiredStance"]
                    and row.get("setsStance") == skill["setsStance"]
                ]
            self.assertEqual(1, len(candidates), skill["inputSlot"])
            self.assertEqual(skill["skillId"], int(candidates[0]["skillId"]))
            clips = clips_by_skill[skill["skillId"]]
            if "comboStage" in skill:
                clips = clips[int(skill["comboStage"]) - 1]
            elif "clipIndex" in skill:
                clips = [clips[int(skill["clipIndex"]) - 1]]
            self.assertEqual(skill["clips"], clips)

        binding_17820 = next(row for row in bindings if int(row["skillId"]) == 17820)
        self.assertEqual(
            [
                "wgl_sk_shieldofbattlefield_01",
                "wgl_sk_shieldofbattlefield_02",
                "wgl_sk_shieldofbattlefield_03",
            ],
            binding_17820["clips"],
        )

    def test_selected_visibility_classification_and_resources(self):
        selected_total = 0
        for skill in self.materialized:
            document = load_json(ROOT / skill["effectDocument"])
            by_id = {element["id"]: element for element in document["elements"]}
            expected_ids = selected_ids(skill)
            projected_ids = self.projected_by_document.get(skill["effectDocument"], [])
            visible_ids = [
                element["id"] for element in document["elements"] if element.get("visible")
            ]
            self.assertEqual(len(expected_ids), len(set(expected_ids)), skill["effectDocument"])
            self.assertEqual(
                set(expected_ids) | set(projected_ids),
                set(visible_ids),
                skill["effectDocument"],
            )
            selected = [by_id[stable_id] for stable_id in expected_ids]
            selected_total += len(selected) + len(projected_ids)

            receipt = skill["selectionReceipt"]
            self.assertEqual(receipt["visibleAfter"], len(selected))
            self.assertEqual(
                receipt["visibleBefore"],
                receipt["visibleAfter"]
                + receipt["hiddenUnpositionedCount"]
                + receipt["explicitRoleExcludedCount"],
            )
            self.assertEqual(receipt["selectedIdsSha256"], selected_id_sha(selected))
            self.assertEqual(
                receipt["selectedResourceSignatureSha256"],
                resource_signature(selected),
            )
            self.assertEqual(
                receipt["selectedFullCount"],
                sum(execution_class(element) == "FULL" for element in selected),
            )
            self.assertEqual(
                receipt["selectedApproximateCount"],
                sum(
                    execution_class(element) == "AUTHORING_APPROXIMATE"
                    for element in selected
                ),
            )

            for element in document["elements"]:
                classification = execution_class(element)
                if element.get("visible"):
                    self.assertIn(
                        classification,
                        ("FULL", "AUTHORING_APPROXIMATE"),
                        element["id"],
                    )
                    runtime_profile = (
                        element.get("material", {})
                        .get("sourceProfile", {})
                        .get("runtimeShaderProfileId", "")
                    )
                    self.assertNotEqual(
                        "effect.ue3.fallback-blocked.v1", runtime_profile, element["id"]
                    )
                if classification == "HARD_FAIL_CLOSED":
                    self.assertFalse(element.get("visible"), element["id"])

            for element in selected:
                for asset_id in physical_asset_ids(element):
                    if asset_id.lower().endswith((".dds", ".wmodel")):
                        self.assertTrue(
                            (RESOURCES_ROOT / Path(asset_id)).is_file(),
                            f"missing selected resource: {asset_id}",
                        )

        self.assertEqual(328, selected_total)

    def test_duplicate_groups_are_exact_hidden_unpositioned_rows(self):
        hidden_total = 0
        for skill in self.materialized:
            document = load_json(ROOT / skill["effectDocument"])
            by_id = {element["id"]: element for element in document["elements"]}
            for group in skill["duplicateGroups"]:
                self.assertEqual("HIDDEN_UNPOSITIONED", group["typedReason"])
                keep = by_id[group["keepStableId"]]
                self.assertTrue(keep["visible"], keep["id"])
                keep_payload = normalized_duplicate_payload(keep)
                keep_event = source_event_number(keep)
                for stable_id in group["hiddenStableIds"]:
                    hidden = by_id[stable_id]
                    self.assertFalse(hidden["visible"], hidden["id"])
                    self.assertEqual(
                        keep_payload,
                        normalized_duplicate_payload(hidden),
                        hidden["id"],
                    )
                    self.assertLessEqual(keep_event, source_event_number(hidden))
                    hidden_total += 1
            for row in skill["explicitlyHiddenRoleRows"]:
                self.assertFalse(by_id[row["stableId"]]["visible"], row["stableId"])
        self.assertEqual(214, hidden_total)

    def test_ba_mesh_only_and_a_chain_approximate_boundary(self):
        ba_skills = [row for row in self.materialized if row["skillId"] == 17000]
        self.assertEqual([2, 2, 6], [len(selected_ids(row)) for row in ba_skills])
        for skill in ba_skills:
            document = load_json(ROOT / skill["effectDocument"])
            by_id = {element["id"]: element for element in document["elements"]}
            for stable_id in selected_ids(skill):
                self.assertEqual("mesh", by_id[stable_id]["sourceRecipe"]["rendererShape"])

        skill_a = next(row for row in self.materialized if row["skillId"] == 17090)
        document_a = load_json(ROOT / skill_a["effectDocument"])
        by_id = {element["id"]: element for element in document_a["elements"]}
        selected = [by_id[stable_id] for stable_id in selected_ids(skill_a)]
        approximate = [
            element
            for element in selected
            if execution_class(element) == "AUTHORING_APPROXIMATE"
        ]
        self.assertEqual(12, len(approximate))
        self.assertEqual(2, sum(execution_class(element) == "FULL" for element in selected))
        for element in approximate:
            self.assertIn("par_d_chain_attraction_", element["sourceNode"].lower())
            assets = physical_asset_ids(element)
            self.assertIn(
                "Effect/Warlord/Textures/FX_TEX_02/fx_d_atypical_028.dds",
                assets,
            )
            self.assertNotIn(
                "Effect/Warlord/Textures/FX_TEX_02/fx_d_grid_016.dds", assets
            )
        self.assertFalse(by_id["authored.source-particle.90630c0e6042f73e91f135c2"]["visible"])
        self.assertFalse(by_id["authored.source-particle.c14fb06d85bbf7aac4178fbf"]["visible"])

    def test_manifest_projected_rows_are_visible_fail_closed_approximate(self):
        provisional = self.manifest["provisionalApproximatePromotions"]
        self.assertEqual(16, len(provisional))
        self.assertEqual(
            "PROJECTED",
            self.manifest["authoringApproximateProjection"]["status"],
        )
        documents = {}
        for row in provisional:
            self.assertEqual("AUTHORING_APPROXIMATE_PROJECTED", row["status"])
            document = documents.setdefault(
                row["effectDocument"], load_json(ROOT / row["effectDocument"])
            )
            by_id = {element["id"]: element for element in document["elements"]}
            element = by_id[row["stableId"]]
            self.assertTrue(element["visible"], row["stableId"])
            self.assertEqual("AUTHORING_APPROXIMATE", execution_class(element))
            self.assertEqual(
                "effect.ue3.grouped-translucent.v1",
                element.get("material", {})
                .get("sourceProfile", {})
                .get("runtimeShaderProfileId", ""),
            )
            self.assertEqual(
                {
                    "enabled": False,
                    "failClosed": True,
                    "authoringApproximate": True,
                },
                element["material"]["execution"],
            )
            self.assertTrue(row["currentTypedReason"])
            for resource in row["resources"]:
                asset_id = resource["assetId"]
                self.assertTrue((RESOURCES_ROOT / Path(asset_id)).is_file(), asset_id)

    def test_z_source_intake_and_visual_count_mismatch_stay_locked(self):
        skill = next(row for row in self.skills if row["inputSlot"] == "Z")
        self.assertEqual(17800, skill["skillId"])
        self.assertEqual("SOURCE_INTAKE_BLOCKED", skill["status"])
        self.assertIsNone(skill["effectDocument"])
        self.assertEqual(5, skill["sourceDecodedShieldBurstCount"])
        self.assertEqual(7, skill["sourceDecodedPeakActiveCount"])
        self.assertIn("SOURCE_VISUAL_SHIELD_COUNT_MISMATCH", skill["additionalBlockers"])
        self.assertIn("SOURCE_DEFAULT_ATTACHMENT_UNRESOLVED", skill["additionalBlockers"])
        self.assertEqual("B_EffectRoot", skill["sourceOccurrences"][1]["attachment"])
        self.assertEqual(
            "SOURCE_DEFAULT_UNRESOLVED", skill["sourceOccurrences"][0]["attachment"]
        )

        animevents = (
            ROOT / "Data/Animation/Authored/Warlord/Warlord.animevents"
        ).read_text(encoding="utf-8-sig")
        self.assertIn(
            '"wgl_sk_defence_start" EFFECT startms=297 payload="FX_PC_WGL_06.Par_O_WGL_ApShield_03_02" src=orig',
            animevents,
        )
        self.assertIn(
            '"wgl_sk_defence_start" EFFECT startms=299 payload="FX_PC_WGL_06.Par_O_WGL_ApShield_03_01" src=orig',
            animevents,
        )
        self.assertNotIn("effect.warlord.skill.17800.unified", animevents)

        for model in skill["provenModels"]:
            path = RESOURCES_ROOT / Path(model["assetId"])
            self.assertTrue(path.is_file(), model["assetId"])
            payload = path.read_bytes()
            self.assertEqual(model["bytes"], len(payload))
            self.assertEqual(model["sha256"], hashlib.sha256(payload).hexdigest())

    def test_visual_evidence_copies_and_unbound_17820_lock(self):
        expected_hashes = {
            "Warlord_Q00.png": "94377e6860475854f5832e0f61b54e3c3e3ca09c9cf9ddfe0eccbb46aaaef59e",
            "Warlord_W00.png": "d791691b0ae7bd0bd542347a4126ff36f08eb2f5cd7c0ab17ccb342d14d35727",
            "Warlord_E00.png": "454c958a2a624148be5e8c015bc4aa0a931f124f25d2be6758aeca042b52a6bd",
            "Warlord_R00.png": "6c77dd55a7682b86485ca45bc6a3e777ebf7ed5c49b3a35581322efa373478f2",
            "Warlord_A00.png": "deac9d5c5f7b306f6f08ea52fcd2e73b633114b7d195527080383f134ad8caa6",
            "Warlord_Z00.png": "deac9d5c5f7b306f6f08ea52fcd2e73b633114b7d195527080383f134ad8caa6",
            "Warlord_Z01.png": "d08970cf7d7058455238ad7f9ba0a9d356e06d02c53750f26ca66ce1d37ce1b1",
            "Warlord_S00.png": "1c1f5844027591e4b53201da95117026f2498c1d90dc8b8fc664b6374eb70b1c",
            "Warlord_D00.png": "f3bb35b0a085c3903d422000a2eff6e3a7fef78377f10b1797eec7c65fe26031",
            "Warlord_F00.png": "33667bd4e3a64c7cc9d86424af4c94a82ff142831858bcdb31bcf716e79a029e",
            "Warlord_F01.png": "c98784f842d858885b3873c4b8b3cb968a12978da355a6c4eb55da5313965880",
            "Warlord_V00.png": "c00db0b0545a1365ca1debaa0662a301f416a0b518abd27bc5002a118408e442",
            "Warlord_V01.png": "3f224298dce0929ab84553097c28c2ef2237c19b18c5d7dca69c7c9e730da956",
        }
        for filename, expected_hash in expected_hashes.items():
            for root in (EVIDENCE_ROOT, DESKTOP_EVIDENCE_ROOT):
                payload = (root / filename).read_bytes()
                self.assertEqual(expected_hash, hashlib.sha256(payload).hexdigest())

        animevents = (
            ROOT / "Data/Animation/Authored/Warlord/Warlord.animevents"
        ).read_text(encoding="utf-8-sig")
        for effect_document in self.manifest["lockedUnboundDefinitions"][0][
            "effectDocuments"
        ]:
            effect_id = Path(effect_document).name.removesuffix(".effect.json")
            self.assertNotIn(effect_id, animevents)


if __name__ == "__main__":
    unittest.main(verbosity=2)

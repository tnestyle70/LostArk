import copy
import hashlib
import json
import math
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
MANIFEST_PATH = ROOT / "Data/Effects/AuthoredCorrections/Artist/Artist.e-r-d-s-t-v-z.composition.json"
RESOURCES_ROOT = ROOT / "Client/Bin/Resources"


def load_json(path: Path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def role_ids(skill):
    return [stable_id for role in skill.get("roles", []) for stable_id in role["stableIds"]]


def normalized_duplicate_payload(element):
    value = copy.deepcopy(element)
    for key in ("id", "displayName", "sourceNode", "visible"):
        value.pop(key, None)
    presentation = value.get("sourcePresentation", {})
    for key in ("sourceEventId", "sourceOccurrenceIndex", "sourceTimeSeconds"):
        presentation.pop(key, None)
    return value


def resource_signature(elements):
    rows = []
    for element in elements:
        for resource in element.get("resources", []):
            if resource.get("assetId"):
                rows.append(f"{element['id']}|R|{resource['slotId']}|{resource['assetId']}")
        source_profile = element.get("material", {}).get("sourceProfile", {})
        for texture in source_profile.get("textures", []):
            if texture.get("assetId"):
                rows.append(f"{element['id']}|T|{texture['name']}|{texture['assetId']}")
    return hashlib.sha256("\n".join(sorted(rows)).encode("utf-8")).hexdigest()


def peak_particle_budget(elements):
    particles = [element for element in elements if element.get("kind") == "particle"]
    start_times = sorted({float(element["detail"]["timing"]["startDelaySeconds"]) for element in particles})
    best = (0, 0, 0, 0.0)
    for time_seconds in start_times:
        active = []
        for element in particles:
            timing = element["detail"]["timing"]
            particle = element["detail"]["particle"]
            start = float(timing["startDelaySeconds"])
            duration = max(float(timing["lifeTimeSeconds"]), max(map(float, particle["lifeTimeSeconds"])))
            if start <= time_seconds < start + duration:
                active.append(element)
        cap = sum(int(element["detail"]["particle"]["maxParticles"]) for element in active)
        burst = sum(
            int(item.get("countMaximum", 0))
            for element in active
            for item in element.get("sourceRecipe", {}).get("bursts", [])
        )
        candidate = (len(active), cap, burst, time_seconds)
        if candidate[:3] > best[:3]:
            best = candidate
    return best


class ArtistRoleCompositionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.manifest = load_json(MANIFEST_PATH)
        cls.skills = {entry["inputSlot"]: entry for entry in cls.manifest["skills"]}

    def test_authority_skill_and_clip_mapping(self):
        self.assertEqual("lostark.effect-role-composition", self.manifest["schema"])
        self.assertEqual(1, self.manifest["formatVersion"])
        player_skills = load_json(ROOT / "Data/Balance/PlayerSkills.json")["skills"]
        authoritative_ids = {
            entry["inputSlot"]: int(entry["skillId"])
            for entry in player_skills
            if entry.get("characterClass") == "ARTIST"
        }
        bindings = load_json(ROOT / "Data/Animation/Authored/Artist/Artist.skillbindings.json")["bindings"]
        authoritative_clips = {int(entry["skillId"]): entry["clips"] for entry in bindings}
        for slot, skill in self.skills.items():
            self.assertEqual(skill["skillId"], authoritative_ids[slot])
            clips = authoritative_clips[skill["skillId"]]
            if "comboStage" in skill:
                clips = clips[int(skill["comboStage"]) - 1]
            self.assertEqual(skill["clips"], clips)
        self.assertEqual(31930, authoritative_ids["ALT_V"])
        self.assertEqual(31050, authoritative_ids["Z"])

    def test_selected_visibility_resources_and_budget(self):
        for slot in ("E", "R", "S", "V"):
            skill = self.skills[slot]
            document = load_json(ROOT / skill["effectDocument"])
            by_id = {element["id"]: element for element in document["elements"]}
            selected_ids = role_ids(skill)
            self.assertEqual(len(selected_ids), len(set(selected_ids)), slot)
            self.assertEqual(set(selected_ids), {element["id"] for element in document["elements"] if element.get("visible")}, slot)
            selected = [by_id[stable_id] for stable_id in selected_ids]
            self.assertTrue(all(element["visible"] for element in selected), slot)
            self.assertEqual(skill["selectedResourceSignatureSha256"], resource_signature(selected), slot)
            for element in selected:
                assets = [row.get("assetId", "") for row in element.get("resources", [])]
                assets += [row.get("assetId", "") for row in element.get("material", {}).get("sourceProfile", {}).get("textures", [])]
                for asset_id in assets:
                    if asset_id.lower().endswith((".dds", ".wmodel")):
                        self.assertTrue((RESOURCES_ROOT / Path(asset_id)).is_file(), f"{slot}: missing {asset_id}")
            budget = skill["budget"]
            self.assertEqual(budget["visibleAfter"], len(selected), slot)
            rows, cap, burst, peak_time = peak_particle_budget(selected)
            self.assertEqual((budget["peakActiveRows"], budget["peakCap"], budget["peakBurst"]), (rows, cap, burst), slot)
            self.assertTrue(math.isclose(float(budget["peakTimeSeconds"]), peak_time, abs_tol=1e-6), slot)

    def test_duplicate_pairs_are_exact_payloads_and_disabled(self):
        for slot in ("R", "S"):
            skill = self.skills[slot]
            document = load_json(ROOT / skill["effectDocument"])
            by_id = {element["id"]: element for element in document["elements"]}
            drops = []
            for keep_id, drop_id in skill["duplicatePairs"]:
                self.assertTrue(by_id[keep_id]["visible"], keep_id)
                self.assertFalse(by_id[drop_id]["visible"], drop_id)
                self.assertEqual(normalized_duplicate_payload(by_id[keep_id]), normalized_duplicate_payload(by_id[drop_id]), drop_id)
                drops.append(drop_id)
            suppressed = []
            for keep_id, drop_id in skill.get("suppressedDuplicatePairs", []):
                self.assertFalse(by_id[keep_id]["visible"], keep_id)
                self.assertFalse(by_id[drop_id]["visible"], drop_id)
                self.assertEqual(normalized_duplicate_payload(by_id[keep_id]), normalized_duplicate_payload(by_id[drop_id]), drop_id)
                suppressed.extend((keep_id, drop_id))
            self.assertEqual(
                skill["budget"]["visibleBefore"],
                skill["budget"]["visibleAfter"] + len(drops) + len(suppressed),
                slot,
            )

    def test_d_t_z_role_manifests_and_s_ribbon_blocker_remain_typed(self):
        converted = ROOT / "Data/Effects/Imported/Artist/CurrentCombat/Converted"
        graphs = ROOT / "Data/Effects/Imported/Artist/CurrentCombat/Graphs"
        imported = ROOT / "Data/Effects/Imported/Artist/CurrentCombat"
        corrections = ROOT / "Data/Effects/AuthoredCorrections/Artist"
        expected_documents = {
            "D": ["Data/Effects/Authored/effect.artist.skill.31490.unified.effect.json"],
            "T": ["Data/Effects/Authored/effect.artist.skill.31950.unified.effect.json"],
            "Z": [
                "Data/Effects/Authored/effect.artist.skill.31050.clip1.unified.effect.json",
                "Data/Effects/Authored/effect.artist.skill.31050.clip2.unified.effect.json",
            ],
        }
        for slot, document_paths in expected_documents.items():
            skill = self.skills[slot]
            skill_id = skill["skillId"]
            self.assertEqual(
                "ROLE_CURATED_PARTIAL_RUNTIME_PROFILE_BLOCKED", skill["status"], slot
            )
            self.assertEqual(
                "EXACT_SELECTED_ROLE_PHYSICAL_CLOSURE", skill["resourceClosure"], slot
            )
            self.assertEqual(document_paths, skill["effectDocuments"], slot)
            self.assertTrue(
                (converted / f"effect.artist.skill.{skill_id}.imported.effect.json").is_file(),
                slot,
            )
            self.assertTrue((graphs / f"skill.{skill_id}.normalized-effect-graph.json").is_file(), slot)
            self.assertTrue((imported / f"skill.{skill_id}.source-receipt.json").is_file(), slot)
            expected_manifest_path = (
                corrections / f"effect.artist.skill.{skill_id}.role-manifest.json"
            )
            self.assertEqual(
                expected_manifest_path.relative_to(ROOT).as_posix(),
                skill["roleManifest"],
                slot,
            )
            role_manifest = load_json(expected_manifest_path)
            self.assertEqual("lostark.effect-role-manifest", role_manifest["schema"], slot)
            self.assertEqual(1, role_manifest["version"], slot)
            for document_path in document_paths:
                document = load_json(ROOT / document_path)
                self.assertEqual(13, document["version"], slot)
                self.assertTrue(document["elements"], slot)
            self.assertGreater(role_manifest["baseline"]["visibleElementCount"], 0, slot)
            self.assertGreater(role_manifest["baseline"]["roleLockedElementCount"], 0, slot)
            self.assertEqual(
                role_manifest["baseline"]["visibleElementCount"],
                skill["visibleElementCount"],
                slot,
            )
            self.assertEqual(
                role_manifest["baseline"]["roleLockedElementCount"],
                skill["roleLockedElementCount"],
                slot,
            )
            self.assertIn(
                "SOURCE_MATERIAL_RUNTIME_PROFILE_NOT_EXECUTABLE",
                skill["remainingBlockers"],
                slot,
            )
        skill = self.skills["S"]
        document = load_json(ROOT / skill["effectDocument"])
        by_id = {element["id"]: element for element in document["elements"]}
        ribbon = next(role for role in skill["blockedRoles"] if role["typedReason"] == "SOURCE_RIBBON_RUNTIME_UNAVAILABLE")
        for stable_id in ribbon["stableIds"]:
            element = by_id[stable_id]
            self.assertFalse(element["visible"], stable_id)
            self.assertTrue(element["material"]["execution"]["failClosed"], stable_id)
            class_names = {module["className"].lower() for module in element["sourceRecipe"]["modules"]}
            self.assertIn("particlemoduletypedataribbon", class_names, stable_id)

    def test_s_orange_carrier_is_not_mislabeled_as_the_black_ending(self):
        skill = self.skills["S"]
        document = load_json(ROOT / skill["effectDocument"])
        by_id = {element["id"]: element for element in document["elements"]}
        self.assertEqual(
            [{
                "role": "SOURCE_ORANGE_PARTICLEMASTER_CARRIER_REVIEW",
                "stableIds": ["authored.source-particle.25b54f02bd198502d3b9f851"],
            }],
            skill["roles"],
        )
        orange = by_id["authored.source-particle.25b54f02bd198502d3b9f851"]
        source_color = next(
            vector["value"]
            for vector in orange["material"]["sourceProfile"]["vectors"]
            if vector["name"] == "62.color"
        )
        self.assertEqual([3.0, 1.0, 0.1, 1.0], source_color)
        black = next(
            role for role in skill["blockedRoles"]
            if role["role"] == "BLACK_SPRITE_ENDING"
        )
        self.assertEqual([0.0, 0.0, 0.0, 1.0], black["sourceColor"])
        self.assertEqual(
            {
                "authored.source-particle.1e1c0028bba7444168a1d1b1",
                "authored.source-particle.02347652cc6d0a0071a842f0",
            },
            set(black["stableIds"]),
        )
        for stable_id in black["stableIds"]:
            self.assertFalse(by_id[stable_id]["visible"], stable_id)
            self.assertTrue(
                by_id[stable_id]["material"]["execution"]["failClosed"],
                stable_id,
            )


if __name__ == "__main__":
    unittest.main(verbosity=2)

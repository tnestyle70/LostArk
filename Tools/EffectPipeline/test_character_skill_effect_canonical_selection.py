import json
import re
import unittest
from pathlib import Path


class CharacterSkillEffectCanonicalSelectionTests(unittest.TestCase):
    ARTIST_EFFECT = "effect.artist.skill.31460.linear-reveal.unified"
    WARLORD_EFFECT = "effect.warlord.skill.17080.clip2.unified"

    @classmethod
    def setUpClass(cls) -> None:
        cls.repository_root = Path(__file__).resolve().parents[2]
        cls.player_skills = cls.load_json("Data/Balance/PlayerSkills.json")[
            "skills"
        ]
        cls.catalog_rows = {
            row["effectAssetId"]: row
            for row in cls.load_json("Data/Effects/EffectCatalog.json")["effects"]
        }

    @classmethod
    def load_json(cls, relative_path: str) -> dict:
        return json.loads(
            (cls.repository_root / relative_path).read_text(encoding="utf-8")
        )

    @classmethod
    def load_binding(cls, asset_name: str, skill_id: int) -> dict:
        document = cls.load_json(
            f"Data/Animation/Authored/{asset_name}/{asset_name}.skillbindings.json"
        )
        return next(
            row for row in document["bindings"] if row["skillId"] == skill_id
        )

    @classmethod
    def collect_product_cues(
        cls, asset_name: str, clip_names: set[str]
    ) -> list[tuple[str, int, str]]:
        event_path = (
            cls.repository_root
            / f"Data/Animation/Authored/{asset_name}/{asset_name}.animevents"
        )
        cue_pattern = re.compile(
            r'^"([^"]+)" EFFECT startms=(\d+) payload="([^"]+)" '
            r'effectref=asset\b'
        )
        cues: list[tuple[str, int, str]] = []
        for line in event_path.read_text(encoding="utf-8").splitlines():
            match = cue_pattern.match(line)
            if match is None or match.group(1) not in clip_names:
                continue
            cues.append((match.group(1), int(match.group(2)), match.group(3)))
        return cues

    def test_player_skill_owners_and_stage_shapes_remain_authoritative(self) -> None:
        artist = next(row for row in self.player_skills if row["skillId"] == 31460)
        warlord = next(row for row in self.player_skills if row["skillId"] == 17080)

        self.assertEqual(
            (artist["characterClass"], artist["inputSlot"], artist["skillKind"]),
            ("ARTIST", "A", "ACTIVE"),
        )
        self.assertEqual(
            (warlord["characterClass"], warlord["inputSlot"], warlord["skillKind"]),
            ("WARLORD", "E", "COMBO"),
        )
        self.assertEqual(artist["effectId"], self.ARTIST_EFFECT)
        self.assertEqual(warlord["effectId"], self.WARLORD_EFFECT)
        self.assertEqual(len(warlord["comboStages"]), 2)

    def test_skillbindings_keep_the_selected_animation_occurrences(self) -> None:
        artist = self.load_binding("Artist", 31460)
        warlord = self.load_binding("Warlord", 17080)

        self.assertEqual(artist["clips"], ["sdm_sk_butterflydream"])
        self.assertEqual(
            warlord["clips"],
            [
                ["wgl_sk_dashupperfire_01"],
                ["wgl_sk_dashupperfire_02"],
            ],
        )

    def test_artist_a_uses_only_the_selected_linear_reveal_product(self) -> None:
        self.assertEqual(
            self.collect_product_cues("Artist", {"sdm_sk_butterflydream"}),
            [("sdm_sk_butterflydream", 0, self.ARTIST_EFFECT)],
        )

    def test_warlord_e_both_combo_occurrences_reuse_clip2_product(self) -> None:
        self.assertEqual(
            self.collect_product_cues(
                "Warlord",
                {"wgl_sk_dashupperfire_01", "wgl_sk_dashupperfire_02"},
            ),
            [
                ("wgl_sk_dashupperfire_01", 0, self.WARLORD_EFFECT),
                ("wgl_sk_dashupperfire_02", 0, self.WARLORD_EFFECT),
            ],
        )

    def test_selected_effects_are_direct_authored_catalog_definitions(self) -> None:
        for effect_asset_id in (self.ARTIST_EFFECT, self.WARLORD_EFFECT):
            row = self.catalog_rows[effect_asset_id]
            self.assertEqual(row["payloadKind"], "DIRECT_AUTHORED_DOCUMENT")
            expected_authoring_path = (
                f"Effects/Authored/{effect_asset_id}.effect.json"
            )
            self.assertEqual(row["authoringPath"], expected_authoring_path)

            document = self.load_json(f"Data/{expected_authoring_path}")
            self.assertEqual(document["effectAssetId"], effect_asset_id)
            self.assertTrue(any(element["visible"] for element in document["elements"]))


if __name__ == "__main__":
    unittest.main()

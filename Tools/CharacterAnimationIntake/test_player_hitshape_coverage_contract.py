import json
import pathlib
import re
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
CLASSES = (
    "LANCE_MASTER",
    "GUNSLINGER",
    "SLAYER",
    "ARTIST",
    "DIMENSIONMASTER",
    "WARLORD",
)
IMPORTED_RUNTIME_FALLBACKS = {
    38120,
    38180,
    38260,
    38290,
    45000,
    45820,
    17240,
    2050010,
}


class PlayerHitShapeCoverageContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.skills = json.loads(
            (ROOT / "Data/Balance/PlayerSkills.json").read_text(encoding="utf-8")
        )["skills"]
        cls.documents = [
            json.loads(path.read_text(encoding="utf-8"))
            for path in sorted((ROOT / "Data/Animation/HitShapes").glob("*.hitshapes.json"))
        ]

    def test_every_class_has_nonzero_authored_coverage_and_report_is_exact(self) -> None:
        covered = {
            int(skill["skillId"])
            for document in self.documents
            for skill in document["skills"]
        }
        missing = set()
        totals = []
        for character_class in CLASSES:
            damage_ids = {
                int(skill["skillId"])
                for skill in self.skills
                if skill["characterClass"] == character_class
                and skill["serverDamageProfileId"]
            }
            authored = damage_ids & covered
            self.assertTrue(authored, character_class)
            missing.update(damage_ids - authored)
            totals.append((character_class, len(authored), len(damage_ids)))
        self.assertEqual(set(), missing)
        self.assertEqual(76, sum(row[1] for row in totals))
        self.assertEqual(76, sum(row[2] for row in totals))

    def test_document_class_owns_every_covered_skill(self) -> None:
        owners = {int(skill["skillId"]): skill["characterClass"] for skill in self.skills}
        seen_classes = set()
        for document in self.documents:
            self.assertEqual("lostark.animation-hit-shapes", document["schema"])
            self.assertEqual(4, document["formatVersion"])
            self.assertNotIn(document["characterClass"], seen_classes)
            seen_classes.add(document["characterClass"])
            for skill in document["skills"]:
                self.assertEqual(
                    document["characterClass"], owners[int(skill["skillId"])]
                )
        self.assertEqual(set(CLASSES), seen_classes)

    def test_explicit_result_graph_and_existing_runtime_fallback_basis(self) -> None:
        identities = set()
        imported = set()
        kinds = set()
        for document in self.documents:
            for skill in document["skills"]:
                if skill.get("sourceBasis") == "EXISTING_SERVER_MAXIMUM_RANGE":
                    imported.add(skill["skillId"])
                for stage in skill.get("stages", [skill]):
                    for owner in [stage] + stage.get("projectiles", []):
                        for hit in owner["hits"]:
                            logic, result = hit["logic"], hit["result"]
                            self.assertEqual("DURATION", logic["logicType"])
                            self.assertEqual("AREA_OVERLAP", logic["judgementKind"])
                            self.assertEqual("RESULT", result["logicType"])
                            self.assertEqual(hit["colliderId"], logic["colliderId"])
                            self.assertEqual(result["resultId"], logic["resultId"])
                            self.assertIn(result["resultKind"], {"DAMAGE", "STAGGER", "COUNTER"})
                            kinds.add(result["resultKind"])
                            for identity in (hit["colliderId"], logic["logicId"], result["resultId"]):
                                self.assertNotIn(identity, identities)
                                identities.add(identity)
        self.assertEqual(IMPORTED_RUNTIME_FALLBACKS, imported)
        self.assertEqual({"DAMAGE", "STAGGER", "COUNTER"}, kinds)

    def test_publisher_reports_partial_coverage_and_rejects_zero_class(self) -> None:
        source = (ROOT / "Tools/GameplayPipeline/Publish-GameplayBalance.ps1").read_text(
            encoding="utf-8-sig"
        )
        self.assertIn("Player hit-shape authored coverage:", source)
        self.assertIn("Player hit-shape coverage is partial:", source)
        self.assertIn("Player hit-shape coverage is zero for class:", source)
        self.assertIn("maximumRange fallback skillIds=", source)

    def test_client_fallback_wire_is_gray_and_presentation_only(self) -> None:
        source = (ROOT / "Client/Private/Character.cpp").read_text(
            encoding="utf-8-sig"
        )
        self.assertIn("Try_Get_CurrentFallbackHitRange", source)
        self.assertRegex(
            source,
            re.compile(
                r"FALLBACK_RANGE_COLOR_RGBA\s*=\s*\n?\s*170u\s*\|"
                r"\s*\(170u\s*<<\s*8\)\s*\|\s*\(170u\s*<<\s*16\)"
            ),
        )
        self.assertIn("fFallbackRangeMeters * 100.f", source)
        self.assertIn("m_EffectCueDocument.Projectiles", source)
        self.assertIn("m_EffectCueDocument.Hits", source)
        self.assertIn("never participates in Client collision", source)


if __name__ == "__main__":
    unittest.main()

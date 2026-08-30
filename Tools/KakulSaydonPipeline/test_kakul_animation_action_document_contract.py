import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
REFERENCE_ROOT = ROOT / "Data" / "Animation" / "Reference" / "KakulSaydon"
AUTHORED_ROOT = ROOT / "Data" / "Animation" / "Authored" / "KakulSaydon"
HEADER = ROOT / "Client" / "Public" / "KakulAnimationActionDocument.h"
SOURCE = ROOT / "Client" / "Private" / "KakulAnimationActionDocument.cpp"

PROFILES = ("MN_RPCT_05", "MN_RPCT_06", "MN_RPCT_07", "MN_RPCZ_00")
REFERENCE_ROOT_FIELDS = {
    "schema",
    "formatVersion",
    "profileId",
    "modelAssetId",
    "sourceEvidenceSha256",
    "referenceRevision",
    "authority",
    "actions",
}
ACTION_FIELDS = {
    "sourceActionId",
    "displayName",
    "reviewStatus",
    "authority",
    "stages",
}
STAGE_FIELDS = {"stageId", "stageOrdinal", "holdoutClipNames", "slots"}
SLOT_FIELDS = {
    "slotId",
    "extractedClip",
    "runtimeClip",
    "sourceStartMs",
    "playMs",
    "playRate",
    "loop",
    "mappingBasis",
    "authority",
}
AUTHORED_ROOT_FIELDS = {
    "schema",
    "formatVersion",
    "authority",
    "profileId",
    "referenceRevision",
    "bindings",
}
BINDING_FIELDS = {
    "sourceActionId",
    "stageId",
    "slotId",
    "runtimeClip",
    "sourceStartMs",
    "playMs",
    "playRate",
    "loop",
    "mappingBasis",
    "authority",
}


class KakulAnimationActionDocumentContractTests(unittest.TestCase):
    def test_generated_reference_and_sparse_authored_fixtures_join_exactly(self):
        for profile in PROFILES:
            reference_path = REFERENCE_ROOT / f"{profile}.actionreference.json"
            authored_path = AUTHORED_ROOT / f"{profile}.actionbindings.json"
            self.assertTrue(reference_path.is_file(), reference_path)
            self.assertTrue(authored_path.is_file(), authored_path)
            reference = json.loads(reference_path.read_text(encoding="utf-8"))
            authored = json.loads(authored_path.read_text(encoding="utf-8"))

            self.assertEqual(set(reference), REFERENCE_ROOT_FIELDS)
            self.assertEqual(reference["schema"], "lostark.kakul-animation-action-reference")
            self.assertEqual(reference["formatVersion"], 1)
            self.assertEqual(reference["profileId"], profile)
            self.assertEqual(reference["authority"], "REFERENCE_ONLY")
            self.assertRegex(reference["sourceEvidenceSha256"], r"^[0-9a-f]{64}$")
            self.assertRegex(reference["referenceRevision"], r"^[0-9a-f]{64}$")
            self.assertTrue(reference["actions"])

            action_ids = set()
            slot_keys = set()
            for action in reference["actions"]:
                self.assertEqual(set(action), ACTION_FIELDS)
                self.assertNotIn(action["sourceActionId"], action_ids)
                action_ids.add(action["sourceActionId"])
                self.assertTrue(action["displayName"])
                self.assertIn(action["reviewStatus"], {"REVIEW_CANDIDATE", "HOLDOUT"})
                self.assertEqual(action["authority"], "REFERENCE_ONLY")
                self.assertTrue(action["stages"])
                has_holdout = False
                stage_ids = set()
                for ordinal, stage in enumerate(action["stages"]):
                    self.assertEqual(set(stage), STAGE_FIELDS)
                    self.assertEqual(stage["stageOrdinal"], ordinal)
                    self.assertNotIn(stage["stageId"], stage_ids)
                    stage_ids.add(stage["stageId"])
                    has_holdout |= bool(stage["holdoutClipNames"])
                    self.assertEqual(
                        len(stage["holdoutClipNames"]),
                        len(set(stage["holdoutClipNames"])),
                    )
                    slot_ids = set()
                    for slot in stage["slots"]:
                        self.assertEqual(set(slot), SLOT_FIELDS)
                        self.assertNotIn(slot["slotId"], slot_ids)
                        slot_ids.add(slot["slotId"])
                        key = (action["sourceActionId"], stage["stageId"], slot["slotId"])
                        self.assertNotIn(key, slot_keys)
                        slot_keys.add(key)
                        self.assertEqual(slot["mappingBasis"], "EXTRACTED_REFERENCE")
                        self.assertEqual(slot["authority"], "REFERENCE_ONLY")
                        self.assertGreater(slot["playMs"], 0)
                        self.assertGreaterEqual(slot["playRate"], 0.01)
                        self.assertLessEqual(slot["playRate"], 16.0)
                self.assertEqual(action["reviewStatus"] == "HOLDOUT", has_holdout)

            self.assertEqual(set(authored), AUTHORED_ROOT_FIELDS)
            self.assertEqual(authored["schema"], "lostark.kakul-animation-action-bindings")
            self.assertEqual(authored["formatVersion"], 1)
            self.assertEqual(authored["authority"], "REFERENCE_ONLY")
            self.assertEqual(authored["profileId"], profile)
            self.assertEqual(authored["referenceRevision"], reference["referenceRevision"])
            claimed = set()
            for binding in authored["bindings"]:
                self.assertEqual(set(binding), BINDING_FIELDS)
                key = (
                    binding["sourceActionId"],
                    binding["stageId"],
                    binding["slotId"],
                )
                self.assertIn(key, slot_keys)
                self.assertNotIn(key, claimed)
                claimed.add(key)
                self.assertEqual(binding["mappingBasis"], "PROJECT_AUTHORED")
                self.assertEqual(binding["authority"], "REFERENCE_ONLY")
                self.assertGreater(binding["playMs"], 0)

    def test_public_boundary_exposes_transactional_load_and_sparse_save(self):
        header = HEADER.read_text(encoding="utf-8")
        for token in (
            "KAKUL_ANIMATION_ACTION_REFERENCE_DOCUMENT",
            "KAKUL_ANIMATION_ACTION_AUTHORED_DOCUMENT",
            "Resolve_ReferencePath",
            "Resolve_AuthoredPath",
            "Parse_ReferenceText",
            "Parse_AuthoredText",
            "Validate_Reference",
            "Validate_Authored",
            "Load_FromPaths",
            "Save_Atomic",
        ):
            self.assertIn(token, header)
        self.assertIn("std::vector<KAKUL_ANIMATION_ACTION_BINDING> Bindings", header)

    def test_cpp_is_strict_reference_only_and_exact_slot_joined(self):
        source = SOURCE.read_text(encoding="utf-8")
        for token in (
            '"lostark.kakul-animation-action-reference"',
            '"lostark.kakul-animation-action-bindings"',
            '"REFERENCE_ONLY"',
            '"EXTRACTED_REFERENCE"',
            '"PROJECT_AUTHORED"',
            "Has_ExactProperties",
            "Build_SlotKey",
            "referenceSlots.contains(key)",
            "claimedSlots.insert(key).second",
            "availableClipSet.contains(binding.strRuntimeClip)",
            "document.strReferenceRevision != reference.strReferenceRevision",
        ):
            self.assertIn(token, source)
        self.assertNotIn(
            "binding.strRuntimeClip != slot.strRuntimeClip",
            source,
            "A local override may select another exact clip exposed by the model.",
        )

    def test_save_is_durable_reparsed_and_atomic(self):
        source = SOURCE.read_text(encoding="utf-8")
        for token in (
            "_commit(_fileno(file))",
            "Parse_AuthoredText(verificationText, reparsed",
            "Validate_Authored(reparsed, currentReference",
            "reparsed != document",
            "finalReference != currentReference",
            "MoveFileExW",
            "MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH",
        ):
            self.assertIn(token, source)
        self.assertLess(source.index("_commit(_fileno(file))"), source.index("reparsed != document"))
        self.assertLess(source.index("reparsed != document"), source.rindex("MoveFileExW"))


if __name__ == "__main__":
    unittest.main()

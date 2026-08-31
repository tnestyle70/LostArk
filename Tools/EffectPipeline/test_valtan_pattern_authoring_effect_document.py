from __future__ import annotations

import copy
import json
import re
import unittest
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
DOCUMENT_PATH = (
    REPOSITORY_ROOT / "Data/Effects/ValtanPatternAuthoringEffects.json"
)
CODEC_CPP = (
    REPOSITORY_ROOT
    / "Client/Private/ValtanPatternAuthoringEffectDocument.cpp"
)

ROOT_FIELDS = {"schema", "formatVersion", "bossArchetypeId", "bindings"}
BINDING_FIELDS = {"patternId", "effectAssetId", "authoringPath", "state"}
STABLE_TOKEN = re.compile(r"^[A-Za-z0-9_.-]{1,255}$")


def binding(
    pattern_id: str = "VALTAN_TEST",
    effect_id: str = "effect.valtan.test",
) -> dict[str, object]:
    return {
        "patternId": pattern_id,
        "effectAssetId": effect_id,
        "authoringPath": f"Effects/Authored/{effect_id}.effect.json",
        "state": "DRAFT_ATTACHED",
    }


def validates(document: object) -> bool:
    if not isinstance(document, dict) or set(document) != ROOT_FIELDS:
        return False
    if document["schema"] != "lostark.valtan-pattern-authoring-effects":
        return False
    if document["formatVersion"] != 1:
        return False
    if document["bossArchetypeId"] != "BOSS_VALTAN":
        return False
    bindings = document["bindings"]
    if not isinstance(bindings, list) or len(bindings) > 256:
        return False
    pattern_ids: set[str] = set()
    effect_ids: set[str] = set()
    paths: set[str] = set()
    for row in bindings:
        if not isinstance(row, dict) or set(row) != BINDING_FIELDS:
            return False
        pattern_id = row["patternId"]
        effect_id = row["effectAssetId"]
        path = row["authoringPath"]
        if not isinstance(pattern_id, str) or STABLE_TOKEN.fullmatch(pattern_id) is None:
            return False
        if not isinstance(effect_id, str) or STABLE_TOKEN.fullmatch(effect_id) is None:
            return False
        if path != f"Effects/Authored/{effect_id}.effect.json":
            return False
        if row["state"] != "DRAFT_ATTACHED":
            return False
        if pattern_id in pattern_ids or effect_id in effect_ids or path in paths:
            return False
        pattern_ids.add(pattern_id)
        effect_ids.add(effect_id)
        paths.add(path)
    return True


class ValtanPatternAuthoringEffectDocumentTests(unittest.TestCase):
    def setUp(self) -> None:
        self.seed = json.loads(DOCUMENT_PATH.read_text(encoding="utf-8"))

    def test_repository_seed_is_a_valid_cross_draft_contract(self) -> None:
        self.assertTrue(validates(self.seed))
        self.assertEqual(
            self.seed["bindings"],
            [binding("VALTAN_CROSS", "effect.valtan.sequence.cross")],
        )

    def test_one_binding_has_no_animation_or_runtime_fields(self) -> None:
        document = copy.deepcopy(self.seed)
        document["bindings"] = [binding()]
        self.assertTrue(validates(document))
        self.assertTrue(
            {
                "stageId",
                "clipName",
                "durationMs",
                "animation",
                "runtimeCueId",
                "anchorSlotId",
            }.isdisjoint(document["bindings"][0])
        )

    def test_unknown_root_and_binding_fields_are_rejected(self) -> None:
        root = copy.deepcopy(self.seed)
        root["runtime"] = True
        self.assertFalse(validates(root))

        document = copy.deepcopy(self.seed)
        row = binding()
        row["stageId"] = "ACTIVE"
        document["bindings"] = [row]
        self.assertFalse(validates(document))

    def test_identity_path_state_and_duplicate_mutations_are_rejected(self) -> None:
        mutations: list[list[dict[str, object]]] = []
        invalid_pattern = binding(pattern_id="../VALTAN_TEST")
        mutations.append([invalid_pattern])
        invalid_effect = binding(effect_id="effect/valtan/test")
        mutations.append([invalid_effect])
        invalid_path = binding()
        invalid_path["authoringPath"] = "Effects/Authored/other.effect.json"
        mutations.append([invalid_path])
        invalid_state = binding()
        invalid_state["state"] = "RUNTIME_BOUND"
        mutations.append([invalid_state])
        mutations.append([binding(), binding(effect_id="effect.valtan.other")])
        mutations.append(
            [binding(), binding(pattern_id="VALTAN_OTHER")]
        )
        for rows in mutations:
            with self.subTest(rows=rows):
                document = copy.deepcopy(self.seed)
                document["bindings"] = rows
                self.assertFalse(validates(document))

    def test_cpp_codec_has_strict_parse_cas_and_durable_replace(self) -> None:
        source = CODEC_CPP.read_text(encoding="utf-8")
        for token in (
            '"schema", "formatVersion", "bossArchetypeId", "bindings"',
            '"patternId", "effectAssetId", "authoringPath", "state"',
            "Has_ExactProperties",
            "MAX_BINDINGS",
            "PatternIds.insert",
            "EffectAssetIds.insert",
            "AuthoringPaths.insert",
            "strExpectedCanonicalBaseline",
            "CurrentCanonical != strExpectedCanonicalBaseline",
            "CValtanPatternAuthoringEffectTransaction Transaction",
            "TRANSACTION_MUTEX_NAME",
            "WaitForSingleObject",
            "BeforeReplaceCanonical",
            "fflush",
            "_commit",
            "MoveFileExW",
            "MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH",
        ):
            self.assertIn(token, source)


if __name__ == "__main__":
    unittest.main(verbosity=2)

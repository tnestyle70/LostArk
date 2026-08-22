from __future__ import annotations

import re
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]


def read_source(relative: str) -> str:
    payload = (REPO_ROOT / relative).read_bytes()
    try:
        source = payload.decode("utf-8")
    except UnicodeDecodeError:
        source = payload.decode("cp949")
    return source.replace("\r\n", "\n")


def braced_block(source: str, marker: str) -> str:
    marker_position = source.find(marker)
    if marker_position < 0:
        raise AssertionError(f"source marker is missing: {marker}")
    opening = source.find("{", marker_position)
    if opening < 0:
        raise AssertionError(f"source marker has no body: {marker}")
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[marker_position : index + 1]
    raise AssertionError(f"source marker has an unterminated body: {marker}")


class GroundTargetPreviewPrototypeScopeTests(unittest.TestCase):
    def test_static_registration_and_target_level_layer_use_distinct_scopes(self) -> None:
        static_registry = braced_block(
            read_source("Client/Private/MainApp.cpp"),
            "HRESULT CMainApp::Ready_Prototype_For_Static()",
        )
        self.assertRegex(
            static_registry,
            r"Add_Prototype\(\s*ETOUI\(LEVEL::STATIC\),\s*"
            r"CSkillGroundTargetPreview::PROTOTYPE_TAG,",
        )

        initialize = braced_block(
            read_source("Client/Private/PlayerController.cpp"),
            "bool_t Client::CPlayerController::Initialize_TargetingPreview(",
        )
        self.assertRegex(
            initialize,
            r"Add_GameObject_to_Layer\(\s*"
            r"ETOUI\(LEVEL::STATIC\),\s*"
            r"CSkillGroundTargetPreview::PROTOTYPE_TAG,\s*"
            r"levelIndex,\s*"
            r'TEXT\("Layer_SkillGroundTargetPreview"\)',
        )

    def test_every_product_level_initializes_the_shared_preview(self) -> None:
        consumers = {
            "Client/Private/Level_CharacterSelect.cpp": "CHARACTER_SELECT",
            "Client/Private/Level_Bern.cpp": "BERN",
            "Client/Private/Level_Development.cpp": "DEVELOPMENT",
            "Client/Private/Level_ValtanArena.cpp": "VALTAN_ARENA",
        }
        for relative, level_name in consumers.items():
            with self.subTest(level=level_name):
                source = read_source(relative)
                self.assertRegex(
                    source,
                    r"Initialize_TargetingPreview\(\s*"
                    + re.escape(f"ETOUI(LEVEL::{level_name})")
                    + r"\s*\)",
                )


if __name__ == "__main__":
    unittest.main()

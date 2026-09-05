#!/usr/bin/env python3
"""Locks the P0-2 authored-tool spelling and protected runtime identities."""

from __future__ import annotations

import ast
from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[2]
PIPELINE = ROOT / "Tools" / "KoukuSaydonPipeline"
LEGACY_PIPELINE = ROOT / "Tools" / "KakulSaydonPipeline"


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8-sig")


def read_source(path: Path) -> str:
    payload = path.read_bytes()
    for encoding in ("utf-8-sig", "cp949"):
        try:
            return payload.decode(encoding)
        except UnicodeDecodeError:
            continue
    raise AssertionError(f"unsupported source encoding: {path}")


def source_files(root: Path, suffixes: set[str]) -> list[Path]:
    if not root.is_dir():
        return []
    return sorted(
        path
        for path in root.rglob("*")
        if path.is_file()
        and path.suffix.casefold() in suffixes
        and "__pycache__" not in path.parts
    )


class KoukuSaydonNamingBoundaryTests(unittest.TestCase):
    def test_pipeline_directory_and_modules_use_canonical_authored_spelling(self) -> None:
        self.assertFalse(LEGACY_PIPELINE.exists(), LEGACY_PIPELINE)
        self.assertTrue(PIPELINE.is_dir(), PIPELINE)

        expected_modules = {
            "build_arena_hidden_ids.py",
            "build_kouku_saydon_animation_reference.py",
            "build_kouku_saydon_world_authoring.py",
            "test_build_kouku_saydon_animation_reference.py",
            "test_build_kouku_saydon_world_authoring.py",
            "test_kouku_saydon_animation_action_document_contract.py",
            "test_kouku_saydon_animation_pattern_document_contract.py",
            "test_kouku_saydon_client_product_level_contract.py",
            "test_kouku_saydon_naming_boundary.py",
            "test_kouku_saydon_world_admission.py",
            "validate_kouku_saydon_world_admission.py",
        }
        actual_modules = {path.name for path in PIPELINE.glob("*.py")}
        self.assertTrue(
            expected_modules.issubset(actual_modules),
            sorted(expected_modules - actual_modules),
        )
        self.assertFalse(any("kakul" in name.casefold() for name in actual_modules))

        build_domains = read("Tools/Build/BuildDomains.json")
        runner = read("Tools/Build/Invoke-BuildAndRegression.ps1")
        domain_owner = read("Tools/Build/Invoke-BuildDomainOwner.ps1")
        self.assertIn(
            "Tools/KoukuSaydonPipeline/build_kouku_saydon_animation_reference.py",
            build_domains,
        )
        # These checks remain available by explicit invocation. Canonical names
        # do not make their source oracles prerequisites of a product build.
        for explicit_check in (
            "Tools/KoukuSaydonPipeline/test_kouku_saydon_naming_boundary.py",
            "Tools/LevelPlacementExtractor/test_placement_transform.py",
        ):
            self.assertTrue((ROOT / explicit_check).is_file(), explicit_check)
            self.assertNotIn(explicit_check, runner)
        self.assertIn(
            "Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py",
            build_domains,
        )
        self.assertIn('"id": "map.kakulsaydon"', build_domains)
        self.assertIn("'map.kakulsaydon'", domain_owner)
        build_surface = build_domains + runner + domain_owner
        self.assertNotIn("Tools/KakulSaydonPipeline", build_surface)
        self.assertNotIn("'map.kakul'", build_surface)
        self.assertNotIn('"map.kakul"', build_surface)

    def test_animation_authoring_names_move_without_changing_resource_alias(self) -> None:
        animation_builder = read(
            "Tools/KoukuSaydonPipeline/build_kouku_saydon_animation_reference.py"
        )
        for token in (
            "lostark.kouku-saydon-animation-action-reference",
            "lostark.kouku-saydon-animation-action-bindings",
            "lostark.kouku-saydon-animation-pattern-bindings",
            '"Reference/KoukuSaydon"',
            '"Authored/KoukuSaydon"',
            r"kakulsaydon\.",
        ):
            self.assertIn(token, animation_builder)
        for legacy in (
            "lostark.kakul-animation-",
            "Reference/KakulSaydon",
            "Authored/KakulSaydon",
        ):
            self.assertNotIn(legacy, animation_builder)

        self.assertIn(
            'REPO_ROOT / "Client/Bin/Resources/Character/KoukuSaton"',
            animation_builder,
        )

        retired_paths = (
            "Data/Animation/Reference/KakulSaydon",
            "Data/Animation/Authored/KakulSaydon",
            "Data/Compositions/Bosses/KakulSaydon.bosscomposition.json",
            "Data/Compositions/Sequences/KakulSaydonArena.sequencer.json",
            "Client/Public/KakulAnimationActionDocument.h",
            "Client/Private/KakulAnimationActionDocument.cpp",
            "Client/Public/KakulAnimationPatternDocument.h",
            "Client/Private/KakulAnimationPatternDocument.cpp",
        )
        for relative in retired_paths:
            self.assertFalse((ROOT / relative).exists(), relative)

        legacy_client_tokens = (
            "KakulAnimation",
            "KAKUL_ANIMATION",
            "Open_KakulAction",
            "Open_KakulProfile",
            "Render_KakulAction",
            "Render_KakulPattern",
        )
        client_violations: list[str] = []
        for directory in (ROOT / "Client/Public", ROOT / "Client/Private"):
            for path in source_files(directory, {".h", ".cpp"}):
                text = read_source(path)
                for line_number, line in enumerate(text.splitlines(), start=1):
                    for token in legacy_client_tokens:
                        if token in line:
                            client_violations.append(
                                f"{path.relative_to(ROOT)}:{line_number}:{token}"
                            )
                    for match in re.finditer("KakulSaydon", line):
                        if line[match.end() :].startswith("Arena"):
                            continue
                        client_violations.append(
                            f"{path.relative_to(ROOT)}:{line_number}:KakulSaydon"
                        )
        self.assertEqual(client_violations, [])

    def test_kouku_saton_is_only_a_physical_resource_alias_in_client_source(self) -> None:
        resource_alias_path = re.compile(
            r"(?:Resources[\\/])?"
            r"(?:Character|Effect|Map|Sound|UI)[\\/]KoukuSaton(?:[\\/]|\")"
        )
        violations: list[str] = []
        for directory in (ROOT / "Client/Public", ROOT / "Client/Private"):
            for path in source_files(directory, {".h", ".cpp"}):
                for line_number, line in enumerate(
                    read_source(path).splitlines(), start=1
                ):
                    if "KoukuSaton" not in line or resource_alias_path.search(line):
                        continue
                    violations.append(
                        f"{path.relative_to(ROOT)}:{line_number}:{line.strip()}"
                    )

        naming_test = Path(__file__).resolve()
        validator = PIPELINE / "validate_kouku_saydon_world_admission.py"
        for path in source_files(PIPELINE, {".py"}):
            if path == naming_test:
                continue
            for line_number, line in enumerate(
                read_source(path).splitlines(), start=1
            ):
                if "KoukuSaton" not in line or resource_alias_path.search(line):
                    continue
                is_alias_contract = path == validator and (
                    "protected physical resource collection alias" in line
                    or 'COLLECTION_NAME = "KoukuSaton"' in line
                )
                if not is_alias_contract:
                    violations.append(
                        f"{path.relative_to(ROOT)}:{line_number}:{line.strip()}"
                    )

        runner = read_source(ROOT / "Tools/Build/Invoke-BuildAndRegression.ps1")
        if "KoukuSaton" in runner:
            violations.append("Tools/Build/Invoke-BuildAndRegression.ps1:human-label")
        self.assertEqual(violations, [])

    def test_valtan_owned_sources_do_not_depend_on_kouku_authoring(self) -> None:
        valtan_owned: list[Path] = []
        for directory in (ROOT / "Client/Public", ROOT / "Client/Private"):
            valtan_owned.extend(
                path
                for path in source_files(directory, {".h", ".cpp"})
                if "valtan" in path.name.casefold()
            )
        for directory in (ROOT / "Server/Public", ROOT / "Server/Private"):
            valtan_owned.extend(
                path
                for path in source_files(directory, {".h", ".cpp"})
                if "valtan" in path.name.casefold()
            )
        for directory in (
            ROOT / "Data/Valtan",
            ROOT / "Data/Animation/Authored/Valtan",
            ROOT / "Data/Animation/Reference/Valtan",
        ):
            valtan_owned.extend(source_files(directory, {".json"}))
        valtan_owned.extend(
            path
            for path in source_files(
                ROOT / "Tools/ValtanPipeline",
                {".py", ".ps1", ".psm1"},
            )
            if not path.name.startswith("test_")
        )
        valtan_owned.extend(
            path
            for path in source_files(ROOT / "Tools", {".py", ".ps1", ".psm1"})
            if "valtan" in path.name.casefold()
            and not path.name.startswith("test_")
        )

        dependency = re.compile(r"kouku|kakul|saydon", re.IGNORECASE)
        violations: list[str] = []
        for path in sorted(set(valtan_owned)):
            for line_number, line in enumerate(
                read_source(path).splitlines(), start=1
            ):
                if dependency.search(line):
                    violations.append(
                        f"{path.relative_to(ROOT)}:{line_number}:{line.strip()}"
                    )
        self.assertEqual(violations, [])

    def test_kouku_pipeline_imports_no_valtan_domain_module(self) -> None:
        violations: list[str] = []
        for path in source_files(PIPELINE, {".py"}):
            tree = ast.parse(read_source(path), filename=str(path))
            for node in ast.walk(tree):
                imported: list[str] = []
                if isinstance(node, ast.Import):
                    imported = [alias.name for alias in node.names]
                elif isinstance(node, ast.ImportFrom):
                    imported = [node.module or ""] + [
                        alias.name for alias in node.names
                    ]
                for module in imported:
                    if "valtan" in module.casefold():
                        violations.append(
                            f"{path.relative_to(ROOT)}:{node.lineno}:{module}"
                        )
        self.assertEqual(violations, [])

        world_builder = read(
            "Tools/KoukuSaydonPipeline/build_kouku_saydon_world_authoring.py"
        )
        neutral_transform = read(
            "Tools/LevelPlacementExtractor/placement_transform.py"
        )
        self.assertIn("from placement_transform import", world_builder)
        self.assertNotIn("from build_valtan_navgrid import", world_builder)
        self.assertNotRegex(
            neutral_transform,
            re.compile(r"valtan|kouku|kakul|saydon", re.IGNORECASE),
        )

    def test_world_and_network_stable_identifiers_remain_protected(self) -> None:
        admission = read(
            "Tools/KoukuSaydonPipeline/validate_kouku_saydon_world_admission.py"
        )
        world_builder = read(
            "Tools/KoukuSaydonPipeline/build_kouku_saydon_world_authoring.py"
        )
        valtan_nav_builder = read(
            "Tools/LevelPlacementExtractor/build_valtan_navgrid.py"
        )
        client_contract = read(
            "Tools/KoukuSaydonPipeline/test_kouku_saydon_client_product_level_contract.py"
        )

        self.assertTrue(
            (ROOT / "Tools/LevelPlacementExtractor/placement_transform.py").is_file()
        )
        self.assertIn("from placement_transform import", world_builder)
        self.assertNotIn("from build_valtan_navgrid import", world_builder)
        self.assertIn("from .placement_transform import", valtan_nav_builder)

        for token in (
            'COLLECTION_NAME = "KoukuSaton"',
            'WORLD_ID = "KAKULSAYDON_ARENA"',
            'CLIENT_LEVEL = "KAKULSAYDON_ARENA"',
            '"raid.kakul-saydon.arena"',
            'Path("Client/Public/Level_KakulSaydonArena.h")',
        ):
            self.assertIn(token, admission)
        for token in (
            'placement_id="stage.kakul."',
            'f"player.spawn.kakul.party{index:02d}"',
            '"schema": "lostark.kakul-stage-markers"',
        ):
            self.assertIn(token, world_builder)
        for token in (
            'LEVEL = "KAKULSAYDON_ARENA"',
            'read("Client/Public/Level_KakulSaydonArena.h")',
            '"stage.kakul.sl01"',
        ):
            self.assertIn(token, client_contract)


if __name__ == "__main__":
    unittest.main()

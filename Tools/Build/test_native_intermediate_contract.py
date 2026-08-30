#!/usr/bin/env python3

from __future__ import annotations

import unittest
import xml.etree.ElementTree as ET
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
MSBUILD_NAMESPACE = {"msb": "http://schemas.microsoft.com/developer/msbuild/2003"}


def int_dirs(relative_project_path: str) -> list[str]:
    root = ET.parse(REPOSITORY_ROOT / relative_project_path).getroot()
    return [
        node.text or ""
        for node in root.findall(".//msb:IntDir", MSBUILD_NAMESPACE)
    ]


class NativeIntermediateContractTests(unittest.TestCase):
    def test_engine_and_client_have_project_absolute_incremental_roots(self) -> None:
        expected = "$(ProjectDir)$(Platform)\\$(Configuration)\\"
        self.assertEqual(
            [expected],
            int_dirs("Engine/Default/Engine.vcxproj"),
            "Engine direct and solution builds must not fork their IntDir",
        )
        self.assertEqual([expected], int_dirs("Client/Default/Client.vcxproj"))

    def test_shared_and_server_use_their_explicit_intermediate_roots(self) -> None:
        expected = "$(ProjectDir)..\\Intermediate\\$(Platform)\\$(Configuration)\\"
        self.assertEqual([expected], int_dirs("Shared/Default/Shared.vcxproj"))
        self.assertEqual([expected], int_dirs("Server/Default/Server.vcxproj"))

    def test_no_native_project_lets_project_name_change_the_cache_path(self) -> None:
        projects = (
            "Engine/Default/Engine.vcxproj",
            "Client/Default/Client.vcxproj",
            "Shared/Default/Shared.vcxproj",
            "Server/Default/Server.vcxproj",
        )
        for project in projects:
            with self.subTest(project=project):
                values = int_dirs(project)
                self.assertEqual(1, len(values))
                self.assertNotIn("$(ProjectName)", values[0])


if __name__ == "__main__":
    unittest.main()

#!/usr/bin/env python3
"""Cross-file contract for the active team LAN endpoint.

This intentionally ignores historical PLAN/RESULT documents and isolated
localhost harnesses.  It verifies only current executable defaults, debugger
settings, and public handoff documents.
"""

from __future__ import annotations

import ipaddress
import json
from pathlib import Path
import unittest
import xml.etree.ElementTree as ET


ROOT = Path(__file__).resolve().parents[2]


class TeamLanEndpointContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.endpoint = json.loads(
            (ROOT / "Tools/Network/TeamLanEndpoint.json").read_text(
                encoding="utf-8"
            )
        )

    def test_endpoint_is_one_concrete_ipv4(self) -> None:
        self.assertEqual("lostark.team-lan-endpoint", self.endpoint["schema"])
        self.assertEqual(1, self.endpoint["version"])
        self.assertEqual("0.0.0.0", self.endpoint["serverBindAddress"])
        address = ipaddress.ip_address(self.endpoint["serverHost"])
        self.assertEqual(4, address.version)
        self.assertFalse(address.is_unspecified)
        self.assertFalse(address.is_loopback)
        self.assertEqual(7777, self.endpoint["port"])

    def test_client_compiled_and_debugger_defaults_match(self) -> None:
        host = self.endpoint["serverHost"]
        network_cpp = (
            ROOT / "Client/Private/NetworkManager.cpp"
        ).read_text(encoding="utf-8")
        self.assertIn(f'DEFAULT_SERVER_HOST[] = "{host}"', network_cpp)

        project = ET.parse(ROOT / "Client/Default/Client.vcxproj")
        namespace = {"m": "http://schemas.microsoft.com/developer/msbuild/2003"}
        values = [
            node.text or ""
            for node in project.findall(".//m:LocalDebuggerEnvironment", namespace)
        ]
        self.assertEqual(2, len(values))
        self.assertTrue(
            all(value == f"LOSTARK_SERVER_HOST={host}" for value in values)
        )

    def test_current_public_handoffs_match(self) -> None:
        host = self.endpoint["serverHost"]
        for relative in (
            "AGENTS.md",
            "CLAUDE.md",
            ".md/TEAM/README.md",
            ".md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md",
            ".md/TEAM/네트워크연결가이드.md",
        ):
            text = (ROOT / relative).read_text(encoding="utf-8")
            self.assertIn(host, text, relative)


if __name__ == "__main__":
    unittest.main()

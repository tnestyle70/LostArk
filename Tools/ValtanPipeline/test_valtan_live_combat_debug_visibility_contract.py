#!/usr/bin/env python3
"""Source contract for revisioned live combat geometry diagnostics."""

from pathlib import Path
import unittest
import xml.etree.ElementTree as ET


ROOT = Path(__file__).resolve().parents[2]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


class LiveCombatDebugVisibilityContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.contract = read("Client/Public/CombatDebugVisibility.h")
        cls.replication_h = read("Client/Public/ClientReplication.h")
        cls.replication_cpp = read("Client/Private/ClientReplication.cpp")
        cls.main_app = read("Client/Private/MainApp.cpp")
        cls.valtan_h = read("Client/Public/Valtan.h")
        cls.valtan_cpp = read("Client/Private/Valtan.cpp")
        cls.character_select = read("Client/Private/Level_CharacterSelect.cpp")

    def test_snapshot_owns_exactly_five_independent_visibility_values(self) -> None:
        for name in (
            "bBossBodyCollider",
            "bBossPatternHitPulse",
            "bBossStageGeometry",
            "bCounterProxy",
            "bPlayerSkillHitGeometry",
        ):
            self.assertIn(name, self.contract)
        self.assertIn("iRevision", self.contract)
        self.assertIn("Has_SameVisibility", self.contract)
        self.assertIn("Next_Revision", self.contract)

    def test_f1_developer_tools_exposes_all_five_global_switches(self) -> None:
        self.assertIn('SeparatorText("Live Combat Geometry")', self.main_app)
        for label in (
            "Boss Body Collider",
            "Boss Pattern Hit Pulse",
            "Boss Stage Geometry (whole Stage)",
            "Counter Proxy",
            "Player Skill Hit Geometry",
        ):
            self.assertIn(f'"{label}"', self.main_app)
        self.assertIn("Set_GlobalCombatDebugVisibility", self.main_app)
        self.assertNotIn('"Show Combat Colliders"', self.character_select)
        self.assertNotIn('"Show Skill Hit Areas"', self.character_select)

    def test_every_replication_consumes_the_process_revision(self) -> None:
        self.assertIn("Get_GlobalCombatDebugVisibility", self.replication_h)
        self.assertIn("Set_GlobalCombatDebugVisibility", self.replication_h)
        self.assertIn("Sync_GlobalCombatDebugVisibility", self.replication_h)
        update_start = self.replication_cpp.index(
            "bool Client::CClientReplication::Update()"
        )
        disconnected = self.replication_cpp.index("if (!isConnected)", update_start)
        sync = self.replication_cpp.index(
            "Sync_GlobalCombatDebugVisibility();", update_start
        )
        self.assertLess(sync, disconnected)
        self.assertIn(
            "Visibility.iRevision == m_CombatDebugVisibility.iRevision",
            self.replication_cpp,
        )
        self.assertIn(
            "Visibility.bPlayerSkillHitGeometry", self.replication_cpp
        )
        self.assertIn("Visibility.bBossBodyCollider", self.replication_cpp)
        self.assertIn("Visibility.bBossPatternHitPulse", self.replication_cpp)
        self.assertIn("Visibility.bBossStageGeometry", self.replication_cpp)
        self.assertIn("Visibility.bCounterProxy", self.replication_cpp)

    def test_live_boss_draw_paths_are_separate_but_preview_stays_independent(self) -> None:
        for name in (
            "m_isPatternHitPulseDebugVisible",
            "m_isPatternStageGeometryDebugVisible",
            "m_isCounterProxyDebugVisible",
        ):
            self.assertIn(name, self.valtan_h)
        self.assertIn(
            "isPreviewDriven || m_isPatternHitPulseDebugVisible",
            self.valtan_cpp,
        )
        self.assertIn(
            "isAuthoringGeometryWindow || isLiveStageGeometryWindow",
            self.valtan_cpp,
        )
        self.assertIn(
            "isPreviewDriven || m_isCounterProxyDebugVisible",
            self.valtan_cpp,
        )
        self.assertIn(
            "m_isCombatColliderDebugVisible && nullptr != m_pColliderCom",
            self.valtan_cpp,
        )

    def test_new_native_contract_is_registered_and_projects_parse(self) -> None:
        client_project = ROOT / "Client/Default/Client.vcxproj"
        harness_project = ROOT / (
            "Tools/ValtanPatternAuditionServiceHarness/Default/"
            "ValtanPatternAuditionServiceHarness.vcxproj"
        )
        ET.parse(client_project)
        ET.parse(harness_project)
        self.assertIn("CombatDebugVisibility.h", client_project.read_text("utf-8"))
        harness_text = harness_project.read_text("utf-8")
        self.assertIn("CombatDebugVisibility.h", harness_text)
        self.assertIn("CombatDebugVisibilityContractTests.cpp", harness_text)
        harness_main = read(
            "Tools/ValtanPatternAuditionServiceHarness/Private/"
            "ValtanPatternAuditionServiceHarness.cpp"
        )
        self.assertIn("Run_CombatDebugVisibilityContractTests", harness_main)


if __name__ == "__main__":
    unittest.main()

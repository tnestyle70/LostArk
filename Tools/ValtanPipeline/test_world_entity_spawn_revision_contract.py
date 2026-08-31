import pathlib
import unittest


REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]


class WorldEntitySpawnRevisionContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.packet_type = (
            REPO_ROOT / "Shared/Public/Network/PacketType.h"
        ).read_text(encoding="utf-8")
        cls.messages_h = (
            REPO_ROOT / "Shared/Public/Network/PacketMessages.h"
        ).read_text(encoding="utf-8")
        cls.messages_cpp = (
            REPO_ROOT / "Shared/Private/Network/PacketMessages.cpp"
        ).read_text(encoding="utf-8")
        cls.game_room_cpp = (
            REPO_ROOT / "Server/Private/GameRoom.cpp"
        ).read_text(encoding="utf-8")
        cls.server_tests = (
            REPO_ROOT / "Server/Private/ServerGameplayContractTests.cpp"
        ).read_text(encoding="utf-8")
        cls.network_manager = (
            REPO_ROOT / "Client/Private/NetworkManager.cpp"
        ).read_text(encoding="utf-8")
        cls.replication_h = (
            REPO_ROOT / "Client/Public/ClientReplication.h"
        ).read_text(encoding="utf-8")
        cls.replication_cpp = (
            REPO_ROOT / "Client/Private/ClientReplication.cpp"
        ).read_text(encoding="utf-8")
        cls.combat_projection_h = (
            REPO_ROOT / "Client/Public/CombatObjectProjectionRuntime.h"
        ).read_text(encoding="utf-8")
        cls.combat_projection_cpp = (
            REPO_ROOT / "Client/Private/CombatObjectProjectionRuntime.cpp"
        ).read_text(encoding="utf-8")

    def test_wire_requires_one_exact_spawn_revision(self) -> None:
        self.assertIn("NETWORK_PROTOCOL_VERSION = 50;", self.packet_type)
        spawned_start = self.messages_h.index("struct S2C_WORLD_ENTITY_SPAWNED")
        spawned_end = self.messages_h.index("bool Write_Message", spawned_start)
        spawned = self.messages_h[spawned_start:spawned_end]
        self.assertIn("GameplayDataRevision PinnedDefinitionRevision{};", spawned)

        writer_start = self.messages_cpp.index(
            "const S2C_WORLD_ENTITY_SPAWNED& spawned)"
        )
        writer_end = self.messages_cpp.index(
            "bool LostArk::Shared::Read_Message(", writer_start
        )
        writer = self.messages_cpp[writer_start:writer_end]
        self.assertIn("!spawned.PinnedDefinitionRevision.Is_Valid()", writer)
        self.assertIn(
            "Write_GameplayDataRevision(\n\t\twriter, spawned.PinnedDefinitionRevision)",
            writer,
        )

        reader_start = writer_end
        reader_end = self.messages_cpp.index(
            "bool LostArk::Shared::Is_Valid_WorldEntitySpawnOwner", reader_start
        )
        reader = self.messages_cpp[reader_start:reader_end]
        self.assertIn("decoded.PinnedDefinitionRevision", reader)
        self.assertIn("!decoded.PinnedDefinitionRevision.Is_Valid()", reader)

    def test_server_and_client_propagate_the_same_pin(self) -> None:
        build_start = self.game_room_cpp.index(
            "CGameRoom::Build_WorldEntitySpawnedPayload("
        )
        build_end = self.game_room_cpp.index(
            "bool LostArk::Server::CGameRoom::Send_WorldEntityDespawned",
            build_start,
        )
        build = self.game_room_cpp[build_start:build_end]
        self.assertIn(
            "message.PinnedDefinitionRevision = entity.PinnedDefinitionRevision;",
            build,
        )

        spawn_case = self.network_manager[
            self.network_manager.index("case PACKET_TYPE::S2C_WORLD_ENTITY_SPAWNED") :
            self.network_manager.index("case PACKET_TYPE::S2C_COMBAT_OBJECT_SPAWNED")
        ]
        self.assertIn(
            "Is_AnnouncedWorldRevision(spawned.PinnedDefinitionRevision)", spawn_case
        )
        self.assertIn(
            "Is_PresentationRevisionAvailable(\n\t\t\t\tspawned.PinnedDefinitionRevision)",
            spawn_case,
        )
        self.assertNotIn("ServerActiveRevision", spawn_case)

    def test_snapshot_revision_change_is_bounded_and_isolated(self) -> None:
        for token in (
            "AdmittedPresentationRevision",
            "RejectedPresentationRevision",
            "bPresentationIsolated",
            "Ensure_ValtanPresentationRevision(",
        ):
            self.assertIn(token, self.replication_h)

        ensure_start = self.replication_cpp.index(
            "Ensure_ValtanPresentationRevision("
        )
        ensure_end = self.replication_cpp.index(
            "Reload_PrimaryValtanPresentationAuthoring(", ensure_start
        )
        ensure = self.replication_cpp[ensure_start:ensure_end]
        self.assertLess(
            ensure.index("RejectedPresentationRevision == ExpectedRevision"),
            ensure.index("Try_Get_ValtanPresentationGenerationReceipt("),
        )
        for token in (
            "Reload_PatternPresentationAuthoring(",
            "Presentation.bPresentationIsolated = true;",
            "CEffectPresentationService::Stop_BossOwner(pValtan);",
            "Presentation.bPresentationIsolated = false;",
        ):
            self.assertIn(token, ensure)

        snapshot_start = self.replication_cpp.index(
            "bool Client::CClientReplication::Apply_WorldSnapshot("
        )
        snapshot_end = self.replication_cpp.index(
            "Client::CClientReplication::CHARACTER_REPLACE_RESULT", snapshot_start
        )
        snapshot = self.replication_cpp[snapshot_start:snapshot_end]
        self.assertLess(
            snapshot.index("Ensure_ValtanPresentationRevision("),
            snapshot.index("valtan->Apply_NetworkState("),
        )
        self.assertIn("boss->second.bPresentationIsolated", snapshot)
        self.assertIn("hitEntity->second.bPresentationIsolated", snapshot)

        self.assertIn(
            "retry.PinnedDefinitionRevision =",
            self.combat_projection_h,
        )
        for token in (
            "!message.PinnedDefinitionRevision.Is_Valid()",
            "message.PinnedDefinitionRevision",
            "!object.PinnedDefinitionRevision.Is_Valid()",
            "object.PinnedDefinitionRevision",
        ):
            self.assertIn(token, self.combat_projection_cpp)

    def test_late_join_native_oracle_keeps_existing_r_old(self) -> None:
        for token in (
            "activeRevision",
            "retainedOccurrenceRevision",
            "message.PinnedDefinitionRevision ==",
            "activeRevision != retainedOccurrenceRevision",
            "active R_new serializes an existing primary and ghost at their exact retained R_old",
        ):
            self.assertIn(token, self.server_tests)


if __name__ == "__main__":
    unittest.main()

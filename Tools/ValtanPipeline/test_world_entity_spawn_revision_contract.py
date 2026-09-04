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
        cls.network_manager_h = (
            REPO_ROOT / "Client/Public/NetworkManager.h"
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
        self.assertIn("NETWORK_PROTOCOL_VERSION = 54;", self.packet_type)
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
        self.assertIn(
            '"presentation.authoritative-entity-forwarded"', spawn_case
        )
        self.assertIn("Record_PresentationIsolation(", spawn_case)
        isolation_start = spawn_case.index(
            "if (!Is_PresentationRevisionAvailable("
        )
        enqueue_start = spawn_case.index(
            "Enqueue_ReplicationEvent(std::move(event))", isolation_start
        )
        self.assertNotIn("break;", spawn_case[isolation_start:enqueue_start])
        self.assertNotIn("return;", spawn_case[isolation_start:enqueue_start])
        self.assertNotIn("ServerActiveRevision", spawn_case)

    def test_unavailable_presentation_keeps_authoritative_snapshot_truth(self) -> None:
        snapshot_case = self.network_manager[
            self.network_manager.index(
                "case PACKET_TYPE::S2C_WORLD_SNAPSHOT"
            ) : self.network_manager.index(
                "case PACKET_TYPE::S2C_WORLD_DESTRUCTION_FULL_SYNC"
            )
        ]
        self.assertIn(
            '"World-entity occurrence"', snapshot_case
        )
        self.assertNotIn("snapshot.Entities.erase(", snapshot_case)
        self.assertIn("snapshot.CombatObjects.erase(", snapshot_case)
        self.assertIn("snapshot.BossCombatEvents.erase(", snapshot_case)
        self.assertIn("isolatedEntityIds", snapshot_case)
        self.assertIn(
            "CClientReplication owns the\n"
            "\t\t   per-entity presentation fallback",
            snapshot_case,
        )

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
            "bEntryReceiptRecoveryPending",
            "hasPendingEntryPresentationBaselineRecovery",
            "Presentation.bPresentationIsolated = true;",
            "CEffectPresentationService::Stop_BossOwner(pValtan);",
            "Presentation.bPresentationIsolated = false;",
        ):
            self.assertIn(token, ensure)
        self.assertIn(
            "bEntryReceiptRecoveryPending ?\n"
            "\t\t\tLostArk::Shared::GameplayDataRevision{} : ExpectedRevision",
            ensure,
        )
        self.assertIn(
            "if (bIsPrimary && !bEntryReceiptRecoveryPending)", ensure
        )

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

    def test_entry_lock_contention_retries_without_latching_primary_rejection(self) -> None:
        for token in (
            "hasPendingEntryPresentationBaselineRecovery",
            "iNextEntryPresentationBaselineRecoveryAtMilliseconds",
            "Try_Recover_EntryPresentationBaseline",
        ):
            self.assertIn(token, self.network_manager_h)

        recovery_start = self.network_manager.index(
            "bool CNetworkManager::Try_Recover_EntryPresentationBaseline("
        )
        recovery_end = self.network_manager.index(
            "bool CNetworkManager::Try_Get_ValtanPresentationGenerationReceipt(",
            recovery_start,
        )
        recovery = self.network_manager[recovery_start:recovery_end]
        for token in (
            "ENTRY_PRESENTATION_BASELINE_RETRY_MILLISECONDS",
            "CapturePresentationArtifactBaseline(",
            "captureDiagnostic.Is_AutomaticRetryable()",
            "BootstrapPresentationReceipt",
            "BootstrapPresentationRevision = activeRevision",
            '"presentation.baseline-recovered"',
        ):
            self.assertIn(token, recovery)
        capture = recovery.index("CapturePresentationArtifactBaseline(")
        commit = recovery.index(
            "m_GameplayRevisionState.PresentationArtifactBaseline =", capture
        )
        self.assertLess(capture, commit)
        self.assertIn(
            "m_GameplayRevisionState.ServerActiveRevision != activeRevision",
            recovery[capture:commit],
        )

        spawn_start = self.replication_cpp.index(
            "bool Client::CClientReplication::Apply_WorldEntitySpawn("
        )
        spawn_end = self.replication_cpp.index(
            "void Client::CClientReplication::Remove_DependentBossPresentations(",
            spawn_start,
        )
        spawn = self.replication_cpp[spawn_start:spawn_end]
        self.assertIn("entryReceiptRecoveryPending", spawn)
        self.assertIn("if (!entryReceiptRecoveryPending)", spawn)
        self.assertIn(
            "presentation.RejectedPresentationRevision =", spawn
        )

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

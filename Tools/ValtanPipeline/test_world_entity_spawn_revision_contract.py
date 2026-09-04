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
        cls.loader_cpp = (
            REPO_ROOT / "Client/Private/Loader.cpp"
        ).read_text(encoding="utf-8")
        cls.valtan_h = (
            REPO_ROOT / "Client/Public/Valtan.h"
        ).read_text(encoding="utf-8")
        cls.valtan_cpp = (
            REPO_ROOT / "Client/Private/Valtan.cpp"
        ).read_text(encoding="utf-8")
        cls.combat_projection_h = (
            REPO_ROOT / "Client/Public/CombatObjectProjectionRuntime.h"
        ).read_text(encoding="utf-8")
        cls.combat_projection_cpp = (
            REPO_ROOT / "Client/Private/CombatObjectProjectionRuntime.cpp"
        ).read_text(encoding="utf-8")

    def test_wire_requires_one_exact_spawn_revision(self) -> None:
        self.assertIn("NETWORK_PROTOCOL_VERSION = 55;", self.packet_type)
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

    def test_dependent_valtan_ghosts_use_one_exact_dormant_pool(self) -> None:
        ready_start = self.loader_cpp.index("HRESULT CLoader::Ready_ValtanPresentation")
        ready_end = self.loader_cpp.index("unique_ptr<CLoader> CLoader::Create", ready_start)
        ready = self.loader_cpp[ready_start:ready_end]
        self.assertLess(ready.index('"BOSS_VALTAN"'), ready.index('"BOSS_VALTAN_GHOST"'))

        for token in (
            "VALTAN_GHOST_PRESENTATION_POOL_CAPACITY = 4u",
            "bUsesValtanGhostPool",
            "AdmittedPresentationReceipt",
            "Copy_AdmittedPatternPresentationFrom(",
            "Activate_ReplicatedPoolOccurrence(",
            "Return_ToReplicatedPool()",
            "m_DeferredValtanGhostPresentationPoolRefresh",
        ):
            self.assertIn(token, self.replication_h + self.valtan_h)

        prepare_start = self.replication_cpp.index(
            "Prepare_ValtanGhostPresentationPool("
        )
        prepare_end = self.replication_cpp.index(
            "Checkout_ValtanGhostPresentation(", prepare_start
        )
        prepare = self.replication_cpp[prepare_start:prepare_end]
        ignore = "EFFECT_V2_TARGET::From_Valtan(valtan), true);"
        self.assertIn(ignore, prepare)
        self.assertLess(
            prepare.index(ignore),
            prepare.index("Copy_AdmittedPatternPresentationFrom("),
        )
        self.assertIn("Slot.AdmittedPresentationReceipt == receipt", prepare)

        spawn_start = self.replication_cpp.index(
            "bool Client::CClientReplication::Apply_WorldEntitySpawn("
        )
        spawn_end = self.replication_cpp.index(
            "void Client::CClientReplication::Remove_DependentBossPresentations(",
            spawn_start,
        )
        spawn = self.replication_cpp[spawn_start:spawn_end]
        for token in (
            '"BOSS_VALTAN_GHOST" == spawned.strArchetypeId',
            '"valtan.ghost.portal-once.active" == spawned.strActionId',
            "bHoldPortalRunnerBodyHiddenUntilPatternSnapshot",
            "CValtanPresentationAssetService::Is_Ready(",
            "Checkout_ValtanGhostPresentation(",
            "has no dormant pool slot for its exact presentation generation",
            "const bool_t joinedReloaded = isPooledGhost ||",
            "presentation.bUsesValtanGhostPool = isPooledGhost;",
        ):
            self.assertIn(token, spawn)
        checkout_start = spawn.index("if (isPooledGhost)")
        checkout = spawn[
            checkout_start : spawn.index("\n\telse if", checkout_start)
        ]
        self.assertNotIn("Add_GameObject_to_Layer(", checkout)
        self.assertNotIn("Reload_PatternPresentationAuthoring(", checkout)

        for token in (
            "m_isReplicationDormant",
            "m_bHoldSpawnBodyHiddenUntilPatternSnapshot",
            "m_bGhostPortalRoutePresentationActive",
            "m_fGhostPortalRoutePresentationAgeSeconds",
            '"VALTAN_GHOST_PORTAL_ONCE" == patternId',
            '"valtan.ghost.portal-once.active" == actionId',
            "m_PatternBodyVisibilityByActionId.find(actionId)",
            "CEffectV2Runtime::Set_Ignored(",
            "Reset_ReplicatedOccurrenceState();",
            "Update_GhostPortalRoutePresentation(fTimeDelta);",
            "XMVectorLerp(vStart, vEnd, fRushRatio)",
            "VALTAN_GHOST_PORTAL_BODY_TERMINAL_HIDE_MS",
        ):
            self.assertIn(token, self.valtan_cpp)

        checkin_start = self.replication_cpp.index(
            "Checkin_ValtanGhostPresentation("
        )
        checkin_end = self.replication_cpp.index(
            "Clear_ValtanGhostPresentationPool()", checkin_start
        )
        checkin = self.replication_cpp[checkin_start:checkin_end]
        self.assertIn("bAllSlotsReturned", checkin)
        self.assertIn(
            "Retry_DeferredValtanGhostPresentationPoolRefresh(", checkin
        )

        apply_start = self.valtan_cpp.index("bool_t CValtan::Apply_NetworkState(")
        apply = self.valtan_cpp[apply_start:]
        self.assertLess(
            apply.index('"VALTAN_GHOST_PORTAL_ONCE" == patternId'),
            apply.index(
                "m_bHoldSpawnBodyHiddenUntilPatternSnapshot = false;"
            ),
        )
        self.assertIn(
            "if (!m_bHoldSpawnBodyHiddenUntilPatternSnapshot)\n"
            "\t\t\tm_isPatternBodyHidden = false;",
            apply,
        )


if __name__ == "__main__":
    unittest.main()

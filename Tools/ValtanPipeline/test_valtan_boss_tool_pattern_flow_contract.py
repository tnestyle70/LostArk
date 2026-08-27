#!/usr/bin/env python3
"""Focused authoring/document contracts for the Valtan Boss Tool Pattern Flow."""

from __future__ import annotations

import copy
import json
import pathlib
import re
import unittest
import xml.etree.ElementTree as ET


ROOT = pathlib.Path(__file__).resolve().parents[2]
DOCUMENT_H = ROOT / "Client/Public/ValtanPatternFlowDocument.h"
DOCUMENT_CPP = ROOT / "Client/Private/ValtanPatternFlowDocument.cpp"
SERVICE_H = ROOT / "Client/Public/ValtanPatternFlowService.h"
SERVICE_CPP = ROOT / "Client/Private/ValtanPatternFlowService.cpp"
BOSS_TOOL_CPP = ROOT / "Client/Private/BossTool.cpp"
NETWORK_H = ROOT / "Client/Public/NetworkManager.h"
NETWORK_CPP = ROOT / "Client/Private/NetworkManager.cpp"
MAIN_APP_CPP = ROOT / "Client/Private/MainApp.cpp"
FLOW_JSON = ROOT / "Data/Encounters/Valtan/ValtanBossAuditionFlows.json"
GAMEPLAY_JSON = ROOT / "Data/Valtan/Valtan.gameplay.json"
PROJECT = ROOT / "Client/Default/Client.vcxproj"
FILTERS = ROOT / "Client/Default/Client.vcxproj.filters"
GOTCHAS = ROOT / ".md/GB/gotchas.md"
PATTERN_TREE_H = ROOT / "Client/Public/ValtanPatternTree.h"
PATTERN_TREE_CPP = ROOT / "Client/Private/ValtanPatternTree.cpp"
PACKET_TYPE_H = ROOT / "Shared/Public/Network/PacketType.h"
PACKET_MESSAGES_H = ROOT / "Shared/Public/Network/PacketMessages.h"
PACKET_MESSAGES_CPP = ROOT / "Shared/Private/Network/PacketMessages.cpp"
PROTOCOL_HARNESS_CPP = (
    ROOT / "Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp"
)
SERVER_ROOM_H = ROOT / "Server/Public/GameRoom.h"
SERVER_ROOM_CPP = ROOT / "Server/Private/GameRoom.cpp"
SERVER_BRAIN_H = ROOT / "Server/Public/ValtanBrain.h"
SERVER_BRAIN_CPP = ROOT / "Server/Private/ValtanBrain.cpp"
SERVER_APP_CPP = ROOT / "Server/Private/ServerApp.cpp"
SERVER_TESTS_CPP = ROOT / "Server/Private/ServerGameplayContractTests.cpp"

SCHEMA = "lostark.valtan-boss-audition-flows"
FLOW_ID = "flow.valtan.boss-tool.default"
MAX_SLOTS = 32
STABLE_ID = re.compile(r"[A-Za-z0-9_.-]{1,128}")
CORE_PATTERN_IDS = [
    "VALTAN_WHIRLWIND",
    "VALTAN_FOUR_SLASH",
    "VALTAN_HIGH_JUMP",
    "VALTAN_DASH_CHARGE",
    "VALTAN_FLOOR_WIPE_130",
    "VALTAN_ARENA_BREAK_109",
    "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK",
    "VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK",
]


def make_document(pattern_ids: list[str]) -> dict:
    """Build test fixtures without depending on the user's saved slot order."""
    return {
        "schema": SCHEMA,
        "formatVersion": 1,
        "flows": [{
            "flowId": FLOW_ID,
            "nextSlotOrdinal": len(pattern_ids) + 1,
            "interStepPursuitMs": 1000,
            "slots": [
                {"slotId": f"{FLOW_ID}.slot.{ordinal:06d}", "patternId": pattern_id}
                for ordinal, pattern_id in enumerate(pattern_ids, start=1)
            ],
        }],
    }


def validate_document(document: object, admitted_pattern_ids: list[str]) -> None:
    if not isinstance(document, dict) or set(document) != {
        "schema", "formatVersion", "flows"
    }:
        raise ValueError("root properties")
    if document["schema"] != SCHEMA or type(document["formatVersion"]) is not int:
        raise ValueError("header types")
    if document["formatVersion"] != 1:
        raise ValueError("version")
    flows = document["flows"]
    if not isinstance(flows, list) or len(flows) != 1:
        raise ValueError("flow count")
    flow = flows[0]
    if not isinstance(flow, dict) or set(flow) != {
        "flowId", "nextSlotOrdinal", "interStepPursuitMs", "slots"
    }:
        raise ValueError("flow properties")
    if flow["flowId"] != FLOW_ID or STABLE_ID.fullmatch(flow["flowId"]) is None:
        raise ValueError("flow id")
    if type(flow["nextSlotOrdinal"]) is not int or not (
        1 <= flow["nextSlotOrdinal"] <= 1_000_000
    ):
        raise ValueError("next ordinal")
    if type(flow["interStepPursuitMs"]) is not int or not (
        100 <= flow["interStepPursuitMs"] <= 10_000
    ):
        raise ValueError("pursuit")
    slots = flow["slots"]
    if not isinstance(slots, list) or len(slots) > MAX_SLOTS:
        raise ValueError("slots")
    admitted = set(admitted_pattern_ids)
    if len(admitted) != len(admitted_pattern_ids) or not admitted:
        raise ValueError("inventory")
    slot_ids: set[str] = set()
    maximum_ordinal = 0
    prefix = FLOW_ID + ".slot."
    for slot in slots:
        if not isinstance(slot, dict) or set(slot) != {"slotId", "patternId"}:
            raise ValueError("slot properties")
        slot_id = slot["slotId"]
        pattern_id = slot["patternId"]
        if (
            not isinstance(slot_id, str)
            or STABLE_ID.fullmatch(slot_id) is None
            or not slot_id.startswith(prefix)
            or len(slot_id) != len(prefix) + 6
            or not slot_id[-6:].isdigit()
            or int(slot_id[-6:]) == 0
            or slot_id in slot_ids
        ):
            raise ValueError("slot id")
        if (
            not isinstance(pattern_id, str)
            or STABLE_ID.fullmatch(pattern_id) is None
            or pattern_id not in admitted
        ):
            raise ValueError("pattern id")
        slot_ids.add(slot_id)
        maximum_ordinal = max(maximum_ordinal, int(slot_id[-6:]))
    if flow["nextSlotOrdinal"] <= maximum_ordinal:
        raise ValueError("slot reuse")


class ValtanBossToolPatternFlowDocumentContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.header = DOCUMENT_H.read_text(encoding="utf-8")
        cls.source = DOCUMENT_CPP.read_text(encoding="utf-8")
        cls.service_header = SERVICE_H.read_text(encoding="utf-8")
        cls.service_source = SERVICE_CPP.read_text(encoding="utf-8")
        cls.boss_tool = BOSS_TOOL_CPP.read_text(encoding="utf-8")
        cls.network_header = NETWORK_H.read_text(encoding="utf-8")
        cls.network_source = NETWORK_CPP.read_text(encoding="utf-8")
        cls.main_app = MAIN_APP_CPP.read_text(encoding="utf-8")
        cls.flow = json.loads(FLOW_JSON.read_text(encoding="utf-8"))
        cls.gameplay = json.loads(GAMEPLAY_JSON.read_text(encoding="utf-8"))
        cls.project = PROJECT.read_text(encoding="utf-8")
        cls.filters = FILTERS.read_text(encoding="utf-8")
        cls.gotchas = GOTCHAS.read_text(encoding="utf-8")
        cls.packet_type = PACKET_TYPE_H.read_text(encoding="utf-8")
        cls.packet_messages = (
            PACKET_MESSAGES_H.read_text(encoding="utf-8")
            + PACKET_MESSAGES_CPP.read_text(encoding="utf-8")
        )
        cls.protocol_harness = PROTOCOL_HARNESS_CPP.read_text(encoding="utf-8")
        cls.server_room_header = SERVER_ROOM_H.read_text(encoding="utf-8")
        cls.server_room = SERVER_ROOM_CPP.read_text(encoding="utf-8")
        cls.server_brain = (
            SERVER_BRAIN_H.read_text(encoding="utf-8")
            + SERVER_BRAIN_CPP.read_text(encoding="utf-8")
        )
        cls.server_app = SERVER_APP_CPP.read_text(encoding="utf-8")
        cls.server_tests = SERVER_TESTS_CPP.read_text(encoding="utf-8")
        manual = cls.gameplay["decisionModel"]["manualAuditions"]
        cls.inventory = CORE_PATTERN_IDS + [row["patternId"] for row in manual]

    def test_saved_document_is_strict_without_requiring_the_initial_seed(self) -> None:
        # This is editable authoring data: reorder, removal, repeated patterns,
        # and even an empty draft are valid. Only its public schema is fixed.
        validate_document(self.flow, self.inventory)

    def test_initial_seed_fixture_starts_from_all_effects_inventory(self) -> None:
        seed = make_document(self.inventory)
        validate_document(seed, self.inventory)
        slots = seed["flows"][0]["slots"]
        self.assertEqual(28, len(self.inventory))
        self.assertEqual(self.inventory, [slot["patternId"] for slot in slots])
        self.assertEqual(29, seed["flows"][0]["nextSlotOrdinal"])
        self.assertEqual(28, len({slot["slotId"] for slot in slots}))

    def test_duplicate_pattern_ids_are_valid_but_duplicate_slot_ids_are_not(self) -> None:
        duplicate_pattern = make_document([self.inventory[0], self.inventory[0]])
        slots = duplicate_pattern["flows"][0]["slots"]
        self.assertNotEqual(slots[0]["slotId"], slots[1]["slotId"])
        validate_document(duplicate_pattern, self.inventory)

        duplicate_slot = copy.deepcopy(duplicate_pattern)
        duplicate_slot["flows"][0]["slots"][-1]["slotId"] = (
            duplicate_slot["flows"][0]["slots"][0]["slotId"]
        )
        with self.assertRaisesRegex(ValueError, "slot id"):
            validate_document(duplicate_slot, self.inventory)

    def test_reordered_slots_and_new_duplicate_pattern_keep_stable_ids(self) -> None:
        document = make_document(self.inventory)
        flow = document["flows"][0]
        slots = flow["slots"]
        original_slots = {slot["slotId"]: slot["patternId"] for slot in slots}

        # Reproduce the reported edit in memory, not by fixing the live JSON
        # to these 29 slots. Later user saves may legitimately differ again.
        new_slot_id = f"{FLOW_ID}.slot.{flow['nextSlotOrdinal']:06d}"
        slots.insert(0, {
            "slotId": new_slot_id,
            "patternId": "VALTAN_FLOOR_WIPE_130",
        })
        flow["nextSlotOrdinal"] += 1
        counter_slot = next(slot for slot in slots if slot["patternId"] == "VALTAN_COUNTER")
        slots.remove(counter_slot)
        after_four = next(
            index for index, slot in enumerate(slots)
            if slot["patternId"] == "VALTAN_SEQUENCE_FOUR"
        ) + 1
        slots.insert(after_four, counter_slot)

        round_trip = json.loads(json.dumps(document, ensure_ascii=False))
        validate_document(round_trip, self.inventory)
        self.assertEqual(document, round_trip)
        self.assertEqual(new_slot_id, slots[0]["slotId"])
        self.assertEqual(counter_slot, slots[after_four])
        self.assertEqual(2, sum(
            slot["patternId"] == "VALTAN_FLOOR_WIPE_130" for slot in slots
        ))
        self.assertEqual(original_slots, {
            slot["slotId"]: slot["patternId"] for slot in slots
            if slot["slotId"] != new_slot_id
        })
        self.assertEqual(30, flow["nextSlotOrdinal"])

    def test_empty_single_and_maximum_slot_documents_are_valid(self) -> None:
        for count in (0, 1, MAX_SLOTS):
            with self.subTest(slot_count=count):
                document = make_document([self.inventory[0]] * count)
                validate_document(document, self.inventory)
                self.assertEqual(count, len(document["flows"][0]["slots"]))

    def test_removed_slots_preserve_sparse_ids_and_next_ordinal(self) -> None:
        document = make_document(self.inventory)
        flow = document["flows"][0]
        original_ordinal = flow["nextSlotOrdinal"]
        flow["slots"] = list(reversed(flow["slots"][::2]))
        flow["interStepPursuitMs"] = 2500
        round_trip = json.loads(json.dumps(document))
        validate_document(round_trip, self.inventory)
        self.assertEqual(document, round_trip)
        self.assertEqual(original_ordinal, round_trip["flows"][0]["nextSlotOrdinal"])

        flow["slots"].clear()
        validate_document(document, self.inventory)
        self.assertEqual(original_ordinal, flow["nextSlotOrdinal"])

    def test_slot_ordinal_exhaustion_preserves_the_last_issued_id(self) -> None:
        document = make_document([self.inventory[0]])
        flow = document["flows"][0]
        flow["slots"][0]["slotId"] = f"{FLOW_ID}.slot.999999"
        flow["nextSlotOrdinal"] = 1_000_000
        validate_document(document, self.inventory)

        for ordinal, reason in ((999_999, "slot reuse"), (1_000_001, "next ordinal")):
            with self.subTest(next_slot_ordinal=ordinal):
                flow["nextSlotOrdinal"] = ordinal
                with self.assertRaisesRegex(ValueError, reason):
                    validate_document(document, self.inventory)

    def test_wrong_version_unknown_property_unknown_pattern_and_overflow_fail(self) -> None:
        mutations = []
        seed = make_document(self.inventory)
        wrong_version = copy.deepcopy(seed)
        wrong_version["formatVersion"] = 2
        mutations.append(("version", wrong_version))
        unknown_property = copy.deepcopy(seed)
        unknown_property["flows"][0]["slots"][0]["index"] = 0
        mutations.append(("slot properties", unknown_property))
        unknown_pattern = copy.deepcopy(seed)
        unknown_pattern["flows"][0]["slots"][0]["patternId"] = "UNKNOWN"
        mutations.append(("pattern id", unknown_pattern))
        # All IDs remain unique so this tests the 32-slot boundary itself,
        # not an unrelated duplicate-slot rejection.
        overflow = make_document([self.inventory[0]] * (MAX_SLOTS + 1))
        mutations.append(("slots", overflow))
        float_integer = copy.deepcopy(seed)
        float_integer["formatVersion"] = 1.0
        mutations.append(("header types", float_integer))

        for reason, candidate in mutations:
            with self.subTest(reason=reason):
                with self.assertRaisesRegex(ValueError, reason):
                    validate_document(candidate, self.inventory)

    def test_cpp_codec_stages_before_commit_and_keeps_pattern_tree_out(self) -> None:
        for marker in (
            "Has_ExactProperties(root, { \"schema\", \"formatVersion\", \"flows\" })",
            "Try_ParseUnsignedInteger",
            "Try_ParseSlotOrdinal",
            "admitted.contains(slot.strPatternId)",
            "outDocument = std::move(staged)",
            "m_Baseline = staged",
            "m_Draft = std::move(staged)",
        ):
            self.assertIn(marker, self.source)
        self.assertNotIn("ValtanPatternTree", self.header + self.source)
        self.assertNotIn("scriptedSequence", self.header + self.source)
        self.assertIn("std::unordered_set<std::string> slotIds", self.source)
        self.assertNotIn("patternIds.insert", self.source)

    def test_save_uses_raw_sha256_cas_durable_temp_and_recovery_backup(self) -> None:
        for marker in (
            "BCRYPT_SHA256_ALGORITHM",
            "diskRevision != m_strSourceRevision",
            "preCommitRevision != m_strSourceRevision",
            "_commit(_fileno(file))",
            "CopyFileW(m_Path.c_str(), backup.c_str(), TRUE)",
            "MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH",
            "backup.c_str(), m_Path.c_str()",
            "m_bExternalConflict = true",
            "m_Baseline = committedDocument",
            "m_strSourceRevision = std::move(committedRevision)",
        ):
            self.assertIn(marker, self.source)
        self.assertLess(
            self.source.index("preCommitRevision != m_strSourceRevision"),
            self.source.index("temporary.c_str(), m_Path.c_str()"),
        )
        self.assertLess(
            self.source.index("committedBytes != serialized"),
            self.source.index("m_Baseline = committedDocument"),
        )

    def test_public_api_keeps_stable_slot_mutation_and_safe_read_view(self) -> None:
        for marker in (
            "Add_Slot(",
            "Move_Slot(",
            "Remove_Slot(",
            "Set_InterStepPursuitMs(",
            "Is_Ready()",
            "Is_Dirty()",
            "Has_ExternalConflict()",
            "Get_Draft()",
            "Get_DefaultFlow()",
            "Get_SourceRevision()",
            "Verify_SourceRevision(",
        ):
            self.assertIn(marker, self.header)
        self.assertIn(
            "const VALTAN_PATTERN_FLOW_DEFINITION* Get_DefaultFlow() const noexcept",
            self.header,
        )

    def test_project_and_filter_register_each_new_source_once(self) -> None:
        ET.parse(PROJECT)
        ET.parse(FILTERS)
        for source in (self.project, self.filters):
            self.assertEqual(1, source.count("ValtanPatternFlowDocument.h"))
            self.assertEqual(1, source.count("ValtanPatternFlowDocument.cpp"))
            self.assertEqual(1, source.count("ValtanPatternFlowService.h"))
            self.assertEqual(1, source.count("ValtanPatternFlowService.cpp"))
            self.assertEqual(1, source.count("ValtanBossAuditionFlows.json"))
        self.assertIn("96.DataFiles\\Encounters", self.filters)

    def test_flow_tab_keeps_verification_and_reuses_single_pattern_submit(self) -> None:
        for marker in (
            'ImGui::BeginTabItem("Boss Verification")',
            'ImGui::BeginTabItem("Pattern Flow")',
            "Render_BossVerificationTab();",
            "Render_PatternFlowTab();",
            "RenderIds(m_AuditionInventory.CorePatternIds);",
            "RenderIds(m_AuditionInventory.AnimatorPatternIds);",
            "FLOW_PREVIEW_CONSUMER_ID",
            "CValtanPatternAuditionService::Get().Submit(",
            "Uses the same Server single-pattern path as Play Selected.",
        ):
            self.assertIn(marker, self.boss_tool)
        start_body = self.boss_tool[
            self.boss_tool.index("bool_t Client::CBossTool::Start_Flow") :
            self.boss_tool.index("bool_t Client::CBossTool::Request_RevivePlayer")
        ]
        self.assertIn("CValtanPatternFlowService::Get().Start(", start_body)
        self.assertNotIn("CValtanPatternAuditionService::Get().Submit(", start_body)
        self.assertNotIn("PendingPatternIds", self.boss_tool)

    def test_start_revalidates_inventory_and_exact_disk_revision(self) -> None:
        start_body = self.boss_tool[
            self.boss_tool.index("bool_t Client::CBossTool::Start_Flow") :
            self.boss_tool.index("bool_t Client::CBossTool::Request_RevivePlayer")
        ]
        self.assertIn("CValtanPatternFlowDocument::Validate(", start_body)
        self.assertIn("m_FlowDocument.Verify_SourceRevision", start_body)
        verify_body = self.source[
            self.source.index("Verify_SourceRevision(") :
            self.source.index("Add_Slot(", self.source.index("Verify_SourceRevision("))
        ]
        self.assertIn("diskRevision != m_strSourceRevision", verify_body)
        self.assertIn("m_bExternalConflict = true", verify_body)
        self.assertNotIn("m_Baseline =", verify_body)
        self.assertNotIn("m_Draft =", verify_body)

    def test_authoring_playback_and_preview_status_are_rendered_separately(self) -> None:
        for marker in (
            'ImGui::TextWrapped("Tool: %s", m_strFlowStatus.c_str())',
            '"Server: %s | %s"',
            '"Isolated preview: %s"',
        ):
            self.assertIn(marker, self.boss_tool)
        self.assertNotIn("m_strFlowStatus = FlowSnapshot.strStatus", self.boss_tool)
        self.assertNotIn("m_strFlowStatus = Audition.strStatus", self.boss_tool)
        isolated_status = self.boss_tool[
            self.boss_tool.index("FLOW_PREVIEW_CONSUMER_ID == Isolated.strConsumerId") :
            self.boss_tool.index("Save a clean revision to enable Start")
        ]
        self.assertIn("!Playback.Is_InFlight()", isolated_status)

    def test_world_change_and_late_lifecycle_cannot_resurrect_flow(self) -> None:
        for marker in (
            "VALTAN_PATTERN_FLOW_STATE::IDLE != m_Snapshot.eState",
            "m_Snapshot.iWorldInboundGeneration = CurrentWorldGeneration",
            "if (!m_Snapshot.Is_InFlight())",
            "VALTAN_PATTERN_FLOW_STATE::REQUEST_PENDING !=",
        ):
            self.assertIn(marker, self.service_source)
        lifecycle_guard = self.service_source.index("if (!m_Snapshot.Is_InFlight())")
        lifecycle_switch = self.service_source.index("switch (Lifecycle.eState)")
        self.assertLess(lifecycle_guard, lifecycle_switch)

    def test_stop_after_current_is_one_shot_with_its_own_timeout_clock(self) -> None:
        for marker in (
            "bStopAfterCurrentRequested",
            "m_iPendingStopStartedAtMilliseconds",
            "Stop After Current is already pending.",
            "no second control was sent for this Flow",
        ):
            self.assertIn(marker, self.service_header + self.service_source)
        self.assertIn("Playback.bStopAfterCurrentRequested", self.boss_tool)

    def test_main_thread_service_and_bounded_network_queues_are_connected(self) -> None:
        self.assertIn("CValtanPatternFlowService::Get().Update();", self.main_app)
        for marker in (
            "Send_ValtanPatternFlowStart(",
            "Send_ValtanPatternFlowStopAfterCurrent(",
            "Try_Consume_ValtanPatternFlowResult(",
            "Try_Consume_ValtanPatternFlowLifecycle(",
            "m_ValtanPatternFlowResults.clear();",
            "m_ValtanPatternFlowLifecycleEvents.clear();",
            "MAX_REVISION_CONTROL_QUEUE",
        ):
            self.assertIn(marker, self.network_header + self.network_source)

    def test_protocol_is_typed_bounded_and_covered_by_round_trip_harness(self) -> None:
        self.assertIn("NETWORK_PROTOCOL_VERSION = 39", self.packet_type)
        for marker in (
            "C2S_DEBUG_VALTAN_PATTERN_FLOW_START",
            "S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT",
            "C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT",
            "S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE",
        ):
            self.assertIn(marker, self.packet_type)
            self.assertIn(marker, self.packet_messages)
            self.assertIn(marker, self.protocol_harness)
        for marker in (
            "MAX_VALTAN_PATTERN_FLOW_SLOTS = 32u",
            "MIN_VALTAN_PATTERN_FLOW_INTER_STEP_PURSUIT_MS = 100u",
            "MAX_VALTAN_PATTERN_FLOW_INTER_STEP_PURSUIT_MS = 10000u",
            "Valtan Pattern Flow Leaves Duplicate Slot Rejection To Server",
        ):
            self.assertIn(marker, self.packet_messages + self.protocol_harness)

    def test_server_runs_one_ephemeral_ordered_suffix_without_client_slot_loop(self) -> None:
        for marker in (
            "Evaluate_ValtanPatternFlowStart(",
            "Evaluate_ValtanPatternFlowStopAfterCurrent(",
            "Resolve_ValtanPatternFlowSequence(",
            "VALTAN_PATTERN_FLOW_AUDITION_STATE",
        ):
            self.assertIn(marker, self.server_room_header + self.server_room)
        start_body = self.server_room[
            self.server_room.index("Evaluate_ValtanPatternFlowStart(") :
            self.server_room.index("Evaluate_ValtanPatternFlowStopAfterCurrent(")
        ]
        for marker in (
            "Build_PatternFlowStartRequestIdentity(request)",
            "BOSS_PATTERN_SEQUENCE_MODE::ORDERED_ONCE_THEN_IDLE",
            "for (std::size_t index = startSlotIndex;",
            "stagedFlow.Sequence.PatternIds.push_back(",
            "Reset_ValtanAuditionState(*boss, resetTick, resetStatus)",
            "m_ValtanPatternFlowAudition = std::move(stagedFlow)",
        ):
            self.assertIn(marker, start_body)
        self.assertLess(
            start_body.index("Reset_ValtanAuditionState(*boss, resetTick, resetStatus)"),
            start_body.index("m_ValtanPatternFlowAudition = std::move(stagedFlow)"),
        )
        self.assertIn("automaticSequenceOverride", self.server_brain)
        self.assertIn("Resolve_ValtanPatternFlowSequence(entity)", self.server_room)
        self.assertNotIn("PendingPatternIds", start_body)

    def test_server_duplicate_identity_stop_hold_and_release_rejection_are_locked(self) -> None:
        for marker in (
            "request.strFlowId == receipt->second.strFlowId",
            "request.iRoomFlowEpoch == receipt->second.iRoomFlowEpoch",
            "VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW",
            "VALTAN_PATTERN_FLOW_RESULT::REJECTED_RELEASE_BUILD",
        ):
            self.assertIn(marker, self.server_room)
        for marker in (
            "Run a saved slot-two suffix through duplicate ordered patterns with one reset, revive pause, preserved between-slot state, terminal hold, and correlated lifecycle",
            "Stop a running Boss Tool flow after its current occurrence and reject a stale flow epoch without starting the next slot",
            "Reject Boss Tool pattern-flow start and stop commands in a Release Server without staging room state",
        ):
            self.assertIn(marker, self.server_tests)
        for marker in (
            "PACKET_TYPE::C2S_DEBUG_VALTAN_PATTERN_FLOW_START",
            "PACKET_TYPE::C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT",
        ):
            self.assertIn(marker, self.server_app)

    def test_all_effects_overwrite_gotcha_preserves_current_strict_join(self) -> None:
        for marker in (
            "오래된 worktree의 PatternTree 전체 파일로 All Effects를 덮어쓰지 않는다",
            "partDamagePolicy",
            "counterProxy",
            "test_valtan_pattern_tree_contract",
            "test_effect_tool_valtan_all_effects_contract",
        ):
            self.assertIn(marker, self.gotchas)
        tree = PATTERN_TREE_H.read_text(encoding="utf-8") + PATTERN_TREE_CPP.read_text(
            encoding="utf-8"
        )
        self.assertIn("partDamagePolicy", tree)
        self.assertIn("counterProxy", tree)


if __name__ == "__main__":
    unittest.main()

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
TUNING_SERVICE_H = ROOT / "Client/Public/ValtanTuningCommandService.h"
TUNING_SERVICE_CPP = ROOT / "Client/Private/ValtanTuningCommandService.cpp"
BOSS_TOOL_CPP = ROOT / "Client/Private/BossTool.cpp"
BOSS_TOOL_H = ROOT / "Client/Public/BossTool.h"
NETWORK_H = ROOT / "Client/Public/NetworkManager.h"
NETWORK_CPP = ROOT / "Client/Private/NetworkManager.cpp"
MAIN_APP_CPP = ROOT / "Client/Private/MainApp.cpp"
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
TUNING_PIPELINE_PY = ROOT / "Tools/ValtanPipeline/valtan_tuning_pipeline.py"
GAMEPLAY_PUBLISHER_PS = (
    ROOT / "Tools/GameplayPipeline/Publish-GameplayBalance.ps1"
)
SERVER_ROOM_H = ROOT / "Server/Public/GameRoom.h"
SERVER_ROOM_CPP = ROOT / "Server/Private/GameRoom.cpp"
SERVER_GAMEPLAY_CATALOG_CPP = ROOT / "Server/Private/GameplayCatalog.cpp"
SERVER_BRAIN_H = ROOT / "Server/Public/ValtanBrain.h"
SERVER_BRAIN_CPP = ROOT / "Server/Private/ValtanBrain.cpp"
SERVER_APP_CPP = ROOT / "Server/Private/ServerApp.cpp"
SERVER_TESTS_CPP = ROOT / "Server/Private/ServerGameplayContractTests.cpp"

SCHEMA = "lostark.valtan-boss-audition-flows"
FLOW_ID = "flow.valtan.boss-tool.default"
MAX_NODES = 255
MAX_EDGES = 255
STABLE_ID = re.compile(r"[A-Za-z0-9_.-]{1,128}")



def make_document(pattern_ids: list[str]) -> dict:
    """Build a deterministic v2 chain without depending on the saved order."""
    nodes = [
        {
            "nodeId": f"{FLOW_ID}.slot.{ordinal:06d}",
            "patternId": pattern_id,
            "watchdogMs": 0,
        }
        for ordinal, pattern_id in enumerate(pattern_ids, start=1)
    ]
    edges = [
        {
            "edgeId": f"{FLOW_ID}.edge.{ordinal:06d}",
            "fromNodeId": nodes[ordinal - 1]["nodeId"],
            "outcome": "COMPLETED",
            "toNodeId": nodes[ordinal]["nodeId"],
            "pursuitMs": 1000,
        }
        for ordinal in range(1, len(nodes))
    ]
    return {
        "schema": SCHEMA,
        "formatVersion": 2,
        "flows": [{
            "flowId": FLOW_ID,
            "entryNodeId": nodes[0]["nodeId"] if nodes else f"{FLOW_ID}.slot.000001",
            "nextNodeOrdinal": len(nodes) + 1,
            "nextEdgeOrdinal": len(edges) + 1,
            "defaultPursuitMs": 1000,
            "maxTransitionsPerRun": 255,
            "nodes": nodes,
            "edges": edges,
        }],
    }


def validate_document(document: object, admitted_pattern_ids: list[str]) -> None:
    if not isinstance(document, dict) or set(document) != {
        "schema", "formatVersion", "flows"
    }:
        raise ValueError("root properties")
    if document["schema"] != SCHEMA or type(document["formatVersion"]) is not int:
        raise ValueError("header types")
    if document["formatVersion"] != 2:
        raise ValueError("version")
    flows = document["flows"]
    if not isinstance(flows, list) or len(flows) != 1:
        raise ValueError("flow count")
    flow = flows[0]
    if not isinstance(flow, dict) or set(flow) != {
        "flowId", "entryNodeId", "nextNodeOrdinal", "nextEdgeOrdinal",
        "defaultPursuitMs", "maxTransitionsPerRun", "nodes", "edges",
    }:
        raise ValueError("flow properties")
    if flow["flowId"] != FLOW_ID or STABLE_ID.fullmatch(flow["flowId"]) is None:
        raise ValueError("flow id")
    if type(flow["nextNodeOrdinal"]) is not int or not (
        1 <= flow["nextNodeOrdinal"] <= 1_000_000
    ):
        raise ValueError("next node ordinal")
    if type(flow["nextEdgeOrdinal"]) is not int or not (
        1 <= flow["nextEdgeOrdinal"] <= 1_000_000
    ):
        raise ValueError("next edge ordinal")
    if type(flow["defaultPursuitMs"]) is not int or not (
        100 <= flow["defaultPursuitMs"] <= 10_000
    ):
        raise ValueError("pursuit")
    if type(flow["maxTransitionsPerRun"]) is not int or not (
        1 <= flow["maxTransitionsPerRun"] <= 4096
    ):
        raise ValueError("transitions")
    nodes = flow["nodes"]
    edges = flow["edges"]
    if not isinstance(nodes, list) or not nodes or len(nodes) > MAX_NODES:
        raise ValueError("nodes")
    if not isinstance(edges, list) or len(edges) > MAX_EDGES:
        raise ValueError("edges")
    admitted = set(admitted_pattern_ids)
    if len(admitted) != len(admitted_pattern_ids) or not admitted:
        raise ValueError("inventory")
    node_ids: set[str] = set()
    node_ordinals: set[int] = set()
    nodes_by_id: dict[str, dict] = {}
    maximum_node_ordinal = 0
    entrance_count = 0
    for node in nodes:
        if not isinstance(node, dict) or set(node) != {
            "nodeId", "patternId", "watchdogMs"
        }:
            raise ValueError("node properties")
        node_id = node["nodeId"]
        pattern_id = node["patternId"]
        match = re.fullmatch(
            re.escape(FLOW_ID) + r"\.(?:slot|node)\.(\d{6})", node_id
        ) if isinstance(node_id, str) else None
        if (
            match is None
            or int(match.group(1)) == 0
            or int(match.group(1)) in node_ordinals
            or node_id in node_ids
        ):
            raise ValueError("node id")
        if (
            not isinstance(pattern_id, str)
            or STABLE_ID.fullmatch(pattern_id) is None
            or pattern_id not in admitted
        ):
            raise ValueError("pattern id")
        watchdog = node["watchdogMs"]
        if type(watchdog) is not int or (
            watchdog != 0 and not 1000 <= watchdog <= 300_000
        ):
            raise ValueError("watchdog")
        if pattern_id == "VALTAN_ENTRANCE_CINEMATIC":
            entrance_count += 1
            if node_id != flow["entryNodeId"]:
                raise ValueError("entry position")
        node_ids.add(node_id)
        node_ordinals.add(int(match.group(1)))
        nodes_by_id[node_id] = node
        maximum_node_ordinal = max(maximum_node_ordinal, int(match.group(1)))
    if flow["entryNodeId"] not in node_ids:
        raise ValueError("entry")
    if entrance_count > 1:
        raise ValueError("entry count")
    if flow["nextNodeOrdinal"] <= maximum_node_ordinal:
        raise ValueError("node reuse")

    edge_ids: set[str] = set()
    outgoing: dict[str, dict] = {}
    maximum_edge_ordinal = 0
    for edge in edges:
        if not isinstance(edge, dict) or set(edge) not in (
            {"edgeId", "fromNodeId", "outcome", "toNodeId", "pursuitMs"},
            {"edgeId", "fromNodeId", "outcome", "toNodeId", "pursuitMs", "maxTraversals"},
        ):
            raise ValueError("edge properties")
        edge_id = edge["edgeId"]
        match = re.fullmatch(
            re.escape(FLOW_ID) + r"\.edge\.(\d{6})", edge_id
        ) if isinstance(edge_id, str) else None
        if match is None or int(match.group(1)) == 0 or edge_id in edge_ids:
            raise ValueError("edge id")
        if edge["outcome"] != "COMPLETED":
            raise ValueError("outcome")
        if edge["fromNodeId"] not in node_ids or edge["toNodeId"] not in node_ids:
            raise ValueError("dangling")
        if edge["fromNodeId"] in outgoing:
            raise ValueError("deterministic")
        if type(edge["pursuitMs"]) is not int or not 100 <= edge["pursuitMs"] <= 10_000:
            raise ValueError("edge pursuit")
        if "maxTraversals" in edge and (
            type(edge["maxTraversals"]) is not int
            or not 1 <= edge["maxTraversals"] <= 255
        ):
            raise ValueError("traversals")
        if entrance_count and edge["toNodeId"] == flow["entryNodeId"]:
            raise ValueError("entry target")
        edge_ids.add(edge_id)
        outgoing[edge["fromNodeId"]] = edge
        maximum_edge_ordinal = max(maximum_edge_ordinal, int(match.group(1)))
    if flow["nextEdgeOrdinal"] <= maximum_edge_ordinal:
        raise ValueError("edge reuse")

    reachable = {flow["entryNodeId"]}
    current = flow["entryNodeId"]
    back_edge: dict | None = None
    while current in outgoing:
        edge = outgoing[current]
        if edge["toNodeId"] in reachable:
            back_edge = edge
            break
        reachable.add(edge["toNodeId"])
        current = edge["toNodeId"]
    if reachable != node_ids:
        raise ValueError("reachable")
    for edge in edges:
        is_back_edge = edge is back_edge
        if is_back_edge != ("maxTraversals" in edge):
            raise ValueError("bounded cycle")

    traversals: dict[str, int] = {}
    current = flow["entryNodeId"]
    transition_count = 0
    while current in outgoing:
        edge = outgoing[current]
        used = traversals.get(edge["edgeId"], 0)
        if "maxTraversals" in edge and used >= edge["maxTraversals"]:
            break
        if transition_count >= flow["maxTransitionsPerRun"]:
            raise ValueError("terminal")
        traversals[edge["edgeId"]] = used + 1
        transition_count += 1
        current = edge["toNodeId"]


class ValtanBossToolPatternFlowDocumentContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.header = DOCUMENT_H.read_text(encoding="utf-8")
        cls.source = DOCUMENT_CPP.read_text(encoding="utf-8")
        cls.service_header = SERVICE_H.read_text(encoding="utf-8")
        cls.service_source = SERVICE_CPP.read_text(encoding="utf-8")
        cls.tuning_service_header = TUNING_SERVICE_H.read_text(encoding="utf-8")
        cls.tuning_service_source = TUNING_SERVICE_CPP.read_text(encoding="utf-8")
        cls.boss_tool = BOSS_TOOL_CPP.read_text(encoding="utf-8")
        cls.network_header = NETWORK_H.read_text(encoding="utf-8")
        cls.network_source = NETWORK_CPP.read_text(encoding="utf-8")
        cls.main_app = MAIN_APP_CPP.read_text(encoding="utf-8")
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
        cls.tuning_pipeline = TUNING_PIPELINE_PY.read_text(encoding="utf-8")
        cls.gameplay_publisher = GAMEPLAY_PUBLISHER_PS.read_text(encoding="utf-8")
        cls.server_room_header = SERVER_ROOM_H.read_text(encoding="utf-8")
        cls.server_room = SERVER_ROOM_CPP.read_text(encoding="utf-8")
        cls.server_gameplay_catalog = SERVER_GAMEPLAY_CATALOG_CPP.read_text(
            encoding="utf-8"
        )
        cls.server_brain = (
            SERVER_BRAIN_H.read_text(encoding="utf-8")
            + SERVER_BRAIN_CPP.read_text(encoding="utf-8")
        )
        cls.server_app = SERVER_APP_CPP.read_text(encoding="utf-8")
        cls.server_tests = SERVER_TESTS_CPP.read_text(encoding="utf-8")
        cls.inventory = [row["patternId"] for row in cls.gameplay["patterns"]]
        cls.sample_pattern_ids = [
            "VALTAN_WHIRLWIND", "VALTAN_FOUR_SLASH", "VALTAN_FLOOR_WIPE_130",
            "VALTAN_SEQUENCE_FOUR", "VALTAN_COUNTER",
        ]

    def test_inline_sequence_projects_to_one_deterministic_in_memory_graph(self) -> None:
        sequence = self.gameplay["decisionModel"]["scriptedSequence"]
        self.assertEqual(
            {"sequenceId", "mode", "interStepPursuitMs", "patternIds"},
            set(sequence),
        )
        self.assertEqual("ORDERED_ONCE_THEN_IDLE", sequence["mode"])
        projected = make_document(sequence["patternIds"])
        projected["flows"][0]["defaultPursuitMs"] = sequence[
            "interStepPursuitMs"
        ]
        for edge in projected["flows"][0]["edges"]:
            edge["pursuitMs"] = sequence["interStepPursuitMs"]
        validate_document(projected, self.inventory)
        flow = projected["flows"][0]
        self.assertEqual(2, projected["formatVersion"])
        self.assertEqual(len(flow["nodes"]) - 1, len(flow["edges"]))
        self.assertEqual(flow["nodes"][0]["nodeId"], flow["entryNodeId"])
        maximum_node_ordinal = max(
            int(node["nodeId"].rsplit(".", 1)[1]) for node in flow["nodes"]
        )
        maximum_edge_ordinal = max(
            int(edge["edgeId"].rsplit(".", 1)[1]) for edge in flow["edges"]
        )
        self.assertGreater(flow["nextNodeOrdinal"], maximum_node_ordinal)
        self.assertGreater(flow["nextEdgeOrdinal"], maximum_edge_ordinal)
        self.assertEqual({0}, {node["watchdogMs"] for node in flow["nodes"]})
        self.assertEqual(
            sequence["patternIds"],
            [node["patternId"] for node in flow["nodes"]],
        )
        self.assertEqual(
            [f"{FLOW_ID}.slot.{index:06d}" for index in range(1, len(flow["nodes"]) + 1)],
            [node["nodeId"] for node in flow["nodes"]],
        )

    def test_flow_selects_a_subset_without_requiring_every_authored_pattern(self) -> None:
        seed = make_document(self.sample_pattern_ids)
        validate_document(seed, self.inventory)
        flow = seed["flows"][0]
        self.assertEqual(
            self.sample_pattern_ids,
            [node["patternId"] for node in flow["nodes"]],
        )
        self.assertEqual(len(flow["nodes"]) + 1, flow["nextNodeOrdinal"])
        self.assertEqual(len(flow["edges"]) + 1, flow["nextEdgeOrdinal"])
        round_trip = json.loads(json.dumps(seed, ensure_ascii=False))
        validate_document(round_trip, self.inventory)
        self.assertEqual(seed, round_trip)

    def test_entry_cinematic_is_optional_and_only_valid_at_entry_node(self) -> None:
        entry = "VALTAN_ENTRANCE_CINEMATIC"
        for pattern_ids in ([entry, "VALTAN_WHIRLWIND"], ["VALTAN_WHIRLWIND"]):
            with self.subTest(valid=pattern_ids):
                validate_document(make_document(pattern_ids), self.inventory)
        for pattern_ids in (["VALTAN_WHIRLWIND", entry], [entry, entry]):
            with self.subTest(invalid=pattern_ids), self.assertRaisesRegex(ValueError, "entry position"):
                validate_document(make_document(pattern_ids), self.inventory)

    def test_duplicate_patterns_are_valid_but_duplicate_nodes_are_not(self) -> None:
        duplicate_pattern = make_document(["VALTAN_WHIRLWIND", "VALTAN_WHIRLWIND"])
        nodes = duplicate_pattern["flows"][0]["nodes"]
        self.assertNotEqual(nodes[0]["nodeId"], nodes[1]["nodeId"])
        validate_document(duplicate_pattern, self.inventory)

        duplicate_node = copy.deepcopy(duplicate_pattern)
        duplicate_node["flows"][0]["nodes"][-1]["nodeId"] = (
            duplicate_node["flows"][0]["nodes"][0]["nodeId"]
        )
        with self.assertRaisesRegex(ValueError, "node id"):
            validate_document(duplicate_node, self.inventory)

    def test_dangling_nondeterministic_and_unreachable_edges_fail(self) -> None:
        seed = make_document(self.sample_pattern_ids)
        cases: list[tuple[str, dict]] = []
        dangling = copy.deepcopy(seed)
        dangling["flows"][0]["edges"][0]["toNodeId"] = f"{FLOW_ID}.node.999999"
        cases.append(("dangling", dangling))
        nondeterministic = copy.deepcopy(seed)
        flow = nondeterministic["flows"][0]
        flow["edges"].append({
            "edgeId": f"{FLOW_ID}.edge.{flow['nextEdgeOrdinal']:06d}",
            "fromNodeId": flow["nodes"][0]["nodeId"],
            "outcome": "COMPLETED",
            "toNodeId": flow["nodes"][-1]["nodeId"],
            "pursuitMs": 1000,
        })
        flow["nextEdgeOrdinal"] += 1
        cases.append(("deterministic", nondeterministic))
        unreachable = copy.deepcopy(seed)
        unreachable["flows"][0]["edges"].pop(1)
        cases.append(("reachable", unreachable))
        for reason, candidate in cases:
            with self.subTest(reason=reason), self.assertRaisesRegex(ValueError, reason):
                validate_document(candidate, self.inventory)

    def test_terminal_and_finite_back_edge_are_unambiguous(self) -> None:
        terminal = make_document(self.sample_pattern_ids)
        validate_document(terminal, self.inventory)
        flow = terminal["flows"][0]
        back_edge = {
            "edgeId": f"{FLOW_ID}.edge.{flow['nextEdgeOrdinal']:06d}",
            "fromNodeId": flow["nodes"][-1]["nodeId"],
            "outcome": "COMPLETED",
            "toNodeId": flow["entryNodeId"],
            "pursuitMs": 1000,
        }
        unbounded = copy.deepcopy(terminal)
        unbounded["flows"][0]["edges"].append(copy.deepcopy(back_edge))
        unbounded["flows"][0]["nextEdgeOrdinal"] += 1
        with self.assertRaisesRegex(ValueError, "bounded cycle"):
            validate_document(unbounded, self.inventory)

        bounded = copy.deepcopy(unbounded)
        bounded["flows"][0]["edges"][-1]["maxTraversals"] = 2
        validate_document(bounded, self.inventory)
        bounded["flows"][0]["maxTransitionsPerRun"] = 10
        with self.assertRaisesRegex(ValueError, "terminal"):
            validate_document(bounded, self.inventory)

        forward_cap = copy.deepcopy(terminal)
        forward_cap["flows"][0]["edges"][0]["maxTraversals"] = 2
        with self.assertRaisesRegex(ValueError, "bounded cycle"):
            validate_document(forward_cap, self.inventory)

    def test_single_and_maximum_node_documents_are_valid_but_empty_is_not(self) -> None:
        for count in (1, 33, MAX_NODES):
            with self.subTest(node_count=count):
                document = make_document(["VALTAN_WHIRLWIND"] * count)
                document["flows"][0]["maxTransitionsPerRun"] = max(255, count - 1)
                validate_document(document, self.inventory)
                self.assertEqual(count, len(document["flows"][0]["nodes"]))
        with self.assertRaisesRegex(ValueError, "nodes"):
            validate_document(make_document([]), self.inventory)

    def test_watchdog_zero_or_bounded_positive_only(self) -> None:
        for watchdog in (0, 1000, 300_000):
            with self.subTest(watchdog=watchdog):
                document = make_document(["VALTAN_WHIRLWIND"])
                document["flows"][0]["nodes"][0]["watchdogMs"] = watchdog
                validate_document(document, self.inventory)
        for watchdog in (-1, 1, 999, 300_001, 1000.0):
            with self.subTest(watchdog=watchdog), self.assertRaisesRegex(ValueError, "watchdog"):
                document = make_document(["VALTAN_WHIRLWIND"])
                document["flows"][0]["nodes"][0]["watchdogMs"] = watchdog
                validate_document(document, self.inventory)

    def test_node_and_edge_ordinal_exhaustion_is_monotonic(self) -> None:
        document = make_document(["VALTAN_WHIRLWIND"])
        flow = document["flows"][0]
        flow["nodes"][0]["nodeId"] = f"{FLOW_ID}.slot.999999"
        flow["entryNodeId"] = flow["nodes"][0]["nodeId"]
        flow["nextNodeOrdinal"] = 1_000_000
        validate_document(document, self.inventory)

        for ordinal, reason in ((999_999, "node reuse"), (1_000_001, "next node ordinal")):
            with self.subTest(next_node_ordinal=ordinal):
                flow["nextNodeOrdinal"] = ordinal
                with self.assertRaisesRegex(ValueError, reason):
                    validate_document(document, self.inventory)

        duplicate_ordinal = make_document(
            ["VALTAN_WHIRLWIND", "VALTAN_FOUR_SLASH"]
        )
        duplicate_flow = duplicate_ordinal["flows"][0]
        duplicate_flow["nodes"][1]["nodeId"] = f"{FLOW_ID}.node.000001"
        duplicate_flow["edges"][0]["toNodeId"] = duplicate_flow["nodes"][1]["nodeId"]
        with self.assertRaisesRegex(ValueError, "node id"):
            validate_document(duplicate_ordinal, self.inventory)

    def test_wrong_version_unknown_property_unknown_pattern_and_overflow_fail(self) -> None:
        mutations = []
        seed = make_document(self.sample_pattern_ids)
        wrong_version = copy.deepcopy(seed)
        wrong_version["formatVersion"] = 1
        mutations.append(("version", wrong_version))
        unknown_property = copy.deepcopy(seed)
        unknown_property["flows"][0]["nodes"][0]["index"] = 0
        mutations.append(("node properties", unknown_property))
        unknown_pattern = copy.deepcopy(seed)
        unknown_pattern["flows"][0]["nodes"][0]["patternId"] = "UNKNOWN"
        mutations.append(("pattern id", unknown_pattern))
        overflow = make_document(["VALTAN_WHIRLWIND"] * (MAX_NODES + 1))
        mutations.append(("nodes", overflow))
        float_integer = copy.deepcopy(seed)
        float_integer["formatVersion"] = 2.0
        mutations.append(("header types", float_integer))

        for reason, candidate in mutations:
            with self.subTest(reason=reason):
                with self.assertRaisesRegex(ValueError, reason):
                    validate_document(candidate, self.inventory)

    def test_cpp_adapter_stages_canonical_sequence_without_a_second_file(self) -> None:
        for marker in (
            "Has_ExactProperties(root, { \"schema\", \"formatVersion\", \"flows\" })",
            "Try_ParseUnsignedInteger",
            "Try_ParseOrdinal",
            "MAX_NODES * 4u + MAX_EDGES * 7u",
            "admitted.contains(node.strPatternId)",
            "Only a cycle-closing back-edge may own maxTraversals",
            "cannot reach its terminal within maxTransitionsPerRun",
            "outDocument = std::move(staged)",
            "m_Baseline = staged",
            "m_Draft = std::move(staged)",
            "CValtanPatternFlowDocument::Load_CanonicalSequence(",
            '"ORDERED_ONCE_THEN_IDLE" != mode',
            "for (const std::string& patternId : patternIds)",
            "Loaded the canonical Valtan.gameplay.json scriptedSequence revision",
        ):
            self.assertIn(marker, self.source)
        self.assertNotIn("ValtanPatternTree", self.header + self.source)
        self.assertIn("std::unordered_map<std::string, const VALTAN_PATTERN_FLOW_NODE*>", self.source)
        self.assertNotIn("patternIds.insert", self.source)
        for retired_disk_owner in (
            "Resolve_Path(",
            "CValtanPatternFlowDocument::Load(",
            "CValtanPatternFlowDocument::Reload(",
            "CValtanPatternFlowDocument::Save(",
            "SCOPED_VALTAN_PATTERN_FLOW_WRITE_LOCK",
            "Read_Bytes(",
            "Write_Bytes(",
        ):
            self.assertNotIn(retired_disk_owner, self.header + self.source)

    def test_adapter_revision_is_content_derived_and_never_a_disk_cas_owner(self) -> None:
        load = self.source[
            self.source.index("CValtanPatternFlowDocument::Load_CanonicalSequence(") :
            self.source.index("CValtanPatternFlowDocument::Verify_SourceRevision(")
        ]
        for marker in (
            "identity << sequenceId",
            "for (const std::string& patternId : patternIds)",
            "Build_Revision(identity.str(), revision)",
            "m_Baseline = staged",
            "m_Draft = std::move(staged)",
            "m_strSourceRevision = std::move(revision)",
        ):
            self.assertIn(marker, load)
        for disk_marker in (
            "ifstream", "ofstream", "MoveFile", "CopyFile", "LockFile",
            "m_Path", "temporary", "backup",
        ):
            self.assertNotIn(disk_marker, load)

    def test_public_api_keeps_stable_slot_mutation_and_safe_read_view(self) -> None:
        for marker in (
            "Add_Slot(",
            "Move_Slot(",
            "Remove_Slot(",
            "Set_InterStepPursuitMs(",
            "Set_NodeWatchdogMs(",
            "Set_MaxTransitionsPerRun(",
            "Has_LegacyLinearProjection(",
            "Is_Ready()",
            "Is_Dirty()",
            "Has_ExternalConflict()",
            "Get_Draft()",
            "Get_DefaultFlow()",
            "Get_SourceRevision()",
            "Compute_SourceRevision(",
            "Verify_SourceRevision(",
            "Load_CanonicalSequence(",
        ):
            self.assertIn(marker, self.header)
        self.assertIn(
            "const VALTAN_PATTERN_FLOW_DEFINITION* Get_DefaultFlow() const noexcept",
            self.header,
        )
        for retired_disk_api in ("Resolve_Path(", "bool Load(", "bool Reload(", "bool Save("):
            self.assertNotIn(retired_disk_api, self.header)

    def test_new_pattern_adds_at_pattern_one_and_preserves_optional_entry(self) -> None:
        add_body = self.source[
            self.source.index("bool_t Client::CValtanPatternFlowDocument::Add_Slot(") :
            self.source.index("bool_t Client::CValtanPatternFlowDocument::Move_Slot(")
        ]
        self.assertIn("OPTIONAL_ENTRY_PATTERN_ID == patternId", add_body)
        self.assertIn('Build_OrdinalId(\n\t\tflow.strFlowId, "node"', add_body)
        self.assertIn("orderedNodeIds.insert(orderedNodeIds.begin(), nodeId)", add_body)
        self.assertIn("OPTIONAL_ENTRY_PATTERN_ID == flow.Slots.front().strPatternId", add_body)
        self.assertIn("std::next(orderedNodeIds.begin())", add_body)
        self.assertIn("orderedNodeIds.insert(insertAt, nodeId)", add_body)
        self.assertIn("Rebuild_LinearFlow(flow, orderedNodeIds, outStatus)", add_body)
        self.assertLess(add_body.index("Validate(staged"), add_body.index("m_Draft = std::move(staged)"))
        self.assertNotIn("27-pattern", self.boss_tool)
        self.assertIn("m_AuditionInventory.Get_PatternCount()", self.boss_tool)

    def test_move_stages_validates_and_preserves_the_previous_draft_on_failure(self) -> None:
        move_body = self.source[
            self.source.index("bool_t Client::CValtanPatternFlowDocument::Move_Slot(") :
            self.source.index("bool_t Client::CValtanPatternFlowDocument::Remove_Slot(")
        ]
        self.assertIn("const std::vector<std::string>& admittedPatternIds", move_body)
        self.assertIn("VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT staged = m_Draft", move_body)
        self.assertIn("Validate(staged, admittedPatternIds, outStatus)", move_body)
        self.assertLess(
            move_body.index("Validate(staged, admittedPatternIds, outStatus)"),
            move_body.index("m_Draft = std::move(staged)"),
        )

    def test_project_and_filter_register_each_new_source_once(self) -> None:
        ET.parse(PROJECT)
        ET.parse(FILTERS)
        retired_flow_name = "ValtanBoss" + "AuditionFlows.json"
        for source in (self.project, self.filters):
            self.assertEqual(1, source.count("ValtanPatternFlowDocument.h"))
            self.assertEqual(1, source.count("ValtanPatternFlowDocument.cpp"))
            self.assertEqual(1, source.count("ValtanPatternFlowService.h"))
            self.assertEqual(1, source.count("ValtanPatternFlowService.cpp"))
            self.assertNotIn(retired_flow_name, source)

    def test_flow_tab_keeps_verification_and_reuses_single_pattern_submit(self) -> None:
        for marker in (
            'ImGui::BeginTabItem("Boss Verification")',
            'ImGui::BeginTabItem("Pattern Flow", nullptr, FlowFlags)',
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
        self.assertIn("void Client::CBossTool::Open_PatternFlow()", self.boss_tool)
        self.assertIn("m_bSelectPatternFlowTab = true", self.boss_tool)

    def test_flow_selection_and_hover_reuse_the_shared_pattern_identity(self) -> None:
        for start, end in (
            ("Render_FlowSlotList()", "Render_AddPatternPopup()"),
            ("Render_AddPatternPopup()", "Render_FlowSelectedSlot()"),
            ("Render_FlowSelectedSlot()", "Render_LiveSummary()"),
        ):
            with self.subTest(section=start):
                begin = self.boss_tool.index("void Client::CBossTool::" + start)
                finish = self.boss_tool.index("void Client::CBossTool::" + end, begin)
                section = self.boss_tool[begin:finish]
                self.assertIn("Find_AuditionPattern(", section)
                self.assertIn("pPattern->strDisplayName", section)
                self.assertIn(
                    "CValtanPatternTree::Build_PatternIdentitySummary(*pPattern)",
                    section,
                )
                self.assertIn("ImGui::IsItemHovered()", section)

    def test_restart_reloads_first_canonical_slot_without_a_flow_publisher(self) -> None:
        start_body = self.boss_tool[
            self.boss_tool.index("bool_t Client::CBossTool::Start_Flow") :
            self.boss_tool.index("bool_t Client::CBossTool::Request_RevivePlayer")
        ]
        self.assertIn("CValtanPatternFlowDocument::Validate(", start_body)
        self.assertIn("m_FlowDocument.Verify_SourceRevision", start_body)
        self.assertIn("pFlow->Slots.front().strSlotId", start_body)
        self.assertIn("CValtanPatternFlowService::Get().Start(", start_body)
        verify_body = self.source[
            self.source.index("Verify_SourceRevision(") :
            self.source.index("Add_Slot(", self.source.index("Verify_SourceRevision("))
        ]
        self.assertIn("m_strSourceRevision.empty()", verify_body)
        self.assertIn("pinned to the staged graph revision", verify_body)
        self.assertNotIn("m_Baseline =", verify_body)
        self.assertNotIn("m_Draft =", verify_body)

        tuning_service = self.tuning_service_header + self.tuning_service_source
        for retired_token in (
            "Publish_SavedPatternFlow",
            "Is_SavedPatternFlowServerActive",
            "PUBLISH_SAVED_FLOW",
            "PublishSavedFlow",
            "Start_PublishProcess",
            "Poll_PublishProcess",
            "strFlowRevision",
        ):
            self.assertNotIn(retired_token, tuning_service)
        self.assertIn("ApplyCandidate(", self.tuning_service_header)
        self.assertIn(
            "CValtanTuningCommandService::ApplyCandidate(",
            self.tuning_service_source,
        )

    def test_tuning_service_has_no_retired_saved_flow_publisher(self) -> None:
        tuning_service = self.tuning_service_header + self.tuning_service_source
        for retired_token in (
            "Publish_SavedPatternFlow",
            "Is_SavedPatternFlowServerActive",
            "PUBLISH_SAVED_FLOW",
            "PublishSavedFlow",
            "Start_PublishProcess",
            "Poll_PublishProcess",
            "strFlowRevision",
        ):
            self.assertNotIn(retired_token, tuning_service)
        self.assertIn("ApplyCandidate(", self.tuning_service_header)
        self.assertIn(
            "CValtanTuningCommandService::ApplyCandidate(",
            self.tuning_service_source,
        )

    def test_load_and_save_share_gameplay_canonical_owner_until_restart(self) -> None:
        reload_body = self.boss_tool[
            self.boss_tool.index("bool_t Client::CBossTool::Reload_FlowDocument()"):
            self.boss_tool.index("bool_t Client::CBossTool::Save_FlowDocument()")
        ]
        self.assertLess(
            reload_body.index("Reload_Graph()"),
            reload_body.index("pFlow->strEntryNodeId"),
        )
        self.assertIn("!pFlow->Nodes.empty()", reload_body)
        self.assertIn("Server playback was not changed", reload_body)
        self.assertNotIn("Has_PendingCommand()", reload_body)
        self.assertNotIn("Start_Flow(", reload_body)
        self.assertNotIn("Set_ServerArenaPreset(", reload_body)
        save_body = self.boss_tool[
            self.boss_tool.index("bool_t Client::CBossTool::Save_FlowDocument()"):
            self.boss_tool.index("void Client::CBossTool::Synchronize_LiveSelection()")
        ]
        self.assertLess(
            save_body.index("Set_ValtanScriptedSequenceDraft("),
            save_body.index("Save_ValtanCanonicalProduct("),
        )
        self.assertLess(
            save_body.index("Save_ValtanCanonicalProduct("),
            save_body.index("Save_ValtanProduct("),
        )
        self.assertIn("Valtan.gameplay.json", save_body)
        self.assertIn("const bool_t bReloaded = Reload_Graph()", save_body)
        self.assertIn("The physical Save succeeded", save_body)
        self.assertIn("Product candidate publication/activation preparation failed", save_body)
        self.assertIn("exact saved Product revision is Server-active", save_body)
        self.assertTrue(save_body.rstrip().endswith("return true;\n}"))
        self.assertNotIn("Start_Flow(", save_body)
        self.assertNotIn("ApplyCandidate(", save_body)
        self.assertNotIn("Apply_SavedFlow", self.boss_tool)
        self.assertNotIn("Render_FlowPublicationStatus", self.boss_tool)
        self.assertIn("flow.Nodes.empty()", self.source)

    def test_failed_canonical_reload_keeps_display_rows_but_revokes_mutation(self) -> None:
        header = (ROOT / "Client/Public/BossTool.h").read_text(encoding="utf-8")
        reload_body = self.boss_tool[
            self.boss_tool.index("bool_t Client::CBossTool::Reload_Graph()") :
            self.boss_tool.index("void Client::CBossTool::Refresh_PresentationFreshness(")
        ]
        self.assertIn("m_bGraphMutationAdmitted", header)
        self.assertIn("m_bGraphReady", header)
        self.assertLess(
            reload_body.index("m_bGraphMutationAdmitted = false"),
            reload_body.index("CValtanPatternTree::Load_WhileAdmitted("),
        )
        self.assertIn("STALE_PRESERVED", reload_body)
        self.assertIn("previous rows are display-only", reload_body)
        self.assertLess(
            reload_body.index("m_bGraphReady = true"),
            reload_body.index("m_bGraphMutationAdmitted = true"),
        )

        gate = self.boss_tool[
            self.boss_tool.index("bool_t Client::CBossTool::Can_MutateCanonicalGraph(") :
            self.boss_tool.index("void Client::CBossTool::Refresh_PresentationFreshness(")
        ]
        self.assertIn("m_bGraphMutationAdmitted", gate)
        self.assertIn("display-only", gate)
        for start, end, marker in (
            ("bool_t Client::CBossTool::Can_Play_ServerPattern(",
             "bool_t Client::CBossTool::Get_ServerPatternOptions(",
             "Can_MutateCanonicalGraph"),
            ("bool_t Client::CBossTool::Set_ServerArenaPreset(",
             "bool_t Client::CBossTool::Get_ServerArenaActiveState(",
             "Can_MutateCanonicalGraph"),
            ("bool_t Client::CBossTool::Start_Flow(",
             "bool_t Client::CBossTool::Request_RevivePlayer(",
             "Acquire_ServerPlaybackAdmission"),
            ("bool_t Client::CBossTool::Save_FlowDocument()",
             "void Client::CBossTool::Synchronize_LiveSelection()",
             "Can_MutateCanonicalGraph"),
            ("void Client::CBossTool::Render_NextPatternPicker()",
             "void Client::CBossTool::Render_FlowSlotList()",
             "Queue_NextServerPattern"),
            ("void Client::CBossTool::Render_FlowSelectedSlot()",
             "void Client::CBossTool::Render_ActionBar()",
             "Can_MutateCanonicalGraph"),
        ):
            body = self.boss_tool[
                self.boss_tool.index(start) : self.boss_tool.index(end)
            ]
            self.assertIn(marker, body)

    def test_flow_commands_have_explicit_non_composed_ui_contracts(self) -> None:
        for label in (
            'ImGui::Button("Load Flow")',
            'ImGui::Button("Save Flow")',
            'ImGui::Button("Discard Changes...")',
            'ImGui::Button("Discard Changes##ConfirmFlowDiscard")',
            'ImGui::Button("Restart Flow (Fresh Arena)")',
            'ImGui::Button("Restart Saved Pattern (Fresh Arena)")',
        ):
            self.assertIn(label, self.boss_tool)
        self.assertNotIn("Save & Apply Flow", self.boss_tool)
        self.assertNotIn("Publish_SavedPatternFlow", self.boss_tool)
        self.assertNotIn("Apply_SavedFlow", self.boss_tool)
        flow_tab = self.boss_tool[
            self.boss_tool.index("void Client::CBossTool::Render_PatternFlowTab()"):
            self.boss_tool.index("void Client::CBossTool::Render_PatternList()")
        ]
        self.assertIn("Has_LegacyLinearProjection(", flow_tab)
        self.assertIn("Reload_FlowDocument()", flow_tab)
        for marker in (
            "restore the authoritative walls, floors, props, collision, Nav, and combat objects",
            "then start Pattern 01 from a fresh arena",
        ):
            self.assertIn(marker, flow_tab)
        restart_flow = self.boss_tool[
            self.boss_tool.index("bool_t Client::CBossTool::Restart_SavedFlow()"):
            self.boss_tool.index("bool_t Client::CBossTool::Request_RevivePlayer(")
        ]
        self.assertIn(
            "Try_GetLatestGameplaySourceServerActiveRevision(", restart_flow
        )
        self.assertLess(
            restart_flow.index("Try_GetLatestGameplaySourceServerActiveRevision("),
            restart_flow.index("Reload_FlowDocument()"),
        )
        self.assertLess(
            restart_flow.index("Reload_FlowDocument()"),
            restart_flow.index("Start_Flow("),
        )
        self.assertIn("&SavedCandidateRevision", restart_flow)
        start_at_slot = self.boss_tool[
            self.boss_tool.index("bool_t Client::CBossTool::Start_FlowAtSlot(") :
            self.boss_tool.index("bool_t Client::CBossTool::Restart_SavedFlow()")
        ]
        self.assertLess(
            start_at_slot.index("ExpectedRevision != *pRequiredDefinitionRevision"),
            start_at_slot.index("CValtanPatternFlowService::Get().Start("),
        )
        self.assertIn("no Flow command was submitted", start_at_slot)
        restart_body = self.boss_tool[
            self.boss_tool.index("bool_t Client::CBossTool::Restart_SelectedPattern()"):
            self.boss_tool.index("bool_t Client::CBossTool::Can_Play_ServerPattern(")
        ]
        self.assertIn("Restart_ActivePattern(", restart_body)

        action_bar = self.boss_tool[
            self.boss_tool.index("void Client::CBossTool::Render_ActionBar()"):
            self.boss_tool.index("void Client::CBossTool::Normalize_CurrentFlowSelection()")
        ]
        self.assertIn("Restart_SavedSinglePatternFreshArena()", action_bar)
        self.assertNotIn("Restart_SelectedPattern()", action_bar)
        single_restart = self.boss_tool[
            self.boss_tool.index(
                "bool_t Client::CBossTool::Restart_SavedSinglePatternFreshArena()"
            ):
            self.boss_tool.index("bool_t Client::CBossTool::Restart_ServerPattern(")
        ]
        for marker in (
            "m_FlowDocument.Get_SavedDefaultFlow()",
            "1u != iSavedSlotCount",
            "exactly one saved scriptedSequence slot",
            "Restart_SavedFlow(true)",
        ):
            self.assertIn(marker, single_restart)
        for marker in (
            "bRequireSingleSavedPattern",
            "Pending.Request.Slots.size() != 1u",
            "Pending.Request.strFlowRevision",
            "exact saved one-slot scriptedSequence request",
        ):
            self.assertIn(marker, restart_flow)

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
            self.boss_tool.index("Save a clean Flow to enable Restart")
        ]
        self.assertIn("!Playback.Is_InFlight()", isolated_status)

    def test_v2_graph_editor_uses_strict_document_mutations_and_one_save_owner(self) -> None:
        for method in (
            "Insert_Node_After",
            "Remove_Node",
            "Set_EntryNode",
            "Connect_CompletedEdge",
            "Remove_Edge",
            "Set_EdgePursuitMs",
            "Set_EdgeMaxTraversals",
        ):
            self.assertIn(method, self.header)
            self.assertIn(
                f"CValtanPatternFlowDocument::{method}", self.source
            )
        self.assertGreaterEqual(
            self.source.count(
                "VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT staged = m_Draft"
            ),
            10,
        )
        self.assertGreaterEqual(
            self.source.count("if (!Validate(staged, admittedPatternIds, outStatus))"),
            10,
        )
        for marker in (
            'ImGui::Checkbox("Pattern Route Editor"',
            "Render_FlowGraphEditor",
            'ImGui::Button("Add From Pattern Slot...")',
            'ImGui::Button("Make First Pattern")',
            'ImGui::Button("Discard Selected")',
            'ImGui::Button("Choose Next Pattern")',
            'ImGui::Button("Set Next to Selected")',
            'ImGui::Button("Remove Next Pattern")',
            '"Maximum Pattern changes"',
            "m_FlowDocument.Connect_CompletedEdge",
            "m_FlowDocument.Set_EdgeMaxTraversals",
            "CValtanPatternFlowDocument::Has_LegacyLinearProjection(*pFlow)",
        ):
            self.assertIn(marker, self.boss_tool)
        selected_slot = self.boss_tool[
            self.boss_tool.index(
                "void Client::CBossTool::Render_FlowSelectedSlot()"
            ) : self.boss_tool.index(
                "void Client::CBossTool::Render_LiveSummary()"
            )
        ]
        for marker in (
            'ImGui::Button("Up##FlowGraphOrder")',
            'ImGui::Button("Down##FlowGraphOrder")',
            "CValtanPatternFlowDocument::Has_LegacyLinearProjection(*pFlow)",
            "m_FlowDocument.Move_Slot(",
            "m_strSelectedFlowEdgeId.clear()",
        ):
            self.assertIn(marker, selected_slot)
        graph_editor = self.boss_tool[
            self.boss_tool.index(
                "void Client::CBossTool::Render_FlowGraphEditor()"
            ) : self.boss_tool.index(
                "bool_t Client::CBossTool::Render_AddPatternNodePopup()"
            )
        ]
        self.assertIn("if (Render_AddPatternNodePopup())", graph_editor)
        self.assertIn("return;", graph_editor)
        for marker in (
            '"##bossPatternRouteTable"',
            '"Pattern"',
            '"Next Pattern"',
            '"Wait"',
            'm_strSelectedFlowSlotId = Node.strNodeId',
            'm_strSelectedFlowEdgeId = Outgoing->strEdgeId',
            '"Ready for Server playback"',
        ):
            self.assertIn(marker, graph_editor)
        for removed_jargon in (
            "LINEAR SERVER PROJECTION READY",
            "GRAPH AUTHORING DRAFT",
            "DOWN COMPLETED",
            "LOOP COMPLETED",
            "ImVec2(0.f, 58.f)",
        ):
            self.assertNotIn(removed_jargon, graph_editor)
        add_popup = self.boss_tool[
            self.boss_tool.index(
                "bool_t Client::CBossTool::Render_AddPatternNodePopup()"
            ) : self.boss_tool.index(
                "void Client::CBossTool::Render_FlowSlotList()"
            )
        ]
        self.assertIn("bDocumentMutated = true", add_popup)
        self.assertIn("return bDocumentMutated", add_popup)
        save = self.boss_tool[
            self.boss_tool.index("bool_t Client::CBossTool::Save_FlowDocument()") :
            self.boss_tool.index("void Client::CBossTool::Synchronize_LiveSelection()")
        ]
        self.assertIn("m_pBalanceTool->Set_ValtanScriptedSequenceDraft", save)
        self.assertIn("m_pBalanceTool->Save_ValtanCanonicalProduct", save)
        self.assertIn("m_pBalanceTool->Save_ValtanProduct", save)
        self.assertNotIn("m_FlowDocument.Save", save)
        self.assertNotIn("CNetworkManager", self.source)

    def test_world_change_and_late_lifecycle_cannot_resurrect_flow(self) -> None:
        for marker in (
            "m_PendingStart.iWorldInboundGeneration",
            "m_Snapshot.iWorldInboundGeneration",
            "if (!m_Snapshot.Is_InFlight())",
            "Matches_Request(m_CurrentRequest, Lifecycle)",
            "Matches_OrderedSlot(m_CurrentRequest, Lifecycle)",
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
            "MAX_VALTAN_PATTERN_FLOW_SLOTS = 255u",
            "MIN_VALTAN_PATTERN_FLOW_INTER_STEP_PURSUIT_MS = 100u",
            "MAX_VALTAN_PATTERN_FLOW_INTER_STEP_PURSUIT_MS = 10000u",
            "Valtan Pattern Flow Leaves Duplicate Slot Rejection To Server",
            "Valtan Pattern Flow 33 Slot Expansion Round Trip",
            "Valtan Pattern Flow 255 Slot Round Trip",
            "Reject 256 Slot Valtan Pattern Flow Start",
            "Reject Valtan Pattern Flow Exceeding Frame Budget",
        ):
            self.assertIn(marker, self.packet_messages + self.protocol_harness)
        for marker in (
            "SCRIPTED_SEQUENCE_MAX_PATTERNS = 255",
            "len(rows) > SCRIPTED_SEQUENCE_MAX_PATTERNS",
            "scriptedSequence.patternIds count is invalid",
        ):
            self.assertIn(marker, self.tuning_pipeline)
        for marker in (
            "$maximumValtanPatternFlowSlots = 255",
            "Count -gt $maximumValtanPatternFlowSlots",
        ):
            self.assertIn(marker, self.gameplay_publisher)
        self.assertIn(
            "stepCount > LostArk::Shared::MAX_VALTAN_PATTERN_FLOW_SLOTS",
            self.server_gameplay_catalog,
        )

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

    def test_boss_verification_separates_inventory_from_saved_current_order(self) -> None:
        boss_header = BOSS_TOOL_H.read_text(encoding="utf-8")
        verification = self.boss_tool[
            self.boss_tool.index("void Client::CBossTool::Render_BossVerificationTab()") :
            self.boss_tool.index("void Client::CBossTool::Render_PatternFlowTab()")
        ]
        for marker in (
            'ImGui::BeginTabItem("All Patterns")',
            'ImGui::BeginTabItem("Current Patterns")',
            "Render_PatternList()",
            "Render_CurrentPatternList()",
        ):
            self.assertIn(marker, verification)
        self.assertIn("Render_CurrentPatternList", boss_header)
        self.assertIn("m_strSelectedCurrentFlowSlotId", boss_header)

        baseline_accessor = self.source[
            self.source.index(
                "CValtanPatternFlowDocument::Get_SavedDefaultFlow() const noexcept"
            ) :
        ]
        self.assertIn("m_Baseline.Flows", baseline_accessor)
        self.assertNotIn("m_Draft.Flows", baseline_accessor)
        current_list = self.boss_tool[
            self.boss_tool.index("void Client::CBossTool::Render_CurrentPatternList()") :
            self.boss_tool.index("void Client::CBossTool::Render_PatternList()")
        ]
        for marker in (
            "m_FlowDocument.Get_SavedDefaultFlow()",
            "m_FlowDocument.Get_SourceRevision()",
            "Playback.strFlowRevision",
            "Playback.strCurrentSlotId",
            "Slot.strSlotId",
            "Unsaved Pattern Flow edits are hidden here",
            "[PLAYING]",
        ):
            self.assertIn(marker, current_list)

    def test_canonical_save_refreshes_adapter_but_active_run_does_not_lock_authoring(self) -> None:
        save = self.boss_tool[
            self.boss_tool.index("bool_t Client::CBossTool::Save_FlowDocument()") :
            self.boss_tool.index("void Client::CBossTool::Synchronize_LiveSelection()")
        ]
        self.assertLess(
            save.index("m_pBalanceTool->Set_ValtanScriptedSequenceDraft("),
            save.index("m_pBalanceTool->Save_ValtanCanonicalProduct("),
        )
        self.assertLess(
            save.index("m_pBalanceTool->Save_ValtanCanonicalProduct("),
            save.index("m_pBalanceTool->Save_ValtanProduct("),
        )
        self.assertLess(
            save.index("m_pBalanceTool->Save_ValtanProduct("),
            save.index("Normalize_CurrentFlowSelection()"),
        )
        self.assertIn("COMMIT_SUCCEEDED_REOPEN_FAILED:", save)
        self.assertIn("const bool_t bCanonicalReopened", save)
        self.assertIn("bCanonicalReopened &&", save)
        self.assertIn("Product candidate publication was skipped", save)
        self.assertIn("The running Flow was not changed", save)
        self.assertIn("exact saved Product revision is Server-active", save)

        for start, end in (
            ("void Client::CBossTool::Render_FlowGraphEditor()",
             "bool_t Client::CBossTool::Render_AddPatternNodePopup()"),
            ("bool_t Client::CBossTool::Render_AddPatternNodePopup()",
             "void Client::CBossTool::Render_FlowSlotList()"),
            ("void Client::CBossTool::Render_FlowSlotList()",
             "void Client::CBossTool::Render_AddPatternPopup()"),
            ("void Client::CBossTool::Render_AddPatternPopup()",
             "void Client::CBossTool::Render_FlowSelectedSlot()"),
        ):
            body = self.boss_tool[
                self.boss_tool.index(start) : self.boss_tool.index(end)
            ]
            self.assertIn("Has_PendingStart()", body)
            self.assertNotIn(
                "CValtanPatternFlowService::Get().Has_PlaybackOwnership()", body
            )

        selected_start = self.boss_tool.index(
            "void Client::CBossTool::Render_FlowSelectedSlot()"
        )
        selected_prefix = self.boss_tool[
            selected_start : self.boss_tool.index(
                "const std::vector<std::string> AdmittedIds = Build_AdmittedPatternIds();",
                selected_start,
            )
        ]
        self.assertIn("Has_PendingStart()", selected_prefix)
        self.assertNotIn("FlowService.Has_PlaybackOwnership()", selected_prefix)
        for label in (
            'ImGui::Button("Up##FlowGraphOrder")',
            'ImGui::Button("Down##FlowGraphOrder")',
            'ImGui::Button("Discard Selected")',
        ):
            self.assertIn(label, self.boss_tool)


if __name__ == "__main__":
    unittest.main()

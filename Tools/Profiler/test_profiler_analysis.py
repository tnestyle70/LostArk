#!/usr/bin/env python3
"""Focused regression tests for the four-client and Server room gates."""

from __future__ import annotations

import pathlib
import sys
import tempfile
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

import analyze_profiler_captures as client_analysis
import analyze_server_room_perf as server_analysis


class ClientProfilerGateTests(unittest.TestCase):
    FRAME_COUNT = 3600
    CLASSES = ("LanceMaster", "Artist", "DimensionMaster", "Warlord")

    def make_summary(
        self,
        slot: int,
        phase: str = "simultaneous_worst",
        cadence_us: int = 16_000,
        cadence_spike_us: int = 0,
        omitted_counter: str = "",
    ) -> dict[str, object]:
        counters = {
            name: (cadence_us if name == "frameCadenceMicroseconds" else 0)
            for name in client_analysis.REQUIRED_V2_COUNTERS
            if name != omitted_counter
        }
        frames = [
            {
                "frameNumber": frame_number,
                "cpuFrameMs": 5.0,
                "gpuFrameMs": 8.0,
                "gpuValid": True,
                "counters": dict(counters),
                "pipeline": {},
                "cpuScopes": [],
            }
            for frame_number in range(1, self.FRAME_COUNT + 1)
        ]
        if cadence_spike_us:
            frames[-1]["counters"]["frameCadenceMicroseconds"] = cadence_spike_us
        capture = {
            "schema": "LostArkProfilerCapture.v2",
            "metadata": {
                "processId": 10_000 + slot,
                "clientSlot": slot,
                "levelId": 5,
                "width": 1280,
                "height": 720,
                "warmupFrames": 600,
                "requestedFrames": self.FRAME_COUNT,
                "configuration": "Release",
                "runId": "synthetic-run",
                "captureStartUtc100ns": 1_000_000_000 + slot,
                "captureEndUtc100ns": 1_600_000_000 + slot,
                "characterClass": self.CLASSES[slot - 1],
                "phase": phase,
                "buildRevision": "synthetic-build",
                "dataRevision": "synthetic-data",
            },
            "tickFrequency": 10_000_000,
            "capturedFrames": self.FRAME_COUNT,
            "droppedCpuScopes": 0,
            "droppedGpuFrames": 0,
            "scopeNames": [],
            "frames": frames,
        }
        return client_analysis.summarize_capture(
            pathlib.Path(f"slot{slot}.json"), capture
        )

    def make_group(self, **overrides: object) -> dict[str, object]:
        summaries = [
            self.make_summary(slot, **(overrides if slot == 4 else {}))
            for slot in range(1, 5)
        ]
        groups = client_analysis.group_worst_of_four(summaries)
        self.assertEqual(1, len(groups))
        return groups[0]

    def test_complete_four_client_group_passes(self) -> None:
        group = self.make_group()
        self.assertTrue(group["pass"])
        self.assertEqual(16.0, group["worstPresentCadenceP95Ms"])

    def test_missing_required_counter_fails(self) -> None:
        group = self.make_group(omitted_counter="effectRejected")
        self.assertFalse(group["pass"])
        self.assertFalse(group["gates"]["requiredCountersPresent"])

    def test_over_budget_present_cadence_fails(self) -> None:
        group = self.make_group(cadence_us=21_000)
        self.assertFalse(group["pass"])
        self.assertFalse(group["gates"]["presentCadenceP95AtMost16_67ms"])
        self.assertFalse(group["gates"]["presentCadenceP99AtMost20ms"])

    def test_single_cadence_hitch_fails_even_when_percentiles_pass(self) -> None:
        group = self.make_group(cadence_spike_us=101_000)
        self.assertFalse(group["pass"])
        self.assertTrue(group["gates"]["presentCadenceP95AtMost16_67ms"])
        self.assertTrue(group["gates"]["presentCadenceP99AtMost20ms"])
        self.assertFalse(group["gates"]["noFramesOver50ms"])
        self.assertFalse(group["gates"]["noFramesOver100ms"])

    def test_wrong_slot_or_roster_fails(self) -> None:
        summaries = [self.make_summary(slot) for slot in range(1, 5)]
        summaries[3]["metadata"]["clientSlot"] = 8
        summaries[3]["metadata"]["characterClass"] = "Gunslinger"
        group = client_analysis.group_worst_of_four(summaries)[0]
        self.assertFalse(group["pass"])
        self.assertFalse(group["gates"]["exactClientSlots"])
        self.assertFalse(group["gates"]["exactCharacterRoster"])

    def test_non_overlapping_capture_intervals_fail(self) -> None:
        summaries = [self.make_summary(slot) for slot in range(1, 5)]
        summaries[3]["metadata"]["captureStartUtc100ns"] = 2_000_000_000
        summaries[3]["metadata"]["captureEndUtc100ns"] = 2_600_000_000
        group = client_analysis.group_worst_of_four(summaries)[0]
        self.assertFalse(group["pass"])
        self.assertFalse(group["gates"]["simultaneousCaptureOverlap"])

    def test_momentary_overlap_does_not_count_as_simultaneous_run(self) -> None:
        summaries = [self.make_summary(slot) for slot in range(1, 5)]
        summaries[3]["metadata"]["captureStartUtc100ns"] = 1_599_990_000
        summaries[3]["metadata"]["captureEndUtc100ns"] = 2_199_990_000
        group = client_analysis.group_worst_of_four(summaries)[0]
        self.assertLess(group["simultaneousOverlapSeconds"], 1.0)
        self.assertFalse(group["pass"])
        self.assertFalse(group["gates"]["simultaneousCaptureOverlap"])

    def test_cli_fails_when_four_client_gate_is_not_evaluated(self) -> None:
        self.assertEqual(1, client_analysis.resolve_exit_code([], [{}], []))
        self.assertEqual(
            1,
            client_analysis.resolve_exit_code(
                [], [{}], [{"pass": None}]
            ),
        )

    def test_analysis_output_is_not_reloaded_as_capture_input(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            capture = (root / "capture.json").resolve()
            output = (root / "four-client-analysis.json").resolve()
            capture.write_text("{}", encoding="utf-8")
            output.write_text("{}", encoding="utf-8")
            resolved = client_analysis.exclude_output_path(
                client_analysis.resolve_inputs([str(root / "*.json")]), output
            )
            self.assertEqual([capture], resolved)

    def test_repeated_scope_occurrences_are_summed_per_frame(self) -> None:
        capture = {
            "schema": "LostArkProfilerCapture.v2",
            "tickFrequency": 1000,
            "capturedFrames": 1,
            "droppedCpuScopes": 0,
            "droppedGpuFrames": 0,
            "scopeNames": ["Effect.Occurrence.Update"],
            "frames": [{
                "frameNumber": 1,
                "cpuFrameMs": 10.0,
                "gpuFrameMs": 5.0,
                "gpuValid": True,
                "counters": {},
                "pipeline": {},
                "cpuScopes": [
                    {"nameId": 0, "beginTick": 10, "endTick": 12},
                    {"nameId": 0, "beginTick": 20, "endTick": 23},
                ],
            }],
        }
        summary = client_analysis.summarize_capture(
            pathlib.Path("scope.json"), capture
        )
        self.assertEqual(
            5.0, summary["cpuScopesMs"]["Effect.Occurrence.Update"]["p50"]
        )


class ServerRoomGateTests(unittest.TestCase):
    COMPLETE_LINE = (
		"[RoomPerf] Kind=heartbeat World=2 Players=4 Tick=1800 "
		"WindowSamples=1800 FourPlayerContinuousTicks=1800 "
		"CadenceP95Us=33500 CadenceP99Us=34500 CadenceOver50ms=0 "
        "TickP50Us=1000 TickP95Us=3000 TickP99Us=5000 TickMaxUs=9000 "
        "CommandP95Us=200 SimulationP95Us=2000 SimulationP99Us=3500 "
        "SnapshotWorkP95Us=500 SnapshotWorkP99Us=800 Remaining=0 "
        "BestEffortDropped=0 ReliableRejected=0 SnapshotEnqueueFailures=0 "
        "SnapshotDropped=0 OutboundQueuedMax=2 OutboundHighMax=4 "
        "OutboundReliableRejected=0 WireSendFailures=0"
    )

    def summarize_line(self, line: str) -> dict[str, object]:
        return server_analysis.summarize(server_analysis.parse_lines([line]))

    def test_complete_room_sample_passes(self) -> None:
        self.assertTrue(self.summarize_line(self.COMPLETE_LINE)["pass"])

    def test_missing_required_field_fails(self) -> None:
        line = self.COMPLETE_LINE.replace(" WindowSamples=1800", "")
        self.assertFalse(self.summarize_line(line)["pass"])

    def test_snapshot_drop_fails(self) -> None:
        line = self.COMPLETE_LINE.replace("SnapshotDropped=0", "SnapshotDropped=1")
        self.assertFalse(self.summarize_line(line)["pass"])

    def test_non_valtan_world_fails_four_client_raid_gate(self) -> None:
        line = self.COMPLETE_LINE.replace("World=2", "World=1")
        self.assertFalse(self.summarize_line(line)["pass"])

    def test_three_players_fail_four_client_raid_gate(self) -> None:
        line = self.COMPLETE_LINE.replace("Players=4", "Players=3")
        self.assertFalse(self.summarize_line(line)["pass"])

    def test_disconnect_anomaly_failure_is_not_hidden(self) -> None:
        anomaly = self.COMPLETE_LINE.replace(
            "Kind=heartbeat World=2 Players=4",
            "Kind=anomaly World=2 Players=0",
        ).replace("WireSendFailures=0", "WireSendFailures=1")
        summary = server_analysis.summarize(
            server_analysis.parse_lines([self.COMPLETE_LINE, anomaly])
        )
        self.assertFalse(summary["pass"])
        self.assertFalse(summary["gates"]["noWireSendFailure"])


if __name__ == "__main__":
    unittest.main()

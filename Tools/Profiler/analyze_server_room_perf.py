#!/usr/bin/env python3
"""Summarize Server [RoomPerf] lines and evaluate the four-client raid gates."""

from __future__ import annotations

import argparse
import glob
import json
import pathlib
import re
import sys
from collections import defaultdict


PREFIX = "[RoomPerf]"
PAIR = re.compile(r"(?P<key>[A-Za-z][A-Za-z0-9]*)=(?P<value>[^\s]+)")
REQUIRED_ROOM_PERF_FIELDS = {
    "Kind",
    "World",
    "Players",
    "Tick",
    "WindowSamples",
    "FourPlayerContinuousTicks",
    "CadenceP95Us",
    "CadenceP99Us",
    "CadenceOver50ms",
    "TickP50Us",
    "TickP95Us",
    "TickP99Us",
    "TickMaxUs",
    "CommandP95Us",
    "SimulationP95Us",
    "SimulationP99Us",
    "SnapshotWorkP95Us",
    "SnapshotWorkP99Us",
    "Remaining",
    "BestEffortDropped",
    "ReliableRejected",
    "SnapshotEnqueueFailures",
    "SnapshotDropped",
    "OutboundQueuedMax",
    "OutboundHighMax",
    "OutboundReliableRejected",
    "WireSendFailures",
}


def parse_lines(lines: list[str]) -> list[dict[str, object]]:
    rows: list[dict[str, object]] = []
    for line in lines:
        marker = line.find(PREFIX)
        if marker < 0:
            continue
        row: dict[str, object] = {}
        for match in PAIR.finditer(line[marker + len(PREFIX) :]):
            key = match.group("key")
            value = match.group("value")
            try:
                row[key] = int(value, 10)
            except ValueError:
                row[key] = value
        if "World" in row and "Tick" in row:
            rows.append(row)
    return rows


def maximum(rows: list[dict[str, object]], key: str) -> int:
    return max((int(row.get(key, 0)) for row in rows), default=0)


def summarize(rows: list[dict[str, object]]) -> dict[str, object]:
    by_world: dict[str, list[dict[str, object]]] = defaultdict(list)
    for row in rows:
        by_world[str(row["World"])].append(row)

    worlds: dict[str, object] = {}
    overall_pass = False
    for world, samples in sorted(by_world.items()):
        required_fields_present = all(
            REQUIRED_ROOM_PERF_FIELDS.issubset(sample) for sample in samples
        )
        values = {
            "logSamples": len(samples),
            "latestTick": int(samples[-1].get("Tick", 0)),
            "maximumPlayers": maximum(samples, "Players"),
            "fourPlayerContinuousTicks": maximum(
                samples, "FourPlayerContinuousTicks"
            ),
            "rollingSamples": maximum(samples, "WindowSamples"),
            "cadenceP95Us": maximum(samples, "CadenceP95Us"),
            "cadenceP99Us": maximum(samples, "CadenceP99Us"),
            "cadenceOver50ms": maximum(samples, "CadenceOver50ms"),
            "tickP50Us": maximum(samples, "TickP50Us"),
            "tickP95Us": maximum(samples, "TickP95Us"),
            "tickP99Us": maximum(samples, "TickP99Us"),
            "tickHardMaxUs": maximum(samples, "TickMaxUs"),
            "commandP95Us": maximum(samples, "CommandP95Us"),
            "simulationP95Us": maximum(samples, "SimulationP95Us"),
            "simulationP99Us": maximum(samples, "SimulationP99Us"),
            "snapshotWorkP95Us": maximum(samples, "SnapshotWorkP95Us"),
            "snapshotWorkP99Us": maximum(samples, "SnapshotWorkP99Us"),
            "remainingCommands": maximum(samples, "Remaining"),
            "bestEffortDropped": maximum(samples, "BestEffortDropped"),
            "reliableRejected": maximum(samples, "ReliableRejected"),
            "snapshotEnqueueFailures": maximum(
                samples, "SnapshotEnqueueFailures"
            ),
            "snapshotDropped": maximum(samples, "SnapshotDropped"),
            "outboundQueuedMax": maximum(samples, "OutboundQueuedMax"),
            "outboundHighMax": maximum(samples, "OutboundHighMax"),
            "outboundReliableRejected": maximum(
                samples, "OutboundReliableRejected"
            ),
            "wireSendFailures": maximum(samples, "WireSendFailures"),
        }
        gates = {
            "requiredFieldsPresent": required_fields_present,
            "hasRollingSamples": values["rollingSamples"] > 0,
            "tickP99AtMost10ms": values["tickP99Us"] <= 10_000,
            "tickHardMaxBelow33_33ms": values["tickHardMaxUs"] < 33_333,
            "noRemainingIngress": values["remainingCommands"] == 0,
            "noBestEffortDrop": values["bestEffortDropped"] == 0,
            "noReliableReject": values["reliableRejected"] == 0,
            "noSnapshotEnqueueFailure": values["snapshotEnqueueFailures"] == 0,
            "noSnapshotDrop": values["snapshotDropped"] == 0,
            "noOutboundReliableReject": values["outboundReliableRejected"] == 0,
            "noWireSendFailure": values["wireSendFailures"] == 0,
        }
        values["gates"] = gates
        values["pass"] = all(gates.values())
        worlds[world] = values

    valtan_rows = by_world.get("2", [])
    four_player_rows = [
        row for row in valtan_rows if int(row.get("Players", 0)) == 4
    ]
    admission_rows = [
        row for row in four_player_rows
        if str(row.get("Kind", "")) == "heartbeat"
        and int(row.get("WindowSamples", 0)) >= 1800
        and int(row.get("FourPlayerContinuousTicks", 0)) >= 1800
    ]
    valtan_required_fields_present = bool(valtan_rows) and all(
        REQUIRED_ROOM_PERF_FIELDS.issubset(row) for row in valtan_rows
    )
    gates = {
        "valtanWorldPresent": bool(valtan_rows),
        "requiredFieldsPresent": valtan_required_fields_present,
        "fourPlayersObserved": bool(four_player_rows),
        "continuousFourPlayersFor60Seconds": bool(admission_rows),
        "noAnomalyRows": not any(
            str(row.get("Kind", "")) == "anomaly" for row in valtan_rows
        ),
        "cadenceP95AtMost35ms": bool(four_player_rows)
        and maximum(four_player_rows, "CadenceP95Us") <= 35_000,
        "cadenceP99AtMost40ms": bool(four_player_rows)
        and maximum(four_player_rows, "CadenceP99Us") <= 40_000,
        "noCadenceOver50ms": maximum(valtan_rows, "CadenceOver50ms") == 0,
        "tickP99AtMost10ms": bool(four_player_rows)
        and maximum(four_player_rows, "TickP99Us") <= 10_000,
        "tickHardMaxBelow33_33ms": maximum(valtan_rows, "TickMaxUs") < 33_333,
        "noRemainingIngress": maximum(valtan_rows, "Remaining") == 0,
        "noBestEffortDrop": maximum(valtan_rows, "BestEffortDropped") == 0,
        "noReliableReject": maximum(valtan_rows, "ReliableRejected") == 0,
        "noSnapshotEnqueueFailure": maximum(
            valtan_rows, "SnapshotEnqueueFailures"
        ) == 0,
        "noSnapshotDrop": maximum(valtan_rows, "SnapshotDropped") == 0,
        "outboundQueueBelowPressureThreshold": maximum(
            valtan_rows, "OutboundQueuedMax"
        ) < 64,
        "outboundHighWatermarkBelowPressureThreshold": maximum(
            valtan_rows, "OutboundHighMax"
        ) < 64,
        "noOutboundReliableReject": maximum(
            valtan_rows, "OutboundReliableRejected"
        ) == 0,
        "noWireSendFailure": maximum(valtan_rows, "WireSendFailures") == 0,
    }
    overall_pass = all(gates.values())
    return {
        "schema": "LostArkServerRoomPerfSummary.v1",
        "pass": overall_pass,
        "gates": gates,
        "worlds": worlds,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("paths", nargs="*", help="Server log files; reads stdin when omitted")
    parser.add_argument("--glob", action="append", default=[], dest="globs")
    parser.add_argument("--json-out", type=pathlib.Path)
    args = parser.parse_args()

    paths = [pathlib.Path(path) for path in args.paths]
    for pattern in args.globs:
        paths.extend(pathlib.Path(path) for path in glob.glob(pattern))
    if paths:
        lines: list[str] = []
        for path in paths:
            lines.extend(path.read_text(encoding="utf-8", errors="replace").splitlines())
    else:
        lines = sys.stdin.read().splitlines()

    summary = summarize(parse_lines(lines))
    rendered = json.dumps(summary, ensure_ascii=False, indent=2)
    print(rendered)
    if args.json_out is not None:
        args.json_out.parent.mkdir(parents=True, exist_ok=True)
        args.json_out.write_text(rendered + "\n", encoding="utf-8")
    return 0 if summary["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())

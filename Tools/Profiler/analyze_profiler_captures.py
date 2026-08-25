#!/usr/bin/env python3
"""Summarize LostArk profiler v1/v2 captures without modifying them."""

from __future__ import annotations

import argparse
import glob
import json
import math
import pathlib
import statistics
import sys
from collections import defaultdict
from typing import Any, Iterable


FRAME_LIMITS_MS = (16.67, 20.0, 33.33, 50.0, 100.0)
REQUIRED_V2_COUNTERS = {
    "frameWaitMicroseconds",
    "frameCadenceMicroseconds",
    "frameUpdateMicroseconds",
    "frameRenderMicroseconds",
    "framePresentMicroseconds",
    "effectActiveOccurrences",
    "effectFixedSteps",
    "effectCatchupSteps",
    "effectParticles",
    "effectActualDraws",
    "effectRejected",
    "effectSuppressed",
    "animationCharacters",
    "animationCorrectionSeeks",
    "audioFirstLoads",
    "processPrivateBytes",
    "processWorkingSetBytes",
    "gpuLocalUsageBytes",
    "gpuLocalBudgetBytes",
}
EXPECTED_CLIENT_SLOTS = {1, 2, 3, 4}
EXPECTED_CHARACTER_CLASSES = {
    "LanceMaster", "Artist", "DimensionMaster", "Warlord"
}


def percentile(values: list[float], quantile: float) -> float:
    if not values:
        return 0.0
    ordered = sorted(values)
    if len(ordered) == 1:
        return ordered[0]
    position = (len(ordered) - 1) * quantile
    lower = math.floor(position)
    upper = math.ceil(position)
    if lower == upper:
        return ordered[lower]
    fraction = position - lower
    return ordered[lower] * (1.0 - fraction) + ordered[upper] * fraction


def distribution(values: list[float]) -> dict[str, float | int]:
    if not values:
        return {"count": 0, "p50": 0.0, "p95": 0.0, "p99": 0.0,
                "max": 0.0, "mean": 0.0, "sum": 0.0}
    return {
        "count": len(values),
        "p50": percentile(values, 0.50),
        "p95": percentile(values, 0.95),
        "p99": percentile(values, 0.99),
        "max": max(values),
        "mean": statistics.fmean(values),
        "sum": sum(values),
    }


def resolve_inputs(patterns: Iterable[str]) -> list[pathlib.Path]:
    resolved: set[pathlib.Path] = set()
    for pattern in patterns:
        matches = glob.glob(pattern, recursive=True)
        if not matches and pathlib.Path(pattern).is_file():
            matches = [pattern]
        for match in matches:
            path = pathlib.Path(match).resolve()
            if path.is_file() and path.suffix.lower() == ".json":
                resolved.add(path)
    return sorted(resolved)


def exclude_output_path(
    paths: list[pathlib.Path], output_path: pathlib.Path | None
) -> list[pathlib.Path]:
    if output_path is None:
        return paths
    resolved_output = output_path.resolve()
    return [path for path in paths if path != resolved_output]


def load_capture(path: pathlib.Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as stream:
        capture = json.load(stream)
    schema = capture.get("schema")
    if schema not in {"LostArkProfilerCapture.v1", "LostArkProfilerCapture.v2"}:
        raise ValueError(f"unsupported schema {schema!r}")
    frames = capture.get("frames")
    if not isinstance(frames, list) or not frames:
        raise ValueError("capture has no frames")
    return capture


def summarize_capture(path: pathlib.Path, capture: dict[str, Any]) -> dict[str, Any]:
    frames = capture["frames"]
    cpu_values = [float(frame.get("cpuFrameMs", 0.0)) for frame in frames]
    gpu_values = [float(frame.get("gpuFrameMs", 0.0)) for frame in frames
                  if frame.get("gpuValid", False)]
    counters: dict[str, list[float]] = defaultdict(list)
    pipeline: dict[str, list[float]] = defaultdict(list)
    for frame in frames:
        for name, value in frame.get("counters", {}).items():
            if isinstance(value, (int, float)):
                counters[name].append(float(value))
        for name, value in frame.get("pipeline", {}).items():
            if isinstance(value, (int, float)):
                pipeline[name].append(float(value))

    tick_frequency = int(capture.get("tickFrequency", 0))
    scope_names = capture.get("scopeNames", [])
    scope_durations_ms: dict[str, list[float]] = defaultdict(list)
    if tick_frequency > 0 and isinstance(scope_names, list):
        milliseconds_per_tick = 1000.0 / float(tick_frequency)
        for frame in frames:
            frame_scope_durations_ms: dict[str, float] = defaultdict(float)
            for scope in frame.get("cpuScopes", []):
                name_id = scope.get("nameId")
                begin_tick = scope.get("beginTick")
                end_tick = scope.get("endTick")
                if (not isinstance(name_id, int) or
                        name_id < 0 or name_id >= len(scope_names) or
                        not isinstance(scope_names[name_id], str) or
                        not isinstance(begin_tick, int) or
                        not isinstance(end_tick, int) or
                        end_tick < begin_tick):
                    continue
                frame_scope_durations_ms[scope_names[name_id]] += (
                    float(end_tick - begin_tick) * milliseconds_per_tick
                )
            for name, duration_ms in frame_scope_durations_ms.items():
                scope_durations_ms[name].append(duration_ms)

    metadata = capture.get("metadata", {})
    worst_frames = sorted(
        ({"frameNumber": int(frame.get("frameNumber", 0)),
          "cpuFrameMs": float(frame.get("cpuFrameMs", 0.0)),
          "gpuFrameMs": float(frame.get("gpuFrameMs", 0.0)),
          "gpuValid": bool(frame.get("gpuValid", False))}
         for frame in frames),
        key=lambda item: item["cpuFrameMs"],
        reverse=True)[:10]

    threshold_counts = {
        f"over{limit:g}Ms": sum(value > limit for value in cpu_values)
        for limit in FRAME_LIMITS_MS
    }
    threshold_rates = {
        name: count / len(cpu_values)
        for name, count in threshold_counts.items()
    }
    cadence_values_us = counters.get("frameCadenceMicroseconds", [])
    cadence_threshold_counts = {
        "over50Ms": sum(value > 50_000.0 for value in cadence_values_us),
        "over100Ms": sum(value > 100_000.0 for value in cadence_values_us),
    }
    return {
        "path": str(path),
        "schema": capture["schema"],
        "metadata": metadata,
        "tickFrequency": tick_frequency,
        "declaredCapturedFrames": int(capture.get("capturedFrames", 0)),
        "droppedCpuScopes": int(capture.get("droppedCpuScopes", 0)),
        "droppedGpuFrames": int(capture.get("droppedGpuFrames", 0)),
        "cpuFrameMs": distribution(cpu_values),
        "gpuFrameMs": distribution(gpu_values),
        "thresholdCounts": threshold_counts,
        "thresholdRates": threshold_rates,
        "cadenceThresholdCounts": cadence_threshold_counts,
        "presentCounters": sorted(counters),
        "counters": {name: distribution(values)
                     for name, values in sorted(counters.items())
                     if any(values)},
        "pipeline": {name: distribution(values)
                     for name, values in sorted(pipeline.items())
                     if any(values)},
        "cpuScopesMs": {name: distribution(values)
                        for name, values in sorted(scope_durations_ms.items())},
        "worstFrames": worst_frames,
    }


def group_worst_of_four(summaries: list[dict[str, Any]]) -> list[dict[str, Any]]:
    groups: dict[tuple[str, str, str, str, str, int, int, int, int, int],
                 list[dict[str, Any]]] = defaultdict(list)
    for summary in summaries:
        metadata = summary.get("metadata", {})
        phase = str(metadata.get("phase", ""))
        configuration = str(metadata.get("configuration", ""))
        if phase:
            groups[(
                configuration,
                phase,
                str(metadata.get("runId", "")),
                str(metadata.get("buildRevision", "")),
                str(metadata.get("dataRevision", "")),
                int(metadata.get("levelId", 0)),
                int(metadata.get("width", 0)),
                int(metadata.get("height", 0)),
                int(metadata.get("warmupFrames", 0)),
                int(metadata.get("requestedFrames", 0)),
            )].append(summary)

    output: list[dict[str, Any]] = []
    for identity, rows in sorted(groups.items()):
        (configuration, phase, run_id, build_revision, data_revision, level_id,
         width, height, warmup_frames, requested_frames) = identity
        worst_p95 = max(rows, key=lambda row: row["cpuFrameMs"]["p95"])
        worst_p99 = max(rows, key=lambda row: row["cpuFrameMs"]["p99"])
        worst_cadence_p95 = max(
            rows,
            key=lambda row: row.get("counters", {}).get(
                "frameCadenceMicroseconds", {}).get("p95", math.inf),
        )
        worst_cadence_p99 = max(
            rows,
            key=lambda row: row.get("counters", {}).get(
                "frameCadenceMicroseconds", {}).get("p99", math.inf),
        )
        cadence_p95_ms = float(worst_cadence_p95.get(
            "counters", {}).get(
                "frameCadenceMicroseconds", {}).get("p95", math.inf)
            ) / 1000.0
        cadence_p99_ms = float(worst_cadence_p99.get(
            "counters", {}).get(
                "frameCadenceMicroseconds", {}).get("p99", math.inf)
            ) / 1000.0
        client_slots = {
            int(row.get("metadata", {}).get("clientSlot", 0)) for row in rows
            if int(row.get("metadata", {}).get("clientSlot", 0)) > 0
        }
        process_ids = {
            int(row.get("metadata", {}).get("processId", 0)) for row in rows
            if int(row.get("metadata", {}).get("processId", 0)) > 0
        }
        character_classes = {
            str(row.get("metadata", {}).get("characterClass", ""))
            for row in rows
            if str(row.get("metadata", {}).get("characterClass", ""))
        }
        capture_starts = [
            int(row.get("metadata", {}).get("captureStartUtc100ns", 0))
            for row in rows
        ]
        capture_ends = [
            int(row.get("metadata", {}).get("captureEndUtc100ns", 0))
            for row in rows
        ]
        valid_capture_intervals = all(
            start > 0 and end > start
            for start, end in zip(capture_starts, capture_ends)
        )
        overlap_100ns = 0
        if valid_capture_intervals:
            overlap_100ns = max(
                0, min(capture_ends) - max(capture_starts)
            )
        # A one-tick intersection does not prove a simultaneous four-client
        # workload. Allow five seconds for process launch/capture skew, then
        # require the remainder of the nominal 60 Hz capture to overlap.
        required_overlap_100ns = max(
            1,
            max(0, requested_frames - 300) * 10_000_000 // 60,
        )
        captures_overlap = overlap_100ns >= required_overlap_100ns
        over_50 = sum(int(row.get("cadenceThresholdCounts", {}).get(
            "over50Ms", 0)) for row in rows)
        over_100 = sum(int(row.get("cadenceThresholdCounts", {}).get(
            "over100Ms", 0)) for row in rows)
        first_loads = sum(float(row.get("counters", {}).get(
            "audioFirstLoads", {}).get("sum", 0.0)) for row in rows)
        rejected = sum(float(row.get("counters", {}).get(
            "effectRejected", {}).get("sum", 0.0)) for row in rows)
        suppressed = sum(float(row.get("counters", {}).get(
            "effectSuppressed", {}).get("sum", 0.0)) for row in rows)
        v2_only = all(row["schema"] == "LostArkProfilerCapture.v2" for row in rows)
        complete_captures = requested_frames > 0 and all(
            int(row["cpuFrameMs"]["count"]) == requested_frames and
            int(row.get("declaredCapturedFrames", 0)) == requested_frames
            for row in rows
        )
        complete_gpu_coverage = all(
            int(row["gpuFrameMs"]["count"]) == int(row["cpuFrameMs"]["count"])
            for row in rows
        )
        required_counters_present = all(
            REQUIRED_V2_COUNTERS.issubset(set(row.get("presentCounters", [])))
            for row in rows
        )
        gates = {
            "exactlyFourCaptures": len(rows) == 4,
            "exactClientSlots": client_slots == EXPECTED_CLIENT_SLOTS,
            "fourDistinctProcesses": len(process_ids) == 4,
            "exactCharacterRoster": (
                character_classes == EXPECTED_CHARACTER_CLASSES
            ),
            "knownConfiguration": configuration in {"Debug", "Release"},
            "hasRunIdentity": bool(run_id),
            "simultaneousCaptureOverlap": captures_overlap,
            "hasRevisionIdentity": bool(build_revision and data_revision),
            "valtanClientLevel": level_id == 5,
            "resolution1280x720": width == 1280 and height == 720,
            "captureAtLeast60Seconds": 3600 <= requested_frames <= 7200,
            "validTickFrequency": all(
                int(row.get("tickFrequency", 0)) > 0 for row in rows
            ),
            "completeRequestedFrames": complete_captures,
            "completeGpuCoverage": complete_gpu_coverage,
            "requiredCountersPresent": required_counters_present,
            "noDroppedProfilerData": all(
                int(row["droppedCpuScopes"]) == 0 and
                int(row["droppedGpuFrames"]) == 0
                for row in rows
            ),
            "cpuWorkP95AtMost16_67ms": worst_p95["cpuFrameMs"]["p95"] <= 16.67,
            "cpuWorkP99AtMost20ms": worst_p99["cpuFrameMs"]["p99"] <= 20.0,
            "presentCadenceP95AtMost16_67ms": cadence_p95_ms <= 16.67,
            "presentCadenceP99AtMost20ms": cadence_p99_ms <= 20.0,
            "noFramesOver50ms": over_50 == 0,
            "noFramesOver100ms": over_100 == 0,
            "noSynchronousAudioFirstLoad": first_loads == 0.0,
            "noRejectedEffects": rejected == 0.0,
            "noSuppressedEffects": suppressed == 0.0,
        }
        output.append({
            "configuration": configuration,
            "phase": phase,
            "runId": run_id,
            "buildRevision": build_revision,
            "dataRevision": data_revision,
            "levelId": level_id,
            "resolution": [width, height],
            "warmupFrames": warmup_frames,
            "requestedFrames": requested_frames,
            "simultaneousOverlapSeconds": overlap_100ns / 10_000_000.0,
            "requiredSimultaneousOverlapSeconds": (
                required_overlap_100ns / 10_000_000.0
            ),
            "captureCount": len(rows),
            "clientSlots": sorted(client_slots),
            "processIds": sorted(process_ids),
            "characterClasses": sorted(character_classes),
            "worstCpuP95Ms": worst_p95["cpuFrameMs"]["p95"],
            "worstCpuP95Path": worst_p95["path"],
            "worstCpuP99Ms": worst_p99["cpuFrameMs"]["p99"],
            "worstCpuP99Path": worst_p99["path"],
            "worstPresentCadenceP95Ms": cadence_p95_ms,
            "worstPresentCadenceP95Path": worst_cadence_p95["path"],
            "worstPresentCadenceP99Ms": cadence_p99_ms,
            "worstPresentCadenceP99Path": worst_cadence_p99["path"],
            "framesOver50Ms": over_50,
            "framesOver100Ms": over_100,
            "audioFirstLoads": first_loads,
            "effectRejected": rejected,
            "effectSuppressed": suppressed,
            "gates": gates if v2_only else {},
            "pass": all(gates.values()) if v2_only else None,
        })
    return output


def compare(baseline: dict[str, Any], candidate: dict[str, Any]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for metric in ("p50", "p95", "p99", "max", "mean"):
        before = float(baseline["cpuFrameMs"][metric])
        after = float(candidate["cpuFrameMs"][metric])
        percent = 0.0 if before == 0.0 else (after - before) * 100.0 / before
        result[metric] = {"baseline": before, "candidate": after,
                          "changePercent": percent}
    return result


def resolve_exit_code(
    failures: list[str],
    summaries: list[dict[str, Any]],
    grouped: list[dict[str, Any]],
) -> int:
    if failures or not summaries or not grouped:
        return 1
    return 1 if any(group.get("pass") is not True for group in grouped) else 0


def print_summary(summary: dict[str, Any]) -> None:
    cpu = summary["cpuFrameMs"]
    gpu = summary["gpuFrameMs"]
    metadata = summary.get("metadata", {})
    identity = " ".join(filter(None, [
        str(metadata.get("configuration", "")),
        f"slot={metadata.get('clientSlot')}" if metadata.get("clientSlot") else "",
        str(metadata.get("characterClass", "")),
        str(metadata.get("phase", "")),
    ]))
    print(f"\n{summary['path']}")
    if identity:
        print(f"  {identity}")
    print("  CPU ms: "
          f"p50={cpu['p50']:.3f} p95={cpu['p95']:.3f} "
          f"p99={cpu['p99']:.3f} max={cpu['max']:.3f} "
          f"frames={cpu['count']}")
    if gpu["count"]:
        print("  GPU ms: "
              f"p50={gpu['p50']:.3f} p95={gpu['p95']:.3f} "
              f"p99={gpu['p99']:.3f} max={gpu['max']:.3f} "
              f"valid={gpu['count']}")
    rates = summary["thresholdRates"]
    print("  CPU threshold rates: " + " ".join(
        f"{name}={rate * 100.0:.3f}%" for name, rate in rates.items()))
    if summary["droppedCpuScopes"] or summary["droppedGpuFrames"]:
        print("  DROPPED: "
              f"cpuScopes={summary['droppedCpuScopes']} "
              f"gpuFrames={summary['droppedGpuFrames']}")
    scope_rows = sorted(
        summary.get("cpuScopesMs", {}).items(),
        key=lambda item: float(item[1]["p95"]),
        reverse=True)[:8]
    if scope_rows:
        print("  CPU scopes inclusive p95: " + ", ".join(
            f"{name}={values['p95']:.3f}ms"
            for name, values in scope_rows))
    print("  Worst CPU frames: " + ", ".join(
        f"{frame['frameNumber']}={frame['cpuFrameMs']:.3f}ms"
        for frame in summary["worstFrames"][:5]))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("captures", nargs="+", help="JSON path or glob")
    parser.add_argument("--baseline", help="Optional baseline JSON")
    parser.add_argument("--json-output", type=pathlib.Path)
    args = parser.parse_args()

    paths = exclude_output_path(resolve_inputs(args.captures), args.json_output)
    if not paths:
        parser.error("no profiler JSON files matched")

    summaries: list[dict[str, Any]] = []
    failures: list[str] = []
    for path in paths:
        try:
            summaries.append(summarize_capture(path, load_capture(path)))
        except (OSError, ValueError, json.JSONDecodeError) as error:
            failures.append(f"{path}: {error}")

    for summary in summaries:
        print_summary(summary)

    grouped = group_worst_of_four(summaries)
    for group in grouped:
        gate_text = "not-evaluated" if group["pass"] is None else (
            "PASS" if group["pass"] else "FAIL")
        print("\n  Four-client gate "
              f"{group['configuration']} {group['phase']}: {gate_text} "
              f"slots={group['clientSlots']} "
              f"cpuWorkP95={group['worstCpuP95Ms']:.3f}ms "
              f"cpuWorkP99={group['worstCpuP99Ms']:.3f}ms "
              f"cadenceP95={group['worstPresentCadenceP95Ms']:.3f}ms "
              f"cadenceP99={group['worstPresentCadenceP99Ms']:.3f}ms")

    report: dict[str, Any] = {
        "schema": "LostArkProfilerAnalysis.v1",
        "captures": summaries,
        "worstOfFour": grouped,
        "failures": failures,
    }
    if args.baseline and summaries:
        baseline_path = pathlib.Path(args.baseline).resolve()
        baseline = summarize_capture(baseline_path, load_capture(baseline_path))
        report["comparison"] = compare(baseline, summaries[-1])

    if args.json_output:
        args.json_output.parent.mkdir(parents=True, exist_ok=True)
        args.json_output.write_text(
            json.dumps(report, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8")

    for failure in failures:
        print(f"ERROR: {failure}", file=sys.stderr)
    return resolve_exit_code(failures, summaries, grouped)


if __name__ == "__main__":
    raise SystemExit(main())

"""Read saved v3 captures without launching the Client or changing their data."""
from __future__ import annotations

import argparse
from collections import defaultdict
import hashlib
import json
import math
from pathlib import Path
import statistics


def distribution(values):
    values = sorted(values)
    if not values:
        return {"samples": 0, "meanMs": None, "p50Ms": None,
                "p95Ms": None, "p99Ms": None, "maxMs": None}
    def percentile(p):
        return values[max(0, math.ceil(p * len(values)) - 1)]
    return {"samples": len(values), "meanMs": statistics.fmean(values),
            "p50Ms": percentile(.50), "p95Ms": percentile(.95),
            "p99Ms": percentile(.99), "maxMs": values[-1]}


def finite_number(value, label, minimum=0):
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ValueError(f"{label}: expected a number")
    if not math.isfinite(value) or value < minimum:
        raise ValueError(f"{label}: invalid number")
    return value


def interval_union(intervals):
    total = 0.0
    right = None
    for begin, end in sorted(intervals):
        if end < begin:
            raise ValueError("GPU scope ends before it begins")
        total += max(0.0, end - max(begin, right if right is not None else begin))
        right = max(end, right if right is not None else end)
    return total


def analyze(document, first=None, last=None):
    if document.get("schema") != "LostArkProfilerCapture.v3":
        raise ValueError("Only LostArkProfilerCapture.v3 is supported")
    source_frames = document["frames"]
    frames = [f for f in source_frames
              if (first is None or f["frameNumber"] >= first)
              and (last is None or f["frameNumber"] <= last)]
    if not frames:
        raise ValueError("Selected frame range is empty")
    ids = [f["frameNumber"] for f in frames]
    if ids != sorted(set(ids)):
        raise ValueError("Frame numbers must be unique and increasing")
    names = document["scopeNames"]
    tick_rate = finite_number(document["ticksPerSecond"], "ticksPerSecond", 1)
    factor = 1000.0 / tick_rate
    main_thread = document["mainThreadId"]
    partial_count = sum(f.get("droppedCpuScopes", 0) > 0 for f in frames)
    # Inclusive rows are intentionally not summed into a fictitious CPU total.
    cpu = defaultdict(lambda: {"totalMs": 0.0, "calls": 0, "frames": set()})
    gpu = defaultdict(lambda: {"totalMs": 0.0, "calls": 0, "frames": set()})
    work = defaultdict(lambda: {"totalMs": 0.0, "calls": 0, "samples": 0})
    counters = defaultdict(list)
    gpu_unions = []
    valid_gpu = []
    pass_gpu = []
    interval_values, cpu_values, animation_values = [], [], []
    detail_count = 0
    for frame in frames:
        frame_id = frame["frameNumber"]
        interval = finite_number(frame["frameIntervalMs"], "frameIntervalMs")
        if interval > 0:  # reset's first interval has no previous frame boundary
            interval_values.append(interval)
        cpu_values.append(finite_number(frame["cpuFrameMs"], "cpuFrameMs"))
        detail_count += bool(frame.get("detailedCpuScopes", False))
        for scope in frame.get("cpuScopes", []):
            index = scope["nameId"]
            if not isinstance(index, int) or not 0 <= index < len(names):
                raise ValueError("Invalid CPU scope nameId")
            begin = finite_number(scope["beginTick"], "beginTick")
            end = finite_number(scope["endTick"], "endTick")
            if end < begin:
                raise ValueError("CPU scope ends before it begins")
            row = cpu[names[index], scope["threadId"]]
            row["totalMs"] += (end - begin) * factor
            row["calls"] += 1
            row["frames"].add(frame_id)
        for item in frame.get("cpuWork", []):
            row = work[item["name"]]
            row["totalMs"] += finite_number(item["cpuMs"], "cpuWork cpuMs")
            row["calls"] += finite_number(item["calls"], "cpuWork calls")
            row["samples"] += 1
        for name, value in frame.get("counters", {}).items():
            counters[name].append(finite_number(value, f"counter {name}"))
        if "animation" in frame:
            animation_values.append(finite_number(frame["animation"]["cpuMs"], "animation cpuMs"))
        gpu_valid = bool(frame.get("gpuValid", False))
        if "gpuStatus" in frame and gpu_valid != (frame["gpuStatus"] == "valid"):
            raise ValueError("gpuValid and gpuStatus disagree")
        if gpu_valid:
            valid_gpu.append(finite_number(frame["gpuFrameMs"], "gpuFrameMs"))
            if frame.get("gpuScopesSupported", False) and frame.get("droppedGpuScopes", 0) == 0:
                pass_gpu.append(frame)
                ranges = []
                for scope in frame.get("gpuScopes", []):
                    index = scope["nameId"]
                    if not isinstance(index, int) or not 0 <= index < len(names):
                        raise ValueError("Invalid GPU scope nameId")
                    begin = finite_number(scope["beginMs"], "beginMs")
                    end = finite_number(scope["endMs"], "endMs")
                    ranges.append((begin, end))
                    row = gpu[names[index]]
                    row["totalMs"] += finite_number(scope["durationMs"], "GPU durationMs")
                    row["calls"] += 1
                    row["frames"].add(frame_id)
                gpu_unions.append(interval_union(ranges))
    count = len(frames)
    cpu_rows = [{"name": name, "threadId": thread, "mainThread": thread == main_thread,
                 "inclusiveMsPerFrame": row["totalMs"] / count,
                 "callsPerFrame": row["calls"] / count, "observedFrames": len(row["frames"])}
                for (name, thread), row in cpu.items()]
    cpu_rows.sort(key=lambda row: row["inclusiveMsPerFrame"], reverse=True)
    gpu_rows = [{"name": name, "inclusiveMsPerValidFrame": row["totalMs"] / len(pass_gpu),
                 "observedFrames": len(row["frames"])} for name, row in gpu.items()]
    gpu_rows.sort(key=lambda row: row["inclusiveMsPerValidFrame"], reverse=True)
    intervals = distribution(interval_values)
    warnings = [
        "CPU/cpuWork/GPU rows are inclusive and must not be added across parents and children.",
        "GPU timestamps are elapsed intervals, not utilization or pure GPU execution time.",
        "Metadata describes export time; identical metadata does not prove identical historical scenes.",
        "frameIntervalMs in row N spans Begin(N-1) to Begin(N); CPU scopes belong to row N.",
        "Texture counters with no runtime writer are not evidence of zero texture work.",
    ]
    if partial_count:
        warnings.append("Raw CPU scope attribution is incomplete; recorded totals can be lower bounds. Self is not computed.")
    if detail_count:
        warnings.append("Detailed CPU instrumentation was enabled; its overhead is part of these observations.")
    return {
        "schema": "LostArkProfilerAnalysis.v1",
        "metadataAtExport": document.get("metadata", {}),
        "captureWindow": document.get("captureWindow"),
        "selection": {"sourceFrames": len(source_frames), "frames": count,
                      "firstFrame": ids[0], "lastFrame": ids[-1],
                      "contiguous": all(b == a + 1 for a, b in zip(ids, ids[1:]))},
        "frameInterval": intervals,
        "fpsFromMeanInterval": 1000 / intervals["meanMs"] if intervals["meanMs"] else None,
        "cpuFrame": distribution(cpu_values), "gpuFrameValidOnly": distribution(valid_gpu),
        "gpuPassSampleFrames": len(pass_gpu), "gpuScopeUnion": distribution(gpu_unions),
        "recordedAnimation": distribution(animation_values),
        "quality": {"detailedFrames": detail_count, "partialCpuFrames": partial_count,
                    "windowDroppedCpuScopes": sum(f.get("droppedCpuScopes", 0) for f in frames),
                    "invalidOrPendingGpuFrames": count - len(valid_gpu)},
        "cpuScopes": cpu_rows, "gpuScopes": gpu_rows,
        "cpuWork": [{"name": name, "samples": row["samples"],
                     "inclusiveMsPerSampleFrame": row["totalMs"] / row["samples"],
                     "callsPerSampleFrame": row["calls"] / row["samples"]}
                    for name, row in sorted(work.items())],
        "counters": {name: {"samples": len(values), "mean": statistics.fmean(values),
                            "min": min(values), "max": max(values)}
                     for name, values in sorted(counters.items())},
        "slowCpuFrames": [{"frameNumber": f["frameNumber"], "cpuFrameMs": f["cpuFrameMs"],
                           "precedingIntervalMs": f["frameIntervalMs"],
                           "drawCalls": f.get("counters", {}).get("drawCalls"),
                           "droppedCpuScopes": f.get("droppedCpuScopes", 0)}
                          for f in sorted(frames, key=lambda f: f["cpuFrameMs"], reverse=True)[:10]],
        "warnings": warnings,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("captures", type=Path, nargs="+")
    parser.add_argument("--first-frame", type=int)
    parser.add_argument("--last-frame", type=int)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.output.resolve() in {p.resolve() for p in args.captures}:
        parser.error("Output must not replace an input capture")
    if args.output.exists():
        parser.error("Output already exists; choose a new analysis filename")
    reports = []
    try:
        for path in args.captures:
            data = path.read_bytes()
            report = analyze(json.loads(data.decode("utf-8-sig")), args.first_frame, args.last_frame)
            report["input"] = {"path": str(path.resolve()), "sha256": hashlib.sha256(data).hexdigest(),
                               "bytes": len(data)}
            reports.append(report)
        result = {"captures": reports}
        if len(reports) == 2:
            a, b = reports
            keys = ("buildConfiguration", "adapter", "viewport", "d3dDebugLayer", "debuggerAttached",
                    "levelId", "effectiveFpsLimit", "shadowEnabled", "ssaoEnabled", "bloomEnabled", "fxaaEnabled")
            result["comparison"] = {
                "differentExportMetadata": [key for key in keys
                    if a["metadataAtExport"].get(key) != b["metadataAtExport"].get(key)],
                "observedMeanIntervalRatioSecondToFirst":
                    b["frameInterval"]["meanMs"] / a["frameInterval"]["meanMs"]
                    if a["frameInterval"]["meanMs"] and b["frameInterval"]["meanMs"] else None,
                "note": "Observed ratio only. Camera path, warmup, tool windows and capture mode require separate matching; not a causal speedup claim."}
        args.output.parent.mkdir(parents=True, exist_ok=True)
        with args.output.open("x", encoding="utf-8", newline="\n") as stream:
            json.dump(result, stream, ensure_ascii=False, indent=2, allow_nan=False)
            stream.write("\n")
    except (OSError, ValueError, KeyError, TypeError) as error:
        parser.exit(2, f"Capture analysis failed: {error}\n")
    for report in reports:
        print(json.dumps({"input": report["input"]["path"], "frames": report["selection"]["frames"],
                          "meanMs": report["frameInterval"]["meanMs"],
                          "fps": report["fpsFromMeanInterval"], "quality": report["quality"]}, ensure_ascii=False))


if __name__ == "__main__":
    main()

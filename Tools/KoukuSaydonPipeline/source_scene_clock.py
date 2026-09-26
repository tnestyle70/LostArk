"""Map an enabled Matinee Slomo track between scene and real elapsed time.

Camera, actor, visibility and event projectors must share this map. Slomo is
scene advance per elapsed second, so elapsed = integral(1 / rate(scene)).
Inputs and outputs use milliseconds; interpolation follows the source UE3
constant, linear or cubic Hermite key segment. This module never writes data.
"""
from __future__ import annotations

import bisect
import math
from dataclasses import dataclass
from typing import Any


def _integrate(function, left: float, right: float, tolerance: float = 1e-10) -> float:
    """Adaptive Simpson integration, bounded to twenty subdivision levels."""
    if left == right:
        return 0.0
    middle = (left + right) * .5
    a, b, c = function(left), function(middle), function(right)
    estimate = (right - left) * (a + 4 * b + c) / 6

    def split(lo, hi, flo, fmid, fhi, whole, budget, depth):
        mid = (lo + hi) * .5
        fl, fr = function((lo + mid) * .5), function((mid + hi) * .5)
        first = (mid - lo) * (flo + 4 * fl + fmid) / 6
        second = (hi - mid) * (fmid + 4 * fr + fhi) / 6
        difference = first + second - whole
        if abs(difference) <= 15 * budget:
            return first + second + difference / 15
        if not depth:
            raise ValueError("Slomo integration did not converge")
        return (split(lo, mid, flo, fl, fmid, first, budget * .5, depth - 1) +
                split(mid, hi, fmid, fr, fhi, second, budget * .5, depth - 1))

    return split(left, right, a, b, c, estimate, tolerance, 20)


@dataclass(frozen=True)
class _Segment:
    begin: float
    end: float
    first: float
    last: float
    leave: float
    arrive: float
    mode: str

    def rate(self, seconds: float) -> float:
        alpha = (seconds - self.begin) / (self.end - self.begin)
        if self.mode == "cim_constant":
            result = self.first
        elif self.mode == "cim_linear":
            result = self.first + alpha * (self.last - self.first)
        else:
            span = self.end - self.begin
            result = ((2 * alpha ** 3 - 3 * alpha ** 2 + 1) * self.first +
                      (alpha ** 3 - 2 * alpha ** 2 + alpha) * span * self.leave +
                      (-2 * alpha ** 3 + 3 * alpha ** 2) * self.last +
                      (alpha ** 3 - alpha ** 2) * span * self.arrive)
        if not math.isfinite(result) or result <= 0:
            raise ValueError("Slomo rate must remain positive and finite")
        return result

    def elapsed(self, seconds: float) -> float:
        if self.mode == "cim_constant":
            return (seconds - self.begin) / self.first
        return _integrate(lambda t: 1 / self.rate(t), self.begin, seconds)


class SourceSceneClock:
    """Immutable clock for one source scene; no per-frame state or game clock."""

    def __init__(self, points: list[dict[str, Any]], duration_ms: float):
        if not math.isfinite(duration_ms) or duration_ms <= 0:
            raise ValueError("Source duration must be positive and finite")
        self.source_duration_ms = duration_ms
        duration = duration_ms * .001
        source = points or [dict(inval=0., outval=1., interpmode="cim_constant")]
        if float(source[0]["inval"]) != 0:
            raise ValueError("Slomo projection requires an explicit first key at scene time zero")
        self._segments = []
        self._scene_starts = []
        self._elapsed_starts = []
        elapsed = 0.
        previous = -math.inf
        for index, key in enumerate(source):
            begin = float(key["inval"])
            if not math.isfinite(begin) or begin <= previous:
                raise ValueError("Slomo keys must have finite, strictly ascending times")
            previous = begin
            if begin >= duration:
                continue
            next_key = source[index + 1] if index + 1 < len(source) else key
            end = float(next_key["inval"]) if next_key is not key else duration
            mode = str(key.get("interpmode", "cim_linear")).lower()
            if mode not in ("cim_constant", "cim_linear", "cim_curveauto", "cim_curveautoclamped", "cim_curveuser", "cim_curvebreak"):
                raise ValueError("Unsupported Slomo interpolation: " + mode)
            # The last key holds indefinitely, regardless of its interpolation tag.
            if next_key is key:
                mode = "cim_constant"
            segment = _Segment(begin, end, float(key["outval"]), float(next_key["outval"]),
                               float(key.get("leavetangent", 0)), float(next_key.get("arrivetangent", 0)), mode)
            if end <= begin:
                raise ValueError("Slomo has an empty source segment")
            self._segments.append(segment)
            self._scene_starts.append(begin)
            self._elapsed_starts.append(elapsed)
            elapsed += segment.elapsed(min(end, duration))
        self.elapsed_duration_ms = elapsed * 1000

    @classmethod
    def from_scene_rows(cls, rows: dict[int, dict], interp_data: int):
        data = rows[interp_data]["p"]
        tracks = [rows[track] for group in data["interpgroups"]
                  for track in rows[group]["p"].get("interptracks", [])
                  if rows[track]["cls"] == "interptrackslomo" and not rows[track]["p"].get("bdisabletrack", False)]
        if len(tracks) > 1:
            raise ValueError("Scene has multiple enabled Slomo tracks")
        points = tracks[0]["p"].get("floattrack", {}).get("points", []) if tracks else []
        return cls(points, float(data["interplength"]) * 1000)

    def to_elapsed_ms(self, source_ms: float) -> float:
        if not math.isfinite(source_ms) or source_ms < 0 or source_ms > self.source_duration_ms + 1e-6:
            raise ValueError("Source time is outside the scene")
        seconds = min(source_ms, self.source_duration_ms) * .001
        index = max(0, bisect.bisect_right(self._scene_starts, seconds) - 1)
        return (self._elapsed_starts[index] + self._segments[index].elapsed(seconds)) * 1000

    def to_source_ms(self, elapsed_ms: float) -> float:
        if not math.isfinite(elapsed_ms) or elapsed_ms < 0 or elapsed_ms > self.elapsed_duration_ms + 1e-6:
            raise ValueError("Elapsed time is outside the scene")
        seconds = min(elapsed_ms, self.elapsed_duration_ms) * .001
        index = max(0, bisect.bisect_right(self._elapsed_starts, seconds) - 1)
        segment = self._segments[index]
        target = seconds - self._elapsed_starts[index]
        if segment.mode == "cim_constant":
            return min(segment.end, segment.begin + target * segment.first) * 1000
        low, high = segment.begin, min(segment.end, self.source_duration_ms * .001)
        value = low + (high - low) * target / segment.elapsed(high)
        for _ in range(48):
            difference = segment.elapsed(value) - target
            if abs(difference) <= 1e-9:
                return value * 1000
            if difference > 0:
                high = value
            else:
                low = value
            next_value = value - difference * segment.rate(value)
            value = next_value if low < next_value < high else (low + high) * .5
        raise ValueError("Slomo inverse did not converge")

    def source_clock_keys(self, maximum_error_ms: float = .05) -> list[dict[str, float]]:
        """Piecewise-linear elapsed/source keys with quarter/midpoint error checks.

        The exact conversion methods remain the authoring authority. Consumers
        should independently verify the key table at their sampling frequency.
        """
        if not math.isfinite(maximum_error_ms) or maximum_error_ms <= 0:
            raise ValueError("Clock key error must be positive and finite")
        elapsed = [x * 1000 for x in self._elapsed_starts] + [self.elapsed_duration_ms]
        result = [(elapsed[0], 0.)]

        def append(left, right, source_left, source_right, depth):
            worst = 0.
            for alpha in (.25, .5, .75):
                t = left + (right - left) * alpha
                worst = max(worst, abs(self.to_source_ms(t) - (source_left + (source_right - source_left) * alpha)))
            if worst <= maximum_error_ms:
                result.append((right, source_right))
                return
            if not depth:
                raise ValueError("Source clock key reduction did not converge")
            middle = (left + right) * .5
            source_middle = self.to_source_ms(middle)
            append(left, middle, source_left, source_middle, depth - 1)
            append(middle, right, source_middle, source_right, depth - 1)

        for left, right in zip(elapsed, elapsed[1:]):
            append(left, right, self.to_source_ms(left), self.to_source_ms(right), 20)
        return [dict(timeMs=t, sourceMs=s) for t, s in result]

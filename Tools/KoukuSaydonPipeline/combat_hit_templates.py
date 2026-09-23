"""Bounded attack templates shared by Kouku dynamic visual instances.

These are attack primitives, never a particle-system bounding box. Times are
relative to the authoritative CombatObject birth; offsets use its facing basis.
"""
from __future__ import annotations

import copy
import math
import re


HIT_DEFAULTS = dict(hitId="", trigger="TIMED", atMs=0, endMs=0,
                    repeatCount=1, repeatIntervalMs=0, shape="CIRCLE",
                    radiusM=1.0, innerRadiusM=0.0, lengthM=0.0, halfWidthM=0.0,
                    angleDegrees=0.0, offsetForwardM=0.0, offsetRightM=0.0,
                    yawOffsetDegrees=0.0, damageKind="MAX_HP_PERCENT",
                    damagePercent=10, damageProfileId="")


def validate_hits(rows, lifetime_ms=600000):
    if not isinstance(rows, list) or len(rows) > 32:
        raise ValueError("Attack templates require a bounded array of at most 32 hits")
    result, ids = [], set()
    for row in rows:
        if not isinstance(row, dict) or set(row) - (HIT_DEFAULTS.keys() | {"riseHeightM", "pushMs"}):
            raise ValueError("Attack hit has unknown fields")
        hit = {**HIT_DEFAULTS, **copy.deepcopy(row)}
        identity = hit["hitId"]
        if not isinstance(identity, str) or not re.fullmatch(r"[A-Za-z0-9_.-]{1,128}", identity) or identity in ids:
            raise ValueError("Attack hit ID must be stable and unique within its template")
        ids.add(identity)
        for field in ("atMs", "endMs", "repeatCount", "repeatIntervalMs", "damagePercent"):
            maximum = 64 if field == "repeatCount" else 100 if field == "damagePercent" else 600000
            if type(hit[field]) is not int or not 0 <= hit[field] <= maximum:
                raise ValueError("Attack hit integer is invalid: " + field)
        for field in ("radiusM", "innerRadiusM", "lengthM", "halfWidthM", "angleDegrees", "offsetForwardM", "offsetRightM", "yawOffsetDegrees"):
            value = hit[field]
            bound = 360 if field in ("angleDegrees", "yawOffsetDegrees") else 1000
            minimum = -bound if field in ("offsetForwardM", "offsetRightM", "yawOffsetDegrees") else 0
            if type(value) not in (int, float) or not math.isfinite(value) or not minimum <= value <= bound:
                raise ValueError("Attack hit number is invalid: " + field)
        if hit["repeatCount"] < 1 or (hit["repeatCount"] > 1 and hit["repeatIntervalMs"] < 34):
            raise ValueError("Attack repetitions need at least one 30 Hz tick between hits")
        last = hit["atMs"] + (hit["repeatCount"] - 1) * hit["repeatIntervalMs"]
        if hit["trigger"] == "TIMED":
            if hit["endMs"] or last > lifetime_ms:
                raise ValueError("Timed attack exceeds the owning lifetime")
        elif hit["trigger"] == "CONTACT":
            if not hit["atMs"] < hit["endMs"] <= lifetime_ms:
                raise ValueError("Contact attack needs a bounded active window")
        else:
            raise ValueError("Attack trigger must be TIMED or CONTACT")
        shape = hit["shape"]
        if shape == "CIRCLE":
            valid = hit["radiusM"] > 0 and hit["innerRadiusM"] == 0
        elif shape == "RING":
            valid = 0 < hit["innerRadiusM"] < hit["radiusM"]
        elif shape == "BOX":
            valid = hit["lengthM"] > 0 and hit["halfWidthM"] > 0
        elif shape == "CONE":
            valid = hit["lengthM"] > 0 and 0 < hit["angleDegrees"] <= 360 and hit["innerRadiusM"] < hit["lengthM"]
        else:
            valid = False
        if not valid:
            raise ValueError("Attack primitive dimensions are invalid")
        kind = hit["damageKind"]
        if kind == "PROFILE":
            valid = isinstance(hit["damageProfileId"], str) and re.fullmatch(r"[A-Za-z0-9_.-]{1,128}", hit["damageProfileId"]) and hit["damagePercent"] == 0
        else:
            valid = hit["damageProfileId"] == "" and ((kind == "MAX_HP_PERCENT" and 1 <= hit["damagePercent"] <= 100) or (kind == "INSTANT_DEATH" and hit["damagePercent"] == 0))
        if not valid:
            raise ValueError("Attack damage policy is invalid")
        height, duration = hit.get("riseHeightM", 0), hit.get("pushMs", 0)
        if (type(height) not in (int, float) or not math.isfinite(height) or not 0 <= height <= 100
                or type(duration) is not int or (duration != 0 if height == 0 else not 100 <= duration <= 5000)):
            raise ValueError("Attack rise height and flight time must form a bounded pair")
        if "riseHeightM" in row or "pushMs" in row:
            hit["riseHeightM"], hit["pushMs"] = height, duration
        result.append(hit)
    return result


def validate_logic_hits(logic):
    kind = logic.get("judgementKind") if logic.get("logicType") == "DURATION" else logic.get("triggerKind", "")
    allowed = {"fixedHits", "trackingHits", "randomVolleyHits"} if kind == "SHOWTIME_PLAYER_TARGETS" else {"projectileHits"} if kind == "PURSUIT_PROJECTILES" else {"fixedHits"} if kind == "ALBION_BLUE_CIRCLE" else set()
    keys = {"fixedHits", "trackingHits", "randomVolleyHits", "projectileHits"} & logic.keys()
    if keys - allowed:
        raise ValueError("Only the matching dynamic attack owner can carry hit templates")
    for key in keys - {"randomVolleyHits"}:
        validate_hits(logic[key], logic.get("effectLifetimeMs", 0) if kind == "ALBION_BLUE_CIRCLE" else 600000)
    if "randomVolleyHits" in keys:
        rows = logic["randomVolleyHits"]
        if not isinstance(rows, list) or len(rows) != len(logic.get("randomVolleyOccurrenceSets", [])):
            raise ValueError("Random attack templates must match the ordered visual set count")
        for hits in rows:
            validate_hits(hits)
    if kind != "ALBION_BLUE_CIRCLE" and logic.get("fixedHits") and not logic.get("fixedSelectionGroupId"):
        raise ValueError("Fixed attack lacks its visual owner")
    if logic.get("trackingHits") and not logic.get("trackingPresentationOccurrenceId"):
        raise ValueError("Tracking attack lacks its visual owner")

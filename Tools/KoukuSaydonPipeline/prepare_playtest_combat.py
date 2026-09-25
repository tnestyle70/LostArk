"""Prepare the September playtest contact fixes on a detached Composition.

The caller owns fresh-save merge and publication. Existing authored colliders
retain their geometry; new attack primitives are explicit project tuning, not
particle-system bounds or a claim about original Lost Ark damage geometry.
"""
from __future__ import annotations

import copy
import json
import math
from pathlib import Path

try:
    from .combat_hit_templates import validate_hits
except ImportError:
    from combat_hit_templates import validate_hits

ROOT = Path(__file__).resolve().parents[2]
PREFIX = "KAKULSAYDON_G1_PATTERN_"
TRIGGER = "kakulsaydon.g1.logic.508"
DAMAGE = "kakulsaydon.g1.logic.98"


def prepare_zero_floor_madness(document):
    """Field-only follow-up: floor damage cannot exceed breath's 3% budget.

    Explicit MADNESS_GAUGE_ADD_PERCENT zero is a damage-verdict override, not
    absence of an authored result. Preserve every floor's original damage,
    clock, geometry and the directly authored three 1% breath verdicts.
    """
    out = copy.deepcopy(document)
    patterns = {p["patternId"]: p for p in out["patterns"]}
    logics = {r["logicId"]: r for r in out["logics"]}
    name = "Breath lingering flame: no additional madness"
    existing = [r for r in out["logics"] if r.get("displayName") == name]
    if len(existing) > 1:
        raise ValueError("Duplicate floor madness override")
    changes = []
    if existing:
        zero = existing[0]
        if zero.get("outcomeKind") != "MADNESS_GAUGE_ADD_PERCENT" or zero.get("percent") != 0:
            raise ValueError("Existing floor madness override was edited")
    else:
        ordinal = out["nextLogicOrdinal"]
        identity = f"kakulsaydon.g1.logic.{ordinal}"
        if identity in logics:
            raise ValueError("Logic ordinal does not reserve a new stable ID")
        out["nextLogicOrdinal"] += 1
        zero = dict(logicId=identity, displayName=name, logicType="RESULT",
                    outcomeKind="MADNESS_GAUGE_ADD_PERCENT", percent=0,
                    durationMs=0, followupPatternId="")
        out["logics"].append(zero)
        changes.append(dict(kind="add-logic", after=zero))
    for number in (43, 102, 114):
        p = patterns[PREFIX + str(number)]
        for suffix in range(2, 9):
            identity = f'{p["patternId"]}.logic.{suffix}'
            occurrence = next(r for r in p["logicOccurrences"] if r["occurrenceId"] == identity)
            if not occurrence.get("enabled", True):
                raise ValueError("Lingering floor was disabled; reinspect: " + identity)
            before = occurrence["onSuccessLogicIds"]
            if before == ["kakulsaydon.g1.logic.515", zero["logicId"]]:
                continue
            if before != ["kakulsaydon.g1.logic.515"]:
                raise ValueError("Lingering floor outcome changed; preserve it: " + identity)
            occurrence["onSuccessLogicIds"] = before + [zero["logicId"]]
            changes.append(dict(kind="field", patternId=p["patternId"], occurrenceId=identity,
                field="onSuccessLogicIds", before=before, after=occurrence["onSuccessLogicIds"]))
    return out, dict(sourceRevision=document["revision"], changes=changes,
        nextLogicOrdinalBefore=document["nextLogicOrdinal"], nextLogicOrdinalAfter=out["nextLogicOrdinal"])


def prepare(document, root=ROOT):
    out = copy.deepcopy(document)
    patterns = {p["patternId"]: p for p in out["patterns"]}
    resources = {r["resourceId"]: r for r in out["presentationResources"]}
    logics = {l["logicId"]: l for l in out["logics"]}
    changes = []

    def pattern(number):
        return patterns[PREFIX + str(number)]

    def new_logic(**values):
        ordinal = out["nextLogicOrdinal"]
        out["nextLogicOrdinal"] += 1
        identity = f"kakulsaydon.g1.logic.{ordinal}"
        row = dict(logicId=identity, **values)
        out["logics"].append(row)
        logics[identity] = row
        return identity

    def resource(label, shape, **geometry):
        ordinal = out["nextPresentationResourceOrdinal"]
        out["nextPresentationResourceOrdinal"] += 1
        identity = f"kakulsaydon.g1.presentation.{ordinal}"
        row = dict(resourceId=identity, displayName=label, defaultAnchorKind="BOSS",
                   kind="COLLIDER", assetId="", resourceKind="GROUP", elementId="",
                   durationMs=100, colliderKind="GEOMETRY", shape=shape,
                   halfExtents=[1, 1, 1], radiusM=1, halfAngleDegrees=45)
        row.update(geometry)
        out["presentationResources"].append(row)
        resources[identity] = row
        return identity

    def window(p, start, duration, result=DAMAGE, trigger=TRIGGER):
        ordinal = p["nextLogicOccurrenceOrdinal"]
        p["nextLogicOccurrenceOrdinal"] += 1
        identity = f'{p["patternId"]}.logic.{ordinal}'
        row = dict(occurrenceId=identity, logicId=trigger, startMs=start,
                   durationMs=duration, enabled=True,
                   onSuccessLogicIds=result if isinstance(result, list) else [result],
                   onFailLogicIds=[], onTimeoutLogicIds=[])
        p["logicOccurrences"].append(row)
        return identity

    def rotate(position, yaw):
        angle = math.radians(yaw)
        x, y, z = position
        return [x * math.cos(angle) + z * math.sin(angle), y,
                -x * math.sin(angle) + z * math.cos(angle)]

    def collider(p, fx, rid, delay=0, duration=100, local=(0, 0, 0),
                 yaw=0, result=DAMAGE, shared_window=None):
        # Existing attack carriers are yaw-only. Refuse to flatten a later edit.
        if any(abs(fx.get("rotationDegrees", [0, 0, 0])[axis]) > .001 for axis in (0, 2)):
            raise ValueError("Reinspect pitched/rolled attack carrier: " + fx["occurrenceId"])
        start = fx["startMs"] + delay
        lid = shared_window or window(p, start, duration, result)
        offset = rotate([a * b for a, b in zip(local, fx["scale"])], fx["rotationDegrees"][1])
        ordinal = p["nextPresentationOccurrenceOrdinal"]
        p["nextPresentationOccurrenceOrdinal"] += 1
        row = copy.deepcopy(fx)
        row.update(occurrenceId=f'{p["patternId"]}.presentation.{ordinal}',
                   resourceId=rid, startMs=start, durationMs=duration,
                   logicOccurrenceId=lid, debugRender=True, bone="", boneTarget="BODY",
                   positionOffset=[a + b for a, b in zip(fx["positionOffset"], offset)],
                   rotationDegrees=[0, fx["rotationDegrees"][1] + yaw, 0])
        for key in ("effectSourceStartMs", "selectionGroupId", "effectTimePolicy"):
            row.pop(key, None)
        p["presentationOccurrences"].append(row)
        changes.append(dict(kind="attack-contact", patternId=p["patternId"],
                            source=fx["occurrenceId"], collider=row["occurrenceId"],
                            startMs=start, durationMs=duration))
        return row

    def effects(p, asset):
        return [o for o in p["presentationOccurrences"]
                if resources[o["resourceId"]].get("assetId") == asset]

    # Steering selection never restricts who can intercept the card. Runtime
    # already compares the contacted player's suit with the card's authored suit.
    for ordinal in range(125, 129):
        row = logics[f"kakulsaydon.g1.logic.{ordinal}"]
        for hit in row["projectileHits"]:
            hit["damagePercent"] = 90
        changes.append(dict(kind="dice-card-damage", logicId=row["logicId"], maxHpPercent=90))

    # The six throws already capture separate Server target positions. Damage
    # belongs to those existing objects at the actual impact occurrence time.
    p = pattern(106)
    for occurrence in p["logicOccurrences"]:
        owner = logics[occurrence["logicId"]]
        group = owner.get("selectedEffectGroupId")
        if not group:
            continue
        impact = next(o for o in p["presentationOccurrences"]
                      if o.get("selectionGroupId") == group and
                      resources[o["resourceId"]].get("assetId") == "effect.kouku.gate2.juggling.target.impact")
        owner["fixedHits"] = validate_hits([dict(hitId="juggling.impact", trigger="TIMED",
            atMs=impact["startMs"] - occurrence["startMs"], radiusM=1.5,
            damageKind="MAX_HP_PERCENT", damagePercent=10)])
        changes.append(dict(kind="juggling-impact", logicId=owner["logicId"],
                            atMs=owner["fixedHits"][0]["atMs"], radiusM=1.5, maxHpPercent=10))

    # Preserve the user's existing wind box and wire it to its disabled result.
    p = pattern(99)
    hit = next(o for o in p["presentationOccurrences"] if o["occurrenceId"].endswith(".11"))
    verdict = next(o for o in p["logicOccurrences"] if o["occurrenceId"].endswith(".2"))
    verdict.update(enabled=True, startMs=hit["startMs"], durationMs=hit["durationMs"], logicId=TRIGGER)
    hit["logicOccurrenceId"] = verdict["occurrenceId"]
    changes.append(dict(kind="connect-existing-wind", collider=hit["occurrenceId"]))

    # Both beam windows covered only 231ms of a 1634ms visible shot. Admit a
    # single contact for the full authored beam window, including first-shot
    # entry after the old window. One shot cannot repeat after its own push.
    p = pattern(85)
    for collider_ordinal, effect_ordinal in ((23, 1), (24, 6)):
        hit = next(o for o in p["presentationOccurrences"] if o["occurrenceId"].endswith(f".{collider_ordinal}"))
        fx = next(o for o in p["presentationOccurrences"] if o["occurrenceId"].endswith(f".{effect_ordinal}"))
        verdict = next(o for o in p["logicOccurrences"] if o["occurrenceId"] == hit["logicOccurrenceId"])
        hit.update(startMs=fx["startMs"], durationMs=fx["durationMs"])
        verdict.update(startMs=fx["startMs"], durationMs=fx["durationMs"], logicId=TRIGGER)
        changes.append(dict(kind="bazooka-visible-contact", collider=hit["occurrenceId"],
                            startMs=fx["startMs"], durationMs=fx["durationMs"]))

    launch = new_logic(displayName="Playtest hit: 10% HP and ballistic knockdown",
        logicType="RESULT", outcomeKind="MAX_HP_PERCENT_DAMAGE", percent=10,
        durationMs=0, followupPatternId="", pushRangeM=4, pushMs=1000,
        pushHeightM=2, forcePush=True, pushBallistic=True, pushDirection="AWAY_FROM_CONTACT")
    # Reconnect the ten actual authored rainbow boxes; preserve all tuned TRS.
    p = pattern(38)
    for hit in list(p["presentationOccurrences"]):
        if resources[hit["resourceId"]]["kind"] == "COLLIDER" and not hit.get("logicOccurrenceId"):
            hit["logicOccurrenceId"] = window(p, hit["startMs"], hit["durationMs"], launch)
            changes.append(dict(kind="connect-existing-rainbow", collider=hit["occurrenceId"]))

    # FireWave positions are user-editable manual groups. Read their current
    # pillar centres and birth times, not the old triangular generator layout.
    for number in (49, 58, 59, 115):
        p = pattern(number)
        for fx in list(p["presentationOccurrences"]):
            asset = resources[fx["resourceId"]].get("assetId", "")
            if not asset.startswith("effect.kouku.common.flame.wave.full"):
                continue
            doc = json.loads((Path(root) / "Data/Effects/Authored" / (asset + ".effect.json")).read_text(encoding="utf-8-sig"))
            seen = set()
            for element in doc["elements"]:
                group = element.get("groupId", "")
                if not group.startswith("manual.flame-wave.") or group in seen or "pillar" not in element["displayName"]:
                    continue
                seen.add(group)
                delay = round(element["detail"]["timing"]["startDelaySeconds"] * 1000)
                if delay >= fx["durationMs"]:
                    continue
                collider(p, fx, "kakulsaydon.g1.presentation.356", delay,
                         min(500, fx["durationMs"] - delay), element["detail"]["transform"]["position"])

    # Reuse the existing authored 1.6m blue-circle attack definition for its
    # static floor occurrences; the dynamic player-target object keeps its hit.
    blue = resource("Albion blue floor impact", "CIRCLE", radiusM=1.6)
    prong = resource("Albion electric prong contact", "BOX", halfExtents=[1.2, 2, 5])
    fan = resource("Albion four fan contact", "SECTOR", radiusM=12, halfAngleDegrees=22.5)
    for number in (39, 120):
        p = pattern(number)
        for occurrence in p["logicOccurrences"]:
            owner = logics[occurrence["logicId"]]
            if owner.get("triggerKind", owner.get("judgementKind")) == "ALBION_BLUE_CIRCLE":
                for hit in owner.get("fixedHits", []):
                    hit.update(pushRangeM=4, riseHeightM=2, pushMs=1000,
                               forcePush=True, pushDirection="AWAY_FROM_CONTACT")
                owner["fixedHits"] = validate_hits(owner.get("fixedHits", []))
        for fx in effects(p, "effect.kouku.albion.bluecircle.warning.impact.runtime"):
            collider(p, fx, blue, 2000, 100, result=launch)
        for fx in effects(p, "effect.kouku.albion.frontthree.electric.impact"):
            # Three authored component roots and yaw values are the direction
            # evidence. Width/length are conservative project gameplay tuning.
            asset = resources[fx["resourceId"]]["assetId"]
            doc = json.loads((Path(root) / "Data/Effects/Authored" / (asset + ".effect.json")).read_text(encoding="utf-8-sig"))
            transforms = {}
            for e in doc["elements"]:
                t = e["detail"]["transform"]
                transforms.setdefault(tuple(round(v, 4) for v in t["position"]), t)
            lid = window(p, fx["startMs"], 100)
            for t in transforms.values():
                yaw = t["rotationDegrees"][1] - 90
                forward = rotate([0, 0, 5], yaw)
                local = [a + b for a, b in zip(t["position"], forward)]
                collider(p, fx, prong, 0, 100, local, yaw, shared_window=lid)
        for fx in effects(p, "effect.kouku.albion.fourfan.impact"):
            lid = window(p, fx["startMs"], 100, launch)
            for yaw in (0, 90, 180, 270):
                collider(p, fx, fan, 0, 100, yaw=yaw, shared_window=lid)

    # The existing breath damage is preserved at 100 per verdict. Three
    # overlap verdicts now carry an explicit 1% gauge result each. The runtime
    # suppresses automatic HP-based gauge only for a verdict with this result.
    madness = new_logic(displayName="Breath contact madness 1%", logicType="RESULT",
        outcomeKind="MADNESS_GAUGE_ADD_PERCENT", percent=1, durationMs=0, followupPatternId="")
    intervals = {}
    for number, suffixes in ((27, (3,)), (43, (1,)), (67, (1,)), (68, (1,)),
                             (69, (1,)), (70, (1,)), (102, (9,)), (114, (1,))):
        p = pattern(number)
        for o in p["logicOccurrences"]:
            if int(o["occurrenceId"].rsplit(".", 1)[1]) not in suffixes or not o.get("enabled", True):
                continue
            interval = max(34, math.ceil(o["durationMs"] / 3))
            if interval not in intervals:
                intervals[interval] = new_logic(displayName="Breath: three contact ticks", logicType="DURATION",
                    judgementKind="AREA_OVERLAP", insideOutcome="SUCCESS", repeatIntervalMs=interval)
            o["logicId"] = intervals[interval]
            o["onSuccessLogicIds"] = ["kakulsaydon.g1.logic.515", madness]
            changes.append(dict(kind="breath-three-ticks", occurrenceId=o["occurrenceId"],
                                damageAmount=100, madnessPerTickPercent=1, intervalMs=interval))

    # The three simultaneous rays form one breath verdict. Moving between
    # them must retain the same per-player tick budget instead of tripling it.
    p = pattern(102)
    primary = next(o for o in p["logicOccurrences"] if o["occurrenceId"].endswith(".9"))
    old_ids = set()
    for o in p["logicOccurrences"]:
        if o["occurrenceId"].endswith((".10", ".11")):
            if (o["startMs"], o["durationMs"]) != (primary["startMs"], primary["durationMs"]):
                raise ValueError("Three-ray breath clocks diverged; inspect before merging verdicts")
            o["enabled"] = False
            old_ids.add(o["occurrenceId"])
    for o in p["presentationOccurrences"]:
        if o.get("logicOccurrenceId") in old_ids:
            o["logicOccurrenceId"] = primary["occurrenceId"]
    changes.append(dict(kind="share-three-ray-breath-verdict", occurrenceId=primary["occurrenceId"]))

    # Bad-feeling breath previously had only an Effect. Use a single front
    # attack primitive per authored occurrence, leaving its saved facing intact.
    breath = resource("Bad feeling breath damage", "BOX", halfExtents=[1.5, 2, 4.5])
    for number in (50, 51, 53, 54, 55, 60):
        p = pattern(number)
        for fx in effects(p, "effect.kouku.gate3.clone.breath"):
            collider(p, fx, breath, 0, min(fx["durationMs"], 1000), local=(4.15, 0, -1.89), yaw=90)

    return out, {"sourceRevision": document["revision"], "changes": changes,
                 "newGeometryBasis": "PROJECT_TUNED; authored point positions/times preserved"}


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    source = ROOT / "Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json"
    before = json.loads(source.read_text(encoding="utf-8-sig"))
    after, receipt = prepare(before)
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output / "before.json").write_bytes(source.read_bytes())
    for name, value in (("candidate.json", after), ("receipt.json", receipt)):
        (args.output / name).write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

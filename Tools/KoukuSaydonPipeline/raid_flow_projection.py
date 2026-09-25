"""Project saved action flows and sequence arrival tracks into one Server raid plan."""
from __future__ import annotations

import math
import copy
from typing import Any


def validate_bingo_special(action: dict[str, Any], flow: dict[str, Any]) -> str:
    """Validate the authored Parent before its children are flattened for runtime."""
    identity = flow.get("bingoSpecialPatternId", "")
    patterns = {row["patternId"]: row for row in action["patterns"]}
    parent = patterns.get(identity) if isinstance(identity, str) else None
    if flow.get("gateId") != "BINGO" or not parent or parent.get("authoringStatus") != "PRODUCT" or \
            parent.get("gateId") != "BINGO" or parent.get("targetBossPlacementId") != "boss.kakulsaydon.bingo.saydon" or \
            not parent.get("patternOccurrences") or parent.get("loopStartPatternOccurrenceId"):
        raise ValueError("BINGO publication requires bingoSpecialPatternId naming an admitted finite Parent on the Bingo boss")
    bundle_map = {row["bundleId"]: row for row in action.get("bundles", [])}
    if any((entry["kind"] == "PATTERN" and entry["targetId"] == identity) or
           (entry["kind"] == "BUNDLE" and any(member["patternId"] == identity for member in
            bundle_map.get(entry["targetId"], {}).get("members", []))) for entry in flow["entries"]):
        raise ValueError("Bingo special cannot also be a normal Flow entry or Bundle member")
    owners = [parent]
    for box in parent["patternOccurrences"]:
        child = patterns.get(box.get("patternId"))
        if not child or child is parent or box.get("repeat", False) or child.get("patternOccurrences") or \
                child.get("authoringStatus") != "PRODUCT" or \
                any(child.get(key) != parent.get(key) for key in ("gateId", "actorProfileId", "targetBossPlacementId")):
            raise ValueError("Bingo special children must be admitted finite Patterns on the same Gate and boss")
        owners.append(child)
    logics = {row["logicId"]: row for row in action.get("logics", [])}
    detonations = sum(1 for owner in owners for box in owner.get("logicOccurrences", [])
                      if box.get("enabled", True) and logics.get(box.get("logicId"), {}).get("logicType") == "TRIGGER"
                      and logics[box["logicId"]].get("triggerKind") == "BINGO_DETONATION")
    if detonations != 1:
        raise ValueError("Bingo special requires exactly one enabled BINGO_DETONATION across its Parent and children")
    return identity


def project_raid_gates(action: dict[str, Any], sequence: dict[str, Any]) -> list[dict[str, Any]]:
    flows = {row["gateId"]: row for row in action.get("patternFlows", [])}
    sequences = {row["patternId"]: row for row in sequence["patterns"]}
    logics = {row["logicId"]: row for row in sequence.get("logics", [])}
    worlds = {row["worldId"]: row for row in sequence.get("worlds", [])}
    products = {row["patternId"] for row in action["patterns"] if row["authoringStatus"] == "PRODUCT"}
    bundles = {row["bundleId"] for row in action.get("bundles", [])}
    result = []
    for gate, intro_number, clear_number, primary in (
        ("GATE1", 4, None, "boss.kakulsaydon.g1.saydon"),
        ("GATE2", 3, None, "boss.kakulsaydon.g2.kouku"),
        ("GATE3", 5, None, "boss.kakulsaydon.g3.saydon"),
        ("BINGO", 10, 9, "boss.kakulsaydon.bingo.saydon"),
    ):
        flow = flows.get(gate)
        if not flow or not flow["entries"]:
            continue
        intro = sequences[f"KAKULSAYDON_G1_PATTERN_{intro_number}"] if intro_number else None
        clear = sequences[f"KAKULSAYDON_G1_PATTERN_{clear_number}"] if clear_number else None
        # Gate 2's complete exit movie includes the Gate 3 arrival; keep its
        # source actor/gate identity while assigning the combat handoff to Gate 3.
        intro_gate = "GATE2" if gate == "GATE3" else gate
        if intro is not None and (intro.get("gateId") != intro_gate or not intro.get("enterCombatOnFinish", False)):
            raise ValueError(f"{gate} raid intro must be its authored combat handoff sequence")
        entries = []
        for row in flow["entries"]:
            targets = products if row["kind"] == "PATTERN" else bundles
            if row["targetId"] not in targets:
                raise ValueError(f"{gate} flow cannot admit unavailable {row['targetId']}")
            entries.append(dict(row))
        special_id = validate_bingo_special(action, flow) if gate == "BINGO" else ""
        loop_start = flow.get("loopStartEntryId", "")
        if not isinstance(loop_start, str) or (loop_start and loop_start not in {row["entryId"] for row in entries}):
            raise ValueError(f"{gate} flow loop start is not a saved entry")
        if "entryGroups" in flow:
            from project_kouku_saydon_composition import validate_flow_groups
            validate_flow_groups(flow)
        arrivals = []
        intro_ready = gate == "BINGO"
        durations = {}
        for phase, pattern in (("INTRO", intro), ("CLEAR", clear)):
            if pattern is None:
                durations[phase] = 0
                continue
            duration = int(pattern.get("durationMs", 0) or sum(s["durationMs"] for s in pattern["stages"]))
            if not 1 <= duration <= 600000:
                raise ValueError(f"{gate} {phase} duration is outside the supported clock")
            durations[phase] = duration
            slots = set()
            for box in pattern.get("logicOccurrences", []):
                if not box.get("enabled", True) or logics[box["logicId"]].get("triggerKind") != "ROOM_PLAYER_ARRIVAL":
                    continue
                arrival = box["roomPlayerArrival"]
                slot, position = arrival["playerSlot"], arrival["position"]
                if slot not in range(4) or slot in slots or not 0 <= box["startMs"] <= duration:
                    raise ValueError(f"{gate} {phase} arrival slot/time is invalid or duplicate")
                if len(position) != 3 or any(not math.isfinite(v) or abs(v) > 100000 for v in position):
                    raise ValueError(f"{gate} {phase} arrival position is invalid")
                slots.add(slot)
                arrivals.append({"phase": phase, "occurrenceId": box["occurrenceId"], "playerSlot": slot,
                                 "startMs": box["startMs"], "position": list(position)})
            if phase == "INTRO":
                intro_ready = (not slots) if gate == "BINGO" else slots == set(range(4))
        # Unfinished arrival authoring must not advertise a runnable raid gate or
        # prevent independent saved patterns from being published.
        if not intro_ready:
            continue
        entry_sequences = {worlds[box["worldId"]]["sequenceInstanceId"]
                           for box in (intro or {}).get("worldOccurrences", [])
                           if gate == "GATE1" and box.get("enabled", True) and box["startMs"] == 0}
        if len(entry_sequences) > 1:
            raise ValueError("Gate 1 entry requires one unambiguous World sequence at time zero")
        result.append({"gateId": gate, "flowId": flow["flowId"],
                       "sequenceCompositionId": sequence["compositionId"], "sequenceRevision": sequence["revision"],
                       "introPatternId": intro["patternId"] if intro else "", "introDurationMs": durations["INTRO"],
                       "clearPatternId": clear["patternId"] if clear else "", "clearDurationMs": durations["CLEAR"],
                       "primaryBossPlacementId": primary, "entries": entries, "arrivals": arrivals,
                       "entrySequenceInstanceId": next(iter(entry_sequences), ""),
                       **({"loopStartEntryId": loop_start} if loop_start else {}),
                       **({"bingoSpecialPatternId": special_id} if special_id else {}),
                       **({"entryGroups": copy.deepcopy(flow["entryGroups"])} if "entryGroups" in flow else {})})
    return result

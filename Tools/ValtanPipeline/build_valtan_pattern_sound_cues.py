#!/usr/bin/env python3
"""Join Valtan's raw extracted SOUND events to the authored pattern/stage/clip
join keys and emit Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json.

Three real, exact-string joins (see .md/TJ/08-25/2026-08-25_VALTAN_PATTERN_SOUND_CUE_PLAN.md
section 1 for the measured proof each key actually lines up):

  Valtan.animevents SOUND row (clip, startMs, bank, event)
    -> Valtan.patternbindings.json bindings[].clips[].clip == clip
       (gives actionId, clipOccurrenceId, that clip's own sourceStartMs/loop)
    -> ValtanEncounter.json patterns[].stages[].actionId == actionId
       (gives patternId, stageId, durationMs)

Rows whose bank is S_BGM_CommanderRaid or S_Systems are skipped (not monster
voice/impact sound -- see PLAN section 5) and reported separately, not counted
as unmatched. A row whose clip or actionId has no join match is also skipped,
not fatal -- Valtan's authored patterns don't cover every clip in the raw
animevents yet (phase 2/3 gaps), same reasoning as the player GunSlinger/
Slayer sound gaps.

No network/DB use. Pure filesystem read + deterministic JSON write.
"""
from __future__ import annotations

import json
import re
import sys
from collections import defaultdict
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
ANIMEVENTS_PATH = (
    REPO_ROOT / "Data" / "Animation" / "Reference" / "Valtan" / "Valtan.animevents"
)
PATTERNBINDINGS_PATH = (
    REPO_ROOT / "Data" / "Animation" / "Authored" / "Valtan" / "Valtan.patternbindings.json"
)
ENCOUNTER_PATH = REPO_ROOT / "Data" / "Encounters" / "Valtan" / "ValtanEncounter.json"
OUT_PATH = (
    REPO_ROOT / "Data" / "Animation" / "Authored" / "Valtan" / "Valtan.patternsoundcues.json"
)

SKIPPED_BANKS = {"S_BGM_COMMANDERRAID", "S_SYSTEMS"}
# These extracted Wwise event names have no entry or payload in the checked-in
# Valtan Sound catalog. Emitting them makes Composition publication fail and
# still produces silence at runtime. The STAGGER_SLOT wipe uses Attack25_Shot2,
# the available impact event from the existing MAGIC_ORB_STAGGER_76 wipe.
UNAVAILABLE_RUNTIME_EVENTS = {
    "G_Voltan2_Attack15_Cast4",
    "G_Voltan2_Attack16_Cast1",
    "G_Voltan2_Attack16_Shot1",
    "G_Voltan2_Attack16_Shot2",
    "G_Voltan2_Attack16_ShotVox1",
}

SOUND_ROW_RE = re.compile(
    r'^"(?P<clip>[^"]+)"\s+SOUND\s+(?P<fields>.*?)\s*src=\S+\s*$'
)
STARTMS_RE = re.compile(r'startms=(\d+)')
PAYLOAD_RE = re.compile(r'payload="([^"]*)"')


def _semantic_cue(
    binding_id: str,
    pattern_id: str,
    stage_id: str,
    action_id: str,
    clip_occurrence_id: str,
    start_ms: int,
    sound_event: str,
) -> dict:
    """Build one authored impact row on the raw model source clock.

    Extracted SOUND rows remain untouched. These rows cover reviewed HIT
    contacts for which the source clip has no impact event within one Server
    tick, while retaining the same typed action/occurrence join as extraction.
    """
    return {
        "bindingId": binding_id,
        "occurrenceId": f"{binding_id}.occurrence.01",
        "patternId": pattern_id,
        "stageId": stage_id,
        "actionId": action_id,
        "clipOccurrenceId": clip_occurrence_id,
        "soundBank": "S_Mob_G_Voltan2",
        "soundEvent": sound_event,
        "repeatPolicy": "once",
        "startMs": start_ms,
    }


def _build_project_authored_semantic_cues() -> dict[str, dict]:
    cues: list[dict] = []

    def add(
        suffix: str,
        pattern_id: str,
        stage_id: str,
        action_id: str,
        clip_occurrence_id: str,
        start_ms: int,
        sound_event: str,
        *,
        binding_id: str | None = None,
    ) -> None:
        cues.append(_semantic_cue(
            binding_id or f"cue.sound.valtan.semantic.{suffix}",
            pattern_id,
            stage_id,
            action_id,
            clip_occurrence_id,
            start_ms,
            sound_event,
        ))

    # Existing project-authored rows are retained with their stable IDs.
    ground_action = "valtan.sequence.sequence.400440.0.step-01"
    ground_occurrence = "valtan.sequence.sequence.400440.0.step-01.clip-01"
    for index, (start_ms, event) in enumerate((
        (1, "G_Voltan2_Attack09_ProjCreat1"),
        (600, "G_Voltan2_FootStep1"),
        (1300, "G_Voltan2_FootStep1"),
        (1300, "G_Voltan2_Attack02_Shot1"),
    ), start=1):
        add(
            f"ground-roar.legacy-{index:02d}",
            "VALTAN_GROUND_ROAR",
            "STEP_01",
            ground_action,
            ground_occurrence,
            start_ms,
            event,
            binding_id=(
                "cue.sound.valtan.sequence.ground-roar.step-01.clip-01."
                f"{index:02d}"
            ),
        )
    add(
        "part-break.recovery.projection",
        "VALTAN_PART_BREAK",
        "PART_BREAK_RECOVERY",
        "valtan.reaction.part-break.recovery",
        "valtan.reaction.part-break.recovery.clip.01",
        1,
        "G_Voltan2_Attack09_ProjCreat1",
        binding_id="cue.sound.valtan.reaction.part-break.recovery.clip-01.01",
    )

    # The merged Ground Roar stage has two stomp contacts, followed by the
    # embedded roar occurrence at stage-local 1800 + clip-local 900ms.
    add(
        "ground-roar.stomp-impact-600",
        "VALTAN_GROUND_ROAR",
        "STEP_01",
        ground_action,
        ground_occurrence,
        600,
        "G_Voltan2_Attack02_Shot1",
    )

    roar_occurrences = (
        (
            "ground-roar",
            "VALTAN_GROUND_ROAR",
            "STEP_01",
            ground_action,
            "valtan.sequence.sequence.400440.0.step-01.roar.clip-05",
        ),
        (
            "bind-slot-recovery",
            "VALTAN_BIND_SLOT",
            "RECOVERY",
            "valtan.authoring.bind-slot.recovery",
            "VALTAN_BIND_SLOT.RECOVERY.composition.clip.01",
        ),
        (
            "roar-charge-step-03",
            "VALTAN_ROAR_CHARGE",
            "STEP_03",
            "valtan.sequence.roar-charge.step-03",
            "valtan.sequence.roar-charge.step-03.clip-01",
        ),
        (
            "terrain-destruction-step-11",
            "VALTAN_TERRAIN_DESTRUCTION",
            "STEP_11",
            "valtan.sequence.jump-whirlwind-roar-roar-charge.step-11",
            "valtan.sequence.jump-whirlwind-roar-roar-charge.step-11.clip-01",
        ),
        (
            "struggling-step-10",
            "VALTAN_STRUGGLING",
            "STEP_10",
            "valtan.sequence.warp-jump-four-hand-twohand-roar-roar-dead.step-10",
            "valtan.sequence.warp-jump-four-hand-twohand-roar-roar-dead.step-10.clip-01",
        ),
    )
    for suffix, pattern_id, stage_id, action_id, occurrence_id in roar_occurrences:
        add(
            f"{suffix}.roar-impact",
            pattern_id,
            stage_id,
            action_id,
            occurrence_id,
            900,
            "G_Voltan2_Attack18_Shot7",
        )

    stomp_occurrences = (
        (
            "bind-slot-step-01",
            "VALTAN_BIND_SLOT",
            "STEP_01",
            "valtan.authoring.bind-slot.step-01",
            "VALTAN_BIND_SLOT.STEP_01.composition.clip.01",
        ),
        (
            "roar-charge-step-01",
            "VALTAN_ROAR_CHARGE",
            "STEP_01",
            "valtan.sequence.roar-charge.step-01",
            "valtan.sequence.roar-charge.step-01.clip-01",
        ),
        (
            "terrain-destruction-step-09",
            "VALTAN_TERRAIN_DESTRUCTION",
            "STEP_09",
            "valtan.sequence.jump-whirlwind-roar-roar-charge.step-09",
            "valtan.sequence.jump-whirlwind-roar-roar-charge.step-09.clip-01",
        ),
        (
            "struggling-step-08",
            "VALTAN_STRUGGLING",
            "STEP_08",
            "valtan.sequence.warp-jump-four-hand-twohand-roar-roar-dead.step-08",
            "valtan.sequence.warp-jump-four-hand-twohand-roar-roar-dead.step-08.clip-01",
        ),
    )
    for suffix, pattern_id, stage_id, action_id, occurrence_id in stomp_occurrences:
        add(
            f"{suffix}.stomp-impact",
            pattern_id,
            stage_id,
            action_id,
            occurrence_id,
            1200,
            "G_Voltan2_Attack02_Shot1",
        )

    for suffix, pattern_id, stage_id, action_id, occurrence_id, start_ms, event in (
        (
            "three-step-01", "VALTAN_THREE", "STEP_01",
            "valtan.sequence.three.step-01",
            "valtan.sequence.three.step-01.clip-01",
            1617, "G_Voltan2_Attack02_Shot1",
        ),
        (
            "three-step-02", "VALTAN_THREE", "STEP_02",
            "valtan.sequence.three.step-02",
            "valtan.sequence.three.step-02.clip-01",
            963, "G_Voltan2_Attack02_Shot1",
        ),
        (
            "three-step-03", "VALTAN_THREE", "STEP_03",
            "valtan.sequence.three.step-03",
            "valtan.sequence.three.step-03.clip-01",
            1300, "G_Voltan2_Attack02_Shot2",
        ),
        (
            "three-step-03.legacy-impact-500", "VALTAN_THREE", "STEP_03",
            "valtan.sequence.three.step-03",
            "valtan.sequence.three.step-03.clip-01",
            500, "G_Voltan2_Attack02_Shot2",
        ),
        (
            "cross-step-01", "VALTAN_CROSS", "STEP_01",
            "valtan.sequence.cross.step-01",
            "valtan.sequence.cross.step-01.clip-01",
            1617, "G_Voltan2_Attack02_Shot1",
        ),
    ):
        add(suffix, pattern_id, stage_id, action_id, occurrence_id, start_ms, event)

    for suffix, pattern_id, action_id, occurrence_id in (
        (
            "sequence-four", "VALTAN_SEQUENCE_FOUR",
            "valtan.sequence.four.step-01",
            "valtan.sequence.four.step-01.clip-01",
        ),
        (
            "struggling-step-04", "VALTAN_STRUGGLING",
            "valtan.sequence.warp-jump-four-hand-twohand-roar-roar-dead.step-04",
            "valtan.sequence.warp-jump-four-hand-twohand-roar-roar-dead.step-04.clip-01",
        ),
    ):
        for start_ms in (1233, 2233, 3233, 4200):
            add(
                f"{suffix}.impact-{start_ms}",
                pattern_id,
                "STEP_01" if pattern_id == "VALTAN_SEQUENCE_FOUR" else "STEP_04",
                action_id,
                occurrence_id,
                start_ms,
                "G_Voltan2_Attack18_Shot2",
            )

    add(
        "roar-charge-step-06.swing-impact",
        "VALTAN_ROAR_CHARGE",
        "STEP_06",
        "valtan.sequence.roar-charge.step-06",
        "valtan.sequence.roar-charge.step-06.clip-01",
        200,
        "G_Voltan2_Attack25_Shot2",
    )

    for start_ms in (2450, 2650, 2850, 3050, 3250, 4600):
        add(
            f"sequence-rush.step-03.impact-{start_ms}",
            "VALTAN_SEQUENCE_RUSH",
            "STEP_03",
            "valtan.sequence.rush.step-03",
            "valtan.sequence.rush.step-03.clip-01",
            start_ms,
            "G_Voltan2_Attack04_Shot1",
        )

    for stage_id, action_id, occurrence_id, times in (
        (
            "STEP_02",
            "valtan.sequence.whirlwind.step-02",
            "valtan.sequence.whirlwind.step-02.clip-01",
            (0, 420),
        ),
        (
            "STEP_03",
            "valtan.sequence.whirlwind.step-03",
            "valtan.sequence.whirlwind.step-03.clip-01",
            (350, 700, 1050),
        ),
    ):
        event = (
            "G_Voltan2_Attack15_Shot2"
            if stage_id == "STEP_02"
            else "G_Voltan2_Attack06_Shot1"
        )
        for start_ms in times:
            add(
                f"sequence-whirlwind.{stage_id.lower()}.impact-{start_ms}",
                "VALTAN_SEQUENCE_WHIRLWIND",
                stage_id,
                action_id,
                occurrence_id,
                start_ms,
                event,
            )

    landing_occurrences = (
        (
            "six-pizza", "VALTAN_SIX_PIZZA_106", "STEP_03",
            "valtan.sequence.center-six-pizza-charge.step-03",
            "valtan.sequence.center-six-pizza-charge.step-03.clip-01",
        ),
        (
            "terrain-destruction-3", "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK", "IMPACT",
            "valtan.mechanic.terrain-destruction-3.impact",
            "valtan.mechanic.terrain-destruction-3.impact.clip-01",
        ),
        (
            "terrain-destruction-9", "VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK", "IMPACT",
            "valtan.mechanic.terrain-destruction-9.impact",
            "valtan.mechanic.terrain-destruction-9.impact.clip-01",
        ),
    )
    for suffix, pattern_id, stage_id, action_id, occurrence_id in landing_occurrences:
        add(
            f"{suffix}.landing-impact",
            pattern_id,
            stage_id,
            action_id,
            occurrence_id,
            267,
            "G_Voltan2_Attack09_Shot1",
        )

    for event in ("G_Voltan2_Attack18_Shot7", "G_Voltan2_Attack18_ShotVox4"):
        add(
            f"six-pizza.step-05.{event.rsplit('_', 1)[-1].lower()}",
            "VALTAN_SIX_PIZZA_106",
            "STEP_05",
            "valtan.sequence.center-six-pizza-charge.step-05",
            "valtan.sequence.center-six-pizza-charge.step-05.clip-01",
            1300,
            event,
        )
    for start_ms in (150, 700, 1150):
        add(
            f"six-pizza.step-11.impact-{start_ms}",
            "VALTAN_SIX_PIZZA_106",
            "STEP_11",
            "valtan.sequence.center-six-pizza-charge.step-11",
            "valtan.sequence.center-six-pizza-charge.step-11.clip-01",
            start_ms,
            "G_Voltan2_Attack18_Shot5",
        )

    # Existing Product hits whose extracted impact rows fall outside one Server
    # tick. startMs remains raw source time; the authored hit stays stage-wall
    # time. The slowed Whirlwind and sliced Dash Charge occurrences therefore
    # use the inverse occurrence transform here.
    for source_ms in (0, 155, 311, 466):
        add(
            f"whirlwind.spin.impact-{source_ms}",
            "VALTAN_WHIRLWIND",
            "SPIN",
            "valtan.attack.whirlwind.active",
            "valtan.attack.whirlwind.active.clip.01",
            source_ms,
            "G_Voltan2_Attack15_Shot2",
        )
    add(
        "dash-charge.charge.impact-0",
        "VALTAN_DASH_CHARGE",
        "CHARGE",
        "valtan.attack.dash-charge.active",
        "valtan.attack.dash-charge.active.clip.01",
        2450,
        "G_Voltan2_Attack04_Shot1",
    )

    for start_ms, event in (
        (1790, "G_Voltan2_Attack08_Shot1"),
        (2560, "G_Voltan2_Attack08_Shot1"),
        (3330, "G_Voltan2_Attack08_Shot2"),
    ):
        add(
            f"four-slash.slashes.impact-{start_ms}",
            "VALTAN_FOUR_SLASH",
            "SLASHES",
            "valtan.attack.triple-slash.active",
            "valtan.attack.four-slash.active.clip.01",
            start_ms,
            event,
        )
    add(
        "four-slash.spin.impact-600",
        "VALTAN_FOUR_SLASH",
        "SPIN",
        "valtan.attack.rotation-slash.active",
        "valtan.attack.four-slash.active.clip.02",
        600,
        "G_Voltan2_Attack08_Shot3",
    )
    add(
        "high-jump.land.impact-900",
        "VALTAN_HIGH_JUMP",
        "LAND",
        "valtan.attack.high-jump.land",
        "valtan.attack.high-jump.land.clip.01",
        900,
        "G_Voltan2_Attack09_Shot1",
    )
    add(
        "arena-break-109.impact.impact-0",
        "VALTAN_ARENA_BREAK_109",
        "IMPACT",
        "valtan.mechanic.arena-break-109.impact",
        "valtan.mechanic.arena-break-109.impact.clip.01",
        400,
        "G_Voltan2_Attack09_Shot1",
    )
    add(
        "stagger-slot.final-attack.impact-2900",
        "VALTAN_STAGGER_SLOT",
        "FINAL_ATTACK",
        "valtan.authoring.stagger-slot.final-attack",
        "VALTAN_STAGGER_SLOT.FINAL_ATTACK.composition.clip.01",
        2900,
        "G_Voltan2_Attack25_Shot2",
    )
    for pattern_id, action_prefix in (
        ("VALTAN_TRASH", "valtan.sequence.center-trash-rush-if"),
        ("VALTAN_TRASH_CATCH_IF", "valtan.sequence.rush-if"),
        ("VALTAN_TRASH_CATCH_SUCCESS", "valtan.sequence.rush-success"),
    ):
        for stage_id, action_suffix in (
            ("CATCH_SLAM", "catch-slam"),
            ("EXECUTE_TAIL", "execute-tail"),
        ):
            action_id = f"{action_prefix}.{action_suffix}"
            add(
                f"{pattern_id.lower().replace('_', '-')}.{action_suffix}.enter",
                pattern_id,
                stage_id,
                action_id,
                f"{action_id}.clip-01",
                1500,
                "G_Voltan2_Attack13_Shot1",
            )
    add(
        "attack-whirlwind.step-01.impact-1800",
        "VALTAN_ATTACK_WHIRLWIND",
        "STEP_01",
        "valtan.sequence.attack-whirlwind.step-01",
        "valtan.sequence.attack-whirlwind.step-01.clip-01",
        1800,
        "G_Voltan2_Attack19_Shot1",
    )
    for start_ms in (0, 420):
        add(
            f"attack-whirlwind.step-03.impact-{start_ms}",
            "VALTAN_ATTACK_WHIRLWIND",
            "STEP_03",
            "valtan.sequence.attack-whirlwind.step-03",
            "valtan.sequence.attack-whirlwind.step-03.clip-01",
            start_ms,
            "G_Voltan2_Attack15_Shot2",
        )
    add(
        "charge.step-03.impact-200",
        "VALTAN_CHARGE",
        "STEP_03",
        "valtan.sequence.charge.step-03",
        "valtan.sequence.charge.step-03.clip-01",
        200,
        "G_Voltan2_Attack06_Shot1",
    )
    add(
        "terrain-destruction.step-10.impact-400",
        "VALTAN_TERRAIN_DESTRUCTION",
        "STEP_10",
        "valtan.sequence.jump-whirlwind-roar-roar-charge.step-10",
        "valtan.sequence.jump-whirlwind-roar-roar-charge.step-10.clip-01",
        400,
        "G_Voltan2_Attack18_Shot7",
    )
    add(
        "catch-breath.step-02.capture-impact-250",
        "VALTAN_CATCH_BREATH",
        "STEP_02",
        "valtan.sequence.catch-breath.step-02",
        "valtan.sequence.catch-breath.step-02.clip-01",
        250,
        "G_Voltan2_Attack13_Shot1",
    )
    for start_ms in (250, 900):
        add(
            f"charge-2.step-03.impact-{start_ms}",
            "VALTAN_CHARGE_2",
            "STEP_03",
            "valtan.sequence.charge2.step-03",
            "valtan.sequence.charge2.step-03.clip-01",
            start_ms,
            "G_Voltan2_Attack06_Shot2",
        )

    result = {cue["bindingId"]: cue for cue in cues}
    if len(result) != len(cues):
        raise RuntimeError("project-authored semantic Sound cue IDs are not unique")
    return result


PROJECT_AUTHORED_SEMANTIC_CUES = _build_project_authored_semantic_cues()


def parse_sound_rows(path: Path) -> list[tuple[str, int, str]]:
    """Returns (clip, startMs, payload) for every real SOUND row, src=orig only."""
    rows: list[tuple[str, int, str]] = []
    text = path.read_text(encoding="utf-8")
    for line in text.splitlines():
        line = line.strip()
        if not line or " SOUND " not in line or "src=orig" not in line:
            continue
        match = SOUND_ROW_RE.match(line)
        if not match:
            continue
        startms_match = STARTMS_RE.search(match.group("fields"))
        payload_match = PAYLOAD_RE.search(match.group("fields"))
        if not startms_match or not payload_match or not payload_match.group(1):
            continue
        rows.append((match.group("clip"), int(startms_match.group(1)), payload_match.group(1)))
    return rows


def split_bank_event(payload: str) -> tuple[str, str]:
    bank, _, event = payload.partition(".")
    return bank, event if event else bank


def build_clip_index(patternbindings: dict) -> dict[str, list[dict]]:
    """clip string -> list of {actionId, clipOccurrenceId, sourceStartMs, playMs}.
    One binding's clip can theoretically repeat across bindings, so this is a
    list, not a single hit. sourceStartMs/playMs carry each clip occurrence's
    own segment window within the raw clip -- some occurrences (e.g. a
    "...project-tuned.prep-repeat..." variant) only cover a slice of the full
    clip's raw timeline, not the whole thing, so a SOUND row's clip-local
    startMs only belongs to occurrences whose window actually contains it."""
    index: dict[str, list[dict]] = defaultdict(list)
    for binding in patternbindings.get("bindings", []):
        action_id = binding.get("actionId")
        for clip in binding.get("clips", []):
            clip_name = clip.get("clip")
            if not action_id or not clip_name:
                continue
            index[clip_name].append({
                "actionId": action_id,
                "clipOccurrenceId": clip.get("clipOccurrenceId"),
                "sourceStartMs": clip.get("sourceStartMs", 0),
                "playMs": clip.get("playMs", 0),
            })
    return index


def build_action_index(encounter: dict) -> dict[str, dict]:
    """actionId -> {patternId, stageId, durationMs} (stage actionIds are unique --
    each stage belongs to exactly one pattern)."""
    index: dict[str, dict] = {}
    for pattern in encounter.get("patterns", []):
        pattern_id = pattern.get("patternId")
        for stage in pattern.get("stages", []):
            action_id = stage.get("actionId")
            if not pattern_id or not action_id:
                continue
            index[action_id] = {
                "patternId": pattern_id,
                "stageId": stage.get("stageId"),
                "durationMs": stage.get("durationMs", 0),
            }
    return index


def build_cues(rows: list[tuple[str, int, str]],
               clip_index: dict[str, list[dict]],
               action_index: dict[str, dict]) -> tuple[list[dict], dict[str, int]]:
    cues: list[dict] = []
    occurrence_counters: dict[tuple[str, str], int] = defaultdict(int)
    stats = {
        "totalSoundRows": len(rows),
        "skippedBank": 0,
        "skippedUnavailableEvent": 0,
        "unmatchedClip": 0,
        "unmatchedAction": 0,
        "outsideClipSegment": 0,
        "matched": 0,
    }

    for clip, start_ms, payload in rows:
        bank, event = split_bank_event(payload)
        if bank.upper() in SKIPPED_BANKS:
            stats["skippedBank"] += 1
            continue
        if event in UNAVAILABLE_RUNTIME_EVENTS:
            stats["skippedUnavailableEvent"] += 1
            continue

        bindings = clip_index.get(clip)
        if not bindings:
            stats["unmatchedClip"] += 1
            continue

        for binding in bindings:
            action_id = binding["actionId"]
            clip_occurrence_id = binding["clipOccurrenceId"]
            action_entry = action_index.get(action_id)
            if not action_entry:
                stats["unmatchedAction"] += 1
                continue

            # Same window check CValtanPatternSoundCueDocument::Parse_Text performs
            # at load time -- filtered here too so a row that only belongs to one
            # of several occurrences sharing this clip name doesn't get attached
            # to occurrences whose own declared segment doesn't contain it (e.g. a
            # "...project-tuned.prep-repeat..." slice occurrence of the same clip).
            source_start_ms = binding["sourceStartMs"]
            play_ms = binding["playMs"]
            if start_ms < source_start_ms or (
                play_ms != 0 and start_ms >= source_start_ms + play_ms
            ):
                stats["outsideClipSegment"] += 1
                continue

            key = (action_id, clip_occurrence_id)
            occurrence_counters[key] += 1
            occurrence_index = occurrence_counters[key]

            # clip_occurrence_id in full, not just its last dot-segment: two
            # clip occurrences on the same action can share a last segment
            # (e.g. "...windup.clip.01" vs "...windup.project-tuned.prep-repeat.clip.01"),
            # which previously collapsed to the same bindingId and got rejected
            # as a duplicate by CValtanPatternSoundCueDocument's uniqueness check.
            binding_id = f"cue.sound.{clip_occurrence_id}.{occurrence_index:02d}"
            cues.append({
                "bindingId": binding_id,
                "occurrenceId": f"{binding_id}.occurrence.01",
                "patternId": action_entry["patternId"],
                "stageId": action_entry["stageId"],
                "actionId": action_id,
                "clipOccurrenceId": clip_occurrence_id,
                "soundBank": bank,
                "soundEvent": event,
                "repeatPolicy": "once",
                "startMs": start_ms,
            })
            stats["matched"] += 1

    return cues, stats


def main() -> int:
    rows = parse_sound_rows(ANIMEVENTS_PATH)
    patternbindings = json.loads(PATTERNBINDINGS_PATH.read_text(encoding="utf-8"))
    encounter = json.loads(ENCOUNTER_PATH.read_text(encoding="utf-8"))

    clip_index = build_clip_index(patternbindings)
    action_index = build_action_index(encounter)

    cues, stats = build_cues(rows, clip_index, action_index)
    native_ids = {cue["bindingId"] for cue in cues}
    semantic_ids = set(PROJECT_AUTHORED_SEMANTIC_CUES)
    duplicate_ids = native_ids & semantic_ids
    if duplicate_ids:
        raise RuntimeError(
            "semantic Sound cue duplicates an extracted identity: "
            f"{sorted(duplicate_ids)}"
        )
    cues.extend(dict(cue) for cue in PROJECT_AUTHORED_SEMANTIC_CUES.values())
    cues.sort(key=lambda c: (c["actionId"], c["clipOccurrenceId"], c["startMs"], c["occurrenceId"]))

    document = {
        "schema": "lostark.valtan-pattern-sound-cues",
        "formatVersion": 1,
        "ownerArchetypeId": "BOSS_VALTAN",
        "cues": cues,
    }

    OUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    OUT_PATH.write_text(
        json.dumps(document, ensure_ascii=False, indent=1) + "\n",
        encoding="utf-8",
    )

    print(f"Total SOUND rows in Valtan.animevents: {stats['totalSoundRows']}")
    print(f"Skipped (S_BGM_CommanderRaid/S_Systems bank, out of scope): {stats['skippedBank']}")
    print(f"Skipped (missing from runtime Sound catalog): {stats['skippedUnavailableEvent']}")
    print(f"Unmatched (no patternbindings clip): {stats['unmatchedClip']}")
    print(f"Unmatched (clip matched but no encounter stage actionId): {stats['unmatchedAction']}")
    print(f"Skipped (startMs outside this occurrence's own clip segment): {stats['outsideClipSegment']}")
    print(f"Matched cue rows written: {stats['matched']}")
    print(f"Wrote {OUT_PATH}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Build the Valtan pattern/action -> clip -> source effect-cue binding document.

This is the boss-side equivalent of a character's ``<Asset>.skillbindings.json``.
A character reaches its effects through
``(class, inputSlot) -> skillId -> ordered clips -> effectref animevent``.
Valtan has no input slot: the Server owns ``patternId`` and ``actionId``, so the
same route is rebuilt from three documents that already exist:

  ValtanEncounter.json          patternId -> gameplay actionId + sourceActionIds
  Valtan.patternbindings.json   gameplay actionId -> clip
  <profile>.action-effects.json numeric actionId -> stages -> clips + notifies

Nothing is inferred from a name. Only actionIds the encounter declares and only
clips the pattern bindings declare are emitted, and an action bound to several
patterns keeps that multiplicity instead of being collapsed to one.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from collections import Counter
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[2]
EXTERNAL_SOURCE_ROOT = (ROOT.parent / "Resource_LostArk").resolve()

ENCOUNTER_PATH = ROOT / "Data/Encounters/Valtan/ValtanEncounter.json"
PATTERN_BINDINGS_PATH = (
    ROOT / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
)
EFFECT_CATALOG_PATH = (
    ROOT / "Data/Effects/Imported/Valtan/Valtan.effect-resource-catalog.json"
)
OUTPUT_PATH = ROOT / "Data/Animation/Authored/Valtan/Valtan.actionbindings.json"
RECEIPT_PATH = (
    ROOT / "Data/Animation/Authored/Valtan/Valtan.actionbindings.receipt.json"
)

# Notify types that carry a visual effect payload. `Effect` is the same notify a
# character clip projects as `effectref=asset`; the rest are the boss-side
# particle/decal/trail carriers observed in the source extraction.
EFFECT_NOTIFY_TYPES = frozenset({
    "Effect",
    "PlayParticleEffect",
    "PlayDecalEffect",
    "DefaultParticle",
    "Trails",
    "TrailGhostEffect",
})


class BindingError(RuntimeError):
    pass


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def source_action_documents(catalog: dict[str, Any]) -> list[dict[str, Any]]:
    documents = catalog.get("sourceActionDocuments")
    if not isinstance(documents, list) or not documents:
        raise BindingError("Valtan effect catalog has no sourceActionDocuments")
    return documents


def load_source_actions(
    catalog: dict[str, Any],
) -> tuple[dict[int, list[dict[str, Any]]], list[dict[str, str]]]:
    """Index every source action object by its numeric actionId."""
    by_action: dict[int, list[dict[str, Any]]] = {}
    inputs: list[dict[str, str]] = []
    for descriptor in source_action_documents(catalog):
        raw_path = str(descriptor.get("path") or "")
        if not raw_path:
            raise BindingError("sourceActionDocument entry has no path")
        path = Path(raw_path)
        if not path.is_absolute():
            path = (ROOT / path).resolve()
        if not path.is_file():
            raise BindingError(f"source action document is missing: {path}")
        recorded = str(descriptor.get("sha256") or "")
        actual = sha256_file(path)
        if recorded and recorded != actual:
            raise BindingError(
                f"source action document drifted: {path}\n"
                f"  catalog {recorded}\n  actual  {actual}"
            )
        inputs.append({"path": path.as_posix(), "sha256": actual})
        document = load_json(path)
        profile_id = str(document.get("profileId") or descriptor.get("profileId") or "")
        for action in document.get("actions", []):
            if not isinstance(action, dict):
                continue
            try:
                action_id = int(action.get("actionId"))
            except (TypeError, ValueError):
                continue
            entry = dict(action)
            entry["profileId"] = profile_id
            by_action.setdefault(action_id, []).append(entry)
    return by_action, inputs


def effect_cues(stage: dict[str, Any]) -> list[dict[str, Any]]:
    cues = []
    for notify in stage.get("notifies", []) or []:
        if not isinstance(notify, dict):
            continue
        source_type = str(notify.get("sourceType") or "")
        if source_type not in EFFECT_NOTIFY_TYPES:
            continue
        cues.append({
            "notifyId": str(notify.get("notifyId") or ""),
            "sourceType": source_type,
            "category": str(notify.get("category") or ""),
            "resolutionStatus": str(notify.get("resolutionStatus") or ""),
            "localTimeSeconds": float(notify.get("localTimeSeconds") or 0.0),
            "durationSeconds": float(notify.get("durationSeconds") or 0.0),
            "assetReferences": list(notify.get("assetReferences") or []),
        })
    return cues


def build_document() -> tuple[dict[str, Any], dict[str, Any]]:
    encounter = load_json(ENCOUNTER_PATH)
    pattern_bindings = load_json(PATTERN_BINDINGS_PATH)
    catalog = load_json(EFFECT_CATALOG_PATH)

    if encounter.get("bossArchetypeId") != "BOSS_VALTAN":
        raise BindingError("encounter bossArchetypeId is not BOSS_VALTAN")
    if pattern_bindings.get("schema") != "lostark.valtan-pattern-bindings":
        raise BindingError("pattern binding schema drifted")

    clip_by_action: dict[str, str] = {}
    for row in pattern_bindings.get("bindings", []):
        action_id = str(row.get("actionId") or "")
        clip = str(row.get("clip") or "")
        if not action_id or not clip:
            raise BindingError(f"pattern binding row is incomplete: {row}")
        if action_id in clip_by_action and clip_by_action[action_id] != clip:
            raise BindingError(f"pattern binding declares two clips: {action_id}")
        clip_by_action[action_id] = clip

    source_actions, inputs = load_source_actions(catalog)

    patterns_out: list[dict[str, Any]] = []
    referenced_actions: set[int] = set()
    missing_source_actions: list[dict[str, Any]] = []
    cue_total = 0
    clip_total = 0

    for pattern in encounter.get("patterns", []):
        pattern_id = str(pattern.get("patternId") or "")
        gameplay_action = str(pattern.get("actionId") or "")
        if not pattern_id or not gameplay_action:
            raise BindingError(f"encounter pattern is incomplete: {pattern}")

        # Stages come only from the authored pattern bindings, never from a name
        # guess: an actionId prefixed by this pattern's gameplay action.
        stages = []
        prefix = gameplay_action + "."
        for action_id, clip in sorted(clip_by_action.items()):
            if not action_id.startswith(prefix):
                continue
            stages.append({
                "semanticStageId": action_id[len(prefix):],
                "gameplayActionId": action_id,
                "clip": clip,
            })
        clip_total += len(stages)

        source_rows = []
        for raw in pattern.get("sourceActionIds", []) or []:
            try:
                source_action_id = int(raw)
            except (TypeError, ValueError):
                raise BindingError(
                    f"{pattern_id} has a non-numeric sourceActionId: {raw!r}"
                )
            referenced_actions.add(source_action_id)
            candidates = source_actions.get(source_action_id)
            if not candidates:
                missing_source_actions.append({
                    "patternId": pattern_id,
                    "sourceActionId": source_action_id,
                })
                continue
            for action in candidates:
                stage_rows = []
                for stage in action.get("stages", []) or []:
                    cues = effect_cues(stage)
                    clips = [
                        str(row.get("clipName") or row.get("clip") or "")
                        for row in (stage.get("animationClips") or [])
                        if isinstance(row, dict)
                    ]
                    clips = [value for value in clips if value]
                    if not cues and not clips:
                        continue
                    cue_total += len(cues)
                    stage_rows.append({
                        "stageIndex": int(stage.get("stageIndex") or 0),
                        "stageName": str(stage.get("stageName") or ""),
                        "animationClips": clips,
                        "effectCues": cues,
                    })
                source_rows.append({
                    "sourceActionId": source_action_id,
                    "profileId": str(action.get("profileId") or ""),
                    "sourceDisplayName": str(action.get("displayName") or ""),
                    "stages": stage_rows,
                })

        patterns_out.append({
            "patternId": pattern_id,
            "displayName": str(pattern.get("displayName") or ""),
            "gameplayActionId": gameplay_action,
            "sourceActionIds": [int(v) for v in (pattern.get("sourceActionIds") or [])],
            "stages": stages,
            "sourceActions": source_rows,
        })

    unmapped = sorted(set(source_actions) - referenced_actions)

    document = {
        "schema": "lostark.valtan-action-effect-bindings",
        "formatVersion": 1,
        "bossArchetypeId": "BOSS_VALTAN",
        "route": (
            "encounter patternId -> gameplay actionId -> authored clip; "
            "source numeric actionId -> stage -> effect notify. "
            "Gameplay authority stays with the Server encounter document."
        ),
        "patterns": patterns_out,
        # Source actions the extraction found that no product pattern claims:
        # hit/groggy reactions and unused variants. Kept visible so they are not
        # silently promoted into the product surface.
        "unmappedSourceActionIds": unmapped,
    }

    cue_types: Counter[str] = Counter()
    for pattern in patterns_out:
        for action in pattern["sourceActions"]:
            for stage in action["stages"]:
                for cue in stage["effectCues"]:
                    cue_types[cue["sourceType"]] += 1

    receipt = {
        "schema": "lostark.valtan-action-effect-bindings-receipt",
        "formatVersion": 1,
        "bossArchetypeId": "BOSS_VALTAN",
        "summary": {
            "patternCount": len(patterns_out),
            "patternsWithStages": sum(1 for p in patterns_out if p["stages"]),
            "patternsWithSourceActions": sum(
                1 for p in patterns_out if p["sourceActions"]
            ),
            "authoredStageCount": clip_total,
            "sourceActionReferencedCount": len(referenced_actions),
            "sourceActionIndexedCount": len(source_actions),
            "unmappedSourceActionCount": len(unmapped),
            "missingSourceActionCount": len(missing_source_actions),
            "effectCueCount": cue_total,
            "effectCueTypeCounts": dict(sorted(cue_types.items())),
        },
        "missingSourceActions": missing_source_actions,
        "sources": [
            {"path": p.relative_to(ROOT).as_posix(), "sha256": sha256_file(p)}
            for p in (ENCOUNTER_PATH, PATTERN_BINDINGS_PATH, EFFECT_CATALOG_PATH)
        ] + inputs,
    }
    return document, receipt


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--dry-run", action="store_true")
    mode.add_argument("--write", action="store_true")
    args = parser.parse_args()

    document, receipt = build_document()
    if args.write:
        OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)
        OUTPUT_PATH.write_text(
            json.dumps(document, ensure_ascii=False, indent=1) + "\n",
            encoding="utf-8",
        )
        RECEIPT_PATH.write_text(
            json.dumps(receipt, ensure_ascii=False, indent=1) + "\n",
            encoding="utf-8",
        )
    label = "written" if args.write else "dry-run"
    print(f"Valtan action bindings {label}: "
          f"{json.dumps(receipt['summary'], ensure_ascii=False)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

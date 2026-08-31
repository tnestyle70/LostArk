#!/usr/bin/env python3
"""Focused Replace Stage Slots occurrence-identity/dependency contract."""

from __future__ import annotations

import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
WORKBENCH_CPP = ROOT / "Client/Private/ActionCompositionWorkbench.cpp"
BALANCE_CPP = ROOT / "Client/Private/BalanceTool.cpp"
ANIMATION_CPP = ROOT / "Client/Private/Animation_Tool.cpp"


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for cursor in range(opening, len(source)):
        if source[cursor] == "{":
            depth += 1
        elif source[cursor] == "}":
            depth -= 1
            if depth == 0:
                return source[opening : cursor + 1]
    raise AssertionError(f"unterminated function: {signature}")


def replace_occurrences(
    pattern_id: str,
    stage_id: str,
    existing: list[dict[str, str]],
    selected_clips: list[str],
) -> list[dict[str, str]]:
    """Executable mirror of the bounded identity-selection rule."""

    reused = [False] * len(existing)
    reserved = {row["id"] for row in existing}
    result: list[dict[str, str]] = []
    for selected_index, selected_clip in enumerate(selected_clips):
        reusable: int | None = None
        if (
            selected_index < len(existing)
            and not reused[selected_index]
            and existing[selected_index]["clip"] == selected_clip
        ):
            reusable = selected_index
        else:
            reusable = next(
                (
                    index
                    for index, row in enumerate(existing)
                    if not reused[index] and row["clip"] == selected_clip
                ),
                None,
            )
        if reusable is not None:
            reused[reusable] = True
            result.append(
                {
                    "id": existing[reusable]["id"],
                    "clip": selected_clip,
                    "mapping": existing[reusable]["mapping"],
                }
            )
            continue

        ordinal = 1
        while True:
            candidate = (
                f"{pattern_id}.{stage_id}.composition.clip.{ordinal:02d}"
            )
            if candidate not in reserved:
                break
            ordinal += 1
        reserved.add(candidate)
        result.append(
            {
                "id": candidate,
                "clip": selected_clip,
                "mapping": "PROJECT_AUTHORED",
            }
        )
    return result


def dependencies_resolve(
    candidate: list[dict[str, str]], dependencies: list[tuple[str, str]]
) -> bool:
    joined = {(row["id"], row["clip"]) for row in candidate}
    return all(dependency in joined for dependency in dependencies)


class ActionCompositionSequenceIdentityContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.workbench = WORKBENCH_CPP.read_text(encoding="utf-8")
        cls.balance = BALANCE_CPP.read_text(encoding="utf-8")
        cls.animation = ANIMATION_CPP.read_text(encoding="utf-8")

    def test_replace_preserves_only_same_clip_identity_and_reserves_removed_ids(
        self,
    ) -> None:
        apply = function_body(
            self.workbench,
            "bool_t Client::CActionCompositionWorkbench::Apply_SelectedSequenceToStage(",
        )
        for token in (
            "ReservedSlots =\n\t\tDraft.animationSlots",
            "ReusedExistingSlots(iExistingCount, false)",
            "Draft.animationSlots[iClip].clip ==",
            "Draft.animationSlots[iExisting].clip ==",
            "BuildNextCompositionSlotId(\n\t\t\t\tPattern.strPatternId, Stage.strStageId, ReservedSlots)",
            'Slot.mappingBasis = "PROJECT_AUTHORED"',
            "ReservedSlots.push_back(Slot)",
        ):
            self.assertIn(token, apply)
        self.assertNotIn("if (!bAppend && iClip < iExistingCount)", apply)

        existing = [
            {"id": "P.S.composition.clip.01", "clip": "A", "mapping": "BASE"},
            {"id": "P.S.composition.clip.02", "clip": "B", "mapping": "REVIEW"},
        ]
        replaced = replace_occurrences("P", "S", existing, ["B", "C"])
        self.assertEqual(
            [
                {"id": "P.S.composition.clip.02", "clip": "B", "mapping": "REVIEW"},
                {
                    "id": "P.S.composition.clip.03",
                    "clip": "C",
                    "mapping": "PROJECT_AUTHORED",
                },
            ],
            replaced,
        )
        self.assertFalse(dependencies_resolve(replaced, [(existing[0]["id"], "A")]))
        self.assertTrue(dependencies_resolve(replaced, [(existing[1]["id"], "B")]))

    def test_same_clip_reorder_and_duplicate_occurrences_keep_distinct_ids(self) -> None:
        existing = [
            {"id": "P.S.composition.clip.01", "clip": "A", "mapping": "M1"},
            {"id": "P.S.composition.clip.02", "clip": "B", "mapping": "M2"},
            {"id": "P.S.composition.clip.03", "clip": "A", "mapping": "M3"},
        ]
        replaced = replace_occurrences("P", "S", existing, ["B", "A", "A", "C"])
        self.assertEqual(
            [
                "P.S.composition.clip.02",
                "P.S.composition.clip.01",
                "P.S.composition.clip.03",
                "P.S.composition.clip.04",
            ],
            [row["id"] for row in replaced],
        )
        self.assertEqual(len(replaced), len({row["id"] for row in replaced}))

    def test_effect_sound_and_shake_dependencies_fail_before_stage_commit(self) -> None:
        wrapper = function_body(
            self.workbench,
            "bool_t SetValtanStageDraftWithSoundDependencyAdmission(",
        )
        self.assertLess(
            wrapper.index("Validate_ValtanCompositionPatternSoundStageDependencies"),
            wrapper.index("Pattern Shake row would dangle"),
        )
        self.assertLess(
            wrapper.index("Pattern Shake row would dangle"),
            wrapper.rindex("Set_ValtanStageDraft"),
        )
        for token in (
            "bAnimationDependencyChanged",
            "the Pattern Shake dependency source is unavailable",
            "1u != iBaselineCount || 1u != iCandidateCount",
            "BaselineClip->strClipName != CandidateClip->strClipName",
            "retarget or remove it explicitly",
        ):
            self.assertIn(token, wrapper)

        balance_setter = function_body(
            self.balance,
            "bool Client::CBalanceTool::Set_ValtanStageDraft(",
        )
        for token in (
            "for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& cue : current.productCues)",
            "OccurrenceIds.end() == OccurrenceIds.find(cue.strClipOccurrenceId)",
            '" still references removed slot "',
        ):
            self.assertIn(token, balance_setter)

        sound_validator = function_body(
            self.animation,
            "Validate_ValtanCompositionPatternSoundStageDependencies(",
        )
        for token in (
            "1u != iBaselineClipCount || 1u != iCandidateClipCount",
            "BaselineClip->strClipName != CandidateClip->strClipName",
            "remove or retarget the Sound row explicitly first",
        ):
            self.assertIn(token, sound_validator)


if __name__ == "__main__":
    unittest.main()

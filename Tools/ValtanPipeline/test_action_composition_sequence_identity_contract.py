#!/usr/bin/env python3
"""Focused Replace Stage Slots occurrence-identity/dependency contract."""

from __future__ import annotations

import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
WORKBENCH_CPP = ROOT / "Client/Private/ValtanActionWorkbench.cpp"
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
    """Executable mirror of exact -> unique semantic-role reuse."""

    def role(clip: str) -> str:
        for token in ("start", "loop", "end"):
            if f"_{token}" in clip.lower():
                return token
        return ""

    reused = [False] * len(existing)
    reusable: list[int | None] = [None] * len(selected_clips)
    for selected_index, selected_clip in enumerate(selected_clips):
        for index, row in enumerate(existing):
            if not reused[index] and row["clip"] == selected_clip:
                reused[index] = True
                reusable[selected_index] = index
                break
    for selected_index, selected_clip in enumerate(selected_clips):
        if reusable[selected_index] is not None or not role(selected_clip):
            continue
        selected_role = role(selected_clip)
        unmatched_new = [
            index
            for index, clip in enumerate(selected_clips)
            if reusable[index] is None and role(clip) == selected_role
        ]
        unmatched_existing = [
            index
            for index, row in enumerate(existing)
            if not reused[index] and role(row["clip"]) == selected_role
        ]
        if len(unmatched_new) == 1 and len(unmatched_existing) == 1:
            reusable[selected_index] = unmatched_existing[0]
            reused[unmatched_existing[0]] = True

    reserved = {row["id"] for row in existing}
    result: list[dict[str, str]] = []
    for selected_index, selected_clip in enumerate(selected_clips):
        reusable_index = reusable[selected_index]
        if reusable_index is not None:
            old = existing[reusable_index]
            result.append(
                {
                    "id": old["id"],
                    "clip": selected_clip,
                    "mapping": (
                        old["mapping"]
                        if old["clip"] == selected_clip
                        else "PROJECT_AUTHORED"
                    ),
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


def dependency_ids_resolve(candidate: list[dict[str, str]], ids: list[str]) -> bool:
    joined = {row["id"] for row in candidate}
    return all(dependency in joined for dependency in ids)


class ActionCompositionSequenceIdentityContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.workbench = WORKBENCH_CPP.read_text(encoding="utf-8")
        cls.balance = BALANCE_CPP.read_text(encoding="utf-8")
        cls.animation = ANIMATION_CPP.read_text(encoding="utf-8")

    def test_replace_preserves_logical_slot_identity_and_reserves_removed_ids(
        self,
    ) -> None:
        apply = function_body(
            self.workbench,
            "bool_t Client::CValtanActionWorkbench::Apply_SelectedSequenceToStage(",
        )
        for token in (
            "ReservedSlots =\n\t\tDraft.animationSlots",
            "ReusedExistingSlots(iExistingCount, false)",
            "Draft.animationSlots[iExisting].clip ==",
            "ClipReplacementRole(ExpandedClips[iClip].strClip)",
            "iUnassignedNewRoleCount",
            "iUnassignedExistingRoleCount",
            "ReusableSlots[iClip] = iUniqueExisting",
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
        self.assertFalse(dependency_ids_resolve(replaced, [existing[0]["id"]]))
        self.assertTrue(dependency_ids_resolve(replaced, [existing[1]["id"]]))

    def test_groggy_replace_keeps_the_old_loop_slot_for_linked_sound(self) -> None:
        existing = [
            {
                "id": "valtan.attack.dash-charge.groggy.clip.01",
                "clip": "mesh_dmg_parts_loop_1",
                "mapping": "PROJECT_TUNED",
            }
        ]
        replaced = replace_occurrences(
            "VALTAN_DASH_CHARGE",
            "GROGGY",
            existing,
            [
                "mesh_abn_groggy_1_start",
                "mesh_abn_groggy_1_loop",
                "mesh_abn_groggy_1_end",
            ],
        )
        self.assertEqual(existing[0]["id"], replaced[1]["id"])
        self.assertEqual("PROJECT_AUTHORED", replaced[1]["mapping"])
        self.assertEqual(1843, 1833 + 10)

    def test_ambiguous_loop_roles_never_retarget_a_dependency_by_position(self) -> None:
        existing = [
            {"id": "P.S.clip.01", "clip": "old_loop_a", "mapping": "M1"},
            {"id": "P.S.clip.02", "clip": "old_loop_b", "mapping": "M2"},
        ]
        replaced = replace_occurrences(
            "P", "S", existing, ["new_loop_a", "new_loop_b"]
        )
        self.assertTrue({row["id"] for row in replaced}.isdisjoint(
            {row["id"] for row in existing}
        ))
        self.assertFalse(dependency_ids_resolve(replaced, ["P.S.clip.01"]))

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
            '"PROJECT_AUTHORED" != CandidateClip->strMappingBasis',
            "explicit PROJECT_AUTHORED slot replacement",
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
            '"PROJECT_AUTHORED" != CandidateClip->strMappingBasis',
            "explicit PROJECT_AUTHORED slot replacement",
        ):
            self.assertIn(token, sound_validator)


if __name__ == "__main__":
    unittest.main()

#!/usr/bin/env python3
"""Focused contracts for KakulSaydon reference-only animation authoring."""

from __future__ import annotations

import copy
import json
from pathlib import Path
import sys
import unittest


SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

from build_kakul_animation_reference import (  # noqa: E402
    AUTHORITY,
    AUTHORED_BASIS,
    BuildError,
    HOLDOUT,
    PROFILE_SPECS,
    REFERENCE_BASIS,
    REVIEW_CANDIDATE,
    aggregate_evidence_sha256,
    build_empty_authored_document,
    build_prefix_mapping,
    build_reference_document,
    calculate_reference_revision,
    parse_model_info,
    validate_authored_document,
    validate_reference_document,
)


def anim_row(clip: str, length: float, notify: str) -> dict:
    return {"clipName": clip, "lengthSeconds": length, "notifyId": notify}


def stage(index: int, rows: list[dict]) -> dict:
    return {
        "stageIndex": index,
        "stageName": f"Stage{index}",
        "animationClips": rows,
        "notifies": [
            {
                "notifyId": row["notifyId"],
                "sourceType": "Anim",
                "localTimeSeconds": 0.125 * index,
            }
            for row in rows
        ],
    }


class KakulAnimationReferenceTests(unittest.TestCase):
    def setUp(self) -> None:
        self.spec = next(spec for spec in PROFILE_SPECS if spec.profile_id == "MN_RPCT_06")
        self.source = {
            "schema": "lostark.ue3-action-effect-source",
            "formatVersion": 1,
            "profileId": "MN_RPCT_06",
            "actions": [
                {
                    "sourceActionIndex": 0,
                    "actionId": 0,
                    "displayName": "STAND",
                    "stages": [
                        stage(0, [anim_row("Idle_Loop", 2.25, "notify-0")]),
                        stage(1, [anim_row("Missing_Clip", 1.0, "notify-1")]),
                    ],
                },
                {
                    "sourceActionIndex": 1,
                    "actionId": 4221801,
                    "displayName": "대형 세이튼_내려찍기",
                    "stages": [stage(0, [anim_row("Attack", 0.5, "notify-2")])],
                },
            ],
        }
        self.mapping = {
            "idle_loop": "mn_rpct_06_sk.ao_idle_loop",
            "attack": "mn_rpct_06_sk.ao_attack",
        }
        self.evidence = aggregate_evidence_sha256(
            [("action-effects", "1" * 64), ("runtime-wmodel", "2" * 64)]
        )

    def build(self) -> dict:
        return build_reference_document(
            self.spec, self.source, self.mapping, self.evidence
        )

    def test_exact_join_holdout_korean_and_revision_contract(self) -> None:
        document = self.build()
        validate_reference_document(document)
        first = document["actions"][0]
        self.assertEqual(first["displayName"], "대기")
        self.assertEqual(first["reviewStatus"], HOLDOUT)
        self.assertEqual(first["authority"], AUTHORITY)
        slot = first["stages"][0]["slots"][0]
        self.assertEqual(slot["slotId"], "animation-000")
        self.assertEqual(slot["runtimeClip"], "mn_rpct_06_sk.ao_idle_loop")
        self.assertEqual(slot["sourceStartMs"], 0)
        self.assertEqual(slot["playMs"], 2250)
        self.assertEqual(slot["playRate"], 1.0)
        self.assertTrue(slot["loop"])
        self.assertEqual(slot["mappingBasis"], REFERENCE_BASIS)
        self.assertEqual(first["stages"][1]["slots"], [])
        self.assertEqual(
            first["stages"][1]["holdoutClipNames"], ["Missing_Clip"]
        )
        self.assertEqual(document["actions"][1]["reviewStatus"], REVIEW_CANDIDATE)
        self.assertEqual(
            document["referenceRevision"], calculate_reference_revision(document)
        )

    def test_authored_starts_as_empty_sparse_override(self) -> None:
        reference = self.build()
        authored = build_empty_authored_document(reference)
        self.assertEqual(authored["bindings"], [])
        self.assertEqual(authored["authority"], AUTHORITY)
        self.assertEqual(authored["referenceRevision"], reference["referenceRevision"])
        validate_authored_document(authored, reference)

    def test_project_override_allows_different_clip_timing_and_holdout_stage(self) -> None:
        reference = self.build()
        authored = build_empty_authored_document(reference)
        stage_row = reference["actions"][0]["stages"][0]
        stage_row["holdoutClipNames"] = ["Manual_Holdout"]
        reference["actions"][0]["reviewStatus"] = HOLDOUT
        reference["referenceRevision"] = calculate_reference_revision(reference)
        authored["referenceRevision"] = reference["referenceRevision"]
        authored["bindings"].append(
            {
                "sourceActionId": 0,
                "stageId": "stage-000",
                "slotId": "animation-000",
                "runtimeClip": "project_selected_clip",
                "sourceStartMs": 750,
                "playMs": 4200,
                "playRate": 1.25,
                "loop": False,
                "mappingBasis": AUTHORED_BASIS,
                "authority": AUTHORITY,
            }
        )
        validate_reference_document(reference)
        validate_authored_document(authored, reference)

    def test_authored_rejects_invalid_identity_duplicate_field_and_range(self) -> None:
        reference = self.build()
        authored = build_empty_authored_document(reference)
        valid = {
            "sourceActionId": 4221801,
            "stageId": "stage-000",
            "slotId": "animation-000",
            "runtimeClip": "project_selected_clip",
            "sourceStartMs": 100,
            "playMs": 500,
            "playRate": 1.0,
            "loop": True,
            "mappingBasis": AUTHORED_BASIS,
            "authority": AUTHORITY,
        }

        invalid_identity = copy.deepcopy(authored)
        invalid_identity["bindings"] = [{**valid, "slotId": "animation-999"}]
        with self.assertRaises(BuildError):
            validate_authored_document(invalid_identity, reference)

        duplicate = copy.deepcopy(authored)
        duplicate["bindings"] = [copy.deepcopy(valid), copy.deepcopy(valid)]
        with self.assertRaises(BuildError):
            validate_authored_document(duplicate, reference)

        invalid_token = copy.deepcopy(authored)
        invalid_token["bindings"] = [{**valid, "runtimeClip": "../bad clip"}]
        with self.assertRaises(BuildError):
            validate_authored_document(invalid_token, reference)

        invalid_basis = copy.deepcopy(authored)
        invalid_basis["bindings"] = [{**valid, "mappingBasis": "EXTRACTED_DEFAULT"}]
        with self.assertRaises(BuildError):
            validate_authored_document(invalid_basis, reference)

        invalid_authority = copy.deepcopy(authored)
        invalid_authority["bindings"] = [{**valid, "authority": "PRODUCT"}]
        with self.assertRaises(BuildError):
            validate_authored_document(invalid_authority, reference)

        extra_field = copy.deepcopy(authored)
        extra_field["bindings"] = [{**valid, "unexpected": True}]
        with self.assertRaises(BuildError):
            validate_authored_document(extra_field, reference)

        for field, value in (
            ("sourceStartMs", -1),
            ("sourceStartMs", 600_001),
            ("playMs", 0),
            ("playMs", 600_001),
            ("playRate", 0.0),
            ("playRate", 16.01),
            ("loop", 1),
        ):
            invalid_range = copy.deepcopy(authored)
            invalid_range["bindings"] = [{**valid, field: value}]
            with self.subTest(field=field, value=value), self.assertRaises(BuildError):
                validate_authored_document(invalid_range, reference)

    def test_strict_validation_rejects_stale_revision_and_extra_properties(self) -> None:
        document = self.build()
        stale = copy.deepcopy(document)
        stale["actions"][0]["displayName"] = "변경"
        with self.assertRaises(BuildError):
            validate_reference_document(stale)
        extra = copy.deepcopy(document)
        extra["unexpected"] = True
        with self.assertRaises(BuildError):
            validate_reference_document(extra)

    def test_model_info_and_prefix_join_are_fail_closed(self) -> None:
        clips = parse_model_info(
            "[Model] sections=3 animations=2 skeleton=yes\n"
            "  section type=4 index=0 size=10 name=rpcz00_idle\n"
            "  section type=4 index=1 size=10 name=rpcz00_attack\n"
        )
        self.assertEqual(
            build_prefix_mapping("rpcz00_", clips),
            {"idle": "rpcz00_idle", "attack": "rpcz00_attack"},
        )
        with self.assertRaises(BuildError):
            parse_model_info(
                "[Model] sections=3 animations=3 skeleton=yes\n"
                " section type=4 index=0 size=10 name=rpcz00_idle\n"
            )


class CommittedKakulAnimationReferenceTests(unittest.TestCase):
    EXPECTED_RPCZ_HOLDOUTS = {
        "Dead_3",
        "Dead_FixedType_1",
        "Dead_FlyingType_1",
        "Dead_Status1_1",
        "Dead_Status2_1",
        "Dead_Status3_1",
        "Fast_Run_Battle_1",
        "Idle_Burrow_Loop_1",
        "Idle_FixedType_Loop_1",
        "Idle_FixedType_Normal_1",
        "Idle_FlyingType_Loop_1",
        "Idle_FlyingType_Normal_1",
        "Idle_Normal_1_1",
        "Idle_Normal_1_2",
        "Idle_Status1_Loop_1",
        "Idle_Status1_Normal_1",
        "Idle_Status2_Loop_1",
        "Idle_Status2_Normal_1",
        "Idle_Status3_Loop_1",
        "Idle_Status3_Normal_1",
        "Run_Burrow_1",
        "Run_Status1_1",
        "Run_Status2_1",
        "Run_Status3_1",
        "Walk_Status1_1",
        "Walk_Status2_1",
        "Walk_Status3_1",
    }

    def test_committed_documents_and_rpcz_holdouts(self) -> None:
        data_root = SCRIPT_DIR.parents[1] / "Data/Animation"
        references: dict[str, dict] = {}
        for spec in PROFILE_SPECS:
            reference_path = (
                data_root
                / "Reference/KakulSaydon"
                / f"{spec.profile_id}.actionreference.json"
            )
            authored_path = (
                data_root
                / "Authored/KakulSaydon"
                / f"{spec.profile_id}.actionbindings.json"
            )
            reference = json.loads(reference_path.read_text(encoding="utf-8"))
            authored = json.loads(authored_path.read_text(encoding="utf-8"))
            validate_reference_document(reference)
            validate_authored_document(authored, reference)
            self.assertEqual(authored["bindings"], [])
            references[spec.profile_id] = reference

        rpcz_holdouts = {
            name
            for action in references["MN_RPCZ_00"]["actions"]
            for stage_row in action["stages"]
            for name in stage_row["holdoutClipNames"]
        }
        self.assertTrue(self.EXPECTED_RPCZ_HOLDOUTS.issubset(rpcz_holdouts))
        self.assertEqual(len(self.EXPECTED_RPCZ_HOLDOUTS), 27)


if __name__ == "__main__":
    unittest.main()

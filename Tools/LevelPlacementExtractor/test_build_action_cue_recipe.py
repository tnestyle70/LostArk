#!/usr/bin/env python3

from __future__ import annotations

import base64
import struct
import unittest

from build_action_cue_recipe import build_action_cue_recipe


class ActionCueRecipeTests(unittest.TestCase):
    def test_selects_lowest_base_stage_and_preserves_payload(self) -> None:
        payload = base64.b64encode(b"notify").decode("ascii")
        notify = {
            "notifyId": "action-10/stage-000/notify-000",
            "sourceType": "TrailGhostEffect",
            "category": "trail",
            "authority": "PRESENTATION",
            "localTimeSeconds": 0.25,
            "durationSeconds": 0.5,
            "assetReferences": [],
            "serializedPayload": {
                "encoding": "base64",
                "byteOffset": 1,
                "byteSize": 6,
                "sha256": "0" * 64,
                "data": payload,
            },
        }
        source = {
            "source": {"sha256": "1" * 64},
            "actions": [
                {
                    "actionId": 10,
                    "stages": [
                        {
                            "stageIndex": 0,
                            "stageName": "base",
                            "animationClips": [{"clipName": "SK_Test"}],
                            "notifies": [notify],
                        },
                        {
                            "stageIndex": 1,
                            "stageName": "variant",
                            "animationClips": [{"clipName": "SK_Test"}],
                            "notifies": [{**notify, "notifyId": "variant"}],
                        },
                    ],
                }
            ],
        }
        receipt = {
            "characterClass": "DIMENSIONMASTER",
            "skillId": 10,
            "inputSlot": "Q",
            "timeline": {
                "clips": [
                    {
                        "sequenceIndex": 0,
                        "clip": "pc_sp_m_00_sk_sk_test",
                        "offsetSeconds": 1.0,
                    }
                ],
                "events": [
                    {
                        "clip": "pc_sp_m_00_sk_sk_test",
                        "sourceType": "TrailGhostEffect",
                        "localTimeSeconds": 0.25,
                        "durationSeconds": 0.5,
                    }
                ],
            },
        }
        result = build_action_cue_recipe(source, receipt)
        self.assertTrue(result["sourceExtractionComplete"])
        self.assertEqual(result["selectedStages"][0]["selectedStageIndex"], 0)
        self.assertEqual(result["cues"][0]["runtimeChannel"], "CHARACTER_AFTERIMAGE")
        self.assertEqual(result["cues"][0]["globalTimeSeconds"], 1.25)

    def test_decodes_play_particle_enabled_header_without_using_label(self) -> None:
        raw = bytearray(96)
        signature = b"CEFActionNotify_PlayParticleEffect\x00"
        raw[: len(signature)] = signature
        raw[47] = 1
        payload = base64.b64encode(bytes(raw)).decode("ascii")
        notify = {
            "notifyId": "action-10/stage-000/notify-000",
            "sourceType": "PlayParticleEffect",
            "category": "particle",
            "authority": "PRESENTATION",
            "localTimeSeconds": 0.25,
            "durationSeconds": 0.5,
            "assetReferences": [
                {"objectPath": "FX_TEST.Par_Test"}
            ],
            "serializedLabels": ["a-label-that-is-not-a-variant"],
            "serializedPayload": {
                "encoding": "base64",
                "byteOffset": 1,
                "byteSize": len(raw),
                "sha256": "0" * 64,
                "data": payload,
            },
        }
        source = {
            "source": {"sha256": "1" * 64},
            "actions": [{
                "actionId": 10,
                "stages": [{
                    "stageIndex": 0,
                    "stageName": "base",
                    "animationClips": [{"clipName": "SK_Test"}],
                    "notifies": [notify],
                }],
            }],
        }
        receipt = {
            "characterClass": "DIMENSIONMASTER",
            "skillId": 10,
            "inputSlot": "Q",
            "timeline": {
                "clips": [{
                    "sequenceIndex": 0,
                    "clip": "pc_sp_m_00_sk_sk_test",
                    "offsetSeconds": 1.0,
                }],
                "events": [{
                    "clip": "pc_sp_m_00_sk_sk_test",
                    "sourceType": "PlayParticleEffect",
                    "sourceAsset": "FX_TEST.Par_Test",
                    "sourceSystemId": "fx_test.par_test",
                    "localTimeSeconds": 0.25,
                    "durationSeconds": 0.5,
                }],
            },
        }
        result = build_action_cue_recipe(source, receipt)
        cue = result["cues"][0]
        self.assertTrue(cue["executionEnabled"])
        self.assertEqual(cue["typedPayload"]["sourceByteOffset"], 47)
        self.assertFalse(cue["typedPayload"]["particleDataDecoded"])
        self.assertEqual(cue["sourceReceiptEventIndex"], 0)

    def test_decodes_particle_anchor_and_local_transform(self) -> None:
        raw = bytearray(512)
        signature = b"CEFActionNotify_PlayParticleEffect\x00"
        raw[: len(signature)] = signature
        raw[47] = 1
        particle_system = b"ParticleSystem'FX_TEST.Par_Test'\x00"
        reference_start = 120
        struct.pack_into("<i", raw, reference_start, len(particle_system))
        raw[reference_start + 4 : reference_start + 4 + len(particle_system)] = (
            particle_system
        )
        base = reference_start + 4 + len(particle_system)
        struct.pack_into("<i", raw, base + 52, 1)
        anchor = b"b_ROOT\x00"
        struct.pack_into("<i", raw, base + 56, len(anchor))
        raw[base + 60 : base + 60 + len(anchor)] = anchor
        transform = base + 60 + len(anchor)
        struct.pack_into("<fff", raw, transform + 20, 25.0, 0.0, 90.0)
        struct.pack_into("<fff", raw, transform + 32, 0.0, 0.0, 45.0)
        struct.pack_into("<fff", raw, transform + 80, 1.2, 1.2, 1.2)

        payload = base64.b64encode(bytes(raw)).decode("ascii")
        notify = {
            "notifyId": "action-10/stage-000/notify-000",
            "sourceType": "PlayParticleEffect",
            "category": "particle",
            "authority": "PRESENTATION",
            "localTimeSeconds": 0.25,
            "durationSeconds": 0.5,
            "assetReferences": [{"objectPath": "FX_TEST.Par_Test"}],
            "serializedPayload": {
                "encoding": "base64",
                "byteOffset": 1,
                "byteSize": len(raw),
                "sha256": "0" * 64,
                "data": payload,
            },
        }
        source = {
            "source": {"sha256": "1" * 64},
            "actions": [{
                "actionId": 10,
                "stages": [{
                    "stageIndex": 0,
                    "stageName": "base",
                    "animationClips": [{"clipName": "SK_Test"}],
                    "notifies": [notify],
                }],
            }],
        }
        receipt = {
            "characterClass": "DIMENSIONMASTER",
            "skillId": 10,
            "inputSlot": "Q",
            "timeline": {
                "clips": [{
                    "sequenceIndex": 0,
                    "clip": "pc_sp_m_00_sk_sk_test",
                    "offsetSeconds": 1.0,
                }],
                "events": [{
                    "clip": "pc_sp_m_00_sk_sk_test",
                    "sourceType": "PlayParticleEffect",
                    "sourceAsset": "FX_TEST.Par_Test",
                    "sourceSystemId": "fx_test.par_test",
                    "localTimeSeconds": 0.25,
                    "durationSeconds": 0.5,
                }],
            },
        }
        result = build_action_cue_recipe(source, receipt)
        typed = result["cues"][0]["typedPayload"]
        self.assertTrue(typed["particleDataDecoded"])
        self.assertEqual(typed["attachment"]["runtimeAnchorSlotId"], "b_ROOT")
        self.assertEqual(typed["attachment"]["runtimeBoneName"], "b_root")
        self.assertEqual(
            typed["attachment"]["runtimeResolutionStatus"],
            "EXACT_SOURCE_BONE",
        )
        self.assertEqual(typed["localTransform"]["position"], [0.25, 0.0, 0.9])
        self.assertEqual(typed["localTransform"]["rotationDegrees"], [0.0, 0.0, 45.0])
        self.assertAlmostEqual(typed["localTransform"]["scale"][0], 1.2)


if __name__ == "__main__":
    unittest.main()

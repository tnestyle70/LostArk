import struct
import tempfile
import unittest
from pathlib import Path

from extract_action_effect_notifies import extract_action_document


def lp_ascii(value: str) -> bytes:
    payload = value.encode("ascii") + b"\0"
    return struct.pack("<i", len(payload)) + payload


def notify(source_type: str, start: float, duration: float, body: bytes = b"") -> bytes:
    marker = f"CEFActionNotify_{source_type}".encode("ascii") + b"\0"
    return (
        lp_ascii(marker[:-1].decode("ascii"))
        + b"\0" * 16
        + struct.pack("<fff", start, start, duration)
        + body
    )


class ActionEffectNotifyTests(unittest.TestCase):
    def test_extracts_action_stage_clip_particle_and_hit_reference(self) -> None:
        action = lp_ascii("CEFActionObject") + struct.pack("<ii", 0, 420621) + lp_ascii("Jump Spin")
        stage = lp_ascii("CEFActionStage") + b"\0" * 48 + lp_ascii("Landing")
        # The production reader uses fixed serializer offsets for Anim fields.
        anim_marker = lp_ascii("CEFActionNotify_Anim")
        anim_payload = b"\0" * 16 + struct.pack("<fff", 0.0, 0.0, 1.25)
        anim_padding = b"\0" * 16
        anim = (
            anim_marker
            + anim_payload
            + anim_padding
            + lp_ascii("Anim")
            + b"\0" * 4
            + lp_ascii("Att_Battle_20_01")
        )
        particle_body = lp_ascii("ParticleSystem'FX_TEST.Par_Test'")
        payload = (
            action
            + stage
            + anim
            + notify("PlayParticleEffect", 0.4, 0.2, particle_body)
            + notify(
                "PlayStaticMesh",
                0.5,
                0.3,
                lp_ascii("StaticMesh'FX_SM_00.Mesh.FM_Test'"),
            )
            + notify("ParticleHit", 0.45, 0.1)
        )

        with tempfile.TemporaryDirectory() as temporary:
            source = Path(temporary) / "Action.loa"
            source.write_bytes(payload)
            document = extract_action_document(source, "TEST")

        self.assertEqual(document["summary"]["selectedActionObjectCount"], 1)
        self.assertEqual(document["summary"]["selectedStageCount"], 1)
        self.assertEqual(document["summary"]["selectedAnimationClipOccurrenceCount"], 1)
        self.assertEqual(document["summary"]["uniqueParticleSystemCount"], 1)
        self.assertEqual(document["summary"]["uniqueDirectMeshCount"], 1)
        self.assertEqual(
            "FX_SM_00.Mesh.FM_Test", document["meshes"][0]["sourceAsset"]
        )
        parsed_stage = document["actions"][0]["stages"][0]
        self.assertEqual(parsed_stage["stageName"], "Landing")
        self.assertEqual(parsed_stage["animationClips"][0]["clipName"], "Att_Battle_20_01")
        particle = next(
            row for row in parsed_stage["notifies"]
            if row["sourceType"] == "PlayParticleEffect"
        )
        self.assertAlmostEqual(particle["localTimeSeconds"], 0.4, places=6)
        self.assertEqual(particle["assetReferences"][0]["objectPath"], "FX_TEST.Par_Test")
        hit = next(row for row in parsed_stage["notifies"] if row["sourceType"] == "ParticleHit")
        self.assertEqual(hit["authority"], "REFERENCE_ONLY")


if __name__ == "__main__":
    unittest.main()

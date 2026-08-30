import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


class BernEntranceCameraContractTests(unittest.TestCase):
    def test_authored_cue_is_bounded_and_stable(self) -> None:
        document = json.loads(read(
            "Data/Encounters/Bern/BernEntranceCamera.json"))
        self.assertEqual("lostark.level-entrance-camera", document["schema"])
        self.assertEqual(1, document["formatVersion"])
        self.assertEqual("BERN", document["levelId"])
        self.assertEqual("PROJECT_AUTHORED", document["provenance"])

        cue = document["cue"]
        self.assertEqual("bern.entrance", cue["cueId"])
        self.assertGreater(cue["durationMs"], 0)
        self.assertIn(cue["interpolation"], {"LINEAR", "CATMULL_ROM"})
        self.assertGreaterEqual(len(cue["keyframes"]), 2)
        self.assertEqual(0, cue["keyframes"][0]["timeMs"])
        self.assertEqual(cue["durationMs"], cue["keyframes"][-1]["timeMs"])
        self.assertEqual(
            len(cue["keyframes"]),
            len({row["sceneId"] for row in cue["keyframes"]}),
        )

    def test_level_uses_shared_product_sampler_and_fail_closed_load(self) -> None:
        source = read("Client/Private/Level_Bern.cpp")
        header = read("Client/Public/Level_Bern.h")
        for token in (
            "Ready_EntranceCinematic()",
            "Update_EntranceCinematic(fTimeDelta)",
            "Parse_BernEntranceCamera",
            'L"Encounters/Bern/BernEntranceCamera.json"',
            "CCamera::PRESENTATION_PRIORITY::SERVER_CINEMATIC",
            "CValtanCinematicCameraController::Sample_Cue",
            "Get_DIKeyState(DIK_ESCAPE)",
            "End_EntranceCinematic()",
            "End_PresentationOverride",
            "Set_FollowTarget(m_pEntranceRestoreTarget.lock())",
            "Set_FollowEnabled(m_bEntranceRestoreFollowRequested)",
        ):
            self.assertIn(token, source)
        self.assertIn("m_hasEntranceCameraCue", header)
        self.assertIn("m_bEntranceCinematicDone", header)
        cinematic = source[
            source.index("bool_t CLevel_Bern::Ready_EntranceCinematic"):
            source.index("HRESULT CLevel_Bern::Render()")
        ]
        self.assertNotIn("CNetworkManager", cinematic)

    def test_project_exposes_single_source_document(self) -> None:
        entry = r"..\..\Data\Encounters\Bern\BernEntranceCamera.json"
        project = read("Client/Default/Client.vcxproj")
        filters = read("Client/Default/Client.vcxproj.filters")
        self.assertEqual(1, project.count(entry))
        self.assertEqual(1, filters.count(entry))
        self.assertIn("96.DataFiles\\Encounters", filters)


if __name__ == "__main__":
    unittest.main()

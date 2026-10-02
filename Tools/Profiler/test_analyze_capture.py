import copy
import unittest

from analyze_capture import analyze, interval_union


class CaptureAnalysisTests(unittest.TestCase):
    def fixture(self):
        return {"schema": "LostArkProfilerCapture.v3", "ticksPerSecond": 1000,
                "mainThreadId": 7, "scopeNames": ["Client.Render", "Render.Draw"],
                "frames": [
                    {"frameNumber": 10, "frameIntervalMs": 0, "cpuFrameMs": 10,
                     "cpuScopes": [{"nameId": 0, "threadId": 7, "beginTick": 0, "endTick": 10},
                                   {"nameId": 1, "threadId": 7, "beginTick": 2, "endTick": 7}],
                     "gpuValid": True, "gpuScopesSupported": True, "gpuFrameMs": 12,
                     "gpuScopes": [{"nameId": 0, "beginMs": 1, "endMs": 8, "durationMs": 7},
                                   {"nameId": 1, "beginMs": 2, "endMs": 6, "durationMs": 4}]},
                    {"frameNumber": 11, "frameIntervalMs": 20, "cpuFrameMs": 30,
                     "droppedCpuScopes": 9, "detailedCpuScopes": True,
                     "cpuScopes": [], "gpuValid": False, "gpuFrameMs": 0}]}

    def test_pending_reset_and_nested_intervals(self):
        result = analyze(self.fixture())
        self.assertEqual(result["frameInterval"]["samples"], 1)
        self.assertEqual(result["fpsFromMeanInterval"], 50)
        self.assertEqual(result["gpuFrameValidOnly"]["meanMs"], 12)
        self.assertEqual(result["gpuScopeUnion"]["meanMs"], 7)
        self.assertEqual(result["cpuScopes"][0]["inclusiveMsPerFrame"], 5)
        self.assertEqual(result["quality"]["partialCpuFrames"], 1)
        self.assertEqual(result["quality"]["windowDroppedCpuScopes"], 9)
        self.assertNotIn("selfMs", result["cpuScopes"][0])

    def test_selected_window_does_not_include_other_frames(self):
        result = analyze(self.fixture(), 10, 10)
        self.assertIsNone(result["fpsFromMeanInterval"])
        self.assertEqual(result["quality"]["windowDroppedCpuScopes"], 0)
        self.assertEqual(result["cpuScopes"][0]["inclusiveMsPerFrame"], 10)
        self.assertEqual(result["selection"]["sourceFrames"], 2)
        self.assertEqual(result["selection"]["frames"], 1)

    def test_partial_gpu_pass_excluded_from_pass_denominator(self):
        document = self.fixture()
        document["frames"][0]["droppedGpuScopes"] = 1
        result = analyze(document)
        self.assertEqual(result["gpuFrameValidOnly"]["samples"], 1)
        self.assertEqual(result["gpuPassSampleFrames"], 0)
        self.assertEqual(result["gpuScopes"], [])

    def test_invalid_data_rejected(self):
        for field, value in [("cpuFrameMs", float("nan")), ("frameIntervalMs", -1)]:
            document = copy.deepcopy(self.fixture())
            document["frames"][0][field] = value
            with self.assertRaises(ValueError):
                analyze(document)
        document = self.fixture()
        document["frames"][0]["cpuScopes"][0]["endTick"] = -1
        with self.assertRaises(ValueError):
            analyze(document)
        with self.assertRaises(ValueError):
            analyze(self.fixture(), 100)

    def test_overlapping_gpu_scopes_count_elapsed_once(self):
        self.assertEqual(interval_union([(1, 4), (2, 7), (8, 9), (1, 2)]), 7)

    def test_inconsistent_gpu_status_rejected(self):
        document = self.fixture()
        document["frames"][0]["gpuStatus"] = "pending"
        with self.assertRaises(ValueError):
            analyze(document)
        document["frames"][0]["gpuStatus"] = "valid"
        self.assertEqual(analyze(document)["gpuFrameValidOnly"]["samples"], 1)
        document["frames"][1]["gpuStatus"] = "valid"
        with self.assertRaises(ValueError):
            analyze(document)


if __name__ == "__main__":
    unittest.main()

from __future__ import annotations

import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
VALTAN_H = ROOT / "Client" / "Public" / "Valtan.h"
VALTAN_CPP = ROOT / "Client" / "Private" / "Valtan.cpp"


class ValtanCounterProxyDebugMirrorSourceContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.header = VALTAN_H.read_text(encoding="utf-8")
        cls.source = VALTAN_CPP.read_text(encoding="utf-8")

    def test_counter_proxy_has_separate_debug_identity_and_pose(self) -> None:
        for token in (
            "bHasCounterProxy",
            "fCounterProxyForwardOffsetM",
            "fCounterProxyRightOffsetM",
            "fCounterProxyRadiusM",
        ):
            self.assertIn(token, self.header)
            self.assertIn(token, self.source)

    def test_counter_proxy_is_not_gated_by_damage_pulse_clock(self) -> None:
        self.assertIn(
            "if (!isHitWindow && !isAuthoringGeometryWindow &&", self.source
        )
        self.assertIn("!area.bHasCounterProxy)\n\t\treturn;", self.source)
        self.assertIn("COUNTER_PROXY_COLOR_RGBA", self.source)
        self.assertIn("CounterShape.iAreaType = 1", self.source)
        self.assertIn("area.fCounterProxyForwardOffsetM", self.source)
        self.assertIn("area.fCounterProxyRightOffsetM", self.source)

    def test_server_and_local_preview_load_the_same_proxy_fields(self) -> None:
        self.assertIn("stage.bHasCounterProxy", self.source)
        self.assertIn("Stage.CounterProxy.has_value()", self.source)
        self.assertIn("Stage.CounterProxy->fForwardOffsetM", self.source)
        self.assertIn("Stage.CounterProxy->fRightOffsetM", self.source)
        self.assertIn("Stage.CounterProxy->fRadiusM", self.source)


if __name__ == "__main__":
    unittest.main()

#!/usr/bin/env python3

from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from extract_ue3_skeletal_mesh_sockets import parse_socket_contract


class SkeletalMeshSocketTests(unittest.TestCase):
    def test_compact_umodel_socket_summary_uses_identity_defaults(self) -> None:
        text = """Sockets[2] =
    {
        Sockets[0] = { Name=effectroot, Bone=b_effectroot }
        Sockets[1] = { Name=wp_sdm_r_battle, Bone=b_wp_1 }
    }
"""
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "pc_sp_00_sk.props.txt"
            source.write_text(text, encoding="utf-8")
            contract = parse_socket_contract(source)
        self.assertEqual(len(contract["sockets"]), 2)
        battle = contract["sockets"][1]
        self.assertEqual(battle["socketName"], "wp_sdm_r_battle")
        self.assertEqual(battle["boneName"], "b_wp_1")
        self.assertEqual(
            battle["runtimeLocalTransform"],
            {
                "position": [0.0, 0.0, 0.0],
                "rotationDegrees": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            },
        )
        self.assertEqual(
            battle["transformEvidence"],
            "UMODEL_COMPACT_DEFAULT_TRANSFORM",
        )


if __name__ == "__main__":
    unittest.main()

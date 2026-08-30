import json
import math
import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


class ValtanRaidCapacityContractTests(unittest.TestCase):
    def test_party_and_raid_capacities_are_distinct(self) -> None:
        packet_type = (ROOT / "Shared/Public/Network/PacketType.h").read_text(
            encoding="utf-8", errors="replace"
        )
        self.assertRegex(packet_type, r"MAX_PARTY_MEMBERS\s*=\s*4\s*;")
        self.assertRegex(packet_type, r"MAX_VALTAN_RAID_PLAYERS\s*=\s*8\s*;")

        room = (ROOT / "Server/Private/GameRoom.cpp").read_text(
            encoding="utf-8", errors="replace"
        )
        self.assertIn("m_Players.size() >= LostArk::Shared::MAX_VALTAN_RAID_PLAYERS", room)
        self.assertIn("return nullptr == Find_AvailablePlayerSpawn();", room)

        publisher = (
            ROOT / "Tools/WorldPipeline/Publish-WorldGameplay.ps1"
        ).read_text(encoding="utf-8", errors="replace")
        self.assertRegex(publisher, r"\$valtanRaidPlayerCapacity\s*=\s*8")
        self.assertIn(
            "$enabledPlayerSpawnCount -ne $valtanRaidPlayerCapacity", publisher
        )

    def test_valtan_has_eight_unique_nav_valid_spawn_slots(self) -> None:
        world = json.loads(
            (ROOT / "Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json").read_text(
                encoding="utf-8-sig"
            )
        )
        spawns = [
            placement
            for placement in world["placements"]
            if placement.get("kind") == "playerSpawn" and placement.get("enabled")
        ]
        self.assertEqual(8, len(spawns))
        self.assertEqual(8, len({placement["placementId"] for placement in spawns}))
        self.assertEqual(
            {f"player_{index}" for index in range(1, 9)},
            {placement["placementId"] for placement in spawns},
        )

        nav_path = ROOT / "Data/Navigation/LV_LUT_HEARTRB_ED.navsource"
        with nav_path.open("r", encoding="utf-8") as stream:
            header = stream.readline().strip()
            match = re.match(
                r'^LOSTARK_NAVGRID_SOURCE\s+\d+\s+"[^"]+"\s+'
                r"(\d+)\s+(\d+)\s+([-+0-9.eE]+)\s+"
                r"([-+0-9.eE]+)\s+([-+0-9.eE]+)",
                header,
            )
            self.assertIsNotNone(match)
            width = int(match.group(1))
            height = int(match.group(2))
            cell_size = float(match.group(3))
            origin_x = float(match.group(4))
            origin_z = float(match.group(5))
            cells = {}
            for line in stream:
                fields = line.split()
                if len(fields) != 5:
                    continue
                column, row = int(fields[0]), int(fields[1])
                cells[(column, row)] = (
                    int(fields[2]),
                    int(fields[3]),
                    float(fields[4]),
                )

        for spawn in spawns:
            x, y, z = spawn["position"]
            column = math.floor((x - origin_x) / cell_size)
            row = math.floor((z - origin_z) / cell_size)
            self.assertGreaterEqual(column, 0)
            self.assertLess(column, width)
            self.assertGreaterEqual(row, 0)
            self.assertLess(row, height)
            self.assertIn((column, row), cells)
            source_present, walkable, nav_y = cells[(column, row)]
            self.assertEqual(1, source_present, spawn["placementId"])
            self.assertEqual(1, walkable, spawn["placementId"])
            self.assertLessEqual(abs(float(y) - nav_y), 0.5, spawn["placementId"])

        collision_boxes = [
            placement
            for placement in world["placements"]
            if placement.get("kind") == "collisionBox"
            and placement.get("enabled")
        ]
        for spawn in spawns:
            x, y, z = (float(value) for value in spawn["position"])
            for box in collision_boxes:
                half_x, half_y, half_z = (
                    float(value) for value in box["halfExtents"]
                )
                box_x, box_y, box_z = (
                    float(value) for value in box["position"]
                )
                if abs((y + 0.90) - box_y) > half_y + 0.90:
                    continue
                yaw = math.radians(float(box["yawDegrees"]))
                delta_x, delta_z = x - box_x, z - box_z
                local_x = math.cos(yaw) * delta_x - math.sin(yaw) * delta_z
                local_z = math.sin(yaw) * delta_x + math.cos(yaw) * delta_z
                overlaps = (
                    abs(local_x) <= half_x + 0.45
                    and abs(local_z) <= half_z + 0.45
                )
                self.assertFalse(
                    overlaps,
                    f"{spawn['placementId']} overlaps {box['placementId']}",
                )

    def test_room_full_regression_targets_ninth_entry(self) -> None:
        server_tests = (
            ROOT / "Server/Private/ServerGameplayContractTests.cpp"
        ).read_text(encoding="utf-8", errors="replace")
        self.assertIn("NinthRaidFixture", server_tests)
        self.assertIn("Reject a ninth Valtan admission", server_tests)
        self.assertIn('find("enabledPlayerSpawns=8")', server_tests)
        self.assertIn("firstEightJoined = raidRoom.Join", server_tests)
        self.assertIn("replacementJoined = releasedSlotAvailable", server_tests)
        self.assertIn("raidRoom.Join(1010u, replacementEntry)", server_tests)
        self.assertIn(
            "Join eight Valtan players through the real admission path",
            server_tests,
        )
        self.assertIn(
            "MAX_VALTAN_RAID_PLAYERS == static_cast<std::size_t>(std::count_if(",
            server_tests,
        )
        self.assertNotIn(
            "Load exactly four enabled Valtan player spawns", server_tests
        )

        lobby = (ROOT / "Client/Private/Level_Lobby.cpp").read_text(
            encoding="utf-8", errors="replace"
        )
        self.assertIn(
            "Valtan human player slots are full. Lobby remains active.",
            lobby,
        )


if __name__ == "__main__":
    unittest.main()

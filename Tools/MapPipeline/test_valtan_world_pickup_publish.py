"""Map/Server pickup pair validation and atomic publication contracts."""
import copy
import json
import struct
import subprocess
import unittest

from test_map_effect_presentation_contract import Fixture, AREA_ID, EFFECT_ID, POWERSHELL, PUBLISHER


class WorldPickupPublishTests(unittest.TestCase):
    def setUp(self):
        self.fixture = Fixture()
        self.addCleanup(self.fixture.close)
        f = self.fixture
        self.row = dict(independentEffectId="ether.01", displayName="Ether 1",
                        presentationKind="EFFECT_DOCUMENT", placementId="world.ether.01",
                        effectAssetId=EFFECT_ID, position=[1.5, 5, 1.5],
                        rotationQuaternion=[0, 0, 0, 1], scale=[1, 1, 1],
                        orientationPolicy="WORLD", activationPolicy="SERVER_PICKUP",
                        activationSetId="", activationWindows=[], playbackPolicy="SOURCE_LOOP",
                        pickup=dict(wallGroupId="wall.01", landingPosition=[1.5, .4, 1.5],
                                    fallDurationMs=900, pickupRadiusM=.75))
        self.document = dict(schema="lostark.map-effect-presentation", formatVersion=1,
                             areaId=AREA_ID, presentations=[self.row])
        f.write_json(f.map_effect_path, self.document)
        events = f.read_json(f.world_events_path)
        events['groups'].append(dict(groupId='wall.01', navPolarity='BLOCK_WHILE_INTACT'))
        events['mutations'].append(dict(mutationId='break.wall.01', groupId='wall.01'))
        events['bindings'].append(dict(mutationId='break.wall.01', enabled=True))
        f.write_json(f.world_events_path, events)
        nav = f.root / f'Client/Bin/DataFiles/Navigation/{AREA_ID}.navgrid'
        nav.parent.mkdir(parents=True)
        nav.write_bytes(struct.pack('<IIfff', 4, 4, 1, 0, 0) + bytes([1]*16) + struct.pack('<16f', *([0]*16)))
        self.server = f.root / 'Server/Bin/DataFiles/World/VALTAN_ARENA.worldpickupsbootstrap'
        self.client = f.runtime_map / f'{AREA_ID}.mapeffects.json'

    def publish(self, *extra):
        return subprocess.run([POWERSHELL, '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File',
            str(PUBLISHER), '-AreaId', AREA_ID, '-Scope', 'Effects', '-ProjectRoot',
            str(self.fixture.root), *extra], capture_output=True, text=True,
            encoding='utf-8', errors='replace', timeout=30)

    def test_client_server_pair_and_check(self):
        result = self.publish()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(json.loads(self.client.read_text()), self.document)
        self.assertIn('"world.ether.01" "wall.01" 1.5 5 1.5 1.5 0.4 1.5 27 0.75', self.server.read_text())
        self.assertEqual(self.publish('-Mode', 'Check').returncode, 0)

    def test_reject_invalid_fields_without_mutating_pair(self):
        self.assertEqual(self.publish().returncode, 0)
        before = self.client.read_bytes(), self.server.read_bytes()
        cases = [('wallGroupId', 'missing'), ('landingPosition', [1.5, 9, 1.5]),
                 ('landingPosition', [90, .4, 90]), ('fallDurationMs', 0),
                 ('fallDurationMs', 1.5), ('pickupRadiusM', -1), ('pickupRadiusM', True)]
        for key, value in cases:
            with self.subTest(key=key, value=value):
                doc = copy.deepcopy(self.document)
                doc['presentations'][0]['pickup'][key] = value
                self.fixture.write_json(self.fixture.map_effect_path, doc)
                result = self.publish()
                self.assertNotEqual(result.returncode, 0, result.stdout)
                self.assertEqual(before, (self.client.read_bytes(), self.server.read_bytes()))

    def test_rollback_after_each_destination(self):
        self.assertEqual(self.publish().returncode, 0)
        before = self.client.read_bytes(), self.server.read_bytes()
        doc = copy.deepcopy(self.document)
        doc['presentations'][0]['pickup']['fallDurationMs'] = 1200
        self.fixture.write_json(self.fixture.map_effect_path, doc)
        for count in [1, 2]:
            result = self.publish('-FailureAfterPromote', str(count))
            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(before, (self.client.read_bytes(), self.server.read_bytes()))

    def test_validate_writes_nothing_and_version15_is_supported(self):
        authored = self.fixture.read_json(self.fixture.authored_effect_path)
        authored['version'] = 15
        self.fixture.write_json(self.fixture.authored_effect_path, authored)
        before = self.fixture.snapshot_runtime()
        result = self.publish('-Mode', 'Validate')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(before, self.fixture.snapshot_runtime())
        self.assertFalse(self.server.exists())


if __name__ == '__main__':
    unittest.main()

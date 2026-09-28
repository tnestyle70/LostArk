"""Published Waterpang entry data, preservation and exact mesh bake guards."""
import json
from pathlib import Path
import struct
import unittest
from configure_maharaka_waterpang_entry import ROOT, AREA, STAGE, OUT, prepare


class EntryContract(unittest.TestCase):
    def test_installer_is_idempotent_and_preserves_unrelated_rows(self):
        staged,_,report=prepare()
        self.assertTrue(all(before==after for before,after in staged.values()))
        current=json.loads((ROOT/f'Data/Worlds/{AREA}/Gameplay.world.json').read_text())
        before=json.loads((OUT/f'before/Data/Worlds/{AREA}/Gameplay.world.json').read_text())
        after={p['placementId']:p for p in current['placements']}
        for row in before['placements']:
            if row['placementId'] not in ('jump1','jump2','jump3'):
                self.assertEqual(row,after[row['placementId']])
            else:
                self.assertEqual(row['position'],after[row['placementId']]['position'])
        self.assertEqual(3,report['jumpPairs'])

    def test_published_holds_and_unmodified_source_camera(self):
        author=ROOT/f'Data/Maps/Authoring/{AREA}/{AREA}.worldsequences.json'
        runtime=ROOT/f'Client/Bin/DataFiles/Map/{AREA}.worldsequences.json'
        self.assertEqual(json.loads(author.read_text()),json.loads(runtime.read_text()))
        seq=json.loads(author.read_text())
        for suffix in ('mokomoko','cannon'):
            instance=next(i for i in seq['instances'] if i['instanceId']==STAGE.rsplit('.',1)[0]+'.'+suffix)
            self.assertEqual('HOLD',instance['motionEnd'])
            template=next(t for t in seq['templates'] if t['sequenceId']==instance['templateId'])
            self.assertTrue(template['tracks'][0]['keys'][-1]['visible'])
        path=f'{AREA}.camerashots.json'
        self.assertEqual(json.loads((ROOT/f'Data/Maps/Authoring/{AREA}'/path).read_text()),
                         json.loads((ROOT/'Client/Bin/DataFiles/Map'/path).read_text()))

    def test_client_server_navigation_and_landings(self):
        name=f'{AREA}.WaterpangEntry.navgrid'
        data=(ROOT/'Server/Bin/DataFiles/Navigation'/name).read_bytes()
        self.assertEqual(data,(ROOT/'Client/Bin/DataFiles/Navigation'/name).read_bytes())
        w,h,size,ox,oz=struct.unpack_from('<IIfff',data)
        world=json.loads((ROOT/f'Data/Worlds/{AREA}/Gameplay.world.json').read_text())
        rows={p['placementId']:p for p in world['placements']}
        for n in range(1,4):
            start=rows[f'jump{n}']; end=rows[f'jump{n}_1']
            self.assertTrue(start['enabled'] and start['requiresInteract'])
            self.assertFalse(end['enabled'])
            self.assertEqual(end['position'],start['events'][0]['targetPosition'])
            x,_,z=end['position']; index=int((z-oz)/size)*w+int((x-ox)/size)
            ground=struct.unpack_from('<f',data,20+w*h+4*index)[0]
            self.assertGreater(ground,22.3); self.assertLess(ground,22.5)


if __name__=='__main__': unittest.main(verbosity=2)

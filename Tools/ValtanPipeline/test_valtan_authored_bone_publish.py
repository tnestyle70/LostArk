from pathlib import Path
import sys, copy, json
ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(Path(__file__).resolve().parent))
import test_valtan_canonical_typed_patch_transaction as canonical
from Tools.ValtanPipeline import valtan_tuning_pipeline as pipeline
from Tools.ValtanPipeline import promote_valtan_animation_chains as promotion
from Tools.ValtanPipeline.valtan_native_animation_inventory import load_valtan_composite_animation_inventory
from Tools.ValtanPipeline.authored_bone_clips import SOURCE_REL, PRODUCT_REL, read_skeleton, validate_document
from Tools.GameplayPipeline.valtan_presentation_generation import build_presentation_generation
import unittest

class ValtanAuthoredBonePublishTests(unittest.TestCase):
    setUp = canonical.ValtanCanonicalTypedPatchTransactionTests.setUp
    tearDown = canonical.ValtanCanonicalTypedPatchTransactionTests.tearDown
    source_manifest = canonical.ValtanCanonicalTypedPatchTransactionTests.source_manifest
    run_pipeline = canonical.ValtanCanonicalTypedPatchTransactionTests.run_pipeline
    parse_command_result = staticmethod(canonical.ValtanCanonicalTypedPatchTransactionTests.parse_command_result)

    def test_authored_publisher_and_source_cas(self):
        r=self.root; inv=load_valtan_composite_animation_inventory(r); body=inv.sources[0].path;hashed,bones=read_skeleton(body)
        key={'timeMs':0,'position':[0,0,0],'rotation':[0,0,0,1],'scale':[1,1,1]}
        document={'schema':'lostark.authored-bone-clips','formatVersion':1,'animationAssetId':'Valtan','skeletonHash':f'{hashed:016x}','clips':[{'name':'authored.test.axe','durationMs':1000,'segments':[{'id':'source.0','sourceClip':next(iter(inv.clips)),'durationMs':1000,'sourceStartMs':0,'playRate':1,'loop':True}],'tracks':[{'bone':'b_wp_r_01','keys':[key,dict(key,timeMs=1000,rotation=[0,0,.1,.995])]}]}]}
        self.assertTrue('b_wp_r_01' in bones,'actual installed axe attachment bone')
        durations={name:row.native_duration_ms for name,row in inv.clips.items()}
        self.assertTrue(validate_document(document,body,durations)=={'authored.test.axe':1000},'actual Valtan WSKL and authored clip admission')
        for label,mutate in [('cycle',lambda d:d['clips'][0]['segments'][0].update(sourceClip='authored.test.axe')),('unknown bone',lambda d:d['clips'][0]['tracks'][0].update(bone='missing')),('zero quaternion',lambda d:d['clips'][0]['tracks'][0]['keys'][0].update(rotation=[0,0,0,0])),('duplicate key',lambda d:d['clips'][0]['tracks'][0]['keys'][1].update(timeMs=0)),('NaN',lambda d:d['clips'][0]['tracks'][0]['keys'][0].update(position=[float('nan'),0,0])),('skeleton',lambda d:d.update(skeletonHash='0'*16)),('missing dependency',lambda d:d['clips'][0]['segments'][0].update(sourceClip='authored.missing'))]:
         changed=copy.deepcopy(document);mutate(changed)
         try:validate_document(changed,body,durations)
         except ValueError:self.assertTrue(True,'reject '+label)
         else:raise AssertionError('accepted '+label)
        baseline=pipeline.source_manifest(r)['sourceManifestId'];source=r/SOURCE_REL;source.write_text(json.dumps(document),encoding='utf8')
        self.assertTrue(pipeline.source_manifest(r)['sourceManifestId']!=baseline,'bone source joins CAS read set')
        self.assertTrue(load_valtan_composite_animation_inventory(r).clips['authored.test.axe'].rounded_native_duration_ms==1000,'authored stable ID joins strict source inventory')
        projection=r/'Intermediate/ValtanProductProjection/bone-test';projection.mkdir(parents=True)
        result=pipeline.stage_repository_product_projection(r,projection)
        self.assertTrue(PRODUCT_REL in {row['path'] for row in result['files']},'projection includes product bone document')
        self.assertTrue((projection/PRODUCT_REL).read_bytes()==source.read_bytes(),'product preserves validated source bytes')
        commit=promotion.commit_projected_products(r,projection,result['sourceManifestId'])
        self.assertTrue((r/PRODUCT_REL).read_bytes()==source.read_bytes(),'existing durable transaction commits new product')
        self.assertTrue(commit['artifactCount']==10,'optional product included in exact commit closure')
        generation=build_presentation_generation(r)
        self.assertTrue(any(a.path==PRODUCT_REL and a.lane=='ANIMATION' for a in generation.artifacts),'generation hash covers product bone clips')
        # Replace one ordinary pattern clip with an equal-duration authored clip. Same
        # Server stage clock and same publisher, no new local-combat runtime.
        path=r/pipeline.PRESENTATION_AUTHORING_REL;pres=json.loads(path.read_text(encoding='utf8'));pat=next(p for p in pres['patterns'] if p['patternId']=='VALTAN_FOUR_SLASH');occ=pat['stages'][0]['animation']['occurrences'][0]
        orig=occ['clip'];duration=inv.clips[orig].rounded_native_duration_ms
        document['clips'][0].update(durationMs=duration);document['clips'][0]['segments'][0].update(sourceClip=orig,durationMs=duration,loop=False);document['clips'][0]['tracks'][0]['keys'][1]['timeMs']=duration
        source.write_text(json.dumps(document),encoding='utf8');occ['clip']='authored.test.axe';path.write_text(json.dumps(pres),encoding='utf8')
        try:
         _,_,outputs=pipeline.build_repository_product_projection(r)
         self.assertTrue('authored.test.axe' in outputs[pipeline.BINDINGS_REL],'authored clip referenced by real pattern publishes through existing owner')
        except Exception as e:raise AssertionError('pattern projection: '+str(e))
        # A prepared projection cannot overwrite a newer bone edit.
        projection2=r/'Intermediate/ValtanProductProjection/bone-cas';projection2.mkdir();result2=pipeline.stage_repository_product_projection(r,projection2);before=(r/PRODUCT_REL).read_bytes();document['clips'][0]['tracks'][0]['keys'][1]['position'][0]=1;source.write_text(json.dumps(document),encoding='utf8')
        try:promotion.commit_projected_products(r,projection2,result2['sourceManifestId'])
        except Exception as e:self.assertTrue('changed' in str(e),'concurrent bone source edit rejects projected commit')
        else:raise AssertionError('stale projection accepted')
        self.assertTrue((r/PRODUCT_REL).read_bytes()==before,'rejected publish preserves previous product')

if __name__ == "__main__":
    unittest.main()

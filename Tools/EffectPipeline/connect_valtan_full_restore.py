"""Stage exact current-clip full restore bindings through the canonical writer.

Existing authored effects and V2 resource bodies are preserved. Replaced cue
rows are archived in the receipt; the writer checks the source revision and
the V2 resource read set before committing all presentation owners together.
"""
from pathlib import Path
import argparse,copy,json,math,sys

ROOT=Path(__file__).resolve().parents[2]
sys.path[:0]=[str(ROOT/'Tools/ValtanPipeline'),str(ROOT/'Tools/EffectToolV2')]
import valtan_tuning_pipeline as pipeline
import effect_v2_binding_pipeline as bindings
from promote_valtan_animation_chains import commit_typed_authoring_patch

def read(path):return json.loads(path.read_bytes())
def write(path,value):
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(json.dumps(value,ensure_ascii=False,indent=2,allow_nan=False)+'\n',encoding='utf8')

def main():
    parser=argparse.ArgumentParser(__doc__)
    parser.add_argument('--equivalence',type=Path,required=True)
    parser.add_argument('--stage-projection',type=Path,required=True)
    parser.add_argument('--source-action',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--commit',action='store_true')
    args=parser.parse_args();out=args.output
    manifest=pipeline.source_manifest(ROOT)
    presentation=read(ROOT/pipeline.PRESENTATION_AUTHORING_REL)
    stage_map={(p['patternId'],s['stageId']):(p,s)for p in presentation['patterns']for s in p['stages']}
    sources={(a['actionId'],s['stageIndex']):s for a in read(args.source_action)['actions']for s in a['stages']}
    ready={(r['sourceActionId'],r['sourceStageIndex']):r for r in read(args.stage_projection)
           if r['completeSourceParticleNotifies'] and r['elementCount']}
    v2path=ROOT/pipeline.EFFECT_V2_BINDINGS_REL;before=v2path.read_bytes();v2=json.loads(before)
    # Capture dependencies at initial read, before constructing the candidate.
    readset=bindings.build_resource_read_set(ROOT,v2)
    operations=[];changes=[];replaced=[];claimed=set();deferred=[]
    for row in read(args.equivalence)['resolved']:
        key=(row['canonicalSourceStage']['actionId'],row['canonicalSourceStage']['stageIndex'])
        if key not in ready:continue
        pattern,stage=stage_map[row['patternId'],row['stageId']]
        occurrence=next(o for o in stage['animation']['occurrences']if o['clipOccurrenceId']==row['occurrence']['clipOccurrenceId'])
        assert occurrence==row['occurrence'],'Current clip changed since source equivalence'
        if occurrence['sourceStartMs']:
            deferred.append(dict(patternId=row['patternId'],stageId=row['stageId'],
                clipOccurrenceId=occurrence['clipOccurrenceId'],reason='Cropped source clip requires an explicit source seek variant'))
            continue
        target=dict(patternId=pattern['patternId'],stageId=stage['stageId'],actionId=stage['actionId'])
        current=[c for c in stage['effectCues']if c.get('clipOccurrenceId')==occurrence['clipOccurrenceId']]
        asset=ready[key]['effectAssetId']
        identity='cue.valtan.full.restore.'+stage['actionId'].removeprefix('valtan.')+'.'+str(stage['animation']['occurrences'].index(occurrence)+1)
        # Notify offsets are source-stage seconds. The enclosing clip owns a
        # finite window, and CUE_END also participates in boss action teardown.
        duration=occurrence['playMs']
        if not duration:
            clips=[c for c in sources[key]['animationClips']if 'mesh_'+c['clipName'].lower()==occurrence['clip'].lower()]
            assert len(clips)==1,('No exact bounded clip duration',key,occurrence['clip'])
            duration=max(1,round(clips[0]['lengthSeconds']*1000))
        cue=dict(cueId=identity,occurrenceId=identity+'.occurrence.01',effectAssetId=asset,
            clipOccurrenceId=occurrence['clipOccurrenceId'],sourceStartMs=0,sourceEndMs=duration,
            anchorSlotId='root',followPolicy='follow',stopPolicy='cue_end',
            repeatPolicy='each_loop'if occurrence['repeatUntilStageEnd']else 'once',
            localTransform=dict(position=[0.,0.,0.],rotationDegrees=[0.,0.,0.],scale=[1.,1.,1.]),
            scalePolicy=copy.deepcopy(current[0]['scalePolicy'])if current else dict(kind='ARENA_ABSOLUTE',worldScale=[1.,1.,1.]),
            mappingBasis=occurrence['mappingBasis'])
        if current:
            cue.update(cueId=current[0]['cueId'],occurrenceId=current[0]['occurrenceId'])
            operations.append(dict(op='UPDATE_EFFECT_CUE',**target,cueId=cue['cueId'],occurrenceId=cue['occurrenceId'],cue=cue))
            for old in current[1:]:
                operations.append(dict(op='REMOVE_EFFECT_CUE',**target,**{k:old[k]for k in ('cueId','occurrenceId','effectAssetId','clipOccurrenceId')}))
        else:operations.append(dict(op='ADD_EFFECT_CUE',**target,cue=cue))
        removed=[]
        for binding in v2['bindings']:
            if binding['scope']!=target:continue
            clock=binding['clock']
            if clock['basis']=='CLIP_OCCURRENCE'and clock['clipOccurrenceId']!=occurrence['clipOccurrenceId']:continue
            if clock['basis']=='STAGE'and len(stage['animation']['occurrences'])!=1:continue
            assert binding['bindingId']not in claimed
            claimed.add(binding['bindingId']);removed.append(binding)
        replaced.extend(removed)
        changes.append(dict(**target,clipOccurrenceId=occurrence['clipOccurrenceId'],sourceActionId=key[0],
            sourceStageIndex=key[1],sourceEquivalenceBasis=row['basis'],effectAssetId=asset,
            sourceParticleNotifiesComplete=True,previousCues=current,previousV2Bindings=removed,
            cue=cue,visualStatus='USER_PENDING'))
    candidate=copy.deepcopy(v2);candidate['bindings']=[b for b in v2['bindings']if b['bindingId']not in claimed]
    candidate_readset=bindings.build_resource_read_set(ROOT,candidate)
    # Removing bindings also removes unused dependencies from the exact read
    # set. Every retained body must still equal the initially captured body.
    assert all(r in readset['resources']for r in candidate_readset['resources']), 'Referenced V2 body changed during preparation'
    assert pipeline.source_manifest(ROOT)==manifest,'Source changed during wiring preparation'
    out.mkdir(parents=True,exist_ok=True)
    (out/'v2.baseline.json').write_bytes(before);write(out/'v2.candidate.json',candidate);write(out/'v2.readset.json',candidate_readset)
    write(out/'draft.json',dict(schema=pipeline.DRAFT_PATCH_SCHEMA,formatVersion=1,
        sourceRevision=manifest['sourceManifestId'],operations=operations))
    write(out/'wiring_receipt.json',dict(currentOccurrences=len(changes),patterns=len({c['patternId']for c in changes}),
        replacedV2Bindings=len(replaced),changes=changes,deferred=deferred,originalAuthoredDocumentsPreserved=True,
        scope='Original PlayParticleEffect and Trails; direct PlayDecalEffect remains a separate source carrier',visualStatus='USER_PENDING'))
    if args.commit:
        result=commit_typed_authoring_patch(ROOT,out/'draft.json',
            effect_v2_baseline_path=out/'v2.baseline.json',effect_v2_candidate_path=out/'v2.candidate.json',
            effect_v2_read_set_path=out/'v2.readset.json')
        write(out/'commit_receipt.json',result)
    print(json.dumps(dict(occurrences=len(changes),operations=len(operations),replacedV2=len(replaced),committed=args.commit)))

if __name__=='__main__':main()

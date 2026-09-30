"""Extract each playable class's native sc_cheer_1 into an attached AnimSet.

This is the clip explicitly referenced by Colosseum interpdata_33. Exact body
skeleton bytes are preserved and body/model materials are not rewritten. Source
PSA playback rates (including28.8fps) are preserved, never normalized to30fps.
"""
import argparse,hashlib,json,subprocess,sys,tempfile
from pathlib import Path

from build_card_maze_player_animations import ROOT,SOURCES,carrier,sections,subset
from build_waterpang_watergun_player_animations import find_psa
import append_psa_clip_to_wmodel as codec


def main():
    p=argparse.ArgumentParser()
    p.add_argument('--source',type=Path,action='append',required=True)
    p.add_argument('--out',type=Path,required=True)
    p.add_argument('--install',action='store_true')
    a=p.parse_args();a.out.mkdir(parents=True,exist_ok=True)
    catalog=json.loads((ROOT/'Data/Actors/CharacterCatalog.json').read_text(encoding='utf8'))
    receipt=[]
    for actor in catalog['characters']:
        name=actor['assetId'];psa=find_psa(a.source,SOURCES[name])
        body=ROOT/'Client/Bin/Resources'/actor['bodyModel'];body_data=body.read_bytes()
        _,rows=sections(body_data)
        skeleton=next(s for s in rows if s[0]==3);donor=next(s for s in rows if s[0]==4)
        bones,info,_=codec.load_clip(psa,'sc_cheer_1')
        if len(set(bones))!=len(bones) or info['rate']<=0:raise ValueError('Invalid PSA '+str(psa))
        with tempfile.TemporaryDirectory(prefix='colosseum-cheer-') as tmp:
            staged=Path(tmp)/'donor.wmodel';staged.write_bytes(carrier(body_data,skeleton,donor))
            result=Path(tmp)/'cheer.wmodel'
            subprocess.run([sys.executable,str(Path(codec.__file__)),'--wmodel',str(staged),'--psa',str(psa),
                            '--clip','sc_cheer_1','--out',str(result)],check=True)
            all_data=result.read_bytes();_,rows=sections(all_data)
            data=subset(all_data,[s for s in rows if s[0] in (1,2,3) or s[0]==4 and s[4].split(b'\0')[0]==b'sc_cheer_1'])
        _,final_rows=sections(data);final_skel=next(s for s in final_rows if s[0]==3)
        assert data[16+final_skel[2]:16+final_skel[2]+final_skel[3]]==body_data[16+skeleton[2]:16+skeleton[2]+skeleton[3]]
        assert len([s for s in final_rows if s[0]==4])==1
        filename=name+'_ColosseumVictoryAnimSet.wmodel'
        (a.out/filename).write_bytes(data)
        asset=f'Character/{name}/AnimSets/{filename}'
        if a.install:
            dest=ROOT/'Client/Bin/Resources'/asset;dest.parent.mkdir(parents=True,exist_ok=True)
            if dest.exists() and dest.read_bytes()!=data:raise ValueError('Refusing to overwrite existing different animation set '+str(dest))
            if not dest.exists():dest.write_bytes(data)
        receipt.append({'class':name,'sourcePackage':SOURCES[name],'sourcePsa':str(psa),
                        'sourceClip':'sc_cheer_1','rate':info['rate'],'frames':info['frames'],
                        'sourceSha256':hashlib.sha256(psa.read_bytes()).hexdigest(),
                        'bodySha256':hashlib.sha256(body_data).hexdigest(),'assetId':asset,
                        'outputSha256':hashlib.sha256(data).hexdigest(),'skeletonPreserved':True})
        print(name,asset,info)
    (a.out/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n',encoding='utf8')


if __name__=='__main__':main()

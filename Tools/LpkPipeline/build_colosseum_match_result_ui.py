"""Lift the retail Colosseum Score state and result announcement timelines.

Input is the exact EFUI_COLOSSEUM.colosseumplaying_loc_int movie plus its
UModel texture pages. Geometry, transforms, colour transforms and frame timing
come from that movie, not from screenshots. The dynamic Korean title is supplied
by the runtime, as it is in the source movie. No ActionScript is executed.
"""
from __future__ import annotations

import argparse
import collections
import hashlib
import json
import math
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

from gfx_scene_extract import parse_all


def matrix(m=None):
    m = m or {}
    return np.array([[m.get('sx', 1), m.get('r1', 0), m.get('tx', 0)],
                     [m.get('r0', 0), m.get('sy', 1), m.get('ty', 0)], [0, 0, 1]], dtype=float)


def display(items, frame):
    result = {}
    for it in items:
        if it.get('frame', 1) > frame:
            continue
        if it['op'] == 'remove':
            result.pop(it['depth'], None)
        elif it['op'] == 'place':
            depth = it['depth']
            prior = result.get(depth, {}) if it.get('move') else {}
            value = dict(prior)
            value.update(it)
            value['born'] = it['frame'] if 'char' in it else prior.get('born', it['frame'])
            result[depth] = value
    return result


class Extractor:
    def __init__(self, source, tex, target):
        self.doc, _ = parse_all(source.read_bytes())
        self.pages = {p.name.lower(): p for p in tex.rglob('*.tga')}
        self.target = target
        target.mkdir(parents=True, exist_ok=True)
        self.images = {}
        self.variants = {}
        self.omitted = collections.Counter()

    def shape(self, cid):
        if cid in self.images:
            return self.images[cid]
        sh = self.doc['shapes'][cid]
        b = sh['bounds']
        w, h = max(1, math.ceil(b[1]-b[0])), max(1, math.ceil(b[3]-b[2]))
        fills = [f for f in sh['fills'] if f['type'] == 'bitmap' and f['bitmap'] in self.doc['sub']]
        if not fills:
            im=Image.new('RGBA',(w,h))
            for index,fill in enumerate(sh['fills'],1):
                if fill['type']=='solid':
                    layer=Image.new('RGBA',(w,h),tuple(fill['color']))
                elif fill['type']=='gradient':
                    yy,xx=np.mgrid[0:h,0:w]
                    points=np.stack([xx+b[0]+.5,yy+b[2]+.5,np.ones_like(xx)],axis=-1)
                    inv=np.linalg.inv(matrix(fill['matrix']))
                    xy=points@inv.T
                    # SWF gradient space is +/-16384 twips; parser translations
                    # and shape coordinates are pixels, hence +/-819.2 here.
                    ratio=(xy[:,:,0]/819.2+1)*127.5 if fill['kind']=='linear' else np.hypot(xy[:,:,0],xy[:,:,1])/819.2*255
                    stops=fill['stops']
                    pixels=np.stack([np.interp(ratio,[q['ratio'] for q in stops],[q['color'][k] for q in stops]) for k in range(4)],axis=-1)
                    layer=Image.fromarray(np.clip(pixels,0,255).astype(np.uint8))
                else:raise ValueError('Unsupported fill '+fill['type'])
                mask=Image.new('L',(w*4,h*4))
                draw=ImageDraw.Draw(mask)
                for path in sh['paths']:
                    if index not in (path['fill0'],path['fill1']) or len(path['pts'])<3:continue
                    draw.polygon([((x-b[0])*4,(y-b[2])*4) for x,y in path['pts']],fill=255)
                a=np.array(layer)
                a[:,:,3]=(a[:,:,3].astype(float)*np.array(mask.resize((w,h),Image.Resampling.LANCZOS))/255).astype(np.uint8)
                im=Image.alpha_composite(im,Image.fromarray(a))
            self.images[cid]=(im,b[0],b[2])
            return self.images[cid]
        fill = fills[-1]
        sub = self.doc['sub'][fill['bitmap']]
        page = self.doc['ext'][sub['image']]['file'].lower()
        im = Image.open(self.pages[page]).convert('RGBA').crop(tuple(sub['rect']))
        # Bitmap matrix has twips/texel scale but pixel translation. Invert it
        # to sample the exact polygon rectangle instead of stretching an atlas.
        m = matrix(fill['matrix'])
        m[:2, :2] /= 20
        origin = np.array([[1,0,-b[0]],[0,1,-b[2]],[0,0,1]])
        inv = np.linalg.inv(origin @ m)
        im = im.transform((w,h), Image.Transform.AFFINE, tuple(inv[:2].reshape(-1)), Image.Resampling.BICUBIC)
        self.images[cid] = (im, b[0], b[2])
        return self.images[cid]

    def asset(self, cid, mul, add):
        base = self.shape(cid)
        if base is None:
            return None
        rgbmul = tuple(round(float(v), 6) for v in mul[:3])
        rgbadd = tuple(round(float(v), 6) for v in add[:3])
        key = (cid, rgbmul, rgbadd)
        if key not in self.variants:
            im = base[0]
            if rgbmul != (1,1,1) or rgbadd != (0,0,0):
                a = np.array(im, dtype=np.float32)
                a[:,:,:3] = np.clip(a[:,:,:3] * np.array(rgbmul) + np.array(rgbadd), 0, 255)
                im = Image.fromarray(a.astype(np.uint8))
            suffix = hashlib.sha1(repr(key).encode()).hexdigest()[:10]
            name = f'shape_{cid}_{suffix}.png'
            im.save(self.target/name)
            self.variants[key] = 'UI/Colosseum/Result/'+name
        return self.variants[key], base[0].size, base[1:]

    def walk(self, cid, frame, mat, mul, add, path=(), blend=False):
        if cid in self.doc['sprites']:
            sp = self.doc['sprites'][cid]
            frame = (frame-1) % sp['frames']+1
            for depth, it in sorted(display(sp['items'],frame).items()):
                child = it.get('char')
                if child is None:
                    continue  # Dynamic string/class linkage belongs to the Client.
                cx = it.get('cx', {})
                cm = np.array(cx.get('mul') or [256]*4,dtype=float)/256
                ca = np.array(cx.get('add') or [0]*4,dtype=float)
                # Source parent transform acts after the child transform.
                mm, aa = mul*cm, mul*ca+add
                if it.get('filters'):
                    self.omitted['source-filters'] += 1
                yield from self.walk(child,frame-it['born']+1,mat@matrix(it.get('matrix')),mm,aa,path+(depth,),blend or it.get('blend')==8)
        elif cid in self.doc['shapes']:
            value = self.asset(cid,mul,add)
            if value is None:return
            asset, size, origin = value
            sx = math.hypot(mat[0,0],mat[1,0])
            sy = np.linalg.det(mat[:2,:2])/max(sx,1e-12)
            if sx < 1e-8 or abs(sy) < 1e-8:return
            shear = (mat[0,0]*mat[0,1]+mat[1,0]*mat[1,1])/max(sx,1e-12)
            if abs(shear)>0.002:self.omitted['source-shear']+=1
            # CUILayout keyframes rotate around the sprite centre.
            center = mat@np.array([origin[0]+size[0]/2,origin[1]+size[1]/2,1])
            rotation = math.degrees(math.atan2(mat[1,0],mat[0,0]))
            if sy < 0:
                sx,sy=-sx,-sy
                rotation+=180
            key={'x':round(float(center[0]-abs(sx)*size[0]/2),5),
                 'y':round(float(center[1]-sy*size[1]/2),5),
                 'scaleX':round(abs(sx),6),'scaleY':round(sy,6),
                 'rotationDeg':round(rotation,5),'flipX':sx<0,
                 'alpha':round(float(np.clip(mul[3]+add[3]/255,0,1)),6),
                 'asset':asset,'additive':bool(blend)}
            yield path, key

    def animation(self, cid):
        sp=self.doc['sprites'][cid]
        tracks={}
        # Keep the source root stage origin; the enclosing result group is at y160.
        for frame in range(1,sp['frames']+1):
            shown=dict(self.walk(cid,frame,np.eye(3),np.ones(4),np.zeros(4)))
            for path in set(tracks)|set(shown):
                key=shown.get(path)
                if key is None:
                    key=dict(tracks[path][-1]);key['alpha']=0
                if path not in tracks:
                    tracks[path]=[]
                    if frame>1:tracks[path].append(dict(key,frame=1,alpha=0))
                tracks[path].append(dict(key,frame=frame))
        return {'frameRate':self.doc['header']['rate'],'frameCount':sp['frames'],
                'loop':False,'labels':{'start':1},'sourceLabels':sp['labels'],
                'layers':[{'keyframes':tracks[p]} for p in sorted(tracks)]}


def slot(name,x,y,w,h,asset=None,kind=0):
    return {'id':name,'ownerClass':None,'type':kind,
            'rect':{'x':x,'y':y,'width':w,'height':h},'rotation':0,
            'stages':{'baseFrom':0,'shineFrom':1},
            'layers':[] if asset is None else [{'path':asset,'hoverPath':None,'tint':[1,1,1,1],'additive':False,'flipX':False}],
            'shine':{'texture':None,'additive':False},
            'animation':{'fps':10,'scale':1,'offset':{'x':0,'y':0},'frames':[],'loop':False,'additive':False}}


def write(path,doc):
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(json.dumps(doc,ensure_ascii=False,indent=2)+'\n',encoding='utf8')


def validate(repo):
    """Validate installed extraction and the existing consumer contracts, without writing."""
    def require(ok,message):
        if not ok:raise ValueError(message)
    def read(path):return json.loads(path.read_text(encoding='utf8'))
    def relative(root,asset):
        require(isinstance(asset,str) and asset and ':' not in asset and
                not asset.startswith(('/','\\')) and '..' not in Path(asset).parts,
                'Unsafe asset path: '+repr(asset))
        path=root/asset
        require(path.is_file(),'Missing asset '+str(path))
        return path
    resources=repo/'Client/Bin/Resources'
    data=repo/'Data/UI/Colosseum'
    images=set()
    for name,required in [('MatchHUD',{'Score_Frame','Score_Time','Score_Left','Score_Right','Score_TimeLabel'}),
                          ('Result',{'Result_Victory','Result_Defeat','Result_Draw','Result_Title','Result_Return'})]:
        doc=read(data/(name+'_Layout.json'))
        require(doc['schema']=='lostark.ui-layout' and doc['formatVersion']==1,'Unsupported UI schema')
        ids=[s['id'] for s in doc['slots']]
        require(len(ids)==len(set(ids)) and required<=set(ids),'Missing/duplicate stable UI slot')
        for slot_data in doc['slots']:
            require(all(math.isfinite(v) for v in slot_data['rect'].values()) and
                    slot_data['rect']['width']>0 and slot_data['rect']['height']>0,'Invalid slot rect')
            for layer in slot_data['layers']:
                images.add(layer['path']);relative(resources,layer['path'])
            if 'keyframeAnimationPath' not in slot_data:continue
            anim=read(relative(repo/'Data',slot_data['keyframeAnimationPath']))
            require(anim['frameRate']==40 and anim['frameCount'] in (139,140) and
                    anim['labels']=={'start':1} and not anim['loop'],'Truncated source result timeline')
            require(bool(anim['layers']),'Empty source animation')
            for layer in anim['layers']:
                frames=[k['frame'] for k in layer['keyframes']]
                require(frames==sorted(set(frames)) and frames[0]==1 and frames[-1]==anim['frameCount'],
                        'Unordered/incomplete source animation keys')
                for key in layer['keyframes']:
                    require(all(math.isfinite(key[f]) for f in ['x','y','scaleX','scaleY','alpha','rotationDeg']) and
                            0<=key['alpha']<=1 and key['scaleX']>0 and key['scaleY']>0,'Invalid source key')
                    images.add(key['asset'])
    for asset in images:relative(resources,asset)
    camera=read(repo/'Data/Camera/ColosseumVictory.cutscene.json')
    require(camera['schema']=='lostark.colosseum-victory-cutscene' and camera['formatVersion']==1,
            'Unsupported victory schema')
    require(camera['bannerDurationMs']==3475 and camera['durationMs']==5000 and len(camera['actors'])==4,
            'Invalid source duration or project 4v4 actor binding')
    keys=camera['shots'][0]['keys']
    require(len(keys)==151 and keys[0]['timeMs']==0 and keys[-1]['timeMs']==5000,'Incomplete sampled camera')
    require([k['timeMs'] for k in keys]==sorted(set(k['timeMs'] for k in keys)),'Unordered camera keys')
    require(all(math.isfinite(v) for k in keys for f in ['eye','forward'] for v in k[f]),'Non-finite camera')
    for actor in camera['actors']:
        require(actor['clipName']=='sc_cheer_1' and actor['yawDegrees']==0 and not actor['loop'],
                'Native winner binding changed')
    for name,title in camera['bannerTitles'].items():
        require(name in ('Victory','Defeat','Draw') and title['frameRate']==40 and
                title['keys'][0]['alpha']==0 and title['keys'][-1]['alpha']==0,'Invalid title track')
    sys.path.insert(0,str(repo/'Tools/ActorXAssetCooker'))
    from build_card_maze_player_animations import SOURCES,sections
    catalog=read(repo/'Data/Actors/CharacterCatalog.json')
    require({a['assetId'] for a in catalog['characters']}==set(SOURCES),'Unexpected playable class roster')
    for actor in catalog['characters']:
        name=actor['assetId'];asset=f'Character/{name}/AnimSets/{name}_ColosseumVictoryAnimSet.wmodel'
        require(actor['animationSetModels'].count(asset)==1,'Missing/duplicate winner animation set')
        binary=relative(resources,asset).read_bytes();_,rows=sections(binary)
        clips=[s[4].split(b'\0')[0] for s in rows if s[0]==4]
        require(clips==[b'sc_cheer_1'],'Not exact source victory clip')
        body=relative(resources,actor['bodyModel']).read_bytes();_,body_rows=sections(body)
        skel=next(s for s in rows if s[0]==3);body_skel=next(s for s in body_rows if s[0]==3)
        require(binary[16+skel[2]:16+skel[2]+skel[3]]==body[16+body_skel[2]:16+body_skel[2]+body_skel[3]],
                'Victory attachment skeleton mismatch')
    print('VALID: 2 layouts, 3 native40fps result timelines,',len(images),'image references,151 camera keys,7 exact native cheer attachments')


def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--movie',type=Path)
    ap.add_argument('--tex',type=Path)
    ap.add_argument('--validate',action='store_true',help='Read-only installed source-presentation contract validation')
    ap.add_argument('--repo',type=Path,default=Path(__file__).resolve().parents[2])
    a=ap.parse_args()
    if a.validate:
        validate(a.repo)
        return
    if a.movie is None or a.tex is None:ap.error('--movie and --tex are required for extraction')
    ex=Extractor(a.movie,a.tex,a.repo/'Client/Bin/Resources/UI/Colosseum/Result')
    data=a.repo/'Data/UI/Colosseum'
    source={'package':'EFUI_COLOSSEUM','movie':'colosseumplaying_loc_int','scoreState':'startGroup_mc.bg_mc.Score (frame 19)'}
    # UI authoring reference resolution is1280x720. Source stage is1920x1080.
    s=2/3
    hud=slot('Score_Frame',(960-263)*s,0,524*s,132*s,ex.asset(397,np.ones(4),np.zeros(4))[0])
    markers=[slot('Score_Time',875*s,38*s,170*s,54.4*s),
             slot('Score_Left',812*s,22*s,100*s,62.5*s),
             slot('Score_Right',982*s,22*s,100*s,62.5*s),
             slot('Score_TimeLabel',873*s,7*s,172*s,37.5*s)]
    base={'schema':'lostark.ui-layout','formatVersion':1,'resolution':{'width':1280,'height':720},'classes':['Default']}
    write(data/'MatchHUD_Layout.json',dict(base,slots=[hud]+markers))
    slots=[]
    titles={}
    for name,cid,y in [('Victory',766,160),('Defeat',755,120.5),('Draw',537,160)]:
        anim=ex.animation(cid)
        write(data/f'Result_{name}.keyframes.json',anim)
        st=slot('Result_'+name,0,y*s,1,1,kind=9)
        st['keyframeAnimationPath']=f'UI/Colosseum/Result_{name}.keyframes.json'
        st['keyframeAnimationScale']=s
        slots.append(st)
        title_keys=[]
        for frame in range(1,anim['frameCount']+1):
            label=next(v for v in display(ex.doc['sprites'][cid]['items'],frame).values()
                       if v.get('name')=='titleString_lb')
            m=label['matrix'];cx=label.get('cx',{})
            title_keys.append({'timeMs':(frame-1)*1000/anim['frameRate'],
                               'x':m['tx']*s,'y':(m['ty']+y)*s,
                               'width':120*m['sx']*s,'height':23*m['sy']*s,
                               'alpha':(cx.get('mul') or [256]*4)[3]/256})
        titles[name]={'frameRate':anim['frameRate'],'keys':title_keys}
        print(name,'frames',anim['frameCount'],'layers',len(anim['layers']))
    slots += [slot('Result_Title',710*s,321*s,500*s,75*s),
              slot('Result_Return',540,627.6,200,28.8,'UI/SystemOption/SystemOption_Btn_Normal.png')]
    write(data/'Result_Layout.json',dict(base,slots=slots))
    write(data/'Result_TitleTracks.json',{'schema':'lostark.colosseum-result-title-tracks',
          'formatVersion':1,'source':source,'bannerTitles':titles})
    print('unique textures',len(ex.variants),'untranslated source features',dict(ex.omitted))


if __name__=='__main__':main()

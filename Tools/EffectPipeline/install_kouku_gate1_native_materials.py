"""Install the selected Kouku native material tables without replacing peers.

The existing Artist material interpreter owns the runtime. This tool updates
only supplied program IDs within 2304..2495 and emits sourceMaterial patches. It does
not rewrite the renderer or Has_ArtistMaterialContract implementation.
"""
from pathlib import Path
import argparse, copy, hashlib, json, re

ROOT=Path(__file__).resolve().parents[2]
FIRST,LAST=2304,2495
def read(path):return json.loads(path.read_text(encoding='utf-8-sig'))
def write(path,value):
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(json.dumps(value,ensure_ascii=False,indent=2,allow_nan=False)+'\n',encoding='utf-8')
def quote(value):return json.dumps(value,ensure_ascii=False)
def profile_id(parent):
    slug=re.sub('[^a-z0-9]+','.',parent.lower()).strip('.')
    return 'ue3.material.'+slug[:72]+'.'+hashlib.sha256(parent.encode()).hexdigest()[:12]
def install(contract_path,evidence,header_path):
    contract=read(contract_path);programs=copy.deepcopy(contract['programs'])
    assert not contract.get('deferredPrograms'),('native programs are incomplete',contract.get('deferredPrograms'))
    owned={p['program'] for p in programs}
    assert owned and len(owned)==len(programs) and owned<=set(range(FIRST,LAST+1)),('native program set differs',len(programs))
    programs.sort(key=lambda p:p['program']);materials=[];arrays=[];entries=[]
    for p in programs:
        i=p['program'];parent=p['parentMaterial'];pid=profile_id(parent);runtime=f'effect.ue3.kouku-{i}-native.v1'
        assert p['nativeBlend'] in ('blend_additive','blend_translucent','blend_opaque','blend_masked'),p['nativeBlend']
        render=('ADDITIVE' if p['nativeBlend']=='blend_additive' else 'ALPHA')+('_TWO_SIDED_DEPTH_READ' if p['nativeTwoSided'] else '_ONE_SIDED_DEPTH_READ')
        if p['nativeBlend']=='blend_opaque' and not p['nativeTwoSided']:render='OPAQUE_BACK_DEPTH_WRITE'
        textures=p['textures'];parameters=p['parameters'];switches=p['staticSwitches']
        for texture in textures:
            assert (ROOT/'Client/Bin/Resources'/texture['assetId']).is_file(),texture
        assert len({v['name'] for v in parameters})==len(parameters)
        arrays.append(f'inline constexpr std::array<std::string_view,{len(textures)}> ARTIST_TEXTURES_{i} = {{{{'+','.join(quote(t['name']) for t in textures)+'}};\n')
        arrays.append(f'inline constexpr std::array<ARTIST_PARAMETER_DESC,{len(parameters)}> ARTIST_PARAMETERS_{i} = {{{{\n'+''.join('    {'+quote(v['name'])+f', {v["row"]}u, {v["lane"] or 0}u, '+str(v['kind']=='vector').lower()+'},\n' for v in parameters)+'}};\n')
        arrays.append(f'inline constexpr std::array<ARTIST_SWITCH_DESC,{len(switches)}> ARTIST_SWITCHES_{i} = {{{{\n'+''.join('    {'+quote(v['parameterName'])+', '+str(v['value']).lower()+'},\n' for v in switches)+'}};\n')
        shape=p['rendererShape']
        entries.append('    {'+f'{i}u,'+','.join(quote(x) for x in [runtime,p['sourceMaterial'],parent,pid])+','+str(shape in ('mesh','staticMesh')).lower()+','+str(p['modelCue']).lower()+','+quote(shape)+','+','.join(str(v).lower() for v in [p['requiresSceneColor'],p['requiresDepthSample'],p['requiresTangentView'],'dynamicparameter' in p['sourceVF']])+',EFFECT_RENDER_PROFILE::'+render+f',ARTIST_TEXTURES_{i},ARTIST_PARAMETERS_{i},ARTIST_SWITCHES_{i}'+'},\n')
        source=dict(enabled=True,profileId=pid,runtimeShaderProfileId=runtime,parentMaterialPath=parent,semanticStatus='reconstructed_profile',
            textures=[{k:t[k] for k in ('name','sourceObjectPath','assetId','addressU','addressV','colorSpace','samplingEvidence')} for t in textures],
            scalars=[dict(name=v['name'],group='None',value=v['effective']) for v in parameters if v['kind']=='scalar'],
            vectors=[dict(name=v['name'],group='None',value=v['effective']) for v in parameters if v['kind']=='vector'],
            staticSwitches=[dict(name=v['parameterName'],group='None',value=v['value']) for v in switches],dynamicParameterSemantics=['unbound']*4,subUVMode='none')
        materials.append(dict(program=i,sourceMaterial=p['sourceMaterial'],rendererShape=shape,occurrences=p['occurrences'],
            material=dict(templateId='effect.source_material',sourceMaterialPath=p['sourceMaterial'],renderProfile=render.lower(),sourceProfile=source)))
    raw=header_path.read_bytes();bom=b'\xef\xbb\xbf' if raw.startswith(b'\xef\xbb\xbf') else b'';payload=raw[len(bom):]
    try:original=payload.decode('utf-8');encoding='utf-8'
    except UnicodeDecodeError:original=payload.decode('cp949');encoding='cp949'
    newline='\r\n' if '\r\n' in original else '\n';text=original
    own='(?:'+'|'.join(str(i) for i in sorted(owned))+')'
    text=re.sub(r'inline constexpr std::array<(?:std::string_view|ARTIST_PARAMETER_DESC|ARTIST_SWITCH_DESC),\d+> ARTIST_(?:TEXTURES|PARAMETERS|SWITCHES)_'+own+r' = \{\{.*?\}\};\r?\n','',text,flags=re.S)
    text=re.sub(r'^    \{'+own+r'u,.*(?:\r?\n|$)','',text,flags=re.M)
    marker='inline constexpr std::array<ARTIST_PROGRAM_DESC,';start=text.index(marker);end=text.index('}};',start)+3
    old_entries=text[text.index('\n',start)+1:end-3]
    combined=old_entries+''.join(entries).replace('\n',newline)
    count=sum(line.startswith('    {') for line in combined.splitlines())
    replacement=''.join(arrays).replace('\n',newline)+f'inline constexpr std::array<ARTIST_PROGRAM_DESC,{count}> ARTIST_PROGRAMS = {{{{'+newline+combined+'}};'
    final=text[:start]+replacement+text[end:]
    # Reapplying changes no bytes; non-owned arrays and the consumer tail remain
    # byte-identical, including their original encoding and newline convention.
    result=bom+final.encode(encoding)
    assert header_path.read_bytes()==raw,'header changed concurrently; rerun against current source'
    if result!=raw:header_path.write_bytes(result)
    write(evidence/'native_material_patch.json',dict(programs=materials))
    write(evidence/'native_table_installation.json',dict(programCount=len(programs),first=min(owned),last=max(owned),header=str(header_path),encoding=encoding,changed=result!=raw))
    print('Kouku native tables',len(programs),'header changed',result!=raw)
if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--contract',type=Path,default=ROOT/'out/KoukuGate1Restore20260911/native_runtime_contract.json');parser.add_argument('--evidence-root',type=Path,default=ROOT/'out/KoukuGate1FullRestore20260911');parser.add_argument('--header',type=Path,default=ROOT/'Client/Public/Effect_ArtistMaterial.h');args=parser.parse_args()
    install(args.contract,args.evidence_root,args.header)

"""Recover the three original Gate1 fireworks materials and their cooked shaders.

Run after build_kouku_gate1_fireworks_restore.py source acquisition. This tool
writes only the supplied evidence directory. Its MaterialMap, native binding,
DXBC and occurrence join feed generate_artist_native_runtime_shader.py; native
runtime installation remains an explicit separate step.
"""
import argparse
import subprocess
import json,sys,pathlib,struct,functools,re
ROOT=pathlib.Path(__file__).resolve().parents[2]
sys.path[:0]=[str(ROOT/'Tools/LevelPlacementExtractor'),str(ROOT/'Tools/EffectPipeline')]
from extract_ue3_effect_material_closure import *
import extract_ue3_material_shader_maps as sm
sys.stdout.reconfigure(encoding='utf8')
OUT=None
SOURCE=pathlib.Path('C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829/CanonicalSource/Effect')
packages={p.parent.name.lower():p for p in (SOURCE/'Closure/SourcePackages').glob('*/*.upk')}
for p in SOURCE.glob('Packages/*/Source/*.upk'):packages.setdefault(p.parent.parent.name.lower(),p)
@functools.lru_cache(None)
def pkg(name):return load_package(packages[name],LOSTARK_KR_AES_KEY)
def fullref(name,p,ref):
 path=package_ref_path(ref,p.imports,p.exports)
 return name+'.'+path if ref>0 else path
@functools.lru_cache(None)
def obj(path):
 name,relative=path.split('.',1);p=pkg(name);e=find_export(p,relative)
 serial=p.logical[e.serial_offset:e.serial_offset+e.serial_size]
 props,end=parse_tagged_properties(serial,p.names,p.summary.version)
 cls=package_ref_name(e.class_index,p.imports,p.exports)
 return dict(path=path,package=name,className=cls,properties=props,tail=serial[end:],serialSha256=hashlib.sha256(serial).hexdigest(),exportIndex=e.index)
@functools.lru_cache(None)
def material(path):
 o=obj(path);p=pkg(o['package']);props=o['properties'];parent=tagged_value(props,'parent')
 if o['className']=='materialinstanceconstant':
  assert parent
  chain=material(fullref(o['package'],p,parent));root=chain['parentMaterial'];baseid=chain['baseId']
  static=sm.decode_static_set_from_tail(o['tail'],bytes.fromhex(baseid),p.names,sm.POLICY_BLOCK_ABSENT)
  if static.get('status')==sm.STATUS_BLOCKED:static=chain['mic']
  original=decode_material_instance(p,path.split('.',1)[1])
  numeric=json.loads(json.dumps(chain['effectiveNumericOverrides']));tex=dict(chain['textureOverrides'])
  for typ,field in [('scalars','scalarParameters'),('vectors','vectorParameters')]:
   for row in original[field]:numeric[typ][row['name']]={'value':row['value'],'owner':path}
  for row in original['textureParameters']:
   ref=row['packageIndex'];tex[row['name']]=fullref(o['package'],p,ref) if ref else None
 else:
  assert o['className'] in ('material','decalmaterial'),o['className']
  root=path;baseid=o['tail'][16:32].hex();assert len(baseid)==32
  s=sm.parse_static_parameter_set(bytes.fromhex(baseid)+struct.pack('<IIII',0,0,0,0),0,p.names)
  eq=sm.engine_equivalent_static_parameter_set(s)
  static={'staticParameterSet':sm.public_static_set(s),'engineEqualityStaticParameterSetSha256':sm.canonical_json_sha256(eq)}
  numeric={'scalars':{},'vectors':{}};tex={}
 return dict(sourceMaterial=path,parentMaterial=root,parentProperties=obj(root)['properties'],baseId=baseid,mic=static,
             effectiveNumericOverrides=numeric,textureOverrides=tex,sourceSerialSha256=o['serialSha256'])
def selected():
 return sorted({r['sourceMaterial'] for r in json.loads((OUT/'source_occurrences.json').read_bytes()) if r['sourceMaterial'] != 'enginematerials.defaultparticle'})


def patch_engine_prefix():
    """Qualify the displaced opacity input by exact source ownership/dataflow."""
    contract_path = OUT/'native_runtime_contract.json'
    contract = json.loads(contract_path.read_bytes())
    row = next(p for p in contract['programs'] if p['program'] == 2360)
    assert (row['sourceMaterial'], row['sourcePS'], row['sourceVS'], row['sourceVF']) == (
        'fx_m_mi_00.fx_mi.fx_b_pa_cd_01_1_tr', 'ab141c44af464947b2a7b26cf17b684e',
        '2dd6d96a7e6c974fac82106409a5b9b8', 'fparticleoffsetcenterdynamicparametervertexfactory')
    material = next(p for p in json.loads((OUT/'native_material_inputs.json').read_bytes())['materials']
        if p['sourceMaterial'] == row['sourceMaterial'])
    assert material['mapKey'] == '57f7bdc060687b68ad0e018532e3ece3f52bcad8bed8239d4b4b2d2ae1c4dbcb'
    pixel = json.loads((OUT/'full_programs'/f"{row['sourcePS']}.json").read_bytes())
    closure = pixel['bindings']['constantBufferClosure']
    assert closure['leadingUnownedConstantBuffer0Slots'] == [0, 1, 2]
    assert closure['boundConstantBuffer0Slots'] == [3, 4, 5, 6, 7]
    # Native reflection names were stripped. The MaterialMap ownership plus
    # this sole external-alpha multiply is the same evidence used by the
    # existing engine-opacity adapter; it cannot alias a material parameter.
    opacity_reads = [line for line in pixel['disassembly']['instructions'] if 'cb0[2]' in line]
    assert opacity_reads == ['mul o0.w, r0.z, cb0[2].x']
    path = OUT/'Shader_EffectArtistNative.hlsli'
    original = path.read_text(encoding='utf8')
    def blocks(text):
        return {number: block for block, number in re.findall(r'(float4 ArtistNative(\d+)\(ARTIST_NATIVE_INPUT input\)\n\{.*?\n\})', text, re.S)}
    peers = blocks(original)
    match = re.search(r'float4 ArtistNative2360\(ARTIST_NATIVE_INPUT input\)\n\{.*?\n\}', original, re.S)
    assert match
    before = match[0]
    old = '    source[0].x=1.f; // Project engine opacity multiplier.'
    opacity = '    source[2].x=1.f; // Exact native external-opacity prefix at byte offset 32.'
    macro = '\n'.join(('    source[0].xy=g_ArtistSourceMacroUV.xy; // Projected PS occurrence center in source NDC.',
        '    source[1].zw=g_ArtistSourceMacroUV.zw; // Signed inverse projected diameter; original +0.5 stays below.'))
    assert old in before or opacity in before
    after = before.replace(old, opacity)
    if macro not in after:
        after = after.replace(opacity, opacity + '\n' + macro)
    result = original.replace(before, after, 1)
    declaration = 'float4 g_ArtistSourceMacroUV;'
    if declaration not in result:
        marker = 'float g_ArtistSourceMaterialTime = 0.f;'
        assert result.count(marker) == 1
        result = result.replace(marker, marker + '\n' + declaration, 1)
    updated = blocks(result)
    assert peers.keys() == updated.keys()
    assert all(peers[number] == updated[number] for number in peers if number != '2360')
    path.write_text(result, encoding='utf8')
    adapter = dict(sourcePS=row['sourcePS'], sourceVS=row['sourceVS'], materialMap=material['mapKey'],
        materialOwnedCB0Rows=closure['boundConstantBuffer0Slots'], externalOpacityByteOffset=32,
        externalOpacityInstruction=opacity_reads[0], externalOpacityValue=1,
        nativeReflectionNamesAvailable=False,
        opacityEvidence='Exact source MaterialMap ownership and sole RT0 alpha-multiply use; same engine opacity policy as existing native 2361.',
        macroUVEngineRows=[0, 1], macroUVRadiusSourceCm=200,
        macroUVSpatialBinding='g_ArtistSourceMacroUV.xy=projected PS occurrence center NDC; zw=signed inverse projected diameter.',
        macroUVSourceEquation='((sourceNdcXY - CB0[0].xy) * CB0[1].zw) + (0.5,0.5)',
        macroUVYAxis='Source NDC Y points up; texture V orientation is carried only by signed scale W.',
        runtimeBindingRequired=declaration)
    row['enginePrefixAdapter'] = adapter
    contract_path.write_text(json.dumps(contract, indent=2)+'\n', encoding='utf8')
    (OUT/'native_engine_prefix_adapter.json').write_text(json.dumps(adapter, indent=2)+'\n', encoding='utf8')
    print('Patched exact native 2360 external-opacity byte offset 32; other programs preserved.')


def scan_materials():
    rows=[]
    for path in selected():
     try:rows.append(material(path))
     except Exception as e:print('MATERIAL ERROR',path,str(e),flush=True);raise
    (OUT/'material_inputs_before_maps.json').write_text(json.dumps(rows,indent=2),encoding='utf8')
    print('materials',len(rows),'parents',len({r['parentMaterial'] for r in rows}),flush=True)
    cache=sm.package_tables(pathlib.Path('C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/EV2LG3OVEH3HGV7THTFFTM7TOKMCC.upk'))
    layout=sm.parse_shader_code_layout(cache);print('cache',layout,flush=True)
    scans=json.loads((OUT/'material_map_scan.json').read_bytes()) if (OUT/'material_map_scan.json').exists() else {}
    missing=sorted({r['baseId'] for r in rows}-set(scans))
    if missing:scans.update(sm.scan_base_material_contexts(cache,layout,missing))
    (OUT/'material_map_scan.json').write_text(json.dumps(scans,indent=2),encoding='utf8')
    for r in rows:
     eq=r['mic']['engineEqualityStaticParameterSetSha256']
     try:
      context=sm.select_unique_map_context(scans[r['baseId']],eq)
      r['materialMap']=sm.parse_material_map(cache,layout,context,eq);r['mapKey']=eq
     except Exception as e:r['mapError']=str(e)
    (OUT/'material_inputs_with_maps.json').write_text(json.dumps(rows,indent=2),encoding='utf8')
    print('mapped',sum('materialMap' in r for r in rows),'errors',[(r['sourceMaterial'],r.get('mapError')) for r in rows if 'mapError' in r],flush=True)
    assert all('materialMap' in r for r in rows), 'Source material map closure is incomplete.'


def extract_programs():
    import inspect
    # Analytic circle material has no material texture expressions. Its sole t0/s0
    # pair is the original scene-depth sample, retained as an unowned engine lane.
    scanner=inspect.getsource(sm.scan_native_binding_array_candidates)
    scanner=scanner.replace('vector_count > 0 and (texture_count > 0 or textureless_shader)', 'vector_count > 0 and (texture_count >= 0)')
    scanner=scanner.replace('allow_empty=textureless_shader', 'allow_empty=textureless_shader or texture_count == 0')
    exec(scanner,sm.__dict__)

    def write(name,value): (OUT/name).write_text(json.dumps(value,indent=2),encoding='utf8')
    rows=json.loads((OUT/'material_inputs_with_maps.json').read_bytes())
    textures={}
    for r in rows:
     base=obj(r['parentMaterial']);p=pkg(base['package'])
     raw=base['tail'];assert len(raw)>=40 and struct.unpack_from('<I',raw)[0]==1
     assert raw[16:32].hex()==r['baseId']
     count=struct.unpack_from('<I',raw,36)[0];assert count<=256 and len(raw)>=40+4*count
     tail={'referencedTextures':[{'packageReference':struct.unpack_from('<i',raw,40+4*i)[0]} for i in range(count)]}
     r['effectiveTextures']=[]
     for i,exp in enumerate(r['materialMap']['uniformExpressionSet']['pixelTexture2DExpressions']):
      name=exp.get('parameterName');number=exp.get('parameterNameNumber',0)
      if number:name+='_'+str(number-1)
      path=r['textureOverrides'].get(name)
      if not path:
       raw=tail['referencedTextures'][exp['referencedTextureIndex']]
       path=fullref(base['package'],p,raw['packageReference'])
      tex=obj(path)
      row=dict(index=i,parameterName=name,sourceObjectPath=path,properties=tex['properties'])
      r['effectiveTextures'].append(row);textures[path]=tex['properties']
    write('native_material_inputs.json',{'materials':rows})
    write('required_native_textures.json',textures)
    refs={};selections=[]
    for r in rows:
     for vf in r['materialMap']['vertexFactories']:
      name=vf['vertexFactoryType']
      # Preserve all distinct compatible source VF variants for final emitter join.
      if name not in ['flocalvertexfactory','flocaldecalvertexfactory','fparticlevertexfactory','fparticledynamicparametervertexfactory','fparticlesubuvvertexfactory','fparticlesubuvdynamicparametervertexfactory','fparticlebeamtrailvertexfactory','fparticlebeamtraildynamicparametervertexfactory','fparticleoffsetcentervertexfactory','fparticleoffsetcenterdynamicparametervertexfactory']:continue
      ps=[s for s in vf['shaderReferences'] if s['shaderType']=='tbasepasspixelshaderfnolightmappolicyskylight']
      vs=[s for s in vf['shaderReferences'] if 'basepassvertexshaderfnolightmappolicy' in s['shaderType'] and 'nodensitypolicy' in s['shaderType']]
      assert len(ps)==1 and len(vs)==1,(name,ps,vs)
      for s in ps+vs:refs[s['shaderIdHex']]=s
      selections.append(dict(resolvedMaterial=r['sourceMaterial'],sourceVF=name,sourceVS=vs[0]['shaderIdHex'],sourcePS=ps[0]['shaderIdHex']))
    write('available_native_programs.json',selections)
    print('textures',len(textures),'program combinations',len(selections),'shader refs',len(refs),flush=True)
    cache=sm.package_tables(pathlib.Path('C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/EV2LG3OVEH3HGV7THTFFTM7TOKMCC.upk'))
    layout=sm.parse_shader_code_layout(cache);print('layout loaded',flush=True)
    (OUT/'dxbc').mkdir(exist_ok=True)
    dx=sm.extract_selected_packed_dxbc(cache,layout,list(refs.values()));print('DXBC recovered',len(dx),flush=True)
    for sid,row in dx.items():(OUT/'dxbc'/(sid+'.dxbc')).write_bytes(row['_bytecode'])
    pixel_refs=[ref for ref in refs.values() if 'pixelshader' in ref['shaderType']]
    objects=sm.extract_selected_shader_objects(cache,layout,pixel_refs);print('shader objects recovered',flush=True)
    disassembler=sm.D3DDisassembler(sm.DEFAULT_D3DCOMPILER)
    (OUT/'full_programs').mkdir(exist_ok=True)
    byid={s['shaderIdHex']:r for r in rows for vf in r['materialMap']['vertexFactories'] for s in vf['shaderReferences']}
    errors=[]
    for sid,ref in refs.items():
     try:
      bytecode=dx[sid]['_bytecode'];dis=disassembler.disassemble(bytecode)
      chunks=sm.dxbc_chunk_payloads(bytecode)
      sig=sm.parse_dxbc_signature(chunks.get('ISGN',chunks.get('ISG1')))
      result={'shaderId':sid,'disassembly':dis,'inputSignature':sig}
      if dis['profile'].startswith('ps'):
       closure=sm.parse_dxbc_declaration_closure(dis,allow_textureless=True);o=objects['byShaderId'][sid]
       expressions=byid[sid]['materialMap']['uniformExpressionSet']['pixelVectorExpressions']
       required_vectors={i for i,e in enumerate(expressions) if e.get('parameterName')=='selectioncolor' and e.get('typeName')=='fmaterialuniformexpressionvectorparameter'} if not byid[sid]['materialMap']['uniformExpressionCounts']['pixelTexture2DExpressions'] else set()
       if not closure['observedSamplePairCounts']:assert len(required_vectors)==1,('textureless BasePass has no exact SelectionColor vector',sid)
       result['bindings']=sm.select_unique_native_binding_arrays(o['_bytes'],o['logicalOffset'],byid[sid]['materialMap']['uniformExpressionCounts'],closure,required_vector_expression_indices=required_vectors)
      (OUT/'full_programs'/(sid+'.json')).write_text(json.dumps(result,indent=2),encoding='utf8')
     except Exception as e:errors.append((sid,str(e)))
    write('native_program_extraction_errors.json',errors)
    print('programs recovered',len(refs)-len(errors),'errors',errors,flush=True)
    assert not errors, ('Original native program extraction failed', errors)


def select_programs():
    from build_warlord_asvf_full_restore import norm,merge
    occurrences=json.loads((OUT/'source_occurrences.json').read_bytes())
    records=json.loads((OUT/'source_module_inputs.json').read_bytes())['records']
    defaults=json.loads((OUT/'source_class_defaults.json').read_bytes())['records']
    if isinstance(defaults,list):defaults={r['fullPath']:r for r in defaults}
    records.update(defaults)
    @functools.lru_cache(None)
    def effective(path):
     row=records[path];parent=row.get('archetypeFullPath')
     if not parent and not path.startswith(('engine.','efgame.','core.')):
      cls=row['classPath'];parent=cls.split('.')[0]+'.default__'+cls.split('.')[-1]
     return merge(effective(parent) if parent else {},norm(row['properties']))
    def value(props,key,default=None):
     v=props.get(key,default);return v.get('value') if isinstance(v,dict) and 'value'in v else v
    materials={r['sourceMaterial']:r for r in json.loads((OUT/'native_material_inputs.json').read_bytes())['materials']}
    programs={};errors=[]
    for o in occurrences:
     if o['rendererShape']=='light' or o['sourceMaterial']=='enginematerials.defaultparticle':continue
     try:
      mat=o['sourceMaterial'];shape=o['rendererShape'];r=materials[mat]
      req=next(k for k in o['moduleOrder'] if 'particlemodulerequired' in k);props=effective(req)
      names={v['vertexFactoryType'] for v in r['materialMap']['vertexFactories']}
      if shape=='mesh':vf='flocalvertexfactory'
      elif shape=='decal':vf='flocaldecalvertexfactory'
      elif shape=='animationTrail':vf='fparticlebeamtraildynamicparametervertexfactory' if any('parameterdynamic' in k for k in o['moduleOrder']) else 'fparticlebeamtrailvertexfactory'
      else:
       # Cooked material usage selects whether the sprite vertex stream has dynamic lanes.
       # Offset-center and SubUV are the emitter's Required module inputs.
       offset=bool(value(props,'boffsetcenter',False));subuv=str(value(props,'interpolationmethod','psuvim_none')).lower()!='psuvim_none'
       dynamic=any('parameterdynamic' in k for k in o['moduleOrder'])
       stem='fparticle'+('subuv' if subuv else '')+('offsetcenter' if offset else '')
       vf=stem+('dynamicparameter' if dynamic else '')+'vertexfactory'
       if vf not in names and not dynamic and stem+'dynamicparametervertexfactory' in names:
        vf=stem+'dynamicparametervertexfactory'
      if vf not in names and 'dynamicparameter' in vf and vf.replace('dynamicparameter','') in names:
       vf=vf.replace('dynamicparameter','')
      vfrow=next(v for v in r['materialMap']['vertexFactories'] if v['vertexFactoryType']==vf)
      ps=next(s for s in vfrow['shaderReferences'] if s['shaderType']=='tbasepasspixelshaderfnolightmappolicyskylight')
      vs=next(s for s in vfrow['shaderReferences'] if 'basepassvertexshaderfnolightmappolicy' in s['shaderType'] and 'nodensitypolicy' in s['shaderType'])
      selected=dict(resolvedMaterial=mat,sourceVF=vf,sourceVS=vs['shaderIdHex'],sourcePS=ps['shaderIdHex'])
      key=(mat,vf,shape)
      row=programs.setdefault(key,dict(**selected,rendererShape=shape,occurrences=[]))
      row['occurrences'].append(o['elementId'])
     except Exception as e:errors.append((o['elementId'],o['sourceMaterial'],str(e)))
    (OUT/'selected_runtime_material_programs.json').write_text(json.dumps({'programs':list(programs.values()),'errors':errors},indent=2),encoding='utf8')
    print('Selected',len(programs),'occurrences',sum(len(r['occurrences'])for r in programs.values()),'errors',errors)
    for r in programs.values():
     if r['rendererShape'] in ('decal','animationTrail'):
      p=OUT/'full_programs'/(r['sourcePS']+'.json')
      print('SPECIAL',r['rendererShape'],r['resolvedMaterial'],r['sourcePS'],p.exists())
    assert not errors, ('Source emitter/native VF join failed', errors)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=pathlib.Path, required=True)
    parser.add_argument('--stage', choices=('all', 'materials', 'programs', 'select', 'generate', 'abi'), default='all')
    options = parser.parse_args()
    OUT = options.evidence_root.resolve()
    assert (OUT/'source_occurrences.json').is_file(), 'Acquire the fireworks source occurrences first.'
    if options.stage in ('all', 'materials'):
        scan_materials()
    if options.stage in ('all', 'programs'):
        extract_programs()
    if options.stage in ('all', 'select'):
        select_programs()
    if options.stage == 'generate':
        subprocess.run([sys.executable, str(ROOT/'Tools/EffectPipeline/generate_artist_native_runtime_shader.py'),
            '--source-dir', str(OUT), '--program-start', '2360', '--profile-domain', 'kouku'], check=True)
    if options.stage in ('generate', 'abi'):
        patch_engine_prefix()

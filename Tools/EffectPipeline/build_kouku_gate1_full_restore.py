"""Restore the two selected Gate 1 source stages through ordinary V1 Effects.

Raw source exports are read-only. This builder owns only the two new authored
documents and its evidence directory; native material programs are supplied by
the existing material pipeline, never inferred from material names.
"""
from pathlib import Path
import argparse, base64, collections, copy, hashlib, json, math, struct, sys, shutil

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/LevelPlacementExtractor'))
import build_imported_effect_documents as imported
from build_action_cue_recipe import decode_typed_payload
from build_warlord_asvf_full_restore import norm, merge
from extract_ue3_effect_material_closure import load_package, find_export
import extract_ue3_placements as ue3
from extract_ue3_particle_graph import property_references, is_particle_graph_class

SOURCE = Path('C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829')
ACTION = SOURCE / 'RemainingCharacterExtraction-20260829/ActionNameSources/MN_RPCT_05.action-effects.json'
GRAPH = SOURCE / 'CanonicalSource/Effect/Graphs/LV_LUT_MIDNIGHTC_ED.particle-graph.json'
SELECTED = {4219877: ([0], '내려치기C'), 4219801: ([0,1], '불뿜기')}

def read(path): return json.loads(path.read_text(encoding='utf-8-sig'))
def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + '\n', encoding='utf-8')

def source_package(domain, logical):
    directory = SOURCE / 'CanonicalSource' / domain / 'Packages' / logical.upper()
    receipt = read(directory / 'source.receipt.json')
    return directory / receipt['outputs'][0]['path']

def qualify(package, path, reference=1):
    return (package + '.' + path if reference > 0 else path).lower()

def record_from_export(package, logical, entry):
    raw = package.logical[entry.serial_offset:entry.serial_offset + entry.serial_size]
    if len(raw) >= 12 and 0 <= struct.unpack_from('<i',raw,4)[0] < len(package.names) and package.names[struct.unpack_from('<i',raw,4)[0]].lower() == 'none' and struct.unpack_from('<i',raw,8)[0] == 0:
        properties, end = {}, 12
    else:
        original_decoder=ue3.decode_property_value
        def source_decoder(property_type,struct_type,payload,names,bool_value,property_name=None,owner_struct_type=None):
            if property_type.lower()=='structproperty' and str(struct_type).lower()=='orbitoptions':
                decoded,consumed=ue3.parse_tagged_properties_at(payload,names,0,struct_type)
                assert consumed==len(payload),('OrbitOptions trailing bytes',entry.object_name)
                return dict(size=len(payload),properties=decoded)
            return original_decoder(property_type,struct_type,payload,names,bool_value,property_name,owner_struct_type)
        ue3.decode_property_value=source_decoder
        try:properties,end=ue3.parse_tagged_properties(raw,package.names,package.summary.version)
        finally:ue3.decode_property_value=original_decoder
    cls = ue3.package_ref_name(entry.class_index, package.imports, package.exports)
    path = ue3.package_ref_path(entry.index + 1, package.imports, package.exports)
    parent = ue3.package_ref_path(entry.archetype_index, package.imports, package.exports)
    refs = property_references(properties, package.imports, package.exports)
    for ref in refs: ref['objectPath'] = qualify(logical, ref['objectPath'], ref['packageIndex'])
    return dict(fullPath=qualify(logical, path), className=cls.lower(),
        classPath=qualify(logical, ue3.package_ref_path(entry.class_index, package.imports, package.exports), entry.class_index),
        archetypeFullPath=qualify(logical, parent, entry.archetype_index) if parent else None,
        properties=properties, references=refs, serialSha256=hashlib.sha256(raw).hexdigest(),
        sourcePackage=str(package.path), exportIndex=entry.index, propertyStreamEnd=end)

def acquire(evidence):
    action = read(ACTION)
    socket_path=evidence/'source_socket_contract.json'
    socket_contract=read(socket_path) if socket_path.is_file() else None
    notifies = []
    for sid, (stages, label) in SELECTED.items():
        a = next(a for a in action['actions'] if int(a['actionId']) == sid)
        offset = 0.0
        for stage in stages:
            s = next(s for s in a['stages'] if s['stageIndex'] == stage)
            for n in s['notifies']:
                if n['sourceType'] not in ('PlayParticleEffect', 'Trails', 'PawnMaterialParam'): continue
                row = copy.deepcopy(n); row.update(actionId=sid, selectedStage=stage,globalTimeSeconds=offset+n['localTimeSeconds'])
                row['cue'] = decode_typed_payload(n['sourceType'], n['serializedPayload'], socket_contract, n['assetReferences'], n['serializedLabels'])
                notifies.append(row)
            offset += sum(c['lengthSeconds'] for c in s['animationClips'])
    systems = {ref['objectPath'].lower() for n in notifies for ref in n['assetReferences'] if ref['className'].lower() == 'particlesystem'}
    records = {}
    for logical in sorted({s.split('.')[0] for s in systems}):
        graph = read(GRAPH / (logical.upper() + '.particle-graph.json'))
        for original in graph['objects']:
            row = copy.deepcopy(original); key = qualify(logical, row['objectPath'])
            row['fullPath'] = key
            row['classPath'] = row['classPath'].lower()
            row['className'] = row['className'].lower()
            row['archetypeFullPath'] = qualify(logical, row['archetypePath'], row['archetypeIndex']) if row['archetypePath'] else None
            for ref in row['references']: ref['objectPath'] = qualify(logical, ref['objectPath'], ref['packageIndex'])
            records[key] = row
    script_errors = []
    for logical in ('core', 'engine', 'efgame'):
        script_path = Path('C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/9L6NC53E9WINO5FELWUN0.u') if logical == 'core' else source_package('Shared', logical)
        package = load_package(script_path, ue3.LOSTARK_KR_AES_KEY)
        for entry in package.exports:
            cls = ue3.package_ref_name(entry.class_index, package.imports, package.exports)
            if is_particle_graph_class(cls) or entry.object_name.lower().startswith('default__'):
                try: row = record_from_export(package, logical, entry)
                except Exception as error:
                    script_errors.append(dict(package=logical,object=entry.object_name,className=cls,error=str(error))); continue
                records[row['fullPath']] = row
    external_packages = {}
    external_records = []
    def ensure_record(key):
        if key in records:return
        logical,relative=key.split('.',1)
        if logical not in external_packages:
            receipt=SOURCE/'CanonicalSource/Effect/Packages'/logical.upper()/'source.receipt.json'
            if receipt.is_file():path=source_package('Effect',logical)
            else:
                candidates=list((SOURCE/'CanonicalSource/Effect/Closure/SourcePackages'/logical).glob('*.upk'))
                assert len(candidates)==1,('missing exact external particle package',key,candidates)
                path=candidates[0]
            external_packages[logical]=load_package(path,ue3.LOSTARK_KR_AES_KEY)
        row=record_from_export(external_packages[logical],logical,find_export(external_packages[logical],relative))
        assert row['fullPath']==key,(row['fullPath'],key)
        records[key]=row;external_records.append(key)
    effective, inherited, effective_refs = {}, {}, {}
    def resolve(key, stack=()):
        if key in effective: return effective[key]
        assert key not in stack, ('archetype cycle', key)
        row = records[key]; parent = row.get('archetypeFullPath')
        cls = row['classPath']; class_default = cls.split('.')[0] + '.default__' + cls.split('.')[-1]
        if not parent and key != class_default: parent = class_default
        if parent and parent not in records:ensure_record(parent)
        effective[key] = merge(resolve(parent, stack + (key,)) if parent else {}, norm(row.get('properties', {})))
        inherited[key] = ([parent] + inherited[parent]) if parent else []
        own_roots = {p.lower() for p in row.get('properties',{})}
        effective_refs[key] = [copy.deepcopy(r) for r in effective_refs.get(parent,[]) if r['property'].split('.')[0].split('[')[0].lower() not in own_roots] + copy.deepcopy(row['references'])
        return effective[key]
    index = imported.SourceIndex({}, {})
    selected_keys = set()
    def collect(key):
        if key in selected_keys:return
        if key not in records:
            name=key.rsplit('.',1)[-1]
            if not name.startswith(('particle','efparticle','distribution','efdistribution','pointlight','lightcomponent','default__')):return
            ensure_record(key)
        selected_keys.add(key)
        resolve(key)
        for ref in effective_refs[key]: collect(ref['objectPath'])
    for system in systems: collect(system)
    for key in selected_keys:
        row = records[key]; obj = imported.SourceObject(key, key, row['className'], key, resolve(key))
        for ref in effective_refs[key]:
            path = ref['objectPath']; prop = ref['property'].lower()
            obj.reference_paths.append((prop, path))
            if path in records: obj.references.append((prop, path))
        index.objects[key] = obj; index.by_source_id[key] = obj
    # Distribution and default subobject references inherited through a CDO
    # retain the package identity in which they were originally serialized.
    for key in set(p for chain in inherited.values() for p in chain):
        if key not in index.objects:
            row = records[key]; obj = imported.SourceObject(key,key,row['className'],key,resolve(key))
            for ref in row['references']:
                obj.reference_paths.append((ref['property'].lower(),ref['objectPath']))
                if ref['objectPath'] in records: obj.references.append((ref['property'].lower(),ref['objectPath']))
            index.objects[key] = obj; index.by_source_id[key] = obj
    occurrences = []; empty_emitters = []
    for n in notifies:
        for ref in n['assetReferences']:
            if ref['className'].lower() != 'particlesystem': continue
            system = ref['objectPath'].lower()
            for reference_property, emitter_key in index.objects[system].references:
                if reference_property != 'emitters': continue
                emitter = index.objects[emitter_key]
                if 'emitter' not in emitter.class_name: continue
                lods = [index.objects[k] for p,k in emitter.references if p.startswith('lodlevels')]
                if not lods:
                    assert not imported.prop(emitter.properties,'lodlevels',[]), ('unresolved LOD',emitter_key)
                    empty_emitters.append(dict(sourceNotify=n['notifyId'],sourceEmitter=emitter_key,reason='SOURCE_EMITTER_HAS_ZERO_LODS',properties=emitter.properties)); continue
                lod = lods[0]
                module_refs=[(p,k) for p,k in lod.reference_paths if p in ('requiredmodule','modules','typedatamodule','spawnmodule')]
                assert all(k in index.objects for p,k in module_refs),('unresolved original LOD module',lod.key,module_refs)
                modules = [index.objects[k] for p,k in module_refs]
                kind, _, shape = imported.classify({'sourceSystemId':system}, modules)
                if any('typedataanimtrail' in m.class_name for m in modules): kind, shape = 'trail', 'animationTrail'
                required = next(m for m in modules if m.class_name == 'particlemodulerequired')
                material = next((p for prop,p in required.reference_paths if prop == 'material'), '')
                mesh = next((p for m in modules for prop,p in m.reference_paths if prop == 'mesh'), '')
                identity = n['notifyId'] + '|' + emitter_key
                element_id = 'kouku.' + str(n['actionId']) + '.' + hashlib.sha256(identity.encode()).hexdigest()[:20]
                occurrences.append(dict(elementId=element_id,actionId=n['actionId'],sourceNotify=n['notifyId'],sourceType=n['sourceType'],
                    sourceSystem=system,sourceEmitter=emitter_key,sourceLOD=lod.key,sourceMaterial=material,
                    rendererShape=shape,kind=kind,sourceMesh=mesh,moduleOrder=[m.key for m in modules],
                    sourceTimeSeconds=n['globalTimeSeconds'],sourceDurationSeconds=n['durationSeconds']))
    closure_rows=[]
    for o in occurrences:
        props=resolve(o['sourceLOD']); raw_count=0
        for field in ('requiredmodule','modules','typedatamodule','spawnmodule'):
            value=imported.prop(props,field,0)
            raw_count+=sum(bool(x) for x in value) if isinstance(value,list) else int(bool(value))
        refs=[r['objectPath'] for r in effective_refs[o['sourceLOD']] if r['property'] in ('requiredmodule','modules','typedatamodule','spawnmodule')]
        assert len(refs)==raw_count and refs==o['moduleOrder'],('raw LOD module closure mismatch',o['sourceLOD'],raw_count,refs,o['moduleOrder'])
        closure_rows.append(dict(elementId=o['elementId'],sourceLOD=o['sourceLOD'],rawNonNullModuleReferences=raw_count,resolvedModules=len(refs),orderedObjectPaths=refs))
    write(evidence/'first_lod_module_closure_validation.json',dict(baseEmitterCount=len(occurrences),unresolvedReferences=0,rows=closure_rows))
    write(evidence/'external_module_closure.json',dict(packages={k:str(v.path) for k,v in external_packages.items()},objects=external_records))
    write(evidence/'source_notifies.json',notifies)
    write(evidence/'unselected_script_export_parse_errors.json',script_errors)
    write(evidence/'source_occurrences.json',occurrences)
    write(evidence/'source_empty_emitters.json',empty_emitters)
    write(evidence/'source_class_defaults.json',{'records':[records[k] for k in sorted(set(p for chain in inherited.values() for p in chain))]})
    write(evidence/'source_module_inputs.json',{'records':{k:records[k] for k in sorted(selected_keys)}})
    return index,notifies,occurrences,records

def project_parameters(index, recipe, cue):
    bindings={p['name'].lower():p for p in cue.get('parameterOverrides',[])}
    projections=[]
    for module in recipe['modules']:
        for dist in module['distributions']:
            source=index.get_path(dist.get('sourceObjectPath'))
            if not source or 'particleparameter' not in source.class_name: continue
            props={k:imported.unwrap(v) for k,v in source.properties.items()}
            name=props.get('parametername',''); n=dist['componentCount']; binding=bindings.get(name)
            if binding and binding['type'] != ('scalar' if n==1 else 'vector'): binding=None
            def vector(key,default):
                v=props.get(key,default)
                return [imported.unwrap(v.get(k,default)) for k in ('x','y','z')[:n]] if isinstance(v,dict) else [v]*n
            value=vector('constant',0); modes=[props.get('parammode' if n==1 else ('parammodes' if i==0 else f'parammodes[{i}]'),'dpm_normal') for i in range(n)]
            if binding:
                supplied=[binding['scalarValue']] if n==1 else binding['vectorValue']
                mini,maxi,mino,maxo=[vector(k,d) for k,d in [('mininput',0),('maxinput',1),('minoutput',0),('maxoutput',1)]]
                for i in range(n):
                    if modes[i]=='dpm_direct': value[i]=supplied[i]
                    elif modes[i] in ('dpm_normal','dpm_abs'):
                        x=abs(supplied[i]) if modes[i]=='dpm_abs' else supplied[i]
                        f=0 if maxi[i]==mini[i] else max(0,min(1,(x-mini[i])/(maxi[i]-mini[i])))
                        value[i]=mino[i]+(maxo[i]-mino[i])*f
                    else: raise ValueError(('unsupported parameter mode',name,modes))
            assert not dist['lookupTable'] and not dist['keys'], ('parameter with competing table',source.key)
            dist['defaultMinimum']=value+[0]*(4-n);dist['defaultMaximum']=value+[0]*(4-n)
            projections.append(dict(module=module['objectPath'],property=dist['propertyPath'],parameter=name,binding=binding,value=value))
    return projections

def distribution_bounds(dist):
    values=dist['lookupTable'] or [v for key in dist['keys'] for v in key.get('minimum',[]) + key.get('maximum',[])]
    if not values: values=dist['defaultMinimum'][:dist['componentCount']]+dist['defaultMaximum'][:dist['componentCount']]
    return min(values),max(values)

def trail_history(evidence):
    package=load_package(source_package('Character','MN_RPCT_05_ANIMNOTIFY_TRAILS'),ue3.LOSTARK_KR_AES_KEY)
    outer=record_from_export(package,'mn_rpct_05_animnotify_trails',find_export(package,'MN_RPCT_05_4219821_0_0_0'))
    reference=next(r for r in outer['references'] if r['property']=='trail_default')
    row=record_from_export(package,'mn_rpct_05_animnotify_trails',package.exports[reference['packageIndex']-1])
    assert next(r['objectPath'] for r in row['references'] if r['property']=='pstemplate')=='fx_bs_01.trail.par_d_trail_23_loc_int'
    samples=[]
    for raw in imported.prop(row['properties'],'trailsampleddata'):
        item=dict(relativeTimeSeconds=imported.prop(raw,'relativetime'))
        for source,target in [('firstedgesample','firstEdgeUE3Cm'),('controlpointsample','controlPointUE3Cm'),('secondedgesample','secondEdgeUE3Cm')]:
            v=imported.prop(raw,source);item[target]=[v[k] for k in ('x','y','z')]
        samples.append(item)
    assert len(samples)==22 and all(b['relativeTimeSeconds']>=a['relativeTimeSeconds'] for a,b in zip(samples,samples[1:]))
    history=dict(historyId='kouku.4219877.animnotify-trails-303.baked-edges',coordinateBasis='UE3_CM_X_Z_NEG_Y_TO_RUNTIME_METERS',
        sourceEndTimeSeconds=samples[-1]['relativeTimeSeconds'],playbackClampSeconds=0.20000000298023224,samples=samples)
    write(evidence/'trail_source.json',dict(outer=outer,history=row,projection=history))
    return history

def prepare_textures(evidence, required_path):
    from PIL import Image
    root=ROOT/'Client/Bin/Resources';required=read(required_path);byname=collections.defaultdict(list)
    for p in root.rglob('*.dds'):byname[p.stem.lower()].append(p)
    result={};receipts=[]
    explicit={
        'fx_mastermaterial.fx_tex.fx_e_normal':SOURCE/'EffectRuntimeClosureExports-20260829/fx_mastermaterial/Export/FX_MASTERMATERIAL/fx_tex/fx_e_normal.dds',
        'fx_tex_00.fx_a_cloud_018':SOURCE/'CanonicalSource/Effect/UModelExports/FX_TEX_00/Export/FX_TEX_00/fx_a_cloud_018.dds',
        'fx_tex_05.fx_k_flowmask_01_v':SOURCE/'EffectRuntimeClosureExports-20260829/fx_tex_05/Export/FX_TEX_05/fx_k_flowmask_01_v.tga'}
    for key,props in required.items():
        package,relative=key.split('.',1);name=relative.rsplit('.',1)[-1]
        candidates=[p for p in byname[name] if p.parent.name.lower()==package]
        candidates.sort(key=lambda p:(0 if '/KoukuSaydon/' in p.as_posix() else 1,len(p.as_posix())))
        if candidates:path=candidates[0];source=path;mode='REUSE_EXACT_PACKAGE_OBJECT_RESOURCE'
        else:
            source=explicit[key];path=root/'Effect/KoukuSaydon/FullRestore/Textures'/package/(name+'.dds');path.parent.mkdir(parents=True,exist_ok=True)
            if source.suffix=='.dds':
                if not path.is_file() or path.read_bytes()!=source.read_bytes():shutil.copyfile(source,path)
                mode='COPY_SOURCE_DDS_BYTES'
            else:
                original=Image.open(source).convert('RGBA');original.save(path,format='DDS')
                assert Image.open(path).convert('RGBA').tobytes()==original.tobytes()
                mode='LOSSLESS_SOURCE_TGA_RGBA_BASE_MIP_DDS_COOK'
        raw=path.read_bytes();assert raw[:4]==b'DDS '
        height,width=struct.unpack_from('<II',raw,12)
        assert (width,height)==(imported.prop(props,'sizex'),imported.prop(props,'sizey')),(key,width,height)
        result[key]=path.relative_to(root).as_posix()
        receipts.append(dict(sourceObjectPath=key,sourceFile=str(source),assetId=result[key],mode=mode,width=width,height=height,sha256=hashlib.sha256(raw).hexdigest()))
    write(evidence/'texture_asset_map.json',result);write(evidence/'texture_resource_receipt.json',receipts)
    print('Exact texture paths',len(result))

def prepare_geometry(evidence):
    sys.path.insert(0,str(ROOT/'Tools/ModelAssetConverter'))
    import cook_wmodel_geometry_contract as geometry
    root=ROOT/'Client/Bin/Resources';target=root/'Effect/KoukuSaydon/FullRestore/Meshes';target.mkdir(parents=True,exist_ok=True)
    reused={'fm_a_cylinder_002':'Effect/Warlord/FullRestore/Meshes/fm_a_cylinder_002.wmodel',
            'fm_b_cylinder_005':'Effect/Artist/Meshes/Native/FX_SM_00/fm_b_cylinder_005.wmodel'}
    rows=[]
    for name,asset in reused.items():
        source=root/asset;payload=source.read_bytes();decoded=geometry.parse_geometry_wmodel(payload)
        assert len(decoded['submeshes'])==1 and decoded['submeshes'][0]['name'].lower()==name
        destination=target/(name+'.wmodel')
        if not destination.is_file() or destination.read_bytes()!=payload:destination.write_bytes(payload)
        rows.append(dict(source='fx_sm_00.'+name,assetId=destination.relative_to(root).as_posix(),reusedAssetId=asset,mode='EXISTING_NATIVE_SOURCE_MESH',modelPreScale=.01,
            hasColor0=decoded['hasColor0'],hasTexcoord1=decoded['hasTexcoord1'],vertexCount=len(decoded['submeshes'][0]['vertices'])))
    name='fm_e_plan_001';source=SOURCE/'EffectRuntimeClosureExports-20260829/fx_sm_00/Export/FX_SM_00'/f'{name}.gltf'
    legacy=root/'Effect/KoukuSaydon/Meshes/fx_sm_00'/f'{name}.wmodel'
    source_manifest=SOURCE/'CanonicalSource/Effect/Closure/closure.manifest.json'
    package_row=next(p for p in read(source_manifest)['sourcePackages'] if p['logicalPackage'].lower()=='fx_sm_00')
    mesh_package=source_manifest.parent/package_row['outputRelativePath']
    # This is an observed source-export/legacy geometry parity receipt, not a
    # claim that a historical exporter invocation or raw UPK vertex oracle ran.
    observed=dict(sourceObject='fx_sm_00.'+name,sourceGltf=str(source),legacyWmodel=str(legacy),
        sourceGltfSha256=hashlib.sha256(source.read_bytes()).hexdigest(),legacyWmodelSha256=hashlib.sha256(legacy.read_bytes()).hexdigest(),
        evidence='OBSERVED_EXISTING_SOURCE_GLTF_AND_LEGACY_WMODEL')
    receipt_path=evidence/'plane_geometry_observation.json';write(receipt_path,observed)
    digest=lambda p:hashlib.sha256(p.read_bytes()).digest()
    provenance=geometry.GeometryProvenanceEvidence('fx_sm_00.'+name,digest(source_manifest),'OBSERVED_SOURCE_RECEIPT',digest(mesh_package),
        digest(ROOT/'Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe'),digest(receipt_path),digest(receipt_path))
    payload,receipt=geometry.cook_wmodel_geometry_contract(source,legacy,provenance)
    destination=target/(name+'.wmodel');destination.write_bytes(payload);decoded=geometry.parse_geometry_wmodel(payload)
    rows.append(dict(source='fx_sm_00.'+name,assetId=destination.relative_to(root).as_posix(),mode='SOURCE_GLTF_WMODEL_GEOMETRY_PARITY',modelPreScale=.01,
        hasColor0=decoded['hasColor0'],hasTexcoord1=decoded['hasTexcoord1'],vertexCount=len(decoded['submeshes'][0]['vertices'])))
    write(evidence/'plane_geometry_cook.json',receipt);write(evidence/'geometry_installation.json',rows)
    print('Geometry installed',len(rows))

def project(evidence,index,notifies,occurrences,records,destination,material_patch=None):
    by_notify={n['notifyId']:n for n in notifies}
    socket_path=evidence/'source_socket_contract.json'
    sockets=read(socket_path) if socket_path.is_file() else None
    if destination.resolve()==(ROOT/'Data/Effects/Authored').resolve():assert material_patch is not None,'Product authoring requires closed native material programs'
    native={key:p['material'] for p in read(material_patch)['programs'] for key in p['occurrences']} if material_patch else {}
    docs={sid:dict(schema='lostark.effect-authoring',version=15 if sid==4219877 else 13,effectAssetId=f'effect.kouku.gate1.{sid}.full.restore',
        displayName='쿠크 1관문 '+label+' 전체 복원',particleSystem=dict(uniformScaleMultiplier=1,yawOffsetDegrees=0,directionYawDegrees=0,initialSpeedMultiplier=1),modelCues=[],elements=[])
        for sid,(_,label) in SELECTED.items()}
    history=trail_history(evidence)
    docs[4219877]['runtimeExtensions']=dict(formatVersion=1,bakedEdgeHistories=[history])
    changes=[]
    for ordinal,o in enumerate(occurrences):
        n=by_notify[o['sourceNotify']];cue=n['cue']
        if n['sourceType']=='PlayParticleEffect':
            cue=decode_typed_payload(n['sourceType'],n['serializedPayload'],sockets,n['assetReferences'],n['serializedLabels'])
            assert cue['enabled'] and cue['particleDataDecoded'] and cue['parameterOverridesDecoded'],n['notifyId']
        modules=[index.objects[k] for k in o['moduleOrder']];lod=index.objects[o['sourceLOD']]
        detail,mappings,bursts=imported.emitter_detail(index,lod,modules,o['sourceTimeSeconds'],o['sourceDurationSeconds'],ordinal+1)
        detail['particle']['authoringApproximate']=False
        detail['particle']['sourceScale']={k:1 for k in ('count','size','lifeTime','speed','rotation','alpha','spawnDelay')}
        recipe=imported.build_source_recipe(index,modules,o['rendererShape'],bursts)
        counts=collections.Counter()
        for module in recipe['modules']:
            counts[module['stableId']]+=1
            if counts[module['stableId']]>1:module['stableId']+=f"@reference:{counts[module['stableId']]}"
        parameters=project_parameters(index,recipe,cue)
        # Portable decals carry only fields used by their existing projector.
        # Engine/editor flags and raw Rotator units remain in source evidence.
        for module in recipe['modules']:
            if module['className']=='efparticlemoduletypedatadecal':
                allowed={'lodvalidity','nearplane','farplane','balwaysdecalupdate','rotation.degrees.roll'}
                module['literals']=[v for v in module['literals'] if v['propertyPath'] in allowed]
        for module,obj in zip(recipe['modules'],modules):
            if obj.class_name=='particlemoduleorbit':
                for option in ('offsetoptions','rotationoptions','rotationrateoptions'):
                    wrapped=obj.properties.get(option)
                    if wrapped is None:continue
                    values=imported.unwrap(wrapped)['properties']
                    module['literals']=[v for v in module['literals'] if not v['propertyPath'].startswith(option+'.')]
                    module['literals'] += [dict(propertyPath=option+'.'+k,kind='boolean',value=imported.unwrap(v)) for k,v in values.items()]
        life=[d for m in recipe['modules'] if m['className']=='particlemodulelifetime' for d in m['distributions'] if d['propertyPath']=='lifetime']
        if life:detail['particle']['lifeTimeSeconds']=list(distribution_bounds(life[0]))
        required=next(m for m in modules if m.class_name=='particlemodulerequired')
        for field,prop in [('emitterDurationSeconds','emitterduration'),('emitterDelaySeconds','emitterdelay'),('emitterLoopCount','emitterloops')]:
            if prop in required.properties:recipe[field]=imported.prop(required.properties,prop)
        active=o['sourceDurationSeconds'] or recipe['emitterDurationSeconds']*max(1,recipe['emitterLoopCount'])+recipe['emitterDelaySeconds']
        assert active>0,('no source active window',o)
        detail['timing'].update(startDelaySeconds=o['sourceTimeSeconds'],lifeTimeSeconds=active)
        detail['particle']['maxParticles']=max(detail['particle']['maxParticles'],int(imported.prop(lod.properties,'peakactiveparticles',0)))
        detail['particle']['burstCount']=0
        if o['rendererShape']=='decal':
            typed=next(m for m in modules if m.class_name=='efparticlemoduletypedatadecal')
            near=float(imported.prop(typed.properties,'nearplane',0));far=float(imported.prop(typed.properties,'farplane'))
            assert far>near and imported.prop(typed.properties,'bonlycalcrotationyaw') is True
            rotation=imported.prop(typed.properties,'rotation',{})
            assert rotation.get('pitch',0)==0 and rotation.get('yaw',0)==0,('unsupported decal source tilt',rotation)
            detail['decal']['depth']=(far-near)*.01
            if not any(m.class_name=='particlemodulesize' for m in modules):
                default=imported.prop(typed.properties,'defaultsize');size=[default['x']*.01,default['y']*.01]
                detail['decal']['size']=size;detail['particle']['startSize']=size;detail['particle']['endSize']=size
        if o['rendererShape']=='mesh':
            detail['mesh'].update(modelPreScale=.01,useModelMaterial=imported.mesh_uses_model_material(modules,mappings))
            detail['particle']['billboard']=False
        event=dict(actionCuePayload=cue,eventId=n['notifyId'],actionCueId=n['notifyId'],globalTimeSeconds=o['sourceTimeSeconds'])
        attachment=imported.action_cue_attachment(event)
        if n['sourceType']=='PlayParticleEffect':
            for k in ('position','rotationDegrees','scale'):detail['transform'][k]=copy.deepcopy(cue['localTransform'][k])
        if attachment.get('enabled') and not attachment.get('follow') and attachment.get('sourceAnchorSlotId')=='root':attachment['snapshotRootSourceBasisYawDegrees']=-90
        e=dict(id=o['elementId'],displayName=n['notifyId'].rsplit('/',1)[-1]+' | '+o['sourceEmitter'].rsplit('.',1)[-1],groupId=f"kouku.{o['actionId']}.full.restore",
            sourceNode=n['notifyId']+'|'+o['sourceEmitter'],visible=bool(imported.prop(lod.properties,'benabled',True)),kind=o['kind'],resources=[],
            material=dict(templateId='effect.source_material',sourceMaterialPath=o['sourceMaterial'],renderProfile='alpha_two_sided_depth_read',sourceProfile=dict(enabled=False)),
            actionCueAttachment=attachment,transformInheritance=dict(enabled=False,masterElementId=''),detail=detail,sourceRecipe=recipe,
            sourcePresentation=imported.default_source_presentation())
        e['sourcePresentation'].update(sourceObjectPath=o['sourceEmitter'],sourceActionCueId=n['notifyId'],sourceEventId=n['notifyId'],sourceTimeSeconds=o['sourceTimeSeconds'])
        if material_patch and o['kind']!='light':
            assert o['elementId'] in native,('missing native material occurrence',o)
            e['material']=copy.deepcopy(native[o['elementId']])
            assert e['material']['sourceMaterialPath']==o['sourceMaterial']
            detail['uv'].update(start=[0,0],speed=[0,0],wave=False,sequence=False)
            detail['color']['emissiveIntensity']=1
        if o['sourceMesh']:
            source_name=o['sourceMesh'].rsplit('.',1)[-1]
            e['resources']=[dict(slotId='meshModel',assetId=f'Effect/KoukuSaydon/FullRestore/Meshes/{source_name}.wmodel')]
        if o['rendererShape']=='animationTrail':
            catalog=read(ROOT/'Data/Actors/BossCatalog.json')
            actor=next(a for a in catalog['bosses'] if a['archetypeId']=='BOSS_KAKULSAYDON_G1_SAYDON')
            assert actor['bodyModel']=='Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel'
            body_scale=float(actor['bodyModelPreScale'])/.01
            assert math.isfinite(body_scale) and body_scale>0
            detail['transform']['scale']=[v*body_scale for v in detail['transform']['scale']]
            e['runtimeCarrier']=dict(formatVersion=1,kind='animationTrailBakedEdgeV1',admission='bounded',historyId=history['historyId'])
            detail['trail'].update(maxPoints=len(history['samples']),pointLifeTimeSeconds=max(detail['particle']['lifeTimeSeconds']),sampleIntervalSeconds=min(b['relativeTimeSeconds']-a['relativeTimeSeconds'] for a,b in zip(history['samples'],history['samples'][1:])),minimumDistance=0,faceCamera=False)
            e['actionCueAttachment']['enabled']=False
            recipe['enabled']=False
            e['sourcePresentation'].update(enabled=True,profileId='kouku.animation-trail-baked-edge-history.v1',status='reconstructed')
            assert abs(detail['timing']['lifeTimeSeconds']-history['playbackClampSeconds'])<1e-6
        elif o['rendererShape']=='light':
            e['material']['templateId']='effect.standard'
            # This original light is one particle burst with zero continuous rate.
            # The existing typed-light carrier owns that particle's lifetime,
            # not the longer PS notify activation window.
            light_spawn=next(m for m in recipe['modules'] if m['className']=='particlemodulespawn')
            rate=next(d for d in light_spawn['distributions'] if d['propertyPath']=='rate')
            assert distribution_bounds(rate)==(0,0) and len(recipe['bursts'])==1
            burst=recipe['bursts'][0];assert burst['countMinimum']==burst['countMaximum']==1
            assert recipe['emitterLoopCount']==1 and len(life)==1 and detail['particle']['lifeTimeSeconds'][0]==detail['particle']['lifeTimeSeconds'][1]
            detail['timing']['startDelaySeconds']+=recipe['emitterDelaySeconds']+burst['timeSeconds']
            detail['timing']['lifeTimeSeconds']=detail['particle']['lifeTimeSeconds'][0]
            typed=next(m for m in modules if 'typedatalight' in m.class_name)
            component=next(index.objects[k] for p,k in typed.references if p=='pointlightcomponent')
            color=imported.prop(component.properties,'lightcolor')
            detail['light'].update(enabled=True,profileId='light.point.reconstructed.v1',status='reconstructed_profile',range=imported.prop(component.properties,'radius')*.01,
                intensity=imported.prop(component.properties,'brightness'),color=[color[k]/255 for k in ('r','g','b')]+[1],ambient=[0,0,0,1],falloffExponent=imported.prop(component.properties,'falloffexponent'))
            # The typed Light path samples over-life modules directly, while
            # initial particle Color is carried by the ordinary Detail tint.
            initial={d['propertyPath']:d for m in recipe['modules'] if m['className']=='particlemodulecolor' for d in m['distributions']}
            if 'startcolor' in initial:
                d=initial['startcolor'];assert not d['lookupTable'] and not d['keys'] and d['defaultMinimum']==d['defaultMaximum']
                detail['color']['multiply'][:3]=d['defaultMinimum'][:3]
            if 'startalpha' in initial:
                d=initial['startalpha'];assert not d['lookupTable'] and not d['keys'] and d['defaultMinimum']==d['defaultMaximum']
                detail['color']['multiply'][3]=d['defaultMinimum'][0]
        docs[o['actionId']]['elements'].append(e)
        # One source notify can request the same PS independently at several
        # exact source sockets. Each socket gets the complete emitter stream.
        for anchor in cue.get('attachment',{}).get('runtimeAnchors',[])[1:]:
            assert anchor['resolutionStatus'] in ('EXACT_SOURCE_SOCKET','EXACT_SOURCE_BONE'),anchor
            extra=copy.deepcopy(e);extra['id']+='.'+anchor['sourceAnchorName'].lower()
            extra['displayName']+=' | '+anchor['sourceAnchorName']
            extra['actionCueAttachment'].update(sourceAnchorSlotId=anchor['sourceAnchorName'],runtimeAnchorSlotId=anchor['runtimeAnchorSlotId'],runtimeBoneName=anchor['runtimeBoneName'],socketLocalTransform=copy.deepcopy(anchor['socketLocalTransform']))
            docs[o['actionId']]['elements'].append(extra)
        changes.append(dict(**o,activeSeconds=active,particleLifeSeconds=detail['particle']['lifeTimeSeconds'],endSeconds=o['sourceTimeSeconds']+active+max(detail['particle']['lifeTimeSeconds']),parameters=parameters,attachment=attachment))
    for doc in docs.values():
        assert len({e['id'] for e in doc['elements']})==len(doc['elements'])
        if material_patch:
            for e in doc['elements']:
                for resource in e['resources']+e['material'].get('sourceProfile',{}).get('textures',[]):
                    assert (ROOT/'Client/Bin/Resources'/resource['assetId']).is_file(),resource
        write(destination/(doc['effectAssetId']+'.effect.json'),doc)
    write(evidence/'runtime_occurrences.json',[dict(elementId=e['id'],sourceEmitter=e['sourcePresentation']['sourceObjectPath'],sourceNotify=e['sourcePresentation']['sourceEventId'],sourceMaterial=e['material']['sourceMaterialPath'],rendererShape=e['sourceRecipe']['rendererShape'],attachment=e['actionCueAttachment']) for d in docs.values() for e in d['elements']])
    duration={str(sid):math.ceil(max(c['endSeconds'] for c in changes if c['actionId']==sid)*1000) for sid in SELECTED}
    write(evidence/'projection.json',dict(durationMs=duration,changes=changes,manualVisualValidation='USER_PENDING'))
    print('Projected durationMs',duration)

if __name__ == '__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--evidence-root',type=Path,default=ROOT/'out/KoukuGate1FullRestore20260911');parser.add_argument('--output',type=Path);parser.add_argument('--required-textures',type=Path);parser.add_argument('--prepare-geometry',action='store_true');parser.add_argument('--native-material-patch',type=Path);options=parser.parse_args()
    index,notifies,occurrences,records=acquire(options.evidence_root)
    print('Source occurrences',dict(collections.Counter(o['actionId'] for o in occurrences)))
    if options.output:project(options.evidence_root,index,notifies,occurrences,records,options.output,options.native_material_patch)
    if options.required_textures:prepare_textures(options.evidence_root,options.required_textures)
    if options.prepare_geometry:prepare_geometry(options.evidence_root)

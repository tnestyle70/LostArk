"""Regenerate the selected original Gate 3 fire-grid native material inputs.

Uses the shared UE3 shader-map reader and existing native HLSL generator. The
private candidate header is review output; this command never edits shared C++
or shader groups. It reads original packages and closes all texture dependencies.
"""
import argparse, functools, hashlib, json, pathlib, re, shutil, struct, subprocess, sys
ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'Tools/LevelPlacementExtractor'), str(ROOT / 'Tools/EffectPipeline')]
from extract_ue3_effect_material_closure import *
from extract_ue3_placements import resolve_physical_package
import extract_ue3_material_shader_maps as sm
import build_kouku_gate1_full_restore as restore
from build_warlord_asvf_full_restore import norm, merge
from native_material_tables import read_material_bytes

SOURCE = restore.SOURCE / 'CanonicalSource/Effect'
RELEASE = pathlib.Path('C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC')
UMODEL = pathlib.Path('C:/LostArkExtract/Tooling/UEViewerLostArkV7_20260220/runtime/umodel_lostark_v7.exe')
packages = {p.parent.name.lower(): p for p in (SOURCE / 'Closure/SourcePackages').glob('*/*.upk')}
for p in SOURCE.glob('Packages/*/Source/*.upk'):
    packages.setdefault(p.parent.parent.name.lower(), p)

@functools.lru_cache(None)
def pkg(name):
    if name not in packages:
        packages[name] = resolve_physical_package(UMODEL, RELEASE, name, 'kr')
    return load_package(packages[name], LOSTARK_KR_AES_KEY)
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


def prepare(evidence):
    out = evidence / 'native'
    out.mkdir(parents=True, exist_ok=True)
    write = lambda name, value: restore.write(out / name, value)
    occurrences = [o for o in restore.read(evidence / 'source_occurrences.json') if o['rendererShape'] != 'light']
    assert len(occurrences) == 17
    rows = [material(p) for p in sorted({o['sourceMaterial'] for o in occurrences})]
    cache = sm.package_tables(RELEASE / 'EV2LG3OVEH3HGV7THTFFTM7TOKMCC.upk')
    layout = sm.parse_shader_code_layout(cache)
    scan_path = out / 'material_map_scan.json'
    scans = restore.read(scan_path) if scan_path.exists() else {}
    missing = sorted({r['baseId'] for r in rows} - set(scans))
    if missing:
        scans.update(sm.scan_base_material_contexts(cache, layout, missing))
    write('material_map_scan.json', scans)
    textures = {}
    for row in rows:
        equality = row['mic']['engineEqualityStaticParameterSetSha256']
        context = sm.select_unique_map_context(scans[row['baseId']], equality)
        row['materialMap'] = sm.parse_material_map(cache, layout, context, equality)
        row['mapKey'] = equality
        parent = obj(row['parentMaterial'])
        package = pkg(parent['package'])
        tail = parent['tail']
        assert len(tail) >= 40 and struct.unpack_from('<I', tail)[0] == 1 and tail[16:32].hex() == row['baseId']
        count = struct.unpack_from('<I', tail, 36)[0]
        assert count <= 256 and len(tail) >= 40 + 4 * count
        references = struct.unpack_from('<' + 'i' * count, tail, 40)
        row['effectiveTextures'] = []
        for index, expression in enumerate(row['materialMap']['uniformExpressionSet']['pixelTexture2DExpressions']):
            name = expression.get('parameterName')
            number = expression.get('parameterNameNumber', 0)
            if number:
                name += '_' + str(number - 1)
            path = row['textureOverrides'].get(name) or fullref(parent['package'], package, references[expression['referencedTextureIndex']])
            texture = obj(path)
            assert texture['className'] == 'texture2d', (path, texture['className'])
            row['effectiveTextures'].append(dict(index=index, parameterName=name, sourceObjectPath=path, properties=texture['properties']))
            textures[path] = texture['properties']
    write('native_material_inputs.json', dict(materials=rows))
    write('required_native_textures.json', textures)
    by_material = {r['sourceMaterial']: r for r in rows}
    records = restore.read(evidence / 'source_module_inputs.json')['records']
    records.update({r['fullPath']: r for r in restore.read(evidence / 'source_class_defaults.json')['records']})
    @functools.lru_cache(None)
    def effective(path):
        row = records[path]
        parent = row.get('archetypeFullPath')
        if not parent and not path.startswith(('engine.', 'efgame.', 'core.')):
            cls = row['classPath']
            parent = cls.split('.')[0] + '.default__' + cls.split('.')[-1]
        return merge(effective(parent) if parent else {}, norm(row['properties']))
    def value(properties, key, default=None):
        result = properties.get(key, default)
        return result.get('value') if isinstance(result, dict) and 'value' in result else result
    selected, refs, shader_material, vf_decisions = {}, {}, {}, []
    for occurrence in occurrences:
        shape = occurrence['rendererShape']
        row = by_material[occurrence['sourceMaterial']]
        required = next(p for p in occurrence['moduleOrder'] if 'particlemodulerequired' in p)
        properties = effective(required)
        dynamic = any('parameterdynamic' in p for p in occurrence['moduleOrder'])
        if shape == 'mesh':
            vf = 'flocalvertexfactory'
        else:
            assert shape == 'sprite'
            subuv = str(value(properties, 'interpolationmethod', 'psuvim_none')).lower() != 'psuvim_none'
            vf = 'fparticle' + ('subuv' if subuv else '') + ('offsetcenter' if value(properties, 'boffsetcenter', False) else '')
            vf += ('dynamicparameter' if dynamic else '') + 'vertexfactory'
        requested_vf = vf
        available = {v['vertexFactoryType'] for v in row['materialMap']['vertexFactories']}
        # Some source MICs cache only the otherwise identical sprite variant
        # with dynamic inputs. Preserve that original material permutation;
        # the existing particle carrier supplies its ordinary default inputs.
        # This does not substitute mesh or beam shaders for a sprite layout.
        if vf not in available and shape == 'sprite' and not dynamic:
            candidate = vf.removesuffix('vertexfactory') + 'dynamicparametervertexfactory'
            if candidate in available:
                vf = candidate
        factories = [v for v in row['materialMap']['vertexFactories'] if v['vertexFactoryType'] == vf]
        assert len(factories) == 1, (occurrence['elementId'], vf)
        vf_decisions.append(dict(elementId=occurrence['elementId'], sourceRequiredModule=required,
            dynamicParameterModule=dynamic, requestedVF=requested_vf, selectedOriginalMaterialVF=vf,
            status='ORIGINAL_MATERIAL_VF' if requested_vf == vf else 'ORIGINAL_SPRITE_DYNAMIC_VARIANT_DEFAULT_INPUTS'))
        shaders = factories[0]['shaderReferences']
        pixels = [s for s in shaders if s['shaderType'] == 'tbasepasspixelshaderfnolightmappolicyskylight']
        vertices = [s for s in shaders if 'basepassvertexshaderfnolightmappolicy' in s['shaderType'] and 'nodensitypolicy' in s['shaderType']]
        assert len(pixels) == len(vertices) == 1
        pixel, vertex = pixels[0], vertices[0]
        key = (row['sourceMaterial'], vf, shape)
        program = selected.setdefault(key, dict(resolvedMaterial=row['sourceMaterial'], sourceVF=vf,
            sourceVS=vertex['shaderIdHex'], sourcePS=pixel['shaderIdHex'], rendererShape=shape, occurrences=[]))
        program['occurrences'].append(occurrence['elementId'])
        for shader in (pixel, vertex):
            refs[shader['shaderIdHex']] = shader
            shader_material[shader['shaderIdHex']] = row
    assert len(selected) == 10
    write('selected_runtime_material_programs.json', dict(programs=list(selected.values()), errors=[]))
    write('source_vf_selections.json', vf_decisions)
    missing_refs = [r for sid, r in refs.items() if not (out / 'full_programs' / (sid + '.json')).exists()]
    if missing_refs:
        (out / 'dxbc').mkdir(exist_ok=True)
        (out / 'full_programs').mkdir(exist_ok=True)
        bytecodes = sm.extract_selected_packed_dxbc(cache, layout, missing_refs)
        pixels = [r for r in missing_refs if 'pixelshader' in r['shaderType']]
        objects = sm.extract_selected_shader_objects(cache, layout, pixels) if pixels else None
        disassembler = sm.D3DDisassembler(sm.DEFAULT_D3DCOMPILER)
        for reference in missing_refs:
            sid = reference['shaderIdHex']
            bytecode = bytecodes[sid]['_bytecode']
            (out / 'dxbc' / (sid + '.dxbc')).write_bytes(bytecode)
            disassembly = disassembler.disassemble(bytecode)
            chunks = sm.dxbc_chunk_payloads(bytecode)
            program = dict(shaderId=sid, disassembly=disassembly,
                inputSignature=sm.parse_dxbc_signature(chunks.get('ISGN', chunks.get('ISG1'))))
            if disassembly['profile'].startswith('ps'):
                declaration = sm.parse_dxbc_declaration_closure(disassembly)
                raw = objects['byShaderId'][sid]
                program['bindings'] = sm.select_unique_native_binding_arrays(raw['_bytes'], raw['logicalOffset'],
                    shader_material[sid]['materialMap']['uniformExpressionCounts'], declaration)
            write('full_programs/' + sid + '.json', program)
    prepare_textures(evidence, out)
    subprocess.run([sys.executable, str(ROOT / 'Tools/EffectPipeline/generate_artist_native_runtime_shader.py'),
        '--source-dir', str(out), '--program-start', '2400', '--profile-domain', 'kouku'], check=True)
    patch_engine_prefix(out)
    import install_kouku_gate1_native_materials as tables
    tables.FIRST, tables.LAST = 2400, 2409
    candidate = out / 'Effect_ArtistMaterial.candidate.h'
    candidate.write_bytes(read_material_bytes(ROOT / 'Client/Public/Effect_ArtistMaterial.h'))
    tables.install(out / 'native_runtime_contract.json', out, candidate)


def patch_engine_prefix(out):
    """Audit live reserved CB0 reads and close one source-qualified color row.

    DXBC retains the register/dataflow contract, but strips original field names.
    The Sprite uniform-color adapter therefore remains a bounded reconstruction
    using the existing particle-color consumer. No authoring alpha is changed.
    """
    from patch_kouku_spider_native_programs import function_blocks
    contract_path = out / 'native_runtime_contract.json'
    contract = restore.read(contract_path)
    materials = restore.read(out / 'native_material_inputs.json')['materials']
    material = next(m for m in materials
        if m['sourceMaterial'] == 'fx_m_mi_k_00.fx_mi.fx_k_me_master_01_09_ad')
    assert material['mapKey'] == '112cedd96706b3decb8221c69ac34e8d4743b0c2c27f236d3fd2b73f53f62c11'
    selected = next(p for p in contract['programs'] if p['program'] == 2409)
    assert (selected['sourcePS'], selected['sourceVS'], selected['sourceVF'], selected['rendererShape']) == (
        'ee5ef69de7de004f8eea2dc7882e5b1d', 'af21e3f3ec2dae46b515c73545c2df86', 'fparticlevertexfactory', 'sprite')
    shader = out / 'Shader_EffectArtistNative.hlsli'
    original = shader.read_text(encoding='utf8')
    before = function_blocks(original)
    assert set(before) == set(range(2400, 2410))
    marker = '    source[0].x=1.f; // Project engine opacity multiplier.'
    color = '    source[1]=input.color; // Bounded Sprite uniform-color ABI adapter; original PS field name is stripped.'
    changed = before[2409]
    if color not in changed:
        assert changed.count(marker) == 1
        changed = changed.replace(marker, marker + '\n' + color)
    result = original.replace(before[2409], changed, 1)
    after = function_blocks(result)
    assert all(before[n] == after[n] for n in before if n != 2409)
    audit = []
    for row in contract['programs']:
        program = row['program']
        owner = next(m for m in materials if m['sourceMaterial'] == row['sourceMaterial'])
        vertex_counts = {k: v for k, v in owner['materialMap']['uniformExpressionCounts'].items() if k.startswith('vertex')}
        assert vertex_counts and all(v == 0 for v in vertex_counts.values()), (program, vertex_counts)
        pixel = restore.read(out / 'full_programs' / (row['sourcePS'] + '.json'))
        bindings = pixel['bindings']
        unowned = bindings['constantBufferClosure']['unownedConstantBuffer0Slots']
        assert unowned == ([0, 1] if program in (2402, 2407, 2409) else [0]), (program, unowned)
        instructions = pixel['disassembly']['instructions']
        last = max(i for i, line in enumerate(instructions) if re.search(r'\bo0\.', line))
        reads = [dict(instructionIndex=i, instruction=line) for i, line in enumerate(instructions[:last + 1])
            if any('cb0[' + str(slot) + ']' in line for slot in unowned)]
        assert reads and all('cb0[0].x' in r['instruction'] or 'cb0[1].' in r['instruction'] for r in reads)
        assert marker in after[program]
        if 1 in unowned:
            assert 'source[1]=input.color;' in after[program]
        if program == 2409:
            assert 'mul_sat r0.y, r0.y, cb0[1].w' in instructions
            assert 'mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz' in instructions
            assert not any(re.search(r'\bv3\.', line) for line in instructions[:last + 1])
        live_inputs = sorted({int(register) for line in instructions[:last + 1]
            for register in re.findall(r'\bv(\d+)\.', line)})
        audit.append(dict(program=program, sourcePS=row['sourcePS'], sourceVS=row['sourceVS'],
            nativeBindingArraysByteOffset=bindings['bindingArraysOffsetInShaderObject'],
            vertexMaterialExpressionCounts=vertex_counts,
            unownedCB0Rows=unowned, reservedReads=reads,
            livePixelInputs=[s for s in pixel['inputSignature'] if s['register'] in live_inputs],
            opacity='CB0[0].x = 1', color='CB0[1] = input.color' if 1 in unowned else 'TEXCOORD1',
            macroUV='NO_ADDITIONAL_RESERVED_CB0_MACRO_UV_READS',
            sourceFieldNames='STRIPPED_FROM_DXBC; REGISTER_AND_DATAFLOW_QUALIFIED'))
    if result != original:
        shader.write_text(result, encoding='utf8')
    selected['enginePrefixAdapter'] = dict(status='BOUNDED_RECONSTRUCTION',
        uniformColor='CB0[1] = existing input.color; same alpha/RGB consumer as source DXBC',
        fieldNameEvidence='Source field name is stripped; register, static permutation, PS/VS and dataflow qualified',
        authoredParametersChanged=False, visualValidation='USER_PENDING')
    restore.write(contract_path, contract)
    restore.write(out / 'engine_prefix_audit.json', dict(programs=audit,
        changedProgram=2409, preservedPeerPrograms=9, authoredParametersChanged=False,
        shaderSha256=hashlib.sha256(shader.read_bytes()).hexdigest(), manualVisualValidation='USER_PENDING'))
    print('Rainbow engine-prefix audit 10 programs; qualified Sprite color row supplied for 2409')


def prepare_textures(evidence, out):
    # This MIC's exact texture is outside the earlier Kouku closure. Export its
    # original Texture2D by object name; do not infer a replacement from pixels.
    relative = 'Effect/KoukuSaydon/FullRestore/Textures/lv_pap_naskas/lv_pap_naskas_bossroom01_d.dds'
    destination = ROOT / 'Client/Bin/Resources' / relative
    export = evidence / 'source_texture_export'
    source = export / 'LV_PAP_NASKAS/Texture2D/lv_pap_naskas_bossroom01_d.dds'
    if not source.is_file():
        command = [str(UMODEL), '-export', '-game=lostark', '-kr', '-nameresolve',
            f'-path={RELEASE}', f'-out={export.resolve()}', '-dds', '-gltf', '-nooverwrite',
            '-obj=lv_pap_naskas_bossroom01_d', 'lv_pap_naskas']
        result = subprocess.run(command, cwd=UMODEL.parent, capture_output=True, text=True,
            encoding='utf8', errors='replace', creationflags=subprocess.CREATE_NO_WINDOW)
        (evidence / 'lv_pap_naskas.export.log').write_text(result.stdout + result.stderr, encoding='utf8')
        assert result.returncode == 0 and source.is_file()
    destination.parent.mkdir(parents=True, exist_ok=True)
    assert not destination.is_file() or destination.read_bytes() == source.read_bytes(), 'Preserve differing resource bytes'
    if not destination.is_file():
        shutil.copyfile(source, destination)
    restore.prepare_textures(out, out / 'required_native_textures.json')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=pathlib.Path, default=ROOT / 'out/KoukuRainbowMatched20260911')
    parser.add_argument('--patch-engine-prefix-only', action='store_true')
    arguments = parser.parse_args()
    if arguments.patch_engine_prefix_only:
        patch_engine_prefix(arguments.evidence_root.resolve() / 'native')
    else:
        prepare(arguments.evidence_root.resolve())

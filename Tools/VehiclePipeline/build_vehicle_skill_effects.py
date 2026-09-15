"""Restore vehicle skill ParticleSystems through the source V1 Effect pipeline.

Source notifies come from the vehicle Action .loa extracts. ParticleSystem
closures are read directly from the installed game packages, reusing the Kouku
full-restore acquisition without its CanonicalSource extract.
"""
from pathlib import Path
import argparse, base64, collections, copy, json, re, struct, sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/EffectPipeline'))
sys.path.insert(0, str(ROOT / 'Tools/LevelPlacementExtractor'))
sys.path.insert(0, str(ROOT / 'Tools/ModelAssetConverter'))
import build_kouku_gate1_full_restore as source
import extract_ue3_placements as ue3
from extract_ue3_placements import resolve_physical_package
from extract_ue3_effect_material_closure import load_package, find_export
from bake_ghost_valtan_animset import read_sections, skeleton_bones

RELEASE = Path('C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC')
UMODEL = Path('C:/Users/95jus/Downloads/umodel_win32/umodel_lostark_v7.exe')
SCRIPT_PACKAGES = {'engine': RELEASE / 'NE1FENCQ4UNE9ZPRENOQS.u', 'efgame': RELEASE / 'NU1V7NCQ4YAE9ZPJVNOQS.u'}
SPEC = ROOT / 'Tools/VehiclePipeline/VehicleSkills.spec.json'
CATALOG = ROOT / 'Data/Actors/VehicleCatalog.json'
RESOURCES = ROOT / 'Client/Bin/Resources'
VEHICLE_TABLE = Path('C:/Users/95jus/Downloads/SourceData/SourceData/LPK/data2/EFGame_Extra/ClientData/TableData/EFTable_Vehicle.db')
SKIN_VARIANT = re.compile(r'-\d+$')
# Body SkeletalMesh of each installed vehicle model (see the vehicle riding RESULTs).
SOURCE_MESHES = {'Terpeion': ('MN_PMSTG_01', 'mesh.mn_pmstg_01_sk'),
    'SereneStarlightBlessing': ('MN_PMSSM_00', 'mesh.mn_pmssm_00_sk'),
    'RainbowMokoboard': ('MN_PMSMK_00', 'mesh.mn_pmsmk_00_sk'),
    'Aufstehen': ('MN_PMSHE_00', 'mesh.mn_pmshe_00_sk'),
    'SeaUnicornTube': ('MN_PMSUT_00', 'mesh.mn_pmsut_00_sk'),
    'AncientMyth': ('MN_PMSDZ_00', 'mesh.mn_pmsdz_00_sk')}


def vehicle_skins():
    import sqlite3
    with sqlite3.connect(VEHICLE_TABLE) as table:
        return {key: model for key, model in table.execute('select PrimaryKey, Model from Vehicle')}


def skin_system(labels, skin):
    # CEFParticleData lists the base system first. Each CEFParticleDataModifier
    # names a skin (EFDLVehi_<mesh>[-N].<mesh>[-N]) and, optionally, the system
    # that skin plays instead. EFTable_Vehicle.Model selects the skin.
    segments, current = [('', [])], None
    for label in labels:
        if label == 'CEFParticleDataModifier':
            current = None
        elif label.startswith('EFDLVehi_') and current is None:
            current = (label, [])
            segments.append(current)
        elif label.startswith("ParticleSystem'"):
            (current or segments[0])[1].append(label[len("ParticleSystem'"):-1])
    base = segments[0][1][0] if segments[0][1] else None
    matched = [systems for name, systems in segments[1:] if name.lower() == skin.lower()]
    if matched and matched[0]:
        return matched[0][0], 'SKIN_MODIFIER_SYSTEM'
    return base, ('SKIN_MODIFIER_BASE_SYSTEM' if matched else 'BASE_SYSTEM') if base else 'NO_SYSTEM_FOR_SKIN'


def fstring(raw, at):
    length = struct.unpack_from('<i', raw, at)[0]
    assert 0 <= length and at + 4 + length <= len(raw), ('FString', at, length)
    if length == 0:
        return '', at + 4
    assert raw[at + 3 + length] == 0, ('FString terminator', at)
    return raw[at + 4:at + 3 + length].decode('ascii'), at + 4 + length


def parameter_table(raw, at, count):
    # Same record as the base CEFParticleData table: name, type, 52 value
    # bytes, a "None" FString sentinel and four zero bytes.
    rows = []
    for index in range(count):
        name, at = fstring(raw, at)
        code = struct.unpack_from('<i', raw, at)[0]
        row = dict(sourceIndex=index, name=name, sourceTypeCode=code)
        if code == 0:
            row.update(type='none', enabled=False)
        elif code == 1:
            row.update(type='scalar', scalarValue=struct.unpack_from('<f', raw, at + 4)[0])
        elif code == 3:
            row.update(type='vector', vectorValue=list(struct.unpack_from('<3f', raw, at + 12)))
        else:
            raise ValueError(('modifier parameter type', name, code))
        sentinel, at = fstring(raw, at + 56)
        assert sentinel == 'None' and raw[at:at + 4] == b'\0' * 4, ('modifier parameter sentinel', name)
        rows.append(row)
        at += 4
    return rows, at


def particle_modifiers(raw):
    # CEFParticleDataModifier: class FString, skin FString, ParticleSystem
    # FString, int32 override flag, int32 parameter count, parameters, int32.
    rows, marker = [], b'\x18\0\0\0CEFParticleDataModifier\0'
    at = raw.find(marker)
    while at >= 0:
        _, cursor = fstring(raw, at)
        skin, cursor = fstring(raw, cursor)
        system, cursor = fstring(raw, cursor)
        flag, count = struct.unpack_from('<ii', raw, cursor)
        parameters, cursor = parameter_table(raw, cursor + 8, count)
        assert flag in (0, 1), ('modifier flag', skin, flag)
        rows.append(dict(skin=skin, sourceFlag=flag,particleSystem=system[len("ParticleSystem'"):-1] if system else '', parameters=parameters))
        at = raw.find(marker, cursor)
    return rows


def with_base_system(raw, system):
    # A notify whose base CEFParticleData plays nothing serializes an empty
    # ParticleSystem FString twelve bytes after the class name. The existing
    # decoder locates the base block by that reference.
    marker = b'\x10\0\0\0CEFParticleData\0'
    at = raw.find(marker)
    assert at >= 0, 'CEFParticleData block is absent'
    slot = at + len(marker) + 12
    if raw[slot:slot + 4] != b'\0' * 4:
        return raw, slot
    reference = f"ParticleSystem'{system}'".encode('ascii') + b'\0'
    return raw[:slot] + struct.pack('<i', len(reference)) + reference + raw[slot + 4:], slot


def without_second_anchor_list(raw, slot):
    # CEFParticleData stores two consecutive anchor name arrays at +52 from the
    # end of its ParticleSystem reference. The shared decoder reads one array
    # and treats an empty second array as four layout bytes, so a populated
    # second array is removed here and returned separately.
    _, base = fstring(raw, slot)
    cursor = base + 52
    first = struct.unpack_from('<i', raw, cursor)[0]
    if first <= 0:
        return raw, []
    cursor += 4
    for _ in range(first):
        _, cursor = fstring(raw, cursor)
    second = struct.unpack_from('<i', raw, cursor)[0]
    if second <= 0:
        return raw, []
    names, end = [], cursor + 4
    for _ in range(second):
        name, end = fstring(raw, end)
        names.append(name)
    return raw[:cursor] + struct.pack('<i', 0) + raw[end:], names


def wmodel_bones(asset_id):
    data = (RESOURCES / asset_id).read_bytes()
    _, sections = read_sections(data)
    return [bone['name'] for bone in skeleton_bones(data, sections)]


def socket_contracts(evidence):
    catalog = {v['archetypeId']: v for v in source.read(CATALOG)['vehicles']}
    spec = source.read(SPEC)
    contracts = {}
    for vehicle in spec['vehicles']:
        logical, mesh_path = SOURCE_MESHES[vehicle['name']]
        package = load_package(resolve_physical_package(UMODEL, RELEASE, logical, 'kr'), ue3.LOSTARK_KR_AES_KEY)
        mesh = source.record_from_export(package, logical.lower(), find_export(package, mesh_path))
        sockets = []
        prop = source.imported.prop
        for order, export_index in enumerate(prop(mesh['properties'], 'sockets')):
            row = source.record_from_export(package, logical.lower(), package.exports[export_index - 1])
            props = row['properties']
            location = prop(props, 'relativelocation', {})
            rotation = prop(props, 'relativerotation', {})
            scale = prop(props, 'relativescale', {})
            position = [float(location.get(k, 0.0)) for k in ('x', 'y', 'z')]
            rotator = [float(rotation.get(k, 0.0)) for k in ('pitch', 'yaw', 'roll')]
            factors = [float(scale.get(k, 1.0)) for k in ('x', 'y', 'z')]
            sockets.append(dict(sourceIndex=order, socketName=prop(props, 'socketname'), boneName=prop(props, 'bonename'),
                sourceObject=row['fullPath'],
                sourceTransform=dict(positionUeUnits=position, rotationUnrealUnits=rotator, scale=factors),
                runtimeLocalTransform=dict(position=[v * 0.01 for v in position],
                    rotationDegrees=[v * 360.0 / 65536.0 for v in rotator], scale=factors),
                transformEvidence='EXPLICIT_SOCKET_PROPERTIES'))
        folded = [s['socketName'].casefold() for s in sockets]
        assert len(folded) == len(set(folded)), ('socket names are not unique', vehicle['name'])
        model = next(v for v in catalog.values() if v['vehicleId'] == vehicle['vehicleId'])['modelAssetId']
        bones = wmodel_bones(model)
        contract = dict(schema='lostark.ue3-skeletal-mesh-sockets', formatVersion=1,
            source=dict(package=logical, skeletalMesh=mesh_path, sourcePackage=str(package.path),
                positionUnitScale=0.01, rotationUnitScaleDegrees=360.0 / 65536.0),
            runtimeModel=dict(assetId=model, bones=bones), sockets=sockets)
        source.write(evidence / 'sockets' / (vehicle['name'] + '.socket-contract.json'), contract)
        contracts[vehicle['name']] = contract
    return contracts


def decode_vehicle_cue(decode, payload, contract):
    raw = base64.b64decode(payload['data'])
    system = payload['vehicleParticleSystem']
    patched, slot = with_base_system(raw, system)
    patched, second_anchors = without_second_anchor_list(patched, slot)
    cue = decode('PlayParticleEffect', dict(payload, data=base64.b64encode(patched).decode('ascii')), contract)
    cue['attachment']['secondAnchorNames'] = second_anchors
    # The two arrays are bone names then socket names (98523: B_*/Bip001-*
    # then FX_*_Hand/Foot). Every entry is its own emitter stream.
    sockets = {s['socketName'].casefold(): s for s in contract['sockets']}
    for name in second_anchors:
        socket = sockets.get(name.casefold())
        cue['attachment']['runtimeAnchors'].append(dict(sourceAnchorName=name, runtimeAnchorSlotId=name,
            runtimeBoneName=socket['boneName'] if socket else '',
            socketLocalTransform=socket['runtimeLocalTransform'] if socket else dict(position=[0.0] * 3, rotationDegrees=[0.0] * 3, scale=[1.0] * 3),
            resolutionStatus='EXACT_SOURCE_SOCKET' if socket else 'MISSING_SOURCE_SOCKET'))
        cue['attachment']['runtimeAnchorSlotIds'].append(name)
    modifiers = particle_modifiers(raw)
    matched = [m for m in modifiers if m['skin'].lower() == payload['vehicleSkin'].lower()]
    assert len(matched) <= 1, ('duplicate skin modifier', payload['vehicleSkin'])
    if matched and matched[0]['parameters']:
        cue['baseParameterOverrides'] = cue['parameterOverrides']
        cue['parameterOverrides'] = matched[0]['parameters']
    cue['sourceParticleSystem'] = f"ParticleSystem'{system}'"
    cue['vehicleSkinSelection'] = dict(skin=payload['vehicleSkin'], selection=payload['vehicleSelection'],
        modifierMatched=bool(matched), modifierParameters=len(matched[0]['parameters']) if matched else 0)
    # Anchors that name a mesh bone outside the socket table still resolve
    # against the installed runtime skeleton.
    bones = {b.casefold(): b for b in contract['runtimeModel']['bones']}
    for anchor in cue['attachment']['runtimeAnchors']:
        runtime = bones.get((anchor['runtimeBoneName'] or anchor['sourceAnchorName']).casefold())
        if runtime is None and anchor['sourceAnchorName'] != anchor['sourceAnchorName'].strip():
            # Aufstehen 97330 Floor-react names "b_effectroot " with a trailing
            # space. Resolved to the installed floor effect bone and recorded.
            runtime = bones.get(anchor['sourceAnchorName'].strip().casefold())
            if runtime:
                anchor.update(runtimeAnchorSlotId=anchor['sourceAnchorName'].strip(), resolutionStatus='EXACT_SOURCE_BONE',
                    sourceNameNormalization='TRAILING_WHITESPACE_STRIPPED')
        if anchor['resolutionStatus'] == 'MISSING_SOURCE_SOCKET' and runtime:
            anchor.update(runtimeBoneName=runtime, resolutionStatus='EXACT_SOURCE_BONE')
        elif anchor['resolutionStatus'] == 'EXACT_SOURCE_BONE' and anchor.get('sourceNameNormalization'):
            anchor['runtimeBoneName'] = runtime
        elif anchor['resolutionStatus'] != 'MISSING_SOURCE_SOCKET':
            anchor['runtimeBoneName'] = runtime or ''
            if not runtime:
                anchor['resolutionStatus'] = 'MISSING_RUNTIME_BONE'
    primary = cue['attachment']['runtimeAnchors'][0] if cue['attachment']['runtimeAnchors'] else None
    if primary:
        cue['attachment'].update(runtimeBoneName=primary['runtimeBoneName'], runtimeResolutionStatus=primary['resolutionStatus'])
    return cue


def selected_actions(actions_root, evidence):
    spec = source.read(SPEC)
    skins = vehicle_skins()
    actions, selected, stage_rows, skin_rows = [], {}, [], []
    for vehicle in spec['vehicles']:
        document = source.read(actions_root / (vehicle['name'] + '.action-effects.json'))
        clip_names = {s['skillId']: [c['clip'].lower() for c in s['clips']] for s in vehicle['skills']}
        skin = skins[vehicle['vehicleId']]
        for skill in vehicle['skills']:
            action = next(a for a in document['actions'] if a['actionId'] == skill['skillId'])
            for stage in action['stages']:
                look = next((n['serializedLabels'][0] for n in stage['notifies'] if n['sourceType'] == 'LookInfoAnim'), None)
                clip = (stage['animationClips'][0]['clipName'] if stage['animationClips'] else look or '').lower()
                assert clip in clip_names[skill['skillId']], (vehicle['name'], skill['skillId'], stage['stageIndex'], clip)
                notifies = []
                for notify in stage['notifies']:
                    if notify['sourceType'] != 'PlayParticleEffect':
                        continue
                    system, selection = skin_system(notify['serializedLabels'], skin)
                    skin_rows.append(dict(vehicle=vehicle['name'], skin=skin, notifyId=notify['notifyId'],
                        selection=selection, particleSystem=system))
                    if system is None:
                        continue
                    assert system.lower() in {r['objectPath'].lower() for r in notify['assetReferences']}, (notify['notifyId'], system)
                    row = copy.deepcopy(notify)
                    row['assetReferences'] = [dict(className='ParticleSystem', objectPath=system)]
                    row['serializedPayload'].update(vehicleName=vehicle['name'], vehicleSkin=skin,
                        vehicleParticleSystem=system, vehicleSelection=selection)
                    notifies.append(row)
                stage_rows.append(dict(vehicle=vehicle['name'], vehicleId=vehicle['vehicleId'], skillId=skill['skillId'],
                    inputSlot=skill['inputSlot'], stageIndex=stage['stageIndex'], clip=clip,
                    clipIndex=clip_names[skill['skillId']].index(clip), particleNotifies=len(notifies)))
                if not notifies:
                    continue
                identity = skill['skillId'] * 10 + stage['stageIndex']
                actions.append(dict(actionId=identity, stages=[dict(stageIndex=0, animationClips=[], notifies=notifies)]))
                selected[identity] = ([0], f"{vehicle['name']} {skill['inputSlot']} stage {stage['stageIndex']}")
    path = evidence / 'selected_source_actions.json'
    source.write(path, dict(sourceKind='VEHICLE_ACTION_PARTICLE_NOTIFIES', actions=actions))
    source.write(evidence / 'source_stages.json', stage_rows)
    source.write(evidence / 'source_skin_selection.json', skin_rows)
    return path, selected


def acquire(evidence, actions_root):
    contracts = socket_contracts(evidence)
    source.ACTION, source.SELECTED = selected_actions(actions_root, evidence)
    source.GRAPH = evidence / 'no-canonical-particle-graph'
    source.source_package = lambda domain, logical: SCRIPT_PACKAGES[logical]
    decode = source.decode_typed_payload
    source.decode_typed_payload = lambda kind, payload, *args: decode_vehicle_cue(decode, payload, contracts[payload['vehicleName']])
    result = source.acquire(evidence, lambda logical: resolve_physical_package(UMODEL, RELEASE, logical, 'kr'))
    index, notifies, occurrences, _ = result
    systems = sorted({ref['objectPath'].lower() for n in notifies for ref in n['assetReferences']})
    emitting = {o['sourceSystem'] for o in occurrences}
    empty = source.read(evidence / 'source_empty_emitters.json')
    anchors = [dict(vehicle=n['serializedPayload']['vehicleName'], notifyId=n['notifyId'], **a)
        for n in notifies for a in n['cue']['attachment']['runtimeAnchors']]
    source.write(evidence / 'source_anchor_resolution.json', anchors)
    summary = dict(particleNotifies=len(notifies), systems=len(systems), emitters=len(occurrences),
        emptyEmitters=len(empty), systemsWithoutEmitters=[s for s in systems if s not in emitting],
        materials=len({o['sourceMaterial'] for o in occurrences}),
        shapes=dict(collections.Counter(o['rendererShape'] for o in occurrences)),
        packages=sorted({s.split('.')[0] for s in systems}),
        sockets={name: len(c['sockets']) for name, c in contracts.items()},
        anchorResolution=dict(collections.Counter(a['resolutionStatus'] for a in anchors)),
        secondAnchorLists=[dict(notifyId=n['notifyId'], first=n['cue']['attachment']['sourceAnchorNames'],
            second=n['cue']['attachment']['secondAnchorNames']) for n in notifies if n['cue']['attachment']['secondAnchorNames']],
        skinParameterOverrides=sum(n['cue']['vehicleSkinSelection']['modifierParameters'] > 0 for n in notifies))
    source.write(evidence / 'source_summary.json', summary)
    print(json.dumps(summary, ensure_ascii=False))
    return result


D3DCOMPILER = Path('C:/Program Files (x86)/Windows Kits/10/bin/10.0.26100.0/x64/d3dcompiler_47.dll')
STARTUP_PACKAGE = 'startup'
TEXTURE_ROOT = 'Effect/Vehicle/FullRestore/Textures'


def native_materials(evidence, first, last):
    """Recover original native programs with the Kouku pattern pipeline.

    Only this process's module globals are redirected: the installed game
    packages replace CanonicalSource, and textures land under Effect/Vehicle.
    The generated tables are written to a candidate header inside evidence.
    """
    import build_kouku_pattern_native as pattern
    import build_kouku_gate3_rainbow_native as rainbow
    rainbow.UMODEL = pattern.UMODEL = UMODEL
    # The shared disassembler pins the 10.0.22621 d3dcompiler_47.dll, which is
    # absent on this PC. Pin the installed 10.0.26100 x64 copy instead.
    import extract_artist_31470_main_ref_shader_cache as shader_cache
    shader_cache.EXPECTED_D3DCOMPILER = dict(fileName=D3DCOMPILER.name, byteSize=D3DCOMPILER.stat().st_size,
        sha256=shader_cache.digest_file(D3DCOMPILER))
    pattern.sm.DEFAULT_D3DCOMPILER = D3DCOMPILER
    original_pkg = pattern._source_package

    @__import__('functools').lru_cache(None)
    def package(name):
        if name in pattern._startup_groups or name == STARTUP_PACKAGE:
            if STARTUP_PACKAGE not in rainbow.packages:
                rainbow.packages[STARTUP_PACKAGE] = resolve_physical_package(UMODEL, RELEASE, STARTUP_PACKAGE, 'kr')
            return original_pkg(STARTUP_PACKAGE)
        return original_pkg(name)
    pattern.pkg = rainbow.pkg = package

    root = evidence / 'material'
    root.mkdir(parents=True, exist_ok=True)
    excluded = []
    occurrences = []
    for occurrence in source.read(evidence / 'source_occurrences.json'):
        if not occurrence['sourceMaterial'] and occurrence['rendererShape'] != 'mesh':
            excluded.append(dict(elementId=occurrence['elementId'], sourceEmitter=occurrence['sourceEmitter'],
                rendererShape=occurrence['rendererShape'], reason='SOURCE_NULL_MATERIAL_NON_MESH_EMITTER'))
            continue
        occurrences.append(occurrence)
    source.write(root / 'source_occurrences.json', occurrences)
    source.write(root / 'native_input_exclusions.json', excluded)
    for name in ('source_module_inputs.json', 'source_class_defaults.json'):
        target = root / name
        if not target.is_file() or target.read_bytes() != (evidence / name).read_bytes():
            target.write_bytes((evidence / name).read_bytes())

    def textures(material_evidence, out):
        required = source.read(out / 'required_native_textures.json')
        export_root = material_evidence / 'source_texture_export'
        by_name = {}
        for path in RESOURCES.rglob('*.dds'):
            by_name.setdefault((path.parent.name.lower(), path.stem.lower()), []).append(path)
        import shutil, subprocess
        for key in required:
            package_name, relative = key.split('.', 1)
            name = relative.rsplit('.', 1)[-1]
            destination = RESOURCES / TEXTURE_ROOT / package_name / (name + '.dds')
            existing = by_name.get((package_name, name), [])
            if any(p.relative_to(RESOURCES).parts[0] == 'Effect' for p in existing):
                continue
            if existing:
                source_file = sorted(existing, key=lambda p: (len(str(p)), str(p)))[0]
            else:
                matches = [p for p in export_root.rglob('*.dds') if p.stem.lower() == name] if export_root.exists() else []
                if not matches:
                    export_root.mkdir(parents=True, exist_ok=True)
                    physical = str(package(package_name).path) if package_name in pattern._startup_groups else package_name
                    result = subprocess.run([str(UMODEL), '-export', '-game=lostark', '-kr', '-nameresolve', f'-path={RELEASE}',
                        f'-out={export_root.resolve()}', '-dds', '-nooverwrite', f'-obj={name}', physical],
                        cwd=UMODEL.parent, capture_output=True, text=True, encoding='utf8', errors='replace',
                        creationflags=subprocess.CREATE_NO_WINDOW)
                    (export_root / (package_name + '.' + name + '.log')).write_text(result.stdout + result.stderr, encoding='utf8')
                    assert result.returncode == 0, (key, result.stdout[-1000:])
                    matches = [p for p in export_root.rglob('*.dds') if p.stem.lower() == name]
                if not matches:
                    tgas = [p for p in export_root.rglob('*.tga') if p.stem.lower() == name]
                    assert len(tgas) == 1, (key, tgas)
                    from PIL import Image
                    image = Image.open(tgas[0]).convert('RGBA')
                    converted = tgas[0].with_suffix('.dds')
                    image.save(converted, format='DDS')
                    assert Image.open(converted).convert('RGBA').tobytes() == image.tobytes()
                    matches = [converted]
                assert len(matches) == 1, (key, matches)
                source_file = matches[0]
            destination.parent.mkdir(parents=True, exist_ok=True)
            assert not destination.exists() or destination.read_bytes() == source_file.read_bytes(), destination
            if not destination.exists():
                shutil.copyfile(source_file, destination)
        source.prepare_textures(out, out / 'required_native_textures.json')
    pattern.prepare_textures = textures
    pattern.prepare(root, first, last)
    contract = source.read(root / 'native' / 'native_runtime_contract.json')
    failures = source.read(root / 'native' / 'source_material_failures.json')
    summary = dict(programs=len(contract['programs']), deferred=len(contract['deferredPrograms']), sourceFailures=len(failures),
        excludedOccurrences=len(excluded), first=first, last=last,
        shapes=dict(collections.Counter(p['rendererShape'] for p in contract['programs'])),
        blends=dict(collections.Counter(p['nativeBlend'] for p in contract['programs'])))
    source.write(root / 'native_summary.json', summary)
    print(json.dumps(summary, ensure_ascii=False))


VEHICLE_NATIVE_FIRST, VEHICLE_NATIVE_LAST = 3712, 3967


def install_native(evidence):
    """Append the recovered vehicle programs to the installed native corpus.

    Mirrors install_kouku_gate1_native_shaders.install for base and original
    distortion passes, but keeps every installed Kouku block as supplied input.
    Programs the generator deferred stay out and are listed for projection.
    """
    import install_kouku_gate1_native_materials as materials
    import install_kouku_gate1_native_shaders as shaders
    folder = evidence / 'material' / 'native'
    reviewed = evidence / 'material' / 'reviewed'
    merged = source.read(folder / 'merged_native_runtime_contract.json')
    rows = merged['programs']
    owned = {row['program'] for row in rows}
    assert len(owned) == len(rows) and owned <= set(range(VEHICLE_NATIVE_FIRST, VEHICLE_NATIVE_LAST + 1))
    source.write(reviewed / 'native_runtime_contract.json', dict(programs=rows, deferredPrograms=[]))
    source.write(reviewed / 'native_deferred_programs.json', merged['deferredPrograms'])
    materials.install(reviewed / 'native_runtime_contract.json', reviewed, ROOT / 'Client/Public/Effect_ArtistMaterial.h')

    generated = (folder / 'Shader_EffectArtistNative.hlsli').read_text(encoding='utf8')
    bodies = {int(i): block for block, i in re.findall(
        r'(#ifndef ARTIST_NATIVE_MODEL_ONLY\n// [^\n]+\nfloat4 ArtistNative(\d+)\(ARTIST_NATIVE_INPUT input\)\n\{.*?\n\}\n#endif)', generated, re.S)}
    companion = (folder / 'KoukuGenericDistortionPrograms.hlsli').read_text(encoding='utf8')
    distortions = {int(i): block for block, i in re.findall(
        r'(float4 ArtistNative(\d+)Distortion\(ARTIST_NATIVE_INPUT input\)\n\{.*?\n\})', companion, re.S)}
    assert set(bodies) >= owned and {r['program'] for r in rows if r.get('distortionPass')} == set(distortions)

    def carrier(row):
        return {'mesh': 'MESH', 'decal': 'DECAL', 'animationTrail': 'TRAIL', 'animTrail': 'TRAIL', 'ribbon': 'TRAIL',
                'beam': 'TRAIL', 'screenPost': 'SCREEN_POST'}.get(row['rendererShape'], 'PARTICLE')

    def guard(row):
        return ' && '.join(f'!defined(EFFECT_NATIVE_{kind}_CARRIER)' for kind in
                           ('MESH', 'PARTICLE', 'DECAL', 'TRAIL', 'SCREEN_POST') if kind != carrier(row))

    shader_root = ROOT / 'Client/Bin/ShaderFiles'
    installed = '\n'.join(path.read_text(encoding='utf8') for path in sorted(shader_root.glob('Shader_EffectKoukuNativeGroup*.hlsli')))
    blocks = [block for block in shaders.conditional_blocks(installed)
              if int(re.search(r'float4 ArtistNative(\d+)', block)[1]) not in owned]
    cases = [block for block in shaders.conditional_blocks(shaders.installed_kouku_cases(shader_root))
             if int(re.search(r'case (\d+)u:', block)[1]) not in owned]
    for row in sorted(rows, key=lambda r: r['program']):
        number = row['program']
        blocks.append('#if ' + guard(row) + '\n' + bodies[number] + '\n#endif\n')
        opaque = '1.f' if row['nativeBlend'] in ('blend_additive', 'blend_masked', 'blend_opaque') else 'nativeColor.a'
        condition = f'#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == {number // 64 * 64}) && {guard(row)}\n'
        if row.get('distortionPass'):
            blocks.append('#if !defined(ARTIST_NATIVE_MODEL_ONLY) && ' + guard(row) + '\n' + distortions[number] + '\n#endif\n')
            cases.append(condition + f'''    case {number}u:
    {{
        nativeColor=ArtistNative{number}(input);
        const float4 accumulated=ArtistNative{number}Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,{opaque});
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }}
#endif
''')
        else:
            flag = 'true' if opaque == '1.f' else 'false'
            cases.append(condition + f'    case {number}u: nativeColor=ArtistNative{number}(input); opaqueCoverage={flag}; break;\n#endif\n')
    summary = shaders.install_partitioned_groups('\n'.join(blocks), ''.join(cases))
    source.write(reviewed / 'native_shader_installation.json', dict(programs=summary[0], groups=summary[1], carriers=summary[2],
        vehiclePrograms=len(rows), distortionPasses=len(distortions), deferred=len(merged['deferredPrograms'])))
    print('Installed native programs/groups/carriers', summary, 'vehicle', len(rows))


MESH_ROOT = 'Effect/Vehicle/FullRestore/Meshes'
KOUKU_MESH_ROOT = 'Effect/KoukuSaydon/FullRestore/Meshes'
CONVERTER = ROOT / 'Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe'
AUTHORED = ROOT / 'Data/Effects/Authored'


def prepare_geometry(evidence, occurrences):
    """Source mesh WModels with the Kouku full-restore scale contract (x100, preScale 0.01).

    A mesh already cooked by the Kouku full restore is reused by path. Others are
    exported from the game package and cooked through the same geometry contract.
    """
    import subprocess
    import cook_wmodel_geometry_contract as geometry
    digest = lambda path: __import__('hashlib').sha256(path.read_bytes()).digest()
    rows, assets = [], {}
    for source_mesh in sorted({o['sourceMesh'] for o in occurrences if o['sourceMesh']}):
        package_name, relative = source_mesh.split('.', 1)
        name = relative.rsplit('.', 1)[-1]
        kouku = RESOURCES / KOUKU_MESH_ROOT / (name + '.wmodel')
        destination = RESOURCES / MESH_ROOT / (name + '.wmodel')
        if kouku.is_file():
            assets[source_mesh] = kouku.relative_to(RESOURCES).as_posix()
            rows.append(dict(sourceObject=source_mesh, assetId=assets[source_mesh], mode='REUSE_KOUKU_FULL_RESTORE_GEOMETRY'))
            continue
        export = evidence / 'geometry' / 'export' / package_name
        gltfs = [p for p in export.rglob('*.gltf') if p.stem.lower() == name]
        if not gltfs:
            export.mkdir(parents=True, exist_ok=True)
            result = subprocess.run([str(UMODEL), '-export', '-game=lostark', '-kr', '-nameresolve', f'-path={RELEASE}',
                f'-out={export.resolve()}', '-gltf', '-nooverwrite', f'-obj={name}', package_name], cwd=UMODEL.parent,
                capture_output=True, text=True, encoding='utf8', errors='replace', creationflags=subprocess.CREATE_NO_WINDOW)
            (export / (name + '.export.log')).write_text(result.stdout + result.stderr, encoding='utf8')
            assert result.returncode == 0, (source_mesh, result.stdout[-1000:])
            gltfs = [p for p in export.rglob('*.gltf') if p.stem.lower() == name]
        assert len(gltfs) == 1, (source_mesh, gltfs)
        legacy = evidence / 'geometry' / 'legacy' / (name + '.wmodel')
        legacy.parent.mkdir(parents=True, exist_ok=True)
        if not legacy.is_file():
            result = subprocess.run([str(CONVERTER), str(gltfs[0]), '-o', str(legacy), '--scale', '100', '--no-auto-textures'],
                capture_output=True, text=True, encoding='utf8', errors='replace', creationflags=subprocess.CREATE_NO_WINDOW)
            (legacy.parent / (name + '.log')).write_text(result.stdout + result.stderr, encoding='utf8')
            assert result.returncode == 0 and legacy.is_file(), (source_mesh, result.stdout[-1000:])
        observation = evidence / 'geometry' / 'observations' / (name + '.json')
        package_path = resolve_physical_package(UMODEL, RELEASE, package_name, 'kr')
        source.write(observation, dict(sourceObject=source_mesh, sourcePackage=str(package_path), sourceGltf=str(gltfs[0]),
            legacyWmodel=str(legacy), sourceGltfSha256=digest(gltfs[0]).hex(), legacyWmodelSha256=digest(legacy).hex(),
            evidence='OBSERVED_GAME_PACKAGE_UMODEL_GLTF_AND_LEGACY_WMODEL'))
        provenance = geometry.GeometryProvenanceEvidence(source_mesh, digest(evidence / 'external_module_closure.json'),
            'OBSERVED_SOURCE_RECEIPT', digest(package_path), digest(CONVERTER), digest(observation), digest(observation))
        payload, receipt = geometry.cook_wmodel_geometry_contract(gltfs[0], legacy, provenance)
        destination.parent.mkdir(parents=True, exist_ok=True)
        if not destination.is_file() or destination.read_bytes() != payload:
            destination.write_bytes(payload)
        source.write(evidence / 'geometry' / 'observations' / (name + '.cook.json'), receipt)
        assets[source_mesh] = destination.relative_to(RESOURCES).as_posix()
        rows.append(dict(sourceObject=source_mesh, assetId=assets[source_mesh], mode='SOURCE_GLTF_WMODEL_GEOMETRY_PARITY'))
    source.write(evidence / 'geometry' / 'installation.json', rows)
    return assets


def played_stages(evidence):
    # Vehicle COMBO chains play each clip once in first-appearance order, so a
    # clip takes its first source stage. Later repeats and branches stay out.
    kept, skipped, seen = {}, [], set()
    for row in source.read(evidence / 'source_stages.json'):
        key = (row['skillId'], row['clip'])
        if key in seen:
            skipped.append(dict(row, reason='CLIP_ALREADY_PLAYED_BY_EARLIER_STAGE'))
            continue
        seen.add(key)
        kept[row['skillId'] * 10 + row['stageIndex']] = row
    return kept, skipped


def project_documents(evidence, install):
    import build_kouku_showtime_restore as library
    from build_kouku_action_effect_groups import project_light_occurrences
    evidence = evidence.resolve()
    projection = evidence / 'projection'
    index, notifies, occurrences, records = acquire(evidence, evidence / 'actions')
    kept, skipped = played_stages(evidence)
    reviewed = evidence / 'material' / 'reviewed'
    deferred = {identity: row for row in source.read(reviewed / 'native_deferred_programs.json')
                for identity in row.get('occurrences', [])}
    # A deferred program whose exact source material and renderer shape already
    # has an installed Kouku native program reuses that installed material.
    installed = {}
    for path in sorted(AUTHORED.glob('effect.kouku.*.effect.json')):
        for element in source.read(path)['elements']:
            material = element.get('material', {})
            if str(material.get('sourceProfile', {}).get('runtimeShaderProfileId', '')).startswith('effect.ue3.kouku-'):
                installed.setdefault((material['sourceMaterialPath'], element['sourceRecipe'].get('rendererShape')), (material, path.name))
    reused = {}
    for identity in list(deferred):
        occurrence = next((o for o in occurrences if o['elementId'] == identity), None)
        match = occurrence and installed.get((occurrence['sourceMaterial'], occurrence['rendererShape']))
        if match:
            reused[identity] = dict(material=match[0], sourceDocument=match[1], deferredReason=deferred.pop(identity)['reason'])
    source.write(projection / 'reused_installed_native_materials.json',
        {k: dict(v, runtimeShaderProfileId=v['material']['sourceProfile']['runtimeShaderProfileId']) for k, v in reused.items()})
    deferred.update({row['elementId']: row for row in source.read(evidence / 'material' / 'native_input_exclusions.json')})
    usable = [o for o in occurrences if o['elementId'] not in deferred]
    patch = library.patch_simulation_providers(projection, reviewed / 'native_material_patch.json', index, usable)
    natives = {key: program['material'] for program in source.read(patch)['programs'] for key in program['occurrences']}
    natives.update({key: value['material'] for key, value in reused.items()})
    distortion_profiles = {row['runtimeShaderProfileId'] for row in source.read(reviewed / 'native_runtime_contract.json')['programs']
                           if row.get('distortionPass')}
    meshes = prepare_geometry(evidence, [o for o in usable if o['actionId'] in kept])
    catalog = {v['vehicleId']: v for v in source.read(CATALOG)['vehicles']}
    documents, failures = [], []
    for identity, stage in sorted(kept.items()):
        own = [o for o in usable if o['actionId'] == identity]
        own_notifies = [n for n in notifies if n['actionId'] == identity]
        if not own_notifies:
            continue
        asset = f"effect.vehicle.{catalog[stage['vehicleId']]['archetypeId'].lower()}.{stage['skillId']}.{stage['clipIndex']}.full.restore"
        try:
            source.SELECTED = {identity: ([0], asset)}
            document, light_failures = None, []
            particles = [o for o in own if o['rendererShape'] != 'light']
            if particles:
                # Without a patch the shared projector keeps Kouku mesh paths
                # unchecked; native materials are applied below exactly as it would.
                source.project(projection, index, own_notifies, particles, records, projection / 'raw')
                document = source.read(projection / 'raw' / f'effect.kouku.gate1.{identity}.full.restore.effect.json')
                for element in document['elements']:
                    base_id = element['id'].split('.')[0] + '.' + element['id'].split('.')[1] + '.' + element['id'].split('.')[2]
                    native = natives.get(base_id)
                    assert native is not None, ('missing native material occurrence', element['id'])
                    element['material'] = copy.deepcopy(native)
                    assert element['material']['sourceMaterialPath'] == next(o['sourceMaterial'] for o in particles if o['elementId'] == base_id)
                    element['detail']['uv'].update(start=[0, 0], speed=[0, 0], wave=False, sequence=False)
                    element['detail']['color']['emissiveIntensity'] = 1
            for notify in own_notifies:
                lights = [o for o in own if o['rendererShape'] == 'light' and o['sourceNotify'] == notify['notifyId']]
                if not lights:
                    continue
                try:
                    light = project_light_occurrences(index, lights, notify['cue'], notify, projection)
                except AssertionError as error:
                    light_failures += [dict(elementId=o['elementId'], sourceEmitter=o['sourceEmitter'], reason=str(error)) for o in lights]
                    continue
                for element in light['elements']:
                    element['detail']['timing']['startDelaySeconds'] += notify['localTimeSeconds']
                    element['sourcePresentation']['sourceTimeSeconds'] = notify['localTimeSeconds']
                if document is None:
                    document = light
                else:
                    document['elements'] += light['elements']
            assert document and document['elements'], 'No projected element'
            # An unserialized EmitterLoops keeps the UE3 ParticleModuleRequired
            # default 0 (loop until the notify window ends), as the World marker
            # builder does. The shared importer fills 1, which stops emitters early.
            # User decision 2026-09-15: emitters with an original distortion pass
            # keep one emission so the notify window does not stack screen distortion.
            for element in document['elements']:
                if element['kind'] == 'light' or element['material'].get('sourceProfile', {}).get(
                        'runtimeShaderProfileId') in distortion_profiles:
                    continue
                emitter = next(o for o in own if o['sourceEmitter'] == element['sourcePresentation']['sourceObjectPath'])
                required = next(key for key in emitter['moduleOrder'] if 'particlemodulerequired' in key)
                if 'emitterloops' not in {k.lower() for k in index.objects[required].properties}:
                    element['sourceRecipe']['emitterLoopCount'] = 0
            document.update(effectAssetId=asset, displayName=f"{catalog[stage['vehicleId']]['archetypeId']} {stage['inputSlot']} {stage['clip']}")
            document.pop('sourceModelPreview', None)
            expanded = []
            for element in document['elements']:
                element['groupId'] = asset
                path = element['sourcePresentation']['sourceObjectPath']
                occurrence = next(o for o in own if o['sourceEmitter'] == path and o['sourceNotify'] == element['sourcePresentation']['sourceEventId'])
                expanded.append(dict(occurrence, elementId=element['id']))
                if occurrence['sourceMesh']:
                    element['resources'] = [dict(slotId='meshModel', assetId=meshes[occurrence['sourceMesh']])]
            library.bind_source_providers(document, index, expanded)
            for element in document['elements']:
                for resource in element['resources'] + element['material'].get('sourceProfile', {}).get('textures', []):
                    assert (RESOURCES / resource['assetId']).is_file(), resource
            path = AUTHORED / (asset + '.effect.json')
            source.write(projection / 'candidate' / path.name, document)
            duration = __import__('math').ceil(max(e['detail']['timing']['startDelaySeconds'] + e['detail']['timing']['lifeTimeSeconds']
                + (0 if e['kind'] == 'light' else max(e['detail']['particle']['lifeTimeSeconds'])) for e in document['elements']) * 1000)
            documents.append(dict(effectAssetId=asset, path=path.relative_to(ROOT).as_posix(), vehicleId=stage['vehicleId'],
                skillId=stage['skillId'], inputSlot=stage['inputSlot'], clipIndex=stage['clipIndex'], clip=stage['clip'],
                sourceStage=stage['stageIndex'], sourceNotifies=len(own_notifies), elementCount=len(document['elements']),
                deferredEmitters=sorted(o['elementId'] for o in occurrences if o['actionId'] == identity and o['elementId'] in deferred),
                lightFailures=light_failures,
                durationMs=duration))
        except Exception as error:
            failures.append(dict(effectAssetId=asset, actionId=identity, errorType=type(error).__name__, reason=str(error)))
    if install and not failures:
        install_documents([d['effectAssetId'] for d in documents], projection / 'candidate')
    source.write(projection / 'installation.json', dict(installed=install and not failures, documents=documents,
        sourceFailures=failures, skippedStages=skipped, manualVisualValidation='USER_PENDING'))
    print(json.dumps(dict(documents=len(documents), elements=sum(d['elementCount'] for d in documents),
        failures=len(failures), skippedStages=len(skipped), installed=install and not failures), ensure_ascii=False))
    for failure in failures:
        print(failure)


def install_documents(assets, candidates):
    for asset in assets:
        document = candidates / (asset + '.effect.json')
        target = AUTHORED / document.name
        if not target.is_file() or target.read_bytes() != document.read_bytes():
            target.write_bytes(document.read_bytes())
    catalog_path = ROOT / 'Data/Effects/EffectCatalog.json'
    text = catalog_path.read_text(encoding='utf-8')
    newline = '\r\n' if '\r\n' in text else '\n'
    rows = [json.dumps(dict(effectAssetId=a, payloadKind='DIRECT_AUTHORED_DOCUMENT', authoringPath=f'Effects/Authored/{a}.effect.json'), ensure_ascii=False)
            for a in assets if f'"effectAssetId": "{a}"' not in text]
    if rows:
        marker = newline + '  ]' + newline + '}'
        assert text.count(marker) == 1
        text = text.replace(marker, ',' + newline + (',' + newline).join('    ' + r for r in rows) + marker)
        json.loads(text)
        catalog_path.write_bytes(text.encode('utf-8'))
    for suffix, entry in (('', '    <None Include="{}"></None>'), ('.filters', '    <None Include="{}"><Filter>96.DataFiles</Filter></None>')):
        project = ROOT / ('Client/Default/Client.vcxproj' + suffix)
        text = project.read_text(encoding='utf-8-sig')
        newline = '\r\n' if '\r\n' in text else '\n'
        items = [entry.format(f'..\\..\\Data\\Effects\\Authored\\{a}.effect.json') for a in assets
                 if f'Data\\Effects\\Authored\\{a}.effect.json"' not in text]
        if items:
            prefix, marker, tail = text.rpartition('</Project>')
            text = prefix + '  <ItemGroup>' + newline + newline.join(items) + newline + '  </ItemGroup>' + newline + marker + tail
            raw = project.read_bytes()
            project.write_bytes((b'\xef\xbb\xbf' if raw.startswith(b'\xef\xbb\xbf') else b'') + text.encode('utf-8'))


def write_skill_cues(evidence):
    """VehicleCatalog formatVersion 3: effectCues from installed documents, soundCues from AKEvents."""
    import build_vehicle_skills
    installation = source.read(evidence / 'projection' / 'installation.json')
    assert installation['installed'], 'Install the projected documents first'
    kept, _ = played_stages(evidence)
    effects = collections.defaultdict(list)
    for document in installation['documents']:
        effects[document['skillId']].append(dict(clipIndex=document['clipIndex'], effectAssetId=document['effectAssetId'],
            startMs=0, stopPolicy='NATURAL'))
    sounds = collections.defaultdict(list)
    spec = source.read(SPEC)
    for vehicle in spec['vehicles']:
        actions = {a['actionId']: a for a in source.read(evidence / 'actions' / (vehicle['name'] + '.action-effects.json'))['actions']}
        for skill in vehicle['skills']:
            for stage in actions[skill['skillId']]['stages']:
                row = kept.get(skill['skillId'] * 10 + stage['stageIndex'])
                if row is None:
                    continue
                for notify in stage['notifies']:
                    if notify['sourceType'] != 'AKEvent':
                        continue
                    events = [r['objectPath'] for r in notify['assetReferences'] if r['className'] == 'AkEvent']
                    assert len(events) == 1, notify['notifyId']
                    sounds[skill['skillId']].append(dict(clipIndex=row['clipIndex'], event=events[0].split('.', 1)[1],
                        startMs=round(notify['localTimeSeconds'] * 1000)))
    catalog = source.read(CATALOG)
    for vehicle in catalog['vehicles']:
        for skill in vehicle['skills']:
            skill['effectCues'] = sorted(effects[skill['skillId']], key=lambda c: (c['clipIndex'], c['startMs']))
            skill['soundCues'] = sorted(sounds[skill['skillId']], key=lambda c: (c['clipIndex'], c['startMs'], c['event']))
    catalog['formatVersion'] = 3
    build_vehicle_skills.write_catalog(catalog)
    counts = dict(effectCues=sum(len(v) for v in effects.values()), soundCues=sum(len(v) for v in sounds.values()))
    source.write(evidence / 'catalog_cues.json', dict(counts, effects=effects, sounds=sounds))
    print(json.dumps(counts))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/VehicleSkillEffects20260914')
    parser.add_argument('--acquire-only', action='store_true')
    parser.add_argument('--native-first', type=int)
    parser.add_argument('--native-last', type=int)
    parser.add_argument('--install-native', action='store_true')
    parser.add_argument('--project', action='store_true')
    parser.add_argument('--install', action='store_true')
    parser.add_argument('--catalog', action='store_true')
    options = parser.parse_args()
    if options.catalog:
        write_skill_cues(options.evidence_root)
    elif options.project:
        project_documents(options.evidence_root, options.install)
    elif options.install_native:
        install_native(options.evidence_root.resolve())
    elif options.acquire_only:
        acquire(options.evidence_root, options.evidence_root / 'actions')
    else:
        assert options.native_first is not None and options.native_last is not None, 'Projection is added after the native material gate'
        native_materials(options.evidence_root.resolve(), options.native_first, options.native_last)

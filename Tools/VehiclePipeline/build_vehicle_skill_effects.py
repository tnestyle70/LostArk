"""Restore vehicle skill ParticleSystems through the source V1 Effect pipeline.

Source notifies come from the vehicle Action .loa extracts. ParticleSystem
closures are read directly from the installed game packages, reusing the Kouku
full-restore acquisition without its CanonicalSource extract.
"""
from pathlib import Path
import argparse, base64, collections, copy, json, math, re, struct, sys

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


# Skeletal meshes a PlaySkeletalMesh notify spawns, restored as V1 model cues.
MODEL_CUE_MESHES = {
    'mn_pmstg_00.mesh.mn_pmstg_00_parts1_sk': dict(contract='TerpeionWing', vehicle='Terpeion', cueId='terpeion.wing',
        package='MN_PMSTG_00', mesh='mesh.mn_pmstg_00_parts1_sk',
        model='Effect/Vehicle/Terpeion/TerpeionWing/TerpeionWing.wmodel'),
    # The Aufstehen hologram doll: cooked per material slot. The puppet is the body's own
    # prop, grafted onto socket wp_2 by build_npc, so it shares the doll skeleton and clip.
    'mn_admg_00.mesh.mn_admg_00_sk': dict(contract='AufstehenDoll', vehicle='Aufstehen', cueId='aufstehen.doll',
        package='MN_ADMG_00', mesh='mesh.mn_admg_00_sk', materialRoot='admg',
        model='Effect/Vehicle/Aufstehen/AufstehenDoll/AufstehenDoll.wmodel',
        parts=[dict(cueId='aufstehen.doll', model='Effect/Vehicle/Aufstehen/AufstehenDoll/AufstehenDoll.wmodel',
                    material='mn_admg_00.mat.sk_admg_00_mi_dead'),
               dict(cueId='aufstehen.doll.hair', model='Effect/Vehicle/Aufstehen/AufstehenDollHair/AufstehenDollHair.wmodel',
                    material='mn_admg_00.mat.sk_admg_00_hair_mi_dead'),
               dict(cueId='aufstehen.doll.lashes', model='Effect/Vehicle/Aufstehen/AufstehenDollLashes/AufstehenDollLashes.wmodel',
                    material='mn_admg_00.mat.sk_admg_00_mi_dead'),
               dict(cueId='aufstehen.puppet', model='Effect/Vehicle/Aufstehen/AufstehenPuppet/AufstehenPuppet.wmodel',
                    material='wp_mn_admg_00.mat.sk_admg_00_mi_dead')]),
}


def socket_contracts(evidence):
    catalog = {v['archetypeId']: v for v in source.read(CATALOG)['vehicles']}
    spec = source.read(SPEC)
    contracts = {}
    meshes = [(vehicle['name'], *SOURCE_MESHES[vehicle['name']],
               next(v for v in catalog.values() if v['vehicleId'] == vehicle['vehicleId'])['modelAssetId'])
              for vehicle in spec['vehicles']]
    meshes += [(row['contract'], row['package'], row['mesh'], row['model']) for row in MODEL_CUE_MESHES.values()]
    for name, logical, mesh_path, model in meshes:
        package = load_package(resolve_physical_package(UMODEL, RELEASE, logical, 'kr'), ue3.LOSTARK_KR_AES_KEY)
        mesh = source.record_from_export(package, logical.lower(), find_export(package, mesh_path))
        if mesh['className'] == 'objectredirector':
            # MN_ADMG_00 keeps an ObjectRedirector and the SkeletalMesh under one path.
            meshes_at_path = [record for record in (source.record_from_export(package, logical.lower(), export)
                              for export in package.exports) if record['fullPath'] == mesh['fullPath'] and record['className'] == 'skeletalmesh']
            assert len(meshes_at_path) == 1, (name, mesh['fullPath'])
            mesh = meshes_at_path[0]
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
                # Vehicle WModels are cooked through the Blender psk importer, which
                # mirrors bone-local Y. Aufstehen FX_*_Engine_01 (+67 cm Y on
                # bip002-spine) lands in front of the torso raw and behind it,
                # beside b_engine_00/01, once mirrored. A Y mirror negates yaw and roll.
                runtimeLocalTransform=dict(position=[position[0] * 0.01, -position[1] * 0.01, position[2] * 0.01],
                    rotationDegrees=[rotator[0] * 360.0 / 65536.0, -rotator[1] * 360.0 / 65536.0, -rotator[2] * 360.0 / 65536.0],
                    scale=factors),
                transformEvidence='EXPLICIT_SOCKET_PROPERTIES_BONE_Y_MIRRORED'))
        folded = [s['socketName'].casefold() for s in sockets]
        assert len(folded) == len(set(folded)), ('socket names are not unique', name)
        bones = wmodel_bones(model)
        contract = dict(schema='lostark.ue3-skeletal-mesh-sockets', formatVersion=1,
            source=dict(package=logical, skeletalMesh=mesh_path, sourcePackage=str(package.path),
                positionUnitScale=0.01, rotationUnitScaleDegrees=360.0 / 65536.0),
            runtimeModel=dict(assetId=model, bones=bones), sockets=sockets)
        source.write(evidence / 'sockets' / (name + '.socket-contract.json'), contract)
        contracts[name] = contract
    return contracts


def skeletal_mesh_particles(notify, skin):
    """Particle entries a PlaySkeletalMesh notify attaches to its spawned mesh.

    Entry layout after the CEFAN_Particle class FString: int32 flag, 8 bytes,
    skin FString (empty for the base skin), float start, float duration, then a
    CEFParticleData block with the same layout as PlayParticleEffect. The skin's
    own entries replace the base entries when present.
    """
    raw = base64.b64decode(notify['serializedPayload']['data'])
    marker = b'\x0f\0\0\0CEFAN_Particle\0'
    starts = []
    at = raw.find(marker)
    while at >= 0:
        starts.append(at)
        at = raw.find(marker, at + len(marker))
    entries = []
    for index, at in enumerate(starts):
        end = starts[index + 1] if index + 1 < len(starts) else len(raw)
        _, cursor = fstring(raw, at)
        flag = struct.unpack_from('<i', raw, cursor)[0]
        assert flag == 1, (notify['notifyId'], 'CEFAN_Particle flag', flag)
        entry_skin, cursor = fstring(raw, cursor + 12)
        start, duration = struct.unpack_from('<ff', raw, cursor)
        block = raw[cursor + 8:end]
        assert block.startswith(b'\x10\0\0\0CEFParticleData\0'), notify['notifyId']
        reference = re.search(rb"ParticleSystem'([^']+)'\0", block)
        assert reference, notify['notifyId']
        entries.append(dict(skin=entry_skin, startSeconds=start, durationSeconds=duration, block=block,
            system=reference.group(1).decode('ascii')))
    own = [e for e in entries if e['skin'].lower() == skin.lower()]
    return own or [e for e in entries if not e['skin']]


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
                    mesh = next((r['objectPath'].lower() for r in notify['assetReferences'] if r['className'] == 'SkeletalMesh'), None)
                    if notify['sourceType'] == 'PlaySkeletalMesh' and mesh in MODEL_CUE_MESHES:
                        cue = MODEL_CUE_MESHES[mesh]
                        header = b'CEFActionNotify_PlayParticleEffect\0'
                        header += b'\0' * (47 - len(header)) + b'\x01'
                        for number, entry in enumerate(skeletal_mesh_particles(notify, skin)):
                            data = header + entry['block']
                            row = dict(notifyId=f"{notify['notifyId']}/mesh-particle-{number:02d}", sourceType='PlayParticleEffect',
                                category='particle', authority='PRESENTATION', resolutionStatus='SOURCE_SKELETAL_MESH_PARTICLE',
                                localTimeSeconds=notify['localTimeSeconds'] + entry['startSeconds'],
                                sourceEndSeconds=0.0, durationSeconds=entry['durationSeconds'],
                                assetReferences=[dict(className='ParticleSystem', objectPath=entry['system'])],
                                serializedLabels=['FX', 'CEFParticleData', f"ParticleSystem'{entry['system']}'"],
                                serializedPayload=dict(encoding='base64', data=base64.b64encode(data).decode('ascii'),
                                    vehicleName=cue['contract'], vehicleSkin=skin, vehicleParticleSystem=entry['system'],
                                    vehicleSelection='SKELETAL_MESH_PARTICLE', modelCueId=cue['cueId'],
                                    sourceSkeletalMeshNotify=notify['notifyId'], sourceEntrySkin=entry['skin']))
                            skin_rows.append(dict(vehicle=vehicle['name'], skin=skin, notifyId=row['notifyId'],
                                selection='SKELETAL_MESH_PARTICLE:' + (entry['skin'] or 'base'), particleSystem=entry['system']))
                            notifies.append(row)
                        continue
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


def native_environment():
    """Redirect the Kouku pattern extractor to the installed game packages (this process only)."""
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
    pattern.prepare_textures = textures
    return pattern


def native_materials_prepare(pattern, root, first, last, reuse_roots=()):
    pattern.prepare(root, first, last, list(reuse_roots))


def native_materials(evidence, first, last):
    """Recover original native programs with the Kouku pattern pipeline.

    Only this process's module globals are redirected: the installed game
    packages replace CanonicalSource, and textures land under Effect/Vehicle.
    The generated tables are written to a candidate header inside evidence.
    """
    pattern = native_environment()
    root = evidence / 'material'
    root.mkdir(parents=True, exist_ok=True)
    excluded = []
    occurrences = []
    for occurrence in source.read(evidence / 'source_occurrences.json'):
        if not occurrence['sourceMaterial'] and occurrence['rendererShape'] != 'mesh':
            # No material means no native program, but the emitter still simulates:
            # a seed emitter routes events to the receivers that do draw. Projection
            # keeps it as a simulationOnly element instead of dropping it.
            excluded.append(dict(elementId=occurrence['elementId'], sourceEmitter=occurrence['sourceEmitter'],
                rendererShape=occurrence['rendererShape'], reason='SOURCE_NULL_MATERIAL_NON_MESH_EMITTER',
                simulationOnly=True))
            continue
        occurrences.append(occurrence)
    source.write(root / 'source_occurrences.json', occurrences)
    source.write(root / 'native_input_exclusions.json', excluded)
    for name in ('source_module_inputs.json', 'source_class_defaults.json'):
        target = root / name
        if not target.is_file() or target.read_bytes() != (evidence / name).read_bytes():
            target.write_bytes((evidence / name).read_bytes())

    # Programs already installed from an earlier cohort keep their IDs; only new
    # source material x vertex factory permutations take IDs from `first`.
    reviewed = root / 'reviewed'
    native_materials_prepare(pattern, root, first, last, [reviewed] if (reviewed / 'native_runtime_contract.json').is_file() else [])
    contract = source.read(root / 'native' / 'native_runtime_contract.json')
    failures = source.read(root / 'native' / 'source_material_failures.json')
    summary = dict(programs=len(contract['programs']), deferred=len(contract['deferredPrograms']), sourceFailures=len(failures),
        excludedOccurrences=len(excluded), first=first, last=last,
        shapes=dict(collections.Counter(p['rendererShape'] for p in contract['programs'])),
        blends=dict(collections.Counter(p['nativeBlend'] for p in contract['programs'])))
    source.write(root / 'native_summary.json', summary)
    print(json.dumps(summary, ensure_ascii=False))


VEHICLE_NATIVE_FIRST, VEHICLE_NATIVE_LAST = 3712, 3967
WING_MATERIALS = ['mn_pmstg_00.mat.mn_pmstg_00-2_aa_mi']
WING_FIRST = 3828


MODEL_CUE_MATERIALS = {
    'wing': (WING_MATERIALS, WING_FIRST, 'vehicle.terpeion.wing.'),
    'admg': (['mn_admg_00.mat.sk_admg_00_mi_dead', 'mn_admg_00.mat.sk_admg_00_hair_mi_dead',
              'wp_mn_admg_00.mat.sk_admg_00_mi_dead'], 3831, 'vehicle.aufstehen.admg.'),
}


def wing_native_materials(evidence, cue='wing'):
    """Recover the skinned model-cue programs of a PlaySkeletalMesh skin material.

    The shared pattern extractor selects only particle and static-mesh vertex
    factories. Its 'mesh' branch is fed with the material map's GPU-skin factory
    renamed to the local factory, then the selection is restored to the
    original fgpuskinvertexfactory / skeletalMesh identity before generation.
    """
    materials, first, prefix = MODEL_CUE_MATERIALS[cue]
    pattern = native_environment()
    root = evidence / cue / 'material'
    root.mkdir(parents=True, exist_ok=True)
    required = prefix + 'particlemodulerequired'
    source.write(root / 'source_module_inputs.json', dict(records={required: dict(fullPath=required,
        classPath='engine.particlemodulerequired', className='particlemodulerequired', archetypeFullPath=None, properties={}, references=[])}))
    source.write(root / 'source_class_defaults.json', source.read(evidence / 'source_class_defaults.json'))
    names = [path.rsplit('.', 1)[-1] for path in materials]
    source.write(root / 'source_occurrences.json', [dict(
        elementId=prefix + (path.rsplit('.', 1)[-1] if names.count(path.rsplit('.', 1)[-1]) == 1 else path.replace('.mat.', '.')),
        sourceMaterial=path, rendererShape='mesh', sourceMesh='', moduleOrder=[required], sourceEmitter=required, actionId=0)
        for path in materials])
    parse = pattern.sm.parse_material_map

    def skinned_as_local(*args, **kwargs):
        result = parse(*args, **kwargs)
        result['vertexFactories'] = [f for f in result['vertexFactories'] if f['vertexFactoryType'] != 'flocalvertexfactory']
        for factory in result['vertexFactories']:
            if factory['vertexFactoryType'] == 'fgpuskinvertexfactory':
                factory['vertexFactoryType'] = 'flocalvertexfactory'
        return result
    pattern.sm.parse_material_map = skinned_as_local
    # The wing MIC is a static permutation: its texture expressions index the
    # MIC's own cooked reference array (normal, diffuse, flat_black, cloud, ...),
    # not the parent's three-entry default array the shared resolver reads.
    obj = pattern.obj
    parents = {}
    for path in materials:
        row = pattern.material(path)
        mic = obj(path)
        count = struct.unpack_from('<I', mic['tail'], 36)[0]
        assert count <= 256 and len(mic['tail']) >= 40 + 4 * count
        parents[row['parentMaterial']] = (mic, count, row['baseId'])

    def permutation_parent(path):
        original = obj(path)
        if path not in parents:
            return original
        mic, count, base_id = parents[path]
        tail = original['tail']
        rebuilt = struct.pack('<I', 1) + tail[4:16] + bytes.fromhex(base_id) + tail[32:36] + mic['tail'][36:40 + 4 * count]
        return dict(original, tail=rebuilt, package=mic['package'], sourceStaticPermutationTextures=mic['path'])
    pattern.obj = permutation_parent
    try:
        native_materials_prepare(pattern, root, first, VEHICLE_NATIVE_LAST)
    except AssertionError as error:
        # prepare() lowers the renamed 'mesh' selection once; the skinned
        # selection is regenerated below.
        print('Expected local-factory generation failure:', str(error)[:200])
    pattern.obj = obj
    pattern.sm.parse_material_map = parse
    out = root / 'native'
    selection = source.read(out / 'selected_runtime_material_programs.json')
    for program in selection['programs']:
        assert program['sourceVF'] == 'flocalvertexfactory'
        program.update(sourceVF='fgpuskinvertexfactory', rendererShape='skeletalMesh')
    source.write(out / 'selected_runtime_material_programs.json', selection)
    pattern.generate_native(root, first, VEHICLE_NATIVE_LAST)
    contract = source.read(out / 'native_runtime_contract.json')
    print(json.dumps(dict(programs=[(p['program'], p['sourceMaterial'], p['rendererShape'], p['nativeBlend']) for p in contract['programs']],
        deferred=contract['deferredPrograms']), ensure_ascii=False))


WING_BASE_PS = '5f33bef7c823444d8983ab12adf5b7bb'
WING_LIGHT_TYPE = 'tlightpixelshaderfdirectionallightpolicyfnostaticshadowingpolicy'
WING_SHADER = ROOT / 'Client/Bin/ShaderFiles/Shader_EffectVehicleModelNative.hlsli'


def function_body(text, name):
    at = text.index(f'float4 {name}(')
    head = text.rfind('\n// ', 0, at) + 1
    return text[head:text.index('\n}\n', at) + 3]


def light_program_function(dump, program, base_text):
    """Translate a model-cue MIC's directional light PS into ArtistNative<program>Light.

    The dump comes from build_vehicle_source_material.py extract (same material
    map as the base program). Both PS read one uniform expression set, so each
    light row reuses the base program's packed parameter expression for the same
    expression index. Two source light VS layouts occur: the ghost skin reads
    v2 UV, v3 tangent light, v5 tangent view with CB0[0].x opacity; the monster
    skins read v0/v1 tangent basis, v4 UV, v5 tangent light, v7 tangent view,
    v8 source world position with the [0,1] world prefix. The trailing engine
    rows are light colour then its unit scale, and an unbound texture is the
    light attenuation (no shadow).
    """
    base_program = dump['programs']['tbasepasspixelshaderfnolightmappolicyskylight']
    light = dump['programs'][WING_LIGHT_TYPE]
    generator = (ROOT / 'Tools/EffectPipeline/generate_artist_native_runtime_shader.py').read_text(encoding='utf-8')
    helpers = {'re': re}
    exec(generator[generator.index('def args(s):'):generator.index('ns = {name:')], helpers)
    base_body = base_text[base_text.index(f'float4 ArtistNative{program}(ARTIST_NATIVE_INPUT input)'):]
    base_body = base_body[:base_body.index('\n}\n')]
    assignments = {}
    for row, lane, value in re.findall(r'^    source\[(\d+)\](?:\.([xyzw]))? = (.*);$', base_body, re.M):
        assert not lane or (value.startswith('(') and value.endswith(').x')), value
        assignments[(int(row), lane)] = value[1:-3] if lane else value
    expression = {}
    for binding in base_program['bindings']['vectors']:
        expression[('vector', binding['expressionIndexOrGroup'])] = assignments[(binding['baseIndex'] // 16, '')]
    for binding in base_program['bindings']['scalarGroups']:
        for index, lane in enumerate('xyzw'):
            key = (binding['baseIndex'] // 16, lane)
            if key in assignments:
                expression[('scalar', binding['expressionIndexOrGroup'], index)] = assignments[key]
    # Light-only uniform expressions (shadowfactor, orennayar, pbr specular...) are
    # static MIC values: fold them with the dump's effective parameters.
    def fold(node):
        kind = node['typeName']
        if kind == 'fmaterialuniformexpressionconstant':
            return [float(v) for v in node['value']] + [0.0] * (4 - len(node['value']))
        if kind == 'fmaterialuniformexpressionscalarparameter':
            return [float(dump['scalars'].get(node['parameterName'], node['defaultValue']))] * 4
        if kind == 'fmaterialuniformexpressionvectorparameter':
            return [float(v) for v in dump['vectors'].get(node['parameterName'], node['defaultValue'])]
        if kind == 'fmaterialuniformexpressionfoldedmath':
            a, b = fold(node['a']), fold(node['b'])
            return [(x + y, x - y, x * y)[node['operationOrdinal']] for x, y in zip(a, b)]
        if kind == 'fmaterialuniformexpressionsine':
            return [(math.cos if node['isCosine'] else math.sin)(x) for x in fold(node['input'])]
        if kind == 'fmaterialuniformexpressionappendvector':
            a, b = fold(node['a']), fold(node['b'])
            return (a[:node['componentsFromA']] + b)[:4]
        if kind == 'fmaterialuniformexpressionclamp':
            return [min(max(x, lo), hi) for x, lo, hi in zip(fold(node['input']), fold(node['minimum']), fold(node['maximum']))]
        raise ValueError(('unsupported light-only uniform expression', kind))

    def literal(values):
        return 'float4(' + ', '.join(format(v, '.9g') + ('' if '.' in format(v, '.9g') or 'e' in format(v, '.9g') else '.0') for v in values) + ')'
    uniforms = dump['uniformExpressionSet']
    for binding in light['bindings']['vectors']:
        key = ('vector', binding['expressionIndexOrGroup'])
        if key not in expression:
            expression[key] = literal(fold(uniforms['pixelVectorExpressions'][key[1]]))
    for binding in light['bindings']['scalarGroups']:
        for index in range(4):
            key = ('scalar', binding['expressionIndexOrGroup'], index)
            position = key[1] * 4 + index
            if key not in expression and position < len(uniforms['pixelScalarExpressions']):
                expression[key] = literal(fold(uniforms['pixelScalarExpressions'][position]))
    declarations = light['disassembly']['declarations']
    rows = int(next(re.search(r'CB0\[(\d+)\]', d)[1] for d in declarations if d.startswith('dcl_constantbuffer CB0[')))
    inputs = sorted(int(re.search(r'\bv(\d+)\.', d)[1]) for d in declarations if d.startswith('dcl_input_ps '))
    unowned = light['bindings']['constantBufferClosure']['unownedConstantBuffer0Slots']
    lines = [f'// {dump["sourceMaterial"]}: directional light PS {light["shaderId"]}; parameter rows reuse program {program} packing.',
             f'float4 ArtistNative{program}Light(ARTIST_NATIVE_INPUT input, float3 tangentLight, float3 lightColor)', '{',
             f'    float4 source[{rows}]; [unroll] for (uint i=0u; i<{rows}u; ++i) source[i]=0.f;']
    if inputs == [2, 3, 5]:
        assert unowned == [0, rows - 1], unowned
        lines += ['    source[0]=float4(input.color.a,0.f,0.f,0.f);', f'    source[{rows - 1}]=float4(lightColor,1.f);']
        registers = ['    float4 v2 = float4(input.uv,0.f,0.f); // native texcoord0',
                     '    float4 v3 = float4(tangentLight,0.f); // native tangent light vector',
                     '    float4 v5 = float4(input.tangentView,0.f); // native tangent camera vector']
    else:
        assert inputs == [0, 1, 4, 5, 7, 8] and unowned == [0, 1, rows - 2, rows - 1], (inputs, unowned)
        lines += ['    source[0]=0.f;', '    source[1]=float4(input.sourceCameraPosition,input.color.a);',
                  f'    source[{rows - 2}]=float4(lightColor,1.f);', f'    source[{rows - 1}].x=1.f;',
                  '    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];']
        registers = ['    float4 v0 = float4(input.sourceBasisX,0.f); // native tangent basis row 0',
                     '    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native tangent basis row 1',
                     '    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0',
                     '    float4 v5 = float4(tangentLight,1.f); // native tangent light vector',
                     '    float4 v7 = float4(input.tangentView,1.f); // native tangent camera vector',
                     '    float4 v8 = float4(input.sourceWorldPosition,1.f); // native source world position']
    for binding in light['bindings']['vectors']:
        lines.append(f'    source[{binding["baseIndex"] // 16}] = {expression[("vector", binding["expressionIndexOrGroup"])]};')
    for binding in light['bindings']['scalarGroups']:
        for index, lane in enumerate('xyzw'):
            key = ('scalar', binding['expressionIndexOrGroup'], index)
            if key in expression:
                lines.append(f'    source[{binding["baseIndex"] // 16}].{lane} = ({expression[key]}).x;')
    temps = next(int(d.split()[1]) for d in declarations if d.startswith('dcl_temps'))
    passes = max(5, int(next(re.search(r'CB2\[(\d+)\]', d)[1] for d in declarations if d.startswith('dcl_constantbuffer CB2['))))
    lines += [f'    float4 passValues[{passes}]; [unroll] for(uint passIndex=0u;passIndex<{passes}u;++passIndex) passValues[passIndex]=0.f;',
              '    passValues[0]=float4(.5f,-.5f,.5f,.5f);',
              f'    passValues[3]=float4(0.f,0.f,0.f,1.f); // Neutral source diffuse override, as program {program}.',
              f'    passValues[4]=float4(0.f,0.f,0.f,1.f); // Neutral specular override, as program {program}.']
    lines += registers
    lines += ['    float4 ' + ', '.join(f'r{i}=0.f' for i in range(temps)) + ';', '    float4 output=0.f;']
    texture_map = {binding['baseIndex']: binding['expressionIndexOrGroup'] for binding in light['bindings']['textures']}
    for number, instruction in enumerate(light['disassembly']['instructions'], 1):
        if re.search(r'\bo[1-9]\.', instruction):
            continue
        if instruction.startswith('sample'):
            op, _, tail = instruction.partition(' ')
            a = helpers['args'](tail)
            register, swizzle = re.match(r't(\d+)\.([xyzw]+)', a[2]).groups()
            if int(register) not in texture_map:
                sample = 'float4(1.f,1.f,1.f,1.f)'
            else:
                bias = helpers['operand'](a[4]) if len(a) > 4 else 'float4(0.f,0.f,0.f,0.f)'
                sample = (f'ArtistNativeSample{texture_map[int(register)]}((' + helpers['operand'](a[1]) + ').xy, ('
                    + bias + ').x, ' + ('true' if 'sample_l' in op else 'false') + ')')
            translated = helpers['result_mask'](a[0], sample + '.' + swizzle)
        elif instruction.startswith('discard_nz'):
            translated = 'if ((' + helpers['uint_operand'](instruction.split(' ', 1)[1]) + ').x != 0u) clip(-1.f);'
        else:
            translated = helpers['translated'](instruction, {}, 'base')
        translated = re.sub(r'\bo0\b', 'output', translated)
        lines += [f'    // {number}: {instruction}', '    ' + translated]
    lines += ['}', '']
    return '\n'.join(lines)


def wing_light_program(evidence):
    dump = source.read(evidence / 'wing' / 'source_light' / (WING_MATERIALS[0] + '.json'))
    assert dump['programs']['tbasepasspixelshaderfnolightmappolicyskylight']['shaderId'] == WING_BASE_PS
    text = WING_SHADER.read_text(encoding='utf-8')
    function = light_program_function(dump, 3828, text)
    if 'float4 ArtistNative3828Light(' in text:
        updated = text.replace(function_body(text, 'ArtistNative3828Light'), function)
    else:
        updated = text + function
    if updated != text:
        WING_SHADER.write_text(updated, encoding='utf-8', newline='\n')
    print('ArtistNative3828Light changed', updated != text)


def install_model_cue_programs(evidence, cue):
    """Install a PlaySkeletalMesh cue's native skin programs with their light PS.

    Tables go through install_kouku_gate1_native_materials; the base and light
    functions live in Shader_EffectVehicleModelNative.hlsli and dispatch from
    Shade_ArtistModelNative and Shade_VehicleModelNativeLight.
    """
    import install_kouku_gate1_native_materials as materials
    folder = evidence / cue / 'material' / 'native'
    contract = source.read(folder / 'native_runtime_contract.json')
    programs = {row['program']: row['sourceMaterial'] for row in contract['programs']}
    assert not contract['deferredPrograms'] and all(row['modelCue'] for row in contract['programs'])
    materials.install(folder / 'native_runtime_contract.json', folder, ROOT / 'Client/Public/Effect_ArtistMaterial.h')
    generated = (folder / 'Shader_EffectArtistNative.hlsli').read_text(encoding='utf-8')
    original = WING_SHADER.read_text(encoding='utf-8')
    text = original
    marker = '// Vehicle model-cue light dispatch.'
    if marker in text:
        text = text[:text.index(marker)]
    for program, material in sorted(programs.items()):
        base = function_body(generated, f'ArtistNative{program}')
        dump = source.read(evidence / cue / 'source_light' / (material + '.json'))
        light = light_program_function(dump, program, base)
        for name in (f'ArtistNative{program}', f'ArtistNative{program}Light'):
            if f'float4 {name}(' in text:
                text = text.replace(function_body(text, name), '')
        text = text.rstrip('\n') + '\n' + base + light
    light_programs = sorted(int(n) for n in re.findall(r'float4 ArtistNative(\d+)Light\(', text))
    text = text.rstrip('\n') + '\n' + marker + '\n'
    text += 'float4 Shade_VehicleModelNativeLight(uint profile, ARTIST_NATIVE_INPUT input, float3 tangentLight, float3 lightColor)\n{\n    switch (profile)\n    {\n'
    text += ''.join(f'    case {n}u: return ArtistNative{n}Light(input, tangentLight, lightColor);\n' for n in light_programs)
    text += '    default: return 0.f;\n    }\n}\n'
    if text != original:
        WING_SHADER.write_text(text, encoding='utf-8', newline='\n')
    main = ROOT / 'Client/Bin/ShaderFiles/Shader_EffectArtistNative.hlsli'
    from native_shader_dispatch import expand_dispatch_includes, write_partitioned_dispatch
    main_text = expand_dispatch_includes(main.read_text(encoding='utf8'), main.parent)
    anchor = '    case 3828u: return ArtistNative3828(input);\n'
    assert main_text.count(anchor) == 1
    for program in sorted(programs):
        case = f'    case {program}u: return ArtistNative{program}(input);\n'
        if case not in main_text:
            main_text = main_text.replace(anchor, anchor + case, 1)
        anchor = case
    write_partitioned_dispatch(main, main_text)
    print('Installed model cue programs', sorted(programs), 'light dispatch', light_programs)


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
    everything = merged['programs']
    assert len({row['program'] for row in everything}) == len(everything)
    assert {row['program'] for row in everything} <= set(range(VEHICLE_NATIVE_FIRST, VEHICLE_NATIVE_LAST + 1))
    # Programs reused from the installed cohort keep their installed tables and
    # shader bodies; only fresh permutations are written to shared sources. The
    # projection patch still covers every occurrence, staged on a candidate header.
    reused_path = folder / 'reused_native_programs.json'
    reused = {row['program'] for row in source.read(reused_path)['programs']} if reused_path.is_file() else set()
    rows = [row for row in everything if row['program'] not in reused]
    owned = {row['program'] for row in rows}
    source.write(reviewed / 'native_runtime_contract.json', dict(programs=everything, deferredPrograms=[]))
    source.write(reviewed / 'native_deferred_programs.json', merged['deferredPrograms'])
    header = ROOT / 'Client/Public/Effect_ArtistMaterial.h'
    from native_material_tables import read_material_bytes
    candidate = folder / 'Effect_ArtistMaterial.projection.candidate.h'
    candidate.write_bytes(read_material_bytes(header))
    materials.install(reviewed / 'native_runtime_contract.json', reviewed, candidate)
    if not rows:
        print('No fresh native programs; installed sources unchanged')
        return
    fresh = evidence / 'material' / 'fresh'
    source.write(fresh / 'native_runtime_contract.json', dict(programs=rows, deferredPrograms=[]))
    materials.install(fresh / 'native_runtime_contract.json', fresh, header)

    generated = (folder / 'Shader_EffectArtistNative.hlsli').read_text(encoding='utf8')
    bodies = {int(i): block for block, i in re.findall(
        r'(#ifndef ARTIST_NATIVE_MODEL_ONLY\n// [^\n]+\nfloat4 ArtistNative(\d+)\(ARTIST_NATIVE_INPUT input\)\n\{.*?\n\}\n#endif)', generated, re.S)}
    companion = (folder / 'KoukuGenericDistortionPrograms.hlsli').read_text(encoding='utf8')
    distortions = {int(i): block for block, i in re.findall(
        r'(float4 ArtistNative(\d+)Distortion\(ARTIST_NATIVE_INPUT input\)\n\{.*?\n\})', companion, re.S)}
    assert set(bodies) >= owned and {r['program'] for r in rows if r.get('distortionPass')} <= set(distortions)

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


WING_BONE_BASIS_INVERSE = 100.0


def material_scalar_event(notify):
    """AnimEvent_MaterialParamterScalar: int32 flag, float start, float duration, name FString, float from, float to.

    The event is nested in the notify listed before it (PlaySkeletalMesh or
    PlaySkeletalMeshMaterialParam); its start is relative to that notify.
    """
    raw = base64.b64decode(notify['serializedPayload']['data'])
    assert raw.startswith(b'CEFActionNotify_AnimEvent_MaterialParamterScalar\0'), notify['notifyId']
    flag, start, duration = struct.unpack_from('<iff', raw, 49)
    name, cursor = fstring(raw, 61)
    begin, end = struct.unpack_from('<ff', raw, cursor)
    assert flag == 1 and duration > 0, notify['notifyId']
    return dict(name=name, start=start, duration=duration, begin=begin, end=end)


def model_cue_material_tracks(notifies, mesh_index):
    """Scalar keys at cue-local time from the material events of one PlaySkeletalMesh notify."""
    origin = notifies[mesh_index]['localTimeSeconds']
    segments = collections.defaultdict(list)
    for index, notify in enumerate(notifies):
        if notify['sourceType'] != 'AnimEvent_MaterialParamterScalar':
            continue
        parent = notifies[index - 1]
        if parent['sourceType'] not in ('PlaySkeletalMesh', 'PlaySkeletalMeshMaterialParam'):
            continue
        if parent['sourceType'] == 'PlaySkeletalMesh' and index - 1 != mesh_index:
            continue
        event = material_scalar_event(notify)
        start = parent['localTimeSeconds'] - origin + event['start']
        segments[event['name']].append((start, start + event['duration'], event['begin'], event['end']))
    tracks = []
    for name, rows in sorted(segments.items()):
        keys = []
        for start, end, begin, finish in sorted(rows):
            assert not keys or start >= keys[-1][0], (name, rows)
            keys += [(start, begin), (end, finish)]
        tracks.append(dict(name=name, kind='SCALAR', keys=[dict(timeSeconds=t, value=[v], arriveTangent=[0.0],
            leaveTangent=[0.0], interpolation='linear') for t, v in keys]))
    return tracks


def model_cues(evidence, stage, identity):
    """V1 model cues for the PlaySkeletalMesh notifies of one played source stage."""
    actions = source.read(evidence / 'actions' / (stage['vehicle'] + '.action-effects.json'))['actions']
    action = next(a for a in actions if a['actionId'] == stage['skillId'])
    notifies = next(s for s in action['stages'] if s['stageIndex'] == stage['stageIndex'])['notifies']
    cues = []
    for mesh_index, notify in enumerate(notifies):
        mesh = next((r['objectPath'].lower() for r in notify['assetReferences'] if r['className'] == 'SkeletalMesh'), None)
        if notify['sourceType'] != 'PlaySkeletalMesh' or mesh not in MODEL_CUE_MESHES:
            continue
        row = MODEL_CUE_MESHES[mesh]
        animation = [label for label in notify['serializedLabels'] if re.match(r'S[CK]_', label)]
        assert len(animation) == 1, (notify['notifyId'], animation)
        clip = 'npc_' + animation[0].lower()
        root = row.get('materialRoot', 'wing')
        patch = {program['sourceMaterial']: program['material']
                 for program in source.read(evidence / root / 'material' / 'native' / 'native_material_patch.json')['programs']}
        tracks = model_cue_material_tracks(notifies, mesh_index)
        duration = notify['durationSeconds']
        loop = duration <= 0
        hold = False
        if loop:
            # Open-ended mesh: it lives until its last material event (the dead fade) ends.
            # The notify carries no window, so the clip repeats for that lifetime instead of
            # ending with its own length. The payload int after the play rate is the source
            # loop count (Q 2, W 3, E 1), and clip x count covers the fade window in each case.
            duration = max(key['timeSeconds'] for track in tracks for key in track['keys'])
        else:
            # A windowed mesh still has to outlive its own dead fade, which the source
            # places after the animation ends. Cutting there drops the fade-out. The cue
            # may not outrun its clip, so the extra tail holds the clip's last pose.
            extended = max([duration] + [key['timeSeconds'] for track in tracks for key in track['keys']])
            hold = extended > duration + 1e-6
            duration = extended
        raw = base64.b64decode(notify['serializedPayload']['data'])
        tail = raw.index(animation[0].encode('ascii') + b'\0') + len(animation[0]) + 1
        local = dict(position=[0, 0, 0], rotationDegrees=[0, 0, 0], revolutionDegreesPerSecond=[0, 0, 0],
            scale=[1, 1, 1], velocityPerSecond=[0, 0, 0])
        if len(raw) >= tail + 64 and struct.unpack_from('<2i', raw, tail + 20) == (1, 1):
            # Relative spawn transform: UE cm location, rotator units, scale. The
            # vehicle cooked skin frame is the snapshot particle basis RotY(-90)
            # applied to UE3_CentimetersToClient (x, z, -y).
            location = struct.unpack_from('<3f', raw, tail + 28)
            pitch, yaw, roll = struct.unpack_from('<3i', raw, tail + 40)
            scale = struct.unpack_from('<3f', raw, tail + 52)
            client = (location[0] * 0.01, location[2] * 0.01, -location[1] * 0.01)
            local.update(position=[-client[2], client[1], client[0]],
                rotationDegrees=[pitch * 360.0 / 65536.0, yaw * 360.0 / 65536.0, roll * 360.0 / 65536.0], scale=list(scale))
        sockets = None
        for part in row.get('parts', [dict(cueId=row['cueId'], model=row['model'], material=next(iter(patch)))]):
            cue = dict(cueId=part['cueId'], modelAssetId=part['model'], clipName=clip,
                startDelaySeconds=notify['localTimeSeconds'], durationSeconds=duration,
                opacity=1, colorMultiply=[1, 1, 1, 1], holdLastFrame=hold, loop=loop,
                alphaMode='TRANSLUCENT', visible=True,
                localTransform=copy.deepcopy(local),
                assetPreTransform=dict(scale=[0.0001] * 3, rotationDegrees=[0, -90, 0]),
                # The cue rides the vehicle's presentation root, and the Server can
                # refuse the movement a skill asks for (a wall, a non-walkable cell)
                # while the action still plays. A clip that carries its own forward
                # root translation would then leave the vehicle behind, so the root
                # bone keeps its rest X/Z and only animates vertically.
                suppressHorizontalRootMotionBone='b_root',
                material=copy.deepcopy(patch[part['material']]))
            if tracks:
                cue['materialParameterTracks'] = tracks
            if part.get('parent'):
                sockets = sockets or {s['socketName'].casefold(): s for s in
                    source.read(evidence / 'sockets' / (row['contract'] + '.socket-contract.json'))['sockets']}
                socket = sockets[part['socket'].casefold()]['runtimeLocalTransform']
                assert socket['rotationDegrees'] in ([0, 0, 0], [0.0, -0.0, -0.0], [0.0, 0.0, 0.0]), socket
                # Sample_ModelCuePose composes localTransform x root only: assetPreTransform
                # is baked into the child's own geometry at load, so the socket carries the
                # source rotation. The psk importer mirrors the bone local Y, which leaves the
                # child facing backwards along the socket, so the source rotation is taken with
                # a yaw turn. The model-cue bone basis 0.01 is undone like wing-attached particles.
                cue.update(localTransform=dict(position=[0, 0, 0], rotationDegrees=[0, 0, 0], revolutionDegreesPerSecond=[0, 0, 0],
                    scale=[1, 1, 1], velocityPerSecond=[0, 0, 0]),
                    parentAttachment=dict(cueId=part['parent'], boneName=sockets[part['socket'].casefold()]['boneName'],
                        socketLocalTransform=dict(position=[v * WING_BONE_BASIS_INVERSE for v in socket['position']],
                            rotationDegrees=[socket['rotationDegrees'][0],
                                socket['rotationDegrees'][1] + 180.0, socket['rotationDegrees'][2]],
                            scale=[WING_BONE_BASIS_INVERSE] * 3)))
            cues.append(cue)
    assert len({c['cueId'] for c in cues}) == len(cues), identity
    return cues


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
    exclusions = source.read(evidence / 'material' / 'native_input_exclusions.json')
    simulation_only = {row['elementId'] for row in exclusions if row.get('simulationOnly')}
    deferred.update({row['elementId']: row for row in exclusions if not row.get('simulationOnly')})
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
                    if base_id in simulation_only:
                        element['material'] = dict(templateId='effect.standard',
                            sourceMaterialPath='enginematerials.defaultparticle',
                            renderProfile='alpha_two_sided_depth_read', sourceProfile=dict(enabled=False))
                        element['sourceRecipe']['simulationOnly'] = True
                        continue
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
            # The runtime bounds emission by EmitterDuration x EmitterLoops only.
            # A notify window shorter than that deactivates the source system at
            # its end (Aufstehen boosters: 10 s x 1 in a 0.73 s window); loop 0
            # lets the element lifetime end emission without changing EmitterTime.
            for element in document['elements']:
                recipe, timing = element['sourceRecipe'], element['detail']['timing']
                if element['kind'] == 'light' or recipe['emitterLoopCount'] == 0:
                    continue
                emitter = next(o for o in own if o['sourceEmitter'] == element['sourcePresentation']['sourceObjectPath'])
                if emitter['sourceDurationSeconds'] > 0 and (recipe['emitterDelaySeconds'] +
                        recipe['emitterDurationSeconds'] * recipe['emitterLoopCount']) > timing['lifeTimeSeconds']:
                    recipe['emitterLoopCount'] = 0
            # Source MaxParticleInTrailCount is an upper bound, not the ribbon's
            # fill. A point older than the trail lifetime is dropped, so capacity
            # is what one lifetime of fixed-step samples can hold. Terpeion Space
            # keeps six 500-point ribbons that never exceed 39, and the document
            # trail budget rejects their sum. build_kouku_backstep_electric_group
            # clamps the same way.
            for element in document['elements']:
                if element['kind'] != 'trail':
                    continue
                trail = element['detail']['trail']
                native = next((m for m in element['sourceRecipe']['modules']
                               if m['className'] == 'particlemoduletypedataribbon'), None)
                source_points = int(next((v['value'] for v in native['literals']
                    if v['propertyPath'] == 'maxparticleintrailcount'), 0)) if native else 0
                required = math.ceil(trail['pointLifeTimeSeconds'] / trail['sampleIntervalSeconds']) + 2
                trail['maxPoints'] = min(source_points, required) if source_points > 0 else required
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
            document['modelCues'] = model_cues(evidence, stage, identity)
            for element in document['elements']:
                attachment = element['actionCueAttachment']
                notify = next(n for n in own_notifies if n['notifyId'] == element['sourcePresentation']['sourceEventId'])
                cue_id = notify['serializedPayload'].get('modelCueId')
                if not cue_id:
                    continue
                assert any(c['cueId'] == cue_id for c in document['modelCues']), (asset, cue_id)
                # Model-cue anchors are socket x bone x cue world without the
                # vehicle bone normalization, so the 0.01 wing bone basis is undone here.
                socket = attachment['socketLocalTransform']
                attachment.update(modelCueId=cue_id, runtimeAnchorSlotId=cue_id + '/' + attachment['runtimeAnchorSlotId'],
                    socketLocalTransform=dict(socket, position=[v * WING_BONE_BASIS_INVERSE for v in socket['position']],
                        scale=[v * WING_BONE_BASIS_INVERSE for v in socket['scale']]))
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
    parser.add_argument('--wing-native', action='store_true')
    parser.add_argument('--model-cue', default='wing', choices=sorted(MODEL_CUE_MATERIALS))
    parser.add_argument('--wing-light', action='store_true')
    parser.add_argument('--install-model-cue', action='store_true')
    options = parser.parse_args()
    if options.install_model_cue:
        install_model_cue_programs(options.evidence_root.resolve(), options.model_cue)
    elif options.wing_light:
        wing_light_program(options.evidence_root.resolve())
    elif options.wing_native:
        wing_native_materials(options.evidence_root.resolve(), options.model_cue)
    elif options.catalog:
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

"""Build Warlord source restore documents from recovered native evidence.

Original authored documents and imported source are read-only. This command owns
the .full.restore files and the evidence-root projection report. The default
selection retains the original A/S/F/V generation; --all-skills expands the
other currently bound slots and --compose-shields-only applies the user layout.
"""
from pathlib import Path
import argparse, base64, collections, copy, hashlib, json, math, struct, sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/LevelPlacementExtractor'))
import build_imported_effect_documents as imported
from build_action_cue_recipe import decode_typed_payload
from build_effect_source_material_contract import stable_profile_id

def read(path): return json.loads(path.read_text(encoding='utf-8-sig'))
def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
def norm(value):
    if isinstance(value, dict): return {(k if k == 'structType' else k.lower()): norm(v) for k, v in value.items()}
    if isinstance(value, list): return [norm(v) for v in value]
    return value.lower() if isinstance(value, str) else value

def merge(base, overlay):
    # RawDistribution is a serialized value. An explicit null Distribution must
    # clear its CDO lookup cache instead of inheriting a synthetic constant table.
    if isinstance(overlay, dict) and str(overlay.get('structType', '')).lower() in ('rawdistributionfloat', 'rawdistributionvector'):
        return copy.deepcopy(overlay)
    result = copy.deepcopy(base)
    for key, value in overlay.items():
        result[key] = merge(result[key], value) if isinstance(value, dict) and isinstance(result.get(key), dict) else copy.deepcopy(value)
    return result

def build(evidence, action_path, destination, all_skills=False):
    occurrences = read(evidence / 'full_occurrence_material_bindings.json')
    native = read(evidence / 'native_module_inputs.json')['records']
    vectorfields = {row['sourceObjectPath']: row['assetId'] for row in read(evidence / 'vector_field_receipt.json')} if (evidence / 'vector_field_receipt.json').exists() else {}
    defaults = read(evidence / 'source_class_defaults.json')['records']
    records = {row['fullPath']: row for row in defaults}
    records.update({key: row for key, row in native.items() if 'error' not in row})
    effective, chains = {}, {}
    def resolve(key, stack=()):
        if key in effective: return effective[key]
        assert key not in stack, ('source archetype cycle', key)
        row = records[key]; parent = row.get('archetypeFullPath'); cls = row.get('classPath', '')
        if not parent and not key.startswith(('engine.', 'efgame.', 'core.')):
            parent = cls.split('.')[0] + '.default__' + cls.split('.')[-1]
        assert not parent or parent in records, ('missing source archetype', key, parent)
        effective[key] = merge(resolve(parent, stack + (key,)) if parent else {}, norm(row.get('properties', {})))
        chains[key] = [parent] + chains.get(parent, []) if parent else []
        return effective[key]
    index = imported.SourceIndex({}, {})
    for key, row in records.items():
        cls = row.get('className') or row.get('classPath', '').split('.')[-1]
        obj = imported.SourceObject(key, key, cls, key, resolve(key))
        for ref in row.get('references', []):
            path = ref['objectPath'].lower(); prop = ref['property'].lower()
            obj.reference_paths.append((prop, path))
            if path in records: obj.references.append((prop, path))
        index.objects[key] = obj; index.by_source_id[key] = obj
    lods = {key.split('.particlelodlevel_')[0]: row for key, row in records.items() if '.particlelodlevel_' in key}
    light_components = {row['fullPath'].rsplit('.', 1)[0]: row for row in read(evidence / 'native_light_components.json')}
    socket_contract = read(evidence / 'source_socket_contract.json')
    action = read(action_path)
    notifies = {n['notifyId']: n for a in action['actions'] for s in a['stages'] for n in s['notifies']}
    programs = read(evidence / 'native_runtime_contract.json')['programs']
    by_occurrence = {origin: row for row in programs for origin in row['occurrences']}
    geometry = {row['legacyModel'].replace('\\', '/').split('/Resources/', 1)[1]: row['runtimeAsset'] for row in read(evidence / 'geometry_installation.json')}
    archive = Path('C:/Users/user/.codex/worktrees/dm-ba-effect-pr/LostArk/Data/Effects/Imported/Warlord')
    selected = sorted({(o['skillId'], o['slot']) for o in occurrences}) if all_skills else [(17040, 'S'), (17090, 'A'), (17140, 'F'), (17170, 'V')]
    # F is the user's accepted reference. Expansion never rewrites A/S/F.
    if all_skills:
        selected = [(sid, slot) for sid, slot in selected if sid not in (17040, 17090, 17140, 17170)]
        occurrences = [o for o in occurrences if o['skillId'] in dict(selected)]
        occurrences = [o for o in occurrences if o['skillId'] != 17100 or '/stage-000/' in o['sourceEvent']['raw']['notify']]
        occurrences = [o for o in occurrences if o['skillId'] != 17820 or o['sourceEvent']['clipSequenceIndex'] in (0, 1, 4)]
    source_docs = {sid: read((archive if sid == 17000 else archive / 'CurrentCombat') / 'Converted' / f'effect.warlord.skill.{sid}.imported.effect.json') for sid, _ in selected}
    docs = {}
    for sid, slot in selected:
        doc = copy.deepcopy(source_docs[sid]); doc.update(version=13, effectAssetId=f'effect.warlord.skill.{sid}.full.restore', displayName=f'이펙트_워로드{slot}_전체')
        doc['elements'] = []; doc['modelCues'] = []; docs[sid] = doc
    changes, excluded = [], []
    for occurrence in occurrences:
        sid = occurrence['skillId']; slot = occurrence['slot']; origin = occurrence['elementId']; event = occurrence['sourceEvent']; source_emitter = origin.split('.event_', 1)[0]
        raw_id = event['raw']['notify']; notify = notifies[raw_id]; raw = base64.b64decode(notify['serializedPayload']['data'])
        assert event['raw']['enabled'] and notify['serializedPayload']['sha256'] == event['raw']['sha256']
        # Same clip names can occur in different action variants. They are not a join key.
        if not all_skills:
            assert ('/stage-000/' in raw_id) if sid != 17170 else int(raw_id.split('/stage-')[1].split('/')[0]) == event['clipSequenceIndex']
        try:
            cue = decode_typed_payload(notify['sourceType'], notify['serializedPayload'], socket_contract, notify['assetReferences'], notify['serializedLabels'])
        except ValueError as error:
            if not all_skills: raise
            excluded.append({'slot': slot, 'sourceElement': origin, 'sourceNotify': raw_id, 'reason': 'SOURCE_ACTION_CUE_INPUT_UNCLOSED', 'detail': str(error)}); continue
        if not (cue.get('enabled') and cue.get('particleDataDecoded') and cue.get('parameterOverridesDecoded')):
            excluded.append({'slot': slot, 'sourceElement': origin, 'sourceNotify': raw_id, 'reason': 'SOURCE_ACTION_CUE_INPUT_UNCLOSED'}); continue
        if all_skills and e_material_missing(origin, by_occurrence, occurrence):
            excluded.append({'slot': slot, 'sourceElement': origin, 'sourceNotify': raw_id, 'reason': 'NATIVE_MATERIAL_OR_VERTEX_INPUT_UNCLOSED'}); continue
        e = copy.deepcopy(occurrence['element']); asset = docs[sid]['effectAssetId']; e['id'] = 'authored.source-particle.full-warlord-' + slot.lower() + '.' + hashlib.sha256(origin.encode()).hexdigest()[:20]
        e['groupId'] = 'authored.source-particle.full-warlord-' + slot.lower(); e['sourceNode'] = f'authored-source-particle:{asset}|source:effect.warlord.skill.{sid}.imported|element:{origin}'
        e['visible'] = True; e['transformInheritance'] = {'enabled': False, 'masterElementId': ''}
        lod = lods[source_emitter]; lod_key = lod['fullPath']; objects = [index.objects[r['objectPath'].lower()] for r in lod['references'] if r['property'] in ('requiredmodule', 'modules', 'typedatamodule', 'spawnmodule')]
        unsupported = [obj for obj in objects if obj.class_name in ('particlemodulelocationemitter', 'efparticlemodulelocationemitter', 'particlemoduletypedataribbon', 'particlemodulecollision')]
        if unsupported:
            excluded.append({'slot': slot, 'sourceElement': origin, 'sourceNotify': raw_id, 'reason': ('SOURCE_PARTICLE_COLLISION_EXECUTOR_UNSUPPORTED' if any(obj.class_name == 'particlemodulecollision' for obj in unsupported) else 'SOLO_EXTERNAL_EMITTER_PARTICLE_STATE'), 'modules': [{'sourceObject': obj.key, 'properties': obj.properties} for obj in unsupported], 'attempt': 'Source LOD, named owner and module properties recovered. Existing independent Solo has no live sibling particle position/velocity sampling provider; core shield/electric carrier is retained.'}); continue
        if occurrence['rendererShape'] == 'decal':
            excluded.append({'slot': slot, 'sourceElement': origin, 'sourceNotify': raw_id, 'reason': 'NATIVE_DECAL_VF_AND_PROJECTOR_PASS_UNCLOSED', 'attempt': 'Actual MIC shader map recovered; no particle VF is compiled. Reusing sprite material would discard source decal projection.'}); continue
        if cue['attachment']['mode'] == 'FOLLOW_NAMED_ANCHORS':
            if not all_skills:
                assert cue['attachment']['runtimeBoneName'] in ('b_effectroot', 'b_weapon_rhand'), cue['attachment']
            if cue['attachment']['runtimeResolutionStatus'] not in ('EXACT_SOURCE_SOCKET', 'EXACT_SOURCE_BONE'):
                if not all_skills: raise ValueError(cue['attachment'])
                excluded.append({'slot': slot, 'sourceElement': origin, 'sourceNotify': raw_id, 'reason': 'SOURCE_OWNER_ANCHOR_UNRESOLVED', 'attachment': cue['attachment']}); continue
        old_modules = {m['objectPath'].lower(): m for m in e['sourceRecipe']['modules']}
        projected, mappings, bursts = imported.emitter_detail(index, index.objects[lod_key], objects, event['globalTimeSeconds'], event['durationSeconds'], e['detail']['particle']['randomSeed'])
        e['detail']['particle'] = projected['particle']; e['detail']['timing'] = projected['timing']; e['detail']['particle']['authoringApproximate'] = False
        e['detail']['particle']['sourceScale'] = {key: 1 for key in ('count', 'size', 'lifeTime', 'speed', 'rotation', 'alpha', 'spawnDelay')}
        e['detail']['timing']['startDelaySeconds'] = event['globalTimeSeconds']
        for key in ('position', 'rotationDegrees', 'scale'): e['detail']['transform'][key] = copy.deepcopy(cue['localTransform'][key])
        attachment = imported.action_cue_attachment({'eventId': event['eventId'], 'actionCuePayload': cue, 'actionCueId': raw_id})
        if attachment is not None: e['actionCueAttachment'] = attachment
        if e['actionCueAttachment']['enabled'] and not e['actionCueAttachment']['follow'] and e['actionCueAttachment']['sourceAnchorSlotId'] == 'root': e['actionCueAttachment']['snapshotRootSourceBasisYawDegrees'] = -90
        change = {'slot': slot, 'sourceElement': origin, 'target': e['id'], 'sourceNotify': raw_id, 'sourcePayloadSha256': event['raw']['sha256'], 'sourceLOD': lod_key, 'sourceLodSha256': lod.get('serialSha256'), 'sourceCue': cue, 'parameterProjections': [], 'moduleOrder': [obj.key for obj in objects]}
        # The shield triplet differs only by its label and this signed rotator yaw.
        if sid == 17040 and source_emitter.startswith('fx_pc_wgl_02.par_n_wgl_shield_02.'):
            offset = cue['sourceTransformByteOffset'] + 40
            rotation = list(struct.unpack_from('<iii', raw, offset)); expected = {'source-event-013': 0, 'source-event-014': 5825, 'source-event-015': -5825}[event['eventId']]
            assert rotation == [0, expected, 0], (raw_id, rotation)
            e['detail']['transform']['rotationDegrees'][1] += expected * 360 / 65536
            change['sourceTripletRotator'] = {'byteOffset': offset, 'value': rotation, 'clientYawDegrees': expected * 360 / 65536, 'evidence': 'Triplet raw payloads differ only in label and signed yaw. Native wire interpretation is local to this verified triplet.'}
        parameters = {value['name'].lower(): value for value in cue['parameterOverrides']}
        e['sourceRecipe']['modules'] = []
        source_reference_counts = collections.Counter()
        for obj in objects:
            m = copy.deepcopy(old_modules.get(obj.key, {'stableId': 'source.module.' + hashlib.sha256(obj.key.encode()).hexdigest()[:24], 'className': obj.class_name, 'objectPath': obj.key}))
            # Newly recovered LOD rows can have no imported class metadata.
            # The actual source export owns the class and path in every case.
            m.update(className=obj.class_name, objectPath=obj.key)
            source_reference_counts[obj.key] += 1
            if source_reference_counts[obj.key] > 1:
                # The original LOD can list the same export twice. Preserve both
                # applications with a stable source-reference occurrence ID.
                source_reference = f'{lod_key}|{obj.key}|reference-occurrence:{source_reference_counts[obj.key]}'
                m['stableId'] = 'source.module.' + hashlib.sha256(source_reference.encode()).hexdigest()[:24]
            m['literals'], m['distributions'] = imported.flatten_source_properties(index, obj, include_source_contract_bindings=False)
            if obj.class_name == 'particlemodulelocalvectorfield':
                source = next(literal['value'] for literal in m['literals'] if literal['propertyPath'] == 'vectorfield.objectpath')
                field_asset = vectorfields[source]
                assert (ROOT / 'Client/Bin/Resources' / field_asset).is_file(), field_asset
                m['literals'].append(dict(propertyPath='vectorfield.assetid', kind='string', value=field_asset))
            if obj.class_name == 'particlemoduleorbit':
                for option in ('offsetoptions', 'rotationoptions', 'rotationrateoptions'):
                    wrapped = obj.properties.get(option)
                    if wrapped is None: continue
                    decoded = imported.unwrap(wrapped)['properties']
                    assert set(decoded) == {'bprocessduringspawn', 'bprocessduringupdate', 'buseemittertime'}
                    m['literals'] = [literal for literal in m['literals'] if not literal['propertyPath'].startswith(option + '.')]
                    m['literals'] += [{'propertyPath': option + '.' + key, 'kind': 'boolean', 'value': imported.unwrap(value)} for key, value in decoded.items()]

            for dist in m['distributions']:
                target = index.get_path(dist.get('sourceObjectPath'))
                if not target or 'particleparameter' not in target.class_name: continue
                props = {key: imported.unwrap(value) for key, value in target.properties.items()}; name = props.get('parametername', ''); n = dist['componentCount']; binding = parameters.get(name)
                if binding and binding['type'] != ('scalar' if n == 1 else 'vector'): binding = None
                def vector(key, default):
                    value = props.get(key, default)
                    return [imported.unwrap(value.get(k, default)) for k in ('x', 'y', 'z')[:n]] if isinstance(value, dict) else [value] * n
                value = vector('constant', 0); modes = [props.get('parammode' if n == 1 else ('parammodes' if i == 0 else f'parammodes[{i}]'), 'dpm_normal') for i in range(n)]
                if binding:
                    assert binding['type'] == ('scalar' if n == 1 else 'vector'), (origin,obj.key,dist,name,n,binding)
                    supplied = [binding['scalarValue']] if n == 1 else binding['vectorValue']; mini = vector('mininput', 0); maxi = vector('maxinput', 1); mino = vector('minoutput', 0); maxo = vector('maxoutput', 1)
                    for i in range(n):
                        if modes[i] == 'dpm_direct': value[i] = supplied[i]
                        elif modes[i] in ('dpm_normal', 'dpm_abs'):
                            supplied_value = abs(supplied[i]) if modes[i] == 'dpm_abs' else supplied[i]; fraction = 0 if maxi[i] == mini[i] else max(0, min(1, (supplied_value - mini[i]) / (maxi[i] - mini[i]))); value[i] = mino[i] + (maxo[i] - mino[i]) * fraction
                        else: raise ValueError(('unsupported parameter mode', name, modes))
                assert not dist['lookupTable'] and not dist['keys']
                dist['defaultMinimum'] = value + [0] * (4 - n); dist['defaultMaximum'] = value + [0] * (4 - n)
                change['parameterProjections'].append({'module': obj.key, 'property': dist['propertyPath'], 'parameter': name, 'mode': modes, 'binding': binding, 'value': value})
            e['sourceRecipe']['modules'].append(m)
            if obj.class_name == 'particlemodulespawn':
                exact = []
                for burst in imported.prop(obj.properties, 'burstlist', []):
                    count = imported.prop(burst, 'count'); low = imported.prop(burst, 'countlow', -1); time = imported.prop(burst, 'time')
                    if count > 0: exact.append({'timeSeconds': time, 'countMinimum': count if low < 0 else low, 'countMaximum': count})
                e['sourceRecipe']['bursts'] = sorted(exact, key=lambda row: row['timeSeconds'])
        required = next(obj for obj in objects if obj.class_name == 'particlemodulerequired')
        for field, prop in (('emitterDurationSeconds', 'emitterduration'), ('emitterDelaySeconds', 'emitterdelay'), ('emitterLoopCount', 'emitterloops')):
            if prop in required.properties: e['sourceRecipe'][field] = imported.unwrap(required.properties[prop])
        if e['kind'] != 'light':
            program = by_occurrence[origin]; render = ('additive' if program['nativeBlend'] == 'blend_additive' else 'alpha') + ('_two_sided_depth_read' if program['nativeTwoSided'] else '_one_sided_depth_read')
            e['material'] = {'templateId': 'effect.source_material', 'sourceMaterialPath': program['sourceMaterial'], 'renderProfile': render, 'sourceProfile': {'enabled': True, 'profileId': stable_profile_id(program['parentMaterial']), 'runtimeShaderProfileId': program['runtimeShaderProfileId'], 'parentMaterialPath': program['parentMaterial'], 'semanticStatus': 'reconstructed_profile', 'textures': [{key: value[key] for key in ('name', 'sourceObjectPath', 'assetId', 'addressU', 'addressV', 'colorSpace', 'samplingEvidence')} for value in program['textures']], 'scalars': [{'name': value['name'], 'group': 'None', 'value': value['effective']} for value in program['parameters'] if value['kind'] == 'scalar'], 'vectors': [{'name': value['name'], 'group': 'None', 'value': value['effective']} for value in program['parameters'] if value['kind'] == 'vector'], 'staticSwitches': [{'name': value['parameterName'], 'group': 'None', 'value': value['value']} for value in program['staticSwitches']], 'dynamicParameterSemantics': ['unbound'] * 4, 'subUVMode': 'none'}}
            e['detail']['uv'].update(start=[0, 0], speed=[0, 0], wave=False, sequence=False); e['detail']['color']['emissiveIntensity'] = 1
            if e['kind'] == 'screenPost':
                e['detail']['screenPost'].update(enabled=True, profileId='screen.zoom-blur.reconstructed.v1' if program['program'] == 415 else 'screen.film-noise.reconstructed.v1', status='reconstructed_profile', intensity=1, secondaryIntensity=0, frequency=1, tint=[1, 1, 1, 1], randomSeed=e['detail']['particle']['randomSeed'])
            if occurrence['rendererShape'] == 'mesh':
                e['detail']['mesh']['modelPreScale'] = .01
                for resource in e['resources']:
                    if resource['slotId'] == 'meshModel' and resource['assetId'] in geometry: resource['assetId'] = geometry[resource['assetId']]
        else:
            # Keep the existing typed light carrier and its native module inputs.
            typed = next(obj for obj in objects if obj.class_name == 'efparticlemoduletypedatalight')
            component = light_components[typed.key]
            light = merge(resolve('engine.default__lightcomponent'), resolve('engine.default__pointlightcomponent'))
            light = merge(light, resolve(component['archetypeFullPath']))
            light = merge(light, norm(component['properties']))
            color = imported.unwrap(light['lightcolor'])
            e['detail']['light'].update(enabled=True, profileId='light.point.reconstructed.v1', status='reconstructed_profile',
                range=imported.unwrap(light['radius']) * .01, intensity=imported.unwrap(light['brightness']),
                color=[color[key] / 255 for key in ('r', 'g', 'b')] + [1], ambient=[0, 0, 0, 1], falloffExponent=imported.unwrap(light['falloffexponent']))
            change['sourceLightComponent'] = component
            change['lightBoundary'] = 'Exact source PointLightComponent radius, brightness, color and class falloff projected into existing typed light. Source particle modules/CDOs retained; original EF Size-to-radius tick is not inferred. Source color alpha is editor swatch; runtime alpha one follows existing map-light adapter.'
        e['sourcePresentation'].update(sourceActionCueId=raw_id, sourceEventId=event['eventId'], sourceTimeSeconds=event['globalTimeSeconds'])
        docs[sid]['elements'].append(e); changes.append(change)
    if all_skills:
        split_docs = {}
        for sid, doc in docs.items():
            sequences = sorted({o['sourceEvent']['clipSequenceIndex'] for o in occurrences if o['skillId'] == sid})
            for sequence in sequences:
                selected_ids = {change['target'] for change in changes if any(o['elementId'] == change['sourceElement'] and o['skillId'] == sid and o['sourceEvent']['clipSequenceIndex'] == sequence for o in occurrences)}
                part = copy.deepcopy(doc); part['elements'] = [e for e in part['elements'] if e['id'] in selected_ids]
                suffix = f'.clip{sequence}' if sid == 17240 and sequence in (3, 4) else f'.ba{sequence}' if sid in (17000, 17240) else f'.clip{sequence + 1}' if len(sequences) > 1 else ''
                part['effectAssetId'] = f'effect.warlord.skill.{sid}{suffix}.full.restore'
                part['displayName'] = doc['displayName'] + (f' {suffix[1:]}' if suffix else '')
                for e in part['elements']:
                    event = next(o['sourceEvent'] for o in occurrences if o['elementId'] == next(c['sourceElement'] for c in changes if c['target'] == e['id']))
                    e['detail']['timing']['startDelaySeconds'] = event['localTimeSeconds']
                    e['sourcePresentation']['sourceTimeSeconds'] = event['localTimeSeconds']
                split_docs[part['effectAssetId']] = part
        docs = split_docs
    for doc in docs.values():
        assert len({e['id'] for e in doc['elements']}) == len(doc['elements'])
        for e in doc['elements']:
            for resource in e['resources'] + e['material'].get('sourceProfile', {}).get('textures', []):
                if resource.get('assetId'): assert (ROOT / 'Client/Bin/Resources' / resource['assetId']).is_file(), resource
        write(destination / (doc['effectAssetId'] + '.effect.json'), doc)
    report = {'sourceAction': action['source'], 'sourceFirstLODPolicy': 'First actual source LOD and canonical stage only; duplicate clip names do not merge variants.', 'counts': {str(sid): len(doc['elements']) for sid, doc in docs.items()}, 'sourceActiveCount': len(occurrences), 'excluded': excluded, 'changes': changes, 'visualValidation': 'USER_PENDING', 'originalDocumentsUnmodified': True}
    write(evidence / 'full_restore_projection.json', report)
    print('Generated', report['counts'], 'excluded', collections.Counter(row['reason'] for row in excluded))

def compose_guardian_shields(evidence, destination):
    """Apply the user's explicit 5-direction and concentric 6+6 shield layout.

    Original mesh and material remain source inputs. Placement, count, scale and
    duration are authored composition values, not recovered source occurrences.
    """
    programs = {p['program']: p for p in read(evidence / 'native_runtime_contract.json')['programs']}
    alt = [read(destination / f'effect.warlord.skill.17250.clip{i}.full.restore.effect.json') for i in (1, 2)]
    shield = next(e for e in alt[1]['elements'] if e['material'].get('sourceProfile', {}).get('runtimeShaderProfileId') == 'effect.ue3.warlord-1124-native.v1')
    vpath = destination / 'effect.warlord.skill.17170.full.restore.effect.json'
    guardian = read(vpath)
    def constant(path, values):
        values = list(values); count = len(values); padded = values + [0] * (4 - count)
        return dict(propertyPath=path, sourceClass='', sourceObjectPath='', componentCount=count,
                    operation=1, randomLockAxes=0, lookupTableChunkSize=0, lookupTableNumElements=0,
                    lookupTableTimeScale=0, lookupTableStartTime=0, defaultMinimum=padded,
                    defaultMaximum=padded, lookupTable=[], keys=[])
    def module(identifier, cls, distributions=(), literals=()):
        return dict(stableId=identifier, className=cls, objectPath=identifier,
                    literals=list(literals), distributions=list(distributions))
    def element(identifier, position, yaw, size, start, duration, program=1124):
        e = copy.deepcopy(shield); e['id'] = identifier; e['groupId'] = 'authored.warlord.guardian-shield-layout'
        e['displayName'] = identifier.rsplit('.', 1)[-1]
        e['sourceNode'] = 'project-authored:2026-09-10-user-requested-guardian-shield-layout'
        e['sourcePresentation'] = {'enabled': False}; e['actionCueAttachment']['enabled'] = False; e['actionCueAttachment']['follow'] = False
        e['detail']['transform'].update(position=position, rotationDegrees=[0, yaw, 0], scale=[1, 1, 1], velocityPerSecond=[0, 0, 0], revolutionDegreesPerSecond=[0, 0, 0])
        e['detail']['timing'].update(startDelaySeconds=start, lifeTimeSeconds=duration)
        e['detail']['particle'].update(maxParticles=1, spawnRatePerSecond=0, burstCount=1,
                                      lifeTimeSeconds=[duration, duration], localSpace=True,
                                      initialPositionMin=[0, 0, 0], initialPositionMax=[0, 0, 0],
                                      initialVelocityMin=[0, 0, 0], initialVelocityMax=[0, 0, 0],
                                      acceleration=[0, 0, 0], randomSeed=1)
        e['sourceRecipe'] = dict(enabled=True, rendererShape='mesh', emitterDelaySeconds=0,
                                emitterDurationSeconds=duration, emitterLoopCount=1,
                                bursts=[dict(timeSeconds=0, countMinimum=1, countMaximum=1)], modules=[
            module(identifier + '.required', 'particlemodulerequired', [constant('spawnrate', [0])],
                   [dict(propertyPath='buselocalspace', kind='boolean', value=True)]),
            module(identifier + '.mesh', 'particlemoduletypedatamesh', (),
                   [dict(propertyPath='boverridematerial', kind='boolean', value=True)]),
            module(identifier + '.life', 'particlemodulelifetime', [constant('lifetime', [duration])]),
            module(identifier + '.size', 'particlemodulesize', [constant('startsize', [size, size, size])]),
            module(identifier + '.color', 'particlemodulecolor', [constant('startcolor', [1, 1, 1]), constant('startalpha', [1])]),
            module(identifier + '.spawn', 'particlemodulespawn', [constant('rate', [0]), constant('ratescale', [1])])])
        e['resources'] = [dict(slotId='meshModel', assetId='Effect/Warlord/FullRestore/Meshes/sk_wgl_gdd_01.wmodel')]
        if program != 1124:
            p = programs[program]; source = e['material']['sourceProfile']
            e['material'].update(sourceMaterialPath=p['sourceMaterial'], renderProfile=('additive' if p['nativeBlend'] == 'blend_additive' else 'alpha') + ('_two_sided_depth_read' if p['nativeTwoSided'] else '_one_sided_depth_read'))
            source.update(profileId=stable_profile_id(p['parentMaterial']), runtimeShaderProfileId=p['runtimeShaderProfileId'], parentMaterialPath=p['parentMaterial'],
                          textures=[{k: t[k] for k in ('name', 'sourceObjectPath', 'assetId', 'addressU', 'addressV', 'colorSpace', 'samplingEvidence')} for t in p['textures']],
                          scalars=[dict(name=x['name'], group='None', value=x['effective']) for x in p['parameters'] if x['kind'] == 'scalar'],
                          vectors=[dict(name=x['name'], group='None', value=x['effective']) for x in p['parameters'] if x['kind'] == 'vector'],
                          staticSwitches=[dict(name=x['parameterName'], group='None', value=x['value']) for x in p['staticSwitches']])
            e['resources'] = [dict(slotId='meshModel', assetId='Effect/Warlord/FullRestore/Meshes/fm_m_sphere_004.wmodel')]
        return e
    def ring(doc, label, count, radius, size, start, duration):
        for i in range(count):
            angle = i * 360 / count; radians = math.radians(angle)
            # Shield's thin normal axis is local X. Rotate it radially outward.
            doc['elements'].append(element(f'authored.warlord.{label}.shield.{i + 1}',
                [round(radius * math.cos(radians), 6), 0, round(-radius * math.sin(radians), 6)], angle,
                size, start, duration))
    guardian['elements'] = [e for e in guardian['elements'] if not e['id'].startswith('authored.warlord.')]
    ring(guardian, 'v.outer', 5, 4.5, 2, 1.1667, 2.2)
    guardian['elements'].append(element('authored.warlord.v.personal-barrier', [0, 1.0, 0], 0, 1.7, 1.1667, 2.2, 2001))
    write(vpath, guardian)
    # Gameplay animation events own one clip clock; keep the combined tool document
    # and project independent start/loop/attack documents without repeated starts.
    for clip, offset, span in ((1, 0, 1.1667), (2, 1.1667, 1.3333), (3, 2.5, 1.1667)):
        part = copy.deepcopy(guardian); part['effectAssetId'] = f'effect.warlord.skill.17170.clip{clip}.full.restore'
        part['displayName'] = guardian['displayName'] + f' clip{clip}'; part['elements'] = []
        for e in guardian['elements']:
            authored = e['id'].startswith('authored.warlord.')
            raw = e.get('sourcePresentation', {}).get('sourceActionCueId', '')
            if (authored and clip > 1) or (not authored and f'/stage-{clip - 1:03}/' in raw):
                selected = copy.deepcopy(e)
                selected['detail']['timing']['startDelaySeconds'] = max(0, e['detail']['timing']['startDelaySeconds'] - offset)
                if authored:
                    remaining = min(span, 3.3667 - offset)
                    selected['detail']['timing']['lifeTimeSeconds'] = remaining
                    selected['detail']['particle']['lifeTimeSeconds'] = [remaining, remaining]
                    selected['sourceRecipe']['emitterDurationSeconds'] = remaining
                    for m in selected['sourceRecipe']['modules']:
                        if m['className'] == 'particlemodulelifetime': m['distributions'] = [constant('lifetime', [remaining])]
                else: selected['sourcePresentation']['sourceTimeSeconds'] -= offset
                part['elements'].append(selected)
        write(destination / (part['effectAssetId'] + '.effect.json'), part)
    for i, doc in enumerate(alt, 1):
        doc['elements'] = [e for e in doc['elements'] if not e['id'].startswith('authored.warlord.') and e['material'].get('sourceProfile', {}).get('runtimeShaderProfileId') != 'effect.ue3.warlord-1124-native.v1']
        duration = 2.4 if i == 1 else 1.7
        ring(doc, f'altv.clip{i}.outer', 6, 6.0, 2, .01 if i == 1 else 0, duration)
        ring(doc, f'altv.clip{i}.inner', 6, 1.4, 1, .01 if i == 1 else 0, duration)
        write(destination / (doc['effectAssetId'] + '.effect.json'), doc)
    write(evidence / 'guardian_shield_composition.json', dict(
        provenance='USER_REQUESTED_PROJECT_AUTHORED_LAYOUT_WITH_ORIGINAL_MESH_AND_NATIVE_MATERIAL',
        v=dict(outerCount=5, outerRadiusMetres=4.5, barrierCount=1),
        altV=dict(outerCount=6, outerRadiusMetres=6, innerCount=6, innerRadiusMetres=1.4),
        sourceShield='fx_sm_00.sk_wgl_gdd_01', material='fx_m_mi_w_00.mi.fx_w_wg_gdd_01_01_ma',
        sourceBarrier='fx_pc_wgl_03.par_y_wgl_protectshield_01', visualValidation='USER_PENDING'))


def e_material_missing(origin, programs, occurrence):
    return occurrence['rendererShape'] != 'light' and origin not in programs


if __name__ == '__main__':
    parser = argparse.ArgumentParser(); parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/WarlordASVFRestore20260909'); parser.add_argument('--action-source', type=Path, default=Path('C:/Users/user/AppData/Local/Temp/lostark-20260909-audit/warlord_action_source.json')); parser.add_argument('--output', type=Path, default=ROOT / 'Data/Effects/Authored'); parser.add_argument('--all-skills', action='store_true'); parser.add_argument('--compose-shields-only', action='store_true'); options = parser.parse_args(); compose_guardian_shields(options.evidence_root, options.output) if options.compose_shields_only else build(options.evidence_root, options.action_source, options.output, options.all_skills)

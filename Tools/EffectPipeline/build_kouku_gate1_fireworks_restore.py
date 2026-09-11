"""Restore the original SCENE03A festival/fireworks through ordinary V1 Effects.

Matinee Toggle intervals and actor transforms are read from the installed source
package. Each preview translates the original layout to its first emitter and
starts at its first activation; relative timing, scale and Cascade motion stay
in the existing source recipe. No actor animation or gameplay action is invented.
"""
from pathlib import Path
import argparse
import copy
import hashlib
import json
import math
import shutil
import struct
import sys

import build_kouku_gate1_full_restore as source
from extract_ue3_particle_graph import extract_package

ROOT = source.ROOT
SCENE = 'LV_LUT_MIDNIGHTC_ED_SCENE03A'
IDS = {0: ('festival', '쿠크 1관문 연출 축포'),
       7: ('fireworks', '쿠크 1관문 연출 폭죽')}


def unwrap(value):
    if isinstance(value, dict):
        if 'type' in value and 'value' in value:
            return unwrap(value['value'])
        if 'properties' in value:
            return unwrap(value['properties'])
        return {k: unwrap(v) for k, v in value.items()}
    if isinstance(value, list):
        return [unwrap(v) for v in value]
    return value


def extract_scene():
    package = source.load_package(source.source_package('Map', SCENE), source.ue3.LOSTARK_KR_AES_KEY)
    rows = {}
    for entry in package.exports:
        raw = package.logical[entry.serial_offset:entry.serial_offset + entry.serial_size]
        try:
            properties, _ = source.ue3.parse_tagged_properties(raw, package.names, package.summary.version)
        except source.ue3.ExtractionError:
            continue
        rows[entry.index + 1] = dict(name=source.ue3.package_ref_path(entry.index + 1, package.imports, package.exports),
            cls=source.ue3.package_ref_name(entry.class_index, package.imports, package.exports).lower(),
            p=unwrap(properties))
    imports = {-x.index - 1: source.ue3.package_ref_path(-x.index - 1, package.imports, package.exports)
               for x in package.imports}
    return rows, imports


def matinee_occurrences(rows, imports):
    result = {}
    for scene_id, (slug, label) in IDS.items():
        matinee = next(r for r in rows.values() if r['name'].endswith(f'.efseqact_matinee_{scene_id}'))
        links = {r['linkdesc'].lower(): r['linkedvariables'] for r in matinee['p']['variablelinks']}
        data = rows[links['data'][0]]
        duration = data['p']['interplength']
        groups = {rows[g]['p']['groupname'].lower(): rows[g] for g in data['p']['interpgroups']
                  if rows[g]['cls'] == 'interpgroup'}
        occurrences = []
        for group_name in ('f01', 'f02'):
            group = groups[group_name]
            tracks = [rows[t] for t in group['p']['interptracks']]
            assert len(tracks) == 1 and tracks[0]['cls'] == 'interptracktoggle', ('unexpected fireworks motion owner', group)
            intervals, start = [], None
            for event in tracks[0]['p']['toggletrack']:
                if event['toggleaction'] == 'etta_on':
                    assert start is None
                    start = event['time']
                elif event['toggleaction'] == 'etta_off':
                    assert start is not None
                    intervals.append((start, event['time']))
                    start = None
                else:
                    raise ValueError(('unsupported original toggle', event))
            if start is not None:
                intervals.append((start, duration))
            for variable in links[group_name]:
                actor_index = rows[variable]['p']['objvalue']
                actor = rows[actor_index]
                component = rows[actor['p']['particlesystemcomponent']]
                system = imports[component['p']['template']].lower()
                assert system in ('fx_q_w_01.fx_par.par_q_festiparticle_01',
                    'bfx_low_01.explosion.par_q_fireworks_01_loop', 'bfx_low_01.explosion.par_q_fireworks_02_02')
                assert component['p']['bautoactivate'] is False
                assert not actor['p'].get('base'), ('unexpected inherited moving anchor', actor)
                for interval, (on, off) in enumerate(intervals):
                    occurrences.append(dict(sourceId=f'{SCENE.lower()}/matinee-{scene_id}/{actor["name"]}/activation-{interval}',
                        sourceActor=actor['name'], sourceGroup=group['name'], sourceTrack=tracks[0]['name'], sourceSystem=system,
                        sourceStartSeconds=on, sourceStopSeconds=off, actorProperties=actor['p'], componentProperties=component['p']))
        result[scene_id] = dict(slug=slug, label=label, sourceDurationSeconds=duration, occurrences=occurrences)
    return result


def acquire(evidence):
    rows, imports = extract_scene()
    scenes = matinee_occurrences(rows, imports)
    source.write(evidence / 'source_matinee_occurrences.json', scenes)
    actions = []
    for scene_id, scene in scenes.items():
        origin = min(o['sourceStartSeconds'] for o in scene['occurrences'])
        pivot = scene['occurrences'][0]['actorProperties']['location']
        notifies = []
        for occurrence in scene['occurrences']:
            p = occurrence['actorProperties']; loc = p['location']; scale = p.get('drawscale', 1)
            scale3 = p.get('drawscale3d', dict(x=1, y=1, z=1))
            rot = p.get('rotation', {}).get('degrees', {})
            transform = dict(position=[(loc['x']-pivot['x'])*.01, (loc['z']-pivot['z'])*.01, -(loc['y']-pivot['y'])*.01],
                rotationDegrees=[-rot.get('pitch', 0), rot.get('yaw', 0), -rot.get('roll', 0)],
                scale=[scale*scale3[k] for k in ('x','z','y')])
            cue = dict(enabled=True, particleDataDecoded=True, parameterOverridesDecoded=True, parameterOverrides=[],
                attachment=dict(mode='SNAPSHOT_ROOT', sourceAnchorNames=['root'], runtimeAnchorSlotId='root'), localTransform=transform)
            notifies.append(dict(notifyId=occurrence['sourceId'], sourceType='PlayParticleEffect',
                localTimeSeconds=occurrence['sourceStartSeconds']-origin,
                durationSeconds=occurrence['sourceStopSeconds']-occurrence['sourceStartSeconds'],
                assetReferences=[dict(className='ParticleSystem', objectPath=occurrence['sourceSystem'])],
                serializedPayload=cue, serializedLabels=[]))
        actions.append(dict(actionId=scene_id, stages=[dict(stageIndex=0,
            animationClips=[dict(lengthSeconds=scene['sourceDurationSeconds']-origin)], notifies=notifies)]))
        scene.update(previewTimeOriginSeconds=origin, previewOriginUE3Cm=pivot)
    # Only adapt the source reader's input shape. These are explicitly Matinee
    # occurrences, never claimed to be serialized CEF Action notifies.
    source.ACTION = evidence / 'matinee_source_adapter.json'
    source.write(source.ACTION, dict(actions=actions, sourceKind='UE3_MATINEE_TOGGLE'))
    source.SELECTED = {i: ([0], v['label']) for i, v in scenes.items()}
    source.decode_typed_payload = lambda kind, payload, *args: copy.deepcopy(payload)
    source.GRAPH = evidence / 'graphs'
    packages = {
        'BFX_LOW_01': source.SOURCE/'CanonicalSource/Effect/Closure/SourcePackages/bfx_low_01/5XFB2C3O82H07FGCLOKEB0DX.upk',
        'FX_Q_W_01': Path('C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/Packages/ZHJ4T4L4TA4P29HVUP0UXGTD3W.upk')}
    for logical, package in packages.items():
        graph_path = source.GRAPH / (logical + '.particle-graph.json')
        if not graph_path.exists():
            graph_path.parent.mkdir(parents=True, exist_ok=True)
            # Raw source graphs can contain unrelated editor infinities; the
            # selected authored projection still rejects all nonfinite numbers.
            graph_path.write_text(json.dumps(extract_package(package, logical, source.ue3.LOSTARK_KR_AES_KEY),
                ensure_ascii=False), encoding='utf8')
    index, notifies, occurrences, records = source.acquire(evidence)
    # EmitterLoops is an integer zero default absent from the Required CDO.
    # The generic importer uses a provisional single loop for unknown data;
    # our complete CDO closure permits the original looping zero here.
    for occurrence in occurrences:
        for key in occurrence['moduleOrder']:
            module = index.objects[key]
            if module.class_name == 'particlemodulerequired' and 'emitterloops' not in module.properties:
                module.properties['emitterloops'] = dict(type='IntProperty', structType=None, value=0)
    source.write(evidence / 'preview_origin.json', scenes)
    return scenes, index, notifies, occurrences, records


def build(evidence, material_patch, output):
    scenes, index, notifies, occurrences, records = acquire(evidence)
    source.write(evidence / 'native_requirements.json', [dict(elementId=o['elementId'], sourceMaterial=o['sourceMaterial'],
        rendererShape=o['rendererShape'], sourceMesh=o['sourceMesh'], sourceSystem=o['sourceSystem'], sourceEmitter=o['sourceEmitter'])
        for o in occurrences])
    if output is None:
        return
    assert material_patch is not None or output.resolve() != (ROOT/'Data/Effects/Authored').resolve(), 'Authored projection requires recovered native materials.'
    if material_patch:
        patch = source.read(material_patch)
        hidden = [o['elementId'] for o in occurrences if o['sourceMaterial'] == 'enginematerials.defaultparticle']
        patch['programs'].append(dict(program=None, occurrences=hidden, material=dict(templateId='effect.standard',
            sourceMaterialPath='enginematerials.defaultparticle', renderProfile='alpha_two_sided_depth_read', sourceProfile=dict(enabled=False))))
        material_patch = evidence/'native_material_with_providers.json'
        source.write(material_patch, patch)
    projected = evidence / 'projected'
    source.project(evidence, index, notifies, occurrences, records, projected, material_patch)
    engine_package = source.SOURCE/'CanonicalSource/Shared/Packages/ENGINE/Source/NE1FENCQ4UNE9ZPRENOQS.u'
    package = source.load_package(engine_package, source.ue3.LOSTARK_KR_AES_KEY)
    enum = next(e for e in package.exports if source.ue3.package_ref_path(e.index + 1, package.imports, package.exports).lower() ==
        'particlemodulelocationemitter.elocationemitterselectionmethod')
    raw = package.logical[enum.serial_offset:enum.serial_offset + enum.serial_size]
    count = struct.unpack_from('<I', raw, 16)[0]
    assert len(raw) == 20 + count * 8
    selections = [package.names[struct.unpack_from('<I', raw, 20 + i * 8)[0]].lower() for i in range(count)]
    assert selections == ['elesm_random', 'elesm_sequential', 'elesm_max']
    source.write(evidence/'source_location_selection_default.json', dict(sourcePackage=str(engine_package),
        sourceEnum='particlemodulelocationemitter.elocationemitterselectionmethod', values=selections,
        absentCdoValue=0, effectiveSelectionMethod=selections[0]))
    defaults = next(e for e in package.exports if source.ue3.package_ref_path(e.index + 1, package.imports, package.exports).lower() == 'default__particlesystem')
    raw_defaults = package.logical[defaults.serial_offset:defaults.serial_offset + defaults.serial_size]
    properties, _ = source.ue3.parse_tagged_properties(raw_defaults, package.names, package.summary.version)
    macro_radius = next(v['value'] for k, v in properties.items() if k.lower() == 'macrouvradius')
    assert macro_radius == 200 and not any(k.lower() == 'macrouvposition' for k in properties)
    graph = source.read(evidence/'graphs/FX_Q_W_01.particle-graph.json')
    particle_system = next(o for o in graph['objects'] if o['objectPath'] == 'fx_par.par_q_festiparticle_01')
    assert not any(k.lower() in ('macrouvradius', 'macrouvposition') for k in particle_system['properties'])
    source.write(evidence/'source_macro_uv.json', dict(sourcePackage=str(engine_package),
        sourceDefault='engine.default__particlesystem', sourceSystem='fx_q_w_01.fx_par.par_q_festiparticle_01',
        radiusCm=macro_radius, positionUE3Cm=[0, 0, 0], radiusUnits='world-space centimetres',
        radiusScalePolicy='World-space radius is not multiplied by actor draw scale.',
        semanticDocumentation='https://dev.epicgames.com/documentation/unreal-engine/particle-expressions?application_version=4.27'))
    by_id = {o['elementId']: o for o in occurrences}
    tails = {}
    for scene_id, scene in scenes.items():
        document = source.read(projected / f'effect.kouku.gate1.{scene_id}.full.restore.effect.json')
        asset = f'effect.kouku.gate1.intro.{scene["slug"]}.full.restore'
        document['effectAssetId'] = asset
        document['displayName'] = scene['label']
        providers = {}
        for element in document['elements']:
            occurrence = by_id[element['id']]
            props = index.objects[occurrence['sourceEmitter']].properties
            if source.imported.prop(props, 'emitterrendermode', '') == 'erm_none':
                name = source.imported.prop(records[occurrence['sourceEmitter']]['properties'], 'emittername', occurrence['sourceEmitter'].rsplit('.', 1)[-1])
                assert (occurrence['sourceNotify'], name) not in providers
                providers[(occurrence['sourceNotify'], name)] = element['id']
        for element in document['elements']:
            occurrence = by_id[element['id']]
            recipe = element['sourceRecipe']
            if occurrence['sourceMaterial'] == 'fx_m_mi_00.fx_mi.fx_b_pa_cd_01_1_tr':
                required = next(m for m in recipe['modules'] if m['className'] == 'particlemodulerequired')
                required['literals'] += [dict(propertyPath='runtime.macrouv.radiuscm', kind='number', value=macro_radius)]
                required['literals'] += [dict(propertyPath='runtime.macrouv.position.' + axis, kind='number', value=0) for axis in ('x', 'y', 'z')]
            props = index.objects[occurrence['sourceEmitter']].properties
            hidden = source.imported.prop(props, 'emitterrendermode', '') == 'erm_none'
            locations = [m for m in recipe['modules'] if m['className'] == 'particlemodulelocationemitter']
            if hidden or locations:
                recipe['particleSystemOccurrenceId'] = occurrence['sourceNotify']
                recipe['emitterName'] = source.imported.prop(records[occurrence['sourceEmitter']]['properties'], 'emittername', occurrence['sourceEmitter'].rsplit('.', 1)[-1])
            if hidden:
                recipe['simulationOnly'] = True
                element['material'] = dict(templateId='effect.standard', sourceMaterialPath='enginematerials.defaultparticle',
                    renderProfile='alpha_two_sided_depth_read', sourceProfile=dict(enabled=False))
                required = next(m for m in recipe['modules'] if m['className'] == 'particlemodulerequired')
                required['literals'].append(dict(propertyPath='source.emitterrendermode', kind='string', value='erm_none'))
            for location in locations:
                if not any(v['propertyPath'] == 'selectionmethod' for v in location['literals']):
                    location['literals'].append(dict(propertyPath='selectionmethod', kind='string', value=selections[0]))
                name = next(v['value'] for v in location['literals'] if v['propertyPath'] == 'emittername')
                location['literals'].append(dict(propertyPath='runtime.providerelementid', kind='string',
                    value=providers[(occurrence['sourceNotify'], name)]))
            event_prefix = 'kouku.fx.' + hashlib.sha256(occurrence['sourceNotify'].encode()).hexdigest()[:16] + '.'
            for module in recipe['modules']:
                for literal in module['literals']:
                    if module['className'] == 'particlemoduleeventgenerator' and literal['propertyPath'].endswith('.customname'):
                        literal['value'] = event_prefix + literal['value']
                    elif module['className'] == 'particlemoduleeventreceiverspawn' and literal['propertyPath'] == 'eventname':
                        literal['value'] = event_prefix + literal['value']
            element['groupId'] = 'kouku.gate1.intro.' + scene['slug']
            # Map transforms are already in the runtime X,Z,-Y basis. The
            # animation-only RPCT root's -90 degree correction does not apply.
            element['actionCueAttachment'].pop('snapshotRootSourceBasisYawDegrees', None)
            element['displayName'] = element['sourcePresentation']['sourceObjectPath'].rsplit('.', 1)[-1]
        tails[scene['slug']] = preserve_event_tails(document)
        tails[scene['slug']]['eventReserveUpdates'] = bound_direct_event_reserves(document)
        source.write(output / (asset + '.effect.json'), document)
    source.write(evidence/'event_tail_bounds.json', tails)


def bound_direct_event_reserves(document):
    """Bound large source peak reserves by the finite activation's total births."""
    updates, generators = [], {}
    for element in document['elements']:
        for module in element['sourceRecipe']['modules']:
            if module['className'] == 'particlemoduleeventgenerator':
                values = {v['propertyPath']: v['value'] for v in module['literals']}
                route = (values['events[0].type'], values['events[0].customname'])
                assert route not in generators
                generators[route] = (element, values)
    for element in document['elements']:
        previous = element['detail']['particle']['maxParticles']
        if previous <= 1000:
            continue
        receivers = [m for m in element['sourceRecipe']['modules'] if m['className'] == 'particlemoduleeventreceiverspawn']
        if not receivers:
            continue
        assert len(receivers) == 1
        receiver = receivers[0]
        values = {v['propertyPath']: v['value'] for v in receiver['literals']}
        route = (values['eventgeneratortype'], values['eventname'])
        sender, generator = generators[route]
        recipe = sender['sourceRecipe']
        assert route[0] == 'epet_spawn' and generator['events[0].frequency'] == 1
        assert not any(m['className'] in ('particlemoduleeventreceiverspawn', 'particlemodulespawnperunit') for m in recipe['modules'])
        assert not recipe['bursts'] and sender['detail']['particle']['sourceScale']['count'] == 1
        spawn = next(m for m in recipe['modules'] if m['className'] == 'particlemodulespawn')
        rate = next(d for d in spawn['distributions'] if d['propertyPath'] == 'rate')
        scale = next(d for d in spawn['distributions'] if d['propertyPath'] == 'ratescale')
        null_scale = not scale['sourceObjectPath'] and not scale['lookupTable'] and not scale['keys'] and all(v == 0 for v in scale['defaultMinimum'] + scale['defaultMaximum'])
        rate_upper = max(source.distribution_bounds(rate)) * (1 if null_scale else max(source.distribution_bounds(scale)))
        duration = recipe['emitterDurationSeconds'] * recipe['emitterLoopCount'] if recipe['emitterLoopCount'] else sender['detail']['timing']['lifeTimeSeconds']
        # Include both endpoint steps of the existing fixed 60 Hz sampler.
        event_bound = math.ceil(max(0, rate_upper) * (duration + 2 / 60))
        spawn_count = next(d for d in receiver['distributions'] if d['propertyPath'] == 'spawncount')
        count_upper = math.ceil(max(source.distribution_bounds(spawn_count)))
        total_birth_bound = event_bound * count_upper
        assert total_birth_bound > 0
        if total_birth_bound < previous:
            element['detail']['particle']['maxParticles'] = total_birth_bound
            updates.append(dict(elementId=element['id'], originalPeakReserve=previous,
                senderElementId=sender['id'], senderRateUpperBound=rate_upper, senderActiveSeconds=duration,
                senderTotalBirthUpperBound=event_bound, receiverSpawnCountUpperBound=count_upper,
                receiverTotalBirthUpperBound=total_birth_bound))
    return updates


def preserve_event_tails(document):
    """Keep event descendants alive without extending an autonomous emitter.

    The existing timing field bounds zero-loop emission. It may be extended
    only for receivers whose original rate and bursts are both zero; source
    event births, particle lifetimes and all authored motion remain untouched.
    Bounds use the original active interval plus each upstream death lifetime.
    """
    elements = {e['id']: e for e in document['elements']}
    incoming, senders, direct = {}, {}, {}
    for key, element in elements.items():
        recipe = element['sourceRecipe']
        timing = element['detail']['timing']
        modules = recipe['modules']
        spawn = next(m for m in modules if m['className'] == 'particlemodulespawn')
        rate = next(d for d in spawn['distributions'] if d['propertyPath'] == 'rate')
        autonomous = max(source.distribution_bounds(rate)) > 0 or bool(recipe['bursts'])
        duration = recipe['emitterDurationSeconds'] * recipe['emitterLoopCount'] if recipe['emitterLoopCount'] else timing['lifeTimeSeconds']
        direct[key] = timing['startDelaySeconds'] + recipe['emitterDelaySeconds'] + duration if autonomous else None
        incoming[key] = []
        for module in modules:
            literals = {v['propertyPath']: v['value'] for v in module['literals']}
            if module['className'] == 'particlemoduleeventgenerator':
                assert not any(v.startswith('events[1]') for v in literals)
                route = (literals['events[0].type'], literals['events[0].customname'])
                assert route[0] in ('epet_spawn', 'epet_death')
                senders.setdefault(route, []).append(key)
            elif module['className'] == 'particlemoduleeventreceiverspawn':
                assert not autonomous, ('event receiver also has autonomous births', key)
                incoming[key].append((literals['eventgeneratortype'], literals['eventname']))
    resolved, visiting = {}, set()
    def last_birth(key):
        if key in resolved:
            return resolved[key]
        assert key not in visiting, 'Source event graph contains a cycle.'
        visiting.add(key)
        values = [direct[key]] if direct[key] is not None else []
        for route in incoming[key]:
            assert route in senders, ('Source event has no same-occurrence sender', route)
            for sender in senders[route]:
                birth = last_birth(sender)
                life = max(elements[sender]['detail']['particle']['lifeTimeSeconds']) if route[0] == 'epet_death' else 0
                values.append(birth + life)
        resolved[key] = max(values) if values else 0
        visiting.remove(key)
        return resolved[key]
    updates = []
    for key, element in elements.items():
        birth = last_birth(key)
        timing, recipe = element['detail']['timing'], element['sourceRecipe']
        if incoming[key] and recipe['emitterLoopCount'] == 0:
            previous = timing['lifeTimeSeconds']
            timing['lifeTimeSeconds'] = max(previous, birth - timing['startDelaySeconds'] - recipe['emitterDelaySeconds'])
            if timing['lifeTimeSeconds'] != previous:
                updates.append(dict(elementId=key, originalSeconds=previous, receiverWindowSeconds=timing['lifeTimeSeconds']))
    def end(element):
        timing, recipe = element['detail']['timing'], element['sourceRecipe']
        duration = recipe['emitterDurationSeconds'] * recipe['emitterLoopCount'] if recipe['emitterLoopCount'] else timing['lifeTimeSeconds']
        return timing['startDelaySeconds'] + recipe['emitterDelaySeconds'] + duration + max(element['detail']['particle']['lifeTimeSeconds'])
    return dict(durationMs=math.ceil(max(end(e) for e in elements.values()) * 1000),
        birthUpperBoundSeconds=resolved, receiverWindowUpdates=updates)


def prepare_geometry(evidence):
    """Cook the source leaf and reuse the installed square's exact source geometry."""
    sys.path.insert(0, str(ROOT/'Tools/ModelAssetConverter'))
    import cook_wmodel_geometry_contract as geometry
    target = ROOT/'Client/Bin/Resources/Effect/KoukuSaydon/FullRestore/Meshes'
    target.mkdir(parents=True, exist_ok=True)
    leaf = evidence/'leaf-export/BFX_SM_00/StaticMesh3/bfm_leaf_001.gltf'
    legacy = evidence/'bfm_leaf_001.legacy.wmodel'
    assert leaf.is_file() and legacy.is_file(), 'Export original bfx_sm_00.bfm_leaf_001 glTF and run ModelAssetConverter first.'
    digest = lambda p: hashlib.sha256(p.read_bytes()).digest()
    manifest = source.SOURCE/'CanonicalSource/Effect/UModelExports/BFX_SM_00/export.receipt.json'
    source_package = source.source_package('Effect', 'BFX_SM_00')
    export_log = evidence/'leaf_export.log'
    cook = evidence/'leaf_geometry_inputs.json'
    source.write(cook, dict(sourceGltf=str(leaf), legacyWmodel=str(legacy),
        sourceGltfSha256=digest(leaf).hex(), legacyWmodelSha256=digest(legacy).hex()))
    provenance = geometry.GeometryProvenanceEvidence('bfx_sm_00.bfm_leaf_001', digest(manifest),
        'OBSERVED_SOURCE_RECEIPT', digest(source_package), digest(ROOT/'Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe'),
        digest(export_log), digest(cook))
    payload, receipt = geometry.cook_wmodel_geometry_contract(leaf, legacy, provenance)
    destination = target/'bfm_leaf_001.wmodel'
    if destination.exists():
        assert destination.read_bytes() == payload, 'Existing source leaf geometry differs.'
    else:
        destination.write_bytes(payload)
    square = ROOT/'Client/Bin/Resources/Effect/Warlord/FullRestore/Meshes/fm_c_square_001.wmodel'
    geometry.parse_geometry_wmodel(square.read_bytes())
    destination = target/'fm_c_square_001.wmodel'
    if destination.exists():
        assert destination.read_bytes() == square.read_bytes(), 'Existing source square geometry differs.'
    else:
        shutil.copyfile(square, destination)
    source.write(evidence/'leaf_geometry_cook.json', receipt)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, default=ROOT/'out/KoukuFireworks20260911')
    parser.add_argument('--native-material-patch', type=Path)
    parser.add_argument('--output', type=Path)
    parser.add_argument('--prepare-geometry', action='store_true')
    options = parser.parse_args()
    options.evidence_root.mkdir(parents=True, exist_ok=True)
    if options.prepare_geometry:
        prepare_geometry(options.evidence_root)
    else:
        build(options.evidence_root, options.native_material_patch, options.output)

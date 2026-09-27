"""Install 28 proven Maharaka scene residents, not guessed Deploy NPC identities.

Selection is authored under the 30-NPC limit; appearance/transform provenance is
the exact scene actor. No attraction-guide identity is inferred. Candidate cook
and installation are separate so a failed cook cannot remove existing residents.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import os
import re
import shutil
import struct
from pathlib import Path
import subprocess
import sys
import tempfile
from types import SimpleNamespace

ROOT = Path(__file__).resolve().parents[2]
WORK = ROOT / 'out/MaharakaMapRestoration20260927/npc-replacement'
AUDIT = ROOT / 'out/MaharakaMapRestoration20260927/source-npcs/source-npc-audit.json'
SCENES = ROOT / 'out/MaharakaMapRestoration20260927/full-source-audit'
RAW = Path('C:/LostArkExtract/MaharakaNpcSource20260927/raw')
UMODEL = Path('C:/LostArkExtract/umodel/umodel_lostark_v7.exe')
PACKAGES = Path('C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/Packages')
sys.path[:0] = [str(ROOT/'Tools/ShipPipeline'), str(ROOT/'Tools/LevelPlacementExtractor')]
import cook_npc
import cook_ships as cs
import extract_ue3_placements as ue


def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def write(path, document):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(document, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')


def value(properties, name, default=None):
    return properties.get(name, {}).get('value', default)


def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def selected_actors():
    audit = read(AUDIT)
    wanted = {'LV_OCN_EVENTIS_MHP_SCENE06A': set(range(67, 91)),
              'LV_OCN_EVENTIS_MHP_SCENE07A': {127, 128, 168, 169}}
    result = []
    for scene in audit['scenes']:
        name = scene['source']['logicalPackage']
        if name not in wanted:
            continue
        assert digest(Path(scene['source']['physicalPackage'])) == scene['source']['sha256']
        source = read(SCENES/(name+'.objects.json'))
        imports = {r['packageIndex']: r for r in source['imports']}
        for actor in scene['actors']:
            number = int(actor['sourceActorId'].split(':')[-1])
            if number not in wanted[name]:
                continue
            assert not actor['attachments'] and len(actor['parts']) == 1
            assert actor['sourceTransform']['bhidden'] in (False, None)
            assert actor['sourceTransform']['base'] in (0, None)
            components = [actor['body'], actor['parts'][0]['component']]
            infos = [value(actor['actorProperties'], 'defaultmeshmaterialinfo')['properties'],
                     value(actor['parts'][0]['serializedPart'], 'materialinfo')['properties']]
            mats = []
            for component, info in zip(components, infos):
                assert len(value(info, 'materials', [0])) == 1
                reference = value(info, 'materials', [0])[0]
                if reference:
                    assert reference < 0
                    material = imports[reference]['resolvedName']
                else:
                    gltf = next(RAW.rglob(component['mesh']['objectPath'].split('.')[-1]+'.gltf'))
                    names = [m['name'] for m in read(gltf)['materials']]
                    assert len(names) == 1
                    material = component['mesh']['objectPath'].split('.')[0]+'.mat.'+names[0]
                variations = value(info, 'materialvariations', [])
                assert len(variations) <= 1, (actor['sourceActorId'], variations)
                assert not variations or value(variations[0], 'itargetindex') == 0
                assert not value(info, 'materialmaskinfos', []) and not value(info, 'bskipflags', [])
                mats.append(dict(sourceMaterial=material,
                                 variation={k: v['value'] for k, v in variations[0].items() if k != 'itargetindex'} if variations else {}))
            loops = []
            for timeline in actor['timelines']:
                for track in timeline['tracks']:
                    p = track.get('properties', {})
                    if track['className'] != 'interptrackanimcontrol' or value(p, 'bdisabletrack', False):
                        continue
                    if value(p, 'slotname') != 'a':
                        continue
                    for sequence in value(p, 'animseqs', []):
                        if value(sequence, 'blooping', False):
                            loops.append(dict(name=value(sequence,'animseqname'),
                                              rate=value(sequence,'animplayrate',1),
                                              start=value(sequence,'animstartoffset',0),
                                              end=value(sequence,'animendoffset',0)))
            clip = loops[0]['name'] if loops else 'idle_normal_1'
            if loops:
                assert loops[0]['rate'] > 0 and loops[0]['start'] == loops[0]['end'] == 0
            result.append(dict(sourceActorId=actor['sourceActorId'], lookInfo=actor['lookInfo'],
                               sourceTransform=actor['sourceTransform'],
                               body=components[0]['mesh']['objectPath'], head=components[1]['mesh']['objectPath'],
                               animSets=sorted({r['objectPath'] for r in actor['body']['animSets']} |
                                               {r['objectPath'] for t in actor['timelines'] for r in t['animSets']}),
                               materials=mats, idleClip=clip, playbackRate=loops[0]['rate'] if loops else 1,
                               motionBasis='FIRST_ORIGINAL_FULL_BODY_LOOP' if loops else 'SOURCE_ANIMSET_IDLE_SELECTION',
                               numericNpcId=None, guideRole=None))
    assert len(result) == 28 and len({r['sourceActorId'] for r in result}) == 28
    return result


def extract():
    actors = selected_actors()
    write(WORK/'selected.json', actors)
    refs = {s for a in actors for s in a['animSets']}
    refs |= {m['sourceMaterial'] for a in actors for m in a['materials']}
    for ref in sorted(refs):
        leaf = ref.split('.')[-1]
        suffix = '.psa' if ref in {s for a in actors for s in a['animSets']} else '.props.txt'
        if list(RAW.rglob(leaf+suffix)):
            continue
        physical = ue.resolve_physical_package(UMODEL, PACKAGES, ref.split('.')[0], 'kr')
        command = [str(UMODEL), '-export', '-game=lostark', '-kr', '-nameresolve', '-dds',
                   '-path='+str(PACKAGES), '-out='+str(RAW), '-obj='+leaf, str(physical)]
        run = subprocess.run(command, capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
        (WORK/(leaf+'.export.log')).write_bytes(run.stdout+run.stderr)
        if run.returncode or not list(RAW.rglob(leaf+suffix)):
            raise RuntimeError(f'export failed {ref}; see log')
        print('EXTRACTED', ref, flush=True)
    print('SELECTED', len(actors), 'MATERIALS', sorted({m['sourceMaterial'] for a in actors for m in a['materials']}))


def one_file(pattern):
    matches = list(RAW.rglob(pattern))
    if not matches or len({digest(p) for p in matches}) != 1:
        raise ValueError(f'{pattern}: expected one source file, found {len(matches)}')
    return sorted(matches)[0]


def select_clip(psa, clip, rate, destination):
    """Preserve the selected original frame AND scale tracks, changing only time rate."""
    data = psa.read_bytes()
    chunks = cs.read_psa_chunks(data)
    info, bones = chunks['ANIMINFO'], chunks['BONENAMES']
    records = [bytearray(data[info['offset']+i*info['size']:info['offset']+(i+1)*info['size']])
               for i in range(info['count'])]
    record, = [r for r in records if r[:64].split(b'\0')[0].decode('ascii') == clip]
    first, frames = struct.unpack_from('<ii', record, 160)
    source_rate, = struct.unpack_from('<f', record, 152)
    struct.pack_into('<f', record, 152, source_rate*rate)
    struct.pack_into('<i', record, 160, 0)
    output = bytearray()
    for name, chunk in chunks.items():
        count = chunk['count']
        payload = data[chunk['offset']:chunk['offset']+chunk['size']*count]
        if name == 'ANIMINFO':
            count, payload = 1, record
        elif name in ('ANIMKEYS', 'SCALEKEYS'):
            stride = bones['count']*chunk['size']
            payload = payload[first*stride:(first+frames)*stride]
            count = frames*bones['count']
            assert len(payload) == count*chunk['size']
        output += chunk['raw_name']+struct.pack('<iii', chunk['flag'], chunk['size'], count)+payload
    destination.write_bytes(output)
    return dict(sourcePsa=str(psa), sourcePsaSha256=digest(psa), clip=clip, frames=frames,
                sourceRate=source_rate, playbackRate=rate,
                seconds=(frames-1)/(source_rate*rate), scaleKeysPreserved='SCALEKEYS' in chunks)


def source_tangent_basis(model_path, staged_path):
    """Join every cooked vertex to the source, then preserve reflected handedness."""
    sys.path.insert(0,str(ROOT/'Tools/ModelAssetConverter'))
    from cook_wmodel_geometry_contract import parse_skinned_uv_wmodel, cook_skinned_basis_uv_contract
    from verify_dimensionmaster_summon_bind_pose import read_wmodel
    document = read(staged_path)
    buffers = [(staged_path.parent/b['uri']).read_bytes() for b in document['buffers']]
    def accessor(index):
        a = document['accessors'][index]
        assert 'sparse' not in a and not a.get('normalized',False)
        v = document['bufferViews'][a['bufferView']]
        kind = {5121:'B',5123:'H',5126:'f'}[a['componentType']]
        width = {'VEC2':2,'VEC3':3,'VEC4':4}[a['type']]
        fmt = '<'+str(width)+kind
        stride = v.get('byteStride',struct.calcsize(fmt))
        start = v.get('byteOffset',0)+a.get('byteOffset',0)
        return [struct.unpack_from(fmt,buffers[v['buffer']],start+i*stride) for i in range(a['count'])]
    data = model_path.read_bytes()
    parsed = parse_skinned_uv_wmodel(data)
    model = read_wmodel(model_path,include_geometry=False,animation_names=())
    joints = [document['nodes'][i]['name'] for i in document['skins'][0]['joints']]
    primitives = document['meshes'][0]['primitives']
    assert len(primitives) == len(parsed['submeshes'])
    signs, worst, count = {}, 0.0, 0
    for submesh,(primitive,desc) in enumerate(zip(primitives,parsed['submeshes'])):
        attrs = {name:accessor(index) for name,index in primitive['attributes'].items()}
        assert len(attrs['POSITION']) == desc[1]
        signs[submesh] = []
        for i in range(desc[1]):
            offset = parsed['vertexStart']+desc[0]+i*parsed['meshHeader'][4]
            blob = parsed['mesh']
            for name,at,width in [('POSITION',0,3),('NORMAL',12,3),('TEXCOORD_0',24,2),('TANGENT',32,3)]:
                actual = struct.unpack_from('<'+str(width)+'f',blob,offset+at)
                expected = list(attrs[name][i][:width])
                if width == 3:
                    expected[2] = -expected[2]  # Actual converter's glTF -> LH basis.
                error = max(abs(a-b) for a,b in zip(actual,expected))
                assert all(math.isfinite(a) for a in actual) and error < 2e-3, (submesh,i,name,error)
                worst = max(worst,error)
            indices = struct.unpack_from('<4I',blob,offset+44)
            weights = struct.unpack_from('<4f',blob,offset+60)
            source_weights, cooked_weights = {}, {}
            for joint,weight in zip(attrs['JOINTS_0'][i],attrs['WEIGHTS_0'][i]):
                if weight > 1e-6:
                    name = joints[joint]
                    source_weights[name] = source_weights.get(name,0)+weight
            for joint,weight in zip(indices,weights):
                if weight > 1e-6:
                    name = model.mesh_bones[joint].name
                    cooked_weights[name] = cooked_weights.get(name,0)+weight
            assert source_weights.keys() == cooked_weights.keys(), (submesh,i,'weighted joints')
            assert all(abs(w-cooked_weights[n]) < 1e-5 for n,w in source_weights.items())
            signs[submesh].append(-attrs['TANGENT'][i][3])
            count += 1
    result,receipt = cook_skinned_basis_uv_contract(data,{},signs)
    receipt.update(sourceGltfSha256=digest(staged_path),joinedVertices=count,maxGeometryDelta=worst,
                   join='All P/N/T reflected Z, unchanged UV0, weighted bone names/weights; no positional nearest match',
                   tangentSign='negative source glTF TANGENT.w: Z reflection, no UV flip')
    return result,receipt


def upgrade_basis():
    """Upgrade this install only; refuse changed resources or authoring receipts."""
    cooked = read(WORK/'cooked.json')
    proof_path = ROOT/'Data/Actors/MaharakaResidents.source.json'
    proof = read(proof_path)
    proofs = {r['modelAssetId']:r for r in proof['models']}
    changes = {}
    for item in cooked['models'].values():
        target = WORK/'cook'/item['asset']
        path = target/(item['asset']+'.wmodel')
        installed = ROOT/'Client/Bin/Resources'/item['modelAssetId']
        assert digest(path) == digest(installed) == item['sha256'] == proofs[item['modelAssetId']]['sha256']
        payload,receipt = source_tangent_basis(path,target/'staged.gltf')
        if payload != path.read_bytes():
            backup = WORK/'backup/resources'/item['modelAssetId']
            if not backup.exists():
                atomic_bytes(backup,path.read_bytes())
            changes[path] = payload
            changes[installed] = payload
        item.update(sha256=hashlib.sha256(payload).hexdigest(),basis=receipt)
        proofs[item['modelAssetId']].update(sha256=item['sha256'],basis=receipt)
        changes[target/'complete.json'] = (json.dumps(item,ensure_ascii=False,indent=2)+'\n').encode('utf-8')
    for path,doc in [(WORK/'cooked.json',cooked),(proof_path,proof)]:
        changes[path] = (json.dumps(doc,ensure_ascii=False,indent=2)+'\n').encode('utf-8')
    before = {p:p.read_bytes() for p in changes}
    committed = []
    try:
        for path,payload in changes.items():
            if path.read_bytes() != before[path]:
                raise RuntimeError('Source basis target changed: '+str(path))
            atomic_bytes(path,payload)
            committed.append(path)
    except BaseException:
        for path in reversed(committed):
            atomic_bytes(path,before[path])
        raise
    print('SOURCE BASIS INSTALLED',len(cooked['models']),'models',flush=True)


def cook():
    actors = selected_actors()
    programs = read(WORK/'native-programs.json')
    models = {}
    for actor in actors:
        identity = {key: actor[key] for key in ('body', 'head', 'materials', 'idleClip', 'playbackRate')}
        key = hashlib.sha256(json.dumps(identity,sort_keys=True).encode()).hexdigest()[:16]
        asset = 'MHP_RESIDENT_'+key.upper()
        actor.update(modelAssetId=f'Character/NPC/Maharaka/Residents/{asset}/{asset}.wmodel',
                     archetypeId='NPC_'+asset, clientPresentationId='npc.maharaka.resident.'+key+'.v1')
        if key in models:
            continue
        target = WORK/'cook'/asset
        completed = target/'complete.json'
        if completed.exists():
            previous = read(completed)
            assert previous['identity'] == identity
            assert digest(target/(asset+'.wmodel')) == previous['sha256']
            models[key] = previous
            continue
        target.mkdir(parents=True, exist_ok=True)
        body = one_file(actor['body'].split('.')[-1]+'.gltf')
        head = one_file(actor['head'].split('.')[-1]+'.gltf')
        merged = cook_npc.merge_head_into_body(body,head,target/'merged.gltf',target/'merged.bin')
        candidates = []
        for ref in actor['animSets']:
            path = one_file(ref.split('.')[-1]+'.psa')
            if actor['idleClip'] in dict(cs.psa_clip_frames(path)):
                candidates.append(path)
        if len(candidates) != 1:
            raise ValueError(f'ambiguous/missing exact clip {actor["sourceActorId"]}: {candidates}')
        motion = select_clip(candidates[0],actor['idleClip'],actor['playbackRate'],target/'clip.psa')
        cs.run([sys.executable,'-B',cs.STAGER,'--gltf',target/'merged.gltf','--psa',target/'clip.psa',
                '--output-gltf',target/'staged.gltf','--output-bin',target/'staged.bin',
                '--report',target/'stage.json','--scale','100','--allow-bone-order-remap','--overwrite'],ROOT,'stage')
        args = [cs.CONVERTER,'staged.gltf','-o',asset+'.wmodel','--no-auto-textures']
        for slot, material in zip(merged['bodyMaterials']+merged['headMaterials'],actor['materials']):
            native = read(WORK/'native'/(material['sourceMaterial']+'.json'))
            for name,flag in [('texture_diffuse','--material-remap'),('texture_normal','--normal-remap')]:
                texture, = [t for t in native['textures'] if t['parameterName'] == name]
                source = one_file(texture['sourceObject'].split('.')[-1]+'.dds')
                args.extend([flag,slot+'='+str(source)])
        cs.run(args,target,'convert')
        cs.run([sys.executable,'-B',cs.RETIMER,'--wmodel',target/(asset+'.wmodel'),
                '--ticks-per-second','30','--expect-ticks-per-second','1000'],ROOT,'retime')
        payload,basis = source_tangent_basis(target/(asset+'.wmodel'),target/'staged.gltf')
        atomic_bytes(target/(asset+'.wmodel'),payload)
        analysis = cs.analyse(target/(asset+'.wmodel'),[])
        animation = analysis['clips'][actor['idleClip']]
        assert abs(animation['seconds']-motion['seconds']) < .001
        stage = read(target/'stage.json')
        receipt = dict(identity=identity,asset=asset,modelAssetId=actor['modelAssetId'],
                       sha256=digest(target/(asset+'.wmodel')),merge=merged,motion=motion,
                       analysis=analysis,stage=stage,basis=basis,
                       materialSlots=merged['bodyMaterials']+merged['headMaterials'])
        write(completed,receipt)
        models[key] = receipt
        print('COOKED',asset,actor['lookInfo'],actor['idleClip'],motion['seconds'],flush=True)
    write(WORK/'cooked.json',dict(actors=actors,models=models,programs=programs))
    print('COOK COMPLETE',len(actors),'placements',len(models),'unique presentation models',flush=True)


def mask_default_evidence():
    package = PACKAGES.parent/'NU1V7NCQ4YAE9ZPJVNOQS.u'
    raw = package.read_bytes()
    summary = ue.parse_summary(raw)
    data = ue.decompress_package(raw,summary,ue.LOSTARK_KR_AES_KEY)
    names = ue.parse_name_table(data,summary)
    exports = ue.parse_export_table(data,summary,names)
    entry = exports[3103]
    assert entry.object_name.lower() == 'efmaterialvariation'
    serial = data[entry.serial_offset:entry.serial_offset+entry.serial_size]
    properties,end = ue.parse_tagged_properties_at(serial,names,52,'EFMaterialVariation')
    defaults = [value(properties,'MaskVariation_'+str(i)) for i in range(1,5)]
    assert end == len(serial) and defaults == [True]*4
    return dict(package=package.name,sha256=digest(package),exportIndex=3103,
                propertyBytesConsumed=end-52,defaultVisibility=defaults,
                conversion='true=1; false=0; source CDO and native shader channel visibility',
                boundary='Native C++ assignment unavailable; original CDO, MIC and shader semantics agree.')


def atomic_bytes(path, payload):
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, temp = tempfile.mkstemp(prefix=path.name+'.',suffix='.tmp',dir=path.parent)
    try:
        with os.fdopen(fd,'wb') as stream:
            stream.write(payload)
        os.replace(temp,path)
    finally:
        if os.path.exists(temp):
            os.unlink(temp)


def install():
    cooked = read(WORK/'cooked.json')
    actors, models = cooked['actors'], cooked['models']
    source = selected_actors()
    assert all({k:a[k] for k in s} == s for a,s in zip(actors,source)) and len(actors) == len(source) == 28
    mask_evidence = mask_default_evidence()
    catalog_path = ROOT/'Data/Actors/NpcCatalog.json'
    world_path = ROOT/'Data/Worlds/LV_OCN_EVENTIS_MHP/Gameplay.world.json'
    proof_path = ROOT/'Data/Actors/MaharakaResidents.source.json'
    before = {p:p.read_bytes() if p.exists() else None for p in (catalog_path,world_path,proof_path)}
    catalog, world = read(catalog_path), read(world_path)
    old = [p for p in world['placements'] if p['placementId'].startswith('npc.maharaka.reconstructed.')]
    already = [p for p in world['placements'] if p['placementId'].startswith('npc.maharaka.scene')]
    if already:
        raise RuntimeError('Residents already installed; validate them instead of rewriting user edits.')
    if len(old) != 28 or len(world['placements']) != 34:
        raise RuntimeError('The expected 28 replacement targets changed; preserving current World.')
    preserved = [p for p in world['placements'] if p not in old]
    resources = ROOT/'Client/Bin/Resources'
    texture_map = {}
    resource_copies = {}
    for path in sorted((WORK/'native').glob('*.json')):
        native = read(path)
        for texture in native['textures']:
            leaf = texture['sourceObject'].split('.')[-1]
            asset_id = 'Character/NPC/Maharaka/Residents/Textures/'+leaf+'.dds'
            original = one_file(leaf+'.dds')
            texture_map[leaf] = asset_id
            resource_copies[resources/asset_id] = original
    write(WORK/'texture-map.json',texture_map)
    sys.path.insert(0,str(ROOT/'Tools/VehiclePipeline'))
    import build_vehicle_source_material as material_tool
    new_catalog, new_materials = [], []
    by_model = {a['modelAssetId']:a for a in actors}
    for item in models.values():
        asset = item['asset']
        model_path = WORK/'cook'/asset/(asset+'.wmodel')
        assert digest(model_path) == item['sha256']
        actor = by_model[item['modelAssetId']]
        new_catalog.append({k:actor[k] for k in ('archetypeId','clientPresentationId','modelAssetId','idleClip')} |
                           dict(animationSetId=None,runtimeStatus='supported'))
        resource_copies[resources/item['modelAssetId']] = model_path
        for texture in (model_path.parent/'textures').glob('*'):
            resource_copies[(resources/item['modelAssetId']).parent/'textures'/texture.name] = texture
        for slot,material in zip(item['materialSlots'],actor['materials']):
            dump = WORK/'native'/(material['sourceMaterial']+'.json')
            family = cooked['programs'][material['sourceMaterial']]['family']
            rowfile = WORK/'rows'/(asset+'.'+slot+'.json')
            rowfile.parent.mkdir(parents=True,exist_ok=True)
            material_tool.command_rows(SimpleNamespace(model=actor['modelAssetId'], texture_map=WORK/'texture-map.json',
                                      out=rowfile,entry=[str(dump)+'='+family+'@'+slot]))
            row, = read(rowfile)
            variation = material['variation']
            for name in ('diffusecolor','diffusecolor_a','diffusecolor_b','diffusecolor_c'):
                if name in variation:
                    assert name in row['parameters']
                    row['parameters'][name] = [variation[name][channel] for channel in ('r','g','b','a')]
            if variation:
                masks = [variation['maskvariation_'+str(i)] for i in range(1,5)]
                assert all(type(m) is bool for m in masks)
                row['parameters']['mask_variation_visible'] = [int(m) for m in masks]
            new_materials.append(row)
    assert not ({r['archetypeId'] for r in new_catalog} & {r['archetypeId'] for r in catalog['npcs']})
    placements = []
    for actor in actors:
        scene,_,number = actor['sourceActorId'].split(':')
        transform = actor['sourceTransform']
        assert transform['drawscale'] in (None,1) and transform['drawscale3d'] in (None,{'x':1,'y':1,'z':1})
        rotation = transform['rotation']['degrees']
        assert rotation['pitch'] == rotation['roll'] == 0
        p = transform['location']
        row = dict(placementId='npc.maharaka.'+scene.rsplit('_',1)[-1].lower()+'.source'+number,
                   kind='npc',archetypeId=actor['archetypeId'],encounterId=None,idleClip=None,behavior=None,
                   position=[p['x']/100,p['z']/100,-p['y']/100],yawDegrees=(rotation['yaw']+90)%360,enabled=True)
        placements.append(row)
        actor['placementId'] = row['placementId']
    catalog['npcs'].extend(new_catalog)
    catalog.setdefault('modelMaterialOverrides',[]).extend(new_materials)
    world['placements'] = preserved+placements
    world['revision'] += 1
    assert len([p for p in world['placements'] if p['kind']=='npc']) == 30
    for destination,original in resource_copies.items():
        if destination.exists() and digest(destination) != digest(original):
            raise RuntimeError('Refusing to overwrite differing resource '+str(destination))
    evidence = dict(schema='lostark.maharaka-source-residents',formatVersion=1,
                    sourceAuditSha256=digest(AUDIT),selection='28 source scene residents under 30-NPC cap; no guide identity inference',
                    animationBoundary='First original full-body loop, or original AnimSet idle; not entire Matinee playback.',
                    maskEvidence=mask_evidence,actors=actors,
                    models=[{k:r[k] for k in ('modelAssetId','sha256','motion','materialSlots','merge','basis')} for r in models.values()],
                    preservedPlacements=preserved,removedPlacementIds=[p['placementId'] for p in old])
    after = {catalog_path:catalog,world_path:world,proof_path:evidence}
    for path,payload in before.items():
        if (path.read_bytes() if path.exists() else None) != payload:
            raise RuntimeError('Authoring changed during candidate stage: '+str(path))
        if payload is not None:
            backup = WORK/'backup'/path.relative_to(ROOT)
            if backup.exists() and backup.read_bytes() != payload:
                raise RuntimeError('Backup already belongs to another authoring revision')
            if not backup.exists():
                atomic_bytes(backup,payload)
    # Fresh resource files are harmless until the catalog/world references commit.
    for destination,original in resource_copies.items():
        if not destination.exists():
            destination.parent.mkdir(parents=True,exist_ok=True)
            shutil.copyfile(original,destination)
    committed = []
    try:
        for path,document in after.items():
            atomic_bytes(path,(json.dumps(document,ensure_ascii=False,indent=2)+'\n').encode('utf-8'))
            committed.append(path)
    except BaseException:
        for path in reversed(committed):
            if before[path] is None:
                path.unlink()
            else:
                atomic_bytes(path,before[path])
        raise
    write(WORK/'installed.json',dict(placements=28,npcTotal=30,models=len(models),materials=len(new_materials),
                                   resources=[str(p.relative_to(resources)).replace('\\','/') for p in resource_copies]))
    print('INSTALLED 28 original scene residents; total NPC 30; models',len(models),'materials',len(new_materials),flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=['extract', 'materials', 'cook', 'install', 'basis'])
    args = parser.parse_args()
    if args.command == 'extract':
        extract()
    elif args.command == 'cook':
        cook()
    elif args.command == 'install':
        install()
    elif args.command == 'basis':
        upgrade_basis()
    else:
        sys.path.insert(0, str(ROOT/'Tools/VehiclePipeline'))
        import build_vehicle_source_material as m
        families = re.findall(r'family == "([^"]+)"', m.read_text(m.PARAMETER_HEADER))
        blocks = {}
        for family in families:
            try:
                blocks[family] = m.installed_configure(family)
            except SystemExit:
                continue
        norm = lambda t: re.sub(r'\s+', '', re.sub(r'//[^\n]*', '', t))
        found = {}
        for path in sorted((WORK/'native').glob('*.json')):
            doc = read(path)
            matches = []
            for family, block in blocks.items():
                if not block:
                    continue
                number = int(re.search(r'staged.program = (\d+)u', block)[1])
                if norm(m.emit_configure(doc, family, number)) != norm(block):
                    continue
                print('CONFIG_MATCH', family, number, flush=True)
                exact = True
                for stage, installed in [('base', m.BASE_PROGRAMS), ('light', m.LIGHT_PROGRAMS)]:
                    name = ('SourceCharacterBase' if stage == 'base' else 'SourceCharacterLight')+str(number)
                    exact &= norm(m.installed_function(installed,name) or '') == norm(m.emit_function(doc,family,number,stage))
                if exact:
                    matches.append(dict(family=family, program=number))
            print(path.name, matches, flush=True)
            if not matches:
                raise RuntimeError('No exact installed native program: '+str(path))
            found[doc['sourceMaterial']] = matches[0]
        write(WORK/'native-programs.json', found)


if __name__ == '__main__':
    main()

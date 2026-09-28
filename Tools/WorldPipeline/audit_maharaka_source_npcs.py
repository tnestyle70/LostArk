"""Recover Maharaka NPC identity evidence, never substitute an unrelated NPC.

Deploy NPC definition IDs and UE scene actors are separate namespaces. A scene
LookInfo proves appearance use in this map, NOT a missing numeric NPC definition
or an attraction-guide role. This tool only writes investigation output.
"""
from __future__ import annotations

import argparse
import array
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import sqlite3
import struct
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/LpkPipeline'))
from dump_loa_strings import strings
import unpack_lpk as lpk
sys.path.insert(0, str(ROOT / 'Tools/LevelPlacementExtractor'))
import extract_ue3_placements as ue


def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def value(props, key, default=None):
    return props.get(key, {}).get('value', default)


def read_deploy(path, database):
    raw = path.read_bytes()
    marker = b'CEFDeployActor_NPC\0'
    records = []
    for match in re.finditer(re.escape(marker), raw):
        at = match.start()
        # UE's serialized string length includes its terminating null byte.
        if struct.unpack_from('<I', raw, at - 4)[0] != 19:
            raise ValueError(f'invalid NPC class marker at {at}')
        start = at + len(marker)
        if start + 0x68 > len(raw):
            raise ValueError('truncated NPC actor')
        position = struct.unpack_from('<3f', raw, start)
        actor, = struct.unpack_from('<i', raw, start + 0x30)
        npc, = struct.unpack_from('<i', raw, start + 0x64)
        yaw, = struct.unpack_from('<i', raw, start + 0x10)
        row = database.execute('SELECT * FROM Npc WHERE PrimaryKey=?', (npc,)).fetchone()
        records.append(dict(actorId=actor, npcId=npc, byteOffset=start,
                            sourcePositionCm=list(position),
                            position=[position[0]/100, position[2]/100, -position[1]/100],
                            sourceYawUnits=yaw, scalePercent=struct.unpack_from('<i', raw, start+0x38)[0],
                            definition=dict(row) if row is not None else None,
                            status='EXACT_DEPLOY_TABLE_JOIN' if row is not None else 'MISSING_TABLE_ROW_NOT_MODEL_ABSENCE'))
    if not records or len({r['actorId'] for r in records}) != len(records):
        raise ValueError('missing or duplicate NPC actor IDs')
    return records


def read_scene(path):
    doc = json.loads(path.read_text(encoding='utf-8'))
    source = doc['source']
    if digest(Path(source['physicalPackage'])).lower() != source['sha256'].lower():
        raise ValueError(f'stale source dump: {path}')
    # The general inventory kept only the first 32 bytes of these unknown
    # structs. Re-read them from the package; that hex preview is not a recipe.
    if any('lookinfokey' in r.get('properties', {}) for r in doc['exports']):
        raw = Path(source['physicalPackage']).read_bytes()
        summary = ue.parse_summary(raw)
        data = ue.decompress_package(raw, summary, ue.LOSTARK_KR_AES_KEY)
        names = ue.parse_name_table(data, summary)
        native_exports = ue.parse_export_table(data, summary, names)
        decode = ue.decode_property_value

        def decode_actor_material(kind, structure, payload, names, boolean, *context):
            if kind.lower() == 'structproperty' and (structure or '').lower() in (
                    'eflookinfosmactorpartmaterialinfo', 'attachment'):
                nested, end = ue.parse_tagged_properties_at(payload, names, 0, structure)
                if end != len(payload):
                    raise ValueError(f'unconsumed actor material/attachment bytes: {structure}')
                return dict(size=len(payload), properties=nested)
            if kind.lower() == 'structproperty' and (structure or '').lower() == 'linearcolor':
                if len(payload) != 16:
                    raise ValueError('invalid linear color size')
                return dict(zip(('r','g','b','a'), struct.unpack('<4f', payload)))
            return decode(kind, structure, payload, names, boolean, *context)

        ue.decode_property_value = decode_actor_material
        try:
            for row in doc['exports']:
                if row['className'] != 'efskeletalmeshactorlookinfomat':
                    continue
                native = native_exports[row['packageIndex']-1]
                props, end = ue.parse_tagged_properties(
                    data[native.serial_offset:native.serial_offset+native.serial_size], names, summary.version)
                row['properties'] = props
        finally:
            ue.decode_property_value = decode
    exports = {r['packageIndex']: r for r in doc['exports']}
    imports = {r['packageIndex']: r for r in doc['imports']}

    def reference(index):
        if index == 0:
            return None  # explicit None: preserve for mesh/default material resolution
        row = (exports if index > 0 else imports)[index]
        return dict(packageIndex=index, objectPath=row.get('resolvedName', row['objectName']),
                    className=row['className'])

    def component(index):
        row = exports[index]
        if row['className'] != 'skeletalmeshcomponent':
            raise ValueError(f'not a skeletal mesh component: {index}')
        p = row['properties']
        return dict(packageIndex=index, mesh=reference(value(p, 'skeletalmesh', 0)),
                    animSets=[reference(x) for x in value(p, 'animsets', [])],
                    materials=[reference(x) for x in value(p, 'materials', [])],
                    properties=p)

    actors = {}
    for row in exports.values():
        p = row.get('properties', {})
        if row['className'] == 'efskeletalmeshactorlookinfomat' and not p:
            raise ValueError(f'unparsed LookInfo actor: {path}:{row["packageIndex"]}')
        if 'lookinfokey' not in p:
            continue
        parts = []
        for part in value(p, 'additionalpartsex', []):
            index = value(part, 'partcomp', 0)
            if index:
                parts.append(dict(component=component(index), serializedPart=part))
        actors[row['packageIndex']] = dict(
            sourceActorId=f"{source['logicalPackage']}:export:{row['packageIndex']-1}",
            packageIndex=row['packageIndex'], lookInfo=value(p, 'lookinfokey'),
            sourceTransform={k: value(p, k) for k in ('location','rotation','drawscale','drawscale3d','base','bhidden')},
            body=component(value(p, 'skeletalmeshcomponent')), parts=parts,
            attachments=value(p, 'additionalpartsattachedtosocketex', []),
            actorProperties=p, timelines=[],
            numericNpcId=None, guideRole=None,
            status='EXACT_SCENE_ACTOR_NOT_DEPLOY_NPC_ID')
    # Resolve Matinee.Data -> InterpData.groups, then name -> linked SeqVar -> actor.
    # Group names are only used within this exact Matinee, never globally.
    for row in exports.values():
        if row['className'] != 'efseqact_matinee':
            continue
        p = row['properties']
        links = value(p, 'variablelinks', [])
        data_links = [x for x in links if value(x, 'linkdesc') == 'Data']
        if len(data_links) != 1:
            continue
        groups = {}
        for data_id in value(data_links[0], 'linkedvariables', []):
            data = exports[data_id]
            for group_id in value(data['properties'], 'interpgroups', []):
                group = exports[group_id]
                groups[value(group['properties'], 'groupname')] = group
        for link in links:
            group = groups.get(value(link, 'linkdesc'))
            if group is None:
                continue
            gp = group['properties']
            tracks = [exports[i] for i in value(gp, 'interptracks', [])]
            for var_id in value(link, 'linkedvariables', []):
                var = exports.get(var_id)
                if not var or var['className'] != 'seqvar_object':
                    continue
                actor = actors.get(value(var['properties'], 'objvalue'))
                if actor is not None:
                    actor['timelines'].append(dict(
                        matinee=row['packageIndex'], variable=var_id, group=group['packageIndex'],
                        groupName=value(gp, 'groupname'), looping=value(p, 'blooping'),
                        animSets=[reference(i) for i in value(gp, 'groupanimsets', [])], tracks=tracks))
    return dict(source=source, actors=list(actors.values()))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-root', type=Path, default=Path('C:/LostArkExtract/MaharakaFunctions20260926'))
    parser.add_argument('--scene-root', type=Path, default=ROOT/'out/MaharakaMapRestoration20260927/full-source-audit')
    parser.add_argument('--game-root', type=Path, default=Path('C:/ProgramData/Smilegate/Games/LOSTARK/EFGame'))
    parser.add_argument('--archive-index', type=Path, default=ROOT/'out/MaharakaReaudit20260926/archive_index.json')
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    dbpath = args.source_root/'db/EFGame_Extra/ClientData/TableData/EFTable_Npc.db'
    deploypath = args.source_root/'mapdata/Common_Extra/MapData/57009/DeployData.loa'
    with sqlite3.connect(dbpath.as_uri()+'?mode=ro', uri=True) as database:
        database.row_factory = sqlite3.Row
        deploy = read_deploy(deploypath, database)
    scenes = [read_scene(p) for p in sorted(args.scene_root.glob('LV_OCN_EVENTIS_MHP_*.objects.json'))]
    actors = [a for s in scenes for a in s['actors']]
    wanted = {a['lookInfo'].lower(): a['lookInfo'] for a in actors}
    wanted.update({r['definition']['Model'].lower(): r['definition']['Model'] for r in deploy if r['definition']})
    entries = json.loads(args.archive_index.read_text(encoding='utf-8'))
    # Speed up the existing archive decoder without changing its byte-order contract.
    def swap(raw):
        words = array.array('I'); words.frombytes(raw); words.byteswap(); return words.tobytes()
    if swap(bytes(range(32))) != lpk.swap32(bytes(range(32))):
        raise ValueError('archive byte order mismatch')
    lpk.swap32 = swap
    lookinfos = []
    for key, label in sorted(wanted.items()):
        matches = [e for e in entries if '/lookinfo/' in e['path'].replace('\\','/').lower()
                   and e['path'].replace('\\','/').split('/')[-1].lower() == key+'.loa']
        if len(matches) != 1:
            lookinfos.append(dict(lookInfo=label, status='MISSING_OR_AMBIGUOUS_LOOKINFO', candidates=matches)); continue
        e = matches[0]
        with (args.game_root/e['archive']).open('rb') as stream:
            stream.seek(e['offset'])
            payload = lpk.extract(stream.read(e['padded']), dict(e, offset=0),
                                  lpk.REGIONS['KR'][0].encode('latin1'), bytes.fromhex(lpk.REGIONS['KR'][1]))
        target = args.out/'lookinfo'/(label+'.loa')
        target.parent.mkdir(exist_ok=True)
        target.write_bytes(payload)
        lookinfos.append(dict(lookInfo=label, source=e, sha256=hashlib.sha256(payload).hexdigest(),
                              status='EXTRACTED_EXACT_REFERENCE', strings=strings(payload),
                              objectReferences=sorted(set(m.decode('ascii') for m in re.findall(rb"[A-Za-z][A-Za-z0-9_]*'[^'\x00]+'",payload)))))
    summary = dict(deployPlacements=len(deploy), deployNpcIds=len({r['npcId'] for r in deploy}),
                   deployResolvedIds=len({r['npcId'] for r in deploy if r['definition']}),
                   sceneActors=len(actors), sceneLookInfos=len({r['lookInfo'] for r in actors}),
                   actorsWithTimeline=sum(bool(r['timelines']) for r in actors),
                   actorsWithBodyMaterialVariations=sum(bool(value(
                       value(r['actorProperties'], 'defaultmeshmaterialinfo', {}).get('properties', {}),
                       'materialvariations', [])) for r in actors),
                   actorsWithSocketAttachments=sum(bool(r['attachments']) for r in actors),
                   extractedLookInfos=sum(r['status']=='EXTRACTED_EXACT_REFERENCE' for r in lookinfos),
                   guideIdentitiesConfirmed=0, runtimeChanged=False)
    report = dict(summary=summary, npcDatabase=dict(path=str(dbpath),sha256=digest(dbpath)),
                  deploySource=dict(path=str(deploypath),sha256=digest(deploypath)),
                  archiveIndex=dict(path=str(args.archive_index),sha256=digest(args.archive_index),
                                    status='EXISTING_INDEX_PAYLOAD_EXTRACTION_NOT_FRESH_FULL_INDEX_SCAN'),
                  deploy=deploy, scenes=scenes, lookinfos=lookinfos,
                  admission='Evidence only. Never infer numeric NPC identity/guide role from proximity or appearance.')
    (args.out/'source-npc-audit.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(summary,ensure_ascii=False))


if __name__ == '__main__':
    main()

"""Build a source-backed Kouku V1 organization input, without publishing assets.

The output distinguishes direct Action particle calls, serialized Matinee
activations, and transitive SkillEffect/Projectile/NPC references. Reachability
is not a claim that every branch executes in a single encounter. Existing
Composition gate assignments are authoritative for those authored uses; other
gate labels are explicitly marked authoring suggestions, never source rules.
"""
from __future__ import annotations

import argparse
import base64
import collections
import csv
import hashlib
import json
import mmap
import re
import sqlite3
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SOURCE = Path('C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829')
sys.path[:0] = [str(ROOT / 'Tools/LpkPipeline'), str(ROOT / 'Tools/LevelPlacementExtractor')]
import unpack_lpk as lpk
from extract_action_effect_notifies import (extract_action_document,
    object_references, scan_length_prefixed_strings)

PROFILES = ('MN_RPCZ_00', 'MN_RPCT_05', 'MN_RPCT_06', 'MN_RPCT_07')
ACTORS = {'MN_RPCZ_00': '쿠크', 'MN_RPCT_05': '세이튼',
          'MN_RPCT_06': '대형 세이튼', 'MN_RPCT_07': '쿠크세이튼'}
SUGGESTED_GATES = {'MN_RPCZ_00': 'GATE2', 'MN_RPCT_05': 'GATE1',
                   'MN_RPCT_06': 'GATE2', 'MN_RPCT_07': 'GATE3'}
COMPOSITIONS = ('Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json',
                'Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json')
# Exact scene/Matinee identities established by the prior source cutscene audit.
# Other Matinees in the same package do not inherit this event identity.
KNOWN_SEQUENCES = {
    ('SCENE03A', 'efseqact_matinee_0'): ('GATE1', '입장 · 팝업북'),
    ('SCENE03A', 'efseqact_matinee_7'): ('GATE1', '입장 · 광장 폭죽'),
    ('SCENE04A', 'efseqact_matinee_2'): ('GATE2', '입장 · 도박 테이블'),
    ('SCENE02A', 'efseqact_matinee_10'): ('GATE3', '입장 · 서커스'),
    ('SCENE07A', 'efseqact_matinee_23'): ('GATE3', '앵콜 · 가짜 클리어'),
    ('SCENE01B', 'efseqact_matinee_0'): ('GATE3', '마지막 연출'),
    ('SCENE01C', 'efseqact_matinee_0'): ('GATE3', '마지막 연출 · 공간 변형'),
}


def read(path):
    return json.loads(Path(path).read_text(encoding='utf-8-sig'))


def write(path, value):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2,
                               allow_nan=False) + '\n', encoding='utf-8')


def stable(prefix, *parts):
    return prefix + '.' + hashlib.sha256('|'.join(map(str, parts)).encode()).hexdigest()[:16]


def particle_paths(references):
    return sorted({r['objectPath'].lower() for r in references
                   if r.get('className', '').lower() == 'particlesystem'})


def effect_payload_id(payload):
    raw = base64.b64decode(payload.get('data', ''))
    if not raw.startswith(b'CEFActionNotify_Effect\0') or len(raw) < 71:
        raise ValueError('Effect payload signature/length unavailable')
    length = struct.unpack_from('<i', raw, 67)[0]
    width = -length * 2 if length < 0 else length
    field = 71 + width + 12
    if not 0 < width <= 4096 or field + 4 > len(raw):
        raise ValueError('Effect source label length unavailable')
    return struct.unpack_from('<I', raw, field)[0], field


def composition_uses():
    uses = collections.defaultdict(list)
    summaries = []
    for relative in COMPOSITIONS:
        doc = read(ROOT / relative)
        summaries.append(dict(path=relative, revision=doc['revision'],
                              patternCount=len(doc['patterns'])))
        for pattern in doc['patterns']:
            stage_pairs = collections.defaultdict(set)
            for stage in pattern['stages']:
                for clip in stage.get('animationOccurrences', []):
                    profile, action = clip.get('profileId'), clip.get('sourceActionId')
                    if profile and action is not None:
                        stage_pairs[profile, action].add(clip.get('sourceStageId', ''))
            for key, stages in stage_pairs.items():
                uses[key].append(dict(compositionPath=relative,
                    compositionId=doc['compositionId'], patternId=pattern['patternId'],
                    displayName=pattern['displayName'], gateId=pattern['gateId'],
                    actorProfileId=pattern['actorProfileId'],
                    authoringStatus=pattern['authoringStatus'], sourceStageIds=sorted(stages)))
    return uses, summaries


def action_inventory(source, uses):
    actions, direct, effect_links, unresolved = [], [], [], []
    csv_path = ROOT / '.md/GB/09-11/2026-09-11_KOUKU_SOURCE_PATTERN_EFFECT_INVENTORY_ACTIONS.csv'
    with csv_path.open(encoding='utf-8-sig', newline='') as stream:
        prior = {(r['profileId'], int(r['actionId'])): r for r in csv.DictReader(stream)}
    for profile in PROFILES:
        path = source / 'RemainingCharacterExtraction-20260829/ActionNameSources' / (profile + '.action-effects.json')
        reference = read(ROOT / 'Data/Animation/Reference/KoukuSaydon' / (profile + '.actionreference.json'))
        reference_actions = {a['sourceActionId']: a for a in reference['actions']}
        for action in read(path)['actions']:
            key = profile, action['actionId']
            authored = uses.get(key, [])
            gates = sorted({u['gateId'] for u in authored})
            source_name = action['displayName']
            previous = prior.get(key, {})
            category = previous.get('analystCategory', 'UNCLASSIFIED')
            section = '연출' if category in ('SYSTEM', 'COMMON') else '패턴'
            gate = gates[0] if len(gates) == 1 else SUGGESTED_GATES[profile]
            row = dict(profileId=profile, actionId=action['actionId'], displayName=source_name,
                sourceActionPath=str(path), sourceActionOffset=action['sourceOffset'],
                currentCompositionUses=authored, authoringGateIds=gates,
                suggestedGateId=gate,
                classificationBasis='CURRENT_COMPOSITION' if len(gates) == 1 else 'AUTHORING_SUGGESTION_PROFILE_FAMILY',
                actorDisplayName=ACTORS[profile], analystCategory=category,
                categoryPath=['KoukuSaydon', gate[-1] + '관문', section, ACTORS[profile], source_name],
                stages=[])
            refstages = {s['stageOrdinal']: s for s in reference_actions.get(action['actionId'], {}).get('stages', [])}
            for stage in action['stages']:
                stage_row = dict(stageIndex=stage['stageIndex'], stageName=stage.get('stageName', ''),
                    sourceOffset=stage['sourceOffset'],
                    runtimeAnimations=refstages.get(stage['stageIndex'], {}).get('slots', []),
                    sourceClips=stage.get('animationClips', []), sourceNotifies=[], particleSystems=[])
                for notify in stage['notifies']:
                    common = dict(profileId=profile, actionId=action['actionId'], stageIndex=stage['stageIndex'],
                        notifyId=notify['notifyId'], sourceType=notify['sourceType'],
                        sourceOffset=notify['sourceOffset'], startSeconds=notify.get('localTimeSeconds', 0),
                        durationSeconds=notify.get('durationSeconds', 0))
                    systems = particle_paths(notify.get('assetReferences', []))
                    if systems:
                        raw = base64.b64decode(notify['serializedPayload'].get('data', ''))
                        enabled = None
                        if notify['sourceType'] == 'PlayParticleEffect' and raw.startswith(b'CEFActionNotify_PlayParticleEffect\0') and len(raw) > 47 and raw[47] in (0, 1):
                            enabled = bool(raw[47])
                        occurrence = dict(**common, particleSystems=systems, enabled=enabled,
                            sourcePayloadSha256=notify['serializedPayload'].get('sha256'),
                            serializedLabels=notify.get('serializedLabels', []))
                        stage_row['sourceNotifies'].append(occurrence)
                        for system in systems:
                            direct.append(dict(**common, sourceSystem=system, enabled=enabled))
                        stage_row['particleSystems'].extend(systems)
                    if notify['sourceType'] == 'Effect':
                        try:
                            effect_id, field = effect_payload_id(notify['serializedPayload'])
                            effect_links.append(dict(**common, skillEffectId=effect_id,
                                sourceFieldOffset=field, basis='CEFActionNotify_Effect FString at +67, then three u32 fields, then Effect ID'))
                        except ValueError as error:
                            unresolved.append(dict(**common, reason=str(error)))
                stage_row['particleSystems'] = sorted(set(stage_row['particleSystems']))
                row['stages'].append(stage_row)
            actions.append(row)
    return actions, direct, effect_links, unresolved


class ArchiveReader:
    def __init__(self, path, output):
        self.path, self.output = path, output
        self.stream = path.open('rb')
        self.packed = mmap.mmap(self.stream.fileno(), 0, access=mmap.ACCESS_READ)
        self.key = lpk.REGIONS['KR'][0].encode('latin1')
        self.base = bytes.fromhex(lpk.REGIONS['KR'][1])
        self.entries = {e['path'].replace('\\', '/').lower(): e for e in lpk.read_index(self.packed, self.key)}
        self.receipts = []

    def acquire(self, domain, name):
        suffix = '/' + domain.lower() + '/' + name.lower()
        matches = [v for k, v in self.entries.items() if k.endswith(suffix)]
        if not matches:
            return None
        if len(matches) != 1:
            raise ValueError('Ambiguous original archive entry: ' + suffix)
        entry = matches[0]
        raw = lpk.extract(self.packed, entry, self.key, self.base)
        path = self.output / domain / name
        path.parent.mkdir(parents=True, exist_ok=True)
        if not path.exists() or path.read_bytes() != raw:
            path.write_bytes(raw)
        self.receipts.append(dict(sourceArchive=str(self.path), entry=entry,
            output=str(path), sha256=hashlib.sha256(raw).hexdigest()))
        return path

    def close(self):
        self.packed.close()
        self.stream.close()


def indirect_inventory(source, archive, action_archive, effect_links):
    db = source / 'WorldObjectExtraction-20260907/EFTable_SkillEffect.db'
    npcdb = source / 'WorldObjectExtraction-20260907/EFTable_Npc.db'
    buffdb = ROOT / 'out/KoukuShowtimeInventory20260911/source/EFTable_SkillBuff.db'
    rows = collections.defaultdict(list)
    with sqlite3.connect(db.as_uri() + '?mode=ro', uri=True) as conn:
        conn.row_factory = sqlite3.Row
        for row in conn.execute('SELECT * FROM SkillEffect'):
            rows[int(row['PrimaryKey'])].append(dict(row))
    npcrows = {}
    with sqlite3.connect(npcdb.as_uri() + '?mode=ro', uri=True) as conn:
        conn.row_factory = sqlite3.Row
        for row in conn.execute('SELECT * FROM Npc'):
            npcrows[int(row['PrimaryKey'])] = dict(row)
    buffrows = collections.defaultdict(list)
    with sqlite3.connect(buffdb.as_uri() + '?mode=ro', uri=True) as conn:
        conn.row_factory = sqlite3.Row
        for row in conn.execute('SELECT * FROM SkillBuff'):
            buffrows[int(row['PrimaryKey'])].append(dict(row))
    owners = collections.defaultdict(set)
    for link in effect_links:
        owners[link['skillEffectId']].add((link['profileId'], link['actionId']))
    edges, missing, projectiles, npcs, npc_actions, buffs = [], [], {}, {}, {}, {}
    excluded_buff_candidates = []
    queue = collections.deque(sorted(owners))
    visited = set()
    marker = b'CEFSequenceSummonsActionSkillEffect\0'
    while queue:
        effect_id = queue.popleft()
        if effect_id in visited:
            continue
        visited.add(effect_id)
        if effect_id not in rows:
            missing.append(dict(kind='SkillEffect', id=effect_id))
            continue
        for row in rows[effect_id]:
            for suffix in ('', '1'):
                chain, chain_type = int(row['ChainIndex' + suffix]), int(row['ChainType' + suffix])
                if chain and chain_type:
                    edges.append(dict(kind='SkillEffectChain', sourceId=effect_id, targetId=chain,
                        rawType=chain_type, field='ChainIndex' + suffix))
                    queue.append(chain)
            kind, target = int(row['Key']), int(row['ValueA'])
            if kind in (12, 33):
                edges.append(dict(kind='SkillEffectToProjectile', sourceId=effect_id, targetId=target, rawKey=kind))
                if target in projectiles:
                    continue
                path = archive.acquire('Projectile', str(target) + '.loa')
                if path is None:
                    missing.append(dict(kind='Projectile', id=target, skillEffectId=effect_id))
                    projectiles[target] = dict(projectileId=target, particleSystems=[], unavailable=True)
                    continue
                raw = path.read_bytes()
                strings = scan_length_prefixed_strings(raw, 0, len(raw))
                classes = [s['value'] for s in strings if s['value'].startswith('CEFSequenceSummonsProjectile')]
                projectiles[target] = dict(projectileId=target, sourcePath=str(path),
                    sourceClass=classes[0] if classes else '',
                    particleSystems=particle_paths(object_references(raw)),
                    sourceSha256=hashlib.sha256(raw).hexdigest(),
                    particleTokens=[dict(sourceOffset=s['sourceOffset'], value=s['value'])
                        for s in strings if s['value'].startswith("ParticleSystem'")])
                cursor = 0
                while True:
                    at = raw.find(marker, cursor)
                    if at < 0:
                        break
                    field = at + len(marker) + 32
                    if field + 4 <= len(raw):
                        callback = struct.unpack_from('<I', raw, field)[0]
                        edges.append(dict(kind='ProjectileSkillEffectCallback', sourceId=target,
                            targetId=callback, sourceFieldOffset=field, databaseMatch=callback in rows))
                        if callback in rows:
                            queue.append(callback)
                        else:
                            missing.append(dict(kind='ProjectileSkillEffectCallback', id=callback, projectileId=target))
                    cursor = at + len(marker)
            elif kind == 15:
                edges.append(dict(kind='SkillEffectToNpc', sourceId=effect_id, targetId=target))
                if target in npcs:
                    continue
                npc = npcrows.get(target)
                if npc is None:
                    missing.append(dict(kind='Npc', id=target, skillEffectId=effect_id))
                    continue
                fields = ('PrimaryKey', 'Desc', 'Comment1', 'Model', 'AiIndex',
                    'OriginalActionObjectGroupName', 'SummonSpawnAction', 'SummonSpawnSkillId',
                    'DieSkillIndex', 'DestroySkillIndex')
                npcs[target] = {k: npc[k] for k in fields}
                group = npc['OriginalActionObjectGroupName']
                action_ids = {int(npc[k]) for k in ('SummonSpawnSkillId', 'DieSkillIndex', 'DestroySkillIndex') if npc[k]}
                for action_id in sorted(action_ids):
                    edges.append(dict(kind='NpcExplicitAction', sourceId=target, targetId=action_id, profileId=group))
                    action_key = group, action_id
                    if action_key in npc_actions:
                        continue
                    path = action_archive.acquire('Action', group + '.loa')
                    if path is None:
                        missing.append(dict(kind='NpcActionDocument', npcId=target, profileId=group, actionId=action_id))
                        continue
                    document = extract_action_document(path, group, action_ids={action_id})
                    particle_notifies = []
                    for action in document['actions']:
                        for stage in action['stages']:
                            for notify in stage['notifies']:
                                for ps in particle_paths(notify.get('assetReferences', [])):
                                    particle_notifies.append(dict(stageIndex=stage['stageIndex'], notifyId=notify['notifyId'], sourceSystem=ps))
                                if notify['sourceType'] == 'Effect':
                                    try:
                                        callback, field = effect_payload_id(notify['serializedPayload'])
                                        edges.append(dict(kind='NpcActionSkillEffect', sourceId=action_id, targetId=callback,
                                            profileId=group, notifyId=notify['notifyId'], sourceFieldOffset=field))
                                        queue.append(callback)
                                    except ValueError as error:
                                        missing.append(dict(kind='NpcActionEffectPayload', profileId=group,
                                            actionId=action_id, notifyId=notify['notifyId'], reason=str(error)))
                    npc_actions[action_key] = dict(profileId=group, actionId=action_id,
                        sourcePath=str(path), particleNotifies=particle_notifies)
            elif kind == 7:
                edges.append(dict(kind='SkillEffectToBuff', sourceId=effect_id, targetId=target))
                if target in buffs:
                    continue
                selected = buffrows.get(target, [])
                buff = dict(buffId=target, rows=[], presentationSources=[], particleSystems=[])
                buffs[target] = buff
                if not selected:
                    missing.append(dict(kind='SkillBuff', id=target))
                for original in selected:
                    columns = ('PrimaryKey', 'SecondaryKey', 'Archetype', 'Name', 'Key', 'ValueA',
                        'ApplySkillEffectId', 'ApplySkillEffectId1', 'OverlapSkillEffectId',
                        'ExpiredChainSkillEffectId0', 'ExpiredChainSkillEffectId1',
                        'ExpiredChainSkillEffectId2', 'ExpiredChainSkillEffectId3')
                    buff['rows'].append({k: original[k] for k in columns})
                    for field in columns:
                        if 'SkillEffectId' in field and original[field]:
                            callback = int(original[field])
                            edges.append(dict(kind='BuffSkillEffectCallback', sourceId=target,
                                targetId=callback, sourceField=field))
                            queue.append(callback)
                    archetype = original['Archetype']
                    if not archetype or archetype.lower() == 'none':
                        continue
                    names = [Path(entry['path'].replace('\\', '/')).name
                        for entry in action_archive.entries.values()
                        if '/particlesoundnew/9_ef_particle_sound_data_buff_' in entry['path'].replace('\\', '/').lower()
                        and entry['path'].lower().endswith('_' + archetype.lower() + '.loa')]
                    for name in names:
                        path = action_archive.acquire('ParticleSoundNew', name)
                        raw = path.read_bytes()
                        strings = scan_length_prefixed_strings(raw, 0, len(raw))
                        if not any(s['value'] == 'CEFParticleSoundDataBuffFX' for s in strings) or not any(s['value'].casefold() == archetype.casefold() for s in strings):
                            excluded_buff_candidates.append(dict(buffId=target, sourcePath=str(path),
                                reason='Filename suffix matches but serialized BuffFX identity differs'))
                            continue
                        ps = particle_paths(object_references(raw))
                        buff['presentationSources'].append(dict(sourcePath=str(path), archetype=archetype,
                            particleSystems=ps, sha256=hashlib.sha256(raw).hexdigest(),
                            basis='Exact SkillBuff Archetype and CEFParticleSoundDataBuffFX serialized identity'))
                        buff['particleSystems'].extend(ps)
                buff['particleSystems'] = sorted(set(buff['particleSystems']))
    # Propagate origin action identity to closure nodes, including cycles. No
    # callback is made unconditional; this is a set of possible source owners.
    origins = collections.defaultdict(set)
    for key, values in owners.items():
        origins['effect', key].update(values)
    for _ in range(len(edges) + 1):
        changed = False
        for edge in edges:
            kind = edge['kind']
            src = 'projectile' if kind == 'ProjectileSkillEffectCallback' else 'npc' if kind == 'NpcExplicitAction' else 'action' if kind == 'NpcActionSkillEffect' else 'buff' if kind == 'BuffSkillEffectCallback' else 'effect'
            dst = 'projectile' if kind == 'SkillEffectToProjectile' else 'npc' if kind == 'SkillEffectToNpc' else 'action' if kind == 'NpcExplicitAction' else 'buff' if kind == 'SkillEffectToBuff' else 'effect'
            before = len(origins[dst, edge['targetId']])
            origins[dst, edge['targetId']].update(origins[src, edge['sourceId']])
            changed |= len(origins[dst, edge['targetId']]) != before
        if not changed:
            break
    for projectile in projectiles.values():
        projectile['originActions'] = [dict(profileId=p, actionId=a) for p, a in sorted(origins['projectile', projectile['projectileId']])]
    for action in npc_actions.values():
        action['originActions'] = [dict(profileId=p, actionId=a) for p, a in sorted(origins['action', action['actionId']])]
    for buff in buffs.values():
        buff['originActions'] = [dict(profileId=p, actionId=a) for p, a in sorted(origins['buff', buff['buffId']])]
    return dict(skillEffectDb=str(db), npcDb=str(npcdb), buffDb=str(buffdb),
        directEffectNotifyLinks=effect_links, visitedSkillEffectIds=sorted(visited),
        edges=edges, projectiles=list(projectiles.values()), npcs=list(npcs.values()),
        npcActions=list(npc_actions.values()), buffs=list(buffs.values()), unresolvedReferences=missing,
        excludedBuffPresentationCandidates=excluded_buff_candidates,
        interpretation='Exact serialized reachability. SkillEffect chain conditions, projectile callback timing, NPC AI action choice and target geometry are not inferred.')


def scene_inventory(cache):
    occurrences, excluded, unresolved = [], [], []
    matinee_count = 0
    for path in sorted(cache.glob('LV_LUT_MIDNIGHTC_ED_SCENE*.json')):
        doc = read(path)
        rows, imports = doc['rows'], doc['imports']
        for mid, matinee in rows.items():
            if matinee['cls'].lower() != 'efseqact_matinee':
                continue
            matinee_count += 1
            mname = matinee['name'].split('.')[-1]
            scene = path.stem.rsplit('_', 1)[-1]
            known = KNOWN_SEQUENCES.get((scene, mname))
            classification = dict(gateId=known[0] if known else '',
                displayName=known[1] if known else scene + ' / ' + mname,
                basis='EXACT_PRIOR_SOURCE_CUTSCENE_AUDIT' if known else 'SOURCE_SEQUENCE_IDENTITY_ONLY')
            links = {v.get('linkdesc', '').lower(): v.get('linkedvariables', [])
                     for v in matinee['p'].get('variablelinks', [])}
            for did in links.get('data', []):
                data = rows.get(str(did), {})
                length = data.get('p', {}).get('interplength')
                for gid in data.get('p', {}).get('interpgroups', []):
                    group = rows.get(str(gid), {})
                    gp = group.get('p', {})
                    name = gp.get('groupname', '')
                    actors = [rows.get(str(v), {}).get('p', {}).get('objvalue') for v in links.get(name.lower(), [])]
                    for actor_id in actors:
                        actor = rows.get(str(actor_id), {})
                        component_id = actor.get('p', {}).get('particlesystemcomponent')
                        component = rows.get(str(component_id), {})
                        template = component.get('p', {}).get('template')
                        system = imports.get(str(template), '')
                        if not system:
                            continue
                        base = dict(sourceScene=path.stem, sourceCache=str(path),
                            matineeExport=int(mid), matineeId=mname, dataExport=did,
                            groupExport=gid, groupName=name, actorExport=actor_id,
                            actorName=actor.get('name'), componentExport=component_id,
                            sourceSystem=system.lower(), classification=classification,
                            sourceDurationSeconds=length, actorProperties=actor.get('p', {}),
                            componentProperties=component.get('p', {}))
                        toggles = []
                        for tid in gp.get('interptracks', []):
                            track = rows.get(str(tid), {})
                            if track.get('cls', '').lower() != 'interptracktoggle':
                                continue
                            tp = track.get('p', {})
                            events = tp.get('toggletrack', [])
                            record = dict(**base, trackExport=tid, trackName=track.get('name'),
                                toggleKeys=events, disabled=bool(tp.get('bdisabletrack', False)),
                                activationIntervals=[], liveReference=False)
                            if record['disabled']:
                                record['exclusionReason'] = 'DISABLED_TRACK'
                                excluded.append(record)
                                continue
                            start = None
                            for event in events:
                                action, time = event.get('toggleaction'), event.get('time')
                                if action in ('etta_on', 'etta_trigger'):
                                    if start is not None:
                                        record['activationIntervals'].append(dict(startSeconds=start, stopSeconds=time, stopBasis='NEXT_ACTIVATION'))
                                    start = time
                                elif action == 'etta_off' and start is not None:
                                    record['activationIntervals'].append(dict(startSeconds=start, stopSeconds=time, stopBasis='EXPLICIT_OFF'))
                                    start = None
                                elif action not in ('etta_off',):
                                    unresolved.append(dict(**base, reason='Unknown toggle action', toggleAction=action))
                            if start is not None:
                                record['activationIntervals'].append(dict(startSeconds=start, stopSeconds=length,
                                    stopBasis='MATINEE_END_PREVIEW_BOUND' if length is not None else 'UNBOUNDED_SOURCE'))
                            record['liveReference'] = bool(record['activationIntervals'])
                            record['sourceOccurrenceId'] = stable('source.sequence', path.stem, mid, gid, actor_id, tid)
                            (occurrences if record['liveReference'] else excluded).append(record)
                            toggles.append(tid)
                        if not toggles:
                            excluded.append(dict(**base, exclusionReason='NO_ACTIVE_TOGGLE_REFERENCE', liveReference=False))
    return dict(sceneCount=len(list(cache.glob('LV_LUT_MIDNIGHTC_ED_SCENE*.json'))),
        matineeCount=matinee_count, occurrences=occurrences, excludedReferences=excluded,
        unresolvedReferences=unresolved,
        interpretation='Active serialized Matinee ON/trigger links only. OFF-only, disabled and unbound emitters are excluded from active counts. External Kismet reachability and event ordering remain separate.')


def current_authored_sources():
    result = collections.defaultdict(set)
    for path in (ROOT / 'Data/Effects/Authored').glob('effect.kouku.*.effect.json'):
        doc = read(path)
        for element in doc.get('elements', []):
            source = element.get('sourcePresentation', {})
            for field in ('sourceParticleSystem', 'sourceParticleSystemPath'):
                if source.get(field):
                    result[source[field].lower()].add(doc['effectAssetId'])
            # Older source imports carry the original PS as sourceSystemPath.
            for field in ('sourceSystemPath', 'sourceSystem'):
                if source.get(field):
                    result[source[field].lower()].add(doc['effectAssetId'])
            emitter = source.get('sourceObjectPath', '').lower()
            if re.search(r'\.(particlespriteemitter|particleemitter)_\d+$', emitter):
                result[emitter.rsplit('.', 1)[0]].add(doc['effectAssetId'])
    return result


def build(args):
    uses, compositions = composition_uses()
    actions, direct, effect_links, unresolved = action_inventory(args.source, uses)
    archive = ArchiveReader(args.archive, args.output.parent / 'source')
    action_archive = ArchiveReader(args.action_archive, args.output.parent / 'source')
    try:
        indirect = indirect_inventory(args.source, archive, action_archive, effect_links)
        receipts = archive.receipts + action_archive.receipts
    finally:
        archive.close()
        action_archive.close()
    sequences = scene_inventory(args.scene_cache)
    systems = collections.defaultdict(lambda: dict(directActionUses=[], indirectProjectileUses=[],
        indirectNpcActionUses=[], indirectBuffUses=[], sequenceUses=[], currentAuthoredEffectIds=[]))
    for occurrence in direct:
        systems[occurrence['sourceSystem']]['directActionUses'].append(occurrence)
    for projectile in indirect['projectiles']:
        for system in projectile['particleSystems']:
            systems[system]['indirectProjectileUses'].append(dict(projectileId=projectile['projectileId'],
                sourcePath=projectile['sourcePath'], originActions=projectile['originActions']))
    for action in indirect['npcActions']:
        for notify in action['particleNotifies']:
            systems[notify['sourceSystem']]['indirectNpcActionUses'].append(dict(profileId=action['profileId'],
                actionId=action['actionId'], stageIndex=notify['stageIndex'], notifyId=notify['notifyId'],
                originActions=action['originActions']))
    for buff in indirect['buffs']:
        for ps in buff['particleSystems']:
            systems[ps]['indirectBuffUses'].append(dict(buffId=buff['buffId'],
                archetypes=sorted({r['Archetype'] for r in buff['rows']}),
                originActions=buff['originActions'], presentationSources=buff['presentationSources']))
    for occurrence in sequences['occurrences']:
        systems[occurrence['sourceSystem']]['sequenceUses'].append({k: occurrence[k] for k in
            ('sourceOccurrenceId', 'sourceScene', 'matineeId', 'groupName', 'trackName', 'classification')})
    for system, assets in current_authored_sources().items():
        if system in systems:
            systems[system]['currentAuthoredEffectIds'] = sorted(assets)
    direct_set = {r['sourceSystem'] for r in direct}
    sequence_set = {r['sourceSystem'] for r in sequences['occurrences']}
    projectile_set = {s for p in indirect['projectiles'] for s in p['particleSystems']}
    npc_set = {r['sourceSystem'] for a in indirect['npcActions'] for r in a['particleNotifies']}
    buff_set = {s for b in indirect['buffs'] for s in b['particleSystems']}
    output = dict(schema='lostark.kouku-effect-source-organization', formatVersion=1,
        authority='AUTHORING_ORGANIZATION_AND_SOURCE_EVIDENCE', compositions=compositions,
        counts=dict(sourceActions=len(actions), sourceStages=sum(len(a['stages']) for a in actions),
            directParticleNotifyReferences=len(direct), uniqueDirectParticleSystems=len(direct_set),
            enabledDirectParticleSystems=len({r['sourceSystem'] for r in direct if r['enabled'] is True}),
            reachableSkillEffectIds=len(indirect['visitedSkillEffectIds']),
            reachableProjectiles=len(indirect['projectiles']), indirectProjectileParticleSystems=len(projectile_set),
            indirectNpcActionParticleSystems=len(npc_set), liveSequenceParticleSystems=len(sequence_set),
            reachableBuffs=len(indirect['buffs']), indirectBuffParticleSystems=len(buff_set),
            sequenceParticleOccurrences=len(sequences['occurrences']), unionParticleSystems=len(systems),
            notInDirectActionParticleSystems=len((sequence_set | projectile_set | npc_set | buff_set) - direct_set),
            sourceSystemsWithoutCurrentAuthoredDocument=sum(not s['currentAuthoredEffectIds'] for s in systems.values()),
            unresolvedIndirectReferences=len(indirect['unresolvedReferences'])),
        scope='Four extracted boss Action profiles plus exact transitive table/Projectile/NPC references and active particle Toggle links in nine scene packages. Package-only candidates are not counted as live effects.',
        actions=actions, particleSystems=[dict(sourceSystem=s, **v) for s, v in sorted(systems.items())],
        sequences=sequences, indirectGraph=indirect, unresolvedActionNotifies=unresolved,
        sourceAcquisition=receipts)
    write(args.output, output)
    print(json.dumps(output['counts'], ensure_ascii=False))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=SOURCE)
    parser.add_argument('--archive', type=Path, default=Path('C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/data1.lpk'))
    parser.add_argument('--action-archive', type=Path, default=Path('C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/data3.lpk'))
    parser.add_argument('--scene-cache', type=Path, default=ROOT / 'out/KoukuFireworks20260911')
    parser.add_argument('--output', type=Path, default=ROOT / 'out/KoukuAllEffects20260912/organization.json')
    build(parser.parse_args())

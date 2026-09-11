"""Decode the bounded original 4219916 stage 002 fire-grid source chain."""
import base64
import hashlib
import math
import mmap
import sqlite3
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'Tools/LpkPipeline'), str(ROOT / 'Tools/LevelPlacementExtractor')]
import unpack_lpk as lpk
from extract_action_effect_notifies import scan_length_prefixed_strings


def decode(source, evidence, read, write,
           archive=Path('C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/data1.lpk')):
    action_path = source / 'RemainingCharacterExtraction-20260829/ActionNameSources/MN_RPCT_07.action-effects.json'
    database = source / 'WorldObjectExtraction-20260907/EFTable_SkillEffect.db'
    action = next(a for a in read(action_path)['actions'] if a['actionId'] == 4219916)
    stage = next(s for s in action['stages'] if s['stageIndex'] == 2)
    selected = {str(i) for i in range(421991401, 421991406)}
    projectiles = {}
    raw_output = evidence / 'projectile_source'
    raw_output.mkdir(parents=True, exist_ok=True)
    with archive.open('rb') as stream, mmap.mmap(stream.fileno(), 0, access=mmap.ACCESS_READ) as packed:
        key = lpk.REGIONS['KR'][0].encode('latin1')
        base = bytes.fromhex(lpk.REGIONS['KR'][1])
        for entry in lpk.read_index(packed, key):
            name = Path(entry['path'].replace('\\', '/')).stem
            if '\\projectile\\' not in entry['path'].lower() or name not in selected:
                continue
            assert name not in projectiles
            raw = lpk.extract(packed, entry, key, base)
            (raw_output / (name + '.loa')).write_bytes(raw)
            strings = scan_length_prefixed_strings(raw, 0, len(raw))
            assert any(s['value'] == 'CEFSequenceSummonsProjectileFixArea' for s in strings)
            timers = [s['sourceOffset'] for s in strings if s['value'] == 'CEFSequenceSummonsActionTimer']
            assert len(timers) == 2
            time_offsets = [timers[1] - 16, len(raw) - 120]
            times = [struct.unpack_from('<f', raw, at)[0] for at in time_offsets]
            assert math.isclose(times[0], .3, abs_tol=1e-6)
            assert math.isclose(times[1], 2 - (int(name) - 421991401) * .2, abs_tol=1e-6)
            for at in time_offsets:
                assert struct.unpack_from('<5i', raw, at - 20) == (1, 1, 1, 1, 0)
                assert struct.unpack_from('<3i', raw, at + 4) == (1, 0, 0)
            particles = []
            for token in strings:
                if not token['value'].startswith("ParticleSystem'"):
                    continue
                at = token['sourceOffset']
                end = at + 4 + len(token['value']) + 1
                position = list(struct.unpack_from('<3f', raw, end + 76))
                rotation = list(struct.unpack_from('<3i', raw, end + 100))
                scale = list(struct.unpack_from('<3f', raw, end + 136))
                assert position == [0, 0, 0] and scale == [1, 1, 1]
                assert rotation[0] == rotation[2] == 0 and rotation[1] in (-16384, 0, 16384)
                cursor = end + 148
                count = struct.unpack_from('<i', raw, cursor)[0]
                cursor += 4
                assert 0 <= count <= 6
                parameters = []
                for _ in range(count):
                    length = struct.unpack_from('<i', raw, cursor)[0]
                    label = raw[cursor + 4:cursor + 3 + length].decode('ascii')
                    cursor += 4 + length
                    kind = struct.unpack_from('<i', raw, cursor)[0]
                    assert kind in (0, 1, 3)
                    value = struct.unpack_from('<f', raw, cursor + 4)[0] if kind == 1 else list(struct.unpack_from('<3f', raw, cursor + 12)) if kind == 3 else None
                    sentinel = cursor + 56
                    length = struct.unpack_from('<i', raw, sentinel)[0]
                    assert raw[sentinel + 4:sentinel + 4 + length] == b'None\0'
                    parameters.append(dict(name=label, sourceTypeCode=kind, value=value, sourceByteOffset=cursor))
                    cursor = sentinel + 4 + length + 4
                particles.append(dict(sourceParticleSystem=token['value'].split("'")[1].lower(),
                    sourceByteOffset=at, timerSeconds=times[at >= timers[1]], sourcePositionCm=position,
                    sourceRotator=rotation, sourceRotationDegrees=[v * 360 / 65536 for v in rotation],
                    sourceScale=scale, parameterOverrides=parameters))
            assert len(particles) == (5 if name == '421991401' else 4)
            lifetime = struct.unpack_from('<f', raw, 1014)[0]
            assert lifetime == 4
            projectiles[name] = dict(sourceArchive=str(archive), sourceArchiveEntry=entry['path'],
                sha256=hashlib.sha256(raw).hexdigest(), sourceLifetimeSeconds=lifetime,
                sourceClass='CEFSequenceSummonsProjectileFixArea', timerByteOffsets=time_offsets,
                timerSeconds=times, particles=particles)
    assert set(projectiles) == selected
    occurrences = []
    with sqlite3.connect(database.as_uri() + '?mode=ro', uri=True) as connection:
        connection.row_factory = sqlite3.Row
        for notify in stage['notifies']:
            if notify['sourceType'] != 'Effect':
                continue
            payload = base64.b64decode(notify['serializedPayload']['data'])
            key = struct.unpack_from('<i', payload, 90)[0]
            if not 421991412 <= key <= 421991421:
                continue
            rows = connection.execute('select PrimaryKey,SecondaryKey,Key,ValueA,AreaOffsetX,AreaOffsetY,AreaOffsetZ,AreaOffsetAngle from SkillEffect where PrimaryKey=?', (key,)).fetchall()
            assert len(rows) == 1
            row = dict(rows[0])
            assert row['Key'] == 12 and row['SecondaryKey'] == 1 and str(row['ValueA']) in projectiles
            occurrences.append(dict(notifyId=notify['notifyId'], notifySeconds=notify['localTimeSeconds'],
                sourceSkillEffect=row, projectileId=str(row['ValueA'])))
    assert len(occurrences) == 10
    assert {r['sourceSkillEffect']['AreaOffsetAngle'] for r in occurrences} == {45, 135}
    result = dict(actionId=4219916, stageIndex=2, actionSource=str(action_path),
        skillEffectDb=str(database), projectiles=projectiles, occurrences=occurrences)
    write(evidence / 'decoded_grid.json', result)
    return result

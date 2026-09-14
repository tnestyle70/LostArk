"""Write vehicle skill rows from VehicleSkills.spec.json into the profile and catalog documents.

Server rows (Data/Vehicles/VehicleProfiles.json) carry slot, cooldown, action length and the root
motion baked from the vehicle AnimSet's b_root keys, in extract_rootmotion.py's axis convention
(forward = 0.01 * x, lateral = -0.01 * y, up = 0.01 * z). Client rows (Data/Actors/VehicleCatalog.json)
carry the vehicle clip chain and each class's rider chain; every name is checked against the cooked
vehicle model and the class's cooked riding animset before anything is written.

usage:
  python build_vehicle_skills.py --psk-root <umodel -groups export root> [--dry-run]
"""
from __future__ import annotations

import argparse
import json
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SPEC = ROOT / 'Tools/VehiclePipeline/VehicleSkills.spec.json'
PROFILES = ROOT / 'Data/Vehicles/VehicleProfiles.json'
CATALOG = ROOT / 'Data/Actors/VehicleCatalog.json'
RESOURCES = ROOT / 'Client/Bin/Resources'
WMODEL_SECTION_NAME_CAP = 39
MIN_ROOT_MOTION_METRES = 0.01
RIDER_CLASSES = (
    ('LANCE_MASTER', 'LanceMaster', 'flm_'),
    ('WARLORD', 'Warlord', 'wgl_'),
    ('ARTIST', 'Artist', 'sdm_'),
    ('DIMENSIONMASTER', 'DimensionMaster', 'pc_sp_m_00_sk_'),
)
FILE_TOKEN = {'horse': 'Horse', 'raptor': 'Raptor', 'swing': 'Swing', 'hoverboard': 'Hoverboard',
              'heavywalker_bm9': 'HeavywalkerBm9', 'tube': 'Tube', 'dragon_2': 'Dragon2'}


def fail(message):
    raise SystemExit('build_vehicle_skills: ' + message)


def psa_chunks(data):
    table, offset = {}, 0
    while offset + 32 <= len(data):
        name = data[offset:offset + 20].split(b'\0')[0].decode()
        size, count = struct.unpack_from('<ii', data, offset + 24)
        table[name] = (offset + 32, size, count)
        offset += 32 + size * count
    return table


def load_psa(path):
    data = path.read_bytes()
    table = psa_chunks(data)
    at, size, count = table['BONENAMES']
    bones = [data[at + i * size:at + i * size + 64].split(b'\0')[0].decode().lower() for i in range(count)]
    at, size, count = table['ANIMINFO']
    sequences = {}
    for i in range(count):
        row = at + i * size
        name = data[row:row + 64].split(b'\0')[0].decode()
        total, _, _, _, _, _, rate, _, first, frames = struct.unpack_from('<iiiifffiii', data, row + 128)
        sequences[name] = (total, rate, first, frames)
    keys_at, key_size, _ = table['ANIMKEYS']
    return data, bones, sequences, keys_at, key_size


def bake_root_motion(psa, clips, duration_ms):
    data, bones, sequences, keys_at, key_size = psa
    if 'b_root' not in bones:
        fail('vehicle AnimSet has no b_root')
    root = bones.index('b_root')
    samples = []
    base_ms = 0
    offset = (0.0, 0.0, 0.0)
    for step in clips:
        if step['clip'] not in sequences:
            fail(f'AnimSet has no sequence {step["clip"]}')
        total, rate, first, frames = sequences[step['clip']]
        if rate <= 0 or frames < 1:
            fail(f'{step["clip"]} has no frames')
        origin = struct.unpack_from('<3f', data, keys_at + first * total * key_size + root * key_size)
        last = origin
        for frame in range(frames):
            local_ms = round(frame * 1000.0 / rate)
            if local_ms > step['ms']:
                break
            position = struct.unpack_from('<3f', data, keys_at + (first + frame) * total * key_size + root * key_size)
            last = position
            time_ms = base_ms + local_ms
            if time_ms > duration_ms or (samples and time_ms <= samples[-1]['timeMs']):
                continue
            samples.append({
                'timeMs': time_ms,
                'forward': round((offset[0] + position[0] - origin[0]) * 0.01, 4),
                'lateral': round(-(offset[1] + position[1] - origin[1]) * 0.01, 4),
                'up': round((offset[2] + position[2] - origin[2]) * 0.01, 4),
            })
        offset = tuple(offset[i] + last[i] - origin[i] for i in range(3))
        base_ms += step['ms']
    if len(samples) > 512:
        fail('root motion exceeds 512 samples')
    peak = max((abs(s['forward']) + abs(s['lateral']) for s in samples), default=0.0)
    for sample in samples:
        for axis in ('forward', 'lateral', 'up'):
            if sample[axis] == 0:
                sample[axis] = 0.0
    return samples if peak >= MIN_ROOT_MOTION_METRES and len(samples) >= 2 else []


def model_clips(path):
    data = path.read_bytes()
    names = set()
    for index in range(struct.unpack_from('<I', data, 20)[0]):
        row = 48 + index * 64
        if struct.unpack_from('<I', data, row)[0] == 4:
            names.add(data[row + 24:row + 64].split(b'\0')[0].decode())
    return names


def rider_clip_names(prefix, vehicle, available):
    """The converter claims truncated section names in sorted order and suffixes a collision "~N"."""
    mode = vehicle['ridingMode']
    tokens = {f'ride_{mode}_idle_normal_1', f'ride_{mode}_run_normal_1'}
    tokens.update(f'ride_{mode}_{step["clip"]}' for skill in vehicle['skills'] for step in skill['clips'])
    names, taken = {}, set()
    for token in sorted(tokens):
        full = prefix + token
        for candidate in [full[:WMODEL_SECTION_NAME_CAP]] + \
                [full[:WMODEL_SECTION_NAME_CAP - len(f'~{n}')] + f'~{n}' for n in range(1, 10)]:
            if candidate not in taken:
                names[token] = candidate
                taken.add(candidate)
                break
    missing = sorted(set(names.values()) - available)
    if missing:
        fail(f'rider clips missing from the cooked animset: {missing}')
    return names


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--psk-root', type=Path, required=True)
    parser.add_argument('--dry-run', action='store_true')
    args = parser.parse_args()

    spec = json.loads(SPEC.read_text(encoding='utf8'))
    profiles = json.loads(PROFILES.read_text(encoding='utf8'))
    catalog = json.loads(CATALOG.read_text(encoding='utf8'))
    profile_by_id = {row['vehicleId']: row for row in profiles['vehicles']}
    catalog_by_id = {row['vehicleId']: row for row in catalog['vehicles']}
    if set(profile_by_id) != {v['vehicleId'] for v in spec['vehicles']} or set(catalog_by_id) != set(profile_by_id):
        fail('spec, profiles and catalog must list the same vehicles')

    report = []
    for vehicle in spec['vehicles']:
        slots = [skill['inputSlot'] for skill in vehicle['skills']]
        if len(set(slots)) != len(slots) or not set(slots) <= {'SPACE', 'Q', 'W', 'E'}:
            fail(f'{vehicle["name"]} slots are invalid: {slots}')
        package = vehicle['animPackage']
        psa = load_psa(args.psk_root / package / 'ani' / f'{package.lower()}_ani.psa')
        cooked_vehicle = model_clips(RESOURCES / catalog_by_id[vehicle['vehicleId']]['modelAssetId'])
        rider_names = {cls: rider_clip_names(prefix, vehicle, model_clips(
                           RESOURCES / f'Character/{folder}/AnimSets/{folder}_Ride{FILE_TOKEN[vehicle["ridingMode"]]}AnimSet.wmodel'))
                       for cls, folder, prefix in RIDER_CLASSES}
        profile_skills, catalog_skills = [], []
        for skill in vehicle['skills']:
            duration = sum(step['ms'] for step in skill['clips'])
            samples = bake_root_motion(psa, skill['clips'], duration)
            profile_skills.append({
                'skillId': skill['skillId'],
                'inputSlot': skill['inputSlot'],
                'cooldownMs': skill['cooldownMs'],
                'actionDurationMs': duration,
                'source': {'vehicleColumn': skill['vehicleColumn'], 'skillTable': 'EFTable_Skill',
                           'cooldownColumn': 'Cooltime', 'action': vehicle['action']},
                'rootMotionSamples': samples,
            })
            vehicle_clips = []
            for step in skill['clips']:
                name = 'npc_' + step['clip']
                if name not in cooked_vehicle:
                    fail(f'{vehicle["name"]} model has no {name}')
                vehicle_clips.append(name)
            catalog_skills.append({
                'skillId': skill['skillId'],
                'inputSlot': skill['inputSlot'],
                'vehicleClips': vehicle_clips,
                'riders': [{'characterClass': cls,
                            'clips': [rider_names[cls][f'ride_{vehicle["ridingMode"]}_{step["clip"]}'] for step in skill['clips']]}
                           for cls, _, _ in RIDER_CLASSES],
            })
            report.append((vehicle['name'], skill['inputSlot'], skill['skillId'], duration,
                           samples[-1]['forward'] if samples else 0.0, len(samples)))
        profile_by_id[vehicle['vehicleId']]['skills'] = profile_skills
        entry = catalog_by_id[vehicle['vehicleId']]
        rebuilt = {}
        for key, value in entry.items():
            rebuilt[key] = value
            if key == 'riders':
                rebuilt['skills'] = catalog_skills
        catalog_by_id[vehicle['vehicleId']].clear()
        catalog_by_id[vehicle['vehicleId']].update(rebuilt)

    for name, slot, skill_id, duration, forward, count in report:
        print(f'{name:24s} {slot:5s} {skill_id} {duration:5d}ms forward={forward:+.3f}m samples={count}')
    if args.dry_run:
        print('dry run, nothing written')
        return
    profiles['formatVersion'] = 2
    catalog['formatVersion'] = 2
    PROFILES.write_text(json.dumps(profiles, ensure_ascii=False, indent=2) + '\n', encoding='utf8')
    write_catalog(catalog)
    print('wrote', PROFILES.relative_to(ROOT), CATALOG.relative_to(ROOT))


def write_catalog(catalog):
    """Keep the catalog's existing compact style: one line per rider, parameter and texture."""
    def inline(value):
        return json.dumps(value, ensure_ascii=False, separators=(', ', ': ')).replace('{"', '{ "').replace('"}', '" }').replace('], ', '], ').replace(']}', '] }')

    lines = ['{', f'  "schema": {json.dumps(catalog["schema"])},', f'  "formatVersion": {catalog["formatVersion"]},', '  "vehicles": [']
    for vi, vehicle in enumerate(catalog['vehicles']):
        lines.append('    {')
        keys = list(vehicle.keys())
        for ki, key in enumerate(keys):
            value = vehicle[key]
            comma = ',' if ki + 1 < len(keys) else ''
            if key == 'riders':
                lines.append('      "riders": [')
                lines += [f'        {inline(r)}' + (',' if i + 1 < len(value) else '') for i, r in enumerate(value)]
                lines.append('      ]' + comma)
            elif key == 'skills':
                lines.append('      "skills": [')
                for si, skill in enumerate(value):
                    lines.append('        {')
                    lines.append(f'          "skillId": {skill["skillId"]},')
                    lines.append(f'          "inputSlot": {json.dumps(skill["inputSlot"])},')
                    lines.append(f'          "vehicleClips": {inline(skill["vehicleClips"])},')
                    lines.append('          "riders": [')
                    lines += [f'            {inline(r)}' + (',' if i + 1 < len(skill['riders']) else '') for i, r in enumerate(skill['riders'])]
                    lines.append('          ]')
                    lines.append('        }' + (',' if si + 1 < len(value) else ''))
                lines.append('      ]' + comma)
            elif key == 'modelMaterialOverrides':
                lines.append('      "modelMaterialOverrides": [')
                for oi, row in enumerate(value):
                    lines.append('        {')
                    row_keys = list(row.keys())
                    for rk, row_key in enumerate(row_keys):
                        row_comma = ',' if rk + 1 < len(row_keys) else ''
                        if row_key == 'parameters':
                            lines.append('          "parameters": {')
                            items = list(row['parameters'].items())
                            lines += [f'            {json.dumps(k, ensure_ascii=False)}: [{", ".join(repr(float(x)) for x in v)}]' + (',' if i + 1 < len(items) else '')
                                      for i, (k, v) in enumerate(items)]
                            lines.append('          }' + row_comma)
                        elif row_key == 'textures':
                            lines.append('          "textures": [')
                            lines += [f'            {inline(t)}' + (',' if i + 1 < len(row['textures']) else '') for i, t in enumerate(row['textures'])]
                            lines.append('          ]' + row_comma)
                        else:
                            lines.append(f'          {json.dumps(row_key)}: {json.dumps(row[row_key], ensure_ascii=False)}' + row_comma)
                    lines.append('        }' + (',' if oi + 1 < len(value) else ''))
                lines.append('      ]' + comma)
            else:
                lines.append(f'      {json.dumps(key)}: {json.dumps(value, ensure_ascii=False)}' + comma)
        lines.append('    }' + (',' if vi + 1 < len(catalog['vehicles']) else ''))
    lines += ['  ]', '}']
    text = '\n'.join(lines) + '\n'
    if json.loads(text) != catalog:
        fail('catalog formatting changed its content')
    raw = CATALOG.read_bytes().decode('utf8')
    newline = '\r\n' if '\r\n' in raw else '\n'
    CATALOG.write_bytes(text.replace('\n', newline).encode('utf8'))


if __name__ == '__main__':
    sys.exit(main())

#!/usr/bin/env python3
"""Remove the observed reflection-lookup-as-emission error from three fire assets.

This is a narrowly scoped repair of installed WMA2 metadata, not an original
material reconstruction. Geometry, UVs, diffuse, normal and specular are preserved.
Run without --apply to inspect. Originals are retained under out before writes.
"""
import argparse
import hashlib
from pathlib import Path
import struct

ASSETS = {
    'MAP_CFEDE8067300_BG_RAD_KOUKUSATON_DECO24D_SM_KHB':
        ('SLOT_000_bg_rad_koukusaton_deco24_mi_khb',
         'textures/79e70292f4fc_bg_rad_koukusaton_deco24_d_khb.dds'),
    'MAP_B71A2EC9D778_BG_RAD_KOUKUSATON_DECO24E_SM_KHB':
        ('SLOT_000_bg_rad_koukusaton_deco24a_mi_khb',
         'textures/cd58264db472_bg_rad_koukusaton_deco24a_d_khb.dds'),
    'MAP_7AC8BB3D2FEE_BG_RAD_KOUKUSATON_DECO24F_SM_KHB':
        ('SLOT_000_bg_rad_koukusaton_deco24a_mi_khb',
         'textures/cd58264db472_bg_rad_koukusaton_deco24a_d_khb.dds'),
}
BAD_EMISSION = 'textures/b2378a8f80d6_t_tds_specular04.dds'


def require(condition, message):
    if not condition:
        raise ValueError(message)


def corrected(data, material_name, diffuse):
    require(len(data) > 176, 'Truncated WModel')
    require(struct.unpack_from('<4sHHII', data) ==
            (b'WINT', 1, 0, 0, len(data) - 16), 'Unexpected WModel header')
    require(struct.unpack_from('<4sIII4I', data, 16) ==
            (b'WMOD', 2, 0, 0, 0, 0, 0, 0), 'Unexpected model section count')
    sections = [struct.unpack_from('<IIQQ40s', data, 48 + i * 64) for i in range(2)]
    require(sorted(s[0] for s in sections) == [1, 2], 'Unexpected sections')
    section = next(s for s in sections if s[0] == 2)
    start, size = 16 + section[2], section[3]
    require(start + size == len(data), 'Material section is not last')
    require(struct.unpack_from('<4sHHII', data, start) ==
            (b'WINT', 1, 0, 0, size - 16), 'Unexpected material header')
    require(struct.unpack_from('<4sI', data, start + 16) == (b'WMA2', 2),
            'Expected two WMA2 material records')
    require(size == 24 + 2 * (76 + 9 * 520), 'Unexpected WMA2 layout')
    record = start + 24
    require(struct.unpack_from('<I', data, record)[0] == 0, 'Wrong material index')
    require(data[record + 12:record + 76].split(b'\0')[0].decode() == material_name,
            'Material identity differs')
    paths = record + 76
    decode_path = lambda offset: data[offset:offset + 520].decode('utf-16-le').split('\0')[0]
    require(decode_path(paths) == diffuse, 'Diffuse identity differs')
    emission_offset = paths + 3 * 520
    emission = decode_path(emission_offset)
    require(emission in ('', BAD_EMISSION), 'Unexpected emissive path; inspect instead of overwriting')
    result = data[:emission_offset] + bytes(520) + data[emission_offset + 520:]
    require(len(result) == len(data), 'Container size changed')
    return result, emission


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repository-root', type=Path, required=True)
    parser.add_argument('--apply', action='store_true')
    args = parser.parse_args()
    root = args.repository_root.resolve(strict=True)
    resource_root = root / 'Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED'
    staged = []
    for asset, (material, diffuse) in ASSETS.items():
        path = resource_root / asset / (asset + '.wmodel')
        require(path.resolve().is_relative_to(resource_root.resolve()), 'Resource escaped root')
        original = path.read_bytes()
        replacement, emission = corrected(original, material, diffuse)
        staged.append((path, original, replacement))
        print(f'{asset}: emissive={emission or "<empty>"}; '
              f'{"repair" if original != replacement else "already repaired"}')
    if not args.apply:
        return
    backup_root = root / 'out/KoukuGate3FireMaterialBackup'
    backup_root.mkdir(parents=True, exist_ok=True)
    # Stage all originals before touching any resource. A concurrent edit aborts.
    for path, original, replacement in staged:
        require(path.read_bytes() == original, f'Concurrent resource edit: {path}')
        if original != replacement:
            backup = backup_root / (path.stem + '.' + hashlib.sha256(original).hexdigest() + '.wmodel')
            if backup.exists():
                require(backup.read_bytes() == original, 'Existing backup differs')
            else:
                backup.write_bytes(original)
    written = []
    try:
        for path, original, replacement in staged:
            require(path.read_bytes() == original, f'Concurrent resource edit: {path}')
            if original == replacement:
                continue
            written.append((path, original))
            path.write_bytes(replacement)
            require(path.read_bytes() == replacement, f'Readback failed: {path}')
    except BaseException:
        for path, original in reversed(written):
            path.write_bytes(original)
        raise
    print(f'Applied {len(written)} material repairs; backups: {backup_root}')


if __name__ == '__main__':
    main()

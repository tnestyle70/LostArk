#!/usr/bin/env python3
"""Prepare the scoped inward black backdrop; --apply installs the reviewed data.

This is project presentation tuning, not a replacement for the original mesh.
Only one stable effect element consumes the derivative. Shader inputs stay intact.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import tempfile

ROOT = Path(__file__).resolve().parents[2]
DOCUMENT = Path('Data/Effects/Authored/effect.kouku.sequence.lv_lut_midnightc_ed_scene02a.efseqact_matinee_10.1.effect.json')
ELEMENT = 'kouku.action.233d0d5adbba2933178bf071'
SOURCE = 'Effect/KoukuSaydon/FullRestore/Meshes/fm_b_cylinder_002.wmodel'
TARGET = 'Effect/KoukuSaydon/FullRestore/Meshes/fm_b_cylinder_002_gate2_clear_inward.wmodel'
OUTPUT = Path('out/KoukuPortalLighting20260922/candidate')


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def make_backdrop(source: bytes) -> bytes:
    require(source[:4] == b'WINT' and source[16:20] == b'WMOD', 'Not a WModel')
    count = struct.unpack_from('<I', source, 20)[0]
    sections = [struct.unpack_from('<IIQQ40s', source, 48 + i * 64) for i in range(count)]
    meshes = [s for s in sections if s[0] == 1]
    require(len(meshes) == 1, 'Expected one mesh section')
    offset = 16 + meshes[0][2] + 16
    header = struct.unpack_from('<4sIIIIIIIB3s', source, offset)
    require(header[:8] == (b'WMSH', 1, 0, 47, 48, 26, 72, 2), 'Original cylinder layout changed')
    vertices = offset + 36 + header[1] * 48
    indices = vertices + header[4] * header[5]
    result = bytearray(source)
    # Enclose the actors without changing the coloured portal's shape or scale.
    # This uncapped cylinder has horizontal normals; radial expansion preserves them.
    for i in range(header[5]):
        address = vertices + i * header[4]
        x, y, z, nx, ny, nz = struct.unpack_from('<6f', source, address)
        require(abs(ny) < .02, 'Cylinder has a non-radial normal beyond source quantization')
        struct.pack_into('<3f', result, address, x * 2, y, z * 2)
    for i in range(0, header[6], 3):
        address = indices + i * header[7]
        a, b, c = struct.unpack_from('<3H', source, address)
        struct.pack_into('<3H', result, address, a, c, b)
    require(len(result) == len(source), 'WModel size changed')
    return bytes(result)


def update_document(original: bytes) -> bytes:
    before = json.loads(original)
    rows = [e for e in before['elements'] if e['id'] == ELEMENT]
    require(len(rows) == 1, 'Stable element missing or duplicated')
    row = rows[0]
    require(row['material']['sourceProfile']['runtimeShaderProfileId'] ==
            'effect.ue3.kouku-3330-native.v1', 'Native material changed')
    bindings = row['resources']
    require(len(bindings) == 1 and bindings[0]['slotId'] == 'meshModel', 'Mesh binding changed')
    current = bindings[0]['assetId']
    require(current in (SOURCE, TARGET), 'User changed the target mesh; preserve it')
    if current == TARGET:
        return original
    # Preserve formatting and every unrelated field in the latest saved document.
    begin = original.index(('"id": "' + ELEMENT + '"').encode())
    end = original.find(b'\n      "id":', begin + 1)
    if end < 0:
        end = len(original)
    block = original[begin:end]
    old = ('"assetId": "' + SOURCE + '"').encode()
    new = ('"assetId": "' + TARGET + '"').encode()
    require(block.count(old) == 1, 'Ambiguous mesh binding text')
    result = original[:begin] + block.replace(old, new, 1) + original[end:]
    bindings[0]['assetId'] = TARGET
    require(json.loads(result) == before, 'Unexpected document changes')
    return result


def atomic_replace(path: Path, data: bytes, expected: bytes | None) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle, temporary = tempfile.mkstemp(prefix=path.name + '.', suffix='.tmp', dir=path.parent)
    try:
        with os.fdopen(handle, 'wb') as stream:
            stream.write(data)
            stream.flush()
            os.fsync(stream.fileno())
        actual = path.read_bytes() if path.exists() else None
        require(actual == expected, f'Concurrent change preserved: {path}')
        os.replace(temporary, path)
    finally:
        if os.path.exists(temporary):
            os.unlink(temporary)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--apply', action='store_true', help='Install only after saved-document approval')
    args = parser.parse_args()
    resource_root = ROOT / 'Client/Bin/Resources'
    source = (resource_root / SOURCE).read_bytes()
    mesh = make_backdrop(source)
    original = (ROOT / DOCUMENT).read_bytes()
    candidate = update_document(original)
    output = ROOT / OUTPUT
    (output / 'Resources' / TARGET).parent.mkdir(parents=True, exist_ok=True)
    (output / 'Resources' / TARGET).write_bytes(mesh)
    (output / DOCUMENT.name).write_bytes(candidate)
    receipt = {'status': 'candidate', 'elementId': ELEMENT, 'sourceAssetId': SOURCE,
               'targetAssetId': TARGET, 'sourceSha256': digest(source), 'targetSha256': digest(mesh),
               'bytes': len(mesh), 'radialScale': 2, 'winding': 'inward',
               'documentBeforeSha256': digest(original), 'documentAfterSha256': digest(candidate)}
    if args.apply:
        target = resource_root / TARGET
        existing = target.read_bytes() if target.exists() else None
        require(existing in (None, mesh), 'Existing derivative differs; preserve it')
        require((resource_root / SOURCE).read_bytes() == source, 'Source mesh changed')
        backup = output / ('document-before-' + digest(original) + '.json')
        backup.write_bytes(original)
        installed = False
        try:
            if existing is None:
                atomic_replace(target, mesh, None)
                installed = True
            atomic_replace(ROOT / DOCUMENT, candidate, original)
        except Exception:
            if installed and target.exists() and target.read_bytes() == mesh:
                target.unlink()
            raise
        receipt['status'] = 'installed-disk-only'
        receipt['backup'] = str(backup)
    (output / 'receipt.json').write_text(json.dumps(receipt, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(receipt, ensure_ascii=False, indent=2))


if __name__ == '__main__':
    main()

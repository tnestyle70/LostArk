"""Read a Lost Ark cutscene matinee completely, or fail.

The shipped desktop export stopped after the first element of every array
property, so the camera cut order, the screen fades, the field-of-view curves
and the toggles that reveal each prop were unknown while still looking like
real values. Those tracks are what decide the shot, so every array is walked
element by element and a document is only written when each array's declared
element count matches the number actually decoded.

Ownership is read the same way the engine reads it. InterpData names its
groups, a group names its tracks, and UInterpGroupDirector plays the first
track in that order whose bDisableTrack is not set, which is how one cut list
is chosen out of the several a scene can carry.

Coordinates and the field-of-view axis are converted here once. UE3 stores
centimetres with Z up and a horizontal FOVAngle; the runtime documents use
metres with Y up and a vertical fovYDegrees, and mixing the two up cost days
on the pop-up book cutscene.
"""
from __future__ import annotations

import argparse
import io
import json
import math
import os
import struct
import sys
from typing import Any

# The package reader lives with the original extraction tools.
_TOOLS_ENV = 'LOSTARK_SCENE_TOOLS'
_DEFAULT_TOOLS = os.path.join(
    os.path.expanduser('~'), 'OneDrive', '바탕 화면',
    '쿠크_컷신_전체추출_20260904', '추출도구')

# UE3 authored these scenes for a 3:2 viewport, and FOVAngle is horizontal.
SOURCE_ASPECT = 1.5
# Every element of an array of tagged structs opens with one of these pairs,
# which is how element boundaries are found without trusting a terminator.
CUT_HEAD_FIELDS = ('time', 'floatproperty')
CURVE_HEAD_FIELDS = ('inval', 'floatproperty')
ANIM_HEAD_FIELDS = ('starttime', 'floatproperty')
# A property tag is name(8) + type(8) + size(4) + arrayIndex(4).
TAG_HEADER_BYTES = 24
# Fields whose decoded value is the name id of 'none' in every element carry
# no information; shotnumber is the one this scene family writes that way.
SUSPECT_CONSTANT_FIELDS = ('shotnumber',)


def _load_package_reader() -> Any:
    tools = os.environ.get(_TOOLS_ENV, _DEFAULT_TOOLS)
    if tools not in sys.path:
        sys.path.insert(0, tools)
    try:
        import la_upk  # type: ignore
    except ImportError as exc:
        raise SystemExit(
            'la_upk.py not found. Set %s to the folder that holds it '
            '(looked in %s).' % (_TOOLS_ENV, tools)) from exc
    return la_upk


class _ShimPackage:
    """One decrypted package plus the tables needed to read its objects.

    Adapter: the original desktop la_upk module is no longer on this PC, so the
    project's own package reader (extract_ue3_placements) supplies the logical
    bytes and the name/import/export tables in the same shape.
    """

    def __init__(self, path: str) -> None:
        import pathlib
        root = pathlib.Path(__file__).resolve().parents[1] / "LevelPlacementExtractor"
        if str(root) not in sys.path:
            sys.path.insert(0, str(root))
        import extract_ue3_placements as x  # type: ignore
        p = pathlib.Path(path)
        summary = x.read_package_summary(p)
        rd = x.LostArkPackageRangeReader(p, summary)
        self.data = rd.read_logical_range(0, rd.logical_size)
        end = min(max(summary.header_size, summary.depends_offset) + 64, rd.logical_size)
        logical = self.data[:end]
        self.names = x.parse_name_table(logical, summary)
        imports = x.parse_import_table(logical, summary, self.names)
        raw_exports = x.parse_export_table(logical, summary, self.names)
        self.imports = imports
        self.exports = []
        for e in raw_exports:
            self.exports.append({
                'name': e.object_name,
                'className': x.package_ref_name(e.class_index, imports, raw_exports),
                'offset': e.serial_offset,
                'size': e.serial_size,
            })
        self.name_id = {name: index for index, name in enumerate(self.names)}
        self.none_id = self.name_id.get('none', -1)

class Package(_ShimPackage):
    def name_at(self, offset: int) -> str:
        index, number = struct.unpack_from('<2I', self.data, offset)
        base = self.names[index] if index < len(self.names) else '<%d>' % index
        return base if number == 0 else '%s_%d' % (base, number - 1)

    def head_bytes(self, first: str, second: str) -> bytes:
        return (struct.pack('<2I', self.name_id[first], 0) +
                struct.pack('<2I', self.name_id[second], 0))

    def export_end(self, export: dict) -> int:
        return export['offset'] + export['size']


def tagged_properties(pkg: Package, start: int, end: int) -> list[tuple]:
    """Tagged properties as (name, type, size, valueOffset, nextOffset).

    Stops at the 'none' terminator or at the first tag that cannot be a
    property, so a caller can tell a clean run from noise. A bool stores its
    value in one byte after the header and declares size zero.
    """
    rows: list[tuple] = []
    offset = start
    while offset + TAG_HEADER_BYTES <= end:
        pname = pkg.name_at(offset)
        if pname.lower() == 'none':
            break
        ptype = pkg.name_at(offset + 8)
        if not ptype.endswith('property'):
            break
        size, _array_index = struct.unpack_from('<2i', pkg.data, offset + 16)
        body = offset + TAG_HEADER_BYTES
        if ptype in ('structproperty', 'byteproperty'):
            body += 8                      # the struct or enum name follows
        if size < 0 or body + size > end:
            break
        width = 1 if ptype == 'boolproperty' else size
        if body + width > end:
            break
        rows.append((pname, ptype, size, body, body + width))
        offset = body + width
    return rows


def find_property(pkg: Package, export: dict, wanted_name: str,
                  wanted_type: str) -> tuple[int, int] | None:
    """Locate one property's (valueOffset, size) anywhere in the object.

    Each UE3 class writes a different amount of native data before its tagged
    properties and some properties sit after a long array, so the whole object
    is scanned on four byte steps rather than only its first bytes.
    """
    start = export['offset']
    end = pkg.export_end(export)
    offset = start
    while offset + TAG_HEADER_BYTES <= end:
        if pkg.name_at(offset) == wanted_name and \
                pkg.name_at(offset + 8) == wanted_type:
            size, _array_index = struct.unpack_from('<2i', pkg.data,
                                                    offset + 16)
            body = offset + TAG_HEADER_BYTES
            if wanted_type in ('structproperty', 'byteproperty'):
                body += 8
            width = 1 if wanted_type == 'boolproperty' else size
            if size >= 0 and body + width <= end:
                return body, size
        offset += 4
    return None


def scalar_value(pkg: Package, ptype: str, size: int, offset: int) -> Any:
    if ptype == 'floatproperty' and size == 4:
        return round(struct.unpack_from('<f', pkg.data, offset)[0], 6)
    if ptype == 'intproperty' and size == 4:
        return struct.unpack_from('<i', pkg.data, offset)[0]
    if ptype == 'nameproperty' and size == 8:
        return pkg.name_at(offset)
    if ptype == 'byteproperty' and size == 8:
        return pkg.name_at(offset)
    if ptype == 'boolproperty':
        return pkg.data[offset] != 0
    if ptype == 'structproperty' and size == 12:
        return [round(v, 6) for v in struct.unpack_from('<3f', pkg.data,
                                                        offset)]
    return None


def read_struct_array(pkg: Package, body: int, size: int,
                      head: tuple[str, str], label: str) -> list[dict]:
    """Every element of an array of tagged structs, or raise.

    Element starts are found by the pair of tags each element opens with,
    because the shipped reader mis-walked the terminator and silently stopped
    after element zero.
    """
    declared = struct.unpack_from('<i', pkg.data, body)[0]
    if declared < 0:
        raise ValueError('%s: negative element count %d' % (label, declared))
    if declared == 0:
        return []
    end = body + size
    pattern = pkg.head_bytes(head[0], head[1])
    starts: list[int] = []
    probe = body + 4
    while True:
        found = pkg.data.find(pattern, probe, end)
        if found < 0:
            break
        starts.append(found)
        probe = found + 8
    rows: list[dict] = []
    for element_start in starts:
        fields: dict[str, Any] = {}
        for pname, ptype, psize, voffset, _next in tagged_properties(
                pkg, element_start, end):
            value = scalar_value(pkg, ptype, psize, voffset)
            if value is None:
                break
            fields[pname] = value
        if fields:
            rows.append(fields)
    if len(rows) != declared:
        raise ValueError('%s: declared %d elements but decoded %d'
                         % (label, declared, len(rows)))
    return rows


def drop_suspect_fields(pkg: Package, rows: list[dict]) -> list[dict]:
    """Remove fields that decode to the name id of 'none' in every element.

    UE3 writes a struct's last property immediately before the element
    terminator, and this build's director cut writes a shotnumber whose bytes
    read back as that terminator's name id in every cut of every track. A
    field that never varies and equals that id is noise, not a shot number.
    """
    cleaned = []
    for row in rows:
        copy = dict(row)
        for field in SUSPECT_CONSTANT_FIELDS:
            if field in copy and copy[field] == pkg.none_id:
                del copy[field]
        cleaned.append(copy)
    return cleaned


def object_array(pkg: Package, export: dict, wanted: str) -> list[int]:
    """An array of object references, as export indices."""
    found = find_property(pkg, export, wanted, 'arrayproperty')
    if not found:
        return []
    body, size = found
    count = struct.unpack_from('<i', pkg.data, body)[0]
    if count <= 0 or size - 4 < count * 4:
        return []
    return list(struct.unpack_from('<%di' % count, pkg.data, body + 4))


def is_disabled(pkg: Package, export: dict) -> bool:
    """UE3 omits a bool at its default, so a written bDisableTrack means on."""
    found = find_property(pkg, export, 'bdisabletrack', 'boolproperty')
    if not found:
        return False
    body, _size = found
    return pkg.data[body] != 0


def cut_keys(pkg: Package, export: dict) -> list[dict]:
    """The camera cut list of one InterpTrackDirector."""
    found = find_property(pkg, export, 'cuttrack', 'arrayproperty')
    if not found:
        return []
    body, size = found
    label = 'cuttrack of %s' % export['name']
    rows = drop_suspect_fields(
        pkg, read_struct_array(pkg, body, size, CUT_HEAD_FIELDS, label))
    return [{'timeMs': int(round(r.get('time', 0.0) * 1000.0)),
             'transitionMs': int(round(r.get('transitiontime', 0.0) * 1000.0)),
             'camera': r.get('targetcamgroup')} for r in rows]


def curve_keys(pkg: Package, export: dict, container: str,
               container_type: str) -> list[dict]:
    """The points of one FInterpCurve, whether it is nested or not.

    A fade track exposes 'points' directly; a float property track wraps the
    same curve in a 'floattrack' struct, so the struct is opened first and
    'points' is searched inside it.
    """
    found = find_property(pkg, export, container, container_type)
    if not found:
        return []
    body, size = found
    if container_type == 'arrayproperty':
        points_body, points_size = body, size
    else:
        points_body = None
        points_size = 0
        end = body + size
        offset = body
        while offset + TAG_HEADER_BYTES <= end:
            if pkg.name_at(offset) == 'points' and \
                    pkg.name_at(offset + 8) == 'arrayproperty':
                points_size, _array_index = struct.unpack_from(
                    '<2i', pkg.data, offset + 16)
                points_body = offset + TAG_HEADER_BYTES
                break
            offset += 4
        if points_body is None:
            return []
    label = '%s of %s' % (container, export['name'])
    rows = read_struct_array(pkg, points_body, points_size,
                             CURVE_HEAD_FIELDS, label)
    return [{'timeMs': int(round(r.get('inval', 0.0) * 1000.0)),
             'value': r.get('outval'),
             'arrive': r.get('arrivetangent', 0.0),
             'leave': r.get('leavetangent', 0.0),
             'mode': r.get('interpmode')} for r in rows]


def switch_keys(pkg: Package, export: dict, container: str) -> list[dict]:
    """Toggle, visibility and event keys all share the cut element head."""
    found = find_property(pkg, export, container, 'arrayproperty')
    if not found:
        return []
    body, size = found
    label = '%s of %s' % (container, export['name'])
    rows = read_struct_array(pkg, body, size, CUT_HEAD_FIELDS, label)
    keys = []
    for row in rows:
        key = {'timeMs': int(round(row.get('time', 0.0) * 1000.0))}
        for field in ('toggleaction', 'action', 'activecondition',
                      'eventname'):
            if field in row:
                key[field] = row[field]
        keys.append(key)
    return keys


def to_our_position(x: float, y: float, z: float) -> list[float]:
    """UE3 centimetres with Z up become our metres with Y up."""
    return [round(x * 0.01, 6), round(z * 0.01, 6), round(-y * 0.01, 6)]


def horizontal_to_vertical_fov(fov_x_degrees: float,
                               aspect: float = SOURCE_ASPECT) -> float:
    """UE3 FOVAngle is horizontal; our documents store a vertical angle."""
    if fov_x_degrees <= 0.0 or fov_x_degrees >= 180.0:
        return fov_x_degrees
    half = math.radians(fov_x_degrees) * 0.5
    return round(math.degrees(2.0 * math.atan(math.tan(half) / aspect)), 4)


def vector_curve(pkg: Package, export: dict, container: str) -> list[dict]:
    """One FInterpCurveVector as our own axes.

    UE3 keeps position and rotation as separate curves on the same track, and
    a constant segment holds its value and then jumps, which is how a prop
    arrives from the staging copy of the set.
    """
    found = find_property(pkg, export, container, 'structproperty')
    if not found:
        return []
    body, size = found
    end = body + size
    points_body = None
    points_size = 0
    offset = body
    while offset + TAG_HEADER_BYTES <= end:
        if pkg.name_at(offset) == 'points' and                 pkg.name_at(offset + 8) == 'arrayproperty':
            points_size, _array_index = struct.unpack_from(
                '<2i', pkg.data, offset + 16)
            points_body = offset + TAG_HEADER_BYTES
            break
        offset += 4
    if points_body is None:
        return []
    label = '%s of %s' % (container, export['name'])
    rows = read_struct_array(pkg, points_body, points_size,
                             CURVE_HEAD_FIELDS, label)
    keys = []
    for row in rows:
        value = row.get('outval') or [0.0, 0.0, 0.0]
        key = {'timeMs': int(round(row.get('inval', 0.0) * 1000.0)),
               'mode': row.get('interpmode')}
        if container == 'postrack':
            key['value'] = to_our_position(*value)
        else:
            # Euler comes out of FRotator::Euler as (Roll, Pitch, Yaw).
            key['rollPitchYawDegrees'] = value
        keys.append(key)
    return keys


def anim_sequences(pkg: Package, export: dict) -> list[dict]:
    """Clip, start time and the playback fields the shipped export dropped."""
    found = find_property(pkg, export, 'animseqs', 'arrayproperty')
    if not found:
        return []
    body, size = found
    label = 'animseqs of %s' % export['name']
    rows = read_struct_array(pkg, body, size, ANIM_HEAD_FIELDS, label)
    keys = []
    for row in rows:
        keys.append({
            'timeMs': int(round(row.get('starttime', 0.0) * 1000.0)),
            'clip': row.get('animseqname'),
            'startOffsetMs': int(round(row.get('animstartoffset', 0.0) * 1000.0)),
            'endOffsetMs': int(round(row.get('animendoffset', 0.0) * 1000.0)),
            'playRate': row.get('animplayrate', 1.0),
            'looping': bool(row.get('blooping', False)),
            'reverse': bool(row.get('breverse', False)),
            'rootMotion': bool(row.get('benablerootmotion', False)),
        })
    return keys


TRACK_READERS = {
    'interptracktoggle': ('toggle', 'toggletrack'),
    'interptrackvisibility': ('visibility', 'visibilitytrack'),
    'interptrackevent': ('event', 'eventtrack'),
}


def read_track(pkg: Package, export: dict) -> dict | None:
    """One track as a record, with every array fully decoded."""
    class_name = export['className']
    # bDisableTrack is written only when set, and it is set on move tracks
    # too, so a track that exists is not automatically a track that plays.
    record = {'track': export['name'], 'class': class_name,
              'offset': export['offset'], 'disabled': is_disabled(pkg, export)}
    if class_name == 'interptrackdirector':
        record['cuts'] = cut_keys(pkg, export)
        return record
    if class_name in ('interptrackfade', 'interptrackslomo',
                      'interptrackcolorscale'):
        keys = curve_keys(pkg, export, 'points', 'arrayproperty')
        if not keys:
            keys = curve_keys(pkg, export, 'floattrack', 'structproperty')
        record['keys'] = keys
        return record
    if class_name == 'interptrackfloatprop':
        named = find_property(pkg, export, 'propertyname', 'nameproperty')
        if not named:
            return None
        record['property'] = pkg.name_at(named[0])
        keys = curve_keys(pkg, export, 'floattrack', 'structproperty')
        if 'fov' in record['property'].lower():
            for key in keys:
                key['fovXDegrees'] = key['value']
                key['fovYDegrees'] = horizontal_to_vertical_fov(key['value'])
        record['keys'] = keys
        return record
    if class_name == 'interptrackmove':
        frame = find_property(pkg, export, 'moveframe', 'byteproperty')
        # Written only when it differs from the class default, so an absent
        # tag and a present one are different contracts and both are recorded.
        record['moveFrame'] = pkg.name_at(frame[0]) if frame else 'default'
        record['position'] = vector_curve(pkg, export, 'postrack')
        record['euler'] = vector_curve(pkg, export, 'eulertrack')
        return record
    if class_name == 'interptrackanimcontrol':
        slot = find_property(pkg, export, 'slotname', 'nameproperty')
        record['slot'] = pkg.name_at(slot[0]) if slot else None
        record['clips'] = anim_sequences(pkg, export)
        return record
    if class_name in TRACK_READERS:
        bucket, container = TRACK_READERS[class_name]
        record['kind'] = bucket
        record['keys'] = switch_keys(pkg, export, container)
        return record
    return None


def collect(pkg: Package) -> list[dict]:
    """Sequences, their groups and the tracks this reader understands."""
    sequences = []
    for index, export in enumerate(pkg.exports, start=1):
        if export['className'] != 'interpdata':
            continue
        groups = []
        for group_ref in object_array(pkg, export, 'interpgroups'):
            if not 0 < group_ref <= len(pkg.exports):
                continue
            group_export = pkg.exports[group_ref - 1]
            tracks = []
            for track_ref in object_array(pkg, group_export, 'interptracks'):
                if not 0 < track_ref <= len(pkg.exports):
                    continue
                record = read_track(pkg, pkg.exports[track_ref - 1])
                if record:
                    tracks.append(record)
            groups.append({'group': group_export['name'],
                           'class': group_export['className'],
                           'tracks': tracks})
        # UE3 plays the first director track that is not disabled.
        live = None
        for group in groups:
            if group['class'] != 'interpgroupdirector':
                continue
            for track in group['tracks']:
                if track['class'] != 'interptrackdirector':
                    continue
                if not track.get('disabled'):
                    live = {'group': group['group'], 'track': track['track'],
                            'offset': track['offset'], 'cuts': track['cuts']}
                    break
            if live:
                break
        sequences.append({'data': export['name'], 'groupCount': len(groups),
                          'liveDirectorTrack': live, 'groups': groups})
    return sequences


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description='Fully decode a Lost Ark cutscene matinee package.')
    parser.add_argument('--package', required=True,
                        help='path to the scene .upk')
    parser.add_argument('--output', required=True,
                        help='path of the JSON document to write')
    parser.add_argument('--label', default='',
                        help='logical scene name recorded in the document')
    args = parser.parse_args(argv)

    pkg = Package(args.package)
    sequences = collect(pkg)

    document = {
        'schema': 'lostark.scene-matinee',
        'formatVersion': 2,
        'package': os.path.basename(args.package),
        'label': args.label or os.path.basename(args.package),
        'coordinateSystem': 'metres, Y up; UE3 (x, y, z) cm mapped to '
                            '(x*0.01, z*0.01, -y*0.01)',
        'fovNote': 'fovXDegrees is the UE3 horizontal FOVAngle; fovYDegrees '
                   'is the vertical angle our camera documents use, at '
                   'aspect %.3f' % SOURCE_ASPECT,
        'directorNote': 'liveDirectorTrack is the first track of the director '
                        'group whose bDisableTrack is not set, which is the '
                        'one UInterpGroupDirector plays.',
        'sequences': sequences,
    }

    with io.open(args.output, 'w', encoding='utf-8') as handle:
        json.dump(document, handle, ensure_ascii=False, indent=1)
        handle.write('\n')

    for sequence in sequences:
        live = sequence['liveDirectorTrack']
        print('%-14s groups=%-4d live director: %s' % (
            sequence['data'], sequence['groupCount'],
            live['track'] if live else 'none'))
        if live:
            print('    %s' % ' | '.join(
                '%.2fs %s' % (c['timeMs'] / 1000.0, c['camera'])
                for c in live['cuts']))
    print('written: %s' % args.output)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())

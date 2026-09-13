"""Keep generated native material arrays in their single private C++ owner.

Generators still edit the complete logical header in memory. Public descriptor
and inline consumer code are saved separately from the table payload, so adding
a program does not rewrite every consumer's header. Standalone candidate headers
retain the original complete format.
"""
from pathlib import Path
import os
import re
import tempfile

FAMILIES = (
    'Effect_ArtistMaterial', 'Effect_LanceMasterVAMaterial',
    'Effect_WarlordNativeMaterial', 'Effect_DimensionMasterALTVMaterial',
    'Effect_DimensionMasterQMaterial', 'Effect_DimensionMasterVMaterial',
    'Effect_DimensionMasterWRMaterial', 'Effect_DimensionMasterSDMaterial',
)
TABLE = re.compile(r'inline constexpr std::array<([A-Z_]+_PROGRAM_DESC),\s*(\d+)> ([A-Z_]+_PROGRAMS) = \{\{')
PUBLIC_CONSTANT = re.compile(r'inline constexpr (?:std::string_view|uint32_t) ([A-Z][A-Z0-9_]*)\s*=.*?;', re.S)
SPLIT = re.compile(
    r'// Native material tables are compiled once; generators use native_material_tables\.py\.\n'
    r'// native-material-tables: \.\./Private/([A-Za-z0-9_]+_Tables\.inl)\n'
    r'(?:// native-material-public-constants-begin\n(.*?)\n// native-material-public-constants-end\n)?'
    r'extern const std::span<const ([A-Z_]+_PROGRAM_DESC)> ([A-Z_]+_PROGRAMS);\n'
    r'// native-material-tables-end', re.S)


def _decode(raw):
    bom = raw.startswith(b'\xef\xbb\xbf')
    payload = raw[3:] if bom else raw
    for encoding in ('utf-8', 'cp949'):
        try:
            return payload.decode(encoding), encoding, bom
        except UnicodeDecodeError:
            pass
    raise ValueError('Native material source is neither UTF-8 nor CP949')


def _text(path):
    return _decode(Path(path).read_bytes())[0].replace('\r\n', '\n')


def _table_path(header_path, table_name):
    expected = header_path.stem + '_Tables.inl'
    if header_path.stem not in FAMILIES or table_name != expected:
        raise ValueError(('Native material table owner differs', header_path, table_name))
    return header_path.parent.parent / 'Private' / table_name


def read_material_source(header_path):
    """Return the complete logical header, with LF newlines, for existing tools."""
    header_path = Path(header_path)
    source = _text(header_path)
    matches = list(SPLIT.finditer(source))
    if not matches:
        if '// native-material-tables:' in source:
            raise ValueError(('Malformed native material table declaration', header_path))
        return source
    if len(matches) != 1:
        raise ValueError(('Multiple native material table declarations', header_path))
    match = matches[0]
    table_name, public_constants, descriptor, programs = match.groups()
    tables = _text(_table_path(header_path, table_name)).removesuffix('\n')
    for constant in PUBLIC_CONSTANT.finditer(public_constants or ''):
        marker = '// native-material-public-constant: ' + constant.group(1)
        if tables.count(marker) != 1:
            raise ValueError(('Native material public constant marker differs', header_path, constant.group(1)))
        tables = tables.replace(marker, constant.group(0))
    if '// native-material-public-constant:' in tables:
        raise ValueError(('Missing native material public constant declaration', header_path))
    tables = tables.replace('constexpr std::array<', 'inline constexpr std::array<')
    storage = programs.removesuffix('_PROGRAMS') + '_PROGRAM_STORAGE'
    expected = re.compile(r'(inline constexpr std::array<' + re.escape(descriptor) + r',\s*\d+>) ' + re.escape(storage) + r' =')
    tables, count = expected.subn(r'\1 ' + programs + ' =', tables)
    if count != 1:
        raise ValueError(('Missing native material program storage', header_path))
    return source[:match.start()] + tables + source[match.end():]


def read_material_bytes(header_path):
    """Expand tables while preserving the public header's encoding and newlines."""
    original, encoding, bom = _decode(Path(header_path).read_bytes())
    source = read_material_source(header_path)
    if '\r\n' in original:
        source = source.replace('\n', '\r\n')
    return (b'\xef\xbb\xbf' if bom else b'') + source.encode(encoding)


def _encoded(path, source, *, existing_format=True):
    encoding, bom, newline = 'utf-8', False, '\n'
    if existing_format and path.exists():
        previous, encoding, bom = _decode(path.read_bytes())
        newline = '\r\n' if '\r\n' in previous else '\n'
    source = source.replace('\r\n', '\n').replace('\n', newline)
    return (b'\xef\xbb\xbf' if bom else b'') + source.encode(encoding)


def _save_changed(outputs):
    # Validate/encode all outputs before replacing any product file. Keep a byte
    # snapshot to reject concurrent edits and restore replaced files on failure.
    originals = {path: path.read_bytes() if path.exists() else None for path in outputs}
    changed = {path: data for path, data in outputs.items() if originals[path] != data}
    staged, replaced = {}, []
    try:
        for path, data in changed.items():
            path.parent.mkdir(parents=True, exist_ok=True)
            with tempfile.NamedTemporaryFile(dir=path.parent, prefix=path.name+'.', suffix='.tmp', delete=False) as temp:
                temp.write(data)
                staged[path] = Path(temp.name)
        for path, previous in originals.items():
            if (path.read_bytes() if path.exists() else None) != previous:
                raise RuntimeError(('Native material source changed concurrently', path))
        for path, temporary in staged.items():
            os.replace(temporary, path)
            replaced.append(path)
    except Exception:
        for path in reversed(replaced):
            if originals[path] is None:
                path.unlink()
            else:
                path.write_bytes(originals[path])
        raise
    finally:
        for temporary in staged.values():
            if temporary.exists():
                temporary.unlink()
    return [str(path) for path in changed]


def write_material_source(header_path, source, *, expected_source=None):
    """Store a generated logical header without widening its include fanout."""
    header_path = Path(header_path)
    source = source.replace('\r\n', '\n')
    if expected_source is not None and read_material_source(header_path) != expected_source.replace('\r\n', '\n'):
        raise RuntimeError(('Native material source changed concurrently; rerun against current source', header_path))
    if header_path.stem not in FAMILIES or header_path.parent.name.lower() != 'public':
        return _save_changed({header_path: _encoded(header_path, source)})

    matches = list(TABLE.finditer(source))
    if len(matches) != 1:
        raise ValueError(('Expected one native material program table', header_path))
    match = matches[0]
    descriptor, count, programs = match.groups()
    start = source.index('inline constexpr std::array<')
    end = source.index('}};', match.end()) + 3
    entries = source[match.end():end-3]
    if len(re.findall(r'^\s*\{\d+u,', entries, re.M)) != int(count):
        raise ValueError(('Native material program table count differs', header_path))
    tables = source[start:end]
    public_constants = list(PUBLIC_CONSTANT.finditer(tables))
    constant_declarations = '\n'.join(constant.group(0) for constant in public_constants)
    for constant in reversed(public_constants):
        tables = tables[:constant.start()] + '// native-material-public-constant: ' + constant.group(1) + tables[constant.end():]
    storage = programs.removesuffix('_PROGRAMS') + '_PROGRAM_STORAGE'
    tables = tables.replace('inline constexpr std::array<', 'constexpr std::array<')
    tables = tables.replace('> '+programs+' =', '> '+storage+' =')
    table_name = header_path.stem + '_Tables.inl'
    declaration = (
        '// Native material tables are compiled once; generators use native_material_tables.py.\n'
        f'// native-material-tables: ../Private/{table_name}\n' +
        ('// native-material-public-constants-begin\n' + constant_declarations +
         '\n// native-material-public-constants-end\n' if constant_declarations else '') +
        f'extern const std::span<const {descriptor}> {programs};\n'
        '// native-material-tables-end')
    header = source[:start] + declaration + source[end:]
    owner = (
        f'#include "{header_path.name}"\n\n'
        'NS_BEGIN(Client)\n'
        f'#include "{table_name}"\n\n'
        '// The backing arrays have static storage; no runtime initialization is needed.\n'
        f'constinit const std::span<const {descriptor}> {programs}{{{storage}}};\n'
        'NS_END\n')
    table_path = _table_path(header_path, table_name)
    cpp_path = table_path.with_name(header_path.stem+'.cpp')
    return _save_changed({
        table_path: _encoded(table_path, tables+'\n'),
        cpp_path: _encoded(cpp_path, owner),
        header_path: _encoded(header_path, header),
    })

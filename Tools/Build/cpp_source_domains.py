"""Read the registered translation units that implement one split C++ source.

This is a source-inspection helper, not a build input or a runtime loader. Source
contract checks must inspect all implementation units while project/encoding
checks can still read the original physical file directly.
"""
from functools import lru_cache
from pathlib import Path
import xml.etree.ElementTree as ET

_NAMESPACE = {'m': 'http://schemas.microsoft.com/developer/msbuild/2003'}
_SPLIT_DOMAINS = frozenset((
    'Effect_Tool', 'Effect_DocumentRenderer', 'MapTool', 'Animation_Tool',
    'Effect_DocumentCodec', 'GameRoom', 'ServerGameplayContractTests',
))


@lru_cache(maxsize=32)
def _registered_sources(project, modified_ns, size):
    del modified_ns, size
    tree = ET.parse(project)
    return tuple((project.parent / item.get('Include').replace('\\', '/')).resolve()
                 for item in tree.findall('m:ItemGroup/m:ClCompile', _NAMESPACE)
                 if item.get('Include') and '$(' not in item.get('Include'))


def cpp_domain_paths(project_root, stem, *, exclude_stems=()):
    """Return core then registered same-prefix units for Engine/Client/Server.

    A standalone source fixture without a vcxproj remains a single source. A
    product project must register its core exactly once, and missing registered
    sources are errors rather than silently omitted implementation evidence.
    """
    project_root = Path(project_root).resolve()
    core = project_root / 'Private' / (stem + '.cpp')
    project = project_root / 'Default' / (project_root.name + '.vcxproj')
    if not project.exists():
        return (core,)
    info = project.stat()
    sources = _registered_sources(project, info.st_mtime_ns, info.st_size)
    if sources.count(core) != 1:
        raise ValueError(f'Expected exactly one registered {core}')
    excluded = set(exclude_stems)
    if stem == 'Effect_Tool':
        excluded.add('Effect_Tool_V2')
    selected = []
    for source in sources:
        if source.parent != core.parent or source.suffix.lower() != '.cpp':
            continue
        if source.stem != stem and not source.stem.startswith(stem + '_'):
            continue
        if any(source.stem == name or source.stem.startswith(name + '_') for name in excluded):
            continue
        if not source.is_file():
            raise FileNotFoundError(source)
        selected.append(source)
    if len(selected) != len(set(selected)):
        raise ValueError(f'Duplicate source registration in {project}: {stem}')
    return (core,) + tuple(sorted((source for source in selected if source != core), key=lambda p: p.name))


def cpp_domain_header_paths(project_root, stem):
    """Return the registered private internal header moved out of this owner."""
    project_root = Path(project_root).resolve()
    header = project_root / 'Private' / (stem + '_Internal.h')
    project = project_root / 'Default' / (project_root.name + '.vcxproj')
    if not project.exists():
        return ()
    tree = ET.parse(project)
    headers = [(project.parent / item.get('Include').replace('\\', '/')).resolve()
               for item in tree.findall('m:ItemGroup/m:ClInclude', _NAMESPACE)
               if item.get('Include') and '$(' not in item.get('Include')]
    if headers.count(header) > 1:
        raise ValueError(f'Duplicate internal header registration in {project}: {stem}')
    if header not in headers:
        return ()
    if not header.is_file():
        raise FileNotFoundError(header)
    return (header,)


@lru_cache(maxsize=512)
def _source_text(path, modified_ns, size):
    del modified_ns, size
    raw = path.read_bytes()
    for encoding in ('utf-8-sig', 'cp949'):
        try:
            return raw.decode(encoding).replace('\r\n', '\n')
        except UnicodeDecodeError:
            pass
    raise UnicodeError(f'Unsupported C++ source encoding: {path}')


def read_cpp_domain(project_root, stem, *, exclude_stems=()):
    """Read exact source text; separators identify units without hiding content."""
    parts = []
    paths = cpp_domain_paths(project_root, stem, exclude_stems=exclude_stems)
    paths += cpp_domain_header_paths(project_root, stem)
    for path in paths:
        info = path.stat()
        parts.append(_source_text(path, info.st_mtime_ns, info.st_size))
    return '\n\n'.join(parts)


def read_source_text(path, *args, **kwargs):
    """Path.read_text-compatible adapter for existing source inspection tools.

    Only the known split core CPPs expand automatically. Headers, project files,
    JSON and unrelated CPPs retain their original physical read and arguments.
    Mixed existing CP949/new UTF-8 C++ units are decoded individually.
    """
    path = Path(path)
    if path.parent.name == 'Private' and path.suffix == '.cpp' and path.stem in _SPLIT_DOMAINS:
        return read_cpp_domain(path.parent.parent, path.stem)
    return path.read_text(*args, **kwargs)


def _cpp_function_bounds(source, signature):
    start = source.index(signature)
    opening = source.index('{', start)
    while ';' in source[start:opening]:
        start = source.index(signature, start + len(signature))
        opening = source.index('{', start)
    depth, cursor = 0, opening
    state = 'code'
    while cursor < len(source):
        char = source[cursor]
        pair = source[cursor:cursor+2]
        if state == 'code':
            if pair == '//':
                state = 'line'
                cursor += 2
                continue
            if pair == '/*':
                state = 'block'
                cursor += 2
                continue
            if pair == 'R"':
                delimiter_end = source.find('(', cursor+2)
                if delimiter_end != -1 and delimiter_end-cursor <= 18:
                    terminator = ')' + source[cursor+2:delimiter_end] + '"'
                    closing = source.find(terminator, delimiter_end+1)
                    if closing == -1:
                        raise ValueError(f'Unterminated C++ raw string in {signature}')
                    cursor = closing + len(terminator)
                    continue
            if char in ('"', "'"):
                state = char
            elif char == '{':
                depth += 1
            elif char == '}':
                depth -= 1
                if depth == 0:
                    return start, opening, cursor + 1
        elif state == 'line':
            if char == '\n':
                state = 'code'
        elif state == 'block':
            if pair == '*/':
                state = 'code'
                cursor += 2
                continue
        elif char == '\\':
            cursor += 2
            continue
        elif char == state:
            state = 'code'
        cursor += 1
    raise ValueError(f'Unterminated C++ function: {signature}')


def cpp_function_body(source, signature):
    """Extract a definition body independently of translation unit ordering."""
    _, opening, end = _cpp_function_bounds(source, signature)
    return source[opening:end]


def cpp_function_definition(source, signature):
    """Extract the signature and body, skipping preceding declarations."""
    start, _, end = _cpp_function_bounds(source, signature)
    return source[start:end]

"""Stage exact native program registration and matching compiled-shader cohorts."""
from pathlib import Path
import os
import re
import uuid

ROOT = Path(__file__).resolve().parents[2]
REGISTRY = Path('Engine/Public/SourceCharacterProgramRegistry.h')
RESERVED = ((600, 999), (1100, 1599))
PROGRAM_BEGIN = '// BEGIN REGISTERED SOURCE CHARACTER PROGRAMS'
PROGRAM_END = '// END REGISTERED SOURCE CHARACTER PROGRAMS'
GROUP_BEGIN = '// BEGIN REGISTERED SOURCE CHARACTER GROUPS'
GROUP_END = '// END REGISTERED SOURCE CHARACTER GROUPS'


def _section(text, begin, end):
    if text.count(begin) != 1 or text.count(end) != 1:
        raise ValueError('SourceCharacter registry markers changed')
    start = text.index(begin) + len(begin)
    finish = text.index(end, start)
    return text[start:finish]


def registered_programs(text):
    values = tuple(map(int, re.findall(r'\b(\d+)u\b', _section(text, PROGRAM_BEGIN, PROGRAM_END))))
    count = re.search(r'std::array<std::uint32_t, (\d+)> SourceCharacterAddedPrograms', text)
    if count is None or int(count[1]) != len(values):
        raise ValueError('SourceCharacter registry array size disagrees with its exact IDs')
    if tuple(sorted(set(values))) != values:
        raise ValueError('SourceCharacter program registry must be sorted and unique')
    if any(not any(first <= value <= last for first, last in RESERVED) for value in values):
        raise ValueError('SourceCharacter registration is outside a reserved allocation')
    return values


def registry_groups(text):
    block = text.split('SourceCharacterProgramGroups[] = {', 1)[1].split('};', 1)[0]
    groups = tuple((int(a), int(b)) for a, b in re.findall(r'\{(\d+)u,\s*(\d+)u\}', block))
    if not groups or any(a > b for a, b in groups):
        raise ValueError('SourceCharacter cohort registry is invalid')
    for index, (a, b) in enumerate(groups):
        if any(a <= other_b and other_a <= b for other_a, other_b in groups[:index]):
            raise ValueError('SourceCharacter cohorts overlap')
    expected = {(value // 64 * 64, value // 64 * 64 + 63) for value in registered_programs(text)}
    actual = {(int(a), int(b)) for a, b in re.findall(r'\{(\d+)u,\s*(\d+)u\}', _section(text, GROUP_BEGIN, GROUP_END))}
    if actual != expected:
        raise ValueError('SourceCharacter exact IDs and cohorts disagree')
    return groups


def read_registry(root=ROOT):
    text = (Path(root) / REGISTRY).read_text(encoding='utf8')
    return registered_programs(text), registry_groups(text)


def extend_registry(text, programs):
    values = tuple(sorted(set(registered_programs(text)) | set(programs)))
    if any(type(value) is not int or not any(a <= value <= b for a, b in RESERVED) for value in values):
        raise ValueError('SourceCharacter program is outside a reserved allocation')
    def replace_section(source, begin, end, body):
        old = _section(source, begin, end)
        return source.replace(begin + old + end, begin + '\n' + body + '        ' + end, 1)
    text = replace_section(text, PROGRAM_BEGIN, PROGRAM_END, ''.join(f'        {value}u,\n' for value in values))
    groups = sorted({(value // 64 * 64, value // 64 * 64 + 63) for value in values})
    text = replace_section(text, GROUP_BEGIN, GROUP_END, ''.join(f'        {{{a}u, {b}u}},\n' for a, b in groups))
    text, count = re.subn(r'std::array<std::uint32_t, \d+> SourceCharacterAddedPrograms',
                         f'std::array<std::uint32_t, {len(values)}> SourceCharacterAddedPrograms', text)
    if count != 1:
        raise ValueError('SourceCharacter array declaration changed')
    registry_groups(text)
    return text


def validate_program_sources(programs, base, light, configure):
    for value in programs:
        for stage, source in (('Base', base), ('Light', light)):
            name = f'SourceCharacter{stage}{value}'
            if len(re.findall(rf'^SOURCE_CHARACTER_NATIVE_OUTPUT {name}\(', source, re.M)) != 1:
                raise ValueError(f'{value}: missing or duplicate {stage} function')
            if len(re.findall(rf'^    case {value}u:.*\b{name}\(input\)', source, re.M)) != 1:
                raise ValueError(f'{value}: missing or duplicate {stage} dispatch')
        if not re.search(rf'\bstaged\.program = {value}u;', configure):
            raise ValueError(f'{value}: no Configure packing')


def _clone_xml_item(text, tag, old_include, new_include):
    if f'Include="{new_include}"' in text:
        return text
    pattern = rf'(?m)^([ \t]*<{tag} Include="{re.escape(old_include)}"(?:\s*/>|>.*?</{tag}>))'
    matches = list(re.finditer(pattern, text, re.S))
    if len(matches) != 1:
        raise ValueError(f'Expected one {tag} project anchor: {old_include}')
    match = matches[0]
    item = match[1].replace(old_include, new_include, 1)
    return text[:match.end()] + '\n' + item + text[match.end():]


def stage_registration(root, programs, base_source, light_source, configure_source):
    """Return path -> (original bytes, new bytes); this function never writes.

    Source identity/ABI admission remains the caller's responsibility. Only actual
    Base/Light/Configure triples may enable an ID; a compile bucket enables no gaps.
    """
    root = Path(root)
    header_path = root / REGISTRY
    originals = {header_path: header_path.read_bytes()}
    header = originals[header_path].decode('utf8').replace('\r\n', '\n')
    requested = tuple(sorted(set(programs)))
    header = extend_registry(header, requested)
    validate_program_sources(registered_programs(header), base_source, light_source, configure_source)
    staged = {}
    def stage(relative, text):
        path = root / relative
        if path not in originals:
            originals[path] = path.read_bytes() if path.exists() else None
        before = originals[path]
        newline = '\r\n' if before is not None and b'\r\n' in before else '\n'
        after = text.replace('\r\n', '\n').replace('\n', newline).encode('utf8')
        if before != after:
            staged[path] = before, after
    stage(REGISTRY, header)
    groups = sorted({value // 64 * 64 for value in registered_programs(header)})
    projects = {}
    for name in ('Engine', 'Client'):
        for suffix in ('.vcxproj', '.vcxproj.filters'):
            relative = Path(f'{name}/Default/{name}{suffix}')
            originals[root / relative] = (root / relative).read_bytes()
            projects[relative] = originals[root / relative].decode('utf8').replace('\r\n', '\n')
            if name == 'Engine':
                projects[relative] = _clone_xml_item(projects[relative], 'ClInclude',
                    '..\\public\\Model.h', '..\\public\\SourceCharacterProgramRegistry.h')
    for group in groups:
        for name in ('Engine', 'Client'):
            shader_dir = Path(f'{name}/Bin/ShaderFiles')
            wrappers = ['Shader_Deferred'] + (['Shader_VtxAnimMeshBinary', 'Shader_VtxMeshBinary'] if name == 'Client' else [])
            for wrapper in wrappers:
                stage(shader_dir / f'{wrapper}_SourceGroup{group:03d}.hlsl',
                      f'#define SOURCE_CHARACTER_PROGRAM_GROUP {group}\n#include "{wrapper}.hlsl"\n')
            for suffix in ('.vcxproj', '.vcxproj.filters'):
                relative = Path(f'{name}/Default/{name}{suffix}')
                text = projects[relative]
                for leaf in ('Shader_SourceCharacterBaseGroup', 'Shader_SourceCharacterLightGroup'):
                    text = _clone_xml_item(text, 'None', f'..\\Bin\\ShaderFiles\\{leaf}237.hlsli',
                                           f'..\\Bin\\ShaderFiles\\{leaf}{group:03d}.hlsli')
                for wrapper in wrappers:
                    tag = 'None' if name == 'Client' and wrapper == 'Shader_Deferred' else 'FxCompile'
                    text = _clone_xml_item(text, tag, f'..\\Bin\\ShaderFiles\\{wrapper}_SourceGroup237.hlsl',
                                           f'..\\Bin\\ShaderFiles\\{wrapper}_SourceGroup{group:03d}.hlsl')
                if name == 'Client' and suffix == '.vcxproj':
                    text = _clone_xml_item(text, 'ClientEngineCompiledShaders', '$(EngineBinaryRoot)Shader_Deferred_SourceGroup237.cso',
                                           f'$(EngineBinaryRoot)Shader_Deferred_SourceGroup{group:03d}.cso')
                projects[relative] = text
    for relative, text in projects.items():
        stage(relative, text)
    return staged, registry_groups(header), registered_programs(header)


def commit_staged_files(staged, *, expected=None):
    """Compare all files before any replacement; roll back only our own bytes."""
    for path, before in (expected or {}).items():
        current = path.read_bytes() if path.exists() else None
        if current != before:
            raise ValueError(f'Concurrent SourceCharacter source edit: {path}')
    for path, (before, _after) in staged.items():
        current = path.read_bytes() if path.exists() else None
        if current != before:
            raise ValueError(f'Concurrent SourceCharacter edit: {path}')
    written = []
    try:
        for path, (before, after) in staged.items():
            path.parent.mkdir(parents=True, exist_ok=True)
            temp = path.with_name(path.name + '.source-stage-' + uuid.uuid4().hex)
            try:
                temp.write_bytes(after)
                if (path.read_bytes() if path.exists() else None) != before:
                    raise ValueError(f'Concurrent SourceCharacter edit: {path}')
                os.replace(temp, path)
                written.append((path, before, after))
            finally:
                if temp.exists():
                    temp.unlink()
    except Exception:
        for path, before, after in reversed(written):
            if not path.exists() or path.read_bytes() != after:
                continue
            if before is None:
                path.unlink()
            else:
                temp = path.with_name(path.name + '.source-rollback-' + uuid.uuid4().hex)
                temp.write_bytes(before)
                os.replace(temp, path)
        raise

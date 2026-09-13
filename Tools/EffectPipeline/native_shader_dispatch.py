"""Keep native dispatch cases beside their existing compile-time profile group.

The installed source remains the authoring input. Installers expand these includes
before editing their existing sections, then partition complete guarded blocks.
Changing one group's cases does not rewrite the shared native ABI/dispatcher.
"""
from pathlib import Path
import re


NATIVE_DISPATCH_FILES = (
    'Shader_EffectArtistNative.hlsli',
    'Shader_EffectDimensionMasterALTVNative.hlsli',
    'Shader_EffectLanceMasterVANative.hlsli',
    'Shader_EffectWarlordNative.hlsli',
)
_DISPATCH_INCLUDE = re.compile(
    r'#if [^\n]+\n#include "(Shader_Effect\w+NativeDispatch\w+\.hlsli)"\n#endif\n')
_WARLORD_BASE_INCLUDE = re.compile(
    r'#include "(Shader_EffectWarlordNativeGroup000.hlsli)"\n')
_DISPATCH = re.compile(
    r'(EFFECT_PS_OUT Shade_Effect\w+Native\([^\n]+\)\n\{.*?'
    r'switch\s*\(profile\)\s*\{)(.*?)(    default: clip\(-1.f\); return output;)',
    re.S)
_GROUP = re.compile(r'\b(EFFECT_(?:NATIVE_PROFILE|WARLORD)_GROUP) == (\d+)')


def write_if_changed(path, text):
    """Preserve the destination's line endings and timestamp when bytes agree."""
    path = Path(path)
    before = path.read_bytes() if path.is_file() else b''
    newline = '\r\n' if b'\r\n' in before else '\n'
    after = text.replace('\r\n', '\n').replace('\n', newline).encode('utf8')
    if before != after:
        path.write_bytes(after)


def expand_dispatch_includes(text, shader_dir):
    """Read generated native leaves while keeping section markers inline."""
    def read_include(match):
        return (Path(shader_dir) / match[1]).read_text(encoding='utf8')
    text = _WARLORD_BASE_INCLUDE.sub(read_include, text)
    return _DISPATCH_INCLUDE.sub(read_include, text)


def _partition_warlord_base_functions(text):
    """Give the original Warlord group the same physical boundary as later groups."""
    marker = '#if !defined(EFFECT_WARLORD_GROUP) || EFFECT_WARLORD_GROUP == 0\n'
    start = text.index(marker) + len(marker)
    depth, cursor = 1, start
    for line in text[start:].splitlines(keepends=True):
        stripped = line.strip()
        if stripped.startswith(('#if ', '#ifdef ', '#ifndef ')):
            depth += 1
        elif stripped == '#endif':
            depth -= 1
            if not depth:
                body = text[start:cursor]
                if 'float4 WarlordNative400(' not in body or 'case ' in body:
                    raise ValueError('Original Warlord function group is missing')
                name = 'Shader_EffectWarlordNativeGroup000.hlsli'
                return text[:start] + f'#include "{name}"\n' + text[cursor:], {name: body}
        cursor += len(line)
    raise ValueError('Original Warlord function group is incomplete')


def insert_grouped_cases(cases, additions):
    """Insert new complete cases after the last existing block of their group.

    Existing case/guard order is retained. Appending a recovered cohort to an
    already installed bucket therefore edits its leaf instead of adding another
    include to the common dispatcher. New groups are appended normally.
    """
    for addition in additions:
        keys = set(_GROUP.findall(addition))
        if len(keys) != 1:
            raise ValueError('A new native case requires one compile-time group')
        key = next(iter(keys))
        depth, start, cursor, insertion = 0, 0, 0, len(cases)
        for line in cases.splitlines(keepends=True):
            stripped = line.strip()
            if stripped.startswith(('#if ', '#ifdef ', '#ifndef ')):
                if not depth:
                    start = cursor
                depth += 1
            elif stripped == '#endif':
                depth -= 1
                if not depth and key in _GROUP.findall(cases[start:cursor + len(line)]):
                    insertion = cursor + len(line)
            cursor += len(line)
        if depth:
            raise ValueError('Unclosed existing native cases')
        cases = cases[:insertion] + addition + cases[insertion:]
    return cases


def append_dispatch_cases(text, additions):
    """Extend the single Effect dispatcher while keeping existing groups local."""
    matches = list(_DISPATCH.finditer(text))
    if len(matches) != 1:
        raise ValueError('Expected one Effect dispatcher when appending cases')
    match = matches[0]
    cases = insert_grouped_cases(match[2], additions)
    return text[:match.start(2)] + cases + text[match.end(2):]


def _partition_cases(cases, stem):
    """Stage complete preprocessor blocks without changing their order or guards."""
    files, output, pending = {}, [], []
    current_key, section, depth, block = None, 'Base', 0, []

    def flush():
        nonlocal pending, current_key
        if not pending:
            return
        macro, group = current_key
        name = f'{stem}Dispatch{section}{group}.hlsli'
        if name in files:
            # Keep historical case order exactly, including cohorts appended
            # after another bucket. A later installer can edit this same run.
            part = 2
            while f'{stem}Dispatch{section}{group}Part{part}.hlsli' in files:
                part += 1
            name = f'{stem}Dispatch{section}{group}Part{part}.hlsli'
        body = ''.join(pending)
        # Decal/Trail's historical group macro intentionally consumes several
        # Kouku buckets. Preserve those additional consumers on the include too.
        carriers = sorted(set(re.findall(
            r'\|\| defined\((EFFECT_NATIVE_(?:DECAL|TRAIL)_CARRIER)\)', body)))
        condition = f'!defined({macro}) || {macro} == {group}'
        condition += ''.join(f' || defined({carrier})' for carrier in carriers)
        files[name] = body
        output.append(f'#if {condition}\n#include "{name}"\n#endif\n')
        pending, current_key = [], None

    for line in cases.splitlines(keepends=True):
        stripped = line.strip()
        if not depth and not stripped.startswith(('#if ', '#ifdef ', '#ifndef ')):
            if not stripped and pending:
                pending.append(line)
                continue
            flush()
            if stripped.startswith('// BEGIN '):
                section = ''.join(part.title() for part in stripped[9:].split())
            elif stripped.startswith('// END '):
                section = 'Base'
            if stripped and not stripped.startswith('//'):
                raise ValueError(f'Unguarded native dispatch input: {stripped}')
            output.append(line)
            continue
        block.append(line)
        if stripped.startswith(('#if ', '#ifdef ', '#ifndef ')):
            depth += 1
        elif stripped == '#endif':
            depth -= 1
        if depth:
            continue
        text = ''.join(block)
        keys = set(_GROUP.findall(text))
        if len(keys) != 1 or not re.search(r'\bcase \d+u:', text):
            raise ValueError(f'A native dispatch block needs one existing group: {text[:120]}')
        key = next(iter(keys))
        if current_key is not None and current_key != key:
            flush()
        current_key = key
        pending.append(text)
        block = []
    if depth or block:
        raise ValueError('Unclosed native dispatch preprocessor block')
    flush()
    return ''.join(output), files


def partition_dispatch_source(text, path):
    """Validate and stage a file and its native case includes before any writes."""
    path = Path(path)
    expanded = expand_dispatch_includes(text, path.parent)
    matches = list(_DISPATCH.finditer(expanded))
    if len(matches) != 1:
        raise ValueError(f'Expected one native Effect dispatcher in {path.name}')
    match = matches[0]
    cases, files = _partition_cases(match[2], path.stem)
    result = expanded[:match.start(2)] + cases + expanded[match.end(2):]
    if path.name == 'Shader_EffectWarlordNative.hlsli':
        result, functions = _partition_warlord_base_functions(result)
        files.update(functions)
    restored = _DISPATCH_INCLUDE.sub(lambda include: files[include[1]], result)
    restored = _WARLORD_BASE_INCLUDE.sub(lambda include: files[include[1]], restored)
    if restored != expanded:
        raise ValueError(f'Native dispatch source changed while partitioning {path.name}')
    return result, files


def write_partitioned_dispatch(path, text=None):
    """Commit validated leaf includes, then the shared file that consumes them."""
    path = Path(path)
    if text is None:
        text = path.read_text(encoding='utf8')
    result, files = partition_dispatch_source(text, path)
    for name, body in files.items():
        write_if_changed(path.parent / name, body)
    write_if_changed(path, result)
    return tuple(files)


def write_partitioned_source_character_programs(path, text=None):
    """Separate the existing Base/Light preprocessor branches physically.

    The Engine source is canonical. A Light-only material edit must not invalidate
    CModel's Base FX, while both passes keep the exact shared packet and helpers.
    """
    path = Path(path)
    if text is None:
        text = path.read_text(encoding='utf8')
    include = re.compile(r'#include "(Shader_SourceCharacter(?:Base|Light)Programs.hlsli)"\n')
    expanded = include.sub(lambda match: (path.parent / match[1]).read_text(encoding='utf8'), text)
    marker = '#ifdef SOURCE_CHARACTER_LIGHT_PASS\n'
    begin = expanded.rindex(marker)
    start = begin + len(marker)
    depth, cursor, alternate, finish = 1, start, None, None
    for line in expanded[start:].splitlines(keepends=True):
        stripped = line.strip()
        if stripped.startswith(('#if ', '#ifdef ', '#ifndef ')):
            depth += 1
        elif stripped == '#else' and depth == 1:
            alternate = (cursor, cursor + len(line))
        elif stripped == '#endif':
            depth -= 1
            if depth == 0:
                finish = cursor
                break
        cursor += len(line)
    if alternate is None or finish is None:
        raise ValueError('Source character native Base/Light branch is incomplete')
    files = {
        'Shader_SourceCharacterLightPrograms.hlsli': expanded[start:alternate[0]],
        'Shader_SourceCharacterBasePrograms.hlsli': expanded[alternate[1]:finish],
    }
    if 'EvaluateSourceCharacterLight(' not in files['Shader_SourceCharacterLightPrograms.hlsli']:
        raise ValueError('Native Light consumer is missing')
    if 'EvaluateSourceCharacterBase(' not in files['Shader_SourceCharacterBasePrograms.hlsli']:
        raise ValueError('Native Base consumer is missing')
    result = (expanded[:start] + '#include "Shader_SourceCharacterLightPrograms.hlsli"\n'
              + expanded[alternate[0]:alternate[1]] + '#include "Shader_SourceCharacterBasePrograms.hlsli"\n'
              + expanded[finish:])
    if include.sub(lambda match: files[match[1]], result) != expanded:
        raise ValueError('Native character source changed while separating passes')
    for name, body in files.items():
        write_if_changed(path.parent / name, body)
    write_if_changed(path, result)
    return tuple(files)


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--shader-directory', type=Path,
                        default=Path(__file__).resolve().parents[2] / 'Client/Bin/ShaderFiles')
    parser.add_argument('--source-character-directory', type=Path)
    arguments = parser.parse_args()
    for filename in NATIVE_DISPATCH_FILES:
        files = write_partitioned_dispatch(arguments.shader_directory / filename)
        print(filename, 'dispatch includes:', len(files))
    if arguments.source_character_directory:
        files = write_partitioned_source_character_programs(
            arguments.source_character_directory / 'Shader_SourceCharacterPrograms.hlsli')
        print('SourceCharacter pass includes:', ', '.join(files))

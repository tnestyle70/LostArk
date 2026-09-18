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
    text = expand_artist_runtime_source(text, shader_dir)
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
    if path.name == 'Shader_EffectArtistNative.hlsli':
        result, runtime_files = partition_artist_runtime_source(result, path.parent, files)
        files.update(runtime_files)
    for name, body in files.items():
        write_if_changed(path.parent / name, body)
    write_if_changed(path, result)
    if path.name == 'Shader_EffectArtistNative.hlsli':
        select_artist_wrapper_inputs(path.parent)
    return tuple(files)


_ARTIST_RUNTIME_BEGIN = '// BEGIN ISOLATED ARTIST RUNTIME\n'
_ARTIST_RUNTIME_END = '// END ISOLATED ARTIST RUNTIME\n'
_ARTIST_RUNTIME_PARTS = ('Inputs', 'Common', 'Programs')
_ARTIST_EXTERNAL_PROGRAMS_MACRO = 'EFFECT_ARTIST_NATIVE_PROGRAMS_EXTERNAL'


def expand_artist_runtime_source(text, shader_dir):
    """Expose the original authoring text to every existing installer."""
    if _ARTIST_RUNTIME_BEGIN not in text:
        return text
    begin = text.index(_ARTIST_RUNTIME_BEGIN)
    end = text.index(_ARTIST_RUNTIME_END, begin) + len(_ARTIST_RUNTIME_END)
    body = ''.join((Path(shader_dir) / f'Shader_EffectArtistNative{part}.hlsli')
                   .read_text(encoding='utf8') for part in _ARTIST_RUNTIME_PARTS)
    return text[:begin] + body + text[end:]


def artist_program_selection_name(group):
    return f'Shader_EffectArtistNativeSelectedGroup{group}.hlsli'


def artist_wrapper_source(carrier, group):
    # FXC requires a literal include name. Preload the same inputs in the same
    # order as the carrier, then select this cohort before its entry points.
    # Each wrapper still consumes only its own selected-program file.
    if carrier not in ('Mesh', 'Particle'):
        raise ValueError(f'Unsupported Artist carrier: {carrier}')
    scene_inputs = ('Depth', 'Color') if carrier == 'Mesh' else ('Color', 'Depth')
    return (f'#define EFFECT_SHADER_FAMILY 7\n#define EFFECT_NATIVE_PROFILE_GROUP {group}\n'
            f'#define EFFECT_NATIVE_{carrier.upper()}_CARRIER 1\n'
            '#include "Shader_EffectCommon.hlsli"\n'
            '#define EFFECT_NATIVE_CARRIER_COMMON_INCLUDED 1\n'
            + ''.join(f'#include "Shader_EffectScene{kind}Input.hlsli"\n' for kind in scene_inputs)
            + f'#define {_ARTIST_EXTERNAL_PROGRAMS_MACRO} 1\n'
            '#include "Shader_EffectArtistNative.hlsli"\n'
            '#ifndef EFFECT_NATIVE_DECLARATIONS_ONLY\n'
            f'#include "{artist_program_selection_name(group)}"\n'
            '#endif\n'
            f'#include "Shader_Effect{carrier}FamilyCarrier.hlsli"\n')


def select_artist_wrapper_inputs(shader_dir):
    """A new cohort does not edit any existing cohort's compile-time input list."""
    for path in Path(shader_dir).glob('Shader_VtxEffect*.hlsl'):
        text = path.read_text(encoding='utf8')
        if not re.search(r'^#define EFFECT_SHADER_FAMILY 7$', text, re.M):
            continue
        group = re.search(r'^#define EFFECT_NATIVE_PROFILE_GROUP (\d+)$', text, re.M)
        carrier = re.search(r'#include "Shader_Effect(Mesh|Particle)FamilyCarrier.hlsli"', text)
        if group and carrier:
            write_if_changed(path, artist_wrapper_source(carrier[1], int(group[1])))


def _artist_selected_programs(text, group):
    """Resolve only the group selector; retain every original carrier guard."""
    output, stack = [], []
    active = True
    for line in text.splitlines(keepends=True):
        stripped = line.strip()
        if stripped.startswith(('#if ', '#ifdef ', '#ifndef ')):
            selected = 'EFFECT_NATIVE_PROFILE_GROUP' in stripped
            keep = True
            if selected:
                expression = stripped.removeprefix('#if ')
                expression = re.sub(r'defined\((EFFECT_NATIVE_PROFILE_GROUP|ARTIST_NATIVE_MODEL_ONLY|EFFECT_NATIVE_DECAL_CARRIER|EFFECT_NATIVE_TRAIL_CARRIER)\)',
                    lambda m: 'True' if m[1] == 'EFFECT_NATIVE_PROFILE_GROUP' else 'False', expression)
                expression = re.sub(r'\bEFFECT_NATIVE_PROFILE_GROUP\b', str(group), expression)
                expression = expression.replace('&&', ' and ').replace('||', ' or ')
                expression = re.sub(r'!(?!=)', ' not ', expression).strip()
                if re.search(r'[^\d\s()=!<>]|\b[A-Za-z_]\w*\b', re.sub(r'\b(?:True|False|and|or|not)\b', '', expression)):
                    raise ValueError(f'Unsupported Artist group selector: {stripped}')
                keep = bool(eval(expression, {'__builtins__': {}}, {}))
            stack.append((active, selected))
            if active and not selected:
                output.append(line)
            active = active and keep
        elif stripped == '#endif' or stripped.startswith('#endif //'):
            if not stack:
                raise ValueError('Unbalanced Artist program guard')
            parent, selected = stack.pop()
            if parent and not selected:
                output.append(line)
            active = parent
        elif stripped.startswith(('#else', '#elif')):
            # Native source currently has no alternate compile-time branches.
            # Reject a future unsupported selector before changing installed files.
            raise ValueError('Alternate Artist program guard needs explicit selection')
        elif active:
            output.append(line)
    if stack:
        raise ValueError('Unclosed Artist program guard')
    return ''.join(output)


def _artist_profile_predicate(programs, shader_dir, staged):
    include = re.compile(r'#include "(Shader_EffectArtistNativeDispatch\w+\.hlsli)"\n')
    def read(match):
        return staged.get(match[1]) if match[1] in staged else (Path(shader_dir) / match[1]).read_text(encoding='utf8')
    expanded = include.sub(read, programs)
    matches = list(_DISPATCH.finditer(expanded))
    if len(matches) != 1:
        raise ValueError('Artist selection must contain one effect dispatcher')
    guards, cases, seen = [], [], set()
    for line in matches[0][2].splitlines():
        stripped = line.strip()
        if stripped.startswith(('#if ', '#ifdef ', '#ifndef ')):
            guards.append(stripped)
        elif stripped == '#endif':
            guards.pop()
        elif stripped.startswith(('#else', '#elif')):
            raise ValueError('Alternate Artist case guard needs explicit selection')
        else:
            match = re.match(r'case (\d+)u:', stripped)
            if not match:
                continue
            identifier = int(match[1])
            if identifier in seen:
                raise ValueError(f'Duplicate Artist profile {identifier}')
            seen.add(identifier)
            cases.extend(guard + '\n' for guard in guards)
            cases.append(f'    case {identifier}u: return true;\n')
            cases.extend('#endif\n' for _ in guards)
    if guards:
        raise ValueError('Unclosed Artist case guard')
    return ('\nbool Has_EffectArtistNativeProfile(uint profile)\n{\n    switch (profile)\n    {\n'
            + ''.join(cases) + '    default: return false;\n    }\n}\n')


def partition_artist_runtime_source(text, shader_dir, staged=None):
    """Split only ownership: authored helpers, functions and cases are unchanged.

    Also accepts the old ungrouped full-reset generator output. In that mode each
    existing wrapper sees the same full corpus it consumed before this split.
    """
    staged = staged or {}
    expanded = expand_artist_runtime_source(text, shader_dir)
    header = '#define EFFECT_ARTIST_NATIVE_HLSLI\n'
    start = expanded.index(header) + len(header)
    common = expanded.index('float4 ArtistNativeAppend(', start)
    samples = list(re.finditer(r'float4 ArtistNativeSample\d+\(', expanded[common:]))
    if not samples:
        raise ValueError('Artist native sampling ABI is missing')
    sample = common + samples[-1].start()
    brace = expanded.index('{', sample)
    depth, end = 1, brace + 1
    while depth and end < len(expanded):
        depth += (expanded[end] == '{') - (expanded[end] == '}')
        end += 1
    if depth:
        raise ValueError('Artist native sampling helper is incomplete')
    end += len(expanded[end:]) - len(expanded[end:].lstrip('\n'))
    footer = re.search(r'#endif\s*\Z', expanded)
    if not footer or footer.start() < end:
        raise ValueError('Artist native header guard is incomplete')
    files = {
        'Shader_EffectArtistNativeInputs.hlsli': expanded[start:common],
        'Shader_EffectArtistNativeCommon.hlsli': expanded[common:end],
        'Shader_EffectArtistNativePrograms.hlsli': expanded[end:footer.start()],
    }
    facade = (expanded[:start] + _ARTIST_RUNTIME_BEGIN
              + '#include "Shader_EffectArtistNativeInputs.hlsli"\n'
              + '#ifndef EFFECT_NATIVE_DECLARATIONS_ONLY\n'
              + '#include "Shader_EffectArtistNativeCommon.hlsli"\n'
              + f'#ifndef {_ARTIST_EXTERNAL_PROGRAMS_MACRO}\n'
              + '#include "Shader_EffectArtistNativePrograms.hlsli"\n#endif\n#endif\n'
              + _ARTIST_RUNTIME_END + expanded[footer.start():])
    if (expanded[:start] + ''.join(files[f'Shader_EffectArtistNative{part}.hlsli']
                                  for part in _ARTIST_RUNTIME_PARTS) + expanded[footer.start():]) != expanded:
        raise ValueError('Artist runtime partition changed authored source')
    groups = {int(group) for group in re.findall(r'EFFECT_NATIVE_PROFILE_GROUP == (\d+)', expanded)}
    # Legacy full replacement does not emit group guards. Preserve its wrapper
    # behavior instead of leaving a stale selected corpus installed beside it.
    for path in Path(shader_dir).glob('Shader_VtxEffect*.hlsl'):
        source = path.read_text(encoding='utf8')
        if re.search(r'^#define EFFECT_SHADER_FAMILY 7$', source, re.M):
            groups.update(map(int, re.findall(r'^#define EFFECT_NATIVE_PROFILE_GROUP (\d+)$', source, re.M)))
    for group in sorted(groups):
        selected = _artist_selected_programs(files['Shader_EffectArtistNativePrograms.hlsli'], group)
        files[artist_program_selection_name(group)] = selected + _artist_profile_predicate(selected, shader_dir, staged)
    return facade, files


def write_artist_runtime_source(path, text):
    """Install legacy ungrouped full source while retaining the runtime split."""
    path = Path(path)
    facade, files = partition_artist_runtime_source(text, path.parent)
    for name, body in files.items():
        write_if_changed(path.parent / name, body)
    write_if_changed(path, facade)
    select_artist_wrapper_inputs(path.parent)
    return tuple(files)


SOURCE_CHARACTER_PROGRAM_GROUPS = ((1, 8), (9, 16), (17, 24), (25, 32), (80, 83), (84, 91))
_SOURCE_GROUP_INCLUDE = re.compile(r'#include "(Shader_SourceCharacter(?:Base|Light)Group\d+\.hlsli)"\n')
_SOURCE_GROUP_GUARD = re.compile(
    r'^#if !defined\(SOURCE_CHARACTER_PROGRAM_GROUP\) \|\| SOURCE_CHARACTER_PROGRAM_GROUP == \d+\n'
    r'|^#endif // SOURCE_CHARACTER_PROGRAM_GROUP\n', re.M)


def expand_source_character_stage(text, shader_dir):
    """Recover the authored function/case text before a generator edits it."""
    expanded = _SOURCE_GROUP_INCLUDE.sub(
        lambda match: (Path(shader_dir) / match[1]).read_text(encoding='utf8'), text)
    return _SOURCE_GROUP_GUARD.sub('', expanded)


def partition_source_character_stage(text, stage, shader_dir):
    """Place exact native functions in independently compiled program cohorts."""
    expanded = expand_source_character_stage(text, shader_dir)
    prefix = f'SourceCharacter{stage}'
    dispatcher = expanded.index(f'SOURCE_CHARACTER_NATIVE_OUTPUT Evaluate{prefix}(')
    functions, tail = expanded[:dispatcher], expanded[dispatcher:]
    starts = list(re.finditer(
        r'^SOURCE_CHARACTER_NATIVE_OUTPUT (?:SourceCharacter(?:Base|Light)|SourceMapMonsterBaked)(\d+)\(',
        functions, re.M))
    if not starts:
        raise ValueError(f'No SourceCharacter {stage} functions')

    def group(number):
        for first, last in SOURCE_CHARACTER_PROGRAM_GROUPS:
            if first <= number <= last:
                return first
        raise ValueError(f'SourceCharacter program {number} needs a registered CSO cohort')

    output, files = [], {}
    # Preserve complete text in its original order, including source comments.
    cursor = 0
    for index, match in enumerate(starts):
        end = starts[index + 1].start() if index + 1 < len(starts) else len(functions)
        cohort = group(int(match[1]))
        name = f'Shader_SourceCharacter{stage}Group{cohort:03d}.hlsli'
        body = functions[cursor:end]
        cursor = end
        if name not in files:
            output.append(f'#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == {cohort}\n'
                          f'#include "{name}"\n#endif // SOURCE_CHARACTER_PROGRAM_GROUP\n')
            files[name] = body
        else:
            files[name] += body
    # Stable cohorts must be contiguous, otherwise reconstruction would reorder source.
    reconstructed = _SOURCE_GROUP_INCLUDE.sub(lambda match: files[match[1]], ''.join(output))
    if _SOURCE_GROUP_GUARD.sub('', reconstructed) != functions:
        raise ValueError(f'Non-contiguous SourceCharacter {stage} function cohort')
    tail = re.sub(r'^(    case (\d+)u:.*\n)',
                  lambda match: f'#if !defined(SOURCE_CHARACTER_PROGRAM_GROUP) || SOURCE_CHARACTER_PROGRAM_GROUP == {group(int(match[2]))}\n'
                  + match[1] + '#endif // SOURCE_CHARACTER_PROGRAM_GROUP\n', tail, flags=re.M)
    result = ''.join(output) + tail
    restored = _SOURCE_GROUP_INCLUDE.sub(lambda match: files[match[1]], result)
    if _SOURCE_GROUP_GUARD.sub('', restored) != expanded:
        raise ValueError(f'SourceCharacter {stage} partition changed authored source')
    return result, files


def write_partitioned_source_character_stage(path, text=None):
    path = Path(path)
    stage = 'Light' if 'Light' in path.name else 'Base'
    result, files = partition_source_character_stage(
        path.read_text(encoding='utf8') if text is None else text, stage, path.parent)
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
    expanded = include.sub(lambda match: expand_source_character_stage(
        (path.parent / match[1]).read_text(encoding='utf8'), path.parent), text)
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
    leaves = []
    for name, body in files.items():
        leaves.extend(write_partitioned_source_character_stage(path.parent / name, body))
    write_if_changed(path, result)
    return tuple(files) + tuple(leaves)


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

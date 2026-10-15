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
    'KoukuSaydonActionWorkbench', 'ValtanActionWorkbench', 'MainApp',
))


# 기존 core만 검사하던 소비자의 범위를 유지한다. 이번에 옮긴 구현은 포함하되
# 이미 별도 계약으로 읽던 sibling을 새로 합쳐 assertion의 검사 대상을 넓히지 않는다.
_CORE_READ_EXCLUSIONS = {
    'ValtanActionWorkbench': ('ValtanActionWorkbench_Blueprint', 'ValtanActionWorkbench_Timeline'),
    'MainApp': ('MainApp_Dragon', 'MainApp_RenderingLighting', 'MainApp_SequenceViewer',
                'MainApp_ValtanAxe', 'MainApp_WorldLevel'),
}


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
        return read_cpp_domain(path.parent.parent, path.stem,
                               exclude_stems=_CORE_READ_EXCLUSIONS.get(path.stem, ()))
    return path.read_text(*args, **kwargs)


@lru_cache(maxsize=8)
def _cpp_code_mask(source):
    """문자열·주석을 같은 길이의 공백으로 가려 원본 오프셋을 유지한다."""
    masked = list(source)
    cursor = 0
    while cursor < len(source):
        char = source[cursor]
        pair = source[cursor:cursor+2]
        end = None
        if char.isdigit() and (cursor == 0 or not (
                source[cursor - 1].isalnum() or source[cursor - 1] == '_')):
            # 숫자 구분자(1'000, 0xFF'00)는 문자 리터럴을 여는 따옴표가 아니다.
            cursor += 1
            while cursor < len(source) and (source[cursor].isalnum() or source[cursor] in "._'"):
                cursor += 1
            continue
        if pair == '//':
            end = source.find('\n', cursor + 2)
            if end == -1:
                end = len(source)
        elif pair == '/*':
            closing = source.find('*/', cursor + 2)
            if closing == -1:
                raise ValueError('Unterminated C++ block comment')
            end = closing + 2
        elif pair == 'R"':
            delimiter_end = source.find('(', cursor + 2, cursor + 19)
            if delimiter_end != -1:
                delimiter = source[cursor + 2:delimiter_end]
                if not any(c.isspace() or c in ')\\' for c in delimiter):
                    terminator = ')' + delimiter + '"'
                    closing = source.find(terminator, delimiter_end + 1)
                    if closing == -1:
                        raise ValueError('Unterminated C++ raw string')
                    end = closing + len(terminator)
        elif char in ('"', "'"):
            end = cursor + 1
            while end < len(source):
                if source[end] == '\\':
                    end += 2
                elif source[end] == char:
                    end += 1
                    break
                else:
                    end += 1
            else:
                raise ValueError('Unterminated C++ quoted literal')
        if end is not None:
            masked[cursor:end] = ' ' * (end - cursor)
            cursor = end
            continue
        cursor += 1
    return ''.join(masked)


def _cpp_function_bounds(source, signature):
    # 같은 이름의 선언·호출을 먼저 만날 수 있다. 인자 괄호를 닫기 전의
    # initializer/lambda 중괄호는 본문이 아니며, 끝의 세미콜론은 다음 후보로 넘긴다.
    if not signature.strip():
        raise ValueError('A C++ function signature is required')
    code = _cpp_code_mask(source)
    search_from = 0
    while (start := source.find(signature, search_from)) != -1:
        search_from = start + len(signature)
        first = start + len(signature) - len(signature.lstrip())
        if code[first].isspace():
            continue
        parens, brackets = 0, 0
        saw_parameters = False
        opening = None
        for cursor in range(start, len(code)):
            char = code[cursor]
            if char == '(':
                parens += 1
                saw_parameters = True
            elif char == ')':
                parens -= 1
                if parens < 0:
                    break  # if/다른 호출 안에 들어간 함수 호출이다.
            elif char == '[':
                brackets += 1
            elif char == ']':
                brackets -= 1
                if brackets < 0:
                    break
            elif parens == 0 and brackets == 0:
                # 인자 목록보다 앞의 comma는 반환형 template의 일부일 수 있다.
                if char in ';}' or (saw_parameters and char in ',=?'):
                    break
                if char == '{':
                    if saw_parameters:
                        opening = cursor
                    break
        if opening is None:
            continue

        # 선언부와 본문 모두 같은 mask를 소비한다. 반환 문자열은 원문에서
        # 잘라 주므로 주석·리터럴과 공백은 소스 계약 검사의 입력에 그대로 남는다.
        depth = 0
        for cursor in range(opening, len(code)):
            if code[cursor] == '{':
                depth += 1
            elif code[cursor] == '}':
                depth -= 1
                if depth == 0:
                    return start, opening, cursor + 1
        raise ValueError(f'Unterminated C++ function: {signature}')
    raise ValueError(f'C++ function definition not found: {signature}')


def cpp_function_body(source, signature):
    """Extract a definition body independently of translation unit ordering."""
    _, opening, end = _cpp_function_bounds(source, signature)
    return source[opening:end]


def cpp_function_definition(source, signature):
    """Extract the signature and body, skipping preceding declarations."""
    start, _, end = _cpp_function_bounds(source, signature)
    return source[start:end]

"""루트 Data 원본의 SolutionItems만 갱신한다. 빌드·게시·데이터 복사는 하지 않는다."""
from __future__ import annotations

import argparse
import os
from pathlib import Path, PurePosixPath
import re
import subprocess
import tempfile
import uuid


_FOLDER_TYPE = '{2150E333-8FDC-42A3-9474-1A3956D46DE8}'
_PROJECT = re.compile(
    r'^Project\("(?P<kind>\{[^}]+\})"\) = "(?P<name>[^"]+)", '
    r'"(?P<path>[^"]+)", "(?P<guid>\{[^}]+\})"\r?\n.*?^EndProject\r?\n',
    re.MULTILINE | re.DOTALL)
_NESTED = re.compile(
    r'^\tGlobalSection\(NestedProjects\) = preSolution\r?\n(?P<body>.*?)'
    r'^\tEndGlobalSection\r?\n', re.MULTILINE | re.DOTALL)
_PARENT = re.compile(r'^\s*(\{[^}]+\}) = (\{[^}]+\})\s*$')


def _folder_guid(path: PurePosixPath) -> str:
    # PC·열거 순서와 무관하게 같은 물리 상대 경로에는 같은 SolutionFolder ID를 준다.
    return '{' + str(uuid.uuid5(uuid.NAMESPACE_URL,
                               'lostark:data-solution:' + path.as_posix())).upper() + '}'


def _source_path(root: Path, path: Path | str) -> PurePosixPath:
    candidate = Path(str(path).replace('\\', '/'))
    candidate = (candidate if candidate.is_absolute() else root / candidate).resolve()
    relative = candidate.relative_to(root)
    if not relative.parts or relative.parts[0] != 'Data' or len(relative.parts) < 2:
        raise ValueError(f'Data 아래 파일만 등록할 수 있습니다: {path}')
    if candidate.is_dir():
        raise ValueError(f'파일 대신 폴더 경로가 전달되었습니다: {path}')
    if any(character in relative.as_posix() for character in ('"', '\r', '\n', '=')):
        raise ValueError(f'SolutionItems로 표시할 수 없는 경로입니다: {path}')
    return PurePosixPath(relative.as_posix())


def _is_temporary(path: PurePosixPath) -> bool:
    # writer의 잠금·중간 저장·백업은 탐색 링크에서만 제외하고 파일 자체는 보존한다.
    return any(part.lower().endswith(('.writer.lock', '.tmp', '.backup', '.previous'))
               or '.tmp.' in part.lower() or '.backup.' in part.lower()
               for part in path.parts)


def _data_paths(root: Path, paths) -> list[PurePosixPath]:
    if (root / '.git').exists():
        result = subprocess.run(
            ['git', '-C', str(root), 'ls-files', '-z', '--cached', '--others',
             '--exclude-standard', '--', 'Data'], check=True, capture_output=True)
        discovered = [os.fsdecode(value) for value in result.stdout.split(b'\0') if value]
    else:
        # 저장소 밖의 격리 fixture에서도 같은 SolutionItems 변환을 검사할 수 있다.
        discovered = [path for path in (root / 'Data').rglob('*') if path.is_file()]
    selected = {_source_path(root, path) for path in discovered
                if (root / Path(path)).is_file()}
    # publisher가 아직 쓰지 않은 후보도 같은 transaction에 링크할 수 있게 받는다.
    selected.update(_source_path(root, path) for path in paths)
    selected = {path for path in selected if not _is_temporary(path)}
    folded = [path.as_posix().casefold() for path in selected]
    if len(folded) != len(set(folded)):
        raise ValueError('대소문자만 다른 Data 경로가 중복됩니다.')
    return sorted(selected, key=lambda path: (path.as_posix().casefold(), path.as_posix()))


def _managed_folders(root: Path, projects, parents) -> dict[str, PurePosixPath]:
    root_path = PurePosixPath('Data')
    root_id = _folder_guid(root_path)
    managed = {}
    if root_id in projects:
        managed[root_id] = root_path
    elif any(row['kind'].upper() == _FOLDER_TYPE and row['name'] == 'Data'
             and guid not in parents for guid, row in projects.items()):
        raise ValueError('다른 ID의 솔루션 직속 Data 폴더가 이미 있습니다.')
    pending = list(managed)
    while pending:
        parent_id = pending.pop()
        for guid, parent in parents.items():
            if parent == parent_id and guid in projects:
                path = managed[parent_id] / projects[guid]['name']
                if guid == _folder_guid(path):
                    managed[guid] = path
                    pending.append(guid)
    for guid, path in managed.items():
        row = projects[guid]
        if row['kind'].upper() != _FOLDER_TYPE or row['name'] != path.name:
            raise ValueError(f'Data 관리 폴더 ID가 다른 항목에 사용되었습니다: {guid}')
        # 관리 블록에 별도 메타데이터가 추가되었으면 지우지 않고 충돌을 알린다.
        for line in row['match'].group().splitlines()[1:-1]:
            value = line.strip()
            if value in ('ProjectSection(SolutionItems) = preProject', 'EndProjectSection'):
                continue
            left, separator, right = value.partition(' = ')
            if (not separator or left != right or not left.startswith('Data\\')
                    or _source_path(root, left).as_posix() != left.replace('\\', '/')):
                raise ValueError(f'Data 폴더의 별도 항목을 보존하려면 수동 확인이 필요합니다: {line}')
    return managed


def stage_data_solution(root: Path, paths=()) -> list[tuple[Path, bytes, bytes]]:
    """생성 도구가 자기 transaction에 더할 (경로, 원본, 후보)를 반환한다.

    Data 원본은 이동·복사하지 않는다. paths는 root 기준 상대 경로나 Data 아래 절대
    경로이며 아직 쓰지 않은 후보 파일도 허용한다. 이 함수는 쓰지 않으므로 호출자는
    commit 직전에 원본 bytes를 다시 비교하고 기존 rollback/원자 교체를 적용해야 한다.
    """
    root = Path(root).resolve()
    target = root / 'Framework.sln'
    before = target.read_bytes()
    bom = b'\xef\xbb\xbf' if before.startswith(b'\xef\xbb\xbf') else b''
    text = before[len(bom):].decode('utf-8')
    newline = '\r\n' if '\r\n' in text else '\n'
    if any(len(re.findall(r'^' + name + r'\r?$', text, re.MULTILINE)) != 1
           for name in ('Global', 'EndGlobal')):
        raise ValueError('Framework.sln의 단일 Global/EndGlobal 블록을 찾을 수 없습니다.')
    matches = list(_PROJECT.finditer(text))
    projects = {match['guid'].upper(): dict(match.groupdict(), match=match) for match in matches}
    if len(projects) != len(matches):
        raise ValueError('솔루션 Project GUID가 중복됩니다.')
    nested_matches = list(_NESTED.finditer(text))
    if len(nested_matches) > 1:
        raise ValueError('NestedProjects가 중복됩니다.')
    nested = nested_matches[0] if nested_matches else None
    parents = {}
    for line in nested['body'].splitlines() if nested else ():
        match = _PARENT.fullmatch(line)
        if not match or match[1].upper() in parents:
            raise ValueError(f'NestedProjects 연결이 잘못되었습니다: {line}')
        parents[match[1].upper()] = match[2].upper()
    managed = _managed_folders(root, projects, parents)
    grouped = {PurePosixPath('Data'): []}
    for path in _data_paths(root, paths):
        grouped.setdefault(path.parent, []).append(path)
        for folder in path.parents:
            if folder == PurePosixPath('.'):
                break
            grouped.setdefault(folder, [])
    folders = sorted(grouped, key=lambda path: (path.as_posix().casefold(), path.as_posix()))
    new_ids = {_folder_guid(path) for path in folders}
    if (new_ids & projects.keys()) - managed.keys():
        raise ValueError('새 Data 폴더 ID가 기존 솔루션 항목과 충돌합니다.')
    for child, parent in parents.items():
        if child not in managed and parent in managed and parent not in new_ids:
            raise ValueError('삭제할 Data 폴더 아래에 관리 대상이 아닌 솔루션 항목이 있습니다.')
    blocks = []
    links = []
    for folder in folders:
        guid = _folder_guid(folder)
        lines = [f'Project("{_FOLDER_TYPE}") = "{folder.name}", "{folder.name}", "{guid}"']
        if grouped[folder]:
            lines.append('\tProjectSection(SolutionItems) = preProject')
            for path in grouped[folder]:
                include = str(path).replace('/', '\\')
                lines.append(f'\t\t{include} = {include}')
            lines.append('\tEndProjectSection')
        lines.append('EndProject')
        blocks.append(newline.join(lines) + newline)
        if folder != PurePosixPath('Data'):
            links.append(f'\t\t{guid} = {_folder_guid(folder.parent)}{newline}')
    # 생성한 폴더와 그 부모 연결만 교체하고 빌드 맵·실제 프로젝트·다른 섹션은 그대로 둔다.
    edits = [(projects[guid]['match'].start(), projects[guid]['match'].end(), '') for guid in managed]
    if nested:
        retained = ''.join(line for line in nested['body'].splitlines(keepends=True)
                           if _PARENT.fullmatch(line.strip())[1].upper() not in managed)
        replacement = ('\tGlobalSection(NestedProjects) = preSolution' + newline
                       + retained + ''.join(links) + '\tEndGlobalSection' + newline)
        edits.append((nested.start(), nested.end(), replacement))
    elif links:
        end = re.search(r'^EndGlobal\r?$', text, re.MULTILINE)
        if not end:
            raise ValueError('Framework.sln의 EndGlobal을 찾을 수 없습니다.')
        edits.append((end.start(), end.start(), '\tGlobalSection(NestedProjects) = preSolution'
                      + newline + ''.join(links) + '\tEndGlobalSection' + newline))
    global_start = re.search(r'^Global\r?$', text, re.MULTILINE).start()
    edits.append((global_start, global_start, ''.join(blocks)))
    for start, end, replacement in sorted(edits, reverse=True):
        text = text[:start] + replacement + text[end:]
    after = bom + text.encode('utf-8')
    return [(target, before, after)] if before != after else []


def _commit(change: tuple[Path, bytes, bytes]) -> None:
    target, before, after = change
    if target.read_bytes() != before:
        raise RuntimeError(f'후보 생성 뒤 파일이 변경되었습니다: {target}')
    name = None
    try:
        with tempfile.NamedTemporaryFile(dir=target.parent, prefix=target.name + '.',
                                         suffix='.tmp', delete=False) as staged:
            name = staged.name
            staged.write(after)
            staged.flush()
            os.fsync(staged.fileno())
        if target.read_bytes() != before:
            raise RuntimeError(f'교체 직전 파일이 변경되었습니다: {target}')
        os.replace(name, target)
        name = None
    finally:
        if name is not None:
            Path(name).unlink(missing_ok=True)


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument('--check', action='store_true', help='변경이 필요하면 쓰지 않고 종료 코드 1을 반환한다.')
    args = parser.parse_args(argv)
    changes = stage_data_solution(args.root)
    if args.check:
        print('Data SolutionItems: ' + ('갱신 필요' if changes else '최신 상태'))
        return int(bool(changes))
    for change in changes:
        _commit(change)
    print('Data SolutionItems: ' + ('갱신 완료' if changes else '변경 없음'))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
